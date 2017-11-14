/*!
  @file
  qapi_lwm2m.h

  @brief
  LWM2M interface file to interact with the LWM2M client module. 
*/

/*===========================================================================

  Copyright (c) 2008-2017 Qualcomm Technologies, Inc. All Rights Reserved

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

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: 

when       who     what, where, why
--------   ---     ----------------------------------------------------------
27/08/17  vpulimi  Added support for extensible LWM2M objects.
===========================================================================*/

#ifndef LWM2M_QAPI_H
#define LWM2M_QAPI_H

#include "stdbool.h"
#include "qapi_status.h"
#include "qapi_data_txm_base.h"


/**
* @brief  Maximum allowed LWM2M message ID size 
*/
#define QAPI_MAX_LWM2M_MSG_ID_LENGTH  10

/* LWM2M client request IDs */
#define  TXM_QAPI_LWM2M_REGISTER_APP               TXM_QAPI_NET_LWM2M_BASE + 1
#define  TXM_QAPI_LWM2M_REGISTER_APP_EXTENDED      TXM_QAPI_NET_LWM2M_BASE + 2
#define  TXM_QAPI_LWM2M_DEREGISTER_APP             TXM_QAPI_NET_LWM2M_BASE + 3
#define  TXM_QAPI_LWM2M_OBSERVE                    TXM_QAPI_NET_LWM2M_BASE + 4
#define  TXM_QAPI_LWM2M_CANCEL_OBSERVE             TXM_QAPI_NET_LWM2M_BASE + 5
#define  TXM_QAPI_LWM2M_CREATE_OBJECT_INSTANCE     TXM_QAPI_NET_LWM2M_BASE + 6
#define  TXM_QAPI_LWM2M_DELETE_OBJECT_INSTANCE     TXM_QAPI_NET_LWM2M_BASE + 7
#define  TXM_QAPI_LWM2M_GET                        TXM_QAPI_NET_LWM2M_BASE + 8
#define  TXM_QAPI_LWM2M_SET                        TXM_QAPI_NET_LWM2M_BASE + 9
#define  TXM_QAPI_LW2M_SEND_MESSAGE                TXM_QAPI_NET_LWM2M_BASE + 10
#define  TXM_QAPI_LWM2M_ENCODE_APP_PAYLOAD         TXM_QAPI_NET_LWM2M_BASE + 11
#define  TXM_QAPI_LWM2M_DECODE_APP_PAYLOAD         TXM_QAPI_NET_LWM2M_BASE + 12
#define  TXM_QAPI_LWM2M_WAKEUP                     TXM_QAPI_NET_LWM2M_BASE + 13


/**
* @brief  Handler provide by LWM2M client to the application
*/
typedef void * qapi_Net_LWM2M_App_Handler_t;

/*
 * @brief This Enum is used to identify the particular object with  object id 
*/
typedef enum qapi_Net_LWM2M_Object_ID_e 
{
  QAPI_NET_LWM2M_DEVICE_OBJECT_ID_E          =  3,
  QAPI_NET_LWM2M_FIRMWARE_UPDATE_OBJECT_ID_E =  5,
  QAPI_NET_LWM2M_LOCATION_OBJECT_ID_E        =  6,
  QAPI_NET_LWM2M_SOFTWARE_MGNT_OBJECT_ID_E   =  9,
  QAPI_NET_LWM2M_DEVICE_CAP_OBJECT_ID_E      = 15,
} qapi_Net_LWM2M_Object_ID_t;

/* 
 *@brief This Enum is used to identify the particular resource of device capability object 
*/  
typedef enum qapi_Net_LWM2M_Devicecap_Resource_Id_e 
{
  QAPI_NET_LWM2M_DEVICE_RES_M_PROPERTY_E         = 0,
  QAPI_NET_LWM2M_DEVICE_RES_M_GROUP_E            = 1,
  QAPI_NET_LWM2M_DEVICE_RES_O_DESCRIPTION_E      = 2,
  QAPI_NET_LWM2M_DEVICE_RES_O_ATTACHED_E         = 3,
  QAPI_NET_LWM2M_DEVICE_RES_M_ENABLED_E          = 4,
  QAPI_NET_LWM2M_DEVICE_RES_M_OP_ENABLE_E        = 5,
  QAPI_NET_LWM2M_DEVICE_RES_M_OP_DISBALE_E       = 6,
  QAPI_NET_LWM2M_DEVICE_RES_O_NOTIFY_EN_E        = 7
} qapi_Net_LWM2M_Devicecap_Resource_Id_t;

