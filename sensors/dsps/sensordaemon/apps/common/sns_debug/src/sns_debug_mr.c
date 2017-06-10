/*============================================================================
@file
sns_debug_mr.c

@brief
Implements the message routing portion of the Debug module.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary
============================================================================*/

/*============================================================================
  INCLUDE FILES
=============================================================================*/
#include "sns_common.h"
#ifdef SNS_BLAST
  #include "sns_debug_str_mdm.h"
#else
  #include "sns_debug_str.h"
#endif
#include "sns_debug_interface_v01.h"
#include "sns_diag_dsps_v01.h"
#include "sns_memmgr.h"
#include "sns_osa.h"
#include "sns_smr_util.h"
#include "sns_debug.h"
#include <stdint.h>
#include <stdbool.h>
#include <qmi_idl_lib.h>
#include <qmi_client.h>
#include <qmi_cci_target_ext.h>

#define QMI_CLNT_WAIT_SIG  0x00010000
#define QMI_CLNT_TIMER_SIG  0x00000001
/**
 * Pending mesage received from QCCI, that remain to be
 * processed by the ACM thread.
 */
typedef struct sns_debug_msg_list {
  void *msg;
  struct sns_debug_msg_list *next;
  struct sns_debug_msg_list *prev;
} sns_debug_msg_list;

/*============================================================================
  GLOBAL DEFINES
  ============================================================================*/
static qmi_client_type dbg_user_handle, diag_user_handle;
static sns_debug_msg_list *debug_rcv_queue_first, *debug_rcv_queue_last;
static OS_EVENT *debug_rcv_queue_mutex;
extern OS_FLAG_GRP *sns_diag_flag_grp;

/*===========================================================================
  FUNCTIONS
  ============================================================================*/

/*===========================================================================
  FUNCTION:   sns_debug_rcv_queue_add
  ===========================================================================*/
/*!
@brief
  This function adds a message to the debug receiving queue

@param[in]  msg                  message pointer
*/
/*=========================================================================*/
static void sns_debug_rcv_queue_add( void *msg )
{
  uint8_t os_err;
  sns_debug_msg_list *queue_msg;

  sns_os_mutex_pend( debug_rcv_queue_mutex, 0, &os_err );
  if( 0 != os_err ) {
    SNS_PRINTF_STRING_ERROR_1( DBG_MOD_DIAG, "Unable to acquire mutex %i", os_err );
    return;
  }

  queue_msg = SNS_OS_MALLOC( SNS_MODULE_APPS_DIAG, sizeof(sns_debug_msg_list) );
  if( NULL == queue_msg )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_DIAG, "Alloc failure" );
    return;
  }

  queue_msg->msg = msg;
  queue_msg->next = debug_rcv_queue_first;
  queue_msg->prev = NULL;
  if( NULL != debug_rcv_queue_first )
  {
    debug_rcv_queue_first->prev = queue_msg;
  }
  debug_rcv_queue_first = queue_msg;

  if( NULL == debug_rcv_queue_last )
  {
    debug_rcv_queue_last = debug_rcv_queue_first;
  }
  sns_os_mutex_post( debug_rcv_queue_mutex );
}

/*===========================================================================
  FUNCTION:   sns_debug_rcv_queue_add
  ===========================================================================*/
/*!
@brief
  This function removes a message from the debug receiving queue

  @return
  Pointer to the message removed from the queue
*/
/*=========================================================================*/
static void * sns_debug_rcv_queue_remove(void)
{
  uint8_t os_err;
  void *msg;

  sns_debug_msg_list *queue_msg = debug_rcv_queue_last;
  if (queue_msg == NULL)
  {
    return NULL;
  }

  sns_os_mutex_pend( debug_rcv_queue_mutex, 0, &os_err );
  if( 0 != os_err ) {
    SNS_PRINTF_STRING_ERROR_1( DBG_MOD_DIAG, "Unable to acquire mutex %i", os_err );
    return NULL;
  }

  if( NULL != debug_rcv_queue_last->prev )
  {
    debug_rcv_queue_last->prev->next = NULL;
    debug_rcv_queue_last = debug_rcv_queue_last->prev;
  }
  else
  {
    debug_rcv_queue_last = NULL;
    debug_rcv_queue_first = NULL;
  }

  msg = queue_msg->msg;
  SNS_OS_FREE( queue_msg );
  sns_os_mutex_post( debug_rcv_queue_mutex );

  return msg;
}

