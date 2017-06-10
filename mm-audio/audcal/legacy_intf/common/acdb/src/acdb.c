
/*===========================================================================
    FILE:           acdb.c

    OVERVIEW:       This file contains the implementaion of the Acdb API and 
                    its helper methods.

    DEPENDENCIES:   None

                    Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_command.h"
#include "acdb.h"
#include "acdb_default_data.h"

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
int32_t acdb_ioctl (uint32_t nCommandId,
                    const uint8_t *pCommandStruct,
                    uint32_t nCommandStructLength,
                    uint8_t *pResponseStruct,
                    uint32_t nResponseStructLength)
{
   int32_t result = ACDB_SUCCESS;
   switch (nCommandId)
   {
   case ACDB_CMD_INITIALIZE:
      if (pCommandStruct != NULL || nCommandStructLength != 0 ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         result = AcdbCmdInitializeAcdb ();
      }
      break;
   case ACDB_CMD_GET_ACDB_VERSION:
      if (pCommandStruct != NULL || nCommandStructLength != 0 ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbModuleVersionType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbModuleVersionType *pSwVrs = (AcdbModuleVersionType *)pResponseStruct;
         result = AcdbCmdGetAcdbSwVersion (pSwVrs);
      }
      break;
   case ACDB_CMD_GET_TARGET_VERSION:
      if (pCommandStruct != NULL || nCommandStructLength != 0 ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbTargetVersionType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbTargetVersionType *pTgtVrs = (AcdbTargetVersionType *)pResponseStruct;
         result = AcdbCmdGetTargetVersion (pTgtVrs);
      }
      break;
   case ACDB_CMD_IS_DEVICE_PAIRED:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbDevicePairType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbDevicePairingResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbDevicePairType *pInput = (AcdbDevicePairType *)pCommandStruct;
         AcdbDevicePairingResponseType *pOutput = (AcdbDevicePairingResponseType *)pResponseStruct;
         result = AcdbCmdGetDevicePair (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_DEVICE_PAIR_LIST:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbGeneralInfoCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGeneralInfoCmdType *pInput = (AcdbGeneralInfoCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetDevicePairList (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_DEVICE_CAPABILITIES:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbDeviceCapabilitiesCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbDeviceCapabilitiesCmdType *pInput = (AcdbDeviceCapabilitiesCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetDeviceCapabilities (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_DEVICE_INFO:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbDeviceInfoCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbDeviceInfoCmdType *pInput = (AcdbDeviceInfoCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetDeviceInfo (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_OEM_INFO:
      if (pCommandStruct == NULL || nCommandStructLength == 0 ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGeneralInfoCmdType *pInput = (AcdbGeneralInfoCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetOEMInfo (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_DATE_INFO:
      if (pCommandStruct == NULL || nCommandStructLength == 0 ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGeneralInfoCmdType *pInput = (AcdbGeneralInfoCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetDateInfo (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_COMMON_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcTableCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcTableCmdType *pInput = (AcdbAudProcTableCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudProcTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_AUDPROC_COMMON_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcTableCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcTableCmdType *pInput = (AcdbAudProcTableCmdType *)pCommandStruct;
         result = AcdbCmdSetAudProcTable (pInput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_COMMON_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcCmdType *pInput = (AcdbAudProcCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudProcData (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_AUDPROC_COMMON_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcCmdType *pInput = (AcdbAudProcCmdType *)pCommandStruct;
         result = AcdbCmdSetAudProcData (pInput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_STREAM_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudStrmCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudStrmCmdType *pInput = (AcdbAudStrmCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudStrmData (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_AUDPROC_STREAM_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudStrmCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudStrmCmdType *pInput = (AcdbAudStrmCmdType *)pCommandStruct;
         result = AcdbCmdSetAudStrmData (pInput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_STREAM_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudStrmTableCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudStrmTableCmdType *pInput = (AcdbAudStrmTableCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudStrmTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_AUDPROC_STREAM_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudStrmTableCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudStrmTableCmdType *pInput = (AcdbAudStrmTableCmdType *)pCommandStruct;
         result = AcdbCmdSetAudStrmTable (pInput);
      }
      break;
   case ACDB_CMD_GET_VOCPROC_COMMON_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocProcTableCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocProcTableCmdType *pInput = (AcdbVocProcTableCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetVocProcTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_VOCPROC_COMMON_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocProcTableCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocProcTableCmdType *pInput = (AcdbVocProcTableCmdType *)pCommandStruct;
         result = AcdbCmdSetVocProcTable (pInput);
      }
      break;
   case ACDB_CMD_GET_VOCPROC_COMMON_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocProcCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocProcCmdType *pInput = (AcdbVocProcCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetVocProcData (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_VOCPROC_COMMON_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocProcCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocProcCmdType *pInput = (AcdbVocProcCmdType *)pCommandStruct;
         result = AcdbCmdSetVocProcData (pInput);
      }
      break;
   case ACDB_CMD_GET_VOCPROC_STREAM_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocStrmTableCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocStrmTableCmdType *pInput = (AcdbVocStrmTableCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetVocStrmTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_VOCPROC_STREAM_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocStrmTableCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocStrmTableCmdType *pInput = (AcdbVocStrmTableCmdType *)pCommandStruct;
         result = AcdbCmdSetVocStrmTable (pInput);
      }
      break;
   case ACDB_CMD_GET_VOCPROC_STREAM_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocStrmCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocStrmCmdType *pInput = (AcdbVocStrmCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetVocStrmData (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_VOCPROC_STREAM_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocStrmCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocStrmCmdType *pInput = (AcdbVocStrmCmdType *)pCommandStruct;
         result = AcdbCmdSetVocStrmData (pInput);
      }
      break;
   case ACDB_CMD_GET_VOL_TABLE_STEP_SIZE:
      if (pCommandStruct != NULL || nCommandStructLength != 0 ||
          pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbVolTblStepSizeRspType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVolTblStepSizeRspType *pOutput = (AcdbVolTblStepSizeRspType *)pResponseStruct;
         result = AcdbCmdGetVolTableStepSize (pOutput);
      }
      break;
   case ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocProcVolTblCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocProcVolTblCmdType *pInput = (AcdbVocProcVolTblCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetVocProcGainDepVolTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcVolTblCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbAudProcGainDepVolTblRspType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcVolTblCmdType *pInput = (AcdbAudProcVolTblCmdType *)pCommandStruct;
         AcdbAudProcGainDepVolTblRspType *pOutput = (AcdbAudProcGainDepVolTblRspType *)pResponseStruct;
         result = AcdbCmdGetAudProcGainDepVolTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_COPP:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcGainDepVolTblStepCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcGainDepVolTblStepCmdType *pInput = (AcdbAudProcGainDepVolTblStepCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudProcGainDepCoppTableStep (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_POPP:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcGainDepVolTblStepCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcGainDepVolTblStepCmdType *pInput = (AcdbAudProcGainDepVolTblStepCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudProcGainDepPoppTableStep (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcVolTblDataCmdType) ||
          pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcVolTblDataCmdType *pInput = (AcdbAudProcVolTblDataCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudProcGainDepData (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAudProcVolTblDataCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAudProcVolTblDataCmdType *pOutput = (AcdbAudProcVolTblDataCmdType *)pCommandStruct;
         result = AcdbCmdSetAudProcGainDepData (pOutput);
      }
      break;
   case ACDB_CMD_GET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocProcVolTblDataCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocProcVolTblDataCmdType *pInput = (AcdbVocProcVolTblDataCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetVocProcGainDepData (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbVocProcVolTblDataCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbVocProcVolTblDataCmdType *pOutput = (AcdbVocProcVolTblDataCmdType *)pCommandStruct;
         result = AcdbCmdSetVocProcGainDepData (pOutput);
      }
      break;
   case ACDB_CMD_GET_ANC_TX_DEVICE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAncDevicePairCmdType) ||
          pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbAncDevicePairRspType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAncDevicePairCmdType *pInput = (AcdbAncDevicePairCmdType *) pCommandStruct;
         AcdbAncDevicePairRspType *pOutput = (AcdbAncDevicePairRspType *) pResponseStruct;
         result = AcdbCmdGetANCDevicePair (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_ANC_DEVICE_PAIR_LIST:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbGeneralInfoCmdType) ||
          pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGeneralInfoCmdType *pInput = (AcdbGeneralInfoCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetANCDevicePairList (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_ADIE_CODEC_PATH_PROFILE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAdiePathProfileCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAdiePathProfileCmdType *pInput = (AcdbAdiePathProfileCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAdieProfileTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_ADIE_CODEC_PATH_PROFILE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAdiePathProfileCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAdiePathProfileCmdType *pInput = (AcdbAdiePathProfileCmdType *)pCommandStruct;
         result = AcdbCmdSetAdieProfileTable (pInput);
      }
      break;
   case ACDB_CMD_GET_ANC_SETTING:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbANCSettingCmdType) ||
         pResponseStruct == NULL || nResponseStructLength == 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbANCSettingCmdType *pInput = (AcdbANCSettingCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAdieANCDataTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_ANC_SETTING:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbANCSettingCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbANCSettingCmdType *pInput = (AcdbANCSettingCmdType *)pCommandStruct;
         result = AcdbCmdSetAdieANCDataTable (pInput);
      }
      break;
   case ACDB_CMD_GET_LOOKUP_TABLE_SIZE:
      if (pCommandStruct == NULL || nCommandStructLength == 0 ||
          pResponseStruct == NULL || nResponseStructLength == 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGetTableSizeCmdType *pInput = (AcdbGetTableSizeCmdType *)pCommandStruct;
         AcdbGetTableSizeRspType *pOutput = (AcdbGetTableSizeRspType *)pResponseStruct;
         result = AcdbCmdGetLookupTableSize (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_TABLE_INDEX_COMBINATION:
      if (pCommandStruct == NULL || nCommandStructLength == 0 ||
          pResponseStruct == NULL || nResponseStructLength == 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbQueriedTblIndexCmdType *pInput = (AcdbQueriedTblIndexCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetTableLookupIndex (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID:
      if (pCommandStruct == NULL || nCommandStructLength == 0 ||
          pResponseStruct == NULL || nResponseStructLength == 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGetAudProcTopIdCmdType *pInput = (AcdbGetAudProcTopIdCmdType *)pCommandStruct;
         AcdbGetTopologyIdRspType *pOutput = (AcdbGetTopologyIdRspType *)pResponseStruct;
         result = AcdbCmdGetAudProcCmnTopId (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID:
      if (pCommandStruct == NULL || nCommandStructLength == 0 ||
          pResponseStruct == NULL || nResponseStructLength == 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGetAudProcStrmTopIdCmdType *pInput = (AcdbGetAudProcStrmTopIdCmdType *)pCommandStruct;
         AcdbGetTopologyIdRspType *pOutput = (AcdbGetTopologyIdRspType *)pResponseStruct;
         result = AcdbCmdGetAudProcStrmTopId (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID_LIST:
      if (pCommandStruct == NULL || nCommandStructLength == 0 ||
          pResponseStruct == NULL || nResponseStructLength == 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGeneralInfoCmdType *pInput = (AcdbGeneralInfoCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudProcCmnTopIdList (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST:
      if (pCommandStruct == NULL || nCommandStructLength == 0 ||
          pResponseStruct == NULL || nResponseStructLength == 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGeneralInfoCmdType *pInput = (AcdbGeneralInfoCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAudProcStrmTopIdList (pInput, pOutput);
      }
      break;
   case ACDB_CMD_GET_AFE_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAfeDataCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAfeDataCmdType *pInput = (AcdbAfeDataCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAfeData (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_AFE_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAfeDataCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAfeDataCmdType *pInput = (AcdbAfeDataCmdType *)pCommandStruct;
         result = AcdbCmdSetAfeData (pInput);
      }
      break;
   case ACDB_CMD_GET_GLBTBL_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof(AcdbGblTblCmdType) ||
          pResponseStruct == NULL || nResponseStructLength != sizeof(AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGblTblCmdType *pInput = (AcdbGblTblCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetGlobalTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_GLBTBL_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbGblTblCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGblTblCmdType *pInput = (AcdbGblTblCmdType *)pCommandStruct;
         result = AcdbCmdSetGlobalTable (pInput);
      }
      break;
   case ACDB_CMD_ESTIMATE_MEMORY_USE:
      if (pCommandStruct != NULL || nCommandStructLength != 0 ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbMemoryUsageType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbMemoryUsageType *pMemUse = (AcdbMemoryUsageType *)pResponseStruct;
         result = AcdbCmdGetMemoryUsage (pMemUse);
      }
      break;
   case ACDB_CMD_RESET:
      if (pCommandStruct != NULL || nCommandStructLength != 0 ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         result = AcdbCmdSystemReset();
      }
      break;
   case ACDB_CMD_GET_AFE_COMMON_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAfeCommonTableCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAfeCommonTableCmdType *pInput = (AcdbAfeCommonTableCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
		 result = AcdbCmdGetAfeCmnTable (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_AFE_COMMON_TABLE:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAfeCommonTableCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAfeCommonTableCmdType *pInput = (AcdbAfeCommonTableCmdType *)pCommandStruct;
		 result = AcdbCmdSetAfeCmnTable (pInput);
      }
      break;
   case ACDB_CMD_GET_AFE_COMMON_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAfeCmnDataCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAfeCmnDataCmdType *pInput = (AcdbAfeCmnDataCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
		 result = AcdbCmdGetAfeCmnData (pInput, pOutput);
      }
      break;
   case ACDB_CMD_SET_AFE_COMMON_DATA:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbAfeCmnDataCmdType) ||
         pResponseStruct != NULL || nResponseStructLength != 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbAfeCmnDataCmdType *pInput = (AcdbAfeCmnDataCmdType *)pCommandStruct;
		 result = AcdbCmdSetAfeCmnData (pInput);
      }
      break;
   case ACDB_CMD_GET_AFE_TOPOLOGY_LIST:
      if (pCommandStruct == NULL || nCommandStructLength != sizeof (AcdbGeneralInfoCmdType) ||
         pResponseStruct == NULL || nResponseStructLength != sizeof (AcdbQueryResponseType))
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbGeneralInfoCmdType *pInput = (AcdbGeneralInfoCmdType *)pCommandStruct;
         AcdbQueryResponseType *pOutput = (AcdbQueryResponseType *)pResponseStruct;
         result = AcdbCmdGetAfeTopIDList (pInput, pOutput);
      }
      break;
   default:
      result = ACDB_NOTSUPPORTED;
      break;
   }
   
   return result;
}
