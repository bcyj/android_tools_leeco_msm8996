/************************************************************************* */
/**
 * HTTPStackCommon.cpp
 * @brief implementation of the HTTPStackCommon.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPStackCommon.cpp#18 $
$DateTime: 2012/07/23 17:07:44 $
$Change: 2621737 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPStackCommon.h"

#include "SourceMemDebug.h"
#include "qtv_msg.h"

#include "StreamNetwork.h"
#include "IPStreamSourceUtils.h"

namespace video
{

const char* HTTPStackCommon::HTTP_VERSION = "HTTP/1.1";

const char *HTTPStackCommon::AUTHENTICATE_KEY = "WWW-Authenticate";
const char *HTTPStackCommon::AUTHORIZATION_KEY = "Authorization";
const char *HTTPStackCommon::PROXY_AUTHENTICATE_KEY = "Proxy-Authenticate";
const char *HTTPStackCommon::PROXY_AUTHORIZATION_KEY = "Proxy-Authorization";
const char *HTTPStackCommon::RANGE_KEY = "Range";

HTTPStackCommon::HTTPStackCommon()
{

}

HTTPStackCommon::~HTTPStackCommon()
{

}

/**
 * @brief:
 *  Extract the hostname, port, relative url.
 *  eg. url: "http://A.B.C.D:pp/somepath
 *  HostName : "A.B.C.D", port: pp, relativePath: "somepath"
 *  Special cases:
 *  a. 'http://' missing: hostName starts at url[0]
 *  b. Relative path missing. Theb relativePath = "/"
 */
HTTPReturnCode
HTTPStackCommon::GetHostPortRelativePathFromUrl(
                          const char *url, size_t urlLen,
                          char *hostName, size_t hostNameBufSize,
                          size_t& hostNameBufSizeRequested,
                          unsigned short & portNumber,
                          char *relativePath, size_t relativePathBufSize,
                          size_t& relativePathBufSizeRequested)
{
  HTTPReturnCode result = HTTP_SUCCESS;

  hostNameBufSizeRequested = 0;
  portNumber = 0;
  relativePathBufSizeRequested = 0;

  // param 'url' is untrusted and may not be null terminated.
  // localUrl ensures it is null terminated.
  char *localUrl = NULL;

  if ((urlLen > (size_t)MAX_URL_LEN))
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPStackCommon::GetHostAndPortFromUrl - UrlLen '%d' invalid",
      urlLen);

    result = HTTP_BADPARAM;
  }
  else
  if (NULL == url)
  {
    // err msg
    result = HTTP_BADPARAM;
  }
  else
  {
    localUrl = (char *)QTV_Malloc((urlLen + 1) * sizeof(char));
    if (NULL == localUrl)
    {
      result = HTTP_FAILURE;

      QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPStackCommon::GetHostAndPortFromUrl - Failed to allocate localUrl");
    }
    else
    {
      std_strlcpy(localUrl, url, urlLen + 1);
      result = HTTP_SUCCESS;
    }
  }

  if (HTTP_SUCCESS == result)
  {
    PopulateHostPortPath(localUrl,
                         hostName, hostNameBufSize,
                         hostNameBufSizeRequested,
                         portNumber,
                         relativePath, relativePathBufSize,
                         relativePathBufSizeRequested);
  }

  if((portNumber == 0) ||
     (hostNameBufSizeRequested == 0) ||
     (relativePathBufSizeRequested == 0))
  {
    QTV_MSG_PRIO3( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "HTTPStackCommon::GetHostAndPortFromUrl - Port '%d', or  "
                   "hostNameBufSizeRequested '%d' or relativePathBufSizeRequested '%d'"
                   "is zero", portNumber, hostNameBufSizeRequested, relativePathBufSizeRequested);

    result = HTTP_FAILURE;
  }

  if (localUrl)
  {
    QTV_Free(localUrl);
    localUrl = NULL;
  }

  return result;
}

