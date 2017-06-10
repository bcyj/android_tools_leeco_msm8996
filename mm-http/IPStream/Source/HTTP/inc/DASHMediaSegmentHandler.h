#ifndef __DASHMEDIASEGMENTHANDLER_H__
#define __DASHMEDIASEGMENTHANDLER_H__
/************************************************************************* */
/**
 * DASHMediaSegmentHandler.h
 * @brief Defines the DASHMediaSegmentHandler interface.
 *
 * COPYRIGHT 2011-2015 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/DASHMediaSegmentHandler.h#69 $
$DateTime: 2014/02/06 23:45:40 $
$Change: 5239579 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include <StreamDataQueue.h>
#include <MPDParser.h>
#include <Scheduler.h>
#include <QsmTypes.h>
#include <sidxparser.h>
#include "HTTPCommon.h"
#include "HTTPResource.h"
#include "SegmentDownloader.h"
#include "HTTPDataStoreBase.h"
#include "HTTPDataManager.h"
#include "HTTPBandwidthEstimator.h"

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

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
class HttpSegmentDataStoreBase;

/**
 * Class to store the decision if SIDX fetch is turned off. Note
 * that, once turned off, SIDX fetch cannot be re-enabled for
 * the adaptation-set.
 */
class SidxFetchDecision
{
public:
  SidxFetchDecision() : m_bSidxFetchDisabled(false) { }
  ~SidxFetchDecision() { }
  void DisableSidxFetch() { m_bSidxFetchDisabled = true; }
  bool IsSidxFetchDisabled() const { return m_bSidxFetchDisabled; }
private:
  bool m_bSidxFetchDisabled;
};

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
//Representation notification handler interface
class iRepresentationNotifier
{
public:
  virtual ~iRepresentationNotifier() { };

  virtual void SegInfoReady(const uint32 nSegKey,
                            const uint32 nNumDataUnits,
                            const HTTPDownloadStatus eStatus) = 0;
  virtual void SegDataReady(const uint32 nSegKey,
                            const uint64 nDataUnitKey,
                            const HTTPDownloadStatus eStatus) = 0;
  virtual void NotifyError(const uint32 nSegKey,
                           const HTTPDownloadStatus eStatus) = 0;

  virtual void NotifySeekStatus(uint32 nSegKey,
                                int64 nSeekEndTime,
                                HTTPDownloadStatus eStatus)=0;

  virtual void NotifyDownloadTooSlow(const uint32 nSegKey,
                                     const uint64 nDataUnitKey) = 0;

  virtual void NotifyDataUnitRemoved(const uint32 nSegKey,
                                     const uint64 nFirstRemovedDataUnitKey,
                                     const uint64 nLastRemovedDataUnitKey) = 0;

  virtual int GetBufferedDurationFromNotifier() = 0;
};

class DASHMediaSegmentHandler : public HTTPResource, public iSegmentNotifier
{
public:
  DASHMediaSegmentHandler(bool& bResult,
                          DASHSessionInfo& sDASHSessionInfo,
                          iRepresentationNotifier* pNotifer,
                          HTTPBandwidthEstimator *pBandwidthEstimator,
                          SidxFetchDecision& rSidxFetchEnableInfo,
                          HttpSegmentDataStoreBase* pInitSegment = NULL,
                          Scheduler* pScheduler = NULL,
                          HTTPStackInterface* pHTTPStack = NULL);

  virtual ~DASHMediaSegmentHandler();

