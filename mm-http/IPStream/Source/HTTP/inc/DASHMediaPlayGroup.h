#ifndef __DASHMEDIAPLAYGROUP_H__
#define __DASHMEDIAPLAYGROUP_H__
/************************************************************************* */
/**
 * DASHMediaPlayGroup.h
 * @brief Defines the DASHMediaPlayGroup interface.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/DASHMediaPlayGroup.h#51 $
$DateTime: 2013/07/27 08:03:59 $
$Change: 4174247 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "Streamlist.h"
#include "DASHMediaRepresentationHandler.h"
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
#define HTTP_MAX_MEDIA_COMPONENTS       3
#define HTTP_MAX_DATA_UNITS             20
#define SEEK_TOLERANCE_THREASHOLD       100
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
//Period notification handler interface
class iPeriodNotifier
{
public:

  virtual ~iPeriodNotifier() { };
  virtual void SegInfoReady(const uint64 nGroupKey,
                            const uint64 nRepKey,
                            const uint64 nStartTime,
                            const uint64 nDuration,
                            const uint32 nNumberofUnits,
                            const HTTPDownloadStatus eStatus) = 0;
  virtual void SegDataReady(const uint64 nGroupKey,
                            const uint64 nRepKey,
                            const uint64 nDataUnitKey,
                            const HTTPDownloadStatus eStatus) = 0;
  virtual void NotifyError(const uint64 nGroupKey,
                           const HTTPDownloadStatus eStatus) = 0;

  virtual void NotifySeekStatus(uint64 nGroupKey,
                                int64 nSeekEndTime,
                                HTTPDownloadStatus eStatus)=0;

  virtual void NotifyDownloadTooSlow(const uint64 nGroupKey,
                                     const uint32 nRepKey,
                                     const uint64 nDataUnitKey) = 0;
};

//Switch element
struct SwitchElement
{
  int nRepKey;
  uint64 nDataUnitKey;
  uint64 nSwitchTime;
};

//Representation
class Representation
{
public:
  Representation(): m_pRepHandler(NULL)
  {
  };
  ~Representation()
  {
    if (m_pRepHandler)
    {
      QTV_Delete(m_pRepHandler);
      m_pRepHandler = NULL;
    }
  };

  RepresentationInfo m_cRepInfo;
  DASHMediaRepresentationHandler* m_pRepHandler;
};

class DASHMediaPlayGroup : public iGroupNotifier
{
public:
  DASHMediaPlayGroup() : m_nMajorType(QSM::MAJOR_TYPE_UNKNOWN),
                         m_nKey(-1),
                         m_pDASHSessionInfo(NULL),
                         m_nNumReps(0),
                         m_pRepresentation(NULL),
                         m_pPeriodNotifier(NULL),
                         m_pScheduler(NULL),
                         m_pHTTPStack(NULL),
                         m_pGroupDataLock(NULL),
                         m_nStartTime(INVALID_START_TIME),
                         m_nSeekTime(-1),
                         m_bSeekPending(false),
                         m_bDownloadComplete(false),
                         m_nSeekEndTime(-1),
                         m_nSeekCurTime(-1),
                         m_eSeekStatus(HTTPCommon::HTTPDL_SUCCESS),
                         m_nNumMediaComponents(0),
                         m_bInvalidGroup(false),
                         m_IslastDataDownloadFailed(false),
                         m_IsReadEos(false),
                         m_pCommonInitSegment(NULL)
  {
  };

  virtual ~DASHMediaPlayGroup()
  {
    DeInit();
  };

  bool Init(const uint32 nKey,
            const uint32 nMajorType,
            RepresentationGroup& cRepGroup,
            DASHSessionInfo* pDASHSessionInfo,
            iPeriodNotifier* pNotifier,
            Scheduler* pScheduler = NULL);

  void DeInit();

  uint64 GetKey()
  {
    return m_nKey;
  };

  bool IsSwitchable() const
  {
    return (m_nMajorType & QSM::MAJOR_TYPE_VIDEO ||
           (m_nMajorType & QSM::MAJOR_TYPE_AUDIO && m_pDASHSessionInfo->sSessionInfo.IsAudioSwitchable()));
  };

  HTTPDownloadStatus GetRepresentationInfo(QSM::CRepresentationInfo* pRepInfo,
                                           uint32 nSizeOfRepInfo,
                                           uint32& nNumRepInfo);

  HTTPDownloadStatus GetSegmentInfo(uint64 nRepKey,
                                    uint64 nStartTime,
                                    uint64 nDuration);

  HTTPDownloadStatus FillDataUnitInfo(uint64 nRepKey,
                                      uint64 nStartTime,
                                      uint64 nDuration,
                                      QSM::CDataUnitInfo* pDataUnitInfo,
                                      uint32 nSizeOfDataUnitInfo,
                                      uint32& nNumDataUnitInfo);

  HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo *pDownloadInfo,
                                             uint32 nSize,
                                             uint32 &nFilled,
                                             uint64 nStartTime);

  HTTPDownloadStatus GetSegmentData(uint64 nRepKey,
                                    uint64 nDataUnitKey);

  HTTPDownloadStatus CancelSegmentData(uint64 nRepKey,
                                    uint64 nDataUnitKey);

  /**
   * Continue downloading the data unit that was indicated as too
   * slow to QSM.
   */
  HTTPDownloadStatus ContinueDownloadDataUnit(uint64 nRepresentationKey, uint64 nKey);

  HTTPDownloadStatus Select(uint64 nRepKey, uint64 nDataUnitKey);

  bool IsReadable(bool bCheckDataReadableOnly = false);
  void DownloadCommonInit();
  InitializationSegment::InitSegState CommonInitDwldState();
  HTTPDownloadStatus Download();

  bool IsValid()
  {
    bool bValid = true;
    MM_CriticalSection_Enter(m_pGroupDataLock);
    bValid = !m_bInvalidGroup;
    MM_CriticalSection_Leave(m_pGroupDataLock);
    return bValid;
  }

  void SetStartTime(const uint64 nStartTime)
  {
    MM_CriticalSection_Enter(m_pGroupDataLock);
    m_nStartTime = nStartTime;
    MM_CriticalSection_Leave(m_pGroupDataLock);
    return;
  };
  uint64 GetStartTime()
  {
    uint64 nStartTime;
    MM_CriticalSection_Enter(m_pGroupDataLock);
    nStartTime = m_nStartTime;
    MM_CriticalSection_Leave(m_pGroupDataLock);
    return nStartTime;
  }

  HTTPDownloadStatus Close();

  void GroupRequestsCompleted(const uint64 nEndTime);

  bool GetSeekStatus();
  void CheckAndHandleSwitchDuringSeek();

  bool CanPlaybackUninterrupted();

