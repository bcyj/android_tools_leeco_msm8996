/*======================================================================

         BIT (BEARER-INDEPENDENT-TRANSPORT) API definitions file

 GENERAL DESCRIPTION
  This file contains API definitions by which BIT services can
  be accessed by clients (e.g Transport Manager) needing user-plane
  connections.

  There is no separate task context for BIT API, it'll be in PDCOMM
  TCP Task's context or in BIT Daemon's context.

 EXTERNALIZED FUNCTIONS


 INITIALIZATION AND SEQUENCING REQUIREMENTS


 Copyright (c) 2012 by Qualcomm Technologies, Inc. All Rights Reserved.

======================================================================*/

/*=====================================================================

 EDIT HISTORY FOR MODULE

 This section contains comments describing changes made to the module.
 Notice that changes are listed in reverse chronological order.

 $Header: //source/qcom/qct/modem/gps/gnss/main/latest/pd_comms/src/gpsone_bit_api.c#3 $$DateTime: 2010/07/09 11:43:08 $$Author: ruih $

 when      who   what, where, why
 --------  ----  --- ---------------------------------------------------
 06/22/10   rh   Fixed some bugs related to delayed open processing
 05/17/10   rh   Added saved connect/ioctl request handling for delayed open
 04/28/10   rh   Initial version
======================================================================*/

#include "customer.h"
#include "comdef.h"

#include "gpsone_bit_api.h"

#ifdef FEATURE_GNSS_BIT_API

#include "gpsone_bit_local_api.h"

#include "aries_os_api.h"  // for mutex


/*****************************************************************************/
/*             Local Data Structures and Variables Definitions               */
/*****************************************************************************/

/* array to hold the BIT registration data, array[0] is dummy (all NULL pointers) */
static gpsone_bit_transport_info_type    bit_transport_array[GPSONE_BIT_MAX_NUM_OF_TRANSPORT];

/* pointer to current BIT transport */
static gpsone_bit_transport_info_type    *cur_bit_transport = NULL;

static gpsone_bit_state_e_type           cur_bit_state = GPSONE_BIT_STATE_UNINITIALIZED;

 /* increasing counter to keep track of transport handle registration, first active one is 1 */
static gpsone_bit_transport_handle_type  bit_handle_counter = 0;

/* current active transport handle, 0xFFFF is invalid */
static gpsone_bit_transport_handle_type  cur_bit_handle = GPSONE_BIT_INVALID_TRANSPORT_HANDLE;

 /* mutex to guard bit global data structure (cur_bit_handle, cur_bit_state, bit_transport_array, etc) */
static os_MutexBlockType                 z_bit_mutex;


/* data structure to save the connect/ioctl requests before BIT is opened
   we'll over support
   these requests will be sent over once successful OPEN_RESULT is returned */
typedef struct
{
  boolean                           valid;
  uint32                            transaction_id;
  gpsone_bit_connect_params_type    connect_param;
}bit_saved_connect_param;

typedef struct
{
  boolean                            valid;
  gpsone_bit_session_handle_type     session_handle;
  uint32                             transaction_id;
  gpsone_bit_ioctl_e_type            ioctl_request;
  gpsone_bit_ioctl_params_type       ioctl_param;
}bit_saved_ioctl_param;

// highly unlikely that we have more than 4/3 connect/ioctl pending while we do open
#define BIT_MAX_SAVED_CONNECT_REQUESTS   4
#define BIT_MAX_SAVED_IOCTL_REQUESTS     3
static bit_saved_connect_param bit_saved_connects[BIT_MAX_SAVED_CONNECT_REQUESTS];
static bit_saved_ioctl_param bit_saved_ioctls[BIT_MAX_SAVED_IOCTL_REQUESTS];

// mutex to protect the saved ioctl/connect requests
static os_MutexBlockType                 z_bit_save_mutex;



/*****************************************************************************/
/*                Local Helper Routines (called from this file)              */
/*****************************************************************************/


/*===========================================================================

FUNCTION: bit_handle_change_state

DESCRIPTION:
  This function is used to change the BIT transport state
===========================================================================*/
static void bit_handle_change_state(
  gpsone_bit_state_e_type new_state
)
{
  cur_bit_state = new_state;
}

/*===========================================================================

FUNCTION: bit_handle_deregister_cleanup

DESCRIPTION:
  This function is used to handle the final clean-up during BIT deregistration
===========================================================================*/

static void  bit_handle_deregister_cleanup(void)
{
  // TODO: if transport is still active/open (with pending operation), need to clean up all states first

  BIT_MSG_FUNC( "bit_handle_deregister_cleanup: state=%d", cur_bit_state, 0, 0 );

  cur_bit_transport = NULL;
  cur_bit_handle = GPSONE_BIT_INVALID_TRANSPORT_HANDLE;

  if (cur_bit_state != GPSONE_BIT_STATE_UNINITIALIZED)
    cur_bit_state = GPSONE_BIT_STATE_UNREGISTERED;

  // memset bit_transport_array[0] so all pointers are NULL
  memset(&bit_transport_array, 0, sizeof(bit_transport_array));
}

