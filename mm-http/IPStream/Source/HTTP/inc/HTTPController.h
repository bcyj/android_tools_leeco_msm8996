#ifndef __HTTPCONTROLLER_H__
#define __HTTPCONTROLLER_H__
/************************************************************************* */
/**
 * HTTPController.h
 * @brief Header file for HTTPController.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPController.h#33 $
$DateTime: 2013/09/20 11:38:26 $
$Change: 4469780 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPController.h
** ======================================================================= */
#if defined (WINCE) || defined (WIN32)
#include <windows.h>
#else
#include "MMThread.h"
#endif // defined (WINCE) || defined (WIN32)
#include "HTTPCommon.h"
#include "HTTPControllerHelper.h"
#include "HTTPSessionInfo.h"
#include "Scheduler.h"
#include "HTTPCmdQueue.h"
#include "IPStreamProtocolHeaders.h"
#include <StreamQueue.h>
#include <MMThread.h>
#include <MMCriticalSection.h>
#include "filesource.h"

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
#define HTTP_CONNECT_TIMEOUT_MSEC                120000
#define HTTP_DATAINACTIVITY_TIMEOUT_MSEC         60000
#define HTTP_CLOSE_TIMEOUT_MSEC                  5000
#define HTTP_SEEK_TIMEOUT_MSEC                   30000
#define HTTP_DEFAULT_TIMEOUT_MSEC                5000

#define HTTP_STREAMER_STACK_SIZE                 12 * 1024
#if defined (WINCE) || defined (WIN32)
#define HTTP_STREAMER_THREAD_PRIORITY            THREAD_PRIORITY_ABOVE_NORMAL
#else
#define HTTP_STREAMER_THREAD_PRIORITY            MM_Thread_DefaultPriority
#endif // defined (WINCE) || defined (WIN32)

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPDownloader;
class HTTPDataInterface;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
//HTTP notification handler interface
class iHTTPNotificationHandler
{
public:
  virtual ~iHTTPNotificationHandler() {};

  virtual void NotifyHTTPEvent(const HTTPControllerCommand HTTPCommand,
                               const HTTPDownloadStatus HTTPStatus,
                               void* pCbData) = 0;
};

class HTTPController : public HTTPStatusHandlerInterface
{
public:
  HTTPController(iHTTPNotificationHandler* pNotificationHandler,
                 void* pHTTPStackStatusHandler,
                 bool& bOk);
  virtual ~HTTPController();

  //HTTPStatusHandlerInterface methods - Handling HTTP stack notifications
  virtual bool Notify(HTTPStackNotifyCode HTTPStackStatus,void *pCbData);

  friend class HTTPControllerCmdExecHelper;

  //HTTP streamer thread entry point routine
  static int HTTPStreamerThreadEntryFunction(void* pTaskParam);

  //HTTP controller command processing sub-task entry function
  static int TaskProcessCommands(void* pTaskParam);

  //Must be public so that HTTP Source Filter can prevent posting commands
  //if HTTP streamer thread is not around
  bool IsHTTPStreamerRunning();
  bool Create();
  //Gets the http flavor from http downloader
  HTTPFlavor GetHTTPFlavor();
  bool IsLiveStreamingSession(bool& bLiveStreamingSession);
  bool GetTotalDuration(uint32 &duration);
  //HTTP controller command execution functions
  bool Open(const char* pURN,
            void* pPlaybackHandler,
            void* pUserData);
  bool Close(void* pUserData);
  bool Start(void* pUserData);
  bool Stop(void* pUserData);
  bool Play(void* pUserData);
  bool Pause(void* pUserData);
  bool Download(void* pUserData);
  bool Seek(const int64 seekTime,
            void* pUserData);
  bool GetTracks(void* pUserData);
  bool SetTrackState(const int trackIndex,
                     const bool bSelected,
                     void* pUserData);
  bool WaitForResources(void* pUserData);

  bool DataRequest(int32 portIdx,
                   void* pBuffHdr);
  bool DataFlush(bool bNotify,
                 int32 trackId,
                 void *pUserData);
  bool NotifyWaterMarkStatus(uint32 portIdxAndWatermarkType);
  bool SetAuthorization(const char *authKey, const char *authValue);

