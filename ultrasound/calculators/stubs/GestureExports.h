#ifndef __GESTURE_EXPORTS_H__
#define __GESTURE_EXPORTS_H__

/*============================================================================
                           GestureExports.h

DESCRIPTION:  Function definitions for the Gesture lib (libqcgesture).

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
  Type Definitions
----------------------------------------------------------------------------*/
#define STUB_VERSION "0.0.0.0"

/**
  Gesture library return status
*/
typedef enum
{
  QC_US_GESTURE_LIB_STATUS_SUCCESS     =  0,   /* Success */
  QC_US_GESTURE_LIB_STATUS_FAILURE     =  1,   /* General failure */
  QC_US_GESTURE_LIB_STATUS_BAD_PARAMS  =  2,   /* Bad parameters */
} QcUsGestureLibStatusType;

/**
  Gesture library outcome type
*/
typedef enum
{
  QC_US_GESTURE_LIB_OUTCOME_LEFT     =  1,   /* Left outcome detected */
  QC_US_GESTURE_LIB_OUTCOME_RIGHT    =  2,   /* Right outcome detected */
  QC_US_GESTURE_LIB_OUTCOME_SELECT   =  4,   /* Select outcome detected */
} QcUsGestureLibOutcomeType;

/*----------------------------------------------------------------------------
  Function Declarations
----------------------------------------------------------------------------*/

/*============================================================================
  FUNCTION:  QcGestureAlgorithmGetSizes
============================================================================*/
/**
  Returns the size of the work space needed to the algorithm.
*/
extern void QcGestureAlgorithmGetSizes(int *piSize);

/*============================================================================
  FUNCTION:  QcGestureAlgorithmInit
============================================================================*/
/**
  Init the Gesture algorithm.
*/
extern int QcGestureAlgorithmInit(signed char *piWork,
                                  float *pfMicX,
                                  float *pfMicY,
                                  float *pfMicZ,
                                  float *pfSpeakerX,
                                  float *pfSpeakerY,
                                  float *pfSpeakerZ,
                                  int iMicFrameSize,
                                  int iSpeakerFrameSize);

/*============================================================================
  FUNCTION:  QcGestureAlgorithmTerminate
============================================================================*/
/**
  Terminate the algorithm.
*/
extern void QcGestureAlgorithmTerminate(signed char *piWork);

/*============================================================================
  FUNCTION:  QcGestureAlgorithmEngine
============================================================================*/
/**
  Returms Gesture algorithm calculation.
*/
extern void QcGestureAlgorithmEngine(signed short *piMicSignal,
                                     signed short *piSpeakerSignal,
                                     int *gesture,
                                     int frame_seq_num,
                                     int *bPatternUpdate);

/*============================================================================
  FUNCTION:  QcGestureAlgorithmGetVersion
============================================================================*/
/**
  Returns the Gesture lib version.
*/
extern void QcGestureAlgorithmGetVersion(char *pcVersion,
                                         int *piLen);

#ifdef __cplusplus
}
#endif

#endif //__GESTURE_EXPORTS_H__
