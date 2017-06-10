/************************************************************************* */
/**
 * HTTPDashAdaptor.cpp
 * @brief implementation for Dash Adaptor.
 *
 * Implements and specializes HTTPDownloadHelper, to do DASH streaming.
 *
 * COPYRIGHT 2011-2013 QUALCOMM Technologies, Inc.
 * All rights reserved. QUALCOMM Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPDashAdaptor.cpp#99 $
$DateTime: 2013/12/03 01:05:22 $
$Change: 4888051 $

========================================================================== */
/* =======================================================================
**               Include files for DASHAdaptor.cpp
** ======================================================================= */
#include "Scheduler.h"
#include "HTTPDASHAdaptor.h"
#include "HTTPSessionInfo.h"

#include <SourceMemDebug.h>

#ifndef WIN32
#include <dlfcn.h>  // for dlopen/dlclose
#endif

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */
#define HTTP_DASH_DEFAULT_PORT                   "80"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

const char* DASHAdaptor::DASH_QSM_LIVE_LIB = "libmmQSM.so";
const char* DASHAdaptor::DASH_QSM_LIVE_CREATE_SOURCE = "QSMCreateInstance";
const char* DASHAdaptor::DASH_QSM_LIVE_DELETE_SOURCE = "QSMDeleteInstance";
/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief DASHAdaptor Constructor.
  *
  * @param[in] sessionInfo - Reference to HTTP session information
  * @param[in] HTTPStack - Reference to HTTP stack
  * @param[in] pHTTPStatusHandler - Interface to recevie aysnc events from stack
  * @param[in] scheduler - Reference to Scheduler
  */
DASHAdaptor::DASHAdaptor
(
 bool& bOk,
 HTTPSessionInfo& sessionInfo,
 uint32 nRequestIDfromResolver,
 HTTPStackInterface& HTTPStack,
 HTTPStatusHandlerInterface *pHTTPStatusHandler,
 Scheduler* pScheduler
)
: HTTPDownloadHelper(sessionInfo, HTTPStack),
  m_pSourceClock(NULL),
  m_bEstimator(NULL),
  m_pScheduler(pScheduler),
  m_playlistParser(sessionInfo, *pScheduler, pHTTPStatusHandler, nRequestIDfromResolver, HTTPStack),
  m_pCurrentStateHandler(NULL),
  m_initiateConnectionStateHandler(this),
  m_getFirstPeriodStateHandler(this),
  m_openPeriodStateHandler(this),
  m_playPeriodStateHandler(this),
  m_errorStateHandler(this),
  m_nBaseMediaOffset(0),
  m_nBaseMPDOffset(0),
  m_nBasePeriodOffset(0),
  m_nTotalDataDownloaded(0),
  m_nPreviousCummulativeDownload(0),
  m_bTracksSetup(false),
  m_pDASHSessionInfo(NULL),
  m_bIsEndOfStream(false),
  m_bCreatePeriod(true),
  m_SuspendedQElement(NULL),
  m_bSeekPending(false),
  m_eSendSeekNotification(SEEK_STATUS_NONE),
  m_nSeekTime(-1),
  m_bElementDeletePending(false),
  m_pDashQsmLiveLib(NULL),
  m_pQSMInstanceCreate(NULL),
  m_pQSMInstanceDestroy(NULL),
  m_pQsmHistory(NULL),
  m_nQsmHistorySize(0),
  m_pCachedTrackSelectionString(NULL),
  m_pAdaptorLock(NULL)
{
  bOk = (MM_CriticalSection_Create(&m_pAdaptorLock) == 0);
  (void)StreamQ_init(&m_PeriodHandlerQueue);

  for (int i = 0; i < NUM_MEDIA_TYPES; ++i)
  {
    (void)StreamQ_init(&m_PeriodHandlerReadQueue[i]);
  }

  if (bOk)
  {
  iMPDParser* iMPDParser =  m_playlistParser.GetMPDParserIface();
  if (iMPDParser)
  {
    bOk = iMPDParser->RegisterMPDUpdateNotification(MPDUpdateNotificationHandler, this);
  }

  if (bOk)
  {
    m_pDASHSessionInfo = QTV_New_Args(
      DASHSessionInfo,
      (GetSessionInfo(), *iMPDParser, m_HeapManager));

    if (NULL == m_pDASHSessionInfo)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Failed to allocated m_pDASHSessionInfo");
      bOk = false;
    }
  }
  }

  if (bOk)
  {
#ifdef WIN32
    m_pQSMInstanceCreate = (IPStreamGetCreateQSM)QSMCreateInstance;
    m_pQSMInstanceDestroy = (IPStreamGetDeleteQSM)QSMDeleteInstance;
#else
    m_pDashQsmLiveLib = ::dlopen(DASH_QSM_LIVE_LIB, RTLD_LAZY);

    if (NULL == m_pDashQsmLiveLib)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Failed to load libmmQSM");
      bOk = false;
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM, "libmmQSM loaded");

      m_pQSMInstanceCreate = (IPStreamGetCreateQSM)dlsym(m_pDashQsmLiveLib, DASH_QSM_LIVE_CREATE_SOURCE);

      m_pQSMInstanceDestroy = (IPStreamGetDeleteQSM)dlsym(m_pDashQsmLiveLib, DASH_QSM_LIVE_DELETE_SOURCE);

      if (NULL == m_pQSMInstanceCreate ||
          NULL == m_pQSMInstanceDestroy )
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "libmmQSM load symbols unsuccessful");
        bOk = false;
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "libmmQSM load symbols successful");
      }
    }
#endif
  }

  if (bOk)
  {
    m_pSourceClock = QTV_New_Args(StreamSourceClock, (bOk));

    if (NULL == m_pSourceClock)
    {
      bOk = false;
    }

    if (bOk)
    {
      m_bEstimator = QTV_New(HTTPBandwidthEstimator);
      if(m_bEstimator)
      {
        bOk = m_bEstimator->Initialize(m_pSourceClock);
      }
    }
  }
}

/** @brief DASHAdaptor Destructor.
  *
  */
DASHAdaptor::~DASHAdaptor()
{
#ifndef WIN32
  if (m_pDashQsmLiveLib)
  {
    int retval = ::dlclose(m_pDashQsmLiveLib);
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "libmmqsm unloaded. retval %d", retval);
  }
#endif

  if (m_pCachedTrackSelectionString)
  {
    QTV_Free(m_pCachedTrackSelectionString);
    m_pCachedTrackSelectionString = NULL;
  }

  if (m_pDASHSessionInfo)
  {
    QTV_Delete(m_pDASHSessionInfo);
    m_pDASHSessionInfo = NULL;
  }

  if (m_bEstimator)
  {
    QTV_Delete(m_bEstimator);
    m_bEstimator = NULL;
  }

  if (m_pSourceClock)
  {
    QTV_Delete(m_pSourceClock);
    m_pSourceClock = NULL;
  }

  if(m_pQsmHistory)
  {
    QTV_Free(m_pQsmHistory);
    m_pQsmHistory = NULL;
  }
  if(m_pAdaptorLock)
  {
    MM_CriticalSection_Release(m_pAdaptorLock);
    m_pAdaptorLock = NULL;
  }
}

/**
  * @brief Initiate HTTP connection and download header.
  *
  * The first step for DASH adaptor is to request the playlist
  * parser to download the playlist.
  *
  * @return
  * HTTPDL_WAITING - In progress - check back later
  * HTTPDL_SUCCESS - Successfully connected and downloaded header
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus DASHAdaptor::InitiateHTTPConnection()
{
  HTTPDownloadStatus status = video::HTTPCommon::HTTPDL_SUCCESS;
  if ( m_pCurrentStateHandler == NULL )
  {
    status = SetStateHandler(&m_initiateConnectionStateHandler);
    if (status != video::HTTPCommon::HTTPDL_SUCCESS)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "State transtion to get period failed %d", status);
    }
  }
  if ( status == video::HTTPCommon::HTTPDL_SUCCESS )
  {
    status = m_pCurrentStateHandler->InitiateHTTPConnection();
  }
  return status;
}
/**
  * @brief Pause data download
  *
  * @return
  * HTTPDL_SUCCESS - Pause successful
  * HTTPDL_WAITING - Pause command queued, will be notified later
  * HTTPDL_ERROR_ABORT - failure
  *
  */
HTTPDownloadStatus DASHAdaptor::Pause()
{
  //Pause is a NOOP for now i.e. while paused, QSM continues to download
  //data till HWM. In future Pause can be issued on the last (downloading)
  //period
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
   return status;
}

bool DASHAdaptor::GetDownloadProgress(HTTPMediaType mediaType,
                                      uint32& /* currStartOffset */,
                                      uint32& /* downloadOffset */,
                                      HTTPDownloadProgressUnitsType /* eUnitsType */,
                                      bool& bEOS)
{
  PeriodHandlerElement *pElem = GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);
  PeriodHandlerElement *pElemLast = NULL;
  bool result = false;
  while(pElem)
  {
    pElemLast = pElem;
    pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
  }
  if(pElemLast)
  {
    if(pElemLast->m_pPeriodHandler !=NULL)
    {
      bEOS = pElemLast->m_pPeriodHandler->IsEndOfMediaType(mediaType);
      result = true;
    }
  }
  return result;
}

/**
  * @brief Resume data download
  *
  * @return
  * HTTPDL_SUCCESS - Resume successful
  * HTTPDL_WAITING - Resume command queued, will be notified later
  * HTTPDL_ERROR_ABORT - failure
  *
  */
HTTPDownloadStatus DASHAdaptor::Resume()
{
  //Pause is a NOOP for now, so nothing to do for Resume!
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
   return status;
}

/**
  * @brief Seek to given time
  *
  * @return
  * HTTPDL_SUCCESS - seek successful
  * HTTPDL_WAITING - command queued will be notified later
  * HTTPDL_ERROR_ABORT - failure
  *
  */
HTTPDownloadStatus DASHAdaptor::Seek(const int64 nSeekTime)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  //If on previous period seek was not complete m_nSeekTime will store
  //the seek time(i.e end of prev period)

  //QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "DASHAdaptor::Seek nSeekTime %u m_nSeekTime %u\n", nSeekTime, m_nSeekTime);
  int64 seekTime = ((m_nSeekTime > 0) ? m_nSeekTime : nSeekTime) + m_nBaseMediaOffset;

  if (m_SuspendedQElement)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "AdaptationSet change: purge suspened element with key %u during SEEK",
      (unsigned int)(((m_SuspendedQElement->m_pPeriodHandler->GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56)));

    MovePeriodElementForPurge(m_SuspendedQElement);
    m_SuspendedQElement = NULL;
  }

  if(m_eSendSeekNotification !=SEEK_STATUS_NONE)
  {
    if(m_eSendSeekNotification == SEEK_STATUS_SUCCESS)
    {
      //Update the dash adaptor state to playing after seek is
      //complete but clear out sendtracksavail flag so that readqs
      //are not updated multiple times
      status = SetStateHandler(&m_playPeriodStateHandler);
      m_pCurrentStateHandler->Reset();
      if(UpdateMediaTrackInfo() != HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        status = HTTPCommon::HTTPDL_SUCCESS;
      }
    }
    else if(m_eSendSeekNotification == SEEK_STATUS_END)
    {
      status = HTTPCommon::HTTPDL_DATA_END;
    }
    m_bSeekPending = false;
    m_nSeekTime = -1;
    m_eSendSeekNotification = SEEK_STATUS_NONE;
  }
  else
  {
    if (false == m_bSeekPending)
    {
      m_bSeekPending = true;

      m_bIsEndOfStream = false;
      m_bCreatePeriod = true;
      // first time in this function after seek called.

      PeriodHandlerElement *pLastElem = (PeriodHandlerElement *)StreamQ_last_check(&m_PeriodHandlerQueue);
      if (pLastElem)
      {
        if (PeriodHandlerElement::PERIOD_ACTIVE == pLastElem->m_ElemState)
        {
          //Store QSM history to be passed on to new QSM
          (void)StoreQsmHistory(*(pLastElem->m_pPeriodHandler));
        }
      }

      // Find the relevant periodhandler if exists, and delete all others.
      MM_CriticalSection_Enter(m_pAdaptorLock);

      // delete all the elements in the readQs. Mark all elements in
      // the periodHandlerQ for deletion.
      for (int i = 0; i < NUM_MEDIA_TYPES; ++i)
      {
        PeriodHandlerReadQElem *pReadQElem =
          (PeriodHandlerReadQElem *)StreamQ_check(&m_PeriodHandlerReadQueue[i]);

        while (pReadQElem)
        {
          MarkReadQElemForDeletion(*pReadQElem);

          pReadQElem = (PeriodHandlerReadQElem *)StreamQ_next(
             &m_PeriodHandlerReadQueue[i], &pReadQElem->m_link);
        }
      }

      {
        PeriodHandlerElement *pElem =
          (PeriodHandlerElement *)StreamQ_check(&m_PeriodHandlerQueue);
        while (pElem)
        {
          MarkPeriodQElemForDeletion(*pElem);
          pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
        }
      }

      MM_CriticalSection_Leave(m_pAdaptorLock);

      // This will start closing all elements other than the one associated with seek (if any)
      PurgePeriodElemQueues();

      //Create period from scratch, call seek directly on newly created period.
      //As seek is like a restart it is not necessary to open the period first.
      //this might need changes if seek behaviour in periodhandler is changed
      status = InitializeAndCreatePeriod(seekTime, true);

      if (HTTPCommon::HTTPDL_SUCCESS == status)
      {
        // Return success for the seek as the new period will have the starttime as the seek time.
        PeriodHandlerElement *pPeriodToSeek =
          (PeriodHandlerElement *)GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);

        if (pPeriodToSeek)
        {
          status = pPeriodToSeek->m_pPeriodHandler->Seek(seekTime,NULL);
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Seek called on period %p for %lld", pPeriodToSeek, seekTime);
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Sanity check failed. m_PeriodHandlerQueue is empty");
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
      }
      else if(HTTPCommon::HTTPDL_DATA_END == status)
      {
        m_bSeekPending = false;
        m_nSeekTime = -1;

        //If seek went till end of clip and returned HTTPDL_DATA_END since no I-frames were found,
        //then return HTTPDL_ERROR_ABORT as it like a seek failure
        //But if seek timestamp from controller was directly on clip duration, return HTTPDL_DATA_END,
        //so that playback will terminate gracefully instead of error popup
        status = HTTPCommon::HTTPDL_ERROR_ABORT;

        uint64 start = m_nBaseMediaOffset, end = 0, duration = 0;
        if(m_playlistParser.GetEndTime(end))
        {
          duration = (end >= start) ? (end - start) : 0;
          if(nSeekTime >= (int64)duration)
          {
            status = HTTPCommon::HTTPDL_DATA_END;
          }
        }

      }
      // going into this else part will result in return code HTTPDL_WAITING to
      // taskSeekSession, so that it will call Seek again. Then, second time around
      // will find that the head element is associated with seek, but the period
      // is not "open'ed" yet so the periodHandlerSeek will immediately return
      // waiting. Then, when the period, finally gets opened, the periodHandlerSeek
      // will perform useful work.
    }
    else
    {
      PeriodHandlerElement *pSeekElem =
        (PeriodHandlerElement *)GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);
      status = (pSeekElem && pSeekElem->m_pPeriodHandler)? pSeekElem->m_pPeriodHandler->Seek(seekTime,NULL) : status;

      PurgePeriodElemQueues();
    }
  }

  return status;
}

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
HTTPDownloadStatus DASHAdaptor::InitializeAndCreatePeriod(const int64 startTime, const bool bSeek)
{
  bool bEOS = false;
  PeriodInfo periodInfo;

  HTTPDownloadStatus status = m_playlistParser.InitializePlaylistForStartTime(
    periodInfo, bEOS, startTime, bSeek);

  if (bEOS)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "End Of Stream no more periods %d", status );
    status = HTTPCommon::HTTPDL_DATA_END;
    m_bIsEndOfStream = true;
  }
  else if (status == HTTPCommon::HTTPDL_SUCCESS)
  {
    status = CreatePeriod(periodInfo, !bSeek);
  }

  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "InitializeAndCreatePeriod startTime %u, status %d, bEOS %d",
                 (uint32)startTime, status, bEOS );

  return status;
}

