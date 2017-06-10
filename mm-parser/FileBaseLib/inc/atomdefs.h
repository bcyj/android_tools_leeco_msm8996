#ifndef __AtomDefs_H__
#define __AtomDefs_H__
/* =======================================================================
                              atomdefs.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2009-2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/atomdefs.h#32 $
$DateTime: 2014/05/13 22:50:58 $
$Change: 5885284 $


========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "oscl_file_io.h"

//static int32 STREAM_TYPE_AUDIO = 0;
//static int32 STREAM_TYPE_VISUAL = 1;
//static int32 STREAM_TYPE_TEXT = 2;
//static int32 STREAM_TYPE_UNKNOWN = 3;

//static bool READ_OLD_DESCRIPTOR;

#define MEDIA_DATA_IN_MEMORY   0
#define MEDIA_DATA_ON_DISK   1

#define DEFAULT_PRESENTATION_TIMESCALE   1000 // For milliseconds
#define INITIAL_TRACK_ID   1 // Initial track ID for first track added to this movie

#define MEDIA_TYPE_UNKNOWN   10

// MediaInformationHeaderAtom types
#define MEDIA_INFORMATION_HEADER_TYPE_AUDIO   0
#define MEDIA_INFORMATION_HEADER_TYPE_VISUAL   1
#define MEDIA_INFORMATION_HEADER_TYPE_HINT   2
#define MEDIA_INFORMATION_HEADER_TYPE_MPEG4   3

#define UNKNOWN_ATOM   0
#define UNKNOWN_DESCRIPTOR   1
#define UNKNOWN_HANDLER   2

#define CODING_TYPE_I   0
#define CODING_TYPE_P   1
#define CODING_TYPE_B   2
#define CODING_TYPE_SPRITE   3

// Mpeg-4 file types
#define FILE_TYPE_AUDIO   1
#define FILE_TYPE_VIDEO   2
#define FILE_TYPE_AUDIO_VIDEO   3 // Logical ORing of the two
#define FILE_TYPE_STILL_IMAGE   4
#define FILE_TYPE_STILL_IMAGE_AUDIO   5 // Logical ORing of the two
#define FILE_TYPE_TEXT   8
#define FILE_TYPE_TEXT_AUDIO   9
#define FILE_TYPE_TEXT_VIDEO   10
#define FILE_TYPE_TEXT_AUDIO_VIDEO   11 // Logical ORing

// Scalability settings on the Mpeg4 file
#define STREAM_SCALABILITY_NONE   0
#define STREAM_SCALABILITY_TEMPORAL   1
#define STREAM_SCALABILITY_SPATIAL   2
#define STREAM_SCALABILITY_BOTH   3 // Logical ORing of the two

// The following are the allowable protocols with respect to the hint tracks:
// HINT_PROTOCOL_TEMPORAL_SCALABILITY - Video stream encoded with PacketVideo Temporal Scalability
// HINT_PROTOCOL_SPATIAL_SCALABILITY - Video stream encoded with PacketVideo Spatial Scalability
// 'pvst' - Video stream encoded with both PacketVideo Spatial and Temporal scalability
// HINT_PROTOCOL_BASE_LAYER_ONLY - Video stream encoded with Base Layer only
// *** Note that only HINT_PROTOCOL_TEMPORAL_SCALABILITY and HINT_PROTOCOL_BASE_LAYER_ONLY video protocols are supported in version 1.0 ***
// 'pvau' - Audio encoding with simple hint track


#define FourCharConstToUint32(a, b, c, d) ( (uint32(a) << 24) | (uint32(b) << 16) | (uint32(c) << 8) | uint32(d) )

#define PACKETVIDEO_FOURCC                 FourCharConstToUint32('p', 'v', 'm', 'm')
#define PVUSER_DATA_ATOM                   FourCharConstToUint32('p', 'v', 'm', 'm')

#define RANDOM_ACCESS_ATOM                 FourCharConstToUint32('r', 'a', 'n', 'd')
#define CONTENT_VERSION_ATOM               FourCharConstToUint32('c', 'v', 'e', 'r')
#define VIDEO_INFO_ATOM                    FourCharConstToUint32('v', 'i', 'n', 'f')
#define WMF_SET_MEDIA                      FourCharConstToUint32('w', 'm', 'f', 'm')
#define WMF_SET_SESSION                    FourCharConstToUint32('w', 'm', 'f', 's')
#define FILE_TYPE_ATOM                     FourCharConstToUint32('f', 't', 'y', 'p')
#define HINT_INFORMATION                   FourCharConstToUint32('h', 'n', 't', 'i')
#define TRACK_INFO_ATOM                    FourCharConstToUint32('p', 'v', 't', 'i')

#define TRACK_INFO_SESSION_ATOM            FourCharConstToUint32('p', 'v', 's', 'i')
#define TRACK_INFO_MEDIA_ATOM              FourCharConstToUint32('p', 'v', 'm', 'i')

#define SYNC_INFO_ATOM                     FourCharConstToUint32('p', 'v', 's', 'y')

#define REQUIREMENTS_ATOM                  FourCharConstToUint32('r', 'q', 'm', 't')
#define DOWNLOAD_ATOM                      FourCharConstToUint32('d', 'n', 'l', 'd')

#define UUID_ATOM                          FourCharConstToUint32('u', 'u', 'i', 'd')

#define MOVIE_ATOM                         FourCharConstToUint32('m', 'o', 'o', 'v')
#define MOVIE_HEADER_ATOM                  FourCharConstToUint32('m', 'v', 'h', 'd')
#define TRACK_ATOM                         FourCharConstToUint32('t', 'r', 'a', 'k')
#define ESD_ATOM                           FourCharConstToUint32('e', 's', 'd', 's')
#define TRACK_HEADER_ATOM                  FourCharConstToUint32('t', 'k', 'h', 'd')
#define TRACK_REFERENCE_ATOM               FourCharConstToUint32('t', 'r', 'e', 'f')

#define HINT_TRACK_REFERENCE_TYPE          FourCharConstToUint32('h', 'i', 'n', 't')
#define DPND_TRACK_REFERENCE_TYPE          FourCharConstToUint32('d', 'p', 'n', 'd')
#define IPIR_TRACK_REFERENCE_TYPE          FourCharConstToUint32('i', 'p', 'i', 'r')
#define MPOD_TRACK_REFERENCE_TYPE          FourCharConstToUint32('m', 'p', 'o', 'd')
#define SYNC_TRACK_REFERENCE_TYPE          FourCharConstToUint32('s', 'y', 'n', 'c')

#define MEDIA_ATOM                         FourCharConstToUint32('m', 'd', 'i', 'a')
#define EDIT_ATOM                          FourCharConstToUint32('e', 'd', 't', 's')
#define EDIT_LIST_ATOM                     FourCharConstToUint32('e', 'l', 's', 't')
#define MEDIA_HEADER_ATOM                  FourCharConstToUint32('m', 'd', 'h', 'd')
#define HANDLER_ATOM                       FourCharConstToUint32('h', 'd', 'l', 'r')
#define MEDIA_INFORMATION_ATOM             FourCharConstToUint32('m', 'i', 'n', 'f')
#define VIDEO_MEDIA_HEADER_ATOM            FourCharConstToUint32('v', 'm', 'h', 'd')
#define SOUND_MEDIA_HEADER_ATOM            FourCharConstToUint32('s', 'm', 'h', 'd')
#define HINT_MEDIA_HEADER_ATOM             FourCharConstToUint32('h', 'm', 'h', 'd')
#define MPEG4_MEDIA_HEADER_ATOM            FourCharConstToUint32('n', 'm', 'h', 'd')
#define DATA_INFORMATION_ATOM              FourCharConstToUint32('d', 'i', 'n', 'f')
#define DATA_REFERENCE_ATOM                FourCharConstToUint32('d', 'r', 'e', 'f')
#define DATA_ENTRY_URL_ATOM                FourCharConstToUint32('u', 'r', 'l', ' ')
#define DATA_ENTRY_URN_ATOM                FourCharConstToUint32('u', 'r', 'n', ' ')
#define SAMPLE_TABLE_ATOM                  FourCharConstToUint32('s', 't', 'b', 'l')
#define TIME_TO_SAMPLE_ATOM                FourCharConstToUint32('s', 't', 't', 's')
#define COMPOSITION_OFFSET_ATOM            FourCharConstToUint32('c', 't', 't', 's')
#define SAMPLE_DESCRIPTION_ATOM            FourCharConstToUint32('s', 't', 's', 'd')
#define SAMPLE_SIZE_ATOM                   FourCharConstToUint32('s', 't', 's', 'z')
#define SAMPLE_TO_CHUNK_ATOM               FourCharConstToUint32('s', 't', 's', 'c')
#define CHUNK_OFFSET_ATOM                  FourCharConstToUint32('s', 't', 'c', 'o')
#define SYNC_SAMPLE_ATOM                   FourCharConstToUint32('s', 't', 's', 's')
#define SHADOW_SYNC_SAMPLE_ATOM            FourCharConstToUint32('s', 't', 's', 'h')
#define DEGRADATION_PRIORITY_ATOM          FourCharConstToUint32('s', 't', 'd', 'p')
#define OBJECT_DESCRIPTOR_ATOM             FourCharConstToUint32('i', 'o', 'd', 's')
#define MEDIA_DATA_ATOM                    FourCharConstToUint32('m', 'd', 'a', 't')
#define FREE_SPACE_ATOM                    FourCharConstToUint32('f', 'r', 'e', 'e')
#define SKIP_ATOM                          FourCharConstToUint32('s', 'k', 'i', 'p')
#define USER_DATA_ATOM                     FourCharConstToUint32('u', 'd', 't', 'a')
#define MEDIA_TYPE_AUDIO                   FourCharConstToUint32('s', 'o', 'u', 'n')
#define MEDIA_TYPE_VISUAL                  FourCharConstToUint32('v', 'i', 'd', 'e')
#define MEDIA_TYPE_HINT                    FourCharConstToUint32('h', 'i', 'n', 't')
#define MEDIA_TYPE_OBJECT_DESCRIPTOR       FourCharConstToUint32('o', 'd', 's', 'm')
#define MEDIA_TYPE_CLOCK_REFERENCE         FourCharConstToUint32('c', 'r', 's', 'm')
#define MEDIA_TYPE_SCENE_DESCRIPTION       FourCharConstToUint32('s', 'd', 's', 'm')
#define MEDIA_TYPE_MPEG7                   FourCharConstToUint32('m', '7', 's', 'm')
#define MEDIA_TYPE_OBJECT_CONTENT_INFO     FourCharConstToUint32('o', 'c', 's', 'm')
#define MEDIA_TYPE_IPMP                    FourCharConstToUint32('i', 'p', 's', 'm')
#define MEDIA_TYPE_MPEG_J                  FourCharConstToUint32('m', 'j', 's', 'm')
#define MEDIA_TYPE_SCALABILITY             FourCharConstToUint32('p', 'v', 's', 'c')
#define MEDIA_TYPE_TEXT                    FourCharConstToUint32('t', 'e', 'x', 't')

#define MPEG_SAMPLE_ENTRY                  FourCharConstToUint32('m', 'p', '4', 's')
#define AUDIO_SAMPLE_ENTRY                 FourCharConstToUint32('m', 'p', '4', 'a')
#define VIDEO_SAMPLE_ENTRY                 FourCharConstToUint32('m', 'p', '4', 'v')

#define AMR_SAMPLE_ENTRY_ATOM              FourCharConstToUint32('s', 'a', 'm', 'r')
#define AMRWB_SAMPLE_ENTRY_ATOM            FourCharConstToUint32('s', 'a', 'w', 'b')
#define H263_SAMPLE_ENTRY_ATOM             FourCharConstToUint32('s', '2', '6', '3')

#define AMR_SPECIFIC_ATOM                  FourCharConstToUint32('d', 'a', 'm', 'r')
#define H263_SPECIFIC_ATOM                 FourCharConstToUint32('d', '2', '6', '3')

#define COPYRIGHT_ATOM                     FourCharConstToUint32('c', 'p', 'r', 't')

#define NULL_MEDIA_HEADER_ATOM             FourCharConstToUint32('n', 'm', 'h', 'd')
#define FONT_TABLE_ATOM                    FourCharConstToUint32('f', 't', 'a', 'b')
#define TEXT_SAMPLE_ENTRY                  FourCharConstToUint32('t', 'x', '3', 'g')

#define PV_ENTITY_TAG_ATOM                 FourCharConstToUint32('e', 't', 'a', 'g')

#define WMF_BRAND                          FourCharConstToUint32('w', 'm', 'f', ' ')
#define BRAND_3GPP                         FourCharConstToUint32('3', 'g', 'p', '4')
#define BRAND_MPEG4                        FourCharConstToUint32('m', 'p', '4', '1')

// KDDI UUID Atoms
  #define KDDI_DRM_ATOM                     FourCharConstToUint32('c', 'p', 'g', 'd')
  #define KDDI_CONTENT_PROPERTY_ATOM        FourCharConstToUint32('p', 'r', 'o', 'p')
  #define KDDI_MOVIE_MAIL_ATOM              FourCharConstToUint32('m', 'v', 'm', 'l')
  #define KDDI_ENCODER_INFO_ATOM            FourCharConstToUint32('e', 'n', 'c', 'i')
  #define KDDI_GPS_ATOM                     FourCharConstToUint32('g', 'p', 's', 'i')
  #define KDDI_GPS_EXTENSION_ATOM           FourCharConstToUint32('g', 'p', 'e', 'x')
  #define KDDI_TELOP_ATOM                   FourCharConstToUint32('t', 's', 'm', 'l')

// Text Sample Modifier Atoms
  #define TEXT_STYLE_BOX                    FourCharConstToUint32('s', 't', 'y', 'l')
  #define TEXT_HIGHLIGHT_BOX                FourCharConstToUint32('h', 'l', 'i', 't')
  #define TEXT_HILIGHT_COLOR_BOX            FourCharConstToUint32('h', 'c', 'l', 'r')
  #define TEXT_KARAOKE_BOX                  FourCharConstToUint32('k', 'r', 'o', 'k')
  #define TEXT_SCROLL_DELAY_BOX             FourCharConstToUint32('d', 'l', 'a', 'y')
  #define TEXT_HYPER_TEXT_BOX               FourCharConstToUint32('h', 'r', 'e', 'f')
  #define TEXT_OVER_RIDE_BOX                FourCharConstToUint32('t', 'b', 'o', 'x')
  #define TEXT_BLINK_BOX                    FourCharConstToUint32('b', 'l', 'n', 'k')

  #define DCMD_DRM_ATOM                     FourCharConstToUint32('d', 'c', 'm', 'd')
  #define EDIT_ATOM                         FourCharConstToUint32('e', 'd', 't', 's')
  #define EDIT_LIST_ATOM                    FourCharConstToUint32('e', 'l', 's', 't')

// Allowable values of the hint type byte
#define HINT_SAMPLE_AUDIO   0
#define HINT_SAMPLE_VIDEO   1
#define HINT_SAMPLE_STILL_IMAGE   2
// MORE TBA - add specific protocols into type value

// VARIOUS DESCRIPTOR TAGS CURRENTLY IN USE

#define ES_DESCRIPTOR_TAG             0x03
#define ES_ID_INC_TAG                 0x0E
#define ES_ID_REF_TAG                 0x0F
#define DECODER_CONFIG_DESCRIPTOR_TAG 0x04
#define DECODER_SPECIFIC_INFO_TAG     0x05
#define SL_CONFIG_DESCRIPTOR          0x06

#define STREAM_TYPE_AUDIO      0x05
#define STREAM_TYPE_VISUAL     0x04

#define IMPERIAL_CONTENT_VERSION 0x03

#define IMPERIAL_PLUS_CONTENT_VERSION 0x04

typedef enum
{
  CODEC_TYPE_AMR_AUDIO = 1,
  CODEC_TYPE_EVRC_AUDIO,
  CODEC_TYPE_AAC_AUDIO

} AUDIO_CODEC_TYPES;

typedef enum
{
  CODEC_TYPE_MPEG4_VIDEO = 1,
  CODEC_TYPE_BASELINE_H263_VIDEO,
  CODEC_TYPE_MPEG4_IMAGE

} VIDEO_CODEC_TYPES;

typedef enum
{
  EVRC_AUDIO         = 0xd1,
  PUREVOICE_AUDIO    = 0xd2,
  AMR_AUDIO          = 0xd3,
  AMR_WB_AUDIO       = 0xd4,
  AMR_WB_PLUS_AUDIO  = 0xd5,
  EVRC_NB_AUDIO      = 0xd6,
  EVRC_WB_AUDIO      = 0xd7,
  EVRC_B_AUDIO       = 0xd8,
  PUREVOICE_AUDIO_2  = 0xE1,
  MPEG4_AUDIO        = 0x40,
  MPEG4_AUDIO_BSAC   = 0x65,
  MPEG2_AAC_LC       = 0x67,
  MP3_AUDIO          = 0x69,
  NONMP4_MP3_AUDIO   = 0x70,
  MIDI_AUDIO         = 0x71,
  QCP_AUDIO          = 0x72,
  VND_QCELP_AUDIO    = 0x73,
  QCF_AUDIO          = 0x74,
  MMF_AUDIO          = 0x75,
  PHR_AUDIO          = 0x76,
  IMELODY_AUDIO      = 0x77,
  ADPCM_AUDIO        = 0x78,
  NONMP4_AAC_AUDIO   = 0x79,
  NONMP4_AMR_AUDIO   = 0x80,
  WMA_AUDIO          = 0x81,
  HVS_AUDIO          = 0x82,
  SAF_AUDIO          = 0x83,
  XMF_AUDIO          = 0x84,
  DLS_AUDIO          = 0x85,
  QCP_QLCM_AUDIO     = 0x86,
  WM_PRO_AUDIO       = 0x87,
  WM_LOSSLESS        = 0x88,
  MPEG4_VIDEO        = 0x20,
  H263_VIDEO         = 0xc0,
  AC3_AUDIO          = 0xA4,
  EAC3_AUDIO         = 0xA5,
  EAC3_JOC_AUDIO     = 0xA6,
  DTS_AUDIO          = 0xA7,

  H264_VIDEO         = 0xc1,   /* temp value, update when correct value is known */

  WM_VIDEO_7         = 0xc2,   /* temp value, update when correct value is known */
  WM_VIDEO_8         = 0xc3,   /* temp value, update when correct value is known */
  WM_VIDEO_9         = 0xc4,   /* temp value, update when correct value is known */
  VC1_VIDEO          = 0x07,

  WM_AUDIO           = 0xc5,   /* temp value, update when correct value is known */
  WM_SPEECH          = 0xE3,
  PCM_AUDIO          = 0xc6,   /* temp value, update when correct value is known */

  VP6F_VIDEO         = 0xc7,   /* temp value, update when correct value is known */

  TIMED_TEXT         = 0xFD,

  JPEG_VIDEO         = 0x6C,
  BMP_VIDEO          = 0xE2,
  SKT_MOD_TEXT       = 0xD0,

  MPEG4_IMAGE        = 0xFE,
  MPEG4_SYS_OD       = 0x01,
  MPEG4_SYS_BIFS     = 0x01,
  DIVX311_VIDEO      = 0x89,
  DIVX40_VIDEO       = 0x90,
  DIVX50_60_VIDEO    = 0x98,
  MPEG2_VIDEO        = 0x99,
  NONSTD_MPEG4_VIDEO = 0x91,
  G711_ALAW_AUDIO    = 0x37,
  G711_MULAW_AUDIO   = 0x38,
  G721_AUDIO         = 0x39,
  G723_AUDIO         = 0x3A,
  GSM_FR_AUDIO       = 0x3B,
  OSCAR_VIDEO        = 0x02,   /* temp value, update when correct value is known */
  VORBIS_AUDIO       = 0xc8,
  THEORA_VIDEO       = 0xc9,
  SPARK_VIDEO        = 0xca,   /* temp value, update when correct value is known */
  VP8F_VIDEO         = 0xcb,
  VP9_VIDEO          = 0xcc,
  MP1_AUDIO          = 0xcd,
  MP2_AUDIO          = 0xce,
  FLAC_AUDIO         = 0xd9,
  OPUS_AUDIO         = 0xda,   /* temp value, update when correct value is known */
  AAC_ADTS_AUDIO     = 0xe4,
  AAC_ADIF_AUDIO     = 0xe5,
  AAC_LOAS_AUDIO     = 0xe6,
  HEVC_VIDEO         = 0xe7,  /* HEVC video format */
  REAL_AUDIO         = 0xe8,
  REAL_VIDEO         = 0xe9,
  MJPEG_VIDEO        = 0xea,
  AVI_BITMAP_TEXT    = 0xeb,
  AVI_SIMPLE_TEXT    = 0xec,
  SMPTE_TIMED_TEXT   = 0xed,
  BITMAP_TEXT        = 0xee,
  SSA_TEXT           = 0xef,
  ASS_TEXT           = 0xf0,
  USF_TEXT           = 0xf1,
  UTF8_TEXT          = 0xf2,
  VOBSUB_TEXT        = 0xf3,
  KARAOKE_TEXT       = 0xf4,
  MPEG1_VIDEO        = 0xf5
} OTI_VALUES;

#endif /* __AtomDefs_H__ */

