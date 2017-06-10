/*============================================================================
  @file sns_acm_mr.c

  @brief
    This implements the message routing portion of the ACM.

  <br><br>

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
  ============================================================================*/

#include "sns_common.h"
#include "sns_acm.h"
#include "sensor1.h"
#include "sns_common_v01.h"
#include "sns_smr_util.h"
#include "sns_memmgr.h"
#include "sns_debug_api.h"
#include "sns_osa.h"
#include "sns_acm_priv.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef SNS_BLAST
  #include "sns_debug_str_mdm.h"
#else
  #include "sns_debug_str.h"
#endif

#include <qmi_client.h>
#include <qmi_idl_lib.h>
#include <qmi_cci_target_ext.h>

/*============================================================================
  Type Declarations
  ============================================================================*/
/**
 * Callback data received in every indication.
 * Contains information previously found in the smr header.
 */
typedef struct sns_acm_ind_cb_data {
  uint32_t svc_num;
  uint32_t ext_clnt_id;
  qmi_idl_service_object_type service_obj;
} sns_acm_ind_cb_data;

/**
 * Pending mesage received from QCCI, that remain to be
 * processed by the ACM thread.
 */
typedef struct sns_acm_mr_msg {
  void *msg;
  struct sns_acm_mr_msg *next;
  struct sns_acm_mr_msg *prev;
} sns_acm_mr_msg;

/**
 * Each external client will have a single entry in the external_clients array.
 */
typedef struct sns_acm_ext_client_handle {
  /** A QMI handle to each sensor1 service this client is connected to */
  qmi_client_type client_handles[ SNS_ACM_MAX_SVC_ID ];
  /** Callback data received for each service.
   * Saved here for the express purpose of freeing when completed.
   */
  sns_acm_ind_cb_data *cb_data[ SNS_ACM_MAX_SVC_ID ];
} sns_acm_ext_client_handle;

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

/** Mutex protecting the pending message queue */
static OS_EVENT *sns_acm_mr_queue_mutex;

/** Counting semaphore for the length of the mr_queue */
static sem_t sns_acm_mr_queue_sem;

/** Pending messages received by QCCI, for the ACM thread to process */
static sns_acm_mr_msg *sns_acm_mr_msg_queue_first;
static sns_acm_mr_msg *sns_acm_mr_msg_queue_last;

/** Array of QMI info, populated at init */
static qmi_service_info service_info[ SNS_ACM_MAX_SVC_ID ];
/**
 * Whether a service is enabled. Determined once at init.
 *  Should be made dynamically updatable in the future.
 */
static bool service_enabled[ SNS_ACM_MAX_SVC_ID ];
static qmi_cci_os_signal_type os_params;
static sns_acm_ext_client_handle external_clients[ SNS_ACM_MAX_CLIENTS ];

/*============================================================================
  Function Definitions and Documentation
  ============================================================================*/

/**
 * Clean-up all resources used by acm_mr associated with an external client.
 *
 * @param[i] ext_clnt_id Client ID to remove
 */
void
sns_acm_mr_close( uint32_t ext_clnt_id )
{
  int i;

  for( i = 0; i < SNS_ACM_MAX_SVC_ID; i++ )
  {
    if( NULL != external_clients[ ext_clnt_id ].cb_data[ i ] )
    {
      SNS_PRINTF_STRING_LOW_2( SNS_DBG_MOD_ACM, "Releasing qmi client %i %i", ext_clnt_id, i );
      qmi_client_release( external_clients[ ext_clnt_id ].client_handles[ i ] );
      SNS_OS_FREE( external_clients[ ext_clnt_id ].cb_data[ i ] );
      external_clients[ ext_clnt_id ].cb_data[ i ] = NULL;
      SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_ACM, "Client released" );
    }
  }

}

/**
 * Removes all messages from the ACM queue that are intended for
 * the given external client.
 *
 * @param[i] ext_clnt_id Client being closed
 *
 * @return Number of entries removed from queue.
 *         -1 upon error
 */
