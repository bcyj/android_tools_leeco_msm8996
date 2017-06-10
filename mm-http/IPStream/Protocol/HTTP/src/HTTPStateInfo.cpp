/************************************************************************* */
/**
 * HTTPStateInfo.cpp
 * @brief implementation of the HTTPStateInfo.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStateInfo.cpp#27 $
$DateTime: 2013/09/20 11:38:26 $
$Change: 4469780 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStateInfo.h"
#include "TransportConnectionTcp.h"
#include "qtv_msg.h"
#include "SourceMemDebug.h"
#include "AEEStdDef.h"
#include "IPStreamSourceUtils.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */


/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

namespace video
{

/**
 *  Constructor
 */
HTTPStateInfo::HTTPStateInfo(HTTPCookieMgr& cookieMgr)
: m_CookieMgr(cookieMgr)
{
  m_Connection = NULL;
  m_IsPreviousConnectionPersistent = false;
  m_SocketSendBufSize = -1;
  m_SocketRcvBufSizeMax = -1;
  m_SocketRcvBufSize = -1;
  m_httpRequestHandler.Reset();
  m_HTTPResponse.Reset();
  m_HTTPRspStatusHandler.Reset();
}

/**
 *  Destructor
 */
HTTPStateInfo::~HTTPStateInfo()
{
  if (NULL != m_Connection)
  {
    QTV_Delete(m_Connection);
    m_Connection = NULL;
  }
}

/**
 * @breif This Method creates a new Request for RequestID
 *
 * @param RequestID
 *
 * @return HTTPReturnCode
 *  HTTP_SUCCESS: on success
 *  HTTP_FAILURE: on failure
 */
HTTPReturnCode HTTPStateInfo::CreateRequest(uint32& requestId)
{
  return m_httpRequestHandler.CreateRequest(requestId);
}

/**
 * @breif This Method deletes Request associated with RequestID
 *
 * @param RequestID
 *
 * @return HTTPReturnCode
 *  HTTP_SUCCESS: on success
 *  HTTP_FAILURE: on failure
 */
HTTPReturnCode HTTPStateInfo::DeleteRequest(uint32 requestId,bool &bInternalClose)
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;
  rsltCode =  m_httpRequestHandler.DeleteRequest(requestId,bInternalClose);
  if(rsltCode == HTTP_SUCCESS)
  {
  m_HTTPResponse.Reset();
  m_HTTPRspStatusHandler.Reset();
  }
  else if(rsltCode == HTTP_WAIT)
  {
    rsltCode = HTTP_SUCCESS;
  }
  return rsltCode;
}

/**
 * @brief This Method sets the Header(key: Value) for the
 *        requestID
 *
 * @param RequestID
 * @param Key
 * @param KeyLen
 * @param Value
 * @param ValLen
 *
 * @return True indicating success , false indicating failure
 */
bool HTTPStateInfo::SetHeader
(
  uint32 RequestID,
  const char *Key,
  int KeyLen,
  const char *Value,
  int ValLen
)
{
  return m_httpRequestHandler.SetHeader(RequestID,Key,KeyLen,Value,ValLen);
}


/**
 * @brief This Method sets Request CommandLine, Headers like
 *        Host ,connection for the Request with RequestID
 *
 * @param RequestID
 * @param method
 * @param Url
 * @param UrlLen
 * @param proxyServer
 *
 * @return HTTP_SUCCESS indicating success, HTTP_FAILURE if
 *         there is a failure and HTTP_BADPARAM is there is any
 *         issue with the passed arguments
 */
HTTPReturnCode HTTPStateInfo::SetRequest
(
  uint32 RequestID,
  HTTPMethodType method,
  const char *Url,
  int UrlLen,
  const char *proxyServer
)
{
  SetCookieReqHeader(RequestID, method, Url, UrlLen);
  return m_httpRequestHandler.SetRequest(RequestID,
                                         method,
                                         Url,
                                         UrlLen,
                                         m_Connection,
                                         proxyServer);
}

/**
 * @brief This Method Queries Cookie manager for the
 *        Cookie header-value pair coresponds to the URL.
 *        and if cookies are persent in the cookiemgr,
 *        method sets the cookie header-value to the request
 *
 * @param RequestID
 * @param method
 * @param Url
 * @param UrlLen
 *
 * @return
 *  None
 */
