/******************************************************************************
  @file    qcril_qmi_sms.c
  @brief   qcril qmi - SMS

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI SMS.

  ---------------------------------------------------------------------------

  Copyright (c) 2010,2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <string.h>
#include <limits.h>
#include <cutils/properties.h>

#ifdef QMI_RIL_UTF
#include <errno.h>
#endif

#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "qcril_arb.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_sms.h"
#include "qcril_qmi_client.h"
#include "qcril_qmi_nas.h"
#include "qcril_qmi_sms_errors.h"
#include "qmi_errors.h"
#include "qcril_qmi_voice.h"
#include "qcril_cm_ss.h"
#include "qmi_ril_platform_dep.h"

/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define QCRIL_BLOCK_SMS_ON_1X                      "persist.radio.block_sms_on_1x"

#define QCRIL_QMI_WMS_LONG_SYNC_REQ_TIMEOUT        (3000)

#define SMS_REQUEST_CONCURRENCY_NUM (1)
#define SMS_REQUEST_PENDING_NUM (50)
#define SMS_TELEPHONY_MEMORY_EXCEEDED_ERROR (211)

#define QCRIL_MT_SMS_ACK_EXPRY_WINDOW       "persist.radio.mt_sms_ack"
#define QCRIL_SMS_LINK_TIMER                "persist.radio.sms_link_timer"
#define QCRIL_ACCEPT_ALL_BC_MSG             "all_bc_msg"
#define QCRIL_FORCE_ON_DC                   "persist.radio.force_on_dc"


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

// sms cache
static qcril_sms_struct_type *qcril_sms; ///< sms cache per instance
static qmi_ril_sms_svc_status_type qmi_ril_sms_svc_status;
static int feature_ims_3gpp_retry = 1; /// accept/reject ims sms retries on 3gpp
static int feature_ims_3gpp2_retry = 1; /// accept/reject ims sms retries on 3gpp2
static int qmi_ril_sms_mt_expiry_window_len;
static int qmi_ril_sms_mt_expiry_window_len_set;
static int qmi_ril_sms_link_timer;
static int feature_all_bc_msg_3gpp = 0; /*accept all BroadCast msg for only 3GPP technology.*/
static int feature_all_bc_msg_3gpp2 = 0; /*accept all BroadCast msg for only 3GPP2 technology.*/

/* "is_force_on_dc" is only applicable for 3GPP2. Force the message to be sent on the CDMA dedicated channel.
Values: 0x00 - Do not care about the channel on which the message is sent
        0x01 - Request to send the message over the dedicated channel.*/
static int is_force_on_dc = 0;

static pthread_mutex_t cache_mutex; /// Mutex for SMS cache data
static wms_service_ready_status_enum_v01 wms_service_state; ///< WMS service state


//============================================================================
// MACROS
//============================================================================
#define SMS_CACHE_LOCK  pthread_mutex_lock(&cache_mutex)
#define SMS_CACHE_UNLOCK  pthread_mutex_unlock(&cache_mutex)

/*===========================================================================

                                FUNCTIONS

===========================================================================*/
void qcril_sms_request_send_gw_sms(const qcril_request_params_type *const params_ptr,
                                   boolean link_control_enabled);

void qcril_sms_request_write_sms_to_uim(wms_raw_write_req_msg_v01 * write_request_msg,
                                        const qcril_request_params_type *const params_ptr,
                                        qcril_instance_id_e_type instance_id,
                                        wms_message_tag_type_enum_v01 tag,
                                        wms_message_mode_enum_v01 message_mode);

static void qcril_sms_mt_transaction_expired(void * param);
void qcril_sms_perform_transport_layer_info_initialization();
int qmi_ril_block_mo_sms_on_1x();

/// sets protected cach value wms_ready_status and logs its value
void set_wms_service_state(wms_service_ready_status_enum_v01);

/// check if wms ready indications are supported by the modem
int wms_ready_supported(void);

/// Queries modem for wms ready status
void query_wms_ready_status(void);

/// callback handler for async wms get ready status msg
void wms_get_ready_status_cb
(
    qmi_client_type user_handle,
    unsigned int    msg_id,
    void           *resp_c_struct,
    unsigned int    resp_c_struct_len,
    void           *resp_cb_data,
    qmi_client_error_type transp_err
);

/// Check that wms service is ready for <expected_tech> and return a boolean
/// @return 0 | 1 --> not ready, ready
int wms_ready(wms_service_ready_status_enum_v01 expected_state);

/// Sends a ril response w/RIL_E_GENERIC_FAILURE
void send_generic_failure(RIL_Token token, int event);

/// Convert sms service state to string for logging
const char* sms_state_str(wms_service_ready_status_enum_v01 service_state);

void qcril_qmi_sms_transfer_sim_ucs2_alpha_to_std_ucs2_alpha
(
    const wms_call_control_modified_info_type_v01 *sim_alpha,
    wms_call_control_modified_info_type_v01       *std_alpha
);

//============================================================================
// FUNCTION: qcril_qmi_sms_pre_init
//
// DESCRIPTION:
// pre initialization, to be called at bootup but not after SSR
//
// RETURN: None
//============================================================================
void qcril_qmi_sms_pre_init(void)
{
   wms_service_state = WMS_SERVICE_READY_STATUS_NONE_V01;
   QCRIL_LOG_INFO("wms service ready state: %s",
                                           sms_state_str(wms_service_state) );
   pthread_mutex_init(&cache_mutex, NULL);
}

//============================================================================
// FUNCTION: qcril_qmi_sms_destroy
//
// DESCRIPTION:
// clean up sms resources allocated at bootup (eg. qcril_qmi_sms_pre_init)
//
// RETURN: None
//============================================================================
void qcril_qmi_sms_destroy(void)
{
   pthread_mutex_destroy(&cache_mutex);
}

/*===========================================================================

  FUNCTION:  qcril_qmi_sms_init

===========================================================================*/
/*!
    @brief
    Initialize the SMS subsystem of the RIL.

    @return
    None.
*/
/*=========================================================================*/
qmi_client_error_type qcril_qmi_sms_init
(
  void
)
{
   uint8 i;
   qcril_sms_struct_type *i_ptr;
   qmi_client_error_type qmi_err = QMI_NO_ERR;

  char property_name[ 40 ];
  char args[ PROPERTY_VALUE_MAX ];
  int  temp_len;
  unsigned long ret_val;
  char *end_ptr;

  /*-----------------------------------------------------------------------*/
   QCRIL_LOG_FUNC_ENTRY();

   /* Allow cache */
   qcril_sms = (qcril_sms_struct_type *) qcril_arb_allocate_cache( QCRIL_ARB_CACHE_SMS );

  if( qcril_sms == NULL )
  {
    QCRIL_LOG_ERROR("Failed to allocate cache for SMS componet.");
    qmi_err = QMI_INTERNAL_ERR;
  }
  else
  {
     /* Initialize internal data */
     for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
     {
       i_ptr = &qcril_sms[ i ];

       pthread_mutex_init( &i_ptr->sms_ack_info_mutex, NULL );

       i_ptr->sms_ack_info.gw_ack_pending = FALSE;
       i_ptr->sms_ack_info.gw_ack_needed = FALSE;
       i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;
       i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
       i_ptr->sms_ack_info.cdma_ack_needed = FALSE;
       i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;

       i_ptr->mt_pending_ack_expry_tmr     = QMI_RIL_ZERO;
     }

     qmi_ril_set_sms_svc_status( QMI_RIL_SMS_SVC_INIT_PENDING );

     QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_MT_SMS_ACK_EXPRY_WINDOW );
     property_get( property_name, args, "" );

     qmi_ril_sms_mt_expiry_window_len_set = FALSE;
     temp_len = strlen( args );
     if ( temp_len > 0 )
     {
       ret_val = strtoul( args, &end_ptr, 0 );
       if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
       {
         QCRIL_LOG_ERROR( "Fail to convert MT_SMS_ACK_EXPRY_WINDOW %s", args );
       }
       else
       {
         qmi_ril_sms_mt_expiry_window_len     = ( uint8 ) ret_val;
         qmi_ril_sms_mt_expiry_window_len_set = TRUE;
       }
     }
     QCRIL_LOG_DEBUG( "MT_SMS_ACK_EXPRY_WINDOW set %d to %d", qmi_ril_sms_mt_expiry_window_len_set, qmi_ril_sms_mt_expiry_window_len );

     QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_SMS_LINK_TIMER );
     property_get( property_name, args, "" );

     qmi_ril_sms_link_timer = 4;
     temp_len = strlen( args );
     if ( temp_len > 0 )
     {
       ret_val = strtoul( args, &end_ptr, 0 );
       if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
       {
         QCRIL_LOG_ERROR( "Fail to convert SMS_LINK_TIMER %s", args );
       }
       else if( ( ret_val > 0 ) && ( ret_val <= 5 ) )
       {
         qmi_ril_sms_link_timer = ( uint8 ) ret_val;
       }
       else
       {
         qmi_ril_sms_link_timer = 5;
       }
     }
     QCRIL_LOG_DEBUG( "SMS_LINK_TIMER val set to %d", qmi_ril_sms_link_timer );

     // set properties
     feature_ims_3gpp_retry = qmi_ril_is_feature_supported(QMI_RIL_FEATURE_IMS_RETRY_3GPP);
     feature_ims_3gpp2_retry = qmi_ril_is_feature_supported(QMI_RIL_FEATURE_IMS_RETRY_3GPP2);

     memset(args,'\0', PROPERTY_VALUE_MAX);
     qcril_db_query_properties_table(QCRIL_ACCEPT_ALL_BC_MSG, args);

     if( !strcmp(args,"all") )
     {
        feature_all_bc_msg_3gpp = 1;
        feature_all_bc_msg_3gpp2 = 1;
     }
     else if( !strcmp(args,"gsm") )
     {
        feature_all_bc_msg_3gpp = 1;
        feature_all_bc_msg_3gpp2 = 0;
     }
     else if( !strcmp(args,"cdma") )
     {
        feature_all_bc_msg_3gpp = 0;
        feature_all_bc_msg_3gpp2 = 1;
     }
     else
     {
        feature_all_bc_msg_3gpp = 0;
        feature_all_bc_msg_3gpp2 = 0;
     }

     QCRIL_LOG_DEBUG( "All Broadcast Msg in 3GPP = %d, 3GPP2 = %d", feature_all_bc_msg_3gpp, feature_all_bc_msg_3gpp2 );

     memset(args,'\0', PROPERTY_VALUE_MAX);
     QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_FORCE_ON_DC );
     property_get( property_name, args, "" );

     if( !strcmp(args,"true") )
     {
        is_force_on_dc = 1;
     }
     else
     {
        is_force_on_dc = 0;
     }

     QCRIL_LOG_DEBUG( "IS FORCE ON DC = %d", is_force_on_dc );
  }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_err);
   return (qmi_err);
} /* qcril_qmi_sms_init() */

/*===========================================================================

  FUNCTION:  qcril_sms_perform_transport_layer_info_initialization

===========================================================================*/
/*!
    @brief
    Perform the initial configuration of transport layer info status

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_perform_transport_layer_info_initialization()
{
  int temp_len;
  unsigned long ret_val;
  char property_name[ 40 ];
  char args[ PROPERTY_VALUE_MAX ];
  char *end_ptr;
  wms_get_transport_layer_resp_msg_v01 wms_get_transport_layer_resp_msg;
  qmi_client_error_type qmi_err;
  RIL_Errno ril_req_res = RIL_E_SUCCESS;
  qcril_sms_struct_type *i_ptr;

  QCRIL_LOG_FUNC_ENTRY();

  i_ptr = &qcril_sms[ QCRIL_DEFAULT_INSTANCE_ID ];

  if( i_ptr )
  {
    pthread_mutex_init( &i_ptr->transport_layer_info_mutex, NULL );
    i_ptr->transport_layer_info.registered_ind_valid = FALSE;
    i_ptr->transport_layer_info.registered_ind = FALSE;
    i_ptr->transport_layer_info.block_sms_on_1x = FALSE;
    QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_BLOCK_SMS_ON_1X );
    property_get( property_name, args, "" );
    temp_len = strlen( args );
    if ( temp_len > 0 )
    {
      ret_val = strtoul( args, &end_ptr, 0 );
      if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
      {
        QCRIL_LOG_ERROR( "Fail to convert BLOCK_SMS_ON_1X %s", args );
      }
      else if ( ret_val > 1 )
      {
        QCRIL_LOG_ERROR( "Invalid saved BLOCK_SMS_ON_1X %ld, use default", ret_val );
      }
      else
      {
        i_ptr->transport_layer_info.block_sms_on_1x = ( uint8 ) ret_val;
      }
    }
    QCRIL_LOG_DEBUG( "BLOCK_SMS_ON_1X=%d", i_ptr->transport_layer_info.block_sms_on_1x);

    memset(&wms_get_transport_layer_resp_msg, 0, sizeof(wms_get_transport_layer_resp_msg));
    qmi_err = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_WMS ),
                                                       QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01,
                                                       NULL,
                                                       QMI_RIL_ZERO,
                                                       (void*) &wms_get_transport_layer_resp_msg,
                                                       sizeof( wms_get_transport_layer_resp_msg ),
                                                       QCRIL_QMI_WMS_LONG_SYNC_REQ_TIMEOUT );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &wms_get_transport_layer_resp_msg.resp );

    if ( RIL_E_SUCCESS == ril_req_res )
    {
      QCRIL_LOG_INFO( "transport layer reg info valid %d, value %d", wms_get_transport_layer_resp_msg.registered_ind_valid, wms_get_transport_layer_resp_msg.registered_ind);
      QCRIL_MUTEX_LOCK( &i_ptr->transport_layer_info_mutex, NULL );
      i_ptr->transport_layer_info.registered_ind_valid = wms_get_transport_layer_resp_msg.registered_ind_valid;
      i_ptr->transport_layer_info.registered_ind = wms_get_transport_layer_resp_msg.registered_ind;
      QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );
    }
    else
    {
      QCRIL_LOG_ERROR( "error while retrieving transport layer reg info");
    }
  }
  else
  {
    QCRIL_LOG_FATAL("sms_cache_ptr is Null pointer");
  }

  QCRIL_LOG_FUNC_RETURN();
} //qcril_sms_perform_transport_layer_info_initialization

/*===========================================================================

  FUNCTION:  qcril_sms_perform_initial_configuration

===========================================================================*/
/*!
    @brief
    Perform the initial configuration of QMI WMS

    @return
    None.
*/
/*=========================================================================*/
qmi_client_error_type qcril_sms_perform_initial_configuration
(
  void
)
{
  wms_set_routes_req_msg_v01 routes_request_msg;
  wms_set_routes_resp_msg_v01 routes_response_msg;
  uint8 i;
  qmi_client_error_type qmi_err;
  wms_set_primary_client_req_msg_v01 primary_request_msg;
  wms_set_primary_client_resp_msg_v01 primary_response_msg;
  wms_set_event_report_req_msg_v01 event_report_request_msg;
  wms_set_event_report_resp_msg_v01 event_report_response_msg;
  wms_indication_register_req_msg_v01 indication_register_req_msg;
  wms_indication_register_resp_msg_v01 indication_register_resp_msg;
  RIL_Errno ril_req_res = RIL_E_SUCCESS;

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_FUNC_ENTRY();
  // Set QCRIL as the primary client of QMI WMS
  memset(&primary_request_msg, 0, sizeof(wms_set_primary_client_req_msg_v01));
  memset(&primary_response_msg, 0, sizeof(wms_set_primary_client_resp_msg_v01));

  primary_request_msg.primary_client = TRUE;


  qmi_err = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_WMS ),
                                                     QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01,
                                                     (void*) &primary_request_msg,
                                                     sizeof( primary_request_msg ),
                                                     (void*) &primary_response_msg,
                                                     sizeof( primary_response_msg ),
                                                     QCRIL_QMI_WMS_LONG_SYNC_REQ_TIMEOUT );

  ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &primary_response_msg.resp );

  if ( RIL_E_SUCCESS != ril_req_res )
  {
    QCRIL_LOG_ERROR( "Failed to set QCRIL as the primary client, err %d", (int)ril_req_res);
  }

  // -----------------------------------------------------------------------

  // Configure the routes
  if (qmi_err == QMI_NO_ERR)
  {
    memset(&routes_request_msg, 0, sizeof(wms_set_routes_req_msg_v01));
    memset(&routes_response_msg, 0, sizeof(wms_set_routes_resp_msg_v01));

    /* Fill in the QMI request to set the routes */
    routes_request_msg.route_list_tuple_len = 6;

    /* Class 2 messages are:
          -- Store and notify
          -- Mem Store is SIM

       All other messages are:
          -- Transfer Only
          -- Mem Store is None
    */
    for (i = 0; i < 6; i++)
    {
      routes_request_msg.route_list_tuple[i].message_type = WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
      routes_request_msg.route_list_tuple[i].message_class = i;
      routes_request_msg.route_list_tuple[i].route_storage = WMS_STORAGE_TYPE_NONE_V01;
      routes_request_msg.route_list_tuple[i].receipt_action = WMS_TRANSFER_ONLY_V01;
    }

    routes_request_msg.route_list_tuple[2].message_type = WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
    routes_request_msg.route_list_tuple[2].message_class = WMS_MESSAGE_CLASS_2_V01;
    routes_request_msg.route_list_tuple[2].route_storage = WMS_STORAGE_TYPE_UIM_V01;
    routes_request_msg.route_list_tuple[2].receipt_action = WMS_STORE_AND_NOTIFY_V01;

    // All Status Reports should be transferred to the client
    routes_request_msg.transfer_ind_valid = TRUE;
    routes_request_msg.transfer_ind = WMS_TRANSFER_IND_CLIENT_V01;

    qmi_err = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_WMS ),
                                                       QMI_WMS_SET_ROUTES_REQ_V01,
                                                       (void*) &routes_request_msg,
                                                       sizeof( routes_request_msg ),
                                                       (void*) &routes_response_msg,
                                                       sizeof( routes_response_msg ),
                                                       QCRIL_QMI_WMS_LONG_SYNC_REQ_TIMEOUT );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &routes_response_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR( "Failed to configure routes, err %d", (int)ril_req_res);
    }
  }

  /*-----------------------------------------------------------------------*/

  // Enable QMI reporting of MT SMS messages
  if (qmi_err == QMI_NO_ERR)
  {
    memset(&event_report_request_msg, 0, sizeof(wms_set_event_report_req_msg_v01));
    memset(&event_report_response_msg, 0, sizeof(wms_set_event_report_resp_msg_v01));

    event_report_request_msg.report_mt_message_valid = TRUE;
    event_report_request_msg.report_mt_message = 0x01;

    event_report_request_msg.report_mwi_message_valid = TRUE;
    event_report_request_msg.report_mwi_message = 0x01;

    event_report_request_msg.report_call_control_info_valid = TRUE;
    event_report_request_msg.report_call_control_info = 0x01;

    qmi_err = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_WMS ),
                                                       QMI_WMS_SET_EVENT_REPORT_REQ_V01,
                                                       (void*) &event_report_request_msg,
                                                       sizeof( event_report_request_msg ),
                                                       (void*) &event_report_response_msg,
                                                       sizeof( event_report_response_msg ),
                                                       QCRIL_QMI_WMS_LONG_SYNC_REQ_TIMEOUT );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &event_report_response_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR( "Failed to enable QMI reporting of MT SMS messages, err %d", (int)ril_req_res);
    }
  }

  if (qmi_err == QMI_NO_ERR)
  {
    // Register for the desired indications
    memset(&indication_register_req_msg, 0, sizeof(wms_indication_register_req_msg_v01));
    memset(&indication_register_resp_msg, 0, sizeof(wms_indication_register_resp_msg_v01));

    indication_register_req_msg.reg_transport_layer_info_events_valid = TRUE;
    indication_register_req_msg.reg_transport_layer_info_events = TRUE;
    indication_register_req_msg.reg_transport_nw_reg_info_events_valid = TRUE;
    indication_register_req_msg.reg_transport_nw_reg_info_events = TRUE;
    indication_register_req_msg.reg_service_ready_events_valid = TRUE;
    indication_register_req_msg.reg_service_ready_events = 0x01; // Enable
    indication_register_req_msg.reg_transport_layer_mwi_info_events_valid = TRUE;
    indication_register_req_msg.reg_transport_layer_mwi_info_events = 0x01; // Enable

    qmi_err = qmi_client_send_msg_sync_with_shm( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_WMS ),
                                                       QMI_WMS_INDICATION_REGISTER_REQ_V01,
                                                       (void*) &indication_register_req_msg,
                                                       sizeof( indication_register_req_msg ),
                                                       (void*) &indication_register_resp_msg,
                                                       sizeof( indication_register_resp_msg ),
                                                       QCRIL_QMI_WMS_LONG_SYNC_REQ_TIMEOUT );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &indication_register_resp_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR( "Failed to register for QMI WMS indications, err %d", (int)ril_req_res);
    }
  }

  if (qmi_ril_is_feature_supported( QMI_RIL_FEATURE_FUSION_CSFB )
   || qmi_ril_is_feature_supported( QMI_RIL_FEATURE_SVLTE2 )
     ) {
     // wms ready indication is not supported for fusion targets yet...
     set_wms_service_state(WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01);
  }
  else
  {
     // update wms ready status to capture state, assumes ready when not
     // supported
     // Note: we do this after we register for the indication to get the
     // current state
     if (wms_ready_supported()) {
        query_wms_ready_status();
     }
     else {
        // wms ready indication is not supported, assume always ready
        set_wms_service_state(WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01);
     }
  }

  qcril_sms_perform_transport_layer_info_initialization(); //SMS transport NW reg status info initialization

  qmi_ril_set_sms_svc_status( QMI_RIL_SMS_SVC_FULLY_OPERATIONAL ); // regardless of qmi_err as we will not retry initialization
  QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_err);

  return(qmi_err);

} // qcril_sms_perform_initial_configuration


/*=========================================================================
  FUNCTION:  qcril_sms_perform_initial_configuration_evt_handler

===========================================================================*/
/*!
    @brief
    Function for handling the QCRIL_EVT_SMS_PERFORM_INITIAL_CONFIGURATION event

    @return
    None
*/
/*=========================================================================*/

void qcril_sms_perform_initial_configuration_evt_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_NOTUSED(params_ptr);
  QCRIL_NOTUSED(ret_ptr);
  (void)qcril_sms_perform_initial_configuration();
} /*qcril_sms_perform_initial_configuration_evt_handler*/

/*===========================================================================

                                RIL REQUEST HANDLERS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_sms_request_send_sms

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_send_sms
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_NOTUSED( ret_ptr );
  qcril_sms_request_send_gw_sms(params_ptr, FALSE);
} /* qcril_sms_request_send_sms */


/*===========================================================================

  FUNCTION:  qcril_sms_request_send_sms_expect_more

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SEND_SMS_EXPECT_MORE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_send_sms_expect_more
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_NOTUSED( ret_ptr );
  qcril_sms_request_send_gw_sms(params_ptr, TRUE);
} /* qcril_sms_request_send_sms_expect_more */


/*===========================================================================

  FUNCTION:  qcril_mo_sms_error_check

===========================================================================*/
/*!
    @brief
    Does error checking on mobile-originated SMS.  This checking should be
    done before sending an SMS, or writing an SMS to SIM.

    @return
    TRUE if SMS passes error-checking; FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_mo_sms_error_check
(
  const char *pdu
)
{
  boolean err_check_passed = TRUE;

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_FUNC_ENTRY();
  /* Check if the PDU is null */
  if ( pdu == NULL )
  {
    QCRIL_LOG_ERROR("PDU in SMS is NULL!");
    err_check_passed = FALSE;
  }

  /* Check if the length exceeds the maximum */
  else if ( ( strlen( pdu ) / 2 ) > WMS_MESSAGE_LENGTH_MAX_V01 )
  {
    QCRIL_LOG_ERROR("PDU in SMS exceeds maximum allowable length!");
    err_check_passed = FALSE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(err_check_passed);
  return (err_check_passed);

} /* qcril_mo_sms_error_check */


/*===========================================================================

  FUNCTION:  qcril_sms_fill_wms_payload

===========================================================================*/
/*!
    @brief
    Fill SMS payload per QMI requirement.

    @return
    TRUE if successful; FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_sms_fill_wms_payload
(
  boolean payload_in_cdma_format,
  RIL_CDMA_SMS_Message *cdma_sms_msg,
  const char *gw_smsc_address,
  const char *gw_pdu,
  wms_message_format_enum_v01  * format,
  uint32 * raw_message_len,
  uint8  * raw_message,
  boolean sms_on_ims,
  boolean is_mo_sms
)
{
  uint8 byte_array_pos = 0;
  boolean success = TRUE;

  QCRIL_LOG_INFO( "payload in cdma format valid %d", payload_in_cdma_format);
  /* 3GPP2 format */
  if ( payload_in_cdma_format )
  {
    if(cdma_sms_msg != NULL)
    {
       /* Convert the message to OTA format */
       if (  qcril_sms_convert_sms_ril_to_qmi(cdma_sms_msg,
                                           raw_message,
                                           WMS_MESSAGE_LENGTH_MAX_V01,
                                           (uint16 *) raw_message_len,
                                           sms_on_ims,
                                           is_mo_sms) != TRUE)
       {
          QCRIL_LOG_ERROR( "Conversion from RIL format to QMI format failed" );
          success = FALSE;
       }
       else
       {
          *format = WMS_MESSAGE_FORMAT_CDMA_V01;
       }
    }
    else
    {
      QCRIL_LOG_ERROR("cdma_sms_msg is NULL!");
      success = FALSE;
    }
  }
  /* 3GPP format */
  else
  {
    if(gw_pdu != NULL)
    {
      *format = WMS_MESSAGE_FORMAT_GW_PP_V01;

       if ( gw_smsc_address == NULL )
       {
          /* There is no SMSC address.  The first byte of the raw data is the SMSC length,
           which is 0. */
          raw_message[0] = 0;

          byte_array_pos = 1;
       }
       else
       {
          /* Convert the SMSC address from ASCII Hex to byte*/
          qcril_sms_hex_to_byte( gw_smsc_address,
                             &raw_message[0],
                             strlen(gw_smsc_address) );

          /* The input is in ASCII hex format, and it is being packed into a byte array.
           Two ASCII characters fit in each byte, so divide the length by two */
          byte_array_pos = strlen(gw_smsc_address) / 2;
       }

      /* Translate the PDU from ASCII hex to bytes */
      qcril_sms_hex_to_byte( gw_pdu,
                           &raw_message[byte_array_pos],
                           strlen( gw_pdu ) );

      *raw_message_len = byte_array_pos + strlen(gw_pdu) / 2;
    }
    else
    {
      QCRIL_LOG_ERROR("gw_pdu is NULL!");
      success = FALSE;
    }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(success);
  return (success);

} /* qcril_sms_fill_wms_payload */


