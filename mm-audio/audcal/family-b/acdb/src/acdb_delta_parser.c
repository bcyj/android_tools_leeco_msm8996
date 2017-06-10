/*===========================================================================
    FILE:           acdb_delta_parser.c

    OVERVIEW:       This file is the parser for the Acdb file. This is an
                    optimization from the previous parser to not use so
                    much ARM9 heap at any given time. This implementation
                    will use the stack from the context of the caller.

    DEPENDENCIES:   None

                    Copyright (c) 2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_delta_parser.c#3 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2014-02-14  avi     Support delta file parser.

========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_delta_parser.h"
#include "acdb_parser.h"

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

int32_t IsAcdbDeltaFileValid(const uint8_t *pFileBuf,const uint32_t nFileSize)
{
   uint32_t offset = 0;
   uint32_t data_len = 0;
   uint32_t fileMajor;
   uint32_t fileMinor;
   if(pFileBuf == NULL || nFileSize == 0)
   {
      return ACDB_PARSE_INVALID_FILE;
   }

   if(nFileSize < sizeof(AcdbDeltaFileHeaderInfo) + sizeof(uint32_t))
      return ACDB_PARSE_INVALID_FILE;

   ACDB_MEM_CPY(&fileMajor,sizeof(uint32_t),pFileBuf + offset,sizeof(uint32_t));
   offset += sizeof(uint32_t);

   ACDB_MEM_CPY(&fileMinor,sizeof(uint32_t),pFileBuf + offset,sizeof(uint32_t));
   offset += sizeof(uint32_t);

   if(!(fileMajor == 0 && fileMinor == 0)) // could only parse 0 , 0 file version, add additional versions here in future.
   {
      return ACDB_PARSE_INVALID_FILE;
   }

   offset += 3 * sizeof(uint32_t);
   ACDB_MEM_CPY(&data_len,sizeof(uint32_t),pFileBuf + offset,sizeof(uint32_t));
   offset += sizeof(uint32_t);

   if(data_len + offset != nFileSize)
   {
      return ACDB_PARSE_INVALID_FILE;
   }

	return ACDB_PARSE_SUCCESS;
}

int32_t AcdbDeltaFileGetSWVersion(const uint8_t *pFileBuf,const uint32_t nFileSize,uint32_t *pMajVer,uint32_t *pMinVer,uint32_t *pRevVer)
{
   uint32_t fileMajor;
   uint32_t fileMinor;
   uint32_t offset = 0;

   if(nFileSize < (sizeof(AcdbDeltaFileHeaderInfo) + sizeof(uint32_t)))
      return ACDB_PARSE_FAILURE;

   ACDB_MEM_CPY(&fileMajor,sizeof(uint32_t),pFileBuf + offset,sizeof(uint32_t));
   offset += sizeof(uint32_t);

   ACDB_MEM_CPY(&fileMinor,sizeof(uint32_t),pFileBuf + offset,sizeof(uint32_t));
   offset += sizeof(uint32_t);

   if(fileMajor == 0 && fileMinor == 0)
   {
      ACDB_MEM_CPY(pMajVer,sizeof(uint32_t),pFileBuf + offset,sizeof(uint32_t));
      offset += sizeof(uint32_t);

      ACDB_MEM_CPY(pMinVer,sizeof(uint32_t),pFileBuf + offset,sizeof(uint32_t));
      offset += sizeof(uint32_t);

      ACDB_MEM_CPY(pRevVer,sizeof(uint32_t),pFileBuf + offset,sizeof(uint32_t));
      offset += sizeof(uint32_t);
   }
   else // don't know how to parse other versions, so return error.
   {
      return ACDB_PARSE_FAILURE;
   }

   return ACDB_PARSE_SUCCESS;
}
