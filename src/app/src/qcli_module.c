/*!
  @file
  qcli_module.c

  @brief
  QCLI framework functionality
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

#include <stdarg.h>

#include "string.h"
#include "pal.h"

#include "qcli.h"
#include "qcli_api.h"

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

/**
   This definition determines the size of the input buffer for CLI
   commansd. It effectively controls the maximum length of a command and
   its parameters.
*/
#define MAXIMUM_QCLI_COMMAND_STRING_LENGTH                              (256)

/**
   This definition determines the maximum number of parameters that can be
   provided. Note that this may also include paramters used to navigete
   into groups.
*/
#define MAXIMUM_NUMBER_OF_PARAMETERS                                    (20)

/**
   This definition determines the size of the buffer used for formatted
   messages to the console when using QCLI_Printf.
*/
#define MAXIMUM_PRINTF_LENGTH                                           (256)

/**
   This definition determines the maximum number of command threads that
   the QCLI allows to be running at a time.
*/
#define MAXIMUM_THREAD_COUNT                                            (5)

/**
   This definition determines the size of the stack (in bytes) that is used
   for command threads.
*/
#define THREAD_STACK_SIZE                                               (8192)

/**
   This definition indicates if received characters should be echoed to
   the console.
*/
#define ECHO_CHARACTERS                                                 (true)

/**
   This defintion determines the index that is used for the first command
   in a command list.  Typically this will be eiher 0 or 1.
*/
#define COMMAND_START_INDEX                                             1

/**
   This definition is the group handle which represents the QCLI itself.
   This is used by the QCLI module to print messages without a group
   prefix.
*/
#define MAIN_PRINTF_HANDLE                                              ((QCLI_Group_Handle_t)&(QCLI_Context_D.Root_Group))

/**
*/
#define THREAD_READY_EVENT_MASK                                         0x00000001

/**
*/
#define COMMAND_THREAD_PRIORITY                                         150

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

/**
   This structure represents a command group list entry.
*/
typedef struct Group_List_Entry_s
{
   const QCLI_Command_Group_t *Command_Group;         /** The command group information.                  */
   struct Group_List_Entry_s  *Parent_Group;          /** the parent group for this subgroup.             */
   struct Group_List_Entry_s  *Subgroup_List;         /** The list of subgroups registerd for this group. */
   struct Group_List_Entry_s  *Next_Group_List_Entry; /** The next entry in the list.                     */
} Group_List_Entry_t;

/**
   This structure reprents the result of a Find_Command() operation.
*/
typedef struct Find_Result_s
{
   qbool_t Is_Group;                           /** A flag indicating if the result is a command or a group. */
   union
   {
      const QCLI_Command_t *Command;           /** The entry that was found if it is a command.             */
      Group_List_Entry_t   *Group_List_Entry;  /** The entry that was found if it is a group.               */
   } Data;
} Find_Result_t;

/**
   This structure contains the information needed for starting a command
   thread.
*/
typedef struct Thread_Info_s
{
   TX_EVENT_FLAGS_GROUP  Thread_Ready_Event; /** Event which indicates the thread no longer needs this information structure. */
   uint32_t              Command_Index;      /** The index of the command that will be executed.                              */
   const QCLI_Command_t *Command;            /** The command that will be executed.                                           */
   uint32_t              Parameter_Count;    /** The number of parameters specified for the command.                          */
   QCLI_Parameter_t     *Parameter_List;     /** The list of paramters for the command.                                       */
} Thread_Info_t;

/**
   This structure contains the context information for the QCLI module.
*/
typedef struct QCLI_Context_s
{
   Group_List_Entry_t  Root_Group;                                            /** The root of the group menu structure.                                    */
   Group_List_Entry_t *Current_Group;                                         /** The current group.                                                       */

   uint32_t            Input_Length;                                          /** The length of the current console input string.                          */
   uint8_t             Input_String[MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1];  /** Buffer containing the current console input string.                      */
   QCLI_Parameter_t    Parameter_List[MAXIMUM_NUMBER_OF_PARAMETERS + 1];      /** List of parameters for input command.                                    */

   uint32_t            Thread_Count;                                          /** THe number of command threads that are currently running.                */
   Thread_Info_t       Thread_Info;                                           /** Information structure for passing information to command threads.        */
   TX_MUTEX            CLI_Mutex;                                             /** The Mutex used to protect shared resources of the module.                */

   uint8_t             Printf_Buffer[MAXIMUM_PRINTF_LENGTH];                  /** The buffer used for formatted output strings.                            */
   QCLI_Group_Handle_t Current_Printf_Group;                                  /** The group handle that was last passed to QCLI_Printf().                  */
   qbool_t             Printf_New_Line;                                       /** Indicates that a newline should be displayed if a printf changes groups. */

} QCLI_Context_t;

QCLI_Context_t QCLI_Context_D;
QCLI_Command_Function_a abort_function_D;         /** Abort Handler Function*/


/*-------------------------------------------------------------------------
 * Static & global Variable Declarations
 *-----------------------------------------------------------------------*/

qbool_t    Start_Thread_Check;
uint8_t    Last_Command[MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1];  /** Buffer containing the last command string.*/
int        Last_Cmd_Length;

extern TX_BYTE_POOL byte_pool_qcli;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

