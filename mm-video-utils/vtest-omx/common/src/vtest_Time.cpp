/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include <sys/time.h>
#include <stdio.h>
#include "vtest_Time.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static long long vt_time_microsec()
{
   struct timeval time;
   long long time_microsec = 0;

   if (gettimeofday(&time, NULL) == 0)
   {
      time_microsec = ((long long) time.tv_sec * 1000 * 1000) + ((long long) time.tv_usec) ;
   }

   return time_microsec;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_TICKS Time::GetTimeMicrosec() {
    return (OMX_TICKS)vt_time_microsec();
}

} // namespace vtest
