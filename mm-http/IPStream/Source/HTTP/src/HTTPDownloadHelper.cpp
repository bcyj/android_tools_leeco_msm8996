/************************************************************************* */
/**
 * HTTPDownloadHelper.cpp
 * @brief implementation of HTTPDownloadHelper.
 *  HTTPDownloadHelper is a helper base class to HTTPDownloader. It supports
 *  certain common methods (for Progressive Download and Fast Track) to
 *  interact with data manager, parse the URL, get download progress etc.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPDownloadHelper.cpp#11 $
$DateTime: 2013/06/12 21:28:48 $
$Change: 3913306 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDownloadHelper.cpp
** ======================================================================= */
#include "HTTPDownloadHelper.h"
#include "HTTPDataManager.h"
#include "HTTPSessionInfo.h"

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
/** @brief HTTPDownloadHelper Constructor.
  *
  * @param[in] sessionInfo - Reference to HTTP session information
  * @param[in] dataManager - Reference to HTTP data manager
  * @param[in] HTTPStack - Reference to HTTP stack
  */
HTTPDownloadHelper::HTTPDownloadHelper
(
 HTTPSessionInfo& sessionInfo,
 HTTPStackInterface& HTTPStack
 ) : m_sessionInfo(sessionInfo),
     m_HTTPStack(HTTPStack),
     m_pLaunchURL(NULL),
     m_pStateHandler(NULL),
     m_pDownloadHelperDataLock(NULL),
     m_nTotalBytesReceived(0),
     m_bEndOfFile(false)
{
  (void)MM_CriticalSection_Create(&m_pDownloadHelperDataLock);
}

/** @brief HTTPDownloadHelper Destructor.
  *
  */
HTTPDownloadHelper::~HTTPDownloadHelper()
{
  if (m_pLaunchURL)
  {
    QTV_Free(m_pLaunchURL);
    m_pLaunchURL = NULL;
  }
  if (m_pDownloadHelperDataLock)
  {
    (void)MM_CriticalSection_Release(m_pDownloadHelperDataLock);
    m_pDownloadHelperDataLock = NULL;
  }
}

/** @brief Process based on current state.
  *
  * @param[in] pDownloadHelper - Reference to HTTPDownloadHelper
  * @return
  * HTTPDL_WAITING - In progress - check back later
  * HTTPDL_SUCCESS - Successfully processed
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus HTTPDownloadHelper::CurrentStateHandler
(
 HTTPDownloadHelper* pDownloadHelper
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

/** @brief Parses the input URL and fills in the default port if port already
  * not present.
  *
  * @param[in] pDefaultPort - Default HTTP port to use if port not present in URL
  * @param[out] pLaunchURL - Launch URL used to connect to server
  * @return
  * TRUE - Launch URL ready
  * FALSE - Otherwise
  */
bool HTTPDownloadHelper::ParseURL
(
 const char* pDefaultPort,
 char*& pLaunchURL
)
{
  bool bOk = false;

  URL url = m_sessionInfo.GetURL();
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
      if (pLaunchURL)
      {
        bOk = true;
        const char* urlBuffer = url.GetUrlBuffer();
        if(urlBuffer)
        {
          (void)std_strlcpy(pLaunchURL, urlBuffer, launchURLLen);
        }
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

  return bOk;
}

/** @brief Closes HTTP connection.
  *
  * @return
  * HTTPDL_WAITING - In progress - check back later
  * HTTPDL_SUCCESS - Successfully torn down
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus HTTPDownloadHelper::IsCloseComplete()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  //Check if TCP connection is brought down
  HTTPReturnCode result = m_HTTPStack.CloseConnection();
  status = (result == HTTP_FAILURE) ? status :
           (result == HTTP_SUCCESS ? HTTPCommon::HTTPDL_SUCCESS
                                   : HTTPCommon::HTTPDL_WAITING);

  return status;
}

}/* namespace video */
