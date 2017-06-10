
/************************************************************************* */
/**
 * @file URL.cpp
 * @brief Implementation of URL class.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/Url.cpp#12 $
$DateTime: 2013/08/02 06:28:55 $
$Change: 4209946 $

========================================================================== */
#include "Url.h"
#include "SourceMemDebug.h"
#include "qtv_msg.h"
#include "oscl_types.h"
#include "oscl_string_utils.h"
#include "dsbsd.h"
#include "IPStreamSourceUtils.h"
#include "string.h"

//definitions for max url length variables
uint32 URL::MaxUrlSize = QTV_MAX_URN_BYTES;

// MaxUrlSizeWithCtrlPath is MaxUrlSize (currently QTV_MAX_URN_BYTES)
//plus "/streamid = *(digit)"
uint32 URL::MaxUrlSizeWithCtrlPath =
                URL::MaxUrlSize + QTV_CONTROL_RELATIVE_URL_LEN;

/**
 * @ brief URL constor
 */
URL::URL()
{
  Init();
}

/**
 * @brief Destructor
 */
URL::~URL()
{
  if (NULL != m_url)
  {
    QTV_Free(m_url);
    m_url = NULL;
  }
}

/**
 * @brief Creates a URL object based on an existing URL object
 */
URL::URL(const URL &otherUrl)
{
  Init();

  size_t len = otherUrl.m_urlLen;
  if (len > MaxUrlSizeWithCtrlPath)
  {
    m_errCode = URL_MAX_LEN_EXCEEDS;
  }
  else if (len > 0)
  {
    m_url = (char *)QTV_Malloc((otherUrl.m_urlLen + 1) * sizeof(char));
    if (NULL == m_url)
    {
      m_errCode = URL_OUT_OF_MEMORY;
    }
    else
    {
      std_strlcpy(m_url, otherUrl.m_url, otherUrl.m_urlLen + 1);
      m_urlLen = otherUrl.m_urlLen;
      m_capacity = otherUrl.m_urlLen + 1;
    }
  }
}

/**
 * @brief Creates a URL object based on an existing string
 */
URL::URL(const char* urlStr)
{
  Init();

  if (urlStr)
  {
    uint32 len = (uint32)std_strlen(urlStr);
    if (len > MaxUrlSizeWithCtrlPath)
    {
      m_errCode = URL_MAX_LEN_EXCEEDS;
    }
    else if (len > 0)
    {
      m_url = (char *)QTV_Malloc((len + 1) * sizeof(char));
      if (NULL == m_url)
      {
        m_errCode = URL_OUT_OF_MEMORY;
      }
      else
      {
        std_strlcpy(m_url, urlStr, len + 1);
        m_urlLen = len;
        m_capacity = len + 1;
      }
    }
  }
}

/**
 * @brief Creates an empty URL object based on the length
 */
URL::URL(uint32 len)
{
  Init();

  if (len > MaxUrlSizeWithCtrlPath)
  {
    m_errCode = URL_MAX_LEN_EXCEEDS;
  }
  else if (len > 0)
  {
    m_url = (char *)QTV_Malloc((len + 1) * sizeof(char));
    if (NULL == m_url)
    {
      m_errCode = URL_OUT_OF_MEMORY;
    }
    else
    {
      memset(m_url, 0x00, len);
      m_capacity = len + 1;
    }
  }
}

/**
 * @brief initializes url object
 *
 */
void URL::Init()
{
  m_url = NULL;
  m_capacity = 0;
  m_urlLen = 0;
  m_errCode = URL_OK;
}

/**
 * @brief Assigns a new value to the URL object
 *
 * @return URL&
 */