/* This Enum is used to identify the particular resource of Firmware  object */ 
typedef enum qapi_Net_LWM2M_Fota_Resource_Id_e 
{
  QAPI_NET_LWM2M_FOTA_RES_M_PACKAGE_E                 =  0 ,
  QAPI_NET_LWM2M_FOTA_RES_M_PACKAGE_URI_E             =  1 ,
  QAPI_NET_LWM2M_FOTA_RES_M_UPDATE_E                  =  2 ,
  QAPI_NET_LWM2M_FOTA_RES_M_STATE_E                   =  3 ,
  QAPI_NET_LWM2M_FOTA_RES_M_UPDATE_RESULT_E           =  5 ,
  QAPI_NET_LWM2M_FOTA_RES_O_PACKAGE_NAME_E            =  6,
  QAPI_NET_LWM2M_FOTA_RES_O_PACKAGE_VERSION_E         =  7,
  QAPI_NET_LWM2M_FOTA_RES_O_UPDATE_PROTOCOL_SUPPORT_E =  8,
  QAPI_NET_LWM2M_FOTA_RES_M_UPDATE_DELIVERY_METHOD_E  =  9,
} qapi_Net_LWM2M_Fota_Resource_Id_t;

typedef enum  qapi_Net_LWM2M_Fota_Result_t_e 
{
  QAPI_NET_LWM2M_FOTA_RESULT_INITIAL_E = 0,
  QAPI_NET_LWM2M_FOTA_RESULT_UPDATE_SUCCESS_E = 1,
  QAPI_NET_LWM2M_FOTA_RESULT_NOT_ENOUGH_STORAGE_E = 2,
  QAPI_NET_LWM2M_FOTA_RESULT_OUT_OF_MEMORY_E = 3,
  QAPI_NET_LWM2M_FOTA_RESULT_CONNECTION_LOST_E = 4,
  QAPI_NET_LWM2M_FOTA_RESULT_CRC_CHECK_FAIL_E = 5,
  QAPI_NET_LWM2M_FOTA_RESULT_UNSUPPORTED_PACKAGE_TYPE_E = 6,
  QAPI_NET_LWM2M_FOTA_RESULT_INVAILD_URI_E = 7,
  QAPI_NET_LWM2M_FOTA_RESULT_UPDATE_FAILED_E = 8,
  QAPI_NET_LWM2M_FOTA_RESULT_UNSUPPORTED_PROTOCOL_E = 9,
} qapi_Net_LWM2M_Fota_Result_t;	

typedef enum qapi_Net_LWM2M_Fota_Supported_Protocols_e 
{
  QAPI_NET_LWM2M_FOTA_PROTOCOL_COAP  = 0, 
  QAPI_NET_LWM2M_FOTA_PROTOCOL_COAPS = 1, 
  QAPI_NET_LWM2M_FOTA_PROTOCOL_HTTP  = 2,
  QAPI_NET_LWM2M_FOTA_PROTOCOL_HTTPS = 3,
} qapi_Net_LWM2M_Fota_Supported_Protocols_t;

typedef enum qapi_Net_LWM2M_Fota_Update_Delivery_Method_e 
{
  QAPI_NET_LWM2M_FOTA_UPDATE_PUSH_E = 0, /**< Supports only Package method */
  QAPI_NET_LWM2M_FOTA_UPDATE_PULL_E = 1, /**< Supports only Package URI method */
  QAPI_NET_LWM2M_FOTA_UPDATE_BOTH_E = 2, /**< Supports both Package and Package URI method */
} qapi_Net_LWM2M_Fota_Update_Delivery_Method_t;

typedef enum qapi_Net_LWM2M_Location_Resource_Id_e
{
  QAPI_NET_LWM2M_LOCATION_RES_O_RADIUS_E = 3,
} qapi_Net_LWM2M_Location_Resource_Id_t;

/* 
 * @brief This Enum is used to identify the particular resource of Software management object 
*/ 
typedef enum qapi_Net_LWM2M_SW_Mgnt_Resource_Id_e 
{
  QAPI_NET_LWM2M_SW_MGNT_RES_O_PACKAGE_NAME_E      = 0,
  QAPI_NET_LWM2M_SW_MGNT_RES_O_PACKAGE_VERSION_E   = 1,
  QAPI_NET_LWM2M_SW_MGNT_RES_O_PACKAGE_E           = 2,  /**< Resource ID for "Package" */
  QAPI_NET_LWM2M_SW_MGNT_RES_O_PACKAGE_URI_E       = 3,  /**< Resource ID for "Package URI" */
  QAPI_NET_LWM2M_SW_MGNT_RES_M_INSTALL_E           = 4,  /**< Resource ID for "Install" */
  QAPI_NET_LWM2M_SW_MGNT_RES_M_UNINSTALL_E         = 6,  /**< Resource ID for "Uninstall" */
  QAPI_NET_LWM2M_SW_MGNT_RES_M_UPDATE_STATE_E      = 7,  /**< Resource ID for "Update State" */
  QAPI_NET_LWM2M_SW_MGNT_RES_M_UPDATE_RESULT_E     = 9,  /**< Resource ID for "Update Result" */   
  QAPI_NET_LWM2M_SW_MGNT_RES_M_ACTIVATE_E          = 10, /**< Resource ID for "Activate" State" */
  QAPI_NET_LWM2M_SW_MGNT_RES_M_DEACTIVATE_E        = 11, /**< Resource ID for "Deactivate" */
  QAPI_NET_LWM2M_SW_MGNT_RES_M_ACTIVATION_STATE_E  = 12, /**< Resource ID for "Activation */
} qapi_Net_LWM2M_SW_Mgnt_Resource_Id_t;