static QCLI_Command_Status_t Command_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Exit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Up(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Root(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static void Display_Group_Name(const Group_List_Entry_t *Group_List_Entry);
static uint32_t Display_Help(Group_List_Entry_t *Command_Group, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static void Display_Usage(uint32_t Command_Index, const QCLI_Command_t *Command);
static void Display_Command_List(const Group_List_Entry_t *Group_List_Entry);

static qbool_t String_To_Integer(const uint8_t *String_Buffer, int32_t *Integer_Value);
static int32_t Memcmpi(const void *Source1, const void *Source2, uint32_t Size);

static void Command_Thread(void *Thread_Parameter);

static void Execute_Command(uint32_t Command_Index, const QCLI_Command_t *Command, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static qbool_t Find_Command(Group_List_Entry_t *Group_List_Entry, QCLI_Parameter_t *Command_Parameter, Find_Result_t *Find_Result);
static void Process_Command(void);
static qbool_t Unregister_Command_Group(Group_List_Entry_t *Group_List_Entry);


   /* The following represents the list of global commands that are     */
   /* supported when not in a group.                                    */
const QCLI_Command_t Root_Command_List_D[] =
{
   {Command_Help, false, "Help", "[Command (optional)]", "Display Command list or usage for a command",NULL},
   {Command_Exit, false, "Exit", "",                     "Exits the application.",NULL}
};

#define ROOT_COMMAND_LIST_SIZE                        (sizeof(Root_Command_List_D) / sizeof(QCLI_Command_t))

   /* The following represents the list of global commands that are     */
   /* supported when in a group.                                        */
const QCLI_Command_t Common_Command_List_D[] =
{
   {Command_Help, false, "Help", "[Command (optional)]", "Display Command list or usage for a command",NULL},
   {Command_Up,   false, "Up",   "",                     "Exit command group (move to parent group)",NULL},
   {Command_Root, false, "Root", "",                     "Move to top-level group list",NULL}
};

#define COMMON_COMMAND_LIST_SIZE                      (sizeof(Common_Command_List_D) / sizeof(QCLI_Command_t))

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/
#define ASSERT_FATAL 1

unsigned long __stack_chk_guard __attribute__((weak)) = 0xDEADDEAD;

void __attribute__((weak)) __stack_chk_fail (void)
{
    return;
}

UINT memscpy(
              void   *dst,
              UINT   dst_size,
              const  void  *src,
              UINT   src_size
              )
    {
      UINT  copy_size = (dst_size <= src_size)? dst_size : src_size;
      memcpy(dst, src, copy_size);
      return copy_size;
    }

/**
   @brief This function processes the "Help" command from the CLI.

   The parameters specified indicate the command or group to display the
   help message for.  If no parameters are specified, the list of commands
   for the current command group will be displayed. If the paramters
   specify a subgroup, the command list for that group will be displayed.
   If the paramters specify a command, the usage message for that command
   will be displayed.
*/
static QCLI_Command_Status_t Command_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Ret_Val;
   uint32_t              Result;
   int32_t               Index;
   int                   status;

   status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
   
   if(status == TX_SUCCESS)
   {
      Result = Display_Help(QCLI_Context_D.Current_Group, Parameter_Count, Parameter_List);

      /* if there was an error parsing the command list, print out an      */
      /* error message here (this is in addition to the usage message that */
      /* will be printed out).                                             */
      if(Result > 0)
      {
         QCLI_Printf(MAIN_PRINTF_HANDLE, "Command \"%s", Parameter_List[0].String_Value);

         for(Index = 1; Index < Result; Index ++)
         {
            QCLI_Printf(MAIN_PRINTF_HANDLE, " %s", Parameter_List[Index].String_Value);
         }

         QCLI_Printf(MAIN_PRINTF_HANDLE, "\" not found.\n");

         Ret_Val = QCLI_STATUS_USAGE_E;
      }
      else
      {
         Ret_Val = QCLI_STATUS_SUCCESS_E;
      }

      tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));
   }
   else
   {
      Ret_Val = QCLI_STATUS_ERROR_E;
   }

   return(Ret_Val);
}

/**
   @brief This function processes the "Quit" command from the CLI.

   For embedded platforms, this is expected to reset the processor. No
   parameters are expected for this command.
*/
static QCLI_Command_Status_t Command_Exit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int status;

   status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
   if(status == TX_SUCCESS)
   {
      PAL_Exit();

      tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));
   }

   return(QCLI_STATUS_SUCCESS_E);
}

/**
   @brief This function processes the "Up" command from the CLI.

   This will change the current group to be the current parent. No
   parameters are expected for this command.
*/
static QCLI_Command_Status_t Command_Up(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int status;
   status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
   if(status == TX_SUCCESS)
   {
      if(QCLI_Context_D.Current_Group != &(QCLI_Context_D.Root_Group))
      {
         QCLI_Context_D.Current_Group = QCLI_Context_D.Current_Group->Parent_Group;

         /* Display the command list again.                             */
         Display_Command_List(QCLI_Context_D.Current_Group);
      }

      tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));
   }

   return(QCLI_STATUS_SUCCESS_E);
}

/**
   @brief This function processes the "Up" command from the CLI.

   This will change the current group to the root group. No parameters
   are expected for this command.
*/
static QCLI_Command_Status_t Command_Root(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   int status;

   status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
   if(status == TX_SUCCESS)
   {
      if(QCLI_Context_D.Current_Group != &(QCLI_Context_D.Root_Group))
      {
         QCLI_Context_D.Current_Group = &(QCLI_Context_D.Root_Group);

         /* Display the command list again.                             */
         Display_Command_List(QCLI_Context_D.Current_Group);
      }

      tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));
   }

   return(QCLI_STATUS_SUCCESS_E);
}

/**
   @brief This function will display the group name, recursively displaying
          the name of the groups parents.

   @param Group_List_Entry is the group list whose name should be
          displayed.  If this isn't the root group, the parent group's
          name will be displayed first.
*/
static void Display_Group_Name(const Group_List_Entry_t *Group_List_Entry)
{
   /* If the group's parent isn't the root, display the parent first.   */
   if(Group_List_Entry->Parent_Group != &(QCLI_Context_D.Root_Group))
   {
      Display_Group_Name(Group_List_Entry->Parent_Group);

      QCLI_Printf(MAIN_PRINTF_HANDLE, "\\");
   }

   /* Display this group's name.                                        */
   QCLI_Printf(MAIN_PRINTF_HANDLE, "%s", Group_List_Entry->Command_Group->Group_String);
}

/**
   @brief This function will processes the help command, recursively
          decending groups if necessary.

   @param Group_List_Entry is the current command group for the help
          command.
   @param Paramter_Count is the number of parameters in the paramter list.
   @param Paramter_List is the paramter list provided to the help command.
          As the groups are recursively decended, the first paramter will
          be stripped off until the list is empty.

   @return
    - 0 if the help was displayed correctly.
    - A positive value indicating the depth of the error if a paramter was
      invalid.
*/
static uint32_t Display_Help(Group_List_Entry_t *Group_List_Entry, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   uint32_t      Ret_Val;
   Find_Result_t Find_Result;

   /* If a parameter was specified, see if we can tie it to a command.  */
   if(Parameter_Count >= 1)
   {
      if(Find_Command(Group_List_Entry, &(Parameter_List[0]), &Find_Result))
      {
         /* Command was found, assign it now.                           */
         if(Find_Result.Is_Group)
         {
            /* If this was a group, recurse into it.                    */
            Ret_Val = Display_Help(Find_Result.Data.Group_List_Entry, Parameter_Count - 1, &(Parameter_List[1]));

            /* If the recursive call returned an error, add one to it.  */
            if(Ret_Val > 0)
            {
               Ret_Val ++;
            }
         }
         else
         {
            /* If this was the last parameter specified, display the    */
            /* usage for the command. If it wasn't, return an error.    */
            if(Parameter_Count == 1)
            {
               Display_Usage(Parameter_List[0].Integer_Value, Find_Result.Data.Command);

               Ret_Val = 0;
            }
            else
            {
               /* The error code indicates that the next parameter is   */
               /* invalid.                                              */
               Ret_Val = 2;
            }
         }
      }
      else
      {
         /* Command not found so return an error.                       */
         Ret_Val = 1;
      }
   }
   else
   {
      /* Display the command list for the current group.                */
      Display_Command_List(Group_List_Entry);

      Ret_Val = 0;
   }

   return(Ret_Val);
}

