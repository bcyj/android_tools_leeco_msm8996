/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_answer.h"
#include "cri_voice_core.h"
#include "cri_voice_temp_defs.h"
#include "cri_rule_handler.h"

static cri_core_error_type cri_voice_answer_request_answer_incoming(cri_core_context_type cri_core_context, const void* user_data, boolean clir_valid,
                                                           cri_voice_clir_type clir, boolean call_type_valid, cri_voice_call_type_type call_type);
static int cri_voice_answer_incoming_call_rule_checker(void *rule_data);
static void cri_voice_answer_resp_wapper(cri_core_context_type context,
                                  cri_core_error_type cri_core_error,
                                  void *hlos_cb_data,
                                  void *cri_resp_data);

typedef struct
{
    void* user_data;
    cri_voice_request_answer_cb_type answer_cb;
} cri_voice_answer_hlos_cb_data_wrapper_type;

cri_core_error_type cri_voice_answer_req_handler(cri_core_context_type cri_core_context, const cri_voice_answer_request_type *req_message_ptr, const void *user_data, cri_voice_request_answer_cb_type answer_cb)
{
    boolean clir_valid = req_message_ptr->clir_valid;
    cri_voice_clir_type clir = req_message_ptr->clir;
    boolean call_type_valid = req_message_ptr->call_type_valid;
    cri_voice_call_type_type call_type = req_message_ptr->call_type;

    cri_core_error_type err = QMI_ERR_NONE_V01;

    cri_voice_answer_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr = util_memory_alloc(sizeof(*hlos_cb_data_wrapper_ptr));
    if (!hlos_cb_data_wrapper_ptr)
    {
        err = QMI_ERR_INTERNAL_V01;
    }
    else
    {
        hlos_cb_data_wrapper_ptr->user_data = (void*)user_data;
        hlos_cb_data_wrapper_ptr->answer_cb = answer_cb;

        err = cri_voice_answer_request_answer_incoming(cri_core_context, hlos_cb_data_wrapper_ptr, clir_valid, clir, call_type_valid, call_type);
    }

//    QCRIL_LOG_FUNC_RETURN_WITH_RET(err);
    return err;
} // cri_voice_core_request_answer

