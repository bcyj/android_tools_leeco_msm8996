/************************************************************************* */
/**
 * HTTPRequest.cpp
 * @brief implementation of the HTTPRequest.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Protocol/HTTP/main/latest/src/HTTPRequest.cpp#32 $
$DateTime: 2013/03/01 13:02:07 $
$Change: 3424415 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "assert.h"
// HTTPStack
#include "HTTPRequest.h"
#include "HTTPStackCommon.h"

// CommonUtils
#include "SourceMemDebug.h"
#include "qtv_msg.h"
#include "IPStreamSourceUtils.h"
#include <ctype.h>

namespace video
{



//URLs use some characters for special use in defining their syntax.
// These characters are reserved characters for URL.
const char HTTPRequest::reservedURLChars[]={':','/','?','#','[',']','@','!',
                                           '$','&','+',',','(',')',';','=',
                                           '*','\''};
/**
 * Default ctor
 */
_HTTPHeaderStruct::_HTTPHeaderStruct() :
  m_Key(NULL), m_Value(NULL)
{

}

/**
 * Destructor
 */
_HTTPHeaderStruct::~_HTTPHeaderStruct()
{
  if (m_Key)
  {
    QTV_Free(m_Key);
    m_Key = NULL;
  }

  if (m_Value)
  {
    QTV_Free(m_Value);
    m_Value = NULL;
  }
}

/**
 * Assumes key and value are null terminated
 */
bool
HTTPHeaderStruct::SetKeyAndValue(char *key, char *value)
{
  bool result = false;

  if (m_Key)
  {
    QTV_Free(m_Key);
    m_Key = NULL;
  }

  if (m_Value)
  {
    QTV_Free(m_Value);
    m_Value = NULL;
  }

  size_t lenKey = std_strlen(key);
  size_t  lenValue = std_strlen(value);

  m_Key = (char *)QTV_Malloc((lenKey + 1) * sizeof(char));

  if (m_Key)
  {
    m_Value = (char *)QTV_Malloc((lenValue + 1) * sizeof(char));
    if (NULL == m_Value)
    {
      QTV_Free(m_Key);
      m_Key = NULL;
    }
  }

  if (m_Key && m_Value)
  {
    std_strlcpy(m_Key, key, lenKey + 1);
    std_strlcpy(m_Value, value, lenValue + 1);
    result = true;
  }

  return result;
}

/**
 * Assumes value is null terminated
 */
bool
HTTPHeaderStruct::SetValue(char *value)
{
  bool result = false;

  QTV_Free(m_Value);
  m_Value = NULL;

  size_t lenValue = std_strlen(value);

  m_Value = (char *)QTV_Malloc((lenValue + 1) * sizeof(char));
  if (NULL == m_Value)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPHeaderStruct::SetValue - failed to allocated m_Value");
  }
  else
  {
    std_strlcpy(m_Value, value, lenValue + 1);
    result = true;
  }

  return result;
}

/**
 * Comparison function needed by queue.h,cpp routines to search for element
 * in queue.
 */
static int CompareHeaderKeys(void *itemPtr, void *compareVal)
{
  int result = 0;

  HTTPHeaderStruct *headerStruct = (HTTPHeaderStruct *)itemPtr;
  char *key = headerStruct->m_Key;

  if (itemPtr && compareVal && key)
  {
    result = (0 == std_strnicmp(key, (char *)compareVal, std_strlen(key))) ? 1 : 0;
  }

  return result;
}

/**
 * Default ctor
 */
HTTPRequest::HTTPRequest() :
  m_HttpMethod(HTTP_GET),
  m_Url(NULL),
  m_HostName(NULL),
  m_Port(0),
  m_RelativePath(0),
  m_HTTPCommandLine(NULL),
  m_LengthOfRequest(0),
  m_Buffer(NULL),
  m_NumBytesSent(0),
  m_NumRetries(0),
  m_NumRedirects(0),
  m_RequestState(REQUEST_INITIAL)
{
  //Initialize header list - queue maintains its own access lock
  StreamQ_init(&m_HeaderList);
}

/**
 * Destructor
 */
