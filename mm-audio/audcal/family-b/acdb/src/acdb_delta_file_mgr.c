/*===========================================================================
FILE: acdb_delta_file_mgr.c

OVERVIEW: This file contains the implementaion of the helper methods
to access delta acdb files info.

DEPENDENCIES: None

Copyright (c) 2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order. Please
use ISO format for dates.

$Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_delta_file_mgr.c#9 $

when who what, where, why
---------- --- -----------------------------------------------------
2014-05-28 mh Removed KW warnings/errors
2014-02-14 avi Support Delta acdb file manager.

========================================================================== */

/* ---------------------------------------------------------------------------
* Include Files
*--------------------------------------------------------------------------- */

#include "acdb_delta_file_mgr.h"
#include "acdb_private.h"
#include "acdb_init_utility.h"
#include "acdb_override.h"
#include "acdb_delta_parser.h"

/* ---------------------------------------------------------------------------
* Preprocessor Definitions and Constants
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Type Declarations
*--------------------------------------------------------------------------- */
typedef struct _AcdbDeltaDataFileInfo
{
	uint32_t noOfFiles;
	AcdbCmdDeltaFileInfo fInfo[ACDB_MAX_ACDB_FILES];
}AcdbDeltaDataFileInfo;

/* ---------------------------------------------------------------------------
* Global Data Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Static Variable Definitions
*--------------------------------------------------------------------------- */
static AcdbDeltaDataFileInfo gDeltaDataFileInfo;

/* ---------------------------------------------------------------------------
* Static Function Declarations and Definitions
*--------------------------------------------------------------------------- */

int32_t AcdbDeltaDataCmdInitData(AcdbCmdDeltaFileInfo *cmdInfo)
{
	int32_t result = ACDB_SUCCESS;
	uint32_t idx;

	if(cmdInfo == NULL)
	{
		ACDB_DEBUG_LOG("ACDBDELTAFILE_MGR: Received NULL input for AcdbDeltaDataCmdInitData\n");
		return ACDB_ERROR;
	}

	if(gDeltaDataFileInfo.noOfFiles == ACDB_MAX_ACDB_FILES)
	{
		ACDB_DEBUG_LOG("ACDBDELTAFILE_MGR: Request exceded the limit of 20 acdb files\n");
		return ACDB_ERROR;
	}

	idx = gDeltaDataFileInfo.noOfFiles++;
	gDeltaDataFileInfo.fInfo[idx].fileIndex = cmdInfo->fileIndex;
	gDeltaDataFileInfo.fInfo[idx].nFileInfo = cmdInfo->nFileInfo;
	gDeltaDataFileInfo.fInfo[idx].isFileUpdated = FALSE;
	gDeltaDataFileInfo.fInfo[idx].deltaFileExists = cmdInfo->deltaFileExists;
	gDeltaDataFileInfo.fInfo[idx].nFileSize = cmdInfo->nFileSize;
	gDeltaDataFileInfo.fInfo[idx].pFileBuf = cmdInfo->pFileBuf;

	return result;
}

int32_t AcdbDeltaDataCmdGetDataCount(uint32_t *delta_data_count)
{
	int32_t result = ACDB_SUCCESS;
	uint32_t idx =0;
	uint32_t cur_delta_data = 0;
	uint32_t offset = 0;
	for(idx=0;idx< gDeltaDataFileInfo.noOfFiles;idx++)
	{
		if(gDeltaDataFileInfo.fInfo[idx].deltaFileExists == TRUE && gDeltaDataFileInfo.fInfo[idx].pFileBuf != NULL)
		{
			offset = 0;
			// parse file and call DM_IOCTL.
			if(gDeltaDataFileInfo.fInfo[idx].nFileSize > sizeof(AcdbDeltaFileHeaderInfo) + sizeof(uint32_t))
			{
				offset += sizeof(AcdbDeltaFileHeaderInfo);
				offset += sizeof(uint32_t); // data_len

				if(offset < gDeltaDataFileInfo.fInfo[idx].nFileSize)
				{
					ACDB_MEM_CPY(&cur_delta_data,sizeof(uint32_t),gDeltaDataFileInfo.fInfo[idx].pFileBuf + offset,sizeof(uint32_t));
					*delta_data_count += cur_delta_data;
				}
			}
		}
	}
	return result;
}

int32_t AcdbDeltaDataCmdGetData(AcdbCmdDeltaFileDataInstance *pDataInstanceList)
{
	int32_t result = ACDB_SUCCESS;
	uint32_t idx =0;
	uint32_t instanceIdx = 0;
	for(idx=0;idx< gDeltaDataFileInfo.noOfFiles;idx++)
	{
		if(gDeltaDataFileInfo.fInfo[idx].deltaFileExists == TRUE && gDeltaDataFileInfo.fInfo[idx].pFileBuf != NULL)
		{
			// parse file and call DM_IOCTL.
			uint32_t data_len = 0;
			uint32_t cur_delta_data_count = 0;
			if(gDeltaDataFileInfo.fInfo[idx].nFileSize > ( sizeof(AcdbDeltaFileHeaderInfo) + sizeof(uint32_t) + sizeof(uint32_t)))
			{
				/* expected format
				fileMajor (uint32_t) | fileMinor (uint32_t) | major(uint32_t) | minor(uint32_t) | revision(uint32_t) | data_len (uint32_t) | num_records (uint32_t)
				tblId (uint32_t) | indicesCount (uint32_t) | pIndices (uint32_t * indicesCount) | mid (uint32_t) | pid (uint32_t) | dataLen (uint32_t) | pData (uint8_t * dataLen)
				.
				.
				.
				*/

				uint32_t curIdx = 0;
				uint8_t * fileBufPtr = gDeltaDataFileInfo.fInfo[idx].pFileBuf;

				// increment pointer by header length.
				fileBufPtr += sizeof(AcdbDeltaFileHeaderInfo); // file major, file minor, major, minor and revision.

				ACDB_MEM_CPY(&data_len,sizeof(uint32_t),fileBufPtr,sizeof(uint32_t));
				fileBufPtr += sizeof(uint32_t);

				ACDB_MEM_CPY(&cur_delta_data_count,sizeof(uint32_t),fileBufPtr,sizeof(uint32_t));
				fileBufPtr += sizeof(uint32_t);

				for(curIdx = 0;curIdx < cur_delta_data_count;curIdx++)
				{
					pDataInstanceList[instanceIdx].pTblId = (uint32_t *)fileBufPtr;
					fileBufPtr += sizeof(uint32_t);

					pDataInstanceList[instanceIdx].pIndicesCount = (uint32_t *)fileBufPtr;
					fileBufPtr += sizeof(uint32_t);

					pDataInstanceList[instanceIdx].pIndices = fileBufPtr;
					fileBufPtr += sizeof(uint32_t)*(*pDataInstanceList[instanceIdx].pIndicesCount);

					pDataInstanceList[instanceIdx].pMid = (uint32_t *)fileBufPtr;
					fileBufPtr += sizeof(uint32_t);

					pDataInstanceList[instanceIdx].pPid = (uint32_t *)fileBufPtr;
					fileBufPtr += sizeof(uint32_t);

					pDataInstanceList[instanceIdx].pDataLen = (uint32_t *)fileBufPtr;
					fileBufPtr += sizeof(uint32_t);

					pDataInstanceList[instanceIdx].pData = fileBufPtr;
					fileBufPtr += sizeof(uint8_t) * (*pDataInstanceList[instanceIdx].pDataLen);

					instanceIdx++;
				}
			}
		}
	}
	return result;
}

int32_t AcdbGetFileIndexFromIndices(uint32_t tblId, uint32_t indicesCount, uint8_t *pIndices)
{
	int32_t fileIdx = -1;
	int32_t tblFound = 1;
	uint32_t devIdxLocation = 0;
	int32_t fileType = ACDB_UNKNOWN_TYPE;
	switch(tblId)
	{
	case AUDPROC_GAIN_INDP_TBL:
	case AUDPROC_COPP_GAIN_DEP_TBL:
	case AUDPROC_AUD_VOL_TBL:
	case VOCPROC_GAIN_INDP_TBL:
	case VOCPROC_COPP_GAIN_DEP_TBL:
	case VOCPROC_DYNAMIC_TBL:
	case VOCPROC_STATIC_TBL:
	case AFE_TBL:
	case AFE_CMN_TBL:
	case ADIE_ANC_TBL:
	case VOCPROC_DEV_CFG_TBL:
	case LSM_TBL:
	case CDC_FEATURES_TBL:
	case ADIE_SIDETONE_TBL:
	case AANC_CFG_TBL:
	case VOCPROC_COPP_GAIN_DEP_V2_TBL:
	case VOICE_VP3_TBL:
	case AUDIO_REC_VP3_TBL:
	case AUDIO_REC_EC_VP3_TBL:
		fileType = ACDB_AV_TYPE;
		devIdxLocation = 0;
		break;
	case ADIE_CODEC_TBL:
		fileType = ACDB_CODEC_TYPE;
		break;
	case AUD_STREAM_TBL:
	case VOC_STREAM_TBL:
	case VOC_STREAM2_TBL:
	case GLOBAL_DATA_TBL:
	case METAINFO_LOOKUP_TBL:
		fileType = ACDB_GLOBAL_TYPE;
		break;
	default:
		tblFound = 0;
		break;
	}

	if(tblFound == 1)
	{
		int32_t idx = 0;
		for(idx=0;idx< (int32_t)gDeltaDataFileInfo.noOfFiles;idx++)
		{
			if(fileType == ACDB_CODEC_TYPE || fileType == ACDB_GLOBAL_TYPE)
			{
				if(gDeltaDataFileInfo.fInfo[idx].nFileInfo.fileType == fileType)
				{
					return idx;
				}
			}
			else //AV_TYPE
			{
				uint32_t devIdInData = 0;
				if(devIdxLocation < indicesCount)
				{
					uint32_t i=0;
					devIdInData = *(pIndices + sizeof(uint32_t)*devIdxLocation);
					for(i=0;i< gDeltaDataFileInfo.fInfo[idx].nFileInfo.noOfDevs;i++)
					{
						if(gDeltaDataFileInfo.fInfo[idx].nFileInfo.pDevList != NULL)
						{
							if(gDeltaDataFileInfo.fInfo[idx].nFileInfo.pDevList[i] == devIdInData)
							{
								return idx;
							}
						}
					}
				}
			}
		}
	}
	return fileIdx;
}

int32_t AcdbSetUpdatedFileIndexFlag(uint32_t fileIndex)
{
	int32_t result = ACDB_SUCCESS;

	if(fileIndex < gDeltaDataFileInfo.noOfFiles)
	{
		gDeltaDataFileInfo.fInfo[fileIndex].isFileUpdated = TRUE;
	}
	else
	{
		result = ACDB_ERROR;
	}

	return result;
}

