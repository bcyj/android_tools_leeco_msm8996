/************************************************************************* */
/**
 * HTTPRequestHandler.cpp
 * @brief implementation of the HTTPRequestHandler.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPRequestHandler.cpp#25 $
$DateTime: 2013/07/27 07:46:50 $
$Change: 4174225 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPRequestHandler.h"
#include "HTTPRequest.h"
#include "qtv_msg.h"
#include "SourceMemDebug.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */


/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

namespace video
{

static int CompareRequestID(void *itemPtr, void *compareVal)
{
  HTTPRequestElem *requestElem =
               (HTTPRequestElem *) itemPtr;
  uint32 *compareValReqId = (uint32 *)compareVal;
  int val= (requestElem->m_requestID == (uint32)(*compareValReqId))? 1: 0;
  return val;
}

/**
 * Constructor
 */
HTTPRequestHandler::HTTPRequestHandler()
  :m_isPipelineSupported(false),
   m_nUniqueKey(0),
   m_ProxyServerName(NULL),
   m_ProxyServerPort(0)
{
  StreamQ_init(&m_requestQ);

}


/**
 * Destructor
 */
HTTPRequestHandler::~HTTPRequestHandler()
{
  Reset();
}


/**
 * @brief This Method sets header key and value with the request
 *        object associated with the requestID, if there is no
 *        request element with the requestID in the requestQ,
 *        then it will create a new requestElem for the
 *        requestID
 *
 * @param RequestID
 * @param Key
 * @param KeyLen
 * @param Value
 * @param ValLen
 *
 * @return True indicating success, false indicating failures
 */
bool HTTPRequestHandler::SetHeader
(
  uint32 RequestID,
  const char *Key,
  int KeyLen,
  const char *Value,
  int ValLen
)
{
  bool bReturn = false;
  HTTPRequestElem *pRequestElem = NULL;


  pRequestElem = (HTTPRequestElem *)StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);
  /*if(!pRequestElem)
  {
    pRequestElem = CreateRequest(RequestID);
  }*/

  if(pRequestElem)
  {
    bReturn = pRequestElem->m_httpRequest->SetHeader(Key,KeyLen,Value,ValLen);
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Request with ID: %u not found",
                   RequestID);
  }

  return bReturn;
}

/**
 * @brief This Method removes header and it key from the HTTP
 *        Request
 *
 * @param RequestID
 * @param key
 * @param keyLen
 *
 * @return True indicating success, false indicating failures
 */
bool HTTPRequestHandler::RemoveHeader
(
  uint32 RequestID,
  const char *key,
  int keyLen
)
{
  bool bReturn = false;
  HTTPRequestElem *pRequestElem = NULL;

  pRequestElem = (HTTPRequestElem *)
                  StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

  if(pRequestElem)
  {
    bReturn = pRequestElem->m_httpRequest->RemoveHeader(key,keyLen);
  }

  return bReturn;
}

/**
 * @brief This Method gets header value for the given key and requestID
 *
 * @param RequestID
 * @param Key
 * @param KeyLen
 * @param Value
 * @param ValLen
 *
 * @return True indicating success, False indicating failure
 */
bool HTTPRequestHandler::GetHeader
(
  uint32 RequestID,
  const char *Key,
  int KeyLen,
  char *Value,
  int& ValLen
)
{
  bool bReturn = false;
  HTTPRequestElem *pRequestElem =
    (HTTPRequestElem *)StreamQ_linear_search(&m_requestQ, CompareRequestID, (void*)&RequestID);

  if (pRequestElem && pRequestElem->m_httpRequest)
  {
    bReturn = pRequestElem->m_httpRequest->GetHeader(Key, KeyLen, Value, ValLen);
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "Request with ID: %u not found", RequestID );
  }

  return bReturn;
}

/**
 * @breif This method sets the CommandLine of the Request,
 *        hostname and connection Headers
 *
 * @param RequestID
 * @param method
 * @param Url
 * @param UrlLen
 * @param proxyServer
 *
 * @return HTTP_SUCCESS on SUCCESSS, otherwise appropriate
 *         failure codes(HTTP_BADPARAM/HTTP_FAILURE)
 */
