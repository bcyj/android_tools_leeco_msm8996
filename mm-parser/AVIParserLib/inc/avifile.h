#ifndef __AVIFile_H__
#define __AVIFile_H__
/* =======================================================================
                              avifile.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2011-2014 QUALCOMM Technologies Incorporated, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/inc/avifile.h#45 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "oscl_file_io.h"
#include "qcplayer_oscl_utils.h"

#include "filebase.h"
#include "aviheaders.h"
#include "avierrors.h"
#include "aviparser.h"
#include "filesourcetypes.h"
/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

//Need to update this value based on MOBILE PROFILE
#define AVI_STREAM_TIME_SCALE 1000      // AVI Code has Milli Sec interface
#define MP3_MIN_FRAME_HEADER_SIZE 4
#define AVI_MAX_VIDEO_FRAMES_READ_AHEAD 4
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class aviParser;
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
typedef struct video_sample
{
  AVI_VOP_TYPE vop_type;
  uint8 bVopCount;
  uint32 size;
  file_sample_info_type   m_sampleInfo;
  uint8 * buff;
}avi_video_sample;

/* This is available only for AVI MPEG4 */
typedef struct read_ahead_buffer
{
  bool                   firstVideoFrame;  // To know if we are reading the first frame
  uint8                  currentSampleIndex; // current frame that we need to send
  uint8                  validSampleIndex;  // index to read the next video frame from file
  bool                   allocatedForReadAhead;  // TO allocate only once the read ahead buffer to hold frames
}avi_read_ahead_buffer;
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
  AVIFile

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
class AVIFile : public FileBase, public Parentable
{

public:

  AVIFile();
  AVIFile(  const FILESOURCE_STRING &filename
            ,unsigned char *pFileBuf = NULL
            ,uint32 bufSize = 0
            ,bool bPlayVideo = true
            ,bool bPlayAudio = true
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
            ,bool bHttpStream = false
            ,uint64 wBufferOffset = 0
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

           ); // Constructor
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  AVIFile( video::iStreamPort* pPort,
           bool bPlayVideo = false,
           bool bPlayAudio = false);
#endif
  virtual ~AVIFile();

  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_AVI;
    return FILE_SOURCE_SUCCESS;
  }

  virtual bool isHTTPStreaming(void) {return m_bStreaming;};

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                              uint32 *pulBufSize, uint32 &rulIndex);
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);

  virtual void resetPlayback();
  virtual uint64 resetPlayback(uint64 repos_time, uint32 id, bool bSetToSyncSample,
                               bool *bError, uint64 currentPosTimeStamp);
  virtual bool resetMediaPlayback(uint32);  // reset playback for a particular track //

  virtual uint64 skipNSyncSamples(int , uint32 , bool* , uint64 );

  virtual uint8 randomAccessDenied();

  virtual PARSER_ERRORTYPE GetClipMetaData(wchar_t* pMetaData, uint32* pLength,
                                           FileSourceMetaDataType ienumData);

  // Methods to get the sample rate (i.e. timescales) for the streams and
  // the overall Movie presentation
  virtual uint64 getMovieDuration() const;
  virtual uint32 getMovieTimescale() const;
  virtual uint64 getMovieDurationMsec() const;

  // From Track
  virtual int32 getNumTracks(){return m_nNumStreams;}
  virtual uint32 getTrackWholeIDList(uint32 *ids);
  virtual int16 getTrackContentVersion(uint32 id);
  virtual uint8 trackRandomAccessDenied(uint32 id);
  virtual float  getTrackVideoFrameRate(uint32 id);
  virtual uint32 getTrackVideoFrameWidth(uint32 id);
  virtual uint32 getTrackVideoFrameHeight(uint32 id);

  // From MediaHeader
  virtual uint64 getTrackMediaDuration(uint32 id);
  virtual uint32 getTrackMediaTimescale(uint32 id);
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);

  virtual uint32 getAudioSamplesPerFrame(uint32 id);
  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid, file_sample_info_type *pSampleInfo);

  virtual uint8  getTrackOTIType(uint32 id); // Based on OTI value
  virtual uint8  getTrackAudioFormat(uint32 id); // based on VideoFMT enum //
  virtual uint8  getFramesPerSample(uint32 id);
  virtual uint16 getTotalNumberOfFrames(uint32 id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);
  virtual int32  getTrackAverageBitrate(uint32 id);
  virtual int32  getTrackMaxBitrate(uint32 id);
  virtual uint32 getLargestFrameSize(uint32 id);
  virtual long   getAudioFrameDuration(int);