/*===========================================================================

  FUNCTION:  qcril_sms_request_send_gw_sms

===========================================================================*/
/*!
    @brief
    Sends a GW SMS message

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_send_gw_sms
(
  const qcril_request_params_type *const params_ptr,
  boolean link_control_enabled
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  const char *smsc_address;
  const char *gw_pdu;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_raw_send_req_msg_v01 request_msg;
  wms_raw_send_resp_msg_v01* response_msg;
  nas_sms_status_enum_type_v01 lte_sms_status;
  uint8_t lte_sms_status_valid;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    qcril_qmi_nas_fetch_lte_sms_status( &lte_sms_status_valid, &lte_sms_status);
    if (!wms_ready(WMS_SERVICE_READY_STATUS_3GPP_V01) &&
        !(lte_sms_status_valid && (NAS_SMS_STATUS_3GPP_V01 == lte_sms_status))
       ) {
       send_generic_failure(params_ptr->t, params_ptr->event_id);
       break;
    }

    instance_id = QCRIL_DEFAULT_INSTANCE_ID;

    if( params_ptr->datalen == 0 || params_ptr->data == NULL)
    {
      QCRIL_LOG_ERROR("No data available to Send a GW SMS message.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    i_ptr = &qcril_sms[ instance_id ];

    /*-----------------------------------------------------------------------*/

    if ( !qcril_mo_sms_error_check( ( (const char **) params_ptr->data)[ 1 ] ) )
    {
      QCRIL_LOG_ERROR("qcril_mo_sms_error_check failed for MO GW SMS message.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /*-----------------------------------------------------------------------*/

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for MO GW SMS message.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the message */
    smsc_address = ( (const char **) params_ptr->data )[ 0 ];
    gw_pdu = ( (const char **) params_ptr->data )[ 1 ];

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_raw_send_req_msg_v01));

    if (!qcril_sms_fill_wms_payload(
                                   FALSE,
                                   NULL,
                                   smsc_address,
                                   gw_pdu,
                                   &request_msg.raw_message_data.format,
                                   (uint32 *)&request_msg.raw_message_data.raw_message_len,
                                   &request_msg.raw_message_data.raw_message[0],
                                   FALSE,
                                   TRUE ))
    {
      QCRIL_LOG_ERROR("Filling SMS payload per QMI requirement is failed for GW SMS message.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (link_control_enabled)
    {
      request_msg.link_timer_valid = TRUE;
      request_msg.link_timer = qmi_ril_sms_link_timer;
    }

    request_msg.sms_on_ims_valid = TRUE;
    request_msg.sms_on_ims = FALSE;



    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if(response_msg == NULL)
    {
      QCRIL_LOG_ERROR("Memory allocation failed to get response message, response_msg.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_WMS,
                                        QMI_WMS_RAW_SEND_REQ_V01,
                                        &request_msg,
                                        sizeof(wms_raw_send_req_msg_v01),
                                        response_msg,
                                        sizeof(wms_raw_send_resp_msg_v01),
                                        (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed in getting the response message from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_request_send_gw_sms */


/*===========================================================================

  FUNCTION:  qcril_mo_sms_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_SEND_SMS, RIL_REQUEST_SEND_SMS_EXPECT_MORE,
    RIL_REQUEST_CDMA_SEND_SMS, and RIL_REQUEST_IMS_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mo_sms_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_instance_id_e_type instance_id;
  RIL_SMS_Response send_msg_response;
  qcril_request_resp_params_type resp;
  wms_raw_send_resp_msg_v01 *raw_send_resp;
  boolean retry_status = FALSE;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;
  uint8 coding_scheme;
  wms_call_control_modified_info_type_v01 wms_call_control_alpha_info;
  char buf_str[WMS_ALPHA_ID_LENGTH_MAX_V01 + 2];
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  /*-----------------------------------------------------------------------*/

  raw_send_resp = (wms_raw_send_resp_msg_v01*)params_ptr->data;

  send_msg_response.messageRef = raw_send_resp->message_id;
  send_msg_response.ackPDU = NULL;
  send_msg_response.errorCode = -1;

  if (raw_send_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP received: SUCCESS");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    resp.resp_pkt = &send_msg_response;
    resp.resp_len = sizeof( send_msg_response );
    qcril_send_request_response( &resp );
  }
  else if (raw_send_resp->resp.result == QMI_RESULT_FAILURE_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP received with error %s",
                    qmisvc_errstr(raw_send_resp->resp.error) );

    /* Fill in the error codes*/
    if (raw_send_resp->resp.error == QMI_ERR_CAUSE_CODE_V01)
    {
      if (raw_send_resp->gw_cause_info_valid)
      {
        send_msg_response.errorCode = raw_send_resp->gw_cause_info.rp_cause;
        QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP cause code %s",
                        gw_cause_code_str(send_msg_response.errorCode) );
      }
      else if (raw_send_resp->cause_code_valid)
      {
        send_msg_response.errorCode = raw_send_resp->cause_code;
        QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP cause code %s",
                        cause_code_str(send_msg_response.errorCode) );
      }
      else
      {
         QCRIL_LOG_INFO("QMI_WMS_RAW_SEND_RESP missing cause code!");
      }

    }

    /* Determine whether the message can be retried or not */
    if (raw_send_resp->resp.error == QMI_ERR_CAUSE_CODE_V01)
    {
      if ((raw_send_resp->gw_cause_info_valid) &&
          (raw_send_resp->gw_cause_info.rp_cause == 41))
      {
        retry_status = TRUE;
      }
      else if ((raw_send_resp->error_class_valid) &&
               (raw_send_resp->error_class == 0))
      {
        retry_status = TRUE;
      }
      else if ((raw_send_resp->gw_cause_info_valid) &&
          (raw_send_resp->gw_cause_info.rp_cause == WMS_RP_CAUSE_SMS_TRANSFER_REJECTED_V01) &&
          (raw_send_resp->gw_cause_info.tp_cause == WMS_TP_CAUSE_SM_REJECTED_OR_DUPLICATE_V01))
      {
        /* This case will happen when first MO SMS has been delivered to network,
        but WMS didn't got response from network during certain amount of time,
        then WMS will send Temporary Failue, but in actual Network is still trying
        to deliver and it got delivered. During RETRY, if RIL gets rp_cause=21 and
        tp_cause= -59 that means SMS Rejected-Duplicate SMS that means same SMS has
        already been received by network. So, RIL should send SUCCESS to Telephony.*/

        ril_err = RIL_E_SUCCESS;
      }
    }
    else if (raw_send_resp->resp.error == QMI_ERR_MESSAGE_DELIVERY_FAILURE_V01)
    {
      if (raw_send_resp->message_delivery_failure_type_valid &&
          (raw_send_resp->message_delivery_failure_type == WMS_MESSAGE_DELIVERY_FAILURE_TEMPORARY_V01))
      {
        retry_status = TRUE;
      }
    }
    else if ((raw_send_resp->resp.error == QMI_ERR_DEVICE_NOT_READY_V01) ||
             (raw_send_resp->resp.error == QMI_ERR_NETWORK_NOT_READY_V01) ||
             (raw_send_resp->resp.error == QMI_ERR_NO_MEMORY_V01))
    {
      retry_status = TRUE;
    }

    if (retry_status == TRUE)
    {
      ril_err = RIL_E_SMS_SEND_FAIL_RETRY;
    }
    else if (raw_send_resp->resp.error == QMI_ERR_MSG_BLOCKED_V01)
    {
      ril_err = RIL_E_FDN_CHECK_FAILURE;
    }

    qcril_default_request_resp_params( instance_id,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       ril_err,
                                       &resp );

    resp.resp_pkt = &send_msg_response;
    resp.resp_len = sizeof( send_msg_response );
    qcril_send_request_response( &resp );
  }

  if( ( TRUE == raw_send_resp->call_control_modified_info_valid ) &&
      ( raw_send_resp->call_control_modified_info.alpha_id_len > 0 ) )
  {
    memset(buf_str, 0 , WMS_ALPHA_ID_LENGTH_MAX_V01 + 2);
    coding_scheme = raw_send_resp->call_control_modified_info.alpha_id[0];
    if( coding_scheme == 0x80 || coding_scheme == 0x81 || coding_scheme == 0x82 ) //UCS2
    {
        QCRIL_LOG_INFO("Coding scheme is %x, ucs2 data",coding_scheme);
        qcril_qmi_sms_transfer_sim_ucs2_alpha_to_std_ucs2_alpha ( &raw_send_resp->call_control_modified_info, &wms_call_control_alpha_info);
        qcril_cm_ss_convert_ussd_string_to_utf8(  QCRIL_QMI_VOICE_USSD_DCS_UCS2,
                                                    wms_call_control_alpha_info.alpha_id_len,
                                                    wms_call_control_alpha_info.alpha_id,
                                                    buf_str );
    }
    else //gsm8
    {
        QCRIL_LOG_INFO("Coding scheme is %x, gsm8 data",coding_scheme);
        if( raw_send_resp->call_control_modified_info.alpha_id < WMS_ALPHA_ID_LENGTH_MAX_V01 )
        {
            qcril_cm_ss_convert_gsm8bit_alpha_string_to_utf8( (char*) raw_send_resp->call_control_modified_info.alpha_id,
                                                                raw_send_resp->call_control_modified_info.alpha_id_len,
                                                                buf_str );
        }
    }

    if ( *buf_str )
    {
        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_STK_CC_ALPHA_NOTIFY, &unsol_resp );
        unsol_resp.resp_pkt    = (void*)buf_str;
        unsol_resp.resp_len    = sizeof( buf_str );

        qcril_send_unsol_response( &unsol_resp );
    }
  }

   QCRIL_LOG_FUNC_RETURN();
} /* qcril_mo_sms_cb */

