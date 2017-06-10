/*============================================================================
  @file sns_sam_mr.c

  @brief
    Implementes the message routing and handling portion of the
    sensors algorithm manager.

  <br><br>

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
  ============================================================================*/
#include "sensor1.h"
#include "sns_common.h"
#include "fixed_point.h"
#include "sns_smr_util.h"
#include "sns_debug_str.h"
#include "sns_memmgr.h"
#include "sns_init.h"
#include "sns_sam_priv.h"
#include <stdbool.h>
#include <errno.h>
#include "sns_sam_mr.h"
#include <qmi_csi.h>
#include <qmi_client.h>
#include <qmi_idl_lib.h>
#include <qmi_cci_target_ext.h>

/*============================================================================
  Variable Definitions
  ============================================================================*/
#if defined(SNS_DSPS_BUILD) || defined(SNS_PCSIM) || defined(QDSP6)
  #define SNS_MODULE_SAM SNS_MODULE_DSPS_SAM
#else
  #define SNS_MODULE_SAM SNS_MODULE_APPS_SAM
#endif

#define SNS_SAM_MAX_CLI_ID 40

/* Maximum number of messages allowed in sns_sam_msg_queue */
#define SNS_SAM_MR_MSG_Q_MAX 30

/* # of entries in service_handles array within each connection handle.
 * Addition entry is for the time service.  Minimum size must be 4. */
#define SNS_SAM_SVC_ARR_LEN SNS_SAM_MAX_ALGO_DEPS + 2

/*============================================================================
  Type Declarations
  ============================================================================*/

/* Data to be returned in response CB */
typedef struct sns_sam_mr_resp_cb_data
{
  sns_smr_header_s                smr_header;
  sns_sam_mr_algo_conn_hndl       *algo_conn_hndl;
} sns_sam_mr_resp_cb_data;

/* Each SAM service will have one of these objects. */
typedef struct sam_qmi_svc_s
{
  uint16_t                service_cookie;
  qmi_csi_os_params       os_params;
  qmi_csi_service_handle  service_handle;
} sam_qmi_svc_s;

/**
 * Callback data received in every indication.
 * Contains information to be used to reconstruct the SMR header.
 */
typedef struct sns_sam_ind_cb_data {
  uint32_t                        svc_num;
  qmi_idl_service_object_type     service_obj;
  uint8_t                         module;
  sns_sam_mr_algo_conn_hndl       *algo_conn_hndl;
  OS_EVENT                        *algo_inst_id_mutex;
  uint8_t                         algo_inst_id;
} sns_sam_ind_cb_data;

/**
 * Each request, response, and indication received by SAM will be
 * placed in this structure and added to sns_sam_msg_queue.
 */
typedef struct sns_sam_q_item_s {
  sns_q_link_s   q_link;
  qmi_req_handle req_handle; /* QMI handle only valid in case of request-type message */
  void           *msg;       /* Message allocated by sns_smr_msg_alloc */
} sns_sam_q_item_s;

/**
 * Each of these objects refers to connection request received by a SAM service.
 * This request may have been made by another SAM service, or by a client such as
 * the ACM.  MR client ID 0 is a special case, and is used for for SAM to send
 * unprompted requests to other services.
 */
typedef struct sns_sam_mr_cli_conn_hndl {
  sns_q_link_s           q_link;
  sns_sam_mr_conn_hndl   mr_clnt_id;  /* Client ID as determined by sam_mr */
  uint8_t                svc_num; /* Service number of this client */
  uint8_t                ready_to_release;  /* If this connection should be freed */
  qmi_client_handle      client_handle;
} sns_sam_mr_cli_conn_hndl;

/**
 * Each SAM algorithm instance will have one of these structs, so that it will
 * capable of sending requests to its dependent algotrithms.
 */
struct sns_sam_mr_algo_conn_hndl_s {
  qmi_client_type        service_handles[ SNS_SAM_SVC_ARR_LEN ];
  sns_sam_ind_cb_data    *ind_cb_data[ SNS_SAM_SVC_ARR_LEN ];
  uint8_t                svc_num; /* Service number of this algorithm instance */
};

/* Data to be returned in client release CB */
typedef struct sns_sam_client_release_cb_data_s {
  sns_sam_mr_algo_conn_hndl * algo_conn_hndl;
  int svc_index;
} sns_sam_client_release_cb_data_s;

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

/**
 * List of clients for SAM services.  A client may have multiple entries in this
 * list if they have requests to multiple services.  A SAM algorithm may
 * itself be a client for another service.
 */
static sns_q_s                sns_sam_cli_queue;
static OS_EVENT               *sns_sam_cli_queue_mutex;

/**
 * List of messages received by the QCCI framework (i.e. responses and indications),
 *  but not yet processed by the main SAM thread.
 */
static sns_q_s                sns_sam_msg_queue;
static OS_EVENT               *sns_sam_msg_queue_mutex;

/**
 * QMI framework state.  Array could be shortened as it only needs an entry for
 * each SAM service.
 */
static sam_qmi_svc_s sam_qmi_svc[ SNS_SAM_MAX_ALGOS ];

/* Connection handle to be used by SAM framework to send messages
 * outside of SAM (e.g. SMGR, Time, or Registry). */
static sns_sam_mr_algo_conn_hndl *sns_sam_mr_fw_clnt_hndl = NULL;

/*============================================================================
  External Variable Definitions
  ============================================================================*/

extern OS_FLAGS sam_sig_mask;
extern sns_sam_algo_s* sns_sam_algo_dbase[];

/*============================================================================
  Function Definitions and Documentation
  ============================================================================*/

