#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
/************************************************************************* */
/**
 * HTTPRequest.h
 * @brief Implements the HTTPRequest
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPRequest.h#16 $
$DateTime: 2012/07/23 17:07:44 $
$Change: 2621737 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackInterface.h"

#include "HTTPStackCommon.h"
#include "TransportConnection.h"
#include "StreamQueue.h"

namespace video
{

/**
 * This structure forms the node of a queue which maintains
 * the key value pair of http headers
 */
typedef struct _HTTPHeaderStruct
{
  StreamQ_link_type link;

  char *m_Key;
  char *m_Value;

  _HTTPHeaderStruct();
  ~_HTTPHeaderStruct();

private:
  // disallow
  _HTTPHeaderStruct(const _HTTPHeaderStruct&);
  _HTTPHeaderStruct& operator=(const _HTTPHeaderStruct&);

  friend class HTTPRequest;
  bool SetKeyAndValue(char *key, char *value);
  bool SetValue(char *value);

} HTTPHeaderStruct;

/**
 * This class manages an HTTP request.
 */
class HTTPRequest
{
public:
  HTTPRequest();
  ~HTTPRequest();

  /**
   * @brief
   *  Reset header queue to have no headers.
   */
  void FlushHeaders();

  /**
   * Extract hostname, port, path and store in
   *   HTTPStateInfo's hostname, port, path.
   *
   * @param HTTPStackHelper
   * @param url - Null terminated string. Caller must
   *              ensure NULL termination. Hence, should not
   *              be called directly on an externally passed
   *              string
   *
   * @return AEEResult
   */
  HTTPReturnCode ParseHostPortPathFromUrlInternal(const char *url);

  /**
   * @brief
   *  There is no support for pieplining currently. This ensures
   *  that IsProcessingARequest() returns false so that a new
   *  request cannot be queued while there is already one in
   *  progress.
   *
   * @return bool
   *  true   Request marked as ready to send
   *  false  Already in the middle of processing of a
   *                request.
   */
  bool MarkReadyToSend();

  /**
   * Composes m_HTTPCommandLine:
   * command + " " + httpUrl + " " HTTP_VERSION + "\r\n"
   */
  bool SetHTTPCommandLine(const char *command,
                          size_t commandLen,
                          const char *httpUrl,
                          size_t httpUrlLen);

  bool SetHeader(const char *key,
                 int keyLen,
                 const char *value,
                 int valueLen);

  bool HeaderExistsForKey(const char *key, int keyLen);

  bool RemoveHeader(const char *key, int keyLen);

  bool GetHeader(const char *key, int keyLen, char *value, int& valLen);

  /**
   * @brief
   *  Send request on network
   *
   * @return AEEResult
   *  AEE_SUCCESS
   *  AEE_EWOULDBLOCK
   *  AEE_EFAILED
   *  AEE_EBADHANDLE Socket Error occured.
   */
  HTTPReturnCode SendRequest(TransportConnection *httpConnection);


  bool IsRequestCompletelySent() const;

  bool IsRequestPartiallyOrFullySent() const;

  /**
   * @brief
   *  Return true if in the middle of processing a request, false
   *  if not. If request state is REQUESR_INITIAL or REQUEST_ERROR
   * , then not processing a request.
   *
   * @return bool
   */
  bool IsProcessingARequest() const;

  bool IsRspFullyRecd() const;

  void SetRspHeadersRecd();
  void SetRspFullyRecd();
  void SetRspError();

  bool IsRspHeadersRecd() const;

  void IncNumRetries();
  int GetNumRetries() const;
  void ResetNumRetries();

  void IncNumRedirects();
  void ResetNumRedirects();
  int GetNumRedirects() const;

  void SetRequestMethod(HTTPMethodType method);
  HTTPMethodType GetRequestMethod() const;

  HTTPReturnCode SetRequestUrl(const char* url, size_t urlLen);
  const char* GetRequestUrl() const;

  HTTPReturnCode SetHostName(const char* hostName);
  const char* GetHostName() const;

  void SetPort(unsigned short port);
  unsigned short GetPort() const;

  HTTPReturnCode SetRelativePath(const char *relativePath);
  const char* GetRelativePath() const;
  HTTPReturnCode EncodeURL(const char *url,char *encodedurl,int& encodeurlLen);

  bool IsReservedCharacter(char c);
  bool IsPercentEncodingRequired(char c);

  /**
   * Reset, reinitialize member variables
   */
  void Reset();

  void Log();

  void MarkSentRequestAsReSend();

private:

  // disallow
  HTTPRequest(const HTTPRequest&);
  HTTPRequest& operator=(const HTTPRequest&);

  enum RequestState
  {
    REQUEST_INITIAL,          // Client has not invoked a SendRequest
    REQUEST_WAITING_TO_SEND,  // Client has successfully invoked a SendRequest.
    REQUEST_SENDING,          // Buffer contents partially sent on network
    REQUEST_SENT,             // Buffer contents fully sent on network
    REQUEST_RSP_HDRS_RECD,    // Http headers received
    REQUEST_RSP_FULLY_RECD,   // Http rsp fully received
    REQUEST_ERROR             // Non recoverable error state
  };

  /**
   * @brief
   *  Compose request buffer from cmd line, hdrs
   *  (and msg body ### TO DO ### )
   *
   * @return bool
   *  true
   *  false
   */
  bool PrepareRequestBuffer();

  /**
   * @brief
   *  Sends as much pending data of the http request to the
   *  network
   *
   * @param httpStackCommon
   * @param sockfd
   *
   * @return AEEResult
   *  AEE_SUCCESS
   *  AEE_EWOULDBLOCK
   *  AEE_EFAILED
   */
  TransportConnection::ResultCode SendRequestOnNetwork(
    TransportConnection* httpConnection);

  HTTPMethodType m_HttpMethod;

  char* m_Url;

  char* m_HostName;
  unsigned short m_Port;

  /**
   * Part of http url after host and port
   */
  char *m_RelativePath;

  char *m_HTTPCommandLine;

  size_t m_LengthOfRequest;

  /**
   * allocated buffer size will be m_LengthOfRequest + 1.
   * extra byte to keep the buffer null terminated.
   */
  char *m_Buffer;

  /**
   * Number of bytes out of m_Length of Request that has been
   * successfully sent to socket
   */
  size_t m_NumBytesSent;

  /**
   * Number of times to retry sending a request.
   * 0 for no retries.
   */
  int m_NumRetries;

  int m_NumRedirects;

  StreamQ_type m_HeaderList;

  RequestState m_RequestState;

  static const char reservedURLChars[];

};

} // end namespace video

#endif /* HTTPREQUEST_H */
