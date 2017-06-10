#ifndef _MKAV_PARSER_DATA_DEFN
#define _MKAV_PARSER_DATA_DEFN

/* =======================================================================
                              MKAVParserDataDefn.h
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
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MKAVParserLib/main/latest/inc/mkavparserdatadefn.h#29 $
========================================================================== */
/*
*******************************************************************
* Data types
*******************************************************************
*/
#include "parserdatadef.h"

#define MKAV_MAX_UID_SIZE                (16)
#define MKAV_VORBIS_CODEC_HDRS           (3)
#define MSFT_MGR_CODEC_FIXED_HEADER_SIZE (40)
#define WAVEFORMATEX_SIZE                (18)

typedef enum mkav_tracktype_enum_t
{
  MKAV_TRACK_TYPE_UNKNOWN     =0x00,
  MKAV_TRACK_TYPE_VIDEO       =0x01,
  MKAV_TRACK_TYPE_AUDIO       =0x02,
  MKAV_TRACK_TYPE_AUDIO_VIDEO =0x03,
  MKAV_TRACK_TYPE_LOGO        =0x10,
  MKAV_TRACK_TYPE_SUBTITLE    =0x11,
  MKAV_TRACK_TYPE_BTN         =0x12,
  MKAV_TRACK_TYPE_CONTROL     =0x20
}mkav_track_type;

//Identify track codec type in MKAV container
typedef enum mkav_media_codec_type_
{
  MKAV_UNKNOWN_CODEC,
  MKAV_MSFT_MGR_CODEC,
  MKAV_RAW_UNCOMPRESSED_VIDEO_CODEC,
  MKAV_MPEG4_ISO_SIMPLE_PROFILE_CODEC,
  MKAV_MPEG4_ISO_ADVANCE_SIMPLE_PROFILE_CODEC,
  MKAV_MPEG4_ISO_ADVANCE_PROFILE_CODEC,
  MKAV_AVC1_VIDEO_CODEC,
  MKAV_THEORA_VIDEO_CODEC,
  MKAV_MPEG2_VIDEO_CODEC,
  MKAV_MPEG1_VIDEO_CODEC,
  MKAV_SPARK_VIDEO,
  MKAV_SORENSON_VIDEO,
  MKAV_VP8_VIDEO,
  MKAV_VP9_VIDEO,
  MKAV_H263_VIDEO,
  MKAV_MPEG4_VIDEO,
  MKAV_DIV3_VIDEO,
  MKAV_DIVX_VIDEO,
  MKAV_DIVX50_VIDEO,
  MKAV_WMVA_VIDEO,
  MKAV_WVC1_VIDEO,
  MKAV_WMV1_VIDEO,
  MKAV_WMV2_VIDEO,
  MKAV_WMV3_VIDEO,
  MKAV_HEVC_VIDEO_CODEC,
  MKAV_MICROSOFT_MPEG_V3_CODEC,
  MKAV_MP3_AUDIO,
  MKAV_MP2_AUDIO,
  MKAV_MP1_AUDIO,
  MKAV_PCM_INT_BENDIAN_CODEC,
  MKAV_PCM_INT_LENDIAN_CODEC,
  MKAV_PCM_FLOAT_CODEC,
  MKAV_AC3_AUDIO_CODEC,
  MKAV_EAC3_AUDIO_CODEC,
  MKAV_DOLBY_AC3_CODEC,
  MKAV_DTS_AUDIO_CODEC,
  MKAV_FLAC_AUDIO_CODEC,
  MKAV_VORBIS_AUDIO_CODEC,
  MKAV_AAC_AUDIO,
  MKAV_MSFT_MGR_AUDIO_CODEC,
  MKAV_WMA_AUDIO,
  MKAV_WMA_PRO_AUDIO,
  MKAV_WM_LOSSLESS,
  MKAV_WM_SPEECH,
  MKAV_OPUS,
  MKAV_UTF8,
  MKAV_SSA,
  MKAV_ASS,
  MKAV_USF,
  MKAV_VOBSUB,
  MKAV_BMP,
  MKAV_KARAOKE,
}mkav_media_codec_type;

typedef enum _EBML_Content_Encode_Type
{
  CONTENT_COMPRESSED = 0,
  CONTENT_ENCRYPTED,
  UNKNOWN_ENCODE_TYPE
} EBML_CONTENT_ENCODE_TYPE;

typedef enum _EBML_Content_Compression_Algo
{
  EBML_CONTENT_COMPRESSION_ZLIB = 0,
  EBML_CONTENT_COMPRESSION_BZLIB,
  EBML_CONTENT_COMPRESSION_LZOLX,
  EBML_CONTENT_COMPRESSION_HEADER_STRIP,
  EBML_CONTENT_COMPRESSION_UNKNOWN
} EBML_CONTENT_COMPRESSION_ALGO;

