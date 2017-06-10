/************************************************************************* */
/**
 * @file StreamSourceTimeUtils.cpp
 * @brief Implementation of StreamSourceTimeUtils.
 *  StreamSourceTimeUtils provides helper routines for time/date/year
 *  conversion routines.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/StreamSourceTimeUtils.cpp#17 $
$DateTime: 2012/09/18 11:58:23 $
$Change: 2815251 $

========================================================================== */
/* =======================================================================
**               Include files for StreamSourceTimeUtils.cpp
** ======================================================================= */
#include "StreamSourceTimeUtils.h"
#include "IPStreamSourceUtils.h"

#include <qtv_msg.h>
#include <oscl_string_utils.h>

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */
const char m_months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May",
                              "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char m_days[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

int m_daysEachMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */

/** @brief This method gets the time from XML in the ISO8601 format with the time zone indicators Z,+,-,
  * (2001-08-05T18:54:30.000Z) or (2001-08-05T18:54:30.000+08:00) and calculates the UTC time in msecs
  *
  * @param[in] pISO8601timeStrBuf - Reference to ISO8601 datetime string
  * @param[out] UTCTime - UTC time in msecs
  *
  * @return
  * TRUE - Conversion successful
  * FALSE - Otherwise
  */

bool StreamSourceTimeUtils::GetUTCTimeInMsecsFromXMLDateTime(const char* pISO8601timeStrBuf, double &UTCTime)

{
  bool bOk = true;
  char *dateStrArray[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  size_t dateStrArraySize = sizeof(dateStrArray) / sizeof(char *);
  int year, month, day, hour, min, sec, msec;
  int offset_index=0;
  int UTCoffsetMsecs = 0;
  year=month=day=hour=min=sec=msec=0;

  char tmpbuf[30];
  std_strlcpy(tmpbuf, pISO8601timeStrBuf, sizeof(tmpbuf));
  IPStreamStringTokenizer dateTimeTokenizer(tmpbuf, "-:.TZ+");

  size_t i = 0;
  char *nextToken = dateTimeTokenizer.GetNextToken();
  while (nextToken && i < dateStrArraySize)
  {
    dateStrArray[i] = nextToken;
    ++i;
    nextToken = dateTimeTokenizer.GetNextToken();
  }

  if (dateStrArray[0])
  {
    year = atoi(static_cast<const char *> (dateStrArray[0]));
    if (year < 1)
    {
      QTV_MSG_PRIO1( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                     "Error: Invalid year field %u", year );
      bOk = false;
    }
  }

  if (bOk && dateStrArray[1])
  {
    month = atoi(static_cast<const char *> (dateStrArray[1]));
    if (month < 1 || month > 12)
    {
      QTV_MSG_PRIO1( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                     "Error: Invalid month field %u", month );
      bOk = false;
    }
  }

  if (bOk && dateStrArray[2])
  {
    day = atoi(static_cast<const char *> (dateStrArray[2]));
    if (day < 1 || day > 31)
    {
      QTV_MSG_PRIO1( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                     "Error: Invalid day field %u", day );
      bOk = false;
    }
  }

  if (bOk && dateStrArray[3])
  {
    hour = atoi(static_cast<const char *> (dateStrArray[3]));

    if (hour > 23)
    {
      QTV_MSG_PRIO1( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                     "Error: Invalid hour field %u", hour );
      bOk = false;
    }
  }

  if (bOk && dateStrArray[4])
  {
    min = atoi(static_cast<const char *> (dateStrArray[4]));

    if (min > 59)
    {
      QTV_MSG_PRIO1( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                     "Error: Invalid minute field %u", min );
      bOk = false;
    }
  }

  if (bOk && dateStrArray[5])
  {
    sec = atoi(static_cast<const char *> (dateStrArray[5]));

    if (sec > 59)
    {
      QTV_MSG_PRIO1( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                     "Error: Invalid second field %u", sec );
      bOk = false;
    }
  }

  //offset_index gives the starting index for UTC offset tokens(ex +10:00). If Msecs field present, offset_index incremented by 1
  offset_index = 6;

  if (bOk && dateStrArray[6])
  {
    const char* pSecStr = pISO8601timeStrBuf + (dateStrArray[5] - dateStrArray[0]);
    if (std_strchr(pSecStr, '.'))
    {
      //Round msec to 3 point accurate value
      msec = (int)(atof(pSecStr) * 1000 + 0.5);
      sec = msec / 1000;
      msec %= 1000;
      offset_index++;
    }
  }

  //UTC offset fields present only when any of the last two tokens in dateStrArray[] are populated.
  //Else we are already in UTC time zone.
  if( bOk && dateStrArray[offset_index])
  {
    int hour_offset=0;
    int min_offset=0;
    const char* pOffsetStr = pISO8601timeStrBuf;
    if (dateStrArray[offset_index] > dateStrArray[0])
    {
      pOffsetStr += dateStrArray[offset_index] - dateStrArray[0] - 1;
    }

    if (std_strchr(pOffsetStr, '+') || std_strchr(pOffsetStr, '-'))
    {
      hour_offset = atoi(static_cast<const char *> (dateStrArray[offset_index]));
      if (dateStrArray[offset_index+1])
      {
        min_offset = atoi(static_cast<const char *> (dateStrArray[offset_index+1]));
      }

      //Hour offset cannot be above 14. If it is 14, min_offset should be 0.
      if ( (hour_offset>14) || ((hour_offset==14)&&(min_offset!=0)) ||
           (min_offset > 59) )
      {
        bOk = false;
      }

      UTCoffsetMsecs = (hour_offset * 3600 + min_offset * 60) * 1000;

      if (std_strchr(pOffsetStr, '+'))
      {
        UTCoffsetMsecs *= -1;
      }
    }
  }

  if(bOk)
  {
    MM_Time_DateTime LocalTime = {0,0,0,0,0,0,0,0};
    LocalTime.m_nYear = year;
    LocalTime.m_nMonth = month;
    LocalTime.m_nDay = day;
    LocalTime.m_nHour = hour;
    LocalTime.m_nMinute = min;
    LocalTime.m_nSecond = sec;
    LocalTime.m_nMilliseconds = msec;

    UTCTime = StreamSourceTimeUtils::ConvertSysTimeToMSec(LocalTime) + UTCoffsetMsecs;

    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "XML date time %s, UTC time in msec %f",
                   pISO8601timeStrBuf, UTCTime );
  }

  return bOk;
}



