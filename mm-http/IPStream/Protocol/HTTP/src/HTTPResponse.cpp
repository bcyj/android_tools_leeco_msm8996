/************************************************************************* */
/**
 * HTTPResponse.cpp
 * @brief implementation of the HTTPResponse.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPResponse.cpp#39 $
$DateTime: 2012/09/04 07:38:31 $
$Change: 2762671 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

// Streamer
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPStackCommon.h"
#include "oscl_string_utils.h"
#include "SourceMemDebug.h"
#include "qtv_msg.h"
#include "IPStreamSourceUtils.h"

#define CSTRLEN(x) ((int)sizeof(x)-1)

#define GETCSTRHEADERVALUE(key, value, valueLen, valueLenReq) \
    GetHeaderValue(key,CSTRLEN(key),value, valueLen, valueLenReq)

namespace video
{

/**
 * Default ctor
 */
HTTPResponse::HTTPResponse() :
  m_Buffer(NULL),
  m_ReasonPhrase(NULL),
  m_nTotalContentLength(-1),
  m_ContentType(NULL),
  m_FlagInvalidContentLength(false),
  m_RspState(RSP_INITIAL),
  m_NumBytesInEntityBody(0),
  m_TmpLocalBuffer(NULL),
  m_TmpLocalBufferSize(0)
{

  ordered_StreamList_init(&m_CookieValueList,
                          ORDERED_STREAMLIST_ASCENDING,
                          ORDERED_STREAMLIST_PUSH_SLT);
  Reset();
}

/**
 * Destructor
 */
HTTPResponse::~HTTPResponse()
{
  if (m_TmpLocalBuffer)
  {
    QTV_Free(m_TmpLocalBuffer);
    m_TmpLocalBufferSize = 0;
  }

  if (m_ContentType)
  {
    QTV_Free(m_ContentType);
    m_ContentType = NULL;
  }
  if (m_ReasonPhrase)
  {
    QTV_Free(m_ReasonPhrase);
    m_ReasonPhrase = NULL;
  }
  if (m_Buffer)
  {
    QTV_Free(m_Buffer);
    m_Buffer = NULL;
  }

  // Free all the elements in the Cookie value list
  HTTPCookieElement *pCookieElement = NULL;
  pCookieElement = (HTTPCookieElement *)
                      ordered_StreamList_peek_front(&m_CookieValueList);
  while (pCookieElement)
  {
    //Remove the Cookie element from the cookie value list
    ordered_StreamList_pop_item(&m_CookieValueList, &pCookieElement->link);

    //Free list element
    QTV_Free(pCookieElement);

    pCookieElement = (HTTPCookieElement *)
                      ordered_StreamList_peek_front(&m_CookieValueList);
    continue;
  }
}

/**
 * Resets private variables.
 */
void
HTTPResponse::Reset()
{
  m_HTTPServerVersion = 0;
  m_HTTPResponseCode = 0;

  if(m_Buffer)
  {
    QTV_Free(m_Buffer);
    m_Buffer = NULL;
  }

  m_BufferLen = 0;
  m_NumBytesReadInBuffer = 0;
  m_ResponseHeaderLen = 0;
  m_NumDataBytesReadByClient = 0;

  m_ContentLength = -1;

  m_IsPersistentConnection = false;

  if (m_ContentType)
  {
    QTV_Free(m_ContentType);
    m_ContentType = NULL;
  }

  if (m_ReasonPhrase)
  {
    QTV_Free(m_ReasonPhrase);
    m_ReasonPhrase = NULL;
  }

  SetRspStateInitial();

  m_EntityBody[0] = '\0';
  m_NumBytesInEntityBody = 0;

  // Free all the elements in the Cookie value list
  HTTPCookieElement *pCookieElement = NULL;
  pCookieElement = (HTTPCookieElement *)
                      ordered_StreamList_peek_front(&m_CookieValueList);
  while (pCookieElement)
  {
    //Remove the Cookie element from the cookie value list
    ordered_StreamList_pop_item(&m_CookieValueList, &pCookieElement->link);

    //Free list element
    QTV_Free(pCookieElement);

    pCookieElement = (HTTPCookieElement *)
                      ordered_StreamList_peek_front(&m_CookieValueList);
    continue;
  }
}

HTTPResponseStatus
HTTPResponse::ReceiveResponse(TransportConnection* httpConnection)
{
  HTTPResponseStatus result = HTTP_RSP_FAILURE;

  if (true == IsResponseHeaderFullyReceived())
  {
    result = HTTP_RSP_SUCCESS;
  }
  else
  {
    if (RSP_ERROR == m_RspState)
    {
      result = HTTP_RSP_FAILURE;
    }
    else
    {
      if (RSP_INITIAL == m_RspState)
      {
        if(true == PrepareBuffer())
        {
          SetRspStateWaitingForHdrs();
          result = HTTP_RSP_SUCCESS;
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                       "HTTPResponse::ReceiveResponse PrepareBuffer FAILED");

          SetRspStateError();
          result = HTTP_RSP_FAILURE;
        }
      }

      if (RSP_WAITING_FOR_HDRS == m_RspState)
      {
        if (m_BufferLen == m_NumBytesReadInBuffer)
        {
          if (true != IncreaseRspBufferSize())
          {
            SetRspStateError();
            result = HTTP_RSP_FAILURE;
          }
        }

        if (RSP_ERROR != m_RspState)
        {
          result = ReceiveHTTPResponse(httpConnection);

          if (HTTP_RSP_SUCCESS == result)
          {
            if ((0 == m_ContentLength) && (false == m_FlagInvalidContentLength))
            {
              SetRspStateDone();
               result = HTTP_RSP_DONE;

            }
            else
            {
              // Either contentLen is not recd, or contentLen was received
              // and is valid and is > 0.
              SetRspStateHeadersRecd();
              result = HTTP_RSP_HDRS_RECVD;
            }
          }
          else
          {
            if (HTTP_RSP_WAIT != result)
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                           "Failed to receive HTTP rsp headers");

              SetRspStateError();
              result = HTTP_RSP_FAILURE;
            }
          }
        }
      }
    }
  }

  return result;
}

