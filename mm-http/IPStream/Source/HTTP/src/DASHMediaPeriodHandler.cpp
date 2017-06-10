/************************************************************************* */
/**
 * DASHMediaPeriodHandler.cpp
 * @brief Implements the DASHMediaPeriodHandler. Each such object handles
 *        a DASH period and manages all the media representations within
 *        this period through a play group. It also interfaces with QSM
 *        and DASHAdaptor for download and playback commands respectively.
 *
 * COPYRIGHT 2011-2015 QUALCOMM Technologies, Inc.
 * All rights reserved. QUALCOMM Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/DASHMediaPeriodHandler.cpp#125 $
$DateTime: 2013/10/30 17:31:44 $
$Change: 4692372 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "DASHMediaPeriodHandler.h"

#ifdef DASH_STATS
#include "HTTPHeapManager.h"
#include <MMFile.h>
#endif /* DASH_STATS */

namespace video {

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define HTTP_MAX_PERIOD_COMMANDS          5

#ifdef DASH_STATS
#if defined (WIN32)
#define DASH_STATS_FILE_PATH              ".\\dash.stats"
#else
#define DASH_STATS_FILE_PATH              "/data/misc/media/dash.stats"
#endif /* WIN32 */
#endif /* DASH_STATS */

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
/** @brief DASHMediaPeriodHandler constructor.
  *
  * @param[out] bResult - Status of initialization
  * @param[in] sDASHSessionInfo - Reference to DASH session info
  * @param[in] cPeriodInfo - Reference to period info
  * @param[in] pNotifier - Reference to DASH adaptor notifier
  * @param[in] pScheduler - Reference to task scheduler
  */
DASHMediaPeriodHandler::DASHMediaPeriodHandler
(
 bool& bResult,
 DASHSessionInfo& sDASHSessionInfo,
 HTTPBandwidthEstimator *pHTTPBandwidthEstimator,
 PeriodInfo& cPeriodInfo,
 iDASHAdaptorNotifier* pNotifier,
 iDASHAdaptorInfoQuery* pInfoQuery,
 IPStreamGetCreateQSM pQSMInstanceCreate,
 IPStreamGetDeleteQSM pQSMInstanceDestroy,
 Scheduler* pScheduler
) : m_sDASHSessionInfo(sDASHSessionInfo),
    m_bEstimator(pHTTPBandwidthEstimator),
    m_cPeriodInfo(cPeriodInfo),
    m_pDANotifier(pNotifier),
    m_pDAInfoQuery(pInfoQuery),
    m_pScheduler(pScheduler),
    m_nTaskID(0),
    m_pPeriodDataLock(NULL),
    m_bIsDownloadCompleteNotified(false),
    m_pQSM(NULL),
    m_cIdleStateHandler(this),
    m_cOpeningStateHandler(this),
    m_cOpenStateHandler(this),
    m_cSeekingStateHandler(this),
    m_cClosingStateHandler(this),
    m_pCurrentStateHandler(NULL),
    m_nStartTime(0),
    m_nSeekTime(-1),
    m_bPauseCmdProcessing(false),
    m_bDeleteQsm(false),
    m_eSeekStatus(HTTPCommon::HTTPDL_SUCCESS),
    m_hQSMStopSignalQ(NULL),
    m_hQSMStopRspSignal(NULL),
    m_pQSMInstanceCreate(pQSMInstanceCreate),
    m_pQSMInstanceDestroy(pQSMInstanceDestroy),
    m_nNextKey(0),
    m_bIsQSMSuspended(false)
#ifdef DASH_STATS
    ,m_pStatsFile(NULL),
    m_nCurrentRepID(-1)
#endif /* DASH_STATS */
{
  //Start with IDLE state
  SetStateHandler(&m_cIdleStateHandler);

  //Create data lock
  bResult = (MM_CriticalSection_Create(&m_pPeriodDataLock) == 0);

  //Initialize cmd queue
  if (bResult)
  {
    bResult = m_cCmdQ.Init(HTTP_MAX_PERIOD_COMMANDS);
  }

  //Create context - use HTTP thread if pScheduler is passed in
  if (bResult)
  {
    if (m_pScheduler)
    {
      //Add TaskMediaPeriod to scheduler
      bResult = false;
      SchedulerTask pTask = TaskMediaPeriod;
      PeriodCmdTaskParam* pTaskParam = QTV_New_Args(PeriodCmdTaskParam,
                                                    ((void*)this));
      if (pTaskParam)
      {
        m_nTaskID = m_pScheduler->AddTask(pTask, (void*)pTaskParam);
        if (!m_nTaskID)
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "TaskMediaRepresentation could not be added to scheduler" );
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
      //ToDo: Create thread with TaskMediaPeriod as the entry point
      bResult = false;
    }
  }

  if (bResult)
  {
    if (MM_SignalQ_Create(&m_hQSMStopSignalQ) != 0)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Unable to create QSM stop signalQ" );
      bResult = false;
    }
  }

  if (bResult)
  {
    if (MM_Signal_Create(m_hQSMStopSignalQ, 0, 0, &m_hQSMStopRspSignal) != 0)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Unable to create QSM stop signal" );
      bResult = false;
    }
  }

  //Create QSM. If QSM create succeeds, ensure that the c'tor will succeed so
  //that d'tor will not be called after QSM create succeeds. This ensures that
  //async stop on QSM need not be dealt with.
  if (bResult)
  {
    //Create a "play" group and notify QSM of group/rep info availability
    bResult = CreatePlayGroup();
    if (bResult)
    {
      if(m_pQSMInstanceCreate)
      {
        m_pQSM = m_pQSMInstanceCreate((IStreamSource*)this, (IDataStateProvider*)this);
      }
      if (m_pQSM == NULL)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "QSM could not be created" );
        bResult = false;
      }
      else
      {
        m_pQSM->RepresentationInfoAvailable();
      }
    }
  }

  for (int i = 0; i < HTTPCommon::HTTP_MAX_TYPE; ++i)
  {
    m_LastSampleTS[i] = MAX_UINT64_VAL;
  }

#ifdef DASH_STATS
  if (MM_File_Create(DASH_STATS_FILE_PATH, MM_FILE_CREATE_W_PLUS, &m_pStatsFile) == 0 && m_pStatsFile)
  {
    char statsFileEntry[100] = {0};
    int numBytesWritten = 0;
    (void)std_strlprintf(statsFileEntry, sizeof(statsFileEntry),
                         "PeriodID\tRepID\tPlaybackTime(msec)\tBandwidth(Kbps)\tBufferOccupancy(msec)\tFreeSpace(KB)\r\n");
    (void)MM_File_Write(m_pStatsFile, statsFileEntry, std_strlen(statsFileEntry), &numBytesWritten);
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Period [0x%02x]: DASH stats file create failed",
                   (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
  }
#endif /* DASH_STATS */

  QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Period [0x%02x]: Created DASH period handler %p QSM %p result %d",
                 (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                 (void *)this, (void *)m_pQSM, bResult );
}

/** @brief DASHMediaPeriodHandler destructor.
  *
  */
DASHMediaPeriodHandler::~DASHMediaPeriodHandler()
{
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "~DASHMediaPeriodHandler %p, key %u",
    (void *)this, (unsigned int)((GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56));

  if (m_pQSM)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Qsm instance not null in PeriodHandler destructor");
  }

  if (m_pPeriodDataLock)
  {
    (void)MM_CriticalSection_Release(m_pPeriodDataLock);
    m_pPeriodDataLock = NULL;
  }

  if (m_pScheduler)
  {
    (void)m_pScheduler->DeleteTask(m_nTaskID);
  }

  if (m_hQSMStopRspSignal)
  {
    (void)MM_Signal_Release(m_hQSMStopRspSignal);
    m_hQSMStopRspSignal = 0;
  }

  if (m_hQSMStopSignalQ)
  {
    (void)MM_SignalQ_Release(m_hQSMStopSignalQ);
    m_hQSMStopSignalQ = 0;
  }

  while (!m_DASMQ.IsEmpty())
  {
    DASMQElem *pDASMElem = m_DASMQ.PeekHeadElem();
    m_DASMQ.Pop();

    if (pDASMElem)
    {
      QTV_Delete(pDASMElem);
    }
  }

  for (uint32 i = 0; i <= QSM::MAJOR_TYPE_MAX;++i)
  {
    m_RepGroupQ[i].Shutdown();
  }

#ifdef DASH_STATS
  if (m_pStatsFile)
  {
    (void)MM_File_Release(m_pStatsFile);
    m_pStatsFile = NULL;
  }
#endif /* DASH_STATS */

  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Period [0x%02x]: Destroyed DASH period handler %p",
                 (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56), (void *)this );
}

/** @brief Media period task.
  *
  * @param[in] pParam - Task parameter
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaPeriodHandler::TaskMediaPeriod
(
 void* pParam
)
{
  int status = -1;
  PeriodCmdTaskParam* pTaskParam = (PeriodCmdTaskParam*) pParam;

  if (pTaskParam == NULL || pTaskParam->pSelf == NULL)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid task param %p", (void *)pTaskParam );
  }
  else
  {
    //Process commands in the current state
    DASHMediaPeriodHandler* pSelf = (DASHMediaPeriodHandler*)pTaskParam->pSelf;
    PeriodBaseStateHandler* pCurrStateHandler = pSelf->GetStateHandler();
    if (pCurrStateHandler)
    {
      status = pCurrStateHandler->ProcessCmds();
    }
  }

  return status;
}

/**
 * Query for number of binning thresholds for switchable groups.
 */
QSM::IStreamSourceStatus
DASHMediaPeriodHandler::GetNumberOfBinningThresholdsForSwitchableGrps(uint32& nNumThresholds)

{
  // Based on design meeting, there will be a constant number thresholds for
  // each switchable group. And per interface contract, this number cannot
  // change within period.
  nNumThresholds = 1;
  return QSM::SS_STATUS_OK;
}

/**
 * Construct the addset, removeset, replaceset by comparing
 * pSelectionsXML and the currently selected adaptation-sets.
 */
bool DASHMediaPeriodHandler::HandleAdaptationSetChangeInfo(const char *pSelectionsXml)
{
  bool rslt = false;

  uint64 newRepGrpKeysForMajorType[1 + QSM::MAJOR_TYPE_MAX];
  uint64 oldRepGrpKeysForMajorType[1 + QSM::MAJOR_TYPE_MAX];
  static const uint64 INVALID_KEY = MAX_UINT64_VAL;
  for (int i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
  {
    newRepGrpKeysForMajorType[i] = INVALID_KEY;
    oldRepGrpKeysForMajorType[i] = INVALID_KEY;
  }

  // Parse the xml to get the new selections info.
  IPStreamList<uint64> newRepGrpXMLKeySet;
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "AdaptationSetChange: New RepGrpKeys");
  {
    uint64 nPeriodKey = (m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56;
    m_sDASHSessionInfo.cMPDParser.GetRepGrpKeysForAdaptationSetChange(
       nPeriodKey, pSelectionsXml, newRepGrpXMLKeySet);

    IPStreamList<uint64>::Iterator iterEnd = newRepGrpXMLKeySet.End();
    for (IPStreamList<uint64>::Iterator iter = newRepGrpXMLKeySet.Begin();
          iter != iterEnd; iter = iter.Next())
    {
      uint64 xmlKey = *iter;
      uint64 grpKey = MAX_UINT64_VAL;
      RepresentationGroup tmpRepGrp;
      rslt = GetRepresentationGroupFromXmlKey(xmlKey, tmpRepGrp);
      if (rslt)
      {
        uint32 majorType;
        GetGroupMajorType(tmpRepGrp, majorType);
        newRepGrpKeysForMajorType[majorType] = tmpRepGrp.getKey();
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "AdaptationSetChange: majorType %u, Key %llu (NEW)",
                     majorType, newRepGrpKeysForMajorType[majorType]);
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "GetRepresentationGroupFromXmlKey failed for group with xml key %llu", xmlKey);
      }
    }

    newRepGrpXMLKeySet.Clear();
  }

  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    if (!m_RepGroupQ[i].IsEmpty())
    {
      RepGroupQElem *pElem = m_RepGroupQ[i].PeekTail();
      if (pElem)
      {
        if (pElem->GetAdaptationSetChangeType() == RepGroupQElem::NONE ||
            pElem->GetAdaptationSetChangeType() == RepGroupQElem::ADD ||
            pElem->GetAdaptationSetChangeType() == RepGroupQElem::REPLACE)
        {
          DASHMediaPlayGroup *pPlayGrp = pElem->m_pRepGroup;

          if (pPlayGrp)
          {
            uint64 nRepGrpInfoKey = pPlayGrp->GetRepGrpInfoKey();
            oldRepGrpKeysForMajorType[pPlayGrp->m_nMajorType] = nRepGrpInfoKey;
          }
        }
      }
    }
  }

  MM_CriticalSection_Enter(m_pPeriodDataLock);

  if (m_DASMQ.IsEmpty())
  {
    for (uint32 i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
    {
      bool bNew = false, bOld = false;
      if (INVALID_KEY != newRepGrpKeysForMajorType[i])
      {
        bNew = true;
      }

      if (INVALID_KEY != oldRepGrpKeysForMajorType[i])
      {
        bOld = true;
      }

      if (bNew && bOld)
      {
        if (newRepGrpKeysForMajorType[i] != oldRepGrpKeysForMajorType[i])
        {
          if (QSM::MAJOR_TYPE_TEXT == i)
          {
            DASMQElem *pNewDASMElem1 = QTV_New(DASMQElem);
            DASMQElem *pNewDASMElem2 = QTV_New(DASMQElem);
            if (pNewDASMElem1 && pNewDASMElem2)
            {
              pNewDASMElem1->SetMajorType(i);
              pNewDASMElem1->SetOldRepGrpInfoKey(oldRepGrpKeysForMajorType[i]);
              rslt = m_DASMQ.Push(pNewDASMElem1);
              if (false == rslt)
              {
                QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Failed to push element into DASMQ");
              }
              else
              {
                pNewDASMElem2->SetMajorType(i);
                pNewDASMElem2->SetNewRepGrpInfoKey(newRepGrpKeysForMajorType[i]);
                rslt = m_DASMQ.Push(pNewDASMElem2);

                if (false == rslt)
                {
                  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                               "Failed to push element into DASMQ");
                }
              }
            }
            else
            {
              if (pNewDASMElem1)
              {
                QTV_Delete(pNewDASMElem1);
              }

              if (pNewDASMElem2)
              {
                QTV_Delete(pNewDASMElem2);
              }
            }
          }
          else
          {
            DASMQElem *pNewDASMElem = QTV_New(DASMQElem);

            if (pNewDASMElem)
            {
              pNewDASMElem->SetMajorType(i);
              pNewDASMElem->SetOldRepGrpInfoKey(oldRepGrpKeysForMajorType[i]);
              pNewDASMElem->SetNewRepGrpInfoKey(newRepGrpKeysForMajorType[i]);
              rslt = m_DASMQ.Push(pNewDASMElem);
              if (false == rslt)
              {
                QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Failed to push element into DASMQ");
              }
            }
            else
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Failed to allocated DASMQElem");
            }
          }
        }
      }
      else if (bNew)
      {
        DASMQElem *pNewDASMElem = QTV_New(DASMQElem);
        if (pNewDASMElem)
        {
          pNewDASMElem->SetMajorType(i);
          pNewDASMElem->SetNewRepGrpInfoKey(newRepGrpKeysForMajorType[i]);
          rslt = m_DASMQ.Push(pNewDASMElem);

          if (false == rslt)
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Failed to push element into DASMQ");
          }
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Failed to allocated DASMQElem");
        }
      }
      else if (bOld)
      {
        DASMQElem *pNewDASMElem = QTV_New(DASMQElem);
        if (pNewDASMElem)
        {
          pNewDASMElem->SetMajorType(i);
          pNewDASMElem->SetOldRepGrpInfoKey(oldRepGrpKeysForMajorType[i]);
          rslt = m_DASMQ.Push(pNewDASMElem);
          if (false == rslt)
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Failed to push element into DASMQ");
          }
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Failed to allocated DASMQElem");
        }
      }
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Unexpected error: DASMQ not empty");
  }

  MM_CriticalSection_Leave(m_pPeriodDataLock);

  if (true == rslt)
  {
    PeriodCmdData cmd;
    cmd.sBaseCmdData.eCmd = PERIOD_CMD_ADAPTATION_SET_CHANGE;

    if (!m_cCmdQ.EnQ(cmd))
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Period [0x%02x]: AdaptationSetChange cmd cannot be queued",
                     (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );

      m_DASMQ.PopTail();
    }
  }

  return rslt;
}

void DASHMediaPeriodHandler::SelectAdaptationSet()
{
  DASMQElem *pDASMHeadElem = m_DASMQ.PeekHeadElem();
  if (pDASMHeadElem)
  {
    uint64 nOldRepGrpInfoKey = pDASMHeadElem->GetOldRepGrpInfoKey();
    uint64 nNewRepGrpInfoKey = pDASMHeadElem->GetNewRepGrpInfoKey();
    uint32 nOldAdapXMLKey = (uint32)((nOldRepGrpInfoKey & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
    uint32 nNewAdapXMLKey = (uint32)((nNewRepGrpInfoKey & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);

    IPStreamList<uint32> nTmpRepIDList; // empty for now.

    uint32 nPeriodXMLKey = (uint32)((GetPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);

    bool bOk = true;

    if (nOldRepGrpInfoKey != MAX_UINT64_VAL && nNewRepGrpInfoKey != MAX_UINT64_VAL)
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "AdaptationSetChange: Replace Selection from %llu to %llu",
        pDASMHeadElem->GetOldRepGrpInfoKey(), pDASMHeadElem->GetNewRepGrpInfoKey());

      bOk = m_sDASHSessionInfo.cMPDParser.ModifySelectedRepresentations(
         nPeriodXMLKey, nOldAdapXMLKey, nTmpRepIDList, false);

      if (bOk)
      {
        bOk = m_sDASHSessionInfo.cMPDParser.ModifySelectedRepresentations(
           nPeriodXMLKey, nNewAdapXMLKey, nTmpRepIDList, true);

        if (false == bOk)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "Failed to modify SelectedReps for new adaptationset xml key %u",
            nOldAdapXMLKey);
          // re-select the old adaptation-set.
          m_sDASHSessionInfo.cMPDParser.ModifySelectedRepresentations(
             nPeriodXMLKey, nNewAdapXMLKey, nTmpRepIDList, false);
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Failed to modify SelectedReps for old adaptationset xml key %u",
          nOldAdapXMLKey);
      }
    }
    else if (nNewRepGrpInfoKey != MAX_UINT64_VAL)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "AdaptationSetChange: ADD Selection %llu", pDASMHeadElem->GetNewRepGrpInfoKey());

      bOk = m_sDASHSessionInfo.cMPDParser.ModifySelectedRepresentations(
         nPeriodXMLKey, nNewAdapXMLKey, nTmpRepIDList, true);
    }
    else if (nOldRepGrpInfoKey != MAX_UINT64_VAL)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "AdaptationSetChange: REMOVE Selection %llu", pDASMHeadElem->GetOldRepGrpInfoKey());
      bOk = m_sDASHSessionInfo.cMPDParser.ModifySelectedRepresentations(
         nPeriodXMLKey, nOldAdapXMLKey, nTmpRepIDList, false);
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "AdaptationSetChange: Unexpected error did not find any adapset to change");
    }

    if (bOk)
    {
      bOk = HandleAdaptationSetChangeQsmCmd();
      if (false == bOk)
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "AdaptationSet Change failed on period with key %llu. Notify resources lost",
          GetPeriodKey());
        m_pDANotifier->NotifyEvent(GetPeriodKey(), PERIOD_CMD_NONE, HTTPCommon::HTTPDL_ERROR_ABORT, NULL);
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "HandleAdaptationSetChangeQsmCmd failed");
    }
  }
  else if (m_bIsQSMSuspended)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "AdaptationSetChange: Resume but don't invoke QSM adaptationset change cmd");
    ResumeFromSuspendedState();
  }
}

bool DASHMediaPeriodHandler::ResumeFromSuspendedState()
{
  bool rslt = true;

  if (m_bIsQSMSuspended)
  {
    m_bIsQSMSuspended = false;

    for (uint32 i= 0; i <= QSM::MAJOR_TYPE_MAX; i++)
    {
      m_RepGroupQ[i].RemoveAllButLastElementFromQ();
    }

    QSM::QsmStatus qsmStatus = m_pQSM->Resume();

    if (QSM::QSM_STATUS_OK != qsmStatus)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Failed to resume suspended QSM qsmstatus %d", qsmStatus);
      rslt = false;
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "Resumed QSM for period %llu", GetPeriodKey());
    }
  }

  return rslt;
}

/**

 * Invoke QSM adaptationset change commands.
 */
