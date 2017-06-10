#ifndef __ACDB_FILE_MGR_H__
#define __ACDB_FILE_MGR_H__
/*===========================================================================
    @file   acdb_data_mgr.h

    This file contains the implementation of the acdb file accessor interfaces.

                    Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_file_mgr.h#6 $ */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2014-05-28  mh      Updated some structure as a packed structure to avoid alignment issues
    2010-07-23  ernanl  Complete get/set function and associated functions API
===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */
#define ACDB_MAX_DEVICES 500

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

typedef struct _AcdbCmdFileNameInfo{
   uint32_t nFileNameLen;
   char chFileName[ACDB_FILENAME_MAX_CHARS];
}AcdbCmdFileNameInfo;

typedef struct _AcdbCmdFileInfo{
	char chFileName[ACDB_FILENAME_MAX_CHARS];
	uint32_t nFileNameLen;
	uint32_t nFileSize;
	uint8_t *pFileBuf;
}AcdbCmdFileInfo;

typedef struct _AcdbDataInfo
{
	uint32_t nDataLen;
	uint8_t *pData;
}AcdbDataInfo;

typedef struct _AcdbDevPropInfo{
	uint32_t devId;
	uint32_t pId;
	AcdbDataInfo dataInfo;
}AcdbDevPropInfo;

typedef struct _AcdbGlbalPropInfo{
	uint32_t pId;
	AcdbDataInfo dataInfo;
}AcdbGlbalPropInfo;

typedef struct _AcdbDevices
{
	uint32_t   noOfDevs;
	uint32_t   devList[ACDB_MAX_DEVICES];
}AcdbDevices;

typedef struct _AcdbFileInfo AcdbFileInfo;
#include "acdb_begin_pack.h"
struct _AcdbFileInfo
{
   int32_t fileType;
   uint32_t maj;
   uint32_t min;
   uint32_t rev;
   uint32_t noOfDevs;
   uint32_t *pDevList;
}
#include "acdb_end_pack.h"
;

typedef struct _VP3MidPidStruct
{
    uint32_t    nModuleId;
    uint32_t    nParamId;
    uint32_t    nMaxParamLen;
}VP3MidPidStruct;

typedef struct _AcdbTableInfo{
	AcdbDataInfo tblLutChnk;
	AcdbDataInfo tblCdftChnk;
	AcdbDataInfo tblCdotChnk;
	AcdbDataInfo dataPoolChnk;
}AcdbTableInfo;

typedef struct _AcdbTableInfoCVD{
	AcdbDataInfo tblLutChnk;
	AcdbDataInfo tblLutCVDChnk;
	AcdbDataInfo tblLutOffsetChnk;
	AcdbDataInfo tblCdftChnk;
	AcdbDataInfo tblCdotChnk;
	AcdbDataInfo dataPoolChnk;
}AcdbTableInfoCVD;

typedef struct _AcdbTableCmd{
	uint32_t devId;
	uint32_t tblId;
}AcdbTableCmd;

typedef struct _AcdbFileMgrGetFileDataReqType
{
	uint32_t nfile_offset;
	uint32_t nfile_data_len;
	uint32_t nfileNameLen;
	uint8_t *pFileName;
}AcdbFileMgrGetFileDataReq;

typedef struct _AcdbFileMgrRespType
{
	uint32_t nresp_buff_len;
	uint32_t nresp_buff_filled;
	uint8_t *pRespBuff;
}AcdbFileMgrResp;

enum AcdbDataCmds {
    ACDBDATACMD_SET_ACDB_DATA= 0,
	ACDBDATACMD_RESET,
	ACDBDATACMD_GET_DEVICE_PROP,
	ACDBDATACMD_GET_GLOBAL_PROP,
	ACDBDATACMD_GET_DEVICE_LIST,
	ACDBDATACMD_GET_TABLE_INFO,
    ACDBDATACMD_GET_AVAILABLE_FILES_SLOT_COUNT,
	ACDBDATACMD_GET_LOADED_FILES_INFO,
	ACDBDATACMD_GET_FILE_DATA,
	ACDBDATACMD_GET_HEAP_INFO,
   ACDBDATACMD_GET_FILE_TYPE_INFO,
   ACDBDATACMD_GET_FILE_NAME,
};

enum AcdbFileType{
   ACDB_AV_TYPE,
   ACDB_CODEC_TYPE,
   ACDB_GLOBAL_TYPE,
   ACDB_UNKNOWN_TYPE
};

int32_t acdbdata_ioctl (uint32_t nCommandId,
                        uint8_t *pInput,
                        uint32_t nInputSize,
                        uint8_t *pOutput,
                        uint32_t nOutputSize);

#endif /* __ACDB_FILE_MGR_H__ */
