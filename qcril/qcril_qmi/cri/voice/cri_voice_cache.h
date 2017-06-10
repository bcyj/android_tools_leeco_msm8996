/***************************************************************************************************
    @file
    cri_voice_cache.h

    @brief
    Provides CRI Voice cache related functionalities

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CRI_VOICE_CACHE_H
#define CRI_VOICE_CACHE_H

#include "cri_voice.h"
#include "cri_voice_settings.h"
#include "cri_voice_call_list.h"
#include "utils_common.h"
#include "voice_service_v02.h"

typedef struct
{
    uint32 total_size;
    uint32 filled_size;
    uint32 last_sequence_number;
    uint8* buffer;
} cri_voice_cache_conf_info_type;

typedef struct
{
    boolean qmi_ril_voice_is_voice_calls_supressed;
    cri_voice_cache_conf_info_type conf_info;
    uint8 clir;
    util_list_info_type* call_list_ptr;
} cri_voice_cache_type;

cri_core_error_type cri_voice_cache_init(cri_voice_cache_type* call_info_ptr);
cri_core_error_type cri_voice_cache_reset(cri_voice_cache_type* call_info_ptr);

util_list_info_type* cri_voice_cache_get_call_list(const cri_voice_cache_type* call_info_ptr);

boolean  cri_voice_cache_get_all_call_supressed(const cri_voice_cache_type* call_info_ptr);
void cri_voice_cache_set_all_call_supressed(cri_voice_cache_type* call_info_ptr, boolean suppressed);

int  cri_voice_cache_get_default_clir(const cri_voice_cache_type* call_info_ptr);
void cri_voice_cache_set_default_clir(cri_voice_cache_type* call_info_ptr, int clir);

void cri_voice_cache_reset_conf_info(cri_voice_cache_type* call_info_ptr);
cri_voice_cache_conf_info_type* cri_voice_cache_get_conf_info(cri_voice_cache_type* call_info_ptr);

#endif
