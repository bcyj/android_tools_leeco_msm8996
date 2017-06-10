/*===========================================================================
    FILE:           acdb_init.c

    OVERVIEW:       This file determines the logic to initialize the ACDB.

    DEPENDENCIES:   None

                    Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_includes.h"
#include "acdb.h"
#include "acdb_init.h"
#include "acdb_init_utility.h"
#include "acdb_parser.h"
#include "acdb_command.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/**
   @def ACDB_INIT_SUPPORT_ACDB_OVERRIDE_FROM_EFS
   @brief This preprocessor controls whether ACDB will support the
          potential override of ACDBialization from EFS.

   This preprocessor is used to encapsulate the code to read the ACDB file
   from EFS. This preprocessor controls two behaviors:
     1) If defined, acdb_init () will attempt to read the ACDB file from
        EFS. And if any failures occur, acdb_init () will then initialize
        ACDB from the statically linked default ACDB data.
     2) If not defined, acdb_init () will only initialize ACDB from the
        statically linked default ACDB data.
 */
#define ACDB_INIT_SUPPORT_ACDB_OVERRIDE_FROM_EFS

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

typedef struct _AcdbClientContext {
   AcdbInitFileContext *fc;

   uint32_t targetversion;
} AcdbClientContext;

/* ---------------------------------------------------------------------------
 * Global Data Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Variable Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

uint32_t acdbParseCallback (
   AcdbParseCmdType  cmd,
   void              *handle,
   void              *status_data)
{
   uint32_t result = 0;
   int32_t cmd_result = 0;
   AcdbClientContext* pClientContext = (AcdbClientContext*) handle;

   switch (cmd)
   {
   case ACDB_DEVICE_INFORMATION:
      break;
   case ACDB_FILE_VERSION:
      break;
   case ACDB_FAIL:
      break;
   case ACDB_GET_FILE_SIZE:
      {
         AcdbParseGetFileSizeType* pData = (AcdbParseGetFileSizeType*)status_data;
         AcdbInitGetFileSize (pClientContext->fc, &pData->size);
      }
      break;
   case ACDB_GET_DATA:
      {
         AcdbParseGetDataType* pData = (AcdbParseGetDataType*)status_data;
         result = AcdbInitFileRead (pClientContext->fc,
                                    pData->pBuffer,
                                    pData->bytesToRead,
                                    &pData->bytesRead);
      }
      break;
   case ACDB_TABLE_DEFINITION:
      {
         AcdbParseTableDefinitionType* pData = (AcdbParseTableDefinitionType*)status_data;
      }
      break;
   case ACDB_TARGET_VERSION:
      {
         AcdbTargetVersionType tv;

         AcdbParseTargetVersionType* pData = (AcdbParseTargetVersionType*)status_data;
         pClientContext->targetversion = pData->targetversion;

         //Query ACDB Target Version         
         result = acdb_ioctl (ACDB_CMD_GET_TARGET_VERSION, NULL, 0, (uint8_t*)&tv, sizeof (tv));
         
         if (result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Unable to query for ACDB Target Version. Error is [%08X].\n", result);
         }
         else
         {
            if (tv.targetversion != pData->targetversion)
            {
               ACDB_DEBUG_LOG("[ACDB Init]->Warning: Target Version Mismatch between Data and Command\n");
               result = ACDB_MISMATCH_TARGET_VERSION;
            }
         }
      }
      break;
   case ACDB_OEM_INFORMATION:
      {         
         AcdbParseOemInformationType* pData = (AcdbParseOemInformationType*)status_data;
         AcdbGeneralInfoCmdType cmdOEMI;

         cmdOEMI.nBufferPointer = pData->pOemInformation;
         cmdOEMI.nBufferLength = pData->oemInformationLength;

         cmd_result = AcdbCmdSetOEMInfo (&cmdOEMI);

         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set OEM Information failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_DATE_MODIFIED:
      {
         AcdbParseDateModifiedType* pData = (AcdbParseDateModifiedType*)status_data;
         AcdbGeneralInfoCmdType cmdDateMod;

         cmdDateMod.nBufferPointer = pData->pDateString;
         cmdDateMod.nBufferLength = pData->dateStringLength;

         cmd_result = AcdbCmdSetDateInfo (&cmdDateMod);

         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set Date Information failed [%08X]\n", cmd_result);
         }
      }
      break;         
   case ACDB_VOC_PROC_CMN_DATA:
      {
         AcdbParseVocProcCmnDataType* pData = (AcdbParseVocProcCmnDataType*)status_data;
         AcdbVocProcCmdType cmdVP;

         cmdVP.nTxDeviceId = pData->nTxDeviceId;
         cmdVP.nRxDeviceId = pData->nRxDeviceId;
         cmdVP.nTxDeviceSampleRateId = pData->nTxDeviceSampleRateId;
         cmdVP.nRxDeviceSampleRateId = pData->nRxDeviceSampleRateId;
         cmdVP.nNetworkId = pData->nNetworkId;
         cmdVP.nVocProcSampleRateId = pData->nVocProcSampleRateId;
         cmdVP.nModuleId = pData->nModuleId;
         cmdVP.nParamId = pData->nParamId;
         cmdVP.nBufferLength = pData->nBufferSize;
         cmdVP.nBufferPointer = pData->pBuffer;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_VOCPROC_COMMON_DATA,
                                 (uint8_t*)&cmdVP, sizeof (cmdVP),
                                 NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set VocProcCmn Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_VOC_PROC_STRM_DATA:
      {
         AcdbParseVocProcStreamDataType* pData = (AcdbParseVocProcStreamDataType*)status_data;
         AcdbVocStrmCmdType cmdVS;

         cmdVS.nNetworkId = pData->nNetworkId;
         cmdVS.nVocProcSampleRateId = pData->nVocProcSampleRateId;
         cmdVS.nModuleId = pData->nModuleId;
         cmdVS.nParamId = pData->nParamId;
         cmdVS.nBufferLength = pData->nBufferSize;
         cmdVS.nBufferPointer = pData->pBuffer;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_VOCPROC_STREAM_DATA,
                                 (uint8_t*)&cmdVS, sizeof (cmdVS),
                                 NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set VocProcStrm Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_VOC_PROC_VOL_DATA:
      {
         AcdbParseVocProcVolDataType* pData = (AcdbParseVocProcVolDataType*)status_data;
         AcdbVocProcVolTblDataCmdType cmdVPVT;

         cmdVPVT.nTxDeviceId = pData->nTxDeviceId;
         cmdVPVT.nRxDeviceId = pData->nRxDeviceId;
         cmdVPVT.nNetworkId = pData->nNetworkId;
         cmdVPVT.nVocProcSampleRateId = pData->nVocProcSampleRateId;
         cmdVPVT.nVolumeIndex = pData->nVolumeIndex;
         cmdVPVT.nModuleId = pData->nModuleId;
         cmdVPVT.nParamId = pData->nParamId;
         cmdVPVT.nBufferLength = pData->nBufferSize;
         cmdVPVT.nBufferPointer = pData->pBuffer;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_VOCPROC_GAIN_DEP_VOLTBL_STEP_DATA,
                                 (uint8_t*)&cmdVPVT, sizeof (cmdVPVT),
                                 NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set VocProcVol Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_AUD_PROC_CMN_DATA:
      {
         AcdbParseAudProcCmnDataType* pData = (AcdbParseAudProcCmnDataType*)status_data;
         AcdbAudProcCmdType cmdAP;

         cmdAP.nDeviceId = pData->nDeviceId;
         cmdAP.nDeviceSampleRateId = pData->nDeviceSampleRateId;
         cmdAP.nApplicationType = pData->nApplicationTypeId;
         cmdAP.nModuleId = pData->nModuleId;
         cmdAP.nParamId = pData->nParamId;
         cmdAP.nBufferLength = pData->nBufferSize;
         cmdAP.nBufferPointer = pData->pBuffer;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_AUDPROC_COMMON_DATA,
                                 (uint8_t*)&cmdAP, sizeof (cmdAP),
                                 NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set AudProcCmn Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_AUD_PROC_STRM_DATA:
      {
         AcdbParseAudProcStreamDataType* pData = (AcdbParseAudProcStreamDataType*)status_data;
         AcdbAudStrmCmdType cmdAS;

         cmdAS.nDeviceId = pData->nDeviceId;
         cmdAS.nApplicationTypeId = pData->nApplicationTypeId;
         cmdAS.nModuleId = pData->nModuleId;
         cmdAS.nParamId = pData->nParamId;
         cmdAS.nBufferLength = pData->nBufferSize;
         cmdAS.nBufferPointer = pData->pBuffer;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_AUDPROC_STREAM_DATA,
                                 (uint8_t*)&cmdAS, sizeof (cmdAS),
                                 NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set AudProcStrm Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_AUD_PROC_VOL_DATA:
      {
         AcdbParseAudProcVolDataType* pData = (AcdbParseAudProcVolDataType*)status_data;
         AcdbAudProcVolTblDataCmdType cmdAPVT;

         cmdAPVT.nDeviceId = pData->nDeviceId;
         cmdAPVT.nApplicationType = pData->nApplicationTypeId;
         cmdAPVT.nVolumeIndex = pData->nVolumeIndex;
         cmdAPVT.nModuleId = pData->nModuleId;
         cmdAPVT.nParamId = pData->nParamId;
         cmdAPVT.nBufferLength = pData->nBufferSize;
         cmdAPVT.nBufferPointer = pData->pBuffer;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_AUDPROC_GAIN_DEP_VOLTBL_STEP_DATA,
                                  (uint8_t*)&cmdAPVT, sizeof (cmdAPVT),
                                  NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set AudProcVol Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_ADIE_PROFILE_DATA:
      {
         AcdbParseAdieProfileDataType* pData = (AcdbParseAdieProfileDataType*)status_data;
         AcdbAdiePathProfileCmdType cmdADPT;

         cmdADPT.ulCodecPathId = pData->nCodecPathId;
         cmdADPT.nOversamplerateId = pData->nOverSampleRateId;
         cmdADPT.nFrequencyId = pData->nFrequencyId;
         cmdADPT.nParamId = pData->nParamId;
         cmdADPT.nBufferPointer = pData->pBuffer;
         cmdADPT.nBufferLength = pData->nBufferSize;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_ADIE_CODEC_PATH_PROFILE,
                                  (uint8_t*)&cmdADPT, sizeof (cmdADPT),
                                  NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set Adie Codec Path Profile Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_ADIE_ANC_CONFIG_DATA:
      {
         AcdbParseAdieANCDataType* pData = (AcdbParseAdieANCDataType*)status_data;
         AcdbANCSettingCmdType cmdADAT;

         cmdADAT.nRxDeviceId = pData->nDeviceId;
         cmdADAT.nOversamplerateId = pData->nOverSampleRateId;
         cmdADAT.nFrequencyId = pData->nFrequencyId;
         cmdADAT.nParamId = pData->nParamId;
         cmdADAT.nBufferPointer = pData->pBuffer;
         cmdADAT.nBufferLength = pData->nBufferSize;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_ANC_SETTING,
                                  (uint8_t*)&cmdADAT, sizeof (cmdADAT),
                                  NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set Adie ANC Config Data Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_AFE_DATA:
      {
         AcdbParseAfeDataType* pData = (AcdbParseAfeDataType*)status_data;
         AcdbAfeDataCmdType cmdAFE;

         cmdAFE.nTxDeviceId = pData->nTxDeviceId;
         cmdAFE.nRxDeviceId = pData->nRxDeviceId;
         cmdAFE.nModuleId = pData->nModuleId;
         cmdAFE.nParamId = pData->nParamId;
         cmdAFE.nBufferLength = pData->nBufferSize;
         cmdAFE.nBufferPointer = pData->pBuffer;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_AFE_DATA,
                                 (uint8_t*)&cmdAFE, sizeof (cmdAFE),
                                 NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set AFE Failed [%08X]\n", cmd_result);
         }
      }
      break;
   case ACDB_GLBTBL_DATA:
      {
         AcdbParseGlbTblDataType* pData = (AcdbParseGlbTblDataType*) status_data;
         AcdbGblTblCmdType cmdGBTbl;

         cmdGBTbl.nModuleId = pData->nModuleId;
         cmdGBTbl.nParamId = pData->nParamId;
         cmdGBTbl.nBufferPointer = pData->pBuffer;
         cmdGBTbl.nBufferLength = pData->nBufferSize;

         cmd_result = acdb_ioctl (ACDB_CMD_SET_GLBTBL_DATA,
                                  (uint8_t*)&cmdGBTbl, sizeof(cmdGBTbl),
                                  NULL, 0);
         if (cmd_result != ACDB_SUCCESS)
         {
            ACDB_DEBUG_LOG ("[ACDB Init]->Set Generic Table Failed [%d]\n", cmd_result);
         }
      }
      break;
	  case ACDB_AFE_CMN_DATA:
		 {
			AcdbParseAfeCmnDataType* pData = (AcdbParseAfeCmnDataType*)status_data;
			AcdbAfeCmnDataCmdType cmdAFE;
	  
			cmdAFE.nDeviceId = pData->nDeviceId;
			cmdAFE.nAfeSampleRateId = pData->nAfeSampleRateId;
			cmdAFE.nModuleId = pData->nModuleId;
			cmdAFE.nParamId = pData->nParamId;
			cmdAFE.nBufferLength = pData->nBufferSize;
			cmdAFE.nBufferPointer = pData->pBuffer;
	  
			cmd_result = acdb_ioctl (ACDB_CMD_SET_AFE_COMMON_DATA,
									(uint8_t*)&cmdAFE, sizeof (cmdAFE),
									NULL, 0);
			if (cmd_result != ACDB_SUCCESS)
			{
			   ACDB_DEBUG_LOG ("[ACDB Init]->Set AFECmn Failed [%08X]\n", cmd_result);
			}
		 }
		 break;
	  
   }
   return result;
}

static int32_t DoesAcdbFileExistOnFileSystem ()
{
   int32_t result = ACDB_INIT_FAILURE;
   const char *pFilename = NULL;
   AcdbInitFileContext* pContext = NULL;

   (void) AcdbInitGetAcdbDefaultFilename (&pFilename);
   result = AcdbInitFileOpen (pFilename, &pContext);
   if (result == ACDB_INIT_SUCCESS)
   {
      result = AcdbInitFileClose (pContext);
      pContext = NULL;
   }

   return result;
}

static int32_t IsAcdbFromFileSystemCompatible ()
{
   int32_t result = ACDB_INIT_FAILURE;
   const char *pFilename = NULL;
   AcdbClientContext context;
   AcdbTargetVersionType tv;

   memset (&tv, 0, sizeof (tv));
   memset (&context, 0, sizeof (context));

   (void) AcdbInitGetAcdbDefaultFilename (&pFilename);

   result = AcdbInitFileOpen (pFilename, &context.fc);
   if (result == ACDB_INIT_SUCCESS)
   {
      result = AcdbParseTargetVersion (&context, acdbParseCallback);
      (void) AcdbInitFileClose (context.fc);

      (void) acdb_ioctl (ACDB_CMD_GET_TARGET_VERSION, NULL, 0,  (uint8_t*)&tv, sizeof (tv));

      if (context.targetversion == tv.targetversion)
      {
         result = ACDB_INIT_SUCCESS;
      }
      else
      {
         result = ACDB_INIT_FAILURE;
      }
   }

   return result;
}

static int32_t InitializeAcdbFromFileSystem ()
{
   int32_t result = ACDB_INIT_FAILURE;
   const char *pFilename = NULL;
   AcdbClientContext context;

   memset (&context, 0, sizeof (context));

   (void) AcdbInitGetAcdbDefaultFilename (&pFilename);
   result = AcdbInitFileOpen (pFilename, &context.fc);
   if (result == ACDB_INIT_SUCCESS)
   {
      result = AcdbParse (&context, acdbParseCallback);
      (void) AcdbInitFileClose (context.fc);
   }

   return result;
}

static int32_t RenameFileToAlertUser ()
{
   int32_t result = ACDB_INIT_FAILURE;

   const char *pOldFilename = NULL, *pNewFilename = NULL;

   (void) AcdbInitGetAcdbDefaultFilename (&pOldFilename);
   (void) AcdbInitGetAcdbDefaultErrorFilename (&pNewFilename);
   result =  AcdbInitRenameFile (pOldFilename, pNewFilename);

   return result;
}

static int32_t CreateDefaultAcdbDirectoryOnFileSystem ()
{
   int32_t result = ACDB_INIT_FAILURE;
   result = AcdbInitCreatePath ();
   return result;
}

/* ---------------------------------------------------------------------------
 * Externalized Function Definitions
 *--------------------------------------------------------------------------- */

