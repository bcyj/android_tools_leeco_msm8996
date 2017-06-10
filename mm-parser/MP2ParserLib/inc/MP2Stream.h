#ifndef _MP2_STREAM_H
#define _MP2_STREAM_H
/* =======================================================================
                              MP2Stream.h
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
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/inc/MP2Stream.h#43 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================
                    INCLUDE FILES FOR MODULE
========================================================================== */
#include "AEEStdDef.h"
#include "filebase.h"
#include "filesourcestring.h"
#include "oscl_file_io.h"
#include "MP2StreamDataDef.h"
#include "MP2StreamParserStatus.h"
#include "MP2StreamParserConstants.h"

/* ==========================================================================
                       DATA DECLARATIONS
========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class MP2StreamParser;
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Function Declaration
** ======================================================================= */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  MP2Stream

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class MP2Stream : public FileBase, public Parentable
{
public:

  MP2Stream(const FILESOURCE_STRING filename, bool bneedcodechdr,
      unsigned char *pFileBuf = NULL, uint32 bufSize = 0,
      bool bPlayVideo = false, bool bPlayAudio = true,
      FileSourceFileFormat m_FileFormat = FILE_SOURCE_UNKNOWN);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  MP2Stream(video::iStreamPort*,bool bneedcodechdr,
      bool bPlayVideo = false, bool bPlayAudio = true,
      FileSourceFileFormat m_FileFormat=FILE_SOURCE_UNKNOWN);
#endif
  virtual ~MP2Stream();
  virtual uint32 FileGetData(uint64, uint32, uint32, uint8*);
  virtual bool ParseMetaData();

  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat);

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                              uint32 *pulBufSize, uint32 &index);
  virtual MP2StreamStatus getNextADTSAudioSample(uint32 id, uint8 *buf, uint32 size,
                                                 int32 *nBytesRead, uint32 &index,
                                                 float* frameTS);
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);

  virtual uint64 resetPlayback(uint64 repos_time, uint32 id, bool bSetToSyncSample,
                               bool *bError, uint64 currentPosTimeStamp);
#ifdef FEATURE_FILESOURCE_REPOSITION_SYNC_FRAME
  virtual uint64 skipNSyncSamples(int nSyncSamplesToSkip, uint32 id, bool *bError,
                                  uint64 currentPosTimeStamp);
#endif
  virtual uint8 randomAccessDenied(){return 0;}

  // Methods to get the sample rate (i.e. timescales) for the streams and
  // the overall Movie presentation
  virtual uint64 getMovieDuration() const {return getMovieDurationMsec();};
  virtual uint32 getMovieTimescale() const {return MPEG2_STREAM_TIME_SCALE;};
  virtual uint64 getMovieDurationMsec() const;

  virtual bool   getBaseTime(uint32 id, uint64* nBaseTime);
  virtual bool   setBaseTime(uint32 id, uint64 nBaseTime);

  // From Track
  virtual int32 getNumTracks(){return m_nNumStreams;}
  virtual uint32 getTrackWholeIDList(uint32 *ids);

  virtual float  getTrackVideoFrameRate(uint32 id);
  virtual uint32 getTrackVideoFrameWidth(uint32 id);
  virtual uint32 getTrackVideoFrameHeight(uint32 id);

  // From MediaHeader
  virtual uint64 getTrackMediaDuration(uint32 /*id*/)
                {return getMovieDurationMsec();};
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/)
                {return MPEG2_STREAM_TIME_SCALE;};
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);
  virtual uint32 GetAACAudioProfile(uint32 id);
  virtual uint32 GetAACAudioFormat(uint32 id);
  virtual uint32 GetNumAudioChannels(int id);
  virtual uint8  getTrackOTIType(uint32 id); // Based on OTI value
  virtual int32  getTrackAverageBitrate(uint32 id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);

  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid,
                                         file_sample_info_type *pSampleInfo);



  // this returns Sequence (VOL) Header and its size //
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize);

  virtual bool isM2TSFileInstance(){return true;}
  virtual bool isVideoInstance(){return m_playVideo;}
  virtual bool isAudioInstance(){return m_playAudio;}
  virtual bool isTextInstance() {return m_playText;}
  virtual bool IsDRMProtection();
  virtual FileSourceStatus GetDRMType(FileSourceDrmType&);

  virtual uint64 getFileSize(){return m_fileSize;}

  virtual int           GetTotalNumberOfAudioStreams();
  virtual int           GetTotalNumberOfVideoStreams();
  virtual int           GetTotalNumberOfTextStreams();
  virtual uint8         MapTrackIdToIndex(bool*,uint32);

  virtual void          SetCriticalSection(MM_HANDLE);
  virtual uint64        GetLastRetrievedSampleOffset(uint32 trackId);
  virtual int32         GetFileError(){ return _fileErrorCode; }

#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  virtual void          CheckAvailableDataSize(uint64* bufDataSize,boolean* pbEndOfData);
  virtual void          GetBufferLowerBound(uint64* pBufLowerOffset, boolean *pbEndOfData);