int
sns_acm_mr_queue_clean( uint8_t ext_clnt_id )
{
  uint8_t os_err = 0;
  sns_acm_mr_msg *queue_msg, *queue_temp;
  int rv = 0;

  sns_os_mutex_pend( sns_acm_mr_queue_mutex, 0, &os_err );
  if( 0 != os_err ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "Unable to aquire mutex %i", os_err );
    return -1;
  }

  queue_msg = sns_acm_mr_msg_queue_first;
  while( NULL != queue_msg )
  {
    sns_smr_header_s smr_header;
    sns_smr_get_hdr( &smr_header, queue_msg->msg );

    if( smr_header.ext_clnt_id == ext_clnt_id )
    {
      if( SNS_SMR_MSG_TYPE_RESP == smr_header.msg_type ||
          SNS_SMR_MSG_TYPE_RESP_INT_ERR == smr_header.msg_type )
      {
        rv++;
      }

      if( sns_acm_mr_msg_queue_first != queue_msg &&
          sns_acm_mr_msg_queue_last != queue_msg )
      {
        queue_msg->prev->next = queue_msg->next;
        queue_msg->next->prev = queue_msg->prev;
      }

      if( sns_acm_mr_msg_queue_first == queue_msg )
      {
        sns_acm_mr_msg_queue_first = sns_acm_mr_msg_queue_first->next;
        if( NULL != sns_acm_mr_msg_queue_first )
        {
          sns_acm_mr_msg_queue_first->prev = NULL;
        }
      }

      if( sns_acm_mr_msg_queue_last == queue_msg )
      {
        sns_acm_mr_msg_queue_last = sns_acm_mr_msg_queue_last->prev;
        if( NULL != sns_acm_mr_msg_queue_last )
        {
          sns_acm_mr_msg_queue_last->next = NULL;
        }
      }

      queue_temp = queue_msg;
      queue_msg = queue_msg->next;

      sns_smr_msg_free( queue_temp->msg );
      SNS_OS_FREE( queue_temp );
      sem_post( &sns_acm_mr_queue_sem );
    }
    else
    {
      queue_msg = queue_msg->next;
    }
  }
  sns_os_mutex_post( sns_acm_mr_queue_mutex );

  return rv;
}

/**
 * Add a new message received from QCCI to the pending message queue.
 * Message will be processed by ACM thread in the order received.
 *
 * @param[i] msg Message allocated using sns_smr_msg_alloc
 */
static void
sns_acm_mr_queue_add( void *msg )
{
  uint8_t os_err = 0;
  sns_acm_mr_msg *queue_msg;
  struct timespec timestamp;

  /* We want to wait for the semaphore, but we also want an error
   * message if something goes wrong.  We don't care if we accidentally
   * have slightly more than SNS_ACM_MAX_MSG_QUEUE.
   */
  if( -1 != clock_gettime( CLOCK_REALTIME, &timestamp ) )
  {
    timestamp.tv_sec += 10;
    if( -1 == sem_timedwait( &sns_acm_mr_queue_sem, &timestamp ) &&
        ETIMEDOUT == errno )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM, "Semaphore timeout" );
    }
  }

  queue_msg = SNS_OS_MALLOC( SNS_MODULE_ACM, sizeof(sns_acm_mr_msg) );
  if( NULL == queue_msg )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM, "Alloc failure" );
    sns_smr_msg_free( msg );
    sem_post( &sns_acm_mr_queue_sem );
    return ;
  }

  queue_msg->msg = msg;
  queue_msg->prev = NULL;

  sns_os_mutex_pend( sns_acm_mr_queue_mutex, 0, &os_err );
  if( 0 != os_err ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "Unable to aquire mutex %i", os_err );
    SNS_OS_FREE( queue_msg );
    sns_smr_msg_free( msg );
    sem_post( &sns_acm_mr_queue_sem );
    return ;
  }

  queue_msg->next = sns_acm_mr_msg_queue_first;

  if( NULL != sns_acm_mr_msg_queue_first )
  {
    sns_acm_mr_msg_queue_first->prev = queue_msg;
  }
  sns_acm_mr_msg_queue_first = queue_msg;

  if( NULL == sns_acm_mr_msg_queue_last )
  {
    sns_acm_mr_msg_queue_last = sns_acm_mr_msg_queue_first;
  }
  sns_os_mutex_post( sns_acm_mr_queue_mutex );

  sns_os_sigs_post( sns_acm_flag_grp,
                    SNS_ACM_SMR_RX_FLAG,
                    OS_FLAG_SET,
                    &os_err );
}

/**
 * Process all pending messages on the queue.
 */
