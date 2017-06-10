
/************************************************************************* */
/**
* PlaylistDownloadHelper.cpp
* @brief implementation of PlaylistDownloadHelper.
*  PlaylistDownloadHelper is a helper class to PlaylistDownloader and helps in
*  connecting to the HTTP server, getting the initial header and downloading
*  the actual content for Playlist Download.
*
* COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
* All rights reserved. Qualcomm Technologies proprietary and confidential.
*
************************************************************************* */
/* =======================================================================
Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/PlaylistDownloadHelper.cpp#26 $
$DateTime: 2013/08/14 18:14:12 $
$Change: 4277631 $

========================================================================== */
/* =======================================================================
**               Include files for PlaylistDownloadHelper.cpp
** ======================================================================= */
#include "PlaylistDownloadHelper.h"
#include <SourceMemDebug.h>

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
/** @brief PlaylistDownloadHelper Constructor.
 *
 */
PlaylistDownloadHelper::PlaylistDownloadHelper(HTTPSessionInfo& sessionInfo,
                                              HTTPStatusHandlerInterface* pStackNotificationHandler,
                                              uint32 nRequestIDfromResolver,
                                              HTTPStackInterface* HTTPStack)
  :m_pLaunchURL(NULL),
  m_RepresentationBuffer(NULL),
  m_nRepresentationBytesRead(0),
  m_nRepresentationLen(0),
  m_url(NULL),
  m_nNumUrls(0),
  m_ncurrentUrl(0),
  m_pHTTPStack(HTTPStack),
  m_sessionInfo(sessionInfo),
  m_pStateHandler(NULL),
  bIsStackLocallyAllocated(true),
  m_nRequestID(nRequestIDfromResolver)
{
  //Initialization
  //Create the HTTP stack
  bool bOk=false;
  int result = -1;

  if (!m_pHTTPStack)
  {
    result = HTTPStackInterface::CreateInstance(&m_pHTTPStack,
                                              pStackNotificationHandler,
                                              sessionInfo.GetCookieStore());
    bOk = (result == 0 && m_pHTTPStack);
    if (bOk)
    {
      //Set socket mode to NON_BLOCKING
      (void)m_pHTTPStack->SetSocketMode(HTTP_NON_BLOCKING);
    }
    if (m_pHTTPStack)
    {
      bOk=HTTPCommon::ConfigureHTTPStack(m_sessionInfo,*m_pHTTPStack);
    }
    SetStateHandler(&m_IdleStateHandler);
  }
  else
  {
    bIsStackLocallyAllocated = false;
    SetStateHandler(&m_WaitingForPlaylistStateHandler);
  }
}

/** @brief PlaylistDownloadHelper Destructor.
*
*/
PlaylistDownloadHelper::~PlaylistDownloadHelper()
{
  if (m_pHTTPStack && bIsStackLocallyAllocated)
  {
    QTV_Delete(m_pHTTPStack);
    m_pHTTPStack = NULL;
  }
  if(m_RepresentationBuffer)
  {
    QTV_Free(m_RepresentationBuffer);
    m_RepresentationBuffer = NULL;
  }
  if (m_pLaunchURL)
  {
    QTV_Free(m_pLaunchURL);
    m_pLaunchURL = NULL;
  }
}

/** @brief DownloadAndUpdatePlaylist.
*
* @return
* HTTPDL_WAITING - Download In progress - check back later
* HTTPDL_SUCCESS - Download Complete
* HTTPDL_ERROR_ABORT - Otherwise
*/
HTTPDownloadStatus PlaylistDownloadHelper::DownloadAndUpdatePlaylist()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "PlaylistDownloadHelper::DownloadAndUpdatePlaylist()" );
  return CurrentStateHandler(this);
}

void PlaylistDownloadHelper::DownloadDone()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "PlaylistDownloadHelper::DownloadDone()" );
  m_pHTTPStack->DeleteRequest(m_nRequestID);
  m_nRequestID = 0;
  SetStateHandler(&(m_IdleStateHandler));
  return;
}

