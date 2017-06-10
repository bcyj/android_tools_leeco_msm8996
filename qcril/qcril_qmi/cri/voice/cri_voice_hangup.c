/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_hangup.h"
#include "cri_voice_core.h"
#include "cri_voice_call_summary.h"
#include "cri_core.h"
#include "voice_service_v02.h"
#include "cri_rule_handler.h"

static cri_core_error_type cri_voice_hangup_with_call_cri_id(cri_core_context_type cri_core_context, const void* user_data, uint8 hlos_call_id);
static cri_core_error_type cri_voice_hangup_background(cri_core_context_type cri_core_context, const void* user_data);
static cri_core_error_type cri_voice_hangup_foreground_resume_background(cri_core_context_type cri_core_context, const void* user_data);
static void cri_voice_hangup_resp_wapper(cri_core_context_type context,
                                  cri_core_error_type cri_core_error,
                                  void *hlos_cb_data,
                                  void *cri_resp_data);

// TODO: add rules in params
cri_core_error_type cri_voice_core_send_manage_ip_call_request(cri_core_context_type cri_core_context, const void *user_data, hlos_resp_cb_type cb, voip_sups_type_enum_v02 sups_type)
{
    cri_core_error_type err = 0;

    voice_manage_ip_calls_req_msg_v02 manage_voip_calls_req;
    memset(&manage_voip_calls_req, 0, sizeof(manage_voip_calls_req));
    manage_voip_calls_req.sups_type = sups_type;

    err =  cri_core_qmi_send_msg_async( cri_core_context,
                                        cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                                        QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                        &manage_voip_calls_req,
                                        sizeof(manage_voip_calls_req),
                                        sizeof(voice_manage_ip_calls_resp_msg_v02),
                                        user_data,
                                        cb,
                                        CRI_CORE_MAX_TIMEOUT,
                                        NULL );
    return err;
}

// TODO: add rules in params
cri_core_error_type cri_voice_core_send_manage_call_request(cri_core_context_type cri_core_context, const void *user_data, hlos_resp_cb_type cb, sups_type_enum_v02 sups_type)
{
    cri_core_error_type err = 0;

    voice_manage_calls_req_msg_v02 manage_calls_req;
    memset(&manage_calls_req, 0, sizeof(manage_calls_req));
    manage_calls_req.sups_type = sups_type;

    err =  cri_core_qmi_send_msg_async( cri_core_context,
                                        cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                                        QMI_VOICE_MANAGE_CALLS_REQ_V02,
                                        &manage_calls_req,
                                        sizeof(manage_calls_req),
                                        sizeof(voice_manage_calls_resp_msg_v02),
                                        user_data,
                                        cb,
                                        CRI_CORE_MAX_TIMEOUT,
                                        NULL );
    return err;
}


int cri_voice_hangup_end_call_rule_checker(void *rule_data)
{
    int ret_code = FALSE;

    int8 qmi_call_id = (intptr_t) rule_data;
    cri_voice_call_obj_type* call_obj_ptr = cri_voice_call_list_find_by_qmi_call_id(cri_voice_core_get_call_list(), qmi_call_id);

    QCRIL_LOG_INFO("qmi_call_id: %d, call_obj_ptr: %p", qmi_call_id, call_obj_ptr);

    if (call_obj_ptr && CALL_STATE_END_V02 == call_obj_ptr->qmi_voice_scv_info.call_state)
    {
        ret_code = TRUE;
    }

    return ret_code;
}


cri_core_error_type cri_voice_core_send_end_call_request(cri_core_context_type cri_core_context, const void *user_data, hlos_resp_cb_type cb, uint8_t qmi_id)
{
    cri_core_error_type err = 0;

    voice_end_call_req_msg_v02 end_call_req;
    memset(&end_call_req, 0, sizeof(end_call_req));
    end_call_req.call_id = qmi_id;

    cri_rule_handler_user_rule_info_type user_rule_info;
    memset(&user_rule_info, 0, sizeof(user_rule_info));
    user_rule_info.rule_data = (void*)(intptr_t)qmi_id;
    user_rule_info.rule_check_handler = cri_voice_hangup_end_call_rule_checker;

    err =  cri_core_qmi_send_msg_async( cri_core_context,
                                        cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                                        QMI_VOICE_END_CALL_REQ_V02,
                                        &end_call_req,
                                        sizeof(end_call_req),
                                        sizeof(voice_end_call_resp_msg_v02),
                                        user_data,
                                        cb,
                                        CRI_CORE_MAX_TIMEOUT,
                                        &user_rule_info );
    return err;
}

typedef struct
{
    void* user_data;
    cri_voice_request_hangup_cb_type hangup_cb;
} cri_voice_hangup_hlos_cb_data_wrapper_type;