HTTPReturnCode HTTPRequestHandler::SetRequest
(
  uint32 RequestID,
  HTTPMethodType method,
  const char *Url,
  size_t UrlLen,
  TransportConnection *pConnection,
  const char *proxyServer
)
{
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,"SetRequestfor RequestID:%u",
                RequestID);

  HTTPReturnCode result = HTTP_FAILURE;

  HTTPRequestElem *pRequestElem = NULL;

  if ((Url == NULL) || (UrlLen > (size_t)HTTPStackCommon::MAX_URL_LEN))
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "HTTPRequestHandler::SetRequest - invalid relativeURL(%s)/ UrlLen(%d)",
                   Url,UrlLen);
    return HTTP_BADPARAM;
  }

  pRequestElem = (HTTPRequestElem *)
                 StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

  if(pRequestElem)
  {
    HTTPRequest* httpRequest = pRequestElem->m_httpRequest;
    if(httpRequest)
    {
      httpRequest->SetRequestMethod(method);
      result = httpRequest->SetRequestUrl(Url, UrlLen);

      if(result == HTTP_SUCCESS)
      {
        result = httpRequest->ParseHostPortPathFromUrlInternal(
                 httpRequest->GetRequestUrl());
      }

      if(result == HTTP_SUCCESS && proxyServer)
      {
        result = httpRequest->SetRelativePath(Url);
      }

      if(result == HTTP_SUCCESS)
      {
        if(!ComposeAndQueueRequest(RequestID,proxyServer))
        {
          result = HTTP_FAILURE;
        }
      }
    }

    if (StreamQ_cnt(&m_requestQ) > 1)
    {
      //If the element is at the head of queue, no need for pipeline check
      if(pRequestElem != StreamQ_check(&m_requestQ))
      {
        result = HTTP_FAILURE;
        HTTPRequestElem *pPrevRequestElem =
        (HTTPRequestElem *)StreamQ_prev(&m_requestQ,&pRequestElem->link);
        if (pPrevRequestElem && pPrevRequestElem->m_httpRequest)
        {
          HTTPRequest *pPrevHTTPRequest = pPrevRequestElem->m_httpRequest;
          if(httpRequest &&
             (pPrevHTTPRequest->GetHostName()!= NULL) && (pPrevHTTPRequest->GetPort() > 0))
          {
            if((httpRequest->GetHostName() != NULL) && (std_strcmp(pPrevHTTPRequest->GetHostName(), httpRequest->GetHostName()) == 0) &&
               pPrevHTTPRequest->GetPort() == httpRequest->GetPort())
            {
              pRequestElem->bPipeline = true;
            }
            result = HTTP_SUCCESS;
          }
          else
          {
            QTV_MSG_PRIO3( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                           "No HostName (%s) or PortNo (%d) for the last Request Elem %u to compare with",
                           pPrevHTTPRequest->GetHostName(),pPrevHTTPRequest->GetPort(),pPrevRequestElem->m_requestID);

          }
        }
      }
    }
    else if (StreamQ_cnt(&m_requestQ) == 1)
    {
      if(pConnection && httpRequest)
      {
        if(pConnection->IsConnected())
        {
          const char *oldHostName = pConnection->GetHostName();
          unsigned int oldPort = pConnection->GetPort();
          unsigned int Port =  httpRequest->GetPort();

          if(m_ProxyServerName == NULL)
          {
            unsigned int Port =  httpRequest->GetPort();

            if((oldHostName != NULL) &&
               ((oldPort != Port)||
               ( (httpRequest->GetHostName() != NULL) &&
                 (0 != std_strcmp(oldHostName, httpRequest->GetHostName())))))
            {
                pRequestElem->bPipeline = false;
            }
          }
        }
      }
    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Request with ID: %u not found",
                   RequestID);
  }
  return result;
}



/**
 * @brief This Method flushes the headers of a requested Request
 *
 * @param RequestId
 *
 * @return HTTP_SUCCESS on success
 */
HTTPReturnCode HTTPRequestHandler::FlushRequest (uint32 RequestID)
{
  HTTPReturnCode result = HTTP_FAILURE;

  HTTPRequestElem *pRequestElem = NULL;
  pRequestElem = (HTTPRequestElem *)
  StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

  if(pRequestElem)
  {
    pRequestElem->m_httpRequest->FlushHeaders();
    result = HTTP_SUCCESS;
  }

  return result;
}
/**
 * @brief This method creates , enqueue a new Request in to
 *        the requestQ and returns the RequestElem
 *
 * @param RequestID
 *
 * @return pointer to requestElem in case of Success, NULL if
 *         failed
 */
HTTPReturnCode HTTPRequestHandler::CreateRequest
(
  uint32& RequestID
)
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,"CreateRequest: No of Elements in the Queue:%d",
                StreamQ_cnt(&m_requestQ));

  HTTPRequestElem *pRequestElem = NULL;

  HTTPRequest *httpRequest =NULL;
  httpRequest = QTV_New(HTTPRequest);
  if(httpRequest)
  {
      pRequestElem = (HTTPRequestElem*)QTV_Malloc(sizeof(HTTPRequestElem));

      if(pRequestElem)
      {
        RequestID = ++m_nUniqueKey;
        StreamQ_link(pRequestElem,&pRequestElem->link);
        pRequestElem->m_httpRequest = httpRequest;
        pRequestElem->m_requestID = RequestID;
        pRequestElem->bPipeline = false;
        pRequestElem->bMarkForDeletion = false;
        StreamQ_put(&m_requestQ,&pRequestElem->link);
        // set the pipeline flag for first first request to true
        if (StreamQ_cnt(&m_requestQ) == 1)
        {
          pRequestElem->bPipeline = true;
        }
        rsltCode = HTTP_SUCCESS;
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,"Creating HTTPRequest Element for RequestID:%u",
                    RequestID);
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Memory allocation failure");
        QTV_Delete(httpRequest);
      }
  }
  else
  {
     QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"HTTPRequest Element with RequestID:%u already exists",
                   RequestID);
     rsltCode = HTTP_FAILURE;
  }
  return rsltCode;
}


/**
 * @breif This Method either delets the request from Queue if
 *        its is head element or marks a request element for
 *        deletion, this is useful when the request to be
 *        deleted in not the head request and is sent out on
 *        network.The previous request to the request marked as
 *        delete is deleted then the connection has to be
 *        closed, delete this request(marked as delete) from
 *        queue and reopen the connection
 *
 * @param RequestID
 *
 * @return HTTPReturnCode
 *  HTTP_SUCCESS: on success
 *  HTTP_FAILURE: on failure
 *  HTTP_WAIT: if the request has been marked as delete (which
 *  will be deleted when previous requests to this request are
 *  completly received
 */