void HTTPStateInfo::SetCookieReqHeader
(
  uint32 RequestID,
  HTTPMethodType /*method*/,
  const char *Url,
  int UrlLen
)
{
  char *cUri = NULL;
  bool bOk = true;
  size_t hlen = 0;
  size_t vlen = 0;
  char *header = NULL;
  char *value = NULL;

  if(Url)
  {
    cUri = (char *)QTV_Malloc(UrlLen + 1);
    if(NULL == cUri)
    {
      bOk = false;
    }

    if(bOk)
    {
      std_strlcpy(cUri, Url, UrlLen);
      cUri[UrlLen] = '\0';
    }

    if(bOk)
    {
      bOk = m_CookieMgr.GetCookies(cUri, NULL, hlen, NULL, vlen);
    }

    if(bOk)
    {
      header = (char *)QTV_Malloc(hlen);
      value = (char *)QTV_Malloc(vlen);
      if((NULL == header) || (NULL == value))
      {
        bOk = false;
      }
    }

    if(bOk)
    {
      bOk = m_CookieMgr.GetCookies(cUri, header, hlen, value, vlen);
    }

    if(bOk)
    {
      SetHeader(RequestID, header, (int)hlen, value, (int)vlen);
    }

    if(header)
    {
      QTV_Free(header);
      header = NULL;
    }

    if(value)
    {
      QTV_Free(value);
      value = NULL;
    }

    if(cUri)
    {
      QTV_Free(cUri);
      cUri = NULL;
    }
  }
}

/**
 * @brief This Method requests HTTPRequestHandler to send out
 *        all pending requests out on the network
 *
 * @return HTTP_SUCCESS indicating that all the Requests have
 *         been sent out on the network
 *
 *         HTTP_WAIT indicates that the connection object
 *         returned wait wile sending requests, this would also
 *         mean that a HTTPRequest is send partial, caller
 *         should try calling method again later
 *
 *         HTTP_FAILURE indicates that sending requests out
 *         failed
 */
HTTPReturnCode HTTPStateInfo::SendPendingRequests(bool& bCreateNewConnection)
{
 return m_httpRequestHandler.SendPendingRequests(m_Connection,bCreateNewConnection);
}

/**
 * @brief This method tells if there are any HTTPRequests being
 *        processed
 *
 * @return true indicating that there is a requests being
 *         processed , false indicating no outstanding requests
 */
bool HTTPStateInfo::IsProcessingARequest()
{
  return (m_httpRequestHandler.GetNumPendingRequestTobeSent() > 0 ? true : false);
}

/**
 * @breif  This Method returns the set HTTP method for a request
 *
 * @return HTTPMethodType
 */
HTTPMethodType HTTPStateInfo::GetRequestMethod(uint32 RequestID)
{
  return m_httpRequestHandler.GetRequestMethod(RequestID);
}

/**
 * @breif  This Method returns the set HTTP method for a request
 *
 * @return HTTPMethodType
 */
const char* HTTPStateInfo::GetRequestUrl(uint32 RequestID)
{
  return (const char*)m_httpRequestHandler.GetRequestUrl(RequestID);
}

/**
 * @brief This Method validates the Request and receives the
 *        response from the transportconnection
 *
 * @param RequestID
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode HTTPStateInfo::ReceiveResponse(uint32 RequestID)
{
  HTTPReturnCode nReturn = HTTP_FAILURE;

  nReturn = m_httpRequestHandler.GetRequestStatus(RequestID );

  if(nReturn == HTTP_SUCCESS &&
     m_httpRequestHandler.IsRequestDone(RequestID))
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                  "RequestID:%u is already marked as done",RequestID);
    return HTTP_SUCCESS;
  }

 /* if(nReturn == HTTP_WAIT &&
     m_httpRequestHandler.IsPreviousRequestDone(RequestID))
  {
    m_httpRequestHandler.SetRequestAsActive(RequestID);
    m_HTTPResponse.Reset();
    nReturn = HTTP_SUCCESS;
  } */

  if(nReturn == HTTP_SUCCESS)
  {
    HTTPResponseStatus rspStatus =
                  m_HTTPResponse.ReceiveResponse(m_Connection);

    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"ReceiveResponse Staus :%d", rspStatus);
    m_httpRequestHandler.SetRequestState(RequestID,rspStatus);

    if (true ==  IsResponseHeaderFullyReceived(RequestID))
    {
      rspStatus = HTTP_RSP_SUCCESS;
    }

    convertRspStatusToCommonStatus(rspStatus,nReturn);

    if(nReturn == HTTP_SUCCESS && m_HTTPResponse.IsResponseHeaderFullyReceived())
    {
      char *Url = 0;
      Url = (char *)GetRequestUrl(RequestID);
      if(Url)
      {
        // Store the all the cookies if present in the response header,
        // irrespective of http response code
        (void)StoreRespCookies(RequestID, Url);
      }

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"Received Successful response for Request( %u)",
                    RequestID);
      //m_httpRequestHandler.SetPipelineSupport(false);
      m_httpRequestHandler.SetPipelineSupport(m_HTTPResponse.IsConnectionPersistent());
    }
  }

  QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"Receive(%u) Response status :%d",RequestID,nReturn);

  return nReturn;
}

