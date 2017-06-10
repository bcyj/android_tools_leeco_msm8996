#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
/************************************************************************* */
/**
 * HTTPResponse.h
 * @brief Implements receiving the HTTP response for an HTTP request.
 *        Maintains book-keeping information about the state of the
 *        received response and of the response headers.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPResponse.h#21 $
$DateTime: 2012/07/30 06:11:41 $
$Change: 2641893 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackCommon.h"
#include "TransportConnection.h"
#include "Streamlist.h"

namespace video
{

class HTTPRequest;

enum HTTPResponseStatus
{
   HTTP_RSP_SUCCESS,
   HTTP_RSP_FAILURE,
   HTTP_RSP_WAIT,
   HTTP_RSP_HDRS_RECVD,
   HTTP_RSP_DONE,
   HTTP_RSP_INSUFFBUFFER
};

// A queue element (stored in Queue) for a Cookie header value element
struct HTTPCookieElement
{
  ordered_StreamList_link_type link;
  uint32 size;
  char value[1];
};

class HTTPResponse
{
public:

  HTTPResponse();
  ~HTTPResponse();

  /**
   * Resets private variables.
   */
  void Reset();

  HTTPResponseStatus ReceiveResponse(TransportConnection* httpConnection);


  /**
  *  Frees old buffer if exists and creates a new one.
  */
  bool PrepareBuffer();


  bool IncreaseRspBufferSize();

 /**
  * @brief:
  *  Reads from socket if response header is not yet fully received.
  *
  *  @returns:
  *  true: Tried to read from socket and no unrecoverable error occured.
  *  false: Unrecoverable error occured.
  */
  HTTPResponseStatus ReceiveHTTPResponse(TransportConnection* httpConnection);

  /**
   * @brief
   *  Check if in the middle of processsing a response.
   *
   * @return bool
   */
  bool IsProcessingAResponse() const;

  bool IsResponseHeaderFullyReceived() const;

  bool IsMessageLengthSpecifierSupported() const;

  bool IsDownloadingData() const;

  int GetHTTPResponseCode() const;
  int64 GetContentLength() const;
  int64 GetTotalContentLength() const;
  const char* GetEntityBody()const;
  void SetContentLength(int contentLength);
  void ResetContentLength();

  const char *GetContentType() const;
  const char *GetReasonPhraseStr() const;
  bool IsTransferEncodingSet() const;

  /**
   * True if HTTP response header has
   * Connection: Keep-Alive
   *
   * @return bool
   */
  bool IsConnectionPersistent() const;

  bool GetHeaderValue(const char *key, int keyLen,
                      char *value, int valueLen, int &valueLenRequested);

  /**
   * @brief
   *  Get the entire HTTP Header string including the traling
   *  \r\n\r\n. Terminate the string with '\0'
   *
   * @param headerStr
   * @param headerStrBufSize
   * @param reqLen
   */
  void GetHeaders(char *headerStr, int headerStrBufSize, int& reqLen);

  /**
   * @brief
   *  Get the reference to HTTP Cookie headers value list
   */
  void GetCookieHeaderValueList(ordered_StreamList_type **cookieList);

  HTTPResponseStatus GetData(TransportConnection* httpConnection,
                         char *readBuf,
                         size_t readBufSize,
                         size_t& readLen);

  /**
   * @brief
   *  This needs to be called when dealing with a FastTrack
   *  download. For FastTrack the length specifed in
   *  'Content-Length' header is less than the actual HTTP data.
   *  This is a violation of HTTP std. In fact the HTTP Data
   *  lenght is unknown. Hence, when download is complete for the
   *  request, client should Close the connection.
   */
  void SetInvalidContentLengthFlag(bool value);

  void MarkRspDone();
  void MarkRspError();

  /**
   * Get ptr to entity buffer resuming at the point past the last
   *   written byte.
   *
   * @param pBuf
   * @param remainingSize
   */
  void GetEntityBodyBuffer(char *&pBuf, int& remainingSize);

  /**
   * @brief
   *  Appended an amount of 'size' bytes to entity buffer.
   *
   * @param size
   */
  void CommitEntityBodyBuffer(size_t size);

  /**
   * Get number of data bytes read by client
   *
   * @return number of bytes read
   */
  uint64 GetNumBytesRead() const
  {
    return (uint64)m_NumDataBytesReadByClient;
  };

