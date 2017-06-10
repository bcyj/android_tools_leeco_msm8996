/*===========================================================================
                           gs_bus_adapter.h

DESCRIPTION: gs_bus_adapter is the header file for the GsBusAdapter class
             which is an abstract class that implements all needed functionality
             to communicate with the gs bus.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __GS_BUS_ADAPTER__
#define __GS_BUS_ADAPTER__

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "framework_adapter.h"
#include "gs_bus_client.h"
#include "listener.h"
#include <unistd.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define MAX_NAME_SIZE 20 // Also the max name size in gesture bus

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/
extern "C" FrameworkAdapter *get_adapter();

class GsBusAdapter : public FrameworkAdapter
{
private:
  /*----------------------------------------------------------------------------
    Variable Definitions
  ----------------------------------------------------------------------------*/
  cfg_params_t cfg_params;

  /*
   * The daemon's pid
   */
  int daemon_pid_;

  /**
   * The mode of the messages supported by the daemon
   */
  GsMode mode_;

  /**
   * Name of the client using the adapter
   */
  char name_[MAX_NAME_SIZE];

  /**
   * Gesture bus instance
   */
  GsBusClient client;

  /**
   * Gesture bus listener instance
   */
  UsGsBusListener listener;

  /**
   * Holds the adapter's current status
   */
  adapter_status_t status;

  // Constants
  /**
   * Number of times to try to connect to gesture bus.
   */
  static const uint32_t CONNECTION_TRIES = 10;
  /**
   * Number of times to try to post a message
   */
  static const uint32_t POSTING_TRIES = 10;
  /**
   * Number of seconds to wait until checking if ultrasound
   * gesture has been activated yet.
   */
  static const uint32_t WAIT_ACTIVATION_SEC = 3;

  /*----------------------------------------------------------------------------
  Function definitions
  ----------------------------------------------------------------------------*/
  /*============================================================================
    CONSTRUCTOR:  GsBusAdapter
  ============================================================================*/
  GsBusAdapter(const GsBusAdapter &to_copy);

  /*============================================================================
    FUNCTION:  operator=
  ============================================================================*/
  GsBusAdapter &operator=(const GsBusAdapter &other);

  /*============================================================================
    FUNCTION:  connect_gs_bus
  ============================================================================*/
  /**
   * Tries to connect to the GS bus, the number of given times.
   *
   * @param connection_tries The number of times to try to connect
   *                         to the GS bus.
   * @param client_name The name of the client that is connecting
   *
   * @return int 0 - success.
   *             1 - failure to conenct, max tried exceeded.
   *             2 - failure to connect, shutdown received.
   */
  int connect_gs_bus(uint32_t connection_tries);

  /*============================================================================
    FUNCTION:  unblock_daemon
  ============================================================================*/
  /*
   * unblocks the daemon, this should be used when there's a chance that the
   * daemon might be stuck in a blocking-function, and immediate response is
   * required from it.
   */
  void unblock_daemon();

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
  virtual GsOutcome map_event(int event) = 0;

  /*============================================================================
    FUNCTION:  activate
  ============================================================================*/
  /**
   * This function changes the adapter status to activated.
   */
  virtual void activate();


  /*============================================================================
    FUNCTION:  deactivate
  ============================================================================*/
  /**
   * This function changes the adapter status to deactivate.
   */
  virtual void deactivate();

public:
  friend class UsGsBusListener;

  /*============================================================================
    CONSTRUCTOR:
  ============================================================================*/
  GsBusAdapter(char *name,
               int name_size,
               GsMode mode);

  /*============================================================================
    FUNCTION:  get_config
  ============================================================================*/
  virtual cfg_params_t &get_config();

  /*============================================================================
    FUNCTION:  wait_n_update
  ============================================================================*/
  /**
   * Connects the adapter to the gesture bus, this function blocks until the
   * adapter gets connected, a failure occurs, a signal is received by the
   * thread, or a shutdown command is received by the adapter.
   *
   * This function should change the adpater status to connected.
   *
   * @return   0 on success
   *           1 on failure to connect gesture bus
   *           2 if shutdown was requested
   *           3 if signal was received
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
  /*
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
  virtual event_mapping_t get_event_mapping() = 0;

  /*============================================================================
    FUNCTION:  disconnect
  ============================================================================*/
  /**
   * This function disconnects from the gesture bus.
   * It always succeeds.
   *
   * @return int 0 - success.
   */
  virtual int disconnect();


  /*============================================================================
    FUNCTION:  send_event
  ============================================================================*/
  /**
   * This functions sends the gesture events to the gesture
   * framework.
   *
   * @param event the event to send
   * @param event_source the event source
   * @param extra if needed, extra information about the event
   *
   * @return int 0 success
   *             1 failure to send event
   *             -1 gesture ignored
   */
  virtual int send_event(int event, event_source_t event_source, int extra);


  /*============================================================================
    FUNCTION:  on_error
  ============================================================================*/
  /*
   * This function should be called when an error occurs in the daemon.
   */
  virtual void on_error();

  /*============================================================================
    FUNCTION:  set_pid
  ============================================================================*/
  /*
   * This function should be called by the daemon with its pid, after creating
   * the adapter.
   *
   * Sets the pid of the daemon in the adapter.
   */
  virtual void set_pid(int pid);
};

/*============================================================================
  FUNCTION:  get_adapter
============================================================================*/
/**
 * This function returns an instance to the ultrasound gesture adapter.
 *
 * @return Instance to the ultrasound gesture adapter.
 */
FrameworkAdapter *get_adapter();

#endif //__GS_BUS_ADAPTER__
