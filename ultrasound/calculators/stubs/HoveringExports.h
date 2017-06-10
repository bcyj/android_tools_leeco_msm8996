#ifndef __HOVERING_EXPORTS_H__
#define __HOVERING_EXPORTS_H__

/*============================================================================
                           HoveringExports.h

DESCRIPTION:  Function definitions for the Hovering lib (libqchovering).

Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
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
  Hovering library return status
*/
typedef enum
{
  QC_US_HOVERING_LIB_STATUS_SUCCESS     =  0,   /* Success */
  QC_US_HOVERING_LIB_STATUS_FAILURE     =  1,   /* General failure */
  QC_US_HOVERING_LIB_STATUS_BAD_PARAMS  =  2,   /* Bad parameters */
} QcUsHoveringLibStatusType;

/*----------------------------------------------------------------------------
  Function Declarations
----------------------------------------------------------------------------*/

/*============================================================================
  FUNCTION:  QcUsHoveringLibGetSizes
============================================================================*/
/**
  Returns the size of the work space needed to the algorithm.
*/
extern void QcUsHoveringLibGetSizes(int *piSize);

/*============================================================================
  FUNCTION:  QcUsHoveringLibInit
============================================================================*/
/**
  Init the Hovering algorithm.
*/
extern int QcUsHoveringLibInit(signed char *piWork,
                               int mics_num,
                               float *pfMicX,
                               float *pfMicY,
                               float *pfMicZ,
                               int spkrs_num,
                               float *pfSpeakerX,
                               float *pfSpeakerY,
                               float *pfSpeakerZ,
                               int iMicFrameSize,
                               int iSpeakerFrameSize);

/*============================================================================
  FUNCTION:  QcUsHoveringLibTerminate
============================================================================*/
/**
  Terminate the algorithm.
*/
extern void QcUsHoveringLibTerminate(signed char *piWork);

/*============================================================================
  FUNCTION:  QcUsHoveringLibEngine
============================================================================*/
/**
  Returms Hovering algorithm calculation.
*/
extern void QcUsHoveringLibEngine(signed short *piMicSignal,
                                  signed short *piSpeakerSignal,
                                  float *coordinateX,
                                  float *coordinateY,
                                  float *coordinateZ,
                                  char *coordinateValid,
                                  int iSequenceNum,
                                  int *pbPatternUpdate);

/*============================================================================
  FUNCTION:  QcUsHoveringLibGetVersion
============================================================================*/
/**
  Returns the Hovering lib version.
*/
extern void QcUsHoveringLibGetVersion(char *pcVersion,
                                      int *piLen);

#ifdef __cplusplus
}
#endif

#endif // __HOVERING_EXPORTS_H__
