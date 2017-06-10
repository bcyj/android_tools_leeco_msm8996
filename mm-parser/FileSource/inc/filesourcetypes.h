#ifndef _FILE_SOURCE_TYPES_H_
#define _FILE_SOURCE_TYPES_H_
/*=======================================================================
                             filesourcetypes.h
DESCRIPTION
  Definiton of data types used by Filesource Module.

Copyright (c) 2008-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*======================================================================== */
/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileSource/main/latest/inc/filesourcetypes.h#109 $$
$DateTime: 2014/05/13 22:50:58 $$
$Change: 5885284 $$
========================================================================== */
#include <wchar.h>
#include "AEEStdDef.h"

//!FileSource is available as DLL on Windows* platform
#if defined WINCE || defined PLATFORM_LTK
  #define FILESOURCE_DLL __declspec( dllexport )
#else
  #define FILESOURCE_DLL
#endif

//! Macros to check the status of the FileSource
#define FILE_SOURCE_SUCCEEDED(x) (x == FILE_SOURCE_SUCCESS)
#define FILE_SOURCE_FAILED(x)    (x != FILE_SOURCE_SUCCESS)

/* The buf size calculated is not correct for low resolution clips such as
   32 X 32.If buffer size is not sufficient to copy frame, FS will return
   FILE_SOURCE_DATA_REALLOCATE_BUFFER as status, which resulted into port
   reconfiguration. In order to avoid such problems, FS will request minimum
   buffer size equivalent to SQCIF resolution. */
#define MIN_VIDEO_FRAME_BUFFER_SIZE (128 * 96 * 1.2)

/* Maximum size of data the ExtraData buffer can hold */
#define MAX_EXTRA_DATA_SIZE 32

// Max Picture description length in ID3 tags.
// 1 extra byte is to store NULL char at the end
#define MAX_DESC_LEN 68

//Image format string length. This string is also NULL terminated
#define MAX_IMG_FORMAT_LEN 32

#define MIN_FRAME_HEADER_SIZE 6

#define MAX_SUBTITLE_SUB_INFO_SIZE 2048
#define MAX_ENCRYPTED_ENTRY         256
#define MAX_KID_SIZE                 16
#define MAX_IV_SIZE                  16
#define MAX_SYSTEMID_SIZE            16

//! forward declaration for istreamport
namespace video
{
  class iStreamPort;
}

//! forward declaration for IxStream
class IxStream;

//! forward declaration for helper class
class FileSourceHelper;

/*!
*@brief Enumeration to track the status returned by various
*       FileSource interfaces.
*/
typedef enum
{
  //! Enumeration for FileSource status
  FILE_SOURCE_SUCCESS            = 0,   //! Default  Success
  FILE_SOURCE_FAIL               = 1,   //! Default Error/Fail
  FILE_SOURCE_NOTAVAILABLE       = 2,   //! File Source is not intialized yet
  FILE_SOURCE_DATA_NOTAVAILABLE  = 3,   //! File Source does not have the Infomation Yet.
  FILE_SOURCE_FILE_NONEXIST      = 4,   //! Regquested File/buffer does not exist
  FILE_SOURCE_INVALID            = 5,   //! Invalid parameter passed to the function
} FileSourceStatus;

/*!
*@brief Enumeration to track FileSource Media Status.
*/
typedef enum
{
  FILE_SOURCE_DATA_OK,
  FILE_SOURCE_DATA_END,
  FILE_SOURCE_DATA_UNDERRUN,
  FILE_SOURCE_DATA_ERROR,
  FILE_SOURCE_DATA_MALLOC_FAILED,
  FILE_SOURCE_DATA_INIT,
  FILE_SOURCE_DATA_FRAGMENT,
  FILE_SOURCE_DATA_REALLOCATE_BUFFER,
  FILE_SOURCE_DATA_CODEC_INFO,
  FILE_SOURCE_DATA_INSUFFICIENT,
  FILE_SOURCE_DATA_NOT_IMPLEMENTED,
  FILE_SOURCE_DATA_INVALID              //! Invalid parameter passed to the function
} FileSourceMediaStatus;

/*!
*@brief Enumeration identifying the specific parser error.
* When user receives OPEN_FAIL for OpenFile API,
* GetFileError API will provide more information about the error.
*/
typedef enum
{
  FILE_SOURCE_PARSER_OK,
  FILE_SOURCE_PARSER_UNKNOWN_ERROR,
  FILE_SOURCE_PARSER_NO_MEDIA_TRACKS,
  FILE_SOURCE_PARSER_PARSE_ERROR,
  FILE_SOURCE_PARSER_DRM_OUT_OF_MEMORY,
  FILE_SOURCE_PARSER_DRM_NO_AUTHORIZATION,
  FILE_SOURCE_PARSER_DRM_DEVICE_NOT_REGISTERED,
  FILE_SOURCE_PARSER_DRM_RENTAL_COUNT_EXPIRED,
  FILE_SOURCE_PARSER_DRM_PLAYBACK_FAIL
}FileSourceParserError;

