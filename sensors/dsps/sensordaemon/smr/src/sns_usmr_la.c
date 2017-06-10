/*============================================================================
  @file sns_usmr_la.c

  @brief Contains the implementation of the usmr interface on non ADSP
  platforms.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/
#include "sns_usmr.h"
#include "sns_memmgr.h"
#include "sns_em.h"
#include "sns_debug_str.h"
#include "sns_common.h"
#include "sns_init.h"
#include "common_log.h"

/*===========================================================================
  TYPE DEFINITIONS
  ===========================================================================*/
struct smr_client_hndl_s
{
  qmi_idl_service_object_type service_obj;
  qmi_client_type             qmi_client;
  smr_client_ind_cb           cb_ptr;
  void                        *cb_data;
  qmi_client_os_params        os_params;
};

struct smr_service_hndl_s
{
  qmi_csi_service_handle service_provider;
  qmi_csi_os_params      os_params;
  smr_service_connect    connect_cb;
  smr_service_disconnect disconnect_cb;
  smr_process_req        process_req_cb;
  void                   *service_cookie;
};

typedef struct
{
  smr_client_init_cb cb_func;
  qmi_idl_service_object_type serviceObj;
  qmi_service_instance instanceID;
  qmi_client_type qmi_notifier_handle;
  sns_em_timer_obj_t timer;
  qmi_client_os_params os_params;
} sns_smr_service_check_cb_data;

typedef struct
{
  qmi_csi_service_handle  service_provider;
  qmi_csi_os_params       os_params;
  smr_service_connect     connect_cb;
  smr_service_disconnect  disconnect_cb;
  smr_process_req         process_req_cb;
  void                    *service_cookie;
} sns_smr_service_data;

/*===========================================================================
  STATIC VARIABLES
  ===========================================================================*/
/* Last return value received from a QMI function */
/* TODO: This field is not thread safe, and ****MUST**** be changed ASAP */
static int last_qmi_err;

/*===========================================================================
  STATIC FUNCTIONS
  ===========================================================================*/

/**
 * @brief
 * Timer callback for this module to check for the existence of
 * a service with a specific instance ID.
 */
static void sns_smr_service_check_timer_cb( void *args )
{
  sns_smr_service_check_cb_data *cb_data;
  qmi_service_info service_info;
  qmi_client_error_type qmi_err;

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR,
                           "sns_smr_service_check_timer_cb" );

  cb_data = args;
  qmi_client_release( cb_data->qmi_notifier_handle );

  /* This is a failsafe - during bringup to workaround any QMI issues */
  /* probe for the service */
  qmi_err = qmi_client_get_service_instance( cb_data->serviceObj,
      cb_data->instanceID, &service_info);

  if( QMI_NO_ERR != qmi_err )
  {
    SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR,
                             "Service not available @ timeout" );

    cb_data->cb_func( cb_data->serviceObj, cb_data->instanceID, true );
  }
  else
  {
    SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR,
                             "Service available @ timeout" );
    cb_data->cb_func( cb_data->serviceObj, cb_data->instanceID, false );
  }
  SNS_OS_FREE( cb_data );
}

/**
 * @brief
 * QMI notifier callback for this module to check for the
 * existence of a service with a specific instance ID.
 */
static void sns_smr_service_check_notifier_cb( qmi_client_type user_handle,
    qmi_idl_service_object_type service_obj,
    qmi_client_notify_event_type service_event, void *data )
{
  sns_smr_service_check_cb_data *cb_data;
  qmi_service_info service_info;
  qmi_client_error_type qmi_err;
  cb_data = (sns_smr_service_check_cb_data *)data;

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR,
                           "sns_smr_service_check_notifier_cb" );

  if( QMI_CLIENT_SERVICE_COUNT_INC == service_event )
  {
    qmi_err = qmi_client_get_service_instance( service_obj,
        cb_data->instanceID, &service_info );

    /* This is interesting - if QMI barfs now - there is nothing we can do
       Just wait for the timeout to expire and clean up then */
    if( QMI_NO_ERR == qmi_err )
    {
      sns_em_cancel_timer( cb_data->timer );
      cb_data->cb_func( cb_data->serviceObj, cb_data->instanceID, false );
      qmi_client_release_async( user_handle, NULL, NULL );
    }
    SNS_OS_FREE( cb_data );
  }
  /* else - number of service instances has gone down - we can't possibly
     have a new instance satisfying what we needed - ignore */
}

