/*===========================================================================
    FILE:           acdb_command.c

    OVERVIEW:       This file contains the implementaion of the helper methods
                    used to service ACDB ioctls.

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

    $Header: //source/qcom/qct/multimedia2/Audio/audcal2/common/8k_PL/acdb/rel/4.2/src/acdb_command.c#2 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2010-11-16  ernanl  1. Added get/set calibration data of Afe table API 
    2010-09-21  ernanl  1. Device Info Data Structure changes, update ACDB to main
                        backward comptible with new changes.
                        2. ADIE calibration support.
    2010-07-23  ernanl  Complete get/set function and introduce new 
                        functions into command specific functions
    2010-07-01  vmn     Split apart the global functions into command
                        specific functions.
    2010-06-16  aas     Fixed the get table so that it returns module id, 
                        param id and data length of each calibration unit
    2010-04-26  aas     Initial implementation of the acdb_ioctl API and 
                        associated helper methods.
	2012-05-15 kpavan   1. New interface for APQ MDM device Mapping.
						2. New interface to get TX RX Device pair for Recording
	2012-06-01 kpavan  	1. New interface to get RX device list for AUDIO EC recording TX device. 

	2012-06-22 kpavan  	1. Topology override fix(CR 370002 ) by naveen. 
    2012-07-10  aboppay Removing warning messages in src


========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_command.h"
#include "acdb.h"
#include "acdb_default_data.h"
#include "acdb_init.h"
#include "acdb_override.h"

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

static int32_t AcdbCmdGetDataIndex (uint32_t ulModuleId,
                                    uint32_t ulParamId,
                                    AcdbDataTopologyType *pTopology,
                                    uint32_t ulTopologySize,
                                    uint32_t *pDataIndex,
                                    uint32_t *pParamMaxSize)
{
   int32_t result = ACDB_SUCCESS;

   if (pTopology == NULL || pDataIndex == NULL || pParamMaxSize == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      uint32_t i =0;
      for (; i < ulTopologySize; ++i)
      {
         if (ulModuleId == pTopology[i].ulModuleId && ulParamId == pTopology[i].ulParamId)
         {
            *pDataIndex = i;
            *pParamMaxSize = pTopology[i].ulMaxParamLen;
            break;
         }
      }
      if (i == ulTopologySize)
      {
         result = ACDB_PARMNOTFOUND;
      }
   }

   return result;
}

/* ---------------------------------------------------------------------------
 * Externalized Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

int32_t AcdbCmdInitializeAcdb ()
{
   int32_t result = ACDB_SUCCESS;
   AcdbDataModuleVersionType modvers;
   AcdbDataTargetVersionType tgtvers;
   AcdbDataModuleVersionType swmodvers;
   AcdbDataTargetVersionType compiledtgtvers;

   // Verify if the ACDBDATA target version is equivalent to the ACDBSW target
   // version and verify if the ACDBDATA data structure version is compatible
   // with the implementation in this file. The expectation is minor version
   // changes should NOT affect the code in this file.
   
   // If a mismatch occurs, fail initialization.
   (void) AcdbDataIoctl (ACDBDATA_GET_ACDB_VERSION, NULL, 0, (uint8_t *)&modvers, sizeof (modvers));
   (void) AcdbDataIoctl (ACDBDATA_GET_TARGET_VERSION, NULL, 0, (uint8_t *)&tgtvers, sizeof (tgtvers));
   (void) AcdbDataIoctl (ACDBDATA_GET_SW_TARGET_VERSION, NULL, 0, (uint8_t *)&compiledtgtvers, sizeof (compiledtgtvers));
   (void) AcdbDataIoctl (ACDBDATA_GET_SW_ACDB_DATA_STRUCTURE_VERSION, NULL, 0, (uint8_t *)&swmodvers, sizeof (swmodvers));


   // Check the Target Version first to protect against a mismatch between
   // Product Lines (where the likelihood of a data structure change is high).
   if (tgtvers.targetversion != compiledtgtvers.targetversion)
   {
      result = ACDB_MISMATCH_TARGET_VERSION;
      ACDB_DEBUG_LOG("[ACDB Command]->Target Version Mismatch between Data and Command\n");
   }
   else if (modvers.major != swmodvers.major)
   {
      result = ACDB_MISMATCH_DATA_SW_VERSION;
      ACDB_DEBUG_LOG("[ACDB Command]->Data Structure Version Mismatch between Data and Command\n");
   }
   else
   {
      (void) acdb_init ();

#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdInitializeAcdb]->Initialization command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdGetAcdbSwVersion (AcdbModuleVersionType *pOutput)
{
   int32_t result = ACDB_SUCCESS;
   uint16_t major = 0;
   uint16_t minor = 0;

   AcdbModuleVersionType majorMinorVersion;

   (void) AcdbDataIoctl (ACDBDATA_GET_SW_ACDB_VERSION, NULL, 0, (uint8_t *)&majorMinorVersion, sizeof (majorMinorVersion));

   major = majorMinorVersion.major;
   minor = majorMinorVersion.minor;

   if (pOutput != NULL)
   {
      memcpy(&pOutput->major,&major,sizeof(pOutput->major));
      memcpy(&pOutput->minor,&minor,sizeof(pOutput->minor));

#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAcdbSwVersion]->Query command request is completed\n");
#endif
   }
   else
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAcdbSwVersion]->System Erorr\n");
      result = ACDB_BADSTATE;
   }

   return result;
}

int32_t AcdbCmdGetTargetVersion (AcdbTargetVersionType *pOutput)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t nTargetVersion = 0;
   AcdbDataTargetVersionType compiledtgtvers;

   (void) AcdbDataIoctl (ACDBDATA_GET_SW_TARGET_VERSION, NULL, 0, (uint8_t *)&compiledtgtvers, sizeof (compiledtgtvers));
   nTargetVersion = compiledtgtvers.targetversion;

   if (pOutput != NULL)
   {
      memcpy(&pOutput->targetversion,&nTargetVersion,sizeof(uint32_t));

#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTargetVersion]->Query command request is completed\n");
#endif
   }
   else
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTargetVersion]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   
   return result;
}

int32_t AcdbCmdGetDevicePair (AcdbDevicePairType *pInput,
                              AcdbDevicePairingResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDevicePair]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      AcdbDataGeneralInfoType tbl;
      result = AcdbDataIoctl (ACDBDATA_GET_DEV_PAIR_TABLE, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));

      if (result == ACDB_SUCCESS && NULL != tbl.pBuffer && 0 != tbl.nBufferLength)
      {
         AcdbDevicePairType *pDevPairTbl = (AcdbDevicePairType *) tbl.pBuffer;
         uint32_t i = tbl.nBufferLength;
         pOutput->ulIsDevicePairValid = 0;
         while (i && pOutput->ulIsDevicePairValid == 0)
         {
            if (pInput->nTxDeviceId == pDevPairTbl->nTxDeviceId &&
                pInput->nRxDeviceId == pDevPairTbl->nRxDeviceId)
            {
               pOutput->ulIsDevicePairValid = 1;
            }
            i -= sizeof(AcdbDevicePairType);
            if (i != 0)
            {
               pDevPairTbl ++;
            }
         }
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDevicePair]->Query command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdGetDevicePairList (AcdbGeneralInfoCmdType *pInput,
                                  AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDevicePairList]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDevicePairList]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGeneralInfoType tbl;
      result = AcdbDataIoctl (ACDBDATA_GET_DEV_PAIR_TABLE, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));

      if (result == ACDB_SUCCESS && tbl.pBuffer != NULL && tbl.nBufferLength != 0)
      {
         if (pInput->nBufferLength >= tbl.nBufferLength)
         {
            memcpy((void*)(*pInput).nBufferPointer,(void*)tbl.pBuffer,tbl.nBufferLength);
            pOutput->nBytesUsedInBuffer = tbl.nBufferLength;
         }
         else
         {
            result = ACDB_BADPARM;            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDevicePairList]->Insufficient buffer to hold the data");            
         }
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDevicePairList]->Query command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudProcTable (AcdbAudProcTableCmdType *pInput,
                                AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAudProcCmnLookupKeyType indx;
      uint32_t nSRId = 0;
      AcdbDataGetTblTopType tbltop;

      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationType;
      // Translate sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nDeviceSampleRateId = pInput->nDeviceSampleRateId;
      }      
      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
	 if( (Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
		(Acdb_DM_Ioctl(ACDB_GET_IS_AUDTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
		)
	 {
		tbltop.nTableEntries = 0;
		tbltop.nTopologyEntries = 0;
		tbltop.ppTable = NULL;
		tbltop.pTopology = NULL;
		result = ACDB_SUCCESS;
	  }
	  else
	  {
		result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY,
					(uint8_t *)&key, sizeof (key),
					(uint8_t *)&tbltop, sizeof (tbltop));
	  }
          if (result == ACDB_SUCCESS)
          {
             result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   (uint8_t*) &pOutput->nBytesUsedInBuffer,
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcTable]->failed. "
                        "DID [0x%08X] DSR [0x%08X] APT [0x%08X].\n",
                        pInput->nDeviceId, pInput->nDeviceSampleRateId,
                        pInput->nApplicationType);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcTable]->Query command request is completed "
                     "DID [0x%08X] DSR [0x%08X] APT [0x%08X].\n",
                     pInput->nDeviceId, pInput->nDeviceSampleRateId,
                     pInput->nApplicationType);
#endif
   }

   return result;
}

int32_t AcdbCmdSetAudProcTable (AcdbAudProcTableCmdType *pInput
                                )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataAudProcCmnLookupKeyType indx;
      AcdbDataLookupKeyType key;
      uint32_t nSRId = 0;

      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationType;
      // Translate sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nDeviceSampleRateId = pInput->nDeviceSampleRateId;
      }

      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key)
                              );

      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
	 if( (Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
	     (Acdb_DM_Ioctl(ACDB_GET_IS_AUDTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
	      )
	 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
	 }
	 else
	 {
		result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY,
					(uint8_t *)&key, sizeof (key),
					(uint8_t *)&tbltop, sizeof (tbltop)
					);
	 }

         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL, 
                                   (AcdbDataGetTblTopType*) &tbltop, 
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   NULL,
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcTable]->Failed. "
                         "DID [0x%08X] DSR [0x%08X] APT [0x%08X].\n",
                         pInput->nDeviceId, pInput->nDeviceSampleRateId,
                         pInput->nApplicationType);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcTable]->Set command request is completed. "
                     "DID [0x%08X] DSR [0x%08X] APT [0x%08X].\n",
                     pInput->nDeviceId, pInput->nDeviceSampleRateId,
                     pInput->nApplicationType);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudProcData (AcdbAudProcCmdType *pInput,
                              AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      AcdbDataLookupKeyType key;
      AcdbDataAudProcCmnLookupKeyType indx;
      AcdbDataGetTblTopType tbltop;
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      uint32_t nSRId = 0;

      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationType;
      // Translate sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nDeviceSampleRateId = pInput->nDeviceSampleRateId;
      }

      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));

      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );

         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {//check pDataNode != NULL
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }
      }
      
      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            uint32_t ulMaxParamSize = 0;
            result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                          pInput->nParamId,
                                          tbltop.pTopology,
                                          tbltop.nTopologyEntries,
                                          &ulPosition,
                                          &ulMaxParamSize);
            if (result == ACDB_SUCCESS)
            {
               ulDataType = ACDB_STATIC_DATA;
               ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

               if (ulDataLen > ulMaxParamSize)
               {
                  result = ACDB_BADSTATE;                  
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcData]->Data length greater than expected max. "
                                  "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                  
               }
            }
         }
      }

      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;            
			pOutput->nBytesUsedInBuffer = ulDataLen;
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcData]->Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);            
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                        pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcData]->Failed. "
                        "DID [0x%08X] DSR [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nDeviceSampleRateId, pInput->nApplicationType,
                        pInput->nModuleId, pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcData]->Query command request is completed. "
                     "DID [0x%08X] DSR [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nDeviceId, pInput->nDeviceSampleRateId, pInput->nApplicationType,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetAudProcData (AcdbAudProcCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataAudProcCmnLookupKeyType indx;
      AcdbDataLookupKeyType key;
      uint32_t nSRId = 0;

      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationType;
      // Translate sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nDeviceSampleRateId = pInput->nDeviceSampleRateId;
      }

      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key)
                              );
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 (Acdb_DM_Ioctl(ACDB_GET_IS_AUDTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 )
		 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
		 }
		 else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&tbltop, sizeof (tbltop)
									 );
		 }
         if (result == ACDB_SUCCESS)
         {

            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
                                   (AcdbDataLookupKeyType*) &key,
                                   (uint32_t*) &pInput->nModuleId,
                                   (uint32_t*) &pInput->nParamId,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer, 
                                   (uint32_t) pInput->nBufferLength, 
                                   NULL, 
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcData]->Failed. "
                        "DID [0x%08X] DSR [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nDeviceSampleRateId, pInput->nApplicationType,
                        pInput->nModuleId, pInput->nParamId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcData]->Set command request is completed. "
                     "DID [0x%08X] DSR [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nDeviceId, pInput->nDeviceSampleRateId, pInput->nApplicationType,
                     pInput->nModuleId, pInput->nParamId);  
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudStrmTable (AcdbAudStrmTableCmdType *pInput,
                                AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudStrmTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudStrmTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAudProcStrmLookupKeyType indx;
      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationTypeId;
      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   (uint8_t*) &pOutput->nBytesUsedInBuffer,
                                   NULL
                                   );
         }
      }

      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudStrmTable]->Failed. "
                        "DID [0x%08X] APT [0x%08X].\n",
                        pInput->nDeviceId, pInput->nApplicationTypeId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudStrmTable]->Query command request is completed. "
                     "DID [0x%08X] APT [0x%08X].\n",
                     pInput->nDeviceId, pInput->nApplicationTypeId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetAudStrmTable (AcdbAudStrmTableCmdType *pInput
                                )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudStrmTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudStrmTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAudProcStrmLookupKeyType indx;
      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationTypeId;

      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL, 
                                   (AcdbDataGetTblTopType*) &tbltop, 
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   NULL,
                                   NULL
                                   );
         }
      }

      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudStrmTable]->Failed. "
                        "DID [0x%08X] APT [0x%08X].\n",
                        pInput->nDeviceId, pInput->nApplicationTypeId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudStrmTable]->Set command request is completed. "
                     "DID [0x%08X] APT [0x%08X].\n",
                     pInput->nDeviceId, pInput->nApplicationTypeId);    
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudStrmData (AcdbAudStrmCmdType *pInput,
                               AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudStrmData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudStrmData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      AcdbDataLookupKeyType key;
      AcdbDataAudProcStrmLookupKeyType indx;
      AcdbDataGetTblTopType tbltop;
      AcdbDynamicUniqueDataType *pDataNode = NULL;

      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationTypeId;
      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));

      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {//check pDataNode != NULL
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }
      }

      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            uint32_t ulMaxParamSize = 0;
            result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                          pInput->nParamId,
                                          tbltop.pTopology,
                                          tbltop.nTopologyEntries,
                                          &ulPosition,
                                          &ulMaxParamSize);
            if (result == ACDB_SUCCESS)
            {
               ulDataType = ACDB_STATIC_DATA;
               ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

               if (ulDataLen > ulMaxParamSize)
               {
                  result = ACDB_BADSTATE;                  
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStreamData]->Data length greater than expected max. "
                                 "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                  
               }
            }
         }
      }

      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;            
			pOutput->nBytesUsedInBuffer = ulDataLen;
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStreamData]->Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);            
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                        pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStreamData]->Failed. "
                        "DID [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nApplicationTypeId,
                        pInput->nModuleId, pInput->nParamId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStreamData]->Query command request is completed. "
                     "DID [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nDeviceId, pInput->nApplicationTypeId,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetAudStrmData (AcdbAudStrmCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudStrmData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudStrmData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataAudProcStrmLookupKeyType indx;
      AcdbDataLookupKeyType key;

      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationTypeId;

      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key)
                              );
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_STRM_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop)
                                 );
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
                                   (AcdbDataLookupKeyType*) &key,
                                   (uint32_t*) &pInput->nModuleId,
                                   (uint32_t*) &pInput->nParamId,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer, 
                                   (uint32_t) pInput->nBufferLength, 
                                   NULL, 
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcStreamData] Failed. "
                        "DID [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nApplicationTypeId,
                        pInput->nModuleId, pInput->nParamId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcStreamData] Set command request is completed. "
                     "DID [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nDeviceId, pInput->nApplicationTypeId,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetVocProcTable (AcdbVocProcTableCmdType *pInput,
                                 AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataVocProcCmnLookupKeyType indx;
      uint32_t nSRId = 0;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      // Translate TX sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nTxDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nTxDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nTxDeviceSampleRateId = pInput->nTxDeviceSampleRateId;
      }
      // Translate RX sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nRxDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nRxDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nRxDeviceSampleRateId = pInput->nRxDeviceSampleRateId;
      }
      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;

		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 ((Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nTxDeviceId,sizeof(indx.nTxDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 || (Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nRxDeviceId,sizeof(indx.nRxDeviceId),NULL,NULL) == ACDB_SUCCESS))
			 )
		 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
		 }else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&tbltop, sizeof (tbltop));
		 }
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   (uint8_t*) &pOutput->nBytesUsedInBuffer,
                                   NULL
                                   );
         }
      }

      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcTable]->Failed. "
                        "TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X] RXSR [0x%08X] NID [0x%08X] VSR [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId,
                        pInput->nTxDeviceSampleRateId, pInput->nRxDeviceSampleRateId,
                        pInput->nNetworkId, pInput->nVocProcSampleRateId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcTable]->Query command request is completed. "
                     "TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X] RXSR [0x%08X] NID [0x%08X] VSR [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId,
                     pInput->nTxDeviceSampleRateId, pInput->nRxDeviceSampleRateId,
                     pInput->nNetworkId, pInput->nVocProcSampleRateId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetVocProcTable (AcdbVocProcTableCmdType *pInput
                                )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataVocProcCmnLookupKeyType indx;
      AcdbDataLookupKeyType key;
      uint32_t nSRId = 0;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      // Translate TX sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nTxDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nTxDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nTxDeviceSampleRateId = pInput->nTxDeviceSampleRateId;
      }
      // Translate RX sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nRxDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nRxDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nRxDeviceSampleRateId = pInput->nRxDeviceSampleRateId;
      }
      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key)
                              );
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 ((Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nTxDeviceId,sizeof(indx.nTxDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 || (Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nRxDeviceId,sizeof(indx.nRxDeviceId),NULL,NULL) == ACDB_SUCCESS))
			 )
		 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
		 }
		 else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&tbltop, sizeof (tbltop)
									 );
		 }
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL, 
                                   (AcdbDataGetTblTopType*) &tbltop, 
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   NULL,
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcTable]->Failed. "
                        "TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X] RXSR [0x%08X] NID [0x%08X] VSR [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId,
                        pInput->nTxDeviceSampleRateId, pInput->nRxDeviceSampleRateId,
                        pInput->nNetworkId, pInput->nVocProcSampleRateId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcTable]->Set command request is completed. "
                     "TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X] RXSR [0x%08X] NID [0x%08X] VSR [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId,
                     pInput->nTxDeviceSampleRateId, pInput->nRxDeviceSampleRateId,
                     pInput->nNetworkId, pInput->nVocProcSampleRateId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetVocProcData (AcdbVocProcCmdType *pInput,
                              AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      AcdbDataLookupKeyType key;
      AcdbDataVocProcCmnLookupKeyType indx;
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      AcdbDataGetTblTopType tbltop;
      uint32_t nSRId = 0;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      // Translate TX sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nTxDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nTxDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nTxDeviceSampleRateId = pInput->nTxDeviceSampleRateId;
      }
      // Translate RX sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nRxDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nRxDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nRxDeviceSampleRateId = pInput->nRxDeviceSampleRateId;
      }
      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {//check pDataNode != NULL
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }
      }

      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            uint32_t ulMaxParamSize = 0;
            result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                          pInput->nParamId,
                                          tbltop.pTopology,
                                          tbltop.nTopologyEntries,
                                          &ulPosition,
                                          &ulMaxParamSize);
            if (result == ACDB_SUCCESS)
            {
               ulDataType = ACDB_STATIC_DATA;
               ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

               if (ulDataLen > ulMaxParamSize)
               {
                  result = ACDB_BADSTATE;                  
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcData]->Data length greater than expected max. "
                                 "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                  
               }
            }
         }
      }

      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;            
			pOutput->nBytesUsedInBuffer = ulDataLen;
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcData] Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);            
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                        pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {      
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcData]->Failed. TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X]"
                        " RXSR [0x%08X] NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nTxDeviceSampleRateId, 
                        pInput->nRxDeviceSampleRateId, pInput->nNetworkId, pInput->nVocProcSampleRateId,
                        pInput->nModuleId, pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcData]->Query command request is completed. TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X]"
                     " RXSR [0x%08X] NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nTxDeviceSampleRateId, 
                     pInput->nRxDeviceSampleRateId, pInput->nNetworkId, pInput->nVocProcSampleRateId,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetVocProcData (AcdbVocProcCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataVocProcCmnLookupKeyType indx;
      uint32_t nSRId = 0;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      // Translate TX sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nTxDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nTxDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nTxDeviceSampleRateId = pInput->nTxDeviceSampleRateId;
      }
      // Translate RX sample rate Id
      result = acdb_translate_sample_rate ((uint32_t) pInput->nRxDeviceSampleRateId,
                                           (uint32_t*) &nSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nRxDeviceSampleRateId = nSRId;
      }
      else
      {
         indx.nRxDeviceSampleRateId = pInput->nRxDeviceSampleRateId;
      }
      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 ((Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nTxDeviceId,sizeof(indx.nTxDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 || (Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nRxDeviceId,sizeof(indx.nRxDeviceId),NULL,NULL) == ACDB_SUCCESS))
			 )
		 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
		 }
		 else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&tbltop, sizeof (tbltop));
		 }
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
                                   (AcdbDataLookupKeyType*) &key,
                                   (uint32_t*) &pInput->nModuleId,
                                   (uint32_t*) &pInput->nParamId,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer, 
                                   (uint32_t) pInput->nBufferLength, 
                                   NULL, 
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcData]->Failed. TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X]"
                        " RXSR [0x%08X] NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nTxDeviceSampleRateId, 
                        pInput->nRxDeviceSampleRateId, pInput->nNetworkId, pInput->nVocProcSampleRateId,
                        pInput->nModuleId, pInput->nParamId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcData]->Set command request is completed. TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X]"
                     " RXSR [0x%08X] NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nTxDeviceSampleRateId, 
                     pInput->nRxDeviceSampleRateId, pInput->nNetworkId, pInput->nVocProcSampleRateId,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetVocStrmTable (AcdbVocStrmTableCmdType *pInput,
                                 AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataVocProcStrmLookupKeyType indx;

      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   (uint8_t*) &pOutput->nBytesUsedInBuffer,
                                   NULL
                                   );
         }
      }

      if (result != ACDB_SUCCESS)
      {
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmTable]->Unable to serialize Qdsp6 Table. "
                        "NID [0x%08X] VSR [0x%08X].\n",pInput->nNetworkId, pInput->nVocProcSampleRateId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmTable]->Query command request is completed. "
                     "NID [0x%08X] VSR [0x%08X].\n",pInput->nNetworkId, pInput->nVocProcSampleRateId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetVocStrmTable (AcdbVocStrmTableCmdType *pInput
                                )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocStrmTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocStrmTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataVocProcStrmLookupKeyType indx;
      AcdbDataLookupKeyType key;

      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL, 
                                   (AcdbDataGetTblTopType*) &tbltop, 
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   NULL,
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocStrmTable]->Unable to serialize Qdsp6 Table. "
                        "NID [0x%08X] VSR [0x%08X].\n",pInput->nNetworkId, pInput->nVocProcSampleRateId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocStrmTable]->Set command request is completed. "
                     "NID [0x%08X] VSR [0x%08X].\n",pInput->nNetworkId, pInput->nVocProcSampleRateId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetVocStrmData (AcdbVocStrmCmdType *pInput,
                              AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      AcdbDataLookupKeyType key;
      AcdbDataVocProcStrmLookupKeyType indx;
      AcdbDataGetTblTopType tbltop;
      AcdbDynamicUniqueDataType *pDataNode = NULL;

      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {//check pDataNode != NULL
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }
      }

      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            uint32_t ulMaxParamSize = 0;
            result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                          pInput->nParamId,
                                          tbltop.pTopology,
                                          tbltop.nTopologyEntries,
                                          &ulPosition,
                                          &ulMaxParamSize);
            if (result == ACDB_SUCCESS)
            {
               ulDataType = ACDB_STATIC_DATA;
               ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

               if (ulDataLen > ulMaxParamSize)
               {
                  result = ACDB_BADSTATE;                  
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmData]->Data length greater than expected max. "
                                 "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                  
               }
            }
         }
      }

      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;            
			pOutput->nBytesUsedInBuffer = ulDataLen;
            ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetVocStrmData]->Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);            
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                        pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmData]->Failed. "
                        "NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nNetworkId, pInput->nVocProcSampleRateId,
                        pInput->nModuleId, pInput->nParamId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStrmData]->Query command request is completed. "
                     "NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nNetworkId, pInput->nVocProcSampleRateId,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetVocStrmData (AcdbVocStrmCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)         
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocStrmData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocStrmData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataVocProcStrmLookupKeyType indx;

      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_STRM_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
                                   (AcdbDataLookupKeyType*) &key,
                                   (uint32_t*) &pInput->nModuleId,
                                   (uint32_t*) &pInput->nParamId,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer, 
                                   (uint32_t) pInput->nBufferLength, 
                                   NULL, 
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdSetVocStrmData]->Failed. "
                        "NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nNetworkId, pInput->nVocProcSampleRateId,
                        pInput->nModuleId, pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdSetVocStrmData]->Set command request is completed. "
                     "NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nNetworkId, pInput->nVocProcSampleRateId,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }
   
   return result;
}

int32_t AcdbCmdGetVocProcGainDepVolTable (AcdbVocProcVolTblCmdType *pInput,
                                          AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataVolTblStepSizeType nVolStep;
      result = AcdbDataIoctl (ACDBDATA_GET_VOL_TABLE_STEP_SIZE,
                              NULL, 0,
                              (uint8_t*)&nVolStep, sizeof(AcdbDataVolTblStepSizeType));
      if (result != ACDB_SUCCESS)
      {
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->"
                        "[ACDBDATA_GET_VOL_TABLE_STEP_SIZE] Failed.\n");         
      }
      else
      {
         uint32_t i = 0;
         uint32_t dst_offset = 0;

         if ((dst_offset+sizeof(uint32_t)) > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;            
			pOutput->nBytesUsedInBuffer = dst_offset+sizeof(uint32_t);
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->Insufficient buffer size\n");            
         }
         else
         {//Number of volume step
            memcpy ((void*) (pInput->nBufferPointer+dst_offset),
                    (const void*) (&nVolStep.VocProcVolTblStepSize),
                     sizeof(uint32_t));
            dst_offset += sizeof(uint32_t);
         }
         for (; i < nVolStep.VocProcVolTblStepSize && result == ACDB_SUCCESS; ++i)
         {
            AcdbDataLookupKeyType key;
            AcdbDataVocProcVolLookupKeyType indx;
            indx.nTxDeviceId = pInput->nTxDeviceId;
            indx.nRxDeviceId = pInput->nRxDeviceId;
            indx.nNetworkId = pInput->nNetworkId;
            indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;
            indx.nVolIndex = i;

            result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_LOOKUP_KEY,
                                    (uint8_t *)&indx, sizeof (indx),
                                    (uint8_t *)&key, sizeof (key));            
            if (result == ACDB_SUCCESS)
            {
				AcdbDataGetTblTopType nTblCal;
				if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
					((Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nTxDeviceId,sizeof(indx.nTxDeviceId),NULL,NULL) == ACDB_SUCCESS)
				 || (Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nRxDeviceId,sizeof(indx.nRxDeviceId),NULL,NULL) == ACDB_SUCCESS))
				 )
				{
					uint8_t temp[4096];
					uint32_t nBytesUsedInBuffer;
					nTblCal.nTableEntries = 0;
					nTblCal.nTopologyEntries = 0;
					nTblCal.ppTable = NULL;
					nTblCal.pTopology = NULL;
					
					result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
										   (AcdbDataLookupKeyType*) &key,
										   NULL,
										   NULL,
										   (AcdbDataGetTblTopType*) &nTblCal,
										   (uint8_t*) temp,
										   (uint32_t) sizeof(temp),
										   (uint8_t*) &nBytesUsedInBuffer,
										   NULL
										   );

					if(result == ACDB_SUCCESS)
					{
						 dst_offset += sizeof(uint32_t);
						 if (nBytesUsedInBuffer+dst_offset > pInput->nBufferLength)
						 {
							result = ACDB_INSUFFICIENTMEMORY;                        
							pOutput->nBytesUsedInBuffer = nBytesUsedInBuffer+dst_offset;
							ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->Insufficient buffer size\n");                        
							break;
						 }
						 else
						 {
							//Table size
							memcpy ((void*) (pInput->nBufferPointer+dst_offset),
									(const void*) (&temp[0]), nBytesUsedInBuffer);
							memcpy ((void*) (pInput->nBufferPointer+dst_offset-sizeof(uint32_t)),
									(const void*) (&nBytesUsedInBuffer), sizeof(uint32_t));
							dst_offset += nBytesUsedInBuffer;
						 }
					}
				}
				else
				{
					result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
										   (uint8_t *)&key, sizeof (key),
										   (uint8_t *)&nTblCal, sizeof (nTblCal));
					if (result == ACDB_SUCCESS)
					{
					  uint32_t j = 0;                       
					  uint32_t nTableSize = 0;

					  for (; j < nTblCal.nTopologyEntries; ++j)
					  {
						 nTableSize += nTblCal.ppTable[j]->ulDataLen + nTblCal.ppTable[j]->ulDataLen%4 
									   + 3*sizeof(uint32_t);
					  }                  
					  if (dst_offset+sizeof(uint32_t) > pInput->nBufferLength)
					  {
						 result = ACDB_INSUFFICIENTMEMORY;                     
						 pOutput->nBytesUsedInBuffer = dst_offset+sizeof(uint32_t);
						 ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->Insufficient buffer size\n");                     
						 break;
					  }
					  else
					  {
						 //Table size
						 dst_offset += sizeof(uint32_t);
					     
						 if (nTableSize+dst_offset > pInput->nBufferLength)
						 {
							result = ACDB_INSUFFICIENTMEMORY;                        
							pOutput->nBytesUsedInBuffer = nTableSize+dst_offset;
							ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->Insufficient buffer size\n");                        
							break;
						 }
						 else
						 {
							uint32_t nBytesUsedInBuffer;
							result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
												   (AcdbDataLookupKeyType*) &key,
												   NULL,
												   NULL,
												   (AcdbDataGetTblTopType*) &nTblCal,
												   (uint8_t*) pInput->nBufferPointer+dst_offset,
												   (uint32_t) pInput->nBufferLength-dst_offset,
												   (uint8_t*) &nBytesUsedInBuffer,
												   NULL
												   );
							//Table size
							memcpy ((void*) (pInput->nBufferPointer+dst_offset-sizeof(uint32_t)),
									(const void*) (&nBytesUsedInBuffer), sizeof(uint32_t));
							dst_offset += nBytesUsedInBuffer;
						 }
					  }
					}
				}
            }
         }
         pOutput->nBytesUsedInBuffer = dst_offset;
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->Failed. "
                        "TXDID [0x%08X] RXDID [0x%08X] NID [0x%08X] VSR [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                        pInput->nVocProcSampleRateId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->Query command request is completed. "
                     "TXDID [0x%08X] RXDID [0x%08X] NID [0x%08X] VSR [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                     pInput->nVocProcSampleRateId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetVocProcGainDepData (AcdbVocProcVolTblDataCmdType *pInput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcGainDepData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcGainDepData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataVocProcVolLookupKeyType indx;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;
      indx.nVolIndex = pInput->nVolumeIndex;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 ((Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nTxDeviceId,sizeof(indx.nTxDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 || (Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nRxDeviceId,sizeof(indx.nRxDeviceId),NULL,NULL) == ACDB_SUCCESS))
			 )
		 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
		 }
		 else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&tbltop, sizeof (tbltop));
		 }
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
                                   (AcdbDataLookupKeyType*) &key,
                                   (uint32_t*) &pInput->nModuleId,
                                   (uint32_t*) &pInput->nParamId,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer, 
                                   (uint32_t) pInput->nBufferLength, 
                                   NULL, 
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcGainDepData]->Failed. TXDID [0x%08X] RXDID [0x%08X]"
                        " NID [0x%08X] VSR [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                        pInput->nVocProcSampleRateId, pInput->nVolumeIndex, pInput->nModuleId,
                        pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcGainDepData]->Set command request is completed. TXDID [0x%08X] RXDID [0x%08X]"
                     " NID [0x%08X] VSR [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                     pInput->nVocProcSampleRateId, pInput->nVolumeIndex, pInput->nModuleId,
                     pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetVocProcGainDepData (AcdbVocProcVolTblDataCmdType *pInput,
                                      AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataGetTblTopType tbltop;
      AcdbDataVocProcVolLookupKeyType indx;
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      indx.nNetworkId = pInput->nNetworkId;
      indx.nVocProcSampleRateId = pInput->nVocProcSampleRateId;
      indx.nVolIndex = pInput->nVolumeIndex;

      result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));         
      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {//check pDataNode != NULL
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }
      }
      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            uint32_t ulMaxParamSize = 0;
            result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                          pInput->nParamId,
                                          tbltop.pTopology,
                                          tbltop.nTopologyEntries,
                                          &ulPosition,
                                          &ulMaxParamSize);
            if (result == ACDB_SUCCESS)
            {
               ulDataType = ACDB_STATIC_DATA;
               ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

               if (ulDataLen > ulMaxParamSize)
               {
                  result = ACDB_BADSTATE;                     
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepData]->Data length greater than expected max. "
                                 "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                     
               }
            }
         }
      }
      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;               
			pOutput->nBytesUsedInBuffer = ulDataLen;
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepData]->Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);               
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                       pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepData]->Failed. TXDID [0x%08X] RXDID [0x%08X] NID [0x%08X]"
                        " VSR [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                        pInput->nVocProcSampleRateId, pInput->nVolumeIndex, pInput->nModuleId,
                        pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepData]->Query command request is completed. TXDID [0x%08X] RXDID [0x%08X] NID [0x%08X]"
                     " VSR [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                     pInput->nVocProcSampleRateId, pInput->nVolumeIndex, pInput->nModuleId,
                     pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetAudProcGainDepData (AcdbAudProcVolTblDataCmdType *pInput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcGainDepData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcGainDepData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAudProcVolLookupKeyType indx;
      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationType;
      indx.nVolIndex = pInput->nVolumeIndex;

      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetAudProcVolTblTopType AudProcTbltop;

		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 (Acdb_DM_Ioctl(ACDB_GET_IS_AUDTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 )
		 {
			 AcdbDataGetTblTopType tbltop;
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
								   (AcdbDataLookupKeyType*) &key,
								   (uint32_t*) &pInput->nModuleId,
								   (uint32_t*) &pInput->nParamId,
								   (AcdbDataGetTblTopType*) &tbltop,
								   (uint8_t*) pInput->nBufferPointer, 
								   (uint32_t) pInput->nBufferLength, 
								   NULL, 
								   NULL
								   );

		 }
		 else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&AudProcTbltop, sizeof (AudProcTbltop));
			 if (result == ACDB_SUCCESS)
			 {//need different handling, set Copp first
				AcdbDataGetTblTopType tbltop = AudProcTbltop.copp;
				result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
									   (AcdbDataLookupKeyType*) &key,
									   (uint32_t*) &pInput->nModuleId,
									   (uint32_t*) &pInput->nParamId,
									   (AcdbDataGetTblTopType*) &tbltop,
									   (uint8_t*) pInput->nBufferPointer, 
									   (uint32_t) pInput->nBufferLength, 
									   NULL, 
									   NULL
									   );
				if (result != ACDB_SUCCESS)
				{//If set data fail, try Popp
				   tbltop = AudProcTbltop.popp;
				        result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
										  (AcdbDataLookupKeyType*) &key,
										  (uint32_t*) &pInput->nModuleId,
										  (uint32_t*) &pInput->nParamId,
										  (AcdbDataGetTblTopType*) &tbltop,
										  (uint8_t*) pInput->nBufferPointer, 
										  (uint32_t) pInput->nBufferLength, 
										  NULL, 
										  NULL
										  );
				}
			 }
		 }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB] Command]->[AcdbCmdSetAudProcGainDepData]->Failed. "
                        "DID [0x%08X] APPID [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex,
                        pInput->nModuleId, pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB] Command]->[AcdbCmdSetAudProcGainDepData]->Set command request is completed. "
                     "DID [0x%08X] APPID [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudProcGainDepData (AcdbAudProcVolTblDataCmdType *pInput,
                                      AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataGetTblTopType tbltop;
      AcdbDataAudProcVolLookupKeyType indx;
      AcdbDataGetAudProcVolTblTopType nTbltop;
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      //to initialize the tbltop
      memset((void*)&tbltop,0, sizeof(tbltop));

      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationType;
      indx.nVolIndex = pInput->nVolumeIndex;

      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {//check pDataNode != NULL
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }

         if (result == ACDB_PARMNOTFOUND)
         {
            result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
                                    (uint8_t *)&key, sizeof (key),
                                    (uint8_t *)&nTbltop, sizeof (nTbltop));
            if (result == ACDB_SUCCESS)
            {
               uint32_t ulMaxParamSize = 0;
               result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                             pInput->nParamId,
                                             nTbltop.copp.pTopology,
                                             nTbltop.copp.nTopologyEntries,
                                             &ulPosition,
                                             &ulMaxParamSize);
               if (result == ACDB_SUCCESS)
               {
                  tbltop = nTbltop.copp;
               }
               else
               {
                  result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                                pInput->nParamId,
                                                nTbltop.popp.pTopology,
                                                nTbltop.popp.nTopologyEntries,
                                                &ulPosition,
                                                &ulMaxParamSize);
                  if (result == ACDB_SUCCESS)
                  {
                     tbltop = nTbltop.popp;
                  }
               }               
               if (result == ACDB_SUCCESS)
               {
                  ulDataType = ACDB_STATIC_DATA;
                  ulDataLen = tbltop.ppTable[ulPosition]->ulDataLen;

                  if (ulDataLen > ulMaxParamSize)
                  {
                     result = ACDB_BADSTATE;                     
                     ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepData]->Data length greater than expected max. "
                                    "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                     
                  }
               }               
            }
         }
      }
      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;               
			pOutput->nBytesUsedInBuffer = ulDataLen;
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepData]->Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);               
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                       pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepData]->Failed. "
                        "DID [0x%08X] APPID [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex,
                        pInput->nModuleId, pInput->nParamId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepData]->Query command request is completed. "
                     "DID [0x%08X] APPID [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex,
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudProcGainDepCoppTableStep (AcdbAudProcGainDepVolTblStepCmdType *pInput,
                                               AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAudProcVolLookupKeyType indx;
      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationType;
      indx.nVolIndex = pInput->nVolumeIndex;
      
      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));        
      //Query Copp table
      if (result == ACDB_SUCCESS)
      {
		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 (Acdb_DM_Ioctl(ACDB_GET_IS_AUDTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 )
		 {
			 uint8_t tempBuff[2048],i;
			 uint8_t *pTemp;
			 uint8_t blnFound;
			 uint32_t *pModList;
			 uint32_t nBytesUsed, noofpoppmodules,nModuleId,offset=0,dstoffset=0;
			 uint16_t nDataLen;
			 AcdbDynamicUniqueDataType *pDataNode = NULL;
			 AcdbDataGetTblTopType tbltop;
			 AcdbDataLookupKeyType key1;
			 

			 key1.nLookupKey = ACDB_PID_AUD_POPP_MODULELIST_GLBTBL;
			 result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key1,NULL,NULL,NULL,NULL,NULL,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
			 if(result == ACDB_SUCCESS)
			 {
				 pTemp = pDataNode->ulDataBuf;
				 memcpy(&noofpoppmodules,pTemp,sizeof(uint32_t));
				 pModList = (uint32_t*)(pTemp + sizeof(uint32_t));
				 tbltop.nTableEntries = 0;
				 tbltop.nTopologyEntries = 0;
				 tbltop.ppTable = NULL;
				 tbltop.pTopology = NULL;

				 
				result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
									   (AcdbDataLookupKeyType*) &key,
									   NULL, 
									   NULL,
									   (AcdbDataGetTblTopType*) &tbltop,
									   (uint8_t*) tempBuff,
									   (uint32_t) sizeof(tempBuff),
									   (uint8_t*) &nBytesUsed,
									   NULL
									   );
				if(result == ACDB_SUCCESS)
				{
					while(offset < nBytesUsed)
					{
						// The recieved data is in the below format
						// uint32 Moduleid
						// uint32 Paramid
						// uint16 datalen
						// uint16 reserved
						// followed by the data of length data_len bytes
						memcpy(&nModuleId,&tempBuff[offset],sizeof(uint32_t));
						memcpy(&nDataLen,&tempBuff[offset+8],sizeof(uint16_t));
						blnFound = FALSE;
						for(i=0;i<noofpoppmodules;i++)
						{
							if(nModuleId == pModList[i])
							{
								blnFound = TRUE;
								break;
							}
						}
						if(blnFound == FALSE) // copying that module and proceed with the next module
						{
							memcpy(pInput->nBufferPointer+dstoffset,&tempBuff[offset],(nDataLen+12));
							dstoffset = dstoffset+12+nDataLen;
						}
						offset = offset + 12 + nDataLen;
					}
					pOutput->nBytesUsedInBuffer = dstoffset;
				}
			 }
			 
		 }
		 else
		 {

			 AcdbDataGetAudProcVolTblTopType nTblCal;
			 result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&nTblCal, sizeof (nTblCal));        
			 if (result == ACDB_SUCCESS && &nTblCal.copp != NULL)
			 {
				uint32_t j = 0;
				uint32_t nTblSize = 0;

				for (; j < nTblCal.copp.nTopologyEntries; ++j)
				{
				   nTblSize += nTblCal.copp.ppTable[j]->ulDataLen + nTblCal.copp.ppTable[j]->ulDataLen%4
							   + 3*sizeof(uint32_t);
				}
				if (nTblSize > pInput->nBufferLength && result == ACDB_SUCCESS)
				{
				   result = ACDB_INSUFFICIENTMEMORY;                  
				   pOutput->nBytesUsedInBuffer = nTblSize;
				   ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepCoppTableStep]->Insufficient buffer size\n");                  
				}
				else
				{
				   result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
										  (AcdbDataLookupKeyType*) &key,
										  NULL, 
										  NULL,
										  (AcdbDataGetTblTopType*) &nTblCal.copp,
										  (uint8_t*) pInput->nBufferPointer,
										  (uint32_t) pInput->nBufferLength,
										  (uint8_t*) &pOutput->nBytesUsedInBuffer,
										  NULL
										  );
				}
			 }//Query Copp Calibration table
		 }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepCoppTableStep]->Failed. "
                        "DID [0x%08X], APPID [0x%08X], VolIndex[%d]\n",
                        pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepCoppTableStep]->Query command request is completed. "
                     "DID [0x%08X], APPID [0x%08X], VolIndex[%d]\n",
                     pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudProcGainDepPoppTableStep (AcdbAudProcGainDepVolTblStepCmdType *pInput,
                                               AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepPoppTableStep]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepPoppTableStep]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAudProcVolLookupKeyType indx;
      indx.nDeviceId = pInput->nDeviceId;
      indx.nApplicationType = pInput->nApplicationType;
      indx.nVolIndex = pInput->nVolumeIndex;

      result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      //Query Copp table
      if (result == ACDB_SUCCESS)
      {
		  if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			  (Acdb_DM_Ioctl(ACDB_GET_IS_AUDTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
			  )
		 {
			 uint8_t tempBuff[2048],i;
			 uint8_t *pTemp;
			 uint8_t blnFound;
			 uint32_t *pModList;
			 uint32_t nBytesUsed, noofpoppmodules,nModuleId,offset=0,dstoffset=0;
			 uint16_t nDataLen;
			 
			 AcdbDataGetTblTopType tbltop;
			 AcdbDataLookupKeyType key1;
			 AcdbDynamicUniqueDataType *pDataNode = NULL;
			 

			 key1.nLookupKey = ACDB_PID_AUD_POPP_MODULELIST_GLBTBL;
			 result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key1,NULL,NULL,NULL,NULL,NULL,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
			 if(result == ACDB_SUCCESS)
			 {
				 pTemp = pDataNode->ulDataBuf;
				 memcpy(&noofpoppmodules,pTemp,sizeof(uint32_t));
				 pModList = (uint32_t*)(pTemp + sizeof(uint32_t));
				 tbltop.nTableEntries = 0;
				 tbltop.nTopologyEntries = 0;
				 tbltop.ppTable = NULL;
				 tbltop.pTopology = NULL;

				 
				result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
									   (AcdbDataLookupKeyType*) &key,
									   NULL, 
									   NULL,
									   (AcdbDataGetTblTopType*) &tbltop,
									   (uint8_t*) tempBuff,
									   (uint32_t) sizeof(tempBuff),
									   (uint8_t*) &nBytesUsed,
									   NULL
									   );
				if(result == ACDB_SUCCESS)
				{
					while(offset < nBytesUsed)
					{
						// The recieved data is in the below format
						// uint32 Moduleid
						// uint32 Paramid
						// uint16 datalen
						// uint16 reserved
						// followed by the data of length data_len bytes
						memcpy(&nModuleId,&tempBuff[offset],sizeof(uint32_t));
						memcpy(&nDataLen,&tempBuff[offset+8],sizeof(uint16_t));
						blnFound = FALSE;
						for(i=0;i<noofpoppmodules;i++)
						{
							if(nModuleId == pModList[i])
							{
								blnFound = TRUE;
								break;
							}
						}
						if(blnFound == TRUE) // copying that module and proceed with the next module
						{
							memcpy(pInput->nBufferPointer+dstoffset,&tempBuff[offset],(nDataLen+12));
							dstoffset = dstoffset+12+nDataLen;
						}
						offset = offset + 12 + nDataLen;
					}
					pOutput->nBytesUsedInBuffer = dstoffset;
				}
			 }
			 
		 }
		  else
		  {
			 AcdbDataGetAudProcVolTblTopType nTblCal;
			 result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&nTblCal, sizeof (nTblCal));
			 if (result == ACDB_SUCCESS && &nTblCal.popp != NULL)
			 {
				uint32_t j = 0;
				uint32_t nTblSize = 0;

				for (; j < nTblCal.popp.nTopologyEntries; ++j)
				{
				   nTblSize += nTblCal.popp.ppTable[j]->ulDataLen + nTblCal.popp.ppTable[j]->ulDataLen%4
							   + 3*sizeof(uint32_t);
				}             
				if (nTblSize > pInput->nBufferLength && result == ACDB_SUCCESS)
				{
				   result = ACDB_INSUFFICIENTMEMORY;                  
				   pOutput->nBytesUsedInBuffer = nTblSize;
				   ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepPoppTableStep]->Insufficient buffer size\n");                  
				}
				else
				{
				   result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
										  (AcdbDataLookupKeyType*) &key,
										  NULL, 
										  NULL,
										  (AcdbDataGetTblTopType*) &nTblCal.popp,
										  (uint8_t*) pInput->nBufferPointer,
										  (uint32_t) pInput->nBufferLength,
										  (uint8_t*) &pOutput->nBytesUsedInBuffer,
										  NULL
										  );
				}//End Query Calibration Data
			 }
		 }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepPoppTableStep]->Failed. "
                        "DID [0x%08X], APPID [0x%08X], VolIndex[%d]\n",
                        pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepPoppTableStep]->Query command request is completed. "
                     "DID [0x%08X], APPID [0x%08X], VolIndex[%d]\n",
                     pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex);
#endif
   }

   return result;
}

int32_t AcdbCmdGetVolTableStepSize (AcdbVolTblStepSizeRspType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVolTableStepSize]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      AcdbDataVolTblStepSizeType param;
      result = AcdbDataIoctl (ACDBDATA_GET_VOL_TABLE_STEP_SIZE,
                              NULL, 0, 
                              (uint8_t *)&param,sizeof(param)
                              );
      memcpy(&pOutput->AudProcVolTblStepSize,&param.AudProcVolTblStepSize,
    	     sizeof(param.AudProcVolTblStepSize));
      memcpy(&pOutput->VocProcVolTblStepSize,&param.VocProcVolTblStepSize,
    	     sizeof(param.VocProcVolTblStepSize));

#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVolTableStepSize]->Query command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdGetMemoryUsage (AcdbMemoryUsageType* pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetMemoryUsage]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      AcdbDataMemoryUsageType rom;
      AcdbDataMemoryUsageType ram;

      AcdbDataIoctl (ACDBDATA_ESTIMATE_MEMORY_USE, NULL, 0,  (uint8_t *)&rom, sizeof (rom));

      pOutput->org_ROM = rom.org_ROM;
      pOutput->data_ROM = rom.data_ROM;

      AcdbDataMemoryRamEstimate ((AcdbDataMemoryUsageType *) &ram);

      pOutput->org_RAM = ram.org_ROM;
      pOutput->data_RAM = ram.data_ROM;

#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetMemoryUsage]->Query command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdSystemReset(void)
{
   int32_t result = ACDB_BADPARM;

   result = Acdb_DM_Ioctl(ACDB_SYS_RESET,
                          NULL,NULL,NULL,NULL,
                          NULL,0,NULL,NULL);

#ifdef ACDB_DEBUG_L1_LOG
   ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSystemReset]->System Reset command request is completed\n");
#endif

   return result;
}

int32_t AcdbCmdGetOEMInfo (AcdbGeneralInfoCmdType *pInput,
                           AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetOEMInfo]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetOEMInfo]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGeneralInfoType InfoBuf;
      result = Acdb_DM_Ioctl(ACDB_GET_OEM_INFO,
                             NULL,NULL,NULL,NULL,
                             (uint8_t*) &InfoBuf,
                             0,NULL,NULL
                             );
      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl(ACDBDATA_GET_OEM_INFO,
                                NULL,0,
                                (uint8_t*)&InfoBuf, sizeof(AcdbDataGeneralInfoType));
      }

      if (result == ACDB_SUCCESS)
      {
         if (InfoBuf.nBufferLength > pInput->nBufferLength)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetOEMInfo]->Insufficient Buffer Length, InputBuffer length %d, Actual Data length %d\n",
                   pInput->nBufferLength,InfoBuf.nBufferLength);            
            result = ACDB_INSUFFICIENTMEMORY;
			pOutput->nBytesUsedInBuffer = InfoBuf.nBufferLength;
         }
         else
         {
            memcpy((void*)pInput->nBufferPointer,(void*)InfoBuf.pBuffer,InfoBuf.nBufferLength);
            pOutput->nBytesUsedInBuffer = InfoBuf.nBufferLength;
         }
      }
      else
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetOEMInfo]->Failed to query OEM Info\n");         
         result = ACDB_BADPARM;
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetOEMInfo]->Set command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdSetOEMInfo (AcdbGeneralInfoCmdType *pInput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetOEMInfo]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetOEMInfo]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      result = Acdb_DM_Ioctl(ACDB_SET_OEM_INFO,
                             NULL,NULL,NULL,NULL,
                             (uint8_t*) pInput,
                             0,NULL,NULL
                             );
   }

   if (result != ACDB_SUCCESS)
   {      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetOEMInfo]->Failed to set OEM Info\n");      
      result = ACDB_BADPARM;
   }
#ifdef ACDB_DEBUG_L1_LOG
   ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetOEMInfo]->Query command request is completed\n");
#endif

   return result;
}

int32_t AcdbCmdGetDateInfo (AcdbGeneralInfoCmdType *pInput,
                            AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDateInfo]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDateInfo]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGeneralInfoType InfoBuf;
      result = Acdb_DM_Ioctl(ACDB_GET_DATE_INFO,
                             NULL,NULL,NULL,NULL,
                             (uint8_t*) &InfoBuf,
                             0,NULL,NULL
                             );

      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl(ACDBDATA_GET_DATE_INFO,
                                NULL,0,
                                (uint8_t*)&InfoBuf, sizeof(AcdbDataGeneralInfoType));
      }

      if (result == ACDB_SUCCESS)
      {
         if (InfoBuf.nBufferLength > pInput->nBufferLength)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDateInfo]->Insufficient Buffer Length,"
                           " InputBuffer length %d, Actual Data length %d\n",
                           InfoBuf.nBufferLength, pInput->nBufferLength);            
            result = ACDB_INSUFFICIENTMEMORY;
			pOutput->nBytesUsedInBuffer = InfoBuf.nBufferLength;
         }
         else
         {
            memcpy((void*)pInput->nBufferPointer,(void*)InfoBuf.pBuffer,InfoBuf.nBufferLength);
            pOutput->nBytesUsedInBuffer = InfoBuf.nBufferLength;
         }
      }
      else
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDateInfo]->Failed to query Date Info\n");         
         result = ACDB_BADPARM;
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDateInfo]->Query command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdSetDateInfo (AcdbGeneralInfoCmdType *pInput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetDateInfo]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetDateInfo]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      result = Acdb_DM_Ioctl(ACDB_SET_DATE_INFO,
                             NULL,NULL,NULL,NULL,
                             (uint8_t*) &(*pInput),
                             0,NULL,NULL
                             );
   }
   
   if (result != ACDB_SUCCESS)
   {      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetDateInfo]->Failed to set Date Info\n");  
   }
#ifdef ACDB_DEBUG_L1_LOG
   ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetDateInfo]->Set command request is completed\n");
#endif

   return result;
}

int32_t AcdbCmdGetANCDevicePair (AcdbAncDevicePairCmdType *pInput,
                                 AcdbAncDevicePairRspType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbAncDevicePairCmdType]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      AcdbDataGeneralInfoType tbl;

      result = AcdbDataIoctl(ACDBDATA_GET_ANC_DEVICE_PAIR_TABLE,
                             NULL, 0,
                             (uint8_t*) &tbl, sizeof(tbl)
                             );
      if (result == ACDB_SUCCESS)
      {
         uint32_t i = tbl.nBufferLength;
         AcdbDevicePairType *pDevPairTbl = (AcdbDevicePairType *) tbl.pBuffer;

         while (i)
         {
            if (pInput->nRxDeviceId == pDevPairTbl->nRxDeviceId)
            {
               pOutput->nTxAncDeviceId = pDevPairTbl->nTxDeviceId;
               break;
            }

            i -= sizeof(AcdbDevicePairType);
            
            if (i < tbl.nBufferLength)
            {
               pDevPairTbl ++;
            }
         }
         if (i == 0)
         {
            result = ACDB_BADPARM;
            pOutput->nTxAncDeviceId = 0;
         }
      }
      else
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetANCDevicePair]->Failed to query ANC Tx Device, RXDID [0x%08X]\n",
                        pInput->nRxDeviceId);
         result = ACDB_BADPARM;
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetANCDevicePair]->Query command request is completed, RXDID [0x%08X]\n",
                     pInput->nRxDeviceId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetANCDevicePairList (AcdbGeneralInfoCmdType *pInput,
                                     AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetANCDevicePairList]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetANCDevicePairList]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGeneralInfoType tbl;

      result = AcdbDataIoctl(ACDBDATA_GET_ANC_DEVICE_PAIR_TABLE,
                             NULL, 0,
                             (uint8_t*) &tbl, sizeof(tbl)
                             );
      if (result == ACDB_SUCCESS)
      {
         if (tbl.nBufferLength > pInput->nBufferLength)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetANCDevicePairList]->Insufficient Buffer Length,"
                           " InputBuffer length %d, Actual table length %d\n",
                           pInput->nBufferLength, tbl.nBufferLength);            
            result = ACDB_BADPARM;
         }
         else
         {
            memcpy((void*) pInput->nBufferPointer,(void*) tbl.pBuffer,tbl.nBufferLength);
            pOutput->nBytesUsedInBuffer = tbl.nBufferLength;
         }
      }
      else
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetANCDevicePairList]->Failed to query ANC Device Pair List\n");         
         result = ACDB_BADPARM;
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetANCDevicePairList]->Query command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdSetAdieProfileTable (AcdbAdiePathProfileCmdType *pInput
                                   )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieProfileTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieProfileTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      // Query Adie Codec Path Profile lookup key
      AcdbDataAdieProfileLookupKeyType indx;
      AcdbDataLookupKeyType key;
      uint32_t nOSRId = 0;

      indx.nCodecPathId = pInput->ulCodecPathId;
      indx.nFrequencyId = pInput->nFrequencyId;
      //Translate nOSRId
      result = acdb_translate_over_sample_rate ((uint32_t) pInput->nOversamplerateId,
                                                (uint32_t*) &nOSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nOverSampleRateId = nOSRId;
      }
      else
      {
         indx.nOverSampleRateId = pInput->nOversamplerateId;
      }

      result = AcdbDataIoctl (ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key)
                              );
         
      if (result == ACDB_SUCCESS)
      {// Query Adie Codec Path Profile table
         AcdbDataGetParamType param;
         result = AcdbDataIoctl (ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_TABLE,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&param, sizeof (param)
                                 );
            
         if (result == ACDB_SUCCESS)
         {
            if (param.nParamId != pInput->nParamId)
            {
               result = ACDB_BADPARM;
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieProfileTable]->ParamID"
                              " version mismatch between acdb file and default_data!\n");               
            }
            else
            {
               int32_t memcmpResult;

               if (pInput->nBufferLength == param.nParamSize)
               {// check if data length is equal to default data length
                  memcmpResult = memcmp((void*)param.pParam,(void*)pInput->nBufferPointer,
                                        param.nParamSize);
               }
               else
               {
                  memcmpResult = ACDB_PARMNOTFOUND;
               }

               if (memcmpResult != ACDB_SUCCESS)
               {// if data does not match to default,set Adie Codec Path Profile Table on heap
                  result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_ADIE_TABLE,
                                         (AcdbDataLookupKeyType*) &key,
                                         NULL,NULL,NULL,
                                         (uint8_t*) pInput->nBufferPointer, 
                                         (uint32_t) pInput->nBufferLength,
                                         NULL,NULL
                                         );
               }
               else
               {// input data is equal to default, free the data node if it is exist on heap
                  result = Acdb_ChecktoFreeAdieTableCalOnHeap((AcdbDataLookupKeyType*) &key,
                                                              (uint8_t*) pInput->nBufferPointer,
                                                              (uint32_t) pInput->nBufferLength
                                                              );
               }
            }
         }
         else
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieProfileTable]->Failed. "
                           "CodecPathID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                           pInput->ulCodecPathId, pInput->nFrequencyId, pInput->nOversamplerateId);            
         }
      }         
      else
      {// Query Adie Codec Path Profile Lookup failed         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieProfileTable]->Failed. "
                        "CodecPathID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                        pInput->ulCodecPathId, pInput->nFrequencyId, pInput->nOversamplerateId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieProfileTable]->Set command request is completed. "
                     "CodecPathID [0x%08X] ParamId [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                     pInput->ulCodecPathId, pInput->nParamId, pInput->nFrequencyId, pInput->nOversamplerateId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAdieProfileTable (AcdbAdiePathProfileCmdType *pInput,
                                    AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAdieProfileLookupKeyType indx;
      uint32_t nOSRId = 0;

      // Query Adie Codec Path Profile Lookup
      indx.nCodecPathId = pInput->ulCodecPathId;
      indx.nFrequencyId = pInput->nFrequencyId;
      //translate nOSRId
      result = acdb_translate_over_sample_rate ((uint32_t) pInput->nOversamplerateId,
                                                (uint32_t*) &nOSRId);

      if (result == ACDB_SUCCESS)
      {
         indx.nOverSampleRateId = nOSRId;
      }
      else
      {
         indx.nOverSampleRateId = pInput->nOversamplerateId;
      }

      result = AcdbDataIoctl (ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));

      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->Failed. "
                        "CodecPathID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                        pInput->ulCodecPathId, pInput->nFrequencyId, pInput->nOversamplerateId);         
      }
      else
      {
         // Query Adie Codec Path Profile table
         AcdbDataGetParamType param;
         result = AcdbDataIoctl (ACDBDATA_GET_ADIE_CODEC_PATH_PROFILE_TABLE,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&param, sizeof (param));

         if (result != ACDB_SUCCESS)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->Failed. "
                           "CodecPathID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                           pInput->ulCodecPathId, pInput->nFrequencyId, pInput->nOversamplerateId);            
         }
         else
         {
            result = acdb_validate_PID ((uint32_t) ACDB_PID_ADIE_PROFILE_TABLE,
                                        (uint32_t*) &pInput->nParamId,
                                        (uint32_t) param.nParamId);  

            if (result != ACDB_SUCCESS)
            {               
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->Adie Codec Path Profile client passed PID "
                              "does not match to default Input PID [0x%08X], Default PID [0x%08X].\n",
                              pInput->nParamId, param.nParamId);
            }            
            else
            {
               AcdbDynamicUniqueDataType *pDataNode = NULL;

               // Query Adie Table from Heap
               result = Acdb_DM_Ioctl(ACDB_GET_ADIE_TABLE,
                                      (AcdbDataLookupKeyType*) &key,
                                      NULL,NULL,NULL,NULL,0,NULL,
                                      (AcdbDynamicUniqueDataType**) &pDataNode
                                      );

               if (result == ACDB_SUCCESS && pDataNode != NULL)
               {
                  param.pParam = pDataNode->ulDataBuf;
                  param.nParamSize = pDataNode->ulDataLen;
               }
               
               if (result == ACDB_PARMNOTFOUND || result == ACDB_SUCCESS)
               {// Lookup key index not found in heap
                  if (pInput->nBufferLength < param.nParamSize)
                  {
                     result = ACDB_INSUFFICIENTMEMORY;
					 pOutput->nBytesUsedInBuffer = param.nParamSize;
                  }
                  else
                  {
                     result = acdb_adieprofile_translation ((uint32_t) pInput->nParamId,
                                                            (uint32_t) param.nParamId,
                                                            (uint8_t*) pInput->nBufferPointer,
                                                            (uint8_t*) param.pParam,
                                                            (uint32_t) param.nParamSize
                                                            );
                     pOutput->nBytesUsedInBuffer = param.nParamSize;
                  }
               }
            }
         }
         
         if (result != ACDB_SUCCESS)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->Failed. CodecPathId [0x%08X] ParamId [0x%08X]."
                           "FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                           pInput->ulCodecPathId, pInput->nParamId, pInput->nFrequencyId, pInput->nOversamplerateId);
         }
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->Query command request is completed. CodecPathId [0x%08X] ParamId [0x%08X]."
                     "FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                     pInput->ulCodecPathId, pInput->nParamId, pInput->nFrequencyId, pInput->nOversamplerateId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetAdieANCDataTable (AcdbANCSettingCmdType *pInput
                                    )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieANCDataTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieANCDataTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      // Query Adie Codec Path Profile lookup key
      AcdbDataAdieANCConfigDataLookupKeyType indx;
      AcdbDataLookupKeyType key;
      uint32_t nOSRId = 0;

      indx.nRxDeviceId = pInput->nRxDeviceId;
      indx.nFrequencyId = pInput->nFrequencyId;
      //Translate nOSRId
      result = acdb_translate_over_sample_rate ((uint32_t) pInput->nOversamplerateId,
                                                (uint32_t*) &nOSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nOverSampleRateId = nOSRId;
      }
      else
      {
         indx.nOverSampleRateId = pInput->nOversamplerateId;
      }

      result = AcdbDataIoctl (ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key)
                              );
         
      if (result == ACDB_SUCCESS)
      {// Query Adie Codec Path Profile table
         AcdbDataGetParamType param;
         result = AcdbDataIoctl (ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_TABLE,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&param, sizeof (param)
                                 );
            
         if (result == ACDB_SUCCESS)
         {
            if (param.nParamId != pInput->nParamId)
            {
               result = ACDB_BADPARM;               
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieANCDataTable]->ParamID version mismatch"
                              " between acdb file and default_data!\n");               
            }
            else
            {
               int32_t memcmpResult;

               if (pInput->nBufferLength == param.nParamSize)
               {// check if data length is equal to default data length
                  memcmpResult = memcmp((void*)param.pParam,(void*)pInput->nBufferPointer,
                                        param.nParamSize);
               }
               else
               {
                  memcmpResult = ACDB_PARMNOTFOUND;
               }
               
               if (memcmpResult != ACDB_SUCCESS)
               {// if data does not match to default,set Adie Codec Path Profile Table on heap
                  result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_ADIE_TABLE,
                                         (AcdbDataLookupKeyType*) &key,
                                         NULL,NULL,NULL,
                                         (uint8_t*) pInput->nBufferPointer, 
                                         (uint32_t) pInput->nBufferLength,
                                         NULL,NULL
                                         );
               }
               else
               {// input data is equal to default, free the data node if it is exist on heap
                  result = Acdb_ChecktoFreeAdieTableCalOnHeap((AcdbDataLookupKeyType*) &key,
                                                              (uint8_t*) pInput->nBufferPointer,
                                                              (uint32_t) pInput->nBufferLength
                                                              );
               }
            }
         }
         else
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieANCDataTable]->Failed. "
                           "RxDeviceID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                           pInput->nRxDeviceId, pInput->nFrequencyId, pInput->nOversamplerateId);            
         }
      }         
      else
      {// Query Adie Codec Path Profile Lookup failed         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieANCDataTable]->Failed. "
                        "RxDeviceID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                        pInput->nRxDeviceId, pInput->nFrequencyId, pInput->nOversamplerateId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieANCDataTable]->Set command request is completed. "
                     "RxDeviceID [0x%08X] ParamID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                     pInput->nRxDeviceId, pInput->nParamId, pInput->nFrequencyId, pInput->nOversamplerateId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAdieANCDataTable (AcdbANCSettingCmdType *pInput,
                                    AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      // Query Adie Codec Path Profile Lookup
      AcdbDataLookupKeyType key;
      AcdbDataAdieANCConfigDataLookupKeyType indx;
      uint32_t nOSRId = 0;

      indx.nRxDeviceId = pInput->nRxDeviceId;
      indx.nFrequencyId = pInput->nFrequencyId;
      //Translate nOSRId
      result = acdb_translate_over_sample_rate ((uint32_t) pInput->nOversamplerateId,
                                                (uint32_t*) &nOSRId);
      if (result == ACDB_SUCCESS)
      {
         indx.nOverSampleRateId = nOSRId;
      }
      else
      {
         indx.nOverSampleRateId = pInput->nOversamplerateId;
      }

      result = AcdbDataIoctl (ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));

      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->Failed. "
                        "RxDeviceID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                        pInput->nRxDeviceId, pInput->nFrequencyId, pInput->nOversamplerateId);         
      }
      else
      {// Query Adie Codec Path Profile table
         AcdbDataGetParamType param;
         result = AcdbDataIoctl (ACDBDATA_GET_ADIE_ANC_CONFIG_DATA_TABLE,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&param, sizeof (param)
                                 );
         if (result != ACDB_SUCCESS)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->Failed. "
                           "RxDeviceID [0x%08X] FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                           pInput->nRxDeviceId, pInput->nFrequencyId, pInput->nOversamplerateId);            
         }
         else
         {
            result = acdb_validate_PID ((uint32_t) ACDB_PID_ANC_CONFIG_DATA_TABLE,
                                        (uint32_t*) &pInput->nParamId,
                                        (uint32_t) param.nParamId);
            if (result != ACDB_SUCCESS)
            {
               ACDB_DEBUG_LOG("[[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->ANC Config data client passed "
                              "PID does not match to default Input PID [0x%08X], Default PID [0x%08X].\n",
                              pInput->nParamId, param.nParamId);               
            }  
            else
            {
               AcdbDynamicUniqueDataType *pDataNode = NULL;

               // Query Adie ANC config data Table from Heap
               result = Acdb_DM_Ioctl(ACDB_GET_ADIE_TABLE,
                                      (AcdbDataLookupKeyType*) &key,
                                      NULL,NULL,NULL,NULL,0,NULL,
                                      (AcdbDynamicUniqueDataType**) &pDataNode
                                      );
               if (result == ACDB_SUCCESS && pDataNode != NULL)
               {//check pDataNode != NULL
                  param.pParam = pDataNode->ulDataBuf;
                  param.nParamSize = pDataNode->ulDataLen;
               }
               
               if (result == ACDB_PARMNOTFOUND || result == ACDB_SUCCESS)
               {// Lookup key index not found in heap
                  if (pInput->nBufferLength < param.nParamSize)
                  {
                     result = ACDB_INSUFFICIENTMEMORY;
					 pOutput->nBytesUsedInBuffer = param.nParamSize;
                  }
                  else
                  {
                     result = ACDB_SUCCESS;
                     memcpy ((void *) pInput->nBufferPointer,
                             (void *) param.pParam,
                             param.nParamSize);
                     pOutput->nBytesUsedInBuffer = param.nParamSize;
                  }
               }
            }
         }
         
         if (result != ACDB_SUCCESS)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->Failed. RxDID [0x%08X] ParamId [0x%08X] "
                           "FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                           pInput->nRxDeviceId, pInput->nParamId, pInput->nFrequencyId, pInput->nOversamplerateId);            
         }
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->Query command request is completed. RxDID [0x%08X] ParamId [0x%08X] "
                     "FrequencyIndexID [0x%08X] OverSampleRateID [0x%08X].\n",
                     pInput->nRxDeviceId, pInput->nParamId, pInput->nFrequencyId, pInput->nOversamplerateId); 
#endif
   }

   return result;
}

int32_t AcdbCmdGetLookupTableSize (AcdbGetTableSizeCmdType *pInput,
                                   AcdbGetTableSizeRspType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetLookupTableSize]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      uint32_t nCommandId = ACDBDATA_GET_TABLE_SIZE;

      AcdbDataGetTblSizeCmdType nTblSizeCmd;
      AcdbDataGetTblSizeRspType nTblSize;

      result = acdb_map_command_PID ((uint32_t) pInput->nParamId,
                                     (uint32_t*) &nTblSizeCmd.nParamId
                                     );

      if (result == ACDB_SUCCESS)
      {
         result = AcdbDataIoctl (nCommandId,
                                 (uint8_t*) &nTblSizeCmd, sizeof(nTblSizeCmd),
                                 (uint8_t*) &nTblSize, sizeof(nTblSize)
                                 );
         
         if (result == ACDB_SUCCESS)
         {
            memcpy((void*)&pOutput->nEntries,(void*)&nTblSize.nEntries,sizeof(nTblSize.nEntries));
         }
         else
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetLookupTableSize]->Query table size fails!, nParamId [0x%08X]\n",
                           pInput->nParamId);            
         }
      }
      else
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetLookupTableSize]->Translate cmdPID to dataPID failed!, nParamId [0x%08X]\n",
                        pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetLookupTableSize]->Query command request is completed, nParamId [0x%08X]\n",
                     pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetTableLookupIndex (AcdbQueriedTblIndexCmdType *pInput,
                                    AcdbQueryResponseType *pOutput
                                    )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      uint32_t nCommandId = ACDBDATA_GET_TABLE_LOOKUP_INDEX;
      AcdbDataGetTblIndexCmdType nTblIndexCmd;
      AcdbDataGetTblIndexRspType nTblIndexRsp;

      nTblIndexCmd.nTblKeyIndex = pInput->nTblKeyIndexStart;

      result = acdb_map_command_PID ((uint32_t) pInput->nParamId,
                                     (uint32_t*) &nTblIndexCmd.nParamId
                                     );

      if (result == ACDB_SUCCESS)
      {
         result = AcdbDataIoctl (nCommandId,
                                 (uint8_t*)&nTblIndexCmd, sizeof(nTblIndexCmd),
                                 (uint8_t*)&nTblIndexRsp, sizeof(nTblIndexRsp)
                                 );

         if (result == ACDB_SUCCESS)
         {
            switch (pInput->nParamId)
            {
               //VocProc Volume Table requires special handling
               /* 
               typedef struct _AcdbDataVocProcVolLookupKeyType {
                  uint32_t nTxDeviceId;
                  uint32_t nRxDeviceId;
                  uint32_t nNetworkId;
                  uint32_t nVocProcSampleRateId;
                  uint32_t nVolIndex; ==> does not require as an index when querying volume table
               } AcdbDataVocProcVolLookupKeyType;
               */
            case ACDB_PID_VOCPROC_VOLUME_TABLE:
               {
                  const uint32_t nLookupTblEntrySize = sizeof(AcdbDataVocProcVolLookupKeyType);
                  const uint32_t nTargetTblEntrySize = nLookupTblEntrySize - sizeof(uint32_t);
                  uint32_t nByteToCopy = pInput->nDataLenToCopy;
                  uint32_t offsetSrc = 0;
                  uint32_t offsetDst = 0;

                  if (nByteToCopy%nTargetTblEntrySize == 0)
                  {
                     if (pInput->nDataLenToCopy/nTargetTblEntrySize <= nTblIndexRsp.nAvailableByteToCopy
                         /nLookupTblEntrySize && pInput->nDataLenToCopy <= pInput->nBufferLength)
                     {
                        AcdbDataVolTblStepSizeType nVolStep;

                        result = AcdbDataIoctl (ACDBDATA_GET_VOL_TABLE_STEP_SIZE,
                                                NULL, 0,
                                                (uint8_t*)&nVolStep, 
                                                sizeof(AcdbDataVolTblStepSizeType)
                                                );

                        if (result == ACDB_SUCCESS)
                        {
                           while (nByteToCopy >= nTargetTblEntrySize)
                           {
                              nByteToCopy -= nTargetTblEntrySize;
                              //Perform memcpy
                              memcpy((void*)(pInput->nBufferPointer + offsetDst), 
                                     (void*)(nTblIndexRsp.pBufferPointer + offsetSrc),
                                     nTargetTblEntrySize);
                           
                              //Calculate offset
                              offsetDst += nTargetTblEntrySize;
                              offsetSrc += nLookupTblEntrySize*nVolStep.VocProcVolTblStepSize;
                           }
                           memcpy((void*)&pOutput->nBytesUsedInBuffer,(void*)&pInput->nDataLenToCopy,
                                  sizeof(pInput->nDataLenToCopy));
                        }
                     }
                     else
                     {
                        result = ACDB_BADPARM;                        
                        ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->Actual data length to copy is out "
                                       "of bounds!, nParamId [0x%08X]\n",pInput->nParamId);                        
                     }
                  }
                  else
                  {
                     result = ACDB_BADPARM;
                  }
               }
               break;
               //AudProc Volume Table requires special handling
               /* 
               typedef struct _AcdbDataAudProcVolLookupKeyType {
                  uint32_t nDeviceId
                  uint32_t nApplicationType;
                  uint32_t nVolIndex; ==> does not require as an index when querying volume table
               } AcdbDataAudProcVolLookupKeyType;
               */
            case ACDB_PID_AUDPROC_VOLUME_TABLE:
               {
                  const uint32_t nLookupTblEntrySize = sizeof(AcdbDataAudProcVolLookupKeyType);
                  const uint32_t nTargetTblEntrySize = nLookupTblEntrySize - sizeof(uint32_t);
                  uint32_t nByteToCopy = pInput->nDataLenToCopy;
                  uint32_t offsetSrc = 0;
                  uint32_t offsetDst = 0;
                     
                  if (nByteToCopy%nTargetTblEntrySize == 0)
                  {
                     if (pInput->nDataLenToCopy/nTargetTblEntrySize <= nTblIndexRsp.nAvailableByteToCopy
                         /nLookupTblEntrySize && pInput->nDataLenToCopy <= pInput->nBufferLength)
                     {
                        AcdbDataVolTblStepSizeType nVolStep;

                        result = AcdbDataIoctl (ACDBDATA_GET_VOL_TABLE_STEP_SIZE,
                                                NULL, 0,
                                                (uint8_t*)&nVolStep, 
                                                sizeof(AcdbDataVolTblStepSizeType)
                                                );

                        if (result == ACDB_SUCCESS)
                        {
                           while (nByteToCopy >= nTargetTblEntrySize)
                           {
                              nByteToCopy -= nTargetTblEntrySize;
                              //Perform memcpy
                              memcpy((void*)(pInput->nBufferPointer + offsetDst), 
                                     (void*)(nTblIndexRsp.pBufferPointer + offsetSrc),
                                     nTargetTblEntrySize);
                           
                              //Calculate offset
                              offsetDst += nTargetTblEntrySize;
                              offsetSrc += nLookupTblEntrySize*nVolStep.AudProcVolTblStepSize;
                           }
                           memcpy((void*)&pOutput->nBytesUsedInBuffer,(void*)&pInput->nDataLenToCopy,
                                  sizeof(pInput->nDataLenToCopy));
                        }
                     }
                     else
                     {
                        result = ACDB_BADPARM;                        
                        ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->Actual data length to "
                                       "copy is out of bounds!, nParamId [0x%08X]\n",pInput->nParamId);                        
                     }
                  }
                  else
                  {
                     result = ACDB_BADPARM;
                  }
               }
               break;
            default:
               if (pInput->nDataLenToCopy <= nTblIndexRsp.nAvailableByteToCopy &&
                   pInput->nDataLenToCopy <= pInput->nBufferLength)
               {
                  uint32_t nRsp = 0;

                  result = acdb_validate_data_to_copy ((uint32_t) pInput->nParamId,
                                                       (uint32_t) pInput->nDataLenToCopy,
                                                       (uint32_t*) &nRsp
                                                       );

                  if (result == ACDB_SUCCESS && nRsp)
                  {
                     memcpy((void*)pInput->nBufferPointer, (void*)nTblIndexRsp.pBufferPointer,
                            pInput->nDataLenToCopy);
                     memcpy((void*)&pOutput->nBytesUsedInBuffer,(void*)&pInput->nDataLenToCopy,
                    	       sizeof(pInput->nDataLenToCopy));
                  }
                  else
                  {
                     result = ACDB_BADPARM;
                  }
               }
               else
               {
                  result = ACDB_BADPARM;                  
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->Actual data length to "
                                 "copy is out of bounds!, nParamId [0x%08X]\n",pInput->nParamId);                  
               }
               break;
            }
         }
         else
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->Query table index fails!, nParamId [0x%08X]\n",
                           pInput->nParamId);            
         }
      }
      else
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->Translate cmdPID to dataPID failed!, nParamId [0x%08X]\n",
                        pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->Query command request is completed, nParamId [0x%08X]\n",
                     pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudProcCmnTopId (AcdbGetAudProcTopIdCmdType *pInput,
                                   AcdbGetTopologyIdRspType *pOutput
                                   )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopId]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      AcdbDataGetAudProcCmnTopIdCmdType nCmd;
	  AcdbDataGetTopologyIdRspType nRsp = {0};

      nCmd.nDeviceId = pInput->nDeviceId;
      nCmd.nAppTypeId = pInput->nApplicationType;

	  if( (Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
		 (Acdb_DM_Ioctl(ACDB_GET_IS_AUDTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&nCmd.nDeviceId,sizeof(nCmd.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
		 )
	  {
		 AcdbDynamicUniqueDataType *pDataNode = NULL;
		 AcdbDataLookupKeyType key;
		 key.nLookupKey = ACDB_PID_AUDPROC_CMN_TOPID;
		 result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,NULL,0,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
		 if(result == ACDB_SUCCESS)
		 {
			 uint32_t i = 0;
			 uint32_t nSize = pDataNode->ulDataLen / sizeof(AcdbDataGetAudProcTopIdType);
			 AcdbDataGetAudProcTopIdType *pAudTopTbl = (AcdbDataGetAudProcTopIdType *)pDataNode->ulDataBuf;
			 for(i=0;i<nSize;i++)
			 {
				 if(pAudTopTbl->nDeviceId == nCmd.nDeviceId)
				 {
					 nRsp.nTopologyId = pAudTopTbl->nTopoId;
					 break;
				 }
				 pAudTopTbl++;
			 }
		 }
		 else
		 {
			 result = ACDB_ERROR;
		 }
	  }
	  else
	  {
      result = AcdbDataIoctl (ACDBDATA_GET_AUDPROC_CMN_TOPOLOGY_ID,
                              (uint8_t*) &nCmd, sizeof(AcdbDataGetAudProcCmnTopIdCmdType),
                              (uint8_t*) &nRsp, sizeof(AcdbDataGetTopologyIdRspType)
                              );
	  }
      if (result == ACDB_SUCCESS)
      {
         pOutput->nTopologyId = nRsp.nTopologyId;
      }
      else
      {        
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopId]->Query for AudProc common topologyId failed! "
                        "DID [0x%08X] ApptypeID [0x%08X]\n",nCmd.nDeviceId, nCmd.nAppTypeId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopId]->Query command request is completed "
                     "DID [0x%08X] ApptypeID [0x%08X]\n",nCmd.nDeviceId, nCmd.nAppTypeId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudProcStrmTopId (AcdbGetAudProcStrmTopIdCmdType *pInput,
                                    AcdbGetTopologyIdRspType *pOutput
                                    )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopId]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      AcdbDataGetAudProcStrmTopIdCmdType nCmd;
      AcdbDataGetTopologyIdRspType nRsp;

      nCmd.nAppTypeId = pInput->nApplicationType;

      result = AcdbDataIoctl (ACDBDATA_GET_AUDPROC_STRM_TOPOLOGY_ID,
                              (uint8_t*) &nCmd, sizeof(AcdbDataGetAudProcStrmTopIdCmdType),
                              (uint8_t*) &nRsp, sizeof(AcdbDataGetTopologyIdRspType)
                              );
      if (result == ACDB_SUCCESS)
      {
         pOutput->nTopologyId = nRsp.nTopologyId;
      }
      else
      {
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopId]->Query for AudProc stream topologyId failed! "
                        "ApptypeID [0x%08X]\n",nCmd.nAppTypeId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopId]->Query command request is completed "
                     "ApptypeID [0x%08X]\n",nCmd.nAppTypeId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetAudProcCmnTopIdList (AcdbGeneralInfoCmdType *pInput,
                                       AcdbQueryResponseType *pOutput
                                       )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopIdList]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopIdList]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      if (pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
      {
         result = ACDB_BADPARM;
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopIdList]->NULL Input!\n");         
      }
      else
      {
         AcdbDataGetTopIdCmdType nCmd;
         AcdbQueryResponseType nRsp;

         


		 AcdbDynamicUniqueDataType *pDataNode = NULL;
		 AcdbDataLookupKeyType key;
		 key.nLookupKey = ACDB_PID_AUDPROC_CMN_TOPID;
		 if(Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS)
		 {
		       result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,NULL,NULL,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
		 }
		 else
		 {
			 result = ACDB_NOTSUPPORTED;
		 }
		 if(result != ACDB_SUCCESS)
		 {
			 nCmd.nParamId = ACDBDATA_PID_AUDPROC_COMMON_TOPOLOGY_ID;
			 result = AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST,
									 (uint8_t*) &nCmd, sizeof(AcdbDataGetTopIdCmdType),
									 (uint8_t*) &nRsp, sizeof(AcdbQueryResponseType));
		 }
		 else
		 {
			 nCmd.nParamId = pDataNode->ulParamId;
			 nCmd.pBufferPointer = pDataNode->ulDataBuf;
			 nRsp.nBytesUsedInBuffer = pDataNode->ulDataLen;
		 }
         if (result != ACDB_SUCCESS)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopIdList]->Query for AudProc common topologyId list failed!\n");            
         }
         else
         {
            if (pInput->nBufferLength >= nRsp.nBytesUsedInBuffer)
            {
               memcpy((void*)pInput->nBufferPointer, (void*)nCmd.pBufferPointer,
                      nRsp.nBytesUsedInBuffer);
               pOutput->nBytesUsedInBuffer = nRsp.nBytesUsedInBuffer;
            }
            else
            {
			   result=ACDB_INSUFFICIENTMEMORY;
			   pOutput->nBytesUsedInBuffer = nRsp.nBytesUsedInBuffer;
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopIdList]->Insufficient buffer size to hold the data!\n");               
            }// check whether buffer size big enough to hold the data
         }// check acdbdata_ioctl query return success or not 
      }// check buffer if it is null
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopIdList]->Query command request is completed\n");
#endif
   }// check input if it is null

   return result;
}

