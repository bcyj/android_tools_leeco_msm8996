/************************************************************************* */
/**
 * HTTPStackHelper.cpp
 * @brief implementation of the HTTPStackHelper.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackHelper.cpp#45 $
$DateTime: 2013/07/28 21:38:43 $
$Change: 4175800 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

// HTTPStack
#include "HTTPResponseStatusHandler.h"
#include "HTTPStackHelper.h"
#include "HTTPStackCommon.h"
#include "HTTPStackStateObjects.h"

// CommonUtils
#include "SourceMemDebug.h"
#include "qtv_msg.h"
#include "TransportConnectionTcp.h"

namespace video
{

void HTTPStackHelper::SetNetAbort()
{
  if(NULL != m_pCStreamNetwork)
  {
    m_pCStreamNetwork->abort();
  }
  return;
}

void
HTTPStackHelper::SetState(HTTPStateBase *state)
{
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
    "HTTPStackHelper::SetState. Transition from '%s' to '%s'",
    m_HTTPState->GetStateName(),
    state->GetStateName());

  m_HTTPState = state;

  // Update state information

  if (&(HTTPStackStateObjects::HTTPStateIdleObj) == state)
  {
    m_HTTPStateInfo.ResetPersistentConnection();
    StartNewRequest();
    ResetOptions();
  }
  else
  if (&(HTTPStackStateObjects::HTTPStateClosedObj) == state)
  {
    m_HTTPStateInfo.ResetPersistentConnection();
  }
}

HTTPStackHelper::HTTPStackHelper(
  void *pOwner,
  NotifyCallback NotifyCb,
  HTTPCookieMgr& cookieMgr) :
    m_HTTPStateInfo(cookieMgr),
    m_ProxyServerName(NULL),
    m_ProxyServerPort(0),
    m_HTTPState(&(HTTPStackStateObjects::HTTPStateIdleObj)),
    m_pCStreamNetwork(NULL),
    m_pOwner(pOwner),
    m_fNotifyCallback(NotifyCb)
{
  ResetOptions();

  m_NetworkIfaceId = -1;
  m_PrimaryPDPProfileNo = -1;

  m_pCStreamNetwork = CStreamNetwork::CreateInstance(true);
  if (NULL == m_pCStreamNetwork)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "HTTPStackHelper Failed to create instance of m_pCStreamNetwork");
  }
}

HTTPStackHelper::~HTTPStackHelper()
{
  Destroy();
}

void
HTTPStackHelper::SetHTTPStackPtr(HTTPAuthorizationInterface *pHTTPStack)
{
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "SetHTTPStackPtr 0x%p to be passed in callback", (void*)pHTTPStack);
  m_pHTTPStack = pHTTPStack;
}

void
HTTPStackHelper::Destroy()
{
  if (NULL != m_pCStreamNetwork)
  {
    QTV_Delete(m_pCStreamNetwork);
    m_pCStreamNetwork = NULL;
  }
  if (m_ProxyServerName)
  {
    QTV_Free(m_ProxyServerName);
    m_ProxyServerName = NULL;
  }
}


HTTPReturnCode
HTTPStackHelper::CloseConnection ()
{
  HTTPReturnCode result = m_HTTPStateInfo.CloseConnection();

  SetState(&(HTTPStackStateObjects::HTTPStateIdleObj));
  return result;
}

HTTPReturnCode HTTPStackHelper::CreateRequest(uint32& requestId)
{
  return m_HTTPStateInfo.CreateRequest(requestId);
}

HTTPReturnCode HTTPStackHelper::DeleteRequest(uint32 requestId)
{
  HTTPReturnCode result = HTTP_FAILURE;
  bool bInternalClose = false;
  result =  m_HTTPStateInfo.DeleteRequest(requestId, bInternalClose);

  if(result == HTTP_SUCCESS && bInternalClose)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,"InternalCloseConnection");
    m_HTTPStateInfo.CloseConnectionInternal();
    SetState(&(HTTPStackStateObjects::HTTPStateClosedObj));
  }

  return result;
}

HTTPReturnCode
HTTPStackHelper::SetHeader(uint32 requestId,
                           const char * key,
                           int keyLen,
                           const char *value,
                           int valueLen)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if (NULL == m_HTTPState)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPStackHelper::SetHeader : m_HTTPState is NULL");
  }
  else
  {
    bool isHeaderSet = m_HTTPStateInfo.SetHeader(requestId,
      key, keyLen, value, valueLen);

    result = (true == isHeaderSet ? HTTP_SUCCESS : HTTP_FAILURE);
  }

  return result;
}

HTTPReturnCode
HTTPStackHelper::UnsetHeader(uint32 /*requestId*/,
                             const char * /* key */,
                               int /* keyLen */)
{
  return HTTP_NOTSUPPORTED;
}


