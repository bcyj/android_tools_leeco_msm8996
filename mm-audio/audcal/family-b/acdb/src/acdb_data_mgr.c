/*===========================================================================
FILE: acdb_init_utility.c

OVERVIEW: This file contains the acdb init utility functions
implemented specifically in the win32 environment.

DEPENDENCIES: None

Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order. Please
use ISO format for dates.

$Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_data_mgr.c#8 $

when who what, where, why
---------- --- -----------------------------------------------------
2014-05-28 mh SW migration from 32-bit to 64-bit architecture
2010-07-08 vmn Initial revision.

========================================================================== */

/* ---------------------------------------------------------------------------
* Include Files
*--------------------------------------------------------------------------- */

#include "acdb.h"
#include "acdb_data_mgr.h"
#include "acdb_override.h"
#include "acdb_datainfo.h"

#define UNREFERENCED_VAR(param) (param)

int32_t GetMidPidIndex(ContentDefTblType cdefTbl,uint32_t mid,uint32_t pid,uint32_t *index)
{
	uint32_t i;
	uint8_t blnFound = 0;
	int32_t result = ACDB_ERROR;
	for(i=0;i<cdefTbl.nLen;i++)
	{
		if(cdefTbl.pCntDef[i].nMid == mid &&
			cdefTbl.pCntDef[i].nPid == pid)
		{
			blnFound = 1;
			*index = i;
			break;
		}
	}
	if(blnFound == 1)
		result = ACDB_SUCCESS;
	return result;
}

int32_t GetCalibData(uint32_t tblId,uintptr_t nLookupKey,uint32_t dataOffset,AcdbDataInfo dataPoolChnk,
	uint8_t *pDstBuff, uint32_t nDstBuffLen,uint32_t *pDstBytesUsed)
{
	AcdbDataType cData;
	AcdbDataLookupKeyType dataLookupKey;
	AcdbDynamicUniqueDataType *pDataNode = NULL;
	uint32_t result = ACDB_SUCCESS;

	dataLookupKey.nTableId = tblId;
	dataLookupKey.nLookupKey = nLookupKey;

	if(dataPoolChnk.pData == NULL)
	{
		ACDB_DEBUG_LOG("Datapool table not provided to look for the data\n");
		return ACDB_ERROR;
	}
	if(dataOffset >= dataPoolChnk.nDataLen)
	{
		ACDB_DEBUG_LOG("Invalid dataoffset provided to retrieve the data from datapool table\n");
		return ACDB_ERROR;
	}
	// Now get the datalen and data pointer

	result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_ADIE_TABLE,
		FALSE,
		0,
		(AcdbDataLookupKeyType*) &dataLookupKey,
		NULL,
		NULL,
		NULL,
		0,
		NULL,
		0,
		NULL,
		0,
		NULL,
		(AcdbDynamicUniqueDataType**) &pDataNode,
		NULL,
		0
		);

	if(result == ACDB_SUCCESS)
	{
		if(pDataNode == NULL)
		{
			ACDB_DEBUG_LOG("Issue with heap, Unable to retrieve data from Heap\n");
			return ACDB_ERROR;
		}
		cData.nLen = pDataNode->ulDataLen;
		cData.pData = pDataNode->ulDataBuf;
	}
	else
	{
		// Now get the datalen and data pointer
		cData.nLen = *((uint32_t *)(dataPoolChnk.pData + dataOffset));
		cData.pData = dataPoolChnk.pData + dataOffset + sizeof(cData.nLen);
	}

	if(nDstBuffLen < cData.nLen)
	{
		ACDB_DEBUG_LOG("insufficient memory provided to copy the requested data\n");
		return ACDB_INSUFFICIENTMEMORY;
	}

	ACDB_MEM_CPY ((void *) pDstBuff,nDstBuffLen,(const void *)cData.pData,cData.nLen);
	*pDstBytesUsed = cData.nLen;

	return ACDB_SUCCESS;
}

