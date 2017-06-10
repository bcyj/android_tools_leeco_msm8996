#ifndef __ACDB_DEFAULT_DATA_H__
#define __ACDB_DEFAULT_DATA_H__
/*===========================================================================
    @file   acdb_default_data.h

                    Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal2/common/8k_PL/acdbdata/default/rel/4.2/inc/acdb_default_data.h#1 $ */

/*===========================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/16/11   ernanl  Introduced first version of MDM9x15 and MSM8960 ACDB API.

===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_includes.h"
#include "acdb.h"
#include "acdb_private.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/*------------------------------------------------------------------------------
Target specific definitions
------------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/// External structure used in AcdbDataIoctl

/**
   @struct  AcdbDataDeviceInfoType
   @brief Response structure containsing two byte buffer for Device info common and
          target specific meta data.
   @param nDeiveId: Device ID
   @param nDeviceInfoCmnPayloadSize: The number of bytes provided in the buffer for common
                                     specific device info payload.
   @param pDeviceInfoCmnBufferPtr: the pointer points to the common device info data payload
   @param nDeviceInfoTSPayloadsSize: The number of bytes provided in the buffer for target
                                     specific device info payload.
   @param pDeviceInfoTSBufferPtr: the pointer points to the target specific device info data
                                  payload.

*/
typedef struct _AcdbDataDeviceInfoType {
   uint32_t nDeviceId;
   uint32_t nDeviceInfoCmnPayloadSize;
   uint8_t* pDeviceInfoCmnBufferPtr;
   uint32_t nDeviceIfnoTSPayloadSize;
   uint8_t* pDeviceInfoTSPBufferPtr;
} AcdbDataDeviceInfoType;

/**
   @struct   AcdbDataGetTableType
   @brief Response structure containing both table array.

   @param pTable: A pointer to the calibration table.
   @param nTableCount: The number of entries in the table.
   
   This structure will contain the calibration table array. The type
   of the array is determined by the command.
*/
typedef struct _AcdbDataGetTableType {
   uint8_t* pTable;
   uint32_t nTableCount;
} AcdbDataGetTableType;

/**
   @struct   AcdbDataUniqueDataNodeType
   @brief   This structure contains a pointer and length to a unique data entry.

   @param pUniqueData: A pointer to the unique data entry.
   @param ulDataLen: The size of the unique data in bytes.

   This structure contains a pointer and length to a unique data entry.
*/
typedef struct _AcdbDataUniqueDataNodeType{
   const uint8_t *pUniqueData;
   const uint32_t ulDataLen;
} AcdbDataUniqueDataNodeType;

/// Internal structure used in AcdbDataIoctl
/**
   @struct   AcdbDataLookupKeyType
   @brief The lookup key structure

   @param nLookupKey: An intermediate lookup key
   
   This structure provides an intermediate lookup key that is used
   to simplify looking up data between several calls to the data base.
*/
typedef struct _AcdbDataLookupKeyType {
   uint32_t nLookupKey;
} AcdbDataLookupKeyType;

/**
   @struct   AcdbDataGetParamType
   @brief Response structure containing a single entry of a calibration table.

   @parma nParamId: The parameter id.
   @param pParam: A pointer to the parameter buffer.
   @param nParamSize: The size of the buffer in bytes.
   
   This structure will contain a single instance of a calibration table.
*/
typedef struct _AcdbDataGetParamType {
   uint32_t nParamId;
   uint8_t* pParam;
   uint32_t nParamSize;
} AcdbDataGetParamType;

/**
   @struct   AcdbDataGetGlbTblParamType
   @brief Response structure containing a single entry of a calibration table.

   @param pParam: A pointer to the parameter buffer.
   @param nParamSize: The size of the buffer in bytes.
   
   This structure will contain a single instance of a calibration table.
*/
typedef struct _AcdbDataGetGlbTblParamType {
   uint8_t* pParam;
   uint32_t nParamSize;
} AcdbDataGetGlbTblParamType;

/**
   @struct   AcdbDataTopologyType
   @brief Topology element description structure

   @param ulModuleId: The module id
   @param ulParamId: The parameter id
   @param ulMaxParamLen: The maximum length for the data of this node.
   
   The topology element description structure is used in parallel to the
   Calibration table to provide metadata for each table entry. This is 
   separate from the table because it's common information and would add
   24 to 32 bytes for every calibration entry in the database (this is
   can cause the database to become very large!).
*/
typedef struct _AcdbDataTopologyType {
   const uint32_t ulModuleId;
   const uint32_t ulParamId;
   const uint32_t ulMaxParamLen;
} AcdbDataTopologyType;

/**
   @struct   AcdbDataGetTblTopType
   @brief Response structure containing both table and topology arrays.

   @param ppTable: A pointer to the calibration table.
   @param nTableEntries: The number of entries in the table.
   @param pTopology: A pointer to the topology describing the individual
                     entries in the calibration table.
   @param ulTopologyEntries: The number of entries in the topology.
   
   This structure will contain the calibration table and topology. The
   number of entries in the table and topology arrays must always be
   identical!
*/
typedef struct _AcdbDataGetTblTopType {
   AcdbDataUniqueDataNodeType **ppTable;
   uint32_t nTableEntries;
   AcdbDataTopologyType *pTopology;
   uint32_t nTopologyEntries;
} AcdbDataGetTblTopType;

/**
   @struct   AcdbDataGetAudProcVolTblTopType
   @brief Response structure containing two sets of AudProc COPP and Volume 
          table and topology arrays.

   @param copp: A structure containing the information for the COPP topology
                and calibration table.
   @param popp: A structure containing the information for the POPP topology
                and calibration table.

   This structure will contain AudProc COPP and Volume calibration table and 
   topology. The number of entries in the table and topology arrays must always 
   be identical!
*/
typedef struct _AcdbDataGetAudProcVolTblTopType {
   AcdbDataGetTblTopType copp;
   AcdbDataGetTblTopType popp;
} AcdbDataGetAudProcVolTblTopType;

/**
   @struct   AcdbDataGeneralInfoType
   @brief Response structure containing a byte buffer to be translated by
          the specific command it is used by.

   @param nBufferLength: The number of bytes provided in the buffer.
   @param pBuffer: A pointer to a buffer containing a payload.
   
   Response structure containing a byte buffer to be translated by the
   specific command it is used by. Please refer to the specific command
   for more information on the format of the payload.
*/
typedef struct _AcdbDataGeneralInfoType {
   uint32_t nBufferLength;
   uint8_t* pBuffer;
} AcdbDataGeneralInfoType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_ACDB_VERSION Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_ACDB_VERSION, ...)
   @brief API to query for the AcdbData data structure version.

   This command will obtain the AcdbData data structure version.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_ACDB_VERSION.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataModuleVersionType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataModuleVersionType).

   @see  AcdbDataIoctl
   @see  AcdbDataModuleVersionType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