/*!
*@brief Enumeration identifying the media file format.
*/
typedef enum
{
  FILE_SOURCE_UNKNOWN,  //unknown                   <0>
  FILE_SOURCE_AC3,      //.ac3                      <1>
  FILE_SOURCE_AAC,      //.aac                      <2>
  FILE_SOURCE_AMR_NB,   //.amr                      <3>
  FILE_SOURCE_AMR_WB,   //.awb                      <4>
  FILE_SOURCE_ASF,      //.wma,.wmv,.asf            <5>
  FILE_SOURCE_AVI,      //.avi,.divx                <6>
  FILE_SOURCE_EVRCB,    //.evb                      <7>
  FILE_SOURCE_EVRC_WB,  //.ewb                      <8>
  FILE_SOURCE_FLAC,     //.flac                     <9>
  FILE_SOURCE_MPEG4,    //.3gp,.mp4,.k3g,.skm,.mp4a <10>
  FILE_SOURCE_3G2,      //.3g2                      <11>
  FILE_SOURCE_MP2PS,    //.vob,.mp2,.ps,.mpg        <12>
  FILE_SOURCE_MP2TS,    //.ts,.m2ts                 <13>
  FILE_SOURCE_MKV,      //.mkv                      <14>
  FILE_SOURCE_MP3,      //.mp3                      <15>
  FILE_SOURCE_OGG,      //.ogg                      <16>
  FILE_SOURCE_QCP,      //.qcp                      <17>
  FILE_SOURCE_WAV,      //.wav                      <18>
  FILE_SOURCE_RAW_H265, //.h265                     <19>
  FILE_SOURCE_DTS,      //.dts, .dtshd, .cpt        <20>
  FILE_SOURCE_MP4_DASH, //New Enum for DASH         <21>
  FILE_SOURCE_REAL,     //.rm                       <22>
  FILE_SOURCE_FLV,      //.flv                      <23>
  FILE_SOURCE_WEBM,     //.webm                     <24>
  FILE_SOURCE_WFD_MP2TS, // .ts for WFD             <25>
  FILE_SOURCE_DASH_MP2TS,// .ts for DASH             <26>
  FILE_SOURCE_RAW,       //Special case. File might be having raw pcm/aac etc.
} FileSourceFileFormat;

/*!
*@brief Enumeration identifying the DRM type.
*/
typedef enum
{
  FILE_SOURCE_NO_DRM,       //There is no DRM
  FILE_SOURCE_DRM_UNKNOWN,  //There is some DRM but failed to identify the scheme
  FILE_SOURCE_DIVX_DRM,     //Underlying file is protected using DIVX DRM
  FILE_SOURCE_JANUS_DRM,    //Underlying file is protected using JANUS/WM DRM
  FILE_SOURCE_PLAYREADY_DRM,//Underlying file is protected using PLAYREADY DRM
  FILE_SOURCE_CENC_DRM,     //Underlying file is protected using common encryption scheme
  FILE_SOURCE_MARLIN_DRM,   //Underlying file is protected using MARLIN DRM
  FILE_SOURCE_HDCP_DRM      //Underlying file is protected using HDCP DRM
} FileSourceDrmType;

/*!
*@brief Enumeration identifying FileSource media major types.
*/
typedef enum
{
  FILE_SOURCE_MJ_TYPE_UNKNOWN,
  FILE_SOURCE_MJ_TYPE_AUDIO,
  FILE_SOURCE_MJ_TYPE_VIDEO,
  FILE_SOURCE_MJ_TYPE_TEXT
} FileSourceMjMediaType;

