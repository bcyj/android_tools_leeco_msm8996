/************************************************************************* */
/**
 * DASHMediaRepresentationHandler.cpp
 * @brief Implements the DASHMediaRepresentationHandler. Each such object
 *        is DASH resource manager that handles a DASH representation and
 *        manages all the media segments (resources) within a representation
 *        for one period.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/DASHMediaRepresentationHandler.cpp#81 $
$DateTime: 2014/02/06 23:45:40 $
$Change: 5239579 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "DASHMediaRepresentationHandler.h"

namespace video {

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define HTTP_MAX_REPRESENTATION_COMMANDS          10

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
/** @brief DASHMediaRepresentationHandler constructor.
  *
  * @param[out] bResult - Status of initialization
  * @param[in] nKey - Representation key
  * @param[in] sDASHSessionInfo - Reference to DASH session info
  * @param[in] cRepInfo - Reference to representation info
  * @param[in] pNotifier - Reference to group notifier
  * @param[in] pScheduler - Reference to task scheduler
  */
DASHMediaRepresentationHandler::DASHMediaRepresentationHandler
(
 bool& bResult,
 const uint32 nKey,
 DASHSessionInfo& sDASHSessionInfo,
 RepresentationInfo& cRepInfo,
 iGroupNotifier* pNotifier,
 Scheduler* pScheduler,
 HTTPStackInterface* pHTTPStack,
 InitializationSegment* cmnInit
) : m_nKey(nKey),
    m_sDASHSessionInfo(sDASHSessionInfo),
    m_cRepInfo(cRepInfo),
    m_bEstimator(m_cRepInfo.m_bEstimator),
    m_pGroupNotifier(pNotifier),
    m_pScheduler(pScheduler),
    m_pHTTPStack(pHTTPStack),
    m_nTaskID(0),
    m_pRepDataLock(NULL),
    m_pInitSegment(NULL),
    m_cIdleStateHandler(this),
    m_cSetupStateHandler(this),
    m_cReadyStateHandler(this),
    m_cErrorStateHandler(this),
    m_pCurrentStateHandler(NULL),
    m_bSeekPending(false),
    m_bIsLmsgSet(false)
{
  //Start with IDLE state
  SetStateHandler(&m_cIdleStateHandler);

  //Create data lock
  bResult = (MM_CriticalSection_Create(&m_pRepDataLock) == 0);

  //Initialize cmd queue
  if (bResult)
  {
    bResult = m_cCmdQ.Init(HTTP_MAX_REPRESENTATION_COMMANDS);
  }

  //Create context - use HTTP thread if pScheduler is passed in
  if (bResult)
  {
    if (m_pScheduler)
    {
      //Add TaskMediaRepresentation to scheduler
      bResult = false;
      SchedulerTask pTask = TaskMediaRepresentation;
      RepresentationCmdTaskParam* pTaskParam = QTV_New_Args(RepresentationCmdTaskParam,
                                                            ((void*)this));
      if (pTaskParam)
      {
        m_nTaskID = m_pScheduler->AddTask(pTask, (void*)pTaskParam);
        if (!m_nTaskID)
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Rep [0x%06x]: TaskMediaRepresentation could not be added to scheduler",
                         (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40)) ;
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
      //ToDo: Create thread with TaskMediaRepresentation as the entry point
      bResult = false;
    }
  }

  //Initialize init segment info
  if (bResult)
  {
    int64 nStartOffset = 0;
    int64 nEndOffset = -1;
    char* pURL = m_cRepInfo.GetInitialisationSegmentUrl();
    (void)m_cRepInfo.GetRangeInitialisationSegment(nStartOffset, nEndOffset);

    // Check if common init can be used
    if((pURL && cmnInit && cmnInit->m_pURL && !std_strcmp(cmnInit->m_pURL, pURL))
       || (!pURL && cmnInit))
    {
      if(InitializationSegment::INITSEG_STATE_ERROR != cmnInit->getInitState())
      {
        //m_pCommonInitSegment = cmnInit;
        m_pInitSegment = cmnInit;
      }
      else
      {
        bResult = false;
      }

      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "Rep [0x%06x]: Common Init state %d",  (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40), cmnInit->getInitState());
    }
    else if (pURL || (m_cRepInfo.GetInitialisationSegmentRange() && m_cRepInfo.GetBaseURL()))
    {
      // if init segment URL is not present, init segment is downloaded based on start/edn offset
      // using base URL
      if(NULL == pURL)
      {
        pURL = m_cRepInfo.GetBaseURL();
      }

      m_pInitSegment = QTV_New_Args(InitializationSegment,
                                    (pURL, nStartOffset, nEndOffset,
                                     m_sDASHSessionInfo, m_pHTTPStack, m_bEstimator,
                                     InitializationSegment::INITSEG_STATE_IDLE, bResult));
    }
  }

  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Rep [0x%06x]: Created DASH representation handler (resource manager) 0x%p result %d",
                 (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40), (void *)this, bResult );
}

/** @brief DASHMediaRepresentationHandler destructor.
  *
  */
DASHMediaRepresentationHandler::~DASHMediaRepresentationHandler()
{
  ClearCmdQ();

  if(m_pInitSegment && m_pInitSegment->isCommonInit())
  {
    m_pInitSegment = NULL;
  }
  else if (m_pInitSegment)
  {
    QTV_Delete(m_pInitSegment);
    m_pInitSegment = NULL;
  }
  if (m_pScheduler)
  {
    (void)m_pScheduler->DeleteTask(m_nTaskID);
  }
  if (m_pRepDataLock)
  {
    (void)MM_CriticalSection_Release(m_pRepDataLock);
    m_pRepDataLock = NULL;
  }

  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Rep [0x%06x]: Destroyed DASH representation handler (resource manager) 0x%p",
                 (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40), (void *)this );
}

void DASHMediaRepresentationHandler::ClearBufferedData(HTTPCommon::HTTPMediaType majorType,
                                                       uint64 nStartTime)
{
  HTTPResourceManager::ClearBufferedData(majorType, nStartTime + m_cRepInfo.GetPTSOffset());
}

/**
 * Get the base time for the representation taking into account
 * pts offset.
 */
bool DASHMediaRepresentationHandler::GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime)
{
  bool rslt = HTTPResourceManager::GetBaseTime(segMediaTime, segMPDTime);
  if (rslt)
  {
    segMediaTime = (segMediaTime >= m_cRepInfo.GetPTSOffset()
                ? segMediaTime - m_cRepInfo.GetPTSOffset()
                : 0);
  }

  return rslt;
}

/**
 * Flsuh taking into account pts offset.
 */
HTTPDownloadStatus DASHMediaRepresentationHandler::Flush(HTTPMediaType majorType,int64 nStartTime)
{
  return HTTPResourceManager::Flush(majorType,nStartTime + m_cRepInfo.GetPTSOffset());
}

/**
 * Get the next media sample with adjusted for non-zero pts
 * offset.
 */
HTTPCommon::HTTPDownloadStatus DASHMediaRepresentationHandler::GetNextMediaSample(
    HTTPCommon::HTTPMediaType majorType, uint8 *pBuffer, uint32 &nSize,HTTPSampleInfo &sampleInfo)
{
  HTTPCommon::HTTPDownloadStatus rslt = HTTPResourceManager::GetNextMediaSample(
    majorType,pBuffer,nSize,sampleInfo);

  sampleInfo.startTime = (sampleInfo.startTime >= m_cRepInfo.GetPTSOffset()
                          ? sampleInfo.startTime - m_cRepInfo.GetPTSOffset()
                          : 0);
  sampleInfo.endTime = (sampleInfo.endTime >= m_cRepInfo.GetPTSOffset()
                          ? sampleInfo.endTime - m_cRepInfo.GetPTSOffset()
                          : 0);

  return rslt;
}

/**
 * Get the download position adjusted for pts offset
 */
bool DASHMediaRepresentationHandler::GetDownloadPosition(
  HTTPCommon::HTTPMediaType majorType,
  uint64& nDownloadPosition)
{
  bool rslt = HTTPResourceManager::GetDownloadPosition(majorType,nDownloadPosition);

  uint64 ptsOffset = m_cRepInfo.GetPTSOffset();
  nDownloadPosition = (nDownloadPosition >= ptsOffset
                       ? nDownloadPosition - ptsOffset
                       : 0);

  return rslt;
}

bool DASHMediaRepresentationHandler::IsLastSegDownloadSucceed()
{
  //If resource (seghandler) not already created do so now
  HTTPResource* pSegHandler = NULL;

  uint64 nLastSegKey = MAX_UINT64;
  if(HTTPCommon::HTTPDL_SUCCESS ==
     m_sDASHSessionInfo.cMPDParser.GetLastSegmentKeyForRepresentation(&nLastSegKey, m_cRepInfo.getKey()))
  {
    nLastSegKey = nLastSegKey & MPD_SEGMENT_MASK;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Representation isLastSegDwld nLastSegKey %llu", nLastSegKey);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Representation isLastSegDwld nLastSegKey Failed");
  }

  //Check if there is any download failure happen on the last segment.
  //if the resource corresponds to last segment is not present
  //Some Error would have happened can be assumed for the segment
  bool rslt = false;
  GetResource(nLastSegKey, &pSegHandler);
  if(pSegHandler)
  {
    rslt = pSegHandler->IsSegErrorHappened();
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Representation isLastSegDwld failed %d", rslt );
  }
  else
  {
    rslt = true;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Representation isLastSegDwld failed %d without last resource", rslt );
  }

  return rslt;
}


