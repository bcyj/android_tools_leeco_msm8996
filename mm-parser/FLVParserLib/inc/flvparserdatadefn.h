#ifndef _FLV_PARSER_DATA_DEFN
#define _FLV_PARSER_DATA_DEFN

/* =======================================================================
                              FLVParserDataDefn.h
DESCRIPTION
Declaration of Flash Parser data types.

Copyright (c) 2012-2014 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FLVParserLib/main/latest/inc/flvparserdatadefn.h#4 $
========================================================================== */
/*
*******************************************************************
* Data types
*******************************************************************
*/
#include "parserdatadef.h"

typedef enum _flv_media_codec_type_
{
  FLV_UNKNOWN_CODEC,
  FLV_UNCOMPRESSED_AUDIO,
  FLV_ADPCM_AUDIO,
  FLV_MP3_AUDIO,
  FLV_NELLYMOSER,
  FLV_AAC,
  FLV_SORENSON_H263_VIDEO,
  FLV_SCREENVIDEO,
  FLV_VP6_VIDEO,
  FLV_VP6_ALPHA_CHANNEL,
  FLV_H264,
  FLV_SCREENVIDEO_V2
}FLVMediaCodecType;

typedef enum _flv_track_type_
{
  FLV_TRACK_TYPE_UNKNOWN,
  FLV_TRACK_AUDIO,
  FLV_TRACK_VIDEO
}FLVTrackType;

/* Video Frame type.
   Based on "ADOBE FLASH VIDEO FILE FORMAT SPECIFICATION VERSION 10.1".
   Section E.4.3.1 (VIDEODATA Section)
*/
typedef enum VIDEO_FRAME_TYPE
{
  //! It is applicable for AVC codec only
  FLV_KEY_FRAME,
  //! It is applicable for AVC codec only
  FLV_NONKEY_FRAME,
  //! It is applicable for H.263 codec only
  FLV_DISPOSABLE_FRAME,
  //! It is reserved for server use case only
  FLV_GENERATED_KEY_FRAME,
  //! Normal frame data, applicable for all video codecs
  FLV_NORMAL_VIDEO_FRAME
} VIDEO_FRAME_TYPE;


/* AVC Video Packet Type.
   Based on "ADOBE FLASH VIDEO FILE FORMAT SPECIFICATION VERSION 10.1".
   Section E.4.3.1 (VIDEODATA Section)
*/
typedef enum AVC_PACKET_TYPE
{
  //! Codec config data
  //! It is similar to avcC box in MP4/FLV file
  AVC_SEQ_HEADER,
  //! One or More NALUs (Full Frames are required)
  AVC_NALU,
  //! AVC End of Sequence
  //! This in genenral not required
  AVC_END_OF_SEQ,
} AVC_PACKET_TYPE;

/* Structure to store SPS/PPS data */
typedef struct NALUDatatype
{
  uint16 usNALLength; //! SPS/PPS data length
  uint8* pucNALBuf;   //! Buffer to store SPS/PPS daa
} NALUDatatype;

/* Codec config data for AVC/H264 in FLV is similar to 3gp/mp4 spec.
   "avcC" atom will be available as first media sample. The atom structure is
   as follows:

   Bits --    Field
   8    --    Configuration version
   8    --    AVC Profile Indication
   8    --    AVC Profile Compatibility
   8    --    AVC Level Indication
   6    --    Reserved (0x3F)
   2    --    NALU LengthSizeMinusOne
   2    --    Reserved (0xC0)
   6    --    Number of Sequence Params sets
  For loop from 0 to number of sequence param sets
   8    --    Sequence Param Set length
   uint8--    Sequence param set data
  End of for loop
   8    --    Number of Picture param sets
  For loop from 0 to number of picture param sets
   8    --    Picture Param Set length
   uint8--    Picture param set data
  End of for loop
*/
typedef struct AVCCodecBuf
{
  uint8  ucConfigVersion;
  uint8  ucAVCProfile;
  uint8  ucProfileCompatibility;
  uint8  ucAVCLevelIndication;
  uint8  ucNALLengthMinusOne;
  uint8  ucNumSeqParams;
  uint8  ucNumPicParams;
  NALUDatatype* pSeqParam;
  NALUDatatype* pPicParam;
} AVCCodecBuf;

