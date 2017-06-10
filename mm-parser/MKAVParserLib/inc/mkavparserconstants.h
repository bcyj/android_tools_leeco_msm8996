#ifndef MKAV_PARSER_CONSTANTS_DEFN
#define MKAV_PARSER_CONSTANTS_DEFN

/* =======================================================================
                              MKAVParserConstants.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2011-2015 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MKAVParserLib/main/latest/inc/mkavparserconstants.h#23 $
========================================================================== */

#include "AEEStdDef.h"

#define MKAV_MSFT_CODEC_MGR                   "V_MS/VFW/FOURCC"
#define MKAV_RAW_UNCOMPRESSED_VIDEO           "V_UNCOMPRESSED"
#define MKAV_MPEG4_ISO_SIMPLE_PROFILE         "V_MPEG4/ISO/SP"
#define MKAV_MPEG4_ISO_ADVANCE_SIMPLE_PROFILE "V_MPEG4/ISO/ASP"
#define MKAV_MPEG4_ISO_ADVANCE_PROFILE        "V_MPEG4/ISO/AP"
#define MKAV_AVC1_VIDEO                       "V_MPEG4/ISO/AVC"
#define MKAV_THEORA_VIDEO                     "V_THEORA"
#define MKAV_MPEG2_VIDEO                      "V_MPEG2"
#define MKAV_MPEG1_VIDEO                      "V_MPEG1"
#define MKAV_MICROSOFT_MPEG_V3                "V_MPEG4/MS/V3"
#define MKAV_HEVC_VIDEO                       "V_MPEGH/ISO/HEVC"
#define MKAV_MPEG_AUDIO_L3                    "A_MPEG/L3"
#define MKAV_MPEG_AUDIO_L2                    "A_MPEG/L2"
#define MKAV_MPEG_AUDIO_L1                    "A_MPEG/L1"
#define MKAV_PCM_INT_BENDIAN                  "A_PCM/INT/BIG"
#define MKAV_PCM_INT_LENDIAN                  "A_PCM/INT/LIT"
#define MKAV_PCM_FLOAT                        "A_PCM/FLOAT/IEEE"
#define MKAV_AC3_AUDIO                        "A_AC3"
#define MKAV_EAC3_AUDIO                       "A_EAC3"
#define MKAV_DOLBY_AC3                        "A_AC3/BSID9"
#define MKAV_DTS_AUDIO                        "A_DTS"
#define MKAV_FLAC_AUDIO                       "A_FLAC"
#define MKAV_VORBIS_AUDIO                     "A_VORBIS"
#define MKAV_AAC_MPEG2_MAIN_PROFILE           "A_AAC/MPEG2/MAIN"
#define MKAV_AAC_MPEG2_LOW_COMPLEXITY         "A_AAC/MPEG2/LC"
#define MKAV_AAC_MPEG2_LOW_COMPLEXITY_SBR     "A_AAC/MPEG2/LC/SBR"
#define MKAV_AAC_MPEG2_SSR                    "A_AAC/MPEG2/SSR"
#define MKAV_AAC_MPEG4_MAIN_PROFILE           "A_AAC/MPEG4/MAIN"
#define MKAV_AAC_MPEG4_LOW_COMPLEXITY         "A_AAC/MPEG4/LC"
#define MKAV_AAC_MPEG4_LOW_COMPLEXITY_SBR     "A_AAC/MPEG4/LC/SBR"
#define MKAV_AAC_MPEG4_SSR                    "A_AAC/MPEG4/SSR"
#define MKAV_AAC_MPEG4_LTP                    "A_AAC/MPEG4/LTP"
#define MKAV_MSFT_AUDIO_CODEC_MGR             "A_MS/ACM"
#define MKAV_OPUS_AUDIO                       "A_OPUS"
#define MKAV_ST_UTF8                          "S_TEXT/UTF8"
#define MKAV_ST_SSA                           "S_TEXT/SSA"
#define MKAV_ST_ASS                           "S_TEXT/ASS"
#define MKAV_ST_USF                           "S_TEXT/USF"
#define MKAV_ST_BMP                           "S_IMAGE/BMP"
#define MKAV_ST_VOBSUB                        "S_VOBSUB"
#define MKAV_ST_KARAOKE                       "S_KATE"
#define MKAV_VFW_SPARK_VIDEO                  "FLV1"
#define MKAV_VFW_SORENSON_VIDEO               "SVQ1"
#define MKAV_VP8_VIDEO_CODEC                  "V_VP8"
#define MKAV_VP9_VIDEO_CODEC                  "V_VP9"
#define MKAV_VFW_DIV3_VIDEO                   "DIV3"
#define MKAV_VFW_DIV4_VIDEO                   "DIV4"
#define MKAV_VFW_I263_VIDEO                   "I263"
#define MKAV_VFW_H263_VIDEO                   "H263"
#define MKAV_VFW_DIVX_VIDEO                   "DIVX"
#define MKAV_VFW_DX40_VIDEO                   "DX40"
#define MKAV_VFW_DX50_VIDEO                   "DX50"
#define MKAV_VFW_XVID_VIDEO                   "XVID"
#define MKAV_VFW_MPEG4_VIDEO                  "MP4V"
#define MKAV_VFW_MP42_VIDEO                   "MP42"
#define MKAV_VFW_WMV3_VIDEO                   "WMV3"
#define MKAV_VFW_WMV2_VIDEO                   "WMV2"
#define MKAV_VFW_WMV1_VIDEO                   "WMV1"
#define MKAV_VFW_WVC1_VIDEO                   "WVC1"
#define MKAV_VFW_WMVA_VIDEO                   "WMVA"
#define MKAV_WAVE_FORMAT_MSAUDIO1             0x160
#define MKAV_WAVE_FORMAT_MSAUDIO2             0x161
#define MKAV_WAVE_FORMAT_MSAUDIO3             0x162
#define MKAV_WAVE_FORMAT_MSAUDIO4             0x163
#define MKAV_WAVE_FORMAT_MSSPEECH             0x0A