/** @brief
  * This method checks whether a given year is a leap year or not.
  * @param[in] year - Year to be checked
  *
  * @return
  * TRUE - It is a leap year
  * FALSE - It is not a leap year
  */
bool StreamSourceTimeUtils::IsLeapYear(const int year)
{
  bool bLeapYear = false;
  if(((year%4==0) && (year %100!=0)) || (year%400==0))
  {
    bLeapYear = true;
  }

  return bLeapYear;
}


/** @brief
  * This method converts the time in MM_Time_DateTime format(year,month,day,hour,min,sec,msecs) to Milliseconds.
  * i.e. It gives the total msecs from 0000-0-0:00:00:00 upto the input time.
  *
  * @param[in] sSysTime - Time in MM_Time_DateTime format
  *
  * @return Time in msecs.
  */
double StreamSourceTimeUtils::ConvertSysTimeToMSec(const MM_Time_DateTime& sSysTime)
{
  int year = sSysTime.m_nYear>0 ? (sSysTime.m_nYear-1) : 0;
  int month = sSysTime.m_nMonth>0 ? (sSysTime.m_nMonth-1) : 0;
  int day = sSysTime.m_nDay>0 ? (sSysTime.m_nDay-1) : 0;

  double daysUptoCurrMonth = 0;
  double daysUptoCurrYear = 0;
  double daysInCurrMonth = day;

  //Calculates the total number of days from the start of the year upto the current month
  for(int i=0; i<month; i++)
  {
    daysUptoCurrMonth += m_daysEachMonth[i];
    //If i==1, month is February. If it is a leap year increment daysUptoCurrMonth.
    if(i==1)
    {
      if(IsLeapYear(sSysTime.m_nYear))
      {
        daysUptoCurrMonth++;
      }
    }
  }

  //Calculate the total number of days from 0000-0-0 upto the current year
  //Computing count for leap years and non-leap years separately
  //Count of leap years is given by, all years divisible by 4, but not divisible by 100 or are divisible by 400
  int numLeapYears = year/4 - year/100 + year/400;

  //Exclude the above numLeapYear count to get the count for non-leap years
  daysUptoCurrYear = (numLeapYears)*366 + (year - numLeapYears)*365;

  double nMSec = ((daysUptoCurrYear*24*3600) + (daysUptoCurrMonth*24*3600) + (daysInCurrMonth*24*3600) +
                  (sSysTime.m_nHour*3600) + (sSysTime.m_nMinute*60) + (sSysTime.m_nSecond));
  nMSec = (nMSec * ((double)1000)) + (double)sSysTime.m_nMilliseconds;
  return nMSec;
}