void qcril_qmi_sms_transfer_sim_ucs2_alpha_to_std_ucs2_alpha(const wms_call_control_modified_info_type_v01 *sim_alpha, wms_call_control_modified_info_type_v01 *std_alpha)
{
  uint8 idx;
  uint8 coding_scheme;
  uint8 num_of_char;
  uint16 base_val;
  uint16 ucs2_val;

  QCRIL_LOG_FUNC_ENTRY();

  memset(std_alpha, 0, sizeof(*std_alpha));

  do
  {
    if ( sim_alpha->alpha_id_len > 0)
    {
      coding_scheme = sim_alpha->alpha_id[0];
    }
    else
    {
      QCRIL_LOG_ERROR("alpha_id_len is 0");
      break;
    }
    QCRIL_LOG_INFO("coding scheme %x", (int)coding_scheme);

    idx = 1;
    switch (coding_scheme)
    {
      case 0x80:
        while ( (uint32)(idx+1) < sim_alpha->alpha_id_len )
        {
          std_alpha->alpha_id[idx-1] = sim_alpha->alpha_id[idx+1];
          std_alpha->alpha_id[idx] = sim_alpha->alpha_id[idx];
          idx += 2;
        }

        if ( (uint32)(idx+1) == sim_alpha->alpha_id_len && 0 != sim_alpha->alpha_id[idx] )
        {
          QCRIL_LOG_ERROR("an unexpected extra non-zero byte in source alpha buffer");
        }

        std_alpha->alpha_id_len = idx-1;
        break;

      case 0x81:
        if ( sim_alpha->alpha_id_len < 3 )
        {
          QCRIL_LOG_ERROR("sim_alpha->alpha_id_len (%d) less than 3", sim_alpha->alpha_id_len);
        }
        else
        {
          num_of_char = sim_alpha->alpha_id[1];
          base_val = sim_alpha->alpha_id[2];
          base_val <<= 7;
          idx = 3;

          if ( idx + num_of_char > sim_alpha->alpha_id_len )
          {
            QCRIL_LOG_DEBUG("num_of_char > sim_alpha->alpha_id_len - 3");
            num_of_char = sim_alpha->alpha_id_len - idx;
          }

          if (num_of_char * 2 > WMS_ALPHA_ID_LENGTH_MAX_V01)
          {
            QCRIL_LOG_DEBUG("num_of_char * 2 > WMS_ALPHA_ID_LENGTH_MAX_V01");
            num_of_char = WMS_ALPHA_ID_LENGTH_MAX_V01 / 2;
          }

          int i;
          for ( i = 0; i< num_of_char; i++, idx++ )
          {
            ucs2_val = sim_alpha->alpha_id[idx];

            if ( ucs2_val >= 0x80 )
            {
              ucs2_val &= 0x7F;
              ucs2_val |= base_val;
            }

            std_alpha->alpha_id[2*i]   = (uint8) (ucs2_val);
            std_alpha->alpha_id[2*i+1] = (uint8) (ucs2_val >> 8);
          }

          std_alpha->alpha_id_len = num_of_char * 2;
        }

        break;

      case 0x82:
        if ( sim_alpha->alpha_id_len < 4 )
        {
          QCRIL_LOG_DEBUG("sim_alpha->alpha_id_len (%d) less than 4", sim_alpha->alpha_id_len);
        }
        else
        {
          num_of_char = sim_alpha->alpha_id[1];
          base_val = sim_alpha->alpha_id[2];
          base_val <<= 8;
          base_val += sim_alpha->alpha_id[3];
          idx = 4;

          if ( idx + num_of_char > sim_alpha->alpha_id_len )
          {
            QCRIL_LOG_DEBUG("num_of_char > sim_alpha->alpha_id_len - 4");
            num_of_char = sim_alpha->alpha_id_len - idx;
          }

          if (num_of_char * 2 > WMS_ALPHA_ID_LENGTH_MAX_V01)
          {
            QCRIL_LOG_DEBUG("num_of_char * 2 > WMS_ALPHA_ID_LENGTH_MAX_V01");
            num_of_char = WMS_ALPHA_ID_LENGTH_MAX_V01 / 2;
          }

          int i;
          for ( i = 0; i< num_of_char; i++, idx++ )
          {
            ucs2_val = sim_alpha->alpha_id[idx];

            if ( ucs2_val >= 0x80 )
            {
              ucs2_val &= 0x7F;
              ucs2_val += base_val;
            }

            std_alpha->alpha_id[2*i]   = (uint8) (ucs2_val);
            std_alpha->alpha_id[2*i+1] = (uint8) (ucs2_val >> 8);
          }

          std_alpha->alpha_id_len = num_of_char * 2;
        }

        break;

      default:
        QCRIL_LOG_ERROR("unknown SIM coding scheme");
    }

  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_sms_request_sms_acknowledge

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SMS_ACKNOWLEDGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_sms_acknowledge
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  int tp_cause;
  uint32 user_data;
  char details[ 80 ];
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_send_ack_req_msg_v01 request_msg;
  wms_send_ack_resp_msg_v01* response_msg;

  int need_reset_ack_expry_tmr = FALSE;

  /*-----------------------------------------------------------------------*/


  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for SMS acknowledgement.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (!i_ptr->sms_ack_info.gw_ack_pending)
    {
      QCRIL_LOG_ERROR("Unable to send ack; no messages to be acked.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (!i_ptr->sms_ack_info.gw_ack_needed)
    {
      QCRIL_LOG_INFO("Dropping ack since no ack is needed.");

      i_ptr->sms_ack_info.gw_ack_pending = FALSE;
      i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;

      need_reset_ack_expry_tmr = TRUE;

      qcril_default_request_resp_params( instance_id, params_ptr->t,
                                  params_ptr->event_id, RIL_E_SUCCESS, &resp );
      qcril_send_request_response( &resp );
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for SMS acknowledgement.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_send_ack_req_msg_v01));
    request_msg.ack_information.transaction_id = i_ptr->sms_ack_info.gw_transaction_id;
    request_msg.ack_information.message_protocol = WMS_MESSAGE_PROTOCOL_WCDMA_V01;

    request_msg.ack_information.success = ((int *) params_ptr->data)[0];

    if (!request_msg.ack_information.success)
    {
      request_msg.wms_3gpp_failure_information_valid = TRUE;

      tp_cause = ( (int *) params_ptr->data)[ 1 ];

      request_msg.wms_3gpp_failure_information.tp_cause = tp_cause;

      /* Initialize the RP cause to unspecified */
      request_msg.wms_3gpp_failure_information.rp_cause = WMS_RP_CAUSE_PROTOCOL_ERROR_ACK_V01;

      /* If the TP cause is memory full, set the RP cause to memory full also */
      if ( tp_cause == SMS_TELEPHONY_MEMORY_EXCEEDED_ERROR )
      {
        request_msg.wms_3gpp_failure_information.rp_cause = WMS_RP_CAUSE_MEMORY_CAP_EXCEEDED_ACK_V01;
      }
    }

    if (i_ptr->sms_ack_info.gw_send_ack_on_ims)
    {
      request_msg.sms_on_ims_valid = TRUE;
      request_msg.sms_on_ims = TRUE;
    }
    else
    {
      request_msg.sms_on_ims_valid = TRUE;
      request_msg.sms_on_ims = FALSE;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_INFO("Failed to allocate memory for response message for acknowledgement request.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_WMS,
                                        QMI_WMS_SEND_ACK_REQ_V01,
                                        &request_msg,
                                        sizeof(wms_send_ack_req_msg_v01),
                                        response_msg,
                                        sizeof(wms_send_ack_resp_msg_v01),
                                        (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response message from qmi for acknowledgement request.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

    /* Reset the ack pending flag.  If the ack is not received by the network,
       the network will resend the SMS, and RIL will have another chance
       to ack it. */
    i_ptr->sms_ack_info.gw_ack_pending = FALSE;
    i_ptr->sms_ack_info.gw_ack_needed = FALSE;
    i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;

    need_reset_ack_expry_tmr = TRUE;

  } while(0);

  if ( need_reset_ack_expry_tmr )
  {
      if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
      {
         QCRIL_LOG_INFO( "resetting MT SMS ack tmr per ack reception" );
         qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
         qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
      }
  }

  QCRIL_SNPRINTF( details, sizeof( details ),
                  "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_sms_acknowledge */


/*===========================================================================

  FUNCTION:  qcril_ack_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_SMS_ACKNOWLEDGE and
    RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_ack_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_request_resp_params_type resp;
  qcril_instance_id_e_type instance_id;
  wms_send_ack_resp_msg_v01 * ack_resp;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  /*-----------------------------------------------------------------------*/

  ack_resp = (wms_send_ack_resp_msg_v01 *) params_ptr->data;

  if(ack_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_SEND_ACK_RESP received: SUCCESS");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("QMI_WMS_SEND_ACK_RESP received with error %d",ack_resp->resp.error);
    qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_ack_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_write_sms_to_sim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_WRITE_SMS_TO_SIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_write_sms_to_sim
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  RIL_SMS_WriteArgs *SMS_Write_info;
  qcril_request_resp_params_type resp;
  wms_raw_write_req_msg_v01 write_request_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  SMS_Write_info = (RIL_SMS_WriteArgs *) params_ptr->data;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to Write the SMS in SIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

  memset(&write_request_msg, 0, sizeof(wms_raw_write_req_msg_v01));

  if ( !qcril_mo_sms_error_check( SMS_Write_info->pdu ) )
  {
    QCRIL_LOG_ERROR("qcril_mo_sms_error_check failed for SMS, which is going to written in SIM.");
    send_generic_failure(params_ptr->t, params_ptr->event_id);
    break;
  }

  /* Invalid status */
  if ( SMS_Write_info->status  > 3 )
  {
    QCRIL_LOG_ERROR("Invalid SMS status, which is going to written in SIM.");
    send_generic_failure(params_ptr->t, params_ptr->event_id);
    break;
  }

  /* Fill in the QMI request to write the message to the SIM */
  write_request_msg.raw_message_write_data.storage_type = WMS_STORAGE_TYPE_UIM_V01;

  if (!qcril_sms_fill_wms_payload( FALSE,
                                   NULL,
                                   SMS_Write_info->smsc,
                                   SMS_Write_info->pdu,
                                   &write_request_msg.raw_message_write_data.format,
                                   (uint32 *)&write_request_msg.raw_message_write_data.raw_message_len,
                                   &write_request_msg.raw_message_write_data.raw_message[0],
                                   FALSE,
                                   qcril_sms_is_tag_mo(SMS_Write_info->status)))
  {
    QCRIL_LOG_ERROR("Filling SMS payload per QMI requirement is failed for SMS, which is going to written in SIM.");
    send_generic_failure(params_ptr->t, params_ptr->event_id);
    break;
  }

  /* Write the message to the SIM */
  qcril_sms_request_write_sms_to_uim(&write_request_msg,
                                     params_ptr,
                                     instance_id,
                                     qcril_sms_map_ril_tag_to_qmi_tag(SMS_Write_info->status),
                                     WMS_MESSAGE_MODE_GW_V01);
  }while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_write_sms_to_sim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_write_sms_to_uim

===========================================================================*/
/*!
    @brief
    Writes an SMS message to UIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_write_sms_to_uim
(
  wms_raw_write_req_msg_v01 * write_request_msg,
  const qcril_request_params_type *const params_ptr,
  qcril_instance_id_e_type instance_id,
  wms_message_tag_type_enum_v01 tag,
  wms_message_mode_enum_v01 message_mode
)
{
  qcril_request_resp_params_type resp;
  wms_raw_write_resp_msg_v01 write_response_msg;
  int index_on_sim;
  RIL_Errno ril_req_res = RIL_E_SUCCESS;
  qmi_client_error_type qmi_err;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED(message_mode);
  if ( write_request_msg != NULL )
  {
    // Check WMS_SERVICE_READY
    //========================================
    // For all storage types (NV and UIM), so just check
    //   WMS_MESSAGE_FORMAT_CDMA_V01 (3GPP2)
    //   WMS_MESSAGE_FORMAT_GW_PP_V01 (3GPP)
    // Note: WMS_MESSAGE_FORMAT_GW_BC_V01 and WMS_MESSAGE_FORMAT_MWI_V01
    // return QMI_ERR_INVALID_ARG and so are not checked
    if ((write_request_msg->raw_message_write_data.format
                                               != WMS_MESSAGE_FORMAT_GW_PP_V01) &&
        ( write_request_msg->raw_message_write_data.format
                                               != WMS_MESSAGE_FORMAT_CDMA_V01))
    {
       QCRIL_LOG_ERROR("Unsupported format %d for write, dropping",
                       (int)write_request_msg->raw_message_write_data.format);
       send_generic_failure(params_ptr->t, params_ptr->event_id);
       QCRIL_LOG_FUNC_RETURN();
       return;
    }

    memset(&write_response_msg, 0, sizeof(wms_raw_write_resp_msg_v01));
    write_request_msg->tag_type_valid = TRUE;
    write_request_msg->tag_type = tag;

    /* Writing the SMS means invoking two QMI requests: to write the message and modify the
       tag.  Make the requests synchronous, because we cannot invoke the second QMI request from
       a callback function.  Writing a message to UIM should be fast, so it should be safe
       to use synchronous calls in this case. */
    qmi_err = qcril_qmi_client_send_msg_sync_ex(QCRIL_QMI_CLIENT_WMS,
                                             QMI_WMS_RAW_WRITE_REQ_V01,
                                             write_request_msg,
                                             sizeof(wms_raw_write_req_msg_v01),
                                             &write_response_msg,
                                             sizeof(wms_raw_write_resp_msg_v01),
                                             QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT);

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_err, &write_response_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR( "Failed to write message, err %d", (int)ril_req_res);
      send_generic_failure(params_ptr->t, params_ptr->event_id);
    }
    else
    {
      /* The message was successfully written.*/
      QCRIL_LOG_INFO("QMI_WMS_RAW_WRITE_RESP received: SUCCESS");

      /* Android uses the GSDI index, since it calls RIL_REQUEST_SIM_IO.
         The GSDI index is the WMS index plus 1. */
      index_on_sim = write_response_msg.storage_index + 1;

      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = &index_on_sim;
      resp.resp_len = sizeof( int );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
    QCRIL_LOG_ERROR("write_request_msg is NULL!");
    send_generic_failure(params_ptr->t, params_ptr->event_id);
  }

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_request_write_sms_to_uim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_delete_sms_on_sim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DELETE_SMS_ON_SIM

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_delete_sms_on_sim
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_delete_req_msg_v01 request_msg;
  wms_delete_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to delete SMS on SIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for delete SMS on SIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_delete_req_msg_v01));
    request_msg.storage_type = WMS_STORAGE_TYPE_UIM_V01;

    request_msg.index_valid = TRUE;
    request_msg.index = ( (int *) params_ptr->data)[ 0 ] - 1;

    request_msg.message_mode_valid = TRUE;
    request_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate memory for response message to delete SMS on SIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_WMS,
                                        QMI_WMS_DELETE_REQ_V01,
                                        &request_msg,
                                        sizeof(wms_delete_req_msg_v01),
                                        response_msg,
                                        sizeof(wms_delete_resp_msg_v01),
                                        (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to receive response message for RIL_REQUEST_DELETE_SMS_ON_SIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_delete_sms_on_sim */


/*===========================================================================

  FUNCTION:  qcril_delete_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_DELETE_SMS_ON_SIM and
    RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_delete_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_request_resp_params_type resp;
  qcril_instance_id_e_type instance_id;
  wms_delete_resp_msg_v01 * delete_resp;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  /*-----------------------------------------------------------------------*/

  delete_resp = (wms_delete_resp_msg_v01 *) params_ptr->data;

  if (delete_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_DELETE_RESP received: SUCCESS");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("QMI_WMS_DELETE_RESP received with error %d",delete_resp->resp.error);
    qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_delete_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_get_smsc_address

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_get_smsc_address
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
   qcril_instance_id_e_type instance_id;
   qcril_sms_struct_type *i_ptr;
   uint32 user_data;
   qcril_reqlist_public_type reqlist_entry;
   qcril_request_resp_params_type resp;
   wms_get_smsc_address_resp_msg_v01* response_msg;
   errno_enum_type result;

   QCRIL_NOTUSED( ret_ptr );

   QCRIL_LOG_FUNC_ENTRY();
   instance_id = QCRIL_DEFAULT_INSTANCE_ID;
   i_ptr = &qcril_sms[ instance_id ];

   do
   {

      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK, QCRIL_EVT_NONE,
                                   NULL, &reqlist_entry );
      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
         // Failed to add to ReqList
         QCRIL_LOG_ERROR("Failed to add entry to request list");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         QCRIL_ASSERT(0); // this is a noop in release build
         break;
      }

      response_msg = qcril_malloc( sizeof(*response_msg) );
      if( response_msg == NULL )
      {
         QCRIL_LOG_ERROR("Failed to allocate memory for response");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         break;
      }

      user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                           reqlist_entry.req_id );

      result = qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_WMS,
                                     QMI_WMS_GET_SMSC_ADDRESS_REQ_V01,
                                     NULL,
                                     0,
                                     response_msg,
                                     sizeof(wms_get_smsc_address_resp_msg_v01),
                                     (void *)(uintptr_t) user_data );
      if ( result != QMI_NO_ERR )
      {
         QCRIL_LOG_ERROR("Failed to get response from qmi for getting SMSC Address");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         qcril_free( response_msg );
      }

   } while(0);

   QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_request_get_smsc_address() */

/*===========================================================================

  FUNCTION:  qcril_get_smsc_addr_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_GET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_get_smsc_addr_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_request_resp_params_type resp;
  qcril_instance_id_e_type instance_id;
  wms_get_smsc_address_resp_msg_v01 * get_smsc_addr_resp;
  char smsc_address[ QCRIL_SMS_MAX_SMSC_ADDRESS_SIZE ];

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  /*-----------------------------------------------------------------------*/

  get_smsc_addr_resp = (wms_get_smsc_address_resp_msg_v01 *) params_ptr->data;

  if (get_smsc_addr_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_GET_SMSC_ADDRESS_RESP received: SUCCESS");

    if ( qcril_sms_convert_smsc_address_to_ril_format( get_smsc_addr_resp, smsc_address ) )
    {
      /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
      resp.resp_pkt = smsc_address;
      resp.resp_len = strlen( smsc_address );
      qcril_send_request_response( &resp );
    }
    else
    {
      QCRIL_LOG_INFO("Failed to convert SMSC Address in RIL format.");
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
    QCRIL_LOG_INFO("QMI_WMS_GET_SMSC_ADDRESS_RESP received with error %d",get_smsc_addr_resp->resp.error);
    qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_get_smsc_addr_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_set_smsc_address

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_set_smsc_address
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_set_smsc_address_req_msg_v01 request_msg;
  wms_set_smsc_address_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for setting the smsc address.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Translate the SMSC address from ATCOP format to QMI format */
    memset(&request_msg, 0, sizeof(wms_set_smsc_address_req_msg_v01));
    if ( !qcril_sms_convert_smsc_address_to_qmi_format( (char *) params_ptr->data, &request_msg ) )
    {
      QCRIL_LOG_ERROR("Unable to convert SMSC address to QMI format");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to make an entry for setting the smsc address.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer of setting the smsc address.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(
                                     QCRIL_QMI_CLIENT_WMS,
                                     QMI_WMS_SET_SMSC_ADDRESS_REQ_V01,
                                     &request_msg,
                                     sizeof(wms_set_smsc_address_req_msg_v01),
                                     response_msg,
                                     sizeof(wms_set_smsc_address_resp_msg_v01),
                                     (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of setting the smsc address from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_set_smsc_address() */


/*===========================================================================

  FUNCTION:  qcril_set_smsc_addr_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_SET_SMSC_ADDRESS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_set_smsc_addr_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_request_resp_params_type resp;
  qcril_instance_id_e_type instance_id;
  wms_set_smsc_address_resp_msg_v01 * set_smsc_addr_resp;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  /*-----------------------------------------------------------------------*/

  set_smsc_addr_resp = (wms_set_smsc_address_resp_msg_v01 *) params_ptr->data;

  if (set_smsc_addr_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_SET_SMSC_ADDRESS_RESP received: SUCCESS");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("QMI_WMS_SET_SMSC_ADDRESS_RESP received with error %d",set_smsc_addr_resp->resp.error);
    qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_set_smsc_addr_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_report_sms_memory_status

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_REPORT_SMS_MEMORY_STATUS

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_report_sms_memory_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  char *mem_full_name = "";
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_set_memory_status_req_msg_v01 request_msg;
  wms_set_memory_status_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for sms memory status.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }


    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to make an entry for sms memory status.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if ( ( (int *) params_ptr->data)[ 0 ] == 0 )
    {
      mem_full_name = "Memory is full";
    }
    else if ( ( (int *) params_ptr->data)[ 0 ] == 1 )
    {
      mem_full_name = "Memory is available";
    }

    QCRIL_LOG_DEBUG( "Report Memory Status - %s", mem_full_name );

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_set_memory_status_req_msg_v01));
    request_msg.memory_available = ( (int *) params_ptr->data)[ 0 ];

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer of getting sms memory status.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(
                                    QCRIL_QMI_CLIENT_WMS,
                                    QMI_WMS_SET_MEMORY_STATUS_REQ_V01,
                                    &request_msg,
                                    sizeof(wms_set_memory_status_req_msg_v01),
                                    response_msg,
                                    sizeof(wms_set_memory_status_resp_msg_v01),
                                    (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of sms memory status from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_report_sms_memory_status() */


/*===========================================================================

  FUNCTION:  qcril_report_mem_status_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_REPORT_SMS_MEMORY_STATUS

    @return
    None.
*/
/*=========================================================================*/
void qcril_report_mem_status_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_request_resp_params_type resp;
  qcril_instance_id_e_type instance_id;
  wms_set_memory_status_resp_msg_v01 * mem_status_resp;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  /*-----------------------------------------------------------------------*/

  mem_status_resp = (wms_set_memory_status_resp_msg_v01 *) params_ptr->data;

  if (mem_status_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_SET_MEMORY_STATUS_RESP received: SUCCESS");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("QMI_WMS_SET_MEMORY_STATUS_RESP received with error %d",mem_status_resp->resp.error);
    qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_report_mem_status_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_get_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_get_broadcast_sms_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_get_broadcast_config_req_msg_v01 request_msg;
  wms_get_broadcast_config_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for get the broadcast sms config request.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_get_broadcast_config_req_msg_v01));
    request_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer to get the broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(
                                 QCRIL_QMI_CLIENT_WMS,
                                 QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01,
                                 &request_msg,
                                 sizeof(wms_get_broadcast_config_req_msg_v01),
                                 response_msg,
                                 sizeof(wms_get_broadcast_config_resp_msg_v01),
                                 (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of broadcast sms config from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_request_gsm_get_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_get_bc_config_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG and
    RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_get_bc_config_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_request_resp_params_type resp;
  qcril_instance_id_e_type instance_id;
  wms_get_broadcast_config_resp_msg_v01 * bc_config_resp;
  RIL_GSM_BroadcastSmsConfigInfo *gw_buf_ptr;
  RIL_GSM_BroadcastSmsConfigInfo **gw_table_ptr;
  RIL_CDMA_BroadcastSmsConfigInfo *cdma_buf_ptr;
  RIL_CDMA_BroadcastSmsConfigInfo **cdma_table_ptr;
  uint16 table_size;
  uint32 i;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  /*-----------------------------------------------------------------------*/

  bc_config_resp = (wms_get_broadcast_config_resp_msg_v01 *) params_ptr->data;

  if (bc_config_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_GET_BROADCAST_CONFIG_RESP received: SUCCESS");

    /* 3GPP */
    if (bc_config_resp->wms_3gpp_broadcast_info_valid)
    {
      table_size = bc_config_resp->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info_len;

      gw_buf_ptr =
      (RIL_GSM_BroadcastSmsConfigInfo *) qcril_malloc( table_size * sizeof( RIL_GSM_BroadcastSmsConfigInfo ) );
      gw_table_ptr =
      (RIL_GSM_BroadcastSmsConfigInfo **) qcril_malloc( table_size * sizeof( RIL_GSM_BroadcastSmsConfigInfo *) );

      if ( (gw_buf_ptr == NULL ) || (gw_table_ptr == NULL ))
      {
        QCRIL_LOG_ERROR("No buffer to compose the GW Broadcast SMS Info");
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
      }
      else
      {
        for ( i = 0; i < table_size; i++ )
        {
          gw_buf_ptr[ i ].fromServiceId =
          bc_config_resp->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].from_service_id;
          gw_buf_ptr[ i ].toServiceId =
          bc_config_resp->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].to_service_id;
          /* The modem does not support getting/setting the data coding
             scheme.  The interface specifies all values outside of
             0x00 to 0xFF will be treated as empty data coding scheme. */
          gw_buf_ptr[ i ].fromCodeScheme = 0xFFF;
          gw_buf_ptr[ i ].toCodeScheme = 0xFFF;
          gw_buf_ptr[ i ].selected =
          bc_config_resp->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].selected;

          gw_table_ptr[ i ] = &gw_buf_ptr[ i ];
        }

        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
        resp.resp_pkt = gw_table_ptr;
        resp.resp_len = table_size * sizeof( RIL_GSM_BroadcastSmsConfigInfo * );
        qcril_send_request_response( &resp );
      }
      if (gw_table_ptr != NULL)
      {
        qcril_free( gw_table_ptr );
      }
      if (gw_buf_ptr != NULL)
      {
        qcril_free( gw_buf_ptr );
      }
    }
    /* 3GPP2 */
    else if (bc_config_resp->wms_3gpp2_broadcast_info_valid)
    {
      table_size = bc_config_resp->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info_len;

      cdma_buf_ptr =
      (RIL_CDMA_BroadcastSmsConfigInfo *) qcril_malloc( table_size * sizeof( RIL_CDMA_BroadcastSmsConfigInfo ) );
      cdma_table_ptr =
      (RIL_CDMA_BroadcastSmsConfigInfo **) qcril_malloc( table_size * sizeof( RIL_CDMA_BroadcastSmsConfigInfo *) );

      if ( (cdma_buf_ptr == NULL ) || (cdma_table_ptr == NULL ))
      {
        QCRIL_LOG_ERROR( "No buffer to compose the CDMA Broadcast SMS Info" );
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
      }
      else
      {
        for ( i = 0; i < table_size; i++ )
        {
          cdma_buf_ptr[ i ].language =
          bc_config_resp->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].language;
          cdma_buf_ptr[ i ].service_category =
          bc_config_resp->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].service_category;
          cdma_buf_ptr[ i ].selected =
          bc_config_resp->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].selected;

          cdma_table_ptr[ i ] = &cdma_buf_ptr[ i ];
        }

        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
        resp.resp_pkt = cdma_table_ptr;
        resp.resp_len = table_size * sizeof( RIL_CDMA_BroadcastSmsConfigInfo * );
        qcril_send_request_response( &resp );
      }
      if (cdma_table_ptr != NULL)
      {
        qcril_free( cdma_table_ptr );
      }
      if (cdma_buf_ptr != NULL)
      {
        qcril_free( cdma_buf_ptr );
      }
    }
    else
    {
      QCRIL_LOG_INFO("QMI_WMS_GET_BROADCAST_CONFIG_RESP did not contain either 3GPP or 3GPP2 info");
      qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
  }
  else
  {
    QCRIL_LOG_INFO("QMI_WMS_GET_BROADCAST_CONFIG_RESP received with error %d",bc_config_resp->resp.error);
    qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_get_bc_config_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_set_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_set_broadcast_sms_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  uint32 i;
  uint8 bc_table_size;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry;
  wms_set_broadcast_config_req_msg_v01 request_msg;
  wms_set_broadcast_config_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for setting the broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Check if the broadcast SMS table is too big */
    bc_table_size = params_ptr->datalen / sizeof( RIL_GSM_BroadcastSmsConfigInfo * );
    if ( bc_table_size > QCRIL_SMS_BC_MM_TABLE_SIZE )
    {
      QCRIL_LOG_ERROR( "Size of GW Broadcast SMS Config table is too big : %d", bc_table_size );
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for setting the broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_set_broadcast_config_req_msg_v01));
    request_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;
    request_msg.wms_3gpp_broadcast_config_info_valid = TRUE;
    request_msg.wms_3gpp_broadcast_config_info_len = bc_table_size;

    for ( i = 0; i < bc_table_size; i++ )
    {
      request_msg.wms_3gpp_broadcast_config_info[i].from_service_id =
      ( (RIL_GSM_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->fromServiceId;
      request_msg.wms_3gpp_broadcast_config_info[i].to_service_id =
      ( (RIL_GSM_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->toServiceId;
      request_msg.wms_3gpp_broadcast_config_info[i].selected =
      ( (RIL_GSM_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->selected;;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer of setting the broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(
                                 QCRIL_QMI_CLIENT_WMS,
                                 QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01,
                                 &request_msg,
                                 sizeof(wms_set_broadcast_config_req_msg_v01),
                                 response_msg,
                                 sizeof(wms_set_broadcast_config_resp_msg_v01),
                                 (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of setting the broadcast sms config from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_gsm_set_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_set_bc_config_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG and
    RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_set_bc_config_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_request_resp_params_type resp;
  qcril_instance_id_e_type instance_id;
  wms_set_broadcast_config_resp_msg_v01 * bc_config_resp;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  /*-----------------------------------------------------------------------*/

  bc_config_resp = (wms_set_broadcast_config_resp_msg_v01 *) params_ptr->data;

  if(bc_config_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_SET_BROADCAST_CONFIG_RESP received: SUCCESS");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("QMI_WMS_SET_BROADCAST_CONFIG_RESP received with error %d",bc_config_resp->resp.error);
    qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_set_bc_config_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_gsm_sms_broadcast_activation

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_gsm_sms_broadcast_activation
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_set_broadcast_activation_req_msg_v01 request_msg;
  wms_set_broadcast_activation_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for sms broadcast activation.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to make an entry for sms broadcast activation.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in QMI message */
    memset(&request_msg, 0, sizeof(wms_set_broadcast_activation_req_msg_v01));
    request_msg.broadcast_activation_info.message_mode = WMS_MESSAGE_MODE_GW_V01;
    request_msg.broadcast_activation_info.bc_activate = !(( (int *) params_ptr->data)[ 0 ]);

    if( feature_all_bc_msg_3gpp )
    {
        request_msg.activate_all_valid = TRUE;
        request_msg.activate_all = 0x01;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer of sms broadcast activation.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(
                             QCRIL_QMI_CLIENT_WMS,
                             QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01,
                             &request_msg,
                             sizeof(wms_set_broadcast_activation_req_msg_v01),
                             response_msg,
                             sizeof(wms_set_broadcast_activation_resp_msg_v01),
                             (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of sms broadcast activation.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_gsm_sms_broadcast_activation() */


/*===========================================================================

  FUNCTION:  qcril_bc_activate_cb

===========================================================================*/
/*!
    @brief
    Handles response to RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION and
    RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION.

    @return
    None.
*/
/*=========================================================================*/
void qcril_bc_activate_cb
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_request_resp_params_type resp;
  qcril_instance_id_e_type instance_id;
  wms_set_broadcast_activation_resp_msg_v01 * bc_activate_resp;


  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  /*-----------------------------------------------------------------------*/

  bc_activate_resp = (wms_set_broadcast_activation_resp_msg_v01 *) params_ptr->data;

  if (bc_activate_resp->resp.result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QMI_WMS_SET_BROADCAST_ACTIVATION_RESP received: SUCCESS");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("QMI_WMS_SET_BROADCAST_ACTIVATION_RESP received with error %d",
                   bc_activate_resp->resp.error);
    qcril_default_request_resp_params( instance_id,params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

} /* qcril_bc_activate_cb */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_send_sms

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_send_sms
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  RIL_CDMA_SMS_Message *cdma_sms_msg;
  qcril_request_resp_params_type resp;
  uint32 user_data;
  wms_raw_send_req_msg_v01 request_msg;
  wms_raw_send_resp_msg_v01* response_msg;
  qcril_reqlist_public_type reqlist_entry;
  qcril_sms_struct_type *i_ptr;
  nas_sms_status_enum_type_v01 lte_sms_status;
  uint8_t lte_sms_status_valid;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    qcril_qmi_nas_fetch_lte_sms_status( &lte_sms_status_valid, &lte_sms_status);
    if (!wms_ready(WMS_SERVICE_READY_STATUS_3GPP2_V01) &&
        !(lte_sms_status_valid && (NAS_SMS_STATUS_1X_V01 == lte_sms_status))
       ) {
       send_generic_failure(params_ptr->t, params_ptr->event_id);
       break;
    }

    instance_id = QCRIL_DEFAULT_INSTANCE_ID;

    cdma_sms_msg = (RIL_CDMA_SMS_Message *) params_ptr->data;

    QCRIL_NOTUSED( ret_ptr );

    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for sending cdma sms.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if( TRUE == qmi_ril_block_mo_sms_on_1x() ) //check If user wants the MO SMS to be blocked when IMS is not yet registered
    {
      QCRIL_LOG_ERROR("Sending cdma MO sms failed because IMS is not registered.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    i_ptr = &qcril_sms[ instance_id ];

    /* Add entry to ReqList. */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList*/
      QCRIL_LOG_ERROR("Failed to add an entry for sending cdma sms.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_raw_send_req_msg_v01));
    if (!qcril_sms_fill_wms_payload(
                                   TRUE,
                                   cdma_sms_msg,
                                   NULL,
                                   NULL,
                                   &request_msg.raw_message_data.format,
                                   (uint32 *)&request_msg.raw_message_data.raw_message_len,
                                   &request_msg.raw_message_data.raw_message[0],
                                   FALSE,
                                   TRUE ))
    {
      QCRIL_LOG_ERROR("Filling SMS payload per QMI requirement is failed for cdma SMS message.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    request_msg.sms_on_ims_valid = TRUE;
    request_msg.sms_on_ims = FALSE;

    QCRIL_LOG_DEBUG( "IS FORCE ON DC = %d", is_force_on_dc );
    if(is_force_on_dc)
    {
        request_msg.force_on_dc_valid = TRUE;
        request_msg.force_on_dc.force_on_dc = 0x01;
    }

    /* Compose the user data */
    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer of cdma sms.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_WMS,
                                        QMI_WMS_RAW_SEND_REQ_V01,
                                        &request_msg,
                                        sizeof(wms_raw_send_req_msg_v01),
                                        response_msg,
                                        sizeof(wms_raw_send_resp_msg_v01),
                                        (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of cdma sms from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_send_sms() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_sms_acknowledge

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_sms_acknowledge
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  RIL_CDMA_SMS_Ack *ack_ptr;
  char details[ 80 ];
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry;
  wms_send_ack_req_msg_v01 request_msg;
  wms_send_ack_resp_msg_v01* response_msg;

  int                        need_reset_ack_expry_tmr;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  ack_ptr = (RIL_CDMA_SMS_Ack *) params_ptr->data;

  QCRIL_NOTUSED( ret_ptr );

  need_reset_ack_expry_tmr = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ),
                  "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for acknowledgement of cdma sms.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (!i_ptr->sms_ack_info.cdma_ack_pending)
    {
      QCRIL_LOG_ERROR("Unable to send ack; no messages to be acked." );

      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (!i_ptr->sms_ack_info.cdma_ack_needed)
    {
      QCRIL_LOG_INFO("Dropping ack since no ack is needed.");

      i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
      i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;

      need_reset_ack_expry_tmr = TRUE;

      qcril_default_request_resp_params( instance_id, params_ptr->t,
                                  params_ptr->event_id, RIL_E_SUCCESS, &resp );
      qcril_send_request_response( &resp );
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for acknowledgement of cdma sms.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_send_ack_req_msg_v01));
    request_msg.ack_information.transaction_id = i_ptr->sms_ack_info.cdma_transaction_id;
    request_msg.ack_information.message_protocol = WMS_MESSAGE_PROTOCOL_CDMA_V01;

    if ( ack_ptr->uErrorClass == RIL_CDMA_SMS_NO_ERROR )
    {
      request_msg.ack_information.success = TRUE;
    }
    else if ( ack_ptr->uErrorClass == RIL_CDMA_SMS_ERROR )
    {
      request_msg.ack_information.success = FALSE;

      request_msg.wms_3gpp2_failure_information_valid = TRUE;
      request_msg.wms_3gpp2_failure_information.error_class = WMS_ERROR_CLASS_3GPP2_FAILURE_TEMPORARY_V01;
      request_msg.wms_3gpp2_failure_information.tl_status = ack_ptr->uSMSCauseCode;
    }

    if (i_ptr->sms_ack_info.cdma_send_ack_on_ims)
    {
      request_msg.sms_on_ims_valid = TRUE;
      request_msg.sms_on_ims = TRUE;
    }
    else
    {
      request_msg.sms_on_ims_valid = TRUE;
      request_msg.sms_on_ims = FALSE;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer for acknowledgement of cdma sms.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_WMS,
                                        QMI_WMS_SEND_ACK_REQ_V01,
                                        &request_msg,
                                        sizeof(wms_send_ack_req_msg_v01),
                                        response_msg,
                                        sizeof(wms_send_ack_resp_msg_v01),
                                        (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response for acknowledgement of cdma sms.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);

      qcril_free( response_msg );
    }

    /* Reset the ack pending flag. If the ack is not received by the network,
       the network will resend the SMS, and RIL will have another chance
       to ack it. */
    i_ptr->sms_ack_info.cdma_ack_pending = FALSE;
    i_ptr->sms_ack_info.cdma_ack_needed = FALSE;
    i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;

    need_reset_ack_expry_tmr = TRUE;

  } while(0);

  if ( need_reset_ack_expry_tmr )
  {
      if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
      {
        QCRIL_LOG_INFO( "resetting MT SMS ack tmr per ack reception" );
        qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
        qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
      }
  }

  QCRIL_SNPRINTF( details, sizeof( details ),
                 "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_request_cdma_sms_acknowledge */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_write_sms_to_ruim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_write_sms_to_ruim
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  RIL_CDMA_SMS_WriteArgs *SMS_Write_info;
  qcril_request_resp_params_type resp;
  wms_raw_write_req_msg_v01 request_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  SMS_Write_info = (RIL_CDMA_SMS_WriteArgs *) params_ptr->data;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    memset(&request_msg, 0, sizeof(wms_raw_write_req_msg_v01));
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for cdma write sms to RUIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

  /* Invalid status */
  if ( SMS_Write_info->status  > 3 )
  {
    QCRIL_LOG_ERROR("Invalid status of cdma write sms to RUIM.");
    send_generic_failure(params_ptr->t, params_ptr->event_id);
    break;
  }

  /* Fill in the QMI request to write the message to the RUIM */
  request_msg.raw_message_write_data.storage_type = WMS_STORAGE_TYPE_UIM_V01;

  if (!qcril_sms_fill_wms_payload( TRUE,
                                   &SMS_Write_info->message,
                                   NULL,
                                   NULL,
                                   &request_msg.raw_message_write_data.format,
                                   (uint32 *)&request_msg.raw_message_write_data.raw_message_len,
                                   &request_msg.raw_message_write_data.raw_message[0],
                                   FALSE,
                                   qcril_sms_is_tag_mo(SMS_Write_info->status)))
  {
    QCRIL_LOG_ERROR("Filling SMS payload per QMI requirement is failed for cdma write sms to RUIM.");
    send_generic_failure(params_ptr->t, params_ptr->event_id);
    break;
  }

  /* Write the message to the UIM */
  qcril_sms_request_write_sms_to_uim(&request_msg,
                                     params_ptr,
                                     instance_id,
                                     qcril_sms_map_ril_tag_to_qmi_tag(SMS_Write_info->status),
                                     WMS_MESSAGE_MODE_CDMA_V01);

  }while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_write_cdma_sms_to_ruim() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_delete_sms_on_ruim

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_delete_sms_on_ruim
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_delete_req_msg_v01 request_msg;
  wms_delete_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for deleting cdma sms on RUIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for deleting cdma sms on RUIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_delete_req_msg_v01));
    request_msg.storage_type = WMS_STORAGE_TYPE_UIM_V01;

    request_msg.index_valid = TRUE;
    request_msg.index = ( (int *) params_ptr->data)[ 0 ] - 1;

    request_msg.message_mode_valid = TRUE;
    request_msg.message_mode = WMS_MESSAGE_MODE_CDMA_V01;

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer of deleting cdma sms on RUIM.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_WMS,
                                        QMI_WMS_DELETE_REQ_V01,
                                        &request_msg,
                                        sizeof(wms_delete_req_msg_v01),
                                        response_msg,
                                        sizeof(wms_delete_resp_msg_v01),
                                        (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of deleting cdma sms on RUIM from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_delete_sms_on_ruim */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_get_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_get_broadcast_sms_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  wms_get_broadcast_config_req_msg_v01 request_msg;
  wms_get_broadcast_config_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for get cdma broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_get_broadcast_config_req_msg_v01));
    request_msg.message_mode = WMS_MESSAGE_MODE_CDMA_V01;

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer to get cdma broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(
                                 QCRIL_QMI_CLIENT_WMS,
                                 QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01,
                                 &request_msg,
                                 sizeof(wms_get_broadcast_config_req_msg_v01),
                                 response_msg,
                                 sizeof(wms_get_broadcast_config_resp_msg_v01),
                                 (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of cdma broadcast sms config from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_get_broadcast_sms_config() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_set_broadcast_sms_config

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_set_broadcast_sms_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  uint8 bc_table_size;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry;
  wms_set_broadcast_config_req_msg_v01 request_msg;
  wms_set_broadcast_config_resp_msg_v01* response_msg;
  uint32 i;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for setting the cdma broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Check if the broadcast SMS table is too big */
    bc_table_size = params_ptr->datalen / sizeof( RIL_CDMA_BroadcastSmsConfigInfo * );
    if ( bc_table_size > QCRIL_SMS_BC_MM_TABLE_SIZE )
    {
      QCRIL_LOG_ERROR( "Size of CDMA Broadcast SMS Config table is too big : %d", bc_table_size );
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for setting the cdma broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in the QMI request */
    memset(&request_msg, 0, sizeof(wms_set_broadcast_config_req_msg_v01));
    request_msg.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
    request_msg.wms_3gpp2_broadcast_config_info_valid = TRUE;
    request_msg.wms_3gpp2_broadcast_config_info_len = bc_table_size;

    for ( i = 0; i < bc_table_size; i++ )
    {
      request_msg.wms_3gpp2_broadcast_config_info[i].service_category =
      ( (RIL_CDMA_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->service_category;
      request_msg.wms_3gpp2_broadcast_config_info[i].language =
      ( (RIL_CDMA_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->language;
      request_msg.wms_3gpp2_broadcast_config_info[i].selected =
      ( (RIL_CDMA_BroadcastSmsConfigInfo **) params_ptr->data )[ i ]->selected;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer for setting cdma broadcast sms config.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(
                                 QCRIL_QMI_CLIENT_WMS,
                                 QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01,
                                 &request_msg,
                                 sizeof(wms_set_broadcast_config_req_msg_v01),
                                 response_msg,
                                 sizeof(wms_set_broadcast_config_resp_msg_v01),
                                 (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of setting cdma broadcast sms config from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_set_broadcast_sms_config */


/*===========================================================================

  FUNCTION:  qcril_sms_request_cdma_sms_broadcast_activation

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_cdma_sms_broadcast_activation
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry;
  wms_set_broadcast_activation_req_msg_v01 request_msg;
  wms_set_broadcast_activation_resp_msg_v01* response_msg;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_ERROR("No data available to request for cdma sms broadcast activation.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for cdma sms broadcast activation.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    /* Fill in QMI message */
    memset(&request_msg, 0, sizeof(wms_set_broadcast_activation_req_msg_v01));
    request_msg.broadcast_activation_info.message_mode = WMS_MESSAGE_MODE_CDMA_V01;
    request_msg.broadcast_activation_info.bc_activate = !(( (int *) params_ptr->data)[ 0 ]);

    if( feature_all_bc_msg_3gpp2 )
    {
        request_msg.activate_all_valid = TRUE;
        request_msg.activate_all = 0x01;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

    response_msg = qcril_malloc( sizeof(*response_msg) );
    if( response_msg == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory for response buffer of cdma sms broadcast activation.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    if (qcril_qmi_client_send_msg_async(
                             QCRIL_QMI_CLIENT_WMS,
                             QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01,
                             &request_msg,
                             sizeof(wms_set_broadcast_activation_req_msg_v01),
                             response_msg,
                             sizeof(wms_set_broadcast_activation_resp_msg_v01),
                             (void *)(uintptr_t) user_data) != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Failed to get the response of cdma sms broadcast activation from qmi.");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      qcril_free( response_msg );
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_cdma_sms_broadcast_activation() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_ims_registration_state

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_IMS_REGISTRATION_STATE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_ims_registration_state
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_sms_struct_type *i_ptr;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  wms_get_transport_layer_resp_msg_v01 response_msg;
  wms_get_transport_nw_reg_resp_msg_v01 nw_reg_response_msg;
  qcril_reqlist_generic_concurency_requests_requirement_type ims_reg_specs;
  uint32 ims_reg_event_value;
  int is_duplicate_request;
  int ims_registration_state[ 2 ] = { FALSE, RADIO_TECH_3GPP};
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_sms[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );

    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_ERROR("Failed to add an entry for IMS registration");
      send_generic_failure(params_ptr->t, params_ptr->event_id);
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    memset( &nw_reg_response_msg, 0, sizeof( nw_reg_response_msg ) );

    if ( qcril_qmi_client_send_msg_sync_ex(
                                  QCRIL_QMI_CLIENT_WMS,
                                  QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01,
                                  NULL,
                                  0,
                                  &nw_reg_response_msg,
                                  sizeof(nw_reg_response_msg),
                                  QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT ) )
    {
      QCRIL_LOG_ERROR("Failed to get the response of IMS NW registration state from qmi.");
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t,
                          params_ptr->event_id, RIL_E_GENERIC_FAILURE );
      break;
    }

    if ( TRUE == nw_reg_response_msg.transport_nw_reg_status_valid )
    {
        if ( WMS_TRANSPORT_NW_REG_STATUS_FULL_SERVICE_V01 == nw_reg_response_msg.transport_nw_reg_status )
        {
            memset( &response_msg, 0, sizeof( response_msg ) );

            if (qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_WMS,
                                  QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01,
                                  NULL,
                                  0,
                                  &response_msg,
                                  sizeof(response_msg),
                                  QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT ) )
            {
                QCRIL_LOG_ERROR("Failed to get the response of IMS registration state from qmi.");
                qcril_send_empty_payload_request_response( instance_id, params_ptr->t,
                                  params_ptr->event_id, RIL_E_GENERIC_FAILURE );
                break;
            }

            QCRIL_LOG_INFO("QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP received: SUCCESS");

            if ( response_msg.registered_ind_valid &&
                response_msg.registered_ind &&
                response_msg.transport_layer_info_valid &&
                (response_msg.transport_layer_info.transport_type == WMS_TRANSPORT_TYPE_IMS_V01)
               )
            {
                ims_registration_state[ 0 ] = TRUE;

                if (response_msg.transport_layer_info.transport_cap == WMS_TRANSPORT_CAP_GW_V01)
                {
                    ims_registration_state[ 1 ] = RADIO_TECH_3GPP;
                }
                else if (response_msg.transport_layer_info.transport_cap == WMS_TRANSPORT_CAP_CDMA_V01)
                {
                    ims_registration_state[ 1 ] = RADIO_TECH_3GPP2;
                }
            }

            /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
            qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
            resp.resp_pkt = ims_registration_state;
            resp.resp_len = sizeof( ims_registration_state );
            qcril_send_request_response( &resp );
        }
        else
        {
            /* Send RIL_E_SUCCESS response, and remove the entry from ReqList. */
            qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
            resp.resp_pkt = ims_registration_state;
            resp.resp_len = sizeof( ims_registration_state );
            qcril_send_request_response( &resp );
        }
    }
    else
    {
        QCRIL_LOG_ERROR("Transport Network registration status is not valid.");
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t,
                       params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        break;
    }

  } while(0);

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_ims_registration_state() */


/*===========================================================================

  FUNCTION:  qcril_sms_request_ims_send_sms

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_IMS_SEND_SMS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_sms_request_ims_send_sms
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
   qcril_instance_id_e_type instance_id;
   qcril_request_resp_params_type resp;
   uint32 user_data;
   wms_raw_send_req_msg_v01 request_msg;
   wms_raw_send_resp_msg_v01* response_msg;
   qcril_reqlist_public_type reqlist_entry;
   RIL_CDMA_SMS_Message *cdma_sms_msg;
   RIL_IMS_SMS_Message *ims_sms_msg;
   boolean sms_payload_in_cdma_format;
   const char *smsc_address = NULL;
   const char *gw_pdu = NULL;
   nas_sms_status_enum_type_v01 lte_sms_status;
   uint8_t lte_sms_status_valid;
   QCRIL_NOTUSED( ret_ptr );

   QCRIL_LOG_FUNC_ENTRY();

   instance_id = QCRIL_DEFAULT_INSTANCE_ID;
   ims_sms_msg = (RIL_IMS_SMS_Message *) params_ptr->data;

   do
   {
      qcril_qmi_nas_fetch_lte_sms_status( &lte_sms_status_valid, &lte_sms_status);
      if ( ( 0 == params_ptr->datalen ) || ( NULL == params_ptr->data ) )
      {
         QCRIL_LOG_ERROR("Data not valid");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         break;
      }

      // Check for WMS SERVICE READY
      if (ims_sms_msg->tech == RADIO_TECH_3GPP)
      {
         if ( ims_sms_msg->retry && (!wms_ready(WMS_SERVICE_READY_STATUS_3GPP_V01) &&
              !(lte_sms_status_valid && (NAS_SMS_STATUS_3GPP_V01 == lte_sms_status) ) )
            )
         {
            send_generic_failure(params_ptr->t, params_ptr->event_id);
            break;
         }
      }
      else if (ims_sms_msg->tech == RADIO_TECH_3GPP2)
      {
         // tech is 3GPP2
         if ( ims_sms_msg->retry && ( !wms_ready(WMS_SERVICE_READY_STATUS_3GPP2_V01) &&
              !(lte_sms_status_valid && (NAS_SMS_STATUS_1X_V01 == lte_sms_status)) )
            )
         {
            send_generic_failure(params_ptr->t, params_ptr->event_id);
            break;
         }
      }
      else
      {
         QCRIL_LOG_ERROR("unsupported radio tech %d", ims_sms_msg->tech);
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         break;
      }

      QCRIL_LOG_INFO("ims retry 3gpp %d, 3gpp2 %d, is_rety %d, tech %d",
                      feature_ims_3gpp_retry, feature_ims_3gpp2_retry,
                      ims_sms_msg->retry, ims_sms_msg->tech);

      // reject retries on 3gpp if retry feature is false
      if (!feature_ims_3gpp_retry
          && ims_sms_msg->retry
          && (ims_sms_msg->tech == RADIO_TECH_3GPP) ) {
         QCRIL_LOG_INFO("Feature ims_retry_on_3gpp disabled - SMS REJECTED");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         break;
      }

      // reject retries on 3gpp2 if feature is false
      if (!feature_ims_3gpp2_retry
          && ims_sms_msg->retry
          && (ims_sms_msg->tech == RADIO_TECH_3GPP2) ) {
         QCRIL_LOG_INFO("Feature ims_retry_on_3gpp2 disabled - SMS REJECTED");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         break;
      }

      if (ims_sms_msg->tech == RADIO_TECH_3GPP2)
      {
         QCRIL_LOG_INFO("sms tech is 3GPP2");
         sms_payload_in_cdma_format = TRUE;
         cdma_sms_msg = ims_sms_msg->message.cdmaMessage;
      }
      else
      {
         // tech is 3GPP
         QCRIL_LOG_INFO("sms tech is 3GPP");

         sms_payload_in_cdma_format = FALSE;
         cdma_sms_msg = NULL;
         smsc_address = ((const char **)ims_sms_msg->message.gsmMessage)[0];
         gw_pdu = ((const char **)ims_sms_msg->message.gsmMessage)[1];

         if ( !qcril_mo_sms_error_check( gw_pdu ) )
         {
            QCRIL_LOG_ERROR("qcril_mo_sms_error_check failed");
            send_generic_failure(params_ptr->t, params_ptr->event_id);
            break;
         }
      }

      /* Add entry to ReqList. */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );
      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
         QCRIL_LOG_ERROR("Failed to add task to reqlist");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         break;
      }

      /* Fill in the QMI request */
      memset(&request_msg, 0, sizeof(wms_raw_send_req_msg_v01));
      if (!qcril_sms_fill_wms_payload(
                       sms_payload_in_cdma_format,
                       cdma_sms_msg,
                       smsc_address,
                       gw_pdu,
                       &request_msg.raw_message_data.format,
                       (uint32 *)&request_msg.raw_message_data.raw_message_len,
                       &request_msg.raw_message_data.raw_message[0],
                       !ims_sms_msg->retry,
                       TRUE ))
      {
         QCRIL_LOG_ERROR("Failed qcril_sms_fill_wms_payload");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         break;
      }

      if (ims_sms_msg->retry)
      {
         QCRIL_LOG_INFO("sms msg is a retry...");
         request_msg.retry_message_valid = TRUE;
         request_msg.retry_message = WMS_MESSAGE_IS_A_RETRY_V01;

         request_msg.retry_message_id_valid = TRUE;
         request_msg.retry_message_id = ims_sms_msg->messageRef;

         /* Indicate the message must NOT go on IMS */
         request_msg.sms_on_ims_valid = TRUE;
         request_msg.sms_on_ims = FALSE;

         QCRIL_LOG_DEBUG( "IS FORCE ON DC = %d", is_force_on_dc );
         if((ims_sms_msg->tech == RADIO_TECH_3GPP2) && is_force_on_dc)
         {
            request_msg.force_on_dc_valid = TRUE;
            request_msg.force_on_dc.force_on_dc = 0x01;
         }
      }
      else
      {
         /* Indicate the message MUST go on IMS */
         QCRIL_LOG_INFO("sending sms on ims");
         request_msg.sms_on_ims_valid = TRUE;
         request_msg.sms_on_ims = TRUE;
      }

      /* Compose the user data */
      user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

      response_msg = qcril_malloc( sizeof(*response_msg) );
      if( response_msg == NULL )
      {
         QCRIL_LOG_ERROR("Failed malloc, must be out of memory");
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         break;
      }

      int result = qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_WMS,
                                             QMI_WMS_RAW_SEND_REQ_V01,
                                             &request_msg,
                                             sizeof(wms_raw_send_req_msg_v01),
                                             response_msg,
                                             sizeof(wms_raw_send_resp_msg_v01),
                                             (void *)(uintptr_t) user_data);
      if (result == QMI_NO_ERR)
      {
         QCRIL_LOG_INFO("Sent QMI_WMS_RAW_SEND_REQ_V01");
      }
      else
      {
         QCRIL_LOG_ERROR("Failed qcril_qmi_client_send_msg_async w/%d", result);
         send_generic_failure(params_ptr->t, params_ptr->event_id);
         qcril_free( response_msg );
      }

   } while(0);

   QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_request_ims_send_sms() */

/*===========================================================================

                   QMI COMMAND CALLBACK HANDLER

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_qmi_sms_command_cb

===========================================================================*/
/*!
    @brief
    Common Callback for all the QMI voice commands.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_sms_command_cb
(
  qmi_client_type              user_handle,
  unsigned int                 msg_id,
  void                        *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint16 req_id;
  qcril_reqlist_public_type req_info;
  qcril_request_params_type req_data;
  /*-----------------------------------------------------------------------*/

  // resp_cb_data is not a pointer, do not check for NULL
  QCRIL_ASSERT( resp_c_struct != NULL );
  QCRIL_ASSERT( user_handle != NULL );
  instance_id = QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( (uint32)(uintptr_t) resp_cb_data );
  modem_id = QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( (uint32)(uintptr_t) resp_cb_data );
  req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( (uint32)(uintptr_t) resp_cb_data );
  req_data.modem_id = modem_id;
  req_data.instance_id = instance_id;
  req_data.datalen = resp_c_struct_len;
  req_data.data = resp_c_struct;
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("msg_id (0x%x) %s", msg_id, qcril_sms_lookup_cmd_name(msg_id));

  /* Lookup the Token ID */
  if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
  {
    req_data.t = req_info.t;
    req_data.event_id = req_info.request;
    if ( transp_err != QMI_NO_ERR )
    {
      QCRIL_LOG_INFO("Transp error (0x%x) recieved from QMI for RIL request %d", transp_err, req_info.request);
      /* Send GENERIC_FAILURE response */

      qcril_send_empty_payload_request_response( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE );
    }
    else if(resp_c_struct == NULL)
    {
        QCRIL_LOG_INFO("Invalid QMI Response for RIL request %d, QMI request 0x%x", req_info.request, msg_id);
      /* Send GENERIC_FAILURE response */
      qcril_send_empty_payload_request_response( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE );
    }
    else
    {
    switch (msg_id)
    {
      case QMI_WMS_RAW_SEND_REQ_V01:
        qcril_mo_sms_cb(&req_data);
        break;

      case QMI_WMS_SEND_ACK_REQ_V01:
        qcril_ack_cb(&req_data);
        break;

      case QMI_WMS_DELETE_REQ_V01:
        qcril_delete_cb(&req_data);
        break;

      case QMI_WMS_GET_SMSC_ADDRESS_REQ_V01:
        qcril_get_smsc_addr_cb(&req_data);
        break;

      case QMI_WMS_SET_SMSC_ADDRESS_REQ_V01:
        qcril_set_smsc_addr_cb(&req_data);
        break;

      case QMI_WMS_SET_MEMORY_STATUS_REQ_V01:
        qcril_report_mem_status_cb(&req_data);
        break;

      case QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01:
        qcril_get_bc_config_cb(&req_data);
        break;

      case QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01:
        qcril_set_bc_config_cb(&req_data);
        break;

      case QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01:
        qcril_bc_activate_cb(&req_data);
        break;

      default:
        QCRIL_LOG_INFO("Unsupported QMI WMS message 0x%x", msg_id);
        break;
    }
  }
  }
  else
  {
    QCRIL_LOG_ERROR( "Req ID: %d not found", req_id );
  }
  if( resp_c_struct != NULL)
  {
  qcril_free( resp_c_struct );  // we own it
  }
}/* qcril_qmi_sms_command_cb */


/*===========================================================================

                   INDICATION PROCESSING FUNCTIONS

===========================================================================*/
/*=========================================================================
  FUNCTION:  qcril_sms_decode_ind

===========================================================================*/
/*!
    @brief
    Decodes a QMI indication

    @return
    If successful, payload of QMI indication
    NULL otherwise.
*/
/*=========================================================================*/
void * qcril_sms_decode_ind
(
  unsigned long      msg_id,
  unsigned char      *ind_buf,
  int                ind_buf_len
)
{
  uint32 decoded_payload_len = 0;
  boolean success = TRUE;
  qmi_client_error_type qmi_err;
  void * decoded_payload = NULL;

  QCRIL_LOG_FUNC_ENTRY();
  qmi_err = qmi_idl_get_message_c_struct_len(qcril_qmi_client_get_service_object(QCRIL_QMI_CLIENT_WMS),
                                             QMI_IDL_INDICATION,
                                             msg_id,
                                             (uint32_t *) &decoded_payload_len);

  if (qmi_err != QMI_NO_ERR)
  {
    QCRIL_LOG_ERROR("Error %d getting message struct len for QMI WMS Indication: %d", qmi_err, msg_id);
    success = FALSE;
  }

  if (success)
  {
    decoded_payload = qcril_malloc(decoded_payload_len);

    if (decoded_payload == NULL)
    {
      QCRIL_LOG_ERROR("Failed to allocate buffer for QMI WMS Indication: %d", msg_id);
      success = FALSE;
    }
  }

  if (success)
  {
    memset(decoded_payload, 0, decoded_payload_len);

    qmi_err = qmi_client_message_decode(qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_WMS),
                                        QMI_IDL_INDICATION,
                                        msg_id,
                                        ind_buf,
                                        ind_buf_len,
                                        decoded_payload,
                                        (int)decoded_payload_len);

    if (qmi_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Error %d decoding QMI WMS Indication: %d", qmi_err, msg_id);
      success = FALSE;
    }
  }

  if(!success && decoded_payload != NULL)
    {
      qcril_free(decoded_payload);
    decoded_payload = NULL;
    }

  QCRIL_LOG_FUNC_RETURN();
  return decoded_payload;

} /* qcril_sms_decode_ind */


/*=========================================================================
  FUNCTION:  qcril_sms_check_if_gw_msg_is_status_report

===========================================================================*/
/*!
    @brief
    Determine if the GW SMS message is a Status Report.

    @return
    TRUE if the GW SMS message is a Status Report; FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_sms_check_if_gw_msg_is_status_report
(
  wms_transfer_route_mt_message_type_v01 * mt_sms
)
{
  boolean is_status_report = FALSE;

  QCRIL_LOG_FUNC_ENTRY();
  /* The first two bits of the first byte of the payload indicates the
     TP-MTI (Message Type Indicator).  A value of 2 means it is a Status
     Report. */
  if ((mt_sms->data[0] & 0x03) ==  2)
  {
    is_status_report = TRUE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(is_status_report);
  return (is_status_report);
} /* qcril_sms_check_if_gw_msg_is_status_report */


/*=========================================================================
  FUNCTION:  qcril_sms_convert_mt_cdma_sms_to_RIL_format

===========================================================================*/
/*!
    @brief
    Translates a CDMA SMS message from QMI format to RIL format.

    @return
    TRUE if the MT CDMA SMS was successfully converted to RIL format
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_convert_mt_cdma_sms_to_RIL_format
(
  wms_transfer_route_mt_message_type_v01 * mt_sms,
  RIL_CDMA_SMS_Message *RIL_sms_message
)
{
  boolean success = FALSE;

  QCRIL_LOG_FUNC_ENTRY();
  if(mt_sms != NULL && RIL_sms_message != NULL)
  {

  success = qcril_sms_convert_mt_sms_qmi_to_ril(mt_sms->data,
                                                mt_sms->data_len,
                                                RIL_sms_message);
  }

  if (success == FALSE)
  {
    QCRIL_LOG_ERROR("Failed to convert MT CDMA SMS to RIL format");
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(success);
  return(success);
} /* qcril_sms_convert_mt_cdma_sms_to_RIL_format */


/*=========================================================================
  FUNCTION:  qcril_sms_process_mt_cdma_sms

===========================================================================*/
/*!
    @brief
    Performs processing necessary when an MT CDMA SMS message is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_mt_cdma_sms
(
  qcril_instance_id_e_type instance_id,
  wms_event_report_ind_msg_v01 * event_report_ind,
  wms_message_format_enum_v01 format
)
{
  qcril_sms_struct_type *i_ptr;
  boolean is_processing_ok = TRUE;
  char *buf_ptr;
  char details[ 80 ];
  qcril_unsol_resp_params_type unsol_resp;
  RIL_CDMA_SMS_Message * cdma_sms_msg;

  struct timeval ack_expry_tmr_window = { 30 , 0 }; // 30 seconds

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  i_ptr = &qcril_sms[ instance_id ];


  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  /*-----------------------------------------------------------------------*/

  /* Allocate buffer to decode new CDMA SMS */
  buf_ptr = qcril_malloc( sizeof( RIL_CDMA_SMS_Message ) );
  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR("Failed to allocate buffer to decode new CDMA SMS.");
  }
  else
  {
    /* According to ril.h, we are to drop new SMS Messages until the previous
       message has been acked.  Note that the network will not send any new
       messages until the previous one has been acked; so the messages we are
       dropping here should all be duplicates.

       We only drop new SMS messages if an ack is needed.  There are some
       messages in the category of "no ack needed" which need to be sent to
       Android immediately.  For example, the segments of a multi-page BC SMS
       message are received too quickly for Android to ack them; waiting for
       an ack from Android will lead to segments being dropped.
    */
    if ( ( i_ptr->sms_ack_info.cdma_ack_pending &&
        i_ptr->sms_ack_info.cdma_ack_needed ) || ( QMI_RIL_EME_CBM_ACTIVE == qmi_ril_nwr_get_eme_cbm() ) )
    {
      QCRIL_LOG_DEBUG("Ignoring CDMA SMS Message. Waiting for RIL to ack a previous SMS or waiting for RIL to Exit from ECBM mode.");
      is_processing_ok = FALSE;
    }

    if (is_processing_ok)
    {
      if (format == WMS_MESSAGE_FORMAT_CDMA_V01)
      {
        /* Translate the CDMA SMS message from QMI format to RIL format. */
        if (qcril_sms_convert_mt_cdma_sms_to_RIL_format( &event_report_ind->transfer_route_mt_message,
                                                         (RIL_CDMA_SMS_Message *) buf_ptr ) == FALSE)
        {
          QCRIL_LOG_DEBUG("Failed to convert mt cdma sms to RIL format.");
          is_processing_ok = FALSE;
        }
      }
      else if (format == WMS_MESSAGE_FORMAT_MWI_V01)
      {
        cdma_sms_msg = (RIL_CDMA_SMS_Message *) buf_ptr;
        cdma_sms_msg->uTeleserviceID = 0x00040000;
        cdma_sms_msg->bIsServicePresent = FALSE;
        cdma_sms_msg->sAddress.number_of_digits = 0;
        cdma_sms_msg->sSubAddress.number_of_digits = 0;
        cdma_sms_msg->uBearerDataLen = 1;
        cdma_sms_msg->aBearerData[0] = event_report_ind->transfer_route_mt_message.data[0];
      }
    }

    if (is_processing_ok)
    {
      /* An MT SMS has been received.  An ack is pending from Android. */
      i_ptr->sms_ack_info.cdma_ack_pending = TRUE;

      /* Ack_indicator of 0x01 means no ack is needed. If no ack is needed, QCRIL will
         drop the ack when it is received from the Android SMS application. */
      i_ptr->sms_ack_info.cdma_ack_needed = !event_report_ind->transfer_route_mt_message.ack_indicator;

      i_ptr->sms_ack_info.cdma_transaction_id = event_report_ind->transfer_route_mt_message.transaction_id;

      if (event_report_ind->sms_on_ims_valid &&
          event_report_ind->sms_on_ims)
      {
        i_ptr->sms_ack_info.cdma_send_ack_on_ims = TRUE;
      }
      else
      {
        i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;
      }

      // ack expiry tmr
      if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
      {
        qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
        qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
      }

      QCRIL_LOG_DEBUG("Considering timeout for ack/nack for MT SMS, enabled %d, timer window %d",
                      qmi_ril_sms_mt_expiry_window_len_set,
                      qmi_ril_sms_mt_expiry_window_len);
      if ( qmi_ril_sms_mt_expiry_window_len_set )
      {
        ack_expry_tmr_window.tv_sec  = qmi_ril_sms_mt_expiry_window_len;
        ack_expry_tmr_window.tv_usec = QMI_RIL_ZERO;

        qcril_setup_timed_callback(   QCRIL_DEFAULT_INSTANCE_ID,
                                      QCRIL_DEFAULT_MODEM_ID,
                                      qcril_sms_mt_transaction_expired,
                                      &ack_expry_tmr_window,
                                      &qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
      }

      qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, &unsol_resp );
      unsol_resp.resp_pkt = ( void * ) buf_ptr;
      unsol_resp.resp_len = sizeof( RIL_CDMA_SMS_Message );
      qcril_send_unsol_response( &unsol_resp );
    }

    qcril_free( buf_ptr );
  }

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_process_mt_cdma_sms */

/*=========================================================================
  FUNCTION:  qcril_sms_mt_transaction_expired

===========================================================================*/
/*!
    @brief
    Performs MT SMS transaction cleanup when ATEL posts no
    ack or nack within prespecified window after CDMA MT SMS

    @return
    None
*/
/*=========================================================================*/

void qcril_sms_mt_transaction_expired(void * param)
{
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    QCRIL_MUTEX_LOCK( &qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info_mutex, "sms_ack_info_mutex" );

    QCRIL_LOG_INFO( "current CDMA ack pending %d, needed %d",
                    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_ack_pending,
                    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_ack_needed  );

    QCRIL_LOG_INFO( "current GW ack pending %d, needed %d",
                    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_ack_pending,
                    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_ack_needed  );

    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_ack_pending     = FALSE;
    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_ack_needed      = FALSE;
    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.cdma_send_ack_on_ims = FALSE;

    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_ack_pending     = FALSE;
    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_ack_needed      = FALSE;
    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info.gw_send_ack_on_ims = FALSE;

    qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr     = QMI_RIL_ZERO;

    QCRIL_MUTEX_UNLOCK( &qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].sms_ack_info_mutex, "sms_ack_info_mutex" );

    QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_mt_transaction_expired */


/*=========================================================================
  FUNCTION:  qcril_sms_clearing_cdma_ack
===========================================================================*/

void qcril_sms_clearing_cdma_ack()
{
    qcril_sms_struct_type *i_ptr;
    qcril_instance_id_e_type instance_id;
    char details[ 80 ];

    QCRIL_LOG_FUNC_ENTRY();
    instance_id = QCRIL_DEFAULT_INSTANCE_ID;
    i_ptr = &qcril_sms[ instance_id ];

    QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
    QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

    QCRIL_LOG_INFO( "current CDMA ack pending %d, needed %d",
                    i_ptr->sms_ack_info.cdma_ack_pending,
                    i_ptr->sms_ack_info.cdma_ack_needed  );

    i_ptr->sms_ack_info.cdma_ack_pending     = FALSE;
    i_ptr->sms_ack_info.cdma_ack_needed      = FALSE;
    i_ptr->sms_ack_info.cdma_send_ack_on_ims = FALSE;

    QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_sms_convert_mt_gw_sms_to_RIL_format

===========================================================================*/
/*!
    @brief
    Translates a GW SMS message from QMI format to RIL format.

    @return
    TRUE if the MT GW SMS was successfully converted to RIL format
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_convert_mt_gw_sms_to_RIL_format
(
  wms_event_report_ind_msg_v01 * event_report_ind,
  char *buf_ptr
)
{
  uint32 hex_pdu_pos = 0;
  int ret_value = FALSE;

  QCRIL_LOG_FUNC_ENTRY();
  if (buf_ptr != NULL)
  {
    if (event_report_ind->mt_message_smsc_address_valid)
    {
      /* The SMSC Address was included.  Translate it to ASCII hex format. */

      /* Translate the length of the SMSC address to an ASCII hex char. */
      qcril_sms_byte_to_hex( (byte *) &event_report_ind->mt_message_smsc_address.data_len,
                             &buf_ptr[ hex_pdu_pos ],
                             1 );
      /* The length is 1 byte long; after conversion to ASCII hex, it is 2 bytes. */
      hex_pdu_pos = 2;

      /* Translate the SMSC address to ASCII hex char format */
      qcril_sms_byte_to_hex( event_report_ind->mt_message_smsc_address.data,
                             &buf_ptr[ hex_pdu_pos ],
                             event_report_ind->mt_message_smsc_address.data_len );

      /* Advance to the end of the SMSC address in the hex pdu */
      hex_pdu_pos += event_report_ind->mt_message_smsc_address.data_len * 2;
    }
    else
    {
      /* The SMSC Address was not included.  Set the length of the SMSC address to 0. */
      buf_ptr[0] = '0';
      buf_ptr[1] = '0';

      /* The length of the SMSC address is 2 bytes in ASCII hex */
      hex_pdu_pos = 2;
    }

    /* Translate the SMS message into ASCII hex format */
    qcril_sms_byte_to_hex( event_report_ind->transfer_route_mt_message.data,
                           &buf_ptr[ hex_pdu_pos ],
                           event_report_ind->transfer_route_mt_message.data_len );

    hex_pdu_pos += event_report_ind->transfer_route_mt_message.data_len * 2;

    /* Terminate the ASCII hex string with a null character */
    buf_ptr[ hex_pdu_pos ] = '\0';
    ret_value = TRUE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret_value);
  return ret_value;

} /* qcril_sms_convert_mt_gw_sms_to_RIL_format */


/*=========================================================================
  FUNCTION:  qcril_sms_process_mt_gw_sms

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a GW SMS message or Status Report
    is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_mt_gw_sms
(
  qcril_instance_id_e_type instance_id,
  wms_event_report_ind_msg_v01 * event_report_ind
)
{
  qcril_sms_struct_type *i_ptr;
  boolean is_processing_ok = TRUE;
  char *buf_ptr;
  char details[ 80 ];
  qcril_unsol_resp_params_type unsol_resp;

  struct timeval ack_expry_tmr_window = { 30 , 0 }; // 30 seconds

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_FUNC_ENTRY();
  i_ptr = &qcril_sms[ instance_id ];

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( details, sizeof( details ), "qcril_sms[%d].sms_ack_info_mutex", instance_id );
  QCRIL_MUTEX_LOCK( &i_ptr->sms_ack_info_mutex, details );

  /*-----------------------------------------------------------------------*/

  /* Allocate buffer to decode new GW SMS */
  buf_ptr = qcril_malloc( QCRIL_SMS_BUF_MAX_SIZE );
  if ( buf_ptr == NULL )
  {
    QCRIL_LOG_ERROR("Failed to allocate buffer to decode new GW SMS.");
  }
  else
  {
    /* According to ril.h, we are to drop new SMS Messages until the previous
       message has been acked.  Note that the network will not send any new
       messages until the previous one has been acked; so the messages we are
       dropping here should all be duplicates.

       We only drop new SMS messages if an ack is needed.  There are some
       messages in the category of "no ack needed" which need to be sent to
       Android immediately.  For example, the segments of a multi-page BC SMS
       message are received too quickly for Android to ack them; waiting for
       an ack from Android will lead to segments being dropped.
    */
    if ( i_ptr->sms_ack_info.gw_ack_pending &&
        i_ptr->sms_ack_info.gw_ack_needed )
    {
      QCRIL_LOG_DEBUG("Ignoring GW SMS Message or Status Report. Waiting for RIL to ack a previous SMS.");
      is_processing_ok = FALSE;
    }

    if (is_processing_ok)
    {
      /* Convert the MT GW SMS message to RIL format. */
      if ( qcril_sms_convert_mt_gw_sms_to_RIL_format( event_report_ind,
                                                      buf_ptr ) == FALSE )
      {
        is_processing_ok = FALSE;
      }
    }

    if (is_processing_ok)
    {
      /* An MT SMS has been received.  An ack is pending from Android. */
      i_ptr->sms_ack_info.gw_ack_pending = TRUE;

      /* Ack_indicator of 0x01 means no ack is needed. If no ack is needed, QCRIL will
         drop the ack when it is received from the Android SMS application. */
      i_ptr->sms_ack_info.gw_ack_needed = !event_report_ind->transfer_route_mt_message.ack_indicator;

      i_ptr->sms_ack_info.gw_transaction_id = event_report_ind->transfer_route_mt_message.transaction_id;

      if (event_report_ind->sms_on_ims_valid &&
          event_report_ind->sms_on_ims)
      {
        i_ptr->sms_ack_info.gw_send_ack_on_ims = TRUE;
      }
      else
      {
        i_ptr->sms_ack_info.gw_send_ack_on_ims = FALSE;
      }

      // ack expiry tmr
      if ( QMI_RIL_ZERO != qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr )
      {
        qcril_cancel_timed_callback( (void*)(uintptr_t)qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
        qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr = QMI_RIL_ZERO;
      }

      QCRIL_LOG_DEBUG("Considering timeout for ack/nack for MT SMS, enabled %d, timer window %d",
                      qmi_ril_sms_mt_expiry_window_len_set,
                      qmi_ril_sms_mt_expiry_window_len);
      if ( qmi_ril_sms_mt_expiry_window_len_set )
      {
        ack_expry_tmr_window.tv_sec  = qmi_ril_sms_mt_expiry_window_len;
        ack_expry_tmr_window.tv_usec = QMI_RIL_ZERO;

        qcril_setup_timed_callback(   QCRIL_DEFAULT_INSTANCE_ID,
                                      QCRIL_DEFAULT_MODEM_ID,
                                      qcril_sms_mt_transaction_expired,
                                      &ack_expry_tmr_window,
                                      &qcril_sms[QCRIL_DEFAULT_INSTANCE_ID].mt_pending_ack_expry_tmr );
      }

      /* Determine whether to send the indication for Status Report or New SMS */
      if (qcril_sms_check_if_gw_msg_is_status_report(&event_report_ind->transfer_route_mt_message))
      {
        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, &unsol_resp );
      }
      else
      {
        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_SMS, &unsol_resp );
      }

      unsol_resp.resp_pkt = ( void * ) buf_ptr;
      unsol_resp.resp_len = strlen( buf_ptr );
      qcril_send_unsol_response( &unsol_resp );
    }

    qcril_free( buf_ptr );
  }

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_UNLOCK( &i_ptr->sms_ack_info_mutex, details );
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_process_mt_gw_sms */


/*=========================================================================
  FUNCTION:  qcril_sms_request_raw_read

===========================================================================*/
/*!
    @brief
    Function for sending a request for reading message stored in NV

    @return
    None
*/
/*=========================================================================*/

void qcril_sms_request_raw_read
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  wms_raw_read_req_msg_v01 *wms_raw_read_req_msg_ptr;
  wms_raw_read_resp_msg_v01 wms_raw_read_resp_msg;
  qmi_client_error_type qmi_client_error;
  char *buf_ptr;
  qcril_unsol_resp_params_type unsol_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_req_res = RIL_E_SUCCESS;
  RIL_CDMA_SMS_Message * cdma_sms_msg;

  QCRIL_LOG_FUNC_ENTRY();

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  wms_raw_read_req_msg_ptr = ( wms_raw_read_req_msg_v01 * ) params_ptr->data;
  QCRIL_NOTUSED( ret_ptr );

  if ( wms_raw_read_req_msg_ptr != NULL )
  {

    // Check for WMS SERVICE READY
    if ((wms_raw_read_req_msg_ptr->message_mode != WMS_MESSAGE_MODE_GW_V01) &&
        (wms_raw_read_req_msg_ptr->message_mode != WMS_MESSAGE_MODE_CDMA_V01))
    {
       // Inavlid message mode - while its an optional TLV this function
       // is only called internaly w/a valid message mode
       QCRIL_LOG_ERROR("Raw Read Req Failed w/invalid message mode %d",
                        wms_raw_read_req_msg_ptr->message_mode);
       QCRIL_LOG_FUNC_RETURN();
       return;
    }

    memset(&wms_raw_read_resp_msg,0,sizeof(wms_raw_read_resp_msg));

    QCRIL_LOG_INFO("MSG PARAMS %d %d %d",
                   wms_raw_read_req_msg_ptr->message_memory_storage_identification.storage_index,
                   wms_raw_read_req_msg_ptr->message_memory_storage_identification.storage_type,
                   wms_raw_read_req_msg_ptr->message_mode);

    qmi_client_error = qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_WMS,
                                                      QMI_WMS_RAW_READ_REQ_V01,
                                                      wms_raw_read_req_msg_ptr,
                                                      sizeof(*wms_raw_read_req_msg_ptr),
                                                      &wms_raw_read_resp_msg,
                                                      sizeof(wms_raw_read_resp_msg),
                                                      QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT );

    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, &wms_raw_read_resp_msg.resp );

    if ( RIL_E_SUCCESS != ril_req_res )
    {
      QCRIL_LOG_ERROR("Raw Read Req Msg Send Failed, err %d", (int)ril_req_res);
    }
    else
    {
      QCRIL_LOG_INFO("Raw Read Request succeeded");

      /* Allocate buffer to decode new CDMA SMS */
      buf_ptr = qcril_malloc( sizeof( RIL_CDMA_SMS_Message ) );
      if ( buf_ptr == NULL )
      {
        QCRIL_LOG_ERROR("Failed to allocate buffer to decode new CDMA SMS.");
      }
      else
      {
        if (wms_raw_read_resp_msg.raw_message_data.format == WMS_MESSAGE_FORMAT_MWI_V01)
        {
          cdma_sms_msg = (RIL_CDMA_SMS_Message *) buf_ptr;
          cdma_sms_msg->uTeleserviceID = 0x00040000;
          cdma_sms_msg->bIsServicePresent = FALSE;
          cdma_sms_msg->sAddress.number_of_digits = 0;
          cdma_sms_msg->sSubAddress.number_of_digits = 0;
          cdma_sms_msg->uBearerDataLen = 1;
          cdma_sms_msg->aBearerData[0] = wms_raw_read_resp_msg.raw_message_data.data[0];

          qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, &unsol_resp );
          unsol_resp.resp_pkt = ( void * ) buf_ptr;
          unsol_resp.resp_len = sizeof( RIL_CDMA_SMS_Message );
          qcril_send_unsol_response( &unsol_resp );
        }
        else if (wms_raw_read_resp_msg.raw_message_data.format == WMS_MESSAGE_FORMAT_CDMA_V01)
        {
          if (qcril_sms_convert_mt_sms_qmi_to_ril(wms_raw_read_resp_msg.raw_message_data.data,
                                                  wms_raw_read_resp_msg.raw_message_data.data_len,
                                                  (RIL_CDMA_SMS_Message *) buf_ptr) == TRUE)
          {
            qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, &unsol_resp );
            unsol_resp.resp_pkt = ( void * ) buf_ptr;
            unsol_resp.resp_len = sizeof( RIL_CDMA_SMS_Message );
            qcril_send_unsol_response( &unsol_resp );
          }
          else
          {
            QCRIL_LOG_ERROR("failed to convert SMS to RIL format.");
          }
        }
        else
        {
          QCRIL_LOG_ERROR("unexpected format.");
        }
        qcril_free(buf_ptr);
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN();

} /*qcril_sms_request_raw_read*/


/*=========================================================================
  FUNCTION:  qcril_sms_process_event_report_ind

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a QMI_WMS_EVENT_REPORT_IND indication
    is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_event_report_ind
(
  wms_event_report_ind_msg_v01 * event_report_ind,
  qcril_instance_id_e_type instance_id
)
{
  int index_on_sim;
  uint8 *data;
  qcril_unsol_resp_params_type unsol_resp;
  uint8 coding_scheme;
  wms_call_control_modified_info_type_v01 src_wms_call_control_alpha_info;
  wms_call_control_modified_info_type_v01 wms_call_control_alpha_info;
  char buf_str[WMS_ALPHA_ID_LENGTH_MAX_V01 + 2];
  qcril_unsol_resp_params_type unsol_resp1;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();
  memset(&src_wms_call_control_alpha_info, 0, sizeof(src_wms_call_control_alpha_info));
  memset(&wms_call_control_alpha_info, 0, sizeof(wms_call_control_alpha_info));

  if( event_report_ind != NULL)
  {
  QCRIL_LOG_INFO( ".. transfer_route_mt_message_valid %d", (int) event_report_ind->transfer_route_mt_message_valid );
  if (event_report_ind->transfer_route_mt_message_valid)
  {
    QCRIL_LOG_INFO( ".. transfer_route_mt_message.format %d", (int) event_report_ind->transfer_route_mt_message.format );
    /* MT CDMA SMS */
    if ((event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_CDMA_V01) ||
        (event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_MWI_V01))
    {
      qcril_sms_process_mt_cdma_sms( instance_id,
                                     event_report_ind,
                                     event_report_ind->transfer_route_mt_message.format);
    }
    /* MT GW SMS */
    else if (event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_GW_PP_V01)
    {
      qcril_sms_process_mt_gw_sms( instance_id, event_report_ind );
    }
    /* GW SMS Broadcast */
    else if (event_report_ind->transfer_route_mt_message.format == WMS_MESSAGE_FORMAT_GW_BC_V01)
    {
      /* It is a broadcast SMS.  Maximum size is 88 bytes long. */
      QCRIL_LOG_VERBOSE("GSM Broadcast SMS Message");

      data = event_report_ind->transfer_route_mt_message.data;

      qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, &unsol_resp );
      unsol_resp.resp_pkt = ( void * ) event_report_ind->transfer_route_mt_message.data;
      unsol_resp.resp_len = event_report_ind->transfer_route_mt_message.data_len;
      qcril_send_unsol_response( &unsol_resp );
    }
  }
  /* SMS on SIM */
  else if ((event_report_ind->mt_message_valid) &&
           (event_report_ind->message_mode_valid))
  {
    if (event_report_ind->mt_message.storage_type == WMS_STORAGE_TYPE_UIM_V01)
    {
       if (event_report_ind->message_mode == WMS_MESSAGE_MODE_GW_V01)
       {
         /* Android uses the GSDI index, since it calls RIL_REQUEST_SIM_IO.
            The GSDI index is the QMI index plus 1. */
         index_on_sim = event_report_ind->mt_message.storage_index + 1;

         /* Notify RIL we received a class 2 message (new SMS on SIM) */
         qcril_default_unsol_resp_params( instance_id,
                       (int) RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, &unsol_resp );
         unsol_resp.resp_pkt = (void *) &index_on_sim;
         unsol_resp.resp_len = sizeof( int );
         qcril_send_unsol_response( &unsol_resp );
       }
       else if (event_report_ind->message_mode == WMS_MESSAGE_MODE_CDMA_V01)
       {
          wms_raw_read_req_msg_v01 read_req;

          read_req.message_mode_valid = TRUE;
          read_req.message_mode = event_report_ind->message_mode;
          read_req.message_memory_storage_identification.storage_type =
          event_report_ind->mt_message.storage_type;
          read_req.message_memory_storage_identification.storage_index =
          event_report_ind->mt_message.storage_index;
          read_req.sms_on_ims_valid = event_report_ind->sms_on_ims_valid;
          read_req.sms_on_ims = event_report_ind->sms_on_ims;
          QCRIL_LOG_INFO("Fetching MT CDMA SMS on SIM at storage index %d",
                          event_report_ind->mt_message.storage_index);
          qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                             QCRIL_DATA_ON_STACK, QCRIL_EVT_SMS_RAW_READ,
                             (void *) &read_req, sizeof( read_req ),
                             (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
       }
       else {
          QCRIL_LOG_ERROR("Invalid WMS message mode %d, ignoring",
                           event_report_ind->message_mode);
       }
    }
    else if ((event_report_ind->mt_message.storage_type == WMS_STORAGE_TYPE_NV_V01) &&
             (event_report_ind->message_mode == WMS_MESSAGE_MODE_CDMA_V01))
    {
      wms_raw_read_req_msg_v01 read_req;

      read_req.message_mode_valid = TRUE;
      read_req.message_mode = event_report_ind->message_mode;
      read_req.message_memory_storage_identification.storage_type =
      event_report_ind->mt_message.storage_type;
      read_req.message_memory_storage_identification.storage_index =
      event_report_ind->mt_message.storage_index;
      read_req.sms_on_ims_valid = event_report_ind->sms_on_ims_valid;
      read_req.sms_on_ims = event_report_ind->sms_on_ims;
      QCRIL_LOG_INFO("storage index %d", event_report_ind->mt_message.storage_index);
      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                         QCRIL_DATA_ON_STACK, QCRIL_EVT_SMS_RAW_READ, (void *) &read_req,
                         sizeof( read_req ), (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
  }
  /* ETWS message */
  else if (event_report_ind->etws_message_valid)
  {
    qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, &unsol_resp );
    unsol_resp.resp_pkt = ( void * ) event_report_ind->etws_message.data;
    unsol_resp.resp_len = event_report_ind->etws_message.data_len;
    qcril_send_unsol_response( &unsol_resp );
  }
  /* Alpha Info */
  else if( ( TRUE == event_report_ind->call_control_info_valid ) &&
    ( event_report_ind->call_control_info.alpha_id_len > 0 ) )
  {
    memset(buf_str, 0 , WMS_ALPHA_ID_LENGTH_MAX_V01 + 2);
    coding_scheme = event_report_ind->call_control_info.alpha_id[0];
    if( coding_scheme == 0x80 || coding_scheme == 0x81 || coding_scheme == 0x82 ) //UCS2
    {
        src_wms_call_control_alpha_info.alpha_id_len = event_report_ind->call_control_info.alpha_id_len;
        memcpy(src_wms_call_control_alpha_info.alpha_id,event_report_ind->call_control_info.alpha_id,WMS_ALPHA_ID_LENGTH_MAX_V01);
        QCRIL_LOG_INFO("Coding scheme is %x, ucs2 data",coding_scheme);
        qcril_qmi_sms_transfer_sim_ucs2_alpha_to_std_ucs2_alpha ( &src_wms_call_control_alpha_info, &wms_call_control_alpha_info);
        qcril_cm_ss_convert_ussd_string_to_utf8(  QCRIL_QMI_VOICE_USSD_DCS_UCS2,
                                                wms_call_control_alpha_info.alpha_id_len,
                                                wms_call_control_alpha_info.alpha_id,
                                                buf_str );
    }
    else //gsm8
    {
        QCRIL_LOG_INFO("Coding scheme is %x, gsm8 data",coding_scheme);
        if( event_report_ind->call_control_info.alpha_id < WMS_ALPHA_ID_LENGTH_MAX_V01 )
        {
            qcril_cm_ss_convert_gsm8bit_alpha_string_to_utf8( (char*) event_report_ind->call_control_info.alpha_id,
                                                            event_report_ind->call_control_info.alpha_id_len,
                                                            buf_str );
        }
    }

    if ( *buf_str )
    {
        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_STK_CC_ALPHA_NOTIFY, &unsol_resp1 );
        unsol_resp1.resp_pkt    = (void*)buf_str;
        unsol_resp1.resp_len    = sizeof( buf_str );

        qcril_send_unsol_response( &unsol_resp1 );
    }
  }

  }

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_sms_process_event_report_ind */


/*=========================================================================
  FUNCTION:  qcril_sms_process_memory_full_ind

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a QMI_WMS_MEMORY_FULL_IND indication
    is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_memory_full_ind
(
  wms_memory_full_ind_msg_v01 * memory_full_ind,
  qcril_instance_id_e_type instance_id
)
{
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  if (memory_full_ind != NULL && memory_full_ind->memory_full_info.storage_type == WMS_STORAGE_TYPE_UIM_V01)
  {
    if (memory_full_ind->memory_full_info.message_mode == WMS_MESSAGE_MODE_CDMA_V01)
    {
      QCRIL_LOG_DEBUG("EVENT_MEMORY_FULL: mem store RUIM");
      qcril_default_unsol_resp_params( instance_id,
                                       (int)RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL,
                                       &unsol_resp );
      qcril_send_unsol_response( &unsol_resp );
    }
    else if (memory_full_ind->memory_full_info.message_mode == WMS_MESSAGE_MODE_GW_V01)
    {
      QCRIL_LOG_DEBUG("EVENT_MEMORY_FULL: mem store SIM");
      qcril_default_unsol_resp_params( instance_id,
                                       (int)RIL_UNSOL_SIM_SMS_STORAGE_FULL,
                                       &unsol_resp );
      qcril_send_unsol_response( &unsol_resp );
    }
  }

} /* qcril_sms_process_memory_full_ind */


/*=========================================================================
  FUNCTION:  qcril_sms_process_transport_nw_reg_info_ind

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a QMI_WMS_TRANSPORT_NW_REG_INFO_IND
    indication is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_transport_nw_reg_info_ind
(
  wms_transport_nw_reg_info_ind_msg_v01 * transport_nw_reg_info_ind,
  qcril_instance_id_e_type instance_id
)
{
  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_sms_report_unsol_ims_state_change();

  qcril_qmi_nas_set_registered_on_ims( (WMS_TRANSPORT_NW_REG_STATUS_FULL_SERVICE_V01 == transport_nw_reg_info_ind->transport_nw_reg_status) ? TRUE : FALSE );

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_process_transport_nw_reg_info_ind */

/*=========================================================================
  FUNCTION:  qcril_sms_process_transport_layer_info_ind

===========================================================================*/
/*!
    @brief
    Performs processing necessary when a QMI_WMS_TRANSPORT_LAYER_INFO_IND
    indication is received.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_process_transport_layer_info_ind
(
  wms_transport_layer_info_ind_msg_v01 * transport_layer_info_ind,
  qcril_instance_id_e_type instance_id
)
{
  qcril_sms_struct_type *i_ptr;

  QCRIL_LOG_FUNC_ENTRY();

  i_ptr = &qcril_sms[ QCRIL_DEFAULT_INSTANCE_ID ];
  QCRIL_MUTEX_LOCK( &i_ptr->transport_layer_info_mutex, NULL );
  QCRIL_LOG_INFO( "transport layer reg info value %d", transport_layer_info_ind->registered_ind);
  i_ptr->transport_layer_info.registered_ind_valid = TRUE;
  i_ptr->transport_layer_info.registered_ind = transport_layer_info_ind->registered_ind;
  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );

  qcril_qmi_sms_report_unsol_ims_state_change();

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_sms_process_transport_layer_info_ind */

//=========================================================================
// FUNCTION: qcril_sms_process_service_ready_ind
//
// DESCRIPTION:
// set wms ready state and relay state change to telephony
//
// RETURN: None
//=========================================================================
void qcril_sms_process_service_ready_ind
(
  wms_service_ready_ind_msg_v01* service_msg
)
{
  QCRIL_LOG_FUNC_ENTRY();

  if (!service_msg) {
     QCRIL_LOG_FATAL("service_msg was NULL!");
     QCRIL_ASSERT(0);
     return;
  }

  // set global service state and log new state
  set_wms_service_state(service_msg->ready_status);

  // send indication to telephony via oem hook, rilwms
  unsigned char data = (unsigned char)(service_msg->ready_status);
  qcril_hook_unsol_response(QCRIL_DEFAULT_INSTANCE_ID,
                            QCRIL_EVT_HOOK_UNSOL_WMS_READY,
                            (char*)&data, 1);


  QCRIL_LOG_FUNC_RETURN();
}

//=========================================================================
// FUNCTION: qcril_sms_process_transport_layer_mwi_ind
//
// DESCRIPTION:
// Message waiting indication handler
//
// RETURN: None
//=========================================================================
void qcril_sms_process_transport_layer_mwi_ind
(
  wms_transport_layer_mwi_ind_msg_v01 *mwi_msg
)
{
  Ims__Mwi ims_mwi = IMS__MWI__INIT;
  Ims__MwiMessageSummary *mwimsgsummary = NULL;
  Ims__MwiMessageDetails *mwimsgdetails = NULL;
  boolean send_unsol = FALSE;
  int i = 0;

  QCRIL_LOG_FUNC_ENTRY();

  if (!mwi_msg) {
     QCRIL_LOG_FATAL("mwi_msg was NULL!");
     QCRIL_ASSERT(0);
     return;
  }

  do
  {
    ims_mwi.n_mwimsgsummary = mwi_msg->message_waiting_summary_info_len;
    QCRIL_LOG_INFO( "n_mwimsgsummary = %d", ims_mwi.n_mwimsgsummary);
    if (ims_mwi.n_mwimsgsummary > 0)
    {
      ims_mwi.mwimsgsummary = qcril_malloc(sizeof (Ims__MwiMessageSummary *) *
                                             ims_mwi.n_mwimsgsummary);
      mwimsgsummary         = qcril_malloc(sizeof (Ims__MwiMessageSummary) *
                                             ims_mwi.n_mwimsgsummary);
      if (ims_mwi.mwimsgsummary == NULL || mwimsgsummary == NULL)
      {
        QCRIL_LOG_FATAL("malloc failed");
        break;
      }
      else
      {
        for ( i = 0; i < ims_mwi.n_mwimsgsummary; i++)
        {
          ims_mwi.mwimsgsummary[i] = &mwimsgsummary[i];
          Ims__MwiMessageSummary mwimsgsummary_tmp = IMS__MWI_MESSAGE_SUMMARY__INIT;
          memcpy(&(mwimsgsummary[i]), &mwimsgsummary_tmp, sizeof(Ims__MwiMessageSummary));

          mwimsgsummary[i].has_messagetype = TRUE;
          mwimsgsummary[i].messagetype     = qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type(
                  mwi_msg->message_waiting_summary_info[i].message_type);
          mwimsgsummary[i].has_newmessage  = TRUE;
          mwimsgsummary[i].newmessage      = mwi_msg->message_waiting_summary_info[i].new_msg;
          mwimsgsummary[i].has_oldmessage  = TRUE;
          mwimsgsummary[i].oldmessage      = mwi_msg->message_waiting_summary_info[i].old_msg;
          mwimsgsummary[i].has_newurgent   = TRUE;
          mwimsgsummary[i].newurgent       = mwi_msg->message_waiting_summary_info[i].new_urgent;
          mwimsgsummary[i].has_oldurgent   = TRUE;
          mwimsgsummary[i].oldurgent       = mwi_msg->message_waiting_summary_info[i].old_urgent;
        }
      }
    }
    ims_mwi.ueaddress = qmi_ril_util_str_clone(mwi_msg->UE_address);
    if (mwi_msg->message_waiting_detail_info_valid &&
        mwi_msg->message_waiting_detail_info_len > 0)
    {
      ims_mwi.n_mwimsgdetail = mwi_msg->message_waiting_detail_info_len;
      ims_mwi.mwimsgdetail   = qcril_malloc(sizeof (Ims__MwiMessageDetails *) *
                                           ims_mwi.n_mwimsgdetail);
      mwimsgdetails          = qcril_malloc(sizeof (Ims__MwiMessageDetails) *
                                           ims_mwi.n_mwimsgdetail);

      if (ims_mwi.mwimsgdetail == NULL || mwimsgdetails == NULL)
      {
        QCRIL_LOG_FATAL("malloc failed");
        break;
      }
      else
      {
        QCRIL_LOG_INFO( "n_mwimsgdetail = %d", ims_mwi.n_mwimsgdetail);
        for ( i = 0; i < ims_mwi.n_mwimsgdetail; i++)
        {
          ims_mwi.mwimsgdetail[i] = &mwimsgdetails[i];
          Ims__MwiMessageDetails mwimsgdetail_tmp = IMS__MWI_MESSAGE_DETAILS__INIT;
          memcpy(&(mwimsgdetails[i]), &mwimsgdetail_tmp, sizeof(Ims__MwiMessageDetails));

          ims_mwi.mwimsgdetail[i]->toaddress = qmi_ril_util_str_clone(
                  mwi_msg->message_waiting_detail_info[i].to_address);
          ims_mwi.mwimsgdetail[i]->fromaddress = qmi_ril_util_str_clone(
                  mwi_msg->message_waiting_detail_info[i].from_address);
          ims_mwi.mwimsgdetail[i]->subject = qmi_ril_util_str_clone(
                  mwi_msg->message_waiting_detail_info[i].subject);
          ims_mwi.mwimsgdetail[i]->date = qmi_ril_util_str_clone(
                  mwi_msg->message_waiting_detail_info[i].date_time);
          ims_mwi.mwimsgdetail[i]->has_priority = TRUE;
          ims_mwi.mwimsgdetail[i]->priority = qcril_qmi_sms_map_qmi_mwi_priority_to_ims_priority(
                  mwi_msg->message_waiting_detail_info[i].priority);
          ims_mwi.mwimsgdetail[i]->messageid = qmi_ril_util_str_clone(
                  mwi_msg->message_waiting_detail_info[i].message_id);
          ims_mwi.mwimsgdetail[i]->has_messagetype = TRUE;
          ims_mwi.mwimsgdetail[i]->messagetype = qcril_qmi_sms_map_qmi_mwi_msg_type_to_ims_msg_type(
                  mwi_msg->message_waiting_detail_info[i].message_type);
        }
      }
    }
    send_unsol = TRUE;
  } while (0);

  if (send_unsol)
  {
    qcril_qmi_ims_socket_send(0,
            IMS__MSG_TYPE__UNSOL_RESPONSE,
            IMS__MSG_ID__UNSOL_MWI,
            IMS__ERROR__E_SUCCESS,
            &ims_mwi,
            sizeof (ims_mwi));
  }

  qcril_free(ims_mwi.mwimsgsummary);
  qcril_free(mwimsgsummary);
  qcril_free(ims_mwi.ueaddress);
  if (ims_mwi.mwimsgdetail)
  {
    for ( i = 0; i < ims_mwi.n_mwimsgdetail; i++)
    {
      qcril_free(ims_mwi.mwimsgdetail[i]->toaddress);
      qcril_free(ims_mwi.mwimsgdetail[i]->fromaddress);
      qcril_free(ims_mwi.mwimsgdetail[i]->subject);
      qcril_free(ims_mwi.mwimsgdetail[i]->date);
      qcril_free(ims_mwi.mwimsgdetail[i]->messageid);
    }
  }
  qcril_free(ims_mwi.mwimsgdetail);
  qcril_free(mwimsgdetails);

  QCRIL_LOG_FUNC_RETURN();
}

//=========================================================================
// FUNCTION: qcril_sms_post_ready_status_update
//
// DESCRIPTION:
// post current wms state to telephony
//
// RETURN: None
//=========================================================================
void qcril_sms_post_ready_status_update(void)
{
   unsigned char state;

   QCRIL_LOG_FUNC_ENTRY();

   if ( qcril_qmi_client_is_available() && QMI_RIL_SMS_SVC_FULLY_OPERATIONAL == qmi_ril_get_sms_svc_status() ) // see if SMS client initialization is in reasonable state
   {
      SMS_CACHE_LOCK;
      state = (unsigned char)wms_service_state;
      SMS_CACHE_UNLOCK;

      QCRIL_LOG_INFO("..posting wms_ready status %d (%s)", (int)state, sms_state_str(state) );

      qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID,
                                 QCRIL_EVT_HOOK_UNSOL_WMS_READY,
                                 (char*)&state, sizeof(state) );
   }

   QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

                   QMI UNSOL INDICATION HANDLER

===========================================================================*/

//============================================================================
// FUNCTION:  qcril_qmi_sms_unsol_ind_cb
//
// DESCRIPTION:
//  Handles QMI WMS indications.
//
// RETURN: None
//============================================================================
void qcril_qmi_sms_unsol_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
)
{
  qmi_ind_callback_type qmi_callback;

  QCRIL_LOG_FUNC_ENTRY();

  memset(&qmi_callback,0,sizeof(qmi_callback));
  qmi_callback.data_buf = qcril_malloc(ind_buf_len);

  if( qmi_callback.data_buf )
  {
    qmi_callback.user_handle = user_handle;
    qmi_callback.msg_id = msg_id;
    memcpy(qmi_callback.data_buf,ind_buf,ind_buf_len);
    qmi_callback.data_buf_len = ind_buf_len;
    qmi_callback.cb_data = ind_cb_data;

    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                   QCRIL_DEFAULT_MODEM_ID,
                   QCRIL_DATA_ON_STACK,
                   QCRIL_EVT_QMI_SMS_HANDLE_INDICATIONS,
                   (void*) &qmi_callback,
                   sizeof(qmi_callback),
                   (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  }
  else
  {
    QCRIL_LOG_FATAL("malloc failed");
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_sms_unsolicited_indication_cb_helper

===========================================================================*/
/*!
    @brief
    helper function for handling sms indication

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_sms_unsolicited_indication_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
   qcril_instance_id_e_type instance_id;
   void* decoded_payload = NULL;
   qmi_ind_callback_type *qmi_callback = (qmi_ind_callback_type*) params_ptr->data;

   QCRIL_LOG_FUNC_ENTRY();

   do
   {
      if( !qmi_callback )
      {
        QCRIL_LOG_ERROR("qmi_callback is NULL");
        QCRIL_ASSERT(0); // this is a noop in release build
        break;
      }
      QCRIL_LOG_INFO("msg_id (0x%04x) %s", qmi_callback->msg_id,
                qcril_sms_lookup_ind_name(qmi_callback->msg_id));

      qmi_ril_gen_operational_status_type op_status
                                            = qmi_ril_get_operational_status();

      if ( op_status != QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED )
      {
        QCRIL_LOG_INFO("operation state is restricted - %d,"
                     " ignoring QMI-WMS msg", (int) op_status );
        break;
      }

      decoded_payload = qcril_sms_decode_ind(qmi_callback->msg_id, qmi_callback->data_buf, qmi_callback->data_buf_len);
      if (!decoded_payload)
      {
        QCRIL_LOG_ERROR("Failed to decode sms unsol indication");
        break;
      }

      instance_id = QCRIL_DEFAULT_INSTANCE_ID;

      switch (qmi_callback->msg_id)
      {
        case QMI_WMS_EVENT_REPORT_IND_V01: {
           qcril_sms_process_event_report_ind(
                 (wms_event_report_ind_msg_v01*) decoded_payload, instance_id);
           break;
           }
        case QMI_WMS_MEMORY_FULL_IND_V01: {
           qcril_sms_process_memory_full_ind(
                  (wms_memory_full_ind_msg_v01*) decoded_payload, instance_id);
           break;
           }
        case QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01: {
           qcril_sms_process_transport_layer_info_ind(
              (wms_transport_layer_info_ind_msg_v01*) decoded_payload, instance_id);
           break;
           }
        case QMI_WMS_TRANSPORT_NW_REG_INFO_IND_V01: {
           qcril_sms_process_transport_nw_reg_info_ind(
              (wms_transport_nw_reg_info_ind_msg_v01*) decoded_payload, instance_id);
           break;
           }
        case QMI_WMS_SERVICE_READY_IND_V01: {
           qcril_sms_process_service_ready_ind(
                (wms_service_ready_ind_msg_v01*) decoded_payload);
           break;
           }
        case QMI_WMS_TRANSPORT_LAYER_MWI_IND_V01: {
           qcril_sms_process_transport_layer_mwi_ind(
                (wms_transport_layer_mwi_ind_msg_v01*) decoded_payload);
           break;
           }
        default: {
           QCRIL_LOG_INFO("Unexpected, ignoring QMI WMS indication 0x%04x",
                         qmi_callback->msg_id);
           break;
           }
      }

      qcril_free(decoded_payload);
   }while(FALSE);

   if( qmi_callback && qmi_callback->data_buf )
   {
      qcril_free(qmi_callback->data_buf);
   }
   QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

                                UTILITY FUNCTIONS

===========================================================================*/
/*===========================================================================

FUNCTION    qcril_sms_strip_quotes

DESCRIPTION
  Strips out quotes from the string that is wrapped in quotes.
  Resultant string will be placed in the out pointer.
  Gives an error if the string does not begin or end with a quote

DEPENDENCIES
  None

RETURN VALUE
  Boolean :
    TRUE : if successful in stripping the quotes out
    FALSE : if there is any error

SIDE EFFECTS
  None

===========================================================================*/
boolean qcril_sms_strip_quotes
(
  const char * in_ptr,   /* Pointer to the string to be processed  */
  char * out_ptr         /* Pointer to the resultant string buffer */
)
{
  int ret_value = FALSE;
  do
  {
  if (*in_ptr != '\0' )
  {
    if ( *in_ptr++ == '"' )
    {
      while ( *in_ptr != '"' && *in_ptr != '\0' )
      {
        *out_ptr++ = *in_ptr++;
      }
      /* Check to see if the string ends with a null */
      if ( *in_ptr == '\0' )
      {
        /* We got a string without ending quotes */
          ret_value = FALSE;
          break;
      }
      else
      {
        /* Everything is happy */
        *out_ptr = '\0';
          ret_value = TRUE;
          break;
      }
    }
    else
    {
      /* We got a string with out quotes */
        ret_value = FALSE;
        break;
    }
  }
  else
  {
      ret_value = FALSE;
      break;
  }
  }while(0);

  return ret_value;
} /* qcril_sms_strip_quotes */


/*=========================================================================
  FUNCTION:  qcril_sms_convert_smsc_address_to_qmi_format

===========================================================================*/
/*!
    @brief
    Converts an SMSC address from RIL format to QMI format.
    Note that the RIL format comes from the CSCA ATCOP command:
    "+358501234567",145

    @return
    TRUE if the SMSC address was successfully translated to QMI format.
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_convert_smsc_address_to_qmi_format
(
  const char * input_smsc_address_ptr,
  wms_set_smsc_address_req_msg_v01 * qmi_request
)
{
  char * asciiDigits = NULL;
  char * toa = NULL;
  boolean status = TRUE;
  char * ascii_digits_ptr = NULL;
  char * toa_ptr = NULL;

  if(input_smsc_address_ptr != NULL && qmi_request != NULL && strlen(input_smsc_address_ptr) != 0)
  {
  /* Allocate space for the SMSC address and SMSC type of address.  Leave
     a space for null terminator. */
  asciiDigits = (char *) qcril_malloc(strlen(input_smsc_address_ptr) + 1);
  toa = (char *) qcril_malloc(strlen(input_smsc_address_ptr) + 1);
  if (asciiDigits == NULL || toa == NULL)
  {
    QCRIL_LOG_ERROR("Unable to allocate buffer for SMSC address");
    status = FALSE;
  }
  else
  {
    ascii_digits_ptr = asciiDigits;
    toa_ptr = toa;

    /* Copy everything up to the first comma into the SMSC address */
    while ((*input_smsc_address_ptr != '\0') &&
           (*input_smsc_address_ptr != ','))
    {
      *ascii_digits_ptr++ = *input_smsc_address_ptr++;
    }
    *ascii_digits_ptr = '\0';

    if (*input_smsc_address_ptr == ',')
    {
      /* Increment past the comma */
      input_smsc_address_ptr++;
      while (*input_smsc_address_ptr != '\0')
      {
        /* Copy the rest of the string into the type of address */
        *toa_ptr++ = *input_smsc_address_ptr++;
      }
    }
    *toa_ptr = '\0';

    /* Strip the quotes from the SMSC address */
    if (!qcril_sms_strip_quotes(asciiDigits, asciiDigits))
    {
      QCRIL_LOG_ERROR("SMSC address either did not begin or end with quotes!");
      status = FALSE;
    }
    else if (strlen(asciiDigits) > WMS_ADDRESS_DIGIT_MAX_V01)
    {
      QCRIL_LOG_ERROR("Too many digits in SMSC address");
      status = FALSE;
    }
    else if (strlen(toa) > WMS_ADDRESS_TYPE_MAX_V01)
    {
      QCRIL_LOG_ERROR("Too many digits in SMSC address type");
      status = FALSE;
    }
    else if (strlen(asciiDigits) == 0)
    {
      QCRIL_LOG_ERROR("SMSC address is empty!");
      status = FALSE;
    }

    if (status)
    {
      /* Fill in the SMSC Address.  Note that the input needs to be NULL-terminated,
         so copy the NULL byte as well. */
      strlcpy(qmi_request->smsc_address_digits,
             asciiDigits,
             WMS_ADDRESS_DIGIT_MAX_V01 + 1);

      if (strlen(toa) > 0)
      {
        /* Include the SMSC Address Type TLV */
        qmi_request->smsc_address_type_valid = TRUE;

        strlcpy(qmi_request->smsc_address_type,
               toa,
               WMS_ADDRESS_TYPE_MAX_V01 + 1);
      }
    }
  }

  if (asciiDigits != NULL)
  {
    qcril_free( asciiDigits );
  }
  if (toa != NULL)
  {
    qcril_free( toa );
  }
  }
  else
  {
    status = FALSE;
  }

  return status;

} /* qcril_sms_convert_smsc_address_to_qmi_format */


/*=========================================================================
  FUNCTION:  qcril_sms_convert_smsc_address_to_ril_format

===========================================================================*/
/*!
    @brief
    Converts an SMSC address from QMI format to RIL format.
    Note that the RIL format comes from the CSCA ATCOP command:
    "+358501234567",145

    @return
    TRUE if the SMSC address was successfully translated to RIL format.
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_sms_convert_smsc_address_to_ril_format
(
  const wms_get_smsc_address_resp_msg_v01 * qmi_response,
  char * output_smsc_address_ptr
)
{
  /* Current position within the output SMSC address. */
  uint32 curr_pos;
  int ret_value = FALSE;

  if(qmi_response != NULL && output_smsc_address_ptr != NULL && qmi_response->smsc_address.smsc_address_digits_len != 0)
  {
  /* Add quote to the beginning of the SMSC address */
  output_smsc_address_ptr[0] = '"';
  curr_pos = 1;

  /* Copy the SMSC address */
  memcpy(&output_smsc_address_ptr[curr_pos],
         qmi_response->smsc_address.smsc_address_digits,
         qmi_response->smsc_address.smsc_address_digits_len);

  curr_pos += qmi_response->smsc_address.smsc_address_digits_len;

  /* Add a quote to the end of the SMSC address, and a comma */
  output_smsc_address_ptr[curr_pos++] = '"';
  output_smsc_address_ptr[curr_pos++] = ',';

  /* Copy the SMSC address type (3 digits) */
  memcpy(&output_smsc_address_ptr[curr_pos],
         qmi_response->smsc_address.smsc_address_type,
         3);

  output_smsc_address_ptr[curr_pos+3] = '\0';
    ret_value = TRUE;
  }

  return ret_value;

} /* qcril_sms_convert_smsc_address_to_ril_format */


/*===========================================================================

  FUNCTION:  qcril_sms_map_ril_tag_to_qmi_tag

===========================================================================*/
/*!
    @brief
    Convert the tag from RIL format to QMI WMS format

    @return
    None.
*/
/*=========================================================================*/
wms_message_tag_type_enum_v01 qcril_sms_map_ril_tag_to_qmi_tag
(
  int ril_tag
)
{
  uint8 qmi_tag;

  /*-----------------------------------------------------------------------*/

  switch ( ril_tag )
  {
    case 0:
      qmi_tag = WMS_TAG_TYPE_MT_NOT_READ_V01;
      break;

    case 1:
      qmi_tag = WMS_TAG_TYPE_MT_READ_V01;
      break;

    case 2:
      qmi_tag = WMS_TAG_TYPE_MO_NOT_SENT_V01;
      break;

    case 3:
      qmi_tag = WMS_TAG_TYPE_MO_SENT_V01;
      break;

    default:
      QCRIL_LOG_ERROR("Attempting to write an SMS with an unrecognized tag: %d", ril_tag);
      qmi_tag = 0xFF;
      break;
  }

  return qmi_tag;

} /* qcril_sms_map_ril_tag_to_qmi_tag */


/*===========================================================================

  FUNCTION:  qcril_sms_is_tag_mo

===========================================================================*/
/*!
    @brief
    Determine if the tag indicates the message is MO or MT.

    @return
    None.
*/
/*=========================================================================*/
boolean qcril_sms_is_tag_mo
(
  int ril_tag
)
{
  boolean is_tag_mo;

  /*-----------------------------------------------------------------------*/

  switch ( ril_tag )
  {
    case 0:
      is_tag_mo = FALSE;
      break;

    case 1:
      is_tag_mo = FALSE;
      break;

    case 2:
      is_tag_mo = TRUE;
      break;

    case 3:
      is_tag_mo = TRUE;
      break;

    default:
      QCRIL_LOG_ERROR("Attempting to write an SMS with an unrecognized tag: %d", ril_tag);
      is_tag_mo = TRUE;
      break;
  }

  return is_tag_mo;

} /* qcril_sms_is_tag_mo */


/*=========================================================================
  FUNCTION:  qcril_sms_lookup_cmd_name

===========================================================================*/
/*!
    @brief
    Look up the name of a QMI WMS command.

    @return
    The string representing the name of a QMI WMS command.
*/
/*=========================================================================*/
const char *qcril_sms_lookup_cmd_name
(
  unsigned long qmi_cmd
)
{
  switch (qmi_cmd)
  {
    case QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01:
      return "QMI_WMS_SET_PRIMARY_CLIENT_REQ";

    case QMI_WMS_SET_ROUTES_REQ_V01:
      return "QMI_WMS_SET_ROUTES_REQ";

    case QMI_WMS_SET_EVENT_REPORT_REQ_V01:
      return "QMI_WMS_SET_EVENT_REPORT_REQ";

    case QMI_WMS_RAW_SEND_REQ_V01:
      return "QMI_WMS_RAW_SEND_REQ";

    case QMI_WMS_SEND_ACK_REQ_V01:
      return "QMI_WMS_SEND_ACK_REQ";

    case QMI_WMS_RAW_WRITE_REQ_V01:
      return "QMI_WMS_RAW_WRITE_REQ";

    case QMI_WMS_MODIFY_TAG_REQ_V01:
      return "QMI_WMS_MODIFY_TAG_REQ";

    case QMI_WMS_DELETE_REQ_V01:
      return "QMI_WMS_DELETE_REQ";

    case QMI_WMS_GET_SMSC_ADDRESS_REQ_V01:
      return "QMI_WMS_GET_SMSC_ADDRESS_REQ";

    case QMI_WMS_SET_SMSC_ADDRESS_REQ_V01:
      return "QMI_WMS_SET_SMSC_ADDRESS_REQ";

    case QMI_WMS_SET_MEMORY_STATUS_REQ_V01:
      return "QMI_WMS_SET_MEMORY_STATUS_REQ";

    case QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01:
      return "QMI_WMS_SET_BROADCAST_ACTIVATION_REQ";

    case QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01:
      return "QMI_WMS_GET_BROADCAST_CONFIG_REQ";

    case QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01:
      return "QMI_WMS_SET_BROADCAST_CONFIG_REQ";

    case QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01:
      return "QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ";

    case QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01:
      return "QMI_WMS_GET_SERVICE_READY_STATUS_REQ";

    case QMI_WMS_GET_INDICATION_REGISTER_REQ_V01:
      return "QMI_WMS_GET_INDICATION_REGISTER_REQ";

    default:
      return "Unknown QMI WMS command";
  }

} /* qcril_sms_lookup_cmd_name */


/*=========================================================================
  FUNCTION:  qcril_sms_lookup_ind_name

===========================================================================*/
/*!
    @brief
    Look up the name of a QMI WMS indication.

    @return
    The string representing the name of a QMI WMS indication.
*/
/*=========================================================================*/
const char *qcril_sms_lookup_ind_name
(
  unsigned long qmi_ind
)
{
  switch (qmi_ind)
  {
    case QMI_WMS_EVENT_REPORT_IND_V01:
      return "QMI_WMS_EVENT_REPORT_IND";

    case QMI_WMS_MEMORY_FULL_IND_V01:
      return "QMI_WMS_MEMORY_FULL_IND";

    case QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01:
      return "QMI_WMS_TRANSPORT_LAYER_INFO_IND";

    case QMI_WMS_SERVICE_READY_IND_V01:
      return "QMI_WMS_SERVICE_READY_IND";

    case QMI_WMS_TRANSPORT_LAYER_MWI_IND_V01:
      return "QMI_WMS_TRANSPORT_LAYER_MWI_IND";

    default:
      return "Unknown QMI WMS indication";
  }
} /* qcril_sms_lookup_ind_name */


/*===========================================================================
MACRO bitsize

DESCRIPTION
   Computes size in bits of the specified data type.
===========================================================================*/
#define bitsize(type) (sizeof(type) * 8)

/*===========================================================================
MACRO copymask

DESCRIPTION
   Creates a mask of bits sized to the number of bits in the given type.
===========================================================================*/
#define copymask(type) ((0xffffffff) >> (32 - bitsize(type)))

/*===========================================================================
MACRO MASK

DESCRIPTION
   Masks the bits in data at the given offset for given number of width bits.
===========================================================================*/
#define MASK(width, offset, data) \
    /*lint -e701 shift left  of signed quantity  */  \
    /*lint -e702 shift right of signed quantity  */  \
    /*lint -e572 Excessive shift value           */  \
    /*lint -e573 Signed-unsigned mix with divide */  \
    /*lint -e506 Constant value boolean          */  \
    /*lint -e649 Sign fill during constant shift */  \
                                                     \
   (((width) == bitsize(data)) ? (data) :   \
   ((((copymask(data) << (bitsize(data) - ((width) % bitsize(data)))) & copymask(data)) >>  (offset)) & (data))) \
                     \
    /*lint +e701 */  \
    /*lint +e702 */  \
    /*lint +e572 */  \
    /*lint +e573 */  \
    /*lint +e506 */  \
    /*lint +e649 */

/*===========================================================================
MACRO MASK_AND_SHIFT

DESCRIPTION
   Same as the macro MASK except also shifts the data in the result by the
   given number of shift bits.
===========================================================================*/
#define MASK_AND_SHIFT(width, offset, shift, data)  \
    /*lint -e504 Unusual shifter value */  \
                  ((((signed) (shift)) < 0) ?       \
                    MASK((width), (offset), (data)) << -(shift) :  \
                    MASK((width), (offset), (data)) >>  (((unsigned) (shift)))) \
    /*lint +e504 */

/*===========================================================================
MACRO MASK_B

DESCRIPTION
   Masks the number of bits give by length starting at the given offset.
   Unlike MASK and MASK_AND_SHIFT, this macro only creates that mask, it
   does not operate on the data buffer.
===========================================================================*/
#define MASK_B(offset, len) \
  ((0xff >> offset) & (0xff << (8 - (offset + len))))


/*============================================================================

FUNCTION B_PACKB

DESCRIPTION
  Packs the given byte into the destination at the given offset for the
  given number of length bits

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

============================================================================*/
void b_packb(
   byte src,
   byte dst[],
   word pos,
   word len
)
{
   word   t_pos = pos % 8;
   word   bits  = 8 - t_pos;

   dst += (pos+len-1)/8;

   if ( bits >= len )
   {
       *dst &= (byte) ~MASK_B(t_pos, len);
       *dst |= (byte) (MASK_B(t_pos, len) & (src << (bits - len)));
   }
   else /* len > bits */
   {
       dst--;
       *dst &= (byte) ~MASK_B(t_pos, bits);
       *dst |= (byte) (MASK_B(t_pos, bits) & (src >> (len - bits)));

       dst++;
       *dst &= (byte) ~MASK_B(0, (len - bits));
       *dst |= (byte) (MASK_B(0, (len - bits)) & (src << (8 - (len - bits))));
   }
} /* END b_packb */

/*============================================================================

FUNCTION B_PACKW

DESCRIPTION
  Packs the given word into the destination at the given offset for the
  given number of length bits

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

============================================================================*/
void b_packw(
   word src,
   byte dst[],
   word pos,
   word len
)
{
   int bits, start;
   byte   mask;

   dst += (len+pos-1)/8;        /* point to last byte to be written */
   pos  = (len+pos-1)%8;        /* index of last bit to be written */

   if (len > pos)  /* if we are filling all of the left part of the byte */
   {
     start = 0;
   }
   else            /* There are going to be untouched bits at left of
                   ** destination byte.                                   */
   {
     start = (pos+1) - len;
   }
   bits = (pos - start) + 1;    /* # of bits to be written in this byte */

   *dst &= (byte) ~MASK_B(start,bits);  /* clear the bits to be written */

   *dst |= (byte) (   ( src << (7 - pos) )    /* left-shift src to line up */
                    & MASK_B(start, bits) );  /* only touch desired bits */

   dst--;                /* back up one byte */
   src >>= bits;         /* get rid of bits we've consumed already */

   if(len > bits)        /* if we need to write into other bytes */
   {
     len -= bits;        /* compute remaining length  */

     /* for full bytes, we can just overwrite the old value with the new */
     for ( ; len >= 8 ; len -= 8 ) {
       *dst = (byte)( src );
       dst--;                         /* back up one byte */
       src >>= 8;                     /* get rid of used bits */
     }

     if (len > 0)     /* if some bits are leftover... */
     {
       mask = (byte) (0xff << len);
       *dst &= mask;                  /* clear bits on right side of byte */
       *dst |= ( (byte)( src ) & ~mask);        /* set appropriate bits */
     }

   }
} /* END b_packw */

/*============================================================================

FUNCTION B_UNPACKB

DESCRIPTION
  Given a buffer and an offset, unpacks the requested number of bits into
  a byte

DEPENDENCIES
  None

RETURN VALUE
  Unpacked item

SIDE EFFECTS
  None

============================================================================*/
byte b_unpackb(
   byte *src,
   word pos,
   word len
)
{
   byte result = 0;
   int rshift = 0;

   src += pos/8;
   pos %= 8;

   rshift = MAX( 8 - (pos + len), 0);

   if ( rshift > 0 ) {

     result = MASK_AND_SHIFT(len, pos, rshift, *src);

   } else {

     result = MASK(8-pos, pos, *src);
     src++;
     len -= 8 - pos;

      if ( len > 0 ) result = ( result<<len ) | (*src >> (8-len));  // if any bits left
   }

   return result;
} /* END b_unpackb */

/*============================================================================

FUNCTION B_UNPACKW

DESCRIPTION
  Given a buffer and an offset, unpacks the requested number of bits into
  a word

DEPENDENCIES
  None

RETURN VALUE
  Unpacked item

SIDE EFFECTS
  None

============================================================================*/
word b_unpackw(
   byte src[],
   word pos,
   word len
)
{
   word result = 0;
   int rshift = 0;

   src += pos/8;
   pos %= 8;

   rshift = MAX( 8 - (pos + len), 0);

   if ( rshift > 0 ) {

     result = MASK_AND_SHIFT(len, pos, rshift, *src);

   } else {

      result = MASK(8-pos, pos, *src);
      src++;
      len -= 8 - pos;

      for ( ; len >= 8  ; len-=8)
      {
         result = ( result<<8 ) | *src++;
      }

      if ( len > 0 ) result = ( result<<len ) | (*src >> (8-len));  // if any bits left
   }

   return result;
} /* END b_unpackw */

/*=========================================================================
  FUNCTION:  qcril_sms_hex_char_to_byte

===========================================================================*/
/*!
    @brief
    Convert a hex character to a byte

    @return
    Byte
*/
/*=========================================================================*/
byte qcril_sms_hex_char_to_byte
(
  char hex_char
)
{
  byte byte_value;

  if (hex_char >= 'A' && hex_char <= 'Z')
  {
    hex_char = hex_char + 'a' - 'A';
  }

  if (hex_char >= 'a' && hex_char <= 'f')
  {
    byte_value = (byte)(hex_char - 'a' + 10);
  }
  else if (hex_char >= 'A' && hex_char <= 'F')
  {
    byte_value = (byte)(hex_char - 'A' + 10);
  }
  else if (hex_char >= '0' && hex_char <= '9')
  {
    byte_value = (byte)(hex_char-'0');
  }
  else
  {
    byte_value = 0;
  }

  return (byte_value);
} /* qcril_sms_hex_char_to_byte */

/*=========================================================================
  FUNCTION:  qcril_sms_hex_to_byte

===========================================================================*/
/*!
    @brief
    Convert an SMS PDU from ASCII hex format to a byte array.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_hex_to_byte
(
  const char * hex_pdu,   // INPUT
  byte * byte_pdu,        // OUTPUT
  uint32 num_hex_chars
)
{
  uint16 buf_pos = 0;
  uint32 i;

  for (i=0; i < num_hex_chars; i++)
  {
    b_packb(qcril_sms_hex_char_to_byte(hex_pdu[i]), byte_pdu, buf_pos, (word)4);
    buf_pos += 4;
  }

} /* qcril_sms_hex_to_byte */


/*=========================================================================
  FUNCTION:  qcril_sms_byte_to_hex_char

===========================================================================*/
/*!
    @brief
    Convert a byte to a hex character.

    @return
    Hex char
*/
/*=========================================================================*/
char qcril_sms_byte_to_hex_char (byte val)
{
  char hex_char;

  if (val <= 9)
  {
    hex_char = (char)(val+'0');
  }
  else if (val >= 10 && val <= 15)
  {
    hex_char = (char)(val-10+'A');
  }
  else
  {
    hex_char = '0';
  }

  return (hex_char);
} /* qcril_sms_byte_to_hex_char */

/*=========================================================================
  FUNCTION:  qcril_sms_byte_to_hex

===========================================================================*/
/*!
    @brief
    Convert a byte array to an SMS PDU in ASCII hex format.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_byte_to_hex
(
  byte * byte_pdu,   // INPUT
  char * hex_pdu,    // OUTPUT
  uint32 num_bytes
)
{
  uint32 i;
  uint8 nibble;
  uint16 buf_pos = 0;

  for (i=0;i<num_bytes*2;i++)
  {
    nibble = b_unpackb(byte_pdu,buf_pos,4);
    buf_pos += 4;
    hex_pdu[i] = qcril_sms_byte_to_hex_char(nibble);
  }

} /* qcril_sms_byte_to_hex */

/*=========================================================================
FUNCTION
  qcril_sms_convert_ril_to_tl

DESCRIPTION
  This function converts the SMS TL data from RIL to the internal structure.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void qcril_sms_convert_ril_to_tl
(
  RIL_CDMA_SMS_Message *cdma_sms_msg,
  qcril_sms_tl_message_type * tl_ptr,
  boolean sms_on_ims,
  boolean is_mo_sms
)
{

  if(cdma_sms_msg != NULL && tl_ptr != NULL)
  {
  tl_ptr->mask = 0;

  tl_ptr->is_mo   = is_mo_sms;

  if ( cdma_sms_msg->uTeleserviceID == QCRIL_SMS_TELESERVICE_BROADCAST )
  {
    tl_ptr->tl_message_type = QCRIL_SMS_TL_TYPE_BROADCAST;
  }
  else
  {
    tl_ptr->tl_message_type = QCRIL_SMS_TL_TYPE_POINT_TO_POINT;
  }

  tl_ptr->mask |= QCRIL_SMS_MASK_TL_TELESERVICE_ID;
  tl_ptr->teleservice = cdma_sms_msg->uTeleserviceID;

  if ( cdma_sms_msg->bIsServicePresent == TRUE )
  {
    tl_ptr->mask |= QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY;
    tl_ptr->service = cdma_sms_msg->uServicecategory;
  }

  // address should always be present
  tl_ptr->mask |= QCRIL_SMS_MASK_TL_ADDRESS;
  tl_ptr->address = cdma_sms_msg->sAddress;

  if ( cdma_sms_msg->sSubAddress.number_of_digits != 0 )
  {
    tl_ptr->mask |= QCRIL_SMS_MASK_TL_SUBADDRESS;
    tl_ptr->subaddress = cdma_sms_msg->sSubAddress;
  }

  if (!sms_on_ims)
  {
    /* TL Ack is always requested for CDMA SMS; it should not be requested for IMS */
    tl_ptr->mask |= QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION;
    tl_ptr->bearer_reply_seq_num = 255;   /* dummy */
  }

  if (cdma_sms_msg->uBearerDataLen != 0)
  {
    tl_ptr->mask |= QCRIL_SMS_MASK_TL_BEARER_DATA;
  }
  }
  else
  {
    QCRIL_LOG_INFO("Invalid Arguments");
  }

} /* qcril_sms_convert_ril_to_tl */


/*=========================================================================
FUNCTION
  qcril_sms_cdma_encode_address

DESCRIPTION
  This function encodes the address

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void qcril_sms_cdma_encode_address
(
  const RIL_CDMA_SMS_Address  * address_ptr,
  uint8                       * parm_len_ptr,
  uint8                       * data
)
{
  uint32       bit_pos = 0;
  uint8        digit_size;
  uint32       i;

  if( address_ptr != NULL && parm_len_ptr != NULL && data != NULL)
  {
  /* Digit Mode */
  b_packb( (uint8) address_ptr->digit_mode,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos++;

  /* Number mode */
  b_packb( (uint8) address_ptr->number_mode,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos++;

  /* Number type */
  if ( address_ptr->digit_mode == RIL_CDMA_SMS_DIGIT_MODE_8_BIT )
  {
    digit_size = 8;

    b_packb( (uint8) address_ptr->number_type,
             data,
             (uint16) (bit_pos),
             3 );
    bit_pos += 3;

    if ( address_ptr->number_mode ==
         RIL_CDMA_SMS_NUMBER_MODE_NOT_DATA_NETWORK )
    {
      b_packb( (uint8) address_ptr->number_plan,
               data,
               (uint16) (bit_pos),
               4 );
      bit_pos += 4;
    }
  }
  else
  {
    digit_size = 4;
  }

  /* Address size */
  b_packb( address_ptr->number_of_digits,
           data,
           (uint16) (bit_pos),
           8 );
  bit_pos += 8;

  /* pack the digits */
  for ( i= 0; i < address_ptr->number_of_digits; i ++ )
  {
    b_packb( address_ptr->digits[i],
             data,
             (uint16) (bit_pos),
             digit_size );
    bit_pos += digit_size;
  }

  if ( bit_pos % 8 != 0 )
  {
    /* pad 0 to remaining bits
    */
    digit_size = (uint8)(8 - bit_pos % 8);
    b_packb( 0, data, (uint16) (bit_pos),  digit_size );
    bit_pos += digit_size;
  }

  *parm_len_ptr = (uint8) (bit_pos/8);
  }
  else
  {
    QCRIL_LOG_INFO("Invalid Arguments");
  }

} /* qcril_sms_cdma_encode_address */


/*=========================================================================
FUNCTION
  qcril_sms_cdma_encode_subaddress

DESCRIPTION
  This function encodes the subaddress

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void qcril_sms_cdma_encode_subaddress
(
  const RIL_CDMA_SMS_Subaddress  * address_ptr,
  uint8                          * parm_len_ptr,
  uint8                          * data
)
{
  uint32             bit_pos = 0;
  uint8              digit_size;
  uint32       i;

  if(address_ptr != NULL && parm_len_ptr != NULL && data != NULL)
  {
  /* subaddress type */
  b_packb( (uint8) address_ptr->subaddressType,
           data,
           (uint16) (bit_pos),
           3 );
  bit_pos += 3;

  /* Odd flag */
  b_packb( address_ptr->odd,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos += 1;

  /* Address size */
  b_packb( address_ptr->number_of_digits,
           data,
           (uint16) (bit_pos),
           8 );
  bit_pos += 8;

  /* Digit size */
  digit_size = 8;  /* always 8 */

  /* Pack the digits */
  for ( i=0; i < address_ptr->number_of_digits; i++ )
  {
    b_packb( address_ptr->digits[i],
             data,
             (uint16) (bit_pos),
             digit_size );
    bit_pos += digit_size;
  }

  /* pad last 4 bits with 0 */
  b_packb( 0,
           data,
           (uint16) (bit_pos),
           4 );
  bit_pos += 4;

  /* return len */
  * parm_len_ptr = (uint8) (bit_pos/8);
  }
  else
  {
    QCRIL_LOG_INFO("Invalid Arguments");
  }

} /* qcril_sms_cdma_encode_subaddress */


/*=========================================================================
FUNCTION
  qcril_sms_convert_tl_to_qmi

DESCRIPTION
  This function converts from TL format to QMI format (OTA format).

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS
  None

=========================================================================*/
boolean qcril_sms_convert_tl_to_qmi
(
  qcril_sms_tl_message_type        * tl_msg_ptr,   /* IN */
  uint32                             raw_bd_len,   /* IN */
  uint8                            * raw_bd_ptr,   /* IN */
  qcril_sms_OTA_message_type       * OTA_msg_ptr   /* OUT */
)
{
  uint16                       pos = 0;
  uint8                        parm_len;
  uint8                        * parm_len_ptr;
  uint8                        * data;

  boolean status = TRUE;

  do
  {
    if(tl_msg_ptr != NULL && raw_bd_ptr != NULL && OTA_msg_ptr != NULL)
    {
  /* Check whether mandatory srv_category is present */
  if ( QCRIL_SMS_TL_TYPE_BROADCAST == tl_msg_ptr->tl_message_type )
  {
    if ( FALSE == (tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY) )
    {
          status = FALSE;
          break;
    }
  }

  data = (uint8*) OTA_msg_ptr->data;

  /* For the following special cases, there is TL information and raw_bd_ptr
  ** has the OTA data ready
  */
  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_TELESERVICE_ID )
  {
    if ( tl_msg_ptr->teleservice == QCRIL_SMS_TELESERVICE_MWI ||
         tl_msg_ptr->teleservice == QCRIL_SMS_TELESERVICE_IS91_PAGE ||
         tl_msg_ptr->teleservice == QCRIL_SMS_TELESERVICE_IS91_VOICE_MAIL ||
         tl_msg_ptr->teleservice == QCRIL_SMS_TELESERVICE_IS91_SHORT_MESSAGE )
    {
      OTA_msg_ptr->data_len = (uint16) MIN (raw_bd_len, RIL_CDMA_SMS_BEARER_DATA_MAX);
      memcpy( (uint8 *)OTA_msg_ptr->data, raw_bd_ptr, OTA_msg_ptr->data_len );
          status = TRUE;
          break;
    }
  }

  /* msg type
  */
  data[0] = (uint8) tl_msg_ptr->tl_message_type;
  pos ++;

  /* the remaining parameters have one or more of the following:
     - PARAMETER_ID    8 bits
     - PARAMETER_LEN   8 bits
     - Parameter Data  8 x PARAMETER_LEN
  */

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_TELESERVICE_ID )
  {
    data[pos] = (uint8) QCRIL_SMS_TL_TELESERVICE_ID;
    pos++; /* skip parm id */

    data[pos] = parm_len = 2;
    pos++; /* skip len */

    b_packw( (uint16) tl_msg_ptr->teleservice,
             data,
             (uint16) (pos * 8),
             parm_len * 8 );

    pos += parm_len;  /* skip parm data */
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY )
  {
    data[pos] = (uint8) QCRIL_SMS_TL_BC_SRV_CATEGORY;
    pos++; /* skip parm id */

    data[pos] = parm_len = 2;
    pos++; /* skip len */

    b_packw( (uint16) (tl_msg_ptr->service),
             data,
             (uint16) (pos * 8),
             parm_len * 8 );

    pos += parm_len;  /* skip parm data */
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_ADDRESS )
  {
    data[pos] = (uint8) (tl_msg_ptr->is_mo ? QCRIL_SMS_TL_DEST_ADDRESS : QCRIL_SMS_TL_ORIG_ADDRESS);
    pos++; /* skip parm id */

    parm_len_ptr = data + pos;   /* will set value later */
    pos ++; /* skip parm len */

    qcril_sms_cdma_encode_address( & tl_msg_ptr->address,
                                   parm_len_ptr,
                                   data+pos );
    pos += * parm_len_ptr; /* skip parm data */
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_SUBADDRESS )
  {
    data[pos] = (uint8) (tl_msg_ptr->is_mo ? QCRIL_SMS_TL_DEST_SUBADDRESS : QCRIL_SMS_TL_ORIG_SUBADDRESS);
    pos++; /* skip parm id */

    parm_len_ptr = data + pos;   /* will set value later */
    pos ++; /* skip parm len */

    qcril_sms_cdma_encode_subaddress( & tl_msg_ptr->subaddress,
                                      parm_len_ptr,
                                      data+pos );
    pos += * parm_len_ptr;
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION )
  {
    data[pos] = (uint8) QCRIL_SMS_TL_BEARER_REPLY_OPTION;
    pos++; /* skip parm id */

    data[pos] = parm_len = 1;
    pos++;

    b_packb( tl_msg_ptr->bearer_reply_seq_num,
             data,
             (uint16) (pos * 8),
             6 );  /* high 6 bits */

    b_packb( 0,
             data,
             (uint16) (pos * 8 + 6),
             2 );  /* low 2 bits: reserved, set to 0 */

    pos += parm_len;  /* skip parm data */
  }

  if ( tl_msg_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_DATA )
  {
    if ( raw_bd_len == 0 || raw_bd_len > RIL_CDMA_SMS_BEARER_DATA_MAX )
    {
      QCRIL_LOG_ERROR( "Invalid bd len: %d", (int)raw_bd_len);
      status = FALSE;
    }
    else
    {
      data[pos] = (uint8) QCRIL_SMS_TL_BEARER_DATA;
      pos++; /* skip parm id */

      data[pos] = (uint8) MIN (raw_bd_len, RIL_CDMA_SMS_BEARER_DATA_MAX);
      pos++; /* skip parm len */

      memcpy( data+pos, raw_bd_ptr, MIN((uint8)raw_bd_len, (uint8)RIL_CDMA_SMS_BEARER_DATA_MAX-pos) );
      pos += (uint16) raw_bd_len; /* skip parm data */
    }
  }

  OTA_msg_ptr->data_len = pos;
  /* data was filled in the above */

  if ( status == TRUE && pos > QCRIL_SMS_TL_MAX_LEN )
  {
    // encoded data has too many bytes
    QCRIL_LOG_ERROR( "Invalid parm size: %d", pos);
    status = FALSE;
  }
    }
    else
    {
      status = FALSE;
    }
  }while(0);

  return status;

} /* qcril_sms_convert_tl_to_qmi */


/*===========================================================================

FUNCTION    qcril_sms_convert_sms_ril_to_qmi

DESCRIPTION
  Convert the SMS from RIL format to QMI format

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS


===========================================================================*/
boolean qcril_sms_convert_sms_ril_to_qmi
(
  RIL_CDMA_SMS_Message        *cdma_sms_msg,
  uint8                       *data_buf,
  uint16                       data_buf_len,
  uint16                      *ota_data_len,
  boolean                     sms_on_ims,
  boolean                     is_mo_sms
)
{
  qcril_sms_OTA_message_type   ota_data;
  qcril_sms_tl_message_type    tl_msg;
  boolean status = FALSE;

  if(cdma_sms_msg != NULL && data_buf != NULL && ota_data_len != NULL)
  {
  (void) memset((uint8 *)ota_data.data, 0, sizeof(ota_data.data));
  (void) memset(&tl_msg, 0, sizeof(tl_msg));

  qcril_sms_convert_ril_to_tl(cdma_sms_msg, &tl_msg, sms_on_ims, is_mo_sms);
  status = qcril_sms_convert_tl_to_qmi(&tl_msg,
                                       cdma_sms_msg->uBearerDataLen,
                                       cdma_sms_msg->aBearerData,
                                       &ota_data);
  if (status)
  {
    if (NULL != ota_data_len)
    {
      *ota_data_len = ota_data.data_len;
    }
    if (NULL != data_buf && data_buf_len > 0)
    {
      (void) memcpy((uint8 *)data_buf, (uint8 *)ota_data.data, MIN(data_buf_len, ota_data.data_len));
    }
  }
  }

  return status;
} /* qcril_sms_convert_sms_ril_to_qmi */


/*=========================================================================
FUNCTION
  qcril_sms_decode_address

DESCRIPTION
  This function decodes the address

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS
  None

=========================================================================*/
boolean qcril_sms_decode_address
(
  const uint8             * data,
  uint8                     parm_len,
  RIL_CDMA_SMS_Address    * address_ptr
)
{
  uint32     bit_pos = 0;
  uint8      digit_size;
  uint32     i;
  boolean    status = TRUE;

  if(address_ptr != NULL && data != NULL)
  {
  /* Digit mode */
  address_ptr->digit_mode = (RIL_CDMA_SMS_DigitMode)
                            b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos ++;

  /* Number mode */
  address_ptr->number_mode = (RIL_CDMA_SMS_NumberMode)
                             b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos ++;


  /* Number type, and determine digit size */
  if ( address_ptr->digit_mode == RIL_CDMA_SMS_DIGIT_MODE_8_BIT )
  {
    address_ptr->number_type = (RIL_CDMA_SMS_NumberType)
                               b_unpackb( (uint8*) data, (uint16) (bit_pos), 3 );
    bit_pos += 3;

    digit_size = 8;
  }
  else
  {
    address_ptr->number_type = RIL_CDMA_SMS_NUMBER_TYPE_UNKNOWN;
    digit_size = 4;
  }


  /* Number plan */
  if ( address_ptr->digit_mode  == RIL_CDMA_SMS_DIGIT_MODE_8_BIT &&
       address_ptr->number_mode == RIL_CDMA_SMS_NUMBER_MODE_NOT_DATA_NETWORK )
  {
    address_ptr->number_plan = (RIL_CDMA_SMS_NumberPlan)
                               b_unpackb( (uint8*) data, (uint16) (bit_pos), 4);
    bit_pos += 4;
  }
  else
  {
    /* number plan is not used */
    address_ptr->number_plan = RIL_CDMA_SMS_NUMBER_PLAN_UNKNOWN;
  }

  /* Address size */
  address_ptr->number_of_digits=
  b_unpackb( (uint8*) data, (uint16) (bit_pos), 8 );
  bit_pos += 8;


  /* Extract all digits:
  */
  if ( address_ptr->number_of_digits > RIL_CDMA_SMS_ADDRESS_MAX )
  {
    QCRIL_LOG_ERROR( "decoding: address digits number too big: %d",
                     address_ptr->number_of_digits);
    status = FALSE;
  }
  else
  {
    for ( i=0; i < address_ptr->number_of_digits; i++ )
    {
      address_ptr->digits[i] = b_unpackb( (uint8*) data,
                                          (uint16) (bit_pos),
                                          digit_size );
      bit_pos += digit_size;
    }
  }

  /* Parm len checking */
  if ( bit_pos > (uint32)(parm_len*8) )
  {
    QCRIL_LOG_ERROR( "decoding: address too long: %d>%d",
                     bit_pos, parm_len*8);
    status = FALSE;
  }
  }
  else
  {
    status = FALSE;
  }

  return status;

} /* qcril_sms_decode_address */


/*=========================================================================
FUNCTION
  qcril_sms_decode_subaddress

DESCRIPTION
  This function decodes the subaddress

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS
  None

=========================================================================*/
boolean qcril_sms_decode_subaddress
(
  const uint8                * data,
  uint8                        parm_len,
  RIL_CDMA_SMS_Subaddress    * address_ptr
)
{
  uint32     bit_pos = 0;
  uint8      digit_size;
  uint32     i;
  boolean    status = TRUE;

  if(address_ptr != NULL && data != NULL)
  {
  /* address type */
  address_ptr->subaddressType = (RIL_CDMA_SMS_SubaddressType)
                                b_unpackb( (uint8*) data, (uint16) (bit_pos), 3 );
  bit_pos += 3;

  /* Odd flag */
  address_ptr->odd = b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos += 1;

  /* Address size */
  address_ptr->number_of_digits = b_unpackb( (uint8*) data, (uint16) (bit_pos), 8 );
  bit_pos += 8;

  /* Extract all digits:
  */
  if ( address_ptr->number_of_digits > RIL_CDMA_SMS_SUBADDRESS_MAX )
  {
    QCRIL_LOG_ERROR( "decoding: subaddress digits number too big: %d",
                     address_ptr->number_of_digits);
    status = FALSE;
  }
  else
  {
    digit_size = 8;  /* always 8 bits so far */

    for ( i=0; i < address_ptr->number_of_digits; i++ )
    {
      address_ptr->digits[i] = b_unpackb( (uint8*) data,
                                          (uint16) (bit_pos),
                                          digit_size );
      bit_pos += digit_size;
    }
  }

  /* parm len check */
  if ( bit_pos > (uint32)(parm_len*8) )
  {
    QCRIL_LOG_ERROR( "decoding: subaddress too long: %d>%d",
                     bit_pos, parm_len*8);
    status = FALSE;
  }
  }
  else
  {
    status = FALSE;
  }

  return status;

} /* qcril_sms_decode_subaddress */


/*=========================================================================
FUNCTION
  qcril_sms_convert_qmi_to_tl

DESCRIPTION
  This function converts from QMI format (OTA format) to TL format.

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS
  None

=========================================================================*/
boolean qcril_sms_convert_qmi_to_tl
(
  qcril_sms_OTA_message_type     * OTA_msg_ptr,   /* IN */
  qcril_sms_tl_message_type      * tl_ptr,        /* OUT */
  uint32                         * raw_bd_len,    /* OUT */
  uint8                          * raw_bd_ptr     /* OUT */
)
{
  uint16                    pos;      /* running position in bytes */
  qcril_sms_tl_parm_id_e_type parm_id = QCRIL_SMS_TL_DUMMY;
  uint8                    parm_len;
  uint16                    tl_len;  /* len of TL data */
  boolean status = TRUE,return_back = FALSE;

  do
  {
    if(tl_ptr != NULL && raw_bd_ptr != NULL && OTA_msg_ptr != NULL && OTA_msg_ptr->data_len > TL_HEADER_SIZE)
    {
  if ( OTA_msg_ptr->data_len > RIL_CDMA_SMS_BEARER_DATA_MAX )
  {
    QCRIL_LOG_ERROR("** TL Msg len too long: %d", OTA_msg_ptr->data_len);
    status = FALSE;
  }
  else
  {
    tl_len = OTA_msg_ptr->data_len;
    pos = 0;

    /* ---- start decoding ---- */

    tl_ptr->mask = 0;

    /* the first byte is msg type */

    if (OTA_msg_ptr->data[pos] > (uint8) QCRIL_SMS_TL_TYPE_MAX )
    {
      QCRIL_LOG_ERROR("WARNING: Invalid TL msg type: %d", OTA_msg_ptr->data[pos]);
          status = FALSE;
          break;
    }
    else
    {
      tl_ptr->tl_message_type = (qcril_sms_tl_message_type_e_type)(OTA_msg_ptr->data[pos]);
      pos ++;
    }

    /* the remain data has one or more of the following:
       - PARAMETER_ID    8 bits
       - PARAMETER_LEN   8 bits
       - Parameter Data  8 x PARAMETER_LEN
    */

    while ( status == TRUE  )
    {

      if ( pos == tl_len )
      {
        /* Good. Done with parameter processing successfully */
        break; /* out of while loop */
      }
      else if ( pos + TL_PARM_SIZE > tl_len )
      {
        /* Current position goes beyond the msg size. */
        QCRIL_LOG_ERROR( "decoding: msg size %d incorrect", tl_len);
        status = FALSE;
        break; /* out of while loop */
      }

      parm_id = (qcril_sms_tl_parm_id_e_type) ( OTA_msg_ptr->data[ pos ] );
      pos ++;  /* skip parm id */

      parm_len = OTA_msg_ptr->data[ pos ];
      pos ++;  /* skip parm len */

      if ( pos + parm_len > tl_len )
      {
        /* parm data passes the max length of bearer data
        */
        QCRIL_LOG_ERROR( "decoding: parameter (id=%d) extends beyond msg size %d",
                         parm_id, tl_len);
        status = FALSE;
        break; /* out of while loop */
      }
      else if(pos >= RIL_CDMA_SMS_BEARER_DATA_MAX)
      {
        /* current position exceeds maximum msg size
        */
        QCRIL_LOG_ERROR( "decoding: current position %u exceeds maximum msg size %u",
                         pos,RIL_CDMA_SMS_BEARER_DATA_MAX);
        status = FALSE;
        break; /* out of while loop */
      }



      /* Now pos should point to the parm data */
      /* After each case below, pos should point to the next parm Id */

      switch ( parm_id )
      {
        case QCRIL_SMS_TL_TELESERVICE_ID:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_TELESERVICE_ID )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }

          tl_ptr->mask |= QCRIL_SMS_MASK_TL_TELESERVICE_ID;

          if ( pos  + 2 > tl_len ||
               parm_len != 2
             )
          {
            /* tl data too short , or bad len field */
            status = FALSE;
          }
          else
          {
            tl_ptr->teleservice = (qcril_sms_teleservice_e_type)
                                  b_unpackw( (uint8*) OTA_msg_ptr->data,
                                             (uint16) (pos*8),
                                             parm_len*8 );
            pos += parm_len; /* skip parm data */
          }

          break;


        case QCRIL_SMS_TL_BC_SRV_CATEGORY:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY;

          if ( pos  + 2 > tl_len ||
               parm_len != 2
             )
          {
            /* tl data too short , or bad len field */
            status = FALSE;
          }
          else
          {
            tl_ptr->service = b_unpackw( (uint8*) OTA_msg_ptr->data,
                                         (uint16) (pos*8),
                                         parm_len*8 );
            pos += parm_len; /* skip parm data */
          }

          break;

        case QCRIL_SMS_TL_ORIG_ADDRESS:
        case QCRIL_SMS_TL_DEST_ADDRESS:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_ADDRESS )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= QCRIL_SMS_MASK_TL_ADDRESS;

          /* Set the MO/MT tag. */
          tl_ptr->is_mo = ( parm_id == QCRIL_SMS_TL_DEST_ADDRESS ) ?
                          TRUE : FALSE;

          status = qcril_sms_decode_address( (uint8*) OTA_msg_ptr->data+pos,
                                             parm_len,
                                             & tl_ptr->address );
          pos += parm_len;
          break;

        case QCRIL_SMS_TL_ORIG_SUBADDRESS:
        case QCRIL_SMS_TL_DEST_SUBADDRESS:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_SUBADDRESS )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= QCRIL_SMS_MASK_TL_SUBADDRESS;

          /* Set the MO/MT tag. */
          tl_ptr->is_mo = ( parm_id == QCRIL_SMS_TL_DEST_SUBADDRESS ) ?
                          TRUE : FALSE;

          status = qcril_sms_decode_subaddress( (uint8*) OTA_msg_ptr->data+pos,
                                                parm_len,
                                                & tl_ptr->subaddress );

          pos += parm_len;  /* skip parm data */
          break;

        case QCRIL_SMS_TL_BEARER_REPLY_OPTION:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= QCRIL_SMS_MASK_TL_BEARER_REPLY_OPTION;

              return_back = FALSE;
          if ( parm_len != 1 )
          {
            QCRIL_LOG_ERROR("decoding: bearer reply option len is not 1: %d. Reset to 1.", parm_len);
                status = FALSE;
                return_back = TRUE;
          }
              else
              {
          tl_ptr->bearer_reply_seq_num = OTA_msg_ptr->data[pos] >> 2;
          /* get high 6 bits */

          pos += parm_len; /* skip parm data */
              }

          break;


        case QCRIL_SMS_TL_BEARER_DATA:

          if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_DATA )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          /* bearer data is not decoded here */
          /* it will be copied to the output */

          tl_ptr->mask |= QCRIL_SMS_MASK_TL_BEARER_DATA;

          if ( parm_len == 0 )
          {
            QCRIL_LOG_ERROR("decoding: bearer data len is 0");
            status = FALSE;
          }
          /* parm_len is uint8, so it won't be greater than WMS_MAX_LEN */
          else if ( pos + parm_len > tl_len )
          {
            QCRIL_LOG_ERROR("decoding: bearer data len %d extends beyond msg size %d",
                            parm_len, tl_len);
            status = FALSE;
          }
          else
          {
            /* copy bearer data to output
            */
            *raw_bd_len = MIN (parm_len, RIL_CDMA_SMS_BEARER_DATA_MAX);
            memcpy( raw_bd_ptr, (uint8*)OTA_msg_ptr->data + pos, *raw_bd_len );
          }

          pos += parm_len; /* skip parm data */

          break;

        default:
          QCRIL_LOG_ERROR( "decoding: Invalid parm id: %d", parm_id);
          pos += parm_len; /* skip parm data */
          break;           /* continue with next parm */

      } /* switch */
          if(return_back == TRUE)
          {
            break;
          }
    } /* while */
        if(return_back == TRUE)
        {
          break;
        }


    /* Null the raw bd */
    if ( ! ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BEARER_DATA ) )
    {
      *raw_bd_len = 0;
    }


    /* -------- check CDMA TL mandatory fields -------- */
    if ( status == TRUE )
    {
      switch ( tl_ptr->tl_message_type )
      {
        case QCRIL_SMS_TL_TYPE_POINT_TO_POINT:

          if ( ! ( tl_ptr->mask & QCRIL_SMS_MASK_TL_ADDRESS ) )
          {
            QCRIL_LOG_ERROR("No address present in msg!");
            status = FALSE;
          }

          break;

        case QCRIL_SMS_TL_TYPE_BROADCAST:

          if ( ! ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY ) )
          {
            QCRIL_LOG_ERROR("decoding: broadcast category is not present!");
            status = FALSE;
          }

          break;

        case QCRIL_SMS_TL_TYPE_ACK:

          if ( ! ( tl_ptr->mask & QCRIL_SMS_MASK_TL_CAUSE_CODES ) )
          {
            QCRIL_LOG_ERROR("decoding: cause code is not present in Ack!");
            status = FALSE;
          }

          break;

        default:

          QCRIL_LOG_ERROR( "decoding: invalid TL msg type: %d",
                           tl_ptr->tl_message_type);
          status = FALSE;
          break;

      } /* switch */

    } /* end of CDMA TL fields checking */
  }
    }
    else
    {
      status = FALSE;
    }
  }while(0);


  return status;

} /* qcril_sms_convert_qmi_to_tl */


