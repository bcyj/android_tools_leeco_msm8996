#ifndef _MP2STREAM_PARSER_H
#define _MP2STREAM_PARSER_H
/* =======================================================================
                              MP2StreamParser.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2009-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/inc/MP2StreamParser.h#91 $
========================================================================== */

#include "SectionHeaderParser.h"
#include "MP2StreamParserStatus.h"
#include "MP2StreamParserDataDefn.h"
#include "MMDebugMsg.h"
#include "parserdatadef.h"
#include "filesourcetypes.h"
#include "filebase.h"

#define DEFAULT_AUDIO_BUFF_SIZE    64000
#define DEFAULT_VIDEO_BUFF_SIZE    316000
#define MPEG2_FILE_READ_CHUNK_SIZE 188000
#define WFD_MIN_TS_PACKETS_PARSED  100
#define SEEK_JUMP_INTERVAL         1000
//#define MPEG2_PARSER_DEBUG

/*
*Callback function used by parser for reading the data.
*Parser does not implement this function, it's the responsibility
*of the APP to implement it.
*/

extern uint32 MP2StreamCallbakGetData (uint64 ullOffset,
                                       uint32 ulNumBytesRequest,
                                       uint8* pData,
                                       uint32 ulMaxBufSize,
                                       void*  pUserData );
