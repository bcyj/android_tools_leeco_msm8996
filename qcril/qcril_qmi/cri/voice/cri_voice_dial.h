/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_DIAL
#define CRI_VOICE_DIAL

#include "cri_voice.h"

cri_core_error_type cri_voice_dial_req_handler(cri_core_context_type cri_core_context, const cri_voice_dial_request_type *req_message, const void *user_data, cri_voice_request_dial_cb_type dial_cb);

void cri_voice_dial_resp_handler(int qmi_service_client_id,
                                            voice_dial_call_resp_msg_v02 *qmi_dial_call_resp_ptr,
                                            cri_core_context_type cri_core_context);

#endif
