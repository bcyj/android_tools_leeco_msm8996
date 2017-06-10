/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_MULTI_CALLS
#define CRI_VOICE_MULTI_CALLS

#include "cri_core.h"
#include "cri_voice.h"

cri_core_error_type cri_voice_multi_calls_switch_req_handler(
    cri_core_context_type cri_core_context,
    const cri_voice_switch_calls_request_type *req_message_ptr,
    const void *user_data,
    cri_voice_request_switch_calls_cb_type switch_cb
);

cri_core_error_type cri_voice_multi_calls_conference_req_handler(
    cri_core_context_type cri_core_context,
    const void *user_data,
    cri_voice_request_switch_calls_cb_type conf_cb
);

cri_core_error_type cri_voice_multi_calls_separate_conn_req_handler(
    cri_core_context_type cri_core_context,
    const cri_voice_separate_conn_request_type *req_message_ptr,
    const void *user_data,
    cri_voice_request_separate_conn_cb_type sperate_conn_cb
);

#endif
