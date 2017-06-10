#ifndef __Mpeg4File_H__
#define __Mpeg4File_H__
/* =======================================================================
                              mpeg4file.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2008-2014 QUALCOMM Technologies Inc, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/mpeg4file.h#55 $
$DateTime: 2014/03/10 04:45:38 $
$Change: 5425181 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "oscl_file_io.h"
#include "qcplayer_oscl_utils.h"
#include "filebase.h"
#include "parentable.h"

#include "MMCriticalSection.h"
#ifdef FEATURE_MP4_CUSTOM_META_DATA
#include "kddiuserdataatom.h"
#include "pdcfatoms.h"
#include "dcmduserdataatom.h"
#include "telop.h"
#endif /* FEATURE_MP4_CUSTOM_META_DATA */

#include "udtaatoms.h"
#include "cencatoms.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "videofmt.h"
//#include "msg.h" //TODO, include MMDebugMsg.h from OS Abstraction Layer

#ifdef __cplusplus
} // extern "C"
#endif

#include "ztl.h"
#include "videofmt_common.h"
#include "textsampleentry.h"
#include "fontrecord.h"

//HEADER FILES REQD FOR MULTIPLE SAMPLE RETRIEVAL API
//#include "oscl_media_data.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
const int16 AMRModeSetMask[8] =
{
    0x0001, 0x0002, 0x0004, 0x0008,
    0x0010, 0x0020, 0x0040, 0x0080
};

const int32 AMRBitRates[8] =
{
    4750, 5150,  5900,  6700,
    7400, 7950, 10200, 12200
};

/* to break the VideoFMT read loop, if videoFMT hangs */
#define MPEG4_VIDEOFMT_MAX_LOOP 50000

#define QTV_MPEG4_COARSE_REPOS_LIMIT 5*60*1000       //5min
static const uint32 maskByte[32] = {
                  0x00000001, 0x00000002, 0x00000004, 0x00000008,
                  0x00000010, 0x00000020, 0x00000040, 0x00000080,
                  0x00000100, 0x00000200, 0x00000400, 0x00000800,
                  0x00001000, 0x00002000, 0x00004000, 0x00008000,
                  0x00010000, 0x00020000, 0x00040000, 0x00080000,
                  0x00100000, 0x00200000, 0x00400000, 0x00800000,
                  0x01000000, 0x02000000, 0x04000000, 0x08000000,
                  0x10000000, 0x20000000, 0x40000000, 0x80000000
                   };
#define DURATION_OF_AMR_FRAME_BLOCK 20
#define MAX_GEO_TAG_ELEMENT_SIZE (18)
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
MACRO MYOBJ

ARGS
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  Mpeg4File

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
class Mpeg4File : public FileBase, public Parentable
{

public:

  Mpeg4File(); // Default Constructor

