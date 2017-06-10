/************************************************************************* */
/**
 * SegmentDownloader.cpp
 * @brief Implements the SegmentDownloader. Each such object interacts with
 *        HTTP stack to download the specified data segment.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/SegmentDownloader.cpp#35 $
$DateTime: 2013/08/16 11:51:00 $
$Change: 4287091 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <HTTPStackInterface.h>
#include "SegmentDownloader.h"
#include "HTTPSessionInfo.h"
#include "Scheduler.h"

namespace video {

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define HTTP_DEFAULT_PORT                  "80"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */


/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
/** @brief SegmentDownloader constructor.
  *
  */
SegmentDownloader::SegmentDownloader()
  : m_pHTTPStack(NULL),
    m_bHTTPStackLocallyAllocated(false),
    m_pSessionInfo(NULL),
    m_pDataLock(NULL),
    m_nNumBytesReceived(0),
    m_nContentLength(-1),
    m_nStartOffset(0),
    m_nKey(-1),
    m_nHTTPRequestID(-1),
    m_cIdleStateHandler(this),
    m_cWaitForDataStateHandler(this),
    m_cDataReadyStateHandler(this),
    m_pCurrentStateHandler(NULL),
    m_bIsDownloadTooSlowProcessed(false),
    m_IsDownloadDisabled(false),
    m_BufferedDurationCheckCount(0),
    m_pSegmentNotifier(NULL),
    m_pClock(NULL),
    m_pBandwidthEstimator(NULL),
    m_bIsSharedTimerRefCountIncreased(false),
    m_pSharedBandwidthEstimator(NULL),
    m_pByteRangeUrl(NULL),
    m_bSentByteRangeUrl(false),
    m_bIsByteRangeUrlInUse(false)
{
  bool rslt = false;
  m_pClock = QTV_New_Args(StreamSourceClock, (rslt));

  if (m_pClock)
  {
    m_pBandwidthEstimator = QTV_New(HTTPBandwidthEstimator);
    if (m_pBandwidthEstimator)
    {
      m_pBandwidthEstimator->Initialize(m_pClock);
      rslt = true;
    }
  }

  if (false == rslt)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Failed to initialize stream clock and bwEstimator in SegmentDownloader");
  }
}

/** @brief SegmentDownloader destructor.
  *
  */
SegmentDownloader::~SegmentDownloader()
{
  if (m_pHTTPStack && m_bHTTPStackLocallyAllocated)
  {
    QTV_Delete(m_pHTTPStack);
    m_pHTTPStack = NULL;
  }

  ReleaseSharedTimerRefCnt();

  if (m_pClock)
  {
    QTV_Delete(m_pClock);
    m_pClock = NULL;
  }

  if (m_pBandwidthEstimator)
  {
    QTV_Delete(m_pBandwidthEstimator);
    m_pBandwidthEstimator = NULL;
  }

  if (m_pDataLock)
  {
    (void)MM_CriticalSection_Release(m_pDataLock);
    m_pDataLock = NULL;
  }

  if (m_pByteRangeUrl)
  {
    QTV_Free(m_pByteRangeUrl);
    m_pByteRangeUrl = NULL;
  }
}

/**
 * Sets the iSegmentNotifier.
 */
void SegmentDownloader::SetSegNotifier(iSegmentNotifier *pSegNotifier)
{
  m_pSegmentNotifier = pSegNotifier;
}

/**
 * Check if the DOWNLOAD_TOO_SLOW needs to be notified for this
 * data unit.
 */
bool SegmentDownloader::CheckDownloadTooSlow()
{
  QTV_NULL_PTR_CHECK(m_pSessionInfo, false);

  bool rslt = false;
  static const int BUFFERED_DURATION_CHECK_MS = 100; // ms
  static const int NUM_ITERATIONS = (SCHEDULER_SLEEP_TIME > 0
                                     ? BUFFERED_DURATION_CHECK_MS / SCHEDULER_SLEEP_TIME
                                     : 50);

  int64 contentLen = GetContentLength();
  uint64 numBytesRecd = GetNumBytesReceived();

  // Perform timeout checks only if
  // (i) content length is known, and,
  // (ii) download is not completed yet.
  // (iii) at least one byte has been received which means this is the active
  //       unit. It is possible that the data unit with zero bytes is an
  //       active unit and timeout can elapse with zero bytes donwloaded. But
  //       that means the effective b/w is zero anyways. This approach can be changed
  //       to query directly into the http-stack to find out if this data unit is
  //       the currently active request.
  if (contentLen > 0 && numBytesRecd < (uint64)contentLen && numBytesRecd > 0 )
  {
    if (false == m_pSessionInfo->IsDataUnitCancellationDisabled()  &&
        false == m_bIsDownloadTooSlowProcessed)
    {
      if (m_BufferedDurationCheckCount % NUM_ITERATIONS == 0)
      {
        // Check for timeout only when at least one byte has been read. This means this
        // is not a pipelined unit. But it is possible this is not a pipelined unit and
        // numBytesRecd is zero. In that case, it is still ok not to send a timeout because
        // there is no available bandwidth anyways.
        if (IsTimeOut())
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Fragment with data unit key %llu detected download too slow",
                        GetKey());
          rslt = true;

          // Disable furthur notifications for this data unit.
          m_bIsDownloadTooSlowProcessed = true;
        }
      }

      ++m_BufferedDurationCheckCount;
    }
  }

  return rslt;
}