/**
 * Determine length of hostname and path or
 * populate host, port, path in client buffers.
 *
 * @param localUrl
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
void
HTTPStackCommon::PopulateHostPortPath(
  char *uri,
  char *hostName, size_t hostNameBufSize,
  size_t& hostNameBufSizeRequested,
  unsigned short & portNumber,
  char *relativePath, size_t relativePathBufSize,
  size_t& relativePathBufSizeRequested)
{
  char *posHost = NULL;

  // Point posHost to start of hostname
  if (uri)
  {
    posHost = std_strstr(uri, "http://");
    if (NULL == posHost)
    {
      posHost = uri;
    }
    else
    {
      posHost += std_strlen("http://");
    }
  }

  // Point posRelativeUrl to start of relative path.
  // If there is no relative path, then "/" is
  // considered the relative path
  char *posRelativeUrl = NULL;
  if (posHost)
  {
    posRelativeUrl = std_strstr(posHost, "/");
  }

  PopulateRelativePath(relativePath,
                       relativePathBufSize,
                       relativePathBufSizeRequested,
                       posRelativeUrl);

  if (posRelativeUrl)
  {
    *posRelativeUrl = 0; // No longer needed
  }

  char *posPort = NULL;
  if (posHost)
  {
    posPort = std_strstr(posHost, ":");
  }

  if (posPort)
  {
    *posPort = 0;
    ++posPort;
  }

  PopulatePortNumber(portNumber, posPort);

  // If valid, posHost points to hostname, posPort points to portNumber,
  // posRelativePath points to relativePath except that "/" is relaced by 0.

  PopulateHostName(hostName, hostNameBufSize, hostNameBufSizeRequested, posHost);
}

/**
 * Extract host name from URI
 *
 * @param hostName - Ptr to client owned buffer
 * @param hostNameBufSize - Size of client owned buffer
 * @param hostNameBufSizeRequested - Size necessary for client
 *                                 owned buffer
 * @param ptrHost - Ptr to hostname in URI.
 */
void
HTTPStackCommon::PopulateHostName(
  char *hostName,
  size_t hostNameBufSize,
  size_t& hostNameBufSizeRequested,
  const char *ptrHost)
{
  if (ptrHost)
  {
    hostNameBufSizeRequested = std_strlen(ptrHost) + 1;

    if (hostName)
    {
      std_strlcpy(hostName, ptrHost, hostNameBufSize);
    }
  }
}

/**
 * Extract port number from URI
 *
 * @param portNumber - Client owned ptr to unsigned short
 * @param ptrPort - Ptr to start of Port Number in URI.
 */
void
HTTPStackCommon::PopulatePortNumber(
  unsigned short& portNumber,
  const char *ptrPort)
{
  portNumber = DEFAULT_HTTP_PORT;
  uint32 temp_portNumber;
  if (ptrPort)
  {
    temp_portNumber = atoi(ptrPort);
    if (!temp_portNumber)
    {
       // not able to parse
       QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPStackCommon::PopulatePortNumber - Error in  parsing");
    }
    else
    {
      portNumber = (unsigned short)temp_portNumber;
    }
  }
}

/**
 * Extract relative path from URI
 *
 * @param relativePath  - Ptr to client owned string.
 * @param relativePathBufSize - Size of client owned string
 *                            buffer.
 * @param relativePathBufSizeRequested - Size necessary for
 *                                     client owned buffer.
 * @param ptrRelativeUrl  - Ptr to start of relative path of
 *                        URI. If it is NULL, then relative path
 *                        is "/".
 */
void
HTTPStackCommon::PopulateRelativePath(
  char *relativePath,
  size_t relativePathBufSize,
  size_t& relativePathBufSizeRequested,
  const char *ptrRelativeUrl)
{
  // Set relativeBufSizeRequested and/or
  // populate relativePath
  if (ptrRelativeUrl)
  {
    relativePathBufSizeRequested = std_strlen(ptrRelativeUrl) + 1;

    if (relativePath)
    {
      std_strlcpy(relativePath, ptrRelativeUrl, relativePathBufSize);
    }
  }
  else
  {
    relativePathBufSizeRequested = std_strlen("/") + 1;

    if (relativePath)
    {
      std_strlcpy(relativePath, "/", relativePathBufSize);
    }
  }
}

HTTPReturnCode
HTTPStackCommon::MapTransportResultCode(TransportConnection::ResultCode rslt)
{
  HTTPReturnCode result = HTTP_FAILURE;

  switch(rslt)
  {
  case TransportConnection::SUCCESS:
    result = HTTP_SUCCESS;
  break;

  case TransportConnection::EWAITING:
  case TransportConnection::ENETWAITING:
    result = HTTP_WAIT;
  break;

  case TransportConnection::ENOMEMORY:
    result = HTTP_FAILURE;
  break;

  case TransportConnection::ECLOSEDBYPEER:
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                 "HTTPStack Connection closed by peer");
    result = HTTP_FAILURE;
  break;

  case TransportConnection::ESOCKERROR:
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "HTTPStack Socket error");
    result = HTTP_FAILURE;
  break;

  default:
    result = HTTP_FAILURE;
    break;
  }

  return result;
}

/**
 * @brief
 *  Get the string for the http method type.
 *
 * @param method
 *
 * @return const char*
 */