  Mpeg4File(  FILESOURCE_STRING filename,
              unsigned char *pFileBuf=NULL,
              uint32 bufSize=0,
              bool bPlayVideo = false,
              bool bPlayAudio = false,
              bool bPlayText  = false
#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || \
    defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
              ,bool bHttpStream     = false
              ,uint32 wBufferOffset = 0
#endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
            ); // local file playback Constructor

#ifdef FEATURE_FILESOURCE_DRM_DCF
  // DRM-file-media playback Constructor
    Mpeg4File( IxStream* inputStream,
               bool bPlayVideo = false,
               bool bPlayAudio = false,
               bool bPlayText  = false);
#endif
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  Mpeg4File(  video::iStreamPort* pPort,
              bool bPlayVideo = false,
              bool bPlayAudio = false,
              bool bPlayText  = false,
              FileSourceFileFormat eFileFormat = FILE_SOURCE_UNKNOWN);
#endif
  virtual ~Mpeg4File();
  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& eFileFormat)
  {
    if (FILE_SOURCE_MP4_DASH == m_eFileFormat)
    {
      eFileFormat = m_eFileFormat;
    }
    else if(!isFileFragmented())
    {
      eFileFormat = FILE_SOURCE_MPEG4;
    }
    else
    {
      eFileFormat = FILE_SOURCE_3G2;
    }
    return FILE_SOURCE_SUCCESS;
  }

  /* method to find if mpeg4 file is fragmented or non-fragmented */
  virtual bool isFileFragmented(void);

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                              uint32 *pulBufSize, uint32 &rulIndex);
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);

  uint32 m_nextAVCSeqSample;
  uint32 m_nextAVCPicSample;
  uint32 m_nextMVCSeqSample;
  uint32 m_nextMVCPicSample;
  virtual uint32 getNumPicParamSet(uint32 trackId);
  virtual uint32 getNumSeqParamSet(uint32 trackId);
  virtual int32 getNextParamSetNAL(uint32 trackId, uint8 *buf, uint32 size);
  virtual void  resetParamSetNAL(uint32 trackId);
  virtual int  GetSizeOfNALLengthField(uint32 trackId);

  virtual uint32 GetData(DataT dType, uint8 *pBuf, uint32 size, uint32 offset);
  virtual bool resetMediaPlayback(uint32 id);  /* reset playback for a particular track */
  uint32 fragmentNumber;

  //Return codec config data
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id,
                                                             uint8* buf,
                                                             uint32 *pbufSize);

  virtual void resetPlayback();
  virtual uint64 resetPlayback(uint64 pos, uint32 id, bool bSetToSyncSample,
                               bool *bError, uint64 currentPosTimeStamp);

  virtual uint64 skipNSyncSamples(int offset, uint32 id, bool *bError,
                                  uint64 currentPosTimeStamp);

  // from 'rand' atom at top level
  virtual uint8 randomAccessDenied();

  /* ==========================================================================
    @brief  getAlbumArt.

    @details    Checks if Album art/cover art info is present or not and provides
                data if it is available

    @param[in/out]  pucDataBuf          O/p Metadata Buffer
    @param[in/out]  pulDatabufLen       O/p Metadata size

    @return  PARSER_ErrorNone if successful
             else returns corresponding error.
    @note       none.
  ===========================================================================*/
  virtual PARSER_ERRORTYPE getAlbumArt(wchar_t *pucDataBuf,
                                       uint32 *pulDatabufLen);
  /* ==========================================================================
  @brief  GetClipMetaData.

  @details    Checks if required metadata is present or not and provides
              data if it is available

  @param[in/out]  pucDataBuf          O/p Metadata Buffer
  @param[in/out]  pulDatabufLen       O/p Metadata size
  @param[in]      ienumData           FileSource Metadata type Enum

  @return  PARSER_ErrorNone if successful
           else returns corresponding error.
  @note       None.
===========================================================================*/
  virtual PARSER_ERRORTYPE GetClipMetaData(wchar_t* pMetaData, uint32* pLength,
                                           FileSourceMetaDataType ienumData);
  virtual FS_TEXT_ENCODING_TYPE GetMetaDataEncodingType() {return m_eEncodeType;};
  virtual void getCodecDatainRawFormat(bool bRawCodec) {m_bRawCodecData = bRawCodec;};

  // From Movie
  virtual int32 getNumTracks();
  virtual uint32 getTrackWholeIDList(uint32 *ids);

  // RETRIEVAL METHODS
  // Methods to get the sample rate (i.e. timescales) for the streams and
  // the overall Mpeg-4 presentation
  virtual uint64 getMovieDuration() const;
  virtual uint32 getMovieTimescale() const;

  // From Track
  virtual uint8 trackRandomAccessDenied(uint32 id);
  virtual float  getTrackVideoFrameRate(uint32 id); // from 'vinf' atom at track level

  virtual uint32 getTrackVideoFrameWidth(uint32 id); // from pvti atom at file level
  virtual uint32 getTrackVideoFrameHeight(uint32 id);

  virtual uint32 GetNumAudioChannels(int id); // from qtv_windows_media
  virtual FILESOURCE_STRING getAudioTrackLanguage(uint32 id);


  // From TrackReference
  virtual uint32  trackDependsOn(uint32 id);

  // From MediaHeader
  virtual uint64 getTrackMediaDuration(uint32 id);
  virtual uint32 getTrackMediaTimescale(uint32 id);
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);

  virtual uint32 getAudioSamplesPerFrame(uint32 id);
  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid, file_sample_info_type *pSampleInfo);

  /* Mp4 Clip may has meta data */
  virtual bool HasMetaData() {return true;};

  /* 3GPP timed text related APIs */
  virtual TextSampleEntry *getTextSampleEntryAt (uint32 trackid, uint32 index);
  virtual void ParseTimedTextAtom(video_fmt_text_data_type *text_atom,OSCL_FILE* localFilePtr);
  virtual uint32 GetNumTX3GAtom();
