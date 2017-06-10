#ifndef OGG_STREAM_CONSTANTS_DEFN
#define OGG_STREAM_CONSTANTS_DEFN

/* =======================================================================
                              OGGStreamParserConstants.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2009-2014 QUALCOMM Technologies Incorporated, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/OGGParserLib/main/latest/inc/OGGStreamParserConstants.h#10 $
========================================================================== */

#include "AEEStdDef.h"

#define PAGE_MAGIC_NUMBER_SIZE     4
#define MAX_PAGE_SIZE              65307
#define PAGE_FIXED_HDR_SIZE        28

#define OGG_PAGE_MAGIC_NUMBER      "OggS"
#define DEFAULT_AUDIO_BUFF_SIZE    MAX_PAGE_SIZE
#define DEFAULT_VIDEO_BUFF_SIZE    MAX_PAGE_SIZE

#define VORBIS_CODEC               "vorbis"
#define THEORA_CODEC               "theora"
#define FLAC_CODEC                 "FLAC"

#define VORBIS_CODEC_SIGN_SIZE     6
#define THEORA_CODEC_SIGN_SIZE     6
#define FLAC_CODEC_SIGN_SIZE       4

#define OGG_STREAM_VERSION_FORMAT  0x00

#define OGG_CONT_PAGE              0x01
#define OGG_BOS_PAGE               0x02
#define OGG_EOS_PAGE               0x04

#define VORBIS_IDENT_HDR_BYTE      0x01
#define VORBIS_COMMENT_HDR_BYTE    0x03
#define VORBIS_SETUP_HDR_BYTE      0x05

#define THEORA_IDENT_HDR_BYTE      0x80
#define THEORA_COMMENT_HDR_BYTE    0x81
#define THEORA_SETUP_HDR_BYTE      0x82

#define FLAC_IDENT_HDR_BYTE        0x7f

#define DEFAULT_OGG_PAGES_TO_INDEX 1000

#define OGG_GENERATOR_POLYNOMIAL   0x04c11db7L
#define OGG_CHECKSUM_OFFSET        22
#define OGG_NUM_CHECKSUM_BYTES     4

// Seek Time Adjust is used to target a prior locatio
// before the actual page to which we have to seek.
// Once this adjusted location is reached use page by page seek
#define OGGPARSER_SEEK_TIME_ADJUST 2000
#define OGGPARSER_SEEK_MAX_DEVIATION_MS 2000


#define MSBIT(x)    (x & 0x80000000L )

#define OGG_APPROX_SEEK_MAX_PASS   4
#define OGG_APPROX_LIMIT_START_RANGE 4000

#endif//#ifndef OGG_STREAM_CONSTANTS_DEFN
