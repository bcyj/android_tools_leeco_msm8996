#ifndef QOMX_STREAMINGEXTENSIONS_PRIVATE_H_
#define QOMX_STREAMINGEXTENSIONS_PRIVATE_H_


/************************************************************************* */
/**
 * @brief Header file for QOMX_StreamingExtensionsPrivate.
 *         This file contains the description of the Qualcomm OpenMax IL
 *         streaming extention interface, through which the IL client and
 *         OpenMax components can access additional streaming capabilities.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/inc/QOMX_StreamingExtensionsPrivate.h#8 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include <OMX_Types.h>
#include <OMX_Component.h>

#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/**
 * Qualcomm vendor streaming extension strings.
 */

#define OMX_QUALCOMM_INDEX_PARAM_COMPLETE_DASH_ADAPTATION_PROPERTIES      "OMX.Qualcomm.index.param.streaming.CompleteDashAdaptationProperties"
#define OMX_QUALCOMM_INDEX_PARAM_DRM_INFO                                 "OMX.Qualcomm.index.param.streaming.DRMInfo"
#define OMX_QUALCOMM_INDEX_PARAM_CONTENTPROTECTION_INFO                   "OMX.Qualcomm.index.param.streaming.ContentProtectionInfo"
#define OMX_QUALCOMM_INDEX_PARAM_PSSH_INFO                                "OMX.Qualcomm.index.param.streaming.PsshInfo"
#define OMX_QUALCOMM_INDEX_PARAM_EXTRASAMPLE_INFO                         "OMX.Qualcomm.index.param.streaming.ExtraSampleInfo"
#define OMX_QUALCOMM_INDEX_PARAM_SELECTED_DASH_ADAPTATION_PROPERTIES      "OMX.Qualcomm.index.param.streaming.SelectedDashAdaptationProperties"
#define OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_DIMENSIONS               "OMX.Qualcomm.index.param.streaming.SMPTETimeTextDimensions"
#define OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_SUBINFO                  "OMX.Qualcomm.index.param.streaming.SMPTETimeTextSubInfo"
#define OMX_QUALCOMM_INDEX_PARAM_DASH_MPD                                 "OMX.Qualcomm.index.param.streaming.DashMPD"
#define OMX_QUALCOMM_INDEX_PARAM_QOE                                      "OMX.Qualcomm.index.param.streaming.QOE"
#define OMX_QUALCOMM_INDEX_PARAM_QOE_PLAY                                 "OMX.Qualcomm.index.param.streaming.QOE.Play"
#define OMX_QUALCOMM_INDEX_PARAM_QOE_STOP                                 "OMX.Qualcomm.index.param.streaming.QOE.Stop"
#define OMX_QUALCOMM_INDEX_PARAM_QOE_SWITCH                               "OMX.Qualcomm.index.param.streaming.QOE.Switch"
#define OMX_QUALCOMM_INDEX_PARAM_QOE_PERIODIC                             "OMX.Qualcomm.index.param.streaming.QOE.Periodic"
#define OMX_QUALCOMM_INDEX_PARAM_CURRENT_REPRESENTATION                   "OMX.Qualcomm.index.param.streaming.CurrentRepresentation"
#define OMX_QUALCOMM_INDEX_PARAM_COOKIES                                  "OMX.Qualcomm.index.param.streaming.Cookies"
#define OMX_QUALCOMM_INDEX_PARAM_AC3                                      "OMX.Qualcomm.index.param.streaming.ac3"
#define OMX_QUALCOMM_INDEX_PARAM_MP2                                      "OMX.Qualcomm.index.param.streaming.mp2"
#define OMX_QUALCOMM_INDEX_PARAM_BUFFERED_DURATION                        "OMX.Qualcomm.index.param.streaming.BufferedDuration"
#define OMX_QUALCOMM_INDEX_PARAM_MEDIA_TRACK_ENCODING                     "OMX.Qualcomm.index.param.streaming.Media.Track.Encoding"
#define OMX_QUALCOMM_INDEX_CONFIG_DASH_RESUME_DISCONTINUITY               "OMX.Qualcomm.index.config.streaming.DASH.resumediscontinuity"
#define OMX_QUALCOMM_INDEX_CONFIG_DASH_REPOSITION_RANGE                   "OMX.Qualcomm.index.config.streaming.DASH.repositionrange"

#define MAX_KID_SIZE         16
#define MAX_IV_SIZE          16
#define MAX_ENCRYPTED_ENTRY 256
#define MAX_SUBTITLE_SUB_INFO_SIZE 2048


