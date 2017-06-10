/************************************************************************* */
/**
 * DASHMediaSegmentHandler.cpp
 * @brief Implements the DASHMediaSegmentHandler. Each such object is a DASH
 *        resource that handles a DASH segment and manages all the media
 *        fragments within a segment. It uses the services of HTTP stack,
 *        data manager and file source for downloading, storing and parsing
 *        data respectively.
 *
 * COPYRIGHT 2011-2015 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/DASHMediaSegmentHandler.cpp#129 $
$DateTime: 2014/02/06 23:45:40 $
$Change: 5239579 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "DASHMediaSegmentHandler.h"
#include "HTTPDataManager.h"
#include "HTTPHeapManager.h"
#include "HTTPSegmentDataStoreStructs.h"
#include <HTTPStackInterface.h>

namespace video {

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define HTTP_MAX_SEGMENT_COMMANDS          5
#define HTTP_NONSELF_INIT_SEG_KEY          0x00000000FFFFFFFEULL
#define HTTP_INIT_SEG_KEY                  0x00000000FFFFFFFFULL
#define SEGMENT_DURATION_FRACTION_FOR_RETRY_DURATION  0.5 /* half of segment duration */
#define SEGMENT_DURATION_FRACTION_FOR_RETRY_INTERVAL 0.1 /* 1/10th of segment duration */
#define DEFAULT_SEG_INFO_GET_REQUEST_LENGTH      1200  // As per one PDU size
#define SEEK_POLL_DATA                           64 * 1024

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
/**
 * @brief SegmentDownload Manager Constructor
 */
DASHMediaSegmentHandler::SegmentDownloaderManager::~SegmentDownloaderManager()
{
  if(m_aDownloader)
  {
    QTV_Delete_Array(m_aDownloader);
  }
}

/**
 * @breif Create HTTPStack and Initialize SegmentDownloaders in
 *        the Pool
 *
 * @param pSessionInfo
 *
 * @return bool
 */
bool DASHMediaSegmentHandler::SegmentDownloaderManager::Init
(
  HTTPSessionInfo* pSessionInfo,
  HTTPStackInterface* pHTTPStack
)
{
  bool bOk = true;
  if(!pHTTPStack)
  {
    bOk = false;
  }
  else
  {
    m_pHTTPStack = pHTTPStack;
  }

  if(bOk)
  {
    bOk = false;
    if(pSessionInfo)
    {
      m_nSegmentDownloaders = pSessionInfo->GetHTTPRequestsLimit();
      if(m_nSegmentDownloaders > 0)
      {
        m_aDownloader = QTV_New_Array(SegmentDownloader,m_nSegmentDownloaders);
        if(m_aDownloader)
        {
          bOk = true;
          for (uint32 i = 0; i < m_nSegmentDownloaders; i++)
          {
            if (!m_aDownloader[i].Init(pSessionInfo, m_pHTTPStack))
            {
              bOk = false;
              break;
            }
          }
        }
      }
    }
   }
  return bOk;
}

/** @brief DASHMediaSegmentHandler constructor.
  *
  * @param[out] bResult - Status of initialization
  * @param[in] sDASHSessionInfo - Reference to DASH session info
  * @param[in] pNotifier - Reference to representation notifier
  * @param[in] pInitSegment - Reference to initialization segment
  * @param[in] pScheduler - Reference to task scheduler
  */
DASHMediaSegmentHandler::DASHMediaSegmentHandler
(
 bool& bResult,
 DASHSessionInfo& sDASHSessionInfo,
 iRepresentationNotifier* pNotifier,
 HTTPBandwidthEstimator *pBandwidthEstimator,
 SidxFetchDecision& rSidxFetchDecision,
 HttpSegmentDataStoreBase* pInitSegment,
 Scheduler* pScheduler,
 HTTPStackInterface* pHTTPStack
) : HTTPResource(bResult, sDASHSessionInfo.cMPDParser.GetMPDProfile()),
    m_bEstimator(pBandwidthEstimator),
    m_sDASHSessionInfo(sDASHSessionInfo),
    m_pRepNotifier(pNotifier),
    m_pInitSegment(pInitSegment),
    m_rSidxFetchDecision(rSidxFetchDecision),
    m_pSegmentDataLock(NULL),
    m_pSidxParser(NULL),
    m_cIdleStateHandler(this),
    m_cParseSidxStateHandler(this),
    m_cSetupStateHandler(this),
    m_cOpeningStateHandler(this),
    m_cOpenStateHandler(this),
    m_cSeekingStateHandler(this),
    m_cErrorStateHandler(this),
    m_cSegInfoRetryStateHandler(this),
    m_pCurrentStateHandler(NULL),
    m_NextCmdID(0),
    m_nStartTime(0),
    m_nDuration(0),
    m_nIndexURLPresent(-1),
    m_bInitAndMediaWithSameURL(false),
    m_pIPAddr(NULL),
    m_pHTTPStack(pHTTPStack),
    m_pScheduler(pScheduler),
    m_nTaskID(0),
    m_bProcessOpenCmd(false),
    m_bIsLmsgSet(false),
    m_bIsReadsStarted(false),
    m_bIsResourceReadDisabled(false),
    m_bIsIPAddressUpdated(false),
    m_pClock(NULL),
    m_numRetriesExcerised(0),
    m_offsetAdjust(0)
{
  //Allocate max for now, change when the API is updated to return error for
  //no sufficent space
  std_memset( &m_httpSegmentsInfo, 0, sizeof(m_httpSegmentsInfo) );
  m_httpSegmentsInfo.maxSegments = HTTP_MAX_NUMBER_OF_SEGMENTS;
  m_httpSegmentsInfo.m_pHTTPSegmentInUseInfoArray = (HTTPSegmentInfo *)
    QTV_Malloc(HTTP_MAX_NUMBER_OF_SEGMENTS * sizeof(HTTPSegmentInfo));

  //Start with IDLE state
  if (bResult)
  {
    SetStateHandler(&m_cIdleStateHandler);
    ResetDataDownloadState();

    //Create data lock
    bResult = (MM_CriticalSection_Create(&m_pSegmentDataLock) == 0);

    //Create FS lock
    bResult = (MM_CriticalSection_Create(&m_pSegmentFSLock) == 0);
  }

  //Initialize cmd queue
  if (bResult)
  {
    // max seg cmds limit =Seg Data + Seg Info will take HTTP requests Limit ,
    uint32 nMaxSegCmds =
        ((sDASHSessionInfo.sSessionInfo.GetHTTPRequestsLimit()) + (SEGMENT_CMD_MAX - 3));
    bResult = m_cCmdQ.Init(nMaxSegCmds);
  }

  if (bResult)
  {
     m_pClock = QTV_New_Args(StreamSourceClock, (bResult));
     bResult = (m_pClock != NULL)? true :false;
  }

  //Create context - use HTTP thread if pScheduler is passed in
  if (bResult)
  {
    if (m_pScheduler)
    {
      //Add TaskMediaSegment to scheduler
      bResult = false;
      SchedulerTask pTask = TaskMediaSegment;
      SegmentCmdTaskParam* pTaskParam = QTV_New_Args(SegmentCmdTaskParam, ((void*)this));
      if (pTaskParam)
      {
        m_nTaskID = m_pScheduler->AddTask(pTask, (void*)pTaskParam);
        if (!m_nTaskID)
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "TaskMediaSegment could not be added to scheduler" );
          if (pTaskParam)
          {
            QTV_Delete(pTaskParam);
            pTaskParam = NULL;
          }
        }
        else
        {
          bResult = true;
          ((SchedulerTaskParamBase*)pTaskParam)->ntaskID = m_nTaskID;
        }
      }
    }
    else
    {
      //ToDo: Create thread with TaskMediaSegment as the entry point
      bResult = false;
    }
  }

  //Initialize segment download manager
  if (bResult)
  {
    bResult = m_cSegDownloadMgr.Init(&m_sDASHSessionInfo.sSessionInfo, pHTTPStack);
  }

  //Setup data storage
  if (bResult)
  {
    bResult = SetupDataStorage(GetDataManager(), m_pInitSegment);
  }

 //Setup sidx data storage
  if(bResult)
  {
    bResult = SetupDataStorage(GetSidxDataManager(), m_pInitSegment);
  }

  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                 "Created DASH segment handler (resource) 0x%p result %d",
                 (void *)this, bResult );
}

bool DASHMediaSegmentHandler::SetupDataStorage(HTTPDataManager*          pDataManager,
                                               HttpSegmentDataStoreBase* initSegment)
{
  bool bResult = false;

  if(pDataManager)
  {
    tBufferReuseParams sParam = {0,0};
    HTTPDownloadStatus status = pDataManager->SetSessionStorageOption(iHTTPBase::SEGMENT_STORE,
                                                                      -1,
                                                                      sParam,
                                                                      (iHTTPFileSourceHelper *)this,
                                                                      NULL, NULL, NULL, NULL,
                                                                      &m_sDASHSessionInfo.cHeapManager);
    bResult = (HTTPSUCCEEDED(status));
  }

  //Create data manager non-purgable segment for init segment
  if (bResult && initSegment)
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (initSegment->IsFullyDownloaded())
    {
      status = pDataManager->CreateSegment(HTTP_NONSELF_INIT_SEG_KEY, iHTTPBase::SEGMENT_DEFAULT,
                                           0, false, initSegment);
      if (HTTPSUCCEEDED(status))
      {
        (void)pDataManager->SetSegmentComplete(HTTP_NONSELF_INIT_SEG_KEY,
                                               initSegment->GetNumBytesDownloaded());
      }
    }
    bResult = (HTTPSUCCEEDED(status));
  }

  return bResult;
}

/** @brief DASHMediaSegmentHandler destructor.
  *
  */
DASHMediaSegmentHandler::~DASHMediaSegmentHandler()
{
  ClearCmdQ();

  if (m_pSidxParser)
  {
    QTV_Delete(m_pSidxParser);
    m_pSidxParser = NULL;
  }
  if (m_pScheduler)
  {
    (void)m_pScheduler->DeleteTask(m_nTaskID);
  }
  if (m_pSegmentDataLock)
  {
    (void)MM_CriticalSection_Release(m_pSegmentDataLock);
    m_pSegmentDataLock = NULL;
  }

  if (m_pSegmentFSLock)
  {
    (void)MM_CriticalSection_Release(m_pSegmentFSLock);
    m_pSegmentFSLock = NULL;
  }

  if (m_httpSegmentsInfo.m_pHTTPSegmentInUseInfoArray)
  {
    QTV_Free(m_httpSegmentsInfo.m_pHTTPSegmentInUseInfoArray);
    m_httpSegmentsInfo.m_pHTTPSegmentInUseInfoArray= NULL;
  }

  if (m_pIPAddr)
  {
    QTV_Free(m_pIPAddr);
    m_pIPAddr = NULL;
  }

  if (m_pClock)
  {
    QTV_Free(m_pClock);
    m_pClock = NULL;
  }


  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Seg [0x%08x%08x]: Destroyed DASH segment handler (resource) 0x%p",
                 (uint32)(m_cSegmentInfo.getKey() >> 32), (uint32)m_cSegmentInfo.getKey(),(void *) this );
}

/** @brief Media segment task.
  *
  * @param[in] pParam - Task parameter
  * @return
  * 0 - SUCCESS
  * -1 - FAILURE
  */
int DASHMediaSegmentHandler::TaskMediaSegment
(
 void* pParam
)
{
  int status = -1;
  SegmentCmdTaskParam* pTaskParam = (SegmentCmdTaskParam*) pParam;

  if (pTaskParam == NULL || pTaskParam->pSelf == NULL)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid task param 0x%p", (void *)pTaskParam );
  }
  else
  {
    //Process commands in the current state
    DASHMediaSegmentHandler* pSelf = (DASHMediaSegmentHandler*)pTaskParam->pSelf;
    SegmentBaseStateHandler* pCurrStateHandler = pSelf->GetStateHandler();
    if (pCurrStateHandler)
    {
      pCurrStateHandler->ProcessCmds();
      status = 0;
    }
  }

  return status;
}

/** @brief Process all commands (in OPENING, OPEN, SEEKING states).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaSegmentHandler::SegmentBaseStateHandler::ProcessCmds()
{
  int ret = 0;
  SegmentCmdData cmd;
  void* pIt = NULL;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  uint64 nSegKey = m_pSegmentHandler->GetKey();
  while(m_pSegmentHandler->m_cCmdQ.Next(pIt, cmd))
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;

    if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGINFO)
    {
      status = m_pSegmentHandler->ProcessGetSegmentInfoCmd(cmd.sGetSegInfoCmdData.nStartTime,
                                        cmd.sGetSegInfoCmdData.nDuration);
    }
    else if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_OPEN)
    {
      //This is just a technicality, as OPEN can't even be queued post SETUP!
      status = HTTPCommon::HTTPDL_SUCCESS;
    }
    else if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA)
    {
      status = m_pSegmentHandler->ProcessGetSegmentDataCmd(cmd);
    }

    //Dequeue cmd if processed
    if (status != HTTPCommon::HTTPDL_WAITING)
    {
      (void)m_pSegmentHandler->m_cCmdQ.Remove(pIt);
    }
  }

  return ret;
}

/** @brief Queue download cmd i.e. SEGMENT_CMD_GET_SEGDATA cmd. Check if data is
  *        already present, else start downloader and enqueue the cmd to be
  *        picked up by TaskMediaSegment.
  *
  * @param[in] nDataUnitKey - Data unit key
  * @return
  * HTTPDL_SUCCESS - Download completed successfully
  * HTTPDL_WAITING - Download request queued (notified later)
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::GetSegmentData
(
 uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  //Get the data unit (fragment) offsets from sidx parser
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  SegmentCmdData cmd;
  cmd.sGetSegDataCmdData.eCmd = SEGMENT_CMD_GET_SEGDATA;
  cmd.sGetSegDataCmdData.nDataUnitKey = nDataUnitKey;
  cmd.sGetSegDataCmdData.bIsRetrying = false;
  cmd.sGetSegDataCmdData.eState = CMD_STATE_INIT;
  if (m_pSegmentHandler->EnQCmd(cmd))
  {
    status = HTTPCommon::HTTPDL_WAITING;
    QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Seg [0x%08x%08x]: GET_SEGDATA cmd queued successfully cnt %u key %u",
                   (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                   m_pSegmentHandler->m_cCmdQ.Count(), (uint32)nDataUnitKey );
  }

  return status;
}

/** @brief Provides the fragment start time and duration
  *
  * @param[in]  nDataUnitKey - Data unit key
  * @param[out] nStartTime   - Start time of the data unit
  * @param[out] nDuration    - Data unit duration
  * @return
  * HTTPDL_SUCCESS - Download completed successfully
  * HTTPDL_WAITING - Download request queued (notified later)
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::GetDataUnitInfoByKey
(
 uint64 nDataUnitKey,
 uint64 &nStartTime,
 uint64 &nDuration
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  if (m_pSegmentHandler->m_pSidxParser)
  {
    uint32 nNumTotalDataUnits = (uint32)m_pSegmentHandler->m_pSidxParser->get_data_chunk_count();
    if ((uint32)nDataUnitKey < nNumTotalDataUnits)
    {
      data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
      if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)nDataUnitKey, &sFragInfo))
      {
        nStartTime = sFragInfo.n_start_time;
        nDuration  = sFragInfo.n_subsegment_duration;
        status = HTTPCommon::HTTPDL_SUCCESS;
      }
    }
    else
    {
     QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Seg [0x%08x%08x]: Invalid data unit key %u/%u",
                    (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey() >> 32),
                    (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey()),
                    (uint32)nDataUnitKey, nNumTotalDataUnits );
    }
  }
  else
  {
    nStartTime = m_pSegmentHandler->m_nStartTime;
    nDuration  = m_pSegmentHandler->m_nDuration;
    status = HTTPCommon::HTTPDL_SUCCESS;
  }

  return status;
}

/** @brief Queue cancel download cmd i.e. SEGMENT_CMD_CANCEL_SEGDATA cmd. Check if segment is
  *        already present, else start downloader and enqueue the cmd to be
  *        picked up by TaskMediaSegment.
  *
  * @param[in] nDataUnitKey - Data unit key
  * @return
  * HTTPDL_SUCCESS - Download cancelled successfully
  * HTTPDL_WAITING - Cancel Download request queued (notified later)
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::CancelSegmentData
(
 uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPDataManager* pDataManager = m_pSegmentHandler->GetDataManager();

  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  SegmentCmdData* cmd;
  void *pIt = NULL;
  while(m_pSegmentHandler->m_cCmdQ.Next(pIt,&cmd))
  {
    if(cmd->sGetSegDataCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA
      && cmd->sGetSegDataCmdData.nDataUnitKey == nDataUnitKey)
    {
      //Moving get segment data cmd state to cancelled
      cmd->sGetSegDataCmdData.eState = CMD_STATE_CANCELLED;

      status = HTTPCommon::HTTPDL_WAITING;
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "Seg [0x%08x%08x]: Cancel Data segment (key %u)"
          "state modified to cancel", (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
          (uint32)nDataUnitKey);
    }
  }

  if(status == HTTPCommon::HTTPDL_ERROR_ABORT)
  {
    //Either command could not be queued or download for the key is already
    //complete. In any case remove the fragment from data manager.
    QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Seg [0x%08x%08x]: CancelSegmentData - GetSegmentData Cmd"
                   "for (key %u) not found",(uint32)(nSegMPDKey >> 32),
                   (uint32)nSegMPDKey,(uint32)nDataUnitKey );
    if(pDataManager)
    {
      status = pDataManager->AbortSegment(nDataUnitKey);
    }
  }

  return status;
}

HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::ContinueDownloadDataUnit
(
 uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPDataManager* pDataManager = m_pSegmentHandler->GetDataManager();

  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  SegmentCmdData* cmd;
  void *pIt = NULL;
  while(m_pSegmentHandler->m_cCmdQ.Next(pIt,&cmd))
  {
    if(cmd->sGetSegDataCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA
      && cmd->sGetSegDataCmdData.nDataUnitKey == nDataUnitKey)
    {
      m_pSegmentHandler->ReEnableResourceReads();
      SegmentDownloader *pDownloader = m_pSegmentHandler->m_cSegDownloadMgr.GetSegmentDownloader(nDataUnitKey);
      if(pDownloader)
      {
        pDownloader->DisableSocketReads(false);
        status = HTTPCommon::HTTPDL_WAITING;
      }
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "Seg [0x%08x%08x]: Continue Data segment (key %u)",
          (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
          (uint32)nDataUnitKey);
    }
  }

  return status;
}

/** @brief Get Data unit download info.
  *
  * @param[in] pDownloadInfo - Reference to QSM::CDataUnitDownloadInfo array
  * @param[in] nSize         - Size of QSM::CDataUnitDownloadInfo array
  * @param[out] nFilled      - Number of elements filled
  * @param[in] nStartTime    - Time starting from which info is requested
  * @return
  * HTTPDL_SUCCESS - Success
  * HTTPDL_INSUFFICIENT_BUFFER - Insufficient buffer
  * HTTPDL_ERROR_ABORT - In case of failures
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::GetDataUnitDownloadInfo
(
  QSM::CDataUnitDownloadInfo *pDownloadInfo,
  uint32 nSize,
  uint32 &nFilled,
  uint64 nStartTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPDataManager *pDataManager = m_pSegmentHandler->GetDataManager();
  if (pDataManager)
  {
    uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
    uint64 nCurrStartTime = m_pSegmentHandler->m_nStartTime;
    uint64 nCurrDuration = m_pSegmentHandler->m_nDuration;
    uint32 nCurrContentLength = (uint32)m_pSegmentHandler->m_cSegmentInfo.GetContentLength();
    data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
    HTTPSegmentsInfo availiableSegmentsInfo;
    HTTPSegmentInfo segmentArray[HTTP_MAX_NUMBER_OF_SEGMENTS];

    availiableSegmentsInfo.maxSegments = HTTP_MAX_NUMBER_OF_SEGMENTS;
    availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray = segmentArray;

    status = pDataManager->GetAvailableSegments(availiableSegmentsInfo, 0);
    if (status == HTTPCommon::HTTPDL_SUCCESS)
    {
      for (int i = 0; i < availiableSegmentsInfo.m_NumSegmentsInUse; i++)
      {
        //Ignore the init segment(s)
        uint64 nCurrKey = availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_Key;
        int64 nCurrStartOffset = availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_nStartOffset;

        if (nCurrKey != HTTP_INIT_SEG_KEY && nCurrKey != HTTP_NONSELF_INIT_SEG_KEY &&
            nCurrStartOffset >= 0)
        {
          //If SIDX is absent use the total segment size and duration
          if (m_pSegmentHandler->m_pSidxParser)
          {
            if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)nCurrKey, &sFragInfo))
            {
              nCurrStartTime = sFragInfo.n_start_time;
              nCurrDuration = sFragInfo.n_subsegment_duration;
              nCurrContentLength = sFragInfo.n_referenced_size;
            }
            else
            {
              QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Seg [0x%08x%08x]: Could not get frag %llu info, returning...",
                             (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, nCurrKey );
              status = HTTPCommon::HTTPDL_ERROR_ABORT;
              break;
            }
          }

          if ((nCurrStartTime + nCurrDuration)  < nStartTime)
          {
            continue;
          }

          /* Valid case. Populate info */
          if (nFilled < nSize && pDownloadInfo)
          {
            /* Populating information about Data unit */
            pDownloadInfo[nFilled].nKey = nCurrKey;
            pDownloadInfo[nFilled].nDuration = nCurrDuration;
            pDownloadInfo[nFilled].nStartTime = nCurrStartTime;
            pDownloadInfo[nFilled].nSize = nCurrContentLength;
            pDownloadInfo[nFilled].nDownloaded =
              (uint32)availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_NumBytesDownloaded;

            if ((pDownloadInfo[nFilled].nSize != pDownloadInfo[nFilled].nDownloaded) &&
                availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_bIsFullyDownloaded)
            {
              pDownloadInfo[nFilled].bFailed = true;
            }
            else
            {
              pDownloadInfo[nFilled].bFailed = false;
            }

            nFilled++;
            status = HTTPCommon::HTTPDL_SUCCESS;
          }
          /* It is a proper Insufficient buffer case, we cann't populate more
             info so lets return Insufficient buffer case */
          else if (nFilled >= nSize && pDownloadInfo)
          {
            status = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
            break;
          }
          /* Call can be to know number of data available.
             Just increment nFilled to inform caller about amount
             of data available */
          else if (!pDownloadInfo && !nSize)
          {
            nFilled++;
            status = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
          }
        }
      }
    }
  }

  return status;
}