int32_t SetCalibData(uint32_t persistData, uint32_t deltaFileIndex, uint32_t tblId,uintptr_t nLookupKey, uint8_t *pIndices,uint32_t nIdxCount, uint32_t dataOffset,AcdbDataInfo dataPoolChnk,
	uint8_t *pInBuff, uint32_t nInBuffLen)
{
	AcdbDataLookupKeyType dataLookupKey;
	AcdbDynamicUniqueDataType *pDataNode = NULL;
	uint32_t result = ACDB_SUCCESS;

	dataLookupKey.nTableId = tblId;
	dataLookupKey.nLookupKey = nLookupKey;

	if(dataPoolChnk.pData == NULL)
	{
		ACDB_DEBUG_LOG("Datapool table not provided to look for the data\n");
		return ACDB_ERROR;
	}
	if(dataOffset >= dataPoolChnk.nDataLen)
	{
		ACDB_DEBUG_LOG("Invalid dataoffset provided to retrieve the data from datapool table\n");
		return ACDB_ERROR;
	}
	// Now get the datalen and data pointer

	result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_ADIE_TABLE,
		persistData,
		deltaFileIndex,
		(AcdbDataLookupKeyType*) &dataLookupKey,
		NULL,
		NULL,
		pInBuff,
		nInBuffLen,
		NULL,
		0,
		NULL,
		0,
		NULL,
		(AcdbDynamicUniqueDataType**) &pDataNode,
		pIndices,
		nIdxCount
		);

	if(result != ACDB_SUCCESS)
	{
		ACDB_DEBUG_LOG("Failed to do set operation\n");
		return result;
	}

	return ACDB_SUCCESS;
}