/*===========================================================================

FUNCTION: bit_handle_check_and_change_state

DESCRIPTION:
  This function is used to check the required state, if match, state will be
  changed to new state  (holding mutex)
===========================================================================*/
static gpsone_bit_status_e_type bit_handle_check_and_change_state(
  gpsone_bit_state_e_type required_state,
  void * bit_fp,
  gpsone_bit_state_e_type new_state
)
{
  //BIT_MSG_FUNC( "bit_handle_check_and_change_state: state=%d", cur_bit_state, 0, 0 );

  // not initialized or registered yet
  if (cur_bit_state == GPSONE_BIT_STATE_UNINITIALIZED || cur_bit_state == GPSONE_BIT_STATE_UNREGISTERED)
  {
    BIT_MSG_ERROR( "BIT transport is not registered yet!", 0, 0, 0 );
    return GPSONE_BIT_STATUS_NOT_REGISTERED;
  }

  if (cur_bit_transport == NULL || cur_bit_handle == GPSONE_BIT_INVALID_TRANSPORT_HANDLE || bit_fp == NULL)
  {
    // should never happen, if state is REGISTERED, these should be valid!
    BIT_MSG_ERROR( "BIT transport is not registered properly, fatal error!", 0, 0, 0 );
    bit_handle_deregister_cleanup();
    return GPSONE_BIT_STATUS_FATAL_ERROR;
  }

  if (cur_bit_state != required_state)
  {
    BIT_MSG_ERROR( "Operation not allowed with BIT transport state 0x%x", cur_bit_state, 0, 0 );
    return GPSONE_BIT_STATUS_NOT_ALLOWED;
  }

  if (new_state != GPSONE_BIT_STATE_UNCHANGED)
  {
      cur_bit_state = new_state; // change state while holding mutex

      BIT_MSG_FUNC( "bit_handle_check_and_change_state: newstate=%d", cur_bit_state, 0, 0 );
  }

  return GPSONE_BIT_STATUS_SUCCESS;
}

/*===========================================================================

FUNCTION: bit_save_connect_request

DESCRIPTION:
  This function saves the connect request to pending list
===========================================================================*/
static gpsone_bit_status_e_type bit_save_connect_request
(
  uint32                            transaction_id,
  gpsone_bit_connect_params_type    *connect_param
)
{
  int i;
  gpsone_bit_status_e_type status = GPSONE_BIT_STATUS_NO_RESOURCES;

  os_MutexLock(&z_bit_save_mutex);
  for(i = 0; i < BIT_MAX_SAVED_CONNECT_REQUESTS; i++)
  {
    if (bit_saved_connects[i].valid == FALSE)
    {
      bit_saved_connects[i].valid = TRUE;
      bit_saved_connects[i].transaction_id = transaction_id;
      bit_saved_connects[i].connect_param = *connect_param;
      status = GPSONE_BIT_STATUS_SUCCESS;
      BIT_MSG_FUNC( "bit_save_connect_request: i=%d transid=%d", i, transaction_id, 0 );
      break;
    }
  }

  os_MutexUnlock(&z_bit_save_mutex);
  return status;
}

/*===========================================================================

FUNCTION: bit_save_ioctl_request

DESCRIPTION:
  This function saves the ioctl request to pending list
===========================================================================*/
static gpsone_bit_status_e_type bit_save_ioctl_request
(
  gpsone_bit_session_handle_type     session_handle,
  uint32                             transaction_id,
  gpsone_bit_ioctl_e_type            ioctl_request,
  gpsone_bit_ioctl_params_type       *ioctl_param
)
{
  int i;
  gpsone_bit_status_e_type status = GPSONE_BIT_STATUS_NO_RESOURCES;

  os_MutexLock(&z_bit_save_mutex);
  for(i = 0; i < BIT_MAX_SAVED_IOCTL_REQUESTS; i++)
  {
    if (bit_saved_ioctls[i].valid == FALSE)
    {
      bit_saved_ioctls[i].valid = TRUE;
      bit_saved_ioctls[i].session_handle = session_handle;
      bit_saved_ioctls[i].transaction_id = transaction_id;
      bit_saved_ioctls[i].ioctl_request = ioctl_request;
      bit_saved_ioctls[i].ioctl_param = *ioctl_param;
      status = GPSONE_BIT_STATUS_SUCCESS;
      BIT_MSG_FUNC( "bit_save_ioctl_request: i=%d transid=%d", i, transaction_id, 0 );
      break;
    }
  }

  os_MutexUnlock(&z_bit_save_mutex);
  return status;

}

/*===========================================================================

FUNCTION: bit_reset_saved_connect_ioctl

DESCRIPTION:
  This function resets the saved connect/ioctl requests
===========================================================================*/
static void bit_reset_saved_connect_ioctl(
  void
)
{
  int i;

  BIT_MSG_FUNC( "bit_reset_saved_connect_ioctl: state=%d", cur_bit_state, 0, 0 );

  os_MutexLock(&z_bit_save_mutex);

  for(i = 0; i < BIT_MAX_SAVED_CONNECT_REQUESTS; i++)
  {
    if (bit_saved_connects[i].valid == TRUE)
      BIT_MSG_ERROR( "Saved connect request %d still exists for CLOSED BIT daemon!", i, 0, 0 );
    bit_saved_connects[i].valid = FALSE;
  }

  for(i = 0; i < BIT_MAX_SAVED_IOCTL_REQUESTS; i++)
  {
    if (bit_saved_ioctls[i].valid == TRUE)
      BIT_MSG_ERROR( "Saved ioctl request %d still exists for CLOSED BIT daemon!", i, 0, 0 );
    bit_saved_ioctls[i].valid = FALSE;
  }

  os_MutexUnlock(&z_bit_save_mutex);
}