//iGroupNotifier methods
public:
  virtual void SegInfoReady(const uint64 nRepKey,
                            const uint64 nStartTime,
                            const uint64 nDuration,
                            const uint32 nNumDataUnits,
                            const HTTPDownloadStatus eStatus)
  {
    if (m_pPeriodNotifier)
    {
      m_pPeriodNotifier->SegInfoReady(m_nKey, nRepKey, nStartTime,
                                      nDuration, nNumDataUnits, eStatus);
    }
  };

  virtual void SegDataReady(const uint64 nRepKey,
                            const uint64 nDataUnitKey,
                            const HTTPDownloadStatus eStatus);

  virtual void NotifyError(const uint64 /* nRepKey */,
                           const HTTPDownloadStatus eStatus)
  {
    //Representation with key nRepKey notified error
    if (m_pPeriodNotifier)
    {
      m_pPeriodNotifier->NotifyError(m_nKey, eStatus);
    }
  };
  virtual void NotifySeekStatus(uint64 nRepKey,
                                int64 nSeekEndTime,
                                int64 nCurTime,
                                HTTPDownloadStatus eStatus);

  virtual void NotifyDownloadTooSlow(const uint64 nRepKey,
                                     const uint64 nDataUnitKey);

  virtual int GetBufferedDurationFromNotifier();