/*=============================================================================
  CALLBACK FUNCTION sns_sam_client_release_cb
=============================================================================*/
/*!
@brief
  This callback function is invoked by the QCCI infrastructure when
  infrastructure processes a client release for this service

@param[i]  cb_data         Callback data
*/
/*=========================================================================*/
static void
sns_sam_client_release_cb( void * data )
{
  uint8_t i;
  sns_sam_mr_algo_conn_hndl *algo_conn_hndl;
  uint8_t os_err;
  bool free_algo_conn_hndl = true;
  sns_sam_client_release_cb_data_s * cb_data = (sns_sam_client_release_cb_data_s *)data;

  if( NULL == cb_data )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Invalid callback data" );
    return;
  }

  algo_conn_hndl = cb_data->algo_conn_hndl;
  if( NULL == algo_conn_hndl )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Invalid algo conn hndl" );
    SNS_OS_FREE( cb_data );
    return;
  }

  i = cb_data->svc_index;
  if( SNS_SAM_SVC_ARR_LEN <= i )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD, "Invalid index %d", i );
    SNS_OS_FREE( algo_conn_hndl );
    SNS_OS_FREE( cb_data );
    return;
  }

  // Process client release from dependent SAM algorithm
  sns_os_mutex_del( algo_conn_hndl->ind_cb_data[i]->algo_inst_id_mutex, 0, &os_err );
  SNS_OS_FREE( algo_conn_hndl->ind_cb_data[i] );
  algo_conn_hndl->service_handles[i] = NULL;

  // Check if all client release requests have been processed
  for( i = 0; i < SNS_SAM_SVC_ARR_LEN; i++ )
  {
    if( NULL != algo_conn_hndl->service_handles[i] )
    {
      free_algo_conn_hndl = false;
    }
  }

  // Free algo_conn_hndl when there are no more releases pending processing
  if( free_algo_conn_hndl )
  {
    SNS_OS_FREE( algo_conn_hndl );
  }

  SNS_OS_FREE( cb_data );
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_delete_algo_inst
  =========================================================================*/
sns_err_code_e
sns_sam_mr_delete_algo_inst( sns_sam_mr_algo_conn_hndl *algo_conn_hndl )
{
  uint8_t i;
  bool delayed_free = false;

  if( NULL == algo_conn_hndl )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Invalid parameter" );
    return SNS_ERR_BAD_PTR;
  }

  // Request to release client from all dependent SAM services
  for( i = 0; i < SNS_SAM_SVC_ARR_LEN; i++ )
  {
    if( NULL != algo_conn_hndl->service_handles[ i ] )
    {
      sns_sam_client_release_cb_data_s * cb_data =
          SNS_OS_MALLOC( SNS_SAM_MODULE, sizeof(sns_sam_client_release_cb_data_s) );

      if( cb_data )
      {
        cb_data->algo_conn_hndl = algo_conn_hndl;
        cb_data->svc_index = i;
      }

      qmi_client_release_async( algo_conn_hndl->service_handles[ i ],
                                sns_sam_client_release_cb, (void*)cb_data );
      delayed_free = true;
    }
  }

  if( !delayed_free )
  {
    // will be freed in sns_sam_client_release_cb() otherwise
    SNS_OS_FREE( algo_conn_hndl );
  }

  return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_init_algo_inst
  =========================================================================*/
sns_err_code_e
sns_sam_mr_init_algo_inst( sns_sam_mr_algo_conn_hndl **algo_conn_hndl,
                           uint8_t svc_num )
{
  uint8_t i;

  *algo_conn_hndl = SNS_OS_MALLOC( SNS_SAM_MODULE, sizeof(sns_sam_mr_algo_conn_hndl) );
  if( NULL == *algo_conn_hndl )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
    return SNS_ERR_NOMEM;
  }

  for( i = 0; i < SNS_SAM_SVC_ARR_LEN; i++ )
  {
    (*algo_conn_hndl)->service_handles[ i ] = NULL;
    (*algo_conn_hndl)->ind_cb_data[ i ] = NULL;
  }

  (*algo_conn_hndl)->svc_num = svc_num;

  return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_que_comp_cli_id
  =========================================================================*/
/*!
@brief
 * Comparison function to be used by linear search feature of the client queue
 * Returns the item that matches the given mr client ID.
 */
/*=========================================================================*/
static int
sns_sam_que_comp_cli_id( void* item_ptr, void* compare_val )
{
  sns_sam_mr_cli_conn_hndl *sam_client_info = (sns_sam_mr_cli_conn_hndl*)item_ptr;
  sns_sam_mr_conn_hndl *client_id = (sns_sam_mr_conn_hndl*)compare_val;

  if( *client_id == sam_client_info->mr_clnt_id )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/*=============================================================================
  CALLBACK FUNCTION sns_sam_client_ind_cb
=============================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an indication for this client

@param[i]  user_handle         Opaque handle used by the infrastructure to
                                identify different services.
@param[i]  msg_id              Message ID of the indication
@param[i]  ind_buf             Buffer holding the encoded indication
@param[i]  ind_buf_len         Length of the encoded indication
@param[i]  ind_cb_data         Cookie value supplied by the client during registration

*/
/*=========================================================================*/
static void sns_sam_mr_ind_cb( qmi_client_type user_handle, unsigned int msg_id,
    void *ind_buf, unsigned int ind_buf_len, void *ind_cb_data )
{
  uint8_t os_err;
  sns_sam_q_item_s *queue_item;
  sns_smr_header_s smr_ind_header;
  void *smr_ind_msg;
  sns_sam_ind_cb_data *cb_data = (sns_sam_ind_cb_data*)ind_cb_data;
  qmi_client_error_type qmi_err;
  uint32_t decoded_size;

  qmi_err = qmi_idl_get_message_c_struct_len( cb_data->service_obj,
      QMI_IDL_INDICATION, msg_id, &decoded_size );
  if( QMI_NO_ERR != qmi_err )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
           "Received error from QMI framework call %d" ,
           qmi_err );
    return ;
  }

  smr_ind_msg = sns_smr_msg_alloc( SNS_SAM_MODULE, decoded_size );
  if( NULL == smr_ind_msg )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
    return ;
  }

  qmi_err = qmi_client_message_decode( user_handle, QMI_IDL_INDICATION,
      msg_id, ind_buf, ind_buf_len, smr_ind_msg, decoded_size );

  if (qmi_err != QMI_NO_ERR)
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
           "Received error from QMI framework call %d" ,
           qmi_err );
    sns_smr_msg_free( smr_ind_msg );
    return;
  }

  queue_item = SNS_OS_MALLOC( SNS_SAM_MODULE, sizeof(sns_sam_q_item_s) );
  if( NULL == queue_item )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
    sns_smr_msg_free( smr_ind_msg );
    return ;
  }
  sns_q_link( queue_item, &queue_item->q_link );
  queue_item->msg = smr_ind_msg;

  sns_os_mutex_pend( cb_data->algo_inst_id_mutex, 0, &os_err );
  if( os_err != OS_ERR_NONE )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
           "Error acquiring mutex %d",
           os_err );
    SNS_OS_FREE( queue_item );
    sns_smr_msg_free( smr_ind_msg );
    return ;
  }

  smr_ind_header.ext_clnt_id = cb_data->algo_inst_id;
  os_err = sns_os_mutex_post( cb_data->algo_inst_id_mutex );
  SNS_ASSERT( os_err == OS_ERR_NONE );

  smr_ind_header.src_module = cb_data->module;
  smr_ind_header.dst_module = SNS_SAM_MODULE;
  smr_ind_header.body_len = decoded_size;
  smr_ind_header.txn_id = 0;
  smr_ind_header.msg_id = msg_id;
  smr_ind_header.msg_type = QMI_IDL_INDICATION;
  smr_ind_header.svc_num = cb_data->svc_num;
  sns_smr_set_hdr( &smr_ind_header, smr_ind_msg );

  sns_os_mutex_pend( sns_sam_msg_queue_mutex, 0, &os_err );
  if( os_err != OS_ERR_NONE )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
           "Error acquiring mutex %d",
           os_err );
    sns_smr_msg_free( smr_ind_msg );
    SNS_OS_FREE( queue_item );
    return ;
  }

  if( SNS_SAM_MR_MSG_Q_MAX < sns_q_cnt( &sns_sam_msg_queue ) )
  {
    sns_smr_msg_free( smr_ind_msg );
    SNS_OS_FREE( queue_item );
    SNS_PRINTF_STRING_HIGH_3( SNS_SAM_DBG_MOD,
           "SAM dropped unexpected message id %d, from module %d for service %d",
           msg_id, cb_data->module, cb_data->svc_num );
  }
  else
  {
    sns_q_put( &sns_sam_msg_queue, &queue_item->q_link );
  }

  os_err = sns_os_mutex_post( sns_sam_msg_queue_mutex );
  SNS_ASSERT( os_err == OS_ERR_NONE );

  sns_os_sigs_post( sns_sam_sig_event_acc(), SNS_SAM_MSG_SIG, OS_FLAG_SET, &os_err );
}