int32_t GetMidPidCalibData(uint32_t tblId,uintptr_t nLookupKey,ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,AcdbDataInfo dataPoolChnk,
	uint32_t mid,uint32_t pid,uint8_t *pDstBuff, uint32_t nDstBuffLen,
	uint32_t *pDstBytesUsed)
{
	uint32_t mpindex;
	AcdbDataType cData;
	uint32_t result = ACDB_SUCCESS;
	AcdbDataLookupKeyType dataLookupKey;
	AcdbDynamicUniqueDataType *pDataNode = NULL;
	if(cdefTbl.nLen != cdotTbl.nLen)
	{
		ACDB_DEBUG_LOG("The no of entries in CDEF and CDOT tables are not matching\n");
		return ACDB_ERROR;
	}

	if(dataPoolChnk.pData == NULL || cdefTbl.pCntDef == NULL || cdotTbl.pDataOffsets == NULL)
	{
		ACDB_DEBUG_LOG("Invalid tables provided to retrieve data\n");
		return ACDB_ERROR;
	}

	// Now find the (mid,pid) index from the CDef table
	if(ACDB_SUCCESS != GetMidPidIndex(cdefTbl,mid,pid,&mpindex))
	{
		ACDB_DEBUG_LOG("mid %08X and pid %08X not found\n",mid,pid);
		return ACDB_PARMNOTFOUND;
	}

	if(mpindex >= dataPoolChnk.nDataLen)
	{
		ACDB_DEBUG_LOG("Invalid dataoffset provided to retrieve the data from datapool table\n");
		return ACDB_ERROR;
	}

	dataLookupKey.nTableId = tblId;
	dataLookupKey.nLookupKey = nLookupKey;
	result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
		FALSE,
		0,
		(AcdbDataLookupKeyType*) &dataLookupKey,
		(uint32_t*) &mid,
		(uint32_t*) &pid,
		NULL,
		0,
		NULL,
		0,
		NULL,
		0,
		NULL,
		(AcdbDynamicUniqueDataType**) &pDataNode,
		NULL,
		0
		);

	if(result == ACDB_SUCCESS)
	{
		if(pDataNode == NULL)
		{
			ACDB_DEBUG_LOG("Issue with heap, Unable to retrieve data from Heap\n");
			return ACDB_ERROR;
		}
		cData.nLen = pDataNode->ulDataLen;
		cData.pData = pDataNode->ulDataBuf;
	}
	else
	{
		// Now get the datalen and data pointer
		cData.nLen = *((uint32_t *)(dataPoolChnk.pData + cdotTbl.pDataOffsets[mpindex]));
		cData.pData = dataPoolChnk.pData + cdotTbl.pDataOffsets[mpindex] + sizeof(cData.nLen);
	}

	if(nDstBuffLen < cData.nLen)
	{
		ACDB_DEBUG_LOG("insufficient memory provided to copy the requested data\n");
		return ACDB_INSUFFICIENTMEMORY;
	}

	ACDB_MEM_CPY ((void *) pDstBuff,nDstBuffLen,(const void *)cData.pData,cData.nLen);
	*pDstBytesUsed = cData.nLen;

	return ACDB_SUCCESS;
}
int32_t GetMidPidCalibCVDData(uint32_t tblId,uintptr_t nLookupKey,uintptr_t nSecLookupKey,ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,AcdbDataInfo dataPoolChnk,
	uint32_t mid,uint32_t pid,uint8_t *pDstBuff, uint32_t nDstBuffLen,
	uint32_t *pDstBytesUsed)
{
	uint32_t mpindex;
	AcdbDataType cData;
	uint32_t result = ACDB_SUCCESS;
	AcdbDataLookupCVDKeyType dataLookupKey;
	AcdbDynamicUniqueDataType *pDataNode = NULL;
	if(cdefTbl.nLen != cdotTbl.nLen)
	{
		ACDB_DEBUG_LOG("The no of entries in CDEF and CDOT tables are not matching\n");
		return ACDB_ERROR;
	}

	if(dataPoolChnk.pData == NULL || cdefTbl.pCntDef == NULL || cdotTbl.pDataOffsets == NULL)
	{
		ACDB_DEBUG_LOG("Invalid tables provided to retrieve data\n");
		return ACDB_ERROR;
	}

	// Now find the (mid,pid) index from the CDef table
	if(ACDB_SUCCESS != GetMidPidIndex(cdefTbl,mid,pid,&mpindex))
	{
		ACDB_DEBUG_LOG("mid %08X and pid %08X not found\n",mid,pid);
		return ACDB_PARMNOTFOUND;
	}

	if(mpindex >= dataPoolChnk.nDataLen)
	{
		ACDB_DEBUG_LOG("Invalid dataoffset provided to retrieve the data from datapool table\n");
		return ACDB_ERROR;
	}

	dataLookupKey.nTableId = tblId;
	dataLookupKey.nLookupKey = nLookupKey;
	dataLookupKey.nSecondaryLookupKey = nSecLookupKey;
	result = Acdb_DM_IoctlEx((uint32_t) ACDB_GET_DATA,
		FALSE,
		0,
		(AcdbDataLookupCVDKeyType*) &dataLookupKey,
		(uint32_t*) &mid,
		(uint32_t*) &pid,
		NULL,
		0,
		NULL,
		0,
		NULL,
		0,
		NULL,
		(AcdbDynamicUniqueDataType**) &pDataNode,
		NULL,
		0
		);

	if(result == ACDB_SUCCESS)
	{
		if(pDataNode == NULL)
		{
			ACDB_DEBUG_LOG("Issue with heap, Unable to retrieve data from Heap\n");
			return ACDB_ERROR;
		}
		cData.nLen = pDataNode->ulDataLen;
		cData.pData = pDataNode->ulDataBuf;
	}
	else
	{
		// Now get the datalen and data pointer
		cData.nLen = *((uint32_t *)(dataPoolChnk.pData + cdotTbl.pDataOffsets[mpindex]));
		cData.pData = dataPoolChnk.pData + cdotTbl.pDataOffsets[mpindex] + sizeof(cData.nLen);
	}

	if(nDstBuffLen < cData.nLen)
	{
		ACDB_DEBUG_LOG("insufficient memory provided to copy the requested data\n");
		return ACDB_INSUFFICIENTMEMORY;
	}

	ACDB_MEM_CPY ((void *) pDstBuff,nDstBuffLen,(const void *)cData.pData,cData.nLen);
	*pDstBytesUsed = cData.nLen;

	return ACDB_SUCCESS;
}
int32_t SetMidPidCalibCVDData(uint32_t persistData, uint32_t deltaFileIndex, uint32_t tblId,uintptr_t nLookupKey,uintptr_t nSecLookupKey, uint8_t *pIndices,uint32_t nIdxCount, ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,
	AcdbDataInfo dataPoolChnk, uint32_t mid,uint32_t pid,uint8_t *pInBuff, uint32_t nInBuffLen)
{
	uint32_t result = ACDB_SUCCESS;
	uint32_t mpindex;
	AcdbDataType cData;
	AcdbDataLookupCVDKeyType dataLookupKey;
	if(cdefTbl.nLen != cdotTbl.nLen)
	{
		ACDB_DEBUG_LOG("The no of entries in CDEF and CDOT tables are not matching\n");
		return ACDB_ERROR;
	}

	if(dataPoolChnk.pData == NULL || cdefTbl.pCntDef == NULL || cdotTbl.pDataOffsets == NULL)
	{
		ACDB_DEBUG_LOG("Invalid tables provided to retrieve data\n");
		return ACDB_ERROR;
	}

	// Now find the (mid,pid) index from the CDef table
	if(ACDB_SUCCESS != GetMidPidIndex(cdefTbl,mid,pid,&mpindex))
	{
		ACDB_DEBUG_LOG("mid %08X and pid %08X not found\n",mid,pid);
		return ACDB_ERROR;
	}

	if(mpindex >= dataPoolChnk.nDataLen)
	{
		ACDB_DEBUG_LOG("Invalid dataoffset provided to retrieve the data from datapool table\n");
		return ACDB_ERROR;
	}

	// Now get the datalen and data pointer
	cData.nLen = *((uint32_t *)(dataPoolChnk.pData + cdotTbl.pDataOffsets[mpindex]));
	cData.pData = dataPoolChnk.pData + cdotTbl.pDataOffsets[mpindex] + sizeof(cData.nLen);

	dataLookupKey.nLookupKey = nLookupKey;
	dataLookupKey.nSecondaryLookupKey = nSecLookupKey;
	dataLookupKey.nTableId = tblId;
	result = Acdb_DM_IoctlEx((uint32_t) ACDB_SET_DATA,
		persistData,
		deltaFileIndex,
		(AcdbDataLookupCVDKeyType*) &dataLookupKey,
		(uint32_t*) &mid,
		(uint32_t*) &pid,
		pInBuff,
		nInBuffLen,
		(const uint8_t *)cData.pData,
		cData.nLen,
		NULL,
		0,
		NULL,
		NULL,
		pIndices,
		nIdxCount
		);

	if(result != ACDB_SUCCESS)
	{
		ACDB_DEBUG_LOG("Failed to do set operation\n");
		return result;
	}

	return ACDB_SUCCESS;
}
int32_t SetMidPidCalibData(uint32_t persistData, uint32_t deltaFileIndex, uint32_t tblId,uintptr_t nLookupKey, uint8_t *pIndices,uint32_t nIdxCount, ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,
	AcdbDataInfo dataPoolChnk, uint32_t mid,uint32_t pid,uint8_t *pInBuff, uint32_t nInBuffLen)
{
	uint32_t result = ACDB_SUCCESS;
	uint32_t mpindex;
	AcdbDataType cData;
	AcdbDataLookupKeyType dataLookupKey;
	if(cdefTbl.nLen != cdotTbl.nLen)
	{
		ACDB_DEBUG_LOG("The no of entries in CDEF and CDOT tables are not matching\n");
		return ACDB_ERROR;
	}

	if(dataPoolChnk.pData == NULL || cdefTbl.pCntDef == NULL || cdotTbl.pDataOffsets == NULL)
	{
		ACDB_DEBUG_LOG("Invalid tables provided to retrieve data\n");
		return ACDB_ERROR;
	}

	// Now find the (mid,pid) index from the CDef table
	if(ACDB_SUCCESS != GetMidPidIndex(cdefTbl,mid,pid,&mpindex))
	{
		ACDB_DEBUG_LOG("mid %08X and pid %08X not found\n",mid,pid);
		return ACDB_ERROR;
	}

	if(mpindex >= dataPoolChnk.nDataLen)
	{
		ACDB_DEBUG_LOG("Invalid dataoffset provided to retrieve the data from datapool table\n");
		return ACDB_ERROR;
	}

	// Now get the datalen and data pointer
	cData.nLen = *((uint32_t *)(dataPoolChnk.pData + cdotTbl.pDataOffsets[mpindex]));
	cData.pData = dataPoolChnk.pData + cdotTbl.pDataOffsets[mpindex] + sizeof(cData.nLen);

	dataLookupKey.nLookupKey = nLookupKey;
	dataLookupKey.nTableId = tblId;
	result = Acdb_DM_Ioctl((uint32_t) ACDB_SET_DATA,
		persistData,
		deltaFileIndex,
		(AcdbDataLookupKeyType*) &dataLookupKey,
		(uint32_t*) &mid,
		(uint32_t*) &pid,
		pInBuff,
		nInBuffLen,
		(const uint8_t *)cData.pData,
		cData.nLen,
		NULL,
		0,
		NULL,
		NULL,
		pIndices,
		nIdxCount
		);

	if(result != ACDB_SUCCESS)
	{
		ACDB_DEBUG_LOG("Failed to do set operation\n");
		return result;
	}

	return ACDB_SUCCESS;
}
int32_t GetMidPidCalibTable(uint32_t tblId,uintptr_t nLookupKey,ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,AcdbDataInfo dataPoolChnk,
	uint8_t *pDstBuff, uint32_t nDstBuffLen,uint32_t *pDstBytesUsed)
{
	uint32_t nMemBytesLeft=0;
	AcdbDataType cData;
	uint32_t i=0;
	uint32_t offset = 0;
	AcdbDataLookupKeyType dataLookupKey;
	AcdbDynamicUniqueDataType *pDataNode = NULL;

	dataLookupKey.nLookupKey = nLookupKey;
	dataLookupKey.nTableId = tblId;

	if(cdefTbl.nLen != cdotTbl.nLen)
	{
		ACDB_DEBUG_LOG("The no of entries in CDEF and CDOT tables are not matching\n");
		return ACDB_ERROR;
	}

	nMemBytesLeft = nDstBuffLen;
	for(i=0 ;i< cdefTbl.nLen;i++)
	{
		uint32_t nMemBytesRequired = 0;
		uint32_t nPaddedBytes = 0;
		DspCalHdrFormatType hdr;
		uint32_t mid = cdefTbl.pCntDef[i].nMid;
		uint32_t pid = cdefTbl.pCntDef[i].nPid;
		// Now get the datalen and data pointer
		uint32_t result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
			FALSE,
			0,
			(AcdbDataLookupKeyType*) &dataLookupKey,
			(uint32_t*) &mid,
			(uint32_t*) &pid,
			NULL,
			0,
			NULL,
			0,
			NULL,
			0,
			NULL,
			(AcdbDynamicUniqueDataType**) &pDataNode,
			NULL,
			0
			);

		if(result == ACDB_SUCCESS)
		{
			if(pDataNode == NULL)
			{
				ACDB_DEBUG_LOG("Issue with heap, Unable to retrieve data from Heap\n");
				return ACDB_ERROR;
			}
			cData.nLen = pDataNode->ulDataLen;
			cData.pData = pDataNode->ulDataBuf;
		}
		else
		{
			// Now get the datalen and data pointer
			cData.nLen = *((uint32_t *)(dataPoolChnk.pData + cdotTbl.pDataOffsets[i]));
			cData.pData = dataPoolChnk.pData + cdotTbl.pDataOffsets[i] + sizeof(cData.nLen);
		}

		if(cData.nLen%4)
		{
			nPaddedBytes = 4-cData.nLen%4;
		}

		nMemBytesRequired = sizeof(DspCalHdrFormatType) + cData.nLen + nPaddedBytes;

		if(nMemBytesLeft < nMemBytesRequired)
		{
			ACDB_DEBUG_LOG("insufficient memory provided to copy the requested data\n");
			return ACDB_INSUFFICIENTMEMORY;
		}

		hdr.nModuleId = cdefTbl.pCntDef[i].nMid;
		hdr.nParamId = cdefTbl.pCntDef[i].nPid;
		hdr.nParamSize = (uint16_t)(cData.nLen + nPaddedBytes);
		hdr.nReserved = 0;

		memset((void *)(pDstBuff+offset),0,nMemBytesRequired);
		ACDB_MEM_CPY ((void *) (pDstBuff+offset),nMemBytesLeft,(const void *)&hdr,sizeof(hdr));
		offset += sizeof(hdr);
		ACDB_MEM_CPY ((void *) (pDstBuff+offset),(nMemBytesLeft-(sizeof(hdr))),(const void *)cData.pData,cData.nLen);
		offset += cData.nLen + nPaddedBytes;
		nMemBytesLeft -= nMemBytesRequired;
	}
	*pDstBytesUsed = nDstBuffLen - nMemBytesLeft;
	return ACDB_SUCCESS;
}

