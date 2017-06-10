/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_multi_calls.h"
#include "cri_voice_core.h"
#include "cri_voice_call_summary.h"

static void cri_voice_multi_calls_switch_resp_wapper(
    cri_core_context_type context,
    cri_core_error_type cri_core_error,
    void *hlos_cb_data,
    void *cri_resp_data
);

typedef struct
{
    void* user_data;
    cri_voice_request_switch_calls_cb_type switch_cb;
} cri_voice_multi_calls_switch_hlos_cb_data_wrapper_type;

static void cri_voice_multi_calls_conference_resp_wapper(
    cri_core_context_type context,
    cri_core_error_type cri_core_error,
    void *hlos_cb_data,
    void *cri_resp_data
);

typedef struct
{
    void* user_data;
    cri_voice_request_conference_cb_type conf_cb;
} cri_voice_multi_calls_conference_hlos_cb_data_wrapper_type;

static void cri_voice_multi_calls_separate_conn_resp_wapper(
    cri_core_context_type context,
    cri_core_error_type cri_core_error,
    void *hlos_cb_data,
    void *cri_resp_data
);

typedef struct
{
    void* user_data;
    cri_voice_request_separate_conn_cb_type seperate_conn_cb;
} cri_voice_multi_calls_separate_conn_hlos_cb_data_wrapper_type;

cri_core_error_type cri_voice_multi_calls_switch_req_handler(
    cri_core_context_type cri_core_context,
    const cri_voice_switch_calls_request_type *req_message_ptr,
    const void *user_data,
    cri_voice_request_switch_calls_cb_type switch_cb
)
{
    cri_core_error_type err = QMI_ERR_NONE_V01;

    cri_voice_call_summary_type call_summary;
    cri_voice_call_summary_get_modom_call_summary(cri_voice_core_get_call_list(), &call_summary);

    do
    {
        if (!call_summary.nof_calls_overall)
        {
            err = QMI_ERR_INTERNAL_V01;
            break;
        }

        cri_voice_multi_calls_switch_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr =
            util_memory_alloc(sizeof(*hlos_cb_data_wrapper_ptr));
        if (!hlos_cb_data_wrapper_ptr)
        {
            err = QMI_ERR_INTERNAL_V01;
            break;
        }

        hlos_cb_data_wrapper_ptr->user_data = (void*)user_data;
        hlos_cb_data_wrapper_ptr->switch_cb = switch_cb;


        if (call_summary.nof_ps_calls)
        {
            voice_manage_ip_calls_req_msg_v02 manage_voip_calls_req;
            memset(&manage_voip_calls_req, 0, sizeof(manage_voip_calls_req));
            manage_voip_calls_req.sups_type = VOIP_SUPS_TYPE_HOLD_ACTIVE_ACCEPT_WAITING_OR_HELD_V02;

            boolean has_call_type = req_message_ptr->has_call_type;
            cri_voice_call_type_type call_type = req_message_ptr->call_type;

            if (has_call_type)
            {
                cri_voice_call_obj_type* call_obj_ptr =
                    cri_voice_find_call_object_by_cri_call_state(CRI_VOICE_CALL_STATE_WAITING);
                if (call_obj_ptr)
                {
                    manage_voip_calls_req.call_id_valid = TRUE;
                    manage_voip_calls_req.call_id = call_obj_ptr->qmi_call_id;
                    // covert cri call type to qmi call type and attribute
                }
            }
            err = cri_core_qmi_send_msg_async(
                      cri_core_context,
                      cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                      QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                      &manage_voip_calls_req,
                      sizeof(manage_voip_calls_req),
                      sizeof(voice_manage_ip_calls_resp_msg_v02),
                      hlos_cb_data_wrapper_ptr,
                      cri_voice_multi_calls_switch_resp_wapper,
                      CRI_CORE_MAX_TIMEOUT,
                      NULL );
        }
        else
        {
            voice_manage_calls_req_msg_v02 manage_calls_req;
            memset(&manage_calls_req, 0, sizeof(manage_calls_req));
            manage_calls_req.sups_type = SUPS_TYPE_HOLD_ACTIVE_ACCEPT_WAITING_OR_HELD_V02;
            err = cri_core_qmi_send_msg_async(
                      cri_core_context,
                      cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                      QMI_VOICE_MANAGE_CALLS_REQ_V02,
                      &manage_calls_req,
                      sizeof(manage_calls_req),
                      sizeof(voice_manage_calls_resp_msg_v02),
                      hlos_cb_data_wrapper_ptr,
                      cri_voice_multi_calls_switch_resp_wapper,
                      CRI_CORE_MAX_TIMEOUT,
                      NULL );
        }

        if (err)
        {
            util_memory_free((void**) hlos_cb_data_wrapper_ptr);
        }
    } while (FALSE);

    return err;
}

