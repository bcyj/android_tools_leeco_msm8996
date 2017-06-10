#ifndef _OGGSTREAM_PARSER_H
#define _OGGSTREAM_PARSER_H
/* =======================================================================
                              OGGStreamParser.h
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/OGGParserLib/main/latest/inc/OGGStreamParser.h#19 $
========================================================================== */
#include "OGGStreamParserConstants.h"
#include "OGGStreamParserStatus.h"
#include "OGGStreamParserDataDefn.h"
#include "MMDebugMsg.h"
#include "parserinternaldefs.h"
#include "filebase.h"

#ifdef FEATURE_FILESOURCE_FLAC_PARSER
#include "FlacParser.h"
#endif

#define FEATURE_OGGPARSER_FAST_START_UP
//#define OGG_PARSER_DEBUG
#define  FEATURE_OGGPARSER_FLAC_FAST_START_UP
#define  OGG_PARSER_APPROXIMATE_TIMESTAMPS

#ifndef FEATURE_OGGPARSER_FAST_START_UP
#define FEATURE_OGGPARSER_BUILD_SEEK_INDEX
#endif

#ifdef FEATURE_FILESOURCE_OGG_PARSER
/*
*Callback function used by parser for reading the data.
*Parser does not implement this function, it's the responsibility
*of the APP to implement it.
*/

extern uint32 OGGStreamCallbakGetData (uint64 nOffset,
                                       uint32 nNumBytesRequest,
                                       unsigned char* pData,
                                       uint32  nMaxSize,
                                       void*   pPtr );
class OGGStreamParser
{
  /*=======================================================================
  * Public Members                                                        *
  * ======================================================================*/
  public:
                        OGGStreamParser(void* pUData,uint64 fsize,bool bAudio);
                        ~OGGStreamParser();
   OGGStreamStatus      StartParsing(void);
   OGGStreamStatus      GetCurrentSample(uint32 trackId,uint8* dataBuffer,
                                         uint32 nMaxBufSize, int32* nBytesRead);
   OGGStreamStatus      GetCurrentSampleTimeStamp(uint32,uint64*, uint64*, bool*);

   /*
   * Parameter details:
   * uint32 Track Id
   * uint64 Requested reposition time
   * uint32 Current Playback Time
   * ogg_stream_sample_info* SampleInfo filled in if seek is successful
   * bool Specifies if need to do fwd or rwd
   * bool Specifies if it's ok tp sync to non key frame
   * int  Specifies num,ber of frames to skip in fwd/backward direction
   */
   OGGStreamStatus      Seek(uint32, uint64, uint64, ogg_stream_sample_info*,
                             bool, bool canSyncToNonKeyFrame=false,
                             int  nSyncFramesToSkip = 0);

   uint32               GetTrackWholeIDList(uint32*);
   uint8*               GetCodecHeader(uint32);
   uint32               GetCodecHeaderSize(uint32);

   uint32               GetTrackMaxBufferSize(uint32);

   uint64               GetClipDurationInMsec();
   uint32               GetVideoWidth(uint32);
   uint32               GetVideoHeight(uint32);
   uint32               GetTrackAverageBitRate(uint32);
   bool                 GetAudioStreamInfo(uint32 ulTrackId,
                                           ogg_audio_info* pAudioInfo);
   uint8                GetTotalNumberOfTracks(void){return m_nstreams;}
   float                GetVideoFrameRate(uint32);
   void                 GetClipMetaData(int,uint8*,uint32*);
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
   OGGStreamStatus      GetFlacStreamInfo(uint32 id,flac_metadata_streaminfo*);
#endif
   ogg_media_codec_type GetTrackType(uint32);
   uint32               GetTrackSerialNo(uint32 ulTrackId);

  private:
  //Functions/data types internal to OGG parser
  OGGStreamStatus       ParseOGGPage(uint64,OggPage*);
  OGGStreamStatus       ParseBOSPage(uint32&,OggPage*);
  OGGStreamStatus       ParseVorbisIdentificationHdr(uint32&,OggPage*);
  OGGStreamStatus       ParseTheoraIdentificationHdr(uint32&,OggPage*);
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
  OGGStreamStatus       ParseFlacIdentificationHdr(uint32&,OggPage*);
#endif
  OGGStreamStatus       ParseSetupHdr(uint32&,ogg_media_codec_type,OggPage*);
  OGGStreamStatus       ParseCommentHdr(uint32&,ogg_media_codec_type);

#ifdef FEATURE_OGGPARSER_BUILD_SEEK_INDEX
   /*
   * Parameter details:
   * uint32 Track Id
   * uint64 Requested reposition time
   * uint32 Current Playback Time
   * ogg_stream_sample_info* SampleInfo filled in if seek is successful
   * bool Specifies if need to do fwd or rwd
   * bool Specifies if it's ok tp sync to non key frame
   * int  Specifies num,ber of frames to skip in fwd/backward direction
   */
  OGGStreamStatus       SearchOGGIndex(uint32, uint64, uint32,
                                       ogg_stream_sample_info*,
                                       bool,
                                       bool canSyncToNonKeyFrame=false,
                                       int  nSyncFramesToSkip = 0);
#endif

#ifdef FEATURE_OGGPARSER_BUILD_SEEK_INDEX
  OGGStreamStatus       IndexOGGPage(OggPage*,ogg_media_codec_type);
  void                  FreeOGGIndexInfo();
#endif

  void                  UpdateGranulePosition(OggPage*);
  bool                  IsMetaDataParsingDone(OggPage*);
  bool                  CheckPageCRC(uint8*, uint32);
  void                  InitCrcComputeTables(void);
  uint32                FindCheckSum(uint8 *, uint32);
  uint64                FindNextPageOffset(uint64);
#if defined (FEATURE_OGGPARSER_FAST_START_UP) || defined(FEATURE_OGGPARSER_FLAC_FAST_START_UP)
  bool                  FindLastValidPage(uint32 trackid, uint64 fileSize, OggPage *pOggPage);
  OGGStreamStatus       ValidateOggPage(uint64 offset,OggPage* pOggPage);
#endif
  OGGStreamStatus       m_eParserState;
  uint64                m_nCurrOffset;
  uint64                m_nFileSize;
  uint64                m_nClipDuration;
  void*                 m_pUserData;
  uint32                m_nOggDataBufferSize;
  uint32                m_nOggDataBufferFillSize;
  uint8*                m_pDataBuffer;
  uint8*                m_pDataCache;
  uint64                m_nDataCacheFileOffset;
  uint32                m_pDataBufferOffset;
  uint8                 m_nstreams;
  uint8                 m_nAudioStreams;
  uint8                 m_nVideoStreams;
  int                   m_nMetaData;
  ogg_audio_info*       m_pOggAudioCodecInfo;
  ogg_video_info*       m_pOggVideoCodecInfo;
  bool                  m_bOggMetaDataParsed;
  bool                  m_bParsedLastPage;
  bool                  m_bPlayAudio;
  bool                  m_bUserInitiatedRW;
  uint64                m_nDataPageOffset;
  OggPage*              m_pOggPage;
  ogg_index*            m_pOggIndex;
  ogg_meta_data*        m_pOggMetaData;
  uint64                m_nStartGranulePosn;  //valid only once playback begins
  uint64                m_nEndGranulePosn;  //valid only once playback begins
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
  FlacParser*           m_pFlacParser;
#endif
  uint32                m_aCRCTable[256];
  uint64                m_nCurrentTimeStampMs;
  bool                  m_bTimeStampValid;

};
#endif //FEATURE_FILESOURCE_OGG_PARSER
#endif//#ifndef _OGGSTREAM_PARSER_H

