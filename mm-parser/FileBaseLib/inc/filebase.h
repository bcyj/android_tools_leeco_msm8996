#ifndef __FileBase_H__
#define __FileBase_H__
/* =======================================================================
                              FileBase.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2008-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential..

========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/filebase.h#136 $
$DateTime: 2014/05/14 21:20:42 $
$Change: 5895031 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "filesourcetypes.h"
#include "atomdefs.h"
#include "parentable.h"
#include "isucceedfail.h"
#include "ztl.h"
#include "MMMalloc.h"

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  #include "DataSourcePort.h"
#endif

//HEADER FILES REQD FOR MULTIPLE SAMPLE RETRIEVAL API
//#include "oscl_media_data.h"

//Global inline functions
inline void FS_REVERSE_ENDIAN(uint8* ptr, size_t nSize)
{
  uint8 ucEndPos = (uint8)(nSize-1);
  if(ptr && (nSize >1))
  {
    for(int i =0; (i < (int)nSize) && (i <= ucEndPos); i++)
    {
      uint8 temp = ptr[i];
      *(ptr+i) = *(ptr+ucEndPos);
      *(ptr+ucEndPos) = temp;
      ucEndPos--;
    }
  }
}
class IxStream;
inline bool IS_AUDIO_FILE_FORMAT(FileSourceFileFormat eFileFormat)
{
  if( ( (eFileFormat) == FILE_SOURCE_AAC )            ||
      ( (eFileFormat) == FILE_SOURCE_AMR_NB )         ||
      ( (eFileFormat) == FILE_SOURCE_AMR_WB )         ||
      ( (eFileFormat) == FILE_SOURCE_AC3 )            ||
      ( (eFileFormat) == FILE_SOURCE_DTS )            ||
      ( (eFileFormat) == FILE_SOURCE_EVRCB )          ||
      ( (eFileFormat) == FILE_SOURCE_EVRC_WB )        ||
      ( (eFileFormat) == FILE_SOURCE_FLAC )           ||
      ( (eFileFormat) == FILE_SOURCE_MP3 )            ||
      ( (eFileFormat) == FILE_SOURCE_OGG )            ||
      ( (eFileFormat) == FILE_SOURCE_QCP )            ||
      ( (eFileFormat) == FILE_SOURCE_WAV )     )
  {
    return true;
  }
  return false;
}
inline bool IS_AUDIO_CODEC(uint8 codecFormat)
{
  if( ( (codecFormat) == WM_AUDIO )           ||
      ( (codecFormat) == WM_SPEECH )          ||
      ( (codecFormat) == WM_PRO_AUDIO )       ||
      ( (codecFormat) == WM_LOSSLESS )        ||
      ( (codecFormat) == EVRC_AUDIO )         ||
      ( (codecFormat) == AMR_AUDIO )          ||
      ( (codecFormat) == AMR_WB_AUDIO )       ||
      ( (codecFormat) == AMR_WB_PLUS_AUDIO )  ||
      ( (codecFormat) == QCP_AUDIO )          ||
      ( (codecFormat) == QCP_QLCM_AUDIO )     ||
      ( (codecFormat) == MPEG4_AUDIO )        ||
      ( (codecFormat) == AAC_ADTS_AUDIO )     ||
      ( (codecFormat) == AAC_ADIF_AUDIO )     ||
      ( (codecFormat) == AAC_LOAS_AUDIO )     ||
      ( (codecFormat) == MP3_AUDIO )          ||
      ( (codecFormat) == MP2_AUDIO )          ||
      ( (codecFormat) == PUREVOICE_AUDIO )    ||
      ( (codecFormat) == AC3_AUDIO )          ||
      ( (codecFormat) == EAC3_AUDIO )         ||
      ( (codecFormat) == EAC3_JOC_AUDIO )     ||
      ( (codecFormat) == PCM_AUDIO )          ||
      ( (codecFormat) == EVRC_B_AUDIO)        ||
      ( (codecFormat) == EVRC_WB_AUDIO )      ||
      ( (codecFormat) == G711_ALAW_AUDIO )    ||
      ( (codecFormat) == G711_MULAW_AUDIO )   ||
      ( (codecFormat) == G721_AUDIO )         ||
      ( (codecFormat) == GSM_FR_AUDIO)        ||
      ( (codecFormat) == G723_AUDIO )         ||
      ( (codecFormat) == VORBIS_AUDIO )       ||
      ( (codecFormat) == FLAC_AUDIO )         ||
      ( (codecFormat) == PUREVOICE_AUDIO_2 )  ||
      ( (codecFormat) == MPEG4_AUDIO_BSAC )   ||
      ( (codecFormat) == DTS_AUDIO )          ||
      ( (codecFormat) == MPEG2_AAC_LC ))
  {
    return true;
  }
  return false;
}
inline bool IS_VIDEO_CODEC(uint8 codecFormat)
{
  if( ( (codecFormat) == WM_VIDEO_7)           ||
      ( (codecFormat) == WM_VIDEO_8)           ||
      ( (codecFormat) == VC1_VIDEO)            ||
      ( (codecFormat) == WM_VIDEO_9)           ||
      ( (codecFormat) == MPEG4_IMAGE)          ||
      ( (codecFormat) == MPEG4_VIDEO)          ||
      ( (codecFormat) == NONSTD_MPEG4_VIDEO)   ||
      ( (codecFormat) == H263_VIDEO)           ||
      ( (codecFormat) == VP6F_VIDEO)           ||
      ( (codecFormat) == VP8F_VIDEO)           ||
      ( (codecFormat) == VP9_VIDEO)            ||
      ( (codecFormat) == THEORA_VIDEO)         ||
      ( (codecFormat) == SPARK_VIDEO)          ||
      ( (codecFormat) == DIVX311_VIDEO)        ||
      ( (codecFormat) == DIVX40_VIDEO)         ||
      ( (codecFormat) == DIVX50_60_VIDEO)      ||
      ( (codecFormat) == MPEG2_VIDEO)          ||
      ( (codecFormat) == MPEG1_VIDEO)          ||
      ( (codecFormat) == H264_VIDEO)           ||
      ( (codecFormat) == HEVC_VIDEO )          ||
      ( (codecFormat) == MJPEG_VIDEO )         ||
      ( (codecFormat) == REAL_VIDEO ) )
  {
    return true;
  }
  return false;
}

inline bool IS_TEXT_CODEC(uint8 codecFormat)
{
  if( (codecFormat == TIMED_TEXT)||
      (codecFormat == SKT_MOD_TEXT) ||
      (codecFormat == AVI_BITMAP_TEXT) ||
      (codecFormat == AVI_SIMPLE_TEXT) ||
      (codecFormat == SMPTE_TIMED_TEXT)||
      (codecFormat == KARAOKE_TEXT)    ||
      (codecFormat == VOBSUB_TEXT)     ||
      (codecFormat == UTF8_TEXT)       ||
      (codecFormat == USF_TEXT)        ||
      (codecFormat == SSA_TEXT)        ||
      (codecFormat == ASS_TEXT)        ||
      (codecFormat == BITMAP_TEXT)
    )
  {
    return true;
  }
  return false;
}

inline bool MAKE_AAC_AUDIO_CONFIG(uint8* pBuffer,uint16 nAudObjectType,
                                  uint16 nSamplingFreq,uint16 nChannelsConfig,
                                  uint8* pConfigSize)
{
  uint16 configData = 0;
  bool bRet = false;
  if(pConfigSize)
  {
    *pConfigSize = (uint8)sizeof(configData);
    bRet = true;
    if(pBuffer)
    {
      memset(pBuffer,0,sizeof(configData));
      //Syntax of Audio Config is as below
      // MSB 5 bits(15-11) gives Audio Object Type
      // Bits(10-7) gives sampling frequency index
      // Bits(6-3)  gives channel configuration
      // Bits(2-0)  unused as of now
      configData |= (uint16)( (nAudObjectType & 0x001F ) << 11);
      configData |= (uint16)( (nSamplingFreq  & 0x000F)  << 7);
      configData |= (uint16)( (nChannelsConfig& 0x000F)  << 3);
      //Convert to little endian
      uint8* pData = (uint8*)&configData;
      configData = (uint16)(((pData[0])<<8) | pData[1]);
      memcpy(pBuffer,&configData,sizeof(configData));
    }
  }
  return bRet;
}

inline void BYTESWAP(uint8 *pBuf, uint32 ulSize)
{
  for(int nCount = 0; nCount +1 < (int)ulSize; nCount += 2)
  {
    uint8 ucTemp     = pBuf[nCount];
    pBuf[nCount]     = pBuf[nCount + 1];
    pBuf[nCount + 1] = ucTemp;
  }
  return;
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
using namespace video;
#endif

//When following feature is defined,
//media type(mpeg4/WM) will be determined by analysing the file extension.
//This avoids File Open/Close once for each track.
//#define FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class iSourcePort;
/* This is the maximum number of streams that can be accessed through video
** format services.
*/
#if defined(FEATURE_FILESOURCE_WINDOWS_MEDIA) && defined(FEATURE_FILESOURCE_WM_SWITCHING_THINNING)
#define FILE_MAX_MEDIA_STREAMS  12
#else
#define FILE_MAX_MEDIA_STREAMS  6
#endif

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* This structure contains information about a single sample (video frame or
** audio packet) in a stream.
*/
enum AudioDataFormatType
{
  AUDIO_UNKNOWN,
  AUDIO_MIDI,
  NONMP4_AUDIO_MP3,
  NONMP4_AUDIO_AAC,
  AUDIO_QCP,
  AUDIO_VND_QCELP,
  AUDIO_QCF,
  AUDIO_MMF,
  AUDIO_PHR,
  AUDIO_IMELODY,
  AUDIO_ADPCM,
  AUDIO_AAC,
  AUDIO_AMR,
  AUDIO_WMA,
  AUDIO_HVS,
  AUDIO_SAF,
  AUDIO_XMF,
  AUDIO_DLS,
  QCP_QLCM,
  VIDEO_PMD
};
typedef struct
{
  uint32 sample;            /* sample number (first sample is zero)                    */
  uint32 size;              /* size of sample in bytes                                 */
  uint64 offset;            /* offset of sample in stream, in bytes                    */
  uint64 time;              /* composition time of sample, in the media timescale      */
  uint64 delta;             /* difference between composition time of this sample and  */
                            /* the next sample, in the media timescale                 */
  uint32 sync;              /* Indication if sample is a random access point           */
                            /* (non-zero) or not (zero)                                */
  uint32 num_frames;        /* normally it is one, but ASF file can give several       */
                            /* frames in one sample (packet)                           */
  bool   btimevalid;        /* Set to true to indicate if sample time is valid or not  */
  uint32 nBytesLost;        /* Approx bytes lost in the frame(used in wifi display)    */
  uint64 nGranule;          /* Number of valid samples in page(defined in OGG Parser)  */
  uint64 nPCRValue;         /* Recent PCR Value(defined in MP2TS Parser)               */
  FS_EXTRA_SAMPLE_INFOTYPE sSubInfo;  /* Struct to store Extra Data from the media file if any  */
} file_sample_info_type;

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */
uint32 GetBitsFromBuffer( uint32       /*uNeededBits*/,
                          uint32       /*uOffset*/,
                          uint8*       /*pucInputBuffer*/,
                          uint32 const /*uMaxSize*/);