HTTPRequest::~HTTPRequest()
{
  FlushHeaders();

  if (m_HTTPCommandLine)
  {
    QTV_Free(m_HTTPCommandLine);
    m_HTTPCommandLine = NULL;
  }

  if (m_Buffer)
  {
    QTV_Free(m_Buffer);
    m_Buffer = NULL;
  }

  if (m_RelativePath)
  {
    QTV_Free(m_RelativePath);
    m_RelativePath = NULL;
  }

  if (m_HostName)
  {
    QTV_Free(m_HostName);
    m_HostName = NULL;
  }

  if (m_Url)
  {
    QTV_Free(m_Url);
    m_Url = NULL;
  }
}

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
bool
HTTPRequest::MarkReadyToSend()
{
  bool result = false;

  if(REQUEST_WAITING_TO_SEND == m_RequestState)
  {
    result = true;
  }
  else if ((REQUEST_INITIAL == m_RequestState) ||
      (REQUEST_ERROR == m_RequestState))
  {
    m_RequestState = REQUEST_WAITING_TO_SEND;
    result = true;
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                 "Failed to queue HTTP request");
  }

  return result;
}

/**
 * Composes m_HTTPCommandLine:
 * command + " " + httpUrl + " " HTTPStackCommon::HTTP_VERSION + "\r\n"
 */
bool
HTTPRequest::SetHTTPCommandLine(
  const char *command, size_t commandLen, const char *httpUrl, size_t httpUrlLen)
{
  bool result = false;

  if (commandLen > (size_t)HTTPStackCommon::MAX_COMMAND_LEN ||
      httpUrlLen > (size_t)HTTPStackCommon::MAX_URL_LEN)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::SetHTTPCommandLine - commandLen '%d' or httpUrlLen '%d' exceeds MAX_URL_LEN",
      commandLen, httpUrlLen);
  }
  else
  if (NULL == command || NULL == httpUrl)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::SetHTTPCommandLine - command or httpUrl is NULL");
  }
  else
  {
    // Make sure args are null terminated
    char *localCommand = NULL;
    char *localHTTPUrl = NULL;
    size_t commandLineLen = 0;

    localCommand = (char *)QTV_Malloc((commandLen + 1) * sizeof(char));
    localHTTPUrl = (char *)QTV_Malloc((httpUrlLen + 1) * sizeof(char));

    if (localCommand && localHTTPUrl)
    {
      std_strlcpy(localCommand, command, commandLen + 1);
      std_strlcpy(localHTTPUrl, httpUrl, httpUrlLen + 1);

      commandLineLen = std_strlen(localCommand) + std_strlen(" ") +
                         std_strlen(localHTTPUrl) + std_strlen(" ") +
                         std_strlen(HTTPStackCommon::HTTP_VERSION) + std_strlen("\r\n");
    }

    if (NULL != m_HTTPCommandLine)
    {
      QTV_Free(m_HTTPCommandLine);
      m_HTTPCommandLine = NULL;
    }

    m_HTTPCommandLine = (char *)QTV_Malloc(
              (commandLineLen + 1) * sizeof(char));
    if (NULL == m_HTTPCommandLine)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPRequest::SetHTTPCommandLine - Failed to allocated m_HTTPCommandLine");
    }
    else
    {
      m_HTTPCommandLine[0] = 0;
      std_strlcat(m_HTTPCommandLine, command, commandLineLen + 1);
      std_strlcat(m_HTTPCommandLine, " ", commandLineLen + 1);
      std_strlcat(m_HTTPCommandLine, httpUrl, commandLineLen + 1);
      std_strlcat(m_HTTPCommandLine, " ", commandLineLen + 1);
      std_strlcat(m_HTTPCommandLine, HTTPStackCommon::HTTP_VERSION, commandLineLen + 1);
      std_strlcat(m_HTTPCommandLine, "\r\n", commandLineLen + 1);

      result = true;
    }

    if (localCommand)
    {
      QTV_Free(localCommand);
    }

    if (localHTTPUrl)
    {
      QTV_Free(localHTTPUrl);
    }

  }

  return result;
}

/**
 * @brief: Clears headerList
 */
void
HTTPRequest::FlushHeaders()
{
  while (StreamQ_cnt(&m_HeaderList) > 0)
  {
    HTTPHeaderStruct *headerStruct =
      (HTTPHeaderStruct *) StreamQ_get(&m_HeaderList);

    if (headerStruct)
    {
      QTV_Delete(headerStruct);
    }
  }
}

/**
 * @brief:
 *  Add key value pair to headerList
 */