/**
 * @brief This stores the cookies all the cookies in the response
 *        into the cookie manager.
 *
 * @param RequestID
 * @param Url
 * @return None
 */
void HTTPStateInfo::StoreRespCookies
(
 uint32 requestId,
 char *Url
)
{
  ordered_StreamList_type *pCookieList = NULL;
  HTTPCookieElement *pElement = NULL;

  GetCookieHeaderValueList(requestId, &pCookieList);
  if(pCookieList)
  {
    // Traverse through the list and store the cookies into cookie manager
    pElement =  (HTTPCookieElement *)ordered_StreamList_peek_front(pCookieList);
    while(pElement)
    {
      m_CookieMgr.StoreCookie(pElement->value, Url);
      pElement = (HTTPCookieElement *)ordered_StreamList_peek_next(&pElement->link);
    }
  }
}

/**
 * @brief  Pass on IPAdress and its size
 *
 * @param size
 *
 * @return ipAddr
 */
char*
HTTPStateInfo::GetIPAddr(int& size)
{
  char *ipAddr = NULL;
  if(m_Connection)
  {
    ipAddr = m_Connection->GetIPAddr(size);
  }
  return ipAddr;
}
/**
 * @brief This method convers
 *
 * @param rspStatus
 * @param httpReturnCode
 */
void HTTPStateInfo::convertRspStatusToCommonStatus
(
  const HTTPResponseStatus rspStatus,
  HTTPReturnCode &httpReturnCode
)
{
  httpReturnCode = HTTP_FAILURE;
  switch(rspStatus)
  {
    case HTTP_RSP_DONE:
    case HTTP_RSP_HDRS_RECVD:
      httpReturnCode = HTTP_SUCCESS;
      break;

    case HTTP_RSP_FAILURE:
      httpReturnCode = HTTP_FAILURE;
      break;

    case HTTP_RSP_SUCCESS:
      httpReturnCode = HTTP_SUCCESS;
      break;

    case HTTP_RSP_WAIT:
      httpReturnCode = HTTP_WAIT;
      break;

    case HTTP_RSP_INSUFFBUFFER:
      httpReturnCode = HTTP_INSUFFBUFFER;
      break;

    default:
       QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                    "unknown RSPStatus :%d",rspStatus);
       break;
  }
}

/**
 * @breif This Method retreives data from the recives buffer
 *
 * @param RequestID
 * @param readBuf
 * @param readBufSize
 * @param readLen
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode HTTPStateInfo::GetData
(
  uint32 RequestID,
  char *readBuf,
  size_t readBufSize,
  size_t& readLen
)
{
  HTTPReturnCode nReturn = HTTP_FAILURE;

  nReturn = m_httpRequestHandler.GetRequestStatus(RequestID );

  if(nReturn == HTTP_SUCCESS)
  {
    if( !m_httpRequestHandler.IsRequestDone(RequestID))
    {
      if(m_HTTPResponse.IsResponseHeaderFullyReceived())
      {
        HTTPResponseStatus rspStatus =
                  m_HTTPResponse.GetData(m_Connection,readBuf,readBufSize,readLen);

        m_httpRequestHandler.SetRequestState(RequestID,rspStatus);
        convertRspStatusToCommonStatus(rspStatus,nReturn);
        if(HTTP_RSP_DONE == rspStatus)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,"Completely Recieved Data for Request( %u)",
                    RequestID);
        }
      }
    }
    else
    {
      /* request is already marked as done*/
      readLen = 0;
      nReturn = HTTP_SUCCESS;
    }
  }
  return nReturn;
}

/**
 * @brief This Method gives the header value from HTTPresponse
 *
 * @param requestId
 * @param key
 * @param keyLen
 * @param value
 * @param valueLen
 * @param valueLenReq
 *
 * @return true indicating success, false indicating failure
 */