//! copies the data from one buffer to another.
//! If byte swap is used, we may need to reverse the data.
void copyByteSwapData( uint8*  /*pucDstBuf*/,
                       uint32  /*uSize*/,
                       uint8*  /*pucSrcBuf*/,
                       boolean /*byteSwap*/,
                       uint32  /*uAmount*/);

//! This function is used to replace NAL unit size value with NALU start code.
uint32 UpdateAVC1SampleWithStartCode( uint32 /*ulNALULengthSize*/,
                                      uint32 /*ulBufferSize*/,
                                      uint8* /*pucDestBuf*/,
                                      uint8* /*pucSrcBuf*/);

/* ============================================================================
  @brief  This function is used to convert PCM sample data as per requirement

  @details This function is used to convert data from 8bit to 16bit or 24bit to
           16 bit or 24bit to 32bit format.

  @param[in]     pInBuf               Input data buffer.
  @param[in]     pOpBuf               Output data buffer.
  @param[in]     ulInSize             Input buffer size.
  @param[in]     pulOpSize            O/p buffer size.
  @param[in]     ucBitWidth           Bits per Sample field.
  @param[in]     bUpgrade             Flag to indicate whether upgrade
                                      or downgrade of data required.

  @return     None.
  @note       None.
============================================================================ */
void ConvertPCMSampleData(uint8* pInBuf, uint8* pOpBuf, uint32 ulInSize,
                          uint32* pulOpSize, uint8 ucBitWidth, bool bUpgrade);