/*=========================================================================
  CALLBACK FUNCTION:  sns_sam_client_resp_cb
  =========================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an asynchronous response for this client

@param[i]   user_handle         Opaque handle used by the infrastructure to
                                 identify different services.
@param[i]   msg_id              Message ID of the response
@param[i]   buf                 Buffer holding the decoded response
@param[i]   len                 Length of the decoded response
@param[i]   cb_data             Cookie value supplied by the client
@param[i]   transp_err          Error value

*/
/*=======================================================================*/
void sns_sam_mr_resp_cb( qmi_client_type user_handle, unsigned int msg_id,
    void *resp_c_struct, unsigned int resp_c_struct_len, void *cb_data,
    qmi_client_error_type transp_err )
{
  sns_smr_header_s smr_resp_header;
  sns_sam_mr_resp_cb_data *resp_cb_data = (sns_sam_mr_resp_cb_data*)cb_data;
  sns_sam_q_item_s *queue_item;
  uint8_t os_err;

  UNREFERENCED_PARAMETER(user_handle);

  smr_resp_header.src_module = sns_smr_get_module_id( resp_cb_data->smr_header.svc_num );
  smr_resp_header.dst_module = SNS_SAM_MODULE;
  smr_resp_header.txn_id = resp_cb_data->smr_header.txn_id;
  smr_resp_header.ext_clnt_id = resp_cb_data->smr_header.ext_clnt_id;
  smr_resp_header.msg_type = SNS_SMR_MSG_TYPE_RESP;
  smr_resp_header.svc_num = resp_cb_data->smr_header.svc_num;
  smr_resp_header.msg_id = msg_id;
  smr_resp_header.body_len = resp_c_struct_len;
  smr_resp_header.priority = SNS_SMR_MSG_PRI_HIGH;

  if( QMI_NO_ERR != transp_err )
  {
    sns_common_resp_s_v01 *resp_buf = (sns_common_resp_s_v01*)resp_c_struct;
    resp_buf->sns_result_t = 1;
    resp_buf->sns_err_t = SENSOR1_EUNKNOWN;

    smr_resp_header.body_len = sizeof(sns_common_resp_s_v01);
  }

  queue_item = SNS_OS_MALLOC( SNS_SAM_MODULE, sizeof(sns_sam_q_item_s) );
  SNS_ASSERT( NULL != queue_item );
  sns_q_link( queue_item, &queue_item->q_link );
  queue_item->msg = resp_c_struct;

  SNS_OS_FREE( resp_cb_data );
  sns_smr_set_hdr( &smr_resp_header, resp_c_struct );

  sns_os_mutex_pend( sns_sam_msg_queue_mutex, 0, &os_err );
  if( os_err != OS_ERR_NONE )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
           "Error acquiring mutex %d",
           os_err );
    sns_smr_msg_free( resp_c_struct );
    SNS_OS_FREE( queue_item );
    return ;
  }

  sns_q_put( &sns_sam_msg_queue, &queue_item->q_link );

  os_err = sns_os_mutex_post( sns_sam_msg_queue_mutex );
  SNS_ASSERT( os_err == OS_ERR_NONE );

  sns_os_sigs_post( sns_sam_sig_event_acc(), SNS_SAM_MSG_SIG, OS_FLAG_SET, &os_err );
}

/*=========================================================================
  CALLBACK FUNCTION:  sns_sam_mr_get_cli_id
  =========================================================================*/
/*!
@brief
  Returns an unused client ID, or SNS_SAM_MAX_ALGO_INSTS if none available.

  sns_sam_cli_queue_mutex must be held prior to making this call.
*/
/*=======================================================================*/
static sns_sam_mr_conn_hndl
sns_sam_mr_get_cli_id( void )
{
  static uint8_t last_id = 0;
  uint8_t start = last_id;
  sns_sam_mr_cli_conn_hndl *sam_client_info;

  do
  {
    sam_client_info = sns_q_linear_search( &sns_sam_cli_queue,
        sns_sam_que_comp_cli_id, &last_id );
    if( NULL == sam_client_info )
    {
      return (sns_sam_mr_conn_hndl)last_id;
    }

    last_id = ( last_id < SNS_SAM_MAX_CLI_ID - 1 ) ? last_id + 1 : 0;
  } while( start != last_id );  /* Only iterate once over all possible IDs */

  return (sns_sam_mr_conn_hndl)SNS_SAM_MAX_CLI_ID;
}

/*=========================================================================
  CALLBACK FUNCTION:  sns_sam_connect_cb
  =========================================================================*/