/**
 * @brief
 *   Instantiate a PeriodHandler instance for given period.
 *
 * @param periodInfo period info
 *
 * @return HTTPDownloadStatus
 */
HTTPDownloadStatus DASHAdaptor::CreatePeriod(PeriodInfo& periodInfo, const bool bOpen)
{
  // Create (and optionally open) this period
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

    if (m_pDASHSessionInfo)
    {
      bool bResult = false;
      DASHMediaPeriodHandler *pPeriodHander =
        QTV_New_Args( DASHMediaPeriodHandler,
                      ( bResult,
                        *m_pDASHSessionInfo,
                        m_bEstimator,
                        periodInfo,
                        (iDASHAdaptorNotifier *)this,
                        (iDASHAdaptorInfoQuery *)this,
                         m_pQSMInstanceCreate,
                         m_pQSMInstanceDestroy,
                        m_pScheduler ));

      if (pPeriodHander && bResult)
      {
        /**
         * ToDo: Hack to disable InitialRateEstimation feature for eMBMS case since this
         * feature not supported for multi dash sessions (multiview playback)
         */

        bool val = m_playlistParser.IsLiveContentHostedLocally() ? false: true;
        pPeriodHander->SetEnableInitialRateEstimation(val);

        if (m_nQsmHistorySize > 0 && m_pQsmHistory)
        {
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "CreatePeriod: SetQsmHistory on period %p history size %d",
            pPeriodHander, (int)m_nQsmHistorySize);
          pPeriodHander->SetQsmHistory(m_pQsmHistory, m_nQsmHistorySize);
        }

        PeriodHandlerElement *pElem = QTV_New_Args(
          PeriodHandlerElement, (periodInfo, pPeriodHander));

        if (pElem)
        {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "Created periodHandler %p for period %u",
                      (void *)pElem->m_pPeriodHandler,
            (uint32)((periodInfo.getPeriodKey() &(uint64)MPD_PERIOD_MASK) >> 56));

          (void)StreamQ_link(pElem, &pElem->m_link);

          MM_CriticalSection_Enter(m_pAdaptorLock);
          StreamQ_put(&m_PeriodHandlerQueue, &pElem->m_link);
          MM_CriticalSection_Leave(m_pAdaptorLock);

        if (pElem->m_pPeriodHandler && bResult == true && bOpen)
          {
          //Post an OPEN request to the period handler. For VOD nFirstSegmentStartTime
          //will be period start time and for live it is based on current time and TSB
            uint64 nFirstAvailableSegmentStartTime = MAX_UINT64;
            uint64 nFirstAvailableSegmentEndTime = MAX_UINT64;
          bool bStartOfPlayback = IsTracksSetup() ? false : true;
            HTTPDownloadStatus ret = m_playlistParser.GetFirstAvailableSegmentTimeForPeriod(periodInfo.getPeriodKey(),
              bStartOfPlayback, nFirstAvailableSegmentStartTime, nFirstAvailableSegmentEndTime);

            if(HTTPCommon::HTTPDL_SUCCESS == ret || HTTPCommon::HTTPDL_WAITING == ret)
            {
              if(HTTPCommon::HTTPDL_WAITING == ret)
              {
                nFirstAvailableSegmentStartTime = 0;
                nFirstAvailableSegmentEndTime = 0;
              }
              nFirstAvailableSegmentStartTime += periodInfo.getStartTime();
            }
            else
            {
              nFirstAvailableSegmentStartTime = MAX_INT64;
            }

            QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "DASHAdaptor::CreatePeriod period key %lu, calling open with start time %llu",
                 (uint32)((periodInfo.getPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56), nFirstAvailableSegmentStartTime );

            status = (pElem->m_pPeriodHandler)->Open(nFirstAvailableSegmentStartTime, NULL);
            if ( status != HTTPCommon::HTTPDL_WAITING )
            {
              QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Open period failed %d", status  );
            }
            else
            {
              status = SetStateHandler(&m_openPeriodStateHandler);
            }
          }
          else if( pElem->m_pPeriodHandler && bResult == true )
          {
            status = HTTPCommon::HTTPDL_SUCCESS;
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                         "Period %p created open is not called",
                         (void *)pElem->m_pPeriodHandler );
          }
          else
          {
              QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Create period %p failed %d",
                             (void *)pElem->m_pPeriodHandler, bResult  );
              status = HTTPCommon::HTTPDL_ERROR_ABORT;
          }
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Failed to allocate PeriodHandlerElement");
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
      }
      else
      {
        if(pPeriodHander)
        {
          QTV_Delete(pPeriodHander);
          pPeriodHander = NULL;
        }

        // should not come here as query for GetNextPeriod with NULL
        // returned SUCCESS before calling this function.
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Sanity check failed");
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }
    }

    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DASHAdaptor::CreatePeriod period key %u, status %d",
                 (uint32)((periodInfo.getPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56), status );

  return status;
}

/**
 * Update the readQ for all media types with the last
 * element of the periodHandlerQ. This should be invoked
 * only after open completed on the last period.
 *
 * @return bool
 */
bool DASHAdaptor::UpdateReadQs()
{
  bool rslt = true;

  MM_CriticalSection_Enter(m_pAdaptorLock);
  PeriodHandlerElement *pElem =
      (PeriodHandlerElement *)StreamQ_last_check(&m_PeriodHandlerQueue);
  if (pElem)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "UpdateReadQs adding period with key %u to the readQs",
                  (unsigned int)((pElem->m_pPeriodHandler->GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56));

    //Add the most recent period to read Q for all media types
    for (int i = 0; i < NUM_MEDIA_TYPES; ++i)
    {
      HTTPCommon::HTTPMediaType majorType = GetReadQMediaType(i);
      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "AddReadQElem majorType %d, PH %p, key %u",
                    majorType, (void *)pElem->m_pPeriodHandler,
                    (unsigned int)((pElem->m_pPeriodHandler->GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56));

      if (false == AddReadQElem(majorType, *pElem->m_pPeriodHandler))
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Failed to add media type %d to readQ", majorType);
        rslt = false;
        break;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Null last PH element");
    rslt = false;
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);

  PrintQueues();

  return rslt;
}

bool DASHAdaptor::IsEndOfFile()
{
  bool bEOF = false;

  HTTPDownloadStatus status =
    m_playlistParser.GetNextPeriod(NULL, bEOF);

  if (bEOF)
  {
    // there are no more periods that will be accessed on mpd parser.
    PeriodHandlerElement *pLastElem = (PeriodHandlerElement *)StreamQ_last_check(&m_PeriodHandlerQueue);

    if (pLastElem)
    {
      if (pLastElem->m_pPeriodHandler)
      {
        bEOF = (pLastElem->m_pPeriodHandler)->IsEndOfPeriod();

        if (bEOF)
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "HTTPAdaptor: EndOfFile reached");
        }
      }
    }
  }

  return bEOF;
}

/**
  * @brief Downlading to data is in progress.
  *
  * This is never ending loop giving the context to the HTTP
  * DASH Adaptoro download data as and when needed and available
  * from server.  The function will never returns success.
  *
  * @return
  * HTTPDL_WAITING - In process of sending a request to the
  * server to download data or waiting to response from server,
  * non blocking operation, call again later.
  * HTTPDL_ERROR_ABORT - failure
  *
  */
HTTPDownloadStatus DASHAdaptor::GetData()
{
  HTTPDownloadStatus status = video::HTTPCommon::HTTPDL_ERROR_ABORT;
  if ( m_pCurrentStateHandler )
  {
    status = m_pCurrentStateHandler->GetData();
  }

  PurgePeriodElemQueues();
  return status;
}
uint64 DASHAdaptor::GetTotalBytesReceived()
{
  uint64 downloadedSize = 0;
  if ( m_pCurrentStateHandler )
  {
    downloadedSize = m_pCurrentStateHandler->GetTotalBytesReceived();
  }
  return downloadedSize;
}
/**
  * Close HTTP connection and cleans up the PlayList parser
  *
  * @return
  * HTTPDL_WAITING - In progress - check back later
  * HTTPDL_SUCCESS - Successfully closed
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus DASHAdaptor::CloseHTTPConnection()
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "CloseHTTPConnection");

  HTTPDownloadStatus status = ClosePeriodHandlers();

  if (HTTPCommon::HTTPDL_WAITING != status)
  {
    m_playlistParser.Close();
  }

  //Set current state to NULL (IDLE) that can take only InitiateHTTPConnection
  if (m_pCurrentStateHandler)
  {
    SetStateHandler(NULL);
  }

  return status;
}

/**
  * The events from the period handler pass it on to the current
  * statehandler
  *
  * @param[in] eCommand event is in reponse to the command
  * @param[in] status response for the command processing
  * @param[in] pCbData callback data sent along the command
  *
  */
void DASHAdaptor::NotifyEvent
(
   uint64 nPeriodKey,
   DASHMediaPeriodHandler::PeriodCmd eCommand,
   HTTPDownloadStatus status,
   void* pCbData
)
{
  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Process Event %d status %d", eCommand, status);

  bool bIgnoreEvent = ShouldIgnoreEvent(nPeriodKey,eCommand);

  if (false == bIgnoreEvent)
  {
    // Process events that are not tied to HTTPAdaptor states.
    if ( eCommand == DASHMediaPeriodHandler::PERIOD_CMD_SEEK )
    {
      // This should have come from the first periodhandler in the queue.
      if ( status == HTTPCommon::HTTPDL_SUCCESS )
      {
        // seek complete
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Period seek complete, start playing the period" );
        m_bSeekPending = false;
        m_eSendSeekNotification = SEEK_STATUS_SUCCESS;
      }
      else if(status == HTTPCommon::HTTPDL_DATA_END)
      {
        MM_CriticalSection_Enter(m_pAdaptorLock);
        PeriodHandlerElement *pElem = GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);
        MM_CriticalSection_Leave(m_pAdaptorLock);

        m_bSeekPending = false;

        if(pElem && pElem->m_pPeriodHandler)
        {
          m_nSeekTime =
            (int64)pElem->m_pPeriodHandler->GetPeriodStartTime() + (int64)pElem->m_pPeriodHandler->GetPeriodDuration()*1000;
          m_nSeekTime -= (int64)m_nBaseMediaOffset;
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Seek returned data end newseektime %d", (int)m_nSeekTime );
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "seek returns DATA END on current period, no other active period is found");
        }
      }
      else
      {
        // seek failed
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Period seek failed %d", status );
        (void )SetStateHandler(&m_errorStateHandler);
      }
    }
    else if ( eCommand == DASHMediaPeriodHandler::PERIOD_CMD_RESUME )
    {
      if ( status != HTTPCommon::HTTPDL_SUCCESS )
      {
        // open period failed
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Period resume failed %d", status );
        (void )SetStateHandler(&m_errorStateHandler);
      }
    }
    else if (eCommand == DASHMediaPeriodHandler::PERIOD_CMD_ADAPTATION_SET_CHANGE_NOTIFICATION)
    {
      if ( status == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        // TO DO
        // populate cbdata to store details about the failure and trigger
        // event to notify app in case of ADD or REPLACE.
        // If failure for REMOVE then it is a critical failure. Currently,
        // only failure to remove is handled.
        SetStateHandler(&m_errorStateHandler);
      }
    }
    else
    {
      // Process events that are tied to HTTPAdaptor states.
      BaseStateHandler *pStateHandler = GetCurrentStateHandler();
      if ( pStateHandler )
      {
        pStateHandler->ProcessEvent( eCommand, status, pCbData);
      }
    }
  }
}

/**
 * Notification from PeriodHandler when end of stream in terms
 * of period download is reached.
 */
void DASHAdaptor::DownloadComplete(uint64 nPeriodKey)
{
  // TO DO: unused currently.
  // When end of period can be reliably notified from lower layers,
  // can be used then.
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "DASHAdaptor::DownloadComplete for period 0x%08x%08x",
                (uint32)(nPeriodKey >> 32), (uint32)nPeriodKey);
}

/**
  * The Global Playbacktime info query from Period handler
  * API returns the minimum of the playback time from all the groups
  * @param[out] nPlayBackTime
  *
  */
bool DASHAdaptor::GetGlobalPlayBackTime
(
 uint64& nPlayBackTime
)
{
  bool bOk = false;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_UNSUPPORTED;

  MM_CriticalSection_Enter(m_pAdaptorLock);

  nPlayBackTime = MAX_UINT64;
  uint64 nPos = 0;

  PeriodHandlerElement *pElem = GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);

  if(pElem && pElem->m_pPeriodHandler)
  {
    HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE, HTTPCommon::HTTP_TEXT_TYPE};
    for (int i = 0; i < STD_ARRAY_SIZE(mediaType); i++)
    {
      if (true == GetCurrentPlaybackPosition(mediaType[i], nPos))
      {
        nPlayBackTime = STD_MIN(nPlayBackTime, nPos);

        // bOk is true if this GetCurrentPlaybackPosition returned true for any media type.
        bOk = true;
      }
    }
  }

  MM_CriticalSection_Leave(m_pAdaptorLock);

  return bOk;
}

QSM::IDataStateProviderStatus DASHAdaptor::GetCumulativeDownloadStats(
    uint64& nTotalTimeDownloadingData, uint64& nTotalBytesDownloaded)
{
  nTotalTimeDownloadingData = 0;
  nTotalBytesDownloaded = 0;
  if (m_bEstimator)
  {
    nTotalTimeDownloadingData = m_bEstimator->GetCummulativeTimeTaken();
    nTotalBytesDownloaded = m_bEstimator->GetCummulativeDataDownloaded();
  }

  return QSM::DSP_STATUS_OK;
}

/**
 * Get the max-bitrate across all active periods.
 */
uint32 DASHAdaptor::GetMaxBitrateAcrossActivePeriods()
{
  uint32 maxBitrate = 0;
  PeriodInfo periodInfo;

  if (m_playlistParser.GetCurrentPeriod(periodInfo))
    {
    (void)m_playlistParser.GetMaxBitrateForPeriod(
        periodInfo.getPeriodKey() , maxBitrate);
    }

    MM_CriticalSection_Enter(m_pAdaptorLock);
    PeriodHandlerElement *pElem = GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);

    while (pElem)
    {
      DASHMediaPeriodHandler *pPeriod = pElem->m_pPeriodHandler;
      if (pPeriod)
      {
        uint32 maxPeriodBitrate = 0;
      (void)m_playlistParser.GetMaxBitrateForPeriod(
          pPeriod->GetPeriodKey(), maxPeriodBitrate);
        maxBitrate = QTV_MAX(maxBitrate, maxPeriodBitrate);
      }

      pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
    }
    MM_CriticalSection_Leave(m_pAdaptorLock);

  return maxBitrate;
}

/**
 * Check if adaptation set change is pending. AdaptationSet
 * change is pending if there is at least one period which has a
 * playgroup which is in a transitionary state of waiting for
 * QSM adaptation set change completed notitification.
 */