/* 
 *@brief This Enum is used to identify the particular Error value of Software management object 
*/ 
typedef enum qapi_Net_LWM2M_SW_Mgnt_Error_Value_e 
{
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_INITIAL_E             = 0 , /**< Update result "Initial" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_DOWNLOADING_E         = 1 , /**< Update result "Downloading" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_INSTALL_SUCCESS_E     = 2 , /**< Update result "install success" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_NO_ENOUGH_STORAGE_E   = 50, /**< Update result "Not enoughstorage" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_OUT_OF_MEMORY_E       = 51, /**< Update result "Device is out ofmemory" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_CONNECTION_LOST_E     = 52, /**< Update result "Connection lost" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_PKG_CHECK_FAILURE_E   = 53, /**< Update result "Package check failure" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_PKG_UNSUPPORTED_E     = 54, /**< Update result "packageunsupported" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_INVALID_URI_E         = 56, /**< Update result "URI is invalid" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_UPDATE_ERROR_E        = 57, /**< Update result "Update error" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_INSTALL_ERROR_E       = 58, /**< Update result "Install error" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_RES_UNINSTALL_ERROR_E     = 59, /**< Update result "uninstall error" */
} qapi_Net_LWM2M_SW_Mgnt_Error_Value_t;

/* 
 *@brief  This Enum is used to identify the particular STATE  of Software management object  
*/
typedef enum qapi_Net_LWM2M_SW_Mgnt_State_e
{
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_STATE_INITIAL_E            = 0, /**< Update state "initial" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_STATE_DOWNLOAD_STARTED_E   = 1, /**< Update state "Download started" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_STATE_DOWNLOADED_E         = 2, /**< Update state "Downloaded" */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_STATE_DELIVERED_E          = 3, /**< Update state "Delivered " */
  QAPI_NET_LWM2M_SW_MGNT_UPDATE_STATE_INSTALLED_E          = 4, /**< Update state "State installed" */
} qapi_Net_LWM2M_SW_Mgnt_State_t;

/* 
 * @brief  This Enum is used to identify the particular STATE  of Firmware object  
*/
typedef enum qapi_Net_Firmware_State_e 
{
  QAPI_NET_LWM2M_FIRWARE_STATE_IDLE_E = 0,
  QAPI_NET_LWM2M_FIRWARE_STATE_DOWNLOADING_E,
  QAPI_NET_LWM2M_FIRWARE_STATE_DOWNLOADED_E,
  QAPI_NET_LWM2M_FIRWARE_STATE_UPDATING_E,
} qapi_Net_Firmware_State_t;

/**
* @brief  This Enum is used to identify the type of ID set in lwm2m object info
*/
typedef enum qapi_Net_LWM2M_ID_e
{
  QAPI_NET_LWM2M_OBJECT_ID_E = 1,
  QAPI_NET_LWM2M_INSTANCE_ID_E = 2,
  QAPI_NET_LWM2M_RESOURCE_ID_E = 4,
  QAPI_NET_LWM2M_RESOURCE_INSTANCE_ID_E  = 8,
} qapi_Net_LWM2M_ID_t;

/**
* @brief  This Enum is used to identify the type of resource value
*/
typedef enum qapi_Net_LWM2M_Value_Type_e
{
  QAPI_NET_LWM2M_TYPE_UNDEFINED         = 0,
  QAPI_NET_LWM2M_TYPE_OBJECT            = 1,
  QAPI_NET_LWM2M_TYPE_OBJECT_INSTANCE   = 2,
  QAPI_NET_LWM2M_TYPE_MULTIPLE_RESOURCE = 3,
  QAPI_NET_LWM2M_TYPE_STRING_E = 4,
  QAPI_NET_LWM2M_TYPE_OPAQUE_E = 5,
  QAPI_NET_LWM2M_TYPE_INTEGER_E = 6,
  QAPI_NET_LWM2M_TYPE_FLOAT_E = 7,
  QAPI_NET_LWM2M_TYPE_BOOLEAN_E = 8,
  QAPI_NET_LWM2M_TYPE_OBJECT_LINK       = 9, 
} qapi_Net_LWM2M_Value_Type_t;

/**
* @brief  LWM2M write attribute types   
*/
typedef enum qapi_Net_LWM2M_Write_Attr_e
{
  QAPI_NET_LWM2M_MIN_PERIOD_E   = 1,
  QAPI_NET_LWM2M_MAX_PERIOD_E   = 2,
  QAPI_NET_LWM2M_GREATER_THAN_E = 4,
  QAPI_NET_LWM2M_LESS_THAN_E    = 8,
  QAPI_NET_LWM2M_STEP_E         = 16,
  QAPI_NET_LWM2M_DIM_E          = 32, 
} qapi_Net_LWM2M_Write_Attr_t;

