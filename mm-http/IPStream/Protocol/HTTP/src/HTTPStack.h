#ifndef HTTPSTACK_H
#define HTTPSTACK_H
/************************************************************************* */
/**
 * HTTPStack.h
 * @brief Implements the HTTPStack
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStack.h#26 $
$DateTime: 2013/07/11 15:46:28 $
$Change: 4081743 $

========================================================================== */
/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
// HTTPStack
#include "HTTPStackCommon.h"
#include "HTTPStackHelper.h"
#include "HTTPStackInterface.h"

namespace video
{

class HTTPStackHelper;

class HTTPStack
  : public HTTPStackInterface, public HTTPAuthorizationInterface
{

public:
  explicit HTTPStack (HTTPStatusHandlerInterface *httpStatusHandlerInterface,
                      HTTPCookieMgr &cMgr);

  virtual ~HTTPStack();

  virtual HTTPReturnCode CloseConnection ();

  virtual HTTPReturnCode CreateRequest(/*in*/ uint32& RequestID);

  virtual HTTPReturnCode DeleteRequest(/*in*/ uint32 RequestID);

  virtual HTTPReturnCode SetHeader (/*in*/ uint32 RequestId,
                            /*in*/ const char* Key,
                            /*in*/ int KeyLen,
                            /*in*/ const char* Value,
                            /*in*/ int ValueLen);

  virtual HTTPReturnCode UnsetHeader (/*in*/ uint32 RequestId,
                              /*in*/ const char* Key,
                              /*in*/ int KeyLen);

  virtual HTTPReturnCode FlushRequest (/*in*/ uint32 RequestId);

  virtual HTTPReturnCode SetMessageBody (/*in*/ uint32 RequestId,
                                 /*in*/ const char* MessageBody,
                                 /*in*/ int MessageBodyLen);

  virtual HTTPReturnCode UnsetMessageBody (/*in*/ uint32 RequestId);

  virtual HTTPReturnCode SendRequest (/*in*/ uint32 RequestId,
                              /*in*/ HTTPMethodType Method,
                              /*in*/ const char* RelativeUrl,
                              /*in*/ int RelativeUrlLen);

  virtual HTTPReturnCode IsResponseReceived (/*in*/ uint32 RequestId);

  virtual HTTPReturnCode GetResponseCode(/*in*/ uint32 RequestId, uint32& nVal);

  virtual HTTPReturnCode GetHeader (/*in*/ uint32 RequestId,
                            /*in*/ const char* Key,
                            /*in*/ int KeyLen,
                            /*rout*/ char* Value,
                            /*in*/ int ValueLen,
                            /*rout*/ int* ValueLenReq);

  virtual HTTPReturnCode GetData (/*in*/ uint32 RequestId,
                          /*rout*/ char* Data,
                          /*in*/ size_t DataLen,
                          /*rout*/ size_t* DataLenReq);

  virtual HTTPReturnCode GetContentLength (/*in*/ uint32 RequestId,
                                         /*rout*/ int64* ContentLength,
                                         /*in*/ bool bTotal = false);

  virtual HTTPReturnCode GetContentType (/*in*/ uint32 RequestId,
                                 /*rout*/ char* ContentType,
                                 /*in*/   size_t ContentTypeLen,
                                 /*rout*/ size_t* ContentTypeLenReq);

  virtual HTTPReturnCode SetOption (/*in*/ HTTPStackOption OptionType,
                                    /*in*/ int Value);

  virtual HTTPReturnCode GetOption (/*in*/ HTTPStackOption OptionType,
                                    /*out*/ int& Value);

  virtual void SetNetworkInterface(int32 iface);

  virtual void SetPrimaryPDPProfile(int32 profileNo);

  virtual HTTPReturnCode GetNetworkInterface(int32 &iface);

  virtual HTTPReturnCode GetPrimaryPDPProfile(int32 &profileNo);


  virtual HTTPReturnCode GetProxyServer (/*rout*/ char* ProxyServer,
                                 /*in*/ size_t ProxyServerLen,
                                 /*rout*/ size_t &ProxyServerLenReq);

  virtual HTTPReturnCode SetProxyServer (/*in*/ const char* ProxyServer,
                                 /*in*/ size_t ProxyServerLen);

  virtual HTTPReturnCode UnsetProxyServer(void);

  virtual HTTPReturnCode GetSocketMode (/*rout*/ HTTPSocketType* SocketMode);

  virtual HTTPReturnCode SetSocketMode (/*in*/ HTTPSocketType SocketMode);

  virtual void SetNetAbort();

  virtual HTTPReturnCode SetAuthorization (uint32 requestId,
                                           const char* key, int keyLen,
                                           const char* value, int valueLen);

  virtual int IsProcessingARequest(void);

  virtual char* GetIPAddr(int& size);

private:
  static HTTPReturnCode NotifyEvent(uint32 requestId,
                                    HTTPStackNotifyCode notifyCode,
                                    void *pCbData,
                                    void *pOwner);

private:

  class LeakTracer
  {
  public:
    LeakTracer();
    ~LeakTracer();

  private:
    LeakTracer(const LeakTracer&);
    LeakTracer& operator=(const LeakTracer&);
  };

  LeakTracer m_LeakTracer;

  friend class HTTPStackHelper;
  HTTPStackHelper m_HTTPStackHelper;

  /**
   * Pointer to HTTPStackClient.
   * If this is NULL, it means client is not interested in
   * notifications.
   */
  HTTPStatusHandlerInterface *m_pOwner;
};

} // end namespace video

#endif /* HTTPSTACK_H */
