/*===========================================================================
                           hovering_stub.cpp

DESCRIPTION: Provide stub to Hovering lib.


INITIALIZATION AND SEQUENCING REQUIREMENTS: None


Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "Hovering_stub"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include "HoveringExports.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define STUB_MSG_INTERVAL 5000

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*============================================================================
  FUNCTION:  QcUsHoveringLibGetSizes
============================================================================*/
/**
  This function returns 0 in the piSize.
*/
extern void QcUsHoveringLibGetSizes(int *piSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);

  *piSize = 0;
}

/*============================================================================
  FUNCTION:  QcUsHoveringLibInit
============================================================================*/
/**
  This function do nothing and returns success.
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
                               int iSpeakerFrameSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);

  return QC_US_HOVERING_LIB_STATUS_SUCCESS;
}

/*============================================================================
  FUNCTION:  QcUsHoveringLibTerminate
============================================================================*/
/**
  This function do nothing.
*/
extern void QcUsHoveringLibTerminate(signed char *piWork)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  QcUsHoveringLibEngine
============================================================================*/
/**
  This function prints stub msg every STUB_MSG_INTERVAL
  times the function is called.
*/
extern void QcUsHoveringLibEngine(signed short *piMicSignal,
                                  signed short *piSpeakerSignal,
                                  float *coordinateX,
                                  float *coordinateY,
                                  float *coordinateZ,
                                  char *coordinateValid,
                                  int iSequenceNum,
                                  int *pbPatternUpdate)
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
  FUNCTION:  QcUsHoveringLibGetVersion
============================================================================*/
/**
  This function gets buffer of 256 bytes, pcVersion,
  and returns string of stub version inside pcVersion.
  It returns the size of the stub version string in the piLen.
*/
extern void QcUsHoveringLibGetVersion(char *pcVersion,
                                      int *piLen)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  strlcpy(pcVersion,
          STUB_VERSION,
          *piLen);
  *piLen = sizeof(STUB_VERSION);
}