/*!
*@brief Enumeration specifying FileSource media minor types.
  These are also called as codec types.
*/
typedef enum
{
  FILE_SOURCE_MN_TYPE_UNKNOWN,
  FILE_SOURCE_MN_TYPE_EVRC,
  FILE_SOURCE_MN_TYPE_QCELP,
  FILE_SOURCE_MN_TYPE_AAC, //General minor type when it is not known whether aac has adts/adif/loas
  FILE_SOURCE_MN_TYPE_AAC_ADTS,
  FILE_SOURCE_MN_TYPE_AAC_ADIF,
  FILE_SOURCE_MN_TYPE_AAC_LOAS,
  FILE_SOURCE_MN_TYPE_GSM_AMR,
  FILE_SOURCE_MN_TYPE_MPEG4,
  FILE_SOURCE_MN_TYPE_NONSTD_MPEG4,
  FILE_SOURCE_MN_TYPE_H263,
  FILE_SOURCE_MN_TYPE_H264,
  FILE_SOURCE_MN_TYPE_HEVC,
  FILE_SOURCE_MN_TYPE_VP6F,
  FILE_SOURCE_MN_TYPE_STILL_IMAGE,
  FILE_SOURCE_MN_TYPE_TIMED_TEXT,
  FILE_SOURCE_MN_TYPE_GENERIC_TEXT,
  FILE_SOURCE_MN_TYPE_XSUB_BITMAP_TEXT,
  FILE_SOURCE_MN_TYPE_SMPTE_TIMED_TEXT, //!Reference: SMPTE ST 2052-1:2010
  FILE_SOURCE_MN_TYPE_UTF8_TEXT,
  FILE_SOURCE_MN_TYPE_USF_TEXT,
  FILE_SOURCE_MN_TYPE_SSA_TEXT,
  FILE_SOURCE_MN_TYPE_ASS_TEXT,
  FILE_SOURCE_MN_TYPE_BITMAP_TEXT,
  FILE_SOURCE_MN_TYPE_VOBSUB_TEXT,
  FILE_SOURCE_MN_TYPE_KARAOKE_TEXT,
  FILE_SOURCE_MN_TYPE_JPEG,
  FILE_SOURCE_MN_TYPE_PNG,
  FILE_SOURCE_MN_TYPE_MP3,
  FILE_SOURCE_MN_TYPE_MP2,
  FILE_SOURCE_MN_TYPE_MP1,
  FILE_SOURCE_MN_TYPE_WMA,
  FILE_SOURCE_MN_TYPE_WM_SPEECH,
  FILE_SOURCE_MN_TYPE_WMA_PRO,
  FILE_SOURCE_MN_TYPE_WMA_LOSSLESS,
  FILE_SOURCE_MN_TYPE_WMV1,
  FILE_SOURCE_MN_TYPE_WMV2,
  FILE_SOURCE_MN_TYPE_VC1,
  FILE_SOURCE_MN_TYPE_WMV3,
  FILE_SOURCE_MN_TYPE_OSCAR,
  FILE_SOURCE_MN_TYPE_CONC,
  FILE_SOURCE_MN_TYPE_COOK,
  FILE_SOURCE_MN_TYPE_SIPR,
  FILE_SOURCE_MN_TYPE_RV30,
  FILE_SOURCE_MN_TYPE_RV40,
  FILE_SOURCE_MN_TYPE_AMR_WB,
  FILE_SOURCE_MN_TYPE_AMR_WB_PLUS,
  FILE_SOURCE_MN_TYPE_EVRC_WB,
  FILE_SOURCE_MN_TYPE_EVRC_B,
  FILE_SOURCE_MN_TYPE_NONMP4_MP3,
  FILE_SOURCE_MN_TYPE_QCP,
  FILE_SOURCE_MN_TYPE_MIDI,
  FILE_SOURCE_MN_TYPE_NONMP4_AAC,
  FILE_SOURCE_MN_TYPE_NONMP4_AMR,
  FILE_SOURCE_MN_TYPE_CONC_AAC,
  FILE_SOURCE_MN_TYPE_CONC_AMR,
  FILE_SOURCE_MN_TYPE_CONC_EVRC,
  FILE_SOURCE_MN_TYPE_CONC_QCELP,
  FILE_SOURCE_MN_TYPE_BSAC,
  FILE_SOURCE_MN_TYPE_DIVX311,
  FILE_SOURCE_MN_TYPE_DIVX40,
  FILE_SOURCE_MN_TYPE_DIVX50_60,
  FILE_SOURCE_MN_TYPE_DTS,
  FILE_SOURCE_MN_TYPE_AC3,
  FILE_SOURCE_MN_TYPE_EAC3,
  FILE_SOURCE_MN_TYPE_EAC3_JOC,
  FILE_SOURCE_MN_TYPE_PCM,
  FILE_SOURCE_MN_TYPE_G711_ALAW,
  FILE_SOURCE_MN_TYPE_G711_MULAW,
  FILE_SOURCE_MN_TYPE_GSM_FR,
  FILE_SOURCE_MN_TYPE_G721,
  FILE_SOURCE_MN_TYPE_G723,
  FILE_SOURCE_MN_TYPE_G729,
  FILE_SOURCE_MN_TYPE_MPEG2,
  FILE_SOURCE_MN_TYPE_VORBIS,
  FILE_SOURCE_MN_TYPE_FLAC,
  FILE_SOURCE_MN_TYPE_THEORA,
  FILE_SOURCE_MN_TYPE_SORENSON_SPARK,
  FILE_SOURCE_MN_TYPE_VP8F,
  FILE_SOURCE_MN_TYPE_VP9,
  FILE_SOURCE_MN_TYPE_REAL,
  FILE_SOURCE_MN_TYPE_MJPEG,
  FILE_SOURCE_MN_TYPE_MPEG1,
  FILE_SOURCE_MN_TYPE_OPUS,
  FILE_SOURCE_MN_TYPE_MAX,
} FileSourceMnMediaType;

/*!
*@brief Enumeration to differentiate among metadata such as Author, Title etc.
*This enum can be used in GetClipMetaData API to retrieve meta-data such as
*author/title etc.
*/
typedef enum
{
  FILE_SOURCE_MD_UNKNOWN,         //!Unknown, used for initialization
  FILE_SOURCE_MD_TITLE,           //! Title string
  FILE_SOURCE_MD_AUTHOR,          //! Author string
  FILE_SOURCE_MD_DESCRIPTION,     //! Description string
  FILE_SOURCE_MD_RATING,          //! Rating string
  FILE_SOURCE_MD_COPYRIGHT,       //! Copyright string
  FILE_SOURCE_MD_VERSION,         //! Version string
  FILE_SOURCE_MD_CREATION_DATE,   //! Creation Date string
  FILE_SOURCE_MD_PERFORMANCE,     //! Performance string
  FILE_SOURCE_MD_GENRE,           //! Genere string
  FILE_SOURCE_MD_CLASSIFICATION,  //! Classifications tring
  FILE_SOURCE_MD_KEYWORD,         //! keyword string
  FILE_SOURCE_MD_LOCATION,        //! Location string
  FILE_SOURCE_MD_GEOTAG,          //! Longitude and Latitude string
  FILE_SOURCE_MD_ARTIST,          //! Artist string
  FILE_SOURCE_MD_ALBUM,           //! Album string
  FILE_SOURCE_MD_INFO,            //! Information string
  FILE_SOURCE_MD_REC_YEAR,        //! Recording Year string
  FILE_SOURCE_MD_COMPOSER,        //! Composer string
  FILE_SOURCE_MD_TRACK_NUM,       //! Track number string
  FILE_SOURCE_MD_ALBUM_ART,       //! Album Art info (Picture) string
  FILE_SOURCE_MD_SOFTWARE,        //! Software
  FILE_SOURCE_MD_ALBUM_ARTIST,    //! Album Artist info string
  FILE_SOURCE_MD_DISK_NUM,        //! Disk Number info string
  FILE_SOURCE_MD_COMPILATION,     //! Info about compilation
  FILE_SOURCE_MD_WRITER,          //! Info about writer
  FILE_SOURCE_MD_OWNER,           //! owner string
  FILE_SOURCE_MD_ENC_DELAY,       //! Encoder Delay string
  FILE_SOURCE_MD_PADDING_DELAY,   //! Padding Delay string
  FILE_SOURCE_MD_SEEK_PREROLL_DELAY, //! Seek Preroll Delay string
  FILE_SOURCE_MD_MUXING_APP,      //! Muxing APP name (Available in MKV)
  FILE_SOURCE_MD_WRITING_APP,     //! Writing APP name (Available in MKV)
  FILE_SOURCE_MD_CODEC_NAME,      //! Codec name (Available in MKV)
  FILE_SOURCE_MD_ANDROID_LOOP,    //! Flag to indicate loop playback (in OGG)
} FileSourceMetaDataType;

