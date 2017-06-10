/*========================================================================


*//** @file jpege_englist_sw_only.c

It contains the encode engine try lists if the platform only has software
encode engine.

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2010,2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
02/08/10   staceyw Added bitstream engine for bitstream input into software
                   only preferred list.
11/19/09   vma     Added back the DON'T CARE preference that was missed.
10/13/09   vma     Created file.
========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpege_engine_hybrid.h"

extern jpege_engine_profile_t jpege_engine_hybrid_profile;

static jpege_engine_profile_t* empty_list[1]   = {0};
static jpege_engine_profile_t* sw_only_list[3] = {&jpege_engine_hybrid_profile, 0, 0};

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
jpege_engine_lists_t jpege_engine_try_lists =
{
    sw_only_list, // JPEG_ENCODER_PREF_HW_ACCELERATED_PREFERRED
    empty_list,   // JPEG_ENCODER_PREF_HW_ACCELERATED_ONLY
    sw_only_list, // JPEG_ENCODER_PREF_SOFTWARE_PREFFERED
    sw_only_list, // JPEG_ENCODER_PREF_SOFTWARE_ONLY
    sw_only_list, // JPEG_ENCODER_PREF_DONT_CARE
};

