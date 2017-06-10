#ifndef __ACDB_UTILITY_H__
#define __ACDB_UTILITY_H__
/*===========================================================================
    @file   acdb_utility.h

    The interface to the ACDBINIT utility functions.

    This file will provide API access to OS-specific init utility functions.

                    Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_utility.h#2 $ */

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
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

#define SEARCH_SUCCESS 0
#define SEARCH_ERROR -1

int32_t AcdbDataBinarySearch(void *voidLookUpArray, int32_t max,int32_t indexCount,
            void *pCmd, int32_t nNoOfIndsCount,uint32_t *index);

#endif /* __ACDB_UTILITY_H__ */