HTTPReturnCode HTTPRequestHandler::DeleteRequest(uint32 RequestID,bool &bInternalClose)
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;
  bInternalClose = false;

  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

  if(pRequestElem)
  {
    if((pRequestElem->m_requestID == RequestID) && pRequestElem->m_httpRequest)
    {
      if(pRequestElem->m_httpRequest->IsRspFullyRecd())
      {
        rsltCode = DeleteRequest(pRequestElem);
        // if there is only one request in the queue and is marked as delete,
        // then  the connection needs to be closed
        if(StreamQ_cnt(&m_requestQ) == 1)
        {
           HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);
           if(pRequestElem->bMarkForDeletion)
           {
             QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                "reamining one Request(%u) is marked for deletion,Deleting and triggering internal close connection",
                pRequestElem->m_requestID);
             DeleteRequest(pRequestElem);
             bInternalClose = true;
             rsltCode = HTTP_SUCCESS;
           }
        }
      }
      else
      {
         rsltCode = DeleteRequest(pRequestElem);
        // if the response is not fully received then return failure,
        // so that StackHelper would trigger close connection
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                      "Delete Requested for current Active RequestID:%u",RequestID);
        // prepare for close and re connect
        ResetAllRequests();
        bInternalClose = true;
        rsltCode = HTTP_SUCCESS;
      }
    }
    else if(StreamQ_cnt(&m_requestQ) > 1)
    {
      pRequestElem = (HTTPRequestElem *)
                  StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

      if(pRequestElem && pRequestElem->m_httpRequest)
      {
        if(!pRequestElem->m_httpRequest->IsRequestPartiallyOrFullySent())
        {
          rsltCode = DeleteRequest(pRequestElem);
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Marking Request with RequestID:%u for Deletion",RequestID);
          pRequestElem->bMarkForDeletion = true;
          rsltCode = HTTP_WAIT;
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"DeleteRequest:HTTPRequest with RequestID:%u is not Found",
                      RequestID);
      }
    }
  }
  else
  {
     QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"DeleteRequest:HTTPRequest with RequestID:%u is not Found",
                   RequestID);
  }

  return rsltCode;
}

/**
 * @brief This Method sets the commandline of the request, sets
 *        relavent header and marks the request as ready to be
 *        sent
 *
 * @param RequestID
 * @param proxyServer
 *
 * @return true indicating success, false indicating failure
 */
bool HTTPRequestHandler::ComposeAndQueueRequest
(
  uint32 RequestID,
  const char* proxyServer
)
{
  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)
                  StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

  QTV_NULL_PTR_CHECK(pRequestElem, false);

  HTTPRequest* pHTTPRequest = pRequestElem->m_httpRequest;

  QTV_NULL_PTR_CHECK(pHTTPRequest, false);
  QTV_NULL_PTR_CHECK(pHTTPRequest->GetHostName(), false);
  QTV_NULL_PTR_CHECK(pHTTPRequest->GetRelativePath(), false);

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,"ComposeAndQueueRequest for RequestID:%u",
                  RequestID);

  bool result = true;

  switch (pHTTPRequest->GetRequestMethod())
  {
  case (HTTP_HEAD):
    if (false == pHTTPRequest->SetHTTPCommandLine(
        "HEAD", std_strlen("HEAD"),
        pHTTPRequest->GetRelativePath(), std_strlen(pHTTPRequest->GetRelativePath())))
    {
      result = false;
    }
    break;

  case (HTTP_GET):
  default:
    if (false == pHTTPRequest->SetHTTPCommandLine(
      "GET", std_strlen("GET"),
      pHTTPRequest->GetRelativePath(), std_strlen(pHTTPRequest->GetRelativePath())))
    {
      result = false;
    }
  }

  // Set mandatory "Host" field.
  if (!proxyServer)
  {
    if (false == pHTTPRequest->SetHeader(
      "Host", (int)std_strlen("Host"),
      pHTTPRequest->GetHostName(), (int)std_strlen(pHTTPRequest->GetHostName())))
    {
      result = false;
    }
  }
  else
  {
    if (false == pHTTPRequest->SetHeader(
      "Host", (int)std_strlen("Host"),
      proxyServer, (int)std_strlen(proxyServer)))
    {
      result = false;
    }
  }

  // Set a preference for key 'Connection' if client has not set this.
  if (result &&
      (false == pHTTPRequest->HeaderExistsForKey("Connection",
                                               (int)std_strlen("Connection"))))

  {
      if (false == pHTTPRequest->SetHeader(
        "Connection", (int)std_strlen("Connection"),
        "Keep-Alive", (int)std_strlen("Keep-Alive")))
    {
      result = false;
    }
  }

  if ( result)
  {
    // Mark the request as ready to send so tha tIsProcessingARequest() will
    // return true. This protects against pipelining for which is currently
    // not supported.
    result = ( true == pHTTPRequest->MarkReadyToSend()
               ? true : false);
  }

  return result;
}