#endif
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum);
  virtual FileSourceStatus GetAudioOutputMode(bool*,FileSourceConfigItemEnum);

  virtual FileSourceStatus SetConfiguration(FileSourceConfigItemEnum);
/*! =======================================================================
   @brief         Returns absolute file offset(in bytes) associated
                  with time stamp 'pbtime'(in milliseconds).

   @param[in]     pbtime:Timestamp(in milliseconds) of the sample
                           that user wants to play/seek
                  ulTrackId: The track ID for which API is called
   @param[out]    offset:Absolute file offset(in bytes) corresponding
                  to 'pbtime'

   @return        true if successful in retrieving the absolute
                  offset(in bytes) else returns false

   @note          It is expected that user has received the successul
                  callback for OpenFile() earlier. When there are
                  multiple tracks in a given clip(audio/video/text),
                  API returns minimum absolute offset(in bytes)among
                  audio/video/text tracks. API is valid only for
                  non-fragmented clips.
   ==========================================================================  */
  bool GetOffsetForTime(uint64 ullPBTime, uint64* pullOffset,
    uint32 ulTrackId,  uint64 ullCurrentPosTimeStamp, uint64& pullReposTime);
  /*! =======================================================================
   @brief    Provides the playback time for buffered/downloaded bytes
             of particular track.

   @param[in]     nTrackID  The track ID.
   @param[in]     nBytes    Buffered/Downloaded bytes (Always
                  provide positive integer)
   @param[out]    pDuration Filled in by FileSource to provided
                  playtime for buffered/downloaded bytes
   @return        FILE_SOURCE_SUCCESS if successful in retrieving
                  buffered duration else returns FILE_SOURCE_FAIL.

   @note          It is expected that user has received the successul
                  callback for OpenFile() earlier. In case of local
                  file playback, value returned is same as track
                  duration.
   ==========================================================================  */
  bool getBufferedDuration(uint32 nTrackID, int64 nBytes,
      uint64 *pDuration);
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
  PARSER_ERRORTYPE GetStreamParameter( uint32 ulTrackId,
                                       uint32 ulParamIndex,
                                       void*  pParamStruct);
  PARSER_ERRORTYPE SetStreamParameter( uint32 ulTrackId,
                                       uint32 ulParamIndex,
                                       void*  pParamStruct);

private:

  void                    InitData();
  bool                    m_playAudio;
  bool                    m_playVideo;
  bool                    m_playText;
  bool                    m_corruptFile;

  // if we are streaming, rather than playing locally
  bool                    m_bStreaming;
  bool                    m_bHaveTransportRate;
  uint32                  m_nNumStreams;

  // These variable are added to know the largest size of audio and video samples respectively//
  uint32                  m_audioLargestSize;
  uint32                  m_videoLargestSize;
  uint32                  m_nPrevPESPTS;

  file_sample_info_type   m_sampleInfo[FILE_MAX_MEDIA_STREAMS];
  uint32                  m_nDecodedDataSize[FILE_MAX_MEDIA_STREAMS];
  uint32                  m_nLargestFrame[FILE_MAX_MEDIA_STREAMS];
  TrackIdToIndexTable*    m_pIndTrackIdTable;

  //only one of "m_pFileBuf" or "m_filename" can be non-zero
  FILESOURCE_STRING     m_filename;  // EFS file path //
  unsigned char  *m_pFileBuf;  // pointer to buffer for playback from memory //

  uint32          m_FileBufSize;
  uint64          m_fileSize;
  OSCL_FILE       *m_pFilePtr;
  float           fpTransportRate;

  /*int16  m_nSelectedAudioStreamId;
  int16  m_nSelectedVideoStreamId;
  int16  m_nSelectedTextStreamId;*/

  // MP2 Stream parser handle
  MP2StreamParser*    m_pMP2StreamParser;
  FileSourceFileFormat     m_FileFormat;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pPort;
#endif
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
   uint64  m_minOffsetRequired;
   bool    bHttpStreaming;
   uint64  m_wBufferOffset;
   boolean bGetMetaDataSize;
   boolean bIsMetaDataParsed;
   ParserStatusCode parserState;
   uint32  m_startupTime;

   struct tHttpDataOffset
   {
      uint64  Offset;
      boolean bValid;
   } m_HttpDataBufferMinOffsetRequired;

   virtual void updateBufferWritePtr ( uint64 writeOffset );

   virtual bool parseHTTPStream ( void );

   void sendHTTPStreamUnderrunEvent(void);
   boolean GetHTTPStreamDownLoadedBufferOffset(uint64 *pOffset);
   bool GetTotalAvgBitRate(uint32 * pBitRate);
   uint32 GetNumBytesToStrip(uint32 id, int codec_type);

#endif //FEATURE_QTV_3GPP_PROGRESSIVE_DNLD
};
#endif  /* _MP2_STREAM_H */