/**
 * @brief Frees old buffer if exists and creates a new one.
 *
 * @return bool
 */
bool
HTTPResponse::PrepareBuffer()
{
  bool result = false;

  if (m_Buffer)
  {
    QTV_Free(m_Buffer);
    m_Buffer = NULL;
  }

  m_Buffer = (char *)QTV_Malloc((HTTP_RSP_HDR_BUF_INITIAL_LEN + 1) * sizeof(char));

  if (NULL == m_Buffer)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPResponse::ReceiveHTTPResponse - m_Buffer is NULL");
  }
  else
  {
    m_BufferLen =  HTTP_RSP_HDR_BUF_INITIAL_LEN;
    memset(m_Buffer, 0, m_BufferLen + 1);

    result = true;
  }

  return result;
}

bool
HTTPResponse::IncreaseRspBufferSize()
{
  bool result = false;

  QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
               "Need to resize m_Buffer");

  if (m_BufferLen >= (size_t)HTTPStackCommon::MAX_HTTP_RSP_HDR_BUF_LEN)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "IncreaseRspBufferSize() Failed to increase buffer size. "
                 "Rsp buffer len already at MAX_RSP_HEADER_STR_LEN");
  }
  else
  {
    size_t reallocMaxStrLen = QTV_MIN(2*m_BufferLen,
                                   (size_t)HTTPStackCommon::MAX_HTTP_RSP_HDR_BUF_LEN);

    char *reallocBuffer = NULL;
    reallocBuffer = (char *)QTV_Malloc((reallocMaxStrLen + 1) * sizeof(char));
    if (NULL == reallocBuffer)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "Failed to resize m_Buffer");
    }
    else
    {
      memset(reallocBuffer, 0, reallocMaxStrLen + 1);
      memcpy(reallocBuffer, m_Buffer, m_BufferLen);
      m_BufferLen = reallocMaxStrLen;

      if (m_Buffer)
      {
        QTV_Free(m_Buffer);
      }

      m_Buffer = reallocBuffer;

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                    "HTTPResponse::ReceiveHTTPResponse "
                    "Realloc'ed m_Buffer to size '%llu' + 1",
                    m_BufferLen);

      result = true;
    }
  }

  return result;
}

/**
 * @brief:
 *  Reads from socket if response header is not yet fully received.
 *
 * @returns:
 *  true: Tried to read from socket and no unrecoverable error occured.
 *  false: Unrecoverable error occured.
 */
HTTPResponseStatus
HTTPResponse::ReceiveHTTPResponse(TransportConnection* httpConnection)
{
  HTTPResponseStatus result = HTTP_RSP_FAILURE;

  size_t numToRead = m_BufferLen - m_NumBytesReadInBuffer;


  if (NULL == m_Buffer)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPResponse::ReceiveHTTPResponse - m_Buffer is NULL");
  }
  else
  {
    if (numToRead == 0)
    {
      if (true == IsResponseHeaderFullyReceived())
      {
        result = HTTP_RSP_SUCCESS;
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "HTTPResponse::ReceiveHTTPResponse Zero bytes to read "
                     "but rsp headers not fully received");
      }
    }
    else
    {
      size_t numRead = 0;

      TransportConnection::ResultCode rslt =
        httpConnection->Recv(m_Buffer + m_NumBytesReadInBuffer,
                             numToRead,
                             numRead);
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"Recv status :%d",rslt);

      result = MapTransportResultCode(rslt);


      if (numRead > 0)
      {
        m_NumBytesReadInBuffer += numRead;

        char *posEndOfHTTPHeader = std_strstr(m_Buffer, "\r\n\r\n");

        if (NULL != posEndOfHTTPHeader)
        {
          m_ResponseHeaderLen = (posEndOfHTTPHeader - m_Buffer) + (int)CSTRLEN("\r\n\r\n");

          QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_LOW, "RESPONSE HEADER:");
          for (uint64 i=0; i < m_ResponseHeaderLen; ++i)
          {
            QTV_MSG_PRIO3(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_LOW,
              "%llu: ascii '%d' %c", i, m_Buffer[i], m_Buffer[i]);
          }

          if (false == ExtractHTTPResponseCode())
          {
            result = HTTP_RSP_FAILURE;
            QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
              "Failed to parse response code");
          }

          ExtractHTTPResponseReasonPhrase();
          ExtractRelevantHeaderValues();

          result = HTTP_RSP_SUCCESS;
        }
        else
        {
          if ((HTTP_RSP_SUCCESS == result) || (HTTP_RSP_WAIT == result))
          {
            result = HTTP_RSP_WAIT;
          }
        }
      }
    }

  } // end if (m_IsResponseHeaderFullyReceived...)

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,"RecieveResponse:Response status: %d",result);
  return result;
}