HTTPReturnCode
HTTPStackHelper::FlushRequest(uint32 requestId)
{
  return m_HTTPStateInfo.FlushRequest(requestId);
}

HTTPReturnCode
HTTPStackHelper::SendRequest(uint32 requestId,
                             HTTPMethodType method,
                             const char *relativeUrl,
                             int relativeUrlLen)
{
  HTTPReturnCode result = HTTP_FAILURE;

  /* If request was already sent/sending and SendRequest() comes in with the same requestId
   * (ex usecase: In case of HTTP1.0 server, we first try with original URL and retry with ByteRangeURL
   * using the same requestId). In such scenario, resetconnection and reset stateinfo to
   * mark all as requests as REQUEST_WAITING_TO_SEND.
   *
   * This ensures req/resp order is preserved. ex: R1-R2-R3 will remain R1'-R2-R3 where R1'=R1 is the
   * new byterangeURl request. If connection and state info was not reset, order will be R2-R3-R1'.
   */
  if(m_HTTPStateInfo.IsRequestPartiallyOrFullySent(requestId))
  {
    m_HTTPStateInfo.ResetConnection();
    m_HTTPStateInfo.Reset();
    SetState(&HTTPStackStateObjects::HTTPStateClosedObj);
  }

  result = m_HTTPStateInfo.SetRequest(requestId,
                                      method,
                                      relativeUrl,
                                      relativeUrlLen,
                                      m_ProxyServerName);

  if (result == HTTP_SUCCESS)
  {
    if ((&(HTTPStackStateObjects::HTTPStateIdleObj) == m_HTTPState) ||
        (&(HTTPStackStateObjects::HTTPStateClosedObj) == m_HTTPState))
    {
      result = SendRequestInternal(requestId);
    }
    else if (&(HTTPStackStateObjects::HTTPStateConnectedObj) == m_HTTPState )
    {
      bool bCreateNewConnection = false;
      result = m_HTTPStateInfo.SendPendingRequests(bCreateNewConnection);

      // if the head request that have been queued recently needs stack to connect to new server,
      //  close the current connection and bring up new connection
      if (result == HTTP_FAILURE && bCreateNewConnection)
      {
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,"Creating New connection,result:%d,bCreate :%d,requestId:%u",
                      result,bCreateNewConnection,requestId);
        result = SendRequestInternal(requestId);
      }
      else if(result == HTTP_FAILURE ||
              result == HTTP_WAIT )
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,"SendPendingRequests Status :%d, For socket errors Reconnect will resend request",
                                                            result);
        result = HTTP_SUCCESS;
      }
    }
  }

  return result;
}

HTTPReturnCode HTTPStackHelper::IsResponseReceived(uint32 requestId)
{
  HTTPReturnCode status = HTTP_FAILURE;

  // poll
  status = m_HTTPState->IsResponseReceived(requestId, *this, NULL);

  // if response hdrs are fully received then return SUCCESS.
  if (HTTP_WAIT != status)
  {
    if (m_HTTPStateInfo.IsRequestRspHeaderReceived(requestId))
    {
      status = HTTP_SUCCESS;
    }
  }

  return status;
}