//HTTPDataInterface methods
public:
  virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo *pTrackInfo );

  virtual HTTPDownloadStatus GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType majorType,
                                                       HTTPMediaTrackInfo &TrackInfo );

  virtual HTTPDownloadStatus GetCurrentKeys(HTTPCommon::HTTPMediaType majorType,
                                            int &nCurrGrpKey,
                                            int &nRepKey);

  virtual HTTPDownloadStatus GetCodecData(uint32 nTrackID,
                                          HTTPCommon::HTTPMediaType majorType,
                                          HTTPMediaMinorType minorType,
                                          HTTPCodecData &CodecData);

  virtual HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType majorType,
                                            uint8* pBuffer,
                                            uint32 &nbufSize);

  virtual HTTPDownloadStatus GetNextMediaSample(HTTPCommon::HTTPMediaType majorType,
                                                uint8 *pBuffer,
                                                uint32 &nSize,
                                                HTTPSampleInfo &sampleInfo);

  void GetVideoInfo(HTTPCommon::HTTPMediaType majorType, char* pVideoURL,size_t& nURLSize, char* pIPAddr, size_t& nIPAddrSize)
  {
    MediaReadStore* pReadStore = GetMediaReadStore(majorType);
    if (pReadStore)
    {
      int nRepKey = pReadStore->GetCurrentRepresentation();
      DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nRepKey);
      if(pRepHandler)
      {
        pRepHandler->GetVideoInfo(majorType,pVideoURL,nURLSize,pIPAddr,nIPAddrSize);
      }
    }
  };

  virtual bool GetCurrentPlaybackPosition(HTTPCommon::HTTPMediaType mediaType,
                                          uint64 &nPlaybackPosition);

  virtual bool GetDurationBuffered(HTTPCommon::HTTPMediaType mediaType,
                                   uint64 &nPlaybackPosition,
                                   uint64 &nBufferedDuration);

  virtual bool GetGroupPlaybackStats(HTTPCommon::HTTPMediaType mediaType,
                                     uint32& nFullyDownloadedOccupancy,
                                     uint32& nPartiallyDownloadedOccupancy,
                                     uint32& nTotalOccupancy,
                                     uint64& nPlaybackPosition,
                                     uint32& nForwardMediaAvailability);

  bool GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime);

  HTTPDownloadStatus Seek(const int64 nSeekTime);

  void SetSeekPending(bool bSeekPending)
  {
    MM_CriticalSection_Enter(m_pGroupDataLock);
    m_bSeekPending = bSeekPending;
    MM_CriticalSection_Leave(m_pGroupDataLock);

  }

  bool IsSeekPending()
  {
    if(m_pCommonInitSegment)
    {
      InitializationSegment::InitSegState m_eState = m_pCommonInitSegment->getInitState();
      if(InitializationSegment::INITSEG_STATE_DOWNLOADING == m_eState)
      {
        MM_CriticalSection_Enter(m_pGroupDataLock);
        DownloadCommonInit();
        MM_CriticalSection_Leave(m_pGroupDataLock);
      }
    }

    bool bSeekPending = false;
    MM_CriticalSection_Enter(m_pGroupDataLock);
    bSeekPending = m_bSeekPending;
    MM_CriticalSection_Leave(m_pGroupDataLock);
    return bSeekPending;
  }

  HTTPDownloadStatus Flush(const int64 nTime);

  bool IsDownloadComplete()
  {
    bool bResult = false;
    MM_CriticalSection_Enter(m_pGroupDataLock);
    bResult = m_bDownloadComplete;
    MM_CriticalSection_Leave(m_pGroupDataLock);
    return bResult;
  };
  void SetDownloadComplete(const bool bComplete = true)
  {
    MM_CriticalSection_Enter(m_pGroupDataLock);
    m_bDownloadComplete = bComplete;
    MM_CriticalSection_Leave(m_pGroupDataLock);
    return;
  };

  bool IsStartTimeInitialized() const
  {
    return (m_nStartTime == INVALID_START_TIME ? false : true);
  }

  void ClearBufferedData(uint64 nTime);

  void RemoveDataUnitFromStats(uint64 nRepKey,
                               uint64 nFirstDataUnitKey,
                               uint64 nLastRemovedDataUnitKey);

  uint32 GetQsmMajorType() const;

  bool GetDataUnitStartTime(uint64 nRepKey,
                            uint64 nDataUnitKey,
                            uint64& nPbTime);

  uint64 GetRepGrpInfoKey();

  virtual void NotifyDataUnitRemoved(const uint64 nRepKey,
   const uint64 nFirstRemovedDataUnitKey,
   const uint64 nLastRemovedDataUnitKey);

//helper functions
public:
  //Representation group info
  uint32 m_nMajorType;
  RepresentationGroup m_cRepGroupInfo;
  /*Bandwidth Estimator object to record amount of
    data downloaded and total time taken.*/
  HTTPBandwidthEstimator *m_bEstimator;

