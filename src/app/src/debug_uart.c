/*!
  @file
  debug_uart.c
  @brief
  QCLI implementation for DEBUG.
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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "qcli.h"
#include "qapi_uart.h"
#include "pal.h"
#include "qapi_timer.h"

#define DEBUG_MAX_SEND_BUFFER_SIZE 256

static qapi_UART_Handle_t handle;

void Uart_Debug_Initialize(void)
{
   qapi_UART_Open_Config_t open_properties;
   open_properties.parity_Mode = QAPI_UART_NO_PARITY_E;
   open_properties.num_Stop_Bits= QAPI_UART_1_0_STOP_BITS_E;
   open_properties.baud_Rate   = 115200;
   open_properties.bits_Per_Char= QAPI_UART_8_BITS_PER_CHAR_E;
   open_properties.rx_CB_ISR = NULL;
   open_properties.tx_CB_ISR = NULL;
   open_properties.enable_Flow_Ctrl = false;
   open_properties.enable_Loopback= false;
   qapi_UART_Open(&handle, QAPI_UART_PORT_001_E, &open_properties);
}

static void UART_Write(uint32_t Length, const char *Buffer)
{
   if((Length) && (Buffer))
   {
       qapi_UART_Transmit(handle,Buffer, Length, (void*)Buffer);
   }
}

void Debug_Printf(char* format, ...)
{
    static char str_tmp[DEBUG_MAX_SEND_BUFFER_SIZE] = {0};
    int Length=0;
    va_list vArgList; 
    va_start (vArgList, format); 
    Length=_vsnprintf(str_tmp, DEBUG_MAX_SEND_BUFFER_SIZE, format, vArgList);
    va_end(vArgList);
    UART_Write(Length,str_tmp);
    qapi_Timer_Sleep(2,QAPI_TIMER_UNIT_MSEC,true);
}








