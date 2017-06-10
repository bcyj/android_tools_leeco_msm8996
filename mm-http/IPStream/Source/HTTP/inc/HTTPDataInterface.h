#ifndef __HTTPDATAINTERFACE_H__
#define __HTTPDATAINTERFACE_H__
/************************************************************************* */
/**
 * HTTPDataInterface.h
 * @brief Header file for HTTP Media Types.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPDataInterface.h#14 $
$DateTime: 2012/09/18 01:46:50 $
$Change: 2813413 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPCommon.h
** ======================================================================= */

namespace video
{
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
 #define HTTP_MAX_SUBTITLE_SUB_INFO_SIZE  2*1024
 #define HTTP_MAX_ENCRYPTED_ENTRY 256

 #define MAX_ENCRYPTED_ENTRY         256
 #define MAX_KID_SIZE                 16
 #define MAX_IV_SIZE                  16
 #define MAX_SYSTEMID_SIZE            16

/*
 * Enumeration to specify media minor type.
 */
typedef enum
{
  HTTP_MINOR_TYPE_UNKNOWN,
  HTTP_MINOR_TYPE_AAC,
  HTTP_MINOR_TYPE_AAC_ADTS,
  HTTP_MINOR_TYPE_AAC_ADIF,
  HTTP_MINOR_TYPE_AAC_LOAS,
  HTTP_MINOR_TYPE_AC3,
  HTTP_MINOR_TYPE_EAC3,
  HTTP_MINOR_TYPE_MP2,
  HTTP_MINOR_TYPE_MPEG2,
  HTTP_MINOR_TYPE_H264,
  HTTP_MINOR_TYPE_HVC,
  HTTP_MINOR_TYPE_SMPTETT,
  HTTP_MINOR_TYPE_PNG
} HTTPMediaMinorType;

typedef enum
{
  PSSH_INFO_NONE,
  PSSH_INFO_NEW,
  PSSH_INFO_QUERIED
} PsshInfoStatus;



/*
 * Structure to store DRM information.
 */
typedef struct
{
  int32 trackID;
  int nUniqueID;
  FileSourceMjMediaType majorType;
  PsshInfoStatus     ePsshInfoStatus;
  bool               isDRMProtected;
  HTTPContentStreamType streamType;
  //DRM Type
  FileSourceDrmType  eDrmType;
  //! Index associated with protection scheme
  uint32             ulDRMIndex;
  //DRM SystemID size
  uint32             systemIdBufSize;
  //number of kid mapping entry
  uint32             kidCount;
  // KID mapping data buffer size
  uint32             kidDataBufSize;
  // PSSH data buffer size
  uint32             psshDataBufSize;
  // ContentProtection Size
  uint32             cpBufSize;
  // ContentProtection Size
  uint32             defaultKeyIDSize;
  char               systemID[MAX_SYSTEMID_SIZE];
  char               defaultKeyID[MAX_KID_SIZE];
  char               *pKidDataBuf;
  char               *pPsshDataBuf;
  char               *pCpDataBuf;
}HTTPDrmInfo;


//ToDo: Can define a base struct containing common info such as
//timeScale, duration etc.
/*
 * Defines video track information needed to configure video decoder.
 */
typedef struct
{
  // subdivision of level
  uint32      layer;
  // time scale of the track
  uint32      timeScale;
  // total duration fo the track in milliseconds
  uint64      duration;
  // bit rate
  uint32      bitRate;
  // frames per second
  float       frameRate;
  // frame width in pixels
  uint32      frameWidth;
  // frame height in pixels
  uint32      frameHeight;
  // max sample size in bytes
  uint32      maxSampleSize;
  // Drm related information
  HTTPDrmInfo drmInfo;
}HTTPVideoTrackInfo;

/*
 * Defines Audio track information needed to configure audio decoder.
 */
typedef struct
{
  // number of audio channels
  uint32      numChannels;
  // time scale of the track
  uint32      timeScale;
  // total duration fo the track in milliseconds
  uint64      duration;
  // bit rate
  uint32      bitRate;
  // min bit rate
  uint32      minBitRate;
  // max bit rate
  uint32      maxBitRate;
  // sampling rate
  uint32      samplingRate;
  // number of valid bits per sample
  uint32      nBitsPerSample;
  // block align, applicable for PCM codec especially
  uint32      nBlockAlign;
  // max sample size in bytes
  uint32      maxSampleSize;
  // Drm related information
  HTTPDrmInfo drmInfo;
}HTTPAudioTrackInfo;

/*
 * Defines Text track information needed to configure Text decoder.
 */
typedef struct
{
  // subdivision of level
  uint32      layer;
    // time scale of the track
  uint32      timeScale;
  // total duration fo the track in milliseconds
  uint64      duration;
  // max sample size in bytes
  uint32      maxSampleSize;
  // height
  uint32 nHeight;
  //width
  uint32 nWidth;
  // Drm related information
  HTTPDrmInfo drmInfo;
}HTTPTextTrackInfo;

/*
 * Defines track information needed to configure decoder.
 */
typedef struct
{
  HTTPCommon::HTTPMediaType majorType;
  HTTPMediaMinorType minorType;
  uint32 nTrackID;
  bool bSelected;
  union
  {
    HTTPAudioTrackInfo audioTrackInfo;
    HTTPVideoTrackInfo videoTrackInfo;
    HTTPTextTrackInfo textTrackInfo;
  };
} HTTPMediaTrackInfo;

/*
 * Enumeration to specify AAC profile type.
 */
typedef enum
{
  HTTP_AAC_PROFILE_NULL = 0,
  HTTP_AAC_PROFILE_MAIN,          // MAIN
  HTTP_AAC_PROFILE_LC,            // LC
  HTTP_AAC_PROFILE_SSR,           // SSR
  HTTP_AAC_PROFILE_LTP,           // LTP
  HTTP_AAC_PROFILE_HE             // HE
} HTTPAACProfile;

/*
 * Enumeration to specify AAC stream format.
 */
typedef enum
{
  HTTP_AAC_FORMAT_UNKNWON,  // UNKNWON
  HTTP_AAC_FORMAT_ADTS,     // ADTS
  HTTP_AAC_FORMAT_ADIF,     // ADIF
  HTTP_AAC_FORMAT_RAW,      // RAW
  HTTP_AAC_FORMAT_LOAS      // LOAS
} HTTPAACStreamFormat;

/*
 * Defines AAC format block needed to configure AAC decoder.
 */
typedef struct
{
  // Identifies AAC profile type
  HTTPAACProfile        eAACProfile;
  // Identifies AAC Stream format
  HTTPAACStreamFormat   eAACStreamFormat;
}HTTPAACCodecData;

/*
 * Defines AAC format block needed to configure AAC decoder.
 */
typedef union
{
  // Identifies AAC profile type
  HTTPAACCodecData        aacCodecData;
}HTTPCodecData;

/**
 * enumeration for subs type in Subtitles
 */
typedef enum
{
  HTTP_SUB_UNKNOWN,
  HTTP_SUB_BITMAP_TEXT
}HTTPSubTitleSubsMediaType;


/*
 * Structure to store subtitle track sub sample information.
 */
typedef struct {
  uint16 subSampleCount;  /* Number of subsample count present for current sample */
  HTTPMediaMinorType eSubMnType; /* sample minor type. In case of smpte-tt a subsample
                                   can have IMAGE or FONT as subsample */
  uint16 subtitleSubInfoSize;    /* Subtitle sub information size*/
  uint8 cSubtitleSubInfo[HTTP_MAX_SUBTITLE_SUB_INFO_SIZE]; /* Subtitle sub info data */
}HTTPSubTitleSubInfo;


/*
 * Structure to store subsample encryption information in a sample
 */
typedef struct
{
  //! Number of bytes of clear data in subsample
  uint16 usSizeOfClearData;
  // Offset of clear data in subsample from start
  uint32 ulOffsetClearData;
  //! Number of bytes of encrypted data in subsample
  uint32 ulSizeOfEncryptedData;
  //! Offset of encrypted data in subsample from start
  uint32 ulOffsetEncryptedData;
}HTTPEncryptedSubSampleType;


/*
 * Structure to store content protection information
 */
typedef struct {
  //! Number of encrypted subsample count present in current sample
  uint16 ulEncrSubSampleCount;
  //! Encryption state flag. 0x0: Not Encrypted, 0x1: Encrypted
  //! 0x1:- AES-CTR, 0x2 - 0xFFFF:- Reserved
  uint16 ucIsEncrypted;
  //! Key Identifier size
  uint8 ucKeyIDSize;
  //! Initialization Vector size
  uint8 ucInitVectorSize;
  //! Default Key Identifier for to decrypt the associated samples
  uint8 ucDefaultKeyID[MAX_KID_SIZE];
  //! Key Identifier for to decrypt the associated samples
  uint8 ucKeyID[MAX_KID_SIZE];
  //! Initialization vector for sample decryption
  uint8 ucInitVector[MAX_IV_SIZE];
  //! Encrypted subsample information, it will carry information
  //! of bytes of clear/encrypted data and its offset from start.
  HTTPEncryptedSubSampleType sEncSubsampleInfo[HTTP_MAX_ENCRYPTED_ENTRY];
}HTTPContentProtectionType;


/*
 *@ Structure to store extra sample information.
 */
typedef struct {
  uint64 periodStartTime; /* period starttime*/
  HTTPSubTitleSubInfo sSubTitle; /* Subsample information for subtitle track */
  HTTPContentProtectionType sContentProtection; /* Content protection information applicable to current sample */
}HTTPSampleExtraInfo;


/*
 * Structure to store media sample information.
 */
typedef struct
{
  // composition time(ms) of sample, in the media timescale
  uint64 startTime;

  // timestamp of the next sample in milli seconds
  uint64 endTime;

  // difference between composition time of this sample and
  // the next sample, in the media timescale
  uint64 delta;

  // Indication if sample is a random access point(non-zero) or not (zero)
  uint32 sync;

  // True indicates if startTime is valid
  bool   bStartTsValid;

  // Extra Sample info
  HTTPSampleExtraInfo sSubInfo;
}HTTPSampleInfo;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class HTTPDataInterface
{
public:
  virtual ~HTTPDataInterface() {};

  /*
   * method to retrieve all valid track information. Pass NULL for trackIdInfo
   * to get total number of valid tracks. Allocate the memory and call again
   * with non NULL pointer.
   *
   * @param[out] pTrackInfo Pointer to HTTPMediaTrackInfo,
   *
   * @return  number of valid tracks.
   */
  virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo *pTrackInfo ) = 0;

