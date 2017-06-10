/*===========================================================================
    FILE:           acdb_translation.c

    OVERVIEW:       This file contains the implementaion of the translation between 
                    data structure versioning changes.
    DEPENDENCIES:   None

                    Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal2/common/8k_PL/acdb/rel/4.2/src/acdb_translation.c#1 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2010-09-20  ernanl  Initial implementation of the APIs and 
                        associated helper methods.
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
         memcpy((void*)pBuffer,(void*)pDataPtr,nDataSize);
      }
   else
   {
      result = ACDB_BADPARM;
      
      ACDB_DEBUG_LOG("[ACDB Translation]->AdieTranslation->NULL Input!\n");
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
      case ACDB_PID_ADIE_SIDETONE_TABLE:
          *pDataPID = ACDBDATA_PID_ADIE_SIDETONE_TABLE;
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
      case ACDB_PID_ADIE_SIDETONE_TABLE:
          if (nDataLenToCopy%sizeof(AcdbDataAdieSidetoneLookupKeyType) == 0)
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
         if (nDefaultParamId == *pParamId)
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

int32_t acdb_devinfo_getSampleMaskOffset (uint32_t nDeviceType)
{
   int32_t offset= 0; //0 byes copied

   switch (nDeviceType)
   {
   case ACDB_DEVICE_TYPE_I2S_PARAM:
      offset = 11*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_PCM_PARAM:
      offset = 10*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_SLIMBUS_PARAM:
      offset = 4*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_DIGITAL_MIC_PARAM:
      offset = 7*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_HDMI_PARAM:
      offset = 6*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_VIRTUAL_PARAM:
      offset = 5*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_INT_BTFM_PARAM:
      offset = 4*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_RT_PROXY_PARAM:
      offset = sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_SIMPLE_PARAM:
      offset = sizeof(uint32_t);
      break;
   default:
      offset = 0;
      break;
   }
   return offset;
}

int32_t acdb_devinfo_getBytesPerSampleMaskOffset (uint32_t nDeviceType)
{
   int32_t offset= 0; //0 byes copied

   switch (nDeviceType)
   {
   case ACDB_DEVICE_TYPE_I2S_PARAM:
      offset = 7*sizeof(uint32_t);         
      break;
   case ACDB_DEVICE_TYPE_PCM_PARAM:
      offset = 8*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_SLIMBUS_PARAM:
      offset = 2*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_DIGITAL_MIC_PARAM:
      offset = 3*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_HDMI_PARAM:
      offset = 4*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_VIRTUAL_PARAM:
      offset = 2*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_INT_BTFM_PARAM:
      offset = 2*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_RT_PROXY_PARAM:
      offset = 5*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_SIMPLE_PARAM:
      offset = 4*sizeof(uint32_t);
      break;
   default:
      offset = 0;
      break;
   }
   return offset;
}