/*===========================================================================
  FUNCTION:   sns_dbg_ind_cb
  ===========================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an indication for this client

@param[in]  user_handle          Opaque handle used by the infrastructure to
                                 identify different services.

@param[in]  msg_id               Message ID of the indication

@param[in]  ind_buf              Buffer holding the encoded indication

@param[in]  ind_buf_len          Length of the encoded indication

@param[in]  ind_cb_data          Cookie value supplied by the client
                                 during registration
*/
/*=========================================================================*/
static void sns_dbg_ind_cb
(
 qmi_client_type                user_handle,
 unsigned int                   msg_id,
 void                           *ind_buf,
 unsigned int                   ind_buf_len,
 void                           *ind_cb_data
)
{
  void *decode_msg_ptr;
  uint16_t decode_msg_size=0;
  sns_smr_header_s smr_hdr;
  qmi_client_error_type err;
  uint8_t os_err;

  UNREFERENCED_PARAMETER(ind_cb_data);

  switch (msg_id)
  {
    case SNS_DEBUG_STRING_ID_IND_V01:
      decode_msg_size = sizeof(sns_debug_string_id_ind_msg_v01);
      break;
    case SNS_DEBUG_LOG_IND_V01:
      decode_msg_size = sizeof(sns_debug_log_ind_msg_v01);
      break;
    default:
      SNS_PRINTF_STRING_ERROR_0( DBG_MOD_DIAG, "Invalid indication (from debug)" );
      return;
  }

  decode_msg_ptr = sns_smr_msg_alloc(SNS_DBG_MOD_APPS_DIAG, decode_msg_size);
  if (decode_msg_ptr == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0( DBG_MOD_DIAG, "debug ind cb: msg alloc failed" );
    return;
  }

  err = qmi_client_message_decode(user_handle,
                                  QMI_IDL_INDICATION,
                                  msg_id,
                                  ind_buf, ind_buf_len,
                                  decode_msg_ptr,
                                  decode_msg_size);
  if (err != QMI_NO_ERR)
  {
    SNS_PRINTF_STRING_ERROR_1( DBG_MOD_DIAG, "Error decoding debug ind %d", err );
    sns_smr_msg_free(decode_msg_ptr);
    return;
  }

  /* These are mandatory fields to fill for later msg processing */
  smr_hdr.svc_num = SNS_DEBUG_SVC_ID_V01;
  smr_hdr.msg_id = (uint16_t)msg_id;
  smr_hdr.msg_type = SNS_SMR_MSG_TYPE_IND;

  /* Fill in rest of msg header just for completeness */
  smr_hdr.src_module = SNS_MODULE_DSPS;
  smr_hdr.dst_module = SNS_MODULE_APPS_DIAG;
  smr_hdr.priority = SNS_SMR_MSG_PRI_LOW;
  smr_hdr.txn_id = 0;
  smr_hdr.ext_clnt_id = 0;

  sns_smr_set_hdr(&smr_hdr, decode_msg_ptr);

  sns_debug_rcv_queue_add(decode_msg_ptr);
  sns_os_sigs_post(sns_diag_flag_grp, SNS_DIAG_RX_SIG, OS_FLAG_SET, &os_err);
}