/** @brief Fill data unit info.
  *
  * @param[in] nStartTime - Start time
  * @param[in] nDuration - Duration
  * @param[in] pDataUnitInfo - Reference to data unit info array
  * @param[in] nSizeOfDataUnitInfo - Size of data unit info array
  * @param[out] nNumDataUnitInfo - Number of data units filled/needed
  * @return
  * HTTPDL_SUCCESS - Data info successfully filled
  * HTTPDL_INSUFFICIENT_BUFFER - Insufficient buffer
  * HTTPDL_DATA_END - Request is beyond segment range
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::FillDataUnitInfo
(
 uint64 nStartTime,
 uint64 nDuration,
 QSM::CDataUnitInfo* pDataUnitInfo,
 uint32 nSizeOfDataUnitInfo,
 uint32& nNumDataUnitInfo
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  nNumDataUnitInfo = 0;
  // If sidx is available fill data unit info from sidx else indicate single data unit
  if (m_pSegmentHandler->m_pSidxParser)
  {
    //Make sure the request is for a valid range
    uint32 nNumTotalDataUnits = (uint32)m_pSegmentHandler->m_pSidxParser->get_data_chunk_count();
    uint64 nSegStart, nSegDuration;
    if (nNumTotalDataUnits > 0 &&
        m_pSegmentHandler->GetSegmentRange(nSegStart, nSegDuration))
    {
      uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
      uint64 nEndTime = nStartTime + nDuration;
      int32 i;
      uint64 nFragStartTime;
      int32 nDataUnitStartIndex = -1, nDataUnitEndIndex = -1;

      status = (nStartTime > nSegStart + nSegDuration) ? HTTPCommon::HTTPDL_DATA_END
                                                       : HTTPCommon::HTTPDL_SUCCESS;

      //Find the data unit (fragment) range based on request start time and duration
      if (status == HTTPCommon::HTTPDL_SUCCESS)
      {
        for (i = 0; i < (int32)nNumTotalDataUnits; i++)
        {
          data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
          if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)i, &sFragInfo))
          {
            nFragStartTime = sFragInfo.n_start_time;
            if (nFragStartTime >= nStartTime)
            {
              nDataUnitStartIndex = (nFragStartTime == nStartTime) ? i : i - 1;
              nDataUnitStartIndex = STD_MAX(nDataUnitStartIndex, 0);
              break;
            }
            else if (i == (int32)nNumTotalDataUnits - 1)
            {
              //Set start to last data unit if not found so far!
              nDataUnitStartIndex = i;
            }
          }
          else
          {
            //Error
            status = HTTPCommon::HTTPDL_ERROR_ABORT;
            break;
          }
        }
      }

      //If start index found (valid start time) proceed to find the end index
      if (status == HTTPCommon::HTTPDL_SUCCESS)
      {
        for (i = nDataUnitStartIndex; i < (int32)nNumTotalDataUnits; i++)
        {
          data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
          if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)i, &sFragInfo))
          {
            nFragStartTime = sFragInfo.n_start_time;
            if (nFragStartTime >= nEndTime)
            {
              nDataUnitEndIndex = i;
              break;
            }
            else if (i == (int32)nNumTotalDataUnits - 1)
            {
              //nEndTime past segment duration
              nDataUnitEndIndex = (int32)nNumTotalDataUnits;
            }
          }
          else
          {
            //Error
            status = HTTPCommon::HTTPDL_ERROR_ABORT;
            break;
          }
        }
      }

      //If data unit range for the request is found, proceed to fill data unit into
      if (status == HTTPCommon::HTTPDL_SUCCESS)
      {
        if (nDataUnitEndIndex > nDataUnitStartIndex)
        {
          //Fill the data unit info from sidx parser (which uses 0 based indexing) if
          //buffer passed in else just return the num data units
          nNumDataUnitInfo = nDataUnitEndIndex - nDataUnitStartIndex;
          if (pDataUnitInfo && nSizeOfDataUnitInfo >= nNumDataUnitInfo)
          {
            QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                           "Seg [0x%08x%08x]: Data unit (fragment) info filled - seg idx %u "
                           "nStartTime %u nDuration %u nNumDataUnitInfo %u",
                           (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                           (uint32)m_pSegmentHandler->GetKey(),
                           (uint32)nStartTime, (uint32)nDuration, nNumDataUnitInfo );
            for (int i = (int)nDataUnitStartIndex; i < (int)nDataUnitEndIndex; i++)
            {
              data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
              if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)i, &sFragInfo))
              {
                pDataUnitInfo[i - nDataUnitStartIndex].m_nKey = i;
                pDataUnitInfo[i - nDataUnitStartIndex].m_nStartTime = sFragInfo.n_start_time;
                pDataUnitInfo[i - nDataUnitStartIndex].m_nDuration = sFragInfo.n_subsegment_duration;
                // If fragment does not start with sap and sap delta time is 0 then it
                // means there is no random access point in the fragment.
                pDataUnitInfo[i - nDataUnitStartIndex].m_nRandomAccessPoint =
                  (!sFragInfo.b_starts_with_sap && sFragInfo.n_sapdelta_time == 0) ? -1: sFragInfo.n_sapdelta_time;
                pDataUnitInfo[i - nDataUnitStartIndex].m_nSize = sFragInfo.n_referenced_size;
              }
            }
          }
          else
          {
            status = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
            QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                           "Seg [0x%08x%08x]: Insufficient buffer for dataunit info %u/%u (actual/required)",
                           (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, nSizeOfDataUnitInfo, nNumDataUnitInfo );
          }
        }
        else
        {
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
          QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Seg [0x%08x%08x]: Invalid data unit range %d/%d (start/end) for %u/%u (start/end)",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, nDataUnitStartIndex, nDataUnitEndIndex,
                         (uint32)nStartTime, (uint32)nEndTime );
        }
      }// if (status == HTTPCommon::HTTPDL_SUCCESS)
    }// if (nNumTotalDataUnits > 0 && m_pSegmentHandler->GetSegmentRange(nSegStart, nSegDuration))
  }// if (m_pSegmentHandler->m_pSidxParser)
  else
  {
    nNumDataUnitInfo = 1;
    if(nSizeOfDataUnitInfo < nNumDataUnitInfo)
    {
      status = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
    }
    else
    {
      pDataUnitInfo[0].m_nDuration = m_pSegmentHandler->m_nDuration;
      pDataUnitInfo[0].m_nStartTime = m_pSegmentHandler->m_nStartTime;
      pDataUnitInfo[0].m_nKey = 0; // Segment key will be appended by rephandler
      pDataUnitInfo[0].m_nSize = 0; //Is it essential to fill the size field ?
                                    //Does qsm needs size info
      //TODO: Temp change need to make the appropriate change later. By default
      // marking that data unit start with rap. Need to query mpd parser to
      // find out if segment starts with rap. If it starts then fill it as 0
      // otherwise what should be filled here ??
      pDataUnitInfo[0].m_nRandomAccessPoint = 0;
      status = HTTPCommon::HTTPDL_SUCCESS;
    }
  }

  return status;
}

/** @brief Select the given data unit. Also return the corresponding playback start time.
  *
  * @param[in] nDataUnitKey - Data unit key to select
  * @param[out] nPbTime - Playback start time
  * @return
  * HTTPDL_SUCCESS - Selection successful
  * HTTPDL_WAITING - Selection in progress
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::Select
(
 uint64 nDataUnitKey,
 uint64& nPbTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  if (m_pSegmentHandler->m_pSidxParser)
  {
    uint32 nNumTotalDataUnits = (uint32)m_pSegmentHandler->m_pSidxParser->get_data_chunk_count();
    if ((uint32)nDataUnitKey < nNumTotalDataUnits)
    {
      //Get info for the given data unit
      data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
      if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)nDataUnitKey, &sFragInfo))
      {
        status = HTTPCommon::HTTPDL_SUCCESS;
        if(nPbTime == 0)
        {
          //ToDo: Instead of start time this should be position of RAP. Needs to be updated
          //when QSM has support for this. For now always switch at fragment boundaries.
          nPbTime = sFragInfo.n_start_time;
          /*
          nPbTime =
             sFragInfo.n_start_time + (sFragInfo.b_contains_rap ? sFragInfo.n_rapdelta_time : 0);
          */
        }
      }
    }
    else
    {
      QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Seg [0x%08x%08x]: Invalid data unit key %u/%u",
                     (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey() >> 32),
                     (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey()),
                     (uint32)nDataUnitKey, nNumTotalDataUnits );
    }
  }
  else
  {
    status = HTTPCommon::HTTPDL_SUCCESS;
    if(nPbTime == 0)
    {
      //TODO: Change this.For now switch at segment boundary if sidx not available
      nPbTime = m_pSegmentHandler->m_nStartTime;
    }
  }
  if(HTTPSUCCEEDED(status))
  {
     QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Seg [0x%08x%08x]: Select key/start %u/%u",
                       (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey() >> 32),
                       (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey()),
                       (uint32)nDataUnitKey, (uint32)nPbTime );
  }
  return status;
}


void DASHMediaSegmentHandler::SegmentBaseStateHandler::ClearBufferedData(uint64 nStartTime)
{
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  bool bResult = false;
  uint32 nFirstRemovedSegKey = MAX_UINT32_VAL, nLastRemovedSegKey = MAX_UINT32_VAL;
  uint64 nFirstRemovedDataUnitKey = MAX_UINT64_VAL, nLastRemovedDataUnitKey = MAX_UINT64_VAL;

  HTTPDataManager *pDataManager = m_pSegmentHandler->GetDataManager();
  if (pDataManager)
  {
    HTTPSegmentsInfo availiableSegmentsInfo;
    HTTPSegmentInfo segmentArray[HTTP_MAX_NUMBER_OF_SEGMENTS];

    availiableSegmentsInfo.maxSegments = HTTP_MAX_NUMBER_OF_SEGMENTS;
    availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray = segmentArray;
    HTTPDownloadStatus status = pDataManager->GetAvailableSegments(availiableSegmentsInfo, 0 );

    if (status == HTTPCommon::HTTPDL_SUCCESS)
    {
      bResult = true;

      //Get the max over all data units (ideally if download is complete on all of them
      //this is the same as querying only the last data unit)
      for (int i = 0; i < availiableSegmentsInfo.m_NumSegmentsInUse; i++)
      {
        uint64 starttime = 0;
        uint64 duration = 0;
        bool bAbort = false;
        //Ignore the init segment(s)
        uint64 nCurrKey = (int)availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_Key;
        if (nCurrKey != HTTP_INIT_SEG_KEY && nCurrKey != HTTP_NONSELF_INIT_SEG_KEY)
        {
          if (m_pSegmentHandler->m_pSidxParser)
          {
            data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
            if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)nCurrKey, &sFragInfo))
            {
              if(sFragInfo.n_start_time >= (uint64)nStartTime)
              {
                 starttime = sFragInfo.n_start_time;
                 duration = sFragInfo.n_subsegment_duration;

                if(!availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_bIsFullyDownloaded)
                {
                  //Process internal cancel for the dataunit
                  m_pSegmentHandler->ProcessCancelSegmentData(nCurrKey, false);
                }
                else
                {
                  pDataManager->AbortSegment(nCurrKey);
                  QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                 "Seg [0x%08x%08x]: Aborting segment %llu startTime %u "
                                 "duration %u flushTime %d",
                                 (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, nCurrKey,
                                 (uint32)starttime, (uint32)duration, (int)nStartTime);
                }

                nLastRemovedSegKey = nSegMPDKey & MPD_SEGMENT_MASK;
                nLastRemovedDataUnitKey = nCurrKey;

                if (MAX_UINT32_VAL == nFirstRemovedSegKey)
                {
                  nFirstRemovedSegKey = nLastRemovedSegKey;
                  nFirstRemovedDataUnitKey = nLastRemovedDataUnitKey;
                }
              }
            }
          }
        }
      }// for()

      if (MAX_UINT64_VAL != nFirstRemovedDataUnitKey)
      {
        m_pSegmentHandler->m_pRepNotifier->NotifyDataUnitRemoved(
           (uint32)m_pSegmentHandler->GetKey(), nFirstRemovedDataUnitKey, nLastRemovedDataUnitKey);
      }
    }
    else
    {
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Seg [0x%08x%08x]: Could not get data segment info %d",
                     (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, status );
    }
  }



}



HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::Flush
(
  HTTPCommon::HTTPMediaType majorType,
  int64 nStartTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  if(nStartTime == -1 ||
     (m_pSegmentHandler->m_nStartTime + m_pSegmentHandler->m_nDuration <= (uint64)nStartTime))
  {
    status = HTTPCommon::HTTPDL_DATA_END;
  }
  else
  {
    uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
    bool bResult = false;

    HTTPDataManager *pDataManager = m_pSegmentHandler->GetDataManager();
    if (pDataManager)
    {
      HTTPSegmentsInfo availiableSegmentsInfo;
      HTTPSegmentInfo segmentArray[HTTP_MAX_NUMBER_OF_SEGMENTS];

      availiableSegmentsInfo.maxSegments = HTTP_MAX_NUMBER_OF_SEGMENTS;
      availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray = segmentArray;
      status = pDataManager->GetAvailableSegments(availiableSegmentsInfo, 0 );
      if (status == HTTPCommon::HTTPDL_SUCCESS)
      {
        bResult = true;

        //Get the max over all data units (ideally if download is complete on all of them
        //this is the same as querying only the last data unit)
        for (int i = 0; i < availiableSegmentsInfo.m_NumSegmentsInUse; i++)
        {
          uint64 starttime = 0;
          uint64 duration = 0;
          bool bAbort = false;
          //Ignore the init segment(s)
          uint64 nCurrKey = (int)availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_Key;
          if (nCurrKey != HTTP_INIT_SEG_KEY && nCurrKey != HTTP_NONSELF_INIT_SEG_KEY)
          {
            if (m_pSegmentHandler->m_pSidxParser)
            {
              data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
              if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)nCurrKey, &sFragInfo))
              {
                if(sFragInfo.n_start_time + sFragInfo.n_subsegment_duration <= (uint64)nStartTime)
                {
                   starttime = sFragInfo.n_start_time;
                   duration = sFragInfo.n_subsegment_duration;
                   bAbort = true;
                   if(m_pSegmentHandler->m_pSidxParser->get_data_chunk_count() - 1 == nCurrKey)
                   {
                     status = HTTPCommon::HTTPDL_DATA_END;
                   }
                }
                else
                {
                  break;
                }
              }
            }
          }
          if(bAbort)
          {
            if(!availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_bIsFullyDownloaded)
            {
              //Process internal cancel for the dataunit
               m_pSegmentHandler->ProcessCancelSegmentData(nCurrKey, false);
            }
            else
            {
              pDataManager->AbortSegment(nCurrKey);
              QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "Seg [0x%08x%08x]: Aborting segment %llu startTime %u "
                            "duration %u flushTime %d",
                             (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, nCurrKey,
                             (uint32)starttime, (uint32)duration, (int)nStartTime);
            }
          } //if(bAbort)
        }// for()
      }
      else
      {
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Seg [0x%08x%08x]: Could not get data segment info %d",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, status );
      }
    }
  }
  if(status == HTTPCommon::HTTPDL_DATA_END)
  {
    m_pSegmentHandler->SetEndOfStream(majorType);
  }
  return status;
}

/** @brief Stores the seek time and calls open on segmenthandler. Open
  * command is processed by segment handler depending on the state.
  * @return
  * HTTPDL_SUCCESS - Seek is complete
  * HTTPDL_WAITING - Seek time is stored and seek will be done on segment handler
  *                  in open state
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::Seek
(
   const int64 nSeekTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  m_pSegmentHandler->m_nSeekTime = nSeekTime;
  if (m_pSegmentHandler->m_pSidxParser)
  {
    //Make sure the request is for a valid range
    uint32 nNumTotalDataUnits = (uint32)m_pSegmentHandler->m_pSidxParser->get_data_chunk_count();
    //If sidx is present, find the data unit which contains the seek time.
    //Call a open on segment handler for that data unit.
    //Open(dataunitkey,seektime) will remove all the dataunits before given dataunit key
    //and processes seek with given seektime.
    uint32 nDataUnitIndex=0;
    for (nDataUnitIndex = 0; nDataUnitIndex < nNumTotalDataUnits; nDataUnitIndex++)
    {
      data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
      if (m_pSegmentHandler->m_pSidxParser && m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)nDataUnitIndex, &sFragInfo))
      {
        if (sFragInfo.n_start_time <= (uint64)nSeekTime && (sFragInfo.n_start_time + sFragInfo.n_subsegment_duration)>= (uint64)nSeekTime)
        {
          break;
        }
      }
    }
    status = m_pSegmentHandler->Open(nDataUnitIndex,nSeekTime,true);
  }
  else
  {
    //If sidx is not present store the seek time and call a open
    status = OpenSegment();
  }
  return status;
}

/** @brief Queue up SEGMENT_CMD_OPEN  cmd from representation (valid in IDLE/SETUP state).
  *
  * @return
  * HTTPDL_WAITING - SEGMENT_CMD_OPEN cmd successfully queued and status notified later
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::OpenSegment
(
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  SegmentCmdData cmd;

  //check if already queued.
  void* pIt = NULL;
  while(m_pSegmentHandler->m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_OPEN)
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "Seg [0x%08x%08x]: OPEN cmd already queued",
                     (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey() >> 32),
                     (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey()) );
      status = HTTPCommon::HTTPDL_WAITING;
      return status;
    }
  }

  cmd.sBaseCmdData.eCmd = SEGMENT_CMD_OPEN;
  cmd.sBaseCmdData.pDownloader = NULL;

  if (m_pSegmentHandler->EnQCmd(cmd))
  {
    status = HTTPCommon::HTTPDL_WAITING;
    QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "Seg [0x%08x%08x]: OPEN cmd queued successfully cnt %u",
                   (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey() >> 32),
                   (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey()),
                   m_pSegmentHandler->m_cCmdQ.Count() );
  }
  return status;
}

/** @brief Get the segment range.
  *
  * @param[out] nStartTime - Segment start time in msec
  * @param[out] nDuration - Segment duration in msec
  * @return
  * TRUE - Range obtained successfully
  * FALSE - Otherwise
  */
bool DASHMediaSegmentHandler::SegmentBaseStateHandler::GetSegmentRange(uint64& nStartTime, uint64& nDuration)
{
  bool bOk = false;

  if (m_pSegmentHandler->m_pSidxParser)
  {
    //Get start time from the first data unit
    data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
    if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info(0, &sFragInfo))
    {
      nStartTime = sFragInfo.n_start_time;

      //Get duration from the last data unit range (instead of looping over all data units)
      uint32 nNumTotalDataUnits = (uint32)m_pSegmentHandler->m_pSidxParser->get_data_chunk_count();
      if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)(nNumTotalDataUnits - 1), &sFragInfo))
      {
        nDuration = sFragInfo.n_start_time + sFragInfo.n_subsegment_duration - nStartTime;
        bOk = true;
      }
    }
  }
  else
  {
    nStartTime = m_pSegmentHandler->m_nStartTime;
    nDuration = m_pSegmentHandler->m_nDuration;
    bOk = true;
  }

  return bOk;
};

/** @brief Close segment (valid in ALL states).
  *
  * @return status of operation
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::Close()
{
  //Clear cmd Q
  m_pSegmentHandler->ClearCmdQ();
  m_pSegmentHandler->m_bProcessOpenCmd = false;
  //Reset segment states
  m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cIdleStateHandler);
  m_pSegmentHandler->ResetDataDownloadState();

  m_pSegmentHandler->m_bIsReadsStarted = false;
  m_pSegmentHandler->m_bIsResourceReadDisabled = false;
  m_pSegmentHandler->m_bIsLmsgSet = false;
  m_pSegmentHandler->MarkSegmentComplete();

 //Abort the INIT segment as it is owned by rep and won't be flushed otherwise
  HTTPDataManager* pDataManager = m_pSegmentHandler->GetDataManager();
  if (pDataManager)
  {
    (void)pDataManager->AbortSegment(HTTP_INIT_SEG_KEY);
  }

  return HTTPCommon::HTTPDL_SUCCESS;
}

/** @brief Process all commands (in IDLE state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaSegmentHandler::SegmentIdleStateHandler::ProcessCmds()
{
  int ret = 0;
  SegmentCmdData cmd;
  void* pIt = NULL;

  while(m_pSegmentHandler->m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGINFO)
    {
      uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
      SegmentDownloader* pDownloader = cmd.sGetSegInfoCmdData.pDownloader;
      HTTPDataManager* pDataManager = m_pSegmentHandler->GetSidxDataManager();
      HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
      if (pDownloader && pDataManager)
      {
        //Delete existing sidx parser on the resource
        if (m_pSegmentHandler->m_pSidxParser)
        {
          QTV_Delete(m_pSegmentHandler->m_pSidxParser);
          m_pSegmentHandler->m_pSidxParser = NULL;
        }

        if ((m_pSegmentHandler->m_rSidxFetchDecision).IsSidxFetchDisabled())
        {
          m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cSetupStateHandler);
          status = HTTPCommon::HTTPDL_SUCCESS;
        }
        else
        {
          //Create data manager non-purgable segment
          int64 nSegStartOffset = (m_pSegmentHandler->m_pInitSegment ?
                                   m_pSegmentHandler->m_pInitSegment->GetNumBytesDownloaded() : 0);
          status = pDataManager->CreateSegment(HTTP_INIT_SEG_KEY, iHTTPBase::SEGMENT_DEFAULT,
                                                 nSegStartOffset, false);
          if (status == HTTPCommon::HTTPDL_SUCCESS)
          {
            m_pSegmentHandler->m_pSidxParser = QTV_New_Args(::sidxparser, ((iStreamPort*)pDataManager));

            if (m_pSegmentHandler->m_pSidxParser &&
                m_pSegmentHandler->SendSegmentInfoRequest(pDownloader))
            {
              status = HTTPCommon::HTTPDL_WAITING;
              m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cParseSidxStateHandler);
            }
            else
            {
              //Abort data manager segment
              status = HTTPCommon::HTTPDL_ERROR_ABORT;
              (void)pDataManager->AbortSegment(HTTP_INIT_SEG_KEY);
            }
          }
          else
          {
            QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Seg [0x%08x%08x]: Data manager CreateSegment() failed %d for init segment",
                           (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, status );
            status = HTTPCommon::HTTPDL_ERROR_ABORT;
          }
        }
      }

      //Process any errors, else the right state will handle it later
      if (status == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        m_pSegmentHandler->OnError(status);
      }
      //Nothing else to process in IDLE
      break;
    }
  }

  return ret;
}

/** @brief Queue up GET_SEGINFO cmd from representation (valid in IDLE state).
  *
  * @param[in] nStartTime - Request start time
  * @param[in] nDuration - Request duration
  * @return
  * HTTPDL_WAITING - GET_SEGINFO cmd successfully queued and status notified later
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentIdleStateHandler::GetSegmentInfo
(
 uint64 nStartTime,
 uint64 nDuration
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  SegmentDownloader* pDownloader =
    m_pSegmentHandler->m_cSegDownloadMgr.GetNextAvailableSegmentDownloader();
  if (pDownloader)
  {
    SegmentCmdData cmd;
    cmd.sGetSegInfoCmdData.eCmd = SEGMENT_CMD_GET_SEGINFO;
    cmd.sGetSegInfoCmdData.pDownloader = pDownloader;
    cmd.sGetSegInfoCmdData.nStartTime = nStartTime;
    cmd.sGetSegInfoCmdData.nDuration = nDuration;
    if (m_pSegmentHandler->EnQCmd(cmd))
    {
      status = HTTPCommon::HTTPDL_WAITING;
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "Seg [0x%08x%08x]: GET_SEGINFO cmd queued successfully cnt %u",
                     (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey() >> 32),
                     (uint32)(m_pSegmentHandler->m_cSegmentInfo.getKey()),
                     m_pSegmentHandler->m_cCmdQ.Count() );
    }
  }

  return status;
}

/** @brief Parses sidx atom using sidx parser
*
* @return
* HTTPDL_SUCCES - Parsing is successfully complete
* HTTPDL_WAITING - enough data is not available
* HTTPDL_ERROR_ABORT - Generic Failure
*/
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentParseSidxStateHandler::Parse_Sidx()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  if (m_pSegmentHandler->m_pSidxParser)
  {
    switch(m_pSegmentHandler->m_pSidxParser->parse_sidx())
    {
      case SIDX_PARSER_INTERNAL_FATAL_ERROR:
      case SIDX_PARSER_READ_FAILED:
      case SIDX_PARSER_OUT_OF_MEMORY:
        m_SidxParseStatus = SIDX_PARSE_FAIL;
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Seg [0x%08x%08x]: Sidx parsing error. Continue with download entire segment",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
        status = HTTPCommon::HTTPDL_SUCCESS;
        break;
      case SIDX_PARSER_SIDX_PARSING_DONE:
        //ToDo: For daisy-chained sidxes this could mean we also downloaded a few
        //data fragments. Might need sidx hole management, for now assuming all sidxes
        //are present before the first moof (simple profile)!
        m_SidxParseStatus = SIDX_PARSE_DONE;
        status = HTTPCommon::HTTPDL_SUCCESS;
        break;
      case SIDX_PARSER_SIDX_NOT_AVAILABLE:
        //There is no sidx atom present in the segment
        m_SidxParseStatus = SIDX_PARSE_NOT_AVAIL;
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Seg [0x%08x%08x]: No sidx atom present in segment. Download entire segment",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
        status = HTTPCommon::HTTPDL_SUCCESS;
        break;
      case SIDX_PARSER_SIDX_PARSING_PENDING:
        m_SidxParseStatus = SIDX_PARSE_UNDER_RUN;
      default:
       status = HTTPCommon::HTTPDL_WAITING;
        break;
      }
  }
  else
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  return status;
}

