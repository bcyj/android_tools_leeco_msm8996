/*===========================================================================
                           listener.h

DESCRIPTION: Holds method declaration and fields of the listener class.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __LISTENER__
#define __LISTENER__

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "gs_bus_client.h"
#include <unistd.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define CAPABILITY_CONFIDENCE 0
#define CAPABILITY_POWER      0

/**
 * Must forward declare here, because listener includes gesture_framework_adapter
 * which includes listener.This way listener is compiled without declaring
 * GsBusListener
 */
class GsBusAdapter;

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/
/*============================================================================
  CLASS:  GsBusListener
============================================================================*/
class UsGsBusListener: public GsBusListener
{
private:

  /*============================================================================
    CONSTRUCTOR:  GsBusListener
  ============================================================================*/
  UsGsBusListener();

  /*============================================================================
    COPY CONSTRUCTOR:  GsBusListener
  ============================================================================*/
  UsGsBusListener(const UsGsBusListener &to_copy);

  /*============================================================================
    FUNCTION:  operator=
  ============================================================================*/
  UsGsBusListener &operator=(const UsGsBusListener &other);

  /**
   * Reference to the gesture framework adapter
   */
  GsBusAdapter &adapter;
public:
  /*============================================================================
    CONSTRUCTOR:  GsBusListener
  ============================================================================*/
  UsGsBusListener(GsBusAdapter &in_adapter);

/*-----------------------------------------------------------------------------
  OnMessage functions overriding
-----------------------------------------------------------------------------*/
  /*============================================================================
    FUNCTION:  OnMessage
  ============================================================================*/
  /**
   * This function is a gesture bus callback function. When called
   * with the Ultrasound parameters, it would set the adapter
   * status to ACTIVATE. Else, it would set the status to
   * DEACTIVATE.
   *
   * @param message The content of the parameter message.
   */
  virtual void OnMessage(GsConfigurationParams const &message);


  /*============================================================================
    FUNCTION:  OnMessage
  ============================================================================*/
  /**
   * Handles shutdown message by changing the adapter status to
   * SHUTDOWN stage.
   *
   * @param message The shutdown message
   */
  virtual void OnMessage(GsBusShutdown const &message);

  /*============================================================================
    FUNCTION:  OnMessage
  ============================================================================*/
  /**
   * Handles connect message by sending a capability message.
   *
   * @param message The connect message
   */
  virtual void OnMessage(GsBusConnect const &message);

  /*============================================================================
    FUNCTION:  OnMessage
  ============================================================================*/
  /**
   * Handles disconnect message by sending a capcbility message.
   *
   * @param message The disconnect message
   */
  virtual void OnMessage(GsBusDisconnect const &message);
};

#endif //__LISTENER__
