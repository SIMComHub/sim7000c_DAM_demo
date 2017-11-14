/*!
   @file
   bench.h
 
   @brief
   Header for performing all the data transefer related operations.
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
 
/*===========================================================================
 
                         EDIT HISTORY FOR MODULE
 
 This section contains comments describing changes made to the module.
 Notice that changes are listed in reverse chronological order.
 
 $Header: 
 
 when       who     what, where, why
 --------   ---     ----------------------------------------------------------
 
===========================================================================*/


#ifndef _BENCH_H_

#define _BENCH_H_

#include "qcli_api.h"
#include "qapi/qapi_ssl.h"
#include "qapi/qapi_socket.h"
#include "qapi/qapi_ns_utils.h"
#include "qapi/qapi_netbuf.h"
#include "netutils.h"       /* time_struct_t */
#include "qapi/qapi_netservices.h"


#undef A_OK
#undef A_ERROR
#undef HOST_TO_LE_LONG

#define A_OK                    QAPI_OK
#define A_ERROR                 -1
#define HOST_TO_LE_LONG(n)      (n)

#define BENCH_TEST_COMPLETED    "**** IOT Throughput Test Completed ****\r\n"
#define V6_PACKET_SIZE_MAX_TX   (1230)
#define CFG_PACKET_SIZE_MAX_TX  (65535)
#define CFG_PACKET_SIZE_MAX_RX  (1556)
#define END_OF_TEST_CODE        (0xAABBCCDD) 
#define MAX_SSL_INST            2
#define SSL_SERVER_INST         0
#define SSL_CLIENT_INST         1
#define CLIENT_WAIT_TIME        30000

#define OFFSETOF(type, field)   ((size_t)(&((type*)0)->field))

typedef qapi_Net_Buf_t * PACKET;

typedef struct udp_zc_rx_info
{
    struct udp_zc_rx_info *next;
    PACKET pkt;
    struct sockaddr from;
} UDP_ZC_RX_INFO;

/*structure to manage stats received from peer during UDP traffic test*/
typedef struct stat_packet
{
    uint32_t bytes;  
    uint32_t kbytes;
    uint32_t msec;
    uint32_t numPackets;
} stat_packet_t;

/**************************************************************************/ /*!
 * TX/RX Test parameters
 ******************************************************************************/
typedef struct transmit_params
{
    uint32_t ip_address;  
    uint8_t v6addr[16];
    int32_t scope_id;
    uint16_t port;
    uint16_t test_mode;
    int packet_size;
    int tx_time;
    int packet_number;
    int interval;
} TX_PARAMS;

typedef struct receive_params
{     
    uint16_t port;
    uint16_t local_if;
    uint32_t local_address;
    uint8_t local_v6addr[16];
    uint8_t mcEnabled;
    uint32_t mcIpaddr;
    uint8_t mcIpv6addr[16];
} RX_PARAMS;

/************************************************************************
*    Benchmark server control structure.
*************************************************************************/
typedef struct throughput_cxt
{
    int32_t sock_local;             /* Listening socket.*/
    int32_t sock_peer;              /* Foreign socket.*/    
    int32_t rxcode;                 /* event code from rx_upcall */
    char* buffer;    
    time_struct_t first_time;       /* Test start time */
    time_struct_t last_time;
    unsigned long long bytes;       /* Number of bytes received in current test */
    unsigned long long kbytes;      /* Number of kilo bytes received in current test */
    unsigned long long last_bytes;  /* Number of bytes received in the previous test */
    unsigned long long last_kbytes;
    unsigned long long sent_bytes;
    uint32_t    pkts_recvd;
    uint32_t    pkts_expctd;
    uint32_t    last_interval;
    uint32_t    last_throughput;
    union params_u
    {
        TX_PARAMS    tx_params;
        RX_PARAMS    rx_params;
    } params;
    uint8_t      test_type; 
} THROUGHPUT_CXT;

