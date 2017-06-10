/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_call_info.h"
//#include "qcril_log.h"
#include "util_log.h"
cri_core_error_type cri_voice_call_info_init(cri_voice_call_info_type* call_info_ptr)
{
    if (call_info_ptr)
    {
        memset(call_info_ptr, 0, sizeof(&call_info_ptr));
        call_info_ptr->call_list_ptr = cri_voice_call_list_create();
    }
    return 0;
}

cri_core_error_type cri_voice_call_info_reset(cri_voice_call_info_type* call_info_ptr)
{

    if (call_info_ptr)
    {
        util_list_cleanup(call_info_ptr->call_list_ptr, NULL);

        if (call_info_ptr->conf_info.buffer)
        {
            util_memory_free((void**)&call_info_ptr->conf_info.buffer);
        }
        memset(&call_info_ptr->conf_info, 0, sizeof(call_info_ptr->conf_info));
    }
    return 0;
}

util_list_info_type* cri_voice_call_info_get_call_list(const cri_voice_call_info_type* call_info_ptr)
{
    if (call_info_ptr)
    {
        return call_info_ptr->call_list_ptr;
    }
    else
    {
        return NULL;
    }
}

boolean cri_voice_call_info_get_all_call_supressed(const cri_voice_call_info_type* call_info_ptr)
{
    return call_info_ptr ? call_info_ptr->qmi_ril_voice_is_voice_calls_supressed : 0;
}

void cri_voice_call_info_set_all_call_supressed(cri_voice_call_info_type* call_info_ptr, boolean suppressed)
{
    if (call_info_ptr)
    {
        call_info_ptr->qmi_ril_voice_is_voice_calls_supressed = suppressed;
    }
}

int cri_voice_call_info_get_default_clir(const cri_voice_call_info_type* call_info_ptr)
{
    if (call_info_ptr)
    {
        return call_info_ptr->clir;
    }
    else
    {
        return 0;
    }
}

void cri_voice_call_info_set_default_clir(cri_voice_call_info_type* call_info_ptr, int clir)
{
    if (call_info_ptr)
    {
        call_info_ptr->clir = clir;
    }
}

void cri_voice_call_info_reset_conf_info(cri_voice_call_info_type* call_info_ptr)
{
    if (call_info_ptr)
    {
        call_info_ptr->conf_info.last_sequence_number = -1;
        call_info_ptr->conf_info.total_size = 0;
        call_info_ptr->conf_info.filled_size = 0;
        if (call_info_ptr->conf_info.buffer)
    {
            util_memory_free((void**)&call_info_ptr->conf_info.buffer);
    }
}
}

cri_voice_call_info_conf_info_type* cri_voice_call_info_get_conf_info(cri_voice_call_info_type* call_info_ptr)
{
    return call_info_ptr ? &call_info_ptr->conf_info : NULL;
}
