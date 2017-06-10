#ifndef __ASFFile_H__
#define __ASFFile_H__
/* =======================================================================
                              asffile.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2011-2012 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ASFParserLib/main/latest/inc/asffile.h#36 $
$DateTime: 2012/07/19 17:12:52 $
$Change: 2611354 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "parserinternaldefs.h"

#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "oscl_file_io.h"
#include "qcplayer_oscl_utils.h"
#include "filebase.h"

typedef enum mediatype_asf
{
    Audio_STREAM,
    Video_STREAM,
    Binary_STREAM
}MediaType_ASF;

#ifndef NO_ASF_STUBS_NEEDED
  struct strHeaderInfo_WMC;
  struct tStreamIdnMediaType_WMC;
  struct WMCContentDescription;
  struct tStreamIdPattern_WMC;
  struct strAudioInfo_WMC;
  struct strVideoInfo_WMC;
  struct strBinaryInfo_WMC;
  struct tMediaType_WMC;
#endif

#include "zrex_string.h"

typedef void Void_ASF;
typedef int32 I32_ASF;
typedef uint32 U32_ASF;
typedef short I16_ASFC;
typedef unsigned short U16_ASF;
typedef unsigned long long  U64_ASF;
typedef unsigned char  U8_ASF;

#ifdef FEATURE_FILESOURCE_WMA_PRO_DECODER

/*
* Define following if want to dump PCM output to WAV file on EFS
* This will make the playback choppy as writing to EFS makes PCM samples retrieval slow.
* Should not be defined, only for debug purpose.
*/
//#define DUMP_PCM_OUTPUT_TO_WAV_FORMAT
#include "wmadecs_api.h"
#include "wmawfxdefs.h"
 #ifdef DUMP_PCM_OUTPUT_TO_WAV_FORMAT
    #include "wavfileexio.h"
 #endif
#endif

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

#define ASF_READ_BUFFER_SIZE 16384      // file i/o buffer size (reading input file)
#define ASF_MOVIE_TIME_SCALE 1000       // WM Code has Milli Sec interface
#define ASF_STREAM_TIME_SCALE 1000      // WM Code has Milli Sec interface

#define PROFILE_TEMPLATE L"DeviceConformanceTemplate"
#define S_PROFILE L"SP"
#define M_PROFILE L"MP"
#define META_DATA_INFO_LEN 56
#define HTTP_DEFAULT_STARTUP_TIME 4000

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Function Declaration
** ======================================================================= */
void ConvertUnicodeToAscii(char *pDest, uint16 *pSrc , uint32 size);
void GetByteFromBitStream(uint8 *pByte, uint8 *pSrc, int nFromBit, int nBits);
void CheckAvailableDataSize(void* pUserData,uint64* bufDataSize,boolean* pbEndOfData);

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  ASFFile

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
class ASFFile : public FileBase, public Parentable
{

public:

  class IDataSource
  {
    public:
    virtual ~IDataSource() { }
    virtual uint32 readData(uint8 *buffer, uint64 pos, uint32 size) = 0;
    virtual bool isEOS() = 0;
    virtual void setEOS(bool flag) = 0;
  };

  ASFFile(); // Default Constructor
  ASFFile(  const FILESOURCE_STRING &filename,
            unsigned char *pFileBuf = NULL,
            uint32 bufSize = 0,
            IDataSource *dataSource = NULL,
            bool bPlayVideo = true,
            bool bPlayAudio = true
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
            ,bool bHttpStream = false
            ,uint32 wBufferOffset = 0
#endif //#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)

   ); // Constructor
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
    ASFFile(video::iStreamPort* pPort, unsigned char *pFileBuf = NULL,
            uint32 bufSize = 0, IDataSource *dataSource = NULL, bool bPlayVideo = false,bool bPlayAudio = false);
#endif
#ifdef FEATURE_FILESOURCE_WM_DRM_API
  void    *pDRMClientData;
  DRMDecryptMethodT  pDRMDecryptFunction;
#endif

#ifdef FEATURE_FILESOURCE_DRM_DCF
  ASFFile( IxStream* inputStream, bool bPlayVideo, bool bPlayAudio);
#endif

