/************************************************************************* */
/**
 * DASHMediaPlayGroup.cpp
 * @brief Implements the DASHMediaPlayGroup. Each such object handles a
 *        DASH play group. A play group is nothing but a selected DASH
 *        representation group and there can be at most one such play
 *        group per media type if media types are grouped separately
 *        in MPD.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/DASHMediaPlayGroup.cpp#86 $
$DateTime: 2013/08/16 11:51:00 $
$Change: 4287091 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "DASHMediaPlayGroup.h"

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

  /*
  * Reset the structure elements
  */
void DASHMediaPlayGroup::HTTPDataUnitElement::Reset()
{
  std_memset(&link, 0x0, sizeof(ordered_StreamList_link_type));
  nRepKey      = 0;
  nDataUnitKey = 0;
  nStartTime   = 0;
  nDuration    = 0;
  nPrevPlayPos = 0;
}

/** @brief Initialize DASHMediaPlayGroup.
  *
  * @param[in] nKey - Group key
  * @param[in] nMajorType - Major type (A, V, AV etc.)
  * @param[in] cRepGroup - Reference to rep group info
  * @param[in] pDASHSessionInfo - Reference to DASH session info
  * @param[in] pNotifier - Reference to period notifier
  * @param[in] pScheduler - Reference to task scheduler
  * @return
  * TRUE - Initialization successful
  * FALSE - Otherwise
  */
bool DASHMediaPlayGroup::Init
(
 const uint32 nKey,
 const uint32 nMajorType,
 RepresentationGroup& cRepGroup,
 DASHSessionInfo* pDASHSessionInfo,
 iPeriodNotifier* pNotifier,
 Scheduler* pScheduler
)
{
  bool bOk = false;
  m_nKey = nKey;
  m_nMajorType = nMajorType;
  m_cRepGroupInfo = cRepGroup;
  m_pDASHSessionInfo = pDASHSessionInfo;
  m_pPeriodNotifier = pNotifier;
  m_pScheduler = pScheduler;
  m_bEstimator = m_cRepGroupInfo.m_bEstimator;
  m_bDownloadComplete = false;
  //Initialize media read store
  m_nNumMediaComponents = 0;
  uint32 majorType = nMajorType;
  m_bInvalidGroup = false;

  for (int i = 0; i < HTTP_MAX_MEDIA_COMPONENTS; i++)
  {
    if (majorType & QSM::MAJOR_TYPE_AUDIO)
    {
      m_cMediaReadStore[i].SetMediaType(HTTPCommon::HTTP_AUDIO_TYPE);
      majorType &= ~QSM::MAJOR_TYPE_AUDIO;
      m_nNumMediaComponents++;
    }
    else if (majorType & QSM::MAJOR_TYPE_VIDEO)
    {
      m_cMediaReadStore[i].SetMediaType(HTTPCommon::HTTP_VIDEO_TYPE);
      majorType &= ~QSM::MAJOR_TYPE_VIDEO;
      m_nNumMediaComponents++;
    }
    else if (majorType & QSM::MAJOR_TYPE_TEXT)
    {
      m_cMediaReadStore[i].SetMediaType(HTTPCommon::HTTP_TEXT_TYPE);
      majorType &= ~QSM::MAJOR_TYPE_TEXT;
      m_nNumMediaComponents++;
    }
  }

  bOk = (MM_CriticalSection_Create(&m_pGroupDataLock) == 0);


  if (bOk)
  {
    bOk = HTTPStackInterface::CreateInstance(&m_pHTTPStack, NULL,
                                             m_pDASHSessionInfo->sSessionInfo.GetCookieStore()) == 0;
    if (bOk)
    {
      bOk = HTTPCommon::ConfigureHTTPStack(m_pDASHSessionInfo->sSessionInfo, *m_pHTTPStack);
    }
  }
  //Get representation info for the group
  if (bOk)
  {
    bOk = false;
    uint32 nNumReps = 0;
    if(m_pDASHSessionInfo)
    {
      (void)(m_pDASHSessionInfo->cMPDParser).GetAllRepresentationsForGroup(NULL, nNumReps,
                                                                         m_cRepGroupInfo.getKey());
    }

    if (nNumReps > 0)
    {
      m_pRepresentation = QTV_New_Array(Representation, nNumReps);
      if (m_pRepresentation)
      {
        //ToDo: What's the key to pass to get a rep by key, then there's no need for
        //the temp pRepInfo array?
        /*
        for (int index = 0; index < m_nNumReps; index++)
        {
          GetRepresentationByKey(??, &m_pRepresentation[index].m_cRepInfo);
          if (!HTTPSUCCEEDED(status))
          {
            bOk = false;
            break;
          }
        }
        */

        RepresentationInfo* pRepInfo = QTV_New_Array(RepresentationInfo, nNumReps);
        if (pRepInfo)
        {
          HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
          if (m_pDASHSessionInfo)
          {
            status =
              (m_pDASHSessionInfo->cMPDParser).GetAllRepresentationsForGroup(pRepInfo, nNumReps,
                                                                             m_cRepGroupInfo.getKey());
          }
          if (HTTPSUCCEEDED(status))
          {
            bOk = true;
            m_nNumReps = (int)nNumReps;
            for (int index = 0; index < m_nNumReps; index++)
            {
              /* Passing Bandwidth Estmator object */
              pRepInfo[index].m_bEstimator = m_bEstimator;
              m_pRepresentation[index].m_cRepInfo = pRepInfo[index];
            }
          }
          QTV_Delete_Array(pRepInfo);
          pRepInfo = NULL;
        }
      }
    }// if (m_nNumReps > 0)
  }

  if(bOk)
  {
    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      m_cMediaReadStore[i].nFullyDownloadedOccupancy     = 0;
      m_cMediaReadStore[i].nPartiallyDownloadedOccupancy = 0;
      m_cMediaReadStore[i].nCurrentDownloadPosition      = 0;
      m_cMediaReadStore[i].m_dataUnitsLock               = 0;

      ordered_StreamList_init(&(m_cMediaReadStore[i].m_dataUnitsFreeList),
                              ORDERED_STREAMLIST_ASCENDING,
                              ORDERED_STREAMLIST_PUSH_SLT);
      ordered_StreamList_init(&(m_cMediaReadStore[i].m_dataUnitsInUseList),
                              ORDERED_STREAMLIST_ASCENDING,
                              ORDERED_STREAMLIST_PUSH_SLT);

      m_cMediaReadStore[i].m_dataUnits = (HTTPDataUnitElement*)QTV_Malloc(HTTP_MAX_DATA_UNITS * sizeof(HTTPDataUnitElement));
      if(m_cMediaReadStore[i].m_dataUnits)
      {
        // populate the free resources
        for( int j = 0; j < HTTP_MAX_DATA_UNITS; j++ )
        {
          m_cMediaReadStore[i].m_dataUnits[j].Reset();
          ordered_StreamList_push(&(m_cMediaReadStore[i].m_dataUnitsFreeList), &(m_cMediaReadStore[i].m_dataUnits[j].link), j);
        }
      }

      bOk = (MM_CriticalSection_Create(&(m_cMediaReadStore[i].m_dataUnitsLock)) == 0);
    }
  }

  if (bOk)
  {
    int64 nStartOffset = 0;
    int64 nEndOffset = -1;

    char* pURL = m_cRepGroupInfo.GetInitialisationSegmentUrl();
    (void)m_cRepGroupInfo.GetRangeInitialisationSegment(nStartOffset, nEndOffset);

    if (pURL)
    {
      m_pCommonInitSegment = QTV_New_Args(InitializationSegment,
                                          (pURL, nStartOffset, nEndOffset, *m_pDASHSessionInfo, m_pHTTPStack,
                                           m_bEstimator, InitializationSegment::INITSEG_STATE_IDLE, bOk));

      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "DASHMediaPlayGroup: Common Init is initialized %p", m_pCommonInitSegment);

      if(bOk && m_pCommonInitSegment)
      {
        m_pCommonInitSegment->setCommonInit();
      }
    }
  }

  return bOk;
}

/** @brief De-initialize DASHMediaPlayGroup.
  *
  */
void DASHMediaPlayGroup::DeInit()
{
  if (m_pRepresentation)
  {
    for (int i = 0; i < m_nNumReps; i++)
    {
      if (m_pRepresentation[i].m_pRepHandler)
      {
        QTV_Delete(m_pRepresentation[i].m_pRepHandler);
        m_pRepresentation[i].m_pRepHandler = NULL;
      }
    }

    QTV_Delete_Array(m_pRepresentation);
    m_pRepresentation = NULL;
  }

  if (m_pGroupDataLock)
  {
    (void)MM_CriticalSection_Release(m_pGroupDataLock);
    m_pGroupDataLock = NULL;
  }

  if (m_pHTTPStack)
  {
    QTV_Delete(m_pHTTPStack);
    m_pHTTPStack = NULL;
  }

  for (int i = 0; i < m_nNumMediaComponents; i++)
  {
    if(m_cMediaReadStore[i].m_dataUnits)
    {
      QTV_Free(m_cMediaReadStore[i].m_dataUnits);
      m_cMediaReadStore[i].m_dataUnits = NULL;
    }

    if (m_cMediaReadStore[i].m_dataUnitsLock)
    {
      (void)MM_CriticalSection_Release(m_cMediaReadStore[i].m_dataUnitsLock);
      m_cMediaReadStore[i].m_dataUnitsLock = NULL;
    }
  }

  if(m_pCommonInitSegment)
  {
    QTV_Delete(m_pCommonInitSegment);
    m_pCommonInitSegment = NULL;
  }
}