/*!
*@brief Enumeration to identify metadata encoding method
*/
typedef enum
{
  FS_ENCODING_TYPE_UNKNOWN,     //! Unknown meta-data type
  FS_TEXT_ENC_ISO8859_1,        //! ISO-8859-1 metadata
  FS_TEXT_ENC_UTF8,             //! UTF-8 metadata
  FS_TEXT_ENC_UTF16,            //! UTF-16 metadata
  FS_TEXT_ENC_UTF16_BE,         //! UTF-16 metadata without BOM
  FS_TEXT_ENC_UTF32,            //! UTF-32 metadata
} FS_TEXT_ENCODING_TYPE;

/*!
*@brief Enumeration to identify configuration item
*/
typedef enum
{
  //!Get/Set basetime.
  //!FileSourceConfigItem.nresult should be used to set/get basetime on given track.
  //!For this enum, FileSourceConfigItem.nresult should be treated as unsinged long long
  //!to represent 64 bit number for base time.
  FILE_SOURCE_MEDIA_BASETIME,

  //!Parser to output one audio frame/buffer
  //!FileSourceConfigItem.nresult should be used to set/get frame output mode.
  //!For this enum, FileSourceConfigItem.nresult should be treated as boolean.
  //!FileSourceConfigItem.nresult will be set to true/false if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME,

  //!Parser to output stream of bytes.
  //!FileSourceConfigItem.nresult should be used to set/get frame output mode.
  //!For this enum, FileSourceConfigItem.nresult should be treated as boolean.
  //!FileSourceConfigItem.nresult will be set to true/false if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM,

  //!Parser strips codec specific header,if any before outputting.
  //!FileSourceConfigItem.nresult should be used to set/get header stripping mode.
  //!For this enum, FileSourceConfigItem.nresult should be treated as boolean.
  //!FileSourceConfigItem.nresult will be set to true/false if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER,

  //!Parser does not strip codec specific header,if any before outputting.
  //!FileSourceConfigItem.nresult should be used to set/get header stripping mode.
  //!For this enum, FileSourceConfigItem.nresult should be treated as boolean.
  //!FileSourceConfigItem.nresult will be set to true/false if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER,

  //!Get Audio & Video profile information.
  //!FileSourceConfigItem.nresult should be used to set/get profile information.
  //!For this enum, FileSourceConfigItem.nresult should be treated as uint8.
  //!FileSourceConfigItem.nresult will be set to profile value if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_PROFILE_INFORMATION ,

  //!Get Audio & Video profile information.
  //!FileSourceConfigItem.nresult should be used to set/get profile information.
  //!For this enum, FileSourceConfigItem.nresult should be treated as uint8.
  //!FileSourceConfigItem.nresult will be set to bsi if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_BIT_STREAM_INFORMATION,

  //!Get number of DRM system supported by media.
  //!FileSourceConfigItem.nresult should be used to set/get profile information.
  //!For this enum, FileSourceConfigItem.nresult should be treated as uint8.
  //!FileSourceConfigItem.nresult will be get to number of DRM system supported,
  //!if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_NUM_DRM_SYSTEM_SUPPORTED,

  //!Disable TimeStamp Discontinuity Correction in TS parser.
  //!FileSourceConfigItem.nresult should be used to set discontinuity correction mode.
  //!For this enum, FileSourceConfigItem.nresult should be treated as boolean.
  //!FileSourceConfigItem.nresult will be set to true/false if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_DISABLE_TS_DISCONTINUITY_CORRECTION,

  //!Enable TimeStamp Discontinuity Correction in TS parser.
  //!FileSourceConfigItem.nresult should be used to set discontinuity correction mode.
  //!For this enum, FileSourceConfigItem.nresult should be treated as boolean.
  //!FileSourceConfigItem.nresult will be set to true/false if set/get is successful for given configuration item.
  FILE_SOURCE_MEDIA_ENABLE_TS_DISCONTINUITY_CORRECTION,

  //!Disable PCM Sample upgrade in WAV Parser
  //!FileSourceConfigItem.nresult should be used to set sample upgrade mode.
  //!For this enum, FileSourceConfigItem.nresult should be treated as boolean.
  //!FileSourceConfigItem.nresult will be set to true/false if set/get is
  //!successful for given configuration item.
  FILE_SOURCE_MEDIA_DISABLE_PCM_SAMPLE_UPGRADE,

  //!Enable PCM Sample upgrade in WAV Parser
  //!FileSourceConfigItem.nresult should be used to set sample upgrade mode.
  //!For this enum, FileSourceConfigItem.nresult should be treated as boolean.
  //!FileSourceConfigItem.nresult will be set to true/false if set/get is
  //!successful for given configuration item.
  FILE_SOURCE_MEDIA_ENABLE_PCM_SAMPLE_UPGRADE,

} FileSourceConfigItemEnum;

/*!
*@brief Union to be used to set/get various configuration items on parser.
*nresult is to be interpreted in the context of configuration item.
*Please refer to FileSourceConfigItemEnum for more information.
*/
typedef union
{
  //!To be interpreted in the context of FileSourceConfigItemEnum.
  unsigned long long nresult;
} FileSourceConfigItem;

