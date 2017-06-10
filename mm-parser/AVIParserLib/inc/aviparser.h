#ifndef AVI_PARSER_H
#define AVI_PARSER_H

/* =======================================================================
                              aviparser.h
DESCRIPTION

Copyright 2011-2014 QUALCOMM Technologies Incorporated, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/inc/aviparser.h#38 $
========================================================================== */
#include <stdio.h>
#include <stdlib.h>

#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "aviheaders.h"
#include "avierrors.h"

/*
* When this feature is defined, parser will not load the indexing information during intial parsing.
* Thus, playback will start as quickly as possible.
*
* Undefining this feature will make parser cache in entire idx1 and prepare key-frame table for video
* and indexing table for all audio streams. This will delay the intial parsing and will also
* consume additional memory. Size of memory depends on size of idx1.
*/
//#define AVI_PARSER_FAST_START_UP

/*
* Enabling following feature will print lots of debug messages from
* AVI parser APIs.
*/
//#define FEATURE_FILESOURCE_AVI_PARSER_DEBUG

/*
* When enabled, PARSER will skip parsing IDX1 information.
* This will reduce the start up time as parsing will be done within a second.
* Not recommended to disable this feature as it will increase
* memory requirement by huge when clip is really really long.
*/

//Debug purpose
#define AVI_PARSER_SEEK_SANITY_TEST

/*
*Callback function used by parser for reading the file data.
*Parser does not implement this function, it's the responsibility
*of the APP to implement it.
*/
extern avi_int32 AVICallbakGetData (uint64 ullOffset,
                                    uint32 ulNumBytesRequest,
                                    uint8* pData,
                                    uint32 ulMaxBufSize,
                                    void*  pUserData);

extern avi_int64 AVICheckAvailableData (void* pUserData);
/*
*aviParser class.
*/
class aviParser
{
  /*=======================================================================
  * Public Members                                                        *
  * ======================================================================*/

  public:
  //aviParser APIs
                         aviParser(void* pUserData,
                                   avi_uint64 fsize,
                                   bool bDiscardAudIndex = false,
                                   bool bDiscardVidIndex = false,
                                   bool bHttpStreaming = false);

  virtual                ~aviParser();
  virtual avi_uint64     GetLastOffsetRead(void){return m_nLastOffsetRead;}
  virtual avi_uint64     GetSampleInfoOffset(void){return m_nSampleInfoOffset;}
  virtual avi_uint64     GetAVIHeaderSize(void){return m_nMoviOffset;}
  virtual avi_uint32     GetTotalNumberOfTracks(void){return m_hAviSummary.n_streams;}
  virtual avi_uint32     GetTotalNumberOfAudioTracks(void){return m_hAviSummary.n_audio_tracks;}
  virtual avi_uint32     GetTotalNumberOfVideoTracks(void){return m_hAviSummary.n_video_tracks;}

  virtual aviErrorType   StartParsing(void);
  virtual aviErrorType   GetNumOfRiff(avi_uint64 nOffset, int* m_nNumOfRiff);
  virtual aviErrorType   GetAVIHeader(avi_mainheader_avih* pAviHdrPtr);
  virtual aviErrorType   GetAudioInfo(avi_uint32 trackId,avi_audio_info* pAudioInfo);
  virtual aviErrorType   GetVideoInfo(avi_uint32 trackId,avi_video_info* pVideoInfo);
  virtual avi_uint64     GetTrackDuration(avi_uint32 trackId);
  virtual avi_uint64     GetClipDurationInMsec();

  virtual avi_uint8*     GetAVIVolHeader(avi_uint32   trackId);
  virtual avi_uint32     GetAVIVolHeaderSize(avi_uint32 trackId);

  virtual aviErrorType   GetCurrentSample(avi_uint32 trackId,
                                          avi_uint8* dataBuffer,
                                          avi_uint32 nMaxBufSize,
                                          avi_uint32 nBytesNeeded);

  virtual aviErrorType   GetNextSampleInfo(avi_uint32 trackId,
                                           avi_sample_info* sampleInfo,
                                           avi_uint32 nMaxBufSize,
                                           avi_uint16* trackIdFound);

  virtual aviErrorType   GetSampleInfo(avi_uint64* sampleOffset,
                                       avi_uint32 trackId,
                                       avi_sample_info* sampleInfo,
                                       avi_uint32 nMaxBufSize,
                                       avi_uint16* trackIdFound);