/**
 * @brief This Method sends out pending HTTP Requests from
 *        requestQ, it indicates if there is a need to create a
 *        newTCP connection if head request in the queue is for
 *        different hostname or port
 *
 * @param pConnection
 * @param bCreateNewConnection
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode HTTPRequestHandler::SendPendingRequests
(
  TransportConnection *pConnection,
  bool& bCreateNewConnection
)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    bCreateNewConnection = DoHavetoCreateNewConnection(pConnection);

    if(!bCreateNewConnection)
    {
      bool bExitLoop = false;
      result = HTTP_SUCCESS;

      for(HTTPRequestElem *pHTTPRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);
           pHTTPRequestElem != NULL && (result != HTTP_FAILURE && result != HTTP_WAIT) && !bExitLoop;
           pHTTPRequestElem = (HTTPRequestElem *)StreamQ_next(&m_requestQ,&pHTTPRequestElem->link))
      {
        // if one of queud request is marked for deletion, stop sending next requests out as the connection has to be brought down
        // when the requests prior to marked request haave been fully received, when the connection is brought up
        // then the pending requests will be sent
        bExitLoop = pHTTPRequestElem->bPipeline ? false : true ||
                    pHTTPRequestElem->bMarkForDeletion;
        if(! bExitLoop)
        {
          HTTPRequest *pHTTPRequest = pHTTPRequestElem->m_httpRequest;
          if(!pHTTPRequest->IsRequestCompletelySent())
          {
            result = pHTTPRequestElem->m_httpRequest->SendRequest(pConnection);
            /* if WAIT status is reported , then try sending the remaining in the next go ,
               when IsResponse received is called*/
            if(result == HTTP_SUCCESS)
            {
              QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"Sent Request( %u),%s out on network",
                            pHTTPRequestElem->m_requestID,pHTTPRequest->GetRequestUrl() );
            }
            else if(result == HTTP_WAIT)
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,"SendPendingRequests: Requests not fully sent, status WAIT ");
            }
            else if(result == HTTP_FAILURE)
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Failed to send Request( %u)",
                            pHTTPRequestElem->m_requestID);
              //If the failure is on a pipelined request, don't notify now as it's not the
              //active request anyway. RECV on a previous request will most likely fail and
              //client will reconnect!
              if (GetRequestStatus(pHTTPRequestElem->m_requestID) == HTTP_WAIT)
              {
                result = HTTP_WAIT;
              }
            }
          }
          if(!m_isPipelineSupported)
          {
            break;
          }
        }
      }
    }
  }
  return result;
}


/**
 * @brief This Method indicates if Request with Request has
 *        completly received the response
 *
 * @param RequestID
 *
 * @return bool
 */
bool HTTPRequestHandler::IsRequestDone
(
  uint32 RequestID
)
{
  bool result = false;

  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)
       StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

   if(pRequestElem)
   {
     result = pRequestElem->m_httpRequest->IsRspFullyRecd();
   }
   else
   {
     QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"unable to find Request with ID:%u",RequestID);
   }
   return result;
}

/**
 * @brief This Method sets the new state of the HTTPRequest
 *        associated with the RequestID based on the
 *        HTTPResponse status
 *
 * @param RequestID
 * @param eRSPStatus
 *
 * @return None
 */
void HTTPRequestHandler::SetRequestState
(
  uint32 RequestID,
  HTTPResponseStatus eRSPStatus
)
{
  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)
       StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);


  if(pRequestElem && pRequestElem->m_httpRequest)
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"SetRequestState for Request( %u ), Rsp Status: %d",
                  RequestID, eRSPStatus );
    switch(eRSPStatus)
    {
    case HTTP_RSP_DONE:
      {
        pRequestElem->m_httpRequest->SetRspFullyRecd();
        break;
      }
    case HTTP_RSP_FAILURE:
      {
        pRequestElem->m_httpRequest->SetRspError();
        break;
      }
    case HTTP_RSP_HDRS_RECVD:
      {
        pRequestElem->m_httpRequest->SetRspHeadersRecd();
      }
    default:
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"Ignoring RSP Status:%d",eRSPStatus);
      break;
    }
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Request( %u) not found",RequestID);
  }
}

/**
 * @brief This Method flushes all the Queued HTTPrequests in the
 *        requestQ
 */
void HTTPRequestHandler::flushRequestQ()
{
  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    for(HTTPRequestElem *pElem = (HTTPRequestElem *)StreamQ_get(&m_requestQ);
         pElem != NULL ;
         pElem = (HTTPRequestElem *)StreamQ_get(&m_requestQ))
    {
      //StreamQ_delete(&pElem->link);
      if(pElem->m_httpRequest)
      {
        QTV_Delete(pElem->m_httpRequest);
        pElem->m_httpRequest = NULL;
      }
      QTV_Free(pElem);
      pElem = NULL;
    }
  }
}

/**
 * @brief This Method indicates if the requestID is the
 *        currently active request
 *
 * @param RequestID
 *
 * @return HTTP_SUCCESS : if the request is the currently active
 *         request , HTTP_WAIT: if the request is not active but
 *         request is a queued one, HTTP_FAILURE if the request
 *         is not present in the queue
 *  */
