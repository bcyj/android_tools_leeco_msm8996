#ifndef __STREAMSOURCETIMEUTILS_H__
#define __STREAMSOURCETIMEUTILS_H__
/************************************************************************* */
/**
 * StreamSourceTimeUtils.h
 * @brief Header file for StreamSourceTimeUtils.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/StreamSourceTimeUtils.h#11 $
$DateTime: 2012/09/18 11:58:23 $
$Change: 2815251 $

========================================================================== */
/* =======================================================================
**               Include files for StreamSourceTimeUtils.h
** ======================================================================= */
#include <AEEStdDef.h>
#include <MMTime.h>

/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class StreamSourceTimeUtils
{
public:

  static double ConvertSysTimeToMSec(const MM_Time_DateTime& sSysTime);

  static bool GetUTCTimeInMsecsFromXMLDateTime(const char* pISO8601timeStrBuf,
                                               double &UTCTime);

  static bool IsLeapYear(const int year);

private:
};

#endif  /* __STREAMSOURCETIMEUTILS_H__ */
