/************************************************************************* */
/**
 * HTTPResolver.cpp
 * @brief implementation of HTTPResolver.
 * HTTPResolver class primarily based on the server responses indicating
 * what content type it is. Its role is to get the content type from the server.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPResolver.cpp#17 $
$DateTime: 2013/07/28 21:38:43 $
$Change: 4175800 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPResolver.cpp
** ======================================================================= */
#include "HTTPResolver.h"

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
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

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief HTTPResolver Constructor.
  *
  * @param[in] sessionInfo - Reference to HTTP session information
  * @param[in] HTTPStack - Reference to HTTP stack
  */
HTTPResolver::HTTPResolver
(
HTTPSessionInfo& sessionInfo,
HTTPStackInterface& HTTPStack,
const int32 nMaxRequestSize
) : m_pStateHandler(NULL),
    m_pHTTPStack(HTTPStack),
    m_sessionInfo(sessionInfo),
    m_pLaunchURL(NULL),
    m_nMaxRequestSize(nMaxRequestSize),
    m_nRequestID(0)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPResolver::HTTPResolver" );

  std_memset((void*)m_contentType, 0x0, sizeof(m_contentType));

 //State Initialization
 SetStateHandler(&m_IdleStateHandler);
}

/** @brief HTTPResolver Destructor.
  *
  */
HTTPResolver::~HTTPResolver()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPResolver::~HTTPResolver" );

  if (m_pLaunchURL)
  {
    QTV_Free(m_pLaunchURL);
    m_pLaunchURL = NULL;
  }
}


uint32 HTTPResolver::GetResolverRequestID() const
{
  return m_nRequestID;
}

/** @brief Find out the HTTP flavor .
  *
  * @return
  * HTTPDL_WAITING - In progress - check back later
  * HTTPDL_SUCCESS - Successfully resolved (copies content length and
  *                  type into session info)
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus HTTPResolver::ResolveHTTPFlavor()
{
  return CurrentStateHandler(this);
}

/** @brief Process based on current state.
  *
  * @param[in] pHTTPResolver - Reference to HTTPResolver
  * @return
  * HTTPDL_WAITING - In progress - check back later
  * HTTPDL_SUCCESS - Successfully resolved
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus HTTPResolver::CurrentStateHandler
(
 HTTPResolver* pHTTPResolver
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  BaseStateHandler* pStateHandler = GetStateHandler();
  if (pStateHandler)
  {
    status = pStateHandler->Execute(pHTTPResolver);
  }

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}

/** @brief IDLE state handler.
  *
  * @param[in] pHttpResolver - Reference to HTTPResolver
  * @return
  * HTTPDL_WAITING - Successfully posted GET request to the HTTP stack
  * HTTPDL_ERROR_ABORT - Error
  */
