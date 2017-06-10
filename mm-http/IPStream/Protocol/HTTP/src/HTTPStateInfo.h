#ifndef HTTPSTATEINFO_H
#define HTTPSTATEINFO_H
/************************************************************************* */
/**
 * HTTPStateInfo.h
 * @brief Implements the HTTPStateInfo
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStateInfo.h#16 $
$DateTime: 2013/09/02 23:30:27 $
$Change: 4369104 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */



/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackCommon.h"
#include "HTTPRequestHandler.h"
#include "HTTPResponse.h"
#include "HTTPResponseStatusHandler.h"
#include "TransportConnection.h"
#include "HTTPCookieStore.h"

namespace video
{


class HTTPStateInfo
{
public:
  HTTPStateInfo(HTTPCookieMgr& cookieMgr);
  ~HTTPStateInfo();

  /** HTTPRequest Related Methods */
  HTTPReturnCode CreateRequest(uint32& requestId);
  HTTPReturnCode DeleteRequest(uint32 requestId,
                               bool &bInternalClose);
  bool SetHeader(uint32 RequestID,
                 const char *Key,
                 int KeyLen,
                 const char *Value,
                 int ValLen);
  HTTPReturnCode SetRequest(uint32 RequestID,
                            HTTPMethodType method,
                            const char *Url,
                            int UrlLen,
                            const char *proxyServer = NULL);
  HTTPReturnCode FlushRequest (uint32 RequestId);
  HTTPReturnCode SendPendingRequests(bool& bCreateNewConnection);
  bool IsProcessingARequest();
  HTTPMethodType GetRequestMethod(uint32 RequestID);
  const char* GetRequestUrl(uint32 RequestID);
  bool IsRequestRspHeaderReceived(uint32 RequestID);
  bool IsRequestSent(const uint32 requestId);
  bool IsRequestPartiallyOrFullySent(const uint32 requestId);
  uint32 GetNumPendingRequestTobeSent();

  /* Response related Methods*/
  HTTPReturnCode ReceiveResponse(uint32 RequestID);
  HTTPReturnCode GetData(uint32 RequestID,
                         char *readBuf,
                         size_t readBufSize,
                         size_t& readLen) ;
  HTTPReturnCode GetHeaderValue(uint32 requestId,
                                const char *key,
                                int keyLen,
                                char *value,
                                int valueLen,
                                int *valueLenReq);
  HTTPReturnCode GetHeaders(uint32 RequestId,
                            char *header,
                            int headerLen,
                            int &headerLenReq);

  HTTPReturnCode GetCookieHeaderValueList(uint32 RequestId,
                                          ordered_StreamList_type **cookieList);
  int64 GetTotalContentLength(uint32 requestId);
  int64 GetContentLength(uint32 requestId);
  const char *GetContentType(uint32 requestId);
  void SetInvalidContentLengthFlag(bool value);
  const char* GetReasonPhraseStr(uint32 requestId);
  int GetHTTPResponseCode(uint32 requestId);
  bool IsResponseHeaderFullyReceived(uint32 requestId);
  const char* GetEntityBody(uint32 requestId);

  HTTPReturnCode HandleResponseStatus(uint32 requestId, HTTPStackHelper& httpStackHelper);

  /* TCPConnection related Methods*/
  HTTPReturnCode CanReuseConnection(bool &bcanReuse);
  HTTPReturnCode InitializeConnection(CStreamNetwork *m_pCStreamNetwork,const char *ProxyServer,uint16 ProxyServerPort);
  bool IsConnected();
  HTTPReturnCode ResetConnection();
  bool GetNetPolicy(NetPolicyInfo* net_policy);
  bool SetNetPolicy(NetPolicyInfo* net_policy);
  HTTPReturnCode GetSockOpt(HTTPStackOption optionType, int& value);
  HTTPReturnCode SetSockOpt(HTTPStackOption optionType, int value);
  void ResetPersistentConnection();
  HTTPReturnCode OpenConnection();
  int GetLastError();
  HTTPReturnCode CloseConnection();
  HTTPReturnCode CloseConnectionInternal();

  void Reset();
  HTTPReturnCode HandleReconnect( uint32 requestId );
  bool ShouldRetry(uint32 nRequestId);

  void SetCookieReqHeader(uint32 requestId,
                          HTTPMethodType method,
                          const char *Url,
                          int UrlLen);
  void StoreRespCookies(uint32 requestId, char *Url);

  char* GetIPAddr(int& size);
  HTTPReturnCode SetProxyInfo(const char *proxyServer, unsigned short proxyPort);
private:

  void convertRspStatusToCommonStatus(const HTTPResponseStatus rspStatus,
                                      HTTPReturnCode &httpReturnCode);

  friend class HTTPResponseStatusHandler_100;
  friend class HTTPResponseStatusHandler_200;
  friend class HTTPResponseStatusHandler_300;
  friend class HTTPResponseStatusHandler_400;
  friend class HTTPResponseStatusHandler_500;
  friend class HTTPResponseStatusHandler;

  /**
   * True if "Connection: Keep-Alive" was set in previous HTTP response.
   */
  bool m_IsPreviousConnectionPersistent;

  HTTPRequestHandler m_httpRequestHandler;
  HTTPResponse m_HTTPResponse;
  HTTPRspStatusHandler m_HTTPRspStatusHandler;

  TransportConnection *m_Connection;

  int m_SocketSendBufSize;
  int m_SocketRcvBufSize;
  // set option with size before tcp connect
  int m_SocketRcvBufSizeMax;

  HTTPCookieMgr& m_CookieMgr;
};


}

#endif /* HTTPSTATEINFO_H*/