#ifdef FEATURE_MP4_CUSTOM_META_DATA
//KDDI Telop related APIs
  virtual bool IsTelopPresent();
  virtual uint32 getTelopTrackDuration();
  virtual TelopElement *getNextTelopElement();
  virtual TelopHeader *getTelopHeader();
  virtual uint32 resetTelopPlayback(uint32 startPos);

  /*KDDI Meta Data Related APIs*/

  // From CopyGaurdAtom
  virtual uint32 getCopyProhibitionFlag();
  virtual uint32 getValidityEffectiveDate();
  virtual uint32 getValidityPeriod();
  virtual uint32 getNumberofAllowedPlayBacks();

  // From Content Property Atom
  virtual FILESOURCE_STRING getContentPropertyTitle();
  virtual FILESOURCE_STRING getContentPropertyCopyRight();
  virtual FILESOURCE_STRING getContentPropertyAuthor();
  virtual FILESOURCE_STRING getContentPropertyMemo();
  virtual uint32      getAuthorDLLVersion();

  //From Movie Mail Atom
  virtual uint32 getEditFlags();
  virtual uint8  getRecordingMode();
  virtual uint32 getRecordingDate();

  //From Encoder Information Atom
  virtual uint8*  getDeviceName() const;
  virtual uint8*  getModelName() const;
  virtual uint8*  getEncoderInformation() const;
  virtual uint8*  getMuxInformation() const;

  virtual uint8*  getTelopInformationString() const;
  virtual uint32  getTelopInformationSize();

  //KDDI GPS Atom
  virtual uint16 getGPSByteOrder();
  virtual uint32  getGPSVersionID();
  virtual uint32  getGPSLatitudeRef();
  virtual uint32  getGPSLongitudeRef();
  virtual uint32  getGPSAltitudeRef();
  virtual uint64 *getGPSLatitudeArray();
  virtual uint64 *getGPSLongitudeArray();
  virtual uint64 getGPSAltitude();
  virtual uint64 *getGPSTimeArray();
  virtual FILESOURCE_STRING getGPSSurveyData();
  virtual FILESOURCE_STRING getPositoningMethod();
  virtual FILESOURCE_STRING getPositioningName();
  virtual FILESOURCE_STRING getGPSDate();
  virtual uint64 getGPSExtensionMapScaleInfo();
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
  virtual uint8  getTrackOTIType(uint32 id); // Based on OTI value
  virtual uint8  getTrackAudioFormat(uint32 id); /* based on VideoFMT enum */
  virtual uint8  getFramesPerSample(uint32 id);
  virtual uint16 getTotalNumberOfFrames(uint32 id);
  virtual uint32 GetAACAudioProfile(uint32 id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);

  virtual int32 getTrackAverageBitrate(uint32 id);
  virtual int32 getTrackMaxBitrate(uint32 id);
  virtual bool isGenericAudioFileInstance(){return false;};

#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
  virtual void updateBufferWritePtr(uint64 writeOffset) = 0;
  virtual bool parsePseudoStream( void ) = 0;
#endif
  virtual uint16 getParseFragmentNum( void ) = 0;
  virtual uint16 getReadFragmentNum( void ) = 0;

  virtual uint64 repositionAccessPoint( int32 skipNumber, uint32 id, bool &bError ,uint64 currentPosTimeStampMsec) = 0;
  virtual void SetCriticalSection(MM_HANDLE);

 /*!
  @brief      Get Audio/Video stream parameter

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
  */
  virtual PARSER_ERRORTYPE GetStreamParameter(uint32 ulTrackId,
                                             uint32 ulParamIndex,
                                             void* pParameterStructure);