class MP2StreamParser
{
  /*=======================================================================
  * Public Members                                                        *
  * ======================================================================*/
  public:
                     MP2StreamParser(void* pUData,uint64 fsize,
                         bool blocatecodechdr,bool bHttpStreaming = false,
                         FileSourceFileFormat format= FILE_SOURCE_UNKNOWN);
   virtual           ~MP2StreamParser();
   /*! =======================================================================
   @brief         Returns current offset from m_nCurrOffset(private)
   @return        value of m_nCurrOffset
   ==========================================================================  */
   uint64            GetCurrOffset()
   {
     return m_nCurrOffset;
   }
   /*! =======================================================================
   @brief         Set current value of m_nCurrOffset
   ==========================================================================  */
   void              SetCurrOffset(uint64 ullCurrOffset)
   {
     m_nCurrOffset = ullCurrOffset;
   }
   uint32            GetTotalNumberOfTracks(void);
   uint32            GetTotalNumberOfAudioTracks(void);
   uint32            GetTotalNumberOfVideoTracks(void);
   MP2StreamStatus   StartParsing(void);
   uint32            GetTrackWholeIDList(uint32*);
   MP2StreamStatus   GetTrackType(uint32,track_type*,media_codec_type*);
   MP2StreamStatus   GetCurrentSample(uint32 trackId,uint8* dataBuffer,
                                      uint32 nMaxBufSize, int32* nBytesRead,
                                      float *frameTS, uint32 *pulIsSync = NULL);
   MP2StreamStatus   GetAssembledPESPacket(uint32 trackId,uint8* dataBuffer,
                                           uint32 nMaxBufSize, int32* nBytesRead,
                                           float *frameTS, uint32 *pulisSync = NULL);
   MP2StreamStatus   GetSampleAtFrameBoundary(uint32 trackId,uint8* dataBuffer,
                                              uint32 nMaxBufSize, int32* nBytesRead,float *frameTS);
   MP2StreamStatus   GetProgStreamSample(uint32 trackId,uint8* dataBuffer,
                                         uint32 nMaxBufSize, int32* nBytesRead,float *frameTS);
   double            GetPTSFromCurrentPESPacket() {return m_currPESPkt.pts;};
   uint64            GetSampleOffset(void){return m_nCurrSampleOffset;};
   uint64            GetClipDurationInMsec();
   uint32            GetVideoWidth(uint32);
   uint32            GetVideoHeight(uint32);
   uint32            GetAudioSamplingFrequency(uint32);
   uint8             GetNumberOfAudioChannels(uint32);
   /*!===========================================================================
    @brief      Get Audio/Video/Text stream parameter

    @details    This function is used to get Audio/Video stream parameter i.e.
                codec configuration, profile, level from specific parser.

    @param[in]  ulTrackId           TrackID of media
    @param[in]  ulParamIndex        Parameter Index of the structure to be
                                    filled.It is from the FS_MEDIA_INDEXTYPE
                                    enumeration.
    @param[in]  pParameterStructure Pointer to client allocated structure to
                                    be filled by the underlying parser.

    @return     PARSER_ErrorNone in case of success otherwise Error.
    @note
   ============================================================================*/
   PARSER_ERRORTYPE  GetStreamParameter( uint32 ulTrackId,
                                        uint32 ulParamIndex,
                                        void* pParamStruct);
   PARSER_ERRORTYPE  SetStreamParameter(uint32 ulTrackId,
                                        uint32 ulParamIndex,
                                        void* pParamStruct);
   uint8             GetLayer(uint32 trackid);
   uint8             GetVersion(uint32 trackid);
   media_codec_type  GetAudioCodec(uint32 trackid);
   uint32            GetTrackAverageBitRate(uint32);
   float             GetVideoFrameRate(uint32);
   uint32            GetAACAudioProfile(uint32);
   uint32            GetAACAudioFormat(uint32);
   /* ======================================================================
   FUNCTION:
  MP2StreamParser::GetAudioInfo

   DESCRIPTION:
    Copies Audio info structure info if available into given o/p param.

   INPUT/OUTPUT PARAMETERS:
    @param[in]  ulTrackId TrackID of media
    @param[in]  psAACInfo Structure pointer to be filled by the parser

   RETURN VALUE:
    none

   SIDE EFFECTS:
    None.
   ======================================================================*/
   bool              GetAudioInfo(uint32 ulTrackID, audio_info *psAACInfo);
  /*! ======================================================================
   @brief   Get AAC Codec Info

   @detail  Retrieve AAC codec information in PS/TS

   @param[in]
            ulTrackid: Identifies the track to be repositioned.
            psAACInfo: AAC information struct pointer
   @return  TRUE if successful other wise returns FALSE
   @note      None.
   ========================================================================== */
   bool              GetAACAudioInfo(uint32 ulTrackID, aac_audio_info *psAACInfo);
   MP2StreamStatus   Seek(uint32, uint64,
                          uint64, mp2_stream_sample_info*, bool,
                          bool canSyncToNonKeyFrame=false,
                          int  nSyncFramesToSkip = 0);
   /*! ======================================================================
   @brief  Repositions given track to specified time

   @detail    Seeks given track in forward/backward direction to specified time
              in Program Stream

   @param[in]3
   trackid: Identifies the track to be repositioned.
   nReposTime:Target time to seek to
   nCurrPlayTime: Current playback time
   sample_info: Sample Info to be filled in if seek is successful
   canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
   nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

   @return    MP2STREAM_SUCCESS if successful other wise returns MP2STREAM_FAIL
   @note      None.
   ========================================================================== */
   MP2StreamStatus   SeekInProgramStream(uint32 ultrackid,
                        uint64 ullReposTime,
                        uint64 ullCurrPlayTime,
                        mp2_stream_sample_info* sample_info,
                        bool bForward,
                        bool bcanSyncToNonKeyFrame,
                        int  nSyncFramesToSkip);
   /*! ======================================================================
   @brief  Repositions given track to specified time

   @detail    Seeks given track in forward/backward direction to specified time
              in Transport Stream

   @param[in]3
   trackid: Identifies the track to be repositioned.
   nReposTime:Target time to seek to
   nCurrPlayTime: Current playback time
   sample_info: Sample Info to be filled in if seek is successful
   canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
   nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

   @return    MP2STREAM_SUCCESS if successful other wise returns MP2STREAM_FAIL
   @note      None.
   ========================================================================== */
   MP2StreamStatus   SeekInTransportStream(uint32 ultrackid,
                        uint64 lluReposTime,
                        uint64 lluCurrPlayTime,
                        mp2_stream_sample_info* sample_info,
                        bool bForward,
                        bool bcanSyncToNonKeyFrame,
                        int  nSyncFramesToSkip);
   /*! ======================================================================
   @brief  Repositions given track to specified time

   @detail    Seeks given track in backward direction to specified time
              in Transport Stream

   @param[in]3
   trackid: Identifies the track to be repositioned.
   nReposTime:Target time to seek to
   nCurrPlayTime: Current playback time
   sample_info: Sample Info to be filled in if seek is successful
   canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
   nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.
   eTrackType: Track type (Video/Audio)
   eMediaType: (H264/MPG2)
   @return    MP2STREAM_SUCCESS if successful other wise returns MP2STREAM_FAIL
   @note      None.
   ========================================================================== */
   MP2StreamStatus   TSSeekBackwards(uint32 ultrackid,
                        uint64 ullReposTime,
                        uint64 ullCurrPlayTime,
                        mp2_stream_sample_info* sample_info,
                        bool bForward,
                        bool bcanSyncToNonKeyFrame,
                        int  nSyncFramesToSkip,
                        track_type eTrackType,
                        media_codec_type eMediaType);
   /*! ======================================================================
   @brief  Repositions given track to specified time

   @detail    Seeks given track in forward direction to specified time
              in Transport Stream

   @param[in]3
   trackid: Identifies the track to be repositioned.
   nReposTime:Target time to seek to
   nCurrPlayTime: Current playback time
   sample_info: Sample Info to be filled in if seek is successful
   canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
   nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.
   eTrackType: Track type (Video/Audio)
   eMediaType: (H264/MPG2)
   @return    MP2STREAM_SUCCESS if successful other wise returns MP2STREAM_FAIL
   @note      None.
   ========================================================================== */
   MP2StreamStatus   TSSeekForward(uint32 ultrackid,
                        uint64 ullReposTime,
                        uint64 ullCurrPlayTime,
                        mp2_stream_sample_info* sample_info,
                        bool bForward,
                        bool bcanSyncToNonKeyFrame,
                        int  nSyncFramesToSkip,
                        track_type eTrackType,
                        media_codec_type eMediaType);
   /*! ======================================================================
   @brief  Checks if current frame is I Frame or not

   @detail Checks if current frame is I Frame or not
           for MPG2 and H264 codecs
   @param[in]3
   foundFrameType: Variable to return if a valid frame was found or not
                   True means valid frame was found and false means valid
                   frame not found
   eMediaType:     Specifies Codec type (H264/MPG2)
   @return    MP2STREAM_SUCCESS if I Frame is found other wise returns MP2STREAM_FAIL
   @note      None.
   ========================================================================== */
   MP2StreamStatus  isKeyFrame(media_codec_type eMediaType, bool *foundFrameType);
   MP2StreamStatus  SkipNSyncSamples(uint32 trackid, mp2_stream_sample_info* sample_info,
                          int  nSyncFramesToSkip = 0);
   MP2StreamStatus  GetTrackDecoderSpecificInfoContent(uint32 id,uint8*,uint32*);
   virtual uint64   GetLastRetrievedSampleOffset(uint32);
   virtual bool     GetBaseTime(uint32 trackid, double* nBaseTime);
   virtual bool     SetBaseTime(uint32 trackid, double nBaseTime);
   void             SetAudioSeekRefPTS(double audioSeekRefPTS){m_nAudioSeekRefPTS = (double)audioSeekRefPTS;};
   void             SetMediaAbortFlag(void) { m_bMediaAbort = true;};
   MP2StreamStatus  GetFileFormat(FileSourceFileFormat& fileFormat);
   uint32           GetBytesLost(void) {return m_nBytesLost;};
   void             SetEOFFlag(uint64 ullOffset) {m_availOffset = ullOffset;m_bEOFFound = true;};
   virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum);
   virtual FileSourceStatus GetAudioOutputMode(bool*,FileSourceConfigItemEnum);
   virtual FileSourceStatus SetConfiguration(FileSourceConfigItemEnum);
   bool             GetPesPvtData(uint32 trackId, uint8* pPvtData);
   virtual bool     IsDRMProtection();
   FileSourceStatus GetRecentPCR(uint32 ulTrackId, uint64 *pullPCRValue);