  virtual aviErrorType   GetTrackChunkType(avi_uint32 trackId,CHUNK_t* type);
  virtual aviErrorType   GetAudioTrackSummaryInfo(avi_uint32 trackId,
                                                  avi_audiotrack_summary_info*);
  virtual avi_uint32     GetTrackWholeIDList(avi_uint32*);
  virtual aviErrorType   Seek(avi_uint32,
                              avi_uint64,
                              avi_uint64,
                              avi_sample_info*,
                              bool bSyncToKeyFrame=true,
                              int  nSyncFramesToSkip = 0);
#ifndef AVI_PARSER_FAST_START_UP
  virtual void           SetIDX1Cache(void*);
  virtual void*          GetIDX1Cache();
#endif
  aviErrorType           GetWMAExtraInfo(uint32 id,avi_header_strf_wma_extra*);

  virtual uint64         GetLastRetrievedSampleOffset(uint32 trackId);
  virtual avi_uint8*     GetDRMInfo(int*);
  virtual bool           IsDRMProtection(){return m_bDRMProtection;};
  PARSER_ERRORTYPE       getDataFromInfoChunk(const char*, wchar_t*, avi_uint16*);
  virtual uint8          randomAccessDenied();

  bool                   setParserState(aviParserState,aviErrorType*);

  avi_int32              parserAVICallbakGetData (avi_int64,
                                                  avi_uint32,
                                                  unsigned char*,
                                                  avi_int32,
                                                  void*,
                                                  aviErrorType*);
  uint64                 GetMoviOffset(){return m_nMoviOffset;};

  /*=======================================================================
  * Private Members                                                       *
  * ======================================================================*/

  private:
  //Private members/variables used by parser.

  //User callback data
  void*            m_pUserData;

  //This is the offset in 'movi' used for retrieval of audio/video samples.
  avi_uint64       m_nCurrOffset;

  avi_uint64       m_nLastOffsetRead;

  //Current audio/video sample size to be retrieved.
  avi_uint32       m_nCurrentChunkDataSize;

  //Start/size of 'movi' chunk
  avi_uint64       m_nMoviOffset;
  avi_uint64       m_nStartOfMovi;
  avi_uint64       m_nMoviSize;

  int              m_nNumOfRiff;
  bool             m_bisAVIXpresent;
  avi_riff_info*   m_pMultipleRiff;

  //Start/size of 'idx1' chunk
  avi_uint64       m_nIdx1Offset;
  avi_uint64       m_nIdx1Size;

  //Start of MOVI and IDX1 Offset in case of JUNK at the beginning
  avi_uint64       m_nAdjustedMoviOffset;
  avi_uint64       m_nAdjustedIdx1Offset;

  //Total file size
  avi_uint64       m_nFileSize;

  //Current parser state
  aviParserState   m_CurrentParserState;
  avi_uint8        m_ReadBuffer[AVI_READ_BUFFER_SIZE];

  avi_uint64       m_nSampleInfoOffset;

  //Points to the current sample being retrieved in 'idx1' chunk.
  //Used to seek backward/forward. This eliminates the need to parse
  //IDX1 at the begining during reposition.
  avi_uint64       m_nCurrentSampleInfoOffsetInIdx1;
  avi_uint32       m_nCurrAudioSampleInIdx1;
  avi_uint32       m_nCurrVideoSampleInIdx1;

  avi_summary_info m_hAviSummary;
  avi_uint32       m_nCurrVideoFrameCount[AVI_MAX_VIDEO_TRACKS];
  avi_uint64       m_nCurrAudioPayloadSize[AVI_MAX_AUDIO_TRACKS];
  avi_uint32       m_nCurrAudioFrameCount[AVI_MAX_AUDIO_TRACKS];
  avi_uint64       m_nParserAudSampleEndTime[AVI_MAX_AUDIO_TRACKS];

  bool             m_bDiscardAudioIndex;
  bool             m_bDiscardVideoIndex;
  bool             m_bDRMProtection;
  bool             m_bByteAdjustedForMOVI;
  bool             m_bSeekDenied;
  avi_info_struct  m_AviClipMetaInfo;
  avi_uint32       m_nBytesToBeAdjustedForMOVI;
  avi_uint16       m_VolSize;

  bool             m_bHttpStreaming;