/**
   @brief This function displays the usage string for a command.

   @param Command_Index is the index of the command in its associated
          commadn group.
   @param Command is the information structure for the command.
*/
static void Display_Usage(uint32_t Command_Index, const QCLI_Command_t *Command)
{
   QCLI_Printf(MAIN_PRINTF_HANDLE, "\n");
   QCLI_Printf(MAIN_PRINTF_HANDLE, "%d: %s %s\n", Command_Index, Command->Command_String, Command->Usage_String);
   QCLI_Printf(MAIN_PRINTF_HANDLE, "    %s\n",    Command->Description);
   QCLI_Printf(MAIN_PRINTF_HANDLE, "\n");
}

/**
   @brief This function displays the list of commands and/or command groups
          for the specified gorup list.

   @param Group_List_Entry is the command group list entry to be displayed.
*/
static void Display_Command_List(const Group_List_Entry_t *Group_List_Entry)
{
   uint32_t              Index;
   uint32_t              Command_Index;
   const QCLI_Command_t *Command_List;
   uint32_t              Command_List_Size;
   Group_List_Entry_t   *Subgroup_List_Entry;

   QCLI_Printf(MAIN_PRINTF_HANDLE, "\n");

   if(Group_List_Entry)
   {
      Command_Index = COMMAND_START_INDEX;

      QCLI_Printf(MAIN_PRINTF_HANDLE, "Command List");

      /* Display the common commands.                                   */
      if(Group_List_Entry == &(QCLI_Context_D.Root_Group))
      {
         Command_List      = Root_Command_List_D;
         Command_List_Size = ROOT_COMMAND_LIST_SIZE;
      }
      else
      {
         QCLI_Printf(MAIN_PRINTF_HANDLE, " (");

         if(Group_List_Entry != &(QCLI_Context_D.Root_Group))
         {
            Display_Group_Name(Group_List_Entry);
         }

         QCLI_Printf(MAIN_PRINTF_HANDLE, ")");

         Command_List      = Common_Command_List_D;
         Command_List_Size = COMMON_COMMAND_LIST_SIZE;
      }

      QCLI_Printf(MAIN_PRINTF_HANDLE, ":\n");

      QCLI_Printf(MAIN_PRINTF_HANDLE, "  Commands:\n");
      for(Index = 0; Index < Command_List_Size; Index ++)
      {
         QCLI_Printf(MAIN_PRINTF_HANDLE, "    %2d. %s\n", Command_Index, Command_List[Index].Command_String);
         Command_Index ++;
      }

      /* Display the command list.                                      */
      if((Group_List_Entry->Command_Group != NULL) && (Group_List_Entry->Command_Group->Command_List != NULL))
      {
         QCLI_Printf(MAIN_PRINTF_HANDLE, "\n");

         for(Index = 0; Index < Group_List_Entry->Command_Group->Command_Count; Index ++)
         {
            QCLI_Printf(MAIN_PRINTF_HANDLE, "    %2d. %s\n", Command_Index, Group_List_Entry->Command_Group->Command_List[Index].Command_String);
            Command_Index ++;
         }
      }

      /* Display the group list.                                        */
      if(Group_List_Entry->Subgroup_List != NULL)
      {
         QCLI_Printf(MAIN_PRINTF_HANDLE, "\n");
         QCLI_Printf(MAIN_PRINTF_HANDLE, "  Subgroups:\n");

         Subgroup_List_Entry = Group_List_Entry->Subgroup_List;
         while(Subgroup_List_Entry != NULL)
         {
            QCLI_Printf(MAIN_PRINTF_HANDLE, "    %2d. %s\n", Command_Index, Subgroup_List_Entry->Command_Group->Group_String);

            Subgroup_List_Entry = Subgroup_List_Entry->Next_Group_List_Entry;
            Command_Index ++;
         }
      }

      QCLI_Printf(MAIN_PRINTF_HANDLE, "\n");
   }
}

/**
   @brief This function attempts to convert a string to an integer.

   It supports strings represented as either a decimal or hexadecimal.  If
   hexadecimal, it is expected for the number to be preceeded by "0x".

   @param String_Buffer is the NULL terminated sring to be converted.
   @param Integer_Value is a pointer to where the integer output will be
          stored upon successful return.

   @return
    - true if the ocnversion was successful.
    - false if the conversion was not succcessful.
*/
static qbool_t String_To_Integer(const uint8_t *String_Buffer, int32_t *Integer_Value)
{
   qbool_t  Ret_Val;
   uint8_t  Character;
   uint32_t Base;
   uint32_t Number;
   uint32_t PrevNumber;
   qbool_t  Number_Is_Negative;

   Ret_Val    = true;
   PrevNumber = 0;
   Number     = 0;

   /* Determine if the number is decimal or hexadecimal.  Note that the */
   /* second character is converted to lower case to make the check     */
   /* simpler.                                                          */
   if((String_Buffer[0] == '0') && ((String_Buffer[1] | 0x20) == 'x'))
   {
      /* Number will be in hexadecimal.  Consume the first two          */
      /* characters ("0x").                                             */
      Base                = 16;
      String_Buffer      += 2;
      Number_Is_Negative  = false;
   }
   else
   {
      /* Number will be in decimal.  Determine if the number is negative*/
      /* and consume the first character of the string ('-') if it is.  */
      Base               = 10;
      Number_Is_Negative = (qbool_t)(String_Buffer[0] == '-');

      if(Number_Is_Negative)
         String_Buffer ++;
   }

   /* Loop until the end of the string is reached or the number is      */
   /* flagged as invalid.                                               */
   while((String_Buffer[0]) && (Ret_Val))
   {
      Number *= Base;

      if((String_Buffer[0] >= '0') && (String_Buffer[0] <= '9'))
      {
         Number += (String_Buffer[0] - '0');
      }
      else
      {
         /* Convert the number to lower case to simplify the check for  */
         /* characters 'a' through 'f'.                                 */
         Character  = String_Buffer[0] | 0x20;

         if((Base == 16) && (Character >= 'a') && (Character <= 'f'))
            Number += (Character + 10 - 'a');
         else
            Ret_Val = false;
      }

      String_Buffer ++;

      /* Make sure that the number has not overflowed. Note that decimal*/
      /* numbers will overflow when it would go negative.               */
      if((Number < PrevNumber) || ((Base == 10) && ((int32_t)Number < 0)))
      {
         Ret_Val = false;
      }
      else
      {
         PrevNumber = Number;
      }
   }

   if(Ret_Val)
   {
      /* If the number is valid and negative, invert it now.            */
      if(Number_Is_Negative)
      {
         /* Make sure the number hasn't overflowed to negative. This is */
         /* mostly for hexadecimal numbers.                             */
         if((int32_t)Number >= 0)
         {
            *Integer_Value = 0 - (int32_t)Number;
         }
         else
         {
            Ret_Val        = false;
            *Integer_Value = 0;
         }
      }
      else
      {
         *Integer_Value = (int32_t)Number;
      }
   }
   else
   {
      /* If the conversion failed, zero out the integer value.          */
      *Integer_Value = 0;
   }

   return(Ret_Val);
}