/*!
  @brief Callback registered with QCSI to receive connect requests
*/
/*=======================================================================*/
static qmi_csi_cb_error
sns_sam_mr_connect_cb( qmi_client_handle client_handle,
    void *service_cookie, void **connection_handle )
{
  sns_sam_mr_cli_conn_hndl *sam_client_info;
  uint8_t os_err;

  SNS_PRINTF_STRING_LOW_1( SNS_SAM_DBG_MOD,
         "Received QCSI connect callback for service %i",
         *((uint16_t*)service_cookie) );

 /* Assign client_handle pointer to connection_handle so that it can be
    used in the handle_req_cb to send indications if necessary */
  if( NULL == connection_handle )
  {
    return QMI_CSI_CB_CONN_REFUSED;
  }

  sam_client_info = SNS_OS_MALLOC( SNS_SAM_MODULE,
      sizeof(sns_sam_mr_cli_conn_hndl) ); /* Freed in disconnect_cb */
  if( NULL == sam_client_info )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
    return QMI_CSI_CB_CONN_REFUSED;
  }
  sns_q_link( sam_client_info, &sam_client_info->q_link );

  sns_os_mutex_pend( sns_sam_cli_queue_mutex, 0, &os_err );
  if( OS_ERR_NONE != os_err  )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
           "Error acquiring mutex %d",
           os_err );
    SNS_OS_FREE( sam_client_info );
    return QMI_CSI_CB_CONN_REFUSED;
  }

  sam_client_info->client_handle = client_handle;
  sam_client_info->svc_num = *((uint16_t*)service_cookie);
  sam_client_info->mr_clnt_id = sns_sam_mr_get_cli_id();
  sam_client_info->ready_to_release = false;
  if( SNS_SAM_MAX_CLI_ID <= (int)sam_client_info->mr_clnt_id  )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Max number of client request reached" );
    SNS_OS_FREE( sam_client_info );

    os_err = sns_os_mutex_post( sns_sam_cli_queue_mutex );
    SNS_ASSERT( os_err == OS_ERR_NONE );

    return QMI_CSI_CB_CONN_REFUSED;
  }

  *connection_handle = (void*)sam_client_info;

  sns_q_put( &sns_sam_cli_queue, &sam_client_info->q_link );

  os_err = sns_os_mutex_post( sns_sam_cli_queue_mutex );
  SNS_ASSERT( os_err == OS_ERR_NONE );

  return QMI_CSI_NO_ERR;
}

/*=========================================================================
  FUNCTION:  sns_sam_que_comp_disc
  =========================================================================*/
/*!
@brief
   Comparison function to be used by linear search feature of the client queue
   Returns the item that has the ready_to_release set.
 */
/*=========================================================================*/
static int
sns_sam_que_comp_disc( void *item_ptr, void *compare_val )
{
  sns_sam_mr_cli_conn_hndl *conn_hndl = (sns_sam_mr_cli_conn_hndl*)item_ptr;

  UNREFERENCED_PARAMETER(compare_val);

  if( conn_hndl->ready_to_release )
  {
    return 1;
  }

  return 0;
}

/*=========================================================================
  CALLBACK FUNCTION:  sns_sam_mr_release_conn
  =========================================================================*/
/*!
  @brief Process all disconnect requests received.
*/
/*=======================================================================*/
static void
sns_sam_mr_release_conn( void )
{
  uint8_t os_err;
  uint8_t clientReqId;
  sns_sam_mr_cli_conn_hndl *conn_hndl;
  sns_err_code_e err;
  sns_smr_header_s msgHdr;
  SNS_PRINTF_STRING_LOW_0( SNS_SAM_DBG_MOD, "Processing all disconnect requests" );

  // TODO: Remove lock for linear search by changing the algorithm
  sns_os_mutex_pend( sns_sam_cli_queue_mutex, 0, &os_err );
  if( os_err != OS_ERR_NONE )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD, "Error acquiring mutex %d", os_err );
    return ;
  }

  do
  {
    conn_hndl = (sns_sam_mr_cli_conn_hndl*)sns_q_linear_search(
        &sns_sam_cli_queue, sns_sam_que_comp_disc, NULL );

    if( conn_hndl )
    {
      SNS_PRINTF_STRING_LOW_2( SNS_SAM_DBG_MOD, "Disconnect clnt id %i (svc num %i)",
                              (int)conn_hndl->mr_clnt_id, conn_hndl->svc_num );

      /* Usually SAM will receive a disable/cancel message for this service from ACM
       * prior to receiving the disconnect, but in case not, we should clean-up the
       * associated SAM and SMGR data and requests */
      msgHdr.svc_num = conn_hndl->svc_num;
      clientReqId = sns_sam_find_client_req( &msgHdr, conn_hndl->mr_clnt_id );
      if( SNS_SAM_INVALID_ID != clientReqId )
      {
        err = sns_sam_disable_algo( clientReqId );
        if( err != SNS_SUCCESS )
        {
          SNS_PRINTF_STRING_ERROR_2( SNS_SAM_DBG_MOD,
                                     "Error disabling client request ID %i (mr clnt id %i)",
                                     clientReqId, (int)conn_hndl->mr_clnt_id );
        }
      }

      sns_q_delete( &conn_hndl->q_link );
      SNS_OS_FREE( conn_hndl );
    }
  } while( conn_hndl );

  os_err = sns_os_mutex_post( sns_sam_cli_queue_mutex );
  SNS_ASSERT( os_err == OS_ERR_NONE );

  SNS_PRINTF_STRING_LOW_0( SNS_SAM_DBG_MOD, "Processing complete" );
}

/*=========================================================================
  CALLBACK FUNCTION:  sns_sam_disconnect_cb
  =========================================================================*/
/*!
  @brief Callback registered with QCSI to receive disconnect requests
*/
/*=======================================================================*/
static void
sns_sam_mr_disconnect_cb( void *connection_handle, void *service_cookie )
{
  uint8_t os_err;
  sns_sam_mr_cli_conn_hndl *conn_hndl = (sns_sam_mr_cli_conn_hndl*)connection_handle;

  SNS_PRINTF_STRING_LOW_1( SNS_SAM_DBG_MOD, "QCSI disconnect callback for cookie %d", *((uint16_t*)service_cookie) );

  sns_os_mutex_pend( sns_sam_cli_queue_mutex, 0, &os_err );
  if( os_err != OS_ERR_NONE )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD, "Error acquiring mutex %d", os_err );
    return ;
  }
  conn_hndl->ready_to_release = true;

  os_err = sns_os_mutex_post( sns_sam_cli_queue_mutex );
  SNS_ASSERT( os_err == OS_ERR_NONE );

  sns_os_sigs_post( sns_sam_sig_event_acc(), SNS_SAM_QMI_DISC_SIG, OS_FLAG_SET, &os_err );
}

