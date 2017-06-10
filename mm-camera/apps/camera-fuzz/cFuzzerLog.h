#if !defined(_FUZZER_LOG_H_)
#define _FUZZER_LOG_H_

/* ========================================================================= *
    Module:   cFuzzerLog.h
    Date:     12/2/2011
    Author:   cdsilva
    Purpose:  LogModule header file


            -------------------------------------------------------
                Copyright © 2005-2013 Qualcomm Technologies, Inc.
                           All Rights Reserved.
                           Qualcomm Technologies Proprietary


 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>

#if defined(QNX_CFUZZER_BUILD)
	#include <sys/select.h>
	#include <sys/socket.h>
#endif

#if defined(UBUNTU_CFUZZER_BUILD)
	#include <sys/time.h>
	#include "string_utils.h"
	#include <linux/netlink.h>
	#include "unetlink.h"
#endif

#if defined(ANDROID_CFUZZER_BUILD)
	#include <android/log.h>
	#include <linux/netlink.h>
//	#include "unetlink.h"
    #if !defined(LE_BUILD)
		#include <cutils/log.h>
		#include <utils/Log.h>
    #else
        #include <sys/time.h>
        #include "string_utils.h"
    #endif
#endif

#if defined(ANDROID_CFUZZER_BUILD)
	#define FUZZ_LOG_DIR     "/data/cFuzzer/cFuzzerLogs"
	#define FUZZ_SOCK_PATH   "/data/cFuzzer/Sockets"
	#define FUZZ_LOG_CONFIG  "/data/cFuzzer/LogConfig.txt"
#elif defined(QNX_CFUZZER_BUILD)
	#define FUZZ_LOG_DIR     "/developer/qcom/usr/share/cFuzzer/cFuzzerLogs"
	#define FUZZ_SOCK_PATH   "/developer/qcom/usr/share/cFuzzer/Sockets"
	#define FUZZ_LOG_CONFIG  "/developer/qcom/usr/share/cFuzzer/LogConfig.txt"
#elif defined(UBUNTU_CFUZZER_BUILD)
	#define FUZZ_LOG_DIR     "./cFuzzerLogs"
	#define FUZZ_SOCK_PATH   "./Sockets"
	#define FUZZ_LOG_CONFIG  "./LogConfig.txt"
#endif

#define DUMP_LOG_NAME  "DumpLog.txt"
#define FUZZ_LOG_NAME1 "FuzzerLog1.txt"
#define FUZZ_LOG_NAME2 "FuzzerLog2.txt"
#define FUZZ_SOCK_NAME "FuzzerLogSocket"

/* ---------------------------------------------------------------------------- *
                                     Defines
 * ---------------------------------------------------------------------------- */

#if defined(ANDROID_CFUZZER_BUILD)
	#define FUZZER_TAG "CFUZZER"
	#define ADBLOG(MSG) __android_log_print(ANDROID_LOG_INFO, FUZZER_TAG, "%s", MSG)
#endif

#define __SFILE__  (strrchr(__FILE__,'/')?strrchr(__FILE__,'/')+1:__FILE__)

// Send Msg to Server
#define DBGPRT(lev, fmt,...)   LogClientSendMsg(&cFL,lev,__SFILE__,__FUNCTION__,__LINE__,(char *)fmt, ##__VA_ARGS__)
#define DBGPRTP(lev, fmt,...)  LogClientSendMsg(cFL,lev, __SFILE__,__FUNCTION__,__LINE__,(char *)fmt, ##__VA_ARGS__)
#define DBGPRTX(lev, fmt,...) LogClientSendMsg(cFL,lev,fil,fun,lin,(char *)fmt, ##__VA_ARGS__)

#define LOGPRT(lev,fmt,...)   LogPrt(cFL, lev, __SFILE__,__FUNCTION__,__LINE__,(char *)fmt, ##__VA_ARGS__)
#define LOGPRTX(lev,fmt,...)  LogPrt(cFL, lev, __SFILE__,__FUNCTION__,__LINE__,(char *)fmt, ##__VA_ARGS__)

#if !defined(COLORS_DEFINED)
	#define CFUZZER_cyanB   "\033[1;36m"    /* 1 -> bold      ; 36 -> cyan */
	#define CFUZZER_blueS   "\033[9;34m"    /* 9 -> strike    ; 34 -> blue */
	#define CFUZZER_greenU  "\033[4;32m"    /* 4 -> underline ; 32 -> green */

	#define CFUZZER_red     "\033[0;31m"    /* 0 -> normal    ; 31 -> red */
	#define CFUZZER_cyan    "\033[0;36m"    /* 0 -> normal    ; 36 -> cyan */
	#define CFUZZER_green   "\033[0;32m"    /* 0 -> normal    ; 32 -> green */
	#define CFUZZER_blue    "\033[1;34m"    /* 0 -> normal    ; 34 -> blue */
	#define CFUZZER_black   "\033[0;30m"    /* 0 -> normal    ; 30 -> black */
	#define CFUZZER_brown   "\033[0;33m"    /* 0 -> normal    ; 33 -> brown */
	#define CFUZZER_magenta "\033[0;35m"    /* 0 -> normal    ; 35 -> magenta */
	#define CFUZZER_grey    "\033[0;37m"    /* 0 -> normal    ; 37 -> grey */

	#define CFUZZER_bold    "\033[1;30m"    /* 1 -> bold      ; 30 -> black */
	#define CFUZZER_under   "\033[4;30m"    /* 1 -> underline ; 30 -> black */

	#define CFUZZER_none    "\033[0m"       /* flush previous property */