private:
  uint32 m_nKey;
  DASHSessionInfo* m_pDASHSessionInfo;

  //Representations
  int m_nNumReps;
  Representation* m_pRepresentation;

  iPeriodNotifier* m_pPeriodNotifier;
  Scheduler* m_pScheduler;
  HTTPStackInterface* m_pHTTPStack;

  //Lock to synchronize data access
  MM_HANDLE m_pGroupDataLock;

  static const uint64 INVALID_START_TIME = MAX_UINT64;
  uint64 m_nStartTime;
  int64 m_nSeekTime;
  bool m_bSeekPending;
  bool m_bDownloadComplete;
  int64 m_nSeekEndTime;
  int64 m_nSeekCurTime;
  HTTPDownloadStatus m_eSeekStatus;
  void SetRepresentationHandler(const int nRepKey,DASHMediaRepresentationHandler* pRepHandler)
  {
    MM_CriticalSection_Enter(m_pGroupDataLock);
    m_pRepresentation[nRepKey].m_pRepHandler = pRepHandler;
    MM_CriticalSection_Leave(m_pGroupDataLock);
    return;
  }
  DASHMediaRepresentationHandler* GetRepresentationHandler(const int nRepKey)
  {
    DASHMediaRepresentationHandler* pRepHandler = NULL;
    MM_CriticalSection_Enter(m_pGroupDataLock);
    if (nRepKey >= 0 && nRepKey < m_nNumReps &&
        m_pRepresentation && m_pRepresentation[nRepKey].m_pRepHandler)
    {
      pRepHandler = m_pRepresentation[nRepKey].m_pRepHandler;
    }
    MM_CriticalSection_Leave(m_pGroupDataLock);
    return pRepHandler;
  };

  enum MediaReadState
  {
    INIT,
    READING
  };

  // Data Unit element state
  enum DataUnitState{
    DOWNLOAD_ERROR,
    DOWNLOAD_PARTIAL,
    DOWNLOAD_COMPLETE
  };

  // A queue element (stored in Queue) for a data unit
  struct HTTPDataUnitElement
  {
    void Reset();
    ordered_StreamList_link_type link;
    uint64 nRepKey;
    uint64 nDataUnitKey;
    uint64 nStartTime;
    uint32 nDuration;
    uint64 nPrevPlayPos;
    DataUnitState state;
  };

  //Media read store (one per media component i.e. A or V)
  class MediaReadStore
  {
  public:
    MediaReadStore() : m_dataUnits(NULL),
                       nFullyDownloadedOccupancy(0),
                       nPartiallyDownloadedOccupancy(0),
                       nCurrentDownloadPosition(0),
                       m_dataUnitsLock(NULL),
                       m_pStoreDataLock(NULL),
                       m_eMediaType(HTTPCommon::HTTP_UNKNOWN_TYPE),
                       m_nCurrentRepKey(-1),
                       m_eReadState(INIT),
                       m_nPlaybackPos(0),
                       m_nPrevMaxPlaybackPos(0)
    {
      (void)MM_CriticalSection_Create(&m_pStoreDataLock);

      // Use a value of 50 assuming max buffered duration is 40s on QSM and worst
      // case is one sec fragment duration.
      m_cSwitchQ.Init(50);
    };

    ~MediaReadStore()
    {
      if (m_pStoreDataLock)
      {
        (void)MM_CriticalSection_Release(m_pStoreDataLock);
        m_pStoreDataLock = NULL;
      };
    };

    void SetMediaType(const HTTPCommon::HTTPMediaType eMediaType)
    {
      m_eMediaType = eMediaType;
    };

    HTTPCommon::HTTPMediaType GetMediaType()
    {
      return m_eMediaType;
    };

    void SetCurrentRepresentation(const int nRepKey)
    {
      MM_CriticalSection_Enter(m_pStoreDataLock);
      m_nCurrentRepKey = nRepKey;
      MM_CriticalSection_Leave(m_pStoreDataLock);
    };

    int GetCurrentRepresentation()
    {
      int nRepKey = -1;
      MM_CriticalSection_Enter(m_pStoreDataLock);
      if (m_nCurrentRepKey == -1)
      {
        SwitchElement sElem = {-1, 0, 0};
        if (m_cSwitchQ.PeekHead(sElem))
        {
          nRepKey = sElem.nRepKey;
        }
      }
      else
      {
        nRepKey = m_nCurrentRepKey;
      }
      MM_CriticalSection_Leave(m_pStoreDataLock);
      return nRepKey;
    };

    int GetLastRepresentation()
    {
      int nRepKey = -1;
      MM_CriticalSection_Enter(m_pStoreDataLock);

      SwitchElement sElem = {-1, 0, 0};
      if (m_cSwitchQ.PeekTail(sElem))
      {
        nRepKey = sElem.nRepKey;
      }
      else
      {
        nRepKey = m_nCurrentRepKey;
      }
      MM_CriticalSection_Leave(m_pStoreDataLock);
      return nRepKey;
    };

    void SetReadState(const MediaReadState eReadState)
    {
      MM_CriticalSection_Enter(m_pStoreDataLock);
      m_eReadState = eReadState;
      MM_CriticalSection_Leave(m_pStoreDataLock);
    };

    MediaReadState GetReadState()
    {
      MediaReadState eReadState;
      MM_CriticalSection_Enter(m_pStoreDataLock);
      eReadState = m_eReadState;
      MM_CriticalSection_Leave(m_pStoreDataLock);
      return eReadState;
    };

    void SetPlaybackPosition(const uint64 nPos)
    {
      MM_CriticalSection_Enter(m_pStoreDataLock);
      m_nPlaybackPos = nPos;
      MM_CriticalSection_Leave(m_pStoreDataLock);
    };

    uint64 GetPlaybackPosition()
    {
      uint64 nPos;
      MM_CriticalSection_Enter(m_pStoreDataLock);
      nPos = m_nPlaybackPos;
      MM_CriticalSection_Leave(m_pStoreDataLock);
      return nPos;
    };

    void Reset()
    {
      MM_CriticalSection_Enter(m_pStoreDataLock);
      m_nCurrentRepKey = -1;
      m_eReadState = INIT;
      m_nPlaybackPos = 0;
      (void)m_cSwitchQ.Reset();
      MM_CriticalSection_Leave(m_pStoreDataLock);
    };

    void UpdateMediaReadStoreStats(uint64 nRepKey,
                                   uint64 nDataUnitKey,
                                   uint64 nStartTime,
                                   uint32 nDuration);

    void UpdateMediaReadStoreStats(uint64 nPlaybackPos);

    void UpdateMediaReadStoreStats(uint64             nRepKey,
                                   uint64             nDataUnitKey,
                                   HTTPDownloadStatus eStatus);

    StreamDataQ<SwitchElement> m_cSwitchQ;

    // free data unit list
    ordered_StreamList_type m_dataUnitsFreeList;

    // In use data unit list
    ordered_StreamList_type m_dataUnitsInUseList;

    // the data unit list element
    HTTPDataUnitElement *m_dataUnits;

    // Fully Downloaded Occupancy of the Group
    uint32 nFullyDownloadedOccupancy;

    // Partial Downloaded Occupancy of the Group
    uint32 nPartiallyDownloadedOccupancy;

    // Current Download Position of the Group
    uint64 nCurrentDownloadPosition;

    // lock to control the access for the data unit
    MM_HANDLE  m_dataUnitsLock;

  private:
    MM_HANDLE m_pStoreDataLock;
    HTTPCommon::HTTPMediaType m_eMediaType;
    int m_nCurrentRepKey;
    MediaReadState m_eReadState;
    uint64 m_nPlaybackPos;
    uint64 m_nPrevMaxPlaybackPos;
  };

  int m_nNumMediaComponents;
  MediaReadStore m_cMediaReadStore[HTTP_MAX_MEDIA_COMPONENTS];
  bool m_bInvalidGroup;

  MediaReadStore* GetMediaReadStore(const HTTPCommon::HTTPMediaType eMediaType)
  {
    MediaReadStore* pReadStore = NULL;
    if (eMediaType != HTTPCommon::HTTP_UNKNOWN_TYPE)
    {
      for (int i = 0; i < m_nNumMediaComponents; i++)
      {
        if (m_cMediaReadStore[i].GetMediaType() == eMediaType)
        {
          pReadStore = &m_cMediaReadStore[i];
          break;
        }
      }
    }
    return pReadStore;
  };

  bool m_IslastDataDownloadFailed;
  bool m_IsReadEos;
  InitializationSegment* m_pCommonInitSegment;
public:
  bool IsLastDataDownloadFailed()
  {
    return m_IslastDataDownloadFailed;
  }

  bool IsReadEnd()
  {
    return m_IsReadEos;
  }

  void SetReadEnd(bool isReadEnd)
  {
    m_IsReadEos = isReadEnd;
  }
};

} // namespace video

#endif //__DASHMEDIAPLAYGROUP_H__
