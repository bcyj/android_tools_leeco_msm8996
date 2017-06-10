#ifndef __HTTPDOWNLOADER_H__
#define __HTTPDOWNLOADER_H__
/************************************************************************* */
/**
 * HTTPDownloader.h
 * @brief Header file for HTTPDownloader.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPDownloader.h#30 $
$DateTime: 2013/09/20 11:38:26 $
$Change: 4469780 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDownloader.h
** ======================================================================= */
#include "HTTPResolver.h"
#include "HTTPCommon.h"
#include "HTTPSessionInfo.h"
#include "HTTPController.h"
#include "IPStreamProtocolHeaders.h"
#include <Scheduler.h>
#include <HTTPStackInterface.h>

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
struct tOpenParams
{
  DataStorageType dataStorageType;
  int32 nHeapStorageLimit;
};

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPResolver;
class HTTPDownloadHelper;
class HTTPDataManager;
class HTTPDataInterface;
/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class HTTPDownloader
{
public:
  HTTPDownloader(void* pStatusHandlerIQI, bool& bOk);
  virtual ~HTTPDownloader();

  bool StartScheduler();
  bool StopScheduler();
  bool AddSchedulerTask(SchedulerTask pTask,
                        void* pTaskParam);
  bool DeleteSchedulerTask(const int taskID);

  HTTPDownloadStatus StartSession(const URL& urn,
                                  const tOpenParams& params,
                                  void* pPlaybackHandler,
                                  HTTPControllerCb pCallback,
                                  const HTTPControllerCbData& callbackData);
  HTTPDownloadStatus PauseSession();
  HTTPDownloadStatus ResumeSession();
  HTTPDownloadStatus InitializeDownloader(HTTPControllerCb pCallback,
                                    const HTTPControllerCbData& callbackData);
  HTTPDownloadStatus StartDownload(HTTPControllerCb pCallback,
                                   const HTTPControllerCbData& callbackData);
  HTTPDownloadStatus CloseSession(HTTPControllerCb pCallback,
                                  const HTTPControllerCbData& callbackData);
  HTTPDownloadStatus SeekSession(const int64 seekTime,
                                 HTTPControllerCb pCallback,
                                 const HTTPControllerCbData& callbackData);

  static int TaskResolveHTTPFlavor(void* pTaskParam);
  static int TaskInitializeDownloader(void* pTaskParam);
  static int TaskConnectAndDownloadHeader(void* pTaskParam);
  static int TaskDownloadData(void* pTaskParam);
  static int TaskCloseSession(void* pTaskParam);
  static int TaskSeekSession(void* pTaskParam);
  static int TaskSelectRepresentations(void *pTaskParam);
  void SetNetAbort();
  bool ResetSession();

  bool IsLiveStreamingSession();

  uint64 GetTotalbytesReceived() const
  {
    return m_nTotalBytesReceived;
  }

  iHTTPPlaybackHandler* GetPlaybackHandler()
  {
    return m_sessionInfo.GetPlaybackHandler();
  }

  int32 GetProxyServer (char* ProxyServer,
                       size_t ProxyServerLen,
                       size_t &ProxyServerLenReq);

  bool SetProxyServer ( const char* ProxyServer,
                         size_t ProxyServerLen);

  bool SetCookie (const char* url,
                   const char* cookie);

  bool GetCookies(const char *url,  const char *cookies, size_t &cookiesLen);

  bool SetNetworkIface(int32 networkIface);
  bool SetPrimaryPDPProfile(int32 pdpProfileNo);

  bool GetNetworkIface(int32 &networkIface);
  bool GetPrimaryPDPProfile(int32 &pdpProfileNo);

  bool SetOemHttpHeaders(IPStreamProtocolHeaderCommand whatToDo,
                         uint32 whichMethodsAffected,
                         const char *headerName,
                         const char *headerValue);
  bool GetOemHttpHeaders(uint32 whichMethodsAffected,
                         const char *headerName,
                         char *headerValue,
                         int& headerValueSize);

  void SetInitialPreroll(const uint32 preroll)
  {
    m_sessionInfo.SetInitialPreroll(preroll);
  }
  uint32 GetInitialPreroll()
  {
    return m_sessionInfo.GetInitialPreroll();
  }
  void SetRebufferPreroll(const uint32 preroll)
  {
    m_sessionInfo.SetRebufferPreroll(preroll);
  }
  uint32 GetRebufferPreroll()
  {
    return m_sessionInfo.GetRebufferPreroll();
  }

  void SetDataUnitCancellationDisabled(bool bIsDisabled)
  {
    m_sessionInfo.SetDataUnitCancellationDisabled(bIsDisabled);
  }

  bool GetDataUnitCancellationDisabled() const
  {
    return m_sessionInfo.IsDataUnitCancellationDisabled();
  }

  void SetMaxSupportedRepBandwidth(uint32 maxBw)
  {
    m_sessionInfo.SetMaxSupportedRepBandwidth(maxBw);
  }

  uint32 GetMaxSupportedRepBandwidth() const
  {
    return m_sessionInfo.GetMaxSupportedRepBandwidth();
  }

  void SetMaxSupportedASCValue(uint32 maxASCVal)
  {
    m_sessionInfo.SetMaxSupportedAudioSpecificConfigValue(maxASCVal);
  }

  uint32 GetMaxSupportedASCValue() const
  {
    return m_sessionInfo.GetMaxSupportedAudioSpecificConfigValue();
  }

  void UseTsbForStartupLatencyImprovement()
  {
    m_sessionInfo.UseTsbForStartupLatencyImprovement();
  }

  bool IsStartupLatencyImprovementEnabled() const
  {
    return m_sessionInfo.IsStartupLatencyImprovementEnabled();
  }

  void SetAudioSwitchingEnabled(bool bIsEnabled)
  {
    m_sessionInfo.SetAudioSwitchingEnabled(bIsEnabled);
  }

  bool IsAudioSwitchingEnabled() const
  {
    return m_sessionInfo.IsAudioSwitchable();
  }

  void SetHTTPRequestsLimit(const uint32 httpReqsLimit)
  {
    m_sessionInfo.SetHTTPRequestsLimit(httpReqsLimit);
  }
  uint32 GetHTTPRequestsLimit()
  {
    return m_sessionInfo.GetHTTPRequestsLimit();
  }

  HTTPFlavor GetHTTPFlavor()
  {
    return m_HTTPFlavor;
  }

  uint32 GetMaxSupportedVideoBufferSize()
  {
    return m_sessionInfo.GetMaxSupportedVideoBufferSize();
  }

  bool GetTotalDuration(uint32 &duration);

  bool GetDownloadProgress(HTTPMediaType mediaType,
                           uint32& currStartOffset,
                           uint32& downloadOffset,
                           HTTPDownloadProgressUnitsType eUnitsType,
                           bool& bEOS);

  void DisableTaskTimeout(bool bShouldDisableTimeout);

  HTTPDataInterface *GetDataInterface();
  bool GetMediaProperties(char *pPropertiesStr,
                          uint32 &nPropertiesLen);
  HTTPDownloadStatus SelectRepresentations(const char* SetSelectionsXML,
                                           HTTPControllerCb pCallback,
                                           const HTTPControllerCbData& callbackData);
  bool GetMPDText(char *pMPDTextStr, uint32 &mpdSize);

  void GetQOEData(uint32 &bandwidth, char *pVideoURL, size_t& nURLSize, char *pIpAddr, size_t& nIPAddrSize);

private:
  HTTPDownloadStatus CreateDownloadHelper();
  HTTPDownloadStatus CreateConnectAndDownloadHeaderTask
                                      (uint32 nStartTime,
                                         const tOpenParams& params,
                                         HTTPControllerCb pCallback,
                                         const HTTPControllerCbData& callbackData);

  bool GetURNInfo(const URL& urn, URL& url);

  bool SetSessionInfo(const URL& url,
                      const tOpenParams& params,
                      void* pPlaybackHandler);
  bool IsPaused();
  struct HTTPControllerCbUserData
  {
    HTTPControllerCb pControllerCb;
    HTTPControllerCbData controllerCbData;
  };
  class HTTPDownloaderTaskParam : public SchedulerTaskParamBase
  {
  public:
    HTTPDownloaderTaskParam()
    {
      pCbSelf = NULL;
      nStartTime = 0;
      std_memset((void*)&openParams, 0x0, sizeof(openParams));
      std_memset((void*)&callbackUserData, 0x0, sizeof(callbackUserData));
    };
    HTTPDownloaderTaskParam
    (
      void* pCbSelfIn,
      uint32 nStartTimeIn,
      HTTPControllerCb pCallbackIn,
      HTTPControllerCbData pCallbackCbDataIn
    ) : pCbSelf(pCbSelfIn),
      nStartTime(nStartTimeIn)
    {
      callbackUserData.pControllerCb = pCallbackIn;
      callbackUserData.controllerCbData = pCallbackCbDataIn;
      std_memset((void*)&openParams, 0x0, sizeof(openParams));
    };
    virtual ~HTTPDownloaderTaskParam(){ };

    void* pCbSelf;
    uint32 nStartTime;
    tOpenParams openParams;
    HTTPControllerCbUserData callbackUserData;
  };
  class SeekSessionTaskParam : public HTTPDownloaderTaskParam
  {
  public:
    SeekSessionTaskParam(){ };
     SeekSessionTaskParam
    (
      void* pCbSelfIn,
      uint32 nStartTimeIn,
      HTTPControllerCb pCallbackIn,
      HTTPControllerCbData pCallbackCbDataIn,
      int64 nSeekTimeIn
    ) : HTTPDownloaderTaskParam(pCbSelfIn,nStartTimeIn,pCallbackIn,pCallbackCbDataIn),
        nSeekTime(nSeekTimeIn)
    {
    };
    virtual ~SeekSessionTaskParam(){ };

    int64 nSeekTime;
  };
  class SelectRepresentationsTaskParam : public HTTPDownloaderTaskParam
  {
  public:
    SelectRepresentationsTaskParam
    (
      void* pCbSelfIn,
      uint32 nStartTimeIn,
      HTTPControllerCb pCallbackIn,
      HTTPControllerCbData pCallbackCbDataIn
    ) : HTTPDownloaderTaskParam(pCbSelfIn,nStartTimeIn,pCallbackIn,pCallbackCbDataIn),
        m_bIsRequestQueued(false),
        m_pSelectionsXML(NULL)
    {
    };
    virtual ~SelectRepresentationsTaskParam()
    {
      if (m_pSelectionsXML)
      {
        QTV_Free(m_pSelectionsXML);
      }
    }

    bool m_bIsRequestQueued;
    char *m_pSelectionsXML;
  private:
    SelectRepresentationsTaskParam();
  };

  Scheduler* m_pScheduler;
  HTTPDownloadHelper* m_pDownloadHelper;
  HTTPStackInterface* m_pHTTPStack;
  HTTPSessionInfo m_sessionInfo;
  HTTPResolver* m_pHttpResolver;
  StreamSourceClock* m_pSourceClock;
  MM_HANDLE m_pDownloaderDataLock;
  HTTPStatusHandlerInterface *m_pStackNotificationHandler;

  HTTPFlavor m_HTTPFlavor;
  bool m_bSessionInterrupted;
  uint64 m_nTotalBytesReceived;
  bool m_bSeekInProgress;
  bool m_bPaused;
};

}/* namespace video */
#endif /* __HTTPDOWNLOADER_H__ */
