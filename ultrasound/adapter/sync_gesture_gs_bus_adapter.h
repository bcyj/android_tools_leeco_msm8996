/*===========================================================================
                           sync_gesture_gs_bus_adapter.h

DESCRIPTION: Header file for gesture bus adapter for sync gesture daemon

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/


#ifndef __SYNC_GESTURE_GS_BUS_ADAPTER_H__
#define __SYNC_GESTURE_GS_BUS_ADAPTER_H__

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "gs_bus_client.h"
#include "gs_bus_adapter.h"

/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Class GestureGsBusAdapter
-----------------------------------------------------------------------------*/
class GestureGsBusAdapter : public GsBusAdapter
{
public:
  /*============================================================================
    CONSTRUCTOR:
  ============================================================================*/
  GestureGsBusAdapter();

  /*============================================================================
    FUNCTION:  get_event_mapping
  ============================================================================*/
  /**
   * Returns the event mapping state of the adapter.
   * The event mapping state indicates whether the adapter expects a daemon
   * mapped events or regular event index when sending an event to the
   * framework.
   *
   * @return event_mapping_t MAPPED - If the current adapter
   *                                  expects mapped events.
   *                         NOT_MAPPED - If the current adapter
   *                                  does not expect mapped
   *                                  events.
   */
  virtual event_mapping_t get_event_mapping();
protected:
  /*============================================================================
    FUNCTION:  map_event
  ============================================================================*/
  /**
   * This function takes as argument the an event index/mapping in corresponse
   * to the get_event_mapping() function, then maps it to a gesture bus outcome.
   *
   * @param event Daemon-unmapped/mapped event to be mapped to the gesture bus
   *              outcome
   *
   * @return The mapping of the given event.
   */
  virtual GsOutcome map_event(int event);
};

#endif