/* This Macro used to parse metadata inside Block/BlockGroup elements.
   Inside Blocks, Media data (actual frame data) will be available after
   metadata. This Metadata contains number of frames stored and each frame
   size value. */
#define MAX_ELEMENT_SIZE                      256
#define EBML_SIZE_TBL_MX_ROWS                 256
#define DEF_DATA_BUF_SIZE                     8192
#define EBML_SIZE_TBL_MX_COLUMNS              3
#define EBML_ELEMENT_ID_SIZE_THREE            3

#define XIPH_LACING_ID                        0x02
#define FIXED_SIZE_LACING_ID                  0x04
#define EBML_LACING_ID                        0x06
#define INVISIBLE_FRAME_ID                    0x08
#define KEY_FRAME_ID                          0x80
#define METADATA_STRING_LENGTH                256
#define AC3_SAMPLE_BLOCK_SIZE                 256
#define AC3_MAX_NUM_BLOCKS                    6
#define MKAV_VORBIS_CODEC_START_SIZE          7
#define MKAV_HEVC_CODEC_RESERVED_DATA_SIZE    (21)
const uint8  MKAV_VORBIS_CODEC_START[MKAV_VORBIS_CODEC_START_SIZE]  ={0x01,0x76,0x6f,0x72,0x62,0x69,0x73};
const uint64 MKAV_MILLISECONDS_TIMECODE       = 1000000;

//! These are required to check tool used to generate some MKV content
#define DIVX_PLUS        "DIVXPLUS"
#define DIVX_MUXING_APP  "LIBDIVXMEDIAFORMAT"
#define DIVX_WRITING_APP "DIVXMKVMUX"

const uint8  EBML_VOID_ID                     = 0xEC;
const uint8  EBML_CRC_ID                      = 0xBF;

const uint32 EBML_DOC_ID                      = 0x1A45DFA3;
const uint16 EBML_VER_ID                      = 0x4286;
const uint16 EBML_READ_VER_ID                 = 0x42F7;
const uint16 EBML_MX_ID_LENGTH_ID             = 0x42F2;
const uint16 EBML_MX_SZ_LENGTH_ID             = 0x42F3;
const uint16 EBML_DOCTYPE_ID                  = 0x4282;
const uint16 EBML_DOCTYPE_VER_ID              = 0x4287;
const uint16 EBML_DOCTYPE_READ_VER_ID         = 0x4285;

