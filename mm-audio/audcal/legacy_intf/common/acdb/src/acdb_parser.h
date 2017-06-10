#ifndef __ACDB_PARSER_H__
#define __ACDB_PARSER_H__
/*===========================================================================
    @file   acdb_parser.h

    The interface of the Acdb Parse project.

    This file will handle the parsing of an ACDB file. It will issue callbacks
    when encountering useful client information in the ACDB file.

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

#define ACDB_PARSE_DEVICE_INFO_MAX_SIZE 1024
#define ACDB_PARSE_INDEX_MAX_SIZE 10
#define ACDB_PARSE_CAL_DATA_MAX_SIZE 1024

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

typedef enum {
   ACDB_FAIL = -1,
   ACDB_GET_FILE_SIZE,
   ACDB_GET_DATA,
   ACDB_FILE_VERSION,
   ACDB_DATE_MODIFIED,
   ACDB_OEM_INFORMATION,
   ACDB_TARGET_VERSION,
   ACDB_TABLE_DEFINITION,
   ACDB_DEVICE_INFORMATION,
   ACDB_VOC_PROC_CMN_DATA,
   ACDB_VOC_PROC_STRM_DATA,
   ACDB_VOC_PROC_VOL_DATA,
   ACDB_AUD_PROC_CMN_DATA,
   ACDB_AUD_PROC_STRM_DATA,
   ACDB_AUD_PROC_VOL_DATA,
   ACDB_ADIE_PROFILE_DATA,
   ACDB_ADIE_ANC_CONFIG_DATA,
   ACDB_AFE_DATA,
   ACDB_GLBTBL_DATA,
   ACDB_AFE_CMN_DATA,
} AcdbParseCmdType;

typedef uint32_t (*AcdbParseCBFuncPtrType)
(
   AcdbParseCmdType  cmd,
   void              *handle,
   void              *status_data
);

typedef struct _AcdbParseGetFileSizeType
{
   uint32_t   size;
} AcdbParseGetFileSizeType;

typedef struct _AcdbParseGetDataType
{
   uint8_t    *pBuffer;
   uint32_t   position;
   uint32_t   bytesToRead;
   uint32_t   bytesRead;
} AcdbParseGetDataType;

typedef struct _AcdbParseFileVersionType
{
   uint16_t   major;
   uint16_t   minor;
   uint16_t   build;
   uint16_t   revision;
} AcdbParseFileVersionType;

typedef struct _AcdbParseDateModifiedType
{
   char     *pDateString;
   uint32_t   dateStringLength;
} AcdbParseDateModifiedType;

typedef struct _AcdbParseOemInformationType
{
   char     *pOemInformation;
   uint32_t   oemInformationLength;
} AcdbParseOemInformationType;

typedef struct _AcdbParseTargetVersionType
{
   uint32_t   targetversion;
} AcdbParseTargetVersionType;

typedef struct _AcdbParseTableDefinitionType
{
   uint32_t   nTableId;
   uint32_t   nTableDataId;
   uint32_t   nTableIndexCount;
   uint32_t   nTableIndexType [ACDB_PARSE_INDEX_MAX_SIZE];
} AcdbParseTableDefinitionType;

typedef struct _AcdbParseDeviceInformationType
{
   uint32_t    nDeviceId;
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseDeviceInformationType;

typedef struct _AcdbParseVocProcCmnDataType
{
   uint32_t    nTxDeviceId;
   uint32_t    nRxDeviceId;
   uint32_t    nTxDeviceSampleRateId;
   uint32_t    nRxDeviceSampleRateId;
   uint32_t    nNetworkId;
   uint32_t    nVocProcSampleRateId;
   uint32_t    nModuleId;
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseVocProcCmnDataType;

typedef struct _AcdbParseVocProcStreamDataType
{
   uint32_t    nNetworkId;
   uint32_t    nVocProcSampleRateId;
   uint32_t    nModuleId;
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseVocProcStreamDataType;

typedef struct _AcdbParseVocProcVolDataType
{
   uint32_t    nTxDeviceId;
   uint32_t    nRxDeviceId;
   uint32_t    nNetworkId;
   uint32_t    nVocProcSampleRateId;
   uint32_t    nModuleId;
   uint32_t    nParamId;
   uint32_t    nVolumeIndex;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseVocProcVolDataType;

typedef struct _AcdbParseAudProcCmnDataType
{
   uint32_t    nDeviceId;
   uint32_t    nDeviceSampleRateId;
   uint32_t    nApplicationTypeId;
   uint32_t    nModuleId;
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseAudProcCmnDataType;

typedef struct _AcdbParseAudProcStraemDataType
{
   uint32_t    nDeviceId;
   uint32_t    nApplicationTypeId;
   uint32_t    nModuleId;
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseAudProcStreamDataType;

typedef struct _AcdbParseAudProcVolDataType
{
   uint32_t    nDeviceId;
   uint32_t    nApplicationTypeId;
   uint32_t    nModuleId;
   uint32_t    nParamId;
   uint32_t    nVolumeIndex;  
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseAudProcVolDataType;

typedef struct _AcdbParseAdieProfileDataType
{
   uint32_t    nCodecPathId;
   uint32_t    nFrequencyId;
   uint32_t    nOverSampleRateId;
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseAdieProfileDataType;

typedef struct _AcdbParseAdieANCDataType
{
   uint32_t    nDeviceId;
   uint32_t    nFrequencyId;
   uint32_t    nOverSampleRateId;   
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseAdieANCDataType;

typedef struct _AcdbParseAfeDataType
{
   uint32_t    nTxDeviceId;
   uint32_t    nRxDeviceId;
   uint32_t    nModuleId;
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseAfeDataType;

typedef struct _AcdbParseGlbTblDataType
{
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferSize;
   uint8_t *pBuffer;
}
AcdbParseGlbTblDataType;

typedef struct _AcdbParseAfeCmnDataType
{
   uint32_t    nDeviceId;
   uint32_t    nAfeSampleRateId;
   uint32_t    nModuleId;
   uint32_t    nParamId;
   uint32_t    nBufferSize;
   uint8_t    *pBuffer;
} AcdbParseAfeCmnDataType;


/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

int32_t AcdbParse (void *pHandle, AcdbParseCBFuncPtrType pfnCallback);
int32_t AcdbParseTargetVersion (void *pHandle, AcdbParseCBFuncPtrType pfnCallback);

#endif /* __ACDB_PARSER_H__ */
