#ifndef AVI_HEADERS_H
#define AVI_HEADERS_H

/* =======================================================================
                              aviHeaders.h
DESCRIPTION

Copyright 2011-2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/inc/aviheaders.h#22 $
========================================================================== */

/*
* Headers as defined in AVI specification.
* Refer to AVI specification for more information
* about each header and it's individual field.
*/
#include "aviconstants.h"
#include "avifourcc.h"

#define MPEG4_VOP_START_CODE                      0x000001B6
typedef struct avi_mainheader_avih_t
{
  avi_int32 dwMicroSecPerFrame;
  avi_int32 dwMaxBytesPerSec;
  avi_int32 dwPaddingGranularity;
  avi_int32 dwFlags;
  avi_int32 dwTotalFrames;
  avi_int32 dwInitialFrames;
  avi_int32 dwStreams;
  avi_int32 dwSuggestedBufferSize;
  avi_int32 dwWidth;
  avi_int32 dwHeight;
  avi_int32 dwReserved[4];
} avi_mainheader_avih;

typedef struct avi_header_strh_t
{
  int present;
  fourCC_t fccType;
  fourCC_t fccHandler;
  avi_int32 dwFlags;
  avi_int16 wPriority;
  avi_int16 wLanguage;
  avi_int32 dwInitialFrames;
  avi_int32 dwScale;
  avi_int32 dwRate;  /* dwRate / dwScale == samples/second */
  avi_int32 dwStart;
  avi_int32 dwLength;
  avi_int32 dwSuggestedBufferSize;
  avi_int32 dwQuality;
  avi_int32 dwSampleSize;
  avi_int16 rcFrame_left;
  avi_int16 rcFrame_top;
  avi_int16 rcFrame_right;
  avi_int16 rcFrame_bottom;
} avi_header_strh;


typedef struct avi_header_video_strf_t
{
  avi_int32   biSize;
  avi_int32   biWidth;
  avi_int32   biHeight;
  avi_int16   biPlanes;
  avi_int16   biBitCount;
  avi_int32   biCompression;
  avi_int32   biSizeImage;
  avi_int32   biXPelsPerMeter;
  avi_int32   biYPelsPerMeter;
  avi_int32   biClrUsed;
  avi_int32   biClrImportant;
  avi_uint8*  pExtraData;
  avi_uint16  nExtraData;
} avi_header_video_strf;


typedef struct avi_header_audio_strf_t
{
  avi_int16 wFormatTag;
  avi_int16 nChannels;
  avi_int32 nSamplesPerSec;
  avi_int32 nAvgBytesPerSec;
  avi_int16 nBlockAlign;
  avi_int16 wBitsPerSample;
  avi_int16 cbSize;
  avi_uint8* extra;
} avi_header_audio_strf;


/* extra data required in 'strf' for MPEG Layer 3 audio */
typedef struct avi_header_strf_mp3_extra_t
{
  avi_uint16 wID;
  avi_uint32 fdwFlags;
  avi_int16  nBlockSize;
  avi_int16  nFramesPerBlock;
  avi_int16  nCodecDelay;
} avi_header_strf_mp3_extra;

/* extra data required in 'strf' for Windows Media audio */
typedef struct avi_header_strf_wma_extra_t
{
  avi_uint16 nEncodeOpt;
  avi_uint16 nAdvancedEncodeOpt;
  avi_uint32 nAdvancedEncodeOpt2;
  avi_uint32 dwChannelMask;
} avi_header_strf_wma_extra;

typedef struct avi_header_mp2_extra_t
{
  /* variables for mp2 */
  avi_int32 layer;
  avi_int32 bitrate;
  avi_int32 mode_extension;
  avi_int32 emphasis;
} avi_header_mp2_extra;

typedef struct avi_header_strn_t
{
  avi_uint16 streamNameSize;
  avi_uint8 *streamName;
} avi_header_strn;

typedef struct avi_video_info_t
{
  avi_header_strh       strhVideo;
  avi_header_video_strf strfVideo;
  avi_header_strn       strnVideo;
} avi_video_info;

typedef struct avi_audio_info_t
{
  avi_header_strh       strhAudio;
  avi_header_audio_strf strfAudio;
  avi_header_strn       strnAudio;
} avi_audio_info;