*/
#define ACDBDATA_GET_ACDB_VERSION            0xACDB0000

/**
   @struct   AcdbDataTargetVersionType
   @brief API to obtain the AcdbData's data structure version.

   @param major: The major version of the data structure.
   
   This is a query command structure to obtain the AcdbData's data
   structure version.
*/
typedef struct _AcdbDataModuleVersionType {
   uint16_t major;
} AcdbDataModuleVersionType;

/**
   @struct   AcdbDataSwModuleVersionType
   @brief This is a response structure for the get ACDB module version command.
   
   @param major: The major version describing the capabilities of the ACDB SW API.
   @param minor: The minor version describing the capabilities of the ACDB SW API.

   This is the response structure for the ACDB module version command
   enabling the client to understand the ACDB APIs and capabilities.
*/
typedef struct _AcdbDataSwModuleVersionType {
   uint16_t major;
   uint16_t minor;
} AcdbDataSwModuleVersionType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_TARGET_VERSION Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_TARGET_VERSION, ...)
   @brief API to query for the target version.

   This command will obtain the target version that is stored in the default
   data file.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_TARGET_VERSION.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataTargetVersionType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataTargetVersionType).

   @see  AcdbDataIoctl
   @see  AcdbDataTargetVersionType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
*/
#define ACDBDATA_GET_TARGET_VERSION            0xACDB0001

/**
   @struct   AcdbDataTargetVersionType
   @brief API to obtain the AcdbData's target version

   @param targetversion: The target version.
   
   This is a query command structure to obtain the AcdbData's target version.
*/
typedef struct _AcdbDataTargetVersionType {
   uint32_t targetversion;
} AcdbDataTargetVersionType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_ESTIMATE_MEMORY_USE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_ESTIMATE_MEMORY_USE, ...)
   @brief API to query for the estimated ROM memory usage of the data structure.

   This command will calculate the ROM memory usage of the data structure in
   two parts: the organizational part and the data part.

   @param[in] nCommandId:
         The command id is ACDBDATA_ESTIMATE_MEMORY_USE.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataMemoryUsageType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataMemoryUsageType).

   @see  AcdbDataIoctl
   @see  AcdbDataMemoryUsageType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
*/
#define ACDBDATA_ESTIMATE_MEMORY_USE         0xACDB0002

/**
   @struct   AcdbDataMemoryUsageType
   @brief This is a query command structure for the memory usage operation.

   @param org_ROM: The calculated memory usage of the organizational part
                  of the AcdbData data structure.
   @param data_ROM: The calculated memory usage of the data part of the
                  AcdbData data structure.
   
   This is a query command structure for the memory usage operation.
*/
typedef struct _AcdbDataMemoryUsageType {
   uint32_t org_ROM;
   uint32_t data_ROM;
} AcdbDataMemoryUsageType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_OEM_INFO Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_OEM_INFO, ...)
   @brief API to query for the OEM Information

   This command will obtain the OEM information data structure. The OEM
   information provided in the AcdbDataGeneralInfoType is a C string and will
   be zero terminated.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_OEM_INFO.
   @param[in] pCommandStruct:
         This should be a pointer to NULL.
   @param[in] nCommandStructLength:
         This should equal to 0
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGeneralInfoType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGeneralInfoType).

   @see  AcdbDataIoctl
   @see  AcdbDataGeneralInfoType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDBDATA_GET_OEM_INFO                             0xACDB0003

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_DATE_INFO Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_DATE_INFO, ...)
   @brief API to query for the Date Information

   This command will obtain the Date information data structure. The Date
   information provided in the AcdbDataGeneralInfoType is a C string and will
   be zero terminated.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_OEM_INFO.
   @param[in] pCommandStruct:
         This should be a pointer to NULL
   @param[in] nCommandStructLength:
         This should equal to 0
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGeneralInfoType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGeneralInfoType).

   @see  AcdbDataIoctl
   @see  AcdbDataGeneralInfoType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDBDATA_GET_DATE_INFO                             0xACDB0004

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY, ...)
   @brief API to query for the lookup key for the Aud Proc Common Table.

   This command will obtain the lookup key for the Aud Proc Common Table.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataAudProcCmnLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataAudProcCmnLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataAudProcCmnLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY      0xACDB0011

/**
   @struct   AcdbDataAudProcCmnLookupKeyType
   @brief This is a query command structure for the Aud Proc Common table.

   @param nDeviceId: The Device ID
   @param nDeviceSampleRateId: The Sample Rate of the Device
   @param nApplicationType: The Application Type ID
   
   This is a query command structure for the Aud Proc Common table.
*/
typedef struct _AcdbDataAudProcCmnLookupKeyType {
   uint32_t nDeviceId;
   uint32_t nDeviceSampleRateId;
   uint32_t nApplicationType;
} AcdbDataAudProcCmnLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY, ...)
   @brief API to query for the lookup key for the Aud Proc Stream Table.

   This command will obtain the lookup key for the Aud Proc Stream Table.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataAudProcStrmLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataAudProcStrmLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataAudProcStrmLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY     0xACDB0012

/**
   @struct   AcdbDataAudProcStrmLookupKeyType
   @brief This is a query command structure for the Aud Proc Stream table.

   @param nDeviceId: The Device ID
   @param nApplicationType: The Application Type ID
   
   This is a query command structure for the Aud Proc Stream table.
*/
typedef struct _AcdbDataAudProcStrmLookupKeyType {
   uint32_t nDeviceId;
   uint32_t nApplicationType;
} AcdbDataAudProcStrmLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY, ...)
   @brief API to query for the lookup key for the Voc Proc Common Table.

   This command will obtain the lookup key for the Voc Proc Common Table.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataVocProcCmnLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataVocProcCmnLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataVocProcCmnLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY      0xACDB0013