int32_t AcdbDeltaDataCmdSaveData()
{
	int32_t result = ACDB_SUCCESS;
	int32_t write_result = ACDB_UTILITY_INIT_FAILURE;
	uint32_t idx =0;
	uint32_t data_len = 0;
	uint32_t new_size = 0;
	uint32_t header_offset = 0;
	uint8_t *newFileBuf = NULL;
	int32_t offset = 0;
	AcdbDeltaFileHeaderInfo deltaHeaderInfo;
	AcdbCmdFileNameInfo fileNameInfo;
	// expected format
	//tblId (uint32_t) | indicesCount (uint32_t) | pIndices (uint32_t * indicesCount) | mid (uint32_t) | pid (uint32_t) | dataLen (uint32_t) | pData (uint8_t * dataLen)

	for(idx=0;idx<gDeltaDataFileInfo.noOfFiles;idx++)
	{
		// only save updated files.
		if(gDeltaDataFileInfo.fInfo[idx].isFileUpdated != TRUE)
		{
			continue;
		}

		memset(&fileNameInfo.chFileName[0],0,ACDB_FILENAME_MAX_CHARS);
		fileNameInfo.nFileNameLen = 0;
		data_len = 0;
		result = Acdb_DM_Ioctl(ACDB_GET_DELTA_FILE_LENGTH,
			FALSE,
			idx,
			NULL,
			NULL,
			NULL,
			NULL,
			0,
			NULL,
			0,
			(uint8_t *)&data_len,
			sizeof(data_len),
			NULL,
			NULL,
			NULL,
			0
			);

		if(result != ACDB_SUCCESS)
		{
			ACDB_DEBUG_LOG("[ACDB DELTA MGR] [AcdbDeltaDataCmdSaveData] Unable to get size to save delta acdb file.");
			return ACDB_ERROR;
		}

		result = acdbdata_ioctl(ACDBDATACMD_GET_FILE_NAME, (uint8_t *)&idx,sizeof(uint32_t),(uint8_t *)&fileNameInfo,sizeof(fileNameInfo));

		if(data_len <= sizeof(uint32_t)) // no data present, so delete delta file.
		{
			write_result = AcdbDeleteDeltaFileData(fileNameInfo.chFileName,fileNameInfo.nFileNameLen);
			if(write_result != ACDB_UTILITY_INIT_SUCCESS)
			{
				ACDB_DEBUG_LOG("[ACDB DELTA MGR] [AcdbDeltaDataCmdSaveData] Unable to delete delta acdb file.\n");
				result = ACDB_ERROR;
			}
			gDeltaDataFileInfo.fInfo[idx].deltaFileExists = FALSE;
			gDeltaDataFileInfo.fInfo[idx].isFileUpdated = FALSE;
			return result;
		}

		// add meta info length
		header_offset = sizeof(AcdbDeltaFileHeaderInfo) + sizeof(uint32_t);

		new_size = data_len + header_offset;

		newFileBuf = (uint8_t *)ACDB_MALLOC(new_size);
		if(newFileBuf != NULL)
		{
			result = Acdb_DM_Ioctl(ACDB_GET_DELTA_FILE_DATA,
				FALSE,
				idx,
				NULL,
				NULL,
				NULL,
				NULL,
				0,
				NULL,
				0,
				(uint8_t *)(newFileBuf + header_offset),
				new_size - header_offset,
				NULL,
				NULL,
				NULL,
				0
				);

			// write header info.
			offset = 0;

			deltaHeaderInfo.fileMajor = ACDB_DELTA_FILE_VERSION_MAJOR;
			deltaHeaderInfo.fileMinor = ACDB_DELTA_FILE_VERSION_MINOR;
			deltaHeaderInfo.versMajor = gDeltaDataFileInfo.fInfo[idx].nFileInfo.maj;
			deltaHeaderInfo.versMinor = gDeltaDataFileInfo.fInfo[idx].nFileInfo.min;
			deltaHeaderInfo.versRevision = gDeltaDataFileInfo.fInfo[idx].nFileInfo.rev;

			ACDB_MEM_CPY(newFileBuf + offset, new_size, &deltaHeaderInfo, sizeof(deltaHeaderInfo));
			offset += sizeof(deltaHeaderInfo);

			ACDB_MEM_CPY(newFileBuf + offset, new_size, &data_len, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			if(result != ACDB_SUCCESS)
			{
				ACDB_DEBUG_LOG("[ACDB DELTA MGR] [AcdbDeltaDataCmdSaveData] Unable to get file name to save delta acdb file.\n");
				ACDB_MEM_FREE(newFileBuf);
				newFileBuf = NULL;
				return result;
			}
			write_result = AcdbWriteDeltaFileData(fileNameInfo.chFileName,fileNameInfo.nFileNameLen, newFileBuf, new_size);

			ACDB_MEM_FREE(newFileBuf);
			newFileBuf = NULL;
			if(write_result != ACDB_UTILITY_INIT_SUCCESS)
			{
				return ACDB_ERROR;
			}
			gDeltaDataFileInfo.fInfo[idx].deltaFileExists = TRUE;
			gDeltaDataFileInfo.fInfo[idx].isFileUpdated = FALSE;
		}
		else
		{
			return ACDB_ERROR;
		}
	}

	return result;
}

int32_t AcdbDeltaDataCmdReset()
{
	int32_t result = ACDB_SUCCESS;
	uint32_t idx =0;
	for(idx=0;idx< gDeltaDataFileInfo.noOfFiles;idx++)
	{
		if(gDeltaDataFileInfo.fInfo[idx].nFileInfo.pDevList != NULL)
		{
			ACDB_MEM_FREE(gDeltaDataFileInfo.fInfo[idx].nFileInfo.pDevList);
			gDeltaDataFileInfo.fInfo[idx].nFileInfo.pDevList = NULL;
		}

		memset(&gDeltaDataFileInfo.fInfo[idx].nFileInfo,0,sizeof(AcdbFileInfo));
		gDeltaDataFileInfo.fInfo[idx].deltaFileExists = FALSE;
		gDeltaDataFileInfo.fInfo[idx].nFileSize = 0;
		if(gDeltaDataFileInfo.fInfo[idx].pFileBuf != NULL)
		{
			ACDB_MEM_FREE(gDeltaDataFileInfo.fInfo[idx].pFileBuf);
			gDeltaDataFileInfo.fInfo[idx].pFileBuf = NULL;
		}
	}
	gDeltaDataFileInfo.noOfFiles = 0;
	return result;
}

int32_t AcdbDeltaDataCmdDeleteFiles()
{
	int32_t result = ACDB_BADPARM;
	int32_t delete_result = ACDB_UTILITY_INIT_FAILURE;
	uint32_t idx =0;
	AcdbCmdFileNameInfo fileNameInfo;

	// expected format
	//tblId (uint32_t) | indicesCount (uint32_t) | pIndices (uint32_t * indicesCount) | mid (uint32_t) | pid (uint32_t) | dataLen (uint32_t) | pData (uint8_t * dataLen)

	for(idx=0;idx<gDeltaDataFileInfo.noOfFiles;idx++)
	{
		// only save updated files.
		if(gDeltaDataFileInfo.fInfo[idx].deltaFileExists != TRUE)
		{
			continue;
		}

		result = acdbdata_ioctl(ACDBDATACMD_GET_FILE_NAME, (uint8_t *)&idx,sizeof(uint32_t),(uint8_t *)&fileNameInfo,sizeof(fileNameInfo));
		if(result != ACDB_SUCCESS)
		{
			ACDB_DEBUG_LOG("[ACDB DELTA MGR] [AcdbDeltaDataCmdSaveData] Unable to get file name to save delta acdb file.\n");
			return result;
		}

		delete_result = AcdbDeleteDeltaFileData(fileNameInfo.chFileName,fileNameInfo.nFileNameLen);

		if(delete_result != ACDB_UTILITY_INIT_SUCCESS)
		{
			return ACDB_ERROR;
		}

		gDeltaDataFileInfo.fInfo[idx].deltaFileExists = FALSE;
		gDeltaDataFileInfo.fInfo[idx].isFileUpdated = FALSE;
		result = ACDB_SUCCESS;
	}

	return result;
}

int32_t AcdbDeltaDataFreeBuf()
{
	int32_t result = ACDB_SUCCESS;
	uint32_t idx =0;

	// expected format
	//tblId (uint32_t) | indicesCount (uint32_t) | pIndices (uint32_t * indicesCount) | mid (uint32_t) | pid (uint32_t) | dataLen (uint32_t) | pData (uint8_t * dataLen)

	for(idx=0;idx<gDeltaDataFileInfo.noOfFiles;idx++)
	{
		if(gDeltaDataFileInfo.fInfo[idx].pFileBuf != NULL)
		{
			ACDB_MEM_FREE(gDeltaDataFileInfo.fInfo[idx].pFileBuf);
			gDeltaDataFileInfo.fInfo[idx].pFileBuf = NULL;

			gDeltaDataFileInfo.fInfo[idx].nFileSize = 0;
		}
	}

	return result;
}

int32_t acdb_delta_data_ioctl (uint32_t nCommandId,
	uint8_t *pInput,
	uint32_t nInputSize,
	uint8_t *pOutput,
	uint32_t nOutputSize)
{
	int32_t result = ACDB_SUCCESS;

	switch (nCommandId)
	{
	case ACDBDELTADATACMD_INIT_DELTA_ACDB_DATA:
		{
			if(pInput == NULL || nInputSize != sizeof(AcdbCmdDeltaFileInfo))
			{
				ACDB_DEBUG_LOG("ACDB_DELTAFILE_MGR: Received invalid input/output params\n");
				return ACDB_BADPARM;
			}
			result = AcdbDeltaDataCmdInitData((AcdbCmdDeltaFileInfo *)pInput);
		}
		break;
	case ACDBDELTADATACMD_GET_DELTA_ACDB_DATA_COUNT:
		{
			uint32_t delta_data_count = 0;
			if(pOutput == NULL || nOutputSize != sizeof(uint32_t))
			{
				ACDB_DEBUG_LOG("ACDB_DELTAFILE_MGR: Received invalid output params to provide the delta data count\n");
				return ACDB_BADPARM;
			}

			result = AcdbDeltaDataCmdGetDataCount(&delta_data_count);

			ACDB_MEM_CPY(pOutput,sizeof(uint32_t),&delta_data_count,sizeof(uint32_t));
			result = ACDB_SUCCESS;
		}
		break;
	case ACDBDELTADATACMD_GET_DELTA_ACDB_FILE_INDEX:
		{
			int32_t delta_file_index = 0;
			AcdbCmdDeltaFileIndexCmdType *pfileTypeType = NULL;
			if(pInput == NULL || nInputSize != sizeof(AcdbCmdDeltaFileIndexCmdType) ||
				pOutput == NULL || nOutputSize != sizeof(uint32_t))
			{
				ACDB_DEBUG_LOG("ACDB_DELTAFILE_MGR: Received invalid output params to provide the delta file index\n");
				return ACDB_BADPARM;
			}

			pfileTypeType = (AcdbCmdDeltaFileIndexCmdType *)pInput;
			delta_file_index = AcdbGetFileIndexFromIndices(*(pfileTypeType->pTblId), *(pfileTypeType->pIndicesCount), pfileTypeType->pIndices);

			if(delta_file_index == -1)
			{
				result = ACDB_ERROR;
			}
			else
			{
				ACDB_MEM_CPY(pOutput,sizeof(uint32_t),&delta_file_index,sizeof(uint32_t));
				result = ACDB_SUCCESS;
			}
		}
		break;
	case ACDBDELTADATACMD_SET_DELTA_ACDB_FILE_UPDATED:
		{
			uint32_t *pDeltaFileIndex = NULL;
			if(pInput == NULL || nInputSize != sizeof(uint32_t))
			{
				ACDB_DEBUG_LOG("ACDB_DELTAFILE_MGR: Received invalid input params to set updated delta file index\n");
				return ACDB_BADPARM;
			}

			pDeltaFileIndex = (uint32_t *)pInput;
			result = AcdbSetUpdatedFileIndexFlag(*pDeltaFileIndex);
		}
		break;
	case ACDBDELTADATACMD_GET_DELTA_ACDB_DATA:
		{
			if(pOutput == NULL)
			{
				ACDB_DEBUG_LOG("ACDB_DELTAFILE_MGR: Received invalid output params to get the delta data\n");
				return ACDB_BADPARM;
			}
			else
			{
				AcdbCmdDeltaFileDataInstance *pDataInstanceList = (AcdbCmdDeltaFileDataInstance *)pOutput;
				result = AcdbDeltaDataCmdGetData(pDataInstanceList);

				result = ACDB_SUCCESS;
			}
		}
		break;
	case ACDBDELTADATACMD_SAVE_DELTA_ACDB_DATA:
		{
			result = AcdbDeltaDataCmdSaveData();
		}
		break;
	case ACDBDELTADATACMD_DELTA_RESET:
		{
			result = AcdbDeltaDataCmdReset();
		}
		break;
	case ACDBDELTADATACMD_DELETE_DELTA_ACDB_FILES:
		{
			result = AcdbDeltaDataCmdDeleteFiles();
		}
		break;
	case ACDBDELTADATACMD_FREE_DELTA_ACDB_BUF:
		{
			result = AcdbDeltaDataFreeBuf();
		}
		break;
	}

	return result;
}
