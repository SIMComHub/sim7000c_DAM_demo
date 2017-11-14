#!/bin/sh
#################################################################
#Copyright (c) 2002-2017 SIMCom Wireless Solutions Co.,Ltd.. 
#All rights reserved. 
#################################################################

#config compile env
Local_lincense=@172.21.1.76
TOOLCHAIN_PATH="/opt/QualComm/RVCT5.05b106/bin"
export ARMLMD_LICENSE_FILE=$Local_lincense

DAM_RO_BASE=0x42000000
DAM_OUTPUT_PATH="./bin"
DAM_INC_BASE="./include"
DAM_LIB_PATH="./libs"
DAM_SRC_PATH="./src"

TS_APP_SRC_PATH="./src/app/src"
TS_APP_INC_PATH="./src/app/inc"
TS_APP_OUTPUT_PATH="./src/app/build"


DAM_LIBNAME="txm_lib.lib"
TIMER_LIBNAME="timer_dam_lib.lib"

DAM_ELF_NAME="cust_app.elf"
DAM_TARGET_BIN="cust_app.bin"

if [ $# -eq 1 ];
  then
    if [ $1 == "-c" ];
	   then
		  echo "Cleaning..."
		  rm -rf $DAM_OUTPUT_PATH/*
		  rm -rf $TS_APP_OUTPUT_PATH/*
		  echo "Done."
		  exit
	   else
        DAM_RO_BASE=$1
	 fi
fi

echo "Application RO base selected = $DAM_RO_BASE"

export DAM_CPPFLAGS="-DT_ARM -D__RVCT__ -D_ARM_ASM_ -DQAPI_TXM_MODULE -DTXM_MODULE -DTX_DAM_QC_CUSTOMIZATIONS -DTX_ENABLE_PROFILING -DTX_ENABLE_EVENT_TRACE -DTX_DISABLE_NOTIFY_CALLBACKS -DTX_DAM_QC_CUSTOMIZATIONS" 

export DAM_CFLAGS="-O1 --diag_suppress=9931 --diag_error=warning --cpu=Cortex-A7 --protect_stack --arm_only --apcs=/interwork"

export DAM_INCPATHS="-I $DAM_INC_BASE -I $DAM_INC_BASE/threadx_api -I $DAM_INC_BASE/qapi -I $TS_APP_INC_PATH"

export APP_CFLAGS="-DTARGET_THREADX -D__SIMCOM_DAM__"
#Turn on verbose mode by default
set -x;

echo "Compiling CustApp application"

if [ ! -d $TS_APP_OUTPUT_PATH ]; then
    mkdir $TS_APP_OUTPUT_PATH
fi

if [ ! -d $DAM_OUTPUT_PATH ]; then
    mkdir $DAM_OUTPUT_PATH 
fi

$TOOLCHAIN_PATH/armcc -E -g $DAM_CPPFLAGS $DAM_CFLAGS $DAM_SRC_PATH/txm_module_preamble.S > txm_module_preamble_pp.S

$TOOLCHAIN_PATH/armcc -g -c $DAM_CPPFLAGS $DAM_CFLAGS txm_module_preamble_pp.S -o $TS_APP_OUTPUT_PATH/txm_module_preamble.o

rm txm_module_preamble_pp.S

$TOOLCHAIN_PATH/armcc -g -c $DAM_CPPFLAGS $DAM_CFLAGS $APP_CFLAGS $DAM_INCPATHS $TS_APP_SRC_PATH/*.c 


mv *.o $TS_APP_OUTPUT_PATH

echo "Linking CustApp application"
$TOOLCHAIN_PATH/armlink -d -o $DAM_OUTPUT_PATH/$DAM_ELF_NAME --elf --ro $DAM_RO_BASE --first txm_module_preamble.o --entry=_txm_module_thread_shell_entry --map --remove --symbols --list $DAM_OUTPUT_PATH/dam_app.map $TS_APP_OUTPUT_PATH/*.o  $DAM_LIB_PATH/$DAM_LIBNAME $DAM_LIB_PATH/$TIMER_LIBNAME

$TOOLCHAIN_PATH/fromelf --bincombined $DAM_OUTPUT_PATH/$DAM_ELF_NAME --output $DAM_OUTPUT_PATH/$DAM_TARGET_BIN

echo "CustApp application is built at" $DAM_OUTPUT_PATH/$DAM_TARGET_BIN