void
sns_acm_mr_handle()
{
  uint8_t os_err =  0;
  sns_acm_mr_msg *queue_msg;

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_ACM, "RX message from QCCI" );
  do
  {
    sns_os_mutex_pend( sns_acm_mr_queue_mutex, 0, &os_err );
    if( 0 != os_err ) {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "Unable to aquire mutex %i", os_err );
      return ;
    }

    queue_msg = sns_acm_mr_msg_queue_last;
    if( NULL != queue_msg && NULL != queue_msg->prev )
    {
      queue_msg->prev->next = NULL;
      sns_acm_mr_msg_queue_last = queue_msg->prev;
    }
    else
    {
      sns_acm_mr_msg_queue_last = NULL;
      sns_acm_mr_msg_queue_first = NULL;
    }

    sns_os_mutex_post( sns_acm_mr_queue_mutex );

    if( NULL != queue_msg )
    {
      sns_acm_handle_rx( queue_msg->msg );
      SNS_OS_FREE( queue_msg );
      sem_post( &sns_acm_mr_queue_sem );
    }
  } while( NULL != queue_msg );
}

/**
 * Calculates the current number of queue entries.
 *
 * @return The number of entries in the queue.
 */
int32_t
sns_acm_msg_q_size()
{
  uint8_t os_err = 0;
  sns_acm_mr_msg *queue_msg;
  uint32_t msg_cnt = 0;

  sns_os_mutex_pend( sns_acm_mr_queue_mutex, 0, &os_err );
  if( 0 != os_err ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "Unable to aquire mutex %i", os_err );
    return -1;
  }

  queue_msg = sns_acm_mr_msg_queue_first;
  while( NULL != queue_msg )
  {
    msg_cnt++;
    queue_msg = queue_msg->next;
  }

  sns_os_mutex_post( sns_acm_mr_queue_mutex );

  return msg_cnt;
}

/**
 * Callback function from QMI whenever a indication is received.
 *
 * @param[i] user_handle
 * @param[i] msg_id Message ID as found in the servcie header file
 * @param[i] ind_buf Buffer containing the raw, undecoded message.
 * @param[i] ind_buf_len Length of the data in ind_buf.  Not length of decoded msg.
 * @param[i] cb_data Data stored in qmi_client_init.
 */
void
sns_acm_qcci_ind_cb( qmi_client_type user_handle, unsigned int msg_id,
    void *ind_buf, unsigned int ind_buf_len, void *cb_data )
{
  sns_smr_header_s smr_header;
  sns_acm_ind_cb_data *ind_cb_data = (sns_acm_ind_cb_data *)cb_data;
  void *smr_ind_msg;
  uint32_t decoded_size;
  qmi_client_error_type qmi_err;

  qmi_err = qmi_idl_get_message_c_struct_len( ind_cb_data->service_obj,
      QMI_IDL_INDICATION, (uint16_t)msg_id, &decoded_size );
  if( QMI_NO_ERR != qmi_err )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "Error getting message length: %i",
                               qmi_err );
    return ;
  }

  smr_header.txn_id = 0;
  smr_header.ext_clnt_id = (uint8_t)(ind_cb_data->ext_clnt_id);
  smr_header.msg_type = SNS_SMR_MSG_TYPE_IND;
  smr_header.svc_num = (uint8_t)(ind_cb_data->svc_num);
  smr_header.msg_id = (uint16_t)msg_id;
  smr_header.body_len = (uint16_t)decoded_size;

  smr_ind_msg = sns_smr_msg_alloc( SNS_DBG_MOD_APPS_ACM, (uint16_t)decoded_size );
  if( NULL == smr_ind_msg )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "Error allocating message (%i)",
                               sns_acm_msg_q_size() );
    return ;
  }

  qmi_err = qmi_client_message_decode( user_handle, QMI_IDL_INDICATION,
      msg_id, ind_buf, ind_buf_len, (void*)smr_ind_msg, decoded_size );
  if( QMI_NO_ERR != qmi_err )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "Error decoding message: %i", qmi_err );
    sns_smr_msg_free( smr_ind_msg );
    return ;
  }

  sns_smr_set_hdr( &smr_header, smr_ind_msg );
  sns_acm_mr_queue_add( smr_ind_msg );
}

/**
 * Callback function for QMI response-type messages.
 *
 * @param[i] user_handle
 * @param[i] msg_id Message ID as found in the servcie header file
 * @param[i] buf Buffer containing the decoded messag
 * @param[i] len Length of the data in buf.
 * @param[i] resp_cb_data Data stored in qmi_client_send_msg_async.
 * @param[i] transp_err QMI errors that may have occurred.
 */