/**
 * Dash Properties Info
 *
 * STRUCT MEMBERS:
 *  nSize             : Size of the structure in bytes
 *  nVersion          : OMX specification version information
 *  nPortIndex        : Port that this structure applies to
 *  nPropertiesSize   : Specifies size of the cDashProperties
 *  cDashProperties   : holds the Properties information
 *
 */
typedef struct QOMX_DASHPROPERTYINFO {
  OMX_U32 nSize;              /**< Size of the structure in bytes. */
  OMX_VERSIONTYPE nVersion;   /**< OpenMAX IL specification version information. */
  OMX_U32 nPortIndex;         /**< Index of the port to which this structure applies. */
  OMX_U32 nPropertiesSize;   /**< Size of the cDashProperties. Set nPropertiesSize to 0
                                   to retrieve the size required for cDashProperties. */
  OMX_U8 cDashProperties[1]; /**< Properties information. */
} QOMX_DASHPROPERTYINFO;


/**
 * Enumeration identifying the DRM type.
 */
typedef enum QOMX_DRM_TYPE{
  QOMX_NO_DRM,          /**< There is no DRM*/
  QOMX_DRM_UNKNOWN,     /**< There is some DRM but failed to identify the scheme*/
  QOMX_DIVX_DRM,        /**< Underlying file is protected using DIVX DRM*/
  QOMX_JANUS_DRM,       /**< Underlying file is protected using JANUS/WM DRM*/
  QOMX_PLAYREADY_DRM,   /**< Underlying file is protected using PLAYREADY DRM*/
  QOMX_CENC_DRM,        /**< Underlying file is protected using common encryption scheme*/
  QOMX_MARLIN_DRM,      /**< Underlying file is protected using MARLIN DRM*/
  QOMX_HDCP_DRM         /**< Underlying file is protected using HDCP DRM*/
} QOMX_DRM_TYPE;


typedef enum ContentStreamType
{
 MP4_STREAM = 0,
 BBTS_STREAM,
 INVALID_STREAM = 0xFF
}ContentStreamType;


/**
 * DRM information
 *
 * STRUCT MEMBERS:
 *
 * nSize                    :Size of the structure in bytes
 * nVersion                 :OMX specification version information
 * nPortIndex               :Port that this structure applies to
 * eDrmType                 :enumeration to determine DRM type
 * nSystemIdBufSize          :systemId buffer size
 * nKidCount                 :number of kid mapping entry. Present only when pssh version > 0
 *                           for Marlin DRM it is zero.
 * nKidDataBufSize           :Size of the KID data buffer string. Present only when pssh version > 0
 *                           for Marlin DRM it is zero.
 *                           (excluding any  terminating characters)
 * nPsshDataBufSize          :Size of the pssh data buffer string(excluding any  terminating characters)
 * nContentProtectionInfoSize : MPD ContentProtection Element size
 * nDefaultKeyIDSize          : Default Key ID size
 * cDrmSystemSpecificData    :the NULL-terminated DRM System SpecificData string
 *                           formed by concatenating systemId,  kid data buffer and
 *                           pssh data buffer string in the following way:
 *                           systemId (of size systemIDBufSize) +
 *                           kidDataBuf (of size kidDataBufSize) +
 *                           psshDataBuf (of size psshDataBufSize) +
 *                           contentProtectionElement of MPD
 *                           (of size nContentProtectionInfoSize) +
 *                           defaultKeyID (of size nDefaultKeyIDSize)
 */
typedef struct QOMX_PARAM_STREAMING_DRMINFO
{
  OMX_U32 nSize;                               /**< Size of the structure in bytes. */
  OMX_VERSIONTYPE nVersion;     /**< OpenMAX IL specification version information. */
  OMX_U32 nPortIndex;                      /**< Index of the port to which this structure applies. */
  QOMX_DRM_TYPE  eDrmType;       /**< DRM Type*/
  OMX_U32  nSytemIdBufSize;          /**< DRM SystemID size */
  OMX_U32  nKidCount;                     /**<  number of kid mapping entry */
  OMX_U32  nKidDataBufSize;          /**< KID mapping data buffer size*/
  OMX_U32  nPsshDataBufSize;         /**< PSSH data buffer size*/
  OMX_U32  nContentProtectionInfoSize; /**< ContentProtectionElement size*/
  OMX_U32  nDefaultKeyIDSize;           /**< Default Key ID size*/
  OMX_U8   cDrmSystemSpecificData[1];   /**< DRM system specific data*/
} QOMX_PARAM_STREAMING_DRMINFO;

