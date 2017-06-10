#ifndef __DASHMEDIAPERIODHANDLER_H__
#define __DASHMEDIAPERIODHANDLER_H__
/************************************************************************* */
/**
 * DASHMediaPeriodHandler.h
 * @brief Defines the DASHMediaPeriodHandler interface.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/DASHMediaPeriodHandler.h#83 $
$DateTime: 2013/10/30 17:31:44 $
$Change: 4692372 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "DASHMediaPlayGroup.h"
#include <IEnhancedStreamSwitchManager.h>
#include <IDataStateProvider.h>
#include <IStreamSource.h>
#include "HTTPDataInterface.h"
#include "HTTPBandwidthEstimator.h"
#include "StreamSourceClock.h"


namespace video {

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define HTTP_MAX_PLAYGROUPS    3
#define HTTP_QSM_STOP_RSP_TIMEOUT 5000

#define AUDIO_TRACK_AVAIL 0x1
#define VIDEO_TRACK_AVAIL 0x2
#define TEXT_TRACK_AVAIL  0x4

// Overhead of Moov / Moof / Sidx in Bytes
#define HTTP_MAX_MEMORY_OVERHEAD (2 * 1024 * 1024)

// Max fragment duration (in Mili Sec)
#define HTTP_MAX_FRAGMENT_DURATION (5 * 1000)

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
class iDASHAdaptorNotifier;
class iDASHAdaptorInfoQuery;
typedef QSM::IEnhancedStreamSwitchManager* (*IPStreamGetCreateQSM)
                                           (QSM::IStreamSource *pIStreamSource,
                                            QSM::IDataStateProvider *pIDsp);

typedef void (*IPStreamGetDeleteQSM)(QSM::IEnhancedStreamSwitchManager*);

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
class DASHMediaPeriodHandler : public QSM::IStreamSource,
                               public QSM::IDataStateProvider,
                               public iPeriodNotifier,
                               public HTTPDataInterface
{
public:
  DASHMediaPeriodHandler(bool& bResult,
                         DASHSessionInfo& sDASHSessionInfo,
                         HTTPBandwidthEstimator *pHTTPBandwidthEstimator,
                         PeriodInfo& cPeriodInfo,
                         iDASHAdaptorNotifier* pNotifier,
                         iDASHAdaptorInfoQuery* pInfoQuery,
                         IPStreamGetCreateQSM pQSMInstanceCreate,
                         IPStreamGetDeleteQSM PQSMInstanceDestroy,
                         Scheduler* pScheduler = NULL);

  virtual ~DASHMediaPeriodHandler();

  //Period handler commands
  enum PeriodCmd
  {
    PERIOD_CMD_NONE,
    PERIOD_CMD_OPEN,
    PERIOD_CMD_SEEK,
    PERIOD_CMD_RESUME,
    PERIOD_CMD_ADAPTATION_SET_CHANGE,
    PERIOD_CMD_ADAPTATION_SET_CHANGE_NOTIFICATION,
    PERIOD_CMD_PURGE_ADAPTATIONSET_Q,
    PERIOD_CMD_SUSPEND
  };

//IStreamSource methods
public:
  virtual QSM::IStreamSourceStatus GetStreamSourceCapabilities
  (
   QSM::StreamSourceCapabilitiesInfo& info
  )
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetStreamSourceCapabilities(info) :
            QSM::SS_STATUS_FAILURE);
  };

  virtual QSM::IStreamSourceStatus GetNumberOfBinningThresholdsForSwitchableGrps
  (
     uint32& nNumThresholds
  );

  virtual QSM::IStreamSourceStatus GetBufferOccupancy
  (
   uint64 /* nGroupKey */,
   uint64 /* nRepresentationKey */,
   uint64 & /* nStartTime */,
   uint64 & /* nDuration */
  )
  {
    //ToDo: Not implemented
    return QSM::SS_STATUS_FAILURE;
  };

  virtual QSM::IStreamSourceStatus GetCurrentPlayBackTime
  (
   uint64 &nTime
  )
  {
    nTime = 0;
    bool bOk = GetCurrentPlaybackPosition(HTTPCommon::HTTP_UNKNOWN_TYPE, nTime);
    QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                   "Current playback time for PH %p = %u msec, bOk %d", (void *)this, (uint32)nTime, bOk  );
    return (bOk ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
  };

  virtual QSM::IStreamSourceStatus GetGlobalPlaybackStats
  (
   uint64& nTime,
   uint64& nOccupancy
  )
  {
    QSM::IStreamSourceStatus status = QSM::SS_STATUS_OK;

    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    status = (pCurrentStateHandler ?
              pCurrentStateHandler->GetGlobalPlaybackStats(nTime, nOccupancy) :
              QSM::SS_STATUS_FAILURE);

    return status;
  };

  virtual QSM::IStreamSourceStatus GetGroupInfo
  (
   QSM::CGroupInfo *pGroupInfo,
   uint32 nSizeOfGroupInfo,
   uint32 &nNumGroupInfo
  )
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetGroupInfo(pGroupInfo, nSizeOfGroupInfo, nNumGroupInfo) :
            QSM::SS_STATUS_FAILURE);
  };

  virtual QSM::IStreamSourceStatus GetRepresentationInfo
  (
   uint64 nGroupKey,
   QSM::CRepresentationInfo *pRepresentationInfo,
   uint32 nSizeOfRepresentationInfo,
   uint32 &nNumRepresentationInfo
  )
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetRepresentationInfo(nGroupKey, pRepresentationInfo,
                                                        nSizeOfRepresentationInfo,
                                                        nNumRepresentationInfo) :
            QSM::SS_STATUS_FAILURE);
  };

  virtual QSM::IStreamSourceStatus SelectRepresentation
  (
   QSM::GroupRepresentationSelection* pSelection,
   uint32 nNumGroupRepresentationSelection
  )
  {
    if(m_bPauseCmdProcessing)
    {
      //Ignoring cmds between play and playcallback
      return QSM::SS_STATUS_FAILURE;
    }
    else
    {
      PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
      return (pCurrentStateHandler ?
              pCurrentStateHandler->SelectRepresentation(pSelection, nNumGroupRepresentationSelection) :
              QSM::SS_STATUS_FAILURE);
    }
  };

  virtual QSM::IStreamSourceStatus GetCurrentRepresentationKey
  (
   uint64 /* nGroupKey */,
   uint64 & /* nRepresentationKey */
  )
  {
    //ToDo: Not implemented
    return QSM::SS_STATUS_FAILURE;
  };

  virtual QSM::IStreamSourceStatus RequestNumberDataUnitsInfo
  (
   uint64 nGroupKey,
   uint64 nRepresentationKey,
   uint64 nStartTime,
   uint64 nDuration
  )
  {
    if(m_bPauseCmdProcessing)
    {
      //Ignoring cmds between play and playcallback
      return QSM::SS_STATUS_FAILURE;
    }
    else
    {
      PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();

      QTV_MSG_PRIO5(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "RequestNumberDataUnitsInfo PH %p, KEYS gr %u, rep %u,startTime %u, duration %u",
        (void *)this, (uint32)nGroupKey, (uint32)nRepresentationKey, (uint32)nStartTime, (uint32)nDuration);

      return (pCurrentStateHandler ?
              pCurrentStateHandler->RequestNumberDataUnitsInfo(nGroupKey, nRepresentationKey,
                                                               nStartTime, nDuration) :
              QSM::SS_STATUS_FAILURE);
    }
  };

  virtual QSM::IStreamSourceStatus ReadDataUnitsInfo
  (
   uint64 nGroupKey,
   uint64 nRepresentationKey,
   uint64 nStartTime,
   uint64 nDuration,
   QSM::CDataUnitInfo *pDataUnitInfo,
   uint32 nElements,
   uint32& nFilledElements
  )
  {
    if(m_bPauseCmdProcessing)
    {
      //Ignoring cmds between play and playcallback
      return QSM::SS_STATUS_FAILURE;
    }
    else
    {
      PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
        QSM::IStreamSourceStatus retVal =
        (pCurrentStateHandler ?
         pCurrentStateHandler->ReadDataUnitsInfo(nGroupKey, nRepresentationKey,
                                                 nStartTime - m_cPeriodInfo.getStartTime(), nDuration,
                                                 pDataUnitInfo, nElements, nFilledElements) :
         QSM::SS_STATUS_FAILURE);

        QTV_MSG_PRIO6(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
          "ReadDataUnitsInfo: GrpKey %llu, repKey %llu, startTime %llu, duration %llu, nElements, %u, nFilled %u",
          nGroupKey, nRepresentationKey, nStartTime, nDuration, nElements, nFilledElements);

        for (uint32 i = 0; i < nFilledElements; ++i)
        {
          QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
            "ReadDataUnitsInfo: unitKey %llu, unitStart %llu, unitDuration %llu, nFilled %u",
            pDataUnitInfo[i].m_nKey, pDataUnitInfo[i].m_nStartTime,
            pDataUnitInfo[i].m_nDuration, (uint32)nFilledElements);
        }
      return retVal;
    }
  };

  virtual QSM::IStreamSourceStatus RequestDownloadDataUnit
  (
   uint64 nGroupKey,
   uint64 nRepresentationKey,
   uint64 nKey
  )
  {
    if(m_bPauseCmdProcessing)
    {
      //Ignoring cmds between play and playcallback
      return QSM::SS_STATUS_FAILURE;
    }
    else
    {
      PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
      QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                     "RequestDownloadDataUnit PH %p, keys gr %u, rep %u, dunit %x:%x",
                     (void *)this, (uint32)nGroupKey, (uint32)nRepresentationKey,
                     (uint32)(nKey >> 32), (uint32)nKey );
      return (pCurrentStateHandler ?
              pCurrentStateHandler->RequestDownloadDataUnit(nGroupKey, nRepresentationKey, nKey) :
              QSM::SS_STATUS_FAILURE);

    }
  };


  virtual QSM::IStreamSourceStatus CancelSelectRepresentation()
  {
    //ToDo: Not implemented
    return QSM::SS_STATUS_FAILURE;
  };

  virtual QSM::IStreamSourceStatus CancelDownloadDataUnit
  (
   uint64 nGroupKey,
   uint64 nRepresentationKey,
   uint64 nKey
  )
  {
    QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "CancelDownloadDataUnit (%d,%d,(%d,%d)",
                   (int)nGroupKey, (int)nRepresentationKey, (int)(nKey >> 32), (int)nKey );

    if(m_bPauseCmdProcessing)
    {
      //Ignoring cmds between play and playcallback
      return QSM::SS_STATUS_FAILURE;
    }
    else
    {
      PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
      return (pCurrentStateHandler ?
              pCurrentStateHandler->CancelDownloadDataUnit(nGroupKey, nRepresentationKey, nKey) :
              QSM::SS_STATUS_FAILURE);
    }
  };

  virtual QSM::IStreamSourceStatus ContinueDownloadDataUnit(
    uint64 nGroupKey, uint64 nRepresentationKey, uint64 nKey)
  {
    QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "ContinueDownloadDataUnit (%d,%d,(%d,%d)",
                   (int)nGroupKey, (int)nRepresentationKey, (int)(nKey >> 32), (int)nKey );

    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->ContinueDownloadDataUnit(nGroupKey,nRepresentationKey,nKey) :
            QSM::SS_STATUS_FAILURE);
  };

  virtual QSM::IStreamSourceStatus GetDataUnitDownloadInfo
  (
   QSM::CDataUnitDownloadInfo* pDownloadInfo,
   uint32 nSize,
   uint32& nFilled,
   uint64 nStartTime
  )
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();

    nFilled = 0;
    (void)pCurrentStateHandler->GetDataUnitDownloadInfo(
      pDownloadInfo, nSize, nFilled, nStartTime);
    return QSM::SS_STATUS_OK;
  };

  virtual QSM::IStreamSourceStatus GetQsmConfigParams(QSM::QsmConfigParams& mConfigParams)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->GetQsmConfigParams(mConfigParams) :
            QSM::SS_STATUS_FAILURE);
  };

  virtual void GroupRequestsCompleted(uint64 nGroupKey)
  {
    if(!m_bPauseCmdProcessing)
    {
      PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
      if (pCurrentStateHandler)
      {
        pCurrentStateHandler->GroupRequestsCompleted( nGroupKey );
      }
    }
    return;
  };

  virtual bool IsEndOfMediaType(video::HTTPCommon::HTTPMediaType mediaType)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    bool result = false;
    if (pCurrentStateHandler)
    {
      result = pCurrentStateHandler->IsEndOfMediaType(mediaType);
    }
    return result;
  };

  virtual void SetLastPeriod()
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->SetLastPeriod();
    }
    return;
  }

  virtual QSM::IDataStateProviderStatus GetCumulativeDownloadStats
  (
   uint64& nTotalTimeDownloadingData,
   uint64& nTotalBytesDownloaded
  );

  virtual QSM::IDataStateProviderStatus GetGroupPlaybackStats(uint64 nGroupKey,
                                                              uint64 &nOccupancy,
                                                              uint64& nPlaybackTime);

  virtual QSM::IDataStateProviderStatus GetGroupPlaybackStats(uint64  nGroupKey,
                                                              uint32& nFullyDownloadedOccupancy,
                                                              uint32& nPartiallyDownloadedOccupancy,
                                                              uint32& nTotalOccupancy,
                                                              uint64& nPlaybackTime,
                                                              uint32& nForwardMediaAvailability);

  virtual QSM::IStreamSourceStatus GetPlaybackTime(uint64 nGrpKey, uint64& nPlaybackTime);

  virtual void CommandStatus
  (
   uint64 /* nTimeStamp */,
   CmdType eCommand,
   CmdStatus /* eComandStatus */
  )
  {
    if (eCommand == IStreamSource::PLAY)
    {
      m_bPauseCmdProcessing = false;
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "HANDLE PLAY rsp recevied");
    }
    else if (eCommand == IStreamSource::STOP)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "QSM STOP response received");
      MM_CriticalSection_Enter(m_pPeriodDataLock);
      m_bDeleteQsm = true;
      MM_CriticalSection_Leave(m_pPeriodDataLock);
    }

    return;
  };