/**
 * @brief: Returns HTTP Data that was read. Also, updates
 *         internal bookkeeping to advance the location of
 *         position from which data will be read the next time
 *         the function is called.
 *
 * @param[in]  readBuf - Pointer to client buffer
 * @param[in]  readBufSize - The size of client buffer
 * @param[out] readLen - Number of bytes of client buffer
 *                       that were filled.
 *
 * Returns HTTPReturnCode
 *  HTTP_SUCCESS      At least one byte written to client
 *                    buffer.
 *  HTTP_WAIT         Zero bytes written to client buffer
 *                    because waiting for data in socket.
 *  HTTP_FAILURE
 */
HTTPResponseStatus
HTTPResponse::GetData(TransportConnection* httpConnection,
                      char *readBuf,
                      size_t readBufSize,
                      size_t& readLen)
{
  HTTPResponseStatus result = HTTP_RSP_FAILURE;
  readLen = 0;

  bool isConnectionClosedByPeer = false;

  if ((true == IsResponseHeaderFullyReceived()) &&
        (RSP_HDRS_RECEIVED == m_RspState))
  {
      SetRspStateDownloadingData();
  }
  if (readBuf && readBufSize > 0)
  {
    if (RSP_DOWNLOADING_DATA == m_RspState)
    {
      // Transfer initial data in buffer to client buffer if necessary.

      size_t numBytesOfInitalDataInBuffer = m_NumBytesReadInBuffer - m_ResponseHeaderLen;

      if (m_ContentLength > 0 &&  numBytesOfInitalDataInBuffer > (uint64)m_ContentLength)
      {
        // case where response buffer has response hrds or data for the next Request
        numBytesOfInitalDataInBuffer = (size_t)m_ContentLength;
      }

      size_t amtToCopy = 0;
      if (m_NumDataBytesReadByClient < numBytesOfInitalDataInBuffer)
      {
        amtToCopy = QTV_MIN(readBufSize,
                            numBytesOfInitalDataInBuffer - m_NumDataBytesReadByClient);

        if (amtToCopy > 0)
        {
          if (m_TransferEncodingFilter.IsTransferEncodingSet())
          {
            m_TransferEncodingFilter.FeedData(m_Buffer + m_ResponseHeaderLen +
                                              m_NumDataBytesReadByClient, amtToCopy);
          }
          else
          {
            memcpy(readBuf, m_Buffer + m_ResponseHeaderLen + m_NumDataBytesReadByClient,
                   amtToCopy);

            m_NumDataBytesReadByClient += amtToCopy;

           // Update client buffer poitner and remaining buffer size
            readBuf += amtToCopy;
            readBufSize -= readBufSize;

            // Update numBytes read into client buffer
            readLen += amtToCopy;
          } /*if (m_TransferEncodingFilter.IsTransferEncodingSet())*/
        } /*if (amtToCopy > 0)*/
      } /*if (m_NumDataBytesReadByClient < numBytesOfInitalDataInBuffer)*/

      // Calculate max number of bytes to read from socket
      // (i) ContentLength specified in rsp hdrs && ContentLen is valid:
      //      Max bytes to read from socket =
      //        MIN((ContentLength - NumDataBytesReadByClient), readbufSize)
      // (ii) ContentLength not specified in rsp hdrs:
      //      Max bytes to read from socket = readBufSize.
      size_t maxToRead =
        (( (false == m_FlagInvalidContentLength)) ?
         QTV_MIN((size_t)m_ContentLength - m_NumDataBytesReadByClient, readBufSize) :
         readBufSize);

      if (maxToRead > 0)
      {
        size_t numRead = 0;
        if (m_TransferEncodingFilter.IsTransferEncodingSet())
        {
          //transfer-encoded content, decode it and give it back
          HTTPReturnCode rslt = m_TransferEncodingFilter.DecodeData(httpConnection, readBuf,
                                                       maxToRead, numRead);

          result = MapCommonResultCodeToRspCode(rslt);
        }
        else
        {
          TransportConnection::ResultCode rslt = httpConnection->Recv(readBuf,
                                                                      maxToRead,
                                                                      numRead);
          if (TransportConnection::ECLOSEDBYPEER == rslt)
          {
            isConnectionClosedByPeer = true;
          }

          result = MapTransportResultCode(rslt);
        }

        if (numRead > 0)
        {
          m_NumDataBytesReadByClient += numRead;
          readLen += numRead;
        }
      }
    }

    //
    // Data download is complete if client contentLength exists
    // and is Valid and client has downloaded entire amount, OR
    // Data download is complete if content length does not exist
    // and connection is closed by peer.
    //
    if (RSP_DONE != m_RspState)
    {
      if ((m_TransferEncodingFilter.IsTransferEncodingSet() && m_TransferEncodingFilter.IsTransferEncodingComplete()) ||
          ((false == m_FlagInvalidContentLength) && (m_NumDataBytesReadByClient == (uint64)m_ContentLength)) ||
          (((-1) == m_ContentLength || true == m_FlagInvalidContentLength) && (true == isConnectionClosedByPeer)))
      {
        SetRspStateDone();
        result = HTTP_RSP_DONE;
      }
    }

    if ((readLen > 0))
    {
      result = HTTP_RSP_SUCCESS;
    }

    if(RSP_DONE == m_RspState)
    {
      result = HTTP_RSP_DONE;
    }
  }
  else
  {
    TransportConnection::ResultCode rslt = httpConnection->GetBytesAvailableToRead((int &)readLen);
    if (rslt != TransportConnection::SUCCESS && rslt != TransportConnection::EWAITING)
    {
      readLen = 0;
    }
    if ( IsResponseHeaderFullyReceived() )
    {
      size_t numBytesInInitalDataInBuffer = m_NumBytesReadInBuffer - m_ResponseHeaderLen;
      if (m_NumDataBytesReadByClient < numBytesInInitalDataInBuffer)
      {
        size_t amtToCopyFromInitalBuffer = numBytesInInitalDataInBuffer - m_NumDataBytesReadByClient;
        if (amtToCopyFromInitalBuffer > 0)
        {
          if (readLen ==  0)
          {
            readLen = amtToCopyFromInitalBuffer;
          }
          else
          {
            readLen += amtToCopyFromInitalBuffer;
          }
        }
      }
    }
    result = HTTP_RSP_INSUFFBUFFER;
  }

  return result;
}