/** @brief Process all commands (in ParseSidx state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaSegmentHandler::SegmentParseSidxStateHandler::ProcessCmds()
{
  int ret = 0;
  SegmentCmdData *cmd;
  void* pIt = NULL;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();

  while(m_pSegmentHandler->m_cCmdQ.Next(pIt, &cmd))
  {
    if (cmd->sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGINFO)
    {
      SegmentDownloader* pDownloader = cmd->sGetSegInfoCmdData.pDownloader;
      HTTPDataManager* pDataManager = m_pSegmentHandler->GetSidxDataManager();

      //Download data and invoke sidx parsing (if not already done)
      HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
      HTTPDownloadStatus downloadStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
      if(pDownloader && pDataManager)
      {
        downloadStatus = m_pSegmentHandler->Download(pDownloader, pDataManager);
      }
      if (m_SidxParseStatus == SIDX_PARSE_DONE)
      {
        status = HTTPCommon::HTTPDL_SUCCESS;
      }
      else if (downloadStatus == HTTPCommon::HTTPDL_WAITING ||
               downloadStatus == HTTPCommon::HTTPDL_SUCCESS ||
               downloadStatus == HTTPCommon::HTTPDL_DATA_END)
      {
        // Set end of file on data manager for seperate index URL
        if(downloadStatus == HTTPCommon::HTTPDL_DATA_END &&
           m_pSegmentHandler->m_cSegmentInfo.IsIndexURLPresent())
        {
          (void)pDataManager->SetSegmentComplete(HTTP_INIT_SEG_KEY, pDownloader->GetNumBytesReceived() - 1);
          pDataManager->SetEndofFile();
        }

        status = Parse_Sidx();
        if (downloadStatus != HTTPCommon::HTTPDL_WAITING)
        {
          m_pSegmentHandler->m_numRetriesExcerised = 0;
        }
      }
      else if(HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND == downloadStatus &&
              m_pSegmentHandler->m_sDASHSessionInfo.cMPDParser.IsLive() &&
              (pDownloader && pDownloader->GetNumBytesReceived() == 0))
      {
        /* Start Retying for the SIDX if there is a download error(covers all HTTP error Response codes)
           even before downloading a single byte and session is live and the segment doesnt fall under TSB window */
        MM_Time_DateTime currentTime;
        MM_Time_GetUTCTime(&currentTime);
        double CurrMsec = StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);
        double SegAvailabilityTimeMsec = m_pSegmentHandler->m_cSegmentInfo.getAvailabilitytime();
        uint32 SegDuration = (uint32)m_pSegmentHandler->m_cSegmentInfo.getDuration();
        if( (SegAvailabilityTimeMsec + (double)SegDuration) > CurrMsec )
        {
          cmd->sGetSegInfoCmdData.nLastFailureTime = m_pSegmentHandler->m_pClock->GetTickCount();
          m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cSegInfoRetryStateHandler);
          status = HTTPCommon::HTTPDL_WAITING;
        }
        else
        {
          QTV_MSG_PRIO5(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "Seg [0x%08x%08x]:not Retrying, as SegmentAbilityTime(%u + dutation(%u)) is before the currentTime(%u)",
                             (uint32)(nSegMPDKey >> 32),(uint32)nSegMPDKey,(uint32)SegAvailabilityTimeMsec,
                             (uint32)SegDuration,(uint32)CurrMsec);
          status = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
        }
      }
      else if (HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND == downloadStatus)
      {
        status = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
      }// if (m_bIsSidxParsingDone)
      //If sidx parsing is complete, mark data manager segment complete(if sidx is present)
      //else abort the segment

      if (status != HTTPCommon::HTTPDL_WAITING)
      {
        if(HTTPSUCCEEDED(status))
        {
          // mark segment complete(for sidx present)
          //or abort the segment(for sidx not present)
          if(m_SidxParseStatus == SIDX_PARSE_DONE)
          {
            if (false == m_pSegmentHandler->m_bIsLmsgSet)
            {
              if (m_pSegmentHandler->m_pSidxParser->is_lmsg_present())
              {
                if (m_pSegmentHandler->m_sDASHSessionInfo.cMPDParser.IsLive())
                {
                  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "lmsg set. Refresh playlist");
                  (m_pSegmentHandler->m_sDASHSessionInfo).cMPDParser.ForceRefreshPlaylist();
                }
                m_pSegmentHandler->m_bIsLmsgSet = true;
              }
            }
            data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
            int64 nAvailableOffset = 0;
            bool bEOS;
            if(m_pSegmentHandler->m_pSidxParser)
            {
              (void)m_pSegmentHandler->m_pSidxParser->get_data_chunk_info(0, &sFragInfo);
            }

            if(pDataManager)
            {
              (void)pDataManager->GetAvailableOffset(&nAvailableOffset, &bEOS);
            }

            if (nAvailableOffset <= (int64)sFragInfo.n_offset)
            {
              if (downloadStatus == HTTPCommon::HTTPDL_DATA_END)
              {
                status = HTTPCommon::HTTPDL_ERROR_ABORT;
                (void)pDownloader->Stop();
                QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                               "Downloading extra bytes between Sidx %lld and First Moof %llu",
                                nAvailableOffset,sFragInfo.n_offset );

                 char* reqURL = m_pSegmentHandler->m_cSegmentInfo.GetIndexURL();
                //Based on if server honors byte range header as part of original URL or
                //uses a byterangeURL since it dishonors byte range header (for http1.0),
                //construct segmentUrl and alternatesegmentUrl i.e. byterangeURL
                if (m_pSegmentHandler->StartDownload(pDownloader,
                                                      HTTP_INIT_SEG_KEY,
                                                      nAvailableOffset, sFragInfo.n_offset, reqURL))
                {
                  status = HTTPCommon::HTTPDL_WAITING ;
                }
              }
              else
              {
                //Wait until start of data fragment in case any optional atoms such as uuid
                //are present between sidx and first moof!
                status = HTTPCommon::HTTPDL_WAITING;
              }
            }
            else
            {
              if(downloadStatus != HTTPCommon::HTTPDL_DATA_END)
              {
                // Request amount of bytes have not been read completely , so drain the data from the stack
                status = DiscardExtraDataAfterSidx(pDownloader);
              }

              if(HTTPSUCCEEDED(status))
              {
                m_SidxParseStatus = SIDX_PARSE_COMPLETE;
                //Note that sFragInfo.n_offset is already relative to start of INIT segment. Also
                //OPEN file parser after data download begins so that parser can possibly complete
                //OPEN processing in one attempt (subject to availability of enough data), else
                //might incur an additional delay (equal to parser's OPEN loop back off duration =
                //100 msec)
                if(m_pSegmentHandler->m_cSegmentInfo.IsIndexURLPresent() && pDataManager)
                {
                  (void)pDataManager->AbortSegment(HTTP_INIT_SEG_KEY);

                  // In seperate sidx (index url) case, Assuming media URL will always start with zero ofset,
                  // as parser provides media offsets from end of the sidx. so adjust offsets for all media
                  // data unit requests
                  data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
                  if (m_pSegmentHandler->m_pSidxParser &&
                      m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)0, &sFragInfo))
                  {
                    m_pSegmentHandler->m_offsetAdjust = sFragInfo.n_offset;
                    m_pSegmentHandler->m_offsetAdjust -= (m_pSegmentHandler->m_pInitSegment ?
                                                          m_pSegmentHandler->m_pInitSegment->GetNumBytesDownloaded() : 0);
                  }
                }
                else if(pDataManager)
                {
                  (void)pDataManager->SetSegmentComplete(HTTP_INIT_SEG_KEY, sFragInfo.n_offset);
                }
                QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Seg [0x%08x%08x]: Init segment downloaded and sidx parsing complete",
                  (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
              }
            }
          }
          else if(m_SidxParseStatus == SIDX_PARSE_NOT_AVAIL || m_SidxParseStatus == SIDX_PARSE_FAIL)
          {
            //Abort the init segment if sidx is not present as now complete segment
            //will be treated as a single data unit and whole segemnt will be downloaded
            //when qsm requests for data unit download. Also delete the sidx parser instance
            if (m_pSegmentHandler->m_pSidxParser)
            {
              QTV_Delete(m_pSegmentHandler->m_pSidxParser);
              m_pSegmentHandler->m_pSidxParser = NULL;
            }
            if (pDataManager)
            {
              (void)pDataManager->AbortSegment(HTTP_INIT_SEG_KEY);
            }

            if(m_SidxParseStatus == SIDX_PARSE_NOT_AVAIL)
            {
            m_pSegmentHandler->m_rSidxFetchDecision.DisableSidxFetch();
            }
          }// if(m_SidxParseStatus == SIDX_PARSE_NOT_AVAIL || m_SidxParseStatus == SIDX_PARSE_FAIL)
        }// if(HTTPSUCCEEDED(status))
      }// if (status != HTTPCommon::HTTPDL_WAITING)
      else if (downloadStatus == HTTPCommon::HTTPDL_DATA_END)
      {
        //Need more data for SIDX parsing to proceed
        if (m_SidxParseStatus == SIDX_PARSE_UNDER_RUN)
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "Sidx Data incomplete, Downloading....");

          status = HTTPCommon::HTTPDL_ERROR_ABORT;
          int64 nStartOffset = (pDownloader->GetStartOffset() + pDownloader->GetNumBytesReceived());
          int64 nEndOffset = nStartOffset + DEFAULT_SEG_INFO_GET_REQUEST_LENGTH - 1;
          int64 nDownloadOffset = (m_pSegmentHandler->m_pSidxParser)? m_pSegmentHandler->m_pSidxParser->get_download_offset() : 0;
          nEndOffset = (nDownloadOffset > 0) ? nDownloadOffset: nEndOffset;

          (void)pDownloader->Stop();

          //Based on if server honors byte range header as part of original URL or
          //uses a byterangeURL since it dishonors byte range header (for http1.0),
          //construct segmentUrl and alternatesegmentUrl i.e. byterangeURL
          char* reqURL = m_pSegmentHandler->m_cSegmentInfo.GetIndexURL();
          if (m_pSegmentHandler->m_pSidxParser &&
              m_pSegmentHandler->StartDownload(pDownloader,
                                               HTTP_INIT_SEG_KEY,
                                               nStartOffset, nEndOffset, reqURL))
          {
            status = HTTPCommon::HTTPDL_WAITING;
          }
        }// if (m_SidxParseStatus == SIDX_PARSE_UNDER_RUN)
      }// if (downloadStatus == HTTPCommon::HTTPDL_DATA_END)

      if (status != HTTPCommon::HTTPDL_WAITING)
      {
        //Stop the downloader
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Seg [0x%08x%08x]: Exiting ParseSidx %d, stopping downloader",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, status );
        m_pSegmentHandler->DownloadEnded(pDownloader);

        if (HTTPSUCCEEDED(status))
        {
          m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cSetupStateHandler);
          (void)m_pSegmentHandler->GetSegmentRange(m_pSegmentHandler->m_nStartTime,
                                                   m_pSegmentHandler->m_nDuration);

          //Notify response, dequeue cmd and wait to process OPEN
          if (m_pSegmentHandler->m_pRepNotifier)
          {
            uint32 nNumDataUnits = 0;
            (void)FillDataUnitInfo(cmd->sGetSegInfoCmdData.nStartTime,
                                   cmd->sGetSegInfoCmdData.nDuration,
                                   NULL, 0, nNumDataUnits);
            status = (nNumDataUnits > 0) ? HTTPCommon::HTTPDL_SUCCESS : HTTPCommon::HTTPDL_ERROR_ABORT;
            QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "Seg [0x%08x%08x]: GET_SEGINFO cmd (nStartTime %u nDuration %u nNumDataUnits %u) "
                           "processing complete %d - notifying representation",
                           (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                           (uint32)cmd->sGetSegInfoCmdData.nStartTime,
                           (uint32)cmd->sGetSegInfoCmdData.nDuration, nNumDataUnits, status );
            m_pSegmentHandler->m_pRepNotifier->SegInfoReady((uint32)m_pSegmentHandler->GetKey(),
                                                            nNumDataUnits, status);
          }
          (void)m_pSegmentHandler->m_cCmdQ.Remove(pIt);
        }
        else
        {
          //Process any errors
          if (pDataManager)
          {
            (void)pDataManager->AbortSegment(HTTP_INIT_SEG_KEY);
          }

          m_pSegmentHandler->OnError(status, cmd->sBaseCmdData.nCmdId);

          //Nothing else to do in SETUP while parsing sidx/error
          break;
        }
      }// if (status != HTTPCommon::HTTPDL_WAITING)
    }// if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGINFO)
  }// while(m_pSegmentHandler->m_cCmdQ.Next(pIt, cmd))

  return ret;
}

bool DASHMediaSegmentHandler::StartDownload(SegmentDownloader* pDownloader,
                                            const uint64 nKey,
                                            const int64 nStartOffset,
                                            const int64 nEndOffset,
                                            char  *reqURL,
                                            const int nDurationMs)
{
    RepresentationInfo *repInfo = NULL;
    m_sDASHSessionInfo.cMPDParser.GetRepresentationByKey(m_cSegmentInfo.getKey(), repInfo);
    char* pSegmentUrl = NULL;
    char* pAlternateSegmentUrl = NULL;
    bool bOk = false;

    pSegmentUrl = reqURL;

    //If byterangeURL request/response is success for a given segment, other segmenthandlers
    //for the same representation send only byterangeURL to the segment downloader
    //For the first segments, when uncertain if http1.1/http1.0, send both
    //original URL and byterangeURL to segment downloader
    if((m_cSegmentInfo.GetByteRangeURLResp() == HTTPCommon::BYTERANGE_URL_USED_TRUE
      || m_cSegmentInfo.GetByteRangeURLResp() == HTTPCommon::BYTERANGE_URL_USED_UNDETERMINED)
      && repInfo->GetByteRangeURLTemplate() && nEndOffset != -1)
    {
      bOk = ConstructByteRangeURL(reqURL,
                                  repInfo->GetByteRangeURLTemplate(),
                                  nStartOffset, nEndOffset);

      if(bOk)
      {
        if(m_cSegmentInfo.GetByteRangeURLResp() == HTTPCommon::BYTERANGE_URL_USED_TRUE)
        {
          pSegmentUrl = NULL;
        }
        pAlternateSegmentUrl = m_cSegmentInfo.GetByteRangeURL();
      }
    }

    bOk = pDownloader->Start(pSegmentUrl,
                             pAlternateSegmentUrl,
                             nKey,
                             nStartOffset,
                             nEndOffset,
                             nDurationMs);

    return bOk;

}

/*
 * Construct byterangeURL from the original URL and byterange template
 * string provided in the MPD. ByteRangeURL will be used for http1.0
 * servers which donot support partial GET requests and hence will
 * dishonor byte range headers fields as part of the original URL GET
 * request.
 */
bool DASHMediaSegmentHandler::ConstructByteRangeURL(char* origUrl, char* templateStr, int64 nStartOffset, int64 nEndOffset)
{
  bool bOk=true;

  if(origUrl && templateStr)
  {
    size_t origUrlLength = std_strlen(origUrl);
    size_t templateStrLength = std_strlen(templateStr);
    char* origUrlPtr = origUrl; //ptr to Original Url
    char* byteRangeUrl = NULL;
    int index=0;

    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "ByteRangeTemplate is %s", templateStr);
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "ByteRangeTemplate length is %d", templateStrLength);

    byteRangeUrl = (char *)QTV_Malloc((origUrlLength + templateStrLength + 1) * sizeof(char));

    if(byteRangeUrl)
    {
      char* byteRangeUrlptr = byteRangeUrl; //ptr to contructed Url
      char *basePtr = NULL;
      char *queryPtr = NULL;
      char *frgmtPtr = NULL;
      ptrdiff_t baseLen = 0;
      ptrdiff_t queryLen = 0;

      //Given url can have base-part/query-part/remaining fragment part
      basePtr = origUrlPtr;
      queryPtr = std_strchr(origUrlPtr, '?');
      frgmtPtr = std_strchr(origUrlPtr, '#');

      //Calculate base url portion length
      if(queryPtr)
      {
        baseLen = queryPtr - basePtr;
      }
      else if(frgmtPtr)
      {
        baseLen = frgmtPtr - basePtr;
      }
      else
      {
        baseLen = (ptrdiff_t)origUrlLength;
      }

      //Calculate query portion length
      if(queryPtr)
      {
        queryPtr++;
        queryLen = frgmtPtr ? (frgmtPtr - queryPtr) : (origUrlLength - baseLen - 1);
      }
      else
      {
        queryLen = 0;
      }

      while(*templateStr)
      {
        //It is an identifier. Substitute it with correspondent Url component
        if(*templateStr == '$')
        {
          char* ptr = templateStr++;

          while(*templateStr++ != '$');

          if(!std_strnicmp(ptr, "$$", 2))
          {
            *byteRangeUrlptr++ = '$';
          }
          else if(!std_strnicmp(ptr, "$base$", 6)) //If identifier is $base$
          {
            if(basePtr)
            {
              std_strlcpy(byteRangeUrlptr, basePtr, baseLen+1);
              byteRangeUrlptr+=baseLen;
              origUrlPtr+=baseLen;
            }
          }
          else if(!std_strnicmp(ptr, "$query$", 7)) //If identifier is $query$
          {
            if(queryPtr)
            {
              std_strlcpy(byteRangeUrlptr, queryPtr, queryLen+1);
              byteRangeUrlptr+=queryLen;
              origUrlPtr+=(queryLen)+1;
            }
            else
            {
              //If idetifier is query and there is no query portion in the original Url, Remove the separator before the "$query$" if it is not ?
              //Else remove the separator after the "$query$" if present
              if(*(byteRangeUrlptr-1) != '?')
              {
                byteRangeUrlptr--;
              }
              else if(*templateStr)
              {
                templateStr++;
              }
            }
          }
          else if(!std_strnicmp(ptr, "$first$", 7)) //If identifier is $first$
          {
            char firstByte[HTTP_MAX_RANGE_LEN] = {0};
            int len = std_strlprintf(firstByte, sizeof(firstByte), "%llu", nStartOffset);
            std_strlcpy(byteRangeUrlptr, firstByte, len+1);
            byteRangeUrlptr+=len;
          }
          else if(!std_strnicmp(ptr, "$last$", 6))  //If identifier is $last$
          {
            char lastByte[HTTP_MAX_RANGE_LEN] = {0};
            int len = std_strlprintf(lastByte, sizeof(lastByte), "%llu", nEndOffset);
            std_strlcpy(byteRangeUrlptr, lastByte, len+1);
            byteRangeUrlptr+=len;
          }
        }
        else
        {
          *byteRangeUrlptr++ = *templateStr++;
        }
      }

      if(origUrlPtr)
      {
        std_strlcpy(byteRangeUrlptr, origUrlPtr, std_strlen(origUrlPtr)+1);
        byteRangeUrlptr+=std_strlen(origUrlPtr);
      }

      m_cSegmentInfo.SetByteRangeURL(byteRangeUrl);

      if(byteRangeUrl)
      {
        QTV_Free(byteRangeUrl);
        byteRangeUrl = NULL;
      }
    }
    else
    {
      bOk = false;
    }
  }
  else
  {
    bOk = false;
  }

  return bOk;

}



/**
 * @brief Discard extra data received after moof atom is found
 *
 * @param pDownloader
 *
 * @return HTTPDownloadStatus : HTTPDL_SUCCESS on completion
 *  HTTPDL_WAIT on not discarding the complete data
 *  HTTPDL_ERROR_ABORT on failure
 */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentParseSidxStateHandler::DiscardExtraDataAfterSidx
(
  SegmentDownloader* pDownloader
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,"DiscardExtraDataAfterSidx");

  HTTPDownloadStatus status = video::HTTPCommon::HTTPDL_ERROR_ABORT;
  if ( pDownloader )
  {
    byte* pBuffer = NULL;
    uint64 nKey = pDownloader->GetKey();
    uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
    char Buffer[DEFAULT_SEG_INFO_GET_REQUEST_LENGTH];
    int64 TotalBytesRead = 0;
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_SUCCESS;

    while(result != HTTPCommon::HTTPDL_DATA_END &&
          result != HTTPCommon::HTTPDL_ERROR_ABORT)
    {
      int64 nBytesRead = 0;
      result = pDownloader->Read((byte*)Buffer, DEFAULT_SEG_INFO_GET_REQUEST_LENGTH, nBytesRead);
      if(result == HTTPCommon::HTTPDL_SUCCESS || result == HTTPCommon::HTTPDL_DATA_END)
      {
        TotalBytesRead += nBytesRead;
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED, "Discarded %lld bytes", TotalBytesRead);
      }

      if(result == HTTPCommon::HTTPDL_DATA_END)
      {
        QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Seg [0x%08x%08x] Discarded %d bytes after Sidx, (requested/downloaded) %lld/%llu bytes",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                       (int)TotalBytesRead, pDownloader->GetContentLength(), pDownloader->GetNumBytesReceived() );
        status = video::HTTPCommon::HTTPDL_SUCCESS;
      }
      else if(result == HTTPCommon::HTTPDL_WAITING)
      {
        status = video::HTTPCommon::HTTPDL_WAITING;
      }
    }
  }

  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,"DiscardDataAfterSidx status :%d",
                 status);

  return status;
}

