#ifndef __ACDB_OVERRIDE_H__
#define __ACDB_OVERRIDE_H__
/*===========================================================================
@file acdb_override.h

Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_override.h#7 $

when who what, where, why
-------- --- ----------------------------------------------------------
05/28/14 mh SW migration from 32-bit to 64-bit architecture
07/23/10 ernanl introduce ACDB heap optimization API
06/02/10 aas Initial revision.

===========================================================================*/

/* ---------------------------------------------------------------------------
* Include Files
*--------------------------------------------------------------------------- */

#include "acdb_os_includes.h"
#include "acdb_linked_list.h"

/* ---------------------------------------------------------------------------
* Preprocessor Definitions and Constants
*--------------------------------------------------------------------------- */

#define ACDB_HEAP_DATA_NOT_FOUND 0x00000001
#define ACDB_HEAP_DATA_FOUND 0x00000002
#define ACDB_HEAP_TABLE_NOT_FOUND 0x00000003
#define ACDB_HEAP_TABLE_FOUND 0x00000004

#define ACDB_SYS_RESET 0xACDBD000
#define ACDB_GET_DATA 0xACDBD001
#define ACDB_SET_DATA 0xACDBD002
#define ACDB_GET_ADIE_TABLE 0xACDBD003
#define ACDB_SET_ADIE_TABLE 0xACDBD004

#define ACDB_GET_NO_OF_TBL_ENTRIES_ON_HEAP 0xACDBD005
#define ACDB_GET_TBL_ENTRIES_ON_HEAP 0xACDBD006

#define ACDB_GET_DELTA_FILE_LENGTH 0xACDBD007
#define ACDB_GET_DELTA_FILE_DATA 0xACDBD008

/*------------------------------------------------------------------------------
Target specific definitions
------------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
* Type Declarations
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Function Declarations and Documentation
*--------------------------------------------------------------------------- */
int32_t Acdb_SetDataCal(uint32_t persistData,
	uint32_t fileIndex,
	AcdbDataLookupKeyType *pKey,
	uint32_t *pModuleId,
	uint32_t *pParamId,
	const uint8_t *pFileDataBuf,
	const uint32_t nFileDataLen,
	uint8_t *pInputBufPtr,
	uint32_t InputBufLen,
	uint8_t *pIndices,
	uint32_t nIndexCount
	);
int32_t Acdb_SetDataCalEx(uint32_t persistData,
	uint32_t fileIndex,
	AcdbDataLookupCVDKeyType *pKey,
	uint32_t *pModuleId,
	uint32_t *pParamId,
	const uint8_t *pFileDataBuf,
	const uint32_t nFileDataLen,
	uint8_t *pInputBufPtr,
	uint32_t InputBufLen,
	uint8_t *pIndices,
	uint32_t nIndexCount
	);
int32_t Acdb_GetDataCal(AcdbDataLookupKeyType *pKey,
	uint32_t *pModuleId,
	uint32_t *pParamId,
	AcdbDynamicUniqueDataType **ppDataNode
	);
int32_t Acdb_GetDataCalEx(AcdbDataLookupCVDKeyType *pKey,
	uint32_t *pModuleId,
	uint32_t *pParamId,
	AcdbDynamicUniqueDataType **ppDataNode
	);
int32_t Acdb_SetAdieTableCal(uint32_t persistData,
	uint32_t fileIndex,
	AcdbDataLookupKeyType *pKey,
	uint8_t *pInputBufPtr,
	const uint32_t InputBufLen,
	uint8_t *pIndices,
	uint32_t nIndexCount
	);

int32_t Acdb_GetAdieTableCal(AcdbDataLookupKeyType *pKey,
	AcdbDynamicUniqueDataType **ppDataNode
	);

int32_t Acdb_sys_reset(void);

int32_t Acdb_GetNoOfTblEntries(uint8_t *pCmd,uint32_t nCmdSize ,uint8_t *pRsp,size_t nRspSizse);

/* ---------------------------------------------------------------------------
* Externalized Function Declarations and Definitions
*--------------------------------------------------------------------------- */

//int32_t Acdb_ChecktoFreeAdieTableCalOnHeap(AcdbDataLookupKeyType *pKey,
// uint8_t *pInputBufPtr,
// const uint32_t InputBufLen
// );

int32_t Acdb_DM_Ioctl(uint32_t Acdb_DM_CMD_Id,
	uint32_t persistData,
	uint32_t deltaFileIndex,
	AcdbDataLookupKeyType *pKey,
	uint32_t *pModuleId,
	uint32_t *pParamId,
	uint8_t *pInputBuf,
	const uint32_t InputBufLen,
	const uint8_t *pFileDataBuf,
	const uint32_t nFileDataLen,
	uint8_t *pOutBuff,
	const size_t nOutBuffLen,
	uint32_t *nOutBuffBytesUsed,
	AcdbDynamicUniqueDataType **ppDataNode,
	uint8_t* pIndices,
	uint32_t nIndexCount
	);
int32_t Acdb_DM_IoctlEx(uint32_t Acdb_DM_CMD_Id,
	uint32_t persistData,
	uint32_t fileIndex,
	AcdbDataLookupCVDKeyType *pKey,
	uint32_t *pModuleId,
	uint32_t *pParamId,
	uint8_t *pInputBuf,
	const uint32_t InputBufLen,
	const uint8_t *pFileDataBuf,
	const uint32_t nFileDataLen,
	uint8_t *pOutBuff,
	const size_t nOutBuffLen,
	uint32_t *nOutBuffBytesUsed,
	AcdbDynamicUniqueDataType **ppDataNode,
	uint8_t* pIndices,
	uint32_t nIndexCount
	);
#endif /* __ACDB_OVERRIDE_H__ */