bool DASHMediaRepresentationHandler::IsLmsgSet()
{
  return m_bIsLmsgSet;
}

/** @brief Media representation task
  *
  * @param[in] pParam - Task parameter
  * @return
  * 0 - SUCCESS
  * -1 - FAILURE
  */
int DASHMediaRepresentationHandler::TaskMediaRepresentation
(
 void* pParam
)
{
  int status = -1;
  RepresentationCmdTaskParam* pTaskParam = (RepresentationCmdTaskParam*) pParam;

  if (pTaskParam == NULL || pTaskParam->pSelf == NULL)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid task param 0x%p", (void *)pTaskParam );
  }
  else
  {
    //Process commands in the current state
    DASHMediaRepresentationHandler* pSelf = (DASHMediaRepresentationHandler*)pTaskParam->pSelf;
    RepresentationBaseStateHandler* pCurrStateHandler = pSelf->GetStateHandler();
    if (pCurrStateHandler)
    {
      pCurrStateHandler->ProcessCmds();
      status = 0;
    }
  }

  return status;
}

HTTPDownloadStatus DASHMediaRepresentationHandler::Open(uint64 nDataUnitKey,int64 nStartTime,bool bSeek)
{
  RepresentationBaseStateHandler* pCurrentStateHandler = GetStateHandler();
  return (pCurrentStateHandler ?
          pCurrentStateHandler->Open(nDataUnitKey,nStartTime,bSeek) :
          HTTPCommon::HTTPDL_ERROR_ABORT);

}
HTTPDownloadStatus DASHMediaRepresentationHandler::IsOpenCmdProcessingAllowed()
{
  //Resource manager will return success if reads are done on all media
  //types. This will ensure that open will be called on segmenthandler
  //only when segment handler is done reading all media types and closed.
  return HTTPResourceManager::Open();
}

HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationBaseStateHandler::Open
(
  uint64 /* nDataUnitKey */,
  int64 /* nStartTime */,
  bool /* bSeek */
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Invalid state %d for Open() call", m_eState );
  return HTTPCommon::HTTPDL_ERROR_ABORT;
}
/* @brief - This function closes rephandler for given media type.
 *          Once all the media types are closed, segment handler
 *          will close the filesource.
 * @param - majortype to be closed
 * @return - status - HTTPDL_SUCCESS if successful
             HTTPDL_WAITING - waiting for close on other media type
             HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHMediaRepresentationHandler::Close(HTTPMediaType majorType)
{
  if(!IsClosing())
  {
     void* pIt = NULL;
     RepresentationCmdData cmd;
     while(m_cCmdQ.Next(pIt, cmd))
     {
       if(cmd.sBaseCmdData.eCmd == REPRESENTATION_CMD_OPEN)
       {
         m_cCmdQ.Remove(pIt);
         QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Rep [0x%06x]: OPEN cmd removed in close "
                       "as resource manager is not in closing state",
                       (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40));
       }
       else if(cmd.sBaseCmdData.eCmd == REPRESENTATION_CMD_SEEK )
       {
          m_cCmdQ.Remove(pIt);
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Rep [0x%06x]: SEEK cmd removed in close "
                         "as resource manager is not in closing state",
                         (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40));
          m_bSeekPending = false;
       }
       else if(cmd.sBaseCmdData.eCmd == REPRESENTATION_CMD_NOTIFY_SEEK )
       {
          m_cCmdQ.Remove(pIt);
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Rep [0x%06x]: Notify SEEK cmd removed in close "
                         "as resource manager is not in closing state",
                         (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40));
       }
     }
  }
  return HTTPResourceManager::Close(majorType);
}

/** @brief Queue up GET_SEGINFO cmd from period (valid in all states).
  *
  * @param[in] nStartTime - Request start time
  * @param[in] nDuration - Request duration
  * @return
  * HTTPDL_WAITING - GET_SEGINFO cmd successfully queued and status notified later
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationBaseStateHandler::GetSegmentInfo
(
 uint64 nStartTime,
 uint64 nDuration
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  RepresentationCmdData cmd;
  cmd.sGetSegInfoCmdData.eCmd = REPRESENTATION_CMD_GET_SEGINFO;
  cmd.sGetSegInfoCmdData.nStartTime = nStartTime;
  cmd.sGetSegInfoCmdData.nDuration = nDuration;

  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Rep [0x%06x]: GET_SEGINFO cmd queued",
                   (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40) );

  if (!m_pRepHandler->m_cCmdQ.EnQ(cmd))
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Rep [0x%06x]: GET_SEGINFO cmd cannot be queued",
                   (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40) );
  }

  return status;
}

/** @brief Close representation (valid in ALL states).
  *
  * @return status of operation
  */
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationBaseStateHandler::Close()
{
  //Clear cmd Q
  m_pRepHandler->m_bIsLmsgSet = false;
  m_pRepHandler->ClearCmdQ();

  //Reset INIT segment (if applicable)
  if (m_pRepHandler->m_pInitSegment && !m_pRepHandler->m_pInitSegment->isCommonInit())
  {
    m_pRepHandler->m_pInitSegment->Reset();
  }
  m_pRepHandler->m_cSegInfoCmdHandler.Reset();

  //Move back to IDLE
  m_pRepHandler->SetStateHandler(&m_pRepHandler->m_cIdleStateHandler);

  return HTTPCommon::HTTPDL_SUCCESS;
}

/** @brief Process all commands (in IDLE state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaRepresentationHandler::RepresentationIdleStateHandler::ProcessCmds()
{
  int ret = 0;
  RepresentationCmdData cmd;

  //Wait for GET_SEGINFO cmd
  if (m_pRepHandler->m_cCmdQ.PeekHead(cmd))
  {
    if (cmd.sBaseCmdData.eCmd == REPRESENTATION_CMD_GET_SEGINFO)
    {
      //Transition to SETUP if INIT segment exists else move directly to READY
      if (m_pRepHandler->m_pInitSegment && !m_pRepHandler->m_pInitSegment->isCommonInit())
      {
        if (m_pRepHandler->m_pInitSegment->m_cDownloader.Start(m_pRepHandler->m_pInitSegment->m_pURL, NULL,
                                                               0,
                                                               m_pRepHandler->m_pInitSegment->m_nStartOffset,
                                                               m_pRepHandler->m_pInitSegment->m_nEndOffset))
        {
          m_pRepHandler->m_pInitSegment->m_cDataStore.SetStartOffset(0);
          m_pRepHandler->m_pInitSegment->m_cDataStore.SetPurgeFlag(false);
          m_pRepHandler->SetStateHandler(&m_pRepHandler->m_cSetupStateHandler);
        }
        else
        {
          (void)m_pRepHandler->OnError(HTTPCommon::HTTPDL_ERROR_ABORT);
        }
      }
      else if(m_pRepHandler->m_pInitSegment && m_pRepHandler->m_pInitSegment->isCommonInit())
      {
        InitializationSegment::InitSegState  intiState = m_pRepHandler->m_pInitSegment->getInitState();
        if(InitializationSegment::INITSEG_STATE_DOWNLOADING == intiState)
        {
          m_pRepHandler->SetStateHandler(&m_pRepHandler->m_cSetupStateHandler);
        }
        else if(InitializationSegment::INITSEG_STATE_DOWNLOADED == intiState)
        {
          m_pRepHandler->SetStateHandler(&m_pRepHandler->m_cReadyStateHandler);
        }
        else
        {
           QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "DASHMediaPlayGroup::Download(): cmn Init segment state %d ", intiState);
           (void)m_pRepHandler->OnError(HTTPCommon::HTTPDL_ERROR_ABORT);
        }
      }
      else
      {
        m_pRepHandler->SetStateHandler(&m_pRepHandler->m_cReadyStateHandler);
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
int DASHMediaRepresentationHandler::RepresentationSetupStateHandler::ProcessCmds()
{
  int ret = 0;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  uint32 nRepMPDKey = (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40);

  if (m_pRepHandler->m_pInitSegment && !m_pRepHandler->m_pInitSegment->isCommonInit())
  {
    SegmentDownloader& cDownloader = m_pRepHandler->m_pInitSegment->m_cDownloader;
    HttpSegmentDataStoreBase* pDataStore = &m_pRepHandler->m_pInitSegment->m_cDataStore;
    byte* pBuffer = NULL;
    int64 nBytesToRead = (int64)HTTP_MAX_DATA_CHUNK_LEN;

    //Get data store buffer, read data into it from downloader (stack) and commit buffer
    if (pDataStore->GetBuffer(pBuffer, nBytesToRead))
    {
      //ToDo: Data inactivity timeout?
      status = HTTPCommon::HTTPDL_SUCCESS;
      int64 nBytesRead = 0;
      /* Increment number of request and record start time if this is first request
         or first request after all active download was finished */
      HTTPCommon::HTTPDownloadStatus result = cDownloader.Read(pBuffer, nBytesToRead, nBytesRead);
      if (result == HTTPCommon::HTTPDL_SUCCESS || result == HTTPCommon::HTTPDL_DATA_END)
      {
        if (nBytesRead >= 0)
        {
          QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Rep [0x%06x]: Init segment download progress %llu/%lld bytes",
                         nRepMPDKey, cDownloader.GetNumBytesReceived(),
                         cDownloader.GetContentLength() );
          if (!pDataStore->CommitBuffer(pBuffer, nBytesRead))
          {
            status = HTTPCommon::HTTPDL_ERROR_ABORT;
          }
        }

        if (result == HTTPCommon::HTTPDL_DATA_END)
        {
          // sanity checks. These don't cover all use cases though.
          // example use case that is not covered is http response header does not contain
          // content length and server prematurely terminates the connection.
          if (0 == cDownloader.GetNumBytesReceived() ||
              (cDownloader.GetContentLength() > 0 &&
               (int64)cDownloader.GetNumBytesReceived() != cDownloader.GetContentLength()))
          {
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "DASHMediaRepresentationHandler sanity check against content-len %lld and num of downloaded bytes %llu failed",
               cDownloader.GetContentLength(), cDownloader.GetNumBytesReceived());

            status = HTTPCommon::HTTPDL_ERROR_ABORT;
          }
        }

        if (HTTPSUCCEEDED(status))
        {
          //Mark data store segment done if download complete and move to READY,
          //else come back later to read more data
          if (result == HTTPCommon::HTTPDL_DATA_END)
          {
            uint64 nEndOffset = cDownloader.GetStartOffset() + cDownloader.GetNumBytesReceived();
            pDataStore->SetSegmentComplete(nEndOffset);
            cDownloader.Stop();

            m_pRepHandler->SetStateHandler(&m_pRepHandler->m_cReadyStateHandler);
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "Rep [0x%06x]: Init segment download complete", nRepMPDKey );
          }
          else
          {
            status = HTTPCommon::HTTPDL_WAITING;
          }
        }
      }
      else if (result == HTTPCommon::HTTPDL_WAITING)
      {
        status = HTTPCommon::HTTPDL_WAITING;
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "Rep [0x%06x]: Init segment waiting for data", nRepMPDKey );
      }
      else if (result == HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND)
      {
          status = HTTPCommon::HTTPDL_INVALID_REPRESENTATION;
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "Invalid representation detected due to missing initialization segment");
      }
      else
      {
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }
    }
  }
  else if(m_pRepHandler->m_pInitSegment && m_pRepHandler->m_pInitSegment->isCommonInit())
  {
    InitializationSegment::InitSegState initState = m_pRepHandler->m_pInitSegment->getInitState();
    if(InitializationSegment::INITSEG_STATE_DOWNLOADING == initState)
    {
      status = HTTPCommon::HTTPDL_WAITING;
    }
    else if(InitializationSegment::INITSEG_STATE_DOWNLOADED == initState)
    {
      m_pRepHandler->SetStateHandler(&m_pRepHandler->m_cReadyStateHandler);
      status = HTTPCommon::HTTPDL_SUCCESS;
    }
    else
    {
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
    }

    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "Rep [0x%06x]: DASHMediaRepresentationHandler::SetupStateHandler Cmn Init seg state %d", nRepMPDKey, initState);
  }

  //Clean up for failure!
  if (status == HTTPCommon::HTTPDL_ERROR_ABORT ||
      HTTPCommon::HTTPDL_INVALID_REPRESENTATION == status)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Rep [0x%06x]: Init segment download failed, closing", nRepMPDKey );
    (void)m_pRepHandler->OnError(status);
  }

  return ret;
}