bool DASHAdaptor::IsAdaptationSetChangePending()
{
  bool bIsPending = false;

  MM_CriticalSection_Enter(m_pAdaptorLock);

  PeriodHandlerElement *pElem = GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);

  while (pElem)
  {
    if (pElem->m_pPeriodHandler)
    {
      bIsPending = pElem->m_pPeriodHandler->IsAdaptationSetChangePending();

      if (bIsPending)
      {
        // QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        //   "DASHAdaptor::IsAdaptationSetChangePending waiting on periodHandler %p",
        //   (void *)pElem->m_pPeriodHandler);
        break;
      }
    }

    pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
  }

  MM_CriticalSection_Leave(m_pAdaptorLock);

  return bIsPending;
}

void DASHAdaptor::UpdateTrackSelections()
{
  if (m_pCachedTrackSelectionString)
  {
    m_playlistParser.SetSelectionsXML(m_pCachedTrackSelectionString);
  }
}

/**
  * Transitions the current state to the new state
  *
  * @param[in] pStateHandler new state to transition to
  *
  */
HTTPDownloadStatus DASHAdaptor::SetStateHandler( BaseStateHandler *pStateHandler)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;

  if (m_pCurrentStateHandler)
  {
    status = m_pCurrentStateHandler->StateExitHandler();
  }
  if ( status == HTTPCommon::HTTPDL_SUCCESS )
  {
    m_pCurrentStateHandler = pStateHandler;
    if (m_pCurrentStateHandler)
    {
      status = m_pCurrentStateHandler->StateEntryHandler();
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASH Adaptor state transition to %d status %d",
                     m_pCurrentStateHandler->GetState(), status );
    }
  }

  return status;
}

/**
  * default handling for the events from the period handler
  *
  * @param[in] eCommand event is in reponse to the command
  * @param[in] status response for the command processing
  * @param[in] pCbData callback data sent along the command
  *
  */
void DASHAdaptor::BaseStateHandler::ProcessEvent
(
   DASHMediaPeriodHandler::PeriodCmd eCommand,
   HTTPDownloadStatus status,
   void* /* pCbData */
)
{
  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Process Event %d status %d", eCommand, status);
  // transtion to error state
  (void )m_pDASHAdaptor->SetStateHandler(
    &m_pDASHAdaptor->m_errorStateHandler);
}

/*
 * Sets the value for the given attribute
 *
 * @param[in] HTTPMediaType media type.
 * @param[in] HTTPAttribute attrType attribute to be set
 * @param[in] HTTPAttrVal  attrVal attribute value to be set
 * @return true if successful else failure
 */
bool DASHAdaptor::BaseStateHandler::SetConfig
(
  HTTPCommon::HTTPMediaType /* mediaType */,
  HTTPCommon::HTTPAttribute attrType,
  HTTPCommon::HTTPAttrVal /* attrVal */
)
{
  bool bResult = false;
  if ( attrType < HTTPCommon::HTTP_ATTR_MAX)
  {
    if(attrType == HTTPCommon::HTTP_ATTR_DURATION)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Set not allowed for attribute %d",attrType );

    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "SetConfig for invalid attribute %d",attrType );
  }
  return bResult;

}
/*
 * Gets the value of a given attribute
 *
 * @param[in] HTTPMediaType media type.
 * @param[in] HTTPAttribute attrType attribute for which value
              should be retrieved
 * @param[out] HTTPAttrVal  attrVal value of the given attribute
 *
 * @return true if successful else failure
 */
bool DASHAdaptor::BaseStateHandler::GetConfig
(
  HTTPCommon::HTTPMediaType /* mediaType */,
  HTTPCommon::HTTPAttribute attrType,
  HTTPCommon::HTTPAttrVal& attrVal
)
{
  bool bResult = false;
  if(attrType < HTTPCommon::HTTP_ATTR_MAX)
  {
    if(attrType == HTTPCommon::HTTP_ATTR_DURATION)
    {
      uint64 start = m_pDASHAdaptor->m_nBaseMediaOffset, end = 0;
        attrVal.int_attr_val = 0;
      //ToDo: Duration update for live streams (e.g. MPD update)
      if (m_pDASHAdaptor->m_playlistParser.IsLive())
      {
        bResult = true;
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "DASHAdaptor::GetConfig HTTP_ATTR_DURATION for live stream is 0");
      }
      else
      {
        if (m_pDASHAdaptor->m_playlistParser.GetEndTime(end))
        {
          attrVal.int_attr_val = (uint32)((end >= start) ? (end - start) : 0);
          bResult = true;
        }
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "DASHAdaptor::GetConfig HTTP_ATTR_DURATION start %u, end %u, duration %u",
                       (uint32)start, (uint32)end, (uint32)attrVal.int_attr_val );
      }
    }
    else if (attrType == HTTPCommon::HTTP_ATTR_PLAYBACK_DISCONTINUITY)
    {
      //Check if playback is continuous (used while resuming a live stream)
      attrVal.bool_attr_val = false;
      bResult = true;

      if(!m_pDASHAdaptor->m_bSeekPending)
      {
      MM_CriticalSection_Enter(m_pDASHAdaptor->m_pAdaptorLock);
      PeriodHandlerElement *pElem = m_pDASHAdaptor->GetActivePeriodQHeadElem(&(m_pDASHAdaptor->m_PeriodHandlerQueue));
      MM_CriticalSection_Leave(m_pDASHAdaptor->m_pAdaptorLock);

      if (pElem && pElem->m_pPeriodHandler)
      {
        attrVal.bool_attr_val = !(pElem->m_pPeriodHandler->CanPlaybackUninterrupted());
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASHAdaptor::GetConfig HTTP_ATTR_PLAYBACK_DISCONTINUITY plem and pElem->m_pPeriodHandlerdestroyed" );
      }
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASHAdaptor::GetConfig HTTP_ATTR_PLAYBACK_DISCONTINUITY bDiscontinuity %d",
                     attrVal.bool_attr_val );
    }
    }
    else if (attrType == HTTPCommon::HTTP_ATTR_REPOSITION_RANGE)
    {
      //Compute the reposition range after tracks are setup
      attrVal.sReposRange.nMin = attrVal.sReposRange.nMax = attrVal.sReposRange.nMaxDepth = 0;
      attrVal.sReposRange.bDataEnd = false;
          bResult = true;
      if (m_pDASHAdaptor->IsTracksSetup())
      {
        uint64 offsetFromAvailTime = (uint64)m_pDASHAdaptor->m_playlistParser.GetOffsetFromAvailabilityTime();
        if (m_pDASHAdaptor->m_playlistParser.IsLive())
        {
          // Return first and last available segment start and end times as TSB boundaries
          HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
          uint64 nFirstAvailableSegmentStartTime = MAX_UINT64;
          uint64 nFirstAvailableSegmentEndTime = MAX_UINT64;
          uint64 nLastAvailableSegmentStartTime = MAX_UINT64;
          uint64 nLastAvailableSegmentEndTime = MAX_UINT64;

          status = m_pDASHAdaptor->m_playlistParser.GetFirstAvailableSegmentTimeForPlayback(
            false, nFirstAvailableSegmentStartTime, nFirstAvailableSegmentEndTime);

                if(HTTPCommon::HTTPDL_SUCCESS == status)
                {
            status = m_pDASHAdaptor->m_playlistParser.GetLastAvailableSegmentTimeForPlayback(
              nLastAvailableSegmentStartTime, nLastAvailableSegmentEndTime);
          }

          if(HTTPCommon::HTTPDL_SUCCESS == status)
          {
            if (nLastAvailableSegmentEndTime > m_pDASHAdaptor->m_nBaseMPDOffset)
            {
              attrVal.sReposRange.nMax =  nLastAvailableSegmentEndTime - m_pDASHAdaptor->m_nBaseMPDOffset;
              attrVal.sReposRange.nMin =
                (nFirstAvailableSegmentStartTime > m_pDASHAdaptor->m_nBaseMPDOffset) ?
                (nFirstAvailableSegmentStartTime - m_pDASHAdaptor->m_nBaseMPDOffset) : 0;
              attrVal.sReposRange.nMaxDepth = attrVal.sReposRange.nMax - attrVal.sReposRange.nMin;
            }
          }
          else if (HTTPCommon::HTTPDL_DATA_END == status)
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,"Repostion Range can not be found !!! DATA_END");
            attrVal.sReposRange.bDataEnd = true;
          }
        }
        else
        {
          //For static streams, as long as current time is past availability start time or availability
          //start time is not defined, right edge is essentially defined by the MPD end time and base
          //media offset. Left edge is always 0
          if (offsetFromAvailTime > 0)
          {
            uint64 end = 0;
            if (m_pDASHAdaptor->m_playlistParser.GetEndTime(end))
            {
              if (end > m_pDASHAdaptor->m_nBaseMPDOffset)
              {
                attrVal.sReposRange.nMax = end - m_pDASHAdaptor->m_nBaseMPDOffset;
              }
              attrVal.sReposRange.nMin = 0;
              attrVal.sReposRange.nMaxDepth = attrVal.sReposRange.nMax;
            }
            else
      {
              bResult = false;
            }
      }
    }
  }
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASHAdaptor::GetConfig HTTP_ATTR_REPOSITION_RANGE nMin %u, nMax %u",
                     (uint32)attrVal.sReposRange.nMin, (uint32)attrVal.sReposRange.nMax );
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "GetConfig for invalid attribute" );
  }

  return bResult;
}

HTTPDownloadStatus DASHAdaptor::BaseStateHandler::GetContentProtectElem(
                                          uint32 /* portIndex */,
                                          HTTPMediaType mediaType,
                                          HTTPDrmType &drmType,
                                          HTTPContentStreamType &streamType,
                                          uint32 &contentProtectionInfoSize,
                                          unsigned char* contentProtectionData)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  if (true == m_pDASHAdaptor->m_playlistParser.IsMPDAvailable() &&
      true == m_pDASHAdaptor->m_playlistParser.IsMPDValid())
  {
    if (mediaType == HTTPCommon::HTTP_UNKNOWN_TYPE)
    {
    status = m_pDASHAdaptor->m_playlistParser.GetContentProtectElem(
              drmType, contentProtectionInfoSize, contentProtectionData);
  }
    else
    {
      int currPeriodkey = 0;
      int currGrpKey = -1,currRepKey = -1;
      MM_CriticalSection_Enter(m_pDASHAdaptor->m_pAdaptorLock);

      DASHMediaPeriodHandler *pPeriodHandler =
                                    m_pDASHAdaptor->GetPeriodHandler(mediaType);
      if (pPeriodHandler )
      {
        currPeriodkey = (int)((uint64)(pPeriodHandler->GetPeriodKey() &
                                  MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);

       status = pPeriodHandler->GetCurrentKeys(mediaType,
                                               currGrpKey,
                                               currRepKey);
      }
      status = (m_pDASHAdaptor->m_playlistParser).GetCurrentPlayingContentProtectElem(
                                            mediaType, drmType, streamType,
                                            (int&)contentProtectionInfoSize,
                                            (char *)contentProtectionData,
                                         currPeriodkey, currGrpKey, currRepKey);
       MM_CriticalSection_Leave(m_pDASHAdaptor->m_pAdaptorLock);
    }
  }
  return status;
}

/*
   * method to retrieve current playing representation
   * @param[out] currRepeSize : current representation size
   * @param[out] currRepeSring : current representation string
   *
   * @return HTTPDL_SUCCESS if successful in retrieving the track information
   * else failure
   */

HTTPDownloadStatus DASHAdaptor::BaseStateHandler::GetCurrentPlayingRepInfo
(
  uint32 &currRepSize,
  unsigned char *currRepSring
)
{
   HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
   int currPeriodkey = 0;
   int currGrpKeyAudio = -1, currGrpKeyVideo = -1, currGrpKeyText = -1;
   int currRepKeyAudio = -1, currRepKeyVideo = -1, currRepKeyText = -1;
   if (true == m_pDASHAdaptor->m_playlistParser.IsMPDAvailable() &&
       true == m_pDASHAdaptor->m_playlistParser.IsMPDValid())
   {
     MM_CriticalSection_Enter(m_pDASHAdaptor->m_pAdaptorLock);


     HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE,
                                             HTTPCommon::HTTP_TEXT_TYPE};
     for (int i = 0; i < STD_ARRAY_SIZE(mediaType); i++)
     {
       DASHMediaPeriodHandler *pPeriodHandler = m_pDASHAdaptor->GetPeriodHandler(mediaType[i]);
       if (pPeriodHandler )
       {
         currPeriodkey = (int)((uint64)(pPeriodHandler->GetPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
         int currGrpKey = -1,currRepKey = -1;

         status = pPeriodHandler->GetCurrentKeys(mediaType[i],
                                                 currGrpKey,
                                                 currRepKey);
         if  (mediaType[i] == HTTPCommon::HTTP_AUDIO_TYPE )
         {
           currGrpKeyAudio = currGrpKey;
           currRepKeyAudio = currRepKey ;
         }
         else if (mediaType[i] == HTTPCommon::HTTP_VIDEO_TYPE )
         {
           currGrpKeyVideo = currGrpKey;
           currRepKeyVideo = currRepKey;
         }
         else if (mediaType[i] == HTTPCommon::HTTP_TEXT_TYPE )
         {
            currGrpKeyText = currGrpKey;
            currRepKeyText = currRepKey;
         }
       }
     }
     status = (m_pDASHAdaptor->m_playlistParser).GetCurrentPlayingRepInfo(currPeriodkey,
                                                                          currGrpKeyAudio, currGrpKeyVideo, currGrpKeyText,
                                                                          currRepKeyAudio, currRepKeyVideo, currRepKeyText,
                                                                          (int &)currRepSize, (char *)currRepSring);
     MM_CriticalSection_Leave(m_pDASHAdaptor->m_pAdaptorLock);
   }

   return status;
 }

/*
   * method to retrieve tracks minor type from mpd first period
   * @param[out] audio : current representation size
   * @param[out] video : current representation string
   * @param[out] other : current representation string
   *
   * @return HTTPDL_SUCCESS if successful in retrieving the track information
   * else failure
   */
HTTPDownloadStatus DASHAdaptor::BaseStateHandler::GetTrackEncodingInfo(FileSourceMnMediaType& audio,
                                                                       FileSourceMnMediaType& video,
                                                                       FileSourceMnMediaType& other)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  if(m_pDASHAdaptor)
  {
    status = (m_pDASHAdaptor->m_playlistParser).GetTrackEncodingInfo(audio, video, other);
  }

  return status;
}

uint64 DASHAdaptor::BaseStateHandler::GetTotalBytesReceived()
{
  uint64 cummulativeDownload = 0;
  uint64 cummulativeTime = 0;
  int64  deltaDataDownload = 0;

  m_pDASHAdaptor->GetCumulativeDownloadStats(cummulativeTime,cummulativeDownload);
  deltaDataDownload = cummulativeDownload - m_pDASHAdaptor->m_nPreviousCummulativeDownload;
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                 "GetTotalBytesReceived previousCumm = %lld" ,m_pDASHAdaptor->m_nPreviousCummulativeDownload);
  m_pDASHAdaptor->m_nPreviousCummulativeDownload = cummulativeDownload;

  if(deltaDataDownload > 0)
  {
    this->m_pDASHAdaptor->m_nTotalDataDownloaded += deltaDataDownload;
  }
  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                 "GetTotalBytesReceived delta = %lld , cummulative = %lld, total = %lld" ,
                 deltaDataDownload,cummulativeDownload,this->m_pDASHAdaptor->m_nTotalDataDownloaded);
  return this->m_pDASHAdaptor->m_nTotalDataDownloaded;
}