/*===========================================================================
  FUNCTION:   sns_diag_ind_cb
  ===========================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an indication for this client

@param[in]  user_handle          Opaque handle used by the infrastructure to
                                 identify different services.

@param[in]  msg_id               Message ID of the indication

@param[in]  ind_buf              Buffer holding the encoded indication

@param[in]  ind_buf_len          Length of the encoded indication

@param[in]  ind_cb_data          Cookie value supplied by the client
                                 during registration
*/
/*=========================================================================*/
static void sns_diag_ind_cb
(
 qmi_client_type                user_handle,
 unsigned int                   msg_id,
 void                           *ind_buf,
 unsigned int                   ind_buf_len,
 void                           *ind_cb_data
)
{
  UNREFERENCED_PARAMETER(user_handle);
  UNREFERENCED_PARAMETER(msg_id);
  UNREFERENCED_PARAMETER(ind_buf);
  UNREFERENCED_PARAMETER(ind_buf_len);
  UNREFERENCED_PARAMETER(ind_cb_data);

  SNS_PRINTF_STRING_ERROR_0( DBG_MOD_DIAG, "Invalid indication (from diag)" );
  return;
}


/*=========================================================================
  CALLBACK FUNCTION:  sns_diag_rx_cb
  =========================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an asynchronous response for this client

@param[in]   user_handle         Opaque handle used by the infrastructure to
                                 identify different services.

@param[in]   msg_id              Message ID of the response

@param[in]   buf                 Buffer holding the decoded response

@param[in]   len                 Length of the decoded response

@param[in]   resp_cb_data        Cookie value supplied by the client

@param[in]   transp_err          Error value

*/
/*=======================================================================*/
static void sns_diag_rx_cb
(
 qmi_client_type                user_handle,
 unsigned int                   msg_id,
 void                           *buf,
 unsigned int                   len,
 void                           *resp_cb_data,
 qmi_client_error_type          transp_err
)
{
  UNREFERENCED_PARAMETER(user_handle);
  UNREFERENCED_PARAMETER(len);
  UNREFERENCED_PARAMETER(resp_cb_data);
  UNREFERENCED_PARAMETER(transp_err);

  /* Print the appropriate message based on the message ID */
  switch (msg_id)
  {
    case SNS_DIAG_SET_DEBUG_MASK_RESP_V01:
    {
      sns_diag_set_debug_mask_resp_msg_v01 *resp_msg = (sns_diag_set_debug_mask_resp_msg_v01*)buf;
      SNS_PRINTF_STRING_MEDIUM_2(DBG_MOD_INIT,
                                 "DIAG SET DEBUG MASK RESP: Err: %d   Result:%d",
                                 resp_msg->resp.sns_err_t,
                                 resp_msg->resp.sns_result_t);
    }
    break;

    case SNS_DIAG_SET_LOG_MASK_RESP_V01:
    {
      sns_diag_set_log_mask_resp_msg_v01 *resp_msg = (sns_diag_set_log_mask_resp_msg_v01*)buf;
      SNS_PRINTF_STRING_MEDIUM_2(DBG_MOD_INIT,
                                 "DIAG SET LOG MASK RESP: Err: %d   Result:%d",
                                 resp_msg->resp.sns_err_t,
                                 resp_msg->resp.sns_result_t);
    }
    break;

    case SNS_DIAG_SET_DEBUG_OPTIONS_RESP_V01:
    {
      sns_diag_set_debug_options_resp_msg_v01 *resp_msg = (sns_diag_set_debug_options_resp_msg_v01*)buf;
      SNS_PRINTF_STRING_MEDIUM_2(DBG_MOD_INIT,
                                 "DIAG SET DEBUG OPTIONS RESP: Err: %d   Result:%d",
                                 resp_msg->resp.sns_err_t,
                                 resp_msg->resp.sns_result_t);
    }
    break;

    default:
      SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT, "Invalid DIAG response");
      break;
  }

  SNS_OS_FREE(buf);
}

