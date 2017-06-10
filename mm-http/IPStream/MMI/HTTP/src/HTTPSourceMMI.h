#ifndef __HTTPSOURCEMMI_H__
#define __HTTPSOURCEMMI_H__
/************************************************************************* */
/**
 * HTTPSourceMMI.h
 * @brief Header file for HTTPSourceMMI.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMI.h#25 $
$DateTime: 2013/05/12 09:54:28 $
$Change: 3751774 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMI.h
** ======================================================================= */
#include "httpInternalDefs.h"
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Index.h>
#include <OMX_Video.h>
#include <OMX_Audio.h>
#include <OMX_Image.h>
#include <OMX_Other.h>
#include <OMX_IVCommon.h>
#include "HTTPDataRequestHandler.h"

#include  "mmiDeviceApi.h"
#include <HTTPAPI.h>
#include <HTTPSource.h>
#include <HTTPStackInterface.h>
#include <oscl_string.h>
#include "HTTPController.h"
#include "HTTPSourceMMIEntry.h"
#include "HTTPSourceMMITrackHandler.h"
#include "HTTPSourceMMIExtensionHandler.h"
#include "StreamDataQueue.h"
#include "MMTimer.h"

#define MAX_STOP_PHRASE_LENGTH 101 //Null adjusted 100 + 1
#define STOP_ERROR_STRING "UNDEFINED ERROR"
#define STOP_STRING "NORMAL STOP"
namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPSourceMMIHelper;
class HTTPSourceMMIPropertiesHandler;
class HTTPSourceMMIStreamPortHandler;
class HTTPSourceMMITrackHandler;
class HTTPController;
class HTTPSourceMMIExtensionHandler;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

/**
@brief   HTTPSourceMMI

@detail This module gives a standard interface to OMX  HTTPSourceMMI component  to access
the Qualcomm HTTP Source implementation.
*/
class HTTPSourceMMI
{
public:
  //MMI entry methods
  static OMX_U32 HTTPSourceMMIOpen(OMX_HANDLETYPE  *pHTTPSourceHandle);
  static OMX_U32 HTTPSourceMMIClose(OMX_HANDLETYPE pHTTPSourceHandle);
  static OMX_U32 HTTPSourceMMICmd(OMX_HANDLETYPE hHTTPSourceHandle,
                                  OMX_U32 nCode, OMX_PTR pData);
  static OMX_U32 HTTPSourceMMIRegisterEventHandler(OMX_HANDLETYPE hHTTPSourceHandle,
                                                   MMI_CmpntEvtHandlerType pfnEvtHdlr,
                                                   void *pClientData);

public:
  //constructor and destructors
  HTTPSourceMMI();
  ~HTTPSourceMMI();

  friend class HTTPSourceMMIHelper;
  friend class HTTPSourceMMIExtensionHandler;

private:
  class HttpSourceMmiPortInfo
  {
  public:
    //Contain port definition of this port
    OMX_PARAM_PORTDEFINITIONTYPE m_portDef;

    //contains private data
    OMX_PTR m_pPortSpecificData;

    //Contains track-id associated with this port
    OMX_U32 m_trackId;

    //True if track-id valid else is set to FALSE
    OMX_BOOL m_isTrackValid;

    //True if switch (validation) is in progress
    bool m_bSwitching;

    // Buffer flags
    OMX_U32 m_nBufferFlags;
  };

private:
  //MMI play back control methods
  OMX_U32 Open(void *pUserData);
  OMX_U32 Close(void *pData = NULL);
  OMX_U32 Start(void *pUserData);
  OMX_U32 Stop(void *pUserData);
  OMX_U32 Play(void *pUserData);
  OMX_U32 Pause(void *pUserData);
  OMX_U32 Seek(const int64 timeToSeek, void* pData);

  //mmi command functionality methods
  OMX_U32 SetParam(void* pUserData);
  OMX_U32 SetCustomParam(void* pUserData);
  OMX_U32 GetParam(void* pUserData);
  OMX_U32 GetCustomParam(void* pUserData);
  OMX_U32 FillThisBuffer (void*);
  OMX_U32 AllocBuffer(void* pUserData);
  OMX_U32 FreeBuffer(void* pUserData);
  OMX_U32 Flush(void* pUserData = NULL);
  OMX_U32 SetPortToAutoDetect(OMX_U32 nPortIndex);
  OMX_U32 SelectStream(OMX_U32 nPortIndex, OMX_U32 nStreamNo);
  OMX_U32 LoadResources(void *pUserData);
  OMX_U32 ReleaseResources(void);
  OMX_U32 WaitForResources(void *);
  OMX_U32 ReleaseWaitForResources(void);