/* ============================================================================
  @brief  This function is used to copy data from Big Endian format.

  @details This function is used to copy data from BE format to LE format.

  @param[in]     pvDest               Destination buffer.
  @param[in]     nDestSize            Destination buffer size.
  @param[in]     pvSrc                Source buffer.
  @param[in]     nSrcSize             source buffer size.
  @param[in]     pszFields            Word Length (4byte or 8 byte).

  @return     None.
  @note       None.
============================================================================ */
int CopyBE( void*        /*pvDest*/,
            int          /*nDestSize*/,
            const void*  /*pvSrc*/,
            int          /*nSrcSize*/,
            const char*  /*pszFields*/);

/* ============================================================================
  @brief  This function is used to convert lower case letters into upper case.

  @details This function is used to convert lower case letters into upper case.

  @param[in]     ulStrLen               Length of string.
  @param[in/out] pucStr                 String which needs to be converted.

  @return     None.
  @note       None.
============================================================================ */
void ConvertToUpperCase(uint8* pucStr, size_t ulStrLen);
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
  FileBase

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
class FileBase : public ISucceedFail
{
public:

  virtual ~FileBase(){}
  /* Each file format will override following to return appropriate format*/
  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_UNKNOWN;
    return FILE_SOURCE_SUCCESS;
  }
  /* Each file format parser will override following to
     return the actual file size value. */
  virtual uint64 GetFileSize() {return 0;};
  /* Each file format parser will override following to accept configuration enum that it supports*/
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum)
  {
    return FILE_SOURCE_FAIL;
  }
  /* Each file format parser will override following to accept configuration enum that it supports*/
  virtual FileSourceStatus GetAudioOutputMode(bool*,FileSourceConfigItemEnum)
  {
    return FILE_SOURCE_FAIL;
  }
  /* Each file format parser will override following to accept configuration enum that it supports*/
  virtual FileSourceStatus SetConfiguration(FileSourceConfigItemEnum)
  {
    return FILE_SOURCE_FAIL;
  }
  /* Each file format will override following to return appropriate DRM scheme*/
  virtual FileSourceStatus GetDRMType(FileSourceDrmType& drmtype)
  {
    drmtype = FILE_SOURCE_NO_DRM;
    return FILE_SOURCE_SUCCESS;
  }
  /* Each file format will override following to return JANUS header.*/
  virtual FileSourceStatus GetJanusDRMInfo(void*,uint32*)
  {
    return FILE_SOURCE_FAIL;
  }
  /* Each file format will override following to return appropriate number of
   * DRM scheme.In case of common encryption scheme there can be multiple DRM
   * supported by same media file.
   */
  virtual FileSourceStatus GetNumberOfDRMSupported(uint32* pulNoOfDRMSupported)
  {
    *pulNoOfDRMSupported = 0;
    return FILE_SOURCE_SUCCESS;
  }
