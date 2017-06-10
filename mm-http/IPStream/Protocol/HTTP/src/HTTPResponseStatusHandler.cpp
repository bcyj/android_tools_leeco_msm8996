/************************************************************************* */
/**
 * HTTPResponseStatusHandler.cpp
 * @brief implementation of the HTTPRequest.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPResponseStatusHandler.cpp#25 $
$DateTime: 2012/08/28 03:15:16 $
$Change: 2739927 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPResponseStatusHandler.h"
#include "HTTPStackCommon.h"
#include "HTTPStackHelper.h"
#include "HTTPStackStateObjects.h"
#include "SourceMemDebug.h"
#include "qtv_msg.h"

namespace video
{
/**
 * @brief
 *  Drain the entity body into buffer. Can retry again later if
 *  not fully drained.
 * @param cmHTTPStackHelper
 *
 * @return HTTPReturnCode
 *  HTTP_SUCCESS  entity body drained
 *  HTTP_WAIT     entity boty drain in progress
 *  HTTP_FAILURE  fail.
 */
HTTPReturnCode
HTTPResponseStatusHandler::DrainEntityBody(uint32 requestId, HTTPStackHelper& cmHTTPStackHelper)
{
  HTTPReturnCode rslt = HTTP_FAILURE;

  HTTPStateInfo& cmHTTPStateInfo = cmHTTPStackHelper.m_HTTPStateInfo;
  HTTPResponse& httpResponse = cmHTTPStateInfo.m_HTTPResponse;

  if (httpResponse.GetContentLength() <= 0)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "No entity body to read, returning...");
    rslt = HTTP_SUCCESS;
  }
  else
  {
    // Using a loop of 2 though not strictly needed. In case the entire msg gets
    // read as part of GetData, then we need another GetData to return 0 bytes and
    // return value of HTTP_SUCCESS to infer that the entire entity body got read.
    // But having a loop can avoid needing anothe call of IsResponseReceived to
    // drain data.
    for (int i = 0; i < 2; ++i)
    {
      char *pEntityBodyCurPtr = NULL;
      int remainingSize = 0;

      httpResponse.GetEntityBodyBuffer(pEntityBodyCurPtr,remainingSize);

      if (pEntityBodyCurPtr && remainingSize)
      {
        size_t readLen = 0;

        rslt = cmHTTPStateInfo.GetData(requestId, pEntityBodyCurPtr,remainingSize,readLen);

        if (HTTP_SUCCESS == rslt)
        {
          if (0 == readLen)
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "HTTPResponseStatusHandler::DrainEntityBody Entity body drained");
            break;
          }
          else
          {
            // entity body may not be fully drained.
            httpResponse.CommitEntityBodyBuffer(readLen);
            rslt = HTTP_WAIT;
          }
        }
        else if (HTTP_WAIT == rslt)
        {
          break;
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "DrainEntityBody Failed to GetData rslt %d", rslt);
          break;
        }
      }
      else
      {
        // entity body buffer must be full.
        static char tmpBuf[5000];
        size_t tmpReadLen = 0;
        rslt = cmHTTPStateInfo.GetData(requestId, tmpBuf, sizeof(tmpBuf),tmpReadLen);

        if (HTTP_SUCCESS == rslt && tmpReadLen > 0)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "HTTPResponseStatusHandler::DrainEntityBody Discarded %d bytes of entity",
            tmpReadLen);

          // entity body may not be fully drained.
          rslt = HTTP_WAIT;
        }
        else
        {
          break;
        }
      }
    }
  }

  return rslt;
}

HTTPResponseStatusHandler::HTTPResponseStatusHandler()
{

}

HTTPResponseStatusHandler::~HTTPResponseStatusHandler()
{


}

HTTPResponseStatusHandler_100::HTTPResponseStatusHandler_100()
{

}

HTTPResponseStatusHandler_100::~HTTPResponseStatusHandler_100()
{

}

void HTTPResponseStatusHandler_100::ResetStatusHandler()
{

}