/**
 * Logic to check for download too slow
 */
bool SegmentDownloader::IsTimeOut()
{
  bool rslt = false;

  if (m_pSegmentNotifier)
  {
    uint32 elapsedTime = m_pBandwidthEstimator->GetCummulativeTimeTaken();
    int durationBuffered = m_pSegmentNotifier->GetBufferedDurationFromNotifier();

    QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "SegmentDownloader: Data unit key %d IsTimeOut. Elapsed time %d, buffdur %d, this %p",
      (int)m_nKey, (int)elapsedTime, durationBuffered, (void*)this);

    // right now partially downloaded segments are not used for buffer
    // occupancy. So, m_Duration / 2 is really half of the earlier segment.
    if (m_nDuration > 0 && elapsedTime >= (uint32)m_nDuration
        && durationBuffered > 0 && durationBuffered < (m_nDuration / 2))
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "SegmentDownloader: Data unit key %d tooslow. Elapsed time %d",
                    (int)m_nKey, (int)elapsedTime);
      rslt = true;
    }
  }
  return rslt;
}

/**
 * Disable or re-enable socket reads on this download unit.
 */
void SegmentDownloader::DisableSocketReads(bool bIsDisabled)
{
  m_IsDownloadDisabled = bIsDisabled;
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "DisableSocketReads: TooSlow du %d, flag set as %d", (int)m_nKey, m_IsDownloadDisabled);
}

/**
 * Increase the ref count of the shared timer
 */
void SegmentDownloader::StartSharedTimer()
{
    if(m_pSharedBandwidthEstimator)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "SegmentDownloader::StartSharedTimer");
  m_pSharedBandwidthEstimator->ResponseHeaderReceived();
}
}

/**
 * Decrease the ref count of the shared timer.
 */
void SegmentDownloader::StopSharedTimer()
{
  if(m_pSharedBandwidthEstimator)
  {
  if (m_bIsSharedTimerRefCountIncreased)
  {
    m_bIsSharedTimerRefCountIncreased = false;
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "SegmentDownloader::StopSharedTimer");
    m_pSharedBandwidthEstimator->ResponseDataReceived();
  }
}
}

/**
 * Decrease the refcnt of the shared timer by the amount it was
 * increased.
 */
void SegmentDownloader::ReleaseSharedTimerRefCnt()
{
  StopSharedTimer();
  m_pSharedBandwidthEstimator = NULL;
}

/** @brief Initialize downloader.
  *
  * @param[in] pSessionInfo - Reference to session info
  * @return
  * TRUE - Initialization successful
  * FALSE - Otherwise
  */
bool SegmentDownloader::Init
(
 HTTPSessionInfo* pSessionInfo,
 HTTPStackInterface* pHTTPStack
)
{
  //Initialize locals
  bool bResult = false;

  if (pSessionInfo && pHTTPStack)
  {
    m_pSessionInfo = pSessionInfo;
    m_pHTTPStack = pHTTPStack;

    bResult = (MM_CriticalSection_Create(&m_pDataLock) == 0);
  }

  //Start with IDLE state if initialization succeeded
  if (bResult)
  {
    SetStateHandler(&m_cIdleStateHandler);
  }

  return bResult;
}

/** @brief Close downloader (valid in all states).
  *
  * @param[in] bKeepAlive - Connection keep alive
  * @return
  * TRUE - Close successful
  * FALSE - Otherwise
  */
