/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_GET_CALLS
#define CRI_VOICE_GET_CALLS

#include "cri_voice_call_list.h"

cri_core_error_type cri_voice_get_calls_request_get_current_all_calls(cri_voice_call_list_type** call_list_dptr);
cri_core_error_type cri_voice_get_calls_request_get_current_specific_calls(cri_voice_call_list_type** call_list_dptr, cri_voice_is_specific_call is_specific_call_checker);
cri_core_error_type cri_voice_get_calls_request_get_current_specific_calls_with_param(cri_voice_call_list_type** call_list_dptr, cri_voice_is_specific_call_with_param is_specific_call_checker, const void* param);

#endif