/*!
*@brief Enumeration to specify AAC stream format.
*/
typedef enum
{
   FILE_SOURCE_AAC_FORMAT_UNKNWON,   //! UNKNWON
   FILE_SOURCE_AAC_FORMAT_ADTS,      //! ADTS
   FILE_SOURCE_AAC_FORMAT_ADIF,      //! ADIF
   FILE_SOURCE_AAC_FORMAT_RAW,       //! RAW
   FILE_SOURCE_AAC_FORMAT_LOAS       //! LOAS
} AACStreamFormat;

/*!
*@brief Enumeration to track the status of the callback
        for FileSource asynchronous APIs.
*/
typedef enum SourceCallbackStatus
{
  FILE_SOURCE_OPEN_COMPLETE,
  FILE_SOURCE_OPEN_FAIL,
  FILE_SOURCE_OPEN_DATA_UNDERRUN,
  FILE_SOURCE_SEEK_COMPLETE,
  FILE_SOURCE_SEEK_UNDERRUN,
  FILE_SOURCE_SEEK_FAIL,
  FILE_SOURCE_ERROR_ABORT,
  FILE_SOURCE_SEEK_UNDERRUN_IN_FRAGMENT
} FileSourceCallBackStatus;

/*!
*@brief Enumeration to list types of seek supported by FileSource.
*/
typedef enum SeekType
{
  FILE_SOURCE_SEEK_UKNOWN,
  FILE_SOURCE_SEEK_ABSOLUTE,
  FILE_SOURCE_SEEK_RELATIVE
} FileSourceSeekType;

typedef enum FS_MEDIA_INDEXTYPE
{
  /* Audio parameters & configurations */
  FS_IndexParamAudioStartUnused = 0x01000000,
  FS_IndexParamAudioAac,        /**< reference: AacCodecData   */
  FS_IndexParamAudioFlac,       /**< reference: WavFormatData  */
  FS_IndexParamAudioWav,        /**< reference: FlacFormatData */
  FS_IndexParamAudioWma,        /**< reference: WmaCodecData   */
  FS_IndexParamAudioAC3,        /*!< reference: FS_AUDIO_PARAM_AC3_SYNCINFOTYPE */
  FS_IndexParamAudioDTS,        /*!< reference: FS_AUDIO_PARAM_DTS_SYNCINFOTYPE */
  FS_IndexParamAudioMP3,        /*!< reference: FS_AUDIO_PARAM_MPGTYPE */

  /* Video parameters & configurations */
  FS_IndexParamVideoStartUnused = 0x02000000,
  FS_IndexParamVideoH264,        /**< reference: FS_VIDEO_PARAM_H264TYPE */

  /* Other parameters & configurations */
  FS_IndexParamOtherStartUnused = 0x03000000,
  FS_IndexParamOtherPSSHInfo,   /**< reference: FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE */
  FS_IndexParamOtherPSSHData,   /**< reference: FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE */
  FS_IndexParamOtherMediaTrackInfo, /**< reference: MediaTrackInfo */
  FS_IndexParamLastPTSValue,    /**< ref: FS_LAST_PTS_VALUE */
}FS_MEDIA_INDEXTYPE;

/*!
 *@brief Enumeration to list type of character code supported in subtitle.
 */
typedef enum FS_SUBTITLE_CHAR_CODETYPE{
  FS_SUBTITLE_CHAR_CODE_UNKNOWN,
  FS_SUBTITLE_CHAR_CODE_UTF8,
  FS_SUBTITLE_CHAR_CODE_UTF16,
  FS_SUBTITLE_CHAR_CODE_UNICODE
}FS_SUBTITLE_CHAR_CODETYPE;

/*!
 *@brief Structure to store subtitle track sub sample information.
 */
typedef struct FS_SUBTITLE_SUB_INFOTYPE{
  //! Number of subsample count present in current sample
  uint16 ulSubSampleCount;
  //! Sub sample minor type. In case of smpte-tt a subsample
  //! can have IMAGE or FONT as subsample
  FileSourceMnMediaType eSubMnType;
  //! Subtitle character code type
  FS_SUBTITLE_CHAR_CODETYPE eCharCode;
  //! Subtitle sub information size
  uint16 usSubtitleSubInfoSize;
  //! Subtitle sub info data, which carry explicit control information
  uint8 ucSubtitleSubInfo[MAX_SUBTITLE_SUB_INFO_SIZE];
}FS_SUBTITLE_SUB_INFOTYPE;

/*!
 *@brief Structure to store subsample encryption information in a sample
 */
typedef struct FS_ENCRYPTED_SUBSAMPLE_INFOTYPE
{
  //! Number of bytes of clear data in subsample
  uint16 usSizeOfClearData;
  // Offset of clear data in subsample from start
  uint32 ulOffsetClearData;
  //! Number of bytes of encrypted data in subsample
  uint32 ulSizeOfEncryptedData;
  //! Offset of encrypted data in subsample from start
  uint32 ulOffsetEncryptedData;
}FS_ENCRYPTED_SUBSAMPLE_INFOTYPE;

/*!
 *@brief Structure to store content protection information
 */
typedef struct FS_CONTENT_PROTECTION_INFOTYPE{
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
  FS_ENCRYPTED_SUBSAMPLE_INFOTYPE sEncSubsampleInfo[MAX_ENCRYPTED_ENTRY];
}FS_CONTENT_PROTECTION_INFOTYPE;

/*!
 *@brief Structure to store extra sample information.
 */
