/************************************************************************* */
/**
 * HTTPStack.cpp
 * @brief implementation of the HTTPStack.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStack.cpp#28 $
$DateTime: 2013/07/11 15:46:28 $
$Change: 4081743 $

========================================================================== */
/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStack.h"

// CommonUtils
#include "SourceMemDebug.h"
#include "qtv_msg.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

namespace video
{

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

/**
 * Constructor, initialize the object
 */
HTTPStack::HTTPStack
(
  HTTPStatusHandlerInterface *httpStatusHandlerInterface,
  HTTPCookieMgr& cookieMgr
)
  : m_HTTPStackHelper(httpStatusHandlerInterface, NotifyEvent, cookieMgr),
    m_pOwner(httpStatusHandlerInterface)
{
  m_HTTPStackHelper.SetHTTPStackPtr(this);
}

/**
 * Destructor, cleanup the object
 */
HTTPStack::~HTTPStack
(
  void
)
{
 (void)CloseConnection();
}

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
HTTPReturnCode
HTTPStack::CloseConnection ()
{
  return m_HTTPStackHelper.CloseConnection();
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
HTTPReturnCode HTTPStack::CreateRequest(/*in*/ uint32& RequestID)
{
  return m_HTTPStackHelper.CreateRequest(RequestID);
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
HTTPReturnCode HTTPStack::DeleteRequest(/*in*/ uint32 RequestID)
{
  return m_HTTPStackHelper.DeleteRequest(RequestID);
}
/**
 * @brief: Set HTTP Header
 *
 * @param[in]: RequestId - RequestID
 * @param[in]: Key - HTTP Header Key
 * @param[in]: KeyLen - String length of Key
 * @param[in]: Value - HTTP Header Value
 * @param[in]: ValueLen - String length of Value
 *
 * @return
 *  HTTP_SUCCESS: on success
 *  HTTP_FAILURE: on failure
 */
HTTPReturnCode
HTTPStack::SetHeader (/*in*/ uint32 RequestId ,
                      /*in*/ const char* Key,
                      /*in*/ int KeyLen,
                      /*in*/ const char* Value,
                      /*in*/ int ValueLen)
{
  return m_HTTPStackHelper.SetHeader(RequestId, Key, KeyLen,
                                     Value, ValueLen);

}

/**
 * Remove header by key.
 *
 * @param[in] RequestId - Ignored for now
 * @param[in] Key - HTTP Header Key
 * @param[in] KeyLen - HTTP Header Key length
 *
 * @return values
 *  HTTP_SUCCESS: Success
 *  HTTP_BADPARAM: Key does not exist in header
 *  HTTP_FAILURE: Failure
 */
HTTPReturnCode
HTTPStack::UnsetHeader (/*in*/ uint32 RequestId,
                        /*in*/ const char* Key,
                        /*in*/ int KeyLen)
{
  return m_HTTPStackHelper.UnsetHeader(RequestId, Key, KeyLen);
}


/**
 * Flushes all information used for constructing HTTP request.
 *
 * @param[in] RequestId - Ignored for now
 *
 * @return values
 *  HTTP_SUCCESS - Success
 *  HTTP_FAILURE - Failure
 */
HTTPReturnCode
HTTPStack::FlushRequest (/*in*/ uint32 RequestId)
{
  return m_HTTPStackHelper.FlushRequest(RequestId);
}

/**
 * Sets optional mesage body
 */
HTTPReturnCode
HTTPStack::SetMessageBody (/*in*/ uint32 /*RequestId*/,
                           /*in*/ const char* /* MessageBody */,
                           /*in*/ int /* MessageBodyLen */)
{
  return HTTP_NOTSUPPORTED;
}

/**
 * Unset mesasge body
 */
HTTPReturnCode
HTTPStack::UnsetMessageBody (/*in*/ uint32 /*RequestId*/)
{
  return HTTP_NOTSUPPORTED;
}

/**
 * Send HTTP Request
 *
 * @return
 *   HTTP_SUCCESS - For blocking socket, successfully sent http request and received response
 *                  For non-blocking socket, successfully sent http request
 *   HTTP_FAILURE - Invalid state for request or failed to send HTTP request
 *   HTTP_NOTSUPPORTED - Unsupported method.
 */
HTTPReturnCode
HTTPStack::SendRequest (/*in*/ uint32 RequestId,
                        /*in*/ HTTPMethodType Method,
                        /*in*/ const char* RelativeUrl,
                        /*in*/ int RelativeUrlLen)
{
  HTTPReturnCode rslt = HTTP_BADPARAM;

  if ((HTTP_HEAD == Method) || (HTTP_GET == Method))
  {
    rslt = m_HTTPStackHelper.SendRequest(RequestId, Method, RelativeUrl, RelativeUrlLen);
  }

  return rslt;
}

/**
 * Check if response to the outstanding HTTP request is received
 *
 * @return values:
 *   HTTP_NOTSUPPORTED for blocking socket
 *   HTTP_SUCCESS if response is ready for non-blocking socket
 *   HTTP_FAILURE if invalid state for request
 *   HTTP_WAIT if response is pending for a non-blocking socket
 */
HTTPReturnCode
HTTPStack::IsResponseReceived (/*in*/ uint32 RequestId)
{
  return m_HTTPStackHelper.IsResponseReceived(RequestId);
}

/**
 * Get the server response code. This should be called only
 * after the http response headers are received.
 *
 * @return HTTPReturnCode
 *  HTTP_SUCCESS  Successfully obtained the response code.
 *  HTTP_FAILURE  Failed to get the response code.
 */
HTTPReturnCode
HTTPStack::GetResponseCode(/*in*/ uint32 RequestId, uint32& nVal)
{
  return m_HTTPStackHelper.GetResponseCode(RequestId, nVal);
}

/**
 * Get the HTTP header
 *
 * @return values:
 *   HTTP_SUCCESS: Header obtained
 *   HTTP_BADPARAM: Key does not exist in header
 *   HTTP_FAILURE: Invalid state for request or generic failure
 */
HTTPReturnCode
HTTPStack::GetHeader (/*in*/ uint32 RequestId,
                      /*in*/ const char* Key,
                      /*in*/ int KeyLen,
                      /*rout*/ char* Value,
                      /*in*/ int ValueLen,
                      /*rout*/ int* ValueLenReq)
{
  return m_HTTPStackHelper.GetHeader(RequestId,
                                     Key,
                                     KeyLen,
                                     Value,
                                     ValueLen,
                                     ValueLenReq);
}

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
HTTPReturnCode
HTTPStack::GetData (/*in*/ uint32 RequestId,
                    /*rout*/ char* Data,
                    /*in*/ size_t DataLen,
                    /*rout*/ size_t* DataLenReq)
{
  QTV_NULL_PTR_CHECK(DataLenReq, HTTP_BADPARAM);

  HTTPReturnCode result = m_HTTPStackHelper.GetData(RequestId,Data, DataLen, DataLenReq);

  if ((HTTP_SUCCESS == result) && (0 == *DataLenReq))
  {
    result = HTTP_NOMOREDATA;
  }

  return result;
}

HTTPReturnCode
HTTPStack::GetContentLength (/*in*/ uint32 RequestId,
                             /*rout*/ int64* ContentLength,
                             /*in*/ bool bTotal)
{
  return m_HTTPStackHelper.GetContentLength(RequestId,ContentLength, bTotal);
}

HTTPReturnCode
HTTPStack::GetContentType (/*in*/ uint32 RequestId,
                           /*rout*/ char* ContentType,
                           /*in*/   size_t ContentTypeLen,
                           /*rout*/ size_t* ContentTypeLenReq)
{
  return m_HTTPStackHelper.GetContentType(RequestId,
                                          ContentType,
                                          ContentTypeLen,
                                          ContentTypeLenReq);
}

/**
 * Set a special option for the stack
 */
HTTPReturnCode
HTTPStack::SetOption (/*in*/ HTTPStackOption OptionType,
                      /*in*/ int Value)
{
  return m_HTTPStackHelper.SetSockOpt(OptionType, Value);
}

HTTPReturnCode
HTTPStack::GetOption (/*in*/ HTTPStackOption OptionType,
                      /*out*/ int& Value)
{
  return m_HTTPStackHelper.GetSockOpt(OptionType, Value);
}

void HTTPStack::SetNetAbort()
{
  return m_HTTPStackHelper.SetNetAbort();
}

HTTPReturnCode HTTPStack::SetAuthorization (uint32 requestId,
                                            const char* key, int keyLen,
                                            const char* value, int valueLen)
{
  return SetHeader(requestId, key, keyLen, value, valueLen);
}

HTTPReturnCode
HTTPStack::GetProxyServer ( char*  ProxyServer,
                            size_t  ProxyServerLen ,
                            size_t  &ProxyServerLenReq )
{
  return m_HTTPStackHelper.GetProxyServer(ProxyServer,ProxyServerLen,ProxyServerLenReq);
}

HTTPReturnCode
HTTPStack::SetProxyServer (/*in*/ const char* ProxyServer,
                           /*in*/ size_t ProxyServerLen)
{
  return m_HTTPStackHelper.SetProxyServer(ProxyServer, ProxyServerLen);
}

HTTPReturnCode
HTTPStack::UnsetProxyServer(void)
{
  return m_HTTPStackHelper.UnsetProxyServer();
}

HTTPReturnCode
HTTPStack::GetSocketMode (/*rout*/ HTTPSocketType* /* SocketMode */)
{
 return HTTP_NOTSUPPORTED;
}

HTTPReturnCode
HTTPStack::SetSocketMode (/*in*/ HTTPSocketType SocketMode)
{
  bool isBlockingSocket = (SocketMode == HTTP_BLOCKING ? true : false);
  return m_HTTPStackHelper.SetSocketMode(isBlockingSocket);
}

void
HTTPStack::SetNetworkInterface(int32 iface)
{
  m_HTTPStackHelper.SetNetworkInterface(iface);
}

void
HTTPStack::SetPrimaryPDPProfile(int32 profileNo)
{
  m_HTTPStackHelper.SetPrimaryPDPProfile(profileNo);
}

HTTPReturnCode
HTTPStack::GetNetworkInterface(int32 &iface)
{
  return m_HTTPStackHelper.GetNetworkInterface(iface);
}

HTTPReturnCode
HTTPStack::GetPrimaryPDPProfile(int32 &profileNo)
{
  return m_HTTPStackHelper.GetPrimaryPDPProfile(profileNo);
}

int
HTTPStack::IsProcessingARequest(void)
{
  return (m_HTTPStackHelper.IsProcessingARequest() ? 1 : 0);
}

char*
HTTPStack::GetIPAddr(int& size)
{
  return m_HTTPStackHelper.GetIPAddr(size);
}

HTTPReturnCode
HTTPStack::NotifyEvent(uint32 /*RequestId*/,
                       HTTPStackNotifyCode notifyCode,
                       void *pCbData,
                       void *pOwner)
{
  HTTPReturnCode result = HTTP_FAILURE;
  HTTPStatusHandlerInterface *statusHandler =
    (HTTPStatusHandlerInterface *)pOwner;

  if (NULL != statusHandler)
  {
    if(statusHandler->Notify(notifyCode,pCbData))
    {
      result = HTTP_SUCCESS;
    }
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                  "CmHTTPStack::NotifyEvent() Discarding notification of HTTPStack event '%d'",
                  notifyCode);

    result = HTTP_SUCCESS;
  }

  return result;
}

HTTPStack::LeakTracer::LeakTracer()
{
  (void)MM_Memory_InitializeCheckPoint();
}

HTTPStack::LeakTracer::~LeakTracer()
{
  (void)MM_Memory_ReleaseCheckPoint();
}

} // end namespace video
