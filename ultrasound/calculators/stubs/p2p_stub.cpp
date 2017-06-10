/*===========================================================================
                           p2p_stub.cpp

DESCRIPTION: Provide stub to P2P lib.


INITIALIZATION AND SEQUENCING REQUIREMENTS: None


Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "P2P_stub"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include "P2PExports.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define STUB_MSG_INTERVAL 5000
/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  QcUsP2PLibGetSizes
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsP2PLibGetSizes(P2PCfg const *pConfig,
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
  FUNCTION:  QcUsP2PLibInit
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsP2PLibInit(P2PCfg const *pConfig,
                          int8_t *pWorkspace,
                          uint32_t workspaceSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return 0;
}

/*==============================================================================
  FUNCTION:  QcUsP2PLibGetPattern
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsP2PLibGetPattern(int16_t *pPattern,
                                uint32_t patternSize)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return 0;
}

/*==============================================================================
  FUNCTION:  QcUsP2PLibEngine
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
extern int QcUsP2PLibEngine(int16_t const *pSamplesBuffer,
                            P2POutput *pOutput)
{
  static int print_stub_msg_counter = STUB_MSG_INTERVAL;
  static int counter = 0;
  uint32_t i;

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

  // TODO temp arbitrary update results for testing the demo app, remove later
  counter++;
  pOutput->request = 0;
  if (counter % 10 == 0)
  {
      pOutput->result = QC_US_P2P_RESULT_STATUS_UPDATED;
      for (i=0; i<QC_US_P2P_MAX_USERS; i++)
      {
          pOutput->usersStatus[i].angle = (i*90 + counter) % 360;
          pOutput->usersStatus[i].distance = 10;
      }
  }
  else
  {
      pOutput->result = QC_US_P2P_RESULT_IDLE;
  }

  return 0;
}