/**
 * @brief
 * Handler task for each service
 */
static void smr_service_task( void *args )
{
  qmi_csi_os_params os_params_in;
  fd_set fds;
  smr_service_hndl service_data = args;

  for( ;; )
  {
    fds = service_data->os_params.fds;
    select( service_data->os_params.max_fd + 1, &fds, NULL, NULL, NULL );
    os_params_in.fds = fds;
    qmi_csi_handle_event( service_data->service_provider, &os_params_in );
  }
}

/**
 * @brief
 * Common connect callback for all services
 */
static qmi_csi_cb_error smr_csi_connect_cb( qmi_client_handle qmi_handle,
    void *service_cookie, void **connection_handle )
{
  smr_service_hndl service_data = (smr_service_hndl)service_cookie;

  SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_SMR,
                           "QMI Handle %p", qmi_handle );

  if( NULL != service_data->connect_cb )
  {
    return service_data->connect_cb( (smr_qmi_client_handle)qmi_handle,
        service_data->service_cookie, connection_handle );
  }
  else
  {
    return QMI_NO_ERR;
  }
}

static void smr_csi_disconnect_cb( void *connection_handle,
    void *service_cookie )
{
  smr_service_hndl service_data = (smr_service_hndl)service_cookie;

  if( NULL != service_data->disconnect_cb )
  {
    service_data->disconnect_cb( connection_handle,
        service_data->service_cookie );
  }
}

static qmi_csi_cb_error smr_csi_process_req_cb( void *connection_handle,
    qmi_req_handle req_handle, unsigned int msg_id, void *req_c_struct,
    unsigned int req_c_struct_len, void *service_cookie )
{
  smr_service_hndl service_data = (smr_service_hndl)service_cookie;
  if( NULL != service_data->process_req_cb )
  {
    return service_data->process_req_cb( connection_handle, req_handle,
        msg_id, req_c_struct, req_c_struct_len, service_data->service_cookie );
  }
  else
  {
    return QMI_NO_ERR;
  }
}

static void smr_cci_ind_cb( qmi_client_type user_handle, unsigned int msg_id,
    void *ind_buf, unsigned int ind_buf_len, void *ind_cb_data )
{
  smr_client_hndl client_handle;
  qmi_client_error_type qmi_err;
  uint32_t decoded_size;
  // TODO: Must add mutex around this function ASAP
  static uint8_t smr_ind_msg[ 2048 ];

  if( NULL != ind_cb_data )
  {
    client_handle = (smr_client_hndl)(ind_cb_data);
    qmi_err = qmi_idl_get_message_c_struct_len( client_handle->service_obj,
        QMI_IDL_INDICATION, (uint16_t)msg_id, &decoded_size );
    if( QMI_NO_ERR != qmi_err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_SMR,
          "Error getting message length: %i", qmi_err );
      return ;
    }

    qmi_err = qmi_client_message_decode( user_handle, QMI_IDL_INDICATION,
        msg_id, ind_buf, ind_buf_len, smr_ind_msg, decoded_size );
    if( QMI_NO_ERR != qmi_err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_SMR,
          "Error decoding message: %i", qmi_err );
      return ;
    }

    if( NULL != client_handle->cb_ptr )
    {
      client_handle->cb_ptr( client_handle, msg_id, smr_ind_msg,
          decoded_size, client_handle->cb_data );
    }
    else
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_SMR,
          "No CallBack registered" );
    }
  }
}

/*===========================================================================
  PUBLIC FUNCTIONS - CLIENT
  ===========================================================================*/
/**
 * @brief
 * Helper blocking function to lookup and initialize a connection with a
 * service with a specific instance ID.
 */