void cri_voice_multi_calls_switch_resp_wapper(
    cri_core_context_type context,
    cri_core_error_type cri_core_error,
    void *hlos_cb_data,
    void *cri_resp_data
)
{
    QCRIL_LOG_FUNC_ENTRY();
    cri_voice_multi_calls_switch_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr =
        (cri_voice_multi_calls_switch_hlos_cb_data_wrapper_type *)hlos_cb_data;
    if (hlos_cb_data_wrapper_ptr)
    {
        if (hlos_cb_data_wrapper_ptr->switch_cb)
        {
            hlos_cb_data_wrapper_ptr->switch_cb(context, cri_core_error, hlos_cb_data_wrapper_ptr->user_data);
        }
        util_memory_free((void**)&hlos_cb_data_wrapper_ptr);
    }
    QCRIL_LOG_FUNC_RETURN();
}

cri_core_error_type cri_voice_multi_calls_conference_req_handler(
    cri_core_context_type cri_core_context,
    const void *user_data,
    cri_voice_request_switch_calls_cb_type conf_cb
)
{
    cri_core_error_type err = QMI_ERR_NONE_V01;

    cri_voice_call_summary_type call_summary;
    cri_voice_call_summary_get_modom_call_summary(cri_voice_core_get_call_list(), &call_summary);

    do
    {
        if (!call_summary.nof_calls_overall)
        {
            err = QMI_ERR_INTERNAL_V01;
            break;
        }

        cri_voice_multi_calls_conference_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr =
            util_memory_alloc(sizeof(*hlos_cb_data_wrapper_ptr));
        if (!hlos_cb_data_wrapper_ptr)
        {
            err = QMI_ERR_INTERNAL_V01;
            break;
        }

        hlos_cb_data_wrapper_ptr->user_data = (void*)user_data;
        hlos_cb_data_wrapper_ptr->conf_cb = conf_cb;


        if (call_summary.nof_ps_calls)
        {
            voice_manage_ip_calls_req_msg_v02 manage_voip_calls_req;
            memset(&manage_voip_calls_req, 0, sizeof(manage_voip_calls_req));
            manage_voip_calls_req.sups_type = VOIP_SUPS_TYPE_MAKE_CONFERENCE_CALL_V02;

            err = cri_core_qmi_send_msg_async(
                      cri_core_context,
                      cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                      QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                      &manage_voip_calls_req,
                      sizeof(manage_voip_calls_req),
                      sizeof(voice_manage_ip_calls_resp_msg_v02),
                      hlos_cb_data_wrapper_ptr,
                      cri_voice_multi_calls_conference_resp_wapper,
                      CRI_CORE_MAX_TIMEOUT,
                      NULL );
        }
        else
        {
            voice_manage_calls_req_msg_v02 manage_calls_req;
            memset(&manage_calls_req, 0, sizeof(manage_calls_req));
            manage_calls_req.sups_type = SUPS_TYPE_MAKE_CONFERENCE_CALL_V02;
            err = cri_core_qmi_send_msg_async(
                      cri_core_context,
                      cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                      QMI_VOICE_MANAGE_CALLS_REQ_V02,
                      &manage_calls_req,
                      sizeof(manage_calls_req),
                      sizeof(voice_manage_calls_resp_msg_v02),
                      hlos_cb_data_wrapper_ptr,
                      cri_voice_multi_calls_conference_resp_wapper,
                      CRI_CORE_MAX_TIMEOUT,
                      NULL );
        }

        if (err)
        {
            util_memory_free((void**) hlos_cb_data_wrapper_ptr);
        }
    } while (FALSE);

    return err;
}