/*=========================================================================
FUNCTION
  qcril_sms_convert_tl_to_ril

DESCRIPTION
  This function converts the SMS data from TL format to RIL format

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void qcril_sms_convert_tl_to_ril
(
  const qcril_sms_tl_message_type * tl_ptr,        /* IN */
  RIL_CDMA_SMS_Message            * cdma_sms_msg   /* OUT */
)
{

  do
  {
    if(tl_ptr == NULL || cdma_sms_msg == NULL)
    {
      break;
    }
  /* msg type
  */
  if ( tl_ptr->tl_message_type == QCRIL_SMS_TL_TYPE_BROADCAST )
  {
    cdma_sms_msg->uTeleserviceID  = QCRIL_SMS_TELESERVICE_BROADCAST;
  }
  else if ( tl_ptr->tl_message_type == QCRIL_SMS_TL_TYPE_POINT_TO_POINT )
  {
    /* teleservice id
    */
    if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_TELESERVICE_ID )
    {
      cdma_sms_msg->uTeleserviceID = tl_ptr->teleservice;
    }
    else
    {
      cdma_sms_msg->uTeleserviceID = QCRIL_SMS_TELESERVICE_UNKNOWN;
    }
  }
  else
  {
    /* it is an ACK msg; not applicable for client msg */
      break;  /* SHORT-RETURN */
  }


  /* address
  */
  if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_ADDRESS )
  {
    cdma_sms_msg->sAddress = tl_ptr->address;
  }
  else
  {
    cdma_sms_msg->sAddress.number_of_digits = 0;
  }

  /* sub-address
  */
  if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_SUBADDRESS )
  {
    cdma_sms_msg->sSubAddress = tl_ptr->subaddress;
  }
  else
  {
    cdma_sms_msg->sSubAddress.number_of_digits = 0;
  }

  /* broadcast category
  */
  if ( tl_ptr->mask & QCRIL_SMS_MASK_TL_BC_SRV_CATEGORY )
  {
    cdma_sms_msg->bIsServicePresent = TRUE;
    cdma_sms_msg->uServicecategory = tl_ptr->service;
  }
  else
  {
    cdma_sms_msg->bIsServicePresent = FALSE;
    cdma_sms_msg->uServicecategory = 0;  /* 0 means unspecified */
  }
  }while(0);

  return;

} /* qcril_sms_convert_tl_to_ril */