int32_t AcdbCmdGetAudProcStrmTopIdList (AcdbGeneralInfoCmdType *pInput,
                                        AcdbQueryResponseType *pOutput
                                        )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopIdList]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopIdList]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGetTopIdCmdType nCmd;
      AcdbQueryResponseType nRsp;

      

	  AcdbDynamicUniqueDataType *pDataNode = NULL;
	  AcdbDataLookupKeyType key;
		 
	  key.nLookupKey = ACDB_PID_AUDPROC_STRM_TOPID;
	  if(Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS)
	  {
	      result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,NULL,NULL,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
	  }
	  else
	  {
		  result = ACDB_NOTSUPPORTED;
	  }
	  if(result != ACDB_SUCCESS)
	  {
		  nCmd.nParamId = ACDBDATA_PID_AUDPROC_STREAM_TOPOLOGY_ID;
		  result = AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST,
								  (uint8_t*) &nCmd, sizeof(AcdbDataGetTopIdCmdType),
								  (uint8_t*) &nRsp, sizeof(AcdbQueryResponseType));
	  }
	  else
	  {
		  nCmd.nParamId = pDataNode->ulParamId;
		  nCmd.pBufferPointer = pDataNode->ulDataBuf;
		  nRsp.nBytesUsedInBuffer = pDataNode->ulDataLen;
	  }


      if (result != ACDB_SUCCESS)
      {            
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopIdList]->Query for AudProc stream topologyId list failed!\n");            
      }
      else
      {
         if (pInput->nBufferLength >= nRsp.nBytesUsedInBuffer)
         {
            memcpy((void*)pInput->nBufferPointer, (void*)nCmd.pBufferPointer,
                    nRsp.nBytesUsedInBuffer);
            pOutput->nBytesUsedInBuffer = nRsp.nBytesUsedInBuffer;
         }
         else
         {
            result = ACDB_INSUFFICIENTMEMORY;               
			pOutput->nBytesUsedInBuffer = nRsp.nBytesUsedInBuffer;
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopIdList]->Insufficient buffer size to hold the data!\n");
         }// check whether buffer size big enough to hold the data
      }// check acdbdata_ioctl query return success or not 
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopIdList]->Query command request is completed\n");
#endif
   }// check input if it is null

   return result;
}

