#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H
/************************************************************************* */
/**
 * HTTPRequestHandler.h
 * @brief Implements the HTTPRequestHandler
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPRequestHandler.h#12 $
$DateTime: 2013/07/27 07:46:50 $
$Change: 4174225 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "StreamQueue.h"
#include "HTTPRequest.h"
#include "HTTPStackInterface.h"
#include "TransportConnection.h"
#include "HTTPResponse.h"



/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

namespace video
{
typedef struct
{
  StreamQ_link_type link;
  HTTPRequest *m_httpRequest;
  uint32 m_requestID;
  bool bPipeline;
  bool bMarkForDeletion;
}HTTPRequestElem;

class HTTPRequestHandler
{
public:

  HTTPRequestHandler();
  ~HTTPRequestHandler();

  HTTPReturnCode CreateRequest(uint32& RequestID);
  HTTPReturnCode DeleteRequest(uint32 RequestID, bool &bInternalClose);
  bool SetHeader(uint32 RequestID,const char *Key,int KeyLen,const char *Value,int ValLen);
  bool RemoveHeader(uint32 RequestID, const char *key, int keyLen);
  bool GetHeader(uint32 RequestID, const char *Key, int KeyLen, char *Value, int& ValLen);
  HTTPReturnCode SetRequest(uint32 RequestID,
                            HTTPMethodType method,
                            const char *Url,
                            size_t UrlLen,
                            TransportConnection *pConnection,
                            const char *proxyServer = NULL);
  bool ComposeAndQueueRequest(uint32 RequestID, const char* proxyServer = NULL);
  HTTPReturnCode FlushRequest (uint32 RequestId);
  HTTPReturnCode SendPendingRequests(TransportConnection *pConnection,
                                     bool& bCreateNewConnection);
  bool IsRequestDone(uint32 RequestID);
  void SetRequestState(uint32 RequestID,HTTPResponseStatus eRSPStatus);
  const char* GetHostName();
  unsigned short GetPort();
  HTTPMethodType GetRequestMethod(uint32 RequestID);
  void ResetAllRequests();
  HTTPReturnCode GetRequestStatus(uint32 RequestID);
  bool IsRequestRspHrdReceived(uint32 RequestID);
  bool IsRequestSent(const uint32 RequestID);
  HTTPReturnCode PrepareForReconnect(uint32 RequestID);
  bool ShouldRetry(uint32 nRequestID);
  int GetNumRedirects(uint32 RequestID);
  HTTPReturnCode HandleRedirect(uint32 RequestID,
                                int ResponseStatusCode,
                                const char *redirectUrl,
                                const char *originalUrl);
  const char* GetRequestUrl(uint32 RequestID);
  HTTPReturnCode SetRequestUrl(uint32 RequestID, const char* url, size_t urlLen);
  bool HeaderExistsForKey(uint32 RequestID,const char *key, int keyLen);
  void SetPipelineSupport(bool bIsSupported);
  uint32 GetNumPendingRequestTobeSent();
  bool DoHavetoCreateNewConnection(TransportConnection *pConnection);
  void Reset();
  bool IsRequestPartiallyOrFullySent(uint32 RequestID);
  HTTPReturnCode SetProxyInfo(const char *proxyServer, unsigned short proxyPort);
  const char* GetProxyServer() const;
  unsigned short GetProxyPort() const;

private:

  void flushRequestQ();
  HTTPReturnCode DeleteRequest(HTTPRequestElem *reqElem);
  void UpdatePiplineStatusForQueuedRequest();
  void CheckAndHandleMarkedAsDeleteRequest(bool &bCreateNewConnection);

  StreamQ_type m_requestQ;
  bool m_isPipelineSupported;
  uint32 m_nUniqueKey;
  char *m_ProxyServerName;
  unsigned short m_ProxyServerPort ;

};

}

#endif /* HTTPREQUESTHANDLER_H */
