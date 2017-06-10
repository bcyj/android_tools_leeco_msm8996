/*===========================================================================
                           sync_gesture_socket_adapter.cpp

DESCRIPTION: Implements the sync gesture socket adapter class.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "SyncGestureSocketAdapter"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "sync_gesture_socket_adapter.h"
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
/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*============================================================================
  CONSTRUCTOR:
============================================================================*/
/**
 * See function definition in header file
 */
SyncGestureSocketAdapter::SyncGestureSocketAdapter()
:daemon_pid_(-1),
 status(ACTIVATE)
{
  m_gesture_data_socket =
    new DataUnSocket("/data/usf/sync_gesture/data_socket");

  if (0 != m_gesture_data_socket->start())
  {
    LOGE("%s: Starting data socket failed.",
         __FUNCTION__);
    status = FAILURE;
  }
}

/*============================================================================
  FUNCTION:  get_event_mapping
============================================================================*/
/**
 * See function definition in header file
 */
event_mapping_t SyncGestureSocketAdapter::get_event_mapping()
{
  return NOT_MAPPED;
}

/*============================================================================
  FUNCTION:  get_config
============================================================================*/
/**
 * See function definition in header file
 */
cfg_params_t &SyncGestureSocketAdapter::get_config()
{
  return cfg_params;
}

/*============================================================================
  FUNCTION:  wait_n_update
============================================================================*/
/**
 * See function definition in header file
 */
adapter_status_t SyncGestureSocketAdapter::wait_n_update()
{
  return status;
}

/*============================================================================
  FUNCTION:  get_status
============================================================================*/
/**
 * See function definition in header file.
 */
adapter_status_t SyncGestureSocketAdapter::get_status()
{
  return status;
}

/*============================================================================
  FUNCTION:  disconnect
============================================================================*/
/**
 * See function definition in header file.
 */
int SyncGestureSocketAdapter::disconnect()
{
  LOGI("%s: Ultrasound daemon disconnecting",
       __FUNCTION__);
  if (NULL != m_gesture_data_socket)
  {
    delete m_gesture_data_socket;
    m_gesture_data_socket = NULL;
  }
  status = DISCONNECT;
  return 0;
}

/*============================================================================
  FUNCTION:  send_event
============================================================================*/
/**
 * See function definition in header file
 */
int SyncGestureSocketAdapter::send_event(int event,
                                         event_source_t event_source,
                                         int velocity)
{
  if (status != ACTIVATE)
  {
    return 0;
  }

  if (0 >= m_gesture_data_socket->send_gesture_event(event,
                                                     velocity,
                                                     event_source))
  {
    LOGE("%s: Couldn't send sync gesture events to socket",
         __FUNCTION__);
    return 1;
  }
  return 0;
}

/*============================================================================
  FUNCTION:  on_error
============================================================================*/
/**
 * See function definition in header file.
 */
void SyncGestureSocketAdapter::on_error()
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
void SyncGestureSocketAdapter::activate()
{
  LOGI("%s: Received",
       __FUNCTION__);
}

/*============================================================================
  FUNCTION:  deactivate
============================================================================*/
/**
 * See function definition in header file.
 */
void SyncGestureSocketAdapter::deactivate()
{
  LOGI("%s: Received",
         __FUNCTION__);
}

/*============================================================================
  FUNCTION:  set_pid
============================================================================*/
/**
 * See function definition in header file
 */
void SyncGestureSocketAdapter::set_pid(int pid)
{
  daemon_pid_ = pid;
}

/*============================================================================
  FUNCTION:  get_adapter
============================================================================*/
/**
 * See function definition in header file
 */
FrameworkAdapter *get_adapter()
{
  LOGD("%s: Sync Gesture socket adapter instance created",
       __FUNCTION__);
  static SyncGestureSocketAdapter adapter;
  return &adapter;
}