int32_t AcdbCmdGetAfeData (AcdbAfeDataCmdType *pInput,
                           AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAfeData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAfeData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      AcdbDataLookupKeyType key;
      AcdbDataAfeLookupKeyType indx;
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      AcdbDataGetTblTopType tbltop;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;     

      result = AcdbDataIoctl (ACDBDATA_GET_AFE_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }
      }

      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl (ACDBDATA_GET_AFE_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            uint32_t ulMaxParamSize = 0;
            result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                          pInput->nParamId,
                                          tbltop.pTopology,
                                          tbltop.nTopologyEntries,
                                          &ulPosition,
                                          &ulMaxParamSize);
            if (result == ACDB_SUCCESS)
            {
               ulDataType = ACDB_STATIC_DATA;
               ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

               if (ulDataLen > ulMaxParamSize)
               {
                  result = ACDB_BADSTATE;                  
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAFEData]->Data length greater than expected max. "
                                 "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                  
               }
            }
         }
      }

      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;            
			pOutput->nBytesUsedInBuffer = ulDataLen;
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAFEData]->Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);            
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                        pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {      
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAFEData]->Failed. TXD [0x%08X] RXD [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nModuleId, pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAFEData]->Query command request is completed. "
                     "TXD [0x%08X] RXD [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetAfeData (AcdbAfeDataCmdType *pInput
                           )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAfeData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAfeData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAfeLookupKeyType indx;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      
      result = AcdbDataIoctl (ACDBDATA_GET_AFE_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_AFE_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
                                   (AcdbDataLookupKeyType*) &key,
                                   (uint32_t*) &pInput->nModuleId,
                                   (uint32_t*) &pInput->nParamId,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer, 
                                   (uint32_t) pInput->nBufferLength, 
                                   NULL, 
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAFEData]->Failed. TXD [0x%08X] RXD [0x%08X] [MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nModuleId, pInput->nParamId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAFEData]->Set command request is completed. "
                     "TXD [0x%08X] RXD [0x%08X] [MID [0x%08X] PID [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetGlobalTable (AcdbGblTblCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetGlobalTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetGlobalTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      // Query global table lookup key
      AcdbDataGlbTblLookupKeyType indx;
      AcdbDataLookupKeyType key;

      indx.nModuleId = pInput->nModuleId;
      indx.nParamId = pInput->nParamId;

      result = AcdbDataIoctl (ACDBDATA_GET_GLOBAL_TABLE_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key)
                              );
         
      if (result == ACDB_SUCCESS)
      {// Query Global table
         AcdbDataGetGlbTblParamType param;
         result = AcdbDataIoctl (ACDBDATA_GET_GLOBAL_TABLE,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&param, sizeof (param)
                                 );
            
         if (result == ACDB_SUCCESS)
         {
            int32_t memcmpResult;
            
            if (pInput->nBufferLength == param.nParamSize)
            {// check if data length is equal to default data length
               memcmpResult = memcmp((void*)param.pParam,(void*)pInput->nBufferPointer,
                                     param.nParamSize);
            }
            else
            {
               memcmpResult = ACDB_PARMNOTFOUND;
            }
               
            if (memcmpResult != ACDB_SUCCESS)
            {// if data does not match to default,set Global Table cal data on heap
               result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_ADIE_TABLE,
                                      (AcdbDataLookupKeyType*) &key,
                                      NULL,NULL,NULL,
                                      (uint8_t*) pInput->nBufferPointer, 
                                      (uint32_t) pInput->nBufferLength,
                                      NULL,NULL
                                      );
            }
            else
            {// input data is equal to default, free the data node if it is exist on heap
               result = Acdb_ChecktoFreeAdieTableCalOnHeap((AcdbDataLookupKeyType*) &key,
                                                           (uint8_t*) pInput->nBufferPointer,
                                                           (uint32_t) pInput->nBufferLength
                                                           );
            }
         }
         else
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetGlobalTable]->Query Global Table Failed "
                           "ModuleID [0x%08X] ParamID [0x%08X]\n",pInput->nModuleId, pInput->nParamId);            
         }
      }         
      else
      {// Query Global table Lookup failed         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetGlobalTable]->Query Global Table Lookup Failed."
                        " ModuleID [0x%08X] ParamID [0x%08X]\n",pInput->nModuleId, pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetGlobalTable]->Set command request is completed."
                     " ModuleID [0x%08X] ParamID [0x%08X]\n",pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetGlobalDefs (AcdbGblDefsCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;
   int32_t topooverride=0;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetGlobalDefs]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetGlobalDefs]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
	   Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_OVERRIDE_STATUS,NULL,NULL,NULL,
	   NULL,(uint8_t*)&topooverride,sizeof(topooverride),NULL,NULL);
	   if(topooverride == TOPOLOGY_OVERRIDE_DEACTIVATED)
	   {
		   result = ACDB_SUCCESS;
		   return result;
	   }

	   switch(pInput->nParamId)
	   {
	   case ACDB_PID_DEVPAIR:
		   {
			   uint32_t noofdevpairs = 0;
			   uint32_t noofdevpairs_static = 0;
			   uint8_t *pTemp = NULL;
			   AcdbDataGeneralInfoType tbl;
			   result = AcdbDataIoctl (ACDBDATA_GET_DEV_PAIR_TABLE, NULL, 0,
									  (uint8_t *)&tbl, sizeof (tbl));
			  if(result != ACDB_SUCCESS)
			  {
				  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
				  {
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
				  }
			  }
			  else
			  {
				  memcpy(&noofdevpairs,pInput->nBufferPointer,sizeof(uint32_t));
				  noofdevpairs_static = tbl.nBufferLength/sizeof(AcdbDevicePairType);
				  if(noofdevpairs_static != noofdevpairs)
				  {
					  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
					  {
						  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
						  break;
					  }
				  }
				  else
				  {
					  pTemp = pInput->nBufferPointer + sizeof(uint32_t);
					  if(memcmp(pTemp,tbl.pBuffer,tbl.nBufferLength)!=0)
					  {
						  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
						  {
							  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
							  break;
						  }
					  }
				  }
			  }
		   }
		   break;
	   case ACDB_PID_AUD_VOL_STEPS:
		   {
			  uint32_t audvolsteps;
			  AcdbDataVolTblStepSizeType nVolStep;
			  memcpy(&audvolsteps,pInput->nBufferPointer,sizeof(audvolsteps));
			  result = AcdbDataIoctl (ACDBDATA_GET_VOL_TABLE_STEP_SIZE,
									  NULL, 0,
									  (uint8_t*)&nVolStep, sizeof(AcdbDataVolTblStepSizeType));
			  if(result != ACDB_SUCCESS)
			  {
				  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
				  {
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
				  }
			  }
			  if(audvolsteps != nVolStep.AudProcVolTblStepSize)
			  {
				  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
				  {
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
				  }
			  }
		   }
		   break;
	   case ACDB_PID_VOC_VOL_STEPS:
		   {
			  uint32_t vocvolsteps;
			  AcdbDataVolTblStepSizeType nVolStep;
			  memcpy(&vocvolsteps,pInput->nBufferPointer,sizeof(vocvolsteps));
			  result = AcdbDataIoctl (ACDBDATA_GET_VOL_TABLE_STEP_SIZE,
									  NULL, 0,
									  (uint8_t*)&nVolStep, sizeof(AcdbDataVolTblStepSizeType));
			  if(result != ACDB_SUCCESS)
			  {
				  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
				  {
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
				  }
			  }
			  if(vocvolsteps != nVolStep.VocProcVolTblStepSize)
			  {
				  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
				  {
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
				  }
			  }
		   }
		   break;
	   case ACDB_PID_ANCDEVPAIR:
		   {
			   uint32_t noofancdevpairs = 0;
			   uint32_t noofancdevpairs_static = 0;
			   uint8_t *pTemp = NULL;
			   AcdbDataGeneralInfoType tbl;
			   result = AcdbDataIoctl (ACDBDATA_GET_ANC_DEVICE_PAIR_TABLE, NULL, 0,
									  (uint8_t *)&tbl, sizeof (tbl));
			  if(result != ACDB_SUCCESS)
			  {
			  }
			  else
			  {
				  memcpy(&noofancdevpairs,pInput->nBufferPointer,sizeof(uint32_t));
				  noofancdevpairs_static = tbl.nBufferLength/sizeof(AcdbDevicePairType);
				  if(noofancdevpairs_static != noofancdevpairs)
				  {
					  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
					  {
						  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
						  break;
					  }
				  }
				  else
				  {
					  pTemp = pInput->nBufferPointer + sizeof(uint32_t);
					  if(memcmp(pTemp,tbl.pBuffer,tbl.nBufferLength)!=0)
					  {
						  if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
						  {
							  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
							  break;
						  }
					  }
				  }
			  }
		   }
		   break;
	   case ACDB_PID_AUDPROC_CMN_TOPID:
		   {
			   uint32_t nooftopids = 0;
			   uint32_t nooftopids_static = 0;
			   uint8_t *pTemp = NULL;
			   uint16_t i,j;
			   AcdbDataGetAudProcTopIdType *pStaticData=NULL;
			   AcdbDataGetAudProcTopIdType *pDynamicData=NULL;

				AcdbDataGetTopIdCmdType nCmd;
				AcdbQueryResponseType nRsp;
				AcdbDataLookupKeyType key;

				nCmd.nParamId = ACDBDATA_PID_AUDPROC_COMMON_TOPOLOGY_ID;

				result = AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST,
									 (uint8_t*) &nCmd, sizeof(AcdbDataGetTopIdCmdType),
									 (uint8_t*) &nRsp, sizeof(AcdbQueryResponseType));

				if(result != ACDB_SUCCESS)
				{
					if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
					{
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
					}
				}
				else
				{
					memcpy(&nooftopids,pInput->nBufferPointer,sizeof(uint32_t));
					nooftopids_static = nRsp.nBytesUsedInBuffer/(sizeof(AcdbDataGetAudProcTopIdType));
					if(nooftopids_static != nooftopids)
					{
						if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
						{
						  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
						  break;
						}
					}
					else
					{
						pTemp = pInput->nBufferPointer + sizeof(uint32_t);
						if(memcmp(pTemp,nCmd.pBufferPointer,nRsp.nBytesUsedInBuffer)!=0)
						{
							if( (topooverride == TOPOLOGY_OVERRIDE_STATUS_NOT_SET) || 
								(topooverride == TOPOLOGY_OVERRIDE_ACTIVATED))
							{
							  topooverride = TOPOLOGY_OVERRIDE_ACTIVATED;
							  
							  key.nLookupKey = ACDB_PID_AUDPROC_CMN_TOPID;
							  Acdb_DM_Ioctl(ACDB_SET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,pInput->nBufferPointer+sizeof(uint32_t),pInput->nBufferLength-sizeof(uint32_t),NULL,NULL);

							  // Now set the devices which has got the topology changes to them
							  pStaticData = (AcdbDataGetAudProcTopIdType *)nCmd.pBufferPointer;
							  pDynamicData = (AcdbDataGetAudProcTopIdType *)pTemp;
							  for(i=0;i<nooftopids;i++)
							  {
								  for(j=0;j<nooftopids;j++)
								  {
									  if(pStaticData[i].nDeviceId == pDynamicData[j].nDeviceId)
									  {
										  if( pStaticData[i].nAppTypeId == pDynamicData[j].nAppTypeId)
										  {
											  if((pStaticData[i].nTopoId != pDynamicData[j].nTopoId))
											  {
												  Acdb_DM_Ioctl(ACDB_SET_AUDTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,
													  (uint8_t *)&(pStaticData[i].nDeviceId),sizeof(uint32_t),NULL,NULL);
											  }
											  break;
										  }
									  }
								  }
							  }
							  break;
							}
						}
					}
				}
		   }
		   break;
	   case ACDB_PID_AUDPROC_STRM_TOPID:
		   {
			   uint32_t nooftopids = 0;
			   uint32_t nooftopids_static = 0;
			   uint8_t *pTemp = NULL;

				AcdbDataGetTopIdCmdType nCmd;
				AcdbQueryResponseType nRsp;
				AcdbDataLookupKeyType key;

				nCmd.nParamId = ACDBDATA_PID_AUDPROC_STREAM_TOPOLOGY_ID;

				result = AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST,
									 (uint8_t*) &nCmd, sizeof(AcdbDataGetTopIdCmdType),
									 (uint8_t*) &nRsp, sizeof(AcdbQueryResponseType));

				if(result != ACDB_SUCCESS)
				{
					if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
					{
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
					}
				}
				else
				{
					memcpy(&nooftopids,pInput->nBufferPointer,sizeof(uint32_t));
					nooftopids_static = nRsp.nBytesUsedInBuffer/(sizeof(AcdbDataGetAudStrmTopIdType));
					if(nooftopids_static != nooftopids)
					{
						if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
						{
						  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
						  break;
						}
					}
					else
					{
						pTemp = pInput->nBufferPointer + sizeof(uint32_t);
						if(memcmp(pTemp,nCmd.pBufferPointer,nRsp.nBytesUsedInBuffer)!=0)
						{
							if( (topooverride == TOPOLOGY_OVERRIDE_STATUS_NOT_SET) || 
								(topooverride == TOPOLOGY_OVERRIDE_ACTIVATED))
							{
							  topooverride = TOPOLOGY_OVERRIDE_ACTIVATED;
							  key.nLookupKey = ACDB_PID_AUDPROC_STRM_TOPID;
							  Acdb_DM_Ioctl(ACDB_SET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,pInput->nBufferPointer+sizeof(uint32_t),pInput->nBufferLength-sizeof(uint32_t),NULL,NULL);
							  break;
							}
						}
					}
				}
		   }
		   break;
	   case ACDB_PID_VOCPROC_CMN_TOPID:
		   {
			   uint32_t nooftopids = 0;
			   uint32_t nooftopids_static = 0;
			   uint8_t *pTemp = NULL;
			   uint16_t i,j;
			   AcdbDataGetVocProcTopIdType *pStaticData=NULL;
			   AcdbDataGetVocProcTopIdType *pDynamicData=NULL;

				AcdbDataGetTopIdCmdType nCmd;
				AcdbQueryResponseType nRsp;
				AcdbDataLookupKeyType key;

				nCmd.nParamId = ACDBDATA_PID_VOCPROC_COMMON_TOPOLOGY_ID;

				result = AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST,
									 (uint8_t*) &nCmd, sizeof(AcdbDataGetTopIdCmdType),
									 (uint8_t*) &nRsp, sizeof(AcdbQueryResponseType));

				if(result != ACDB_SUCCESS)
				{
					if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
					{
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
					}
				}
				else
				{
					memcpy(&nooftopids,pInput->nBufferPointer,sizeof(uint32_t));
					nooftopids_static = nRsp.nBytesUsedInBuffer/(sizeof(AcdbDataGetVocProcTopIdType));
					if(nooftopids_static != nooftopids)
					{
						if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
						{
						  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
						  break;
						}
					}
					else
					{
						pTemp = pInput->nBufferPointer + sizeof(uint32_t);
						if(memcmp(pTemp,nCmd.pBufferPointer,nRsp.nBytesUsedInBuffer)!=0)
						{
							if( (topooverride == TOPOLOGY_OVERRIDE_STATUS_NOT_SET) || 
								(topooverride == TOPOLOGY_OVERRIDE_ACTIVATED))
							{
							  topooverride = TOPOLOGY_OVERRIDE_ACTIVATED;
							  key.nLookupKey = ACDB_PID_VOCPROC_CMN_TOPID;
							  Acdb_DM_Ioctl(ACDB_SET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,pInput->nBufferPointer+sizeof(uint32_t),pInput->nBufferLength-sizeof(uint32_t),NULL,NULL);
							  // Now set the devices which has got the topology changes to them
							  pStaticData = (AcdbDataGetVocProcTopIdType *)nCmd.pBufferPointer;
							  pDynamicData = (AcdbDataGetVocProcTopIdType *)pTemp;
							  for(i=0;i<nooftopids;i++)
							  {
								  for(j=0;j<nooftopids;j++)
								  {
									  if(pStaticData[i].nDeviceId == pDynamicData[j].nDeviceId)
									  {
										  if(pStaticData[i].nTopoId != pDynamicData[j].nTopoId)
										  {
											  Acdb_DM_Ioctl(ACDB_SET_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,
												  (uint8_t *)&(pStaticData[i].nDeviceId),sizeof(uint32_t),NULL,NULL);
										  }
										  break;
									  }
								  }
							  }

							  break;
							}
						}
					}
				}
		   }
		   break;
	   case ACDB_PID_AFE_CMN_TOP_TABLE:
		   {
				uint32_t nooftopids = 0;
				uint32_t nooftopids_static = 0;
				uint8_t *pTemp = NULL;
			   uint16_t i,j;
			   AcdbDataGetAfeCmnTopIdType *pStaticData=NULL;
			   AcdbDataGetAfeCmnTopIdType *pDynamicData=NULL;

				AcdbDataGeneralInfoType tbl;
				AcdbDataLookupKeyType key;
				result = AcdbDataIoctl (ACDBDATA_CMD_GET_AFE_TOPOLOGY_LIST, NULL, 0,
									  (uint8_t *)&tbl, sizeof (tbl));

				if(result != ACDB_SUCCESS)
				{
					if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
					{
					  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
					  break;
					}
				}
				else
				{
					memcpy(&nooftopids,pInput->nBufferPointer,sizeof(uint32_t));
					nooftopids_static = tbl.nBufferLength/(sizeof(AcdbDataGetAfeCmnTopIdType));
					if(nooftopids_static != nooftopids)
					{
						if(topooverride != TOPOLOGY_OVERRIDE_DEACTIVATED)
						{
						  topooverride = TOPOLOGY_OVERRIDE_DEACTIVATED;
						  break;
						}
					}
					else
					{
						pTemp = pInput->nBufferPointer + sizeof(uint32_t);
						if(memcmp(pTemp,tbl.pBuffer,tbl.nBufferLength)!=0)
						{
							if( (topooverride == TOPOLOGY_OVERRIDE_STATUS_NOT_SET) || 
								(topooverride == TOPOLOGY_OVERRIDE_ACTIVATED))
							{
							  topooverride = TOPOLOGY_OVERRIDE_ACTIVATED;
							  key.nLookupKey = ACDB_PID_AFE_CMN_TOP_TABLE;
							  Acdb_DM_Ioctl(ACDB_SET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,pInput->nBufferPointer+sizeof(uint32_t),pInput->nBufferLength-sizeof(uint32_t),NULL,NULL);
							  // Now set the devices which has got the topology changes to them
							  pStaticData = (AcdbDataGetAfeCmnTopIdType *)tbl.pBuffer;
							  pDynamicData = (AcdbDataGetAfeCmnTopIdType *)pTemp;
							  for(i=0;i<nooftopids;i++)
							  {
								  for(j=0;j<nooftopids;j++)
								  {
									  if(pStaticData[i].nDeviceId == pDynamicData[j].nDeviceId)
									  {
										  if(pStaticData[i].nTopoId != pDynamicData[j].nTopoId)
										  {
											  Acdb_DM_Ioctl(ACDB_SET_AFECMNTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,
												  (uint8_t *)&(pStaticData[i].nDeviceId),sizeof(uint32_t),NULL,NULL);
										  }
										  break;
									  }
								  }
							  }

							  break;
							}
						}
					}
				}
		   }
		   break;
		case ACDB_PID_AUD_POPP_MODULELIST_GLBTBL:
		   {
			    int32_t topooverridesupported = TOPOLOGY_OVERRIDE_SUPPORTED;
				AcdbDataLookupKeyType key;
				  key.nLookupKey = ACDB_PID_AUD_POPP_MODULELIST_GLBTBL;
				  Acdb_DM_Ioctl(ACDB_SET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,pInput->nBufferPointer,pInput->nBufferLength,NULL,NULL);

				Acdb_DM_Ioctl(ACDB_SET_TOPOLOGY_OVERRIDE_SUPPORT,NULL,NULL,NULL,
				  NULL,(uint8_t*)&topooverridesupported,sizeof(topooverridesupported),NULL,NULL);

		   }
		   break;
	   };
		Acdb_DM_Ioctl(ACDB_SET_TOPOLOGY_OVERRIDE_STATUS,NULL,NULL,NULL,
		  NULL,(uint8_t*)&topooverride,sizeof(topooverride),NULL,NULL);
		if(topooverride == TOPOLOGY_OVERRIDE_DEACTIVATED)
		{
			int32_t topooverridesupported = TOPOLOGY_OVERRIDE_NOT_SUPPORTED;
			Acdb_DM_Ioctl(ACDB_CLEAR_TOPOLOGY_OVERRIDE_HEAP_DATA,NULL,NULL,NULL,
			  NULL,NULL,0,NULL,NULL);
			Acdb_DM_Ioctl(ACDB_SET_TOPOLOGY_OVERRIDE_SUPPORT,NULL,NULL,NULL,
				NULL,(uint8_t*)&topooverridesupported,sizeof(topooverridesupported),NULL,NULL);
		}

   }
   return result;
}

