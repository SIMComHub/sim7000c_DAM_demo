/*!
  @file
  http_demo.c

  @brief
  QCLI implementation for HTTP.
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
#include "qcli.h"
#include "qcli_api.h"
#include "http_demo.h"
#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_fs.h"
#include "qapi/qapi_timer.h"
#include "msgcfg.h"
#include "msg_mask.h"
#include "qapi_diag.h"
#include "qapi/qapi_dss.h"
#include "qapi/qapi_socket.h"
#include "qapi/qapi_dnsc.h"
#include "netutils.h"
#include "qapi/qapi_ns_utils.h"

static qapi_DSS_Hndl_t NW_dss_handle;

/*=============================================================================
                               HUMAN READABLE Messages
=============================================================================*/

/*=============================================================================
                               HTTP Demo code
=============================================================================*/

static QCLI_Command_Status_t socket_demo_NWinit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t socket_demo_TCP(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t socket_demo_UDP(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);


const QCLI_Command_t socket_cmd_list[] =
{
    /* Command_Function, Start_Thread, Command_String, Usage_String, Description */
    {socket_demo_NWinit, false, "NETWORK INIT", "[<APN>]", "Init the network, the APN is not integrant"},
    {socket_demo_TCP, false, "TCP SOCKET", "<IP><port><data>", "Send and receive TCP packet"},
    {socket_demo_UDP, false, "UDP SOCKET", "<data>", "Send and receive UDP packet"},
};

const QCLI_Command_Group_t socket_cmd_group =
{
    "SOCKET",              /* Group_String: will display cmd prompt as "HTTP> " */
    sizeof(socket_cmd_list)/sizeof(socket_cmd_list[0]),   /* Command_Count */
    socket_cmd_list        /* Command_List */
};

QCLI_Group_Handle_t qcli_socket_handle;     /* Handle for HTTP Command Group. */

extern int myAtoi(char *str);


/*****************************************************************************
 * This function is used to register the HTTP Command Group with QCLI.
 *****************************************************************************/
 static uint32 simcom_ipaddr8_to_32(uint8 *buf,uint32 *ipaddr)
{
    uint8      c = 0;
    uint8      i = 0;
    uint8      j = 0;
    bool    result = true; 
    uint8      length = strlen((char *)buf);
    uint8      ip[4];
    uint8      cnt = 0;
    uint16     temp_ip = 0;     

    while(i < length)
    {
        c = buf[i++];
        if((c >= '0') && (c <= '9'))
        {
            temp_ip = temp_ip * 10 + (c - '0');
            if(temp_ip > 255)
            {
                result = false;
                break;
            }
        }
        else if((c == '.' ) && (temp_ip < 256)) 
        {
            ip[j++] = (uint8)temp_ip;
            cnt ++;
            temp_ip = 0;
        }
        else
        {
            result = false;
            break;
        }
    }

    if(result == true)
    {
        ip[j] = (uint8)temp_ip;
        cnt++;
        
        if(cnt == 4)
        {
            *ipaddr =  ((((uint32)(ip[3]<<24))&0xff000000) | (((uint32)(ip[2]<<16))&0x00ff0000) | (((uint32)(ip[1]<<8))&0x0000ff00) | (((uint32)ip[0])&0x000000ff));
        }
        else
        {
            result =false;
        }
    }

    return result;
}
 
void Initialize_SOCK_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_socket_handle = QCLI_Register_Command_Group(NULL, &socket_cmd_group);
    if (qcli_socket_handle)
    {
      QCLI_Printf(qcli_socket_handle, "SOCKET Registered\n");
    }

    return;
}
/*SIMCOM tong.zhao 2017-11-3 modify for configure DNS when the network connection start*/
char g_APN[50] = {0};
qapi_DSS_Net_Evt_t socket_last_evt;
static bool NW_dss_init(char * APN);
static void NW_dss_cb(
    qapi_DSS_Hndl_t                 hndl,                /* Handle for which this event is associated */
    void                                     *user_data,         /* Application-provided user data */
    qapi_DSS_Net_Evt_t          evt,                     /* Event identifier */
    qapi_DSS_Evt_Payload_t *payload_ptr      /* Associated event information */
)
{
    qapi_DSS_Addr_Info_t *info_ptr = NULL;
    unsigned int len = 0;
    char dp_v4[16];
    char ds_v4[16];

    if(evt == QAPI_DSS_EVT_NET_IS_CONN_E)
    {
        if (evt != socket_last_evt) {
            QCLI_Printf(qcli_socket_handle, "active network\n");
        }
        if(0 == qapi_Net_DNSc_Is_Started())
        {
            qapi_Net_DNSc_Command(QAPI_NET_DNS_START_E);
        }


        if (qapi_DSS_Get_IP_Addr_Count(NW_dss_handle, &len) == QAPI_OK)
        {
            info_ptr = malloc(sizeof(qapi_DSS_Addr_Info_t)*len);
            memset(info_ptr, 0, (sizeof(qapi_DSS_Addr_Info_t)*len));
            if (qapi_DSS_Get_IP_Addr(NW_dss_handle,info_ptr,len) == QAPI_OK)
            {
                if((NULL != inet_ntop(AF_INET,&info_ptr->dnsp_addr_s.addr.v4,dp_v4,sizeof(dp_v4)))
                    &&(NULL != inet_ntop(AF_INET,&info_ptr->dnss_addr_s.addr.v4,ds_v4,sizeof(ds_v4))))
                {
                    if((0 != qapi_Net_DNSc_Add_Server(dp_v4, QAPI_NET_DNS_ANY_SERVER_ID) )
                    	|| (0 !=qapi_Net_DNSc_Add_Server(ds_v4, QAPI_NET_DNS_ANY_SERVER_ID)))
                    {
                    		qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
                    		qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);
                    }
                }
                else
                {
                    qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
                    qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);
                }
            }
            else
            {
                qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
                qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);
            }
        }
        else
        {
            qapi_Net_DNSc_Add_Server("114.114.114.114", QAPI_NET_DNS_ANY_SERVER_ID);
            qapi_Net_DNSc_Add_Server("8.8.8.8", QAPI_NET_DNS_ANY_SERVER_ID);
        }
    }
    else
    {
        if (evt != socket_last_evt) {
            QCLI_Printf(qcli_socket_handle, "not active network\n");
        }
        if(NW_dss_handle != NULL)
            qapi_DSS_Rel_Data_Srvc_Hndl(NW_dss_handle);
        NW_dss_handle = NULL;
        NW_dss_init(g_APN);
    }
    socket_last_evt = evt;
}
static bool NW_dss_init(char * APN)
{
    int result = 0;
    qapi_DSS_Call_Param_Value_t param_info;

 
    result = qapi_DSS_Get_Data_Srvc_Hndl ( NW_dss_cb, NULL,  &NW_dss_handle );
 
    if ( NW_dss_handle != NULL )
    {   
        param_info.buf_val = NULL;
        param_info.num_val = QAPI_DSS_RADIO_TECH_UNKNOWN;
        result = qapi_DSS_Set_Data_Call_Param(NW_dss_handle, QAPI_DSS_CALL_INFO_TECH_PREF_E, &param_info);
 
        param_info.buf_val = NULL;
        param_info.num_val = QAPI_DSS_IP_VERSION_4;
        result = qapi_DSS_Set_Data_Call_Param(NW_dss_handle, QAPI_DSS_CALL_INFO_IP_VERSION_E, &param_info);
 
        param_info.buf_val = NULL;
        param_info.num_val = 1;
        result = qapi_DSS_Set_Data_Call_Param(NW_dss_handle, QAPI_DSS_CALL_INFO_UMTS_PROFILE_IDX_E, &param_info);

        if(APN != NULL)
        {
	    memcpy(g_APN, APN, strlen(APN));
            param_info.buf_val = APN;
            param_info.num_val = strlen(APN);
            result = qapi_DSS_Set_Data_Call_Param(NW_dss_handle, QAPI_DSS_CALL_INFO_APN_NAME_E, &param_info);
        }
	else
        {
            memset(g_APN, 0x00, sizeof(g_APN));
        }
    }   
 
    result = qapi_DSS_Start_Data_Call ( NW_dss_handle);
    return result;
}
/*SIMCOM tong.zhao 2017-11-3 modify for configure DNS when the network connection end*/