  void ExecuteOpen(URL *pUrn,
                   void* pPlaybackHandler,
                   void* pUserData);
  void ExecuteClose(void* pUserData);
  void ExecuteStart(void* pUserData);
  void ExecuteStop(void* pUserData);
  void ExecutePlay(void* pUserData);
  void ExecutePause(void* pUserData);
  void ExecuteDownload(void* pUserData);
  void ExecuteSeek(const int64 seekTime,
                   void* pUserData);
  void ExecuteGetTracks(void* pUserData);
  void ExecuteSetTrackState(const int32 trackIndex,
                            const bool bSelected,
                            void* pUserData);
  void ExecuteWaitForResources(void* pUserData);
  void ExecuteNotifyWaterMarkStatus(uint32 portIdxAndWatermarkType);
  void ExecuteSetAuthorization(const char *authKey, const char *authValue);
  void ExecuteSelectRepresentations(const char *pSelectionsXML);

  //HTTP downloader callbacks
  static void ExecuteOpenCallback(HTTPDownloadStatus HTTPStatus,
                                   const HTTPControllerCbData& callbackData);
  static void ExecuteStartCallback(HTTPDownloadStatus HTTPStatus,
                                   const HTTPControllerCbData& callbackData);
  static void ExecuteCloseCallback(HTTPDownloadStatus HTTPStatus,
                                   const HTTPControllerCbData& callbackData);
  static void ExecuteDownloadCallback(HTTPDownloadStatus HTTPStatus,
                                      const HTTPControllerCbData& callbackData);
  static void ExecuteSeekCallback(HTTPDownloadStatus HTTPStatus,
                                  const HTTPControllerCbData& callbackData);
  static void ExecuteSelectRepresentationsCallback(HTTPDownloadStatus HTTPStatus,
                                                   const HTTPControllerCbData& callbackData);


  //HTTP properties
  void SetDataStorageOption(const DataStorageType dataStorageOption)
  {
    m_dataStorageOption = dataStorageOption;
  };
  bool GetDataStorageOption(DataStorageType& dataStorageOption)
  {
    dataStorageOption = m_dataStorageOption;
    return true;
  };

  void SetDataHeapStorageLimit(const int32 heapStorageLimit)
  {
    m_nHeapStorageLimit = heapStorageLimit;
  };

  int32 GetDataHeapStorageLimit() const{ return m_nHeapStorageLimit; };

  void SetDataUnitCancellationDisabled(bool bIsDisabled);
  bool IsDataUnitCancellationDisabled() const;

  void SetMaxSupportedRepBandwidth(uint32 maxBw);
  uint32 GetMaxSupportedRepBandwidth() const;

  void SetMaxSupportedASCValue(uint32 maxASCVal);
  uint32 GetMaxSupportedASCValue() const;

  void UseTsbForStartupLatencyImprovement();
  bool IsStartupLatencyImprovementEnabled() const;


  void SetAudioSwitchingEnabled(bool bIsEnabled);
  bool IsAudioSwitchingEnabled() const;

  // vendor Specific extensions support methods
  int32 GetProxyServer(char* ProxyServer,
                       size_t ProxyServerLen,
                       size_t& ProxyServerLenReq);

  bool SetProxyServer(const char* ProxyServer,
                       size_t ProxyServerLen);

  bool SetCookie (const char* url,  const char* cookie);

  bool GetCookies(const char *url,  const char *cookies, size_t &cookiesLen);

  bool SetNetworkProfile(uint32 networkprofileId);

  bool GetNetworkProfile(uint32 &pNetworkProfileId);

  bool SetNetworkInterface(int32 networkIfaceId);

  bool GetNetworkInterface(int32 &networkIfaceId);

  bool GetTotalbytesReceived(int64 &nBytesReceived);

  void SetInitialPreroll(const uint32 preroll);

  uint32 GetInitialPreroll();

  void SetHTTPRequestsLimit(const uint32 httpReqsLimit);

  uint32 GetHTTPRequestsLimit();

  uint32 GetHTTPSegInfoGetRequestLength();

  void SetRebufferPreroll(const uint32 preroll);

  uint32 GetRebufferPreroll();

  bool SetOemHttpHeaders(IPStreamProtocolHeaderCommand whatToDo,
                         uint32 whichMethodsAffected,
                         const char *headerName,
                         const char *headerValue);
  bool GetOemHttpHeaders(uint32 whichMethodsAffected,
                         const char *headerName,
                         char *headerValue,
                         int& headerValueSize);