int32_t AcdbCmdGetGlobalTable (AcdbGblTblCmdType *pInput,
                               AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetGlobalTable]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetGlobalTable]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      // Query Global table Lookup
      AcdbDataLookupKeyType key;
      AcdbDataGlbTblLookupKeyType indx;

      indx.nModuleId = pInput->nModuleId;
      indx.nParamId = pInput->nParamId;

      result = AcdbDataIoctl (ACDBDATA_GET_GLOBAL_TABLE_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));

      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetGlobalTable]->Query Global table Lookup Failed."
                        " ModuleID [0x%08X] ParamID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);         
      }
      else
      {// Query Global table
         AcdbDataGetGlbTblParamType param;
         result = AcdbDataIoctl (ACDBDATA_GET_GLOBAL_TABLE,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&param, sizeof (param)
                                 );
         if (result != ACDB_SUCCESS)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetGlobalTable]->Query Global table Failed."
                           " ModuleID [0x%08X] ParamID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);            
         }
         else
         {
            AcdbDynamicUniqueDataType *pDataNode = NULL;

            // Query Global Table from Heap
            result = Acdb_DM_Ioctl(ACDB_GET_ADIE_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL,NULL,NULL,NULL,0,
                                   (uint8_t*) &pOutput->nBytesUsedInBuffer,
                                   (AcdbDynamicUniqueDataType**) &pDataNode
                                   );
            if (result == ACDB_SUCCESS && pDataNode != NULL)
            {//check pDataNode != NULL
               param.pParam = pDataNode->ulDataBuf;
               param.nParamSize = pDataNode->ulDataLen;
            }
             
            
            if (result == ACDB_PARMNOTFOUND || result == ACDB_SUCCESS)
            {// Lookup key index not found in heap
               if (pInput->nBufferLength < param.nParamSize)
               {
                  result = ACDB_INSUFFICIENTMEMORY;
				  pOutput->nBytesUsedInBuffer = param.nParamSize;
               }
               else
               {
                  result = ACDB_SUCCESS;
                  memcpy ((void *) pInput->nBufferPointer,
                          (const void *) param.pParam,
                          param.nParamSize);
                  pOutput->nBytesUsedInBuffer = param.nParamSize;
               }
            }
         }
         
         if (result != ACDB_SUCCESS)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetGlobalTable]->Failed. ModuleID [0x%08X] ParamId [0x%08X].\n",
                           pInput->nModuleId, pInput->nParamId);
         }
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetGlobalTable]->Query command request is completed. "
                     "ModuleID [0x%08X] ParamId [0x%08X].\n",
                     pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetCmnDeviceInfo (AcdbDeviceInfoCmnCmdType* pInput,
                                 AcdbQueryResponseType* pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetCmnDeviceInfo]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetCmnDeviceInfo]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGetTableType tbl;
      result = AcdbDataIoctl (ACDBDATA_GET_DEV_INFO_TABLE, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));

      if (result == ACDB_SUCCESS && NULL != tbl.pTable && 0 != tbl.nTableCount)
      {
         uint32_t i = 0;
         AcdbDataDeviceInfoType *pDevInfo = NULL;

         pDevInfo = (AcdbDataDeviceInfoType*) tbl.pTable;
         for (i = 0; i < tbl.nTableCount; ++i)
         {            
            if (pDevInfo->nDeviceId == pInput->nDeviceId)
            {
               if (pInput->nBufferLength >= pDevInfo->nDeviceInfoCmnPayloadSize)
               {
                  memcpy((void*)pInput->nBufferPointer, (void*)pDevInfo->pDeviceInfoCmnBufferPtr,
                         pDevInfo->nDeviceInfoCmnPayloadSize);
                  pOutput->nBytesUsedInBuffer = pDevInfo->nDeviceInfoCmnPayloadSize;
               }
               else
               {
                  ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetCmnDeviceInfo->Insufficient buffer size, Input buffer " 
                                 "size [%d], required buffer size [%d]",
                                 pInput->nBufferLength, pDevInfo->nDeviceInfoCmnPayloadSize);
                  result = ACDB_INSUFFICIENTMEMORY;
				  pOutput->nBytesUsedInBuffer = pDevInfo->nDeviceInfoCmnPayloadSize;

               }
               break;
            }
            else
            {
               pDevInfo ++;
            }
         }
         if (i == tbl.nTableCount)
         {
            result = ACDB_DEVICENOTFOUND;
            ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetCmnDeviceInfo->DID not found [0x%08X]\n", pInput->nDeviceId);
         }
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetCmnDeviceInfo->Query command request is completed, DID [0x%08X]\n", pInput->nDeviceId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetTSDeviceInfo (AcdbDeviceInfoTargetSpecificCmdType* pInput,
                                AcdbQueryResponseType* pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTSDeviceInfo]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->pBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTSDeviceInfo]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGetTableType tbl;
      result = AcdbDataIoctl (ACDBDATA_GET_DEV_INFO_TABLE, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));

      if (result == ACDB_SUCCESS && NULL != tbl.pTable && 0 != tbl.nTableCount)
      {
         uint32_t i = 0;
         AcdbDataDeviceInfoType *pDevInfo = NULL;

         pDevInfo = (AcdbDataDeviceInfoType*) tbl.pTable;
         for (i = 0; i < tbl.nTableCount; ++i)
         {            
            if (pDevInfo->nDeviceId == pInput->nDeviceId)
            {
               if (pInput->nBufferLength >= pDevInfo->nDeviceIfnoTSPayloadSize)
               {
                  memcpy((void*)pInput->pBufferPointer, (void*)pDevInfo->pDeviceInfoTSPBufferPtr,
                         pDevInfo->nDeviceIfnoTSPayloadSize);
                  pOutput->nBytesUsedInBuffer = pDevInfo->nDeviceIfnoTSPayloadSize;
               }
               else
               {
                  ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetTSDeviceInfo->Insufficient buffer size, Input buffer " 
                                 "size [%d], required buffer size [%d]",
                                 pInput->nBufferLength, pDevInfo->nDeviceIfnoTSPayloadSize);
                  result = ACDB_INSUFFICIENTMEMORY;
				  pOutput->nBytesUsedInBuffer = pDevInfo->nDeviceIfnoTSPayloadSize;

               }
               break;
            }
            else
            {
               pDevInfo ++;
            }
         }
         if (i == tbl.nTableCount)
         {
            result = ACDB_DEVICENOTFOUND;
            ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetTSDeviceInfo->DID not found [0x%08X]\n", pInput->nDeviceId);
         }
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetTSDeviceInfo->Query command request is completed, DID [0x%08X]\n", pInput->nDeviceId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetDeviceCapabilities(AcdbDeviceCapabilitiesCmdType *pInput,
                                     AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDeviceCapabilities]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDeviceCapabilities]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGetTableType tbl;
      result = AcdbDataIoctl (ACDBDATA_GET_DEV_INFO_TABLE, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));
      if (result == ACDB_SUCCESS && NULL != tbl.pTable && 0 != tbl.nTableCount)
      {
         if (pInput->nBufferLength < sizeof (uint32_t) +
             tbl.nTableCount * sizeof (AcdbDeviceCapabilityType))
         {
            result = ACDB_INSUFFICIENTMEMORY;
			pOutput->nBytesUsedInBuffer = sizeof (uint32_t) +
             tbl.nTableCount * sizeof (AcdbDeviceCapabilityType);
            ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetDeviceCapabilities->InsufficientMemory\n");
         }
         else
         {
            AcdbDataDeviceInfoType *pDevInfo = NULL;
            uint32_t nDeviceType = 0, i = 0;
            int32_t dstOffset = 0, srcOffset = 0;
            
            pDevInfo = (AcdbDataDeviceInfoType*) tbl.pTable;

            memcpy((void*)(pInput->nBufferPointer+dstOffset),(void*)&tbl.nTableCount,sizeof(uint32_t));
            dstOffset += sizeof(uint32_t);

            for (i = 0; i < tbl.nTableCount; ++i)
            {               
               //Get Device Type
               memcpy((void*)&nDeviceType,(void*)(pDevInfo[i].pDeviceInfoCmnBufferPtr+sizeof(uint32_t)),
                      sizeof(uint32_t));
               //Memcopy device Id
               memcpy((void*)(pInput->nBufferPointer+dstOffset),
                      (void*)&pDevInfo[i].nDeviceId,sizeof(uint32_t));
               dstOffset += sizeof(uint32_t);
               //Memcopy Sample rate mask
               srcOffset = acdb_devinfo_getSampleMaskOffset(nDeviceType);
               if (!srcOffset)
               {
                  ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetDeviceCapabilities->getSampleMaskOffset failed\n");
                  result = ACDB_BADSTATE;
                  break;
               }
               else
               {
                  memcpy((void*)(pInput->nBufferPointer+dstOffset),
                         (void*)(pDevInfo[i].pDeviceInfoTSPBufferPtr+srcOffset),sizeof(uint32_t));
                  dstOffset += sizeof(uint32_t);
               
                  //Memcopy Byte per sample rate mask
                  srcOffset = acdb_devinfo_getBytesPerSampleMaskOffset(nDeviceType);
                  if (!srcOffset)
                  {
                     ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetDeviceCapabilities->getBytesPerSampleMaskOffset failed\n");
                     result = ACDB_BADSTATE;
                     break;
                  }
                  else
                  {
                     memcpy((void*)(pInput->nBufferPointer+dstOffset),
                            (void*)(pDevInfo[i].pDeviceInfoTSPBufferPtr+srcOffset),sizeof(uint32_t));
                     dstOffset += sizeof(uint32_t);
                  }
               }
            }
            if (result == ACDB_SUCCESS)
            {
               pOutput->nBytesUsedInBuffer = tbl.nTableCount * sizeof (AcdbDeviceCapabilityType) + sizeof(uint32_t);
            }
         }
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDeviceCapabilities]->Query command request is completed"
                     "ParamID [0x%08X].\n", pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetVocProcCmnTopId (AcdbGetVocProcTopIdCmdType *pInput,
                                   AcdbGetTopologyIdRspType *pOutput
                                   )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcCmnTopId]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      AcdbDataGetVocProcCmnTopIdCmdType nCmd;
	  AcdbDataGetTopologyIdRspType nRsp = {0};

      nCmd.nDeviceId = pInput->nDeviceId;

	  if( (Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
		 (Acdb_DM_Ioctl(ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&nCmd.nDeviceId,sizeof(nCmd.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
		 )
	  {
		 AcdbDynamicUniqueDataType *pDataNode = NULL;
		 AcdbDataLookupKeyType key;
		 key.nLookupKey = ACDB_PID_VOCPROC_CMN_TOPID;
		 result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,NULL,0,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
		 if(result == ACDB_SUCCESS)
		 {
			 uint32_t i = 0;
			 uint32_t nSize = pDataNode->ulDataLen / sizeof(AcdbDataGetVocProcTopIdType);
			 AcdbDataGetVocProcTopIdType *pVocTopTbl = (AcdbDataGetVocProcTopIdType *)pDataNode->ulDataBuf;
			 for(i=0;i<nSize;i++)
			 {
				 if(pVocTopTbl->nDeviceId == nCmd.nDeviceId)
				 {
					 nRsp.nTopologyId = pVocTopTbl->nTopoId;
					 break;
				 }
				 pVocTopTbl++;
			 }
		 }
		 else
		 {
			 result = ACDB_ERROR;
		 }
	  }
	  else
	  {
      result = AcdbDataIoctl (ACDBDATA_GET_VOCPROC_CMN_TOPOLOGY_ID,
                              (uint8_t*) &nCmd, sizeof(AcdbDataGetVocProcCmnTopIdCmdType),
                              (uint8_t*) &nRsp, sizeof(AcdbDataGetTopologyIdRspType)
                              );
	  }
      if (result == ACDB_SUCCESS)
      {
         pOutput->nTopologyId = nRsp.nTopologyId;
      }
      else
      {
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcCmnTopId]->Query for VocProc Common topologyId failed! "
                        "DeviceId [0x%08X]\n",nCmd.nDeviceId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcCmnTopId]->Query command request is completed "
                     "DeviceId [0x%08X]\n",nCmd.nDeviceId);
#endif
   }

   return result;
}