const char* HTTPStackCommon::GetStringForMethod(HTTPMethodType method)
{
  const char *methodName = NULL;

  switch (method)
  {
  case HTTP_HEAD:
    methodName = "HEAD";
    break;
  case HTTP_GET:
    methodName = "GET";
    break;
  case HTTP_POST:
    methodName = "POST";
    break;
  case HTTP_PUT:
    methodName = "PUT";
    break;
  case HTTP_DELETE:
    methodName = "DELETE";
    break;
  case HTTP_OPTIONS:
    methodName = "OPTIONS";
    break;
  case HTTP_CONNECT:
    methodName = "CONNECT";
    break;
  default:
    break;
  }

  return methodName;
}

/**
 * HTTPTransferEncodingFilter C'tor
 *
 * @return none
 */
HTTPTransferEncodingFilter::HTTPTransferEncodingFilter()
{
  m_buffer = QTV_New_Array(char, BUF_SIZE);
  Reset();
}

/**
 * HTTPTransferEncodingFilter D'tor
 *
 * @return none
 */
HTTPTransferEncodingFilter::~HTTPTransferEncodingFilter()
{
  if (m_buffer)
  {
    QTV_Delete_Array(m_buffer);
  }
  Reset();
}

/**
 * Reset transfer encoding filter
 *
 * @return none.
 */
void HTTPTransferEncodingFilter::Reset()
{
  m_IsTransferEncodingSet = false;
  m_bufLen = 0;
  m_state = DS_CHUNK_HDR;
  m_marker = (const char *)m_buffer;
  m_chunkSize = 0;
}

/**
 * checks transfer encoding was set or not
 *
 * @return true if transfer encoding set, fasle otherwise.
 */
bool HTTPTransferEncodingFilter::IsTransferEncodingSet() const
{
  return m_IsTransferEncodingSet;
}

/**
 * Extract host name from URI
 *
 * @param encoding - value ptr of "transfer-encoding"
 *
 * @return none.
 */
void HTTPTransferEncodingFilter::SetTransferEncoding(const char *encoding)
{
  m_IsTransferEncodingSet = !std_stricmp(encoding, "chunked");
}

/**
 * checks transfer-encoding complete or not
 *
 * @return true if transfer encoding complete, fasle otherwise.
 */
bool HTTPTransferEncodingFilter::IsTransferEncodingComplete() const
{
  return (DS_CHUNK_LAST == m_state);
}

/**
 * This fucntions lets client to give filter initial unused (yet to parse) data
 * @param[in]  buf - Pointer to client buffer
 * @param[in]  bufSize - The size of client buffer
 *
 * @return none.
 */
void HTTPTransferEncodingFilter::FeedData(char *buf, size_t bufSize)
{
  if (buf && m_buffer)
  {
    if (bufSize > BUF_SIZE)
    {
      QTV_Delete_Array(m_buffer);
      m_buffer = QTV_New_Array(char, BUF_SIZE);
    }

    if (m_buffer)
    {
      //copy partial buffer which is left after header parsing
      memcpy(m_buffer, buf, bufSize);
      m_bufLen = bufSize;
    }

    //reset decoding context as feeddata indicates beginning of a new session
    m_marker = m_buffer;
  }

  return;
}

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
HTTPReturnCode HTTPTransferEncodingFilter::DecodeData(
                   TransportConnection* httpConnection,
                   char *readBuf, size_t readBufSize,
                   size_t& readLen)
{
  HTTPReturnCode result = HTTP_SUCCESS;
  size_t actualRead = 0;
  const int MAX_LOOP_ITERATIONS = 20;
  int loopCount = MAX_LOOP_ITERATIONS;
  readLen = 0;

  //read more data
  if (!m_bufLen && m_marker)
  {
    result = RecvData(httpConnection, (char *)m_marker+m_bufLen,
                      (BUF_SIZE - (m_marker-m_buffer)), actualRead);
    m_bufLen += actualRead;

    //check recv data length, otherwise reset filter and bailout
    if ((HTTP_FAILURE == result) ||
        ((m_bufLen <= 0) && (HTTP_WAIT != result)))
    {
      readLen += m_bufLen;
      m_state = DS_CHUNK_LAST;
    }
  }

  while((readBufSize != readLen) &&
        (HTTP_SUCCESS == result) &&
        (loopCount > 0))
  {
    switch(m_state)
    {
      case DS_CHUNK_HDR:
      {
        result = DecodeChunkHeader(httpConnection);
      }
      break;

      case DS_CHUNK_DATA:
      {
        result = DecodeChunkData(httpConnection, readBuf, readBufSize, readLen);
      }
      break;

      case DS_CHUNK_LAST:
      {
        //terminate all \r\n
        const char *start = m_marker;
        m_marker = skip_whitespace_and_line_term(start, start + m_bufLen);
        m_bufLen -= m_marker - start;
        loopCount = 0;
        //we are done with all chunks stay in this state until filter reset
      }
      break;
    } /*switch(m_state)*/

    //loopcount protects from excessive looping which
    //triggered ny etreme low bitrates
    loopCount--;
  }

  return result;
}

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
HTTPReturnCode HTTPTransferEncodingFilter::RecvData(TransportConnection* httpConnection,
                                           char *buf, size_t maxToRead, size_t &numRead)
{
  HTTPReturnCode result = HTTP_FAILURE;
  size_t readBytes = 0;
  TransportConnection::ResultCode rslt = httpConnection->Recv(
                                    buf, maxToRead, readBytes);
  numRead = readBytes;
  result = HTTPStackCommon::MapTransportResultCode(rslt);
  return result;
}

