/*============================================================================
  @file sns_time_mr.c

  @brief
  Implementes the message routing and handling portion of the
  sensors time sync service.

  <br><br>

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================
  INCLUDE FILES
============================================================================*/
#include "sensor1.h"
#include "sns_common.h"
#include "sns_smr_util.h"
#include "sns_debug_str.h"
#include "sns_memmgr.h"
#include "sns_init.h"
#include "sns_time_priv.h"
#include "sns_osa.h"
#include "sns_time_api_v02.h"
#include <stdbool.h>
#include <qmi_csi.h>

static qmi_csi_os_params os_params;
static qmi_csi_service_handle service_handle;

#ifdef _WIN32
  static KEVENT qmi_event;  /* event for os_params */
  static KEVENT exit_event; /* event to exit thread */
#endif

/*============================================================================
  FUNCTION DEFINITIONS
==========================================================================*/

/**
 * Callback function for QMI framework
 */
qmi_csi_cb_error
sns_time_mr_csi_connect( qmi_client_handle client_handle, void *service_cookie,
    void **connection_handle )
{
  intptr_t *client_hndl;

  UNREFERENCED_PARAMETER(client_handle);
  UNREFERENCED_PARAMETER(service_cookie);

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_TIME, "Received connect" );

  client_hndl = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_TIME, sizeof(qmi_client_handle) );
  if( NULL == client_hndl )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_TIME, "alloc failed" );
    return QMI_CSI_NO_MEM;
  }

  *client_hndl = (intptr_t)client_handle;
  *connection_handle = client_hndl;
  return QMI_CSI_NO_ERR;
}

/**
 * Callback function for QMI framework
 */
void
sns_time_mr_csi_disconnect( void *connection_handle, void *service_cookie )
{
  UNREFERENCED_PARAMETER(connection_handle);
  UNREFERENCED_PARAMETER(service_cookie);

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_TIME, "Received disconnect" );

  sns_time_client_delete( *((intptr_t*)connection_handle) );
  SNS_OS_FREE( connection_handle );
}

/**
 * Handle a cancel message, and generate the response.
 *
 * @param[o] resp_msg Response message
 * @param[0] body_len Length of the response message
 *
 * @return Error code
 */
static qmi_csi_cb_error
sns_time_mr_handle_cancel( sns_common_cancel_resp_msg_v01 **resp_msg,
    unsigned int *body_len )
{
  qmi_csi_cb_error rv = QMI_CSI_CB_NO_ERR;

  *resp_msg = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_TIME, sizeof(sns_common_cancel_resp_msg_v01) );
  if( NULL == *resp_msg )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_TIME, "msg alloc failed" );
    return QMI_CSI_CB_NO_MEM;
  }
  else
  {
    (*resp_msg)->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
    (*resp_msg)->resp.sns_err_t = SENSOR1_SUCCESS;
    *body_len = sizeof( sns_common_cancel_resp_msg_v01 );
  }

  return rv;
}

/**
 * Handle a timestamp request message, and generate the response.
 *
 * @param[o] resp_msg Response message
 * @param[0] body_len Length of the response message
 *
 * @return Error code
 */
static qmi_csi_cb_error
sns_time_mr_handle_timestamp( sns_time_timestamp_resp_msg_v02 **resp_msg, unsigned int *body_len )
{
  int error;
  sns_time_timestamp_resp_msg_v02 *resp_msg_ptr;

  *resp_msg = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_TIME, sizeof(sns_time_timestamp_resp_msg_v02) );
  if( NULL == *resp_msg )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_TIME, "msg alloc failed" );
    return QMI_CSI_CB_NO_MEM;
  }
  else
  {
    resp_msg_ptr = *resp_msg;

    error = sns_time_generate( &resp_msg_ptr->timestamp_apps,
                               &resp_msg_ptr->timestamp_dsps,
                               &resp_msg_ptr->dsps_rollover_cnt,
                               &resp_msg_ptr->timestamp_apps_boottime );
    *body_len = sizeof(sns_time_timestamp_resp_msg_v02);

    if( 0 != error )
    {
      resp_msg_ptr->timestamp_apps_valid = false;
      resp_msg_ptr->timestamp_dsps_valid = false;
      resp_msg_ptr->dsps_rollover_cnt_valid = false;
      resp_msg_ptr->timestamp_apps_boottime_valid = false;
      resp_msg_ptr->error_code = SENSOR_TIME_EINTERNAL_V02;
    }
    else
    {
      resp_msg_ptr->timestamp_apps_valid = true;
      resp_msg_ptr->timestamp_dsps_valid = true;
      resp_msg_ptr->dsps_rollover_cnt_valid = true;
      resp_msg_ptr->timestamp_apps_boottime_valid = true;
      resp_msg_ptr->error_code = SENSOR_TIME_ESUCCESS_V02;
    }

    resp_msg_ptr->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
    resp_msg_ptr->resp.sns_err_t = SENSOR1_SUCCESS;
  }

  return QMI_CSI_CB_NO_ERR;
}

