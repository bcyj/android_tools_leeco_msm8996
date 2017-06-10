/*===========================================================================
                           gs_bus_adapter.cpp

DESCRIPTION: Implements the gesture bus adapter class.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "GsBusAdapter"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "gs_bus_adapter.h"
#include <unistd.h>
#include "usf_log.h"
#include <signal.h>
#include <math.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*============================================================================
  CONSTRUCTOR:
============================================================================*/
GsBusAdapter::GsBusAdapter(char *name,
                           int name_size,
                           GsMode mode)
:daemon_pid_(-1),
 mode_(mode),
 listener(*this),
 status(DISCONNECT)
{
  if (name_size > MAX_NAME_SIZE)
  {
    LOGW("%s, given client name size(%d) will be truncated(%d)",
         __FUNCTION__,
         name_size,
         MAX_NAME_SIZE);
  }
  strlcpy(name_,
          name,
          fmin(name_size,
               MAX_NAME_SIZE));
  cfg_params.time_stamp = 0;
}

/*============================================================================
  FUNCTION:  get_config
============================================================================*/
cfg_params_t &GsBusAdapter::get_config()
{
  return cfg_params;
}

/*============================================================================
  FUNCTION:  wait_n_update
============================================================================*/
/**
 * Calls connect_gs_bus
 */
adapter_status_t GsBusAdapter::wait_n_update()
{
  int res = 0;
  if (DISCONNECT == status)
  {
    // Connect to the GS bus
    res = connect_gs_bus(CONNECTION_TRIES);
    if (0 != res)
    {
      return FAILURE;
    }
    // Connected, but not active yet.
    status = DEACTIVATE;
  }

  // Wait for activation command
  while (ACTIVATE != status &&
         SHUTDOWN != status)
  {
    LOGI("%s: Waiting for activation or shutdown",
         __FUNCTION__);
    if (0 != sleep(WAIT_ACTIVATION_SEC))
    {
      // We should check for SHUTDOWN first. When setting SHUTDOWN, the manager
      // might interrupt the daemon to unblock him.
      if (SHUTDOWN == status)
      {
        return SHUTDOWN;
      }
      LOGI("%s: SIGNAL received",
         __FUNCTION__);
      return INTERRUPT;
    }
  }
  if (SHUTDOWN == status)
  {
    return SHUTDOWN;
  }
  return ACTIVATE;
}

/*============================================================================
  FUNCTION:  get_status
============================================================================*/
/**
 * See function definition in header file.
 */
adapter_status_t GsBusAdapter::get_status()
{
  return status;
}

/*============================================================================
  FUNCTION:  disconnect
============================================================================*/
/**
 * See function definition in header file.
 */
int GsBusAdapter::disconnect()
{
  LOGI("%s: Ultrasound daemon disconnecting",
       __FUNCTION__);
  if (DISCONNECT != status)
  {
    client.Disconnect();
  }
  status = DISCONNECT;
  return 0;
}

/*============================================================================
  FUNCTION:  send_event
============================================================================*/
/**
 * Calls map_event() to map the event, then tries POSTING_TRIES times to call
 * client.PostMessage() until it succeeds.
 */
int GsBusAdapter::send_event(int event,
                             event_source_t event_source,
                             int extra)
{
  if (status != ACTIVATE)
  {
    return 0;
  }

  GsOutcome outcome = map_event(event);
  outcome.velocity_ = extra;

  LOGI("%s: Input: %d Output: %d",
       __FUNCTION__,
       event,
       (int)outcome.subtype_);

  if (GsOutcome::kSubtypeNone == outcome.subtype_)
  { // Event not mapped
    return -1;
  }

  uint32_t i = 0;
  while (true != client.PostMessage(outcome) &&
         POSTING_TRIES > i)
  {
    i++;
  }

  return (POSTING_TRIES <= i);
}

/*============================================================================
  FUNCTION:  on_error
============================================================================*/
/**
 * See function definition in header file.
 */
void GsBusAdapter::on_error()
{
  LOGI("%s: called",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  activate
============================================================================*/
/**
 * See function definition in header file.
 */
void GsBusAdapter::activate()
{
  LOGI("%s: Received",
       __FUNCTION__);
  status = ACTIVATE;
}

/*============================================================================
  FUNCTION:  deactivate
============================================================================*/
/**
 * See function definition in header file.
 */
void GsBusAdapter::deactivate()
{
  LOGI("%s: Received",
         __FUNCTION__);
  if (status != DEACTIVATE)
  {
    status = DEACTIVATE;
    unblock_daemon();
  }
}

/*============================================================================
  FUNCTION:  connect_gs_bus
============================================================================*/
/**
 * See function definition in header file.
 */
int GsBusAdapter::connect_gs_bus(uint32_t connection_tries)
{
  while (0 != connection_tries &&
         SHUTDOWN != status)
  {
    LOGI("%s: Trying to connect, left: %d tries",
         __FUNCTION__,
         connection_tries);
    if (true == client.Connect(name_,
                               listener))
    {
      LOGI("%s: Connected to gs_bus",
         __FUNCTION__);
      client.PostMessage(GsCapability(mode_,
                                      CAPABILITY_CONFIDENCE,
                                      CAPABILITY_POWER));
      return 0;
    }
    connection_tries--;
  }
  if (SHUTDOWN == status)
  {
    LOGI("%s: Failed to connect to gs bus, shutdown received.",
         __FUNCTION__);
    return 2;
  }

  LOGI("%s: Failed to connect to gs bus",
       __FUNCTION__);
  return 1;
}

/*============================================================================
  FUNCTION:  set_pid
============================================================================*/
void GsBusAdapter::set_pid(int pid)
{
  daemon_pid_ = pid;
}

/*============================================================================
  FUNCTION:  unblock_daemon
============================================================================*/
void GsBusAdapter::unblock_daemon()
{
  if (-1 != daemon_pid_)
  {
    kill(daemon_pid_, SIGALRM);
  }
}