HTTPReturnCode HTTPRequestHandler::GetRequestStatus
(
  uint32 RequestID
)
{
  HTTPReturnCode status = HTTP_FAILURE;

  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

    if(pRequestElem && pRequestElem->m_requestID == RequestID)
    {
      status = HTTP_SUCCESS;
    }
    else
    {
      pRequestElem = (HTTPRequestElem *)
      StreamQ_linear_search(&m_requestQ,CompareRequestID,(void *)&RequestID);
      if(pRequestElem)
      {
        status = HTTP_WAIT;
      }
    }
  }
  return status;
}


/**
 * @breif This Method returns the Hostname from first
 *        RequestElement
 *
 * @return hostname*
 */
const char* HTTPRequestHandler::GetHostName()
{
  const char *hostName = NULL;
  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);
    if(pRequestElem && pRequestElem->m_httpRequest)
    {
      hostName = pRequestElem->m_httpRequest->GetHostName();
    }
  }
  return hostName;
}

/**
 * @breif This Method retuns the port from the first request
 *
 * @return port number
 */
 unsigned short HTTPRequestHandler::GetPort()
{
  unsigned short port = 0;

  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);
    if(pRequestElem && pRequestElem->m_httpRequest)
    {
      port = pRequestElem->m_httpRequest->GetPort();
    }
  }
  return port;
}

/**
 * @brief This Method returns no of pending Requests to be sent
 *        out in the requestQ
 *
 * @return 0 if not requests in the queue , >0 if there is
 *         atleast one pending requests in the queue
 */
uint32 HTTPRequestHandler::GetNumPendingRequestTobeSent()
{
  uint32 nPendingRequests = 0;

  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    for(HTTPRequestElem *pElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);
         pElem != NULL ;
         pElem = (HTTPRequestElem *)StreamQ_next(&m_requestQ,&pElem->link))
    {
      if(pElem->m_httpRequest && !pElem->m_httpRequest->IsRequestCompletelySent())
      {
        ++nPendingRequests;
      }
    }
  }
  return nPendingRequests;
}

/**
 * @breif  This Method returns the set HTTP method for a request
 *
 * @return HTTPMethodType
 */
HTTPMethodType HTTPRequestHandler::GetRequestMethod(uint32 RequestID)
{
  HTTPMethodType httpMethod = HTTP_GET;

  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)
       StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

  if(pRequestElem && pRequestElem->m_httpRequest)
  {
    httpMethod = pRequestElem->m_httpRequest->GetRequestMethod();
  }
  return httpMethod;
}

void HTTPRequestHandler::ResetAllRequests()
{
  for(HTTPRequestElem *pElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);
      pElem != NULL;
      pElem = (HTTPRequestElem *)StreamQ_next(&m_requestQ,&pElem->link))
  {
    if(pElem->m_httpRequest)
    {
      pElem->m_httpRequest->MarkSentRequestAsReSend();
    }
  }
}


/**
 * @brief This Method indicates if the response headers have
 *        been received for given request
 *
 * @param RequestID
 *
 * @return True if response headers received , false otherwise
 */
bool HTTPRequestHandler::IsRequestRspHrdReceived(uint32 RequestID)
{
  bool rsltCode = true;

  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

    if(pRequestElem && pRequestElem->m_requestID == RequestID &&
       pRequestElem->m_httpRequest)
    {
      rsltCode = pRequestElem->m_httpRequest->IsRspHeadersRecd();
    }
  }

  return rsltCode;
}

/**
 * @brief This method indicates if the request is sent out fully
 *        for the given requestID
 *
 * @param[in] requestId
 *
 * @return true indicating that request is sent out fully,
 *         false otherwise
 */
bool HTTPRequestHandler::IsRequestSent(const uint32 RequestID)
{
  bool rsltCode = true;

  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

    if(pRequestElem && pRequestElem->m_requestID == RequestID &&
       pRequestElem->m_httpRequest)
    {
      rsltCode = pRequestElem->m_httpRequest->IsRequestCompletelySent();
    }
  }

  return rsltCode;
}

/**
 * @brief This Method prepares the Request for reconnecting
 *
 * @param RequestID
 *
 * @return HTTP_SUCCESS: if the HTTPRequest has been successfuly
 *         initialized for reconneting
 *         HTTP_FAILURE: if the max retired for the request has
 *         been reached or if the request with requestID is not
 *         found
 */
HTTPReturnCode HTTPRequestHandler::PrepareForReconnect(uint32 RequestID)
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  if (StreamQ_cnt(&m_requestQ) > 0)
  {
    HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

    if (pRequestElem && pRequestElem->m_requestID == RequestID)
    {
      if (pRequestElem->m_httpRequest)
      {
        if(ShouldRetry(RequestID))
        {
          HTTPRequest *pHTTPRequest = pRequestElem->m_httpRequest;
          pHTTPRequest->IncNumRetries();
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                        "PrepareForReconnect '%d'th RETRY out of '%d",
                        pHTTPRequest->GetNumRetries(), HTTPStackCommon::MAX_RETRIES);
          pHTTPRequest->MarkSentRequestAsReSend();
          rsltCode = HTTP_SUCCESS;
        }
      }
    }
    else if(pRequestElem)
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,
                    "PrepareForReconnect: RequestID:%u, is the currently Active Request(%u)",
                    RequestID, pRequestElem->m_requestID );
      rsltCode = HTTP_WAIT;
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"PrepareForReconnect:RequestQ is empty");
  }
  return rsltCode;
}