smr_err smr_client_init( qmi_idl_service_object_type service_obj,
    qmi_service_instance instance_id, smr_client_ind_cb ind_cb,
    void *ind_cb_data, uint32_t timeout, smr_client_error_cb err_cb,
    void *err_cb_data, smr_client_hndl *client_handle )
{
  smr_client_hndl smr_client =
    SNS_OS_MALLOC( SNS_SMR, sizeof(struct smr_client_hndl_s) );

  SNS_PRINTF_STRING_LOW_3( SNS_DBG_MOD_APPS_SMR,
      "service_obj %p, cb %p, smr_client %p",
      service_obj, ind_cb, smr_client );
  if( NULL == smr_client )
  {
    return SMR_OUT_OF_MEMORY;
  }

  *client_handle = smr_client;
  smr_client->cb_data = ind_cb_data;
  smr_client->cb_ptr = ind_cb;
  smr_client->service_obj = service_obj;

  QMI_CCI_OS_SIGNAL_INIT( &smr_client->os_params, NULL );
  last_qmi_err = qmi_client_init_instance( service_obj,
      (qmi_service_instance)instance_id, smr_cci_ind_cb, smr_client,
      &smr_client->os_params, timeout, &smr_client->qmi_client );

  if( QMI_NO_ERR == last_qmi_err )
  {
    last_qmi_err = qmi_client_register_error_cb( smr_client->qmi_client,
        (qmi_client_error_cb) err_cb, err_cb_data );
  }

  return QMI_NO_ERR == last_qmi_err ? SMR_NO_ERR : SMR_XPORT_ERR;
}

/**
 * @brief
 * Releases the connection to a service and return immediately without blocking.
 * the provided callback is called when the connection is fully released and
 * it is safe for the caller to release any associated resources.
 */
smr_err smr_client_release( smr_client_hndl client_handle,
    smr_client_release_cb release_cb, void *release_cb_data )
{
  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR, "smr_client_release" );

  last_qmi_err = qmi_client_release_async( client_handle->qmi_client,
      (qmi_client_release_cb) release_cb, release_cb_data);
  return QMI_NO_ERR == last_qmi_err ? SMR_NO_ERR: SMR_XPORT_ERR;
}

/**
 * @brief
 * Sends an asynchronous message on the specified connection.
 * The callback function is expected to decode the message before sending it
 * through this function.
 */
smr_err smr_client_send_req( smr_client_hndl client_handle, unsigned int msg_id,
    void *req_c_struct, unsigned int req_c_struct_len, void *resp_c_struct,
    unsigned int resp_c_struct_len, smr_client_resp_cb resp_cb,
    void *resp_cb_data, smr_txn_handle *txn_handle )
{
  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR, "smr_client_send_req" );

  last_qmi_err = qmi_client_send_msg_async( client_handle->qmi_client, msg_id,
      req_c_struct, req_c_struct_len, resp_c_struct, resp_c_struct_len,
      (qmi_client_recv_msg_async_cb)resp_cb, resp_cb_data,
      (qmi_txn_handle*)txn_handle);

  return QMI_NO_ERR == last_qmi_err ? SMR_NO_ERR: SMR_XPORT_ERR;
}

/*===========================================================================
  PUBLIC FUNCTIONS - SERVICE
  ===========================================================================*/

/**
 * @brief
 * Register a service with the SMR infrastructure.
 */
