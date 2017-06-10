/*===========================================================================
FILE: acdb_file_mgr.c

OVERVIEW: This file contains the implementaion of the helper methods
to access acdb files info.

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

$Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_file_mgr.c#15 $

when who what, where, why
---------- --- -----------------------------------------------------
2014-05-28 mh SW migration from 32-bit to 64-bit architecture
2014-02-14 avi Support new commands for ACDB persistence feature.
2013-06-07 avi Support Volume boost feature (VocProcVolV2 tbl)
2010-11-16 ernanl 1. Added get/set calibration data of Afe table API

========================================================================== */

/* ---------------------------------------------------------------------------
* Include Files
*--------------------------------------------------------------------------- */

#include "acdb_file_mgr.h"
#include "acdb_parser.h"
#include "acdb_private.h"
#include "acdb_init_utility.h"

/* ---------------------------------------------------------------------------
* Preprocessor Definitions and Constants
*--------------------------------------------------------------------------- */
#define ACDB_MAX_ACDB_FILES 20

/* ---------------------------------------------------------------------------
* Type Declarations
*--------------------------------------------------------------------------- */
typedef struct _AcdbDataFileInfo
{
	uint32_t noOfFiles;
	AcdbCmdFileInfo fInfo[ACDB_MAX_ACDB_FILES];
}AcdbDataFileInfo;

typedef struct _AcdbDevProp
{
	uint32_t devid;
	uint32_t pid;
	uint32_t dataoffset;
}AcdbDevProp;

typedef struct _AcdbDevPropLut
{
	uint32_t devPropLen;
	AcdbDevProp *devProp;
}AcdbDevPropLut;

typedef struct _AcdbGlbProp
{
	uint32_t pid;
	uint32_t dataoffset;
}AcdbGlbProp;

typedef struct _AcdbGlbPropLut
{
	uint32_t glbPropLen;
	AcdbGlbProp *glbProp;
}AcdbGlbPropLut;

/* ---------------------------------------------------------------------------
* Global Data Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Static Variable Definitions
*--------------------------------------------------------------------------- */
static AcdbDataFileInfo gDataFileInfo;
static bool_t gAcdbDataGlobalFileLoaded=FALSE;
static bool_t gAcdbDataCodecFileLoaded=FALSE;
/* ---------------------------------------------------------------------------
* Static Function Declarations and Definitions
*--------------------------------------------------------------------------- */

int32_t AcdbDataCmdSetData(AcdbCmdFileInfo *cmdInfo)
{
	int32_t result = ACDB_SUCCESS;
	uint32_t idx;

	if(cmdInfo == NULL)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Received NULL input for AcdbDataCmdSetData\n");
		return ACDB_ERROR;
	}

	if(gDataFileInfo.noOfFiles == ACDB_MAX_ACDB_FILES)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Request exceded the limit of 20 acdb files\n");
		return ACDB_ERROR;
	}

	// check if its a global file or codec file,
	// if they are already loaded then return simply success
	if(IsCodecFileType(cmdInfo->pFileBuf,cmdInfo->nFileSize) == ACDB_PARSE_SUCCESS &&
		gAcdbDataCodecFileLoaded == TRUE)
	{
		AcdbFreeFileData((void *)cmdInfo->pFileBuf);
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Duplicate Codec Acdb file set request, so skipping the set request\n");
		return ACDB_SUCCESS;
	}

	if(IsGlobalFileType(cmdInfo->pFileBuf,cmdInfo->nFileSize) == ACDB_PARSE_SUCCESS &&
		gAcdbDataGlobalFileLoaded == TRUE)
	{
		AcdbFreeFileData((void *)cmdInfo->pFileBuf);
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Duplicate Global Acdb file set request, so skipping the set request\n");
		return ACDB_SUCCESS;
	}

	// check if set acdb data request is a duplicate request.
	// if so take the latest acdb
	for(idx=0;idx<gDataFileInfo.noOfFiles;idx++)
	{
		if( memcmp(&cmdInfo->chFileName[0],&gDataFileInfo.fInfo[idx].chFileName[0],ACDB_FILENAME_MAX_CHARS) == 0)
		{
			AcdbFreeFileData((void *)cmdInfo->pFileBuf);
			ACDB_DEBUG_LOG("ACDBFILE_MGR: Duplicate Acdb file set request, so skipping the set request\n");
			return ACDB_SUCCESS;
		}
	}
	idx = gDataFileInfo.noOfFiles++;
	ACDB_MEM_CPY(&gDataFileInfo.fInfo[idx].chFileName[0],sizeof(gDataFileInfo.fInfo[idx].chFileName),&cmdInfo->chFileName[0],ACDB_FILENAME_MAX_CHARS);
	gDataFileInfo.fInfo[idx].nFileNameLen = cmdInfo->nFileNameLen;
	gDataFileInfo.fInfo[idx].nFileSize = cmdInfo->nFileSize;
	gDataFileInfo.fInfo[idx].pFileBuf = cmdInfo->pFileBuf;

	return result;
}

int32_t AcdbDataCmdReset()
{
	int32_t result = ACDB_SUCCESS;
	uint32_t idx =0;
	for(idx=0;idx< gDataFileInfo.noOfFiles;idx++)
	{
		gDataFileInfo.fInfo[idx].nFileNameLen = 0;
		gDataFileInfo.fInfo[idx].nFileSize = 0;
		memset(&gDataFileInfo.fInfo[idx].chFileName[0],0,ACDB_FILENAME_MAX_CHARS);
		AcdbFreeFileData((void *)gDataFileInfo.fInfo[idx].pFileBuf);
		gDataFileInfo.fInfo[idx].pFileBuf = NULL;
	}
	gDataFileInfo.noOfFiles = 0;
	return result;
}

int32_t AcdbDataGetDataPtr(AcdbCmdFileInfo fileInfo,uint32_t dataoffset,AcdbDataInfo *pDataInfo)
{
	int32_t result = ACDB_SUCCESS;
	uint8_t *pChkBuf = NULL;
	uint32_t nChkBufSize;
	uint32_t nDataLen;
	if(pDataInfo == NULL)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Received NULL input for AcdbDataGetDataPtr\n");
		return ACDB_ERROR;
	}
	result = AcdbFileGetChunkData(fileInfo.pFileBuf,fileInfo.nFileSize,ACDB_CHUNKID_DATAPOOL,&pChkBuf,&nChkBufSize);
	if(result != ACDB_PARSE_SUCCESS)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: DataPool not found\n");
		return ACDB_ERROR;
	}

	if(dataoffset > nChkBufSize)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Out of bounds dataoffset value Received\n");
		return ACDB_ERROR;
	}

	nDataLen = *((uint32_t *)(pChkBuf+dataoffset));
	if((dataoffset + nDataLen +sizeof(nDataLen)) > nChkBufSize)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Valid dataoffset Received, but invalid data len. Trying to access beyond DATAPOOL chunk\n");
		return ACDB_ERROR;
	}

	pDataInfo->nDataLen = nDataLen;
	pDataInfo->pData = pChkBuf + dataoffset + sizeof(uint32_t);
	return result;
}