/**
* @brief  LWM2M downlink message types 
*/
typedef enum qapi_Net_LWM2M_DL_Msg_e
{
  QAPI_NET_LWM2M_READ_REQ_E                  = 1,
  QAPI_NET_LWM2M_WRITE_REPLACE_REQ_E         = 2,
  QAPI_NET_LWM2M_WRITE_PARTIAL_UPDATE_REQ_E  = 3,
  QAPI_NET_LWM2M_WRITE_ATTR_REQ_E            = 4,
  QAPI_NET_LWM2M_DISCOVER_REQ_E              = 5,
  QAPI_NET_LWM2M_EXECUTE_REQ_E               = 6,
  QAPI_NET_LWM2M_DELETE_REQ_E                = 7,
  QAPI_NET_LWM2M_OBSERVE_REQ_E               = 8, 
  QAPI_NET_LWM2M_CANCEL_OBSERVE_REQ_E        = 9,
  QAPI_NET_LWM2M_ACK_MSG_E                   = 10,
  QAPI_NET_LWM2M_INTERNAL_CLIENT_IND_E       = 11,
  QAPI_NET_LWM2M_CREATE_REQ_E                = 12,
  QAPI_NET_LWM2M_DELETE_ALL_REQ_E            = 13,
} qapi_Net_LWM2M_DL_Msg_t;

/**
* @brief  LWM2M uplink message types 
*/
typedef enum qapi_Net_LWM2M_UL_Msg_e
{
  QAPI_NET_LWM2M_RESPONSE_MSG_E              = 1,   
  QAPI_NET_LWM2M_NOTIFY_MSG_E                = 2,
  QAPI_NET_LWM2M_CREATE_RESPONSE_MSG_E       = 3,
} qapi_Net_LWM2M_UL_Msg_t;

/**
* @brief  LWM2M event information 
*/
typedef enum qapi_Net_LWM2M_Event_e
{
  QAPI_NET_LWM2M_STATE_INITIAL_E                 = 0,
  QAPI_NET_LWM2M_BOOTSTRAP_REQUIRED_E            = 1,
  QAPI_NET_LWM2M_BOOTSTRAP_COMPLETED_E           = 2,
  QAPI_NET_LWM2M_BOOTSTRAP_FAILED_E              = 3,
  QAPI_NET_LWM2M_REGISTERTION_COMPELTED_E        = 4,
  QAPI_NET_LWM2M_REGISTRATION_FAILED_E           = 5,
  QAPI_NET_LWM2M_DEVICE_REBOOT_E                 = 6,
  QAPI_NET_LWM2M_DEVICE_FACTORY_RESET_E          = 7,
  QAPI_NET_LWM2M_DEVICE_REBOOTSTRAPPING_E        = 8,
  QAPI_NET_LWM2M_TX_MESSAGE_MAX_RETRY_FAILURE_E  = 9, 
  QAPI_NET_LWM2M_RX_MESSAGE_INTERNAL_FAILURE_E   = 10,
  QAPI_NET_LWM2M_SLEEP_E                         = 11, 
  QAPI_NET_LWM2M_WAKEUP_E                        = 12, 
} qapi_Net_LWM2M_Event_t;

/**
* @brief  LWM2M response status codes  
*/
typedef enum qapi_Net_LWM2M_Response_Code_e
{
  QAPI_NET_LWM2M_IGNORE_E                 = 0x00,
  QAPI_NET_LWM2M_201_CREATED_E            = 0x41,
  QAPI_NET_LWM2M_202_DELETED_E            = 0x42,
  QAPI_NET_LWM2M_204_CHANGED_E            = 0x44,
  QAPI_NET_LWM2M_205_CONTENT_E            = 0x45,
  QAPI_NET_LWM2M_400_BAD_REQUEST_E        = 0x80,
  QAPI_NET_LWM2M_401_UNAUTHORIZED_E       = 0x81,
  QAPI_NET_LWM2M_402_BAD_OPTION_E         = 0x82,
  QAPI_NET_LWM2M_403_FORBIDDEN_E          = 0x83,
  QAPI_NET_LWM2M_404_NOT_FOUND_E          = 0x84,
  QAPI_NET_LWM2M_405_METHOD_NOT_ALLOWED_E = 0x85,
  QAPI_NET_LWM2M_406_NOT_ACCEPTABLE_E     = 0x86,
  QAPI_NET_LWM2M_500_INTERNAL_SERVER_E    = 0xA0,
} qapi_Net_LWM2M_Response_Code_t;