/**
   @brief This function does a case-insensitive comparison of two buffers.

   It supports strings represented as either a decimal or hexadecimal.  If
   hexadecimal, it is expected for the number to be preceeded by "0x".

   @param Source1 is the first string to be compared.
   @param Source2 is the second string to be compared.

   @return
    - 0  if the two strings are equivalent up to the specified size.
    - -1 if Source1 is "less than" Source2.
    - 1  if Source1 is "greater than" Source2.
*/
static int32_t Memcmpi(const void *Source1, const void *Source2, uint32_t size)
{
   int32_t  Ret_Val;
   uint8_t  Byte1;
   uint8_t  Byte2;
   uint32_t Index;

   Ret_Val = 0;

   /* Simply loop through each byte pointed to by each pointer and check*/
   /* to see if they are equal.                                         */
   for(Index = 0; (Index < size) && (!Ret_Val); Index ++)
   {
      /* Note each Byte that we are going to compare.                   */
      Byte1 = ((uint8_t *)Source1)[Index];
      Byte2 = ((uint8_t *)Source2)[Index];

      /* If the Byte in the first array is lower case, go ahead and make*/
      /* it upper case (for comparisons below).                         */
      if((Byte1 >= 'a') && (Byte1 <= 'z'))
         Byte1 = Byte1 - ('a' - 'A');

      /* If the Byte in the second array is lower case, go ahead and    */
      /* make it upper case (for comparisons below).                    */
      if((Byte2 >= 'a') && (Byte2 <= 'z'))
         Byte2 = Byte2 - ('a' - 'A');

      /* If the two Bytes are equal then there is nothing to do.        */
      if(Byte1 != Byte2)
      {
         /* Bytes are not equal, so set the return value accordingly.   */
         if(Byte1 < Byte2)
            Ret_Val = -1;
         else
            Ret_Val = 1;
      }
   }

   /* Simply return the result of the above comparison(s).              */
   return(Ret_Val);
}

/**
   @brief This function is a wrapper for commands which start in their own
          thread.

   @param Thread_Parameter is the parameter specified when the thread was
          started. It is expected to be a pointer to a Thread_Info_t
          structure.
*/
void Command_Thread(void *Thread_Parameter)
{
   uint32_t              Index;
   Thread_Info_t         Thread_Info;
   uint8_t               Input_String[MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1];
   QCLI_Parameter_t      Parameter_List[MAXIMUM_NUMBER_OF_PARAMETERS];
   QCLI_Command_Status_t Result;
   int status;

   if(Thread_Parameter)
   {
      /* Copy the thread info to local storage.                         */
      memscpy(&Thread_Info, sizeof(Thread_Info), Thread_Parameter, sizeof(Thread_Info_t));
      memscpy(&Input_String, sizeof(Input_String), QCLI_Context_D.Input_String, sizeof(QCLI_Context_D.Input_String));
      memscpy(&Parameter_List, sizeof(Parameter_List), Thread_Info.Parameter_List, Thread_Info.Parameter_Count * sizeof(QCLI_Parameter_t));

      /* Adjust the pointers in the paramter list for the local input   */
      /* string.                                                        */
      for(Index = 0; Index < Thread_Info.Parameter_Count; Index ++)
      {
         Parameter_List[Index].String_Value += (Input_String - QCLI_Context_D.Input_String);
      }

      /* Singal that the thread is ready.                               */
      tx_event_flags_set(&(Thread_Info.Thread_Ready_Event), THREAD_READY_EVENT_MASK, TX_OR);

      /* Execute the command.                                           */
      Result = (*(Thread_Info.Command->Command_Function))(Thread_Info.Parameter_Count, Parameter_List);

      /* Take the mutex before modifying any global variables.          */
      status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
      if(status == TX_SUCCESS)
      {
         if(Result == QCLI_STATUS_USAGE_E)
         {
            /* Print the usage message.                                 */
            Display_Usage(Thread_Info.Command_Index, Thread_Info.Command);
            QCLI_Display_Prompt();
         }

         /* Decrement the number of active threads.                     */
         QCLI_Context_D.Thread_Count --;

         tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));
      }
   }

}

/**
   @brief This function executes a given command function.

   @param Command_Index is the index of the command to be executed in its
          associated command group.
   @param command is the information structure for the command to be
          executed.
   @param Parameter_Count is the parameter count that is passed to the
          command.
   @param Parameter_List is a pointer to the parameter list that is passed
          to the command.
*/
static void Execute_Command(uint32_t Command_Index, const QCLI_Command_t *Command, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   QCLI_Command_Status_t Result;
   int                   ret;
   int                   status;

   char *cmd_stack = NULL;
   TX_THREAD *Thread_Handle_cmd = NULL;
   
   if(Command->Start_Thread)
   {
       tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));
      /* Make sure we haven't maxed out the number of supported threads.*/
      if(QCLI_Context_D.Thread_Count < MAXIMUM_THREAD_COUNT)
      {
         QCLI_Context_D.Thread_Count ++;

         /* Make sure the running event semaphore is taken.             */
         tx_event_flags_set(&(QCLI_Context_D.Thread_Info.Thread_Ready_Event), ~THREAD_READY_EVENT_MASK, TX_AND);

         /* Pass the function to the thread pool.                       */
         QCLI_Context_D.Thread_Info.Command         = Command;
         QCLI_Context_D.Thread_Info.Command_Index   = Command_Index;
         QCLI_Context_D.Thread_Info.Parameter_Count = Parameter_Count;
         QCLI_Context_D.Thread_Info.Parameter_List  = Parameter_List;

         /* Allocate the stack for cmd thread. */ 
         tx_byte_allocate(&byte_pool_qcli, (VOID **) &cmd_stack, THREAD_STACK_SIZE, TX_NO_WAIT);
         tx_byte_allocate(&byte_pool_qcli, (VOID **) &Thread_Handle_cmd, sizeof(TX_THREAD), TX_NO_WAIT);
         if(cmd_stack == NULL || Thread_Handle_cmd == NULL)
         {
            QCLI_Printf(MAIN_PRINTF_HANDLE, "Memory Allocation failure for command Thread!! \n");
            return ;
         }


         /* Create a thread for the command.                            */

         ret = tx_thread_create(Thread_Handle_cmd, "Command Thread", Command_Thread, (void *)&(QCLI_Context_D.Thread_Info),
                                cmd_stack, THREAD_STACK_SIZE, COMMAND_THREAD_PRIORITY, COMMAND_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);

         if(ret == TX_SUCCESS)
         {
            QCLI_Printf(MAIN_PRINTF_HANDLE, "Seperate thread created for command (%d).\n", ret);
         }
         else
         {
            QCLI_Printf(MAIN_PRINTF_HANDLE, "Failed to create thread for command (%d).\n", ret);

            QCLI_Context_D.Thread_Count --;
         }
      }
      else
      {
         QCLI_Printf(MAIN_PRINTF_HANDLE, "Max threads reached.\n");
      }
   }
   else
   {
      /* Release the mutex while the function is being executed.        */
      tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));

      abort_function_D = Command->abort_function;
      Result = (*(Command->Command_Function))(Parameter_Count, Parameter_List);

      status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
      if(status != TX_SUCCESS)
      {
         QCLI_Printf(MAIN_PRINTF_HANDLE, "Failed to re-take the mutex!\n");
      }

      if(Result == QCLI_STATUS_USAGE_E)
         Display_Usage(Command_Index, Command);
   }
}