HTTPReturnCode HTTPStateInfo::GetHeaderValue
(
  uint32 RequestId,
  const char *key,
  int keyLen,
  char *value,
  int valueLen,
  int *valueLenReq
)
{
  HTTPReturnCode Status = HTTP_FAILURE;

  Status = m_httpRequestHandler.GetRequestStatus(RequestId );
  if(Status == HTTP_SUCCESS)
  {
    if(!m_HTTPResponse.GetHeaderValue(key,keyLen,value,valueLen,*valueLenReq))
    {
      Status = HTTP_FAILURE;
    }
  }
  else
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Get Request Status Failed:%d for RequestID:%u",
                  Status, RequestId);
  }
  return Status;
}

/**
 * @brief This Method gives the header value from HTTPresponse
 *
 * @param requestId
 * @param key
 * @param keyLen
 * @param value
 * @param valueLen
 * @param valueLenReq
 *
 * @return true indicating success, false indicating failure
 */
HTTPReturnCode HTTPStateInfo::GetHeaders
(
  uint32 RequestId,
  char *header,
  int headerLen,
  int& headerLenReq
)
{
  HTTPReturnCode Status = HTTP_FAILURE;

  Status = m_httpRequestHandler.GetRequestStatus(RequestId );
  if(Status == HTTP_SUCCESS)
  {
    (void)m_HTTPResponse.GetHeaders(header, headerLen, headerLenReq);
  }
  else
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Get Request Status Failed:%d for RequestID:%u",
                  Status, RequestId);
  }
  return Status;
}

/**
 * @brief This Method get the cookie headers value list reference from HTTPresponse
 *
 * @param requestId
 * @param key
 * @return true indicating success, false indicating failure
 */
HTTPReturnCode HTTPStateInfo::GetCookieHeaderValueList
(
 uint32 RequestId,
 ordered_StreamList_type **cookieList
)
{
  HTTPReturnCode Status = HTTP_FAILURE;

  Status = m_httpRequestHandler.GetRequestStatus(RequestId );
  if(Status == HTTP_SUCCESS)
  {
    (void)m_HTTPResponse.GetCookieHeaderValueList(cookieList);
  }
  else
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,"Get Request Status Failed:%d for RequestID:%u",
                  Status, RequestId);
  }
  return Status;
}

/**
 * @brief This Method retuns the total content length of the
 *        entity boby of respone of the  request
 *
 * @param RequestId
 *
 * @return 0 indicating that the requestId is not the
 *         current/valid request, > 0 if the content length of
 *         response is valid, -1 if response doesnt have content
 *         length
 */
int64 HTTPStateInfo::GetTotalContentLength(uint32 RequestId)
{
  int64 TotalContentLength = 0;
  HTTPReturnCode RqstStatus = HTTP_FAILURE;

  RqstStatus = m_httpRequestHandler.GetRequestStatus(RequestId );
  if(RqstStatus == HTTP_SUCCESS )
  {
    TotalContentLength = m_HTTPResponse.GetTotalContentLength();
  }
  return TotalContentLength;
}

/**
 * @brief This Method retuns the  actual content length of the
 *        entity boby of respone as per the partial/complete
 *        byte request in the response
 *
 * @param RequestId
 *
 * @return 0 indicating that the requestId is not the
 *         current/valid request, > 0 if the content length of
 *         response is valid, -1 if response doesnt have content
 *         length
 */
int64 HTTPStateInfo::GetContentLength(uint32 RequestId)
{
  int64 ContentLength = -1;
  HTTPReturnCode RqstStatus = HTTP_FAILURE;

  RqstStatus = m_httpRequestHandler.GetRequestStatus(RequestId );
  if(RqstStatus == HTTP_SUCCESS )
  {
    ContentLength = m_HTTPResponse.GetContentLength();
  }
  return ContentLength;
}

/**
 * @brief This Method provides the content type of the entity
 *        body of the response
 *
 * @return NULL is the requestID is not valid , otherwise
 *         Contenttype String
 */
const char *HTTPStateInfo::GetContentType(uint32 requestId)
{
  char *contentType = NULL;

  HTTPReturnCode RqstStatus = HTTP_FAILURE;

  RqstStatus = m_httpRequestHandler.GetRequestStatus(requestId );
  if(RqstStatus == HTTP_SUCCESS )
  {
    contentType = (char *)m_HTTPResponse.GetContentType();
  }
  return contentType;

}

