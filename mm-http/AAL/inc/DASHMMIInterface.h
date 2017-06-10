/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
/* =======================================================================
**               Include files for DASHMMIInterface.h
** ======================================================================= */
#ifndef DASH_MMI_INTERFACE_H
#define DASH_MMI_INTERFACE_H

#include <utils/RefBase.h>

#include "AEEStdDef.h"
#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>
#include <utils/List.h>
#include "MMSignal.h"
#include "OMX_Types.h"
#include "mmiDeviceApi.h"
#include "HTTPSourceMMIEntry.h" // !Warning
#include "DASHMMIMediaSource.h"

#include <DashPlayer.h>
#include <DashPlayerSource.h>

#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MetaData.h>

#include <drm/DrmManagerClient.h>
#include "QOMX_StreamingExtensionsPrivate.h"
#include "oscl_string_utils.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "AEEstd.h"


namespace android {

typedef void (*SourceNotifyCB)(sp<ABuffer> accessUnit, status_t nStatus);

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */
#define IS_SUCCESS(x) ((MMI_S_COMPLETE == (x)) || (MMI_S_PENDING == (x)))
#define MMI_WAIT_INTERVAL 60000
#define MMI_DATA_WAIT_INTERVAL 60000
#define DRM_CORRUPT_FRAME_COUNT_LIMIT 5
#define MAX_NUM_TRACK_INFO 10
/* -----------------------------------------------------------------------
** Type Declarations, forward declarations
** ----------------------------------------------------------------------- */
//forward declarations
class CHTTPAALStateBase;
//class DASHMMIMediaSource;
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
// HTTP MMI States
enum HTTPAALState
{
  HTTPAALStateIdle = 0,
  HTTPAALStateConnecting,
  HTTPAALStatePlaying,
  HTTPAALStatePausing,
  HTTPAALStatePaused,
  HTTPAALStateResuming,
  HTTPAALStateClosing,
  HTTPAALStateMax
};

enum HTTPAALAttributeKey
{
  //ToDo: Can add more attributes such as duration etc.
  HTTP_AAL_ATTR_METADATA_AVAILABLE
};
typedef union
{
  //ToDo: Can add more data types such as int etc.
  bool bBoolVal;
  //int nIntVal;
  //double nFloatVal;
  //char* pStrVal;
}HTTPAALAttributeValue;


class DASHMMIInterface  : public RefBase
{
public:
  DASHMMIInterface(const char *uri,
                   const KeyedVector<String8, String8> *headers,
                   OMX_U32 &nReturn);

protected:
  ~DASHMMIInterface();

  //data source methods


  OMX_U32 postCmd(OMX_HANDLETYPE handle, OMX_U32 nCode, void *pData);

/* HTTP AAL Commands */
public:
  enum HTTPAALCmd
  {
    AAL_PAUSE,
    AAL_RESUME,
    AAL_SEEK,
    AAL_STOP,
    AAL_PREPARE
  };

  enum HTTPAALSignalSet
  {
    AAL_INVALID_SIG = 0x00000000,
    AAL_WAKEUP_SIG =  0x00020000,
    AAL_VIDEO_SIG =   0x00040000,
    AAL_AUDIO_SIG =   0x00080000,
    AAL_ABORT_SIG =   0x00200000
  };

    enum DashStreamType {
        DASH_STREAM_TYPE_VOD,
        DASH_STREAM_TYPE_LIVE
    };

  // Keep MEDIA_TRACK_TYPE_* in sync with MediaPlayer.java.
  enum media_track_type {
    MEDIA_TRACK_TYPE_UNKNOWN = 0,
    MEDIA_TRACK_TYPE_VIDEO = 1,
    MEDIA_TRACK_TYPE_AUDIO = 2,
    MEDIA_TRACK_TYPE_TIMEDTEXT = 3,
    MEDIA_TRACK_TYPE_SUBTITLE = 4,
  };

  void incFlushCount()
  {
    mFlushCount++;
  }

  void resetFlushCount()
  {
    mFlushCount = 0;
  }

  int getFlushCount()
  {
    return mFlushCount;
  }