/**
 * Retrevies the Dash Properties from the MDP
 *
 * @param pPropertiesXMLStr
 * @param nPropertiesLen
 *
 * @return True on sucess , false on failure
 */
bool DASHAdaptor::BaseStateHandler::GetMediaProperties
(
  char *pPropertiesXMLStr,
  uint32 &nPropertiesLen
)
{
  bool status = false;
  if (true == m_pDASHAdaptor->m_playlistParser.IsMPDAvailable() &&
      true == m_pDASHAdaptor->m_playlistParser.IsMPDValid())
  {
    m_pDASHAdaptor->m_playlistParser.GetPropertiesXML(pPropertiesXMLStr,(int&)nPropertiesLen);
    status =  true;
  }
  return status;
}

bool DASHAdaptor::BaseStateHandler::SelectRepresentations(const char* pSetSelectionsXML)
{
  bool rslt = false;

  /**
   * 1. make sure it is adaptation-set change only.
   * 2. check if the adaptation-set change is allowed by checking
   * the current adaptation-set (or the pending one waiting for
   * QSM rsp instead if exists) with the new one.
   * 3. If the checks go through fine, then,
   *  -> Update the mpd with so that getSelctedRepgroup will
   *   return the updated stuff on creation of playgroups.
   *  -> Queue the cmd on each of the periods where it needs a
   *   change.
   */
  if(pSetSelectionsXML)
  {
    rslt = true;

    if (NULL == StreamQ_check(&m_pDASHAdaptor->m_PeriodHandlerQueue))
    {
      // first time
      m_pDASHAdaptor->m_playlistParser.SetSelectionsXML(pSetSelectionsXML);
    }
    else
    {
      MM_CriticalSection_Enter(m_pDASHAdaptor->m_pAdaptorLock);

      m_pDASHAdaptor->m_bIsEndOfStream = false;
      m_pDASHAdaptor->m_bCreatePeriod = true;

      PeriodHandlerElement *pSearchElem = NULL, *pSuspendElem = NULL;
      m_pDASHAdaptor->FindPeriodHandlerElementForAdaptationSetChange(pSearchElem, pSuspendElem);

      if (pSearchElem)
      {
        // Invoke adaptation-set change commands on last element of periodQ
        // and the only element in suspend if any.
        // Also suspend the element in the suspended state after the
        // adaptation-set change completes on it.
        if (pSearchElem->m_pPeriodHandler)
        {
          pSearchElem->m_pPeriodHandler->HandleAdaptationSetChangeInfo(pSetSelectionsXML);
        }
        else
        {
          rslt = false;
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Unexpected error: failed to find periodElem for adaptationset change");
        rslt = false;
      }

      MM_CriticalSection_Leave(m_pDASHAdaptor->m_pAdaptorLock);
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR," NULL SelectRepresentation String");
  }

  return rslt;
}

/**
 * @brief Retreive the MPD Text
 *
 * @param pMPDTextStr
 * @param mpdSize
 *
 * @return True if success, false otherwise
 */
bool DASHAdaptor::BaseStateHandler::GetMPDText
(
  char *pMPDTextStr,
  uint32 &mpdSize
)
{
  bool status = false;
  if (true == m_pDASHAdaptor->m_playlistParser.IsMPDAvailable() &&
      true == m_pDASHAdaptor->m_playlistParser.IsMPDValid())
  {
    m_pDASHAdaptor->m_playlistParser.GetMPDText(pMPDTextStr,mpdSize);
    status =  true;
  }
  return status;
}

/*
 * Get media track info - this call is made by MMI layer.
 *
 * @param[out] pTrackInfo Pointer to HTTPMediaTrackInfo,
 *
 * @return  number of valid tracks.
 */
uint32 DASHAdaptor::GetMediaTrackInfo
(
   HTTPMediaTrackInfo *pTrackInfo
)
{
  uint32 nNumTracks = 0;
  if ( m_pCurrentStateHandler)
  {
    nNumTracks = m_pCurrentStateHandler->GetMediaTrackInfo(pTrackInfo);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler");
  }
  return nNumTracks;
}

/*
 * Get selected media track info - redirect to the current state handler
 *
 * @param[in] HTTPMediaType major media type for which track information
 * needs to be retreived
 * @param[out] TrackInfo populates the TrackInfo on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved track info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHAdaptor::GetSelectedMediaTrackInfo
(
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaTrackInfo &TrackInfo
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  if ( m_pCurrentStateHandler)
  {
    eStatus = m_pCurrentStateHandler->GetSelectedMediaTrackInfo(majorType,
                                                                TrackInfo);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler");
  }
  return eStatus;
}


HTTPDownloadStatus DASHAdaptor::GetContentProtectElem(uint32 portIndex,
                                          HTTPMediaType mediaType,
                                          HTTPDrmType &drmType,
                                          HTTPContentStreamType &streamType,
                                          uint32 &contentProtectionInfoSize,
                                          unsigned char* contentProtectionData)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
  if ( m_pCurrentStateHandler)
  {
    eStatus = m_pCurrentStateHandler->GetContentProtectElem(portIndex,
                                                            mediaType,
                                                            drmType,
                                                            streamType,
                                                            contentProtectionInfoSize,
                                                            contentProtectionData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler");
  }
  return eStatus;
}

 HTTPDownloadStatus DASHAdaptor::GetCurrentPlayingRepInfo
 (
   uint32 &currRepSize,
   unsigned char *currRepSring
 )
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;

  if ( m_pCurrentStateHandler)
  {
    eStatus = m_pCurrentStateHandler->GetCurrentPlayingRepInfo(currRepSize,
                           currRepSring);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler");
  }

  return eStatus;
}

HTTPDownloadStatus DASHAdaptor::GetTrackEncodingInfo(FileSourceMnMediaType& audio,
                                                     FileSourceMnMediaType& video,
                                                     FileSourceMnMediaType& other)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;

  if(m_pCurrentStateHandler)
  {
    eStatus = m_pCurrentStateHandler->GetTrackEncodingInfo(audio, video, other);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  " DASHAdaptor::GetTrackEncodingInfo Invalid state handler");
  }

  return eStatus;
}

/*
 * redirect the call to the current period handler
 *
 * @param[in] nTrackID   Identifies the track for which codec data needs to
 * be retrieved
 * @param[in] HTTPMediaType major media type for which track information
 * needs to be retreived
 * @param[in] minorType   media minor type for which the codec info is being
 * requested
 * @param[out] CodecData populates the codec data on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved codec info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHAdaptor::GetCodecData
(
  uint32 nTrackID,
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaMinorType minorType,
  HTTPCodecData &CodecData
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  if ( m_pCurrentStateHandler)
  {
    eStatus = m_pCurrentStateHandler->GetCodecData(nTrackID,
                                                   majorType,
                                                   minorType,
                                                   CodecData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler");
  }
  return eStatus;
}

/*
 * redirect the call to the current state handler
 *
 * @param[in] majorType media type.
 * @param[out] pBuffer  Buffer provies the format block info to the caller
 * @param[out] pbufSize Size of the FormatBlock buffer
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved format block info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHAdaptor::GetFormatBlock
(
 HTTPCommon::HTTPMediaType majorType,
 uint8* pBuffer,
 uint32 &nBufSize
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  int idx = GetReadQArrayIdx(majorType);

  MM_CriticalSection_Enter(m_pAdaptorLock);
  if (idx >= 0 && idx < HTTPCommon::HTTP_MAX_TYPE)
  {
    PeriodHandlerReadQElem *pReadQElem =
      GetActiveReadQHeadElem(&m_PeriodHandlerReadQueue[idx]);

    if (pReadQElem)
    {
      eStatus = pReadQElem->m_rPeriodHandler.GetFormatBlock(majorType,pBuffer,nBufSize);
    }
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);

  return eStatus;
}

/**
 * @brief Retreive the MPD adapatation set properties across all
 *        periods
 *
 * @param pPropertiesStr
 * @param nPropertiesLen
 *
 * @return True if success, false otherwise
 */
bool DASHAdaptor::GetMediaProperties
(
  char *pPropertiesStr,
  uint32 &nPropertiesLen
)
{
  bool eStatus = false;
  if ( m_pCurrentStateHandler)
  {
    eStatus = m_pCurrentStateHandler->GetMediaProperties(pPropertiesStr,
                                                         nPropertiesLen);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler");
  }
  return eStatus;
}

/**
 * This Method sets the selected representations with the MPD
 *
 * @param SetSelectionsXML
 *
 * @return True if success, false otherwise
 */
bool DASHAdaptor::SelectRepresentations(const char* SetSelectionsXML)
{
  bool eStatus = false;
  if ( m_pCurrentStateHandler)
  {
    eStatus = m_pCurrentStateHandler->SelectRepresentations(SetSelectionsXML);

    if (SetSelectionsXML)
    {
      size_t reqdSize = 1 + std_strlen(SetSelectionsXML);

      if (m_pCachedTrackSelectionString)
      {
        QTV_Free(m_pCachedTrackSelectionString);
      }

      m_pCachedTrackSelectionString = (char *)QTV_Malloc(reqdSize * sizeof(char));

      if (m_pCachedTrackSelectionString)
      {
        std_strlcpy(m_pCachedTrackSelectionString, SetSelectionsXML, reqdSize);
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "DASHAdaptor::SelectRepresentations null string");
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler");
  }
  return eStatus;
}

/**
 * @brief Retreive the MPD Text
 *
 * @param pMPDTextStr
 * @param mpdSize
 *
 * @return True if success, false otherwise
 */
bool DASHAdaptor::GetMPDText
(
   char *pMPDTextStr,
   uint32 &mpdSize
)
{
  bool eStatus = false;
  if ( m_pCurrentStateHandler)
  {
    eStatus = m_pCurrentStateHandler->GetMPDText(pMPDTextStr,
                                                 mpdSize);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler");
  }
  return eStatus;
}

/**
 * @brief Retreive the QOE data
 *
 * @param bandwidth
 * @param pVideoURL
 * @param nURLSize
 * @param pIpAddr
 * @param nIPAddrSize
 *
 */
void DASHAdaptor::GetQOEData
(
  uint32 &bandwidth,
  char *pVideoURL,
  size_t& nURLSize,
  char *pIpAddr,
  size_t& nIPAddrSize)
{
  uint64 bw = 0;
  this->GetNetworkBandwidth(bw);
  bandwidth = (uint32)bw;
  this->GetVideoInfo(video::HTTPCommon::HTTP_VIDEO_TYPE,pVideoURL,nURLSize,pIpAddr,nIPAddrSize);
}

/**
 * @brief Calculate network bandwidth
 *
 * @param bandwidth
 *
 */
void DASHAdaptor::GetNetworkBandwidth(uint64& bandwidth)
{
  bandwidth = 0;
  if(m_bEstimator)
  {
    uint64 dataDownloaded = m_bEstimator->GetCummulativeDataDownloaded();
    uint32 timeTaken = m_bEstimator->GetCummulativeTimeTaken();
    if(timeTaken > 0)
    {
      bandwidth = dataDownloaded/timeTaken;
    }
  }
}

/**
 * @brief Get video info
 *
 * @param majorType
 * @param pVideoURL
 * @param nURLSize
 * @param pIPAddr
 * @param nIPAddrSize
 *
 */
void DASHAdaptor::GetVideoInfo(HTTPCommon::HTTPMediaType majorType, char* pVideoURL, size_t& nURLSize, char* pIPAddr, size_t& nIPAddrSize)
{
  int idx = GetReadQArrayIdx(majorType);
  MM_CriticalSection_Enter(m_pAdaptorLock);
  if (idx >= 0 && idx < NUM_MEDIA_TYPES)
  {
    PeriodHandlerReadQElem *pReadQElem = GetActiveReadQHeadElem(&m_PeriodHandlerReadQueue[idx]);
    if (pReadQElem)
    {
      pReadQElem->m_rPeriodHandler.GetVideoInfo(majorType,pVideoURL,nURLSize,pIPAddr,nIPAddrSize);
    }
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);
}

/*
 * redirect the call to the current state handler
 *
 * @param[in] HTTPMediaType media type.
 * @param[out]pBuffer  A pointer to the buffer into which to place the sample.
 * @param[out] nSize The size of the data buffer.
 * @param[out] sampleInfo Provides information about the sample
 *
 * @return HTTPDL_ERROR_SUCCESS if successful in retrieving the format block
 * else appropraite error code
 */
HTTPCommon::HTTPDownloadStatus DASHAdaptor::GetNextMediaSample
(
  HTTPCommon::HTTPMediaType majorType,
  uint8 *pBuffer,
  uint32 &nSize,
  HTTPSampleInfo &sampleInfo
)
{
  HTTPCommon::HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

  int idx = GetReadQArrayIdx(majorType);

  MM_CriticalSection_Enter(m_pAdaptorLock);
  if (idx >= 0 && idx < NUM_MEDIA_TYPES)
  {
    PeriodHandlerReadQElem *pReadQElem = GetActiveReadQHeadElem(&m_PeriodHandlerReadQueue[idx]);

    if (pReadQElem)
    {
      eStatus = pReadQElem->m_rPeriodHandler.GetNextMediaSample(
        majorType,pBuffer,nSize,sampleInfo);

      sampleInfo.startTime -= (sampleInfo.startTime >= m_nBaseMediaOffset ? m_nBaseMediaOffset : sampleInfo.startTime);
      sampleInfo.endTime -= (sampleInfo.endTime >= m_nBaseMediaOffset ? m_nBaseMediaOffset : sampleInfo.endTime);

      if (HTTPCommon::HTTPDL_DATA_END == eStatus || HTTPCommon::HTTPDL_DATA_END_WITH_ERROR == eStatus)
      {
        // Iterate thru the periodH Q to find the PH corresponding to this element.
        // Then,
        //  (i) If there is at least one more element following the element corresponding to
        // this readQ element, switch resource.
        //  (ii) If not, check if mpd is EOS. If EOS then no switch, if !EOS then return
        //      SWITCH which should then start returning underrun as emptyReadQ or readQElem is not readable.

        PeriodHandlerElement *pElem = GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);

        while (pElem)
        {
          if (&pReadQElem->m_rPeriodHandler == pElem->m_pPeriodHandler)
          {
            break;
          }

          pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
        }

        if (NULL == pElem)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "Sanity check failed on reading sample for mediatype %d",
                        majorType);
          PrintQueues();
        }
        else
        {
          PeriodHandlerReadQElem *pNextReadQElem =
            (PeriodHandlerReadQElem *)StreamQ_next(&m_PeriodHandlerReadQueue[idx], &pReadQElem->m_link);

          if (pNextReadQElem)
          {
            // report SWITCHING to higher layer
            pReadQElem = GetActiveReadQHeadElem(&m_PeriodHandlerReadQueue[idx]);

            DASHMediaPeriodHandler& rTmpNext = pNextReadQElem->m_rPeriodHandler;

            bool bIsSwitchableTo =
              rTmpNext.IsPeriodReadableForMajorType(majorType);

            if (false == bIsSwitchableTo)
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "Override from DL_SWITCH to underrun as adap change pending on period %llu",
                rTmpNext.GetPeriodKey());
              eStatus = HTTPCommon::HTTPDL_WAITING;
            }
            else
            {
              //Dont remove Text track, because usually text track in a period contains
              // much lesser duration as compare to A/V. Period from Text track PeriodReadQ
              // is removed when data_end triggers for AorV
              if ((pReadQElem) && (majorType != HTTPCommon::HTTP_TEXT_TYPE))
              {
                QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                              "Delete readQElem for majorType %d for period key %llu, 0x%p",
                              majorType, pReadQElem->m_rPeriodHandler.GetPeriodKey(),
                              &pReadQElem->m_rPeriodHandler);

                MarkReadQElemForDeletion(*pReadQElem);
              }

              // Dequeued the head elem

              // Text track will not trigger DL_SWITCH return code if read is not
              // complete on other tracks of the period
              if (HTTPCommon::HTTP_TEXT_TYPE == majorType)
              {
                eStatus = video::HTTPCommon::HTTPDL_WAITING;
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                              "Return HTTPDL_WAITING for mediatype %d", majorType);
              }
              else
              {
              eStatus = video::HTTPCommon::HTTPDL_SWITCH;
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                            "Return HTTPDL_SWITCH for mediatype %d", majorType);
              }

              // check to see if the head element in the periodhandlerQ can be deleted.
              PeriodHandlerElement *pPHQElem =
                GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);

              if (pPHQElem)
              {
                bool bCanDelete = true;

                for (int i = HTTPCommon::HTTP_UNKNOWN_TYPE;
                      i < HTTPCommon::HTTP_MAX_TYPE;
                      ++i)
                {
                  int tmpOtherReadQIdx = GetReadQArrayIdx((HTTPCommon::HTTPMediaType)i);

                  if (tmpOtherReadQIdx >= 0 && tmpOtherReadQIdx < HTTPCommon::HTTP_MAX_TYPE)
                  {
                    if (i == majorType)
                    {
                      continue;
                    }

                    PeriodHandlerReadQElem *pOtherReadQElem =
                      GetActiveReadQHeadElem(&m_PeriodHandlerReadQueue[tmpOtherReadQIdx]);

                    while (pOtherReadQElem)
                    {
                      if (&pOtherReadQElem->m_rPeriodHandler == pPHQElem->m_pPeriodHandler)
                      {
                        HTTPMediaTrackInfo trackInfo;
                        std_memset((void*)&trackInfo, 0x0, sizeof(trackInfo));
                        HTTPCommon::HTTPDownloadStatus eTempStatus = GetSelectedMediaTrackInfo((HTTPCommon::HTTPMediaType)i, trackInfo);

                        bool bIsOtherNextReadable = false;

                        if (HTTPCommon::HTTPDL_DATA_END == eTempStatus)
                        {
                          PeriodHandlerReadQElem *pOtherReadQElemNext = (PeriodHandlerReadQElem *)
                            StreamQ_next(&m_PeriodHandlerReadQueue[tmpOtherReadQIdx], &pOtherReadQElem->m_link);

                          if (pOtherReadQElemNext)
                          {
                            bIsOtherNextReadable = pOtherReadQElemNext->m_rPeriodHandler.IsReadable(i);
                            QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                              "IsOtherReadble (Period %u, major %d) %d",
                              (unsigned int)((pOtherReadQElemNext->m_rPeriodHandler.GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56),
                              i, bIsOtherNextReadable);
                          }
                        }

                        if(HTTPCommon::HTTPDL_UNSUPPORTED == eTempStatus || bIsOtherNextReadable)
                        {
                          //Remove the readQ element if track is unsupported.
                          //Text track will not remove any element fom the readQ.
                          //Since Text track only period is not supported, unsupported
                          //readQ elements would be removed when DL_SWITCH goes for track
                          //other then Text track
                          if(majorType != HTTPCommon::HTTP_TEXT_TYPE)
                          {
                            MarkReadQElemForDeletion(*pOtherReadQElem);
                            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                            "Deleted Period handler from ReadQ for unsupported mediatype %d", i);
                            break;
                          }
                        }
                        else
                        {
                          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                        "Not deleting PH %p, as it is needed for media type %d",
                                        (void *)&pOtherReadQElem->m_rPeriodHandler, i);
                          bCanDelete = false;
                        }

                        break;
                      }

                      pOtherReadQElem = (PeriodHandlerReadQElem *)StreamQ_next(
                        &m_PeriodHandlerReadQueue[tmpOtherReadQIdx], &pOtherReadQElem->m_link);
                    }

                    if (false == bCanDelete)
                    {
                      break;
                    }
                  }
                }

                if (bCanDelete)
                {
                  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                  "GetNextMediaSample: majorType %d, Mark for deletion PH %p",
                                  majorType, (void *)pPHQElem->m_pPeriodHandler);
                  MarkPeriodQElemForDeletion(*pPHQElem);

                  // For text track media type if bCanDelete = TRUE, means text track
                  // is the last track in the element, hence it triggers DL_SWITCH
                  if(majorType == HTTPCommon::HTTP_TEXT_TYPE)
                  {
                    if(pReadQElem)
                    {
                      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Delete readQElem for text for period key %llu, 0x%p",
                        pReadQElem->m_rPeriodHandler.GetPeriodKey(),
                       &pReadQElem->m_rPeriodHandler);
                      MarkReadQElemForDeletion(*pReadQElem);
                    }

                    eStatus = video::HTTPCommon::HTTPDL_SWITCH;
                    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                  "Return HTTPDL_SWITCH for mediatype %d", majorType);
                  }
                }
              }
            }
          }
          else if (m_SuspendedQElement)
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
              "Return underrun for mediatype %d, as suspended element exists", majorType);
            eStatus = HTTPCommon::HTTPDL_WAITING;
          }
          else if (NULL == StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link) && m_bIsEndOfStream)
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "Read sample for media %d. End of stream", majorType);
          }
          // Check if download dosent reaches to the end of the last period due
          // to some error. Notify this as dataend with error
          else if((GetCurrentStateHandler() == &m_errorStateHandler) && m_bIsEndOfStream)
          {
            PeriodHandlerElement *pLastElem = (PeriodHandlerElement *)StreamQ_last_check(&m_PeriodHandlerQueue);
            if(pLastElem && (pLastElem->m_ElemState != PeriodHandlerElement::PERIOD_ACTIVE))
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                            "Read sample for media %d. End of stream with Error", majorType);
              eStatus = video::HTTPCommon::HTTPDL_DATA_END_WITH_ERROR;
            }
            else
            {
              eStatus = video::HTTPCommon::HTTPDL_WAITING;
              QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                            "Return HTTPDL_WAITING for mediatype %d. m_bIsEndOfStream %d with adatpor in error state",
                             majorType, m_bIsEndOfStream);
            }
          }
          else
          {
            eStatus = video::HTTPCommon::HTTPDL_WAITING;
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "Return HTTPDL_WAITING for mediatype %d. m_bIsEndOfStream %d",
                          majorType, m_bIsEndOfStream);
          }
        }
      }
      else if (HTTPCommon::HTTPDL_UNSUPPORTED == eStatus)
      {
        eStatus = HTTPCommon::HTTPDL_WAITING;
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "DASHAdaptor::GetNextMediaSample returning HTTPDL_UNSUPPORTED");
      }
    }
    else
    {
      if (m_SuspendedQElement)
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                     "Element suspended for media type %d. REPORT UNDERRUN", majorType);
        eStatus = HTTPCommon::HTTPDL_WAITING;

      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "DASHAdaptor::GetNextMediaSample Unexpected NULL pReadQElem");
      }
    }
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);

  return eStatus;
}

