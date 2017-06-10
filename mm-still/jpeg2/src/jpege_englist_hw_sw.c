/*========================================================================


*//** @file jpege_englist_q5_sw.c

It contains the encode engine try lists if the platform that has both qdsp5
and software encode engine.

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011,2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/12/10   staceyw Added hardware engine with software/bitstream engines into all
                   preferred lists.
07/05/10   staceyw Created file.
========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpege_engine_hybrid.h"
#include "jpege_engine_hw.h"
#include "jpege_engine_bs.h"

extern jpege_engine_profile_t jpege_engine_bs_profile;
extern jpege_engine_profile_t jpege_engine_hybrid_profile;
extern jpege_engine_profile_t jpege_engine_hw_profile;

static jpege_engine_profile_t* hw_only_list[3] = {&jpege_engine_bs_profile, &jpege_engine_hw_profile, 0};
static jpege_engine_profile_t* sw_only_list[3] = {&jpege_engine_bs_profile, &jpege_engine_hybrid_profile, 0};
static jpege_engine_profile_t* hw_sw_list[4] = {&jpege_engine_bs_profile, &jpege_engine_hw_profile, &jpege_engine_hybrid_profile, 0};
static jpege_engine_profile_t* sw_hw_list[4] = {&jpege_engine_bs_profile, &jpege_engine_hybrid_profile, &jpege_engine_hw_profile, 0};

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
jpege_engine_lists_t jpege_engine_try_lists =
{
    hw_sw_list,   // JPEG_ENCODER_PREF_HW_ACCELERATED_PREFERRED
    hw_only_list, // JPEG_ENCODER_PREF_HW_ACCELERATED_ONLY
    sw_hw_list,   // JPEG_ENCODER_PREF_SOFTWARE_PREFFERED
    sw_only_list, // JPEG_ENCODER_PREF_SOFTWARE_ONLY
    hw_sw_list,   // JPEG_ENCODER_PREF_DONT_CARE
};