HTTPReturnCode
HTTPStackHelper::GetHeader(uint32 requestId,
                           const char *key,
                           int keyLen,
                           char *value,
                           int valueLen,
                           int *valueLenReq)
{
  return m_HTTPStateInfo.GetHeaderValue(requestId,key, keyLen, value,
                                        valueLen, valueLenReq);
}

HTTPReturnCode HTTPStackHelper::GetData(uint32 requestId, char *readBuf, size_t readBufSize, size_t *readLen)
{
  GetDataArgument getDataArg(readBuf, readBufSize, readLen);
  HTTPReturnCode result = m_HTTPState->GetData(requestId,*this, &getDataArg);

  return result;
}

HTTPReturnCode HTTPStackHelper::SetProxyServer (const char* proxyServer, size_t proxyServerLen)
{
  HTTPReturnCode result = HTTP_FAILURE;

  char *localProxyUrl = NULL;

  if (proxyServerLen > (size_t)HTTPStackCommon::MAX_PROXY_URL_LEN)
  {
    result = HTTP_BADPARAM;

    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPStackHelper::SetProxyServer : Invalid proxyServerLen '%d'",
       proxyServerLen);
  }
  else
  {
    localProxyUrl = (char *)QTV_Malloc( (proxyServerLen + 1) * sizeof(char));
    if (NULL == localProxyUrl)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPStackHelper::SetProxyServer : Failed to allocate localProxyUrl");
    }
    else
    {
      std_strlcpy(localProxyUrl, proxyServer, proxyServerLen + 1);
      localProxyUrl[proxyServerLen] = 0;

      if (NULL != m_ProxyServerName)
      {
        QTV_Free(m_ProxyServerName);
        m_ProxyServerName = NULL;
      }

      m_ProxyServerName = (char *)QTV_Malloc((proxyServerLen + 1) * sizeof(char));
      if (NULL == m_ProxyServerName)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPStackHelper::SetProxyServer : Failed to allocate m_ProxyServerName");
      }
      else
      {
        result = HTTP_SUCCESS;
        char *posPort = std_strstr(localProxyUrl, ":");
        if (NULL == posPort)
        {
          m_ProxyServerPort = HTTPStackCommon::DEFAULT_HTTP_PORT;
          std_strlcpy(m_ProxyServerName, localProxyUrl, (proxyServerLen + 1));
        }
        else
        {
          int32 m_temp_ProxyServerPort = atoi(posPort+1);
          if (!m_temp_ProxyServerPort)
          {
            // not able to parse
            QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
             "HTTPStackHelper::SetProxyServer - Error in  parsing");
          }
          else
          {
            m_ProxyServerPort = (unsigned short)m_temp_ProxyServerPort;
          }
          *posPort = '\0';
          std_strlcpy(m_ProxyServerName, localProxyUrl, proxyServerLen + 1);
        }
      } // end if (m_ProxyServerName...)

    } // end if(localProxyUrl...)

    if (localProxyUrl)
    {
      QTV_Free(localProxyUrl);
      localProxyUrl = NULL;
    }
  }

  if (result == HTTP_SUCCESS)
  {
    result = m_HTTPStateInfo.SetProxyInfo(m_ProxyServerName,m_ProxyServerPort);
  }
  return result;
}



