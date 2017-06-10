#ifndef __ACDB_INIT_UTILITY_H__
#define __ACDB_INIT_UTILITY_H__
/*===========================================================================
    @file   acdb_init_utility.h

    The interface to the ACDBINIT utility functions.

    This file will provide API access to OS-specific init utility functions.

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

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

typedef void* AcdbInitFileContext;

/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

int32_t AcdbInitGetAcdbDefaultFilename (const char** ppFilename);
int32_t AcdbInitGetAcdbDefaultErrorFilename (const char** ppErrorFilename);

int32_t AcdbInitDoesPathExist (const char* pPath);
int32_t AcdbInitRenameFile (const char* pOldFilename, const char* pNewFilename);
int32_t AcdbInitCreatePath (void);
 
int32_t AcdbInitFileOpen (const char* pFilename, AcdbInitFileContext** ppFileContext);
int32_t AcdbInitGetFileSize (AcdbInitFileContext *pFileContext, uint32_t *pSize);
int32_t AcdbInitFileRead (AcdbInitFileContext *pFileContext, uint8_t *pBfr, uint32_t nBytesToRead, uint32_t *pBytesRead);
int32_t AcdbInitFileClose (AcdbInitFileContext *pFileContext);

#endif /* __ACDB_INIT_UTILITY_H__ */
