
/************************************************************************* */
/**
 * @file StreamSourceClock.cpp
 * @brief Implementation of StreamSourceClock.
 *  StreamSourceClock is a wrapper class that acts as a reference clock for
 *  the Stream Source module. Internally it makes use of the ISysTimer for system time.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */

/* =======================================================================
**               Include files for StreamSourceClock.cpp
** ======================================================================= */
#include "StreamSourceClock.h"
#include "qtv_msg.h"

#include "MMTime.h"

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
//uint32 RTPSourceClock::m_nSystemTimeZero = 0;
//ISysTimer* RTPSourceClock::m_piSysTimer = NULL;

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/**
 * @brief This constructor creates an ISysTimer instance and sets the clock.
 * @param piEnv
 * @param result false if unsuccessfull true otherwise
 */
StreamSourceClock::StreamSourceClock(bool& result)
  : m_nSystemTimeZero(0)
{
  //Set system clock offset
  if (MM_Time_GetTime(&m_nSystemTimeZero) == 0)
  {
    result = true;
  }
  else
  {
    result = false;
  }
}

/**
 * This destructor releases the ISysTimer interface.
 */
StreamSourceClock::~StreamSourceClock()
{
}

/** @brief This method returns the current value of the play clock in milliseconds.
  *
  * @return Current clock time
  */
uint32 StreamSourceClock::GetTickCount()
{
  unsigned long timeNow = 0;
  int64 tickCount = 0;

  //Get the current system clock offset
  if (MM_Time_GetTime(&timeNow) == 0)
  {
    tickCount = timeNow;
    //Handle wrap-around
    if (timeNow <= m_nSystemTimeZero)
    {
      tickCount += (static_cast<int64>(1) << 32);
    }
    //Compute offset from m_nSystemTimeZero
    tickCount -= m_nSystemTimeZero;
  }
  else
  {
    tickCount = m_nSystemTimeZero;
  }

  return static_cast<uint32>(tickCount);
}