/*===========================================================================

FUNCTION    qcril_sms_convert_mt_sms_qmi_to_ril

DESCRIPTION
  Convert the MT SMS from QMI format to RIL format

DEPENDENCIES
  None

RETURN VALUE
  TRUE if successful
  FALSE otherwise

SIDE EFFECTS


===========================================================================*/
boolean qcril_sms_convert_mt_sms_qmi_to_ril
(
  const uint8                 *data,
  uint16                      length,
  RIL_CDMA_SMS_Message        *cdma_sms_msg
)
{
  qcril_sms_OTA_message_type   ota_data;
  qcril_sms_tl_message_type    tl_msg;
  boolean status = TRUE;

  if(cdma_sms_msg != NULL && data != NULL)
  {
  /* reset static structures */
  (void) memset((uint8 *)ota_data.data, 0, sizeof(ota_data.data));
  (void) memset(&tl_msg, 0, sizeof(tl_msg));

  ota_data.data_len  = MIN(length, RIL_CDMA_SMS_BEARER_DATA_MAX);
  (void) memcpy((uint8 *)ota_data.data, (uint8 *)data, ota_data.data_len);

  status = qcril_sms_convert_qmi_to_tl(&ota_data,
                                       &tl_msg,
                                       (uint32 *) &cdma_sms_msg->uBearerDataLen,
                                       cdma_sms_msg->aBearerData);
  if (status == TRUE)
  {
    qcril_sms_convert_tl_to_ril(&tl_msg, cdma_sms_msg);
    }
  }
  else
  {
    status = FALSE;
  }

  return status;
} /* qcril_sms_convert_mt_sms_qmi_to_ril */
//===========================================================================
//qmi_ril_set_sms_svc_status
//===========================================================================
void qmi_ril_set_sms_svc_status(qmi_ril_sms_svc_status_type new_status)
{
  QCRIL_LOG_INFO( "new_status %d", new_status );
  qmi_ril_sms_svc_status = new_status;
} // qmi_ril_set_sms_svc_status
//===========================================================================
//qmi_ril_get_sms_svc_status
//===========================================================================
qmi_ril_sms_svc_status_type qmi_ril_get_sms_svc_status(void)
{
  return qmi_ril_sms_svc_status;
} // qmi_ril_get_sms_svc_status