typedef struct _flv_hdr_t
{
  uint8  ucFLVVersionInfo;
  uint8  ucAudioPresentFlag;
  uint8  ucVideoPresentFlag;
  uint32 ulDataStartOffset;
}FLVHeader;

/* Index table structure. */
typedef struct _flv_index_table_t
{
  uint8  ucTagType;
  bool   bFrameType;
  uint64 ullTimeStamp;
  uint64 ullTagOffset;
}FLVIndexTable;

typedef struct _tag_info_t
{
  uint8  ucTagType;                //tag type as per FLV spec.
  uint8  ucExtendedTimeStamp;      //Extended timestamp information
  uint8  ucAACFrameType;           //AAC Frame type, if codec is AAC
  uint8  ucCodecHeader;            //Codec Header byte
  uint8  ucTagHeaderSize;          //Tag Header size
  VIDEO_FRAME_TYPE eFrameType;     //Video Frame type, if codec is Video type
  AVC_PACKET_TYPE  eAVCPacketType; //AVC frame type, if codec is AVC
  uint32 ulCompositionTime;        //Composition time, available only for AVC
  uint32 ulTagDataSize;            //Data size in bytes for this tag
  uint32 ulTime;                   //timestamp associated with this tag
  uint32 ulStreamId;               //StreamId as per FLV spec.
  uint64 ullOffset;                //Absolute file offset where this tag starts
  uint32 ulPayloadSize;            //Size of AU data
  uint32 ulPrevTagSize;            //Tag size of the previous tag
}FLVTagInfo;

typedef struct _flv_audio_info_t
{
  uint8                 ucTrackId;
  uint8                 ucNumChannels;
  uint8                 ucBitsPerSample;
  uint8*                pucCodecConfigBuf;
  uint32                ulCodecConfigSize;
  uint32                ulSamplingRate;
  uint32                ulAudStartOffset;
  FLVMediaCodecType     eAudioCodec;
}FLVAudioInfo;

typedef struct _flv_video_info_t
{
  uint8                 ulTrackId;
  uint32                ulCodecConfigSize;
  AVCCodecBuf*          psCodecConfig;
  uint32                ulVidStartOffset;
  FLVMediaCodecType     eVideoCodec;
}FLVVideoInfo;

typedef struct _FLV_stream_sample_info_
{
  bool   bsync;
  uint64 ullSampleOffset;
  uint64 ullSampleTime;
  uint32 ulSampleNum;
  uint32 ullSize;
}FLVStreamSampleInfo;

typedef struct _flv_metadata_info_t
{
  uint32                ulAudioCodecId;
  uint32                ulAudioDataRate;
  uint32                ulAudioDelay;
  uint32                ulAudioSampleRate;
  uint32                ulAudioSampleSize;
  uint32                ulDuration;
  float                 fVidFrameRate;
  uint32                ulVidFrameWidth;
  uint32                ulVidFrameHeight;
  uint32                ulVidDataRate;
  uint32                ulVidCodecId;
  boolean               bCanSeekToEnd;
  boolean               bIsStereo;
}FLVMetaDataInfo;

#define MAX_NO_META_DATA_STR 15
#define MAX_SIZE_DATA_STR    256
const char m_cMetadataStr[MAX_NO_META_DATA_STR][MAX_SIZE_DATA_STR] =
{
  "onMetaData",   /* It is mandatory field */
  "audiocodecid",
  "audiodatarate",
  "audiodelay",
  "audiosamplerate",
  "audiosamplesize",
  "canSeekToEnd",
  "creationdate",
  "duration",
  "framerate",
  "height",
  "stereo",
  "videocodecid",
  "videodatarate",
  "width",
};

#endif//#ifndef _FLV_PARSER_DATA_DEFN