/**
 * Handle a version request message, and generate the response.
 *
 * @param[o] resp_msg Response message
 * @param[0] body_len Length of the response message
 *
 * @return Error code
 */
static qmi_csi_cb_error
sns_time_mr_handle_version( sns_common_version_resp_msg_v01 **resp_msg,
    unsigned int *body_len )
{
  qmi_csi_cb_error rv = QMI_CSI_CB_NO_ERR;

  *resp_msg = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_TIME,
      sizeof(sns_common_version_resp_msg_v01) );
  if( NULL == *resp_msg )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_TIME, "msg alloc failed" );
    return QMI_CSI_CB_NO_MEM;
  }
  else
  {
    (*resp_msg)->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
    (*resp_msg)->resp.sns_err_t = SENSOR1_SUCCESS;
    (*resp_msg)->interface_version_number = SNS_TIME2_SVC_V02_IDL_MINOR_VERS;
    (*resp_msg)->max_message_id = SNS_TIME2_SVC_V02_MAX_MESSAGE_ID;

    *body_len = sizeof( sns_common_version_resp_msg_v01 );
  }

  return rv;
}

void
sns_time_mr_send_ind( intptr_t client_hndl )
{
  sns_time_timestamp_ind_msg_v02 ind_msg;
  qmi_csi_error qmi_err;
  int error;

  error = sns_time_generate( &ind_msg.timestamp_apps,
                             &ind_msg.timestamp_dsps, &ind_msg.dsps_rollover_cnt,
                             &ind_msg.timestamp_apps_boottime );
  if( 0 != error )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                               "Error preparing ind %i", error );
  }
  else
  {
    ind_msg.timestamp_apps_boottime_valid = true;
    qmi_err = qmi_csi_send_ind( (qmi_client_handle)client_hndl,
        SNS_TIME_TIMESTAMP_IND_V02, &ind_msg, sizeof(sns_time_timestamp_ind_msg_v02) );

    if( QMI_CSI_NO_ERR != qmi_err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                                "CSI send ind failure %i", qmi_err );
    }
  }
}

/*!
  @brief  Callback function requests made to the time service.  Handle the
  request and send back the response.

  See QMI documentation for function parameters and return values.

*/
qmi_csi_cb_error
sns_time_mr_csi_process_req( void *connection_handle, qmi_req_handle req_handle,
    unsigned int msg_id, void *req_c_struct, unsigned int req_c_struct_len,
    void *service_cookie )
{
  qmi_csi_error csi_err;
  qmi_csi_cb_error rv = QMI_CSI_CB_NO_ERR;
  void *resp_msg_ptr = NULL;
  unsigned int body_len;

  UNREFERENCED_PARAMETER(service_cookie);
  UNREFERENCED_PARAMETER(req_c_struct_len);

  SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_TIME, "Received request %i", msg_id );

  if( SNS_TIME_CANCEL_REQ_V02 == msg_id )
  {
    sns_time_client_delete( *(intptr_t*)connection_handle );
    rv = sns_time_mr_handle_cancel( (sns_common_cancel_resp_msg_v01**)&resp_msg_ptr, &body_len );
  }
  else if( SNS_TIME_TIMESTAMP_REQ_V02 == msg_id )
  {
    rv = sns_time_mr_handle_timestamp( (sns_time_timestamp_resp_msg_v02**)&resp_msg_ptr, &body_len );

    if( ((sns_time_timestamp_req_msg_v02*)req_c_struct)->reg_report_valid &&
        ((sns_time_timestamp_req_msg_v02*)req_c_struct)->reg_report )
    {
      sns_time_client_add( *(intptr_t*)connection_handle );
    }
  }
  else if( SNS_TIME_VERSION_REQ_V02 == msg_id )
  {
    rv = sns_time_mr_handle_version( (sns_common_version_resp_msg_v01**)&resp_msg_ptr, &body_len );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
        "Unknown request message %i", msg_id );
    rv = QMI_CSI_CB_UNSUPPORTED_ERR;
  }

  if( QMI_CSI_CB_NO_ERR == rv )
  {
    csi_err = qmi_csi_send_resp( req_handle, msg_id, resp_msg_ptr, body_len );
    if( QMI_CSI_NO_ERR != csi_err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME, "CSI send failure %i", csi_err );
      rv = QMI_CSI_CB_INTERNAL_ERR;
    }
  }
  if( NULL != resp_msg_ptr )
  {
    SNS_OS_FREE( resp_msg_ptr );
  }

  return rv;
}