  //mmi port realted auxiliary methods
  void InitPorts();
  void UpdatePorts(bool bSendPortEvents=true);
  HTTPDownloadStatus HandleSwitch(const HTTPMediaType mediaType, bool &isPortConfigEventPending);
  void UpdateAudioPort(OMX_U32 PortIndex,
                       HTTPSourceMMITrackHandler::TrackDescription &,
                       bool &bSendPortEvents);
  void InvalidateAudioPort( OMX_U32 PortIndex, bool &bSendPortEvents);
#ifdef FEATURE_HTTP_AMR
  void UpdateAmrAudioPort(OMX_U32 audioPortIndex,
                          QOMX_AUDIO_PARAM_AMRWBPLUSTYPE* pAmrParam,
                          MediaTrackInfo &trackInfo);
#endif // FEATURE_HTTP_AMR
  void UpdateAacAudioPort(OMX_U32 audioPortIndex,
                          OMX_AUDIO_PARAM_AACPROFILETYPE* pAacParam,
                          HTTPMediaTrackInfo &trackInfo);
  void UpdateAc3AudioPort(OMX_U32 audioPortIndex,
                          QOMX_AUDIO_PARAM_AC3PROFILETYPE* pAc3Param,
                          HTTPMediaTrackInfo &trackInfo);
  void UpdateMp2AudioPort(OMX_U32 audioPortIndex,
                          QOMX_AUDIO_PARAM_MP2PROFILETYPE* pMp2Param,
                          HTTPMediaTrackInfo &trackInfo);
#ifdef FEATURE_HTTP_WM
  void UpdateWmvVideoPort(OMX_U32 videoPortIndex,
                          OMX_VIDEO_PARAM_WMVTYPE* pWmvParam,
                          MediaTrackInfo &trackInfo);
#endif
  void UpdateVideoPort(OMX_U32 , HTTPSourceMMITrackHandler::TrackDescription &,
                       bool bSendPortEvents, FileSource *pFileSource);
  void UpdateVideoPort(OMX_U32 , HTTPSourceMMITrackHandler::TrackDescription &,
                       bool &bSendPortEvents);
  void InvalidateVideoPort( OMX_U32 PortIndex, bool &bSendPortEvents);
  void UpdateOtherPort(OMX_U32 , HTTPSourceMMITrackHandler::TrackDescription &,
                       bool &bSendPortEvents);
  void InvalidateOtherPort( OMX_U32 PortIndex, bool &bSendPortEvents);
  void CheckAndSendPortConfigChangeEvent(HTTPCommon::HTTPMediaType mediaType,
                                         OMX_U32 portIndex,
                                         bool isSendPortConfig);
  void FlushPort(HttpSourceMmiPortInfo *pPortInfo);
  bool IsValidPort(OMX_U32 index, OMX_PORTDOMAINTYPE domain = OMX_PortDomainMax);
  bool IsEndOfStream(int32 portIdx);
  OMX_U32 ComparePortDefinition(MMI_ParamDomainDefType *pDomainDef);
  HTTPDownloadStatus CompareAndUpdatePorts(bool &isAudioPortConfigPending,
                                           bool &isVideoPortConfigPending,
                                           bool &isOtherPortConfigPending);

  void GetQOEData(uint32& nRebufCount,char* pStopPhrase,size_t& nStopPhraseSize)
  {
    nRebufCount = m_nRebufCount;
    if(m_pStopPhrase)
    {
      nStopPhraseSize = std_strlen(m_pStopPhrase);
      if(pStopPhrase)
      {
        std_strlcpy(pStopPhrase,m_pStopPhrase,std_strlen(m_pStopPhrase)+1);
      }
    }else
    {
      nStopPhraseSize = 0;
    }
  };

  /* Update Buffering count */
  void UpdateRebufCount(int nCount)
  {
    m_nRebufCount+=nCount;
  };

  /* Update Stop reason based on input phrase */
  void UpdateStopPhrase(char* pStopPhrase)
  {
    /* If stop phrase has memory and storage string is less than max size */
    if(m_pStopPhrase && (std_strlen(pStopPhrase) < MAX_STOP_PHRASE_LENGTH))
    {
      std_strlcpy(m_pStopPhrase,pStopPhrase,std_strlen(pStopPhrase)+1);
    }
    else
    {
      if(m_pStopPhrase) /* If stop phrase is allocated memory*/
      {
        QTV_Free(m_pStopPhrase);
      }
      /* Reallocate as per requirement*/
      m_pStopPhrase = (char*)QTV_Malloc(std_strlen(pStopPhrase)+1);
      if(m_pStopPhrase)
      {
        std_strlcpy(m_pStopPhrase,pStopPhrase,std_strlen(pStopPhrase)+1);
      }
      else
      {
        m_pStopPhrase = NULL;
      }
    }
  };

  OMX_AUDIO_CODINGTYPE GetOmxAudioMinorType(FileSourceMnMediaType minorType)
  {
    return m_pHTTPSourceMMITrackHandler ?
           m_pHTTPSourceMMITrackHandler->GetOmxAudioMinorType(minorType) : (OMX_AUDIO_CODINGTYPE)0;
  };

  OMX_VIDEO_CODINGTYPE GetOmxVideoMinorType(FileSourceMnMediaType minorType)
  {
    return m_pHTTPSourceMMITrackHandler ?
           m_pHTTPSourceMMITrackHandler->GetOmxVideoMinorType(minorType) : (OMX_VIDEO_CODINGTYPE)0;
  };