  //Segment handler commands
  enum SegmentCmd
  {
    SEGMENT_CMD_NONE,
    SEGMENT_CMD_GET_SEGINFO,
    SEGMENT_CMD_GET_SEGDATA,
    SEGMENT_CMD_CHECK_DATA_AVAIL_FOR_SEEK,
    SEGMENT_CMD_NOTIFY_SEEK_STATUS,
    SEGMENT_CMD_PROCESS_SEEK_FAILURE,
    SEGMENT_CMD_OPEN,
    SEGMENT_CMD_MAX
  };
 /*Bandwidth Estimator object to record amount of
    data downloaded and total time taken.*/
  HTTPBandwidthEstimator *m_bEstimator;

//HTTPResource methods
public:
  virtual HTTPDownloadStatus GetSegmentInfo(uint64 nStartTime,
                                            uint64 nDuration)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetSegmentInfo(nStartTime, nDuration) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus OpenSegment()
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->OpenSegment():HTTPCommon::HTTPDL_ERROR_ABORT);

  };

  virtual HTTPDownloadStatus FillDataUnitInfo(uint64 nStartTime,
                                              uint64 nDuration,
                                              QSM::CDataUnitInfo* pDataUnitInfo,
                                              uint32 nNumDataUnits,
                                              uint32& nNumDataUnitsFilled)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->FillDataUnitInfo(nStartTime,
                                                                          nDuration,
                                                                          pDataUnitInfo,
                                                                          nNumDataUnits,
                                                                          nNumDataUnitsFilled)
                                 : HTTPCommon::HTTPDL_ERROR_ABORT);
  };
  virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo *pDownloadInfo,
                                                     uint32 nSize,
                                                     uint32 &nFilled,
                                                     uint64 nStartTime)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->GetDataUnitDownloadInfo(pDownloadInfo,
                                                                                 nSize,
                                                                                 nFilled,
                                                                                 nStartTime)
                                 : HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus GetSegmentData(uint64 nDataUnitKey)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetSegmentData(nDataUnitKey) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64 nDataUnitKey,
                                                  uint64 &nStartTime,
                                                  uint64 &nDuration)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetDataUnitInfoByKey(nDataUnitKey,
                                                       nStartTime,
                                                       nDuration) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus Seek(uint64 nSeekTime)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->Seek(nSeekTime) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus Select(uint64 nDataUnitKey,
                                    uint64& nPbTime)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->Select(nDataUnitKey, nPbTime) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual char* GetIPAddr(size_t& size)
  {
    size = 0;
    if(m_pIPAddr)
    {
      size = std_strlen(m_pIPAddr);
    }
    return m_pIPAddr;
  };

  virtual HTTPDownloadStatus CancelSegmentData(uint64 nDataUnitKey)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->CancelSegmentData(nDataUnitKey) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  HTTPDownloadStatus ContinueDownloadDataUnit(uint64 nDataUnitKey)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->ContinueDownloadDataUnit(nDataUnitKey) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  }
  virtual HTTPDownloadStatus IsReadable(HTTPMediaType majorType)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->IsReadable(majorType) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual void ProcessFileSourceEvent(FileSourceCallBackStatus status)
  {
    MM_CriticalSection_Enter(m_pSegmentFSLock);
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->ProcessFileSourceEvent(status);
    }
    MM_CriticalSection_Leave(m_pSegmentFSLock);
  };
  virtual HTTPDownloadStatus Close()
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "Seg [0x%08x%08x]: Closing",
                   (uint32)(m_cSegmentInfo.getKey() >> 32), (uint32)m_cSegmentInfo.getKey() );
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      (void)pCurrentStateHandler->Close();
    }
    return HTTPResource::Close();
  };

  virtual void ClearBufferedData(uint64 nStartTime);

  virtual HTTPDownloadStatus Flush(HTTPMediaType majorType,int64 nStartTime);

  virtual HTTPDownloadStatus Close(HTTPMediaType majorType)
  {
    HTTPDownloadStatus status = CloseMedia(majorType);
    if(HTTPSUCCEEDED(status))
    {
      m_nSeekTime = -1;
      ResetDataDownloadState();
    }
    return status;
  }

  virtual HTTPDownloadStatus Open(uint64 nDataUnitKey,int64 nStartTime,bool bSeek=false);

  virtual void SetSegmentInfo(const SegmentInfo &cSegInfo, const uint64 nStartTime,
                              const uint64 nDuration,const uint64 nKey, const char *InitUrl)
  {
    m_cSegmentInfo = cSegInfo;
    m_nStartTime = nStartTime;
    m_nDuration = nDuration;
    m_nIndexURLPresent = m_cSegmentInfo.IsIndexURLPresent();
    SetKey(nKey);

    // As init segment sit before the SIDX segment in the datamanager, hence Fragments offsets from
    // Sidx parser requires adjustment for init segment size only if init segment is from different URL
    // Fragment offset adjustment is not required for Same Init and Media URL.
    // Cache the information regarding for init URL and use aptly for segment offset adjustment
    if(InitUrl && m_cSegmentInfo.GetURL())
    {
      m_bInitAndMediaWithSameURL = !std_strcmp(InitUrl, m_cSegmentInfo.GetURL());
    }
  };

  bool SetupDataStorage(HTTPDataManager*          pDataManager,
                        HttpSegmentDataStoreBase* initSegment);

  virtual HTTPDataManager* GetSidxDataManager()
  {
    HTTPDataManager* pSidxDataManager = NULL;
    if(-1 == m_nIndexURLPresent)
    {
      pSidxDataManager = HTTPResource::GetSidxDataManager();
    }
    else
    {
      pSidxDataManager = m_nIndexURLPresent ?
                       HTTPResource::GetSidxDataManager() : GetDataManager();
    }

    return pSidxDataManager;
  };

  char* GetVideoURL(size_t& size)
  {
    char* pURL = m_cSegmentInfo.GetURL();
    size = 0;
    if(pURL)
    {
      size = std_strlen(pURL);
    }
    return pURL;
  }

  void UpdateIPAddr(HTTPStackInterface* pHTTPStack)
  {
    if(!m_bIsIPAddressUpdated)
    {
      int size = 0;
      char* ipAddr = NULL;
      char dot = '.';
      ipAddr = pHTTPStack->GetIPAddr(size);
      if(m_pIPAddr)
      {
        QTV_Free(m_pIPAddr);
      }
      m_pIPAddr = (char*)QTV_Malloc((size_t)size+1);
      if(m_pIPAddr && ipAddr)
      {
        memset(m_pIPAddr,0,size+1);
        std_strlcpy(m_pIPAddr,ipAddr,(size_t)size+1);
        m_bIsIPAddressUpdated = true;
      }
      else
      {
        if(m_pIPAddr)
        {
          QTV_Free(m_pIPAddr);
        }
        m_pIPAddr = NULL;
      }
    }
  }
  virtual bool GetSegmentRange(uint64& nStartTime, uint64& nDuration)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetSegmentRange(nStartTime, nDuration) :
            false);
  };
  virtual bool IsDownloadComplete()
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->IsDownloadComplete() : false);
  }
  virtual bool IsDownloading()
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->IsDownloading() : false);
  };

  virtual bool IsSegErrorHappened()
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->IsSegErrorHappened() : false);
  };

  /**
   * Get the start time of the last data unit.
   */
  virtual bool GetStartTimeForLastDataUnit(uint64& nStartTime, double nRepEndTime, bool &bIsLmsgSet);

  virtual void MarkSegmentComplete()
  {
    HTTPDataManager *pDataManager = GetDataManager();
    if(pDataManager)
    {
      //Set end of file in datamanager to mark segment as complete
      pDataManager->SetEndofFile();
    }
  }

  /**
   * Mark that the resource reads have started on in. This means
   * that the fragment which has been detected as too slow cannot
   * be cancelled.
   */
  virtual void MarkResourceReadsStarted();

  /**
   * Check if reads have started on the resource. If not, then
   * disable the resource for sample reads and return true.
   *
   * @return bool
   *  true: if resource is disabled for sample reads.
   *  false: if resource is not disabled for sample reads.
   */
  virtual bool CheckReadsStartedAndDisableReads();

  /**
   * Check if this resource is disabled for sample reads.
   */
  virtual bool IsResourceReadDisabled() const;

  /**
   * Re-enable sample reads on the resource in case it was
   * disabled for sample reads.
   */
  virtual void ReEnableResourceReads();

  /**
   * Disable socket reads on the data unit depending on
   * 'bIsDisabled'.
   */
  virtual bool DisableSocketReads(const uint64 nDataUnitKey, bool bIsDisabled);

  /**
   * Check to see if data segment with key 'nKey' is present in
   * this resource.
   */
  virtual bool IsDataUnit(uint64 nKey) const;

  /**
   * Check to see if this data segment is available for download.
   */
  virtual bool IsSegmentAvailable()
  {
    uint64 nSegMPDKey = m_cSegmentInfo.getKey();
    return m_sDASHSessionInfo.cMPDParser.IsSegmentAvailable(nSegMPDKey);
  };