int32_t AcdbDataGetDevPropData(AcdbDevPropInfo *dpInfo)
{
	int32_t result = ACDB_ERROR;
	AcdbDevPropLut devPropLut;
	uint8_t *pChkBuf = NULL;
	uint32_t nChkBufSize=0;
	uint32_t i = 0,idx=0;
	if(dpInfo == NULL)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Received NULL input for AcdbDataGetDevPropData\n");
		return ACDB_ERROR;
	}
	if(gDataFileInfo.noOfFiles == 0)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: No acdb files loaded to fetch data\n");
		return ACDB_ERROR;
	}
	for(i=0;i<gDataFileInfo.noOfFiles;i++)
	{
		int8_t nDataFound = 0;

		result = AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,gDataFileInfo.fInfo[i].nFileSize,ACDB_CHUNKID_DEVPROPLUT,&pChkBuf,&nChkBufSize);
		if(result != ACDB_PARSE_SUCCESS)
		{
			result = ACDB_ERROR;
			continue;
		}
		devPropLut.devPropLen = *((uint32_t *)pChkBuf);
		devPropLut.devProp = (AcdbDevProp *)(pChkBuf + sizeof(devPropLut.devPropLen));
		for(idx=0;idx<devPropLut.devPropLen;idx++)
		{
			if(dpInfo->devId == devPropLut.devProp[idx].devid &&
				dpInfo->pId == devPropLut.devProp[idx].pid)
			{
				result = AcdbDataGetDataPtr(gDataFileInfo.fInfo[i],devPropLut.devProp[idx].dataoffset,&dpInfo->dataInfo);
				nDataFound = 1;
				break;
			}
		}
		if(nDataFound == 1)
		{
			break;
		}
		else
		{
			result = ACDB_ERROR;
		}
	}

	if(result != ACDB_SUCCESS)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: CmnDevinfo for the devid %08X not found\n",dpInfo->devId);
	}

	return result;
}

int32_t AcdbDataGetGlobalPropData(AcdbGlbalPropInfo *glbInfo)
{
	int32_t result = ACDB_ERROR;
	AcdbGlbPropLut glbPropLut;
	uint8_t *pChkBuf = NULL;
	uint32_t nChkBufSize=0;
	uint32_t i = 0,idx=0;
	if(glbInfo == NULL)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Received NULL input for AcdbDataGetGlobalPropData\n");
		return ACDB_BADPARM;
	}
	if(gDataFileInfo.noOfFiles == 0)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: No acdb files loaded to fetch data\n");
		return ACDB_ERROR;
	}
	for(i=0;i<gDataFileInfo.noOfFiles;i++)
	{
		int8_t nDataFound = 0;

		result = AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,gDataFileInfo.fInfo[i].nFileSize,ACDB_CHUNKID_GLOBALPROPLUT,&pChkBuf,&nChkBufSize);
		if(result != ACDB_PARSE_SUCCESS)
		{
			result = ACDB_ERROR;
			continue;
		}
		glbPropLut.glbPropLen = *((uint32_t *)pChkBuf);
		glbPropLut.glbProp = (AcdbGlbProp *)(pChkBuf + sizeof(glbPropLut.glbPropLen));
		for(idx=0;idx<glbPropLut.glbPropLen;idx++)
		{
			if(glbInfo->pId == glbPropLut.glbProp[idx].pid)
			{
				result = AcdbDataGetDataPtr(gDataFileInfo.fInfo[i],glbPropLut.glbProp[idx].dataoffset,&glbInfo->dataInfo);
				nDataFound = 1;
				break;
			}
		}
		if(nDataFound == 1)
			break;
	}

	if(result != ACDB_SUCCESS)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Property for the pid %08X not found\n",glbInfo->pId);
	}

	return result;
}
int32_t AcdbDataGetDevicesList(AcdbDevices *pDevs)
{
	int32_t result = ACDB_SUCCESS;
	uint8_t *pChkBuf = NULL;
	uint32_t nChkBufSize=0;
	uint32_t i = 0;
	int8_t nAleastOneSuccess = 0;
	if(pDevs == NULL)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Received NULL input for AcdbDataGetDevicesList\n");
		return ACDB_ERROR;
	}
	pDevs->noOfDevs = 0;

	if(gDataFileInfo.noOfFiles == 0)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: No acdb files loaded to fetch devices list\n");
		return ACDB_ERROR;
	}
	for(i=0;i<gDataFileInfo.noOfFiles;i++)
	{
		uint32_t j=0,k=0;
		AcdbDevPropLut devPropLut;
		result = AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,gDataFileInfo.fInfo[i].nFileSize,ACDB_CHUNKID_DEVPROPLUT,&pChkBuf,&nChkBufSize);
		if(result != ACDB_PARSE_SUCCESS)
		{
			continue;
		}
		if(nAleastOneSuccess == 0)
		{
			nAleastOneSuccess = 1;
		}
		devPropLut.devPropLen = *((uint32_t *)pChkBuf);
		devPropLut.devProp = (AcdbDevProp *)(pChkBuf + sizeof(devPropLut.devPropLen));

		if(devPropLut.devPropLen == 0)
		{
			ACDB_DEBUG_LOG("ACDBFILE_MGR:Read the devices count as zero, please check the acdb file\n");
			continue;
		}

		for(j=0;j<devPropLut.devPropLen;j++)
		{
			uint8_t found = 0;
			if(pDevs->noOfDevs >= ACDB_MAX_DEVICES)
			{
				ACDB_DEBUG_LOG("ACDBFILE_MGR:Max devices limit of %08X reached\n",ACDB_MAX_DEVICES);
				return ACDB_SUCCESS;
			}

			for(k=0;k<pDevs->noOfDevs;k++)
			{
				if(pDevs->devList[k] == devPropLut.devProp[j].devid)
				{
					found =1;
					break;
				}
			}
			if(found)
			{
				continue;
			}
			pDevs->devList[pDevs->noOfDevs++] = devPropLut.devProp[j].devid;
		}
	}

	if(nAleastOneSuccess == 0)
	{
		result = ACDB_ERROR;
	}
	else
	{
		result = ACDB_SUCCESS;
	}
	return result;
}

