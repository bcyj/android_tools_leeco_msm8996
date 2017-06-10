#ifndef _MM_TIME_H_
#define _MM_TIME_H_

/*===========================================================================
                          V i d e o   W r a p p e r
                    f o r   s y s t e m    t i m e    i n f o

@file MMDateTimeUtils.h
  This file defines functions that can be used to obtain and convert date and
  time

 COPYRIGHT 2009-2010, 2012-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/platform/OSAbstraction/main/latest/inc/MMTime.h#3 $
$DateTime: 2010/07/07 18:38:32 $
$Change: 1359407 $

============================================================================*/


/*===========================================================================
 Include Files
============================================================================*/

typedef struct
{
  int m_nYear;
  int m_nMonth;
  int m_nDayOfWeek;
  int m_nDay;
  int m_nHour;
  int m_nMinute;
  int m_nSecond;
  long m_nMilliseconds;

} MM_Time_DateTime;

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
 * Returns the system time (as a 32-bit value), the time elapsed since system was started.
 *
 * @param[out] pTime - time in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Time_GetTime
(
  unsigned long *pTime
);

/**
 * Returns the system time (as a 64-bit value), the time elapsed since system was started.
 *
 * @param[out] pTime - time in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Time_GetTimeEx
(
  unsigned long long *pTime
);

/**
 * @brief
 *  Retrieve the current local date and time.
 *
 * @param[out] pMMDateTime - local time
 *
 * @return zero value is success else failure
 */
int MM_Time_GetLocalTime(MM_Time_DateTime *pMMDateTime);

/**
 *  Returns the current UTC data and time.
 *
 * @param[out] pMMDateTime - UTC time
 *
 * @return zero value is success else failure
 */
int MM_Time_GetUTCTime(MM_Time_DateTime *pMMDateTime);

/**
 * Return the Current Time since Epoch
 *
 * @param[out] pTime - time in millseconds
 *
 * @return zero value is success else failure
 */
int MM_Time_GetCurrentTimeInMilliSecsFromEpoch(unsigned long long *pTime);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif
