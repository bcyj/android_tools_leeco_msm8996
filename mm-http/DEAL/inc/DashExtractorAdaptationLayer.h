/************************************************************************* */
/**
 * DashExtractorAdaptationLayer.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

#ifndef DASH_EXTRACTOR_ADAPTATION_H_

#define DASH_EXTRACTOR_ADAPTATION_H_

/* =======================================================================
**               Include files for DashExtractorAdaptationLayer.h
** ======================================================================= */

#include <utils/RefBase.h>
#include "AEEStdDef.h"
#include "OMX_Types.h"
#include "OMX_Core.h"
#include "MMCriticalSection.h"
#include "mmiDeviceApi.h"
#include "HTTPSourceMMIEntry.h" // !Warning
#include "qtv_msg.h"
#include "QCMetaData.h"
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaSource.h>
#include "ExtractorCryptoAdaptationLayer.h"

#include <drm/DrmManagerClient.h>
#include "common_log.h"
#include "MMDebugMsg.h"
#include "DashMediaSource.h"



namespace android {

  /* =======================================================================
  **                      Data Declarations
  ** ======================================================================= */

  /* -----------------------------------------------------------------------
  ** Type Declarations
  ** ----------------------------------------------------------------------- */
  typedef void (*NotifyCallback)(int evntCode, int evntData, void *cbData, const Parcel *obj);

  /* -----------------------------------------------------------------------
  ** Constant / Macro Definitions
  ** ----------------------------------------------------------------------- */

#define IS_SUCCESS(x) ((MMI_S_COMPLETE == (x)) || (MMI_S_PENDING == (x)))
#define IS_VIDEO_PORT(x) ((x)==MMI_HTTP_VIDEO_PORT_INDEX)
#define IS_AUDIO_PORT(x) ((x)==MMI_HTTP_AUDIO_PORT_INDEX)
#define IS_TEXT_PORT(x)  ((x)==MMI_HTTP_OTHER_PORT_INDEX)

  //max num supported tracks
#define MAX_NUM_TRACKS 3
#define APPROX_CONFIG_HDR_SIZE 128

#define DEAL_PREPARE_RSP             0x0001
#define DEAL_START_RSP               0x0002
#define DEAL_SEEK_RSP                0x0003
#define DEAL_PAUSE_RSP               0x0004
#define DEAL_RESUME_RSP              0x0005
#define DEAL_STOP_RSP                0x0006
#define DEAL_FORMAT_CHANGE           0x0007
#define DEAL_BUFFERING_START         0x0008
#define DEAL_BUFFERING_END           0x0009
#define DEAL_UNKNOWN_ERROR           0x8000

  //QoE notify keys
#define kWhatQOE                     0x0100
#define kWhatQOEPlay                 0x0101
#define kWhatQOEStop                 0x0102
#define kWhatQOESwitch               0x0103
#define kWhatQOEPeriodic             0x0104

  //Keys to be used in setparam and get param function call
#define KEY_DASH_ADAPTION_PROPERTIES 0x0200
#define KEY_DASH_MPD_QUERY           0x0201
#define KEY_DASH_QOE_EVENT           0x0202
#define KEY_DASH_QOE_PERIODIC_EVENT  0x0203
#define KEY_DASH_PARAM_PREROLL       0x0204
#define KEY_DASH_PARAM_HWM           0x0205
#define KEY_DASH_PARAM_LWM           0x0206
#define KEY_DASH_PARAM_BUF_DURATION  0x0207

  /* =======================================================================
  **                        Class & Function Definitions
  ** ======================================================================= */

  class MetaData;

  class  DEALInterface : public RefBase{

  public:
    class DEALStateBase;
    DEALInterface(bool& bOk);

    void RegisterCallBack(NotifyCallback cb, void *cbdata);

    size_t countTracks();
    sp<MediaSource> getTrack(size_t index);
    sp<MetaData> getTrackMetaData(size_t index, uint32_t flags = 0);

    bool setParameter(int nParamId, void* data, size_t size);
    bool getParameter(int nParamId, void* data, size_t *size);

    /* Custom APIs to be called from delegate layer to Adaptation */
    bool setDataSource(
      const char *path,
      uint32 pathLength,
      const KeyedVector<String8, String8> *headers = NULL);

    bool prepareAsync();
    bool start();
    bool seekTo(int64_t seekTimeUs);
    bool stop();
    bool pause();
    bool resume();

  public:

    // To be used into the Adaptation Layer
    enum DEALInterfaceState
    {
      DEALInterfaceStateIdle = 0 ,
      DEALInterfaceStateInit     ,
      DEALInterfaceStateLoading  ,
      DEALInterfaceStateLoaded   ,
      DEALInterfaceStateOpening  ,
      DEALInterfaceStatePlaying  ,
      DEALInterfaceStateSeeking  ,
      DEALInterfaceStatePausing  ,
      DEALInterfaceStatePaused   ,
      DEALInterfaceStateResuming ,
      DEALInterfaceStateClosing  ,
      DEALInterfaceStateMax
    };

    enum DEALCmd
    {
      DEAL_PREPARE ,
      DEAL_START   ,
      DEAL_SEEK    ,
      DEAL_PAUSE   ,
      DEAL_RESUME  ,
      DEAL_STOP    ,
    };

    enum DashStreamType
    {
      DASH_STREAM_TYPE_VOD,
      DASH_STREAM_TYPE_LIVE
    };

    enum DASHSourceTrackType
    {
      eTrackAudio = 0,
      eTrackVideo    ,
      eTrackText     ,
      eTrackAll      ,
      eTrackNone     ,
    };

    class InBufferQItem : public RefBase
    {
    public:
      sp<ABuffer> mBuffer;
      OMX_BUFFERHEADERTYPE mBufHdr;
      InBufferQItem()
      {
        mBuffer = NULL;
        QOMX_STRUCT_INIT(mBufHdr,OMX_BUFFERHEADERTYPE);
      }
      ~InBufferQItem() {;}
    };


    class InBufferQ : public RefBase
    {
      Mutex mInBufQLock;
      List<sp<DEALInterface::InBufferQItem> > mBuffers;
      int32_t count;

    public:
      InBufferQ();
      ~InBufferQ();
      void enqueue(sp<DEALInterface::InBufferQItem> buf); //queues at the end
      status_t dequeue(sp<DEALInterface::InBufferQItem> *buf); // dequeues from the begining
      uint32 getQueueSize();
    };


    class TrackInfo
    {
      public:

      TrackInfo()
      {
        bIsEncrypted = false;
      }
      ~TrackInfo()
      {
        if(m_metaData != NULL)
        {
          m_metaData = NULL;
        }
        if(mInBufferQ != NULL)
        {
          mInBufferQ = NULL;
        }
      };


      sp<MetaData>* setMetaData()
      {
        return &m_metaData;
      }

      sp<MetaData> getMetaData()
      {
        return m_metaData;
      }


      void createInBufferQ()
      {
        if(mInBufferQ == NULL)
        {
          mInBufferQ = new InBufferQ;
        }
      }

      sp<DEALInterface::InBufferQ> getInBufferQ()
      {
        return mInBufferQ;
      }

      void createOutBufferQ(const sp<DEALInterface> &pDEALDataCache)
      {
        if(mOutBufferQ == NULL)
        {
          mOutBufferQ = new DashMediaSource(m_nPortIndex, m_nTrackId, pDEALDataCache, m_metaData);
        }
      }

      sp<DashMediaSource> getOutBufferQ()
      {
        return mOutBufferQ;
      }

      void setMaxBufferCount(int32_t nMaxBufferCount)
      {
        m_nMaxBufferCount = nMaxBufferCount;
      }

      uint32 getMaxBufferCount()
      {
        return m_nMaxBufferCount;
      }

      void setMaxBufferSize(uint32_t nMaxBufferSize)
      {
        m_nMaxBufferSize = nMaxBufferSize;
      }

      size_t getMaxBufferSize()
      {
        return m_nMaxBufferSize;
      }

      void SetEncrypted(bool encStatus)
      {
        bIsEncrypted = encStatus;
      }

      bool IsEncrypted()
      {
        return bIsEncrypted;
      }

      int32_t m_nTrackId;
      int32_t m_nPortIndex;

    private:
      sp<DEALInterface::InBufferQ> mInBufferQ;
      sp<DashMediaSource> mOutBufferQ;
      int32_t m_nMaxBufferCount;
      size_t m_nMaxBufferSize;
      sp<MetaData> m_metaData;
      bool bIsEncrypted;
    };


    //public functions used in DEAL layer
    bool InitializeDEALStates();

    OMX_HANDLETYPE GetMMIHandle();
    void SetMMIHandle(OMX_HANDLETYPE handle);

    DEALStateBase *GetCurrentDEALState();
    bool SetDEALState(DEALInterfaceState eDEALState);

    bool ProcessCmd(DEALCmd cmd, int32 arg1 = 0,
      int32 arg2 = 0, int64 arg3 = 0);

    void flush();