/** @brief Process all commands (in ParseSidx state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaSegmentHandler::SegmentInfoRetryHandler::ProcessCmds()
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_WAITING;
  int ret = 0;
  SegmentCmdData cmd;
  void* pIt = NULL;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  HTTPDataManager* pDataManager = m_pSegmentHandler->GetSidxDataManager();

  while(m_pSegmentHandler->m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGINFO)
    {
      uint64 segKey = 0;
      uint64 lastAvailableSegmentEndTime, lastAvailableSegmentStartTime = 0;
      SegmentDownloader* pDownloader = cmd.sGetSegInfoCmdData.pDownloader;

      HTTPDownloadStatus mpdStatus =
        m_pSegmentHandler->m_sDASHSessionInfo.cMPDParser.GetLastAvailableSegmentTimeForRepresentation(
                                                                              m_pSegmentHandler->m_cSegmentInfo.getKey(),
                                                                              lastAvailableSegmentStartTime,
                                                                              lastAvailableSegmentEndTime);
      // Retry if Next Segment is unavailable or the current segment is the last segment
      if (((mpdStatus == HTTPCommon::HTTPDL_SUCCESS) &&
          (m_pSegmentHandler->m_cSegmentInfo.getStartTime()>= lastAvailableSegmentStartTime)) ||
           mpdStatus == HTTPCommon::HTTPDL_DATA_END)
      {
        MM_Time_DateTime currentTime;
        MM_Time_GetUTCTime(&currentTime);
        uint32 nElapsedTimeSinceLastFailure = HTTPCommon::GetElapsedTime(m_pSegmentHandler->m_pClock,
                                                                         cmd.sGetSegInfoCmdData.nLastFailureTime);
        double currMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);
        double SegAvailabilityTimeMsec = m_pSegmentHandler->m_cSegmentInfo.getAvailabilitytime();
        uint32 SegDuration = (uint32)m_pSegmentHandler->m_cSegmentInfo.getDuration();
        uint32 nMaxRetryDuration = (uint32)((double)SegDuration * SEGMENT_DURATION_FRACTION_FOR_RETRY_DURATION);
        uint32 SleepTime = (uint32)((double)SegDuration * SEGMENT_DURATION_FRACTION_FOR_RETRY_INTERVAL);


        if ( ((SegAvailabilityTimeMsec + (double)nMaxRetryDuration) > currMSeconds)  && nElapsedTimeSinceLastFailure >= SleepTime) //&& nElapsedTime <  nMaxRetryDuration)
        {
          (void)pDownloader->Stop();
          if(m_pSegmentHandler->SendSegmentInfoRequest(pDownloader))
          {
            ++m_pSegmentHandler->m_numRetriesExcerised;
            QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,"Seg [0x%08x%08x]:SegmentSegmentInfoRetryHandler: Retrying(%d)..currTime(%u)/MaxRetryWindow(%u)",
                           (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey , m_pSegmentHandler->m_numRetriesExcerised, (uint32)currMSeconds,
                           (uint32)(SegAvailabilityTimeMsec + (double)nMaxRetryDuration));
            m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cParseSidxStateHandler);
          }
        }
        else if((SegAvailabilityTimeMsec + (double)nMaxRetryDuration) <= currMSeconds)
        {
          QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Seg [0x%08x%08x]:SegmentSegmentInfoRetryHandler:Beyond retry window SegAvaibilityTime(%u), CurrMsec (%u),MaxRetryTime"
                          "(%u),Retries attempted(%d)",
                          (uint32)(nSegMPDKey >> 32),(uint32)nSegMPDKey,(uint32)SegAvailabilityTimeMsec,
                         (uint32)currMSeconds,(uint32)(SegAvailabilityTimeMsec + (double)nMaxRetryDuration), m_pSegmentHandler->m_numRetriesExcerised);
          eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
      }
      else if ( lastAvailableSegmentStartTime >= m_pSegmentHandler->m_cSegmentInfo.getStartTime())
      {
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,"Seg [0x%08x%08x]:SegmentSegmentInfoRetryHandler: Stopping Retries as Next Segment with time(%llu) is available",
                           (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,lastAvailableSegmentStartTime);
        eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
      }
      else if (mpdStatus != HTTPCommon::HTTPDL_SUCCESS || mpdStatus != HTTPCommon::HTTPDL_WAITING)
      {
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,"Seg [0x%08x%08x]:SegmentSegmentInfoRetryHandler:GetLastAvailableSegment Failed status:%d",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,mpdStatus);
        eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
      }

      if (eStatus != HTTPCommon::HTTPDL_WAITING)
      {
        m_pSegmentHandler->m_numRetriesExcerised = 0;
        if(pDataManager)
        {
          pDataManager->AbortSegment(HTTP_INIT_SEG_KEY);
        }
        m_pSegmentHandler->DownloadEnded(pDownloader);
        m_pSegmentHandler->OnError(HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND, cmd.sBaseCmdData.nCmdId);
      }
    }
  }
  return ret;
}

/** @brief Process all commands (in SETUP state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaSegmentHandler::SegmentSetupStateHandler::ProcessCmds()
{
  int ret = 0;
  SegmentCmdData *pCmd = NULL;
  void* pIt = NULL;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();

  while(m_pSegmentHandler->m_cCmdQ.Next(pIt, &pCmd))
  {
    SegmentCmdData& cmd = *pCmd;

    HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
    if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGINFO)
    {
     status = m_pSegmentHandler->ProcessGetSegmentInfoCmd(cmd.sGetSegInfoCmdData.nStartTime,
                                                           cmd.sGetSegInfoCmdData.nDuration);
    }
    if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_OPEN)
    {
      if(m_pSegmentHandler->m_bProcessOpenCmd)
      {
        HTTPDataManager* pDataManager = m_pSegmentHandler->GetDataManager();
        status = HTTPCommon::HTTPDL_SUCCESS;
        m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cOpeningStateHandler );
        FileSource* pFileSource = m_pSegmentHandler->GetFileSource();
        MPDProfileType mpdProfile = m_pSegmentHandler->m_sDASHSessionInfo.cMPDParser.GetMPDProfile();
        FileSourceFileFormat fileformat = FILE_SOURCE_UNKNOWN;

        fileformat = ((DASH_PROFILE_MP2T_MAIN == mpdProfile) || (DASH_PROFILE_MP2T_SIMPLE == mpdProfile)) ?
                     FILE_SOURCE_DASH_MP2TS : FILE_SOURCE_MP4_DASH;

        if (pFileSource == NULL ||
          FILE_SOURCE_FAILED(pFileSource->OpenFile((iStreamPort *)pDataManager, fileformat)))
        {
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }

        if (status != HTTPCommon::HTTPDL_WAITING)
        {
          //Process any errors and move to OPENING for success
          if (HTTPSUCCEEDED(status))
          {
            QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "Seg [0x%08x%08x]: Opening file source and moving to OPENING",
              (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
            (void)m_pSegmentHandler->m_cCmdQ.Remove(pIt);
          }
          else
          {
            m_pSegmentHandler->OnError(status);
          }

          //Nothing else to do in SETUP after processing OPEN
          break;
        }
      }
    }
    else if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA)
    {
      status = m_pSegmentHandler->ProcessGetSegmentDataCmd(cmd);
    }

    //Dequeue cmd if processed
    if (status != HTTPCommon::HTTPDL_WAITING)
    {
      (void)m_pSegmentHandler->m_cCmdQ.Remove(pIt);
    }
  }
  return ret;
}

/** @brief Process file source event (in OPENING state).
  *
  * @param[in] status - File source event/status
  */
void DASHMediaSegmentHandler::SegmentOpeningStateHandler::ProcessFileSourceEvent
(
 FileSourceCallBackStatus status
)
{
  bool bError = true;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();

  //Process file source OPEN_COMPLETE notification, set up tracks
  if (status == FILE_SOURCE_OPEN_COMPLETE)
  {
    if (m_pSegmentHandler->SetupTracks() == HTTPCommon::HTTPDL_SUCCESS)
    {
      bError = false;
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Seg [0x%08x%08x]: File source reported OPEN_COMPLETE and track setup successful, "
                     "moving to OPEN", (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
      m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cOpenStateHandler);
    }
  }
  //Notify any file source errors (note this is fatal!!)
  if (bError)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Seg [0x%08x%08x]: File source reported error, notify representation",
                   (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
    m_pSegmentHandler->OnError(HTTPCommon::HTTPDL_ERROR_ABORT);

  }
}

HTTPDownloadStatus DASHMediaSegmentHandler::SegmentOpenStateHandler::Seek(const int64 nSeekTime)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  m_pSegmentHandler->m_nSeekTime = nSeekTime;
  uint64 nSegKey = m_pSegmentHandler->GetKey();
  FileSource* pFileSource = m_pSegmentHandler->GetFileSource();
  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Seg [0x%08x%08x]: Seek %lld issued on File source"
                  " moving to seeking state",(uint32)(nSegMPDKey >> 32),
                  (uint32)nSegMPDKey, m_pSegmentHandler->m_nSeekTime );
  m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cSeekingStateHandler);
  if (pFileSource == NULL ||
      FILE_SOURCE_FAILED(pFileSource->SeekAbsolutePosition(m_pSegmentHandler->m_nSeekTime)))
  {
    //Notify error to representation handler
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    m_pSegmentHandler->OnError(HTTPCommon::HTTPDL_ERROR_ABORT);
    if(m_pSegmentHandler->m_pRepNotifier)
    {
      m_pSegmentHandler->m_pRepNotifier->NotifySeekStatus((uint32)nSegKey,
                                           m_pSegmentHandler->m_nSeekTime,
                                           HTTPCommon::HTTPDL_ERROR_ABORT);
    }
  }
  return status;
}

void DASHMediaSegmentHandler::SegmentOpenStateHandler::StateEntryHandler()
{
  if(m_pSegmentHandler->m_nSeekTime >= 0)
  {
    Seek(m_pSegmentHandler->m_nSeekTime);
  }
  return;
}

void DASHMediaSegmentHandler::SegmentSeekingStateHandler::ProcessFileSourceEvent
(
   FileSourceCallBackStatus status
)
{
  bool bError = true;
  bool bNotifySeek = true;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  FileSource* pFileSource = m_pSegmentHandler->GetFileSource();
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
  uint32 nSegKey =  (uint32)m_pSegmentHandler->GetKey();
  int64 nSeekTime = -1;
  HTTPCommon::HTTPMediaType mType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE,
                                       HTTPCommon::HTTP_TEXT_TYPE};

  //Process file source SEEK notification, notify rephandler of seek status
  if (status == FILE_SOURCE_SEEK_COMPLETE)
  {
    bError = false;
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Seg [0x%08x%08x]: File source reported SEEK_COMPLETE"
                   "moving back to OPEN", (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
    m_pSegmentHandler->m_nSeekTime = -1;

    if(pFileSource)
    {
      for(int i = 0; i < STD_ARRAY_SIZE(mType); i++)
      {
        HTTPResourceTrackMap *map = m_pSegmentHandler->GetTrackMapByMediaType(mType[i]);
        if(map && (map->m_nMediaTimeScale > 0))
        {
          nSeekTime = ((pFileSource->GetMediaCurrentPosition() * DASH_TIMESCALE) / map->m_nMediaTimeScale);
          break;
        }
     }
    }

    eStatus = HTTPCommon::HTTPDL_SUCCESS;

    m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cOpenStateHandler);
  }
  else if(status == FILE_SOURCE_SEEK_UNDERRUN || status == FILE_SOURCE_SEEK_UNDERRUN_IN_FRAGMENT)
  {
    if(pFileSource)
    {
      // If we are repeating a seek on filsesource and the media position returned
      // by filesource is greater than the desired seek time, then repeat the seek
      // with the returned media position for optimization. This case should happen
      // when clip contains Video track, filesource underrun is hit in between the
      // desired seek location and the location of the I-Frame which would result in
      // backward scan by filesource on a seek to desired seek location.
      uint64 mediaPosition = 0;

      for(int i = 0; i < STD_ARRAY_SIZE(mType); i++)
      {
        HTTPResourceTrackMap *map = m_pSegmentHandler->GetTrackMapByMediaType(mType[i]);
         if(map && (map->m_nMediaTimeScale > 0))
         {
           mediaPosition = ((pFileSource->GetMediaCurrentPosition() * DASH_TIMESCALE) / map->m_nMediaTimeScale);
           break;
         }
      }

      if ((mediaPosition <= 0x7fffffffffffffffULL) &&
          (m_pSegmentHandler->m_nSeekTime < (int64)mediaPosition))
      {
        m_pSegmentHandler->m_nSeekTime = mediaPosition;
      }
      //TODO: Remove this notify seek cmd should be queued only if it is
      //underrun at fragment boundary. The filesource status is not working
      //as expected. Remove this when we have proper changes from filesource
      if(status == FILE_SOURCE_SEEK_UNDERRUN_IN_FRAGMENT)
      {
        eStatus = HTTPCommon::HTTPDL_WAITING;
        nSeekTime = m_pSegmentHandler->m_nSeekTime;
      }
      else
      {
        bNotifySeek = false;
      }

      SegmentCmdData cmd;
      cmd.sCheckCmdData.eCmd = SEGMENT_CMD_CHECK_DATA_AVAIL_FOR_SEEK;
      cmd.sCheckCmdData.nCurrDataOffset = 0;
      bool bEOS = false;
      int64 nAvailOffset = 0;
      HTTPDataManager* pDataManager = m_pSegmentHandler->GetDataManager();
      pDataManager->GetAvailableOffset(&nAvailOffset,&bEOS);
      cmd.sCheckCmdData.nCurrDataOffset = nAvailOffset;
      if (m_pSegmentHandler->EnQCmd(cmd))
      {
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Data Segment  [0x%08x%08x] seek underrun "
                       "checkdataavail cmd successfully, cur download pos %d ",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                       (int)cmd.sCheckCmdData.nCurrDataOffset );
        bError = false;
      }
    }
  }
  else if (status == FILE_SOURCE_SEEK_FAIL)
  {
    //Queue a cmd to process SEEK failure
    SegmentCmdData cmd;
    cmd.sBaseCmdData.eCmd = SEGMENT_CMD_PROCESS_SEEK_FAILURE;
    cmd.sBaseCmdData.pDownloader = NULL;
    if (m_pSegmentHandler->EnQCmd(cmd))
    {
      bError = bNotifySeek = false;
    }
  }

  //Notify any file source errors (note this is fatal!!)
  if (bError)
  {
    m_pSegmentHandler->m_nSeekTime = -1;
    nSeekTime = -1;
    eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Seg [0x%08x%08x]: File source reported error, remove the segment",
                   (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
    m_pSegmentHandler->OnError(HTTPCommon::HTTPDL_ERROR_ABORT);
  }

  if(bNotifySeek)
  {
    if(m_pSegmentHandler->m_pRepNotifier)
    {
      m_pSegmentHandler->m_pRepNotifier->NotifySeekStatus((uint32)m_pSegmentHandler->GetKey(),nSeekTime,eStatus);
    }
  }
}

int DASHMediaSegmentHandler::SegmentSeekingStateHandler::ProcessCmds()
{
  int ret = 0;
  SegmentCmdData cmd;
  void* pIt = NULL;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  uint64 nSegKey = m_pSegmentHandler->GetKey();


  while(m_pSegmentHandler->m_cCmdQ.Next(pIt, cmd))
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;

    if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGINFO)
    {
      status = m_pSegmentHandler->ProcessGetSegmentInfoCmd(cmd.sGetSegInfoCmdData.nStartTime,
                                                           cmd.sGetSegInfoCmdData.nDuration);
    }
    else if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA)
    {
      status = m_pSegmentHandler->ProcessGetSegmentDataCmd(cmd);
    }
    else if (cmd.sCheckCmdData.eCmd == SEGMENT_CMD_CHECK_DATA_AVAIL_FOR_SEEK)
    {
      status = HTTPCommon::HTTPDL_WAITING;
      HTTPDataManager* pDataManager = m_pSegmentHandler->GetDataManager();
      int64 nAvailableOffset = 0;
      bool bEndOfFrag = false;
      (void)pDataManager->GetAvailableOffset(&nAvailableOffset, &bEndOfFrag);
      bEndOfFrag = bEndOfFrag || (m_pSegmentHandler->GetDataDownloadState() == DATA_DOWNLOAD_PARTIAL_COMPLETE);
      if( bEndOfFrag  ||
         ((uint64)nAvailableOffset > cmd.sCheckCmdData.nCurrDataOffset &&
          ((uint64)nAvailableOffset - cmd.sCheckCmdData.nCurrDataOffset) >= SEEK_POLL_DATA))
      {
        FileSource* pFileSource = m_pSegmentHandler->GetFileSource();
        if (pFileSource == NULL ||
          FILE_SOURCE_FAILED(pFileSource->SeekAbsolutePosition(m_pSegmentHandler->m_nSeekTime)))
        {
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
          //Notify error to representation handler
          m_pSegmentHandler->OnError(status);
          if(m_pSegmentHandler->m_pRepNotifier)
          {
            m_pSegmentHandler->m_pRepNotifier->NotifySeekStatus((uint32)nSegKey,
                m_pSegmentHandler->m_nSeekTime,HTTPCommon::HTTPDL_ERROR_ABORT);
          }
        }
        else
        {
          status = HTTPCommon::HTTPDL_SUCCESS;
        }
      }
    }
    else if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_PROCESS_SEEK_FAILURE)
    {
      //Process SEEK failure by moving the seek point to the start of the
      //next fragment. If current fragment is the last or there's no
      //fragment info, move segment to ERROR and notify above. PG/Rep
      //will try to move over to the next segment and if current segment
      //is the last should end the session!
      bool bError = true;
      int64 nSeekTime = m_pSegmentHandler->m_nSeekTime;
      if (m_pSegmentHandler->CloseMedia(HTTPCommon::HTTP_UNKNOWN_TYPE) == HTTPCommon::HTTPDL_SUCCESS)
      {
        if (m_pSegmentHandler->m_pSidxParser)
        {
          uint32 nNumTotalDataUnits = (uint32)m_pSegmentHandler->m_pSidxParser->get_data_chunk_count();
          for (uint32 i = 0; i < nNumTotalDataUnits; i++)
          {
            data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
            if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)i, &sFragInfo))
            {
              if (sFragInfo.n_start_time > (unsigned long long)nSeekTime)
              {
                nSeekTime = sFragInfo.n_start_time;
                bError = false;
                status = HTTPCommon::HTTPDL_NEXT_SEGMENT;
                QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                               "Seg [0x%08x%08x]: Seek failed in current fragment, moving over to"
                               " fragment %u, new seek time %lld msec",
                               (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, i, nSeekTime );
                break;
              }
            }
          }// for ()
        }// if (m_pSegmentHandler->m_pSidxParser)
      }// if (m_pSegmentHandler->CloseMedia() == HTTPCommon::HTTPDL_SUCCESS)

      //Notify any file source errors (note this is fatal!!)
      if (bError)
      {
        nSeekTime = m_pSegmentHandler->m_nSeekTime = -1;
        status = HTTPCommon::HTTPDL_ERROR_ABORT;

        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Seg [0x%08x%08x]: Seek error, remove the segment and move over",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey );
        m_pSegmentHandler->OnError(HTTPCommon::HTTPDL_ERROR_ABORT);
      }

      if (m_pSegmentHandler->m_pRepNotifier)
      {
        m_pSegmentHandler->m_pRepNotifier->NotifySeekStatus((uint32)m_pSegmentHandler->GetKey(), nSeekTime, status);
      }
    }

    //Dequeue cmd if processed
    if (status != HTTPCommon::HTTPDL_WAITING)
    {
      (void)m_pSegmentHandler->m_cCmdQ.Remove(pIt);
    }
  }

  return ret;
}

