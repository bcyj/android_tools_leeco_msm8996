#ifndef __DASHMEDIAREPRESENTATIONHANDLER_H__
#define __DASHMEDIAREPRESENTATIONHANDLER_H__
/************************************************************************* */
/**
 * DASHMediaRepresentationHandler.h
 * @brief Defines the DASHMediaRepresentationHandler interface.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/DASHMediaRepresentationHandler.h#42 $
$DateTime: 2013/07/28 21:38:43 $
$Change: 4175800 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "DASHMediaSegmentHandler.h"
#include "HTTPResourceManager.h"
#include "HTTPSegmentDataStoreStructs.h"
#include "HTTPBandwidthEstimator.h"
#include <HTTPStackInterface.h>

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

namespace video {

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define USE_PERIOD_END_TIME -1
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
//Group notification handler interface
class iGroupNotifier
{
public:
  virtual ~iGroupNotifier() { };

  virtual void SegInfoReady(const uint64 nRepKey,
                            const uint64 nStartTime,
                            const uint64 nDuration,
                            const uint32 nNumberofUnits,
                            const HTTPDownloadStatus eStatus) = 0;
  virtual void SegDataReady(const uint64 nRepKey,
                            const uint64 nDataUnitKey,
                            const HTTPDownloadStatus eStatus) = 0;
  virtual void NotifyError(const uint64 nRepKey,
                           const HTTPDownloadStatus eStatus) = 0;

  virtual void NotifySeekStatus(uint64 nRepKey,
                                int64 nSeekEndTime,
                                int64 nCurTime,
                                HTTPDownloadStatus eStatus)=0;

  virtual void NotifyDownloadTooSlow(const uint64 nRepKey,
                                     const uint64 nDataUnitKey) = 0;

  virtual void NotifyDataUnitRemoved(const uint64 nRepKey,
                                     const uint64 nFirstRemovedDataUnitKey,
                                     const uint64 nLastRemovedDataUnitKey) = 0;

  virtual int GetBufferedDurationFromNotifier() = 0;
};

  //Initialization segment
class InitializationSegment
{
public:

  //Representation handler commands
  enum InitSegState
  {
    INITSEG_STATE_IDLE,
    INITSEG_STATE_DOWNLOADING,
    INITSEG_STATE_DOWNLOADED,
    INITSEG_STATE_ERROR
  };

  InitializationSegment(const char* pURL, uint64 nStartOffset, uint64 nEndOffset,
                        DASHSessionInfo& sDASHSessionInfo, HTTPStackInterface* HTTPStack,
                        HTTPBandwidthEstimator * /* pSharedBandwidthEstimator */, InitSegState state, bool& bOk)
    :m_pURL(pURL),
    m_nStartOffset(nStartOffset),
    m_nEndOffset(nEndOffset),
    m_pHTTPStack(HTTPStack),
    m_bState(state),
    m_InitLock(NULL),
    m_bIsCommonInit(false)
  {
    m_cDataStore.SetHeapManager(&sDASHSessionInfo.cHeapManager);
    bOk = m_cDownloader.Init(&sDASHSessionInfo.sSessionInfo, m_pHTTPStack);
    MM_CriticalSection_Create(&m_InitLock);
  };

  ~InitializationSegment()
  {
    if(m_InitLock)
    {
      MM_CriticalSection_Release(m_InitLock);
      m_InitLock = NULL;
    }
  };

  void Reset()
  {
    (void)m_cDownloader.Stop();
    m_cDataStore.Reset();
  };

  InitSegState getInitState()
  {
    InitSegState mState;
    MM_CriticalSection_Enter(m_InitLock);
    mState = m_bState;
    MM_CriticalSection_Leave(m_InitLock);
    return mState;
  };

  void setInitState(InitSegState mState)
  {
    MM_CriticalSection_Enter(m_InitLock);
    m_bState = mState;
    MM_CriticalSection_Leave(m_InitLock);
  };

  void setCommonInit()
  {
    MM_CriticalSection_Enter(m_InitLock);
    m_bIsCommonInit = true;
    MM_CriticalSection_Leave(m_InitLock);
  }

  bool isCommonInit()
  {
    bool ret = false;
    MM_CriticalSection_Enter(m_InitLock);
    ret = m_bIsCommonInit;
    MM_CriticalSection_Leave(m_InitLock);
    return ret;
  }

  const char* m_pURL;
  int64 m_nStartOffset;
  int64 m_nEndOffset;
  HttpSegmentDataStoreHeap m_cDataStore;
  SegmentDownloader m_cDownloader;
  HTTPStackInterface* m_pHTTPStack;
  InitSegState m_bState;

  /* For init lock */
  MM_HANDLE m_InitLock;

  bool m_bIsCommonInit;
};

