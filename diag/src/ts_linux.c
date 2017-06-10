
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

              Diag Legacy Service Mapping DLL

GENERAL DESCRIPTION

Implementation of entry point and Initialization functions for Diag_LSM.dll.


EXTERNALIZED FUNCTIONS
DllMain
Diag_LSM_Init
Diag_LSM_DeInit

INITIALIZATION AND SEQUENCING REQUIREMENTS


# Copyright (c) 2007-2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

$Header:

when       who    what, where, why
--------   ---    ----------------------------------------------------------
10/01/08   SJ     Created

===========================================================================*/

#include "ts_linux.h"
#include "string.h"
#include <sys/time.h>
#include <time.h>
/*===========================================================================

FUNCTION    ts_get

DESCRIPTION
  This extracts time from the system and feeds the pointer passed in.

DEPENDENCIES
  None.

RETURN VALUE
  none

SIDE EFFECTS
  None

===========================================================================*/

void ts_get (void *timestamp)
{
		struct timeval tv;
		char *temp1;
		char *temp2;
		long long int seconds, microseconds;
		int i;

		gettimeofday(&tv, NULL);
		seconds = (long long int)tv.tv_sec;
		/* Offset to sync timestamps between Modem & Apps Proc.
		 Number of seconds between Jan 1, 1970 & Jan 6, 1980 */
		seconds = seconds - (10*365+5+2)*24*60*60 ;
		seconds = seconds * 1000;
		microseconds = (long long int)tv.tv_usec;
		microseconds = microseconds/1000;
		seconds = seconds + microseconds;
		seconds = seconds*4;
		seconds = seconds/5;
		seconds = seconds << 16;
		temp1 = (char *) (timestamp);
		temp2 = (char *) &(seconds);
		/*
		 * This is assuming that you have 8 consecutive
		 * bytes to store the data.
		 */
		for (i=0;i<8;i++)
			*(temp1+i) = *(temp2+i);
}     /* ts_get */

/*===========================================================================
FUNCTION   ts_get_lohi

DESCRIPTION
  Extracts timestamp from system and places into lo and hi parameters

DEPENDENCIES
  None

RETURN VALUE


SIDE EFFECTS
  None

===========================================================================*/
void ts_get_lohi(uint32 *ts_lo, uint32 *ts_hi)
{
	char buf[8];

	ts_get(buf);
	*ts_lo = *(uint32 *)&buf[0];
	*ts_hi = *(uint32 *)&buf[4];

	return;
}

