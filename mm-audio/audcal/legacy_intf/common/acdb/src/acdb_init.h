#ifndef __ACDB_INIT_H__
#define __ACDB_INIT_H__
/*===========================================================================
    @file   acdb_init.h

    The interface of the Acdb Initialization software.

    This file is the interface of the module responsible for initializing
    the ACDB with calibration data.

                    Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_includes.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

#define ACDB_INIT_SUCCESS 0
#define ACDB_INIT_FAILURE -1

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

int32_t acdb_init ();

#endif /* __ACDB_INIT_H__ */