  DashPlayer::DASHHTTPLiveSource *mHttpLiveSrcObj;

  //public functions used in AAL layer
  CHTTPAALStateBase *GetCurrentHTTPAALState();
  bool SetHTTPAALState(HTTPAALState eHTTPAALState);
  status_t ProcessCmd(HTTPAALCmd cmd, int32 arg1 = 0,
                      int32 arg2 = 0, int64 arg3 = 0);
  uint32 Wait(uint32 signal, int time, int *timeout);
  int32 Signal(HTTPAALSignalSet signal);
  OMX_HANDLETYPE GetMMIHandle();
  void SetMMIHandle(OMX_HANDLETYPE handle);
  status_t MapMMIToAALStatus(uint32 nValue);

  bool isAudioDiscontinuity() {return bAudioDiscontinuity;}
  bool isVideoDiscontinuity() {return bVideoDiscontinuity;}
  void ResetDiscontinuity(bool audio, bool resetTo);

  void setHttpLiveSourceObj(DashPlayer::DASHHTTPLiveSource *Obj);
  void flush();
  status_t seekTo(int64_t seekTimeUs);
  uint32 ProcessSeek(int32_t seekMode, int64_t seekTimeUs);

  uint32 GetNumTracks();

  void setAudioSource(sp<DASHMMIMediaSource> mSourcePtr) { mAudioSource = mSourcePtr;}
  void setVideoSource(sp<DASHMMIMediaSource> mSourcePtr) { mVideoSource = mSourcePtr;}
  void setTextSource(sp<DASHMMIMediaSource> mSourcePtr) { mTextSource = mSourcePtr;}
  void resetAudioSource() { mAudioSource = NULL;}
  void resetVideoSource() { mVideoSource = NULL;}
  void resetTextSource() { mTextSource = NULL;}

  status_t ProcessFrameNotify(sp<ABuffer> mAbuffer, MMI_BufferCmdType *pMMICmdBuf,OMX_U32 nEvtStatus);
  status_t readFrameAsync(int audio, sp<ABuffer> fBuffer);
  OMX_U32 OpenMMI(const KeyedVector<String8, String8> *headers);
  void QueryStreamType(OMX_U32 nPort);

  bool GetAttribute(const HTTPAALAttributeKey nKey, HTTPAALAttributeValue& nVal);
  bool SetAttribute(const HTTPAALAttributeKey nKey, const HTTPAALAttributeValue nVal);

  OMX_U32 GetOmxIndexByExtensionString(MMI_GetExtensionCmdType &nMMICmdType);
  status_t pause();
  status_t resume();

  bool IsSeekable();
  status_t getRepositionRange(uint64_t* pMin, uint64_t* pMax, uint64_t* pMaxDepth);
  bool isPlaybackDiscontinued();
  status_t getTrackInfo(Parcel *reply);
  bool addTrackInfoEntry(media_track_type eTrackType, char* sLang);
  bool resizeTrackInfo(int newSize);
  void resetTrackInfoParams();

  MMI_GetExtensionCmdType paramStructSMTPTimedTextDimensions;
  MMI_GetExtensionCmdType paramStructSMTPTimedTextSubInfo;

 OMX_INDEXTYPE mSMTPTimedTextDimensionsIndex;
 OMX_INDEXTYPE mSMTPTimedTextSubInfoIndex;
 OMX_INDEXTYPE mWatermarkExtIndex;
 OMX_INDEXTYPE mWatermarkStatusExtIndex;


  status_t prepareAsync();
  void sendPrepareDoneCallBack(status_t status);
  status_t getParameter(int key, void **data, size_t *size);
  status_t setParameter(int key, void *data, size_t size);


  OMX_INDEXTYPE mDrmExtraSampleInfo;
  OMX_INDEXTYPE mDrmParamPsshInfo;

  OMX_INDEXTYPE mQOEDataPlay;
  OMX_INDEXTYPE mQOEDataStop;
  OMX_INDEXTYPE mQOEDataSwitch;
  OMX_INDEXTYPE mQOEDataPeriodic;