/* ======================================================================
FUNCTION   : getNextMediaSample
DESCRIPTION: gets next sample of the given track.

INPUT/OUTPUT PARAMETERS:
@param[in] ulTrackID  TrackID requested
@param[in] pucDataBuf DataBuffer pointer to fill the frame(s)
@param[in/out]
           pulBufSize Amount of data request /
                      Amount of data filled in Buffer
@param[in] rulIndex   Index

RETURN VALUE:
 PARSER_ErrorNone in Successful case /
 Corresponding error code in failure cases

SIDE EFFECTS:
  None.
======================================================================*/
  virtual   PARSER_ERRORTYPE getNextMediaSample( uint32  /*ulTrackID*/,
                                                 uint8*  /*pucDataBuf*/,
                                                 uint32* /*pulBufSize*/,
                                                 uint32& /*rulIndex*/)
                             {return PARSER_ErrorDefault;};

  virtual int  GetSizeOfNALLengthField(uint32) {return 0;};

  /* Returns the timestamp for the previously returned media samples from
  // the requested track
  id: The track ID of the track from which the method is to retrieve the sample timestamp.
  return: The timestamp of the most recently return media sample in the "media timescale"
  */
  virtual  uint64 getMediaTimestampForCurrentSample(uint32 /*id*/){return 0;};

  /* resets the playback for given track */
  virtual bool resetMediaPlayback(uint32 /*id*/) {return false;};

  // META DATA APIS

  // From RandomAccessAtom 'rand'
  virtual uint8  randomAccessDenied() {return true;};  // return _randomAccessDenied flag

  // From PVUserDataAtom 'pvmm'
  virtual FILESOURCE_STRING getTitle()const {return NULL;};        // return _title string
  virtual FILESOURCE_STRING getAuthor()const {return NULL;};       // return _author string
  virtual FILESOURCE_STRING getDescription()const {return NULL;};  // return _description string
  virtual FILESOURCE_STRING getRating()const {return NULL;};       // return _rating string
  virtual FILESOURCE_STRING getCopyright()const {return NULL;};    // return _copyright string
  virtual FILESOURCE_STRING getVersion()const {return NULL;};      // return _version string
  virtual FILESOURCE_STRING getCreationDate()const {return NULL;}; // return _creationDate string
  virtual FILESOURCE_STRING getPerf()const {return NULL;};         // return _performance string
  virtual FILESOURCE_STRING getGenre()const{return NULL;};         // return _genre string
  virtual FILESOURCE_STRING getClsf()const{return NULL;};          // return _classification string
  virtual FILESOURCE_STRING getKywd()const{return NULL;};          // return _keyword string
  virtual FILESOURCE_STRING getLoci()const{return NULL;};          // return _location string
  virtual FILESOURCE_STRING getAlbum()const{return NULL;};         // return _location string
  virtual FILESOURCE_STRING getArtist()const{return NULL;};        // return _location string

  virtual PARSER_ERRORTYPE GetClipMetaData(wchar_t*               /*pMetaData*/,
                                           uint32*                /*pLength*/,
                                           FileSourceMetaDataType /*ienumData*/)
                                        {return PARSER_ErrorNotImplemented;};
  virtual FS_TEXT_ENCODING_TYPE GetMetaDataEncodingType() {return FS_ENCODING_TYPE_UNKNOWN;};
  virtual PARSER_ERRORTYPE getAlbumArt( wchar_t* /*pucDataBuf*/,
                                        uint32*  /*pulDatabufLen*/)
                                        {return PARSER_ErrorNotImplemented;};

  virtual bool  getBaseTime(uint32 /*id*/, uint64* /*nbaseTime*/){return false;};
  virtual bool  setBaseTime(uint32 /*id*/, uint64 /*nbaseTime*/){return false;};

  // from 'ftyp' atom
  virtual uint32 getCompatibiltyMajorBrand(){return 0;};
  virtual uint32 getCompatibiltyMajorBrandVersion(){return 0;};
  virtual ZArray<uint32> *getCompatibiltyList(){return NULL;};

  // From Movie
  virtual int32 getNumTracks(){return 0;};
  virtual uint32 getTrackWholeIDList( uint32* /*ids*/){return 0;};
  virtual bool  isAudioPresentInClip(){return false;}


  // From MovieHeader
  virtual uint64 getMovieDuration() const{return 0;};
  virtual uint32 getMovieTimescale() const{return 0;};

  // From Track
  virtual int16 getTrackContentVersion(uint32 /*id*/){return 0;}; // from 'cver' atom at track level
  virtual uint8 trackRandomAccessDenied(uint32 /*id*/){return 0;}; // from 'rand' atom at track level
  virtual float getTrackVideoFrameRate(uint32 /*id*/){return 0;}; // from 'vinf' atom at track level

  virtual uint32 getTrackVideoFrameWidth(uint32 /*id*/) {return 0;};// from pvti atom at file level
  virtual uint32 getTrackVideoFrameHeight(uint32 /*id*/) {return 0;};

  // From TrackReference
  // Returns the track ID of the track on which this track depends
  virtual  uint32  trackDependsOn( uint32) {return 0;}; //tao more work

  // From MediaHeader
  virtual uint64 getTrackMediaDuration(uint32 /*id*/){return 0;};
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/){return 0;};
  virtual uint32 getTrackAudioSamplingFreq(uint32 /*id*/){return 0;};

  virtual uint32 getAudioSamplesPerFrame(uint32 /*id*/){return 0;};

  virtual uint8  getTrackOTIType(uint32 /*id*/) {return 0;};// Based on OTI value
  virtual uint8  getTrackAudioFormat(uint32 /*id*/) {return 0;}; /* based on VideoFMT enum */
  virtual uint8 getFramesPerSample(uint32 /*id*/) {return 0;};
  virtual uint16 getTotalNumberOfFrames(uint32 /*id*/) {return 0;};

  //virtual int32 getTrackOTIValue(uint32 id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 /*id*/){return 0;};
  virtual int32  getTrackAverageBitrate(uint32 /*id*/){return 0;};
  virtual int32  getTrackMaxBitrate(uint32 /*id*/){return 0;};
  virtual int32  getTrackMinBitrate(uint32 /*id*/){return 0;};
  virtual uint32 getLargestFrameSize(uint32 /*id*/){return 0;};
  virtual uint32 getRotationDegrees(uint32 /*id*/){return 0;};

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
  virtual PARSER_ERRORTYPE GetStreamParameter( uint32 /*ulTrackId*/,
                                               uint32 /*ulParamIndex*/,
                                               void*  /*pParameterStructure*/)
  {
    return PARSER_ErrorNotImplemented;
  }
  //! This API is to get required stream param
  virtual PARSER_ERRORTYPE SetStreamParameter( uint32 /*ulTrackId*/,
                                               uint32 /*ulParamIndex*/,
                                               void*  /*pParameterStructure*/)
  {
    return PARSER_ErrorNotImplemented;
  }

  virtual uint32 GetNumAudioChannels(int /*id*/) {return 0;};
  virtual uint32 GetAACAudioProfile(uint32 /*id*/) {return 0;};
  virtual uint32 GetAACAudioFormat(uint32 /*id*/) {return 0;};
  virtual void resetInitialization(){m_bMediaInitialized = false;};
  virtual FILESOURCE_STRING getAudioTrackLanguage(uint32 ) {return 0;};
  virtual unsigned int UserCompare(bool &, int, int, int) {return 0;}

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  /*
  @brief Returns absolute file offset(in bytes) associated with time stamp 'pbtime'(in milliseconds).

  @param[in]  pbtime Timestamp(in milliseconds) of the sample that user wants to play/seek
  @param[out] offset Absolute file offset(in bytes) corresponding to 'pbtime'

  @return true if successful in retrieving the absolute offset(in bytes) else returns false

  @note When there are multiple tracks in a given clip(audio/video/text),API returns
   minimum absolute offset(in bytes)among audio/video/text tracks.
   API is valid only for non-fragmented clips.
  */
  virtual bool GetOffsetForTime(uint64  /*pbtime*/,
                                uint64* /*fileoffset*/,
                                uint32  /*id*/,
                                uint64  /*currentPosTimeStamp*/,
                                uint64& /*reposTime*/)
  {
    return false;
  }

  /*
  @brief Returns absolute file offset(in bytes)
         corresponding to current sample being retrieved via getNextMediaSample.

  @param[in]  trackid TrackId identifying the media track
  @param[out] bError  Indicates if error occured while retrieving file offset.

  @note       bError is set to true when error occurs otherwise, set to false.
  @return Returns absolute file offset(in bytes) that corresponds to
          current sample retrieved via getNextMediaSample.
  */
  virtual uint64 GetLastRetrievedSampleOffset(uint32){return 0;}