int32_t AcdbCmdGetVocProcCmnTopIdList (AcdbGeneralInfoCmdType *pInput,
                                       AcdbQueryResponseType *pOutput
                                       )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcCmnTopIdList]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcCmnTopIdList]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGetTopIdCmdType nCmd;
      AcdbQueryResponseType nRsp;

	  AcdbDynamicUniqueDataType *pDataNode = NULL;
	  AcdbDataLookupKeyType key;
		 
	  key.nLookupKey = ACDB_PID_VOCPROC_CMN_TOPID;
	  if(Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS)
	  {
	      result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,NULL,NULL,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
	  }
	  else
	  {
		  result = ACDB_NOTSUPPORTED;
	  }
	  if(result != ACDB_SUCCESS)
	  {
		 nCmd.nParamId = ACDBDATA_PID_VOCPROC_COMMON_TOPOLOGY_ID;
		 result = AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST,
			 (uint8_t*) &nCmd, sizeof(AcdbDataGetTopIdCmdType),
			 (uint8_t*) &nRsp, sizeof(AcdbQueryResponseType));
	  }
	  else
	  {
		 nCmd.nParamId = pDataNode->ulParamId;
		 nCmd.pBufferPointer = pDataNode->ulDataBuf;
		 nRsp.nBytesUsedInBuffer = pDataNode->ulDataLen;
	  }
      if (result != ACDB_SUCCESS)
      {            
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcCmnTopIdList]->Query for VocProc common topologyId list failed!\n");            
      }
      else
      {
         if (pInput->nBufferLength >= nRsp.nBytesUsedInBuffer)
         {
            memcpy((void*)pInput->nBufferPointer, (void*)nCmd.pBufferPointer,
               nRsp.nBytesUsedInBuffer);
            pOutput->nBytesUsedInBuffer = nRsp.nBytesUsedInBuffer;
         }
         else
         {
            result = ACDB_BADPARM;               
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcCmnTopIdList]->Insufficient buffer size to hold the data!\n");               
         }// check whether buffer size big enough to hold the data
      }// check acdbdata_ioctl query return success or not 
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcCmnTopIdList]->Query command request is completed\n");
#endif
   }
   
   return result;
}