bool
HTTPRequest::SetHeader(const char *key, int keyLen, const char *value, int valueLen)
{
  bool result = false;

  int numHeaders = StreamQ_cnt(&m_HeaderList);

  if ((keyLen > HTTPStackCommon::MAX_HTTP_REQUEST_HEADER_KEY_LEN) ||
      (valueLen > HTTPStackCommon::MAX_HTTP_REQUEST_VALUE_LEN) ||
      (keyLen < 0) || (valueLen < 0) ||
      (numHeaders >= HTTPStackCommon::MAX_HTTP_REQ_HDR_ENTRIES))
  {
    QTV_MSG_PRIO4( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::SetHeader - keyLen '%d' or valueLen '%d' invalid "
      "or numHeaderEntriens '%d' max allowed '%d'",
      keyLen, valueLen, numHeaders, HTTPStackCommon::MAX_HTTP_REQ_HDR_ENTRIES);
  }
  else
  if ((NULL == key) || (NULL == value))
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::SetHeader - key or value is NULL");
  }
  else
  {
    char *localKey = NULL;
    char *localValue = NULL;
    localKey = (char *)QTV_Malloc((keyLen + 1) * sizeof(char));
    localValue = (char *)QTV_Malloc((valueLen + 1) * sizeof(char));

    if (localKey && localValue)
    {
      result = true;
      std_strlcpy(localKey, key, keyLen + 1);
      std_strlcpy(localValue, value, valueLen + 1);

      if (std_strnicmp(localKey, HTTPStackCommon::RANGE_KEY,
                       std_strlen(HTTPStackCommon::RANGE_KEY)) == 0)
      {
        //Since multipart/byteranges response is not supported, better to
        //fail the multiple byte- "Range" header so that client can try
        //splitting it up!
        if (std_strchr(localValue, ','))
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                        "HTTPRequest::SetHeader - Range header with "
                        "multiple byte-range specifiers not supported" );
          result = false;
        }
      }

      if (result)
      {
        HTTPHeaderStruct *headerStruct =
          (HTTPHeaderStruct *)StreamQ_linear_search(&m_HeaderList,
                                                    CompareHeaderKeys,
                                                    (void *)localKey);

        if (headerStruct)
        {
          headerStruct->SetValue(localValue);
        }
        else
        {
          headerStruct = QTV_New(HTTPHeaderStruct);

          if (NULL == headerStruct)
          {
            result = false;
            QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR, "NO MEM");
          }
          else
          {
            result = headerStruct->SetKeyAndValue(localKey, localValue);
            if (result)
            {
              StreamQ_link(headerStruct, &(headerStruct->link));
              StreamQ_put(&m_HeaderList, &(headerStruct->link));
            }
            else
            {
              QTV_Delete(headerStruct);
              headerStruct = NULL;
            }
          }
        } // end if (headerStruct...)
      }//if (result)
    } //end if ((localKey && localValue)
    if (localKey)
    {
      QTV_Free(localKey);
    }
    if(localValue)
    {
      QTV_Free(localValue);
    }
  }
  return result;
}

/**
 * @return:
 *  true   if value for key exists
 *  false  otherwise
 */
bool
HTTPRequest::HeaderExistsForKey(const char *key, int keyLen)
{
  bool result = false;

  if ((keyLen > HTTPStackCommon::MAX_HTTP_REQUEST_HEADER_KEY_LEN) ||
          (keyLen < 0))
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::HeaderExistsForKey - keyLen '%d' invalid ",
      keyLen);
  }
  else
  if (NULL == key)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::HeaderExistsForKey - key is NULL");
  }
  else
  {
    char *localKey = NULL;
    localKey = (char *)QTV_Malloc((keyLen + 1) * sizeof(char));

    if (localKey)
    {
      std_strlcpy(localKey, key, keyLen + 1);
    }

    HTTPHeaderStruct *headerStruct =
       (HTTPHeaderStruct *)StreamQ_linear_search(&m_HeaderList,
                                           CompareHeaderKeys,
                                           (void *)localKey);

    if (headerStruct)
    {
      result = true;
    }

    if (localKey)
    {
      QTV_Free(localKey);
      localKey = NULL;
    }
  }

  return result;
}

/**
 * Removes a header key-value pair from HeaderList
 */