/*===========================================================================

FUNCTION: bit_handle_saved_connect_ioctl

DESCRIPTION:
  This function handles the saved connect/ioctl requests (fail them or resubmit
  after BIT daemon has been opened)
===========================================================================*/
static void bit_handle_saved_connect_ioctl(
  boolean                    resubmit_request,
  gpsone_bit_status_e_type   default_status
)
{
  int i;
  gpsone_bit_status_e_type       status = GPSONE_BIT_STATUS_SUCCESS;
  gpsone_bit_event_payload_type  event_payload;

  BIT_MSG_FUNC( "bit_handle_saved_connect_ioctl: state=%d", cur_bit_state, 0, 0 );

  os_MutexLock(&z_bit_save_mutex);
  for(i = 0; i < BIT_MAX_SAVED_CONNECT_REQUESTS; i++)
  {
    if (bit_saved_connects[i].valid == TRUE)
    {
      bit_saved_connects[i].valid = FALSE;  // mark it as invalid
      status = default_status;

      if (resubmit_request && cur_bit_state == GPSONE_BIT_STATE_OPENED)
      {
        BIT_MSG_MED( "Sending saved connect: i=%d transid=%d", i, bit_saved_connects[i].transaction_id, 0 );

        status = gpsone_bit_connect(bit_saved_connects[i].transaction_id, &bit_saved_connects[i].connect_param);
        if (status == GPSONE_BIT_STATUS_WAIT) continue; // process next one, CONNECT_RESULT will come
      }

      BIT_MSG_HIGH( "Saved connect %d result 0x%x", i, status, 0 );  // sucess or failure

      // connect can't return immediately since we'll need session_handle returned in notify()!
      event_payload.event = GPSONE_BIT_EVENT_CONNECT_RESULT;
      event_payload.result = status;

      // status is now failure, need to generate CONNECT_RESULT manually
      if (gpsone_bit_post_event(GPSONE_BIT_INVALID_SESSION_HANDLE,
                bit_saved_connects[i].transaction_id, &event_payload) != GPSONE_BIT_STATUS_SUCCESS)
      {
        // We should not terminate, just ignore this connect request and process remaining
        //os_MutexUnlock(&z_bit_save_mutex);
        //return GPSONE_BIT_STATUS_IPC_FAILURE;
      }
    }
  }

  for(i = 0; i < BIT_MAX_SAVED_IOCTL_REQUESTS; i++)
  {
    if (bit_saved_ioctls[i].valid == TRUE)
    {
      bit_saved_ioctls[i].valid = FALSE;
      // mark it as invalid
      status = default_status;

      if (resubmit_request && cur_bit_state == GPSONE_BIT_STATE_OPENED)
      {
        BIT_MSG_MED( "Sending saved ioctl: i=%d transid=%d", i, bit_saved_ioctls[i].transaction_id, 0 );

        status = gpsone_bit_ioctl(bit_saved_ioctls[i].session_handle,
                                  bit_saved_ioctls[i].transaction_id,
                                  bit_saved_ioctls[i].ioctl_request,
                                  &bit_saved_ioctls[i].ioctl_param);
        if (status == GPSONE_BIT_STATUS_WAIT) continue; // process next one, IOCTL_RESULT will come
      }

      BIT_MSG_HIGH( "Saved ioctl %d result 0x%x", i, status, 0 ); // success or failure

      // status is now failure or success, need to generate IOCTL_RESULT manually
      event_payload.event = GPSONE_BIT_EVENT_IOCTL_RESULT;
      event_payload.result = status;

      if (gpsone_bit_post_event(bit_saved_ioctls[i].session_handle,
                                bit_saved_ioctls[i].transaction_id, &event_payload) != GPSONE_BIT_STATUS_SUCCESS)
      {
         // We should not terminate, just ignore this ioctl request and process remaining
        //os_MutexUnlock(&z_bit_save_mutex);
        //return GPSONE_BIT_STATUS_IPC_FAILURE;  // IPC failure
      }
    }
  }

  os_MutexUnlock(&z_bit_save_mutex);
}


/*===========================================================================

FUNCTION: bit_handle_delayed_open

DESCRIPTION:
  This function is used to send open() request to BIT daemon
  If it's already opened, it'll return SUCCESS
===========================================================================*/
static gpsone_bit_status_e_type bit_handle_delayed_open(
  void
)
{
  gpsone_bit_status_e_type status;
  gpsone_bit_open_params_type open_param;

  switch(cur_bit_state)
  {
  case GPSONE_BIT_STATE_OPENING:
    status = GPSONE_BIT_STATUS_WAIT;  // open already requested, pending now
    break;

  case GPSONE_BIT_STATE_OPENED:
    status = GPSONE_BIT_STATUS_SUCCESS; // already opened
    break;

  case GPSONE_BIT_STATE_CLOSING:
    status = GPSONE_BIT_STATUS_FAIL;  // closing now, should not re-open
    break;

  case GPSONE_BIT_STATE_CLOSED:
    open_param.force_connection_up = FALSE;  // specify default parameters here
    status = gpsone_bit_open(&open_param);  // request to open
    break;

  default:
    status = GPSONE_BIT_STATUS_NOT_REGISTERED;
    break;
  }

  return status;
}

/*===========================================================================

FUNCTION: bit_handle_open_result

DESCRIPTION:
  This function is used to handle the open result. It will return SUCCESS,
  WAIT or Error Code for open operation
===========================================================================*/
static gpsone_bit_status_e_type bit_handle_open_result(
  gpsone_bit_status_e_type status,
  boolean allow_wait
)
{
  BIT_MSG_FUNC( "bit_handle_open_result: status=0x%x allow_wait=%d", status, allow_wait, 0 );

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    if (cur_bit_state == GPSONE_BIT_STATE_OPENING)  // make sure we are in OPENING state
    {
      bit_handle_change_state(GPSONE_BIT_STATE_OPENED); // from OPENING to OPENED
      BIT_MSG_HIGH( "BIT transport opened successfully", 0, 0, 0 );

      bit_handle_saved_connect_ioctl(TRUE, GPSONE_BIT_STATUS_SUCCESS); // open success
    }
  }
  else if (allow_wait && status == GPSONE_BIT_STATUS_WAIT)
  {
    // pending, wait for OPEN_RESULT event
  }
  else
  {
     bit_handle_change_state(GPSONE_BIT_STATE_CLOSED);  // from OPENING to CLOSED
     BIT_MSG_ERROR( "Failed to open BIT transport, error=0x%x", status, 0, 0 );

     bit_handle_saved_connect_ioctl(FALSE, GPSONE_BIT_STATUS_OPEN_FAILED); // open failed
  }

  return status;
}

