/*===========================================================================
                           sync_gesture_socket_adapter.h

DESCRIPTION: sync_gesture_socket_adapter is the header file for the
             GestureSocketAdapter class which implements all needed
             functionality to send sync gesture events to socket.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __SYNC_GESTURE_SOCKET_ADAPTER__
#define __SYNC_GESTURE_SOCKET_ADAPTER__

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "framework_adapter.h"
#include <usf_unix_domain_socket.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/
extern "C" FrameworkAdapter *get_adapter();

class SyncGestureSocketAdapter : public FrameworkAdapter
{
private:
  /**
   * m_gesture_data_socket is pointer to the thread which handles
   * data socket communication with the service.
   */
  DataUnSocket *m_gesture_data_socket;

  /**
   * The daemon's pid
   */
  int daemon_pid_;

  /**
   * Holds the adapter's current status
   */
  adapter_status_t status;

  /**
   * Holds the adapter's cfg parameters
   */
  cfg_params_t cfg_params;

  /*----------------------------------------------------------------------------
  Function definitions
  ----------------------------------------------------------------------------*/
  /*============================================================================
    CONSTRUCTOR:  GsBusAdapter
  ============================================================================*/
  SyncGestureSocketAdapter(const SyncGestureSocketAdapter &to_copy);

  /*============================================================================
    FUNCTION:  operator=
  ============================================================================*/
  SyncGestureSocketAdapter &operator=(const SyncGestureSocketAdapter &other);

protected:
  /*============================================================================
    FUNCTION:  activate
  ============================================================================*/
  /**
   * This function does nothing.
   */
  virtual void activate();


  /*============================================================================
    FUNCTION:  deactivate
  ============================================================================*/
  /**
   * This function does nothing.
   */
  virtual void deactivate();


public:
  /*============================================================================
    CONSTRUCTOR:
  ============================================================================*/
  SyncGestureSocketAdapter();

  /*============================================================================
    FUNCTION:  get_config
  ============================================================================*/
  virtual cfg_params_t &get_config();

  /*============================================================================
    FUNCTION:  wait_n_update
  ============================================================================*/
  /**
   * This function returns the current status.
   *
   * @return adapter_status_t The current adapter status.
   */
  virtual adapter_status_t wait_n_update();


  /*============================================================================
    FUNCTION:  get_status
  ============================================================================*/
  /**
   * Returns the current adapter status.
   *
   * @return adapter_status_t The current adapter status.
   */
  virtual adapter_status_t get_status();

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

  /*============================================================================
    FUNCTION:  disconnect
  ============================================================================*/
  /**
   * This function cleans up the socket instance.
   * It always succeeds.
   *
   * @return int 0 - success.
   */
  virtual int disconnect();


  /*============================================================================
    FUNCTION:  send_event
  ============================================================================*/
  /**
   * This functions sends the sync gesture events to the sync
   * gesture socket framework.
   *
   * @param event the event to send
   * @param event_source the source of event
   * @param velocity the velocity of the gesture
   *
   * @return int 0 success
   *             1 failure to send event
   *             -1 gesture ignored
   */
  virtual int send_event(int event, event_source_t event_source, int velocity);

  /*============================================================================
    FUNCTION:  on_error
  ============================================================================*/
  /**
   * This function does nothing.
   */
  virtual void on_error();

  /*============================================================================
    FUNCTION:  set_pid
  ============================================================================*/
  /**
   * This function sets the pid of the daemon in the adapter.
   */
  virtual void set_pid(int pid);
};

/*============================================================================
  FUNCTION:  get_adapter
============================================================================*/
/**
 * This function returns an instance to the ultrasound sync gesture socket adapter.
 *
 * @return Instance to the ultrasound sync gesture socket adapter.
 */
FrameworkAdapter *get_adapter();

#endif //__SYNC_GESTURE_SOCKET_ADAPTER__
