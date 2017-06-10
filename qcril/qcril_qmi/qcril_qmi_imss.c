/******************************************************************************
  @file    qcril_qmi_imss.c
  @brief   qcril qmi - IMS Setting

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI IMS Setting.

  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ---------------------------------------------------------------------------
******************************************************************************/

#include "qcril_qmi_client.h"
#include "qcril_qmi_imss.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_ims_socket.h"
#include "qcril_qmi_ims_misc.h"

//===========================================================================
//                    INTERNAL DEFINITIONS AND TYPES
//===========================================================================
struct ims_cached_info_type
{
    pthread_mutex_t       imss_info_lock_mutex;
    pthread_mutexattr_t   imss_info_lock_mutex_atr;

    uint8_t imss_reg_state_valid;
    Ims__Registration__RegState imss_reg_state;

    uint8_t imss_state_change_requested;
    Ims__Registration__RegState imss_new_reg_state;
};

//===========================================================================
//                     GLOBALS
//===========================================================================
static struct ims_cached_info_type   qcril_qmi_ims_cached_info;

//===========================================================================
// qcril_qmi_imss_init
//===========================================================================
void qcril_qmi_imss_init(void)
{
   pthread_mutexattr_init( &qcril_qmi_ims_cached_info.imss_info_lock_mutex_atr );
   pthread_mutex_init( &qcril_qmi_ims_cached_info.imss_info_lock_mutex,
                       &qcril_qmi_ims_cached_info.imss_info_lock_mutex_atr );

   qcril_qmi_imss_info_lock();
   qcril_qmi_ims_cached_info.imss_reg_state_valid = FALSE;
   qcril_qmi_ims_cached_info.imss_reg_state = IMS__REGISTRATION__REG_STATE__NOT_REGISTERED;
   qcril_qmi_ims_cached_info.imss_state_change_requested = FALSE;
   qcril_qmi_ims_cached_info.imss_new_reg_state = IMS__REGISTRATION__REG_STATE__NOT_REGISTERED;
   qcril_qmi_imss_info_unlock();

   //Get the intial config values
   qcril_qmi_imss_get_ims_reg_config();

} // qcril_qmi_imss_init

//===========================================================================
// qcril_qmi_imss_info_lock
//===========================================================================
void qcril_qmi_imss_info_lock()
{
    pthread_mutex_lock( &qcril_qmi_ims_cached_info.imss_info_lock_mutex );
} // qcril_qmi_imss_info_lock

//===========================================================================
// qcril_qmi_imss_info_unlock
//===========================================================================
void qcril_qmi_imss_info_unlock()
{
    pthread_mutex_unlock( &qcril_qmi_ims_cached_info.imss_info_lock_mutex );
} // qcril_qmi_imss_info_unlock