bool DASHMediaPeriodHandler::HandleAdaptationSetChangeQsmCmd()
{
  bool rslt = false;

  // at this point should have add addset, removeset, replaceset constructed.
  uint32 nMajorType = QSM::MAJOR_TYPE_UNKNOWN;

  QSM::IEnhancedStreamSwitchManager::AdaptationSetChangeType asct;

  QSM::CGroupInfo cInfo;
  cInfo.m_bDataUnitsAligned = true;
  cInfo.m_bDataUnitsStartWithRap = true;
  cInfo.m_eMajorType = nMajorType;

  cInfo.m_nBinThresholds = QTV_New_Array(uint32, 1);

  // TO DO: The part below is if-else because QSM's interface currently takes
  // only one change at a time whereas the design document takes a list. We
  // need to come to a conclusion if Source will serialize this or QSM will change
  // the interface to take a list.
  uint64 nAddOrReplaceGrpKey = MAX_UINT64_VAL;
  uint64 nRemoveKey = MAX_UINT64_VAL;

  MM_CriticalSection_Enter(m_pPeriodDataLock);

  DASMQElem *pDASMElem = m_DASMQ.PeekHeadElem();
  if (pDASMElem)
  {
    if (pDASMElem->GetOldRepGrpInfoKey() != MAX_UINT64_VAL &&
        pDASMElem->GetNewRepGrpInfoKey() != MAX_UINT64_VAL)
    {
      nAddOrReplaceGrpKey = pDASMElem->GetNewRepGrpInfoKey();

      RepresentationGroup repGrp;
      GetRepresentationGroupFromGrpKey(nAddOrReplaceGrpKey, repGrp);
      GetGroupMajorType(repGrp, nMajorType);

      asct.mType = QSM::IEnhancedStreamSwitchManager::AdaptationSetChangeType::REPLACE;
    }
    else if (pDASMElem->GetNewRepGrpInfoKey() != MAX_UINT64_VAL)
    {
      nAddOrReplaceGrpKey = pDASMElem->GetNewRepGrpInfoKey();

      RepresentationGroup repGrp;
      GetRepresentationGroupFromGrpKey(nAddOrReplaceGrpKey, repGrp);
      GetGroupMajorType(repGrp, nMajorType);

      asct.mType = QSM::IEnhancedStreamSwitchManager::AdaptationSetChangeType::ADD;
    }
    else if (pDASMElem->GetOldRepGrpInfoKey() != MAX_UINT64_VAL)
    {
      nRemoveKey = pDASMElem->GetOldRepGrpInfoKey();

      RepresentationGroup repGrp;
      GetRepresentationGroupFromGrpKey(nRemoveKey, repGrp);
      GetGroupMajorType(repGrp, nMajorType);

      asct.mType = QSM::IEnhancedStreamSwitchManager::AdaptationSetChangeType::REMOVE;
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Nothing to do for adaptationset change");
      rslt = true;
    }

    if (MAX_UINT64_VAL != nAddOrReplaceGrpKey ||
        MAX_UINT64_VAL != nRemoveKey)
    {
      if (MAX_UINT64_VAL != nAddOrReplaceGrpKey)
      {
        RepresentationGroup newRepGrp;
        rslt = GetRepresentationGroupFromGrpKey(nAddOrReplaceGrpKey, newRepGrp);

        if (rslt)
        {
          newRepGrp.m_bEstimator = m_bEstimator;

          DASHMediaPlayGroup *pNewPlayGroup = QTV_New(DASHMediaPlayGroup);
          if (pNewPlayGroup)
          {
            pNewPlayGroup->Init(m_nNextKey, // m_nKey
                                nMajorType,
                                newRepGrp, &m_sDASHSessionInfo,
                                (iPeriodNotifier *)this, m_pScheduler);

            cInfo.m_nKey = m_nNextKey;
            cInfo.m_eMajorType = pNewPlayGroup->m_nMajorType;
            cInfo.m_bSwitchable = pNewPlayGroup->IsSwitchable();

            if (pNewPlayGroup->IsSwitchable())
            {
              PopulateBinningThresholdsForGroup(*pNewPlayGroup, cInfo);
            }

            RepGroupQElem *pElem = QTV_New_Args(RepGroupQElem, (pNewPlayGroup));

            if (pElem)
            {
              QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "AdaptationSet Change periodKey %llu adaptationSetKey %llu on repgrp idx %u - pending QSM notification",
                GetPeriodKey(), pNewPlayGroup->GetKey(), nMajorType);
              RepGroupQ& rRepGroupQ = m_RepGroupQ[nMajorType];
              rslt = rRepGroupQ.Push(pElem);
              rRepGroupQ.Print();

              if (rslt)
              {
                ++m_nNextKey;
              }
              else
              {
                QTV_Delete(pElem);
                QTV_Delete(pNewPlayGroup);
              }
            }
            else
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                "Failed to allocate RepGroupQElem for majorType %u", nMajorType);
              QTV_Delete(pNewPlayGroup);
            }
          }
          else
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "Failed to allocate DMPG for majorType %u", nMajorType);
          }
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "AdapationSetChange: failed to get RepresentationGroup for key %llu",
            nAddOrReplaceGrpKey);
        }
      }
      else if (MAX_UINT64_VAL != nRemoveKey)
      {
        IPStreamList<uint64> dmpgRemoveList;
        GetPlayGrpByRepresentationInfoKey(nRemoveKey, dmpgRemoveList);
        if (!dmpgRemoveList.IsEmpty())
        {
          uint64 nRemovePlayGrpKey = MAX_UINT64_VAL;
          dmpgRemoveList.PeekTail(nRemovePlayGrpKey);

          cInfo.m_nKey = nRemovePlayGrpKey;
          DASHMediaPlayGroup *pPlayGrp = GetPlayGrpByKey(cInfo.m_nKey);
          if (pPlayGrp)
          {
            cInfo.m_eMajorType = pPlayGrp->m_nMajorType;
            cInfo.m_bSwitchable = pPlayGrp->IsSwitchable();
            rslt = true;

            if (pPlayGrp->IsSwitchable())
            {
              PopulateBinningThresholdsForGroup(*pPlayGrp, cInfo);
            }
          }
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Unexpected adaptationset change error HandleAdaptationSetChangeQsmCmd");
      }

      asct.mAsInfo = cInfo;

      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "Adaptationsetchange: invoked on qsm for period with key %llu", GetPeriodKey());

      static uint32 nTid = 0;
      QSM::QsmStatus qsmStatus = m_pQSM->AdaptationSetChangeRequest(nTid, asct);
      ++nTid;

      QTV_MSG_PRIO5(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "AdaptationSetChangeRequest: qsmStatus %d, rslt %d, isQsmSuspended %d, periodKey 0x%x%x",
        qsmStatus, rslt, m_bIsQSMSuspended, (unsigned int)(GetPeriodKey() >> 32), (unsigned int)GetPeriodKey());

      if (true == rslt)
      {
        if (QSM::QSM_STATUS_OK == qsmStatus)
        {
          if (m_bIsQSMSuspended)
          {
            rslt = ResumeFromSuspendedState();
          }
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "Failed to queue AdaptationSetChangeRequest on QSM qsmstatus %d", qsmStatus);
          rslt = false;
        }
      }
    }
  }
  else
  {
    // nothing to do
    rslt = true;
  }
  MM_CriticalSection_Leave(m_pPeriodDataLock);

  return rslt;
}

/**
 * Queue the QSM notification for the adaptation set change cmd.
 */
QSM::IStreamSourceStatus DASHMediaPeriodHandler::AdaptationSetChangeResponse(
     uint32 tid, QSM::IStreamSource::AdaptationSetChangeStatus s)
{
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "AdaptationSetChangeResponse tid %u, adapsetChangeStatus %d",
                tid, (int)s);

  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  PeriodCmdData cmd;
  cmd.sAdaptationSetChangeData.eCmd = PERIOD_CMD_ADAPTATION_SET_CHANGE_NOTIFICATION;
  cmd.sAdaptationSetChangeData.nTid = tid;
  cmd.sAdaptationSetChangeData.nChangeStatus = s;

  if (!m_cCmdQ.EnQ(cmd))
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Period [0x%02x]: AdaptationSetChangeResponse cmd cannot be queued",
                   (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
  }

  return (status == HTTPCommon::HTTPDL_WAITING ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
}

/**
 * Process the Notification from QSM
 */
void DASHMediaPeriodHandler::HandleAdaptationSetChangeNotification(
   uint32 /* nTid */,
   QSM::IStreamSource::AdaptationSetChangeStatus nChangeStatus,
   RepGroupQElem::AdaptationSetChangeType& eChangeType)
{
  eChangeType = RepGroupQElem::NONE;

  uint64 nAddOrReplaceKey = MAX_UINT64_VAL;
  bool bTextTrackRemoveFailed = false;

  MM_CriticalSection_Enter(m_pPeriodDataLock);

  DASMQElem *pDASMElem = m_DASMQ.PeekHeadElem();
  if (pDASMElem)
  {
    if (pDASMElem->GetNewRepGrpInfoKey() != MAX_UINT64_VAL &&
        pDASMElem->GetOldRepGrpInfoKey() == MAX_UINT64_VAL)
    {
      eChangeType = RepGroupQElem::ADD;
    }

    if (pDASMElem->GetNewRepGrpInfoKey() == MAX_UINT64_VAL &&
        pDASMElem->GetOldRepGrpInfoKey() != MAX_UINT64_VAL)
    {
      eChangeType = RepGroupQElem::REMOVE;
    }

    if (pDASMElem->GetNewRepGrpInfoKey() != MAX_UINT64_VAL &&
        pDASMElem->GetOldRepGrpInfoKey() != MAX_UINT64_VAL)
    {
      eChangeType = RepGroupQElem::REPLACE;
    }

    if (QSM::IStreamSource::ADAPTATION_SET_CHANGE_REQ_SUCCESS == nChangeStatus)
    {
      if (pDASMElem->GetNewRepGrpInfoKey() != MAX_UINT64_VAL)
      {
        if (pDASMElem->GetOldRepGrpInfoKey() == MAX_UINT64_VAL)
        {
          // ADD
          nAddOrReplaceKey = pDASMElem->GetNewRepGrpInfoKey();
        }
        else
        {
          // REPLACE
          nAddOrReplaceKey = pDASMElem->GetNewRepGrpInfoKey();
        }

        for (int i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
        {
          RepGroupQ& rQ = m_RepGroupQ[i];
          if (!rQ.IsEmpty())
          {
            RepGroupQElem *pElem = rQ.PeekTail();

            if (pElem)
            {
              if (false == pElem->IsCommitted())
              {
                if (RepGroupQElem::ADD == eChangeType)
                {
                  pElem->MarkChangeType(RepGroupQElem::ADD);

                  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "AdaptationSetChange notify switch when period with key %llu and "
                    "playgrp(ADD) with key %llu becomes readable on Q with idx %d",
                    GetPeriodKey(), pElem->m_pRepGroup->GetKey(), i);
                }
                else
                {
                  pElem->MarkChangeType(RepGroupQElem::REPLACE);

                  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "AdaptationSetChange period with key %llu and playgrp(REPLACE) "
                    "with key %llu will notify switch later along read path on Q with idx %d",
                    GetPeriodKey(), pElem->m_pRepGroup->GetKey(), i);
                }

                pElem->Commit();
              }
            }
          }
        }
      }
      else if (pDASMElem->GetOldRepGrpInfoKey() != MAX_UINT64_VAL)
      {
        // Mark all the adaptation sets associated with the same RepresentationGroup
        // key as removed.
        uint64 nRemoveKey = pDASMElem->GetOldRepGrpInfoKey();

        for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
        {
          m_RepGroupQ[i].MarkRemoved(nRemoveKey);
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Unexpected error in adaptationset change both old and new keys invalid");
      }
    }
    else
    {
      // Rollback the adaptation-set change. This will cause taskSelectRepresentations
      // to exit as there is no pending adaptation-set change.
      uint32 nPeriodXMLKey = (uint32)((GetPeriodKey() & MPD_PERIOD_MASK) >> MPD_PERIOD_SHIFT_COUNT);
      uint32 nOldAdapXMLKey = (uint32)((pDASMElem->GetOldRepGrpInfoKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
      uint32 nNewAdapXMLKey = (uint32)((pDASMElem->GetNewRepGrpInfoKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
      for (int i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
      {
        RepGroupQ& rQ = m_RepGroupQ[i];
        if (!rQ.IsEmpty())
        {
          if (pDASMElem->GetNewRepGrpInfoKey() != MAX_UINT64_VAL)
          {
            // add or replace
            if (rQ.RemoveLastElementFromQ())
            {
              IPStreamList<uint32> nRepInfoKeyList; // empty list

              if (pDASMElem->GetOldRepGrpInfoKey() != MAX_UINT64_VAL)
              {
                // REPLACE
                m_sDASHSessionInfo.cMPDParser.ModifySelectedRepresentations(
                   nPeriodXMLKey, nOldAdapXMLKey, nRepInfoKeyList, true);
              }

              m_sDASHSessionInfo.cMPDParser.ModifySelectedRepresentations(
                 nPeriodXMLKey, nNewAdapXMLKey, nRepInfoKeyList, false);
            }

            break;
          }
          else if (pDASMElem->GetOldRepGrpInfoKey() != MAX_UINT64_VAL)
          {
            uint64 nRemoveKey = pDASMElem->GetOldRepGrpInfoKey();
            RepGroupQElem *pRemoveElem = m_RepGroupQ[i].GetLastCommittedElem();
            if (pRemoveElem)
            {
              DASHMediaPlayGroup *pRemovePlayGrp = pRemoveElem->m_pRepGroup;

              if (pRemovePlayGrp)
              {
                if (pRemovePlayGrp->GetRepGrpInfoKey() == nRemoveKey)
                {
                  pRemoveElem->MarkChangeType(RepGroupQElem::NONE);

                  IPStreamList<uint32> nRepInfoKeyList; // empty list

                  m_sDASHSessionInfo.cMPDParser.ModifySelectedRepresentations(
                     nPeriodXMLKey, nOldAdapXMLKey, nRepInfoKeyList, true);

                  if (QSM::MAJOR_TYPE_TEXT == i)
                  {
                    // removal for text track failed. In general, remove should never fail.
                    // As the order of elements in the DASMQ is such that text (if exists)
                    // is the last, then in case there is any following this remove would be
                    // an add of text corresponding to a text replace which got mapped to
                    // text remove-add.
                    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                                 "Failed to remove text track");

                    bTextTrackRemoveFailed = true;
                  }
                  break;
                }
              }
            }
          }
          else
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Unexpected error in adaptationset change on rollback");
          }
        }
      }
    }

    m_DASMQ.Pop();
    QTV_Delete(pDASMElem);

    if (bTextTrackRemoveFailed)
    {
      if (!m_DASMQ.IsEmpty())
      {
        pDASMElem = m_DASMQ.PeekHeadElem();

        if (pDASMElem)
        {
          if (pDASMElem->GetMajorType() == QSM::MAJOR_TYPE_TEXT)
          {
            // this element must be an ADD for text.
            m_DASMQ.Pop();
            QTV_Delete(pDASMElem);
          }
        }
      }
    }
  }

  if (!m_DASMQ.IsEmpty())
  {
    SelectAdaptationSet();
  }

  MM_CriticalSection_Leave(m_pPeriodDataLock);
}

/** @brief Update the buffer preroll values to QSM
  *
  * @return
  * None
  */
bool DASHMediaPeriodHandler::PeriodBaseStateHandler::UpdateBufferPrerollValues()
{
  bool status = false;

  if(m_pPeriodHandler->m_pQSM != NULL)
  {
    uint32 nStartupVal = 0;
    uint32 nRebufVal   = 0;
    nStartupVal = m_pPeriodHandler->m_sDASHSessionInfo.sSessionInfo.GetInitialPreroll();
    nRebufVal   = m_pPeriodHandler->m_sDASHSessionInfo.sSessionInfo.GetRebufferPreroll();
    if(QSM::QSM_STATUS_OK == m_pPeriodHandler->m_pQSM->UpdateBufferPrerollValues(
                                                       nStartupVal, nRebufVal))
    {
      status = true;
    }
  }

  return status;
}

/** @brief Stores the history of QSM
  *
  * @param[out] history - Buffer pointer to store history
  * @param[out] size - size of buffer pointer
  * @return
  * true
  */
bool DASHMediaPeriodHandler::PeriodBaseStateHandler::StoreQsmHistory
(
 uint8* history,
 uint32& size
)
{
  bool status = false;

  if(m_pPeriodHandler->m_pQSM != NULL)
  {
    if(0 == history)
    {
      size = 0;
      size = m_pPeriodHandler->m_pQSM->GetStateSize();
      status = true;
    }
    else
    {
      if(QSM::QSM_STATUS_OK == m_pPeriodHandler->m_pQSM->RetrieveHistory(history, size))
      {
        status = true;
      }
    }
  }

  return status;
}

/** @brief Returns true if all data is downloaded on period.
  *
  * @param[] void
  * @return
  * true - Period is completely downloaded
  * false - download is going on
  */
bool DASHMediaPeriodHandler::PeriodBaseStateHandler::IsEndOfPeriod()
{
  bool bEOF = true;
  //Query all playgroups to check if download is complete

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  if (m_pPeriodHandler->IsAdaptationSetChangePending())
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "Overlay endofperiod for period %llu to false as adapset pending",
      m_pPeriodHandler->GetPeriodKey());
    bEOF = false;
  }
  else
  {
    for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX ; i++)
    {
      // check if download has completed on all playgroups.
      RepGroupQ& rRepGrpDownloadQ =
        m_pPeriodHandler->m_RepGroupQ[i];

      bEOF = rRepGrpDownloadQ.IsEndOfPeriod();

      if(!bEOF)
      {
        break;
      }
    }
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  return bEOF;
}
/** @brief Close period (valid in ALL states).
  *
  * @return status of operation
  */
HTTPDownloadStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::Close()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  PeriodCmdData cmd;
  void* pIt = NULL;
  uint32 nKey = (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56);

  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "Period [0x%02x]: Closing", nKey );

  //Move to CLOSING to block all calls from QSM and DA!
  m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cClosingStateHandler);

  //Stop QSM (so that no new cmds are issued)
  if (m_pPeriodHandler->m_pQSM)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Period [0x%02x]: QSM STOP called", nKey );

    if (QSM::QSM_STATUS_OK == m_pPeriodHandler->m_pQSM->Stop())
    {
      status = HTTPCommon::HTTPDL_WAITING;
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "Failed to queue QSM stop on period %p", m_pPeriodHandler);
      m_pPeriodHandler->m_bDeleteQsm = true;
    }
  }

  //Clear cmd Q
  while(m_pPeriodHandler->m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_OPEN)
    {
      //Notify failure
      if (m_pPeriodHandler->m_pDANotifier)
      {
        m_pPeriodHandler->m_pDANotifier->NotifyEvent(m_pPeriodHandler->GetPeriodKey(),
                                                     PERIOD_CMD_OPEN, HTTPCommon::HTTPDL_ERROR_ABORT,
                                                     cmd.sOpenCmdData.pUserData);
      }
    }
    else if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_SEEK)
    {
      //Notify failure
      if (m_pPeriodHandler->m_pDANotifier)
      {
        m_pPeriodHandler->m_pDANotifier->NotifyEvent(m_pPeriodHandler->GetPeriodKey(),
                                                     PERIOD_CMD_SEEK, HTTPCommon::HTTPDL_ERROR_ABORT,
                                                     cmd.sSeekCmdData.pUserData);
      }
    }

    //Remove the period cmd
    (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
  }

  (void)m_pPeriodHandler->m_cCmdQ.Reset();
  m_pPeriodHandler->m_bPauseCmdProcessing = false;
  m_pPeriodHandler->m_nSeekTime = -1;
  m_pPeriodHandler->m_nSeekTimeRelative = -1;

  return status;
}

/**
 * @brief
 *  Request for number of data units complete. When all
 *  playgroups for this period are downloaded, informs
 *  DASHAdaptor.
 */
void DASHMediaPeriodHandler::PeriodBaseStateHandler::SegInfoReady(
  const uint64 nGroupKey,
  const uint64 nRepKey,
  const uint64 nStartTime,
  const uint64 nDuration,
  const uint32 nNumDataUnits,
  const HTTPDownloadStatus eStatus)
{
  if (m_pPeriodHandler)
  {
    uint64 startTime = nStartTime + m_pPeriodHandler->GetPeriodStartTime();

    if (m_pPeriodHandler->m_pQSM)
    {
      QSM::IStreamSourceStatus eQsmStatus;
      switch (eStatus)
      {
      case HTTPCommon::HTTPDL_DATA_END:
        eQsmStatus = QSM::SS_STATUS_NO_MORE_DATA;
      break;
      case HTTPCommon::HTTPDL_SUCCESS:
        eQsmStatus = QSM::SS_STATUS_OK;
      break;
       case HTTPCommon::HTTPDL_TIMEOUT:
        eQsmStatus = QSM::SS_STATUS_TIME_SHIFTED_DATA;
      break;
       case HTTPCommon::HTTPDL_OUT_OF_MEMORY:
        eQsmStatus = QSM::SS_STATUS_INSUFF_BUFF;
      break;
      case HTTPCommon::HTTPDL_INVALID_REPRESENTATION:
        eQsmStatus = QSM::SS_STATUS_INVALID_REPRESENTATION;
        break;
      case HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND:
        eQsmStatus = QSM::SS_STATUS_DATA_NOT_AVAILABLE;
        break;
      default:
        eQsmStatus = QSM::SS_STATUS_FAILURE;
      }

      QTV_MSG_PRIO7(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "SegInfoReady: PH %p, grKey '%llu', repKey %llu, startTime %u, duration %u, numUnits %u, qStat %d",
        (void *)m_pPeriodHandler, nGroupKey, nRepKey, (uint32)startTime,
       (uint32)nDuration, (uint32)nNumDataUnits, (int)eQsmStatus);

      m_pPeriodHandler->m_pQSM->DataUnitsInfoAvaliable(nGroupKey, nRepKey, startTime,
                                                       nDuration, nNumDataUnits,
                                                       eQsmStatus);
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Null periodHandler");
  }
}

void DASHMediaPeriodHandler::PeriodBaseStateHandler::SegDataReady(
  const uint64 nGroupKey,
  const uint64 nRepKey,
  const uint64 nDataUnitKey,
  const HTTPDownloadStatus eStatus)
{
  if (m_pPeriodHandler)
  {
    if (m_pPeriodHandler->m_pQSM)
    {
      QSM::IEnhancedStreamSwitchManager::DataUnitDownloadCompletionStatus status;

      switch (eStatus)
      {
      case HTTPCommon::HTTPDL_SUCCESS:
      case HTTPCommon::HTTPDL_EXISTS:
        status = QSM::IEnhancedStreamSwitchManager::DOWNLOAD_SUCCESS;
      break;
      case HTTPCommon::HTTPDL_INTERRUPTED:
        status = QSM::IEnhancedStreamSwitchManager::DOWNLOAD_CANCELLED;
      break;
      case HTTPCommon::HTTPDL_OUT_OF_MEMORY:
        status = QSM::IEnhancedStreamSwitchManager::DOWNLOAD_MEMORY_FAILURE;
      break;
      case HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND:
        status = QSM::IEnhancedStreamSwitchManager::DOWNLOAD_FAILED_DATA_NOT_AVAILABLE;
        break;
      default:
        status = QSM::IEnhancedStreamSwitchManager::DOWNLOAD_FAILED;
      break;
      }

      QTV_MSG_PRIO5(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "SegDataReady: DataUnitDownloadCompletionStatus status %d,"
                    "grp %d, rep %d, dataUnit [%d,%d]",
                    (int)status, (int)nGroupKey, (int)nRepKey, (int)(nDataUnitKey >> 32), (int)nDataUnitKey);
      m_pPeriodHandler->m_pQSM->DownloadDataUnitDone(nGroupKey, nRepKey, nDataUnitKey, status);
    }
  }
}

void DASHMediaPeriodHandler::PeriodBaseStateHandler::NotifyDownloadTooSlow(
  const uint64 nGroupKey, const uint32 nRepKey, const uint64 nDataUnitKey)
{
  if (m_pPeriodHandler)
  {
    if (m_pPeriodHandler->m_pQSM)
    {
      QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "NotifyDownloadTooSlow to QSM for (%d,%d,(%d,%d)",
                    (int)nGroupKey, (int)nRepKey, (int)(nDataUnitKey >> 32), (int)nDataUnitKey);
      m_pPeriodHandler->m_pQSM->DataUnitDownloadProgressInd(
        nGroupKey, nRepKey, nDataUnitKey,  QSM::IEnhancedStreamSwitchManager::DOWNLOAD_TOO_SLOW);
    }
  }
}

/** @brief Get Global Playback stats (valid in all states).
  *
  * @param[out] nTime  - Global playback time
  * @param[out] nOcc  - Global Occupancy
  * @return
  * QSM::SS_STATUS_OK - Success
  */
QSM::IStreamSourceStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::GetGlobalPlaybackStats
(
 uint64& nTime,
 uint64& nOcc
)
{
  nTime = 0;
  nOcc =  0;

  uint64 nDwldPos = MAX_UINT64;
  bool bOk = m_pPeriodHandler->m_pDAInfoQuery->GetGlobalPlayBackTime(nTime);

  if(bOk)
  {
    DASHMediaPlayGroup* pPlayGroup = NULL;
    HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE,
                                             HTTPCommon::HTTP_TEXT_TYPE};

    MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

    for (int i = 0; i < STD_ARRAY_SIZE(mediaType); i++)
    {
      pPlayGroup = m_pPeriodHandler->GetFirstPlayGroupInUse(mediaType[i]);
      if (pPlayGroup)
      {
        uint64 nPbPosition = 0;
        uint32 nFullyDowldOcc = 0;
        uint32 nPartiallyDwldOcc = 0;
        uint32 nTotalOcc = 0;
        uint32 nFMA = 0;
        if(true == GetMediaDurationBuffered(mediaType[i],
                                            nFullyDowldOcc,
                                            nPartiallyDwldOcc,
                                            nTotalOcc,
                                            nPbPosition,
                                            nFMA))
        {
          uint64 nGrpOcc = nPbPosition + uint64(nFullyDowldOcc + nPartiallyDwldOcc);
          nDwldPos = STD_MIN(nDwldPos, nGrpOcc);
        }
      }
    }

    MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);
  }

  if(bOk)
  {
    if(nDwldPos < MAX_UINT64)
    {
      nOcc = ((nDwldPos > nTime) ? (nDwldPos - nTime) : 0);
    }
    else
    {
      bOk = false;
    }
  }

  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "GetGlobalPlaybackStats isOk %d, downloadpos %llu, nOcc %llu", bOk, nTime, nOcc);

  return (bOk ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
}