typedef struct FS_EXTRA_SAMPLE_INFOTYPE{
  //! Index of sample entry to which Subtitle sub information applies.
  //! Index ranges from 1 to number of entry in sample description box
  uint16 usSampleDescriptionIndex;
  //! Subsample information for subtitle track
  FS_SUBTITLE_SUB_INFOTYPE sSubTitle;
  //! Content protection information applicable to current sample
  FS_CONTENT_PROTECTION_INFOTYPE sCP;
}FS_EXTRA_SAMPLE_INFOTYPE;

/*!
 *@brief Structure to store protection system specific information
 *       applicable to one particular DRM system.
 */
typedef struct FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE{
  //! Protection scheme type of content
  FileSourceDrmType ePSDRMType;
  //! Index associated with protection scheme
  uint32 ulDRMIndex;
  //! Protection scheme SystemID(UUID)
  uint8 ucSystemID[MAX_SYSTEMID_SIZE];
  //! Number of KID Mapping entry
  uint32 ulKIDCount;
  //! Size of KID mapping entry
  uint32 ulKIDDataSize;
  //! Size of protection system specific data
  uint32 ulPSSHDataSize;
}FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE;

/*!
 *@brief Structure to store protection system specific data
 *       applicable to one particular DRM system.
 */
typedef struct FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE{
  //! Protection scheme data queried
  //FileSourceDrmType ePSDRMType;
  uint32 ulDRMIndex;
  //! Default Key Identifier for to decrypt the associated samples
  uint8 ucDefaultKeyID[MAX_KID_SIZE];
  //! KID mapping data buffer size
  uint32 ulKIDDataBufSize;
  //! KID mapping data buffer. Client has to allocate
  // buffer size of ulKIDDataSize
  uint8* pucKIDDataBuf;
  //! PSSH data buffer size
  uint32 ulPSSHDataBufSize;
  //! PSSH data buffer. Client has to allocate of buffer
  //  size of ulPSSHDataSize
  uint8* pucPSSHDataBuf;
}FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE;

/*!
*@brief Structure to store media sample information.
*/
typedef struct
{
  //! composition time of sample, in the media timescale units
  uint64 startTime;

  //! timestamp of the next sample in media timescale units
  uint64 endTime;

  //! difference between composition time of this sample and
  //! the next sample, in the media timescale
  uint64 delta;

  //! Indication if sample is a random access point(non-zero) or not (zero)
  uint32 sync;

  //! True indicates if startTime is valid
  bool   bStartTsValid;

  //! Indication if any bytes lost in the sample(approx)
  uint32 nBytesLost;

  //! Number of valid samples in page(defined in OGG Parser)
  uint64 nGranule;

  //! PCR (only for MP2TS Parser)
  uint64 nPCRValue;

  //! Structure that holds extra information associated to current sample
  FS_EXTRA_SAMPLE_INFOTYPE sSubInfo;
}FileSourceSampleInfo;

//! structure with information about the trackId, informs the user if the track is selected or not.
typedef struct
{
  //! Track Id
  uint32 id;
  //! Indicates whether the track is selected for playback
  bool   selected;
  //!Stores the major type for the given track-id
  FileSourceMjMediaType majorType;
}FileSourceTrackIdInfoType;

/*!
*@brief Structure to store audio track information.
*/
typedef struct
{
  //! Track Id
  uint32    id;
  //! Audio codec ( eg: AAC, AMR etc)
  FileSourceMnMediaType audioCodec;
  //! number of audio channels
  uint32      numChannels;
  //! time scale of the track
  uint32      timeScale;
  //! total duration fo the track in milliseconds
  uint64      duration;
  //! bit rate
  uint32      bitRate;

  //! min bit rate
  uint32      minBitRate;

  //!max bit rate
  uint32      maxBitRate;

  //! sampling rate
  uint32      samplingRate;
  //!number of valid bits per sample
  uint32      nBitsPerSample;
  //!Block align, applicable for PCM codec especially
  uint32      nBlockAlign;
  //!Encoder delay
  uint32      nEncoderDelay;
  //!Padding delay
  uint32      nPaddingDelay;
  //! Seek Preroll delay (applicable for OPUS audio codec)
  uint64      nSeekPrerollDelay;
  //! Number of independent streams
  uint8       ucNoOfIndSubStrm;
}FileSourceAudioInfo;

/*!
*@brief Structure to store video track information.
*/
typedef struct
{
  //! Track Id
  uint32    id;
  //! Video codec ( eg: H264, H263, MPEG4 etc)
  FileSourceMnMediaType videoCodec;
  //! subdivision of level
  uint32     layer;
  //! time scale of the track
  uint32     timeScale;
  //! total duration of the track in milliseconds
  uint64     duration;
  //! bit rate
  uint32     bitRate;
  //! frames per second
  float      frameRate;
  //! frame width in pixels
  uint32     frameWidth;
  //! frame height in pixels
  uint32     frameHeight;
  //! rotation degree in degrees
  uint32     ulRotationDegrees;
}FileSourceVideoInfo;

/*!
*@brief Structure to store text track information.
*/
typedef struct
{
  //! Track Id
  uint32    id;
  //! Text codec ( eg: timed, generic etc)
  FileSourceMnMediaType textCodec;
  //! subdivision of level
  uint32      layer;
  //! time scale of the track
  uint32      timeScale;
  //! total duration for the track in milliseconds
  uint64      duration;
  //! frame width in pixels
  uint32      frameWidth;
  //! frame height in pixels
  uint32      frameHeight;
}FileSourceTextInfo;

/*!
*@brief Structure to store Last PTS information.
*/
typedef struct last_pts_info
{
  FileSourceMjMediaType eMajorType; //! Track Type
  uint64                ullLastPTS; //! Last PTS value
}FS_LAST_PTS_VALUE;

