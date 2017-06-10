#ifndef _MKAV_PARSER_H
#define _MKAV_PARSER_H
/* =======================================================================
                              MKAVParser.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2011-2014 Qualcomm Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MKAVParserLib/main/latest/inc/mkavparser.h#35 $
========================================================================== */
#include "mkavparserstatus.h"
#include "mkavparserdatadefn.h"
#include "MMDebugMsg.h"
#include "parserinternaldefs.h"
#include "filesourcetypes.h"
#include "ztl.h"
#ifdef FEATURE_FILESOURCE_MKV_PARSER
//#define MKAV_PARSER_DEBUG

/*
*Callback function used by parser for reading the data.
*Parser does not implement this function, it's the responsibility
*of the APP to implement it.
*/

extern uint32 MKAVFileCallbakGetData (uint64 ulOffset,
                                      uint32 ulNumBytesRequest,
                                      uint8* pData,
                                      uint32 ulMaxBufSize,
                                      void*  pUserData );

extern bool   MKAVCheckAvailableData(uint64*,bool*,void*);

typedef struct _tag_info_type
{
  uint8* pTagName;
  uint32 ulTagNameLen;
  uint8* pTagLang;
  uint32 ulTagLangLen;
  uint8* pTagString;
  uint32 ulTagStringLen;
  uint32 ulTagBinValue;
  bool   bTagDefault;
}tag_info_type;

class MKAVParser
{
  /*=======================================================================
  * Public Members                                                        *
  * ======================================================================*/
  public:
                            MKAVParser(void*,uint64 fsize,bool,bool);
                            ~MKAVParser();
   MKAV_API_STATUS          StartParsing(void);
   MKAV_API_STATUS          ParseByteStream(void);
   MKAV_API_STATUS          GetCurrentSample(uint32 trackId,uint8* pucDataBuf,
                                         uint32 ullBufSize, int32* plBytesRead,
                                         mkav_stream_sample_info* psampleInfo);

  /*! ======================================================================
  @brief  Repositions given track to specified time

  @detail  Seeks given track in forward/backward direction to specified time

  @param[in]
   trackid: Identifies the track to be repositioned.
   nReposTime:Target time to seek to
   nCurrPlayTime: Current playback time
   pSampleInfo: Sample Info to be filled in if seek is successful
   canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
   nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

  @return    MKAVPARSER_SUCCESS if successful other wise returns MKAVPARSER_FAIL
  @note      None.
  ========================================================================== */

   MKAV_API_STATUS          Seek(uint32, uint64, uint64,
                                 mkav_stream_sample_info*,
                                 bool, bool canSyncToNonKeyFrame=false,
                                 int  nSyncFramesToSkip = 0);
   uint8                    randomAccessDenied();
   uint32                   GetTrackWholeIDList(uint32*);
   uint8*                   GetCodecHeader(uint32, bool bRawCodec = false);
   uint32                   GetCodecHeaderSize(uint32, bool bRawCodec = false);

   uint64                   GetClipDurationInMsec();
   uint32                   GetVideoWidth(uint32);
   uint32                   GetVideoHeight(uint32);
   bool                     GetAudioTrackProperties(uint32,
                                                    mkav_audio_info*);
   uint8                    GetTotalNumberOfTracks(void){return m_nstreams;}
   uint32                   GetTrackBufferSize(uint32);
   mkav_media_codec_type    GetTrackCodecType(uint32);
   mkav_track_type          GetTrackType(uint32);
   bool                     bIsWebm() {return m_bIsWebm;}

  tag_info_type*            GetClipMetaData(uint32 ulIndex);
  FileSourceStatus          SetAudioOutputMode(FileSourceConfigItemEnum Enum);
  FileSourceStatus          GetAudioOutputMode(bool* bRet,
                                               FileSourceConfigItemEnum Enum);
/* ============================================================================
  @brief  getBufferedDuration.

  @details    This function is used to calculate the playback time based on the
              given Offset value.

  @param[in]      ulTrackId           Track Id.
  @param[in]      sllAvailBytes       Available offset.
  @param[in/out]  pullBufferedTime    Playback Time.

  @return  "TRUE" if successful in calculating the approximate playback time
           else returns "FALSE".
  @note       None.
=============================================================================*/
  bool                      getBufferedDuration(uint32  ulTrackId,
                                                int64   sllOffset,
                                                uint64* pullBufferedTime);
/* ============================================================================
  @brief  GetOffsetForTime.

  @details    This function is used to calculate the approximate offset value
              based on the given Playback timestamp value.

  @param[in]      ullPBTime           Given Playback Time.
  @param[in/out]  pullFileOffset      Parameter to store o/p Offset Value.
  @param[in]      ulTrackId           Track Id.
  @param[in]      ullCurPosTimeStamp  Current Playback Time.
  @param[in]      reposTime           Reposition Time.

  @return  "TRUE" if successful in calculating the approximate offset value
           else returns "FALSE".
  @note       None.
=============================================================================*/
  bool                      GetOffsetForTime(uint64  ullPBTime,
                                             uint64* pullFileOffset,
                                             uint32  /*ulTrackID*/,
                                             uint64  /*currentPosTimeStamp*/,
                                             uint64& /*reposTime*/);