/**
   @brief This function searches the command and/or group lists for a
          match to the provided parameter.

   @param Group_List_Entry is the group to search.
   @param Command_Parameter is the paramter to search for.
   @param Find_Result is a pointer to where the found entry will be stored
          if successful (i.e., true was returned.

   @return
    - true if a matching command or group was found in the list.
    - false if the command or group was not found.
*/
static qbool_t Find_Command(Group_List_Entry_t *Group_List_Entry, QCLI_Parameter_t *Command_Parameter, Find_Result_t *Find_Result)
{
   qbool_t               Ret_Val;
   uint32_t              Index;
   uint32_t              Command_Index;
   uint32_t              String_Length;
   const QCLI_Command_t *Command_List;
   uint32_t              Command_List_Length;
   Group_List_Entry_t   *Subgroup_List_Entry;

   /* Get the size of the string. Include the null byte so the          */
   /* comparison doesn't match substrings.                              */
   String_Length = strlen((const char *)(Command_Parameter->String_Value)) + 1;

   if(Group_List_Entry != NULL)
   {
      /* Determine which common command list is going to be used.       */
      if(Group_List_Entry == &(QCLI_Context_D.Root_Group))
      {
         Command_List        = Root_Command_List_D;
         Command_List_Length = ROOT_COMMAND_LIST_SIZE;
      }
      else
      {
         Command_List        = Common_Command_List_D;
         Command_List_Length = COMMON_COMMAND_LIST_SIZE;
      }

      if(Command_Parameter->Integer_Is_Valid)
      {
         /* Command was specified as an integer.                        */
         if(Command_Parameter->Integer_Value >= COMMAND_START_INDEX)
         {
            Command_Index = Command_Parameter->Integer_Value - COMMAND_START_INDEX;

            /* If the integer is a valid value for the command group,   */
            /* use it.                                                  */
            if(Command_Index < Command_List_Length)
            {
               /* Command is in the common command list.                */
               Ret_Val                   = true;
               Find_Result->Is_Group     = false;
               Find_Result->Data.Command = &(Command_List[Command_Index]);
            }
            else
            {
               Command_Index -= Command_List_Length;

               if((Group_List_Entry->Command_Group != NULL) && (Command_Index < Group_List_Entry->Command_Group->Command_Count))
               {
                  /* Command is in the group's command list.            */
                  Ret_Val                   = true;
                  Find_Result->Is_Group     = false;
                  Find_Result->Data.Command = &(Group_List_Entry->Command_Group->Command_List[Command_Index]);
               }
               else
               {
                  if(Group_List_Entry->Command_Group != NULL)
                  {
                     Command_Index -= Group_List_Entry->Command_Group->Command_Count;
                  }

                  /* Search the group list.                             */
                  Group_List_Entry = Group_List_Entry->Subgroup_List;
                  while((Group_List_Entry != NULL) && (Command_Index != 0))
                  {
                     Group_List_Entry = Group_List_Entry->Next_Group_List_Entry;
                     Command_Index --;
                  }

                  if(Group_List_Entry != NULL)
                  {
                     /* Command is in the subgroup list.                */
                     Ret_Val                            = true;
                     Find_Result->Is_Group              = true;
                     Find_Result->Data.Group_List_Entry = Group_List_Entry;
                  }
                  else
                  {
                     Ret_Val = false;
                  }
               }
            }
         }
         else
         {
            Ret_Val = false;
         }
      }
      else
      {
         /* Command was specified as a string.                          */
         Command_Index = COMMAND_START_INDEX;
         Ret_Val       = false;

         /* Search the common command list.                             */
         for(Index = 0; (Index < Command_List_Length) && (!Ret_Val); Index ++)
         {
            if(Memcmpi(Command_Parameter->String_Value, Command_List[Index].Command_String, String_Length) == 0)
            {
               /* Command found.                                        */
               Ret_Val                          = true;
               Find_Result->Is_Group            = false;
               Find_Result->Data.Command        = &(Command_List[Index]);
               Command_Parameter->Integer_Value = Command_Index;
            }
            else
            {
               Command_Index ++;
            }
         }

         /* Only search the command group if it isn't NULL.             */
         if((!Ret_Val) && (Group_List_Entry->Command_Group != NULL))
         {
            /* If the comamnd wasn't found yet, search the group's   */
            /* command list.                                         */
            for(Index = 0; (Index < Group_List_Entry->Command_Group->Command_Count) && (!Ret_Val); Index ++)
            {
               if(Memcmpi(Command_Parameter->String_Value, Group_List_Entry->Command_Group->Command_List[Index].Command_String, String_Length) == 0)
               {
                  /* Command found.                                  */
                  Ret_Val                           = true;
                  Find_Result->Is_Group             = false;
                  Find_Result->Data.Command         = &(Group_List_Entry->Command_Group->Command_List[Index]);
                  Command_Parameter->Integer_Value  = Command_Index;
               }
               else
               {
                  Command_Index ++;
               }
            }
         }

         if(!Ret_Val)
         {
            /* If the comamnd wasn't found yet, search the group's   */
            /* subgroup list.                                        */
            Subgroup_List_Entry = Group_List_Entry->Subgroup_List;
            while((Subgroup_List_Entry != NULL) && (!Ret_Val))
            {
               if(Memcmpi(Command_Parameter->String_Value, Subgroup_List_Entry->Command_Group->Group_String, String_Length) == 0)
               {
                  /* Command found.                                  */
                  Ret_Val                            = true;
                  Find_Result->Is_Group              = true;
                  Find_Result->Data.Group_List_Entry = Subgroup_List_Entry;
                  Command_Parameter->Integer_Value   = Command_Index;
               }
               else
               {
                  Command_Index ++;
                  Subgroup_List_Entry = Subgroup_List_Entry->Next_Group_List_Entry;
               }
            }
         }
      }
   }
   else
   {
      Ret_Val = false;
   }

   return(Ret_Val);
}

