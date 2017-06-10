/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_CORE
#define CRI_VOICE_CORE

#include "cri_voice.h"
#include "cri_voice_call_info.h"
#include "cri_voice_settings.h"
#include "cri_voice_cache.h"
#include "cri_voice_temp_defs.h"

cri_core_error_type cri_voice_core_init(hlos_ind_cb_type hlos_ind_cb_func_ptr);

void cri_voice_core_unsol_ind_handler(int qmi_service_client_id,
                               unsigned long message_id,
                               void *ind_data,
                               int ind_data_len);

void cri_voice_core_async_resp_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *resp_data,
                                int resp_data_len,
                                cri_core_context_type cri_core_context);

cri_voice_cache_type* cri_voice_core_get_call_info();
util_list_info_type* cri_voice_core_get_call_list();
cri_voice_settings_type* cri_voice_core_get_settings();
cri_voice_qmi_client_info_type* cri_voice_core_get_qmi_client_info();

#endif