/**
   @struct   AcdbDataVocProcCmnLookupKeyType
   @brief This is a query command structure for the Voc Proc Common table.

   @param nTxDeviceId: The TX Device ID
   @param nRxDeviceId: The RX Device ID
   @param nTxDeviceSampleRateId: The Sample Rate of the TX Device
   @param nRxDeviceSampleRateId: The Sample Rate of the DX device
   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The Voc Proc Sample Rate Id
   
   This is a query command structure for the Voc Proc Common table.
*/
typedef struct _AcdbDataVocProcCmnLookupKeyType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nTxDeviceSampleRateId;
   uint32_t nRxDeviceSampleRateId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
} AcdbDataVocProcCmnLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY, ...)
   @brief API to query for the lookup key for the Voc Proc Stream Table.

   This command will obtain the lookup key for the Voc Proc Stream Table.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataVocProcStrmLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataVocProcStrmLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataVocProcStrmLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY     0xACDB0014

/**
   @struct   AcdbDataVocProcStrmLookupKeyType
   @brief This is a query command structure for the Voc Proc Stream table.

   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The Voc Proc Sample Rate ID
   
   This is a query command structure for the Voc Proc Stream table.
*/
typedef struct _AcdbDataVocProcStrmLookupKeyType {
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
} AcdbDataVocProcStrmLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY, ...)
   @brief API to query for the lookup key for the Aud Proc Vol Table.

   This command will obtain the lookup key for the Aud Proc Vol Table.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataAudProcVolLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataAudProcVolLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataAudProcVolLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY      0xACDB0015

/**
   @struct   AcdbDataAudProcVolLookupKeyType
   @brief This is a query command structure for the Aud Proc Vol table.

   @param nDeviceId: The Device ID
   @param nApplicationType: The Application Type ID
   @param nVolIndex: The Volume table index
   
   This is a query command structure for the Aud Proc Vol table.
*/
typedef struct _AcdbDataAudProcVolLookupKeyType {
   uint32_t nDeviceId;
   uint32_t nApplicationType;
   uint32_t nVolIndex;
} AcdbDataAudProcVolLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_VOC_PROC_VOL_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_LOOKUP_KEY, ...)
   @brief API to query for the lookup key for the Voc Proc Vol Table.

   This command will obtain the lookup key for the Voc Proc Vol Table.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_VOC_PROC_VOL_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataVocProcVolLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataVocProcVolLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataVocProcVolLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_VOC_PROC_VOL_LOOKUP_KEY      0xACDB0016

/**
   @struct   AcdbDataVocProcVolLookupKeyType
   @brief This is a query command structure for the Voc Proc Vol table.

   @param nTxDeviceId: The TX Device ID
   @param nRxDeviceId: The RX Device ID
   @param nNetworkId: The Network ID
   @param nVocProcSampleRateId: The Voc Proc Sample Rate Id
   @param nVolIndex: The Volume table index
   
   This is a query command structure for the Voc Proc Vol table.
*/
typedef struct _AcdbDataVocProcVolLookupKeyType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   uint32_t nVolIndex;
} AcdbDataVocProcVolLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_DEV_PAIR_TABLE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_DEV_PAIR_TABLE, ...)
   @brief API to query for the device pair table.

   This command will obtain the device pair table. The Device Pair table will
   be of format:
      struct DevicePairList {
         uint32_t nCount;
         struct DevicePair {
            uint32_t nTxDeviceId;
            uint32_t nRxDeviceId;
         } aDevicePairList [nCount];
      };

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_DEV_PAIR_TABLE.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGeneralInfoType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGeneralInfoType).

   @see  AcdbDataIoctl
   @see  AcdbDataGeneralInfoType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_DEV_PAIR_TABLE                         0xACDB0021

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the Aud Proc Common data table and topology

   This command will obtain the Aud Proc Common data table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY        0xACDB0022

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the Aud Proc Stream data table and topology

   This command will obtain the Aud Proc Stream data table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY       0xACDB0023

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the Voc Proc Common data table and topology

   This command will obtain the Voc Proc Common data table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY        0xACDB0024

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the Voc Proc Stream data table and topology

   This command will obtain the Voc Proc Stream data table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY       0xACDB0025

/* ------------------------------------------------------------------------------------
 * ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY Declarations and Documentation
 *------------------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the Volume Gain Dependent Aud Proc Volume data 
         table and topology

   This command will obtain the Volume Gain Dependent Aud Proc Volume data 
         table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetAudProcVolTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetAudProcVolTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetAudProcVolTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY        0xACDB0026

/* ------------------------------------------------------------------------------------
 * ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY Declarations and Documentation
 *------------------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the Voc Proc Vol Gain Dependent data table and topology

   This command will obtain the Voc Proc Vol Gain Dependent data table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY        0xACDB0027

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOL_TABLE_STEP_SIZE
 *    Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_VOL_TABLE_STEP_SIZE, ...)
   @brief API to query for a audio volume table step size.

   This command will obtain the generic AudProc and VocProc volume table step size 
   queried for.

   @param[out] nCommandId:
         The command id is ACDB_CMD_GET_VOL_TABLE_STEP_SIZE.
   @param[in] There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbVolTblStepSizeType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbVolTblStepSizeType).

   @see  acdb_ioctl
   @see  AcdbDataGetVolTblStepSize
*/
#define ACDBDATA_GET_VOL_TABLE_STEP_SIZE                    0xACDB0029

/**
   @struct   AcdbDataVolTblStepSizeType

   @param VocProcVolTblStepSize: Number of VocProc Volume table step size.
   @param AudProcVolTblStepSize: Number of AudProc Volume table step size.
   
   This structure will contain the VocProc and AudProc calibration volume table 
   step size entries. The number of step size entries in the AudProc and VocProc 
   volume table must always be the same across tables!
*/
typedef struct _AcdbDataVolTblStepSizeType {
   uint32_t VocProcVolTblStepSize;
   uint32_t AudProcVolTblStepSize;
} AcdbDataVolTblStepSizeType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_ANC_DEVICE_PAIR_TABLE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_ANC_DEVICE_PAIR_TABLE, ...)
   @brief API to query for the a list of all ANC paired Tx device Id and 
          Rx device Id.

   This command will obtain a list of all ANC device pairs Rx and Tx devices. 

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_ANC_DEVICE_PAIR_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to NULL.
   @param[in] nCommandStructLength:
         This should equal to 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGeneralInfoType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGeneralInfoType).

   @see  AcdbDataIoctl
   @see  AcdbDataGeneralInfoType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDBDATA_GET_ANC_DEVICE_PAIR_TABLE                    0xACDB002A

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE, ...)
   @brief API to query for the Adie codec path profile.

   This command will obtain Adie codec path profile lookup key. 

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataAdieProfileLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataAdieProfileLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataAdieProfileLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_LOOKUP_KEY      0xACDB002B

