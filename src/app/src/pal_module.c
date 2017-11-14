/*!
  @file
  pal_module.c

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



/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "qapi/qapi_types.h"

#include "pal.h"
#include "qcli.h"
#include "qcli_api.h"
#include "net_demo.h"
#include "dss_netapp.h"
#include "psm_demo.h"

#include "qapi/qapi.h"
#include "qapi/qapi_status.h"
#include "qapi_uart.h"

#include "qapi_fs.h"
#include "qapi_timer.h"

#include "qapi_diag.h"
#include "msgcfg.h"
#include "msg_mask.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/


/* The following is a simple macro to facilitate printing strings directly
   to the console. As it uses the sizeof operator on the size of the string
   provided, it is intended to be used with string literals and will not
   work correctly with pointers.
*/
#define PAL_CONSOLE_WRITE_STRING_LITERAL(__String__)    do { PAL_Console_Write(sizeof(__String__) - 1, (__String__)); } while(0)

#define QCLI_STACK_SIZE 16384


/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/



/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

static PAL_Context_t PAL_Context_D;

TX_EVENT_FLAGS_GROUP Start_Thread_Event;

UCHAR			   app_stack[QCLI_STACK_SIZE];
TX_THREAD		   Thread_Handle;

UCHAR free_memory_qcli[CLI_BYTE_POOL_SIZE];

TX_BYTE_POOL byte_pool_qcli;

char send_buf[PAL_MAX_SEND_BUFFER_SIZE];

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

static void Initialize_Samples(void);
static void cli_rx_cb(uint32_t num_bytes, void *cb_data);
void QCLI_Thread(void *Param);
static qbool_t PAL_Initialize(void);
extern void loc_qcli_iface_init(void);
extern void Initialize_PSM_Demo(void);
/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/

/**
   @brief This function is responsible for initializing the sample
          applications.
*/
static void Initialize_Samples(void)
{
   //Initialize_Net_Demo();
   //Initialize_Dss_Netapp_Demo();
   Initialize_PSM_Demo();
}

/**
   @brief This function handles receive callbacks from the UART.

   @param Num_Bytes[in] is the number of bytes received.
   @param CB_Data[in]   is the application defined callback data.  In this case
                        it is the index of the buffer received on.
*/
static void cli_rx_cb(uint32_t num_bytes, void *cb_data)
{

  int Length = num_bytes;

  if(PAL_Context_D.Rx_Buffers_Free != 0)
  {
      /* See how much data can be read.                                    */
      if(Length > PAL_Context_D.Rx_Buffers_Free)
      {
         Length = PAL_Context_D.Rx_Buffers_Free;
      }


      PAL_Context_D.Rx_In_Index += Length;
      if(PAL_Context_D.Rx_In_Index == PAL_RECIEVE_BUFFER_SIZE)
      {
         PAL_Context_D.Rx_In_Index = 0;
      }
//      PAL_ENTER_CRITICAL();
      PAL_Context_D.Rx_Buffers_Free -= Length;
//      PAL_EXIT_CRITICAL();

      if(Length > 0)
      {
         /* Signal the event that data was received.                       */
          tx_event_flags_set(&(PAL_Context_D.Rx_Event), PAL_RECEIVE_EVENT_MASK, TX_OR);
      }

  }

}

/**
   @brief This function represents the main thread of execution.

   It will finish initialization of the sample and then function as a
   receive thread for the console.
*/
void QCLI_Thread(void *Param)
{

   ULONG set_signal = 0;
   uint8_t Length;

   PAL_CONSOLE_WRITE_STRING_LITERAL("QCLI DAM Thread Started");
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);

   /* Initiailze the CLI. */
   if(QCLI_Initialize())
   {
      /* Initialize the samples. */
      Initialize_Samples();
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      PAL_CONSOLE_WRITE_STRING_LITERAL("--------------------------------");
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      PAL_CONSOLE_WRITE_STRING_LITERAL("----------- QCLI DAM -----------");
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      PAL_CONSOLE_WRITE_STRING_LITERAL("--------------------------------");
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);

      /* Display the initialize command list. */
      QCLI_Display_Command_List();

      /* Create a receive event. */
      tx_event_flags_create(&(PAL_Context_D.Rx_Event), "qcli_sig");

      /* Loop waiting for received data. */
      while(true)
      {
         /* Wait for data to be received. */
         while((Length = PAL_RECIEVE_BUFFER_SIZE - PAL_Context_D.Rx_Buffers_Free) == 0)
         {
             tx_event_flags_get(&(PAL_Context_D.Rx_Event), PAL_RECEIVE_EVENT_MASK, TX_OR, &set_signal, TX_WAIT_FOREVER );
             tx_event_flags_set( &(PAL_Context_D.Rx_Event), ~PAL_RECEIVE_EVENT_MASK, TX_AND );
         }

         /* Make sure the length doesn't wrap the buffer.                  */
         if(Length > PAL_RECIEVE_BUFFER_SIZE - PAL_Context_D.Rx_Out_Index)
         {
            Length = PAL_RECIEVE_BUFFER_SIZE - PAL_Context_D.Rx_Out_Index;
         }

         /* Send the next buffer's data to QCLI for processing. */
         QCLI_Process_Input_Data(Length, &(PAL_Context_D.Rx_Buffer[0]));

         qapi_UART_Receive(PAL_Context_D.uart_handle, (char *)&(PAL_Context_D.Rx_Buffer[0]), PAL_RECIEVE_BUFFER_SIZE, (void*)1);

         /* Adjust the indexes for the received data. */
         PAL_Context_D.Rx_Out_Index += Length;
         if(PAL_Context_D.Rx_Out_Index == PAL_RECIEVE_BUFFER_SIZE)
         {
            PAL_Context_D.Rx_Out_Index = 0;
         }

//         PAL_ENTER_CRITICAL();
         PAL_Context_D.Rx_Buffers_Free +=Length;
//         PAL_EXIT_CRITICAL();

      }
   }
   else
   {
      PAL_CONSOLE_WRITE_STRING_LITERAL("QCLI initialization failed");
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
   }
}