/**
 * @brief
 *  Server has not rejected the request. So, ignnore the
 *  response and reset http response object so it starts
 *  receiving the actual http response afresh.
 *
 * @param cmHTTPStackHelper
 *
 * @return HTTPReturnCode
 *  HTTP_WAIT
 */
HTTPReturnCode
HTTPResponseStatusHandler_100::HandlerResponseStatus(
  uint32 /*requestId*/, int /*statusCode*/, HTTPStackHelper& cmHTTPStackHelper)
{
  HTTPResponse& httpResponse = cmHTTPStackHelper.m_HTTPStateInfo.m_HTTPResponse;
  httpResponse.Reset();

  return HTTP_WAIT;
}

HTTPResponseStatusHandler_200::HTTPResponseStatusHandler_200()
{

}

HTTPResponseStatusHandler_200::~HTTPResponseStatusHandler_200()
{

}

void HTTPResponseStatusHandler_200::ResetStatusHandler()
{

}

HTTPReturnCode
HTTPResponseStatusHandler_200::HandlerResponseStatus(
  uint32 requestId, int statusCode, HTTPStackHelper& cmHTTPStackHelper)
{
  HTTPReturnCode result = HTTP_FAILURE;

  HTTPStateInfo& cmHTTPStateInfo = cmHTTPStackHelper.m_HTTPStateInfo;

  HTTPRequestHandler& httpRequestHandler =
                 cmHTTPStateInfo.m_httpRequestHandler;

  HTTPResponse& httpResponse =
                cmHTTPStateInfo.m_HTTPResponse;

  if (HTTP_GET == httpRequestHandler.GetRequestMethod(requestId))
  {
    switch (statusCode)
    {
    case (204):
      {
        // Explicit no content. Disregard header actual header fields.
        // Set content lenght to zero so client can pick up this info.
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
          "HTTPResponseStatusHandler_200::HandlerResponseStatus No content");

        // Override any specifir of message length eg content length in the
        // message headers.
        httpResponse.SetContentLength(0);
        httpResponse.MarkRspDone();
        httpRequestHandler.SetRequestState(requestId,HTTP_RSP_DONE);

        result = HTTP_SUCCESS;
      }
      break;

    default:
      {
        result = HTTP_SUCCESS;

        if (false == httpResponse.IsMessageLengthSpecifierSupported())
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                       "HTTPResponseStatusHandler_200::HandlerResponseStatus "
                       "Cannot interpret message length");

          httpResponse.MarkRspError();
          httpRequestHandler.SetRequestState(requestId,HTTP_RSP_FAILURE);

          result = HTTP_FAILURE;
        }
      }
      break;
    }
  }
  else
  {
    if (HTTP_HEAD == httpRequestHandler.GetRequestMethod(requestId))
    {
      httpResponse.MarkRspDone();
      httpRequestHandler.SetRequestState(requestId, HTTP_RSP_DONE );
      result = HTTP_SUCCESS;
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponseStatusHandler_200::HandlerResponseStatus "
        "Unknown http request method '%d'", httpRequestHandler.GetRequestMethod(requestId));

      result = HTTP_FAILURE;
    }
  }

  return result;
}

HTTPResponseStatusHandler_300::HTTPResponseStatusHandler_300()
{

}

HTTPResponseStatusHandler_300::~HTTPResponseStatusHandler_300()
{

}

void HTTPResponseStatusHandler_300::ResetStatusHandler()
{

}