/** @brief Close HTTP connection.
*
* @return
* HTTPDL_WAITING - In progress - check back later
* HTTPDL_SUCCESS - Successfully closed connection
* HTTPDL_ERROR_ABORT - Otherwise
*/
HTTPDownloadStatus PlaylistDownloadHelper::CloseConnection()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "PlaylistDownloadHelper::CloseHTTPConnection()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  status = IsCloseComplete();
  if (HTTPSUCCEEDED(status))
  {
    SetStateHandler(&m_IdleStateHandler);
  }
  return status;
}
/* @brief Sets the url for download
 * @param - url
 */
void PlaylistDownloadHelper::SetURL(char *url)
{
  m_url=url;
}
/** @brief Parses the input URL and fills in the default port if port already
  * not present.
  *
  * @param[in] pDefaultPort - Default HTTP port to use if port not present in URL
  * @param[out] pLaunchURL - Launch URL used to connect to server
  * @return
  * TRUE - Launch URL ready
  * FALSE - Otherwise
  */
bool PlaylistDownloadHelper::ParseURL
(
 const char* pDefaultPort,
 char*& pLaunchURL
 )
{
  bool bOk = false;

  URL url(GetURL());
  size_t launchURLLen = url.GetUrlLength() + 1;
  uint32 port = 0;

  if (pLaunchURL)
  {
    QTV_Free(pLaunchURL);
    pLaunchURL = NULL;
  }

  if (url.GetPort(&port) == URL::URL_OK)
  {
    if (port)
    {
      pLaunchURL = (char*)QTV_Malloc(launchURLLen);
      if (pLaunchURL && url.GetUrlBuffer())
      {
        bOk = true;
        (void)std_strlcpy(pLaunchURL, url.GetUrlBuffer(), launchURLLen);
      }
    }
    else if (pDefaultPort)
    {
      char hostName[HTTP_MAX_HOSTNAME_LEN] = {0};

      //Extra space for ":9300"
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

/** @brief Process based on current state.
  *
  * @param[in] pDownloadHelper - Reference to PlaylistDownloadHelper
  * @return
  * HTTPDL_WAITING - In progress - check back later
  * HTTPDL_SUCCESS - Successfully processed
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus PlaylistDownloadHelper::CurrentStateHandler
(
 PlaylistDownloadHelper* pDownloadHelper
 )
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  BaseStateHandler* pStateHandler = GetStateHandler();
  if (pStateHandler)
  {
    status = pStateHandler->Execute(pDownloadHelper);
  }

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}

/** @brief Closes HTTP connection.
  *
  * @return
  * HTTPDL_WAITING - In progress - check back later
  * HTTPDL_SUCCESS - Successfully torn down
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus PlaylistDownloadHelper::IsCloseComplete()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  //Check if TCP connection is brought down
  HTTPReturnCode result = m_pHTTPStack->CloseConnection();
  status = (result == HTTP_FAILURE) ? status :
    (result == HTTP_SUCCESS ? HTTPCommon::HTTPDL_SUCCESS
    : HTTPCommon::HTTPDL_WAITING);

  return status;
}


/** @brief IDLE state handler.
*
* @param[in] pDownloadHelper - Reference to PlaylistDownloadHelper
* @return
* HTTPDL_WAITING - Successfully initiated connection and GET request posted
*                  to the HTTP stack
* HTTPDL_ERROR_ABORT - Otherwise
*/
HTTPDownloadStatus PlaylistDownloadHelper::IdleStateHandler::Execute
(
 PlaylistDownloadHelper* pDownloadHelper
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "PlaylistDownloadHelper::IdleStateHandler::Execute()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  PlaylistDownloadHelper* pSelf = pDownloadHelper;

  //Validity check
  if (pSelf)
  {
    char* pLaunchURL = pSelf->m_pLaunchURL;

    //Fill in default port if port already not present in URL
    if (pSelf->ParseURL(HTTP_DEFAULT_PORT, pLaunchURL) && pLaunchURL)
    {
      HTTPMethodType methodType = HTTP_GET;
      status = HTTPCommon::HTTPDL_SUCCESS;
      pSelf->m_pLaunchURL = pLaunchURL;
      HTTPStackInterface& HTTPStack = *(pSelf->m_pHTTPStack);

      if(HTTPStack.CreateRequest(pSelf->m_nRequestID) == HTTP_SUCCESS)
      {
        URL url = pSelf->GetURL();
        char hostName[HTTP_MAX_HOSTNAME_LEN] = {0};
        if (url.GetHost(hostName, sizeof(hostName)) == URL::URL_OK)
        {
            (void)HTTPStack.SetHeader(pSelf->m_nRequestID, "Host", (int)std_strlen("Host"),
            hostName, (int)std_strlen(hostName));
        }
        const char* pUserAgent = (pSelf->m_sessionInfo).GetUserAgent();
        if (pUserAgent)
        {
          (void)HTTPStack.SetHeader(pSelf->m_nRequestID, "User-Agent",
          (int)std_strlen("User-Agent"),
                                     pUserAgent, (int)std_strlen(pUserAgent));
        }
        HTTPCommon::AddIPStreamProtocolHeaders(pSelf->m_sessionInfo,HTTPStack,methodType, pSelf->m_nRequestID);
        //Send GET to get the playlist

          HTTPReturnCode result = HTTPStack.SendRequest(pSelf->m_nRequestID, methodType,
          pLaunchURL,
          (int)std_strlen(pLaunchURL));
        if (result == HTTP_SUCCESS)
        {
          //HTTP GET request successfully composed and posted to the HTTP stack.
          //So change state to WAITING_FOR_PLAYLIST and check back later
          status = HTTPCommon::HTTPDL_WAITING;
          pSelf->SetStateHandler(&pSelf->m_WaitingForPlaylistStateHandler);
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "HTTP GET request to get playlist successfully composed and posted "
            "to HTTP stack - proceeding to WAITING_FOR_PLAYLIST" );
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "Error: HTTP GET request send failed %d",
            result );
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error Creating Request");
      }
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: ParseURL failed" );
    }// if (pSelf->ParseURL(HTTP_DEFAULT_PORT, pLaunchURL) && pLaunchURL)
  }// if (pSelf && pSelf->GetState() == IDLE)

  return status;
}


/** @brief WAITING_FOR_PLAYLIST state handler.
*
* @param[in] pDownloadHelper - Reference to
*       PlaylistDownloadHelper
* @return
* HTTPDL_WAITING - In progress - check back later
* HTTPDL_SUCCESS - GET response received
* HTTPDL_ERROR_ABORT - Otherwise
*/
HTTPDownloadStatus PlaylistDownloadHelper::WaitingForPlaylistStateHandler::Execute
(
 PlaylistDownloadHelper* pDownloadHelper
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "PlaylistDownloadHelper::WaitingForPlaylistStateHandler::Execute()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  PlaylistDownloadHelper* pSelf = pDownloadHelper;

  //Validity check
  if (pSelf)
  {
    HTTPStackInterface& HTTPStack = *(pSelf->m_pHTTPStack);
    //Check if GET response is received
    HTTPReturnCode result = HTTPStack.IsResponseReceived(pSelf->m_nRequestID);
    if (HTTP_SUCCESS == result)
    {
      uint32 nServerCode = 0;
      HTTPStack.GetResponseCode(pSelf->m_nRequestID, nServerCode);
      if (!(nServerCode >= 200 && nServerCode <= 206))
      {
        result = HTTP_FAILURE;
      }
    }

    if (result == HTTP_WAIT || result == HTTP_SUCCESS)
    {
      //Could be WAIT or SUCCESS
      status = HTTPCommon::HTTPDL_WAITING;
      if (result == HTTP_SUCCESS)
      {
        //Get Representation length
        int representationLen = 0;
        (void)HTTPStack.GetContentLength(pSelf->m_nRequestID, (int64*)&representationLen);
        if (representationLen <= 0)
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "Error: Invalid content length using max length for representation" );
          representationLen = MAX_REPRESENTATION_LEN;
        }
          //Playlist response received
          if (pSelf->m_RepresentationBuffer)
          {
            QTV_Free(pSelf->m_RepresentationBuffer);
            pSelf->m_RepresentationBuffer = NULL;
            pSelf->m_nRepresentationLen = 0;
          }
          pSelf->m_nRepresentationLen = representationLen + 1;
          pSelf->m_RepresentationBuffer = (char*)QTV_Malloc(pSelf->m_nRepresentationLen);
        if(pSelf->m_RepresentationBuffer)
        {
          std_memset(pSelf->m_RepresentationBuffer,0x0,pSelf->m_nRepresentationLen);
          pSelf->m_nRepresentationBytesRead=0;
          pSelf->SetStateHandler(&pSelf->m_PlaylistReadyStateHandler);
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
            "PLAYLIST_READY - moving on to reading data" );
        }
        else
        {
          status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
        }

      }// if (result == HTTP_SUCCESS)
      else if( result == HTTP_WAIT  )
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "Waiting for GET response" );
      }
    }// if (result == HTTP_WAIT || result == HTTP_SUCCESS)
    else
    {
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }// if (pSelf && pSelf->GetState() == WAITING_FOR_PLAYLIST)

  return status;
}