/** @brief Process all commands (in READY state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaRepresentationHandler::RepresentationReadyStateHandler::ProcessCmds()
{
  int ret = 0;
  RepresentationCmdData* cmd = NULL;
   void* pIt = NULL;
  bool bGetSegInfoCmdProcessed = false;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  while(m_pRepHandler->m_cCmdQ.Next(pIt, &cmd))
  {
    //bGetSegInfoCmdProcessed is to make sure we process only the first GET_SEGINFO cmd
    if (cmd->sBaseCmdData.eCmd == REPRESENTATION_CMD_GET_SEGINFO
        && bGetSegInfoCmdProcessed == false)
    {
      bGetSegInfoCmdProcessed = true;
      HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
      uint32 nNumDataUnits = 0;

      //Get the cmd handler (for now processing only one cmd at a time, if this changes might
      //need a SegmentInfoCmdHandlerManager!)
      SegmentInfoCmdHandler& cSegInfoCmdHandler = m_pRepHandler->m_cSegInfoCmdHandler;
      SegmentInfoCmdHandlerState eState = cSegInfoCmdHandler.m_eState;

      //Request segment info on the first applicable media segment (based on requested start
      //time and duration). Notify success as soon as info is available for partial or
      //complete range, fail the request only if info can't be fetched for any part of the
      //requested range, keep waiting if no part of the range is accessible yet (e.g. live)
      if (eState == SEGINFO_CMDHANDLER_IDLE)
      {
        HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
        uint64 nSegMPDKey = MAX_UINT64_VAL;
        double nFirstAvailSegStart = 0;
        SegmentInfo cSegInfo;
        uint64 nEndTime = cmd->sGetSegInfoCmdData.nStartTime + cmd->sGetSegInfoCmdData.nDuration;
        uint64 nStartTime = (cSegInfoCmdHandler.m_nNextStartTime < 0) ?
                             cmd->sGetSegInfoCmdData.nStartTime : cSegInfoCmdHandler.m_nNextStartTime;
        cSegInfoCmdHandler.m_nCmdStartTime = cmd->sGetSegInfoCmdData.nStartTime;
        int64 nDuration = (nEndTime > nStartTime) ? (nEndTime - nStartTime) : 0;

        //Loop through all media segments and request info on the right segment
        MM_CriticalSection_Enter(pResourcesLock);

        while (nDuration > 0)
        {
          uint32 nNumSegments = 1;
          status =
            m_pRepHandler->m_sDASHSessionInfo.cMPDParser.GetAllSegmentsForRepresentationRange(&nSegMPDKey,
                                                                                              nNumSegments,
                                                                                              m_pRepHandler->m_cRepInfo.getKey(),
                                                                                              nStartTime,
                                                                                              nEndTime,
                                                                                              nFirstAvailSegStart);

          if (status == HTTPCommon::HTTPDL_WAITING)
          {
            //If mpdparser returns waiting check if there is an already created resource
            //with required starttime and duration. If yes return the info from that
            //segment else keep the same status which mpdparser returns
            HTTPResource* pSegHandler = NULL;
            status = m_pRepHandler->GetResource(nStartTime + m_pRepHandler->m_cRepInfo.GetPTSOffset(),
                                                nDuration, pSegHandler);
            if (pSegHandler)
            {
              uint32 nSegKey = (uint32)(pSegHandler->GetKey() & MPD_SEGMENT_MASK);
              uint64 nCompleteKey = (m_pRepHandler->m_cRepInfo.getKey() | pSegHandler->GetKey());

              if (!m_pRepHandler->m_sDASHSessionInfo.cMPDParser.GetSegmentInfoByKey(nCompleteKey, cSegInfo))
              {
                result = HTTPCommon::HTTPDL_ERROR_ABORT;
                QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                              "Failed to retrieve segment for key with period %u",
                              (uint32)(pSegHandler->GetKey() >> 56));
                break;
              }
              status = pSegHandler->GetSegmentInfo(nStartTime + m_pRepHandler->m_cRepInfo.GetPTSOffset(),
                                                   nDuration);

              QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                             "Rep [0x%06x]: Segment info requested %d on seg %u - "
                             "nStartTime %u msec nDuration %u msec PTO %llu msec",
                             (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                             status, nSegKey, (uint32)nStartTime, (uint32)nDuration,
                             (uint64)m_pRepHandler->m_cRepInfo.GetPTSOffset() );

              //Compute start time and duration for the next request
              nStartTime = (uint64)(cSegInfo.getStartTime() + cSegInfo.getDuration());
              nDuration = nEndTime - nStartTime;
              if (status == HTTPCommon::HTTPDL_SUCCESS || status == HTTPCommon::HTTPDL_WAITING)
              {
                if (eState == SEGINFO_CMDHANDLER_IDLE)
                {
                  eState = (status == HTTPCommon::HTTPDL_SUCCESS) ? SEGINFO_CMDHANDLER_AVAILABLE
                                                                  : SEGINFO_CMDHANDLER_WAITING;
                  cSegInfoCmdHandler.m_nNextStartTime = nStartTime;
                  cSegInfoCmdHandler.m_nStartIdx = (int)nSegKey;
                  cSegInfoCmdHandler.m_eState = eState;
                }
                break;
              }
              else
              {
                //Move on to next seg on failure!
                result = HTTPCommon::HTTPDL_ERROR_ABORT;
                QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                               "Rep [0x%06x]: Error in getting seg info on seg %u, "
                               "moving on start time %u duration %u",
                               (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                               nSegKey,(uint32)nStartTime,(uint32)nDuration );
              }
            }
            else
            {
              //Requesting way too early, try later (e.g. availabilityStartTime > current time)
              result = HTTPCommon::HTTPDL_WAITING;
              break;
            }
          }
          else if (status == HTTPCommon::HTTPDL_SUCCESS || status == HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER)
          {
            if (!m_pRepHandler->m_sDASHSessionInfo.cMPDParser.GetSegmentInfoByKey(nSegMPDKey, cSegInfo))
            {
              result = HTTPCommon::HTTPDL_ERROR_ABORT;
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "Failed to retrieve segment");
              break;
            }
            else
            {
              //If resource (seghandler) not already created do so now
              HTTPResource* pSegHandler = NULL;
              uint32 nSegKey = (uint32)(nSegMPDKey & MPD_SEGMENT_MASK);
              (void)m_pRepHandler->GetResource(nSegKey, &pSegHandler);
              if (pSegHandler == NULL)
              {
                (void)m_pRepHandler->CreateSegmentHandler(nSegKey, cSegInfo, &pSegHandler);
              }

              if (pSegHandler)
              {
                status = pSegHandler->GetSegmentInfo(nStartTime + m_pRepHandler->m_cRepInfo.GetPTSOffset(),
                                                     nDuration);
                QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                               "Rep [0x%06x]: Segment info requested %d on seg %u - "
                               "nStartTime %u msec nDuration %u msec PTO %llu msec",
                               (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                               status, nSegKey, (uint32)nStartTime, (uint32)nDuration,
                               (uint64)m_pRepHandler->m_cRepInfo.GetPTSOffset() );

                //Compute start time and duration for the next request
                nStartTime = (uint64)(cSegInfo.getStartTime() + cSegInfo.getDuration());
                nDuration = nEndTime - nStartTime;
                if (status == HTTPCommon::HTTPDL_SUCCESS || status == HTTPCommon::HTTPDL_WAITING)
                {
                  if (eState == SEGINFO_CMDHANDLER_IDLE)
                  {
                    eState = (status == HTTPCommon::HTTPDL_SUCCESS) ? SEGINFO_CMDHANDLER_AVAILABLE
                                                                    : SEGINFO_CMDHANDLER_WAITING;
                    cSegInfoCmdHandler.m_nNextStartTime = nStartTime;
                    cSegInfoCmdHandler.m_nStartIdx = (int)nSegKey;
                    cSegInfoCmdHandler.m_eState = eState;
                  }

                  //Segment info available/requested
                  break;
                }
                else
                {
                  //Move on to next seg on failure!
                  result = HTTPCommon::HTTPDL_ERROR_ABORT;
                }
              }
              else
              {
                //No free resource available, QSM should hopefully try another rep (if available)
                //after certain #failures!
                result = HTTPCommon::HTTPDL_ERROR_ABORT;
                break;
              }
            }
          }
          else if (status == HTTPCommon::HTTPDL_TIMEOUT)
          {
            uint64 nStartTime = cmd->sGetSegInfoCmdData.nStartTime;
            uint64 nStartTimeWithPTSOffset =
              nStartTime + m_pRepHandler->m_cRepInfo.GetPTSOffset();
            uint64 nDuration = cmd->sGetSegInfoCmdData.nDuration;

            HTTPResource* pSegHandler = NULL;

            //Find the resource (if exists) for nStartTime and mark it complete
            if (m_pRepHandler->GetResource(nStartTimeWithPTSOffset,
                                           nDuration,
                                           pSegHandler) == HTTPCommon::HTTPDL_SUCCESS)
            {
              if (pSegHandler)
              {
                pSegHandler->MarkSegmentComplete();
              }
              else
              {
                QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Unexpected error: pSegHandler is null");
              }
            }
            else
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                            "Unexpected error: Failed to get resource for nStartTime %d",
                            (int)nStartTime);
            }

            //JUMP usecase where available segment is ahead of requested segment (e.g. live pause).
            //Notify QSM of the new time to save self-learning, and QSM moves its time window based
            //on this notification
            cmd->sGetSegInfoCmdData.nDuration = (uint64)nFirstAvailSegStart - nStartTime;
            QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "JUMP case starttime %d/%d and duration %d/%d (modified/original)",
                          (int)nFirstAvailSegStart, (int)nStartTime,
                          (int)cmd->sGetSegInfoCmdData.nDuration, (int)nDuration);
            result = HTTPCommon::HTTPDL_TIMEOUT;
            break;
          }
          else if (status == HTTPCommon::HTTPDL_DATA_END)
          {
            //Notify QSM of the end so that there are no more data unit info requests
            result = HTTPCommon::HTTPDL_DATA_END;
            break;
          }
          else
          {
            result = HTTPCommon::HTTPDL_ERROR_ABORT;
            break;
          }
        }

        MM_CriticalSection_Leave(pResourcesLock);
      }

      //Wait till seg info becomes available on the right segment
      if (eState == SEGINFO_CMDHANDLER_AVAILABLE)
      {
        result = HTTPCommon::HTTPDL_SUCCESS;
        (void)FillDataUnitInfo(cmd->sGetSegInfoCmdData.nStartTime + m_pRepHandler->m_cRepInfo.GetPTSOffset(),
                               cmd->sGetSegInfoCmdData.nDuration,
                               NULL, 0, nNumDataUnits);

        //Loop back to IDLE and start again if could not accumulate info on the current segment!
        if (nNumDataUnits == 0)
        {
          if (cSegInfoCmdHandler.m_bIsSegmentNotFound)
          {
            cmd->sGetSegInfoCmdData.nDuration = cSegInfoCmdHandler.m_nNextStartTime - cmd->sGetSegInfoCmdData.nStartTime;
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "'404' for segment with starttime %llu, duration %llu",
              cmd->sGetSegInfoCmdData.nStartTime, cmd->sGetSegInfoCmdData.nDuration);
            result = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
          }
          else
          {
            uint64 nStartTime = cSegInfoCmdHandler.m_nNextStartTime;
            cSegInfoCmdHandler.Reset();
            cSegInfoCmdHandler.m_bMoveToPrevSegment = false;
            cSegInfoCmdHandler.m_nNextStartTime = nStartTime;
            result = HTTPCommon::HTTPDL_WAITING;
          }
        }
      }
      else if (eState == SEGINFO_CMDHANDLER_WAITING)
      {
        result = HTTPCommon::HTTPDL_WAITING;
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "Rep [0x%06x]: Waiting for segment info from seg %d",
                       (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                       cSegInfoCmdHandler.m_nStartIdx );
      }

      if (result != HTTPCommon::HTTPDL_WAITING)
      {
        //Notify status and dequeue GET_SEGINFO cmd
        if (m_pRepHandler->m_pGroupNotifier)
        {
          HTTPDownloadStatus statusSegInfoReady = result;
          if (HTTPCommon::HTTPDL_SUCCESS == statusSegInfoReady)
          {
            uint64 nStartTime = cmd->sGetSegInfoCmdData.nStartTime;
            uint64 nStartTimeWithPTSOffset =
              nStartTime + m_pRepHandler->m_cRepInfo.GetPTSOffset();

            HTTPResource* pSegHandler = NULL;
            MM_CriticalSection_Enter(pResourcesLock);
            if (m_pRepHandler->GetResource(nStartTimeWithPTSOffset,
                                           cmd->sGetSegInfoCmdData.nDuration,
                                           pSegHandler) == HTTPCommon::HTTPDL_SUCCESS)
            {
              if (pSegHandler)
              {
                uint64 nLastStartTime = 0;

                double nRepEndTime =
                  m_pRepHandler->m_sDASHSessionInfo.cMPDParser.GetDuration(m_pRepHandler->m_cRepInfo.getKey());

                if (nRepEndTime > 0.0)
                {
                  nRepEndTime += (double)m_pRepHandler->m_cRepInfo.GetPTSOffset();

                  if (pSegHandler->GetStartTimeForLastDataUnit(nLastStartTime, nRepEndTime, m_pRepHandler->m_bIsLmsgSet))
                  {
                    //Signal DATA_END only if the QSM's request range contains the last data unit
                    //for the representation (period)
                    if ((nStartTimeWithPTSOffset + cmd->sGetSegInfoCmdData.nDuration) > nLastStartTime)
                    {
                      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                     "Rep [0x%06x]: Signaling DATA_END - nLastStartTime %d",
                                     (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                                     (int)nLastStartTime );
                      statusSegInfoReady = HTTPCommon::HTTPDL_DATA_END;
                    }
                  }
                }
              }
              else
              {
                QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unexpected error: pSegHandler is null");
              }
            }
            else
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                            "Unexpected error: Failed to get resource for startTime %d",
                            (int)nStartTime);
            }

            MM_CriticalSection_Leave(pResourcesLock);
          }

          QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                         "Rep [0x%06x]: GET_SEGINFO cmd (nStartTime %u msec nDuration %u msec "
                         "nNumDataUnits %u processing complete %d - notifying group",
                         (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                         (uint32)cmd->sGetSegInfoCmdData.nStartTime,
                         (uint32)cmd->sGetSegInfoCmdData.nDuration, nNumDataUnits, result );

          if (HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND == statusSegInfoReady)
          {
            if (m_pRepHandler->m_sDASHSessionInfo.cMPDParser.IsRepVODProfile(
               m_pRepHandler->m_cRepInfo.getKey()))
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "Representation is VOD profile");
              statusSegInfoReady = HTTPCommon::HTTPDL_INVALID_REPRESENTATION;
            }
          }

          m_pRepHandler->m_pGroupNotifier->SegInfoReady(m_pRepHandler->m_nKey,
                                                        cmd->sGetSegInfoCmdData.nStartTime,
                                                        cmd->sGetSegInfoCmdData.nDuration,
                                                        nNumDataUnits, statusSegInfoReady);

          // remove any resources in error state that are sitting around eg,
          // due to filesource open failure.
          m_pRepHandler->RemoveResourcesInErrorState();
        }
        cSegInfoCmdHandler.Reset();
        (void)m_pRepHandler->m_cCmdQ.Remove(pIt);
      }
    }
    else if(cmd->sBaseCmdData.eCmd == REPRESENTATION_CMD_OPEN)
    {
      HTTPDownloadStatus openCmdStatus = m_pRepHandler->IsOpenCmdProcessingAllowed();
      if(HTTPSUCCEEDED(openCmdStatus))
      {
        if(openCmdStatus == HTTPCommon::HTTPDL_SUCCESS)
        {
          m_pRepHandler->Flush(HTTPCommon::HTTP_UNKNOWN_TYPE,
                               cmd->sOpenCmdData.nStartTime);

          MM_CriticalSection_Enter(pResourcesLock);

          HTTPResource* pSegHandler = m_pRepHandler->GetSegmentHandler(
                                      cmd->sOpenCmdData.nDataUnitKey);
          if (pSegHandler)
          {
            //The least significant 32 bits indicates the data unit key
            (void)pSegHandler->Open(cmd->sOpenCmdData.nDataUnitKey & 0x00000000FFFFFFFF,
                                       cmd->sOpenCmdData.nStartTime + m_pRepHandler->m_cRepInfo.GetPTSOffset(),
                                       cmd->sOpenCmdData.bSeek);
          }

          MM_CriticalSection_Leave(pResourcesLock);
        }
        else
        {
          //If the open cmd is already processed by representation handler
          //and resource manager is in opening state then there is no need
           //to process open cmd it can directly be dequeued.
        }
        m_pRepHandler->m_cCmdQ.Remove(pIt);

      }
    }
    else if(cmd->sBaseCmdData.eCmd == REPRESENTATION_CMD_SEEK &&
             !m_pRepHandler->m_bSeekPending)
    {
      HTTPResource* pSegHandler = NULL;
      HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;

      MM_CriticalSection_Enter(pResourcesLock);
      if(cmd->sSeekCmdData.nSeekTime == -1)
      {
        //If seek time passed is -1 then call seek on first valid resource
        //with seek time as segment start time.
        eReturn = m_pRepHandler->GetFirstResource(&pSegHandler);
      }
      else
      {
        eReturn = m_pRepHandler->GetResource(
          cmd->sSeekCmdData.nSeekTime + m_pRepHandler->m_cRepInfo.GetPTSOffset(),
          pSegHandler);
      }

      bool bNotifySeek = false;
      int64 nNotifySeekEndTime = 0;
      int64 nNotifySeekCurTime = 0;
      HTTPDownloadStatus eNotifySeekStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

      uint64 nStartTime=0,nDuration =0;
      if(eReturn == HTTPCommon::HTTPDL_DATA_END)
      {
        //If there are no more resources available notify seek as success
        //with seek end time as period end time
        if ( m_pRepHandler->m_pGroupNotifier)
        {
          bNotifySeek = true;
          nNotifySeekEndTime = USE_PERIOD_END_TIME;
          nNotifySeekCurTime = USE_PERIOD_END_TIME;
          eNotifySeekStatus = HTTPCommon::HTTPDL_DATA_END;

          m_pRepHandler->m_cCmdQ.Remove(pIt);
        }
      }
      else
      {
        uint64 segStartTime = m_pRepHandler->m_cRepInfo.GetPTSOffset();
        if(pSegHandler && pSegHandler->GetSegmentRange(segStartTime, nDuration))
        {
          if(cmd->sSeekCmdData.nSeekTime == -1)
          {
            //If seek time is passed as -1, call seek with seektime as segment start time
            cmd->sSeekCmdData.nSeekTime =  segStartTime;
          }
          else
          {
            //Add pts offset to seek time as seektime is passed by upper layer
            cmd->sSeekCmdData.nSeekTime += m_pRepHandler->m_cRepInfo.GetPTSOffset();
          }
          if(pSegHandler->Seek(cmd->sSeekCmdData.nSeekTime) ==
             HTTPCommon::HTTPDL_ERROR_ABORT)
          {
            //If there is an error in calling seek on segmnet handler queue notify seek
            //command status as failure. This can be moved to segment handler, whenever
            //segment handler gets error while queueing or processing seek command
            //it can notify rephandler of error directly
            m_pRepHandler->NotifySeekStatus((uint32)pSegHandler->GetKey(),-1,
                                            HTTPCommon::HTTPDL_ERROR_ABORT);

          }
          else
          {
            m_pRepHandler->m_bSeekPending = true;
            //If seek is successfully called on segment handler. Call a open on
            //next segment handler in queue
            HTTPResource* pNextSegHandler = NULL;
            m_pRepHandler->GetResource((segStartTime + nDuration),&pNextSegHandler);
            if(pNextSegHandler)
            {
              (void)pNextSegHandler->OpenSegment();
            }
          }
        }
        else
        {
          //Wait for QSM to request segment info on the segment
        }
      }

      MM_CriticalSection_Leave(pResourcesLock);

      if (bNotifySeek)
      {
        m_pRepHandler->m_pGroupNotifier->NotifySeekStatus(m_pRepHandler->m_nKey,
                                                          nNotifySeekEndTime,
                                                          nNotifySeekCurTime,
                                                          eNotifySeekStatus);
      }
    }
    else if(cmd->sBaseCmdData.eCmd == REPRESENTATION_CMD_NOTIFY_SEEK)
    {
      HTTPDownloadStatus status = cmd->sNotifySeekCmdData.eStatus;
      int64 nCurTime = cmd->sNotifySeekCmdData.nSeekTime;
      uint32  nSegKey = cmd->sNotifySeekCmdData.nSegKey;
      int64 nSeekTime = cmd->sNotifySeekCmdData.nSeekTime;
      if(status == HTTPCommon::HTTPDL_WAITING)
      {
        status = HTTPCommon::HTTPDL_SEGMENT_BOUNDARY;
      }
      else if(status == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        HTTPResource* pSegHandler = NULL;

        MM_CriticalSection_Enter(pResourcesLock);

        (void)m_pRepHandler->GetResource(nSegKey, &pSegHandler);
        uint64 nStartTime = 0, nDuration = 0;
        if(pSegHandler)
        {
          pSegHandler->GetSegmentRange(nStartTime,nDuration);
        }

        MM_CriticalSection_Leave(pResourcesLock);

        nCurTime = nStartTime + nDuration;
        //Flush all the data before current time, just in case if there is any newly
        //added resource remaining to be flushed
        m_pRepHandler->Flush(HTTPCommon::HTTP_UNKNOWN_TYPE,(nCurTime - m_pRepHandler->m_cRepInfo.GetPTSOffset()));
        //Get the start time of first resource in inuse list. If there is a resource and
        //we know its start time then this time is passed from rephandler to playgroup
        //and playgroup depending on switch time and this resource's start time can either
        //switch and then continue seek or it can contunue seek on same representation
        if(m_pRepHandler->GetFirstResourceStartTime(nStartTime))
        {
          nSeekTime = nStartTime;
          status = HTTPCommon::HTTPDL_NEXT_SEGMENT;
        }
        else
        {
           nSeekTime = -1;
           status = HTTPCommon::HTTPDL_NO_MORE_RESOURCES;
        }
      }
      if(m_pRepHandler->m_pGroupNotifier)
      {
        m_pRepHandler->m_pGroupNotifier->NotifySeekStatus(
          m_pRepHandler->m_nKey,
          (-1 == nSeekTime ? nSeekTime : nSeekTime - m_pRepHandler->m_cRepInfo.GetPTSOffset()),
          (-1 == nCurTime ? nCurTime : nCurTime - m_pRepHandler->m_cRepInfo.GetPTSOffset()),
          status);
      }
      // Dequeue the Seek cmd
       void* pSeekIt = NULL;
       RepresentationCmdData seekCmd;
       while(m_pRepHandler->m_cCmdQ.Next(pSeekIt, seekCmd))
       {
         if (seekCmd.sBaseCmdData.eCmd == REPRESENTATION_CMD_SEEK)
         {
             QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                 "Rep [0x%06x]:: SEEK cmd dequeued",
                 (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40));
             m_pRepHandler->m_cCmdQ.Remove(pSeekIt);
             m_pRepHandler->m_bSeekPending = false;
             break;
         }
       }

      m_pRepHandler->m_cCmdQ.Remove(pIt);
    }
  }
  return ret;
}

/** @brief Segment info is available for segment. Processes the callback from segmenthandler.
  * @param[in] - nSegKey - Segment key for which segment info is avaialble
  * @param[in] - nNumDataUnits - Number of data units available
  * @param[in] - eStatus - Indicates the return status of get_segment_info cmd.
  * @return
  */