  //Stores AVI Base Index('indx'), if available.
  avi_indx_tbl     m_base_indx_tbl[AVI_MAX_TRACKS];

  memory_struct*   m_AviVOLHeader[AVI_MAX_TRACKS];

  avi_parser_seek_buffer_cache* m_pIdx1SeekCache;

  avi_header_strf_wma_extra*    m_pWmaAudioInfo;

  avi_header_strd  m_AviSTRDHeader;

  //Private functions for parsing various AVI headers.
  aviErrorType     parseAVIH(avi_uint64*,uint32);
  aviErrorType     parseSTRL(avi_uint64*,avi_uint32);
  aviErrorType     parseVideoSTRH(avi_uint64*,avi_uint64,avi_video_info*);
  aviErrorType     parseAudioSTRH(avi_uint64*,avi_uint64);
  aviErrorType     parseAudioSTRF(avi_uint64*,uint32);
  aviErrorType     parseVideoSTRF(avi_uint64*,avi_video_info*,uint32);
  aviErrorType     parseVideoSTRN(avi_uint64*,avi_video_info*,uint32);
  aviErrorType     parseAudioSTRN(avi_uint64*,uint32);
  aviErrorType     parseODML(avi_uint64);
  aviErrorType     parseJUNK(avi_uint64*);
  aviErrorType     parseMOVI(avi_uint64);
  aviErrorType     parseHDRL(avi_uint64*, uint32);
  aviErrorType     parseINFO(avi_uint64 offset,int);
  avi_uint64       skipToNextValidMediaChunk(avi_uint64);
  aviErrorType     parseIDX1(avi_uint64*);
  aviErrorType     cacheIDX1(avi_uint64*,avi_uint32);
  aviErrorType     updateSampleRetrievedOffset(CHUNK_t,avi_uint32);
  aviErrorType     parseINDX(avi_uint64);
  aviErrorType     parseIX(avi_std_ix_tbl*,avi_uint64);
  aviErrorType     getNextFourCC(avi_uint64,avi_uint32*);
  aviErrorType     updateInfoChunkInfo(fourCC_t,avi_uint32,avi_uint64);
  avi_info_chunk*  getInfoChunkHandle(fourCC_t);
  aviErrorType     parseSTRD(avi_uint64*,uint32);
  bool             isVOLReadDone(avi_uint32,avi_uint64,avi_int8*,bool*,avi_uint8* membuf=NULL);

  aviErrorType     searchIDX1Cache(avi_uint32,
                                   avi_uint64,
                                   avi_idx1_entry*,
                                   bool,
                                   CHUNK_t,
                                   int nSyncFramesToSkip,
                                   bool* endOfFileReached);
  aviErrorType     seekInIDX1(avi_uint32,
                              avi_uint64,
                              avi_idx1_entry*,
                              bool,
                              CHUNK_t,
                              int nSyncFramesToSkip,
                              bool* endOfFileReached,
                              bool  bSyncToKeyFrame=true);
  aviErrorType    seekInSuperIndex(avi_uint32 /*ulTrackID*/,
                              avi_uint64 /*ullReposTime*/,
                              avi_idx1_entry* /*outputEntry*/,
                              bool /*bSearchForward*/,
                              CHUNK_t /*chunkType*/,
                              int /*nSyncFramesToSkip*/,
                              bool* /*endOfFileReached*/,
                              bool  bSyncToKeyFrame=true);
  avi_uint32       readFromIdx1SeekCache(avi_uint64 nOffset,
                                         avi_int32 nNumBytesRequest,
                                         avi_int32 nMaxSize,
                                         unsigned char *ppData);

  void             flushIdx1SeekCache(void);
#if defined(AVI_PARSER_SEEK_SANITY_TEST)
  bool             doSanityCheckBeforeSeek(avi_uint32,CHUNK_t,aviParserState);
#endif
  avi_uint64        getCurrentPlaybackTime(avi_uint32);
  avi_uint32        isCurrentFrameEncrypted(avi_uint32/*trackid*/,
                                           avi_uint64/*current offset in idx1*/,
                                           avi_uint64/*current offset in MOVI*/);
  aviErrorType      parseSTRFExtraData(avi_uint8*,uint16);
  avi_int64         getTimeStampFromSubtitle(avi_uint8* buf, CHUNK_t chunkType);
  bool ValidateChunkSize( uint32 ulChildChunkSize , uint32 ulParentChunkSize);
};
#endif