  sp<AMessage> dataPeriodic;

  bool isLiveStream()
  {
    return (DASH_STREAM_TYPE_LIVE == eStreamType);
  }

  class TrackInfo
  {
  public:
    TrackInfo()
    {
      m_eTrackType = DASHMMIInterface::MEDIA_TRACK_TYPE_UNKNOWN;
      for(int i=0;i<4;i++)
      {
        m_sLang[i] = '\0';
      }
    }

    ~TrackInfo()
    {
    }

    /* @brief: Assignment operator overloading
    */
    TrackInfo& operator=(const TrackInfo& trackInfo)
    {
      Copy(trackInfo);
      return *this;
    }

    /* @brief: Copy constructor
    */
    TrackInfo(const TrackInfo &trackInfo)
    {
      Copy(trackInfo);
    }

    void Copy(const DASHMMIInterface::TrackInfo& trackInfo)
    {
      m_eTrackType = trackInfo.m_eTrackType;
      (void)std_strlcpy(m_sLang, trackInfo.m_sLang, std_strlen(trackInfo.m_sLang)+1);
    }

    void setTrackType(media_track_type eTrackType)
    {
      m_eTrackType = eTrackType;
    }

    void setLang(char* sLang)
    {
      if(sLang)
      {
        (void)std_strlcpy(m_sLang, sLang, std_strlen(sLang)+1);
      }
    }

    media_track_type getTrackType()
    {
      return m_eTrackType;
    }

    char* getLang()
    {
      return m_sLang;
    }

  private:
    media_track_type m_eTrackType;
    //MediaPlayer.java doc @lang returns a language code in either way of ISO-639-1 or ISO-639-2.
    //ISO-639-1 is 2ALPHA codes and ISO-639-2 is 3ALPHA codes.
    char m_sLang[4];
  };


private:
  void AddOemHeaders(const KeyedVector<String8, String8> *headers);
  void SetStreamType(DASHMMIInterface::DashStreamType lStreamType);

  bool bVideoDiscontinuity;
  bool bAudioDiscontinuity;

  bool bQOENotify;
  DashStreamType eStreamType;

  sp<DASHMMIMediaSource> mAudioSource;
  sp<DASHMMIMediaSource> mVideoSource;
  sp<DASHMMIMediaSource> mTextSource;
  void * mGetParamStruct;
  // DRM
  sp<DecryptHandle> mDecryptHandle;
  DrmManagerClient* mDrmManagerClient;
  Mutex mDrmLock;
  int mNumDecryptUnits;
  int mDecryptUnitIds[MAX_TRACKS];
  QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO* mContentProtectionInfo;

  int mLastVideoUniqueID;
  int mLastAudioUniqueID;

  uint32_t mAudioPSSHDataSize;
  uint32_t mVideoPSSHDataSize;
  uint32_t mTextPSSHDataSize;

  const DrmBuffer* openDecryptBuffer;
  char mBufferingIndication;
  status_t mAALDrmError;
  int mDrmCorruptFrameCount;
  int TotalFrameCount;

  TrackInfo *m_pTrackInfo;
  int m_nTotalNumTrackInfo;
  int m_nTrackInfoArrSize;

