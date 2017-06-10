/*========================================================================


*//** @file jpegd_englist_q5_sw.c

It contains the decode engine try lists if the platform that has both qdsp5
and software decode engine.

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008, 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/19/09   vma     Added back the DON'T CARE preference that was missed.
10/22/09   vma     Created file.
========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpegd_engine_sw.h"
#include "jpegd_engine_q5.h"

static jpegd_engine_profile_t* q5_only_list[2] = {&jpegd_engine_q5_profile, 0};
static jpegd_engine_profile_t* sw_only_list[2] = {&jpegd_engine_sw_profile, 0};
static jpegd_engine_profile_t* q5_sw_list[3]   = {&jpegd_engine_q5_profile, &jpegd_engine_sw_profile, 0};
static jpegd_engine_profile_t* sw_q5_list[3]   = {&jpegd_engine_sw_profile, &jpegd_engine_q5_profile, 0};

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
jpegd_engine_lists_t jpegd_engine_try_lists =
{
    q5_sw_list,   // JPEG_DECODER_PREF_HW_ACCELERATED_PREFERRED
    q5_only_list, // JPEG_DECODER_PREF_HW_ACCELERATED_ONLY
    sw_q5_list,   // JPEG_DECODER_PREF_SOFTWARE_PREFFERED
    sw_only_list, // JPEG_DECODER_PREF_SOFTWARE_ONLY
    q5_sw_list,   // JPEG_DECODER_PREF_DONT_CARE
};