typedef struct avi_audiotrack_summary_info_t
{
  avi_uint32 audioBytesPerSec;
  avi_uint32 audioFrequency;
  avi_int16  wFormatTag;
  avi_int16  nBlockAlign;
  avi_int16  nChannels;
  avi_uint8  isVbr;
  avi_int16  nbitsPerAudioSample;
  avi_uint8  trackID;
}avi_audiotrack_summary_info;

typedef struct avi_aac_info_t
{
  uint16      audioObject;
  uint32      subFormat;
  avi_uint8   samplingFreq;
  avi_uint8   channelConfig;
} avi_aac_info;

typedef struct avi_header_strd_t
{
  /// drm version number.
  int version;

  int drm_size;

  /// 'STRD' info.
  avi_uint8* drm_info;

  int drm_offset;

} avi_header_strd;

////////////////////////////// BEGIN AVI INDEXES  //////////////////////////////////////

/*
The AVI file format 1.0 specifies an optional ‘idx1’ chunk, which contains a list of the offsets and
chunk types of every chunk inside the LIST ‘movi’ chunk. This list is used to make AVI
playback and seeks more efficient because the location of each frame of video can be found
without scanning through each sub-chunk of the LIST ‘movi’ data.
*/
typedef struct avi_idx1_entry_t
{
  //First double word contains trackId and chunkType.
  avi_uint16 trackId;
  CHUNK_t chunkType;

  //Flags to indicate whether a given frame
  //is a key frame and other addiontal information.
  avi_uint32 dwFlags;

  /*
  * Stores the offset of this entry in IDX1.
  * This is used in case of reposition as parser does not store all audio/video entries.
  * If a seek time falls between 2 entries, parser begins looking at the each and every
  * entry from this offset untill the closest TS is found.
  */
  avi_uint64 dwOffsetInIDX1;

  //Offset of the chunk from the start of 'MOVI'
  avi_uint64 dwOffset;

  //Size of the chunk in bytes
  avi_uint32 dwSize;

  //Stores number of video frames before current entry from MOVI.
  avi_uint32 dwVidFrameCount;

  //Store number of audio frames/chunk before current entry from MOVI.
  avi_uint32 dwAudFrameCount;

  //stores the timestamp corresponding to current entry.
  uint64 nTimeStamp;

  /*
  * Stores the total size (in bytes) for all the chunks
  * from (1-dwAudFrameCount) and (1-dwVidFrameCount)
  */
  avi_uint64 nTotalSizeInBytes;
}avi_idx1_entry;

typedef struct avi_idx1_tbl_t
{
  //Start offset for IDX1 chunk.
  avi_uint64 nStartOffset;

  //Total size of IDX1 in bytes.
  avi_uint32 nTotalSize;

  //Total number of audio entries found in IDX1.
  avi_uint32 nAudioEntriesInIDX1;

  //Total number of video entries found in IDX1.
  avi_uint32 nVideoEntriesInIDX1;

  //Total entries found in IDX1.Sometimes, this is is not same as (nAudioEntries+nVideoEntries)
  avi_uint32 nTotalEntriesInIDX1;

  //Memory, pAudioEntries, is allocated for (nAudioEntiresAllocated * sizeof(avi_idx1_entry)).
  /*
  * When parsing IDX1, by default,
  * nAudioEntiresAllocated is set to (nTotalSize/AVI_IDX1_ENTRY_SIZE)/2;
  * if we run out of entries, parser will re-alloc the memory for pAudioEntries.
  * Typically, we might have to re-alloc only once as upper bound on these
  * index entries is (nTotalSize/AVI_IDX1_ENTRY_SIZE);
  */
  avi_uint32 nAudioEntriesAllocated;
  avi_idx1_entry* pAudioEntries;

  //Total number of audio frames found so far in IDX1.
  avi_uint32 nCurrParsedAudioEntriesInIDX1;


  //Memory, pKeyVideoEntries, is allocated for (nVideoEntriesAllocated * sizeof(avi_idx1_entry)).
  /*
  * When parsing IDX1, by default,
  * nVideoEntriesAllocated is set to (nTotalSize/AVI_IDX1_ENTRY_SIZE)/2;
  * if we run out of entries, parser will re-alloc the memory for pKeyVideoEntries.
  * Typically, we might have to re-alloc only once as upper bound on these
  * index entries is (nTotalSize/AVI_IDX1_ENTRY_SIZE);
  * Since, we usually store only key/sync frames for video,we might not have to re-alloc at all.
  */
  avi_uint32 nVideoEntriesAllocated;
  avi_idx1_entry* pKeyVideoEntries;

  //Total number of key/sync frames found so far in IDX1.
  avi_uint32 nKeyVideoyEntriesInIDX1;
  avi_uint64 nTotalAudioBytes;
}avi_idx1_table;