URL & URL::operator =(const URL & otherUrl)
{
  m_errCode = URL_OK;

  //no buffer, delete our source url buffer as well
  if (otherUrl.IsNull())
  {
    Clear();
    return *this;
  }

  //url object empty, reset source url object
  if (otherUrl.IsEmpty())
  {
    Reset();
    return *this;
  }

  if (m_capacity <= otherUrl.m_urlLen)
  {
    if (NULL != m_url)
    {
      QTV_Free(m_url);
    }
    m_url = (char *)QTV_Malloc((otherUrl.m_urlLen + 1) * sizeof(char));

    //we resized allocated memory, so update length
    m_capacity = otherUrl.m_urlLen + 1;
  }

  if (NULL == m_url)
  {
    Clear();
    m_errCode = URL_OUT_OF_MEMORY;
  }
  else
  {
    std_strlcpy(m_url, otherUrl.m_url, otherUrl.m_urlLen + 1);
    m_urlLen = otherUrl.m_urlLen;
  }

  return *this;
}

/**
 * @brief Assigns a new value to the URL object
 *
 * @return URL&
 */
URL & URL::operator =(const char *urlStr)
{
  m_errCode = URL_OK;

  if (NULL == urlStr)
  {
    Clear();
  }
  else
  {
    uint32 urlSize = (uint32)std_strlen(urlStr);

    if (urlSize > MaxUrlSizeWithCtrlPath)
    {
      Reset();
      m_errCode = URL_MAX_LEN_EXCEEDS;
      return *this;
    }

    if (m_urlLen < urlSize)
    {
      if (NULL != m_url)
      {
        QTV_Free(m_url);
      }
      m_url = (char *)QTV_Malloc((urlSize + 1) * sizeof(char));
      m_capacity = urlSize + 1;
    }
    if (NULL == m_url)
    {
      Clear();
      m_errCode = URL_OUT_OF_MEMORY;
    }
    else
    {
      std_strlcpy(m_url, urlStr, urlSize + 1);
      m_urlLen = urlSize;
    }
  } //if (NULL == urlStr)
  return *this;
}

/**
 * @brief Checks whether the object and otherUrl are identical.
 *
 * @return bool
 */
bool URL::operator == (const URL & otherUrl) const
{
  if(!IsNull() && !otherUrl.IsNull() &&
     (m_urlLen == otherUrl.m_urlLen) &&
     (std_strnicmp(m_url, otherUrl.m_url, m_urlLen) == 0))
  {
    return true;
  }
  return false;
}

/**
 * @brief Checks whether the object and urlstr are identical.
 *
 * @return bool
 */
bool URL::operator == (const char * urlStr) const
{
  if(!IsNull() && (NULL != urlStr) &&
      (m_urlLen == (uint32)std_strlen(urlStr)) &&
      (std_strnicmp(m_url, urlStr, m_urlLen) == 0))
  {
    return true;
  }
  return false;
}

/**
 * @brief Checks whether the object and otherUrl are not identical.
 *
 * @return bool
 */
bool URL::operator != (const URL & otherUrl) const
{
    if(!(*this == otherUrl))
  {
    return true;
  }
  return false;
}

/**
 * @brief Checks whether the object and urlstr are not identical.
 *
 * @return bool
 */
bool URL::operator != (const char * urlStr) const
{
  if(!(*this == urlStr))
  {
    return true;
  }
  return false;
}

/**
 * @brief provides “const char*” access to URL object
 */
URL::operator const char*() const
{
  return m_url;
}

/**
 * @brief reset url object, don't delete buffer
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::Reset()
{
  ResetErrorCode();

  if (NULL != m_url)
  {
    memset(m_url, 0x00, m_capacity);
  }

  m_urlLen = 0;

  return m_errCode;
}

/**
 * @brief Checks if URL is empty
 *
 * @return bool
 */
bool URL::IsEmpty() const
{
  if ((NULL == m_url) || (m_capacity == 0) || (m_urlLen == 0) || !std_strlen(m_url))
  {
    return true;
  }
  return false;
}

/**
 * @brief Finds out if url string is null
 *
 * @return bool
 */
bool URL::IsNull() const
{
  if (NULL == m_url)
  {
    return true;
  }
  return false;
}

