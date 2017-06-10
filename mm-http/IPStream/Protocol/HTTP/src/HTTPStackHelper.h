#ifndef HTTPSTACKHELPER_H
#define HTTPSTACKHELPER_H
/************************************************************************* */
/**
 * HTTPStackHelper.h
 * @brief Implements the CmHTTPStackHelper
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackHelper.h#31 $
$DateTime: 2013/07/11 15:46:28 $
$Change: 4081743 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */



/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackInterface.h"

// HTTPStack
#include "HTTPStackCommon.h"
#include "HTTPStackStateMachine.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPResponseStatusHandler.h"
#include "TransportConnection.h"
#include "HTTPStateInfo.h"
#include "HTTPCookieStore.h"

namespace video
{

typedef HTTPReturnCode (*NotifyCallback)(uint32 requestId,
                              HTTPStackNotifyCode notifyCode,
                              void *cbData,
                              void *instancePtr);

class HTTPStackHelper
{
public:
   HTTPStackHelper(void *cmHTTPStack,
                  NotifyCallback NotifyCb,
                  HTTPCookieMgr& cookieMgr);
  ~HTTPStackHelper();

  void SetHTTPStackPtr(HTTPAuthorizationInterface *pHTTPStack);

  HTTPReturnCode CloseConnection ();

  HTTPReturnCode CreateRequest(uint32& requestId);

  HTTPReturnCode DeleteRequest(uint32 requestId);

  HTTPReturnCode SetHeader(uint32 requestId,
                           const char * key,
                           int keyLen,
                           const char *value,
                           int valueLen);

  HTTPReturnCode UnsetHeader(uint32 requestId,
                             const char *key,
                             int keyLen);

  HTTPReturnCode FlushRequest(uint32 requestId);

  HTTPReturnCode SendRequest(uint32 requestId,
                             HTTPMethodType method,
                             const char *relativeUrl,
                             int relativeUrlLen);

  HTTPReturnCode IsResponseReceived(uint32 requestId);

  HTTPReturnCode GetResponseCode(/*in*/ uint32 RequestId, uint32& nVal);

  HTTPReturnCode GetHeader(uint32 requestId,
                           const char *Key,
                           int keyLen,
                           char *value,
                           int valueLen,
                           int* valueLenReq);

  HTTPReturnCode GetData(uint32 requestId,
                         char *readBuf,
                         size_t readBufSize,
                         size_t *readLen);

  HTTPReturnCode SetProxyServer(const char* proxyServer,
                                size_t proxyServerLen);

  HTTPReturnCode UnsetProxyServer();

  HTTPReturnCode SetSocketMode(bool flagIsBlockingSocket);

  HTTPReturnCode GetContentLength (uint32 requestId, int64* ContentLength, bool bTotal = false);

  HTTPReturnCode GetContentType(uint32 requestId,
                                char *contentType,
                                size_t contentTypeLen,
                                size_t *contentTypeLenReq);

  void SetNetworkInterface(int32 networkIfaceId);

  HTTPReturnCode GetNetworkInterface(int32 &networkIfaceId);

  void SetPrimaryPDPProfile(int32 primaryProfileNo);

  HTTPReturnCode GetPrimaryPDPProfile(int32 &primaryProfileNo);

  HTTPReturnCode GetProxyServer( char* ProxyServer,
                                 size_t ProxyServerLen,
                                 size_t &ProxyServerLenReq);
  void SetNetAbort();

  void SetDisableAutoClose (bool value);

  bool IsProcessingARequest(void);

  char* GetIPAddr(int& pIPAddr);

 HTTPReturnCode GetSockOpt(HTTPStackOption optionType, int& value);

 HTTPReturnCode SetSockOpt(HTTPStackOption optionType, int value);

public:
    /**
   * @brief
   *  Compose the HTTPRequest: Cmd, Hdrs, body.
   *  Also sets following headers if not set by client:
   *    'Host' (mandatory header)
   *    'Connection: KeepAlive'
   *
   * @param requestUri  URI in the HTTP request command line.
   * @return AEEResult
   */
  //HTTPReturnCode ComposeAndQueueRequest();

  HTTPReturnCode CreateOrReuseConnectionObject();

  HTTPReturnCode SendRequestInternal(uint32 requestId);

  //HTTPReturnCode CloseConnectionInternal();

  void StartNewRequest();


  HTTPStateInfo m_HTTPStateInfo;

  void SetState(HTTPStateBase *state);

  char *m_ProxyServerName;
  unsigned short m_ProxyServerPort;

  int32 m_NetworkIfaceId;
  int32 m_PrimaryPDPProfileNo;

  HTTPReturnCode NotifyEvent(uint32 requestId, HTTPStackNotifyCode notifyCode);

private:
  void Destroy();
  void ResetOptions();

private:
  // These functions can be called by friend classes which ensure
  // that strings passed as arguments are null terminated.
  friend class HTTPResponseStatusHandler_200;
  friend class HTTPResponseStatusHandler_300;

  /**
   * Copies 'url' into HTTPStatINfo's m_HTTPUrl. Ensures
   *    m_HTTPUrl is NULL terminated.
   *  Calls a helper function to extract hostname, port, path and
   *   store in HTTPStateInfo's hostname, port, path
   *
   * @param HTTPStackHelper
   * @param url - String passed by client. Treat as untrusted.
   * @param urlLen
   *
   * @return HTTPReturnCode
   */
  //HTTPReturnCode ParseHostPortPathFromUrl(const char *url, int urlLen);

  bool IsProxyServerSet();

  bool ConfigureNetPolicy();

private:

  friend class HTTPStateBase;
  friend class HTTPStateIdle;
  friend class HTTPStateConnecting;
  friend class HTTPStateConnected;

  HTTPStateBase *m_HTTPState;

  CStreamNetwork *m_pCStreamNetwork;

  void *m_pOwner;
  NotifyCallback m_fNotifyCallback;

  // ptr to httpstack to be sent in callback notification
  HTTPAuthorizationInterface *m_pHTTPStack;
};

} // end namespace video

#endif /* HTTPSTACKHELPER_H */