/**
   @struct   AcdbDataAdieProfileLookupKeyType
   @brief This is a query command structure for the Adie codec path profile lookup key.

   @param nCodecPathId: The Codec Path ID 
   @param nFrequencyId: The Frequency Index ID
   @param nOverSampleRateId: The Over Sample Rate ID
   
   This is a query command structure for the Adie codec path profile lookup key.
*/
typedef struct _AcdbDataAdieProfileLookupKeyType {
   uint32_t nCodecPathId;
   uint32_t nFrequencyId;
   uint32_t nOverSampleRateId;
} AcdbDataAdieProfileLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_LOOKUP_KEY, ...)
   @brief API to query for the Adie codec path ANC config data lookup key.

   This command will obtain Adie codec path ANC config data lookup key. 

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataAdieANCConfigDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataAdieANCConfigDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataAdieANCConfigDataLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_LOOKUP_KEY       0xACDB002C

/**
   @struct   AcdbDataAdieANCConfigDataLookupKeyType
   @brief This is a query command structure for the Adie codec path ANC config 
          data lookup key.

   @param nRxDeviceId: The RX Device ID
   @param nFrequencyId: The Frequency Index ID
   @param nOverSampleRateId: The Over Sample Rate ID
   
   This is a query command structure for the Adie codec path ANC config data lookup
   key.
*/
typedef struct _AcdbDataAdieANCConfigDataLookupKeyType {
   uint32_t nRxDeviceId;
   uint32_t nFrequencyId;
   uint32_t nOverSampleRateId;
} AcdbDataAdieANCConfigDataLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_TABLE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_TABLE, ...)
   @brief API to query for the Adie codec path profile table

   This command will obtain the Adie codec path profile table structure.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetParamType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetParamType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetParamType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_TABLE         0xACDB002D

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_TABLE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_TABLE, ...)
   @brief API to query for the Adie codec path ANC config data table

   This command will obtain the Adie codec path ANC config data table structure.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetParamType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetParamType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetParamType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_TABLE          0xACDB002E

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_TABLE_SIZE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_TABLE_SIZE, ...)
   @brief API to query for the table size

   This command will obtain size of VocProc common, VocProc stream, VocProc volume, 
                AudProc common, AudProc stream, AudProc volume, Adie codec path profile 
                and ANC config data table structure.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_TABLE_SIZE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataGetTblSizeCmdType.
   @param[in] nCommandStructLength:
         This should equal to size of (AcdbDataGetTblSizeCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblSizeRspType.
   @param[in] nResponseStructLength:
         This should equal to sizeof (AcdbDataGetTblSizeRspType).

   @see  AcdbDataIoctl
   @see  AcdbDataGetTblSizeCmdType
   @see  AcdbDataGetTblSizeRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_BADSTATE: When ACDB is in a bad state.
*/
#define ACDBDATA_GET_TABLE_SIZE                       0xACDB000C

/**
   @struct   AcdbDataGetTblSizeCmdType

   @param nParamId: A parameter Id query information of assocaiting with VocProc 
                    common, VocProc stream, VocProc volume, AudProc common, AudProc 
                    stream, AudProc volume, Adie codec path profile, ANC config 
                    data tables.
   
   This structure will contain calibration table parameter Id associate with.
*/
typedef struct _AcdbDataGetTblSizeCmdType {
   uint32_t nParamId;
} AcdbDataGetTblSizeCmdType;

/**
   @struct   AcdbDataGetTblSizeRspType

   @param nEntries: Number of table entries size.
   
   This structure will contain calibration table number of entries size.
*/
typedef struct _AcdbDataGetTblSizeRspType {
   uint32_t nEntries;
} AcdbDataGetTblSizeRspType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_TABLE_LOOKUP_INDEX Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_TABLE_LOOKUP_INDEX, ...)
   @brief API to query for the table lookup indices

   This command will obtain lookup indices of VocProc common, VocProc stream, VocProc 
                volume, AudProc common, AudProc stream, AudProc volume, Adie codec path 
                profile and ANC config data table structure.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_TABLE_LOOKUP_INDEX.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataGetTblIndexCmdType.
   @param[in] nCommandStructLength:
         This should equal to size of AcdbDataGetTblIndexCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblIndexRspType.
   @param[in] nResponseStructLength:
         This should equal to sizeof (AcdbDataGetTblIndexRspType).

   @see  AcdbDataIoctl
   @see  AcdbDataGetTblIndexCmdType
   @see  AcdbDataGetTblIndexRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_BADSTATE: When ACDB is in a bad state.
*/
#define ACDBDATA_GET_TABLE_LOOKUP_INDEX               0xACDB0005

/**
   @struct  AcdbDataGetTblIndexCmdType
   
   @param nParamId: Parameter ID associate with vocproc common, audproc common, 
                    vocproc stream, audproc stream, vocproc gain-dependent, audproc 
                    gain-dependent, adie codec path profile and ANC config data tables.
   @param nTblKeyIndex: The table key Index

   This is a query structure for query the lookup index combination of vocproc common,
   audproc common, vocproc stream, audproc stream, vocproc gain-dependent, audproc 
   gain-dependent, adie codec path profile and ANC config data tables.
*/
typedef struct _AcdbDataGetTblIndexCmdType {
   uint32_t nParamId;
   uint32_t nTblKeyIndex;
} AcdbDataGetTblIndexCmdType;

/**
   @struct  AcdbDataGetTblIndexCmdType
   
   @param pBufferPointer: A table to table entry
   @param nAvailableByteToCopy: This is to use for boundary check to ensure no memory
                                corruption when performing memcpy

   This is a response structure of query the lookup index combination of vocproc common,
   audproc common, vocproc stream, audproc stream, vocproc gain-dependent, audproc 
   gain-dependent, adie codec path profile and ANC config data tables.
*/
typedef struct _AcdbDataGetTblIndexRspType {
   uint8_t *pBufferPointer;
   uint32_t nAvailableByteToCopy;
} AcdbDataGetTblIndexRspType;

/**
   @def ACDBDATA_PID_VOCPROC_COMMON_TABLE
*/
#define ACDBDATA_PID_VOCPROC_COMMON_TABLE              0xACDB000D
/**
   This paramId is to use for query VocProc common table lookup index and
   number of VocProc common table entries
   
   Returned VocProc common table entries buffer structure format
   struct VocProcCmnLookupIndex{
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nTxDeviceSampleRateId;
   uint32_t nRxDeviceSampleRateId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   }

   struct VocProcCmnIndexList {
   VocProcCmmLookupIndex [number of vocProc common table entries];
   }
*/