/*===========================================================================

FUNCTION: bit_handle_close_result

DESCRIPTION:
  This function is used to handle the close result. It will return SUCCESS,
  WAIT or Error Code for close operation
===========================================================================*/
static gpsone_bit_status_e_type bit_handle_close_result(
  gpsone_bit_status_e_type status,
  boolean allow_wait
)
{
  BIT_MSG_FUNC( "bit_handle_close_result: status=0x%x allow_wait=%d", status, allow_wait, 0 );

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    if (cur_bit_state == GPSONE_BIT_STATE_CLOSING)  // make sure we are in CLOSING state
    {
      bit_handle_change_state(GPSONE_BIT_STATE_CLOSED); // from CLOSING to CLOSED
      BIT_MSG_HIGH( "BIT transport closed successfully", 0, 0, 0 );
    }
  }
  else if (allow_wait && status == GPSONE_BIT_STATUS_WAIT)
  {
    // pending, wait for CLOSE_RESULT event
  }
  else
  {
    bit_handle_change_state(GPSONE_BIT_STATE_CLOSED);  // from CLOSING to CLOSED
    BIT_MSG_ERROR( "Failed to close BIT transport! Mark it as CLOSED anyway", 0, 0, 0 );
  }

  /*   //
  if (status != GPSONE_BIT_STATUS_WAIT) //  success or failure, generate an event for PDCOMM task to clean up
  {
      gpsone_bit_event_payload_type    event_payload;

      event_payload.event = GPSONE_BIT_EVENT_CLOSE_RESULT;
      event_payload.result = GPSONE_BIT_STATUS_SUCCESS;

      gpsone_bit_notify(cur_bit_handle, GPSONE_BIT_INVALID_SESSION_HANDLE, 0, &event_payload);
  }
  */

  // make sure no saved connect/ioctl after BIT daemon is closed.
  bit_reset_saved_connect_ioctl();

  return status;
}

/*===========================================================================

FUNCTION bit_handle_deregistration

DESCRIPTION
  This function handles deregistration (non-RPC local call)
===========================================================================*/
static gpsone_bit_status_e_type bit_handle_deregistration
(
  gpsone_bit_transport_handle_type     transport_handle
)
{
  BIT_MSG_FUNC( "bit_handle_deregistration: state=%d", cur_bit_state, 0, 0 );

  // currently not registered or transport handle doesn't match, return error
  if (cur_bit_state == GPSONE_BIT_STATE_UNINITIALIZED || cur_bit_state == GPSONE_BIT_STATE_UNREGISTERED ||
      transport_handle == GPSONE_BIT_INVALID_TRANSPORT_HANDLE || transport_handle != cur_bit_handle)
  {
    return GPSONE_BIT_STATUS_NOT_REGISTERED;
  }

  // state is OPENING/OPENED/CLOSING/CLOSED
  switch(cur_bit_state)
  {
  case GPSONE_BIT_STATE_OPENING:
    // got deregister during OPENING state? fail all the saved requests
    bit_handle_saved_connect_ioctl(FALSE, GPSONE_BIT_STATUS_NOT_REGISTERED); // not registered
    break;

  case GPSONE_BIT_STATE_OPENED:
    // TODO: There may be some pending connect/disconnect/send/recv/ioctl operations, need to send failure event
    // Or we can rely on the timeout mechanism
    BIT_MSG_ERROR( "Trying to deregister while BIT daemon is still OPENED!", 0, 0, 0 );
    break;

  case GPSONE_BIT_STATE_CLOSING:
    bit_handle_close_result(GPSONE_BIT_STATUS_NOT_REGISTERED, FALSE); // WAIT is not allowed, will force it to CLOSED
    break;

  case GPSONE_BIT_STATE_CLOSED:
    // already in CLOSED, don't need to do anything
    break;

  default:
    break;
  }

  // final cleanup
  bit_handle_deregister_cleanup();

  return GPSONE_BIT_STATUS_SUCCESS;
}



/*****************************************************************************/
/*      Routines for PDCOMM TCP Task (non-RPC, BIT callback wrappers, etc)   */
/*****************************************************************************/


/*===========================================================================

FUNCTION: gpsone_bit_init

DESCRIPTION:
  This function is used by PDSM during startup to initialize BIT (mutex
  setup, etc). If it's not called once, gpsone_bit_register() will automatically
  call it.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_init
(
 void
)
{
  boolean ret;
  int i;

  BIT_MSG_FUNC( "gpsone_bit_init: state=%d", cur_bit_state, 0, 0 );

    /* protect data access by mutex */
  ret = os_MutexInit(&z_bit_mutex, MUTEX_DATA_ONLY_CONTEXT);

  if (ret == TRUE)
    ret = os_MutexInit(&z_bit_save_mutex, MUTEX_DATA_ONLY_CONTEXT);

  if (ret == FALSE)
  {
    BIT_MSG_ERROR("Mutex init failed!",0,0,0);
    return GPSONE_BIT_STATUS_FATAL_ERROR;
  }

  cur_bit_state = GPSONE_BIT_STATE_UNREGISTERED;

  os_MutexLock(&z_bit_save_mutex);
  for(i = 0; i < BIT_MAX_SAVED_CONNECT_REQUESTS; i++)
    bit_saved_connects[i].valid = FALSE;

  for(i = 0; i < BIT_MAX_SAVED_IOCTL_REQUESTS; i++)
    bit_saved_ioctls[i].valid = FALSE;
  os_MutexUnlock(&z_bit_save_mutex);

  return GPSONE_BIT_STATUS_SUCCESS;
}