/*!
 *@brief Structure to PSSH information for a track
 * STRUCT MEMBERS:
 *
 * nSize                    :Size of the structure in bytes
 * nVersion                 :OMX specification version information
 * nPortIndex               :Port that this structure applies to
 * nUniqueID                :uniqueID associated with each PSSH query
 * nPsshDataBufSize         :Size of the pssh data string
 * cPSSHData                :PSSH data
 *
 ** Associated with extension: OMX_QUALCOMM_INDEX_PARAM_PSSH_INFO
 */
typedef struct QOMX_PARAM_STREAMING_PSSHINFO
{
  OMX_U32 nSize;                         /**< Size of the structure in bytes. */
  OMX_VERSIONTYPE nVersion;              /**< OpenMAX IL specification version information. */
  OMX_U32  nPortIndex;                   /**< Index of the port to which this structure applies. */
  OMX_S32  nUniqueID;                    /**< Unique ID to query a scpecifc PSSHInfo*/
  OMX_U32  nPsshDataBufSize;             /**< PSSH data buffer size*/
  OMX_U8   cDefaultKeyID[MAX_KID_SIZE];  /**< Default Key ID buffer of size MAX_KID_SIZE>*/
  OMX_U8   cPSSHData[1];                 /**< PSSH data byte array*/
} QOMX_PARAM_STREAMING_PSSHINFO;


/*!
 *@brief Structure to Content Protection information
 * STRUCT MEMBERS:
 *
 * nSize                    :Size of the structure in bytes
 * nVersion                 :OMX specification version information
 * eDrmType                 :enumeration to determine DRM type
 * nContentProtectionInfoSize  :Size of the contentProtection Elemenet data
 * cContentProtectionData      :contentProtection Elemenet data
 *
 * Associated with extension: OMX_QUALCOMM_INDEX_PARAM_CONTENTPROTECTION_INFO
 */
typedef struct QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO
{
  OMX_U32 nSize;                         /**< Size of the structure in bytes. */
  OMX_VERSIONTYPE nVersion;              /**< OpenMAX IL specification version information. */
  OMX_U32  nPortIndex;                   /**< Index of the port to which this structure applies. */
  QOMX_DRM_TYPE  eDrmType;               /**< DRM Type*/
  ContentStreamType  nStreamType;        /**< Stream TYpe */
  OMX_U32  nContentProtectionInfoSize;    /**< ContentProtectionElement size*/
  OMX_U8   cContentProtectionData[1];     /**< ContentProtection byte array*/
} QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO;



/*!
 *@brief Structure to store subsample encryption information in a sample
 */
typedef struct QOMX_ENCRYPTED_SUBSAMPLE_INFO
{
  OMX_U16  nSizeOfClearData;          /**< Number of bytes of clear data in subsample */
  OMX_U32  nOffsetClearData;          /**<Offset of clear data in subsample from start*/
  OMX_U32  nSizeOfEncryptedData;      /**< Number of bytes of encrypted data in subsample*/
  OMX_U32  nOffsetEncryptedData;      /**< Offset of encrypted data in subsample from start */
}QOMX_ENCRYPTED_SUBSAMPLE_INFO;

/**
 * STRUCT MEMBERS:
 *
 * nSize                    :Size of the structure in bytes
 * nVersion                 :OMX specification version information
 * eDrmType                 :enumeration to determine DRM type
 * nSubSampleCount              :Number of subsample
 * nIVSize                      : Initialisation vector size
 * nDefaultKeyID                : Default key data of size MAX_KID_SIZE
 * nKeyID                       : Key data of size MAX_KID_SIZE
 * nInitVector                  : Initialisation vector data of size MAX_IV_SIZE
 * sEncSubsampleInfo            : Actual sub sample (Clear and Enc data) data
 * Structure to store extra sample information.
 */
typedef struct QOMX_EXTRA_SAMPLE_INFO
{
  OMX_U16 nSubSampleCount;           /**< Number of subsample count present for current sample*/
  OMX_U16 nIsEncrypted;              /**< Encryption state flag. 0x0: Not Encrypted, 0x1: Encrypted
                                      0x1:- AES-CTR, 0x2 - 0xFFFF:- Reserved */
  OMX_U8  nKeyIDSize;                /**< Key Identifier size */
  OMX_U8  nIVSize;                   /**< Initialization Vector size */
  OMX_U8  nDefaultKeyID[MAX_KID_SIZE]; /**< Default Key Identifier for to decrypt the associated samples */
  OMX_U8  nKeyID[MAX_KID_SIZE];      /**< Key Identifier  to decrypt the associated samples */
  OMX_U8  nInitVector[MAX_IV_SIZE];  /**< Initialization vector for sample decryption */
  QOMX_ENCRYPTED_SUBSAMPLE_INFO sEncSubsampleInfo[MAX_ENCRYPTED_ENTRY];   /**< Encrypted subsample
                                         information, it will carry information! of bytes of
                                         clear/encrypted data and its  offset from start. */
}QOMX_EXTRA_SAMPLE_INFO;