bool
HTTPRequest::RemoveHeader(const char *key, int keyLen)
{
  bool result = false;

  if ((keyLen < 0) || (keyLen > HTTPStackCommon::MAX_HTTP_REQUEST_HEADER_KEY_LEN))
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::RemoveHeader: keyLen '%d' is invalid",
      keyLen);
  }
  else
  if (NULL == key)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::RemoveHeader: key is NULL");
  }
  else
  {
    char *localKey = NULL;
    localKey = (char *)QTV_Malloc((keyLen + 1) * sizeof(char));

    if (NULL == localKey)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPRequest::RemoveHeader: Failed to allocate localKey");
    }
    else
    {
      std_strlcpy(localKey, key, keyLen + 1);

      HTTPHeaderStruct *headerStruct =
        (HTTPHeaderStruct *)StreamQ_linear_search(&m_HeaderList,
                                       CompareHeaderKeys,
                                       (void *)localKey);

      if (headerStruct)
      {
        StreamQ_delete(&(headerStruct->link));
        QTV_Delete(headerStruct);
      }

    }

    if (localKey)
    {
      QTV_Free(localKey);
      localKey = NULL;
    }
  }

  return result;
}

/**
 * @brief
 *  Get the header value for specified header key.
 *
 * @return:
 *  true   header successfully obtained
 *  false  otherwise
 */
bool
HTTPRequest::GetHeader(const char *key, int keyLen, char *value, int& valLen)
{
  bool result = false;

  if ((NULL == key) ||
      (keyLen > HTTPStackCommon::MAX_HTTP_REQUEST_HEADER_KEY_LEN) || (keyLen < 0))
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "HTTPRequest::HeaderExistsForKey - Invalid key - keyLen '%d'", keyLen );
  }
  else
  {
    HTTPHeaderStruct *headerStruct =
      (HTTPHeaderStruct *)StreamQ_linear_search(&m_HeaderList, CompareHeaderKeys, (void *)key);

    if (headerStruct)
    {
      if (value)
      {
        result = true;
        (void)std_strlcpy(value, headerStruct->m_Value,
                          STD_MIN(valLen, (int)std_strlen(headerStruct->m_Value) + 1));
      }
      valLen = (int)std_strlen(headerStruct->m_Value) + 1;
    }
  }

  return result;
}

/**
 * @brief
 *  Marks request as ready to send.
 *
 * @return HTTPReturnCode
 *  HTTP_SUCCESS Request successfully marked ready to send or
 *    already in the middle of being sent
 *  HTTP_FAILURE Request is in non recoverable error state.
 */
HTTPReturnCode
HTTPRequest::SendRequest(TransportConnection *httpConnection)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if (REQUEST_SENT == m_RequestState)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                 "HTTPRequest::SendRequest() Nothing to do");

    result = HTTP_SUCCESS;
  }
  else
  {
    if (REQUEST_ERROR != m_RequestState)
    {
      if (REQUEST_WAITING_TO_SEND == m_RequestState)
      {
        result = (true == PrepareRequestBuffer() ? HTTP_SUCCESS : HTTP_FAILURE);

        if (HTTP_SUCCESS == result)
        {
          m_RequestState = REQUEST_SENDING;
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                       "HTTPRequest::SendRequesT() Failed to Prepare request buffer");

          m_RequestState = REQUEST_ERROR;
        }
      }

      if (REQUEST_SENDING == m_RequestState)
      {
        TransportConnection::ResultCode rslt = SendRequestOnNetwork(httpConnection);

        if (TransportConnection::SUCCESS == rslt)
        {
          m_RequestState = REQUEST_SENT;
          result = HTTP_SUCCESS;
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,"HTTPRequest::SendRequest() failed :%d",
                        result);
          if (TransportConnection::EWAITING == rslt)
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                         "HTTPRequest::SendRequest() Not fully sent");

            result = HTTP_WAIT;
          }
          else
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                         "Failed to send HTTP Request. "
                         "Moving http request to error state");

            m_RequestState = REQUEST_ERROR;

            // If error is networking related, move to state CLOSED and give it
            // a cahnce to recover, eg if 'RETRY' is activated.
            if ((TransportConnection::ECLOSEDBYPEER == rslt) ||
                (TransportConnection::ESOCKERROR == rslt))
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                           "HTTPRequest::SendRequest() Socket error");
            }

            result = HTTP_FAILURE;
          }
        }
      }
    }
  }

  return result;
}

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
 * @return HTTPReturnCode
 */