/**
 * @brief This Method checks if the TCPConnection can be reused
 *
 * @param bcanReuse
 *
 * @return HTTP_SUCCESS indicating success, HTTP_FAILURE
 *         indicating failure
 */
HTTPReturnCode HTTPStateInfo::CanReuseConnection(bool &bcanReuse)
{
  HTTPReturnCode result = HTTP_SUCCESS;
  QTV_NULL_PTR_CHECK(m_httpRequestHandler.GetHostName(), HTTP_FAILURE);
  bool canReuseConnection = false;

  if(m_Connection)
  {
    const char *oldHostName = m_Connection->GetHostName();
    unsigned int oldPort = m_Connection->GetPort();

    if(m_Connection->IsConnected() &&
       (oldHostName != NULL) &&
       (oldPort != m_httpRequestHandler.GetPort()&&
       (true == m_IsPreviousConnectionPersistent)))
    {
      const char *nwhostname = m_httpRequestHandler.GetHostName();
      if ((nwhostname) && (0 == std_strcmp(oldHostName, nwhostname)))
      {
        canReuseConnection = true;
      }
    }
  }
  bcanReuse = canReuseConnection;
  return result;
}

/**
 * @brief This Method Initiates TCP connection
 *
 * @param ProxyServer
 * @param ProxyServerPort
 *
 * @return HTTP_SUCCESS indicating success, HTTP_FAILURE
 *         indicating failure, HTTP_WAIT indicating connection
 *         establishment is in progress
 */
HTTPReturnCode HTTPStateInfo::InitializeConnection
(
  CStreamNetwork *m_pCStreamNetwork,
  const char *ProxyServer,
  uint16 ProxyServerPort
)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if(m_Connection == NULL)
  {
    if( m_pCStreamNetwork )
    {
      TransportConnection::ResultCode rsltCode = TransportConnection::EFAILED;
      if(ProxyServer && ProxyServerPort > 0 )
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_LOW,
                       "Using Proxy server:%s:%d",ProxyServer,ProxyServerPort);

        m_Connection = QTV_New_Args(TransportConnectionTcp,
                                    (rsltCode,
                                     m_pCStreamNetwork,
                                     ProxyServer,
                                     std_strlen(ProxyServer),
                                     ProxyServerPort,
                                     m_SocketRcvBufSizeMax,
                                     m_SocketRcvBufSize,
                                     m_SocketSendBufSize));
      }
      else
      {
         const char *hostName = m_httpRequestHandler.GetHostName();

         if(hostName)
         {
           m_Connection = QTV_New_Args(TransportConnectionTcp,
                                      (rsltCode,
                                       m_pCStreamNetwork,
                                       hostName,
                                       std_strlen(hostName),
                                       m_httpRequestHandler.GetPort(),
                                       m_SocketRcvBufSizeMax,
                                       m_SocketRcvBufSize,
                                       m_SocketSendBufSize));
         }

      }

      if(m_Connection)
      {
        result = HTTP_SUCCESS;
      }
    }
  }
  else
  {
    // reusable persistent connection.
    // HTTPStack State machine remains in connected state.
    result = HTTP_SUCCESS;
  }
  return result;
}

/**
 * @brief This Method resets the TCPconnection i.e closes
 *        Connections and Deletes the connection object
 *
 * @return True on success,false on failure
 */
HTTPReturnCode HTTPStateInfo::ResetConnection()
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  if(m_Connection)
  {
    if(m_Connection->Close() == TransportConnection::SUCCESS)
    {
      QTV_Delete(m_Connection);
      m_Connection = NULL;
      rsltCode = HTTP_SUCCESS;
    }
  }

  return rsltCode;
}

/**
 * @brief This Method Gets Netpolicy in use
 *
 * @param net_policy
 *
 * @return  True on success,false on failure
 */
bool HTTPStateInfo::GetNetPolicy(NetPolicyInfo* net_policy)
{
  bool result = false;
  if(m_Connection)
  {
    result = m_Connection->GetNetPolicy(net_policy);
  }
  return result;
}

/**
 * @brief This Method sets the netpolicy with the network
 *        subsytem via connection object
 *
 * @param net_policy
 *
 * @return True on success,false on failure
 */
bool HTTPStateInfo::SetNetPolicy(NetPolicyInfo* net_policy)
{
  bool result = false;
  if(m_Connection)
  {
    result = m_Connection->SetNetPolicy(net_policy);
  }
  return result;
}