void
HTTPResponse::MarkRspDone()
{
  SetRspStateDone();
}

void
HTTPResponse::MarkRspError()
{
  SetRspStateError();
}

void
HTTPResponse::SetRspStateInitial()
{
  m_RspState = RSP_INITIAL;
}

void
HTTPResponse::SetRspStateWaitingForHdrs()
{
  m_RspState = RSP_WAITING_FOR_HDRS;
}

void
HTTPResponse::SetRspStateHeadersRecd()
{
  m_RspState = RSP_HDRS_RECEIVED;
}

void
HTTPResponse::SetRspStateDownloadingData()
{
  m_RspState = RSP_DOWNLOADING_DATA;
}

void
HTTPResponse::SetRspStateDone()
{
  m_RspState = RSP_DONE;
}

void
HTTPResponse::SetRspStateError()
{
  m_RspState = RSP_ERROR;
}

bool
HTTPResponse::IsProcessingAResponse() const
{
  return ((RSP_INITIAL == m_RspState) ||
          (RSP_DONE == m_RspState) ||
          (RSP_ERROR == m_RspState)
          ? false : true);
}

bool
HTTPResponse::IsResponseHeaderFullyReceived() const
{
  return (((RSP_HDRS_RECEIVED == m_RspState) ||
           (RSP_DOWNLOADING_DATA == m_RspState) ||
           (RSP_DONE == m_RspState))
          ? true : false);
}

/**
 * @brief
 *  Should be called only when the rsp hdrs are fully received.
 *  Partial usage of:
 *  http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html.
 *  Section 4.4 Message Length. Inferring message length from
 *  'Transfer-Encoding' and 'multipart/byteranges' header are
 *  not implemented.
 *
 * @return bool
 *  true    Http stack understands how to interpret message
 *          length.
 *  false   Http stack cannot interpret message length. Either
 *          not supported currently or server did not specify it
 *          correctly.
 */
bool
HTTPResponse::IsMessageLengthSpecifierSupported() const
{
  bool rslt = false;

  if (m_ContentLength >= 0)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,
                  "HTTPResponse::IsMessageLengthSpecifierSupported "
                  "Use content length '%lld' for message length",
                  m_ContentLength);

    rslt = true;
  }
  else
  {
    if (m_TransferEncodingFilter.IsTransferEncodingSet())
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "HTTPResponse::IsMessageLengthSpecifierSupported "
                   "Using Transfer encoding set");
      rslt = true;
    }
    else
    {
      bool isMultipartByteRangeSet = false;

      if(GetContentType())
      {
        static const int hdrStrLen = (int)CSTRLEN("multipart/byteranges");

        if (0 == std_strnicmp("multipart/byteranges",
                              GetContentType(),
                              hdrStrLen))
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                       "HTTPResponse::IsMessageLengthSpecifierSupported "
                       "Using multipart/byteranges not supported");
          isMultipartByteRangeSet = true;
        }
      }
      if (false == isMultipartByteRangeSet)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "HTTPResponse::IsMessageLengthSpecifierSupported "
                     "Use server connection termination for downloading");

        rslt = true;
      }

    }
  }

  if (true == m_FlagInvalidContentLength)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "HTTPResponse::IsMessageLengthSpecifierSupported Content length "
                 "if any will be disregarded");

    rslt = true;
  }

  return rslt;
}

bool
HTTPResponse::IsDownloadingData() const
{
  return (RSP_DOWNLOADING_DATA == m_RspState ? true : false);
}

int64
HTTPResponse::GetContentLength() const
{
  return m_ContentLength;
}

int64
HTTPResponse::GetTotalContentLength() const
{
  return m_nTotalContentLength;
}

const char *
HTTPResponse::GetContentType() const
{
  return m_ContentType;
}
const char *
HTTPResponse::GetEntityBody() const
{
  return m_EntityBody;
}
const char *
HTTPResponse::GetReasonPhraseStr() const
{
  return m_ReasonPhrase;
}