#endif

virtual uint32 GetAudioBitsPerSample(int) {return 0;};
#if defined(FEATURE_FILESOURCE_WINDOWS_MEDIA) || defined(FEATURE_FILESOURCE_WMA_PRO_DSP_DECODER)
  /* use these functions only for windows media audio, other formats may not implement it */
  virtual uint32 GetFixedAsfAudioPacketSize(int) {return 0;};
  virtual uint32 GetAudioEncoderOptions(int) {return 0;}

  //Returns audio virtual packet size in bytes.
  virtual uint16 GetAudioVirtualPacketSize(int){return 0;}

  //Returns estimated AUDIO ASF packet duration in msec.
  virtual long   GetAudioASFPacketDuration(int){return 0;}

  //Returns true if asffile has started sending dummy bytes.
   virtual bool  isStartOfDummyBytes(int){return false;}

   virtual uint16 GetAudioAdvancedEncodeOptions(int) {return 0;}
   virtual uint32 GetAudioAdvancedEncodeOptions2(int) {return 0;}
   virtual uint16 GetAudioArmDataReqThr(int) {return 0;}
   virtual uint8  GetAudioValidBitsperSample(int) {return 0;}
   virtual uint16 GetFormatTag(int){return 0;}
   virtual uint32 GetBlockAlign(int /*id*/){return 0;}