private:

  static const int HTTP_RSP_HDR_BUF_INITIAL_LEN = 1000;

  enum RspState
  {
    RSP_INITIAL,          // Initial state
    RSP_WAITING_FOR_HDRS, // Request sennt. Waiting for response.
    RSP_HDRS_RECEIVED,    // Set to true when "\r\n\r\n" is found in m_Buffer
    RSP_DOWNLOADING_DATA, // Moves to this state only on call to GetData.
    RSP_DONE,             // Rsp hdrs and content body (if any) fully received.
    RSP_ERROR             // Error occured in trying to get response, or
                          //   http stack does not support interpreting
                          //   the response headers.
  };

  bool ExtractHTTPResponseCode();
  bool ExtractHTTPResponseReasonPhrase();
  void ExtractRelevantHeaderValues();
  bool ExtractMessageLength();
  bool ExtractContentType();
  bool ExtractTransferEncoding();
  bool ExtractConnection();
  void ExtractCookieHeadersValue();
  bool GetNextCookieHeaderValue(char **buf, char *value,
                                int valueLen, int &valueLenReq);

  void SetRspStateInitial();
  void SetRspStateWaitingForHdrs();
  void SetRspStateHeadersRecd();
  void SetRspStateDownloadingData();
  void SetRspStateDone();
  void SetRspStateError();
  HTTPResponseStatus MapTransportResultCode(const TransportConnection::ResultCode rslt);
  HTTPResponseStatus MapCommonResultCodeToRspCode(const HTTPReturnCode rslt );

private:
  /**
   * To hold the http response headers and the data
   * just past it.
   */
  char *m_Buffer;

  /**
   * The maximum string length of a null terminated string that
   * can be accomodated in m_Buffer. The actual size of the
   * allocated buffer is one more than m_BufferLen to account for
   * the trailing '\0'
   */
  size_t m_BufferLen;

  /**
  HTTP response reason phrase str

  */
  char *m_ReasonPhrase;

  /**
   * Number of bytes read into m_Buffer
   */
  size_t m_NumBytesReadInBuffer;

  /**
   * Length of http response header. This is valid
   * after the response header has been completely received.
   */
  size_t m_ResponseHeaderLen;

  /**
   * When the http response is received, there will be data past the
   * end of the header. Until
   * m_ResponseHeaderLen + m_NumDataBytesReadByClient = m_NumBytesReadInBuffer,
   * a client read operation will receive 'http data' from this buffer
   */
  size_t m_NumDataBytesReadByClient;

  /**
   * HTTP Server Version
   */
  double m_HTTPServerVersion;

  /**
   * HTTP response status code. This is valid only after the
   * response hdrs are fully received.
   */
  int m_HTTPResponseCode;

  /**
   * Cached value of "Content-Length". This is valid only after
   * the rsp hdrs are fully received.
   */
  int64  m_ContentLength;

  /**
   * Cached value of total content length extracted from "Content-Range". If
   * the header doesn't exist this is the same as m_ContentLength. This is
   * valid only after the rsp hdrs are fully received.
   */
  int64 m_nTotalContentLength;

  /**
   * Cached value of "Content-Type". This is valid only after the
   * rsp hdrs are fully received.
   */
  char *m_ContentType;

  /**chunked encoding context
  */
  HTTPTransferEncodingFilter m_TransferEncodingFilter;

  /**
   * Cached value of "Connection".
   * True if value is "Keep-Alive".
   * False otherwise.
   */
  bool m_IsPersistentConnection;

  /**
   * Set to true. If the http stack client wants the stack to
   * disregard interpreting message length from the rsp headers
   * and will call an explicit "Close" on the stack after thre
   * response data is downloaded.
   * This also means if this option is set to true, then request
   * following this cannot be pipelined even if host and port are
   * the same as for the current request. Anyways pipelining is
   * not supported currently.
   */
  bool m_FlagInvalidContentLength;

  RspState m_RspState;

  // for 3xx,4xx,5xx,...
  static const int MAX_ENTITY_BODY_SIZE = 4000;
  char m_EntityBody[MAX_ENTITY_BODY_SIZE];
  int m_NumBytesInEntityBody;

  /**
   * Use as a scratch buffer within a single function only to
   * reduce the number of heap allocations and de-allocations.
   */
  char *m_TmpLocalBuffer;

  /**
   * Current size of m_TmpLocalBuffer
   */
  size_t m_TmpLocalBufferSize;

  // Response Cookie Header Values list
  ordered_StreamList_type m_CookieValueList;
};

} // end namespace video

#endif /* HTTPRESPONSE_H */