/*
 * redirect the call to the current state handler
 *
 * @param[in] HTTPMediaType media type.
 * @param[out] nPlaybackPosition populates in time uints on success
 *
 * @return true if successful else failure
 */
bool DASHAdaptor::GetCurrentPlaybackPosition
(
  HTTPCommon::HTTPMediaType mediaType,
  uint64 &nPlaybackPosition
)
{
  bool bResult = false;
  nPlaybackPosition = 0;

  int idx = GetReadQArrayIdx(mediaType);

  MM_CriticalSection_Enter(m_pAdaptorLock);
  if (idx >= 0 && idx < HTTPCommon::HTTP_MAX_TYPE)
  {
    PeriodHandlerReadQElem *pReadQElem  =
      GetActiveReadQHeadElem(&m_PeriodHandlerReadQueue[idx]);

    if (pReadQElem)
    {
      bResult = pReadQElem->m_rPeriodHandler.GetCurrentPlaybackPosition(mediaType, nPlaybackPosition);
    }
    else
    {
      // there is no readable period handler for this media type.
      PeriodHandlerElement *pElem = GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);

      if (pElem)
      {
        if (pElem->m_pPeriodHandler)
        {
          bResult = pElem->m_pPeriodHandler->GetCurrentPlaybackPosition(
            mediaType, nPlaybackPosition);
        }
      }
    }
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DASHAdaptor::GetCurrentPlaybackPosition Unknown media type %d",
                  mediaType);
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);

  return bResult;
}

/*
 * redirect the call to the current state handler
 *
 * @param[in] HTTPMediaType media type.
 * @param[out] nPlaybackPosition populates in time uints on success
 * @param[out] nBufferedDuration populates buffered duration in time units on success
 *
 * @return true if successful else failure
 */
bool DASHAdaptor::GetDurationBuffered
(
  HTTPCommon::HTTPMediaType mediaType,
  uint64 &nPlaybackPosition,
  uint64 &nBufferedDuration
)
{
  nPlaybackPosition = 0;
  nBufferedDuration = 0;

  int idx = GetReadQArrayIdx(mediaType);

  PeriodHandlerReadQElem *pHead = NULL;

  MM_CriticalSection_Enter(m_pAdaptorLock);
  if (idx >= 0 && idx < HTTPCommon::HTTP_MAX_TYPE)
  {
    pHead = GetActiveReadQHeadElem(&m_PeriodHandlerReadQueue[idx]);

    PeriodHandlerReadQElem *pElem = pHead;

    if (pElem)
    {
      HTTPMediaTrackInfo sTrackInfo;
      if (mediaType == HTTPCommon::HTTP_AUDIO_TYPE)
      {
        sTrackInfo.audioTrackInfo.drmInfo.eDrmType = FILE_SOURCE_DRM_UNKNOWN;
      }
      else if (mediaType == HTTPCommon::HTTP_VIDEO_TYPE)
      {
        sTrackInfo.videoTrackInfo.drmInfo.eDrmType = FILE_SOURCE_DRM_UNKNOWN;
      }
      else if (mediaType == HTTPCommon::HTTP_TEXT_TYPE)
      {
        sTrackInfo.textTrackInfo.drmInfo.eDrmType = FILE_SOURCE_DRM_UNKNOWN;
      }
      bool bCont = false;
      if(pElem->m_rPeriodHandler.GetSelectedMediaTrackInfo(mediaType, sTrackInfo)
           == HTTPCommon::HTTPDL_UNSUPPORTED)
      {
        //Ideally it should never come here as if the first element in read
        //queue does not have the mediatype present then it should be removed
        //from the tracklist at mmi level as well.
        nPlaybackPosition = pElem->m_rPeriodHandler.GetPeriodStartTime();
        nBufferedDuration = (uint64)pElem->m_rPeriodHandler.GetPeriodDuration() * 1000;
        if(nBufferedDuration == 0.0)
        {
          pElem->m_rPeriodHandler.GetDurationBuffered(HTTPCommon::HTTP_UNKNOWN_TYPE,
                                                nPlaybackPosition,nBufferedDuration);
        }
      }
      else
      {
      // nPlaybackPosition is associated with the first element in the ReadQ.
      if (true == pElem->m_rPeriodHandler.GetDurationBuffered(
        mediaType,nPlaybackPosition,nBufferedDuration))
      {
          bCont = true;
        }
      }

      if(bCont)
      {
        pElem = (PeriodHandlerReadQElem *)StreamQ_next(
          &m_PeriodHandlerReadQueue[idx], &pElem->m_link);

        while (pElem)
        {
          uint64 nPbPos = 0;
          uint64 nBufDur = 0;
          if(pElem->m_rPeriodHandler.GetSelectedMediaTrackInfo(mediaType, sTrackInfo)
           == HTTPCommon::HTTPDL_UNSUPPORTED)
          {
            nPbPos = pElem->m_rPeriodHandler.GetPeriodStartTime();
            nBufDur = (uint64)pElem->m_rPeriodHandler.GetPeriodDuration() * 1000;
            if(nBufDur == 0.0)
            {
              pElem->m_rPeriodHandler.GetDurationBuffered(
                HTTPCommon::HTTP_UNKNOWN_TYPE,nPbPos,nBufDur);
            }
          }
          else
          {
            if (false == pElem->m_rPeriodHandler.GetDurationBuffered(
                  mediaType,nPbPos,nBufDur))
            {
              bCont = false;
            }
          }
          if (false == bCont)
          {
            break;
          }

          nBufferedDuration = nPbPos + nBufDur - nPlaybackPosition;

          QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "DASHAdaptor::GetDurationBuffered mediaType %d,  pbPos %u, buffDur %u",
                        mediaType, (uint32)nPlaybackPosition, (uint32)nBufferedDuration);

          pElem = (PeriodHandlerReadQElem *)StreamQ_next(&m_PeriodHandlerReadQueue[idx], &pElem->m_link);
        }
      }
    }
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DASHAdaptor::GetDurationBuffered Unknown media type %d",
                  mediaType);
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);

  static int logCount = 0;
  if (0 == logCount)
  {
    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DASHAdaptor::GetDurationBuffered Cumulative Buffer Occupancy mediaType %d, pbPos %u, buffDur %u",
                  mediaType, (uint32)nPlaybackPosition, (uint32)nBufferedDuration);
  }

  ++logCount;
  if (logCount > 25)
  {
    logCount = 0;
  }

  return true;
}

HTTPDownloadStatus DASHAdaptor::CreateNextPeriod()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  if (false == m_playlistParser.IsMPDAvailable() &&
      true == m_playlistParser.IsMPDValid())
  {
    status = HTTPCommon::HTTPDL_WAITING;
  }
  else
  {
    if (m_playlistParser.IsLive())
    {
      bool bStartOfPlayback = IsTracksSetup() ? false : true;
      status =
        InitializeAndCreatePeriod((int64)m_playlistParser.GetOffsetFromAvailabilityTimeToStartPlayback(bStartOfPlayback));
    }
    else
    {
      bool bEOS = false;
      PeriodInfo periodInfo;

      status = m_playlistParser.GetNextPeriod(&periodInfo, bEOS);

      if ( bEOS )
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "End Of Stream no more periods %d",  status );
        m_bIsEndOfStream = true;
        status = video::HTTPCommon::HTTPDL_DATA_END;
      }
      else if ( status == HTTPCommon::HTTPDL_SUCCESS )
      {
        status = CreatePeriod(periodInfo);
      }
    }
  }

  return status;
}

/*
 * redirect the call to the current state handler
 *
 * @param[in] HTTPMediaType media type.
 * @param[in] HTTPAttribute attrType attribute to be set
 * @param[in] HTTPAttrVal  attrVal attribute value to be set
 * @return true if successful else failure
 */
bool DASHAdaptor::SetConfig
(
  HTTPCommon::HTTPMediaType mediaType,
  HTTPCommon::HTTPAttribute attrType,
  HTTPCommon::HTTPAttrVal attrVal
)
{
  bool bResult = false;
  if ( m_pCurrentStateHandler)
  {
    bResult = m_pCurrentStateHandler->SetConfig(mediaType, attrType, attrVal);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler" );
  }
  return bResult;

}
/*
 * redirect the call to the current state handler
 *
 * @param[in] HTTPMediaType media type.
 * @param[in] HTTPAttribute attrType attribute for which value
              should be retrieved
 * @param[out] HTTPAttrVal  attrVal value of the given attribute
 *
 * @return true if successful else failure
 */
