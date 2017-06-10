#ifndef __HTTPSESSIONINFO_H__
#define __HTTPSESSIONINFO_H__
/************************************************************************* */
/**
 * HTTPSessionInfo.h
 * @brief Header file for HTTPSessionInfo.
 *
 * COPYRIGHT 2011-2015 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPSessionInfo.h#26 $
$DateTime: 2013/09/20 11:38:26 $
$Change: 4469780 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSessionInfo.h
** ======================================================================= */
#include "HTTPCommon.h"
#include "IPStreamProtocolHeaders.h"
#include "HTTPCookieStore.h"
#include <oscl_string.h>
#include <Url.h>

#ifndef WIN32
#include <cutils/properties.h>
#endif

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
#define HTTP_MAX_USERAGENT_LEN                             256
#define HTTP_MAX_CONTENTTYPE_LEN                           100
#define HTTP_MAX_HOSTNAME_LEN                              50
#define HTTP_MAX_RANGE_LEN                                 50
#define HTTP_DEFAULT_FILE_PATH_PREFIX                      "./"
#define HTTP_DEFAULT_TEMP_DOWNLOAD_FILE                    "HTTPTempDnldFile.3gp"
#define HTTP_MAX_FILEPATH_LEN                              256
#define HTTP_DEFAULT_TEMP_DOWNLOAD_FILE_PATH               "./HTTPTempDnldFilePath.dat"
#define HTTP_DEFAULT_BROKEN_DOWNLOAD_INFO_FILE_PATH        "./HTTPBrokenDownload.dat"
#define HTTP_MAX_BROKEN_DOWNLOAD_INFO_FILE_LEN             512
#define HTTP_MAX_DATA_CHUNK_LEN                            (8 * 1024)
#define HTTP_MAX_DATA_READ_LIMIT                           (64 * 1024)
#define HTTP_MAX_REP_BANDWIDTH                             (20 * 1024 * 1024)
#define HTTP_MAX_AUDIO_SPECIFIC_CONFIG_VALUE               6
//max 720p resolution for HEVC 8916
#define MAX_HEVC_HD_RESOLUTION                             (1280 *720)

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class HTTPSessionInfo
{
public:
  HTTPSessionInfo() :
    m_networkIface(-1),
    m_primaryPDPProfileNo(-1),
    m_bIsDashEmbmsSesison(false),
    m_ProxyServer(NULL),
    m_bIsCancellationDisabled(false),
    m_MaxSupportedAudioSpecificConfigValue(HTTP_MAX_AUDIO_SPECIFIC_CONFIG_VALUE),
    m_bIsStartupOptimizationEnabled(false),
    m_bIsAudioSwitchingEnabled(false)
  {
    Reset();

    m_MaxSupportedRepResolution = MAX_RESOLUTION;
    m_MaxSupportedHevcRepResolution = MAX_RESOLUTION;
    m_MaxSupportedRepBw = HTTP_MAX_REP_BANDWIDTH;

#ifndef WIN32

    char maxResolution[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.dash.max.rep.resolution", maxResolution, NULL);
    if(*maxResolution)
    {
      char* pMaxHeight = std_strchr(maxResolution, '*');
      if(pMaxHeight)
      {
        uint32 maxWidth = atoi(maxResolution);
        uint32 maxHeight = atoi(pMaxHeight+1);
        if((maxWidth * maxHeight) > 0)
        {
          m_MaxSupportedRepResolution = maxWidth * maxHeight;
          m_MaxSupportedHevcRepResolution = maxWidth * maxHeight;
        }
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Using m_MaxSupportedRepResolution %u", (uint32)m_MaxSupportedRepResolution);
      }
    }

    char platform_name[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.board.platform", platform_name, "0");
    if (!strncmp(platform_name, "msm8916",7))
    {
       m_MaxSupportedHevcRepResolution = MAX_HEVC_HD_RESOLUTION;
       QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Target identified as 8916 and max supported hevc rosulution is %u",
                      m_MaxSupportedHevcRepResolution);
    }

    char maxBw[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.dash.max.rep.bandwidth", maxBw, NULL);
    if(*maxBw)
    {
      if(atoi(maxBw) > 0)
      {
        m_MaxSupportedRepBw = atoi(maxBw);
      }

      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "persist.dash.max.rep.bandwidth %u (capped to %u)",
                       m_MaxSupportedRepBw, HTTP_MAX_REP_BANDWIDTH);
    }
#endif
    m_MaxSupportedVideoBufferSize = (m_MaxSupportedRepResolution * 3/2);
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Using m_MaxSupportedVideoBufferSize %u",
                      (uint32)m_MaxSupportedVideoBufferSize);

  };
  virtual ~HTTPSessionInfo();

  void SetURL(const URL& url)
  {
    m_URL = url;
  }
  const URL& GetURL() const
  {
    return m_URL;
  }

  void SetDataHeapStorageLimit(const int32 nHeapStorageLimit)
  {
    m_nDataHeapStorageLimit = nHeapStorageLimit;
  }
  int32 GetDataHeapStorageLimit() const
  {
    return m_nDataHeapStorageLimit;
  }

  void SetUserAgent(const char* pUserAgent)
  {
    if (pUserAgent)
    {
      std_strlcpy(m_userAgent, pUserAgent, sizeof(m_userAgent));
    }
  }
  const char* GetUserAgent() const
  {
    return (const char*)m_userAgent;
  }

  void SetPlaybackHandler(void* pPlaybackHandler)
  {
    m_pPlaybackHandler = (iHTTPPlaybackHandler*)pPlaybackHandler;
  }
  iHTTPPlaybackHandler* GetPlaybackHandler()
  {
    return m_pPlaybackHandler;
  }

  void SetDataStorageType(const DataStorageType dataStorageType)
  {
    m_dataStorageType = dataStorageType;
  }

  void SetHTTPRequestsLimit(const uint32 httpReqsLimit)
  {
    m_nHttpRequestsLimit = httpReqsLimit;
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "MaxHttpRequestsLimit overriden to %d", (int)httpReqsLimit);
  }

  uint32 GetHTTPRequestsLimit()
  {
    return m_nHttpRequestsLimit;
  }

  void SetInitialPreroll(const uint32 preroll, const bool bConfig = true)
  {
    m_sInitialPreroll.nComputed = preroll;
    if (bConfig)
    {
      m_sInitialPreroll.nConfigured = (int)m_sInitialPreroll.nComputed;
    }
  }

  uint32 GetInitialPreroll()
  {
    //Preroll takes effect in the following order of preference
    //configured (e.g. HTTPPropertiesConfig/app) -> computed (e.g. MPD/M3U8) -> default
    return ((m_sInitialPreroll.nConfigured >= 0) ? m_sInitialPreroll.nConfigured
                                                 : m_sInitialPreroll.nComputed);
  }

  void SetRebufferPreroll(const uint32 preroll, const bool bConfig = true)
  {
    m_sRebufferPreroll.nComputed = preroll;
    if (bConfig)
    {
      m_sRebufferPreroll.nConfigured = (int)m_sRebufferPreroll.nComputed;
    }
  }

  uint32 GetRebufferPreroll()
  {
    //Preroll takes effect in the following order of preference
    //configured (e.g. HTTPPropertiesConfig/app) -> computed (e.g. MPD/M3U8) -> default
    return ((m_sRebufferPreroll.nConfigured >= 0) ? m_sRebufferPreroll.nConfigured
                                                  : m_sRebufferPreroll.nComputed);
  }

  void SetNetworkInterface(int32 iface)
  {
    m_networkIface = iface;
  }

  void SetPrimaryPDPProfile(int32 profileNo)
  {
    m_primaryPDPProfileNo = profileNo;
  }

  int32 GetNetworkInterface() const
  {
    return m_networkIface;
  }

  int32 GetPrimaryPDPProfile() const
  {
    return m_primaryPDPProfileNo;
  }

  const char* GetProxyServer() const
  {
    return m_ProxyServer? m_ProxyServer->get_cstr():NULL;
  }

  bool SetProxyServer(const char* proxyServer, size_t ProxyServerLen);

  bool SetCookie (const char* url,  const char* cookie);

  bool GetCookies(const char *url,  const char *cookies, size_t &cookiesLen);

  void Reset();


  void SetEmbmsSession(bool bIsEmbmsSession)
  {
    m_bIsDashEmbmsSesison = bIsEmbmsSession;
  }

  bool IsEmbmsSession()
  {
    return m_bIsDashEmbmsSesison;
  }

  bool SetOemHttpHeaders(IPStreamProtocolHeaderCommand whatToDo,
                         uint32 whichMethodsAffected,
                         const char *headerName,
                         const char *headerValue);

  bool GetOemHttpHeaders(uint32 whichMethodsAffected,
                         const char *headerName,
                         char *headerValue,
                         int& headerValueSize);

  IPStreamProtocolHeaders& GetProtocolHeadersCache();

  HTTPCookieMgr& GetCookieStore();

  void SetDisableTimeout(bool bShouldDisable);
  bool IsTaskTimeoutDisabled() const;

  void SetAudioSwitchingEnabled(bool bIsEnabled);

  bool IsAudioSwitchable() const;

  void SetDataUnitCancellationDisabled(bool bIsDisabled)
  {
    m_bIsCancellationDisabled = bIsDisabled;
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Set cancellation disabled config to %d", (int)bIsDisabled);
  }

  bool IsDataUnitCancellationDisabled() const
  {
    return m_bIsCancellationDisabled;
  }

  void UseTsbForStartupLatencyImprovement()
  {
    m_bIsStartupOptimizationEnabled = true;
  }

  bool IsStartupLatencyImprovementEnabled() const
  {
    return m_bIsStartupOptimizationEnabled;
  }

  void SetMaxSupportedRepBandwidth(uint32 maxBw)
  {
    m_MaxSupportedRepBw = maxBw;
  }

  uint32 GetMaxSupportedRepBandwidth() const
  {
    return m_MaxSupportedRepBw;
  }

  uint32 GetMaxSupportedVideoBufferSize()
  {
    return m_MaxSupportedVideoBufferSize;
  }

  void SetMaxSupportedVideoBufferSize(uint32 maxResolution)
  {
    m_MaxSupportedVideoBufferSize = (maxResolution * 3/2);
  }

  void SetMaxSupportedRepResolution(uint32 maxResolution)
  {
    m_MaxSupportedRepResolution = maxResolution;
  }

  uint32 GetMaxSupportedRepResolution() const
  {
    return m_MaxSupportedRepResolution;
  }

  uint32 GetMaxSupportedHevcRepResolution() const
  {
    return m_MaxSupportedHevcRepResolution;
  }

  void SetMaxSupportedAudioSpecificConfigValue(uint32 maxASCValue)
  {
    m_MaxSupportedAudioSpecificConfigValue = maxASCValue;
  }

  uint32 GetMaxSupportedAudioSpecificConfigValue() const
  {
    return m_MaxSupportedAudioSpecificConfigValue;
  }