/*!
*@brief Defines WMA format block needed to configure WMA decoder
*/
typedef struct wma_codec_data
{
  //! WMA advance encode option
  uint16        nAdvEncodeOpt;
  //! WMA advance encode option2
  uint32        nAdvEncodeOpt2;
  //! WMA Channel mask
  uint32        nChannelMask;
  //! Bits per audio sample
  unsigned long nBitsPerSample;
  //! WMA encode option
  unsigned long nEncodeOpt;
  //! ASF virtual packet size
  unsigned long nVirtualPktSize;
  //! WMA format tag
  unsigned long nFormatTag;
  //! ASF packet size
  unsigned long nASFPacketSize;
  //! Block align for ASF packet
  uint32        nBlockAlign;
}WmaCodecData;

/*!
*@brief Defines AAC format block needed to configure AAC decoder.
*/
typedef struct aac_codec_data
{
   //! Identifies AAC profile type
   uint8             ucAACProfile;
   //! Identifies AAC Stream format
   AACStreamFormat   eAACStreamFormat;
}AacCodecData;

/*!
*@brief Defines WAV format block needed to configure WAV decoder.
*/
typedef struct wav_format_data
{
  uint8 format;            ///< format
  uint8 iadpcm_format;    ///< format
  uint16 channels;        ///< number of channels
  uint16 frame_size;      ///< frame size
  uint16 bits_per_sample; ///< bits per sample
  uint16 block_align;     ///< Block align
  uint16 samples_per_block;  ///< samples per block
  uint32 type;            ///< first member of the union defines its type
  uint32 sample_rate;     ///< sample rate
  uint32 bytes_rate;      ///< bit rate
  uint32 channel_mask;   /// channel mask
}WavFormatData;

/*!
*@brief Defines MPG format metadata needed to configure MPG decoder.
*/
typedef struct FS_AUDIO_PARAM_MPGTYPE
{
  uint8 ucVersion;        ///< Version
  uint8 ucLayer;          ///< Layer
  uint16 usChannels;      ///< Number of channels
  uint32 ulSamplingFreq;  ///< Sample rate
  uint32 ulBitRate;       ///< Bit rate
}FS_AUDIO_PARAM_MPGTYPE;

typedef struct flac_format_data
{
  uint16              nMinBlockSize;
  uint16              nMaxBlockSize;
  uint32              nMinFrameSize;
  uint32              nMaxFrameSize;
  uint32              nSamplingRate;
  uint32              nFixedBlockSize;
  uint8               nChannels;
  uint8               nBitsPerSample;
  uint64              nTotalSamplesInStream;
}FlacFormatData;

typedef union
{
  FileSourceAudioInfo audioTrackInfo;
  FileSourceVideoInfo videoTrackInfo;
  FileSourceTextInfo  textTrackInfo;
} MediaTrackInfo;

/*!
*@brief Defines H264(AVC/MVC/SVC/HVC) video specific parameter.
*/
typedef struct FS_VIDEO_PARAM_H264TYPE
{
  //! AVC/HVC/MVC/SVC profile information
  uint8   ucH264ProfileInfo;
  //! AVC/HVC/MVC/SVC level information
  uint8   ucH264LevelInfo;
  //! MVC flag to represent complete representation
  uint8   ucCompleteRepresentation;
  //! MVC flag to represent explicate AU track
  uint8   ucExplicitAuTrack;
  //! MVC/SVC minimum temporal id
  uint8  ucMinTemporalId;
  //! MVC/SVC maximum temporal id
  uint8  ucMaxTemporalId;
  //! Number of views available in track
  uint16  ulNumberOfViews;
}FS_VIDEO_PARAM_H264TYPE;

/*!
*@brief Defines AC3/EAC3 stream specific parameter.
*/
typedef struct FS_AUDIO_PARAM_AC3TYPE
{
  //! Sampling Rate in kHz
  uint32  ulSamplingRate;
  //! Bit rate in kBits
  uint32  ulBitRate;
  //! Frame size
  uint32  usFrameSize;
  //! Number of channels
  uint16  usNumChannels;
  //! Channel location
  uint16 usChannelLocation;
  //! Program ID to play, ec3 support 8 program.
  uint8 ucProgramID;
  //! Type of service bit-stream convey
  uint8 ucBitStreamMode;
  //! Main service channels are in use
  uint8 ucAudioCodingMode;
  //! Number of independent sub-streams present
  uint8 ucNumOfIndSubs;
  //! EC3 extension type A present
  uint8 ucEc3ExtTypeA;
}FS_AUDIO_PARAM_AC3TYPE;