void
sns_acm_qcci_resp_cb( qmi_client_type user_handle, unsigned int msg_id, void *buf,
    unsigned int len, void *resp_cb_data, qmi_client_error_type transp_err )
{
  UNREFERENCED_PARAMETER(user_handle);
  sns_smr_header_s *smr_req_header = (sns_smr_header_s*)resp_cb_data, smr_resp_header;

  smr_resp_header.txn_id = smr_req_header->txn_id;
  smr_resp_header.ext_clnt_id = smr_req_header->ext_clnt_id;
  smr_resp_header.msg_type = SNS_SMR_MSG_TYPE_RESP;
  smr_resp_header.svc_num = smr_req_header->svc_num;
  smr_resp_header.msg_id = (uint16_t)msg_id;
  smr_resp_header.body_len = (uint16_t)len;

  if( QMI_NO_ERR != transp_err )
  {
    sns_common_resp_s_v01 *resp_buf = (sns_common_resp_s_v01*)buf;

    SNS_OS_MEMZERO( buf, len );
    resp_buf->sns_result_t = SNS_RESULT_FAILURE_V01;
    resp_buf->sns_err_t = SENSOR1_EUNKNOWN;

    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "Transport error %i", transp_err );
  }

  SNS_OS_FREE( smr_req_header );
  sns_smr_set_hdr( &smr_resp_header, buf );
  sns_acm_mr_queue_add( buf );
}

/**
 * Acquire QMI information for a single QMI/sensor1 service
 *
 * @param[i] svc_id Sensors service number
 *
 * @return NULL
 */
void* sns_acm_mr_client_init( void *svc_id )
{
  sns_err_code_e sns_err;
  uint32_t j,
           svc_num = *((uint32_t *)svc_id);
  int timeout_ms = (svc_num == 0) ? 10000 : 2000;
  qmi_idl_service_object_type service;

  service = sns_smr_get_svc_obj( svc_num );
  if( service == NULL )
  {
    SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_ACM, "Service %i is NULL", svc_num );
  }
  else
  {
    SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_ACM, "Initializing connection for svc %i", svc_num );

    sns_err = sns_smr_get_qmi_service_info( &service, timeout_ms, &service_info[ svc_num ] );
    if( SNS_SUCCESS == sns_err )
    {
      SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_ACM, "Retrieved info for service %i", svc_num );
      service_enabled[ svc_num ] = true;
    }
    else
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "Error getting info for service %i", svc_num );
      service_enabled[ svc_num ] = false;
    }

    for( j = 0; j < SNS_ACM_MAX_CLIENTS; j++ )
    {
      external_clients[ j ].cb_data[ svc_num ] = NULL;
    }
  }

  SNS_OS_FREE( svc_id );
  return NULL;
}

/**
 * Acquire QMI info for all sensor1 services.
 *
 * @param[i] sig_grp_ptr Unused.
 * @param[i] sig_flag Unused.
 *
 * @return SNS_SUCCESS in all cases.
 */
sns_err_code_e
sns_acm_mr_init( OS_FLAG_GRP *sig_grp_ptr, OS_FLAGS sig_flag )
{
  UNREFERENCED_PARAMETER( sig_grp_ptr );
  UNREFERENCED_PARAMETER( sig_flag );

  uint32_t i;
  uint8_t os_err = 0;
  pthread_t thread_id[SNS_ACM_MAX_SVC_ID];
  pthread_attr_t thread_attr;

  sns_acm_mr_msg_queue_first = NULL;
  sns_acm_mr_msg_queue_last = NULL;

  sns_acm_mr_queue_mutex = sns_os_mutex_create( SNS_MODULE_PRI_APPS_ACM_MUTEX, &os_err );
  if( 0 != os_err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_DBG_MOD_ACM, "Cannot create mutex %i", os_err );
    return SNS_ERR_FAILED;
  }

  if( -1 == sem_init( &sns_acm_mr_queue_sem, 0, SNS_ACM_MAX_MSG_QUEUE ) )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_DBG_MOD_ACM, "Cannot create semaphore %i", errno );
    return SNS_ERR_FAILED;
  }

  pthread_attr_init( &thread_attr );
  pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_JOINABLE );

  for( i = 0; i < SNS_ACM_MAX_SVC_ID; i++ )
  {
    uint32_t *svc_id = SNS_OS_MALLOC( SNS_MODULE_ACM, sizeof(uint32_t) );
    if( NULL == svc_id )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM, "Malloc Error" );
      return SNS_ERR_FAILED;
    }
    else
    {
      *svc_id = i;
      pthread_create( &thread_id[ i ], &thread_attr, sns_acm_mr_client_init, svc_id );
    }
  }
  pthread_attr_destroy( &thread_attr );

  for( i = 0; i < SNS_ACM_MAX_SVC_ID; i++ )
  {
    pthread_join( thread_id[ i ], NULL );
  }

  return SNS_SUCCESS;
}