// Operations
public:
  static void mp4ParseStatusCallback (video_fmt_status_type status,
                                      void *client_data,
                                      void *info,
                                      video_fmt_end_cb_func_type end);

  void mp4ParseStatus ( video_fmt_status_type status,
                        video_fmt_status_cb_info_type *info,
                        video_fmt_end_cb_func_type end);

  static void mp4ReadStatusCallback (video_fmt_status_type status,
                                     void *client_data,
                                     void *info,
                                     video_fmt_end_cb_func_type end);

  void mp4ReadStatus (uint32 streamNum,
                      video_fmt_status_type status,
                      video_fmt_status_cb_info_type *info,
                      video_fmt_end_cb_func_type end);

  static void mp4SyncStatusCallback (video_fmt_status_type status,
                                     void *client_data,
                                     void *info,
                                     video_fmt_end_cb_func_type end);

  void mp4SyncStatus (uint32 streamNum,
                      video_fmt_status_type status,
                      video_fmt_status_cb_info_type *info,
                      video_fmt_end_cb_func_type end);

  video_fmt_stream_info_type *getTrackInfoForID (uint32 id);

  uint32 getTrackIdFromStreamNum (uint32 streamNum);

  PARSER_ERRORTYPE getSampleInfo (uint32 streamNum,
                                  uint64 startingSample,
                                  uint64 sampleCount,
                                  video_fmt_sample_info_type *buffer);
  PARSER_ERRORTYPE getSample (uint32 streamNum,
                              video_fmt_data_unit_type unitDef,
                              uint64 startingUnit,
                              uint64 unitCount,
                              uint8 *buffer);
  bool getSyncSampleInfo (uint32 streamNum,
                          uint64 sampleNum,
                          bool reverse,
                          video_fmt_sample_info_type *buffer);

  virtual bool getTimestampedSampleInfo(video_fmt_stream_info_type *p_track,
                                uint64                      TimeStamp,
                                video_fmt_sample_info_type *sampleInfo,
                                uint64                     *newTimeStamp,
                                bool                        bSetToSyncSample,
                                uint64                      currentPosTimeStamp);

  virtual  bool getSampleAtTimestamp(video_fmt_stream_info_type *p_track,
                            uint64                      timestamp,
                            bool                        rewind,
                            video_fmt_sample_info_type *sampleInfo);

  virtual void parseFirstFragment();
  virtual uint32 getLargestFrameSize (uint32 id);
  virtual uint32 getRotationDegrees( uint32 ulTrackId);
  virtual bool ParseStream();
#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
  bool getMetaDataSize ( void );
#endif
  bool parseMetaData (void);
  bool peekMetaDataSize (uint32 fragment_num);
  void sendParserEvent(ParserStatusCode status) {parserState = status;};
  boolean initializeVideoFMT(void);
  virtual bool parseHTTPStream( void );
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  virtual bool CanPlayTracks(uint64 pbTime);
  virtual bool getBufferedDuration(uint32 id,
                                   int64 nBytes,
                                   uint64 *pBufferedTime);
  virtual void updateBufferWritePtr(uint64 writeOffset);
  void sendParseHTTPStreamEvent( void );
  void sendHTTPStreamUnderrunEvent(void);
  uint64 getSampleAbsOffset (uint32 streamNum,
                             uint64 sampleOffset,
                             uint32 sampleSize);
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

  uint32 getNumUUIDAtom();
  ZArray<Atom *> UUIDatomEntryArray;
  uint32 UUIDatomEntryCount;

  virtual void resetPlaybackPos(uint32 tId);

  const video_fmt_info_type& getVideoFmtInfo( void ) const
                             { return m_videoFmtInfo; };

  virtual EncryptionTypeT getEncryptionType();
  virtual EncryptionTypeT getEncryptionType(uint32 track_id);
  virtual bool RegisterDRMDecryptMethod(DRMDecryptMethodT pDRMDecriptFunction,
                                        void *pClientData);
  /* 'DASH' related APIs */
  virtual bool setBaseTime(uint32 id, uint64 nBaseTime);
  virtual bool getBaseTime(uint32 id, uint64 *nBaseTime);

  /**************************************************************************!
  @brief   Function to find out if the clip is DRM protected or not.
  @return  True if file is protected else returns false.
  @note    It is expected that user has received the successul callback
           for OpenFile() earlier.
  ************************************************************************/
  virtual bool IsDRMProtection();