/**
 *
 * @brief Get Proxyserver info
 *
 * @param[out] ProxyServer
 * @param[in] ProxyServerLen
 * @param[out] ProxyServerLenReq
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode HTTPStackHelper::GetProxyServer
(
 char* ProxyServer,
 size_t ProxyServerLen,
 size_t &ProxyServerLenReq
)
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;
  ProxyServerLenReq = 0;

  if (m_ProxyServerName)
  {
    size_t servLen = std_strlen(m_ProxyServerName);
    if (servLen > 0)
    {
      size_t reqProxyStrLen = servLen + 7; //(7 characters assuming max port 65535,":","\0" )

      char *localProxyStr = (char *)QTV_Malloc((reqProxyStrLen) * sizeof(char));
      if (localProxyStr)
      {
        size_t offset = 0;
        offset += std_strlprintf(localProxyStr+offset,reqProxyStrLen-offset, "%s:%d",
                                 m_ProxyServerName,m_ProxyServerPort);

        size_t actualProxyStrLen = std_strlen(localProxyStr);
        if (actualProxyStrLen > 0)
        {
          if (ProxyServer != NULL && ProxyServerLen > 0 )
          {
            if (ProxyServerLen > actualProxyStrLen)
            {
              std_strlcpy(ProxyServer,localProxyStr,ProxyServerLen);
              ProxyServerLenReq = actualProxyStrLen ;
              rsltCode = HTTP_SUCCESS;
            }
            else if (ProxyServerLen <= actualProxyStrLen )
            {
              ProxyServerLenReq = actualProxyStrLen+1 ;
              rsltCode = HTTP_BADPARAM;
            }
          }
          else if(ProxyServer == NULL && ProxyServerLen == 0)
          {
            ProxyServerLenReq = actualProxyStrLen ;
            rsltCode = HTTP_SUCCESS;
          }
        }
        QTV_Free(localProxyStr);
        localProxyStr = NULL;
      }
    }
  }
  return rsltCode;
}


HTTPReturnCode
HTTPStackHelper::UnsetProxyServer()
{
  if (NULL != m_ProxyServerName)
  {
    QTV_Free(m_ProxyServerName);
    m_ProxyServerName = NULL;
  }

  m_ProxyServerPort = 0;

  return HTTP_SUCCESS;
}

HTTPReturnCode
HTTPStackHelper::SetSocketMode(bool /* flagIsBlockingSocket */)
{
  return HTTP_SUCCESS;
}

HTTPReturnCode HTTPStackHelper::GetResponseCode(/*in*/ uint32 RequestId, uint32& nVal)
{
  nVal =  m_HTTPStateInfo.GetHTTPResponseCode(RequestId);
  return (nVal > 0 ? HTTP_SUCCESS : HTTP_FAILURE);
}

HTTPReturnCode
HTTPStackHelper::GetContentLength (uint32 requestId, int64* ContentLength, bool bTotal)
{
  *ContentLength = (bTotal == true) ? m_HTTPStateInfo.GetTotalContentLength(requestId)
                                    : m_HTTPStateInfo.GetContentLength(requestId);
  return (*ContentLength >= 0 ? HTTP_SUCCESS : HTTP_FAILURE);
}

HTTPReturnCode
HTTPStackHelper::GetContentType(uint32 requestId,
                                char *contentType,
                                size_t contentTypeLen,
                                size_t *contentTypeLenReq)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if (NULL == m_HTTPState)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPStackHelper::GetContentType : m_HTTPState is NULL");
  }
  else
  {
    GetContentTypeArgument arg(contentType, contentTypeLen, contentTypeLenReq);

    const char *respContentType = m_HTTPStateInfo.GetContentType(requestId);
    if (NULL == contentType)
    {
      if(NULL != respContentType)
      {
        *contentTypeLenReq = std_strlen(respContentType) + 1;
        result = HTTP_SUCCESS;
      }
    }
    else
    {
      if (NULL != respContentType)
      {
        std_strlcpy(contentType, respContentType, contentTypeLen);
        result = HTTP_SUCCESS;
      }
    }
  }

  return result;
}

void
HTTPStackHelper::SetDisableAutoClose (bool value)
{
  m_HTTPStateInfo.SetInvalidContentLengthFlag(value);
}

bool
HTTPStackHelper::IsProcessingARequest()
{
  return m_HTTPStateInfo.IsProcessingARequest();
}