/**
   @def ACDBDATA_PID_VOCPROC_STREAM_TABLE
*/
#define ACDBDATA_PID_VOCPROC_STREAM_TABLE             0xACDB000E
/**
   This paramId is to use for query VocProc stream table lookup index and
   number of VocProc stream table entries

   Returned VocProc stream table entries buffer structure format
   struct VocProcStrmLookupIndex{
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   }

   struct VocProcStrmIndexList {
   VocProcCmmLookupIndex [number of vocProc stream table entries];
   }
*/

/**
   @def ACDBDATA_PID_VOCPROC_VOLUME_TABLE
*/
#define ACDBDATA_PID_VOCPROC_VOLUME_TABLE             0xACDB000F
/**
   This paramId is to use for query VocProc volume gain-dependent table lookup
   index and number of VocProc volume table entries

   Returned VocProc volume table entries buffer structure format
   struct VocProcVolLookupIndex{
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   }

   struct VocProcVolIndexList {
   VocProcCmmLookupIndex [number of vocProc volume table entries];
   }
   */

/**
   @def ACDBDATA_PID_AUDPROC_COMMON_TABLE
*/
#define ACDBDATA_PID_AUDPROC_COMMON_TABLE             0xACDB0010
/**
   This paramId is to use for query AudProc common table lookup index and 
   number of AudProc common table entries

   Returned AudProc common table entries buffer structure format
   struct AudProcCmnLookupIndex{
   uint32_t nDeviceId;
   uint32_t nDeviceSampleRateId;
   uint32_t nApplicationType;
   }

   struct AudProcCmnIndexList {
   AudProcCmnLookupIndex [number of audproc common table entries];
   }
*/

/**
   @def ACDBDATA_PID_AUDPROC_STREAM_TABLE
*/
#define ACDBDATA_PID_AUDPROC_STREAM_TABLE              0xACDB0017
/**
   This paramId is to use for query AudProc stream table lookup index and 
   number of AudProc stream table entries

   Returned AudProc stream table entries buffer structure format
   struct AudProcStrmLookupIndex{
   uint32_t nDeviceId;
   uint32_t nApplicationTypeId;
   }

   struct AudProcStrmIndexList {
   AudProcStrmLookupIndex [number of audproc stream table entries];
   }
*/

/**
   @def ACDBDATA_PID_AUDPROC_VOLUME_TABLE
*/
#define ACDBDATA_PID_AUDPROC_VOLUME_TABLE             0xACDB0018
/**
   This paramId is to use for query AudProc volume gain-dependent table lookup
   index and number of AudProc volume table entries

   Returned AudProc volume table entries buffer structure format
   struct AudProcVolLookupIndex{
   uint32_t nDeviceId;
   uint32_t nApplicationType
   }

   struct AudProcVolIndexList {
   AudProcVolLookupIndex [number of audproc volume table entries];
   }
*/

/**
   @def ACDBDATA_PID_ADIE_PROFILE_TABLE
*/
#define ACDBDATA_PID_ADIE_PROFILE_TABLE               0xACDB0019
/**
   This paramId is to use for query Adie codec path prfile table lookup index
   and number of Adie codec path profile table entries

   Returned Adie codec path profile table entries buffer structure format
   struct AdieProfileLookupIndex{
   uint32_t ulCodecPathId;
   uint32_t nFrequencyId;
   uint32_t nOversamplerateId;
   }

   struct AdieProfileIndexList {
   AdieProfileLookupIndex [number of Adie codec path profile table entries];
   }
*/

/**
   @def ACDBDATA_PID_ANC_CONFIG_DATA_TABLE
*/
#define ACDBDATA_PID_ANC_CONFIG_DATA_TABLE            0xACDB001A
/**
   This paramId is to use for query ANC config data table lookup index and number
   of ANC config data table entries
   
   Returned ANC config data table entries buffer structure format
   struct ANCConfigDataLookupIndex{
   uint32_t nRxDeviceId;
   uint32_t nFrequencyId;
   uint32_t nOversamplerateId;
   }

   struct ANCConfigDataIndexList {
   ANCConfigDataLookupIndex [number of ANC config data table entries];
   }
*/

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AUDPROC_CMN_TOPOLOGY_ID Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AUDPROC_CMN_TOPOLOGY_ID, ...)
   @brief API to query for the AudProc common (COPP) topology Id

   This command will obtain topology Id of AudProc common (COPP)

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AUDPROC_CMN_TOPOLOGY_ID.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataGetAudProcCmnTopIdCmdType.
   @param[in] nCommandStructLength:
         This should equal to size of AcdbDataGetAudProcCmnTopIdCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTopologyIdRspType.
   @param[in] nResponseStructLength:
         This should equal to sizeof (AcdbDataGetTopologyIdRspType).

   @see  AcdbDataIoctl
   @see  AcdbDataGetAudProcCmnTopIdCmdType
   @see  AcdbDataGetTopologyIdRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/

#define ACDBDATA_GET_AUDPROC_CMN_TOPOLOGY_ID           0xACDB0006

/**
   @struct  AcdbDataGetAudProcCmnTopIdCmdType
   
   @param nDeviceId: The device ID.
   @param nAppTypeId: The application type ID.

   This is a query structure for query the AudProc common topology ID.
*/
typedef struct _AcdbDataGetAudProcCmnTopIdCmdType {
   uint32_t nDeviceId;
   uint32_t nAppTypeId;
} AcdbDataGetAudProcCmnTopIdCmdType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AUDPROC_STRM_TOPOLOGY_ID Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AUDPROC_STRM_TOPOLOGY_ID, ...)
   @brief API to query for the AudProc stream (POPP) topology Id

   This command will obtain topology Id of AudProc stream (POPP)

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AUDPROC_STRM_TOPOLOGY_ID.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataGetAudProcStrmTopIdCmdType.
   @param[in] nCommandStructLength:
         This should equal to size of AcdbDataGetAudProcStrmTopIdCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTopologyIdRspType.
   @param[in] nResponseStructLength:
         This should equal to sizeof (AcdbDataGetTopologyIdRspType).

   @see  AcdbDataIoctl
   @see  AcdbDataGetAudProcStrmTopIdCmdType
   @see  AcdbDataGetTopologyIdRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/

#define ACDBDATA_GET_AUDPROC_STRM_TOPOLOGY_ID         0xACDB0007