#endif /* defined(FEATURE_FILESOURCE_WINDOWS_MEDIA) || defined(FEATURE_FILESOURCE_WMA_PRO_DSP_DECODER)  */
  virtual uint32 GetAudioChannelMask(int) {return 0;}
  virtual bool GetFlacCodecData(int /*id*/,flac_format_data* /*pData*/)
               {return false;}

  virtual long getAudioFrameDuration(int){return 0;}
  virtual void  SetIDX1Cache(void*){}
  virtual void* GetIDX1Cache(){return NULL;}
  virtual void  SetCriticalSection(MM_HANDLE){};

  virtual int32  getNextFragTrackMaxBufferSizeDB(uint32){return 0;};
  virtual void setPausedVideo(boolean){};
  virtual void setPausedAudio(boolean){};
  virtual void setPausedText(boolean){};
  virtual boolean getPausedVideo( void ){return FALSE;};
  virtual boolean getPausedAudio( void ){return FALSE;};
  virtual boolean getPausedText( void ){return FALSE;};
  virtual void resumeMedia( void ){};
  virtual uint16 getParseFragmentNum( void ){return 0;};
  virtual uint16 getReadFragmentNum( void ){return 0;};

  #ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
    virtual bool parsePseudoStream( void ){return FALSE;};
  #endif

  #if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
    virtual void updateBufferWritePtr(uint64){};
    virtual bool parseHTTPStream( void ){return true;}
    virtual bool CanPlayTracks(uint64 /*nTotalPBTime*/) {return true;}
  #endif /* FEATURE_FILESOURCE_PSEUDO_STREAM || FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD */


  #ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    virtual bool getBufferedDuration( uint32  /*id*/,
                                      int64   /*nBytes*/,
                                      uint64* /*pBufferedTime*/)
                                     {return FALSE;}
  #endif /* FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD */

  virtual uint64 repositionAccessPoint(int32 /*skipNumber*/, uint32 /*id*/,
                                       bool& bError ,
                                       uint64 /*currentPosTimeStampMsec*/ )
  {
     bError = true;
     return 0;
  }

  virtual void  getCodecDatainRawFormat(bool /*bRawCodec*/) {return;};
  virtual uint8 *getTrackDecoderSpecificInfoContent(uint32){return NULL;};
  virtual uint32 getTrackDecoderSpecificInfoSize(uint32){return 0;};
  virtual uint8 *getTrackDecoderSpecificInfoAtSDI(uint32, uint32){return NULL;};
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32  /*id*/,
                                                              uint8*  /*buf*/,
                                                              uint32* /*pSize*/)
                      {return PARSER_ErrorInvalidParam;}

  // Static method to read in an MP4 file from disk and return the FileBase interface

static FileBase *openMediaFile(  FILESOURCE_STRING /*filename*/,
                                   bool /*bPlayVideo*/,
                                   bool /*bPlayAudio*/,
                                   bool /*bPlayText*/,
                                   bool /*blookforcodechdr*/,
                                   FileSourceFileFormat format = FILE_SOURCE_UNKNOWN);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  static FileBase *openMediaFile(video::iStreamPort* /*pPort*/,
                                 bool /*bPlayVideo*/,
                                 bool /*bPlayAudio*/,
                                 bool /*bPlayText*/,
                                 bool /*blookforcodechdr*/,
                                 FileSourceFileFormat format = FILE_SOURCE_UNKNOWN);
#endif

#ifdef FEATURE_FILESOURCE_DRM_DCF

  // DRM DCF file support
  static FileBase *openMediaFile(  IxStream* inputStream,
                                   bool bPlayVideo,
                                   bool bPlayAudio,
                                   bool bPlayText);
