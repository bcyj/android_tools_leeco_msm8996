/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_call_summary.h"

void cri_voice_call_summary_get_modom_call_summary(
    const util_list_info_type* call_list_ptr,
    cri_voice_call_summary_type* call_summary_ptr
)
{
    if (!call_list_ptr || !call_summary_ptr)
    {
        return;
    }

    memset(call_summary_ptr, 0, sizeof(*call_summary_ptr));

    uint32 num_of_calls;
    cri_voice_call_obj_type** call_obj_dptr;

    cri_voice_call_list_get_filtered_call_objects(
       call_list_ptr, cri_voice_call_obj_is_modem_call,
       &num_of_calls,
       &call_obj_dptr);

    if (num_of_calls && call_obj_dptr)
    {
        uint32 i;
        for (i=0; i<num_of_calls; i++)
        {
            call_summary_ptr->nof_calls_overall++;
            call_summary_ptr->nof_cs_calls += cri_voice_call_obj_is_cs(call_obj_dptr[i]);
            call_summary_ptr->nof_ps_calls += cri_voice_call_obj_is_ps(call_obj_dptr[i]);
            call_summary_ptr->nof_fg_calls += cri_voice_call_obj_is_fg(call_obj_dptr[i]);
            call_summary_ptr->nof_bg_calls += cri_voice_call_obj_is_bg(call_obj_dptr[i]);

            call_summary_ptr->nof_cs_3gpp_calls +=
                cri_voice_call_obj_is_3gpp(call_obj_dptr[i]) &&
                cri_voice_call_obj_is_cs(call_obj_dptr[i]);
            call_summary_ptr->nof_cs_3gpp2_calls +=
                cri_voice_call_obj_is_3gpp2(call_obj_dptr[i]) &&
                cri_voice_call_obj_is_cs(call_obj_dptr[i]);

            call_summary_ptr->has_emergency_call =
                cri_voice_call_obj_is_emergency_or_emergency_ip(call_obj_dptr[i]);
        }
    }

    if (call_obj_dptr)
    {
        util_memory_free((void**) &call_obj_dptr);
    }
}