/** @brief Get representation info.
  *
  * @param[in] pRepInfo - Reference to rep info array
  * @param[in] nSizeOfRepInfo - Size of rep info array
  * @param[out] nNumRepInfo - Number of elements filled/needed
  * @return
  * HTTPDL_INSUFFICIENT_BUFFER - Insufficient buffer
  * HTTPDL_SUCCESS - Rep info filled
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPlayGroup::GetRepresentationInfo
(
 QSM::CRepresentationInfo* pRepInfo,
 uint32 nSizeOfRepInfo,
 uint32& nNumRepInfo
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  nNumRepInfo = 0;

  if (m_pRepresentation)
  {
    status = HTTPCommon::HTTPDL_SUCCESS;

    //ToDo: Filter out reps based on bitrate/source buffer reqs. if needed (only
    //supported reps are presented to QSM)
    for (int i = 0; i < m_nNumReps; i++)
    {
      //Populate representation attributes
      if (pRepInfo && nNumRepInfo < nSizeOfRepInfo)
      {
        pRepInfo[nNumRepInfo].m_nKey = i;
        Resolution* pResolution = m_pRepresentation[i].m_cRepInfo.getResolution();
        if (pResolution)
        {
          QSM::CResolution resolution(pResolution->width, pResolution->height);
          pRepInfo[nNumRepInfo].m_resolution = resolution;
        }
        uint32 bitrate = m_pRepresentation[i].m_cRepInfo.getBandwidth() / 1024;
        pRepInfo[nNumRepInfo].m_nBitrate = (bitrate > 0 ? bitrate : 1);
        pRepInfo[nNumRepInfo].m_nFrameRate = (uint32)m_pRepresentation[i].m_cRepInfo.getFrameRate();

        //minor type
        pRepInfo[nNumRepInfo].m_nMimeType = QSM::MINOR_TYPE_UNKNOWN;
        int nNumCodecs = 0;
        (void)m_pRepresentation[i].m_cRepInfo.getCodec(NULL, nNumCodecs);
        if (nNumCodecs > 0)
        {
          CodecInfo* pCodec = (CodecInfo *)QTV_Malloc(nNumCodecs * sizeof(CodecInfo));
          if (pCodec)
          {
            if (m_pRepresentation[i].m_cRepInfo.getCodec(pCodec, nNumCodecs))
            {
              for (int i = 0; i < nNumCodecs; i++)
              {
                if (pCodec[i].minorType == MN_TYPE_HE_AAC || pCodec[i].minorType == MN_TYPE_AAC_LC)
                {
                  pRepInfo[nNumRepInfo].m_nMimeType |= QSM::MINOR_TYPE_AAC;
                }
                else if (pCodec[i].minorType == MN_TYPE_AVC)
                {
                  pRepInfo[nNumRepInfo].m_nMimeType |= QSM::MINOR_TYPE_H264;
                }
                else if (pCodec[i].minorType == MN_TYPE_HVC)
                {
                  pRepInfo[nNumRepInfo].m_nMimeType |= QSM::MINOR_TYPE_HEVC;
                }
              }
            }
            QTV_Free(pCodec);
            pCodec = NULL;
          }
        }
      }

      nNumRepInfo++;
    }

    //nNumRepInfo contains total number of filtered reps and info about the first
    //MIN(nNumRepInfo, nSizeOfRepInfo) reps filled in pRepInfo
    if (pRepInfo == NULL || nSizeOfRepInfo < nNumRepInfo)
    {
      status = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
    }
  }

  return status;
}

/** @brief Get segment info for the specified representation.
  *
  * @param[in] nRepKey - Representation key
  * @param[in] nStartTime - Request start time
  * @param[in] nDuration - Request duration
  * @return
  * HTTPDL_WAITING - Request queued and will be notified later
  * HTTPDL_SUCCESS - Segment info available
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPlayGroup::GetSegmentInfo
(
 uint64 nRepKey,
 uint64 nStartTime,
 uint64 nDuration
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  if (nRepKey < (uint64)m_nNumReps && m_pRepresentation && m_pDASHSessionInfo)
  {
    //Create the right rep handler if needed and get segment info
    bool bResult = true;

    if(m_pCommonInitSegment &&
      (InitializationSegment::INITSEG_STATE_IDLE == m_pCommonInitSegment->getInitState()))
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "DASHMediaPlayGroup::GetSegmentInfo start Common Init dwld %p", m_pCommonInitSegment);
      MM_CriticalSection_Enter(m_pGroupDataLock);
      DownloadCommonInit();
      MM_CriticalSection_Leave(m_pGroupDataLock);
    }

    if (GetRepresentationHandler((int)nRepKey) == NULL)
    {
      DASHMediaRepresentationHandler *pRepHandler =
        QTV_New_Args(DASHMediaRepresentationHandler, (bResult,
                                                      (uint32)nRepKey,
                                                      *m_pDASHSessionInfo,
                                                      m_pRepresentation[nRepKey].m_cRepInfo,
                                                      (iGroupNotifier *)this,
                                                      m_pScheduler,
                                                      m_pHTTPStack, m_pCommonInitSegment));
      if (!bResult)
      {
        if (pRepHandler)
        {
          QTV_Delete(pRepHandler);
          pRepHandler = NULL;
        }
      }
      else
      {
        SetRepresentationHandler((int)nRepKey,pRepHandler);
      }
    }

    if (bResult)
    {
      status =  m_pRepresentation[nRepKey].m_pRepHandler->GetSegmentInfo(nStartTime, nDuration);
    }
  }

  return status;
}

/** @brief Read data unit info for the specified representation and time range.
  *
  * @param[in] nRepKey - Representation key
  * @param[in] nStartTime - Request start time
  * @param[in] nDuration - Request duration
  * @param[in] pDataUnitInfo - Reference to data unit info array
  * @param[in] nSizeOfDataUnitInfo - Size of data unit info array
  * @param[out] nNumDataUnitInfo - Number of data units filled/needed
  * @return
  * HTTPDL_SUCCESS - Data unit info read
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPlayGroup::FillDataUnitInfo
(
 uint64 nRepKey,
 uint64 nStartTime,
 uint64 nDuration,
 QSM::CDataUnitInfo* pDataUnitInfo,
 uint32 nSizeOfDataUnitInfo,
 uint32& nNumDataUnitInfo
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler((int)nRepKey);

  //Read data unit info from the right representation
  if (pRepHandler)
  {
    status = pRepHandler->FillDataUnitInfo(nStartTime,
                                           nDuration,
                                           pDataUnitInfo,
                                           nSizeOfDataUnitInfo,
                                           nNumDataUnitInfo);
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
HTTPDownloadStatus DASHMediaPlayGroup::GetDataUnitDownloadInfo
(
 QSM::CDataUnitDownloadInfo *pDownloadInfo,
 uint32 nSize,
 uint32 &nFilled,
 uint64 nStartTime
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  uint32 previousFilled = 0;
  /* Iterate for number of Representation present in this Group to gather info */
  for(int repIterate = 0; repIterate < m_nNumReps && (status != HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER); repIterate++)
  {
    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(repIterate);
    if(m_pRepresentation && pRepHandler)
    {
      previousFilled = nFilled;
      if(status == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        status = pRepHandler->GetDataUnitDownloadInfo(pDownloadInfo,nSize,nFilled,nStartTime);
      }
      else
      {
        (void)pRepHandler->GetDataUnitDownloadInfo(pDownloadInfo,nSize,nFilled,nStartTime);
      }
      for(; previousFilled < nFilled && pDownloadInfo; previousFilled++)
      {
        /* For all newly added data, Representation key will be updated */
        pDownloadInfo[previousFilled].nRepresentationKey = repIterate;
      }
    }
  }
  return status;
}

/** @brief Get segment data for the specified representation and data unit.
  *
  * @param[in] nRepKey - Representation key
  * @param[in] nDataUnitKey - Data unit to be downloaded
  * @return
  * HTTPDL_WAITING - Request queued and will be notified later
  * HTTPDL_SUCCESS - Data unit available
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPlayGroup::GetSegmentData
(
 uint64 nRepKey,
 uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler((int)nRepKey);

  //Get data unit from the right representation
  if (pRepHandler)
  {
      uint64 nStartTime = 0;
      uint64 nDuration = 0;

      //Get the Data unit info
      if(HTTPCommon::HTTPDL_SUCCESS ==
         pRepHandler->GetDataUnitInfoByKey(nDataUnitKey, nStartTime, nDuration))
      {
        //Update the Download stats for all the active Media stores of the group
        for (int i = 0; i < m_nNumMediaComponents; i++)
        {
          m_cMediaReadStore[i].UpdateMediaReadStoreStats(nRepKey,
                                                         nDataUnitKey,
                                                         nStartTime,
                                                         (uint32)nDuration);
        } //for (int i = 0; i < m_nNumMediaComponents; i++)
      }

    status = pRepHandler->GetSegmentData(nDataUnitKey);
    if(HTTPCommon::HTTPDL_ERROR_ABORT == status)
    {
      //Update the Download stats for all the active Media stores of the group
      for (int i = 0; i < m_nNumMediaComponents; i++)
      {
        m_cMediaReadStore[i].UpdateMediaReadStoreStats(nRepKey,
                                                       nDataUnitKey,
                                                       status);
      } //for (int i = 0; i < m_nNumMediaComponents; i++)
    }
  } //if (pRepHandler)

  return status;
}

void DASHMediaPlayGroup::SegDataReady
(
 const uint64 nRepKey,
 const uint64 nDataUnitKey,
 const HTTPDownloadStatus eStatus
)
{
  if (m_pPeriodNotifier)
  {
    m_pPeriodNotifier->SegDataReady(m_nKey, nRepKey, nDataUnitKey, eStatus);
    //Update the download stats for each Media read store
    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      m_cMediaReadStore[i].UpdateMediaReadStoreStats(nRepKey, nDataUnitKey, eStatus);
    }
  }
}

/** @brief Cancel data download for the specified representation and data unit.
  *
  * @param[in] nRepKey - Representation key
  * @param[in] nDataUnitKey - Data unit download to be cancelled
  * @return
  * HTTPDL_WAITING - Request queued and will be notified later
  * HTTPDL_SUCCESS - Data unit cancel success
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPlayGroup::CancelSegmentData
(
 uint64 nRepKey,
 uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler((int)nRepKey);

  //Get data unit from the right representation
  if (pRepHandler)
  {
    status = pRepHandler->CancelSegmentData(nDataUnitKey);

    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      MM_CriticalSection_Enter(m_cMediaReadStore[i].m_dataUnitsLock);

      HTTPDataUnitElement *pDataUnitElement = NULL;
      pDataUnitElement = (HTTPDataUnitElement *)
                          ordered_StreamList_peek_back(&(m_cMediaReadStore[i].m_dataUnitsInUseList));
      while (pDataUnitElement)
      {
        if((pDataUnitElement->nDataUnitKey == nDataUnitKey) &&
           (pDataUnitElement->nRepKey == nRepKey))
        {
          break;
        }

        pDataUnitElement = (HTTPDataUnitElement *)
                          ordered_StreamList_peek_prev(&pDataUnitElement->link);
      }

      if(pDataUnitElement)
      {
        if(DOWNLOAD_PARTIAL == pDataUnitElement->state)
        {
          if (m_cMediaReadStore[i].nPartiallyDownloadedOccupancy >= pDataUnitElement->nDuration)
          {
            m_cMediaReadStore[i].nPartiallyDownloadedOccupancy -= pDataUnitElement->nDuration;
          }
          else
          {
            QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Error in updating partial occupancy %u duration %u",
                           m_cMediaReadStore[i].nPartiallyDownloadedOccupancy, pDataUnitElement->nDuration );
            m_cMediaReadStore[i].nPartiallyDownloadedOccupancy = 0;
          }
        }
        else if(DOWNLOAD_COMPLETE == pDataUnitElement->state)
        {
          if (m_cMediaReadStore[i].nFullyDownloadedOccupancy >= pDataUnitElement->nDuration)
          {
            m_cMediaReadStore[i].nFullyDownloadedOccupancy -= pDataUnitElement->nDuration;
          }
          else
          {
            QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Error in updating full occupancy %u duration %u",
                           m_cMediaReadStore[i].nFullyDownloadedOccupancy, pDataUnitElement->nDuration );
            m_cMediaReadStore[i].nFullyDownloadedOccupancy = 0;
          }
        }

        m_cMediaReadStore[i].nCurrentDownloadPosition = STD_MAX(m_cMediaReadStore[i].nCurrentDownloadPosition,
                                          (pDataUnitElement->nStartTime + pDataUnitElement->nDuration));
        pDataUnitElement->state = DOWNLOAD_ERROR;
      }
      else
      {
        //DataUnitKey not found in inuse dataunit list
      }

      MM_CriticalSection_Leave(m_cMediaReadStore[i].m_dataUnitsLock);
    }
  }

  return status;
}

/**
 * Continue downloading the data unit that was indicated as too
 * slow to QSM.
 */
HTTPDownloadStatus DASHMediaPlayGroup::ContinueDownloadDataUnit(
  uint64 nRepresentationKey, uint64 nKey)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler((int)nRepresentationKey);

  if (pRepHandler)
  {
    status = pRepHandler->ContinueDownloadDataUnit(nKey);
  }

  return status;
}

/**
 * Get's the fragment start-time for the key.
 */
bool DASHMediaPlayGroup::GetDataUnitStartTime
(
 uint64 nRepKey,
 uint64 nDataUnitKey,
 uint64& nPbTime
)
{
  bool rslt = false;
  nPbTime = 0;

  DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler((int)nRepKey);
  if (pRepHandler)
  {
    rslt = (HTTPCommon::HTTPDL_SUCCESS == pRepHandler->Select(nDataUnitKey, nPbTime)
            ? true : false);
  }

  return rslt;
}