//IDataStateProvider methods
public:
  virtual int SetObservedBandwidthInterval
  (
   int nInterval
  )
  {
    //ToDo: RA should take this!
    return nInterval;
  };

  virtual int GetObservedBandwidth
  (
    uint32 nInterval
  );

  virtual QSM::IDataStateProviderStatus GetBufferOccupancy
  (
   uint64 &nDuration
  );

  virtual bool GetMediaDurationBuffered(HTTPCommon::HTTPMediaType mediaType,
                                        uint32& nFullyDownloadedOccupancy,
                                        uint32& nPartiallyDownloadedOccupancy,
                                        uint32& nTotalOccupancy,
                                        uint64& nPlaybackPosition,
                                        uint32& nForwardMediaAvailability)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return pCurrentStateHandler->GetMediaDurationBuffered(
       mediaType,nFullyDownloadedOccupancy,nPartiallyDownloadedOccupancy,
       nTotalOccupancy,nPlaybackPosition,nForwardMediaAvailability);
  }

  bool GetRepresentationGroupFromXmlKey(uint64 xmlRepGrpKey, RepresentationGroup& repGrp);
  bool GetRepresentationGroupFromGrpKey(uint64 nRepGrpKey, RepresentationGroup& repGrp);

  bool IsAdaptationSetChangePending();

  /**
   * Return true if the first playgroup in the period is readable.
   * This should be called on a period transition for the media
   * type only. Eg, on a period change, we need to wait before the
   * first playgroup for the media type is readable.
   */
  bool IsPeriodReadableForMajorType(HTTPCommon::HTTPMediaType majorType);

  /**
   * Queue the QSM notification for the adaptation set change cmd.
   */
  virtual QSM::IStreamSourceStatus AdaptationSetChangeResponse(
     uint32 tid, QSM::IStreamSource::AdaptationSetChangeStatus s);

