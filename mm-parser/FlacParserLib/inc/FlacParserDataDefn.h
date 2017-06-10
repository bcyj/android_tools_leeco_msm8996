#ifndef _FLAC_PARSER_DATA_DEFN
#define _FLAC_PARSER_DATA_DEFN

/* =======================================================================
                              FlacParserDataDefn.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2009-2014 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FlacParserLib/main/latest/inc/FlacParserDataDefn.h#8 $
========================================================================== */

#include "FlacParserConstants.h"
#include "filesourcetypes.h"
/*
*******************************************************************
* Data types
*******************************************************************
*/

typedef struct _flac_stream_sample_info_
{
  uint64 noffset;
  uint64 ntime;
  uint64 duration;
}flac_stream_sample_info;

typedef struct flac_metadata_streaminfo_t
{
  uint64              nTotalSamplesInStream;
  uint8               MD5Signature[MD5_SIG_SIZE];
  uint32              nMinFrameSize;
  uint32              nMaxFrameSize;
  uint32              nSamplingRate;
  uint32              nFixedBlockSize;
  uint16              nMinBlockSize;
  uint16              nMaxBlockSize;
  uint8               nChannels;
  uint8               nBitsPerSample;
}flac_metadata_streaminfo;

typedef struct flac_seek_point_t
{
  uint64            nSampleNoTargetFrame;
  uint64            nOffSetTargetFrameHeader;
  uint64            nTS;
  uint64            nDuration;
  uint16            nNumberOfSampleInTargetFrame;
}flac_seek_point;

typedef struct flac_metadata_seektable_t
{
  flac_seek_point*  pSeekPoints;
  uint32            nSeekPoints;
}flac_metadata_seektable;

typedef struct flac_metadata_picture_t
{
  uint8*            pPictData;
  uint8*            pMimeString;
  uint8*            pDescString;
  FS_PICTURE_TYPE   ePicType;
  uint32            ulLengthPictData;
  uint32            ulLengthMimeTypeString;
  uint32            ulLengthDescString;
  uint32            ulWidth;
  uint32            ulHeight;
  uint32            ulBitsPerPixel;
  uint32            ulIndexedColoPict;
}flac_metadata_picture;

typedef struct flac_frame_header_t
{
  uint64 nTimeStampMs;
  uint16 nSyncCode;
  uint8  nBlockingStrategy;
  uint8  nBlockSizeInterChannles;
  uint8  nSampleRate;
  uint8  nChannelsAssignment;
  uint8  nSampleSizeinBits;
}flac_frame_header;

typedef enum flac_hdr_sample_rate_enum
{
  SAMPLE_RATE_SAME_AS_STREAMINFO,
  SAMPLE_RATE_88_2,
  SAMPLE_RATE_176_4,
  SAMPLE_RATE_192,
  SAMPLE_RATE_8,
  SAMPLE_RATE_16,
  SAMPLE_RATE_22_05,
  SAMPLE_RATE_24,
  SAMPLE_RATE_32,
  SAMPLE_RATE_44_1,
  SAMPLE_RATE_48,
  SAMPLE_RATE_96,
  SAMPLE_RATE_8BIT_END_OF_HEADER_IN_HZ,
  SAMPLE_RATE_16BIT_END_OF_HEADER_IN_KHZ,
  SAMPLE_RATE_16BIT_END_OF_HEADER_IN_10_HZ,
  SAMPLE_RATE_INVALID
}flac_hdr_sample_rate_enum;

typedef enum flac_sample_size_enum_t
{
  SAMPLE_SIZE_SAMEAS_STREAMINFO,
  SAMPLE_SIZE_8_BITS_PER_SAMPLE,
  SAMPLE_SIZE_12_BITS_PER_SAMPLE,
  SAMPLE_SIZE_RESERVED1,
  SAMPLE_SIZE_16_BITS_PER_SAMPLE,
  SAMPLE_SIZE_20_BITS_PER_SAMPLE,
  SAMPLE_SIZE_24_BITS_PER_SAMPLE,
  SAMPLE_SIZE_RESERVED2
}flac_sample_size_enum;

typedef struct flac_meta_data_t
{
  uint8*            pMetaData;
  uint32            nMetaDataLength;
  uint16            nMetaDataFieldIndex;
  bool              bAvailable;
}flac_meta_data;

#endif//#ifndef _FLAC_PARSER_DATA_DEFN