/**
 * True if HTTP response header has
 * Connection: Keep-Alive
 *
 * @return bool
 */
bool
HTTPResponse::IsConnectionPersistent() const
{
  return m_IsPersistentConnection;
}

void
HTTPResponse::SetContentLength(int contentLength)
{
  m_ContentLength = contentLength;
}

void
HTTPResponse::ResetContentLength()
{
  m_ContentLength = -1;
}

int
HTTPResponse::GetHTTPResponseCode() const
{
  return m_HTTPResponseCode;
}
bool
HTTPResponse::ExtractHTTPResponseReasonPhrase()
{
  bool result = false;
  if (NULL != m_ReasonPhrase)
  {
    QTV_Free (m_ReasonPhrase);
    m_ReasonPhrase = NULL;
  }
  if (NULL == m_Buffer)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractHTTPResponseReasonPhrase - m_Buffer is NULL");
  }
  else
  {
    char *posEndResponseLine = std_strstr(m_Buffer, "\r\n");
    char *posFirstSpace = std_strchr(m_Buffer, ' ');
    char *posOnePastEndOfStatusCode = NULL;

    if (NULL == posEndResponseLine)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractHTTPResponseReasonPhrase - End of Response Line not found in m_buffer");
    }
    posFirstSpace=skip_whitespace(posFirstSpace);

    if (NULL == posFirstSpace || posFirstSpace > posEndResponseLine)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractHTTPResponseReasonPhrase - Reason Phrase does not exist");
    }
    else
    {
      posOnePastEndOfStatusCode = std_strchr(posFirstSpace, ' ');
      posOnePastEndOfStatusCode = skip_whitespace(posOnePastEndOfStatusCode);
    }

    if (posOnePastEndOfStatusCode >= posEndResponseLine)
    {
      // Reason Phrase does not exist.
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
        "HTTPResponse::ExtractHTTPResponseReasonPhrase - ReasonPhrase does not exist");
    }
    else
    {

      if (posEndResponseLine && posOnePastEndOfStatusCode)
      {
         size_t reasonPhraseStrLen = posEndResponseLine - posOnePastEndOfStatusCode;
         m_ReasonPhrase = (char *)QTV_Malloc((reasonPhraseStrLen + 1) * sizeof(char));
         if (NULL == m_ReasonPhrase)
         {
           QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
             "HTTPResponse::ExtractHTTPResponseReasonPhrase - Failed to allocate m_ReasonPhrase");
           result = false;
         }
         else
         {
           std_strlcpy(m_ReasonPhrase, posOnePastEndOfStatusCode, reasonPhraseStrLen + 1);
           QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_LOW,
             "HTTPResponse::ExtractHTTPResponseReasonPhrase - HTTP Response code Reason Phrase '%s'",
              m_ReasonPhrase);
           result = true;
         }
      }
    }
  }
  return result;
}

bool
HTTPResponse::ExtractHTTPResponseCode()
{
  bool result = false;

  if (NULL == m_Buffer)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractHTTPResponseCode - m_Buffer is NULL");
  }
  else
  {
    // Get the server version
    static const char* version_string = "HTTP/";

    char *posEndResponseLine = std_strstr(m_Buffer, "\r\n");
    char *posVersionStart = std_strstri(m_Buffer, version_string);
    char *posFirstSpace = std_strchr(m_Buffer, ' ');
    char *posOnePastEndOfStatusCode = NULL;

    if (NULL == posEndResponseLine)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractHTTPResponseCode - End of Response Line not found in m_buffer");
    }

    if (posVersionStart && posFirstSpace)
    {
      posVersionStart += std_strlen(version_string);
      int versionLen = int(posFirstSpace - posVersionStart) + 1;

      size_t  reqdBufSize = versionLen * sizeof(char);
      if (m_TmpLocalBufferSize < reqdBufSize)
      {
        if (m_TmpLocalBuffer)
        {
          QTV_Free(m_TmpLocalBuffer);
          m_TmpLocalBufferSize = 0;

        }

        m_TmpLocalBuffer = (char *)QTV_Malloc(reqdBufSize);
      }

      if (m_TmpLocalBuffer)
      {
        std_strlcpy(m_TmpLocalBuffer, posVersionStart, reqdBufSize);
        m_HTTPServerVersion = atof(m_TmpLocalBuffer);
        m_TmpLocalBufferSize = reqdBufSize;
      }
    }

    posFirstSpace = skip_whitespace(posFirstSpace);
    if (NULL == posFirstSpace)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractHTTPResponseCode - Failed to find a space in m_buffer");
    }
    else
    {
      posOnePastEndOfStatusCode = std_strchr(posFirstSpace + 1, ' ');
      posOnePastEndOfStatusCode = skip_whitespace(posOnePastEndOfStatusCode);
    }

    if (posOnePastEndOfStatusCode > posEndResponseLine)
    {
      // Reason code does not exist.
      posOnePastEndOfStatusCode = posEndResponseLine;
    }

    char *responseCodeStr = NULL;

    if (posEndResponseLine && posFirstSpace && posOnePastEndOfStatusCode)
    {
      size_t responseCodeStrLen = posOnePastEndOfStatusCode - posFirstSpace;
      responseCodeStr = (char *)QTV_Malloc((responseCodeStrLen + 1) * sizeof(char));
      if(responseCodeStr == NULL)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
          "HTTPResponse::Could not allocate memory to responseCodeStr");
        return result;
      }
      std_strlcpy(responseCodeStr, posFirstSpace , responseCodeStrLen + 1);

      uint32 m_temp_HTTPResponseCode = atoi(responseCodeStr);

      if (!m_temp_HTTPResponseCode)
      {
        // not able to parse
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
         "HTTPResponse::ExtractHTTPResponseCode - Error in  parsing");
        result = false;
      }
      else
      {
        m_HTTPResponseCode = (int)m_temp_HTTPResponseCode;
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
        "HTTPResponse::ExtractHTTPResponseCode - HTTP Response code '%d'",
        m_HTTPResponseCode);

        result = true;
      }
    }

    if (responseCodeStr)
    {
      QTV_Free(responseCodeStr);
      responseCodeStr = NULL;
    }
  }

  return result;
}