int32_t AcdbCmdGetQACTInfo (AcdbGeneralInfoCmdType *pInput,
                            AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbGeneralInfoCmdType]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbGeneralInfoCmdType]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGetGlbTblParamType InfoBuf;

      result = AcdbDataIoctl(ACDBDATA_GET_QACT_DATA,
                             NULL,0,
                             (uint8_t*)&InfoBuf, sizeof(AcdbDataGetGlbTblParamType));

      if (result == ACDB_SUCCESS)
      {
         if (InfoBuf.nParamSize > pInput->nBufferLength)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetQACTInfo]->Insufficient Buffer Length, InputBuffer length %d, Actual Data length %d\n",
                           pInput->nBufferLength,InfoBuf.nParamSize);            
            result = ACDB_INSUFFICIENTMEMORY;
			pOutput->nBytesUsedInBuffer = InfoBuf.nParamSize;

         }
         else
         {
            memcpy((void*)pInput->nBufferPointer,(void*)InfoBuf.pParam,InfoBuf.nParamSize);
            pOutput->nBytesUsedInBuffer = InfoBuf.nParamSize;
         }
      }
      else
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetQACTInfo]->Failed to query QACT Info\n");         
         result = ACDB_BADPARM;
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetQACTInfo]->Query command request is completed\n");
#endif
   }

   return result;
}