/*=========================================================================
  CALLBACK FUNCTION:  sns_dbg_rx_cb
  =========================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an asynchronous response for this client

@param[in]   user_handle         Opaque handle used by the infrastructure to
                                 identify different services.

@param[in]   msg_id              Message ID of the response

@param[in]   buf                 Buffer holding the decoded response

@param[in]   len                 Length of the decoded response

@param[in]   resp_cb_data        Cookie value supplied by the client

@param[in]   transp_err          Error value

*/
/*=======================================================================*/
static void sns_dbg_rx_cb
(
 qmi_client_type                user_handle,
 unsigned int                   msg_id,
 void                           *buf,
 unsigned int                   len,
 void                           *resp_cb_data,
 qmi_client_error_type          transp_err
)
{
  UNREFERENCED_PARAMETER(user_handle);
  UNREFERENCED_PARAMETER(len);
  UNREFERENCED_PARAMETER(resp_cb_data);
  UNREFERENCED_PARAMETER(transp_err);

  /* Print the appropriate message based on the message ID */
  if (msg_id == SNS_DEBUG_VERSION_RESP_V01)
  {
    sns_common_version_resp_msg_v01 *resp_msg = (sns_common_version_resp_msg_v01*)buf;
    SNS_PRINTF_STRING_MEDIUM_2(DBG_MOD_INIT,
                               "DEBUG VERSION RESP: Ver: %d   Max ID:%d",
                               resp_msg->interface_version_number,
                               resp_msg->max_message_id);
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT, "Invalid DEBUG response");
  }
  SNS_OS_FREE(buf);
}

/*===========================================================================

  FUNCTION:   sns_diag_send_mask_info

===========================================================================*/
/*!
  @brief
  Sends mask info to DSPS through SMR

  @param[i] mask_type    : Indicates what mask is being set (log/debugstr)
  @param[i] bit_mask     : Bit mask to be sent to DSPS
  @param[i] bit_mask_ext : Extended Bit mask to be sent to DSPS

  @return
  No return value
*/
/*=========================================================================*/
void sns_diag_send_mask_info(uint8_t mask_type,
                             uint64_t bit_mask,
                             uint64_t bit_mask_ext)
{
  qmi_txn_handle txn_handle;
  qmi_client_error_type rc;
  uint8_t req_msg_body_size=0, resp_msg_body_size=0;
  void *mask_resp_msg_ptr;
  uint32_t msg_id;

  union {
    sns_diag_set_debug_mask_req_msg_v01 str;
    sns_diag_set_log_mask_req_msg_v01 log;
  } msg;

  switch (mask_type)
  {
    case DEBUG_STR_MASK_INFO:
      {
        msg.str.debug_mask.mask = bit_mask;
        msg_id = SNS_DIAG_SET_DEBUG_MASK_REQ_V01;

        req_msg_body_size = sizeof(sns_diag_set_debug_mask_req_msg_v01);
        resp_msg_body_size = sizeof(sns_diag_set_debug_mask_resp_msg_v01);
        break;
      }
    case LOG_MASK_INFO:
      {
        msg.log.log_mask.mask = bit_mask;
        msg.log.log_mask_ext_valid = 1;
        msg.log.log_mask_ext.mask = bit_mask_ext;
        msg_id = SNS_DIAG_SET_LOG_MASK_REQ_V01;

        req_msg_body_size = sizeof(sns_diag_set_log_mask_req_msg_v01);
        resp_msg_body_size = sizeof(sns_diag_set_log_mask_resp_msg_v01);
        break;
      }
    default:
      return;
  }

  mask_resp_msg_ptr = SNS_OS_MALLOC(SNS_DBG_MOD_APPS_DIAG, resp_msg_body_size);
  if (mask_resp_msg_ptr == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0( DBG_MOD_INIT, "send_mask_info: msg alloc failed" );
    return;
  }

  rc = qmi_client_send_msg_async(diag_user_handle,
                                 msg_id,
                                 &msg,
                                 req_msg_body_size,
                                 mask_resp_msg_ptr,
                                 resp_msg_body_size,
                                 sns_diag_rx_cb,
                                 NULL,
                                 &txn_handle);
  if (rc != QMI_NO_ERR)
  {
    SNS_PRINTF_STRING_ERROR_1( DBG_MOD_INIT, "Error sending mask rc=%d", rc );
    SNS_OS_FREE(mask_resp_msg_ptr);
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( DBG_MOD_INIT, "sent mask successfully rc=%d", rc );
  }
}
// end of function sns_diag_send_mask_info

