/*===========================================================================
    FILE:           acdb_override.c

    OVERVIEW:       This file contains the implementaion of the heap optimization 
                    API and functions
    DEPENDENCIES:   None

                    Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

#include "acdb_includes.h"
#include "acdb_default_data.h"
#include "acdb_override.h"
#include "acdb_linked_list.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Global Data Definitions
 *--------------------------------------------------------------------------- */
static AcdbDynamicTblNodeType *g_pTbl = NULL;
static AcdbDynamicDataNodeType *g_pData = NULL;
static AcdbDynamicGeneralInfoType *g_pGenInfo = NULL;
static AcdbDynamicAdieTblNodeType *g_pAdieTbl = NULL;

/* ---------------------------------------------------------------------------
 * Static Variable Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

int32_t Acdb_SetDataCal(AcdbDataLookupKeyType *pKey,
                        uint32_t *pModuleId,
                        uint32_t *pParamId,
                        uint8_t *pInputBufPtr,
                        uint32_t InputBufLen,
                        AcdbDataUniqueDataNodeType **ppTable,
                        AcdbDataTopologyType *pTopology,
                        const uint32_t nTopologyEntries
                        )
{
   int32_t result = ACDB_BADPARM;

   if (pKey != NULL && pModuleId != NULL && pParamId != NULL 
       && pInputBufPtr != NULL && InputBufLen != 0 && ppTable != NULL 
       && nTopologyEntries != 0)
   {
      result = CompareStaticData((uint32_t*) pModuleId,
                                 (uint32_t*) pParamId,
                                 (AcdbDataUniqueDataNodeType**) ppTable,
                                 (AcdbDataTopologyType*) pTopology,
                                 (uint32_t) nTopologyEntries,
                                 (uint8_t*) pInputBufPtr,
                                 (uint32_t) InputBufLen
                                 );
      if (result == ACDB_SUCCESS)
      {//If data node exist on default,check table node on heap
         AcdbDynamicTblType *pTblNode = NULL;
         result = FindTableNodeOnHeap((AcdbDataLookupKeyType*) pKey,
                                      (AcdbDynamicTblNodeType*) g_pTbl,
                                      (AcdbDynamicTblType**) &pTblNode
                                      );
         if (result == ACDB_SUCCESS)
         {
            AcdbDynamicTopologyType *pTopNode = NULL;
            result = FindTopologyNodeOnHeap((uint32_t*) pModuleId,
                                            (uint32_t*) pParamId,
                                            (AcdbDynamicTblType*) pTblNode,
                                            (AcdbDynamicTopologyType**) &pTopNode
                                            );
            if (result == ACDB_SUCCESS)
            {//Free Topology Node
               uint32_t fReeTblResult = ACDB_HEAP_NOTFREE_NODE;
               result = FreeTopologyNode((uint32_t*) pModuleId,
                                         (uint32_t*) pParamId,
                                         (AcdbDynamicTblType*) pTblNode,
                                         (uint32_t*) &fReeTblResult
                                         );
               if (fReeTblResult == ACDB_HEAP_FREE_NODE)
               {//Free Table Node if topology node no longer exist on table node
                  result = FreeTableNode((AcdbDataLookupKeyType*) pKey,
                                         (AcdbDynamicTblNodeType*) g_pTbl
                                         );
               }
            }
            if (result == ACDB_SUCCESS)
            {
               result = FreeDataNode((uint32_t*) pParamId,
                                     (AcdbDynamicDataNodeType*) g_pData
                                     );            
            }
         }
         if (result == ACDB_PARMNOTFOUND)
         {
            result = ACDB_SUCCESS;
         }
      }
      else if (result == ACDB_PARMNOTFOUND)
      {//Data not member of static data
         AcdbDynamicUniqueDataType* pDataNode = NULL;
         uint32_t dataType = ACDB_HEAP_DATA_FOUND;

         result = IsDataOnHeap((uint32_t*) pParamId,
                               (uint8_t *) pInputBufPtr,
                               (uint32_t) InputBufLen,
                               (AcdbDynamicDataNodeType*) g_pData
                               );
         if (result == ACDB_PARMNOTFOUND)
         {
            result = CreateDataNodeOnHeap((uint32_t*) pParamId,
                                          (uint8_t *) pInputBufPtr,
                                          (uint32_t) InputBufLen,
                                          (AcdbDynamicDataNodeType*) g_pData,
                                          (AcdbDynamicUniqueDataType**) &pDataNode
                                          );
            dataType = ACDB_HEAP_DATA_NOT_FOUND;
         }//Data Node not found, create data node and return its address
         else if (result == ACDB_SUCCESS)
         {
            result = FindDataNodeOnHeap((uint32_t*) pParamId,
                                        (uint8_t *) pInputBufPtr,
                                        (uint32_t) InputBufLen,
                                        (AcdbDynamicDataNodeType*) g_pData,
                                        (AcdbDynamicUniqueDataType**) &pDataNode
                                        );
         }//if Data node found, find data node ptr address
         if (result == ACDB_SUCCESS)
         {
            AcdbDynamicTblType *pTblNode = NULL;
            result = FindTableNodeOnHeap((AcdbDataLookupKeyType*) pKey,
                                       (AcdbDynamicTblNodeType*) g_pTbl,
                                       (AcdbDynamicTblType**) &pTblNode
                                       );
            if (result == ACDB_PARMNOTFOUND)
            {
               result = CreateTableNodeOnHeap((AcdbDataLookupKeyType*) pKey,
                                              (AcdbDynamicTblNodeType*) g_pTbl,
                                              (AcdbDynamicTblType**) &pTblNode
                                              );
            }//table not created, create table node.
            if (result == ACDB_SUCCESS)
            {
               AcdbDynamicTopologyType *pTopNode = NULL;
               result = FindTopologyNodeOnHeap((uint32_t*) pModuleId,
                                               (uint32_t*) pParamId,
                                               (AcdbDynamicTblType*) pTblNode,
                                               (AcdbDynamicTopologyType**) &pTopNode
                                               );
               if (result == ACDB_SUCCESS)
               {
                  if (dataType == ACDB_HEAP_DATA_NOT_FOUND || pTopNode->pDataNode != pDataNode)
                  {
                     //Condition to decrement refcount
                     //1. check if found data node is different than what has already stored in the organization
                     //2. a new data node was created
                     //Decrease reference count from previous data node
                     pTopNode->pDataNode->refcount--;
                     //if data node reference = 0, free the data node
                     if (pTopNode->pDataNode->refcount == 0)
                     {
                        result = FreeDataNode((uint32_t*) pParamId,
                                              (AcdbDynamicDataNodeType*) g_pData
                                              );
                     }
                     //Link to new added data node
                     pTopNode->pDataNode = pDataNode;
                     pTopNode->pDataNode->refcount++;
                  }
               }
               else if (result == ACDB_PARMNOTFOUND)
               {
                  result = CreateTopologyNodeOnHeap((uint32_t*) pModuleId,
                                                    (uint32_t*) pParamId,
                                                    (AcdbDynamicUniqueDataType*) pDataNode,
                                                    (AcdbDynamicTblType*) pTblNode
                                                    );
               }//Create Topology node
            }//Create Table node
         }//Create Data node
      }
   }

   return result;
}

int32_t Acdb_GetDataCal(AcdbDataLookupKeyType *pKey,
                        uint32_t *pModuleId,
                        uint32_t *pParamId,
                        AcdbDynamicUniqueDataType **ppDataNode
                        )
{
   int32_t result = ACDB_BADPARM;

   if (pKey != NULL && pModuleId != NULL && pParamId && ppDataNode != NULL)
   {
      AcdbDynamicTblType *pTblNode = NULL;
      result = FindTableNodeOnHeap((AcdbDataLookupKeyType*) pKey,
                                   (AcdbDynamicTblNodeType*) g_pTbl,
                                   (AcdbDynamicTblType**) &pTblNode
                                   );
      if (result == ACDB_SUCCESS)
      {
         AcdbDynamicTopologyType *pTopNode = NULL;
         result = FindTopologyNodeOnHeap((uint32_t*) pModuleId,
                                         (uint32_t*) pParamId,
                                         (AcdbDynamicTblType*) pTblNode,
                                         (AcdbDynamicTopologyType**) &pTopNode
                                         );
         if (result == ACDB_SUCCESS)
         {
            *ppDataNode = pTopNode->pDataNode;
         }
      }
   }
   else
   {
      ACDB_DEBUG_LOG("[ACDB Override ERROR]->[Acdb_GetDataCal]->NULL Input pointer");
   }
   return result;
}

int32_t Acdb_SetTableCal(AcdbDataLookupKeyType *pKey,
                         AcdbDataGetTblTopType *pTbltop,
                         uint8_t *pInputBufPtr,
                         const uint32_t InputBufLen,
                         AcdbDynamicTblNodeType *pTbl,
                         AcdbDynamicDataNodeType *pData
                         )
{
   int32_t result = ACDB_BADPARM;
   
   if (pKey != NULL && pTbltop != NULL && pInputBufPtr != NULL && InputBufLen != 0 
      && pTbl != NULL && pData != NULL)
   {
      uint32_t i = 0;
      uint32_t offset = 0;

      AcdbDynamicUniqueDataType *pDataNode = NULL;

      uint32_t ModuleId, ParamId, Datalen = 0;
      uint8_t *DataPtr;

      for (; i < pTbltop->nTableEntries; ++i)
      { 
         memcpy((void*)&ModuleId,(void*)(pInputBufPtr+offset),sizeof(uint32_t));
         memcpy((void*)&ParamId,(void*)(pInputBufPtr+offset+sizeof(uint32_t)),sizeof(uint32_t));
         memcpy((void*)&Datalen,(void*)(pInputBufPtr+offset+sizeof(uint32_t)*2),sizeof(uint32_t));
         DataPtr = pInputBufPtr + offset + sizeof(uint32_t)*3;

         offset += (sizeof(uint32_t)*3+Datalen);

         result = Acdb_SetDataCal((AcdbDataLookupKeyType*) pKey,
                                 (uint32_t*) &ModuleId,
                                 (uint32_t*) &ParamId,
                                 (uint8_t*) DataPtr,
                                 (uint32_t) Datalen,
                                 (AcdbDataUniqueDataNodeType**) pTbltop->ppTable,
                                 (AcdbDataTopologyType*) pTbltop->pTopology,
                                 (uint32_t) pTbltop->nTableEntries
                                 );
      }
   }
   else
   {
      ACDB_DEBUG_LOG("[ACDB Override ERROR]->[Acdb_SetTableCal]->NULL Input pointer");
   }
   return result;
}

int32_t Acdb_GetTableCal(AcdbDataLookupKeyType *pKey,
                         AcdbDynamicTblNodeType *pTbl,
                         AcdbDynamicDataNodeType *pData,
                         AcdbDataGetTblTopType *pTbltop,
                         uint8_t *pDst,
                         const uint32_t ulDstsize,
                         uint32_t *pBytesUsed
                         )
{
   int32_t result = ACDB_SUCCESS;
   
   if (pKey != NULL && pTbl != NULL && pData != NULL && pTbltop != NULL 
       && pDst != NULL && ulDstsize != 0 && pBytesUsed != NULL)
   {
      uint32_t i = 0;
      uint32_t offset = 0;
      uint8_t* pDstEnd = pDst + ulDstsize;

      AcdbDataUniqueDataNodeType** ppCalTbl = (*pTbltop).ppTable;
      AcdbDataTopologyType* pTopology = (*pTbltop).pTopology;
      AcdbDynamicUniqueDataType *pDataNode = NULL;

      uint32_t data_size = 0;
      uint32_t ulDataPadLen = 0;

      for (; i < pTbltop->nTableEntries; ++i)
      {
         result = IsDataNodeOnHeap((uint32_t*) &pTopology[i].ulParamId,
                                   (AcdbDynamicDataNodeType*) pData
                                   );
         if (result == ACDB_SUCCESS)
         {
            result = GetDataCal((AcdbDataLookupKeyType*) pKey,
                                (uint32_t*) &pTopology[i].ulModuleId,
                                (uint32_t*) &pTopology[i].ulParamId,
                                (AcdbDynamicTblNodeType*) pTbl,
                                (AcdbDynamicUniqueDataType**) &pDataNode
                                );
         }
         //Calculate the Data Size
         if (result == ACDB_SUCCESS && pDataNode != NULL)
         {
            if (pDataNode->ulDataLen%4)
            {
               ulDataPadLen = 4-pDataNode->ulDataLen%4;
               data_size = 3*sizeof(uint32_t) + pDataNode->ulDataLen + ulDataPadLen;
            }
            else
            {
               ulDataPadLen = 0;
               data_size = 3*sizeof(uint32_t) + pDataNode->ulDataLen;
            }
         }
         else if (result == ACDB_PARMNOTFOUND)
         {
            if (ppCalTbl[i]->ulDataLen%4)
            {
               ulDataPadLen = 4-ppCalTbl[i]->ulDataLen%4;
               data_size = 3*sizeof(uint32_t) + ppCalTbl[i]->ulDataLen + ulDataPadLen;
            }
            else
            {
               ulDataPadLen = 0;
               data_size = 3*sizeof(uint32_t) + ppCalTbl[i]->ulDataLen;
            }
         }
         //Check memory big enough to hold the data
         if (pDstEnd < pDst + offset + data_size && (result == ACDB_PARMNOTFOUND || result == ACDB_SUCCESS))
         {
            result = ACDB_INSUFFICIENTMEMORY;
         }//Send data
         else
         {
            uint32_t ulDataLen = 0;

            memcpy ((void *)(pDst + offset), (const void *) &pTopology[i].ulModuleId, sizeof (uint32_t));
            memcpy ((void *)(pDst + offset +  4), (const void *) &pTopology[i].ulParamId, sizeof (uint32_t));
            if (result == ACDB_SUCCESS)
            {
               if (pDataNode != NULL)
               {
                  ulDataLen = pDataNode->ulDataLen + ulDataPadLen;
                  memcpy ((void *)(pDst + offset +  8), (const void *) &ulDataLen, sizeof (uint16_t));
                  memset ((void *)(pDst + offset + 10), 0, sizeof (uint16_t));
                  memcpy ((void *)(pDst + offset + 12), (const void *) pDataNode->ulDataBuf, pDataNode->ulDataLen);
                  offset += pDataNode->ulDataLen;
                  //Padding the data into multiple 4 bytes.
                  if (ulDataPadLen)
                  {
                     memset ((void*)(pDst+offset+12),0,ulDataPadLen);
                     offset += ulDataPadLen;
                  }
               }
               else
               {
                  result = ACDB_BADPARM;
               }
            }
            else if (result == ACDB_PARMNOTFOUND)
            {
               ulDataLen = ppCalTbl[i]->ulDataLen + ulDataPadLen;
               memcpy ((void *)(pDst + offset +  8), (const void *) &ulDataLen, sizeof (uint16_t));
               memset ((void *)(pDst + offset + 10), 0, sizeof (uint16_t));
               memcpy ((void *)(pDst + offset + 12), (const void *) ppCalTbl[i]->pUniqueData, ppCalTbl[i]->ulDataLen);
               offset += ppCalTbl[i]->ulDataLen;
               //Padding the data into multiple 4 bytes.
               if (ulDataPadLen)
               {
                  memset ((void*)(pDst+offset+12),0,ulDataLen-ppCalTbl[i]->ulDataLen);
                  offset += ulDataPadLen;
               }
            }
            offset += 12;
            result = ACDB_SUCCESS;
         }
      }
      *pBytesUsed = offset;
   }
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG("[ACDB Override ERROR]->[Acdb_GetTableCal]->NULL Input pointer");
   }//Null input
   return result;
}

int32_t Acdb_sys_reset(void)
{
   int32_t result = ACDB_SUCCESS;

   //Free general table on Heap
   if (g_pTbl != NULL)
   {
      AcdbDynamicTblType *pCur;
      pCur = g_pTbl->pTblHead;
      while (pCur != NULL)
      {
         AcdbDynamicTopologyType *pCurTop;
         if (pCur->pTopologyNode != NULL)
         {
            pCurTop = pCur->pTopologyNode->pTopHead;
            while (pCurTop)
            {
               pCur->pTopologyNode->pTopHead = pCurTop->pNext;
               ACDB_MEM_FREE(pCurTop);
               pCurTop = pCur->pTopologyNode->pTopHead;               
            }
            if (pCurTop != NULL)
            {
               ACDB_MEM_FREE(pCur->pTopologyNode->pTopHead);
            }
         }
         g_pTbl->pTblHead = pCur->pNext;
         ACDB_MEM_FREE(pCur->pKey);
         ACDB_MEM_FREE(pCur->pTopologyNode);
         ACDB_MEM_FREE(pCur);
         pCur = g_pTbl->pTblHead;
         if (pCur == NULL)
         {
            g_pTbl->pTblTail = NULL;
         }
      }
      ACDB_MEM_FREE(g_pTbl);
      g_pTbl = NULL;
   }

   //Free Data on Heap
   if (g_pData != NULL)
   {
      AcdbDynamicUniqueDataType *pCurData = g_pData->pDatalHead;
      
      while (pCurData)
      {
         g_pData->pDatalHead = pCurData->pNext;
         ACDB_MEM_FREE(pCurData->ulDataBuf);
         ACDB_MEM_FREE(pCurData);
         pCurData = g_pData->pDatalHead;
      }
      if (pCurData != NULL)
      {
         ACDB_MEM_FREE(g_pData->pDatalHead);
      }
      ACDB_MEM_FREE(g_pData);
      g_pData = NULL;
   }

   //Free General Info on Heap
   if (g_pGenInfo != NULL)
   {
      if (g_pGenInfo->pOEMInfo != NULL)
      {
         if (g_pGenInfo->pOEMInfo->pBuffer != NULL)
         {
            ACDB_MEM_FREE(g_pGenInfo->pOEMInfo->pBuffer);
         }
         ACDB_MEM_FREE(g_pGenInfo->pOEMInfo);
      }

      if (g_pGenInfo->pDateInfo != NULL)
      {
         if (g_pGenInfo->pDateInfo->pBuffer != NULL)
         {
            ACDB_MEM_FREE(g_pGenInfo->pDateInfo->pBuffer);
         }
         ACDB_MEM_FREE(g_pGenInfo->pDateInfo);
      }
      ACDB_MEM_FREE(g_pGenInfo);
      g_pGenInfo = NULL;
   }

   //Free Adie Table on Heap
   if (g_pAdieTbl != NULL)
   {
      AcdbDynamicAdieTblType *pCurAdieTbl;
      pCurAdieTbl = g_pAdieTbl->pAdieTblHead;
      while (pCurAdieTbl)
      {
         g_pAdieTbl->pAdieTblHead = pCurAdieTbl->pNext;
         ACDB_MEM_FREE(pCurAdieTbl->pKey);
         ACDB_MEM_FREE(pCurAdieTbl);
         pCurAdieTbl = g_pAdieTbl->pAdieTblHead;
      }
      if (pCurAdieTbl != NULL)
      {
         ACDB_MEM_FREE(g_pAdieTbl->pAdieTblHead);
      }
      ACDB_MEM_FREE(g_pAdieTbl);
      g_pAdieTbl = NULL;
   }

   return result;
}

/* ---------------------------------------------------------------------------
 * Externalized Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

int32_t Acdb_GetOEMInfo(AcdbDataGeneralInfoType *pInput
                        )
{
   int32_t result = ACDB_PARMNOTFOUND;

   if (pInput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (g_pGenInfo != NULL)
      {
         if (g_pGenInfo->pOEMInfo != NULL)
         {       
            result = GetInfoDataNodeOnHeap((AcdbDataGeneralInfoType*) pInput,
                                           (AcdbDataGeneralInfoType*) g_pGenInfo->pOEMInfo);
         }
      }
   }
   return result;
}

int32_t Acdb_SetOEMInfo(AcdbDataGeneralInfoType *pInput
                        )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      AcdbDataGeneralInfoType InfoBuf;
      result = AcdbDataIoctl(ACDBDATA_GET_OEM_INFO,
                             NULL, 0,
                             (uint8_t*)&InfoBuf, 
                             sizeof(AcdbGeneralInfoCmdType));
         
      if (result == ACDB_SUCCESS)
      {
         int32_t memcmpResult = ACDB_PARMNOTFOUND;
         if (InfoBuf.nBufferLength == pInput->nBufferLength)
         {//Compare data with default data
            memcmpResult = memcmp((void*)pInput->pBuffer,(void*)InfoBuf.pBuffer,
                                  InfoBuf.nBufferLength);
         }

         if (memcmpResult != ACDB_SUCCESS)
         {//Data is different than default data
            result = IsInfoDataOnHeap((AcdbDataGeneralInfoType*) pInput,
                                      (AcdbDataGeneralInfoType*) g_pGenInfo->pOEMInfo);
            
            if (result == ACDB_PARMNOTFOUND)
            {//Data not found on heap, create data on heap
               result = CreateInfoDataNodeOnHeap((AcdbDataGeneralInfoType*) pInput,
                                                 (AcdbDataGeneralInfoType**) &g_pGenInfo->pOEMInfo);
            }
         }
         else
         {//Free OEMInfo data node
            if (g_pGenInfo->pOEMInfo != NULL)
            {
               result = FreeInfoDataNodeOnHeap((AcdbDataGeneralInfoType**) &g_pGenInfo->pOEMInfo);
               if (result == ACDB_SUCCESS && g_pGenInfo->pDateInfo == NULL)
               {
                  ACDB_MEM_FREE(g_pGenInfo);
                  g_pGenInfo = NULL;
               }
            }
         }
      }
   }
   return result;
}

int32_t Acdb_GetDateInfo(AcdbDataGeneralInfoType *pInput
                         )
{
   int32_t result = ACDB_PARMNOTFOUND;

   if (pInput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (g_pGenInfo != NULL)
      {
         if (g_pGenInfo->pDateInfo != NULL)
         {
            result = GetInfoDataNodeOnHeap((AcdbDataGeneralInfoType*) pInput,
                                           (AcdbDataGeneralInfoType*) g_pGenInfo->pDateInfo);
         }
      }   
   }
   return result;
}

int32_t Acdb_SetDateInfo(AcdbDataGeneralInfoType *pInput
                         )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      AcdbDataGeneralInfoType InfoBuf;
      result = AcdbDataIoctl(ACDBDATA_GET_DATE_INFO,
                             NULL, 0,
                             (uint8_t*)&InfoBuf, 
                             sizeof(AcdbGeneralInfoCmdType));

      if (result == ACDB_SUCCESS)
      {
         int32_t memcmpResult = ACDB_PARMNOTFOUND;
         if (InfoBuf.nBufferLength == pInput->nBufferLength)
         {//Compare data with default data
            memcmpResult = memcmp((void*)pInput->pBuffer,(void*)InfoBuf.pBuffer,
                                  InfoBuf.nBufferLength);
         }
            
         if (memcmpResult != ACDB_SUCCESS)
         {//Data is different than default data
            result = IsInfoDataOnHeap((AcdbDataGeneralInfoType*) pInput,
                                      (AcdbDataGeneralInfoType*) g_pGenInfo->pDateInfo);

            if (result == ACDB_PARMNOTFOUND)
            {//If data not found on heap, create data node on heap
               result = CreateInfoDataNodeOnHeap((AcdbDataGeneralInfoType*) pInput,
                                                 (AcdbDataGeneralInfoType**) &g_pGenInfo->pDateInfo);
            }
         }
         else
         {//Free OEMInfo data node
            if (g_pGenInfo->pDateInfo != NULL)
            {
               result = FreeInfoDataNodeOnHeap((AcdbDataGeneralInfoType**) &g_pGenInfo->pDateInfo);
               if (result == ACDB_SUCCESS && g_pGenInfo->pDateInfo == NULL)
               {
                  ACDB_MEM_FREE(g_pGenInfo);
                  g_pGenInfo = NULL;
               }
            }
         }
      }
   }
   return result;
}

int32_t AcdbDataMemoryRamEstimate(AcdbDataMemoryUsageType *ram
                                  )
{
   int32_t result = ACDB_SUCCESS;

   if (ram != NULL)
   {
      ram->data_ROM = 0;
      ram->org_ROM = 0;

      if (g_pTbl != NULL)
      {
         AcdbDynamicTblType *pCur;
         pCur = g_pTbl->pTblHead;
         while (pCur)
         {
            if (pCur->pTopologyNode != NULL)
            {
               AcdbDynamicTopologyType *pCurTop;
               pCurTop = pCur->pTopologyNode->pTopHead;
               while (pCurTop)
               {
                  ram->org_ROM += sizeof(AcdbDynamicTopologyType);
                  pCurTop = pCurTop->pNext;
               }
            }
            ram->org_ROM += sizeof(AcdbDynamicTopologyNodeType);
            ram->org_ROM += sizeof(AcdbDataLookupKeyType);
            ram->org_ROM += sizeof(AcdbDynamicTblType);
            pCur = pCur->pNext;
         }
      }

      if (g_pData != NULL)
      {
         AcdbDynamicUniqueDataType *pCurData;
         pCurData = g_pData->pDatalHead;
         while (pCurData)
         {
            ram->data_ROM += pCurData->ulDataLen;
            ram->org_ROM += sizeof(AcdbDynamicUniqueDataType);
            pCurData = pCurData->pNext;
         }
      }

      if (g_pGenInfo != NULL)
      {
         if (g_pGenInfo->pOEMInfo != NULL)
         {
            ram->data_ROM += g_pGenInfo->pOEMInfo->nBufferLength;
            ram->org_ROM += sizeof(AcdbDataGeneralInfoType);

         }
         if (g_pGenInfo->pDateInfo != NULL)
         {
            ram->data_ROM += g_pGenInfo->pDateInfo->nBufferLength;
            ram->org_ROM += sizeof(AcdbDataGeneralInfoType);
         }
      }

      if (g_pAdieTbl != NULL)
      {
         AcdbDynamicAdieTblType *pCur;
         pCur = g_pAdieTbl->pAdieTblHead;
         while (pCur)
         {
            ram->org_ROM += sizeof(AcdbDataLookupKeyType);
            ram->org_ROM += sizeof(AcdbDynamicAdieTblType);
            pCur = pCur->pNext;
         }
      }
   }
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG("[ACDB Override ERROR]->[AcdbDataMemoryRamEstimate]->NULL Input pointer");
   }
   return result;
}

int32_t Acdb_ChecktoFreeAdieTableCalOnHeap(AcdbDataLookupKeyType *pKey,
                                           uint8_t *pInputBufPtr,
                                           const uint32_t InputBufLen
                                           )
{
   int32_t result = ACDB_SUCCESS;

   if (pKey != NULL && pInputBufPtr != NULL && InputBufLen != 0)
   {
      if (g_pAdieTbl != NULL)
      {
         AcdbDynamicAdieTblType *pTblNode = NULL;
         result = FindAdieTableNodeOnHeap((AcdbDataLookupKeyType*) pKey,
                                          (AcdbDynamicAdieTblNodeType*) g_pAdieTbl,
                                          (AcdbDynamicAdieTblType**) &pTblNode
                                          );
         if (result == ACDB_SUCCESS)
         {
            result = FreeAdieTableNode((AcdbDataLookupKeyType*) pKey,
                                       (AcdbDynamicAdieTblNodeType*) g_pAdieTbl
                                       );
            if (result == ACDB_SUCCESS)
            {
               result = FreeDataNode((uint32_t*) &pKey->nLookupKey,
                                     (AcdbDynamicDataNodeType*) g_pData
                                     );
            }
         }
         if (result == ACDB_PARMNOTFOUND)
         {
            result = ACDB_SUCCESS;
         }
      }
   }
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG("[ACDB Override ERROR]->[Acdb_ChecktoFreeAdieTableCalOnHeap]->NULL Input pointer");
   }//Null input

   return result;
}

int32_t Acdb_SetAdieTableCal(AcdbDataLookupKeyType *pKey,
                             uint8_t *pInputBufPtr,
                             const uint32_t InputBufLen
                             )
{
   int32_t result = ACDB_BADPARM;

   if (pKey != NULL && pInputBufPtr != NULL && InputBufLen != 0)
   {
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      AcdbDynamicAdieTblType *pTblNode = NULL;

      uint32_t dataType = ACDB_HEAP_DATA_FOUND;

      result = IsDataOnHeap((uint32_t*) &pKey->nLookupKey,
                            (uint8_t*) pInputBufPtr,
                            (uint32_t) InputBufLen,
                            (AcdbDynamicDataNodeType*) g_pData
                            ); //check if data is on heap
      if (result == ACDB_PARMNOTFOUND)
      {
         result = CreateDataNodeOnHeap((uint32_t*) &pKey->nLookupKey,
                                       (uint8_t*) pInputBufPtr,
                                       (uint32_t) InputBufLen,
                                       (AcdbDynamicDataNodeType*) g_pData,
                                       (AcdbDynamicUniqueDataType**) &pDataNode
                                       ); //if not on heap, create data node
         dataType = ACDB_HEAP_DATA_NOT_FOUND;
      }
      else if (result == ACDB_SUCCESS)
      {
         result = FindDataNodeOnHeap((uint32_t*) &pKey->nLookupKey,
                                     (uint8_t*) pInputBufPtr,
                                     (uint32_t) InputBufLen,
                                     (AcdbDynamicDataNodeType*) g_pData,
                                     (AcdbDynamicUniqueDataType**) &pDataNode
                                      ); //if yes, find the data node
      }
      result = FindAdieTableNodeOnHeap((AcdbDataLookupKeyType*) pKey,
                                       (AcdbDynamicAdieTblNodeType*) g_pAdieTbl,
                                       (AcdbDynamicAdieTblType**) &pTblNode
                                       ); // check if volume table node is on heap
      if (result == ACDB_SUCCESS)
      {
         //Both pDataNode and PTblNode should be filled
         if (pDataNode != NULL && pTblNode != NULL)
         {
            if (dataType == ACDB_HEAP_DATA_NOT_FOUND || pDataNode != pTblNode->pDataNode)
            {
               //Condition to decrement refcount
               //1. check if found data node is different than what has already stored in the organization
               //2. a new data node was creatd
               //Decrease previous data node count
               pTblNode->pDataNode->refcount--;
               if (pTblNode->pDataNode->refcount == 0)
               {
                  result = FreeDataNode((uint32_t*) &pKey->nLookupKey,
                                        (AcdbDynamicDataNodeType*) g_pData
                                        );
               }
               //Assign new data node
               pTblNode->pDataNode = pDataNode;
               pTblNode->pDataNode->refcount++;   
            }
         }
      }
      else if (result == ACDB_PARMNOTFOUND)
      {
         result = CreateAdieTableNodeOnHeap((AcdbDataLookupKeyType*) pKey,
                                            (AcdbDynamicUniqueDataType*) pDataNode,
                                            (AcdbDynamicAdieTblNodeType*) g_pAdieTbl
                                            ); //if not, create the volume table node
      }
   }
   else
   {
      ACDB_DEBUG_LOG("[ACDB Override ERROR]->[Acdb_SetAdieTableCal]->NULL Input pointer");
   }//Null input

   return result;
}

int32_t Acdb_GetAdieTableCal(AcdbDataLookupKeyType *pKey,
                             AcdbDynamicUniqueDataType **ppDataNode
                             )
{
   int32_t result = ACDB_BADPARM;
   
   if (pKey != NULL && ppDataNode != NULL)
   {
      AcdbDynamicAdieTblType *pTblNode = NULL;

      result = IsDataNodeOnHeap((uint32_t*) &pKey->nLookupKey,
                                (AcdbDynamicDataNodeType*) g_pData
                                );
      
      if (result == ACDB_SUCCESS)
      {
         result = FindAdieTableNodeOnHeap((AcdbDataLookupKeyType*) pKey,
                                          (AcdbDynamicAdieTblNodeType*) g_pAdieTbl,
                                          (AcdbDynamicAdieTblType**) &pTblNode
                                          );
         if (result == ACDB_SUCCESS && pTblNode != NULL)
         {
            *ppDataNode = pTblNode->pDataNode;
         }
      }
   }
   else
   {
      ACDB_DEBUG_LOG("[ACDB Override ERROR]->[Acdb_GetAdieTableCal]->NULL Input pointer");
   }//Null input
   return result;
}

// Externalized Function
int32_t Acdb_DM_Ioctl(uint32_t Acdb_DM_CMD_Id,
                      AcdbDataLookupKeyType *pKey,
                      uint32_t *pModuleId,
                      uint32_t *pParamId,
                      AcdbDataGetTblTopType *pTbltop,                     
                      uint8_t *pInputBuf,
                      const uint32_t InputBufLen,
                      uint8_t *pBytesUsed,
                      AcdbDynamicUniqueDataType **ppDataNode
                      )
{
   int32_t result = ACDB_SUCCESS;

   //Global Variable Initialization
   if (g_pTbl == NULL)
   {
      g_pTbl = (AcdbDynamicTblNodeType*)ACDB_MALLOC(sizeof(AcdbDynamicTblNodeType));
      if (g_pTbl != NULL)
      {
         g_pTbl->pTblHead = NULL;
         g_pTbl->pTblTail = NULL;
      }
      else
      {
         result = ACDB_BADSTATE;
      }
   }
   if (g_pData == NULL)
   {
      g_pData = (AcdbDynamicDataNodeType*)ACDB_MALLOC(sizeof(AcdbDynamicDataNodeType));
      if (g_pData != NULL)
      {
         g_pData->pDatalHead = NULL;
         g_pData->pDatalTail = NULL;
      }
      else
      {
         result = ACDB_BADSTATE;
      }
   }
   if (g_pAdieTbl == NULL)
   {
      g_pAdieTbl = (AcdbDynamicAdieTblNodeType*)ACDB_MALLOC(sizeof(AcdbDynamicAdieTblNodeType));
      if (g_pAdieTbl != NULL)
      {
         g_pAdieTbl->pAdieTblHead = NULL;
         g_pAdieTbl->pAdieTblTail = NULL;
      }
      else
      {
         result = ACDB_BADSTATE;
      }
   }
   if (g_pGenInfo ==NULL)
   {
      g_pGenInfo = (AcdbDynamicGeneralInfoType*)ACDB_MALLOC(sizeof(AcdbDynamicGeneralInfoType));
      if (g_pGenInfo != NULL)
      {
         g_pGenInfo->pDateInfo = NULL;
         g_pGenInfo->pOEMInfo = NULL;
      }
      else
      {
         result = ACDB_BADSTATE;
      }
   }

   switch (Acdb_DM_CMD_Id)
   {
   case ACDB_GET_TABLE:
      
      result = Acdb_GetTableCal((AcdbDataLookupKeyType*) pKey,
                                (AcdbDynamicTblNodeType*) g_pTbl,
                                (AcdbDynamicDataNodeType*) g_pData,
                                (AcdbDataGetTblTopType*) pTbltop,
                                (uint8_t*) pInputBuf,
                                (uint32_t) InputBufLen,
                                (uint32_t*) pBytesUsed
                                );
      break;

   case ACDB_SET_TABLE:
      
      result = Acdb_SetTableCal((AcdbDataLookupKeyType*) pKey,
                                (AcdbDataGetTblTopType*) pTbltop,
                                (uint8_t*) pInputBuf,
                                (uint32_t) InputBufLen,
                                (AcdbDynamicTblNodeType*) g_pTbl,
                                (AcdbDynamicDataNodeType*) g_pData
                                );
      break;

   case ACDB_GET_DATA:
      
      result = Acdb_GetDataCal((AcdbDataLookupKeyType*) pKey,
                               (uint32_t*) pModuleId,
                               (uint32_t*) pParamId,
                               (AcdbDynamicUniqueDataType**) &(*ppDataNode)
                               );
      break;

   case ACDB_SET_DATA:

      result = Acdb_SetDataCal((AcdbDataLookupKeyType*) pKey,
                               (uint32_t*) pModuleId,
                               (uint32_t*) pParamId,
                               (uint8_t*) pInputBuf,
                               (uint32_t) InputBufLen,
                               (AcdbDataUniqueDataNodeType**) pTbltop->ppTable,
                               (AcdbDataTopologyType*) pTbltop->pTopology,
                               (uint32_t) pTbltop->nTopologyEntries
                               );
      break;

   case ACDB_GET_ADIE_TABLE:

      result = Acdb_GetAdieTableCal((AcdbDataLookupKeyType*) pKey,
                                   (AcdbDynamicUniqueDataType**) &(*ppDataNode)
                                   );
      break;

   case ACDB_SET_ADIE_TABLE:

      result = Acdb_SetAdieTableCal((AcdbDataLookupKeyType*) pKey,
                                   (uint8_t*) pInputBuf,
                                   (uint32_t) InputBufLen
                                   );
      break;

   case ACDB_SYS_RESET:

      result = Acdb_sys_reset();

      break;

   case ACDB_GET_OEM_INFO:
      if (pInputBuf == NULL)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbDataGeneralInfoType *pInput = (AcdbDataGeneralInfoType*) pInputBuf;
         result = Acdb_GetOEMInfo((AcdbDataGeneralInfoType*) pInput
                                  );
      }
      break;

   case ACDB_SET_OEM_INFO:
      if (pInputBuf == NULL)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbDataGeneralInfoType *pInput = (AcdbDataGeneralInfoType*) pInputBuf;
         result = Acdb_SetOEMInfo((AcdbDataGeneralInfoType*) pInput
                                  );
      }
      break;

   case ACDB_GET_DATE_INFO:
      if (pInputBuf == NULL)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbDataGeneralInfoType *pInput = (AcdbDataGeneralInfoType*) pInputBuf;
         result = Acdb_GetDateInfo((AcdbDataGeneralInfoType*) pInput
                                   );
      }
      break;

   case ACDB_SET_DATE_INFO:
      if (pInputBuf == NULL)
      {
         result = ACDB_BADPARM;
      }
      else
      {
         AcdbDataGeneralInfoType *pInput = (AcdbDataGeneralInfoType*) pInputBuf;
         result = Acdb_SetDateInfo((AcdbDataGeneralInfoType*) pInput
                                   );
      }
      break;
   }

   return result;
}