/**
 * Check if request should be retried on connection error.
 */
bool HTTPRequestHandler::ShouldRetry(uint32 nRequestID)
{
  bool bRslt = false;

  if (StreamQ_cnt(&m_requestQ) > 0)
  {
    HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

    if (pRequestElem && pRequestElem->m_requestID == nRequestID)
    {
      HTTPRequest *pHTTPRequest = pRequestElem->m_httpRequest;
      if (pHTTPRequest)
      {
        if ((MAX_INT32 != HTTPStackCommon::MAX_RETRIES &&
             pHTTPRequest->GetNumRetries() >= HTTPStackCommon::MAX_RETRIES) ||
            (NULL == pHTTPRequest->GetRequestUrl()))
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                        "PrepareForReconnect: Reached max retry limit '%d' "
                        "Or httprequest url is NULL",
                        HTTPStackCommon::MAX_RETRIES);
        }
        else
        {
          bRslt = true;
        }
      }
    }
  }

  return bRslt;
}

/**
 * @brief This Method indicated how many redirection for a
 *        request has already happened
 *
 * @param RequestID
 *
 * @return integer indicating number of redirects happened
 *
 */
int HTTPRequestHandler::GetNumRedirects(uint32 RequestID)
{
  int numRedirects = 0;

  HTTPRequestElem *pRequestElem = NULL;
  pRequestElem = (HTTPRequestElem *)
  StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

  if(pRequestElem && pRequestElem->m_httpRequest)
  {
    numRedirects = pRequestElem->m_httpRequest->GetNumRedirects();
  }
  return numRedirects;
}

/**
 * @brief This Method initializes the HTTPRequest with the
 *        redirect URL, sets the hostname and port to connect,
 *        sets the new command line , headers etc.
 *
 * @param RequestID
 * @param ResponseStatusCode
 * @param redirectUrl
 *
 * @return HTTP_SUCCESS/HTTP_FAILURE
 */
HTTPReturnCode HTTPRequestHandler::HandleRedirect
(
  uint32 RequestID,
  int ResponseStatusCode,
  const char *redirectUrl,
  const char *originalUrl
)
{
  HTTPReturnCode result = HTTP_FAILURE;

  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

  if(pRequestElem && pRequestElem->m_requestID == RequestID)
  {
    HTTPRequest *httpRequest = pRequestElem->m_httpRequest;
    if( httpRequest)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                    "Handle Redirect for Request(%u)",RequestID );

      httpRequest->IncNumRedirects();
      // Reset HTTP request ,set the request's
      // hostname, port and path using Location hdr value and appropriate
      // use of status code that is 305 or non-305 status.
      // caller should reset the response
      httpRequest->Reset();

      // turn Off pipeline support flag, relearn the support from the response
      m_isPipelineSupported = false;

      char *hostName = NULL;
      uint16 portNumber = 0;
      char *relativePath = NULL;

      if (NULL != redirectUrl)
      {
        // Extract hostName, port, relativePath from RedirectURL
        size_t hostNameBufSizeRequested = 0;
        size_t relativePathBufSizeRequested = 0;

        result = HTTPStackCommon::GetHostPortRelativePathFromUrl(
          redirectUrl, std_strlen(redirectUrl),
          NULL, 0, hostNameBufSizeRequested,
          portNumber,
          NULL, 0, relativePathBufSizeRequested);

        if (HTTP_SUCCESS != result)
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
            "HTTPRequestHandler::HandleRedirect: "
            "Failed to parse redirectURL");
        }
        else
        {
          hostName = (char *)QTV_Malloc(hostNameBufSizeRequested * sizeof(char));
          relativePath = (char *)QTV_Malloc(relativePathBufSizeRequested * sizeof(char));

          if ((NULL == hostName) || (NULL == relativePath))
          {
            result = HTTP_FAILURE;

            QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
              "HTTPRequestHandler::HandleRedirect: "
              "Failed to allocate memory for hostName or relativePath");
          }
          else
          {
            size_t temp = 0;
            result = HTTPStackCommon::GetHostPortRelativePathFromUrl(
                                      redirectUrl,
                                      std_strlen(redirectUrl),
                                      hostName, hostNameBufSizeRequested, temp,
                                      portNumber,
                                      relativePath, relativePathBufSizeRequested, temp);
          }
        }

        if (HTTP_SUCCESS == result)
        {
          bool redirectUrlContainsHTTP = (NULL != std_strstr(redirectUrl, "http://") ?
                                          true :
                                          false);

          if (true == redirectUrlContainsHTTP)
          {
            httpRequest->SetPort(portNumber);
            result = httpRequest->SetHostName(hostName);
          }

          if (305 == ResponseStatusCode)
          {
            // For 305, request uri is the absolute http uri of the
            // original request (that received the REDIRECT).
            result = httpRequest->SetRelativePath(originalUrl);
          }
          else
          {
            // For 3XX other than 305, request Uri is the relative
            // path in the 'Location' header.
            result = httpRequest->SetRelativePath(relativePath);
          }

          if (HTTP_SUCCESS == result)
          {
            //update the pipeline flag for the second request in the Queue
            //based on the changed hostname and portNo of the first request
            UpdatePiplineStatusForQueuedRequest();
            if(!ComposeAndQueueRequest(RequestID))
            {
              result = HTTP_FAILURE;
            }
          }
        }
      }
      else
      {
         QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                       "HTTPRequestHandler::HandleRedirect: NULL RedirectURL");
      }

       if (NULL != hostName)
       {
         QTV_Free(hostName);
         hostName = NULL;
       }

       if (NULL != relativePath)
       {
         QTV_Free(relativePath);
         relativePath = NULL;
       }
    }
  }
  else if(pRequestElem)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                   "RequestID :%u is not the active Request(%u)",
                    RequestID,pRequestElem->m_requestID);
  }

  return result;
}