//===========================================================================
// qcril_qmi_imss_request_set_ims_registration
//===========================================================================
void qcril_qmi_imss_request_set_ims_registration
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
   Ims__Registration* ims_in_data_ptr = NULL;

   IxErrnoType qmi_client_error;
   RIL_Errno res = RIL_E_GENERIC_FAILURE;
   uint32 user_data;
   qcril_reqlist_public_type reqlist_entry;
   qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

   ims_settings_set_reg_mgr_config_req_msg_v01 qmi_req;
   ims_settings_set_reg_mgr_config_rsp_msg_v01 *qmi_resp = NULL;
   uint8_t qmi_imss_reg_state_valid = FALSE;
   Ims__Registration__RegState qmi_imss_reg_state = IMS__REGISTRATION__REG_STATE__NOT_REGISTERED;

   QCRIL_NOTUSED( ret_ptr );

   QCRIL_LOG_FUNC_ENTRY();

   do
   {
      qcril_qmi_imss_info_lock();
      qmi_imss_reg_state_valid = qcril_qmi_ims_cached_info.imss_reg_state_valid;
      qmi_imss_reg_state = qcril_qmi_ims_cached_info.imss_reg_state;
      qcril_qmi_imss_info_unlock();

      ims_in_data_ptr = (Ims__Registration *)params_ptr->data;
      QCRIL_LOG_INFO("has_state: %d, state: %d", ims_in_data_ptr->has_state, ims_in_data_ptr->state);

      if( TRUE == qmi_imss_reg_state_valid && qmi_imss_reg_state == ims_in_data_ptr->state )
      {
        QCRIL_LOG_INFO(".. No change in ims state");
        res = RIL_E_SUCCESS;
        qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, res);
      }
      else
      {
        qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                    QCRIL_EVT_NONE, NULL, &reqlist_entry );
        if ( qcril_reqlist_new( instance_id, &reqlist_entry ) == E_SUCCESS )
        {
            memset(&qmi_req, 0, sizeof(qmi_req));
            qmi_req.ims_test_mode_enabled_valid = TRUE;
            qmi_req.ims_test_mode_enabled = (IMS__REGISTRATION__REG_STATE__REGISTERED == ims_in_data_ptr->state) ? FALSE : TRUE;
            qmi_resp = qcril_malloc(sizeof(*qmi_resp));

            if (NULL == qmi_resp)
            {
                QCRIL_LOG_ERROR("qcril_malloc failed");
                break;
            }

            qcril_qmi_imss_info_lock();
            qcril_qmi_ims_cached_info.imss_state_change_requested = TRUE;
            qcril_qmi_ims_cached_info.imss_new_reg_state = ims_in_data_ptr->state;
            qcril_qmi_imss_info_unlock();

            user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

            qmi_client_error = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_IMS_SETTING,
                                                                QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_REQ_V01,
                                                                &qmi_req,
                                                                sizeof(qmi_req),
                                                                qmi_resp,
                                                                sizeof(*qmi_resp),
                                                                (void*)(uintptr_t)user_data );
            QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );

            if (E_SUCCESS == qmi_client_error)
            {
                res = RIL_E_SUCCESS;
            }
        }
      }
    }while(FALSE);

    if (RIL_E_SUCCESS != res)
    {
        qcril_send_empty_payload_request_response(instance_id, params_ptr->t, params_ptr->event_id, res);
        if( qmi_resp != NULL )
        {
           qcril_free( qmi_resp );
        }
    }

    if (ims_in_data_ptr)
    {
        qcril_qmi_ims__registration__free_unpacked(ims_in_data_ptr, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_request_set_ims_registration

//===========================================================================
// qcril_qmi_imss_request_set_ims_srv_status
//===========================================================================
void qcril_qmi_imss_request_set_ims_srv_status
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
    Ims__Info* ims_in_data_ptr = NULL;
    IxErrnoType qmi_client_error;
    RIL_Errno res = RIL_E_GENERIC_FAILURE;
    uint32 user_data;
    qcril_reqlist_public_type reqlist_entry;
    qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

    ims_settings_set_qipcall_config_req_msg_v01 qmi_req;
    ims_settings_set_qipcall_config_rsp_msg_v01 *qmi_resp = NULL;

    QCRIL_NOTUSED( ret_ptr );

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        ims_in_data_ptr = (Ims__Info *)params_ptr->data;

        qcril_reqlist_default_entry(
            params_ptr->t,
            params_ptr->event_id,
            QCRIL_DEFAULT_MODEM_ID,
            QCRIL_REQ_AWAITING_CALLBACK,
            QCRIL_EVT_NONE,
            NULL,
            &reqlist_entry );
        if ( qcril_reqlist_new( instance_id, &reqlist_entry ) == E_SUCCESS )
        {
            memset(&qmi_req, 0, sizeof(qmi_req));

            QCRIL_LOG_INFO( "has_calltype: %d, calltype: %d",
                            ims_in_data_ptr->has_calltype,
                            ims_in_data_ptr->calltype );
            if ( ims_in_data_ptr->has_calltype &&
                 ims_in_data_ptr->acctechstatus &&
                 ims_in_data_ptr->acctechstatus[0] && // only one acctechstatus will be used
                 ims_in_data_ptr->acctechstatus[0]->has_status )
            {
                QCRIL_LOG_INFO( "has_status: %d, status: %d",
                                ims_in_data_ptr->acctechstatus[0]->has_status,
                                ims_in_data_ptr->acctechstatus[0]->status );

                uint8_t enabled;
                if (IMS__STATUS_TYPE__STATUS_DISABLED == ims_in_data_ptr->acctechstatus[0]->status)
                {
                    enabled = FALSE;
                }
                else
                {
                    enabled = TRUE;
                }

                if (IMS__CALL_TYPE__CALL_TYPE_VOICE == ims_in_data_ptr->calltype)
                {
                    qmi_req.volte_enabled_valid = TRUE;
                    qmi_req.volte_enabled = enabled;
                }
                else if (IMS__CALL_TYPE__CALL_TYPE_VT == ims_in_data_ptr->calltype)
                {
                    qmi_req.vt_calling_enabled_valid = TRUE;
                    qmi_req.vt_calling_enabled = enabled;
                }
                else
                {
                    QCRIL_LOG_ERROR("request call_type is not a valid value");
                    break;
                }
            }
            else
            {
                QCRIL_LOG_ERROR("request misses some necessary information");
                break;
            }

            qmi_resp = qcril_malloc(sizeof(*qmi_resp));
            if (NULL == qmi_resp)
            {
                QCRIL_LOG_ERROR("qcril_malloc failed");
                break;
            }
            user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

            qmi_client_error = qcril_qmi_client_send_msg_async(
                                   QCRIL_QMI_CLIENT_IMS_SETTING,
                                   QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_REQ_V01,
                                   &qmi_req, sizeof(qmi_req),
                                   qmi_resp, sizeof(*qmi_resp),
                                   (void*)(uintptr_t)user_data );
            QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );

            if (E_SUCCESS == qmi_client_error)
            {
                res = RIL_E_SUCCESS;
            }
        }
    } while(FALSE);

    if (RIL_E_SUCCESS != res)
    {
        qcril_send_empty_payload_request_response(instance_id, params_ptr->t, params_ptr->event_id, res);
        if( qmi_resp != NULL )
        {
           qcril_free( qmi_resp );
        }
    }

    if (ims_in_data_ptr)
    {
        qcril_qmi_ims__info__free_unpacked(ims_in_data_ptr, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_request_set_ims_srv_status

//===========================================================================
// qcril_qmi_imss_request_query_vt_call_quality
//===========================================================================
void qcril_qmi_imss_request_query_vt_call_quality
(
 const qcril_request_params_type *const params_ptr,
 qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
    IxErrnoType               qmi_client_error;
    RIL_Errno                 res = RIL_E_GENERIC_FAILURE;
    uint32                    user_data;
    qcril_reqlist_public_type reqlist_entry;

    ims_settings_get_qipcall_config_rsp_msg_v01 *qmi_resp = NULL;

    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        qcril_reqlist_default_entry(params_ptr->t,
                params_ptr->event_id,
                QCRIL_DEFAULT_MODEM_ID,
                QCRIL_REQ_AWAITING_CALLBACK,
                QCRIL_EVT_NONE,
                NULL,
                &reqlist_entry);
        if (qcril_reqlist_new(QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry) == E_SUCCESS)
        {
            qmi_resp = qcril_malloc(sizeof(*qmi_resp));

            if (NULL == qmi_resp)
            {
                QCRIL_LOG_ERROR("qcril_malloc failed");
                break;
            }

            user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    reqlist_entry.req_id);

            qmi_client_error = qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_IMS_SETTING,
                    QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_REQ_V01,
                    NULL,
                    0,
                    qmi_resp,
                    sizeof(*qmi_resp),
                    (void*)(uintptr_t)user_data );
            QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error);

            if (E_SUCCESS == qmi_client_error)
            {
                res = RIL_E_SUCCESS;
            }
        }
    } while(FALSE);

    if (RIL_E_SUCCESS != res)
    {
        qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                params_ptr->t,
                params_ptr->event_id,
                res);
        if( qmi_resp != NULL )
        {
            qcril_free( qmi_resp );
        }
    }
} // qcril_qmi_imss_request_query_vt_call_quality