/**
 * @brief make url buffer empty, and deletes buffer
 *
 * if you don't want to delete buffer, use Reset()
 */
void URL::Clear()
{
  m_errCode = URL_OK;

  if (NULL != m_url)
  {
    QTV_Free(m_url);
    m_url = NULL;
  }
  m_urlLen = 0;
  m_capacity = 0;
}
#if 0
/**
 * @brief Gets the length of the url
 *
 * @return uint32
 */
uint32 URL::GetCapacity() const
{
  return m_capacity;
}
#endif

/**
 * @brief returns the length of URL
 *
 * @return uint32
 */
size_t URL::GetUrlLength() const
{
  return m_urlLen;
}

/**
 * @brief Gets the url pointer, useful for manipulations directly on string
 * (c string like modifications)
 *
 * @return char*
 */
char *URL::GetUrlBuffer()
{
  return m_url;
}

/**
 * @brief returns the URL
 *
 * @return const char*
 */
const char *URL::GetUrlBuffer() const
{
  return (const char *) m_url;
}
#if 0

/**
 * @brief Sets the URL
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::SetUrl(const char* urlStr, int32 copylen)
{
  uint32 len;
  m_errCode = URL_OK;

  if (NULL == urlStr)
  {
    Clear();
    return m_errCode;
  }

  if (copylen > -1)
  {
    len = copylen;
  }
  else
  {
    len = (uint32)std_strlen(urlStr);
  }

  //check whether exceeds maximum length limitation
  if (len > MaxUrlSizeWithCtrlPath)
  {
    Reset();
    m_errCode = URL_MAX_LEN_EXCEEDS;
    return m_errCode;
  }

  if (m_capacity <= len)
  {
    if (NULL != m_url)
    {
      QTV_Free(m_url);
    }

    m_url = (char *)QTV_Malloc((len + 1) * sizeof(char));

    //num alloc bytes changed, so update length
    m_capacity = len + 1;
  }

  if (NULL == m_url)
  {
    m_errCode = URL_OUT_OF_MEMORY;
    m_capacity = 0;
    m_urlLen = 0;
  }
  else
  {
    (void)std_strlcpy(m_url, urlStr, len+1);
    m_urlLen = len;
  }

  return m_errCode;
}

/**
 * @brief concatenate str to url
 *
 * @return UrlErrCode
 */

URL::UrlErrCode URL::ConcatUrl(const char * str)
{
  char *tmpUrl;
  bool capacityAdjusted = false;

  m_errCode = URL_OK;

  if ((NULL == str) || IsNull())
  {
    //no need to concat
    m_errCode = URL_BUFFER_NULL;
    return m_errCode;
  }

  uint32 srcLen = (uint32)std_strlen(str);

  //check whether concatenated string exceeds maximum length limitation
  if ((m_urlLen + srcLen) > MaxUrlSizeWithCtrlPath)
  {
    //no need to concatenate
    m_errCode = URL_MAX_LEN_EXCEEDS;
    return m_errCode;
  }

  tmpUrl = m_url;

  if ((m_urlLen + srcLen) > m_capacity)
  {
    tmpUrl = (char *)QTV_Malloc((m_urlLen + srcLen + 1 + 1) * sizeof(char));
    if (NULL == tmpUrl)
    {
      m_errCode = URL_OUT_OF_MEMORY;
      //lets leave the original state of URL object as it is, in this case
      return m_errCode;
    }
    std_strlcpy(tmpUrl, m_url, m_urlLen + srcLen + 1 + 1);

    //update capacity (extra +1 is probable "/" that may be
    //required to add to url
    m_capacity = m_urlLen + srcLen + 1 + 1;

    capacityAdjusted = true;
  }

  // Offset from the beginning of srcURL, from which starts to copy
  uint8 offset   = 0;

  // Extra len introduced by '/'
  int8 extraLen  = 0;

  // prepare to concatenate
  if ((tmpUrl[m_urlLen - 1] != '/') && (str[0] != '/'))
  {
    // If destURL doesn't ends with a '/' and srcURL doesn't begin with a '/',
    //add a '/' at the end
    extraLen = 1;
  }
  else if ((tmpUrl[m_urlLen - 1] == '/') &&
           (str[0] == '/') )
  {
    // If both have a '/' use only one of them
    offset   = 1;
    extraLen = -1;
  }
  else
  {
    // If only one of them has it('/'), we are fine
  }

  uint32 cpyLen = 0;
  cpyLen = srcLen - offset;

  if (cpyLen > 0)
  {
    if (extraLen > 0)
    {
      std_strlcat(tmpUrl, "/", m_capacity);
    }
    std_strlcat(tmpUrl, str + offset, m_capacity);
  }

  if ((capacityAdjusted) && (NULL != m_url))
  {
    QTV_Free(m_url);
  }

  m_url = tmpUrl;
  m_urlLen = m_urlLen + extraLen + offset + cpyLen;

  return m_errCode;
}