HTTPReturnCode
HTTPRequest::ParseHostPortPathFromUrlInternal(
  const char *url)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if (NULL == url)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPStateBase::ParseHostPortPathFromUrlInternal: url is NULL");
  }
  else
  {
    size_t urlLen = std_strlen(url);

    // Extract host, port, relative path into state variables
    if (m_HostName)
    {
      QTV_Free(m_HostName);
      m_HostName = NULL;
    }

    if (m_RelativePath)
    {
      QTV_Free(m_RelativePath);
      m_RelativePath = NULL;
    }

    size_t hostNameBufLen = 0, relativePathBufLen =0;

    result = HTTPStackCommon::GetHostPortRelativePathFromUrl(
                                url,
                                urlLen,
                                NULL, 0, hostNameBufLen,
                                m_Port,
                                NULL, 0, relativePathBufLen);

    // Allocate memory for hostName and relative path.
    if (result == HTTP_SUCCESS)
    {
      m_HostName = (char *)QTV_Malloc(hostNameBufLen * sizeof(char));

      if (NULL == m_HostName)
      {
        result = HTTP_FAILURE;

        QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
          "HTTPStateIdle::SendRequest Failed to allocate m_HTTPStateInfo.m_HostName");
      }

      if (HTTP_SUCCESS == result)
      {
        m_RelativePath = (char *)QTV_Malloc(relativePathBufLen * sizeof(char));

        if (NULL == m_RelativePath)
        {
          result = HTTP_FAILURE;

          QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
            "HTTPStateIdle::SendRequest Failed to allocate m_HTTPStateInfo.m_RelativePath");

          QTV_Free(m_HostName);
          m_HostName = NULL;
        }
      }
    }

    if ((HTTP_SUCCESS == result) && (NULL != m_HostName) &&
        (NULL != m_RelativePath))
    {
      size_t temp = 0;
      result = HTTPStackCommon::GetHostPortRelativePathFromUrl(
                                  url,
                                  urlLen,
                                  m_HostName, hostNameBufLen, temp,
                                  m_Port,
                                  m_RelativePath, relativePathBufLen, temp);
    }
  }

  return result;
}

/**
 * @brief
 *  Compose request buffer from cmd line, hdrs
 *  (and msg body ### TO DO ### )
 *
 * @return bool
 *  true
 *  false
 */
bool
HTTPRequest::PrepareRequestBuffer()
{
  bool result = false;

  // Compute the size of buffer to hold the request.
  if (NULL == m_HTTPCommandLine)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
      "HTTPRequest::PrepareRequest(): m_HTTPCommandLine is NULL");
  }
  else
  {
    m_LengthOfRequest = std_strlen(m_HTTPCommandLine);
    HTTPHeaderStruct *headerStruct = (HTTPHeaderStruct *)StreamQ_check(&m_HeaderList);

    while (headerStruct)
    {
      size_t keyLen = std_strlen(headerStruct->m_Key);
      size_t valueLen = std_strlen(headerStruct->m_Value);
      m_LengthOfRequest += keyLen + std_strlen(": ") + valueLen + std_strlen("\r\n");

      headerStruct = (HTTPHeaderStruct *)StreamQ_next(&m_HeaderList, &(headerStruct->link));
    }

    m_LengthOfRequest += std_strlen("\r\n");

    // accomdate null char as well though we wont send to socket
    size_t requiredBufSize = m_LengthOfRequest + 1;

    if(m_Buffer)
    {
      QTV_Free(m_Buffer);
      m_Buffer = NULL;
    }

    m_Buffer = (char *)QTV_Malloc((requiredBufSize) * sizeof(char));
    if (NULL == m_Buffer)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "HTTPRequest::PrepareRequestBuffeR() failed to alloc m_Buffer");
    }
    else
    {
      // Populate the request
      size_t bufSizeFilled = 0;

      m_Buffer[0] = '\0';
      if((requiredBufSize - bufSizeFilled) > std_strlen(m_HTTPCommandLine))
      {
        bufSizeFilled = std_strlcat(m_Buffer, m_HTTPCommandLine, requiredBufSize);
        result = true;
      }
      headerStruct = (HTTPHeaderStruct *)StreamQ_check(&m_HeaderList);

      while (headerStruct)
      {
        if((result) && ((requiredBufSize - bufSizeFilled) > std_strlen(headerStruct->m_Key)))
        {
          bufSizeFilled = std_strlcat(m_Buffer, headerStruct->m_Key, requiredBufSize);
        }
        else
        {
          result = false;
          break;
      }

        if((requiredBufSize - bufSizeFilled) > std_strlen(": "))
        {
          bufSizeFilled = std_strlcat(m_Buffer, ": ", requiredBufSize);
        }
        else
        {
          result = false;
          break;
        }

        if((requiredBufSize - bufSizeFilled) > std_strlen(headerStruct->m_Value))
        {
          bufSizeFilled = std_strlcat(m_Buffer, headerStruct->m_Value, requiredBufSize);
        }
        else
        {
          result = false;
          break;
        }

        if((requiredBufSize - bufSizeFilled) > std_strlen("\r\n"))
        {
          bufSizeFilled = std_strlcat(m_Buffer, "\r\n", requiredBufSize);
        }
        else
        {
          result = false;
          break;
        }

        headerStruct = (HTTPHeaderStruct *)StreamQ_next(&m_HeaderList, &(headerStruct->link));
      }

      if((result) && ((requiredBufSize - bufSizeFilled) > std_strlen("\r\n")))
      {
        bufSizeFilled = std_strlcat(m_Buffer, "\r\n", requiredBufSize);
      }
    }

  }

  return result;
}

