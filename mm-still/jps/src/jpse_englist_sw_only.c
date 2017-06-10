/*========================================================================


*//** @file jpse_englist_sw_only.c

It contains the encode engine try lists if the platform only has software
encode engine.

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2010-11 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/15/2010 staceyw Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpege_engine_sw.h"

static jpege_engine_profile_t* jps_empty_list[1]   = {0};
static jpege_engine_profile_t* jps_sw_only_list[3] = {&jpege_engine_sw_profile, 0};

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
jpege_engine_lists_t jpse_engine_try_lists =
{
    jps_sw_only_list, // JPS_ENCODER_PREF_HW_ACCELERATED_PREFERRED
    jps_empty_list,   // JPS_ENCODER_PREF_HW_ACCELERATED_ONLY
    jps_sw_only_list, // JPS_ENCODER_PREF_SOFTWARE_PREFFERED
    jps_sw_only_list, // JPS_ENCODER_PREF_SOFTWARE_ONLY
    jps_sw_only_list, // JPS_ENCODER_PREF_DONT_CARE
};