/**
   @struct  AcdbDataGetAudProcStrmTopIdCmdType
   
   @param nAppTypeId: The application type ID.

   This is a query structure for query the AudProc common topology ID.
*/
typedef struct _AcdbDataGetAudProcStrmTopIdCmdType {
   uint32_t nAppTypeId;
} AcdbDataGetAudProcStrmTopIdCmdType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_VOCPROC_CMN_TOPOLOGY_ID Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_VOCPROC_CMN_TOPOLOGY_ID, ...)
   @brief API to query for the VocProc common (COPP) topology Id

   This command will obtain topology Id of VocProc common (COPP)

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_VOCPROC_CMN_TOPOLOGY_ID.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataGetVocProcCmnTopIdCmdType.
   @param[in] nCommandStructLength:
         This should equal to size of AcdbDataGetVocProcCmnTopIdCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTopologyIdRspType.
   @param[in] nResponseStructLength:
         This should equal to sizeof (AcdbDataGetTopologyIdRspType).

   @see  AcdbDataIoctl
   @see  AcdbDataGetAudProcCmnTopIdCmdType
   @see  AcdbDataGetTopologyIdRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/

#define ACDBDATA_GET_VOCPROC_CMN_TOPOLOGY_ID           0xACDB001F

#define ACDBDATA_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID            0xACDB0020

/**
   @struct  AcdbDataGetVocProcCmnTopIdCmdType
   
   @param nDeviceId: The device ID.

   This is a query structure for query the AudProc common topology ID.
*/
typedef struct _AcdbDataGetVocProcCmnTopIdCmdType {
   uint32_t nDeviceId;
} AcdbDataGetVocProcCmnTopIdCmdType;

/**
   @struct  AcdbDataGetTopologyIdRspType
   
   @param nTopologyId: The Topology ID.

   This is a response structure for query the AudProc common and stream 
   topology ID.
*/
typedef struct _AcdbDataGetTopologyIdRspType {
   uint32_t nTopologyId;
} AcdbDataGetTopologyIdRspType;

/**
   @struct  AcdbDataGetRmtCompDevIdCmdType
   
   @param nNativeDeviceId: The device ID.

   This is a query structure for query the compatible remote device id.
*/
typedef struct _AcdbDataGetRmtCompDevIdCmdType {
   uint32_t nNativeDeviceId;
} AcdbDataGetRmtCompDevIdCmdType;

/**
   @struct  AcdbDataGetRmtCompDevIdRspTyp
   
   @param nRemoteDeviceId: The Device ID.

   This is a response structure for query the get compatible remote device id.
*/
typedef struct _AcdbDataGetRmtCompDevIdRspTyp {
   uint32_t nRemoteDeviceId;
} AcdbDataGetRmtCompDevIdRspTyp;


/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_TOPOLOGY_ID_LIST Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST, ...)
   @brief API to query for the table lookup indices

   This command will obtain topology Id of AudProc common (COPP), AudProc stream
        (POPP).

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_TOPOLOGY_ID_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataGetTopIdCmdType.
   @param[in] nCommandStructLength:
         This should equal to size of AcdbDataGetTopIdCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal to sizeof (AcdbQueryResponseType).

   @see  AcdbDataIoctl
   @see  AcdbDataGetTopIdCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDBDATA_GET_TOPOLOGY_ID_LIST                 0xACDB0008

/**
   @def ACDBDATA_PID_AUDPROC_COMMON_TOPOLOGY_ID
*/
#define ACDBDATA_PID_AUDPROC_COMMON_TOPOLOGY_ID       0xACDB0009
/**
   This paramId is to use for query AudProc common topology Id table

   Returned AudProc common topology table entries buffer structure format
   struct _AudPricCmnTopologyListType {
      uint32_t nCount;
      struct AudProcCmnTopologyType {
         uint32_t nDeviceId;
         uint32_t nApplicationType;
         uint32_t nTopologyId;
      } AudProcCmnTopologyTable[nCount];
   } AudPricCmnTopologyListType;
*/

/**
   @def ACDBDATA_PID_AUDPROC_STREAM_TOPOLOGY_ID
*/
#define ACDBDATA_PID_AUDPROC_STREAM_TOPOLOGY_ID       0xACDB000A
/**
   This paramId is to use for query AudProc stream topology Id table

   Returned AudProc stream topology table entries buffer structure format
   struct _AudProcStrmTopologyListType {
      uint32_t nCount;
      struct AudProcStrmTopologyType {
         uint32_t nApplicationType;
         uint32_t nTopologyId;
      } AudProcStrmTopologyTable[nCount];
   } AudProcStrmTopologyListType;
*/

/**
   @def ACDBDATA_PID_VOCPROC_COMMON_TOPOLOGY_ID
*/
#define ACDBDATA_PID_VOCPROC_COMMON_TOPOLOGY_ID       0xACDB0020
/**
   This paramId is to use for query VocProc common topology Id table

   Returned VocProc common topology table entries buffer structure format
   struct _VocProcCmnTopologyListType {
      uint32_t nCount;
      struct _VocProcCmnTopologyType {
         uint32_t nDeviceId;
         uint32_t nTopologyId;
      } VocProcCmnTopologyType[nCount];
   } VocProcCmnTopologyListType;
*/

/**
   @struct  AcdbDataGetTopologyIdCmdType
   
   @param nParamId: Parameter ID associate with audproc common, audproc stream.
   @param pBufferPoint: The pointer points to the AudProc common, AudProc Stream
                        Topology table.

   This is a query structure for query the AudProc common, AudProc stream 
           topology tables.
*/
typedef struct _AcdbDataGetTopIdCmdType {
   uint32_t nParamId;
   uint8_t *pBufferPointer;
} AcdbDataGetTopIdCmdType;

typedef struct _AcdbDataTopIdCmdType {
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *pBufferPointer;
} AcdbDataTopIdCmdType;


/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AFE_DATA_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AFE_DATA_LOOKUP_KEY, ...)
   @brief API to query for the Afe table lookup key.

   This command will obtain Afe table lookup key. 

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AFE_DATA_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataAfeLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataAfeLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataAfeLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AFE_DATA_LOOKUP_KEY                0xACDB000B