HTTPReturnCode
HTTPResponseStatusHandler_300::HandlerResponseStatus(
  uint32 requestId, int statusCode, HTTPStackHelper& cmHTTPStackHelper)
{
  HTTPReturnCode result = HTTP_FAILURE;
  HTTPStateInfo& httpStateInfo = cmHTTPStackHelper.m_HTTPStateInfo;
  HTTPRequestHandler& httpRequestHandler =
                 httpStateInfo.m_httpRequestHandler;
  HTTPResponse& httpResponse = httpStateInfo.m_HTTPResponse;

  switch (httpRequestHandler.GetRequestMethod(requestId))
  {
  case HTTP_HEAD:
    result = HTTP_SUCCESS;
    httpResponse.MarkRspDone();
    httpRequestHandler.SetRequestState(requestId, HTTP_RSP_DONE );
    break;
  case HTTP_GET:
    result = DrainEntityBody(requestId, cmHTTPStackHelper);
    break;
  default:
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unsuppored httpmethod %d", httpRequestHandler.GetRequestMethod(requestId));
    break;
  }

  if (HTTP_SUCCESS == result)
  {
    if (304 == statusCode)
    {
      // Override any specifir of message length eg content length in the
      // message headers.
      httpResponse.SetContentLength(0);
    }

    if (httpRequestHandler.GetNumRedirects(requestId) >= HTTPStackCommon::MAX_REDIRECTS)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                    "HTTPResponseStatusHandler_300::HandlerResponseStatus() "
                    "Num Redirects reached max '%d'", httpRequestHandler.GetNumRedirects(requestId));

      cmHTTPStackHelper.SetState(&HTTPStackStateObjects::HTTPStateErrorObj);
    }
    else
    {
      const char *location = "Location";
      char *redirectUrl = NULL;
      int redirectBufSizeRequested = 0;

      // Get the header value for 'location'
      if (HTTP_SUCCESS != httpStateInfo.GetHeaderValue(requestId,
                         location, (int)std_strlen(location),
                         NULL, 0, &redirectBufSizeRequested))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
          "HTTPResponseStatusHandler_300::HandlerResponseStatus: "
          "'Location' not found int response header");
        result = HTTP_FAILURE;
      }
      else
      {
        redirectUrl = (char *)QTV_Malloc(redirectBufSizeRequested * sizeof(char));
        if (NULL == redirectUrl)
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
            "HTTPResponseStatusHandler_300::HandlerResponseStatus: "
            "Failed to allocate memory for redirectUrl");
        }
        else
        {
          result = HTTP_SUCCESS;
        }
      }

      if (HTTP_SUCCESS == result)
      {
        int temp;
        result = httpStateInfo.GetHeaderValue(requestId,
                       location, (int)std_strlen(location),
                       redirectUrl, redirectBufSizeRequested, &temp);
      }

      if (HTTP_SUCCESS == result)
      {
        // Notify REDIRECT
        cmHTTPStackHelper.NotifyEvent(requestId, HTTP_PROTOCOL_ERROR_CODE);

        QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                       "HTTPResponseStatusHandler_300::HandlerResponseStatus: "
                       "Redirect URL '%s'", redirectUrl);
        result = Redirect(requestId, statusCode, redirectUrl, cmHTTPStackHelper);
      }

      if (NULL != redirectUrl)
      {
        QTV_Free(redirectUrl);
        redirectUrl = NULL;
      }
    }

    if (HTTP_SUCCESS == result)
    {
      result = HTTP_WAIT;
    }
  }

  return result;
}