int32_t GetMidPidCalibHeapDataEx(uint32_t tblId,uintptr_t nLookupKey,uintptr_t nSecLookupKey,uint32_t mid,uint32_t pid,AcdbDataType *cData)
{

	AcdbDataLookupCVDKeyType dataLookupKey;
	AcdbDynamicUniqueDataType *pDataNode = NULL;
	uint32_t result =0;
	dataLookupKey.nLookupKey = nLookupKey;
	dataLookupKey.nSecondaryLookupKey = nSecLookupKey;
	dataLookupKey.nTableId = tblId;



	// Now get the datalen and data pointer
	result = Acdb_DM_IoctlEx((uint32_t) ACDB_GET_DATA,
		FALSE,
		0,
		(AcdbDataLookupCVDKeyType*) &dataLookupKey,
		(uint32_t*) &mid,
		(uint32_t*) &pid,
		NULL,
		0,
		NULL,
		0,
		NULL,
		0,
		NULL,
		(AcdbDynamicUniqueDataType**) &pDataNode,
		NULL,
		0
		);

	if(result == ACDB_SUCCESS)
	{
		if(pDataNode == NULL)
		{
			ACDB_DEBUG_LOG("Issue with heap, Unable to retrieve data from Heap\n");
			return ACDB_ERROR;
		}
		cData->nLen = pDataNode->ulDataLen;
		cData->pData = pDataNode->ulDataBuf;
		return ACDB_SUCCESS;
	}
	else
	{
		return ACDB_ERROR;
	}

}