/** @brief Select the specified representation and data unit.
  *
  * @param[in] nRepKey - Representation to be selected
  * @param[in] nDataUnitKey - Data unit to be selected
  * @return
  * HTTPDL_WAITING - Request queued and will be notified later
  * HTTPDL_SUCCESS - Selection complete
  * HTTPDL_ERROR_ABORT - Generic failure
  */
HTTPDownloadStatus DASHMediaPlayGroup::Select
(
 uint64 nRepKey,
 uint64 nDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler((int)nRepKey);

  if (pRepHandler)
  {
    uint64 nPbTime = 0;
    //If there is a seek issued on playgroup i.e m_nSeekTime > 0 make bForceSeek to true
    //This ensures that seek will be issued on segment handlers even though segment start
    //time is same as seek time.
    bool bForceSeek = false;
    if (m_nSeekTime >= 0)
    {
      nPbTime = m_nSeekTime;
      bForceSeek = true;
      m_nSeekTime = -1;
      status = HTTPCommon::HTTPDL_SUCCESS;
    }
    else
    {
      status = pRepHandler->Select(nDataUnitKey, nPbTime);
    }

    if (status == HTTPCommon::HTTPDL_SUCCESS || status == HTTPCommon::HTTPDL_WAITING)
    {
      // nPbTime is the start time of the first fragment of the selected rep.

      if (INVALID_START_TIME == m_nStartTime)
      {
        m_nStartTime = nPbTime;
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "Updated m_nStartTime of playgrp with key %llu to %llu",
                      GetKey(), m_nStartTime);
      }

      //For interleaved AV case, make copies of switch point for both A and V,
      //so that each media component can switch on its own!
      for (int i = 0; i < m_nNumMediaComponents; i++)
      {
        //Validate switch point against the playback position and ignore switch
        //if already past (this logic might change if sample endTime is not given)
        uint64 nMediaPbTime = m_cMediaReadStore[i].GetPlaybackPosition();
        if (nPbTime < nMediaPbTime)
        {
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
          QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Playback %u msec past switch point %u msec, ignoring switch %u/%u",
                         (uint32)nMediaPbTime, (uint32)nPbTime, (uint32)nRepKey, (uint32)nDataUnitKey );
          break;
        }
        //Clean up switch Q to get rid of overlapping switches and enqueue
        //the new switch point
        SwitchElement sElem = {0,0,0};
        SwitchElement sHeadElem = {0,0,0};
        void* pIt = NULL;
        m_cMediaReadStore[i].m_cSwitchQ.PeekHead(sHeadElem);
        while(m_cMediaReadStore[i].m_cSwitchQ.Next(pIt, sElem))
        {
          if (sElem.nSwitchTime >= nPbTime)
          {
            //We should not issue a close on current playing representation
            //if switch sequence before was from R1->R2->R1 and there is
            //an overlapping switch point after R2 for R5 then switch
            //sequence becomes R1->R2->R5 but we can not issue a close on R1
            //we are currently playing from it!!!

            //The other case is if queue contains only R1 and playback has not
            //started and there is a overlapping switch to R2.In this case we
            //should issue a close on R1. So check if head of switch queue is
            //same as current playing rep then issue a close!!!
            if(sElem.nRepKey != m_cMediaReadStore[i].GetCurrentRepresentation() ||
              (sElem.nRepKey == sHeadElem.nRepKey && sElem.nDataUnitKey ==sHeadElem.nDataUnitKey
              && sElem.nSwitchTime == sHeadElem.nSwitchTime))
            {
              DASHMediaRepresentationHandler* pSwitchRepHandler = GetRepresentationHandler((int)sElem.nRepKey);
              if(pSwitchRepHandler)
              {
                (void)pSwitchRepHandler->Close(m_cMediaReadStore[i].GetMediaType());
                //If seek is pending and we are closing the head of switch queue
                //i.e the rep on which seek is in progress, seek needs to be called
                //on the new rep
                if(IsSeekPending() && (sElem.nSwitchTime == sHeadElem.nSwitchTime))
                {
                  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                "Seek is pending so setting seek flag to true");
                  bForceSeek = true;
                }
              }
            }
            QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "Overlapping switch point, removing duplicate %u/%u/%u (rep/new/old)",
                           (uint32)sElem.nRepKey, (uint32)nPbTime, (uint32)sElem.nSwitchTime );
           (void)m_cMediaReadStore[i].m_cSwitchQ.Remove(pIt);
          }
        }

        //Validate switch point against the playback position and ignore switch
        //if already past (this logic might change if sample endTime is not given)
        sElem.nRepKey = (int)nRepKey;
        sElem.nSwitchTime = nPbTime;
        sElem.nDataUnitKey = nDataUnitKey;

        if (!m_cMediaReadStore[i].m_cSwitchQ.EnQ(sElem))
        {
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Could not queue switch point, ignoring switch %u/%u",
                         (uint32)nRepKey, (uint32)nDataUnitKey );
          break;
        }
        else
        {
          QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Queued switch point %u/%u/%u/%u/%u",
                         (uint32)m_nKey, (uint32)nRepKey, (uint32)(nDataUnitKey>>32), (uint32)nDataUnitKey,(uint32)nPbTime );
          if(m_cMediaReadStore[i].m_cSwitchQ.Count() < 2)
          {
            pRepHandler->Open(nDataUnitKey,nPbTime,bForceSeek);
          }
        }
      }

      if (status == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        //ToDo: Restore switch points on all media components if more than 1
        //(e.g. for AV case if either A or V fails, highly unlikely!)
      }
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Rep %u could not switch %d, ignoring switch", (uint32)nRepKey, status );
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }

  return status;
}

/** @brief Check if group is readable.
  *
  * @return
  * TRUE - Readable
  * FALSE - Otherwise
  */
bool DASHMediaPlayGroup::IsReadable(bool bCheckDataReadableOnly)
{
  bool bOk = false;

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

  //Check if current representation is readable on all media components
  if (m_nNumMediaComponents > 0)
  {
    bOk = true;
    for (int i = 0; i < m_nNumMediaComponents && bOk; i++)
    {
      bOk = false;
      int nCurrKey = m_cMediaReadStore[i].GetCurrentRepresentation();
      DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nCurrKey);
      if (pRepHandler)
      {
        HTTPDownloadStatus eStatus = pRepHandler->IsReadable();

        if (!bCheckDataReadableOnly)
        {
          if (eStatus == HTTPCommon::HTTPDL_SUCCESS || eStatus == HTTPCommon::HTTPDL_DATA_END)
          {
            bOk = true;
          }
        }
        else
        {
          if (eStatus == HTTPCommon::HTTPDL_SUCCESS)
          {
            bOk = true;
          }
        }
      }
    }
  }

  return bOk;
}

InitializationSegment::InitSegState DASHMediaPlayGroup::CommonInitDwldState()
{
  return (m_pCommonInitSegment ? m_pCommonInitSegment->getInitState() :
                                 InitializationSegment::INITSEG_STATE_ERROR);
}

void DASHMediaPlayGroup::DownloadCommonInit()
{
  if(m_pCommonInitSegment)
  {
    InitializationSegment::InitSegState m_eState = m_pCommonInitSegment->getInitState();
    switch(m_eState)
    {
    case InitializationSegment::INITSEG_STATE_IDLE :
    {
      if (m_pCommonInitSegment->m_cDownloader.Start(m_pCommonInitSegment->m_pURL, NULL,
                                                    0,
                                                    m_pCommonInitSegment->m_nStartOffset,
                                                    m_pCommonInitSegment->m_nEndOffset))
      {
        m_pCommonInitSegment->m_cDataStore.SetStartOffset(0);
        m_pCommonInitSegment->m_cDataStore.SetPurgeFlag(false);
        m_pCommonInitSegment->setInitState(InitializationSegment::INITSEG_STATE_DOWNLOADING);
      }
      else
      {
        m_pCommonInitSegment->setInitState(InitializationSegment::INITSEG_STATE_ERROR);
        //(void)m_pRepHandler->OnError(HTTPCommon::HTTPDL_ERROR_ABORT);
      }

      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "DASHMediaPlayGroup::DownloadCommonInit init segment state changed from %d to %d",
                    m_eState, m_pCommonInitSegment->getInitState());
    }
    break;

    case InitializationSegment::INITSEG_STATE_DOWNLOADING :
    {
      InitializationSegment::InitSegState m_eNewState = InitializationSegment::INITSEG_STATE_DOWNLOADING;
      HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
      status = Download();
      if(HTTPCommon::HTTPDL_SUCCESS == status)
      {
        m_eNewState = InitializationSegment::INITSEG_STATE_DOWNLOADED;
      }
      else if(HTTPCommon::HTTPDL_ERROR_ABORT == status      ||
              HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND == status )
      {
        m_eNewState = InitializationSegment::INITSEG_STATE_ERROR;
        m_pCommonInitSegment->Reset();
      }

      if(m_eState != m_eNewState)
      {
        m_pCommonInitSegment->setInitState(m_eNewState);
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "DASHMediaPlayGroup::DownloadCommonInit init segment state changed from %d to %d",
                     m_eState, m_pCommonInitSegment->getInitState());
      }
    }
    break;

    case InitializationSegment::INITSEG_STATE_DOWNLOADED :

    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "DASHMediaPlayGroup::DownloadCommonInit init segment state is already INITSEG_STATE_DOWNLOADED. no op");

    break;

    case InitializationSegment::INITSEG_STATE_ERROR :

    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "DASHMediaPlayGroup::DownloadCommonInit cmn init segment is in INITSEG_STATE_ERROR. check if rep has its own init seg");

    break;

    default:
    break;
    }
  }
}

/** @brief Process all commands (in SETUP state).
  *
  * @return
  * 0 - Success
  * -1 - Failure
  */
HTTPDownloadStatus DASHMediaPlayGroup::Download()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;

  if (m_pCommonInitSegment)
  {
    SegmentDownloader& cDownloader = m_pCommonInitSegment->m_cDownloader;
    HttpSegmentDataStoreBase* pDataStore = &m_pCommonInitSegment->m_cDataStore;
    byte* pBuffer = NULL;
    int64 nBytesToRead = HTTP_MAX_DATA_CHUNK_LEN;

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
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "DASHMediaPlayGroup::Download() : Common Init segment download progress %llu/%lld bytes",
                         cDownloader.GetNumBytesReceived(),
                         cDownloader.GetContentLength());

          /* Update size of data downloaded */
          m_bEstimator->UpdateSize(nBytesRead);
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
              "DASHMediaPlayGroup::Download() sanity check against content-len %lld and num of downloaded bytes %llu failed",
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

            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "DASHMediaPlayGroup::Download(): Init segment download complete");
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
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "DASHMediaPlayGroup::Download(): Init segment waiting for data");
      }
      else if (result == HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND)
      {
          status = HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND;
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "DASHMediaPlayGroup::Download(): error detected due to missing initialization segment");
      }
      else
      {
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }
    }
  }

  //Clean up for failure!
  if(HTTPCommon::HTTPDL_ERROR_ABORT  == status ||
     HTTPCommon::HTTPDL_SEGMENT_NOT_FOUND == status)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "DASHMediaPlayGroup::Download(): Init segment download failed, closing");
  }

  return status;
}

/** @brief Get the base time for the play group.
  * @param[out] basetime - basetime for play group
  * @return
  */