/*=========================================================================
  CALLBACK FUNCTION:  sns_sam_handle_req_cb
  =========================================================================*/
/*!
  @brief Callback registered with QCSI to receive service requests
*/
/*=======================================================================*/
static qmi_csi_cb_error
sns_sam_mr_process_req( void *connection_handle, qmi_req_handle req_handle,
    unsigned int msg_id, void *req_c_struct, unsigned int req_c_struct_len,
    void *service_cookie )
{
  sns_smr_header_s smr_req_header;
  void *smr_req_msg;
  sns_sam_q_item_s *queue_item;
  uint16_t svc_num = *(uint16_t*)service_cookie;
  sns_sam_mr_cli_conn_hndl *sam_client_info = (sns_sam_mr_cli_conn_hndl*)connection_handle;
  uint8_t os_err;

  smr_req_msg = sns_smr_msg_alloc( SNS_SAM_MODULE, req_c_struct_len );
  if( NULL == smr_req_msg )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
    return QMI_CSI_CB_NO_MEM;
  }

  queue_item = SNS_OS_MALLOC( SNS_SAM_MODULE, sizeof(sns_sam_q_item_s) );
  if( NULL == queue_item )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
    sns_smr_msg_free( smr_req_msg );
    return QMI_CSI_CB_NO_MEM;
  }
  sns_q_link( queue_item, &queue_item->q_link );
  queue_item->msg = smr_req_msg;
  queue_item->req_handle = req_handle;

  smr_req_header.ext_clnt_id = (int)sam_client_info->mr_clnt_id;
  smr_req_header.src_module = 0;
  smr_req_header.dst_module = SNS_SAM_MODULE;
  smr_req_header.msg_type = SNS_SMR_MSG_TYPE_REQ;
  smr_req_header.svc_num = svc_num;
  smr_req_header.msg_id = msg_id;
  smr_req_header.body_len = req_c_struct_len;
  sns_smr_set_hdr( &smr_req_header, smr_req_msg );

  SNS_OS_MEMCOPY( smr_req_msg, req_c_struct, req_c_struct_len );
  sns_os_mutex_pend( sns_sam_msg_queue_mutex, 0, &os_err );
  if( os_err != OS_ERR_NONE )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
           "Error acquiring mutex %d",
           os_err );
    sns_smr_msg_free( smr_req_msg );
    SNS_OS_FREE( queue_item );
    return QMI_CSI_CB_INTERNAL_ERR;
  }
  sns_q_put( &sns_sam_msg_queue, &queue_item->q_link );

  os_err = sns_os_mutex_post( sns_sam_msg_queue_mutex );
  SNS_ASSERT( os_err == OS_ERR_NONE );

  sns_os_sigs_post( sns_sam_sig_event_acc(), SNS_SAM_MSG_SIG, OS_FLAG_SET, &os_err );
  return QMI_CSI_CB_NO_ERR;
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_send_ind
  =========================================================================*/
/*!
@brief
  Send an indication.  To be used by SAM services.

  @param[i] msg_ptr Message allocated by sns_smr_msg_alloc.
  @param[i] mr_clnt_id MR Client ID.
  @param[i] send_ind_during_suspend If this indication should be sent when
            processor is suspended.

  @return SNS_SUCCESS or error code.
 */
/*=========================================================================*/
sns_err_code_e
sns_sam_mr_send_ind( void *msg_ptr, sns_sam_mr_conn_hndl mr_clnt_id, bool send_ind_during_suspend )
{
  sns_smr_header_s smr_header;
  sns_sam_mr_cli_conn_hndl *sam_client_info;
  qmi_client_handle client_handle;
  uint8_t os_err;
  sns_err_code_e rv = SNS_SUCCESS;

  if( NULL == msg_ptr )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "NULL msg_ptr" );
    rv = SNS_ERR_BAD_PTR;
  }
  else
  {
    /* Filter out indications when kernel has suspended and when
       client has opted not to get indications during suspend */
      UNREFERENCED_PARAMETER(send_ind_during_suspend);
    //if( (!linux_kernel_suspend) || (send_ind_during_suspend) )
    //{
      sns_smr_get_hdr( &smr_header, msg_ptr );

      // TODO: Remove lock for linear search by changing the algorithm
      sns_os_mutex_pend( sns_sam_cli_queue_mutex, 0, &os_err );
      if( os_err != OS_ERR_NONE )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
             "Error acquiring mutex %d",
             os_err );
        rv = SNS_ERR_FAILED;
      }
      else
      {
        sam_client_info = sns_q_linear_search( &sns_sam_cli_queue,
            sns_sam_que_comp_cli_id, &mr_clnt_id );
        if( NULL == sam_client_info )
        {
          SNS_PRINTF_STRING_ERROR_2( SNS_SAM_DBG_MOD,
                          "Unable to find ext cli id %d (svc %d)",
                          smr_header.ext_clnt_id, smr_header.svc_num );

          os_err = sns_os_mutex_post( sns_sam_cli_queue_mutex );
          SNS_ASSERT( os_err == OS_ERR_NONE );

          rv = SNS_ERR_FAILED;
        }
        else
        {
          client_handle = sam_client_info->client_handle;

          SNS_PRINTF_STRING_LOW_2( SNS_SAM_DBG_MOD,
                          "Sent algo report to client %d for service %d",
                          smr_header.ext_clnt_id, smr_header.svc_num );

          os_err = sns_os_mutex_post( sns_sam_cli_queue_mutex );
          SNS_ASSERT( os_err == OS_ERR_NONE );

          qmi_csi_send_ind( client_handle, smr_header.msg_id,
                          (void*)msg_ptr, smr_header.body_len );
        }
      }
    //}

    sns_smr_msg_free( msg_ptr );
  }

  return rv;
}

