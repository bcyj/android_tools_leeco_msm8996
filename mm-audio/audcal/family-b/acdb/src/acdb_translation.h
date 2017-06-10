#ifndef __ACDB_TRANSLATION_H__
#define __ACDB_TRANSLATION_H__

/*===========================================================================
    FILE:           acdb_translation.h

    OVERVIEW:       This file contains the implementaion of the translation between
                    data structure versioning changes.
    DEPENDENCIES:   None

                    Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_translation.h#2 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2010-09-20  ernanl  Initial implementation of the APIs and
                        associated helper methods.
========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_os_includes.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Global Data Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Variable Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

//Translate sample rate bit flag mask to actual sample rate value
int32_t acdb_translate_sample_rate (const uint32_t nInput,
                                    uint32_t* pOutput
                                    );

int32_t acdb_devinfo_getSampleMaskOffset (uint32_t nDeviceType);

int32_t acdb_devinfo_getBytesPerSampleMaskOffset (uint32_t nDeviceType);

#endif /* __ACDB_TRANSLATION_H__ */