/**
 * Returns QCCI client handle, and initializes connection to service
 * if necessary.
 *
 * @param svc_num[i] Service number; guaranteed to be valid ID.
 * @param ext_clnt_id[i] External client ID.
 * @param service[i] QMI Service object.
 *
 * @return Client handle or NULL if not available/error.
 */
qmi_client_type
sns_acm_mr_init_hndl( uint32_t svc_num, uint32_t ext_clnt_id,
    qmi_idl_service_object_type service )
{
  sns_acm_ind_cb_data *ind_cb_data;
  sns_acm_ext_client_handle *ext_clnt;
  qmi_client_error_type qmi_err;

  // Client ID already verified, don't need to do it again here
  ext_clnt = &external_clients[ ext_clnt_id ];
  if( NULL == ext_clnt->cb_data[ svc_num ] )
  {
    ind_cb_data = SNS_OS_MALLOC( SNS_MODULE_ACM, sizeof(sns_acm_ind_cb_data) );
    if( NULL == ind_cb_data )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM, "Error allocating callback data" );
    }
    else
    {
      ind_cb_data->svc_num = svc_num;
      ind_cb_data->ext_clnt_id = ext_clnt_id;
      ind_cb_data->service_obj = service;
      ext_clnt->cb_data[ svc_num ] = ind_cb_data;

      qmi_err = qmi_client_init( &service_info[ svc_num ], service,
                                sns_acm_qcci_ind_cb,
                                ind_cb_data, &os_params,
                                &ext_clnt->client_handles[ svc_num ] );
      SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_ACM,
                               "Initialized QMI service %i",
                               svc_num );

      if( QMI_NO_ERR != qmi_err )
      {
        SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_ACM,
                                   "Failure initializing QMI service %i: %i",
                                   svc_num, qmi_err );
        ext_clnt->cb_data[ svc_num ] = NULL;
        SNS_OS_FREE( ind_cb_data );
      }
    }
  }

  return ext_clnt->client_handles[ svc_num ];
}

/**
 * Deinitialize the ACM MR service
 *
 * @return SNS_SUCCESS in all cases.
 */
sns_err_code_e
sns_acm_mr_deinit()
{
  uint32_t i;
  uint8_t os_err;

  // Close all clients
  for( i = 0; i < SNS_ACM_MAX_CLIENTS; i++ )
  {
    sns_acm_mr_close(i);
  }

  // Remove all items from queue
  sns_os_mutex_pend( sns_acm_mr_queue_mutex, 0, &os_err );

  while( NULL != sns_acm_mr_msg_queue_first )
  {
    sns_acm_mr_msg *queue_msg = sns_acm_mr_msg_queue_first;

    sns_acm_mr_msg_queue_first = sns_acm_mr_msg_queue_first->next;

    sns_smr_msg_free( queue_msg->msg );
    SNS_OS_FREE( queue_msg );
  }

  sns_acm_mr_msg_queue_first = NULL;
  sns_acm_mr_msg_queue_last = NULL;

  sns_os_mutex_post( sns_acm_mr_queue_mutex );

  // Delete queue mutex
  sns_os_mutex_del( sns_acm_mr_queue_mutex, 0, &os_err );
  if( 0 != os_err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_MODULE_ACM, "can't delete mutex %i", os_err );
  }

  sem_destroy( &sns_acm_mr_queue_sem );

  return SNS_SUCCESS;
}

/**
 * Send a cancel request from a specific external client to
 * a certain sensors service.
 *
 * @param ext_clnt_id[i] External client that is closing
 * @param svc_num[i] Destination service number
 *
 * @return SNS_SUCCESS
 *         SNS_ERR_BAD_PARM Invalid svc_num/ext_clnt_id pair
 *         SNS_ERR_FAILED Error sending/receiving QMI mesg
 */
