/* Copyright (c) 2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "jpegd_engine_sw.h"
#include "jpegd_engine_hw.h"

static jpegd_engine_profile_t* sw_only_list[2] = {&jpegd_engine_sw_profile, 0};
static jpegd_engine_profile_t* hw_only_list[2] = {&jpegd_engine_hw_profile, 0};

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
jpegd_engine_lists_t jpegd_engine_try_lists =
{
    sw_only_list,               // JPEG_DECODER_PREF_HW_ACCELERATED_PREFERRED
    hw_only_list,               // JPEG_DECODER_PREF_HW_ACCELERATED_ONLY
    sw_only_list,               // JPEG_DECODER_PREF_SOFTWARE_PREFFERED
    sw_only_list,               // JPEG_DECODER_PREF_SOFTWARE_ONLY
    sw_only_list,               // JPEG_DECODER_PREF_DONT_CARE
};

