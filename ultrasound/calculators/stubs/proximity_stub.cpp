/*===========================================================================
                           proximity_stub.cpp

DESCRIPTION: Provide stub to Proximity library


INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#include<ProximityExports.h>

/*============================================================================
  FUNCTION:  QcUsProximityLibGetSizes
============================================================================*/
/**
  Returns the size of the work space needed to the algorithm.
*/
extern int QcUsProximityLibGetSizes()
{
  return 0;
}

/*============================================================================
  FUNCTION:  QcUsProximityLibUpdatePattern
============================================================================*/
extern int QcUsProximityLibUpdatePattern(int16_t *patterns,
                                         int length)
{
  return 0;
}

/*============================================================================
  FUNCTION:  QcUsProximityLibInit
============================================================================*/
/**
  Init the Proximity algorithm.
*/
extern int QcUsProximityLibInit(signed char *piWork, int *cfg)
{
  return 0;
}

extern int  QcUsProximityLibEngine(short *data)
{
  return 0;
}
