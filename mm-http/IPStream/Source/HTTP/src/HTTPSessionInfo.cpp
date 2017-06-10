/************************************************************************* */
/**
 * HTTPSessionInfo.cpp
 * @brief implementation of HTTPSessionInfo.
 *  HTTPSessionInfo maintains important paramaters for the HTTP session. This
 *  information is set by HTTPDownloader and used by HTTPDownloadHelper classes.
 *  Progressive Download and Fast Track specific parameters are stored in
 *  ProgressiveDownloadInfo and FastTrackInfo respectively.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPSessionInfo.cpp#24 $
$DateTime: 2013/09/25 22:26:01 $
$Change: 4497223 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSessionInfo.cpp
** ======================================================================= */
#include "HTTPSessionInfo.h"
#include "IPStreamSourceUtils.h"
#include <SourceMemDebug.h>

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
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

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief HTTPSessionInfo Destructor.
  *
  */
HTTPSessionInfo::~HTTPSessionInfo()
{
  m_networkIface = -1;
  m_primaryPDPProfileNo = -1;
}
/** @brief Initializes the class members.
  *
  */
void HTTPSessionInfo::Reset()
{
  m_URL = NULL;
  m_nDataHeapStorageLimit = -1;
  std_memset((void*)m_userAgent, 0x0, sizeof(m_userAgent));
  m_pPlaybackHandler = NULL;
  m_dataStorageType = iHTTPAPI::DEFAULT_STORAGE;

  m_sInitialPreroll.nComputed = m_sRebufferPreroll.nComputed = HTTP_DEFAULT_ONDEMAND_PREROLL_MSEC;
  m_sInitialPreroll.nConfigured = m_sRebufferPreroll.nConfigured = -1;

  m_bDisableTimeout = false;
  m_nHttpRequestsLimit = HTTP_MAX_CONNECTIONS;
}

/** @brief Sets oem headers
  *
  * @param[in] whatToDo action to be taken
  * @param[in] whichMethodsAffected list of all methods to which
  *            these headers applied for
  * @param[in] headerName header name
  * @param[in] headerValue header value
  *
  * @return
  * TRUE - if successful
  * FALSE - Otherwise
  */
bool HTTPSessionInfo::SetOemHttpHeaders(IPStreamProtocolHeaderCommand whatToDo,
                                           uint32 whichMethodsAffected,
                                           const char *headerName,
                                           const char *headerValue)
{
  bool rslt = false;

  // Check if the header is cookie header
  if(!std_strnicmp(headerName, "Set-Cookie", QTV_MIN(std_strlen(headerName), std_strlen("Set-Cookie"))))
  {
    if(headerValue)
    {
      // Cookies will get stored in the cookie manager
      GetCookieStore().StoreCookie((char *)headerValue, (char *)GetURL().GetUrlBuffer());
    }

    rslt = true;
  }
  else
  {
    rslt = (IPSTREAM_PROTOCOL_RESULT_OK == m_httpOemHeaders.EditIPStreamProtocolHeaders(
                             whatToDo, (int)whichMethodsAffected, headerName, headerValue))
         ? true : false;
  }

  return rslt;
}

/** @brief Gets oem headers
  *
  * @param[in] whichMethodsAffected list of all methods to which
  *            these headers applied for
  * @param[in] headerName header name
  * @param[out] headerValue header value
  * @param[out] headerValueSize header value
  *
  * @return
  * TRUE - if successful
  * FALSE - Otherwise
  */