  int  mFlushCount;

public:
  void RegisterDecryptUnit(int track, int portIndex);
  int SetupDRMEnv();
  int GetContentProtectionInfo();
  int DrmCheckAndDecrypt(MMI_BufferCmdType* pFillBuffCmdResp);
  void DrmCleanup();
  void GetIndexForExtensions();
  void GetIndexForQOEExtensions(bool bNotify);
  QOMX_DRM_TYPE getDrmType();
  void setPSSHDataSizeOnPort(int portIndex, uint32_t size);
  uint32_t getPSSHDataSizeOnPort(int portIndex);
  void HandleDRMError();
  bool getBufferingStatus() {return (mBufferingIndication & 0x03);}
  bool checkAndSetPortValidityForBufferingEvents(uint32_t nPortIndex);

private:
  int DrmInit();
  int OMXPrefetch(OMX_STRING param, void* omxStruct, OMX_INDEXTYPE *omxIndex);
  int GetPSSHInfo(int portIndex, QOMX_PARAM_STREAMING_PSSHINFO** psshPtr, int uniqueId = 0);
  void PrintPSSHInfo(QOMX_PARAM_STREAMING_PSSHINFO* pPsshInfo);
  void PrintExtraSampleInfo(QOMX_EXTRA_SAMPLE_INFO *pExtraSampleInfo);
  void SetLastUniqueID(int nPortIndex, int uniqueId);
  int GetLastUniqueID(int nPortIndex);
  bool canBufferingBeSent(uint32_t mPort, status_t iBuffStatus);
  void updatePortObjectValidity(uint32_t mPort, bool bValidity);
  bool canBufferingEndBeSent(uint32_t mPort, status_t iBuffStatus);
  status_t getAALDRMError() {return mAALDrmError;}
  void setAALDRMError(status_t error);

private:
  class SignalHandler
  {
  public:
    SignalHandler();
    ~SignalHandler();

    uint32 Wait(HTTPAALSignalSet signal, int time, int *timeout);
    int32 Signal(HTTPAALSignalSet signal);

//  private:
public:
    MM_HANDLE m_signalQ;
    MM_HANDLE m_wakeupSignal;
    MM_HANDLE m_videoSignal;
    MM_HANDLE m_audioSignal;
    MM_HANDLE m_stopSignal;
  }m_signalHandler;

public:

class CPortsValidity : public RefBase
{
 private:
  uint32_t mPort;
  bool bPortValid;
  status_t mBufferingStatus;

  public:
  CPortsValidity(uint32_t nPort);

  void setPort(uint32_t nPort) {mPort = nPort;}
  uint32_t getPort() {return mPort;}

  void updatePortValidity(bool bValid) {bPortValid = bValid;}
  bool isValidPort() {return bPortValid;}

  void setStatus(status_t iStatus) {mBufferingStatus = iStatus;}
  status_t getStatus() {return mBufferingStatus;}

  ~CPortsValidity(){;}
};

 List<sp<DASHMMIInterface::CPortsValidity> > mAvailableValidPorts;

 sp<DASHMMIInterface::CPortsValidity> getObjectByPort(uint32_t mPort);

 sp<DASHMMIInterface::CPortsValidity> mValidAudioPort;
 sp<DASHMMIInterface::CPortsValidity> mValidVideoPort;

class CSrcQueueSt : public RefBase
{
public:
   sp<ABuffer> mBuffer;
   OMX_BUFFERHEADERTYPE bufHdr;
   CSrcQueueSt() {
     mBuffer = NULL;
     QOMX_STRUCT_INIT(bufHdr,OMX_BUFFERHEADERTYPE);
   }
   ~CSrcQueueSt() {;}
};

class CSourceBuffer
{


   Mutex mSrcBufLock;
   List<sp<DASHMMIInterface::CSrcQueueSt> > mSrcBuf;
   int32_t count;
   public:
   CSourceBuffer();
   ~CSourceBuffer();

public:
   void enqueue(sp<DASHMMIInterface::CSrcQueueSt> buf); //queues at the end
   status_t dequeue(sp<DASHMMIInterface::CSrcQueueSt> *buf); // dequeues from the begining
   int32_t getBufCount();
};

public:
    CSourceBuffer mVidBuffer;
    CSourceBuffer mAudBuffer;
    CSourceBuffer mTextBuffer;

protected:
  static void EventHandler(OMX_IN OMX_U32 nEvtCode,
                           OMX_IN OMX_U32 nEvtStatus,
                           OMX_IN OMX_U32 nPayloadLen,
                           OMX_IN OMX_PTR pEvtData,
                           OMX_IN OMX_PTR pClientData);

  int InitializeHTTPAALStates();



public:
  static const uint32 BITRATE = 512000;
  uint32 m_highWatermark;
  uint32 m_lowWatermark;
  bool m_bRebuffering;
  //latest status from mmi
  uint32 m_recentAudStatus;
  uint32 m_recentVidStatus;
  uint32 m_recentTextStatus;
  bool m_bAudEos;
  bool m_bVidEos;
  bool m_bTextEos;

private:

