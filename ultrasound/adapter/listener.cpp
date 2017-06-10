/*===========================================================================
                           listener.cpp

DESCRIPTION: This class implements the onMessage callback functions
  of the Gesture Bus.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "gs_bus_adapter.h"
#include "listener.h"
#include <unistd.h>
#include "usf_log.h"
/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/
/*============================================================================
  CONSTRUCTOR:  GsBusListener
============================================================================*/
/**
 * See function definition in header file
 */
UsGsBusListener::
UsGsBusListener(GsBusAdapter &in_adapter):
  adapter(in_adapter)
{
}

/*============================================================================
  FUNCTION:  OnMessage
============================================================================*/
/**
 * See function definition in header file
 */
void UsGsBusListener::OnMessage(GsConfigurationParams const &message)
{
  if (adapter.mode_ == message.mode_)
  {
    LOGI("%s: Ultrasound gesture received activate, sub_mode is %d",
       __FUNCTION__, message.submode_);
    adapter.cfg_params.sub_mode = message.submode_;
    adapter.activate();
  }
  else
  { // GB bus activated a different source: us gesture needs to deactivate.
    LOGI("%s: Ultrasound gesture received deactivate",
       __FUNCTION__);
    adapter.deactivate();
  }
}

/*============================================================================
  FUNCTION:  OnMessage
============================================================================*/
/**
 * See function definition in header file
 */
void UsGsBusListener::OnMessage(GsBusShutdown const &message)
{
  LOGI("%s: Ultrasound gesture received shutdown",
       __FUNCTION__);
  adapter.status = SHUTDOWN;
  adapter.unblock_daemon();
}

/*============================================================================
  FUNCTION:  OnMessage
============================================================================*/
/**
 * See function definition in header file
 */
void UsGsBusListener::OnMessage(GsBusConnect const &message)
{
  adapter.client.PostMessage(GsCapability(adapter.mode_,
                                          CAPABILITY_CONFIDENCE,
                                          CAPABILITY_POWER));
}

/*============================================================================
  FUNCTION:  OnMessage
============================================================================*/
/**
 * See function definition in header file
 */
void UsGsBusListener::OnMessage(GsBusDisconnect const &message)
{
  adapter.client.PostMessage(GsCapability(adapter.mode_,
                                          CAPABILITY_CONFIDENCE,
                                          CAPABILITY_POWER));
}