HTTPReturnCode HTTPStateInfo::GetSockOpt(HTTPStackOption optionType, int& value)
{
  HTTPReturnCode ret = HTTP_FAILURE;

  switch(optionType)
  {
     case HTTP_SOCKET_RECV_BUF_SIZE:
       ret = HTTP_SUCCESS;
       break;
     case HTTP_SOCKET_SEND_BUF_SIZE:
       ret = HTTP_SUCCESS;
       break;
     default:
       QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "SetSockOption: Unknown socket option %d", optionType);
       break;
  }

  if(HTTP_SUCCESS == ret)
  {
    if(m_Connection)
    {
      TransportConnection::ResultCode r = m_Connection->GetSockOpt(optionType, value);
      switch(r)
      {
         case TransportConnection::SUCCESS:
           ret = HTTP_SUCCESS;
           break;
         case TransportConnection::EWAITING:
           ret = HTTP_WAIT;
           break;
         default:
           ret = HTTP_FAILURE;
           break;
      }
    }
    else
    {
      ret = HTTP_WAIT;
    }
  }

  return ret;
}

HTTPReturnCode HTTPStateInfo::SetSockOpt(HTTPStackOption optionType, int value)
{
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "SetSockOption: Socket option %d, value %d", optionType, value);

  HTTPReturnCode ret = HTTP_FAILURE;

  switch(optionType)
  {
     case HTTP_SOCKET_RECV_BUF_SIZE:
       m_SocketRcvBufSize = value;
       m_SocketRcvBufSizeMax =
         STD_MAX(m_SocketRcvBufSizeMax, m_SocketRcvBufSize);
       ret = HTTP_SUCCESS;
       break;
     case HTTP_SOCKET_SEND_BUF_SIZE:
       m_SocketSendBufSize = value;
       ret = HTTP_SUCCESS;
       break;
     default:
       QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "SetSockOption: Unknown socket option %d", optionType);
       break;
  }

  if(HTTP_SUCCESS == ret)
  {
    if(m_Connection)
    {
      TransportConnection::ResultCode r = m_Connection->SetSockOpt(optionType, value);
      switch(r)
      {
         case TransportConnection::SUCCESS:
           ret = HTTP_SUCCESS;
           break;
         case TransportConnection::EWAITING:
           ret = HTTP_WAIT;
           break;
         default:
           ret = HTTP_FAILURE;
           break;
      }
    }
    else
    {
      ret = HTTP_WAIT;
    }
  }

  return ret;
}

/**
 * @brief This Method checks if the transport connection is
 *        established
 *
 * @return True is TCP connection established, false otherwise
 */
bool HTTPStateInfo::IsConnected()
{
  bool result = false;
  if(m_Connection)
  {
    result = m_Connection->IsConnected();
  }
  return result;

}

/**
 * @brief This Method resets the PersistentConnection falg to
 *        false
 */
void HTTPStateInfo::ResetPersistentConnection()
{
  m_IsPreviousConnectionPersistent = false;
}

/**
 * @brief This Method open TCP connection with the server
 *
 * @return HTTP_SUCCESS on successfull connection establishment,
 *         HTTP_WAIT if Connection is not established,
 *         HTTP_FAILURE on any failure
 */
HTTPReturnCode HTTPStateInfo::OpenConnection()
{
  HTTPReturnCode rslt = HTTP_FAILURE;

  if(m_Connection)
  {
    TransportConnection::ResultCode rsltCode = m_Connection->Open();
    rslt = HTTPStackCommon::MapTransportResultCode(rsltCode);
  }

  return rslt;
}

/**
 * @brief This Method retreives the last error code from
 *        connection object
 *
 * @return int
 */
int HTTPStateInfo::GetLastError()
{
  int rslt = -1;

  if(m_Connection)
  {
    rslt = m_Connection->GetLastError();
  }
  return rslt;
}

/**
 * @brief This Method Flushes the headers for a request
 *
 * @param RequestId
 */
HTTPReturnCode HTTPStateInfo::FlushRequest(uint32 RequestId)
{
  return m_httpRequestHandler.FlushRequest(RequestId);
}

/**
 * @brief This Method sets content length of response to invalid
 *
 * @param value
 */
void HTTPStateInfo::SetInvalidContentLengthFlag(bool value)
{
  m_HTTPResponse.SetInvalidContentLengthFlag(value);
}

