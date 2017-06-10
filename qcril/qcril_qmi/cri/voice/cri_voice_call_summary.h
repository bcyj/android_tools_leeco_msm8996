/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_CALL_SUMMARY
#define CRI_VOICE_CALL_SUMMARY

#include "comdef.h"
#include "cri_voice_call_list.h"

typedef struct
{
    uint nof_cs_calls;
    uint nof_ps_calls;
    uint nof_fg_calls;
    uint nof_bg_calls;
    uint nof_cs_3gpp_calls;
    uint nof_cs_3gpp2_calls;
    uint nof_calls_overall;
    boolean has_emergency_call;
} cri_voice_call_summary_type;

void cri_voice_call_summary_get_modom_call_summary(const util_list_info_type* call_list_ptr, cri_voice_call_summary_type* call_summary_ptr);

#endif
