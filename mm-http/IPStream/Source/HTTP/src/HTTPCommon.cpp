/************************************************************************* */
/**
 * HTTPCommon.cpp
 * @brief implementation of HTTPCommon.
 *  HTTPCommon provides generic utility/helper routines for the HTTP module.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPCommon.cpp#19 $
$DateTime: 2013/06/12 21:28:48 $
$Change: 3913306 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPCommon.cpp
** ======================================================================= */
#include "HTTPCommon.h"
#include "HTTPSessionInfo.h"
#include "HTTPStackInterface.h"

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief This method prints the HTTP download status
  *
  * @param[in] status - HTTP download status code
  */
void HTTPCommon::ShowHTTPDownloadStatus
(
 const HTTPDownloadStatus status
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPCommon::ShowHTTPDownloadStatus" );

  //Print the HTTP download status
  switch (status)
  {
    case HTTPDL_SUCCESS:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_SUCCESS" );
      break;
    case HTTPDL_ERROR_ABORT:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_ERROR_ABORT" );
      break;
    case HTTPDL_OUT_OF_MEMORY:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_OUT_OF_MEMORY" );
      break;
    case HTTPDL_WAITING:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_WAITING" );
      break;
    case HTTPDL_UNSUPPORTED:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_UNSUPPORTED" );
      break;
    case HTTPDL_INTERRUPTED:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_INTERRUPTED" );
      break;
    case HTTPDL_TIMEOUT:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_TIMEOUT" );
      break;
    case HTTPDL_INIT_PARSER:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_INIT_PARSER" );
      break;
    case HTTPDL_TRACKS_AVALIABLE:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_TRACKS_AVALIABLE" );
      break;
    case HTTPDL_INSUFFICIENT_BUFFER:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_INSUFFICIENT_BUFFER" );
      break;
    case HTTPDL_CODEC_INFO:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_CODEC_INFO" );
      break;
    case HTTPDL_DATA_END:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_DATA_END" );
      break;
    case HTTPDL_SWITCH:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "HTTP download status - HTTPDL_SWITCH" );
      break;
    default:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "Unknown HTTP download status" );
      break;
  }
}

/** @brief This method returns the elapsed time from a specified start time.
  *
  * @param[in] pSourceClock - Reference to stream source clock
  * @param[in] startTime - Start time
  * @return Elapsed time since the start time
  */
uint32 HTTPCommon::GetElapsedTime
(
 StreamSourceClock* pSourceClock,
 const uint32 startTime
)
{
  uint64 timeDiff = 0;

  if (pSourceClock)
  {
    uint32 currTime = pSourceClock->GetTickCount();

    if (currTime >= startTime)
    {
      timeDiff = static_cast<uint64>(currTime - startTime);
    }
    else
    {
      //Don't have to worry about currTime wraparound (since session start).
      //Can be here if system time rolls back say due to NTP sync correction
      //(e.g. device roams from LTE->CDMA system), in which case caller will
      //just ignore this sample
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error: Incorrect startTime %u/%u start/curr",
                     startTime, currTime );
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: pSourceClock is NULL" );
  }

  return static_cast<uint32>(timeDiff);
}

/** @brief Convert a string to unsigned integer (of 4 bytes).
  *
  * @param[in] pStr - Reference to input string
  *
  * @return Converted value
  */
uint32 HTTPCommon::ConvertString2UnsignedInteger
(
 const char* pStr
)
{
  uint32 value = 0;

  if (pStr)
  {
    //Don't know whether pStr is NULL terminated - so
    //convert only 4 bytes
    for (uint32 i = 0; i < sizeof(value); i++)
    {
      reinterpret_cast<char *> (&value)[i] = pStr[i];
    }
  }

  return value;
}
/**
 * @brief Add protocol headers to request methods
 *
 * @param methodType request method type
 *
 * @return none
 */
void HTTPCommon::AddIPStreamProtocolHeaders(HTTPSessionInfo& httpSessionInfo,
  HTTPStackInterface& httpStack, HTTPMethodType methodType, uint32 httpRequestId)
{
  const char *name = NULL;
  const char *value = NULL;
  // convert the method type to enum understood by oem headers object
  AffectedMethod oemMethod = HttpMethodEnumConvert(methodType);
  bool moreHeaders = httpSessionInfo.GetProtocolHeadersCache().FindFirst(oemMethod, name, value);

  while (moreHeaders)
  {
    QTV_MSG_SPRINTF_2(QTVDIAG_GENERAL,"Add OEM Hdr: '%s: %s", name, value);
    httpStack.SetHeader(httpRequestId, name, (int)std_strlen(name),
                          value, (int)std_strlen(value));
    moreHeaders = httpSessionInfo.GetProtocolHeadersCache().FindNext(name, value);
  }
  return;
}