void
HTTPResponse::ExtractRelevantHeaderValues()
{
  (void)ExtractMessageLength();
  (void)ExtractContentType();
  (void)ExtractTransferEncoding();
  (void)ExtractConnection();
  (void)ExtractCookieHeadersValue();

  return;
}
/**
 * @brief:
 *  Extracts the value of mesage length from the
 *  HTTP response headers.
 *
 * @return
 *  true - able to extract message length
 *  false - failed to extract message length
 */
bool
HTTPResponse::ExtractMessageLength()
{
  bool result = false;
  int nErr;
  char value[64];
  int ignored;

  if (GETCSTRHEADERVALUE("Content-Length", value, (int)STD_SIZEOF(value), ignored))
  {
    const char * end_ptr = NULL;
    m_nTotalContentLength = m_ContentLength = (size_t)std_scanul(value,
                                                      10,
                                                      &end_ptr,
                                                      &nErr);
    if (nErr == STD_NEGATIVE)
    {
      m_nTotalContentLength = m_ContentLength = -m_ContentLength;
    }

    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
      "HTTPResponse::ExtractMessageLength: Message Length: '%llu'",
       m_ContentLength);

    result = true;
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractMessageLength - Failed to extract content length");
  }

  //Also try to extract total content length from the content range (if exists)
  if (GETCSTRHEADERVALUE("Content-Range", value, (int)STD_SIZEOF(value), ignored))
  {
    char* posContentRange = std_strchr(value, '/');
    if (posContentRange)
    {
      const char * end_ptr = NULL;
      m_nTotalContentLength = (uint64)std_scanul(posContentRange + 1, 10,&end_ptr, &nErr);
      if (nErr == STD_NEGATIVE)
      {
        m_nTotalContentLength = -m_nTotalContentLength;
      }

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
        "HTTPResponse::ExtractMessageLength: Total Content Length: '%lld'",
         m_nTotalContentLength);
    }
  }

  return result;
}

/**
 * @brief:
 *  Extracts the value of content type from the
 *  HTTP response headers.
 *
 * @return
 *  true - able to extract the content type
 *  false - failed to extract content type
 */
bool
HTTPResponse::ExtractContentType()
{
  bool result = false;

  if (NULL == m_Buffer)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractContentType - m_Buffer is NULL");
  }
  else
  {

    int contentTypeStrBufLen = 0;
    result = GETCSTRHEADERVALUE("Content-Type", NULL, 0, contentTypeStrBufLen);

    if (true == result)
    {
      if (NULL != m_ContentType)
      {
        QTV_Free (m_ContentType);
        m_ContentType = NULL;
      }

      m_ContentType = (char *)QTV_Malloc (contentTypeStrBufLen * sizeof(char));

      if (NULL == m_ContentType)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
          "HTTPResponse::ExtractContentType - Failed to allocate m_ContentType");

        result = false;
      }
      else
      {
        int temp = 0;
        result = GETCSTRHEADERVALUE("Content-Type", m_ContentType, contentTypeStrBufLen, temp);
      }
    }
  }

  return result;
}

bool
HTTPResponse::ExtractTransferEncoding()
{
  const int ENCODING_MAX = 28;
  char encoding[ENCODING_MAX];

  int bufLen = 0;

  bool bTransferEncodingSet =
    GETCSTRHEADERVALUE("Transfer-Encoding", encoding, ENCODING_MAX, bufLen);

  m_TransferEncodingFilter.Reset();
  if (bTransferEncodingSet)
  {
    m_TransferEncodingFilter.SetTransferEncoding(encoding);
  }

  return bTransferEncodingSet;
}

/**
 * Connection is persistent if either keep-alive is set or
 * server version is 1.1 && Connection: close is not set.
 *
 * @return bool
 */
bool
HTTPResponse::ExtractConnection()
{
  bool result = false;
  m_IsPersistentConnection = (m_HTTPServerVersion > 1.0001 ? true : false);

  int connectionLen = 0;
  char *connectionValue = NULL;
  result = GETCSTRHEADERVALUE("Connection", NULL, 0, connectionLen);

  if (true == result)
  {
    int temp = 0;

    connectionValue = (char *)QTV_Malloc(connectionLen * sizeof(char));
    if (NULL == connectionValue)
    {
      result = false;
      QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPResponse::ExtractConnection Failed to allocate connectionKeepAliveValue");
    }
    else
    {
      GETCSTRHEADERVALUE("Connection", connectionValue, connectionLen, temp);
    }
  }

  if (connectionValue)
  {
    if (0 == std_strnicmp("Keep-Alive", connectionValue,
                        CSTRLEN("Keep-Alive") + 1))
    {
      m_IsPersistentConnection = true;
    }
    else if (0 == std_strnicmp("close", connectionValue,
                          CSTRLEN("close") + 1))
    {
      m_IsPersistentConnection = false;
    }
  }

   QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_LOW,
                 "Setting Persistent Connection to :%d",m_IsPersistentConnection);

  if (NULL != connectionValue)
  {
    QTV_Free(connectionValue);
  }

  return result;
}

