/*===========================================================================
    FILE:           acdb_command.c

    OVERVIEW:       This file contains the implementaion of the helper methods
                    used to service ACDB ioctls.

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

static int32_t AcdbCmdSerializeQdsp6Table (AcdbDataUniqueDataNodeType **ppCalTbl,
                                           uint32_t szCalTbl,
                                           AcdbDataTopologyType* pTopology,
                                           uint32_t ulTopologySize,
                                           uint8_t* pDst,
                                           uint32_t ulDstSize,
                                           uint32_t* pBytesUsed)
{
   int32_t result = ACDB_SUCCESS;

   if (szCalTbl != ulTopologySize)
   {
      result = ACDB_ERROR;
   }
   else
   {
      uint32_t i = 0;
      uint32_t offset = 0;
      uint8_t* pDstEnd = pDst + ulDstSize;
      for (; i < ulTopologySize && result == ACDB_SUCCESS; ++i)
      {
         const uint32_t data_size = 3*sizeof (uint32_t) + ppCalTbl[i]->ulDataLen + ppCalTbl[i]->ulDataLen % 4;
         if (pDstEnd < pDst + offset + data_size)
         {
            result = ACDB_INSUFFICIENTMEMORY;
         }
         else
         {
            memcpy ((void *)(pDst + offset), (const void *) &pTopology[i].ulModuleId, sizeof (uint32_t));
            memcpy ((void *)(pDst + offset +  4), (const void *) &pTopology[i].ulParamId, sizeof (uint32_t));
            memcpy ((void *)(pDst + offset +  8), (const void *) &(ppCalTbl[i]->ulDataLen), sizeof (uint16_t));
            memset ((void *)(pDst + offset + 10), 0, sizeof (uint16_t));
            memcpy ((void *)(pDst + offset + 12), (const void *) ppCalTbl[i]->pUniqueData, ppCalTbl[i]->ulDataLen);
            offset += data_size;
         }
      }
      *pBytesUsed = offset;
   }
   return result;
}

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

   // Verify if the ACDBDATA target version is equivalent to the ACDBSW target
   // version and verify if the ACDBDATA data structure version is compatible
   // with the implementation in this file. The expectation is minor version
   // changes should NOT affect the code in this file.
   
   // If a mismatch occurs, fail initialization.
   (void) AcdbDataIoctl (ACDBDATA_GET_ACDB_VERSION, NULL, 0, (uint8_t *)&modvers, sizeof (modvers));
   (void) AcdbDataIoctl (ACDBDATA_GET_TARGET_VERSION, NULL, 0, (uint8_t *)&tgtvers, sizeof (tgtvers));

   // Check the Target Version first to protect against a mismatch between
   // Product Lines (where the likelihood of a data structure change is high).
   if (tgtvers.targetversion != ACDB_CURRENT_TARGET_VERSION)
   {
      result = ACDB_MISMATCH_TARGET_VERSION;
      ACDB_DEBUG_LOG("[ACDB Command]->Target Version Mismatch between Data and Command\n");
   }
   else if (modvers.major != ACDBDATA_CURRENT_DATA_STRUCTURE_VERSION_MAJOR)
   {
      result = ACDB_MISMATCH_DATA_SW_VERSION;
      ACDB_DEBUG_LOG("[ACDB Command]->Data Structure Version Mismatch between Data and Command\n");
   }
   else
   {
      (void) acdb_init ();
   }

   return result;
}

int32_t AcdbCmdGetAcdbSwVersion (AcdbModuleVersionType *pOutput)
{
   int32_t result = ACDB_SUCCESS;
   uint16_t major = ACDB_SOFTWARE_VERSION_MAJOR;
   uint16_t minor = ACDB_SOFTWARE_VERSION_MINOR;
   if (pOutput != NULL)
   {
      memcpy(&pOutput->major,&major,sizeof(pOutput->major));
      memcpy(&pOutput->minor,&minor,sizeof(pOutput->minor));
   }
   else
   {
      result = ACDB_BADPARM;
   }

   return result;
}

int32_t AcdbCmdGetTargetVersion (AcdbTargetVersionType *pOutput)
{
   int32_t result = ACDB_SUCCESS;
   uint32_t nTargetVersion = ACDB_CURRENT_TARGET_VERSION;

   if (pOutput != NULL)
   {
      memcpy(&pOutput->targetversion,&nTargetVersion,sizeof(uint32_t));
   }
   else
   {
      result = ACDB_BADPARM;
   }
   
   return result;
}

int32_t AcdbCmdGetDeviceInfo (AcdbDeviceInfoCmdType* pInput,
                              AcdbQueryResponseType* pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      AcdbDataGetTableType tbl;

      tbl.nParamId = ACDB_DEVICE_INFO_PARAM;
      result = AcdbDataIoctl (ACDBDATA_GET_DEV_INFO_TABLE, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));

      if (result == ACDB_SUCCESS && NULL != tbl.pTable && 0 != tbl.nTableCount)
      {
         uint32_t i = 0;
         uint32_t nBufferSize = 0;

         switch (tbl.nParamId)
         {
         case ACDB_DEVICE_INFO_PARAM4:
            {
               AcdbDataDeviceInfo4Type *pDevInfoTbl = (AcdbDataDeviceInfo4Type *)tbl.pTable;

               for (; i < tbl.nTableCount; ++i)
               {
                  if (pInput->nDeviceId == pDevInfoTbl[i].ulDeviceId)
                  {
                     break;
                  }
               }
               if (i != tbl.nTableCount)
               {
                  result = acdb_deviceinfo_translation ((uint32_t) pInput->nParamId,
                                                        (uint32_t) tbl.nParamId,
                                                        (uint8_t*) pDevInfoTbl[i].pDeviceInfo,
                                                        (uint8_t*) pInput->nBufferPointer,
                                                        (uint32_t) pInput->nBufferLength,
                                                        (uint32_t*)&pOutput->nBytesUsedInBuffer
                                                        );
               }
               break;
            }
         case ACDB_DEVICE_INFO_PARAM3:
            {
               AcdbDataDeviceInfo3Type *pDevInfoTbl = (AcdbDataDeviceInfo3Type *)tbl.pTable;

               for (; i < tbl.nTableCount; ++i)
               {
                  if (pInput->nDeviceId == pDevInfoTbl[i].ulDeviceId)
                  {
                     break;
                  }
               }
               if (i != tbl.nTableCount)
               {
                  result = acdb_deviceinfo_translation ((uint32_t) pInput->nParamId,
                                                        (uint32_t) tbl.nParamId,
                                                        (uint8_t*) pDevInfoTbl[i].pDeviceInfo,
                                                        (uint8_t*) pInput->nBufferPointer,
                                                        (uint32_t) pInput->nBufferLength,
                                                        (uint32_t*)&pOutput->nBytesUsedInBuffer
                                                        );
               }
               break;
            }
         case ACDB_DEVICE_INFO_PARAM2:
            {
               AcdbDataDeviceInfo2Type *pDevInfoTbl = (AcdbDataDeviceInfo2Type *)tbl.pTable;

               for (; i < tbl.nTableCount; ++i)
               {
                  if (pInput->nDeviceId == pDevInfoTbl[i].ulDeviceId)
                  {
                     break;
                  }
               }
               if (i != tbl.nTableCount)
               {
                  result = acdb_deviceinfo_translation ((uint32_t) pInput->nParamId,
                                                        (uint32_t) tbl.nParamId,
                                                        (uint8_t*) pDevInfoTbl[i].pDeviceInfo,
                                                        (uint8_t*) pInput->nBufferPointer,
                                                        (uint32_t) pInput->nBufferLength,
                                                        (uint32_t*)&pOutput->nBytesUsedInBuffer
                                                        );
               }
               break;
            }
         case ACDB_DEVICE_INFO_PARAM:
            {
               AcdbDataDeviceInfoType *pDevInfoTbl = (AcdbDataDeviceInfoType *)tbl.pTable;

               for (; i < tbl.nTableCount; ++i)
               {
                  if (pInput->nDeviceId == pDevInfoTbl[i].ulDeviceId)
                  {
                     break;
                  }
               }
               if (i != tbl.nTableCount)
               {
                  result = acdb_deviceinfo_translation ((uint32_t) pInput->nParamId,
                                                        (uint32_t) tbl.nParamId,
                                                        (uint8_t*) pDevInfoTbl[i].pDeviceInfo,
                                                        (uint8_t*) pInput->nBufferPointer,
                                                        (uint32_t) pInput->nBufferLength,
                                                        (uint32_t*)&pOutput->nBytesUsedInBuffer
                                                        );
               }
               break;
            }
         }

         if (i == tbl.nTableCount)
         {
            result = ACDB_DEVICENOTFOUND;
            ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetDeviceInfo->DID not found [0x%08X]\n", pInput->nDeviceId);
         }
      }
   }
   return result;
}

int32_t AcdbCmdGetDeviceCapabilities(AcdbDeviceCapabilitiesCmdType *pInput,
                                     AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      AcdbDataGetTableType tbl;
      tbl.nParamId = ACDB_DEVICE_INFO_PARAM;
      result = AcdbDataIoctl (ACDBDATA_GET_DEV_INFO_TABLE, NULL, 0,
                              (uint8_t *)&tbl, sizeof (tbl));
      if (result == ACDB_SUCCESS && NULL != tbl.pTable && 0 != tbl.nTableCount)
      {
         if (pInput->nBufferLength < sizeof (uint32_t) +
             tbl.nTableCount * sizeof (AcdbDeviceCapabilityType))
         {
            result = ACDB_INSUFFICIENTMEMORY;
         }
         else
         {
            uint8_t* pDst = pInput->nBufferPointer;
            uint32_t i = 0;

            switch (tbl.nParamId)
            {
            case ACDB_DEVICE_INFO_PARAM4:
               {
                  AcdbDataDeviceInfo4Type *pDevInfoTbl = (AcdbDataDeviceInfo4Type *)tbl.pTable;

                  memcpy ((void*) pDst, (const void*) &tbl.nTableCount, sizeof (uint32_t));
                  pDst += sizeof (uint32_t);
               
                  for (; i < tbl.nTableCount; ++i)
                  {
                     memcpy ((void *) pDst, (const void *) &pDevInfoTbl[i].ulDeviceId, sizeof (uint32_t));
                     pDst += sizeof (uint32_t);

                     memcpy ((void *) pDst, (const void *) &pDevInfoTbl[i].pDeviceInfo->ulSampleRateMask,sizeof (uint32_t));
                     pDst += sizeof (uint32_t);

                     memcpy ((void *) pDst, (const void *) &pDevInfoTbl[i].pDeviceInfo->ulAfeBytesPerSampleMask,sizeof (uint32_t));
                     pDst += sizeof (uint32_t);
                  }
               }
               break;
            case ACDB_DEVICE_INFO_PARAM3:
               {
                  AcdbDataDeviceInfo3Type *pDevInfoTbl = (AcdbDataDeviceInfo3Type *)tbl.pTable;

                  memcpy ((void*) pDst, (const void*) &tbl.nTableCount, sizeof (uint32_t));
                  pDst += sizeof (uint32_t);
               
                  for (; i < tbl.nTableCount; ++i)
                  {
                     memcpy ((void *) pDst, (const void *) &pDevInfoTbl[i].ulDeviceId, sizeof (uint32_t));
                     pDst += sizeof (uint32_t);

                     memcpy ((void *) pDst,(const void *) &pDevInfoTbl[i].pDeviceInfo->ulSampleRateMask,sizeof (uint32_t));
                     pDst += sizeof (uint32_t);

                     memcpy ((void *) pDst,(const void *) &pDevInfoTbl[i].pDeviceInfo->ulAfeBytesPerSampleMask,sizeof (uint32_t));
                     pDst += sizeof (uint32_t);
                  }
               }
               break;
            case ACDB_DEVICE_INFO_PARAM2:
               {
                  AcdbDataDeviceInfo2Type *pDevInfoTbl = (AcdbDataDeviceInfo2Type *)tbl.pTable;

                  memcpy ((void*) pDst, (const void*) &tbl.nTableCount, sizeof (uint32_t));
                  pDst += sizeof (uint32_t);
               
                  for (; i < tbl.nTableCount; ++i)
                  {
                     memcpy ((void *) pDst, (const void *) &pDevInfoTbl[i].ulDeviceId, sizeof (uint32_t));
                     pDst += sizeof (uint32_t);

                     memcpy ((void *) pDst,(const void *) &pDevInfoTbl[i].pDeviceInfo->ulSampleRateMask,sizeof (uint32_t));
                     pDst += sizeof (uint32_t);

                     memcpy ((void *) pDst,(const void *) &pDevInfoTbl[i].pDeviceInfo->ulAfeBytesPerSampleMask,sizeof (uint32_t));
                     pDst += sizeof (uint32_t);
                  }
               }
               break;
            case ACDB_DEVICE_INFO_PARAM:
               {
                  AcdbDataDeviceInfoType *pDevInfoTbl = (AcdbDataDeviceInfoType *)tbl.pTable;

                  memcpy ((void*) pDst, (const void*) &tbl.nTableCount, sizeof (uint32_t));
                  pDst += sizeof (uint32_t);
               
                  for (; i < tbl.nTableCount; ++i)
                  {
                     memcpy ((void *) pDst, (const void *) &pDevInfoTbl[i].ulDeviceId, sizeof (uint32_t));
                     pDst += sizeof (uint32_t);

                     memcpy ((void *) pDst,(const void *) &pDevInfoTbl[i].pDeviceInfo->ulSampleRateMask,sizeof (uint32_t));
                     pDst += sizeof (uint32_t);

                     memcpy ((void *) pDst,(const void *) &pDevInfoTbl[i].pDeviceInfo->ulAfeBytesPerSampleMask,sizeof (uint32_t));
                     pDst += sizeof (uint32_t);
                  }
               }
               break;
            default:
               result = ACDB_BADPARM;
               break;
            }
            pOutput->nBytesUsedInBuffer = sizeof (uint32_t) + tbl.nTableCount *
                                          sizeof (AcdbDeviceCapabilityType);
         }
      }      
   }

   return result;
}

int32_t AcdbCmdGetDevicePair (AcdbDevicePairType *pInput,
                              AcdbDevicePairingResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
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
   }

   return result;
}

int32_t AcdbCmdGetDevicePairList (AcdbGeneralInfoCmdType *pInput,
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
   }

   return result;
}

int32_t AcdbCmdGetAudProcTable (AcdbAudProcTableCmdType *pInput,
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
      AcdbDataAudProcCmnLookupKeyType indx;
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
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY,
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
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcTable]->failed. "
                         "DID [0x%08X] DSR [0x%08X] APT [0x%08X].\n",
                         pInput->nDeviceId, pInput->nDeviceSampleRateId,
                         pInput->nApplicationType);         
      }
   }
   return result;
}

int32_t AcdbCmdSetAudProcTable (AcdbAudProcTableCmdType *pInput
                                )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput != NULL)
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
         uint32_t i = 0;
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop)
                                 );

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
   }
   return result;
}

int32_t AcdbCmdGetAudProcData (AcdbAudProcCmdType *pInput,
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
   }
   return result;
}

int32_t AcdbCmdSetAudProcData (AcdbAudProcCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      int32_t result = ACDB_SUCCESS;

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
         result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_CMN_TABLE_AND_TOPOLOGY,
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
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAudProcData]->Failed. "
                        "DID [0x%08X] DSR [0x%08X] APT [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nDeviceSampleRateId, pInput->nApplicationType,
                        pInput->nModuleId, pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdGetAudStrmTable (AcdbAudStrmTableCmdType *pInput,
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
   }
   return result;
}

int32_t AcdbCmdSetAudStrmTable (AcdbAudStrmTableCmdType *pInput
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
   }
   return result;
}

int32_t AcdbCmdGetAudStrmData (AcdbAudStrmCmdType *pInput,
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
   }
   return result;
}

int32_t AcdbCmdSetAudStrmData (AcdbAudStrmCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {
      result = ACDB_BADPARM;
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
   }
   return result;
}

int32_t AcdbCmdGetVocProcTable (AcdbVocProcTableCmdType *pInput,
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
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY,
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
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcTable]->Failed. "
                        "TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X] RXSR [0x%08X] NID [0x%08X] VSR [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId,
                        pInput->nTxDeviceSampleRateId, pInput->nRxDeviceSampleRateId,
                        pInput->nNetworkId, pInput->nVocProcSampleRateId);         
      }
   }
   return result;
}

int32_t AcdbCmdSetVocProcTable (AcdbVocProcTableCmdType *pInput
                                )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput != NULL)
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
         uint32_t i = 0;
         AcdbDataGetTblTopType tbltop;
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop)
                                 );
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
   }
   return result;
}

int32_t AcdbCmdGetVocProcData (AcdbVocProcCmdType *pInput,
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
   }   
   return result;
}

int32_t AcdbCmdSetVocProcData (AcdbVocProcCmdType *pInput
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
         result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_CMN_TABLE_AND_TOPOLOGY,
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
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcData]->Failed. TXD [0x%08X] RXD [0x%08X] TXSR [0x%08X]"
                        " RXSR [0x%08X] NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nTxDeviceSampleRateId, 
                        pInput->nRxDeviceSampleRateId, pInput->nNetworkId, pInput->nVocProcSampleRateId,
                        pInput->nModuleId, pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdGetVocStrmTable (AcdbVocStrmTableCmdType *pInput,
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
   }

   return result;
}

int32_t AcdbCmdSetVocStrmTable (AcdbVocStrmTableCmdType *pInput
                                )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput != NULL)
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
   }
   return result;
}

int32_t AcdbCmdGetVocStrmData (AcdbVocStrmCmdType *pInput,
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
                  ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcStreamData]->Data length greater than expected max. "
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
            ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdGetVocProcStreamData]->Data length greater than provided buffer. "
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
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocStreamData]->Failed. "
                        "NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nNetworkId, pInput->nVocProcSampleRateId,
                        pInput->nModuleId, pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdSetVocStrmData (AcdbVocStrmCmdType *pInput
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
         ACDB_DEBUG_LOG("[ACDB Command]->AcdbCmdSetVocStreamData]->Failed. "
                        "NID [0x%08X] VSR [0x%08X] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nNetworkId, pInput->nVocProcSampleRateId,
                        pInput->nModuleId, pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdGetVocProcGainDepVolTable (AcdbVocProcVolTblCmdType *pInput,
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
            result = ACDB_BADPARM;            
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
                     result = ACDB_BADPARM;                     
                     ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcGainDepVolTable]->Insufficient buffer size\n");                     
                     break;
                  }
                  else
                  {
                     //Table size
                     dst_offset += sizeof(uint32_t);
                     
                     if (nTableSize+dst_offset > pInput->nBufferLength)
                     {
                        result = ACDB_BADPARM;                        
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
         pOutput->nBytesUsedInBuffer = dst_offset;
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcVolumeTable]->Failed. "
                        "TXDID [0x%08X] RXDID [0x%08X] NID [0x%08X] VSR [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                        pInput->nVocProcSampleRateId);         
      }
   }
   return result;
}

int32_t AcdbCmdSetVocProcGainDepData (AcdbVocProcVolTblDataCmdType *pInput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
      {
         result = ACDB_BADPARM;
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
            result = AcdbDataIoctl (ACDBDATA_GET_VOC_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
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
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetVocProcVolCalData]->Failed. TXDID [0x%08X] RXDID [0x%08X]"
                        " NID [0x%08X] VSR [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                        pInput->nVocProcSampleRateId, pInput->nVolumeIndex, pInput->nModuleId,
                        pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdGetVocProcGainDepData (AcdbVocProcVolTblDataCmdType *pInput,
                                      AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
      {
         result = ACDB_BADPARM;
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
                     ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcVolStepData]->Data length greater than expected max. "
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
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcVolStepData]->Data length greater than provided buffer. "
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
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetVocProcVolCalData]->Failed. TXDID [0x%08X] RXDID [0x%08X] NID [0x%08X]"
                        " VSR [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nTxDeviceId, pInput->nRxDeviceId, pInput->nNetworkId, 
                        pInput->nVocProcSampleRateId, pInput->nVolumeIndex, pInput->nModuleId,
                        pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdSetAudProcGainDepData (AcdbAudProcVolTblDataCmdType *pInput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
      {
         result = ACDB_BADPARM;
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
         ACDB_DEBUG_LOG("[ACDB] Command]->[AcdbCmdSetAudProcVolCalData]->Failed. "
                        "DID [0x%08X] APPID [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex,
                        pInput->nModuleId, pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdGetAudProcGainDepData (AcdbAudProcVolTblDataCmdType *pInput,
                                      AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
      {
         result = ACDB_BADPARM;
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
                  ulDataLen = tbltop.ppTable [ulPosition]->ulDataLen;

                  if (ulDataLen > ulMaxParamSize)
                  {
                     result = ACDB_BADSTATE;                     
                     ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcVolStepData]->Data length greater than expected max. "
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
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcVolStepData]->Data length greater than provided buffer. "
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
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcVolCalData]->Failed. "
                        "DID [0x%08X] APPID [0x%08X] VolInd [%d] MID [0x%08X] PID [0x%08X].\n",
                        pInput->nDeviceId, pInput->nApplicationType, pInput->nVolumeIndex,
                        pInput->nModuleId, pInput->nParamId);         
      }
   }
   return result;
}

int32_t AcdbCmdGetAudProcGainDepVolTable (AcdbAudProcVolTblCmdType *pInput,
                                          AcdbAudProcGainDepVolTblRspType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (pInput->nCoppBufferPointer == NULL || pInput->nCoppBufferLength == 0 ||
          pInput->nPoppBufferPointer == NULL || pInput->nPoppBufferLength == 0)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbDataVolTblStepSizeType nVolStep;
         result = AcdbDataIoctl (ACDBDATA_GET_VOL_TABLE_STEP_SIZE,
                                 NULL, 0,
                                 (uint8_t*)&nVolStep, sizeof(AcdbDataVolTblStepSizeType));
         
         if (result != ACDB_SUCCESS)
         {            
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepVolTable]->"
                           "[ACDBDATA_GET_VOL_TABLE_STEP_SIZE] Failed.\n");            
         }
         else
         {
            uint32_t dstCopp_offset = 0;
            uint32_t dstPopp_offset = 0;
            uint32_t i = 0;

            //number of volume step to COPP
            if (dstCopp_offset+sizeof(uint32_t) > pInput->nCoppBufferLength)
            {
               result = ACDB_BADPARM;               
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepVolTable]->Insufficient buffer size\n");                              
            }
            else
            {
               memcpy ((void*) (pInput->nCoppBufferPointer+dstCopp_offset),
                       (const void *) (&nVolStep.AudProcVolTblStepSize), sizeof(uint32_t));
               dstCopp_offset += sizeof(uint32_t);
            }
            //number of volume step to POPP
            if (dstPopp_offset+sizeof(uint32_t) > pInput->nPoppBufferLength
                && result == ACDB_SUCCESS)
            {
               result = ACDB_BADPARM;               
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepVolTable]->Insufficient buffer size\n");                              
            }
            else
            {
               memcpy ((void*) (pInput->nPoppBufferPointer+dstPopp_offset),
                       (const void *) (&nVolStep.AudProcVolTblStepSize), sizeof(uint32_t));
               dstPopp_offset += sizeof(uint32_t);
            }
            //Get Volume data and gain dependent calibration Data
            for (; i < nVolStep.AudProcVolTblStepSize && result == ACDB_SUCCESS; ++i)
            {
               AcdbDataLookupKeyType key;
               AcdbDataAudProcVolLookupKeyType indx;
               indx.nDeviceId = pInput->nDeviceId;
               indx.nApplicationType = pInput->nApplicationType;
               indx.nVolIndex = i;

               result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_LOOKUP_KEY,
                                       (uint8_t *)&indx, sizeof (indx),
                                       (uint8_t *)&key, sizeof (key));

               //Query Volume Gain
               if (result == ACDB_SUCCESS)
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

                     if (dstPopp_offset+sizeof(uint32_t) > pInput->nPoppBufferLength)
                     {
                        result = ACDB_BADPARM;                        
                        ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepVolTable]->Insufficient buffer size\n");                          
                     }
                     else
                     {
                        //Table Size
                        dstPopp_offset += sizeof(uint32_t);

                        if ((dstPopp_offset + nTblSize) > pInput->nPoppBufferLength)
                        {
                           result = ACDB_BADPARM;                           
                           ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepVolTable]->Insufficient buffer size\n");                            
                        }
                        else
                        {
                           uint32_t nBytesUsedInBuffer;
                           result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
                                                  (AcdbDataLookupKeyType*) &key,
                                                  NULL, 
                                                  NULL,
                                                  (AcdbDataGetTblTopType*) &nTblCal.popp,
                                                  (uint8_t*) pInput->nPoppBufferPointer+dstPopp_offset,
                                                  (uint32_t) pInput->nPoppBufferLength-dstPopp_offset,
                                                  (uint8_t*) &nBytesUsedInBuffer,
                                                  NULL
                                                  );
                           //Table size
                           memcpy ((void*) (pInput->nPoppBufferPointer+dstPopp_offset-sizeof(uint32_t)),
                                   (const void *) (&nBytesUsedInBuffer), sizeof(uint32_t));
                           if (nBytesUsedInBuffer)
                           {
                              dstPopp_offset += nBytesUsedInBuffer;
                           }
                           else
                           {
                              dstPopp_offset -= dstPopp_offset;
                           }
                        }
                     }
                  }

                  if (result == ACDB_SUCCESS && &nTblCal.copp != NULL)
                  {
                     uint32_t j = 0;
                     uint32_t nTblSize = 0;

                     for (; j < nTblCal.copp.nTopologyEntries; ++j)
                     {
                        nTblSize += nTblCal.copp.ppTable[j]->ulDataLen + nTblCal.copp.ppTable[j]->ulDataLen%4
                                    + 3*sizeof(uint32_t);
                     }
                     if (dstCopp_offset+nTblSize > pInput->nCoppBufferLength)
                     {
                        result = ACDB_BADPARM;                        
                        ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepVolTable]->Insufficient buffer size\n");                         
                     }
                     else
                     {
                        //Table Size
                        dstCopp_offset += sizeof(uint32_t);

                        if ((dstCopp_offset + nTblSize) > pInput->nCoppBufferLength)
                        {
                           result = ACDB_BADPARM;                           
                           ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcGainDepVolTable]->Insufficient buffer size\n");                           
                        }
                        else
                        {
                           uint32_t nBytesUsedInBuffer;
                           result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_TABLE,
                                                  (AcdbDataLookupKeyType*) &key,
                                                  NULL, 
                                                  NULL,
                                                  (AcdbDataGetTblTopType*) &nTblCal.copp,
                                                  (uint8_t*) pInput->nCoppBufferPointer+dstCopp_offset,
                                                  (uint32_t) pInput->nCoppBufferLength-dstCopp_offset,
                                                  (uint8_t*) &nBytesUsedInBuffer,
                                                  NULL
                                                  );
                           //Table size
                           memcpy ((void*) (pInput->nCoppBufferPointer + dstCopp_offset-sizeof(uint32_t)),
                                   (const void *) (&nBytesUsedInBuffer), sizeof(uint32_t));
                           if (nBytesUsedInBuffer)
                           {
                              dstCopp_offset += nBytesUsedInBuffer;
                           }
                           else
                           {
                              dstCopp_offset -= dstCopp_offset;
                           }
                        }
                     }
                  }
               }
            }
            pOutput->nBytesUsedCoppBuffer = dstCopp_offset;
            pOutput->nBytesUsedPoppBuffer = dstPopp_offset;
         }
      }
      if (result != ACDB_SUCCESS)
      {         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcVolumeTable]->Failed. "
                        "DID [0x%08X], APPID [0x%08X]\n",pInput->nDeviceId, pInput->nApplicationType);         
      }
   }
   return result;
}

int32_t AcdbCmdGetAudProcGainDepCoppTableStep (AcdbAudProcGainDepVolTblStepCmdType *pInput,
                                               AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      
      if (pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
      {
         result = ACDB_BADPARM;
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
                  result = ACDB_BADPARM;                  
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
   }
   return result;
}

int32_t AcdbCmdGetAudProcGainDepPoppTableStep (AcdbAudProcGainDepVolTblStepCmdType *pInput,
                                               AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      
      if (pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
      {
         result = ACDB_BADPARM;
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
            AcdbDataGetAudProcVolTblTopType nTblCal;
            result = AcdbDataIoctl (ACDBDATA_GET_AUD_PROC_VOL_GAIN_DEP_TABLE_AND_TOPOLOGY,
                                    (uint8_t *)&key, sizeof (key),
                                    (uint8_t *)&nTblCal, sizeof (nTblCal));
               
            if (result == ACDB_SUCCESS && &nTblCal.copp != NULL)
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
                  result = ACDB_BADPARM;                  
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
   }
   return result;
}

int32_t AcdbCmdGetVolTableStepSize (AcdbVolTblStepSizeRspType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pOutput == NULL)
   {
      result = ACDB_BADPARM;
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
   }
   return result;
}

int32_t AcdbCmdGetMemoryUsage (AcdbMemoryUsageType* pOutput)
{
   int32_t result = ACDB_SUCCESS;

   AcdbDataMemoryUsageType rom;
   AcdbDataMemoryUsageType ram;

   AcdbDataIoctl (ACDBDATA_ESTIMATE_MEMORY_USE, NULL, 0,  (uint8_t *)&rom, sizeof (rom));

   pOutput->org_ROM = rom.org_ROM;
   pOutput->data_ROM = rom.data_ROM;

   AcdbDataMemoryRamEstimate ((AcdbDataMemoryUsageType *) &ram);

   pOutput->org_RAM = ram.org_ROM;
   pOutput->data_RAM = ram.data_ROM;

   return result;
}

int32_t AcdbCmdSystemReset(void)
{
   int32_t result = ACDB_BADPARM;

   result = Acdb_DM_Ioctl(ACDB_SYS_RESET,
                          NULL,NULL,NULL,NULL,
                          NULL,0,NULL,NULL);

   return result;
}

int32_t AcdbCmdGetOEMInfo (AcdbGeneralInfoCmdType *pInput,
                           AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetOEMInfo]->NULL input for querying OEM Info\n");      
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
   }

   return result;
}

int32_t AcdbCmdSetOEMInfo (AcdbGeneralInfoCmdType *pInput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferLength == 0)
   {
      result = ACDB_BADPARM;
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

   return result;
}

int32_t AcdbCmdGetDateInfo (AcdbGeneralInfoCmdType *pInput,
                            AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferLength == 0 || pOutput == NULL)
   {
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetDateInfo]->NULL input for querying Date Info\n");
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
   }

   return result;
}

int32_t AcdbCmdSetDateInfo (AcdbGeneralInfoCmdType *pInput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferLength == 0)
   {
      result = ACDB_BADPARM;
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

   return result;
}

int32_t AcdbCmdGetANCDevicePair (AcdbAncDevicePairCmdType *pInput,
                                 AcdbAncDevicePairRspType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;
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
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetDateInfo]->Failed to query ANC Tx Device, RXDID [0x%08X]\n",
                        pInput->nRxDeviceId);         
         result = ACDB_BADPARM;
      }
   }

   return result;
}

int32_t AcdbCmdGetANCDevicePairList (AcdbGeneralInfoCmdType *pInput,
                                     AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pOutput == NULL)
   {
      result = ACDB_BADPARM;
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
   }

   return result;
}

int32_t AcdbCmdSetAdieProfileTable (AcdbAdiePathProfileCmdType *pInput
                                   )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {// check if input is NULL
      result = ACDB_BADPARM;
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieProfileTable]->NULL Input!\n");      
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
               uint32_t memcmpResult;

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
   }
   return result;
}

int32_t AcdbCmdGetAdieProfileTable (AcdbAdiePathProfileCmdType *pInput,
                                    AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL ||
         pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {//Check if input is NULL
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->NULL Input!\n");
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
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieProfileTable]->Failed. CodecPathId [0x%08X] ParamId [0x%08X].\n",
                           pInput->ulCodecPathId, pInput->nParamId);            
         }
      }
   }

   return result;
}

int32_t AcdbCmdSetAdieANCDataTable (AcdbANCSettingCmdType *pInput
                                    )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {// check if input is NULL
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetAdieANCDataTable]->NULL Input!\n");      
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
               uint32_t memcmpResult;

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
   }
   return result;
}

int32_t AcdbCmdGetAdieANCDataTable (AcdbANCSettingCmdType *pInput,
                                    AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL ||
         pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {//Check if input is NULL
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->NULL Input!\n");      
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
            ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAdieANCDataTable]->Failed. RxDID [0x%08X] ParamId [0x%08X].\n",
                           pInput->nRxDeviceId, pInput->nParamId);            
         }
      }
   }

   return result;
}

int32_t AcdbCmdGetLookupTableSize (AcdbGetTableSizeCmdType *pInput,
                                   AcdbGetTableSizeRspType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL)
   {
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetLookupTableSize]->NULL Input!\n");      
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
      result = ACDB_BADPARM;
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetTableLookupIndex]->NULL Input!\n");      
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
                  uint32_t nAvailableByteToCopy = nTblIndexRsp.nAvailableByteToCopy;
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
                  uint32_t nAvailableByteToCopy = nTblIndexRsp.nAvailableByteToCopy;
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
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopId]->NULL Input!\n");      
   }
   else
   {
      AcdbDataGetAudProcCmnTopIdCmdType nCmd;
      AcdbDataGetTopologyIdRspType nRsp;

      nCmd.nDeviceId = pInput->nDeviceId;
      nCmd.nAppTypeId = pInput->nApplicationType;

      result = AcdbDataIoctl (ACDBDATA_GET_AUDPROC_CMN_TOPOLOGY_ID,
                              (uint8_t*) &nCmd, sizeof(AcdbDataGetAudProcCmnTopIdCmdType),
                              (uint8_t*) &nRsp, sizeof(AcdbDataGetTopologyIdRspType)
                              );
      if (result == ACDB_SUCCESS)
      {
         pOutput->nTopologyId = nRsp.nTopologyId;
      }
      else
      {        
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopId]->Query for AudProc common topologyId failed! "
                        "DID [0x%08X] ApptypeID [0x%08X]\n",nCmd.nDeviceId, nCmd.nAppTypeId);         
      }
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
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopId]->NULL Input!\n");      
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
      result = ACDB_BADPARM;
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopIdList]->NULL Input!\n");      
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

         nCmd.nParamId = ACDBDATA_PID_AUDPROC_COMMON_TOPOLOGY_ID;

         result = AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST,
                                 (uint8_t*) &nCmd, sizeof(AcdbDataGetTopIdCmdType),
                                 (uint8_t*) &nRsp, sizeof(AcdbQueryResponseType));
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
               result = ACDB_BADPARM;               
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcCmnTopIdList]->Insufficient buffer size to hold the data!\n");               
            }// check whether buffer size big enough to hold the data
         }// check acdbdata_ioctl query return success or not 
      }// check buffer if it is null
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
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopIdList]->NULL Input!\n");      
   }
   else
   {
      if (pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
      {
         result = ACDB_BADPARM;         
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopIdList]->NULL Input!\n");         
      }
      else
      {
         AcdbDataGetTopIdCmdType nCmd;
         AcdbQueryResponseType nRsp;

         nCmd.nParamId = ACDBDATA_PID_AUDPROC_STREAM_TOPOLOGY_ID;

         result = AcdbDataIoctl (ACDBDATA_GET_TOPOLOGY_ID_LIST,
                                 (uint8_t*) &nCmd, sizeof(AcdbDataGetTopIdCmdType),
                                 (uint8_t*) &nRsp, sizeof(AcdbQueryResponseType));
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
               result = ACDB_BADPARM;               
               ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAudProcStrmTopIdList]->Insufficient buffer size to hold the data!\n");
               
            }// check whether buffer size big enough to hold the data
         }// check acdbdata_ioctl query return success or not 
      }// check buffer if it is null
   }// check input if it is null

   return result;
}

int32_t AcdbCmdGetAfeData (AcdbAfeDataCmdType *pInput,
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
      AcdbDataAfeLookupKeyType indx;
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      AcdbDataGetTblTopType tbltop;
      uint32_t nSRId = 0;

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
   }
   return result;
}

int32_t AcdbCmdSetAfeData (AcdbAfeDataCmdType *pInput
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
      AcdbDataAfeLookupKeyType indx;
      uint32_t nSRId = 0;

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
         result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY,
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
         result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY,
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
         ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetAfeCmnTable]->failed. "
                         "DID [0x%08X] DSR [0x%08X].\n",
						 pInput->nDeviceId, pInput->nSampleRateId);         
      }
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
         result = AcdbDataIoctl (ACDBDATA_GET_AFECMN_TABLE_AND_TOPOLOGY,
                                 (uint8_t *)&key, sizeof (key),
                                 (uint8_t *)&tbltop, sizeof (tbltop)
                                 );

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
      result = AcdbDataIoctl (ACDBDATA_CMD_GET_AFE_TOPOLOGY_LIST, NULL, 0,
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
   }

   return result;
}




int32_t AcdbCmdSetGlobalTable (AcdbGblTblCmdType *pInput
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {// check if input is NULL
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdSetGlobalTable]->NULL Input!\n");      
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
            uint32_t memcmpResult;
            
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
   }
   return result;
}

int32_t AcdbCmdGetGlobalTable (AcdbGblTblCmdType *pInput,
                               AcdbQueryResponseType *pOutput)
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL || pOutput == NULL ||
       pInput->nBufferPointer == NULL || pInput->nBufferLength == 0)
   {//Check if input is NULL
      result = ACDB_BADPARM;      
      ACDB_DEBUG_LOG("[ACDB Command]->[AcdbCmdGetGlobalTable]->NULL Input!\n");      
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
   }

   return result;
}