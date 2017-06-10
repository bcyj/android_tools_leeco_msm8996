#ifndef _OGG_STREAM_PARSER_DATA_DEFN
#define _OGG_STREAM_PARSER_DATA_DEFN

/* =======================================================================
                              OGGStreamParserDataDefn.h
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
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/OGGParserLib/main/latest/inc/OGGStreamParserDataDefn.h#11 $
========================================================================== */

#include "OGGStreamParserConstants.h"
/*
*******************************************************************
* Data types
*******************************************************************
*/

//Identify track codec type in OGG container
typedef enum media_codec_type_
{
  OGG_UNKNOWN_AUDIO_VIDEO_CODEC,
  OGG_AUDIO_CODEC_VORBIS,
  OGG_AUDIO_CODEC_FLAC,
  OGG_VIDEO_CODEC_THEORA
}ogg_media_codec_type;

//To store ogg meta-data
typedef struct ogg_meta_data_t
{
  uint16            nMetaDataFieldIndex;
  uint32            nMetaDataLength;
  uint8*            pMetaData;
  bool              bAvailable;
}ogg_meta_data;

//Stores video codec information
typedef struct ogg_video_info_t
{
  uint32                TrackId;
  uint8                 Vmaj;
  uint8                 Vmin;
  uint8                 Vrev;
  uint16                FmbW;
  uint16                FmbH;
  uint32                Width;
  uint32                Height;
  uint8                 PicX;
  uint8                 PicY;
  float                 FrameRate;
  uint32                AspectRatio_Nmr;
  uint32                AspectRatio_Dmr;
  uint8                 CS;
  int32                 BitRate;
  uint8                 QUAL;
  uint8                 Kfgshift;
  uint8                 PF;
  uint32                TimeInMsecPerFrame;
  uint32                SerialNo;
  uint32                SeqNo;
  uint64                EndGranulePosn;
  uint8*                IdentificationHdr;
  uint32                nIdentificationHdrSize;
  uint8*                CommentHdr;
  uint32                nCommentHdrSize;
  uint8*                SetupHdr;
  uint32                nSetupHdrSize;
  bool                  bSentIdentificationHdr;
  bool                  bSentCommentHdr;
  bool                  bSentSetupHdr;
  uint8*                pCodecHeader;
  ogg_media_codec_type  Video_Codec;
}ogg_video_info;

//Stores audio codec information
typedef struct ogg_audio_info_t
{
  uint32               TrackId;
  uint32               SamplingFrequency;
  uint8                NumberOfChannels;
  int32                NominalBitRate;
  int32                MaximumBitRate;
  int32                MinimumBitRate;
  uint32               BlockSize_0;
  uint32               BlockSize_1;
  bool                 FramingFlag;
  uint32               SerialNo;
  uint32               SeqNo;
  uint64               EndGranulePosn;
  uint8*               IdentificationHdr;
  uint32               nIdentificationHdrSize;
  uint8*               CommentHdr;
  uint32               nCommentHdrSize;
  uint8*               SetupHdr;
  uint32               nSetupHdrSize;
  uint16               nBitsPerSample;
  bool                 bSentIdentificationHdr;
  bool                 bSentCommentHdr;
  bool                 bSentSetupHdr;
  ogg_media_codec_type Audio_Codec;
  uint8*               pCodecHeader;
}ogg_audio_info;

//Stores infortion regarding OGG Page
typedef struct _ogg_page_t
{
  ogg_media_codec_type  Codec;
  uint8                 Version;
  uint8                 HdrType;
  uint64                Granule;
  uint32                SerialNo;
  uint32                SeqNo;
  uint32                CheckSum;
  uint32                PageSegments;
  uint64                PageOffset;   //Absolute file offset
  uint64                PageEndOffset;//Absolute file offset
  uint64                SegmentTableOffset;//Relative offset w.r.t PageOffset
  uint8                 ContFlag;
  uint8                 BOSFlag;
  uint8                 EOSFlag;
  uint32                nPageSegmentsTransmitted;
  uint32                nPageSegmentsCorruptSample;
}OggPage;

typedef struct _ogg_stream_sample_info_
{
  uint32 nsample;
  uint32 nsize;
  uint32 noffset;
  uint64 ntime;
  bool   bsync;
}ogg_stream_sample_info;

typedef struct ogg_stream_index_page_info_t
{
  uint64   nPageStartOffset;
  uint64   nPageEndOffset;
  uint64   nPageEndGranule;
  uint64   nTS;
}ogg_stream_index_page_info;

typedef struct _ogg_stream_index_t
{
  uint32                       serialNo;
  uint32                       nPagesAllocated;
  uint32                       nPagesIndexed;
  ogg_stream_index_page_info*  pPagesInfo;
  ogg_media_codec_type         streamCodecType;
  bool                         bInUse;
}ogg_stream_index;

typedef struct _ogg_index_t
{
  uint8              nAudioStreamsIndexed;
  ogg_stream_index** pAudioIndex;
  uint8              nVideoStreamsIndexed;
  ogg_stream_index** pVideoIndex;
}ogg_index;
#endif//#ifndef _OGG_STREAM_PARSER_DATA_DEFN

