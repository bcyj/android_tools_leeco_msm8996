/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice.h"
#include "cri_voice_core.h"
#include "cri_voice_call_info.h"
#include "cri_voice_get_calls.h"
#include "cri_voice_dial.h"
#include "cri_voice_answer.h"
#include "cri_voice_hangup.h"
#include "cri_voice_utils.h"
#include "cri_voice_multi_calls.h"

cri_core_error_type cri_voice_request_get_current_all_calls(cri_voice_call_list_type** call_list_dptr)
{
    return cri_voice_get_calls_request_get_current_all_calls(call_list_dptr);
}

cri_core_error_type cri_voice_request_get_current_specific_calls(cri_voice_call_list_type** call_list_dptr, cri_voice_is_specific_call is_specific_call_checker)
{
    return cri_voice_get_calls_request_get_current_specific_calls(call_list_dptr, is_specific_call_checker);
}

cri_core_error_type cri_voice_request_get_current_specific_calls_with_param(cri_voice_call_list_type** call_list_dptr, cri_voice_is_specific_call_with_param is_specific_call_checker, const void* param)
{
    return cri_voice_get_calls_request_get_current_specific_calls_with_param(call_list_dptr, is_specific_call_checker, param);
}

cri_core_error_type cri_voice_request_dial(cri_core_context_type cri_core_context, const cri_voice_dial_request_type *req_message, const void *user_data, cri_voice_request_dial_cb_type dial_cb)
{
    return cri_voice_dial_req_handler(cri_core_context, req_message, user_data, dial_cb);
}

cri_core_error_type cri_voice_request_answer(cri_core_context_type cri_core_context, const cri_voice_answer_request_type *req_message, const void *user_data, cri_voice_request_answer_cb_type answer_cb)
{
    return cri_voice_answer_req_handler(cri_core_context, req_message, user_data, answer_cb);
}


cri_core_error_type cri_voice_request_hangup(cri_core_context_type cri_core_context, const cri_voice_call_hangup_request_type *req_message, const void *user_data, cri_voice_request_hangup_cb_type hangup_cb)
{
    return cri_voice_hangup_req_handler(cri_core_context, req_message, user_data, hangup_cb);
}

cri_core_error_type cri_voice_request_switch_calls(
    cri_core_context_type cri_core_context,
    const cri_voice_switch_calls_request_type *req_message_ptr,
    const void *user_data,
    cri_voice_request_switch_calls_cb_type switch_cb
)
{
    return cri_voice_multi_calls_switch_req_handler(
               cri_core_context,
               req_message_ptr,
               user_data,
               switch_cb );
}

cri_core_error_type cri_voice_request_conference(
    cri_core_context_type cri_core_context,
    const void *user_data,
    cri_voice_request_switch_calls_cb_type switch_cb
)
{
    return cri_voice_multi_calls_conference_req_handler(
               cri_core_context,
               user_data,
               switch_cb );
}

void cri_voice_free_call_list(cri_voice_call_list_type** call_list_dptr)
{
    cri_voice_util_free_call_list(call_list_dptr);
}

cri_voice_call_obj_type* cri_voice_find_call_object_by_call_bit(cri_voice_call_obj_bit_field_type bit)
{
    cri_voice_call_obj_type *call_obj_ptr = cri_voice_call_list_find_by_call_bit(cri_voice_core_get_call_list(), bit);
    return call_obj_ptr;
}

cri_voice_call_obj_type* cri_voice_find_call_object_by_cri_call_state(cri_voice_call_state_type cri_call_state)
{
    cri_voice_call_obj_type *call_obj_ptr = cri_voice_call_list_find_by_cri_call_state(
                                                cri_voice_core_get_call_list(),
                                                cri_call_state );
    return call_obj_ptr;
}

cri_voice_call_obj_type* cri_voice_find_call_object_by_qmi_call_id(uint8_t qmi_call_id)
{
    cri_voice_call_obj_type *call_obj_ptr = cri_voice_call_list_find_by_qmi_call_id(
                                                cri_voice_core_get_call_list(),
                                                qmi_call_id );
    return call_obj_ptr;
}

cri_core_error_type cri_voice_set_call_obj_user_data(uint8_t cri_call_id, void* user_data, hlos_user_data_deleter_type hlos_user_data_deleter)
{
    cri_core_error_type err = QMI_ERR_NONE_V01;
    cri_voice_call_obj_type *call_obj_ptr = cri_voice_call_list_find_by_cri_call_id(cri_voice_core_get_call_list(), cri_call_id);
    if (call_obj_ptr)
    {
        cri_voice_call_obj_set_hlos_data_and_deleter(call_obj_ptr, user_data, hlos_user_data_deleter);
    }
    else
    {
        err = QMI_ERR_INTERNAL_V01;
    }
    return err;
}

cri_core_error_type cri_voice_set_default_call_obj_user_data_deleter(void (*hlos_user_data_deleter)(void**))
{
    cri_voice_settings_set_hlos_user_data_deleter(cri_voice_core_get_settings(), hlos_user_data_deleter);
    return 0;
}

void cri_voice_delete_call_obj(cri_voice_call_obj_type *call_obj_ptr)
{
    cri_voice_call_list_delete(cri_voice_core_get_call_list(), call_obj_ptr);
}