  friend class CHTTPAALStateBase;
  friend class CHTTPAALStatePlaying;
  friend class DASHMMIMediaSource;   /* !Warning : */
  friend class DASHMMIMediaInfo; /* !Warning : */


  struct SessionSeek
  {
    int64 m_seekTime;
    bool m_bTrackSeek[MAX_TRACKS];
  public:
    void Reset();
  }m_sessionSeek;

public:
  /* HTTP AAL States */
  CHTTPAALStateBase *m_pHTTPAALStates[HTTPAALStateMax];
  /* Active HTTP AAL State */
  CHTTPAALStateBase *m_pCurrentHTTPAALState;

  MM_HANDLE m_hHTTPAALStateLock;

  //lock for handling synchronization among
  //sources in context of mmi calls
  MM_HANDLE m_hHTTPAALSeekLock;

  MM_HANDLE m_hHTTPAALReadLock;
  MM_HANDLE m_hHTTPAALAudioReadLock;
  MM_HANDLE m_hHTTPAALVideoReadLock;
  MM_HANDLE m_hHTTPAALTextReadLock;

  //http mmi handle
  OMX_HANDLETYPE m_handle;

  //uri
  OMX_STRING m_uri;
};

class CHTTPAALStateBase
{
public:
  CHTTPAALStateBase(DASHMMIInterface &pMMHTTPDataCache,
                    HTTPAALState eHTTPAALState);
  virtual ~CHTTPAALStateBase(void);

public:
  /*
   * Returns the current state enumeration
   *
   *  @return current state
   */
  HTTPAALState GetState()
  {
    return m_eHTTPAALState;
  }

  virtual uint32 ProcessCmd(DASHMMIInterface::HTTPAALCmd cmd,
                            int32 arg1, int32 arg2, int64 arg3);
  virtual bool GetAttribute(const HTTPAALAttributeKey /*nKey*/, HTTPAALAttributeValue& /*nVal*/)
  {
    return false;
  };
  virtual bool SetAttribute(const HTTPAALAttributeKey /*nKey*/, const HTTPAALAttributeValue /*nVal*/)
  {
    //ToDo: Not supported for now!
    return false;
  };

  virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
                            OMX_IN OMX_U32 nEvtStatus,
                            OMX_IN OMX_U32 nPayloadLen,
                            OMX_IN OMX_PTR pEvtData);
  virtual bool EntryHandler();
  virtual bool ExitHandler();

/*
 * Private data
 */
protected:
  //HTTP AAL Data Source
  DASHMMIInterface &m_pMMHTTPDataCache;

  //Enumration of the current state the instace of this object is
  HTTPAALState m_eHTTPAALState;
};

class CHTTPAALStateIdle: public CHTTPAALStateBase
{
public:
  CHTTPAALStateIdle(DASHMMIInterface &pMMHTTPDataCache);
  virtual ~CHTTPAALStateIdle(void);

public:
  virtual uint32 ProcessCmd(DASHMMIInterface::HTTPAALCmd cmd,
                            int32 arg1, int32 arg2, int64 arg3);
  virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
                            OMX_IN OMX_U32 nEvtStatus,
                            OMX_IN OMX_U32 nPayloadLen,
                            OMX_IN OMX_PTR pEvtData);
  //virtual bool EntryHandler();
  //virtual bool ExitHandler();
};

class CHTTPAALStateConnecting: public CHTTPAALStateBase
{
public:
  CHTTPAALStateConnecting(DASHMMIInterface &pMMHTTPInterface);
  virtual ~CHTTPAALStateConnecting(void);

private:
  uint32 m_nNumTracksAvailable;
  uint32 m_nNumTracksReady;
  uint32 m_nValidPortCount;
  struct MMICmd
  {
      DASHMMIInterface::HTTPAALCmd cmd;
      int32      arg1;
      int32      arg2;
      int64      arg3;
  } *mStoredSeekCmnd;


public:
  virtual uint32 ProcessCmd(DASHMMIInterface::HTTPAALCmd cmd,
                            int32 arg1, int32 arg2, int64 arg3);
  virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
                    OMX_IN OMX_U32 nEvtStatus,
                    OMX_IN OMX_U32 nPayloadLen,
                    OMX_IN OMX_PTR pEvtData);
  //virtual bool EntryHandler();
  //virtual bool ExitHandler();
};

