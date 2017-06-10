/*===========================================================================
                           sync_gesture_stub.cpp

DESCRIPTION: Provide stub to Sync Gesture lib.


INITIALIZATION AND SEQUENCING REQUIREMENTS: None


Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "Sync_Gesture_stub"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include "SyncGestureExports.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define STUB_MSG_INTERVAL 5000
/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  QcUsGestureLibGetSizes
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsGestureLibGetSizes(GestureCfg const *pConfig,
                                  uint32_t *pWorkspaceSize,
                                  uint32_t *pPatternSizeSamples)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  *pWorkspaceSize = 0;
  *pPatternSizeSamples = 0;
  return 0;
}

/*==============================================================================
  FUNCTION:  QcUsGestureLibInit
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsGestureLibInit(GestureCfg const *pConfig,
                              int8_t *pWorkspace,
                              uint32_t workspaceSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return 0;
}

/*==============================================================================
  FUNCTION:  QcUsGestureLibGetPattern
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsGestureLibGetPattern(int16_t *pPattern,
                                    uint32_t patternSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return 0;
}

/*==============================================================================
  FUNCTION:  QcUsGestureLibEngine
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsGestureLibEngine(int16_t const *pSamplesBuffer,
                                GestureOutput *pGesture)
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
  return 0;
}

/*==============================================================================
  FUNCTION:  QcUsGestureLibIsBusy
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsGestureLibIsBusy(int* pIsBusy)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return 0;
}

/*==============================================================================
  FUNCTION:  QcUsGestureLibSetDynamicConfig
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsGestureLibSetDynamicConfig(int* pDynamicConfig,
                                          uint32_t dynamicConfigSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return 0;
}