//iface towards DASHAdaptor
public:
  HTTPDownloadStatus Open(const uint64 nStartTime, void* pCallbackData)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->Open(nStartTime, pCallbackData)
                                 : HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus Seek(const int64 nSeekTime, void* pCallbackData)
  {

    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->Seek(nSeekTime, pCallbackData)
                                 : HTTPCommon::HTTPDL_ERROR_ABORT);

  };

  virtual bool StoreQsmHistory(uint8* history, uint32& size)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->StoreQsmHistory(history, size) : false);
  };

  virtual bool UpdateBufferPrerollValues()
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->UpdateBufferPrerollValues() : false);
  };

  virtual bool IsEndOfPeriod()
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->IsEndOfPeriod() : false);
  };

  virtual HTTPDownloadStatus Pause()
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->Pause() :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual HTTPDownloadStatus Resume()
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->Resume() :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  virtual bool CanPlaybackUninterrupted()
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->CanPlaybackUninterrupted() :
            false);
  };

  HTTPDownloadStatus Close()
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->Close()
                                 : HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  /**
   * Construct the addset, removeset, replaceset by comparing
   * pSelectionsXML and the currently selected adaptation-sets.
   */
  bool HandleAdaptationSetChangeInfo(const char *pSelectionsXml);

  /**
   * Invoke QSM adaptationset change commands.
   */
  bool HandleAdaptationSetChangeQsmCmd();

//iPeriodNotifier methods
public:
  virtual void SegInfoReady(const uint64 nGroupKey,
                            const uint64 nRepKey,
                            const uint64 nStartTime,
                            const uint64 nDuration,
                            const uint32 nNumDataUnits,
                            const HTTPDownloadStatus eStatus)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->SegInfoReady(nGroupKey, nRepKey, nStartTime, nDuration, nNumDataUnits, eStatus);
    }
  };

  virtual void SegDataReady(const uint64 nGroupKey,
                            const uint64 nRepKey,
                            const uint64 nDataUnitKey,
                            const HTTPDownloadStatus eStatus)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->SegDataReady(nGroupKey, nRepKey, nDataUnitKey, eStatus);
    }
  };

  virtual void NotifyError(const uint64 nGroupKey,
                           const HTTPDownloadStatus eStatus);

  virtual void NotifySeekStatus(uint64 nGroupKey,
                                int64 nSeekEndTime,
                                HTTPDownloadStatus eStatus)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->NotifySeekStatus(nGroupKey, nSeekEndTime, eStatus);
    }
  };

  virtual void NotifyDownloadTooSlow(const uint64 nGroupKey,
                                     const uint32 nRepKey,
                                     const uint64 nDataUnitKey)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->NotifyDownloadTooSlow(nGroupKey, nRepKey, nDataUnitKey);
    }
  };

//HTTPDataInterface methods
public:
  virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo *pTrackInfo)
  {
    uint32 nNumTracks = 0;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      nNumTracks = pCurrentStateHandler->GetMediaTrackInfo(pTrackInfo);
    }
    return nNumTracks;
  };

  virtual HTTPDownloadStatus GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType majorType,
                                                       HTTPMediaTrackInfo &TrackInfo)
  {
    HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      eStatus = pCurrentStateHandler->GetSelectedMediaTrackInfo(majorType, TrackInfo);
    }
    return eStatus;
  };

  virtual HTTPDownloadStatus GetCodecData(uint32 nTrackID,
                                          HTTPCommon::HTTPMediaType majorType,
                                          HTTPMediaMinorType minorType,
                                          HTTPCodecData &CodecData)
  {
    HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      eStatus = pCurrentStateHandler->GetCodecData(nTrackID, majorType, minorType,
                                                   CodecData);
    }
    return eStatus;
  };

  virtual HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType majorType,
                                            uint8* pBuffer,
                                            uint32 &nBufSize)
  {
    HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      eStatus = pCurrentStateHandler->GetFormatBlock(majorType, pBuffer,
                                                     nBufSize);
    }
    return eStatus;
  };

  virtual HTTPDownloadStatus GetNextMediaSample(HTTPCommon::HTTPMediaType majorType,
                                                uint8 *pBuffer,
                                                uint32 &nSize,
                                                HTTPSampleInfo &sampleInfo)
  {
    HTTPCommon::HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      eStatus = pCurrentStateHandler->GetNextMediaSample(majorType, pBuffer,
                                                         nSize, sampleInfo);
    }
    return eStatus;
  };

  void GetVideoInfo(HTTPCommon::HTTPMediaType majorType, char* pVideoURL,size_t& nURLSize, char* pIPAddr, size_t& nIPAddrSize)
  {
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      pCurrentStateHandler->GetVideoInfo(majorType,pVideoURL,nURLSize,pIPAddr,nIPAddrSize);
    }
  };

  virtual bool GetCurrentPlaybackPosition(HTTPCommon::HTTPMediaType mediaType,
                                          uint64 &nPlaybackPosition)
  {
    bool bResult = false;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    nPlaybackPosition = 0;

    if (pCurrentStateHandler)
    {
      bResult = pCurrentStateHandler->GetCurrentPlaybackPosition(mediaType,
                                                                 nPlaybackPosition);
    }
    return bResult;
  };

  virtual bool GetDurationBuffered(HTTPCommon::HTTPMediaType mediaType,
                                   uint64 &nPlaybackPosition,
                                   uint64 &nBufferedDuration)
  {
    bool bResult = false;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      bResult = pCurrentStateHandler->GetDurationBuffered(mediaType,
                                                          nPlaybackPosition,
                                                          nBufferedDuration);

      nPlaybackPosition += GetPeriodStartTime();
    }
    return bResult;
  };

  virtual bool GetGroupDurationBuffered(uint64 nGroupKey,
                                        HTTPCommon::HTTPMediaType mediaType,
                                        uint32& nFullyDownloadedOccupancy,
                                        uint32& nPartiallyDownloadedOccupancy,
                                        uint32& nTotalOccupancy,
                                        uint64& nPlaybackPosition,
                                        uint32& nForwardMediaAvailability)
  {
    bool bResult = false;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
      bResult = pCurrentStateHandler->GetGroupDurationBuffered(nGroupKey, mediaType,
                                                               nFullyDownloadedOccupancy,
                                                               nPartiallyDownloadedOccupancy,
                                                               nTotalOccupancy,
                                                               nPlaybackPosition,
                                                               nForwardMediaAvailability);
    }
    return bResult;
  };

  virtual bool SetConfig(HTTPCommon::HTTPMediaType /* mediaType */,
                         HTTPCommon::HTTPAttribute /* attrType */,
                         HTTPCommon::HTTPAttrVal /* attrVal */)
  {
    return false;
  };
  virtual bool GetConfig(HTTPCommon::HTTPMediaType /* mediaType */,
                         HTTPCommon::HTTPAttribute /* attrType */,
                         HTTPCommon::HTTPAttrVal& /* attrVal */)
  {
    return false;
  };

  virtual bool GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime);

  virtual uint64 GetPeriodKey() const
  {
    return m_cPeriodInfo.getPeriodKey();
  };

  virtual uint64 GetPeriodStartTime()
  {
    return m_cPeriodInfo.getStartTime();
  };

  virtual double GetPeriodDuration()
  {
    return m_cPeriodInfo.getDuration();
  };

  virtual HTTPDownloadStatus GetContentProtectElem(uint32 /* portIndex */,
                                                   HTTPMediaType /* mediaType */,
                                                   HTTPDrmType &/* drmType */,
                                                   HTTPContentStreamType &/* streamType */,
                                                   uint32 &/* contentProtectionInfoSize */,
                                                   unsigned char* /* contentProtectionData */)
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
    return status;
  };

  virtual HTTPDownloadStatus GetCurrentPlayingRepInfo(uint32 & /* currRepSize */,
                                                      unsigned char * /* currRepSring */)
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
    return status;
  };

  virtual HTTPDownloadStatus GetTrackEncodingInfo(FileSourceMnMediaType& /* audio */,
                                                  FileSourceMnMediaType& /* video */,
                                                  FileSourceMnMediaType& /* other */)
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
    return status;
  };

  virtual HTTPDownloadStatus GetCurrentKeys(HTTPCommon::HTTPMediaType mediaType,
                                            int &currGrpInfoKey,
                                            int &currRepInfoKey)
  {
    HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    if (pCurrentStateHandler)
    {
     eStatus = pCurrentStateHandler->GetCurrentKeys(mediaType, currGrpInfoKey,
                                                    currRepInfoKey);
    }
    return eStatus;

  }

  void SetQsmHistory(uint8 *pHistory, uint32 size);

  void SetEnableInitialRateEstimation(bool val);

  void SuspendQSM();
  bool ProcessSuspendQSM();

  bool IsOpenCompleted();

  bool IsReadable(uint32 majorType);