/**
   @brief This function is used to initialize the Platform, predominately
          the console port.

   @return
    - true if the platform was initialized successfully.
    - false if initialization failed.
*/
static qbool_t PAL_Initialize(void)
{

   uint8_t Ret_Val=true;
//   int filehandle = -1;
//   uint32_t bytes;
//   uint8 val = 0;

   qapi_UART_Open_Config_t open_properties;
   tx_byte_pool_create(&byte_pool_qcli, "byte pool 0", free_memory_qcli, CLI_BYTE_POOL_SIZE);

   memset(&PAL_Context_D, 0, sizeof(PAL_Context_D));
   memset (&open_properties, 0, sizeof (open_properties));


#if  0     /* ----- #if 0 : If0Label_1 ----- */
   qapi_FS_Open( "/datatx/qcli_config",QAPI_FS_O_RDWR_E|QAPI_FS_O_CREAT_E, &filehandle);

   if(filehandle < 0 )
   {
     return 0;
   }
   else
   {
     qapi_FS_Read(filehandle, (void *)&val, sizeof(val), &bytes);
   }
   /* ASCII value of '1' */
   if(val != 0x32)
   {
     qapi_FS_Write(filehandle, "i had runing", 13, &bytes);
     qapi_FS_Close(filehandle);
     return 0;
   }

#endif     /* ----- #if 0 : If0Label_1 ----- */

   open_properties.parity_Mode = QAPI_UART_NO_PARITY_E;
   open_properties.num_Stop_Bits= QAPI_UART_1_0_STOP_BITS_E;
   open_properties.baud_Rate   = 115200;

   open_properties.bits_Per_Char= QAPI_UART_8_BITS_PER_CHAR_E;
   open_properties.rx_CB_ISR = cli_rx_cb;
   open_properties.tx_CB_ISR = NULL;

   open_properties.enable_Flow_Ctrl = false;
   open_properties.enable_Loopback= false;

   if(qapi_UART_Open(&PAL_Context_D.uart_handle, QAPI_UART_PORT_003_E, &open_properties) != QAPI_OK){
       Ret_Val = false;
   }else{
       qapi_UART_Receive(PAL_Context_D.uart_handle, (char *)&(PAL_Context_D.Rx_Buffer[PAL_Context_D.Rx_In_Index]), PAL_RECIEVE_BUFFER_SIZE, (void*)1);
   }

#if  0     /* ----- #if 0 : If0Label_2 ----- */
   qapi_FS_Write(filehandle, "open uart 001", 13, &bytes);
   qapi_FS_Close(filehandle);
#endif     /* ----- #if 0 : If0Label_2 ----- */


   return (Ret_Val);
}

/**
   @brief This function is the main entry point for the application.
*/
int qcli_dam_app_start(void)
{
   int                Result = PAL_Initialize();
   /*Sleep for 100 ms*/
   tx_thread_sleep(10);
   QAPI_MSG_SPRINTF( MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "SIMCOM cust app version 1" );
   /* Initialize the platform.                                             */
   if(Result)
   {
      /* Start the main demo thread. */
      Result = tx_thread_create(&Thread_Handle, "QCLI DAM Thread", QCLI_Thread, 152, app_stack,
                                QCLI_STACK_SIZE, 152, 152, TX_NO_TIME_SLICE, TX_AUTO_START);

      if(Result != TX_SUCCESS)
      {
         PAL_CONSOLE_WRITE_STRING_LITERAL("Failed to start QCLI thread.");
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
         PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
      }
   }

   return(TX_SUCCESS);
}

/**
   @brief This function is used to write a buffer to the console. Note
          that when this function returns, all data from the buffer will
          be written to the console or buffered locally.

   @param Length is the length of the data to be written.
   @param Buffer is a pointer to the buffer to be written to the console.
*/
void PAL_Console_Write(uint32_t Length, const uint8_t *Buffer)
{
   memset (send_buf, 0, sizeof (send_buf));
   if((Length) && (Buffer))
   {
     if(send_buf != NULL)
     {
        memcpy(send_buf, (char*)Buffer, Length);
        qapi_UART_Transmit(PAL_Context_D.uart_handle,send_buf, Length, (void*)send_buf);
     }
     qapi_Timer_Sleep(2,QAPI_TIMER_UNIT_MSEC,true);
   }
}

/**
   @brief This function is indicates to the PAL layer that the application
          should exit. For embedded applications this is typically a reset.
*/
void PAL_Exit(void)
{
   PAL_CONSOLE_WRITE_STRING_LITERAL("Resetting...");
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);

   /* Wait for the transmit buffers to flush.                           */

   /* Reset the platform.                                               */
//xxx
   PAL_CONSOLE_WRITE_STRING_LITERAL("Not Implemented.");
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
   PAL_CONSOLE_WRITE_STRING_LITERAL(PAL_OUTPUT_END_OF_LINE_STRING);
}