  bool GetDownloadProgress(HTTPMediaType mediaType,
                           uint32& currStartOffset,
                           uint32& downloadOffset,
                           HTTPDownloadProgressUnitsType eUnitsType,
                           bool& bEOS);

  void NotifyProtocolEvent();

  void SetAuthHandlingDiscarded();

  void GetDataInterface(HTTPDataInterface *&pDataInterface);
  bool GetMediaProperties(char *pPropertiesStr,
                          uint32 &nPropertiesLen);

   bool SelectRepresentations(const char* SetSelectionsXML);

  bool GetMPDText(char *pMPDTextStr, uint32 &mpdSize);

  void GetQOEData(uint32 &bandwidth, char *pVideoURL, size_t& nURLSize, char *pIpAddr, size_t& nIPAddrSize);

  uint32 GetMaxSupportedVideoBufferSize();

private:
  enum State
  {
    IDLE,
    CONNECTING,
    CONNECTED,
    DOWNLOADING,
    DOWNLOAD_DONE,
    CLOSING,
    STATE_MAX
  };
  void Reset();
  void ResetSession();
  void SetHTTPStreamerRunning(bool bHTTPStreamerRunning);

  bool StartThread();
  void ReleaseThread();

  void DestroySession(HTTPDownloadStatus HTTPStatus,
                      void* pUserData);

  bool AddSchedulerTask(SchedulerTask pTask,
                        void* pTaskParam);
  bool DeleteSchedulerTask(const int taskID);

  void SetState(const State newState);
  State GetState() const{ return m_state; };
  bool IsStateGood2ExecuteCmd(const HTTPControllerCommand cmd, bool& bNotify);

  void NotifyHTTPEvent(const HTTPControllerCommand HTTPCommand,
                       const HTTPDownloadStatus HTTPStatus,
                       void* pCbData);

  iHTTPPlaybackHandler* GetPlaybackHandler();

  //HTTP controller state
  State m_state;
  static const bool CommandStateMatrix[HTTPCommon::CMD_MAX][STATE_MAX][2];

  HTTPDownloader* m_pDownloader;

  bool m_bHTTPStreamerRunning;

  //Thread control block
  MM_HANDLE m_pHTTPStreamerThread;

  //Lock to synchronize HTTPController internal variable access
  //from Source Filter thread
  MM_HANDLE m_pHTTPStreamerCS;

  //HTTPController command queue
  HTTPCtrlCmdQueue m_ctrlCmdQueue;

  //Base task class for all HTTP dynamic scheduler tasks
  class HTTPControllerCmdTaskParam : public SchedulerTaskParamBase
  {
  public:
    HTTPControllerCmdTaskParam() : pCbSelf(NULL){ };
    virtual ~HTTPControllerCmdTaskParam(){ };

    void* pCbSelf;
  };

  //HTTP source notification handler
  iHTTPNotificationHandler* m_pHTTPSourceNotificationHandler;

  HTTPStatusHandlerInterface *m_pStackNotificationHandler;

  //HTTP properties
  DataStorageType m_dataStorageOption;
  int32 m_nHeapStorageLimit;
  uint32 m_nHttpRequestsLimit;

  // Queue of httpstacks which are expecting a SetHeader on 'Authorization'
  // or 'Proxy-Authorization' header.
  StreamQ_type m_PendingAuthQueue;

  class HTTPStackNotificationCbDataForAuth
  {
  public:
    HTTPStackNotificationCbDataForAuth();
    ~HTTPStackNotificationCbDataForAuth();

    int32 m_serverCode;
    char *m_serverMessage;
    char *m_entityBody;
    bool m_msgType;
    char *m_method;
    char *m_protocolHeaders; // it will contain the complete authentication header.
    HTTPAuthorizationInterface *m_pHTTPStack;

  private:
    HTTPStackNotificationCbDataForAuth(const HTTPStackNotificationCbDataForAuth&);
    HTTPStackNotificationCbDataForAuth& operator=(const HTTPStackNotificationCbDataForAuth&);
  };

  typedef struct
  {
    StreamQ_link_type link;
    HTTPStackNotifyCode m_HTTPNotifyCode; // passed to httpmmihelper on notification
    HTTPStackNotificationCbDataForAuth m_HTTPStackNotificationData;

  } PendingAuthElem;
};

}/* namespace video */
#endif /* __HTTPCONTROLLER_H__ */