/**
 * @brief compares (case-insensitive) the url object with start of the url
 *
 * @return bool
 */
bool URL::StartsWith(const URL & otherUrl) const
{
  if ((!otherUrl.IsEmpty()) && !IsEmpty() &&
      (std_stribegins(m_url,otherUrl.m_url) != NULL))
  {
    return true;
  }
  return false;
}
#endif

/**
 * @brief compares the string with start of the url uses
 * case-insensivecompare
 *
 * @return bool
 */
bool URL::StartsWith(const char * str) const
{
  if ((NULL != str) && !IsEmpty() &&
      (std_stribegins(m_url, str) != NULL))
  {
    return true;
  }
  return false;
}

/**
 * @brief Compares (case-insensitive) the string with end of the URL
 *
 * @return bool
 */
bool URL::EndsWith(const char * str) const
{
  if ((NULL != str) && !IsEmpty() &&
      (std_striends(m_url, str) != NULL))
  {
    return true;
  }
  return false;
}

#if 0
/**
 * @brief Validates the format
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::ValidateURL() const
{
  ResetErrorCode();

  if (NULL == std_strstr(m_url, "//"))
  {
    m_errCode = URL_BAD_FORMAT;
  }

  return m_errCode;
}

/**
 * @brief strips the control character
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::StripControlChars()
{
  ResetErrorCode();

  if (IsEmpty())
  {
    m_errCode = URL_EMPTY;
    return m_errCode;
  }

  int newLength = strip_control_chars(m_url, m_urlLen);

  // Cast to uint32 is safe because strip_control_chars returns
  // a value >= 0.
  if (((uint32)newLength) != m_urlLen)
  {
    QTV_MSG_PRIO2(QTVDIAG_RTSP, QTVDIAG_PRIO_HIGH,
      "Modified original length from %d to %d",
      m_urlLen, newLength);
  }
  m_urlLen = newLength;

  return m_errCode;
}
#endif

/**
*@brief returns the streaming protocol
*
*@return UrlProtocol
*/
enum URL::UrlProtocol URL::GetProtocol() const
{
  if (std_strnicmp(m_url, "rtsp", 4) == 0)
  {
    return PROTO_RTSP;
  }
  else if (std_strnicmp(m_url, "http", 4) == 0)
  {
    return PROTO_HTTP;
  }
  else if (std_strnicmp(m_url, "isdb", 4) == 0)
  {
    return PROTO_ISDB;
  }
  else if (std_strnicmp(m_url, "sdp", 4) == 0)
  {
    return PROTO_SDP_BUFFER;
  }

  return PROTO_UNKNOWN;
}

