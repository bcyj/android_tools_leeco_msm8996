/*===========================================================================
                          V i d e o   T i m e r
               f o r   S i g n a l  Q u e u e  O b j e c t

*//** @file MMTimerPosix.c
  This file defines provides functions to create waitable timers.

 COPYRIGHT 2012-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Common/OSAbstraction/POSIX/main/latest/src/VideoTimerPosix.c#1 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/28/09   rmandal    Created file.

============================================================================*/

/*===========================================================================
 Include Files
============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "MMTimer.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include "MMCriticalSection.h"
#include "MMDebugMsg.h"


#include <time.h>
#include <string.h>
#include "AEEStdDef.h"
#include <cutils/properties.h>
#include <time_genoff.h>

/* =======================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Forward declarations
** ----------------------------------------------------------------------- */

#define TIME_NSEC_IN_MSEC 1000000
#define TIME_MSEC_IN_SEC 1000
#define TIME_MICROSEC_IN_MSEC 1000

typedef struct _video_Timer MM_Timer;

/* -----------------------------------------------------------------------
** Type declarations
** ----------------------------------------------------------------------- */

struct _video_Timer {
    /*
     * timer callback handler to be invoked on timeout
     */
    void         (*m_pfnTimerHandler)(void* );
    /*
     * optional user data pointer to be passed in the call  back
     */
    void          *m_pvUserArg;

    /*
     * Lock to synchronize the callback and release
     */
    MM_HANDLE   m_timerLock;

    /*
     *  Timer struct - timer_t
     */
    timer_t       m_timerId;

    /*
     * non-zero indicates if the timer is periodic
     */
    int m_nPeriodic;

};

 /*
  * Callback function registered with REX for the timer events which in turn
  * calls the user defined call back registered when the timer was created
  *
  * @param[in] time_elapsed_ms - Time elapsed from timer setting (start) in milliseconds
  * @param[in] user_data - data to pass to user callback
  */
void video_Timer_Callback(sigval_t user_data)
{
    MM_Timer *pTimer = (MM_Timer *)(user_data.sival_ptr);

    if(pTimer)
    {
        MM_CriticalSection_Enter( pTimer->m_timerLock );
        pTimer->m_pfnTimerHandler( pTimer->m_pvUserArg );
        MM_CriticalSection_Leave( pTimer->m_timerLock );
    }

    return;
}

/*
 * Starts timer and returns the handle, when the timer expires the registered
 * invoked
 *
 *
 * @param[in] nTimeOut - timeout in millseconds
 * @param[in] nPeriodic - a non zero value to make the timer periodic
 * @param[in] pfnTimerHandler - timer callback handler to be invoked on timeout
 * @param[in] pvUserArg - optional user data pointer to be passed in the callback
 * @param[out] pHandle - timer callback handler to be invoked on timeout
 *
 * @return zero value is success else failure
 */
int MM_Timer_Create
(
    int            nTimeOut,
    int            nPeriodic,
    void         (*pfnTimerHandler)(void* ),
    void          *pvUserArg,
    MM_HANDLE  *pHandle
)
{
    int nResult = MM_Timer_CreateEx( nPeriodic, pfnTimerHandler, pvUserArg,
                                 pHandle );
    if ( nResult == 0 )
    {
        nResult = MM_Timer_Start( *pHandle, nTimeOut );
        if ( nResult != 0 )
        {
            (void)MM_Timer_Release( *pHandle );
            *pHandle = NULL;
        }
    }

    return nResult;
}

/*
 * Creates a timer and returns the handle
 *
 * @param[in] nPeriodic - a non zero value to make the timer periodic
 * @param[in] pfnTimerHandler - timer callback handler to be invoked on timeout
 * @param[in] pvUserArg - optional user data pointer to be passed in the callback
 * @param[out] pHandle - timer callback handler to be invoked on timeout
 *
 * @return zero value is success else failure
 */
int MM_Timer_CreateEx
(
  int            nPeriodic,
  void         (*pfnTimerHandler)(void* ),
  void          *pvUserArg,
  MM_HANDLE     *pHandle
)
{
    int nResult = 1; //failure

    MM_Timer *pTimer = 0;

    if ( pHandle && pfnTimerHandler )
    {
        pTimer = MM_Malloc( sizeof(MM_Timer) );

        if ( pTimer )
        {
            memset(pTimer, 0, sizeof(MM_Timer));

            pTimer->m_pfnTimerHandler = pfnTimerHandler;
            pTimer->m_pvUserArg = pvUserArg;
            pTimer->m_nPeriodic = nPeriodic;

            if ( MM_CriticalSection_Create(&pTimer->m_timerLock) == 0 )
            {
                struct sigevent ev;
                ev.sigev_notify = SIGEV_THREAD;
                ev.sigev_notify_function = video_Timer_Callback;
                ev.sigev_notify_attributes = NULL;
                ev.sigev_value.sival_ptr = pTimer;

                nResult = timer_create(CLOCK_MONOTONIC, &ev, &pTimer->m_timerId);
                if (nResult == 0)
                {
                   nResult = 0;
                   *pHandle = pTimer;
                }
                else
                {
                   MM_CriticalSection_Release(&pTimer->m_timerLock);
                   MM_Free(pTimer);
                }
            }
            else
            {
                MM_Free(pTimer);
            }
        }
    }

    return nResult;
}