private:
  URL m_URL;
  int32 m_nDataHeapStorageLimit;
  char m_userAgent[HTTP_MAX_USERAGENT_LEN];
  iHTTPPlaybackHandler* m_pPlaybackHandler;
  DataStorageType m_dataStorageType;

  //oem headers
  IPStreamProtocolHeaders m_httpOemHeaders;

  //Cookie Manager
  HTTPCookieMgr m_httpCookieMgr;

  //preroll values
  //ToDo: SessionInfo can parse/maintain config database instead of at MMI layer,
  //for now store configured preroll value alone here!
  struct sPreroll
  {
    uint32 nComputed;
    int nConfigured;
  };
  sPreroll m_sInitialPreroll;
  sPreroll m_sRebufferPreroll;

  uint32 m_nHttpRequestsLimit;

  // NetPolicy info
  int32 m_networkIface;
  int32 m_primaryPDPProfileNo;

  bool m_bIsDashEmbmsSesison;

  OSCL_STRING* m_ProxyServer;

  // Used to temporarily disable timeouts eg when waiting on an action
  // from client.
  bool m_bDisableTimeout;

  bool m_bIsCancellationDisabled;

  uint32 m_MaxSupportedRepBw;

  uint32 m_MaxSupportedRepResolution;

  uint32 m_MaxSupportedHevcRepResolution;

  uint32 m_MaxSupportedVideoBufferSize;

  uint32 m_MaxSupportedAudioSpecificConfigValue;

  bool m_bIsStartupOptimizationEnabled;

  bool m_bIsAudioSwitchingEnabled;
};

}/* namespace video */
#endif /* __HTTPSESSIONINFO_H__ */