int32_t GetMidPidCalibHeapData(uint32_t tblId,uintptr_t nLookupKey,uint32_t mid,uint32_t pid,AcdbDataType *cData)
{

	AcdbDataLookupKeyType dataLookupKey;
	AcdbDynamicUniqueDataType *pDataNode = NULL;
	uint32_t result =0;
	dataLookupKey.nLookupKey = nLookupKey;
	dataLookupKey.nTableId = tblId;



	// Now get the datalen and data pointer
	result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
		FALSE,
		0,
		(AcdbDataLookupKeyType*) &dataLookupKey,
		(uint32_t*) &mid,
		(uint32_t*) &pid,
		NULL,
		0,
		NULL,
		0,
		NULL,
		0,
		NULL,
		(AcdbDynamicUniqueDataType**) &pDataNode,
		NULL,
		0
		);

	if(result == ACDB_SUCCESS)
	{
		if(pDataNode == NULL)
		{
			ACDB_DEBUG_LOG("Issue with heap, Unable to retrieve data from Heap\n");
			return ACDB_ERROR;
		}
		cData->nLen = pDataNode->ulDataLen;
		cData->pData = pDataNode->ulDataBuf;
		return ACDB_SUCCESS;
	}
	else
	{
		return ACDB_ERROR;
	}

}

