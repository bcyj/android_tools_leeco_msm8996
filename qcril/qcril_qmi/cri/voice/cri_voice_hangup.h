/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_HANGUP
#define CRI_VOICE_HANGUP

#include "cri_voice.h"

cri_core_error_type cri_voice_hangup_req_handler(cri_core_context_type cri_core_context, const cri_voice_call_hangup_request_type *req_message, const void *user_data, cri_voice_request_hangup_cb_type hangup_cb);

#endif