QSM::IStreamSourceStatus DASHMediaPeriodHandler::PeriodSeekingStateHandler::GetGlobalPlaybackStats
(
 uint64& nTime,
 uint64& nOcc
)
{
  nTime = 0;
  nOcc =  0;

  uint64 nDwldPos = MAX_UINT64;
  //We first do a seek on video followed by a seek on audio and text. So only
  //playback position of video will be updated in case RAP is not present.
  //Playback position of audio and text will remain same as seektime. When
  //GetGlobalPlaybackStats query for playback position and return min on all
  //mediatypes it will always be audio or text and downloads on video will
  //stop as high watermark will reach. So in seeking state always return global occupancy
  //wrt video playback position.
  HTTPCommon::HTTPMediaType majorType = HTTPCommon::HTTP_UNKNOWN_TYPE;

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  DASHMediaPlayGroup* pPlayGroup = m_pPeriodHandler->GetPlayGroup(HTTPCommon::HTTP_VIDEO_TYPE);
  if(pPlayGroup)
  {
    majorType = HTTPCommon::HTTP_VIDEO_TYPE;
  }
  bool bOk = m_pPeriodHandler->GetCurrentPlaybackPosition(majorType,nTime);

  if(bOk)
  {
    DASHMediaPlayGroup* pPlayGroup = NULL;
    HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE,
                                             HTTPCommon::HTTP_TEXT_TYPE};
    for (int i = 0; i < STD_ARRAY_SIZE(mediaType); i++)
    {
      pPlayGroup = m_pPeriodHandler->GetPlayGroup(mediaType[i]);
      if (pPlayGroup)
      {
        uint64 nPbPosition = 0;
        uint32 nFullyDowldOcc = 0;
        uint32 nPartiallyDwldOcc = 0;
        uint32 nTotalOcc = 0;
        uint32 nFMA = 0;
        if(true == m_pPeriodHandler->GetMediaDurationBuffered(mediaType[i],
                                                              nFullyDowldOcc,
                                                              nPartiallyDwldOcc,
                                                              nTotalOcc,
                                                              nPbPosition,
                                                              nFMA))
        {
          uint64 nGrpOcc = nPbPosition + uint64(nTotalOcc - nPartiallyDwldOcc);
          nDwldPos = STD_MIN(nDwldPos, nGrpOcc);
        }
        else
        {
          bOk = false;
          break;
        }
      }
    }
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  if(bOk)
  {
    if(nDwldPos < MAX_UINT64)
    {
      nOcc = ((nDwldPos > nTime) ? (nDwldPos - nTime) : 0);
    }
    else
    {
      bOk = false;
    }
  }

  return (bOk ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
}

/** @brief Get group information (valid in all states).
  *
  * @param[in] pGroupInfo - Reference to group info array
  * @param[in] nSizeOfGroupInfo - Size of group info array
  * @param[out] nNumGroupInfo - Number of elements filled/needed
  * @return
  * QSM::SS_STATUS_OK - Success
  * QSM::SS_STATUS_INSUFF_BUFF - Insufficient buffer
  */
QSM::IStreamSourceStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::GetGroupInfo
(
 QSM::CGroupInfo *pGroupInfo,
 uint32 nSizeOfGroupInfo,
 uint32 &nNumGroupInfo
)
{
  QSM::IStreamSourceStatus status = QSM::SS_STATUS_OK;

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);
  nNumGroupInfo = m_pPeriodHandler->GetNumGroups();

  if (pGroupInfo && nSizeOfGroupInfo >= nNumGroupInfo)
  {
    int groupArrayIdx = 0;

    //Get info about all groups
    for (uint32 i= 0; i <= QSM::MAJOR_TYPE_MAX; i++)
    {
      RepGroupQElem *pLastElem =
        m_pPeriodHandler->m_RepGroupQ[i].GetLastCommittedElem();

      if (pLastElem)
      {
        DASHMediaPlayGroup *pPlayGroup = pLastElem->m_pRepGroup;

        if (pPlayGroup)
        {
          pGroupInfo[groupArrayIdx].m_nKey = pPlayGroup->GetKey();
          pGroupInfo[groupArrayIdx].m_eMajorType = pPlayGroup->m_nMajorType;

          //Get minor type if A or V only group (superfluous for QSM since present at rep level!)
          if (pGroupInfo[groupArrayIdx].m_eMajorType == QSM::MAJOR_TYPE_AUDIO ||
              pGroupInfo[groupArrayIdx].m_eMajorType == QSM::MAJOR_TYPE_VIDEO)
          {
            int nNumCodecs = 1;
            CodecInfo sCodecInfo;

            (void)pPlayGroup->m_cRepGroupInfo.getCodec(&sCodecInfo, nNumCodecs);
            switch(sCodecInfo.minorType)
            {
            case MN_TYPE_AVC:
              pGroupInfo[groupArrayIdx].m_eMinorType = QSM::MINOR_TYPE_H264;
              break;
            case MN_TYPE_HVC:
              pGroupInfo[groupArrayIdx].m_eMinorType = QSM::MINOR_TYPE_HEVC;
              break;
            case MN_TYPE_AAC_LC:
            case MN_TYPE_HE_AAC:
              pGroupInfo[groupArrayIdx].m_eMinorType = QSM::MINOR_TYPE_AAC;
              break;
            default:
              pGroupInfo[groupArrayIdx].m_eMinorType = QSM::MINOR_TYPE_UNKNOWN;
            }
          }

          pGroupInfo[groupArrayIdx].m_bDataUnitsAligned =
            pPlayGroup->m_cRepGroupInfo.IsSubSegmentAlignment();
          if (pGroupInfo[groupArrayIdx].m_bDataUnitsAligned)
          {
            pGroupInfo[groupArrayIdx].m_bDataUnitsStartWithRap = true;
          }
          pGroupInfo[groupArrayIdx].m_bMandatory = (pPlayGroup->m_nMajorType & QSM::MAJOR_TYPE_AUDIO);
          pGroupInfo[groupArrayIdx].m_bSwitchable = pPlayGroup->IsSwitchable();
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "GetGroupInfo: Key %llu, IsSwitchable %d", pPlayGroup->GetKey(), pPlayGroup->IsSwitchable());
          pGroupInfo[groupArrayIdx].m_nQuality = 1.0; //quality distribution among groups, 1.0 is fair distribution
          pGroupInfo[groupArrayIdx].m_bRAPInfoAvailable = true;
          pGroupInfo[groupArrayIdx].m_nSwitchBuffReq = 0;

          if (pPlayGroup->IsSwitchable())
          {
            m_pPeriodHandler->PopulateBinningThresholdsForGroup(*pPlayGroup, pGroupInfo[groupArrayIdx]);
          }

          ++groupArrayIdx;
        }
      }

    }

    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Period [0x%02x]: Group info for %u groups obtained successfully",
                   (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                   nNumGroupInfo );
  }
  else
  {
    status = QSM::SS_STATUS_INSUFF_BUFF;
    QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Period [0x%02x]: Insufficient buffer for group info %u/%u (actual/required)",
                   (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                   nSizeOfGroupInfo, nNumGroupInfo );
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  return status;
}

/** @brief Get representation information for given group (valid in all states).
  *
  * @param[in] nGroupKey - Group key
  * @param[in] pRepresentationInfo - Reference to rep info array
  * @param[in] nSizeOfRepresentationInfo - Size of rep info array
  * @param[out] nNumRepresentationInfo - Number of elements filled/needed
  * @return
  * QSM::SS_STATUS_OK - Success
  * QSM::SS_STATUS_INSUFF_BUFF - Insufficient buffer
  * QSM::SS_STATUS_FAILURE - Otherwise (e.g. bad index)
  */
QSM::IStreamSourceStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::GetRepresentationInfo
(
 uint64 nGroupKey,
 QSM::CRepresentationInfo *pRepresentationInfo,
 uint32 nSizeOfRepresentationInfo,
 uint32 &nNumRepresentationInfo
)
{
  QSM::IStreamSourceStatus status = QSM::SS_STATUS_FAILURE;
  uint32 nPerKey = (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56);

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  DASHMediaPlayGroup *pPlayGroup = m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);

  if (pPlayGroup)
  {
    if (pPlayGroup->IsValid())
    {
      //Get info about all reps for the given group
      switch(pPlayGroup->GetRepresentationInfo(pRepresentationInfo,
                                               nSizeOfRepresentationInfo,
                                               nNumRepresentationInfo))
      {
      case HTTPCommon::HTTPDL_SUCCESS:
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Period [0x%02x]: Representation info for %u reps in group %u obtained successfully",
                       nPerKey, nNumRepresentationInfo, (uint32)nGroupKey );

        //ToDo: Hack to update preroll for live switch case to work around a QSM limitation
      if (m_pPeriodHandler->m_sDASHSessionInfo.cMPDParser.IsLiveContentHostedLocally() &&
          (nNumRepresentationInfo == 1) && pPlayGroup->IsSwitchable())
        {
          m_pPeriodHandler->m_sDASHSessionInfo.sSessionInfo.SetEmbmsSession(true);
          m_pPeriodHandler->m_sDASHSessionInfo.sSessionInfo.SetInitialPreroll(HTTP_DEFAULT_LIVE_NOSWITCH_PREROLL_MSEC, false);
          m_pPeriodHandler->m_sDASHSessionInfo.sSessionInfo.SetRebufferPreroll(HTTP_DEFAULT_LIVE_NOSWITCH_PREROLL_MSEC, false);
          (void)UpdateBufferPrerollValues();
        }
        status = QSM::SS_STATUS_OK;
        break;
      case HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER:
        status = QSM::SS_STATUS_INSUFF_BUFF;
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Period [0x%02x]: Insufficient buffer for rep info %u/%u (actual/required)",
                       nPerKey, nSizeOfRepresentationInfo, nNumRepresentationInfo );
        break;
      default:
        status = QSM::SS_STATUS_FAILURE;
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Period [0x%02x]: Could not get rep info for group %u", nPerKey, (uint32)nGroupKey );
      }
    }
  }
  else
  {
    status = QSM::SS_STATUS_FAILURE;
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Period [0x%02x]: Invalid group/ group key %u (actual)",
                   nPerKey, (uint32)nGroupKey);
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  return status;
}

/** @brief Get Data unit download info.
  *
  * @param[in] pDownloadInfo - Reference to QSM::CDataUnitDownloadInfo array
  * @param[in] nSize         - Size of QSM::CDataUnitDownloadInfo array
  * @param[out] nFilled      - Number of elements filled
  * @param[in] nStartTime    - Time starting from which info is requested
  * @return
  * QSM::SS_STATUS_OK - Success
  * QSM::SS_STATUS_INSUFF_BUFF - Insufficient buffer
  * QSM::SS_STATUS_FAILURE - In case of failures
  */
QSM::IStreamSourceStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::GetDataUnitDownloadInfo
(
  QSM::CDataUnitDownloadInfo *pDownloadInfo,
  uint32 nSize,
  uint32 &nFilled,
  uint64 nStartTime
)
{
  nFilled = 0;

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  if(nStartTime == USE_PLAYBACK_TIME_AS_START_TIME)
  {
    (void)m_pPeriodHandler->GetCurrentPlayBackTime(nStartTime);
  }
  if(nStartTime >= m_pPeriodHandler->m_nStartTime)
  {
    nStartTime = nStartTime - m_pPeriodHandler->GetPeriodStartTime();
  }

  /* pDownloadInfo is null but size is a valid value , then it is
     not permitted. As in that case there is no way to store the data */
  if(!pDownloadInfo && nSize)
    return QSM::SS_STATUS_FAILURE;

  QSM::IStreamSourceStatus status = QSM::SS_STATUS_OK;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  uint32 previousFilled = 0;
  QSM::CDataUnitDownloadInfo *pDownloadDataInfo = pDownloadInfo;

  /* Iterate for number of PlayGroup in this period to gather info */
  for (uint32 i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
  {
    RepGroupQ &rQ = m_pPeriodHandler->m_RepGroupQ[i];
    DASHMediaPlayGroup *pPlayGroup = NULL;

    if (!rQ.IsEmpty())
    {
      RepGroupQElem *pLastElem = rQ.GetLastCommittedElem();

      if (pLastElem)
      {
        pPlayGroup = pLastElem->m_pRepGroup;

        if (pPlayGroup)
        {
          previousFilled = nFilled;
          if(pPlayGroup->IsValid())
          {
            if(eStatus == HTTPCommon::HTTPDL_ERROR_ABORT)
            {
              eStatus = pPlayGroup->GetDataUnitDownloadInfo(pDownloadDataInfo,nSize,nFilled,nStartTime);

              if(eStatus != HTTPCommon::HTTPDL_SUCCESS)
              {
                if(eStatus == HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER)
                  status = QSM::SS_STATUS_INSUFF_BUFF;
                else if(eStatus == HTTPCommon::HTTPDL_ERROR_ABORT)
                  status = QSM::SS_STATUS_FAILURE;
              }
              else
                status = QSM::SS_STATUS_OK;
            }
            else
            {
              (void)pPlayGroup->GetDataUnitDownloadInfo(pDownloadDataInfo,nSize,nFilled,nStartTime);
            }
          }
        }
      }

      if(pPlayGroup &&
         (status == QSM::SS_STATUS_OK || status == QSM::SS_STATUS_INSUFF_BUFF))
      {
        /* For all newly added data, group key will be updated, as they belong to current play group */
        for(; previousFilled < nFilled && pDownloadDataInfo; previousFilled++)
        {
          pDownloadDataInfo[previousFilled].nGroupKey = pPlayGroup->GetKey();
        }
      }
    }
  }
  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  if(status == QSM::SS_STATUS_OK)
  {
    //Add period start time to all data units information
    for(uint32 i = 0; i < nFilled && pDownloadDataInfo; i++)
    {
      pDownloadDataInfo[i].nStartTime+= m_pPeriodHandler->GetPeriodStartTime();
    }
  }

  return status;
}

/** @brief Get Config Params.
  *
  * @param[out] mConfigParams - Reference to QSM::QsmConfigParams structure
  * @return
  * QSM::SS_STATUS_OK - Success
  */
QSM::IStreamSourceStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::GetQsmConfigParams
(
 QSM::QsmConfigParams& mConfigParams
)
{
  QSM::IStreamSourceStatus status = QSM::SS_STATUS_FAILURE;

  if(m_pPeriodHandler)
  {
    mConfigParams.nStartupPreroll   = m_pPeriodHandler->m_sDASHSessionInfo.sSessionInfo.GetInitialPreroll();
    mConfigParams.nRebuffPreroll    = m_pPeriodHandler->m_sDASHSessionInfo.sSessionInfo.GetRebufferPreroll();
    if(m_pPeriodHandler->m_sDASHSessionInfo.cMPDParser.IsLive())
    {
      mConfigParams.nMediaAvailWindow = (uint32)(m_pPeriodHandler->m_sDASHSessionInfo.cMPDParser.GetTimeShiftBufferDepth() * 1000);
    }
    else
    {
      mConfigParams.nMediaAvailWindow = MAX_UINT32;
    }

    uint32 heapSize = (uint32)m_pPeriodHandler->m_sDASHSessionInfo.cHeapManager.GetMaxAllocatableSpace();

    // Consider overhead due to Moov / Moof and Sidx in heap
    if(heapSize > HTTP_MAX_MEMORY_OVERHEAD)
    {
      heapSize -= HTTP_MAX_MEMORY_OVERHEAD;
    }

    if (m_pPeriodHandler->m_pDAInfoQuery)
    {
      uint32 maxBitRate = m_pPeriodHandler->m_pDAInfoQuery->GetMaxBitrateAcrossActivePeriods();

      //In case the mpd does not have 'Bandwidth' param across any of the reps return a default
      // value to QSM. QSM will work with only one rep in the group which it receives as part of GetRepresentationInfo() call.
      if(maxBitRate == 0)
      {
        maxBitRate = HTTP_MAX_REP_BANDWIDTH;
      }

      uint32 nMaxDuration = (uint32(((float)heapSize * 8) / (float)maxBitRate)) * 1000;
      mConfigParams.nMaxBufferDuration = nMaxDuration;

      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "GetMaxBitrateAcrossActivePeriods %d, maxBufferDuration %d",
        (int)maxBitRate, (int)nMaxDuration);
    }

    mConfigParams.nMaxResponseTime = 0;

    status = QSM::SS_STATUS_OK;
  }

  return status;
}

/** @brief Select representation (in OPENING state).
  *
  * @param[in] pSelection - Reference to selection
  * @param[in] nNumGroupRepresentationSelection - Size of selection array
  * @return
  * QSM::SS_STATUS_OK - Selection complete or queued
  * QSM::SS_STATUS_FAILURE - Selection failure
  */
QSM::IStreamSourceStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::SelectRepresentation
(
 QSM::GroupRepresentationSelection* pSelection,
 uint32 nNumGroupRepresentationSelection
)
{
  bool bOk = false;
  QSM::GroupRepresentationSelection* pSelectionStatus =
    QTV_New_Array(QSM::GroupRepresentationSelection, nNumGroupRepresentationSelection);

  if (pSelection && pSelectionStatus)
  {
    bOk = true;

    //Populate the selection status array
    for (uint32 i = 0; i < nNumGroupRepresentationSelection; i++)
    {
      pSelectionStatus[i].nGroupKey = pSelection[i].nGroupKey;
      pSelectionStatus[i].nRepresentationKey = pSelection[i].nRepresentationKey;

      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "Select key for grp %llu, rep %llu, data unit %llu",
        pSelectionStatus[i].nGroupKey, pSelectionStatus[i].nRepresentationKey, pSelection[i].nKey);

      pSelectionStatus[i].nKey = pSelection[i].nKey;
      pSelectionStatus[i].bSelectionSuccessful = false;
    }

    //Select specified rep in all groups

    MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

    for (uint32 i = 0; i < nNumGroupRepresentationSelection; i++)
    {
      HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
      DASHMediaPlayGroup *pPlayGroup =
        m_pPeriodHandler->GetPlayGrpByKey(pSelection[i].nGroupKey);

      if (pPlayGroup)
      {
        if(pPlayGroup->IsValid())
        {
          RepGroupQ &rQ = m_pPeriodHandler->m_RepGroupQ[pPlayGroup->GetQsmMajorType()];
          RepGroupQElem *pElem = rQ.GetElemByKey(pPlayGroup->GetKey());
          if (pElem)
          {
            if (!pElem->IsCommitted())
            {
              bOk = m_pPeriodHandler->ValidateAdaptationSetChangeSwitch(
                  pPlayGroup, pSelection[i].nRepresentationKey, pSelection[i].nKey);
              if (false == bOk)
              {
                break;
              }
            }
          }
          else
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
              "Failed to find RepGroupElement for playgrp with key %llu", pPlayGroup->GetKey());
          }

          if (bOk)
          {
            status =
              pPlayGroup->Select(pSelection[i].nRepresentationKey,
                                 pSelection[i].nKey);

            if (HTTPCommon::HTTPDL_SUCCESS == status)
            {
              m_pPeriodHandler->ClearBufferedData(*pPlayGroup, pPlayGroup->GetStartTime());
            }
          }
        }
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "SelectRepresentation failed. Failed to find group with key %llu",
          pSelection[i].nGroupKey);
      }

      QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Period [0x%02x]: Selection status %d for %u/%u/%u (group/rep/data unit)",
                     (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                     status, (uint32)pSelection[i].nGroupKey, (uint32)pSelection[i].nRepresentationKey,
                     (uint32)pSelection[i].nKey );

      if (status == HTTPCommon::HTTPDL_SUCCESS || status == HTTPCommon::HTTPDL_WAITING)
      {
        pSelectionStatus[i].bSelectionSuccessful = true;

#ifdef DASH_STATS
        //For now collecting switch points for video since only video is switchable!
        if (pPlayGroup)
        {
          if (m_pPeriodHandler->m_pStatsFile &&
              pPlayGroup->m_nMajorType & QSM::MAJOR_TYPE_VIDEO)
          {
            m_pPeriodHandler->m_nCurrentRepID = (int)pSelection[i].nRepresentationKey;
          }
        }
#endif /* DASH_STATS */
      }
      else
      {
        //Since only video is switchable, no need for unselecting all selected playgroups
        //if Select on one of the playgroups fails for now!
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
        bOk = false;
        break;
      }
    }

    MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

    //Notify any failure or success rightaway
    if (m_pPeriodHandler->m_pQSM && bOk)
    {
      m_pPeriodHandler->m_pQSM->SelectRepresentationDone(pSelectionStatus,
                                                         nNumGroupRepresentationSelection,
                                                         bOk ? QSM::SELREP_STATUS_OK : QSM::SELREP_STATUS_ERROR);
    }

    QTV_Delete_Array(pSelectionStatus);
    pSelectionStatus = NULL;
  }
  else
  {
    if(pSelectionStatus)
    {
      QTV_Delete_Array(pSelectionStatus);
      pSelectionStatus = NULL;
    }
  }

  return (bOk ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
}

/**
 * For SLCT of dynamic adaptation set change, ensure that the
 * SLCT time is not earlier than the current playtime of the
 * playgroup.
 */
bool DASHMediaPeriodHandler::ValidateAdaptationSetChangeSwitch(DASHMediaPlayGroup *pPlayGroup,
                                                               uint64 nRepresentationKey,
                                                               uint64 nDataUnitKey)
{
  bool bOk = true;

  // dasm: fail the SLCT if the SLCT time is ahead of the playtime
  // for the majorType.
  uint64 nPbPos = 0;
  if (pPlayGroup->GetDataUnitStartTime(nRepresentationKey, nDataUnitKey, nPbPos))
  {
    nPbPos += GetPeriodStartTime();

    HTTPCommon::HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
    switch (pPlayGroup->GetQsmMajorType())
    {
    case QSM::MAJOR_TYPE_AUDIO:
      mediaType = HTTPCommon::HTTP_AUDIO_TYPE;
      break;
    case QSM::MAJOR_TYPE_VIDEO:
      mediaType = HTTPCommon::HTTP_VIDEO_TYPE;
      break;
    case QSM::MAJOR_TYPE_TEXT:
      mediaType = HTTPCommon::HTTP_TEXT_TYPE;
      break;
    }

    if (mediaType != HTTPCommon::HTTP_UNKNOWN_TYPE)
    {
      if (pPlayGroup->GetQsmMajorType() != QSM::MAJOR_TYPE_TEXT)
      {
        // allow for overlapping segments in text
        if (m_LastSampleTS[mediaType] < MAX_UINT64_VAL &&
            m_LastSampleTS[mediaType] >= nPbPos )
        {
          QTV_MSG_PRIO6(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "Failed the select of DASM playgroup select with key 0x%x%x, rep %llu, dataUnit %llu, "
            "as GroupPbTime %llu >= SLCT time %llu",
            (unsigned int)(pPlayGroup->GetKey() >> 32), (unsigned int)(pPlayGroup->GetKey()),
            nRepresentationKey,nDataUnitKey, m_LastSampleTS[mediaType], nPbPos);
          bOk = false;
        }
      }
    }
    else
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Invalid majorType %d for DASM playgroup %llu, interleaved ?",
        mediaType, pPlayGroup->GetKey());
      bOk = false;
    }
  }
  else
  {
    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "ValidateAdaptationSetChangeSwitch switched for (%llu,%llu,%llu)",
                  pPlayGroup->GetKey(), nRepresentationKey, nDataUnitKey);
  }

  return bOk;
}

