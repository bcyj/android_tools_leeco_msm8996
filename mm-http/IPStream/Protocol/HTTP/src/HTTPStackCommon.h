#ifndef HTTPSTACKCOMMON_H
#define HTTPSTACKCOMMON_H
/************************************************************************* */
/**
 * HTTPStackCommon.h
 * @brief Implements the HTTPStackCommon. Contains networking related
 *        functions for http stack.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackCommon.h#15 $
$DateTime: 2013/07/27 08:03:59 $
$Change: 4174247 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "HTTPStackInterface.h"
#include "TransportConnection.h"
#include "oscl_string_utils.h"

namespace video
{

class HTTPStackCommon
{
public:
  static const unsigned short DEFAULT_HTTP_PORT = 80;
  static const char* HTTP_VERSION;  // "HTTP/1.1";

  // The following max lengths prevent an untrusted client from
  // causing the stack to allocate too much memory.
  static const int MAX_COMMAND_LEN                  = 200;
  static const int MAX_URL_LEN                      = 2048;
  static const int MAX_PROXY_URL_LEN                = 2048;
  static const int MAX_HTTP_REQUEST_HEADER_KEY_LEN  = 2048;
  static const int MAX_HTTP_REQUEST_VALUE_LEN       = 2048;
  static const int MAX_HTTP_REQ_HDR_ENTRIES         = 50;

  // The maximum length for HTTP Response Header that is supported.
  // This prevents an untrusted server from causing the stack to allocate
  // too much memory.
  static const int MAX_HTTP_RSP_HDR_BUF_LEN = 6000;

  // Specifies number of times to retry establishing connection.
  static const int MAX_RETRIES = MAX_INT32;

  // Max number of successive redirect attempts before giving up.
  static const int MAX_REDIRECTS = 5;

  static const char* AUTHENTICATE_KEY;
  static const char* AUTHORIZATION_KEY;
  static const char* PROXY_AUTHENTICATE_KEY;
  static const char* PROXY_AUTHORIZATION_KEY;
  static const char* RANGE_KEY;

  // For socket related functiosn
  enum ReturnCode
  {
    SUCCESS,
    FAILED,
    IN_PROGRESS,
    REDIRECT,
    SERVER_CLOSED_CONNECTION,
    UNSUPPORTED,
    BAD_STATE
  };

  HTTPStackCommon();
  ~HTTPStackCommon();

  static HTTPReturnCode GetHostPortRelativePathFromUrl(
                          const char *url, size_t urlLen,
                          char *hostName, size_t hostNameBufSize,
                          size_t& hostNameBufSizeRequested,
                          unsigned short & portNumber,
                          char *relativePath, size_t relativePathBufSize,
                          size_t& relativePathBufSizeRequested);
  static HTTPReturnCode MapTransportResultCode(TransportConnection::ResultCode rslt);

  /**
   * @brief
   *  Get the string for the http method type.
   *
   * @param method
   *
   * @return const char*
   */
  static const char* GetStringForMethod(HTTPMethodType method);

private:

  /**
   * Determine length of hostname and path or
   * populate host, port, path in client buffers.
   *
   * @param uri
   * @param hostName  - Ptr to client hostname buffer
   * @param hostNameBufSize - Size of client hostname buffer
   * @param hostNameBufSizeRequested - Size necessary for client
   *                                 hostname buffer
   * @param portNumber - Ptr to client owned unsigend short
   * @param relativePath - Ptr to client relativePath buffer
   * @param relativePathBufSize - Size of client relativePath
   *                            buffer
   * @param relativePathBufSizeRequested - Size necessary for
   *                                     client relativePath
   *                                     buffer.
   */
  static void PopulateHostPortPath(
    char *uri,
    char *hostName, size_t hostNameBufSize,
    size_t& hostNameBufSizeRequested,
    unsigned short & portNumber,
    char *relativePath, size_t relativePathBufSize,
    size_t& relativePathBufSizeRequested);

  /**
   * Extract host name from URI
   *
   * @param hostName - Ptr to client owned buffer
   * @param hostNameBufSize - Size of client owned buffer
   * @param hostNameBufSizeRequested - Size necessary for client
   *                                 owned buffer.
   * @param ptrHost - Ptr to hostname in URI.
   */
  static void PopulateHostName(
    char *hostName,
    size_t hostNameBufSize,
    size_t& hostNameBufSizeRequested,
    const char *ptrHost);

  /**
   * Extract port number from URI
   *
   * @param portNumber - Client owned ptr to unsigned short
   * @param ptrPort - Ptr to start of Port Number in URI.
   */
  static void PopulatePortNumber(
    unsigned short& portNumber,
    const char *ptrPort);

  /**
   * Extract relative path from URI
   *
   * @param relativePath  - Ptr to client owned string.
   * @param relativePathBufSize - Size of client owned string
   *                            buffer.
   * @param relativePathBufSizeRequested - Size necessary for
   *                                     client owned buffer
   * @param ptrRelativeUrl  - Ptr to start of relative path of
   *                        URI. If it is NULL, then relative path
   *                        is "/".
   */
  static void PopulateRelativePath(
    char *relativePath,
    size_t relativePathBufSize,
    size_t& relativePathBufSizeRequested,
    const char *ptrRelativeUrl);


};