bool SegmentDownloader::DownloaderBaseStateHandler::Close
(
)
{
  HTTPStackInterface* pHTTPStack = m_pDownloader->m_pHTTPStack;

  if (pHTTPStack)
  {
    (void)pHTTPStack->DeleteRequest(m_pDownloader->m_nHTTPRequestID);
  }

  //Reset
  m_pDownloader->m_pBandwidthEstimator->StopTimer();
  m_pDownloader->Reset();

  return true;
}

/** @brief Start downloader (valid in IDLE state).
  *
  * @param[in] pURL - Reference to URL
  * @param[in] nStartOffset - Start offset
  * @param[in] nEndOffset - End offset
  * @return
  * TRUE - Start successful
  * FALSE - Otherwise
  */
bool SegmentDownloader::DownloaderIdleStateHandler::Open
(
 const char* pURL,
 const char* pByteRangeURL,
 const int64 nStartOffset,
 const int64 nEndOffset,
 const int64 nDurationMs,
 const uint32 nHTTPRequestID
)
{
  bool bOk = false;
  URL url = pURL ? pURL : pByteRangeURL;
  char* pLaunchURL = NULL;
  m_pDownloader->m_nNumBytesReceived = 0;
  m_pDownloader->m_nContentLength = -1;
  m_pDownloader->m_nStartOffset = STD_MAX(nStartOffset, 0);
  m_pDownloader->m_nEndOffset = nEndOffset;
  m_pDownloader->m_nDuration = nDurationMs;

  //If byterange URL exists, cache it. It is used in case of requests for http1.0.
  if(pByteRangeURL)
  {
    m_pDownloader->SetByteRangeURL(pByteRangeURL);
  }

  //Prepare launch URL
  if (HTTPCommon::ParseURL(url, HTTP_DEFAULT_PORT, pLaunchURL) && pLaunchURL)
  {
    HTTPSessionInfo* pSessionInfo = m_pDownloader->m_pSessionInfo;
    HTTPStackInterface* pHTTPStack = m_pDownloader->m_pHTTPStack;
    HTTPMethodType methodType = HTTP_GET;

    if (pSessionInfo && pHTTPStack)
    {
      HTTPReturnCode result = HTTP_SUCCESS;
      if(nHTTPRequestID == UNDEFINED_VAL_REQUESTID)
      {
          result = pHTTPStack->CreateRequest(m_pDownloader->m_nHTTPRequestID);
      }
      else
      {
        m_pDownloader->m_nHTTPRequestID = nHTTPRequestID;
      }

      if (result == HTTP_SUCCESS)
      {
        //Add HTTP headers
        char hostName[HTTP_MAX_HOSTNAME_LEN] = {0};
        if (url.GetHost(hostName, sizeof(hostName)) == URL::URL_OK)
        {
          (void)pHTTPStack->SetHeader(m_pDownloader->m_nHTTPRequestID, "Host", (int)std_strlen("Host"),
                                      hostName, (int)std_strlen(hostName));
        }

        /* Set the 'Range' header field only if it is original URL.
         * For ByteRangeURL unset the 'Range' header field since the URL contains the range info.
         */
        char range[HTTP_MAX_RANGE_LEN] = {0};
        if(pURL)
        {
          if (nEndOffset >= 0)
          {
            (void)std_strlprintf(range, sizeof(range), "bytes=%llu-%lld",
              m_pDownloader->m_nStartOffset, nEndOffset);
            m_pDownloader->m_nContentLength = nEndOffset - m_pDownloader->m_nStartOffset + 1;
          }
          else
          {
            (void)std_strlprintf(range, sizeof(range), "bytes=%llu-",m_pDownloader->m_nStartOffset);
          }
          (void)pHTTPStack->SetHeader(m_pDownloader->m_nHTTPRequestID, "Range", (int)std_strlen("Range"),
            range, (int)std_strlen(range));
        }
        else
        {
          (void)pHTTPStack->UnsetHeader(m_pDownloader->m_nHTTPRequestID, "Range", (int)std_strlen("Range"));
        }

        const char* pUserAgent = pSessionInfo->GetUserAgent();
        if (pUserAgent)
        {
          (void)pHTTPStack->SetHeader(m_pDownloader->m_nHTTPRequestID, "User-Agent",
                                      (int)std_strlen("User-Agent"),
                                      pUserAgent, (int)std_strlen(pUserAgent));
        }

        //Add any OEM headers
        HTTPCommon::AddIPStreamProtocolHeaders(*pSessionInfo, *pHTTPStack, methodType, m_pDownloader->m_nHTTPRequestID);

        //Send HTTP GET request
        if(pURL)
        {
          QTV_MSG_SPRINTF_PRIO_2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                "SegmentDownloader::IDLE: Posting GET %s url is %s", range,pLaunchURL);
        }
        else
        {
          QTV_MSG_SPRINTF_PRIO_1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                "SegmentDownloader::IDLE: Posting GET ByteRangeUrl is %s", pLaunchURL);
        }
        result = pHTTPStack->SendRequest(m_pDownloader->m_nHTTPRequestID, methodType,
                                                        pLaunchURL,
                                                        (int)std_strlen(pLaunchURL));
        if (result == HTTP_SUCCESS)
        {
          //HTTP GET request successfully composed and posted to the HTTP stack,
          //move to WAITFORDATA
          bOk = true;

          if(m_pDownloader->m_pSharedBandwidthEstimator)
          {
          m_pDownloader->m_pSharedBandwidthEstimator->RequestSent();

          if (!m_pDownloader->m_bIsSharedTimerRefCountIncreased)
          {
            m_pDownloader->m_bIsSharedTimerRefCountIncreased = true;
          }
          else
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "Unexpected error. Will not start timer for bw estimates");
          }
          }

          m_pDownloader->SetStateHandler(&m_pDownloader->m_cWaitForDataStateHandler);
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                        "SegmentDownloader::IDLE: HTTP GET request successfully "
                        "composed and posted to HTTP stack - moving to WAITFORDATA" );
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "SegmentDownloader::IDLE: Posting HTTP GET failed %d", result );
        }
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error Creating Request" );
      }
    }

    QTV_Free(pLaunchURL);
    pLaunchURL = NULL;
  }// if (HTTPCommon::ParseURL())

  return bOk;
}