/** @brief Download data.
  *
  * @param[in] pDownloader - Reference to downloader
  * @return
  * HTTPDL_SUCCESS - Download completed successfully
  * HTTPDL_WAITING - More data to download
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::Download
(
 SegmentDownloader* pDownloader,
 HTTPDataManager* pDataManager
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  if (pDownloader && pDataManager)
  {
    byte* pBuffer = NULL;
    uint64 nKey = pDownloader->GetKey();
    uint64 nSegMPDKey = m_cSegmentInfo.getKey();

    //Get max data available in the stack and cap it at HTTP_MAX_DATA_READ_LIMIT, we're
    //going to read that much!
    int64 nBytesAvailable = 0;
    int64 nBytesToRead = 0;
    HTTPDownloadStatus result = pDownloader->Read(NULL, 0, nBytesAvailable);
    if (result == HTTPCommon::HTTPDL_WAITING)
    {
      status = HTTPCommon::HTTPDL_WAITING;
    }
    else if (result == HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER)
    {
      nBytesToRead = (nBytesAvailable <= 0) ? HTTP_MAX_DATA_CHUNK_LEN
                     : STD_MIN(nBytesAvailable, HTTP_MAX_DATA_READ_LIMIT);
      status = HTTPCommon::HTTPDL_SUCCESS;
    }
    else if (result == HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND)
    {
      status = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
    }

    //Based on if server honors byte range header as part of original url or uses byterangeURL
    //since it dishonors byte range header(for http1.0) update the m_eByteRangeURLRespState
    //If byterangeURL request/response is success for a given segment, other segmenthandlers
    //for the same representation send only byterangeURL to the segment downloader
    if(status == HTTPCommon::HTTPDL_SUCCESS)
    {
      if(m_cSegmentInfo.GetByteRangeURLResp() == HTTPCommon::BYTERANGE_URL_USED_UNDETERMINED)
      {
        if(pDownloader->IsByteRangeUrlInUse() == true)
        {
          m_cSegmentInfo.SetByteRangeURLResp(HTTPCommon::BYTERANGE_URL_USED_TRUE);
        }
        else
        {
          m_cSegmentInfo.SetByteRangeURLResp(HTTPCommon::BYTERANGE_URL_USED_FALSE);
        }
      }
    }

    //Read possibly multiple times and dump data into possibly multiple data manager segments,
    //yield if nBytesToRead bytes read or we run out of heap space or segment download done or
    //we hit an error
    while (status == HTTPCommon::HTTPDL_SUCCESS)
    {
      //Get data manager buffer, read data into it from stack and commit buffer. nBytesAvailable
      //at the end of the call indicates the max #bytes available (size of pBuffer)
      status = pDataManager->GetBuffer(pBuffer, nBytesAvailable, nKey);
      if (HTTPSUCCEEDED(status))
      {
        int64 nBytesRead = 0;
        UpdateIPAddr(m_pHTTPStack);
        result = pDownloader->Read(pBuffer, STD_MIN(nBytesAvailable, nBytesToRead), nBytesRead);
        if (result == HTTPCommon::HTTPDL_SUCCESS || result == HTTPCommon::HTTPDL_DATA_END)
        {
          uint64 nBytesReceived = pDownloader->GetNumBytesReceived();
          uint64 nContentLength = pDownloader->GetContentLength();
          int64 nSegContentLength = (int64)m_cSegmentInfo.GetContentLength();

          nBytesToRead -= nBytesRead;

          //Set total segment content length
          if (nSegContentLength < 0 && (nKey != HTTP_INIT_SEG_KEY && nKey != HTTP_NONSELF_INIT_SEG_KEY))
          {
            if (m_pSidxParser)
            {
              nSegContentLength = 0;
              data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
              uint32 nNumTotalDataUnits = (uint32)m_pSidxParser->get_data_chunk_count();
              for (uint32 i = 0; i < nNumTotalDataUnits; i++)
              {
                if (m_pSidxParser->get_data_chunk_info((unsigned int)i, &sFragInfo))
                {
                  nSegContentLength += sFragInfo.n_referenced_size;
                }
                else
                {
                  nSegContentLength = nContentLength;
                  break;
                }
              }
            }
            else
            {
              nSegContentLength = nContentLength;
            }

            m_cSegmentInfo.SetContentLength(nSegContentLength);
          }

          QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Seg [0x%08x%08x]: Data segment (key %u) download progress %llu/%llu/%lld bytes",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nKey,
                         nBytesReceived, nContentLength, nSegContentLength );

          //Commit the data buffer (bandwidth estimator is already updated in RA)
          status = pDataManager->CommitBuffer(pBuffer, nBytesRead, nKey);
          if (HTTPSUCCEEDED(status))
          {
            //Done with the segment if either stack says so or nContentLength bytes read out
            //(Note stack needs an additional read call to return HTTP_NOMOREDATA!)
            if (result == HTTPCommon::HTTPDL_DATA_END || (nBytesReceived == (uint32)nContentLength))
            {
              status = HTTPCommon::HTTPDL_DATA_END;
              QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "Seg [0x%08x%08x]: Data segment (key %u) download complete",
                             (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nKey );
            }
            else if (nBytesToRead <= 0)
            {
              status = HTTPCommon::HTTPDL_WAITING;
            }
          }
          else
          {
            QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Seg [0x%08x%08x]: Data segment (key %u) download buffer commit failed %d",
                           (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nKey, status );
            status = HTTPCommon::HTTPDL_ERROR_ABORT;
          }
        }
        else if (result == HTTPCommon::HTTPDL_WAITING)
        {
          status = HTTPCommon::HTTPDL_WAITING;
          QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                         "Seg [0x%08x%08x]: Data segment (key %u) waiting for data",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nKey );
        }
        else if (result == HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND)
        {
          status = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
        }
        else
        {
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
          QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Seg [0x%08x%08x]: Data segment (key %u) read failed %d",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nKey, result );
        }
      }
      else if (status == HTTPCommon::HTTPDL_OUT_OF_MEMORY)
      {
        //Wait for more memory to be freed up
        status = HTTPCommon::HTTPDL_WAITING;
        QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Seg [0x%08x%08x]: Data segment (key %u) download buffer unavailable, "
                       "Available space is %d bytes",(uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                       (uint32)nKey,m_sDASHSessionInfo.cHeapManager.GetMaxAvailableSpace() );
      }
    }// while (status == HTTPCommon::HTTPDL_SUCCESS)

    if (HTTPCommon::HTTPDL_SUCCESS == status ||
        HTTPCommon::HTTPDL_WAITING == status)
    {
      bool bIsTooSlow = pDownloader->CheckDownloadTooSlow();
      if (bIsTooSlow)
      {
        if (IsDataUnit(nKey))
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "DownloadTooSlow notification to be sent to rephandler for dataunit %d", (int)nKey);

          m_pRepNotifier->NotifyDownloadTooSlow(nSegMPDKey & MPD_SEGMENT_MASK, nKey);
        }
      }
    }
  }// if (pDownloader && pDataManager)

  return status;
}

/** @brief Process GetSegmentInfo Cmd. Finds the number of data units in
  * in the given range and notifies representation handler.
  * @param[in] nStartTime - start time of request
  * @param[in] nDuration - duration of request
  * @return
  * HTTPDL_SUCCESS - SegInfoReady notified successfully
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::ProcessGetSegmentInfoCmd
(
  uint64 nStartTime,
  uint64 nDuration
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  uint64 nSegMPDKey = m_cSegmentInfo.getKey();

  if (m_pRepNotifier)
  {
    uint32 nNumDataUnits = 0;
    (void)FillDataUnitInfo(nStartTime,
                           nDuration,
                           NULL, 0, nNumDataUnits);
    status = (nNumDataUnits > 0) ? HTTPCommon::HTTPDL_SUCCESS : HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Seg [0x%08x%08x]: GET_SEGINFO cmd (nStartTime %u nDuration %u nNumDataUnits %u) "
                   "processing complete %d - notifying representation",
                   (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                   (uint32)nStartTime,(uint32)nDuration,
                   nNumDataUnits, status );
    m_pRepNotifier->SegInfoReady((uint32)GetKey(),nNumDataUnits, status);
  }
  return status;
}

/** @brief Process GET_SEGDATA_INIT cmd , send out request and Qeue GET_SEGDATA cmd.
  *
  * @param[in] nDataUnitKey
  * @return
  * HTTPDL_SUCCESS - Download completed successfully
  * HTTPDL_WAITING - More data to download
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::ProcessGetSegmentDataCmd
(
  SegmentCmdData& cmd
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  uint64 nSegMPDKey = m_cSegmentInfo.getKey();
  if(cmd.sGetSegDataCmdData.eState == CMD_STATE_INIT)
  {
    status = ProcessGetSegmentDataInit(cmd);
  }
  else if(cmd.sGetSegDataCmdData.eState == CMD_STATE_DOWNLOADING)
  {
    status = ProcessGetSegmentDataDownload(cmd);
  }
  else if (cmd.sGetSegDataCmdData.eState == CMD_STATE_RETRY)
  {
    status = ProcessGetSegmentDataRetry(cmd);
  }
  else if(cmd.sGetSegDataCmdData.eState == CMD_STATE_CANCELLED)
  {
    status = ProcessCancelSegmentData(cmd.sGetSegDataCmdData.nDataUnitKey);
  }
  else if(cmd.sGetSegDataCmdData.eState == CMD_STATE_READ_ERROR)
  {
    OnError(HTTPCommon::HTTPDL_ERROR_ABORT);
    status = HTTPCommon::HTTPDL_WAITING;
  }

  return status;
}

/** @brief Process GET_SEGDATA cmd in init state, send out HTTP Get request
  *
  * @param[in] nDataUnitKey
  * @return
  * HTTPDL_SUCCESS - Download completed successfully
  * HTTPDL_WAITING - More data to download
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::ProcessGetSegmentDataInit
(
  SegmentCmdData& cmd
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  uint64 nDataUnitKey = cmd.sGetSegDataCmdData.nDataUnitKey;
  uint64 nSegMPDKey = m_cSegmentInfo.getKey();
  DataDownloadState eCurrDownloadState = GetDataDownloadState();
  bool bError = false;

  if (eCurrDownloadState != DATA_DOWNLOAD_ERROR)
  {
    //Get the next available downloader from the segment downloader pool
    SegmentDownloader* pDownloader =
    m_cSegDownloadMgr.GetNextAvailableSegmentDownloader();

    if(pDownloader != NULL)
    {
      pDownloader->SetSharedBwEstimator(m_bEstimator);
    }

    HTTPDataManager* pDataManager = GetDataManager();
    bool bSegAvailable = m_sDASHSessionInfo.cMPDParser.IsSegmentAvailable(nSegMPDKey);
    if (pDownloader && pDataManager && bSegAvailable)
    {
      //Get the data unit (fragment) offsets from sidx parser
      int64 nStartOffset = -1, nEndOffset = -1;
      int64 nDMStartOffset = -1;
      int nDurationMs = -1;
      uint64 nStartTime = m_nStartTime;
      if(m_pSidxParser)
      {
        //If sidx is present then compute the range offsets for data unit
        data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
        if (m_pSidxParser &&
            m_pSidxParser->get_data_chunk_info((unsigned int)nDataUnitKey, &sFragInfo))
        {
          //Compute the url byte range offsets for the data unit (discount init seg)
          // m_offsetAdjust is to adjust offsets in case of seperate index url
          nStartOffset = sFragInfo.n_offset - m_offsetAdjust;

          // As init segment sit before the SIDX segment in the datamanager, hence Fragments offsets from
          // Sidx parser requires adjustment for init segment size only if init segment is from different URL
          // Fragment offset adjustment is not required for Same Init and Media URL.
          // Cache the information regarding for init URL and use aptly for segment offset adjustment
          nStartOffset -= ((m_pInitSegment && (!m_bInitAndMediaWithSameURL)) ?
                           m_pInitSegment->GetNumBytesDownloaded() : 0);
          nEndOffset = nStartOffset + sFragInfo.n_referenced_size - 1;
          nDMStartOffset = sFragInfo.n_offset - m_offsetAdjust;
          nStartTime = sFragInfo.n_start_time;
          nDurationMs = (int)sFragInfo.n_subsegment_duration;
        }
      }
      else
      {
        //If sidx is not present, send the request for complete segment from beginning
        nStartOffset = 0;
        nDMStartOffset = (m_pInitSegment ?
                          m_pInitSegment->GetNumBytesDownloaded() : 0);
      }

      //Data unit start offset must be strictly greater than 0 for sidx present case
      // If sidx not present then it can be 0
      if (nStartOffset >= 0)
      {
        if (cmd.sGetSegDataCmdData.bIsRetrying)
        {
          // retry with SIDX fetch disabled.
          QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Seg [0x%08x%08x]: Data segment (key %u) retry for SIDX fetch disabled",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nDataUnitKey );
          status = HTTPCommon::HTTPDL_SUCCESS;
        }
        else
        {
          //Create data manager segment with the given key
          status = pDataManager->CreateSegment(nDataUnitKey, iHTTPBase::SEGMENT_DEFAULT,
                                               nDMStartOffset, true);
        }

        if (status == HTTPCommon::HTTPDL_EXISTS)
        {
          //Segment already present, nothing to do
          status = HTTPCommon::HTTPDL_WAITING;
          QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Seg [0x%08x%08x]: Data segment (key %u) already exists",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nDataUnitKey );
        }
        else if (status == HTTPCommon::HTTPDL_SUCCESS)
        {
          //Start the downloader and enqueue the download cmd
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
          char *urlRange = m_cSegmentInfo.GetUrlRange();
          if(urlRange)
          {
            int64 segmentRangeStart = 0;
            parseByteRange(urlRange,segmentRangeStart,nEndOffset);
            nStartOffset = segmentRangeStart + nStartOffset;
          }

          //The downloaded segment URL is updated to the original segment URL or ByteRangeURL,
          //based on if server is HTTP1.1 or HTTP1.0
          //If server is HTTP1.0, it doesnot support partial GET requests and dishonors byte range header fields.
          //Hence a byterangeURL is constructed and sent to the downloader.
          if (StartDownload(pDownloader,
                            nDataUnitKey,
                            nStartOffset,
                            nEndOffset,
                            m_cSegmentInfo.GetURL(),
                            nDurationMs))
          {
            pDownloader->SetSegNotifier(this);
            SegmentCmdData* cmd;
            void *pIt = NULL;
            while(m_cCmdQ.Next(pIt,&cmd))
            {
              if(cmd->sGetSegDataCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA
                 && cmd->sGetSegDataCmdData.nDataUnitKey == nDataUnitKey)
              {
                //Move the get segment data cmd state to downloading
                cmd->sGetSegDataCmdData.eState = CMD_STATE_DOWNLOADING;

                status = HTTPCommon::HTTPDL_WAITING;
                QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                               "Seg [0x%08x%08x]: ProcessGetSegmentDataInit GetSegmentData"
                               "(key %u) state modified to downloading",
                               (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nDataUnitKey);
                break;
              }
            }
          }
          else
          {
            (void)pDataManager->AbortSegment(nDataUnitKey);
            status = HTTPCommon::HTTPDL_ERROR_ABORT;
            QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Seg [0x%08x%08x]: Download (key %u) could not be started",
                           (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nDataUnitKey );
          }
        }
        else
        {
          QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Seg [0x%08x%08x]: Data manager CreateSegment() failed %d (key %u)",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, status, (uint32)nDataUnitKey );
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
      }
    }
    else if(!bSegAvailable)
    {
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Seg [0x%08x%08x]: Segment %d is not available now on server",
                     (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,(uint32)nDataUnitKey );

      //Move to ERROR if download hasn't started on the segment, else just mark segment complete
      //and read path will take care of resource cleanup once entirely read!
      if (eCurrDownloadState == DATA_DOWNLOAD_NOT_STARTED)
      {
        OnError(status);
        bError = true;
      }
      else
      {
        SetDataDownloadState(DATA_DOWNLOAD_ERROR);
        MarkSegmentComplete();
      }
    }
  }

  if (status != HTTPCommon::HTTPDL_WAITING)
  {
    //Notify download status (OnError already notifies no need to double-notify)
    if (!bError)
    {
      if (m_pRepNotifier)
      {
        QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Seg [0x%08x%08x]: GET_SEGDATA cmd (key %u) processing complete %d in CMD_STATE_INIT - "
                       "notifying representation", (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                       (uint32)nDataUnitKey, status );
        m_pRepNotifier->SegDataReady((uint32)GetKey(), nDataUnitKey, status);
      }
    }
  }

  return status;
}

/** @brief Process GET_SEGDATA cmd - download data and notify.
  *
  * @param[in] pDownloader - Reference to downloader
  * @return
  * HTTPDL_SUCCESS - Download completed successfully
  * HTTPDL_WAITING - More data to download
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::ProcessGetSegmentDataDownload
(
   SegmentCmdData& cmd
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  uint64 nDataUnitKey = cmd.sGetSegDataCmdData.nDataUnitKey;
  uint32 nCmdID = cmd.sGetSegDataCmdData.nCmdId;

  uint64 nSegMPDKey = m_cSegmentInfo.getKey();
  SegmentDownloader* pDownloader = m_cSegDownloadMgr.GetSegmentDownloader(nDataUnitKey);
  bool bError = false;

  if (pDownloader && pDownloader->IsInUse())
  {
    //Download only when in non-error state
    DataDownloadState eCurrDownloadState = GetDataDownloadState();
    if (eCurrDownloadState != DATA_DOWNLOAD_ERROR)
    {
      //Try downloading some data
      status = Download(pDownloader, GetDataManager());
      if (pDownloader->GetNumBytesReceived() > 0)
      {
        eCurrDownloadState = DATA_DOWNLOAD_STARTED;
        m_bProcessOpenCmd = true;
      }
      else if (HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND == status &&
               m_rSidxFetchDecision.IsSidxFetchDisabled())
      {
        // move the data cmd associated th fragment download to retry state
        // if segment is available
        MM_Time_DateTime currentTime;
        MM_Time_GetUTCTime(&currentTime);
        double CurrMsec = StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);
        double SegAvailabilityTimeMsec = m_cSegmentInfo.getAvailabilitytime();
        uint32 SegDuration = (uint32)m_cSegmentInfo.getDuration();

        QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Seg [0x%08x%08x]: Data unit (key %u) SIDX fetch disabled will retry if %d < %u",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nDataUnitKey,
                       (int)(CurrMsec - SegAvailabilityTimeMsec),SegDuration) ;

        if ((SegAvailabilityTimeMsec + (double)SegDuration) > CurrMsec)
        {
          cmd.sGetSegInfoCmdData.nLastFailureTime = m_pClock->GetTickCount();
          cmd.sGetSegDataCmdData.eState = CMD_STATE_RETRY;
          status = HTTPCommon::HTTPDL_WAITING;
        }
        else
        {
          QTV_MSG_PRIO5(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "Seg [0x%08x%08x]:not Retrying, as SegmentAbilityTime(%u + dutation(%u)) is before the currentTime(%u)",
                             (uint32)(nSegMPDKey >> 32),(uint32)nSegMPDKey,(uint32)SegAvailabilityTimeMsec,
                             (uint32)SegDuration,(uint32)CurrMsec);
          status = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
        }
      }

      DataDownloadState eNewDownloadState = eCurrDownloadState;
      if (status != HTTPCommon::HTTPDL_WAITING)
      {
        uint64 nBytesReceived = pDownloader->GetNumBytesReceived();
        uint64 nEndOffset =
          (m_pInitSegment  ? m_pInitSegment ->GetNumBytesDownloaded() : 0) +
          pDownloader->GetStartOffset() + nBytesReceived;
        HTTPDataManager* pDataManager = GetDataManager();

        //Mark DM segment complete on success
        if (status == HTTPCommon::HTTPDL_DATA_END)
        {
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
          if (pDataManager)
          {
            (void)pDataManager->SetSegmentComplete(nDataUnitKey, nEndOffset);
            status = HTTPCommon::HTTPDL_SUCCESS;
          }
        }

        bool bLast = ((m_pSidxParser && m_pSidxParser->get_data_chunk_count() - 1 == nDataUnitKey) ||
                      (m_pSidxParser == NULL));
        eNewDownloadState = ((status == HTTPCommon::HTTPDL_SUCCESS) ?
                             (bLast ? DATA_DOWNLOAD_COMPLETE : DATA_DOWNLOAD_PARTIAL_COMPLETE) : DATA_DOWNLOAD_ERROR);
        if (bLast || eNewDownloadState == DATA_DOWNLOAD_ERROR)
        {
          if (eNewDownloadState == DATA_DOWNLOAD_ERROR)
          {
            if (pDataManager)
            {
              //Abort segment only if download hasn't started. Partially downloaded (sub)segments
              //are allowed to be read and by marking DM segment complete in that case occupancy
              //will count the total (sub)segment duration, so mark DM segment complete
              if (nBytesReceived == 0)
              {
                (void)pDataManager->AbortSegment(nDataUnitKey);
              }
              else
              {
                (void)pDataManager->SetSegmentComplete(nDataUnitKey, nEndOffset);
              }
            }

            //Mark ERROR if download hasn't started on the segment
            if (eCurrDownloadState == DATA_DOWNLOAD_NOT_STARTED)
            {
              bError = true;
            }
          }

          //Mark EOF when we want downloaded data on this unit to be read
          if (!bError)
          {
            MarkSegmentComplete();
          }

          QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Seg [0x%08x%08x]: Data unit (key %u) download state %d/%d",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (uint32)nDataUnitKey,
                         eCurrDownloadState, eNewDownloadState );
        }
      }

      SetDataDownloadState(eNewDownloadState);

      //Process any errors
      if (bError)
      {
        // cmdID will be set only in case of '404'.
        OnError(status, nCmdID);
      }
    }// if (eCurrDownloadState != DATA_DOWNLOAD_ERROR)
  }// if (pDownloader && pDownloader->IsInUse())

  if (status != HTTPCommon::HTTPDL_WAITING)
  {
    //Notify download status (OnError already notifies no need to double-notify)
    if (!bError)
    {
      DownloadEnded(pDownloader);
      if (m_pRepNotifier)
      {
        QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Seg [0x%08x%08x]: GET_SEGDATA cmd (key %u) processing complete %d in CMD_STATE_DOWNLOADING - "
                       "notifying representation", (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                       (uint32)nDataUnitKey, status );
        m_pRepNotifier->SegDataReady((uint32)GetKey(), nDataUnitKey, status);
      }
    }
  }

  return status;
}

/**
 * For this representation, fetching SIDX is turned off. Stay in
 * this state for the data unit while retrying entire segment
 * during its retry window.
 */