/**
 * @brief This Method provides the requestURL for given request
 *
 * @param RequestID
 *
 * @return const char*
 */
const char* HTTPRequestHandler::GetRequestUrl(uint32 RequestID)
{
  char* url = NULL;

  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)
  StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

  if(pRequestElem && pRequestElem->m_httpRequest)
  {
    url = (char*)pRequestElem->m_httpRequest->GetRequestUrl();
  }
  return url;
}

/**
 * @brief This Method Sets RequestURL for a request
 *
 * @param url
 * @param urlLen
 *
 * @return HTTP_SUCCESS/HTTP_FAILURE
 */
HTTPReturnCode HTTPRequestHandler::SetRequestUrl(uint32 RequestID, const char* url, size_t urlLen)
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)
  StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);
  if(pRequestElem && pRequestElem->m_httpRequest)
  {
    rsltCode = pRequestElem->m_httpRequest->SetRequestUrl(url,urlLen);
  }
  return rsltCode;
}

/**
 * @brief This Method indicates if the given Header exisits with
 *        HTTPrequest
 *
 * @param RequestID
 * @param key
 * @param keyLen
 *
 * @return true indicating that the header exists, false
 *         otherwise
 */
bool HTTPRequestHandler::HeaderExistsForKey(uint32 RequestID,const char *key, int keyLen)
{
  bool result = false;

  HTTPRequestElem *pRequestElem = (HTTPRequestElem *)
  StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);
  if(pRequestElem && pRequestElem->m_httpRequest)
  {
    result = pRequestElem->m_httpRequest->HeaderExistsForKey(key,keyLen);
  }
  return result;
}

/**
 * @brief This Method sets pipepline support
 *
 * @param bIsSupported
 */
void HTTPRequestHandler::SetPipelineSupport(bool bIsSupported)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"Setting Pipelining support to :%d",
                bIsSupported);
  m_isPipelineSupported = bIsSupported;
}

/**
 * @brief This Method updates the pipeline status for second
 *        requestElem in the queue, this is required after first
 *        request has got redirection message
 *
 * @return bool
 */
void HTTPRequestHandler::UpdatePiplineStatusForQueuedRequest()
{
  if(StreamQ_cnt(&m_requestQ) > 0)
  {
    HTTPRequestElem *pCurrHTTPRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);
    if(pCurrHTTPRequestElem && pCurrHTTPRequestElem->m_httpRequest)
    {
      HTTPRequestElem *pNextRequestElem = (HTTPRequestElem *)StreamQ_next(&m_requestQ,&pCurrHTTPRequestElem->link);
      if(pNextRequestElem != NULL && pNextRequestElem->m_httpRequest)
      {
        const char *CurrHostname = pCurrHTTPRequestElem->m_httpRequest->GetHostName();
        int CurrportNo = pCurrHTTPRequestElem->m_httpRequest->GetPort();
        const char *NextHostname = pNextRequestElem->m_httpRequest->GetHostName();
        int NextportNo = pNextRequestElem->m_httpRequest->GetPort();

        if(CurrHostname && NextHostname && (CurrportNo > 0) && (NextportNo > 0) )
        {
          if(std_strcmp(CurrHostname, NextHostname) == 0 &&
             (CurrportNo == NextportNo))
          {
            pNextRequestElem->bPipeline = true;
          }
          else
          {
            pNextRequestElem->bPipeline = false;
          }
        }
      }
    }
  }
  return;
}

/**
 * @brief This Method indicated if current connection has to be
 *        closed and a new connection has to be created as the
 *        current request is for different host or port
 *
 * @param pConnection
 *
 * @return true indicating that a new connection has to be
 *         created , false indicating existing connection can be
 *         reused
 */