void HTTPResponse::ExtractCookieHeadersValue()
{
  char *buf = this->m_Buffer;
  int valueLenReq = 0;
  int valueLen    = 0;
  int counter     = 0;

  while(GetNextCookieHeaderValue(&buf, NULL, 0, valueLenReq))
  {
    HTTPCookieElement *pElement = NULL;
    pElement = (HTTPCookieElement *)QTV_Malloc((sizeof(HTTPCookieElement) + valueLenReq + 1) * sizeof(char));
    if(pElement)
    {
      char *cookieValue = pElement->value;
      valueLen = valueLenReq;
      pElement->size = valueLen + 1;
      if(GetNextCookieHeaderValue(&buf, cookieValue, (int)pElement->size, valueLenReq))
      {
        cookieValue[valueLen] = '\0';
        ordered_StreamList_push(&m_CookieValueList,
                                &pElement->link,
                                counter++);
      }
      else
      {
        QTV_Free(pElement);
      }
    }
  }
}

bool
HTTPResponse::GetHeaderValue(const char *key, int keyLen,
                             char *value, int valueLen, int &valueLenReq)
{
  valueLenReq = -1;

  bool result = false;

  if (NULL == m_Buffer)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "HTTPResponse::GetHeaderValue - Response Buffer is NULL");
  }
  else if (keyLen < 0)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                  "HTTPResponse::GetHeaderValue - Invalid KeyLen '%d' ",
                  keyLen);
  }
  else
  {
    const char* line;
    const char* valueStart = NULL;

    for (line = std_strstr(m_Buffer, "\r\n"); // skip start line
        NULL != line;
        line = std_strstr(line, "\r\n"))
    {
      line += (int)CSTRLEN("\r\n");

      if (!std_memcmp(line, "\r\n", CSTRLEN("\r\n")))
      {
        break;
      }

      if (!std_strnicmp(line, key, keyLen) && ':' == line[keyLen])
      {
        valueStart = line + keyLen + 1;

        // remove LWS
        while (' ' == *valueStart || '\t' == *valueStart)
        {
          valueStart++;
        }

        char* valueEnd = std_strstr(valueStart, "\r\n");
        if (NULL == valueEnd && value)
        {
          valueEnd = value + std_strlen(value);
        }

        // remove trailing "LWS"
        if (valueEnd)
        {
          while (' ' == valueEnd[-1] || '\t' == valueEnd[-1])
          {
            valueEnd--;
          }

          valueLenReq = (int)(valueEnd - valueStart) + 1;
        }

        break;
      }
    }
    if (NULL == valueStart)
    {
      char safeKey[32];
      std_strlcpy(safeKey, key, STD_MIN(keyLen+1,STD_SIZEOF(safeKey)));

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MEDIUM,
                    "HTTPResponse::GetHeaderValue - '%s' not in response header",
                    safeKey);
    }
    else
    {
      // Populate valueLenReq and
      // value if value is not null.
      // extract 'value' for 'key'
      if (value)
      {
        std_strlcpy(value, valueStart, QTV_MIN(valueLen, valueLenReq));

        char safeKey[32];
        std_strlcpy(safeKey, key, STD_MIN(keyLen+1,STD_SIZEOF(safeKey)));

        QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_DEBUG,
                      "HTTPResponse::GetHeaderValue - %s: %s",
                      safeKey, value);
      }

      result = true;
    }

  }

  return result;
}

bool HTTPResponse::GetNextCookieHeaderValue(char **buf, char *value,
                                            int valueLen, int &valueLenReq)
{
  valueLenReq = -1;
  const char *key = "Set-Cookie";
  size_t keyLen = std_strlen(key);
  bool result = false;

  char *httpHeader = *buf;

  if (NULL == httpHeader)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "HTTPResponse::GetNextCookieHeaderValue - Response Buffer is NULL");
  }
  else
  {
    const char* line;
    const char* valueStart = NULL;
    char* valueEnd = NULL;

    for (line = std_strstr(httpHeader, "\r\n"); // skip start line
        NULL != line;
        line = std_strstr(line, "\r\n"))
    {
      line += std_strlen("\r\n");

      if (!std_memcmp(line, "\r\n", std_strlen("\r\n")))
      {
        break;
      }

      if (!std_strnicmp(line, key, keyLen) && ':' == line[keyLen])
      {
        valueStart = line + keyLen + 1;

        // remove LWS
        while (' ' == *valueStart || '\t' == *valueStart)
        {
          valueStart++;
        }

        valueEnd = std_strstr(valueStart, "\r\n");
        if (NULL == valueEnd && value)
        {
          valueEnd = value + std_strlen(value);
        }

        // remove trailing "LWS"
        if (valueEnd)
        {
          while (' ' == valueEnd[-1] || '\t' == valueEnd[-1])
          {
            valueEnd--;
          }

          valueLenReq = (int)((valueEnd - valueStart) + 1);
        }

        break;
      }
    }

    if(valueStart)
    {
      // Populate valueLenReq and
      // value if value is not null.
      // extract 'value' for 'key'
      if (value)
      {
        std_strlcpy(value, valueStart, QTV_MIN(valueLen, valueLenReq));

        char safeKey[32];
        std_strlcpy(safeKey, key, STD_MIN(keyLen+1,STD_SIZEOF(safeKey)));

        QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_DEBUG,
                      "HTTPResponse::GetNextCookieHeaderValue - %s: %s",
                      safeKey, value);

        *buf = (char *)valueEnd;
      }

      result = true;
    }
  }

  return result;
}