/************************************************************************!
  @brief   Function to get the number of DRM Scheme supported.
  @param[in,out]   pulNoOfDRMSupported will provide the number of DRM scheme
                   supported by media.
  @note    It is expected that user has received the successul callback
           for OpenFile() earlier.
  ****************************************************************************/
  FileSourceStatus GetNumberOfDRMSupported(uint32* pulNoOfDRMSupported);
 /************************************************************************!
  @brief   Function to get the DRM Scheme.

  @param[in,out]   drmtype specify the DRM Scheme.

  @return         Returns FILE_SOURCE_SUCCESS if DRM INFO available.
  ****************************************************************************/
  FileSourceStatus GetDRMType(FileSourceDrmType& drmtype);

  //! Declaration of ZArray of PSSH Atoms
  ZArray<CPsshAtom*> m_aPSSHatomEntryArray;
  //! Declaration of ZArray of SINF Atoms :
  ZArray<CSinfAtom*> m_aSINFatomEntryArray;

protected:
  uint32 m_ulPSSHatomEntryCount;
  uint32 m_ulSelectedDRMIdx;

  //Qtv mpeg4 default value for coarse repositioning
  uint64 m_defaultCoarseJumpLimit;

  // These variables are used to communicate between the class and the
  // callback function registered with the video format services.
  struct videoFMTClientData
  {
    Mpeg4File * Mp4FilePtr;
    int streamNum;
  };

  video_fmt_status_type             m_mp4ParseLastStatus;
  bool                              m_hasAudio, m_hasVideo, m_hasText;
  bool                              m_allSyncVideo; /* all video frames are SYNC frames (STSS not present) */

  uint64                            m_parseIODoneSize;
  video_fmt_continue_cb_func_type   m_mp4ParseContinueCb;
  FS_TEXT_ENCODING_TYPE             m_eEncodeType;

  bool                              m_bRawCodecData;
  bool                              m_playAudio;
  bool                              m_playVideo;
  bool                              m_playText;
  bool                              m_isFragmentedFile;
  uint8*                            m_pMoovAtomBuf;
  uint32                            m_ulMoovBufSize;
  uint64                            m_ullMoovAtomOffset;
  uint32                            m_ulMoovBufDataRead;

  uint32                            m_ulGeoTagSize;
  uint8                             m_ucGeoTagLoc[MAX_GEO_TAG_ELEMENT_SIZE];

  bool                              m_bDataUnderRun;
  /*
  * Variables to indicate whether clip has audio/video/text tracks.
  * These are useful in reporting UNSUPPORTED_FORMAT notifications if we don't end up selecting
  * any of the following media tracks but clip has corresponding tracks.
  */
  bool                              m_bAudioPresentInClip;
  bool                              m_bVideoPresentInClip;
  bool                              m_bTextPresentInClip;

  /* Flag to mark if the current parsing position is end of fragment */
  bool                              m_parsedEndofFragment;
  /* Flag to mark if the current parsing position is end of file */
  bool                              m_parsedEndofFile;

  bool                              m_corruptFile;
  uint64                            m_iodoneSize[VIDEO_FMT_MAX_MEDIA_STREAMS];
  /* using this member variable to distinguish between udta vs. kddi 'titl' atom */
  bool                              m_bUdtaAtomPresent;


  uint64                            m_absFileOffset[VIDEO_FMT_MAX_MEDIA_STREAMS];

  videoFMTClientData                m_clientData[VIDEO_FMT_MAX_MEDIA_STREAMS];

  video_fmt_status_type             m_mp4ReadLastStatus[VIDEO_FMT_MAX_MEDIA_STREAMS];
  video_fmt_continue_cb_func_type   m_mp4ReadContinueCb[VIDEO_FMT_MAX_MEDIA_STREAMS];
  void                             *m_mp4ReadServerData[VIDEO_FMT_MAX_MEDIA_STREAMS];

  video_fmt_status_type             m_mp4SyncLastStatus[VIDEO_FMT_MAX_MEDIA_STREAMS];
  video_fmt_continue_cb_func_type   m_mp4SyncContinueCb[VIDEO_FMT_MAX_MEDIA_STREAMS];
  void                             *m_mp4SyncServerData[VIDEO_FMT_MAX_MEDIA_STREAMS];

  // This is a copy of the stream information returned by the video format
  // services after parsing an MP4 file.
  video_fmt_end_cb_func_type        m_mp4ParseEndCb;
  void                              *m_mp4ParseServerData;
  video_fmt_info_type               m_videoFmtInfo;

  video_fmt_sample_info_type  m_sampleInfo [VIDEO_FMT_MAX_MEDIA_STREAMS];
  bool m_bSampleInfoChanged;
  uint64  m_nextSample [VIDEO_FMT_MAX_MEDIA_STREAMS];
  uint32  m_reposStreamPending;
  uint64 m_nextReposSample [VIDEO_FMT_MAX_MEDIA_STREAMS];

  video_fmt_stream_info_type* m_track[VIDEO_FMT_MAX_MEDIA_STREAMS];
  uint32 m_trackCount;

  ZArray<TextSampleEntry *> textSampleEntryArray;
  uint32 textSampleEntryCount;

  OSCL_FILE*  m_parseFilePtr;
  FILESOURCE_STRING m_filename;
  uint64      m_fileSize;
  boolean     m_fileSizeFound;

  /* only one of "m_pFileBuf" or "m_filename" can be non-zero */
  unsigned char *m_pFileBuf;  /* pointer to buffer for playback from memory */