/**
   @struct   AcdbDataAfeLookupKeyType
   @brief This is a query command structure for the Afe table lookup key.

   @param nTxDeviceId: The Tx path device Id 
   @param nRxDeviceId: The Rx path device Id
   
   This is a query command structure for the Afe table lookup key.
*/
typedef struct _AcdbDataAfeLookupKeyType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
} AcdbDataAfeLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AFE_TABLE_AND_TOPOLOGY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AFE_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the AFE data table and topology

   This command will obtain the AFE data table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AFE_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AFE_TABLE_AND_TOPOLOGY        0xACDB001C

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_GLOBAL_TABLE_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_GLOBAL_TABLE_LOOKUP_KEY, ...)
   @brief API to query for the Global table lookup key.

   This command will obtain global table lookup key. 

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_GLOBAL_TABLE_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataGlbTblLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataGlbTblLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataGlbTblLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_GLOBAL_TABLE_LOOKUP_KEY       0xACDB001B
/**
   @struct   AcdbDataGlbTblLookupKeyType
   @brief This is a query command structure for the Adie codec path ANC config 
          data lookup key.

   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   
   This is a query command structure for the global table lookup
   key.
*/
typedef struct _AcdbDataGlbTblLookupKeyType {
   uint32_t nModuleId;
   uint32_t nParamId;
} AcdbDataGlbTblLookupKeyType;

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_GLOBAL_TABLE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_GLOBAL_TABLE, ...)
   @brief API to query for the global table

   This command will obtain the global table structure.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_GLOBAL_TABLE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetGlbTblParamType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetGlbTblParamType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetGlbTblParamType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_GLOBAL_TABLE                  0xACDB001D

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_DEVICE_INFO_TABLE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_DEVICE_INFO_TABLE, ...)
   @brief API to query for the device info table.

   This command will obtain the device info table.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_DEV_INFO_TABLE.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTableType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTableType).

   @see  AcdbDataIoctl
   @see  AcdbDataGetTableType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_DEV_INFO_TABLE               0xACDB001E

/* ---------------------------------------------------------------------------
 *  ACDBDATA_GET_SW_TARGET_VERSION Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_SW_TARGET_VERSION, ...)
   @brief API to query for the compiled target version.

   This command will obtain the compiled target version that is stored in the default
   includes file.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_SW_TARGET_VERSION.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataTargetVersionType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataTargetVersionType).

   @see  AcdbDataIoctl
   @see  AcdbDataTargetVersionType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
*/

#define ACDBDATA_GET_SW_TARGET_VERSION      0xACDB0034

/* ---------------------------------------------------------------------------
 *  ACDBDATA_GET_SW_ACDB_VERSION Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_SW_ACDB_VERSION, ...)
   @brief API to query for the software major and minor version.

   This command will obtain the software major and minor version version that is stored in the sw_version
   includes file.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_SW_ACDB_VERSION.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbModuleVersionType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbModuleVersionType).

   @see  AcdbDataIoctl
   @see  AcdbModuleVersionType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
*/

#define ACDBDATA_GET_SW_ACDB_VERSION      0xACDB0035

/* ---------------------------------------------------------------------------
 *  ACDBDATA_GET_SW_ACDB_DATA_STRUCTURE_VERSION Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_SW_ACDB_DATA_STRUCTURE_VERSION, ...)
   @brief API to query for the compiled target version.

   This command will obtain the software data structure version that is stored in the sw_version
   includes file.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_SW_ACDB_DATA_STRUCTURE_VERSION.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataModuleVersionType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataModuleVersionType).

   @see  AcdbDataIoctl
   @see  AcdbDataModuleVersionType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
*/

#define ACDBDATA_GET_SW_ACDB_DATA_STRUCTURE_VERSION      0xACDB0036

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_QACT_DATA Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_QACT_DATA, ...)
   @brief API to query for the QACT data.

   This command will obtain the QACT data.

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_QACT_DATA.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetParamType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetParamType).

   @see  AcdbDataIoctl
   @see  AcdbDataGetGlbTblParamType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_QACT_DATA                           0xACDB0028



/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY, ...)
   @brief API to query for the Afe table lookup key.

   This command will obtain Afe table lookup key. 

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataAfeCmnLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataAfeCmnLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataAfeCmnLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY                0xACDB0030


typedef struct _AcdbDataAfeCmnLookupKeyType {
   uint32_t nDeviceId;
   uint32_t nAfeSampleRateId;
}AcdbDataAfeCmnLookupKeyType;



/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the AFE data table and topology

   This command will obtain the AFE data table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY        0xACDB0031

/**
   @def ACDBDATA_PID_AFECMN_TABLE
*/
/**
   This paramId is to use for query Afe common table lookup index and 
   number of Afe common table entries

   Returned Afe common table entries buffer structure format
   struct AfeCmnLookupIndex{
   uint32_t nDeviceId;
   uint32_t nAfeSampleRateId;
   }

   struct AfeCmnIndexList {
   AfeCmnLookupIndex [number of afe common table entries];
   }
*/
#define ACDBDATA_PID_AFE_COMMON_TABLE                       0xACDB0032

/* ---------------------------------------------------------------------------
 * ACDBDATA_CMD_GET_AFE_TOPOLOGY_LIST Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_CMD_GET_AFE_TOPOLOGY_LIST, ...)
   @brief API to query for the device pair table.

   This command will obtain the device pair table. The Device Pair table will
   be of format:
   struct AfeTopologyList {
      uint32_t nCount;
      struct AfeTopItem {
         uint32_t nDeviceID;
         uint32_t nAfeTopologyID;
      } AfeTopItem[nCount];
   };

   @param[in] nCommandId:
         The command id is ACDBDATA_CMD_GET_AFE_TOPOLOGY_LIST.
   @param[in] pCommandStruct:
         There is no command structure. This should be NULL.
   @param[in] nCommandStructLength:
         There is no command structure. This should be 0.
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGeneralInfoType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGeneralInfoType).

   @see  AcdbDataIoctl
   @see  AcdbDataGeneralInfoType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_CMD_GET_AFE_TOPOLOGY_LIST                         0xACDB0033


#define ACDBDATA_CMD_GET_DEVICE_CHANNEL_TYPE_LIST                  0xACDB003B

#define ACDBDATA_CMD_GET_AUD_VOL_POPP_MODULE_LIST                         0xACDB0037

/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_ADIE_SIDETONE_DATA_LOOKUP_KEY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_ADIESIDETONE_DATA_LOOKUP_KEY, ...)
   @brief API to query for the AdieSidetone table lookup key.

   This command will obtain AdieSidetone table lookup key. 

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_ADIE_SIDETONE_DATA_LOOKUP_KEY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataAdieSidetoneLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataAdieSidetoneCmnLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).

   @see  AcdbDataIoctl
   @see  AcdbDataAdieSidetoneLookupKeyType
   @see  AcdbDataLookupKeyType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_ADIE_SIDETONE_DATA_LOOKUP_KEY                0xACDB0038

typedef struct _AcdbDataAdieSidetoneLookupKeyType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
}AcdbDataAdieSidetoneLookupKeyType;



/* ---------------------------------------------------------------------------
 * ACDBDATA_GET_ADIE_SIDETONE_TABLE_AND_TOPOLOGY Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDBDATA_GET_ADIE_SIDETONE_TABLE_AND_TOPOLOGY, ...)
   @brief API to query for the AdieSidetone data table and topology

   This command will obtain the AdieSidetone data table and topology

   @param[in] nCommandId:
         The command id is ACDBDATA_GET_ADIE_SIDETONE_TABLE_AND_TOPOLOGY.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbDataLookupKeyType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbDataLookupKeyType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbDataGetTblTopType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbDataGetTblTopType).

   @see  AcdbDataIoctl
   @see  AcdbDataLookupKeyType
   @see  AcdbDataGetTblTopType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_PARMNOTFOUND: When the key is not found in the table.
*/
#define ACDBDATA_GET_ADIE_SIDETONE_TABLE_AND_TOPOLOGY        0xACDB0039