/**
 * Populate binning thresholds into CGroupInfo if the group is
 * switchable.
 */
void DASHMediaPeriodHandler::PopulateBinningThresholdsForGroup(
   const DASHMediaPlayGroup& rPlayGrp, QSM::CGroupInfo& rGroupInfo)
{
  if (rPlayGrp.IsSwitchable())
  {
    // ### TO DO ### Dynamically calculate from MPD
    if (rGroupInfo.m_nBinThresholds)
    {
      if (rPlayGrp.m_nMajorType == QSM::MAJOR_TYPE_AUDIO)
      {
        rGroupInfo.m_nNumThresholds = 1;

        if (rGroupInfo.m_nNumThresholds > 0)
        {
          rGroupInfo.m_nBinThresholds[0] = 192; // Kbps for now
        }
      }
      else if (rPlayGrp.m_nMajorType == QSM::MAJOR_TYPE_VIDEO)
      {
        rGroupInfo.m_nNumThresholds = 1;
        if (rGroupInfo.m_nNumThresholds > 0)
        {
          rGroupInfo.m_nBinThresholds[0] = 2560; // 2.5 Mbps for now
        }
      }
    }
  }
}

/**
 * Clear the buffered data for all playgrps which are associated
 * with mahorType.
 */
void DASHMediaPeriodHandler::ClearBufferedData(DASHMediaPlayGroup& rPlayGrp,
                                               uint64 nTime)
{
  MM_CriticalSection_Enter(m_pPeriodDataLock);

  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    RepGroupQ& rQ = m_RepGroupQ[i];
    if ((i & rPlayGrp.m_nMajorType) && !rQ.IsEmpty())
    {
      rQ.ClearBufferedData(nTime, rPlayGrp.GetKey()); // don't fluch data for this playgrp.
    }
  }

  MM_CriticalSection_Leave(m_pPeriodDataLock);
}

/**
 * @breif This Method indicates that all the requests in a group
 *        have been completed
 *
 * @param nGroupKey
 */
void DASHMediaPeriodHandler::PeriodBaseStateHandler::GroupRequestsCompleted
(
  uint64 nGroupKey
)
{
  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "GroupRequestsCompleted on period %p, repGrp 0x%x%x",
                m_pPeriodHandler, (unsigned int)(nGroupKey >> 32), (unsigned int)nGroupKey);
  //Notify group of period end (relative to period start)
  uint64 nRelPeriodEndTime =
    (uint64)m_pPeriodHandler->m_sDASHSessionInfo.cMPDParser.GetDuration(m_pPeriodHandler->GetPeriodKey());

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  if(nGroupKey == ALL_GROUPS )
  {
    for (uint32 i= 0; i <= QSM::MAJOR_TYPE_MAX; i++)
    {
      m_pPeriodHandler->m_RepGroupQ[i].GroupRequestsCompleted(nRelPeriodEndTime);
    }
  }
  else
  {
    DASHMediaPlayGroup *pPlayGroup =  m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);

    if (pPlayGroup)
    {
      pPlayGroup->GroupRequestsCompleted(nRelPeriodEndTime);
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "GroupRequestsCompleted Failed to find grp with key %llu", nGroupKey);
    }
  }
  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);
}

bool DASHMediaPeriodHandler::PeriodBaseStateHandler::IsEndOfMediaType(video::HTTPCommon::HTTPMediaType mediaType)
{
  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  DASHMediaPlayGroup* pPlayGroup = m_pPeriodHandler->GetPlayGroup(mediaType);
  bool result = false;
  if((pPlayGroup && pPlayGroup->IsDownloadComplete()) ||
     (!pPlayGroup))
  {
    if(m_pPeriodHandler->m_sDASHSessionInfo.cMPDParser.IsLastPeriod(
       m_pPeriodHandler->m_cPeriodInfo.getPeriodKey()))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "IsEndOfMediaType  returns true");
      result = true;
    }
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  return result;
}

/** @Setting last period in current period handler
  *
  * @return void
  */

void DASHMediaPeriodHandler::PeriodBaseStateHandler::SetLastPeriod()
{
  m_pPeriodHandler->m_cPeriodInfo.SetLastPeriod();
}

/** @brief Process all commands (in IDLE state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaPeriodHandler::PeriodIdleStateHandler::ProcessCmds()
{
  int status = 0;
  PeriodCmdData cmd;

  //Wait for OPEN cmd
  if (m_pPeriodHandler->m_cCmdQ.PeekHead(cmd))
  {
    if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_OPEN)
    {
      m_pPeriodHandler->m_nStartTime = cmd.sOpenCmdData.nStartTime;

      // set this specifically for calucluation of bufferedduration before playtime
      // for the mediaType is updated with the timestamp of the first sample.
      MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

      for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
      {
        RepGroupQElem *pElem = m_pPeriodHandler->m_RepGroupQ[i].GetLastCommittedElem();
        if (pElem)
        {
          DASHMediaPlayGroup *pPlayGroup = pElem->m_pRepGroup;
          if (pPlayGroup)
          {
            pPlayGroup->SetStartTime(
               m_pPeriodHandler->m_nStartTime - m_pPeriodHandler->GetPeriodStartTime());
          }
        }

      }

      for (uint32 i = 0; i < HTTPCommon::HTTP_MAX_TYPE; ++i)
      {
        m_pPeriodHandler->m_LastSampleTS[i] = MAX_UINT64_VAL;
      }

      MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

      //Start QSM and move to OPENING state. After posting play command on QSM wait
      //for the play response from QSM. Do not process any commands in between play
      //and play response as the commands might be of the time before the time passed
      //in this play request. Once we get receive play response from QSM start processing
      //the cmds again!!! There is a timing issue on target sometimes if we make the
      //flag true after posting the command on QSM. To avoid that make the flag true
      //first then post command
      m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cOpeningStateHandler);

      m_pPeriodHandler->m_bPauseCmdProcessing = true;
      if (m_pPeriodHandler->m_pQSM == NULL ||
          m_pPeriodHandler->m_pQSM->Play(m_pPeriodHandler->m_nStartTime ) != QSM::QSM_STATUS_OK)
      {

        //Notify failure and dequeue OPEN cmd
        m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cIdleStateHandler);
        if (m_pPeriodHandler->m_pDANotifier)
        {
          m_pPeriodHandler->m_pDANotifier->NotifyEvent(m_pPeriodHandler->GetPeriodKey(),
                                                       PERIOD_CMD_OPEN, HTTPCommon::HTTPDL_ERROR_ABORT,
                                                       cmd.sOpenCmdData.pUserData);
        }
        (void)m_pPeriodHandler->m_cCmdQ.DeQ(cmd);
      }
      else
      {
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Period [0x%02x]: QSM started (nStartTime %u), period moving to OPENING",
                       (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                       (uint32)m_pPeriodHandler->m_nStartTime );
      }
    }
    else if(cmd.sBaseCmdData.eCmd == PERIOD_CMD_SEEK)
    {

      HTTPDownloadStatus result =
        m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cSeekingStateHandler);
      if(result == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        m_pPeriodHandler->m_cCmdQ.DeQ(cmd);
      }
    }
  }

  return status;
}

/** @brief Queue up OPEN cmd from DASHAdaptor (valid ONLY in IDLE state).
  *
  * @param[in] nStartTime - Start time
  * @param[in] pCallbackData - Callback data
  * @return
  * HTTPDL_WAITING - OPEN cmd successfully queued and status notified later
  * HTTPDL_SUCCESS - OPEN completed successfully
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPeriodHandler::PeriodIdleStateHandler::Open
(
 const uint64 nStartTime,
 void* pCallbackData
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  PeriodCmdData cmd;
  cmd.sOpenCmdData.eCmd = PERIOD_CMD_OPEN;
  cmd.sOpenCmdData.pUserData = pCallbackData;
  cmd.sOpenCmdData.nStartTime = nStartTime;
  if (!m_pPeriodHandler->m_cCmdQ.EnQ(cmd))
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Period [0x%02x]: OPEN cmd cannot be queued",
                   (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
  }

  return status;
}

HTTPDownloadStatus DASHMediaPeriodHandler::PeriodIdleStateHandler::Close()
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "PeriodIdleStateHandler::Close Return SUCCESS");

  return HTTPCommon::HTTPDL_SUCCESS;
}

/** @brief Process all commands (in OPENING state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaPeriodHandler::PeriodOpeningStateHandler::ProcessCmds()
{
  int status = 0;
  PeriodCmdData cmd;
  void *pIt = NULL;
  while(m_pPeriodHandler->m_cCmdQ.Next(pIt, cmd))
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    //Process OPEN cmd
    if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_OPEN)
    {
      MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

      //Wait until all groups are ready (readable)
      for (uint32 i= 0; i < QSM::MAJOR_TYPE_MAX; i++)
      {
        RepGroupQElem *pElem = m_pPeriodHandler->m_RepGroupQ[i].GetLastCommittedElem();
        if (pElem)
        {
          DASHMediaPlayGroup *pPlayGroup = pElem->m_pRepGroup;
          if (pPlayGroup)
          {
            bool bValid = pPlayGroup->IsValid();
            if (bValid && pPlayGroup->IsReadable())
            {
              if(result != HTTPCommon::HTTPDL_WAITING)
              {
              result = HTTPCommon::HTTPDL_SUCCESS;
            }
            }
            else if(bValid)
            {
              result = HTTPCommon::HTTPDL_WAITING;
              continue;
            }
          }
        }
      }

      MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

      //If all groups are ready, notify success and dequeue OPEN cmd
      if (result != HTTPCommon::HTTPDL_WAITING)
      {
        if (m_pPeriodHandler->m_pDANotifier)
        {
          m_pPeriodHandler->m_pDANotifier->NotifyEvent(m_pPeriodHandler->GetPeriodKey(),
                                                       PERIOD_CMD_OPEN, result,
                                                       cmd.sOpenCmdData.pUserData);
        }

        if(result == HTTPCommon::HTTPDL_SUCCESS)
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Period [0x%02x]: All groups ready, period moving to OPEN",
                         (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
          m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cOpenStateHandler);
        }
        else
        {
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "Period [0x%02x]: Period open failed, closing the period",
                           (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
            m_pPeriodHandler->Close();
        }

        (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
        break;
      }
      else
      {
        // Dequeue the Open cmd
        void* pSeekIt = NULL;
        PeriodCmdData seekCmd;
        while(m_pPeriodHandler->m_cCmdQ.Next(pSeekIt, seekCmd))
        {
          if (seekCmd.sBaseCmdData.eCmd == PERIOD_CMD_SEEK)
          {
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                          "Period [0x%02x]: Open cmd dequeued as period moving from"
                          "opening to seeking",
                          (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56));
            (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
            break;
          }
        }
        //ToDo: Is there a timeout logic needed?
      }
    }
    else if(cmd.sBaseCmdData.eCmd == PERIOD_CMD_SEEK)
    {
      //Start seek, and dequeue the open cmd
      HTTPDownloadStatus result =
        m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cSeekingStateHandler);
      if(result == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
        break;
      }
    }
    else if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_ADAPTATION_SET_CHANGE)
    {
      m_pPeriodHandler->SelectAdaptationSet();
      (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
    }
    else if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_SUSPEND)
    {
      m_pPeriodHandler->ProcessSuspendQSM();
      (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
    }
  }

  return status;
}

/** @brief Process all commands (in OPEN state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaPeriodHandler::PeriodOpenStateHandler::ProcessCmds()
{
  int status = 0;
  PeriodCmdData cmd;

  void *pIt = NULL;

  while(m_pPeriodHandler->m_cCmdQ.Next(pIt, cmd))
  {
    if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_OPEN)
    {
      //Open is already complete, notify dash adaptor of open cmd status
      if (m_pPeriodHandler->m_pDANotifier)
      {
        m_pPeriodHandler->m_pDANotifier->NotifyEvent(m_pPeriodHandler->GetPeriodKey(),
                                                     PERIOD_CMD_OPEN, HTTPCommon::HTTPDL_SUCCESS,
                                                     cmd.sOpenCmdData.pUserData);
      }
      (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
    }
    else if(cmd.sBaseCmdData.eCmd == PERIOD_CMD_SEEK)
    {
      HTTPDownloadStatus result =
        m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cSeekingStateHandler);
      if(result == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        m_pPeriodHandler->m_cCmdQ.Remove(pIt);
      }
    }
    else if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_ADAPTATION_SET_CHANGE)
    {
      m_pPeriodHandler->SelectAdaptationSet();
      (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
    }
    else if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_ADAPTATION_SET_CHANGE_NOTIFICATION)
    {
      RepGroupQElem::AdaptationSetChangeType eChangeType = RepGroupQElem::NONE;

      m_pPeriodHandler->HandleAdaptationSetChangeNotification(
          cmd.sAdaptationSetChangeData.nTid,
          cmd.sAdaptationSetChangeData.nChangeStatus,
          eChangeType);

      HTTPDownloadStatus result =
        (QSM::IStreamSource::ADAPTATION_SET_CHANGE_REQ_SUCCESS == cmd.sAdaptationSetChangeData.nChangeStatus
         ? HTTPCommon::HTTPDL_SUCCESS : HTTPCommon::HTTPDL_ERROR_ABORT);

      (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
    }
    else if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_PURGE_ADAPTATIONSET_Q)
    {
      m_pPeriodHandler->PurgeAdaptationSetQ();
      (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
    }
    else if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_SUSPEND)
    {
      m_pPeriodHandler->ProcessSuspendQSM();
      (void)m_pPeriodHandler->m_cCmdQ.Remove(pIt);
    }
  }

  return status;
}

bool DASHMediaPeriodHandler::PeriodOpenStateHandler::IsOpenCompleted() const
{
  return true;
}

bool DASHMediaPeriodHandler::GetRepresentationGroupFromXmlKey(
   uint64 xmlRepGrpKey, RepresentationGroup& repGrp)
{
  bool rslt = false;

  uint32 nNumRepGroups = 0;
  (void)m_sDASHSessionInfo.cMPDParser.GetAllRepGroupForPeriod(
     NULL, nNumRepGroups, m_cPeriodInfo.getPeriodKey(), false);
  if (nNumRepGroups > 0)
  {
    RepresentationGroup* pRepGroup = QTV_New_Array(RepresentationGroup, nNumRepGroups);
    if (pRepGroup)
    {
      HTTPDownloadStatus status =
        m_sDASHSessionInfo.cMPDParser.GetAllRepGroupForPeriod(
           pRepGroup, nNumRepGroups, m_cPeriodInfo.getPeriodKey(), false);
      if (HTTPSUCCEEDED(status))
      {
        for (uint32 i = 0; i < nNumRepGroups; ++i)
        {
          RepresentationGroup& rRepGrp = pRepGroup[i];
          const uint64 adaptationSetKey =
              ((uint64)(rRepGrp.getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);

          if (adaptationSetKey == xmlRepGrpKey)
          {
            repGrp = pRepGroup[i];
            rslt = true;
            break;
          }
        }
      }

      QTV_Delete_Array(pRepGroup);
    }
  }

  if (false == rslt)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetRepresentationGroupFromXmlKey failed for repgrp xml key %llu", xmlRepGrpKey);
  }

  return rslt;
}

bool DASHMediaPeriodHandler::GetRepresentationGroupFromGrpKey(
   uint64 nRepGrpKey, RepresentationGroup& repGrp)
{
  bool rslt = false;

  uint32 nNumRepGroups = 0;
  (void)m_sDASHSessionInfo.cMPDParser.GetAllRepGroupForPeriod(
     NULL, nNumRepGroups, m_cPeriodInfo.getPeriodKey(), false);
  if (nNumRepGroups > 0)
  {
    RepresentationGroup* pRepGroup = QTV_New_Array(RepresentationGroup, nNumRepGroups);
    if (pRepGroup)
    {
      HTTPDownloadStatus status =
        m_sDASHSessionInfo.cMPDParser.GetAllRepGroupForPeriod(
           pRepGroup, nNumRepGroups, m_cPeriodInfo.getPeriodKey(), false);
      if (HTTPSUCCEEDED(status))
      {
        for (uint32 i = 0; i < nNumRepGroups; ++i)
        {
          RepresentationGroup& rRepGrp = pRepGroup[i];

          if (rRepGrp.getKey() == nRepGrpKey)
          {
            repGrp = pRepGroup[i];
            rslt = true;
            break;
          }
        }
      }

      QTV_Delete_Array(pRepGroup);
    }
  }

  if (false == rslt)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Failed to get representationGroup for key %llu", nRepGrpKey);
  }

  return rslt;
}

bool DASHMediaPeriodHandler::IsAdaptationSetChangePending()
{
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_SUCCESS;

  MM_CriticalSection_Enter(m_pPeriodDataLock);

  //Wait until common init gets downloaded on all groups
  //before adaptation set change done
  for (uint32 i= 0; i < QSM::MAJOR_TYPE_MAX; i++)
  {
    RepGroupQ& rQ = m_RepGroupQ[i];
    if (!rQ.IsEmpty())
    {
      RepGroupQElem *pElem = rQ.PeekTail();
      if (pElem && pElem->GetAdaptationSetChangeType() != RepGroupQElem::PURGE &&
                   pElem->GetAdaptationSetChangeType() != RepGroupQElem::REMOVE)
      {
        DASHMediaPlayGroup *pPlayGroup = pElem->m_pRepGroup;
        if (pPlayGroup)
        {
          bool bValid = pPlayGroup->IsValid();
          if (bValid && (pPlayGroup->CommonInitDwldState() ==
                         InitializationSegment::INITSEG_STATE_DOWNLOADING))
          {
             QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "CommonInit download pending for track %u", i);
            pPlayGroup->DownloadCommonInit();
          }
        }
      }
    }
  }

  bool bIsEmpty = m_DASMQ.IsEmpty();

  MM_CriticalSection_Leave(m_pPeriodDataLock);

  return bIsEmpty ? false : true;
}

/**
 * Return true if the first playgroup in the period is readable.
 * This should be called on a period transition for the media
 * type only. Eg, on a period change, we need to wait before the
 * first playgroup for the media type is readable.
 */
bool DASHMediaPeriodHandler::IsPeriodReadableForMajorType(
   HTTPCommon::HTTPMediaType majorType)
{
  bool rslt = true;

  // default to true in case nothing is found that indicates
  // we have to wait. Eg, if the q's are empty then its ok to dl_switch as well.
  uint32 qsmMajorType = GetQSMMajorType(majorType);

  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    if (i & qsmMajorType)
    {
      RepGroupQ& rQ = m_RepGroupQ[i];

      if (!rQ.IsEmpty())
      {
        rslt = rQ.IsSwitchableTo();
        break;
      }
    }
  }

  return rslt;
}

/*
 * get the information from all the playgroups
 *
 * @param[out] pTrackInfo Pointer to HTTPMediaTrackInfo,
 *
 * @return  number of valid tracks.
 */
uint32 DASHMediaPeriodHandler::PeriodOpenStateHandler::GetMediaTrackInfo
(
  HTTPMediaTrackInfo * /* pTrackInfo */
)
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
               "GetMediaTrackInfo: not supported");
  return 0;
}

/*
 * redirect to the right play group
 *
 * @param[in] majorType  major media type for which track information
 * needs to be retreived
 * @param[out] TrackInfo populated the track information on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved track info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHMediaPeriodHandler::PeriodOpenStateHandler::GetSelectedMediaTrackInfo
(
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaTrackInfo &TrackInfo
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_UNSUPPORTED;
  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);
  DASHMediaPlayGroup* pPlayGroup = m_pPeriodHandler->GetPlayGroup(majorType);

  if (pPlayGroup)
  {
    eStatus = pPlayGroup->GetSelectedMediaTrackInfo(majorType, TrackInfo);
  }
  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  return eStatus;
}

/*
 * redirect to the right play group
 *
 * @param[in] nTrackID   Identifies the track for which codec data needs to
 * be retrieved
 * @param[in] majorType  media major type for which the codec info is being
 * requested
 * @param[in] minorType  media minor type for which the codec info is being
 * requested
 * @param[out] CodecData populates the codec data on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved codec info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHMediaPeriodHandler::PeriodOpenStateHandler::GetCodecData
(
  uint32 nTrackID,
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaMinorType minorType,
  HTTPCodecData &CodecData
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);
  DASHMediaPlayGroup* pPlayGroup = m_pPeriodHandler->GetPlayGroup(majorType);

  if (pPlayGroup)
  {
    eStatus = pPlayGroup->GetCodecData(nTrackID, majorType, minorType, CodecData);
  }
  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  return eStatus;
}

/*
 * redirect to the right play group
 *
 * @param[in] majorType media type.
 * @param[out] pBuffer  Buffer provies the format block info to the caller
 * @param[out] nBufSize Size of the FormatBlock buffer
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved format block info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHMediaPeriodHandler::PeriodOpenStateHandler::GetFormatBlock
(
  HTTPCommon::HTTPMediaType majorType,
  uint8* pBuffer,
  uint32 &nBufSize
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  DASHMediaPlayGroup* pPlayGroup = m_pPeriodHandler->GetPlayGroup(majorType);

  if (pPlayGroup)
  {
    eStatus = pPlayGroup->GetFormatBlock(majorType, pBuffer, nBufSize);
  }

  return eStatus;
}

/** @brief Check if playback can continue uninterrupted on the
  *        current playing representation.
  *
  * @param  - void
  * @return
  * TRUE - Playback can continue uninterrupted
  * FALSE - Otherwise (only for live)
  */
bool DASHMediaPeriodHandler::PeriodBaseStateHandler::CanPlaybackUninterrupted()
{
  bool bOk = false;

  //For live stream, check if all play groups can resume uninterrupted i.e.
  //current playing segment is still available on server. If not, notify player
  //and it can choose to jump ahead by seeking to the right position!
  if (m_pPeriodHandler->m_sDASHSessionInfo.cMPDParser.IsLive())
  {
    MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);
    for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX ; i++)
    {
      RepGroupQ &rQ = m_pPeriodHandler->m_RepGroupQ[i];
      if (!rQ.IsEmpty())
      {
        bOk = rQ.CanPlaybackUninterrupted();
        if(bOk)
        {
          break;
        }
      }
    }
    MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);
  }
  else//For vod stream, segment is always available.
{
    bOk = true;
  }

  return bOk;
}