int32_t AcdbCmdGetAfeCmnData (AcdbAfeCmnDataCmdType *pInput,
                           AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL ||
         pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      AcdbDataLookupKeyType key;
      AcdbDataAfeCmnLookupKeyType indx;
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      AcdbDataGetTblTopType tbltop;
      uint32_t nSRId = 0;

	  indx.nDeviceId = pInput->nDeviceId;
	  
	  // Translate sample rate Id
	  result = acdb_translate_sample_rate ((uint32_t) pInput->nAfeSampleRateId,(uint32_t*) &nSRId);
	  if (result == ACDB_SUCCESS)
	  {
	  	indx.nAfeSampleRateId = nSRId;
	  }
	  else
	  {
	        indx.nAfeSampleRateId = pInput->nAfeSampleRateId;     
	  }
	  

      result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }
      }

      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            uint32_t ulMaxParamSize = 0;
            result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                          pInput->nParamId,
                                          tbltop.pTopology,
                                          tbltop.nTopologyEntries,
                                          &ulPosition,
                                          &ulMaxParamSize);
            if (result == ACDB_SUCCESS)
            {
               ulDataType = ACDB_STATIC_DATA;
               ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

               if (ulDataLen > ulMaxParamSize)
               {
                  result = ACDB_BADSTATE;                  
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAFECmnData]->Data length greater than expected max. "
                                 "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                  
               }
            }
         }
      }

      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;            
			pOutput->nBytesUsedInBuffer = ulDataLen;

            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAFECmnData]->Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);            
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                        pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {      
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAFECmnData]->Failed. DID [0x%08X] SRID [0x%08X] MID [0x%08X] PID [0x%08X].\n",
			 pInput->nDeviceId, pInput->nAfeSampleRateId, pInput->nModuleId, pInput->nParamId);      
      }
   }
   return result;
}