const uint32 EBML_SEGMENT_ID                  = 0x18538067;
const uint32 EBML_SEEK_HEAD_ID                = 0x114D9B74;
const uint16 EBML_SEEK_ELEMENT_ID             = 0x4DBB;
const uint16 EBML_SEEK_ID                     = 0x53AB;
const uint16 EBML_SEEK_POSN_ID                = 0x53AC;

const uint32 EBML_SEGMENT_INFO_ID             = 0x1549A966;
const uint16 EBML_SEGMENTUID_ID               = 0x73A4;
const uint16 EBML_SEGFILENAME_ID              = 0x7384;
const uint32 EBML_PRVUID_ID                   = 0x3CB923;
const uint32 EBML_PRVFILENAME_ID              = 0x3C83AB;
const uint32 EBML_NEXTUID_ID                  = 0x3EB923;
const uint32 EBML_NXTFILENAME_ID              = 0x3E83BB;
const uint32 EBML_TIMECODESCLE_ID             = 0x2AD7B1;
const uint16 EBML_DURATION_ID                 = 0x4489;
const uint16 EBML_TITLE_ID                    = 0x7BA9;
const uint16 EBML_MUX_APP_ID                  = 0x4D80;
const uint16 EBML_WRITER_APP_ID               = 0x5741;
const uint16 EBML_DATE_UTC_ID                 = 0x4461;

const uint32 EBML_TRACKS_ID                   = 0x1654AE6B;
const uint8  EBML_TRACK_ENTRY_ID              = 0xAE;
const uint8  EBML_TRACKNO_ID                  = 0xD7;
const uint8  EBML_TRACKTYPE_ID                = 0x83;
const uint8  EBML_FLG_ENABLE_ID               = 0xB9;
const uint8  EBML_FLG_DEF_ID                  = 0x88;
const uint8  EBML_FLG_LACING_ID               = 0x9C;
const uint8  EBML_TRACK_CODEC_ID              = 0x86;
const uint8  EBML_TRACK_VIDEO_ID              = 0xE0;
const uint8  EBML_TRACK_AUDIO_ID              = 0xE1;
const uint16 EBML_MIN_CACHE_ID                = 0x6DE7;
const uint16 EBML_MX_CACHE_ID                 = 0x6DF8;
const uint16 EBML_FLG_FLG_FORCED_ID           = 0x55AA;
const uint16 EBML_TRACKUID_ID                 = 0x73C5;
const uint16 EBML_TRACK_CODEC_PVT_ID          = 0x63A2;
const uint16 EBML_TRACK_CONTENTS_ENCODING_ID  = 0x6D80;
const uint16 EBML_TRACK_ATTACHMENT_LINK_ID    = 0x7446;
const uint16 EBML_TRACK_NAME_ID               = 0x536E;
const uint32 EBML_DEF_DURATION_ID             = 0x23E383;
const uint32 EBML_TRACK_TIMECODESCALE_ID      = 0x23314F;
const uint32 EBML_TRACK_CODEC_NAME_ID         = 0x258688;
const uint32 EBML_TRACK_LANGUAGE_ID           = 0x22B59C;
const uint16 EBML_TRACK_CODEC_DELAY           = 0x56AA;
const uint16 EBML_TRACK_SEEK_PREROLL          = 0x56BB;

const uint8  EBML_VIDEO_FLAG_INTERLACED       = 0x9A;
const uint16 EBML_VIDEO_STEREO_MODE           = 0x53B8;
const uint8  EBML_VIDEO_PIXEL_WIDTH           = 0xB0;
const uint8  EBML_VIDEO_PIXEL_HEIGHT          = 0xBA;
const uint16 EBML_VIDEO_PIXEL_CROP_BOTTOM     = 0x54AA;
const uint16 EBML_VIDEO_PIXEL_CROP_TOP        = 0x54BB;
const uint16 EBML_VIDEO_PIXEL_CROP_LEFT       = 0x54CC;
const uint16 EBML_VIDEO_PIXEL_CROP_RIGHT      = 0x54DD;
const uint16 EBML_VIDEO_DISPLAY_WIDTH         = 0x54B0;
const uint16 EBML_VIDEO_DISPLAY_HEIGHT        = 0x54BA;
const uint16 EBML_VIDEO_DISPLAY_UNIT          = 0x54B2;
const uint16 EBML_VIDEO_ASPECTRATIO_TYPE      = 0x54B3;
const uint32 EBML_VIDEO_COLOUR_SPACE          = 0x2EB524;
const uint32 EBML_VIDEO_GAMMA_VALUE           = 0x2FB523;
const uint32 EBML_VIDEO_FRAME_RATE            = 0x2383E3;