#ifdef FEATURE_FILESOURCE_DRM_DCF
  //member variable to store IxStream
  IxStream* m_inputStream;
#endif

   bool m_bOMADRMV2Encrypted;
   uint64 m_minOffsetRequired;
   bool bHttpStreaming;
   bool m_bMOOVPresent;
   uint64 m_wBufferOffset;        //valid downloaded bytes [0..m_wBufferOffset)
   boolean m_bEndOfData;
   uint32 m_currentParseFragment;
   boolean bGetMetaDataSize;
   boolean bDataIncomplete;
   boolean bQtvPlayerPaused;
   boolean bsendParseFragmentCmd;
   MM_HANDLE videoFMT_Access_CS;
   ParserStatusCode parserState;
   uint64 m_startupTime;
   uint64 m_pbTime;
   boolean Parsed;
   uint64 mdat_size;
   boolean Initialized;

   video_fmt_sample_info_type m_bufferedUptoSample[VIDEO_FMT_MAX_MEDIA_STREAMS];

/* This is to hold the each pdcf atom */
  typedef struct
  {
   uint32 track_id;
   EncryptionTypeT encryptionType;
  }Track_Encryption_Type;

  Track_Encryption_Type   m_EncryptionType[VIDEO_FMT_MAX_MEDIA_STREAMS];
  uint8                   m_OffsetinEncryptionTypeArray;
  void                    *m_pDRMClientData;           /* client data provided by OEM when registering callback */

  boolean (*m_pDRMDecryptFunction)
  (
    uint32      TrackId,          /* track type */
    void      *pEncryptedDataBuf,   /* pointer to encrypted data buffer, which has to be decrypted */
    void      *pDecryptedDataBuf,   /* pointer to destination buffer to copy decrypted data,
                                       OEM is resposible for copying the decrypted data  */
    uint32    wEncryptedDataSize,   /* encrypted data buffer size */
    uint32    *pDecryptedDataSize,   /* pointer to decrypted data buffer size,
                                       OEM is resposible for copying the decrypted data size  */
    void      *pDRMClientData       /* client data provided by OEM when registering callback */
  );
  uint8 *m_pEncryptedDataBuffer;