bool HTTPRequestHandler::DoHavetoCreateNewConnection
(
  TransportConnection *pConnection
)
{
  bool createnewConnection = false;

  CheckAndHandleMarkedAsDeleteRequest(createnewConnection);

  HTTPRequestElem *pHTTPRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

  if(pHTTPRequestElem && pHTTPRequestElem->m_httpRequest &&
     !pHTTPRequestElem->bPipeline && !pHTTPRequestElem->m_httpRequest->IsRequestCompletelySent())
  {
    HTTPRequest* pHTTPRequest = pHTTPRequestElem->m_httpRequest;
    if(pConnection)
    {
      const char *oldHostName = pConnection->GetHostName();
      unsigned int oldPort = pConnection->GetPort();

      if(m_ProxyServerName == NULL)
      {
        unsigned int Port =  pHTTPRequest->GetPort();

        if(pConnection->IsConnected() &&
         (oldHostName != NULL) &&
         ((oldPort != Port)||
         (0 != std_strcmp(oldHostName, pHTTPRequest->GetHostName()))))
        {
          createnewConnection = true;
          pHTTPRequestElem->bPipeline = true;
          QTV_MSG_PRIO5(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                     "SendPendingRequests:Head Request(%u) have different hostname: %s : %d, current connections is on %s : %d",
                     pHTTPRequestElem->m_requestID, pHTTPRequest->GetHostName(),pHTTPRequest->GetPort(),
                     pConnection->GetHostName(),pConnection->GetPort());
        }
      }
      else
      {
        if(pConnection->IsConnected() &&
         (oldHostName != NULL) &&
         ((oldPort != m_ProxyServerPort)||
         (0 != std_strcmp(oldHostName, m_ProxyServerName))))
        {
          createnewConnection = true;
          pHTTPRequestElem->bPipeline = true;
          QTV_MSG_PRIO5(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                     "SendPendingRequests:Head Request(%u) have ProxyServer: %s : %d, which deffers from current connections is on %s : %d",
                     pHTTPRequestElem->m_requestID, m_ProxyServerName,m_ProxyServerPort,
                     pConnection->GetHostName(),pConnection->GetPort());

        }

      }
    }
  }
  return createnewConnection;
}

/**
 * @brief This Method will delete all the Requests in the Queue
 *        and resets information
 */
void HTTPRequestHandler::Reset()
{
  flushRequestQ();
  m_isPipelineSupported = false;
  if(m_ProxyServerName)
  {
    QTV_Free(m_ProxyServerName);
    m_ProxyServerName = NULL;
  }
}

/**
 * @breif This Method checks if the head request element is
 *        marked for Deletion, deletes the request and indicated
 *        to reconnect
 *
 * @param bCreateNewConnection
 */
void HTTPRequestHandler::CheckAndHandleMarkedAsDeleteRequest
(
  bool &bCreateNewConnection
)
{
  bCreateNewConnection = false;
  HTTPRequestElem *pHTTPRequestElem = (HTTPRequestElem *)StreamQ_check(&m_requestQ);

  if(pHTTPRequestElem && pHTTPRequestElem->m_httpRequest && pHTTPRequestElem->bMarkForDeletion)
  {
    bCreateNewConnection = true;

    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Deleting Request with RequestID:%u",
                  pHTTPRequestElem->m_requestID);
    StreamQ_delete(&pHTTPRequestElem->link);
    QTV_Delete(pHTTPRequestElem->m_httpRequest);
    pHTTPRequestElem->m_httpRequest = NULL;
    QTV_Delete(pHTTPRequestElem);
    pHTTPRequestElem = NULL;

    // reset the state to resend of Next Requests in the Queue
    ResetAllRequests();
  }
  return;
}

/**
 * @breif This Method deletes the request element from the queue
 *
 * @param pRequestElem
 *
 * @return HTTP_SUCCESS on success and HTTP_FAILURE in failure
 */
HTTPReturnCode HTTPRequestHandler::DeleteRequest(HTTPRequestElem *pRequestElem)
{
  QTV_NULL_PTR_CHECK(pRequestElem,HTTP_FAILURE);
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Deleting Request with RequestID:%u",
                pRequestElem->m_requestID);
  StreamQ_delete(&pRequestElem->link);
  if(pRequestElem->m_httpRequest)
  {
    QTV_Delete(pRequestElem->m_httpRequest);
    pRequestElem->m_httpRequest = NULL;
  }
  QTV_Delete(pRequestElem);
  pRequestElem = NULL;
  return  HTTP_SUCCESS;
}


bool HTTPRequestHandler::IsRequestPartiallyOrFullySent(uint32 RequestID)
{
  bool result = false;
  HTTPRequestElem *pRequestElem = NULL;

  pRequestElem = (HTTPRequestElem *)
    StreamQ_linear_search(&m_requestQ,CompareRequestID,(void*)&RequestID);

  if(pRequestElem && pRequestElem->m_httpRequest)
  {
    result = pRequestElem->m_httpRequest->IsRequestPartiallyOrFullySent();
  }

  return result;
}

HTTPReturnCode HTTPRequestHandler::SetProxyInfo
(
  const char *proxyServer,
  unsigned short proxyPort
)
{
  HTTPReturnCode rsltCode  = HTTP_FAILURE;;
  if(proxyServer)
  {
    if(m_ProxyServerName != NULL)
    {
      QTV_Free(m_ProxyServerName);
      m_ProxyServerName = NULL;
    }

    size_t proxyServerLen = std_strlen(proxyServer) + 1;

    m_ProxyServerName = (char *)QTV_Malloc(proxyServerLen * sizeof(char));

    if (NULL == m_ProxyServerName)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "HTTPStackHelper::SetProxyInfo() Failed to allocate ProxyServer");
    }
    else
    {
      std_strlcpy(m_ProxyServerName, proxyServer, proxyServerLen);
      m_ProxyServerPort = (proxyPort > 0)? proxyPort: 80;
      rsltCode = HTTP_SUCCESS;
    }
  }
  return rsltCode;

}

const char* HTTPRequestHandler::GetProxyServer() const
{
  return m_ProxyServerName;
}

unsigned short HTTPRequestHandler::GetProxyPort() const
{
  return m_ProxyServerPort;
}


} // end of namespace