private:

  bool GetCurrentPlaybackPosition(uint64 nGroupKey,
                                  HTTPCommon::HTTPMediaType majorType,
                                  uint64 &nPlaybackPosition);

  //Period handler states
  enum PeriodState
  {
    PERIOD_STATE_IDLE,
    PERIOD_STATE_OPENING,
    PERIOD_STATE_OPEN,
    PERIOD_STATE_SEEKING,
    PERIOD_STATE_CLOSING
  };

  //State handlers
  class PeriodBaseStateHandler
  {
  public:
    PeriodBaseStateHandler(PeriodState eState,
                           DASHMediaPeriodHandler* pPeriodHdlr)
      : m_eState(eState),
        m_pPeriodHandler(pPeriodHdlr)
    {
    };

    virtual ~PeriodBaseStateHandler()
    {
    };

    virtual int ProcessCmds() = 0;

    virtual HTTPDownloadStatus Open(const uint64 nStartTime,
                                    void* pCallbackData)
    {
      (void)nStartTime;
      (void)pCallbackData;
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid state %d for Open() call", m_eState );
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Close();

    virtual bool IsEndOfPeriod();

    virtual bool StoreQsmHistory(uint8* history, uint32& size);

    virtual bool UpdateBufferPrerollValues();

    virtual HTTPDownloadStatus Pause()
    {
      //Nothing to do (QSM to continue download until HWM)
      return HTTPCommon::HTTPDL_SUCCESS;
    };

    virtual HTTPDownloadStatus Resume()
    {
      //Nothing to do (as pause is a NOOP)
      return HTTPCommon::HTTPDL_SUCCESS;
    };

    virtual bool CanPlaybackUninterrupted();

    virtual HTTPDownloadStatus Seek(const int64 nSeekTime, void* pCallbackData);

    virtual bool IsOpenCompleted() const;

    virtual HTTPDownloadStatus StateEntryHandler()
    {
      return HTTPCommon::HTTPDL_SUCCESS;
    };

    virtual QSM::IStreamSourceStatus GetStreamSourceCapabilities(QSM::StreamSourceCapabilitiesInfo& info)
    {
      //Max parallel connections (more like max #outstanding QSM requests across all groups) is configurable
      info.nNumConnections = m_pPeriodHandler->m_sDASHSessionInfo.sSessionInfo.GetHTTPRequestsLimit();
      info.bLive = m_pPeriodHandler->m_sDASHSessionInfo.cMPDParser.IsLive();
      info.bMustStartWithAV = false;
        return QSM::SS_STATUS_OK;
    };

    virtual QSM::IStreamSourceStatus GetGlobalPlaybackStats(uint64& nTime, uint64& nOcc);

    virtual QSM::IStreamSourceStatus GetGroupInfo(QSM::CGroupInfo *pGroupInfo,
                                                  uint32 nSizeOfGroupInfo,
                                                  uint32 &nNumGroupInfo);

    virtual QSM::IStreamSourceStatus GetRepresentationInfo(uint64 nGroupKey,
                                                           QSM::CRepresentationInfo *pRepresentationInfo,
                                                           uint32 nSizeOfRepresentationInfo,
                                                           uint32 &nNumRepresentationInfo);

    virtual QSM::IStreamSourceStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo* pDownloadInfo,
                                                             uint32 nSize,
                                                             uint32& nFilled,
                                                             uint64 nStartTime);

    virtual QSM::IStreamSourceStatus GetQsmConfigParams(QSM::QsmConfigParams& mConfigParams);

    virtual QSM::IStreamSourceStatus RequestNumberDataUnitsInfo(uint64 nGroupKey,
                                                                uint64 nRepresentationKey,
                                                                uint64 nStartTime,
                                                                uint64 nDuration)
    {
      QSM::IStreamSourceStatus qStatus = QSM::SS_STATUS_FAILURE;

      DASHMediaPlayGroup *pPlayGroup = m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);
      if (pPlayGroup)
      {
        if(pPlayGroup->IsValid())
        {
          HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
          nStartTime -= m_pPeriodHandler->GetPeriodStartTime();

          status =
            pPlayGroup->GetSegmentInfo(nRepresentationKey,
                                       nStartTime,
                                       nDuration);
          qStatus = (status == HTTPCommon::HTTPDL_WAITING ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
        }
        else
        {
          qStatus = QSM::SS_STATUS_NO_MORE_DATA;
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "RequestNumberDataUnitsInfo: Group with key %llu not found", nGroupKey);
      }

      return qStatus;
    };

    virtual QSM::IStreamSourceStatus ReadDataUnitsInfo(uint64 nGroupKey,
                                                       uint64 nRepresentationKey,
                                                       uint64 nStartTime,
                                                       uint64 nDuration,
                                                       QSM::CDataUnitInfo *pDataUnitInfo,
                                                       uint32 nElements,
                                                       uint32& nFilledElements)
    {
      QSM::IStreamSourceStatus qStatus = QSM::SS_STATUS_FAILURE;

      DASHMediaPlayGroup *pPlayGroup = m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);
      if (pPlayGroup)
      {
        if (pPlayGroup->IsValid())
        {
          HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
          status =
            pPlayGroup->FillDataUnitInfo(nRepresentationKey,
                                         nStartTime,
                                         nDuration,
                                         pDataUnitInfo,
                                         nElements,
                                         nFilledElements);

          if (pDataUnitInfo)
          {
            for (uint32 i = 0; i < nFilledElements; ++i)
            {
              pDataUnitInfo[i].m_nStartTime += m_pPeriodHandler->GetPeriodStartTime();
            }
          }
          qStatus = (HTTPSUCCEEDED(status) ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
        }
        else
        {
          qStatus = QSM::SS_STATUS_NO_MORE_DATA;
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "ReadDataUnitsInfo: Failed to find playgrp with key %llu", nGroupKey);
      }
      return qStatus;
    };

    virtual QSM::IStreamSourceStatus RequestDownloadDataUnit(uint64 nGroupKey,
                                                             uint64 nRepresentationKey,
                                                             uint64 nKey)
    {
      HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

      DASHMediaPlayGroup *pPlayGroup = m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);
      if (pPlayGroup)
      {
        if (pPlayGroup->IsValid())
        {
          status = pPlayGroup->GetSegmentData(nRepresentationKey, nKey);
        }
        if (HTTPSUCCEEDED(status) && m_pPeriodHandler->m_pQSM)
        {
          m_pPeriodHandler->m_pQSM->DownloadDataUnitDone(nGroupKey, nRepresentationKey,
            nKey, QSM::IEnhancedStreamSwitchManager::DOWNLOAD_SUCCESS);
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "RequestDownloadDataUnit: Did not find playgrp for key %llu", nGroupKey);
      }
      return ((status == HTTPCommon::HTTPDL_SUCCESS || status == HTTPCommon::HTTPDL_WAITING) ?
              QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
    };

    virtual QSM::IStreamSourceStatus CancelDownloadDataUnit(uint64 nGroupKey,
                                                            uint64 nRepresentationKey,
                                                            uint64 nKey)
    {
      HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

      DASHMediaPlayGroup *pPlayGroup = m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);

      if (pPlayGroup)
      {
        status = pPlayGroup->CancelSegmentData(nRepresentationKey, nKey);
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "CancelDownloadDataUnit: Failed to find playgrp with key %llu", nGroupKey);
      }

      if (HTTPSUCCEEDED(status) && m_pPeriodHandler->m_pQSM)
      {
        m_pPeriodHandler->m_pQSM->DownloadDataUnitDone(nGroupKey, nRepresentationKey,
                                                       nKey, QSM::IEnhancedStreamSwitchManager::DOWNLOAD_FAILED);
      }
      return ((status == HTTPCommon::HTTPDL_SUCCESS || status == HTTPCommon::HTTPDL_WAITING) ?
                  QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
    };

    virtual QSM::IStreamSourceStatus ContinueDownloadDataUnit(uint64 nGroupKey,
        uint64 nRepresentationKey, uint64 nKey)
    {
      HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

      DASHMediaPlayGroup *pPlayGroup = m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);

      if (pPlayGroup)
      {
        if (pPlayGroup->IsValid())
        {
          status = pPlayGroup->ContinueDownloadDataUnit(nRepresentationKey, nKey);
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "ContinueDownloadDataUnit: Failed to find playgrp with key %llu", nGroupKey);
      }

      return (HTTPCommon::HTTPDL_WAITING == status ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
    };

    virtual QSM::IStreamSourceStatus SelectRepresentation(QSM::GroupRepresentationSelection* pSelection,
                                                          uint32 nNumGroupRepresentationSelection);

    virtual void SegInfoReady(const uint64 nGroupKey,
                              const uint64 nRepKey,
                              const uint64 nStartTime,
                              const uint64 nDuration,
                              const uint32 nNumDataUnits,
                              const HTTPDownloadStatus eStatus);

    virtual void SegDataReady(const uint64 nGroupKey,
                              const uint64 nRepKey,
                              const uint64 nDataUnitKey,
                              const HTTPDownloadStatus eStatus);

    virtual void NotifyDownloadTooSlow(const uint64 nGroupKey,
                                     const uint32 nRepKey,
                                     const uint64 nDataUnitKey);

    virtual void NotifySeekStatus(uint64 /* nGroupKey */,
                                int64 /* nSeekEndTime */,
                                HTTPDownloadStatus /* eStatus */)
    {
      return;
    };


    // Default implementation for HTTP Data Interface methods
    virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo * /* pTrackInfo */)
    {
      return 0;
    };

    virtual HTTPDownloadStatus GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType /* majorType */,
                                                         HTTPMediaTrackInfo & /* TrackInfo */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetCodecData(uint32 /* nTrackID */,
                                            HTTPCommon::HTTPMediaType /* majorType */,
                                            HTTPMediaMinorType /* minorType */,
                                            HTTPCodecData & /* CodecData */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus GetCurrentKeys(HTTPCommon::HTTPMediaType mediaType,
                                              int &currGrpInfoKey,
                                              int &currRepInfoKey);

    virtual HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType /* majorType */,
                                              uint8* /* pBuffer */,
                                              uint32 & /* nBufSize */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample(HTTPCommon::HTTPMediaType /* majorType */,
                                                              uint8 * /* pBuffer */,
                                                              uint32 & /* nSize */,
                                                              HTTPSampleInfo & /* sampleInfo */)
    {
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    void GetVideoInfo(HTTPCommon::HTTPMediaType majorType, char* pVideoURL,size_t& nURLSize, char* pIPAddr, size_t& nIPAddrSize)
    {
      DASHMediaPlayGroup* pPlayGroup = m_pPeriodHandler->GetPlayGroup(majorType);
      if(pPlayGroup)
      {
        pPlayGroup->GetVideoInfo(majorType,pVideoURL,nURLSize,pIPAddr,nIPAddrSize);
      }
    };

    virtual bool GetCurrentPlaybackPosition( HTTPCommon::HTTPMediaType majorType,
                                             uint64 &nPlaybackPosition);

    virtual bool GetDurationBuffered( HTTPCommon::HTTPMediaType mediaType,
                                      uint64 &nPlaybackPosition,
                                      uint64 &nBufferedDuration );

    virtual bool GetGroupDurationBuffered(uint64 nGroupKey,
                                          HTTPCommon::HTTPMediaType mediaType,
                                          uint32& nFullyDownloadedOccupancy,
                                          uint32& nPartiallyDownloadedOccupancy,
                                          uint32& nTotalOccupancy,
                                          uint64& nPlaybackPosition,
                                          uint32& nForwardMediaAvailability);

    virtual bool GetMediaDurationBuffered(HTTPCommon::HTTPMediaType mediaType,
                                          uint32& nFullyDownloadedOccupancy,
                                          uint32& nPartiallyDownloadedOccupancy,
                                          uint32& nTotalOccupancy,
                                          uint64& nPlaybackPosition,
                                          uint32& nForwardMediaAvailability);

    virtual void GroupRequestsCompleted(uint64 nGroupKey);

    virtual bool IsEndOfMediaType(HTTPCommon::HTTPMediaType mediaType);

    virtual void SetLastPeriod();

    virtual const char *GetStateName() const = 0;

  protected:
    PeriodState m_eState;
    DASHMediaPeriodHandler* m_pPeriodHandler;
  };

  class PeriodIdleStateHandler : public PeriodBaseStateHandler
  {
  public:
    PeriodIdleStateHandler(DASHMediaPeriodHandler* pPeriodHdlr)
      : PeriodBaseStateHandler(PERIOD_STATE_IDLE, pPeriodHdlr)
    {
    };

    virtual ~PeriodIdleStateHandler()
    {
    };

    virtual int ProcessCmds();

    virtual HTTPDownloadStatus Open(const uint64 nStartTime,
                                    void* pCallbackData);

    virtual HTTPDownloadStatus Close();

    virtual QSM::IStreamSourceStatus RequestNumberDataUnitsInfo(uint64 /* nGroupKey */,
                                                                uint64 /* nRepresentationKey */,
                                                                uint64 /* nStartTime */,
                                                                uint64 /* nDuration */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus ReadDataUnitsInfo(uint64 /* nGroupKey */,
                                                       uint64 /* nRepresentationKey */,
                                                       uint64 /* nStartTime */,
                                                       uint64 /* nDuration */,
                                                       QSM::CDataUnitInfo * /* pDataUnitInfo */,
                                                       uint32 /* nElements */,
                                                       uint32& /* nFilledElements */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus RequestDownloadDataUnit(uint64 /* nGroupKey */,
                                                             uint64 /* nRepresentationKey */,
                                                             uint64 /* nKey */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual void SegInfoReady(const uint64 /* nGroupKey */,
                              const uint64 /* nRepKey */,
                              const uint64 /* nStartTime */,
                              const uint64 /* nDuration */,
                              const uint32 /* nNumDataUnits */,
                              const HTTPDownloadStatus /* eStatus */)
    {
      //Nothing to do
    };

    virtual void SegDataReady(const uint64 /* nGroupKey */,
                              const uint64 /* nRepKey */,
                              const uint64 /* nDataUnitKey */,
                              const HTTPDownloadStatus /* eStatus */)
    {
      //Nothing to do
    };

    virtual QSM::IStreamSourceStatus SelectRepresentation(QSM::GroupRepresentationSelection* /* pSelection */,
                                                          uint32 /* nNumGroupRepresentationSelection */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    QSM::IStreamSourceStatus GetDataUnitDownloadInfo
    (
     QSM::CDataUnitDownloadInfo * /* pDownloadInfo */,
     uint32 /* nSize */,
     uint32 &nFilled,
     uint64 /* nStartTime */
    )
    {
      nFilled = 0;
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus ContinueDownloadDataUnit(uint64 /* nGroupKey */,
                                                              uint64 /* nRepresentationKey */,
                                                              uint64 /* nKey */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual bool GetDurationBuffered(HTTPCommon::HTTPMediaType /* mediaType */,
                                     uint64 &nPlaybackPosition,
                                     uint64 &nBufferedDuration)
    {
      nPlaybackPosition = m_pPeriodHandler->m_nStartTime;
      nBufferedDuration = 0;
      return true;
    };

    virtual bool GetGroupDurationBuffered(uint64 /* nGroupKey */,
                                          HTTPCommon::HTTPMediaType /* mediaType */,
                                          uint32& nFullyDownloadedOccupancy,
                                          uint32& nPartiallyDownloadedOccupancy,
                                          uint32& nTotalOccupancy,
                                          uint64& nPlaybackPosition,
                                          uint32& nForwardMediaAvailability)
    {
      nPlaybackPosition = m_pPeriodHandler->m_nStartTime;
      nFullyDownloadedOccupancy     = 0;
      nPartiallyDownloadedOccupancy = 0;
      nTotalOccupancy               = 0;
      nForwardMediaAvailability     = 0;
      return true;
    };

    virtual HTTPDownloadStatus Pause()
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Resume()
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual bool CanPlaybackUninterrupted()
    {
      return false;
    };

    virtual bool GetCurrentPlaybackPosition(HTTPCommon::HTTPMediaType /* majorType */,
                                            uint64 &nPlaybackPosition)
    {
      nPlaybackPosition = m_pPeriodHandler->m_nStartTime;
      return true;
    };

    virtual const char *GetStateName() const
    {
      return "Idle";
    }
  };

  class PeriodOpeningStateHandler : public PeriodBaseStateHandler
  {
  public:
    PeriodOpeningStateHandler(DASHMediaPeriodHandler* pPeriodHdlr)
      : PeriodBaseStateHandler(PERIOD_STATE_OPENING, pPeriodHdlr)
    {
    };

    virtual ~PeriodOpeningStateHandler()
    {
    };

    virtual int ProcessCmds();

    virtual const char *GetStateName() const
    {
      return "Opening";
    }
  };

  class PeriodOpenStateHandler : public PeriodBaseStateHandler
  {
  public:
    PeriodOpenStateHandler(DASHMediaPeriodHandler* pPeriodHdlr)
      : PeriodBaseStateHandler(PERIOD_STATE_OPEN, pPeriodHdlr)
    {
    };

    virtual ~PeriodOpenStateHandler()
    {
    };

    virtual int ProcessCmds();

    virtual bool IsOpenCompleted() const;

    virtual HTTPDownloadStatus StateEntryHandler();

  // implementation for HTTP Data Interface methods
  public:
    virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo *pTrackInfo );

    virtual HTTPDownloadStatus GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType majorType,
                                                         HTTPMediaTrackInfo &TrackInfo );

    virtual HTTPDownloadStatus GetCodecData(uint32 nTrackID,
                                            HTTPCommon::HTTPMediaType majorType,
                                            HTTPMediaMinorType minorType,
                                            HTTPCodecData &CodecData);

    virtual HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType majorType,
                                              uint8* pBuffer,
                                              uint32 &nBufSize);

    virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample(HTTPCommon::HTTPMediaType majorType,
                                                              uint8 *pBuffer,
                                                              uint32 &nSize,
                                                              HTTPSampleInfo &sampleInfo);

    virtual const char *GetStateName() const
    {
      return "Open";
    }
  };

  class PeriodSeekingStateHandler : public PeriodBaseStateHandler
  {
  public:
    PeriodSeekingStateHandler(DASHMediaPeriodHandler* pPeriodHdlr)
      : PeriodBaseStateHandler(PERIOD_STATE_SEEKING, pPeriodHdlr)
    {
    };

    virtual ~PeriodSeekingStateHandler()
    {
    };
    QSM::IStreamSourceStatus GetGlobalPlaybackStats(uint64& nTime, uint64& nOcc);
    virtual int ProcessCmds();

    virtual HTTPDownloadStatus StateEntryHandler();

    virtual HTTPDownloadStatus Seek(const int64 /* nSeekTime */, void* /* pCallbackData */)
    {
      return HTTPCommon::HTTPDL_WAITING;
    };

    virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample( HTTPCommon::HTTPMediaType /* majorType */,
                                                               uint8 * /* pBuffer */,
                                                               uint32 & /* nSize */,
                                                               HTTPSampleInfo & /* sampleInfo */)
    {
      //Block the read's while SEEKing
      return video::HTTPCommon::HTTPDL_WAITING;
    };

    virtual void NotifySeekStatus(uint64 nGroupKey,
                                  int64 nSeekEndTime,
                                  HTTPDownloadStatus eStatus);

    virtual const char *GetStateName() const
    {
      return "Seeking";
    }
  };

  class PeriodClosingStateHandler : public PeriodBaseStateHandler
  {
  public:
    PeriodClosingStateHandler(DASHMediaPeriodHandler* pPeriodHdlr)
      : PeriodBaseStateHandler(PERIOD_STATE_CLOSING, pPeriodHdlr)
    {
    };

    virtual ~PeriodClosingStateHandler()
    {
    };

    virtual int ProcessCmds()
    {
      //Nothing to do
      return 0;
    };

    virtual HTTPDownloadStatus Close();

    virtual bool IsOpenCompleted() const;

    virtual QSM::IStreamSourceStatus GetStreamSourceCapabilities(QSM::StreamSourceCapabilitiesInfo& /* info */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus GetGroupInfo(QSM::CGroupInfo */* pGroupInfo */,
                                                  uint32 /* nSizeOfGroupInfo */,
                                                  uint32 & /* nNumGroupInfo */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus GetRepresentationInfo(uint64 /* nGroupKey */,
                                                           QSM::CRepresentationInfo * /* pRepresentationInfo */,
                                                           uint32 /* nSizeOfRepresentationInfo */,
                                                           uint32 & /* nNumRepresentationInfo */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus RequestNumberDataUnitsInfo(uint64 /* nGroupKey */,
                                                                uint64 /* nRepresentationKey */,
                                                                uint64 /* nStartTime */,
                                                                uint64 /* nDuration */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus ReadDataUnitsInfo(uint64 /* nGroupKey */,
                                                       uint64 /* nRepresentationKey */,
                                                       uint64 /* nStartTime */,
                                                       uint64 /* nDuration */,
                                                       QSM::CDataUnitInfo * /* pDataUnitInfo */,
                                                       uint32 /* nElements */,
                                                       uint32& /* nFilledElements */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus RequestDownloadDataUnit(uint64 /* nGroupKey */,
                                                             uint64 /* nRepresentationKey */,
                                                             uint64 /* nKey */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus SelectRepresentation(QSM::GroupRepresentationSelection* /* pSelection */,
                                                          uint32 /* nNumGroupRepresentationSelection */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual bool GetCurrentPlaybackPosition(HTTPCommon::HTTPMediaType /* majorType */,
                                            uint64 &/* nPlaybackPosition */)
    {
      return false;
    };

    virtual bool GetDurationBuffered(HTTPCommon::HTTPMediaType /* mediaType */,
                                     uint64 &/* nPlaybackPosition */,
                                     uint64 &/* nBufferedDuration */)
    {
      return false;
    };

    virtual bool GetGroupDurationBuffered(uint64 /* nGroupKey */,
                                          HTTPCommon::HTTPMediaType /* mediaType */,
                                          uint32& /* nFullyDownloadedOccupancy */,
                                          uint32& /* nPartiallyDownloadedOccupancy */,
                                          uint32& /* nTotalOccupancy */,
                                          uint64& /* nPlaybackPosition */,
                                          uint32& /* nForwardMediaAvailability */)
    {
      return false;
    };

    virtual HTTPDownloadStatus Pause()
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual HTTPDownloadStatus Resume()
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual bool CanPlaybackUninterrupted()
    {
      return false;
    };

    virtual HTTPDownloadStatus Seek(const int64 /* nSeekTime */, void* /* pCallbackData */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    QSM::IStreamSourceStatus GetDataUnitDownloadInfo
    (
      QSM::CDataUnitDownloadInfo * /*pDownloadInfo */,
      uint32 /* nSize */,
      uint32 &nFilled,
      uint64 /* nStartTime */
    )
    {
      nFilled = 0;
      return QSM::SS_STATUS_FAILURE;
    };

    virtual QSM::IStreamSourceStatus ContinueDownloadDataUnit(uint64 /* nGroupKey */,
        uint64 /* nRepresentationKey */, uint64 /* nKey */)
    {
      return QSM::SS_STATUS_FAILURE;
    };

    virtual const char *GetStateName() const
    {
      return "Closing";
    };

    virtual bool StoreQsmHistory(uint8* /* history */, uint32& /* size */)
    {
      return true;
    };

    virtual void SegInfoReady(const uint64 /* nGroupKey */,
                              const uint64 /* nRepKey */,
                              const uint64 /* nStartTime */,
                              const uint64 /* nDuration */,
                              const uint32 /* nNumDataUnits */,
                              const HTTPDownloadStatus /* eStatus */)
    {
      return;
    };

    virtual void SegDataReady(const uint64 /* nGroupKey */,
                              const uint64 /* nRepKey */,
                              const uint64 /* nDataUnitKey */,
                              const HTTPDownloadStatus /* eStatus */)
    {
      return;
    };

    virtual void NotifyDownloadTooSlow(const uint64 /* nGroupKey */,
                                     const uint32 /* nRepKey */,
                                     const uint64 /* nDataUnitKey */)
    {
      return;
    };

    virtual bool UpdateBufferPrerollValues()
    {
      return true;
    };
};

  //Period base cmd data - to be dequeued by TaskMediaPeriod
  struct PeriodBaseCmdData
  {
    PeriodCmd eCmd;
    void* pUserData;
  };

  //Period OPEN cmd data
  struct PeriodOpenCmdData : public PeriodBaseCmdData
  {
    uint64 nStartTime;
  };

  //Period SEEK cmd data
  struct PeriodSeekCmdData : public PeriodBaseCmdData
  {
    uint64 nSeekTime;
  };

  struct PeriodAdaptationSetChangeData : public PeriodBaseCmdData
  {
    uint32 nTid;
    QSM::IStreamSource::AdaptationSetChangeStatus nChangeStatus;
  };

  //Period cmd data
  union PeriodCmdData
  {
    PeriodBaseCmdData sBaseCmdData;
    PeriodOpenCmdData sOpenCmdData;
    PeriodSeekCmdData sSeekCmdData;
    PeriodAdaptationSetChangeData sAdaptationSetChangeData;
  };

  //Period command task param
  class PeriodCmdTaskParam : public SchedulerTaskParamBase
  {
  public:
    PeriodCmdTaskParam(void* pParam) : pSelf(pParam){ };
    virtual ~PeriodCmdTaskParam(){ };

    void* pSelf;
  };

  HTTPDownloadStatus SetStateHandler(PeriodBaseStateHandler* pCurrStateHandler)
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

    QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "DASHMediaPeriodHandler State transition to '%s' for period %p",
      pCurrStateHandler ? pCurrStateHandler->GetStateName() : "None", this);


    MM_CriticalSection_Enter(m_pPeriodDataLock);
    m_pCurrentStateHandler = pCurrStateHandler;

    if(m_pCurrentStateHandler)
    {
      status = m_pCurrentStateHandler->StateEntryHandler();
    }

    MM_CriticalSection_Leave(m_pPeriodDataLock);
    return status;
  };

  PeriodBaseStateHandler* GetStateHandler()
  {
    PeriodBaseStateHandler* pCurrStateHandler = NULL;
    MM_CriticalSection_Enter(m_pPeriodDataLock);
    pCurrStateHandler = m_pCurrentStateHandler;
    MM_CriticalSection_Leave(m_pPeriodDataLock);
    return pCurrStateHandler;
  };

  static int TaskMediaPeriod(void* pParam);
  bool CreatePlayGroup();
  bool GetGroupMajorType(RepresentationGroup& sRepGroup, uint32& nMajorType);
  uint32 GetQSMMajorType(const HTTPCommon::HTTPMediaType mediaType);
  uint32 GetDASHMajorType(const QSM::MajorType mediaType);
  DASHMediaPlayGroup* GetDownloadingPlayGroup(const HTTPCommon::HTTPMediaType mediaType);
  DASHMediaPlayGroup* GetPlayGroup(const HTTPCommon::HTTPMediaType mediaType);
  DASHMediaPlayGroup* GetFirstPlayGroupInUse(HTTPCommon::HTTPMediaType mediaType);
  void SelectAdaptationSet();
  bool ResumeFromSuspendedState();

  void GetFirstAndSecondPlayGroup(DASHMediaPlayGroup*& pFirstPlayGrp,
                                  DASHMediaPlayGroup*& pSecondPayGrp,
                                  const HTTPCommon::HTTPMediaType mediaType);

  bool ShouldNotifySwitch(HTTPCommon::HTTPMediaType majorType, bool bNotified);

  /**
   * For SLCT of dynamic adaptation set change, ensure that the
   * SLCT time is not earlier than the current playtime of the
   * playgroup.
   */
  bool ValidateAdaptationSetChangeSwitch(DASHMediaPlayGroup *pPlayGroup,
                                         uint64 nRepresentationKey,
                                         uint64 nDataUnitKey);

  /**
   * Populate binning thresholds into CGroupInfo if the group is
   * switchable.
   */
  void PopulateBinningThresholdsForGroup(const DASHMediaPlayGroup& rPlayGrp,
                                         QSM::CGroupInfo& rGroupInfo);

  //Session and period info
  DASHSessionInfo m_sDASHSessionInfo;
  //Bandwidth Estimator object to record amount of data downloaded and total time taken
  HTTPBandwidthEstimator *m_bEstimator;

  PeriodInfo m_cPeriodInfo;

  iDASHAdaptorNotifier* m_pDANotifier;

  iDASHAdaptorInfoQuery* m_pDAInfoQuery;

  Scheduler* m_pScheduler;
  int m_nTaskID;

  //Lock to synchronize period data access
  MM_HANDLE m_pPeriodDataLock;

  /**
   * An element of the RepGroupQ. It stores a DMPG object.
   */
  class RepGroupQElem
  {
  public:
    /**
     * c'tor
     *
     * @param pRepGroup
     */
    RepGroupQElem(DASHMediaPlayGroup *pRepGroup);

    /**
     * d'tor
     */
    ~RepGroupQElem();

    /**
     * Mark the playgroup as 'readable'.
     */
    void Commit();

    /**
     * Check if the playgroup is 'readable'.
     */
    bool IsCommitted() const;

    /**
     * After adaptation set is committed which means QSM is done
     * processing the request, then appropriate action needs to be
     * done along the read sample path to generate port settings
     * changed. Along the read sample path, a check will be made to
     * see if the adaptationsetchangetype is not 'NONE'.
     * (1) If it is ADD then on when a read is for another
     *      majorType, DL_SWITCH will be returned.The type will be
     *      then reset to NONE.
     * (2) If it is of type REMOVE and the read
     *     is for the same majortype as the adaptation set marked
     *     for REMOVE, then DL_SWITCH will be returned. The
     *     adaptset element is then deleted.
     * (3) If it is of type REPLACE, then DL_SWITCH will be
     *     returned an the element type reset to NONE.
     */
    enum AdaptationSetChangeType
    {
      NONE,
      ADD,
      REMOVE,
      REPLACE,
      PURGE,    // special value to purge the element in Source Context.
    };

    /**
     * Get the AdaptationSetChangeType that is pending along the
     * read path.
     */
    AdaptationSetChangeType GetAdaptationSetChangeType() const;

    /**
     * Mark the AdaptationSetChangeType action that is pending along
     * the read path.
     */
    void MarkChangeType(AdaptationSetChangeType changeType);
    bool IsMarkedForRemove() const;

    DASHMediaPlayGroup *m_pRepGroup;

  private:
    RepGroupQElem();
    RepGroupQElem(const RepGroupQElem&);
    RepGroupQElem& operator=(const RepGroupQElem&);

    /**
     * Tracks whether the playgrp is in a transient state. If false,
     * then this playgrp is waiting for QSM to be a able to SLCT a
     * data unit on this repgrp. True initially, and when the
     * playgrp is not in the transient state. When, it is in the
     * transient state, it should not be accessible on the 'read'
     * path.
     */
    bool m_bIsCommitted;

    AdaptationSetChangeType m_ChangeType;
  };

  /**
   * A queue of RepGroupQElem's.
   */
  class RepGroupQ
  {
  public:
    /**
     * c'tor
     */
    RepGroupQ();

    /**
     * d'tor
     */
    ~RepGroupQ();

    /**
     * Add a new playgrp to the Q.
     */
    bool Push(RepGroupQElem *pQElem);

    void Pop();

    /**
     * Get a handle to the first elem in the Q.
     */
    RepGroupQElem* PeekHead();

    void PeekFirstAndSecondElem(RepGroupQElem*& pFirst, RepGroupQElem*& pSecond);

    /**
     * Get a handle to the last element in the Q.
     */
    RepGroupQElem* PeekTail();

    RepGroupQElem *PeekFirstElementInUse();

    /**
     * Get the first element which is not marked for Remove.
     */
    RepGroupQElem *PeekFirstElementForRead();

    /**
     * Get a handle to the last element which is readable in the Q.
     */
    RepGroupQElem* GetLastCommittedElem();

    /**
     * Get a handle to the Q element associated with the playgroup
     * with key nGroupKey.
     */
    RepGroupQElem* GetElemByKey(uint64 nGroupKey);

    /**
     * De-initialize and delete all playgroups, and delete the
     * RepGroupQElem assocaited with the playgroups.
     */
    void Shutdown();

    /**
     * Check if the Q is empty.
     */
    bool IsEmpty();

    /**
     * For debugging.
     */
    void Print();

    /**
     * Return true if the first playgroup in the repGroupQ is
     * readable. This should be called on a period transition for
     * the media type only. Eg, on a period change, we need to wait
     * before the first playgroup for the media type is readable.
     */
    bool IsSwitchableTo();

    void QueueOptimize();
    bool IsPendingSwitchDLSwitchNotificationForAdd();
    bool IsPendingSwitchDLSwitchNotificationForRemove(uint32 eQsmMajorType);

    /**
     * Commit the elements from 'read' perspective. If an element
     * was added, the switch notification was sent so this can be
     * marked readable. If an element was removed, then it is okay
     * to remove it as switch notification was sent.
     */
    void DLSwitchNotificationSent();

    bool IsReadable();

    /**
     * Checks to see if there is any repGroupQ for which for which
     * there is an element that was added or removed. If an element
     * was added then this returns true irrespective of the media
     * type. If an element was removed, returns true if the
     * mediatype matches the removed element.
     */
    bool IsFirstElementWaitingForRemove();
    void RemoveFirstElementFromQ();
    bool RemoveLastElementFromQ();

    void RemoveAllButLastElementFromQ();

    void GroupRequestsCompleted(uint64 nRelPeriodEndTime);
    bool IsEndOfPeriod();

    void GetPlayGrpByRepresentationInfoKey(uint64 nRepresentationGrpInfoKey,
                                           IPStreamList<uint64>& rrDMPGKeyList);

    void MarkRemoved(uint64 nRepresentationGrpInfoKey);

    bool DeleteElementByPlayGroupKey(uint64 nPlayGrpKey);

    void GetPlaybackStatsForQ(HTTPCommon::HTTPMediaType,
                              uint32& nFullyDownloadedOccupancy,
                              uint32& nPartiallyDownloadedOccupancy,
                              uint32& nTotalOccupancy,
                              uint64& nPlaybackPosition,
                              uint32& nForwardMediaAvailability);

    void ClearBufferedData(uint64 nTime, uint64 nExceptPlayGrpKey);

    void PurgeAdaptationSetQ();
    bool CanPlaybackUninterrupted();

  private:
    RepGroupQ(const RepGroupQ&);
    RepGroupQ& operator=(const RepGroupQ&);

    IPStreamList<RepGroupQElem *> m_RepGroupQ;
  };

  RepGroupQ m_RepGroupQ[1 + QSM::MAJOR_TYPE_MAX];

  DASHMediaPlayGroup *GetPlayGrpByKey(uint64 nGroupKey);

  void GetPlayGrpByRepresentationInfoKey(uint64 rRepresentationGrpInfoKey,
                                         IPStreamList<uint64>& rDMPGKeyList);
  int GetNumGroups();

  void DeleteElementByPlayGrpKey(uint64 nPlayGrpKey);

  void PurgeAdaptationSetQ();

  void ClearBufferedData(DASHMediaPlayGroup& rPlayGrp, uint64 nTime);

  void GetCumulativeDurationBuffered(HTTPCommon::HTTPMediaType majorType,
                                     uint64 &nPlaybackPosition, uint64 &nBuffDur);

  /**
   * Process the Notification from QSM
   */
  void HandleAdaptationSetChangeNotification(
     uint32 nTid,
     QSM::IStreamSource::AdaptationSetChangeStatus nChangeStatus,
     RepGroupQElem::AdaptationSetChangeType& eChangeType);

private:


  // prevents more that one download complete notification in case QSM
  // requests for number of data units even after receiving no_more_data
  bool m_bIsDownloadCompleteNotified;

  //QSM iface
  QSM::IEnhancedStreamSwitchManager* m_pQSM;

  //Period state handlers
  PeriodIdleStateHandler m_cIdleStateHandler;
  PeriodOpeningStateHandler m_cOpeningStateHandler;
  PeriodOpenStateHandler m_cOpenStateHandler;
  PeriodSeekingStateHandler m_cSeekingStateHandler;
  PeriodClosingStateHandler m_cClosingStateHandler;

  PeriodBaseStateHandler* m_pCurrentStateHandler;

  //Period cmd queue
  StreamDataQ<PeriodCmdData> m_cCmdQ;

  uint64 m_nStartTime;
  int64 m_nSeekTime;
  int64 m_nSeekTimeRelative;
  bool m_bPauseCmdProcessing;

  // Set to true when QSM can be destroyed.
  bool m_bDeleteQsm;

  HTTPDownloadStatus m_eSeekStatus;

  //Signals for handling the asynchronous QSM STOP Command
  MM_HANDLE m_hQSMStopSignalQ;
  MM_HANDLE m_hQSMStopRspSignal;

  // QSM Create and destroy Interface pointers
  IPStreamGetCreateQSM m_pQSMInstanceCreate;
  IPStreamGetDeleteQSM m_pQSMInstanceDestroy;

  uint32 m_nNextKey;

  /**
   * A unit of dynamic adaptation set change to be performed by
   * QSM. For QSM, if m_nOldRepGrpInfoKey is not set, and
   * m_NewRepGrpInfoKey is set, then it is ADD. If
   * m_nOldRepGrpInfoKey is set, and m_NewRepGrpInfoKey is not
   * set, then it is REMOVE. If both are set, then it is a
   * replace.
   */
  class DASMQElem
  {
  public:
    DASMQElem();
    ~DASMQElem();

    void SetMajorType(uint32 nQsmMajorType);
    void SetOldRepGrpInfoKey(uint64 nKey);
    void SetNewRepGrpInfoKey(uint64 nKey);
    bool AddOldRepInfoKey(uint64 nKey);
    bool AddNewRepInfoKey(uint64 nKey);
    uint32 GetMajorType() const;
    uint64 GetOldRepGrpInfoKey() const;
    uint64 GetNewRepGrpInfoKey() const;

  private:
    uint32 m_MajorType;
    uint64 m_nOldRepGrpInfoKey;
    uint64 m_nNewRepGrpInfoKey;
    IPStreamList<uint64> m_OldSelectedRepInfoList;
    IPStreamList<uint64> m_NewSelectedRepInfoList;

    friend class DASMQ;
  };

  /**
   * Store for dynamic adaptation-set changes that are pending.
   */
  class DASMQ
  {
  public:
    DASMQ();
    ~DASMQ();

    bool Push(DASMQElem *pElem);
    DASMQElem *PeekHeadElem();
    void Pop();
    void PopTail();
    bool IsEmpty();

  private:
    IPStreamList<DASMQElem *> m_Q;
  };

  DASMQ m_DASMQ;

  // Used to fail SLCT if switch is to a time less
  // than the playback time of the majorType on an
  // adaptation set change.
  uint64 m_LastSampleTS[HTTPCommon::HTTP_MAX_TYPE];

  /**
   * true when Resume needs to be called on QSM after queueing
   * adaptationset change cmd on qsm.
   */
  bool m_bIsQSMSuspended;

#ifdef DASH_STATS
  //Test code for collecting stats
  MM_HANDLE m_pStatsFile;
  int m_nCurrentRepID;
#endif /* DASH_STATS */
};

//DA notification handler interface
class iDASHAdaptorNotifier
{
public:

  virtual ~iDASHAdaptorNotifier() { };

  virtual void NotifyEvent(uint64 nPeriodKey,
                           DASHMediaPeriodHandler::PeriodCmd eCmd,
                           HTTPDownloadStatus eStatus,
                           void* pCbData) = 0;

  virtual void DownloadComplete(uint64 nPeriodKey) = 0;
};

//DA InfoQuery handler interface
class iDASHAdaptorInfoQuery
{
public:

  virtual ~iDASHAdaptorInfoQuery() { };

  virtual bool GetGlobalPlayBackTime(uint64& nPlayBackTime) = 0;

  virtual QSM::IDataStateProviderStatus GetCumulativeDownloadStats
  (
   uint64& nTotalTimeDownloadingData,
   uint64& nTotalBytesDownloaded
  ) = 0;

  virtual uint32 GetMaxBitrateAcrossActivePeriods() = 0;
};

} // namespace video

#endif //__DASHMEDIAREPRESENTATIONHANDLER_H__
