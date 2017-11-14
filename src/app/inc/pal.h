/*!
  @file
  pal.h

  @brief
  Console functionality for QCLI
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



#ifndef __PAL_H__
#define __PAL_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include "qapi/qapi_types.h"
#include "txm_module.h"
#include "qapi_uart.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/


#define PAL_RECIEVE_BUFFER_SIZE                         (128)

#define PAL_MAX_SEND_BUFFER_SIZE                        (256)

#define PAL_RECEIVE_EVENT_MASK                          (0x00000001)
#define START_THREAD_EVENT_MASK                          (0x00000001)


#define PAL_ENTER_CRITICAL()                            do { __asm("cpsid i"); } while(0)
#define PAL_EXIT_CRITICAL()                             do { __asm("cpsie i"); } while(0)


/**
   This definition indicates the character that is inpretted as an end of 
   line for inputs from the console.
*/
#define PAL_INPUT_END_OF_LINE_CHARACTER                     '\r'

/**
   This definition represents the string that is used as the end of line 
   for outputs to the console.
*/
#define PAL_OUTPUT_END_OF_LINE_STRING                       "\r\n"


/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

typedef struct PAL_Context_s
{
   uint8_t             Rx_Buffer[PAL_RECIEVE_BUFFER_SIZE];
   uint8_t             Rx_In_Index;
   uint8_t             Rx_Out_Index;
   volatile uint32_t   Rx_Buffers_Free;
   TX_EVENT_FLAGS_GROUP Rx_Event;
   qapi_UART_Handle_t   uart_handle;
} PAL_Context_t;


/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

extern TX_EVENT_FLAGS_GROUP         Start_Thread_Event;

#define CLI_BYTE_POOL_SIZE 30720 
/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/

/**
   @brief This function is used to write a buffer to the console. Note
          that when this function returns, all data from the buffer will
          be written to the console or buffered locally. 

   @param Length is the length of the data to be written.
   @param Buffer is a pointer to the buffer to be written to the console.
*/
void PAL_Console_Write(uint32_t Length, const uint8_t *Buffer);

/**
   @brief This function is indicates to the PAL layer that the application 
          should exit. For embedded applications this is typically a reset.
*/
void PAL_Exit(void);

#endif