bool DASHMediaPlayGroup::GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime)
{
  bool bRet = false;
  //If current representation is readable query for base time
  uint64 tmpBaseTime = MAX_UINT64;
  uint64 tmpMPDBaseTime = MAX_UINT64;
  if (m_nNumMediaComponents > 0)
  {
    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      int nCurrKey = m_cMediaReadStore[i].GetCurrentRepresentation();
      DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nCurrKey);
      if (pRepHandler)
      {
        HTTPDownloadStatus eStatus = pRepHandler->IsReadable();
        if (eStatus == HTTPCommon::HTTPDL_SUCCESS || eStatus == HTTPCommon::HTTPDL_DATA_END)
        {
          //Representation should be same on all media components. So there is no need to take a min here
          //as both values will be same. Having this from code completeness perspective.
          bRet = pRepHandler->GetBaseTime(segMediaTime, segMPDTime);

          if(MAX_UINT64 == tmpBaseTime /*first rep*/ || tmpBaseTime > segMediaTime /*min across reps*/)
          {
            tmpBaseTime = segMediaTime;
            tmpMPDBaseTime = segMPDTime;
          }
        }
      }
    }
  }
  if(tmpBaseTime != MAX_UINT64)
  {
    segMediaTime = tmpBaseTime;
    segMPDTime = tmpMPDBaseTime;
  }
  else
  {
    bRet = false;
  }

  return bRet;
}

/** @brief Close playgroup.
  *
  * @return status of operation
  */
HTTPDownloadStatus DASHMediaPlayGroup::Close()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  SetDownloadComplete(false);

  //Reset all read stores
  for (int i = 0; i < m_nNumMediaComponents; i++)
  {
    m_cMediaReadStore[i].Reset();
  }

  //Reset Init segment
  if(m_pCommonInitSegment)
  {
    m_pCommonInitSegment->Reset();
  }

  //Close all representations
  if (m_pRepresentation)
  {
    for (int i = 0; i < m_nNumReps; i++)
    {
      DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(i);
      if (pRepHandler)
      {
        (void)pRepHandler->Close();
      }
    }
  }

  return status;
}

/*
 * redirect to the right representation handler
 *
 * @param[out] pTrackInfo Pointer to HTTPMediaTrackInfo,
 *
 * @return  number of valid tracks.
 */
uint32 DASHMediaPlayGroup::GetMediaTrackInfo
(
  HTTPMediaTrackInfo *pTrackInfo
)
{
  uint32 nNumTracks = 0;

  if (m_nNumMediaComponents > 0)
  {
    int nCurrKey = -1;
    DASHMediaRepresentationHandler* pRepHandler = NULL;

    //Accumulate tracks over all media components
    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      nCurrKey = m_cMediaReadStore[i].GetCurrentRepresentation();
      pRepHandler = GetRepresentationHandler(nCurrKey);
      if (pRepHandler)
      {
        nNumTracks += pRepHandler->GetMediaTrackInfo(pTrackInfo + nNumTracks);
      }
    }
  }

  return nNumTracks;
}

/** @brief Check if playback can continue uninterrupted on the
  *        current playing representation.
  *
  * @return
  * true - Can continue uninterrupted
  * false - Cannot continue uninterrupted
  */
bool DASHMediaPlayGroup::CanPlaybackUninterrupted()
{
  int nCurrKey = -1;
  DASHMediaRepresentationHandler* pRepHandler = NULL;
  bool bOk = (m_nNumMediaComponents > 0);

  //Check if playback can continue uninterrupted on the current playing
  //representation in all media components (reps could be different on
  //different media components only if they don't switch together)
  for (int i = 0; i < m_nNumMediaComponents && bOk; i++)
  {
    nCurrKey = m_cMediaReadStore[i].GetCurrentRepresentation();
    pRepHandler = GetRepresentationHandler(nCurrKey);
    if (pRepHandler)
    {
      bOk = pRepHandler->CanPlaybackUninterrupted();
    }
    else
    {
      bOk = false;
      break;
    }
  }

  return bOk;
}

/*
 * redirect to the right representation handler
 *
 * @param[in] majorType  media major type for which track information
 * needs to be retrieved
 * @param[out] TrackInfo populated the track information on success
 *
 * @return
 * HTTPDL_SUCCESS - successfully retrieved track info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHMediaPlayGroup::GetSelectedMediaTrackInfo
(
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaTrackInfo &TrackInfo
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_UNSUPPORTED;
  MediaReadStore* pReadStore = GetMediaReadStore(majorType);

  if (pReadStore)
  {
    int nRepKey = pReadStore->GetCurrentRepresentation();
    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nRepKey);
    if (pRepHandler)
    {
      eStatus = pRepHandler->GetSelectedMediaTrackInfo(majorType, TrackInfo);
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid (%d) rep selection in playgroup %u",
                     nRepKey, m_nMajorType );
    }
  }

  return eStatus;
}

/*
 * redirect to the right representation handler
 *
 * @param[in] nTrackID   Identifies the track for which codec data needs to
 * be retrieved
 * @param[in] majorType  media major type for which track information
 * needs to be retrieved
 * @param[in] minorType   media minor type for which the codec info is being
 * requested
 * @param[out] AACCodecData populates the codec data on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved codec info
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus DASHMediaPlayGroup::GetCodecData
(
  uint32 nTrackID,
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaMinorType minorType,
  HTTPCodecData &CodecData
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  MediaReadStore* pReadStore = GetMediaReadStore(majorType);

  if (pReadStore)
  {
    int nRepKey = pReadStore->GetCurrentRepresentation();
    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nRepKey);
    if (pRepHandler)
    {
      eStatus = pRepHandler->GetCodecData(nTrackID, minorType, CodecData);
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid (%d) rep selection in playgroup %u",
                     nRepKey, m_nMajorType );
    }
  }

  return eStatus;
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

 HTTPDownloadStatus DASHMediaPlayGroup::GetCurrentKeys
 (
   HTTPCommon::HTTPMediaType majorType,
   int &nCurrGrpInfoKey,
   int &nRepInfoKey
 )
 {
   HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
   MediaReadStore* pReadStore = GetMediaReadStore(majorType);
   nCurrGrpInfoKey =  (int)((m_cRepGroupInfo.getKey() & MPD_REPGRP_MASK) >> MPD_REPGRP_SHIFT_COUNT);
   if (pReadStore)
   {
     DASHMediaRepresentationHandler* pRepHandler =
        GetRepresentationHandler(pReadStore->GetCurrentRepresentation());
     if (pRepHandler)
     {
       nRepInfoKey = pRepHandler->GetRepInfoKey();
     }
   }
   return eStatus;
 }

/*
 * redirect to the right representation handler
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
HTTPDownloadStatus DASHMediaPlayGroup::GetFormatBlock
(
  HTTPCommon::HTTPMediaType majorType,
  uint8* pBuffer,
  uint32 &nbufSize
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  MediaReadStore* pReadStore = GetMediaReadStore(majorType);

  if (pReadStore)
  {
    int nRepKey = pReadStore->GetCurrentRepresentation();
    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nRepKey);
    if (pRepHandler)
    {
      eStatus = pRepHandler->GetFormatBlock(majorType,
                                            pBuffer,
                                            nbufSize);
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid (%d) rep selection in playgroup %u",
                     nRepKey, m_nMajorType );
    }
  }

  return eStatus;
}

/*
 * redirect to the right representation handler
 *
 * @param[in] HTTPMediaType media type.
 * @param[out]pBuffer  A pointer to the buffer into which to place the sample.
 * @param[out] nSize The size of the data buffer.
 * @param[out] sampleInfo Provides information about the sample
 *
 * @return HTTPDL_ERROR_SUCCESS if successful in retrieving the format block
 * else appropraite error code
 */