#endif

  static FileBase *openMediaFile(  unsigned char* /*pBuf*/,
                                   uint32 /*bufSize*/,
                                   bool /*bPlayVideo*/,
                                   bool /*bPlayAudio*/,
                                   bool /*bPlayText*/
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || \
    defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
                                   ,bool bPseudoStream = false
                                   ,uint32 wBufferOffset = 0
#endif  /* FEATURE_FILESOURCE_PSEUDO_STREAM */
        /* FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD */
                                   ,FileSourceFileFormat format = FILE_SOURCE_UNKNOWN
                                 );

  virtual bool RegisterDRMDecryptMethod(DRMDecryptMethodT,void*) { return false; }

  virtual void resetPlayback() {};
  virtual uint64 resetPlayback(uint64 /*pos*/, uint32 /*id*/,
                               bool /*bSetToSyncSample*/, bool* /*bError*/,
                               uint64 /*currentPosTimeStamp*/) {return 0;};

  virtual uint64 skipNSyncSamples(int, uint32, bool *, uint64 ){return 0;};

  virtual void resetPlaybackPos(uint32){return;}

  virtual bool parseFragment( void ){return FALSE;};
  virtual void setAudioPlayerData(const void *){};
  virtual void setVideoPlayerData(const void *){};
  virtual void setTextPlayerData(const void *){};

  virtual uint8 getAllowAudioOnly() {return 0;};
  virtual uint8 getAllowVideoOnly() {return 0;};

  virtual PARSER_ERRORTYPE peekCurSample(uint32 /*trackid*/,
                                        file_sample_info_type* /*pSampleInfo*/)
          {return PARSER_ErrorInvalidParam;};

  virtual bool HasMetaData() {return false;};

  /*KDDI Meta Data Related APIs*/

  // From CopyGaurdAtom
  virtual uint32 getCopyProhibitionFlag(){return 0;};
  virtual uint32 getValidityEffectiveDate(){return 0;};
  virtual uint32 getValidityPeriod(){return 0;};
  virtual uint32 getNumberofAllowedPlayBacks(){return 0;};

  // From Content Property Atom
  virtual FILESOURCE_STRING getContentPropertyTitle(){return NULL;};
  virtual FILESOURCE_STRING getContentPropertyCopyRight(){return NULL;};
  virtual FILESOURCE_STRING getContentPropertyAuthor(){return NULL;};
  virtual FILESOURCE_STRING getContentPropertyMemo(){return NULL;};
  virtual uint32      getAuthorDLLVersion(){return 0;};

  //From Movie Mail Atom
  virtual uint32 getEditFlags(){return 0;};
  virtual uint8  getRecordingMode(){return 0;};
  virtual uint32 getRecordingDate(){return 0;};

  //From Encoder Information Atom
  virtual uint8*  getDeviceName() const {return NULL;};
  virtual uint8*  getModelName() const {return NULL;};
  virtual uint8*  getEncoderInformation() const {return NULL;};
  virtual uint8*  getMuxInformation() const {return NULL;};

  //From GPS Atom
  virtual uint16  getGPSByteOrder(){return 0;};
  virtual uint32  getGPSVersionID(){return 0;};
  virtual uint32  getGPSLatitudeRef(){return 0;};
  virtual uint32  getGPSLongitudeRef(){return 0;};
  virtual uint32  getGPSAltitudeRef(){return 0;};
  virtual uint64 *getGPSLatitudeArray(){return NULL;};
  virtual uint64 *getGPSLongitudeArray(){return NULL;};
  virtual uint64 getGPSAltitude(){return 0;};
  virtual uint64 *getGPSTimeArray(){return NULL;};
  virtual FILESOURCE_STRING getGPSSurveyData(){return NULL;};
  virtual FILESOURCE_STRING getPositoningMethod(){return NULL;};
  virtual FILESOURCE_STRING getPositioningName(){return NULL;};
  virtual FILESOURCE_STRING getGPSDate(){return NULL;};
  virtual uint64 getGPSExtensionMapScaleInfo(){return 0;};

  virtual bool IsMidiDataPresent(){return false;};
  virtual uint32 GetMidiDataSize(){return 0;};
  virtual uint32 GetMidiData(uint8 *, uint32, uint32){return 0;};

  virtual bool IsLinkDataPresent() {return false;}
  virtual uint32 GetLinkDataSize() {return 0;}
  virtual uint32 GetLinkData(uint8 *, uint32) {return 0;}

  virtual bool IsDataPresent(DataT, uint32) {return false;}
  virtual uint32 GetDataSize(DataT, uint32){return 0;}
  virtual uint32 GetData(DataT, uint8 *, uint32, uint32)
  {return 0;}
  virtual bool isGenericAudioFileInstance() {return false;};

  virtual bool isAviFileInstance(){return false;}
  virtual bool isAC3FileInstance(){return false;}
  virtual bool isM2TSFileInstance(){return false;}
  virtual bool IsDRMProtection(){return false;}
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  virtual void CopyDRMContextInfo(void*){}
  virtual void GetClipDrmInfo(void*){}
  virtual bool CommitDivXPlayback(){return false;}