    status_t initialize();
    OMX_U32 OpenMMI();
    OMX_U32 GetOmxIndexByExtensionString(MMI_GetExtensionCmdType &nMMICmdType);
    OMX_U32 postMMICmd(OMX_HANDLETYPE handle, OMX_U32 nCode, void *pData);
    DashStreamType GetStreamType();
    void SetStreamType(DashStreamType lStreamType);
    bool isTrackDiscontinuity(uint32 trackId);
    void ResetDiscontinuity(uint32 trackId, bool resetTo);
    bool ReadFrameAsync(uint32 nPortIndex);
  status_t DoDecyrpt(sp<ABuffer> buffer, int Fd, uint32 portIndex);
  void GetIndexForDRMExtensions();
  void SignalEOSonError(status_t result);
#if DRM_DEBUG_CONTENT_PRINT
     void printBuf(const char *buffer, int bufLength);
     void PrintExtraSampleInfo( QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo);
     void PrintPSSH( QOMX_PARAM_STREAMING_PSSHINFO *psshInfo);
     void PrintSubSampleInfo(int subSampleCount,
                             QOMX_ENCRYPTED_SUBSAMPLE_INFO *encSubsampleInfo);
#endif
    //bool ProcessFrameNotify(sp<DEALInterface::InBufferQItem> pInBufferQItem, uint32 portIndex, OMX_U32 nEvtStatus);
    bool ProcessFrameNotify(sp<ABuffer> mABuffer, MMI_BufferCmdType *pMMICmdBuf, uint32 nPortIndex, OMX_U32 nEvtStatus);
    void QueryAndUpdateMetaData(uint32 nPortIndex);
    status_t postSeek(int64 timeUs);

    void setFlushPending(bool value)
    {
      MM_CriticalSection_Enter(m_hDEALFlushLock);
      m_bFlushPending = value;
      MM_CriticalSection_Leave(m_hDEALFlushLock);
    }

    bool isFlushPending()
    {
      bool value = false;
      MM_CriticalSection_Enter(m_hDEALFlushLock);
      value = m_bFlushPending;
      MM_CriticalSection_Leave(m_hDEALFlushLock);
      return value;
    }

    void incFlushCount()
    {
      MM_CriticalSection_Enter(m_hDEALFlushLock);
      n_flushCount = n_flushCount + 1;
      MM_CriticalSection_Leave(m_hDEALFlushLock);
    }

    int getFlushCount()
    {
      int value = 0;
      MM_CriticalSection_Enter(m_hDEALFlushLock);
      value = n_flushCount;
      MM_CriticalSection_Leave(m_hDEALFlushLock);
      return value;
    }

    void resetFlushCount()
    {
      MM_CriticalSection_Enter(m_hDEALFlushLock);
      n_flushCount = 0;
      MM_CriticalSection_Leave(m_hDEALFlushLock);
    }

    void storeSeekValue(int64 value)
    {
      MM_CriticalSection_Enter(m_hDEALSeekLock);
      cachedSeekValue = value;
      MM_CriticalSection_Leave(m_hDEALSeekLock);
    }

    int64 getSeekValue()
    {
      int64 value = -1;
      MM_CriticalSection_Enter(m_hDEALSeekLock);
      value = cachedSeekValue;
      MM_CriticalSection_Leave(m_hDEALSeekLock);
      return value;
    }

    void resetSeek()
    {
      MM_CriticalSection_Enter(m_hDEALSeekLock);
      cachedSeekValue = (int64)-1;
      MM_CriticalSection_Leave(m_hDEALSeekLock);
    }


    NotifyCallback notify;
    void *cbData;

    OMX_INDEXTYPE mSMTPTimedTextDimensionsIndex;
    OMX_INDEXTYPE mSMTPTimedTextSubInfoIndex;
    OMX_INDEXTYPE mWatermarkExtIndex;
    OMX_INDEXTYPE mWatermarkStatusExtIndex;
    OMX_INDEXTYPE mBuffDurationExtIndex;
    OMX_INDEXTYPE  mIndexDrmExtraSampleInfo;
    OMX_INDEXTYPE  mIndexDrmParamPsshInfo;

    TrackInfo mTrackList[MAX_NUM_TRACKS];

    MM_HANDLE m_hDEALTrackInfoReadLock[MAX_NUM_TRACKS];

    //lock for handling synchronization among
    //sources in context of mmi calls
    MM_HANDLE m_hDEALStateLock;
    //MM_HANDLE m_hDEALInQReadLock[MAX_NUM_TRACKS];
    //MM_HANDLE m_hDEALOutQReadLock[MAX_NUM_TRACKS];