const uint8  EBML_AUDIO_SAMPLING_FREQ         = 0xB5;
const uint16 EBML_AUDIO_OUT_SAMPL_FREQ        = 0x78B5;
const uint8  EBML_AUDIO_NO_CHANNELS           = 0x9F;
const uint16 EBML_AUDIO_CHANNLES_POSN         = 0x7D7B;
const uint16 EBML_AUDIO_SAMPLE_BITDEPTH       = 0x6264;

const uint32 EBML_CLUSTER_ID                  = 0x1F43B675;
const uint8  EBML_CLUSTER_TIMECODE_ID         = 0xE7;
const uint8  EBML_CLUSTER_POSN_ID             = 0xA7;
const uint8  EBML_CLUSTER_PRVSIZE_ID          = 0xAB;
const uint8  EBML_CLUSTER_BLOCKGROUP_ID       = 0xA0;
const uint8  EBML_CLUSTER_SIMPLE_BLOCK_ID     = 0xA3;
const uint8  EBML_CLUSTER_BLOCK_ID            = 0xA1;
const uint8  EBML_CLUSTER_REFERENCE_BLOCK_ID  = 0xFB;
const uint8  EBML_CLUSTER_BLOCK_DUR_ID        = 0x9B;

const uint32 EBML_CUES_ID                     = 0x1C53BB6B;
const uint8  EBML_CUE_POINT                   = 0xBB;
const uint8  EBML_CUE_TIME                    = 0xB3;
const uint8  EBML_CUE_TRACK_POSITIONS         = 0xB7;
const uint8  EBML_CUE_TRACK                   = 0xF7;
const uint8  EBML_CUE_CLUSTER_POSN            = 0xF1;
const uint16 EBML_CUE_BLOCK_NO                = 0x5378;
const uint8  EBML_CUE_CODEC_STATE             = 0xEA;
const uint8  EBML_CUE_REFERENCE               = 0xDB;
const uint8  EBML_CUE_REF_TIME                = 0x96;
const uint8  EBML_CUE_REF_CLUSTER             = 0x97;
const uint8  EBML_CUE_REF_NUMBER              = 0x5F;
const uint8  EBML_CUE_REF_CODEC_STATE         = 0xEB;

const uint32 EBML_ATTACHMENTS_ID              = 0x1941A469;
const uint32 EBML_CHAPTERS_ID                 = 0x1043A770;
const uint32 EBML_TAGS_ID                     = 0x1254C367;

const uint32 EBML_SEGFAMILY_ID                = 0x4444;
const uint32 EBML_CHAPT_TRANSLATE_ID          = 0x6924;
const uint32 EBML_CHAPT_TRAN_EDTN_UID_ID      = 0x69FC;
const uint32 EBML_CHAPT_TRANSLATE_CODEC_ID    = 0x69BF;
const uint32 EBML_CHAPT_TRANSLATEID_ID        = 0x69A5;

const uint8 DEF_EBML_VER_VAL                  = 1;
const uint8 DEF_EBML_READ_VER_VAL             = 1;
const uint8 DEF_EBML_MX_ID_LENGTH_VAL         = 4;
const uint8 DEF_EBML_MX_SZ_LENGTH_VAL         = 8;
const uint8 DEF_EBML_MAX_SIGNATURE_LENGTH_VAL = 12;
const uint8 DEF_EBML_DOCTYPE_VAL1[]           = "matroska";
const uint8 DEF_EBML_DOCTYPE_VAL2[]           = "webm";
const uint8 DEF_EBML_DOCTYPE_VER_VAL          = 1;
const uint8 DEF_EBML_DOCTYPE_READ_VER_VAL     = 1;