smr_err smr_service_register( qmi_idl_service_object_type service_obj,
    qmi_service_instance instance_id, smr_service_connect service_connect,
    smr_service_disconnect service_disconnect, smr_process_req service_process_req,
    void *service_cookie, smr_service_hndl *smr_handle )
{
  qmi_csi_options qmi_options;
  uint8_t osa_err;
  smr_service_hndl smr_service = NULL;
  smr_err rv = SMR_NO_ERR;

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR, "smr_service_register" );

  smr_service = SNS_OS_MALLOC( SNS_SMR, sizeof(struct smr_service_hndl_s) );
  if( NULL == smr_service)
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_SMR,
        "No memory to process register" );
    rv = SMR_OUT_OF_MEMORY;
  }
  else
  {
    smr_service->service_cookie = service_cookie;
    smr_service->connect_cb = service_connect;
    smr_service->disconnect_cb = service_disconnect;
    smr_service->process_req_cb = service_process_req;

    *smr_handle = smr_service;

    QMI_CSI_OPTIONS_INIT(qmi_options);
    QMI_CSI_OPTIONS_SET_INSTANCE_ID(qmi_options, instance_id);
    last_qmi_err = qmi_csi_register_with_options( service_obj,
        smr_csi_connect_cb, smr_csi_disconnect_cb, smr_csi_process_req_cb,
        smr_service, &smr_service->os_params, &qmi_options,
        &smr_service->service_provider );

    if( QMI_NO_ERR != last_qmi_err )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_SMR,
        "QMI register error %i", last_qmi_err );
      SNS_OS_FREE( smr_service );
      rv = SMR_XPORT_ERR;
    }
    else
    {
      osa_err = sns_os_task_create( smr_service_task, smr_service,
          NULL, SNS_MODULE_PRI_APPS_SMR );
      if( 0 != osa_err )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_SMR,
          "Error creating task %i", osa_err );
        qmi_csi_unregister( smr_service->service_provider );
        SNS_OS_FREE( smr_service );
        rv = SMR_OS_ERR;
      }
    }
  }

  return rv;
}

/**
 * @brief
 * Unregisters a service.
 */
smr_err smr_service_unregister( smr_service_hndl service_provider )
{
  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR, "smr_service_unregister" );

  last_qmi_err = qmi_csi_unregister(
      (qmi_csi_service_handle)service_provider );
  return QMI_NO_ERR == last_qmi_err ? SMR_NO_ERR: SMR_XPORT_ERR;
}

/**
 * @brief
 * Sends a response to the client.  Response must always be sent, as the
 * client will likely not free it's pre-allocated response buffer until
 * it receives one.
 */
smr_err smr_service_send_resp(smr_req_handle req_handle, unsigned int msg_id,
    void *resp_c_struct, unsigned int resp_c_struct_len )
{
  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR, "smr_service_send_resp" );

  last_qmi_err = qmi_csi_send_resp( (qmi_req_handle)req_handle, msg_id,
      resp_c_struct, resp_c_struct_len );
  return QMI_NO_ERR == last_qmi_err ? SMR_NO_ERR: SMR_XPORT_ERR;
}

/**
 * @brief
 * Sends an indication to the client.
 */
smr_err smr_service_send_ind( smr_qmi_client_handle client_handle,
    unsigned int msg_id, void *ind_c_struct, unsigned int ind_c_struct_len)
{
  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR, "smr_service_send_ind" );

  last_qmi_err = qmi_csi_send_ind( (qmi_client_handle)client_handle, msg_id,
      ind_c_struct, ind_c_struct_len );
  return QMI_NO_ERR == last_qmi_err ? SMR_NO_ERR: SMR_XPORT_ERR;
}

/**
 * @brief
 * Sends a broadcast indication to all registered clients.
 *
 * @param[i] service_provider Handle used by SMR infrastructure to identify
 *                            the service that intends to send a broadcast message.
 * @param[i] msg_id Message ID for this particular message.
 * @param[i] ind_c_struct C data structure for this broadcast indication.
 * @param[i] ind_c_struct_len Size of the broadcast indication.
 *
 * @return SMR_NO_ERR if function is successful, error code otherwise.
 */
smr_err smr_service_broadcast_ind( smr_service_hndl *service_provider,
    unsigned int msg_id, void *ind_c_struct, unsigned int ind_c_struct_len )
{
  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR, "smr_service_broadcast_ind" );

  last_qmi_err = qmi_csi_send_broadcast_ind( (qmi_csi_service_handle)service_provider,
      msg_id, ind_c_struct, ind_c_struct_len );
  return QMI_NO_ERR == last_qmi_err ? SMR_NO_ERR: SMR_XPORT_ERR;
}

/**
 * @brief
 * Returns the qmi error
 *
 * @return Last QMI error, if the last call was successful,
 *         QMI_NO_ERROR
 */
int smr_get_qmi_err( void )
{
  return last_qmi_err;
}

