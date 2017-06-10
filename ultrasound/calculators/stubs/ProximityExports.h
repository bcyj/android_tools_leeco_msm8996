#ifndef __PROXIMITY_EXPORTS_H__
#define __PROXIMITY_EXPORTS_H__

/*============================================================================
                           ProximityExports.h

DESCRIPTION:  Function definitions for the Proximity lib (libqcproximity).

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------------*/
#define STUB_VERSION "0.0.0.0"

// PROX RESULTS
#define RES_NO_PROX       1
#define RES_PROX_DETECTED 2
#define RES_COVERED       3

/*----------------------------------------------------------------------------
  Function Declarations
----------------------------------------------------------------------------*/

/*============================================================================
  FUNCTION:  QcUsProximityLibGetSizes
============================================================================*/
/**
  Returns the size of the work space needed to the algorithm.
*/
extern int QcUsProximityLibGetSizes();

/*============================================================================
  FUNCTION:  QcUsProximityLibInit
============================================================================*/
/**
  Init the Proximity algorithm.
*/
extern int QcUsProximityLibInit(signed char *piWork,
                                int *cfg);

/*============================================================================
  FUNCTION:  QcUsProximityLibUpdatePattern
============================================================================*/
extern int QcUsProximityLibUpdatePattern(int16_t *patterns,
                                         int length);

/*============================================================================
  FUNCTION:  QcUsProximityLibInit
============================================================================*/
/**
  Init the Proximity algorithm.
*/
extern int QcUsProximityLibEngine(short *data);

#ifdef __cplusplus
}
#endif

#endif //__PROXIMITY_EXPORTS_H__
