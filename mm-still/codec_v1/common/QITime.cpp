/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QITime.h"
#include "QICommon.h"

/*===========================================================================
 * MACROS
 *==========================================================================*/

#define GET_TIME_IN_MS(ts) \
  ((ts->tv_sec * 1000LL) + (ts->tv_nsec / 1000000LL))

#define GET_TIME_IN_US(ts) \
  ((ts->tv_sec * 1000000LL) + (ts->tv_nsec / 1000LL))

/*===========================================================================
 * Function: Start
 *
 * Description: Starts the clock
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QITime::Start()
{
  int lrc = clock_gettime(CLOCK_REALTIME, &mTime);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, lrc);
    return QI_ERR_GENERAL;
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: GetTimeInMilliSec
 *
 * Description: Gets the time in milli seconds from the time the clock is
 *              started
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   time in milliseconds
 *
 * Notes: none
 *==========================================================================*/
uint64_t QITime::GetTimeInMilliSec()
{
  uint64_t lDelta = 0LL;
  struct timespec *lpTs1 = &mTime;
  struct timespec lNow;
  struct timespec *lpTs2 = &lNow;

  int lrc = clock_gettime(CLOCK_REALTIME, &lNow);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, lrc);
    return lDelta;
  }

  lDelta = (GET_TIME_IN_MS(lpTs2) - GET_TIME_IN_MS(lpTs1));
  return lDelta;
}

/*===========================================================================
 * Function: GetTimeInMicroSec
 *
 * Description: Gets the time in micro seconds from the time the clock is
 *              started
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   time in microseconds
 *
 * Notes: none
 *==========================================================================*/
uint64_t QITime::GetTimeInMicroSec()
{
  uint64_t lDelta = 0LL;
  struct timespec *lpTs1 = &mTime;
  struct timespec lNow;
  struct timespec *lpTs2 = &lNow;

  int lrc = clock_gettime(CLOCK_REALTIME, &lNow);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, lrc);
    return lDelta;
  }

  lDelta = (GET_TIME_IN_US(lpTs2) - GET_TIME_IN_US(lpTs1));
  return lDelta;
}