static QCLI_Command_Status_t socket_demo_NWinit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char   *temp_str = NULL;
    
    if ( Parameter_List != NULL )
    {
        if(Parameter_Count != 1)
            return QCLI_STATUS_USAGE_E;
        temp_str = (char*)Parameter_List[0].String_Value;
    }

    NW_dss_init(temp_str);
    return QCLI_STATUS_SUCCESS_E;
}
static QCLI_Command_Status_t socket_demo_TCP(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *url =NULL;
    char *temp_str = NULL;
    char *data = NULL;
    int port;
    int tcp_fd;
    struct sockaddr_in dest_addr;
    if ( Parameter_List == NULL || Parameter_Count != 3 )
    {
        return QCLI_STATUS_USAGE_E;
    }
    url = (char*)Parameter_List[0].String_Value;
    temp_str = (char*)Parameter_List[1].String_Value;
    port = myAtoi(temp_str);
    data = (char*)Parameter_List[2].String_Value;

    memset(&dest_addr, 0x00, sizeof(struct sockaddr_in));
    //send TCP packet
    tcp_fd = qapi_socket(AF_INET, SOCK_STREAM, 0);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    simcom_ipaddr8_to_32((uint8 *)url, (uint32 *)&dest_addr.sin_addr.s_addr);
    memset(dest_addr.sin_zero, 0x00, 8);
    if(0 == qapi_connect(tcp_fd, (struct sockaddr * )&dest_addr, sizeof(dest_addr)))
    {
        int len,bytes_send;
        len = strlen(data);
        bytes_send = qapi_send(tcp_fd, data, len, 0);
        if(bytes_send > 0)
        {
            char buf[512] = {0};
            int bytes_recv = 0;
            bytes_recv = qapi_recv(tcp_fd, buf, 512, 0);
            if(bytes_recv > 0)
            {
                QCLI_Printf(qcli_socket_handle, "Receive data: len=%d,data=%s", strlen(buf), buf);
            }
            else
            {
                QCLI_Printf(qcli_socket_handle, "Receive TCP packet error!");
            }
        }
        else
        {
            QCLI_Printf(qcli_socket_handle, "Send TCP packet error!");
        }
    }
    else
    {
        QCLI_Printf(qcli_socket_handle, "Connect error!");
    }
    qapi_socketclose(tcp_fd);
	return QCLI_STATUS_SUCCESS_E;
}