class DASHMediaRepresentationHandler : public HTTPResourceManager,
                                       public iRepresentationNotifier
{
public:
  DASHMediaRepresentationHandler(bool& bResult,
                                 const uint32 nKey,
                                 DASHSessionInfo& sDASHSessionInfo,
                                 RepresentationInfo& cRepInfo,
                                 iGroupNotifier* pNotifier,
                                 Scheduler* pScheduler = NULL,
                                 HTTPStackInterface* pHTTPStack = NULL,
                                 InitializationSegment* m_pCmnInitSegment = NULL);

  virtual ~DASHMediaRepresentationHandler();

  //Representation handler commands
  enum RepresentationCmd
  {
    REPRESENTATION_CMD_NONE,
    REPRESENTATION_CMD_GET_SEGINFO,
    REPRESENTATION_CMD_OPEN,
    REPRESENTATION_CMD_SEEK,
    REPRESENTATION_CMD_NOTIFY_SEEK
  };

  void ClearBufferedData(HTTPCommon::HTTPMediaType majorType, uint64 nTime);

  /**
   * Get the base time for the representation taking into account
   * pts offset.
   */
  bool GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime);

  /**
   * Flsuh taking into account pts offset.
   */
  HTTPDownloadStatus Flush(HTTPMediaType majorType,int64 nStartTime);

  /**
   * Get the next media sample with adjusted for non-zero pts
   * offset.
   */
  HTTPCommon::HTTPDownloadStatus GetNextMediaSample(HTTPCommon::HTTPMediaType majorType,
                                                    uint8 *pBuffer,
                                                    uint32 &nSize,
                                                    HTTPSampleInfo &sampleInfo);

  void GetVideoInfo(HTTPCommon::HTTPMediaType majorType, char* pVideoURL,size_t& nURLSize, char* pIPAddr, size_t& nIPAddrSize)
  {
    HTTPResourceManager::GetVideoInfo(majorType,pVideoURL,nURLSize,pIPAddr,nIPAddrSize);
  }

  /**
   * Get the download position adjusted for pts offset
   */
  bool GetDownloadPosition( HTTPCommon::HTTPMediaType majorType,
                            uint64& nDownloadPosition);

  bool IsLastSegDownloadSucceed();

  bool IsLmsgSet();