/**
 * @brief returns the hosts ip
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::GetHost(char * host,size_t maxHostLen) const
{
  char *server_ip_ptr;
  char *server_port_ptr = NULL;
  char *ptr_to_first_slash_in_url = NULL;
  char *url_end;
  bool ipv4_ip_addr = false;
  mbchar ip_addr[HOST_NAME_SIZE];

  ResetErrorCode();

  if (IsEmpty())
  {
    m_errCode = URL_EMPTY;
    return m_errCode;
  }

  url_end = m_url + m_urlLen;

  server_ip_ptr = std_strstr(m_url, "//");
  if (server_ip_ptr == NULL)
  {
    m_errCode = URL_BAD_FORMAT;
    return m_errCode;
  }
  server_ip_ptr += 2;

  //process ipv6 urls
  if (std_strstr(m_url, "//[") == NULL)
  {
    //address seems to be an IPv4 address or literal address
    ipv4_ip_addr = true;
    server_port_ptr = std_strstr(server_ip_ptr, ":");

    ptr_to_first_slash_in_url = std_strstr(server_ip_ptr, "/");

    if ((NULL != server_port_ptr) &&
        (NULL != ptr_to_first_slash_in_url) &&
        (ptr_to_first_slash_in_url < server_port_ptr))
    {
      server_port_ptr = ptr_to_first_slash_in_url;
    }

    if (NULL == server_port_ptr)
    {
      //port not found so check for clip name start
      server_port_ptr = std_strstr(server_ip_ptr, "/");
      //we didn't find clip start, lets just return what we have
      if ((NULL == server_port_ptr) && (server_ip_ptr < url_end))
      {
        server_port_ptr = url_end;
      }

      if (NULL == server_port_ptr)
      {
        m_errCode = URL_BAD_FORMAT;
        return m_errCode;
      }
    }
    memset(ip_addr, 0, sizeof(ip_addr));
    size_t ipAddrLen = QTV_MIN((size_t)(server_port_ptr - server_ip_ptr) + 1, sizeof(ip_addr));
    (void)std_strlcpy(ip_addr, server_ip_ptr, ipAddrLen);
  }
  else
  {
    //ipv6 address
    server_ip_ptr += 1;
    server_port_ptr = std_strstr(server_ip_ptr, "]:");
  }

  //now lets check for ending "]"
  if (NULL == server_port_ptr)
  {
    server_port_ptr = std_strstr(server_ip_ptr, "]");
  }

  //if literal address or ipv4 ip_addr, check for port with ":"
  if (NULL == server_port_ptr)
  {
    server_port_ptr = std_strstr(server_ip_ptr, ":");
  }

  if (NULL != host)
  {
    if (ipv4_ip_addr)
    {
       (void)std_strlcpy(host,ip_addr,maxHostLen);
    }
    else
    {
       size_t len = QTV_MIN(maxHostLen-1,size_t(server_port_ptr - server_ip_ptr + 1));
       std_strlcpy(host, server_ip_ptr, len);
    }
  }

  return m_errCode;
}

/**
 * @brief Gets the server port
 * @param port
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::GetPort(uint32 * port) const
{
  char *server_ip_ptr, *clip_name;
  char *server_port_ptr = NULL;
  char port_str[PORT_STRING_SIZE];
  char *url_end;

  ResetErrorCode();

  if (NULL == port)
  {
    return m_errCode;
  }

  if (IsEmpty())
  {
    m_errCode = URL_EMPTY;
    return m_errCode;
  }

  url_end = m_url + m_urlLen;

  server_ip_ptr = std_strstr(m_url, "//");
  if (server_ip_ptr == NULL)
  {
    m_errCode = URL_BAD_FORMAT;
    return URL_BAD_FORMAT;
  }

  server_ip_ptr += 2;
  if (std_strstr(m_url, "//[") == NULL)
  {
    //address seems to be an IPv4 address or literal address
    server_port_ptr = std_strstr(server_ip_ptr, ":");
  }
  else
  {
    //ipv6 address
    server_port_ptr = std_strstr(server_ip_ptr, "]:");
  }

  clip_name = std_strstr(server_ip_ptr, "/");

  //in case port is present but no clip name start found
  if ((NULL == clip_name) && (server_port_ptr < url_end))
  {
    clip_name = url_end;
  }

  if (NULL == clip_name)
  {
    m_errCode = URL_BAD_FORMAT;
    return URL_BAD_FORMAT;
  }

  if ((NULL == server_port_ptr) || (server_port_ptr > clip_name))
  {
    UrlProtocol protocol = GetProtocol();

    //port not found; check if valid protocol
    if (protocol == PROTO_RTSP)
    {
      *port = DEF_RTSP_PORT;
      return URL_OK;
    }
    else if (protocol == PROTO_HTTP)
    {
      //No default port for HTTP since it could be
      //Progressive Download or Fast Track
      return URL_OK;
    }
    m_errCode = URL_UNKNOWN_PROTOCOL;
    return m_errCode;
  }

  if (*server_port_ptr == ']')
  {
    server_port_ptr += 2;
  }
  else
  {
    server_port_ptr++;
  }

  size_t port_str_len = QTV_MIN(PORT_STRING_SIZE,((clip_name - server_port_ptr) + 1));
  (void)std_strlcpy(port_str, server_port_ptr,port_str_len);

  const char * end_ptr = NULL;
  int errorno;
  *port = std_scanul(port_str, 0, &end_ptr, &errorno);

  return URL_OK;
}

/**
 * @brief gets the clip name from the url
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::GetClipName(char * clipName, size_t size) const
{
  char *server_ip_ptr, *clip_fname;

  ResetErrorCode();

  if (NULL == clipName)
  {
    return m_errCode;
  }

  if (IsEmpty())
  {
    m_errCode = URL_EMPTY;
    return m_errCode;
  }

  server_ip_ptr = std_strstr(m_url, "//");
  if (server_ip_ptr == NULL)
  {
    m_errCode = URL_BAD_FORMAT;
    return URL_BAD_FORMAT;
  }

  server_ip_ptr += 2;

  clip_fname = std_strstr(server_ip_ptr, "/");
  if (NULL == clip_fname)
  {
    m_errCode = URL_BAD_FORMAT;
    return URL_BAD_FORMAT;
  }

  std_strlcpy(clipName, clip_fname+1, size);

  return URL_OK;
}
#if 0
/**
 * @brief returns the error code
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::GetErrorCode() const
{
  return m_errCode;
}
#endif

void URL::ResetErrorCode() const
{
  m_errCode = URL_OK;
}

#if 0
/**
 * @brief returns true if the ip is ip version 6
 *
 * @return bool
 */