HTTPDownloadStatus HTTPResolver::IdleStateHandler::Execute
(
 HTTPResolver* pHttpResolver
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPResolver::IdleStateHandler::Execute()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPResolver* pSelf = static_cast<HTTPResolver *> (pHttpResolver);

  //Validity check
  if (pSelf)
  {
    HTTPSessionInfo& sessionInfo = pSelf->m_sessionInfo;
    URL url = sessionInfo.GetURL();
    char* pLaunchURL = pSelf->m_pLaunchURL;
    HTTPMethodType methodType = HTTP_GET;

    //Prepare launch URL
    if (HTTPCommon::ParseURL(url, HTTP_DEFAULT_PORT, pLaunchURL) && pLaunchURL)
    {
      pSelf->m_pLaunchURL = pLaunchURL;
      HTTPStackInterface& HTTPStack = pSelf->m_pHTTPStack;

      if(HTTPStack.CreateRequest(pSelf->m_nRequestID) == HTTP_SUCCESS )
      {
        //Add HTTP headers
        char hostName[HTTP_MAX_HOSTNAME_LEN] = {0};
        if (url.GetHost(hostName, sizeof(hostName)) == URL::URL_OK)
        {
            (void)HTTPStack.SetHeader(pSelf->m_nRequestID, "Host", (int)std_strlen("Host"),
                                    hostName, (int)std_strlen(hostName));
        }

        char range[HTTP_MAX_RANGE_LEN] = {0};
        if (pSelf->m_nMaxRequestSize >= 0)
        {
          (void)std_strlprintf(range, sizeof(range), "bytes=%d-%d", 0, (int)pSelf->m_nMaxRequestSize);
        }
        else
        {
        (void)std_strlprintf(range, sizeof(range), "bytes=%d-", 0);
        }

          (void)HTTPStack.SetHeader(pSelf->m_nRequestID, "Range", (int)std_strlen("Range"),
                                  range, (int)std_strlen(range));

        const char* pUserAgent = sessionInfo.GetUserAgent();
        if (pUserAgent)
        {
            (void)HTTPStack.SetHeader(pSelf->m_nRequestID, "User-Agent",
                                    (int)std_strlen("User-Agent"),
                                    pUserAgent, (int)std_strlen(pUserAgent));
        }

        //Add any OEM headers
        HTTPCommon::AddIPStreamProtocolHeaders(sessionInfo, HTTPStack, methodType, pSelf->m_nRequestID);

        //Send HTTP GET request
        QTV_MSG_SPRINTF_PRIO_2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                "HTTPResolver::IdleStateHandler: Posting GET %s for %s", range, pLaunchURL );
          HTTPReturnCode result = HTTPStack.SendRequest(pSelf->m_nRequestID, methodType,
                                                      pLaunchURL,
                                                      (int)std_strlen(pLaunchURL));
        if (result == HTTP_SUCCESS)
        {
          //HTTP GET request successfully composed and posted to the HTTP stack,
          //move to WAIT_FOR_DATA
          status = HTTPCommon::HTTPDL_WAITING;
          pSelf->SetStateHandler(&pSelf->m_WaitForDataStateHandler);
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                        "HTTPResolver::IdleStateHandler: HTTP GET request successfully "
                        "composed and posted to HTTP stack - moving to WAIT_FOR_DATA" );
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "HTTPResolver::IdleStateHandler: Posting HTTP GET failed %d",
                         result );
        }
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error Creating Request");
      }
    }// if (HTTPCommon::ParseURL())
  }// if (pSelf)

  return status;
}

/** @brief Waiting for data state handler.
  *
  * @param[in] pHttpResolver - Reference to HTTPResolver
  * @return
  * HTTPDL_WAITING - Successfully posted GET request to the HTTP stack
  * HTTPDL_SUCCESS - Received GET response
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus HTTPResolver::WaitForDataStateHandler::Execute
(
 HTTPResolver* pHttpResolver
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPResolver::WaitForDataStateHandler::Execute()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPResolver* pSelf = static_cast<HTTPResolver *> (pHttpResolver);

  //Validity check
  if (pSelf)
  {
    HTTPStackInterface& HTTPStack = pSelf->m_pHTTPStack;

    //Check if GET response is received
    HTTPReturnCode result = HTTPStack.IsResponseReceived(pSelf->m_nRequestID);

    if (HTTP_SUCCESS == result)
    {
      uint32 nVal = 0;
      HTTPStack.GetResponseCode(pSelf->m_nRequestID, nVal);
      if (!(nVal >= 200 && nVal <= 206))
      {
        result = HTTP_FAILURE;
      }
    }

    if (result == HTTP_WAIT || result == HTTP_SUCCESS)
    {
      //Could be WAIT or SUCCESS
      if (result == HTTP_SUCCESS)
      {
        status = HTTPCommon::HTTPDL_SUCCESS;

        int64 contentLength = -1;
        char* pContentType = NULL;
        (void)HTTPCommon::GetContentLengthAndType(pSelf->m_nRequestID,
            HTTPStack, contentLength, pContentType);
        if (pContentType)
        {
          pSelf->SetContentType(pContentType);
          QTV_Free(pContentType);
          pSelf->SetStateHandler(&pSelf->m_IdleStateHandler);
        }
      }
      else
      {
        status = HTTPCommon::HTTPDL_WAITING;
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                      "HTTPResolver::WaitForDataStateHandler: Waiting for GET response" );
      }
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HTTPResolver::WaitForDataStateHandler: GET failed %d...closing connection",
                     result );
      pSelf->SetStateHandler(&pSelf->m_IdleStateHandler);
    }//if (result == HTTP_WAIT || result == HTTP_SUCCESS)
  }// if (pSelf)

  return status;
}

}/* namespace video */