/*
* Another index structure exists for AVI files as described below.
* It has index information for each stream as opposed to idx1
* which has indexing information for all the streams.
* @refer to aviConstants.h for various types/sub-types of AVI indexes.
*/

//Individual entry when index is of type CHUNK_INDEX
//Also known as standard index.
typedef struct avi_indx_chunk_index_entry_t
{
   avi_uint64 dwOffset;
   avi_uint32 dwSize;
   bool       bKeyFrame;
}avi_indx_chunk_index_entry;

//Individual entry when index is of type FIELD_INDEX
typedef struct avi_indx_field_index_entry_t
{
   avi_uint64 dwOffset;
   avi_uint32 dwSize;
   avi_uint64 dwOffsetField2;
}avi_indx_field_index_entry;

//Individual entry when index is of type INDEX_OF_INDEXS, SUPER Index.
typedef struct avi_indx_super_index_entry_t
{
   avi_uint64 qwOffset;
   avi_uint32 dwSize;
   avi_uint32 dwDuration;
}avi_indx_super_index_entry;
/*
* Format for AVI Standard Index Chunk(information that indexes AVI frames) and
* AVI Field Index Chunk(information that indexes location of each field in the frame)
* is same. These indexes are identified by 'ix##' chunk where ## is stream number.
*
* There is an array of
* avi_indx_chunk_index_entry/avi_indx_field_index_entry following
* dwReserved and it's size is included in 'cb'.
*/
typedef struct avi_std_ix_tbl_t
{
    fourCC_t    fcc;              //FourCC code, 'indx'
    avi_int32   cb;               //Size of 'indx' excluding intial sizeof(fcc) and sizeof(cb) bytes.
    avi_int16   wLongsPerEntry;   // size of each entry in aIndex array in words.
    char        bIndexSubType;    // Index Sub Type.
    char        bIndexType;       // Type of Index.
    avi_uint32  nEntriesInUse;    // index of first unused number in aIndex arrary.
    avi_uint32  dwChunkId;        // '##dc' OR '##db' OR ...
    avi_uint64  qwBaseOffset;     //All dwOffsets in aIndex are relative to this offset.
    avi_uint32  dwReserved;
    avi_indx_chunk_index_entry* pIndxChunkIndexEntry;
    avi_indx_field_index_entry* pIndxFieldIndexEntry;
    avi_indx_super_index_entry* pIndxSuperIndexEntry;
} avi_std_ix_tbl;

/*
* Format for AVI Base index and AVI super index is same.
* Super index is identified by 'ix##'in 'movi', where ## is stream number.
* Base index is identified by 'indx'.
* 'indx' can either be an index of indexes(super index, then there is 'ix##' in 'movi')
* or index to the chunks directly.
*
* If 'indx' chunk is not super index then stream has only one index chunk and there
* is none in 'MOVI'.
*
* If 'indx' is super index then corresponding index chunks are marked with 'ix##' in 'MOVI'.
*
* There is an array of
* avi_indx_super_index_entry/avi_indx_chunk_index_entry/avi_indx_field_index_entry following
* dwReserved[3] and it's size is included in 'cb'.
*/
typedef struct avi_indx_tbl_t
{
    bool        isAvailable;      //Set when present
    avi_int32   cb;               //Size of 'indx' excluding intial sizeof(fcc) and sizeof(cb) bytes.
    avi_int16   wLongsPerEntry;   // size of each entry in aIndex array in words.
    char        bIndexSubType;    // Index Sub Type.
    char        bIndexType;       // Type of Index.
    avi_uint32  nEntriesInUse;    // index of first unused number in aIndex arrary.
    avi_uint32  dwChunkId;        // '##dc' OR '##db' OR ...
    avi_uint32  dwReserved[3];
    avi_indx_super_index_entry* pIndxSuperIndexEntry;
    avi_std_ix_tbl* pIXIndexChunk;
} avi_indx_tbl;