//HTTPResourceManager methods
public:
  virtual HTTPDownloadStatus Open(uint64 nDataUnitKey, int64 nStartTime, bool bSeek=false);

  virtual HTTPDownloadStatus GetSegmentInfo(uint64 nStartTime,
                                            uint64 nDuration)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetSegmentInfo(nStartTime, nDuration) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };
  virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo *pDownloadInfo,
                                                       uint32 nSize,
                                                       uint32 &nFilled,
                                                       uint64 nStartTime)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
        pCurrentStateHandler->GetDataUnitDownloadInfo(pDownloadInfo, nSize,
                                                  nFilled, nStartTime):
        HTTPCommon::HTTPDL_ERROR_ABORT);
  };


  virtual HTTPDownloadStatus FillDataUnitInfo(uint64 nStartTime,
                                              uint64 nDuration,
                                              QSM::CDataUnitInfo* pDataUnitInfo,
                                              uint32 nSizeOfDataUnitInfo,
                                              uint32& nNumDataUnitInfo)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();

    HTTPDownloadStatus rslt =
      (pCurrentStateHandler ?
       pCurrentStateHandler->FillDataUnitInfo(nStartTime + m_cRepInfo.GetPTSOffset(),
                                              nDuration, pDataUnitInfo,
                                              nSizeOfDataUnitInfo, nNumDataUnitInfo) :
       HTTPCommon::HTTPDL_ERROR_ABORT);


    if (HTTPCommon::HTTPDL_SUCCESS == rslt)
    {
      for (uint32 i = 0; i < nNumDataUnitInfo; ++i)
      {
        if (pDataUnitInfo)
        {
          if (pDataUnitInfo[i].m_nStartTime > m_cRepInfo.GetPTSOffset())
          {
            pDataUnitInfo[i].m_nStartTime -= m_cRepInfo.GetPTSOffset();
          }
          else
          {
            pDataUnitInfo[i].m_nStartTime = 0;
          }
        }
      }
    }

    return rslt;
  };

  virtual HTTPDownloadStatus GetSegmentData(uint64 nDataUnitKey)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetSegmentData(nDataUnitKey) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64 nDataUnitKey,
                                                  uint64 &nStartTime,
                                                  uint64 &nDuration)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetDataUnitInfoByKey(nDataUnitKey, nStartTime, nDuration):
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus CancelSegmentData(uint64 nDataUnitKey)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->CancelSegmentData(nDataUnitKey) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus ContinueDownloadDataUnit(uint64 nDataUnitKey)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->ContinueDownloadDataUnit(nDataUnitKey) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  }

  virtual HTTPDownloadStatus Seek(uint64 nSeekTime)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->Seek(nSeekTime) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus Select(uint64 nDataUnitKey,
                                    uint64& nPbTime)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->Select(nDataUnitKey, nPbTime) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus Close(HTTPMediaType majorType);

  virtual HTTPDownloadStatus IsOpenCmdProcessingAllowed();

  virtual HTTPDownloadStatus Close()
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "Rep [0x%06x]: Closing",
                   (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40) );
    HTTPDownloadStatus status = HTTPResourceManager::Close();
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      (void)pCurrentStateHandler->Close();
    }
    return status;
  };

  virtual void SetEndTime(uint64 nEndTime);

  virtual int GetRepInfoKey();

//iRepresentationNotifier methods
public:
  virtual void SegInfoReady(const uint32 nSegKey,
                            const uint32 nNumDataUnits,
                            const HTTPDownloadStatus eStatus)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->SegInfoReady(nSegKey, nNumDataUnits, eStatus);
    }
  };

  virtual void SegDataReady(const uint32 nSegKey,
                            const uint64 nDataUnitKey,
                            const HTTPDownloadStatus eStatus)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->SegDataReady(nSegKey, nDataUnitKey, eStatus);
    }
  };

  virtual void NotifyError(const uint32 /* nSegKey */,
                           const HTTPDownloadStatus eStatus)
  {
    //Segment with key nSegKey notified error
    if (m_pGroupNotifier)
    {
      m_pGroupNotifier->NotifyError(m_nKey, eStatus);
    }
  };

  virtual void NotifySeekStatus(uint32 nSegKey,int64 nSeekTime,HTTPDownloadStatus status)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->NotifySeekStatus(nSegKey, nSeekTime, status);
    }
  };

  virtual void NotifyDownloadTooSlow(const uint32 nSegKey,
                                     const uint64 nDataUnitKey)
  {
    RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->NotifyDownloadTooSlow(nSegKey ,nDataUnitKey);
    }
  }
  virtual void NotifyDataUnitRemoved(const uint32 nSegKey,
                                     const uint64 nFirstRemovedDataUnitKey,
                                     const uint64 nLastRemovedDataUnitKey)
  {
    if (m_pGroupNotifier)
    {
      m_pGroupNotifier->NotifyDataUnitRemoved(
         m_nKey,
         (((uint64)nSegKey << 32) | nFirstRemovedDataUnitKey),
         (((uint64)nSegKey << 32) | nLastRemovedDataUnitKey));
    }
  }

  virtual int GetBufferedDurationFromNotifier()
  {
    int duration = 0;

    if (m_pGroupNotifier)
    {
      duration = m_pGroupNotifier->GetBufferedDurationFromNotifier();
    }

    return duration;
  }