  QOMX_OTHER_CODINGTYPE GetOmxOtherMinorType(FileSourceMnMediaType minorType)
  {
    return m_pHTTPSourceMMITrackHandler ?
           m_pHTTPSourceMMITrackHandler->GetOmxOtherMinorType(minorType) : (QOMX_OTHER_CODINGTYPE)QOMX_OTHER_CodingUnused;
  };

  uint32 GetMaxSupportedVideoBufferSize()
  {
    return (m_pHTTPController ?
               m_pHTTPController->GetMaxSupportedVideoBufferSize() : 0);
  }
private:
  //auxiliary methods
  bool Create();
  void Destroy();
  bool InitializeHTTPStreamer();
  void UninitializeHTTPStreamer();
  bool InitOpen();
  bool OpenSession(const char* pURL);
  void SetClosePending(const bool bClosePending);
  bool IsClosePending() const;
  void SetShutDownInProgress(const bool bIsShutDownInProgress);
  bool IsShutDownInProgress() const;
  void SetHTTPStreamerInitialized(const bool bInitialized);
  bool IsHTTPStreamerInitialized() const;
  bool IsPortSetToAutoDetect();

  //seek auxiliary methods
  void SetSeekPending(const bool bSeekPending);
  bool IsSeekPending();
  HTTPDownloadStatus IsRepositioningAllowed(const int64 timeToSeek);
  bool StartDataRequestProcessing();


  void SetPortSwitchingFlag(HttpSourceMmiPortInfo* pPortInfo,const bool bPortInfoAvailable);
  bool IsPortSwitching(HttpSourceMmiPortInfo* pPortInfo);
  void SetOpenComplete(const bool bOpenCompleted)
  {
    (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
    m_bOpenCompleted = bOpenCompleted;
    (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);
  }
  bool IsOpenComplete() const
  {
    bool bOpenCompleted = false;
    (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
    bOpenCompleted = m_bOpenCompleted;
    (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);
    return bOpenCompleted;
  }
  void SetDataInterface(HTTPDataInterface *pHTTPDataInterface)
  {
    m_pHTTPDataInterface = pHTTPDataInterface;
  }

  //callback fn of underrun task
  static int TryRead(void* pTaskParam);

  // SetURL
  void SetURL(const char*);

  //set start time buffer flags on all ports for seek or
  //in case of resume in live stream
  void SetStartTimeBufferFlag();
  void SetEOSBufferFlag();
  void ClearEOSBufferFlag();

  //Read data for track
  OMX_U32 GetSample(int32 portIdx, OMX_BUFFERHEADERTYPE *pBuffHdr);

  void SetSelectRepresentationsPending(bool flag);
  bool IsSelectRepresentationsPending() const;
  bool CacheSelectedRepresentations(const char *str);
  const char *GetCachedSelectedRepresentations() const;
  void ClearCachedSelectRepresentions();

  //controller interface
  HTTPController* GetController();

private:
  HTTPSourceMMIHelper* m_pHTTPSourceMMIHelper;
  HTTPSourceMMIPropertiesHandler* m_pHTTPSourceMMIPropertiesHandler;
  HTTPSourceMMIStreamPortHandler* m_pHTTPSourceMMIStreamPortHandler;
  HTTPSourceMMITrackHandler* m_pHTTPSourceMMITrackHandler;
  HTTPDataRequestHandler* m_pHTTPDataReqHandler;

  //port data structures for audio, video, etc
  HttpSourceMmiPortInfo m_portVideo[MMI_HTTP_NUM_VIDEO_PORTS];
  HttpSourceMmiPortInfo m_portAudio[MMI_HTTP_NUM_AUDIO_PORTS];
  HttpSourceMmiPortInfo m_portImage[MMI_HTTP_NUM_IMAGE_PORTS];
  HttpSourceMmiPortInfo m_portOther[MMI_HTTP_NUM_OTHER_PORTS];

  //mmi notification callback
  MMI_CmpntEvtHandlerType m_HTTPSourceMMIAsync_CB;

  //mmi cleint data
  void* m_pClientData;

  //url
  OSCL_STRING* m_pURL;

  //role of our component
  OSCL_STRING* m_pRole;

  //http controller
  HTTPController* m_pHTTPController;
  //http data interface
  HTTPDataInterface* m_pHTTPDataInterface;

  //Critical section used to synchronize access to MMI data
  MM_HANDLE m_pHTTPSourceMMIDataLock;

  // HTTP OEM extension handler
  HTTPSourceMMIExtensionHandler m_pHTTPSourceMMIExtensionHandler;

  //seek data structres.
  int64 m_nCurrentSeekTime;
  int64 m_nPendingSeekTime;
  bool m_bSeekPending;
  int32 m_seekMode;
  uint32 m_nRebufCount;
  char* m_pStopPhrase;
  //streamer control flags
  bool m_bHTTPStreamerInitialized;
  bool m_bClosePending;
  bool m_bIsShutdownInProgress;
  bool m_bOpenCompleted;

  bool m_bIsSelectRepresentationsPending;
  char *m_pCachedSelectedRepresentations;
};

}/* namespace video */

#endif /* __HTTPSOURCEMMI_H__ */