#ifdef FEATURE_MP4_CUSTOM_META_DATA
  PdcfAtom * _pdcfAtom;
  DcmdDrmAtom * _dcmdAtom; /* DRM atom */
  KDDIDrmAtom                *_kddiDRMAtom;
  KDDIContentPropertyAtom    *_kddiContentPropertyAtom;
  KDDIMovieMailAtom          *_kddiMovieMailAtom;
  KDDIEncoderInformationAtom *_kddiEncoderInformationAtom;
  KDDIGPSAtom                *_kddiGPSAtom;
  KDDITelopAtom              *_kddiTelopElement;
  TsmlAtom                   *_pTsmlAtom;
  FtypAtom * _ftypAtom; /* File Type atom*/
  UdtaMidiAtom * _midiAtom;
  virtual bool IsMidiDataPresent();
  virtual uint32 GetMidiDataSize();
  virtual uint32 GetMidiData(uint8 *pBuf, uint32 size, uint32 offset);

  UdtaLinkAtom * _linkAtom;
  virtual bool IsLinkDataPresent();
  virtual uint32 GetLinkDataSize();
  virtual uint32 GetLinkData(uint8 *pBuf, uint32 size);
  void process_mod_midi_atom();
  virtual bool IsDataPresent(DataT dType, uint32 track_id);
  void process_kddi_telop_text();
#endif /* FEATURE_MP4_CUSTOM_META_DATA */

  UdtaCprtAtom * _cprtAtom; /* Copyright atom*/
  UdtaAuthAtom * _authAtom; /* Author atom */
  UdtaTitlAtom * _titlAtom; /* Title atom */
  UdtaDscpAtom * _dscpAtom; /* Description atom */
  UdtaRtngAtom * _rtngAtom; /* Rating atom */
  UdtaGnreAtom * _gnreAtom; /* Genre atom */
  UdtaPerfAtom * _perfAtom; /* Performance atom */
  UdtaClsfAtom * _clsfAtom; /* Classification atom */
  UdtaKywdAtom * _kywdAtom; /* Keyword */
  UdtaLociAtom * _lociAtom; /* Location atom */
  UdtaAlbumAtom * _albumAtom; /* Album atom */
  UdtaMetaAtom * _metaAtom; /* Meta atom*/
  CSubsAtom    * pSubsAtom; /* SUBS atom */
  UdtaYrrcAtom*  pYrccAtom; /* YRRC atom */

  virtual uint32 GetDataSize(DataT dType, uint32 offset);

  void mp4ParseUUIDAtom(video_fmt_uuid_data_type *pAtomInfo,
                        OSCL_FILE* localFilePtr);

  virtual PARSER_ERRORTYPE getSampleInfoError(video_fmt_stream_info_type *)
                                             { return PARSER_ErrorEndOfFile; }

  virtual void process_video_fmt_info(video_fmt_status_type status,
                                      video_fmt_status_cb_info_type *info);

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  virtual bool GetOffsetForTime(uint64  pbtime,
                                uint64* fileoffset,
                                uint32  id,
                                uint64  currentPosTimeStamp,
                                uint64& reposTime);
  virtual uint64 GetLastRetrievedSampleOffset(uint32 trackid);