/**
 * @brief rtspMethodEnumConvert
 * helper function.  Converts method from RTSPMethod enumeration to
 * QtvPlayer.h equivalent values, enum AffectedRTSPMethod.
 * @param originalMethodValue
 *
 * @return AffectedRTSPMethod/ Converted enum.  If fails, returns RTSP_METHOD_NONE
 */
AffectedMethod HTTPCommon::HttpMethodEnumConvert(HTTPMethodType originalMethodValue)
{
   AffectedMethod result = HTTP_METHOD_NONE;
   switch (originalMethodValue)
   {
      case HTTP_GET:
         result = HTTP_METHOD_GET;
         break;
      case HTTP_OPTIONS:
         result = HTTP_METHOD_OPTIONS;
         break;
      case HTTP_HEAD:
         result = HTTP_METHOD_HEAD;
         break;
      case HTTP_POST:
         result = HTTP_METHOD_POST;
         break;
      default:
         break;
   }
   return result;
}
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
bool HTTPCommon::ConfigureHTTPStack
(
  const HTTPSessionInfo &httpSessionInfo,
  HTTPStackInterface& httpStack
)
{
  bool rsltCode = true;
  int32 networkIfaceId = httpSessionInfo.GetNetworkInterface();
  int32 pdpProfileNo = httpSessionInfo.GetPrimaryPDPProfile();
  const char *proxyServer = httpSessionInfo.GetProxyServer();

  //Set socket mode to NON_BLOCKING
  (void)httpStack.SetSocketMode(HTTP_NON_BLOCKING);

  if(networkIfaceId >= 0)
  {
    httpStack.SetNetworkInterface(networkIfaceId);
  }

  if(pdpProfileNo >= 0)
  {
    httpStack.SetPrimaryPDPProfile(pdpProfileNo);
  }

  if(proxyServer != NULL)
  {
    if(HTTP_SUCCESS != httpStack.SetProxyServer(proxyServer,std_strlen(proxyServer)))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "ConfigureHTTPStack: SetProxyServer failed " );
      rsltCode = false;
    }
  }
  return rsltCode;
}

/** @brief Parses the input URL and prepares the launch URL to use. Default port is
  *        filled in if not already present in the input URL.
  *
  * @param[in] url - Reference to input URL
  * @param[in] pDefaultPort - Default HTTP port to use if port not present in URL
  * @param[out] pLaunchURL - Launch URL used to connect to server (memory for pLaunchURL
  *                          is allocated here - to be freed by caller)
  * @return
  * TRUE - Launch URL ready
  * FALSE - Otherwise
  */
bool HTTPCommon::ParseURL
(
 const URL &url,
 const char* pDefaultPort,
 char*& pLaunchURL
)
{
  bool bOk = false;
  size_t launchURLLen = url.GetUrlLength() + 1;
  uint32 port = 0;

  if (url.GetPort(&port) == URL::URL_OK)
  {
    if (port)
    {
      pLaunchURL = (char*)QTV_Malloc(launchURLLen);
      if (pLaunchURL)
      {
        bOk = true;
        (void)std_strlcpy(pLaunchURL, url.GetUrlBuffer(), launchURLLen);
      }
    }
    else if (pDefaultPort)
    {
      char hostName[HTTP_MAX_HOSTNAME_LEN] = {0};

      //Extra space for default port
      launchURLLen += std_strlen(pDefaultPort) + 1;
      pLaunchURL = (char*)QTV_Malloc(launchURLLen);
      if (pLaunchURL &&
          url.GetHost(hostName, sizeof(hostName)) == URL::URL_OK)
      {
        //Write http://
        size_t arg2 = std_strlen("http://") + 1;
        size_t dstBufLen = STD_MIN(launchURLLen, arg2);
        size_t numURLBytes = std_strlcpy(pLaunchURL,
                                      "http://", dstBufLen);

        //Write hostname
        arg2 = std_strlen(hostName) + 1;
        dstBufLen = STD_MIN(launchURLLen - numURLBytes, arg2);
        numURLBytes += std_strlcpy(pLaunchURL + numURLBytes,
                                   hostName, dstBufLen);

        //Write :
        arg2 = std_strlen(":") + 1;
        dstBufLen = STD_MIN(launchURLLen - numURLBytes, arg2);
        numURLBytes += std_strlcpy(pLaunchURL + numURLBytes,
                                   ":", dstBufLen);

        //Write default port
        arg2 = std_strlen(pDefaultPort) + 1;
        dstBufLen = STD_MIN(launchURLLen - numURLBytes, arg2);
        numURLBytes += std_strlcpy(pLaunchURL + numURLBytes,
                                   pDefaultPort, dstBufLen);

        //Write /
        arg2 = std_strlen("/") + 1;
        dstBufLen = STD_MIN(launchURLLen - numURLBytes, arg2);
        numURLBytes += std_strlcpy(pLaunchURL + numURLBytes,
                                   "/", dstBufLen);

        //Write clipname
        dstBufLen = launchURLLen - numURLBytes;
        char* pClipName = NULL;
        pClipName = (char*)QTV_Malloc(dstBufLen);
        if (pClipName)
        {
          if (url.GetClipName(pClipName, dstBufLen) == URL::URL_OK)
          {
            bOk = true;
            (void)std_strlcpy(pLaunchURL + numURLBytes,
                              pClipName, dstBufLen);
          }
          QTV_Free(pClipName);
        }
      }
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: Invalid port" );
    }
  }
  if(!bOk)
  {
    if(pLaunchURL)
    {
      QTV_Free(pLaunchURL);
    }
  }
  return bOk;
}

