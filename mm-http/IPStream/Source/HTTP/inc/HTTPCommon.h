#ifndef __HTTPCOMMON_H__
#define __HTTPCOMMON_H__
/************************************************************************* */
/**
 * HTTPCommon.h
 * @brief Header file for HTTPCommon.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPCommon.h#59 $
$DateTime: 2013/08/16 11:51:00 $
$Change: 4287091 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPCommon.h
** ======================================================================= */
#include <AEEstd.h>
#include <HTTPAPI.h>
#include <qtv_msg.h>
#include <StreamSourceClock.h>
#include <StreamQueue.h>
#include <MMCriticalSection.h>
#include <filesource.h>
#include "IPStreamProtocolHeaders.h"
#include "HTTPStackInterface.h"
#include "Url.h"
#include "httpInternalDefs.h"

using namespace HTTP_API;

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
#define HTTPSUCCEEDED(status) \
  (status == HTTPCommon::HTTPDL_SUCCESS || status==HTTPCommon::HTTPDL_EXISTS)

#define HTTP_MAX_NUMBER_OF_SEGMENTS              60

#define HTTP_DEFAULT_ONDEMAND_PREROLL_MSEC       3000
#define HTTP_DEFAULT_LIVE_SWITCH_PREROLL_MSEC    HTTP_DEFAULT_ONDEMAND_PREROLL_MSEC
#define HTTP_DEFAULT_LIVE_NOSWITCH_PREROLL_MSEC  900

#define MAX_SUPPORTED_BITRATE_BPS                2 * 1024 * 1024  //(bps)
#define HTTP_MAX_CONNECTIONS                     4  // Num of connecttions to be announced to QSM
#define HTTP_RESOLVER_REQUEST_LENGTH             -1 // default request length (-1 = entire content)
#define HTTP_HEAP_SIZE_UNKNOWN                   -1

#define MAX_UINT32_VAL 0xffffffff
#define MAX_UINT64_VAL 0xffffffffffffffffULL
#define MAX_RESOLUTION                           1920 * 1080

#define MAX_TSB_BUFFER_SECS                      60
#define MAX_MINUPDATE_PERIOD_SECS                5 * 60 // 5 minutes

#define MARLIN_SCHEMEID_URI                      "urn:uuid:5E629AF5-38DA-4063-8977-97FFBD9902D4"
#define PLAYREADY_SCHEMEID_URI                   "urn:uuid:9a04f079-9840-4286-ab92-e65be0885f95"
#define CENC_MP4_SCHEMEID_URI                    "urn:mpeg:dash:mp4protection:2011"
#define CENC_MP2TS_SCHEMEID_URI                  "urn:mpeg:dash:13818:1:CA_descriptor:2011"



/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPSessionInfo;
class HTTPStackInterface;
class iMPDParser;
class HTTPHeapManager;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class HTTPCommon
{
public:
  enum HTTPDownloadStatus
  {
    HTTPDL_SUCCESS,
    HTTPDL_ERROR_ABORT,
    HTTPDL_OUT_OF_MEMORY,
    HTTPDL_WAITING,
    HTTPDL_UNSUPPORTED,
    HTTPDL_INTERRUPTED,
    HTTPDL_TIMEOUT,
    HTTPDL_INIT_PARSER,
    HTTPDL_EXISTS,
    HTTPDL_TRACKS_AVALIABLE,
    HTTPDL_INSUFFICIENT_BUFFER,
    HTTPDL_CODEC_INFO,
    HTTPDL_CODEC_INFO_CHANGED,
    HTTPDL_DATA_END,
    HTTPDL_SWITCH,
    HTTPDL_NO_MORE_RESOURCES,
    HTTPDL_SEGMENT_BOUNDARY,
    HTTPDL_NEXT_SEGMENT,
    HTTPDL_DATA_END_WITH_ERROR,
    HTTPDL_INVALID_REPRESENTATION,
    HTTPDL_SEGMENT_NOT_FOUND,
  };
  enum HTTPFlavor
  {
    HTTP_PROGRESSIVE_DOWNLOAD,
    HTTP_FASTTRACK,
    HTTP_LIVE,
    HTTP_DASH,
    HTTP_NONE
  };
  enum HTTPDataCommand
  {
    FLUSH,
    DATA_REQUEST,
    DATA_CMD_MAX
  };
  enum HTTPControllerCommand
  {
    OPEN,
    CLOSE,
    START,
    STOP,
    PLAY,
    PAUSE,
    DOWNLOAD,
    SEEK,
    GET_TRACKS,
    SET_TRACK_STATE,
    WAIT_FOR_RESOURCES,
    NOTIFY_CURRENT_WATERMARK_STATUS,
    SET_AUTHORIZATION,
    SELECT_REPRESENTATIONS,
    CMD_MAX
  };
  enum HTTPDownloadProgressUnitsType
  {
    HTTP_DOWNLOADPROGRESS_UNITS_DATA,
    HTTP_DOWNLOADPROGRESS_UNITS_TIME
  };