class CHTTPAALStatePlaying: public CHTTPAALStateBase
{
public:
  CHTTPAALStatePlaying(DASHMMIInterface &pMMHTTPDataCache);
  virtual ~CHTTPAALStatePlaying(void);

public:
  virtual uint32 ProcessCmd(DASHMMIInterface::HTTPAALCmd cmd,
                            int32 arg1, int32 arg2, int64 arg3);
  virtual bool GetAttribute(const HTTPAALAttributeKey nKey, HTTPAALAttributeValue& nVal);
  virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
                            OMX_IN OMX_U32 nEvtStatus,
                            OMX_IN OMX_U32 nPayloadLen,
                            OMX_IN OMX_PTR pEvtData);
};

class CHTTPAALStatePausing: public CHTTPAALStateBase
{
public:
  CHTTPAALStatePausing(DASHMMIInterface &pMMHTTPInterface);
  virtual ~CHTTPAALStatePausing(void);

public:
  virtual uint32 ProcessCmd(DASHMMIInterface::HTTPAALCmd cmd,
                            int32 arg1, int32 arg2, int64 arg3);
  virtual bool GetAttribute(const HTTPAALAttributeKey nKey, HTTPAALAttributeValue& nVal);
  virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
                            OMX_IN OMX_U32 nEvtStatus,
                            OMX_IN OMX_U32 nPayloadLen,
                            OMX_IN OMX_PTR pEvtData);
};

class CHTTPAALStatePaused: public CHTTPAALStateBase
{
public:
  CHTTPAALStatePaused(DASHMMIInterface &pMMHTTPInterface);
  virtual ~CHTTPAALStatePaused(void);

public:
  virtual uint32 ProcessCmd(DASHMMIInterface::HTTPAALCmd cmd,
                            int32 arg1, int32 arg2, int64 arg3);
  virtual bool GetAttribute(const HTTPAALAttributeKey nKey, HTTPAALAttributeValue& nVal);
  virtual void EventHandler(OMX_IN OMX_U32 nEvtCode,
                    OMX_IN OMX_U32 nEvtStatus,
                    OMX_IN OMX_U32 nPayloadLen,
                    OMX_IN OMX_PTR pEvtData);
};

class CHTTPAALStateResuming: public CHTTPAALStateBase
{
public:
  CHTTPAALStateResuming(DASHMMIInterface &pMMHTTPInterface);
  virtual ~CHTTPAALStateResuming(void);

public:
  virtual uint32 ProcessCmd(DASHMMIInterface::HTTPAALCmd cmd,
                            int32 arg1, int32 arg2, int64 arg3);
  virtual bool GetAttribute(const HTTPAALAttributeKey nKey, HTTPAALAttributeValue& nVal);
  void EventHandler(OMX_IN OMX_U32 nEvtCode,
                    OMX_IN OMX_U32 nEvtStatus,
                    OMX_IN OMX_U32 nPayloadLen,
                    OMX_IN OMX_PTR pEvtData);
};

class CHTTPAALStateClosing: public CHTTPAALStateBase
{
public:
  CHTTPAALStateClosing(DASHMMIInterface &pMMHTTPInterface);
  virtual ~CHTTPAALStateClosing(void);

public:
  virtual uint32 ProcessCmd(DASHMMIInterface::HTTPAALCmd cmd,
                            int32 arg1, int32 arg2, int64 arg3);
  void EventHandler(OMX_IN OMX_U32 nEvtCode,
                    OMX_IN OMX_U32 nEvtStatus,
                    OMX_IN OMX_U32 nPayloadLen,
                    OMX_IN OMX_PTR pEvtData);
  virtual bool EntryHandler();
  virtual bool ExitHandler();
};

}

#endif  // DASH_MMI_INTERFACE_H