/** @brief Read data (in WAIT_FORDATA state).
  *
  * @param[in] pBuffer - Buffer to read data into
  * @param[in] nSizeToRead - Buffer size
  * @param[out] nSizeRead - Num bytes read
  * @return HTTPReturnCode
  */
HTTPCommon::HTTPDownloadStatus SegmentDownloader::DownloaderWaitForDataStateHandler::Read
(
 byte* /* pBuffer */,
 const int64 /* nSizeToRead */,
 int64& nSizeRead
)
{
  HTTPCommon::HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPStackInterface* pHTTPStack = m_pDownloader->m_pHTTPStack;
  nSizeRead = 0;

  if (pHTTPStack)
  {
    //Check if GET response is received
    {
      HTTPReturnCode rslt = pHTTPStack->IsResponseReceived(m_pDownloader->m_nHTTPRequestID);

      switch (rslt)
      {
      case HTTP_SUCCESS:
        result = HTTPCommon::HTTPDL_SUCCESS;
        break;
      case HTTP_WAIT:
        result = HTTPCommon::HTTPDL_WAITING;
        break;
      case HTTP_NOMOREDATA:
        result = HTTPCommon::HTTPDL_DATA_END;
        break;
      case HTTP_INSUFFBUFFER:
        result = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
        break;
      default:
        result = HTTPCommon::HTTPDL_ERROR_ABORT;
        break;
      }
    }

    if (HTTPCommon::HTTPDL_SUCCESS == result)
    {
      m_pDownloader->StartSharedTimer();

      uint32 nServerCode = 0;
      pHTTPStack->GetResponseCode(m_pDownloader->m_nHTTPRequestID, nServerCode);
      if (!(nServerCode >= 200 && nServerCode <= 206))
      {
        // other codes eg 401 not handled
        result = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
      }
    }

    if (result == HTTPCommon::HTTPDL_SUCCESS)
    {
      int64 nContentLength = m_pDownloader->GetContentLength();
      int64 nChunkLength = -1;
      (void)pHTTPStack->GetContentLength(m_pDownloader->m_nHTTPRequestID, &nChunkLength);

      //Validate content length
      if (nChunkLength > 0 && nContentLength > 0 && nChunkLength > nContentLength)
      {
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "SegmentDownloader::WAITFORDATA: nContentLength %lld does not "
                       "match with nChunkLength %d from GET response",
                       nContentLength, (int)nChunkLength );
        result = HTTPCommon::HTTPDL_ERROR_ABORT;

        //If byteRange URL exists, resend request using the it
        if(m_pDownloader->GetByteRangeURL() && m_pDownloader->IsByteRangeUrlRequested() == false)
        {
          uint64 nKey = m_pDownloader->GetKey();
          uint64 nStartOffset = m_pDownloader->GetStartOffset();
          uint64 nEndOffset = m_pDownloader->GetEndOffset();
          int64 nDuration = m_pDownloader->GetDuration();
          uint32 nRequestID = m_pDownloader->m_nHTTPRequestID;

          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "SegmentDownloader::WAITFORDATA: Retrying GET request with ByteRangeURL %s", m_pDownloader->GetByteRangeURL());

          //Reset
          m_pDownloader->m_pBandwidthEstimator->StopTimer();
          m_pDownloader->Reset();

          if(m_pDownloader->Start(NULL, m_pDownloader->GetByteRangeURL(), nKey,
                                  nStartOffset, nEndOffset, nDuration, nRequestID))
          {
            m_pDownloader->SetByteRangeUrlRequested(true);
            result = HTTPCommon::HTTPDL_WAITING;
          }
        }
        else
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "SegmentDownloader::WAITFORDATA: Nobyterangeurl ");
        }
      }

      //Move to DATAREADY state where data is read from stack
      if (result == HTTPCommon::HTTPDL_SUCCESS)
      {
        result = HTTPCommon::HTTPDL_WAITING;
        m_pDownloader->m_bIsDownloadTooSlowProcessed = false;
        m_pDownloader->m_pBandwidthEstimator->StartTimer();
        m_pDownloader->SetContentLength(nChunkLength);
        m_pDownloader->SetStateHandler(&m_pDownloader->m_cDataReadyStateHandler);
        //Indicates back to DASHMediaSegmentHandler that ByteRangeURL has been read

        if(m_pDownloader->IsByteRangeUrlRequested() == true)
        {
          m_pDownloader->SetByteRangeUrlInUse(true);
          m_pDownloader->SetByteRangeUrlRequested(false);
        }
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "SegmentDownloader::WAITFORDATA: HTTP GET response received - "
                      "moving to DATAREADY" );
      }
    }
    else if (result == HTTPCommon::HTTPDL_WAITING)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "SegmentDownloader::WAITFORDATA: Waiting for GET response" );
    }

    if (result != HTTPCommon::HTTPDL_WAITING && result != HTTPCommon::HTTPDL_SUCCESS)
    {
      //Notify error, caller to cleanup!
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "SegmentDownloader::WAITFORDATA: GET failed %d", result );
    }
  }

  return result;
}