typedef enum _EBML_Content_Encoding_Scope
{
  ALL_FRAMES          = 1,
  CODEC_PRIVATE_DATA  = 2,
  CONTENT_COMPRESSION = 4,
} EBML_CONTENT_ENCODING_SCOPE;

typedef union _decimal_value_t
{
  float  floatVal;
  double doubleVal;
}decimal_value;

typedef struct _ebml_doc_hdr_t
{
  uint8  nVersion;
  uint8  nReadVersion;
  uint8  nMxIdLength;
  uint8  nMxSzLength;
  uint8  nDocTypeVersion;
  uint8  nDocTypeRead;
  uint8* pDocType;
  uint8  nDocTypeLength;
}ebml_doc_hdr;

typedef struct _seg_element_info_t
{
  uint64 nDataOffset;
  uint64 nDataSize;
}segment_element_info;

typedef struct _seg_info_t
{
  uint8  SegUID[MKAV_MAX_UID_SIZE];
  uint8  PrvUID[MKAV_MAX_UID_SIZE];
  uint8  nextUID[MKAV_MAX_UID_SIZE];
  uint8* pSegFileName;
  uint8* pPrvFileName;
  uint8* pNextFileName;
  uint8* pTitle;
  uint8* pMuxingApp;
  uint8* pWritingApp;
  uint64 nTimeCodeScale;
  uint64 nDate;
  uint64 nDuration;
}segment_info;

typedef struct _seek_info_t
{
  uint32 ebml_id;
  uint64 ebml_id_file_posn;
}seek_info;

typedef struct _seek_head_t
{
  seek_info* pSeekInfo;
  uint32     nSeekInfo;
}seekhead;

typedef struct _seek_head_info_t
{
  seekhead* pSeekHead;
  uint32    nSeekHead;
}seekheadinfo;

typedef struct _block_info_t
{
  uint32 *pFramesSize;    //! Pointer to store frames size
  uint64 nDataOffset;     //! Offset where media data starts after header
  uint32 nTrackNumber;    //! Track number
  uint32 nDataSize;       //! Media data size after header
  int16  nTimeCode;       //! Timestamp with respect to cluster
  uint8  nBlockHeaderSize;//! Block Header size
  uint8  nFlags;          //! Block properties (contains key frame info)
  uint8  nFramesParsed;   //! Number of frames parsed
  uint8  nFrames;         //! Total number of frames in the block
}blockinfo;

typedef struct _block_group_t
{
  blockinfo* pBlockInfo;        //! Pointer to store current block properties
  uint64     nBlockStartOffset; //! Block Group Start Offset
  uint64     nBlockEndOffset;   //! Block Group End offset
  uint64     nBlockDuration;    //! Block Duration
  int64      nRefBlock;         //! It indicates the frames are depending on
                                //! other frames. If it is not present, then
                                //! frames in block group are key frames
  uint32     nBlockinfo;        //! Total Number of blocks in the block group
  uint32     nBlocksParsed;     //! Number of blocks parsed
}blockgroup_info;

typedef struct _cluster_info_t
{
  uint64           nPrevSize;
  uint64           nPosn;
  uint64           nTimeCode;
  blockgroup_info* pBlockGroup;
  uint32           nBlockGroup;
  blockinfo*       pSimpleBlocks;
  uint32           nSimpleBlocks;
  uint8            nHdrSize;
  uint64           nOffset;//absolute file offset where first block starts in a given cluster
  uint64           nSize;
}cluster_info;

typedef struct _all_clusters_info_t
{
  cluster_info* pClusterInfo;
  uint32        nClusters;
}all_clusters_info;

typedef struct _cue_ref_info_t
{
  uint64 nCueRefTime;
  uint64 nCueRefCluster;
  uint64 nCueRefNo;
  uint64 nCueRefCodecState;
}cue_ref_info;

typedef struct _cue_track_posn_info_t
{
  uint64 nCueTrack;
  uint64 nCueClusterPosn;
  uint64 nCueBlockNumber;
  uint64 nCueCodecState;
  uint64 nCueRef;
  cue_ref_info* pCueRefInfo;
}cue_track_posn_info;

typedef struct _cue_point_info_t
{
  uint64               nCueTime;
  uint64               nCueTrackPosnInfo;
  cue_track_posn_info* pCueTrackPosnInfo;
}cue_point_info;

typedef struct _allcuesinfo_t
{
  cue_point_info* pCuePointInfo;
  uint32 nCuePoints;
}all_cues_info;