/**
* @brief  LWM2M message content type 
*/
typedef enum qapi_Net_LWM2M_Content_Type_t
{
  QAPI_NET_LWM2M_TEXT_PLAIN                    = 0,
  QAPI_NET_LWM2M_TEXT_XML                      = 1,
  QAPI_NET_LWM2M_TEXT_CSV                      = 2,
  QAPI_NET_LWM2M_TEXT_HTML                     = 3,
  QAPI_NET_LWM2M_APPLICATION_LINK_FORMAT       = 40,
  QAPI_NET_LWM2M_APPLICATION_XML               = 41,
  QAPI_NET_LWM2M_APPLICATION_OCTET_STREAM      = 42,
  QAPI_NET_LWM2M_APPLICATION_RDF_XML           = 43,
  QAPI_NET_LWM2M_APPLICATION_SOAP_XML          = 44,
  QAPI_NET_LWM2M_APPLICATION_ATOM_XML          = 45,
  QAPI_NET_LWM2M_APPLICATION_XMPP_XML          = 46,
  QAPI_NET_LWM2M_APPLICATION_EXI               = 47,
  QAPI_NET_LWM2M_APPLICATION_FASTINFOSET       = 48,
  QAPI_NET_LWM2M_APPLICATION_SOAP_FASTINFOSET  = 49,
  QAPI_NET_LWM2M_APPLICATION_JSON              = 50,
  QAPI_NET_LWM2M_APPLICATION_X_OBIX_BINARY     = 51,
  QAPI_NET_LWM2M_M2M_TLV                       = 11542,
  QAPI_NET_LWM2M_M2M_JSON                      = 11543
}qapi_Net_LWM2M_Content_Type_t;

/**
* @brief  This Structure is used to indicate which Object/Instance/Resource ID that the 
*	        application is interested in monitoring or getting the value
*/
typedef struct qapi_Net_LWM2M_Id_Info_s
{
  struct qapi_Net_LWM2M_Id_Info_s *next;
  uint8_t id_set;
  uint16_t object_ID;
  uint8_t instance_ID;
  uint8_t resource_ID;  
} qapi_Net_LWM2M_Id_Info_t;

/**
* @brief  This Structure is used to indicate which Object/Instance/Resource that the application is 
* 	      interested in monitoring or getting the value
*/
typedef struct qapi_Net_LWM2M_Object_Info_s
{
  uint8_t no_object_info; 
  qapi_Net_LWM2M_Id_Info_t *id_info;  
} qapi_Net_LWM2M_Object_Info_t;

/**
* @brief  This Structure indicates the resource information that needs to be created
*/
typedef struct qapi_Net_LWM2M_Resource_Info_s
{
  qapi_Net_LWM2M_Value_Type_t type;
  uint8_t resource_ID;
  union
  {
    bool asBoolean;
    int64_t asInteger;
    double asFloat;
    struct
    {
      size_t length;
      uint8_t *buffer;
    } asBuffer;
  } value;
  struct qapi_Net_LWM2M_Resource_Info_s *next;
} qapi_Net_LWM2M_Resource_Info_t;

/**
* @brief  This Structure indicates the instance information that needs to be created 
*/
typedef struct qapi_Net_LWM2M_Instance_Info_s
{
  struct qapi_Net_LWM2M_Instance_Info_s *next;
  uint8_t instance_ID;
  uint8_t no_resources;
  qapi_Net_LWM2M_Resource_Info_t * resource_info;
} qapi_Net_LWM2M_Instance_Info_t;

/**
* @brief  This Structure is populated by the application and provided to LWM2M client
*         when the application wants to create an instance of the 
*         LWM2M Object, perform set and get operations. 
*/
typedef struct qapi_Net_LWM2M_Data_s
{
  struct qapi_Net_LWM2M_Data_s *next;
  uint16_t object_ID;
  uint8_t no_instances;
  qapi_Net_LWM2M_Instance_Info_t *instance_info; 
} qapi_Net_LWM2M_Data_t;

/**
* @brief  LWM2M object/uri related information. 
*/
typedef struct qapi_Net_LWM2M_Obj_Info_s
{
  qapi_Net_LWM2M_ID_t    obj_mask;         /* Bit-map indicating valid object fields */
  uint16_t               obj_id;           /* Object ID */
  uint16_t               obj_inst_id;      /* Object Instance ID */
  uint16_t               res_id;           /* Resource ID */
  uint16_t               res_inst_id;      /* Resource Instance ID */
} qapi_Net_LWM2M_Obj_Info_t;

/**
* @brief  LWM2M write attribute information.
*/
typedef struct qapi_Net_LWM2M_Attributes_s
{
  qapi_Net_LWM2M_Obj_Info_t     obj_info;       /* LWM2M object information associated with write attributes */ 
  qapi_Net_LWM2M_Write_Attr_t   set_attr_mask;  /* Bit-map indicating valid attribute fields to set */
  qapi_Net_LWM2M_Write_Attr_t   clr_attr_mask;  /* Bit-map indicating attribute fields to clear */
  uint8_t                       dim;            /* Dimension */
  uint32_t                      minPeriod;      /* Minimum period */
  uint32_t                      maxPeriod;      /* Maximum period */
  double                        greaterThan;    /* Greater than */
  double                        lessThan;       /* Less than */
  uint8_t                       step_valid;     /* Step validity */
  double                        step;           /* Step */
  struct qapi_Net_LWM2M_Attributes_s *next;
} qapi_Net_LWM2M_Attributes_t;

