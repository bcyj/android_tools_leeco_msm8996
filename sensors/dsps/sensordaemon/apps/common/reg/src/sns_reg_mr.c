/*============================================================================
  @file sns_reg_mr.c

  @brief
    Implementes the message routing and handling portion of the
    sensors registry.

  <br><br>

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
  ============================================================================*/
#include "sensor1.h"
#include "sns_common.h"
#include "sns_reg.h"
#include "fixed_point.h"
#include "sns_smr_util.h"
#include "sns_debug_str.h"
#include "sns_memmgr.h"
#include "sns_init.h"
#include "sns_common_v01.h"
#include "sns_reg_api_v02.h"
#include "sns_reg_priv.h"
#include <stdbool.h>
#include "sns_reg_platform.h"
#include "sns_reg_mr.h"
#include <qmi_csi.h>

static qmi_csi_os_params os_params;
static qmi_csi_service_handle service_handle;

/*============================================================================
  Static Function Definitions and Documentation
  ============================================================================*/

/**
 * Translate an internal sensors error, into the error we want the sensor1
 * client to receive.
 *
 * @param[i] sns_err Error in internal format
 *
 * @return Equivalent error at sensor1 layer
 */
static sensor1_error_e
sns_reg_conv_err( sns_err_code_e sns_err )
{
  if( SNS_SUCCESS == sns_err )
  {
    return SENSOR1_SUCCESS;
  }
  else if( SNS_ERR_BUFFER == sns_err )
  {
    return SENSOR1_EBUFFER;
  }
  else if( SNS_ERR_NOMEM == sns_err )
  {
    return SENSOR1_ENOMEM;
  }
  else if( SNS_ERR_INVALID_HNDL == sns_err )
  {
    return SENSOR1_EINVALID_CLIENT;
  }
  else if( SNS_ERR_UNKNOWN == sns_err )
  {
    return SENSOR1_EUNKNOWN;
  }
  else if( SNS_ERR_FAILED == sns_err )
  {
    return SENSOR1_EFAILED;
  }
  else if( SNS_ERR_NOTALLOWED == sns_err ||
           SNS_ERR_NOTSUPPORTED == sns_err )
  {
    return SENSOR1_ENOTALLOWED;
  }
  else if( SNS_ERR_BAD_PARM == sns_err )
  {
    return SENSOR1_EBAD_PARAM;
  }
  else if( SNS_ERR_BAD_PTR == sns_err )
  {
    return SENSOR1_EBAD_PTR;
  }
  else if( SNS_ERR_BAD_MSG_ID == sns_err )
  {
    return SENSOR1_EBAD_MSG_ID;
  }
  else if( SNS_ERR_BAD_MSG_SIZE == sns_err )
  {
    return SENSOR1_EBAD_MSG_SZ;
  }
  else if( SNS_ERR_WOULDBLOCK == sns_err )
  {
    return SENSOR1_EWOULDBLOCK;
  }

  return SENSOR1_EUNKNOWN;
}

/**
 * Callback function for QMI framework - Not used
 */
static qmi_csi_cb_error
sns_reg_mr_csi_connect( qmi_client_handle client_handle, void *service_cookie,
    void **connection_handle )
{
  UNREFERENCED_PARAMETER(client_handle);
  UNREFERENCED_PARAMETER(service_cookie);
  UNREFERENCED_PARAMETER(connection_handle);

  /* Do Nothing */
  SNS_PRINTF_STRING_LOW_0( SNS_MODULE_APPS_REG, "Received connect" );
  return 0;
}

/**
 * Callback function for QMI framework - Not used
 */
static void
sns_reg_mr_csi_disconnect( void *connection_handle, void *service_cookie )
{
  UNREFERENCED_PARAMETER(service_cookie);
  UNREFERENCED_PARAMETER(connection_handle);

  /* Do Nothing */
  SNS_PRINTF_STRING_LOW_0( SNS_MODULE_APPS_REG, "received disconnect" );
}

/**
 * Translate a V02 request message so that it can be processed
 * by the registry service.
 *
 * @param[i] msg_id QMI IDL message ID
 * @param[i] req_c_struct Request message contents
 * @param[o] resp_msg_ptr The response message; invalid if return != QMI_CSI_NO_ERR
 *
 * @return QMI CB error code
 */