HTTPCommon::HTTPDownloadStatus DASHMediaPlayGroup::GetNextMediaSample
(
  HTTPCommon::HTTPMediaType majorType,
  uint8 *pBuffer,
  uint32 &nSize,
  HTTPSampleInfo &sampleInfo
)
{
  HTTPCommon::HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  MediaReadStore* pReadStore = GetMediaReadStore(majorType);

  if (pReadStore)
  {
    int nRepKey = pReadStore->GetCurrentRepresentation();
    MediaReadState eReadState = pReadStore->GetReadState();
    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nRepKey);
    SwitchElement sElem;
    if (pReadStore->m_cSwitchQ.PeekHead(sElem))
    {
      if (sElem.nRepKey == nRepKey)
      {
        //This is the first time GetNextMediaSample is called
        //after seek/start
        pReadStore->SetCurrentRepresentation(nRepKey);
        pReadStore->m_cSwitchQ.DeQ(sElem);
      }
    }
    //Open the next representation in switch queue. Current representation should already be opened
    if(pReadStore->m_cSwitchQ.PeekHead(sElem))
    {
      DASHMediaRepresentationHandler* pNextRepHandler = GetRepresentationHandler(sElem.nRepKey);
      if(pNextRepHandler)
      {
        if(pNextRepHandler->Open(sElem.nDataUnitKey,sElem.nSwitchTime) == HTTPCommon::HTTPDL_ERROR_ABORT)
        {
          QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                         "Open on representation %d dataunitkey %lld switchtime %lld failed",
                         sElem.nRepKey,sElem.nDataUnitKey,sElem.nSwitchTime);
        }
      }//if(pNextRepHandler)
    }// if(m_cMediaReadStore[i].m_cSwitchQ.PeekHead(sElem))

    if (pRepHandler)
    {
      bool bRead = false;

      if (eReadState == INIT)
      {
        //Send codec info in the first buffer for video/Text
        if(HTTPCommon::HTTP_VIDEO_TYPE == majorType ||
           HTTPCommon::HTTP_TEXT_TYPE == majorType)
        {
          uint32 nLength = 0;

          //Get the format block (size first)
          eStatus = GetFormatBlock(majorType, NULL, nLength);
          if (eStatus == HTTPCommon::HTTPDL_SUCCESS)
          {
            if (nLength <= nSize)
            {
              eStatus = GetFormatBlock(majorType, pBuffer, nSize);
              if (eStatus == HTTPCommon::HTTPDL_SUCCESS)
              {
                QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                               "Sending format block for majorType %d rep %d",
                               majorType, nRepKey );
                eStatus = HTTPCommon::HTTPDL_CODEC_INFO;
                eReadState = READING;
              }
            }
            else
            {
              QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Insufficient buffer to fill format block for majorType %d rep %d "
                             "- %u/%u (available/required)", majorType, nRepKey, nSize, nLength );
              eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
            }
          }
        }
        else
        {
          //Move to READING directly for non-video and read sample in this
          //iteration itself
          eReadState = READING;
          bRead = true;
        }
      }
      else if (eReadState == READING)
      {
        bRead = true;
      }

      if (bRead)
      {
        bool bTrySwitch = false;
        eStatus = pRepHandler->GetNextMediaSample(majorType, pBuffer, nSize, sampleInfo);
        if (eStatus == HTTPCommon::HTTPDL_SUCCESS)
        {
          //Check if we need to switch to a new representation
          SwitchElement sElem;
          if (pReadStore->m_cSwitchQ.PeekHead(sElem) &&
              (sElem.nSwitchTime <= sampleInfo.startTime))
          {
            //Head of the switch Q must be the earliest switch point, so check if sample
            //pulled has start time greater than switch point (e.g. R1*->R2->R1). If yes
            //attempt a switch
            bTrySwitch = true;
          }
          else
          {
            //Set playback position (using endTime is accurate) and update stats
            //If sampleInfo.endTime exceeds the current period duration, cap the sampleInfo.endTime
            double currPeriodDuration = m_pDASHSessionInfo->cMPDParser.GetDuration(m_pRepresentation[nRepKey].m_cRepInfo.getKey());
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "Current Period Duration: %f Sample end time: %llu", currPeriodDuration,sampleInfo.endTime );

            if((sampleInfo.endTime > currPeriodDuration) && (currPeriodDuration >  0.0))
            {
              sampleInfo.endTime = (uint64)currPeriodDuration;
            }

            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "Updated Sample end time: %llu", sampleInfo.endTime );
            pReadStore->SetPlaybackPosition(sampleInfo.endTime);
            pReadStore->UpdateMediaReadStoreStats(sampleInfo.endTime);
          }
        }
        else if (eStatus == HTTPCommon::HTTPDL_NO_MORE_RESOURCES ||
                 eStatus == HTTPCommon::HTTPDL_SEGMENT_BOUNDARY ||
                 eStatus == HTTPCommon::HTTPDL_DATA_END)
        {
          // If there is underrun at segment fragment boundary check if
          // there is an element in switch queue
          double currPeriodDuration = m_pDASHSessionInfo->cMPDParser.GetDuration(
             m_pRepresentation[nRepKey].m_cRepInfo.getKey());

          if (eStatus != HTTPCommon::HTTPDL_DATA_END &&
              currPeriodDuration > 0 &&
              pReadStore->GetPlaybackPosition() >= currPeriodDuration)
          {
            eStatus = HTTPCommon::HTTPDL_DATA_END;
          }
          else
          {
            bTrySwitch = true;
            QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "DASHMediaPlayGroup::GetNextMediaSample Status: %d", eStatus);
            if (eStatus != HTTPCommon::HTTPDL_DATA_END)
            {
              eStatus = HTTPCommon::HTTPDL_WAITING;
            }
          }

          if((currPeriodDuration > 0) && (HTTPCommon::HTTPDL_DATA_END == eStatus))
          {
            pReadStore->SetPlaybackPosition((uint64)currPeriodDuration);
            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "DASHMediaPlayGroup::GetNextMediaSample  SetPlaybackPostion to %f for mediaType %d", currPeriodDuration, majorType);
          }
        }

        //Try switching (if appropriate)
        if (bTrySwitch)
        {
          if (pReadStore->m_cSwitchQ.Count() > 0)
          {
            bool bLoop = true;
            SwitchElement sElem;
            int nNextRepKey = nRepKey;

            //Loop through the switch Q to find a readable rep cleaning up on the way
            //(if rep has no more readable resources). Stick to the current rep if
            //next rep is also not ready to be read
            while (bLoop)
            {
              bLoop = false;
              if (pReadStore->m_cSwitchQ.PeekHead(sElem))
              {
                DASHMediaRepresentationHandler* pNextRepHandler =
                  GetRepresentationHandler(sElem.nRepKey);
                if (pNextRepHandler)
                {
                  eStatus = pNextRepHandler->IsReadable(majorType);
                  if (eStatus != HTTPCommon::HTTPDL_WAITING)
                  {
                    //Clean up switch Q and try to find the rep to switch to
                    if (eStatus == HTTPCommon::HTTPDL_NO_MORE_RESOURCES ||
                        eStatus == HTTPCommon::HTTPDL_DATA_END)
                    {
                      //Preserve the last switch point (needed for buff occupancy
                      //calculations and this is where QSM is so keep it)
                      if (pReadStore->m_cSwitchQ.Count() > 1)
                      {
                        pNextRepHandler->Close(majorType);
                        pReadStore->m_cSwitchQ.DeQ(sElem);
                        bLoop = true;
                      }
                    }
                    else
                    {
                      //This is ideally a switch success case, so dequeue switch
                      //and mark it as next rep
                      nNextRepKey = sElem.nRepKey;
                      pReadStore->m_cSwitchQ.DeQ(sElem);
                    }
                  }// if (eStatus != HTTPCommon::HTTPDL_WAITING)
                }// if (pNextRepHandler)
              }// if (pReadStore->m_cSwitchQ.PeekHead(sElem))
            }// while (bLoop)

            if (eStatus != HTTPCommon::HTTPDL_DATA_END)
            {
              //Close current rep and initiate switch sequence only if next rep is different
              //from the current one (else it could be a soft switch where we are switching
              //to the same rep after possibly cleaning up one or more reps, in which case
              //just continue reads!)
              if (nNextRepKey != nRepKey)
              {
                QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                               "GetNextMediaSample - majorType %d switch to rep %d from rep %d",
                               majorType, nNextRepKey, nRepKey );

                pRepHandler->Close(majorType);
                nRepKey = nNextRepKey;
                eStatus = video::HTTPCommon::HTTPDL_SWITCH;
                eReadState = INIT;
              }
              else
              {
                QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                               "GetNextMediaSample - majorType %d no need to switch from rep %d",
                               majorType, nRepKey );
                eStatus = HTTPCommon::HTTPDL_WAITING;
              }
            }// if (eStatus != HTTPCommon::HTTPDL_DATA_END)
          }// if (pReadStore->m_cSwitchQ.Count() > 0)
        }// if (bTrySwitch)
        else
        {
          QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                         "GetNextMediaSample - majorType %d rep %d eStatus %d playback %u msec",
                         majorType, nRepKey, eStatus, (uint32)pReadStore->GetPlaybackPosition() );
        }
      }

      //Update the read store
      pReadStore->SetCurrentRepresentation(nRepKey);
      pReadStore->SetReadState(eReadState);
    }

    //Flush all other reps based on the read time
    //ToDo: Also delete the FileSource alone on current rep to save on memory
    //provided it doesn't feature in switch list!
    if (eStatus == HTTPCommon::HTTPDL_SUCCESS)
    {
      for (int i = 0; i < m_nNumReps; i++)
      {
        DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(i);
        if (i != nRepKey && pRepHandler)
        {
          (void)pRepHandler->Flush(majorType,sampleInfo.startTime);
        }
      }
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                   "Invalid media type %d in playgroup %u",
                   majorType, m_nMajorType );
  }

  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "GetNextSample on PG key %llu status %d, major %d",
                GetKey(), eStatus, majorType);

  return eStatus;
}

void DASHMediaPlayGroup::NotifySeekStatus
(
  uint64 /* nRepKey */,
  int64 nSeekEndTime,
  int64 nCurTime,
  HTTPDownloadStatus eStatus
)
{
  m_nSeekEndTime = nSeekEndTime ;
  m_nSeekCurTime = nCurTime;
  m_eSeekStatus = eStatus ;

  for ( int i = 0; i< m_nNumReps ;i++)
  {
    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(i);
    if(pRepHandler)
    {
      pRepHandler->Flush(HTTPCommon::HTTP_UNKNOWN_TYPE,nCurTime);
    }
  }

  for (int i = 0; i < m_nNumMediaComponents; i++)
  {
    m_cMediaReadStore[i].SetPlaybackPosition(nCurTime);
    m_cMediaReadStore[i].UpdateMediaReadStoreStats(nCurTime);
  }

  if(eStatus == HTTPCommon::HTTPDL_SUCCESS || eStatus == HTTPCommon::HTTPDL_ERROR_ABORT ||
     eStatus == HTTPCommon::HTTPDL_DATA_END)
  {
    if(m_pPeriodNotifier)
    {
      m_pPeriodNotifier->NotifySeekStatus(m_nKey,nSeekEndTime,eStatus);
    }
  }
  return;
}

void DASHMediaPlayGroup::NotifyDownloadTooSlow(const uint64 nRepKey,
                                               const uint64 nDataUnitKey)
{
  if(m_pPeriodNotifier)
  {
    m_pPeriodNotifier->NotifyDownloadTooSlow(m_nKey, (uint32)nRepKey, nDataUnitKey);
  }
}

/**
 * Get the buffered duration for the video group from the
 * iPlayGroup notifier interface. Used by segment downloader to
 * determine if a data unit is being downloaded too slowly.
 */
int DASHMediaPlayGroup::GetBufferedDurationFromNotifier()
{
  int duration = -1;
  int nAudioDuration = -1, nVideoDuration = -1, nTextDuration = -1;
  uint64 nPlaybackPosition = 0;
  uint64 nBufferedDuration = 0;
  int numMediaTypes = 0;

  // check if video is one of the readStores.
  for (int i = 0; i < HTTP_MAX_MEDIA_COMPONENTS; ++i)
  {
    HTTPCommon::HTTPMediaType mediaType = m_cMediaReadStore[i].GetMediaType();
    nBufferedDuration = 0;
    GetDurationBuffered(mediaType, nPlaybackPosition, nBufferedDuration);

    if (HTTPCommon::HTTP_AUDIO_TYPE == mediaType)
    {
      if (-1 == nAudioDuration)
      {
        nAudioDuration = (int)nBufferedDuration;
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                      "GetBufferedDuration: audio buffered duration %d, nPlaybackPosition %d",
                      (int)nAudioDuration, (int)nPlaybackPosition);
        ++numMediaTypes;
      }
    }
    else if (HTTPCommon::HTTP_VIDEO_TYPE == mediaType)
    {
      if (-1 == nVideoDuration)
      {
        nVideoDuration = (int)nBufferedDuration;
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "GetBufferedDuration: Video buffered duration %d, nPlaybackPosition %d",
                      (int)nVideoDuration, (int)nPlaybackPosition);
        ++numMediaTypes;
      }
    }
    else if (HTTPCommon::HTTP_TEXT_TYPE == mediaType)
    {
      if (-1 == nTextDuration)
      {
        nTextDuration = (int)nBufferedDuration;
        ++numMediaTypes;
      }
    }
  }

  if (1 == numMediaTypes)
  {
    // non-interleaved.
    duration = QTV_MAX(nAudioDuration, nVideoDuration);
    duration = QTV_MAX(duration, nTextDuration);
  }
  else
  {
    // interleaved case.
    // Rationale here:
    // If there is interleaved audio, then player will go into
    // buffering when audio goes into underun. If there is no
    // interleaved audio, then just pick video as the next one to
    // base a decision off of.
    if (nAudioDuration >= 0)
    {
      duration = nAudioDuration;
    }
    else if (nVideoDuration >= 0)
    {
      duration = nVideoDuration;
    }
    else if (nTextDuration >= 0)
    {
      duration = nTextDuration;
    }
  }

  return duration;
}

HTTPDownloadStatus DASHMediaPlayGroup::Seek(const int64 nSeekTime)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  //Set the current playback time in read store as given seek time.
  for (int i = 0; i < m_nNumMediaComponents; i++)
  {
    m_cMediaReadStore[i].SetPlaybackPosition(nSeekTime);
    m_cMediaReadStore[i].UpdateMediaReadStoreStats(nSeekTime);
  }

  //Reset download failure status on seek
  m_IslastDataDownloadFailed = false;

  bool isRepPresent = false;
  for (int i = 0; i < m_nNumMediaComponents; i++)
  {
    int nRepKey = m_cMediaReadStore[i].GetCurrentRepresentation();
    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nRepKey);
    if(pRepHandler)
    {
      pRepHandler->Close(m_cMediaReadStore[i].GetMediaType());
      pRepHandler->Flush(m_cMediaReadStore[i].GetMediaType(),nSeekTime);
      status = pRepHandler->Seek(nSeekTime);
      isRepPresent = true;
    }
  }
  if(!isRepPresent)
  {
    //Currently no representation in read store. Store the seek time, seek
    //will be processed as part of select.
    m_nSeekTime = nSeekTime;
  }
  SetSeekPending(true);
  return status;
}