/*=============================================================================
  FUNCTION sns_sam_mr_get_svc_index
=============================================================================*/
/*!
@brief
  Retrieves the array index of a src-des pair, where each are SAM services.

  @param[i] src_svc_num Service Number of sender
  @param[i] dst_svc_num Service number of destination

  @return Index into ind_cb_data or service_handles arrays
*/
/*=======================================================================*/
static uint8_t
sns_sam_mr_get_svc_index( uint8_t src_svc_num, uint8_t dst_svc_num )
{
  uint8_t i, src_algo_index, rv = SNS_SAM_SVC_ARR_LEN;

  if( SNS_TIME2_SVC_ID_V01 == dst_svc_num )
  {
    rv = SNS_SAM_SVC_ARR_LEN - 1;
  }
  // The registry and SMGR are only accessed from SAM framework, which has a
  // dedicated mr_clnt_id of 0, but no entries in sns_sam_algo_dbase.
  else if( SNS_SMGR_SVC_ID_V01 == dst_svc_num )
  {
    rv = 0;
  }
  else if( SNS_REG2_SVC_ID_V01 == dst_svc_num )
  {
    rv = 1;
  }
  else if( SNS_PM_SVC_ID_V01 == dst_svc_num )
  {
    rv = 2;
  }
  else if( SNS_SMGR_INTERNAL_SVC_ID_V01 == dst_svc_num )
  {
    rv = 3;
  }
  else
  {
    src_algo_index = sns_sam_get_algo_index( src_svc_num );
    for( i = 0; i < SNS_SAM_MAX_ALGO_DEPS; i++ )
    {
      if( NULL != sns_sam_algo_dbase_acc( src_algo_index ) &&
          dst_svc_num == sns_sam_algo_dbase_acc( src_algo_index )->algoDepDbase[ i ] )
      {
        rv = i;
        break;
      }
    }
  }

  return rv;
}

/*=============================================================================
  FUNCTION sns_sam_mr_get_handle
=============================================================================*/
/*!
@brief
  Retrieves the service handle for a given service number, client ID pair.
  Will initialize the handle if it does not already exist.

  @param[i] svc_num Service Number
  @param[i] ext_clnt_id External client ID
  @param[i] algo_inst_id Algorithm instance ID as specified by the SAM framework

  @return QMI service handle
*/
/*=======================================================================*/
static qmi_client_type
sns_sam_mr_get_handle( uint16_t svc_num, sns_sam_mr_algo_conn_hndl *algo_conn_hndl,
                       uint8_t algo_inst_id )
{
  qmi_service_info service_info;
  sns_sam_ind_cb_data *ind_cb_data;
  qmi_client_error_type qmi_err;
  uint8_t os_err;
  uint8_t svc_index;
  sns_err_code_e sns_err;

  sns_os_mutex_pend( sns_sam_cli_queue_mutex, 0, &os_err );
  if( os_err != OS_ERR_NONE )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
           "Error acquiring mutex %d",
           os_err );
    return NULL;
  }

  svc_index = sns_sam_mr_get_svc_index( algo_conn_hndl->svc_num, svc_num );
  if( SNS_SAM_SVC_ARR_LEN <= svc_index )
  {
    SNS_PRINTF_STRING_ERROR_2( SNS_SAM_DBG_MOD,
           "Invalid index %d for algorithm service %d",
           svc_index, svc_num );
    os_err = sns_os_mutex_post( sns_sam_cli_queue_mutex );
    SNS_ASSERT( os_err == OS_ERR_NONE );
    return NULL;
  }

  os_err = sns_os_mutex_post( sns_sam_cli_queue_mutex );
  SNS_ASSERT( os_err == OS_ERR_NONE );

  if( NULL == algo_conn_hndl->service_handles[ svc_index ] )
  {
    int timeout_ms = ( svc_num == SNS_SMGR_SVC_ID_V01 ) ? 10000 : 100;
    qmi_idl_service_object_type service = sns_smr_get_svc_obj( svc_num );
    SNS_ASSERT( service != NULL );

    ind_cb_data = SNS_OS_MALLOC( SNS_MODULE_SAM, sizeof(sns_sam_ind_cb_data) );
    if( NULL == ind_cb_data )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
      return NULL;
    }

    sns_err = sns_smr_get_qmi_service_info( &service, timeout_ms, &service_info );
    if( SNS_SUCCESS != sns_err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
             "Unable to initialize service %d with QCCI",
             svc_num );
      SNS_OS_FREE( ind_cb_data );
      return NULL;
    }

    ind_cb_data->svc_num = svc_num;
    ind_cb_data->service_obj = service;
    ind_cb_data->algo_conn_hndl = algo_conn_hndl;
    ind_cb_data->module = sns_smr_get_module_id( svc_num );
    ind_cb_data->algo_inst_id = algo_inst_id;

    const uint8_t priority_0 = 0;
    ind_cb_data->algo_inst_id_mutex = sns_os_mutex_create( priority_0, &os_err );
    SNS_ASSERT( os_err == OS_ERR_NONE );

    algo_conn_hndl->ind_cb_data[ svc_index ] = ind_cb_data;

    qmi_err = qmi_client_init( &service_info, service,
                               sns_sam_mr_ind_cb,
                               ind_cb_data, NULL,
                               &algo_conn_hndl->service_handles[ svc_index ] );
  }
  else
  {
    ind_cb_data = algo_conn_hndl->ind_cb_data[ svc_index ];

    sns_os_mutex_pend( ind_cb_data->algo_inst_id_mutex, 0, &os_err );
    if( os_err != OS_ERR_NONE )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
             "Error acquiring mutex %d",
             os_err );
      return NULL;
    }

    ind_cb_data->algo_inst_id = algo_inst_id;

    os_err = sns_os_mutex_post( ind_cb_data->algo_inst_id_mutex );
    SNS_ASSERT( os_err == OS_ERR_NONE );
  }

  return algo_conn_hndl->service_handles[ svc_index ];
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_send
  =========================================================================*/