/**
   @def ACDBDATA_PID_ADIE_SIDETONE_TABLE
*/
/**
   This paramId is to use for query Adie sidetone table lookup index and 
   number of AdieSidetone common table entries

   Returned Adie Sidetone table entries buffer structure format
   struct AdieSidetoneLookupIndex{
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   }

   struct AdieSidetoneIndexList {
   AdieSidetoneLookupIndex [number of adie Sidetone table entries];
   }
*/
#define ACDBDATA_PID_ADIE_SIDETONE_TABLE                       0xACDB003A






typedef struct _AcdbDataGetAudProcTopIdType {
   uint32_t nDeviceId;
   uint32_t nAppTypeId;
   uint32_t nTopoId;
} AcdbDataGetAudProcTopIdType;

typedef struct _AcdbDataGetAudStrmTopIdType {
   uint32_t nDeviceId;
   uint32_t nTopoId;
} AcdbDataGetAudStrmTopIdType;

typedef struct _AcdbDataGetVocProcTopIdType {
   uint32_t nDeviceId;
   uint32_t nTopoId;
} AcdbDataGetVocProcTopIdType ;

typedef struct _AcdbDataGetAfeCmnTopIdType {
   uint32_t nDeviceId;
   uint32_t nTopoId;
} AcdbDataGetAfeCmnTopIdType;



#define ACDB_PID_DEVPAIR                 0x00011184     //VocProc Device Pairs
#define ACDB_PID_AUD_VOL_STEPS           0x00011187     //Audio vol steps
#define ACDB_PID_VOC_VOL_STEPS           0x00011188     //Voice vol steps
#define ACDB_PID_ANCDEVPAIR              0x00011189     //ANC device pairs
#define ACDB_PID_AUDPROC_CMN_TOPID       0x0001122B     //AudProc Common TopologyId list
#define ACDB_PID_AUDPROC_STRM_TOPID      0x00011231     //AudProc Stream TopologyId list
#define ACDB_PID_VOCPROC_CMN_TOPID       0x000112A6     //VocProc Common TopologyId list
#define ACDB_PID_AFE_CMN_TOP_TABLE       0x000112F8     //Afe topology list    

#define ACDB_PID_AUD_POPP_MODULELIST_GLBTBL  0x00011322  //Audio POPP module list in the global table


/*===========================================================================

FUNCTION AcdbDataIoctl

DESCRIPTION
   Main entry function to the ACDB. This entry function will take any
   supported ACDB IOCTL and provide the appropriate response.

   @param[in] nCommandId
         Command id to execute on the Audio Calibration Database. The
         pCommandStruct and the pResponseStruct should match the expected
         structures for that command. If not, the command will fail.
   @param[in] pInput
         Pointer to the command structure.
   @param[in] nInputSize
         The size of command structure.
   @param[out] pOutput
         Pointer to the response structure.
   @param[in] nOutputSize
         The size of the response structure.

   Supported commands are:
      ACDBDATA_GET_ACDB_VERSION
      ACDBDATA_GET_TARGET_VERSION
      ACDBDATA_GET_OEM_INFO
      ACDBDATA_GET_DATE_INFO
      ACDBDATA_ESTIMATE_MEMORY_USE
      ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY
      ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY
      ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY
      ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY
      ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY
      ACDBDATA_GET_VOC_PROC_VOL_LOOKUP_KEY      
      ACDBDATA_GET_DEV_PAIR_TABLE
      ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY
      ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY
      ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY
      ACDBDATA_GET_AUD_PROC_VOl_GAIN_DEP_CAL_TABLE_AND_TOPOLOGY
      ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY
      ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY
      ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY
      ACDBDATA_GET_ANC_PAIRED_TX_DEVICE
      ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_LOOKUP_KEY
      ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_LOOKUP_KEY
      ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_TABLE
      ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_TABLE
      ACDBDATA_GET_TABLE_SIZE
      ACDBDATA_GET_TABLE_LOOKUP_INDEX
      ACDBDATA_GET_AUDPROC_CMN_TOPOLOGY_ID
      ACDBDATA_GET_AUDPROC_STRM_TOPOLOGY_ID
      ACDBDATA_GET_TOPOLOGY_ID_LIST
      ACDBDATA_GET_AFE_SIDETONE_DATA_LOOKUP_KEY
      ACDBDATA_GET_AFE_TABLE_AND_TOPOLOGY
      ACDBDATA_GET_GLOBAL_TABLE_LOOKUP_KEY
      ACDBDATA_GET_GLBTBL_TOPOLOGY
      ACDBDATA_GET_DEV_INFO_TABLE
      ACDBDATA_GET_VOCPROC_CMN_TOPOLOGY_ID
      ACDBDATA_GET_QACT_DATA
      ACDBDATA_GET_SW_TARGET_VERSION
      ACDBDATA_GET_SW_ACDB_VERSION
      ACDBDATA_GET_SW_ACDB_DATA_STRUCTURE_VERSION
      ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY
      ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY
      ACDBDATA_PID_AFE_COMMON_TABLE
      ACDBDATA_CMD_GET_AFE_TOPOLOGY_LIST
      ACDBDATA_CMD_GET_DEVICE_CHANNEL_TYPE_LIST

   Please see individual command documentation for more details on the
   expectations of the command.

DEPENDENCIES
   None

RETURN VALUE
   The result of the call as defined by the command.

SIDE EFFECTS
   None.

===========================================================================*/

int32_t AcdbDataIoctl (uint32_t nCommandId,
                        uint8_t *pInput,
                        uint32_t nInputSize,
                        uint8_t *pOutput,
                        uint32_t nOutputSize);

#endif /* __ACDB_DEFAULT_DATA_H__ */