//===========================================================================
// qcril_qmi_imss_request_set_vt_call_quality
//===========================================================================
void qcril_qmi_imss_request_set_vt_call_quality
(
 const qcril_request_params_type *const params_ptr,
 qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
    Ims__VideoCallQuality    *ims_vcq_data_ptr = NULL;
    IxErrnoType               qmi_client_error;
    uint32                    user_data;
    qcril_reqlist_public_type reqlist_entry;
    RIL_Errno                 res = RIL_E_GENERIC_FAILURE;

    ims_settings_set_qipcall_config_req_msg_v01  qmi_req;
    ims_settings_set_qipcall_config_rsp_msg_v01 *qmi_resp = NULL;

    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        ims_vcq_data_ptr = (Ims__VideoCallQuality *)params_ptr->data;

        qcril_reqlist_default_entry( params_ptr->t,
                params_ptr->event_id,
                QCRIL_DEFAULT_MODEM_ID,
                QCRIL_REQ_AWAITING_CALLBACK,
                QCRIL_EVT_NONE,
                NULL,
                &reqlist_entry );
        if (qcril_reqlist_new(QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry) == E_SUCCESS)
        {
            memset(&qmi_req, 0, sizeof(qmi_req));

            QCRIL_LOG_INFO( "has_quality: %d, quality: %d",
                            ims_vcq_data_ptr->has_quality,
                            ims_vcq_data_ptr->quality );
            if (ims_vcq_data_ptr->has_quality)
            {
                qmi_req.vt_quality_selector_valid = TRUE;
                if (IMS__QUALITY__HIGH == ims_vcq_data_ptr->quality)
                {
                    qmi_req.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_0_V01;
                }
                else if (IMS__QUALITY__LOW == ims_vcq_data_ptr->quality)
                {
                    qmi_req.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_1_V01;
                }
                else
                {
                    QCRIL_LOG_ERROR("request quality is not a valid value");
                    break;
                }
            }
            else
            {
                QCRIL_LOG_ERROR("request misses some necessary information");
                break;
            }

            qmi_resp = qcril_malloc(sizeof(*qmi_resp));
            if (NULL == qmi_resp)
            {
                QCRIL_LOG_ERROR("qcril_malloc failed");
                break;
            }
            user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    reqlist_entry.req_id );

            qmi_client_error = qcril_qmi_client_send_msg_async( QCRIL_QMI_CLIENT_IMS_SETTING,
                    QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_REQ_V01,
                    &qmi_req,
                    sizeof(qmi_req),
                    qmi_resp,
                    sizeof(*qmi_resp),
                    (void*)(uintptr_t)user_data );
            QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );

            if (E_SUCCESS == qmi_client_error)
            {
                res = RIL_E_SUCCESS;
            }
        }
    } while(FALSE);

    if (RIL_E_SUCCESS != res)
    {
        qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                params_ptr->t,
                params_ptr->event_id,
                res);
        if(qmi_resp != NULL)
        {
           qcril_free(qmi_resp);
        }
    }

    if (ims_vcq_data_ptr)
    {
        qcril_qmi_ims__video_call_quality__free_unpacked(ims_vcq_data_ptr, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_request_set_vt_call_quality

//===========================================================================
// qcril_qmi_imss_request_get_wifi_calling_status
//===========================================================================
void qcril_qmi_imss_request_get_wifi_calling_status
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
    IxErrnoType               qmi_client_error;
    RIL_Errno                 res = RIL_E_GENERIC_FAILURE;
    uint32                    user_data;
    qcril_reqlist_public_type reqlist_entry;

    ims_settings_get_client_provisioning_config_rsp_msg_v01 *qmi_resp = NULL;

    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        qcril_reqlist_default_entry(params_ptr->t,
                params_ptr->event_id,
                QCRIL_DEFAULT_MODEM_ID,
                QCRIL_REQ_AWAITING_CALLBACK,
                QCRIL_EVT_NONE,
                NULL,
                &reqlist_entry);
        if (qcril_reqlist_new(QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry) == E_SUCCESS)
        {
            qmi_resp = qcril_malloc(sizeof(*qmi_resp));

            if (NULL == qmi_resp)
            {
                QCRIL_LOG_ERROR("qcril_malloc failed");
                break;
            }

            user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    reqlist_entry.req_id);

            qmi_client_error = qcril_qmi_client_send_msg_async(QCRIL_QMI_CLIENT_IMS_SETTING,
                    QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01,
                    NULL,
                    0,
                    qmi_resp,
                    sizeof(*qmi_resp),
                    (void*)(uintptr_t)user_data );
            QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error);

            if (E_SUCCESS == qmi_client_error)
            {
                res = RIL_E_SUCCESS;
            }
        }
    } while(FALSE);

    if (RIL_E_SUCCESS != res)
    {
        qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                params_ptr->t,
                params_ptr->event_id,
                res);
        if( qmi_resp != NULL )
        {
            qcril_free( qmi_resp );
        }
    }
} // qcril_qmi_imss_request_get_wifi_calling_status