void DASHMediaRepresentationHandler::RepresentationReadyStateHandler::SegInfoReady
(
 const uint32 nSegKey,
 const uint32 nNumDataUnits,
 const HTTPDownloadStatus eStatus
 )
{
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  //Wait till info becomes available on the right segment
  if (nSegKey == (uint32)m_pRepHandler->m_cSegInfoCmdHandler.m_nStartIdx)
  {
    if(eStatus == HTTPCommon::HTTPDL_SUCCESS || (eStatus == HTTPCommon::HTTPDL_ERROR_ABORT))
    {
      HTTPResource* pSegHandler = NULL;

      MM_CriticalSection_Enter(pResourcesLock);

      m_pRepHandler->GetResource(nSegKey,&pSegHandler);
      if(pSegHandler)
      {
        uint64 nSegStartTime=0,nSegDuration=0;
        uint64 nSegMPDStartTime = 0;
        SegmentInfo cSegInfo;
        m_pRepHandler->m_sDASHSessionInfo.cMPDParser.GetSegmentInfoByKey(
          m_pRepHandler->m_cRepInfo.getKey() << 40 | nSegKey,cSegInfo);
        nSegMPDStartTime = (uint64)cSegInfo.getStartTime();
        if(pSegHandler->GetSegmentRange(nSegStartTime,nSegDuration))
        {
          nSegStartTime-=  m_pRepHandler->m_cRepInfo.GetPTSOffset();
          //Check whether there is a need to move to previous segment. If there
          //is a mismatch between mpd start time and segment start time as per
          //sidx then it is possible that we dont get the requested time range
          //in the segment which we received from mpd and we need to go to
          //previous segment.
          if(m_pRepHandler->m_cSegInfoCmdHandler.m_bMoveToPrevSegment == true  &&
             nSegStartTime > nSegMPDStartTime  &&  nSegStartTime >
             m_pRepHandler->m_cSegInfoCmdHandler.m_nCmdStartTime  &&
             nSegMPDStartTime <= m_pRepHandler->m_cSegInfoCmdHandler.m_nCmdStartTime)
          {
            m_pRepHandler->m_cSegInfoCmdHandler.Reset();

            m_pRepHandler->m_cSegInfoCmdHandler.m_nNextStartTime = nSegMPDStartTime -
                                                     (nSegStartTime - nSegMPDStartTime);
            QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "Rep [0x%06x]: GET_SEGINFO mismatch in mpd start time %u and segment start"
                           "time %u requesting info on previous segment. New start time for request %lld",
                           (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                           (uint32)nSegMPDStartTime, (uint32)nSegStartTime,
                           m_pRepHandler->m_cSegInfoCmdHandler.m_nNextStartTime );
            result = HTTPCommon::HTTPDL_WAITING;
          }
          else
          {
            result = eStatus;
          }
        }
      }
      MM_CriticalSection_Leave(pResourcesLock);
    }
    if(result != HTTPCommon::HTTPDL_WAITING)
    {
      QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
              "Rep [0x%06x]: GET_SEGINFO cmd processed %d by seg %u (nNumDataUnits %u)",
              (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
              eStatus, nSegKey, nNumDataUnits );
      //Segment with key nSegKey notified error
      if (HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND == eStatus)
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Marking segment with key %u as NOT_FOUND", nSegKey);
        m_pRepHandler->m_cSegInfoCmdHandler.m_bIsSegmentNotFound = true;
      }

      m_pRepHandler->m_cSegInfoCmdHandler.m_eState = SEGINFO_CMDHANDLER_AVAILABLE;
    }
  }
}

