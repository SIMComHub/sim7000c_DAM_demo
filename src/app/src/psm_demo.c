/*!
  @file
  psm_demo.c

  @brief
  QCLI implementation for PSM.
*/
/*===========================================================================
  Copyright (c) 2017  by Qualcomm Technologies, Inc.  All Rights Reserved.
 
  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.
 ======================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
//#include "qurt_types.h"
#include "qcli.h"
#include "qcli_api.h"
#include "psm_demo.h"
/*#include "timer.h"*/

#include "qapi/qapi.h"
#include "qapi/qapi_status.h"

#include "qapi/qapi_psm.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_psm_status.h"
#include "qapi/qapi_fs.h"
#include "qapi/qapi_timer.h"
#include "qapi/qapi_ns_utils.h"
#include "qapi/qapi_socket.h"
#include "qapi/qapi_ali_iot.h"
#include "msgcfg.h"
#include "msg_mask.h"
#include "qapi_diag.h"

#define SIMCOM_MAX_LOG_MSG_SIZE  100
#define SIMCOM_LOG_MSG( fmt, ... )                                           \
    {                                                                            \
        char log_fmt[ SIMCOM_MAX_LOG_MSG_SIZE ]; \
        char log_buf[ SIMCOM_MAX_LOG_MSG_SIZE ]; \
        strlcpy(log_fmt, "SIMCOM_%s: ", sizeof(log_fmt));\
        strlcat(log_fmt, fmt, sizeof(log_fmt));\
        simcom_format_log_msg( log_buf, SIMCOM_MAX_LOG_MSG_SIZE, log_fmt, __func__, ##__VA_ARGS__ );\
        QAPI_MSG_SPRINTF( MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "%s", log_buf );\
    }

#if 0
#include "pmapp_rtc.h"
#endif

#define FAIL    -1
#define SUCCESS  0


/*=============================================================================
                               HUMAN READABLE Messages
=============================================================================*/
typedef struct
{
    int    err;
    char * err_name;
}psm_demo_err_map_type;

static psm_demo_err_map_type psm_demo_reject_string[] = {
    {PSM_REJECT_REASON_NONE, "NONE"},
    {PSM_REJECT_REASON_NOT_ENABLED, "PSM FEATURE NOT ENABLED"},
    {PSM_REJECT_REASON_MODEM_NOT_READY, "MODEM NOT READY FOR PSM"},
    {PSM_REJECT_REASON_DURATION_TOO_SHORT, "ENTERED PSM DURATION IS TOO SHORT"},
};

static psm_demo_err_map_type psm_demo_status_string[] = {
    {PSM_STATUS_NONE,               "NONE"},
    {PSM_STATUS_REJECT,             "REJECT"},
    {PSM_STATUS_READY,              "READY TO ENTER PSM MODE"},
    {PSM_STATUS_NOT_READY,          "NOT READY TO ENTER PSM MODE"},
    {PSM_STATUS_COMPLETE,           "ENTERED INTO PSM MODE"},
    {PSM_STATUS_DISCONNECTED,       "COMMUNICATION WITH SERVER LOST"},
    {PSM_STATUS_MODEM_LOADED,       "MODEM LOADED"},
    {PSM_STATUS_MODEM_NOT_LOADED,   "MODEM NOT LOADED"},
    {PSM_STATUS_NW_OOS,             "NW OOS"},
    {PSM_STATUS_NW_LIMITED_SERVICE, "NW LIMITED SERVICE"},
    {PSM_STATUS_HEALTH_CHECK,       "HEALT CHECK"},
    {PSM_STATUS_FEATURE_ENABLED,    "PSM FEATURE ENABLED"},
    {PSM_STATUS_FEATURE_DISABLED,   "PSM FEATURE DISABLED"},
};


/*=============================================================================
                               PSM Demo code
=============================================================================*/

static QCLI_Command_Status_t psm_demo_register(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t psm_demo_enter_psm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t psm_demo_unregister(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t psm_demo_cancel_psm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t psm_demo_load_modem(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t psm_demo_simulate_hc_failure(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t update(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t atcmd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

const QCLI_Command_t psm_cmd_list[] =
{
    /* Command_Function, Start_Thread, Command_String, Usage_String, Description */
    {psm_demo_register, false, "REGISTER APPLICATION", "", "Register as PSM aware Application"},
    {psm_demo_unregister, false, "UNREGISTER APPLICATION", "<client_id>", "Unregister as PSM aware Application"},
    {psm_demo_enter_psm, false, "ENTER POWER SAVE MODE", "<client_id> <psm_time_in_secs> <active_time_in_secs> <0/1 - load modem", "Enter into PSM Mode"},
    {psm_demo_cancel_psm, false, "CANCEL POWER SAVE MODE", "<client_id>", "Cancel PSM Mode"},
    {psm_demo_load_modem, false, "LOAD MODEM", "<client_id>", "Load Modem Dynamically"},
    {psm_demo_simulate_hc_failure, false, "SIMULATE HEALTH CHECK FAILURE", "<client_id>", "Simulate Health Check Failure"},
    {update, false, "update", "update", "update self version 3"},
    {atcmd, false, "atcmd", "<at command>" , "Send At command to modem",NULL},
};

const QCLI_Command_Group_t psm_cmd_group =
{
    "PSM",              /* Group_String: will display cmd prompt as "PSM> " */
    sizeof(psm_cmd_list)/sizeof(psm_cmd_list[0]),   /* Command_Count */
    psm_cmd_list        /* Command_List */
};


int health_check_fail;
int backoff;
int psm_complete;
int is_modem_loaded;

QCLI_Group_Handle_t qcli_psm_handle;     /* Handle for PSM Command Group. */

void simcom_format_log_msg ( char *buf_ptr, int buf_size, char *fmt, ...)
{
    va_list ap;
    va_start( ap, fmt );

    if ( NULL != buf_ptr && buf_size > 0 )
    {
        vsnprintf( buf_ptr, buf_size, fmt, ap );
    }

    va_end( ap );
} /* ril_format_log_msg */

int myAtoi(char *str)
{
    int res = 0;
    int i = 0;  

    for (i = 0; str[i] != '\0'; ++i)
        res = res*10 + str[i] - '0';
  
    return res;
}

char * psm_demo_error_to_string(qapi_PSM_Status_t error)
{
    char * err_string;

    switch(error)
    {
        case QAPI_ERR_PSM_FAIL:
            err_string = "FAIL";
            break;

        case QAPI_ERR_PSM_GENERIC_FAILURE:
            err_string = "GENERIC FAILURE";
            break;

        case QAPI_ERR_PSM_APP_NOT_REGISTERED:
            err_string = "APP NOT REGISTERED";
            break;

        case QAPI_ERR_PSM_WRONG_ARGUMENTS:
            err_string = "WRONG VALUES PASSED";
            break;

        case QAPI_ERR_PSM_IPC_FAILURE:
            err_string = "FAILED TO COMMUNICATE WITH SERVER";
            break;

        case QAPI_OK:
            err_string = "SUCCESS";
            break;

        default:
            err_string = "UNKNOWN";
            break;
    }

    return err_string;
}

/*****************************************************************************/
void psm_demo_app_call_back_func(psm_status_msg_type *psm_status)
{
    if ( psm_status )
    {
        // to avoid fludding of logs, don't print health check.
        if ( psm_status->status != PSM_STATUS_HEALTH_CHECK &&
             psm_status->status != PSM_STATUS_NONE )
        {
            QCLI_Printf(qcli_psm_handle, "APPLICATION ID: %d\n", psm_status->client_id);
            QCLI_Printf(qcli_psm_handle, "PSM STATUS:    %s\n", psm_demo_status_string[psm_status->status].err_name);
            QCLI_Printf(qcli_psm_handle, "REJECT REASON: %s\n", psm_demo_reject_string[psm_status->reason].err_name);
        }

        switch(psm_status->status)
        {
            // special handling for some cases like modem loaded, not loaded etc.
            case PSM_STATUS_MODEM_LOADED:
                is_modem_loaded = true;
                break;

            case PSM_STATUS_MODEM_NOT_LOADED:
                is_modem_loaded = false;
                break;

            case PSM_STATUS_HEALTH_CHECK:
                if ( !health_check_fail )
                {
                    qapi_PSM_Client_Hc_Ack(psm_status->client_id);
                }
                break;

            case PSM_STATUS_NW_OOS:
                if ( backoff )
                {
                    qapi_PSM_Client_Enter_Backoff(psm_status->client_id);
                }
                break;

            case PSM_STATUS_NW_LIMITED_SERVICE:
                if ( backoff )
                {
                    qapi_PSM_Client_Enter_Backoff(psm_status->client_id);
                }
                break;

            case PSM_STATUS_COMPLETE:
                psm_complete = true;
                QCLI_Printf(qcli_psm_handle, "Received PSM complete\n");
                break;

            default:
                break;
        }
    }
}

static QCLI_Command_Status_t psm_demo_register(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int id = 0;
    qapi_PSM_Status_t  result = QAPI_OK;

    if ( Parameter_Count != 0 )
    {
        return QCLI_STATUS_USAGE_E;
    }

    result = qapi_PSM_Client_Register(&id, psm_demo_app_call_back_func);

    QCLI_Printf(qcli_psm_handle, "Result: %s\n", result ? "FAIL": "SUCCESS");
    QCLI_Printf(qcli_psm_handle, "Application Id: %d\n", id);
    return QCLI_STATUS_SUCCESS_E;
}


static QCLI_Command_Status_t psm_demo_enter_psm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int  id;
    qapi_PSM_Status_t  result = QAPI_OK;

    int    wake_up_type = PSM_WAKEUP_MEASUREMENT_NW_ACCESS;
    uint32_t psm_time_in_secs = 0;
    uint32_t active_time_in_secs = 0;
    char   *temp_str;

    psm_info_type      psm_info;

    if ( Parameter_List == NULL || Parameter_Count != 4 )
    {
        return QCLI_STATUS_USAGE_E;
    }

    temp_str = (char*)Parameter_List[0].String_Value;
    id = myAtoi(temp_str);

    temp_str = (char*)Parameter_List[1].String_Value;
    psm_time_in_secs = myAtoi(temp_str);

    temp_str = (char*)Parameter_List[2].String_Value;
    active_time_in_secs = myAtoi(temp_str);

    temp_str = (char*)Parameter_List[3].String_Value;
    wake_up_type = myAtoi(temp_str);

    memset(&psm_info, 0, sizeof(psm_info));
    psm_info.psm_time_info.time_format_flag     = PSM_TIME_IN_SECS;
    psm_info.psm_time_info.psm_duration_in_secs = psm_time_in_secs;
    psm_info.active_time_in_secs                = active_time_in_secs;
    if ( wake_up_type == PSM_WAKEUP_MEASUREMENT_ONLY )
    {
        psm_info.psm_wakeup_type = PSM_WAKEUP_MEASUREMENT_ONLY;
    }
    else
    {
        psm_info.psm_wakeup_type = PSM_WAKEUP_MEASUREMENT_NW_ACCESS;
    }

    result = qapi_PSM_Client_Enter_Psm(id, &psm_info);

    QCLI_Printf(qcli_psm_handle, "Result: %s\n", result ? "FAIL":"SUCCESS");
    if ( result != QAPI_OK )
    {
        QCLI_Printf(qcli_psm_handle, "Reason: %s\n", psm_demo_error_to_string(result));
    }
    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t psm_demo_unregister(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int     id;
    char   *temp_str;
    qapi_PSM_Status_t  result = QAPI_OK;


    if ( Parameter_List == NULL || Parameter_Count != 1 )
    {
        return QCLI_STATUS_USAGE_E;
    }

    temp_str = (char*)Parameter_List[0].String_Value;
    id = myAtoi(temp_str);

    result = qapi_PSM_Client_Unregister(id);

    QCLI_Printf(qcli_psm_handle, "Result: %s\n", result ? "FAIL": "SUCCESS");
    if ( result != QAPI_OK )
    {
        QCLI_Printf(qcli_psm_handle, "Reason: %s\n", psm_demo_error_to_string(result));
    }
    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t psm_demo_cancel_psm(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int                id;
    char              *temp_str;
    qapi_PSM_Status_t  result = QAPI_OK;


    if ( Parameter_List == NULL || Parameter_Count != 1 )
    {
        return QCLI_STATUS_USAGE_E;
    }

    temp_str = (char*)Parameter_List[0].String_Value;
    id = myAtoi(temp_str);

    result = qapi_PSM_Client_Cancel_Psm(id);

    QCLI_Printf(qcli_psm_handle, "Result: %s\n", result ? "FAIL":"SUCCESS");
    if ( result != QAPI_OK )
    {
        QCLI_Printf(qcli_psm_handle, "Reason: %s\n", psm_demo_error_to_string(result));
    }
    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t psm_demo_load_modem(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int                id;
    char              *temp_str;
    qapi_PSM_Status_t  result = QAPI_OK;


    if ( Parameter_List == NULL || Parameter_Count != 1 )
    {
        return QCLI_STATUS_USAGE_E;
    }

    temp_str = (char*)Parameter_List[0].String_Value;
    id = myAtoi(temp_str);

    result = qapi_PSM_Client_Load_Modem(id);

    QCLI_Printf(qcli_psm_handle, "Result: %s\n", result ? "FAIL":"SUCCESS");
    if ( result != QAPI_OK )
    {
        QCLI_Printf(qcli_psm_handle, "Reason: %s\n", psm_demo_error_to_string(result));
    }
    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t psm_demo_simulate_hc_failure(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ( Parameter_List == NULL || Parameter_Count != 1 )
    {
        return QCLI_STATUS_USAGE_E;
    }

    health_check_fail = 1;
    QCLI_Printf(qcli_psm_handle, "Result: %s\n", "SUCCESS");
    return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t update(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_FS_Update_App();
    return QCLI_STATUS_SUCCESS_E;
}

/*****************************************************************************
 * This function is used to register the PSM Command Group with QCLI.
 *****************************************************************************/
void Initialize_PSM_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_psm_handle = QCLI_Register_Command_Group(NULL, &psm_cmd_group);
    if (qcli_psm_handle)
    {
      QCLI_Printf(qcli_psm_handle, "PSM Registered\n");
    }

    return;
}

static QCLI_Command_Status_t atcmd(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{   
    if (Parameter_Count == 1)  
    { 
        unsigned char data[2048];       
        int len = 0;         
        memset(data,0,2048);   
        memcpy(data,Parameter_List[0].String_Value,strlen((char *)Parameter_List[0].String_Value));
        
        memcpy(data+strlen((char *)data),"\r\n",2);
        IOT_Visual_AT_Input(data,strlen((char *)data));
        SIMCOM_LOG_MSG("atcmd is :%s,%d", data,strlen((char *)data));
      
        do
        {
            qapi_Timer_Sleep(50,QAPI_TIMER_UNIT_MSEC,false);
            memset(data,0x00,2048);         
            len=  IOT_Visual_AT_Output(data,2048);
            if(len > 0)        
            {
                QCLI_Printf(qcli_psm_handle, "%s",data);
            }
        }while(len > 0);       
    }
    else 
    {
        return QCLI_STATUS_USAGE_E;     
    }
    
    return QCLI_STATUS_SUCCESS_E;   
}