char*
HTTPStackHelper::GetIPAddr(int& size)
{
  return m_HTTPStateInfo.GetIPAddr(size);
}

HTTPReturnCode HTTPStackHelper::GetSockOpt(HTTPStackOption optionType, int& value)
{
  return m_HTTPStateInfo.GetSockOpt(optionType, value);
}

HTTPReturnCode HTTPStackHelper::SetSockOpt(HTTPStackOption optionType, int value)
{
  return m_HTTPStateInfo.SetSockOpt(optionType, value);
}

/**
 * @brief
 *  If the old host name and port in the current Connection
 *  object (if non null) match those in the latest http request
 *  and the stack is currently connected to peer and the
 *  connection is Keep-Alive per last server response, then the
 *  same connection object can be reused without tearing down
 *  TCP. Else a new connection object is created.
 *  If an old connection object exists which cannot be reused,
 *  it calls CLose before deleting the object.
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode
HTTPStackHelper::CreateOrReuseConnectionObject()
{
  HTTPReturnCode result = HTTP_FAILURE;

  bool canReuseConnection = false;

  result = m_HTTPStateInfo.CanReuseConnection(canReuseConnection);

  if (result == HTTP_SUCCESS && (false == canReuseConnection))
  {
    if ((&HTTPStackStateObjects::HTTPStateConnectedObj == m_HTTPState))
    {
      m_HTTPStateInfo.ResetConnection();
      SetState(&HTTPStackStateObjects::HTTPStateClosedObj);
    }

    result =  m_HTTPStateInfo.InitializeConnection(m_pCStreamNetwork,m_ProxyServerName,m_ProxyServerPort);

    if (result != HTTP_SUCCESS)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "Failed to create HTTP connection object");

      SetState(&HTTPStackStateObjects::HTTPStateErrorObj);
    }
    else
    {
      SetState(&HTTPStackStateObjects::HTTPStateConnectingObj);

      //set Network Interface and Profileno right after creating connection object
      if (!ConfigureNetPolicy())
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "Failed to Set NetPolicy ");
      }
    }
  }

  return result;
}

HTTPReturnCode
HTTPStackHelper::SendRequestInternal(uint32 requestId)
{
  HTTPReturnCode result = HTTP_FAILURE;

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                "HTTPStackHelper::SendRequestInternal(%u)",requestId);

  result = CreateOrReuseConnectionObject();

  if (HTTP_SUCCESS == result)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                 "HTTPStackHelper::SendRequest Request queued");
  }

  return result;
}

/**
 * @brief
 *  Call when sending a new request to reset request, response,
 *  response status handler.
 */
void HTTPStackHelper::StartNewRequest()
{
  m_HTTPStateInfo.Reset();
}

bool
HTTPStackHelper::IsProxyServerSet()
{
  return m_ProxyServerName ? true : false;
}


