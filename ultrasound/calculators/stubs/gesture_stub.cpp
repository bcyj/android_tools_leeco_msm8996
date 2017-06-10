/*===========================================================================
                           gesture_stub.cpp

DESCRIPTION: Provide stub to Gesture lib.


INITIALIZATION AND SEQUENCING REQUIREMENTS: None


Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "Gesture_stub"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include "GestureExports.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define STUB_MSG_INTERVAL 5000
/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*============================================================================
  FUNCTION:  QcGestureAlgorithmGetSizes
============================================================================*/
/**
  This function returns 0 in the piSize.
*/
extern void QcGestureAlgorithmGetSizes(int *piSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);

  *piSize = 0;
}

/*============================================================================
  FUNCTION:  QcGestureAlgorithmInit
============================================================================*/
/**
  This function do nothing and returns success.
*/
extern int QcGestureAlgorithmInit(signed char *piWork,
                                  float *pfMicX,
                                  float *pfMicY,
                                  float *pfMicZ,
                                  float *pfSpeakerX,
                                  float *pfSpeakerY,
                                  float *pfSpeakerZ,
                                  int iMicFrameSize,
                                  int iSpeakerFrameSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);

  return QC_US_GESTURE_LIB_STATUS_SUCCESS;
}

/*============================================================================
  FUNCTION:  QcGestureAlgorithmTerminate
============================================================================*/
/**
  This function do nothing.
*/
extern void QcGestureAlgorithmTerminate(signed char *piWork)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  QcGestureAlgorithmEngine
============================================================================*/
/**
  This function prints stub msg every STUB_MSG_INTERVAL
  times the function is called.
*/
extern void QcGestureAlgorithmEngine(signed short *piMicSignal,
                                     signed short *piSpeakerSignal,
                                     int *gesture,
                                     int frame_seq_num,
                                     int *bPatternUpdate)
{
  static int print_stub_msg_counter = STUB_MSG_INTERVAL;
  if (STUB_MSG_INTERVAL == print_stub_msg_counter)
  {
    LOGW("%s: Stub.",
         __FUNCTION__);
    print_stub_msg_counter = 0;
  }
  else
  {
    print_stub_msg_counter++;
  }
}

/*============================================================================
  FUNCTION:  QcGestureAlgorithmGetVersion
============================================================================*/
/**
  This function gets buffer of 256 bytes, pcVersion,
  and returns string of stub version inside pcVersion.
  It returns the size of the stub version string in the piLen.
*/
extern void QcGestureAlgorithmGetVersion(char *pcVersion,
                                         int *piLen)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  strlcpy(pcVersion,
          STUB_VERSION,
          *piLen);
  *piLen = sizeof(STUB_VERSION);
}
