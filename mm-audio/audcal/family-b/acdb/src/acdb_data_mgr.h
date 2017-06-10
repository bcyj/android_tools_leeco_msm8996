#ifndef __ACDB_DATA_MGR_H__
#define __ACDB_DATA_MGR_H__
/*===========================================================================
@file acdb_data_mgr.h

The interface to the accessing the acdb data.

This file will provide API access acdb data.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_data_mgr.h#8 $ */

/* ---------------------------------------------------------------------------
* Include Files
*--------------------------------------------------------------------------- */
#include "acdb_os_includes.h"
#include "acdb_datainfo.h"
#include "acdb_file_mgr.h"
/* ---------------------------------------------------------------------------
* Preprocessor Definitions and Constants
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Type Declarations
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Function Declarations and Documentation
*--------------------------------------------------------------------------- */

int32_t GetMidPidIndex(ContentDefTblType cdefTbl,uint32_t mid,uint32_t pid,uint32_t *index);
int32_t GetCalibData(uint32_t tblId,uintptr_t nLookupKey,uint32_t dataOffset,AcdbDataInfo dataPoolChnk,
	uint8_t *pDstBuff, uint32_t nDstBuffLen,uint32_t *pDstBytesUsed);
int32_t SetCalibData(uint32_t persistData, uint32_t deltaFileIndex, uint32_t tblId,uintptr_t nLookupKey, uint8_t *pIndices,uint32_t nIdxCount, uint32_t dataOffset,AcdbDataInfo dataPoolChnk,
	uint8_t *pDstBuff, uint32_t nDstBuffLen);
int32_t GetMidPidCalibData(uint32_t tblId,uintptr_t nLookupKey,ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,AcdbDataInfo dataPoolChnk,
	uint32_t mid,uint32_t pid,uint8_t *pDstBuff, uint32_t nDstBuffLen,
	uint32_t *pDstBytesUsed);
int32_t GetMidPidCalibCVDData(uint32_t tblId,uintptr_t nLookupKey,uintptr_t nSecLookupKey,ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,AcdbDataInfo dataPoolChnk,
	uint32_t mid,uint32_t pid,uint8_t *pDstBuff, uint32_t nDstBuffLen,
	uint32_t *pDstBytesUsed);
int32_t GetMidPidCalibTable(uint32_t tblId,uintptr_t nLookupKey,ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,AcdbDataInfo dataPoolChnk,
	uint8_t *pDstBuff, uint32_t nDstBuffLen,uint32_t *pDstBytesUsed);
int32_t GetMidPidCalibHeapData(uint32_t tblId,uintptr_t nLookupKey,uint32_t mid,uint32_t pid,
	AcdbDataType *cData);
int32_t GetMidPidCalibHeapDataEx(uint32_t tblId,uintptr_t nLookupKey,uintptr_t nSecLookupKey,uint32_t mid,uint32_t pid,AcdbDataType *cData);
int32_t GetMidPidCalibTableSize(uint32_t tblId,uintptr_t nLookupKey,ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,AcdbDataInfo dataPoolChnk,
	uint32_t *pSize);


int32_t SetMidPidCalibData(uint32_t persistData, uint32_t deltaFileIndex, uint32_t tblId,uintptr_t nLookupKey, uint8_t *pIndices,uint32_t nIdxCount, ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,
	AcdbDataInfo dataPoolChnk, uint32_t mid,uint32_t pid,uint8_t *pInBuff, uint32_t nInBuffLen);
int32_t SetMidPidCalibCVDData(uint32_t persistData, uint32_t deltaFileIndex, uint32_t tblId,uintptr_t nLookupKey,uintptr_t nSecLookupKey, uint8_t *pIndices,uint32_t nIdxCount, ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,
	AcdbDataInfo dataPoolChnk, uint32_t mid,uint32_t pid,uint8_t *pInBuff, uint32_t nInBuffLen);
int32_t GetNoOfTblEntriesOnHeap(uint8_t *pCmd,uint32_t nCmdSize ,uint8_t *pRsp,uint32_t nRspSizse);

int32_t GetTblEntriesOnHeap(uint8_t *pCmd,uint32_t nCmdSize ,uint8_t *pRsp,uint32_t nRspSizse);
int32_t ACDBHeapReset(void);

#endif /* __ACDB_DATA_MGR_H__ */