/**
 * @brief
 *  Sends as much pending data of the http request to the
 *  network
 *
 * @param TransportConnection
 *
 * @return AEEResult
 *  AEE_SUCCESS
 *  AEE_EWOULDBLOCK
 *  AEE_EFAILED
 */
TransportConnection::ResultCode
HTTPRequest::SendRequestOnNetwork(TransportConnection *httpConnection)
{
  QTV_NULL_PTR_CHECK(httpConnection, TransportConnection::EFAILED);

  TransportConnection::ResultCode rslt = TransportConnection::SUCCESS;
  ASSERT(m_LengthOfRequest > 0);

  if (m_NumBytesSent < m_LengthOfRequest)
  {
    rslt = TransportConnection::EFAILED;

    size_t numSent = 0;
    rslt = httpConnection->Send(
      m_Buffer + m_NumBytesSent, m_LengthOfRequest - m_NumBytesSent, numSent);

    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_HIGH,"HTTPRequest::SendRequest() status:%d",rslt);

    if (numSent > 0)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                    "SendRequestOnNetwork(): '%d' bytes sent",
                    numSent);

      m_NumBytesSent += numSent;

      if (TransportConnection::SUCCESS == rslt)
      {
        if (m_NumBytesSent == m_LengthOfRequest)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                        "SendRequestOnNetwork(): Entire request sent( Len '%d')",
                        m_LengthOfRequest);
          rslt = TransportConnection::SUCCESS;
        }
        else // request not fully sent yet
        {
          if (m_NumBytesSent < m_LengthOfRequest)
          {
            rslt = TransportConnection::EWAITING;
          }
          else
          {
            // sanity error check
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                          "SendRequestOnNetwork(): Error numBytesSent '%d' > "
                          "Length of request '%d'", m_NumBytesSent, m_LengthOfRequest);
            rslt = TransportConnection::EFAILED;
          }
        }
      } // end if (TransportConnection::SUCCESS == rslt)

    } // end if (numSent > 0)
  }

  return rslt;
}

void
HTTPRequest::IncNumRetries()
{
  ++m_NumRetries;
}

int
HTTPRequest::GetNumRetries() const
{
  return m_NumRetries;
}

void
HTTPRequest::ResetNumRetries()
{
  m_NumRetries = 0;
}

void
HTTPRequest::IncNumRedirects()
{
  ++m_NumRedirects;
}

void
HTTPRequest::ResetNumRedirects()
{
  m_NumRedirects = 0;
}

int
HTTPRequest::GetNumRedirects() const
{
  return m_NumRedirects;
}

/**
 * Checks if the all bytes and request have been
 * written to tcp socket
 */
bool
HTTPRequest::IsRequestCompletelySent() const
{
  return  (REQUEST_SENT == m_RequestState ||
           REQUEST_RSP_HDRS_RECD == m_RequestState ||
           REQUEST_RSP_FULLY_RECD == m_RequestState
           ? true : false);
}

/**
 * Checks if the partial or complete request written to tcp
 * socket
 */
bool
HTTPRequest::IsRequestPartiallyOrFullySent() const
{
  return  (REQUEST_SENDING == m_RequestState ||
           REQUEST_SENT == m_RequestState ||
           REQUEST_RSP_HDRS_RECD == m_RequestState ||
           REQUEST_RSP_FULLY_RECD == m_RequestState
           ? true : false);
}

bool
HTTPRequest::IsProcessingARequest() const
{
  return ((REQUEST_INITIAL == m_RequestState) ||
          (REQUEST_RSP_FULLY_RECD == m_RequestState) ||
          (REQUEST_ERROR == m_RequestState)
          ? false
          : true);
}