/* The following structures are used to maintain internal seek table,
   if cue points are not available in the clip.
   Delta is configurable parameter. Currently we kept this as 1sec.
   Depending on file duration, we will configure this value in accordance
   with Maximum Entries in Seek Table. */
#define SEEK_ENTRY_DELTA (500)
#define MAX_SEEK_ENTRIES (1000)

/* Maximum allowed delta between Seek table entry and requested seek time.
   If Difference between Seek time and closest seek entry are more than this
   limit, then Parser will use Cluster properties to find out the nearest sync
   sample. */
#define MAX_SEEK_DELTA (4000)
typedef struct _seek_table_entry_t
{
  uint64 ullTime;   /* Timestamp of the entry */
  uint64 ullOffset; /* Offset of the entry    */
  /* Flag to set whether stored frame is key frame or not */
  uint8  ucIsKeyFrame;
} seek_table_entry;

typedef struct _seek_table_t
{
  uint32 ulDelta;                      /* Delta for each entry               */
  uint32 ulEntries;                    /* Total number of entries            */
  uint32 ulLowerBound;                 /* Lower Bound to the valid range     */
  uint32 ulUpperBound;                 /* Upper Bound to the valid range     */
  seek_table_entry *pSeekTableEntries; /* Pointer to store Seek Table Entries*/
} seek_table_info;

typedef struct avc1_info
{
  uint8   nProfile;
  uint8   nLevel;
  uint8   NALU_Len_Minus1;
  uint8   nSP;
  uint16* pSPLengths;
  uint8*  pSPData;
  uint32  nSPData;
  uint8   nPP;
  uint16* pPPLengths;
  uint8*  pPPData;
  uint32  nPPData;
  //Each SP/PP is prefixed by 4 byte code followed by the parameter data itself
  uint8*  pOutData;
  uint32  nOutData;
}mkav_avc1_info;

typedef struct mkav_video_info_t
{
  uint64 FlagInterlaced;
  uint64 StereoMode;
  uint64 PixelWidth;
  uint64 PixelHeight;
  uint64 PixelCropBottom;
  uint64 PixelCropTop;
  uint64 PixelCropLeft;
  uint64 PixelCropRight;
  uint64 DisplayWidth;
  uint64 DisplayHeight;
  uint64 DisplayUnit;
  uint64 AspectRatioType;
  uint64 ColourSpace;
  double GammaValue;
  double FrameRate;
  //pointer needs to be interpreted based on the codec type
  void*  pCodecHdr;
}mkav_video_info;

/*
*@brief Structure to store bitmap header information.
It is useful to find the extra data required for Microsoft codecs.
More info is available at http://msdn.microsoft.com/en-us/library/windows/desktop/dd183376%28v=vs.85%29.aspx.
WMV related codecs do not require this info.
If there is any extra data after this structure, that info has to be provided
as Codec Config data during init.
*/
typedef struct _mkav_vfw_info_t
{
  //! The number of bytes required by the structure.
  uint32     biSize;
  //! The width of the bitmap, in pixels.
  uint32     biWidth;
  //! The height of the bitmap, in pixels.
  uint32     biHeight;
  //! The number of planes for the target device. This value must be set to 1.
  uint16     biPlanes;
  //! The number of bits-per-pixel.
  uint16     biBitCount;
  //! The type of compression for a compressed bottom-up bitmap
  uint32     biCompression;
  //! The size, in bytes, of the image. This may be set to zero for BI_RGB bitmaps.
  uint32     biSizeImage;
  //! The horizontal resolution, in pixels-per-meter.
  uint32     biXPelsPerMeter;
  //! The vertical resolution, in pixels-per-meter
  uint32     biYPelsPerMeter;
  //! The number of color indexes in the color table that are actually used
  uint32     biClrUsed;
  //! The number of color indexes that are required for display
  uint32     biClrImportant;
} mkav_vfw_info;

/*
 * @brief The WAVEFORMATEX structure defines the format of waveform-audio data.
 * This structure is used to update WMA related audio codecs Config data.
*/
typedef struct _mkav_afw_info_t
{
  //! Waveform-audio format type.
  uint16  wFormatTag;
  //! Number of channels in the waveform-audio data.
  uint16  nChannels;
  //! Sample rate, in samples per second (hertz).
  uint32  nSamplesPerSec;
  //! Required average data-transfer rate, in bytes per second
  uint32  nAvgBytesPerSec;
  //! Block alignment, in bytes.
  uint16  nBlockAlign;
  //! Bits per sample
  uint16  wBitsPerSample;
  //! Size, in bytes, of extra format information appended to the end of struct
  uint16  cbSize;
} mkav_afw_info;