/*===========================================================================

FUNCTION: gpsone_bit_open

DESCRIPTION:
  This function is used to inform BIT daemon that PDSM will start using its
  service. Upon success, this function should return GPSONE_BIT_STATUS_SUCCESS.
  If for some reason, BIT daemon fails to initialize, it should return
  GPSONE_BIT_STATUS_FAIL or other error code. Otherwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that BIT daemon is in the process of being
  brought up and open result will be sent asynchronously via gpsone_bit_notify.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_open
(
  gpsone_bit_open_params_type       *open_param
)
{
  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_open: state=%d (tp_handle=%x)", cur_bit_state, cur_bit_handle, 0 );

  os_MutexLock(&z_bit_mutex);  // protect against deregister

  if (cur_bit_transport == NULL)
  {
    status = GPSONE_BIT_STATUS_NOT_REGISTERED;
  }
  else
  {
    // only allow open() call when we are in CLOSED state, not CLOSING/OPENING/OPENED
    status = bit_handle_check_and_change_state(GPSONE_BIT_STATE_CLOSED, (void *)cur_bit_transport->open_fp,
                                               GPSONE_BIT_STATE_OPENING);
  }

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    status = cur_bit_transport->open_fp(cur_bit_handle, open_param);
    BIT_MSG_HIGH("Called BIT daemon open, status=%d", status, 0, 0);

    //status = bit_handle_open_result(status, TRUE); // allow wait
    if (status != GPSONE_BIT_STATUS_WAIT)  // immediate success or failure
    {
      gpsone_bit_event_payload_type event_payload;

      event_payload.event = GPSONE_BIT_EVENT_OPEN_RESULT;
      event_payload.result = status;

      gpsone_bit_notify(cur_bit_handle, GPSONE_BIT_INVALID_SESSION_HANDLE, 0, &event_payload);
      status = GPSONE_BIT_STATUS_WAIT;  // change status to WAIT for consistent behavior
    }
  }

  os_MutexUnlock(&z_bit_mutex);

  return status;
}


/*===========================================================================

FUNCTION: gpsone_bit_close

DESCRIPTION:
  This function is used to inform BIT daemon that PDSM will stop using its
  service. Upon success, this function should return GPSONE_BIT_STATUS_SUCCESS.
  If for some reason, BIT daemon fails to close, it should return
  GPSONE_BIT_STATUS_FAIL or other error code. Otherwise, it should return
  GPSONE_BIT_STATUS_WAIT to inform PDSM that BIT daemon is in the process of being
  shut down and open result will be sent asynchronously via gpsone_bit_notify.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_close
(
  gpsone_bit_close_params_type       *close_param
)
{
  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_close: state=%d (tp_handle=%x)", cur_bit_state, cur_bit_handle, 0 );

  os_MutexLock(&z_bit_mutex);

  if (cur_bit_transport == NULL)
  {
    status = GPSONE_BIT_STATUS_NOT_REGISTERED;
  }
  else
  {
    // only allow close() call when we are in OPENED state (not OPENING/CLOSING)
    status = bit_handle_check_and_change_state(GPSONE_BIT_STATE_OPENED, (void *)cur_bit_transport->close_fp,
                                             GPSONE_BIT_STATE_CLOSING);
  }

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    status = cur_bit_transport->close_fp(cur_bit_handle, close_param);
    BIT_MSG_HIGH("Called BIT daemon close, status=%d", status, 0, 0);
    //status = bit_handle_close_result(status, TRUE); // allow WAIT

    if (status != GPSONE_BIT_STATUS_WAIT)  // immediate success or failure
    {
      gpsone_bit_event_payload_type event_payload;

      event_payload.event = GPSONE_BIT_EVENT_CLOSE_RESULT;
      event_payload.result = status;

      gpsone_bit_notify(cur_bit_handle, GPSONE_BIT_INVALID_SESSION_HANDLE, 0, &event_payload);
      status = GPSONE_BIT_STATUS_WAIT;   // change status to WAIT for consistent behavior
    }
  }

  os_MutexUnlock(&z_bit_mutex);

  return status;
}


/*===========================================================================

FUNCTION: gpsone_bit_connect

DESCRIPTION:
  This function is used to establish end-to-end connection to a network server.
  If no end-to-end connection needs to be established, this function should
  return GPSONE_BIT_STATUS_SUCCESS. If for some reason, end-to-end
  connection can not be established, it should return GPSONE_BIT_STATUS_FAIL.
  Otherwise, it should return GPSONE_BIT_STATUS_WAIT to inform PDSM
  that end-to-end connection is in the process of being established.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_connect
(
  uint32                            transaction_id,
  gpsone_bit_connect_params_type    *connect_param
)
{
  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_connect: state=%d (tp_handle=%x)", cur_bit_state, cur_bit_handle, 0 );

  // delayed open handling, can't grab mutex here since bit_handle_delayed_open() may need it!
  status = bit_handle_delayed_open();
  if (status == GPSONE_BIT_STATUS_WAIT)
  {
    // open pending, save & delay connect
    if (bit_save_connect_request(transaction_id, connect_param) != GPSONE_BIT_STATUS_SUCCESS)
      return GPSONE_BIT_STATUS_NO_RESOURCES;  // can't save it
  }

  if (status != GPSONE_BIT_STATUS_SUCCESS)
  {
    return status;  // WAIT or failure, can't continue
  }

  os_MutexLock(&z_bit_mutex);

  if (cur_bit_transport == NULL)
  {
    status = GPSONE_BIT_STATUS_NOT_REGISTERED;
  }
  else
  {
    // only allow connect() call when we are in OPENED state (not OPENING/CLOSING)
    status = bit_handle_check_and_change_state(GPSONE_BIT_STATE_OPENED, (void *)cur_bit_transport->connect_fp,
                                  GPSONE_BIT_STATE_UNCHANGED);
  }

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    status = cur_bit_transport->connect_fp(cur_bit_handle, transaction_id, connect_param);
    BIT_MSG_HIGH("Called BIT daemon connect, status=%d", status, 0, 0);
  }

  os_MutexUnlock(&z_bit_mutex);

  return status;
}


/*===========================================================================

FUNCTION: gpsone_bit_disconnect

DESCRIPTION:
  This function is used to tear down end-to-end connection to a network server.
  If no end-to-end connection needs to be torn down, this function should return
  GPSONE_BIT_STATUS_SUCCESS. If for some reason, end-to-end
  connection can not be torn down, it should return GPSONE_BIT_STATUS_FAIL.
  Otherwise, it should return GPSONE_BIT_STATUS_WAIT to inform PDSM that
  end-to-end connection is in the process of being torn down.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_disconnect
(
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  gpsone_bit_disconnect_params_type *disconnect_param
)
{
  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_disconnect: state=%d (session_handle=0x%x)", cur_bit_state, session_handle, 0 );

  os_MutexLock(&z_bit_mutex);

  if (cur_bit_transport == NULL)
  {
    status = GPSONE_BIT_STATUS_NOT_REGISTERED;
  }
  else
  {
    // only allow disconnect() call when we are in OPENED state (not UNREGISTERED/OPENING/CLOSING/CLOSED)
    status = bit_handle_check_and_change_state(GPSONE_BIT_STATE_OPENED, (void *)cur_bit_transport->disconnect_fp,
                                  GPSONE_BIT_STATE_UNCHANGED);
  }

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    if (session_handle == GPSONE_BIT_INVALID_SESSION_HANDLE)
      status = GPSONE_BIT_STATUS_INVALID_PARAM;
    else
    {
      status = cur_bit_transport->disconnect_fp(cur_bit_handle, session_handle, transaction_id, disconnect_param);
      BIT_MSG_HIGH("Called BIT daemon disconnect, session_handle=0x%x status=%d", session_handle, status, 0);
    }
  }

  os_MutexUnlock(&z_bit_mutex);

  return status;
}


/*===========================================================================

FUNCTION: gpsone_bit_send

DESCRIPTION:
  This function is used by PDSMto send data to the network. It passes a pointer
  to a data buffer (*start) and also specifies the size (in bytes) of the data buffer.
  If for some reason, transport layer proxy is not able to process the send request,
  it should return GPSONE_BIT_STATUS_FAIL or other error code to inform PDSM that the
  send request has failed. Otherwise, it should return GPSONE_BIT_STATUS_WAIT and return
  result asynchronously via gpsone_bit_notify()

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_send
(
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  uint8                             *send_buf,
  uint32                            length
)
{
  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_send: state=%d (session_handle=0x%x length=%d)", cur_bit_state, session_handle, length );

  os_MutexLock(&z_bit_mutex);

  if (cur_bit_transport == NULL)
  {
    status = GPSONE_BIT_STATUS_NOT_REGISTERED;
  }
  else if (send_buf == NULL || length <= 0 || length > GPSONE_BIT_MAX_BUF_SIZE ||
           session_handle == GPSONE_BIT_INVALID_SESSION_HANDLE)
  {
    status = GPSONE_BIT_STATUS_INVALID_PARAM;
  }
  else
  {
    // only allow send() call when we are in OPENED state (not UNREGISTERED/OPENING/CLOSING/CLOSED)
    status = bit_handle_check_and_change_state(GPSONE_BIT_STATE_OPENED, (void *)cur_bit_transport->send_fp,
                                             GPSONE_BIT_STATE_UNCHANGED);
  }

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    status = cur_bit_transport->send_fp(cur_bit_handle, session_handle, transaction_id, send_buf, length);
    BIT_MSG_HIGH("Called BIT daemon send, session_handle=0x%x length=%d status=%d", session_handle, length, status);
  }

  os_MutexUnlock(&z_bit_mutex);

  return status;
}


/*===========================================================================

FUNCTION: gpsone_bit_recv

DESCRIPTION:
  This function is used by PDSM to receive data from transport layer proxy,
  when it gets read event from BIT daemon. Upon getting recv request,
  BIT daemon should copy incoming data to the data buffer pointed
  by *start. If incoming data size is larger than max_buff_size, it should only
  copy max_buf_size to the data buffer pointed by *start.
  BIT daemon should return the number of bytes copied to the data buffer.
  A value of -1 for bytes_returned is a special case which means BIT daemon gets
  EOF from the network and it is used to inform PDSM to tear down the connection.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_recv
(
  gpsone_bit_session_handle_type     session_handle,
  uint8                              *recv_buf,
  uint32                             max_buf_size,
  uint32                             *bytes_returned,
  uint32                             *bytes_leftover
)
{
  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_recv: state=%d (session_handle=0x%x)", cur_bit_state, session_handle, 0 );

  os_MutexLock(&z_bit_mutex);

  if (cur_bit_transport == NULL)
  {
    status = GPSONE_BIT_STATUS_NOT_REGISTERED;
  }
  else if (session_handle == GPSONE_BIT_INVALID_SESSION_HANDLE || recv_buf == NULL ||
      bytes_returned == NULL || bytes_leftover == NULL || max_buf_size <= 0 || max_buf_size > GPSONE_BIT_MAX_BUF_SIZE)
  {
    status = GPSONE_BIT_STATUS_INVALID_PARAM;
  }
  else
  {
    // only allow recv() call when we are in OPENED state (not UNREGISTERED/OPENING/CLOSING/CLOSED)
    status = bit_handle_check_and_change_state(GPSONE_BIT_STATE_OPENED, (void *)cur_bit_transport->recv_fp,
                                               GPSONE_BIT_STATE_UNCHANGED);
  }

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    status = cur_bit_transport->recv_fp(cur_bit_handle, session_handle, recv_buf, max_buf_size, bytes_returned, bytes_leftover);
    BIT_MSG_HIGH("Called BIT daemon recv: status=%d bytes=%d left=%d", status, *bytes_returned, *bytes_leftover);
  }

  os_MutexUnlock(&z_bit_mutex);

  return status;
}


/*===========================================================================

FUNCTION: pgpsone_bit_ioctl

DESCRIPTION:
  This function is used by PDSM to request ioctl operation on BIT daemon
  Result will be sent back asynchronously by gpsone_bit_notify() later with
  IOCTL_RESULT event.
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_ioctl
(
 gpsone_bit_session_handle_type     session_handle,
 uint32                             transaction_id,
 gpsone_bit_ioctl_e_type            ioctl_request,
 gpsone_bit_ioctl_params_type       *ioctl_param
)
{
  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_ioctl: state=%d (session_handle=0x%x ioctl_request=%d)", cur_bit_state, session_handle, ioctl_request );

  // delayed open handling, can't grab mutex here since bit_handle_delayed_open() may need it!
  status = bit_handle_delayed_open();
  if (status == GPSONE_BIT_STATUS_WAIT)
  {
    // open pending, save & delay ioctl
    if (bit_save_ioctl_request(session_handle, transaction_id, ioctl_request, ioctl_param) != GPSONE_BIT_STATUS_SUCCESS)
      return GPSONE_BIT_STATUS_NO_RESOURCES;  // can't save it
  }

  if (status != GPSONE_BIT_STATUS_SUCCESS)
  {
    return status;  // WAIT or failure, can't continue
  }

  os_MutexLock(&z_bit_mutex);

  if (cur_bit_transport == NULL)
  {
    status = GPSONE_BIT_STATUS_NOT_REGISTERED;
  }
  else
  {
    // only allow ioctl() call when we are in OPENED state (not UNREGISTERED/OPENING/CLOSING/CLOSED)
    status = bit_handle_check_and_change_state(GPSONE_BIT_STATE_OPENED, (void *)cur_bit_transport->ioctl_fp,
                                               GPSONE_BIT_STATE_UNCHANGED);
  }

  if (status == GPSONE_BIT_STATUS_SUCCESS)
  {
    /*
    if (session_handle == GPSONE_BIT_INVALID_SESSION_HANDLE)
    {
      status = GPSONE_BIT_STATUS_INVALID_PARAM;
      // We don't need to check session_handle here since ioctl can be a global operation
    }
    else
    */
    {
      status = cur_bit_transport->ioctl_fp(cur_bit_handle, session_handle, transaction_id, ioctl_request, ioctl_param);
      BIT_MSG_HIGH("Called BIT daemon ioctl: request=%d status=%d", ioctl_request, status, 0);
    }
  }

  os_MutexUnlock(&z_bit_mutex);

  return status;
}