/*
 * @brief - Sets the seek time and flushes all the data from representations
 *          All the switch points are removed from the queue. Seek will be
 *          processed as part of select
 *
 * @param[in] nSeekTime - position to be seeked
 *
 * @return HTTPDL_ERROR_SUCCESS if successful in
 */
HTTPDownloadStatus DASHMediaPlayGroup::Flush(const int64 /* nTime */)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  SetDownloadComplete(false);

  //Flush all the representations
  for (int i = 0; i < m_nNumReps; i++)
  {
    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(i);
    if (pRepHandler)
    {
      //TODO: Change this to flush only the data before given time.
      //Currently the input nTime is ignored and all the data is flushed.
      (void)pRepHandler->Close();
    }
  }

  //Clear Switch queue for all media components and set current representation
  // to -1(i.e invalidate current rep)
  for (int i = 0; i < m_nNumMediaComponents; i++)
  {
    HTTPDataUnitElement *pDataUnitElement = NULL;

    MM_CriticalSection_Enter(m_cMediaReadStore[i].m_dataUnitsLock);

    pDataUnitElement = (HTTPDataUnitElement *)
                        ordered_StreamList_peek_front(&(m_cMediaReadStore[i].m_dataUnitsInUseList));
    while (pDataUnitElement)
    {
      ordered_StreamList_pop_item(&(m_cMediaReadStore[i].m_dataUnitsInUseList), &pDataUnitElement->link);
      ordered_StreamList_push(&(m_cMediaReadStore[i].m_dataUnitsFreeList),
                              &pDataUnitElement->link,
                              pDataUnitElement->nStartTime);

       pDataUnitElement = (HTTPDataUnitElement *)
       ordered_StreamList_peek_front(&(m_cMediaReadStore[i].m_dataUnitsInUseList));
    } //while (pDataUnitElement)

    m_cMediaReadStore[i].nCurrentDownloadPosition      = 0;
    m_cMediaReadStore[i].nFullyDownloadedOccupancy     = 0;
    m_cMediaReadStore[i].nPartiallyDownloadedOccupancy = 0;

    MM_CriticalSection_Leave(m_cMediaReadStore[i].m_dataUnitsLock);

    m_cMediaReadStore[i].Reset();
  }
  m_bInvalidGroup = false;

  return status;
}

void DASHMediaPlayGroup::ClearBufferedData(uint64 nTime)
{
  uint32 nQsmMajorType = m_nMajorType;
  HTTPCommon::HTTPMediaType majorType = video::HTTPCommon::HTTP_UNKNOWN_TYPE;

  if (QSM::MAJOR_TYPE_AUDIO == nQsmMajorType)
  {
    majorType = HTTPCommon::HTTP_AUDIO_TYPE;
  }
  else if (QSM::MAJOR_TYPE_VIDEO == nQsmMajorType)
  {
    majorType = HTTPCommon::HTTP_VIDEO_TYPE;
  }
  else if (QSM::MAJOR_TYPE_TEXT == nQsmMajorType)
  {
    majorType = HTTPCommon::HTTP_TEXT_TYPE;
  }

  if (HTTPCommon::HTTP_UNKNOWN_TYPE != majorType)
  {
    if (m_pRepresentation)
    {
      for (int i = 0; i < m_nNumReps; i++)
      {
        DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(i);
        if (pRepHandler)
        {
          //QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          //  "AdaptationSetChange: ClearBufferedData PG %llu, rep 0x%x%x, from time %llu",
          //  GetKey(), (unsigned int)(pRepHandler->GetKey() >> 32), (unsigned int)pRepHandler->GetKey());
          pRepHandler->ClearBufferedData(majorType, nTime);
        }
      }
    }
  }
}

void DASHMediaPlayGroup::RemoveDataUnitFromStats
(
 uint64 nRepKey,
 uint64 nFirstDataUnitKey,
 uint64 nLastRemovedDataUnitKey
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler((int)nRepKey);

  QTV_MSG_PRIO5(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "RemoveDataUnitFromStats for rep %llu, first dataunit 0x%x 0x%x, last dataunit 0x%x 0x%x",
    nRepKey, (unsigned int)(nFirstDataUnitKey >> 32), (unsigned int)nFirstDataUnitKey,
    (unsigned int)(nLastRemovedDataUnitKey >> 32), (unsigned int)nLastRemovedDataUnitKey);

  if (pRepHandler)
  {
    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      MM_CriticalSection_Enter(m_cMediaReadStore[i].m_dataUnitsLock);

      HTTPDataUnitElement *pDataUnitElementFirst = NULL;
      HTTPDataUnitElement *pDataUnitElementLast = NULL;

      // find the elements associated with the first and last keys.
      {
        HTTPDataUnitElement *pDataUnitElement =
          (HTTPDataUnitElement *)ordered_StreamList_peek_front(&(m_cMediaReadStore[i].m_dataUnitsInUseList));

        while (pDataUnitElement)
        {
          if (pDataUnitElement->nRepKey == nRepKey)
          {
            if (pDataUnitElement->nDataUnitKey == nFirstDataUnitKey)
            {
              pDataUnitElementFirst = pDataUnitElement;
            }
            if (pDataUnitElement->nDataUnitKey == nLastRemovedDataUnitKey)
            {
              pDataUnitElementLast = pDataUnitElement;
              break;
            }
          }

          pDataUnitElement = (HTTPDataUnitElement *)ordered_StreamList_peek_next(&pDataUnitElement->link);
        }
      }

      if (pDataUnitElementLast)
      {
        HTTPDataUnitElement *pDataUnitElement = (HTTPDataUnitElement *)
          ordered_StreamList_peek_back(&(m_cMediaReadStore[i].m_dataUnitsInUseList));

        if (pDataUnitElementLast != pDataUnitElement)
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "RemoveDataUnitFromStats: Sanity check failed");
          pDataUnitElementFirst = NULL;
          pDataUnitElementLast = NULL;
        }
      }

      if (pDataUnitElementFirst)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "RemoveDataUnitFromStats Found first and last element for clearing");

        HTTPDataUnitElement *pDataUnitElement =
          (HTTPDataUnitElement *)ordered_StreamList_peek_back(&(m_cMediaReadStore[i].m_dataUnitsInUseList));

        while (pDataUnitElement)
        {
          if(DOWNLOAD_PARTIAL == pDataUnitElement->state)
          {
            if (m_cMediaReadStore[i].nPartiallyDownloadedOccupancy >= pDataUnitElement->nDuration)
            {
              m_cMediaReadStore[i].nPartiallyDownloadedOccupancy -= pDataUnitElement->nDuration;
            }
            else
            {
              QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Error in updating partial occupancy %u duration %u",
                             m_cMediaReadStore[i].nPartiallyDownloadedOccupancy, pDataUnitElement->nDuration );
              m_cMediaReadStore[i].nPartiallyDownloadedOccupancy = 0;
            }
          }
          else if(DOWNLOAD_COMPLETE == pDataUnitElement->state)
          {
            if (m_cMediaReadStore[i].nFullyDownloadedOccupancy >= pDataUnitElement->nDuration)
            {
              m_cMediaReadStore[i].nFullyDownloadedOccupancy -= pDataUnitElement->nDuration;
            }
            else
            {
              QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Error in updating full occupancy %u duration %u",
                             m_cMediaReadStore[i].nFullyDownloadedOccupancy, pDataUnitElement->nDuration );
              m_cMediaReadStore[i].nFullyDownloadedOccupancy = 0;
            }
          }

          m_cMediaReadStore[i].nCurrentDownloadPosition = pDataUnitElement->nStartTime;

          ordered_StreamList_pop_item(&m_cMediaReadStore[i].m_dataUnitsInUseList,
              &pDataUnitElement->link);

          if(pDataUnitElementFirst ==  pDataUnitElement)
          {
            break;
          }

          pDataUnitElement =
            (HTTPDataUnitElement *)ordered_StreamList_peek_back(&(m_cMediaReadStore[i].m_dataUnitsInUseList));
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "RemoveDataUnitFromStats failed. Did not find notified units in list");
      }

      MM_CriticalSection_Leave(m_cMediaReadStore[i].m_dataUnitsLock);
    }
  }
}

uint32 DASHMediaPlayGroup::GetQsmMajorType() const
{
  return m_nMajorType;
}

void DASHMediaPlayGroup::NotifyDataUnitRemoved(const uint64 nRepKey,
   const uint64 nFirstRemovedDataUnitKey,
   const uint64 nLastRemovedDataUnitKey)
{
  RemoveDataUnitFromStats(nRepKey, nFirstRemovedDataUnitKey, nLastRemovedDataUnitKey);
}

uint64 DASHMediaPlayGroup::GetRepGrpInfoKey()
{
  return m_cRepGroupInfo.getKey();
}

bool DASHMediaPlayGroup::GetSeekStatus()
{
  bool status = false;
  if(IsValid())
  {
    if(m_eSeekStatus == HTTPCommon::HTTPDL_SUCCESS)
    {
      status = true;
    }
  }
  return status;
}
/*
 * Get the current playback position.
 *
 * @param[in] mediaType - HTTP media type
 * @param[out] nPlaybackPosition - playback time in msec on success
 *
 * @return true if successful else failure
 */
bool DASHMediaPlayGroup::GetCurrentPlaybackPosition
(
  HTTPCommon::HTTPMediaType mediaType,
  uint64 &nPlaybackPosition
)
{
  bool bResult = true;
  nPlaybackPosition = 0;
  uint64 nStartTime = GetStartTime();

  if (INVALID_START_TIME == nStartTime)
  {
    bResult = false;
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "GetCurrentPlaybackPosition return FALSE for grpKey %llu, majorType %d",
                  GetKey(), mediaType);
  }
  else
  {
    if(m_bInvalidGroup)
    {
      nPlaybackPosition = nStartTime;
    }
    else
    {
      MediaReadStore* pReadStore = GetMediaReadStore(mediaType);
      nPlaybackPosition = nStartTime;
      if (pReadStore)
      {
        // At least one sample was read from here.
        nPlaybackPosition = STD_MAX(nPlaybackPosition, pReadStore->GetPlaybackPosition());
      }
      else
      {
        bResult = false;
      }
    }
  }

  return bResult;
}