//=========================================================================================
//qmi_ril_block_mo_sms_on_1x
//
//returns TRUE if 3gpp2 MO SMS needs to be blocked
//conditions to be met:
//User sets persist.radio.block_sms_on_1x to "1", Target is NOT SVLTE capable and IMS is not registered but LTE service is available
//=========================================================================================
int qmi_ril_block_mo_sms_on_1x()
{
  qcril_sms_struct_type *i_ptr;
  int ret = FALSE;

  QCRIL_LOG_FUNC_ENTRY();

  i_ptr = &qcril_sms[ QCRIL_DEFAULT_INSTANCE_ID ];
  QCRIL_MUTEX_LOCK( &i_ptr->transport_layer_info_mutex, NULL );
  QCRIL_LOG_INFO( "transport layer info status block %d, valid %d value %d", i_ptr->transport_layer_info.block_sms_on_1x,
                                                               i_ptr->transport_layer_info.registered_ind_valid,
                                                               i_ptr->transport_layer_info.registered_ind );
  if( TRUE == i_ptr->transport_layer_info.block_sms_on_1x )
  {
    if( !(QMI_DMS_MASK_SVLTE_CAPABLE_V01 & qcril_qmi_nas_retrieve_sv_capability()) &&
        (FALSE == i_ptr->transport_layer_info.registered_ind_valid ||
        FALSE == i_ptr->transport_layer_info.registered_ind ) &&
        RADIO_TECH_LTE == qcril_qmi_nas_retrieve_reported_data_technology() )
    {
      ret = TRUE;
    }
  }
  QCRIL_MUTEX_UNLOCK( &i_ptr->transport_layer_info_mutex, NULL );

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
  return ret;
} // qmi_ril_block_mo_sms_on_1x

