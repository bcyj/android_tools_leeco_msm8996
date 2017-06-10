#ifndef __ACDB_OVERRIDE_H__
#define __ACDB_OVERRIDE_H__
/*===========================================================================
    @file   acdb_override.h

                    Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal2/common/8k_PL/acdb/rel/4.2/src/acdb_override.h#1 $

/*===========================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/23/10   ernanl  introduce ACDB heap optimization API
06/02/10   aas     Initial revision.

===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_includes.h"
#include "acdb_default_data.h"
#include "acdb_linked_list.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

#define ACDB_HEAP_TABLE_NOT_FOUND            0x00000003
#define ACDB_HEAP_TABLE_FOUND                0x00000004
#define ACDB_HEAP_DATA_NOT_FOUND             0x00000000
#define ACDB_HEAP_DATA_FOUND                 0x00000005

#define ACDB_DYNAMIC_DATA                    0x00000001
#define ACDB_STATIC_DATA                     0x00000002

#define ACDB_SYS_RESET                       0xACDBD000
#define ACDB_GET_TABLE                       0xACDBD001
#define ACDB_SET_TABLE                       0xACDBD002
#define ACDB_GET_DATA                        0xACDBD003
#define ACDB_SET_DATA                        0xACDBD004
#define ACDB_GET_VOLUME_TABLE                0xACDBD005
#define ACDB_SET_VOLUME_TABLE                0xACDBD006
#define ACDB_GET_OEM_INFO                    0xACDBD007
#define ACDB_SET_OEM_INFO                    0xACDBD008
#define ACDB_GET_DATE_INFO                   0xACDBD009
#define ACDB_SET_DATE_INFO                   0xACDBD00A
#define ACDB_GET_ADIE_TABLE                  0xACDBD00B
#define ACDB_SET_ADIE_TABLE                  0xACDBD00C
#define ACDB_SET_TOPOLOGY_OVERRIDE_STATUS    0xACDBD00D
#define ACDB_GET_TOPOLOGY_OVERRIDE_STATUS    0xACDBD00E
#define ACDB_SET_TOPOLOGY_LIST			     0xACDBD00F
#define ACDB_GET_TOPOLOGY_LIST			     0xACDBD010
#define ACDB_SET_AUDTOPO_CHANGE_DEVICE			0xACDBD011
#define ACDB_GET_IS_AUDTOPO_CHANGE_DEVICE		0xACDBD012
#define ACDB_SET_VOCTOPO_CHANGE_DEVICE			0xACDBD013
#define ACDB_GET_IS_VOCTOPO_CHANGE_DEVICE		0xACDBD014
#define ACDB_SET_AFECMNTOPO_CHANGE_DEVICE		0xACDBD015
#define ACDB_GET_IS_AFECMNTOPO_CHANGE_DEVICE	0xACDBD016
#define ACDB_CLEAR_TOPOLOGY_OVERRIDE_HEAP_DATA  0xACDBD017
#define ACDB_SET_TOPOLOGY_OVERRIDE_SUPPORT    0xACDBD018
#define ACDB_GET_TOPOLOGY_OVERRIDE_SUPPORT    0xACDBD019


#define TOPOLOGY_OVERRIDE_STATUS_NOT_SET -1
#define TOPOLOGY_OVERRIDE_ACTIVATED 1
#define TOPOLOGY_OVERRIDE_DEACTIVATED 0

#define TOPOLOGY_OVERRIDE_SUPPORTED 1
#define TOPOLOGY_OVERRIDE_NOT_SUPPORTED 0


/*------------------------------------------------------------------------------
Target specific definitions
------------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */
int32_t Acdb_SetDataCal(AcdbDataLookupKeyType *pKey,
                        uint32_t *pModuleId,
                        uint32_t *pParamId,
                        uint8_t *pInputBufPtr,
                        uint32_t InputBufLen,
                        AcdbDataUniqueDataNodeType **ppTable,
                        AcdbDataTopologyType *pTopology,
                        const uint32_t nTopologyEntries
                        );

int32_t Acdb_GetDataCal(AcdbDataLookupKeyType *pKey,
                        uint32_t *pModuleId,
                        uint32_t *pParamId,
                        AcdbDynamicUniqueDataType **ppDataNode
                        );

int32_t Acdb_SetTableCal(AcdbDataLookupKeyType *pKey,
                         AcdbDataGetTblTopType *pTbltop,
                         uint8_t *pInputBufPtr,
                         const uint32_t InputBufLen,
                         AcdbDynamicTblNodeType *pTbl,
                         AcdbDynamicDataNodeType *pData
                         );

int32_t Acdb_GetTableCal(AcdbDataLookupKeyType *pKey,
                         AcdbDynamicTblNodeType *pTbl,
                         AcdbDynamicDataNodeType *pData,
                         AcdbDataGetTblTopType *pTbltop,
                         uint8_t *pDst,
                         const uint32_t ulDstsize,
                         uint32_t *pBytesUsed
                         );

int32_t Acdb_SetAdieTableCal(AcdbDataLookupKeyType *pKey,
                             uint8_t *pInputBufPtr,
                             const uint32_t InputBufLen
                             );

int32_t Acdb_GetAdieTableCal(AcdbDataLookupKeyType *pKey,
                             AcdbDynamicUniqueDataType **ppDataNode
                             );

int32_t Acdb_sys_reset(void);

int32_t Acdb_GetOEMInfo(AcdbDataGeneralInfoType *pInput
                        );

int32_t Acdb_SetOEMInfo(AcdbDataGeneralInfoType *pInput
                        );

int32_t Acdb_GetDateInfo(AcdbDataGeneralInfoType *pInput
                         );

int32_t Acdb_SetDateInfo(AcdbDataGeneralInfoType *pInput
                         );

int32_t Acdb_IsTopologyOverrideSupported();

/* ---------------------------------------------------------------------------
 * Externalized Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

int32_t AcdbDataMemoryRamEstimate(AcdbDataMemoryUsageType *ram
                                  );

int32_t Acdb_ChecktoFreeAdieTableCalOnHeap(AcdbDataLookupKeyType *pKey,
                                           uint8_t *pInputBufPtr,
                                           const uint32_t InputBufLen
                                           );

int32_t Acdb_DM_Ioctl(uint32_t Acdb_DM_CMD_Id,
                      AcdbDataLookupKeyType *pKey,
                      uint32_t *pModuleId,
                      uint32_t *pParamId,
                      AcdbDataGetTblTopType *tbltop,                     
                      uint8_t *pInputBuf,
                      const uint32_t InputBufLen,
                      uint8_t *pBytesUsed,
                      AcdbDynamicUniqueDataType **ppDataNode
                      );

#endif /* __ACDB_OVERRIDE_H__ */