/** @brief Fill data unit info (in READY state).
  *
  * @param[in] nStartTime - Start time
  * @param[in] nDuration - Duration
  * @param[in] pDataUnitInfo - Reference to data unit info array
  * @param[in] nSizeOfDataUnitInfo - Size of data unit info array
  * @param[out] nNumDataUnitInfo - Number of data units filled/needed
  * @return
  * HTTPDL_SUCCESS - Data info successfully filled
  * HTTPDL_INSUFFICIENT_BUFFER - Insufficient buffer
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::FillDataUnitInfo
(
 uint64 nStartTime,
 uint64 nDuration,
 QSM::CDataUnitInfo* pDataUnitInfo,
 uint32 nSizeOfDataUnitInfo,
 uint32& nNumDataUnitInfo
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  uint32 nNumDataUnitsToFill = nSizeOfDataUnitInfo;
  nNumDataUnitInfo = 0;
  bool bLoop = (nDuration > 0);
  int64 nPrevSegKey = -1;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  MM_CriticalSection_Enter(pResourcesLock);

  //Read data unit info from the right segment(s) for the specified duration (all
  //data units that fall in the requested range)
  while (bLoop)
  {
    HTTPResource* pSegHandler = NULL;
    uint64 nSegStartTime, nSegDuration;

    //Find the right segment based on range
    (void)m_pRepHandler->GetResource(nStartTime, nDuration, pSegHandler);

    if (pSegHandler && pSegHandler->GetSegmentRange(nSegStartTime, nSegDuration))
    {
      uint64 nSegKey = pSegHandler->GetKey();
      if (nPrevSegKey < 0 || (nSegKey == (uint64)nPrevSegKey + 1))
      {
        nPrevSegKey = nSegKey;
        uint64 nReqStartTime = STD_MAX(nSegStartTime, nStartTime);
        uint64 nReqDuration = STD_MIN(((nSegStartTime + nSegDuration) - nReqStartTime), nDuration);
        double nRepEndTime =
          m_pRepHandler->m_sDASHSessionInfo.cMPDParser.GetDuration(m_pRepHandler->m_cRepInfo.getKey());
        if (nRepEndTime > 0.0)
        {
          nReqDuration =
            STD_MIN(nReqDuration,
                    (uint64)((uint64)nRepEndTime + m_pRepHandler->m_cRepInfo.GetPTSOffset()) - nReqStartTime);
        }
        uint32 nFilled = 0;

        (void)pSegHandler->FillDataUnitInfo(nReqStartTime,
                                            nReqDuration,
                                            pDataUnitInfo ? (pDataUnitInfo + nNumDataUnitInfo) : NULL,
                                            nNumDataUnitsToFill, nFilled);
        if (nFilled > 0)
        {
          nNumDataUnitInfo += nFilled;
          nStartTime = nSegStartTime + nSegDuration;
          nDuration -= nReqDuration;

          //If pDataUnitInfo is NULL total units required for nDuration is provided, else
          //array of data units is provided
          if (pDataUnitInfo)
          {
            for (uint32 j = 0; j < nFilled; j++)
            {
              //Prefix dataunit key with segment key so that all dataunit requests
              //can be indexed easily
              pDataUnitInfo[nNumDataUnitInfo - nFilled + j].m_nKey |= (nSegKey << 32);
            }

            QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                           "Rep [0x%06x]: Num data units filled %u/%u from seg %u, remaining duration %u msec",
                           (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                           nFilled, nNumDataUnitsToFill, (uint32)nSegKey, (uint32)nDuration );
            nNumDataUnitsToFill -= nFilled;
            if (nNumDataUnitsToFill <= 0)
            {
              bLoop = false;
            }
          }

          if (nDuration <= 0)
          {
            bLoop = false;
          }
        }
        else
        {
          //Couldn't read from the segment
          bLoop = false;
        }
      }
      else
      {
        //Non-contiguous segment found
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Rep [0x%06x]: Non-contiguous data unit %u/%u (curr/prev), breaking",
                       (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                       (uint32)nSegKey, (uint32)nPrevSegKey );
        bLoop = false;
      }
    }
    else
    {
      //Couldn't find the right segment
      bLoop = false;
    }
  }

  MM_CriticalSection_Leave(pResourcesLock);

  return (nNumDataUnitInfo > 0 ? HTTPCommon::HTTPDL_SUCCESS : HTTPCommon::HTTPDL_ERROR_ABORT);
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
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::GetDataUnitDownloadInfo
(
 QSM::CDataUnitDownloadInfo *pDownloadInfo,
 uint32 nSize,
 uint32 &nFilled,
 uint64 nStartTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPResource *pSegHandler = NULL;
  bool bLoop = true;
  int nSegment = 0;
  uint32 prevFilled = nFilled;
  nStartTime += m_pRepHandler->m_cRepInfo.GetPTSOffset();
  uint64 nSegStartTime = nStartTime;
  uint64 nSegDuration;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  MM_CriticalSection_Enter(pResourcesLock);

  while(bLoop)
  {
    /* Find a resource with provided start time*/
    (void)m_pRepHandler->GetResource(nSegStartTime, pSegHandler);
    if (pSegHandler && pSegHandler->GetSegmentRange(nSegStartTime, nSegDuration))
    {
      if(nSegDuration <= 0)
      {
        break;
      }
      if(status == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        /* Calling function to gather data unit download info  */
        status = pSegHandler->GetDataUnitDownloadInfo(pDownloadInfo,nSize,nFilled,nStartTime);
      }
      else
      {
        (void)pSegHandler->GetDataUnitDownloadInfo(pDownloadInfo,nSize,nFilled,nStartTime);
      }
      /* Update start time to look for next resource if any */
      nSegStartTime += nSegDuration;
    }
    else
    {
      bLoop = false;
    }
    pSegHandler = NULL;
  }

  if(status == HTTPCommon::HTTPDL_SUCCESS)
  {
    //Add PTSOffset to all data units information
     for(uint32 i = prevFilled; i < nFilled; i++)
     {
       pDownloadInfo[i].nStartTime-= m_pRepHandler->m_cRepInfo.GetPTSOffset();
     }
  }
  MM_CriticalSection_Leave(pResourcesLock);

  return status;
}

