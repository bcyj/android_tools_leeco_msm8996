/*===========================================================================
                           framework_adapter.h

DESCRIPTION: Adapter base class.
  Holds function definitions and enums.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __FRAMEWORK_ADAPTER__
#define __FRAMEWORK_ADAPTER__
/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include <time.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/
enum adapter_status_t
{
  ACTIVATE,
  DEACTIVATE,
  DISCONNECT,
  SHUTDOWN,
  INTERRUPT,
  FAILURE
};

/**
 * This enum indicates whether the event would be mapped to
 * daemon's specific events or not.
 */
enum event_mapping_t
{
  MAPPED,
  NOT_MAPPED
};

/**
 * This enum indicates whether the event was calculated in the
 * application processor or the DSP.
 */
enum event_source_t
{
  EVENT_SOURCE_APSS = 0,
  EVENT_SOURCE_DSP,
  EVENT_NUM_SOURCES
};

/**
 * This struct holds cfg parameters that could be changed by the
 * framework.
 */
struct cfg_params_t
{
  time_t time_stamp;
  int sub_mode;
};

/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/
/*============================================================================
  CLASS:  FrameworkAdapter
============================================================================*/
/**
 * This class serves as an interface to adapter classes.
 * The adapter classes are meant to connect the Ultrasound daemons to various
 * other frameworks.
 */
class FrameworkAdapter
{
protected:
  /*============================================================================
    CONSTRUCTOR:  FrameworkAdapter
  ============================================================================*/
  FrameworkAdapter() {}

  /*============================================================================
    FUNCTION:  activate
  ============================================================================*/
  /**
   * Activates the adapter
   */
  virtual void activate() = 0;

  /*============================================================================
    FUNCTION:  deactivate
  ============================================================================*/
  /**
   * Deactivates the adapter.
   */
  virtual void deactivate() = 0;

public:
  /*============================================================================
    D'TOR:  FrameworkAdapter
  ============================================================================*/
  virtual ~FrameworkAdapter() {}

  /*============================================================================
    FUNCTION:  get_config
  ============================================================================*/
  virtual cfg_params_t &get_config() = 0;

  /*============================================================================
    FUNCTION:  wait_n_update
  ============================================================================*/
  /**
   * Connects the adapter to the framework, this function blocks until the
   * adapter gets connected, a failure occurs, a signal is received by the
   * thread, or a shutdown command is received by the adapter.
   *
   * @returns  0 on success
   *           1 on failure to connect framework
   *           2 if shutdown was requested
   *           3 if signal was received
   */
  virtual adapter_status_t wait_n_update() = 0;

  /*============================================================================
    FUNCTION:  disconnect
  ============================================================================*/
  virtual int disconnect() = 0;

  /*============================================================================
    FUNCTION:  send_event
  ============================================================================*/
  /*
   * Sends an event to the framework, the event arg should comply with the
   * mapped/unmapped state that the adapter expects.
   * @return int 0 success
   *             1 failure to send event
   *             -1 event not mapped
   */
  virtual int send_event(int event, event_source_t event_source, int extra) = 0;

  /*============================================================================
    FUNCTION:  on_error
  ============================================================================*/
  /*
   * This function should be called when an error occurs in the daemon.
   */
  virtual void on_error() = 0;

  /*============================================================================
    FUNCTION:  get_status
  ============================================================================*/
  virtual adapter_status_t get_status() = 0;

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
    FUNCTION:  set_pid
  ============================================================================*/
  /*
   * This function should be called by the daemon with his pid, after creating
   * the adapter.
   *
   * Sets the pid of the daemon in the adapter.
   */
  virtual void set_pid(int pid) = 0;
};

/**
  Types of the class factories
*/
typedef FrameworkAdapter *get_adapter_t();

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
#endif //__FRAMEWORK_ADAPTER__