/**
* @brief  LWM2M resource information (in flat format) to encode/decode data payload.
*/
typedef struct qapi_Net_LWM2M_Flat_Data_s
{
  qapi_Net_LWM2M_Value_Type_t type;
  uint16_t  id;
  union
  {
    bool     asBoolean;
    int64_t  asInteger;
    double   asFloat;
    struct
    {
      size_t    length;
      uint8_t  *buffer;
    } asBuffer;
    struct
    {
      size_t   count;
      struct qapi_Net_LWM2M_Flat_Data_s  *array;
    } asChildren;
  } value;
} qapi_Net_LWM2M_Flat_Data_t;

/**
* @brief  LWM2M server request message data and internal LWM2M client state information.
*/
typedef struct qapi_Net_LWM2M_Server_Data_s
{
  qapi_Net_LWM2M_DL_Msg_t         msg_type;        /* DL Message type (requests, acknowledgements or internal) */
  qapi_Net_LWM2M_Obj_Info_t       obj_info;        /* Object information */
  uint8_t                         msg_id_len;      /* Message ID length */ 
  /* Message ID is transparent to the application, but is passed to the application for every message received from the server. 
   * The expectation is the application stores the message ID associated with the message and passes to the LWM2M client when 
   * any response or notification needs to be sent to the server. After the transaction pertaining to the message is complete 
   * the message ID can be discarded from the application. 
   */ 
  uint8_t                         msg_id[QAPI_MAX_LWM2M_MSG_ID_LENGTH];   
  /* When a notification is sent using qapi_Net_LWM2M_Send_Message, the notification ID associated with the message is returned 
   * to the caller. It is the caller's responsibility to maintain the notification ID to observation mapping. Later, when the 
   * network does a cancel observation for a particular notification using RESET that would be indicated using the notification 
   * ID to the caller. Using this notification ID, the caller can cancel the observation. If the cancel observation was not using
   * RESET then the obj_info should have the information based on which observation to cancel. */
  uint16_t                        notification_id; 
  qapi_Net_LWM2M_Content_Type_t   content_type;    /* Encoded data payload content type */
  uint32_t                        payload_len;     /* Encoded data payload length */
  uint8_t                        *payload;         /* Encoded data payload */
  qapi_Net_LWM2M_Attributes_t    *lwm2m_attr;      /* Write attributes */
  qapi_Net_LWM2M_Event_t          event;           /* Internal events */ 
} qapi_Net_LWM2M_Server_Data_t;

/**
* @brief  LWM2M application response message data and notification related information. 
*/
typedef struct qapi_Net_LWM2M_App_Ex_Obj_Data_s
{
  qapi_Net_LWM2M_UL_Msg_t         msg_type;        /* UL message type (responses or notifications) */
  qapi_Net_LWM2M_Obj_Info_t       obj_info;        /* Object information */
  qapi_Net_LWM2M_Response_Code_t  status_code;     /* Application message status (applicable for responses only) */
  uint8_t                         conf_msg;        /* Confirmable (ACK) or Non-Confirmable application response/notifications */  
  uint8_t                         msg_id_len;      /* Message ID length */ 
  uint8_t                         msg_id[QAPI_MAX_LWM2M_MSG_ID_LENGTH]; /* Message ID */
  /* When a notification is sent using qapi_Net_LWM2M_Send_Message, the notification ID associated with the message is returned 
   * to the caller. It is the caller's responsibility to maintain the notification ID to observation mapping. Later, when the 
   * network does a cancel observation for a particular notification using RESET that would be indicated using the notification 
   * ID to the caller. Using this notification ID, the caller can cancel the observation. */
  uint16_t                        notification_id; 
  qapi_Net_LWM2M_Content_Type_t   content_type;    /* Encoded data payload content type */
  uint32_t                        payload_len;     /* Encoded data payload length */
  uint8_t                        *payload;         /* Encoded data payload */
} qapi_Net_LWM2M_App_Ex_Obj_Data_t;


