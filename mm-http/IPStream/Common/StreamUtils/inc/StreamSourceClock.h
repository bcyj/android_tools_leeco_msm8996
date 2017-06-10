#ifndef __STREAMSOURCECLOCK_H__
#define __STREAMSOURCECLOCK_H__
/************************************************************************* */
/**
 * StreamSourceClock.h
 * @brief Header file for StreamSourceClock.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */

/* =======================================================================
**               Include files for StreamSourceClock.h
** ======================================================================= */
#include "AEEStdDef.h"

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
class StreamSourceClock
{
public:
  StreamSourceClock(bool& result);
  ~StreamSourceClock();
  uint32 GetTickCount();

private:
  //Member data
  unsigned long m_nSystemTimeZero;
};

#endif  /* __STREAMSOURCECLOCK_H__ */
