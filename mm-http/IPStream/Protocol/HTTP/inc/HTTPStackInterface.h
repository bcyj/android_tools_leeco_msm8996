#ifndef _HTTP_STACK_INTERFACE_
#define _HTTP_STACK_INTERFACE_

/************************************************************************* */
/**
 * HTTPStackInterface.h
 * @brief Interface for the HTTP Protocl Stack
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/inc/HTTPStackInterface.h#26 $
$DateTime: 2013/07/11 15:46:28 $
$Change: 4081743 $

========================================================================== */

#include "AEEStdDef.h"
#include "HTTPCookieStore.h"

namespace video
{

enum HTTPMethodType
{
  HTTP_HEAD,
  HTTP_GET,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE,
  HTTP_OPTIONS,
  HTTP_CONNECT
};

enum HTTPReturnCode
{
  HTTP_SUCCESS,       /* Operation successful */
  HTTP_FAILURE,       /* Generic failure */
  HTTP_WAIT,          /* Operation would block if not non-blocking; wait and try again */
  HTTP_BADPARAM,      /* Bad (input) parameter(s) */
  HTTP_NOTSUPPORTED,  /* Operation not supported */
  HTTP_NOMOREDATA,    /* End of data/file */
  HTTP_INSUFFBUFFER   /* Insufficient buffer, allocate more buffer space and try again */
};

enum HTTPSocketType
{
  HTTP_BLOCKING,
  HTTP_NON_BLOCKING
};

enum HTTPStackOption
{
  HTTP_SOCKET_RECV_BUF_SIZE,  //Sets/Gets the receive Buffer size on the socket
  HTTP_SOCKET_SEND_BUF_SIZE   //Sets/Gets the send buffer size on the socket
};

enum HTTPStackNotifyCode
{
  HTTP_PROTOCOL_ERROR_CODE
};

class HTTPAuthorizationInterface;

typedef struct
{
  int32 m_serverCode;
  char *m_serverMessage;
  char *m_entityBody;
  bool m_msgType;
  char *m_method;
  char *m_protocolHeaders; // it will contain the complete authentication header.
  HTTPAuthorizationInterface *m_pHTTPStack;
} HTTPStackNotificationCbData;

/**
 * @brief
 *  Interface to be defined by client to be able to receive
 *  notification from HTTPStack.
 */
class HTTPStatusHandlerInterface
{
public:
  virtual ~HTTPStatusHandlerInterface()
  {  }
  virtual bool Notify(HTTPStackNotifyCode httpNotifyCode,void *pCbData) = 0;
};

class HTTPAuthorizationInterface
{
public:
  virtual ~HTTPAuthorizationInterface() { }

  virtual HTTPReturnCode SetAuthorization (
    uint32 RequestId, const char* Key,
    int KeyLen, const char* Value, int ValueLen) = 0;
};

/**
 * @brief
 *  HTTP stack interface - implements HTTP 1.1 protocol RFC 2616.
 */
class HTTPStackInterface
{
public:
  static HTTPReturnCode CreateInstance(
    HTTPStackInterface** ppIface,
    HTTPStatusHandlerInterface *ownerInterface,
    HTTPCookieMgr& cookieMgr);

  HTTPStackInterface();
  virtual ~HTTPStackInterface() = 0;

  /**
   * Close connection to HTTP Server. This should be called only once.
   * It will block for a small period of time and then close tcp connection
   * in the background if tcp close did not complete during the duration
   * the call is blocked.
   *
   * return values:
   *  HTTP_SUCCESS    - tcp close successful
   *  HTTP_WAIT       - tcp close in progress.
   *  HTTP_FAILURE    - failed
   */
  virtual HTTPReturnCode CloseConnection () = 0;

  /**
   * @brief  Create a new Request for RequestID
   *
   * @param RequestID
   *
   * @return HTTPReturnCode
   *   HTTP_SUCCESS: on success
   *   HTTP_FAILURE: on failure
   */
  virtual HTTPReturnCode CreateRequest(/*in*/ uint32& RequestID) = 0;