const uint16 EBML_TRACK_CONTENTS_ENCODING         = 0x6240;
const uint16 EBML_TRACK_CONTENTS_ENCODING_ORDER   = 0x5031;
const uint16 EBML_TRACK_CONTENTS_ENCODING_SCOPE   = 0x5032;
const uint16 EBML_TRACK_CONTENTS_ENCODING_TYPE    = 0x5033;
const uint16 EBML_TRACK_CONTENTS_COMPRESSION_TYPE = 0x5034;
const uint16 EBML_TRACK_CONTENTS_ENCRYPT_TYPE     = 0x5035;
const uint16 EBML_TRACK_CONTENTS_COMPRESSION_ALGO = 0x4254;
const uint16 EBML_TRACK_CONTENTS_COMPRESSION_SET  = 0x4255;

const uint16 EBML_TAGS_TAG_ID                     = 0x7373;
const uint16 EBML_TAGS_TARGET_ID                  = 0x63C0;
const uint16 EBML_TAGS_TARGET_TYPE_ID             = 0x63CA;
const uint16 EBML_TAGS_TARGET_TYPE_VAL_ID         = 0x68CA;
const uint16 EBML_TAGS_TRACK_UID_ID               = 0x63C5;
const uint16 EBML_TAGS_EDITION_UID_ID             = 0x63C9;
const uint16 EBML_TAGS_CHAPTER_UID_ID             = 0x63C4;
const uint16 EBML_TAGS_ATTACHMENT_UID_ID          = 0x63C6;
const uint16 EBML_TAGS_SMPLE_TAG_ID               = 0x67C8;
const uint16 EBML_TAGS_TAG_NAME_ID                = 0x45A3;
const uint16 EBML_TAGS_TAG_LANG_ID                = 0x447A;
const uint16 EBML_TAGS_TAG_DEFAULT_ID             = 0x4484;
const uint16 EBML_TAGS_TAG_STRING_ID              = 0x4487;
const uint16 EBML_TAGS_TAG_BINARY_ID              = 0x4485;

/* METADATAINDEX is used to map between FileSource enum types and
   Metadata strings in MKAV file format.
*/
typedef enum _META_DATA_INDEX_
{
  TAG_TITLE,
  TAG_ARTIST,
  TAG_ARTIST1,
  TAG_COMPOSER,
  TAG_GENRE,
  TAG_AUTHOR,
  TAG_ALBUM,
  TAG_REC_DATE,
  TAG_REL_DATE,
  MAX_INDEX
} METADATAINDEX;
/* ucMETADATASTR: Tag Names used in MKAV file format.
   Update "METADATAINDEX" ENUM if new Metadata fields are added in below list.
*/
const uint8 ucMETADATASTR[MAX_INDEX][METADATA_STRING_LENGTH] =
{
  "TITLE",
  "ARTIST",
  "LEAD_PERFORMER",
  "COMPOSER",
  "GENRE",
  "AUTHOR",
  "ALBUM",
  "DATE_RECORDED",
  "DATE_RELEASED"
};