// iSegmentNotifier methods
public:
  virtual int GetBufferedDurationFromNotifier();

//iHTTPFileSourceHelper methods
public:
  virtual bool GetMinimumMediaOffset(uint64& nMinOffset)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->GetMinimumMediaOffset(nMinOffset) : false);
  };

  virtual bool GetOffsetForTime(const int64 nTime, int64& nOffset)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->GetOffsetForTime(nTime, nOffset) : false);
  };

  virtual bool GetTimeForOffset(HTTPMediaType eMediaType, uint64 nOffset, uint64& nTimeOffset)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->GetTimeForOffset(eMediaType, nOffset, nTimeOffset) : false);
  };

//iHTTPDataInterface methods
public:
  virtual bool GetDownloadPosition(HTTPCommon::HTTPMediaType eMediaType,
                                   uint64& nDownloadPosition,
                                   bool& bIsPartiallyDownloaded)
  {
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler
            ? pCurrentStateHandler->GetDownloadPosition(
              eMediaType, nDownloadPosition, bIsPartiallyDownloaded)
            : false);
  };

  virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample( HTTPCommon::HTTPMediaType majorType,
                                                             uint8 *pBuffer,
                                                             uint32 &nSize,
                                                             HTTPSampleInfo &sampleInfo)
  {
    uint32 cacheBufSize = nSize;
    HTTPCommon::HTTPDownloadStatus eStatus = video::HTTPCommon::HTTPDL_ERROR_ABORT;
    do
    {
      nSize = cacheBufSize;
      std_memset((void*)&sampleInfo, 0x0, sizeof(sampleInfo));
      eStatus = HTTPResource::GetNextMediaSample(majorType, pBuffer, nSize, sampleInfo);
    }while (video::HTTPCommon::HTTPDL_SUCCESS == eStatus && 0 == nSize);

    if(video::HTTPCommon::HTTPDL_WAITING == eStatus)
    {
      //check if already queued.
      SegmentCmdData cmd;
      void* pIt = NULL;
      MM_CriticalSection_Enter(m_pSegmentDataLock);
      while(m_cCmdQ.Next(pIt, cmd))
      {
        if (SEGMENT_CMD_GET_SEGDATA == cmd.sBaseCmdData.eCmd)
        {
          eStatus = HTTPCommon::HTTPDL_WAITING;
          break;
        }
      }
      MM_CriticalSection_Leave(m_pSegmentDataLock);

      if (HTTPCommon::HTTPDL_WAITING == eStatus)
      {
        return eStatus;
      }

      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "GetNextMediaSample WAITING and no cmd in Q, check for switch");

      // If no data download cmd pending in a Q, check for representation swich.
      eStatus = HTTPCommon::HTTPDL_SEGMENT_BOUNDARY;
    }
    //below use case is handled only for eMBMS
    else if (m_sDASHSessionInfo.sSessionInfo.IsEmbmsSession() &&
             video::HTTPCommon::HTTPDL_ERROR_ABORT == eStatus)
    {
      //If file source returned read error mark data end on this current segment.
      //Reads will continue from the next segment.

      uint64 nSegMPDKey = m_cSegmentInfo.getKey();
      SegmentCmdData* cmd;
      void *pIt = NULL;
      MM_CriticalSection_Enter(m_pSegmentDataLock);
      while(m_cCmdQ.Next(pIt,&cmd))
      {
        if(cmd->sGetSegDataCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA)
        {
          //Moving get segment data cmd state to cancelled
          if(cmd->sGetSegDataCmdData.eState != CMD_STATE_READ_ERROR)
          {
            cmd->sGetSegDataCmdData.eState = CMD_STATE_READ_ERROR;
          }

          QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "Seg [0x%08lx%08lx]: GetNextMediaSample  CMD_STATE_READ_ERROR (key %lu)"
              "state modified to read error", (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
              (uint32)cmd->sGetSegDataCmdData.nDataUnitKey);
          eStatus = HTTPCommon::HTTPDL_WAITING;
          break;
        }
      }
      MM_CriticalSection_Leave(m_pSegmentDataLock);

      if (HTTPCommon::HTTPDL_WAITING == eStatus)
      {
        return eStatus;
      }

      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "Seg [0x%08lx%08lx]: GetNextMediaSample DATA_END due to HTTPDL_ERROR_ABORT (key %lu)"
            "state modified to read error", (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey);

      HTTPResource::SetEndOfStream(majorType);

      eStatus = video::HTTPCommon::HTTPDL_DATA_END;
    }

    return eStatus;
  }

  virtual bool GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime)
  {
    segMediaTime = m_nStartTime;
    segMPDTime = m_cSegmentInfo.getStartTime();
    return true;
  }