/**
 * @ This Method provides ReasonPhrase string from th response
 *
 * @param requestId
 *
 * @return Pointer to string containing reason phrase
 */
const char* HTTPStateInfo::GetReasonPhraseStr(uint32 requestId)
{
  char *reasonPhraseStr = NULL;

  HTTPReturnCode RqstStatus = HTTP_FAILURE;

  RqstStatus = m_httpRequestHandler.GetRequestStatus(requestId );
  if(RqstStatus == HTTP_SUCCESS )
  {
     reasonPhraseStr =(char *) m_HTTPResponse.GetReasonPhraseStr();
  }
  return reasonPhraseStr;
}

/**
 * @ This Method provides response code from the response
 *
 * @param requestId
 *
 * @return interger response code
 */
int HTTPStateInfo::GetHTTPResponseCode(uint32 requestId)
{
  int responseCode = 0;

  HTTPReturnCode RqstStatus = HTTP_FAILURE;

  RqstStatus = m_httpRequestHandler.GetRequestStatus(requestId );
  if(RqstStatus == HTTP_SUCCESS )
  {
     responseCode = m_HTTPResponse.GetHTTPResponseCode();
  }
  return responseCode;
}

const char* HTTPStateInfo::GetEntityBody(uint32 requestId)
{
  char * entityBody = NULL;

  HTTPReturnCode RqstStatus = HTTP_FAILURE;

  RqstStatus = m_httpRequestHandler.GetRequestStatus(requestId );
  if(RqstStatus == HTTP_SUCCESS )
  {
     entityBody = (char*)m_HTTPResponse.GetEntityBody();
  }
  return entityBody;
}

void HTTPStateInfo::Reset()
{
  m_httpRequestHandler.ResetAllRequests();
  m_HTTPResponse.Reset();
  m_HTTPRspStatusHandler.Reset();
}

HTTPReturnCode HTTPStateInfo::HandleResponseStatus
(
  uint32 requestId,
  HTTPStackHelper& httpStackHelper
)
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  int rspStatusCode = m_HTTPResponse.GetHTTPResponseCode();

  if (false == m_HTTPRspStatusHandler.IsInitialized())
  {
    // initialize the statusHandler once for every response.
    m_HTTPRspStatusHandler.SetStatusHandler(rspStatusCode);

    if (true == m_httpRequestHandler.IsRequestRspHrdReceived(requestId))
    {
      m_IsPreviousConnectionPersistent = m_HTTPResponse.IsConnectionPersistent();
    }
  }

  rsltCode = m_HTTPRspStatusHandler.HandleResponseStatus(requestId,
                                                         rspStatusCode,
                                                         httpStackHelper);

  return rsltCode;
}

/**
 * @brief This method indicates if the response headers are
 *        received fully
 *
 * @param requestId
 *
 * @return true indicating that headers are fullr received ,
 *         false otherwise
 */
bool HTTPStateInfo::IsResponseHeaderFullyReceived(uint32 requestId)
{
  bool rsltCode = false;

  HTTPReturnCode RqstStatus = HTTP_FAILURE;

  RqstStatus = m_httpRequestHandler.GetRequestStatus(requestId );
  if(RqstStatus == HTTP_SUCCESS )
  {
    rsltCode = m_HTTPResponse.IsResponseHeaderFullyReceived();
  }

  return rsltCode;
}

