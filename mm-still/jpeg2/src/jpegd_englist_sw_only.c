/*========================================================================


*//** @file jpegd_englist_sw_only.c

It contains the decode engine try lists if the platform only has software
decode engine.

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
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


/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
extern jpegd_engine_lists_t jpegd_engine_try_lists;