HTTPReturnCode
HTTPStackHelper::NotifyEvent(uint32 requestId, HTTPStackNotifyCode notifyCode)
{
  HTTPReturnCode rsltCode = HTTP_FAILURE;

  HTTPStackNotificationCbData cbData = {0,NULL,NULL,false,NULL,NULL,NULL};
  cbData.m_serverCode = m_HTTPStateInfo.GetHTTPResponseCode(requestId);
  cbData.m_serverMessage = (char*)m_HTTPStateInfo.GetReasonPhraseStr(requestId);
  cbData.m_entityBody = (char*)m_HTTPStateInfo.GetEntityBody(requestId);
  cbData.m_method = (char *)HTTPStackCommon::GetStringForMethod(m_HTTPStateInfo.GetRequestMethod(requestId));
  cbData.m_protocolHeaders = NULL;
  cbData.m_pHTTPStack = m_pHTTPStack;

  int reqLen = 0;

  /**
  * This commented code is in line with what is specified for
  * protocol header events. However, current decision is to send
  * protocol header event for authorization or porxy-authoirzation
  * only. If this changes then this can be uncommented and th the
  * code following this can be
  * removed.
  *
  * httpResponse.GetHeaders(NULL, 0, reqLen);
  *
  * if (reqLen > 0)
  *{
  * int bufSize = reqLen; cbData.m_protocolHeaders = (char
  *
  *  *)QTV_Malloc(bufSize * sizeof(char));
  *
  * if (cbData.m_protocolHeaders)
  * {
  *   httpResponse.GetHeaders(cbData.m_protocolHeaders,bufSize,
  *reqLen);
  *  }
  *  }
  *  else {
  *  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
  *
  *   "HTTPStackHelper::NotifyEvent Did not find headers for
  *   response. ServerCode %d", cbData.m_serverCode);
  *   }
  */

  static const int AUTHENTICATE_KEY_LEN = (int)std_strlen(HTTPStackCommon::AUTHENTICATE_KEY);
  static const int PROXY_AUTHENTICATE_KEY_LEN = (int)std_strlen(HTTPStackCommon::PROXY_AUTHENTICATE_KEY);
  static const char *DELIM = ": ";
  static const int DELIM_LEN = (int)std_strlen(DELIM);
  static const char *TERMINATE = "\r\n\r\n";
  static const int TERMINATE_LEN = (int)std_strlen(TERMINATE);

    m_HTTPStateInfo.GetHeaderValue(requestId,
    HTTPStackCommon::AUTHENTICATE_KEY, AUTHENTICATE_KEY_LEN,
    NULL, 0, &reqLen);

  if (reqLen > 0)
  {
    // response header contains "WWW-Authenticate"
    int reqdBufSize = AUTHENTICATE_KEY_LEN + DELIM_LEN + reqLen + TERMINATE_LEN + 1;
    cbData.m_protocolHeaders = (char *)QTV_Malloc(reqdBufSize * sizeof(char));
    if (cbData.m_protocolHeaders)
    {
      std_strlcpy(cbData.m_protocolHeaders, HTTPStackCommon::AUTHENTICATE_KEY, reqdBufSize);
      std_strlcat(cbData.m_protocolHeaders, DELIM, reqdBufSize);
      int tmpLen = reqLen;
      m_HTTPStateInfo.GetHeaderValue(requestId,
        HTTPStackCommon::AUTHENTICATE_KEY, AUTHENTICATE_KEY_LEN,
        cbData.m_protocolHeaders + AUTHENTICATE_KEY_LEN + DELIM_LEN, reqLen, &tmpLen);
      std_strlcat(cbData.m_protocolHeaders, TERMINATE, reqdBufSize);
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "cbData.m_protocolHeaders for authentication is NULL");
    }
  }
  else
  {
      m_HTTPStateInfo.GetHeaderValue(requestId,
      HTTPStackCommon::PROXY_AUTHENTICATE_KEY, PROXY_AUTHENTICATE_KEY_LEN,
      NULL, 0, &reqLen);

    if (reqLen > 0)
    {
      int reqdBufSize = PROXY_AUTHENTICATE_KEY_LEN + DELIM_LEN + reqLen + TERMINATE_LEN + 1;
      cbData.m_protocolHeaders = (char *)QTV_Malloc(reqdBufSize * sizeof(char));

      if (cbData.m_protocolHeaders)
      {
        std_strlcpy(cbData.m_protocolHeaders, HTTPStackCommon::PROXY_AUTHENTICATE_KEY, reqdBufSize);
        std_strlcat(cbData.m_protocolHeaders, DELIM, reqdBufSize);
        int tmpLen = reqLen;
        m_HTTPStateInfo.GetHeaderValue(requestId,
          HTTPStackCommon::PROXY_AUTHENTICATE_KEY, PROXY_AUTHENTICATE_KEY_LEN,
          cbData.m_protocolHeaders + PROXY_AUTHENTICATE_KEY_LEN + DELIM_LEN, reqLen, &tmpLen);
        std_strlcat(cbData.m_protocolHeaders, TERMINATE, reqdBufSize);
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "cbData.m_protocolHeaders for proxy-authentication is NULL");
      }
    }
  }

  rsltCode =  m_fNotifyCallback(requestId, notifyCode,&cbData, m_pOwner);

  return rsltCode;
}

