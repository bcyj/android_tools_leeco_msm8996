#ifndef __ACDB_PRIVATE_H__
#define __ACDB_PRIVATE_H__
/*===========================================================================
    @file   acdb_private.h

    This file contains the private interface to the Audio Calibration Database
    (ACDB) module.

    The private interface of the Audio Calibration Database (ACDB) module,
    providing calibration storage for the audio and the voice path. The ACDB
    module is coupled with Qualcomm Audio Calibration Tool (QACT) v2.x.x.
    Qualcomm recommends to always use the latest version of QACT for
    necessary fixes and compatibility.

                    Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_private.h#6 $ */

/*===========================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
2013-05-23  avi     Support Voice Volume boost feature
2011-06-22  ernanl  Introduced first version of 9615 ACDB Private API.

===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */
//File info
#define ACDB_MAX_ACDB_FILES 20

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

enum AcdbTableIds{
	AUDPROC_GAIN_INDP_TBL=1,
	AUDPROC_COPP_GAIN_DEP_TBL,
	AUDPROC_AUD_VOL_TBL,
	AUD_STREAM_TBL,
	VOCPROC_GAIN_INDP_TBL,
	VOCPROC_COPP_GAIN_DEP_TBL,
	VOC_STREAM_TBL,
	AFE_TBL,
	AFE_CMN_TBL,
	ADIE_ANC_TBL,
	ADIE_CODEC_TBL,
	GLOBAL_DATA_TBL,
	VOCPROC_DEV_CFG_TBL,
	LSM_TBL,
	CDC_FEATURES_TBL,
	ADIE_SIDETONE_TBL,
	AANC_CFG_TBL,
    VOCPROC_COPP_GAIN_DEP_V2_TBL,
    VOICE_VP3_TBL,
    AUDIO_REC_VP3_TBL,
    AUDIO_REC_EC_VP3_TBL,
	METAINFO_LOOKUP_TBL,
   VOCPROC_DYNAMIC_TBL,
   VOCPROC_STATIC_TBL,
   VOC_STREAM2_TBL,
};

typedef struct _AcdbQueryNoOfTblEntriesCmdType {
   uint32_t nTblId;
} AcdbQueryNoOfTblEntriesCmdType;

typedef struct _AcdbRespNoOfTblEntriesCmdType {
   uint32_t nNoOfEntries;
} AcdbRespNoOfTblEntriesCmdType;

typedef struct _AcdbQueryTblEntriesCmdType {
   uint32_t nTblId;
   uint32_t nTblEntriesOffset;
   uint8_t *pBuff;
   uint32_t nBuffSize;
} AcdbQueryTblEntriesCmdType;

#endif /* __ACDB_PRIVATE_H__ */