/** @brief Read data (in DATAREADY state).
  *
  * @param[in] pBuffer - Buffer to read data into
  * @param[in] nSizeToRead - Buffer size
  * @param[out] nSizeRead - Num bytes read
  * @return HTTPReturnCode
  */
HTTPCommon::HTTPDownloadStatus SegmentDownloader::DownloaderDataReadyStateHandler::Read
(
 byte* pBuffer,
 const int64 nSizeToRead,
 int64 & nSizeRead
)
{
  HTTPCommon::HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPStackInterface* pHTTPStack = m_pDownloader->m_pHTTPStack;
  nSizeRead = 0;

  if (pHTTPStack)
  {
    //Read data from stack
    if (false == m_pDownloader->m_IsDownloadDisabled)
    {
      HTTPReturnCode rslt = pHTTPStack->GetData(m_pDownloader->m_nHTTPRequestID, (char*)pBuffer,
                                   (size_t)nSizeToRead, (size_t *)&nSizeRead);

      switch (rslt)
      {
      case HTTP_SUCCESS:
        result = HTTPCommon::HTTPDL_SUCCESS;
        break;
      case HTTP_WAIT:
        result = HTTPCommon::HTTPDL_WAITING;
        break;
      case HTTP_NOMOREDATA:
        result = HTTPCommon::HTTPDL_DATA_END;
        break;
      case HTTP_INSUFFBUFFER:
        result = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
        break;
      default:
        result = HTTPCommon::HTTPDL_ERROR_ABORT;
        break;
      }
    }
    else
    {
      result = HTTPCommon::HTTPDL_WAITING;
    }

    if (result == HTTPCommon::HTTPDL_SUCCESS)
    {
      m_pDownloader->UpdateNumBytesReceived(nSizeRead);
    }
    else if (result == HTTPCommon::HTTPDL_ERROR_ABORT)
    {
      //Notify error, caller to cleanup!
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "SegmentDownloader::DATAREADY: Read failed %d, numBytesRead %llu",
                     result, m_pDownloader->GetNumBytesReceived() );
    }
  }

  return result;
}

} // namespace video