int32_t AcdbDataGetFileTypeInfo(char * fileName, AcdbFileInfo *pFileInfo)
{
	int32_t result = ACDB_SUCCESS;
	uint8_t *pChkBuf = NULL;
	uint32_t nChkBufSize=0;
	uint32_t i = 0;
	int8_t nAleastOneSuccess = 0;
	uint32_t major = 0;
	uint32_t minor = 0;
	uint32_t revision = 0;

	if(pFileInfo == NULL)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: Received NULL input for AcdbDataGetFileTypeInfo\n");
		return ACDB_ERROR;
	}
	pFileInfo->noOfDevs = 0;
	pFileInfo->pDevList = NULL;

	if(gDataFileInfo.noOfFiles == 0)
	{
		ACDB_DEBUG_LOG("ACDBFILE_MGR: No acdb files loaded to fetch devices list\n");
		return ACDB_ERROR;
	}
	for(i=0;i<gDataFileInfo.noOfFiles;i++)
	{
		if(0 == strcmp(gDataFileInfo.fInfo[i].chFileName, fileName))
		{
			// get maj, min, rev, swp info.
			AcdbFileGetSWVersion(gDataFileInfo.fInfo[i].pFileBuf,gDataFileInfo.fInfo[i].nFileSize,&major, &minor, &revision);

			pFileInfo->maj = major;
			pFileInfo->min = minor;
			pFileInfo->rev = revision;

			if(ACDB_PARSE_SUCCESS == IsAVFileType(gDataFileInfo.fInfo[i].pFileBuf, gDataFileInfo.fInfo[i].nFileSize))
			{
				uint32_t j=0,k=0;
				AcdbDevPropLut devPropLut;
				AcdbDevices *pDevList = (AcdbDevices *)ACDB_MALLOC(sizeof(AcdbDevices));

				if(pDevList == NULL)
				{
					ACDB_DEBUG_LOG("ACDBFILE_MGR:Could not allocate memory for pDevList \n");
					return ACDB_BADSTATE;
				}

				pDevList->noOfDevs = 0;
				result = AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,gDataFileInfo.fInfo[i].nFileSize,ACDB_CHUNKID_DEVPROPLUT,&pChkBuf,&nChkBufSize);
				if(result != ACDB_PARSE_SUCCESS)
				{
					ACDB_MEM_FREE(pDevList);
					continue;
				}
				if(nAleastOneSuccess == 0)
				{
					nAleastOneSuccess = 1;
				}
				devPropLut.devPropLen = *((uint32_t *)pChkBuf);
				devPropLut.devProp = (AcdbDevProp *)(pChkBuf + sizeof(devPropLut.devPropLen));

				if(devPropLut.devPropLen == 0)
				{
					ACDB_DEBUG_LOG("ACDBFILE_MGR:Read the devices count as zero, please check the acdb file\n");
					ACDB_MEM_FREE(pDevList);
					continue;
				}

				for(j=0;j<devPropLut.devPropLen;j++)
				{
					uint8_t found = 0;
					if(pFileInfo->noOfDevs >= ACDB_MAX_DEVICES)
					{
						ACDB_DEBUG_LOG("ACDBFILE_MGR:Max devices limit of %08X reached\n",ACDB_MAX_DEVICES);
						ACDB_MEM_FREE(pDevList);
						return ACDB_SUCCESS;
					}

					for(k=0;k<pFileInfo->noOfDevs;k++)
					{
						if(pDevList->devList[k] == devPropLut.devProp[j].devid)
						{
							found =1;
							break;
						}
					}
					if(found)
					{
						continue;
					}
					pDevList->devList[pFileInfo->noOfDevs++] = devPropLut.devProp[j].devid;
					pDevList->noOfDevs++;
				}
				pFileInfo->pDevList = (uint32_t *)ACDB_MALLOC(sizeof(uint32_t) * pFileInfo->noOfDevs);

				if(pFileInfo->pDevList == NULL)
				{
					ACDB_DEBUG_LOG("ACDBFILE_MGR:Could not allocate memory for pDevList \n");
					ACDB_MEM_FREE(pDevList);
					return ACDB_BADSTATE;
				}
				ACDB_MEM_CPY(pFileInfo->pDevList, sizeof(uint32_t) * pFileInfo->noOfDevs, &pDevList->devList[0], sizeof(uint32_t) * pFileInfo->noOfDevs);
				ACDB_MEM_FREE(pDevList);
				pDevList = NULL;

				if(nAleastOneSuccess == 0)
				{
					result = ACDB_ERROR;
				}
				else
				{
					result = ACDB_SUCCESS;
				}

				pFileInfo->fileType = ACDB_AV_TYPE;
			}
			else if(ACDB_PARSE_SUCCESS == IsCodecFileType(gDataFileInfo.fInfo[i].pFileBuf, gDataFileInfo.fInfo[i].nFileSize))
			{
				pFileInfo->fileType = ACDB_CODEC_TYPE;
			}
			else if(ACDB_PARSE_SUCCESS == IsGlobalFileType(gDataFileInfo.fInfo[i].pFileBuf, gDataFileInfo.fInfo[i].nFileSize))
			{
				pFileInfo->fileType = ACDB_GLOBAL_TYPE;
			}
			else
			{
				pFileInfo->fileType = ACDB_UNKNOWN_TYPE;
				ACDB_DEBUG_LOG("ACDBFILE_MGR:Unknown file type!\n");
				result = ACDB_ERROR;
			}
			break;
		}
	}

	return result;
}

int32_t AcdbDataGetGlobalTblsInfo(AcdbTableCmd *tblCmd,AcdbTableInfo *tblInfo)
{
	int32_t tblFound = 1;
	uint64_t lutId=0;
	uint64_t cdefId=0;
	uint64_t cdotId=0;
	uint64_t dataPoolId=ACDB_CHUNKID_DATAPOOL;

	// check if the tblid req is from global acdb file
	switch(tblCmd->tblId)
	{
	case AUD_STREAM_TBL:
		lutId=ACDB_CHUNKID_AUDSTREAMCLUT;
		cdefId=ACDB_CHUNKID_AUDSTREAMCDFT;
		cdotId=ACDB_CHUNKID_AUDSTREAMCDOT;
		break;
	case VOC_STREAM_TBL:
		lutId=ACDB_CHUNKID_VOCSTREAMCLUT;
		cdefId=ACDB_CHUNKID_VOCSTREAMCDFT;
		cdotId=ACDB_CHUNKID_VOCSTREAMCDOT;
		break;
	case VOC_STREAM2_TBL:
		lutId=ACDB_CHUNKID_VOCSTREAM2CLUT;
		cdefId=ACDB_CHUNKID_VOCSTREAM2CDFT;
		cdotId=ACDB_CHUNKID_VOCSTREAM2CDOT;
		break;
	case GLOBAL_DATA_TBL:
		lutId=ACDB_CHUNKID_GLOBALLUT;
		break;
	case METAINFO_LOOKUP_TBL:
		lutId = ACDB_CHUNKID_MINFOLUT;
		break;
	default:
		tblFound = 0;
	}

	if(tblFound == 1)
	{
		uint32_t i=0;
		// the table request is from global table
		for(i=0;i<gDataFileInfo.noOfFiles;i++)
		{
			if(IsGlobalFileType(gDataFileInfo.fInfo[i].pFileBuf,
				gDataFileInfo.fInfo[i].nFileSize) != ACDB_PARSE_SUCCESS)
			{
				continue;
			}
			else
			{
				if(lutId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,lutId,&tblInfo->tblLutChnk.pData,
						&tblInfo->tblLutChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid Global file. It doesnt contain LUT for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdefId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdefId,&tblInfo->tblCdftChnk.pData,
						&tblInfo->tblCdftChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid Global file. It doesnt contain CDEF tbale for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdotId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdotId,&tblInfo->tblCdotChnk.pData,
						&tblInfo->tblCdotChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid Global file. It doesnt contain CDOT table for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(dataPoolId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,dataPoolId,&tblInfo->dataPoolChnk.pData,
						&tblInfo->dataPoolChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid Global file. It doesn't contain datapool\n");
						return ACDB_ERROR;
					}
				}
				return ACDB_SUCCESS;
			}
		}
	}
	return ACDB_ERROR;
}
int32_t AcdbDataGetGlobalTblsInfoCVD(AcdbTableCmd *tblCmd,AcdbTableInfoCVD *tblInfo)
{
	int32_t tblFound = 1;
	uint64_t lutId=0;
	uint64_t cdefId=0;
	uint64_t cdotId=0;
	uint64_t dataPoolId=ACDB_CHUNKID_DATAPOOL;

	// check if the tblid req is from global acdb file
	switch(tblCmd->tblId)
	{
	case AUD_STREAM_TBL:
		lutId=ACDB_CHUNKID_AUDSTREAMCLUT;
		cdefId=ACDB_CHUNKID_AUDSTREAMCDFT;
		cdotId=ACDB_CHUNKID_AUDSTREAMCDOT;
		break;
	case VOC_STREAM_TBL:
		lutId=ACDB_CHUNKID_VOCSTREAMCLUT;
		cdefId=ACDB_CHUNKID_VOCSTREAMCDFT;
		cdotId=ACDB_CHUNKID_VOCSTREAMCDOT;
		break;
	case VOC_STREAM2_TBL:
		lutId=ACDB_CHUNKID_VOCSTREAM2CLUT;
		cdefId=ACDB_CHUNKID_VOCSTREAM2CDFT;
		cdotId=ACDB_CHUNKID_VOCSTREAM2CDOT;
		break;
	case GLOBAL_DATA_TBL:
		lutId=ACDB_CHUNKID_GLOBALLUT;
		break;
	case METAINFO_LOOKUP_TBL:
		lutId = ACDB_CHUNKID_MINFOLUT;
		break;
	default:
		tblFound = 0;
	}

	if(tblFound == 1)
	{
		uint32_t i=0;
		// the table request is from global table
		for(i=0;i<gDataFileInfo.noOfFiles;i++)
		{
			if(IsGlobalFileType(gDataFileInfo.fInfo[i].pFileBuf,
				gDataFileInfo.fInfo[i].nFileSize) != ACDB_PARSE_SUCCESS)
			{
				continue;
			}
			else
			{
				if(lutId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,lutId,&tblInfo->tblLutChnk.pData,
						&tblInfo->tblLutChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid Global file. It doesnt contain LUT for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdefId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdefId,&tblInfo->tblCdftChnk.pData,
						&tblInfo->tblCdftChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid Global file. It doesnt contain CDEF tbale for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdotId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdotId,&tblInfo->tblCdotChnk.pData,
						&tblInfo->tblCdotChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid Global file. It doesnt contain CDOT table for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(dataPoolId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,dataPoolId,&tblInfo->dataPoolChnk.pData,
						&tblInfo->dataPoolChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid Global file. It doesn't contain datapool\n");
						return ACDB_ERROR;
					}
				}
				return ACDB_SUCCESS;
			}
		}
	}
	return ACDB_ERROR;
}

