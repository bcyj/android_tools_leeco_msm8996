#ifndef DALSYS_COMMON_FUNCTIONS_H
#define DALSYS_COMMON_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
  @file DALSYS_common.h
  @brief  DALSYS wrapper for LA
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/DALSYS_common.h#1 $
$DateTime: 2011/04/05 20:05:46 $
$Author: zhongl $

Revision History:
                            Modification     Tracking
Author (core ID)                Date         CR Number   Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
ZhongL                      02/15/2011                    File creation.



====================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "comdef.h"
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/*==================================================================================================
                                           CONSTANTS
==================================================================================================*/



/*==================================================================================================
                                            MACROS
==================================================================================================*/

typedef int     DALResult;

/* -----------------------------------------------------------------------
** Standard Types
** ----------------------------------------------------------------------- */

/* The following definitions are the same accross platforms.  This first
** group are the sanctioned types.
*/

typedef uint32  DALBOOL;
typedef uint32  DALHandle;
typedef void  * DALEnvHandle;
typedef void  * DALSYSEventHandle;
typedef void  * DALSYSSyncHandle;
typedef void    DALSYSSyncObj;
typedef void    DALSYSEventObj;


#ifndef TRUE
#define TRUE   1   /* Boolean true value. */
#endif

#ifndef FALSE
#define FALSE  0   /* Boolean false value. */
#endif

#ifndef NULL
#define NULL  0
#endif


#define  DAL_SUCCESS                      0  // No error
#define  DAL_ERROR                       -1  // General failure



#define DALSYS_memset( pDest, val, size ) \
    ((void)memset((void *)pDest,val,size))

#define DALSYS_memcpy( pDest, pSrc, len ) \
    ((void)memcpy((void *)pDest,(void*)pSrc,len))

/*
// This is for logcat support.
#include "cutils/properties.h"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_TAG "AUDIOFTM"
#define LOG_PRI(priority, tag, ...)                                     \
    ({                                                                  \
       if (((priority == ANDROID_LOG_VERBOSE) && (LOG_NDEBUG == 0)) ||  \
           ((priority == ANDROID_LOG_DEBUG) && (LOG_NDDEBUG == 0))  ||  \
           ((priority == ANDROID_LOG_INFO) && (LOG_NIDEBUG == 0))   ||  \
            (priority == ANDROID_LOG_WARN)                          ||  \
            (priority == ANDROID_LOG_ERROR)                         ||  \
            (priority == ANDROID_LOG_FATAL))                            \
                (void)android_printLog(priority, tag, __VA_ARGS__);     \
    })
#include "cutils/log.h"

#define DALSYS_Log_Info(...)  LOGV(__VA_ARGS__)
#define DALSYS_Log_Err(...)   LOGE(__VA_ARGS__)
*/

#define DALSYS_Log_Info(fmt, ...)  printf("FTM: %s:%d - " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define DALSYS_Log_Err(fmt, ...)   printf("FTM: %s:%d - " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define DAL_ATOMIC  int32
#define DALSYS_atomic_init(x)  (*x=0)
#define DALSYS_atomic_read(x)  (*x)
#define AddRef(v)   (*v=*v+1)
#define Release(v)  (*v=*v-1)

/*  constant macro for DALSYS compatability */
#define DALSYS_SYNC_ATTR_RESOURCE           0x00000010
#define DALSYS_SYNC_ATTR_RESOURCE_INTERRUPT 0x00000020
#define DALSYS_SYNC_ATTR_IRQLEVEL           0x80000000
#define DALSYS_SYNC_ATTR_NO_PREEMPTION      0x00000040
#define DALSYS_SYNC_ATTR_HARD_NO_PREEMPTION 0x00000080

#define DALSYS_EVENT_ATTR_NORMAL            0x00000000
#define DALSYS_EVENT_ATTR_WORKLOOP_EVENT    0x00000010
#define DALSYS_EVENT_ATTR_CALLBACK_EVENT    0x00000020
#define DALSYS_EVENT_ATTR_TIMEOUT_EVENT     0x00000040
#define DALSYS_EVENT_ATTR_TIMER_EVENT       0x00000080
#define DALSYS_EVENT_ATTR_CLIENT_DEFAULT    0x00000100


typedef enum AFEDevAudIfDirInfo
{
    AUDIO_IF_DIR_INVALID = -1,
    AUDIO_IF_SINK = 0 ,
    AUDIO_IF_SOURCE
} AFEDevAudIfDirType;


/*==================================================================================================
                                             ENUMS
==================================================================================================*/


/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/




/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATION
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

DALSYS_Malloc(uint32 dwSize, void **ppMem);

DALResult
DALSYS_Free(void *pmem);



DALResult
DALSYS_SyncCreate(uint32 dwAttribs,
                  DALSYSSyncHandle *phSync,
                  DALSYSSyncObj *pObj);

void
DALSYS_SyncEnter(DALSYSSyncHandle hSync);

void
DALSYS_SyncLeave(DALSYSSyncHandle hSync);


DALResult
DALSYS_EventCreate(uint32 dwEvtAttrib,  DALSYSEventHandle *phEvent,
                                   DALSYSEventObj *pObj);

DALResult
DALSYS_EventCtrl(DALSYSEventHandle hEvent, uint32 dwCtrl,uint32 dwParam,
                                  void *pPayload, uint32 dwPayloadSize);

DALResult
DALSYS_EventWait(DALSYSEventHandle hEvent);

DALResult
DALSYS_EventWaitTimeout(DALSYSEventHandle hEvent, uint32 timeout);

DALResult
DALSYS_DestroySyncObject(DALSYSSyncHandle hSync);

DALResult
DALSYS_DestroyEventObject(DALSYSEventHandle hEvent);


void DALSYS_Delay(uint32 delay_ms);


/*=================================================================================================*/
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* DALSYS_COMMON_FUNCTIONS_H */