/** @brief Queue download request into the right segment (in READY state).
  *
  * @param[in] nDataUnitKey - Data unit key
  * @return
  * HTTPDL_SUCCESS - Download completed successfully
  * HTTPDL_WAITING - Download request queued (notified later)
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::GetSegmentData
(
 uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  MM_CriticalSection_Enter(pResourcesLock);

  HTTPResource* pSegHandler = m_pRepHandler->GetSegmentHandler(nDataUnitKey);

  if (pSegHandler)
  {
    //The least significant 32 bits indicates the data unit key
    status = pSegHandler->GetSegmentData(nDataUnitKey & 0x00000000FFFFFFFF);
  }

  MM_CriticalSection_Leave(pResourcesLock);

  return status;
}

HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::GetDataUnitInfoByKey
(
 uint64 nDataUnitKey,
 uint64 &nStartTime,
 uint64 &nDuration
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  MM_CriticalSection_Enter(pResourcesLock);

  HTTPResource* pSegHandler = m_pRepHandler->GetSegmentHandler(nDataUnitKey);

  if (pSegHandler)
  {
    //The least significant 32 bits indicates the data unit key
    status = pSegHandler->GetDataUnitInfoByKey((nDataUnitKey & 0x00000000FFFFFFFF),
                                               nStartTime,
                                               nDuration);

    nStartTime -= m_pRepHandler->m_cRepInfo.GetPTSOffset();
  }

  MM_CriticalSection_Leave(pResourcesLock);

  return status;
}
/** @brief Queue cancel download request into the right segment (in READY state).
  *
  * @param[in] nDataUnitKey - Data unit key
  * @return
  * HTTPDL_SUCCESS - Download cancelled successfully
  * HTTPDL_WAITING - Cancel Download request queued (notified later)
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::CancelSegmentData
(
 uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  MM_CriticalSection_Enter(pResourcesLock);

  HTTPResource* pSegHandler = m_pRepHandler->GetSegmentHandler(nDataUnitKey);

  if (pSegHandler)
  {
    //The least significant 32 bits indicates the data unit key
    status = pSegHandler->CancelSegmentData(nDataUnitKey & 0x00000000FFFFFFFF);
  }

  MM_CriticalSection_Leave(pResourcesLock);

  return status;
}

HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::ContinueDownloadDataUnit
(
  uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  MM_CriticalSection_Enter(pResourcesLock);
  HTTPResource* pSegHandler = m_pRepHandler->GetSegmentHandler(nDataUnitKey);
  if (pSegHandler)
  {
    //The least significant 32 bits indicates the data unit key
    status = pSegHandler->ContinueDownloadDataUnit(nDataUnitKey & 0x00000000FFFFFFFF);
  }

  MM_CriticalSection_Leave(pResourcesLock);

  return status;
}

void DASHMediaRepresentationHandler::RepresentationReadyStateHandler::NotifySeekStatus
(
  const uint32 nSegKey,
  int64 nSeekTime,
  HTTPDownloadStatus status
)
{
  RepresentationCmdData cmd;
  //check if already queued.
  void* pIt = NULL;
  cmd.sNotifySeekCmdData.eCmd = REPRESENTATION_CMD_NOTIFY_SEEK;
  cmd.sNotifySeekCmdData.eStatus = status;
  cmd.sNotifySeekCmdData.nSeekTime = nSeekTime;
  cmd.sNotifySeekCmdData.nSegKey = nSegKey;
  if (!m_pRepHandler->m_cCmdQ.EnQ(cmd))
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Rep [0x%06x]: Notify Seek cmd cannot be queued",
                   (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40) );
  }
  else
  {
     QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "Rep [0x%06x]: Notify Seek cmd queued successfully cnt %u",
                    (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                    m_pRepHandler->m_cCmdQ.Count());

  }
}
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::Seek
(
  int64 nSeekTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  RepresentationCmdData cmd;
  //check if already queued.
  void* pIt = NULL;
  while(m_pRepHandler->m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sBaseCmdData.eCmd == REPRESENTATION_CMD_SEEK)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "Rep [0x%06x]:: SEEK cmd already queued",
                     (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40));
      status = HTTPCommon::HTTPDL_WAITING;
      return status;
    }
  }

  cmd.sSeekCmdData.eCmd = REPRESENTATION_CMD_SEEK;
  cmd.sSeekCmdData.nSeekTime = nSeekTime;
  if (!m_pRepHandler->m_cCmdQ.EnQ(cmd))
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Rep [0x%06x]: Seek cmd cannot be queued",
                   (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40) );
  }
  else
  {
     QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "Rep [0x%06x]: Seek cmd queued successfully cnt %u",
                    (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                    m_pRepHandler->m_cCmdQ.Count());

  }

  return status;
}

void DASHMediaRepresentationHandler::RepresentationReadyStateHandler::NotifyDownloadTooSlow(
  const uint32 nSegKey, const uint64 nDataUnitKey)
{
  // Get the start-time for the fragment that is slow in downloading.
  uint64 nTooSlowStartTime = MAX_UINT64_VAL;;

  uint64 nFirstCancellableUnitSegKey = MAX_UINT64_VAL;
  uint64 nFirstCancellableDataUnitKey = MAX_UINT64_VAL;

  bool bNotify = false;
  uint64 nNotifyDataUnitKey = 0;

  QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "Notify tooSlow received for rep with id %s",
    m_pRepHandler->m_cRepInfo.getRepIdentifier());

  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  MM_CriticalSection_Enter(pResourcesLock);

  HTTPResource *pSegHandler = m_pRepHandler->GetSegmentHandler(((uint64)nSegKey << 32) | nDataUnitKey);
  if (pSegHandler)
  {
    if (pSegHandler->CheckReadsStartedAndDisableReads())
    {
      nFirstCancellableUnitSegKey = nSegKey;
      nFirstCancellableDataUnitKey = nDataUnitKey;

      // This data unit can be cancelled. It is okay for socket reads to conitnue here,
      // as this resource will not become readable anyways. However, to limit the number
      // use cases to run into, disabling it anyways so that there is no chance of furthur
      // units getting downloaded and too-slow getting reported on those while cancel
      // processing is outstanding.
      if (false == pSegHandler->DisableSocketReads(nFirstCancellableDataUnitKey, true))
      {
        // sanity check failed.
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Failed to disable socket reads on units (%d,%d)",
                      (int)nFirstCancellableUnitSegKey, (int)nFirstCancellableDataUnitKey);
      }
      else
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "DataUnit (seg %d, du %d) is cancellable and resource is not read yet",
                      (int)nFirstCancellableUnitSegKey, (int)nFirstCancellableDataUnitKey);

        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Notify downloadTooSlow for firstCancellableDataUnit seg %d, dataunit %d",
                      (int) nSegKey, (int)nFirstCancellableDataUnitKey);

        bNotify = true;
        nNotifyDataUnitKey = ((uint64)nFirstCancellableUnitSegKey << 32) | nFirstCancellableDataUnitKey;
      }
    }
    else
    {
      (void)m_pRepHandler->GetFirstCancellableDataUnit(
        nSegKey, nDataUnitKey, nFirstCancellableUnitSegKey, nFirstCancellableDataUnitKey);

      if (m_pRepHandler->m_pGroupNotifier)
      {
        if( nFirstCancellableUnitSegKey < MAX_UINT64_VAL &&
            nFirstCancellableDataUnitKey < MAX_UINT64_VAL)
        {
          pSegHandler = m_pRepHandler->GetSegmentHandler(
            ((uint64)nFirstCancellableUnitSegKey << 32) | nFirstCancellableUnitSegKey);

          if (pSegHandler)
          {
            // disable socket reads for this data unit to be cancelled. Otherwise,
            // GetAvailOffset will become inconsistent. BUt is okay for getnextSample
            // to have been called on fragments preceding the fragment to be cancelled.
            if (pSegHandler->DisableSocketReads(nFirstCancellableDataUnitKey, true))
            {
              QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                            "Notify downloadTooSlow for firstCancellableDataUnit seg %d, dataunit %d",
                            (int) nFirstCancellableUnitSegKey, (int)nFirstCancellableDataUnitKey);

              bNotify = true;
              nNotifyDataUnitKey =
                ((uint64)nFirstCancellableUnitSegKey << 32) | nFirstCancellableDataUnitKey;
            }
            else
            {
              // Sanity check failed
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Failed to disable socket reads");
            }
          }
          else
          {
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                          "NotifyDownloadTooSlow: Failed to find seghandler for (%d,%d)",
                          (int)((uint64)nFirstCancellableUnitSegKey << 32),
                          (int)nFirstCancellableDataUnitKey);
          }
        }
        else
        {
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "DownloadTooSlow: did not find a cancellable unit following seg (%d,%d)",
                        (int)nSegKey, (int)nDataUnitKey);
        }
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "Null seghandler");
  }

  MM_CriticalSection_Leave(pResourcesLock);

  if (bNotify)
  {
    m_pRepHandler->m_pGroupNotifier->NotifyDownloadTooSlow(
       m_pRepHandler->m_nKey, nNotifyDataUnitKey);
  }
}


/** @brief Queues open cmd
  *
  * @param[in] nDataUnitKey - Data unit key
  * @param[in] nStartTime - starttime
  * @param[in] bSeek - flag to indicate if seek needs to be issued.
  * @return
  * HTTPDL_WAITING - Cmd queued successfully
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::Open
(
  uint64 nDataUnitKey,
  int64 nStartTime,
  bool bSeek
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  RepresentationCmdData cmd;
  //check if already queued.
  void* pIt = NULL;
  m_pRepHandler->m_bIsLmsgSet = false;
  while(m_pRepHandler->m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sBaseCmdData.eCmd == REPRESENTATION_CMD_OPEN)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "Rep [0x%06x]:: OPEN cmd already queued",
                     (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40));
      status = HTTPCommon::HTTPDL_WAITING;
      return status;
    }
  }
  //Queue open cmd only if resource manager is not in opening state.
  if(!m_pRepHandler->IsOpened())
  {
    cmd.sOpenCmdData.eCmd = REPRESENTATION_CMD_OPEN;
    cmd.sOpenCmdData.nStartTime = nStartTime;
    cmd.sOpenCmdData.nDataUnitKey = nDataUnitKey;
    cmd.sOpenCmdData.bSeek = bSeek;

    if (!m_pRepHandler->m_cCmdQ.EnQ(cmd))
    {
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Rep [0x%06x]: OPEN cmd cannot be queued",
                     (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40) );
    }
    else
    {
       QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Rep [0x%06x]: OPEN cmd queued successfully cnt %u",
                     (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                     m_pRepHandler->m_cCmdQ.Count());

    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "Rep [0x%06x]:: OPEN cmd already processed",
                     (uint32)((m_pRepHandler->m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40));
    status = HTTPCommon::HTTPDL_WAITING;
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
HTTPDownloadStatus DASHMediaRepresentationHandler::RepresentationReadyStateHandler::Select
(
 uint64 nDataUnitKey,
 uint64& nPbTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  MM_HANDLE pResourcesLock = m_pRepHandler->m_resourcesLock;

  MM_CriticalSection_Enter(pResourcesLock);

  HTTPResource* pSegHandler = m_pRepHandler->GetSegmentHandler(nDataUnitKey);

  if (pSegHandler)
  {
    //The least significant 32 bits indicates the data unit key
    status = pSegHandler->Select(nDataUnitKey & 0x00000000FFFFFFFF, nPbTime);

    nPbTime = (nPbTime > m_pRepHandler->m_cRepInfo.GetPTSOffset()
               ? nPbTime - m_pRepHandler->m_cRepInfo.GetPTSOffset()
               : 0);
  }

  MM_CriticalSection_Leave(pResourcesLock);

  return status;
}

/**
 * Set the end time adjusted for pts offset
 */