/* @brief WMA Extra data structure format
 * This is Extra info available after WaveFormatEx structure.
 * By using this structure, Audio Track Info Structure will be updated.
*/
typedef struct _mkav_ms2_audio_info_t
{
  //! Samples per block
  uint32        ulSamplesPerBlock;
  //! Encoder Options field
  uint16        usEncodeOptions;
  //! Super block align in bytes
  uint32        ulSuperBlockAlign;
} mkav_afw_ms2_audio_info;

/* @brief WMA Pro Extra data structure format
 * This is Extra info available after WaveFormatEx structure.
 * By using this structure, Audio Track Info Structure is filled.
*/
typedef struct _mkav_wm3_audio_info_t
{
  //! Actual bits per sample
  uint16        usValidBitsPerSample;
  //! Channel Mask info
  uint32        ulChannelMask;
  //! Reserved field
  uint32        ulReserved1;
  //! Advanced Encoder Options field
  uint32        ulAdvancedEncodeOptions2;
  //! Encoder Options field
  uint16        usEncodeOptions;
  //! Advanced Encoder Options field
  uint16        usAdvancedEncodeOptions;
} mkav_afw_wm3_audio_info;

/* @brief Audio Metadata Info struct
 * This structure contains metadata required to initialize audio codec.
*/
typedef struct _mkav_audio_info_t
{
  //! Encoder Delay
  uint64 ullEncoderDelay;
  //! Seek PreRoll Delay
  uint64 ullSeekPrerollDelay;
  //! Sampling Frequency
  uint64 SamplingFrequency;
  //! Output Sampling Frequency
  uint64 OutputSamplingFrequency;
  //! Number of channels
  uint64 ullNumChannels;
  //! Channel Mask
  uint64 ullChannelMask;
  //! Bits per sample
  uint64 ullBitsPerSample;
  //! Following parameters are required for WMA related audio codecs
  //! Format tag (required for WMA related audio codecs)
  uint16 usFormatTag;
  //! Block align in bytes
  uint16 usBlockAlign;
  //! Bit rate
  uint32 ulBitRate;
  //! Encoder options
  uint16 usEncoderOptions;
  //! Advanced Encoder options field (for WMA Pro codecs)
  uint16 usAdvEncoderOptions;
  //! Extra Advanced Encoder Options (for WMA Pro codecs)
  uint32 ulAdvEncoderOptions2;
  //! Number of Samples per Block
  uint32 ulSamplesPerBlock;
}mkav_audio_info;

typedef struct _mkav_encode_info_t
{
  uint32 ulEncodeOrder;
  void*  pCompSettings;
  uint32 ulCompSettingLen;
  EBML_CONTENT_ENCODING_SCOPE   eEncodeScope;
  EBML_CONTENT_ENCODE_TYPE      eEncodeType;
  EBML_CONTENT_COMPRESSION_ALGO eCompAlgo;
}mkav_encode_info;

typedef struct mkav_track_entry_info_t
{
  mkav_video_info*         pVideoInfo;
  mkav_audio_info*         pAudioInfo;
  mkav_encode_info*        pEncodeInfo;
  uint8*                   pName;
  uint8*                   pLanguage;
  uint8*                   pCodecPvt;
  uint8*                   pCodecName;
  mkav_track_type          TrackType;
  mkav_media_codec_type    CodecType;
  uint64                   TrackNo;
  uint64                   TrackUID;
  uint64                   FlagEnabled;
  uint64                   FlagDefault;
  uint64                   FlagForced;
  uint64                   FlagLacing;
  uint64                   MinCache;
  uint64                   MaxCache;
  uint64                   AttachmentLink;
  uint64                   CodecDecodeAll;
  uint64                   TrackOverlay;
  uint64                   DefaultDuration;
  uint64                   nCodecDelay;
  uint64                   nSeekPreRoll;
  double                   TrackTimeCodeScale;
  uint32                   nCodecPvtSize;
  uint32                   nCodecConfigDataSize;
  uint32                   nNALULenMinusOne;
 //mkav_content_encoding* pEncoding;
}mkav_track_entry_info;

//To store mkav meta-data
typedef struct mkav_meta_data_t
{
  uint8*            pMetaData;
  uint32            nMetaDataLength;
  uint16            nMetaDataFieldIndex;
  bool              bAvailable;
}mkav_meta_data;

typedef struct _mkav_stream_sample_info_
{
  uint64 nMinTSAfterSeek;
  uint64 nFirstSampleTS;
  uint64 noffset;
  uint64 ntime;
  uint64 nTimeCodeScale;
  uint64 nTrackNo;
  uint32 nsample;
  uint32 nsize;
  bool   bsync;
  bool   bStartAfterSeek;
}mkav_stream_sample_info;

#endif//#ifndef _MKAV_PARSER_DATA_DEFN