//============================================================================
// FUNCTION: set_wms_ready_state <state>
//
// DESCRIPTION:
// Sets protected cache value wms_ready_state and logs its new value
//
// RETURN: None
//============================================================================
void set_wms_service_state(wms_service_ready_status_enum_v01 state) {

   SMS_CACHE_LOCK;
   wms_service_state = state;
   SMS_CACHE_UNLOCK;

   /* if if wms status is becoming none, then clear sms ack cache so that
      even if there are any un acked sms's qcril won't cause it to block further sms
      when wms reports status as fine */
   if (WMS_SERVICE_READY_STATUS_NONE_V01 == state)
   {
       qcril_setup_timed_callback(QCRIL_DEFAULT_INSTANCE_ID,
                                  QCRIL_DEFAULT_MODEM_ID,
                                  qcril_sms_mt_transaction_expired,
                                  NULL,
                                  NULL);
   }

   QCRIL_LOG_INFO("service ready state updated: %s", sms_state_str(state) );

}

//============================================================================
// FUNCTION: wms_ready_supported
//
// DESCRIPTION: Queries modem to determine whether wms ready indications
// are supported or not
// Note: we use the indication registration as verification of support
//
// RETURN: 0 | 1 --> not supported, supported
//============================================================================
int wms_ready_supported(void) {

   int result = 0;
   qmi_client_error_type qmi_err;

   QCRIL_LOG_FUNC_ENTRY();

   wms_get_indication_register_resp_msg_v01 wms_resp;
   bzero(&wms_resp, sizeof(wms_resp));

   qmi_err = qmi_client_send_msg_sync_with_shm(
                      qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_WMS ),
                      QMI_WMS_GET_INDICATION_REGISTER_REQ_V01,
                      NULL,
                      0,
                      (void*) &wms_resp,
                      sizeof(wms_resp),
                      QCRIL_QMI_WMS_LONG_SYNC_REQ_TIMEOUT );

   if ( qmi_err != QMI_NO_ERR)
   {
      QCRIL_LOG_ERROR("Failed wms get indication register request w/%s",
                                                          qmi_errstr(qmi_err));
      result = 0;
   }
   else {
      if ( ! (wms_resp.reg_service_ready_events_valid
              && (wms_resp.reg_service_ready_events == 0x01) ) ) {
         QCRIL_LOG_WARN( "wms service ready not supported, assuming ready.");
         result = 0;
      }
      else {
         QCRIL_LOG_WARN( "wms service ready is supported.");
         result = 1;
      }
   }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: query_wms_ready_status