/** @brief Queue up SEEK cmd from DASHAdaptor.
  *
  * @param[in] nSeekTime - Seek time
  * @param[in] pCallbackData - Callback data
  * @return
  * HTTPDL_WAITING - OPEN cmd successfully queued and status notified later
  * HTTPDL_SUCCESS - OPEN completed successfully
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::Seek
(
 const int64 nSeekTime,
 void* pCallbackData
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
  m_pPeriodHandler->m_nSeekTime = nSeekTime;
  PeriodCmdData cmd;
  cmd.sSeekCmdData.eCmd = PERIOD_CMD_SEEK;
  cmd.sSeekCmdData.pUserData = pCallbackData;
  cmd.sSeekCmdData.nSeekTime = nSeekTime;
  if (!m_pPeriodHandler->m_cCmdQ.EnQ(cmd))
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Period [0x%02x]: Seek cmd cannot be queued",
                   (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
  }

  return status;
}

bool DASHMediaPeriodHandler::PeriodBaseStateHandler::IsOpenCompleted() const
{
  return false;
}

HTTPDownloadStatus DASHMediaPeriodHandler::PeriodOpenStateHandler::StateEntryHandler()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  return status;
}
/** @brief PeriodSeeking State entry handler. Issues a play on QSM and calls
  *        seek on all play groups
  * @return
  * HTTPDL_WAITING - OPEN cmd successfully queued and status notified later
  * HTTPDL_SUCCESS - OPEN completed successfully
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPeriodHandler::PeriodSeekingStateHandler::StateEntryHandler()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  int64 seekTime = m_pPeriodHandler->m_nSeekTime - m_pPeriodHandler->GetPeriodStartTime();
  m_pPeriodHandler->m_nSeekTimeRelative = seekTime;
  if (seekTime < 0)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "seekTime %d < 0. Set as zero", (int)seekTime);
    seekTime = 0;
  }
  //After posting play command on QSM wait for the play response from QSM. Do not
  //process any commands in between play and play response as the commands might
  //be of the time before the time passed in this play request. Once we get
  //receive play response from QSM start processing the cmds again !!!
  //There is a timing issue on target sometimes if we make the flag true
  //after posting the command on QSM. To avoid that make the flag true first
  //then post command
  m_pPeriodHandler->m_bPauseCmdProcessing = true;

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  for (uint32 i= 0; i < QSM::MAJOR_TYPE_MAX; i++)
  {
    m_pPeriodHandler->m_RepGroupQ[i].RemoveAllButLastElementFromQ();

    RepGroupQElem *pElem = m_pPeriodHandler->m_RepGroupQ[i].GetLastCommittedElem();
    if (pElem)
    {
      DASHMediaPlayGroup *pPlayGroup = pElem->m_pRepGroup;

      if (pPlayGroup)
      {
        pPlayGroup->SetStartTime(seekTime);
        pPlayGroup->Flush(seekTime);
        pPlayGroup->SetSeekPending(true);
      }
    }
  }

  //In case of non interleaved streams execute seek on video playgroup
  //first then after video playgroup reports seek complete,  execute
  //seek on audio.
  //TODO: In case of text media type, this needs to be further enhanced.
  //Call seek on interleaved audio-video playgroup and  based on seek position
  //returned call seek on text play group
  {
    DASHMediaPlayGroup* pPlayGrp = m_pPeriodHandler->GetDownloadingPlayGroup(HTTPCommon::HTTP_VIDEO_TYPE);
    if(pPlayGrp)
    {
      status = pPlayGrp->Seek(seekTime);
    }
    else
    {
      //There is no playgroup for video i.e it is an audio only stream. Call seek on audio playgroup.
      //TODO: Can there be text grp along with audio play grp ??
      DASHMediaPlayGroup* pAudGrp = m_pPeriodHandler->GetPlayGroup(HTTPCommon::HTTP_AUDIO_TYPE);
      if(pAudGrp)
      {
        status = pAudGrp->Seek(seekTime);
      }
      else
      {
        //What should be done here ?...Neither there is audio grp nor video grp.
      }
    }
  }

  for (uint32 i = 0; i < HTTPCommon::HTTP_MAX_TYPE; ++i)
  {
    m_pPeriodHandler->m_LastSampleTS[i] = MAX_UINT64_VAL;
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  if(HTTPSUCCEEDED(status) &&
     m_pPeriodHandler->m_pQSM &&
     m_pPeriodHandler->m_pQSM->Play(m_pPeriodHandler->m_nSeekTime) == QSM::QSM_STATUS_OK)
  {
    status = HTTPCommon::HTTPDL_WAITING;
  }
  else
  {
     //Notify failure and dequeue SEEK cmd
     m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cIdleStateHandler);
     if (m_pPeriodHandler->m_pDANotifier)
     {
       m_pPeriodHandler->m_pDANotifier->NotifyEvent(m_pPeriodHandler->GetPeriodKey(),
                                                    PERIOD_CMD_SEEK, HTTPCommon::HTTPDL_ERROR_ABORT,
                                                       NULL);
     }
  }
  return status;
}
/** @brief Process all commands (in Seeking state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
int DASHMediaPeriodHandler::PeriodSeekingStateHandler::ProcessCmds()
{
  int status = 0;
  PeriodCmdData cmd;
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_WAITING;
  //Process Seek cmd
  if (m_pPeriodHandler->m_cCmdQ.PeekHead(cmd))
  {
    if (cmd.sBaseCmdData.eCmd == PERIOD_CMD_SEEK)
    {
      uint8 numGrpsDataEnd = 0;
      //Wait until seek is complete on all the groups
      result =  m_pPeriodHandler->m_eSeekStatus;

      // true if at least one group is valid. Then don't return EOS and exit.
      bool bIsEOS = true;

      MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

      for (uint32 i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
      {
        RepGroupQ& rQ = m_pPeriodHandler->m_RepGroupQ[i];
        if (!rQ.IsEmpty())
        {
          RepGroupQElem *pElem = rQ.PeekTail();
          if (pElem)
          {
            DASHMediaPlayGroup *pPlayGroup = pElem->m_pRepGroup;
            if (pPlayGroup)
            {
              if (pPlayGroup->IsValid())
              {
                bIsEOS = false;
                break;
              }
            }
          }
        }
      }

      if(bIsEOS)
      {
        result = HTTPCommon::HTTPDL_DATA_END;
      }

      if(result != HTTPCommon::HTTPDL_DATA_END)
      {
        for (uint32 i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
        {
          RepGroupQ &rQ = m_pPeriodHandler->m_RepGroupQ[i];
          if (!rQ.IsEmpty())
          {
            RepGroupQElem *pElem =  rQ.PeekTail();

            if (pElem)
            {
              DASHMediaPlayGroup *pPlayGroup = pElem->m_pRepGroup;
              if (pPlayGroup)
              {
                if (pPlayGroup->IsValid())
                {
                  if (pPlayGroup->IsSeekPending())
                  {
                    result = HTTPCommon::HTTPDL_WAITING;
                    continue;
                  }
                  //Check for isreadable also so that if any switch queued in between
                  //removes the current switch point we will report seek_complete only
                  //after current rep becomes readable.
                  else if(!(pPlayGroup->IsReadable()))
                  {
                    result = HTTPCommon::HTTPDL_WAITING;
                    continue;
                  }
                }
              }
            }
          }
        }
      }
      if(result == HTTPCommon::HTTPDL_WAITING)
      {
        for (uint32 i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
        {
          RepGroupQ &rQ = m_pPeriodHandler->m_RepGroupQ[i];
          if (!rQ.IsEmpty())
          {
            RepGroupQElem *pElem =  rQ.PeekTail();

            if (pElem)
            {
              DASHMediaPlayGroup *pPlayGroup = pElem->m_pRepGroup;
              if (pPlayGroup)
              {
                if (pPlayGroup->IsValid())
                {
                  pPlayGroup->CheckAndHandleSwitchDuringSeek();
                }
              }
            }
          }
        }
      }

      MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

      //If all groups are ready, notify success and dequeue SEEK cmd
      if (result != HTTPCommon::HTTPDL_WAITING)
      {
        if (m_pPeriodHandler->m_pDANotifier)
        {
          m_pPeriodHandler->m_pDANotifier->NotifyEvent(m_pPeriodHandler->GetPeriodKey(),
                                                       PERIOD_CMD_SEEK, result,
                                                       cmd.sSeekCmdData.pUserData);
        }
        (void)m_pPeriodHandler->m_cCmdQ.DeQ(cmd);
         m_pPeriodHandler->m_eSeekStatus = HTTPCommon::HTTPDL_SUCCESS;
        if(result == HTTPCommon::HTTPDL_SUCCESS)
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Period [0x%02x]: All groups ready, period moving to OPEN from SEEK",
                         (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
          m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cOpenStateHandler);
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Period [0x%02x]: Period Seek failed closing the period reporting data_end",
                         (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
          m_pPeriodHandler->Close();
        }
      }
      else
      {
        //ToDo: Is there a timeout logic needed?
      }
    }
  }
  return status;
}

bool DASHMediaPeriodHandler::ShouldNotifySwitch(HTTPCommon::HTTPMediaType majorType,
                                                bool bNotified)
{
  bool rslt = false;

  uint32 qsmMajorType = GetQSMMajorType(majorType);
  m_RepGroupQ[qsmMajorType].QueueOptimize();

  // mark all readable groups that are waiting for add as completed.
  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    RepGroupQ& rQ = m_RepGroupQ[i];
    if (!rQ.IsEmpty())
    {
      if (rQ.IsPendingSwitchDLSwitchNotificationForAdd())
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "AdaptationSetChange: RepGroupQ with idx %u. Send DL switch "
          "notification as ADD", i);
        if (bNotified)
        {
          rQ.DLSwitchNotificationSent();
        }
        rslt = true;
      }
    }
  }

  // if the playgrp associated with this majorType is marked from Remove,
  // also ensure that there is some other readable group that is not marked
  // for remove that is present before sending a switch notification for remove.
  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    RepGroupQ& rQ = m_RepGroupQ[i];
    uint32 qsmMajorType = GetQSMMajorType(majorType);

    if (rQ.IsPendingSwitchDLSwitchNotificationForRemove(qsmMajorType))
    {
      // playgrp associated with this major type needs to be removed.
      // but remove only if there is a readable playgrp for another
      // majorType that is not marked for REMOVE. Otherwise there is,
      // pending add thats waiting. Let that complete so that there is at least
      // one media type for which samples are being read from.

      for (uint32 j = 0; j < QSM::MAJOR_TYPE_MAX; ++j)
      {
        if ((qsmMajorType & j) == 0)
        {
          RepGroupQ& rQOther = m_RepGroupQ[j];
          if (!rQOther.IsEmpty())
          {
            if (rQOther.IsReadable() &&
                false == rQOther.IsPendingSwitchDLSwitchNotificationForRemove(j))
            {
              QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "AdaptationSetChange: RepGroupQ with idx %u. Send DL switch "
                "notification as REMOVE", qsmMajorType);
              if (bNotified)
              {
                rQ.DLSwitchNotificationSent();
              }
              rslt = true;
              break;
            }
          }
        }
      }
    }
  }
  return rslt;
}

/*
 * redirect to the right play group
 *
 * @param[in] HTTPMediaType media type.
 * @param[out]pBuffer  A pointer to the buffer into which to place the sample.
 * @param[out] nSize The size of the data buffer.
 * @param[out] sampleInfo Provides information about the sample
 *
 * @return HTTPDL_ERROR_SUCCESS if successful in retrieving the format block
 * else appropraite error code
 */
HTTPCommon::HTTPDownloadStatus DASHMediaPeriodHandler::PeriodOpenStateHandler::GetNextMediaSample
(
  HTTPCommon::HTTPMediaType majorType,
  uint8 *pBuffer,
  uint32 &nSize,
  HTTPSampleInfo &sampleInfo
)
{
  HTTPCommon::HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_UNSUPPORTED;
  DASHMediaPlayGroup *pPlayGroup = NULL, *p2ndPlayGrp = NULL;
  const uint32 nAllocedSize = nSize;

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  if (true == m_pPeriodHandler->ShouldNotifySwitch(majorType, false))
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
    PeriodCmdData cmd;
    cmd.sBaseCmdData.eCmd = PERIOD_CMD_PURGE_ADAPTATIONSET_Q;
    if (!m_pPeriodHandler->m_cCmdQ.EnQ(cmd))
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Failed to enqueue PERIOD_CMD_PURGE_ADAPTATIONSET_Q");
    }
    else
    {
      m_pPeriodHandler->ShouldNotifySwitch(majorType, true);
      eStatus = HTTPCommon::HTTPDL_SWITCH;
    }
  }

  if (HTTPCommon::HTTPDL_SWITCH != eStatus)
  {
    m_pPeriodHandler->GetFirstAndSecondPlayGroup(pPlayGroup, p2ndPlayGrp, majorType);

    if (pPlayGroup)
    {
      bool bCanSwitchToSecondPlayGrp = false;
      if (p2ndPlayGrp)
      {
        if (p2ndPlayGrp->IsReadable())
        {
          bCanSwitchToSecondPlayGrp = true;
        }
      }

      QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "GetNextMediaSample 1st PG %llu, isReadable %d, 2ndPG addr %p, isReadable %d",
        pPlayGroup->GetKey(), pPlayGroup->IsReadable(), p2ndPlayGrp, bCanSwitchToSecondPlayGrp);

      eStatus = pPlayGroup->GetNextMediaSample(majorType, pBuffer, nSize, sampleInfo);

      if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
      {
        if (p2ndPlayGrp)
        {
          if (sampleInfo.endTime >= p2ndPlayGrp->GetStartTime())
          {
            if (bCanSwitchToSecondPlayGrp)
            {
              QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "AdaptationSetChange along read path: Switching to repGrp with key %llu for majorType %d before end of playgrp",
                p2ndPlayGrp->GetKey(), majorType);
              eStatus = HTTPCommon::HTTPDL_SWITCH;

              m_pPeriodHandler->DeleteElementByPlayGrpKey(pPlayGroup->GetKey());
            }
            else
            {
              QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "GetNextMediaSample wait for pg %llu to become readable, majorTYpe %d",
                p2ndPlayGrp->GetKey(), majorType);
              eStatus = HTTPCommon::HTTPDL_WAITING;
            }
          }
        }
      }
      else
      {
        if (video::HTTPCommon::HTTPDL_SUCCESS != eStatus &&
            video::HTTPCommon::HTTPDL_CODEC_INFO != eStatus)
        {
          // check if there is a readable playgroup following the current one.
          // if yes, then remove the current element and report DL_SWITCH.
          if (bCanSwitchToSecondPlayGrp)
          {
            QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "AdaptationSetChange along read path. Switching to repGrp with key %llu for majorType %d as read status is %d",
              p2ndPlayGrp->GetKey(), majorType, eStatus);

            eStatus = HTTPCommon::HTTPDL_SWITCH;

            m_pPeriodHandler->DeleteElementByPlayGrpKey(pPlayGroup->GetKey());
          }
        }
      }

      if(HTTPCommon::HTTPDL_SUCCESS == eStatus)
      {
        uint64 periodStartTime = m_pPeriodHandler->GetPeriodStartTime();
        sampleInfo.startTime += periodStartTime;
        sampleInfo.endTime += periodStartTime;
        sampleInfo.sSubInfo.periodStartTime = periodStartTime;

        if (m_pPeriodHandler->m_LastSampleTS[majorType] < sampleInfo.endTime ||
            MAX_UINT64_VAL == m_pPeriodHandler->m_LastSampleTS[majorType])
        {
          m_pPeriodHandler->m_LastSampleTS[majorType] = sampleInfo.endTime;
        }

        uint64 nPeriodDuration =
          (uint64)m_pPeriodHandler->m_sDASHSessionInfo.cMPDParser.GetDuration(m_pPeriodHandler->GetPeriodKey());

        if(nPeriodDuration > 0.0)
        {
          uint64 periodStartTime = m_pPeriodHandler->GetPeriodStartTime();
          uint64 periodEndTime = periodStartTime + nPeriodDuration;
          if(eStatus == HTTPCommon::HTTPDL_SUCCESS && sampleInfo.startTime >= periodEndTime)
          {
            eStatus = HTTPCommon::HTTPDL_DATA_END;
            pPlayGroup->SetDownloadComplete();
          }
        }
      }

      // Download error is nofified to the adaptor if data ended on all the tracks and
      // In any of the track data end happened due to data download failure
      if(HTTPCommon::HTTPDL_DATA_END == eStatus)
      {
        HTTPMediaTrackInfo TrackInfo;
        HTTPDownloadStatus eTrackInfoStatus = HTTPCommon::HTTPDL_UNSUPPORTED;
        HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE, HTTPCommon::HTTP_TEXT_TYPE};
        pPlayGroup->SetReadEnd(true);
        bool isDwldErrorSendReq = true;

        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "periox %d data end with status %d",
                         (int)m_pPeriodHandler->GetPeriodKey(), eStatus);

        //Check for the datadownload error and Playback end on all the group
        //Data Error is notified with EOS on the last playgroup
        for (int i = 0; i < STD_ARRAY_SIZE(mediaType); i++)
        {
          eTrackInfoStatus = m_pPeriodHandler->GetSelectedMediaTrackInfo(mediaType[i], TrackInfo);
          DASHMediaPlayGroup* pPlayGrp = m_pPeriodHandler->GetPlayGroup(mediaType[i]);
          if((HTTPCommon::HTTPDL_UNSUPPORTED == eTrackInfoStatus) || (HTTPCommon::HTTPDL_ERROR_ABORT == eTrackInfoStatus))
          {
            continue;
          }
          else
          {
            if(pPlayGrp)
            {
              if(pPlayGrp->IsReadEnd())
              {
                if(!(pPlayGrp->IsLastDataDownloadFailed()))
                 {
                   isDwldErrorSendReq = false;
                   break;
                 }
              }// if(pPlayGrp->m_IsReadEos)
              else
              {
                // Datadownload error is only notified in the last sample read of the playback
                isDwldErrorSendReq = false;
                break;
              }
            } //if(pPlayGrp)
          }
        }

        if(isDwldErrorSendReq)
        {
          eStatus = HTTPCommon::HTTPDL_DATA_END_WITH_ERROR;
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "periox %d DWLD_FAILURE notified status %d",
                         (int)m_pPeriodHandler->GetPeriodKey(), eStatus);
        }

        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "GetNextMediaSample Data End for track %d mark invalid",
             majorType);

      } //if(HTTPCommon::HTTPDL_DATA_END == eStatus)
    }
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  return eStatus;
}

HTTPDownloadStatus DASHMediaPeriodHandler::PeriodClosingStateHandler::Close()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);
  bool bIsQsmStopped = m_pPeriodHandler->m_bDeleteQsm;
  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  if (bIsQsmStopped)
  {
    if (m_pPeriodHandler->m_pQSMInstanceDestroy)
    {
      m_pPeriodHandler->m_pQSMInstanceDestroy(m_pPeriodHandler->m_pQSM);
      m_pPeriodHandler->m_pQSM = NULL;
    }

    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "QSM for periox %llu closed and destroyed",
                  m_pPeriodHandler->GetPeriodKey());
    m_pPeriodHandler->SetStateHandler(&m_pPeriodHandler->m_cIdleStateHandler);
    status = HTTPCommon::HTTPDL_SUCCESS;
  }
  return status;
}

bool DASHMediaPeriodHandler::PeriodClosingStateHandler::IsOpenCompleted() const
{
  return true;
}

/*
 * redirect to the right play group
 *
 * @param[in] HTTPMediaType media type.
 * @param[out] nPlaybackPosition populates in time units on success
 *
 * @return true if successful else failure
 */
bool DASHMediaPeriodHandler::PeriodBaseStateHandler::GetCurrentPlaybackPosition
(
  HTTPCommon::HTTPMediaType majorType,
  uint64 &nPlaybackPosition
)
{
  bool bResult = false;
  DASHMediaPlayGroup* pPlayGroup = NULL;

  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  //ToDo: Remove hack - return video duration for now. If there is no play group
  //for video then return appropriate track duration.
  //QSM logic of download to be revisited - one option is to not look at playback
  //time and buffer occupancy for downloading!
  if (majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
  {
    pPlayGroup = m_pPeriodHandler->GetFirstPlayGroupInUse(HTTPCommon::HTTP_VIDEO_TYPE);
    if(pPlayGroup)
    {
      majorType = HTTPCommon::HTTP_VIDEO_TYPE;
    }
  }

  //If media type is unknown get the min over all media types
  if (majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
  {
    HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE,HTTPCommon::HTTP_TEXT_TYPE};
    for (int i = 0; i < STD_ARRAY_SIZE(mediaType); i++)
    {
      pPlayGroup = m_pPeriodHandler->GetFirstPlayGroupInUse(mediaType[i]);
      if (pPlayGroup)
      {
        uint64 nPos = 0;
        if (pPlayGroup->IsValid() && pPlayGroup->GetCurrentPlaybackPosition(mediaType[i], nPos))
        {
          //Since this playback position will be used for switching purposes also,
          //return the max over all media components to avoid a replay
          nPlaybackPosition = (nPlaybackPosition == 0) ? nPos
                                                       : STD_MAX(nPlaybackPosition, nPos);
          bResult = true;
        }
      }
    }
  }
  else
  {
    pPlayGroup = m_pPeriodHandler->GetFirstPlayGroupInUse(majorType);
    if (pPlayGroup)
    {
      bResult = pPlayGroup->GetCurrentPlaybackPosition(majorType, nPlaybackPosition);
    }
  }

  if (true == bResult)
  {
    if(MAX_UINT64_VAL != nPlaybackPosition)
    {
      nPlaybackPosition += m_pPeriodHandler->GetPeriodStartTime();
    }
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  return bResult;
}

bool DASHMediaPeriodHandler::GetCurrentPlaybackPosition
(
  uint64 nGroupKey,
  HTTPCommon::HTTPMediaType majorType,
  uint64 &nPlaybackPosition
)
{
  // Playgrp key must be valid.
  nPlaybackPosition = 0;
  bool bResult = false;

  MM_CriticalSection_Enter(m_pPeriodDataLock);

  DASHMediaPlayGroup* pPlayGroup = GetPlayGrpByKey(nGroupKey);

  if (pPlayGroup)
  {
    //If media type is unknown get the min over all media types
    if (majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
    {
      HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE,HTTPCommon::HTTP_TEXT_TYPE};
      for (int i = 0; i < STD_ARRAY_SIZE(mediaType); i++)
      {
        uint64 nPos = 0;
        if (pPlayGroup->IsValid() && pPlayGroup->GetCurrentPlaybackPosition(mediaType[i], nPos))
        {
          nPlaybackPosition =
            (nPlaybackPosition == 0 ? nPos : STD_MAX(nPlaybackPosition, nPos));
          bResult = true;
        }
      }
    }
    else
    {
      bResult = pPlayGroup->GetCurrentPlaybackPosition(majorType, nPlaybackPosition);
    }

    if (true == bResult)
    {
      if(MAX_UINT64_VAL != nPlaybackPosition)
      {
        nPlaybackPosition += GetPeriodStartTime();
      }
    }
  }

  MM_CriticalSection_Leave(m_pPeriodDataLock);

  QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "GetCurrentPlaybackPosition (%llu,%llu), %llu, isOk %d",
                GetPeriodKey(), nGroupKey, nPlaybackPosition, bResult);

  return bResult;
}

/*
  * method to retrieve current keys (group key, representation key)
  * @param[in]  majorType    : major type of current representation
  * @param[out] nCurrGrpInfoKey : current group key
  * @param[out] nRepInfoKey : current representation key
  *
  * @return HTTPDL_SUCCESS if successful in retrieving the track information
  * else failure
  */

  HTTPDownloadStatus DASHMediaPeriodHandler::PeriodBaseStateHandler::GetCurrentKeys
  (
    HTTPCommon::HTTPMediaType mediaType,
    int &currGrpInfoKey,
    int &currRepInfoKey
  )
  {
    HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
    MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);
    DASHMediaPlayGroup* pPlayGroup = m_pPeriodHandler->GetPlayGroup(mediaType);
    if (pPlayGroup)
    {
      (void)pPlayGroup->GetCurrentKeys(mediaType, currGrpInfoKey, currRepInfoKey);
    }
    MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);
    return eStatus;
  }

/*
 * redirect to the right play group
 *
 * @param[in] HTTPMediaType media type.
 * @param[out] nFullyDownloadedOccupancy populates occupancy in time uints on success
 * @param[out] nPartiallyDownloadedOccupancy populates occupancy duration in time units on success
 * @param[out] nTotalOccupancy populates occupancy in time uints on success
 * @param[out] nForwardMediaAvailability populates Availibality in time units on success
 * @param[out] nPlaybackPosition populates in time uints on success
 *
 * @return true if successful else failure
 */