int32_t GetMidPidCalibTableSize(uint32_t tblId,uintptr_t nLookupKey,ContentDefTblType cdefTbl,ContentDataOffsetsTblType cdotTbl,AcdbDataInfo dataPoolChnk,
	uint32_t *pSize)
{
	AcdbDataType cData;
	uint32_t i=0;
	uint32_t nSizeRequired=0;
	AcdbDataLookupKeyType dataLookupKey;
	AcdbDynamicUniqueDataType *pDataNode = NULL;

	dataLookupKey.nLookupKey = nLookupKey;
	dataLookupKey.nTableId = tblId;

	if(cdefTbl.nLen != cdotTbl.nLen)
	{
		ACDB_DEBUG_LOG("The no of entries in CDEF and CDOT tables are not matching\n");
		return ACDB_ERROR;
	}
	for(i=0 ;i< cdefTbl.nLen;i++)
	{
		uint32_t nPaddedBytes = 0;
		// Now get the datalen and data pointer
		uint32_t mid = cdefTbl.pCntDef[i].nMid;
		uint32_t pid = cdefTbl.pCntDef[i].nPid;

		uint32_t result = Acdb_DM_Ioctl((uint32_t) ACDB_GET_DATA,
			FALSE,
			0,
			(AcdbDataLookupKeyType*) &dataLookupKey,
			(uint32_t*) &mid,
			(uint32_t*) &pid,
			NULL,
			0,
			NULL,
			0,
			NULL,
			0,
			NULL,
			(AcdbDynamicUniqueDataType**) &pDataNode,
			NULL,
			0
			);

		if(result == ACDB_SUCCESS)
		{
			if(pDataNode == NULL)
			{
				ACDB_DEBUG_LOG("Issue with heap, Unable to retrieve data from Heap\n");
				return ACDB_ERROR;
			}
			cData.nLen = pDataNode->ulDataLen;
		}
		else
		{
			cData.nLen = *((uint32_t *)(dataPoolChnk.pData + cdotTbl.pDataOffsets[i]));
		}

		if(cData.nLen%4)
		{
			nPaddedBytes = 4-cData.nLen%4;
		}

		nSizeRequired += (sizeof(DspCalHdrFormatType) + cData.nLen + nPaddedBytes);
	}
	*pSize = nSizeRequired;
	return ACDB_SUCCESS;
}