cri_core_error_type cri_voice_hangup_req_handler(cri_core_context_type cri_core_context, const cri_voice_call_hangup_request_type *req_message, const void *user_data, cri_voice_request_hangup_cb_type hangup_cb)
{
    cri_voice_hangup_type_type hangup_type = req_message->hangup_type;
    boolean call_uri_valid = req_message->call_uri_valid;
    const char* call_uri = req_message->call_uri;
    boolean call_hlos_id_valid = req_message->call_hlos_id_valid;
    uint8   call_hlos_id = req_message->call_hlos_id;
    boolean hangup_cause_valid = req_message->hangup_cause_valid;
    int hangup_cause  = req_message->hangup_cause;

    cri_core_error_type err = QMI_ERR_INTERNAL_V01;

    cri_voice_hangup_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr = util_memory_alloc(sizeof(*hlos_cb_data_wrapper_ptr));
    if (!hlos_cb_data_wrapper_ptr)
    {
        err = QMI_ERR_INTERNAL_V01;
    }
    else
    {
        hlos_cb_data_wrapper_ptr->user_data = (void*)user_data;
        hlos_cb_data_wrapper_ptr->hangup_cb = hangup_cb;

        switch (hangup_type)
        {
        case CRI_VOICE_HANGUP_ALL_CALLS:
            err = QMI_ERR_INTERNAL_V01;
            break;
        case CRI_VOICE_HANGUP_ALL_NONE_EMERGENCY_CALLS:
            err = QMI_ERR_INTERNAL_V01;
            break;
        case CRI_VOICE_HANGUP_FOREGROUND_CALLS_RESUME_BACKGROUND_CALLS:
            err = cri_voice_hangup_foreground_resume_background(cri_core_context, hlos_cb_data_wrapper_ptr);
            break;
        case CRI_VOICE_HANGUP_BACKGROUND_CALLS:
            err = cri_voice_hangup_background(cri_core_context, hlos_cb_data_wrapper_ptr);
            break;
        case CRI_VOICE_HANGUP_WITH_CALL_HLOS_ID:
            if (call_hlos_id_valid)
            {
                err = cri_voice_hangup_with_call_cri_id(cri_core_context, hlos_cb_data_wrapper_ptr, call_hlos_id);
            }
            else
            {
                QCRIL_LOG_ERROR("CRI_VOICE_CORE_HANGUP_WITH_CALL_HLOS_ID without hlos id specified");
                err = QMI_ERR_INTERNAL_V01;
            }
            break;
        case CRI_VOICE_HANGUP_WITH_CALL_URI:
            err = QMI_ERR_INTERNAL_V01;
            break;
        case CRI_VOICE_HANGUP_WITH_CALL_HLOS_ID_AND_URI:
            err = QMI_ERR_INTERNAL_V01;
            break;
        default:
            err = QMI_ERR_INTERNAL_V01;
            break;
        }

        if (err)
        {
            util_memory_free((void**) hlos_cb_data_wrapper_ptr);
        }
    }

//    QCRIL_LOG_FUNC_RETURN_WITH_RET(err);
    return err;
}

void cri_voice_hangup_resp_wapper(cri_core_context_type context,
                                  cri_core_error_type cri_core_error,
                                  void *hlos_cb_data,
                                  void *cri_resp_data)
{
    QCRIL_LOG_FUNC_ENTRY();
    cri_voice_hangup_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr = (cri_voice_hangup_hlos_cb_data_wrapper_type *)hlos_cb_data;
    if (hlos_cb_data_wrapper_ptr)
    {
        if (hlos_cb_data_wrapper_ptr->hangup_cb)
        {
            hlos_cb_data_wrapper_ptr->hangup_cb(context, cri_core_error, hlos_cb_data_wrapper_ptr->user_data);
        }
        util_memory_free((void**)&hlos_cb_data_wrapper_ptr);
    }
    QCRIL_LOG_FUNC_RETURN();
}