bool URL::IsIPv6Url() const
{
  if (NULL != std_strstr(m_url, "//["))
  {
    return true;
  }
  return false;
}

/**
 * @brief map the IPv4 url to IPv6 url
 * Implemented according to rfc2732
 * member variable changed to mapped url
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::MapIPv4urlToIPv4mappedIPv6url()
{
  mbchar *ip_ptr, *port_ptr;
  mbchar *mappedURL;
  ptrdiff_t len;

  ResetErrorCode();

  if (IsEmpty())
  {
    m_errCode = URL_EMPTY;
    return m_errCode;
  }

  //check for correctness of the URL
  ip_ptr = std_strstr(m_url, "//");
  if (ip_ptr == NULL)
  {
    return URL_BAD_FORMAT;
  }

  //create memory for mappedURL
   mappedURL = (mbchar *)QTV_Malloc(sizeof(mbchar)*(m_urlLen+10));
  if (NULL == mappedURL)
  {
    m_errCode = URL_OUT_OF_MEMORY;
    return m_errCode;
  }

  //now start processing of the url
  //copy until 'rtsp://'
  len = ip_ptr - m_url + 2;
  memcpy(mappedURL, m_url, len);

  //copy [::FFFF: into url
  memcpy(mappedURL+len, "[::FFFF:", 8);
  len += 8;

  ip_ptr += 2;
  port_ptr = std_strstr(ip_ptr, ":");

  if (NULL == port_ptr)
  {
    //port not found so check for clip name start
    port_ptr = std_strstr(ip_ptr, "/");
    if (NULL == port_ptr)
    {
      QTV_Free(mappedURL);
      m_errCode = URL_BAD_FORMAT;
      return m_errCode;
    }
  }

  //copy ip addr
  memcpy(mappedURL+len, ip_ptr, port_ptr - ip_ptr);
  len += port_ptr - ip_ptr;

  //enclose ip addr in ']'
  *(mappedURL+len) = ']';

  //copy rest of the string
  memcpy(mappedURL+len+1, port_ptr, std_strlen(port_ptr));
  len += 1 + (uint32)std_strlen(port_ptr);
  mappedURL[len] = '\0';

  //now free url, no longer required
  QTV_Free(m_url);

  //now copy the Ipv6 address
  m_url = mappedURL;

  return m_errCode;
}

/**
 * @brief Convert IPv4 Mapped IPv6 Url to IPv4 url, updates parsedURL itself
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::ConvertIPv4mappedIPv6urlToIPv4url()
{
  char *ip_addr_type_ptr;
  char *port_ptr;
  uint32 url_len;

  ResetErrorCode();

  if (IsEmpty())
  {
    m_errCode = URL_EMPTY;
    return m_errCode;
  }
  //TODO: a wrapper for std_strstri (zrex_strstri) has been written by satish.
  // we will use that.
  if ((ip_addr_type_ptr = std_strstr(m_url, "//[::FFFF:")) != NULL)
  {
    //address is IPv4 mapped IPv6 address
    char *turl = (char *) m_url;
    url_len = (uint32)std_strlen(m_url);
    port_ptr = std_strstr(turl, "]:");
    if (NULL == port_ptr)
    {
      port_ptr = std_strstr(turl, "]/");
      if (NULL == port_ptr)
      {
        m_errCode = URL_BAD_FORMAT;
      }
    }

    if (URL_OK == m_errCode)
    {
      //now  skip [::FFFF: and ]:
      (void)std_strlcpy(ip_addr_type_ptr + 2,
                        ip_addr_type_ptr + 10,
                        (size_t)(port_ptr - ip_addr_type_ptr - 9));

      (void)std_strlcpy(port_ptr - 8,
                        port_ptr + 1,
                        (size_t)(turl + url_len - port_ptr));
    }
  }
  return m_errCode;
}

/**
 * @brief Converts a IPv4 binary address to a IPv4 mapped IPv6 address
 *
 * @return URL::UrlErrCode
 */