bool DASHAdaptor::GetConfig
(
  HTTPCommon::HTTPMediaType mediaType,
  HTTPCommon::HTTPAttribute attrType,
  HTTPCommon::HTTPAttrVal& attrVal
)
{
  bool bResult = false;
  if ( m_pCurrentStateHandler)
  {
    bResult = m_pCurrentStateHandler->GetConfig(mediaType, attrType, attrVal);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid state handler" );
  }
  return bResult;
}

/**
 * MPD may have changed.
 */
void DASHAdaptor::MPDUpdateNotificationHandler(void *privData)
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "MPDUpdate: Set flag to check for new periods");

  DASHAdaptor *pDA = (DASHAdaptor *)privData;

  if (pDA)
  {
    pDA->m_bCreatePeriod = true;
  }
}

/**
  * Initializes the DASH Adaptor
  *
  *
  * @return
  * HTTPDL_SUCCESS - Successfully initiated a request to the
  * playlist parser to download the playlist
  * HTTPDL_ERROR_ABORT - failure
  */
HTTPDownloadStatus DASHAdaptor::InitiateConnectionStateHandler::StateEntryHandler
(
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

    //Fill in default port if port already not present in URL
    char *pLaunchURL = m_pDASHAdaptor->GetLaunchURL();
    if ( m_pDASHAdaptor->ParseURL(HTTP_DASH_DEFAULT_PORT, pLaunchURL) &&
         pLaunchURL )
    {
      m_pDASHAdaptor->SetLaunchURL(pLaunchURL);
      m_pDASHAdaptor->m_playlistParser.SetURL(pLaunchURL);
      status = HTTPCommon::HTTPDL_SUCCESS;
    }

  return status;
}


/**
  * Check if MPD available and move to create period
  *
  *
  * @return
  * HTTPDL_SUCCESS - Successfully initiated a request to creat period
  * HTTPDL_ERROR_ABORT - failure
  */
HTTPDownloadStatus DASHAdaptor::InitiateConnectionStateHandler::InitiateHTTPConnection
(
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  if (false == m_pDASHAdaptor->m_playlistParser.IsMPDAvailable())
  {
    status = HTTPCommon::HTTPDL_WAITING;
  }
  else
  {
    if(false == m_pDASHAdaptor->m_playlistParser.IsMPDValid())
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "DASHAdaptor::InitiateConnectionStateHandler MPD is invalid");
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
    else
    {
      status = m_pDASHAdaptor->SetStateHandler(&m_pDASHAdaptor->m_getFirstPeriodStateHandler);
      if (status == HTTPCommon::HTTPDL_SUCCESS)
      {
        //ToDo: Hack to lower the preroll just for live (e.g. eMBMS)
        if (m_pDASHAdaptor->m_playlistParser.IsMPDValid() && m_pDASHAdaptor->m_playlistParser.IsLive())
        {
          m_pDASHAdaptor->m_sessionInfo.SetInitialPreroll(HTTP_DEFAULT_LIVE_SWITCH_PREROLL_MSEC, false);
          m_pDASHAdaptor->m_sessionInfo.SetRebufferPreroll(HTTP_DEFAULT_LIVE_SWITCH_PREROLL_MSEC, false);
        }
      }
    }
  }
  return status;
}


/**
 * This Method sets the sets the selected representations
 *
 * @param SetSelectionsXML : char array containing the selected
 *                         Representation in XML string format
 *                         as per defined TrackSelection Schema.
 *
 * @return True on success, false on failure
 */
bool DASHAdaptor::GetFirstPeriodStateHandler::SelectRepresentations(const char* SetSelectionsXML)
{
  bool status = false;

  if(SetSelectionsXML)
  {
    status = m_pDASHAdaptor->m_playlistParser.SetSelectionsXML(SetSelectionsXML);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR," NULL SelectRepresentation String");
  }
  return status;
}

/**
  * Get the details of the first period from the playlist parser
  *
  * @return
  * HTTPDL_WAITING - waiting for first period information
  * HTTPDL_SUCCESS - first period info is avaliable
  * HTTPDL_ERROR_ABORT - Otherwise
  */
HTTPDownloadStatus DASHAdaptor::GetFirstPeriodStateHandler::InitiateHTTPConnection
(
)
{
  return m_pDASHAdaptor->CreateNextPeriod();
}

/**
  * Posts a open request to the DASH period handler
  *
  * @return
  * HTTPDL_SUCCESS - Successfully initiated a request to the
  * DASH period handler to open
  * HTTPDL_ERROR_ABORT - failure
  */
HTTPDownloadStatus DASHAdaptor::OpenPeriodStateHandler::StateEntryHandler
(
)
{
  m_bIsOpenFailed = false;
  m_bWaitForNewPeriod = false;
  return HTTPCommon::HTTPDL_SUCCESS;
}

/**
  * Waits for the period to be opened, if the period is opened
  * via the callback event will trigger the transition to play
  * period
  *
  * @return
  * HTTPDL_WAITING - success and waiting for open success on
  * period
  * HTTPDL_ERROR_ABORT - failure
  */
HTTPDownloadStatus DASHAdaptor::OpenPeriodStateHandler::GetData
(
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;

  if (m_bIsOpenFailed)
  {
    m_bIsOpenFailed = false;

    // this period does not have any element in the readQ's. Safe to delete it.
    PeriodHandlerElement *pLastElem = (PeriodHandlerElement *)StreamQ_last_check(
      &m_pDASHAdaptor->m_PeriodHandlerQueue);

    if (pLastElem)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "DASHAdaptor::OpenPeriodStateHandler Process open failure on period 0x%p",
                    pLastElem->m_pPeriodHandler);

      if (PeriodHandlerElement::PERIOD_ACTIVE == pLastElem->m_ElemState)
      {
        m_pDASHAdaptor->MarkPeriodQElemForDeletion(*pLastElem);
      }

      if (m_pDASHAdaptor->m_SuspendedQElement)
      {
        MoveSuspendedElementToPeriodQ();
        status = HTTPCommon::HTTPDL_SUCCESS;
      }
      else
      {
        status = m_pDASHAdaptor->CreateNextPeriod();
      }

      if (HTTPCommon::HTTPDL_WAITING == status)
      {
        // set this flag so that this state will try to add a new period
        // when MPD is refreshed.
        m_bWaitForNewPeriod = true;
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "DASHAdaptor: Unexpected error. Empty PeriodHandlerQ");
      status = video::HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    if (HTTPCommon::HTTPDL_SUCCESS == status ||
        HTTPCommon::HTTPDL_WAITING == status)
    {
      status = video::HTTPCommon::HTTPDL_WAITING;
    }
    else
    {
      (void )m_pDASHAdaptor->SetStateHandler(
        &m_pDASHAdaptor->m_errorStateHandler);
    }
  }

  if (true == m_bWaitForNewPeriod)
  {
    PeriodHandlerElement *pElem = (PeriodHandlerElement *)StreamQ_last_check(
      &m_pDASHAdaptor->m_PeriodHandlerQueue);

    if (NULL == pElem ||
        (pElem && pElem->m_pPeriodHandler && pElem->m_pPeriodHandler->IsEndOfPeriod()))
    {
      // this should be the case when the last period was deleted, but no new periods
      // could be added because it is waiting for MPD refresh.
      if (m_pDASHAdaptor->m_bCreatePeriod)
      {
        // this will move it to period open state if success.
        HTTPDownloadStatus createStatus = m_pDASHAdaptor->CreateNextPeriod();

        if (HTTPCommon::HTTPDL_WAITING == createStatus)
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "Dynamic mpd use case. Waiting for mpd with added periods");

          m_pDASHAdaptor->m_bCreatePeriod = false;
        }
        else
        {
          m_bWaitForNewPeriod = false;
        }

        status = (HTTPCommon::HTTPDL_SUCCESS == createStatus ||
                  HTTPCommon::HTTPDL_WAITING == createStatus
                  ? HTTPCommon::HTTPDL_WAITING
                  : HTTPCommon::HTTPDL_ERROR_ABORT);

      }
    }
  }

  return status;
}

/**
  * handles the open command response from the period handler
  *
  * @param[in] eCommand event is in reponse to the command
  * @param[in] status for the command operation
  * @param[in] pCbData callback data sent along the command
  *
  */
void DASHAdaptor::OpenPeriodStateHandler::ProcessEvent
(
  DASHMediaPeriodHandler::PeriodCmd eCommand,
  HTTPDownloadStatus status,
  void* pCbData
)
{
  if ( eCommand == DASHMediaPeriodHandler::PERIOD_CMD_OPEN )
  {
    if ( status == HTTPCommon::HTTPDL_SUCCESS )
    {
      // opened this period
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "Period opened, start playing the period" );
      status = m_pDASHAdaptor->SetStateHandler(
        &m_pDASHAdaptor->m_playPeriodStateHandler);
    }
    else
    {
      // open period failed
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "Period open failed %d", status );

      // mark the Period as open_failed. But don't delete it right here because
      // this is called as part of periodHandler's command processing tasks. The
      // same scheduler task from which this is called may then call other
      // tasks in TaskMediaPeriod.
      m_bIsOpenFailed = true;
    }
  }
  else
  {
    // the default processing
    BaseStateHandler::ProcessEvent( eCommand, status, pCbData );
  }
}

/**
  * Updates the track info in dash adaptor by querying periodhandler
  *
  *@return - DL_SUCCESS if track info update is successfull
  *          DL_ERROR_ABORT - for any failure
  */
HTTPDownloadStatus DASHAdaptor::UpdateMediaTrackInfo()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  if (!IsTracksSetup())
  {
    // get the base time
    MM_CriticalSection_Enter(m_pAdaptorLock);
    PeriodHandlerElement *pHead = GetActivePeriodQHeadElem (&m_PeriodHandlerQueue);
    MM_CriticalSection_Leave(m_pAdaptorLock);
    if (pHead && pHead->m_pPeriodHandler)
    {
      DASHMediaPeriodHandler *pPH = pHead->m_pPeriodHandler;
      (void)pPH->GetBaseTime(m_nBaseMediaOffset, m_nBaseMPDOffset);
      m_nBasePeriodOffset = pPH->GetPeriodStartTime();
      m_nBaseMediaOffset += m_nBasePeriodOffset;
      m_nBaseMPDOffset += m_nBasePeriodOffset;
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "m_nBasePeriodOffset %d, m_nBaseMediaOffset %d, m_nBaseMPDOffset %d",
                     (uint32)m_nBasePeriodOffset, (uint32)m_nBaseMediaOffset, (uint32)m_nBaseMPDOffset);
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Unexpected: Empty period handler queue");
    }

    status = HTTPCommon::HTTPDL_TRACKS_AVALIABLE;

    SetTracksSetup(true);
  }

  if (!UpdateReadQs())
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }

  return status;
}
/**
  * The period playing wait for end of stream or download complete
  *
  * @return
  * HTTPDL_WAITING - success and waiting for eos or download complete
  * HTTPDL_ERROR_ABORT - failure
  */
HTTPDownloadStatus DASHAdaptor::PlayPeriodStateHandler::GetData
(
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  if (m_bWaitingForTracks)
  {
    m_bWaitingForTracks = false;
    status = m_pDASHAdaptor->UpdateMediaTrackInfo();
    if(status == HTTPCommon::HTTPDL_SUCCESS)
    {
      status = HTTPCommon::HTTPDL_WAITING;
    }
  }

  //Attempt creating next period, if download of current period is complete and
  //tracks are also available on the current period (i.e. OPEN is complete)
  if (status == HTTPCommon::HTTPDL_WAITING)
  {
    PeriodHandlerElement *pElem =
      (PeriodHandlerElement *)StreamQ_last_check(&m_pDASHAdaptor->m_PeriodHandlerQueue);
    if (pElem)
    {
      if (PeriodHandlerElement::PERIOD_ACTIVE == pElem->m_ElemState)
      {
        if (pElem->m_pPeriodHandler && pElem->m_pPeriodHandler->IsEndOfPeriod())
        {
          if (m_pDASHAdaptor->m_bCreatePeriod && !m_bWaitingForTracks)
          {
            //Store QSM history to be passed on to new QSM
            m_pDASHAdaptor->StoreQsmHistory(*(pElem->m_pPeriodHandler));

            if (m_pDASHAdaptor->m_SuspendedQElement)
            {
              MoveSuspendedElementToPeriodQ();
            }
            else
            {
              HTTPDownloadStatus createStatus = m_pDASHAdaptor->CreateNextPeriod();
              if (HTTPCommon::HTTPDL_WAITING == createStatus)
              {
                QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "Dynamic mpd use case. Waiting for mpd with added periods");

                m_pDASHAdaptor->m_bCreatePeriod = false;
              }
              else if (m_pDASHAdaptor->m_bIsEndOfStream)
              {
                QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "No more periods to play. Waiting...");

                m_pDASHAdaptor->m_bCreatePeriod = false;
              }
            }
          }
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "DASHAdaptor: Sanity check failed elem state is %d", pElem->m_ElemState);
      }
    }
  }

  return status;
}

void DASHAdaptor::BaseStateHandler::MoveSuspendedElementToPeriodQ()
{
  PeriodHandlerElement *pElem = m_pDASHAdaptor->m_SuspendedQElement;
  if (pElem)
  {
    DASHMediaPeriodHandler *pPeriod = pElem->m_pPeriodHandler;

    if (pPeriod)
    {
      // move the suspended element to the periodQ and call adaptation-set change on it.
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "MoveSuspendedElementToReadQ: moved period with key %llu", pPeriod->GetPeriodKey());

      StreamQ_put(&m_pDASHAdaptor->m_PeriodHandlerQueue, &pElem->m_link);

      PeriodHandlerElement *pLastElem =
        (PeriodHandlerElement *)StreamQ_last_check(&m_pDASHAdaptor->m_PeriodHandlerQueue);

      if (pLastElem)
      {
        DASHMediaPeriodHandler *pLastPeriod = pLastElem->m_pPeriodHandler;

        if (pLastPeriod->IsOpenCompleted())
        {
          // add the elements to the readQ.
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Adaptationset Change: adding period with key %u to the readQs",
                        (unsigned int)((pLastPeriod->GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56));
          m_pDASHAdaptor->UpdateReadQs();
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Adaptationset Change: not adding period with key %u to the readQs. Override to state OPEN",
                        (unsigned int)((pLastPeriod->GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56));
          m_pDASHAdaptor->SetStateHandler(&m_pDASHAdaptor->m_openPeriodStateHandler);
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Unexpected error, periodQ empty");
      }

      if (m_pDASHAdaptor->m_pCachedTrackSelectionString)
      {
        // this will be a no-op if nothing to do
        pPeriod->HandleAdaptationSetChangeInfo(m_pDASHAdaptor->m_pCachedTrackSelectionString);
      }
    }

    m_pDASHAdaptor->m_SuspendedQElement = NULL;
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Did not find suspended periodQ elem");
  }
}

/**
  * handles the seek and resume command response from the period handler
  *
  * @param[in] eCommand event is in reponse to the command
  * @param[in] status for the command operation
  * @param[in] pCbData callback data sent along the command
  *
  */
void DASHAdaptor::PlayPeriodStateHandler::ProcessEvent
(
  DASHMediaPeriodHandler::PeriodCmd eCommand,
  HTTPDownloadStatus status,
  void* pCbData
)
{
  // the default processing
  BaseStateHandler::ProcessEvent( eCommand, status, pCbData );
}

/*
 * Get (selected) media track info - redirect to the current period handler
 *
 * @param[out] pTrackInfo Pointer to HTTPMediaTrackInfo,
 *
 * @return  number of valid tracks.
 */
