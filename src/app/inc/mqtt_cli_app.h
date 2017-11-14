/*!
  @file
  mqtt_cli_app.h

  @brief
  Implementation of MQTT protocol Related functionality.
*/

/*===========================================================================

  Copyright (c) 2017 Qualcomm Technologies, Inc. All Rights Reserved

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


/**
   @brief This function registers the networking demo commands with QCLI.
*/
#define MAX_SUB_LEN 1024
#define IN_MUTEX void *
#define MAX_QOS 2
#define MAX_TOPIC_LEN 64
#define MAX_MSG_LEN 256
#define MAX_MSG 64
#define MAX_UN 32
#define MAX_PWD MAX_UN
#define MAX_IP_ADDR MAX_TOPIC_LEN
#define SOCKTYPE long /* preferred generic socket type */
#define MEMSET(d, c, l) memset((void *)(d), (int32_t)(c), (int32_t)(l))
#define CLI_MAX_CLIENT_ID_LEN 23
#define HOST_SET flag |= 1
#define MESG_SET flag |= (1 << 1)
#define TOPIC_SET flag |= (1 << 2)
#define HOST_CHK (flag & 1)
#define MESG_CHK ((flag >> 1) & 1)
#define TOPIC_CHK ((flag >> 2) & 1)
#ifdef IP_V6
#define ADDRBUF_LEN IP6_ADDRBUF_LEN
#else
#define ADDRBUF_LEN 40
#endif

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */
#define MQTT_PRINTF(...) QCLI_Printf(qcli_net_handle, __VA_ARGS__)
/**
   This enumeration represents the valid error codes that can be returned
   by the command functions.  They represent a success or an error.
*/
enum 
{
   MQTT_CLI_STATUS_ERROR_E = -1,
   MQTT_CLI_STATUS_SUCCESS_E = 1
};