/*
 * get the cummulative buffered duration from the represenation based on when
 * switches
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
bool DASHMediaPlayGroup::GetGroupPlaybackStats
(
 HTTPCommon::HTTPMediaType mediaType,
 uint32& nFullyDownloadedOccupancy,
 uint32& nPartiallyDownloadedOccupancy,
 uint32& nTotalOccupancy,
 uint64& nPlaybackPosition,
 uint32& nForwardMediaAvailability
)
{
  bool bResult = false;
  nPlaybackPosition = 0;
  nFullyDownloadedOccupancy = 0;
  nPartiallyDownloadedOccupancy = 0;
  nTotalOccupancy = 0;
  nForwardMediaAvailability = 0;

  uint64 endTime = 0;
  if(m_bInvalidGroup)
  {
    endTime = (uint64)m_pDASHSessionInfo->cMPDParser.GetDuration(m_cRepGroupInfo.getKey());
    if(!m_pDASHSessionInfo->cMPDParser.IsLive())
    {
      nPlaybackPosition = endTime;
      nForwardMediaAvailability = MAX_UINT32;
    }

    return true;
  }

  MediaReadStore* pReadStore = GetMediaReadStore(mediaType);

  if (pReadStore)
  {
    bResult = true;
    //If switch Q is non-empty get the tail, else use current rep for calculation
    int nRepKey = -1;
    MM_CriticalSection_Enter(pReadStore->m_dataUnitsLock);
    HTTPDataUnitElement *pDataUnitElement = NULL;
    pDataUnitElement = (HTTPDataUnitElement *)
                       ordered_StreamList_peek_front(&(pReadStore->m_dataUnitsInUseList));
    while (pDataUnitElement)
    {
      if(pDataUnitElement->state != DOWNLOAD_ERROR)
      {
        break;
      }

      pDataUnitElement = (HTTPDataUnitElement *)
                         ordered_StreamList_peek_next(&pDataUnitElement->link);
    }

    if(pDataUnitElement)
    {
      nFullyDownloadedOccupancy = pReadStore->nFullyDownloadedOccupancy;
      nPartiallyDownloadedOccupancy = pReadStore->nPartiallyDownloadedOccupancy;
      nTotalOccupancy = (pReadStore->nCurrentDownloadPosition >= pDataUnitElement->nPrevPlayPos) ?
                        (uint32)(pReadStore->nCurrentDownloadPosition - pDataUnitElement->nPrevPlayPos) : 0;
    }
    else
    {
      nFullyDownloadedOccupancy = 0;
      nPartiallyDownloadedOccupancy = 0;
      nTotalOccupancy = 0;
    }

    MM_CriticalSection_Leave(pReadStore->m_dataUnitsLock);

    if(m_pDASHSessionInfo->cMPDParser.IsLive())
    {
      //If switch Q is non-empty get the tail, else use current rep for calculation
      int nRepKey = pReadStore->GetCurrentRepresentation();
      SwitchElement switchElement = {0, 0, 0} ;
      if (pReadStore->m_cSwitchQ.PeekTail(switchElement))
      {
        nRepKey = switchElement.nRepKey;
      }

      if(nRepKey < 0)
      {
        nRepKey = 0;
      }

      uint64 lastAvailableSegmentEndTime = 0, lastAvailableSegmentStartTime = 0;
      HTTPDownloadStatus mpdStatus =
        m_pDASHSessionInfo->cMPDParser.GetLastAvailableSegmentTimeForRepresentation(
           m_pRepresentation[nRepKey].m_cRepInfo.getKey(),
           lastAvailableSegmentStartTime,
           lastAvailableSegmentEndTime);

      if(HTTPCommon::HTTPDL_SUCCESS == mpdStatus)
      {
        endTime = lastAvailableSegmentEndTime;
      }

      if(0 == endTime)
      {
        //TODO: seperate function to get current dowld pos similar to GetCurrentPlaybackPosition()
        endTime = STD_MAX(pReadStore->nCurrentDownloadPosition, nPlaybackPosition);
      }

      nForwardMediaAvailability = ((endTime > nPlaybackPosition) ? (uint32)(endTime - nPlaybackPosition) : 0);
    }
    else
    {
      nForwardMediaAvailability = MAX_UINT32;
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                   "Invalid media type %d in playgroup %u", mediaType, m_nMajorType );
  }

  return bResult;
}

/*
 * get the cummulative buffered duration from the represenation based on when
 * switches
 *
 * @param[in] HTTPMediaType media type.
 * @param[out] nPlaybackPosition populates in time uints on success
 * @param[out] nBufferedDuration duration buffered the playback position
 *
 * @return true if successful else failure
 */
bool DASHMediaPlayGroup::GetDurationBuffered
(
  HTTPCommon::HTTPMediaType mediaType,
  uint64 &nPlaybackPosition,
  uint64 &nBufferedDuration
)
{
  bool bResult = false;
  nPlaybackPosition = 0;
  nBufferedDuration = 0;
  if(m_bInvalidGroup)
  {
    return true;
  }
  (void)GetCurrentPlaybackPosition(mediaType, nPlaybackPosition);
  MediaReadStore* pReadStore = GetMediaReadStore(mediaType);

  if (pReadStore)
  {
    bResult = true;
    //If switch Q is non-empty get the tail, else use current rep for calculation
    int nRepKey = pReadStore->GetCurrentRepresentation();
    uint64 nSwitchPos = nPlaybackPosition;
    SwitchElement switchElement = {0, 0, 0} ;
    if (pReadStore->m_cSwitchQ.PeekTail(switchElement))
    {
      nRepKey = switchElement.nRepKey;
      nSwitchPos = switchElement.nSwitchTime;
    }

    DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(nRepKey);
    if (pRepHandler)
    {
      bResult = true;

      //Compute buffered duration as the difference of max download position and current
      //playback position. If downloaded has not yet started on the tail of switch queue
      //take the switch point as the max buffered position
      uint64 nBufferedPosition = 0;
     (void)pRepHandler->GetDownloadPosition(mediaType, nBufferedPosition);
      nBufferedPosition = STD_MAX(nBufferedPosition, nSwitchPos);
      if (nBufferedPosition >= nPlaybackPosition)
      {
        nBufferedDuration = (nBufferedPosition - nPlaybackPosition);
      }
      else
      {
        QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Invalid buffered pos %u playback pos %u in %d/%u rep/playgroup",
                       (uint32)nBufferedPosition, (uint32)nPlaybackPosition, nRepKey, m_nMajorType );
      }
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid representation key %d in playgroup %u", nRepKey, m_nMajorType );
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                   "Invalid media type %d in playgroup %u", mediaType, m_nMajorType );
  }

  return bResult;
}

/**
 * @brief This Method marks the group as complete when
 *        requests complete call comes for a group
 */
void DASHMediaPlayGroup::GroupRequestsCompleted(const uint64 nEndTime)
{

  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "GroupRequestsCompleted Playgroup");

  //Check if there is any download failure at the ebd of the playback
  //Download failure is checked only for the last switched representation
  //as it is expected the last switched representation should be played
  //till the end
  if(!IsDownloadComplete())
  {
    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      int nLastKey = m_cMediaReadStore[i].GetLastRepresentation();
      if (nLastKey != -1)
      {
        DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler((int)nLastKey);
        m_IslastDataDownloadFailed = pRepHandler ? pRepHandler->IsLastSegDownloadSucceed(): false;
        bool b_IsLmsgSet = pRepHandler ? pRepHandler->IsLmsgSet(): false;
        if (b_IsLmsgSet)
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "b_IsLmsgSet is true");
        }
        m_IslastDataDownloadFailed = m_IslastDataDownloadFailed && (!b_IsLmsgSet);
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "GroupRequestsCompleted group %llu isLastDwldFail %d", GetKey(), m_IslastDataDownloadFailed);
        break;
      }
    }
  }

  //Mark the group as complete, also mark END on all representations in this group
  SetDownloadComplete();
  if (m_pRepresentation)
  {
    for (int i = 0; i < m_nNumReps; i++)
    {
      DASHMediaRepresentationHandler* pRepHandler = GetRepresentationHandler(i);
      if (pRepHandler)
      {
        pRepHandler->SetEndOfSession(true);
        pRepHandler->SetEndTime(nEndTime);
      }
    }
  }

  //Mark the group as invalid if no representation is selected (so that
  //playback continues on the other group)
  if (m_nNumMediaComponents > 0)
  {
    bool bRepresentationSelected = false;
    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      int nCurrKey = m_cMediaReadStore[i].GetCurrentRepresentation();
      if (nCurrKey != -1)
      {
        bRepresentationSelected = true;
        break;
      }
    }

    if(!bRepresentationSelected)
    {
      if(IsSeekPending())
      {
        //If on seek QSM tried select on bad rep with successive DATAINFO failures till end
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Seek error for media (or major) type %u", m_nMajorType );

        m_eSeekStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
        m_nSeekEndTime = -1;
        m_nSeekCurTime = -1;
        SetSeekPending(false);

        if(m_pPeriodNotifier)
        {
          m_pPeriodNotifier->NotifySeekStatus(m_nKey,m_nSeekTime,m_eSeekStatus);
        }
      }

      uint64 endTime = 0;
      endTime = (uint64)m_pDASHSessionInfo->cMPDParser.GetDuration(m_cRepGroupInfo.getKey());

      if((!m_pDASHSessionInfo->cMPDParser.IsLive()) && (endTime > 0))
      {
        m_nStartTime = endTime;
      }

      MM_CriticalSection_Enter(m_pGroupDataLock);
      Close();
      m_bInvalidGroup = true;
      MM_CriticalSection_Leave(m_pGroupDataLock);
    }
  }
  return;
}

/**
 * @brief This Method checks if there is any switch point queued
 *        and issues seek on the new switch point closing the
 *        previous switch, if there is no switch queued then
 *        seek is reissued on the same representation
 */
void DASHMediaPlayGroup::CheckAndHandleSwitchDuringSeek()
{
  if(m_eSeekStatus != HTTPCommon::HTTPDL_SUCCESS)
  {
    bool bSwitched = false;
    bool bResetSeekStatus = false;

    for (int i = 0; i < m_nNumMediaComponents; i++)
    {
      SwitchElement sElem = {0,0,0};
      SwitchElement sHeadElem = {0,0,0};
      void* pIt = NULL;
      m_cMediaReadStore[i].m_cSwitchQ.PeekHead(sHeadElem);
      if( m_cMediaReadStore[i].m_cSwitchQ.Count() >= 2)
      {
        m_cMediaReadStore[i].m_cSwitchQ.Next(pIt, sElem);
        m_cMediaReadStore[i].m_cSwitchQ.Next(pIt, sElem);
        //TODO: Remove Seek tolerance threashold. Temporary change for somc unaligned reps.
        if(m_eSeekStatus == HTTPCommon::HTTPDL_SEGMENT_BOUNDARY || m_eSeekStatus == HTTPCommon::HTTPDL_NO_MORE_RESOURCES ||
          (m_eSeekStatus == HTTPCommon::HTTPDL_NEXT_SEGMENT &&
          (m_nSeekEndTime > -1) &&(uint64)m_nSeekEndTime + SEEK_TOLERANCE_THREASHOLD >= sElem.nSwitchTime ))
        {
          DASHMediaRepresentationHandler* pCurRepHandler = GetRepresentationHandler((int)sHeadElem.nRepKey);
          DASHMediaRepresentationHandler* pNextRepHandler = GetRepresentationHandler((int)sElem.nRepKey);
          if(pNextRepHandler)
          {
            pNextRepHandler->Open(sElem.nDataUnitKey,sElem.nSwitchTime,true);
          }

          //Close the head element and call open on
          m_cMediaReadStore[i].m_cSwitchQ.DeQ(sHeadElem);
          if(pCurRepHandler)
          {
            pCurRepHandler->Close(m_cMediaReadStore[i].GetMediaType());
          }

          bSwitched = true;
          bResetSeekStatus = true;
        }
      }
    }

    if(bSwitched ==false)
    {
      if(m_eSeekStatus == HTTPCommon::HTTPDL_SEGMENT_BOUNDARY ||
         m_eSeekStatus == HTTPCommon::HTTPDL_NO_MORE_RESOURCES ||
         m_eSeekStatus == HTTPCommon::HTTPDL_NEXT_SEGMENT)
      {
        for (int i = 0; i < m_nNumMediaComponents; i++)
        {
          int nCurrKey = m_cMediaReadStore[i].GetCurrentRepresentation();
          DASHMediaRepresentationHandler* pCurRepHandler = GetRepresentationHandler(nCurrKey);
          //Call a seek on current rep handler with -1. RepHandler will call the seek on first
          //valid segment in resource list(which is not in error state) and will remove all the
          //segments which are in error state before first valid segment.
          if (pCurRepHandler)
          {
            pCurRepHandler->Seek(m_nSeekCurTime);
          }
        }
      }
    }

    if(bResetSeekStatus)
    {
      m_nSeekEndTime = -1;
      m_nSeekCurTime = -1;
      m_eSeekStatus = HTTPCommon::HTTPDL_SUCCESS;
    }
  }
  return;
}