class HTTPTransferEncodingFilter
{
public:
  /**
   * HTTPTransferEncodingFilter C'tor
   *
   * @return none
   */
  HTTPTransferEncodingFilter();

  /**
   * HTTPTransferEncodingFilter D'tor
   *
   * @return none
   */
  ~HTTPTransferEncodingFilter();

  /**
   * Reset transfer encoding filter
   *
   * @return none.
   */
  void Reset();

  /**
   * checks transfer encoding was set or not
   *
   * @return true if transfer encoding set, fasle otherwise.
   */
  bool IsTransferEncodingSet() const;

  /**
   * Extract host name from URI
   *
   * @param encoding - value ptr of "transfer-encoding"
   *
   * @return none.
   */
  void SetTransferEncoding(const char *encoding);

  /**
   * checks transfer-encoding complete or not
   *
   * @return true if transfer encoding complete, fasle otherwise.
   */
  bool IsTransferEncodingComplete() const;

  /**
   * This fucntions lets client to give filter initial unused (yet to parse) data
   * @param[in]  buf - Pointer to client buffer
   * @param[in]  bufSize - The size of client buffer
   *
   * @return none.
   */
  void FeedData(char *buf, size_t bufSize);

  /**
   * Decodes encoded data and give it back to client
   *
   * @param httpConnection - http transport connection
   * @param[in]  readBuf - Pointer to client buffer
   * @param[in]  readBufSize - The size of client buffer
   * @param[out] readLen - Number of bytes of client buffer
   *                       that were filled.
   * @return success code.
   */
  HTTPReturnCode DecodeData(TransportConnection* httpConnection,
                            char *readBuf, size_t readBufSize, size_t& readLen);

private:
  /**
   * Recv data from transport connection
   *
   * @param httpConnection - http transport connection
   * @param[in]  buf - Pointer to client buffer
   * @param[in]  maxToRead - The size of client buffer
   * @param[out] numRead - Number of bytes of client buffer
   *                       that were filled.
   * @return success code.
   */
  HTTPReturnCode RecvData(TransportConnection* httpConnection,
                          char *buf, size_t maxToRead, size_t &numRead);

  /**
   * decode chunk header
   *
   * @param httpConnection - http transport connection
   *
   * @return true if needs more data, false otherwise.
   */
  HTTPReturnCode DecodeChunkHeader(TransportConnection* httpConnection);

  /**
   * decode chunk data
   *
   * @param httpConnection - http transport connection
   * @param[in]  readBuf - Pointer to client buffer
   * @param[in]  readBufSize - The size of client buffer
   * @param[out] readLen - Number of bytes of client buffer
   *                       that were filled.
   *
   * @return true if needs more data, false otherwise.
   */
  HTTPReturnCode DecodeChunkData(TransportConnection* httpConnection,
                                 char *readBuf, size_t readBufSize, size_t& readLen);

private:
  /**
   * Set to true if there is a key-value pair in the header  for
   * 'Transfer-Encoding'. This value is valid only after the rsp
   * hdrs are fully received.
   */
  bool m_IsTransferEncodingSet;

  ///buffer size and length for holding read data from transport layer
  char *m_buffer;
  size_t m_bufLen;

  ///decoding state maintains decoding context between data read calls
  enum DecodingState
  {
    DS_CHUNK_HDR,
    DS_CHUNK_DATA,
    DS_CHUNK_LAST
  }m_state;

  ///marker marks a point used for next read call
  const char *m_marker;

  ///remanining chunk size of the current chunk
  size_t m_chunkSize;

  ///maximum buffer size
  static const size_t BUF_SIZE = 8192;
};

} // end namespace video


#endif /* HTTPSTACKCOMMON_H */