typedef struct end_of_test {
    int code;
    int packet_count;
} EOT_PACKET;

enum test_type
{
    UDP_TX,   //UDP Transmit (Uplink Test)
    UDP_RX,   //UDP Receive (Downlink Test)
    TCP_TX,   //TCP Transmit (Uplink Test)
    TCP_RX,   //TCP Receive (Downlink Test)
    SSL_TX,   //SSL Transmit (Uplink Test)
    SSL_RX    //SSL Receive (Downlink Test)
};

enum Test_Mode
{
	TIME_TEST,
	PACKET_TEST
};

QCLI_Command_Status_t benchrx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t benchtx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t benchquit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t uapsdtest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

#if INCLUDE_SSLDEMO
#define CERT_HEADER_LEN         sizeof(CERT_HEADER_T)
#define SSL_CERT_BUF_LEN        (1200)
typedef struct
{
    uint8_t id[4];
    uint32_t length;
    uint8_t data[1];
    //uint8_t data[0];
} CERT_HEADER_T;

#if INCLUDE_SSLDEMO
typedef struct ssl_inst
{
    qapi_Net_SSL_Obj_Hdl_t sslCtx;
    qapi_Net_SSL_Con_Hdl_t ssl;
    qapi_Net_SSL_Config_t   config;
    uint8_t      config_set;
    qapi_Net_SSL_Role_t role;
} SSL_INST;


extern SSL_INST ssl_inst[MAX_SSL_INST];
#endif
extern uint8_t const *ssl_default_cert;
extern const uint16_t ssl_default_cert_len;
extern uint8_t const *ssl_default_calist;
extern const uint16_t ssl_default_calist_len;

QCLI_Command_Status_t ssl_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_start(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
void sslconfig_help(const char *str);
QCLI_Command_Status_t ssl_parse_config_parameters(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List, qapi_Net_SSL_Config_t *cfg, qbool_t server);
QCLI_Command_Status_t ssl_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_add_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
#endif /* INCLUDE_SSLDEMO */ 

extern uint8_t *cert_data_buf;
extern uint16_t cert_data_buf_len;
QCLI_Command_Status_t cert_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t store_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t covert_store_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t delete_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t list_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t get_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

//udp test pattern
typedef struct udp_pattern_of_test{
  unsigned int code;
  unsigned short seq;
} UDP_PATTERN_PACKET;

#define CODE_UDP    ('U'|('D'<<8)|'P'<<16)

#define IEEE80211_SEQ_SEQ_MASK          0xfff0
#define IEEE80211_SEQ_SEQ_SHIFT         4

#define IEEE80211_SN_MASK       ((IEEE80211_SEQ_SEQ_MASK) >> IEEE80211_SEQ_SEQ_SHIFT)
#define IEEE80211_MAX_SN        IEEE80211_SN_MASK
#define IEEE80211_SN_MODULO     (IEEE80211_MAX_SN + 1)

#define IEEE80211_SN_LESS(sn1, sn2) \
    ((((sn1) - (sn2)) & IEEE80211_SN_MASK) > (IEEE80211_SN_MODULO >> 1))

#define IEEE80211_SN_ADD(sn1, sn2) \
    (((sn1) + (sn2)) & IEEE80211_SN_MASK)

#define IEEE80211_SN_INC(sn)    \
    IEEE80211_SN_ADD((sn), 1)

#define IEEE80211_SN_SUB(sn1, sn2)  \
    (((sn1) - (sn2)) & IEEE80211_SN_MASK)

typedef struct stat_udp_pattern
{
    uint32_t pkts_plan;
    uint32_t pkts_recvd;
    uint32_t pkts_seq_recvd;
    uint32_t pkts_seq_less;
    unsigned short seq_last;
    unsigned short ratio_of_drop;
    unsigned short ratio_of_seq_less;
    char stat_valid;
}stat_udp_pattern_t;

#endif /* _BENCH_H_ */