private:
  //Segment handler states
  enum SegmentState
  {
    SEGMENT_STATE_IDLE,
    SEGMENT_STATE_PARSE_SIDX,
    SEGMENT_STATE_SETUP,
    SEGMENT_STATE_OPENING,
    SEGMENT_STATE_OPEN,
    SEGMENT_STATE_SEEKING,
    SEGMENT_STATE_ERROR
  };


  //Segment data download states
  enum DataDownloadState
  {
    DATA_DOWNLOAD_NOT_STARTED = 0,
    DATA_DOWNLOAD_STARTED,
    DATA_DOWNLOAD_PARTIAL_COMPLETE,
    DATA_DOWNLOAD_COMPLETE,
    DATA_DOWNLOAD_ERROR
  };

  enum SidxParseStatus
  {
    SIDX_PARSE_INIT,
    SIDX_PARSE_DONE,
    SIDX_PARSE_COMPLETE,
    SIDX_PARSE_FAIL,
    SIDX_PARSE_NOT_AVAIL,
    SIDX_PARSE_UNDER_RUN
  };

  //State handlers
  class SegmentBaseStateHandler
  {
  public:
    SegmentBaseStateHandler(SegmentState eState,
                            DASHMediaSegmentHandler* pSegHdlr)
      : m_eState(eState),
        m_pSegmentHandler(pSegHdlr)
    {
    };

    virtual ~SegmentBaseStateHandler()
    {
    };
    virtual void StateEntryHandler()
    {
      return;
    }
    virtual int ProcessCmds();

    virtual HTTPDownloadStatus GetSegmentInfo(uint64 /*nStartTime*/,
                                              uint64 /*nDuration*/)
    {
      return HTTPCommon::HTTPDL_SUCCESS;
    };

    virtual HTTPDownloadStatus OpenSegment();

    virtual void ClearBufferedData(uint64 nStartTime);

    virtual HTTPDownloadStatus Flush(HTTPCommon::HTTPMediaType majorType,
                                     int64 nStartTime);

    virtual HTTPDownloadStatus GetSegmentData(uint64 nDataUnitKey);

    virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64 nDataUnitKey,
                                                    uint64 &nStartTime,
                                                    uint64 &nDuration);

    virtual HTTPDownloadStatus CancelSegmentData(uint64 nDataUnitKey);
    virtual HTTPDownloadStatus ContinueDownloadDataUnit(uint64 nDataUnitKey);

    virtual HTTPDownloadStatus FillDataUnitInfo(uint64 nStartTime,
                                                uint64 nDuration,
                                                QSM::CDataUnitInfo* pDataUnitInfo,
                                                uint32 nNumDataUnits,
                                                uint32& nNumDataUnitsFilled);
    virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo *pDownloadInfo,
                                                       uint32 nSize,
                                                       uint32 &nFilled,
                                                       uint64 nStartTime);
    virtual HTTPDownloadStatus Select(uint64 nDataUnitKey, uint64& nPbTime);

    virtual HTTPDownloadStatus Seek(const int64 nSeekTime);

    virtual HTTPDownloadStatus IsReadable(HTTPMediaType /*majorType*/)
    {
      return HTTPCommon::HTTPDL_WAITING;
    };

    virtual HTTPDownloadStatus Close(HTTPMediaType majorType);

    virtual bool GetSegmentRange(uint64& nStartTime, uint64& nDuration);

    virtual bool IsDownloading()
    {
      return (m_pSegmentHandler->m_eDataDownloadState != DATA_DOWNLOAD_NOT_STARTED);
    };

    virtual void ProcessFileSourceEvent(FileSourceCallBackStatus /* status */)
    {
      //Nothing to process in this state
    };

    virtual bool IsDownloadComplete();

    virtual bool GetDownloadPosition(HTTPCommon::HTTPMediaType eMediaType,
                                     uint64& nDownloadPosition,
                                     bool& bIsPartiallyDownloaded);

    virtual bool IsSegErrorHappened()
    {
      return ((SEGMENT_STATE_ERROR == GetSegmentState()) || (DATA_DOWNLOAD_ERROR == m_pSegmentHandler->GetDataDownloadState()));
    };

    virtual bool GetMinimumMediaOffset(uint64& nMinOffset)
    {
      nMinOffset = 0;
      return true;
    };

    virtual bool GetOffsetForTime(const int64 nTime, int64& nOffset)
    {
      return m_pSegmentHandler->GetFileOffsetForTime(nTime, nOffset);
    };

    virtual bool GetTimeForOffset(HTTPMediaType eMediaType, uint64 nOffset, uint64& nTimeOffset)
    {
      return m_pSegmentHandler->GetTimeForFileOffset(eMediaType, nOffset, nTimeOffset);
    };

    virtual HTTPDownloadStatus Close();

    virtual SegmentState GetSegmentState()
    {
      return m_eState;
    };

    protected:
    SegmentState m_eState;
    DASHMediaSegmentHandler* m_pSegmentHandler;
  };

  class SegmentIdleStateHandler : public SegmentBaseStateHandler
  {
  public:
    SegmentIdleStateHandler(DASHMediaSegmentHandler* pSegHdlr)
      : SegmentBaseStateHandler(SEGMENT_STATE_IDLE, pSegHdlr)
    {
    };

    virtual ~SegmentIdleStateHandler()
    {
    };

    virtual int ProcessCmds();
    virtual HTTPDownloadStatus Close(HTTPMediaType /* majorType */)
    {
      return HTTPCommon::HTTPDL_SUCCESS;
    }
    virtual HTTPDownloadStatus GetSegmentInfo(uint64 nStartTime,
                                              uint64 nDuration);

    virtual HTTPDownloadStatus GetSegmentData(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64   /* nDataUnitKey */,
                                                    uint64 & /* nStartTime */,
                                                    uint64 & /* nDuration */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus CancelSegmentData(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    HTTPDownloadStatus ContinueDownloadDataUnit(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus FillDataUnitInfo(uint64              /* nStartTime */,
                                                uint64              /* nDuration */,
                                                QSM::CDataUnitInfo* /* pDataUnitInfo */,
                                                uint32 /* nNumDataUnits */,
                                                uint32& /* nNumDataUnitsFilled */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo * /*pDownloadInfo*/,
                                                       uint32 /* nSize */,
                                                       uint32 & /* nFilled */,
                                                       uint64 /* nStartTime */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Select(uint64 /* nDataUnitKey */,
                                      uint64& /* nPbTime */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual bool GetSegmentRange(uint64& /*nStartTime*/, uint64& /*nDuration*/)
    {
      return false;
    };

    virtual bool GetDownloadPosition(HTTPCommon::HTTPMediaType /* eMediaType */,
                                     uint64& nDownloadPosition,
                                     bool& bIsPartiallyDownloaded)
    {
      nDownloadPosition = 0;
      bIsPartiallyDownloaded = false;
      return false;
    };

    virtual bool GetMinimumMediaOffset(uint64& /* nMinOffset */)
    {
      return false;
    };

    virtual bool GetOffsetForTime(const int64 /* nTime */, int64& /* nOffset */)
    {
      return false;
    };

    virtual bool GetTimeForOffset(HTTPMediaType /* eMediaType */, uint64 /* nOffset */, uint64& /* nTimeOffset */)
    {
      return false;
    };
  };

  class SegmentInfoRetryHandler : public SegmentBaseStateHandler
  {
  public:
    SegmentInfoRetryHandler(DASHMediaSegmentHandler* pSegHdlr)
      : SegmentBaseStateHandler(SEGMENT_STATE_IDLE, pSegHdlr)
    {
    };

    virtual ~SegmentInfoRetryHandler()
    {
    };

    virtual int ProcessCmds();
    virtual HTTPDownloadStatus Close(HTTPMediaType /* majorType */)
    {
      return HTTPCommon::HTTPDL_SUCCESS;
    }
    virtual HTTPDownloadStatus GetSegmentInfo(uint64 /* nStartTime */,
                                              uint64 /* nDuration */)
    {
      return HTTPCommon::HTTPDL_WAITING;
    };

    virtual HTTPDownloadStatus GetSegmentData(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64 /* nDataUnitKey */,
                                                    uint64 /* &nStartTime */,
                                                    uint64 /* &nDuration */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus CancelSegmentData(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    HTTPDownloadStatus ContinueDownloadDataUnit(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus FillDataUnitInfo(uint64 /* nStartTime */,
                                                uint64 /* nDuration */,
                                                QSM::CDataUnitInfo* /* pDataUnitInfo */,
                                                uint32 /* nNumDataUnits */,
                                                uint32& /* nNumDataUnitsFilled */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo * /* pDownloadInfo*/,
                                                       uint32  /* nSize*/,
                                                       uint32 & /* nFilled*/,
                                                       uint64  /* nStartTime */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Select(uint64 /* nDataUnitKey */,
                                      uint64& /* nPbTime */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual bool GetSegmentRange(uint64& /* nStartTime */, uint64& /* nDuration */)
    {
      return false;
    };

    virtual bool GetDownloadPosition(HTTPCommon::HTTPMediaType /* eMediaType */,
                                     uint64& nDownloadPosition,
                                     bool& bIsPartiallyDownloaded)
    {
      nDownloadPosition = 0;
      bIsPartiallyDownloaded = false;
      return false;
    };

    virtual bool GetMinimumMediaOffset(uint32& /* nMinOffset */)
    {
      return false;
    };

    virtual bool GetOffsetForTime(const int64 /* nTime */, int64& /* nOffset */)
    {
      return false;
    };

    virtual bool GetTimeForOffset(HTTPMediaType /* eMediaType */, uint64 /* nOffset */, uint64& /* nTimeOffset */)
    {
      return false;
    };
  };

  class SegmentParseSidxStateHandler : public SegmentBaseStateHandler
  {


  public:
    SegmentParseSidxStateHandler(DASHMediaSegmentHandler* pSegHdlr)
      : SegmentBaseStateHandler(SEGMENT_STATE_PARSE_SIDX, pSegHdlr),
      m_SidxParseStatus(SIDX_PARSE_INIT),
      m_LevelOneComplete(false)
    {

    };

    virtual ~SegmentParseSidxStateHandler()
    {
    };

    virtual int ProcessCmds();
    virtual HTTPDownloadStatus Close(HTTPMediaType /* majorType */)
    {
      return HTTPCommon::HTTPDL_SUCCESS;
    }
    virtual bool GetSegmentRange(uint64& /* nStartTime */, uint64& /* nDuration */)
    {
      return false;
    };
    virtual HTTPDownloadStatus GetSegmentInfo(uint64 /* nStartTime */,
                                              uint64 /* nDuration */)
    {
      return HTTPCommon::HTTPDL_WAITING;
    };
    virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo * /* pDownloadInfo */,
                                                       uint32 /* nSize */,
                                                       uint32 & /* nFilled */,
                                                       uint64 /* nStartTime */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual void StateEntryHandler()
    {
      m_SidxParseStatus = SIDX_PARSE_INIT;
      m_LevelOneComplete = false;
      return;
    }


    HTTPDownloadStatus Parse_Sidx();

    HTTPDownloadStatus DiscardExtraDataAfterSidx(SegmentDownloader* pDownloader);

    SidxParseStatus m_SidxParseStatus;

    bool m_LevelOneComplete;

  };


  class SegmentSetupStateHandler : public SegmentBaseStateHandler
  {

  public:
    SegmentSetupStateHandler(DASHMediaSegmentHandler* pSegHdlr)
      : SegmentBaseStateHandler(SEGMENT_STATE_SETUP, pSegHdlr)
    {

    };

    virtual ~SegmentSetupStateHandler()
    {
    };

    virtual int ProcessCmds();

  };

  class SegmentOpeningStateHandler : public SegmentBaseStateHandler
  {
  public:
    SegmentOpeningStateHandler(DASHMediaSegmentHandler* pSegHdlr)
      : SegmentBaseStateHandler(SEGMENT_STATE_OPENING, pSegHdlr)
    {
    };

    virtual ~SegmentOpeningStateHandler()
    {
    };

    virtual HTTPDownloadStatus OpenSegment()
    {
      return HTTPCommon::HTTPDL_WAITING;
    };

    virtual void ProcessFileSourceEvent(FileSourceCallBackStatus status);
  };

  class SegmentOpenStateHandler : public SegmentBaseStateHandler
  {
  public:
    SegmentOpenStateHandler(DASHMediaSegmentHandler* pSegHdlr)
      : SegmentBaseStateHandler(SEGMENT_STATE_OPEN, pSegHdlr)
    {
    };

    virtual ~SegmentOpenStateHandler()
    {
    };

    virtual void StateEntryHandler();

    virtual HTTPDownloadStatus OpenSegment()
    {
      return HTTPCommon::HTTPDL_SUCCESS;
    };

    virtual HTTPDownloadStatus Seek(const int64 nSeekTime);

    virtual HTTPDownloadStatus IsReadable(HTTPMediaType majorType)
    {
      return m_pSegmentHandler->IsResourceReadable(majorType);
    };

    virtual bool GetMinimumMediaOffset(uint64& nMinOffset)
    {
      return m_pSegmentHandler->GetMinimumMediaFileOffset(nMinOffset);
    };
  };

  class SegmentSeekingStateHandler : public SegmentBaseStateHandler
  {
  public:
    SegmentSeekingStateHandler(DASHMediaSegmentHandler* pSegHdlr)
      : SegmentBaseStateHandler(SEGMENT_STATE_SEEKING, pSegHdlr)
    {
    };

    virtual ~SegmentSeekingStateHandler()
    {
    };

    virtual HTTPDownloadStatus OpenSegment()
    {
      return HTTPCommon::HTTPDL_SUCCESS;
    };

    virtual void ProcessFileSourceEvent(FileSourceCallBackStatus status);

    virtual HTTPDownloadStatus Select(uint64 /* nDataUnitKey */,
                                      uint64& /* nPbTime */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Seek(const int64 /* nSeekTime */)
    {
      return HTTPCommon::HTTPDL_WAITING;
    };

    virtual int ProcessCmds();
  };

  class SegmentErrorStateHandler : public SegmentBaseStateHandler
  {
  public:
    SegmentErrorStateHandler(DASHMediaSegmentHandler* pSegHdlr)
      : SegmentBaseStateHandler(SEGMENT_STATE_ERROR, pSegHdlr)
    {
    };

    virtual ~SegmentErrorStateHandler()
    {
    };

    virtual int ProcessCmds()
    {
      return -1;
    }

    virtual HTTPDownloadStatus GetSegmentInfo(uint64 /* nStartTime */,
                                              uint64 /* nDuration */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus OpenSegment()
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetSegmentData(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64 /* nDataUnitKey */,
                                                    uint64 &/* nStartTime */,
                                                    uint64 &/* nDuration */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus CancelSegmentData(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus ContinueDownloadDataUnit(uint64 /* nDataUnitKey */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus Close(HTTPMediaType /* majorType */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus FillDataUnitInfo(uint64 /* nStartTime */,
                                                uint64 /* nDuration */,
                                                QSM::CDataUnitInfo* /* pDataUnitInfo */,
                                                uint32 /* nNumDataUnits */,
                                                uint32& /* nNumDataUnitsFilled */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Select(uint64 /* nDataUnitKey */,
                                      uint64& /* nPbTime */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus IsReadable(HTTPMediaType /* majorType */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual bool GetSegmentRange(uint64& nStartTime, uint64& nDuration)
    {
      nStartTime = m_pSegmentHandler->m_nStartTime;
      nDuration = m_pSegmentHandler->m_nDuration;
      return false;
    };

    virtual bool GetDownloadPosition(HTTPCommon::HTTPMediaType /* eMediaType */,
                                     uint64& nDownloadPosition,
                                     bool& bIsPartiallyDownloaded)
    {
      nDownloadPosition = 0;
      bIsPartiallyDownloaded = false;
      return false;
    };
    virtual bool IsDownloadComplete()
    {
      return true;
    }

    virtual bool GetMinimumMediaOffset(uint64& /* nMinOffset */)
    {
      return false;
    };

    virtual bool GetOffsetForTime(const int64 /* nTime */, int64& /* nOffset */)
    {
      return false;
    };

    virtual bool GetTimeForOffset(HTTPMediaType /* eMediaType */, uint64 /* nOffset */, uint64& /* nTimeOffset */)
    {
      return false;
    };
  };

  struct SegmentCmdBaseResponseData
  {
    SegmentCmd eCmd;
    uint32 nSegKey;
    HTTPDownloadStatus eStatus;
  };

  struct SegmentGetSegInfoCmdResponseData : public SegmentCmdBaseResponseData
  {
    uint32 nNumDataUnits;
  };

  struct SegmentGetSegDataCmdResponseData : public SegmentCmdBaseResponseData
  {
    uint64 nDataUnitKey;
  };

  //Segment cmd response data
  union SegmentCmdResponseData
  {
    SegmentCmdBaseResponseData sCmdBaseResponseData;
    SegmentGetSegInfoCmdResponseData sGetSegInfoCmdResponseData;
    SegmentGetSegDataCmdResponseData sGetSegDataCmdResponseData;
  };

  enum GetSegDataCmdState
  {
    CMD_STATE_INIT,
    CMD_STATE_DOWNLOADING,
    CMD_STATE_RETRY,
    CMD_STATE_CANCELLED,
    CMD_STATE_READ_ERROR
  };

  //Segment base cmd data - to be dequeued by TaskMediaSegment
  struct SegmentBaseCmdData
  {
    uint32 nCmdId;
    SegmentCmd eCmd;
    SegmentDownloader* pDownloader;
    uint32 nLastFailureTime;
  };

  //Segment GET_SEGINFO cmd data
  struct SegmentGetSegInfoCmdData : public SegmentBaseCmdData
  {
    uint64 nStartTime;
    uint64 nDuration;
  };

  //Segment CHECK_DATA_AVAIL_FOR_SEEK cmd data
  struct SegmentCheckCmdData : public SegmentBaseCmdData
  {
    uint64 nCurrDataOffset;
  };

  struct SegmentGetSegData: public SegmentBaseCmdData
  {
    uint64 nDataUnitKey;
    GetSegDataCmdState eState;
    bool bIsRetrying;
  };

  //Segment cmd data
  union SegmentCmdData
  {
    SegmentBaseCmdData sBaseCmdData;
    SegmentGetSegInfoCmdData sGetSegInfoCmdData;
    SegmentCheckCmdData sCheckCmdData;
    SegmentGetSegData sGetSegDataCmdData;
  };

  //Segment download manager to manage the downloader pool
  class SegmentDownloaderManager
  {
  public:
    SegmentDownloaderManager() :m_aDownloader(NULL),
                                m_nSegmentDownloaders(0),
                                m_pHTTPStack(NULL) { };
    ~SegmentDownloaderManager();

    //Initialize downloader pool
    bool Init(HTTPSessionInfo* pSessionInfo,
              HTTPStackInterface* pHTTPStack);

    //Get downloader based on key
    SegmentDownloader* GetSegmentDownloader(const uint64 nDataUnitKey)
    {
      SegmentDownloader* pDownloader = NULL;
      if(m_aDownloader)
      {
        for (uint32 i = 0; i < m_nSegmentDownloaders; i++)
        {
          if (m_aDownloader[i].GetKey() == nDataUnitKey)
          {
            pDownloader = &m_aDownloader[i];
            break;
          }
        }
      }
      return pDownloader;
    };

    //Find next available downloader
    SegmentDownloader* GetNextAvailableSegmentDownloader()
    {
      SegmentDownloader* pDownloader = NULL;
      if(m_aDownloader)
      {
        for (uint32 i = 0; i < m_nSegmentDownloaders; i++)
        {
          if (!m_aDownloader[i].IsInUse())
          {
            pDownloader = &m_aDownloader[i];
            break;
          }
        }
      }
      return pDownloader;
    };

    //Close downloader pool
    void Close()
    {
      for (uint32 i = 0; i < m_nSegmentDownloaders; i++)
      {
        (void)m_aDownloader[i].Stop();
      }
    };

  private:
    SegmentDownloader* m_aDownloader;
    uint32 m_nSegmentDownloaders;
    HTTPStackInterface* m_pHTTPStack;
  };

  //Segment command task param
  class SegmentCmdTaskParam : public SchedulerTaskParamBase
  {
  public:
    SegmentCmdTaskParam(void* pParam) : pSelf(pParam){ };
    virtual ~SegmentCmdTaskParam(){ };

    void* pSelf;
  };

  void SetStateHandler(SegmentBaseStateHandler* pCurrStateHandler)
  {
    MM_CriticalSection_Enter(m_pSegmentDataLock);
    m_pCurrentStateHandler = pCurrStateHandler;

    if(m_pCurrentStateHandler)
    {
      m_pCurrentStateHandler->StateEntryHandler();
    }

    MM_CriticalSection_Leave(m_pSegmentDataLock);
  }

  SegmentBaseStateHandler* GetStateHandler()
  {
    SegmentBaseStateHandler* pCurrStateHandler = NULL;
    MM_CriticalSection_Enter(m_pSegmentDataLock);
    pCurrStateHandler = m_pCurrentStateHandler;
    MM_CriticalSection_Leave(m_pSegmentDataLock);
    return pCurrStateHandler;
  }

  //Segment task
  static int TaskMediaSegment(void* pParam);
  HTTPDownloadStatus ProcessGetSegmentDataCmd(SegmentCmdData& cmd);
  HTTPDownloadStatus ProcessGetSegmentDataInit(SegmentCmdData& cmd);
  HTTPDownloadStatus ProcessGetSegmentDataDownload(SegmentCmdData& cmd);
  HTTPDownloadStatus ProcessGetSegmentDataRetry(SegmentCmdData& cmd);
  HTTPDownloadStatus ProcessCancelSegmentData(uint64 nDataUnitKey, bool bStopTimer = true);
  HTTPDownloadStatus ProcessGetSegmentInfoCmd(uint64 nStartTime,uint64 nDuration);
  HTTPDownloadStatus Download(SegmentDownloader* pDownloader, HTTPDataManager* pDataManager);

  /**
   * Tie the Download status to a cmd id if needed by overriding
   * the args.
   */
  void ClearCmdQ(HTTPDownloadStatus eCmdIdStatus = HTTPCommon::HTTPDL_ERROR_ABORT,
                 uint32 nCmdId = MAX_UINT32_VAL);

  /**
   * Tie the Download status to a cmd id if needed by overriding
   * the args.
   */
  void OnError(const HTTPDownloadStatus eStatus, uint32 nCmdID = MAX_UINT32_VAL);

  bool SendSegmentInfoRequest(SegmentDownloader* pDownloader);

  /**
   * Asociate each cmd with a unique id (assuming that there
   * cannot be MAX_UINT32 commands active !)
   */
  bool EnQCmd(SegmentCmdData& rCmd);


  void SetDataDownloadState(const DataDownloadState eNewState)
  {
    m_eDataDownloadState = eNewState;
  };

  DataDownloadState GetDataDownloadState()
  {
    return m_eDataDownloadState;
  };
  void ResetDataDownloadState()
  {
    m_eDataDownloadState = DATA_DOWNLOAD_NOT_STARTED;
  };

  HTTPDownloadStatus CloseMedia(const HTTPMediaType eMediaType)
  {
    HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    MM_CriticalSection_Enter(m_pSegmentFSLock);
    SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      eStatus = pCurrentStateHandler->Close(eMediaType);
    }
    MM_CriticalSection_Leave(m_pSegmentFSLock);
    if (eStatus == HTTPCommon::HTTPDL_SUCCESS)
    {
      eStatus = HTTPResource::Close(eMediaType);
    }

    return eStatus;
  };

  void DownloadEnded(SegmentDownloader* pDownloader)
  {
    //Stop the downloader (keep connection alive as an optimization)
    if (pDownloader)
    {
      (void)pDownloader->Stop();
    }
  };

  bool StartDownload(SegmentDownloader* pDownloader,
                     const uint64 nKey,
                     const int64 nStartOffset,
                     const int64 nEndOffset,
                     char* reqURL,
                     const int nDurationMs = -1);

  bool ConstructByteRangeURL(char* origURL, char* templateStr, int64 nStartOffset, int64 nEndOffset);


  //Session and segment info
  DASHSessionInfo& m_sDASHSessionInfo;
  SegmentInfo m_cSegmentInfo;

  iRepresentationNotifier* m_pRepNotifier;

  //Initialization segment
  HttpSegmentDataStoreBase* m_pInitSegment;

  /**
   * The SidxFetchDecision object for this representation.
   */
  SidxFetchDecision& m_rSidxFetchDecision;

  //Lock to synchronize data access
  MM_HANDLE m_pSegmentDataLock;

   //Lock to synchronize fs access
  MM_HANDLE m_pSegmentFSLock;

  //Segment download manager
  SegmentDownloaderManager m_cSegDownloadMgr;

  //Sidx parser
  ::sidxparser* m_pSidxParser;

  //Segment state handlers
  SegmentIdleStateHandler m_cIdleStateHandler;
  SegmentParseSidxStateHandler m_cParseSidxStateHandler;
  SegmentSetupStateHandler m_cSetupStateHandler;
  SegmentOpeningStateHandler m_cOpeningStateHandler;
  SegmentOpenStateHandler m_cOpenStateHandler;
  SegmentSeekingStateHandler m_cSeekingStateHandler;
  SegmentErrorStateHandler m_cErrorStateHandler;
  SegmentInfoRetryHandler m_cSegInfoRetryStateHandler;

  SegmentBaseStateHandler* m_pCurrentStateHandler;

  //Segment cmd queues
  StreamDataQ<SegmentCmdData> m_cCmdQ;

  // unique id associated with each command in the Q
  uint32 m_NextCmdID;

  //Temporary cache, so as to avoid recreating the segmentinfo array
  HTTPSegmentsInfo m_httpSegmentsInfo;
  uint64 m_nStartTime;
  uint64 m_nDuration;
  int m_nIndexURLPresent;
  bool m_bInitAndMediaWithSameURL;
  char* m_pIPAddr;
  HTTPStackInterface* m_pHTTPStack;
  Scheduler *m_pScheduler;
  int m_nTaskID;
  DataDownloadState m_eDataDownloadState;
  bool m_bProcessOpenCmd;

  // set to true when 'lmsg' is set for the segment.
  bool m_bIsLmsgSet;

  // once this is true there is no dataunit within the resource that can
  // be cancelled. This is used to allow cancels on a fragment in a resource
  // which is not yet readable.
  bool m_bIsReadsStarted;

  /**
   * Sample reads not allowed if this set.
   */
  bool m_bIsResourceReadDisabled;
  bool m_bIsIPAddressUpdated;
  StreamSourceClock *m_pClock;
  int m_numRetriesExcerised;

  /**
    * In seperate sidx (index url) case, Assuming media URL will always start with zero ofset,
    * as parser provides media offsets from end of the sidx. so adjust offsets for all media
    * data unit requests
    */
  int64 m_offsetAdjust;
};

} // namespace video

#endif //__DASHMEDIASEGMENTHANDLER_H__