/*===========================================================================

  FUNCTION:   sns_diag_send_debug_options

===========================================================================*/
/*!
  @brief
  Sends debug options info to DSPS through SMR

  @param[i] options : debug options array

  @return
  No return value
*/
/*=========================================================================*/
void sns_diag_send_debug_options(uint64_t *options)
{
  qmi_txn_handle txn_handle;
  qmi_client_error_type rc;
  int req_msg_body_size=0, resp_msg_body_size=0;
  void *resp_msg_ptr;
  uint32_t msg_id;
  uint8_t i, num_options;
  sns_diag_set_debug_options_req_msg_v01 options_msg;

  num_options = 0;
  SNS_PRINTF_STRING_ERROR_0( DBG_MOD_INIT, "send_debug_options: Called");
  for (i = 0; i < SNS_DIAG_MAX_OPTIONS_V01; i++)
  {
    if (options[i] != 0) // Do not send defaults
    {
      SNS_PRINTF_STRING_ERROR_2( DBG_MOD_INIT, "Setting option %d to %d: ", i, options[i]);
      options_msg.options[num_options].option= i;
      options_msg.options[num_options].value = options[i];
      num_options++;
    }
  }

  options_msg.options_len = num_options;

  msg_id = SNS_DIAG_SET_DEBUG_OPTIONS_REQ_V01;
  resp_msg_body_size = sizeof(sns_diag_set_debug_options_resp_msg_v01);
  req_msg_body_size = sizeof(sns_diag_set_debug_options_req_msg_v01);

  resp_msg_ptr = SNS_OS_MALLOC(SNS_DBG_MOD_APPS_DIAG, resp_msg_body_size);
  if (resp_msg_ptr == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0( DBG_MOD_INIT, "send_debug_options: msg alloc failed" );
    return;
  }

  SNS_PRINTF_STRING_ERROR_0( DBG_MOD_INIT, "send_debug_options: Sending msg");
  rc = qmi_client_send_msg_async(diag_user_handle,
                                 msg_id,
                                 &options_msg,
                                 req_msg_body_size,
                                 resp_msg_ptr,
                                 resp_msg_body_size,
                                 sns_diag_rx_cb,
                                 NULL,
                                 &txn_handle);
  if (rc != QMI_NO_ERR)
  {
    SNS_PRINTF_STRING_ERROR_1( DBG_MOD_INIT, "Error sending debug options rc=%d", rc );
    SNS_OS_FREE(resp_msg_ptr);
  }
}
// end of function sns_diag_send_debug_options
/*===========================================================================

  FUNCTION:   sns_debug_send_version_req

===========================================================================*/
/*!
  @brief
  Sends version request to debug service

  @return
  No return value
*/
/*=========================================================================*/
static void sns_debug_send_version_req(void)
{
  uint32 ver_req = 0;
  void *ver_resp_ptr = NULL;
  qmi_txn_handle txn_handle;
  qmi_client_error_type rc;

  ver_resp_ptr = SNS_OS_MALLOC(SNS_DBG_MOD_APPS_DIAG, sizeof(sns_common_version_resp_msg_v01));
  if( NULL == ver_resp_ptr )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_MODULE_APPS_DIAG, "Alloc failure" );
    return;
  }

  rc = qmi_client_send_msg_async(dbg_user_handle,
                                 SNS_DEBUG_VERSION_REQ_V01,
                                 &ver_req,
                                 0,
                                 ver_resp_ptr,
                                 sizeof(sns_common_version_resp_msg_v01),
                                 sns_dbg_rx_cb,
                                 NULL,
                                 &txn_handle);
  if (rc != QMI_NO_ERR)
  {
    SNS_OS_FREE(ver_resp_ptr);
  }
}

/*===========================================================================

  FUNCTION:   sns_debug_mr_init
  ===========================================================================*/
