/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_UTILS
#define CRI_VOICE_UTILS

#include "cri_voice_core.h"
#include "cri_core.h"
#include "voice_service_v02.h"
#include "cri_voice_call_info.h"

cri_core_error_type cri_voice_util_get_qmi_call_type_info
(
    cri_voice_call_type_type           cri_call_type,
    cri_voice_call_domain_type         cri_call_domain,
    int                                is_emergency,
    call_type_enum_v02                 *qmi_call_type,
    uint8_t                            *qmi_audio_attrib_valid,
    voice_call_attribute_type_mask_v02 *qmi_audio_attrib,
    uint8_t                            *qmi_video_attrib_valid,
    voice_call_attribute_type_mask_v02 *qmi_video_attrib
);

void cri_voice_util_free_call_list(cri_voice_call_list_type** call_list_dptr);

uint32_t cri_voice_utils_call_num_copy_with_toa_check(
    char* dest,
    uint32_t dest_buffer_size,
    const char* src,
    uint32_t src_size,
    voice_num_type_enum_v02 num_type);

#endif