/**
 * @brief
 *  Parse the URI in the 'Location' header field and set the
 *   HttpStateInfo's hostname, portNumber, httpCommandLineURI
 *   for the http request to the redirected URI.
 *
 * @param statusCode
 * @param redirectUrl
 * @param cmHTTPStackHelper
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode
HTTPResponseStatusHandler_300::Redirect(
  uint32 requestId,
  int statusCode,
  char *redirectUrl,
  HTTPStackHelper &cmHTTPStackHelper)
{
  HTTPReturnCode result = HTTP_FAILURE;
  HTTPStateInfo& httpStateInfo = cmHTTPStackHelper.m_HTTPStateInfo;
  HTTPRequestHandler& httpRequestHandler =
                       httpStateInfo.m_httpRequestHandler;

  char* tmpOriginalUrl = NULL;
  size_t tmpOriginalUrlBufSize = 0;
  const char* requestURL = httpRequestHandler.GetRequestUrl(requestId);

  if (NULL != requestURL)
  {
    tmpOriginalUrlBufSize = std_strlen(requestURL);
    tmpOriginalUrlBufSize++;
    tmpOriginalUrl = (char *)QTV_Malloc(tmpOriginalUrlBufSize * sizeof(char));

    if (NULL == tmpOriginalUrl)
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                    "HTTPRequestHandler::HandleRedirect: Failed to allocate "
                    "'%d' bytes for tmp url", tmpOriginalUrlBufSize);
    }
    else
    {
      std_strlcpy(
        tmpOriginalUrl, requestURL, tmpOriginalUrlBufSize);
    }
  }


  result = httpRequestHandler.HandleRedirect(requestId,statusCode,redirectUrl,tmpOriginalUrl);

      if (HTTP_SUCCESS == result)
      {
    // This will reset all the Queued Requests to initial state,
    // resets response and response status handler
    httpStateInfo.Reset();
        result = cmHTTPStackHelper.CreateOrReuseConnectionObject();
      }

      if (HTTP_SUCCESS == result)
      {
    if (false == httpStateInfo.IsConnected())
        {
          // cannot reuse connection. So, it must have been closed in
          // the call to CreateOrReuseConnection();
          cmHTTPStackHelper.SetState(&HTTPStackStateObjects::HTTPStateConnectingObj);
        }
      }

      if (HTTP_SUCCESS == result)
      {
        // State machine is either in state 'CONNECTING' or 'CONNECTED'
        // depending on whether we could reuse the connection or not.
    (void)cmHTTPStackHelper.IsResponseReceived(requestId);
  }

  if (NULL != tmpOriginalUrl)
  {
    // Populate the request url again so that if retry mechanim is activated,
    // then in case of error, the request will be retried and a redirect
    // possibly received again.
    httpRequestHandler.SetRequestUrl(requestId, tmpOriginalUrl, tmpOriginalUrlBufSize);

    QTV_Free(tmpOriginalUrl);
    tmpOriginalUrl = NULL;
  }

  return result;
}

HTTPResponseStatusHandler_400::HTTPResponseStatusHandler_400() :
  m_PendingAuthState(STATE_0)
{

}

HTTPResponseStatusHandler_400::~HTTPResponseStatusHandler_400()
{

}

void HTTPResponseStatusHandler_400::ResetStatusHandler()
{
  m_PendingAuthState = STATE_0;
}

HTTPReturnCode
HTTPResponseStatusHandler_400::HandlerResponseStatus
(
  uint32 requestId,
  int statusCode,
  HTTPStackHelper& cmHTTPStackHelper
)
{
  HTTPReturnCode result = HTTP_FAILURE;

  HTTPStateInfo& httpStateInfo = cmHTTPStackHelper.m_HTTPStateInfo;

  HTTPRequestHandler& httpRequestHandler =
                 httpStateInfo.m_httpRequestHandler;


  HTTPResponse& httpResponse =
    cmHTTPStackHelper.m_HTTPStateInfo.m_HTTPResponse;

  switch (httpRequestHandler.GetRequestMethod(requestId))
  {
  case HTTP_HEAD:
    result = HTTP_SUCCESS;
    httpResponse.MarkRspDone();
    httpRequestHandler.SetRequestState(requestId, HTTP_RSP_DONE );
    break;
  case HTTP_GET:
    result = DrainEntityBody(requestId,cmHTTPStackHelper);
    break;
  default:
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unsuppored httpmethod %d", httpRequestHandler.GetRequestMethod(requestId));
    break;
  }

  if (HTTP_SUCCESS == result)
  {
    if (STATE_0 == m_PendingAuthState)
    {
      switch (statusCode)
      {
      case 401:
        result = HTTP_SUCCESS;
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "HTTPResponseStatusHandler 401::HandlerResponseStatus Set state WAIT_FOR_AUTH_SET");
        m_PendingAuthState = STATE_1;
        break;
      case 405:
        result = HTTP_NOTSUPPORTED;
        break;
      case 407:
        result = HTTP_SUCCESS;
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "HTTPResponseStatusHandler 407::HandlerResponseStatus Set state WAIT_FOR_AUTH_SET");
        m_PendingAuthState = STATE_1;
        break;
      default:
        result = HTTP_FAILURE;
        break;
      }

      cmHTTPStackHelper.NotifyEvent(requestId,HTTP_PROTOCOL_ERROR_CODE);
    }

    if (STATE_1 == m_PendingAuthState)
    {
      result = HTTP_WAIT;

      // check the headers to check if auth has been set.
      // wait if needed for the client to set header
      // and then sent the request with auth header.
      bool isAuthHdrSet = httpRequestHandler.HeaderExistsForKey(requestId,
        HTTPStackCommon::AUTHORIZATION_KEY, (int)std_strlen(HTTPStackCommon::AUTHORIZATION_KEY));

      if (isAuthHdrSet)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Auth header is set for request");
      }
      else
      {
        isAuthHdrSet = httpRequestHandler.HeaderExistsForKey(requestId,
          HTTPStackCommon::PROXY_AUTHORIZATION_KEY, (int)std_strlen(HTTPStackCommon::PROXY_AUTHORIZATION_KEY));

        if (true == isAuthHdrSet)
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Proxy Auth header is set for request");
        }
      }

      if (isAuthHdrSet)
      {
        // Save the request url before it gets reset in SendRequestInternal.
        const char *requestURL = httpRequestHandler.GetRequestUrl(requestId);
        if(requestURL)
        {
          size_t requestURLBufSize = 1 + std_strlen(requestURL);
          char *tmpAuthUrl = (char *)QTV_Malloc(requestURLBufSize);

          if (tmpAuthUrl)
          {
            std_strlcpy(tmpAuthUrl, requestURL, requestURLBufSize);

            // If the connection cannot be reused, the state will be moved to
            // 'connecting', else it will remain in 'connected'.
            if (HTTP_SUCCESS == httpRequestHandler.SetRequest(requestId,
                                      httpRequestHandler.GetRequestMethod(requestId),
                                      tmpAuthUrl, std_strlen(tmpAuthUrl),httpStateInfo.m_Connection))
            {
              // request with auth-header queued and sent or waiting to send over the
              // connection.
              httpRequestHandler.RemoveHeader(requestId,HTTPStackCommon::AUTHORIZATION_KEY,
                                       (int)std_strlen(HTTPStackCommon::AUTHORIZATION_KEY));

              httpRequestHandler.RemoveHeader(requestId, HTTPStackCommon::PROXY_AUTHORIZATION_KEY,
                                       (int)std_strlen(HTTPStackCommon::PROXY_AUTHORIZATION_KEY));

              m_PendingAuthState = STATE_0; // auth handling done

              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "HTTP Request with auth (or proxy) auth sent to server");
            }
            else
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Failed to send http request in response to auth");
            }

            QTV_Free(tmpAuthUrl);
            tmpAuthUrl = NULL;
          }
        }
      }
    }
  }


  return result;
}

HTTPResponseStatusHandler_500::HTTPResponseStatusHandler_500() :
  m_ResponseHandler500Status(STATE_0)
{

}

HTTPResponseStatusHandler_500::~HTTPResponseStatusHandler_500()
{

}

void HTTPResponseStatusHandler_500::ResetStatusHandler()
{
  m_ResponseHandler500Status = STATE_0;
}

HTTPReturnCode
HTTPResponseStatusHandler_500::HandlerResponseStatus
(
  uint32 requestId,
  int statusCode,
  HTTPStackHelper&  cmHTTPStackHelper
)
{
  HTTPReturnCode result = HTTP_FAILURE;

  HTTPStateInfo& httpStateInfo = cmHTTPStackHelper.m_HTTPStateInfo;

  HTTPRequestHandler& httpRequestHandler =
                 httpStateInfo.m_httpRequestHandler;

  HTTPResponse& httpResponse =
    cmHTTPStackHelper.m_HTTPStateInfo.m_HTTPResponse;

  switch (httpRequestHandler.GetRequestMethod(requestId))
  {
  case HTTP_HEAD:
    result = HTTP_SUCCESS;
    httpResponse.MarkRspDone();
    httpRequestHandler.SetRequestState(requestId, HTTP_RSP_DONE );
    break;
  case HTTP_GET:
    result = DrainEntityBody(requestId ,cmHTTPStackHelper);
    break;
  default:
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unsuppored httpmethod %d", httpRequestHandler.GetRequestMethod(requestId));
    break;
  }

  if (HTTP_SUCCESS == result)
  {
    switch (statusCode)
    {
      case 501:
        result = HTTP_NOTSUPPORTED;
        break;
    default:
      result = HTTP_FAILURE;
      break;
    }

    if (STATE_0 == m_ResponseHandler500Status)
    {
      cmHTTPStackHelper.NotifyEvent(requestId,HTTP_PROTOCOL_ERROR_CODE);
      m_ResponseHandler500Status = STATE_1;
    }
  }
  return result;
}

HTTPRspStatusHandler::HTTPRspStatusHandler() :
  m_pHTTPStatusHandler(NULL)
{

}

HTTPRspStatusHandler::~HTTPRspStatusHandler()
{

}

/**
 * Call when need to start work on the new response.
 */