  /**
   * @brief  Delete a Request associated RequestID
   *
   * @param RequestID
   *
   * @return HTTPReturnCode
   *   HTTP_SUCCESS: on success
   *   HTTP_FAILURE: on failure
   */
  virtual HTTPReturnCode DeleteRequest(/*in*/ uint32 RequestID) = 0;

  /**
   * Set HTTP header key and value
   *
   * @return values
   *  HTTP_SUCCESS: on success
   *  HTTP_FAILURE: on failure
   */
  virtual HTTPReturnCode SetHeader (/*in*/ uint32 RequestId,
                                    /*in*/ const char* Key,
                                    /*in*/ int KeyLen,
                                    /*in*/ const char* Value,
                                    /*in*/ int ValueLen) = 0;

  /**
   * Remove header by key.
   *
   * @return values
   *  HTTP_SUCCESS: Success
   *  HTTP_BADPARAM: Key does not exist in header
   *  HTTP_FAILURE: Failure
   */
  virtual HTTPReturnCode UnsetHeader (/*in*/ uint32 RequestId,
                                      /*in*/ const char* Key,
                                      /*in*/ int KeyLen) = 0;

  /**
   * Flushes all information used for constructing HTTP request.
   *
   * @return values
   *  HTTP_SUCCESS - Success
   *  HTTP_FAILURE - Failure
   */
  virtual HTTPReturnCode FlushRequest (/*in*/ uint32 RequestId) = 0;

  /**
   * Sets optional mesage body
   */
  virtual HTTPReturnCode SetMessageBody (/*in*/ uint32 RequestId,
                                         /*in*/ const char* MessageBody,
                                         /*in*/ int MessageBodyLen) = 0;

  /**
   * Unsets optional mesage body that was set earlier
   */
  virtual HTTPReturnCode UnsetMessageBody (/*in*/ uint32 RequestId) = 0;

  /**
   * Send HTTP Request
   *
   * @return
   *   HTTP_SUCCESS - For blocking socket, successfully sent http request and received response
   *                  For non-blocking socket, successfully sent http request
   *   HTTP_FAILURE - Invalid state for request or failed to send HTTP request
   *   HTTP_NOTSUPPORTED - Unsupported method.
   */
  virtual HTTPReturnCode SendRequest (/*in*/ uint32 RequestId,
                                      /*in*/ HTTPMethodType Method,
                                      /*in*/ const char* RelativeUrl,
                                      /*in*/ int RelativeUrlLen) = 0;

  virtual char* GetIPAddr(int& size) = 0;

  /**
   * Check if response to the outstanding HTTP request is received
   *
   * @return values:
   *   HTTP_NOTSUPPORTED for blocking socket
   *   HTTP_SUCCESS if response is ready for non-blocking socket
   *   HTTP_FAILURE if invalid state for request
   *   HTTP_WAIT if response is pending for a non-blocking socket
   */
  virtual HTTPReturnCode IsResponseReceived (/*in*/ uint32 RequestId) = 0;

  /**
   * Get the HTTP header
   *
   * @return values:
   *   HTTP_SUCCESS: Header obtained
   *   HTTP_BADPARAM: Key does not exist in header
   *   HTTP_FAILURE: Invalid state for request or generic failure
   */
  virtual HTTPReturnCode GetHeader (/*in*/ uint32 RequestId,
                                    /*in*/ const char* Key,
                                    /*in*/ int KeyLen,
                                    /*rout*/ char* Value,
                                    /*in*/ int ValueLen,
                                    /*rout*/ int* ValueLenReq) = 0;

  /**
   * Get/Read data from the TCP layer
   *
   * @return values
   *   HTTP_SUCCESS     - Data available to read (*DataLenReq indicates
                          number of bytes read)
   *   HTTP_FAILURE     - Invalid state for request or Connection aborted prematurely
                          or generic failure to process request
   *   HTTP_WAIT        - Zero bytes available to read (underrun), check back later
   *   HTTP_NOMOREDATA  - End of file (data download complete)
   */
  virtual HTTPReturnCode GetData (/*in*/ uint32 RequestId,
                                  /*rout*/ char* Data,
                                  /*in*/ size_t DataLen,
                                  /*rout*/ size_t* DataLenReq) = 0;