/**
 * Enumeration used to define the possible Other compression
 * coding.
 */
typedef enum QOMX_OTHER_CODINGTYPE {
  QOMX_OTHER_CodingUnused = 0x7F000000,
  QOMX_OTHER_CodingAutoDetect,
  QOMX_OTHER_CodingSMPTETT,
  QOMX_OTHER_CodingHevc
}QOMX_OTHER_CODINGTYPE;

typedef enum QOMX_EXT_AUDIO_CODINGTYPE {
  QOMX_EXT_AUDIO_CodingUnused = 0x7F000000,
  QOMX_EXT_AUDIO_CodingAC3,
  QOMX_EXT_AUDIO_CodingMP2
}QOMX_EXT_AUDIO_CODINGTYPE;


/* AC3 params */
typedef struct QOMX_AUDIO_PARAM_AC3PROFILETYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSampleRate;           /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    OMX_U32 nAudioBandWidth;       /**< Audio band width (in Hz) to which an encoder should
                                        limit the audio signal. Use 0 to let encoder decide */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
} QOMX_AUDIO_PARAM_AC3PROFILETYPE;

/* MP2 params */
typedef struct QOMX_AUDIO_PARAM_MP2PROFILETYPE {
    OMX_U32 nSize;                 /**< size of the structure in bytes */
    OMX_VERSIONTYPE nVersion;      /**< OMX specification version information */
    OMX_U32 nPortIndex;            /**< port that this structure applies to */
    OMX_U32 nChannels;             /**< Number of channels */
    OMX_U32 nBitRate;              /**< Bit rate of the input data.  Use 0 for variable
                                        rate or unknown bit rates */
    OMX_U32 nSampleRate;           /**< Sampling rate of the source data.  Use 0 for
                                        variable or unknown sampling rate. */
    OMX_U32 nAudioBandWidth;       /**< Audio band width (in Hz) to which an encoder should
                                        limit the audio signal. Use 0 to let encoder decide */
    OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
} QOMX_AUDIO_PARAM_MP2PROFILETYPE;

/**
 * Structure to indicating Height, width and Duration of the
 * subtitile sample
 */
typedef struct QOMX_BUFFERED_DURATION
{
  OMX_U32 nPortIndex;
  OMX_U64 bufDuration;
}QOMX_BUFFERED_DURATION;

/**
 * Structure to indicating Height, width and Duration of the
 * subtitile sample
 */
typedef struct QOMX_SUBTITILE_DIMENSIONS
{
  OMX_U32 nHeight;
  OMX_U32 nWidth;
  OMX_U32 nDuration;
  OMX_U64 nStartOffset;
}QOMX_SUBTITILE_DIMENSIONS;


/**
 * Structure to store subtitle track sub sample information.
 */
typedef struct QOMX_SUBTITLE_SUB_INFOTYPE{
  OMX_U16 subSampleCount;  /**< Number of subsample count present for current sample */
  OMX_IMAGE_CODINGTYPE eSubCodingType; /**< encoding type of the subs*/
  OMX_U32 nSubtitleSubInfoSize; /**< Subtitle sub information size */
  OMX_U8 cSubtitleSubInfo[MAX_SUBTITLE_SUB_INFO_SIZE]; /**< Subtitle sub info data,
                                                          which carry explicit control information*/
}QOMX_SUBTITLE_SUB_INFOTYPE;

typedef struct QOMX_QOE_EVENT_REG{
  OMX_BOOL bNotify;               /** Value to set for QOE Notofication */
}QOMX_QOE_EVENT_REG;

typedef struct QOMX_QOE_DATA_PLAY{
  OMX_U32 size;
  OMX_U64 timeOfDay;               /** Time of day() in milliseconds */
}QOMX_QOE_DATA_PLAY;