bool DASHMediaPeriodHandler::PeriodBaseStateHandler::GetGroupDurationBuffered
(
 uint64 nGroupKey,
 HTTPCommon::HTTPMediaType majorType,
 uint32& nFullyDownloadedOccupancy,
 uint32& nPartiallyDownloadedOccupancy,
 uint32& nTotalOccupancy,
 uint64& nPlaybackPosition,
 uint32& nForwardMediaAvailability
)
{
  bool bResult = false;

  nFullyDownloadedOccupancy = 0;
  nPartiallyDownloadedOccupancy = 0;
  nTotalOccupancy = 0;
  nPlaybackPosition = 0;
  nForwardMediaAvailability = 0;

  //ToDo: Remove hack - return video duration for now. QSM logic of download to be
  //revisited - one option is to not look at playback time and buffer occupancy for
  //downloading!
  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  if (majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
  {
    if (m_pPeriodHandler->GetFirstPlayGroupInUse(HTTPCommon::HTTP_VIDEO_TYPE))
    {
      majorType = HTTPCommon::HTTP_VIDEO_TYPE;
    }
  }

  DASHMediaPlayGroup *pSearchPlayGroup = m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);
  if (NULL == pSearchPlayGroup)
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "GetGroupDurationBuffered failed to find grp with key %llu in period %p",
                  nGroupKey, m_pPeriodHandler);
  }
  else
  {
    // get the playback position for the group
    bResult = m_pPeriodHandler->GetCurrentPlaybackPosition(nGroupKey,majorType, nPlaybackPosition);

    if (true == bResult)
    {
      bResult = false;

      //If media type is unknown get the min over all media types
      if (majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
      {
        HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE, HTTPCommon::HTTP_TEXT_TYPE};
        QSM::MajorType qsmMajorTypeArr[] = { QSM::MAJOR_TYPE_AUDIO, QSM::MAJOR_TYPE_VIDEO, QSM::MAJOR_TYPE_TEXT };

        static const int qsmMajorTypeArrSize = STD_ARRAY_SIZE(qsmMajorTypeArr);

        for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
        {
          uint64 nPbPosition = 0;
          bResult = pSearchPlayGroup->GetGroupPlaybackStats(
             majorType,
             nFullyDownloadedOccupancy, nPartiallyDownloadedOccupancy,
             nTotalOccupancy, nPbPosition, nForwardMediaAvailability);

          nPlaybackPosition = STD_MAX(nPlaybackPosition, nPbPosition);
        }
      }
      else
      {
        uint64 nPbPosition = 0;
        bResult = pSearchPlayGroup->GetGroupPlaybackStats(
           majorType,
           nFullyDownloadedOccupancy, nPartiallyDownloadedOccupancy,
           nTotalOccupancy, nPbPosition, nForwardMediaAvailability);

        nPlaybackPosition = STD_MAX(nPlaybackPosition, nPbPosition);
      }
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "GetGroupDuration returning false for GrpKey %llu", nGroupKey);
    }
  }

  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  static int logCount[HTTPCommon::HTTP_MAX_TYPE] =  {0,0,0,0};

  if (0 == logCount[majorType])
  {
    QTV_MSG_PRIO7(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "GetGroupPlaybackStats: ok %d, GetGroupDurationBuffered major %d, (%u,%u,%u), pbPos %llu, fma %u",
                  bResult, majorType, nFullyDownloadedOccupancy, nPartiallyDownloadedOccupancy,
                  nTotalOccupancy, nPlaybackPosition, nForwardMediaAvailability);
  }
  ++logCount[majorType];
  if (logCount[majorType] > 25)
  {
    logCount[majorType] = 0;
  }

  return bResult;
}

bool DASHMediaPeriodHandler::PeriodBaseStateHandler::GetMediaDurationBuffered
(
 HTTPCommon::HTTPMediaType majorType,
 uint32& nFullyDownloadedOccupancy,
 uint32& nPartiallyDownloadedOccupancy,
 uint32& nTotalOccupancy,
 uint64& nPlaybackPosition,
 uint32& nForwardMediaAvailability
)
{
  nFullyDownloadedOccupancy = 0;
  nPartiallyDownloadedOccupancy = 0;
  nTotalOccupancy = 0;
  nPlaybackPosition = 0;
  nForwardMediaAvailability = 0;

  //ToDo: Remove hack - return video duration for now. QSM logic of download to be
  //revisited - one option is to not look at playback time and buffer occupancy for
  //downloading!
  MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

  if (majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
  {
    RepGroupQ& rQVideo = m_pPeriodHandler->m_RepGroupQ[QSM::MAJOR_TYPE_VIDEO];
    if (rQVideo.PeekFirstElementInUse())
    {
      majorType = HTTPCommon::HTTP_VIDEO_TYPE;
    }
  }

  // get the playback psotion for the mediaType
  bool bResult = GetCurrentPlaybackPosition(majorType, nPlaybackPosition);

  if (true == bResult)
  {
    bResult = false;

    //If media type is unknown get the min over all media types
    if (majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
    {
      HTTPCommon::HTTPMediaType mediaType[] = {HTTPCommon::HTTP_AUDIO_TYPE, HTTPCommon::HTTP_VIDEO_TYPE, HTTPCommon::HTTP_TEXT_TYPE};
      QSM::MajorType qsmMajorTypeArr[] = { QSM::MAJOR_TYPE_AUDIO, QSM::MAJOR_TYPE_VIDEO, QSM::MAJOR_TYPE_TEXT };

      static const int qsmMajorTypeArrSize = STD_ARRAY_SIZE(qsmMajorTypeArr);

      for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
      {
        RepGroupQ& rQ = m_pPeriodHandler->m_RepGroupQ[i];

        for (int j = 0; j < qsmMajorTypeArrSize; ++j)
        {
          if (!rQ.IsEmpty())
          {
            RepGroupQElem *pElem = rQ.PeekHead();

            if (pElem)
            {
              DASHMediaPlayGroup *pHeadPlayGrp = pElem->m_pRepGroup;

              if (pHeadPlayGrp)
              {
                uint32 playGrpMajorType = pElem->m_pRepGroup->m_nMajorType;
                if (playGrpMajorType & qsmMajorTypeArr[j])
                {
                  uint64 nPbPosition = 0;
                  uint32 nFullyDowldOcc = 0;
                  uint32 nPartiallyDwldOcc = 0;
                  uint32 nTotalOcc = 0;
                  uint32 nFMA = 0;

                  rQ.GetPlaybackStatsForQ(mediaType[j], nFullyDownloadedOccupancy,nPartiallyDownloadedOccupancy,
                    nTotalOccupancy, nPbPosition, nForwardMediaAvailability);

                  nPlaybackPosition = STD_MAX(nPlaybackPosition, nPbPosition);

                  nFullyDownloadedOccupancy     = STD_MAX(nFullyDownloadedOccupancy, nFullyDowldOcc);
                  nPartiallyDownloadedOccupancy = STD_MAX(nPartiallyDownloadedOccupancy, nPartiallyDwldOcc);
                  nTotalOccupancy               = STD_MAX(nTotalOccupancy, nTotalOcc);
                  nForwardMediaAvailability     = STD_MAX(nForwardMediaAvailability, nFMA);

                  bResult = true;
                }
              }
            }
          }
        }
      }
    }
    else
    {
      for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
      {
        uint32 qsmMajorType = m_pPeriodHandler->GetQSMMajorType(majorType);
        RepGroupQ& rQ = m_pPeriodHandler->m_RepGroupQ[i];
        if (!rQ.IsEmpty())
        {
          RepGroupQElem *pElem = rQ.PeekHead();
          if (pElem)
          {
            DASHMediaPlayGroup *pHeadPlayGrp = pElem->m_pRepGroup;
            if (pHeadPlayGrp)
            {
              uint32 playGrpMajorType = pElem->m_pRepGroup->m_nMajorType;
              if (qsmMajorType & playGrpMajorType)
              {
                uint64 nPbPosition = 0;
                rQ.GetPlaybackStatsForQ(majorType, nFullyDownloadedOccupancy,nPartiallyDownloadedOccupancy,
                  nTotalOccupancy, nPbPosition, nForwardMediaAvailability);
                nPlaybackPosition = STD_MAX(nPlaybackPosition, nPbPosition);
                bResult = true;
                break;
              }
            }
          }
        }
      }
    }
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetMediaDurationBuffered returning FALSE for Period %p", m_pPeriodHandler);
  }


  MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);

  static int logCount[HTTPCommon::HTTP_MAX_TYPE] =  {0,0,0,0};
  if (0 == logCount[majorType])
  {
    QTV_MSG_PRIO7(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "GlobalStats: ok %d, GetGroupDurationBuffered major %d, (%u,%u,%u), pbPos %llu, fma %u",
                  bResult, majorType, nFullyDownloadedOccupancy, nPartiallyDownloadedOccupancy,
                  nTotalOccupancy, nPlaybackPosition, nForwardMediaAvailability);
  }
  ++logCount[majorType];
  if (logCount[majorType] > 25)
  {
    logCount[majorType] = 0;
  }


  return bResult;
}

/*
 * redirect to the right play group
 *
 * @param[in] HTTPMediaType media type.
 * @param[out] nPlaybackPosition populates in time uints on success
 * @param[out] nBufferedDuration populates buffered duration in time units on success
 *
 * @return true if successful else failure
 */
bool DASHMediaPeriodHandler::PeriodBaseStateHandler::GetDurationBuffered
(
  HTTPCommon::HTTPMediaType majorType,
  uint64 &nPlaybackPosition,
  uint64 &nBufferedDuration
)
{
 uint32 nFullyDownloadedOccupancy = 0;
 uint32 nPartiallyDownloadedOccupancy = 0;
 uint32 nTotalOccupancy = 0;
 nPlaybackPosition = 0;
 nBufferedDuration = 0;
 uint32 nForwardMediaAvailability = 0;
 if(GetMediaDurationBuffered(majorType,
                             nFullyDownloadedOccupancy,
                             nPartiallyDownloadedOccupancy,
                             nTotalOccupancy,
                             nPlaybackPosition,
                             nForwardMediaAvailability))
 {
   nBufferedDuration = nFullyDownloadedOccupancy;
   return true;
 }
 else
 {
   return false;
 }
}

/** @brief Notify async error (valid in all states).
  *
  * @param[in] nGroupKey - Group key
  * @param[in] eStatus - Async error
  */
void DASHMediaPeriodHandler::NotifyError
(
 const uint64 /* nGroupKey */,
 const HTTPDownloadStatus eStatus
)
{
  //Group with key nGroupKey notified error
  //ToDo: async error - what to do? Should filter errors and selectively ignore?
  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Period [0x%02x]: Async fatal error %d",
                 (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56), eStatus );
}
/** @brief Seek command notification is received from play group
  * Set the seek pending to false on the playgroup which reported
  * seek complete. Call seek on the audio playgroup when seek
  * notification is received for video playgroup
  * @param[in] nGroupKey - Group key
  * @param[in] eStatus - Async error
  */
void DASHMediaPeriodHandler::PeriodSeekingStateHandler::NotifySeekStatus
(
  uint64 nGroupKey,
  int64 nSeekEndTime,
  HTTPDownloadStatus eStatus

)
{
  if(eStatus == HTTPCommon::HTTPDL_SUCCESS || HTTPCommon::HTTPDL_ERROR_ABORT ||
    eStatus == HTTPCommon::HTTPDL_DATA_END)
  {
    //Seek failure will also result in reporting success to upper layer
    //GetNextMediaSample after seek will return data end in failure case

    MM_CriticalSection_Enter(m_pPeriodHandler->m_pPeriodDataLock);

    DASHMediaPlayGroup *pPlayGroup =  m_pPeriodHandler->GetPlayGrpByKey(nGroupKey);

    if (pPlayGroup)
    {
      pPlayGroup->SetSeekPending(false);

      if(nSeekEndTime != USE_PERIOD_END_TIME)
      {
        m_pPeriodHandler->m_nSeekTimeRelative = nSeekEndTime;
      }
      else
      {
        nSeekEndTime = m_pPeriodHandler->m_nSeekTimeRelative;
      }
      DASHMediaPlayGroup* pVideoGrp = m_pPeriodHandler->GetPlayGroup(HTTPCommon::HTTP_VIDEO_TYPE);
      //Check if the group which has returned seek notification is video playgroup or not
      if(pVideoGrp && pVideoGrp->GetKey() == nGroupKey)
      {
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
         "Period [0x%02x]: Seek completed on video playgroup seek time %u",
         (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
         (uint32)nSeekEndTime);
        DASHMediaPlayGroup* pAudioGrp = m_pPeriodHandler->GetPlayGroup(HTTPCommon::HTTP_AUDIO_TYPE);
        if(pAudioGrp && pAudioGrp != pVideoGrp)
        {
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Period [0x%02x]: Seek started on audio playgroup seek time %u",
             (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
             (uint32)nSeekEndTime);
          pAudioGrp->Seek(nSeekEndTime);
        }
        else
        {
          // there is no Audio Group then issue Seek on Text Track
          DASHMediaPlayGroup* pTextGrp = m_pPeriodHandler->GetPlayGroup(HTTPCommon::HTTP_TEXT_TYPE);
          if(pTextGrp && pVideoGrp->GetSeekStatus())
          {
             QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Period [0x%02x]: Seek started on Text playgroup seek time %u",
             (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
             (uint32)nSeekEndTime);
             pTextGrp->Seek(nSeekEndTime);
          }
          else if(!pVideoGrp->GetSeekStatus())
          {
            m_pPeriodHandler->m_eSeekStatus = HTTPCommon::HTTPDL_DATA_END;
          }
        }
      }
      else
      {
        DASHMediaPlayGroup* pAudioGrp = m_pPeriodHandler->GetPlayGroup(HTTPCommon::HTTP_AUDIO_TYPE);
        if(pAudioGrp && pAudioGrp->GetKey() == nGroupKey)
        {
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "Period [0x%02x]: Seek completed on audio playgroup seek time %u",
              (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
               (uint32)nSeekEndTime);
          // issue Seek on Text Track
          DASHMediaPlayGroup* pTextGrp = m_pPeriodHandler->GetPlayGroup(HTTPCommon::HTTP_TEXT_TYPE);
          if(pTextGrp && ((pVideoGrp && pVideoGrp->GetSeekStatus()) || pAudioGrp->GetSeekStatus()))
          {
             QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Period [0x%02x]: Seek started on Text playgroup seek time %u",
             (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
             (uint32)nSeekEndTime);
             pTextGrp->Seek(nSeekEndTime);
          }
          else if(!((pVideoGrp && pVideoGrp->GetSeekStatus()) || pAudioGrp->GetSeekStatus()))
          {
            m_pPeriodHandler->m_eSeekStatus = HTTPCommon::HTTPDL_DATA_END;
          }
        }
        else
        {
          DASHMediaPlayGroup* pTextGrp = m_pPeriodHandler->GetPlayGroup(HTTPCommon::HTTP_TEXT_TYPE);
           if(pTextGrp && pTextGrp->GetKey() == nGroupKey)
          {
            QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "Period [0x%02x]: Seek completed on Text playgroup seek time %u",
                (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                 (uint32)nSeekEndTime);
           }
           else
           {
             QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                "Period [0x%02x]: Seek completed on unknown playgroup %llu seek time %u",
                (uint32)((m_pPeriodHandler->m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                 nGroupKey,(uint32)nSeekEndTime);
           }
        }
      }
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Failed to find repgroup with nGroupKey %llu", nGroupKey);
    }

    MM_CriticalSection_Leave(m_pPeriodHandler->m_pPeriodDataLock);
  }
  else
  {
    //Ignoring all other seek status in period handler
  }
  return;
}
/** @brief Create a "play" group based on period info.
  *
  * @return
  * TRUE - Success
  * FALSE - Failure
  */
bool DASHMediaPeriodHandler::CreatePlayGroup()
{
  bool bResult = false;

  int numPlayGrps = 0;
  uint32 nNumRepGroups = 0;
  (void)m_sDASHSessionInfo.cMPDParser.GetAllRepGroupForPeriod(NULL, nNumRepGroups,
                                                              m_cPeriodInfo.getPeriodKey());

  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "CreatePlayGroup for period %llu, numGroups %u",
                m_cPeriodInfo.getPeriodKey(), nNumRepGroups);

  if (nNumRepGroups > 0)
  {
    RepresentationGroup* pRepGroup = QTV_New_Array(RepresentationGroup, nNumRepGroups);
    if (pRepGroup)
    {
      HTTPDownloadStatus status =
        m_sDASHSessionInfo.cMPDParser.GetAllRepGroupForPeriod(pRepGroup, nNumRepGroups,
                                                              m_cPeriodInfo.getPeriodKey());
      if (HTTPSUCCEEDED(status))
      {
        const int nMaxPlayGroups = 3;

        //Filter based on language and pick english (ToDo: filtering across
        //rep groups can be enhanced to look at codecs, supported bitrates etc.)
        for (uint32 index = 0; index < nNumRepGroups; index++)
        {
          //Pick the first A, V or AV rep group (no T for now)
          uint32 nMajorType = QSM::MAJOR_TYPE_UNKNOWN;
          bool bCreate = false;
          (void)GetGroupMajorType(pRepGroup[index], nMajorType);

          if (numPlayGrps <= HTTP_MAX_PLAYGROUPS)
          {
            /* Passing Bandwidth Estmator object */
            pRepGroup[index].m_bEstimator = m_bEstimator;

            DASHMediaPlayGroup *pNewPlayGroup = QTV_New(DASHMediaPlayGroup);
            if (pNewPlayGroup)
            {
              pNewPlayGroup->Init(m_nNextKey, // m_nKey
                                  nMajorType,
                                  pRepGroup[index], &m_sDASHSessionInfo,
                                  (iPeriodNotifier *)this, m_pScheduler);

              RepGroupQElem *pElem = QTV_New_Args(RepGroupQElem, (pNewPlayGroup));
              if (pElem)
              {
                pElem->Commit();
                m_RepGroupQ[nMajorType].Push(pElem);
                QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                              "RepGrpQ: Added DownloadQElem key %llu, map arrayIdx %u to majorType(arrayIdx) %u",
                              pNewPlayGroup->GetKey(), nMajorType, nMajorType);
              }
              else
              {
                QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "CreatePlayGroup failed to allocated RepGroupQElem");
              }

              ++m_nNextKey;
            }
            else
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "CreatePlayGroup failed to allocated DASHMediaPlayGroup");
            }


            numPlayGrps++;
          }

          if (numPlayGrps >= nMaxPlayGroups)
          {
            break;
          }
        }

        if (numPlayGrps > 0)
        {
          bResult = true;
        }
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Period [0x%02x]: Created %d play groups successfully from %u rep groups in the period",
                       (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                       numPlayGrps, nNumRepGroups );
      }

      QTV_Delete_Array(pRepGroup);
      pRepGroup = NULL;
    }
  }

  return bResult;
}

/** @brief Gets major type for rep group.
  *
  * @param[in] sRepGroup - Reference to rep group
  * @param[in] nMajorType - Major type bit mask
  * @return
  * TRUE - Success
  * FALSE - Failure
  */
bool DASHMediaPeriodHandler::GetGroupMajorType
(
 RepresentationGroup& sRepGroup,
 uint32& nMajorType
)
{
  bool bResult = false;
  int nNumCodecs = 0;

  nMajorType = QSM::MAJOR_TYPE_UNKNOWN;
  (void)sRepGroup.getCodec(NULL, nNumCodecs);
  if (nNumCodecs > 0)
  {
    CodecInfo* pCodec = (CodecInfo *)QTV_Malloc(nNumCodecs * sizeof(CodecInfo));
    if (pCodec)
    {
      if (sRepGroup.getCodec(pCodec, nNumCodecs))
      {
        bResult = true;
        for (int i = 0; i < nNumCodecs; i++)
        {
          if (pCodec[i].majorType == MJ_TYPE_AUDIO)
          {
            nMajorType |= QSM::MAJOR_TYPE_AUDIO;
          }
          else if (pCodec[i].majorType == MJ_TYPE_VIDEO)
          {
            nMajorType |= QSM::MAJOR_TYPE_VIDEO;
          }
          else if (pCodec[i].majorType == MJ_TYPE_TEXT)
          {
              nMajorType |= QSM::MAJOR_TYPE_TEXT;
          }
        }
      }
      QTV_Free(pCodec);
      pCodec = NULL;
    }
  }

  return bResult;
}

/** @brief Gets QSM major type based on media major type.
  *
  * @param[in] mediaType - HTTP media type
  * @return QSM major type
  */
uint32 DASHMediaPeriodHandler::GetQSMMajorType
(
 const HTTPCommon::HTTPMediaType mediaType
)
{
  uint32 QSMMajorType;
  switch(mediaType)
  {
    case HTTPCommon::HTTP_AUDIO_TYPE:
      QSMMajorType = QSM::MAJOR_TYPE_AUDIO;
      break;
    case HTTPCommon::HTTP_VIDEO_TYPE:
      QSMMajorType = QSM::MAJOR_TYPE_VIDEO;
      break;
    case HTTPCommon::HTTP_TEXT_TYPE:
        QSMMajorType = QSM::MAJOR_TYPE_TEXT;
      break;
    default:
      QSMMajorType = QSM::MAJOR_TYPE_UNKNOWN;
      break;
  }
  return QSMMajorType;
}

/** @brief Gets DASH Major type based on QSM major type.
  *
  * @param[in] mediaType - QSM major type
  * @return HTTP media type
  */
uint32 DASHMediaPeriodHandler::GetDASHMajorType
(
 const QSM::MajorType mediaType
)
{
  uint32 DASHMajorType;
  switch(mediaType)
  {
    case QSM::MAJOR_TYPE_AUDIO:
      DASHMajorType = HTTPCommon::HTTP_AUDIO_TYPE;
      break;
    case QSM::MAJOR_TYPE_VIDEO:
      DASHMajorType = HTTPCommon::HTTP_VIDEO_TYPE;
      break;
    case QSM::MAJOR_TYPE_TEXT:
      DASHMajorType = HTTPCommon::HTTP_TEXT_TYPE;
      break;
    default:
      DASHMajorType = HTTPCommon::HTTP_UNKNOWN_TYPE;
      break;
  }
  return DASHMajorType;
}

/** @brief Gets play group based on media major type.
  *
  * @param[in] mediaType - HTTP media type
  * @return reference to play group if found else NULL
  */
DASHMediaPlayGroup* DASHMediaPeriodHandler::GetDownloadingPlayGroup
(
 const HTTPCommon::HTTPMediaType mediaType
)
{
  DASHMediaPlayGroup* pPlayGroup = NULL;
  uint32 nMajorType = GetQSMMajorType(mediaType);

  RepGroupQElem *pElem = m_RepGroupQ[nMajorType].GetLastCommittedElem();

  if (pElem)
  {
    DASHMediaPlayGroup *pPlayGrp = pElem->m_pRepGroup;

    if (pPlayGrp)
    {
      if ((pPlayGrp->m_nMajorType & nMajorType) &&
          pPlayGrp->IsValid())
      {
        pPlayGroup = pPlayGrp;
      }
    }
  }
  return pPlayGroup;
}

/** @brief Gets play group based on media major type.
  *
  * @param[in] mediaType - HTTP media type
  * @return reference to play group if found else NULL
  */
DASHMediaPlayGroup* DASHMediaPeriodHandler::GetPlayGroup
(
 const HTTPCommon::HTTPMediaType mediaType
)
{
  DASHMediaPlayGroup* pPlayGroup = NULL;
  uint32 nMajorType = GetQSMMajorType(mediaType);

  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    if (i & nMajorType)
    {
      RepGroupQ& rQ = m_RepGroupQ[i];
      RepGroupQElem *pElem = rQ.PeekFirstElementForRead();
      if (pElem)
      {
        pPlayGroup = pElem->m_pRepGroup;
        break;
      }
    }
  }

  return pPlayGroup;
}