bool HTTPSessionInfo::GetOemHttpHeaders(uint32 whichMethodsAffected,
                                           const char *headerName,
                                           char *headerValue,
                                           int& headerValueSize)
{
  bool ret = true;
  if (NULL == headerValue)
  {
    // query for reqd buf size to hold value
    headerValueSize = 0;
  }

  if (NULL == headerName)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "UserConfig::ValueFor Null oem header name");

    headerValueSize = 0;
    ret = false;
  }
  else
  {
    const char *val = m_httpOemHeaders.ValueFor((int)whichMethodsAffected, headerName);

    if (val)
    {
      size_t valLen = 1 + std_strlen(val);
      QTV_MSG_SPRINTF_2( QTVDIAG_HTTP_STREAMING, "Oem value for '%s' found. reqdBufsize %d",
                         headerName, valLen);

      if (NULL == headerValue)
      {
        // Client is requesting the reqd buffer size for header value
        headerValueSize = (int)valLen;
      }
      else
      {
        // populate headerValue
        int maxToCopy = QTV_MIN(headerValueSize, (int)valLen);

        if (maxToCopy > 0)
        {
          std_strlcpy(headerValue, val, maxToCopy);
          headerValueSize = maxToCopy;
        }
        else
        {
          headerValueSize = 0;
        }
      }
    }
    else
    {
      QTV_MSG_SPRINTF_1( QTVDIAG_HTTP_STREAMING, "No oem header for key '%s'", headerName);
      headerValueSize = 0;
      ret = false;
    }
  }
  return ret;
}

/** @brief Gets protocol headers member
  *
  * @return
  * protocol headers member reference
  */
IPStreamProtocolHeaders& HTTPSessionInfo::GetProtocolHeadersCache()
{
  return m_httpOemHeaders;
}

/** @brief Gets Cookie Store Instance
  *
  * @return
  * protocol headers member reference
  */
HTTPCookieMgr& HTTPSessionInfo::GetCookieStore()
{
  return m_httpCookieMgr;
}

/**
 * @brief This method stores the proxyServer info
 *
 * @param proxyServer
 * @param ProxyServerLen
 *
 * @return bool
 */
bool HTTPSessionInfo::SetProxyServer(const char* proxyServer, size_t ProxyServerLen)
{
  bool retVal = false;

  if(proxyServer != NULL && ProxyServerLen > 0)
  {
    if(m_ProxyServer != NULL)
    {
      QTV_Delete(m_ProxyServer);
      m_ProxyServer = NULL;
    }
    m_ProxyServer = QTV_New_Args(OSCL_STRING, (proxyServer));
    if(m_ProxyServer)
    {
      retVal = true;
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "SetproxyServer: Invalid input; proxyServer:0x%p,proxyServerLen:%d",
                     (void *)proxyServer, ProxyServerLen);
  }
  return retVal;
}

/**
 * @brief This method stores the Cookie info
 *
 * @param proxyServer
 * @param ProxyServerLen
 *
 * @return bool
 */
bool HTTPSessionInfo::SetCookie(const char* url, const char* cookie)
{
  bool rslt = false;
  if(cookie)
  {
    // Cookies will get stored in the cookie manager
    GetCookieStore().StoreCookie((char *)cookie, (char *)url, true);
    rslt = true;
  }
  return rslt;
}


/**
* @breif: Get the Cookied info
*
* @param[in] url
* @param[in] cookie
* @param[in/out] cookie len
*
* @return 0 if success, -1 otherwise
*/
bool HTTPSessionInfo::GetCookies
(
  const char *url,
  const char *cookies,
  size_t &cookiesLen
)
{
  size_t headerLen = 0;
  return  GetCookieStore().GetCookies((char *)url, NULL, headerLen, (char *)cookies, cookiesLen, true);
}

/**
 * @brief
 *  Set the 'disable' timeout to determine if tasks should
 *  refrain from timing out.
 *
 * @param bShouldDisable
 */
void HTTPSessionInfo::SetDisableTimeout(bool bShouldDisable)
{
  if (bShouldDisable != m_bDisableTimeout)
  {
    m_bDisableTimeout = bShouldDisable;
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
      "HTTPSessionInfo disabletimeout flag updated to %d", m_bDisableTimeout);
  }
}

/**
 * @brief
 *  Determine if task should refrani from timing out.
 *
 * @return bool
 */
bool HTTPSessionInfo::IsTaskTimeoutDisabled() const
{
  return m_bDisableTimeout;
}

void HTTPSessionInfo::SetAudioSwitchingEnabled(bool bIsEnabled)
{
  m_bIsAudioSwitchingEnabled = bIsEnabled;

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "AudioSwitchingEnabled? %d", m_bIsAudioSwitchingEnabled);
}

bool HTTPSessionInfo::IsAudioSwitchable() const
{
  return m_bIsAudioSwitchingEnabled;
}

}/* namespace video */