sns_err_code_e
sns_sam_mr_send( void *msg_ptr, sns_sam_mr_algo_conn_hndl *algo_conn_hndl )
{
  qmi_idl_service_object_type service;
  uint32_t resp_msg_size;
  void *resp_msg;
  qmi_txn_handle qmi_txn;
  qmi_client_error_type qmi_err;
  qmi_client_type service_handle = NULL;
  sns_sam_mr_resp_cb_data *resp_cb_data;
  sns_err_code_e rv = SNS_SUCCESS;
  int32_t qmi_idl_err;

  // If SAM is trying to send a request to a non-SAM service
  if( NULL == algo_conn_hndl )
  {
    algo_conn_hndl = sns_sam_mr_fw_clnt_hndl;
  }

  if( NULL == msg_ptr )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "NULL msg_ptr" );
    rv = SNS_ERR_BAD_PTR;
  }
  else
  {
    resp_cb_data = SNS_OS_MALLOC( SNS_MODULE_SAM, sizeof(sns_sam_mr_resp_cb_data) );
    if( NULL == resp_cb_data )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
      rv = SNS_ERR_NOMEM;
    }
    else
    {
      sns_smr_get_hdr( &resp_cb_data->smr_header, msg_ptr );
      resp_cb_data->algo_conn_hndl = algo_conn_hndl;

      SNS_PRINTF_STRING_LOW_1( SNS_SAM_DBG_MOD, "Preparing request for service %i",
                               resp_cb_data->smr_header.svc_num );

      service = sns_smr_get_svc_obj( resp_cb_data->smr_header.svc_num );
      service_handle = sns_sam_mr_get_handle( resp_cb_data->smr_header.svc_num,
                                              algo_conn_hndl,
                                              resp_cb_data->smr_header.ext_clnt_id );

      if( NULL == service_handle )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD, "Unable to acquire service handle %d",
            resp_cb_data->smr_header.svc_num );
        SNS_OS_FREE( resp_cb_data );
        rv = SNS_ERR_INVALID_HNDL;
      }
      else if( NULL == service )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD, "Unable to acquire service handle %d",
            resp_cb_data->smr_header.svc_num );
        SNS_OS_FREE( resp_cb_data );
        rv = SNS_ERR_INVALID_HNDL;
      }
      else
      {
        qmi_idl_err = qmi_idl_get_message_c_struct_len(
                        service, QMI_IDL_RESPONSE, resp_cb_data->smr_header.msg_id, &resp_msg_size );

        if( QMI_IDL_LIB_NO_ERR != qmi_idl_err)
        {
          SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
            "Received error from QMI framework call %d" , qmi_idl_err );
          SNS_OS_FREE( resp_cb_data );
          rv = ((qmi_idl_err==QMI_IDL_LIB_MESSAGE_ID_NOT_FOUND) ? SNS_ERR_BAD_MSG_ID : SNS_ERR_FAILED);
        }
        else
        {
          resp_msg = sns_smr_msg_alloc( SNS_DBG_MOD_APPS_ACM, resp_msg_size );
          if( NULL == resp_msg )
          {
            SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
            SNS_OS_FREE( resp_cb_data );
            rv = SNS_ERR_NOMEM;
          }
          else
          {
            qmi_err = qmi_client_send_msg_async( service_handle, resp_cb_data->smr_header.msg_id, msg_ptr,
                resp_cb_data->smr_header.body_len, resp_msg, resp_msg_size, sns_sam_mr_resp_cb,
                (void*)resp_cb_data, &qmi_txn );

            if( QMI_NO_ERR != qmi_err )
            {
              SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
              "Received error from QMI framework call %d" ,
              qmi_err );
              SNS_OS_FREE( resp_cb_data );
              sns_smr_msg_free( resp_msg );
              rv = SNS_ERR_FAILED;
            }
          }
        }
      }
    }
    sns_smr_msg_free( msg_ptr );
  }

  return rv;
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_msg_rcv
  =========================================================================*/
void* sns_sam_mr_msg_rcv( void )
{
  uint8_t os_err;
  sns_sam_q_item_s *q_item = NULL;
  void *msg = NULL;
  sns_smr_header_s smr_header;

  while( true )
  {
    sns_os_mutex_pend( sns_sam_msg_queue_mutex, 0, &os_err );
    if( os_err != OS_ERR_NONE )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
             "Error acquiring mutex %d",
             os_err );
      return NULL;
    }
    q_item = sns_q_get( &sns_sam_msg_queue );
    os_err = sns_os_mutex_post( sns_sam_msg_queue_mutex );
    SNS_ASSERT( os_err == OS_ERR_NONE );

    if( NULL == q_item )
    {
      msg = NULL;
      break;
    }
    else
    {
      msg = q_item->msg;
      sns_smr_get_hdr( &smr_header, msg );

      if( QMI_IDL_REQUEST == smr_header.msg_type )
      {
        qmi_csi_error csi_err;
        sns_smr_header_s smr_resp_header;
        void *smr_resp_msg;
        sns_err_code_e err;

        SNS_PRINTF_STRING_LOW_2( SNS_SAM_DBG_MOD,
            "SAM processing request %i for service %i",
            smr_header.msg_id, smr_header.svc_num );

        err = sns_sam_process_client_req(
            msg, (sns_sam_mr_conn_hndl)smr_header.ext_clnt_id, &smr_resp_msg );
        if( err != SNS_SUCCESS )
        {
          sns_sam_report_client_error( msg, &smr_resp_msg, err );
        }
        SNS_PRINTF_STRING_LOW_0( SNS_SAM_DBG_MOD, "SAM request processed" );

        sns_smr_get_hdr( &smr_resp_header, smr_resp_msg );
        csi_err = qmi_csi_send_resp( q_item->req_handle, smr_header.msg_id,
                                     smr_resp_msg, smr_resp_header.body_len );
        if( QMI_CSI_NO_ERR != csi_err )
        {
          SNS_PRINTF_STRING_HIGH_1( SNS_SAM_DBG_MOD,
              "qmi_csi_send_resp error %i", csi_err );
        }

        sns_smr_msg_free( msg );
        sns_smr_msg_free( smr_resp_msg );
        SNS_OS_FREE( q_item );
        q_item = NULL;
      }
      else
      {
        SNS_OS_FREE( q_item );
        break;
      }
    }
  }
  return msg;
}

#if !defined(SNS_DSPS_BUILD) && !defined(SNS_PCSIM) && !defined(QDSP6)
/*=========================================================================
  FUNCTION:  sns_sam_mr_thread
  =========================================================================*/
static void
sns_sam_mr_thread( void *p_arg )
{
  qmi_csi_os_params os_params_in;
  fd_set fds;
  uint16_t svc_num = *((uint16_t*)p_arg);
  uint8_t algo_index = sns_sam_get_algo_index( svc_num );

  SNS_OS_FREE( p_arg );

  if(algo_index < SNS_SAM_MAX_ALGOS)
  {
    while( true )
    {
      fds = sam_qmi_svc[ algo_index ].os_params.fds;
      select( sam_qmi_svc[ algo_index ].os_params.max_fd + 1, &fds, NULL, NULL, NULL );
      os_params_in.fds = fds;
      qmi_csi_handle_event( sam_qmi_svc[ algo_index ].service_handle, &os_params_in );
    }

    qmi_csi_unregister( sam_qmi_svc[ algo_index ].service_handle );
  }
}

#endif /* !defined(SNS_DSPS_BUILD) && !defined(SNS_PCSIM) && !defined(QDSP6)  */

