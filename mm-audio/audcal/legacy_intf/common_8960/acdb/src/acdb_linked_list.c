/*===========================================================================

    acdb_linked_list.c

DESCRIPTION
  This file defines methods used to access and manipulate ACDB data structures.

REFERENCES
  None.

Copyright (c) 2010-2012 by Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal2/common/8k_PL/acdb/rel/4.2/src/acdb_linked_list.c#1 $ */
/*===========================================================================

EDIT HISTORY FOR FILE
  
This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //source/qcom/qct/multimedia2/Audio/acdb

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/23/10   ernanl  Introduce new function used in ACDB_DM_Ioctl.
07/06/10   ernanl  Initial revision.

===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_includes.h"
#include "acdb_linked_list.h"
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

int32_t FindTableNodeOnHeap(AcdbDataLookupKeyType *pKey,
                            AcdbDynamicTblNodeType *pTblOnHeap,
                            AcdbDynamicTblType **ppTblNode
                            )
{
   int32_t result = ACDB_PARMNOTFOUND;

   if (pKey != NULL && pTblOnHeap != NULL)
   {
      AcdbDynamicTblType *pCur = NULL;
      int32_t memcmpResult;

      if (pTblOnHeap->pTblHead != NULL)
      {
         pCur = pTblOnHeap->pTblHead;

         while (pCur)
         {
            memcmpResult = memcmp((void*)pCur->pKey, (void*)pKey, sizeof(AcdbDataLookupKeyType));
            if (memcmpResult == ACDB_SUCCESS)
            {
               result = ACDB_SUCCESS;
               *ppTblNode = pCur;
               break;
            }//if found matched Key, break
            pCur = pCur->pNext;
         }
      }//pTblHead not created yet, return
   }//Not NULL ptr
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[FindTableNodeOnHeap]->NULL Input pointer");
   }//Error message
   return result;
}

int32_t FindTopologyNodeOnHeap(uint32_t *pModuleId,
                               uint32_t *pParamId,
                               AcdbDynamicTblType *pTblNode,
                               AcdbDynamicTopologyType **ppTopNode
                               )
{
   int32_t result = ACDB_PARMNOTFOUND;
   int32_t memcmpResult;

   if (pModuleId != NULL && pTblNode != NULL && ppTopNode != NULL && pParamId != NULL)
   {      
      if (pTblNode->pTopologyNode != NULL)
      {
         if (pTblNode->pTopologyNode->pTopHead != NULL)
         {
            AcdbDynamicTopologyType *pCurTopology;
            pCurTopology = pTblNode->pTopologyNode->pTopHead;
            
            while (pCurTopology)
            {
               memcmpResult = memcmp((void*)&pCurTopology->ulModuleId,
                                     (void*)pModuleId,sizeof(uint32_t));
               if (memcmpResult == ACDB_SUCCESS)
               {
                  memcmpResult = memcmp((void*)&pCurTopology->pDataNode->ulParamId,
                                        (void*)pParamId,sizeof(uint32_t));
                  if (memcmpResult == ACDB_SUCCESS)
                  {
                     result = ACDB_SUCCESS;
                     *ppTopNode = pCurTopology;
                     break;
                  }
               }
               pCurTopology = pCurTopology->pNext;
            }
         }
      }//pTblHead not created yet, return
   }//Not NULL ptr
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[FindTopologyNodeOnHeap]->NULL Input pointer");
   }//Error message
   return result;
}

int32_t IsDataNodeOnHeap(uint32_t *pParamId,
                         AcdbDynamicDataNodeType *pUniqDataOnHeap
                         )
{
   uint32_t result = ACDB_PARMNOTFOUND;
   uint32_t memcmpResult;

   if (pParamId != NULL && pUniqDataOnHeap != NULL)
   {
      AcdbDynamicUniqueDataType *pCur = NULL;

      if (pUniqDataOnHeap->pDatalHead != NULL)
      {
         pCur = pUniqDataOnHeap->pDatalHead;
         //Search Data Node
         if (pCur != NULL)
         {
            while(pCur)
            {
               memcmpResult = memcmp((void*)&pCur->ulParamId,(void*)pParamId, sizeof(uint32_t));
               if (memcmpResult == ACDB_SUCCESS)
               {
                  result = ACDB_SUCCESS;
                  break;
               }//if found matched Key, get table
               pCur = pCur->pNext;
            }
         }//Search from pHead to pTail Node
      }//if data not exist on heap, pDatalHead != NULL 
   }//Not Null Input
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[IsDataNodeOnHeap]->NULL Input pointer");
   }//Null input
   return result;
}

int32_t FindDataNodeOnHeap(uint32_t *pParamId,
                           uint8_t *pInData,
                           const uint32_t InDataLen,
                           AcdbDynamicDataNodeType *pUniqDataOnHeap,
                           AcdbDynamicUniqueDataType **ppDataNode
                           )
{
   uint32_t result = ACDB_PARMNOTFOUND;
   uint32_t memcmpResult;

   if (pParamId != NULL && pUniqDataOnHeap != NULL && ppDataNode != NULL)
   {
      AcdbDynamicUniqueDataType *pCur = NULL;

      if (pUniqDataOnHeap->pDatalHead != NULL)
      {
         pCur = pUniqDataOnHeap->pDatalHead;
         //Search Data Node
         if (pCur != NULL)
         {
            while(pCur)
            {
               memcmpResult = memcmp((void*)&pCur->ulParamId,(void*)pParamId, sizeof(uint32_t));
               if (memcmpResult == ACDB_SUCCESS)
               {
                  if (pCur->ulDataLen == InDataLen)
                  {
                     memcmpResult = memcmp(pCur->ulDataBuf,pInData,InDataLen);
                  }
                  else
                  {
                     memcmpResult = ACDB_BADPARM;
                  }
                  if (memcmpResult == ACDB_SUCCESS)
                  {
                     result = ACDB_SUCCESS;
                     *ppDataNode = pCur;
                     break;
                  }
               }//if found matched Key, get table
               pCur = pCur->pNext;
            }
         }//Search from pHead to pTail Node
      }//if data not exist on heap, pDatalHead != NULL 
   }//Not Null Input
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[FindDataNodeOnHeap]->NULL Input pointer");
   }//Null input
   return result;
}

int32_t IsDataOnHeap(uint32_t *ulParamId,
                     uint8_t *pUniqueData,
                     const uint32_t ulDataLen,
                     AcdbDynamicDataNodeType *pUniqDataOnHeap
                     )
{
   int32_t result = ACDB_PARMNOTFOUND;
   int32_t memcmpResult;

   if (pUniqueData != NULL && pUniqDataOnHeap != NULL && ulParamId != NULL && ulDataLen != 0)
   {
      AcdbDynamicUniqueDataType *pDataNode = NULL;
      result = FindDataNodeOnHeap((uint32_t*) ulParamId,
                                  (uint8_t*) pUniqueData,
                                  (uint32_t) ulDataLen,
                                  (AcdbDynamicDataNodeType*) pUniqDataOnHeap,
                                  (AcdbDynamicUniqueDataType**) &pDataNode
                                );
      if (result == ACDB_SUCCESS && pDataNode != NULL)
      {//check data node not equal to NULL
         if (pDataNode->ulDataLen == ulDataLen)
         {
            memcmpResult = memcmp((void*)pUniqueData,(void*)pDataNode->ulDataBuf,pDataNode->ulDataLen);
            if (memcmpResult == ACDB_SUCCESS)
            {
               result = ACDB_SUCCESS;
            }
         }
      }      
   }//Not Null Input
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[IsDataOnHeap]->NULL Input pointer");
   }//Null input
   return result;
}

int32_t CreateTopologyNodeOnHeap(uint32_t *pModuleId,
                                 uint32_t *pParamId,
                                 AcdbDynamicUniqueDataType *pDataNode,
                                 AcdbDynamicTblType *pTblNodeOnHeap
                                 )                                 
{
   int32_t result = ACDB_BADPARM;

   if (pModuleId != NULL && pParamId != NULL && pDataNode != NULL && pTblNodeOnHeap != NULL)
   {
      AcdbDynamicTopologyType* pCurTop = NULL;

      if (pTblNodeOnHeap->pTopologyNode == NULL)
      {
         pTblNodeOnHeap->pTopologyNode = (AcdbDynamicTopologyNodeType*)ACDB_MALLOC(sizeof(AcdbDynamicTopologyNodeType));
         if (pTblNodeOnHeap->pTopologyNode != NULL)
         {
            pTblNodeOnHeap->pTopologyNode->pTopHead = (AcdbDynamicTopologyType*)ACDB_MALLOC(sizeof(AcdbDynamicTopologyType));
            if (pTblNodeOnHeap->pTopologyNode->pTopHead != NULL)
            {
               pTblNodeOnHeap->pTopologyNode->pTopTail = pTblNodeOnHeap->pTopologyNode->pTopHead;
                 
               pCurTop = pTblNodeOnHeap->pTopologyNode->pTopHead;
               pCurTop->ulModuleId = (uint32_t)*pModuleId;
               pCurTop->pDataNode = pDataNode;
               pCurTop->pDataNode->refcount ++;

               pCurTop->pNext = NULL;

               result = ACDB_SUCCESS;
            }
            else
            {//malloc pTblNodeOnHeap->pTopologyNode->pTopHead failed
               result = ACDB_BADSTATE;
            }
         }
         else
         {//malloc pTblNodeOnHeap->pTopologyNode failed
            result = ACDB_BADSTATE;
         }
      }//Create Data Node on Heap
      else
      {
         pCurTop = (AcdbDynamicTopologyType *)ACDB_MALLOC(sizeof(AcdbDynamicTopologyType));
         if (pCurTop != NULL)
         {
            pCurTop->ulModuleId = (uint32_t)*pModuleId;
            pCurTop->pDataNode = pDataNode;
            pCurTop->pDataNode->refcount ++;

            pTblNodeOnHeap->pTopologyNode->pTopTail->pNext = pCurTop;
            pTblNodeOnHeap->pTopologyNode->pTopTail = pCurTop;
            pTblNodeOnHeap->pTopologyNode->pTopTail->pNext = NULL;

            result = ACDB_SUCCESS;
         }
         else
         {
            result = ACDB_BADSTATE;
         }
      }//Create Data Node on Heap
   }//check if Null Input
   else
   {
      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[CreateTopologyNodeOnHeap]->NULL Input pointer");
   }
   return result;
}

int32_t FreeTopologyNode(uint32_t *pModuleId,
                         uint32_t *pParamId,
                         AcdbDynamicTblType *pTblNode,
                         uint32_t *fReeTblResult
                         )
{
   int32_t result = ACDB_BADPARM;
   int32_t memcmpResult;

   if (pModuleId != NULL && pParamId != NULL && pTblNode != NULL)
   {
      if (pTblNode->pTopologyNode != NULL)
      {
         if (pTblNode->pTopologyNode->pTopHead != NULL)
         {            
            AcdbDynamicTopologyType* pPrev = pTblNode->pTopologyNode->pTopHead;
            AcdbDynamicTopologyType* pCur = pPrev->pNext;

            memcmpResult = memcmp((void*)pModuleId,(void*)&pPrev->ulModuleId,sizeof(uint32_t));
            if (memcmpResult == ACDB_SUCCESS)
            {
               memcmpResult = memcmp((void*)pParamId,(void*)&pPrev->pDataNode->ulParamId,sizeof(uint32_t));
               if (memcmpResult == ACDB_SUCCESS)
               {
                  pTblNode->pTopologyNode->pTopHead = pCur;
                  if(pCur == NULL)
                  {
                     pTblNode->pTopologyNode->pTopTail = NULL;
                     *fReeTblResult = ACDB_HEAP_FREE_NODE;
                  }
                  // if refcount > 0, decrease reference
                  if (pPrev->pDataNode->refcount > 0)
                  {
                     pPrev->pDataNode->refcount--;
                  }
                  ACDB_MEM_FREE(pPrev);
               }
            }
            if (memcmpResult != ACDB_SUCCESS)
            {
               while (pCur)
               {
                  memcmpResult = memcmp((void*)pModuleId,(void*)&pCur->ulModuleId,sizeof(uint32_t));
                  if (memcmpResult == ACDB_SUCCESS)
                  {
                     memcmpResult = memcmp((void*)pParamId,(void*)&pCur->pDataNode->ulParamId,sizeof(uint32_t));
                     {
                        if (memcmpResult == ACDB_SUCCESS)
                        {
                           break;
                        }
                     }
                  }
                  pPrev = pPrev->pNext;
                  pCur = pCur->pNext;
               }//Searching for the node
               if (pCur != NULL)
               {
                  pPrev->pNext = pCur->pNext;
                  if(pCur->pNext == NULL)
                  {
                     pTblNode->pTopologyNode->pTopTail = pPrev;
                  }
                  // if refcount > 0, decrease reference
                  if (pCur->pDataNode->refcount > 0)
                  {
                     pCur->pDataNode->refcount--;
                  }
                  ACDB_MEM_FREE(pCur);    //Free current node
               }
            }            
         }
      }
      result = ACDB_SUCCESS;
   }//Input not NULL
   else
   {
      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[FreeTopologyNode]->NULL Input pointer");
   }
   return result;
}

int32_t CreateTableNodeOnHeap(AcdbDataLookupKeyType *pKey,
                              AcdbDynamicTblNodeType *pTblOnHeap,
                              AcdbDynamicTblType **ppTblNode
                              )
{
   int32_t result = ACDB_BADPARM;

   if (pTblOnHeap != NULL && pKey != NULL)
   {
      AcdbDynamicTblType *pCur = NULL;

      if (pTblOnHeap == NULL)
      {
         AcdbDynamicTblNodeType *pTblOnHeap = (AcdbDynamicTblNodeType *)ACDB_MALLOC(sizeof(AcdbDynamicTblNodeType));
         pTblOnHeap->pTblHead = NULL; 
         pTblOnHeap->pTblTail = NULL;
      }

      if (pTblOnHeap->pTblHead == NULL)
      {
         pTblOnHeap->pTblHead = (AcdbDynamicTblType *)ACDB_MALLOC(sizeof(AcdbDynamicTblType));
         if (pTblOnHeap->pTblHead != NULL)
         {
            pTblOnHeap->pTblTail = pTblOnHeap->pTblHead;

            pCur = pTblOnHeap->pTblHead;
            pCur->pTopologyNode = NULL;

            pCur->pKey = (AcdbDataLookupKeyType*)ACDB_MALLOC(sizeof(AcdbDataLookupKeyType));
            if (pCur->pKey != NULL)
            {
               memcpy((void*)pCur->pKey,(void*)pKey,sizeof(AcdbDataLookupKeyType));
               
               *ppTblNode = pCur;
               pCur->pNext = NULL;
               
               result = ACDB_SUCCESS;
            }
            else
            {//malloc pCur->pKey failed
               result = ACDB_BADSTATE;
            }
         }
         else
         {//malloc pTblOnHeap->pTblHead failed
            result = ACDB_BADSTATE;
         }
      }
      else
      {
         pCur = (AcdbDynamicTblType*)ACDB_MALLOC(sizeof(AcdbDynamicTblType));
         if (pCur != NULL)
         {
            pCur->pTopologyNode = NULL;

            pCur->pKey = (AcdbDataLookupKeyType*)ACDB_MALLOC(sizeof(AcdbDataLookupKeyType));
            if (pCur->pKey != NULL)
            {
               memcpy((void*)pCur->pKey,(void*)pKey,sizeof(AcdbDataLookupKeyType));
               *ppTblNode = pCur;
               pCur->pNext = NULL;

               pTblOnHeap->pTblTail->pNext = pCur;
               pTblOnHeap->pTblTail = pCur;
               pTblOnHeap->pTblTail->pNext = NULL;

               result = ACDB_SUCCESS;
            }
            else
            {//malloc pCur->pKey failed
               result = ACDB_BADSTATE;
               ACDB_MEM_FREE(pCur);
            }
         }
         else
         {//malloc pCur failed
            result = ACDB_BADSTATE;
         }
      }
   }
   else
   {
      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[CreateTableNodeOnHeap]->NULL Input pointer");
   }
   return result;
}

int32_t FreeTableNode(AcdbDataLookupKeyType *pKey,
                      AcdbDynamicTblNodeType *pTblOnHeap
                      )
{
   int32_t result = ACDB_BADPARM;
   int32_t memcmpResult;

   if (pKey != NULL && pTblOnHeap != NULL)
   {
      if (pTblOnHeap->pTblHead != NULL)
      {      
         AcdbDynamicTblType* pPrev = pTblOnHeap->pTblHead;
         AcdbDynamicTblType* pCur = pPrev->pNext;

         memcmpResult = memcmp((void*)pKey,(void*)pPrev->pKey,sizeof(AcdbDataLookupKeyType));
         if (memcmpResult == ACDB_SUCCESS)
         {
            pTblOnHeap->pTblHead = pCur;
            if(pCur == NULL)
            {
               pTblOnHeap->pTblTail = NULL;
            }
            ACDB_MEM_FREE(pPrev->pKey);
            ACDB_MEM_FREE(pPrev);
         }
         if (memcmpResult != ACDB_SUCCESS)
         {
            while (pCur)
            {
               memcmpResult = memcmp((void*)pKey,(void*)pCur->pKey,sizeof(AcdbDataLookupKeyType));
               if (memcmpResult == ACDB_SUCCESS)
               {
                  break;
               }
               pPrev = pPrev->pNext;
               pCur = pCur->pNext;
            }
            if (pCur != NULL)
            {
               pPrev->pNext = pCur->pNext;
               if(pCur->pNext == NULL)
               {
                  pTblOnHeap->pTblTail = pPrev;
               }
               ACDB_MEM_FREE(pCur->pKey);
               ACDB_MEM_FREE(pCur);
            }
         }
      }
      result = ACDB_SUCCESS;
   }
   else
   {
      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[FreeTableNode]->NULL Input pointer");
   }
   return result;
}

int32_t CreateDataNodeOnHeap(uint32_t *pParamId, 
                             uint8_t *pInData,
                             const uint32_t InDataLen,
                             AcdbDynamicDataNodeType *pUniqDataNode,
                             AcdbDynamicUniqueDataType **ppDataNode
                             )
{
   int32_t result = ACDB_BADPARM;

   if (pParamId != NULL && pInData != NULL && pUniqDataNode && InDataLen != 0)
   {
      AcdbDynamicUniqueDataType *pCur = NULL;

      if (pUniqDataNode->pDatalHead == NULL)
      {
         pUniqDataNode->pDatalHead = (AcdbDynamicUniqueDataType*)ACDB_MALLOC(sizeof(AcdbDynamicUniqueDataType));
         pUniqDataNode->pDatalTail = pUniqDataNode->pDatalHead;

         if (pUniqDataNode->pDatalHead != NULL)
         {
            pCur = pUniqDataNode->pDatalHead;
            pCur->ulParamId = (uint32_t)*pParamId;
            pCur->ulDataBuf = (uint8_t *)ACDB_MALLOC(InDataLen);
            if (pCur->ulDataBuf != NULL)
            {
               memcpy ((void *) pCur->ulDataBuf, (const void *) pInData, InDataLen);
               pCur->ulDataLen = InDataLen;
               pCur->refcount = 0;

               *ppDataNode = pCur;

               pCur->pNext = NULL;

               result = ACDB_SUCCESS;
            }
            else
            {
               result = ACDB_BADSTATE;
            }
         }
         else
         {
            result = ACDB_BADSTATE;
         }
      }//Create Data Node on Heap
      else
      {
         pCur = (AcdbDynamicUniqueDataType *)ACDB_MALLOC(sizeof(AcdbDynamicUniqueDataType));
         if (pCur != NULL)
         {
            pCur->ulParamId = (uint32_t)*pParamId;
            pCur->ulDataBuf = (uint8_t *)ACDB_MALLOC(InDataLen);
            if (pCur->ulDataBuf != NULL)
            {
               memcpy ((void *) pCur->ulDataBuf, (const void *) pInData, InDataLen);
               pCur->ulDataLen = InDataLen;       
               pCur->refcount = 0;

               *ppDataNode = pCur;

               pUniqDataNode->pDatalTail->pNext = pCur;
               pUniqDataNode->pDatalTail = pCur;
               pUniqDataNode->pDatalTail->pNext = NULL;

               result = ACDB_SUCCESS;
            }
            else
            {//malloc pCur->pKey failed
               result = ACDB_BADSTATE;
               ACDB_MEM_FREE(pCur);
            }
         }
         else
         {//malloc pCur failed
            result = ACDB_BADSTATE;
         }         
      }//Create Data Node on Heap
   }//check if Null Input
   else
   {
      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[CreateDataNodeOnHeap]->NULL Input pointer");
   }//Null input
   return result;
}

int32_t FreeDataNode(uint32_t *pParamId,
                     AcdbDynamicDataNodeType *pDataOnHeap
                     )
{
   int32_t result = ACDB_BADPARM;
   int32_t memcmpResult;

   if (pParamId != NULL && pDataOnHeap != NULL)
   {
      if (pDataOnHeap->pDatalHead != NULL)
      {      
         AcdbDynamicUniqueDataType* pPrev = pDataOnHeap->pDatalHead;
         AcdbDynamicUniqueDataType* pCur = pPrev->pNext;

         memcmpResult = memcmp((void*)pParamId,(void*)&pPrev->ulParamId,sizeof(uint32_t));
         if (memcmpResult == ACDB_SUCCESS) //Only interested in removing the first node
         {
            if (pPrev->refcount == 0)
            {
               pDataOnHeap->pDatalHead = pCur;
               if(pCur == NULL)
               {
                  pDataOnHeap->pDatalTail = NULL;
               }
               ACDB_MEM_FREE(pPrev->ulDataBuf);
               ACDB_MEM_FREE(pPrev);
            }
            else
            {
               memcmpResult = ACDB_PARMNOTFOUND;
            }
         }
         if (memcmpResult != ACDB_SUCCESS) //Removing other than the first node
         {
            while (pCur) //Started at the second node
            {
               memcmpResult = memcmp((void*)pParamId,(void*)&pCur->ulParamId,sizeof(uint32_t));;
               if (memcmpResult == ACDB_SUCCESS)
               {
                  if (pCur->refcount == 0) //If paramId match and refcount = 0, free the node
                  {
                     pPrev->pNext = pCur->pNext;
                     if(pCur->pNext == NULL)
                     {
                        pDataOnHeap->pDatalTail = pPrev;
                     }
                     ACDB_MEM_FREE(pCur->ulDataBuf);
                     ACDB_MEM_FREE(pCur);
                     break;
                  }
               }
               pPrev = pPrev->pNext;
               pCur = pCur->pNext;  //Perform comparison in next node
            }
         }
      }
      result = ACDB_SUCCESS;
   }
   else
   {
      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[FreeDataNode]->NULL Input pointer");
   }
   return result;
}

int32_t CompareStaticData(uint32_t *pModuleId,
                          uint32_t *pParamId,
                          AcdbDataUniqueDataNodeType **ppCalTbl,
                          AcdbDataTopologyType *pTopology,
                          const uint32_t nTopologyEntries,
                          uint8_t *pInBufPtr,
                          const uint32_t InBufLen
                          )
{
   int32_t result = ACDB_PARMNOTFOUND;
   int32_t memcmpResult = ACDB_SUCCESS;

   if (pParamId != NULL && pInBufPtr != NULL && InBufLen != 0 && ppCalTbl != NULL 
       && pTopology != NULL && nTopologyEntries != 0)
   {
      uint32_t i = 0;

      for (; i < nTopologyEntries; ++i)
      {
         memcmpResult = memcmp((void*)&(pTopology[i].ulModuleId),(void*)pModuleId,sizeof(uint32_t));
         if (memcmpResult == ACDB_SUCCESS)
         {
            memcmpResult = memcmp((void*)&(pTopology[i].ulParamId),(void*)pParamId,sizeof(uint32_t));
            if (memcmpResult == ACDB_SUCCESS)
            {
               break;
            }//Offset = 3*uint32 bytes
         }//Compare ParamId
      }//Compare ModuleId
      if (memcmpResult != ACDB_SUCCESS)        
      {
         result = ACDB_BADPARM;
      }//No Algorithm block found from the give topology
      else
      {
         if (memcmpResult == ACDB_SUCCESS)
         {
            memcmpResult = memcmp((void*)&((*ppCalTbl[i]).ulDataLen),(void*)&InBufLen,sizeof(uint32_t));        
            if (memcmpResult == ACDB_SUCCESS)
            {
               memcmpResult = memcmp((void*)(*ppCalTbl[i]).pUniqueData,(void*)pInBufPtr,InBufLen);
               if (memcmpResult == ACDB_SUCCESS)
               {
                  result = ACDB_SUCCESS;
               }
            }
         }//Compare data
      }//find if data length matches
   }
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[CompareStaticData]->NULL Input pointer");
   }//Null input
   return result;
}

int32_t GetDataCal(AcdbDataLookupKeyType *pKey,
                   uint32_t *pModuleId,
                   uint32_t *pParamId,
                   AcdbDynamicTblNodeType *pTbl,
                   AcdbDynamicUniqueDataType **ppDataNode
                   )
{
   int32_t result = ACDB_PARMNOTFOUND;

   if (pKey != NULL && pModuleId != NULL && pParamId != NULL && pTbl != NULL && ppDataNode != NULL)
   {
      int32_t memcmpResult = ACDB_PARMNOTFOUND;
      AcdbDynamicTblType *pCur = pTbl->pTblHead;
      while (pCur)
      {
         memcmpResult = memcmp((void*)pCur->pKey,(void*)pKey,sizeof(AcdbDataLookupKeyType));
         if (memcmpResult == ACDB_SUCCESS)
         {
            break;
         }
         pCur = pCur->pNext;
      }
      if (memcmpResult == ACDB_SUCCESS && pCur->pTopologyNode != NULL)
      {//check pTopologyNode if it is NULL
         AcdbDynamicTopologyType *pCurData = pCur->pTopologyNode->pTopHead;
         while (pCurData)
         {
            memcmpResult = memcmp((void*)&pCurData->ulModuleId,(void*)pModuleId,sizeof(uint32_t));
            if (memcmpResult == ACDB_SUCCESS)
            {
               memcmpResult = memcmp((void*)&pCurData->pDataNode->ulParamId,(void*)pParamId,sizeof(uint32_t));
               if (memcmpResult == ACDB_SUCCESS)
               {
                  *ppDataNode = pCurData->pDataNode;
                  result = ACDB_SUCCESS;
                  break;
               }//check paramId if found
            }//check moduleId match if found
            pCurData = pCurData->pNext;
         }//Search Table node
      }//check key if found
   }
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[GetDataCal]->NULL Input pointer");
   }//Null input
   return result;
}

int32_t IsInfoDataOnHeap(AcdbDataGeneralInfoType* pInput,
                         AcdbDataGeneralInfoType* pInfoData
                         )
{
   int32_t result = ACDB_PARMNOTFOUND;

   if (pInput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (pInfoData != NULL)
      {//Search if info data is on heap
         int32_t memcmpResult = ACDB_PARMNOTFOUND;
         if (pInput->nBufferLength == pInfoData->nBufferLength)
         {
            memcmpResult = memcmp((void*)pInfoData->pBuffer,(void*)pInput->pBuffer,
                                   pInput->nBufferLength);
         }
         if (memcmpResult == ACDB_SUCCESS)
         {
            result = ACDB_SUCCESS;
         }
      }
   }
   return result;
}

int32_t GetInfoDataNodeOnHeap(AcdbDataGeneralInfoType* pInput,
                              AcdbDataGeneralInfoType* pInfoData
                              )
{
   int32_t result = ACDB_PARMNOTFOUND;

   if (pInput == NULL)
   {
      result = ACDB_BADPARM;
   }
   else
   {
      if (pInfoData != NULL)
      {
         pInput->pBuffer = pInfoData->pBuffer;
         pInput->nBufferLength = pInfoData->nBufferLength;

         result = ACDB_SUCCESS;
      }
   }
   return result;
}

int32_t FreeInfoDataNodeOnHeap(AcdbDataGeneralInfoType** ppInfoData
                               )
{
   int32_t result = ACDB_SUCCESS;

   if (ppInfoData != NULL)
   {
      ACDB_MEM_FREE((*ppInfoData)->pBuffer);
      ACDB_MEM_FREE(*ppInfoData);
      *ppInfoData = NULL;
   }
   else
   {
      result = ACDB_BADPARM;
   }
   return result;
}

int32_t CreateInfoDataNodeOnHeap(AcdbDataGeneralInfoType* pInput,
                                 AcdbDataGeneralInfoType** ppInfoData
                                 )
{
   int32_t result = ACDB_SUCCESS;

   if (pInput != NULL && ppInfoData != NULL)
   {
      if (*ppInfoData == NULL)
      {//If pInfoData is null, allocate the buffer on heap
         (*ppInfoData) = (AcdbDataGeneralInfoType*)ACDB_MALLOC(sizeof(AcdbDataGeneralInfoType));
         if (*ppInfoData != NULL)
         {
            (*ppInfoData)->pBuffer = NULL;
         }
         else
         {
            result = ACDB_BADSTATE;
         }
      }
      if (result == ACDB_SUCCESS)
      {
         if ((*ppInfoData)->pBuffer == NULL)
         {//If pInfoData->pBuf is null, allocate the buffer on heap
            (*ppInfoData)->pBuffer = (uint8_t*)ACDB_MALLOC(pInput->nBufferLength);
         }
         else
         {//If pBuf is not null, deallocate the buffer and reallocate buffer with proper size
            ACDB_MEM_FREE((*ppInfoData)->pBuffer);
            (*ppInfoData)->pBuffer = (uint8_t*)ACDB_MALLOC(pInput->nBufferLength);
         }
         if ((*ppInfoData)->pBuffer != NULL)
         {
            memcpy((void*)(*ppInfoData)->pBuffer,(void*)pInput->pBuffer,pInput->nBufferLength);
            (*ppInfoData)->nBufferLength = pInput->nBufferLength;
         }
         else
         {
            result = ACDB_BADSTATE;
         }
      }
   }
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[CreateInfoDataNodeOnHeap]->NULL Input pointer");
   }
   return result;
}

int32_t FindAdieTableNodeOnHeap(AcdbDataLookupKeyType *pKey,
                                AcdbDynamicAdieTblNodeType *pTblOnHeap,
                                AcdbDynamicAdieTblType **ppTblNode
                                )
{
   int32_t result = ACDB_PARMNOTFOUND;
   int32_t memcmpResult;

   if (pKey != NULL && pTblOnHeap != NULL)
   {
      AcdbDynamicAdieTblType *pCur = NULL;

      if (pTblOnHeap->pAdieTblHead != NULL)
      {
         pCur = pTblOnHeap->pAdieTblHead;

         while (pCur)
         {
            memcmpResult = memcmp((void*)pCur->pKey, (void*)pKey, sizeof(AcdbDataLookupKeyType));
            if (memcmpResult == ACDB_SUCCESS)
            {
               result = ACDB_SUCCESS;
               *ppTblNode = pCur;
               break;
            }//if found matched Key, break
            pCur = pCur->pNext;
         }
      }//pTblHead not created yet, return
   }//Not NULL ptr
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[FindAdieTableNodeOnHeap]->NULL Input pointer");
   }//Error message
   return result;
}

int32_t CreateAdieTableNodeOnHeap(AcdbDataLookupKeyType *pKey,
                                  AcdbDynamicUniqueDataType *pDataNode,
                                  AcdbDynamicAdieTblNodeType *pTblOnHeap
                                  )
{
   int32_t result = ACDB_BADPARM;

   if (pTblOnHeap != NULL && pKey != NULL && pDataNode != NULL)
   {
      AcdbDynamicAdieTblType *pCur = NULL;

      if (pTblOnHeap == NULL)
      {
         AcdbDynamicAdieTblNodeType *pTblOnHeap = (AcdbDynamicAdieTblNodeType *)ACDB_MALLOC(sizeof(AcdbDynamicAdieTblNodeType));
         pTblOnHeap->pAdieTblHead = NULL; 
         pTblOnHeap->pAdieTblTail = NULL;
      }

      if (pTblOnHeap->pAdieTblHead == NULL)
      {
         pTblOnHeap->pAdieTblHead = (AcdbDynamicAdieTblType *)ACDB_MALLOC(sizeof(AcdbDynamicAdieTblType));
         pTblOnHeap->pAdieTblTail = pTblOnHeap->pAdieTblHead;

         pCur = pTblOnHeap->pAdieTblHead;

         if (pCur != NULL)
         {
            pCur->pKey = (AcdbDataLookupKeyType*)ACDB_MALLOC(sizeof(AcdbDataLookupKeyType));
            if (pCur->pKey != NULL)
            {
               memcpy((void*)pCur->pKey,(void*)pKey,sizeof(AcdbDataLookupKeyType));
            
               pCur->pDataNode = pDataNode;
               pCur->pDataNode->refcount++;
               pCur->pNext = NULL;

               result = ACDB_SUCCESS;
            }
            else
            {//malloc pCur->pKey failed
               result = ACDB_BADSTATE;
            }
         }
         else
         {//malloc pCur failed
            result = ACDB_BADSTATE;
         }
      }
      else
      {
         pCur = (AcdbDynamicAdieTblType*)ACDB_MALLOC(sizeof(AcdbDynamicAdieTblType));

         if (pCur != NULL)
         {
            pCur->pKey = (AcdbDataLookupKeyType*)ACDB_MALLOC(sizeof(AcdbDataLookupKeyType));

            if (pCur->pKey != NULL)
            {
               memcpy((void*)pCur->pKey,(void*)pKey,sizeof(AcdbDataLookupKeyType));
               pCur->pNext = NULL;

               pCur->pDataNode = pDataNode;
               pCur->pDataNode->refcount++;

               pTblOnHeap->pAdieTblTail->pNext = pCur;
               pTblOnHeap->pAdieTblTail = pCur;
               pTblOnHeap->pAdieTblTail->pNext = NULL;

               result = ACDB_SUCCESS;
            }
            else
            {//malloc pCur->pKey failed
               result = ACDB_BADSTATE;
               ACDB_MEM_FREE(pCur);
            }
         }
         else
         {//malloc pCur failed
            result = ACDB_BADSTATE;
         }
      }
   }
   else
   {
      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[CreateAdieTableNodeOnHeap]->NULL Input pointer");
   }
   return result;
}

int32_t FreeAdieTableNode(AcdbDataLookupKeyType *pKey,
                          AcdbDynamicAdieTblNodeType *pTblOnHeap
                         )
{
   int32_t result = ACDB_BADPARM;

   if (pKey != NULL && pTblOnHeap)
   {
      int32_t memcmpResult;

      if (pTblOnHeap->pAdieTblHead != NULL)
      {      
         AcdbDynamicAdieTblType* pPrev = pTblOnHeap->pAdieTblHead;
         AcdbDynamicAdieTblType* pCur = pPrev->pNext;

         memcmpResult = memcmp((void*)pKey,(void*)pPrev->pKey,sizeof(AcdbDataLookupKeyType));
         if (memcmpResult == ACDB_SUCCESS)
         {
            pTblOnHeap->pAdieTblHead = pCur;
            if(pCur == NULL)
            {
               pTblOnHeap->pAdieTblTail = NULL;
            }
            pPrev->pDataNode->refcount--;
            ACDB_MEM_FREE(pPrev->pKey);
            ACDB_MEM_FREE(pPrev);
         }
         else
         {
            if (pCur != NULL)
            {
               while (pCur)
               {
                  memcmpResult = memcmp((void*)pKey,(void*)pCur->pKey,sizeof(AcdbDataLookupKeyType));
                  if (memcmpResult == ACDB_SUCCESS)
                  {
                     break;
                  }
                  pPrev = pPrev->pNext;
                  pCur = pCur->pNext;
               }
               if (pCur != NULL)
               {
                  pPrev->pNext = pCur->pNext;
                  if(pCur->pNext == NULL)
                  {
                     pTblOnHeap->pAdieTblTail = pPrev;
                  }
                  pCur->pDataNode->refcount--;
                  ACDB_MEM_FREE(pCur->pKey);
                  ACDB_MEM_FREE(pCur);
               }
            }
         }
      }
      result = ACDB_SUCCESS;
   }
   else
   {
      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[FreeAdieTableNode]->NULL Input pointer");
   }
   return result;
}

int32_t GetDataNodeOnHeap(uint32_t *pParamId,
                           AcdbDynamicDataNodeType *pUniqDataOnHeap,
                           AcdbDynamicUniqueDataType **ppDataNode
                           )
{
   uint32_t result = ACDB_PARMNOTFOUND;
   uint32_t memcmpResult;

   if (pParamId != NULL && pUniqDataOnHeap != NULL && ppDataNode != NULL)
   {
      AcdbDynamicUniqueDataType *pCur = NULL;

      if (pUniqDataOnHeap->pDatalHead != NULL)
      {
         pCur = pUniqDataOnHeap->pDatalHead;
         //Search Data Node
         if (pCur != NULL)
         {
            while(pCur)
            {
               memcmpResult = memcmp((void*)&pCur->ulParamId,(void*)pParamId, sizeof(uint32_t));
               if (memcmpResult == ACDB_SUCCESS)
               {
					result = ACDB_SUCCESS;
					*ppDataNode = pCur;
					break;
               }//if found matched Key, get table
               pCur = pCur->pNext;
            }
         }//Search from pHead to pTail Node
      }//if data not exist on heap, pDatalHead != NULL 
   }//Not Null Input
   else
   {
      result = ACDB_BADPARM;

      ACDB_DEBUG_LOG ("[ACDB Linked_List ERROR]->[GetDataNodeOnHeap]->NULL Input pointer");
   }//Null input
   return result;
}