HTTPDownloadStatus DASHMediaSegmentHandler::ProcessGetSegmentDataRetry(SegmentCmdData& cmd)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_WAITING;

  uint64 nSegMPDKey = m_cSegmentInfo.getKey();
  HTTPDataManager* pDataManager = GetSidxDataManager();

  SegmentDownloader* pDownloader =
    m_cSegDownloadMgr.GetSegmentDownloader(cmd.sGetSegDataCmdData.nDataUnitKey);

  if (pDownloader)
  {
    uint64 segKey = 0;
    uint64 lastAvailableSegmentEndTime, lastAvailableSegmentStartTime = 0;

    HTTPDownloadStatus mpdStatus =
      m_sDASHSessionInfo.cMPDParser.GetLastAvailableSegmentTimeForRepresentation(
         m_cSegmentInfo.getKey(),
         lastAvailableSegmentStartTime,
         lastAvailableSegmentEndTime);

    // Retry if Next Segment is unavailable or the current segment is the last segment
    if (((mpdStatus == HTTPCommon::HTTPDL_SUCCESS) &&
         (m_cSegmentInfo.getStartTime()>= lastAvailableSegmentStartTime)) ||
        mpdStatus == HTTPCommon::HTTPDL_DATA_END)
    {
      MM_Time_DateTime currentTime;
      MM_Time_GetUTCTime(&currentTime);
      uint32 nElapsedTimeSinceLastFailure = HTTPCommon::GetElapsedTime(m_pClock,
                                                                       cmd.sBaseCmdData.nLastFailureTime);
      double currMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);
      double SegAvailabilityTimeMsec = m_cSegmentInfo.getAvailabilitytime();
      uint32 SegDuration = (uint32)m_cSegmentInfo.getDuration();
      uint32 nMaxRetryDuration = (uint32)((double)SegDuration * SEGMENT_DURATION_FRACTION_FOR_RETRY_DURATION);
      uint32 SleepTime = (uint32)((double)SegDuration * SEGMENT_DURATION_FRACTION_FOR_RETRY_INTERVAL);

      if ( ((SegAvailabilityTimeMsec + (double)nMaxRetryDuration) > currMSeconds)  &&  nElapsedTimeSinceLastFailure >= SleepTime) //&& nElapsedTime <  nMaxRetryDuration)
      {
        (void)pDownloader->Stop();

        QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,"Seg [0x%08x%08x]:SegmentSegmentInfoRetryHandler: Retrying(%d)..currTime(%u)/MaxRetryWindow(%u) with SIDX optimization",
                       (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey , m_numRetriesExcerised, (uint32)currMSeconds,
                       (uint32)(SegAvailabilityTimeMsec + (double)nMaxRetryDuration));

        cmd.sGetSegDataCmdData.eState = CMD_STATE_INIT;
        cmd.sGetSegDataCmdData.bIsRetrying = true;
      }
      else if((SegAvailabilityTimeMsec + (double)nMaxRetryDuration) <= currMSeconds)
      {
        QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Seg [0x%08x%08x]:SegmentSegmentInfoRetryHandler:Beyond retry window SegAvaibilityTime(%u), CurrMsec (%u),MaxRetryTime"
                       "(%u),Retries attempted(%d)",
                       (uint32)(nSegMPDKey >> 32),(uint32)nSegMPDKey,(uint32)SegAvailabilityTimeMsec,
                       (uint32)currMSeconds,(uint32)(SegAvailabilityTimeMsec + (double)nMaxRetryDuration), m_numRetriesExcerised);
        eStatus = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
      }
    }
    else if ( lastAvailableSegmentStartTime >= m_cSegmentInfo.getStartTime())
    {
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,"Seg [0x%08x%08x]:SegmentSegmentInfoRetryHandler: Stopping Retries as Next Segment with time(%llu) is available",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,lastAvailableSegmentStartTime);
      eStatus = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
    }
    else if (mpdStatus != HTTPCommon::HTTPDL_SUCCESS || mpdStatus != HTTPCommon::HTTPDL_WAITING)
    {
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,"Seg [0x%08x%08x]:SegmentSegmentInfoRetryHandler:GetLastAvailableSegment Failed status:%d",
                     (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,mpdStatus);
      eStatus = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
    }

    if (eStatus != HTTPCommon::HTTPDL_WAITING)
    {
      if(pDataManager)
      {
        pDataManager->AbortSegment(HTTP_INIT_SEG_KEY);
      }
      DownloadEnded(pDownloader);
      OnError(eStatus, cmd.sBaseCmdData.nCmdId);
    }
  }

  return eStatus;
}

/** @brief Process CANCEL_SEGDATA cmd - cancel download and notify.
  *
  * @param[in] pDownloader - Reference to downloader
  * @return
  * HTTPDL_SUCCESS - Download completed successfully
  * HTTPDL_WAITING - More data to download
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaSegmentHandler::ProcessCancelSegmentData
(
 uint64 nDataUnitKey,
 bool /* bStopTimer */
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_INTERRUPTED;
  uint64 nSegMPDKey = m_cSegmentInfo.getKey();

  // If the Download Request has been assgined to any downloader then
  // Get the segment downloader for the given key
  SegmentDownloader* pDownloader =
       m_cSegDownloadMgr.GetSegmentDownloader(nDataUnitKey);
 //Check if pending download exists, if so don't change download state (e.g. 2
 //GET_SEGDATA cmds and state is STARTED and one of the download cmds is
 //canceled, in which case no need to go back to NOT_STARTED. However this is
 //fairly minor coz even if we did go back to NOT_STARTED, the pending
 //GET_SEGDATA cmd will take the state forward!)
  bool bPendingDownload = false;
  SegmentCmdData cmd;
  void* pIt = NULL;
  while(m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sGetSegDataCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA &&
        cmd.sGetSegDataCmdData.nDataUnitKey != nDataUnitKey)
    {
      bPendingDownload = true;
      break;
    }
  }
  if (!bPendingDownload)
  {
    SetDataDownloadState(DATA_DOWNLOAD_NOT_STARTED);
  }

  //Stop the downloader and abort the segment
  DownloadEnded(pDownloader);

  HTTPDataManager* pDataManager = GetDataManager();
  if (pDataManager)
  {
    (void)pDataManager->AbortSegment(nDataUnitKey);
  }

  QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Seg [0x%08x%08x]: GET_SEGDATA cmd (key %u) processing"
                  "complete(CANCEL state) %d - ",(uint32)(nSegMPDKey >> 32),
                  (uint32)nSegMPDKey,(uint32)nDataUnitKey, status );

  return status;
}

/*
 * Get the current download position for the given media type
 *
 * @param[in] eMediaType - media type
 * @param[out] nDownloadPosition - download position
 *
 * @return TRUE - success, FALSE - otherwise
 */
bool DASHMediaSegmentHandler::SegmentBaseStateHandler::IsDownloadComplete()
{
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  bool bResult = false;

  HTTPDataManager *pDataManager = m_pSegmentHandler->GetDataManager();
  if (pDataManager)
  {
    //Query data manager for the EOS flag. bResult is set to true if end of file is
    // set in data manager
    int64 nAvailOffset = 0;
    pDataManager->GetAvailableOffset(&nAvailOffset,&bResult);
  }
  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                 "Seg [0x%08x%08x]: Download status on data unit %d",
                 (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, bResult );

  return bResult;
}

/*
 * Get the current download position for the given media type
 *
 * @param[in] eMediaType - media type
 * @param[out] nDownloadPosition - download position
 *
 * @return TRUE - success, FALSE - otherwise
 */
bool DASHMediaSegmentHandler::SegmentBaseStateHandler::GetDownloadPosition
(
 HTTPCommon::HTTPMediaType /* eMediaType */,
 uint64& nDownloadPosition,
 bool& bIsPartiallyDownloadedResourceReached
)
{
  nDownloadPosition = 0;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  bool bResult = false;

  HTTPDataManager *pDataManager = m_pSegmentHandler->GetDataManager();
  if (pDataManager)
  {
    HTTPSegmentsInfo &segmentsInfo = m_pSegmentHandler->m_httpSegmentsInfo;
    HTTPDownloadStatus status = pDataManager->GetAvailableSegments(segmentsInfo, 0 );
    if (status == HTTPCommon::HTTPDL_SUCCESS)
    {
      bool bQueryFS = false;
      bResult = true;

      //Get the max over all data units (ideally if download is complete on all of them
      //this is the same as querying only the last data unit)
      for (int i = 0; i < segmentsInfo.m_NumSegmentsInUse; i++)
      {
        //Ignore the init segment(s)
        int nCurrKey = (int)segmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_Key;
        int64 nCurrStartOffset = segmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_nStartOffset;
        if (nCurrKey != (int)HTTP_INIT_SEG_KEY && nCurrKey!= (int)HTTP_NONSELF_INIT_SEG_KEY)
        {
          //Consider only fully downloaded segments for calculation (for now)
          //ToDo: Accumulate download offset to get the max download byte offset and use
          //GetTimeForFileOffset() to convert. Even FS needs to support this API for 3g2
          //Also if downloads are not sequential inside seg mgr this might change!
          //nOffset += segmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_NumBytesDownloaded;
          if (m_pSegmentHandler->m_pSidxParser)
          {
            data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};
            if (m_pSegmentHandler->m_pSidxParser->get_data_chunk_info((unsigned int)nCurrKey, &sFragInfo))
            {
              if (segmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_bIsFullyDownloaded)
              {
                nDownloadPosition = sFragInfo.n_start_time + sFragInfo.n_subsegment_duration;
              }
              else
              {
                //Not fully downloaded, stop accumulating
                nDownloadPosition = sFragInfo.n_start_time;
                QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                               "Seg [0x%08x%08x]: Download incomplete on data unit %d, breaking",
                               (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, nCurrKey );
                bQueryFS = true;
                bIsPartiallyDownloadedResourceReached = true;
                break;
              }
            }
            else
            {
              bQueryFS = true;
            }
          }
          else
          {
            if(segmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_bIsFullyDownloaded)
            {
              nDownloadPosition = m_pSegmentHandler->m_nStartTime +
                                    m_pSegmentHandler->m_nDuration;
            }
            else
            {
              bQueryFS = true;
              bIsPartiallyDownloadedResourceReached = true;
              break;
            }
          }
        }
      }// for()

      if (bQueryFS)
      {
        //(void)GetTimeForFileOffset(eMediaType, nOffset, nDownloadPosition);
      }
    }
    else
    {
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Seg [0x%08x%08x]: Could not get data segment info %d",
                     (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, status );
    }
  }

  return bResult;
}

/** @brief Clear segment cmd Q.
  *
  */