/**
   @brief This function processes a command received from the console.
*/
static void Process_Command(void)
{
   qbool_t       Result;
   Find_Result_t Find_Result;
   uint32_t      Parameter_Count;
   uint32_t      Index;
   uint32_t      Command_Index;
   Start_Thread_Check = false;

   /* Parse the command until its end is reached or the parameter list  */
   /* is full.                                                          */
   Parameter_Count = 0;
   Index       = 0;
   while((Index < QCLI_Context_D.Input_Length) && (Parameter_Count <= MAXIMUM_NUMBER_OF_PARAMETERS))
   {
      /* Consume any leading white space.                               */
      while(QCLI_Context_D.Input_String[Index] == ' ')
         Index ++;

      /* Assuming the end of the command hasn't been reached, assign the*/
      /* current string location as the current parameter's string.     */
      if(Index < QCLI_Context_D.Input_Length)
      {
         QCLI_Context_D.Parameter_List[Parameter_Count].String_Value = &QCLI_Context_D.Input_String[Index];

         /* Find the end of the string and NULL terminate it.           */
         while((Index < QCLI_Context_D.Input_Length) && (QCLI_Context_D.Input_String[Index] != ' '))
            Index ++;

         QCLI_Context_D.Input_String[Index] = '\0';
         Index++;

         /* Try to convert the command to an integer.                   */
         QCLI_Context_D.Parameter_List[Parameter_Count].Integer_Is_Valid = String_To_Integer(QCLI_Context_D.Parameter_List[Parameter_Count].String_Value, &(QCLI_Context_D.Parameter_List[Parameter_Count].Integer_Value));

         Parameter_Count++;
      }
   }

   if(Parameter_Count > 0)
   {
      /* initialize the find rsults to the current group state so that  */
      /* it can be used to recursively search the groups.               */
      Find_Result.Data.Group_List_Entry = QCLI_Context_D.Current_Group;
      Find_Result.Is_Group              = true;
      Result                            = true;
      Index                             = 0;

      /* Search for the command that was entered. Note that if the      */
      /* command or group is found, the index will actually indicate    */
      /* the first parameter for the command.                           */
      while((Result) && (Find_Result.Is_Group) && (Index < Parameter_Count))
      {
         Result = Find_Command(Find_Result.Data.Group_List_Entry, &(QCLI_Context_D.Parameter_List[Index]), &Find_Result);

         Index ++;
      }

      if(Result)
      {
         if(Find_Result.Is_Group)
         {
            /* Final command is a group, navigate into it.              */
            QCLI_Context_D.Current_Group = Find_Result.Data.Group_List_Entry;
         }
         else
         {
            if(Find_Result.Data.Command->Start_Thread)
               Start_Thread_Check= true;

            /* Execute the command.                                     */
            Execute_Command(QCLI_Context_D.Parameter_List[Index - 1].Integer_Value, Find_Result.Data.Command, Parameter_Count - Index, (Parameter_Count > Index) ? &(QCLI_Context_D.Parameter_List[Index]) : NULL);
         }
      }
      else
      {
         QCLI_Printf(MAIN_PRINTF_HANDLE, "Command \"%s", QCLI_Context_D.Parameter_List[0].String_Value);

         for(Command_Index = 1; Command_Index < Index; Command_Index ++)
         {
            QCLI_Printf(MAIN_PRINTF_HANDLE, " %s", QCLI_Context_D.Parameter_List[Command_Index].String_Value);
         }

         QCLI_Printf(MAIN_PRINTF_HANDLE, "\" not found.\n");
      }
   }
}

/**
   @brief This function will unregister the specified group from the
          command list and recursively unregister any subgroup's that are
          registered for the group.

   @param Group_List_Entry is a pointer to the comand group to be removed.

   @return
    - true if the current group changed as a result of the group being
      unregistered.
    - false if the current group didn't change.
*/
static qbool_t Unregister_Command_Group(Group_List_Entry_t *Group_List_Entry)
{
   qbool_t             Ret_Val;
   Group_List_Entry_t *Current_Entry;
   qbool_t             Group_Is_Valid;

   /* First, remove the group from its parent's list.                   */
   if(Group_List_Entry->Parent_Group->Subgroup_List == Group_List_Entry)
   {
      /* Group is at the head of the subgroup list.                     */
      Group_List_Entry->Parent_Group->Subgroup_List = Group_List_Entry->Next_Group_List_Entry;
      Group_Is_Valid = true;
   }
   else
   {
      /* Find the entry in its parent's subgroup list.                  */
      Current_Entry = Group_List_Entry->Parent_Group->Subgroup_List;

      while((Current_Entry != NULL) && (Current_Entry->Next_Group_List_Entry != Group_List_Entry))
      {
         Current_Entry = Current_Entry->Next_Group_List_Entry;
      }

      if(Current_Entry != NULL)
      {
         Current_Entry->Next_Group_List_Entry = Group_List_Entry->Next_Group_List_Entry;

         Group_Is_Valid = true;
      }
      else
      {
         Group_Is_Valid = false;
      }
   }

   if(Group_Is_Valid)
   {
      /* Unregsiter any subgroups of the command.                       */
      Current_Entry = Group_List_Entry->Subgroup_List;
      Ret_Val       = false;

      while(Current_Entry != NULL)
      {
         if(Unregister_Command_Group(Current_Entry))
            Ret_Val = true;
      }

      /* If this is the current group, move up to its parent.           */
      if(QCLI_Context_D.Current_Group == Group_List_Entry)
      {
         QCLI_Context_D.Current_Group = Group_List_Entry->Parent_Group;
         Ret_Val                    = true;
      }

      /* Free the resources for the group.                              */
      tx_byte_release(Group_List_Entry);
   }
   else
   {
      Ret_Val = false;
   }

   return(Ret_Val);
}