void cri_voice_multi_calls_conference_resp_wapper(
    cri_core_context_type context,
    cri_core_error_type cri_core_error,
    void *hlos_cb_data,
    void *cri_resp_data
)
{
    QCRIL_LOG_FUNC_ENTRY();
    cri_voice_multi_calls_conference_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr =
        (cri_voice_multi_calls_conference_hlos_cb_data_wrapper_type *)hlos_cb_data;
    if (hlos_cb_data_wrapper_ptr)
    {
        if (hlos_cb_data_wrapper_ptr->conf_cb)
        {
            hlos_cb_data_wrapper_ptr->conf_cb(context, cri_core_error, hlos_cb_data_wrapper_ptr->user_data);
        }
        util_memory_free((void**)&hlos_cb_data_wrapper_ptr);
    }
    QCRIL_LOG_FUNC_RETURN();
}

cri_core_error_type cri_voice_multi_calls_separate_conn_req_handler(
    cri_core_context_type cri_core_context,
    const cri_voice_separate_conn_request_type *req_message_ptr,
    const void *user_data,
    cri_voice_request_separate_conn_cb_type separate_conn_cb
)
{
    cri_core_error_type err = QMI_ERR_NONE_V01;

    do
    {
        cri_voice_call_summary_type call_summary;
        cri_voice_call_summary_get_modom_call_summary(cri_voice_core_get_call_list(), &call_summary);
        if ( !call_summary.nof_calls_overall ||
             call_summary.nof_ps_calls > 0 ||
             call_summary.nof_cs_3gpp2_calls > 0 )
        {
            err = QMI_ERR_INTERNAL_V01;
            break;
        }

        if (1 == call_summary.nof_calls_overall)
        {
            err = QMI_ERR_INTERNAL_V01;
            break;
        }
        cri_voice_multi_calls_separate_conn_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr =
            util_memory_alloc(sizeof(*hlos_cb_data_wrapper_ptr));
        if (!hlos_cb_data_wrapper_ptr)
        {
            err = QMI_ERR_INTERNAL_V01;
            break;
        }

        hlos_cb_data_wrapper_ptr->user_data = (void*)user_data;
        hlos_cb_data_wrapper_ptr->seperate_conn_cb = separate_conn_cb;

        voice_manage_calls_req_msg_v02 manage_calls_req;
        memset(&manage_calls_req, 0, sizeof(manage_calls_req));
        manage_calls_req.sups_type = SUPS_TYPE_HOLD_ALL_EXCEPT_SPECIFIED_CALL_V02;
        manage_calls_req.call_id_valid = TRUE;
        manage_calls_req.call_id = 1;
        err = cri_core_qmi_send_msg_async(
                      cri_core_context,
                      cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                      QMI_VOICE_MANAGE_CALLS_REQ_V02,
                      &manage_calls_req,
                      sizeof(manage_calls_req),
                      sizeof(voice_manage_calls_resp_msg_v02),
                      hlos_cb_data_wrapper_ptr,
                      cri_voice_multi_calls_switch_resp_wapper,
                      CRI_CORE_MAX_TIMEOUT,
                      NULL );
        if (err)
        {
            util_memory_free((void**) hlos_cb_data_wrapper_ptr);
        }
    } while (FALSE);

    return err;
}