/** @brief Get content length and type from HTTP stack.
  *
  * @param[in] nRequestID - HTTP Stack request ID
  * @param[in] httpStack - Reference to HTTP stack
  * @param[out] contentLength - Reference to content length
  * @param[out] pContentType - Reference to content type (memory for
  *             pContentType is allocated here - to be freed by caller)
  * @return
  * HTTPDL_SUCCESS - Content length and type successfully obtained
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus HTTPCommon::GetContentLengthAndType
(
 uint32 nRequestID,
 HTTPStackInterface& httpStack,
 int64& contentLength,
 char*& pContentType
)
{
  HTTPDownloadStatus status = HTTPDL_ERROR_ABORT;

  //Get content length
  if (httpStack.GetContentLength(nRequestID, &contentLength) == HTTP_SUCCESS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "HTTP Content length - %lld", contentLength );

    //Get content (mime) type - get the content type length first
    size_t contentTypeLen = 0;
    (void)httpStack.GetContentType(nRequestID, NULL, 0, &contentTypeLen);
    if (contentTypeLen > 0)
    {
      pContentType = (char*)QTV_Malloc(contentTypeLen);
      if (pContentType)
      {
        status = HTTPDL_SUCCESS;
        (void)httpStack.GetContentType(nRequestID, pContentType,
                                       contentTypeLen, &contentTypeLen);
        QTV_MSG_SPRINTF_PRIO_1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                "HTTP Content type - %s", pContentType );
      }
    }
  }

  return status;
}

/**
 * @brief This method maps HTTPMediaType to Filesource major type
 *
 * @param mediaType
 * @param majorType
 */
void HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType
(
 HTTPMediaType mediaType,
 FileSourceMjMediaType& majorType
)
{
  majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;

  if(mediaType == HTTPCommon::HTTP_AUDIO_TYPE)
  {
    majorType = FILE_SOURCE_MJ_TYPE_AUDIO;
  }
  else if (mediaType == HTTPCommon::HTTP_VIDEO_TYPE)
  {
    majorType = FILE_SOURCE_MJ_TYPE_VIDEO;
  }
  else if (mediaType == HTTPCommon::HTTP_TEXT_TYPE)
  {
    majorType = FILE_SOURCE_MJ_TYPE_TEXT;
  }
}

/**
 *@brief This Method maps FileSource MajorType to HTTPMedia Type
 *
 * @param fsMajorType
 * @param mediaType
 */
void HTTPCommon::MapFileSourceMajorTypeToHTTPMediaType
(
  FileSourceMjMediaType fsMajorType,
  HTTPMediaType &mediaType
)
{
  mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;

  if(fsMajorType ==  FILE_SOURCE_MJ_TYPE_AUDIO)
  {
    mediaType = HTTPCommon::HTTP_AUDIO_TYPE;
  }
  else if(fsMajorType ==  FILE_SOURCE_MJ_TYPE_VIDEO )
  {
    mediaType = HTTPCommon::HTTP_VIDEO_TYPE;
  }
  else if( fsMajorType ==  FILE_SOURCE_MJ_TYPE_TEXT)
  {
    mediaType = HTTPCommon::HTTP_TEXT_TYPE;
  }
}

DASHSessionInfo::DASHSessionInfo(
  HTTPSessionInfo& rSessionInfo,
  iMPDParser& rMPDparser,
  HTTPHeapManager& rHeapManager) :
  sSessionInfo(rSessionInfo),
  cMPDParser(rMPDparser),
  cHeapManager(rHeapManager)
{

}

DASHSessionInfo::~DASHSessionInfo()
{

}

}/* namespace video */
