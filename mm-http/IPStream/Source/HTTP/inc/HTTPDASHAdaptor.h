#ifndef __DASHADAPTOR_H__
#define __DASHADAPTOR_H__
/************************************************************************* */
/**
 * DASHAdaptor.h
 * @brief Header file for DASH Adaptor.
 *
 * Implements and specializes HTTPDownloadHelper, to do DASH streaming.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPDASHAdaptor.h#53 $
$DateTime: 2013/06/28 16:10:11 $
$Change: 4009813 $

========================================================================== */
/* =======================================================================
**               Include files for DASHAdaptor.h
** ======================================================================= */
#include "HTTPCommon.h"
#include "HTTPDataInterface.h"
#include "HTTPDownloadHelper.h"
#include "HTTPSessionInfo.h"
#include "MPDParser.h"
#include "DASHMediaPeriodHandler.h"
#include  "HTTPHeapManager.h"
#include "StreamQueue.h"

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
class DASHMediaPeriodHandler;
class MPDParser;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class DASHAdaptor : public HTTPDownloadHelper,
                    public iDASHAdaptorNotifier,
                    public iDASHAdaptorInfoQuery,
                    public HTTPDataInterface
{
public:
  DASHAdaptor( bool& bOk,
               HTTPSessionInfo& sessionInfo, uint32 nRequestIDfromResolver, HTTPStackInterface& HTTPStack,
               HTTPStatusHandlerInterface *pHTTPStatusHandler,
                           Scheduler* pScheduler);
  virtual ~DASHAdaptor();

  // HTTPDownloadHelper interface
  virtual HTTPDownloadStatus InitiateHTTPConnection();
  virtual HTTPDownloadStatus GetData();
  virtual uint64 GetTotalBytesReceived();
  virtual HTTPDownloadStatus CloseHTTPConnection();
  virtual HTTPDownloadStatus Seek(const int64 nSeekTime);
  virtual HTTPDownloadStatus Pause();
  virtual HTTPDownloadStatus Resume();
  virtual bool GetDownloadProgress(HTTPMediaType mediaType,
                                   uint32& currStartOffset,
                                   uint32& downloadOffset,
                                   HTTPDownloadProgressUnitsType eUnitsType,
                                   bool& bEOS);

  virtual bool IsAdaptationSetChangePending();
  virtual void UpdateTrackSelections();

  virtual bool IsEndOfFile();

  virtual bool IsLiveStreamingSession()
  {
    return m_playlistParser.IsLive();
  }

  // HTTPDashAdaptorCallbackInterface methods
  virtual void NotifyEvent(uint64 nPeriodKey,
                           DASHMediaPeriodHandler::PeriodCmd eCommand,
                           HTTPDownloadStatus HTTPStatus,
                           void* pCbData);

  /**
   * Notification from PeriodHandler when end of stream in terms
   * of period download is reached.
   */
  virtual void DownloadComplete(uint64 nPeriodKey);

  // iDASHAdaptorInfoQuery Interface methods
  virtual bool GetGlobalPlayBackTime(uint64& nPlayBackTime);

  virtual QSM::IDataStateProviderStatus GetCumulativeDownloadStats(
    uint64& nTotalTimeDownloadingData, uint64& nTotalBytesDownloaded);

  // Query the maxbitrate across active periods.
  virtual uint32 GetMaxBitrateAcrossActivePeriods();

  // HTTPDataInterface methods
  virtual HTTPDataInterface *GetDataInterface()
  {
    return (HTTPDataInterface *)this;
  }

  virtual bool GetMediaProperties(char *pPropertiesStr,uint32 &nPropertiesLen);

  virtual bool SelectRepresentations(const char* SetSelectionsXML);

  virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo *pTrackInfo );

  virtual HTTPDownloadStatus GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType majorType,
                                                       HTTPMediaTrackInfo &TrackInfo );

  virtual HTTPDownloadStatus GetCodecData(uint32 nTrackID,
                                          HTTPCommon::HTTPMediaType majorType,
                                          HTTPMediaMinorType minorType,
                                          HTTPCodecData &CodecData);

  virtual HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType majorType,
                                            uint8* pBuffer,
                                            uint32 &nbufSize);


  virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample( HTTPCommon::HTTPMediaType majorType,
                                                             uint8 *pBuffer,
                                                             uint32 &nSize,
                                                             HTTPSampleInfo &sampleInfo);

  void GetVideoInfo(HTTPCommon::HTTPMediaType majorType, char* pVideoURL,size_t& nURLSize, char* pIPAddr, size_t& nIPAddrSize);

  virtual bool GetCurrentPlaybackPosition( HTTPCommon::HTTPMediaType,
                                           uint64 &nPlaybackPosition);

  virtual bool GetDurationBuffered( HTTPCommon::HTTPMediaType mediaType,
                                    uint64 &nPlaybackPosition,
                                    uint64 &nBufferedDuration );

 virtual bool SetConfig( HTTPCommon::HTTPMediaType mediaType,
                         HTTPCommon::HTTPAttribute attrType,
                         HTTPCommon::HTTPAttrVal attrVal );

 virtual bool GetConfig( HTTPCommon::HTTPMediaType mediaType,
                         HTTPCommon::HTTPAttribute attrType,
                         HTTPCommon::HTTPAttrVal& attrVal );

  virtual bool GetMPDText(char *pMPDTextStr, uint32 &mpdSize);

  virtual void GetQOEData(uint32 &bandwidth, char *pVideoURL, size_t& nURLSize, char *pIpAddr, size_t& nIPAddrSize);

  void GetNetworkBandwidth(uint64& bandwidth);

  /**
   * MPD may have changed.
   */
 static void MPDUpdateNotificationHandler(void *privData);

 virtual HTTPDownloadStatus GetContentProtectElem(uint32 portIndex,
                                          HTTPMediaType mediaType,
                                          HTTPDrmType &drmType,
                                          HTTPContentStreamType &streamType,
                                          uint32 &contentProtectionInfoSize,
                                          unsigned char* contentProtectionData);

 virtual HTTPDownloadStatus GetCurrentPlayingRepInfo(uint32 &currRepSize,
                                                     unsigned char *currRepSring);

 virtual HTTPDownloadStatus GetTrackEncodingInfo(FileSourceMnMediaType& audio,
                                                 FileSourceMnMediaType& video,
                                                 FileSourceMnMediaType& other);


 protected:
   enum SeekNotificationStatus
  {
    SEEK_STATUS_NONE,
    SEEK_STATUS_SUCCESS,
    SEEK_STATUS_FAIL,
    SEEK_STATUS_END
  };
  enum DASHAdaptorState
  {
    INITIATE_CONNECTION,
    GET_FIRST_PERIOD,
    OPEN_PERIOD,
    PLAY_PERIOD,
    ERROR_STATE
  };

  class BaseStateHandler : public HTTPDataInterface
  {
  public:
    BaseStateHandler(DASHAdaptor *pDASHAdaptor) : m_pDASHAdaptor(pDASHAdaptor){ };
    virtual ~BaseStateHandler(){ };

    virtual HTTPDownloadStatus StateEntryHandler()
    {
      return video::HTTPCommon::HTTPDL_SUCCESS;
    }

    virtual HTTPDownloadStatus StateExitHandler()
    {
      return video::HTTPCommon::HTTPDL_SUCCESS;
    }

    virtual HTTPDownloadStatus InitiateHTTPConnection()
    {
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus Pause()
    {
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus Resume()
    {
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus Seek(const int64 /* nSeekTime */)
    {
      // return waiting so that TaskSeekSession will continue to call Seek
      // on DASHAdaptor.
      return video::HTTPCommon::HTTPDL_WAITING;
    }
    virtual void Reset()
    {
      return;
    }

    virtual HTTPDownloadStatus GetData()
    {
      return video::HTTPCommon::HTTPDL_WAITING;
    }

    virtual uint64 GetTotalBytesReceived();

    virtual void ProcessEvent( DASHMediaPeriodHandler::PeriodCmd eCommand,
                               HTTPDownloadStatus status, void* pCbData );

    DASHAdaptorState GetState() { return m_state; }

    // Default implementation for HTTP Data Interface methods
    virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo * /* pTrackInfo */ )
    {
      return 0;
    }

    virtual HTTPDownloadStatus GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType majorType,
                                                         HTTPMediaTrackInfo &TrackInfo );

    virtual HTTPDownloadStatus GetCodecData(uint32 /* nTrackID */,
                                            HTTPCommon::HTTPMediaType /* majorType */,
                                            HTTPMediaMinorType /* minorType */,
                                            HTTPCodecData & /* CodecData */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType /* majorType */,
                                              uint8* /* pBuffer */,
                                              uint32 & /* nbufSize */)
    {
      // unused
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample( HTTPCommon::HTTPMediaType /* majorType */,
                                                               uint8 * /* pBuffer */,
                                                               uint32 & /* nSize */,
                                                               HTTPSampleInfo & /* sampleInfo */)
    {
      // Unused
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual bool GetCurrentPlaybackPosition( HTTPCommon::HTTPMediaType,
                                             uint64 & /* nPlaybackPosition */)
    {
      // Unused
      return false;
    }

    virtual bool GetDurationBuffered( HTTPCommon::HTTPMediaType /* mediaType */,
                                      uint64 & /* nPlaybackPosition */,
                                      uint64 & /* nBufferedDuration */)
    {
      // unused
      return false;
    }

    virtual bool SetConfig( HTTPCommon::HTTPMediaType mediaType,
                         HTTPCommon::HTTPAttribute attrType,
                         HTTPCommon::HTTPAttrVal attrVal );
    virtual bool GetConfig( HTTPCommon::HTTPMediaType mediaType,
                         HTTPCommon::HTTPAttribute attrType,
                         HTTPCommon::HTTPAttrVal& attrVal );
    virtual bool GetMediaProperties(char *pPropertiesXMLStr,
                                    uint32 &nPropertiesLen);
    virtual bool SelectRepresentations(const char* SetSelectionsXML);

    virtual bool GetMPDText(char *pMPDTextStr, uint32 &mpdSize);

    virtual HTTPDownloadStatus GetContentProtectElem(uint32 portIndex,
                         HTTPMediaType mediaType,
                         HTTPDrmType &drmType,
                         HTTPContentStreamType &streamType,
                         uint32 &contentProtectionInfoSize,
                         unsigned char* contentProtectionData);

    virtual HTTPDownloadStatus GetCurrentPlayingRepInfo(uint32 &currRepSize,
                                                        unsigned char *currRepSring);

    virtual HTTPDownloadStatus GetTrackEncodingInfo(FileSourceMnMediaType& audio,
                                                    FileSourceMnMediaType& video,
                                                    FileSourceMnMediaType& other);
    virtual void MoveSuspendedElementToPeriodQ();

  protected:
    DASHAdaptorState m_state;
    DASHAdaptor *m_pDASHAdaptor;
  };

  class InitiateConnectionStateHandler : public BaseStateHandler
  {
  public:
    InitiateConnectionStateHandler(DASHAdaptor *pDASHAdaptor):
    BaseStateHandler(pDASHAdaptor) { m_state = INITIATE_CONNECTION; };
    virtual ~InitiateConnectionStateHandler(){ };
    virtual HTTPDownloadStatus StateEntryHandler();
    virtual HTTPDownloadStatus InitiateHTTPConnection();
    virtual bool GetMediaProperties(char * /* pPropertiesStr */,
                                    uint32 & /* nPropertiesLen */)
    {
      return false;
    }
    virtual bool GetMPDText(char * /* pMPDTextStr */, uint32 & /* mpdSize */)
    {
      return false;
    }
  };

  class GetFirstPeriodStateHandler : public BaseStateHandler
  {
  public:
    GetFirstPeriodStateHandler(DASHAdaptor *pDASHAdaptor) :
      BaseStateHandler(pDASHAdaptor) { m_state = GET_FIRST_PERIOD; };
    virtual ~GetFirstPeriodStateHandler(){ };

    virtual HTTPDownloadStatus InitiateHTTPConnection();

    virtual bool SelectRepresentations(const char* SetSelectionsXML);
  };

  class OpenPeriodStateHandler : public BaseStateHandler
  {
  public:
    OpenPeriodStateHandler(DASHAdaptor *pDASHAdaptor) :
      BaseStateHandler(pDASHAdaptor),
      m_bIsOpenFailed(false),
      m_bWaitForNewPeriod(false)
    {
      m_state = OPEN_PERIOD;
    };
    virtual ~OpenPeriodStateHandler(){ };

    virtual HTTPDownloadStatus StateEntryHandler();
    virtual HTTPDownloadStatus GetData();
    virtual void ProcessEvent( DASHMediaPeriodHandler::PeriodCmd eCommand,
                               HTTPDownloadStatus HTTPStatus,
                               void* pCbData);

  private:
    bool m_bIsOpenFailed;
    bool m_bWaitForNewPeriod;
  };

  class PlayPeriodStateHandler : public BaseStateHandler
  {
  public:
    PlayPeriodStateHandler(DASHAdaptor *pDASHAdaptor) :
      BaseStateHandler(pDASHAdaptor) { m_state = PLAY_PERIOD; };
    virtual ~PlayPeriodStateHandler(){ };

    virtual HTTPDownloadStatus StateEntryHandler()
    {
      m_bWaitingForTracks = true;
      return HTTPCommon::HTTPDL_SUCCESS;
    }
    virtual HTTPDownloadStatus GetData();

    virtual void Reset()
    {
      //reset m_bWaitingForTracks flag
      m_bWaitingForTracks = false;
      return;
    }

    // implementation for HTTP Data Interface methods
    virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo *pTrackInfo );

    virtual HTTPDownloadStatus GetCodecData(uint32 nTrackID,
                                            HTTPCommon::HTTPMediaType majorType,
                                            HTTPMediaMinorType minorType,
                                            HTTPCodecData &CodecData);

    virtual HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType /* majorType */,
                                              uint8* /* pBuffer */,
                                              uint32 & /* nbufSize */)
    {
      // unused
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample( HTTPCommon::HTTPMediaType /* majorType */,
                                                               uint8 * /* pBuffer */,
                                                               uint32 &/* nSize */,
                                                               HTTPSampleInfo &/* sampleInfo */)
    {
      // unused
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual bool GetCurrentPlaybackPosition( HTTPCommon::HTTPMediaType,
                                             uint64 &/* nPlaybackPosition */)
    {
      // unused
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual bool GetDurationBuffered( HTTPCommon::HTTPMediaType /* mediaType */,
                                      uint64 & /* nPlaybackPosition */,
                                      uint64 & /* nBufferedDuration */)
    {
      // unused
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    virtual void ProcessEvent(DASHMediaPeriodHandler::PeriodCmd eCommand, HTTPDownloadStatus status,
                                void* pCbData);

  private:
    bool m_bWaitingForTracks;
  };

  class ErrorStateHandler : public BaseStateHandler
  {
  public:
    ErrorStateHandler(DASHAdaptor *pDASHAdaptor) :
      BaseStateHandler(pDASHAdaptor){ m_state = ERROR_STATE; };
    virtual ~ErrorStateHandler(){ };

    virtual HTTPDownloadStatus GetData()
    {
      return video::HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual bool GetMediaProperties(char * /* pPropertiesStr */,
                                    uint32 & /* nPropertiesLen */)
    {
      return false;
    }

    virtual bool GetMPDText(char * /* pMPDTextStr */, uint32 & /* mpdSize */)
    {
      return false;
    }
  };

  HTTPDownloadStatus SetStateHandler( BaseStateHandler *pStateHandler);

  BaseStateHandler *GetCurrentStateHandler()
  {
    return m_pCurrentStateHandler;
  }

  char *GetLaunchURL()
  {
    return m_pLaunchURL;
  }

  void SetLaunchURL(char *pLaunchURL)
  {
    m_pLaunchURL = pLaunchURL;
  }

  StreamSourceClock *GetStreamSourceClock()
  {
    return m_pSourceClock;
  }

  void SetStreamSourceClock(StreamSourceClock *pSourceClock)
  {
    m_pSourceClock = pSourceClock;
  }

  HTTPSessionInfo &GetSessionInfo()
  {
    return m_sessionInfo;
  }

  /**
   * Get array idx on m_PeriodHandlerReadQueue for majorType.
   */
  int GetReadQArrayIdx(HTTPCommon::HTTPMediaType majorType) const;

  /**
   * Get media type on m_PeriodHandlerReadQueue for array idx.
   */
  HTTPCommon::HTTPMediaType GetReadQMediaType(const int idx) const;

  /**
   * Add ptr to PeriodHandler to the readQ for the majorType.
   */
  bool AddReadQElem(HTTPCommon::HTTPMediaType majorType,
                    DASHMediaPeriodHandler &rPeriodHandler);

  /**
   * Get ptr to PeriodHandler to the first element in the readQ
   * for the majorType.
   */
  DASHMediaPeriodHandler *GetPeriodHandler(HTTPCommon::HTTPMediaType majorType);

  /**
   * For debugging
   */
  void PrintQueues();

  /**
   * @brief
   *   Locate period with startTime in MPD and instantiate a
   *   PeriodHandler instance for this period.
   *
   * @param startTime playlist initialization start time
   * @param bSeek true if creating period as part of seek
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus InitializeAndCreatePeriod(const int64 startTime, const bool bSeek = false);

  /**
   * @brief
   *   Instantiate a PeriodHandler instance for given period.
   *
   * @param periodInfo period info
   * @param bOpen true if period needs to be opened after creation
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus CreatePeriod(PeriodInfo& periodInfo, const bool bOpen = true);

   /**
   * Update the readQ for all media types with the last
   * element of the periodHandlerQ. This should be invoked
   * only after open completed on the last period.
   *
   * @return bool
   */
  bool UpdateReadQs();

  HTTPDownloadStatus UpdateMediaTrackInfo();

  /**
   * Delete the periodHandlers in all the queues.
   */
  HTTPDownloadStatus ClosePeriodHandlers();

  HTTPDownloadStatus CreateNextPeriod();

  /**
   * Store the QSM history.
   */
  void StoreQsmHistory(DASHMediaPeriodHandler& m_rPeriodHandler);

  void SetTracksSetup(const bool bTracksSetup)
  {
    MM_CriticalSection_Enter(m_pAdaptorLock);
    m_bTracksSetup = bTracksSetup;
    MM_CriticalSection_Leave(m_pAdaptorLock);
  };

  bool IsTracksSetup() const
  {
    bool bTracksSetup;
    MM_CriticalSection_Enter(m_pAdaptorLock);
    bTracksSetup = m_bTracksSetup;
    MM_CriticalSection_Leave(m_pAdaptorLock);
    return bTracksSetup;
  };

  StreamSourceClock *m_pSourceClock;
  HTTPBandwidthEstimator *m_bEstimator;

  Scheduler *m_pScheduler;

  MPDParser m_playlistParser;

  HTTPHeapManager m_HeapManager;

  PeriodInfo m_currentPeriodInfo;

  BaseStateHandler *m_pCurrentStateHandler;

  InitiateConnectionStateHandler m_initiateConnectionStateHandler;
  GetFirstPeriodStateHandler m_getFirstPeriodStateHandler;
  OpenPeriodStateHandler m_openPeriodStateHandler;
  PlayPeriodStateHandler m_playPeriodStateHandler;
  ErrorStateHandler m_errorStateHandler;
  uint64 m_nBaseMediaOffset;
  uint64 m_nBaseMPDOffset;
  uint64 m_nBasePeriodOffset;
  uint64 m_nTotalDataDownloaded;
  uint64 m_nPreviousCummulativeDownload;
  /**
   * Num media types supported.
   */
  static const int NUM_MEDIA_TYPES = 3;

  /**
   * True if tracks are setup for the first time.
   */
  bool m_bTracksSetup;

  /**
   * DASHSessionInfo shared across all periods.
   */
  DASHSessionInfo *m_pDASHSessionInfo;

  /**
   * True if last period is downloaded.
   */
  bool m_bIsEndOfStream;

  /**
   * Set to true if an attempt should be made to download a new
   * period.
   */
  bool m_bCreatePeriod;

  /**
   * Queue element for the periods being downloaded.
   */
  class PeriodHandlerElement
  {
  public:
    StreamQ_link_type m_link;
    PeriodHandlerElement(PeriodInfo& rPeriodInfo, DASHMediaPeriodHandler *pPeriodHandler);
    ~PeriodHandlerElement();

    DASHMediaPeriodHandler *m_pPeriodHandler;

    enum ElementStateEnum
    {
      PERIOD_ACTIVE,
      PERIOD_MARKED_FOR_DELETION,
      PERIOD_QSM_DELETED
    };

    ElementStateEnum m_ElemState;

  private:
    PeriodHandlerElement();
    PeriodHandlerElement(const PeriodHandlerElement&);
    PeriodHandlerElement& operator=(const PeriodHandlerElement&);

    PeriodInfo m_PeriodInfo;
  };

  /**
   * Queue of active period handlers.
   */
  StreamQ_type m_PeriodHandlerQueue;

  class PeriodHandlerReadQElem
  {
  public:
    StreamQ_link_type m_link;
    PeriodHandlerReadQElem(DASHMediaPeriodHandler& rPeriodHandler);
    ~PeriodHandlerReadQElem();
    DASHMediaPeriodHandler& m_rPeriodHandler;
    bool m_bIsMarkedForDeletion;

  private:
    PeriodHandlerReadQElem();
    PeriodHandlerReadQElem(const PeriodHandlerReadQElem&);
    PeriodHandlerReadQElem& operator=(const PeriodHandlerReadQElem&);
  };
  StreamQ_type m_PeriodHandlerReadQueue[NUM_MEDIA_TYPES];

  PeriodHandlerElement *m_SuspendedQElement;

  /**
   * Get a handle to the first active (not being deleted) element
   * in the periodHandlerQ.
   */
  PeriodHandlerElement* GetActivePeriodQHeadElem(StreamQ_type *pStreamQ);

  /**
   * Get a handle to the first active (not marked for deletion)
   * element in the period Handler readQ.
   */
  PeriodHandlerReadQElem* GetActiveReadQHeadElem(StreamQ_type *pStreamQ);

  /**
   * Mark a periodHandler element for deletion. There must be no
   * element before this element that is 'active' and there must
   * be no element after this one that is 'being deleted'.
   */
  void MarkPeriodQElemForDeletion(PeriodHandlerElement& rElem);

  /**
   * Mark a readQ element for deletion. There must be no element
   * before this element that is 'active' and there must be no
   * element after this one that is 'being deleted'.
   */
  void MarkReadQElemForDeletion(PeriodHandlerReadQElem& rReadQElem);

  /**
   * Purge the periodhandler elements that have been marked for
   * deletion in the periodHandlerQ and readQs. Elements are added
   * and deleted in the downloader thread context only. This
   * facilitates calling QSM close and destroy without taking
   * dashAdaptor lock. The elements that are marked for deletion
   * upto the first active head element are purged. There should
   * be no elements that are marked for deletion following an
   * 'active' element.
   */
  HTTPDownloadStatus PurgePeriodElemQueues();

  /**
   * Find the period handler element associated with the global
   * playback time, whcih is max across groups.
   * This is a helper function and returns a pointer to the queue
   * element which is used by the caller. So, caller needs to take
   * critical section when calling this function.
   */
  PeriodHandlerElement* FindPeriodHandlerElementForAdaptationSetChange(
     PeriodHandlerElement *&pSearchElem, PeriodHandlerElement *&pSuspendElem);

  bool FindAndDeletePeriodHandlerElementFromReadQ(
     int mediaType, PeriodHandlerElement *pElem);

  void SetSuspendedQElement(PeriodHandlerElement *pSuspendElem);

  /**
   * Puts the period element in front of the periodQ and marks for
   * delete.
   */
  void MovePeriodElementForPurge(PeriodHandlerElement* pElem);

 bool ShouldIgnoreEvent(uint64 nPeriodKey,
                        DASHMediaPeriodHandler::PeriodCmd eCommand);

  bool m_bSeekPending;
  SeekNotificationStatus m_eSendSeekNotification;
  int64 m_nSeekTime;

  /**
   * Set to true if there is at least one element in one of the
   * queues that needs to be deleted. Reset to false when the all
   * elements marked for delete have been purged.
   */
  bool m_bElementDeletePending;

  static const char* DASH_QSM_LIVE_LIB;
  static const char* DASH_QSM_LIVE_CREATE_SOURCE;
  static const char* DASH_QSM_LIVE_DELETE_SOURCE;

  void *m_pDashQsmLiveLib;
  IPStreamGetCreateQSM m_pQSMInstanceCreate;
  IPStreamGetDeleteQSM m_pQSMInstanceDestroy;

  //Pointer to Store QSM history
  uint8* m_pQsmHistory;
  uint32 m_nQsmHistorySize;

  char *m_pCachedTrackSelectionString;

  MM_HANDLE m_pAdaptorLock;
};

}/* namespace video */

#endif /* __DASHADAPTOR_H__ */