#ifndef AVI_PARSER_FAST_START_UP
  virtual void   SetIDX1Cache(void*);
  virtual void*  GetIDX1Cache();
#endif
  virtual avi_uint64 CheckAvailableData();
  virtual uint32 GetAACAudioProfile(uint32 id);
  virtual uint32 GetAACAudioFormat(uint32 id);

  // use these functions only for windows media audio, other formats may not implement it //
  virtual uint32 GetAudioBitsPerSample(int id);
  virtual uint32 GetNumAudioChannels(int id);
  virtual uint32 GetAudioEncoderOptions(int id);
  virtual uint32 GetFixedAsfAudioPacketSize(int id);
  virtual uint16 GetAudioAdvancedEncodeOptions(int id);
  virtual uint32 GetAudioAdvancedEncodeOptions2(int id);
  virtual uint32 GetAudioChannelMask(int id);
  virtual uint16 GetAudioVirtualPacketSize(int id);
  virtual uint16 GetFormatTag(int id);
  virtual uint32 GetBlockAlign(int id);

  // this returns Sequence (VOL) Header and its size //
  virtual uint8 *getTrackDecoderSpecificInfoContent(uint32 id);
  virtual uint32 getTrackDecoderSpecificInfoSize(uint32 id);
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize);

  virtual uint8 getAllowAudioOnly();
  virtual uint8 getAllowVideoOnly();
  virtual bool isGenericAudioFileInstance(){return false;};
  virtual bool isAviFileInstance(){return true;}
  virtual void SetCriticalSection(MM_HANDLE);
  virtual uint64 GetLastRetrievedSampleOffset(uint32 trackId);

  bool SetTimeStampedSample(uint32 id, uint64 TimeStamp, uint64 *newTimeStamp, boolean isRewind);

  uint32 FileGetData(  uint64 nOffset, uint32 nNumBytesRequest, avi_uint32 nMaxSize, uint8 *ppData  );

  uint64 getFileSize();
  void getCodecName(char *codecName,uint32 bufLen,uint32 trackId);

  int           GetTotalNumberOfAudioStreams();
  int           GetTotalNumberOfVideoStreams();
  int           GetTotalNumberOfTextStreams();

  bool          isVideoInstance(){return m_playVideo;}
  bool          isAudioInstance(){return m_playAudio;}
  bool          isTextInstance(){return m_playText;}

  uint32        GetMaximumBitRateForTrack(uint32 trackId);
  virtual FileSourceStatus GetDRMType(FileSourceDrmType&);

#ifdef FEATURE_FILESOURCE_DIVX_DRM
  virtual bool        IsDRMProtection();
  virtual void        SetDRMContextInfo(avi_uint8*,avi_uint32);
  virtual avi_uint8*  GetDRMContextInfo(avi_uint32*);
  virtual void        CopyDRMContextInfo(void*);
  virtual void        GetClipDrmInfo(void*);
  virtual bool        CommitDivXPlayback();
#endif
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum);
  virtual FileSourceStatus GetAudioOutputMode(bool*,FileSourceConfigItemEnum);

protected:

  aviErrorType                      m_aviParseLastStatus;

private:

  bool                              m_playAudio;
  bool                              m_playVideo;
  bool                              m_playText;
  bool                              m_corruptFile;

  bool                              m_bVideoReposPending;
  bool                              m_bAudioReposPending;
  bool                              m_bAudioFrameBoundary;

  // if we are streaming, rather than playing locally //
  bool                              m_bStreaming;
  uint32                            m_nNumStreams;
  uint8                             m_hMP3Sync[MP3_MIN_FRAME_HEADER_SIZE];
  bool                              m_bSetSync;

  // These variable are added to know the largest size of audio and video samples respectively//
  avi_uint32                        m_audioLargestSize;
  avi_uint32                        m_videoLargestSize;

  file_sample_info_type   m_sampleInfo[FILE_MAX_MEDIA_STREAMS];

  //These are required to read ahead to know any B-Frames are following for Mpeg4
  bool m_bframesMayPresent;
  bool m_repositionProgress;
  avi_video_sample       m_avi_video_samples[AVI_MAX_VIDEO_FRAMES_READ_AHEAD];
  avi_read_ahead_buffer  m_videoFramesReadAhead;

  uint32                  m_nDecodedDataSize[FILE_MAX_MEDIA_STREAMS];
  uint32                  m_nLargestFrame[FILE_MAX_MEDIA_STREAMS];
  avi_aac_info*           m_pAACAudioInfo;

  //only one of "m_pFileBuf" or "m_filename" can be non-zero
  FILESOURCE_STRING     m_filename;  // EFS file path //
  unsigned char  *m_pFileBuf;  // pointer to buffer for playback from memory //
  uint32          m_FileBufSize;
  uint64          m_fileSize;

  OSCL_FILE     *m_AviFilePtr;

  // AVI parser handle
  aviParser*                     m_pAVIParser;

  //Current output mode
  FileSourceConfigItemEnum m_hFrameOutputModeEnum;
  FileSourceConfigItemEnum m_hHeaderOutputModeEnum;