  /**
   * Get the server response code. This should be called only
   * after the http response headers are received.
   *
   * @return HTTPReturnCode
   *  HTTP_SUCCESS  Successfully obtained the response code.
   *  HTTP_FAILURE  Failed to get the response code.
   */
  virtual HTTPReturnCode GetResponseCode(/*in*/ uint32 RequestId, uint32& nVal) = 0;

  /**
   * Get the content length
   */
  virtual HTTPReturnCode GetContentLength (/*in*/ uint32 RequestId,
                                           /*rout*/ int64* ContentLength,
                                           /*in*/ bool bTotal = false) = 0;

  /**
   * Get the content type
   */
  virtual HTTPReturnCode GetContentType (/*in*/ uint32 RequestId,
                                         /*rout*/ char* ContentType,
                                         /*in*/ size_t ContentTypeLen,
                                         /*rout*/ size_t* ContentTypeLenReq) = 0;

  /**
   * Set an option for the HTTPStack.
   *
   * @return
   *  HTTP_SUCCESS - Set option was sucessfully completed.
   *  HTTP_WAIT    - The set option could not be completed and
   *                 will be performed later when
   *                 possible. Client does not need to call this
   *                 again.
   *  HTTP_FAILED  - Any failure.
   */
  virtual HTTPReturnCode SetOption (/*in*/ HTTPStackOption OptionType,
                                    /*in*/ int Value) = 0;

  /**
   * Get an option for the HTTPStack.
   *
   * @return
   *  HTTP_SUCCESS - Get option was sucessfully completed. Value
   *                 holds appropriate value.
   *  HTTP_WAIT    - Client needs to retry calling this API to get
   *                 the value.
   *  HTTP_FAILED  - Any failure.
   */
  virtual HTTPReturnCode GetOption (/*in*/ HTTPStackOption OptionType,
                                    /*out*/ int& Value) = 0;

  /**
   *  Set the Network Interface
   */
  virtual void SetNetworkInterface(int32 iface)= 0;


  /**
   * Set the PrimaryPDPProfile
   */
  virtual void SetPrimaryPDPProfile(int32 profileNo)= 0;

   /**
   * Get the Network Interface
   */
  virtual HTTPReturnCode GetNetworkInterface(int32 &iface)= 0;

   /**
   * Get the PrimaryPDPProfile
   */
  virtual HTTPReturnCode GetPrimaryPDPProfile(int32 &profileNo)= 0;

  /**
   * Set an optional proxy server
   */
  virtual HTTPReturnCode SetProxyServer (/*in*/ const char* ProxyServer,
                                         /*in*/ size_t ProxyServerLen) = 0;

  virtual HTTPReturnCode UnsetProxyServer(void) = 0;

  /**
   * Get the optional proxy server
   */
  virtual HTTPReturnCode GetProxyServer (/*rout*/ char* ProxyServer,
                                         /*in*/ size_t ProxyServerLen,
                                         /*rout*/ size_t &ProxyServerLenReq) = 0;

  /**
   * Set the socket operation mode - BLOCKING vs NON_BLOCKING
   */
  virtual HTTPReturnCode SetSocketMode (/*in*/ HTTPSocketType SocketMode) = 0;

  /**
   * Get the socket operation mode - BLOCKING vs NON_BLOCKING
   */
  virtual HTTPReturnCode GetSocketMode (/*rout*/ HTTPSocketType* SocketMode) = 0;

  virtual void SetNetAbort()=0;

  /**
   * return 1 if stack is in the middle of processing a request, 0
   * otherwise.
   */
  virtual int IsProcessingARequest(void) = 0;

private:
  HTTPStackInterface(const HTTPStackInterface&);
  HTTPStackInterface& operator=(const HTTPStackInterface&);
};

} // NS_END

#endif
