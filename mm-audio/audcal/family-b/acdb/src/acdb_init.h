#ifndef __ACDB_INIT_H__
#define __ACDB_INIT_H__
/*===========================================================================
    @file   acdb_init.h

    The interface of the Acdb Initialization software.

    This file is the interface of the module responsible for initializing
    the ACDB with calibration data.

                    Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_init.h#4 $ */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_os_includes.h"

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

int32_t acdb_init (const char* pFilename,uint8_t **pBfr,uint32_t *pFileSize,uint32_t *pVerMaj,uint32_t *pVerMin, uint32_t *pRevVer);
int32_t acdb_delta_init (const char* pFilename,uint32_t fileNameLen, uint32_t *pDeltaFileExists, uint8_t **pBfr,uint32_t *pFileSize,uint32_t *pVerMaj,uint32_t *pVerMin, uint32_t *pRevVer);

#endif /* __ACDB_INIT_H__ */