  virtual ~ASFFile();

  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_ASF;
    return FILE_SOURCE_SUCCESS;
  }

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                              uint32 *pulBufSize, uint32 &rulIndex);
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);

  virtual void resetPlayback();
  virtual uint64 resetPlayback(uint64 repos_time, uint32 id, bool bSetToSyncSample,
                               bool *bError, uint64 currentPosTimeStamp);
  virtual bool resetMediaPlayback(uint32 id);  /* reset playback for a particular track */

#ifdef FEATURE_FILESOURCE_REPOSITION_SYNC_FRAME
  virtual uint64 skipNSyncSamples(int , uint32 , bool* , uint64  );
#endif

  virtual uint8 randomAccessDenied();

  virtual PARSER_ERRORTYPE  getAlbumArt(wchar_t *pucDataBuf,
                                        uint32 *pulDatabufLen);
  virtual PARSER_ERRORTYPE GetClipMetaData(wchar_t* pMetaData, uint32* pLength,
                                           FileSourceMetaDataType ienumData);

  // Methods to get the sample rate (i.e. timescales) for the streams and
  // the overall Movie presentation
  virtual uint64 getMovieDuration() const;
  virtual uint32 getMovieTimescale() const;
  virtual uint64 getMovieDurationMsec() const;

  // From Track
  virtual int32 getNumTracks();
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
  virtual uint8  getTrackAudioFormat(uint32 id); /* based on VideoFMT enum */
  virtual uint8  getFramesPerSample(uint32 id);
  virtual uint16  getTotalNumberOfFrames(uint32 id);
  virtual int32 getTrackMaxBufferSizeDB(uint32 id);
  virtual int32 getTrackAverageBitrate(uint32 id);
  virtual int32 getTrackMaxBitrate(uint32 id);
  virtual uint32 getLargestFrameSize(uint32 id);

  virtual uint16 GetAudioVirtualPacketSize(int id);

  virtual bool   isStartOfDummyBytes(int id);

  /* use these functions only for windows media audio, other formats may not implement it */
  virtual uint32 GetAudioBitsPerSample(int id);
  virtual uint32 GetNumAudioChannels(int id);
  virtual uint32 GetFixedAsfAudioPacketSize(int /*id*/)
                 {return GetAsfPacketSize();};
  virtual uint32 GetAudioEncoderOptions(int id);
  virtual uint16 GetAudioAdvancedEncodeOptions(int id);
  virtual uint32 GetAudioAdvancedEncodeOptions2(int id);
  virtual uint32 GetAudioChannelMask(int id);
  virtual uint16 GetAudioArmDataReqThr(int id);
  virtual uint8  GetAudioValidBitsperSample(int id);
  virtual uint16 GetFormatTag(int id);
  virtual uint32 GetBlockAlign(int id);

  /* this returns Sequence (VOL) Header and its size */
  virtual uint8 *getTrackDecoderSpecificInfoContent(uint32 id);
  virtual uint32 getTrackDecoderSpecificInfoSize(uint32 id);
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize);

  virtual uint8 getAllowAudioOnly();
  virtual uint8 getAllowVideoOnly();
  virtual bool isGenericAudioFileInstance(){return false;};

  virtual void SetCriticalSection(MM_HANDLE);

  //Following are useful when HTTP is implemented using circular buffer
  virtual bool getBufferedDuration(uint32 id, int64 nBytes,
                                   uint64 *pBufferedTime);
  virtual bool GetOffsetForTime(uint64 pbtime, uint64* fileoffset, uint32 id,
                                uint64 /*curPBTime*/, uint64& /*reposTime*/);
  virtual uint64 GetLastRetrievedSampleOffset(uint32 trackid);
  virtual bool IsDRMProtection();
  virtual FileSourceStatus GetDRMType(FileSourceDrmType&);
  virtual FileSourceStatus GetJanusDRMInfo(void*,uint32*);


  bool SetTimeStampedSample(uint32 id, uint64 TimeStamp, uint64 *newTimeStamp,
                            boolean isRewind);

  uint32 FileGetData(  uint64 nOffset,
                       uint32 nNumBytesRequest,
                       uint8 **ppData  );

  uint32 getPrerollTime();
  uint64 getFileSize();
  int32 getCodecName(wchar_t *codecName, int32 bufLen, MediaType_ASF mediaType);

  void   updateASFStreamingRepositioningInfo(U32_ASF reposTime,int id);

  U16_ASF           GetTotalNumberOfAudioStreams();
  U16_ASF           GetTotalNumberOfVideoStreams();
  U16_ASF           GetTotalNumberOfBinaryStreams();
  U64_ASF           GetFirstPacketOffset();
  U64_ASF           GetLastPacketOffset();
  U32_ASF           GetAsfPacketSize();
  U32_ASF           GetWMVCompressionType();
  bool              isVideoInstance();
  bool              isAudioInstance();
  bool              isTextInstance();
  uint32            GetMaximumBitRateForTrack(uint32 trackId);


