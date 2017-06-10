// -*- Mode: C++ -*-
//============================================================
// FILE: aacconsts.h
//
// SERVICES: Audio
//
// DESCRIPTION:
/// @file: aacconsts.h
/// Declaration of constants required fro AAC Format Parser Implementation
///
/// Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
/// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AACParserLib/main/latest/inc/aacconstants.h#11 $
$DateTime: 2012/06/27 02:38:12 $
$Change: 2539269 $

========================================================================== */

#ifndef AAC_CONSTS_H
#define AAC_CONSTS_H

//============================================================
// INCLUDES
//============================================================


//============================================================
// CONSTANTS
//============================================================



//AAC File Header size
#define AAC_FILE_HEADER_SIZE 9

#define AAC_ADIF_HEADER_SIZE 64

//AAC Frame Header size
#define AAC_FRAME_HEADER_SIZE 7

//MAX ADTS frame is 2^13  = 8192
#define AAC_MAX_FRAME_SIZE 8192

#define AAC_CACHE_SIZE (5 * AAC_MAX_FRAME_SIZE)

//Preferred number of buffers for AAC
#define AAC_PREF_BUFF_NUM 4

// file i/o buffer size
#define AAC_READ_BUFFER_SIZE 1024

//AAC maximum number of frames in one output sample to decoder
#define AAC_MAX_FRAMES_IN_OUTPUT_SAMPLE 1
#define AAC_SAMPLES_PER_DATA_BLOCK 1024
#define AAC_ADTS_FRAME_PARSE_COUNT 2
#define LOAS_FIXED_SYNC_SIZE 3

//2Bytes are used to append CRC data as part of frame header.
//This field is optional.
#define AAC_CRC_DATA_LEN 2

#endif // AAC_CONSTS_H

