#ifndef AVI_CONSTANTS_H
#define AVI_CONSTANTS_H

/* =======================================================================
                              aviConstants.h
DESCRIPTION

Copyright (c) 2011-2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/inc/aviconstants.h#22 $
========================================================================== */

#include "avidatatypes.h"

#ifdef FILESOURCE_LEGACY_DIVX_DRM
  #define DRM_REGISTRATION_CODE_BYTES 9
#endif
//Define MIN and MAX STRF header size.
#define MIN_AUDIO_STRF_HDR_SIZE 16
#define AVI_STRF_MAX_EXTRA_SIZE 22

//Define MAX audio/video tracks
#define AVI_MAX_VIDEO_TRACKS        6
#define AVI_MAX_AUDIO_TRACKS        8
#define AVI_MAX_TEXT_TRACKS         8

//Define MAX number of tracks per clip.
#define AVI_MAX_TRACKS ( (AVI_MAX_VIDEO_TRACKS) + (AVI_MAX_AUDIO_TRACKS) + (AVI_MAX_TEXT_TRACKS) )

//4 byte FOURCC and 4 byte size
#define AVI_SIZEOF_FOURCC_PLUS_LENGTH  ((sizeof(fourCC_t))+(sizeof(avi_uint32)))

//Various format tag to identify the audio track.
#define AVI_AUDIO_PCM 0x0001
#define AVI_AUDIO_MP2 0x0050
#define AVI_AUDIO_MP3 0x0055
#define AVI_AUDIO_AC3 0x2000
#define AVI_AUDIO_DTS 0x2001
#define AVI_AUDIO_AAC 0x00ff
#define AVI_AUDIO_WMA_STD 0x0161
#define AVI_AUDIO_WMA_PRO 0x0162
#define AVI_AUDIO_AAC_FAAD 0x706d

#define AVI_IDX1_ENTRY_SIZE 16

#define AVI_DEFAULT_VIDEO_BUF_SIZE 76000
#define AVI_DEFAULT_AUDIO_BUF_SIZE 128000

#define AVI_READ_BUFFER_SIZE  1024  // file i/o buffer size
#define AVI_SUBTITLE_HDR_SIZE 57    //27 bytes of duration + 30 bytes

//If there are more than 1200 audio frames in given AVI file,
//Parser will store audio entries at multiple of AVI_AUDIO_ENTRY_INTERVAL milliseconds.

#define AVI_AUDIO_ENTRY_INTERVAL          300000
#define AVI_DEFAULT_AUDIO_ENTRIES_TO_LOAD 3600
#define AVI_DEFAULT_KEY_ENTRIES_TO_LOAD   300

#define AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE (AVI_IDX1_ENTRY_SIZE * 1000)

#define AVI_KEY_FRAME_MASK 0x00000010

#define DIVX_DRM_FRAME_DRM_INFO_SIZE               10

//Constant to distinguish various types/sub-types of AVI index.
#define AVI_INDEX_OF_INDEXES 0x00
#define AVI_INDEX_OF_CHUNKS  0x01
#define AVI_INDEX_OF_DATA    0x80
#define AVI_INDEX_2FIELD     0x01

#define MAX_VOL_HDR_SIZE 1024

#define AUDIO_SAMPLE_CACHING_INTERVAL 1000
/*
* DIVX encoder puts start codes in little endian
* such as 0x00000100 and 0x00000120.
* Following are corresponding codes in big endian to do the match.
*/

static const int IVOP_START_CODE     = 0x00010000;
static const int IVOP_VOL_START_CODE = 0x20010000;

/*
* Constants to be used for constructing VOL header.
* DIVX encoder does not put Visual Object Sequence Start Code,
* Visual Object Start code,Profile and level indication and
* video signal type.
*/
static const int MPEG4_VISUAL_OBJECT_SEQUENCE_START_CODE = 0xB0010000;// 0x000001B0;
static const int MPEG4_VISUAL_OBJECT_START_CODE          = 0xB5010000; //0x000001B5;
/*
* DIVX encoder puts start code in little endian such as 0x000001B6
* Following is corresponding big endian to do the match.
*/
static const int MPEG4_VOP_HEADER_CODE                   = 0xB6010000;
static const int MPEG4_VOP_HEADER_CODE_FMP4              = 0xB2010000;
//static int MPEG4_VOP_HEADER_CODE                   = 0xB2010000;

static const int MPEG4_PROFILE_AND_LEVEL_INDICATION      = 0xF5;

/*
* 0                 0 0 0 1          0                 0 1 = 0x09
* visual obj. iden
*                   visual obj type
*                                    video signal type
*                                                      unused
*/
static const int MPEG4_VIDEO_OBJECT_IDEN_VIDEO_SIG_TYPE  =     0x09;

#define AVI_INVALID_TRACK_ID 0xFFFF
#define AAC_CONFIG_DATA_BUFFER_SIZE 0x1388
#define AVI_AAC_ADTS_HDR_LEN 7
#endif