/*===========================================================================
FUNCTION gpsone_bit_process_event

DESCRIPTION
  This function is called by PDCOMM TCP Task if some events need processing
  by BIT API layer within PDCOMM TCP Task context (mainly for OPEN_RESULT &
  CLOSE_RESULT)
===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_process_event
(
  gpsone_bit_event_payload_type    *event_payload
)
{
  BIT_MSG_FUNC( "gpsone_bit_process_event: state=%x event=%d", cur_bit_state, 0, 0 );

  switch(event_payload->event)
  {
  case GPSONE_BIT_EVENT_OPEN_RESULT:       /* event to report result for open */
    //bit_open_cb(event_payload->result);  // handle locally
    if (cur_bit_state == GPSONE_BIT_STATE_OPENING)
    {
      bit_handle_open_result(event_payload->result, FALSE);  // no WAIT status allowed
    }
    break;

  case GPSONE_BIT_EVENT_CLOSE_RESULT:      /* event to report result for close */
    if (cur_bit_state == GPSONE_BIT_STATE_CLOSING)
    {
      bit_handle_close_result(event_payload->result, FALSE); // no WAIT status allowed
    }
    break;

  default:
    //  no processing needed
    break;
  }

  return GPSONE_BIT_STATUS_SUCCESS;
}