sns_err_code_e
sns_time_mr_init()
{
  uint32_t *service_cookie = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_TIME, sizeof(uint32_t) );
  qmi_csi_error csi_err;
  qmi_csi_options qmi_options;
  qmi_idl_service_object_type service_obj = sns_smr_get_svc_obj( SNS_TIME2_SVC_ID_V01 );

  if( NULL == service_cookie )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_TIME, "Alloc failure" );
    return SNS_ERR_FAILED;
  }
  else if( NULL == service_obj )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME,
                               "Unable to get svc obj (%i)",
                               SNS_TIME2_SVC_ID_V01 );
    SNS_OS_FREE( service_cookie );
    return SNS_ERR_FAILED;
  }

#ifdef _WIN32
  // Initialize a notification for QMI events (requests from client)
  KeInitializeEvent(&qmi_event, NotificationEvent, FALSE);
  os_params.event = &qmi_event;

  // Initialize an event for allowing thread to exit
  KeInitializeEvent(&exit_event, NotificationEvent, FALSE);
#endif

  // Only register for Time service V2
  *service_cookie = SNS_TIME2_SVC_ID_V01;
  QMI_CSI_OPTIONS_INIT( qmi_options);
  QMI_CSI_OPTIONS_SET_INSTANCE_ID(qmi_options, SNS_APPS_IMPL_V02);
  csi_err = qmi_csi_register_with_options( service_obj, sns_time_mr_csi_connect,
      sns_time_mr_csi_disconnect, sns_time_mr_csi_process_req, service_cookie,
      &os_params, &qmi_options, &service_handle );

  if( QMI_CSI_NO_ERR != csi_err )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_TIME, "CSI init failure %i", csi_err );
    SNS_OS_FREE( service_cookie );
    return SNS_ERR_FAILED;
  }

  return SNS_SUCCESS;
}

sns_err_code_e
sns_time_mr_deinit()
{
#ifdef _WIN32
  // Set exit event
  KeSetEvent(&exit_event, 0, FALSE);
#endif

  return SNS_SUCCESS;
}

void
sns_time_mr_thread( void *p_arg )
{
  qmi_csi_error csi_error;

#ifdef _WIN32
  PVOID wait_events[2];

  /* Set the events we want to wait for in the WaitEvents array */
  wait_events[0] = &qmi_event;
  wait_events[1] = &exit_event;
#else /* _WIN32 */
  qmi_csi_os_params os_params_in;
  fd_set fds;
#endif /* else _WIN32 */

  UNREFERENCED_PARAMETER(p_arg);

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_TIME, "Time thread started" );
  sns_init_done();

  for( ; ; )
  {
#ifdef _WIN32
    NTSTATUS status = KeWaitForMultipleObjects(2, wait_events, WaitAny, Executive,
                                               KernelMode, FALSE, NULL, NULL);
    if (status == STATUS_WAIT_0)
    {
        // QMI Event
        KeClearEvent( &qmi_event );
        qmi_csi_handle_event( service_handle, &os_params );
    }
    else if (status == STATUS_WAIT_1)
    {
        // Exit Event
        break;
    }
#else /* _WIN32 */
    fds = os_params.fds;
    select( os_params.max_fd + 1, &fds, NULL, NULL, NULL );
    os_params_in.fds = fds;
    qmi_csi_handle_event( service_handle, &os_params_in );
#endif /* else _WIN32 */
  }

  csi_error = qmi_csi_unregister( service_handle );
  if (QMI_CSI_NO_ERR != csi_error)
  {
      SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG, "Failed to unregister from csi %d", csi_error );
  }

  sns_os_task_del(SNS_MODULE_PRI_APPS_TIME);
}