/*
 * Starts a timer created using MM_Timer_CreateEx
 *
 * @param[in] pHandle - handle of timer
 * @param[in] nTimeOut - timeout in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Timer_Start
(
  MM_HANDLE     handle,
  int           nTimeOut
)
{
    int nResult = 1;
    if (handle)
    {
        struct itimerspec t;
        memset(&t, 0, sizeof(t));
        MM_Timer *pTimer = (MM_Timer *) handle;
        MM_CriticalSection_Enter( pTimer->m_timerLock );
        t.it_value.tv_sec = nTimeOut / TIME_MSEC_IN_SEC;
        t.it_value.tv_nsec = (nTimeOut % TIME_MSEC_IN_SEC) * TIME_NSEC_IN_MSEC;

        if (pTimer->m_nPeriodic != 0)
        {
            t.it_interval.tv_sec = t.it_value.tv_sec;
            t.it_interval.tv_nsec = t.it_value.tv_nsec;
        }

        nResult = timer_settime(pTimer->m_timerId, 0, &t, NULL);
        MM_CriticalSection_Leave( pTimer->m_timerLock );
    }
    return nResult;
}

/*
 * Starts a timer created using MM_Timer_CreateEx
 *
 * @param[in] pHandle - handle of timer
 * @param[in] nTimeOutSec - timeout in Seconds
 * @param[in] nTimeOutnSec - timeout nano Seconds
 *
 * @return zero value is success else failure
 */
int MM_Timer_StartEx
(
  MM_HANDLE     handle,
  int           nTimeOutSec,
  int           nTimeOutNSec
)
{
    int nResult = 1;
    if (handle)
    {
        struct itimerspec t;
        memset(&t, 0, sizeof(t));
        MM_Timer *pTimer = (MM_Timer *) handle;
        MM_CriticalSection_Enter( pTimer->m_timerLock );
        t.it_value.tv_sec = nTimeOutSec;
        t.it_value.tv_nsec = nTimeOutNSec;

        if (pTimer->m_nPeriodic != 0)
        {
            t.it_interval.tv_sec = t.it_value.tv_sec;
            t.it_interval.tv_nsec = t.it_value.tv_nsec;
        }

        nResult = timer_settime(pTimer->m_timerId, 0, &t, NULL);
        MM_CriticalSection_Leave( pTimer->m_timerLock );
    }
    return nResult;
}

/*
 * Stops a timer started using MM_Timer_Start
 *
 * @param[in] pHandle - handle of timer
 *
 * @return zero value is success else failure
 */
int MM_Timer_Stop
(
  MM_HANDLE     handle
)
{
    int nResult = 1;
    if (handle)
    {
        struct itimerspec its;
        memset(&its, 0, sizeof(its));
        MM_Timer *pTimer = (MM_Timer *) handle;
        MM_CriticalSection_Enter( pTimer->m_timerLock );
        /* Stop the timer */
        its.it_value.tv_sec = 0;
        its.it_value.tv_nsec = 0;
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
        nResult = timer_settime(pTimer->m_timerId, 0, &its, NULL);
        MM_CriticalSection_Leave( pTimer->m_timerLock );
    }
    return nResult;
}

/*
 * There are no further refernces to the object mark for deletion
 *
 * @param[in] handle - reference to the timer handle
 *
 * @return zero value on success else failure
 */
int MM_Timer_Release
(
    MM_HANDLE handle
)
{
    int nResult = 1; // failure
    MM_Timer *pTimer = (MM_Timer *)handle;

    if ( pTimer )
    {
        MM_CriticalSection_Enter(pTimer->m_timerLock);

        timer_delete(pTimer->m_timerId);

        MM_CriticalSection_Leave(pTimer->m_timerLock);

        MM_CriticalSection_Release(pTimer->m_timerLock);

        MM_Free( handle );

        nResult = 0;
    }

    return nResult;
}