#endif /* FEATURE_GNSS_BIT_API */


/*****************************************************************************/
/*          Public AGPS BIT API (RPC, for BIT daemon on Apps to call)        */
/*****************************************************************************/


/*===========================================================================

FUNCTION gpsone_bit_register

DESCRIPTION
  This function is called by BIT daemon to register with PDCOMM TCP Task
  (callback functions need to be top-level parameters due to ONCRPC)

DEPENDENCIES

RETURN VALUE
  GPSONE_BIT_STATUS_SUCCESS if registration is successful, otherwise Error Code

SIDE EFFECTS

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_register
(
  const gpsone_bit_register_params_type     *reg_param,
  gpsone_bit_transport_handle_type          *transport_handle,
  gpsone_bit_open_f_type                    *open_fp,
  gpsone_bit_close_f_type                   *close_fp,
  gpsone_bit_connect_f_type                 *connect_fp,
  gpsone_bit_disconnect_f_type              *disconnect_fp,
  gpsone_bit_send_f_type                    *send_fp,
  gpsone_bit_recv_f_type                    *recv_fp,
  gpsone_bit_ioctl_f_type                   *ioctl_fp
)
{
#ifdef FEATURE_GNSS_BIT_API

  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_register: state=%d", cur_bit_state, 0, 0 );

  if (reg_param == NULL || transport_handle == NULL || reg_param->bit_type != GPSONE_BIT_TYPE_IP ||
      open_fp == NULL || close_fp == NULL || connect_fp == NULL || disconnect_fp == NULL ||
      send_fp == NULL || recv_fp == NULL || ioctl_fp == NULL)
  {
    BIT_MSG_ERROR("Invalid param for bit_register()!", 0, 0, 0 );
    return GPSONE_BIT_STATUS_INVALID_PARAM;
  }

  if (cur_bit_state == GPSONE_BIT_STATE_UNINITIALIZED)
  {
    // TODO: this can be called from PDCOMM task init
    BIT_MSG_HIGH( "gpsone_bit_register(), intializing BIT API...", 0, 0, 0 );
    status = gpsone_bit_init();
    if (status != GPSONE_BIT_STATUS_SUCCESS)
      return status;
  }

  os_MutexLock(&z_bit_mutex);  // no other BIT API call can happen

  // can be in OPENING/OPENED/CLOSING/CLOSED state
  if (cur_bit_state != GPSONE_BIT_STATE_UNREGISTERED)
  {
    // call non-RPC local function, make sure previous instance is deregistered
    // need to handle re-registration case (BIT daemon crashes)
    bit_handle_deregistration(cur_bit_handle);
  }

  // at this point, BIT API should have been initialized(mutex, etc) and unregistered
  cur_bit_handle = ++bit_handle_counter; // increase counter and assign it to active handle
  *transport_handle = cur_bit_handle; // pass transport handle back to BIT daemon

  bit_transport_array[0].reg_param = *reg_param;
  bit_transport_array[0].open_fp = open_fp;
  bit_transport_array[0].close_fp = close_fp;
  bit_transport_array[0].connect_fp = connect_fp;
  bit_transport_array[0].disconnect_fp = disconnect_fp;
  bit_transport_array[0].send_fp = send_fp;
  bit_transport_array[0].recv_fp = recv_fp;
  bit_transport_array[0].ioctl_fp = ioctl_fp;

  cur_bit_transport = &(bit_transport_array[0]);

  cur_bit_state = GPSONE_BIT_STATE_CLOSED;  // BIT: registered but currently closed

  // state is CLOSED now, just in case, make sure there is no saved requests
  bit_reset_saved_connect_ioctl();

  os_MutexUnlock(&z_bit_mutex);

  BIT_MSG_HIGH( "gpsone_bit_register() returns transport handle 0x%x", cur_bit_handle, 0, 0 );

  return GPSONE_BIT_STATUS_SUCCESS;

#else
  return GPSONE_BIT_STATUS_NOT_IMPLEMENTED;
#endif /* FEATURE_GNSS_BIT_API */
}