uint32 DASHAdaptor::PlayPeriodStateHandler::GetMediaTrackInfo
(
  HTTPMediaTrackInfo *pTrackInfo
)
{
  uint32 nNumTracks = 0;
  HTTPMediaTrackInfo sTrackInfo;
  HTTPCommon::HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  //Accumulate track info over all selected tracks in the playing (first) period in all media types
  MM_CriticalSection_Enter(m_pDASHAdaptor->m_pAdaptorLock);
  for (int idx = 0; idx < NUM_MEDIA_TYPES; ++idx)
  {
    PeriodHandlerReadQElem *pReadQElem =
      m_pDASHAdaptor->GetActiveReadQHeadElem(&m_pDASHAdaptor->m_PeriodHandlerReadQueue[idx]);

    if (pReadQElem)
    {
      sTrackInfo.audioTrackInfo.drmInfo.eDrmType = FILE_SOURCE_NO_DRM;
      sTrackInfo.videoTrackInfo.drmInfo.eDrmType = FILE_SOURCE_NO_DRM;
      sTrackInfo.textTrackInfo.drmInfo.eDrmType = FILE_SOURCE_NO_DRM;
      int currPeriodkey = 0,currGrpKey = -1,currRepKey = -1;
      currPeriodkey = (int)((uint64)(pReadQElem->m_rPeriodHandler.GetPeriodKey() &
                                  MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);

      status = pReadQElem->m_rPeriodHandler.GetCurrentKeys(m_pDASHAdaptor->GetReadQMediaType(idx),
                                               currGrpKey,
                                               currRepKey);
      status =
        pReadQElem->m_rPeriodHandler.GetSelectedMediaTrackInfo(m_pDASHAdaptor->GetReadQMediaType(idx), sTrackInfo);

      if (m_pDASHAdaptor->GetReadQMediaType(idx) == HTTPCommon::HTTP_AUDIO_TYPE)
      {
        sTrackInfo.audioTrackInfo.drmInfo.cpBufSize = 0;
        sTrackInfo.audioTrackInfo.drmInfo.pCpDataBuf = 0;
        status = (m_pDASHAdaptor->m_playlistParser).GetCurrentPlayingContentProtectElem(
                                            m_pDASHAdaptor->GetReadQMediaType(idx),
                                            (HTTPDrmType &)sTrackInfo.audioTrackInfo.drmInfo.eDrmType,
                                            sTrackInfo.audioTrackInfo.drmInfo.streamType,
                                            (int&)sTrackInfo.audioTrackInfo.drmInfo.cpBufSize,
                                            (char *)sTrackInfo.audioTrackInfo.drmInfo.pCpDataBuf,
                                            currPeriodkey, currGrpKey, currRepKey);
        sTrackInfo.audioTrackInfo.drmInfo.cpBufSize += 1;
      }
      else if (m_pDASHAdaptor->GetReadQMediaType(idx) == HTTPCommon::HTTP_VIDEO_TYPE)
      {
        sTrackInfo.videoTrackInfo.drmInfo.cpBufSize = 0;
        sTrackInfo.videoTrackInfo.drmInfo.pCpDataBuf = 0;
        status = (m_pDASHAdaptor->m_playlistParser).GetCurrentPlayingContentProtectElem(
                                            m_pDASHAdaptor->GetReadQMediaType(idx),
                                            (HTTPDrmType &)sTrackInfo.videoTrackInfo.drmInfo.eDrmType,
                                            sTrackInfo.videoTrackInfo.drmInfo.streamType,
                                            (int&)sTrackInfo.videoTrackInfo.drmInfo.cpBufSize,
                                            (char *)sTrackInfo.videoTrackInfo.drmInfo.pCpDataBuf,
                                            currPeriodkey, currGrpKey, currRepKey);
        sTrackInfo.videoTrackInfo.drmInfo.cpBufSize += 1;
      }
    }

    if (status == HTTPCommon::HTTPDL_SUCCESS)
    {
      if (pTrackInfo)
      {
        if (pTrackInfo + nNumTracks)
        {
          *(pTrackInfo + nNumTracks) = sTrackInfo;
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Insufficient space to populate track info for %u tracks", nNumTracks );
          break;
        }
      }
      nNumTracks++;
    }
  }
  MM_CriticalSection_Leave(m_pDASHAdaptor->m_pAdaptorLock);

  return nNumTracks;
}

/*
 * redirect the call to the current period handler
 *
 * @param[in] HTTPMediaType major media type for which track information
 * needs to be retreived
 * @param[out] TrackInfo populated the track information on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved track info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHAdaptor::BaseStateHandler::GetSelectedMediaTrackInfo
(
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaTrackInfo &TrackInfo
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

  MM_CriticalSection_Enter(m_pDASHAdaptor->m_pAdaptorLock);
  DASHMediaPeriodHandler *pPeriodHandler =
    m_pDASHAdaptor->GetPeriodHandler(majorType);

  if ( pPeriodHandler )
  {
    eStatus = pPeriodHandler->GetSelectedMediaTrackInfo(
      majorType, TrackInfo);
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Period handler not ready for Read on majorType %d", majorType );

    eStatus = video::HTTPCommon::HTTPDL_WAITING;
  }
  MM_CriticalSection_Leave(m_pDASHAdaptor->m_pAdaptorLock);

  return eStatus;
}

/*
 * redirect the call to the current period handler
 *
 * @param[in] nTrackID   Identifies the track for which codec data needs to
 * be retrieved
 * @param[in] HTTPMediaType major media type for which track information
 * needs to be retreived
 * @param[in] minorType   media minor type for which the codec info is being
 * requested
 * @param[out] CodecData populates the codec data on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved codec info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHAdaptor::PlayPeriodStateHandler::GetCodecData
(
  uint32 nTrackID,
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaMinorType minorType,
  HTTPCodecData &CodecData
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

  MM_CriticalSection_Enter(m_pDASHAdaptor->m_pAdaptorLock);
  DASHMediaPeriodHandler *pPeriodHandler = m_pDASHAdaptor->GetPeriodHandler(majorType);
  if ( pPeriodHandler )
  {
    eStatus = pPeriodHandler->GetCodecData(nTrackID,
                                           majorType,
                                           minorType,
                                           CodecData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid period handler" );

  }
  MM_CriticalSection_Leave(m_pDASHAdaptor->m_pAdaptorLock);
  return eStatus;
}

/**
 * Store QSM history for the Period.
 */

void DASHAdaptor::StoreQsmHistory(DASHMediaPeriodHandler& m_rPeriodHandler)
{
  m_nQsmHistorySize = 0;
  m_rPeriodHandler.StoreQsmHistory(NULL, m_nQsmHistorySize);

  if(m_nQsmHistorySize > 0)
  {
    if(m_pQsmHistory)
    {
      QTV_Free(m_pQsmHistory);
      m_pQsmHistory = NULL;
    }

    m_pQsmHistory = (uint8 *)QTV_Malloc(m_nQsmHistorySize);
    if(m_pQsmHistory)
    {
      if(m_rPeriodHandler.StoreQsmHistory(m_pQsmHistory,
                                          m_nQsmHistorySize) != true)
      {
        QTV_Free(m_pQsmHistory);
        m_pQsmHistory = NULL;
        m_nQsmHistorySize = 0;
      }
    }
  }
}

/**
 * Get array idx on m_PeriodHandlerReadQueue for majorType.
 */
int DASHAdaptor::GetReadQArrayIdx(HTTPCommon::HTTPMediaType majorType) const
{
  int arrayIdx = -1;

  switch (majorType)
  {
  case video::HTTPCommon::HTTP_AUDIO_TYPE:
    arrayIdx = 0;
  break;

  case video::HTTPCommon::HTTP_VIDEO_TYPE:
    arrayIdx = 1;
  break;

  case video::HTTPCommon::HTTP_TEXT_TYPE:
    arrayIdx = 2;
  break;

  default: break;
  }

  return arrayIdx;
}

/**
 * Get media type on m_PeriodHandlerReadQueue for array idx.
 */
HTTPCommon::HTTPMediaType DASHAdaptor::GetReadQMediaType(const int idx) const
{
  HTTPCommon::HTTPMediaType mediaType = video::HTTPCommon::HTTP_UNKNOWN_TYPE;

  switch (idx)
  {
  case 0:
    mediaType = video::HTTPCommon::HTTP_AUDIO_TYPE;
  break;

  case 1:
    mediaType = video::HTTPCommon::HTTP_VIDEO_TYPE;
  break;

  case 2:
    mediaType = video::HTTPCommon::HTTP_TEXT_TYPE;
    break;

  default: break;
  }

  return mediaType;
}

/**
 * Add ptr to PeriodHandler to the readQ for the majorType. The
 * caller must have taken criical section.
 */
bool DASHAdaptor::AddReadQElem(HTTPCommon::HTTPMediaType majorType,
                               DASHMediaPeriodHandler &rPeriodHandler)
{
  bool isOk = false;

  PeriodHandlerReadQElem *pReadQElem = QTV_New_Args(
    PeriodHandlerReadQElem, (rPeriodHandler));

  int arrayIdx = GetReadQArrayIdx(majorType);

  if (arrayIdx >= 0 && arrayIdx < NUM_MEDIA_TYPES)
  {
    if (pReadQElem)
    {
      // Insert the period handler into the track readQ even if the
      // track is not supported on the period.
      // Removal of supported or unsupported period handler from the track readQ
      // is done in read path.
      (void)StreamQ_link(pReadQElem, &pReadQElem->m_link);
      StreamQ_put(&m_PeriodHandlerReadQueue[arrayIdx] , &pReadQElem->m_link);

      isOk = true;

      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "AddReadQelem mediatype %d cnt %d",
                     arrayIdx, StreamQ_cnt(&m_PeriodHandlerReadQueue[arrayIdx]));
    }
  }

  if (false == isOk && pReadQElem)
  {
    QTV_Delete(pReadQElem);
  }

  return isOk;
}

/**
 * Get ptr to PeriodHandler to the first element in the readQ
 * for the majorType.
 */
DASHMediaPeriodHandler *DASHAdaptor::GetPeriodHandler(HTTPCommon::HTTPMediaType majorType)
{
  DASHMediaPeriodHandler *pPeriodHander = NULL;
  PeriodHandlerReadQElem *pReadQElem = NULL;

  int nReadQArrayIdx = GetReadQArrayIdx(majorType);

  if (nReadQArrayIdx >= 0 && nReadQArrayIdx < NUM_MEDIA_TYPES)
  {
    pReadQElem = GetActiveReadQHeadElem(&m_PeriodHandlerReadQueue[nReadQArrayIdx]);

    if (pReadQElem)
    {
      pPeriodHander = &pReadQElem->m_rPeriodHandler;
    }
  }

  return pPeriodHander;
}

/**
 * For debugging
 */
void DASHAdaptor::PrintQueues()
{
  MM_CriticalSection_Enter(m_pAdaptorLock);
  PeriodHandlerElement *pElem = (PeriodHandlerElement *)StreamQ_check(&m_PeriodHandlerQueue);

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "DASHAdaptor::PrintQueues: PHQsize %d", StreamQ_cnt(&m_PeriodHandlerQueue));
  while (pElem)
  {
    QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DASHAdaptor::PrintQueues: PHQueueElem elem %p, PH %p, pk %u, state %d",
                  (void *)pElem, (void *)pElem->m_pPeriodHandler,
                  (unsigned int)((pElem->m_pPeriodHandler->GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56),
                  pElem->m_ElemState);

    pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
  }

  for (int idx = 0; idx < NUM_MEDIA_TYPES; ++idx)
  {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "DASHAdaptor::PrintQueues: arrayIdx %d, PHReadQsize %d", idx, StreamQ_cnt(&m_PeriodHandlerReadQueue[idx]));
    PeriodHandlerReadQElem *pReadQElem = (PeriodHandlerReadQElem *)StreamQ_check(&m_PeriodHandlerReadQueue[idx]);

    while (pReadQElem)
    {
      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "DASHAdaptor::PrintQueues: PHReadQueueElem elem %p, PH %p, bIsDeleted %d",
                    (void *)pReadQElem, (void *)&pReadQElem->m_rPeriodHandler, pReadQElem->m_bIsMarkedForDeletion);
      pReadQElem = (PeriodHandlerReadQElem *)StreamQ_next(&m_PeriodHandlerReadQueue[idx], &pReadQElem->m_link);

    }
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);
}

/**
 * Delete the periodHandlers in all the queues.
 */
HTTPDownloadStatus DASHAdaptor::ClosePeriodHandlers()
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "DASHAdaptor::ClosePeriodHandlers");

  MM_CriticalSection_Enter(m_pAdaptorLock);
  for (int i = 0; i < NUM_MEDIA_TYPES; ++i)
  {
    PeriodHandlerReadQElem *pReadQElem =
      (PeriodHandlerReadQElem *)StreamQ_check(&m_PeriodHandlerReadQueue[i]);

    while (pReadQElem)
    {
      if (!pReadQElem->m_bIsMarkedForDeletion)
      {
        MarkReadQElemForDeletion(*pReadQElem);
        m_bElementDeletePending = true;
      }
      pReadQElem = (PeriodHandlerReadQElem *)StreamQ_next(&m_PeriodHandlerReadQueue[i], &pReadQElem->m_link);
    }
  }

  {
    PeriodHandlerElement *pElem =
      (PeriodHandlerElement *)StreamQ_check(&m_PeriodHandlerQueue);

    while (pElem)
    {
      if (PeriodHandlerElement::PERIOD_ACTIVE == pElem->m_ElemState)
      {
        pElem->m_ElemState = PeriodHandlerElement::PERIOD_MARKED_FOR_DELETION;
        m_bElementDeletePending = true;
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "ClosePeriodHandlers: Marked period %p for deletion.", pElem->m_pPeriodHandler);
      }
      pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
    }
  }

  MM_CriticalSection_Leave(m_pAdaptorLock);

  HTTPDownloadStatus status = PurgePeriodElemQueues();
  return status;
}

/**
 * Get a handle to the first active (not being deleted) element
 * in the periodHandlerQ.
 */
DASHAdaptor::PeriodHandlerElement* DASHAdaptor::GetActivePeriodQHeadElem(StreamQ_type *pStreamQ)
{
  PeriodHandlerElement *pActiveHead = NULL;

  if (pStreamQ)
  {
    PeriodHandlerElement *pElem = (PeriodHandlerElement *)StreamQ_check(pStreamQ);

    while (pElem)
    {
      if (PeriodHandlerElement::PERIOD_ACTIVE == pElem->m_ElemState)
      {
        pActiveHead = pElem;
        break;
      }

      pElem = (PeriodHandlerElement *)StreamQ_next(pStreamQ, &pElem->m_link);
    }
  }

  return pActiveHead;
}

/**
 * Get a handle to the first active (not marked for deletion)
 * element in the period Handler readQ.
 */
DASHAdaptor::PeriodHandlerReadQElem* DASHAdaptor::GetActiveReadQHeadElem(StreamQ_type *pStreamQ)
{
  PeriodHandlerReadQElem *pActiveHead = NULL;

  if (pStreamQ)
  {
    PeriodHandlerReadQElem *pElem = (PeriodHandlerReadQElem *)StreamQ_check(pStreamQ);

    while (pElem)
    {
      if (false == pElem->m_bIsMarkedForDeletion)
      {
        pActiveHead = pElem;
        break;
      }

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "Skip ReadQElem %p", &pElem->m_rPeriodHandler);

      pElem = (PeriodHandlerReadQElem *)StreamQ_next(pStreamQ, &pElem->m_link);
    }
  }

  return pActiveHead;
}

/**
 * Mark a periodHandler element for deletion. There must be no
 * element before this element that is 'active' and there must
 * be no element after this one that is 'being deleted'.
 */