  /* ==========================================================================
  @brief  Function to provide Metadata strings available in SegmentInfo.

  @details    This function is used to provide metadata available in SegmentIno
              and Tracks elements.

  @param[in/out]  pucDataBuf    Buffer to read metadata.
  @param[in/out]  pulBufSize    Size of the buffer.
  @param[in]      ienumData     Metadata type.

  @return     PARSER_ErrorNone indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       BufferSize should be more than metadata size value.
  ========================================================================== */
  PARSER_ERRORTYPE GetSegmentInfo(wchar_t *pucDataBuf,
                                  uint32 *pulDatabufLen,
                                  FileSourceMetaDataType ienumData);
 private:
  MKAV_PARSER_STATE         m_eParserState;
  MKAV_PARSER_STATE         m_eParserPrvState;

  uint64                    m_nCurrOffset;
  uint64                    m_nFileSize;
  uint64                    m_nSegmentPosn;//absolute file offset where segment data starts
  uint64                    m_nClipDuration;
  uint64                    m_nCurrCluster;
  uint64                    m_nSizeReq;//valid only in case of http to handle under run..

  uint8                     m_nCodecHdrToSend;
  uint8                     m_nstreams;
  uint8                     m_nAudioStreams;
  uint8                     m_nVideoStreams;

  uint8*                    m_pTempBuffer;
  uint8*                    m_pDataBuffer;

  uint32                    m_nMetaData;
  uint32                    m_nCodecHdrSizes[MKAV_VORBIS_CODEC_HDRS];
  uint32                    m_nTrackEntry;
  uint32                    m_nTempBuffSize;

  void*                     m_pUserData;
  bool                      m_bPlayAudio;
  bool                      m_bHttpPlay;
  bool                      m_bEndOfData;
  bool                      m_bIsWebm;

  ebml_doc_hdr*             m_pEBMlDocHdr;
  mkav_track_entry_info*    m_pTrackEntry;
  seekheadinfo*             m_pSeekHeadInfo;
  all_clusters_info*        m_pAllClustersInfo;
  cluster_info*             m_pCurrCluster;
  segment_element_info*     m_pSegmentElementInfo;
  segment_info*             m_pSegmentInfo;
  mkav_stream_sample_info*  m_pSampleInfo;
  mkav_vfw_info*            m_pVFWHeader;
  all_cues_info*            m_pAllCuesInfo;
  seek_table_info           m_SeekTableInfo;

  ZArray<tag_info_type *> TagInfoArray;
  uint32 m_nTagInfoCount;

  //Current output mode
  FileSourceConfigItemEnum  m_eFrameOutputModeEnum;
  FileSourceConfigItemEnum  m_eHeaderOutputModeEnum;

  bool                      EnsureDataCanBeRead(uint64, uint64);
  bool                      IsMetaDataParsingDone();
  bool                      MapFileOffsetToCluster(uint64  ullOffset,
                                                   uint32* pulClusterNo,
                                                   uint8*  pucHdrSize);

  void                      MapMKAVCodecID(uint8*,uint8,mkav_track_entry_info*);
  void                      PrepareAVCCodecInfo(mkav_avc1_info*);
  void                      FreeUpSegmentInfoMemory(segment_info*);
  void                      ResetCurrentClusterInfo();
  void                      InitData();
  void                      UpdateClustersInfoFromSEEKHeads();
  void                      FreeClustersInfo();

  uint32                    GetDataFromSource(uint64, uint32,
                                              unsigned char*, uint32);
  uint64                    DoWeNeedToParseMoreSeekHead();