/**
 * decode chunk header
 *
 * @param httpConnection - http transport connection *
 * @return true if needs more data, false otherwise.
 */
HTTPReturnCode HTTPTransferEncodingFilter::DecodeChunkHeader(TransportConnection* httpConnection)
{
  const char *extStart = NULL;
  const char *tmp = NULL, *crlfStart = NULL;
  HTTPReturnCode result = HTTP_SUCCESS;
  const char *start = m_marker;
  const char *end = start + m_bufLen;

  //trim all \r\n in case of last chunk and no harm in doing when
  //data decodng begins as well
  start = skip_whitespace_and_line_term(start, end);
  m_bufLen -= (start - m_marker);
  m_marker = start;

  //read chunk-size, chunk-extension (if any) and CRLF
  crlfStart = skip_to_line_term(start, end);

  if (crlfStart && (crlfStart < end))
  {
    extStart = skip_to_char(start, crlfStart, ';');
    tmp = skip_to_whitespace(start, crlfStart);

    if (extStart && (extStart < crlfStart) && (extStart < tmp))
    {
      tmp = extStart;
    }

    int pError;
    m_chunkSize = std_scanul(start, 16, &tmp, &pError);

    //skip until chunk data start
    //Skip CRLF after that there will be chunk data, so skip only 2 bytes
    // from crlfStart
    m_marker = crlfStart + 2;
    m_bufLen -= (m_marker - start);

    m_state = DS_CHUNK_DATA;
    if (!m_chunkSize)
    {
      //encountered last chunk
      m_state = DS_CHUNK_LAST;
    }
  }
  else
  {
    size_t actualRead = 0;
    result = RecvData(httpConnection, (char *)m_marker + m_bufLen,
                      (BUF_SIZE - (m_marker-m_buffer) - m_bufLen),
                      actualRead);
    m_bufLen += actualRead;
  } /*if (crlfStart && (crlfStart < end))*/

  return result;
}

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
HTTPReturnCode HTTPTransferEncodingFilter::DecodeChunkData(TransportConnection* httpConnection,
                                                           char *readBuf, size_t readBufSize,
                                                           size_t& readLen)
{
  HTTPReturnCode result = HTTP_SUCCESS;
  size_t actualRead = 0;
  size_t size = 0;
  const char *start = NULL, *end = NULL;

  if (!m_marker)
  {
    size = QTV_MIN(m_chunkSize,
                   (readBufSize > readLen)? (uint32)(readBufSize - readLen) : 0);
    result = RecvData(httpConnection, (char *)readBuf + readLen,
                      size, actualRead);
    readLen += actualRead;
    m_chunkSize -= size;
  }
  else
  {
    start = m_marker;
    end = start + m_bufLen;
  }

  if (m_marker && m_chunkSize)
  {
    size = QTV_MIN(m_chunkSize, (uint32)m_bufLen);
    size = QTV_MIN(size,
           (readBufSize > readLen)? (uint32)(readBufSize - readLen) : 0);

    m_chunkSize -= size;

    //read chunk-data and CRLF
    memcpy(readBuf+readLen, start, size);
    m_marker = start = start + size;
    readLen += size;
    m_bufLen -= size;
    if (!m_bufLen)
    {
      //switch to direct client buffer
      m_marker = NULL;
    }
  }

  if (!m_chunkSize)
  {
    //decoding of this chunk completed, move on to next chunk
    m_state = DS_CHUNK_HDR;

    if (m_marker)
    {
      //terminate all \r\n
      start = skip_whitespace_and_line_term(start, end);
      m_bufLen -= (start - m_marker);
      m_marker = start;
    }
    else
    {
      //switch to filter buffer for header parsing
      m_marker = m_buffer;
      m_bufLen = 0;
    }
  }

  return result;
}

} // end namespace video