/* LWM2M client DAM QAPIs */
#ifdef  QAPI_TXM_MODULE    
#define qapi_Net_LWM2M_Register_App(a)                     ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_REGISTER_APP, (ULONG) a, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Register_App_Extended(a,b,c)        ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_REGISTER_APP_EXTENDED, (ULONG) a, (ULONG) b, (ULONG) c, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_DeRegister_App(a)                   ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_DEREGISTER_APP, (ULONG) a, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Observe(a,b,c)                      ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_OBSERVE, (ULONG) a, (ULONG) b, (ULONG) c, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Cancel_Observe(a,b)                 ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_CANCEL_OBSERVE, (ULONG) a, (ULONG) b, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Create_Object_Instance(a,b)         ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_CREATE_OBJECT_INSTANCE, (ULONG) a, (ULONG) b, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Delete_Object_Instance(a,b)         ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_DELETE_OBJECT_INSTANCE, (ULONG) a, (ULONG) b, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Get(a,b,c)                          ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_GET, (ULONG) a, (ULONG) b, (ULONG) c, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Set(a,b)                            ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_SET, (ULONG) a, (ULONG) b, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Send_Message(a,b)                   ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LW2M_SEND_MESSAGE, (ULONG) a, (ULONG) b, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Encode_App_Payload(a,b,c,d,e,f,g)   ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_ENCODE_APP_PAYLOAD, (ULONG) a, (ULONG) b, (ULONG) c, (ULONG) d, (ULONG) e, (ULONG) f, (ULONG) g, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Decode_App_Payload(a,b,c,d,e,f)     ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_DECODE_APP_PAYLOAD, (ULONG) a, (ULONG) b, (ULONG) c, (ULONG) d, (ULONG) e, (ULONG) f, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))
#define qapi_Net_LWM2M_Wakeup(a,b,c)                         ((qapi_Status_t) (_txm_module_system_call12)(TXM_QAPI_LWM2M_WAKEUP, (ULONG) a, (ULONG) b, (ULONG) c, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0, (ULONG) 0))

#else
qapi_Status_t qapi_LWM2Mc_Handler(UINT id, UINT a1, UINT a2, UINT a3, UINT a4, UINT a5, UINT a6, UINT a7, UINT a8, UINT a9, UINT a10, UINT a11, UINT a12);


/**
* @brief  This is the callback registered from the application, which is used by the 
*         LWM2M client to indicate the resource value change to the application 
*/
typedef qapi_Status_t (*qapi_Net_LWM2M_App_CB_t)
(
  qapi_Net_LWM2M_App_Handler_t handle, 
  qapi_Net_LWM2M_Data_t *lwm2m_data
);

/**
* @brief  This is the callback registered from the application, which is used by the LWM2M client to indicate 
*         any extended object specific messages from the server to the appropriate application. Each server 
*         message request is associated with a message ID and passed to the caller as part of the LWM2M server. 
*         The application needs to maintain the message ID to message mapping and need to use the message ID for 
*         any futher transactions that involves responses or notification events pertaining to the message.
*/
typedef qapi_Status_t (*qapi_Net_LWM2M_App_Extended_CB_t)
(
  qapi_Net_LWM2M_App_Handler_t handle, 
  qapi_Net_LWM2M_Server_Data_t *lwm2m_srv_data,
  void *user_data
);