#endif
private:
  virtual void InitData();

  /************************************************************************!
  @brief     Retrieve auxiliary sample information of supported
             DRM systems.

  @details   This API will provides  auxiliary sample information
             applicable to supported DRM system in same media
             file.It contains SystemID of the DRM system along with size of
             content protection system specific data for each supported
             DRM in sequential order.

  @param[in]      ulTrackID TrackId identifying the media track.
  @param[in]      streamNum identifying the stream number.
  @param[out]     pSampleInfo: Update the auxiliary sample info details
                  in sample info structure.

  @return         Returns PARSER_ErrorNone if DRM Auxiliary INFO available.
  ****************************************************************************/
  virtual PARSER_ERRORTYPE getAuxiliarySampleInfo(uint32 trackid ,
                                        uint32 streamNum,
                                        file_sample_info_type *pSampleInfo);
  /* ==========================================================================
    @brief  ReadMetaDataFromiLst.

    @details    Read Metadata from iLst Atom

    @param[in]      ienumData           Metadata type requested
    @param[in/out]  pucDataBuf          O/p Metadata Buffer
    @param[in/out]  pulDatabufLen       O/p Metadata size
    @param[in]      pMetaData           Metadata structure Pointer

    @return  PARSER_ErrorNone if successful
             else returns corresponding error.
    @note       This will be called only if "IsMetaDataPresentiniLst" function
                returns True.
  ===========================================================================*/
  virtual PARSER_ERRORTYPE ReadMetaDataFromiLst(
                                              FileSourceMetaDataType ienumData,
                                              wchar_t* pucDataBuf,
                                              uint32 *pulDatabufLen,
                                              ItunesMetaData *pMetaData);
  /* ==========================================================================
  @brief  IsMetaDataPresentiniLst.

  @details    Checks if the required metadata type info is present in
              iLst atom or not

  @param[in]      ienumData           Metadata type requested
  @param[in/out]  dpMetaData          Metadata structure Double Pointer

  @return  "true" if required metadata field is available
           else returns "false".
  @note       None.
===========================================================================*/
  bool           IsMetaDataPresentiniLst(FileSourceMetaDataType ienumData,
                                         ItunesMetaData**       dpMetaData);
  /* ==========================================================================
  @brief  MapFileSourceEnumtoDataEnum.

  @details    This function used to Map FileSource Metadata Enums with UDTA .
              metadata atom Enums

  @param[in]      ienumData           FileSource Metadata Enum
  @param[in/out]  eDataType           UDTA atom type
  @param[in/out]  pulDatabufLen       O/p Buffer size

  @return     None
  @note       None.
=============================================================================*/
  virtual void  MapFileSourceEnumtoDataEnum(FileSourceMetaDataType ienumData,
                                            DataT &eDataType);
  /* ==========================================================================
    @brief  ReadMetaDataFromUDTA.

    @details    This function used to read data from UDTA meta atoms.
                This function also calculates the size of metadata

    @param[in]      eDataType           UDTA atom type
    @param[in/out]  pucDataBuf          O/p buffer to read metadata.
    @param[in/out]  pulDatabufLen       O/p Buffer size

    @return  PARSER_ErrorNone if successful
             else returns corresponding error.
    @note       None.
  ===========================================================================*/
  virtual PARSER_ERRORTYPE ReadMetaDataFromUDTA(DataT    eDataType,
                                                wchar_t* pucDataBuf,
                                                uint32*  pulDatabufLen);

  /* ==========================================================================
    @brief  IsMetaDataPresentinID3.

    @details    Checks if the required metadata type info is present in
                ID3 atom or not

    @param[in]      ienumData           Metadata type requested
    @param[in/out]  pucDataBuf          Buffer to store Metadata
    @param[in/out]  pulDatabufLen       Buffer size

    @return  "PARSER_ErrorNone" if successful
             else returns corresponding error.
    @note       None.
  ===========================================================================*/
  virtual PARSER_ERRORTYPE IsMetaDataPresentinID3(
                                              FileSourceMetaDataType ienumData,
                                              wchar_t* pBuf, uint32* pulBufLen);
protected:
  bool         m_bTimeToOffsetInvoked;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pStreamPort;
#endif
  uint8* m_pBuffer;
  uint32 m_nBufferSize;
  uint8* m_pSampleInfoBuffer;     // This buffer is used for Auxiliary sample.
  uint32 m_nSampleInfoBufferSize; // Auxiliary sample buffer size.

  /* 'DASH' related params */
  FileSourceFileFormat m_eFileFormat;
  uint64 m_baseTimeStamp [VIDEO_FMT_MAX_MEDIA_STREAMS];
  bool   m_bIsDashClip;      // This flag used to do mark the difference b/w DASH and non-DASH clips.
                             // For DASH clips, processing of fragments will be done differently.
  bool   m_bSeekDone;        // This flag is used to indicate whether seek has been done or not.
                             // This flag is used to indicate Videofmt not to use TFDT value at fragment boundaries.
  bool   m_bFragBoundary;    // This flag used to mark the fragment boundary
  uint64 m_nLastFragOffset;  // This param used to store last fragment offset value.
                             // It will be used to restore Videofmt context offset value during data under run cases
};

#endif  // __Mpeg4File_H__