/**
 * @brief
 *  Get the entire HTTP Header string including the traling
 *  \r\n\r\n. Terminate the string with '\0'
 *
 * @param headerStr
 * @param headerStrBufSize
 * @param reqLen
 */
void
HTTPResponse::GetHeaders(char *headerStr, int headerStrBufSize, int& reqLen)
{
  char *posStartOFHeader = NULL;
  char *posEndOfHeader = NULL;
  reqLen = 0;

  if (m_Buffer)
  {
    posStartOFHeader = std_strstr(m_Buffer, "\r\n");

    if (posStartOFHeader)
    {
      posStartOFHeader += (int)CSTRLEN("\r\n");
    }

    posEndOfHeader = std_strstr(m_Buffer, "\r\n\r\n");

    if (posStartOFHeader && posEndOfHeader)
    {
      reqLen = (int)(posEndOfHeader - posStartOFHeader) + (int)CSTRLEN("\r\n\r\n") + 1;
    }
  }

  if (headerStr && posStartOFHeader && headerStrBufSize > 0)
  {
    // query to populate
    int bufSizeToUse = (headerStrBufSize >= reqLen ? reqLen : headerStrBufSize);
    std_strlcpy(headerStr, posStartOFHeader, bufSizeToUse);
  }
}

/**
 * @brief
 *  Get the entire reference to cookie headers value list
 */
void HTTPResponse::GetCookieHeaderValueList(ordered_StreamList_type **cookieList)
{
  if(cookieList != NULL)
  {
    (*cookieList) = &m_CookieValueList;
  }
}

void
HTTPResponse::SetInvalidContentLengthFlag(bool value)
{
  m_FlagInvalidContentLength = value;
}

/**
 * Get ptr to entity buffer resuming at the point past the last
 *   written byte.
 *
 * @param pBuf
 * @param remainingSize
 */
void HTTPResponse::GetEntityBodyBuffer(char *&pBuf, int& remainingSize)
{
  pBuf = m_EntityBody + m_NumBytesInEntityBody;
  remainingSize = MAX_ENTITY_BODY_SIZE - m_NumBytesInEntityBody;
}

/**
 * @brief
 *  Appended an amount of 'size' bytes to entity buffer.
 *
 * @param size
 */
void HTTPResponse::CommitEntityBodyBuffer(size_t size)
{
  m_NumBytesInEntityBody += (int)size;

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "CommitEntityBodyBuffer: Entity body size updated to %d",
                m_NumBytesInEntityBody);
}


/**
 * @brief This Method maps the transport result codes to
 *        response status code
 *
 * @param rslt
 *
 * @return HTTPResponseStatus
 */
HTTPResponseStatus HTTPResponse::MapTransportResultCode
(
  const TransportConnection::ResultCode rslt
)
{
  HTTPResponseStatus result = HTTP_RSP_FAILURE;

  switch(rslt)
  {
  case TransportConnection::SUCCESS:
    result = HTTP_RSP_SUCCESS;
  break;

  case TransportConnection::EWAITING:
    result = HTTP_RSP_WAIT;
  break;

  case TransportConnection::ENOMEMORY:
    result = HTTP_RSP_FAILURE;
  break;

  case TransportConnection::ECLOSEDBYPEER:
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,
                 "HTTPStack Connection closed by peer");
    result = HTTP_RSP_FAILURE;
  break;

  case TransportConnection::ESOCKERROR:
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "HTTPStack Socket error");
    result = HTTP_RSP_FAILURE;
  break;

  default:
    result = HTTP_RSP_FAILURE;
    break;
  }

  return result;
}

HTTPResponseStatus HTTPResponse::MapCommonResultCodeToRspCode
(
  const HTTPReturnCode rslt
)
{
  HTTPResponseStatus result = HTTP_RSP_FAILURE;
  switch(rslt)
  {
  case HTTP_SUCCESS:
    result = HTTP_RSP_SUCCESS;
    break;
  case HTTP_FAILURE:
    result = HTTP_RSP_FAILURE;
    break;
  case HTTP_WAIT:
    result = HTTP_RSP_WAIT;
    break;
  case HTTP_BADPARAM:
    result = HTTP_RSP_FAILURE;
    break;
  case HTTP_NOTSUPPORTED:
    result = HTTP_RSP_FAILURE;
    break;
  case HTTP_NOMOREDATA:
    result = HTTP_RSP_DONE;
    break;
  default:
    result = HTTP_RSP_FAILURE;
    break;
  }

  return result;
}

} // end namespace video