/**
   @brief This function is used to initialize the QCLI module.

   This function must be called before any other QCLI functions.

   @return
    - true if QCLI was initialized successfully.
    - false if initialization failed.
*/
qbool_t QCLI_Initialize(void)
{
   /* Initialize the context information.                               */
   memset(&QCLI_Context_D, 0, sizeof(QCLI_Context_D));
   QCLI_Context_D.Current_Group = &(QCLI_Context_D.Root_Group);

   /* Attempt to create a mutex for the QCLI module.                    */
   
   tx_mutex_create(&(QCLI_Context_D.CLI_Mutex), "qcli_mutex", TX_INHERIT);

   /* Initialize the thread ready event.                                */
   tx_event_flags_create(&(QCLI_Context_D.Thread_Info.Thread_Ready_Event), "qcli_sig");

   /* Initialize the abort_function_D with NULL.                                */
   abort_function_D = NULL;

   return(true);
}

/**
   @brief This function passes characters input from the command line to
          the QCLI module for processing.

   @param Length is the number of bytes in the provided buffer.
   @param Buffer is a pointer to the buffer containing the inputted data.

   @return
    - true if QCLI was initialized successfully.
    - false if initialization failed.
*/
void QCLI_Process_Input_Data(uint32_t Length, uint8_t *Buffer)
{
   int status;

   status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
   if(status == TX_SUCCESS)
   {
      if((Length) && (Buffer))
      {
         /* Process all received data.                                  */
         while(Length)
         {
            /* Check for an end of line character.                      */
            if(Buffer[0] == PAL_INPUT_END_OF_LINE_CHARACTER)
            {
#if ECHO_CHARACTERS

               PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING), PAL_OUTPUT_END_OF_LINE_STRING);

#endif

               /* Command is complete, copy to Last_Command & process it now.                  */

               memscpy(&Last_Command, sizeof(Last_Command), QCLI_Context_D.Input_String, sizeof(QCLI_Context_D.Input_String));
               Last_Cmd_Length = QCLI_Context_D.Input_Length;

               Process_Command();

               /* Set the command length back to zero in preparation of */
               /* the next command and display the prompt.              */
               QCLI_Context_D.Input_Length = 0;
               memset(QCLI_Context_D.Input_String, '\0', sizeof(QCLI_Context_D.Input_String));
               QCLI_Display_Prompt();
            }
            else
            {
               /* Check for backspace character.                           */
               if(Buffer[0] == 8 || Buffer[0] == 127)
               {
                  /* Consume a character from the command if one has    */
                  /* been entered.                                      */
                  if(QCLI_Context_D.Input_Length)
                  {
#if ECHO_CHARACTERS

                     PAL_Console_Write(3, "\b \b");

#endif

                     QCLI_Context_D.Input_Length --;
                     QCLI_Context_D.Input_String[QCLI_Context_D.Input_Length] = '\0';
                  }
               }
               else if(Buffer[0] == 27 && Buffer[1] == '[' && Buffer[2] == 'A')
               {
                   while(QCLI_Context_D.Input_Length > 0)
                   {
                       PAL_Console_Write(3, "\b \b");
                       QCLI_Context_D.Input_Length--;
                   }
                   PAL_Console_Write(Last_Cmd_Length, &Last_Command[0]);
                   memscpy( QCLI_Context_D.Input_String, sizeof(QCLI_Context_D.Input_String), &Last_Command, sizeof(Last_Command));
                   QCLI_Context_D.Input_Length = Last_Cmd_Length;
                   break;
               }
               else if(Buffer[0] == 3)
               {
#if ECHO_CHARACTERS

               PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING), PAL_OUTPUT_END_OF_LINE_STRING);

#endif

               /* Set the command length back to zero in preparation of */
               /* the next command and display the prompt.              */
               QCLI_Context_D.Input_Length = 0;
               memset(QCLI_Context_D.Input_String, '\0', sizeof(QCLI_Context_D.Input_String));
               QCLI_Display_Prompt();
               }
               else
               {
                  /* Check for a valid character, which here is any non */
                  /* control code lower ASCII (0x20 ' ' to 0x7E '~').   */
                  if((*Buffer >= ' ') && (*Buffer <= '~'))
                  {
                     /* Make sure that the command buffer can fit the   */
                     /* character.                                      */
                     if(QCLI_Context_D.Input_Length < MAXIMUM_QCLI_COMMAND_STRING_LENGTH)
                     {
#if ECHO_CHARACTERS

                        PAL_Console_Write(1, Buffer);

#endif

                        QCLI_Context_D.Input_String[QCLI_Context_D.Input_Length] = Buffer[0];
                        QCLI_Context_D.Input_Length++;
                     }
                  }
               }
            }

            /* Move to the next character in the buffer.                */
            Buffer ++;
            Length --;
         }
      }

	  tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));

   }
}

/**
   @brief This function displays the current command list.

   It is intended to provide a means for the initial command list to be
   displayed once platform initialization is complete.
*/
void QCLI_Display_Command_List(void)
{
   int status;
   status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
   if(status == TX_SUCCESS)
   {
      Display_Command_List(QCLI_Context_D.Current_Group);

      QCLI_Display_Prompt();

	  tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));

   }
}

/**
   @brief This function is used to register a command group with the CLI.

   @param Parent_Group is the group which this group should be registerd
          under as a subgroup.  If this parameter is NULL, then the group
          will be registered at the top level.
   @param Command_Group is the command group to be registered.  Note that
          this function assumes the command group information will be
          constant and simply stores a pointer to the data.  If the command
          group and its associated information is not constant, its memory
          MUST be retained until the command is unregistered.

   @return
    - THe handle for the group that was added.
    - NULL if there was an error registering the group.
*/
QCLI_Group_Handle_t QCLI_Register_Command_Group(QCLI_Group_Handle_t Parent_Group, const QCLI_Command_Group_t *Command_Group)
{
   Group_List_Entry_t *New_Entry = NULL;
   Group_List_Entry_t *Current_Entry = NULL;
   int status;

   status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
   if(status == TX_SUCCESS)
   {
      /* Create the new entry.                                          */
      tx_byte_allocate(&byte_pool_qcli, (VOID **) &New_Entry, sizeof(Group_List_Entry_t), TX_NO_WAIT);
      if(New_Entry)
      {
         New_Entry->Command_Group         = Command_Group;
         New_Entry->Next_Group_List_Entry = NULL;
         New_Entry->Subgroup_List         = NULL;

         if(Parent_Group == NULL)
         {
            New_Entry->Parent_Group = &(QCLI_Context_D.Root_Group);
         }
         else
         {
            New_Entry->Parent_Group = (Group_List_Entry_t *)Parent_Group;
         }

         /* Add the new entry to its parents subgroup list.                */
         if(New_Entry->Parent_Group->Subgroup_List == NULL)
         {
            New_Entry->Parent_Group->Subgroup_List = New_Entry;
         }
         else
         {
            Current_Entry = New_Entry->Parent_Group->Subgroup_List;
            while(Current_Entry->Next_Group_List_Entry != NULL)
            {
               Current_Entry = Current_Entry->Next_Group_List_Entry;
            }
   
            Current_Entry->Next_Group_List_Entry = New_Entry;
         }
      }

      tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));

   }
   else
   {
      New_Entry = NULL;
   }

   return((QCLI_Group_Handle_t)New_Entry);
}

