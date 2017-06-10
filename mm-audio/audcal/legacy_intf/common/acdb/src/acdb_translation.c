/*===========================================================================
    FILE:           acdb_translation.c

    OVERVIEW:       This file contains the implementaion of the translation between 
                    data structure versioning changes.
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
#define DEFAULT_TARGET_VERSION          0xFFFFFFFF
#define DEVICE_INFO_TRANSLATE_SUCCESS   0x00000000
#define DEVICE_INFO_TRANSLATE_STOP      0x00000001
#define DEVICE_INFO_TRANSLATE_CONT      0x00000002

/* ---------------------------------------------------------------------------
 * Global Data Definitions
 *--------------------------------------------------------------------------- */
uint32_t nCurrentTargetVersionOnTarget = DEFAULT_TARGET_VERSION;

/* ---------------------------------------------------------------------------
 * Static Variable Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

//Translate sample rate bit flag mask to actual sample rate value
int32_t acdb_translate_sample_rate (const uint32_t nInput,
                                    uint32_t* pOutput
                                    )
{
   int32_t result = ACDB_PARMNOTFOUND;
   
   //default FS = 0, it is an unexpected value, FS can never equal to 0
   *pOutput = 0;

   if (nCurrentTargetVersionOnTarget == DEFAULT_TARGET_VERSION)
   {
      AcdbDataTargetVersionType tv;
      (void) AcdbDataIoctl (ACDBDATA_GET_TARGET_VERSION, NULL, 0, (uint8_t *)&tv, sizeof (tv));
      nCurrentTargetVersionOnTarget = tv.targetversion;
   }

   if (nCurrentTargetVersionOnTarget == 0x00011185)
   {
      //translate the Sample rate ID
      switch (nInput)
      {
         case 8000:
            *pOutput = ACDB_SAMPLERATE_8000Hz;
            break;
         case 11025:
            *pOutput = ACDB_SAMPLERATE_11025Hz;
            break;
         case 12000:
            *pOutput = ACDB_SAMPLERATE_12000Hz;
            break;
         case 16000:
            *pOutput = ACDB_SAMPLERATE_16000Hz;
            break;
         case 22050:
            *pOutput = ACDB_SAMPLERATE_22050Hz;
            break;
         case 24000:
            *pOutput = ACDB_SAMPLERATE_24000Hz;
            break;
         case 32000:
            *pOutput = ACDB_SAMPLERATE_32000Hz;
            break;
         case 44100:
            *pOutput = ACDB_SAMPLERATE_44100Hz;
            break;
         case 48000:
            *pOutput = ACDB_SAMPLERATE_48000Hz;
            break;
      }
   }
   else
   {
      switch (nInput)
      {
         //translate the Sample rate ID
         case ACDB_SAMPLERATE_8000Hz:
            *pOutput = 8000;
            break;
         case ACDB_SAMPLERATE_11025Hz:
            *pOutput = 11025;
            break;
         case ACDB_SAMPLERATE_12000Hz:
            *pOutput = 12000;
            break;
         case ACDB_SAMPLERATE_16000Hz:
            *pOutput = 16000;
            break;
         case ACDB_SAMPLERATE_22050Hz:
            *pOutput = 22050;
            break;
         case ACDB_SAMPLERATE_24000Hz:
            *pOutput = 24000;
            break;
         case ACDB_SAMPLERATE_32000Hz:
            *pOutput = 32000;
            break;
         case ACDB_SAMPLERATE_44100Hz:
            *pOutput = 44100;
            break;
         case ACDB_SAMPLERATE_48000Hz:
            *pOutput = 48000;
            break;
      }
   }

   if (*pOutput != 0)
   {
      result = ACDB_SUCCESS;
   }

   return result;
}

//Translate over sample rate enumerated ID to actual OSR value
int32_t acdb_translate_over_sample_rate (const uint32_t nInput,
                                         uint32_t *pOutput
                                         )
{
   int32_t result = ACDB_PARMNOTFOUND;
   //default OSR = 0, it is an unexpected value, OSR can never equal to 0
   *pOutput = 0;

   switch (nInput)
   {
      //translate the Sample rate ID
      case ACDB_OVERSAMPLERATE_64:
         *pOutput = 64;
         break;
      case ACDB_OVERSAMPLERATE_128:
         *pOutput = 128;
         break;
      case ACDB_OVERSAMPLERATE_256:
         *pOutput = 256;
         break;
   }

   if (*pOutput != 0)
   {
      result = ACDB_SUCCESS;
   }

   return result;
}

//Translate Adie codec path profile from latest version to a different version.
//Version of PIDs are defined in acdbh.h
int32_t acdb_adieprofile_translation (const uint32_t nParamId,
                                      const uint32_t nDefaultParamId,
                                      uint8_t *pBuffer,
                                      uint8_t *pDataPtr,
                                      const uint32_t nDataSize
                                      )
{
   int32_t result = ACDB_SUCCESS;

   if (pBuffer != NULL && pDataPtr != NULL && nDataSize != 0)
   {
      if (nDefaultParamId == ACDB_PID_ADIE_CODEC_PATH_TIMPANI_V2 &&
          nParamId == ACDB_PID_ADIE_CODEC_PATH_TIMPANI)
      {
         //Hard coded to translate from V2 to V1
         memcpy((void*)pBuffer,pDataPtr+2*sizeof(uint32_t),8*sizeof(uint32_t));
      }
      else
      {
         memcpy((void*)pBuffer,(void*)pDataPtr,nDataSize);
      }
   }
   else
   {
      result = ACDB_BADPARM;
      
      ACDB_DEBUG_LOG("[ACDB Translation]->AdieTranslation->NULL Input!\n");
   }

   return result;
}

//Translate device info from latest version to a different version.
//Version of PIDs is defined in acdb.h
static int32_t acdb_devinfo_check_size(uint32_t nBufferSize, uint32_t actual_size, 
                                       uint32_t *pRspSize)
{
   if(nBufferSize >= actual_size)
   {
      *pRspSize = actual_size;
      return ACDB_SUCCESS;
   }
   else
   {
      return ACDB_INSUFFICIENTMEMORY;
   }
}

int32_t acdb_deviceinfo_translation (const uint32_t nParamId,
                                     const uint32_t nDefaultParamId,
                                     uint8_t *pSrcData,
                                     uint8_t *pDstData,
                                     uint32_t nBufferSize,
                                     uint32_t *pRspSize
                                     )
{
   int32_t result = ACDB_SUCCESS;

   if (pDstData == NULL || pRspSize == NULL || !nBufferSize)
   {
      result = ACDB_BADPARM;
   }
   else if (pSrcData == NULL)
   {
      result = ACDB_BADSTATE;
   }
   else
   {
      //check the input buffer size
      switch (nParamId)
      {
      case ACDB_DEVICE_INFO_PARAM4:
         result = acdb_devinfo_check_size(nBufferSize,sizeof(AcdbDeviceInfo4Type),pRspSize);
         break;
      case ACDB_DEVICE_INFO_PARAM3:
         result = acdb_devinfo_check_size(nBufferSize,sizeof(AcdbDeviceInfo3Type),pRspSize);
         break;
      case ACDB_DEVICE_INFO_PARAM2:
         result = acdb_devinfo_check_size(nBufferSize,sizeof(AcdbDeviceInfo2Type),pRspSize);
         break;
      case ACDB_DEVICE_INFO_PARAM:
         result = acdb_devinfo_check_size(nBufferSize,sizeof(AcdbDeviceInfoType),pRspSize);
         break;
      default:
         result = ACDB_BADSTATE;
         break;
      }

      if(result == ACDB_SUCCESS && nParamId == nDefaultParamId)
      {
         memcpy((void*)pDstData,(void*)pSrcData,*pRspSize);
      }
      else
      {
         //default device info ParamID is not equal to client device info ParamID, translation is needed
         if (nDefaultParamId != ACDB_DEVICE_INFO_PARAM3 && nDefaultParamId != ACDB_DEVICE_INFO_PARAM2 
            && nDefaultParamId != ACDB_DEVICE_INFO_PARAM)
         {
            AcdbDeviceInfo3Type nDevInfoRef;
            int32_t localresult = DEVICE_INFO_TRANSLATE_CONT;

            localresult = acdb_deviceinfo_translationLev1 ((uint32_t) ACDB_DEVICE_INFO_PARAM3,
                                                           (uint32_t) nDefaultParamId,
                                                           (uint8_t*) pSrcData,
                                                           (uint8_t*) &nDevInfoRef,
                                                           (uint32_t) sizeof(AcdbDeviceInfo3Type));
            //check if algorithm should continuous translate
            if (nParamId == ACDB_DEVICE_INFO_PARAM3)
            {
               memcpy((void*)pDstData,(void*)&nDevInfoRef,*pRspSize);
               localresult = DEVICE_INFO_TRANSLATE_SUCCESS;
            }
            //continous translate if indicates continous
            if (localresult == DEVICE_INFO_TRANSLATE_CONT)
            {            
               result = acdb_deviceinfo_translationLev2 ((uint32_t) nParamId,
                                                         (uint32_t) ACDB_DEVICE_INFO_PARAM3,
                                                         (uint8_t*) &nDevInfoRef,
                                                         (uint8_t*) pDstData,
                                                         (uint32_t) nBufferSize,
                                                         (uint32_t*) pRspSize);
            }
            else if (localresult == DEVICE_INFO_TRANSLATE_STOP)
            {
               result = ACDB_BADSTATE;
            }
         }
         else
         {
            result = acdb_deviceinfo_translationLev2 ((uint32_t) nParamId,
                                                      (uint32_t) nDefaultParamId,
                                                      (uint8_t*) pSrcData,
                                                      (uint8_t*) pDstData,
                                                      (uint32_t) nBufferSize,
                                                      (uint32_t*) pRspSize);
         }
      }
   }

   return result;
}

int32_t acdb_deviceinfo_translationLev1 (const uint32_t nParamId,
                                         const uint32_t nDefaultParamId,
                                         uint8_t *pSrcData,
                                         uint8_t *pDstData,
                                         uint32_t nBufferSize
                                         )
{
   int32_t result = DEVICE_INFO_TRANSLATE_CONT;

   if (pDstData == NULL || pSrcData == NULL || !nBufferSize)
   {
      result = DEVICE_INFO_TRANSLATE_STOP;
   }
   else
   {
         if (nParamId == ACDB_DEVICE_INFO_PARAM3 && nDefaultParamId == ACDB_DEVICE_INFO_PARAM4)
         {
            memcpy((void*)pDstData,(void*)pSrcData, nBufferSize);
            result = DEVICE_INFO_TRANSLATE_CONT;
         }
         else
         {//stop the translation
            result = DEVICE_INFO_TRANSLATE_STOP;
         }
   }

   return result;
}

int32_t acdb_deviceinfo_translationLev2 (const uint32_t nParamId,
                                         const uint32_t nDefaultParamId,
                                         uint8_t *pSrcData,
                                         uint8_t *pDstData,
                                         uint32_t nBufferSize,
                                         uint32_t *pRspSize
                                         )
{
   int32_t result = ACDB_SUCCESS;

   AcdbDeviceInfo3Type *pInputV3 = NULL;
   AcdbDeviceInfo2Type *pInputV2 = NULL;

   if (pDstData == NULL || !nBufferSize)
   {
      result = ACDB_BADPARM;
   }
   else if (pSrcData == NULL || pRspSize == NULL)
   {
      result = ACDB_BADSTATE;
   }
   else
   {
      switch (nDefaultParamId)
      {
      case ACDB_DEVICE_INFO_PARAM3:
         pInputV3 = (AcdbDeviceInfo3Type *) pSrcData;
         break;
      case ACDB_DEVICE_INFO_PARAM2:
         pInputV2 = (AcdbDeviceInfo2Type *) pSrcData;
         break;
      default:
         result = ACDB_BADSTATE;
         break;
      }

      if(result == ACDB_SUCCESS && nParamId == nDefaultParamId)
      {
         memcpy((void*)pDstData,(void*)pSrcData,*pRspSize);
      }
      else
      {
         if (nParamId == ACDB_DEVICE_INFO_PARAM && nDefaultParamId == ACDB_DEVICE_INFO_PARAM2 && pInputV2 != NULL)
         {
            //translate from v2 version to v1 version
            ((AcdbDeviceInfoType*) (pDstData))->ulAfeBytesPerSampleMask = pInputV2->ulAfeBytesPerSampleMask;
            ((AcdbDeviceInfoType*) (pDstData))->ulAfeChannelMode = pInputV2->ulAfeChannelMode;
            ((AcdbDeviceInfoType*) (pDstData))->ulAfePortConfig = pInputV2->ulAfePortConfig;
            ((AcdbDeviceInfoType*) (pDstData))->ulAfePortId = pInputV2->ulAfePortId;
            ((AcdbDeviceInfoType*) (pDstData))->ulAfeSyncSrcSelect = pInputV2->ulAfeSyncSrcSelect;
            ((AcdbDeviceInfoType*) (pDstData))->ulAudioTopologyId = pInputV2->ulAudioTopologyId;
            ((AcdbDeviceInfoType*) (pDstData))->ulChannelConfig = pInputV2->ulChannelConfig;
            ((AcdbDeviceInfoType*) (pDstData))->ulCodecPathId = pInputV2->ulCodecPathId;
            ((AcdbDeviceInfoType*) (pDstData))->ulDirectionMask = pInputV2->ulDirectionMask;
            ((AcdbDeviceInfoType*) (pDstData))->ulI2sMClkMode = pInputV2->ulI2sMClkMode;
            ((AcdbDeviceInfoType*) (pDstData))->ulOverSamplerateId = pInputV2->ulOverSamplerateId;
            ((AcdbDeviceInfoType*) (pDstData))->ulSampleRateMask = pInputV2->ulSampleRateMask;
            ((AcdbDeviceInfoType*) (pDstData))->ulVoiceTopologyId = pInputV2->ulVoiceTopologyId;
            *pRspSize =sizeof(AcdbDeviceInfoType);
         }
         else if (nParamId == ACDB_DEVICE_INFO_PARAM && nDefaultParamId == ACDB_DEVICE_INFO_PARAM3 && pInputV3 != NULL)
         {
            //translate from v3 version to v1 version
            ((AcdbDeviceInfoType*) (pDstData))->ulAfeBytesPerSampleMask = pInputV3->ulAfeBytesPerSampleMask;
            ((AcdbDeviceInfoType*) (pDstData))->ulAfeChannelMode = pInputV3->ulAfeChannelMode;
            ((AcdbDeviceInfoType*) (pDstData))->ulAfePortConfig = pInputV3->ulAfePortConfig;
            ((AcdbDeviceInfoType*) (pDstData))->ulAfePortId = pInputV3->ulAfePortId;
            ((AcdbDeviceInfoType*) (pDstData))->ulAfeSyncSrcSelect = pInputV3->ulAfeSyncSrcSelect;
            ((AcdbDeviceInfoType*) (pDstData))->ulAudioTopologyId = 0x00000000;     //Hard coded
            ((AcdbDeviceInfoType*) (pDstData))->ulChannelConfig = pInputV3->ulChannelConfig;
            ((AcdbDeviceInfoType*) (pDstData))->ulCodecPathId = pInputV3->ulCodecPathId;
            ((AcdbDeviceInfoType*) (pDstData))->ulDirectionMask = pInputV3->ulDirectionMask;
            ((AcdbDeviceInfoType*) (pDstData))->ulI2sMClkMode = pInputV3->ulI2sMClkMode;
            ((AcdbDeviceInfoType*) (pDstData))->ulOverSamplerateId = pInputV3->ulOverSamplerateId;
            ((AcdbDeviceInfoType*) (pDstData))->ulSampleRateMask = pInputV3->ulSampleRateMask;
            ((AcdbDeviceInfoType*) (pDstData))->ulVoiceTopologyId = pInputV3->ulVoiceTopologyId;
            *pRspSize =sizeof(AcdbDeviceInfoType);
         }
         else if (nParamId == ACDB_DEVICE_INFO_PARAM2 && nDefaultParamId == ACDB_DEVICE_INFO_PARAM3 && pInputV3 != NULL)
         {
            //translate from v3 version to v2 version
            ((AcdbDeviceInfo2Type*) (pDstData))->ulAfeBytesPerSampleMask = pInputV3->ulAfeBytesPerSampleMask;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulAfeChannelMode = pInputV3->ulAfeChannelMode;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulAfePortConfig = pInputV3->ulAfePortConfig;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulAfePortId = pInputV3->ulAfePortId;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulAfeSyncSrcSelect = pInputV3->ulAfeSyncSrcSelect;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulAudioTopologyId = 0x00000000;     //Hard coded
            ((AcdbDeviceInfo2Type*) (pDstData))->ulChannelConfig = pInputV3->ulChannelConfig;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulCodecPathId = pInputV3->ulCodecPathId;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulDirectionMask = pInputV3->ulDirectionMask;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulI2sMClkMode = pInputV3->ulI2sMClkMode;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulOverSamplerateId = pInputV3->ulOverSamplerateId;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulSampleRateMask = pInputV3->ulSampleRateMask;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulVoiceTopologyId = pInputV3->ulVoiceTopologyId;
            ((AcdbDeviceInfo2Type*) (pDstData))->ulCodecPathId2 = pInputV3->ulCodecPathId2;
            *pRspSize =sizeof(AcdbDeviceInfo2Type);
         }
         else
         {
            result = ACDB_BADPARM;
         }
      }
   }

   return result;
}

//This function is to help to map command PID to default data PID
int32_t acdb_map_command_PID (const uint32_t nCmdPID,
                              uint32_t *pDataPID
                              )
{
   int32_t result = ACDB_SUCCESS;

   if (pDataPID == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      switch (nCmdPID)
      {
      case ACDB_PID_VOCPROC_COMMON_TABLE:         
         *pDataPID= ACDBDATA_PID_VOCPROC_COMMON_TABLE;
         break;
      case ACDB_PID_VOCPROC_STREAM_TABLE:
         *pDataPID = ACDBDATA_PID_VOCPROC_STREAM_TABLE;
         break;
      case ACDB_PID_VOCPROC_VOLUME_TABLE:
         *pDataPID = ACDBDATA_PID_VOCPROC_VOLUME_TABLE;
         break;
      case ACDB_PID_AUDPROC_COMMON_TABLE:
         *pDataPID = ACDBDATA_PID_AUDPROC_COMMON_TABLE;
         break;
      case ACDB_PID_AUDPROC_STREAM_TABLE:
         *pDataPID = ACDBDATA_PID_AUDPROC_STREAM_TABLE;
         break;
      case ACDB_PID_AUDPROC_VOLUME_TABLE:
         *pDataPID = ACDBDATA_PID_AUDPROC_VOLUME_TABLE;
         break;
      case ACDB_PID_ADIE_PROFILE_TABLE:
         *pDataPID = ACDBDATA_PID_ADIE_PROFILE_TABLE;
         break;
      case ACDB_PID_ANC_CONFIG_DATA_TABLE:
         *pDataPID = ACDBDATA_PID_ANC_CONFIG_DATA_TABLE;
         break;
      case ACDB_PID_AFE_COMMON_TABLE:
         *pDataPID = ACDBDATA_PID_AFE_COMMON_TABLE;
         break;
      default:
         result = ACDB_BADPARM;
         break;
      }
   }

   return result;
}

//This function is to validate if data length to copy % table size = 0
// 0 == invalid, 1 == valid
int32_t acdb_validate_data_to_copy (const uint32_t nCmdPID,
                                    const uint32_t nDataLenToCopy,
                                    uint32_t *pRsp
                                    )
{
   int32_t result = ACDB_SUCCESS;

   if (pRsp == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      *pRsp = 0;

      switch (nCmdPID)
      {
      case ACDB_PID_VOCPROC_COMMON_TABLE:
         if (nDataLenToCopy%sizeof(AcdbDataVocProcCmnLookupKeyType) == 0)
         {           
             *pRsp = 1;
         }
         break;
      case ACDB_PID_VOCPROC_STREAM_TABLE:
         if (nDataLenToCopy%sizeof(AcdbDataVocProcStrmLookupKeyType) == 0)
         {
            *pRsp = 1;
         }
         break;
      case ACDB_PID_AUDPROC_COMMON_TABLE:
         if (nDataLenToCopy%sizeof(AcdbDataAudProcCmnLookupKeyType) == 0)
         {
            *pRsp = 1;
         }
         break;
      case ACDB_PID_AUDPROC_STREAM_TABLE:
         if (nDataLenToCopy%sizeof(AcdbDataAudProcStrmLookupKeyType) == 0)
         {
            *pRsp = 1;
         }
         break;
      case ACDB_PID_ADIE_PROFILE_TABLE:
         if (nDataLenToCopy%sizeof(AcdbDataAdieProfileLookupKeyType) == 0)
         {
            *pRsp = 1;
         }
         break;
      case ACDB_PID_ANC_CONFIG_DATA_TABLE:
         if (nDataLenToCopy%sizeof(AcdbDataAdieANCConfigDataLookupKeyType) == 0)
         {
            *pRsp = 1;
         }
         break;
      case ACDB_PID_AFE_COMMON_TABLE:
		 if (nDataLenToCopy%sizeof(AcdbDataAfeCmnLookupKeyType) == 0)
         {
            *pRsp = 1;
         }
         break;
      default:
         result = ACDB_BADPARM;
         break;
      }
   }

   return result;
}

//This function is to validate if PID version
int32_t acdb_validate_PID (uint32_t cmdPID,
                           uint32_t* pParamId,
                           uint32_t nDefaultParamId)
{
   int32_t result = ACDB_SUCCESS;

   if (pParamId == NULL)
   {
      result = ACDB_BADSTATE;
   }
   else
   {
      switch (cmdPID)
      {
      case ACDB_PID_ADIE_PROFILE_TABLE:
         if (ACDB_PID_ADIE_CODEC_PATH_TIMPANI == *pParamId ||
             nDefaultParamId == *pParamId)
         {
            //do nothing
         }
         else
         {
            result = ACDB_BADPARM;
         }
         break;
      case ACDB_PID_ANC_CONFIG_DATA_TABLE:
         if (nDefaultParamId == *pParamId)
         {
            //do nothing
         }
         else
         {
            result = ACDB_BADPARM;
         }
         break;
      default:
         result = ACDB_BADPARM;
         break;
      }
   }

   return result;
}