/** @brief PLAYLIST_READY state handler.
*
* @param[in] pDownloadHelper - Reference to
*       PlaylistDownloadHelper
* @return
* HTTPDL_WAITING - Data successfully written or no data available to write
*                  and download incomplete
* HTTPDL_SUCCESS - Download complete
* HTTPDL_ERROR_ABORT - Otherwise
*/
HTTPDownloadStatus PlaylistDownloadHelper::PlaylistReadyStateHandler::Execute
(
 PlaylistDownloadHelper* pDownloadHelper
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "PlaylistDownloadHelper::PlaylistReadyStateHandler::Execute()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  PlaylistDownloadHelper* pSelf = pDownloadHelper;

  //Validity check
  if (pSelf)
  {
    char* pRepresentationBuf = pSelf->m_RepresentationBuffer;
    int numBytesToRead = STD_MAX(0, pSelf->m_nRepresentationLen - pSelf->m_nRepresentationBytesRead);
    HTTPStackInterface& HTTPStack = *(pSelf->m_pHTTPStack);
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "numBytesToRead %d, (numBytesReceived %d)",
      numBytesToRead, pSelf->m_nRepresentationBytesRead );

    if (pSelf->m_RepresentationBuffer && numBytesToRead > 0)
    {
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
      //Get a data chunk from HTTP stack
      size_t numBytesRead = 0;
      HTTPReturnCode result =
        HTTPStack.GetData(pSelf->m_nRequestID,
        pRepresentationBuf+pSelf->m_nRepresentationBytesRead,
        numBytesToRead,
        &numBytesRead);
      if (result != HTTP_FAILURE && result != HTTP_BADPARAM)
      {
        //Update total number of bytes dowloaded/written so far
        pSelf->m_nRepresentationBytesRead += (int)numBytesRead;
        status = HTTPCommon::HTTPDL_WAITING;
        if(result == HTTP_NOMOREDATA ||
          pSelf->m_nRepresentationBytesRead >= (pSelf->m_nRepresentationLen -1))
        {
          pSelf->m_nRepresentationLen = pSelf->m_nRepresentationBytesRead;
          status = HTTPCommon::HTTPDL_SUCCESS;
        }
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "Total numBytesReceived %d",  pSelf->m_nRepresentationBytesRead );
      }// result check
      else
      {
        //Data read failure
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: Data read failure %d from HTTP stack", result );
      }
    }// if (HTTPSUCCEEDED(status))
  }// if (pSelf && pSelf->GetState() == DATA_READY)
  return status;
}

}/* namespace video */