#endif

  virtual EncryptionTypeT getEncryptionType() {return ENCRYPT_NONE;};
  virtual EncryptionTypeT getEncryptionType(uint32) {return ENCRYPT_NONE;};

  virtual bool setAudioInfo(FileSourceAudioInfo){ return false;}
  virtual bool setVideoInfo(FileSourceVideoInfo){ return false;}

protected:

        /*
        * When a media/interface is created,
        * m_bMediaInitialized will be initialized to FALSE to indicate that
        * initialization is not yet done. This is particularly
        * useful when playing generic audio formats as CMX expects
        * certain commands only when their internal state is initialized.
        *
        * For generic audio formats, when posting Seek command,
        * Mpeg4Player will wait on this flag(inside GenericAudioFile::ResetPlayback)before issuing SEEK commands to CMX.
        */
        bool m_bMediaInitialized;

        /*
        *When set, media(mpeg4file/avifile/asffile)will abort the
        *seek/read operations to stop the playback gracefully.
        *For example, If media is busy in doing the seek and if user
        *wants to stop the playback, FILESOURCE will invoke setMediaAbort API
        *and subsequent READ will fail in Media thus breaking the Seek operation.
        */
        bool m_bMediaAbort;

  #ifdef FEATURE_FILESOURCE_DRM_DCF
     /*
      * To read few bytes from IxStream to determine if it is
      * 3GP/WM.
      */
  static uint32 readFile(IxStream* inputStream, uint8 *buffer,
                         uint32 pos, uint32 size );

  #endif

  static uint32 readFile( FILESOURCE_STRING filename,
                          uint8 *buffer,
                          uint64 pos,
                          uint32 size );

  static uint32 readFile( OSCL_FILE *fp,
                          uint8 *buffer,
                          uint64 pos,
                          uint32 size,
                          bool *pbDataUnderRun = NULL,
                          bool bMediaAbort = false);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  static uint32 readFile( video::iStreamPort* pPort,
                          uint8 *buffer,
                          uint64 pos,
                          uint32 size,
                          bool *pbDataUnderRun = NULL);
#endif
  static int32 seekFile ( OSCL_FILE *fp,
                          uint64 offset,
                          uint32 origin = SEEK_SET);

public:
#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
   static bool IsGenericAudioFile(FILESOURCE_STRING filename);
#endif

#endif /* FEATURE_FILESOURCE_WINDOWS_MEDIA */
  static bool IsAC3File(FILESOURCE_STRING,uint8*,bool);
  static bool IsAVIFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsASFFile(uint8 * pBuf, uint32 size);
  static bool IsASFFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsRMFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsAMRFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsAMRWBFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsEVRCWBFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsEVRCBFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsMP3File(FILESOURCE_STRING,void **,bool,IxStream* ixstream=NULL);
  static bool IsAACFile(FILESOURCE_STRING,void **,bool,IxStream* ixstream=NULL);
  static bool IsQCPFile(FILESOURCE_STRING,uint8*,bool,IxStream* ixstream=NULL);
  static bool IsWAVADPCMFile(FILESOURCE_STRING,uint8*,bool,IxStream* ixstream=NULL);
  static bool IsMPEG2File(FILESOURCE_STRING /*filename*/,uint8* /*pBuf*/,
                          uint32 /*ulbufSize*/, bool /*bExtension*/,
                          bool* pbIsProgStream=NULL);
  static bool IsMPEG1VideoFile(FILESOURCE_STRING, uint8*);
  static bool IsOggFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsFlacFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsMP4_3GPFile(FILESOURCE_STRING /* filename*/,uint8* /*pbuf*/,
                            uint32/*bufsize*/, bool /*bUseExtension*/);
  static bool Is3GPP2File(FILESOURCE_STRING,uint8*, uint32,bool);
  static bool IsMKVFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsFLVFile(FILESOURCE_STRING,uint8*,bool);
  static bool IsRawH265File(FILESOURCE_STRING,uint8*,bool);
  static bool IsID3TagPresent(uint8* pucDataBuf,uint32* pulID3TagLen);
  static bool IsDTSFile(uint8* pBuf, uint32 ulBufSize);

  static PARSER_ERRORTYPE IsMP3Format (uint8* pucFrameBuff,
                                       uint32  ulFrameBufLen );
  static PARSER_ERRORTYPE IsAACFormat (uint8*  pucFrameBuff,
                                       uint32  ulFrameBufLen);

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  static bool IsMP3File(video::iStreamPort*,void **);
  static bool IsAACFile(video::iStreamPort*,void **);
  static bool IsAVIFile(video::iStreamPort* pPort,uint8*,bool);
  static bool IsRawH265File(video::iStreamPort* pPort,uint8*,bool);
#endif

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  static FileBase* initFormatParser ( video::iStreamPort* pPort,
                                      bool bPlayVideo,
                                      bool bPlayAudio,
                                      bool bPlayText,
                                      bool blookforcodechdr,
                                      FileSourceFileFormat format );
#endif

public:
   virtual void setMediaAbort(){m_bMediaAbort = true;}

};

#endif