  /*
   * method to retrieve track information for particular track.
   *
   * @param[in] HTTPMediaType major media type for which track information
   * needs to be retreived
   * @param[out] TrackInfo populated the track information on success
   *
   * @return HTTPDL_SUCCESS if successful in retrieving the track information
   * HTTPDL_WAITING if the information is not avaliable check back later
   * else failure
   */
  virtual HTTPCommon::HTTPDownloadStatus GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType majorType,
                                                                   HTTPMediaTrackInfo &TrackInfo ) = 0;

  virtual HTTPCommon::HTTPDownloadStatus GetContentProtectElem(uint32 portIndex,
                                                  HTTPMediaType mediaType,
                                                  HTTPDrmType &drmType,
                                                  HTTPContentStreamType &streamType,
                                                  uint32 &contentProtectionInfoSize,
                                                  unsigned char* contentProtectionData)=0;

  /*
   * method to retrieve current playing representation
   * @param[in]  majorType    : major type of current representation
   * @param[out] currRepeSize : current representation size
   * @param[out] currRepeSring : current representation string
   *
   * @return HTTPDL_SUCCESS if successful in retrieving the track information
   * else failure
   */

  virtual HTTPCommon::HTTPDownloadStatus GetCurrentPlayingRepInfo(uint32 &currRepSize,
                                                            unsigned char *currRepSring = NULL) = 0;

  /*
   * method to retrieve track encoding types from MPD
   * @param[out] audio : audio minor type
   * @param[out] video : video minor type
   * @param[out] other : text minor type
   *
   * @return HTTPDL_SUCCESS if successful in retrieving the track information
   * else failure
   */

  virtual HTTPCommon::HTTPDownloadStatus GetTrackEncodingInfo(FileSourceMnMediaType& audio,
                                                              FileSourceMnMediaType& video,
                                                              FileSourceMnMediaType& other) = 0;

  /*
   * Retrieve the codec specfic data needed to configure decoder
   *
   * @param[in] nTrackID   Identifies the track for which codec data needs to
   * be retrieved
   * @param[in] HTTPMediaType major media type for which track information
   * needs to be retreived
   * @param[in] minorType   media minor type for which the codec info is being
   * requested
   * @param[out] AACCodecData populates the codec data on success
   *
   * @return HTTPDL_SUCCESS if successful in retrieving the codec data
   * HTTPDL_WAITING if the information is not avaliable check back later
   * else failure
   */
  virtual HTTPCommon::HTTPDownloadStatus GetCodecData(uint32 nTrackID,
                                                      HTTPCommon::HTTPMediaType majorType,
                                                      HTTPMediaMinorType minorType,
                                                      HTTPCodecData &CodecData) = 0;

  /*
   * Method retrieves the Decoder specific/Format Block information for the
   * given track. If buf = NULL, then the function give the size of the required
   * buffer/format block.
   *
   * Following is an example of retrieving the format block.
   * 1. Invoke getFormatBlock for a given track identifier by passing in
   *    NULL for buf.
   * 2. If a track is valid, *pbufSize will give you the size of format block
   *    that exist.
   * 3  Allocate the memory and invoke getFormatBlock API for a given track
   *    identifier by passing handle to allocated memory.
   *
   * @param[in] majorType media type.
   * @param[out] pBuffer  Buffer provies the format block info to the caller
   * @param[out] pbufSize Size of the FormatBlock buffer
   *
   * @return HTTPDL_SUCCESS if successful in retrieving the format block info
   * HTTPDL_WAITING if the information is not avaliable check back later
   * else failure
   */
  virtual HTTPCommon::HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType majorType,
                                                        uint8* pBuffer,
                                                        uint32 &nbufSize) = 0;

 /*
  * Get the next media sample for the specified media type
  *
  * @param[in] HTTPMediaType media type.
  * @param[out]pBuffer  A pointer to the buffer into which to place the sample.
  * @param[out] nSize The size of the data buffer.
  * @param[out] sampleInfo Provides information about the sample
  *
  * @return HTTPDL_SUCCESS if successful in retrieving the format block
  * HTTPDL_WAITING the information is not avaliable check back later
  * else failure
  */
  virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample( HTTPCommon::HTTPMediaType,
                                                             uint8 *pBuffer,
                                                             uint32 &nSize,
                                                             HTTPSampleInfo &sampleInfo) = 0;

 /*
  * Get the current playback position for the specified media type
  *
  * @param[in] HTTPMediaType media type.
  * @param[out] nPlaybackPosition populates in time uints on success
  *
  * @return true if successful else failure
  */
  virtual bool GetCurrentPlaybackPosition( HTTPCommon::HTTPMediaType,
                                           uint64 &nPlaybackPosition) = 0;

 /*
  * Get the current buffered duration from the playback position for the given
  * media type
  *
  * @param[in] HTTPMediaType media type.
  * @param[out] nPlaybackPosition populates in time units on success
  * @param[out] nBufferedDuration populates buffered duration in time units
  *
  * @return true if successful else failure
  */
 virtual bool GetDurationBuffered( HTTPCommon::HTTPMediaType mediaType,
                                  uint64 &nPlaybackPosition,
                                  uint64 &nBufferedDuration ) = 0;

 /*
  * Gets the value of the given attribute for a given media type
  *
  * @param[in] HTTPMediaType media type.
  * @param[in] HTTPAttribute attrType attribute for which value
                should be retrieved
  * @param[out] HTTPAttrVal  attrVal value of the given attribute
  *
  * @return true if successful else failure
  */
 virtual bool GetConfig( HTTPCommon::HTTPMediaType mediaType,
                         HTTPCommon::HTTPAttribute attrType,
                         HTTPCommon::HTTPAttrVal& attrVal ) = 0;

 /*
  * Set the value for the given attribute and given media type
  *
  * @param[in] HTTPMediaType media type.
  * @param[in] HTTPAttribute attrType attribute to be set
  * @param[in] HTTPAttrVal  attrVal attribute value to be set
  *
  * @return true if successful else failure
  */
 virtual bool SetConfig( HTTPCommon::HTTPMediaType mediaType,
                         HTTPCommon::HTTPAttribute attrType,
                         HTTPCommon::HTTPAttrVal attrVal  ) = 0;
};

}// namespace video

#endif // __HTTPDATAINTERFACE_H__