int32_t AcdbDataGetCodecTblsInfo(AcdbTableCmd *tblCmd,AcdbTableInfo *tblInfo)
{
	int32_t tblFound = 1;
	uint64_t lutId=0;
	uint64_t cdefId=0;
	uint64_t cdotId=0;
	uint64_t dataPoolId=ACDB_CHUNKID_DATAPOOL;
	// check if the tblid req is from codec acdb file
	switch(tblCmd->tblId)
	{
	case ADIE_CODEC_TBL:
		lutId=ACDB_CHUNKID_ADIELUT;
		break;
	default:
		tblFound = 0;
	}

	if(tblFound == 1)
	{
		uint32_t i=0;
		// the table request is from global table
		for(i=0;i<gDataFileInfo.noOfFiles;i++)
		{
			if(IsCodecFileType(gDataFileInfo.fInfo[i].pFileBuf,
				gDataFileInfo.fInfo[i].nFileSize) != ACDB_PARSE_SUCCESS)
			{
				continue;
			}
			else
			{
				if(lutId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,lutId,&tblInfo->tblLutChnk.pData,
						&tblInfo->tblLutChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid codec file. It doesnt contain LUT for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdefId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdefId,&tblInfo->tblCdftChnk.pData,
						&tblInfo->tblCdftChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid codec file. It doesnt contain CDEF tbale for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdotId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdotId,&tblInfo->tblCdotChnk.pData,
						&tblInfo->tblCdotChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid codec file. It doesnt contain CDOT table for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(dataPoolId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,dataPoolId,&tblInfo->dataPoolChnk.pData,
						&tblInfo->dataPoolChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid codec file. It doesn't contain datapool\n");
						return ACDB_ERROR;
					}
				}
				return ACDB_SUCCESS;
			}
		}
	}
	return ACDB_ERROR;
}
int32_t AcdbDataGetCodecTblsInfoCVD(AcdbTableCmd *tblCmd,AcdbTableInfoCVD *tblInfo)
{
	int32_t tblFound = 1;
	uint64_t lutId=0;
	uint64_t cdefId=0;
	uint64_t cdotId=0;
	uint64_t dataPoolId=ACDB_CHUNKID_DATAPOOL;
	// check if the tblid req is from codec acdb file
	switch(tblCmd->tblId)
	{
	case ADIE_CODEC_TBL:
		lutId=ACDB_CHUNKID_ADIELUT;
		break;
	default:
		tblFound = 0;
	}

	if(tblFound == 1)
	{
		uint32_t i=0;
		// the table request is from global table
		for(i=0;i<gDataFileInfo.noOfFiles;i++)
		{
			if(IsCodecFileType(gDataFileInfo.fInfo[i].pFileBuf,
				gDataFileInfo.fInfo[i].nFileSize) != ACDB_PARSE_SUCCESS)
			{
				continue;
			}
			else
			{
				if(lutId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,lutId,&tblInfo->tblLutChnk.pData,
						&tblInfo->tblLutChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid codec file. It doesnt contain LUT for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdefId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdefId,&tblInfo->tblCdftChnk.pData,
						&tblInfo->tblCdftChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid codec file. It doesnt contain CDEF tbale for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdotId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdotId,&tblInfo->tblCdotChnk.pData,
						&tblInfo->tblCdotChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid codec file. It doesnt contain CDOT table for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(dataPoolId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,dataPoolId,&tblInfo->dataPoolChnk.pData,
						&tblInfo->dataPoolChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid codec file. It doesn't contain datapool\n");
						return ACDB_ERROR;
					}
				}
				return ACDB_SUCCESS;
			}
		}
	}
	return ACDB_ERROR;
}
int32_t AcdbDataGetAVTblsInfoCVD(AcdbTableCmd *tblCmd,AcdbTableInfoCVD *tblInfo)
{
	int32_t tblFound = 1;
	uint64_t lutId=0,lutCVDId=0, lutOffsetId=0;
	uint64_t cdefId=0;
	uint64_t cdotId=0;
	uint64_t dataPoolId=ACDB_CHUNKID_DATAPOOL;
	switch(tblCmd->tblId)
	{
	case AUDPROC_GAIN_INDP_TBL:
		lutId=ACDB_CHUNKID_AUDPROCLUT;
		cdefId=ACDB_CHUNKID_AUDPROCDFT;
		cdotId=ACDB_CHUNKID_AUDPROCDOT;
		break;
	case AUDPROC_COPP_GAIN_DEP_TBL:
		lutId=ACDB_CHUNKID_AUDPROGAINDEPCLUT;
		cdefId=ACDB_CHUNKID_AUDPROGAINDEPCDFT;
		cdotId=ACDB_CHUNKID_AUDPROGAINDEPCDOT;
		break;
	case AUDPROC_AUD_VOL_TBL:
		lutId=ACDB_CHUNKID_AUDVOLCLUT;
		cdefId=ACDB_CHUNKID_AUDVOLCDFT;
		cdotId=ACDB_CHUNKID_AUDVOLCDOT;
		break;
	case AUD_STREAM_TBL:
		lutId=ACDB_CHUNKID_AUDSTREAMCLUT;
		cdefId=ACDB_CHUNKID_AUDSTREAMCDFT;
		cdotId=ACDB_CHUNKID_AUDSTREAMCDOT;
		break;
	case VOCPROC_GAIN_INDP_TBL:
		lutId=ACDB_CHUNKID_VOCPROCLUT;
		cdefId=ACDB_CHUNKID_VOCPROCDFT;
		cdotId=ACDB_CHUNKID_VOCPROCDOT;
		break;
	case VOCPROC_DYNAMIC_TBL:
		lutId=ACDB_CHUNKID_VOCPROCDYNLUT;
		cdefId=ACDB_CHUNKID_VOCPROCDYNDFT;
		cdotId=ACDB_CHUNKID_VOCPROCDYNDOT;
		lutCVDId=ACDB_CHUNKID_VOCPROCDYNLUTCVD;
		lutOffsetId=ACDB_CHUNKID_VOCPROCDYNLUTOFFSET;
		break;
	case VOCPROC_STATIC_TBL:
		lutId=ACDB_CHUNKID_VOCPROCSTATLUT;
		cdefId=ACDB_CHUNKID_VOCPROCSTATDFT;
		cdotId=ACDB_CHUNKID_VOCPROCSTATDOT;
		lutCVDId=ACDB_CHUNKID_VOCPROCSTATLUTCVD;
		lutOffsetId=ACDB_CHUNKID_VOCPROCSTATLUTOFFSET;
		break;

	case VOCPROC_COPP_GAIN_DEP_TBL:
		lutId=ACDB_CHUNKID_VOCVOLCLUT;
		cdefId=ACDB_CHUNKID_VOCVOLCDFT;
		cdotId=ACDB_CHUNKID_VOCVOLCDOT;
		break;
	case VOC_STREAM_TBL:
		lutId=ACDB_CHUNKID_VOCSTREAMCLUT;
		cdefId=ACDB_CHUNKID_VOCSTREAMCDFT;
		cdotId=ACDB_CHUNKID_VOCSTREAMCDOT;
		break;
	case VOC_STREAM2_TBL:
		lutId=ACDB_CHUNKID_VOCSTREAM2CLUT;
		cdefId=ACDB_CHUNKID_VOCSTREAM2CDFT;
		cdotId=ACDB_CHUNKID_VOCSTREAM2CDOT;
		break;
	case AFE_TBL:
		lutId=ACDB_CHUNKID_AFECLUT;
		cdefId=ACDB_CHUNKID_AFECDFT;
		cdotId=ACDB_CHUNKID_AFECDOT;
		break;
	case AFE_CMN_TBL:
		lutId=ACDB_CHUNKID_AFECMNCLUT;
		cdefId=ACDB_CHUNKID_AFECMNCDFT;
		cdotId=ACDB_CHUNKID_AFECMNCDOT;
		break;
	case ADIE_ANC_TBL:
		lutId=ACDB_CHUNKID_ADIEANCLUT;
		cdefId=0;
		cdotId=0;
		break;
	case VOCPROC_DEV_CFG_TBL:
		lutId=ACDB_CHUNKID_DEVPAIRCFGCLUT;
		cdefId=ACDB_CHUNKID_DEVPAIRCFGCDFT;
		cdotId=ACDB_CHUNKID_DEVPAIRCFGCDOT;
		break;
	case LSM_TBL:
		lutId=ACDB_CHUNKID_LSMLUT;
		cdefId=ACDB_CHUNKID_LSMCDFT;
		cdotId=ACDB_CHUNKID_LSMCDOT;
		break;
	case CDC_FEATURES_TBL:
		lutId=ACDB_CHUNKID_CDCFEATUREDATALUT;
		cdefId=0;
		cdotId=0;
		break;
	case ADIE_SIDETONE_TBL:
		lutId=ACDB_CHUNKID_ADSTLUT;
		cdefId=ACDB_CHUNKID_ADSTCDFT;
		cdotId=ACDB_CHUNKID_ADSTCDOT;
		break;
	case AANC_CFG_TBL:
		lutId=ACDB_CHUNKID_AANCLUT;
		cdefId=ACDB_CHUNKID_AANCCDFT;
		cdotId=ACDB_CHUNKID_AANCCDOT;
		break;
	case VOCPROC_COPP_GAIN_DEP_V2_TBL:
		lutId=ACDB_CHUNKID_VOCVOL2CLUT;
		cdefId=ACDB_CHUNKID_VOCVOL2CDFT;
		cdotId=ACDB_CHUNKID_VOCVOL2CDOT;
		break;
	case VOICE_VP3_TBL:
		lutId=ACDB_CHUNKID_VOICEVP3CLUT;
		cdefId=ACDB_CHUNKID_VOICEVP3CDFT;
		cdotId=ACDB_CHUNKID_VOICEVP3CDOT;
		break;
	case AUDIO_REC_VP3_TBL:
		lutId=ACDB_CHUNKID_AUDIORECVP3CLUT;
		cdefId=ACDB_CHUNKID_AUDIORECVP3CDFT;
		cdotId=ACDB_CHUNKID_AUDIORECVP3CDOT;
		break;
	case AUDIO_REC_EC_VP3_TBL:
		lutId=ACDB_CHUNKID_AUDIORECECVP3CLUT;
		cdefId=ACDB_CHUNKID_AUDIORECECVP3CDFT;
		cdotId=ACDB_CHUNKID_AUDIORECECVP3CDOT;
		break;
	default:
		tblFound = 0;
	}

	if(tblFound == 1)
	{
		uint32_t i=0;
		// the table request is from global table
		for(i=0;i<gDataFileInfo.noOfFiles;i++)
		{
			if(IsAVFileType(gDataFileInfo.fInfo[i].pFileBuf,
				gDataFileInfo.fInfo[i].nFileSize) != ACDB_PARSE_SUCCESS)
			{
				continue;
			}
			else
			{
				//check if the device id exists in the av acdb file
				int32_t devFound = 0;
				uint32_t idx = 0;
				uint8_t *pChkBuf = NULL;
				uint32_t nChkBufSize=0;
				int32_t result;
				AcdbDevPropLut devPropLut;
				//Fetch devices list
				//devs.noOfDevs = 0;
				//result = AcdbDataGetDevicesList(&devs);
				//if(result != ACDB_SUCCESS)
				// continue;

				result = AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,gDataFileInfo.fInfo[i].nFileSize,ACDB_CHUNKID_DEVPROPLUT,&pChkBuf,&nChkBufSize);
				if(result != ACDB_PARSE_SUCCESS)
				{
					continue;
				}
				devPropLut.devPropLen = *((uint32_t *)pChkBuf);
				devPropLut.devProp = (AcdbDevProp *)(pChkBuf + sizeof(devPropLut.devPropLen));

				if(devPropLut.devPropLen == 0)
				{
					ACDB_DEBUG_LOG("ACDBFILE_MGR:Read the devices count as zero, please check the acdb file\n");
					continue;
				}

				// check if the devices exists
				for(idx = 0;idx<devPropLut.devPropLen;idx++)
				{
					if(devPropLut.devProp[idx].devid == tblCmd->devId)
					{
						devFound = 1;
					}
				}

				if(devFound == 0)
					continue;

				if(lutId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,lutId,&tblInfo->tblLutChnk.pData,
						&tblInfo->tblLutChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesnt contain LUT for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}

				if(lutCVDId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,lutCVDId,&tblInfo->tblLutCVDChnk.pData,
						&tblInfo->tblLutCVDChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesnt contain LUT for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdefId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdefId,&tblInfo->tblCdftChnk.pData,
						&tblInfo->tblCdftChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesnt contain CDEF tbale for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdotId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdotId,&tblInfo->tblCdotChnk.pData,
						&tblInfo->tblCdotChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesnt contain CDOT table for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(lutOffsetId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,lutOffsetId,&tblInfo->tblLutOffsetChnk.pData,
						&tblInfo->tblLutOffsetChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesnt contain LUT for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}

				if(dataPoolId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,dataPoolId,&tblInfo->dataPoolChnk.pData,
						&tblInfo->dataPoolChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesn't contain datapool\n");
						return ACDB_ERROR;
					}
				}
				return ACDB_SUCCESS;
			}
		}
	}
	return ACDB_ERROR;
}
int32_t AcdbDataGetAVTblsInfo(AcdbTableCmd *tblCmd,AcdbTableInfo *tblInfo)
{
	int32_t tblFound = 1;
	uint64_t lutId=0;
	uint64_t cdefId=0;
	uint64_t cdotId=0;
	uint64_t dataPoolId=ACDB_CHUNKID_DATAPOOL;
	switch(tblCmd->tblId)
	{
	case AUDPROC_GAIN_INDP_TBL:
		lutId=ACDB_CHUNKID_AUDPROCLUT;
		cdefId=ACDB_CHUNKID_AUDPROCDFT;
		cdotId=ACDB_CHUNKID_AUDPROCDOT;
		break;
	case AUDPROC_COPP_GAIN_DEP_TBL:
		lutId=ACDB_CHUNKID_AUDPROGAINDEPCLUT;
		cdefId=ACDB_CHUNKID_AUDPROGAINDEPCDFT;
		cdotId=ACDB_CHUNKID_AUDPROGAINDEPCDOT;
		break;
	case AUDPROC_AUD_VOL_TBL:
		lutId=ACDB_CHUNKID_AUDVOLCLUT;
		cdefId=ACDB_CHUNKID_AUDVOLCDFT;
		cdotId=ACDB_CHUNKID_AUDVOLCDOT;
		break;
	case AUD_STREAM_TBL:
		lutId=ACDB_CHUNKID_AUDSTREAMCLUT;
		cdefId=ACDB_CHUNKID_AUDSTREAMCDFT;
		cdotId=ACDB_CHUNKID_AUDSTREAMCDOT;
		break;
	case VOCPROC_GAIN_INDP_TBL:
		lutId=ACDB_CHUNKID_VOCPROCLUT;
		cdefId=ACDB_CHUNKID_VOCPROCDFT;
		cdotId=ACDB_CHUNKID_VOCPROCDOT;
		break;
	case VOCPROC_DYNAMIC_TBL:
		lutId=ACDB_CHUNKID_VOCPROCDYNLUT;
		cdefId=ACDB_CHUNKID_VOCPROCDYNDFT;
		cdotId=ACDB_CHUNKID_VOCPROCDYNDOT;
		break;
	case VOCPROC_STATIC_TBL:
		lutId=ACDB_CHUNKID_VOCPROCSTATLUT;
		cdefId=ACDB_CHUNKID_VOCPROCSTATDFT;
		cdotId=ACDB_CHUNKID_VOCPROCSTATDOT;
		break;

	case VOCPROC_COPP_GAIN_DEP_TBL:
		lutId=ACDB_CHUNKID_VOCVOLCLUT;
		cdefId=ACDB_CHUNKID_VOCVOLCDFT;
		cdotId=ACDB_CHUNKID_VOCVOLCDOT;
		break;
	case VOC_STREAM_TBL:
		lutId=ACDB_CHUNKID_VOCSTREAMCLUT;
		cdefId=ACDB_CHUNKID_VOCSTREAMCDFT;
		cdotId=ACDB_CHUNKID_VOCSTREAMCDOT;
		break;
	case VOC_STREAM2_TBL:
		lutId=ACDB_CHUNKID_VOCSTREAM2CLUT;
		cdefId=ACDB_CHUNKID_VOCSTREAM2CDFT;
		cdotId=ACDB_CHUNKID_VOCSTREAM2CDOT;
		break;
	case AFE_TBL:
		lutId=ACDB_CHUNKID_AFECLUT;
		cdefId=ACDB_CHUNKID_AFECDFT;
		cdotId=ACDB_CHUNKID_AFECDOT;
		break;
	case AFE_CMN_TBL:
		lutId=ACDB_CHUNKID_AFECMNCLUT;
		cdefId=ACDB_CHUNKID_AFECMNCDFT;
		cdotId=ACDB_CHUNKID_AFECMNCDOT;
		break;
	case ADIE_ANC_TBL:
		lutId=ACDB_CHUNKID_ADIEANCLUT;
		cdefId=0;
		cdotId=0;
		break;
	case VOCPROC_DEV_CFG_TBL:
		lutId=ACDB_CHUNKID_DEVPAIRCFGCLUT;
		cdefId=ACDB_CHUNKID_DEVPAIRCFGCDFT;
		cdotId=ACDB_CHUNKID_DEVPAIRCFGCDOT;
		break;
	case LSM_TBL:
		lutId=ACDB_CHUNKID_LSMLUT;
		cdefId=ACDB_CHUNKID_LSMCDFT;
		cdotId=ACDB_CHUNKID_LSMCDOT;
		break;
	case CDC_FEATURES_TBL:
		lutId=ACDB_CHUNKID_CDCFEATUREDATALUT;
		cdefId=0;
		cdotId=0;
		break;
	case ADIE_SIDETONE_TBL:
		lutId=ACDB_CHUNKID_ADSTLUT;
		cdefId=ACDB_CHUNKID_ADSTCDFT;
		cdotId=ACDB_CHUNKID_ADSTCDOT;
		break;
	case AANC_CFG_TBL:
		lutId=ACDB_CHUNKID_AANCLUT;
		cdefId=ACDB_CHUNKID_AANCCDFT;
		cdotId=ACDB_CHUNKID_AANCCDOT;
		break;
	case VOCPROC_COPP_GAIN_DEP_V2_TBL:
		lutId=ACDB_CHUNKID_VOCVOL2CLUT;
		cdefId=ACDB_CHUNKID_VOCVOL2CDFT;
		cdotId=ACDB_CHUNKID_VOCVOL2CDOT;
		break;
	case VOICE_VP3_TBL:
		lutId=ACDB_CHUNKID_VOICEVP3CLUT;
		cdefId=ACDB_CHUNKID_VOICEVP3CDFT;
		cdotId=ACDB_CHUNKID_VOICEVP3CDOT;
		break;
	case AUDIO_REC_VP3_TBL:
		lutId=ACDB_CHUNKID_AUDIORECVP3CLUT;
		cdefId=ACDB_CHUNKID_AUDIORECVP3CDFT;
		cdotId=ACDB_CHUNKID_AUDIORECVP3CDOT;
		break;
	case AUDIO_REC_EC_VP3_TBL:
		lutId=ACDB_CHUNKID_AUDIORECECVP3CLUT;
		cdefId=ACDB_CHUNKID_AUDIORECECVP3CDFT;
		cdotId=ACDB_CHUNKID_AUDIORECECVP3CDOT;
		break;
	default:
		tblFound = 0;
	}

	if(tblFound == 1)
	{
		uint32_t i=0;
		// the table request is from global table
		for(i=0;i<gDataFileInfo.noOfFiles;i++)
		{
			if(IsAVFileType(gDataFileInfo.fInfo[i].pFileBuf,
				gDataFileInfo.fInfo[i].nFileSize) != ACDB_PARSE_SUCCESS)
			{
				continue;
			}
			else
			{
				//check if the device id exists in the av acdb file
				int32_t devFound = 0;
				uint32_t idx = 0;
				uint8_t *pChkBuf = NULL;
				uint32_t nChkBufSize=0;
				int32_t result;
				AcdbDevPropLut devPropLut;
				//Fetch devices list
				//devs.noOfDevs = 0;
				//result = AcdbDataGetDevicesList(&devs);
				//if(result != ACDB_SUCCESS)
				// continue;

				result = AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,gDataFileInfo.fInfo[i].nFileSize,ACDB_CHUNKID_DEVPROPLUT,&pChkBuf,&nChkBufSize);
				if(result != ACDB_PARSE_SUCCESS)
				{
					continue;
				}
				devPropLut.devPropLen = *((uint32_t *)pChkBuf);
				devPropLut.devProp = (AcdbDevProp *)(pChkBuf + sizeof(devPropLut.devPropLen));

				if(devPropLut.devPropLen == 0)
				{
					ACDB_DEBUG_LOG("ACDBFILE_MGR:Read the devices count as zero, please check the acdb file\n");
					continue;
				}

				// check if the devices exists
				for(idx = 0;idx<devPropLut.devPropLen;idx++)
				{
					if(devPropLut.devProp[idx].devid == tblCmd->devId)
					{
						devFound = 1;
					}
				}

				if(devFound == 0)
					continue;

				if(lutId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,lutId,&tblInfo->tblLutChnk.pData,
						&tblInfo->tblLutChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesnt contain LUT for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdefId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdefId,&tblInfo->tblCdftChnk.pData,
						&tblInfo->tblCdftChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesnt contain CDEF tbale for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(cdotId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,cdotId,&tblInfo->tblCdotChnk.pData,
						&tblInfo->tblCdotChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesnt contain CDOT table for tblid %08X \n",tblCmd->tblId);
						return ACDB_ERROR;
					}
				}
				if(dataPoolId)
				{
					if(ACDB_PARSE_SUCCESS != AcdbFileGetChunkData(gDataFileInfo.fInfo[i].pFileBuf,
						gDataFileInfo.fInfo[i].nFileSize,dataPoolId,&tblInfo->dataPoolChnk.pData,
						&tblInfo->dataPoolChnk.nDataLen))
					{
						ACDB_DEBUG_LOG("Invalid AV file. It doesn't contain datapool\n");
						return ACDB_ERROR;
					}
				}
				return ACDB_SUCCESS;
			}
		}
	}
	return ACDB_ERROR;
}

