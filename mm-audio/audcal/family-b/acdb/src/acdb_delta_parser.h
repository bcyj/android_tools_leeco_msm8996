#ifndef __ACDB_DELTA_PARSER_H__
#define __ACDB_DELTA_PARSER_H__
/*===========================================================================
    @file   acdb_delta_parser.h

    The interface of the Acdb Parse project.

    This file will handle the parsing of an ACDB file. It will issue callbacks
    when encountering useful client information in the ACDB file.

                    Copyright (c) 2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_delta_parser.h#3 $ */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_delta_parser.h#3 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2014-02-14  avi     Support delta acdb file parser.

========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_os_includes.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

#define ACDB_DELTA_FILE_VERSION_MAJOR                      0x00000000
#define ACDB_DELTA_FILE_VERSION_MINOR                      0x00000000

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

typedef struct _AcdbDeltaFileHeaderInfo
{
   uint32_t fileMajor;
   uint32_t fileMinor;
   uint32_t versMajor;
   uint32_t versMinor;
   uint32_t versRevision;
}AcdbDeltaFileHeaderInfo;

/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

int32_t IsAcdbDeltaFileValid(const uint8_t *pFileBuf,const uint32_t nFileSize);

int32_t AcdbDeltaFileGetSWVersion(const uint8_t *pFileBuf,const uint32_t nFileSize,uint32_t *pMajVer,uint32_t *pMinVer, uint32_t *pRevVer);

#endif /* __ACDB_DELTA_PARSER_H__ */