/*
 * Returns the system time, the time elapsed since system was started.
 *
 * @param[out] pTime - time in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Time_GetTime
(
   unsigned long *pTime
)
{
    int nResult = 1; // failure

    if ( pTime )
    {
        struct timespec t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        *pTime = (t.tv_sec * TIME_MSEC_IN_SEC) + (t.tv_nsec / TIME_NSEC_IN_MSEC);

        nResult = 0;;
    }

    return nResult;
}

/*
 * Returns the system time (as a 64-bit value), the time elapsed since system was started.
 *
 * @param[out] pTime - time in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Time_GetTimeEx
(
  unsigned long long *pTime
)
{
    int nResult = 1; // failure

    if ( pTime )
    {
        struct timespec t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        *pTime = (t.tv_sec * TIME_MSEC_IN_SEC) + (t.tv_nsec / TIME_NSEC_IN_MSEC);

        nResult = 0;;
    }

    return nResult;
}


/*
 * Sleeps for specified time.
 *
 * @param[in] nSleepTime - Sleep time in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Timer_Sleep
(
    long nSleepTime
)
{
    int nResult = 1; // Failure
    struct timespec t;
    t.tv_sec = nSleepTime / TIME_MSEC_IN_SEC;
    t.tv_nsec = (nSleepTime % TIME_MSEC_IN_SEC) * TIME_NSEC_IN_MSEC;
    nResult = nanosleep(&t, NULL);
    return nResult;
}
/**
 * @brief
 *  Retrieve the current local date and time.
 *
 * @param[out] pMMDateTime - local time
 *
 * @return zero value is success else failure
 */
int MM_Time_GetLocalTime(MM_Time_DateTime *pMMDateTime)
{
  int ret = 1;
  if (pMMDateTime)
  {
    struct tm now;
    struct timeval  tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    localtime_r(&tv.tv_sec, &now);

    pMMDateTime->m_nYear = now.tm_year + 1900;
    pMMDateTime->m_nMonth = now.tm_mon + 1;
    pMMDateTime->m_nDay = now.tm_mday;
    pMMDateTime->m_nDayOfWeek = now.tm_wday;
    pMMDateTime->m_nHour = now.tm_hour;
    pMMDateTime->m_nMinute = now.tm_min;
    pMMDateTime->m_nSecond = now.tm_sec;
    pMMDateTime->m_nMilliseconds = tv.tv_usec / 1000;

    ret = 0;
  }

  return ret;

}

/**
 *  Returns the current UTC data and time.
 *
 * @param[out] pMMDateTime - UTC time
 *
 * @return zero value is success else failure
 */
int MM_Time_GetUTCTime(MM_Time_DateTime *pMMDateTime)
{
  int ret = 1;


  if (pMMDateTime)
  {
     memset(pMMDateTime , 0x00, sizeof(MM_Time_DateTime));
     struct tm now;
     int isSIB16Enabled = 0;
     char value[PROPERTY_VALUE_MAX];
     property_get("persist.radio.sib16_support", value, "0");
     isSIB16Enabled = atoi(value);

     if(isSIB16Enabled > 0)
     {
       uint64 timeval = 0;
       time_genoff_info_type timeGenOffInfo;
       timeGenOffInfo.base = ATS_UTC;
       timeGenOffInfo.unit = TIME_MSEC;
       timeGenOffInfo.operation = T_GET;
       timeGenOffInfo.ts_val = &timeval;
       if(!time_genoff_operation(&timeGenOffInfo))
       {
         time_t t = (time_t)(timeval/1000);
         gmtime_r(&t, &now);

         pMMDateTime->m_nYear = now.tm_year + 1900;
         pMMDateTime->m_nMonth = now.tm_mon + 1;
         pMMDateTime->m_nDay = now.tm_mday;
         pMMDateTime->m_nDayOfWeek = now.tm_wday;
         pMMDateTime->m_nHour = now.tm_hour;
         pMMDateTime->m_nMinute = now.tm_min;
         pMMDateTime->m_nSecond = now.tm_sec;
         pMMDateTime->m_nMilliseconds = (long)(timeval%1000);

         ret = 0;
       }
       else
       {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "SIB16 Enabled but time_genoff_operation failed");
       }
     }
     if (ret != 0)
     {
       struct timeval  tv;
       struct timezone tz;
       gettimeofday(&tv, &tz);
       gmtime_r(&tv.tv_sec, &now);

       pMMDateTime->m_nYear = now.tm_year + 1900;
       pMMDateTime->m_nMonth = now.tm_mon + 1;
       pMMDateTime->m_nDay = now.tm_mday;
       pMMDateTime->m_nDayOfWeek = now.tm_wday;
       pMMDateTime->m_nHour = now.tm_hour;
       pMMDateTime->m_nMinute = now.tm_min;
       pMMDateTime->m_nSecond = now.tm_sec;
       pMMDateTime->m_nMilliseconds = tv.tv_usec / 1000;

       ret = 0;
     }
  }

  return ret;
}


/**
 * Return the Current Time since Epoch
 *
 * @param[out] pTime - time in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Time_GetCurrentTimeInMilliSecsFromEpoch(unsigned long long *pTime)
{
  int ret = 1;

  if (pTime)
  {
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    *pTime = ((unsigned long long)tv.tv_sec * TIME_MSEC_IN_SEC) +
             ((unsigned long long)tv.tv_usec / TIME_MICROSEC_IN_MSEC );
    ret = 0;
  }
  return ret;
}