/*! =======================================================================
   @brief    Provides the closes PTS from a PES packet in the
             backward direction from a given offset.

   @param[in]      nTrackID  The track ID.
   @param[in]      ullAvailOffset  Buffered/Downloaded bytes.
   @param[out]     Derived timestamp
   @return         FILE_SOURCE_SUCCESS if successful in retrieving
                   buffered duration else returns FILE_SOURCE_FAIL.

   @note    It is expected that user has received the successul
            callback for OpenFile() earlier. In case of local file
            playback, value returned is same as track duration.
   ==========================================================================  */
   MP2StreamStatus GetPTSFromLastPES(uint32 trackId, uint64 ullAvailOffset,
       uint64 *pDuration, uint64 *pullEndOffset=NULL);
   virtual bool     IsMPeg1Video(){return m_sContext.bMPEG1Video;};

   /*! ======================================================================
   @brief  Get Available TimeStamp from the first PES packet in FWD direction

   @detail    Starts parsing from the position specified for a PES packet with
              non-zero PTS value
   @param[in] trackId Identifies elementary stream to demultiplex from
                   current pes packet
   @param[in] ullStartPos Absolute offset from where to start looking for
                       PES packet
   @param[out] ullStartOffset Start offset of PES packet
   @param[out] ullEndOffset   End offset of PES packet
   @param[out] *ullTimeStamp TS from PES packet

   @return    Void.
   @note      None.
   ========================================================================== */
   void GetPTSFromNextPES(uint32 ulTrackId,uint64 ullStartPos,
       uint64 *ullStartOffset, uint64 *ullEndOffset, uint64 *ullTimeStamp);

   //! Context to store/maintain current TS/PES Packet properties.
   //! It also maintains various stream properties.
   //! The flag in context indicates whether all metadata parsed successfully.
   MP2ParserContext m_sContext;
   PESPacket                 m_currPESPkt;
   MP2TransportStreamPkt     m_currTSPkt;
   pack_header               m_currPackHeader;
  private:

  //! Functions to parse Video Metadata
  bool                      findFrameTypeMPEG4(uint8* picType);
  bool                      findVC1FrameStartCode(uint8* pucPicType);
  bool                      findH264NALTypeForFrame(uint8* nalType);
  bool                      findPicCodingTypeForFrame(uint8* picType);

  MP2StreamStatus           ParseVideoPESPacket(MP2ParserContext* pContext);
  MP2StreamStatus           ReadTSPacket();
  MP2StreamStatus           ParsePayloadHeader();
  MP2StreamStatus           parsePCIPacket(uint64,uint32);
  MP2StreamStatus           parseDSIPacket(uint64,uint32);

  MP2StreamStatus           parsePackHeader(uint64&, bool bParseMultiplePESPkts = true,
                                            uint32 trackId = 0xff,uint8* dataBuffer=NULL,
                                            uint32 nMaxBufSize = 0, int32* nBytesRead = NULL
                                            );

  MP2StreamStatus           parsePESPacket(uint64&,uint32,uint32 trackId = 0xff,uint8* dataBuffer=NULL,
                                           uint32 nMaxBufSize = 0, int32* nBytesRead = NULL);

  MP2StreamStatus           parseElementaryStream(uint64&,uint32,
                                                  track_type strmType,
                                                  uint8* dataBuffer=NULL,
                                                  uint32 nMaxBufSize = 0,
                                                  int32* nBytesRead = NULL);

  MP2StreamStatus           parseSystemTargetHeader(uint32, uint64&);
  MP2StreamStatus           parseProgStreamMap(uint64& nOffset);
  MP2StreamStatus           parseProgStream();
  uint32                    readMpeg2StreamData (uint64 nOffset, uint32 nNumBytesRequest, unsigned char* pData,
                                       uint32  nMaxSize, void* pUserData );
  uint8                     getNumberOfStreamsFromTargetHeader(int,uint64);

  bool                      isInitialParsingDone();
  bool                      getLastPTS();

  bool                      isSameStream(uint32* trackId, uint8* newPayload);
  MP2StreamStatus           MakeAccessUnit(uint32 trackId, uint8* dataBuffer, uint32 totalBytes);
  bool                      isAssembledAtFrameBoundary(uint32 trackId, uint8* buf, uint32* dataLen, uint32 maxBufSize);
  bool                      getPidForTrackId(uint32 trackId, uint16* pid);
  MP2StreamStatus           LocateAudioFrameBoundary(uint8* buf, uint32* frame_len, float* frame_time, int* index, uint32 dataLen);
  void                      correctTSDiscontinuity(uint32 trackId);
  float                     getSampleDelta(uint32 trackId);
  float                     getADTSTimestamp(float pesPTS, float curFrameDuration);
  float                     getPSTimestamp(float pesPTS, float curFrameDuration);
  int                       getContinuityCounterJump(uint8 presentCounter);
  MP2StreamStatus           backupInUnderrunBuffer(uint8* dataBuffer, uint32 bytesToCopy, uint32 nPESLen);
  uint32                    restoreFromUnderrunBuffer(uint8* dataBuffer, uint32* pPESLen);
  MP2StreamStatus           scanTSPacketToSeek(uint64* pcr, bool* bPCRFound, bool bForward);
  MP2StreamStatus           parsePTS(uint64 offset,uint8 ptsFlags,uint64* pts,uint64* dts = NULL);
  void                      updateOffsetToNextPacket(uint64 ullStartOffset, bool bIsBDMVFormat = false,
                                                     bool bForward = true);
  MP2StreamStatus           updateTotalTracks(uint32 streamType, uint16 pid);
  void                      GetClipDurationFromPTS(uint32);
  void                      TSGetClipDurationFromPTS(uint32);
  //Private data members used by MP2 parser

  static double             m_nRefAfterDisc; /* used when discontinuity occurs */
  static int                m_nTimesRefUsed; /* used when discontinuity occurs */

  MP2StreamStatus           m_eParserState;
  uint64                    m_nCurrOffset;
  uint64                    m_nFileSize;
  uint64                    m_nEndPESPktTS;
  uint64                    m_nClipDuration;
  uint64                    m_nClipStartTime;
  void*                     m_pUserData;
  bool                      m_bProgramStream;

  uint32                    m_nDataBufferSize;
  uint8*                    m_pDataBuffer;
  uint8*                    m_pReadBuffer;
  uint64                    m_ullCurrentEnd;
  uint64                    m_ullCurrentStart;
  double                    m_nTotalADTSTrackDuration;
  double                    m_nTotalProgStreamDuration;
  double                    m_nPrevPESTime;

  uint32                    m_nBytesLost;

  uint64                    m_nCurrSampleOffset;

  uint8                     m_nInitialPacksParsed;
  int                       m_nInitialTSPktParsed;

  double                    m_nFirstAudioPTS; //first pts for current segment
  double                    m_nAudioSeekRefPTS; //ref pts after seek has been performed

  bool                      m_bpartialPESTS;
  bool                      m_bHttpStreaming;
  bool                      m_bLocateCodecHdr;
  bool                      m_bEOFFound; //bool used for streaming only
  bool                      m_bMediaAbort;
  int                       m_nPrevCC;
  uint64                    m_availOffset;

  DescriptionSection        m_DescSection;
  TSStreamSection           m_TSStreamSection;
  mp2_stream_sample_info    m_sampleInfo;
  pci_pkt*                  m_pFirstVOBUPCIPkt;
  pci_pkt*                  m_pCurrVOBUPCIPkt;
  dsi_pkt*                  m_pFirstVOBUDSIPkt;
  dsi_pkt*                  m_pCurrVOBUDSIPkt;
  partial_frame_data*       m_pPartialFrameData;
  underrun_frame_data       m_UnderrunBuffer; //Our buffer to store collected data in case of underrun during sample
                                              //collection so that we can restore during next getNextSample call
  FileSourceConfigItemEnum m_hFrameOutputModeEnum;
  FileSourceConfigItemEnum m_hHeaderOutputModeEnum;
  FileSourceConfigItemEnum m_hTSDiscCorrectModeEnum;

  FileSourceFileFormat     m_eFileFormat;
#ifdef ATSC_COMPLIANCE
  EAC3AudioDescriptor*     m_pEAC3AudioDescriptor;
#endif
};
#endif