cri_core_error_type cri_voice_hangup_foreground_resume_background(cri_core_context_type cri_core_context, const void* user_data)
{
    cri_core_error_type err = 0;

    cri_voice_call_summary_type call_summary;
    cri_voice_call_summary_get_modom_call_summary(cri_voice_core_get_call_list(), &call_summary);

    do
    {
        if (call_summary.nof_fg_calls < 1)
        {
            err = QMI_ERR_INTERNAL_V01;
            QCRIL_LOG_DEBUG("do not have a foreground call");
            break;
        }

        if (1 == call_summary.nof_calls_overall)
        {
            uint32 num_of_calls;
            cri_voice_call_obj_type** call_obj_dptr;
            cri_voice_call_list_get_filtered_call_objects(cri_voice_core_get_call_list(), cri_voice_call_obj_is_modem_call, &num_of_calls, &call_obj_dptr);
            if (num_of_calls && call_obj_dptr)
            {
                if (1 == num_of_calls)
                {
                    uint8 qmi_id = call_obj_dptr[0]->qmi_call_id;
                    err = cri_voice_core_send_end_call_request(cri_core_context, user_data, cri_voice_hangup_resp_wapper, qmi_id);
                    QCRIL_LOG_INFO("send end_call request result: %d", err);
                }
                else
                {
                    QCRIL_LOG_ERROR("num_of_calls != 1");
                }
                util_memory_free((void**) &call_obj_dptr);
            }
            else
            {
                QCRIL_LOG_ERROR("num_of_calls == 0 or call_obj_dptr == NULL");
            }
        }
        else if (call_summary.nof_ps_calls)
        {
            err = cri_voice_core_send_manage_ip_call_request(cri_core_context, user_data, cri_voice_hangup_resp_wapper, VOIP_SUPS_TYPE_RELEASE_ACTIVE_ACCEPT_HELD_OR_WAITING_V02);
            QCRIL_LOG_INFO("send manangement_ip_call request result: %d", err);
        }
        else
        {
            err = cri_voice_core_send_manage_call_request(cri_core_context, user_data, cri_voice_hangup_resp_wapper, SUPS_TYPE_RELEASE_ACTIVE_ACCEPT_HELD_OR_WAITING_V02);
            QCRIL_LOG_INFO("send manangement_call request result: %d", err);
        }
    }
    while (0);

    return err;
}

cri_core_error_type cri_voice_hangup_background(cri_core_context_type cri_core_context, const void* user_data)
{
    cri_core_error_type err = 0;

    cri_voice_call_summary_type call_summary;
    cri_voice_call_summary_get_modom_call_summary(cri_voice_core_get_call_list(), &call_summary);

    do
    {
        if ( call_summary.nof_bg_calls < 1)
        {
            err = QMI_ERR_INTERNAL_V01;
            QCRIL_LOG_DEBUG("do not have a background call");
            break;
        }

        if (1 == call_summary.nof_calls_overall)
        {
            uint32 num_of_calls;
            cri_voice_call_obj_type** call_obj_dptr;
            cri_voice_call_list_get_filtered_call_objects(cri_voice_core_get_call_list(), cri_voice_call_obj_is_modem_call, &num_of_calls, &call_obj_dptr);
            if (num_of_calls && call_obj_dptr)
            {
                if (1 == num_of_calls)
                {
                    uint8 qmi_id = call_obj_dptr[0]->qmi_call_id;
                    err = cri_voice_core_send_end_call_request(cri_core_context, user_data, cri_voice_hangup_resp_wapper, qmi_id);
                    QCRIL_LOG_INFO("send end_call request result: %d", err);
                }
                else
                {
                    QCRIL_LOG_ERROR("num_of_calls != 1");
                }
                util_memory_free((void**) &call_obj_dptr);
            }
            else
            {
                QCRIL_LOG_ERROR("num_of_calls == 0 or call_obj_dptr == NULL");
            }
        }
        else if (call_summary.nof_ps_calls)
        {
            err = cri_voice_core_send_manage_ip_call_request(cri_core_context, user_data, cri_voice_hangup_resp_wapper, VOIP_SUPS_TYPE_RELEASE_HELD_OR_WAITING_V02);
            QCRIL_LOG_INFO("send manangement_ip_call request result: %d", err);
        }
        else
        {
            err = cri_voice_core_send_manage_call_request(cri_core_context, user_data, cri_voice_hangup_resp_wapper, SUPS_TYPE_RELEASE_HELD_OR_WAITING_V02);
            QCRIL_LOG_INFO("send manangement_call request result: %d", err);
        }
    }
    while (0);

    return err;
}

cri_core_error_type cri_voice_hangup_with_call_cri_id(cri_core_context_type cri_core_context, const void* user_data, uint8 cri_call_id)
{
    cri_core_error_type err = 0;

    cri_voice_call_obj_type* call_obj_ptr = cri_voice_call_list_find_by_cri_call_id(cri_voice_core_get_call_list(), cri_call_id);

    if (call_obj_ptr)
    {
        cri_voice_core_send_end_call_request(cri_core_context, user_data, cri_voice_hangup_resp_wapper, call_obj_ptr->qmi_call_id);
    }
    else
    {
        err = QMI_ERR_INTERNAL_V01;
        QCRIL_LOG_DEBUG("did not find the call with hlos call id %d", cri_call_id);
    }
    return err;
}