/* This table will be used to find out number of bytes to store ID/Size fields.
   1st --> This is the value available in input file
   2nd --> This indicates number of bytes required to store size/Id data.
   3rd --> This gives the Mask data to calculate actual Size.
   For eg, If size is stored as 0x 40 3F
   first byte 0x40 means, it requires two bytes to store size.
   Bit mask to calculate actual size is 0xBF.
*/
const uint8 EBML_LENGTH_TBLE[EBML_SIZE_TBL_MX_ROWS][EBML_SIZE_TBL_MX_COLUMNS]=
{
  {0x0, 0,0x00},  {0x1, 8,0xfe},  {0x2, 7,0xfd},  {0x3, 7,0xfd},
  {0x4, 6,0xfb},  {0x5, 6,0xfb},  {0x6, 6,0xfb},  {0x7, 6,0xfb},
  {0x8, 5,0xf7},  {0x9, 5,0xf7},  {0xa, 5,0xf7},  {0xb, 5,0xf7},
  {0xc, 5,0xf7},  {0xd, 5,0xf7},  {0xe, 5,0xf7},  {0xf, 5,0xf7},
  {0x10,4,0xef},  {0x11,4,0xef},  {0x12,4,0xef},  {0x13,4,0xef},
  {0x14,4,0xef},  {0x15,4,0xef},  {0x16,4,0xef},  {0x17,4,0xef},
  {0x18,4,0xef},  {0x19,4,0xef},  {0x1a,4,0xef},  {0x1b,4,0xef},
  {0x1c,4,0xef},  {0x1d,4,0xef},  {0x1e,4,0xef},  {0x1f,4,0xef},
  {0x20,3,0xdf},  {0x21,3,0xdf},  {0x22,3,0xdf},  {0x23,3,0xdf},
  {0x24,3,0xdf},  {0x25,3,0xdf},  {0x26,3,0xdf},  {0x27,3,0xdf},
  {0x28,3,0xdf},  {0x29,3,0xdf},  {0x2a,3,0xdf},  {0x2b,3,0xdf},
  {0x2c,3,0xdf},  {0x2d,3,0xdf},  {0x2e,3,0xdf},  {0x2f,3,0xdf},
  {0x30,3,0xdf},  {0x31,3,0xdf},  {0x32,3,0xdf},  {0x33,3,0xdf},
  {0x34,3,0xdf},  {0x35,3,0xdf},  {0x36,3,0xdf},  {0x37,3,0xdf},
  {0x38,3,0xdf},  {0x39,3,0xdf},  {0x3a,3,0xdf},  {0x3b,3,0xdf},
  {0x3c,3,0xdf},  {0x3d,3,0xdf},  {0x3e,3,0xdf},  {0x3f,3,0xdf},
  {0x40,2,0xbf},  {0x41,2,0xbf},  {0x42,2,0xbf},  {0x43,2,0xbf},
  {0x44,2,0xbf},  {0x45,2,0xbf},  {0x46,2,0xbf},  {0x47,2,0xbf},
  {0x48,2,0xbf},  {0x49,2,0xbf},  {0x4a,2,0xbf},  {0x4b,2,0xbf},
  {0x4c,2,0xbf},  {0x4d,2,0xbf},  {0x4e,2,0xbf},  {0x4f,2,0xbf},
  {0x50,2,0xbf},  {0x51,2,0xbf},  {0x52,2,0xbf},  {0x53,2,0xbf},
  {0x54,2,0xbf},  {0x55,2,0xbf},  {0x56,2,0xbf},  {0x57,2,0xbf},
  {0x58,2,0xbf},  {0x59,2,0xbf},  {0x5a,2,0xbf},  {0x5b,2,0xbf},
  {0x5c,2,0xbf},  {0x5d,2,0xbf},  {0x5e,2,0xbf},  {0x5f,2,0xbf},
  {0x60,2,0xbf},  {0x61,2,0xbf},  {0x62,2,0xbf},  {0x63,2,0xbf},
  {0x64,2,0xbf},  {0x65,2,0xbf},  {0x66,2,0xbf},  {0x67,2,0xbf},
  {0x68,2,0xbf},  {0x69,2,0xbf},  {0x6a,2,0xbf},  {0x6b,2,0xbf},
  {0x6c,2,0xbf},  {0x6d,2,0xbf},  {0x6e,2,0xbf},  {0x6f,2,0xbf},
  {0x70,2,0xbf},  {0x71,2,0xbf},  {0x72,2,0xbf},  {0x73,2,0xbf},
  {0x74,2,0xbf},  {0x75,2,0xbf},  {0x76,2,0xbf},  {0x77,2,0xbf},
  {0x78,2,0xbf},  {0x79,2,0xbf},  {0x7a,2,0xbf},  {0x7b,2,0xbf},
  {0x7c,2,0xbf},  {0x7d,2,0xbf},  {0x7e,2,0xbf},  {0x7f,2,0xbf},
  {0x80,1,0x7f},  {0x81,1,0x7f},  {0x82,1,0x7f},  {0x83,1,0x7f},
  {0x84,1,0x7f},  {0x85,1,0x7f},  {0x86,1,0x7f},  {0x87,1,0x7f},
  {0x88,1,0x7f},  {0x89,1,0x7f},  {0x8a,1,0x7f},  {0x8b,1,0x7f},
  {0x8c,1,0x7f},  {0x8d,1,0x7f},  {0x8e,1,0x7f},  {0x8f,1,0x7f},
  {0x90,1,0x7f},  {0x91,1,0x7f},  {0x92,1,0x7f},  {0x93,1,0x7f},
  {0x94,1,0x7f},  {0x95,1,0x7f},  {0x96,1,0x7f},  {0x97,1,0x7f},
  {0x98,1,0x7f},  {0x99,1,0x7f},  {0x9a,1,0x7f},  {0x9b,1,0x7f},
  {0x9c,1,0x7f},  {0x9d,1,0x7f},  {0x9e,1,0x7f},  {0x9f,1,0x7f},
  {0xa0,1,0x7f},  {0xa1,1,0x7f},  {0xa2,1,0x7f},  {0xa3,1,0x7f},
  {0xa4,1,0x7f},  {0xa5,1,0x7f},  {0xa6,1,0x7f},  {0xa7,1,0x7f},
  {0xa8,1,0x7f},  {0xa9,1,0x7f},  {0xaa,1,0x7f},  {0xab,1,0x7f},
  {0xac,1,0x7f},  {0xad,1,0x7f},  {0xae,1,0x7f},  {0xaf,1,0x7f},
  {0xb0,1,0x7f},  {0xb1,1,0x7f},  {0xb2,1,0x7f},  {0xb3,1,0x7f},
  {0xb4,1,0x7f},  {0xb5,1,0x7f},  {0xb6,1,0x7f},  {0xb7,1,0x7f},
  {0xb8,1,0x7f},  {0xb9,1,0x7f},  {0xba,1,0x7f},  {0xbb,1,0x7f},
  {0xbc,1,0x7f},  {0xbd,1,0x7f},  {0xbe,1,0x7f},  {0xbf,1,0x7f},
  {0xc0,1,0x7f},  {0xc1,1,0x7f},  {0xc2,1,0x7f},  {0xc3,1,0x7f},
  {0xc4,1,0x7f},  {0xc5,1,0x7f},  {0xc6,1,0x7f},  {0xc7,1,0x7f},
  {0xc8,1,0x7f},  {0xc9,1,0x7f},  {0xca,1,0x7f},  {0xcb,1,0x7f},
  {0xcc,1,0x7f},  {0xcd,1,0x7f},  {0xce,1,0x7f},  {0xcf,1,0x7f},
  {0xd0,1,0x7f},  {0xd1,1,0x7f},  {0xd2,1,0x7f},  {0xd3,1,0x7f},
  {0xd4,1,0x7f},  {0xd5,1,0x7f},  {0xd6,1,0x7f},  {0xd7,1,0x7f},
  {0xd8,1,0x7f},  {0xd9,1,0x7f},  {0xda,1,0x7f},  {0xdb,1,0x7f},
  {0xdc,1,0x7f},  {0xdd,1,0x7f},  {0xde,1,0x7f},  {0xdf,1,0x7f},
  {0xe0,1,0x7f},  {0xe1,1,0x7f},  {0xe2,1,0x7f},  {0xe3,1,0x7f},
  {0xe4,1,0x7f},  {0xe5,1,0x7f},  {0xe6,1,0x7f},  {0xe7,1,0x7f},
  {0xe8,1,0x7f},  {0xe9,1,0x7f},  {0xea,1,0x7f},  {0xeb,1,0x7f},
  {0xec,1,0x7f},  {0xed,1,0x7f},  {0xee,1,0x7f},  {0xef,1,0x7f},
  {0xf0,1,0x7f},  {0xf1,1,0x7f},  {0xf2,1,0x7f},  {0xf3,1,0x7f},
  {0xf4,1,0x7f},  {0xf5,1,0x7f},  {0xf6,1,0x7f},  {0xf7,1,0x7f},
  {0xf8,1,0x7f},  {0xf9,1,0x7f},  {0xfa,1,0x7f},  {0xfb,1,0x7f},
  {0xfc,1,0x7f},  {0xfd,1,0x7f},  {0xfe,1,0x7f},  {0xff,1,0x7f}
};

#endif//#ifndef MKAV_PARSER_CONSTANTS_DEFN

