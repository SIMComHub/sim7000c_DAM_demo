@echo off
rem ################################################################
rem Copyright (c) 2002-2017 SIMCom Wireless Solutions Co.,Ltd.. 
rem All rights reserved. 
rem ################################################################

rem config compile env
set TOOLCHAIN_PATH="c:\Program Files (x86)\ARM\bin64"
set ARMLMD_LICENSE_FILE=@172.21.1.76
rem set ARMLMD_LICENSE_FILE=c:\Program Files\ARM\Licenses\license.dat

set DAM_RO_BASE=0x42000000
set DAM_OUTPUT_PATH=.\bin
set DAM_INC_BASE=.\include
set DAM_LIB_PATH=.\libs
set DAM_SRC_PATH=.\src

set TS_APP_SRC_PATH=.\src\app\src
set TS_APP_INC_PATH=.\src\app\inc
set TS_APP_OUTPUT_PATH=.\src\app\build


set DAM_LIBNAME=txm_lib.lib
set TIMER_LIBNAME=timer_dam_lib.lib

set DAM_ELF_NAME=cust_app.elf
set DAM_TARGET_BIN=cust_app.bin
if not exist %TS_APP_OUTPUT_PATH% md %TS_APP_OUTPUT_PATH%
if not exist %DAM_OUTPUT_PATH% md %DAM_OUTPUT_PATH%
if  not "%1" == ""  (
if  "%1" == "-c" (
        echo "Cleaning..."
        del /F /Q %DAM_OUTPUT_PATH%\*
        del /F /Q %TS_APP_OUTPUT_PATH%\*
        echo "Done."
        goto :END
        ) else (
        DAM_RO_BASE=%1
        )
)
echo "Application RO base selected = %DAM_RO_BASE%"

set DAM_CPPFLAGS=-DT_ARM -D__RVCT__ -D_ARM_ASM_ -DQAPI_TXM_MODULE -DTXM_MODULE -DTX_DAM_QC_CUSTOMIZATIONS -DTX_ENABLE_PROFILING -DTX_ENABLE_EVENT_TRACE -DTX_DISABLE_NOTIFY_CALLBACKS -DTX_DAM_QC_CUSTOMIZATIONS 

set DAM_CFLAGS=-O1 --diag_suppress=9931  --cpu=Cortex-A7 --protect_stack --arm_only --apcs=/noswst/interwork
rem --diag_error=warning

set DAM_INCPATHS=-I %DAM_INC_BASE% -I %DAM_INC_BASE%\threadx_api -I %DAM_INC_BASE%\qapi -I %TS_APP_INC_PATH%

set APP_CFLAGS=-DTARGET_THREADX -D__SIMCOM_DAM__
rem Turn on verbose mode by default
rem set -x;

echo "Compiling CustApp application"
%TOOLCHAIN_PATH%\armcc -E -g %DAM_CPPFLAGS% %DAM_CFLAGS% %DAM_SRC_PATH%\txm_module_preamble.S > txm_module_preamble_pp.S

%TOOLCHAIN_PATH%\armcc -g -c %DAM_CPPFLAGS% %DAM_CFLAGS% txm_module_preamble_pp.S -o %TS_APP_OUTPUT_PATH%\txm_module_preamble.o

rem %TOOLCHAIN_PATH%\armcc -E -g -c %DAM_CPPFLAGS% %DAM_CFLAGS% %DAM_SRC_PATH%\txm_module_preamble.S -o %TS_APP_OUTPUT_PATH%\txm_module_preamble.o
del txm_module_preamble_pp.S

@echo on
for /R %TS_APP_SRC_PATH% %%i in (*.c) do %TOOLCHAIN_PATH%\armcc -g -c %DAM_CPPFLAGS% %DAM_CFLAGS% %APP_CFLAGS% %DAM_INCPATHS% %%i 

@echo off
move  *.o %TS_APP_OUTPUT_PATH%
del  %TS_APP_OUTPUT_PATH%\obj.list
for /R %TS_APP_OUTPUT_PATH% %%i in (*.o) do echo %%i >> %TS_APP_OUTPUT_PATH%\obj.list

echo "Linking CustApp application"
@echo on
%TOOLCHAIN_PATH%\armlink --elf --ro %DAM_RO_BASE% --first txm_module_preamble.o -d -o %DAM_OUTPUT_PATH%\%DAM_ELF_NAME%  --entry=_txm_module_thread_shell_entry --map --remove --symbols --list %DAM_OUTPUT_PATH%\dam_app.map  %DAM_LIB_PATH%\%DAM_LIBNAME% %DAM_LIB_PATH%\%TIMER_LIBNAME%  --via %TS_APP_OUTPUT_PATH%\obj.list

%TOOLCHAIN_PATH%\fromelf --bincombined %DAM_OUTPUT_PATH%\%DAM_ELF_NAME% --output %DAM_OUTPUT_PATH%\%DAM_TARGET_BIN%

echo "CustApp application is built at" %DAM_OUTPUT_PATH%\%DAM_TARGET_BIN%
:END
pause
