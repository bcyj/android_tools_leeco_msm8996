#ifndef ACDB_LINKED_LIST_H
#define ACDB_LINKED_LIST_H
/*==============================================================================

FILE:      acdb_linked_list.h

DESCRIPTION: Functions and definitions to access the ACDB data structure.

PUBLIC CLASSES:  Not Applicable

INITIALIZATION AND SEQUENCING REQUIREMENTS:  N/A

        Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
               All Rights Reserved.
            Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_includes.h"
#include "acdb_default_data.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

#define ACDB_HEAP_FREE_NODE                  0x00000006
#define ACDB_HEAP_NOTFREE_NODE               0x00000007

/*------------------------------------------------------------------------------
Target specific definitions
------------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

typedef struct AcdbDynamicUniqueDatastruct{
   uint32_t refcount;
   uint32_t ulParamId;
   uint8_t *ulDataBuf;
   uint32_t ulDataLen;
   struct AcdbDynamicUniqueDatastruct *pNext;
}AcdbDynamicUniqueDataType;

typedef struct AcdbDynamicDataNodeStruct{
   AcdbDynamicUniqueDataType *pDatalHead;
   AcdbDynamicUniqueDataType *pDatalTail;
}AcdbDynamicDataNodeType;

typedef struct AcdbDynamicTopologyStruct{
   uint32_t ulModuleId;
   AcdbDynamicUniqueDataType *pDataNode;   
   struct AcdbDynamicTopologyStruct *pNext;
}AcdbDynamicTopologyType;

typedef struct AcdbDynamicTopologyNodeStruct{
   AcdbDynamicTopologyType *pTopHead;
   AcdbDynamicTopologyType *pTopTail;
}AcdbDynamicTopologyNodeType;

typedef struct AcdbDynamicTblStruct{
   AcdbDataLookupKeyType *pKey;
   struct AcdbDynamicTopologyNodeStruct *pTopologyNode;
   struct AcdbDynamicTblStruct *pNext;
}AcdbDynamicTblType;

typedef struct AcdbDynamicTblNodeStruct{
   AcdbDynamicTblType *pTblHead;
   AcdbDynamicTblType *pTblTail;
}AcdbDynamicTblNodeType;

typedef struct AcdbDynamicGeneralInfoStruct {
   AcdbDataGeneralInfoType *pOEMInfo;
   AcdbDataGeneralInfoType *pDateInfo;
}AcdbDynamicGeneralInfoType;

typedef struct AcdbDynamicAdieTblStruct{
   AcdbDataLookupKeyType *pKey;
   AcdbDynamicUniqueDataType *pDataNode; 
   struct AcdbDynamicAdieTblStruct *pNext;
}AcdbDynamicAdieTblType;

typedef struct AcdbDynamicAdieTblNodeStruct{
   AcdbDynamicAdieTblType *pAdieTblHead;
   AcdbDynamicAdieTblType *pAdieTblTail;
}AcdbDynamicAdieTblNodeType;

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

int32_t IsDataNodeOnHeap(uint32_t *pParamId,
                         AcdbDynamicDataNodeType *pUniqDataOnHeap
                         );

int32_t IsDataOnHeap(uint32_t *ulParamId,
                     uint8_t *pUniqueData,
                     const uint32_t ulDataLen,
                     AcdbDynamicDataNodeType *pUniqDataOnHeap
                     );

int32_t FindTableNodeOnHeap(AcdbDataLookupKeyType *pKey,
                            AcdbDynamicTblNodeType *pTblOnHeap,
                            AcdbDynamicTblType **ppTblNode
                            );

int32_t FindTopologyNodeOnHeap(uint32_t *pModuleId,
                               uint32_t *pParamId,
                               AcdbDynamicTblType *pTblOnHeap,
                               AcdbDynamicTopologyType **ppTblNode
                               );

int32_t FindDataNodeOnHeap(uint32_t *pParamId,
                           uint8_t *pInData,
                           const uint32_t InDataLen,
                           AcdbDynamicDataNodeType *pUniqDataOnHeap,
                           AcdbDynamicUniqueDataType **ppDataNode
                           );

int32_t CreateDataNodeOnHeap(uint32_t *pParamId, 
                             uint8_t *pInData,
                             const uint32_t InDataLen,
                             AcdbDynamicDataNodeType *pUniqDataNode,
                             AcdbDynamicUniqueDataType **pDataNode
                             );

int32_t CreateTableNodeOnHeap(AcdbDataLookupKeyType *pKey,
                              AcdbDynamicTblNodeType *pTblOnHeap,
                              AcdbDynamicTblType **pTblNode
                              );

int32_t CreateTopologyNodeOnHeap(uint32_t *pModuleId,
                                 uint32_t *pParamId,
                                 AcdbDynamicUniqueDataType *pDataNode,
                                 AcdbDynamicTblType *pTbNodelOnHeap
                                 );

int32_t FreeTopologyNode(uint32_t *pModuleId,
                         uint32_t *pParamId,
                         AcdbDynamicTblType *pTblNode,
                         uint32_t *fReeTblResult
                         );

int32_t FreeTableNode(AcdbDataLookupKeyType *pKey,
                      AcdbDynamicTblNodeType *pTblOnHeap
                      );

int32_t FreeDataNode(uint32_t *pParamId,
                     AcdbDynamicDataNodeType *pDataOnHeap
                     );

int32_t CompareStaticData(uint32_t *pModuleId,
                          uint32_t *pParamId,
                          AcdbDataUniqueDataNodeType **ppCalTbl,
                          AcdbDataTopologyType *pTopology,
                          const uint32_t nTopologyEntries,
                          uint8_t *pInBufPtr,
                          const uint32_t InBufLen
                          );

int32_t GetDataCal(AcdbDataLookupKeyType *pKey,
                   uint32_t *pModuleId,
                   uint32_t *pParamId,
                   AcdbDynamicTblNodeType *pTbl,
                   AcdbDynamicUniqueDataType **ppDataNode
                   );

int32_t IsInfoDataOnHeap(AcdbDataGeneralInfoType* pInput,
                         AcdbDataGeneralInfoType* pInfoData
                         );

int32_t GetInfoDataNodeOnHeap(AcdbDataGeneralInfoType* pInput,
                              AcdbDataGeneralInfoType* pInfoData
                              );

int32_t CreateInfoDataNodeOnHeap(AcdbDataGeneralInfoType* pInput,
                                 AcdbDataGeneralInfoType** ppInfoData
                                 );

int32_t FreeInfoDataNodeOnHeap(AcdbDataGeneralInfoType** ppInfoData
                               );

int32_t FindAdieTableNodeOnHeap(AcdbDataLookupKeyType *pKey,
                                AcdbDynamicAdieTblNodeType *pTblOnHeap,
                                AcdbDynamicAdieTblType **ppTblNode
                                );

int32_t FreeAdieTableNode(AcdbDataLookupKeyType *pKey,
                          AcdbDynamicAdieTblNodeType *pTblOnHeap
                         );

int32_t CreateAdieTableNodeOnHeap(AcdbDataLookupKeyType *pKey,
                                  AcdbDynamicUniqueDataType *pDataNode,
                                  AcdbDynamicAdieTblNodeType *pTblOnHeap
                                  );

#endif//ACDB_LINKED_LIST_H