void
HTTPStackHelper::ResetOptions()
{
  m_HTTPStateInfo.SetInvalidContentLengthFlag(false);
}

/**
 * @brief Sets the Network Interface
 *
 * @param networkIfaceId
 */
void HTTPStackHelper::SetNetworkInterface(int32 networkIfaceId)
{
  m_NetworkIfaceId = networkIfaceId;
}

/**
 *  @brief Gets the Network Interface
 *
 * @param networkIfaceId
 *
 * @return HTTPReturnCode
 */
HTTPReturnCode HTTPStackHelper::GetNetworkInterface(int32 &networkIfaceId)
{
  networkIfaceId = m_NetworkIfaceId;
  return HTTP_SUCCESS;
}


/**
 * @brief Sets the primaryPDPProfile no
 *
 * @param primaryProfileNo
 *
 * @return None
 */
void HTTPStackHelper::SetPrimaryPDPProfile(int32 primaryProfileNo)
{
  m_PrimaryPDPProfileNo = primaryProfileNo;
}

/**
 * @brief Gets the primaryPDPProfile no
 *
 * @param primaryProfileNo
 *
 * @return None
 */
HTTPReturnCode HTTPStackHelper::GetPrimaryPDPProfile(int32 &primaryProfileNo)
{
  primaryProfileNo = m_PrimaryPDPProfileNo;
  return HTTP_SUCCESS;
}

/**
 * @brief This methods sets the Netpolicy info:NetworkIface,
 *        PDPProfileNo, Address Family with the TCPConnection
 *        object
 *
 * @return bool
 */
bool HTTPStackHelper::ConfigureNetPolicy()
{
  bool retVal = false;
  bool bSetNetPolicy = false;
  NetPolicyInfo netPolicyInfo;

  std_memset(&netPolicyInfo, 0, sizeof(netPolicyInfo));

  if(m_HTTPStateInfo.GetNetPolicy(&netPolicyInfo))
  {
    netPolicyInfo.addrFamily = AF_INET;

    if(m_NetworkIfaceId >= 0 &&
       m_NetworkIfaceId != netPolicyInfo.ifaceName )
    {
      netPolicyInfo.ifaceName = m_NetworkIfaceId;
      bSetNetPolicy = true;
    }

    if(m_PrimaryPDPProfileNo >= 0 &&
       m_PrimaryPDPProfileNo != netPolicyInfo.primaryPDPProfileNo)
    {
      netPolicyInfo.primaryPDPProfileNo = m_PrimaryPDPProfileNo;
      bSetNetPolicy = true;
    }

    if(bSetNetPolicy)
    {
      if(false == m_HTTPStateInfo.SetNetPolicy(&netPolicyInfo))
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                      "Failed to Set NetPolicy ");
      }
      else
      {
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                      "Set NetPolicy succeeded,Iface:%d , PDPProfileNo:%d, Addressfamily:%d",
                      netPolicyInfo.ifaceName,netPolicyInfo.primaryPDPProfileNo,netPolicyInfo.addrFamily);
        retVal = true;
      }
    }
    else
    {
      QTV_MSG_PRIO3(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                    "Skipping setting NetPolicy, Values in use:iface:%d, Addrfamily:%d, pdpProfileNo:%d",
                    netPolicyInfo.ifaceName,netPolicyInfo.addrFamily,netPolicyInfo.primaryPDPProfileNo);
      retVal = true;
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                      "Failed to Retreive NetPolicy for comparision ");
  }

  return retVal;
}

} // end namespace video