int32_t GetNoOfTblEntriesOnHeap(uint8_t *pCmd,uint32_t nCmdSize ,uint8_t *pRsp,uint32_t nRspSizse)
{
	int32_t result = ACDB_SUCCESS;
	if (pCmd == NULL || pRsp == NULL || nCmdSize == 0 || nRspSizse == 0)
	{
		ACDB_DEBUG_LOG("[ACDB Command]->Invalid NULL value parameters are provided to get tbl entries\n");
		return ACDB_ERROR;
	}
	result = Acdb_DM_Ioctl(ACDB_GET_NO_OF_TBL_ENTRIES_ON_HEAP, FALSE, 0, NULL,
		NULL,NULL,pCmd,nCmdSize,NULL,0,pRsp,nRspSizse,NULL,NULL, NULL, 0);

	return result;
}

int32_t GetTblEntriesOnHeap(uint8_t *pCmd,uint32_t nCmdSize ,uint8_t *pRsp,uint32_t nRspSizse)
{
	int32_t result = ACDB_SUCCESS;
	if (pCmd == NULL || pRsp == NULL || nCmdSize == 0 || nRspSizse == 0)
	{
		ACDB_DEBUG_LOG("[ACDB Command]->Invalid NULL value parameters are provided to get tbl entries\n");
		return ACDB_ERROR;
	}
	result = Acdb_DM_Ioctl(ACDB_GET_TBL_ENTRIES_ON_HEAP,FALSE, 0 ,NULL,
		NULL,NULL,pCmd,nCmdSize,NULL,0,pRsp,nRspSizse,NULL,NULL,NULL,0);

	return result;
}

int32_t ACDBHeapReset(void)
{
	int32_t result = ACDB_SUCCESS;
	result = Acdb_DM_Ioctl(ACDB_SYS_RESET, FALSE, 0, NULL,
		NULL,NULL,NULL,0,
		NULL,0,NULL,0,NULL,NULL, NULL, 0);

	return result;
}