//===========================================================================
// qcril_qmi_imss_request_set_wifi_calling_status
//===========================================================================
void qcril_qmi_imss_request_set_wifi_calling_status
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
    Ims__WifiCallingInfo     *ims_wci_data_ptr = NULL;
    IxErrnoType               qmi_client_error;
    uint32                    user_data;
    qcril_reqlist_public_type reqlist_entry;
    RIL_Errno                 res = RIL_E_GENERIC_FAILURE;

    ims_settings_set_client_provisioning_config_req_msg_v01  qmi_req;
    ims_settings_set_client_provisioning_config_rsp_msg_v01 *qmi_resp = NULL;

    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        ims_wci_data_ptr = (Ims__WifiCallingInfo *)params_ptr->data;

        qcril_reqlist_default_entry( params_ptr->t,
                params_ptr->event_id,
                QCRIL_DEFAULT_MODEM_ID,
                QCRIL_REQ_AWAITING_CALLBACK,
                QCRIL_EVT_NONE,
                NULL,
                &reqlist_entry );
        if (qcril_reqlist_new(QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry) == E_SUCCESS)
        {
            memset(&qmi_req, 0, sizeof(qmi_req));

            QCRIL_LOG_INFO("has_status: %d, status: %d "
                    "has_preference: %d, preference: %d",
                    ims_wci_data_ptr->has_status, ims_wci_data_ptr->status,
                    ims_wci_data_ptr->has_preference, ims_wci_data_ptr->preference);

            if (ims_wci_data_ptr->has_status)
            {
                qmi_req.wifi_call_valid =
                    qcril_qmi_ims_map_wificallingstatus_to_ims_settings_wfc_status(
                            ims_wci_data_ptr->status, &qmi_req.wifi_call);
            }
            if (ims_wci_data_ptr->has_preference)
            {
                qmi_req.wifi_call_preference_valid =
                    qcril_qmi_ims_map_wificallingpreference_to_ims_settings_wfc_preference(
                            ims_wci_data_ptr->preference, &qmi_req.wifi_call_preference);
            }

            if (!qmi_req.wifi_call_valid && !qmi_req.wifi_call_preference_valid)
            {
                QCRIL_LOG_ERROR("request misses some necessary information");
                break;
            }

            qmi_resp = qcril_malloc(sizeof(*qmi_resp));
            if (NULL == qmi_resp)
            {
                QCRIL_LOG_ERROR("qcril_malloc failed");
                break;
            }
            user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
                    QCRIL_DEFAULT_MODEM_ID,
                    reqlist_entry.req_id );

            qmi_client_error = qcril_qmi_client_send_msg_async( QCRIL_QMI_CLIENT_IMS_SETTING,
                    QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01,
                    &qmi_req,
                    sizeof(qmi_req),
                    qmi_resp,
                    sizeof(*qmi_resp),
                    (void*)(uintptr_t)user_data );
            QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );

            if (E_SUCCESS == qmi_client_error)
            {
                res = RIL_E_SUCCESS;
            }
        }
    } while(FALSE);

    if (RIL_E_SUCCESS != res)
    {
        qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                params_ptr->t,
                params_ptr->event_id,
                res);
        if(qmi_resp != NULL)
        {
           qcril_free(qmi_resp);
        }
    }

    if (ims_wci_data_ptr)
    {
        qcril_qmi_ims__wifi_calling_info__free_unpacked(ims_wci_data_ptr, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
}  // qcril_qmi_imss_request_set_wifi_calling_status

//===========================================================================
// qcril_qmi_imss_get_ims_reg_config
//===========================================================================
void qcril_qmi_imss_get_ims_reg_config()
{
   IxErrnoType qmi_client_error;
   ims_settings_get_reg_mgr_config_rsp_msg_v01 qmi_ims_get_reg_config_resp;

   QCRIL_LOG_FUNC_ENTRY();

   memset(&qmi_ims_get_reg_config_resp, 0x0, sizeof(qmi_ims_get_reg_config_resp));
   qmi_client_error = qcril_qmi_client_send_msg_sync ( QCRIL_QMI_CLIENT_IMS_SETTING,
                                                       QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01,
                                                       NULL,
                                                       0,
                                                       &qmi_ims_get_reg_config_resp,
                                                       sizeof(qmi_ims_get_reg_config_resp) );
   QCRIL_LOG_INFO(".. qmi send sync res %d", (int) qmi_client_error );

   if (E_SUCCESS == qmi_client_error)
   {
      if( qmi_ims_get_reg_config_resp.resp.result == QMI_RESULT_SUCCESS_V01 )
      {
          qcril_qmi_imss_info_lock();
          qcril_qmi_ims_cached_info.imss_reg_state_valid = qmi_ims_get_reg_config_resp.ims_test_mode_valid;
          qcril_qmi_ims_cached_info.imss_reg_state = (qmi_ims_get_reg_config_resp.ims_test_mode == TRUE ) ? IMS__REGISTRATION__REG_STATE__NOT_REGISTERED : IMS__REGISTRATION__REG_STATE__REGISTERED;
          qcril_qmi_imss_info_unlock();
          QCRIL_LOG_INFO(".. IMS has_state: %d, state: %d", qcril_qmi_ims_cached_info.imss_reg_state_valid, qcril_qmi_ims_cached_info.imss_reg_state);
      }
   }

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_get_ims_reg_config

//===========================================================================
// qcril_qmi_imss_set_reg_mgr_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_set_reg_mgr_config_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
   ims_settings_set_reg_mgr_config_rsp_msg_v01 *resp;
   RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

   QCRIL_LOG_FUNC_ENTRY();

   if (NULL != params_ptr)
   {
      resp = params_ptr->data;

      if (NULL != resp)
      {
         ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR, &(resp->resp));
      }
      else
      {
         QCRIL_LOG_ERROR("params_ptr->data is NULL");
      }

      if( RIL_E_SUCCESS == ril_err )
      {
          if( TRUE == qcril_qmi_ims_cached_info.imss_state_change_requested )
          {
              qcril_qmi_imss_info_lock();
              qcril_qmi_ims_cached_info.imss_reg_state_valid = TRUE;
              qcril_qmi_ims_cached_info.imss_reg_state = qcril_qmi_ims_cached_info.imss_new_reg_state;
              qcril_qmi_ims_cached_info.imss_state_change_requested = FALSE;
              qcril_qmi_imss_info_unlock();
              QCRIL_LOG_INFO(".. IMS state changed to %d\n", qcril_qmi_ims_cached_info.imss_reg_state);
          }
      }
      else
      {
          qcril_qmi_imss_info_lock();
          qcril_qmi_ims_cached_info.imss_state_change_requested = FALSE;
          qcril_qmi_imss_info_unlock();
          QCRIL_LOG_INFO(".. Failed to change IMS state and remains in state %d\n", qcril_qmi_ims_cached_info.imss_reg_state);
      }

      qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_err);
   }
   else
   {
      QCRIL_LOG_ERROR("params_ptr is NULL");
   }

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_set_reg_mgr_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_set_qipcall_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_set_qipcall_config_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
    ims_settings_set_qipcall_config_rsp_msg_v01 *resp;
    RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    if (NULL != params_ptr)
    {
        resp = params_ptr->data;

        if (NULL != resp)
        {
            ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR, &(resp->resp));
        }
        else
        {
            QCRIL_LOG_ERROR("params_ptr->data is NULL");
        }
        qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_err);
    }
    else
    {
        QCRIL_LOG_ERROR("params_ptr is NULL");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_set_qipcall_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_get_qipcall_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_get_qipcall_config_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
    ims_settings_get_qipcall_config_rsp_msg_v01 *resp = NULL;
    RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    if (NULL != params_ptr)
    {
        resp = params_ptr->data;

        if (NULL != resp)
        {
            ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR,
                    &(resp->resp));
        }
        else
        {
            QCRIL_LOG_ERROR("params_ptr->data is NULL");
        }

        if (RIL_E_SUCCESS == ril_err)
        {
            if (QCRIL_EVT_IMS_SOCKET_REQ_QUERY_VT_CALL_QUALITY == params_ptr->event_id)
            {
                Ims__VideoCallQuality video_quality = IMS__VIDEO_CALL_QUALITY__INIT;

                QCRIL_LOG_INFO( "vt_quality_selector_valid: %d, vt_quality_selector: %d",
                                resp->vt_quality_selector_valid,
                                resp->vt_quality_selector );

                if (resp->vt_quality_selector_valid)
                {
                    if (resp->vt_quality_selector == IMS_SETTINGS_VT_QUALITY_LEVEL_0_V01)
                    {
                        video_quality.has_quality = TRUE;
                        video_quality.quality = IMS__QUALITY__HIGH;
                    }
                    else if (resp->vt_quality_selector == IMS_SETTINGS_VT_QUALITY_LEVEL_1_V01)
                    {
                        video_quality.has_quality = TRUE;
                        video_quality.quality = IMS__QUALITY__LOW;
                    }
                    else
                    {
                        ril_err = RIL_E_GENERIC_FAILURE;
                        QCRIL_LOG_ERROR("Unexpected value from modem\n");
                    }
                    qcril_qmi_ims_socket_send(params_ptr->t,
                        IMS__MSG_TYPE__RESPONSE,
                        IMS__MSG_ID__REQUEST_QUERY_VT_CALL_QUALITY,
                        qcril_qmi_ims_map_ril_error_to_ims_error(ril_err),
                        (void *)&video_quality,
                        sizeof(video_quality));
                }
            }
        }
        else
        {
            qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                    params_ptr->t,
                    params_ptr->event_id,
                    ril_err);
        }
    }
    else
    {
        QCRIL_LOG_ERROR("params_ptr is NULL");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_get_qipcall_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_set_client_provisioning_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_set_client_provisioning_config_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
    ims_settings_set_client_provisioning_config_rsp_msg_v01 *resp;
    RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    if (NULL != params_ptr)
    {
        resp = params_ptr->data;

        if (NULL != resp)
        {
            ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                    QMI_NO_ERR, &(resp->resp));
            QCRIL_LOG_INFO("ril_err: %d, qmi res: %d", (int) ril_err,
                    (int)resp->resp.error);
        }
        else
        {
            QCRIL_LOG_ERROR("params_ptr->data is NULL");
        }
        qcril_send_empty_payload_request_response(
                QCRIL_DEFAULT_INSTANCE_ID,
                params_ptr->t,
                params_ptr->event_id,
                ril_err);
    }
    else
    {
        QCRIL_LOG_ERROR("params_ptr is NULL");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_set_client_provisioning_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_get_client_provisioning_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_get_client_provisioning_config_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
    ims_settings_get_client_provisioning_config_rsp_msg_v01 *resp = NULL;
    RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    if (NULL != params_ptr)
    {
        resp = params_ptr->data;

        if (NULL != resp)
        {
            ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR,
                    &(resp->resp));
            QCRIL_LOG_INFO("ril_err: %d, qmi res: %d", (int) ril_err, (int)resp->resp.error);
        }
        else
        {
            QCRIL_LOG_ERROR("params_ptr->data is NULL");
        }

        if (RIL_E_SUCCESS == ril_err)
        {
            if (QCRIL_EVT_IMS_SOCKET_REQ_GET_WIFI_CALLING_STATUS == params_ptr->event_id)
            {
                Ims__WifiCallingInfo wifi_calling_info = IMS__WIFI_CALLING_INFO__INIT;

                QCRIL_LOG_INFO("wifi_call_valid: %d, wifi_call: %d "
                        "wifi_call_preference_valid: %d, wifi_call_preference: %d",
                        resp->wifi_call_valid, resp->wifi_call,
                        resp->wifi_call_preference_valid, resp->wifi_call_preference);

                if (resp->wifi_call_valid)
                {
                    wifi_calling_info.has_status =
                        qcril_qmi_ims_map_ims_settings_wfc_status_to_wificallingstatus(
                                resp->wifi_call, &wifi_calling_info.status);
                }
                if (resp->wifi_call_preference_valid)
                {
                    wifi_calling_info.has_preference =
                        qcril_qmi_ims_map_ims_settings_wfc_preference_to_wificallingpreference(
                                resp->wifi_call_preference, &wifi_calling_info.preference);
                }
                qcril_qmi_ims_socket_send(params_ptr->t,
                        IMS__MSG_TYPE__RESPONSE,
                        qcril_qmi_ims_map_event_to_request(params_ptr->event_id),
                        qcril_qmi_ims_map_ril_error_to_ims_error(ril_err),
                        (void *)&wifi_calling_info,
                        sizeof(wifi_calling_info));
            }
        }
        else
        {
            qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                    params_ptr->t,
                    params_ptr->event_id,
                    ril_err);
        }
    }
    else
    {
        QCRIL_LOG_ERROR("params_ptr is NULL");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_get_client_provisioning_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_command_cb_helper
//===========================================================================
void qcril_qmi_imss_command_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
   qcril_instance_id_e_type instance_id;
   uint32 user_data;
   uint16 req_id;
   qcril_reqlist_public_type req_info;
   qcril_request_params_type req_data;
   qmi_resp_callback_type * qmi_resp_callback;

   QCRIL_LOG_FUNC_ENTRY();
   QCRIL_NOTUSED(ret_ptr);

   qmi_resp_callback = (qmi_resp_callback_type *) params_ptr->data;
   if( qmi_resp_callback )
   {
      if (qmi_resp_callback->data_buf != NULL)
      {
         user_data = ( uint32 )(uintptr_t) qmi_resp_callback->cb_data;
         instance_id = QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data );
         req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( user_data );

         memset(&req_data, 0, sizeof(req_data));
         req_data.modem_id = QCRIL_DEFAULT_MODEM_ID;
         req_data.instance_id = instance_id;
         req_data.datalen = qmi_resp_callback->data_buf_len;
         req_data.data = qmi_resp_callback->data_buf;

         /* Lookup the Token ID */
         if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
         {
            if( qmi_resp_callback->transp_err != QMI_NO_ERR )
            {
               QCRIL_LOG_INFO("Transp error (%d) recieved from QMI for RIL request %d", qmi_resp_callback->transp_err, req_info.request);
               /* Send GENERIC_FAILURE response */
               qcril_send_empty_payload_request_response( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE );
            }
            else
            {
               req_data.t = req_info.t;
               req_data.event_id = req_info.request;
               switch(qmi_resp_callback->msg_id)
               {
               case QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_RSP_V01:
                  qcril_qmi_imss_set_reg_mgr_config_resp_hdlr(&req_data);
                  break;

               case QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_RSP_V01:
                  qcril_qmi_imss_set_qipcall_config_resp_hdlr(&req_data);
                  break;

               case QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_RSP_V01:
                  qcril_qmi_imss_get_qipcall_config_resp_hdlr(&req_data);
                  break;

               case QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01:
                  qcril_qmi_imss_set_client_provisioning_config_resp_hdlr(&req_data);
                  break;

               case QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01:
                  qcril_qmi_imss_get_client_provisioning_config_resp_hdlr(&req_data);
                  break;

               default:
                  QCRIL_LOG_INFO("Unsupported QMI IMSS message %d", qmi_resp_callback->msg_id);
                  break;
               }
            }
         }
         else
         {
            QCRIL_LOG_ERROR( "Req ID: %d not found", req_id );
         }

         qcril_free( qmi_resp_callback->data_buf );
      }
      else
      {
         QCRIL_LOG_ERROR("qmi_resp_callback->data_buf is NULL");
      }
   }
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_command_cb_helper

//===========================================================================
// qcril_qmi_imss_command_cb
//===========================================================================
void qcril_qmi_imss_command_cb
(
   qmi_client_type              user_handle,
   unsigned int                 msg_id,
   void                        *resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
   qmi_resp_callback_type qmi_resp_callback;
   memset(&qmi_resp_callback, 0, sizeof(qmi_resp_callback));
   QCRIL_LOG_FUNC_ENTRY();
   QCRIL_LOG_INFO(".. msg id %.2x", (int) msg_id );
   qmi_resp_callback.user_handle = user_handle;
   qmi_resp_callback.msg_id = msg_id;
   qmi_resp_callback.data_buf = (void*) resp_c_struct;
   qmi_resp_callback.data_buf_len = resp_c_struct_len;
   qmi_resp_callback.cb_data = resp_cb_data;
   qmi_resp_callback.transp_err = transp_err;
   qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                      QCRIL_DEFAULT_MODEM_ID,
                      QCRIL_DATA_ON_STACK,
                      QCRIL_EVT_QMI_IMSS_HANDLE_COMM_CALLBACKS,
                      (void*) &qmi_resp_callback,
                      sizeof(qmi_resp_callback),
                      (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_command_cb