int32_t acdb_init ()
{
   int32_t result = ACDB_INIT_SUCCESS;

#ifdef ACDB_INIT_SUPPORT_ACDB_OVERRIDE_FROM_EFS
   result = DoesAcdbFileExistOnFileSystem ();
   if (result == ACDB_INIT_SUCCESS)
   {
      //Only print this msg on target
      ACDB_DEBUG_LOG("[ACDB Init]->File is found on the system\n");
      
      result = IsAcdbFromFileSystemCompatible ();
      if (result == ACDB_INIT_SUCCESS)
      {
         //Only print this msg on target
         ACDB_DEBUG_LOG("[ACDB Init]->File is compatible with the system\n");
         
         result = InitializeAcdbFromFileSystem ();
      }
      else
      {
         //print this msg regarless on target or win32
         ACDB_DEBUG_LOG("[ACDB Init]->File is not compatible with the system!\n");
         
         (void) RenameFileToAlertUser ();
         result = ACDB_INIT_FAILURE;
      }

      // If there was a failure when trying to initialize from the file,
      // for safety concerns, let's reset the acdb.
      if (result != ACDB_INIT_SUCCESS)
      {
         ACDB_DEBUG_LOG("[ACDB Init] failed to initialize, reset the system\n");
         (void) acdb_ioctl (ACDB_CMD_RESET, NULL, 0, NULL, 0);
      }
   }
   else
   {
      ACDB_DEBUG_LOG("[ACDB Init]->File is not found on the system\n");

      (void) CreateDefaultAcdbDirectoryOnFileSystem ();
      result = ACDB_INIT_SUCCESS;
   }
#endif //* ACDB_INIT_SUPPORT_ACDB_OVERRIDE_FROM_EFS 

   if (result != ACDB_INIT_SUCCESS)
   {
	   ACDB_DEBUG_LOG("[ACDB Init]->ACDB init failed!\n");
   }
   else
   {
	   ACDB_DEBUG_LOG("[ACDB Init]->ACDB init success!\n");
   }
   return result;
}