    static const uint32 BITRATE = 512000;


protected:

    virtual ~DEALInterface();
    static void EventHandler(OMX_IN OMX_U32 nEvtCode,
      OMX_IN OMX_U32 nEvtStatus,
      OMX_IN OMX_U32 nPayloadLen,
      OMX_IN OMX_PTR pEvtData,
      OMX_IN OMX_PTR pClientData);

  private:

    char* m_uri;
    KeyedVector<String8, String8> mUriHeaders;

    //http mmi handle
    OMX_HANDLETYPE m_handle;

    /* DEAL States */
    DEALStateBase *m_pDEALStates[DEALInterfaceStateMax];

    /* Active DEAL State */
    DEALStateBase *m_pCurrentDEALState;

    DashStreamType eStreamType;

    bool m_bAudioDiscontinuity;
    bool m_bVideoDiscontinuity;
    bool m_bTextDiscontinuity;

    bool m_bFlushPending;
    int  n_flushCount;

    // lock while process flush and flush count value
    MM_HANDLE m_hDEALFlushLock;

    // lock while process cached seek
    MM_HANDLE m_hDEALSeekLock;

    int64 cachedSeekValue;

    sp<ECALInterface> mECALInterface;


  public:

    class DEALStateBase
    {
    public:
      DEALStateBase(DEALInterface      &pDEALDataCache,
        DEALInterfaceState eDEALState);
      virtual ~DEALStateBase(void);

    public:

      DEALInterfaceState GetState()
      {
        return m_eDEALState;
      }

      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);

      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);

      virtual bool EntryHandler();
      virtual bool ExitHandler();

    protected:

      DEALInterface &m_pMMHTTPDataCache;
      DEALInterfaceState m_eDEALState;
    };

    class DEALStateIdle: public DEALStateBase
    {
    public:
      DEALStateIdle(DEALInterface &pMMHTTPDataCache);
      virtual ~DEALStateIdle(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
    };

    class DEALStateInit: public DEALStateBase
    {
    public:
      DEALStateInit(DEALInterface &pMMHTTPDataCache);
      virtual ~DEALStateInit(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
    };

    class DEALStateLoading: public DEALStateBase
    {
    public:
      DEALStateLoading(DEALInterface &pMMHTTPDataCache);
      virtual ~DEALStateLoading(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
    };

    class DEALStateLoaded: public DEALStateBase
    {
    public:
      DEALStateLoaded(DEALInterface &pMMHTTPDataCache);
      virtual ~DEALStateLoaded(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
    };

    class DEALStateOpening: public DEALStateBase
    {
    public:
      DEALStateOpening(DEALInterface &pMMHTTPInterface);
      virtual ~DEALStateOpening(void);

    private:
      uint32 m_nNumTracksReady;

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
    };

    class DEALStatePlaying: public DEALStateBase
    {
    public:
      DEALStatePlaying(DEALInterface &pMMHTTPDataCache);
      virtual ~DEALStatePlaying(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
    };

    class DEALStateClosing: public DEALStateBase
    {
    public:
      DEALStateClosing(DEALInterface &pMMHTTPInterface);
      virtual ~DEALStateClosing(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
      virtual bool EntryHandler();
    };

    class DEALStateSeeking: public DEALStateBase
    {
    public:
      DEALStateSeeking(DEALInterface &pMMHTTPDataCache);
      virtual ~DEALStateSeeking(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
      virtual bool EntryHandler();
    };

    class DEALStatePausing: public DEALStateBase
    {
    public:
      DEALStatePausing(DEALInterface &pMMHTTPInterface);
      virtual ~DEALStatePausing(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
      virtual bool EntryHandler();
    };

    class DEALStatePaused: public DEALStateBase
    {
    public:
      DEALStatePaused(DEALInterface &pMMHTTPInterface);
      virtual ~DEALStatePaused(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
    };

    class DEALStateResuming: public DEALStateBase
    {
    public:
      DEALStateResuming(DEALInterface &pMMHTTPInterface);
      virtual ~DEALStateResuming(void);

    public:
      virtual uint32 ProcessCmd(DEALInterface::DEALCmd cmd,
        int32 arg1, int32 arg2, int64 arg3);
      void EventHandler(OMX_IN OMX_U32 nEvtCode,
        OMX_IN OMX_U32 nEvtStatus,
        OMX_IN OMX_U32 nPayloadLen,
        OMX_IN OMX_PTR pEvtData);
      virtual bool EntryHandler();
    };
  };

}  // namespace android

#endif  // DASH_EXTRACTOR_ADAPTATION_H_