int32_t AcdbCmdSetAfeCmnData (AcdbAfeCmnDataCmdType *pInput
                           )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAfeCmnLookupKeyType indx;
      uint32_t nSRId = 0;

	  indx.nDeviceId = pInput->nDeviceId;
	  
	  // Translate sample rate Id
	  result = acdb_translate_sample_rate ((uint32_t) pInput->nAfeSampleRateId,(uint32_t*) &nSRId);
	  if (result == ACDB_SUCCESS)
	  {
	  	indx.nAfeSampleRateId = nSRId;
	  }
	  else
	  {
	        indx.nAfeSampleRateId = pInput->nAfeSampleRateId;
	  }
      
      result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 (Acdb_DM_Ioctl(ACDB_GET_IS_AFECMNTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 )
		 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
		 }
		 else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&tbltop, sizeof (tbltop));
		 }
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
                                   (AcdbDataLookupKeyType*) &key,
                                   (uint32_t*) &pInput->nModuleId,
                                   (uint32_t*) &pInput->nParamId,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer, 
                                   (uint32_t) pInput->nBufferLength, 
                                   NULL, 
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAFECmnData]->Failed. DIE [0x%08X] SRID [0x%08X] [MID [0x%08X] PID [0x%08X].\n",
			 pInput->nDeviceId, pInput->nAfeSampleRateId, pInput->nModuleId, pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdGetAfeCmnTable (AcdbAfeCommonTableCmdType *pInput,
                                AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL ||
         pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      AcdbDataLookupKeyType key;
	  AcdbDataAfeCmnLookupKeyType indx;
      uint32_t nSRId = 0;

      indx.nDeviceId = pInput->nDeviceId;
	  
	  // Translate sample rate Id
	  result = acdb_translate_sample_rate ((uint32_t) pInput->nSampleRateId,(uint32_t*) &nSRId);
	  if (result == ACDB_SUCCESS)
	  {
	  	indx.nAfeSampleRateId = nSRId;
	  }
	  else
	  {
	        indx.nAfeSampleRateId = pInput->nSampleRateId;
	  }
      
      result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));

      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 (Acdb_DM_Ioctl(ACDB_GET_IS_AFECMNTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 )
		 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
		 }
		 else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&tbltop, sizeof (tbltop));
		 }
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   (uint8_t*) &pOutput->nBytesUsedInBuffer,
                                   NULL
                                   );
         }
      }

      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAfeCmnTable]->failed. "
                         "DID [0x%08X] DSR [0x%08X].\n",
						 pInput->nDeviceId, pInput->nSampleRateId);         
      }
   }
   return result;
}
int32_t AcdbCmdGetCompRemoteDevId (AcdbGetRmtCompDevIdCmdType *pInput,
                                   AcdbGetRmtCompDevIdRspType *pOutput
                                   )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetCompRemoteDevId]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else
   {
      AcdbDataGetRmtCompDevIdCmdType nCmd;
      AcdbDataGetRmtCompDevIdRspTyp nRsp;

	  nCmd.nNativeDeviceId = pInput->nNativeDeviceId;

      result = AcdbDataIoctl (ACDBDATA_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID,
                              (uint8_t*) &nCmd, sizeof(AcdbDataGetRmtCompDevIdCmdType),
                              (uint8_t*) &nRsp, sizeof(AcdbDataGetRmtCompDevIdRspTyp)
                              );
      if (result == ACDB_SUCCESS)
      {
		  pOutput->nRmtDeviceId = nRsp.nRemoteDeviceId;
      }
	  else if(result== ACDB_PARMNOTFOUND)
		{
			 ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetCompRemoteDevId]-> Remote dev id not found for"
			 "Native DeviceId [0x%08X]\n",nCmd.nNativeDeviceId);
		}
      else
      {
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetCompRemoteDevId]->Query for compatible Remote dev id failed! "
			 "Native DeviceId [0x%08X]\n",nCmd.nNativeDeviceId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetCompRemoteDevId]->Query command request is completed "
                     "Native DeviceId [0x%08X]\n",nCmd.nNativeDeviceId);
#endif
   }

   return result;
}
int32_t AcdbCmdGetRecordPairList (AcdbGeneralInfoCmdType *pInput,
                                    AcdbQueryResponseType *pOutput) 
                                   
{

int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordPairList]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordPairList]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataGeneralInfoType tbl;

      result = AcdbDataIoctl(ACDB_CMD_GET_AUDIO_RECORD_PAIR_LIST,
                             NULL, 0,
                             (uint8_t*) &tbl, sizeof(tbl)
                             );
      if (result == ACDB_SUCCESS)
      {
         if (tbl.nBufferLength > pInput->nBufferLength)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordPairList]->Insufficient Buffer Length,"
                           " InputBuffer length %d, Actual table length %d\n",
                           pInput->nBufferLength, tbl.nBufferLength);            
            result = ACDB_BADPARM;
         }
         else
         {
            memcpy((void*) pInput->nBufferPointer,(void*) tbl.pBuffer,tbl.nBufferLength);
            pOutput->nBytesUsedInBuffer = tbl.nBufferLength;
         }
      }
      else
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordPairList]->Failed to query  Device Pair List\n");         
         result = ACDB_BADPARM;
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordPairList]->Query command request is completed\n");
#endif
   }
  return result;
}
int32_t AcdbCmdGetRecordRxDeviceList (AcdbAudioRecRxListCmdType *pInput,
                                    AcdbAudioRecRxListRspType *pOutput) 
                                   
{

int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordRxDeviceList]->System Erorr\n");
      result = ACDB_BADSTATE;
   }   
   else
   {
	  AcdbAudioRecRxListCmdType nCmd;
      AcdbAudioRecRxListRspType nRsp = {0};

	  nCmd.nTxDeviceId = pInput->nTxDeviceId;

      result = AcdbDataIoctl (ACDB_CMD_GET_AUDIO_RECORD_RX_DEVICE_LIST,
                              (uint8_t*) &nCmd, sizeof(AcdbAudioRecRxListCmdType),
                              (uint8_t*) pOutput, sizeof(AcdbAudioRecRxListRspType)
                              );
      if (result == ACDB_BADPARM)
      {
		  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordRxDeviceList]-> BadParam for"
			 "Tx Device id [0x%08X]\n",nCmd.nTxDeviceId);
      }
	  else if(result== ACDB_PARMNOTFOUND)
		{
			 ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordRxDeviceList]-> Audio EC Rx dev id not found for"
			 "Tx Device id [0x%08X]\n",nCmd.nTxDeviceId);
		}
      else
      {
		  //success
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordRxDeviceList]->Query for Audio EC Rx dev id success! "
			 "Tx Device id [0x%08X]\n",nCmd.nTxDeviceId);
      }

#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetRecordRxDeviceList]->Query command request is completed\n");
#endif
   }
  return result;
   
}
   
int32_t AcdbCmdSetAfeCmnTable (AcdbAfeCommonTableCmdType *pInput
                                )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput != NULL)
   {
	   AcdbDataAfeCmnLookupKeyType indx;
      AcdbDataLookupKeyType key;
      uint32_t nSRId = 0;

      indx.nDeviceId = pInput->nDeviceId;
	  
	  // Translate sample rate Id
	  result = acdb_translate_sample_rate ((uint32_t) pInput->nSampleRateId,(uint32_t*) &nSRId);
	  if (result == ACDB_SUCCESS)
	  {
	  	indx.nAfeSampleRateId = nSRId;
	  }
	  else
	  {
	        indx.nAfeSampleRateId = pInput->nSampleRateId;
	  }

      result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key)
                              );

      if (result == ACDB_SUCCESS)
      {
         uint32_t i = 0;
         AcdbDataGetTblTopType tbltop;
		 if((Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS) &&
			 (Acdb_DM_Ioctl(ACDB_GET_IS_AFECMNTOPO_CHANGE_DEVICE,NULL,NULL,NULL,NULL,(uint8_t *)&indx.nDeviceId,sizeof(indx.nDeviceId),NULL,NULL) == ACDB_SUCCESS)
			 )
		 {
			 tbltop.nTableEntries = 0;
			 tbltop.nTopologyEntries = 0;
			 tbltop.ppTable = NULL;
			 tbltop.pTopology = NULL;
			 result = ACDB_SUCCESS;
		 }
		 else
		 {
			 result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY,
									 (uint8_t *)&key, sizeof (key),
									 (uint8_t *)&tbltop, sizeof (tbltop)
									 );
		 }

         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL, 
                                   (AcdbDataGetTblTopType*) &tbltop, 
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   NULL,
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAfeCmnTable]->Failed. "
                         "DID [0x%08X] DSR [0x%08X] .\n",
						 pInput->nDeviceId, pInput->nSampleRateId);         
      }
   }
   return result;
}


int32_t AcdbCmdGetAfeTopIDList (AcdbGeneralInfoCmdType *pInput,
                                  AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {

      AcdbDataGeneralInfoType tbl;
	  AcdbDynamicUniqueDataType *pDataNode = NULL;
	  AcdbDataLookupKeyType key;
		 
	  key.nLookupKey = ACDB_PID_AFE_CMN_TOP_TABLE;
	  if(Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS)
	  {
	      result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,NULL,NULL,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
	  }
	  else
	  {
		  result = ACDB_NOTSUPPORTED;
	  }
	  if(result != ACDB_SUCCESS)
	  {

	      result = AcdbDataIoctl (ACDBDATA_CMD_GET_AFE_TOPOLOGY_LIST, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));
	  }
	  else
	  {
		  tbl.pBuffer = pDataNode->ulDataBuf;
		  tbl.nBufferLength = pDataNode->ulDataLen;
	  }

      if (result == ACDB_SUCCESS && tbl.pBuffer != NULL && tbl.nBufferLength != 0)
      {
         if (pInput->nBufferLength >= tbl.nBufferLength)
         {
            memcpy((void*)(*pInput).nBufferPointer,(void*)tbl.pBuffer,tbl.nBufferLength);
            pOutput->nBytesUsedInBuffer = tbl.nBufferLength;
         }
         else
         {
            result = ACDB_BADPARM;            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDevicePairList]->Insufficient buffer to hold the data");            
         }
      }
   }

   return result;
}
int32_t AcdbCmdGetDeviceChannelTypeList (AcdbGeneralInfoCmdType *pInput,
                                  AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      AcdbDataGeneralInfoType tbl;
      result = AcdbDataIoctl (ACDBDATA_CMD_GET_DEVICE_CHANNEL_TYPE_LIST, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));

      if (result == ACDB_SUCCESS && tbl.pBuffer != NULL && tbl.nBufferLength != 0)
      {
         if (pInput->nBufferLength >= tbl.nBufferLength)
         {
            memcpy((void*)(*pInput).nBufferPointer,(void*)tbl.pBuffer,tbl.nBufferLength);
            pOutput->nBytesUsedInBuffer = tbl.nBufferLength;
         }
         else
         {
            result = ACDB_BADPARM;            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDeviceChannelTypeList]->Insufficient buffer to hold the data");            
         }
      }
   }

   return result;
}

int32_t AcdbCmdGetAudcalPoppModIDsList (AcdbGeneralInfoCmdType *pInput,
                                  AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {

      AcdbDataGeneralInfoType tbl;
	  AcdbDynamicUniqueDataType *pDataNode = NULL;
	  AcdbDataLookupKeyType key;
		 
	  key.nLookupKey = ACDB_PID_AUD_POPP_MODULELIST_GLBTBL;
	  if(Acdb_IsTopologyOverrideSupported() == ACDB_SUCCESS)
	  {
	      result = Acdb_DM_Ioctl(ACDB_GET_TOPOLOGY_LIST,&key,NULL,NULL,NULL,NULL,NULL,NULL,(AcdbDynamicUniqueDataType **)&pDataNode);
	  }
	  else
	  {
		  result = ACDB_NOTSUPPORTED;
	  }
	  if(result != ACDB_SUCCESS)
	  {

	      result = AcdbDataIoctl (ACDBDATA_CMD_GET_AUD_VOL_POPP_MODULE_LIST, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));
	  }
	  else
	  {
		  tbl.pBuffer = pDataNode->ulDataBuf;
		  tbl.nBufferLength = pDataNode->ulDataLen;
	  }

      if (result == ACDB_SUCCESS && tbl.pBuffer != NULL && tbl.nBufferLength != 0)
      {
         if (pInput->nBufferLength >= tbl.nBufferLength)
         {
            memcpy((void*)(*pInput).nBufferPointer,(void*)tbl.pBuffer,tbl.nBufferLength);
            pOutput->nBytesUsedInBuffer = tbl.nBufferLength;
         }
         else
         {
            result = ACDB_BADPARM;            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDevicePairList]->Insufficient buffer to hold the data");            
         }
      }
   }

   return result;
}


int32_t AcdbCmdGetAdieSidetoneTable (AcdbAdieSidetoneTableCmdType *pInput,
                                AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL ||
         pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      AcdbDataLookupKeyType key;
	  AcdbDataAdieSidetoneLookupKeyType indx;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      
      result = AcdbDataIoctl (ACDBDATA_GET_ADIE_SIDETONE_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));

      if (result == ACDB_SUCCESS)
      {
          AcdbDataGetTblTopType tbltop;
          result = AcdbDataIoctl (ACDBDATA_GET_ADIE_SIDETONE_TABLE_AND_TOPOLOGY,
              (uint8_t *)&key, sizeof (key),
              (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
                                   (AcdbDataLookupKeyType*) &key,
                                   NULL, 
                                   NULL,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer,
                                   (uint32_t) pInput->nBufferLength,
                                   (uint8_t*) &pOutput->nBytesUsedInBuffer,
                                   NULL
                                   );
         }
      }

      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieSidetoneTable]->failed. "
                         "DTX [0x%08X] DRX [0x%08X].\n",
                         pInput->nTxDeviceId, pInput->nRxDeviceId);         
      }
   }
   return result;
}

int32_t AcdbCmdGetAdieSidetoneData (AcdbAdieSidetoneDataCmdType *pInput,
                           AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieSidetoneData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieSidetoneData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      uint8_t ulDataType = ACDB_HEAP_DATA_NOT_FOUND;  //2 == Static, 1 == Dynamic, 0 == Data not found on static and heap
      uint32_t ulDataLen = 0;
      uint32_t ulPosition = 0; 

      AcdbDataLookupKeyType key;
      AcdbDataAdieSidetoneLookupKeyType indx;
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      AcdbDataGetTblTopType tbltop;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;     

      result = AcdbDataIoctl (ACDBDATA_GET_ADIE_SIDETONE_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
                                (AcdbDataLookupKeyType*) &key,
                                (uint32_t*) &pInput->nModuleId,
                                (uint32_t*) &pInput->nParamId,
                                NULL, 
                                NULL, 
                                0, 
                                NULL, 
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {
            ulDataType = ACDB_DYNAMIC_DATA;
            ulDataLen = pDataNode->ulDataLen;
         }
      }

      if (result == ACDB_PARMNOTFOUND)
      {
         result = AcdbDataIoctl (ACDBDATA_GET_ADIE_SIDETONE_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            uint32_t ulMaxParamSize = 0;
            result = AcdbCmdGetDataIndex (pInput->nModuleId,
                                          pInput->nParamId,
                                          tbltop.pTopology,
                                          tbltop.nTopologyEntries,
                                          &ulPosition,
                                          &ulMaxParamSize);
            if (result == ACDB_SUCCESS)
            {
               ulDataType = ACDB_STATIC_DATA;
               ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

               if (ulDataLen > ulMaxParamSize)
               {
                  result = ACDB_BADSTATE;                  
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieSidetoneData]->Data length greater than expected max. "
                                 "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);                  
               }
            }
         }
      }

      if (result == ACDB_SUCCESS)
      {
         if (ulDataLen > pInput->nBufferLength)
         {
            result = ACDB_INSUFFICIENTMEMORY;
			pOutput->nBytesUsedInBuffer = ulDataLen;           
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieSidetoneData]->Data length greater than provided buffer. "
                           "MID [0x%08X] PID [0x%08X].\n",pInput->nModuleId, pInput->nParamId);            
         }
         else
         {
            if (ulDataType == ACDB_STATIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)tbltop.ppTable [ulPosition]->pUniqueData,
                       tbltop.ppTable [ulPosition]->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = tbltop.ppTable [ulPosition]->ulDataLen;
            }
            else if(ulDataType == ACDB_DYNAMIC_DATA)
            {
               memcpy ((void *) pInput->nBufferPointer,
                       (const void *)pDataNode->ulDataBuf,
                        pDataNode->ulDataLen);
               result = ACDB_SUCCESS;
               pOutput->nBytesUsedInBuffer = pDataNode->ulDataLen;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
      }
      if (result != ACDB_SUCCESS)
      {      
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieSidetoneData]->Failed. TXD [0x%08X] RXD [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nModuleId, pInput->nParamId);
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieSidetoneData]->Query command request is completed. "
                     "TXD [0x%08X] RXD [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

int32_t AcdbCmdSetAdieSidetoneData (AcdbAdieSidetoneDataCmdType *pInput
                           )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieSidetoneData]->System Erorr\n");
      result = ACDB_BADSTATE;
   }
   else if (pInput->nBufferPointer == NULL)
   {
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieSidetoneData]->NULL pointer\n");
      result = ACDB_NULLPOINTER;
   }
   else
   {
      AcdbDataLookupKeyType key;
      AcdbDataAdieSidetoneLookupKeyType indx;

      indx.nTxDeviceId = pInput->nTxDeviceId;
      indx.nRxDeviceId = pInput->nRxDeviceId;
      
      result = AcdbDataIoctl (ACDBDATA_GET_ADIE_SIDETONE_DATA_LOOKUP_KEY,
                              (uint8_t *)&indx, sizeof (indx),
                              (uint8_t *)&key, sizeof (key));
      if (result == ACDB_SUCCESS)
      {
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_ADIE_SIDETONE_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop));
         if (result == ACDB_SUCCESS)
         {
            result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
                                   (AcdbDataLookupKeyType*) &key,
                                   (uint32_t*) &pInput->nModuleId,
                                   (uint32_t*) &pInput->nParamId,
                                   (AcdbDataGetTblTopType*) &tbltop,
                                   (uint8_t*) pInput->nBufferPointer, 
                                   (uint32_t) pInput->nBufferLength, 
                                   NULL, 
                                   NULL
                                   );
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieSidetoneData]->Failed. TXD [0x%08X] RXD [0x%08X] [MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nModuleId, pInput->nParamId);         
      }
#ifdef ACDB_DEBUG_L1_LOG
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieSidetoneData]->Set command request is completed. "
                     "TXD [0x%08X] RXD [0x%08X] [MID [0x%08X] PID [0x%08X].\n",
                     pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nModuleId, pInput->nParamId);
#endif
   }

   return result;
}