int32_t AcdbDataGetTableInfoCVD(AcdbTableCmd *tblCmd,AcdbTableInfoCVD *tblInfo)
{
	int32_t result = ACDB_SUCCESS;

	if(ACDB_SUCCESS != AcdbDataGetGlobalTblsInfoCVD(tblCmd,tblInfo) &&
		ACDB_SUCCESS != AcdbDataGetCodecTblsInfoCVD(tblCmd,tblInfo) &&
		ACDB_SUCCESS != AcdbDataGetAVTblsInfoCVD(tblCmd,tblInfo))
	{
		return ACDB_DATA_INTERFACE_NOT_FOUND;
	}

	return result;
}
int32_t AcdbDataGetTableInfo(AcdbTableCmd *tblCmd,AcdbTableInfo *tblInfo)
{
	int32_t result = ACDB_SUCCESS;

	if(ACDB_SUCCESS != AcdbDataGetGlobalTblsInfo(tblCmd,tblInfo) &&
		ACDB_SUCCESS != AcdbDataGetCodecTblsInfo(tblCmd,tblInfo) &&
		ACDB_SUCCESS != AcdbDataGetAVTblsInfo(tblCmd,tblInfo))
	{
		return ACDB_ERROR;
	}

	return result;
}

int32_t AcdbDataGetFileName(uint32_t *pFileIndex,AcdbCmdFileNameInfo *pFileNameInfo)
{
	int32_t result = ACDB_SUCCESS;

	if( *pFileIndex < gDataFileInfo.noOfFiles)
	{
		ACDB_MEM_CPY(pFileNameInfo->chFileName, sizeof(pFileNameInfo->chFileName),&gDataFileInfo.fInfo[*pFileIndex].chFileName[0],sizeof(gDataFileInfo.fInfo[*pFileIndex].chFileName));
		pFileNameInfo->nFileNameLen = gDataFileInfo.fInfo[*pFileIndex].nFileNameLen;
	}
	else
	{
		ACDB_DEBUG_LOG("ACDB_FILEMGR [AcdbDataGetFileName]: Received invalid input/output params.\n");
		return ACDB_ERROR;
	}

	return result;
}