void DASHMediaRepresentationHandler::SetEndTime
(
 uint64 nEndTime
)
{
  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Setting EOS on rep %p endtime %llu, isLive %d",
                 (void *)this, nEndTime, m_sDASHSessionInfo.cMPDParser.IsLive());

  // In case of Live MPD with no duration, endtime is zero.
  // to allow playback till last downloaded resource, not to set endtime
  // for live usecase without duration
  if(nEndTime > 0)
  {
    return HTTPResourceManager::SetEndTime(nEndTime + m_cRepInfo.GetPTSOffset());
  }
}

/** @brief Create a DASH data resource (i.e. segment handler).
  *
  * @param[in] nSegKey - Segment key
  * @param[in] cSegInfo - Segment info
  * @param[in] ppSegHandler - Reference to seg handler
  * @return
  * TRUE - Resource created successfully
  * FALSE - Otherwise
  */
bool DASHMediaRepresentationHandler::CreateSegmentHandler
(
 const uint32 nSegKey,
 SegmentInfo& cSegInfo,
 HTTPResource** ppSegHandler
)
{
  // This function does not take the resources lock, as the caller must take
  // the resources lock as *ppSegHandler is ptr to resource (pResource) which
  // itself points to a queue element.
  bool bResult = false;

  if (ppSegHandler)
  {
    //Try to reuse the resource from the free list, if not create a new resource
    bool bReusableResource = true;
    HTTPResource* pResource = NULL;
    (void)GetFreeResource(&pResource);

    if(!pResource)
    {
      pResource =
        QTV_New_Args(DASHMediaSegmentHandler, (bResult,
                                               m_sDASHSessionInfo,
                                               (iRepresentationNotifier *)this,
                                               m_bEstimator,
                                               m_rSidxFetchDecision,
                                              (m_pInitSegment ?
                                               (HttpSegmentDataStoreBase*)&m_pInitSegment->m_cDataStore : NULL),
                                               m_pScheduler,
                                               m_pHTTPStack)
                                               );
      bReusableResource = false;
    }

    if (pResource)
    {
      /* Passing Bandwidth Estimator Object */
      cSegInfo.m_bEstimator = m_bEstimator;
      uint64 nStartTime = (uint64)cSegInfo.getStartTime() + m_cRepInfo.GetPTSOffset();
      uint64 nDuration = (uint64)cSegInfo.getDuration();
      pResource->SetSegmentInfo(cSegInfo,nStartTime,nDuration, nSegKey, (m_pInitSegment ? m_pInitSegment->m_pURL : NULL));
      HTTPDownloadStatus status = AddResource(nSegKey, pResource);

      //Remove resource (delete also) if unable to add and not a reusable resource
      if (!HTTPSUCCEEDED(status))
      {
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Rep [0x%06x]: Could not add resource (seg handler) %d",
                       (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40), status );
        if (!bReusableResource)
        {
          QTV_Delete(pResource);
        }
        pResource = NULL;
      }
      else
      {
        bResult = true;
      }
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Could not create resource for key %u", (uint32) nSegKey );
    }

    *ppSegHandler = pResource;
  }

  return bResult;
}