private:
  //Representation handler states
  enum RepresentationState
  {
    REPRESENTATION_STATE_IDLE,
    REPRESENTATION_STATE_SETUP,
    REPRESENTATION_STATE_READY,
    REPRESENTATION_STATE_ERROR
  };

  //State handlers
  class RepresentationBaseStateHandler
  {
  public:
    RepresentationBaseStateHandler(RepresentationState eState,
                                   DASHMediaRepresentationHandler* pRepHdlr)
      : m_eState(eState),
        m_pRepHandler(pRepHdlr)
    {
    };

    virtual ~RepresentationBaseStateHandler()
    {
    };

    virtual int ProcessCmds() = 0;

    virtual HTTPDownloadStatus GetSegmentInfo(uint64 nStartTime,
                                              uint64 nDuration);

    virtual HTTPDownloadStatus FillDataUnitInfo(uint64 /* nStartTime */,
                                                uint64 /* nDuration */,
                                                QSM::CDataUnitInfo* /* pDataUnitInfo */,
                                                uint32 /* nNumDataUnits */,
                                                uint32& /* nNumDataUnitsFilled */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for FillDataUnitInfo() call", m_eState );
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo */* pDownloadInfo */,
                                                       uint32 /* nSize */,
                                                       uint32 &/* nFilled */,
                                                       uint64 /* nStartTime */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for GetDataUnitDownloadInfo() call", m_eState );
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetSegmentData(uint64 /* nDataUnitKey */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for GetSegmentData() call", m_eState );
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64 /* nDataUnitKey */,
                                                    uint64 & /* nStartTime */,
                                                    uint64 & /* nDuration */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for GetDataUnitInfoByKey() call", m_eState );
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Open(uint64 nDataUnitKey,int64 nStartTime,bool bSeek);

    virtual HTTPDownloadStatus CancelSegmentData(uint64 /* nDataUnitKey */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for CancelSegmentData() call", m_eState );
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus ContinueDownloadDataUnit(uint64 /* nKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus Seek(int64 /* nSeekTime */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for Seek() call", m_eState );
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Select(uint64 /* nDataUnitKey */,
                                      uint64& /* nPbTime */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for Select() call", m_eState );
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual void SegInfoReady(const uint32 /* nSegKey */,
                              const uint32 /* nNumDataUnits */,
                              const HTTPDownloadStatus /* eStatus */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for SegInfoReady() call", m_eState );
    };

    virtual void SegDataReady(const uint32 /* nSegKey */,
                              const uint64 /* nDataUnitKey */,
                              const HTTPDownloadStatus /* eStatus */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for SegDataReady() call", m_eState );
    };
    virtual void NotifySeekStatus(uint32 /* nSegKey */,int64 /* nSeekTime */,HTTPDownloadStatus /*status */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for NotifySeekStatus() call", m_eState );
    }

    virtual HTTPDownloadStatus Close();

    virtual void NotifyDownloadTooSlow(const uint32 /* nSegKey */,
                                       const uint64 /* nDataUnitKey */)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for NotifyDownloadTooSlow() call", m_eState );
    }

  protected:
    RepresentationState m_eState;
    DASHMediaRepresentationHandler* m_pRepHandler;
  };

  class RepresentationIdleStateHandler : public RepresentationBaseStateHandler
  {
  public:
    RepresentationIdleStateHandler(DASHMediaRepresentationHandler* pRepHdlr)
      : RepresentationBaseStateHandler(REPRESENTATION_STATE_IDLE, pRepHdlr)
    {
    };

    virtual ~RepresentationIdleStateHandler()
    {
    };

    virtual int ProcessCmds();
  };

  class RepresentationSetupStateHandler : public RepresentationBaseStateHandler
  {
  public:
    RepresentationSetupStateHandler(DASHMediaRepresentationHandler* pRepHdlr)
      : RepresentationBaseStateHandler(REPRESENTATION_STATE_SETUP, pRepHdlr)
    {
    };

    virtual ~RepresentationSetupStateHandler()
    {
    };

    virtual int ProcessCmds();
  };

  class RepresentationReadyStateHandler : public RepresentationBaseStateHandler
  {
  public:
    RepresentationReadyStateHandler(DASHMediaRepresentationHandler* pRepHdlr)
      : RepresentationBaseStateHandler(REPRESENTATION_STATE_READY, pRepHdlr)
    {
    };

    virtual ~RepresentationReadyStateHandler()
    {
    };

    virtual int ProcessCmds();

    virtual HTTPDownloadStatus Open(uint64 nDataUnitKey,int64 nStartTime,bool bSeek);

    virtual HTTPDownloadStatus FillDataUnitInfo(uint64 nStartTime,
                                                uint64 nDuration,
                                                QSM::CDataUnitInfo* pDataUnitInfo,
                                                uint32 nNumDataUnits,
                                                uint32& nNumDataUnitsFilled);

    virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo *pDownloadInfo,
                                                       uint32 nSize,
                                                       uint32 &nFilled,
                                                       uint64 nStartTime);


    virtual HTTPDownloadStatus GetSegmentData(uint64 nDataUnitKey);

    virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64 nDataUnitKey,
                                                    uint64 &nStartTime,
                                                    uint64 &nDuration);

    virtual HTTPDownloadStatus CancelSegmentData(uint64 nDataUnitKey);
    virtual HTTPDownloadStatus ContinueDownloadDataUnit(uint64 nKey);

    virtual HTTPDownloadStatus Select(uint64 nDataUnitKey,
                                      uint64& nPbTime);

    virtual void SegInfoReady(const uint32 nSegKey,
                              const uint32 nNumDataUnits,
                              const HTTPDownloadStatus eStatus);

    virtual void SegDataReady(const uint32 nSegKey,
                              const uint64 nDataUnitKey,
                              const HTTPDownloadStatus eStatus)
    {
      if (m_pRepHandler->m_pGroupNotifier)
      {
        m_pRepHandler->m_pGroupNotifier->SegDataReady(m_pRepHandler->m_nKey,
                                                      (((uint64)nSegKey << 32) | nDataUnitKey),
                                                      eStatus);
      }
    };

    virtual void NotifyDownloadTooSlow(const uint32 nSegKey,
                                       const uint64 nDataUnitKey);

    virtual void NotifySeekStatus(uint32 nSegKey,int64 nSeekTime,HTTPDownloadStatus status);

    virtual HTTPDownloadStatus Seek(int64 nSeekTime);
  };

  class RepresentationErrorStateHandler : public RepresentationBaseStateHandler
  {
  public:
    RepresentationErrorStateHandler(DASHMediaRepresentationHandler* pRepHdlr)
      : RepresentationBaseStateHandler(REPRESENTATION_STATE_ERROR, pRepHdlr)
    {
    };

    virtual ~RepresentationErrorStateHandler()
    {
    };

    virtual int ProcessCmds()
    {
      return -1;
    };

    virtual HTTPDownloadStatus GetSegmentInfo(uint64 /* nStartTime */,
                                              uint64 /* nDuration */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Open(uint64 /* nDataUnitKey */, int64 /* nStartTime */, bool /* bSeek */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };
  };

  //Segment GET_SEGINFO cmd handler state
  enum SegmentInfoCmdHandlerState
  {
    SEGINFO_CMDHANDLER_IDLE,
    SEGINFO_CMDHANDLER_WAITING,
    SEGINFO_CMDHANDLER_AVAILABLE
  };

  //Segment GET_SEGINFO cmd handler
  class SegmentInfoCmdHandler
  {
  public:
    SegmentInfoCmdHandler(){ Reset(); };
    ~SegmentInfoCmdHandler(){ };

    SegmentInfoCmdHandlerState m_eState;
    int64 m_nNextStartTime;
    uint64 m_nCmdStartTime;
    int m_nStartIdx;
    bool m_bMoveToPrevSegment;

    // 404 associated with this segment.
    bool m_bIsSegmentNotFound;

    void Reset()
    {
      m_eState = SEGINFO_CMDHANDLER_IDLE;
      m_nNextStartTime = -1;
      m_nStartIdx = -1;
      m_nCmdStartTime = 0;
      m_bMoveToPrevSegment = true;
      m_bIsSegmentNotFound = false;
    };
  };

  //Representation base cmd data - to be dequeued by TaskMediaRepresentation
  struct RepresentationBaseCmdData
  {
    RepresentationCmd eCmd;
  };

  //Representation GET_SEGINFO cmd data
  struct RepresentationGetSegInfoCmdData : public RepresentationBaseCmdData
  {
    uint64 nStartTime;
    uint64 nDuration;
  };

   //Representation OPEN cmd data
  struct RepresentationOpenCmdData : public RepresentationBaseCmdData
  {
    uint64 nStartTime;
    uint64 nDataUnitKey;
    bool bSeek;
  };
  struct RepresentationSeekCmdData : public RepresentationBaseCmdData
  {
    int64 nSeekTime;
  };
  struct RepresentationNotifySeekCmdData : public RepresentationBaseCmdData
  {
    HTTPDownloadStatus eStatus;
    int64 nSeekTime;
    uint32 nSegKey;
  };

  //Representation cmd data
  union RepresentationCmdData
  {
    RepresentationBaseCmdData sBaseCmdData;
    RepresentationGetSegInfoCmdData sGetSegInfoCmdData;
    RepresentationOpenCmdData sOpenCmdData;
    RepresentationSeekCmdData sSeekCmdData;
    RepresentationNotifySeekCmdData sNotifySeekCmdData;
  };

  //Representation command task param
  class RepresentationCmdTaskParam : public SchedulerTaskParamBase
  {
  public:
    RepresentationCmdTaskParam(void* pParam) : pSelf(pParam){ };
    virtual ~RepresentationCmdTaskParam(){ };

    void* pSelf;
  };

  void SetStateHandler(RepresentationBaseStateHandler* pCurrStateHandler)
  {
    MM_CriticalSection_Enter(m_pRepDataLock);
    m_pCurrentStateHandler = pCurrStateHandler;
    MM_CriticalSection_Leave(m_pRepDataLock);
  }

  RepresentationBaseStateHandler* GetStateHandler()
  {
    RepresentationBaseStateHandler* pCurrStateHandler = NULL;
    MM_CriticalSection_Enter(m_pRepDataLock);
    pCurrStateHandler = m_pCurrentStateHandler;
    MM_CriticalSection_Leave(m_pRepDataLock);
    return pCurrStateHandler;
  }

  //Representation task
  static int TaskMediaRepresentation(void* pParam);

  bool CreateSegmentHandler(const uint32 nSegKey, SegmentInfo& cSegnfo, HTTPResource** ppSegHandler);
  HTTPResource* GetSegmentHandler(const uint64 nDataUnitKey);

  /**
   * Override the download status for all commands in the cmd
   * queue if needed. The current use case is only for invalid
   * representation, so not needed to associate a cmd with the
   * status, as there should be only one cmd when the rep is being
   * setup.
   */
  void ClearCmdQ(HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT);

  void OnError(const HTTPDownloadStatus eStatus);

  //Session and representation info
  uint32 m_nKey;
  DASHSessionInfo& m_sDASHSessionInfo;
  RepresentationInfo& m_cRepInfo;
  /*Bandwidth Estimator object to record amount of
    data downloaded and total time taken.*/
  HTTPBandwidthEstimator *m_bEstimator;

  iGroupNotifier* m_pGroupNotifier;

  Scheduler* m_pScheduler;
  HTTPStackInterface* m_pHTTPStack;
  int m_nTaskID;

  //Lock to synchronize representation data access
  MM_HANDLE m_pRepDataLock;

  //INIT segment
  InitializationSegment* m_pInitSegment;

  SegmentInfoCmdHandler m_cSegInfoCmdHandler;

  //Representation state handlers
  RepresentationIdleStateHandler m_cIdleStateHandler;
  RepresentationSetupStateHandler m_cSetupStateHandler;
  RepresentationReadyStateHandler m_cReadyStateHandler;
  RepresentationErrorStateHandler m_cErrorStateHandler;

  RepresentationBaseStateHandler* m_pCurrentStateHandler;

  //Representation cmd queue
  StreamDataQ<RepresentationCmdData> m_cCmdQ;

  bool m_bSeekPending;

  // reference to the SidxFetchDecision object for the adaptation-set.
  SidxFetchDecision m_rSidxFetchDecision;
  bool m_bIsLmsgSet;
};

} // namespace video

#endif //__DASHMEDIAREPRESENTATIONHANDLER_H__