void DASHMediaSegmentHandler::ClearCmdQ(HTTPDownloadStatus eCmdIdStatus, uint32 nCmdId)
{
  SegmentCmdData cmd;
  void* pIt = NULL;
  SegmentCmdResponseData* pSegCmdRespArray = NULL;
  int nResp = 0;
  uint64 nSegMPDKey = m_cSegmentInfo.getKey();

  MM_CriticalSection_Enter(m_pSegmentDataLock);

  if( m_nSeekTime > 0 )
  {
    m_pRepNotifier->NotifySeekStatus((uint32)GetKey(), -1, HTTPCommon::HTTPDL_ERROR_ABORT);
  }
  m_nSeekTime = -1;

  //Clear and reset cmd Q, notify failure for any outstanding cmds
  if (!m_cCmdQ.Empty())
  {
    //Store the notifications in a response queue and invoke callbacks
    //after relinquishing the lock
    pSegCmdRespArray = (SegmentCmdResponseData *)
      QTV_Malloc(m_cCmdQ.Count() * sizeof(SegmentCmdResponseData));
    if (pSegCmdRespArray)
    {
      while(m_cCmdQ.Next(pIt, cmd))
      {
        HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

        if (nCmdId == cmd.sBaseCmdData.nCmdId)
        {
          // override the status for this command, rest all are
          // error abort.
          eStatus = eCmdIdStatus;
        }

        if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGINFO)
        {
          SegmentDownloader* pDownloader =
              m_cSegDownloadMgr.GetSegmentDownloader(HTTP_INIT_SEG_KEY);
          //Stop downloader (if not already done) and notify failure
          DownloadEnded(pDownloader);
          pSegCmdRespArray[nResp].sGetSegInfoCmdResponseData.eCmd = SEGMENT_CMD_GET_SEGINFO;
          pSegCmdRespArray[nResp].sGetSegInfoCmdResponseData.nSegKey = (uint32)GetKey();
          pSegCmdRespArray[nResp].sGetSegInfoCmdResponseData.eStatus = eStatus;
          pSegCmdRespArray[nResp].sGetSegInfoCmdResponseData.nNumDataUnits = 0;
          nResp++;
          QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Seg [0x%08x%08x]: GET_SEGINFO cmd (nStartTime %u nDuration %u) "
                         "cleared - notifying representation",
                         (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                         (uint32)cmd.sGetSegInfoCmdData.nStartTime,
                         (uint32)cmd.sGetSegInfoCmdData.nDuration );
        }
        else if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_GET_SEGDATA)
        {
          //Stop downloader (if not already done - e.g. connection errors can cause
          //downloader to close itself) and notify failure
          SegmentDownloader* pDownloader =
              m_cSegDownloadMgr.GetSegmentDownloader(cmd.sGetSegDataCmdData.nDataUnitKey);
          DownloadEnded(pDownloader);
          pSegCmdRespArray[nResp].sGetSegDataCmdResponseData.eCmd = SEGMENT_CMD_GET_SEGDATA;
          pSegCmdRespArray[nResp].sGetSegDataCmdResponseData.nSegKey = (uint32)GetKey();
          pSegCmdRespArray[nResp].sGetSegDataCmdResponseData.eStatus = eStatus;
          pSegCmdRespArray[nResp].sGetSegDataCmdResponseData.nDataUnitKey = cmd.sGetSegDataCmdData.nDataUnitKey;
          nResp++;
          QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Seg [0x%08x%08x]: GET_SEGDATA cmd (key %u) cleared - "
                         "notifying representation", (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey,
                         (uint32)cmd.sGetSegDataCmdData.nDataUnitKey );
        }
        else if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_CHECK_DATA_AVAIL_FOR_SEEK ||
                 cmd.sBaseCmdData.eCmd == SEGMENT_CMD_PROCESS_SEEK_FAILURE)
        {
          //ToDo: Is there a need to notify SEEK failure?
        }

        //Remove the segment cmd
        (void)m_cCmdQ.Remove(pIt);
      }

      (void)m_cCmdQ.Reset();
    }
  }

  MM_CriticalSection_Leave(m_pSegmentDataLock);

  if (pSegCmdRespArray)
  {
    for (int i = 0; i < nResp; i++)
    {
      switch (pSegCmdRespArray[i].sCmdBaseResponseData.eCmd)
      {
      case SEGMENT_CMD_GET_SEGINFO:
        if (m_pRepNotifier)
        {
          m_pRepNotifier->SegInfoReady(pSegCmdRespArray[i].sGetSegInfoCmdResponseData.nSegKey,
                                       pSegCmdRespArray[i].sGetSegInfoCmdResponseData.nNumDataUnits,
                                       pSegCmdRespArray[i].sGetSegInfoCmdResponseData.eStatus);
        }
        break;

      case SEGMENT_CMD_GET_SEGDATA:
        if (m_pRepNotifier)
        {
          m_pRepNotifier->SegDataReady(pSegCmdRespArray[i].sGetSegDataCmdResponseData.nSegKey,
                                       pSegCmdRespArray[i].sGetSegDataCmdResponseData.nDataUnitKey,
                                       pSegCmdRespArray[i].sGetSegDataCmdResponseData.eStatus);
        }
        break;

      default:
        //No need to notify
        break;
      }
    }

    QTV_Free(pSegCmdRespArray);
    pSegCmdRespArray = NULL;
  }
}

/*
 * Flushes all data before given data unit. Stores the seek time
 * if start time given is not same as segment start time or bSeek
 * flag is true.
 *
 * @param[in] nDataUnitKey - data unit key
 * @param[in] nStartTime - start time
 * @param[in] bSeek - seek flag
 *
 * @return HTTPDL_WAITING- queued the command
 *         HTTPDL_ERROR_ABORT - failure
 */
HTTPDownloadStatus DASHMediaSegmentHandler::Open
(
  uint64 nDataUnitKey,
  int64 nStartTime,
  bool bSeek
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  //Flush all data before nDataUnitKey and reset offset in datamanager.
  HTTPDataManager* pDataManager = GetDataManager();
  uint64 nSegMPDKey = m_cSegmentInfo.getKey();

  SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();
  if (pCurrentStateHandler)
  {
    status = pCurrentStateHandler->Flush(HTTPCommon::HTTP_UNKNOWN_TYPE,nStartTime);
  }
  if(pDataManager)
  {
    //Reset the offset in datamanager so that offset for segment
    //corresponding to nDataUnitKey is immediately after sidx or
    //in case of no sidx immediately after init segment(or 0 for
    //no init segment).
    (void)pDataManager->ResetOffset(nDataUnitKey);
  }
  ResetDataDownloadState();

  m_bIsResourceReadDisabled = false;
  m_bIsLmsgSet = false;

  uint64 nDownloadPos = 0;
  bool bIsPartiallyDownloadedResourceReached = false;
  if(GetDownloadPosition(HTTPCommon::HTTP_UNKNOWN_TYPE,nDownloadPos, bIsPartiallyDownloadedResourceReached))
  {
    //If any fragment is completely downloaded make the download
    //state to download started. If download is partial, next time when
    //GET_SEGDATA is processed state will be updated.
    if(nDownloadPos > 0)
    {
      SetDataDownloadState(DATA_DOWNLOAD_STARTED);
    }
  }
  //If this is a seek case or switch is in middle of segment issue a
  //seek on filesource. Store the seek time here, seek will be issued
  //in openstateentry handler
  uint64 nFragStartTime = 0;
  if(m_pSidxParser)
  {
    data_chunk_info sFragInfo;
    (void)m_pSidxParser->get_data_chunk_info((unsigned int)nDataUnitKey,&sFragInfo);
    nFragStartTime = sFragInfo.n_start_time;
  }
  else
  {
    nFragStartTime = m_nStartTime;
  }
  if(bSeek)
  {
    m_nSeekTime = nStartTime;
  }
  status = OpenSegment();
  return status;
}

/*
 * Flushes data for given media type before given start time.
 * If start time is -1 flushes all the data.
 * @param[in] eMediaType - media type
 * @param[in] nStarttime - start time.
 *
 * @return HTTPDL_SUCCESS - success
 *         HTTPDL_ERROR_ABORT - failure
 */
HTTPDownloadStatus DASHMediaSegmentHandler::Flush
(
  HTTPMediaType majorType,
  int64 nStartTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  //If start time given is -1. Flush everything for given media type.
  if(nStartTime != -1)
  {
    //Set flush time in HTTPResource for the given major type. And
    //Get the minimum flush time across all media types. This ensures
    //that only the data which is read out on all media types is deleted
    SetFlushTime(majorType,nStartTime);
    nStartTime = GetMinFlushTime();
  }
  SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();

  if (pCurrentStateHandler)
  {
    status = pCurrentStateHandler->Flush(majorType,nStartTime);
  }

  return status;
}

void DASHMediaSegmentHandler::ClearBufferedData(uint64 nStartTime)
{
  SegmentBaseStateHandler* pCurrentStateHandler = GetStateHandler();

  if (pCurrentStateHandler)
  {
    pCurrentStateHandler->ClearBufferedData(nStartTime);
  }
}
/**
 * Get the start time of the last data unit.
 */
bool DASHMediaSegmentHandler::GetStartTimeForLastDataUnit(uint64& nStartTime,
                                       double repEndTime,
                                       bool &bIsLmsgSet)
{
  //Check if 'lmsg' is set or the segment is last segment of period in mpd
  //and then fetch the start time of the last data unit
  bool rslt = false;

  if (repEndTime > 0.0)
  {
    bool bLastSegment = false;
    bIsLmsgSet = m_bIsLmsgSet;
    if (m_bIsLmsgSet ||
        m_sDASHSessionInfo.cMPDParser.IsLastSegment(m_cSegmentInfo.getKey()))
    {
      bLastSegment = true;
    }

    if (bLastSegment)
    {
      if (m_pSidxParser)
      {
        uint32 nNumTotalDataUnits = (uint32)m_pSidxParser->get_data_chunk_count();

        if (nNumTotalDataUnits > 0)
        {
          // TO DO: more efficient to do binary search here. However, in the common
          // use case the period duration in the mpd is expected to be inline with
          // the media data.
          for (int64 i = nNumTotalDataUnits - 1; i >= 0; --i)
          {
            data_chunk_info sFragInfo = {0,0,0,0,0,0,0,0,0};

            if (m_pSidxParser->get_data_chunk_info((unsigned int)i, &sFragInfo))
            {
              if (sFragInfo.n_start_time >= repEndTime)
              {
                continue;
              }

              nStartTime = sFragInfo.n_start_time;
              rslt = true;
              break;
            }
            else
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Unexpected error: get_data_chunk_info failed");
              break;
            }
          }
        }
      }
      else
      {
        //If sidx is not present segment start time is the last data unit start
        if (m_nStartTime < repEndTime)
        {
          nStartTime = m_nStartTime;
          rslt = true;
        }
      }
    }
  }

  return rslt;
}

/**
 * Mark that the resource reads have started on in. This means
 * that the fragment which has been detected as too slow cannot
 * be cancelled.
 */
void DASHMediaSegmentHandler::MarkResourceReadsStarted()
{
  MM_CriticalSection_Enter(m_pSegmentDataLock);
  if (false == m_bIsReadsStarted)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "MarkReadsStarted for segkey %d", (int)GetKey());
    m_bIsReadsStarted = true;
  }
  MM_CriticalSection_Leave(m_pSegmentDataLock);
}

/**
 * Check if reads have started on the resource. If not, then
 * disable the resource for sample reads and return true.
 *
 * @return bool
 *  true: if resource is disabled for sample reads.
 *  false: if resource is not disabled for sample reads.
 */
bool DASHMediaSegmentHandler::CheckReadsStartedAndDisableReads()
{
  MM_CriticalSection_Enter(m_pSegmentDataLock);
  if (false == m_bIsReadsStarted)
  {
    m_bIsResourceReadDisabled = true;
  }
  MM_CriticalSection_Leave(m_pSegmentDataLock);

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "CheckReadsStartedAndDisableReads returned %d",
     m_bIsResourceReadDisabled);

  return m_bIsResourceReadDisabled;
}

/**
 * Check if this resource is disabled for sample reads.
 */
bool DASHMediaSegmentHandler::IsResourceReadDisabled() const
{
  MM_CriticalSection_Enter(m_pSegmentDataLock);
  bool bIsDisabled = m_bIsResourceReadDisabled;
  MM_CriticalSection_Leave(m_pSegmentDataLock);

  return m_bIsResourceReadDisabled;
}

/**
 * Re-enable sample reads on the resource in case it was
 * disabled for sample reads.
 * This called on ContinueDataDownloadUnit only. In case of
 * CNCL, it is not called as we want to leave the resource in
 * non-readable state. If it is re-enabled, then it is possible
 * for sample reads to access this resource as it become the
 * active resource to read from. So, sample reads will get
 * blocked waiting for more data on the cancelled fragment,
 * whereas QSM will switch to a differet representation.
 */
void DASHMediaSegmentHandler::ReEnableResourceReads()
{
  MM_CriticalSection_Enter(m_pSegmentDataLock);
  m_bIsResourceReadDisabled = false;
  MM_CriticalSection_Leave(m_pSegmentDataLock);
}

/**
 * Disable socket reads on the data unit depending on
 * 'bIsDisabled'.
 */
bool DASHMediaSegmentHandler::DisableSocketReads(const uint64 nDataUnitKey,
                                                 bool bIsDisabled)
{
  bool rslt = false;
  SegmentDownloader *pDownloader = m_cSegDownloadMgr.GetSegmentDownloader(nDataUnitKey);

  if (pDownloader)
  {
    pDownloader->DisableSocketReads(bIsDisabled);
    rslt = true;
  }

  return rslt;
}

/**
 * Check to see if data segment with key 'nKey' is present in
 * this resource.
 */
bool DASHMediaSegmentHandler::IsDataUnit(uint64 nKey) const
{
  return (nKey < HTTP_NONSELF_INIT_SEG_KEY ? true : false);
}

int DASHMediaSegmentHandler::GetBufferedDurationFromNotifier()
{
  int duration = -1;

  if (m_pRepNotifier)
  {
    duration = m_pRepNotifier->GetBufferedDurationFromNotifier();
  }

  return duration;
}

/*
 * Close resource for given media type
 * If close is issued on all media types, state is changed to setup.
 * @param[in] eMediaType - media type
 *
 * @return HTTPDL_SUCCESS - moved to setup
 *         HTTPDL_WAITING - waiting for close to be called on all
 *                          media types.
 *         HTTPDL_ERROR_ABORT - failure
 */
HTTPDownloadStatus DASHMediaSegmentHandler::SegmentBaseStateHandler::Close(HTTPMediaType majorType)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  uint64 nSegMPDKey = m_pSegmentHandler->m_cSegmentInfo.getKey();
  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Seg [0x%08x%08x]: Close for majorType %d - ",
                 (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, (int)majorType );
  //Set close flag for major type on which close is
  //called. Once close is complete for both media types(i.e close
  //flag is set for all media types), filesource will be closed by
  // HTTPResource and track map is cleared
  m_pSegmentHandler->SetTrackClosed(majorType);
  if(m_pSegmentHandler->CloseComplete())
  {
    //Dequeue only check_data_avail_for_seek cmd as part of close(majortype)
    //So that segment handler can stop seeking
    void* pIt = NULL;
    SegmentCmdData cmd;
    while(m_pSegmentHandler->m_cCmdQ.Next(pIt, cmd))
    {
      if (cmd.sBaseCmdData.eCmd == SEGMENT_CMD_CHECK_DATA_AVAIL_FOR_SEEK)
      {
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "Seg [0x%08x%08x]: dequeing check data avail for seek cmd",
                        (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey);
        m_pSegmentHandler->m_cCmdQ.Remove(pIt);
        m_pSegmentHandler->m_nSeekTime = -1;
        break;
      }
     }

    (void)m_pSegmentHandler->SetStateHandler(&m_pSegmentHandler->m_cSetupStateHandler);
     status = HTTPCommon::HTTPDL_SUCCESS;
  }

  return status;
};

/** @brief Process error notification.
  *
  * @param[in] eStatus - Error status
  */
void DASHMediaSegmentHandler::OnError
(
 const HTTPDownloadStatus eStatus, uint32 nCmdID
)
{
  //Move to ERROR state only for fatal segment errors. The only way out of this is
  //to close the segment! Should not be used for data download errors since might
  //want to play out partially downloaded data before moving on to next segment.
  uint64 nSegMPDKey = m_cSegmentInfo.getKey();
  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Seg [0x%08x%08x]: Moving to ERROR %d and notifying representation",
                 (uint32)(nSegMPDKey >> 32), (uint32)nSegMPDKey, eStatus );
  SetStateHandler(&m_cErrorStateHandler);

  ResetDataDownloadState();

  ClearCmdQ(eStatus, nCmdID);

  //Set EOF so that reads (if already started on this resource) will break once
  //entire resource is read out!
  MarkSegmentComplete();

  if (m_pRepNotifier)
  {
    m_pRepNotifier->NotifyError((uint32)GetKey(), eStatus);
  }
}

/**
 * @brief Send Initial GetInfo request
 *
 * @param SegmentDownlaoder object through which Request to be
 *                          sent
 *
 * @return True if success otherwise false
 */
bool DASHMediaSegmentHandler::SendSegmentInfoRequest(SegmentDownloader* pDownloader)
{
  bool eResult = false;

  if (pDownloader)
  {
     //Compute the url byte range offsets for the data unit (discount init seg)
    int64 nStartOffset = 0;
    int64 nEndOffset = m_cSegmentInfo.IsIndexURLPresent() ? (int64)-1 : DEFAULT_SEG_INFO_GET_REQUEST_LENGTH;

    //If we have index url range specified use it else we have to
    //construct index url range using media range specified
    if (m_cSegmentInfo.GetIndexUrlRange())
    {
      char *indexRange = m_cSegmentInfo.GetIndexUrlRange();
      parseByteRange(indexRange, nStartOffset, nEndOffset);

      // Initialization segment could be present before sidx, hence try to download
      // init segment as a part of sidx download only.
      // If no initialization element present consider this as self init
      // segment and download sidx from the start of initialization segment
      if (NULL == m_pInitSegment)
      {
        nStartOffset = 0;
      }
    }
    /* else we have to construct index url range using media range specified.*/
    else if (m_cSegmentInfo.GetUrlRange())
    {
      char *urlRange = m_cSegmentInfo.GetUrlRange();
      parseByteRange(urlRange, nStartOffset, nEndOffset);
      nEndOffset = nStartOffset + DEFAULT_SEG_INFO_GET_REQUEST_LENGTH;
    }

    //Based on if server honors byte range header as part of original URL or
    //uses a byterangeURL since it dishonors byte range header (for http1.0),
    //construct segmentUrl and alternatesegmentUrl i.e. byterangeURL
    if (StartDownload(pDownloader,
                      HTTP_INIT_SEG_KEY,
                      nStartOffset, nEndOffset,
                      m_cSegmentInfo.GetIndexURL()))
    {
      eResult = true;
    }
  }

  return eResult;
}

/**
 * Asociate each cmd with a unique id (assuming that there
 * cannot be MAX_UINT32 commands active)
 */
bool DASHMediaSegmentHandler::EnQCmd(SegmentCmdData& rCmd)
{
  rCmd.sBaseCmdData.nCmdId = m_NextCmdID;
  ++m_NextCmdID;
  return m_cCmdQ.EnQ(rCmd);
}

} // namespace video