//
// DESCRIPTION: Queries modem for wms ready status
// We use this to capture the status in case we missed an indication set
// while starting up before we've had a chance to register for it
//
// RETURN: None
//============================================================================
void query_wms_ready_status(void)
{
   wms_service_ready_status_enum_v01 result;
   qmi_client_error_type qmi_err;
   qmi_txn_handle txn_handle; // holder required for async send


   QCRIL_LOG_FUNC_ENTRY();

   wms_get_service_ready_status_resp_msg_v01* wms_resp;
   wms_resp = qcril_malloc(sizeof(*wms_resp));
   if (!wms_resp)
   {
      QCRIL_LOG_ERROR("Failed to allocate memory.");
      QCRIL_LOG_FUNC_RETURN();
      return;
   }
   bzero(wms_resp, sizeof(*wms_resp));

   qmi_err = qmi_client_send_msg_async(
                     qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_WMS),
                     QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01,
                     NULL,
                     0,
                     wms_resp,
                     sizeof(*wms_resp),
                     wms_get_ready_status_cb,
                     NULL,
                     &txn_handle);

   if ( qmi_err != QMI_NO_ERR )
   {
      QCRIL_LOG_ERROR("Failed wms service ready status request w/%s",
                                                      qmi_errstr(qmi_err));
      qcril_free( wms_resp );
   }
   else
   {
      QCRIL_LOG_INFO("QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01 sent to modem");
   }

   QCRIL_LOG_FUNC_RETURN();
}

//============================================================================
// FUNCTION: wms_get_ready_status_cb <parms type> <parms ptr>
//
// DESCRIPTION:
// Callback handler for async wms_get_ready_status msg
// sets internal cache wms_service_state
//
// RETURN: None
//============================================================================
void wms_get_ready_status_cb
(
    qmi_client_type       user_handle,
    unsigned int          msg_id,
    void                 *resp_c_struct,
    unsigned int          resp_c_struct_len,
    void                 *resp_cb_data,
    qmi_client_error_type transp_err
)
{
   QCRIL_LOG_FUNC_ENTRY();

   QCRIL_ASSERT( resp_c_struct != NULL );
   QCRIL_ASSERT( user_handle != NULL );

   QCRIL_NOTUSED(transp_err);
   QCRIL_NOTUSED(resp_cb_data);
   QCRIL_NOTUSED(resp_c_struct_len);
   QCRIL_NOTUSED(msg_id);

   wms_get_service_ready_status_resp_msg_v01* wms_resp
              = (wms_get_service_ready_status_resp_msg_v01*) resp_c_struct;

   if (!wms_resp) {
      QCRIL_LOG_ERROR("Unexpected Null pointer in response message");
      QCRIL_LOG_FUNC_RETURN();
      return;
   }

   if (wms_resp->resp.result != QMI_RESULT_SUCCESS_V01) {
      QCRIL_LOG_ERROR("wms_get_service_ready_status returned w/%s",
                                          qmisvc_errstr(wms_resp->resp.error));
      qcril_free( wms_resp );
      QCRIL_LOG_FUNC_RETURN();
      return;
   }

   if (!wms_resp->ready_status_valid) {
      QCRIL_LOG_WARN( "wms service ready tlv missing, assuming ready");
      set_wms_service_state(WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01);
   }
   else {
      set_wms_service_state(wms_resp->ready_status);
   }

   qcril_free( wms_resp );
   QCRIL_LOG_FUNC_RETURN();
}

//============================================================================
// FUNCTION: wms_ready <expected_tech>
//
// DESCRIPTION:
// Check that wms service is ready for <expected_tech> and return a boolean
// an error is logged if not ready.
//
// RETURN: 0 | 1 --> not ready, ready
//============================================================================
int wms_ready(wms_service_ready_status_enum_v01 expected_state)
{
   int result = 0;
   QCRIL_LOG_FUNC_ENTRY();

   SMS_CACHE_LOCK;
   wms_service_ready_status_enum_v01 state = wms_service_state;
   SMS_CACHE_UNLOCK;

   if (state & expected_state) {
      result = 1;
   }
   else
   {
      // wms service not ready for expected_state
      const char* state_str = sms_state_str(state);

      switch (expected_state) {
         case WMS_SERVICE_READY_STATUS_3GPP_V01:
            QCRIL_LOG_WARN("WMS 3GPP service not ready, %s", state_str);
            break;
         case WMS_SERVICE_READY_STATUS_3GPP2_V01:
            QCRIL_LOG_WARN("WMS 3GPP2 service not ready, %s", state_str);
            break;
         case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
            QCRIL_LOG_WARN("WMS 3GPP and 3GPP2 expected, service not ready, %s",
                             state_str);
            break;
         default:
            QCRIL_LOG_WARN("WMS service not ready, %s", state_str);
            break;
      }
   }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//============================================================================
// FUNCTION: qcril_send_generic_failure <token> <event>
//
// DESCRIPTION:
// Shortcut function to send a GENERIC_FAILURE result
//
// RETURN: service state string
//============================================================================
void send_generic_failure(RIL_Token token, int event)
{
   qcril_request_resp_params_type resp;
   qcril_default_request_resp_params(QCRIL_DEFAULT_INSTANCE_ID, token,
                                     event, RIL_E_GENERIC_FAILURE, &resp);
   qcril_send_request_response(&resp);
}

//============================================================================
// FUNCTION: sms_state_str <service state>
//
// DESCRIPTION:
// Convert sms service state (wms_service_ready_status_enum_v01) to string
// for logging
//
// RETURN: service state string
//============================================================================
const char* sms_state_str(wms_service_ready_status_enum_v01 service_state)
{
   switch (service_state) {

      case WMS_SERVICE_READY_STATUS_NONE_V01:
         return "WMS_SERVICE_READY_STATUS_NONE";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP2_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP2";
         break;
      case WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2_V01:
         return "WMS_SERVICE_READY_STATUS_3GPP_AND_3GPP2";
         break;
      default:
         QCRIL_LOG_WARN("wms service state 0x%02x is unmapped", service_state);
         return "unmapped";
         break;
   }
}

//=========================================================================================
//qcril_qmi_sms_report_unsol_ims_state_change
//=========================================================================================
void qcril_qmi_sms_report_unsol_ims_state_change()
{
    qcril_unsol_resp_params_type unsol_resp;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&unsol_resp, 0, sizeof(unsol_resp));
    if ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_IMS) )
    {
        qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                    (int)RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED,
                                    &unsol_resp );
        qcril_send_unsol_response( &unsol_resp );
    }

    QCRIL_LOG_FUNC_RETURN();
}
