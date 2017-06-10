/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QITIME_H__
#define __QITIME_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*===========================================================================
 * Class: QITime
 *
 * Description: This class represents the time utility. This class could be
 *              used for profiling
 *
 * Notes: none
 *==========================================================================*/
class QITime {

public:

  /** Start:
   *
   *  starts the timer
   **/
  int Start();

  /** GetTimeInMilliSec:
   *
   *  gets the time in milliseconds from the start
   **/
  uint64_t GetTimeInMilliSec();

  /** GetTimeInMilliSec:
   *
   *  gets the time in milliseconds from the start
   **/
  uint64_t GetTimeInMicroSec();

private:

  /** mTime:
   *
   *  linux timespec object
   **/
  struct timespec mTime;
};

#endif //__QITIME_H__