static QCLI_Command_Status_t socket_demo_UDP(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *url =NULL;
    char *temp_str = NULL;
    char *data = NULL;
    int port;
    int udp_fd;
    struct sockaddr_in dest_addr;
    if ( Parameter_List == NULL || Parameter_Count != 3 )
    {
        return QCLI_STATUS_USAGE_E;
    }
    url = (char*)Parameter_List[0].String_Value;
    temp_str = (char*)Parameter_List[1].String_Value;
    port = myAtoi(temp_str);
    data = (char*)Parameter_List[2].String_Value;
    memset(&dest_addr, 0x00, sizeof(struct sockaddr_in));
    udp_fd = qapi_socket(AF_INET, SOCK_DGRAM, 0);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    simcom_ipaddr8_to_32((uint8 *)url, (uint32 *)&dest_addr.sin_addr.s_addr);
    memset(dest_addr.sin_zero, 0x00, 8);
    {
        int len,bytes_send;
        len = strlen(data);
        bytes_send = qapi_sendto(udp_fd, data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if(bytes_send > 0)
        {
            char buf[512] = {0};
            int bytes_recv = 0;
            int fromlen = sizeof(struct sockaddr);
            bytes_recv = qapi_recvfrom(udp_fd, buf, 512, 0, (struct sockaddr *)&dest_addr, &fromlen);
            if(bytes_recv > 0)
            {
                QCLI_Printf(qcli_socket_handle, "Receive data: len=%d,data=%s", strlen(buf), buf);
            }
            else
            {
                QCLI_Printf(qcli_socket_handle, "Receive UDP packet error!");
            }
        }
        else
        {
            QCLI_Printf(qcli_socket_handle, "Send UDP packet error=%d", qapi_errno(udp_fd));
        }
    }
    qapi_socketclose(udp_fd);
	return QCLI_STATUS_SUCCESS_E;
}