URL::UrlErrCode URL::ConvertIPv4AddrToIPv4MappedIPv6Addr(char *ipv4Addr,
                                                   struct in6_addr *ipv6Addr)
{
  URL::UrlErrCode eCode = URL_ERROR;

  if ((NULL != ipv6Addr) && (NULL != ipv4Addr))
  {
    memset(ipv6Addr, 0x00, sizeof(struct in6_addr));
    memcpy((char *) ipv6Addr +
            sizeof(struct in6_addr) - sizeof(struct in_addr),
            ipv4Addr,
            sizeof(struct in_addr));

    *((char *) ipv6Addr +
           sizeof(struct in6_addr) - sizeof(struct in_addr) - 1) = 0x7F;
    *((char *) ipv6Addr +
           sizeof(struct in6_addr) - sizeof(struct in_addr) - 2) = 0x7F;
    eCode = URL_OK;
  }
  return eCode;
}

/**
 * @brief Sets the maximum URL length (with and without control path)
 *
 * @param maxURLSize
 */
void URL::SetMaxURLSize(const uint32 maxURLSize)
{
  MaxUrlSize = maxURLSize;
  MaxUrlSizeWithCtrlPath = MaxUrlSize + QTV_CONTROL_RELATIVE_URL_LEN;
}
#endif