/*!
  @brief
  Initialize with message router. Register with debug/diag services.

  @param[i]
  Not used.

  @return
  None.
*/
/*=========================================================================*/
sns_err_code_e sns_debug_mr_init(void)
{
  qmi_client_type clnt_notifier;
  qmi_cci_os_signal_type os_params;
  qmi_service_info svc_info;
  unsigned int num_services, num_entries;
  qmi_idl_service_object_type sns_dbg_svc_obj = sns_smr_get_svc_obj(SNS_DEBUG_SVC_ID_V01);
  qmi_idl_service_object_type sns_diag_svc_obj = sns_smr_get_svc_obj(SNS_DIAG_DSPS_SVC_ID_V01);
  qmi_client_error_type rc;
  uint8_t os_err;

  if ( (NULL == sns_dbg_svc_obj) || (NULL == sns_diag_svc_obj) )
  {
    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                              "Sensors DIAG Init: Failed to obtain service object" );
    return SNS_ERR_FAILED;
  }

  //Check for service before initialize and wait.
  if(QMI_NO_ERR != (rc = qmi_client_get_any_service( sns_dbg_svc_obj, &svc_info )))
  {
    /* Initialize debug svc client-notifier */
#ifdef SNS_QDSP_SIM
  os_params.ext_signal = NULL;
  os_params.sig = QMI_CLNT_WAIT_SIG;
  os_params.timer_sig = QMI_CLNT_TIMER_SIG;
#endif

    rc = qmi_client_notifier_init(sns_dbg_svc_obj, &os_params, &clnt_notifier);
    if (rc != QMI_NO_ERR)
    {
      SNS_PRINTF_STRING_ERROR_1(DBG_MOD_INIT,
                                "Sensors DIAG Init: Error initializing debug client notifier rc=%d",
                                rc);
      qmi_client_release( clnt_notifier );
      return SNS_ERR_FAILED;
    }

    /* Check if the debug service is up, if not wait on a signal */
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, 10000);
    if ( QMI_CCI_OS_SIGNAL_TIMED_OUT(&os_params) )
    {
      SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                                "Sensors DIAG Init: Waiting for debug service timed out");
      qmi_client_release( clnt_notifier );
      return SNS_ERR_FAILED;
    }
    QMI_CCI_OS_SIGNAL_CLEAR(&os_params);

    SNS_PRINTF_STRING_LOW_0(DBG_MOD_INIT, "Init Debug service");

    /* The debug server has come up, store the information in info variable */
    num_entries=1;
    rc = qmi_client_get_service_list(sns_dbg_svc_obj, &svc_info, &num_entries, &num_services);
    if (rc != QMI_NO_ERR)
    {
      SNS_PRINTF_STRING_ERROR_1(DBG_MOD_INIT,
                                "Sensors DIAG Init: Error getting debug service list rc=%d",
                                rc);
      qmi_client_release( clnt_notifier );
      return SNS_ERR_FAILED;
    }
  }
  SNS_PRINTF_STRING_LOW_0(DBG_MOD_INIT, "Got Debug service");

  /* Register for debug service */
  rc = qmi_client_init(&svc_info, sns_dbg_svc_obj, sns_dbg_ind_cb, NULL, &os_params, &dbg_user_handle);
  if (rc != QMI_NO_ERR)
  {
    SNS_PRINTF_STRING_ERROR_1(DBG_MOD_INIT,
                              "Sensors DIAG Init: Error initializing debug client rc=%d",
                              rc);
    return SNS_ERR_FAILED;
  }

  /* Send version request to debug service.
     This establishes connection with debug service, so that
     debug service can send indication to us later */
  sns_debug_send_version_req();

  //Check for service before initialize and wait.
  if(QMI_NO_ERR != (rc = qmi_client_get_any_service( sns_diag_svc_obj, &svc_info )))
  {
    /* Initialize diag svc client-notifier */
#ifdef SNS_QDSP_SIM
  os_params.ext_signal = NULL;
  os_params.sig = QMI_CLNT_WAIT_SIG;
  os_params.timer_sig = QMI_CLNT_TIMER_SIG;
#endif

    rc = qmi_client_notifier_init(sns_diag_svc_obj, &os_params, &clnt_notifier);
    if (rc != QMI_NO_ERR)
    {
      SNS_PRINTF_STRING_ERROR_1(DBG_MOD_INIT,
                                "Sensors DIAG Init: Error initializing diag client notifier rc=%d",
                                rc);
      return SNS_ERR_FAILED;
    }

    /* Check if the diag service is up, if not wait on a signal */
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, 10000);
    if ( QMI_CCI_OS_SIGNAL_TIMED_OUT(&os_params) )
    {
      SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                                "Sensors DIAG Init: Waiting for diag service timed out");
      qmi_client_release( clnt_notifier );
      return SNS_ERR_FAILED;
    }
    QMI_CCI_OS_SIGNAL_CLEAR(&os_params);

    SNS_PRINTF_STRING_LOW_0(DBG_MOD_INIT, "Init Diag service");

    /* The diag server has come up, store the information in info variable */
    num_entries=1;
    rc = qmi_client_get_service_list(sns_diag_svc_obj, &svc_info, &num_entries, &num_services);
    if (rc != QMI_NO_ERR)
    {
      SNS_PRINTF_STRING_ERROR_1(DBG_MOD_INIT,
                                "Sensors DIAG Init: Error getting diag service list rc=%d",
                                rc);
      qmi_client_release( clnt_notifier );
      return SNS_ERR_FAILED;
    }
  }
  SNS_PRINTF_STRING_LOW_0(DBG_MOD_INIT, "Got Diag service");


  /* Register for diag service */
  rc = qmi_client_init(&svc_info, sns_diag_svc_obj, sns_diag_ind_cb, NULL, &os_params, &diag_user_handle);
  if (rc != QMI_NO_ERR)
  {
    SNS_PRINTF_STRING_ERROR_1(DBG_MOD_INIT,
                              "Sensors DIAG Init: Error initializing diag client rc=%d",
                              rc);
    return SNS_ERR_FAILED;
  }

  debug_rcv_queue_first = NULL;
  debug_rcv_queue_last = NULL;

  debug_rcv_queue_mutex = sns_os_mutex_create( SNS_SMR_APPS_QUE_MUTEX, &os_err );
  if( 0 != os_err )
  {
    qmi_client_release(dbg_user_handle);
    qmi_client_release(diag_user_handle);

    SNS_PRINTF_STRING_ERROR_0(DBG_MOD_INIT,
                              "Sensors DIAG init: Can't create mutex" );
    return SNS_ERR_FAILED;
  }

  return SNS_SUCCESS;
}
// end of sns_debug_mr_init()