  MKAV_API_STATUS           ParseEBMLDocHeader(uint64,uint64);
  MKAV_API_STATUS           ParseSegmentElement(uint64, uint64,
                                                bool bupdateoffset=false);
  MKAV_API_STATUS           ParseSegmentInfo(uint64,uint64);
  MKAV_API_STATUS           ParseTracksElement(uint64,uint64);
  MKAV_API_STATUS           ParseSeekHeadElement(uint64,uint64);
  MKAV_API_STATUS           ParseSeekElement(uint64,uint64,seekhead*);
  MKAV_API_STATUS           ParseTrackEntryElement(uint64,uint64);
  MKAV_API_STATUS           ParseClusterElement(uint64,uint64,
                                                cluster_info* pcluster= NULL,
                                                uint8 nhdrSize=0);
  MKAV_API_STATUS           ParseBlockGroupElement(uint64,uint64,cluster_info*);
  MKAV_API_STATUS           ParseBlockElement(uint64,uint64,cluster_info*);
  MKAV_API_STATUS           ParseVideoInfo(uint8 *pDataBuffer, uint32 ullSize,
                                           mkav_video_info* pVideoinfo);
  MKAV_API_STATUS           ParseAudioInfo(uint8 *pDataBuffer,uint64 ullAtomSize,
                                           mkav_audio_info* pAudioInfo);
  MKAV_API_STATUS           ParseContentEncodeInfo(uint8 *pDataBuffer,
                                                uint64 ullElementSize,
                                                mkav_encode_info* pEncodeInfo);

  MKAV_API_STATUS           ParseCuesInfo(uint64,uint64);
  MKAV_API_STATUS           ParseCuePoint(uint64,uint64,cue_point_info*);
  MKAV_API_STATUS           ParseCueTrackPosnInfo(
                                          uint8 *pDataBuffer,
                                          uint64 ullElementSize,
                                          cue_track_posn_info* pCurrTrPosInfo);
  MKAV_API_STATUS           ParseCueRefInfo(uint8 *pDataBuffer,
                                            uint64 ullElementSize,
                                            cue_ref_info* pCueRefInfo);

  MKAV_API_STATUS           ParseTagsElement(uint64,uint64);
  MKAV_API_STATUS           ParseTagElement(uint64,uint64);
  MKAV_API_STATUS           ReadFrames(uint32 ulTrackId,
                                       uint8 *pucDataBuf,
                                       uint32 *pulBufferSize,
                                       mkav_stream_sample_info *pSampleInfo,
                                       bool   &bIsFrameAvailable);
  void                      UpdateSampleProperties(uint32 ulTrackId,
                                          uint32 ulFrameDurinMS,
                                          uint32 ulBufferSize,
                                          mkav_stream_sample_info *pSampleInfo,
                                          blockinfo       *pBlockInfo,
                                          bool   &rbIsFrameAvailable);
  MKAV_API_STATUS           ParseNextClusterHeader(uint64 &ullClusterOffset,
                                                   uint64 &ullClusterSize);
  bool                      GetOffsetFromSeekHead(uint32 ulElementId,
                                                  uint64 &ullOffset);
  MKAV_API_STATUS           CalcFrameSizes(blockinfo* pBlock,
                                           uint64  ullOffset,
                                           uint32  ulDataRead,
                                           uint64  ullElemSize,
                                           uint32& rulIndex);
  MKAV_API_STATUS           UpdateCodecDetails();

  /*! =========================================================================
  @brief  Prepares codec config data for HEVC codec

  @detail  Reads codec private data atom from hevc track and prepares codec
           config data in the form of sequence of NALs

  @param[in/out]
   pucCodecPvt         : Codec private data pointer.
   ulCodecSize         : Private data size
   pulNALULenMinusOne  : Pointer to store NAL Unit Size
   pHEVCInfo           : Buffer to store HEVC Codec Config data

  @return    Total size for codec config data
  @note      None.
  ========================================================================== */
  uint32                 PrepareHEVCCodecInfo(uint8* pucCodecPvt,
                                              uint32 ulCodecSize,
                                              uint32* pulNALULenMinusOne,
                                              uint8*  pHEVCInfo = NULL);
  #ifndef FEATURE_DIVXHD_PLUS
  /* ==========================================================================
  @brief  Function to check the tools used to generate content.

  @details    This function is used to check whether DivxPlus tool has been
              used to generate test content or not.
              If this tool is used and customer do not have Certification then
              the content will be marked as unsupported.

  @param[in]  None.

  @return     None.
  @note       None.
  ========================================================================== */
  void                      IsDivxEnabled();
  #endif
  /*! ======================================================================
  @brief  Repositions given track to ZERO

  @detail  Update track position to start of the file

  @param[in]
   pSampleInfo: Sample Info to be filled in
   trackid:     Identifies the track to be repositioned.

  @return    MKAV_API_SUCCESS if successful other wise returns MKAV_API_FAIL
  @note      None.
  ========================================================================== */
  MKAV_API_STATUS           SeekToZERO(mkav_stream_sample_info *pSampleInfo,
                                       uint32 ulTrackID);