/** @brief Update the read store download stats for the data unit.
  *
  * @param[in] nRepKey - Representation key
  * @param[in] nDataUnitKey - Data unit to be updated
  * @param[in] nStartTime - Start time of the Data unit
  * @param[in] nDuration - Duration of the Data unit
  * @return
  * none
  */
void DASHMediaPlayGroup::MediaReadStore::UpdateMediaReadStoreStats
(
  uint64 nPlaybackPos
)
{
  HTTPDataUnitElement *pDataUnitElement = NULL;

  MM_CriticalSection_Enter(m_dataUnitsLock);

  pDataUnitElement = (HTTPDataUnitElement *)
                      ordered_StreamList_peek_front(&m_dataUnitsInUseList);
  while (pDataUnitElement)
  {
    //Remove all the data units from the list
    //- data unit end time is lesser than the current playbackpos or
    //- data unit is in download error state
    if((DOWNLOAD_ERROR == pDataUnitElement->state) ||
      ((pDataUnitElement->nStartTime + pDataUnitElement->nDuration) < nPlaybackPos))
    {
      //Adjust the time difference in case there is duration gap between data unit end time
      //And the last sample end time from the data unit
      if(DOWNLOAD_COMPLETE == pDataUnitElement->state)
      {
        if (nFullyDownloadedOccupancy >=
            (uint32)((pDataUnitElement->nStartTime + pDataUnitElement->nDuration) - pDataUnitElement->nPrevPlayPos))
        {
          nFullyDownloadedOccupancy -= (uint32)((pDataUnitElement->nStartTime + pDataUnitElement->nDuration) -
                                       pDataUnitElement->nPrevPlayPos);
        }
        else
        {
          QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Error in updating full occupancy %u duration %u start %llu prev %llu",
                         nFullyDownloadedOccupancy, pDataUnitElement->nDuration,
                         pDataUnitElement->nStartTime, pDataUnitElement->nPrevPlayPos );
          nFullyDownloadedOccupancy = 0;
        }

        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "calculation1 - end time %llu",
                       pDataUnitElement->nPrevPlayPos);
      }

      //Remove the data unit from the Inuse list
      ordered_StreamList_pop_item(&m_dataUnitsInUseList, &pDataUnitElement->link);

      //Insert data unit in the Free list for reuse
      ordered_StreamList_push(&m_dataUnitsFreeList,
                              &pDataUnitElement->link,
                              pDataUnitElement->nStartTime);

      pDataUnitElement = (HTTPDataUnitElement *)
                         ordered_StreamList_peek_front(&m_dataUnitsInUseList);
      continue;
    }

    //Updata the stats corresponds to the current sample read. Due to the integer
    //division in the calculation of start time of the sample in the Parser,
    //duration gap is found between the samples of the same data units, i.e. Start
    //time of next sample is not exactly equal to end time of the current sample.
    //To adjust the duration gap between the samples, each sample end time is stored,
    //and used for calculation of the duration of next sample by taking a difference
    //with stored previous sample end time
    else if((nPlaybackPos > pDataUnitElement->nStartTime) &&
           (nPlaybackPos <= (pDataUnitElement->nStartTime + pDataUnitElement->nDuration)))
    {
      //Ignore the B frame in the calculation of stats
      if(nPlaybackPos > pDataUnitElement->nPrevPlayPos)
      {
        if(DOWNLOAD_PARTIAL == pDataUnitElement->state)
        {
          if (nPartiallyDownloadedOccupancy >= (uint32)(nPlaybackPos - pDataUnitElement->nPrevPlayPos))
          {
            nPartiallyDownloadedOccupancy -= (uint32)(nPlaybackPos - pDataUnitElement->nPrevPlayPos);
          }
          else
          {
            QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Error in updating partial occupancy %u current %llu prev %llu",
                           nFullyDownloadedOccupancy, nPlaybackPos, pDataUnitElement->nPrevPlayPos );
            nPartiallyDownloadedOccupancy = 0;
          }
        }
        else if(DOWNLOAD_COMPLETE == pDataUnitElement->state)
        {
          if (nFullyDownloadedOccupancy >= (uint32)(nPlaybackPos - pDataUnitElement->nPrevPlayPos))
          {
            nFullyDownloadedOccupancy -= (uint32)(nPlaybackPos - pDataUnitElement->nPrevPlayPos);
          }
          else
          {
            QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Error in updating full occupancy %u current %llu prev %llu",
                           nFullyDownloadedOccupancy, nPlaybackPos, pDataUnitElement->nPrevPlayPos );
            nFullyDownloadedOccupancy = 0;
          }
        }

        //Store the current sample end time.
        pDataUnitElement->nPrevPlayPos = nPlaybackPos;
      }
      break;
    }
    else if(nPlaybackPos <= pDataUnitElement->nStartTime)
    {
      break;
    }

    pDataUnitElement = (HTTPDataUnitElement *)
                        ordered_StreamList_peek_next(&pDataUnitElement->link);
  } //while (pDataUnitElement)

  MM_CriticalSection_Leave(m_dataUnitsLock);
}

/** @brief Update the read store download stats for the data unit.
  *
  * @param[in] nRepKey - Representation key
  * @param[in] nDataUnitKey - Data unit to be updated
  * @param[in] nStartTime - Start time of the Data unit
  * @param[in] nDuration - Duration of the Data unit
  * @return
  * none
  */
void DASHMediaPlayGroup::MediaReadStore::UpdateMediaReadStoreStats
(
 uint64 nRepKey,
 uint64 nDataUnitKey,
 uint64 nStartTime,
 uint32 nDuration
)
{
  HTTPCommon::HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_WAITING;

  MM_CriticalSection_Enter(m_dataUnitsLock);

  HTTPDataUnitElement *pDataUnitElement = NULL;
  while (eReturn == HTTPCommon::HTTPDL_WAITING)
  {
    //Pop data unit element from Free List
    pDataUnitElement =
    (HTTPDataUnitElement *)ordered_StreamList_pop_front(&m_dataUnitsFreeList);

    if ( pDataUnitElement )
    {
      //Update the data unit download parameters
      pDataUnitElement->nDataUnitKey = nDataUnitKey;
      pDataUnitElement->nRepKey      = nRepKey;
      pDataUnitElement->nStartTime   = nStartTime;
      pDataUnitElement->nDuration    = nDuration;
      pDataUnitElement->nPrevPlayPos = nStartTime;
      pDataUnitElement->state        = DOWNLOAD_PARTIAL;

      //Update Media store download stats
      nPartiallyDownloadedOccupancy += pDataUnitElement->nDuration;
      nCurrentDownloadPosition       = STD_MAX(nCurrentDownloadPosition,
                                       (pDataUnitElement->nStartTime + pDataUnitElement->nDuration));

      // put the element to the inuse list, insertion into the list always happens
      // on the data segment request. All the data units with download not complete
      // are inserted at the end of the list
      ordered_StreamList_push(&m_dataUnitsInUseList,
                              &pDataUnitElement->link,
                              pDataUnitElement->nStartTime);

      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                     "added DataUnit with key [%d %d] to the active list",
                     (int)(nDataUnitKey >> 32), (int)nDataUnitKey );
      eReturn = HTTPCommon::HTTPDL_SUCCESS;
    }
    else
    {
      //add more resources if the max resource limit has been reached.
      StreamList_size_type inUseListSize = ordered_StreamList_size(&m_dataUnitsInUseList);

      //Reallocate the extra Memory
      uint32 nNewPoolSize = (uint32)inUseListSize + HTTP_MAX_DATA_UNITS;
      HTTPDataUnitElement* pNewPool =
        (HTTPDataUnitElement*)QTV_Realloc(m_dataUnits, (nNewPoolSize * sizeof(HTTPDataUnitElement)));
      if (pNewPool)
      {
        m_dataUnits = pNewPool;
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                     "No more free dataunit available, creating new pool of %d",
                     HTTP_MAX_DATA_UNITS );

        ordered_StreamList_init(&m_dataUnitsFreeList,
                                ORDERED_STREAMLIST_ASCENDING,
                                ORDERED_STREAMLIST_PUSH_SLT);
        ordered_StreamList_init(&m_dataUnitsInUseList,
                                ORDERED_STREAMLIST_ASCENDING,
                                ORDERED_STREAMLIST_PUSH_SLT);

        //Update inUse list after reallocation
        for( uint32 j = 0; j < inUseListSize; j++ )
        {
          ordered_StreamList_push(&m_dataUnitsInUseList, &m_dataUnits[j].link, m_dataUnits[j].nStartTime);
        }

        //Update Free list after reallocation
        for( uint32 k = (uint32)inUseListSize; k < nNewPoolSize; k++ )
        {
          m_dataUnits[k].Reset();
          ordered_StreamList_push(&m_dataUnitsFreeList, &m_dataUnits[k].link, k);
        }
      }
      else
      {
        eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
      }
    }
  } //while (eReturn == HTTPCommon::HTTPDL_WAITING)

  MM_CriticalSection_Leave(m_dataUnitsLock);
}

/** @brief Update the read store download stats for the data unit.
  *
  * @param[in] nRepKey - Representation key
  * @param[in] nDataUnitKey - Data unit to be updated
  * @param[in] eStatus - Download status
  * @return
  * none
  */
void DASHMediaPlayGroup::MediaReadStore::UpdateMediaReadStoreStats
(
 uint64 nRepKey,
 uint64 nDataUnitKey,
 HTTPDownloadStatus eStatus
)
{
  HTTPDataUnitElement *pDataUnitElement = NULL;

  MM_CriticalSection_Enter(m_dataUnitsLock);

  pDataUnitElement = (HTTPDataUnitElement *)
                      ordered_StreamList_peek_back(&m_dataUnitsInUseList);
  while (pDataUnitElement)
  {
    if((pDataUnitElement->nDataUnitKey == nDataUnitKey) &&
       (pDataUnitElement->nRepKey == nRepKey))
    {
      break;
    }

    pDataUnitElement = (HTTPDataUnitElement *)
                        ordered_StreamList_peek_prev(&pDataUnitElement->link);
  }

  //Update download stats based on the download status for the data unit
  if(pDataUnitElement)
  {
    uint32 nPlayed = pDataUnitElement->nDuration - (uint32)(pDataUnitElement->nPrevPlayPos - pDataUnitElement->nStartTime);
    if (nPartiallyDownloadedOccupancy >= nPlayed)
    {
      nPartiallyDownloadedOccupancy -= nPlayed;
    }
    else
    {
      QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error in updating partial occupancy %u duration %u start %llu prev %llu",
                     nPartiallyDownloadedOccupancy, pDataUnitElement->nDuration,
                     pDataUnitElement->nStartTime, pDataUnitElement->nPrevPlayPos );
      nPartiallyDownloadedOccupancy = 0;
    }
    nCurrentDownloadPosition = STD_MAX(nCurrentDownloadPosition,
                                       (pDataUnitElement->nStartTime + pDataUnitElement->nDuration));
    if((HTTPCommon::HTTPDL_WAITING == eStatus) ||
       (HTTPCommon::HTTPDL_SUCCESS == eStatus))
    {
      nFullyDownloadedOccupancy += nPlayed;
      pDataUnitElement->state = DOWNLOAD_COMPLETE;
    }
    else
    {
      pDataUnitElement->state = DOWNLOAD_ERROR;
    }
  }
  else
  {
    //DataUnitKey not found in inuse dataunit list
    QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Error Not Found DataUnit rep %llu data %llu status %d", nRepKey, nDataUnitKey, eStatus);
  }

  MM_CriticalSection_Leave(m_dataUnitsLock);
}

} // namespace video
