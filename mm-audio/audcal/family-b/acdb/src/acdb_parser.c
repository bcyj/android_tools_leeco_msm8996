/*===========================================================================
    FILE:           acdb_parser.c

    OVERVIEW:       This file is the parser for the Acdb file. This is an
                    optimization from the previous parser to not use so
                    much ARM9 heap at any given time. This implementation
                    will use the stack from the context of the caller.

    DEPENDENCIES:   None

                    Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_parser.c#3 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2010-06-26  vmn     Establish initial implementation.

========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
//#include "acdb_os_includes.h"
#include "acdb_parser.h"
//#include "acdb_init.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

#define ACDB_CHUNK_ID_SIZE 8
#define ACDB_CHUNK_SIZE 4
#define ACDH_CHUNK_HEADER_SIZE (ACDB_CHUNK_ID_SIZE + ACDB_CHUNK_SIZE)

#define ACDB_GLOBAL_FILE_TYPE			 0x42444347 //  GCDB
#define ACDB_CODEC_FILE_TYPE		     0x42444343 //  CCDB
#define ACDB_AV_FILE_TYPE                0x42445641 //  AVDB

#define ACDB_FILE_ID			            0x4244444e534d4351ULL //  QCMSNDDB

#define ACDB_FILE_COMPRESSED 0x1

#define ACDB_FILE_VERSION_MAJOR                      0x00000000
#define ACDB_FILE_VERSION_MINOR                      0x00000000

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

typedef struct _AcdbFileProperties
{
   uint64_t id;
   uint32_t major;
   uint32_t minor;
   uint32_t acdb_type;
   uint32_t comp_flag;
   uint32_t acdb_actual_len;
   uint32_t acdb_data_len;
} AcdbFileProperties;

typedef struct _AcdbSwVersion
{
	uint32_t major;
	uint32_t minor;
	uint32_t rev;
}AcdbSwVersion;

#include "acdb_begin_pack.h"
struct _AcdbChunkHeader
{
   uint64_t id;
   uint32_t length;
}
#include "acdb_end_pack.h"
;

typedef struct _AcdbChunkHeader		AcdbChunkHeader;

/* ---------------------------------------------------------------------------
 * Global Data Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Variable Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

int32_t AcdbFileGetChunkData(const uint8_t *pFileBuf,const uint32_t nFileBufLen,uint64_t chunkID,uint8_t **pChkBuf,uint32_t *pChkLen)
{
	int32_t result = ACDB_PARSE_CHUNK_NOT_FOUND;
	uint32_t offset = 0;
	if(ACDB_PARSE_SUCCESS != IsAcdbFileValid(pFileBuf,nFileBufLen))
		return ACDB_PARSE_INVALID_FILE;

	// skip the file properties part of the acdb file to point to the first chunk
	offset += sizeof(AcdbFileProperties);
	while(offset < nFileBufLen && ((nFileBufLen-offset)>=sizeof(AcdbChunkHeader)))
	{
		AcdbChunkHeader ChkHdr = {0};
		AcdbChunkHeader *pChkHdr = &ChkHdr;
		ACDB_MEM_CPY(&ChkHdr,sizeof(AcdbChunkHeader),pFileBuf + offset, sizeof(AcdbChunkHeader));

		offset += ACDH_CHUNK_HEADER_SIZE;
		if(pChkHdr->id == chunkID)
		{
			*pChkBuf = (uint8_t *)(pFileBuf + offset);
			*pChkLen = pChkHdr->length;
			result = ACDB_PARSE_SUCCESS;
			break;
		}
		offset += pChkHdr->length;
	}
	return result;
}

int32_t IsAcdbFileValid(const uint8_t *pFileBuf,const uint32_t nFileSize)
{
	AcdbFileProperties FileProp = {0};
	AcdbFileProperties *pFileProp = &FileProp;
	ACDB_MEM_CPY(&FileProp,sizeof(AcdbFileProperties),pFileBuf, sizeof(AcdbFileProperties));
	//const AcdbFileProperties *pFileProp = (AcdbFileProperties *)pFileBuf;
	if(pFileProp->id != ACDB_FILE_ID)
		return ACDB_PARSE_INVALID_FILE;

	if(nFileSize < sizeof(AcdbFileProperties))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->major > ACDB_FILE_VERSION_MAJOR)
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->major == ACDB_FILE_VERSION_MAJOR && pFileProp->minor > ACDB_FILE_VERSION_MINOR)
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->acdb_type != ACDB_GLOBAL_FILE_TYPE &&
		pFileProp->acdb_type != ACDB_CODEC_FILE_TYPE &&
		pFileProp->acdb_type != ACDB_AV_FILE_TYPE)
	{
		return ACDB_PARSE_INVALID_FILE;
	}

	if(nFileSize != (pFileProp->acdb_data_len + sizeof(AcdbFileProperties)))
		return ACDB_PARSE_INVALID_FILE;

	return ACDB_PARSE_SUCCESS;
}

int32_t IsCodecFileType(const uint8_t *pFileBuf,const uint32_t nFileSize)
{
	AcdbFileProperties FileProp = {0};
	AcdbFileProperties *pFileProp = &FileProp;
	ACDB_MEM_CPY(&FileProp,sizeof(AcdbFileProperties),pFileBuf, sizeof(AcdbFileProperties));
	//const AcdbFileProperties *pFileProp = (AcdbFileProperties *)pFileBuf;
	if(pFileProp->id != ACDB_FILE_ID)
		return ACDB_PARSE_INVALID_FILE;

	if(nFileSize < sizeof(AcdbFileProperties))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->major > ACDB_FILE_VERSION_MAJOR)
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->major == ACDB_FILE_VERSION_MAJOR && pFileProp->minor > ACDB_FILE_VERSION_MINOR)
		return ACDB_PARSE_INVALID_FILE;

	if(nFileSize != (pFileProp->acdb_data_len + sizeof(AcdbFileProperties)))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->acdb_type == ACDB_CODEC_FILE_TYPE)
	{
		return ACDB_PARSE_SUCCESS;
	}

	return ACDB_PARSE_FAILURE;
}

int32_t IsAVFileType(const uint8_t *pFileBuf,const uint32_t nFileSize)
{
	AcdbFileProperties FileProp = {0};
	AcdbFileProperties *pFileProp = &FileProp;
	ACDB_MEM_CPY(&FileProp,sizeof(AcdbFileProperties),pFileBuf, sizeof(AcdbFileProperties));
	//const AcdbFileProperties *pFileProp = (AcdbFileProperties *)pFileBuf;;
	if(pFileProp->id != ACDB_FILE_ID)
		return ACDB_PARSE_INVALID_FILE;

	if(nFileSize < sizeof(AcdbFileProperties))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->major > ACDB_FILE_VERSION_MAJOR)
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->major == ACDB_FILE_VERSION_MAJOR && pFileProp->minor > ACDB_FILE_VERSION_MINOR)
		return ACDB_PARSE_INVALID_FILE;

	if(nFileSize != (pFileProp->acdb_data_len + sizeof(AcdbFileProperties)))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->acdb_type == ACDB_AV_FILE_TYPE)
	{
		return ACDB_PARSE_SUCCESS;
	}

	return ACDB_PARSE_FAILURE;
}

int32_t IsGlobalFileType(const uint8_t *pFileBuf,const uint32_t nFileSize)
{
	AcdbFileProperties FileProp = {0};
	AcdbFileProperties *pFileProp = &FileProp;
	ACDB_MEM_CPY(&FileProp,sizeof(AcdbFileProperties),pFileBuf, sizeof(AcdbFileProperties));
	//const AcdbFileProperties *pFileProp = (AcdbFileProperties *)pFileBuf;
	if(pFileProp->id != ACDB_FILE_ID)
		return ACDB_PARSE_INVALID_FILE;

	if(nFileSize < sizeof(AcdbFileProperties))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->major > ACDB_FILE_VERSION_MAJOR)
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->major == ACDB_FILE_VERSION_MAJOR && pFileProp->minor > ACDB_FILE_VERSION_MINOR)
		return ACDB_PARSE_INVALID_FILE;

	if(nFileSize != (pFileProp->acdb_data_len + sizeof(AcdbFileProperties)))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->acdb_type == ACDB_GLOBAL_FILE_TYPE)
	{
		return ACDB_PARSE_SUCCESS;
	}

	return ACDB_PARSE_FAILURE;
}

int32_t IsAcdbFileZipped(const uint8_t *pFileBuf, const uint32_t nFileSize)
{
	AcdbFileProperties FileProp = {0};
	AcdbFileProperties *pFileProp = &FileProp;
	ACDB_MEM_CPY(&FileProp,sizeof(AcdbFileProperties),pFileBuf, sizeof(AcdbFileProperties));
	//const AcdbFileProperties *pFileProp = (AcdbFileProperties *)pFileBuf;
	if(pFileProp->id != ACDB_FILE_ID)
		return ACDB_PARSE_INVALID_FILE;

	if(nFileSize < sizeof(AcdbFileProperties))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->acdb_type != ACDB_GLOBAL_FILE_TYPE &&
		pFileProp->acdb_type != ACDB_CODEC_FILE_TYPE &&
		pFileProp->acdb_type != ACDB_AV_FILE_TYPE)
	{
		return ACDB_PARSE_INVALID_FILE;
	}

	if(nFileSize != (pFileProp->acdb_data_len + sizeof(AcdbFileProperties)))
		return ACDB_PARSE_INVALID_FILE;

	if(pFileProp->comp_flag != ACDB_FILE_COMPRESSED)
		return ACDB_PARSE_FAILURE;

	return ACDB_PARSE_COMPRESSED;
}

int32_t AcdbFileGetVersion(const uint8_t *pFileBuf,const uint32_t nFileSize,uint32_t *pMajVer,uint32_t *pMinVer,uint32_t *pRevVer)
{
	uint8_t *pChkBuf = NULL;
	uint32_t nChkBufSize=0;
	AcdbSwVersion *swver = NULL;

	int32_t result = AcdbFileGetChunkData(pFileBuf,nFileSize,ACDB_CHUNKID_SWVERS,&pChkBuf,&nChkBufSize);
	if(result != ACDB_PARSE_SUCCESS)
		return result;
	if(nChkBufSize != sizeof(AcdbSwVersion))
		return ACDB_PARSE_FAILURE;
	swver = (AcdbSwVersion *)pChkBuf;
	*pMajVer = swver->major;
	*pMinVer = swver->minor;
        *pRevVer = swver->rev;
	return ACDB_PARSE_SUCCESS;
}

int32_t AcdbFileGetSWVersion(const uint8_t *pFileBuf,const uint32_t nFileSize,uint32_t *pMajVer,uint32_t *pMinVer,uint32_t *pRevVer)
{
	uint8_t *pChkBuf = NULL;
	uint32_t nChkBufSize=0;
	AcdbSwVersion *swver = NULL;

	int32_t result = AcdbFileGetChunkData(pFileBuf,nFileSize,ACDB_CHUNKID_ACSWVERS,&pChkBuf,&nChkBufSize);
	if(result != ACDB_PARSE_SUCCESS)
   {
       if(result==ACDB_PARSE_CHUNK_NOT_FOUND)//old version of file there is no sw version chunk in it.
       {
              return(AcdbFileGetVersion(pFileBuf,nFileSize,pMajVer,pMinVer,pRevVer));
       }
		 return result;
   }
   else//new version verify minimum compatible version
   {
      if(nChkBufSize != sizeof(AcdbSwVersion))
		  return ACDB_PARSE_FAILURE;
	   swver = (AcdbSwVersion *)pChkBuf;
	   *pMajVer = swver->major;
	   *pMinVer = swver->minor;
           *pRevVer = swver->rev;
   }
   return ACDB_PARSE_SUCCESS;
}