/*===========================================================================
  FUNCTION:   sns_debug_mr_deinit
  ===========================================================================*/
/*!
  @brief
  Deinitialize with message router.

  @param[i]
  Not used.

  @return
  None.
*/
/*=========================================================================*/
sns_err_code_e sns_debug_mr_deinit(void)
{
  uint8_t os_err;

  // Close all clients
  qmi_client_release(dbg_user_handle);
  qmi_client_release(diag_user_handle);

  // Remove all items from queue
  sns_os_mutex_pend( debug_rcv_queue_mutex, 0, &os_err );

  while( NULL != debug_rcv_queue_first )
  {
    sns_debug_msg_list *queue_msg = debug_rcv_queue_first;

    debug_rcv_queue_first = debug_rcv_queue_first->next;

    sns_smr_msg_free( queue_msg->msg );
    SNS_OS_FREE( queue_msg );
  }

  debug_rcv_queue_first = NULL;
  debug_rcv_queue_last = NULL;

  sns_os_mutex_post( debug_rcv_queue_mutex );

  // Delete queue mutex
  sns_os_mutex_del( debug_rcv_queue_mutex, 0, &os_err );
  if( 0 != os_err )
  {
    SNS_PRINTF_STRING_FATAL_1( DBG_MOD_INIT, "can't delete mutex %i", os_err );
    return SNS_ERR_FAILED;
  }

  return SNS_SUCCESS;
}
// end of sns_debug_mr_deinit()

/*===========================================================================
  FUNCTION:   sns_debug_rcv
  ===========================================================================*/
/*!
  @brief
  Get message pointer from receiving queue.

  @param[i]
  Not used.

  @return
  Message pointer from the queue.
*/
/*=========================================================================*/
void * sns_debug_rcv(void)
{
  return (sns_debug_rcv_queue_remove());
}