#ifdef FEATURE_FILESOURCE_WM_SWITCHING_THINNING
  void              MarkVideoTrackAsValid(uint32 id);
#endif //FEATURE_FILESOURCE_WM_SWITCHING_THINNING
  virtual long              GetAudioASFPacketDuration(int);
  virtual FileSourceStatus GetNumberOfDRMSupported(uint32* pulNoOfDRMSupported);
  virtual PARSER_ERRORTYPE GetStreamParameter(uint32 ulTrackId,
                                              uint32 ulParamIndex,
                                              void* pParamStruct);

#ifdef FEATURE_FILESOURCE_WM_DRM_API
  virtual bool DRMDecryptSwitch(uint32 wTrackID,             /* to which track this data belong to */
                               void *pEncryptedDataBuf,     /* pointer to encrypted data buffer, which has to be decrypted */
                               void *pDecryptedDataBuf,     /* pointer to destination buffer to copy decrypted data,
                                                               OEM is resposible for copying the decrypted data  */
                               uint32 wEncryptedDataSize,   /* encrypted data buffer size */
                               uint32 *pDecryptedDataSize); /* pointer to decrypted data buffer size,
                                                               OEM is resposible for copying the decrypted data size  */
#endif
  virtual FileSourceStatus SetConfiguration(FileSourceConfigItemEnum eConfig);