bool
HTTPRequest::IsRspFullyRecd() const
{
  return (REQUEST_RSP_FULLY_RECD ==m_RequestState)
          ? true:false;
}

void
HTTPRequest::SetRspHeadersRecd()
{
  m_RequestState = REQUEST_RSP_HDRS_RECD;
}

void
HTTPRequest::SetRspFullyRecd()
{
  m_RequestState = REQUEST_RSP_FULLY_RECD;
}

void
HTTPRequest::SetRspError()
{
  m_RequestState = REQUEST_ERROR;
}

bool
HTTPRequest::IsRspHeadersRecd() const
{
  return (REQUEST_RSP_HDRS_RECD == m_RequestState ||
          REQUEST_RSP_FULLY_RECD == m_RequestState
          ? true
          : false);
}

/**
 * Reset member variables excpet the header list. The headers
 * should be cleared via FlushHeaders
 */
void
HTTPRequest::Reset()
{
  m_RequestState = REQUEST_INITIAL;

  if (m_Url)
  {
    QTV_Free(m_Url);
    m_Url = NULL;
  }

  if (m_HostName)
  {
    QTV_Free(m_HostName);
    m_HostName = NULL;
  }

  m_Port = 0;

  if (m_RelativePath)
  {
    QTV_Free(m_RelativePath);
    m_RelativePath = NULL;
  }

  if (m_HTTPCommandLine)
  {
    QTV_Free(m_HTTPCommandLine);
    m_HTTPCommandLine = NULL;
  }

  m_LengthOfRequest = 0;

  if (m_Buffer)
  {
    QTV_Free(m_Buffer);
    m_Buffer = NULL;
  }

  m_NumBytesSent = 0;
}

void
HTTPRequest::SetRequestMethod(HTTPMethodType method)
{
  m_HttpMethod = method;
}

HTTPMethodType
HTTPRequest::GetRequestMethod() const
{
  return m_HttpMethod;
}

/* @brief: Checks whether given character is a from reserved character set of url
 * param[in] - character c to be checked for reserved character
 * Returns true if char is reserved
 * else returns false
 */
bool HTTPRequest::IsReservedCharacter(char c)
{
  bool ret = false;
  for(uint16 i=0;i< ARR_SIZE(reservedURLChars);i++)
  {
    if(reservedURLChars[i]==c)
    {
      ret = true;
      break;
    }
  }
  return ret;

}

/* @brief: Checks whether percent encoding is required for the given character
 * param[in] - character c to be checked for percent encoding
 * Returns true if percent encoding required
 * else returns false
 */
bool HTTPRequest::IsPercentEncodingRequired(char c)
{
  bool ret=false;
  // Percent encoding is not required for alphabets, digits, some
  // unreserved characters such as -,_,.,~
  // Currenly not encoding reserved characters
  // TODO: Reserved characters should be percent encoded if they
  // are used for some purpose other than what they are reserved for.
  if(!(isalnum(c) || IsReservedCharacter(c) || c=='-' ||
       c=='_' || c=='.' || c=='~'))
  {
    ret=true;
  }
  return ret;
}
/* @brief: Encodes the url
 * param[in] - url-url to be encoded
 * param[out] - encodedurl - url after encode
 * param[in/out] - size of encoded url
 * Returns HTTP_BADPARAM if size of encoded url is not sufficient
 * HTTP_SUCCESS - on successful encode
 * HTTP_FAILURE - otherwise
 */
HTTPReturnCode HTTPRequest::EncodeURL(const char *url,char *encodedurl,int& encodeurlLen)
{
  HTTPReturnCode result = HTTP_FAILURE;
  int requiredLen = 0;
  char *temp=(char*)url;
  while(*temp)
  {
    if(IsPercentEncodingRequired(*temp))
    {
      requiredLen+=3;
    }
    else
    {
      requiredLen++;
    }
    temp++;
  }
  temp=(char*)url;
  if(encodeurlLen < requiredLen)
  {
    encodeurlLen = requiredLen;
    result = HTTP_BADPARAM;
  }
  else if (encodedurl)
  {
    int i=0;
    while(*temp)
    {
      if(IsPercentEncodingRequired(*temp))
      {
        encodedurl[i]='%';
        int ascval=*temp;
        std_strlprintf(encodedurl+i+1,encodeurlLen-i-1,"%.2x",ascval);
        i+=3;
      }
      else
      {
        encodedurl[i++]=*temp;
      }
      temp++;
    }
    result = HTTP_SUCCESS;
  }
  return result;
}