void HTTPRspStatusHandler::Reset()
{
  m_StatusCode = 0;
  m_pHTTPStatusHandler = NULL;

  m_100Handler.ResetStatusHandler();
  m_200Handler.ResetStatusHandler();
  m_300Handler.ResetStatusHandler();
  m_400Handler.ResetStatusHandler();
  m_500Handler.ResetStatusHandler();
}

/**
 * Check is status handler is already started post processing a
 *  response.
 *
 * @return bool
 */
bool HTTPRspStatusHandler::IsInitialized() const
{
  return (m_StatusCode > 0 ? true : false);
}

/**
 * Initialize the status handler with the status code being
 *   processed.
 * @param statusCode
 */
void HTTPRspStatusHandler::SetStatusHandler(int httpResponseStatus)
{
  int statusType = httpResponseStatus / 100;
  m_StatusCode = httpResponseStatus;
  m_pHTTPStatusHandler = NULL;

  switch (statusType)
  {
  case 1:
    m_pHTTPStatusHandler = &m_100Handler;
    break;
  case 2:
    m_pHTTPStatusHandler = &m_200Handler;
    break;
  case 3:
    m_pHTTPStatusHandler = &m_300Handler;
    break;
  case 4:
    m_pHTTPStatusHandler = &m_400Handler;
    break;
  case 5:
    m_pHTTPStatusHandler = &m_500Handler;
    break;
  }
}

/**
 * Start and/or continue to post-process the response.
 *
 * @param responseCode      For sanity check purpose only.
 * @param rHTTPStackHelper
 *
 * @return HTTPReturnCode
 *  HTTP_SUCCESS  Post processing complete.
 *  HTTP_WAIT     Post processing in progress.
 *  HTTP_FAILURE  General failure
 */
HTTPReturnCode HTTPRspStatusHandler::HandleResponseStatus(
  uint32 requestId, int responseCode,  HTTPStackHelper& rHTTPStackHelper)
{
  HTTPReturnCode rslt = HTTP_FAILURE;

  if (m_pHTTPStatusHandler && responseCode == m_StatusCode)
  {
    rslt = m_pHTTPStatusHandler->HandlerResponseStatus(requestId, m_StatusCode, rHTTPStackHelper);
  }
  else
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "HandleResponseStatus Null m_pHTTPStatusHandler or incorrect status code "
      "%d, should have been %d", responseCode, m_StatusCode);
  }

  return rslt;
}

} // end namespace video