int32_t AcdbDataGetFileData(AcdbFileMgrGetFileDataReq *pInput,AcdbFileMgrResp *pOutput)
{
	int32_t result = ACDB_ERROR;
	uint32_t i=0;

	if(pInput == NULL || pOutput == NULL)
	{
		ACDB_DEBUG_LOG("ACDB_FILEMGR: Received invalid input/output params\n");
		return ACDB_ERROR;
	}

	for(i=0;i<gDataFileInfo.noOfFiles;i++)
	{
		uint32_t nDataLenToCopy = 0;
		if(pInput->nfileNameLen != gDataFileInfo.fInfo[i].nFileNameLen)
			continue;
		if(memcmp(pInput->pFileName,&gDataFileInfo.fInfo[i].chFileName[0],pInput->nfileNameLen)!=0)
			continue;
		if(pOutput->nresp_buff_len < pInput->nfile_data_len)
		{
			ACDB_DEBUG_LOG("ACDB_FILEMGR: Insufficient memory buffer provided to copy the requested length of file data\n");
			return ACDB_ERROR;
		}
		if(gDataFileInfo.fInfo[i].nFileSize < pInput->nfile_offset)
		{
			ACDB_DEBUG_LOG("ACDB_FILEMGR: Invalid offset provided to copy the to copy the data from the file\n");
			return ACDB_ERROR;
		}
		if( (gDataFileInfo.fInfo[i].nFileSize - pInput->nfile_offset) < pInput->nfile_data_len )
		{
			nDataLenToCopy = gDataFileInfo.fInfo[i].nFileSize - pInput->nfile_offset;
		}
		else
		{
			nDataLenToCopy = pInput->nfile_data_len;
		}

		ACDB_MEM_CPY(pOutput->pRespBuff,pOutput->nresp_buff_len,gDataFileInfo.fInfo[i].pFileBuf+pInput->nfile_offset,nDataLenToCopy);
		pOutput->nresp_buff_filled = nDataLenToCopy;
		return ACDB_SUCCESS;
	}
	return result;
}