typedef struct QOMX_QOE_DATA_STOP{
  OMX_U32 size;
  OMX_U32 bandwidth;               /** Bandwidth in kbps */
  OMX_U32 reBufCount;              /** Buffering count */
  OMX_U64 timeOfDay;               /** Time of day() in milliseconds */
  OMX_U32 nStopPhraseLen;          /** Stop Prase Length including Null terminating character */
  OMX_U32 nVideoURLLen;            /** Video URL Length including Null terminating character */
  OMX_U32 nInfoStopLen;            /** infoStop Length*/
  OMX_U8  infoStop[1];             /** this will hold StopPharase string + NULL Terminating character +
                                       Video URL + NULL Terminating character*/
}QOMX_QOE_DATA_STOP;

typedef struct QOMX_QOE_DATA_SWITCH{
  OMX_U32 size;
  OMX_U32 bandwidth;               /** Bandwidth in kbps */
  OMX_U32 reBufCount;              /** Buffering count */
  OMX_U64 timeOfDay;               /** Time of day() in milliseconds */
}QOMX_QOE_DATA_SWITCH;

typedef struct QOMX_QOE_DATA_PERIODIC{
  OMX_U32 size;
  OMX_U32 bandwidth;               /** Bandwidth in kbps */
  OMX_U64 timeOfDay;               /** Time of day() in milliseconds */
  OMX_U32 nIpAddressLen;           /** IPAddress Length including Null terminating character */
  OMX_U32 nVideoURLLen;            /** Video URL Length including Null terminating character */
  OMX_U32 nInfoPeriodicLen;         /** infoPerodic Len*/
  OMX_U8  infoPeriodic[1];         /** this will hold IpAddress string + NULL Terminating character +
                                       Video URL + NULL Terminating character*/
}QOMX_QOE_DATA_PERIODIC;


typedef struct QOMX_PARAM_REPRESENTATION{
  OMX_U32 nSize;                      /**< Size of the structure in bytes. */
  OMX_U32 nRepSize;                   /**< Current representation size */
  OMX_U8  cRepString[1];               /**< Representation String + NULL Terminating character*/
}QOMX_PARAM_REPRESENTATION;

typedef struct QOMX_PARAM_MEDIA_TRACK_ENCODING{
    OMX_U32 nSize;                       /**< Size of the structure in bytes. */
    OMX_AUDIO_CODINGTYPE  audioEncoding; /**< Audio encoding */
    OMX_VIDEO_CODINGTYPE  videoEncoding; /**< Video encoding */
    QOMX_OTHER_CODINGTYPE otherEncoding; /**< Other encoding */
}QOMX_PARAM_MEDIA_TRACK_ENCODING;

typedef struct QOMX_PARAM_COOKIES{
  OMX_U32 nSize;                      /**< Size of the structure in bytes. */
  OMX_U32 nURLLen;                    /**< URL String length (NULL termination character included)*/
  OMX_U32 nCookieLen;                  /**< URL String length (NULL termination character included)*/
  OMX_U8 cURLString[1];              /**< NULL terminated URL String*/
  OMX_U8 cCookieString[1];           /**< NULL terminated Cookie String*/
}QOMX_PARAM_COOKIES;


/**
* Structure to query for resume discontinuity
*/
typedef struct QOMX_DASH_RESUME_DISCONTINUITY
{
  OMX_U32 nSize;                  /**< Size of the structure in bytes. */
  OMX_VERSIONTYPE nVersion;       /**< OpenMAX IL specification version information. */
  OMX_U32 nPortIndex;             /**< Index of the port to which this structure applies. */
  OMX_BOOL bDiscontinuity;        /**< Resume discontinuity flag (true if discontinuous i.e. resuming ahead). */
}QOMX_DASH_RESUME_DISCONTINUITY;

/**
* Structure to indicate the reposition time range on the playback scale (in msec)
*/
typedef struct QOMX_DASH_REPOSITION_RANGE
{
  OMX_U32 nSize;                  /**< Size of the structure in bytes. */
  OMX_VERSIONTYPE nVersion;       /**< OpenMAX IL specification version information. */
  OMX_U32 nPortIndex;             /**< Index of the port to which this structure applies. */
  OMX_U64 nMin;                   /**< Minimum reposition time (left edge of reposition window). */
  OMX_U64 nMax;                   /**< Maximum reposition time (right edge of reposition window). */
  OMX_U64 nMaxDepth;              /**< Maximum TSB depth */
  OMX_BOOL  bDataEnd;             /**< Indicates Data End */
}QOMX_DASH_REPOSITION_RANGE;

#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* QOMX_STREAMINGEXTENSIONS_PRIVATE_H_ */