  enum HTTPMediaType
  {
    HTTP_UNKNOWN_TYPE,
    HTTP_AUDIO_TYPE,
    HTTP_VIDEO_TYPE,
    HTTP_TEXT_TYPE,
    HTTP_MAX_TYPE
  };

  /*
  * Determine if server honors byte range header as part of original url
  * Or uses a byterangeURL since it dishonors byte range header
  * (for http1.0)
  */
  enum ByteRangeURLRespState
  {
    BYTERANGE_URL_USED_TRUE,
    BYTERANGE_URL_USED_FALSE,
    BYTERANGE_URL_USED_UNDETERMINED
  };

  enum HTTPAttribute
  {
    HTTP_ATTR_DURATION,
    HTTP_ATTR_PLAYBACK_DISCONTINUITY,
    HTTP_ATTR_REPOSITION_RANGE,
    HTTP_ATTR_MAX
  };
  struct RepositionRange
  {
    bool   bDataEnd;
    uint64 nMin;
    uint64 nMax;
    uint64 nMaxDepth;
  };
  typedef union HTTPAttrVal
  {
    uint32 int_attr_val;
    //char* string_attr_val;
    //double double_attr_val;
    bool bool_attr_val;
    RepositionRange sReposRange;
  }HTTPAttrVal;

  /*!
  *@brief Enumeration identifying the DRM type.
  */
  typedef enum
  {
    NO_DRM,       //There is no DRM
    DRM_UNKNOWN,  //There is some DRM but failed to identify the scheme
    DIVX_DRM,     //Underlying file is protected using DIVX DRM
    JANUS_DRM,    //Underlying file is protected using JANUS/WM DRM
    PLAYREADY_DRM,//Underlying file is protected using PLAYREADY DRM
    CENC_DRM,     //Underlying file is protected using common encryption scheme
    MARLIN_DRM,   //Underlying file is protected using MARLIN DRM
    HDCP_DRM      //Underlying file is protected using HDCP DRM
  } HTTPDrmType;


  typedef enum
  {
    MP4_STREAM = 0,
    BBTS_STREAM,
    INVALID_STREAM = 0xFF
  }HTTPContentStreamType;


  static void ShowHTTPDownloadStatus(const HTTPDownloadStatus status);

  static uint32 GetElapsedTime(StreamSourceClock* pSourceClock,
                               const uint32 startTime);

  static uint32 ConvertString2UnsignedInteger(const char* pStr);

  /** @brief Convert an unsigned integer (of 4 bytes) to string.
  *
  * @param[in] value - Input integer
  * @param[out] pStr - Reference to converted string
  */
  static void ConvertUnsignedInteger2String(const uint32 value, char* pStr)
  {
    if (pStr)
    {
      std_memmove(pStr, (const char*)&value, sizeof(value));
    }
  };
  static void AddIPStreamProtocolHeaders(HTTPSessionInfo& httpSessionInfo,
                                         HTTPStackInterface& httpStack, HTTPMethodType methodType,
                                         uint32 httpRequestId);
  static AffectedMethod HttpMethodEnumConvert( HTTPMethodType originalMethodValue);

  /**
   * @brief This Method configures the given HTTPStack reference
   *        with the Network Interface, PDPProfileNo, and proxy
   *        server info required to bringup the data call and
   *        initiate HTTP connection
   *
   * @param[in]      httpSessionInfo pointer to retrive the info
   * @param[in/out]  reference to the httpStack that needs to be
   *                   configured
   *
   * @return bool
   */
  static bool ConfigureHTTPStack(const HTTPSessionInfo &httpSessionInfo, HTTPStackInterface& httpStack);