#ifdef FEATURE_FILESOURCE_DIVX_DRM
  //DRM context info.
  avi_uint8 *drmContext;
  avi_uint32 drmContextLength;
  avi_uint8  drm_frame_info[DIVX_DRM_FRAME_DRM_INFO_SIZE];
  avi_uint8* m_pDRMExDDInfo;
#endif

  int16  m_nSelectedAudioStreamId;
  int16  m_nSelectedVideoStreamId;
  int16  m_nSelectedTextStreamId;

  //function to begin parsing avi file and update media information such as
  //total number of tracks, if drm is present etc.
  bool ParseMetaData();

#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
   avi_uint64 m_minOffsetRequired;
   bool    bHttpStreaming;
   bool    m_bEndOfData;
   uint64 m_wBufferOffset;        //valid downloaded bytes [0..m_wBufferOffset)
   boolean bGetMetaDataSize;
   boolean bIsMetaDataParsed;
   ParserStatusCode parserState;
   uint64 m_maxPlayableTime[FILE_MAX_MEDIA_STREAMS];

   struct tHttpDataOffset
   {
      uint64 Offset;
      boolean bValid;
   } m_HttpDataBufferMinOffsetRequired;
   void sendParserEvent(ParserStatusCode status);
   void sendParseHTTPStreamEvent(void);
   virtual void updateBufferWritePtr ( uint64 writeOffset );
   bool getMetaDataSize ( void );
   virtual bool getBufferedDuration(uint32 id,
                                    int64 nBytes,
                                    uint64 *pBufferedTime);
   /*! =======================================================================
   @brief         Returns absolute file offset(in bytes) associated
                  with time stamp 'pbtime'(in milliseconds).

   @param[in]     ullPBtime:Timestamp(in milliseconds) of the sample
                           that user wants to play/seek
   @param[in]     ulTrackId: Identifies elementary stream to
                  demultiplex from current pes packet
   @param[in]     ullCurrTS: Current playback TS
   @param[in]     ullReposTime: Reposition TS
   @param[out]    offset:Absolute file offset(in bytes) corresponding
                  to 'pbtime'

   @return        true if successful in retrieving the absolute
                  offset(in bytes) else returns false

   @note          This API can be called once FileSource report
                  successful OPEN_COMPLETE. In case of multiple
                  tracks audio, video & text, this API will return
                  minimum absolute offset in bytes

   ==========================================================================*/
   virtual bool GetOffsetForTime(uint64 ullPBtime, uint64* pullOffset,
       uint32 ulTrackId, uint64 ullCurrTS, uint64& ullReposTime);
   virtual bool CanPlayTracks(uint64 pbTime);
   virtual bool parseHTTPStream ( void );
   void sendHTTPStreamUnderrunEvent(void);
   boolean GetHTTPStreamDownLoadedBufferOffset(uint64 *pOffset);
   bool GetTotalAvgBitRate(uint32 * pBitRate);
   PARSER_ERRORTYPE GetStreamParameter(uint32 ulTrackId,
                                       uint32 ulParamIndex,
                                       void* pParamStruct);

   //tWMCDecStatus GetMediaMaxPlayableTime(U32_WMC *nMaxPBTime);

#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  virtual bool        initDivXDrmSystem();
  virtual avi_uint8*  GetDRMInfo(int*);
  void*        m_pClipDrmInfo;
#endif

  AVI_VOP_TYPE whichVop(uint8*, int, uint8 *);
  PARSER_ERRORTYPE getNextAVIMediaSample(uint32 id, uint8 *buf, uint32 *size, uint32 &index);
  void InitData();
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  video::iStreamPort* m_pStreamPort;
#endif
};

#endif  /* __AVIFile_H__ */