/**
   @brief This function is used to usregister a command group from the CLI.

   @param Group_Handle is the handle for the group to be unregistered.
          This will be the value returned form
          QCLI_Register_Command_Group() when the function was registered.
          Note that if the specified group has subgroups, they will be
          unregistred as well.
*/
void QCLI_Unregister_Command_Group(QCLI_Group_Handle_t Group_Handle)
{
   int status;
   if(Group_Handle != NULL)
   {
      status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
      if(status == TX_SUCCESS)
      {
         if(Unregister_Command_Group((Group_List_Entry_t *)Group_Handle))
         {
            /* The current menu level changed so update the prompt.     */
            QCLI_Display_Prompt();
         }

         tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));

      }
   }
}

/**
   @brief This function prints the prompt to the console.

   This provides a means to re-display the prompt after printing data to
   the console from an asynchronous function such as a callback or seperate
   command thread.
*/
void QCLI_Display_Prompt(void)
{
   int status;
   status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
   if(status == TX_SUCCESS)
   {
      QCLI_Printf(MAIN_PRINTF_HANDLE, "\n");

      /* Recursively display the name for the current group.            */
      if(QCLI_Context_D.Current_Group != &(QCLI_Context_D.Root_Group))
      {
         Display_Group_Name(QCLI_Context_D.Current_Group);
      }

      QCLI_Printf(MAIN_PRINTF_HANDLE, "> ");

      /* Display the current command string.                            */
      if(QCLI_Context_D.Input_Length != 0)
      {
         PAL_Console_Write(QCLI_Context_D.Input_Length, QCLI_Context_D.Input_String);
      }

      tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));

   }
}

/**
   @brief This function prints a formated string to the CLI.

   Note that this function will also replace newline characters ('\n') with
   the string specified by PAL_OUTPUT_END_OF_LINE_STRING.

   @param Group_Handle is the handle for the group associated with the
          message being printed.  This will be the value returned when the
          group was registered using QCLI_Regsiter_Command_Group().
   @param Format is the formatted string to be printed.
   @param ... is the variatic parameter for the format string.
*/
void QCLI_Printf(QCLI_Group_Handle_t Group_Handle, const uint8_t *format, ...)
{
   uint32_t            Index;
   uint32_t            Next_Print_Index;
   uint32_t            Length;
   va_list             Arg_List;
   Group_List_Entry_t *Group_List_Entry;
   int status;

   if((Group_Handle != NULL) && (format != NULL))
   {
      status = tx_mutex_get(&(QCLI_Context_D.CLI_Mutex) , TX_WAIT_FOREVER);
      if(status == TX_SUCCESS)
      {
         Group_List_Entry = (Group_List_Entry_t *)Group_Handle;

         /* Print the string to the buffer.                                */
         va_start(Arg_List, format);
         Length = vsnprintf((char *)(QCLI_Context_D.Printf_Buffer), sizeof(QCLI_Context_D.Printf_Buffer), (char *)format, Arg_List);
         va_end(Arg_List);

         /* Make sure the length is not greater than the buffer size       */
         /* (taking the NULL terminator into account).                     */
         if(Length > sizeof(QCLI_Context_D.Printf_Buffer) - 1)
            Length = sizeof(QCLI_Context_D.Printf_Buffer) - 1;

         /* Print the group name first. Note that the main handle          */
         /* indicates the message is from the QCLI itself and as such      */
         /* doesn't print a group name.                                    */
         if(Group_Handle != QCLI_Context_D.Current_Printf_Group)
         {
            if(QCLI_Context_D.Printf_New_Line)
            {
               PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING) - 1, PAL_OUTPUT_END_OF_LINE_STRING);
               QCLI_Context_D.Printf_New_Line = false;
            }

            if((Group_Handle != MAIN_PRINTF_HANDLE) && (QCLI_Context_D.Printf_Buffer[0] != '\n'))
            {
               PAL_Console_Write(strlen((char *)(Group_List_Entry->Command_Group->Group_String)), Group_List_Entry->Command_Group->Group_String);
               PAL_Console_Write(2, ": ");
            }
         }

         QCLI_Context_D.Current_Printf_Group = Group_Handle;

         /* Write the buffer to the console, setting EOL characters        */
         /* accordingly.                                                   */
         Next_Print_Index = 0;
         for(Index = 0; Index < Length; Index ++)
         {
            if(QCLI_Context_D.Printf_Buffer[Index] == '\n')
            {
               /* Print out the buffer so far and replace the '\n' with the*/
               /* configured EOL string.                                   */
               if(Index != Next_Print_Index)
                  PAL_Console_Write(Index - Next_Print_Index, &(QCLI_Context_D.Printf_Buffer[Next_Print_Index]));

               PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING) - 1, PAL_OUTPUT_END_OF_LINE_STRING);

               Next_Print_Index = Index + 1;

               if(Length != (Index + 1))
               {
                  /* Redsiplay the group name at the start of a new line if*/
                  /* its not immidiately succeeded by another new line.    */
                  if(QCLI_Context_D.Printf_Buffer[Index + 1] != '\n')
                  {
                     if (Group_List_Entry && Group_List_Entry->Command_Group)
                     {
                        PAL_Console_Write(strlen((char *)(Group_List_Entry->Command_Group->Group_String)), Group_List_Entry->Command_Group->Group_String);
                        PAL_Console_Write(2, ": ");
                     }
                  }
               }
               else
               {
                  /* This printout stopped on the newline so set the       */
                  /* current print group to the main group to prompt the   */
                  /* next line to redisplay the group name.                */
                  QCLI_Context_D.Current_Printf_Group = MAIN_PRINTF_HANDLE;
               }

               QCLI_Context_D.Printf_New_Line = false;
            }
            else
            {
               QCLI_Context_D.Printf_New_Line = true;
            }
         }

         /* Print the remaining buffer after the last newline.             */
         if(Length != Next_Print_Index)
            PAL_Console_Write(Length - Next_Print_Index, &(QCLI_Context_D.Printf_Buffer[Next_Print_Index]));

         tx_mutex_put(&(QCLI_Context_D.CLI_Mutex));

      }
   }
}


QCLI_Command_Status_t abort_handler(void)
{
QCLI_Command_Status_t res = QCLI_STATUS_SUCCESS_E;

    if(abort_function_D != NULL)
       res = (*abort_function_D)();

    abort_function_D = NULL;
    return res;
}