void DASHAdaptor::MarkPeriodQElemForDeletion(PeriodHandlerElement& rElem)
{
  rElem.m_ElemState = PeriodHandlerElement::PERIOD_MARKED_FOR_DELETION;
  m_bElementDeletePending = true;
}

/**
 * Mark a readQ element for deletion. There must be no element
 * before this element that is 'active' and there must be no
 * element after this one that is 'being deleted'.
 */
void DASHAdaptor::MarkReadQElemForDeletion(PeriodHandlerReadQElem& rReadQElem)
{
  rReadQElem.m_bIsMarkedForDeletion = true;
  m_bElementDeletePending = true;
}

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
HTTPDownloadStatus DASHAdaptor::PurgePeriodElemQueues()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  MM_CriticalSection_Enter(m_pAdaptorLock);
  bool bElementDeletePending = m_bElementDeletePending;
  // reset this
  m_bElementDeletePending = false;
  MM_CriticalSection_Leave(m_pAdaptorLock);

  if (bElementDeletePending)
  {
    {
      PeriodHandlerElement *pElem =
        (PeriodHandlerElement *)StreamQ_check(&m_PeriodHandlerQueue);

      while (pElem)
      {
        MM_CriticalSection_Enter(m_pAdaptorLock);
        bool bIsMarkedForDeletion =
          (PeriodHandlerElement::PERIOD_MARKED_FOR_DELETION == pElem->m_ElemState);
        MM_CriticalSection_Leave(m_pAdaptorLock);

        if (!bIsMarkedForDeletion)
        {
          break;
        }

        if (pElem->m_pPeriodHandler->Close() == HTTPCommon::HTTPDL_WAITING)
        {
          m_bElementDeletePending = true;
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "PurgePeriodElemQueues: Element with Period %p moved to state QSM_DELETED",
            pElem->m_pPeriodHandler);

          pElem->m_ElemState = DASHAdaptor::PeriodHandlerElement::PERIOD_QSM_DELETED;
        }

        pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
      }
    }

    MM_CriticalSection_Enter(m_pAdaptorLock);

    for (int i = 0; i < NUM_MEDIA_TYPES; ++i)
    {
      PeriodHandlerReadQElem *pReadQElem =
        (PeriodHandlerReadQElem *)StreamQ_check(&m_PeriodHandlerReadQueue[i]);

      while (pReadQElem)
      {
        if (false == pReadQElem->m_bIsMarkedForDeletion)
        {
          break;
        }

        pReadQElem = (PeriodHandlerReadQElem *)StreamQ_get(&m_PeriodHandlerReadQueue[i]);

        if (pReadQElem)
        {
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "PurgePeriodElemQueues: PH in readQ %d for period %p deleted",
            i, &pReadQElem->m_rPeriodHandler);

          QTV_Delete(pReadQElem);
        }

        pReadQElem = (PeriodHandlerReadQElem *)StreamQ_check(&m_PeriodHandlerReadQueue[i]);
      }
    }

    {
      PeriodHandlerElement* pElem =
        (PeriodHandlerElement *)StreamQ_check(&m_PeriodHandlerQueue);

      while (pElem)
      {
        if (PeriodHandlerElement::PERIOD_QSM_DELETED != pElem->m_ElemState)
        {
          break;
        }

        pElem = (PeriodHandlerElement *)StreamQ_get(&m_PeriodHandlerQueue);

        if (pElem)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "PurgePeriodElemQueues: PH in PeriodHandlerQ for period %p deleted",
            pElem->m_pPeriodHandler);

          QTV_Delete(pElem);
        }

        pElem =
          (PeriodHandlerElement *)StreamQ_check(&m_PeriodHandlerQueue);
      }
    }

    MM_CriticalSection_Leave(m_pAdaptorLock);

    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "PurgePeriodElemQueues completed");
  }

  MM_CriticalSection_Enter(m_pAdaptorLock);
  if (StreamQ_cnt(&m_PeriodHandlerQueue) == 0)
  {
    status = HTTPCommon::HTTPDL_SUCCESS;
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);

  return status;
}

/**
 * Find the period handler element associated with the global
 * playback time, whcih is max across groups.
 * This is a helper function and returns a pointer to the queue
 * element which is used by the caller. So, caller needs to take
 * critical section when calling this function.
 */
DASHAdaptor::PeriodHandlerElement* DASHAdaptor::FindPeriodHandlerElementForAdaptationSetChange(
   PeriodHandlerElement *&pSearchElem, PeriodHandlerElement *&pSuspendElem)
{
  pSearchElem = NULL;
  pSuspendElem = NULL;

  uint64 nPlaybackTime = 0;
  GetGlobalPlayBackTime(nPlaybackTime);

  PeriodHandlerElement *pElem = GetActivePeriodQHeadElem(&m_PeriodHandlerQueue);

  if (pElem)
  {
    if (pElem->m_pPeriodHandler)
    {
      if (nPlaybackTime >= pElem->m_pPeriodHandler->GetPeriodStartTime())
      {
        while (pElem)
        {
          DASHMediaPeriodHandler *pPeriodHandler = pElem->m_pPeriodHandler;

          if (pPeriodHandler)
          {
            double periodEndTime = (double)pPeriodHandler->GetPeriodStartTime() +
              1000.0 * pPeriodHandler->GetPeriodDuration();

            if (nPlaybackTime < periodEndTime)
            {
              // found
              break;
            }
          }

          if (NULL == StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link))
          {
            // Should never come here. If it does it means that the globalPbTime
            // returned is past the periodhandlerQ. Just use the last period then.
            break;
          }

          pElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);
        }
      }
    }
  }

  if (NULL == pElem)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Did not find periodQElem for adaptationset change using searchtime "
      "of %llu", nPlaybackTime);
  }
  else
  {
    pSearchElem = pElem;
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "FindPeriodHandlerElementForAdaptationSetChange Found pSearchElem with "
      "periodKey %llu using searchtime of %llu",
      pSearchElem->m_pPeriodHandler ? pSearchElem->m_pPeriodHandler->GetPeriodKey() : MAX_UINT64,
      nPlaybackTime);

    pSuspendElem = (PeriodHandlerElement *)StreamQ_next(&m_PeriodHandlerQueue, &pElem->m_link);

    uint64 nDeletedPeriodStartTime = MAX_UINT64_VAL;

    if (pSuspendElem)
    {
      // Move all elements following pNext to the periodDeleteQ as the periodClose is async.
      bool bLoop = true;

      while (bLoop)
      {
        bLoop = false;

        PeriodHandlerElement *pLast =
          (PeriodHandlerElement *)StreamQ_last_check(&m_PeriodHandlerQueue);

        if (pLast != pSuspendElem)
        {
          PeriodHandlerElement *pHead = (PeriodHandlerElement *)StreamQ_check(&m_PeriodHandlerQueue);

          if (pHead && pHead != pLast)
          {
            for (int i = 0; i < NUM_MEDIA_TYPES; ++i)
            {
              FindAndDeletePeriodHandlerElementFromReadQ(i, pLast);
            }

            DASHMediaPeriodHandler *pDeletedPeriod = pLast->m_pPeriodHandler;
            if (pDeletedPeriod)
            {
              PeriodInfo periodInfo;
              bool bEOS = false;

              // subtract 1ms so it will fall in the earlier period
              m_playlistParser.InitializePlaylistForStartTime(
                 periodInfo, bEOS, pDeletedPeriod->GetPeriodStartTime() - 1, true);
            }

            // move element from the periodQ to the front of the Q after marking for delete.
            StreamQ_delete(&pLast->m_link);
            MovePeriodElementForPurge(pLast);

            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "FindPeriodHandlerElementForAdaptationSetChange Marked period with key %llu for delete",
                      pLast->m_pPeriodHandler ? pLast->m_pPeriodHandler->GetPeriodKey() : MAX_UINT64);

            bLoop = true;
          }
        }
      }

      SetSuspendedQElement(pSuspendElem);

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "FindPeriodHandlerElementForAdaptationSetChange Found pSuspendElem with periodKey %llu",
                    pSuspendElem->m_pPeriodHandler ? pSuspendElem->m_pPeriodHandler->GetPeriodKey() : MAX_UINT64);

      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "AdaptationSetChange: PeriodQs following calling suspend (if any)");
      PrintQueues();

      if (m_SuspendedQElement)
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "AdaptationSetChange: Suspended element with period key %u",
          (unsigned int)((m_SuspendedQElement->m_pPeriodHandler->GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56));
      }

      {
        PeriodHandlerElement *pLastElem = (PeriodHandlerElement *)StreamQ_last_check(&m_PeriodHandlerQueue);
        if (pLastElem)
        {
          DASHMediaPeriodHandler *pLastPeriod = pLastElem->m_pPeriodHandler;
          if (pLastPeriod)
          {
            if (pLastPeriod->IsOpenCompleted())
            {
              if (m_pCurrentStateHandler->GetState() != PLAY_PERIOD)
              {
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Adaptationset change: Override DA state to PLAY_PERIOD for period with key %llu",
                  pLastPeriod->GetPeriodKey());
                SetStateHandler(&m_playPeriodStateHandler);

                // Set the waitingfor tracks flag to false, otherwise, it
                // will add this to the ReadQ's again.
                m_pCurrentStateHandler->Reset();
              }
            }
            else
            {
              if (m_pCurrentStateHandler->GetState() != OPEN_PERIOD)
              {
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Adaptationset change: Override DA state to OPEN_PERIOD for period with key %llu",
                  pLastPeriod->GetPeriodKey());
                SetStateHandler(&m_openPeriodStateHandler);
              }
            }

          }
        }
      }
    }
  }

  return pElem;
}

bool DASHAdaptor::FindAndDeletePeriodHandlerElementFromReadQ(
   int mediaType, PeriodHandlerElement *pElem)
{
  bool rslt = false;

  if (mediaType >= 0 && mediaType < NUM_MEDIA_TYPES)
  {
    StreamQ_type& rReadQ = m_PeriodHandlerReadQueue[mediaType];
    PeriodHandlerReadQElem *pReadQElem = (PeriodHandlerReadQElem *)StreamQ_check(&rReadQ);

    while (pReadQElem)
    {
      if (pReadQElem->m_rPeriodHandler.GetPeriodKey() == pElem->m_pPeriodHandler->GetPeriodKey())
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "AdaptationSet change: Deleted readQelem with key %u for mediaType %d",
                      (unsigned int)(((pReadQElem->m_rPeriodHandler.GetPeriodKey() & (uint64)MPD_PERIOD_MASK)) >> 56), mediaType);

        PeriodHandlerReadQElem *pReadQHead =
          (PeriodHandlerReadQElem *)StreamQ_check(&rReadQ);

        MarkReadQElemForDeletion(*pReadQElem);
        if (pReadQHead && pReadQElem != pReadQHead)
        {
          StreamQ_delete(&pReadQElem->m_link);
          StreamQ_insert(&pReadQElem->m_link, &pReadQHead->m_link);

          rslt = true;
        }

        break;
      }

      pReadQElem = (PeriodHandlerReadQElem *)StreamQ_next(&rReadQ, &pReadQElem->m_link);
    }

  }

  return rslt;
}

void DASHAdaptor::SetSuspendedQElement(PeriodHandlerElement *pSuspendElem)
{
  if (pSuspendElem)
  {
    for (int i = 0; i < NUM_MEDIA_TYPES; ++i)
    {
      // move from readQ if pElem exists in readQ.
      FindAndDeletePeriodHandlerElementFromReadQ(i, pSuspendElem);
    }

    StreamQ_delete(&pSuspendElem->m_link);

    if (m_SuspendedQElement)
    {
      // there is already a suspended element currently. Mark that for
      // deletion and move it to the head of the periodQ so that it will
      // get purged.
      MovePeriodElementForPurge(m_SuspendedQElement);
    }

    m_SuspendedQElement = pSuspendElem;

    if (m_SuspendedQElement)
    {
      DASHMediaPeriodHandler *pSuspendPeriod = m_SuspendedQElement->m_pPeriodHandler;
      if (pSuspendPeriod)
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "Adaptationset change: suspend period with key %llu", pSuspendPeriod->GetPeriodKey());
        pSuspendPeriod->SuspendQSM();
      }
    }
  }
}

/**
 * Puts the period element in front of the periodQ and marks for
 * delete.
 */
void DASHAdaptor::MovePeriodElementForPurge(PeriodHandlerElement* pElem)
{
  if (pElem)
  {
    // there is already a suspended element currently. Mark that for
    // deletion and move it to the head of the periodQ so that it will
    // get purged.
    PeriodHandlerElement *pHead = (PeriodHandlerElement *)StreamQ_check(&m_PeriodHandlerQueue);
    if (pHead)
    {
      StreamQ_insert(&pElem->m_link, &pHead->m_link);
      MarkPeriodQElemForDeletion(*pElem);
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "MovePeriodElementForPurge: empty periodElemQ");
    }
  }
}

/**
 * Right now it ignores any OPEN_status from a command which is
 * not in the periodQ. A period won't be in the activeQ if
 * (i) it is marked purged.
 * (ii) Just got moved to suspended state. When
 * IsDownloadCompleted, there wil be a check anyways to figure
 * out in which state to put the DASHAdaptor in.
 * ### to do ### If the period is in error state at that time,
 * mark, it for purge....
 */
bool DASHAdaptor::ShouldIgnoreEvent(uint64 nPeriodKey,
                                    DASHMediaPeriodHandler::PeriodCmd eCommand)
{
  bool rslt = false;
  MM_CriticalSection_Enter(m_pAdaptorLock);
  PeriodHandlerElement *pElem =
    (PeriodHandlerElement *)StreamQ_last_check(&m_PeriodHandlerQueue);

  if (pElem)
  {
    if (pElem->m_ElemState != PeriodHandlerElement::PERIOD_ACTIVE)
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "NotifyEvent: Event for period %llu ignored as period state %d",
                    nPeriodKey, pElem->m_ElemState);
      rslt = true;
    }
    else
    {
      if (DASHMediaPeriodHandler::PERIOD_CMD_OPEN == eCommand)
      {
        if (pElem)
        {
          DASHMediaPeriodHandler *pPeriod = pElem->m_pPeriodHandler;
          if (pPeriod)
          {
            if (pPeriod->GetPeriodKey() != nPeriodKey)
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "NotifyEvent: Event for period %llu ignored", nPeriodKey);
              rslt = true;
            }
          }
        }
      }
    }
  }
  MM_CriticalSection_Leave(m_pAdaptorLock);

  return rslt;
}

DASHAdaptor::PeriodHandlerElement::PeriodHandlerElement(
  PeriodInfo& rPeriodInfo, DASHMediaPeriodHandler *pPeriodHandler)
{
  m_PeriodInfo = rPeriodInfo;
  m_pPeriodHandler = pPeriodHandler;
  m_ElemState = PERIOD_ACTIVE;

  // Initialize the periodreqaQ list element
  memset(&m_link, 0, sizeof(StreamQ_link_type));
}

DASHAdaptor::PeriodHandlerElement::~PeriodHandlerElement()
{
  if (m_pPeriodHandler)
  {
    QTV_Delete(m_pPeriodHandler);
    m_pPeriodHandler = NULL;
  }
}

DASHAdaptor::PeriodHandlerReadQElem::PeriodHandlerReadQElem(DASHMediaPeriodHandler& rPeriodHandler) :
  m_rPeriodHandler(rPeriodHandler), m_bIsMarkedForDeletion(false)
{
  // Initialize the periodreqaQ list element
  memset(&m_link, 0, sizeof(StreamQ_link_type));
}

DASHAdaptor::PeriodHandlerReadQElem::~PeriodHandlerReadQElem()
{

}

}/* namespace video */