/**
* @brief  This API is used to Register an application with LWM2M client, Application gets a handle  
*         on successful registration with LWM2M Client
* @param  handle (IN/OUT) Indicates the handle that will be provided to application on successful registration 
* @return qapi_Status_t  QAPI_OK on success 
*                        QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Register_App 
(
  qapi_Net_LWM2M_App_Handler_t *handle
);

/**
* @brief  This API is used to Register an application with LWM2M client along with a callback handle. Application 
*         gets a handle on successful registration with LWM2M Client and need to use this handle for any subsequent
*         calls to the LWM2M client in the APIs.
* @param  handle (IN/OUT) Indicates the handle that will be provided to application on successful registration 
* @param  user_data (IN) Transparent user data payload (to be returned in the user callback)
* @param  app_cb (IN) User client callback handle to forward data to the application 
* @return qapi_Status_t  QAPI_OK on success 
*                        QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Register_App_Extended
(
  qapi_Net_LWM2M_App_Handler_t *handle, 
  void *user_data, 
  qapi_Net_LWM2M_App_Extended_CB_t user_cb_fn
);

/**
* @brief  This API is used to De-Register an application. Any object instances associated with the handle are 
*         automatically cleaned-up as a result of deregister. 
* @param  handle (IN) Indicates the handle that is provided to application on successful registration 
* @return qapi_Status_t  QAPI_OK on success 
*                        QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_DeRegister_App
(
  qapi_Net_LWM2M_App_Handler_t handle
);

/**
* @brief  This API is used from the application to indicate the LWM2M Client the object/instance/resource that 
*         the application is interested in observing  
* @param  handle (IN) Handler received after successful registration of the application
* @param  observe_cb_fn (IN) Application callback to be invoked on value change
* @param  observe_info (IN) Object/Instance/resource information that the application is interested in monitoring on LWM2M Client
* @return qapi_Status_t QAPI_OK on success
*                       QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Observe 
(
  qapi_Net_LWM2M_App_Handler_t handle,  
  qapi_Net_LWM2M_App_CB_t observe_cb_fn, 
  qapi_Net_LWM2M_Object_Info_t *observe_info
);

/**
* @brief  This API is used from the application to cancel the observation 
* @param  handle (IN) Handler received after successful registration of the application
* @param  observe_info (IN) Object/Instance/resource information that the application wants to cancel the observation
* @return qapi_Status_t QAPI_OK on success
*                       QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Cancel_Observe 
(
  qapi_Net_LWM2M_App_Handler_t handle,  
  qapi_Net_LWM2M_Object_Info_t *observe_info
);

/**
* @brief  This API is used to create an LWM2M object instance from the application 
* @param  handle (IN) Handler received after successful registration of the application
* @param  lwm2m_data (IN) lwm2m object instance and its resource information 
* @return qapi_Status_t QAPI_OK on success
*                       QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Create_Object_Instance 
(
  qapi_Net_LWM2M_App_Handler_t handle,   
  qapi_Net_LWM2M_Data_t *lwm2m_data
);

/**
* @brief  This API is used to delete an LWM2M object instance from the application 
* @param  handle (IN) Handler received after successful registration of the application
* @param  lwm2m_data (IN) lwm2m object instance and its resource information 
* @return qapi_Status_t QAPI_OK on success
*                       QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Delete_Object_Instance 
(
  qapi_Net_LWM2M_App_Handler_t handle,  
  qapi_Net_LWM2M_Object_Info_t *instance_info
);

/**
* @brief  This API is used to get the value of LWM2M Object/Instance/Resource from application
* @param  handle (IN) Handler received after successful registration of the application
* @param  lwm2m_info_req (IN) Object/Instance/Resource information requested from application
* @param  lwm2m_data (OUT) Value of LWM2M Object/Instance/Resource information 
* @return qapi_Status_t  QAPI_OK on success
*                        QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Get
(
  qapi_Net_LWM2M_App_Handler_t handle, 
  qapi_Net_LWM2M_Object_Info_t *lwm2m_info_req, 
  qapi_Net_LWM2M_Data_t **lwm2m_data
);

/**
* @brief  This API is used to Set the value of LWM2M Resources
* @param  handle (IN) Handler received after successful registration of the application
* @param  lwm2m_data (IN) Value of the LWM2M resource to be set
* @return qapi_Status_t  QAPI_OK on success
*                        QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Set 
(
  qapi_Net_LWM2M_App_Handler_t handle, 
  qapi_Net_LWM2M_Data_t *lwm2m_data
);

/**
* @brief  This API is used to send application data which can be responses or notification events to the server  
* @param  handle (IN) Handler received after successful registration of the application
* @param  lwm2m_app_data (IN) Value of the LWM2M extended/custom object information to be set
* @return qapi_Status_t  QAPI_OK on success
*                        QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Send_Message 
(
  qapi_Net_LWM2M_App_Handler_t handle,
  qapi_Net_LWM2M_App_Ex_Obj_Data_t *lwm2m_app_data
);

/**
* @brief  This API is used to encode application responses or notification data before sending to the server.
* @param  obj_info (IN) Object/URI information  
* @param  in_dec_data (IN) Input data that needs to be encoded 
* @param  in_dec_data_size (IN) Input data size (in buffers)
* @param  write_attr (IN) Write attribute information 
* @param  enc_content_type (IN) Encoding format of the data 
* @param  out_enc_data (OUT) Output data buffer in encoded format 
* @param  out_enc_data_len (OUT) Output encoded data buffer length 
* @return qapi_Status_t  QAPI_OK on success
*                        QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Encode_App_Payload 
(
  qapi_Net_LWM2M_Obj_Info_t *obj_info,
  qapi_Net_LWM2M_Flat_Data_t *in_dec_data,
  size_t in_dec_data_size,
  qapi_Net_LWM2M_Attributes_t *write_attr,  
  qapi_Net_LWM2M_Content_Type_t enc_content_type,
  uint8_t **out_enc_data,
  uint32_t *out_enc_data_len
);

/**
* @brief  This API is used to decode application request data received from the server.  
* @param  obj_info (IN) Object/URI information
* @param  in_enc_data (IN) Input data that needs to be decoded 
* @param  in_enc_data_len (IN) Input data length 
* @param  enc_content_type (IN) Encoding format of the input data 
* @param  out_enc_data (OUT) Output data buffer in decoded format 
* @param  out_enc_data_size (OUT) Output decoded data size (in buffers)
* @return qapi_Status_t  QAPI_OK on success
*                        QAPI_ERROR on failure
*/
extern qapi_Status_t qapi_Net_LWM2M_Decode_App_Payload 
(
  qapi_Net_LWM2M_Obj_Info_t *obj_info,
  uint8_t *in_enc_data,
  uint32_t in_enc_data_len,
  qapi_Net_LWM2M_Content_Type_t enc_content_type,
  qapi_Net_LWM2M_Flat_Data_t **out_dec_data,
  size_t *out_dec_data_size
);

/**
* @brief  This API is used to wake-up the LWM2M client module to send notifications to the server.
* @param  handle (IN) Handler received after successful registration of the application 
* @param  msg_id (IN) Message ID information associated with the request 
* @param  msg_id_len (IN) Message ID information length 
* @return qapi_Status_t  QAPI_OK on success
*                        QAPI_ERROR on failure
*/
qapi_Status_t qapi_Net_LWM2M_Wakeup 
(
  qapi_Net_LWM2M_App_Handler_t handle,
  uint8_t *msg_id,
  uint8_t  msg_id_len
);

#endif
#endif