  /*! ======================================================================
  @brief  Updates the Sample Info properties

  @detail  Updates the sample info properties with given seek point

  @param[in]
   ulTrackID:     Identifies the track to be repositioned.
   pSampleInfo:   Sample Info to be updated
   pCuePointInfo: Cue point info structure

  @return    MKAV_API_SUCCESS if successful other wise returns MKAV_API_FAIL
  @note      None.
========================================================================== */
  MKAV_API_STATUS           UpdateSeekSampleProperties(
                                       uint32 ulTrackID,
                                       mkav_stream_sample_info *pSampleInfo,
                                       cue_point_info* pCuePointInfo);

  /*! ======================================================================
  @brief  Updates the Sample Offset by using block num info

  @detail  Updates the offset value near to the block number mentioned in
           Cue point structure.

  @param[in]
  pCuePoint:      Cue point info structure
  ulTrackID:      Identifies the track to be repositioned.

  @return    true if successful other wise returns false.
  @note      None.
  ========================================================================== */
  bool UpdateSampleOffset(cue_point_info* pCuePoint, uint32 ulTrackId);

  /*! ======================================================================
  @brief  Seek to the sync sample using Dynamic seek table.

  @detail  If seek table is filled then by using seek table, Parser seeks to
           the closest sync sample. If seek table is not filled or entries in
           seek table are far apart from the desired timestamp, then Parser
           searches clusters linearly and seeks to the closest sync sample.

  @param[in]
   nReposTime:    Reposition timestamp value.
   ulTrackID:     Identifies the track to be repositioned.
   bForward:      Seek direction
   bIsClosestSeek: Flag to indicate whether closest seek is required or not
   pSampleInfo:   Sample Info to be updated

  @return    MKAV_API_SUCCESS if successful other wise returns error
  @note      None.
  ========================================================================== */
  MKAV_API_STATUS SeekUsingDynamicSeekTable(uint64 nReposTime,
                                            uint32 ulTrackId,
                                            bool bForward,
                                            bool bIsClosestSeek,
                                         mkav_stream_sample_info *pSampleInfo);
/* ============================================================================
  @brief  Update Seek Table

  @details        This function is used to update internal seek table.

  @param[in]      pBlockGroup   Block Group Info Structure Ptr.
  @param[in]      pBlockInfo    Block Info Structure Ptr.

  @return     Nothing.
  @note       This function will be called only if CUES are not available.
============================================================================ */
void UpdateSeekTable(blockgroup_info *pBlockGroup, blockinfo *pBlockInfo);

/* ============================================================================
  @brief  Find Nearest Cluster to the seek time requested.

  @details        This function is used to find the nearest cluster based on
                  the seek time requested.

  @param[in]      ulTrackID       TrackID for which seek request came.
  @param[in]      ullReposTime    Seek time requested.
  @param[in]      bForward        Flag to check whether fwd/backward seek.
  @param[in/out]  pullOffset      Parameter to store nearest cluster offset.

  @return     Nothing.
  @note       This function will be called only if CUES are not available.
============================================================================ */
MKAV_API_STATUS FindNearestCluster(uint32 ulTrackId, uint64 ullReposTime,
                                   bool   bForward,  uint64 *pullOffset);

/* ============================================================================
  @brief  Find Nearest sync sample.

  @details        This function is used to find the nearest sync sample based
                  on the seek time requested.

  @param[in]      ulTrackID       TrackID for which seek request came.
  @param[in]      ullPrevOffset   Current cluster offset.
  @param[in]      ullReposTime    Reposition time.
  @param[in]      bForward        Flag to check whether fwd/backward seek.
  @param[in]      bIsClosestSeek  Closest seek flag.
  @param[in/out]  pSampleInfo     Structure to store sample properties.

  @return     Nothing.
  @note       This function will be called only if CUES are not available.
============================================================================ */
MKAV_API_STATUS FindNearestSyncSample(uint32 ulTrackId, uint64 ullPrevOffset,
                                      uint64 ullReposTime, bool   bForward,
                                      bool bIsClosestSeek,
                                      mkav_stream_sample_info* pSampleInfo);
};
#endif//#ifndef _MKAV_PARSER_H
#endif //#ifdef FEATURE_FILESOURCE_MKV_PARSER