/*===========================================================================

FUNCTION gpsone_bit_deregister

DESCRIPTION
  This function is called by BIT daemon to deregister with PDCOMMS TCP task

DEPENDENCIES

RETURN VALUE
  GPSONE_BIT_STATUS_SUCCESS if deregistration is successful, otherwise Error Code

SIDE EFFECTS

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_deregister
(
  gpsone_bit_transport_handle_type     transport_handle
)
{
#ifdef FEATURE_GNSS_BIT_API

  gpsone_bit_status_e_type status;

  BIT_MSG_FUNC( "gpsone_bit_deregister: state=%d", cur_bit_state, 0, 0 );

  os_MutexLock(&z_bit_mutex); // no other BIT API call can happen

  // call non-RPC local function
  status = bit_handle_deregistration(transport_handle);

  os_MutexUnlock(&z_bit_mutex);

  return status;

#else
  return GPSONE_BIT_STATUS_NOT_IMPLEMENTED;
#endif /* FEATURE_GNSS_BIT_API */
}


/*===========================================================================

FUNCTION gpsone_bit_notify

DESCRIPTION
  This function is called by BIT daemon to notify PDCOMM TCP task about
  asynchronous events and command results

DEPENDENCIES

RETURN VALUE
  GPSONE_BIT_STATUS_SUCCESS if PDSM handles the notification succesfully,
  otherwise Error Code

SIDE EFFECTS

===========================================================================*/
gpsone_bit_status_e_type  gpsone_bit_notify
(
  gpsone_bit_transport_handle_type       transport_handle,
  gpsone_bit_session_handle_type         session_handle,
  uint32                                 transaction_id,
  const gpsone_bit_event_payload_type    *event_payload
)
{
#ifdef FEATURE_GNSS_BIT_API

  BIT_MSG_FUNC( "gpsone_bit_notify: state=%d transport_handle=%d transid=0x%x", cur_bit_state, transport_handle, transaction_id );

  // currently not registered or transport handle doesn't match, return error
  if (cur_bit_state == GPSONE_BIT_STATE_UNINITIALIZED || cur_bit_state == GPSONE_BIT_STATE_UNREGISTERED
       || transport_handle != cur_bit_handle)
  {
    BIT_MSG_ERROR( "Got BIT event without registered daemon!", 0, 0, 0 );
    return GPSONE_BIT_STATUS_NOT_REGISTERED;
  }

  BIT_MSG_HIGH( "Got BIT event 0x%x (result=0x%x, session_handle=%d)", event_payload->event, event_payload->result, session_handle );

  switch(event_payload->event)
  {
  case GPSONE_BIT_EVENT_OPEN_RESULT:       /* event to report result for open */
    if (cur_bit_state != GPSONE_BIT_STATE_OPENING)
    {
      BIT_MSG_ERROR( "Got BIT event OPEN_RESULT while not in OPENING state", 0, 0, 0 );
      return GPSONE_BIT_STATUS_NOT_ALLOWED;
    }
    break;

  case GPSONE_BIT_EVENT_CLOSE_RESULT:      /* event to report result for close */
    if (cur_bit_state != GPSONE_BIT_STATE_CLOSING)
    {
      BIT_MSG_ERROR( "Got BIT event CLOSE_RESULT while not in CLOSING state", 0, 0, 0 );
      return GPSONE_BIT_STATUS_NOT_ALLOWED;
    }
    break;

  case GPSONE_BIT_EVENT_CONNECT_RESULT:    /* event to report result for connect */
  case GPSONE_BIT_EVENT_DISCONNECT_RESULT: /* event to report result for disconnect */
  case GPSONE_BIT_EVENT_SEND_RESULT:       /* event to report result for send */
  case GPSONE_BIT_EVENT_IOCTL_RESULT:      /* event to report result for ioctl */
  case GPSONE_BIT_EVENT_DATA_READY:        /* event to notify PDSM of new data ready */
  case GPSONE_BIT_EVENT_NETWORK_STATUS:    /* event to notify PDSM of network status change */
    if (cur_bit_state != GPSONE_BIT_STATE_OPENED && cur_bit_state != GPSONE_BIT_STATE_CLOSING)
    {
      BIT_MSG_ERROR( "Got BIT event 0x%x while not in OPENED state", event_payload->event, 0, 0 );
      return GPSONE_BIT_STATUS_NOT_ALLOWED;
    }

    // DEBUG code, close BIT daemon upon failure, should comment out for production
    /*
    if (event_payload->result != GPSONE_BIT_STATUS_SUCCESS)
    {
      gpsone_bit_close_params_type close_param;
      close_param.modem_restarting = FALSE;
      close_param.force_dormancy = FALSE;
      gpsone_bit_close(&close_param);  // in caller's context?
    }
    */

    // do nothing additional here, just send the event to PDCOMM task
    break;

  default:
    BIT_MSG_ERROR( "Got unrecognized BIT event 0x%x!", event_payload->event, 0, 0 );
    return GPSONE_BIT_STATUS_INVALID_PARAM; /* unknown event ? */
  }

  // pass it to upper layer
  return gpsone_bit_post_event(session_handle, transaction_id, event_payload); // success or failure

#else
  return GPSONE_BIT_STATUS_NOT_IMPLEMENTED;
#endif /* FEATURE_GNSS_BIT_API */
}