DASHMediaPlayGroup* DASHMediaPeriodHandler::GetFirstPlayGroupInUse
(
 const HTTPCommon::HTTPMediaType mediaType
)
{
  DASHMediaPlayGroup* pPlayGroup = NULL;
  uint32 nMajorType = GetQSMMajorType(mediaType);

  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    if (i & nMajorType)
    {
      RepGroupQ& rQ = m_RepGroupQ[i];
      RepGroupQElem *pElem = rQ.PeekFirstElementInUse();
      if (pElem)
      {
        pPlayGroup = pElem->m_pRepGroup;
      }
    }
  }

  return pPlayGroup;
}


/** @brief Gets play group based on media major type.
  *
  * @param[in] mediaType - HTTP media type
  * @return reference to play group if found else NULL
  */
void DASHMediaPeriodHandler::GetFirstAndSecondPlayGroup
(
 DASHMediaPlayGroup*& pFirstPlayGrp,
 DASHMediaPlayGroup*& pSecondPayGrp,
 const HTTPCommon::HTTPMediaType mediaType
)
{
  pFirstPlayGrp = NULL;
  pSecondPayGrp = NULL;

  uint32 nMajorType = GetQSMMajorType(mediaType);

  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    if (i & nMajorType)
    {
      RepGroupQ& rQ = m_RepGroupQ[i];
      if (!rQ.IsEmpty())
      {
        RepGroupQElem *pElemFirst = NULL;
        RepGroupQElem *pElemSecond = NULL;
        rQ.PeekFirstAndSecondElem(pElemFirst, pElemSecond);

        if (pElemFirst)
        {
          if (pElemFirst->IsCommitted())
          {
            pFirstPlayGrp = pElemFirst->m_pRepGroup;
          }

          if (pElemSecond)
          {
            if (pElemSecond->IsCommitted())
            {
              pSecondPayGrp = pElemSecond->m_pRepGroup;
            }
          }
        }

        break;
      }
    }
  }
}

/** @brief Get Base time for the period
  *
  * @param[out] baseTime - baseTime for period
  * @return
  */
bool DASHMediaPeriodHandler::GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime)
{
  bool bRet = false;
  uint64 tmpBaseTime = MAX_UINT64;
  uint64 tmpMPDBaseTime = MAX_UINT64;
  //Get the minimum basetime across all play groups

  MM_CriticalSection_Enter(m_pPeriodDataLock);

  for (uint32 i = 0 ; i <= QSM::MAJOR_TYPE_MAX; ++i)
  {
    RepGroupQ& rQ = m_RepGroupQ[i];
    if (!rQ.IsEmpty())
    {
      RepGroupQElem *pElem = rQ.PeekHead();
      if (pElem)
      {
        DASHMediaPlayGroup *pPlayGroup = pElem->m_pRepGroup;

        if (pPlayGroup)
        {
          bRet = pPlayGroup->GetBaseTime(segMediaTime, segMPDTime);

          if(MAX_UINT64 == tmpBaseTime /*first rep*/ || tmpBaseTime > segMediaTime /*min across groups*/)
          {
            tmpBaseTime = segMediaTime;
            tmpMPDBaseTime = segMPDTime;
          }
        }
      }
    }
  }

  MM_CriticalSection_Leave(m_pPeriodDataLock);

  if(tmpBaseTime != MAX_UINT64)
  {
    segMediaTime = tmpBaseTime;
    segMPDTime = tmpMPDBaseTime;
  }
  else
  {
    segMediaTime = m_nStartTime;
    segMPDTime = m_nStartTime;
    bRet = true;
  }
  return bRet;
}

/** @brief Get observed bandwidth.
  *
  * @param[in] nInterval - interval
  * @return bandwidth (in Kbps)
  */
int DASHMediaPeriodHandler::GetObservedBandwidth(uint32 /* nInterval */)
{
  int nBW = 0;
  if (m_bEstimator)
  {
    nBW = m_bEstimator->GetEstimatedBandwidth();
  }
  if (nBW >= 0)
  {
    //Convert to Kbps
    nBW /= 1024;
  }
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                 "Observed bandwidth = %d kbps", nBW );

#ifdef DASH_STATS
  //Collecting some stats
  if (m_pStatsFile)
  {
    uint64 nPbTime = 0;
    (void)GetCurrentPlaybackPosition(HTTPCommon::HTTP_UNKNOWN_TYPE, nPbTime);

    uint64 nBuffDuration = 0;
    (void)GetDurationBuffered(HTTPCommon::HTTP_UNKNOWN_TYPE, nPbTime, nBuffDuration);

    char statsFileEntry[100] = {0};
    int numBytesWritten = 0;
    (void)std_strlprintf(statsFileEntry, sizeof(statsFileEntry), "%u\t%d\t%u\t%d\t%llu\t%d\r\n",
                         (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56),
                         m_nCurrentRepID, (uint32)nPbTime, nBW, nBuffDuration,
                         m_sDASHSessionInfo.cHeapManager.GetMaxAvailableSpace() / 1024);
    (void)MM_File_Write(m_pStatsFile, statsFileEntry, std_strlen(statsFileEntry), &numBytesWritten);
  }
#endif /* DASH_STATS */

  return nBW;
};

/** @brief Get current buffer occupancy.
  *
  * @param[out] nDuration - Buffer duration (in msec)
  * @return
  * DSP_STATUS_OK - Success
  * DSP_STATUS_FAILURE - Failure
  */
QSM::IDataStateProviderStatus DASHMediaPeriodHandler::GetBufferOccupancy
(
 uint64 &nDuration
)
{
  nDuration = 0;
  uint64 nPlaybackPosition = 0;
  uint64 nBufferedDuration = 0;
  bool bOk = GetDurationBuffered(HTTPCommon::HTTP_UNKNOWN_TYPE,
                                 nPlaybackPosition, nBufferedDuration);
  if (bOk)
  {
    nDuration = nBufferedDuration;
  }
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                 "Buffer occupancy = %u msec", (uint32)nDuration );

  return (bOk ? QSM::DSP_STATUS_OK : QSM::DSP_STATUS_FAILURE);
};

/** @brief Get Current Playback Time
  *
  * @param[in] nGrpKey - Group key
  * @param[out] nPlaybackTime - current playback time
  * @return
  * DSP_STATUS_OK - Success
  * DSP_STATUS_FAILURE - Failure
  */
QSM::IStreamSourceStatus DASHMediaPeriodHandler::GetPlaybackTime
(
 uint64 nGroupKey, uint64& nPlaybackTime
)
{
  nPlaybackTime = 0;
  bool bOk = false;

  HTTPCommon::HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;

  MM_CriticalSection_Enter(m_pPeriodDataLock);

  DASHMediaPlayGroup *pPlayGroup =  GetPlayGrpByKey(nGroupKey);

  if (pPlayGroup)
  {
    if(pPlayGroup->m_nMajorType == QSM::MAJOR_TYPE_AUDIO )
    {
      mediaType = HTTPCommon::HTTP_AUDIO_TYPE;
    }
    else if((pPlayGroup->m_nMajorType == QSM::MAJOR_TYPE_VIDEO)||
            // for interleaved Audio Video, majortype=3
            (pPlayGroup->m_nMajorType == (QSM::MAJOR_TYPE_VIDEO|QSM::MAJOR_TYPE_AUDIO)))
    {
      mediaType = HTTPCommon::HTTP_VIDEO_TYPE;
    }

    bOk = pPlayGroup->GetCurrentPlaybackPosition(mediaType, nPlaybackTime);
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetPlaybackTime: Invalid GroupKey :%llu", nGroupKey);
  }

  if (bOk)
  {
    nPlaybackTime += GetPeriodStartTime();
  }

  MM_CriticalSection_Leave(m_pPeriodDataLock);

  QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "GetPlaybackTime: (%llu,%llu), nPlaybackTime %llu, stat %d",
                GetPeriodKey(), nGroupKey, nPlaybackTime, bOk);

  return (bOk ? QSM::SS_STATUS_OK : QSM::SS_STATUS_FAILURE);
}

/**
 * Get the cumulative download statistics across all periods.
 */
QSM::IDataStateProviderStatus DASHMediaPeriodHandler::GetCumulativeDownloadStats
(
 uint64& nTotalTimeDownloadingData,
 uint64& nTotalBytesDownloaded
)
{
  return (m_pDAInfoQuery
          ? m_pDAInfoQuery->GetCumulativeDownloadStats(nTotalTimeDownloadingData, nTotalBytesDownloaded)
          : QSM::DSP_STATUS_FAILURE);
}

/** @brief Get current buffer occupancy and Current Playback
  *        Time
  *
  * @param[out] nFullyDownloadedOccupancy - Fully downloaded Buffer duration (in msec)
  * @param[out] nPartiallyDownloadedOccupancy - Partially downloaded Buffer duration (in msec)
  * @param[out] nTotalOccupancy - Total Occupancy Buffer duration (in msec)
  * @param[out] nPlaybackTime - playback time (in msec)
  * @param[out] nForwardMediaAvailability - (in msec)
  * @return
  * DSP_STATUS_OK - Success
  * DSP_STATUS_FAILURE - Failure
  */
QSM::IDataStateProviderStatus DASHMediaPeriodHandler::GetGroupPlaybackStats
(
 uint64 nGroupKey,
 uint32& nFullyDownloadedOccupancy,
 uint32& nPartiallyDownloadedOccupancy,
 uint32& nTotalOccupancy,
 uint64& nPlaybackTime,
 uint32& nForwardMediaAvailability
)
{
  // QSM calls this with nGroupKey MAX_UNIT64_VAL as well.
  nFullyDownloadedOccupancy     = 0;
  nPartiallyDownloadedOccupancy = 0;
  nTotalOccupancy               = 0;
  nForwardMediaAvailability     = 0;
  nPlaybackTime                 = 0;
  uint64 nPlaybackPosition = 0;
  uint64 nBufferedDuration = 0;
  QSM::IDataStateProviderStatus status = QSM::DSP_STATUS_FAILURE;
  HTTPCommon::HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;

  bool bOK = false;

  MM_CriticalSection_Enter(m_pPeriodDataLock);

  DASHMediaPlayGroup *pPlayGroup =  GetPlayGrpByKey(nGroupKey);

  if (pPlayGroup)
  {
    if (pPlayGroup->IsStartTimeInitialized())
    {
      if(pPlayGroup->m_nMajorType == QSM::MAJOR_TYPE_AUDIO )
      {
        mediaType = HTTPCommon::HTTP_AUDIO_TYPE;
      }
      else if(pPlayGroup->m_nMajorType & QSM::MAJOR_TYPE_VIDEO)
      {
        // for interleaved video as well.
        mediaType = HTTPCommon::HTTP_VIDEO_TYPE;
      }
      else if (pPlayGroup->m_nMajorType == QSM::MAJOR_TYPE_TEXT)
      {
        mediaType = HTTPCommon::HTTP_TEXT_TYPE;
      }

      //GetDurationBuffered() takes care of audio-only or video-only content
      bOK = GetGroupDurationBuffered(nGroupKey, mediaType,
                                     nFullyDownloadedOccupancy,
                                     nPartiallyDownloadedOccupancy,
                                     nTotalOccupancy,
                                     nPlaybackPosition,
                                     nForwardMediaAvailability);
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "GetGroupPlaybackStats invalid starttime");
    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "Invalid GroupKey :%llu", nGroupKey);

    //GetDurationBuffered() takes care of audio-only or video-only content
    bOK = GetMediaDurationBuffered(mediaType,
                                   nFullyDownloadedOccupancy,
                                   nPartiallyDownloadedOccupancy,
                                   nTotalOccupancy,
                                   nPlaybackPosition,
                                   nForwardMediaAvailability);
  }

  MM_CriticalSection_Leave(m_pPeriodDataLock);

  if (bOK)
  {
    nPlaybackTime = nPlaybackPosition;
  }

  QTV_MSG_PRIO6(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
    "GetGroupPlaybackStats: ok %d: PG 0x%x, Media type %d Current Playback time = %llu Full Occ %u Partial Occ %u",
    bOK, (unsigned int)nGroupKey,mediaType, nPlaybackTime, nFullyDownloadedOccupancy, nPartiallyDownloadedOccupancy);

  status = (bOK ? QSM::DSP_STATUS_OK : QSM::DSP_STATUS_FAILURE);

  return status;
}

/** @brief Get current buffer occupancy and Current Playback
  *        Time
  *
  * @param[out] nDuration - Buffer duration (in msec)
    @param[out] nPlaybackTime - playback time (in msec)
  * @return
  * DSP_STATUS_OK - Success
  * DSP_STATUS_FAILURE - Failure
  */
QSM::IDataStateProviderStatus DASHMediaPeriodHandler::GetGroupPlaybackStats
(
  uint64 /* nGroupKey */,
  uint64 & /* nOccupancy */,
  uint64& /* nPlaybackTime */
)
{
  // Unused
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
               "GetGroupPlaybackStats: deprecated API called");
  return QSM::DSP_STATUS_FAILURE;
}

/**
 * Sets the QSM history on this period
 */
void DASHMediaPeriodHandler::SetQsmHistory(uint8 *pHistory, uint32 size)
{
  //Pass QSM history from previous QSM if available
  if(m_pQSM)
  {
    if(pHistory && size > 0)
    {
      m_pQSM->SeedHistory(pHistory, size);
    }
  }
}


void DASHMediaPeriodHandler::SetEnableInitialRateEstimation(bool val)
{
  if(m_pQSM)
  {
    m_pQSM->SetEnableInitialRateEstimation(val);
  }
}


void DASHMediaPeriodHandler::SuspendQSM()
{
  PeriodCmdData cmd;
  cmd.sBaseCmdData.eCmd = PERIOD_CMD_SUSPEND;

  if (!m_cCmdQ.EnQ(cmd))
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Period [0x%02x]: PERIOD_CMD_SUSPEND cmd cannot be queued",
                   (uint32)((m_cPeriodInfo.getPeriodKey() & 0xFF00000000000000ULL) >> 56) );
  }
}

bool DASHMediaPeriodHandler::ProcessSuspendQSM()
{
  bool rslt = false;

  if (m_pQSM)
  {
    QSM::QsmStatus qsmStatus = m_pQSM->Suspend();
    rslt = (QSM::QSM_STATUS_OK == qsmStatus ? true : false);
  }

  if (rslt)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Suspended period %llu", GetPeriodKey());
    m_bIsQSMSuspended = true;
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Failed to suspend QSM for period %llu", GetPeriodKey());
  }

  return rslt;
}

bool DASHMediaPeriodHandler::IsOpenCompleted()
{
  bool bResult = false;

  PeriodBaseStateHandler* pCurrentStateHandler = GetStateHandler();
  if (pCurrentStateHandler)
  {
    bResult = pCurrentStateHandler->IsOpenCompleted();
  }

  return bResult;
}

bool DASHMediaPeriodHandler::IsReadable(uint32 majorType)
{
  bool rslt = false;
  HTTPCommon::HTTPMediaType mediaType = (HTTPCommon::HTTPMediaType)majorType;

  MM_CriticalSection_Enter(m_pPeriodDataLock);
  DASHMediaPlayGroup* pPlayGroup = GetPlayGroup(mediaType);

  if (pPlayGroup)
  {
    if (pPlayGroup->IsReadable())
    {
      rslt = true;
    }
  }

  MM_CriticalSection_Leave(m_pPeriodDataLock);

  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "IsReadable for period %u, val %d",
    (unsigned int)((GetPeriodKey() & (uint64)MPD_PERIOD_MASK) >> 56), rslt);

  return rslt;
}

DASHMediaPlayGroup *DASHMediaPeriodHandler::GetPlayGrpByKey(uint64 nGroupKey)
{
  DASHMediaPlayGroup *pPlayGroup = NULL;

  for (uint32 i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
  {
    RepGroupQ &rQ = m_RepGroupQ[i];
    if (!rQ.IsEmpty())
    {
      RepGroupQElem *pQElem = m_RepGroupQ[i].GetElemByKey(nGroupKey);
      if (pQElem)
      {
        pPlayGroup = pQElem->m_pRepGroup;
        break;
      }
    }
  }

  return pPlayGroup;
}

void DASHMediaPeriodHandler::GetPlayGrpByRepresentationInfoKey(uint64 rRepresentationGrpInfoKey,
                                                               IPStreamList<uint64>& rDMPGKeyList)
{
  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    RepGroupQ& rQ = m_RepGroupQ[i];
    if (!rQ.IsEmpty())
    {
      rQ.GetPlayGrpByRepresentationInfoKey(rRepresentationGrpInfoKey, rDMPGKeyList);
    }
  }
}

int DASHMediaPeriodHandler::GetNumGroups()
{
  int nNumGroups = 0;

  for (int i = 0; i <= QSM::MAJOR_TYPE_MAX; ++i)
  {
    if (m_RepGroupQ[i].GetLastCommittedElem())
    {
      ++nNumGroups;
    }
  }

  return nNumGroups;
}

void DASHMediaPeriodHandler::DeleteElementByPlayGrpKey(uint64 nPlayGrpKey)
{
  // does not need critsect as it is called by GetNextMediaSample which has crit-sect.
  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    RepGroupQ& rQ = m_RepGroupQ[i];
    if (!rQ.IsEmpty())
    {
      if (rQ.DeleteElementByPlayGroupKey(nPlayGrpKey))
      {
        PeriodCmdData cmd;
        cmd.sBaseCmdData.eCmd = PERIOD_CMD_PURGE_ADAPTATIONSET_Q;
        if (!m_cCmdQ.EnQ(cmd))
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "DeleteElementByPlayGrpKey: Failed to queue cmd to purge playgrp %llu",
            nPlayGrpKey);
        }

        break;
      }
    }
  }
}

void DASHMediaPeriodHandler::PurgeAdaptationSetQ()
{
  MM_CriticalSection_Enter(m_pPeriodDataLock);
  for (uint32 i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
  {
    m_RepGroupQ[i].PurgeAdaptationSetQ();
  }
  MM_CriticalSection_Leave(m_pPeriodDataLock);
}

/**
 * c'tor
 *
 * @param pRepGroup
 */
DASHMediaPeriodHandler::RepGroupQElem::RepGroupQElem(DASHMediaPlayGroup *pRepGroup) :
  m_pRepGroup(pRepGroup),
  m_bIsCommitted(false),
  m_ChangeType(NONE)
{

}

/**
 * d'tor
 */
DASHMediaPeriodHandler::RepGroupQElem::~RepGroupQElem()
{

}

/**
 * Mark the playgroup as 'readable'.
 */
void DASHMediaPeriodHandler::RepGroupQElem::Commit()
{
  m_bIsCommitted = true;
}

/**
 * Check if the playgroup is 'readable'.
 */
bool DASHMediaPeriodHandler::RepGroupQElem::IsCommitted() const
{
  return m_bIsCommitted;
}

/**
 * Get the AdaptationSetChangeType that is pending along the
 * read path.
 */
DASHMediaPeriodHandler::RepGroupQElem::AdaptationSetChangeType
DASHMediaPeriodHandler::RepGroupQElem::GetAdaptationSetChangeType() const
{
  return m_ChangeType;
}

/**
 * Mark the AdaptationSetChangeType action that is pending along
 * the read path.
 */
void DASHMediaPeriodHandler::RepGroupQElem::MarkChangeType(AdaptationSetChangeType changeType)
{
  m_ChangeType = changeType;
}

bool DASHMediaPeriodHandler::RepGroupQElem::IsMarkedForRemove() const
{
  return (REMOVE == m_ChangeType) ? true : false;
}

DASHMediaPeriodHandler::RepGroupQ::RepGroupQ()
{

}

DASHMediaPeriodHandler::RepGroupQ::~RepGroupQ()
{
  // Make sure its all deleted.
  Shutdown();
}

/**
 * Add a new playgrp to the Q.
 */
bool DASHMediaPeriodHandler::RepGroupQ::Push(RepGroupQElem *pQElem)
{
  return m_RepGroupQ.Push(pQElem);
}

void DASHMediaPeriodHandler::RepGroupQ::Pop()
{
  m_RepGroupQ.Pop();
}

/**
 * Get a handle to the first elem in the Q.
 */
DASHMediaPeriodHandler::RepGroupQElem* DASHMediaPeriodHandler::RepGroupQ::PeekHead()
{
  RepGroupQElem* pHead = NULL;
  (void)m_RepGroupQ.PeekHead(pHead);
  return pHead;
}

void DASHMediaPeriodHandler::RepGroupQ::PeekFirstAndSecondElem(
   RepGroupQElem*& pFirst, RepGroupQElem*& pSecond)
{
  pFirst = NULL;
  pSecond = NULL;

  IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

  if (!m_RepGroupQ.IsEmpty())
  {
    for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
          iter != iterEnd; iter = iter.Next())
    {
      if ((*iter)->IsCommitted() &&
          (*iter)->GetAdaptationSetChangeType() != RepGroupQElem::PURGE &&
          (*iter)->GetAdaptationSetChangeType() != RepGroupQElem::REMOVE)
      {
        pFirst = *iter;
        iter = iter.Next();

        if (m_RepGroupQ.End() != iter)
        {
          pSecond = *iter;
        }

        break;
      }
    }
  }
}

/**
 * Get a handle to the last element in the Q.
 */
DASHMediaPeriodHandler::RepGroupQElem* DASHMediaPeriodHandler::RepGroupQ::PeekTail()
{
  RepGroupQElem* pTail = NULL;
  (void)m_RepGroupQ.PeekTail(pTail);
  return pTail;
}

/**
 * Return handle to first element in Q which in use. It need not
 * be committed. Eg, playback time should return true as soon as
 * SLCT was called on the playgrp and false before that.
 */
DASHMediaPeriodHandler::RepGroupQElem *DASHMediaPeriodHandler::RepGroupQ::PeekFirstElementInUse()
{
  RepGroupQElem *pElem = NULL;

  if (!m_RepGroupQ.IsEmpty())
  {
    IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();
    for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
          iter != iterEnd; iter = iter.Next())
    {
      RepGroupQElem *pCur = *iter;
      if (pCur)
      {
        if (pCur->GetAdaptationSetChangeType() != RepGroupQElem::REMOVE &&
            pCur->GetAdaptationSetChangeType() != RepGroupQElem::PURGE)
        {
          pElem = pCur;
          break;
        }
      }
    }
  }

  return pElem;
}


DASHMediaPeriodHandler::RepGroupQElem *DASHMediaPeriodHandler::RepGroupQ::PeekFirstElementForRead()
{
  RepGroupQElem *pFirst = NULL, *p2nd = NULL;
  PeekFirstAndSecondElem(pFirst,p2nd);
  return pFirst;
}

/**
 * Get a handle to the last element which is readable in the Q.
 */
DASHMediaPeriodHandler::RepGroupQElem* DASHMediaPeriodHandler::RepGroupQ::GetLastCommittedElem()
{
  RepGroupQElem *pElem = NULL;

  if (!m_RepGroupQ.IsEmpty())
  {
    IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
    if ((*iter)->IsCommitted())
    {
      IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

      for (; iter != iterEnd; iter = iter.Next())
      {
        pElem = *iter;

        IPStreamList<RepGroupQElem *>::Iterator iterNext = iter.Next();

        if (iterNext != iterEnd)
        {
          RepGroupQElem *pNextElem = *iterNext;

          if (!pNextElem->IsCommitted())
          {
            break;
          }
        }
      }
    }
  }

  return pElem;
}