sns_err_code_e
sns_acm_mr_cancel( uint8_t ext_clnt_id, uint8_t svc_num )
{
  sns_err_code_e rv = SNS_SUCCESS;
  qmi_client_type qmi_clnt_hndl;
  sns_common_cancel_req_msg_v01 cancel_req;
  sns_common_cancel_resp_msg_v01 cancel_resp;
  qmi_client_error_type qmi_err;
  qmi_idl_service_object_type service;

  service = sns_smr_get_svc_obj( svc_num );
  qmi_clnt_hndl = sns_acm_mr_init_hndl( svc_num, ext_clnt_id, service );
  if( NULL == qmi_clnt_hndl )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "Unable to acquire QMI service handle %i",
                               svc_num );
    rv = SNS_ERR_BAD_PARM;
  }
  else
  {
    qmi_err = qmi_client_send_msg_sync( qmi_clnt_hndl, 0, /* Cancel msg ID is always 0 */
        &cancel_req,sizeof(sns_common_cancel_req_msg_v01), &cancel_resp,
        sizeof(sns_common_cancel_resp_msg_v01), 100 );
    rv = QMI_NO_ERR == qmi_err ? SNS_SUCCESS : SNS_ERR_FAILED;
  }

  return rv;
}

/**
 * Send a message to a sensor1 service.
 *
 * @param msg_ptr[i]  Message allocated using sns_smr_msg_alloc.
 *                    Header must be appropriately set.
 *                    Will be freed only upon success.
 * @return See sns_err_code_e.
 */
sns_err_code_e
sns_acm_mr_send( void *msg_ptr )
{
  qmi_client_type qmi_clnt_hndl;
  sns_smr_header_s *smr_header;
  qmi_idl_service_object_type service;
  uint32_t resp_msg_size;
  void *resp_msg = NULL;
  sns_err_code_e rv = SNS_SUCCESS;

  smr_header = SNS_OS_MALLOC( SNS_MODULE_ACM, sizeof(sns_smr_header_s) );
  if( NULL == smr_header )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM, "Error allocating header" );
    return SNS_ERR_NOMEM;
  }
  else
  {
    sns_smr_get_hdr( smr_header, msg_ptr );
    service = sns_smr_get_svc_obj( smr_header->svc_num );

    SNS_PRINTF_STRING_LOW_2( SNS_MODULE_ACM,
                            "Sending message to service %i with id %i",
                            smr_header->svc_num, smr_header->msg_id );

    if( smr_header->svc_num >= SNS_ACM_MAX_SVC_ID )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                "Invalid service number %i",
                                smr_header->svc_num );
      rv = SNS_ERR_INVALID_HNDL;
    }
    else if( !service_enabled[ smr_header->svc_num ] )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM, "Request made to disabled service" );
      rv = SNS_ERR_INVALID_HNDL;
    }
    else if( NULL == service )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                "Service %i has NULL object",
                                smr_header->svc_num );
      rv = SNS_ERR_INVALID_HNDL;
    }
    else
    {
      qmi_idl_get_message_c_struct_len( service, QMI_IDL_RESPONSE,
                                        smr_header->msg_id, &resp_msg_size );
      resp_msg = sns_smr_msg_alloc( SNS_DBG_MOD_APPS_ACM, resp_msg_size );
      if( NULL == resp_msg ) {
        SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM, "Error allocating response message" );
        rv = SNS_ERR_NOMEM;
      }
      else
      {
        qmi_clnt_hndl = sns_acm_mr_init_hndl( smr_header->svc_num, smr_header->ext_clnt_id, service );
        if( NULL == qmi_clnt_hndl )
        {
          SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                    "Unable to initialize QMI service %i",
                                    smr_header->svc_num );
          rv = SNS_ERR_FAILED;
        }
        else
        {
          qmi_txn_handle qmi_txn;
          qmi_client_error_type qmi_err = qmi_client_send_msg_async(
              qmi_clnt_hndl, smr_header->msg_id, msg_ptr, smr_header->body_len, resp_msg,
              resp_msg_size, sns_acm_qcci_resp_cb, (void*)smr_header, &qmi_txn );

          if( QMI_NO_ERR != qmi_err )
          {
            SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_ACM,
                                       "Failure sending msg: client %i; result %i",
                                       smr_header->svc_num, qmi_err );
            rv = SNS_ERR_FAILED;
          }
        }
      }
    }
  }

  if( SNS_SUCCESS != rv )
  {
    sns_smr_msg_free( resp_msg );
    SNS_OS_FREE( smr_header );
  }
  else
  {
    sns_smr_msg_free( msg_ptr );
  }

  return rv;
}
