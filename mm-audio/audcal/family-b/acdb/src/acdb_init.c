/*===========================================================================
    FILE:           acdb_init.c

    OVERVIEW:       This file determines the logic to initialize the ACDB.

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

    $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_init.c#4 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2014-02-14  avi     Support ACDB persistence.
    2013-06-07  mh      Corrected checkpatch errors
    2010-09-21  ernanl  Enable ADIE calibration support.
    2010-06-26  vmn     Initial revision.

========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_init.h"
#include "acdb_init_utility.h"
#include "acdb_parser.h"
#include "acdb_delta_parser.h"

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

/* ---------------------------------------------------------------------------
 * Externalized Function Definitions
 *--------------------------------------------------------------------------- */
int32_t acdb_init (const char* pFilename,uint8_t **pBfr,uint32_t *pFileSize,uint32_t *pVerMaj,uint32_t *pVerMin, uint32_t *pRevVer)
{
	// acdb_init module should be able to handle both the zipped and unzipped format of acdb file
   int32_t result = ACDB_INIT_SUCCESS;

   result = AcdbGetFileData (pFilename,pBfr,pFileSize);
   if(result != ACDB_PARSE_SUCCESS || *pBfr == NULL)
	   return result;
   if(ACDB_PARSE_SUCCESS != IsAcdbFileValid(*pBfr,*pFileSize))
	   result = ACDB_INIT_FAILURE;
   else
   {
	   //Check if its a zipped acdb file, if so process it
	   if(ACDB_PARSE_COMPRESSED == IsAcdbFileZipped(*pBfr,*pFileSize))
	   {
		   // This is a zipped file, we have to unzip this using zlib
		   // and check the acdb file validity once again.
	   }
   }
    if(AcdbFileGetSWVersion(*pBfr,*pFileSize,pVerMaj,pVerMin,pRevVer)!=ACDB_PARSE_SUCCESS)
	   return ACDB_INIT_FAILURE;

   return result;
}

int32_t acdb_delta_init (const char* pFilename,uint32_t fileNameLen, uint32_t *pDeltaFileExists, uint8_t **pBfr,uint32_t *pFileSize,uint32_t *pVerMaj,uint32_t *pVerMin, uint32_t *pRevVer)
{
	// acdb_init module should be able to handle both the zipped and unzipped format of acdb file
   int32_t result = ACDB_INIT_SUCCESS;
   uint32_t deltaFileSize = 0;
   int32_t deltaFileExists = ACDB_UTILITY_INIT_FAILURE;

   *pBfr = NULL;
   deltaFileExists = AcdbIsDeltaFileAvailable(pFilename, fileNameLen, &deltaFileSize);
   *pFileSize = deltaFileSize;

   if(ACDB_UTILITY_INIT_SUCCESS == deltaFileExists)
   {
      *pBfr = (uint8_t *)ACDB_MALLOC(deltaFileSize);
      if(*pBfr == NULL)
      {
         ACDB_DEBUG_LOG("ACDB_COMMAND: Unable to allocate memory in acdb_delta_init()\n");
         return ACDB_INIT_FAILURE;
      }

      result = AcdbGetDeltaFileData (pFilename,fileNameLen,*pBfr,*pFileSize);
      if(result != ACDB_PARSE_SUCCESS || *pBfr == NULL)
         return result;

      if(ACDB_PARSE_SUCCESS != IsAcdbDeltaFileValid(*pBfr,*pFileSize))
         result = ACDB_INIT_FAILURE;

      if(AcdbDeltaFileGetSWVersion(*pBfr,*pFileSize,pVerMaj,pVerMin,pRevVer)!=ACDB_PARSE_SUCCESS)
         return ACDB_INIT_FAILURE;

   *pDeltaFileExists = TRUE;
   }
   else
   {
      *pBfr = NULL;
      *pDeltaFileExists = FALSE;
   }

   return result;
}
