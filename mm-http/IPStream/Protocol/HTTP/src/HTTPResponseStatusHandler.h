#ifndef HTTPRESPONSESTATUSHANDLER_H
#define HTTPRESPONSESTATUSHANDLER_H
/************************************************************************* */
/**
 * HTTPResponseStatusHandler.h
 * @brief Implements the HTTPRequest
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPResponseStatusHandler.h#12 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackInterface.h"

namespace video
{

class HTTPStackHelper;

/**
 * This class performs an action based on HTTP Response Code.
 * This is called indirectly by IsResponseReceived(). Hence, if
 * connection needs to be torn down and brought up again, the
 * action may include changing the HTTP Stack State and return
 * AEE_EWOULDBLOCK which is propagated to client by IsResponseReceived.
 */
class HTTPResponseStatusHandler
{
public:
  HTTPResponseStatusHandler();
  virtual ~HTTPResponseStatusHandler() = 0;

  /**
   * @brief
   *  Handle the response status.
   *
   * @param statusCode
   * @param cmHTTPStackHelper
   *
   * @return HTTPReturnCode
   */
  virtual HTTPReturnCode HandlerResponseStatus(uint32 requestId,
                                               int statusCode,
                                               HTTPStackHelper &cmHTTPStackHelper) = 0;

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
  HTTPReturnCode DrainEntityBody(uint32 requestId, HTTPStackHelper &cmHTTPStackHelper);
};

/**
 * Handles 100 class of responses
 */
class HTTPResponseStatusHandler_100 : public HTTPResponseStatusHandler
{
public:
  HTTPResponseStatusHandler_100();
  virtual ~HTTPResponseStatusHandler_100();

  /**
   * Reset the status handler to start handling a new response.
   */
  virtual void ResetStatusHandler();

  /**
   * @brief
   *  Server has not rejected the request. So, ignnore the
   *  response and reset http response object so it starts
   *  receiving the actual http response afresh.
   *
   * @param statusCode
   * @param cmHTTPStackHelper
   *
   * @return HTTPReturnCode
   *  HTTP_WAIT
   */
  virtual HTTPReturnCode HandlerResponseStatus(uint32 requestId,
                                               int statusCode,
                                               HTTPStackHelper &cmHTTPStackHelper);

private:

  friend class HTTPResponseStatusHandler;
};

/**
 * Handles 200 class of responses
 */
class HTTPResponseStatusHandler_200 : public HTTPResponseStatusHandler
{
public:
  HTTPResponseStatusHandler_200();
  virtual ~HTTPResponseStatusHandler_200();
  virtual void ResetStatusHandler();
  virtual HTTPReturnCode HandlerResponseStatus(uint32 requestId,
                                               int statusCode,
                                               HTTPStackHelper &cmHTTPStackHelper);

private:

  friend class HTTPResponseStatusHandler;
};

/**
 * Handles 300 class of responses
 */
class HTTPResponseStatusHandler_300 : public HTTPResponseStatusHandler
{
public:
  HTTPResponseStatusHandler_300();
  virtual ~HTTPResponseStatusHandler_300();
  virtual void ResetStatusHandler();
  virtual HTTPReturnCode HandlerResponseStatus(uint32 requestId,
                                               int statusCode,
                                               HTTPStackHelper& cmHTTPStackHelper);

private:

  /**
   * @brief
   * Parse the URI in the 'Location' header field and set
   * the HttpStateInfo's hostname, portNumber, httpCommandLineURI
   * for the http request to the redirected URI.
   *
   * @param m_Status
   * @param redirectUrl
   * @param cmHTTPStackHelper
   *
   * @return AEEResult
   */
  HTTPReturnCode Redirect(
    uint32 requestId,
    int m_Status,
    char *redirectUrl,
    HTTPStackHelper &cmHTTPStackHelper);
};

/**
 * Handles 400 class of responses
 */
class HTTPResponseStatusHandler_400 : public HTTPResponseStatusHandler
{
public:
  HTTPResponseStatusHandler_400();
  virtual ~HTTPResponseStatusHandler_400();
  virtual void ResetStatusHandler();
  virtual HTTPReturnCode HandlerResponseStatus(uint32 requestId,
    int statusCode, HTTPStackHelper &cmHTTPStackHelper);

private:
  friend class HTTPResponseStatusHandler;
  enum PendingAuthState
  {
    STATE_0,  // send notifcation and look at statusCode 401/407 return EWAIT
    STATE_1,  // Wait for client to respond with auth header and send
              // httprequest when it arrives.
  } m_PendingAuthState;
};

/**
 * Handles 500 class of responses.
 */
class HTTPResponseStatusHandler_500 : public HTTPResponseStatusHandler
{
public:
  HTTPResponseStatusHandler_500();
  virtual ~HTTPResponseStatusHandler_500();
  virtual void ResetStatusHandler();
  virtual HTTPReturnCode HandlerResponseStatus(uint32 requestId,
                                               int statusCode,
                                               HTTPStackHelper &cmHTTPStackHelper);

private:
  friend class HTTPResponseStatusHandler;
  enum ResponseHandlingState
  {
    STATE_0, // notification sent
    STATE_1, // wait for reset.
  } m_ResponseHandler500Status;
};

/**
 * HTTPRspStatusHandler
 *  Entry point to the status handlers. The status handlers are
 *  for post processing the response after the httpstack has
 *  received the following for the different status codes:
 *  1XX - received the entire response
 *  2XX - received the response headers, special case 204
 *        received the entire response.
 *  3XX,4XX,5XX - Received the entire response.
 */
class HTTPRspStatusHandler
{
public:
  HTTPRspStatusHandler();
  ~HTTPRspStatusHandler();

  /**
   * Call when need to start work on the new response.
   */
  void Reset();

  /**
   * Check is status handler is already started post processing a
   *  response.
   *
   * @return bool
   */
  bool IsInitialized() const;

  /**
   * Initialize the status handler with the status code being
   *   processed.
   * @param statusCode
   */
  void SetStatusHandler(int statusCode);

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
  HTTPReturnCode HandleResponseStatus(uint32 requestId,
                                      int responseCode,
                                      HTTPStackHelper &cmHTTPStackHelper);

private:
  HTTPResponseStatusHandler_100 m_100Handler;
  HTTPResponseStatusHandler_200 m_200Handler;
  HTTPResponseStatusHandler_300 m_300Handler;
  HTTPResponseStatusHandler_400 m_400Handler;
  HTTPResponseStatusHandler_500 m_500Handler;

  int m_StatusCode;
  HTTPResponseStatusHandler *m_pHTTPStatusHandler;
};

} // end namespace video

#endif /* HTTPRESPONSESTATUSHANDLER_H */