cri_core_error_type cri_voice_answer_request_answer_incoming(cri_core_context_type cri_core_context, const void* user_data, boolean clir_valid,
                                                           cri_voice_clir_type clir, boolean call_type_valid, cri_voice_call_type_type call_type)
{
    QCRIL_LOG_FUNC_ENTRY();

    cri_core_error_type err = 0;
    do
    {
        cri_voice_call_obj_type *call_obj_ptr = cri_voice_call_list_find_by_qmi_call_state(cri_voice_core_get_call_list(), CALL_STATE_INCOMING_V02);
        if ( NULL == call_obj_ptr )
        {
            QCRIL_LOG_INFO(".. pending incoming call record entry not found");
            err = QMI_ERR_INTERNAL_V01;
            break;
        }

        voice_answer_call_req_msg_v02 ans_call_req;
        memset( &ans_call_req, 0, sizeof( ans_call_req ) );
        ans_call_req.call_id = call_obj_ptr->qmi_call_id;
        QCRIL_LOG_INFO(".. call id qmi %d", (int) ans_call_req.call_id );

        if (call_type_valid)
        {
            if( CRI_VOICE_CALL_TYPE_VT == call_type )
            {
                ans_call_req.call_type_valid = TRUE;
                ans_call_req.call_type = CALL_TYPE_VT_V02;
                ans_call_req.audio_attrib_valid = TRUE;
                ans_call_req.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                ans_call_req.video_attrib_valid = TRUE;
                ans_call_req.video_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
            }
            else if ( CRI_VOICE_CALL_TYPE_VT_TX == call_type )
            {
                ans_call_req.call_type_valid = TRUE;
                ans_call_req.call_type = CALL_TYPE_VT_V02;
                ans_call_req.audio_attrib_valid = TRUE;
                ans_call_req.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                ans_call_req.video_attrib_valid = TRUE;
                ans_call_req.video_attrib = VOICE_CALL_ATTRIB_TX_V02;
            }
            else if ( CRI_VOICE_CALL_TYPE_VT_RX == call_type )
            {
                ans_call_req.call_type_valid = TRUE;
                ans_call_req.call_type = CALL_TYPE_VT_V02;
                ans_call_req.audio_attrib_valid = TRUE;
                ans_call_req.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                ans_call_req.video_attrib_valid = TRUE;
                ans_call_req.video_attrib = VOICE_CALL_ATTRIB_RX_V02;
            }
            else
            {
                ans_call_req.call_type_valid = TRUE;
                ans_call_req.call_type = CALL_TYPE_VOICE_IP_V02;
            }
        }
        QCRIL_LOG_DEBUG("cri call type = %d, modem call type = %d", call_type, ans_call_req.call_type);

        if (clir_valid)
        {
            ans_call_req.pi_valid = TRUE;
            if (CRI_VOICE_CLIR_INVOCATION == clir)
            {
                ans_call_req.pi = IP_PRESENTATION_NUM_RESTRICTED_V02;
            }
            else
            {
                ans_call_req.pi = IP_PRESENTATION_NUM_ALLOWED_V02;
            }
        }

        cri_rule_handler_user_rule_info_type user_rule_info;
        memset(&user_rule_info, 0, sizeof(user_rule_info));
        user_rule_info.rule_data = (void*)((intptr_t)call_obj_ptr->qmi_call_id);
        user_rule_info.rule_check_handler = cri_voice_answer_incoming_call_rule_checker;

        err =  cri_core_qmi_send_msg_async( cri_core_context,
                                        cri_voice_qmi_client_get_voice_client(cri_voice_core_get_qmi_client_info()),
                                        QMI_VOICE_ANSWER_CALL_REQ_V02,
                                        &ans_call_req,
                                        sizeof(ans_call_req),
                                        sizeof(voice_answer_call_resp_msg_v02),
                                        user_data,
                                        cri_voice_answer_resp_wapper,
                                        CRI_CORE_MAX_TIMEOUT,
                                        &user_rule_info );
    } while ( FALSE );

//    QCRIL_LOG_FUNC_RETURN_WITH_RET(err);
    return err;
}

void cri_voice_answer_resp_wapper(cri_core_context_type context,
                                  cri_core_error_type cri_core_error,
                                  void *hlos_cb_data,
                                  void *cri_resp_data)
{
    cri_voice_answer_hlos_cb_data_wrapper_type *hlos_cb_data_wrapper_ptr = (cri_voice_answer_hlos_cb_data_wrapper_type *)hlos_cb_data;
    if (hlos_cb_data_wrapper_ptr)
    {
        if (hlos_cb_data_wrapper_ptr->answer_cb)
        {
            hlos_cb_data_wrapper_ptr->answer_cb(context, cri_core_error, hlos_cb_data_wrapper_ptr->user_data);
        }
        util_memory_free((void**)&hlos_cb_data_wrapper_ptr);
    }
}

int cri_voice_answer_incoming_call_rule_checker(void *rule_data)
{
    int ret_code = FALSE;

    int8 qmi_call_id = (intptr_t) rule_data;
    cri_voice_call_obj_type* call_obj_ptr = cri_voice_call_list_find_by_qmi_call_id(cri_voice_core_get_call_list(), qmi_call_id);

    QCRIL_LOG_INFO("qmi_call_id: %d, call_obj_ptr: %p", qmi_call_id, call_obj_ptr);

    if ( call_obj_ptr &&
         ( CALL_STATE_CONVERSATION_V02 == call_obj_ptr->qmi_voice_scv_info.call_state ||
           CALL_STATE_END_V02 == call_obj_ptr->qmi_voice_scv_info.call_state
         )
       )
    {
        ret_code = TRUE;
    }

    return ret_code;
}
