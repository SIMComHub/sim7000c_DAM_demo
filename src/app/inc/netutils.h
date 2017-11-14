/*!
  @file
  netutils.h

  @brief
  Networking stack related suporting API's implementation.
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



#ifndef _netutils_h_
#define _netutils_h_

#include <stdint.h>
#include "txm_module.h"

#undef htons
#undef ntohs
#undef htonl
#undef ntohl
#define htons(s)    ((((s) >> 8) & 0xff) | (((s) << 8) & 0xff00))
#define ntohs(s)    htons(s)
#define htonl(l)    (((((l) >> 24) & 0x000000ff)) | \
                    ((((l) >>  8) & 0x0000ff00)) | \
                    (((l) & 0x0000ff00) <<  8) | \
                    (((l) & 0x000000ff) << 24))
#define ntohl(l)    htonl(l)

#ifndef min
#define  min(a,b)    (((a) <= (b)) ? (a) : (b))
#endif

#define atoi(s)  ds_atoi(s)
static int ds_atoi(char *str)
{
    int res = 0, i;

    for (i = 0; str[i] != '\0' && str[i] != '.'; ++i)
    {
        res = res*10 + (str[i] - '0');
    }

    return res;
}

/* queue element: cast to right type */
struct q_elt
{     
    struct q_elt *qe_next;  /* pointer to next elt */
};
typedef struct q_elt  *qp;  /* q pointer */

/* queue header */
typedef struct queue   
{
    struct q_elt *q_head;   /* first element in queue */
    struct q_elt *q_tail;   /* last element in queue */
    int  q_len;             /* number of elements in queue */
} QUEUE_T;

typedef struct
{
    uint32_t    seconds;        /* number of seconds */
    uint32_t    milliseconds;   /* number of milliseconds */
} time_struct_t;

void enqueue(QUEUE_T *q, void *item);
void * dequeue(QUEUE_T *q);

/* Return milliseconds */
uint32_t app_get_time(time_struct_t *time);

void app_msec_delay(uint32_t ms);



#endif /* _netutils_h_ */
