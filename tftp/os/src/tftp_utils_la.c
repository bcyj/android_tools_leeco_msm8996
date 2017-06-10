/***********************************************************************
 * tftp_utils_la.c
 *
 * Short description.
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose Description
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2014-07-28   rp    Add debug_info to measure timings.
2014-06-04   rp    Create

===========================================================================*/

#include "tftp_config_i.h"
#include "tftp_comdef.h"
#include "tftp_utils.h"

#if !defined (TFTP_LA_BUILD)
  #error "This file is to be included only in LA builds."
#endif

#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#include "tftp_assert.h"


/*--------------------------------------------------------------------------
  Host-To-Network and Network-To-Host functions.
--------------------------------------------------------------------------*/
uint16
tftp_htons (uint16 hostshort)
{
  return htons (hostshort);
}

uint16
tftp_ntohs (uint16 netshort)
{
  return ntohs (netshort);
}

/*--------------------------------------------------------------------------
  Time Diff functions.
--------------------------------------------------------------------------*/
uint64
tftp_timetick_get (void)
{
  struct timeval tv;
  uint64 ret_time;
  int result;

  result = gettimeofday (&tv, NULL);
  if (result < 0)
  {
    return 0;
  }
  ret_time =  ((uint64)tv.tv_sec * (uint64)1000000) + (uint64)tv.tv_usec;

  return ret_time;
}

uint64
tftp_timetick_diff_in_usec (uint64 start_time, uint64 end_time)
{
  uint64 ret_time_us = 0;

  if (start_time < end_time)
  {
    ret_time_us = (end_time - start_time);
  }
  return ret_time_us;
}

void
tftp_utils_init (void)
{
}