/*!
*@brief Defines SubStream types of DTS standard.
*/
typedef enum FS_AUDIO_DTS_SUBTYPE
{
  FILE_SOURCE_SUB_MN_TYPE_DTS_LEGACY,
  FILE_SOURCE_SUB_MN_TYPE_DTS_HD,
  FILE_SOURCE_SUB_MN_TYPE_DTS_XCH,
  FILE_SOURCE_SUB_MN_TYPE_DTS_XXCH,
  FILE_SOURCE_SUB_MN_TYPE_DTS_X96K,
  FILE_SOURCE_SUB_MN_TYPE_DTS_XBR,
  FILE_SOURCE_SUB_MN_TYPE_DTS_LBR,
  FILE_SOURCE_SUB_MN_TYPE_DTS_XLL,
  FILE_SOURCE_SUB_MN_TYPE_DTS_PCM,
  FILE_SOURCE_SUB_MN_TYPE_DTS_SUBSTREAM_CORE2,
}FS_AUDIO_DTS_SUBTYPE;
/*!
*@brief Defines DTS/HD stream specific parameter.
*/
typedef struct FS_AUDIO_PARAM_DTSTYPE
{
  //! Type of service bit-stream convey
  uint8  ucBitStreamMode;
  //! Main service channels are in use
  uint8  ucAudioCodingMode;
  //! Number of channels
  uint16 usNumChannels;
  //! Sampling Rate in kHz
  uint32 ulSamplingRate;
  //! Bit rate in kBits
  uint32 ulBitRate;
  //! Frame size
  uint16 usFrameSize;
  //! Number of Samples per Frame
  uint16 usSamplesperFrame;
  //! Dynamic Range Flag
  bool bDynamicRangeFlag;

  /* Rev2Aux data fields */
  //! Size of the Rev2Aux data chunk
  uint8  nRev2AUXDataByteSize;
  //! Indicates if metadata is present or not
  bool   bESMetaDataFlag;
  //! Indicates if Broadcast flag is set or not
  bool   bBroadcastMetadataPresent;
  //! Indicates if DRC Metadata is present or not
  bool   bDRCMetadataPresent;
  //! Indicates if DialNorm data is present or not
  bool   bDialnormMetadata;

  //! SubStream Type
  FS_AUDIO_DTS_SUBTYPE eSubType;
}FS_AUDIO_PARAM_DTSTYPE;

/*!
*@brief Defines ENUM values for Image Format types
*/
typedef enum
{
  IMG_PNG,
  IMG_JPG,
  IMG_BMP,
  IMG_GIF,
  IMG_UNK
}FS_IMAGE_FORMAT;

/*!
*@brief Defines ENUM values for Picture types
*/
typedef enum
{
  PIC_TYPE_OTHER,
  PIC_TYPE_32_32_PNG,
  PIC_TYPE_OTHER_FILE_ICON,
  PIC_TYPE_COVER_FRONT,
  PIC_TYPE_COVER_BACK,
  PIC_TYPE_LEAFLET_PAGE,
  PIC_TYPE_LABEL,
  PIC_TYPE_LEAD_ARTIST_PERFORM_SOLOIST,
  PIC_TYPE_ARTIST_PERFORMER,
  PIC_TYPE_CONDUCTOR,
  PIC_TYPE_BAND_ORCHESTRA,
  PIC_TYPE_COMPOSER,
  PIC_TYPE_LYRICIST_TEXT_WRTER,
  PIC_TYPE_REC_LOCN,
  PIC_TYPE_DURING_RECORDING,
  PIC_TYPE_DURING_PERFORMANCE,
  PIC_TYPE_MOVIE_VIDEO_SCREEN_CAPTURE,
  PIC_TYPE_BRIGHT_COLOURED_FISH,
  PIC_TYPE_ILLUSTRATION,
  PIC_TYPE_BAND_ARTIST_LOGOTYPE,
  PIC_TYPE_PUBLISHER_STUDIO_LOGOTYPE,
  PIC_TYPE_UNKNOWN
}FS_PICTURE_TYPE;

/*!
*@brief Defines structure to be used to populate Album Art info available in ID3 tags.
*/
typedef struct
{
  uint8           ucTextEncodeType;
  FS_IMAGE_FORMAT imgFormat;
  FS_PICTURE_TYPE picType;
  uint8           ucImgFormatStr[MAX_IMG_FORMAT_LEN];
  uint8           ucDesc[MAX_DESC_LEN];
  uint32          ulPicDataLen;
  uint8           pucPicData[1];
}FS_ALBUM_ART_METADATA;

/*-------------------------------------------------------------------------
  DRM decryption function type (Implemented by OEM).
-------------------------------------------------------------------------*/
typedef boolean (*DRMDecryptMethodT)
(
  uint32     TrackId,
  void      *pEncryptedDataBuf,   /* pointer to encrypted data buffer, which has to be decrypted */
  void      *pDecryptedDataBuf,   /* pointer to destination buffer to copy decrypted data,
                                     OEM is resposible for copying the decrypted data  */
  uint32    wEncryptedDataSize,   /* encrypted data buffer size */
  uint32    *pDecryptedDataSize,  /* pointer to decrypted data buffer size,
                                     OEM is resposible for copying the decrypted data size  */
  void      *pClientData          /* client data provided by OEM when registering callback */
);

/**
Description:
Following structure defines DivX Clip specific DRM information.

Members:

nUseLimit                - The counter to specify maximum number of times this clip can be played.
nUseCounter              - Current playback count for the given clip.
                           Clip can be played only 'nUseLimit' times. User has to renew the count once it expires.
nCgmsaSignal             - External signal protection. Copy protection mechanism for analog television signals.

nAcptbSignal             - External signal protection. If set and if device does not have MacroVision,
                             TvOut should be disabled.
nDigitalProtectionSignal - External signal protection for digital output. When set,
                             content can only be output over secure digital outputs approved by DivX or
                             analog video outputs that are constrained to maximum 480p resolution.

Comments:
Clip specific information,will be available once parsing is done successfully!!
*/
extern "C"
{
  typedef struct DrmInfoT
  {
    bool     isRental;
    uint16   useLimit;
    uint16   useCounter;
    uint8    cgmsaSignal;
    uint8    acptbSignal;
    uint8    digitalProtectionSignal;
    uint8    ictSignal;
  }ClipDrmInfoT;
}

/*!
*@brief Defines audio Buffer structure used for finding audio at frame boundary.
*/

typedef struct _audio_data_buff_t
{
  uint8*  pDataBuff;
  uint32  nDataBufSize;
  uint32  nDataSize;
  uint32  nReadIndex;
  double  nBaseTime;
  double  nFrameTime;
  float   nDelta;
  uint64  nSamplesConsumed;
}audio_data_buffer;
#endif