private:

  bool                              m_playAudio;
  bool                              m_playVideo;
  bool                              m_playText;
  bool                              m_corruptFile;
  bool                              m_bVideoReposPending;
  bool                              m_bAudioReposPending;
  bool                              m_bStreaming; /* if we are streaming, rather than playing locally */
  bool                              m_bWMADecodeDone;
  unsigned int                      m_nWMATotalDummyBytesSent;
  bool                              m_bIsDummyBytesStart;
  long                              m_nASFAudioPacketDuration;


  file_sample_info_type   m_sampleInfo[FILE_MAX_MEDIA_STREAMS];
  file_sample_info_type   m_prvSampleInfo[FILE_MAX_MEDIA_STREAMS];
  uint32                  m_nDecodedDataSize[FILE_MAX_MEDIA_STREAMS];
  uint32                  m_nLargestFrame[FILE_MAX_MEDIA_STREAMS];

  //The size of the file. This is used to prevent data underruns, when the file size on
  //disk is less than the actual file size.
  uint64                  m_filesize;

  /* only one of "m_pFileBuf" or "m_filename" can be non-zero */
  FILESOURCE_STRING     m_filename;     /* EFS file path */
  unsigned char  *m_pFileBuf;  /* pointer to buffer for playback from memory */
  uint32          m_FileBufSize;

  OSCL_FILE     *m_AsfFilePtr;
  uint8         *m_ReadBuffer;    // file i/o buffer for our input content file
  uint32        m_ReadBufferSize; // by default, Parser will allocate 16KB for ReadBuffer
                                  //If packet size is greater, then it will update bufPtr and bufSize
  IDataSource   *m_dataSource;

  /* WM Parser/Decoder related variables */
  void*                             m_hASFDecoder;
  U32_ASF                           m_nNumStreams;
  strHeaderInfo_WMC                *m_pStrHeaderInfo;
  tStreamIdnMediaType_WMC          *m_ptMediaTypeStreams;
  const WMCContentDescription      *m_pContentDesc;
  tStreamIdPattern_WMC             *m_pStreamDecodePattern;
  strAudioInfo_WMC                 *m_pStrAudioInfo;
  strVideoInfo_WMC                 *m_pStrVideoInfo;
  strBinaryInfo_WMC                *m_pStrBinaryInfo;
  bool                              m_bAllowWmaPackets;  /* audio stream is in packet format */
  bool                              m_bHandleSeekUnderRun;
  FileSourceConfigItemEnum          m_eConfigItem;
  uint8*                            m_pSampleBuf;
  uint32                            m_ulSampleBufSize;

  /* variables needed to decode audio packets into chunk of frames
     which microsoft calls "packet layer decoding */
  uint8 *m_pAudioPacketBuffer;  /* we read audio packet into this buffer */
  uint32 m_nAudioPacketBufferSize;
  uint32 m_nWmaNumPrevFrameBits;

  //Total number of bytes generated from ASFFILE
  uint32 m_nTotalAudioBytesSent;

  //Variables used to determine packet drop
  uint16 m_nPreviousAudioPacketNum;
  uint16 m_nPreviousAudioPacketFramesNum;
  bool   m_bIsPacketDrop;

  int16  m_nSelectedAudioStreamId;
  int16  m_nSelectedVideoStreamId;
  int16  m_nSelectedBinaryStreamId;
  uint32 m_nAsfDataBufferSize;


  /* internal helper functions */
  int    ParseMetaData ( void );
  int    GetMediaTypeForStreamID(tMediaType_WMC *pType, uint32 wStreamId);
  int    GetStreamNumForID(uint32 *pStreamNum, uint32 wStreamId);
  void*  GetAudioStreamInfo(uint32 wStreamId);
  void*  GetVideoStreamInfo(uint32 wStreamId);
  void*  GetBinaryStreamInfo(uint32 wStreamId);
  int    GetVideoFrame( uint32 id, tMediaType_WMC streamType, uint8 *buf,
                                   uint32 size, U32_ASF * pOutDataSize, bool bSyncFrame );
  int    GetAudioFrame( uint32 id, tMediaType_WMC streamType,
                                   U8_ASF * buf, U32_ASF size, U32_ASF * pOutDataSize );

  int    GetStandardAudioFrame( uint32 id, tMediaType_WMC streamType,
                                          U8_ASF * buf, U32_ASF size, U32_ASF * pOutDataSize );

  PARSER_ERRORTYPE UpdateAlbumArtFromMetaData(wchar_t* pucDataBuf,
                                              uint32*  pulDatabufLen,
                                              uint8*   pDataPtr,
                                              uint32   ulDataSize);
#ifdef FEATURE_FILESOURCE_WMA_PRO_DECODER
  //To pull PCM samples out of WMA decoder
  tWMCDecStatus GetAudioDecodedSamples(uint32 id, tMediaType_WMC streamType,
                                       U8_ASF * buf, U32_ASF size,
                                       U32_ASF * pOutDataSize,
                                       U32_ASF * pnSamples);
  //Prepare WAV file format from PCM format.
  void PCMFormat2WaveFormatEx(PCMFormat* pFormat, WAVEFORMATEX* wfx);

  /*
  * Scans all audio track and sets 'm_bUseARMDecoder' to true if
  * we need to use ARM based WMA decoder.
  */
  void          UpdateDecoderTrackInfo();


  //@see above for UpdateDecoderTrackInfo.
  bool          m_bUseARMDecoder;

  WAVEFORMATEXTENSIBLE  m_sWAVEFormatEx;

  #ifdef DUMP_PCM_OUTPUT_TO_WAV_FORMAT
    WavFileIO *pwfioOut;
  #endif
