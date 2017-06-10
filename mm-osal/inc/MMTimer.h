#ifndef MMTIMER_H
#define MMTIMER_H
/*===========================================================================
                          M M    W r a p p e r
                       F o r   T i m e r  S e r v i c e s

*//** @file MMTimer.h
  This file defines provides functions to create waitable timers.

Copyright (c) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/platform/OSAbstraction/main/latest/inc/MMTimer.h#2 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/28/09   rmandal     Converted comments to doxygen style.
07/02/08   gkapalli    Created file.

============================================================================*/

/*===========================================================================
 Include Files
============================================================================*/

// Included MMtime.h as MM_Time_GetTime was moved to this file. Once teams
// outside of streaming migrate to using MMTime.h for MM_Time_GetTime, then
// this include should be removed.
#include "MMTime.h"

/* =======================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Forward declarations
** ----------------------------------------------------------------------- */
#ifndef _MM_HANDLE
typedef void* MM_HANDLE;
#define _MM_HANDLE
#endif


#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
 * Starts timer and returns the handle, when the timer expires the registered
 * handler is invoked
 *
 *
 * @param[in] nTimeOut - timeout in millseconds
 * @param[in] nPeriodic - a non zero value to make the timer periodic
 * @param[in] pfnTimerHandler - timer callback handler to be invoked on timeout
 * @param[in] pvUserArg - optional user data pointer to be passed in the callback
 * @param[out] pHandle - returns a refernce to the handler on success
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
);

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
);

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
  MM_HANDLE      handle,
  int            nTimeOut
);

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
);

/*
 * Stops a timer started using MM_Timer_Start
 *
 * @param[in] pHandle - handle of timer
 *
 * @return zero value is success else failure
 */
int MM_Timer_Stop
(
  MM_HANDLE      handle
);

/**
 * Releases the resources associated with the timer handle
 *
 * @param[in] handle - reference to the timer handle
 *
 * @return zero value on success else failure
 */
int MM_Timer_Release
(
  MM_HANDLE handle
);

/**
 * Sleeps for specified time.
 *
 * @param[in] nSleepTime - Sleep time in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Timer_Sleep
(
  long nSleepTime
);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // MMTIMER_H