static qmi_csi_cb_error
sns_reg_mr_handle( int msg_id, void *req_c_struct,
    sns_common_resp_s_v01 *resp_msg_ptr)
{
  uint16_t data_len;
  sns_err_code_e err;

  if( NULL == resp_msg_ptr )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG, "Resp message allocation failed" );
    return QMI_CSI_CB_NO_MEM;
  }


  if( SNS_REG_CANCEL_REQ_V02 == msg_id )
  {
    sns_common_cancel_resp_msg_v01 *resp_msg = (sns_common_cancel_resp_msg_v01*)resp_msg_ptr;

    resp_msg->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
    resp_msg->resp.sns_err_t = SNS_SUCCESS;
    return QMI_CSI_CB_NO_ERR;
  }
  else if( SNS_REG_VERSION_REQ_V02 == msg_id )
  {
    sns_common_version_resp_msg_v01 *resp_msg = (sns_common_version_resp_msg_v01*)resp_msg_ptr;

    resp_msg->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
    resp_msg->resp.sns_err_t = SNS_SUCCESS;
    resp_msg->interface_version_number = SNS_REG2_SVC_V02_IDL_MINOR_VERS;
    resp_msg->max_message_id = SNS_REG2_SVC_V02_MAX_MESSAGE_ID;
    return QMI_CSI_CB_NO_ERR;
  }
  else if( SNS_REG_SINGLE_READ_REQ_V02 == msg_id )
  {
    sns_reg_single_read_resp_msg_v02 *resp_msg = (sns_reg_single_read_resp_msg_v02*)resp_msg_ptr;
    resp_msg->item_id = ((sns_reg_single_read_req_msg_v02*)req_c_struct)->item_id;

    err = sns_reg_handle_req( resp_msg->item_id, true, true, &data_len, resp_msg->data );
    if( SNS_SUCCESS == err )
    {
      resp_msg->data_len = data_len;
    }
  }
  else if( SNS_REG_GROUP_READ_REQ_V02 == msg_id )
  {
    sns_reg_group_read_resp_msg_v02 *resp_msg = (sns_reg_group_read_resp_msg_v02*)resp_msg_ptr;
    resp_msg->group_id = ((sns_reg_group_read_req_msg_v02*)req_c_struct)->group_id;

    err= sns_reg_handle_req( resp_msg->group_id, true, false, &data_len, resp_msg->data );
    if( SNS_SUCCESS == err )
    {
      resp_msg->data_len = data_len;
    }
  }
  else if( SNS_REG_SINGLE_WRITE_REQ_V02 == msg_id )
  {
    sns_reg_single_write_req_msg_v02 *req_msg = (sns_reg_single_write_req_msg_v02*)req_c_struct;

    data_len = req_msg->data_len;
    err = sns_reg_handle_req( req_msg->item_id, false, true, &data_len, req_msg->data );
  }
  else if( SNS_REG_GROUP_WRITE_REQ_V02 == msg_id )
  {
    sns_reg_group_write_req_msg_v02 *req_msg = (sns_reg_group_write_req_msg_v02*)req_c_struct;

    data_len = req_msg->data_len;
    err = sns_reg_handle_req( req_msg->group_id, false, false, &data_len, req_msg->data );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG, "Invalid msg id" );
    return QMI_CSI_CB_UNSUPPORTED_ERR;
  }

  resp_msg_ptr->sns_result_t =
    (SNS_SUCCESS == err) ? SNS_RESULT_SUCCESS_V01 : SNS_RESULT_FAILURE_V01;
  resp_msg_ptr->sns_err_t = sns_reg_conv_err( err );

  return QMI_CSI_CB_NO_ERR;
}

/**
 * Callback function requests made to the registry service.  Handle the request
 * and send back the response.  May involve modifying the sensors registry.
 *
 * See QMI documentation for function parameters.
 */
