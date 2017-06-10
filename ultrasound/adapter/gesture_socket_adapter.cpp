/*===========================================================================
                           gesture_socket_adapter.cpp

DESCRIPTION: Implements the gesture socket adapter class.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "GestureSocketAdapter"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "gesture_socket_adapter.h"
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
GestureSocketAdapter::GestureSocketAdapter()
:daemon_pid_(-1),
 status(ACTIVATE)
{
  m_gesture_data_socket =
    new DataUnSocket("/data/usf/gesture/data_socket");

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
event_mapping_t GestureSocketAdapter::get_event_mapping()
{
  return MAPPED;
}

/*============================================================================
  FUNCTION:  get_config
============================================================================*/
/**
 * See function definition in header file
 */
cfg_params_t &GestureSocketAdapter::get_config()
{
  return cfg_params;
}

/*============================================================================
  FUNCTION:  wait_n_update
============================================================================*/
/**
 * See function definition in header file
 */
adapter_status_t GestureSocketAdapter::wait_n_update()
{
  return status;
}

/*============================================================================
  FUNCTION:  get_status
============================================================================*/
/**
 * See function definition in header file.
 */
adapter_status_t GestureSocketAdapter::get_status()
{
  return status;
}

/*============================================================================
  FUNCTION:  disconnect
============================================================================*/
/**
 * See function definition in header file.
 */
int GestureSocketAdapter::disconnect()
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
int GestureSocketAdapter::send_event(int event,
                                     event_source_t event_source,
                                     int extra)
{
  if (status != ACTIVATE)
  {
    return 0;
  }

  if (0 >= m_gesture_data_socket->send_gesture_event(event,
                                                     1,
                                                     event_source) ||
      0 >= m_gesture_data_socket->send_gesture_event(event,
                                                     0,
                                                     event_source))
  {
    LOGE("%s: Couldn't send gesture events to socket",
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
void GestureSocketAdapter::on_error()
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
void GestureSocketAdapter::activate()
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
void GestureSocketAdapter::deactivate()
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
void GestureSocketAdapter::set_pid(int pid)
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
  LOGD("%s: Gesture socket adapter instance created",
       __FUNCTION__);
  static GestureSocketAdapter adapter;
  return &adapter;
}