/*
* IndexType and SubIndexType are used to distinguish
* among AVI standard Index Chunk, AVI Super Index and AVI Field Index chunk.
*/
////////////////////////////// END AVI INDEXES  //////////////////////////////////////

typedef struct avi_stream_entry_info_t
{
  CHUNK_t type;
  avi_uint32 index;
  avi_uint8 audioIndex;
  avi_uint8 videoIndex;
  bool bParsed;
} avi_stream_entry_info;


typedef struct avi_sample_info_t
{
  avi_uint32 nSampleSize;
  avi_uint64 nTimeStamp;
  avi_uint32 nDuration;
  CHUNK_t    chunkType;
  bool       bSync;
}avi_sample_info;

typedef struct avi_riff_info_t
{
  avi_uint64 startOffset;
  avi_uint32 size;
}avi_riff_info;

typedef struct avi_summary_info_t
{
  avi_mainheader_avih* avih;
  avi_int32 odml_total_frames;
  avi_video_info text_info[AVI_MAX_VIDEO_TRACKS];
  avi_int32 n_text_tracks;
  avi_video_info video_info[AVI_MAX_VIDEO_TRACKS];
  avi_int32 n_video_tracks;
  avi_audio_info audio_info[AVI_MAX_AUDIO_TRACKS];
  avi_int32 n_audio_tracks;
  avi_uint32 n_streams;
  avi_stream_entry_info stream_index[AVI_MAX_TRACKS];
  avi_idx1_table* pIdx1Table;
  avi_uint8 nCurrAudioTrackInfoIndex;
  avi_uint8 nCurrVideoTrackInfoIndex;
  avi_uint8 nCurrTextTrackInfoIndex;
  /*
  *This keeps increasing whenever parser encounters STRL.
  *Current value is assigned to audio/video track id before incrementing.
  */
  avi_uint8 nNextAvailTrackIndex;
} avi_summary_info;

typedef struct avi_info_chunk_t
{
  char* Ptr;
  avi_uint16 nSize;
}avi_info_chunk;

typedef struct avi_info_struct_t
{
  avi_info_chunk ArchLocn;
  avi_info_chunk Artist;
  avi_info_chunk Commissioned;
  avi_info_chunk Comments;
  avi_info_chunk Copyright;
  avi_info_chunk CreateDate;
  avi_info_chunk Genre;
  avi_info_chunk Keyword;
  avi_info_chunk Name;
  avi_info_chunk Product;
  avi_info_chunk Subject;
  avi_info_chunk Software;
  avi_info_chunk Source;
}avi_info_struct;

typedef struct memory_struct_t
{
  avi_uint8* pMemory;
  avi_uint32 nSize;
}memory_struct;

typedef struct avi_parser_seek_buffer_cache_t
{
  avi_uint8* pMemory;
  avi_uint64 nSize;
  avi_uint64 nStartOffset;
  avi_uint64 nEndOffset;
  avi_int64  nReadOffset;
}avi_parser_seek_buffer_cache;

typedef enum AVIVOPTYPE
{
  NO_VOP = -1, // bitstream contains no VOP.
  MPEG4_I_VOP = 0,   // bitstream contains an MPEG4 I-VOP
  MPEG4_P_VOP = 1,   // bitstream contains an MPEG4 P-VOP
  MPEG4_B_VOP = 2,   // bitstream contains an MPEG4 B-VOP
  MPEG4_S_VOP = 3,   // bitstream contains an MPEG4 S-VOP
  H264_I_VOP = 4,   // bitstream contains an H264 I-VOP
  H264_P_VOP = 5,   // bitstream contains an H264 P-VOP
  H264_B_VOP = 6,   // bitstream contains an H264 B-VOP
  H264_S_VOP = 7   // bitstream contains an H264 S-VOP
}AVI_VOP_TYPE;

#endif