/**
* @brief
* This callback function is called by the SMR infrastructure when a service
* becomes available, or if the client-specified timeout passes.
*
* @param[i] serviceObj QMI service object
* @param[i] instanceID Instance ID of the service found
* @param[i] timeoutExpired Whether the timeout expired
*/
smr_err smr_client_check( qmi_idl_service_object_type serviceObj,
    qmi_service_instance instanceID, unsigned int timeout,
    smr_client_init_cb cb_func )
{
  qmi_client_error_type qmi_err;
  qmi_service_info service_info;
  sns_smr_service_check_cb_data *smr_cb_data;
  sns_err_code_e sns_err;

  SNS_PRINTF_STRING_LOW_3( SNS_DBG_MOD_APPS_SMR,
      "timeout %d ms, service %p, instance %d",
      timeout / 1000, serviceObj, instanceID );

  qmi_err = qmi_client_get_service_instance(
      serviceObj, instanceID, &service_info );

  if( QMI_NO_ERR != qmi_err )
  {
    SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR,
        "Service not available now" );

    smr_cb_data = SNS_OS_MALLOC(
        SNS_SMR, sizeof(sns_smr_service_check_cb_data) );

    if( NULL == smr_cb_data )
    {
      return SMR_OUT_OF_MEMORY;
    }

    smr_cb_data->qmi_notifier_handle = NULL; // We break abstraction here - find a cleaner method
    smr_cb_data->timer = NULL;
    smr_cb_data->cb_func = cb_func;

    QMI_CCI_OS_SIGNAL_INIT( &smr_cb_data->os_params, NULL );
    qmi_err = qmi_client_notifier_init( serviceObj, &smr_cb_data->os_params,
        &(smr_cb_data->qmi_notifier_handle) );
    if( QMI_NO_ERR != qmi_err )
    {
      return SMR_XPORT_ERR;
    }

    /* Start the timer - we start the timer first, as qmi notification callback
       can fire in the callback itself */
    sns_err = sns_em_create_timer_obj( sns_smr_service_check_timer_cb,
        smr_cb_data, SNS_EM_TIMER_TYPE_ONESHOT, &smr_cb_data->timer );
    if( SNS_SUCCESS != sns_err )
    {
      SNS_OS_FREE( smr_cb_data );
      return SMR_OUT_OF_MEMORY;
    }

    sns_err = sns_em_register_timer( smr_cb_data->timer, timeout );
    if( SNS_SUCCESS != sns_err )
    {
      SNS_OS_FREE( smr_cb_data );
      return SMR_OUT_OF_MEMORY;
    }

    qmi_err = qmi_client_register_notify_cb( smr_cb_data->qmi_notifier_handle,
        sns_smr_service_check_notifier_cb, smr_cb_data );
    if( QMI_NO_ERR != qmi_err )
    {
      return SMR_XPORT_ERR;
    }
  }
  else
  {
    SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR,
        "Service available now" );
    cb_func( serviceObj, instanceID, false );
  }
  return SMR_NO_ERR;
}

void sns_smr_timer_cb( void *args )
{
  UNREFERENCED_PARAMETER(args);
  static int num = 0;

  num++;
  SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_SMR, "SMR_TIMER_CB %d", num );
}

/**
* @brief
* Cleanup any state before closing the service
*/
sns_err_code_e sns_smr_close( void )
{
  return SNS_SUCCESS;
}

/**
* @brief
* Entry point to SMR
*/
sns_err_code_e sns_smr_init( void )
{
  int smr_timer_data;
  sns_em_timer_obj_t smr_timer;
  sns_err_code_e sns_err;

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_SMR, "Initializing SMR" );

  sns_err = sns_em_create_timer_obj( sns_smr_timer_cb, &smr_timer_data,
      SNS_EM_TIMER_TYPE_PERIODIC, &smr_timer );
  if( SNS_SUCCESS != sns_err )
  {
    return SNS_ERR_FAILED;
  }

  sns_em_register_timer( smr_timer, 1000000 );
  sns_init_done();

  return SNS_SUCCESS;
}