static qmi_csi_cb_error
sns_reg_mr_csi_process_req( void *connection_handle, qmi_req_handle req_handle,
    unsigned int msg_id, void *req_c_struct, unsigned int req_c_struct_len,
    void *service_cookie )
{
  qmi_csi_error csi_err;
  void *resp_msg_ptr;
  qmi_csi_cb_error rv = QMI_CSI_CB_NO_ERR;
  unsigned int resp_len;
  uint32_t svc_num = *(uint32_t*)service_cookie;
  int32_t qmi_err;

  UNREFERENCED_PARAMETER( connection_handle );

  SNS_PRINTF_STRING_LOW_3( SNS_MODULE_APPS_REG,
                           "svc:%d; msg id:%d; len:%d",
                           svc_num, msg_id, req_c_struct_len );

  qmi_err = qmi_idl_get_message_c_struct_len( sns_smr_get_svc_obj(SNS_REG2_SVC_ID_V01),
      QMI_IDL_RESPONSE, msg_id, &resp_len );
  if( QMI_IDL_LIB_NO_ERR != qmi_err )
  {
    SNS_PRINTF_STRING_ERROR_2( SNS_MODULE_APPS_REG,
        "Unable to get response msg size; msg id %i (err %i)", msg_id, qmi_err );
    return QMI_CSI_CB_UNSUPPORTED_ERR;
  }

  resp_msg_ptr = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_REG, resp_len );

  rv = sns_reg_mr_handle( msg_id, req_c_struct,
      (sns_common_resp_s_v01*)resp_msg_ptr);

  if( QMI_CSI_CB_NO_ERR == rv )
  {
    csi_err = qmi_csi_send_resp( req_handle, msg_id, resp_msg_ptr, resp_len );
    if( QMI_CSI_NO_ERR != csi_err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG, "CSI send failure %i", csi_err );
      rv = QMI_CSI_CB_INTERNAL_ERR;
    }
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG,
                               "Error processing request %i", rv );
  }

  SNS_OS_FREE( resp_msg_ptr);

  SNS_PRINTF_STRING_LOW_0( SNS_MODULE_APPS_REG, "Processed request" );
  return rv;
}

/*===========================================================================
                        Public Function Definitions
===========================================================================*/

sns_err_code_e
sns_reg_mr_init()
{
  uint32_t *service_cookie = SNS_OS_MALLOC( SNS_MODULE_APPS_REG, sizeof(uint32_t) );
  qmi_csi_error csi_err;
  qmi_idl_service_object_type service_obj = sns_smr_get_svc_obj( SNS_REG2_SVC_ID_V01 );
  sns_err_code_e rv = SNS_SUCCESS;


  if( NULL == service_cookie )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_REG, "Alloc failure" );
    rv = SNS_ERR_FAILED;
  }
  else if( NULL == service_obj )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG,
                               "Unable to get svc obj (%i)",
                               SNS_REG2_SVC_ID_V01 );
    SNS_OS_FREE( service_cookie );
    rv = SNS_ERR_FAILED;
  }
  else
  {
    sns_reg_mr_params_init(&os_params);
    // Register for registry service V2
    *service_cookie = SNS_REG2_SVC_ID_V01;
    csi_err = qmi_csi_register( service_obj, sns_reg_mr_csi_connect,
        sns_reg_mr_csi_disconnect, sns_reg_mr_csi_process_req, service_cookie,
        &os_params, &service_handle );

    if( QMI_CSI_NO_ERR != csi_err )
    {
      SNS_PRINTF_STRING_ERROR_2( SNS_MODULE_APPS_REG,
                                "CSI init failure %i (svc %i)",
                                csi_err, *service_cookie );
      rv = SNS_ERR_FAILED;
    }

    /*
    os_params = os_params_v02;
    for( fd = 0; fd < FD_SETSIZE; fd++ )
    {
      if( FD_ISSET( fd, &os_params_v02.fds ) || FD_ISSET( fd, &os_params_v03.fds ) ) {
        FD_SET( fd, &os_params.fds );
        os_params.max_fd = fd;
      }
    }
    */
  }

  if( SNS_SUCCESS != rv )
  {
    SNS_OS_FREE( service_cookie );
  }

  return rv;
}

sns_err_code_e
sns_reg_mr_deinit()
{
  sns_reg_mr_params_deinit();

  return SNS_SUCCESS;
}

void
sns_reg_mr_thread( void *p_arg )
{
  sns_err_code_e  err;
  qmi_csi_error csi_error;

  UNREFERENCED_PARAMETER(p_arg);

  SNS_PRINTF_STRING_LOW_0( SNS_MODULE_APPS_REG, "Reg thread started" );

  err = sns_reg_storage_init();
  if( SNS_SUCCESS != err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_MODULE_APPS_REG, "File init failed %i", err );
  }
  sns_init_done();

  /* Handle events */
  sns_reg_mr_handle_evts(&service_handle, &os_params);

  csi_error = qmi_csi_unregister( service_handle );
  if( QMI_CSI_NO_ERR != csi_error )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_MODULE_APPS_REG, "Failed to unregister from csi %d", csi_error );
  }

  err = sns_reg_storage_deinit();
  if( SNS_SUCCESS != err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_MODULE_APPS_REG, "File deinit failed %i", err );
  }

  sns_os_task_del(SNS_MODULE_PRI_APPS_REG);
}