/*=========================================================================
  FUNCTION:  sns_sam_mr_init
  =========================================================================*/
sns_err_code_e
sns_sam_mr_init( void )
{
  uint8_t os_err;
  sns_err_code_e sns_err;

  sns_sam_msg_queue_mutex = sns_os_mutex_create( SNS_SAM_APPS_MSG_QUE_MUTEX, &os_err );
  SNS_ASSERT( os_err == OS_ERR_NONE );
  sns_q_init( &sns_sam_msg_queue );

  sns_sam_cli_queue_mutex = sns_os_mutex_create( SNS_SAM_APPS_CLI_QUE_MUTEX, &os_err );
  SNS_ASSERT( os_err == OS_ERR_NONE );
  sns_q_init( &sns_sam_cli_queue );

  if( SNS_SUCCESS !=
      ( sns_err = sns_sam_mr_init_algo_inst( &sns_sam_mr_fw_clnt_hndl, 0 ) ) )
  {
    return sns_err;
  }

  return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_qcsi_reg
  =========================================================================*/
sns_err_code_e
sns_sam_mr_qcsi_reg( uint8_t algoSvcId )
{
  uint8_t i = algoSvcId;
  uint8_t os_err;
  uint8_t sam_sig_mask_idx = 0;
  unsigned int csi_inst_id;

  qmi_csi_options qmi_options;
  qmi_client_error_type qmi_err;

  if( NULL != sns_smr_get_svc_obj( i ) )
  {
    if( (sam_sig_mask_idx = sns_sam_get_algo_index( i )) == SNS_SAM_INVALID_ID )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
                "Unable to find algorithm index for service %d", i );
      return SNS_ERR_FAILED;
    }


#if defined(SNS_DSPS_BUILD) || defined(SNS_PCSIM) || defined(QDSP6)
    uint8_t err;
    // DSPS/ADSP builds will use signal flags instead of multiple waiting threads
    sns_os_sigs_add( sns_sam_sig_event_acc(), SNS_SAM_QMI_SV_MSG_SIG << sam_sig_mask_idx );
    sam_sig_mask |= SNS_SAM_QMI_SV_MSG_SIG << sam_sig_mask_idx;
    sns_os_set_qmi_csi_params( sns_sam_sig_event_acc(),
                              ( SNS_SAM_QMI_SV_MSG_SIG << sam_sig_mask_idx ),
                              &( sam_qmi_svc[ sam_sig_mask_idx ].os_params ), &err );
    if( err != OS_ERR_NONE )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_SAM_DBG_MOD,
                "Failed to set csi parameters for algorithm service %d", i);
      return SNS_ERR_FAILED;
    }
    csi_inst_id = SNS_SMR_SVC_PRI_MED;
#else
    csi_inst_id = SNS_SMR_SVC_PRI_LOW;
#endif

    QMI_CSI_OPTIONS_INIT( qmi_options );
    QMI_CSI_OPTIONS_SET_INSTANCE_ID( qmi_options, csi_inst_id );
    sam_qmi_svc[ sam_sig_mask_idx ].service_cookie = i;
    qmi_err = qmi_csi_register_with_options(
        sns_smr_get_svc_obj( i ), sns_sam_mr_connect_cb,
        sns_sam_mr_disconnect_cb, sns_sam_mr_process_req,
        &sam_qmi_svc[ sam_sig_mask_idx ].service_cookie,
        &sam_qmi_svc[ sam_sig_mask_idx ].os_params,
        &qmi_options,
        &sam_qmi_svc[ sam_sig_mask_idx ].service_handle );

    if( qmi_err != QMI_NO_ERR )
    {
      SNS_PRINTF_STRING_ERROR_2( SNS_SAM_DBG_MOD,
                "Failed to register algorithm service %d with QMI %d",
                i, qmi_err );
      return SNS_ERR_FAILED;
    }

#if !defined(SNS_DSPS_BUILD) && !defined(SNS_PCSIM) && !defined(QDSP6)
    uint16_t *svc_num = SNS_OS_MALLOC( SNS_MODULE_SAM, sizeof(uint16_t) );
    if( NULL == svc_num )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_SAM_DBG_MOD, "Failed to allocate resources" );
      return SNS_ERR_NOMEM;
    }
    *svc_num = i;

    os_err = sns_os_task_create( sns_sam_mr_thread, svc_num, NULL,
                                 SNS_SAM_MR_MODULE_PRIORITY );
    SNS_ASSERT( os_err == 0 );
#endif
  }
  return SNS_SUCCESS;
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_confirm_client_req
  =========================================================================*/
bool
sns_sam_mr_confirm_client_req( struct sns_sam_client_req* clientReqPtr,
                               const sns_smr_header_s *msgHdrPtr,
                               sns_sam_mr_conn_hndl mr_clnt_id )
{
  UNREFERENCED_PARAMETER(msgHdrPtr);
  return clientReqPtr->mrClientId == mr_clnt_id;
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_handle_event
  =========================================================================*/
/*!
  @brief Handle additional signal-based events pertaining to message routing.

  @param[io] sigFlags Bit array indicating signal received
*/
/*=======================================================================*/
void
sns_sam_mr_handle_event( OS_FLAGS *sigFlags )
{
  unsigned int i;

  if( *sigFlags & SNS_SAM_QMI_DISC_SIG )
  {
    sns_sam_mr_release_conn();
    *sigFlags &= (~SNS_SAM_QMI_DISC_SIG);
  }

  for( i = 0; i < SNS_SAM_MAX_ALGOS; i++ )
  {
    if( *sigFlags & ( SNS_SAM_QMI_SV_MSG_SIG << i ) )
    {
      qmi_csi_handle_event( sam_qmi_svc[ i ].service_handle, &sam_qmi_svc[ i ].os_params );
      *sigFlags &= ~( SNS_SAM_QMI_SV_MSG_SIG << i );
    }
  }
}

/*=========================================================================
  FUNCTION:  sns_sam_mr_get_client_id
  =========================================================================*/
uint8_t sns_sam_mr_get_client_id( sns_sam_mr_conn_hndl mr_clnt_id )
{
  // Connections that are managed by MR use client IDs from 0 to
  // SNS_SAM_MAX_CLI_ID - 1. Connections that are not managed by MR (dependent
  // algorithms that do not use messaging to communicate) use client IDs >=
  // SNS_SAM_MAX_CLI_ID.
  return (uint8_t)(mr_clnt_id % SNS_SAM_MAX_CLI_ID);
}