#endif

enum
{
	SOCK_NAME_SIZE=256,
} ;

typedef enum
{
	LOG_SERVER,
	LOG_CLIENT
} log_socket_type_t ;

typedef enum
{
	LEVEL_NONE,
	LEVEL_ERROR,
	LEVEL_WARN,
	LEVEL_INFO,
	LEVEL_DEBUG,
	LEVEL_INIT
} fuzzer_log_level_t ;

enum
{
	FUZZ_LOG_NAME_SIZE=128,
	FUZZ_LOG_DIR_SIZE=128
} ;

enum
{
	FUZZ_LIB_NAME_SIZE=64,
	FUZZ_MAX_LINE_SIZE=128

} ;

enum
{
	FUZZER_LOG_BUF_SIZE=256,
	TIME_BUF_SIZE=64
} ;

typedef enum
{
	LOG_FILE_NEITHER,
	LOG_FILE_ONE,
	LOG_FILE_TWO
} log_file_open_t ;

typedef enum
{
	TO_STDOUT,
	TO_ADB,
	TO_DIAG,
	TO_LOG
} log_dest_t ;

#define MAX_LOG_SIZE  0x100000

typedef enum
{
	DUMP_FIRST,
	DUMP_TO_BIT_BUCKET,
	DUMP_TO_CONSOLE,
	DUMP_TO_FILE,
	DUMP_LAST
} dump_dest_t;

#define KPDIR  "/sys/kernel/debug/keel"
#define KEE_LOG_ENABLE KPDIR"/LogEnable"
#define KPRINT KPDIR"/LogPrint"

/* ---------------------------------------------------------------------------- *
                                   Global Vars
 * ---------------------------------------------------------------------------- */

typedef struct _CFUZZER_LOG
{
	int                logSize ;
	int                logMaxSize ;
	log_dest_t         logDest ;
	fuzzer_log_level_t logLevel ;
	char               logDir[FUZZ_LOG_DIR_SIZE] ;
	char               logName[FUZZ_LOG_NAME_SIZE] ;
	FILE              *logFile ;
	char              *logRBuf ;
	char              *logWBuf ;
	char               libName[FUZZ_LIB_NAME_SIZE] ;
	log_file_open_t    openFile ;
	pthread_t          logThread ;

	bool               errorOccurred ;
	dump_dest_t        dumpOption ;
	FILE              *dumpFile ;
	bool               dumpFileOpen ;

	pthread_mutex_t    printMutex ;
	pthread_mutex_t    sendMutex ;

	bool        kernelMode ;       // enable kernel printing to log
	bool        kernelPrint ;      // enable kee to print INFO msgs to syslog or dmesg
	char        kernelPBuf[FUZZ_MAX_LINE_SIZE] ;
	bool        kernelLogEnable ;  // enable KEELOG msgs to /kee_...
	struct _CFUZZER_LOG  *thisPt ;

	// for sockets
	int   xFD ;         // for sending info to log socket
	int   rFD ;         // for reading from log socket

	int   kFD ;         // Kernel NetLink Socket

	log_socket_type_t  sockType ;

} cFuzzerLog ;

/* ---------------------------------------------------------------------------- *
                                    Prototypes
 * ---------------------------------------------------------------------------- */

// Methods
bool LogInit(cFuzzerLog *cFL, log_dest_t dst, char *name) ;
bool LogServerSetup(cFuzzerLog *cFL) ;
bool LogClientSetup(cFuzzerLog *cFL, const char *name) ;

bool LogSetup(cFuzzerLog *cFL, log_dest_t dst) ;
void LogDeinit(cFuzzerLog *cFL) ;

bool  SetupLogging(cFuzzerLog *cFL) ;
bool  OpenLogFile(cFuzzerLog *cFL) ;
bool  SetupAdb(cFuzzerLog *cFL) ;
bool  SetupDiag(cFuzzerLog *cFL) ;
char *GetTimeStamp(char *tbuffer, struct timeval *t) ;

bool  LogPrt(cFuzzerLog *cFL, fuzzer_log_level_t level, const char *file, const char *fun, int line, char *format, ...) ;
bool  LogServerPrintMsg(cFuzzerLog *cFL, char *format, ...) ;
bool  LogClientSendMsg(cFuzzerLog *cFL,  fuzzer_log_level_t level, const char *file, const char *fun, int line, char *format, ...) ;

bool  SetupReceiveSock(cFuzzerLog *cFL) ;
bool  SetupXmitSock(cFuzzerLog *cFL) ;
int   GetLogLevel(cFuzzerLog *cFL, char *str) ;

void *cFuzzerLogThread(void *arg) ;

bool  OpenDumpFile(cFuzzerLog *cFL) ;

// in cFuzzerMain
extern void  GetKernelState(cFuzzerLog *) ;

#endif