HTTPReturnCode
HTTPRequest::SetRequestUrl(const char* url, size_t /*urlLen*/)
{
  HTTPReturnCode result = HTTP_FAILURE;
  char *fragment_id_ptr = 0;

  // Copy url into m_HTTPUrl
  if (m_Url)
  {
    QTV_Free(m_Url);
    m_Url = NULL;
  }

  /* Remove fragment identifier from the URL
     Fragment identifer appears in the last part of the URL,
     starts with # till the end of the url search for the fragment
     id(#) in the URL and strip it */
  fragment_id_ptr = std_strchr((char *)url,'#');
  if(0 != fragment_id_ptr)
  {
    *fragment_id_ptr = '\0';
  }

  int encodedURLlen=0;
  result = EncodeURL(url,m_Url,encodedURLlen);
  if(result == HTTP_BADPARAM)
  {
    m_Url = (char *)QTV_Malloc((encodedURLlen + 1) * sizeof(char));

    if (NULL == m_Url)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                    "HTTPStackCommon::GetHostAndPortFromUrl - Failed to allocate stateInfo.m_HTTPUrl");
      result = HTTP_FAILURE;
    }
    else
    {
      result = EncodeURL(url,m_Url,encodedURLlen);
      m_Url[encodedURLlen]='\0';

    }
  }
  return result;
}

const char*
HTTPRequest::GetRequestUrl() const
{
  return m_Url;
}

HTTPReturnCode
HTTPRequest::SetHostName(const char *hostName)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if (hostName)
  {
    if (m_HostName)
    {
      QTV_Free(m_HostName);
      m_HostName = NULL;
    }

    size_t hostNameBufSize = std_strlen(hostName) + 1;

    m_HostName = (char *)QTV_Malloc(
      hostNameBufSize * sizeof(char));

    if (NULL == m_HostName)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "HTTPStackHelper::SetHostName() Failed to allocate hostName");
    }
    else
    {
      std_strlcpy(m_HostName, hostName, hostNameBufSize);
      result = HTTP_SUCCESS;
    }
  }

  return result;
}

const char*
HTTPRequest::GetHostName() const
{
  return m_HostName;
}

void
HTTPRequest::SetPort(unsigned short port)
{
  m_Port = port;
}

unsigned short
HTTPRequest::GetPort() const
{
  return m_Port;
}

HTTPReturnCode
HTTPRequest::SetRelativePath(const char *relativePath)
{
  HTTPReturnCode result = HTTP_FAILURE;

  if (relativePath)
  {
    if (m_RelativePath)
    {
      QTV_Free(m_RelativePath);
      m_RelativePath = NULL;
    }

    int encodedPathlen=0;
    result = EncodeURL(relativePath,m_RelativePath,encodedPathlen);
    if(result == HTTP_BADPARAM)
    {
      m_RelativePath = (char *)QTV_Malloc((encodedPathlen + 1) * sizeof(char));

      if (NULL == m_RelativePath)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "HTTPStackHelper::SetRelativePath() Failed to allocate relative path");
          result = HTTP_FAILURE;
      }
      else
      {
        result = EncodeURL(relativePath,m_RelativePath,encodedPathlen);
        m_RelativePath[encodedPathlen]='\0';

      }
    }
  }
  return result;
}

const char*
HTTPRequest::GetRelativePath() const
{
  return m_RelativePath;
}

void
HTTPRequest::Log()
{
  // Get head of queue
  HTTPHeaderStruct *headerStruct =
    (HTTPHeaderStruct *)StreamQ_check(&m_HeaderList);

  while (headerStruct)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED, "Header :'%s', Value '%s'",
      (char *)(headerStruct->m_Key), (char *)(headerStruct->m_Value));

    headerStruct = (HTTPHeaderStruct *)StreamQ_next(&m_HeaderList, &(headerStruct->link));
  }
}

void HTTPRequest::MarkSentRequestAsReSend()
{
 if(m_RequestState == REQUEST_WAITING_TO_SEND ||
    m_RequestState == REQUEST_SENDING ||
    m_RequestState == REQUEST_SENT ||
    m_RequestState == REQUEST_ERROR )
    {
     m_RequestState = REQUEST_WAITING_TO_SEND;
     m_NumBytesSent = 0;
    }
}

} // end namespace video