/**
 * Get a handle to the Q element associated with the playgroup
 * with key nGroupKey.
 */
DASHMediaPeriodHandler::RepGroupQElem* DASHMediaPeriodHandler::RepGroupQ::GetElemByKey(uint64 nGroupKey)
{
  RepGroupQElem* pElem = NULL;

  if (!m_RepGroupQ.IsEmpty())
  {
    IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

    for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
         iter != iterEnd; iter = iter.Next())
    {
      RepGroupQElem* pCurElem = *iter;
      DASHMediaPlayGroup *pPlayGrp = pCurElem->m_pRepGroup;

      if (pPlayGrp)
      {
        if (pPlayGrp->GetKey() == nGroupKey)
        {
          pElem = pCurElem;
          break;
        }
      }
    }
  }

  return pElem;
}

/**
 * De-initialize and delete all playgroups, and delete the
 * RepGroupQElem assocaited with the playgroups.
 */
void DASHMediaPeriodHandler::RepGroupQ::Shutdown()
{
  //Close all playgroups
  RepGroupQElem *pElem = NULL;
  m_RepGroupQ.PeekHead(pElem);

  while (pElem)
  {
    DASHMediaPlayGroup* pPlayGroup = pElem->m_pRepGroup;

    if (pPlayGroup)
    {
      (void)pPlayGroup->Close();
      pPlayGroup->DeInit();
      QTV_Delete(pPlayGroup);
    }

    m_RepGroupQ.Pop();

    QTV_Delete(pElem);
    pElem = NULL;
    m_RepGroupQ.PeekHead(pElem);
  }
}

/**
 * Check if the Q is empty.
 */
bool DASHMediaPeriodHandler::RepGroupQ::IsEmpty()
{
  return m_RepGroupQ.IsEmpty();
}

/**
 * For debugging.
 */
void DASHMediaPeriodHandler::RepGroupQ::Print()
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "RepGroupQ::Print");

  IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

  for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
       iter != iterEnd; iter = iter.Next())
  {
    DASHMediaPlayGroup* pPlayGroup = (*iter)->m_pRepGroup;
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "### repGrpKey %llu, isCommitted %d",
                  pPlayGroup->GetKey(), (*iter)->IsCommitted() );
  }
}

/**
 * Return true if the first playgroup in the repGroupQ is
 * readable. This should be called on a period transition for
 * the media type only. Eg, on a period change, we need to wait
 * before the first playgroup for the media type is readable.
 */
bool DASHMediaPeriodHandler::RepGroupQ::IsSwitchableTo()
{
  bool rslt = false;

  IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();
  for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
        iter != iterEnd; iter = iter.Next())
  {
    RepGroupQElem *pElem = *iter;
    if (pElem)
    {
      DASHMediaPlayGroup *pPlayGrp = pElem->m_pRepGroup;
      if (pPlayGrp)
      {
        if (pElem->IsCommitted() == false)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "IsSwitchableTo returning false as pg %llu is not committed",
            pPlayGrp->GetKey());

          break;
        }

        if (pElem->GetAdaptationSetChangeType() == RepGroupQElem::NONE ||
            pElem->GetAdaptationSetChangeType() == RepGroupQElem::ADD ||
            pElem->GetAdaptationSetChangeType() == RepGroupQElem::REPLACE)
        {
          if (pPlayGrp->IsReadable(true) == true)
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "IsSwitchableTo returning true as pg %llu isreadable",
              pPlayGrp->GetKey());
            rslt = true;
          }

          break;
        }
      }
    }
  }

  return rslt;
}

void DASHMediaPeriodHandler::RepGroupQ::QueueOptimize()
{
  bool bLoop = true;

  while (bLoop && m_RepGroupQ.Size() > 2)
  {
    bLoop = false;

    IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();
    for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin().Next();
          iter != iterEnd; iter = iter.Next())
    {
      DASHMediaPlayGroup *pPlayGrp = (*iter)->m_pRepGroup;
      if ((*iter)->GetAdaptationSetChangeType() != RepGroupQElem::PURGE)
      {
        IPStreamList<RepGroupQElem *>::Iterator iterNext = iter.Next();
        if (iterNext != iterEnd)
        {
          DASHMediaPlayGroup *pNextPlayGrp = (*iterNext)->m_pRepGroup;
          if (pNextPlayGrp->IsReadable(true))
          {
            if (false == pPlayGrp->IsReadable(true) ||
                pPlayGrp->GetStartTime() >= pNextPlayGrp->GetStartTime())
            {
              QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "AdaptationSet change: QueueOptimize Optmize by deleting playGrp 0x%x%x "
                "as pg not readable or pg start %llu >= pg_next start %llu",
                (unsigned int)(pPlayGrp->GetKey() >> 32),
                (unsigned int)(pPlayGrp->GetKey()),
                pPlayGrp->GetStartTime(), pNextPlayGrp->GetStartTime());

              RepGroupQElem *pPurgedElem = *iter;
              pPurgedElem->MarkChangeType(RepGroupQElem::PURGE);
              m_RepGroupQ.PushFront(pPurgedElem);
              iter.Erase();

              bLoop = true;
              break;
            }
          }
        }
      }
    }
  }
}

bool DASHMediaPeriodHandler::RepGroupQ::IsPendingSwitchDLSwitchNotificationForAdd()
{
  bool rslt = false;

  RepGroupQElem *pHead = PeekHead();
  if (pHead)
  {
    if (RepGroupQElem::ADD == pHead->GetAdaptationSetChangeType())
    {
      DASHMediaPlayGroup *pPlayGrp = pHead->m_pRepGroup;
      if (pPlayGrp)
      {
        if (pPlayGrp->IsReadable())
        {
          rslt = true;
        }
      }
    }
  }

  return rslt;
}

bool DASHMediaPeriodHandler::RepGroupQ::IsPendingSwitchDLSwitchNotificationForRemove(uint32 eQsmMajorType)
{
  bool rslt = false;

  RepGroupQElem *pHead = PeekHead();
  if (pHead)
  {
    if (RepGroupQElem::REMOVE == pHead->GetAdaptationSetChangeType())
    {
      DASHMediaPlayGroup *pRemovedPlayGrp = pHead->m_pRepGroup;
      if (pRemovedPlayGrp)
      {
        if (eQsmMajorType & (uint32)pRemovedPlayGrp->m_nMajorType)
        {
          rslt = IsFirstElementWaitingForRemove();
        }
      }
    }
  }

  return rslt;
}

/**
 * Commit the elements from 'read' perspective. If an element
 * was added, the switch notification was sent so this can be
 * marked readable. If an element was removed, then it is okay
 * to remove it as switch notification was sent.
 */
void DASHMediaPeriodHandler::RepGroupQ::DLSwitchNotificationSent()
{
  RepGroupQElem *pHead = PeekHead();
  if (pHead)
  {
    if (RepGroupQElem::ADD == pHead->GetAdaptationSetChangeType())
    {
      pHead->MarkChangeType(RepGroupQElem::NONE);
    }
    else if (RepGroupQElem::REMOVE == pHead->GetAdaptationSetChangeType())
    {
      RemoveFirstElementFromQ();
    }
  }
}

bool DASHMediaPeriodHandler::RepGroupQ::IsReadable()
{
  bool rslt = false;

  if (!IsEmpty())
  {
    RepGroupQElem *pElem = *(m_RepGroupQ.Begin());
    if (pElem)
    {
      DASHMediaPlayGroup *pPlayGrp = pElem->m_pRepGroup;
      if (pPlayGrp)
      {
        if (pPlayGrp->IsReadable())
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "AdaptationSetChange: Playgrp with key %llu is readable",
                        pPlayGrp->GetKey());
          rslt = true;
        }
      }
    }
  }

  return rslt;
}

bool DASHMediaPeriodHandler::RepGroupQ::IsFirstElementWaitingForRemove()
{
  bool rslt = false;

  if (!m_RepGroupQ.IsEmpty())
  {
    IPStreamList<RepGroupQElem *>::Iterator iterHead = m_RepGroupQ.Begin();
    RepGroupQElem *pElem = *iterHead;
    if (pElem)
    {
      if (pElem->IsMarkedForRemove())
      {
        rslt = true;
      }
    }
  }

  return rslt;
}

void DASHMediaPeriodHandler::RepGroupQ::RemoveFirstElementFromQ()
{
  if (!m_RepGroupQ.IsEmpty())
  {
    IPStreamList<RepGroupQElem *>::Iterator iterHead = m_RepGroupQ.Begin();
    RepGroupQElem *pElem = *iterHead;
    if (pElem)
    {
      if (pElem->IsMarkedForRemove())
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "AdaptationSetChange: Mark regroup %llu for purge",
                      pElem->m_pRepGroup->GetKey());
        pElem->MarkChangeType(RepGroupQElem::PURGE);
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "RemoveFirstElementFromQ: Element with playgrp key %llu not marked for remove",
          pElem->m_pRepGroup->GetKey());
      }
    }
  }
}

bool DASHMediaPeriodHandler::RepGroupQ::RemoveLastElementFromQ()
{
  bool rslt = false;
  if (!m_RepGroupQ.IsEmpty())
  {
    IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
    IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

    if (m_RepGroupQ.Size() > 1)
    {
      for (; iter.Next() != iterEnd; iter = iter.Next())
      {
        if (iter.Next() == iterEnd)
        {
          break;
        }
      }
    }

    if (iter != iterEnd)
    {
      RepGroupQElem *pElem = *iter;
      if (pElem)
      {
        if (!pElem->IsCommitted())
        {
          DASHMediaPlayGroup *pPlayGrp = pElem->m_pRepGroup;
          if (pPlayGrp)
          {
            QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "AdaptationSetChange: Rollback. Delete element with "
                          "key 0x%x%x, adaptationChangeState %d",
                          (unsigned int)(pPlayGrp->GetKey() >> 32), (unsigned int)pPlayGrp->GetKey(),
                          pElem->GetAdaptationSetChangeType());
            iter.Erase();

            rslt = true;
          }
        }
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Failed to remove last element from DASMQ");
    }
  }

  return rslt;
}

void DASHMediaPeriodHandler::RepGroupQ::RemoveAllButLastElementFromQ()
{
  IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

  for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
       iter != iterEnd; iter = iter.Next())
  {
    if (iter.Next() != iterEnd)
    {
      (*iter)->MarkChangeType(RepGroupQElem::PURGE);
    }
  }

  PurgeAdaptationSetQ();
}

void DASHMediaPeriodHandler::RepGroupQ::GroupRequestsCompleted(
   uint64 nRelPeriodEndTime)
{
  IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

  for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
       iter != iterEnd; iter = iter.Next())
  {
    if ((*iter)->GetAdaptationSetChangeType() == RepGroupQElem::PURGE)
    {
      continue;
    }

    DASHMediaPlayGroup* pPlayGroup = (*iter)->m_pRepGroup;

    if (pPlayGroup)
    {
      (void)pPlayGroup->GroupRequestsCompleted(nRelPeriodEndTime);
    }
  }
}

bool DASHMediaPeriodHandler::RepGroupQ::IsEndOfPeriod()
{
  bool bEOF = true;
  RepGroupQElem *pLastElem = NULL;
  if (m_RepGroupQ.PeekTail(pLastElem))
  {
    if (pLastElem)
    {
      if (false == pLastElem->IsCommitted())
      {
        bEOF = false;
      }
    }
  }

  if (bEOF && !m_RepGroupQ.IsEmpty())
  {
    IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();
    for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
          iter != iterEnd; iter = iter.Next())
    {
      DASHMediaPlayGroup *pPlayGrp = (*iter)->m_pRepGroup;

      if (pPlayGrp)
      {
        if (pPlayGrp->IsValid())
        {
          bEOF = pPlayGrp->IsDownloadComplete();

          if(!bEOF)
          {
            break;
          }
        }
      }
    }
  }

  return bEOF;
}

void DASHMediaPeriodHandler::RepGroupQ::GetPlayGrpByRepresentationInfoKey(
   uint64 nRepresentationGrpInfoKey, IPStreamList<uint64>& rDMPGKeyList)
{
  IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();
  for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
        iter != iterEnd; iter = iter.Next())
  {
    RepGroupQElem *pElem = *iter;
    DASHMediaPlayGroup *pPlayGrp = pElem->m_pRepGroup;
    RepresentationGroup& rInfo = pPlayGrp->m_cRepGroupInfo;
    if (rInfo.getKey() == nRepresentationGrpInfoKey)
    {
      rDMPGKeyList.Push(pPlayGrp->GetKey());
    }
  }
}

void DASHMediaPeriodHandler::RepGroupQ::MarkRemoved(uint64 nRepresentationGrpInfoKey)
{
  RepGroupQElem *pTailElem = NULL;
  m_RepGroupQ.PeekTail(pTailElem);

  if (pTailElem)
  {
    DASHMediaPlayGroup *pPlayGrp = pTailElem->m_pRepGroup;
    RepresentationGroup& rInfo = pPlayGrp->m_cRepGroupInfo;
    if (nRepresentationGrpInfoKey == rInfo.getKey())
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "AdaptationSetChange: MarkRemoved with repGrp key %llu, from Q with majorType %d",
                    nRepresentationGrpInfoKey, (int)pPlayGrp->m_nMajorType);
      pTailElem->MarkChangeType(RepGroupQElem::REMOVE);
    }
  }
}

bool DASHMediaPeriodHandler::RepGroupQ::DeleteElementByPlayGroupKey(uint64 nPlayGrpKey)
{
  bool rslt = false;
  IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();
  for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
        iter != iterEnd; iter = iter.Next())
  {
    RepGroupQElem *pElem = *iter;
    DASHMediaPlayGroup *pPlayGrp = pElem->m_pRepGroup;
    if (nPlayGrpKey == pPlayGrp->GetKey())
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "AdaptationSetChange: DeleteElementByPlayGroupKey with playGrp key %llu, from Q with majorType %d",
                    nPlayGrpKey, (int)pPlayGrp->m_nMajorType);
      (*iter)->MarkChangeType(RepGroupQElem::PURGE);

      rslt = true;
      break;
    }
  }

  return rslt;
}

void DASHMediaPeriodHandler::RepGroupQ::GetPlaybackStatsForQ(
   HTTPCommon::HTTPMediaType majorType,
   uint32& nFullyDownloadedOccupancy,
   uint32& nPartiallyDownloadedOccupancy,
   uint32& nTotalOccupancy,
   uint64& nPlaybackPosition,
   uint32& nForwardMediaAvailability)
{
  nFullyDownloadedOccupancy = 0;
  nPartiallyDownloadedOccupancy = 0;
  nTotalOccupancy = 0;
  nPlaybackPosition = 0;
  nForwardMediaAvailability = 0;

  if (!m_RepGroupQ.IsEmpty())
  {
    bool bIsPbTimeSet = false;
    IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();
    for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
          iter != iterEnd; iter = iter.Next())
    {
      if ((*iter)->IsMarkedForRemove() ||
          (*iter)->GetAdaptationSetChangeType() == RepGroupQElem::PURGE)
      {
        continue;
      }

      DASHMediaPlayGroup *pPlayGrp = (*iter)->m_pRepGroup;

      if (pPlayGrp)
      {
        if (pPlayGrp->IsStartTimeInitialized())
        {
          uint32 nCurFullyDownloadedOccupancy;
          uint32 nCurPartiallyDownloadedOccupancy;
          uint32 nCurTotalOccupancy;
          uint64 nCurPlaybackPosition;
          uint32 nCurForwardMediaAvailability;

          bool bResult = pPlayGrp->GetGroupPlaybackStats(
             majorType,
             nCurFullyDownloadedOccupancy, nCurPartiallyDownloadedOccupancy,
             nCurTotalOccupancy, nCurPlaybackPosition, nCurForwardMediaAvailability);
          nFullyDownloadedOccupancy += nCurFullyDownloadedOccupancy;
          nPartiallyDownloadedOccupancy += nCurPartiallyDownloadedOccupancy;
          nTotalOccupancy += nCurTotalOccupancy;

          if (false == bIsPbTimeSet)
          {
            nPlaybackPosition = nCurPlaybackPosition;
            bIsPbTimeSet = true;
          }
          nForwardMediaAvailability = nCurForwardMediaAvailability;
        }
      }
    }
  }
}

/**
 * Clear buffered data for playgroups other that gr with key
 * nExceptPlayGrpKey.
 */
void DASHMediaPeriodHandler::RepGroupQ::ClearBufferedData(uint64 nTime, uint64 nExceptPlayGrpKey)
{
  IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

  for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
        iter != m_RepGroupQ.End(); iter = iter.Next())
  {
    if ((*iter)->GetAdaptationSetChangeType() == RepGroupQElem::PURGE)
    {
      continue;
    }

    DASHMediaPlayGroup *pPlayGrp = (*iter)->m_pRepGroup;
    if (pPlayGrp)
    {
      if (pPlayGrp->GetKey() != nExceptPlayGrpKey)
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "ClearBufferedData for pg %llu, from time %llu",
          pPlayGrp->GetKey(), nTime);
        pPlayGrp->ClearBufferedData(nTime);

        if (false == pPlayGrp->IsReadable(true))
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "AdaptationSet change: repGrp with key %llu "
            "not readable after ClearBufferedData", pPlayGrp->GetKey());
        }
      }
    }
  }
}

void DASHMediaPeriodHandler::RepGroupQ::PurgeAdaptationSetQ()
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "PurgeAdaptationSetQ");
  bool bLoop = true;
  while (bLoop && !m_RepGroupQ.IsEmpty())
  {
    bLoop = false;
    IPStreamList<RepGroupQElem *>::Iterator iterEnd = m_RepGroupQ.End();

    for (IPStreamList<RepGroupQElem *>::Iterator iter = m_RepGroupQ.Begin();
          iter != iterEnd; iter = iter.Next())
    {
      RepGroupQElem *pRepGroupElem = *iter;
      if (pRepGroupElem)
      {
        if (pRepGroupElem->GetAdaptationSetChangeType() == RepGroupQElem::PURGE)
        {
          DASHMediaPlayGroup *pPlayGroup = pRepGroupElem->m_pRepGroup;
          if (pPlayGroup)
          {
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "PurgeAdaptationSet with playgrp %llu", pPlayGroup->GetKey());

            (void)pPlayGroup->Close();
            pPlayGroup->DeInit();
            QTV_Delete(pPlayGroup);
            QTV_Delete(pRepGroupElem);
          }

          iter.Erase();
          bLoop = true;
          break;
        }
      }
    }
  }
}

bool DASHMediaPeriodHandler::RepGroupQ::CanPlaybackUninterrupted()
{
  bool bOk = false;
  RepGroupQElem *pFirstElem = NULL;

  //Redirect to head of the queue
  if (m_RepGroupQ.PeekHead(pFirstElem) && pFirstElem && pFirstElem->m_pRepGroup)
  {
    bOk = pFirstElem->m_pRepGroup->CanPlaybackUninterrupted();
  }

  return bOk;
}

void DASHMediaPeriodHandler::GetCumulativeDurationBuffered(
   HTTPCommon::HTTPMediaType majorType,
   uint64 &nPlaybackPosition, uint64 &nBuffDur)
{
  nBuffDur = 0;
  uint32 nFullyDownloadedOccupancy = 0;
  uint32 nPartiallyDownloadedOccupancy = 0;
  uint32 nTotalOccupancy = 0;
  nPlaybackPosition = 0;
  uint32 nForwardMediaAvailability = 0;
  if (HTTPCommon::HTTP_AUDIO_TYPE == majorType ||
      HTTPCommon::HTTP_VIDEO_TYPE == majorType ||
      HTTPCommon::HTTP_TEXT_TYPE == majorType)
  {
    MM_CriticalSection_Enter(m_pPeriodDataLock);

    uint32 nMajorType = GetQSMMajorType(majorType);

    for (int i = 0; i < QSM::MAJOR_TYPE_MAX; ++i)
    {
      RepGroupQ& rQ = m_RepGroupQ[i];
      if (nMajorType & i)
      {
        rQ.GetPlaybackStatsForQ(majorType, nFullyDownloadedOccupancy,nPartiallyDownloadedOccupancy,
                                nTotalOccupancy, nPlaybackPosition, nForwardMediaAvailability);
        nBuffDur =  nFullyDownloadedOccupancy;
        break;
      }
    }

    MM_CriticalSection_Leave(m_pPeriodDataLock);
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "GetCumulativeDurationBuffered Unknown majorType %d", majorType);
  }
}

DASHMediaPeriodHandler::DASMQElem::DASMQElem() :
  m_MajorType(QSM::MAJOR_TYPE_UNKNOWN),
  m_nOldRepGrpInfoKey(MAX_UINT64_VAL),
  m_nNewRepGrpInfoKey(MAX_UINT64_VAL)
{

}

DASHMediaPeriodHandler::DASMQElem::~DASMQElem()
{

}

void DASHMediaPeriodHandler::DASMQElem::SetMajorType(uint32 nQsmMajorType)
{
  m_MajorType = nQsmMajorType;
}

void DASHMediaPeriodHandler::DASMQElem::SetOldRepGrpInfoKey(uint64 nKey)
{
  m_nOldRepGrpInfoKey = nKey;
}

void DASHMediaPeriodHandler::DASMQElem::SetNewRepGrpInfoKey(uint64 nKey)
{
  m_nNewRepGrpInfoKey = nKey;
}

bool DASHMediaPeriodHandler::DASMQElem::AddOldRepInfoKey(uint64 nKey)
{
  return m_OldSelectedRepInfoList.Push(nKey);
}

bool DASHMediaPeriodHandler::DASMQElem::AddNewRepInfoKey(uint64 nKey)
{
  return m_NewSelectedRepInfoList.Push(nKey);
}

uint32 DASHMediaPeriodHandler::DASMQElem::GetMajorType() const
{
  return m_MajorType;
}

uint64 DASHMediaPeriodHandler::DASMQElem::GetOldRepGrpInfoKey() const
{
  return m_nOldRepGrpInfoKey;
}

uint64 DASHMediaPeriodHandler::DASMQElem::GetNewRepGrpInfoKey() const
{
  return m_nNewRepGrpInfoKey;
}

bool DASHMediaPeriodHandler::DASMQ::Push(DASMQElem *pElem)
{
  return m_Q.Push(pElem);
}

DASHMediaPeriodHandler::DASMQ::DASMQ()
{

}

DASHMediaPeriodHandler::DASMQ::~DASMQ()
{
  while (!m_Q.IsEmpty())
  {
    DASMQElem *pElem = NULL;
    m_Q.PeekHead(pElem);
    if (pElem)
    {
      QTV_Delete(pElem);
    }

    m_Q.Pop();
  }
}

DASHMediaPeriodHandler::DASMQElem *DASHMediaPeriodHandler::DASMQ::PeekHeadElem()
{
  DASMQElem *pHeadElem = NULL;
  if (!m_Q.IsEmpty())
  {
    m_Q.PeekHead(pHeadElem);
  }

  return pHeadElem;
}

void DASHMediaPeriodHandler::DASMQ::Pop()
{
  if (!m_Q.IsEmpty())
  {
    m_Q.Pop();
  }
}

void DASHMediaPeriodHandler::DASMQ::PopTail()
{
  if (!m_Q.IsEmpty())
  {
      IPStreamList<DASMQElem *>::Iterator iter = m_Q.Begin();
      IPStreamList<DASMQElem *>::Iterator iterEnd = m_Q.End();
    while(iter.Next() != iterEnd)
    {
      iter = iter.Next();
    }

    iter.Erase();
  }
}

bool DASHMediaPeriodHandler::DASMQ::IsEmpty()
{
  return m_Q.IsEmpty();
}

} // namespace video