  /** @brief Parses the input URL and fills in the default port if port already
    * not present.
    *
    * @param[in] nRequestID - HTTP Stack request ID
    * @param[in] url - URL to be scanned
    * @param[in] pDefaultPort - Default HTTP port to use if port not present in URL
    * @param[out] pLaunchURL - Launch URL used to connect to server
    * @return
    * TRUE - Launch URL ready
    * FALSE - Otherwise
    */
  static bool ParseURL( const URL &url, const char* pDefaultPort, char*& pLaunchURL );

  /** @brief Get content length and type.
    *
    * @param[in] nRequestID - HTTP Stack request ID
    * @param[in] httpStack - HTTP Stack interface to query from
    * @param[out] contentLength - Reference to content length
    * @param[out] pContentType - Reference to content type
    * @return
    * HTTPDL_SUCCESS - Content length and type successfully obtained
    * HTTPDL_ERROR_ABORT - Otherwise
    */
  static HTTPDownloadStatus GetContentLengthAndType( uint32 nRequestID,
    HTTPStackInterface& httpStack, int64& contentLength, char*& pContentType );

  static void MapHTTPMediaTypeToFileSourceMajorType(HTTPMediaType mediaType,
                                                    FileSourceMjMediaType& majorType);
  static void MapFileSourceMajorTypeToHTTPMediaType(FileSourceMjMediaType majorType,
                                                    HTTPMediaType &mediaType);
};

//Playback handler interface
class iHTTPPlaybackHandler
{
public:
  virtual ~iHTTPPlaybackHandler() {};

  //Check if player is buffering
  virtual bool IsBuffering() = 0;
};

//Readable interface
class iHTTPReadable
{
public:
  virtual ~iHTTPReadable() {};

  //Get total bytes downloaded
  virtual uint64 GetTotalBytesReceived() = 0;

  //Check if data requested to be read is actually downloaded/written
  //into buffer
  virtual bool IsReadable(int32 readOffset, int32 bufSize) = 0;
};

// define HTTPMediaType before it is referenced
typedef HTTPCommon::HTTPMediaType HTTPMediaType;

class iHTTPFileSourceHelper
{
public:
  virtual ~iHTTPFileSourceHelper() {};

  // Get the current cumulative (over all tracks) read point
  virtual bool GetMinimumMediaOffset(uint64& nMediaOffset) = 0;

  // Get the media offset for the given time
  virtual bool GetOffsetForTime(const int64 nTime, int64& nOffset) = 0;

  // Get play TimeOffset for a given byteoffset
  virtual bool GetTimeForOffset(HTTPMediaType mediaType,
                                uint64 offset,uint64 &nTimeOffset) = 0;
};

//DASH session info
class DASHSessionInfo
{
public:
  DASHSessionInfo(HTTPSessionInfo& rSessionInfo,
                  iMPDParser& rMPDparser,
                  HTTPHeapManager& rHeapManager);
  ~DASHSessionInfo();
  HTTPSessionInfo& sSessionInfo;
  iMPDParser& cMPDParser;
  HTTPHeapManager& cHeapManager;
};

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
struct QElem
{
  StreamQ_link_type link;
  void* data;
};

typedef iHTTPAPI::DataStorageType DataStorageType;

typedef HTTPCommon::HTTPDownloadStatus HTTPDownloadStatus;
typedef HTTPCommon::HTTPFlavor HTTPFlavor;
typedef HTTPCommon::HTTPControllerCommand HTTPControllerCommand;
typedef HTTPCommon::HTTPDataCommand HTTPDataCommand;
typedef HTTPCommon::HTTPDownloadProgressUnitsType HTTPDownloadProgressUnitsType;
typedef HTTPCommon::HTTPDrmType HTTPDrmType;
typedef HTTPCommon::HTTPContentStreamType HTTPContentStreamType;


typedef int (*SchedulerTask)(void*);

typedef void (*SegmentPurgedHandler) (uint64 nSegmentKey, void *pHandlerData);

}/* namespace video */
#endif /* __HTTPCOMMON_H__ */