/** @brief Get a DASH data resource (i.e. segment handler) based on data unit key.
  *
  * @param[in] nDataUnitKey - Data unit key
  * @return DASH data resource
  */
HTTPResource* DASHMediaRepresentationHandler::GetSegmentHandler
(
 const uint64 nDataUnitKey
)
{
  HTTPResource* pSegHandler = NULL;

  //Extract segment index from the data unit key (most significant 32 bits)
  uint32 nSegIdx = (uint32)(nDataUnitKey >> 32);
  (void)GetResource(nSegIdx, &pSegHandler);
  if (pSegHandler == NULL)
  {
    QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Rep [0x%06x]: Invalid nDataUnitKey %u (nSegIdx %u)",
                   (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                   (uint32)nDataUnitKey, nSegIdx );
  }

  return pSegHandler;
}

/** @brief Clear representation cmd Q.
  *
  */
void DASHMediaRepresentationHandler::ClearCmdQ(HTTPDownloadStatus eStatus)
{
  RepresentationCmdData cmd;
  void* pIt = NULL;

  //Clear and reset cmd Q, notifying failure for any outstanding cmds
  while(m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sBaseCmdData.eCmd == REPRESENTATION_CMD_GET_SEGINFO)
    {
      //Notify failure
      if (m_pGroupNotifier)
      {
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Rep [0x%06x]: GET_SEGINFO cmd (nStartTime %u nDuration %u) "
                       "cleared - notifying group",
                       (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40),
                       (uint32)cmd.sGetSegInfoCmdData.nStartTime,
                       (uint32)cmd.sGetSegInfoCmdData.nDuration );
        m_pGroupNotifier->SegInfoReady(m_nKey,
                                       (uint32)cmd.sGetSegInfoCmdData.nStartTime,
                                       (uint32)cmd.sGetSegInfoCmdData.nDuration,
                                       0, eStatus);
      }
    }

    //Remove the representation cmd
    (void)m_cCmdQ.Remove(pIt);
  }

  (void)m_cCmdQ.Reset();
}

/** @brief Process error notification.
  *
  * @param[in] eStatus - Error status
  */
void DASHMediaRepresentationHandler::OnError
(
 const HTTPDownloadStatus eStatus
)
{
  //Move to ERROR state only for fatal rep errors such as INIT seg download failures.
  //The only way out of this is to close the rep!
  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Rep [0x%06x]: Moving to ERROR %d and notifying representation",
                 (uint32)((m_cRepInfo.getKey() & 0xFFFFFF0000000000ULL) >> 40), eStatus );
  SetStateHandler(&m_cErrorStateHandler);

  //Reset INIT segment (if applicable)
  if (m_pInitSegment && !m_pInitSegment->isCommonInit())
  {
    m_pInitSegment->Reset();
  }
  m_cSegInfoCmdHandler.Reset();

  // In case the status is INVALID_REP it is okay to not tie with any command, as currently
  // the only cmd in GET_SEGINFO and notification should be INVALID_REP if there is no init segment.
  ClearCmdQ(eStatus);

  if (m_pGroupNotifier)
  {
    m_pGroupNotifier->NotifyError(m_nKey, eStatus);
  }
}

/** @brief Get key for current Representation Info.
  *
  */
int DASHMediaRepresentationHandler::GetRepInfoKey()
{
  return ((int)((m_cRepInfo.getKey() & MPD_REPR_MASK) >> MPD_REPR_SHIFT_COUNT));
}

} // namespace video