bool HTTPStateInfo::IsRequestRspHeaderReceived(uint32 RequestID)
{
  return m_httpRequestHandler.IsRequestRspHrdReceived(RequestID);
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
bool HTTPStateInfo::IsRequestSent(const uint32 requestId)
{
  return m_httpRequestHandler.IsRequestSent(requestId);
}

bool HTTPStateInfo::IsRequestPartiallyOrFullySent(const uint32 requestId)
{
  return m_httpRequestHandler.IsRequestPartiallyOrFullySent(requestId);
}

/**
 * @brief This method resets HTTPRequest and connection for
 *        reconnect
 *
 * @param requestId
 *
 * @return HTTP_SUCCESS if it is success, HTTP_FAILURE If there
 *         is a failure
 */
HTTPReturnCode HTTPStateInfo::HandleReconnect( uint32 requestId )
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  rsltCode = m_httpRequestHandler.PrepareForReconnect(requestId);

  if(rsltCode == HTTP_SUCCESS)
  {
    if (m_HTTPResponse.IsDownloadingData())
    {
      char cCurrRange[32] = {0};
      int nRangeLen = (int)sizeof(cCurrRange);
      rsltCode = HTTP_FAILURE;
      QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                    "HTTPStateInfo::HandleReconnect Error in middle of downloading" );

      //Based on the Range header of the request that met with an ERROR, construct
      //the Range header for the new request
      if (m_httpRequestHandler.GetHeader(requestId,
                                         HTTPStackCommon::RANGE_KEY,
                                         (int)std_strlen(HTTPStackCommon::RANGE_KEY),
                                         cCurrRange, nRangeLen))
      {
        const char* pVal = std_strchr(cCurrRange, '=');
        if (pVal++)
        {
          char cNewRange[32] = {0};
          uint64 nNumBytesDownloaded = m_HTTPResponse.GetNumBytesRead();
          int64 nStart = 0;
          int64  nEnd = -1;

          if (*pVal == '-')
          {
            //DASH client doesn't use this, but added for completeness!
            nEnd = atoi(++pVal);
            nEnd -= nNumBytesDownloaded;
            if (nEnd > 0)
            {
              rsltCode = HTTP_SUCCESS;
              (void)std_strlprintf(cNewRange, sizeof(cNewRange), "bytes=-%lld", nEnd);
            }
            else
            {
              //Should not happen, but this means the current request is entirely served!
              rsltCode = HTTP_NOMOREDATA;
            }
          }
          else
          {
            nStart = atoi(pVal);
            pVal = std_strchr(pVal, '-');
            if (pVal++)
            {
              rsltCode = HTTP_SUCCESS;
              if (std_strlen(pVal) > 0)
              {
                nEnd = atoi(pVal);
              }

              nStart += nNumBytesDownloaded;
              if (nEnd >= 0)
              {
                if (nEnd >= nStart)
                {
                  (void)std_strlprintf(cNewRange, sizeof(cNewRange), "bytes=%lld-%lld",
                                       nStart, nEnd);
                }
                else
                {
                  //Should not happen, but this means the current request is entirely served!
                  rsltCode = HTTP_NOMOREDATA;
                }
              }
              else
              {
                //Open-ended request
                (void)std_strlprintf(cNewRange, sizeof(cNewRange), "bytes=%lld-", nStart);
              }
            }
          }// if (*pVal == '-')

          if (rsltCode == HTTP_SUCCESS)
          {
            (void)m_httpRequestHandler.RemoveHeader(requestId,
                                                    HTTPStackCommon::RANGE_KEY,
                                                    (int)std_strlen(HTTPStackCommon::RANGE_KEY));
            if (!m_httpRequestHandler.SetHeader(requestId,
                                                HTTPStackCommon::RANGE_KEY,
                                                (int)std_strlen(HTTPStackCommon::RANGE_KEY),
                                                cNewRange, (int)std_strlen(cNewRange)))
            {
              rsltCode = HTTP_FAILURE;
            }
            QTV_MSG_SPRINTF_PRIO_1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                                    "HTTPStateInfo::HandleReconnect cNewRange %s", cNewRange );
          }
        }
      }// if (m_httpRequestHandler.GetHeader())
    }// if (m_HTTPResponse.IsDownloadingData())

    if (rsltCode == HTTP_SUCCESS)
    {
      (void)ResetConnection();
    }
  }

  return rsltCode;
}

/**
 * Check if request should be retried on connection error
 */
bool HTTPStateInfo::ShouldRetry(uint32 nRequestId)
{
  return m_httpRequestHandler.ShouldRetry(nRequestId);
}

uint32 HTTPStateInfo::GetNumPendingRequestTobeSent()
{
  return m_httpRequestHandler.GetNumPendingRequestTobeSent();
}

/**
 * @brief This Method closes the TCP connection, deletes all the
 *        HTTPReuquests from the Queue
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode HTTPStateInfo::CloseConnection()
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  m_httpRequestHandler.Reset();
  rsltCode = CloseConnectionInternal();
  return rsltCode;
 }

HTTPReturnCode HTTPStateInfo::CloseConnectionInternal()
  {
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  m_HTTPResponse.Reset();
  rsltCode = ResetConnection();
  return rsltCode;
}

HTTPReturnCode HTTPStateInfo::SetProxyInfo
(
  const char *proxyServer,
  unsigned short proxyPort
 )
{
   return  m_httpRequestHandler.SetProxyInfo(proxyServer,proxyPort);
}

} /* video namespace */