int32_t acdbdata_ioctl (uint32_t nCommandId,
	uint8_t *pInput,
	uint32_t nInputSize,
	uint8_t *pOutput,
	uint32_t nOutputSize)
{
	int32_t result = ACDB_SUCCESS;
	switch (nCommandId)
	{
	case ACDBDATACMD_SET_ACDB_DATA:
		{
			if(pInput == NULL || nInputSize != sizeof(AcdbCmdFileInfo))
			{
				ACDB_DEBUG_LOG("ACDBFILE_MGR: Received invalid input/output params\n");
				return ACDB_BADPARM;
			}
			result = AcdbDataCmdSetData((AcdbCmdFileInfo *)pInput);
		}
		break;
	case ACDBDATACMD_RESET:
		{
			result = AcdbDataCmdReset();
		}
		break;
	case ACDBDATACMD_GET_DEVICE_PROP:
		{
			if(pInput == NULL || nInputSize != sizeof(AcdbDevPropInfo))
			{
				ACDB_DEBUG_LOG("ACDBFILE_MGR: Received invalid input/output params\n");
				return ACDB_BADPARM;
			}
			else
			{
				AcdbDevPropInfo *pDInfo = (AcdbDevPropInfo *)pInput;
				result = AcdbDataGetDevPropData(pDInfo);
			}
		}
		break;
	case ACDBDATACMD_GET_GLOBAL_PROP:
		{
			if(pInput == NULL || nInputSize != sizeof(AcdbGlbalPropInfo))
			{
				ACDB_DEBUG_LOG("ACDBFILE_MGR: Received invalid input/output params\n");
				return ACDB_BADPARM;
			}
			else
			{
				AcdbGlbalPropInfo *pGInfo = (AcdbGlbalPropInfo *)pInput;
				result = AcdbDataGetGlobalPropData(pGInfo);
			}
		}
		break;
	case ACDBDATACMD_GET_DEVICE_LIST:
		{
			if(pInput == NULL || nInputSize != sizeof(AcdbDevices))
			{
				ACDB_DEBUG_LOG("ACDBFILE_MGR: Received invalid input/output params\n");
				return ACDB_BADPARM;
			}
			else
			{
				AcdbDevices *pDevs = (AcdbDevices *)pInput;
				result = AcdbDataGetDevicesList(pDevs);
			}
		}
		break;
	case ACDBDATACMD_GET_FILE_TYPE_INFO:
		{
			if(pInput == NULL ||
				pOutput == NULL || nOutputSize != sizeof(AcdbFileInfo))
			{
				ACDB_DEBUG_LOG("ACDBFILE_MGR: Received invalid input/output params\n");
				return ACDB_BADPARM;
			}
			else
			{
				char *fileName = (char *)pInput;
				AcdbFileInfo *pFileInfo = (AcdbFileInfo *)pOutput;
				result = AcdbDataGetFileTypeInfo(fileName, pFileInfo);
			}
		}
		break;
	case ACDBDATACMD_GET_TABLE_INFO:
		{

			if(pInput == NULL || nInputSize != sizeof(AcdbTableCmd) ||
				pOutput == NULL || nOutputSize != sizeof(AcdbTableInfo))
			{
				if(pInput != NULL && nInputSize == sizeof(AcdbTableCmd) &&
					pOutput != NULL && nOutputSize == sizeof(AcdbTableInfoCVD))
				{
					AcdbTableCmd *pCmd = (AcdbTableCmd *)pInput;
					AcdbTableInfoCVD *pTbl = (AcdbTableInfoCVD *)pOutput;
					result = AcdbDataGetTableInfoCVD(pCmd,pTbl);
				}
				else
				{
					ACDB_DEBUG_LOG("ACDBFILE_MGR: Received invalid input/output params\n");
					return ACDB_BADPARM;
				}
			}
			else
			{
				AcdbTableCmd *pCmd = (AcdbTableCmd *)pInput;
				AcdbTableInfo *pTbl = (AcdbTableInfo *)pOutput;
				result = AcdbDataGetTableInfo(pCmd,pTbl);
			}
		}
		break;
	case ACDBDATACMD_GET_LOADED_FILES_INFO:
		{
			if(pInput == NULL || nInputSize == 0 ||
				pOutput == NULL || nOutputSize != sizeof(uint32_t))
			{
				ACDB_DEBUG_LOG("ACDB_FILEMGR: Received invalid input/output params\n");
				return ACDB_ERROR;
			}
			else
			{
				// The data should be filled in the following format
				// <no of files:4 bytes>
				// <file len:4 bytes>
				// <file name len:4 bytes>
				// <file data: file name len bytes>
				// . . . .

				uint32_t i = 0;
				uint32_t nBytesRequired = 0;
				int32_t offset =0;
				//<no of files:4 bytes>
				nBytesRequired += sizeof(gDataFileInfo.noOfFiles);

				for(i=0;i<gDataFileInfo.noOfFiles;i++)
				{
					//<file len:4 bytes>
					nBytesRequired += sizeof(gDataFileInfo.fInfo[i].nFileSize);
					//<file name len:4 bytes>
					nBytesRequired += sizeof(gDataFileInfo.fInfo[i].nFileNameLen);
					//<file data: file name len bytes>
					nBytesRequired += gDataFileInfo.fInfo[i].nFileNameLen;
				}
				if(nBytesRequired > nInputSize)
				{
					ACDB_DEBUG_LOG("ACDB_FILEMGR: Insufficient memory to copy the files info data\n");
					return ACDB_ERROR;
				}
				ACDB_MEM_CPY(&pInput[offset],sizeof(gDataFileInfo.noOfFiles),&gDataFileInfo.noOfFiles,sizeof(gDataFileInfo.noOfFiles));
				offset+=sizeof(gDataFileInfo.noOfFiles);
				for(i=0;i<gDataFileInfo.noOfFiles;i++)
				{
					//copy <file len>
					ACDB_MEM_CPY(&pInput[offset],sizeof(gDataFileInfo.fInfo[i].nFileSize),&gDataFileInfo.fInfo[i].nFileSize,sizeof(gDataFileInfo.fInfo[i].nFileSize));
					offset += sizeof(uint32_t);
					//copy <file name len>
					ACDB_MEM_CPY(&pInput[offset],sizeof(gDataFileInfo.fInfo[i].nFileNameLen),&gDataFileInfo.fInfo[i].nFileNameLen,sizeof(gDataFileInfo.fInfo[i].nFileNameLen));
					offset += sizeof(gDataFileInfo.fInfo[i].nFileNameLen);
					//copy <file data>
					ACDB_MEM_CPY(&pInput[offset],gDataFileInfo.fInfo[i].nFileNameLen,&gDataFileInfo.fInfo[i].chFileName[0],gDataFileInfo.fInfo[i].nFileNameLen);
					offset += gDataFileInfo.fInfo[i].nFileNameLen;
				}
				ACDB_MEM_CPY((void *)pOutput,sizeof(nBytesRequired),(void *)&nBytesRequired,sizeof(nBytesRequired));
			}
		}
		break;
	case ACDBDATACMD_GET_FILE_DATA:
		{
			if(pInput == NULL || nInputSize != sizeof(AcdbFileMgrGetFileDataReq) ||
				pOutput == NULL || nOutputSize != sizeof(AcdbFileMgrResp))
			{
				ACDB_DEBUG_LOG("ACDB_FILEMGR: Received invalid input/output params\n");
				return ACDB_ERROR;
			}
			else
			{
				AcdbFileMgrGetFileDataReq *pCmd = (AcdbFileMgrGetFileDataReq *)pInput;
				AcdbFileMgrResp *pRsp = (AcdbFileMgrResp *)pOutput;
				result = AcdbDataGetFileData(pCmd,pRsp);
			}
		}
		break;
	case ACDBDATACMD_GET_AVAILABLE_FILES_SLOT_COUNT:
		{
			int32_t no_of_avail_slots = 0;
			if(pOutput == NULL || nOutputSize != sizeof(uint32_t))
			{
				ACDB_DEBUG_LOG("ACDBFILE_MGR: Received invalid output params to provide the slot count info\n");
				return ACDB_BADPARM;
			}
			no_of_avail_slots = ACDB_MAX_ACDB_FILES - gDataFileInfo.noOfFiles;
			ACDB_MEM_CPY(pOutput,sizeof(int32_t),&no_of_avail_slots,sizeof(int32_t));
			result = ACDB_SUCCESS;
		}
		break;
	case ACDBDATACMD_GET_FILE_NAME:
		{
			if(pInput == NULL || nInputSize != sizeof(uint32_t) ||
				pOutput == NULL || nOutputSize != sizeof(AcdbCmdFileNameInfo))
			{
				ACDB_DEBUG_LOG("ACDBFILE_MGR: Received invalid input/output params\n");
				return ACDB_BADPARM;
			}
			else
			{
				uint32_t *pfileIndex = (uint32_t *)pInput;
				AcdbCmdFileNameInfo *pfileNameInfo = (AcdbCmdFileNameInfo *)pOutput;
				result = AcdbDataGetFileName(pfileIndex,pfileNameInfo);
			}
		}
		break;
	}
	return result;
}

