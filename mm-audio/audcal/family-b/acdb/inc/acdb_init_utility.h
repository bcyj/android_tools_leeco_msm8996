#ifndef __ACDB_INIT_UTILITY_H__
#define __ACDB_INIT_UTILITY_H__
/*===========================================================================
    @file   acdb_init_utility.h

    The interface to the ACDBINIT utility functions.

    This file will provide API access to OS-specific init utility functions.

                    Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/inc/acdb_init_utility.h#3 $ */

/*===========================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      ----------------------------------------------------------
12/03/13   avi      Added new functions for supporting ACDB persistence.

===========================================================================*/

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

#define ACDB_UTILITY_INIT_SUCCESS 0
#define ACDB_UTILITY_INIT_FAILURE -1

int32_t AcdbGetFileData (const char* pFilename,uint8_t **pBfr,uint32_t *pFileSize);
void  AcdbFreeFileData (void *pBfr);

/* ---------------------------------------------------------------------------
 * AcdbIsPersistenceSupported() Documentation
 *--------------------------------------------------------------------------- */

/** @addtogroup AcdbIsPersistenceSupported
@{ */

/**
   Function signature to know if ACDB persistence supported.

   @return
   - ACDB_UTILITY_INIT_SUCCESS -- ACDB persistence is supported.
   - ACDB_UTILITY_INIT_FAILURE -- ACDB persistence is not supported.

   @sa
   acdb_init_utility \n
*/

int32_t AcdbIsPersistenceSupported(void);

/* ---------------------------------------------------------------------------
 * AcdbIsDeltaFileAvailable() Documentation
 *--------------------------------------------------------------------------- */

/** @addtogroup AcdbIsDeltaFileAvailable
@{ */

/**
   Function signature to know if delta acdb file is present for existing acdb file.
   If present, return with the length of corresponding delta ACDB file data length.

   @param[in] pFilename
         ACDB file name.

   @param[in] fileNameLen
         ACDB file name length.

	@param[out] pDeltaFileDataLen
         Delta ACDB file length (if present)

   @return
   - ACDB_UTILITY_INIT_SUCCESS -- Delta ACDB file is present.
   - ACDB_UTILITY_INIT_FAILURE -- Delta ACDB file is not present.

   @sa
   acdb_init_utility \n
*/

int32_t AcdbIsDeltaFileAvailable (const char* pFilename, uint32_t fileNameLen, uint32_t* pDeltaFileDataLen);

/* ---------------------------------------------------------------------------
 * AcdbGetDeltaFileData() Documentation
 *--------------------------------------------------------------------------- */

/** @addtogroup AcdbGetDeltaFileData
@{ */

/**
   Function signature to know get delta acdb file data for the acdb file.

   @param[in] pFilename
         ACDB file name.

   @param[in] fileNameLen
         ACDB file name length.

   @param[out] pFileBfr
         Pointer to Delta file content.

   @param[in] nFileBfrSize
         Delta file content length.

   @return
   - ACDB_UTILITY_INIT_SUCCESS -- Operation successful.
   - ACDB_UTILITY_INIT_FAILURE -- Operation failure.

   @sa
   acdb_init_utility \n
*/

int32_t AcdbGetDeltaFileData (const char* pFilename, uint32_t fileNameLen, uint8_t *pFileBfr, uint32_t nFileBfrSize);

/* ---------------------------------------------------------------------------
 * AcdbWriteDeltaFileData() Documentation
 *--------------------------------------------------------------------------- */

/** @addtogroup AcdbWriteDeltaFileData
@{ */

/**
   Function signature to know write delta acdb file data for the acdb file.

   @param[in] pFilename
         ACDB file name.

   @param[in] fileNameLen
         ACDB file name length.

   @param[in] pDataBfr
         Pointer to Delta file content.

   @param[in] nDataSize
         Delta file content length.

   @return
   - ACDB_UTILITY_INIT_SUCCESS -- Operation successful.
   - ACDB_UTILITY_INIT_FAILURE -- Operation failure.

   @sa
   acdb_init_utility \n
*/

int32_t AcdbWriteDeltaFileData (const char* pFilename, uint32_t fileNameLen, uint8_t *pDataBfr, uint32_t nDataSize);

/* ---------------------------------------------------------------------------
 * AcdbDeleteDeltaFileData() Documentation
 *--------------------------------------------------------------------------- */

/** @addtogroup AcdbDeleteDeltaFileData
@{ */

/**
   Function signature to delete delta acdb file for existing acdb file.

   @param[in] pFilename
         ACDB file name.

   @param[in] fileNameLen
         ACDB file name length.

   @return
   - ACDB_UTILITY_INIT_SUCCESS -- Operation successful.
   - ACDB_UTILITY_INIT_FAILURE -- Operation failure.

   @sa
   acdb_init_utility \n
*/

int32_t AcdbDeleteDeltaFileData (const char* pFilename, uint32_t fileNameLen);

#endif /* __ACDB_INIT_UTILITY_H__ */
