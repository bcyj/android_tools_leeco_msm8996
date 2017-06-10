/*===========================================================================
                           gesture_gs_bus_adapter.cpp

DESCRIPTION: Implements a gesture bus adapter for gesture daemon

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "gesture_gs_bus_adapter.h"
#include "usf_log.h"
#include <GestureExports.h>
/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define NAME "usf_gesture"

/*-----------------------------------------------------------------------------
  GestureGsBusAdapter Function Implementation
-----------------------------------------------------------------------------*/
/*============================================================================
  GestureGsBusAdapter CONSTRUCTOR
============================================================================*/
GestureGsBusAdapter::GestureGsBusAdapter()
:GsBusAdapter((char *)NAME, sizeof(NAME), kGsModeUltrasoundSwipe)
{
}

/*============================================================================
  FUNCTION:  GestureGsBusAdapter::get_event_mapping
============================================================================*/
event_mapping_t GestureGsBusAdapter::get_event_mapping()
{
  return NOT_MAPPED;
}

/*============================================================================
  FUNCTION:  GestureGsBusAdapter::map_event
============================================================================*/
GsOutcome GestureGsBusAdapter::map_event(int event)
{
  GsOutcome outcome;
  switch (event)
  {
  case QC_US_GESTURE_LIB_OUTCOME_RIGHT:
    outcome.type_ = GsOutcome::kTypeSwipe;
    outcome.subtype_ = GsOutcome::kSubtypeSwipeRight;
    break;
  case QC_US_GESTURE_LIB_OUTCOME_LEFT:
    outcome.type_ = GsOutcome::kTypeSwipe;
    outcome.subtype_ = GsOutcome::kSubtypeSwipeLeft;
    break;
  default:
    // Gesture is not mapped
    outcome.subtype_ = GsOutcome::kSubtypeNone;
  }
  return outcome;
}

/*============================================================================
  FUNCTION:  get_adapter
============================================================================*/
/**
 * Returns this adapter instance. This function is declared in gs_bus_adapter.h
 * and should be implemented for each gesture bus adapter library. As this
 * function is the one that gets called after dynamically loading the DLL.
 */
FrameworkAdapter *get_adapter()
{
  LOGD("%s: Gesture gs bus adapter instance created",
       __FUNCTION__);
  static GestureGsBusAdapter adapter;
  return &adapter;
}