#endif

  bool m_isDSPWMA_pro;


#if defined(FEATURE_FILESOURCE_WMA_PRO_DECODER) || defined(FEATURE_FILESOURCE_WMA_PRO_DSP_DECODER)
  //Returns true if we will be using ARM based decoder for playback of given trackid.
  bool  isWmaProDecoderNeeded(uint32 trackid,uint8* format=NULL);
#endif /* defined(FEATURE_FILESOURCE_WMA_PRO_DECODER) || defined(FEATURE_FILESOURCE_WMA_PRO_DSP_DECODER) */

  bool isVideoCodecSupported();
  bool isIndexObjectPresent();

  uint32 getMSecValueForFrameRateCalculation(U64_ASF);


  typedef struct m_asfStreamingInfo
  {
    bool bAudioValid;
    bool bVideoValid;
  }ASFSTREAMINGINFO;

  ASFSTREAMINGINFO m_wmStreamingInfo;

  //Variables used to process 2 ASF packets. This will make sure we send full complete data for all audio frames
  //in a given ASF packet. This kind of assumes that last frame data can be spilled only in to the next ASF packet.
  U8_ASF* m_pasfDataBuffer;
  int m_nCntPrvBytes;
  int m_nCntCurBytes;
  bool m_bBufferValid;

#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
   uint32 m_minOffsetRequired;
   bool    bHttpStreaming;
   boolean bGetMetaDataSize;
   boolean bIsMetaDataParsed;
   ParserStatusCode parserState;
   U64_ASF m_maxPlayableTime[FILE_MAX_MEDIA_STREAMS];

   struct tHttpDataOffset
   {
      U64_ASF Offset;
      boolean bValid;
   } m_HttpDataBufferMinOffsetRequired;

   void sendParserEvent(ParserStatusCode status);
   virtual void updateBufferWritePtr ( uint64 writeOffset );
   int getMetaDataSize ( void );
   virtual bool CanPlayTracks(uint64 pbTime);
   virtual bool parseHTTPStream ( void );
   boolean GetHTTPStreamDownLoadedBufferOffset(U64_ASF *pOffset,boolean &bEod);
   bool GetTotalAvgBitRate(U32_ASF * pBitRate);
   int GetMediaMaxPlayableTime(U64_ASF *nMaxPBTime);

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  virtual void sendParseHTTPStreamEvent(void);
  virtual void sendHTTPStreamUnderrunEvent(void);

#elif defined (FEATURE_FILESOURCE_PSEUDO_STREAM)
  virtual bool parsePseudoStream( void );

  //To PAUSE Mpeg4/audio/video player
  virtual void sendPlayerPauseEvent();
  virtual void sendAudioPauseEvent(void);
  virtual void sendVideoPauseEvent(void);

  //To RESUME audio/video player
  virtual void sendAudioResumeEvent( void );
  virtual void sendVideoResumeEvent( void );
  virtual void resumeMedia( void );
#endif
#endif //#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)

#ifdef FEATURE_FILESOURCE_WM_DRM_API
   virtual bool RegisterDRMDecryptMethod(DRMDecryptMethodT pDRMDecFunction, void *pClientData);
#endif /* FEATURE_FILESOURCE_WM_DRM_API */

inline void UpdateSamplesInformation(uint8 cPacketNum, uint8 cNumFrm,
                                     uint32 StreamNum, uint64 PresentationTime,
                                     uint32 size, uint32 wNumFrmInPacket);
typedef enum Profile
{
  UNKNOWN_PROFILE,
  SIMPLEPROFILE,
  MAINPROFILE,
  UNSUPPORTED_PROFILE
}WMProfile;

WMProfile m_VidProfile;

 void InitData();

 #ifdef FEATURE_FILESOURCE_DRM_DCF
  /*
  * Class members for supporting DCF playback
  */
  IxStream* m_inputStream;
 #endif
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
 video::iStreamPort* m_pPort;
#endif
};

#endif /* FEATURE_FILESOURCE_WINDOWS_MEDIA */
#endif  /* __ASFFile_H__ */
