/************************************************************************* */
/**
 * HTTPSourceMMITrackHandler.cpp
 * @brief Implementation of HTTPSourceMMITrackHandler.
 *  HTTPSourceMMITrackHandler class implements the iTrackContainer and
 *  iDataReader interfaces. Besides some HTTP specific functionality, it
 *  delegates most calls to File Source.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMITrackHandler.cpp#63 $
$DateTime: 2013/09/03 20:46:31 $
$Change: 4375062 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMITrackHandler.cpp
** ======================================================================= */
#include "httpInternalDefs.h"
#include "HTTPController.h"
#include "HTTPSourceMMI.h"
#include "HTTPSourceMMIPropertiesHandler.h"
#include "HTTPSourceMMITrackHandler.h"
#include "HTTPSourceMMIExtensionHandler.h"
#include "OMX_Image.h"

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
//Maintain the same order as FileSource::FileSourceMjMediaType
const HTTPDiagInterfaceHandler::eCodecType
HTTPSourceMMITrackHandler::MajorTypeToCodecTable[] =
{
  HTTPDiagInterfaceHandler::UNKNOWN,
  HTTPDiagInterfaceHandler::AUDIO,
  HTTPDiagInterfaceHandler::VIDEO,
  HTTPDiagInterfaceHandler::TEXT
};

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief Initialize the track.
  *
  * @param[in] pHTTPController - Reference to HTTP controller
  * @param[in] pHTTPSourceMMIPropertiesHandler - Reference to MMI property handler
  * @param[out] bResult - "true" the initialization was success else failure
  */
HTTPSourceMMITrackHandler::HTTPSourceMMITrackHandler
(
 HTTPController* pHTTPController,
 HTTPSourceMMIPropertiesHandler* pHTTPSourceMMIPropertiesHandler,
 bool &bResult
): m_pHTTPController(pHTTPController),
   m_pHTTPSourceMMIPropertiesHandler(pHTTPSourceMMIPropertiesHandler),
   m_nRebuffPreroll(0),
   m_bInbandH264ParamSet(true),
   m_bEOS(false),
   m_nAbsoluteSeekOffset(0),
   m_bIsSeekPending(false),
   m_pTrackHandlerDataLock(NULL),
   m_lastPSSHIndexQueried(-1),
   m_nUniqueID(0),
   m_nLastAudioPsshUniqId(0),
   m_nLastVideoPsshUniqId(0)
{
  bResult = true;
  if (m_pHTTPController)
  {
    m_nRebuffPreroll = m_pHTTPController->GetRebufferPreroll();
  }
  if ( MM_CriticalSection_Create(&m_pTrackHandlerDataLock) != 0 )
  {
    bResult = false;
  }
  //initialising Track DRM info array
  for (uint32 i = 0; i < MAX_PSSH_INFO_ENTRY ; i++)
  {
    InitializeTrackDescDrmInfo(TrackDrmInfo[i]);
  }
  PrintTrackDRMInfo();
}

/** @brief Sets the data interface
  *
  * @param[in] pDataInterface - Reference to data interface
  * @param[in] pDataResourceManager - Reference to data resource manager
  */
void HTTPSourceMMITrackHandler::SetDataInterface
(
 HTTPDataInterface* pDataInterface
)
{
  m_pDataInterface = pDataInterface;
}

/** @brief Resets a local cached information, prepare for the another session.
*
*/
void HTTPSourceMMITrackHandler::Close()
{
  (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
  m_bInbandH264ParamSet = true;
  m_bEOS = false;
  SetSeekPending(false);
  m_nAbsoluteSeekOffset = 0;
  m_trackList.clear();
  (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
}


/** @brief Selects or mutes (or deselects or unmutes) a given track.
*
* @param[in] trackIdentifier - Track identifier
* @param[in] majorType - HTTP media major type
* @param[in] trackState - Current state of the track
* @param[in] pResultRecipient - Reference to the callback handler for job
*                               result notification
*
* @return
* TRUE - Job accepted successfully
* FALSE - Otherwise
*/
bool HTTPSourceMMITrackHandler::SetTrackState
(
 const int trackIdentifier,
 HTTPMediaType majorType,
 const tTrackState trackState,
 void* pResultRecipient
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMITrackHandler::SetTrackState" );
  bool bOk = false;

  if (m_pHTTPController && m_pHTTPController->IsHTTPStreamerRunning())
  {
    //Get the trackDescription and update the track state if found
    TrackDescription* pTrackDescription = NULL;
    if (FindTrack(trackIdentifier, majorType, &pTrackDescription) &&
        pTrackDescription)
    {
      //Queue the SET_TRACK_STATE request on HTTPController
      if (m_pHTTPController->SetTrackState(trackIdentifier,
                                           trackState.isSelected,
                                           reinterpret_cast<void *> (pResultRecipient)))
      {
        //SET_TRACK_STATE request queued, notification will be sent later
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "HTTP SET_TRACK_STATE request queued" );
        (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
        pTrackDescription->selectionState = trackState;
        (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
        bOk = true;
      }
      else
      {
        //Failed to queue SET_TRACK_STATE request
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: Failed to queue HTTP SET_TRACK_STATE request" );
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either m_pHTTPController is NULL or "
                  "HTTP streamer thread inactive" );
  }

  return bOk;
}

/** @brief Read data from media (synchronously or asynchronously) -
* delegate to File Source. Currently only synchronous operation (specified
* by a timeout of 0) is supported and will return UNDERRUN immediately if
* data sample is unavailable at the time of read.
*
* @param[in] trackIdentifier - Track identifier
* @param[in] majorType - HTTP media major type
* @param[in] timeout - Read timeout for asynchronous operation
* @param[out] pDataBlock - Media sample data
* @param[out] pDataHeader - Media sample data header
* @param[in] offset - Buffer offset
* @param[out] bSwitch - Flag to indicate possible codec switch
*
* @return
* rc_OK - Data successfully read
* rc_underrun - Data underrun (buffering)
* rc_endOfStream - EOF
* rc_trackDeselected - Read attempted on an unselected track
* rc_trackMuted - Read attempted on a muted track
* rc_notOK - Generic failure
*/
OMX_U32 HTTPSourceMMITrackHandler::Read
(
 const int32 trackIdentifier,
 HTTPMediaType majorType,
 const int64 timeout,
 OMX_BUFFERHEADERTYPE *pBuffHdr,
 int32 offset,
 bool& bSwitch
)
{
  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                 "HTTPSourceMMITrackHandler::Read for media type %d track %d ",
                 majorType, trackIdentifier );
  OMX_U32 ret = MMI_S_EBADPARAM;
  OMX_U32 nPortIndex;
  (void)timeout;
  uint64 nStartTime =0;
  uint32 sampleSize = pBuffHdr ? (pBuffHdr->nAllocLen - offset) : 0;
  bSwitch = false;
  QOMX_PARAM_STREAMING_PSSHINFO *psshInfo = NULL;
  QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo = NULL;
  if( IsSeekPending() )
  {
    //There might be a switch from A->AV after seek. So return pending from
    //here till seek is complete and tracklist is updated
    return MMI_S_PENDING;
  }
  //Check if valid track
  TrackDescription* pTrackDescription = NULL;
  if (FindTrack(trackIdentifier, majorType, &pTrackDescription) &&
      pTrackDescription)
  {
    ret = MMI_S_PENDING;
  }
  else
  {
    return MMI_S_PENDING;
  }

  if (pBuffHdr == NULL ||
      ret != MMI_S_PENDING)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Read failed for media type %d track %d"
                  "- either m_pFileSource is NULL or invalid input",
                  majorType, trackIdentifier );
  }
  else if (pTrackDescription && !pTrackDescription->selectionState.isSelected)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Read attempted on an unselected  media type %d track %d",
                   majorType, trackIdentifier );
  }
  else if (pTrackDescription && pTrackDescription->selectionState.isMuted)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Read attempted on a muted media type %d track %d ",
                   majorType, trackIdentifier );
  }
  else
  {
    //Check for buffering before attempting to fill data buffer
    HTTPDownloadStatus status = CanPlayTrack(trackIdentifier, majorType);
    if (status == HTTPCommon::HTTPDL_WAITING)
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "Data underrun for media type %d track %d",
                     majorType, trackIdentifier );
      ret = MMI_S_PENDING;
    }
    else if (!HTTPSUCCEEDED(status))
    {
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "CanPlayTrack() failed %d for media type %d track %d",
                     status, majorType, trackIdentifier );
      ret = MMI_S_EFAIL;
    }
    else
    {
      //Extract media sample and fill in data header
      FileSourceMediaStatus status = FILE_SOURCE_DATA_ERROR;
      HTTPSampleInfo httpSampleInfo;

      //Reset the Sample Info structure
      std_memset((void*)&httpSampleInfo, 0x0, sizeof(httpSampleInfo));
      HTTPDownloadStatus httpStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
      if (m_pDataInterface)
      {
         httpStatus = m_pDataInterface->GetNextMediaSample(
          majorType, pBuffHdr->pBuffer + offset, sampleSize, httpSampleInfo);

        switch (httpStatus)
        {
        case HTTPCommon::HTTPDL_SUCCESS:
          status = FILE_SOURCE_DATA_OK;
          break;
        case HTTPCommon::HTTPDL_DATA_END:
          status = FILE_SOURCE_DATA_END;
          break;
        case HTTPCommon::HTTPDL_DATA_END_WITH_ERROR:
          ret = MMI_S_EFATAL;
          status = FILE_SOURCE_DATA_UNDERRUN;
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "HTTPDL_DATA_END_WITH_ERROR in Read path due to download failure");
          break;
        case HTTPCommon::HTTPDL_WAITING:
          status = FILE_SOURCE_DATA_UNDERRUN;
          break;
        case HTTPCommon::HTTPDL_CODEC_INFO:
          status = FILE_SOURCE_DATA_CODEC_INFO;
          break;
        case HTTPCommon::HTTPDL_SWITCH:
          //Source has switched to a new representation, try reading again
          //after comparing codec info
          status = FILE_SOURCE_DATA_UNDERRUN;
          bSwitch = true;
          break;
        default:
          status = FILE_SOURCE_DATA_ERROR;
          break;
        }
      }

      switch (status)
      {
      case FILE_SOURCE_DATA_OK:
        pBuffHdr->nFilledLen = sampleSize + offset;
        pBuffHdr->nOffset = 0;

        //convert msec to usec
        nStartTime = ((uint64)(httpSampleInfo.startTime))*1000;
  #ifndef OMX_SKIP64BIT
        pBuffHdr->nTimeStamp= (unsigned long long)nStartTime;
  #else
        pBuffHdr->nTimeStamp.nLowPart = (unsigned long)nStartTime;
        pBuffHdr->nTimeStamp.nHighPart = (unsigned long)(nStartTime >> 32);
  #endif

        if(httpSampleInfo.sync)
        {
          pBuffHdr->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
        }
        if ( pTrackDescription->minorType != FILE_SOURCE_MN_TYPE_DIVX311 &&
             pTrackDescription->minorType != FILE_SOURCE_MN_TYPE_DIVX40 &&
             pTrackDescription->minorType != FILE_SOURCE_MN_TYPE_DIVX50_60 )
        {
          pBuffHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
        }
        ret = MMI_S_COMPLETE;

        // if sample is encrypted, send PSSH and extra sample info in pBuffHdr
        if ((majorType == HTTPCommon::HTTP_AUDIO_TYPE || majorType == HTTPCommon::HTTP_VIDEO_TYPE)
            &&  httpSampleInfo.sSubInfo.sContentProtection.ucIsEncrypted)
        {
          MapMediaTypeToPortID(nPortIndex, majorType);
          FileSourceMjMediaType fsMajorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
          HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType( majorType, fsMajorType );
          psshInfo = (QOMX_PARAM_STREAMING_PSSHINFO *)QTV_Malloc(sizeof(QOMX_PARAM_STREAMING_PSSHINFO));
          if (psshInfo)
          {
            memset(psshInfo, 0, sizeof(QOMX_PARAM_STREAMING_PSSHINFO));
            psshInfo->nVersion = pBuffHdr->nVersion;
            psshInfo->nPortIndex = nPortIndex;
            psshInfo->nUniqueID = GetPsshUniqueId(fsMajorType);
            psshInfo->nPsshDataBufSize = 0;
            psshInfo->cPSSHData[0] = '\0';
            psshInfo->nSize = (OMX_U32)sizeof(QOMX_PARAM_STREAMING_PSSHINFO);

            union
            {
              HTTPContentProtectionType* in;
              QOMX_EXTRA_SAMPLE_INFO* out;
            }u;
            u.in = &httpSampleInfo.sSubInfo.sContentProtection;
            QOMX_EXTRA_SAMPLE_INFO *extrasample = u.out;
            if (FillExtraDataForExtraSampleInfo(nPortIndex,
                                           pBuffHdr,
                                           psshInfo,
                                           extrasample
                                         )!= MMI_S_COMPLETE)
            {
              ret = MMI_S_EFAIL;
            }
            QTV_Free(psshInfo);
            psshInfo = NULL;
          }
          else
          {
            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_FATAL,
                       "FILE_SOURCE_DATA_OK - Error in extra sample memory Allocation");
             ret = MMI_S_EFAIL;
          }
        }
        if(majorType == HTTPCommon::HTTP_TEXT_TYPE &&
           pTrackDescription->minorType == FILE_SOURCE_MN_TYPE_SMPTE_TIMED_TEXT )
        {
          ret = FillExtraDataForSubTitles(pBuffHdr->nOutputPortIndex,pBuffHdr,pTrackDescription,httpSampleInfo);
        }

        //First successful sample retrieval means that initial buffering
        //is over - so set the track preroll to rebuff preroll
        if (!pTrackDescription->reBuffering)
        {
         (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
          pTrackDescription->reBuffering = true;
         (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
        }
        break;

      case FILE_SOURCE_DATA_END:
        pBuffHdr->nFilledLen = 0;
        pBuffHdr->nOffset = 0;
        pBuffHdr->nFlags |= OMX_BUFFERFLAG_EOS;
        (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
        pTrackDescription->bEndOfTrack = true;
        (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
        ret = MMI_S_COMPLETE;
        break;

      case FILE_SOURCE_DATA_UNDERRUN:
      case FILE_SOURCE_DATA_INSUFFICIENT:
        if(httpStatus != HTTPCommon::HTTPDL_DATA_END_WITH_ERROR)
        {
          ret = MMI_S_PENDING;
        }

        if (!bSwitch)
        {
          (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
          pTrackDescription->playState = BUFFERING;
          (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Re-buffering started for track %d",
                         trackIdentifier);
        }
        break;
      case FILE_SOURCE_DATA_CODEC_INFO:
        pBuffHdr->nFilledLen = sampleSize + offset;
        pBuffHdr->nOffset = 0;
        pBuffHdr->nFlags = OMX_BUFFERFLAG_CODECCONFIG;

        //convert msec to usec
        nStartTime = ((uint64)(httpSampleInfo.startTime))*1000;
  #ifndef OMX_SKIP64BIT
        pBuffHdr->nTimeStamp= (unsigned long long)nStartTime;
  #else
        pBuffHdr->nTimeStamp.nLowPart = (unsigned long)nStartTime;
        pBuffHdr->nTimeStamp.nHighPart = (unsigned long)(nStartTime >> 32);
  #endif
        ret = MMI_S_COMPLETE;
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "isDRMProtected %d",pTrackDescription->isDRMProtected );
        if (pTrackDescription->isDRMProtected)
        {
          MapMediaTypeToPortID(nPortIndex, majorType);
          FileSourceMjMediaType fsMajorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
          HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType( majorType, fsMajorType );
          psshInfo = (QOMX_PARAM_STREAMING_PSSHINFO *)QTV_Malloc(sizeof(QOMX_PARAM_STREAMING_PSSHINFO));
          if (psshInfo)
          {
            psshInfo->nVersion = pBuffHdr->nVersion;
            psshInfo->nPortIndex = nPortIndex;
            psshInfo->nUniqueID = GetPsshUniqueId(fsMajorType);
            psshInfo->nPsshDataBufSize = 0;
            psshInfo->cDefaultKeyID[0] = '\0';
            psshInfo->nSize = (OMX_U32)sizeof(QOMX_PARAM_STREAMING_PSSHINFO);
            //if (FillExtraDataForPsshInfo(nPortIndex, pBuffHdr, psshInfo) != MMI_S_COMPLETE)
            if (FillExtraDataForExtraSampleInfo(nPortIndex,
                                         pBuffHdr,
                                         psshInfo) !=
                                         MMI_S_COMPLETE)
            {
              ret = MMI_S_EFAIL;
            }
            QTV_Free(psshInfo);
            psshInfo = NULL;
          }
          else
          {
            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "FILE_SOURCE_DATA_CODEC_INFO - Error in pssh memory Allocation");
          }
        }
        if(majorType == HTTPCommon::HTTP_TEXT_TYPE &&
            pTrackDescription->minorType == FILE_SOURCE_MN_TYPE_SMPTE_TIMED_TEXT )
        {
          ret = FillExtraDataForSubTitles(pBuffHdr->nOutputPortIndex,pBuffHdr,pTrackDescription,httpSampleInfo);
        }
        break;
      default:
        ret = MMI_S_EFAIL;
        pBuffHdr->nFilledLen = 0;
        pBuffHdr->nOffset = 0;
        pBuffHdr->nFlags |= OMX_BUFFERFLAG_DATACORRUPT;
        break;
      }
  #ifndef OMX_SKIP64BIT
      QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Read sample for media %d - status %d, "
                     "size %d bytes, timestamp %u msec, isSyncPoint %d httpstatus %d",
                     (int)pTrackDescription->majorType, (int)status, (int)pBuffHdr->nFilledLen,
                     (uint32)pBuffHdr->nTimeStamp/1000, (int)httpSampleInfo.sync,(int)httpStatus );
  #else
      QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Read sample for media %d - status %d, "
                     "size %d bytes, timestamp %u %u usec, isSyncPoint %d",
                     pTrackDescription->majorType, status, pBuffHdr->nFilledLen,
                     pBuffHdr->nTimeStamp.nLowPart,pBuffHdr->nTimeStamp.nHighPart,
                     dataHeader.sync );
  #endif

      //Logging the media sample (as log packet)
      if ((unsigned int)pTrackDescription->majorType < ARR_SIZE(MajorTypeToCodecTable) &&
          m_pHTTPSourceMMIPropertiesHandler && m_pHTTPSourceMMIPropertiesHandler->IsMediaSampleLoggingAllowed())
      {
        HTTPDiagInterfaceHandler::tHTTPSampleInfo sampleInfo = {0,0,0,0,0};
        sampleInfo.trackID = (uint8)trackIdentifier;
        sampleInfo.absTS = (uint32)(((float)httpSampleInfo.startTime * 0.001F) * (float)pTrackDescription->timeScale);;
        sampleInfo.dispTS = (uint32)httpSampleInfo.startTime;
        sampleInfo.layerID = 0;
        sampleInfo.duration = 0;
        HTTPDiagInterfaceHandler::LogHTTPMediaSample(MajorTypeToCodecTable[pTrackDescription->majorType],
                                                     (const char*)pBuffHdr->pBuffer,
                                                     pBuffHdr->nFilledLen,
                                                     sampleInfo);
      }
    }
  }

  return ret;
}

/** @brief Provides format block for requested media type
*
* @param[in] mediaType - Media type
* @param[out] buf - buffer for format block
* @param[in/out] size - size of the buffer
*
* @return operation status
*/
OMX_U32 HTTPSourceMMITrackHandler::GetFormatBlock
(
 const HTTPMediaType mediaType,
 uint8 *buf,
 uint32 &size
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPSourceMMITrackHandler::FormatBlock for mediaType %d", mediaType );
  OMX_U32 ret = MMI_S_EFAIL;

  if (m_pDataInterface)
  {
    //Return cached copy
    //ToDo: Consolidate all track params in one place, e.g. AAC params is also
    //cached in MMI port info!
    TrackDescription* pTrackDescription = NULL;
    if (FindMediaType(mediaType, &pTrackDescription))
    {
      if (NULL == buf)
      {
        size = pTrackDescription->config.size;
        ret = MMI_S_COMPLETE;
      }
      else if ((pTrackDescription->config.size > 0) && (size > 0))
      {
        //Copy config data into dest buf
        size = STD_MIN(pTrackDescription->config.size, size);
        std_memmove(buf, pTrackDescription->config.data, size);
        ret = MMI_S_COMPLETE;
      }
    }
  }

  return ret;
}

/** @brief Provides config data for requested media type
*
* @param[in] mediaType - Media type
* @param[out] buf - buffer for format block
* @param[in/out] size - size of the buffer
*
* @return operation status
*/
HTTPDownloadStatus HTTPSourceMMITrackHandler::GetConfigData
(
 const HTTPMediaType mediaType,
 uint8 *buf,
 uint32 &size
 )
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPSourceMMITrackHandler::GetConfigData for mediaType %d", mediaType );
  HTTPDownloadStatus ret = HTTPCommon::HTTPDL_ERROR_ABORT;

  //Extract format block
  if (m_pDataInterface)
  {
    uint32 nLength = 0;

    // get the length to begin with
    (void)m_pDataInterface->GetFormatBlock(mediaType, NULL, nLength);
    if (NULL == buf)
    {
      size = nLength;
      ret = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
    }
    else if ((nLength > size) && (size > 0))
    {
      //partial copy
      uint8 *pTmpBuf = (uint8 *) QTV_Malloc(sizeof(uint8) * nLength);
      if (pTmpBuf)
      {
        ret = m_pDataInterface->GetFormatBlock(mediaType, pTmpBuf, nLength);
        if (ret == HTTPCommon::HTTPDL_SUCCESS)
        {
          std_memmove(buf, pTmpBuf, size);
        }
        QTV_Free(pTmpBuf);
      }
    }
    else if ((nLength > 0) && (size > 0))
    {
      // get the format block
      ret = m_pDataInterface->GetFormatBlock(mediaType, buf, size);
    }
  }
  return ret;
}


/*
* @brief Get buffering progress as a fractional percentage.
*
* @param[in]  trackID - Track id
* @param[in] majorType - HTTP media major type
* @param[out] buffProgress - Duration buffered (in msecs)
*
* @return
* TRUE - Buffering progress obtained successfully
* FALSE - Otheriwse
*/
bool HTTPSourceMMITrackHandler::GetBufferingProgress
(
  const int32 trackID,
  HTTPMediaType majorType,
  uint64& buffProgress,
  uint64& dwldPos
 )
{
  bool bOk = false;
  uint64 downloadPos = 0, currPos = 0;
  buffProgress = dwldPos = 0;

  //Check if valid track
  TrackDescription* pTrackDescription = NULL;
  if (FindTrack(trackID, majorType, &pTrackDescription))
  {
    if (pTrackDescription)
    {
      if (pTrackDescription->selectionState.isSelected &&
          !pTrackDescription->selectionState.isMuted)
      {
        //Get download and current media positions
        bOk = GetDownloadAndCurrentMediaPos(pTrackDescription->majorType,
                                            downloadPos,
                                            currPos);
        if (bOk)
        {
          if (downloadPos > currPos)
          {
            buffProgress = (uint32)(downloadPos - currPos);
            dwldPos = downloadPos;
          }
        }
        else
        {
          QTV_MSG_PRIO2( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                         "GetDownloadAndCurrentMediaPos failed for track %d majorType %d",
                         trackID, majorType );
        }
      }
      else
      {
        QTV_MSG_PRIO4( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Track %d majorType %d not in a good state, isSelected/isMuted %d/%d",
                       trackID, majorType,
                       pTrackDescription->selectionState.isSelected,
                       pTrackDescription->selectionState.isMuted );
      }
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                    "NULL track description" );
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid track %d or media type %d",
                   trackID, majorType );
  }


  QTV_MSG_PRIO3( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                 "Buffering progress - %llu msec for track %d mediaType %d",
                 buffProgress, trackID, majorType );

  return bOk;
}


/* Compare to make sure the codecs are same and update the track description
 *
 * @param[in] mediaType - HTTP media type
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully compared and updated track
 * HTTPDL_WAITING - info unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus HTTPSourceMMITrackHandler::CompareAndUpdateTrack
(
 const HTTPMediaType mediaType
)
{
  QTV_NULL_PTR_CHECK(m_pDataInterface, HTTPCommon::HTTPDL_ERROR_ABORT);
  HTTPMediaTrackInfo trackInfo;
  std_memset((void*)&trackInfo, 0x0, sizeof(trackInfo));
  HTTPDownloadStatus eStatus = m_pDataInterface->GetSelectedMediaTrackInfo(mediaType, trackInfo);

  // make sure codec is the same
  if ( eStatus == HTTPCommon::HTTPDL_SUCCESS )
  {
    FileSourceMjMediaType majorType;
    HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType(trackInfo.majorType, majorType);
    FileSourceMnMediaType minorType;
    MapHTTPMinorTypeToFileSourceMinorType(trackInfo.minorType, minorType);
    TrackDescription trackDescription;
    eStatus = HTTPCommon::HTTPDL_WAITING;
    //ToDo: Add format block check for audio and declare codec mismatch if comp fails!
    (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
    for ( int index = 0; index < m_trackList.size(); index++ )
    {
      (void)m_trackList.get(index, trackDescription);
      if( trackDescription.majorType == majorType )
      {
        if((trackDescription.minorType != minorType) ||
            ((majorType == FILE_SOURCE_MJ_TYPE_AUDIO)&&
             (trackDescription.numChannels != trackInfo.audioTrackInfo.numChannels ||
              trackDescription.samplingRate != trackInfo.audioTrackInfo.samplingRate ||
              trackDescription.bitRate != trackInfo.audioTrackInfo.bitRate)))
        {
          // codec / number of audio channels / sampling rate is different!
          // Abort since we don't support codec change in between playback.
          // For audio track even if codec is same, if the channels/sampling rate is different, abort since we dont support audio switch.
          QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Codec change seen in between playback for track %d - old minortype/new minortype %d/%d",
                         trackDescription.trackID, trackDescription.minorType, minorType );
          eStatus = HTTPCommon::HTTPDL_CODEC_INFO_CHANGED;
        }
        else
        {
          eStatus = HTTPCommon::HTTPDL_SUCCESS;
        }
        break;
      }
    }
    if(eStatus == HTTPCommon::HTTPDL_WAITING)
    {
      //It means mediatype was not present in previous tracklist so this mediatype is newly
      //added so we need to send portconfig change event.
      eStatus = HTTPCommon::HTTPDL_CODEC_INFO_CHANGED;
    }
    (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
  }
  else if ((eStatus == HTTPCommon::HTTPDL_UNSUPPORTED)||
          (eStatus == HTTPCommon::HTTPDL_DATA_END))
  {
    FileSourceMjMediaType majorType;
    HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType(mediaType, majorType);

    TrackDescription trackDescription;
    (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
    // remove the track from the list with the same media type
    // this is true for failure too (bReturn == false)
    for ( DeepListIterator<TrackDescription> it = m_trackList; it.hasMore();
          it.advance() )
    {
      trackDescription = it.element();
      if ( (int)trackDescription.majorType == (int)majorType )
      {
        //Remove the trackDescription from the list
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Codec removed for track %d major/minor %d/%d",
                       trackDescription.trackID, trackDescription.majorType, trackDescription.minorType );
        it.remove();
        break;
      }
    }
    (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
  }

  // update the track description if track comparison was successful
  if ( eStatus == HTTPCommon::HTTPDL_SUCCESS ||
       eStatus == HTTPCommon::HTTPDL_CODEC_INFO_CHANGED)
  {
    uint32 nClipBitrate;
    if (!UpdateTrackDescription(trackInfo, false, nClipBitrate))
    {
      eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }

  return eStatus;
}

/* Reset Port Info
 *
 * return result of operation
 */
bool HTTPSourceMMITrackHandler::ResetPortInfo()
{
  bool bReturn = true;
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "clearing track List");
  m_trackList.clear();
  return bReturn;
}

/*
 * Updates the track description in HTTP MMI track handler
 *
 * @param[in] TrackInfo - The track information
 * @param[in] bRebuffering - indicates if the track is rebuffering
 * @param[out] nClipBitrate - adds the current track clip bit rate to nClipBitrate
 *
 * @return true on success the track description has been updated
 */
bool HTTPSourceMMITrackHandler::UpdateTrackDescription
(
  HTTPMediaTrackInfo &TrackInfo,
  bool bRebuffering,
  uint32 &nClipBitrate
)
{
  bool bReturn = true;
  TrackDescription trackDescription;

  //Time scale, layer ID and format block
  trackDescription.trackID = TrackInfo.nTrackID;
  trackDescription.playState = BUFFERING;
  trackDescription.reBuffering = bRebuffering;
  trackDescription.selectionState.isSelected = true;
  trackDescription.selectionState.isMuted = false;
  trackDescription.bEndOfTrack = false;

  HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType( TrackInfo.majorType,
                                                     trackDescription.majorType );
  MapHTTPMinorTypeToFileSourceMinorType(TrackInfo.minorType,
                                        trackDescription.minorType);

  trackDescription.layerID = 0;
  trackDescription.timeScale = 0;

  switch (TrackInfo.majorType)
  {
  case HTTPCommon::HTTP_VIDEO_TYPE:
    {
      QTV_MSG_PRIO7( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "Video track %u info: codec type %d, bitrate %u bps, duration %llu msec, "
        "height %u, width %u, framerate %f fps",
        TrackInfo.nTrackID,
        trackDescription.minorType,
        TrackInfo.videoTrackInfo.bitRate,
        TrackInfo.videoTrackInfo.duration,
        TrackInfo.videoTrackInfo.frameHeight,
        TrackInfo.videoTrackInfo.frameWidth,
        TrackInfo.videoTrackInfo.frameRate );
      trackDescription.layerID = TrackInfo.videoTrackInfo.layer;
      trackDescription.timeScale = TrackInfo.videoTrackInfo.timeScale;
      nClipBitrate += TrackInfo.videoTrackInfo.bitRate;
      trackDescription.duration = TrackInfo.videoTrackInfo.duration;
      trackDescription.maxSampleSize = TrackInfo.videoTrackInfo.maxSampleSize;
      trackDescription.frameHeight = TrackInfo.videoTrackInfo.frameHeight;
      trackDescription.frameWidth = TrackInfo.videoTrackInfo.frameWidth;
      HTTPDownloadStatus eStatus  = UpdateTrackDRMInfo(TrackInfo);
      trackDescription.isDRMProtected = TrackInfo.videoTrackInfo.drmInfo.isDRMProtected;
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
         "video track DRMProtected %d", trackDescription.isDRMProtected);
      if (TrackInfo.videoTrackInfo.drmInfo.isDRMProtected &&
          eStatus == HTTPCommon::HTTPDL_SUCCESS &&
          CheckDrmInfoUniqueness(trackDescription.majorType, TrackInfo.videoTrackInfo.drmInfo))
      {
        // updating DRM related info
        for (uint32 id = 0; id < MAX_PSSH_INFO_ENTRY ; id++)
        {
          if ((TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NONE ||
               TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NEW) &&
              (TrackDrmInfo[id].majorType == trackDescription.majorType ||
               TrackDrmInfo[id].majorType == FILE_SOURCE_MJ_TYPE_UNKNOWN))
          {
            bReturn = UpdateTrackDescDRMInfo(TrackDrmInfo[id],
                                           TrackInfo.nTrackID,
                                           TrackInfo.majorType,
                                           TrackInfo.videoTrackInfo.drmInfo);
           break;
          }
        }
      }
      DeleteTrackInfoDrmMem(TrackInfo.videoTrackInfo.drmInfo);
    }
    break;

  case HTTPCommon::HTTP_AUDIO_TYPE:
    {
       QTV_MSG_PRIO6( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
       "Audio track %u info: codec type %d,channel %d, bitrate %u bps, duration %llu msec,"
        "samplingrate %u sps",
        TrackInfo.nTrackID,
        trackDescription.minorType,
        TrackInfo.audioTrackInfo.numChannels,
        TrackInfo.audioTrackInfo.bitRate,
        TrackInfo.audioTrackInfo.duration,
        TrackInfo.audioTrackInfo.samplingRate );
      trackDescription.timeScale = TrackInfo.audioTrackInfo.timeScale;
      nClipBitrate += TrackInfo.audioTrackInfo.bitRate;
      trackDescription.duration = TrackInfo.audioTrackInfo.duration;
      trackDescription.maxSampleSize = TrackInfo.audioTrackInfo.maxSampleSize;
      trackDescription.numChannels = TrackInfo.audioTrackInfo.numChannels;
      trackDescription.samplingRate = TrackInfo.audioTrackInfo.samplingRate;
      HTTPDownloadStatus eStatus  = UpdateTrackDRMInfo(TrackInfo);
      trackDescription.isDRMProtected = TrackInfo.audioTrackInfo.drmInfo.isDRMProtected;
      trackDescription.bitRate = TrackInfo.audioTrackInfo.bitRate;
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
         "audio track DRMProtected %d", trackDescription.isDRMProtected);
      if (TrackInfo.audioTrackInfo.drmInfo.isDRMProtected &&
          eStatus == HTTPCommon::HTTPDL_SUCCESS  &&
          CheckDrmInfoUniqueness(trackDescription.majorType, TrackInfo.audioTrackInfo.drmInfo))
      {
        // updating DRM related info
        for (uint32 id = 0; id < MAX_PSSH_INFO_ENTRY ; id++)
        {
          if ((TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NONE ||
               TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NEW) &&
              (TrackDrmInfo[id].majorType == trackDescription.majorType ||
               TrackDrmInfo[id].majorType == FILE_SOURCE_MJ_TYPE_UNKNOWN))
          {
            bReturn = UpdateTrackDescDRMInfo(TrackDrmInfo[id],
                                             TrackInfo.nTrackID,
                                             TrackInfo.majorType,
                                             TrackInfo.audioTrackInfo.drmInfo);
            break;
          }
        }
      }
      DeleteTrackInfoDrmMem(TrackInfo.audioTrackInfo.drmInfo);
    }
    break;

  case HTTPCommon::HTTP_TEXT_TYPE:
    {
      QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "Text track %u info: codec type %d, duration %llu msec,"
         "height %u, width %u",
        TrackInfo.nTrackID,
        trackDescription.minorType,
        TrackInfo.audioTrackInfo.duration,
        TrackInfo.videoTrackInfo.frameHeight,
        TrackInfo.videoTrackInfo.frameWidth);
      trackDescription.layerID = TrackInfo.textTrackInfo.layer;
      trackDescription.timeScale = TrackInfo.textTrackInfo.timeScale;
      trackDescription.duration = TrackInfo.textTrackInfo.duration;
      trackDescription.maxSampleSize = TrackInfo.textTrackInfo.maxSampleSize;
      trackDescription.frameHeight = TrackInfo.textTrackInfo.nHeight;
      trackDescription.frameWidth = TrackInfo.textTrackInfo.nWidth;
      HTTPDownloadStatus eStatus  = UpdateTrackDRMInfo(TrackInfo);
      trackDescription.isDRMProtected = TrackInfo.textTrackInfo.drmInfo.isDRMProtected;
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
         "Text track DRMProtected %d", trackDescription.isDRMProtected);
      if (TrackInfo.textTrackInfo.drmInfo.isDRMProtected &&
          eStatus == HTTPCommon::HTTPDL_SUCCESS  &&
          CheckDrmInfoUniqueness(trackDescription.majorType, TrackInfo.textTrackInfo.drmInfo))
      {
        // updating DRM related info
        for (uint32 id = 0; id < MAX_PSSH_INFO_ENTRY ; id++)
        {
          if ((TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NONE ||
               TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NEW) &&
              (TrackDrmInfo[id].majorType == trackDescription.majorType ||
               TrackDrmInfo[id].majorType == FILE_SOURCE_MJ_TYPE_UNKNOWN))
          {
            bReturn = UpdateTrackDescDRMInfo(TrackDrmInfo[id],
                                             TrackInfo.nTrackID,
                                             TrackInfo.majorType,
                                             TrackInfo.textTrackInfo.drmInfo);
            break;
          }
        }
      }
      DeleteTrackInfoDrmMem(TrackInfo.textTrackInfo.drmInfo);
    }
    break;

  default:
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Unknown codec type %d", TrackInfo.majorType );
     bReturn = false;
     break;
  }

  //Get and cache config data
  if (bReturn)
  {
    uint32 size = m_nFormatBlockBufferSize;
    trackDescription.config.size = 0;
    if (GetConfigData(TrackInfo.majorType, (uint8*)&trackDescription.config.data, size) !=
        HTTPCommon::HTTPDL_SUCCESS)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Could not get config data for media type %d, proceeding...",
                     TrackInfo.majorType );
    }
    else
    {
      trackDescription.config.size = size;
    }
  }

  (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
  // remove the track from the list with the same media type
  // this is true for failure too (bReturn == false)
  for ( DeepListIterator<TrackDescription> it = m_trackList; it.hasMore();
        it.advance() )
  {
    if ( (int)it.element().majorType == (int)TrackInfo.majorType )
    {
      //Remove the trackDescription from the list
      it.remove();
      break;
    }
  }
  if ( bReturn )
  {
    m_trackList.append(trackDescription);
  }
  (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);


  return bReturn;
}

/*
 * Deleting memory of DrmInfo (if allocated) for each track
 *
 * @param[in/out] trackDrmInfo - Drm info for each track
 *
 * @return - none
 */
void HTTPSourceMMITrackHandler::DeleteTrackInfoDrmMem(HTTPDrmInfo &trackDrmInfo)
{
  if (trackDrmInfo.pKidDataBuf)
  {
    QTV_Delete(trackDrmInfo.pKidDataBuf);
    trackDrmInfo.pKidDataBuf = NULL;
  }
  if (trackDrmInfo.pPsshDataBuf)
  {
    QTV_Delete(trackDrmInfo.pPsshDataBuf);
    trackDrmInfo.pPsshDataBuf = NULL;
  }
  if (trackDrmInfo.pCpDataBuf)
  {
     QTV_Delete(trackDrmInfo.pCpDataBuf);
     trackDrmInfo.pCpDataBuf = NULL;
  }
  return;
}

/*
 * Initializing HTTPDrmInfo array
 *
 * @param[in/out] TrackDrmInfo - array of HTTPDrmInfo structure
 *
 * @return - none
 */
void HTTPSourceMMITrackHandler::InitializeTrackDescDrmInfo
(
  HTTPDrmInfo &TrackDrmInfo
)
{
  TrackDrmInfo.trackID = -1;
  TrackDrmInfo.nUniqueID = 0;
  TrackDrmInfo.majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
  TrackDrmInfo.ePsshInfoStatus = PSSH_INFO_NONE;
  TrackDrmInfo.isDRMProtected = false;
  TrackDrmInfo.eDrmType = FILE_SOURCE_NO_DRM ;
  TrackDrmInfo.systemIdBufSize = 0;
  TrackDrmInfo.kidCount = 0;
  TrackDrmInfo.kidDataBufSize = 0;
  TrackDrmInfo.psshDataBufSize = 0;
  TrackDrmInfo.defaultKeyIDSize = 0;
  TrackDrmInfo.cpBufSize = 0;
  TrackDrmInfo.systemID[0] = '\0';
  TrackDrmInfo.defaultKeyID[0] = '\0';
  TrackDrmInfo.pKidDataBuf = NULL;
  TrackDrmInfo.pPsshDataBuf = NULL;
  TrackDrmInfo.pCpDataBuf = NULL;

  return;
}

/*
 * Updating TrackDrmInfo in TrackDesc with retrieved values in trackInfoDrmInfo
 *
 * @param[in/out] TrackDrmInfo
 * @param[in] nTrackID
 * @param[in] majorType
 * @param[in] trackInfoDrmInfo
 * @return - true/false
 */
bool HTTPSourceMMITrackHandler::UpdateTrackDescDRMInfo
(
  HTTPDrmInfo &TrackDrmInfo,
  uint32 nTrackID,
  HTTPCommon::HTTPMediaType majorType,
  HTTPDrmInfo &trackInfoDrmInfo
)
{

  bool bReturn = true;
  DeleteTrackInfoDrmMem(TrackDrmInfo);
  TrackDrmInfo.trackID = nTrackID;
  TrackDrmInfo.isDRMProtected = trackInfoDrmInfo.isDRMProtected;
  if (trackInfoDrmInfo.isDRMProtected)
  {
    bReturn = false;
    TrackDrmInfo.nUniqueID = GenerateUniqueID();
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "TrackDrmInfo.nUniqueID %d", TrackDrmInfo.nUniqueID);
    HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType(majorType,
                                                      TrackDrmInfo.majorType );
    //TrackDrmInfo.isDRMProtected = trackInfoDrmInfo.isDRMProtected;
    TrackDrmInfo.eDrmType       = trackInfoDrmInfo.eDrmType;
    TrackDrmInfo.systemIdBufSize  = trackInfoDrmInfo.systemIdBufSize;
    TrackDrmInfo.kidCount       = trackInfoDrmInfo.kidCount;
    TrackDrmInfo.kidDataBufSize = trackInfoDrmInfo.kidDataBufSize;
    TrackDrmInfo.psshDataBufSize = trackInfoDrmInfo.psshDataBufSize;
    TrackDrmInfo.defaultKeyIDSize = trackInfoDrmInfo.defaultKeyIDSize;
    TrackDrmInfo.cpBufSize        = trackInfoDrmInfo.cpBufSize;
    TrackDrmInfo.ePsshInfoStatus = PSSH_INFO_NEW;



    if (TrackDrmInfo.systemIdBufSize > 0)
    {
      memcpy(TrackDrmInfo.systemID, trackInfoDrmInfo.systemID,
             MAX_SYSTEMID_SIZE);
      bReturn = true;
    }

    if (TrackDrmInfo.kidDataBufSize > 0)
    {
      //allocating memory
      TrackDrmInfo.pKidDataBuf =
               (char *)QTV_Malloc(TrackDrmInfo.kidDataBufSize + 1);
      if (TrackDrmInfo.pKidDataBuf)
      {
        memset(TrackDrmInfo.pKidDataBuf, 0, TrackDrmInfo.kidDataBufSize + 1);
        memcpy(TrackDrmInfo.pKidDataBuf, trackInfoDrmInfo.pKidDataBuf,
               TrackDrmInfo.kidDataBufSize);
        bReturn = true;
      }
    }
    if (TrackDrmInfo.psshDataBufSize > 0)
    {
      TrackDrmInfo.pPsshDataBuf =
         (char *)QTV_Malloc(TrackDrmInfo.psshDataBufSize + 1);
      if (TrackDrmInfo.pPsshDataBuf)
      {
        memset(TrackDrmInfo.pPsshDataBuf, 0, TrackDrmInfo.psshDataBufSize + 1);
        memcpy(TrackDrmInfo.pPsshDataBuf, trackInfoDrmInfo.pPsshDataBuf,
              TrackDrmInfo.psshDataBufSize);
        bReturn = true;
      }
    }


    if (TrackDrmInfo.defaultKeyIDSize > 0)
    {
      bReturn = true;
      memcpy(TrackDrmInfo.defaultKeyID, trackInfoDrmInfo.defaultKeyID,
               TrackDrmInfo.defaultKeyIDSize);
    }

    if (TrackDrmInfo.cpBufSize > 0)
    {
      TrackDrmInfo.pCpDataBuf =
         (char *)QTV_Malloc(TrackDrmInfo.cpBufSize);
      if(TrackDrmInfo.pCpDataBuf)
      {
        memcpy(TrackDrmInfo.pCpDataBuf, trackInfoDrmInfo.pCpDataBuf,
               TrackDrmInfo.cpBufSize);
      }
    }

    PrintTrackDRMInfo();
  }
  return bReturn;
}


/*
 * Check for same DRM info already exists
 *
 * @param[in] majorType
 *
 * @param[in] trackInfoDrmInfo (already retrieved from file source)
 *
 * @return - bool
 */


bool HTTPSourceMMITrackHandler::CheckDrmInfoUniqueness(FileSourceMjMediaType majorType,
                              HTTPDrmInfo trackInfoDrmInfo)
{
  bool bGenerateUniqueId = true;

  PrintTrackDRMInfo();

  for (uint32 id = 0; id < MAX_PSSH_INFO_ENTRY ; id++)
  {
    if (majorType == TrackDrmInfo[id].majorType)
    {
      if ((TrackDrmInfo[id].defaultKeyID && trackInfoDrmInfo.defaultKeyID) &&
          (TrackDrmInfo[id].pPsshDataBuf && trackInfoDrmInfo.pPsshDataBuf) &&
          (TrackDrmInfo[id].pCpDataBuf && trackInfoDrmInfo.pCpDataBuf))
      {
        if (0 == memcmp(TrackDrmInfo[id].defaultKeyID, trackInfoDrmInfo.defaultKeyID, MAX_KID_SIZE) &&
            0 == memcmp(TrackDrmInfo[id].pPsshDataBuf, trackInfoDrmInfo.pPsshDataBuf, TrackDrmInfo[id].psshDataBufSize) &&
            0 == memcmp(TrackDrmInfo[id].pCpDataBuf, trackInfoDrmInfo.pCpDataBuf, TrackDrmInfo[id].cpBufSize))
        {
          bGenerateUniqueId = false;
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "media type [%d]:Same DrmInfo present, no UniqueID generation",
                 TrackDrmInfo[id].majorType);
          break;
        }
      }
    }
  }
  return bGenerateUniqueId;
}


/*
 * Updating TrackInfo with DRM info to be retrived from HTTPResourre
 *
 * @param[in/out] TrackInfo
 *
 * @return - HTTPDownloadStatus
 */

HTTPDownloadStatus HTTPSourceMMITrackHandler::UpdateTrackDRMInfo
(
  HTTPMediaTrackInfo &TrackInfo
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(m_pDataInterface, eStatus);
  OMX_U32 portIdx = 0;
  MapMediaTypeToPortID(portIdx, TrackInfo.majorType);
  switch (TrackInfo.majorType)
  {
    case HTTPCommon::HTTP_AUDIO_TYPE:
    {
      TrackInfo.audioTrackInfo.drmInfo.pKidDataBuf = NULL;
      TrackInfo.audioTrackInfo.drmInfo.pPsshDataBuf = NULL;
      if (TrackInfo.audioTrackInfo.drmInfo.kidDataBufSize > 0)
      {
        TrackInfo.audioTrackInfo.drmInfo.pKidDataBuf =
            (char *)QTV_Malloc(TrackInfo.audioTrackInfo.drmInfo.kidDataBufSize + 1);
      }
      if (TrackInfo.audioTrackInfo.drmInfo.psshDataBufSize > 0)
      {
        TrackInfo.audioTrackInfo.drmInfo.pPsshDataBuf =
           (char *)QTV_Malloc(TrackInfo.audioTrackInfo.drmInfo.psshDataBufSize + 1);
      }

      eStatus = m_pDataInterface->GetSelectedMediaTrackInfo(TrackInfo.majorType, TrackInfo);

      if (TrackInfo.audioTrackInfo.drmInfo.cpBufSize == 0 )
      {
        QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Query for QOMX_HTTP_IndexParamContentProtectionInfo : size");
        eStatus = m_pDataInterface->GetContentProtectElem(
                    portIdx,
                    TrackInfo.majorType,
                    (HTTPDrmType&)TrackInfo.audioTrackInfo.drmInfo.eDrmType,
                    TrackInfo.audioTrackInfo.drmInfo.streamType,
                    TrackInfo.audioTrackInfo.drmInfo.cpBufSize,
                    (unsigned char *)TrackInfo.audioTrackInfo.drmInfo.pCpDataBuf);

        if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
        {
          if (TrackInfo.audioTrackInfo.drmInfo.cpBufSize > 0)
          {
            TrackInfo.audioTrackInfo.drmInfo.cpBufSize += 1;
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Query for QOMX_HTTP_IndexParamContentProtectionInfo : size : Success");
          }
          else
          {
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
              "Retreived contentProtection Size is zero");
          }
        }
      }
      if (TrackInfo.audioTrackInfo.drmInfo.cpBufSize > 0 )
      {
        QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Query for QOMX_HTTP_IndexParamContentProtectionInfo : data");
        TrackInfo.audioTrackInfo.drmInfo.pCpDataBuf =
           (char *)QTV_Malloc(TrackInfo.audioTrackInfo.drmInfo.cpBufSize);
        eStatus = m_pDataInterface->GetContentProtectElem(
                    portIdx,
                    TrackInfo.majorType,
                    (HTTPDrmType&)TrackInfo.audioTrackInfo.drmInfo.eDrmType,
                    TrackInfo.audioTrackInfo.drmInfo.streamType,
                    TrackInfo.audioTrackInfo.drmInfo.cpBufSize,
                    (unsigned char *)TrackInfo.audioTrackInfo.drmInfo.pCpDataBuf);
        if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
        {
          QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
            "Query for QOMX_HTTP_IndexParamContentProtectionInfo : data : Success");
        }
      }
    }
    break;
    case HTTPCommon::HTTP_VIDEO_TYPE:
    {
      TrackInfo.videoTrackInfo.drmInfo.pKidDataBuf = NULL;
      TrackInfo.videoTrackInfo.drmInfo.pPsshDataBuf = NULL;
      if (TrackInfo.videoTrackInfo.drmInfo.kidDataBufSize > 0)
      {
        TrackInfo.videoTrackInfo.drmInfo.pKidDataBuf =
            (char *)QTV_Malloc(TrackInfo.videoTrackInfo.drmInfo.kidDataBufSize + 1);
      }
      if (TrackInfo.videoTrackInfo.drmInfo.psshDataBufSize > 0)
      {
        TrackInfo.videoTrackInfo.drmInfo.pPsshDataBuf =
           (char *)QTV_Malloc(TrackInfo.videoTrackInfo.drmInfo.psshDataBufSize + 1);
      }

      eStatus = m_pDataInterface->GetSelectedMediaTrackInfo(TrackInfo.majorType, TrackInfo);

      if (TrackInfo.videoTrackInfo.drmInfo.cpBufSize == 0 )
      {
        QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Query for QOMX_HTTP_IndexParamContentProtectionInfo : size");
        eStatus = m_pDataInterface->GetContentProtectElem(
                    portIdx,
                    TrackInfo.majorType,
                    (HTTPDrmType&)TrackInfo.videoTrackInfo.drmInfo.eDrmType,
                    TrackInfo.videoTrackInfo.drmInfo.streamType,
                    TrackInfo.videoTrackInfo.drmInfo.cpBufSize,
                    (unsigned char *)TrackInfo.videoTrackInfo.drmInfo.pCpDataBuf);

        if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
        {
      if (TrackInfo.videoTrackInfo.drmInfo.cpBufSize > 0 )
      {
            TrackInfo.videoTrackInfo.drmInfo.cpBufSize += 1;
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Query for QOMX_HTTP_IndexParamContentProtectionInfo : size : Success");
          }
          else
          {
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
              "Retreived contentProtection Size is zero");
          }
        }
      }
      if (TrackInfo.videoTrackInfo.drmInfo.cpBufSize > 0)
      {
        QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Query for QOMX_HTTP_IndexParamContentProtectionInfo : data");
        TrackInfo.videoTrackInfo.drmInfo.pCpDataBuf =
           (char *)QTV_Malloc(TrackInfo.videoTrackInfo.drmInfo.cpBufSize);
        eStatus = m_pDataInterface->GetContentProtectElem(
                    portIdx,
                    TrackInfo.majorType,
                    (HTTPDrmType&)TrackInfo.videoTrackInfo.drmInfo.eDrmType,
                    TrackInfo.videoTrackInfo.drmInfo.streamType,
                    TrackInfo.videoTrackInfo.drmInfo.cpBufSize,
                    (unsigned char *)TrackInfo.videoTrackInfo.drmInfo.pCpDataBuf);
        if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
        {
          QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
            "Query for QOMX_HTTP_IndexParamContentProtectionInfo : data : Success");
        }
      }
    }
    break;
    case HTTPCommon::HTTP_TEXT_TYPE:
    {
      TrackInfo.textTrackInfo.drmInfo.pKidDataBuf = NULL;
      TrackInfo.textTrackInfo.drmInfo.pPsshDataBuf = NULL;
      if (TrackInfo.textTrackInfo.drmInfo.kidDataBufSize > 0)
      {
        TrackInfo.textTrackInfo.drmInfo.pKidDataBuf =
            (char *)QTV_Malloc(TrackInfo.textTrackInfo.drmInfo.kidDataBufSize + 1);
      }
      if (TrackInfo.textTrackInfo.drmInfo.psshDataBufSize > 0)
      {
        TrackInfo.textTrackInfo.drmInfo.pPsshDataBuf =
           (char *)QTV_Malloc(TrackInfo.textTrackInfo.drmInfo.psshDataBufSize + 1);
      }
      eStatus = m_pDataInterface->GetSelectedMediaTrackInfo(TrackInfo.majorType, TrackInfo);

      if (TrackInfo.textTrackInfo.drmInfo.cpBufSize == 0 )
      {
        QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Query for QOMX_HTTP_IndexParamContentProtectionInfo : size");
        eStatus = m_pDataInterface->GetContentProtectElem(
                    portIdx,
                    TrackInfo.majorType,
                    (HTTPDrmType&)TrackInfo.textTrackInfo.drmInfo.eDrmType,
                    TrackInfo.textTrackInfo.drmInfo.streamType,
                    TrackInfo.textTrackInfo.drmInfo.cpBufSize,
                    (unsigned char *)TrackInfo.textTrackInfo.drmInfo.pCpDataBuf);

        if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
        {
          if (TrackInfo.textTrackInfo.drmInfo.cpBufSize > 0)
          {
            TrackInfo.textTrackInfo.drmInfo.cpBufSize += 1;
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Query for QOMX_HTTP_IndexParamContentProtectionInfo : size : Success");
          }
          else
          {
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
              "Retreived contentProtection Size is zero");
          }
        }
      }
      if (TrackInfo.textTrackInfo.drmInfo.cpBufSize > 0)
      {
        QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
             "Query for QOMX_HTTP_IndexParamContentProtectionInfo : data");
        TrackInfo.textTrackInfo.drmInfo.pCpDataBuf =
               (char *)QTV_Malloc(TrackInfo.textTrackInfo.drmInfo.cpBufSize);
        eStatus = m_pDataInterface->GetContentProtectElem(
                    portIdx,
                    TrackInfo.majorType,
                    (HTTPDrmType&)TrackInfo.textTrackInfo.drmInfo.eDrmType,
                    TrackInfo.textTrackInfo.drmInfo.streamType,
                    TrackInfo.textTrackInfo.drmInfo.cpBufSize,
                   (unsigned char *)TrackInfo.textTrackInfo.drmInfo.pCpDataBuf);
        if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
        {
          QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
            "Query for QOMX_HTTP_IndexParamContentProtectionInfo : data : Success");
        }
      }
    }
    break;
    default:
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Unknown codec type %d", TrackInfo.majorType );
    break;
  }
  return eStatus;
}





/*
 * Retrieving PSSH info
 *
 */

HTTPDownloadStatus HTTPSourceMMITrackHandler::GetPsshInfo
(
  uint32 portIndex,
  int &nUniqueID,
  unsigned char *cDefaultKeyID,
  uint32 &nPsshDataBufSize,
  unsigned char *cPSSHData,
  bool isQueryForSize
)
{
  HTTPDownloadStatus eStatus =  HTTPCommon::HTTPDL_ERROR_ABORT;
  FileSourceMjMediaType majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;

  HTTPMediaType mediaType;
  MapPortIDToMediaType(portIndex,mediaType);
  HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType(mediaType,majorType);
  (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
  QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "Queried using nUniqueID %d portIndex %u mediaType %d majorType %d",
               nUniqueID, portIndex,mediaType, majorType );


  PrintTrackDRMInfo();


  for (int id = 0; id < MAX_PSSH_INFO_ENTRY; id++)
  {
    if (true == isQueryForSize)
    {
      if (TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NEW &&
          TrackDrmInfo[id].majorType == majorType)
      {
        if (nUniqueID > 0 && nUniqueID == TrackDrmInfo[id].nUniqueID)
        {
          eStatus =  HTTPCommon::HTTPDL_SUCCESS;
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "Queried during playback..\
                 found uniqueID %d at index %d ",nUniqueID, id );
          TrackDrmInfo[id].ePsshInfoStatus = PSSH_INFO_QUERIED;
          nPsshDataBufSize = TrackDrmInfo[id].psshDataBufSize + 1;
        }
        else if (nUniqueID == 0)
        {
          eStatus =  HTTPCommon::HTTPDL_SUCCESS;
          nUniqueID = TrackDrmInfo[id].nUniqueID;
          if (TrackDrmInfo[id].psshDataBufSize > 0)
          {
          nPsshDataBufSize = TrackDrmInfo[id].psshDataBufSize + 1;
          }
          TrackDrmInfo[id].ePsshInfoStatus = PSSH_INFO_QUERIED;
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "uniqueID %d Queried before playback", nUniqueID);
        }
        else
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
              "Looking for uniqueID...");
          continue;
        }
        break;
      }
    }
    else
    {

      if (TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_QUERIED &&
           nUniqueID == TrackDrmInfo[id].nUniqueID &&
           TrackDrmInfo[id].majorType == majorType)
      {
        eStatus =  HTTPCommon::HTTPDL_SUCCESS;
        memset(cDefaultKeyID, 0, MAX_KID_SIZE);
        (void)memcpy(cDefaultKeyID,
                         TrackDrmInfo[id].defaultKeyID,
                         MAX_KID_SIZE);

        if (nPsshDataBufSize > 0 && TrackDrmInfo[id].pPsshDataBuf)
        {
          (void)memcpy(cPSSHData,
                       TrackDrmInfo[id].pPsshDataBuf,
                       nPsshDataBufSize);
        }
        SetLastPsshUniqueIdQueried(majorType,nUniqueID );
        TrackDrmInfo[id].ePsshInfoStatus = PSSH_INFO_NONE;
        break;
      }
    }
  }
  if (eStatus ==  HTTPCommon::HTTPDL_ERROR_ABORT)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "Error:nUniqueID %d Not found",nUniqueID);
  }
  PrintTrackDRMInfo();

  (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);

  return eStatus;
}

/*
 *  Caches the essential details of tracks the HTTP MMI track handler
 *
 */
void HTTPSourceMMITrackHandler::ProcessGetTracksStatus
(
)
{
  uint32 clipBitrate = 0;
  FileSourceMjMediaType majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
  FileSourceMnMediaType minorType = FILE_SOURCE_MN_TYPE_UNKNOWN;

  if ( m_pHTTPController == NULL )
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid HTTPController object" );
    return;
  }

  if ( m_pDataInterface == NULL )
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "Neither of the "
                  "interfaces DataResourceManager and DataInterface exist" );
    return;
  }

  m_trackList.clear();


  //Get the number of tracks first, allocate needed memory for track list
  //and then get the track list info
  HTTPMediaTrackInfo* pTrackInfo = NULL;
  uint32 numTracks = m_pDataInterface->GetMediaTrackInfo(pTrackInfo);
  pTrackInfo = (HTTPMediaTrackInfo *) (QTV_Malloc(numTracks *
                                                  sizeof(HTTPMediaTrackInfo)));
  if (pTrackInfo)
  {
    numTracks = m_pDataInterface->GetMediaTrackInfo(pTrackInfo);
    for (uint32 index = 0; index < numTracks; index++)
    {
      //Populate track description - move onto the next track for any failure
      //because that's better than populating incorrect information!!
      if (pTrackInfo + index == NULL)
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Error: Track info for track %d is NULL", index );
        continue;
      }
      else
      {
        // copy only selected tracks, the support for track selection is not
        // there in HTTP MMI, needs to be added based on
        // OMX_IndexParamActiveStream and OMX_IndexParamNumAvailableStreams
        if ( pTrackInfo[index].bSelected )
        {
          if ( UpdateTrackDescription( pTrackInfo[index], false,
                                       clipBitrate ) == false)
          {
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "unable to copy the track details for track %u",
                           pTrackInfo[index].nTrackID );
            continue;
          }
        }
      }
    }
    QTV_Free(pTrackInfo);
    pTrackInfo = NULL;
  }
  return;
}

/** @brief Convert the HTTP status code to mmi status code for
* SET_TRACK_STATE command.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pResultRecipient - Reference to the registered mmi callback data
*/
void HTTPSourceMMITrackHandler::ProcessSetTrackStateStatus
(
 const HTTPDownloadStatus /*HTTPStatus*/,
 void* /*pResultRecipient*/
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "MMI notification suppressed" );
  return;
}

/**
* @brief Sets the Seek Pending flag. If this is set to true, then
* 'Read' calls will return underrun.
*
* @param[in] value - Seek pending flag
*/
void HTTPSourceMMITrackHandler::SetSeekPending
(
 bool value
 )
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPSourceMMITrackHandler::SetSeekPending - value '%d'",
                 value );

  (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
  m_bIsSeekPending = value;
  (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
}
bool HTTPSourceMMITrackHandler::IsSeekPending()
{
  bool bSeekPending = false;
  (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
  bSeekPending = m_bIsSeekPending;
  (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
  return bSeekPending;
}
/**
* @brief Sets the absolute seek offset.
*
* @param[in] timeToSeek - Absolute seek offset
*/
void HTTPSourceMMITrackHandler::SetAbsoluteSeekOffset
(
 const int64 timeToSeek
 )
{
  if (timeToSeek >= 0)
  {
    //This is called when a SEEK is successful, store the seek time
    //and also reset the EOS flag as the stack could be downloading
    //the new data
    (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
    m_nAbsoluteSeekOffset = timeToSeek;
    m_bEOS = false;
    TrackDescription* pTrackDescription = NULL;
    for (ListPair<TrackDescription>* it = m_trackList.iterator();
         it != NULL; it = it->tail())
    {
      pTrackDescription = &it->head();
      if(pTrackDescription)
      {
        pTrackDescription->bEndOfTrack = false;
      }
    }
    (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);

    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "HTTPSourceMMITrackHandler::SetAbsoluteSeekOffset - offset '%lld'",
                   timeToSeek );
  }
}

/** @brief Check if specified track is playable.
*
* @param[in] trackIdentifier - Track ID
* @param[in] majorType - HTTP media major type
*
* @return
* HTTPDL_SUCCESS - Track playable
* HTTPDL_WAITING - Data underrun
* HTTPDL_ERROR_ABORT - Otherwise
*/
HTTPDownloadStatus HTTPSourceMMITrackHandler::CanPlayTrack
(
  const int32 trackIdentifier,
  HTTPMediaType majorType
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMITrackHandler::CanPlayTrack" );
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
  if ( !m_pHTTPController)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: m_pHTTPController is NULL" );
    return result;
  }

  //Get the track in question
  TrackDescription* pTrackDescription = NULL;
  if (!FindTrack(trackIdentifier, majorType, &pTrackDescription) ||
      !pTrackDescription)
  {
    return result;
  }

  //Check for download completion, no need to check for buffered
  //duration if yes
  TrackPlayState nextPlayState = UNKNOWN;

  if(IsSeekPending())
  {
    nextPlayState = BUFFERING;
    result = HTTPCommon::HTTPDL_WAITING;
  }
  else if (IsDownloadComplete(majorType))
  {
    result = HTTPCommon::HTTPDL_SUCCESS;
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Download complete - no need to check for buffered duration, "
                  "buffering ended" );
    nextPlayState = PLAYING;
  }
  else
  {
    //Check if seek is in progress, if so return underrun
    result = HTTPCommon::HTTPDL_WAITING;

    switch (pTrackDescription->playState)
    {
      case BUFFERING:
      {
        //Compute the buffered duration relative to current media position
        //and preroll worth of data must be accumulated to come out of
        //BUFFERING state. Return underrun even if GetDownloadAndCurrentMediaPos()
        //returns failure (could be because file source status is error), that
        //is the source filter behavior anyways!!
        uint64 downloadPos = 0, currPos = 0;
        uint64 preroll;

        // get the preroll to be used
        if ( pTrackDescription->reBuffering )
        {
          preroll = (uint64)m_pHTTPController->GetRebufferPreroll();
        }
        else
        {
          preroll = (uint64)m_pHTTPController->GetInitialPreroll();
        }

        (void)GetDownloadAndCurrentMediaPos(pTrackDescription->majorType,
                                            downloadPos,
                                            currPos);
        uint64 buffDuration = downloadPos - currPos;
        QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "media type %d Track %d: Buffering data - "
                       "available data (msec) %llu, preroll (msec) %llu",
                       majorType, pTrackDescription->trackID,
                       buffDuration, preroll );
        if (buffDuration >= preroll)
        {
          result = HTTPCommon::HTTPDL_SUCCESS;
          nextPlayState = PLAYING;
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Buffering ended for media type %d track %d",
                         majorType, pTrackDescription->trackID );
        }
      }
      break;

      case PLAYING:
      {
        result = HTTPCommon::HTTPDL_SUCCESS;
      }
      break;

      default:
      {
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Bad play state for media type %d track %d",
                       majorType, pTrackDescription->trackID );
        result = HTTPCommon::HTTPDL_ERROR_ABORT;
      }
      break;
    }
  }

  if (nextPlayState != UNKNOWN &&
      nextPlayState != pTrackDescription->playState)
  {
    (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
    pTrackDescription->playState = nextPlayState;
    (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
  }

  return result;
}

/**
* @brief Get the (max) download and current media positions.
*
* @param[in] trackID - Track ID
* @param[out] downloadPos - Download position (msec)
* @param[out] currPos - Current media position (msec)
*
* @return
* TRUE - Download and current media positions obtained successfully
* FALSE - Otherwise
*/
bool HTTPSourceMMITrackHandler::GetDownloadAndCurrentMediaPos
(
 FileSourceMjMediaType majorType,
 uint64& downloadPos,
 uint64& currPos
 )
{
  bool rslt = false;
  HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;

  HTTPCommon::MapFileSourceMajorTypeToHTTPMediaType(majorType,mediaType);

  if (m_pDataInterface)
  {
    uint64 n64CurrentPos;
    uint64 nBuffDuration;
    if (m_pDataInterface->GetDurationBuffered(mediaType, n64CurrentPos,
                                              nBuffDuration) == false)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                     "GetDurationBuffered() - failed for media type %d",
                     mediaType );
    }
    else
    {
      currPos = (uint32)n64CurrentPos;
      downloadPos = currPos + nBuffDuration;
      rslt = true;
    }
  }
  return rslt;
}

/** @brief Convert the File Source major type to major type.
*
* @param[in] portIdx - port index
* @param[in] streamNo - stream number
* @param[out] trk - Reference to track ID
* @param[out] majorType - Reference to HTTP media major type
* @return success status
*/
bool HTTPSourceMMITrackHandler::MapStreamNumberToTrackID
(
  OMX_U32 portIdx,
  OMX_U32 streamNo,
  int32 &trk,
  HTTPMediaType &majorType
)
{
  int audNo = 0;
  int vidNo = 0;
  int otherNo = 0;
  bool bRet = false;

  if (m_pHTTPController && m_pHTTPController->IsHTTPStreamerRunning())
  {
    //Loop through the track list and update the track state if found
    for (ListPair<TrackDescription>* it = m_trackList.iterator();
         it != NULL; it = it->tail())
    {
      TrackDescription& trackDescription = it->head();
      if ((portIdx == MMI_HTTP_AUDIO_PORT_INDEX) &&
          (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_AUDIO))
      {
        if(streamNo == (unsigned int)audNo)
        {
          trk = trackDescription.trackID;
          majorType = HTTPCommon::HTTP_AUDIO_TYPE;
          bRet = true;
          break;
        }
        audNo++;
      }
    }
        //Loop through the track list and update the track state if found
    for (ListPair<TrackDescription>* it = m_trackList.iterator();
         it != NULL; it = it->tail())
    {
      TrackDescription& trackDescription = it->head();
      if ((portIdx == MMI_HTTP_VIDEO_PORT_INDEX) &&
          (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_VIDEO))
      {
        if(streamNo == (unsigned int)vidNo)
        {
          trk = trackDescription.trackID;
          majorType = HTTPCommon::HTTP_VIDEO_TYPE;
          bRet = true;
          break;
        }
        vidNo++;
      }
    }

    for (ListPair<TrackDescription>* it = m_trackList.iterator();
         it != NULL; it = it->tail())
    {
      TrackDescription& trackDescription = it->head();
      if ((portIdx == MMI_HTTP_OTHER_PORT_INDEX) &&
          (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_TEXT))
      {
        if(streamNo == (unsigned int)otherNo)
        {
          trk = trackDescription.trackID;
          majorType = HTTPCommon::HTTP_TEXT_TYPE;
          bRet = true;
          break;
        }
        vidNo++;
      }
    }
  }
  return bRet;
}

/** @brief Convert the File Source minor type to minor type.
*
* @param[in] minorType - File Source minor type
* @param[out] pMimeMinorType - Reference to minor type
*/
OMX_AUDIO_CODINGTYPE HTTPSourceMMITrackHandler::GetOmxAudioMinorType
(
 const FileSourceMnMediaType minorType
 )
{
  OMX_AUDIO_CODINGTYPE audioCoding = OMX_AUDIO_CodingUnused;

  switch (minorType)
  {
    case FILE_SOURCE_MN_TYPE_EVRC:
    case FILE_SOURCE_MN_TYPE_CONC_EVRC:
    case FILE_SOURCE_MN_TYPE_QCP:
      audioCoding = OMX_AUDIO_CodingEVRC;
      break;
#ifdef FEATURE_HTTP_EVRC
    case FILE_SOURCE_MN_TYPE_EVRC_B:
      audioCoding = (OMX_AUDIO_CODINGTYPE) QOMX_AUDIO_CodingEVRCB;
      break;
    case FILE_SOURCE_MN_TYPE_EVRC_WB:
      audioCoding = (OMX_AUDIO_CODINGTYPE) QOMX_AUDIO_CodingEVRCWB;
      break;
#endif
    case FILE_SOURCE_MN_TYPE_QCELP:
    case FILE_SOURCE_MN_TYPE_CONC_QCELP:
      audioCoding = OMX_AUDIO_CodingQCELP13;
      break;
    case FILE_SOURCE_MN_TYPE_AAC:
    case FILE_SOURCE_MN_TYPE_AAC_ADTS:
    case FILE_SOURCE_MN_TYPE_AAC_ADIF:
    case FILE_SOURCE_MN_TYPE_AAC_LOAS:
    case FILE_SOURCE_MN_TYPE_NONMP4_AAC:
    case FILE_SOURCE_MN_TYPE_CONC_AAC:
      audioCoding = OMX_AUDIO_CodingAAC;
      break;
    case FILE_SOURCE_MN_TYPE_AC3:
    case FILE_SOURCE_MN_TYPE_EAC3:
      audioCoding = (OMX_AUDIO_CODINGTYPE) QOMX_EXT_AUDIO_CodingAC3;
      break;
    case FILE_SOURCE_MN_TYPE_MP2:
      audioCoding = (OMX_AUDIO_CODINGTYPE) QOMX_EXT_AUDIO_CodingMP2;
      break;
#ifdef FEATURE_HTTP_AMR
    case FILE_SOURCE_MN_TYPE_GSM_AMR:
    case FILE_SOURCE_MN_TYPE_NONMP4_AMR:
    case FILE_SOURCE_MN_TYPE_CONC_AMR:
    case FILE_SOURCE_MN_TYPE_AMR_WB:
    case FILE_SOURCE_MN_TYPE_AMR_WB_PLUS:
      audioCoding = OMX_AUDIO_CodingAMR;
      break;
#endif // FEATURE_HTTP_AMR
    case FILE_SOURCE_MN_TYPE_MP3:
    case FILE_SOURCE_MN_TYPE_NONMP4_MP3:
      audioCoding = OMX_AUDIO_CodingMP3;
      break;
#ifdef FEATURE_HTTP_WM
    case FILE_SOURCE_MN_TYPE_WMA:
    case FILE_SOURCE_MN_TYPE_WMA_PRO:
    case FILE_SOURCE_MN_TYPE_WMA_LOSSLESS:
      audioCoding = OMX_AUDIO_CodingWMA;
      break;
#endif
    case FILE_SOURCE_MN_TYPE_MIDI:
      audioCoding = OMX_AUDIO_CodingMIDI;
      break;
    case FILE_SOURCE_MN_TYPE_PCM:
      audioCoding = OMX_AUDIO_CodingPCM;
      break;
    case FILE_SOURCE_MN_TYPE_G711_ALAW:
    case FILE_SOURCE_MN_TYPE_G711_MULAW:
      audioCoding = OMX_AUDIO_CodingG711;
      break;
    default:
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error: Unknown minor type %d", minorType );
      break;
  }
  return audioCoding;
}

/** @brief Convert the File Source minor type to minor type.
*
* @param[in] minorType - File Source minor type
* @param[out] pMimeMinorType - Reference to minor type
*/
OMX_VIDEO_CODINGTYPE HTTPSourceMMITrackHandler::GetOmxVideoMinorType
(
 const FileSourceMnMediaType minorType
 )
{
  OMX_VIDEO_CODINGTYPE videoCoding = OMX_VIDEO_CodingUnused;

  switch (minorType)
  {
    case FILE_SOURCE_MN_TYPE_MPEG4:
      videoCoding = OMX_VIDEO_CodingMPEG4;
      break;
    case FILE_SOURCE_MN_TYPE_H263:
      videoCoding = OMX_VIDEO_CodingH263;
      break;
    case FILE_SOURCE_MN_TYPE_H264:
      videoCoding = OMX_VIDEO_CodingAVC;
      break;
  case FILE_SOURCE_MN_TYPE_HEVC:
      videoCoding = (OMX_VIDEO_CODINGTYPE)QOMX_OTHER_CodingHevc;
      break;
#ifdef FEATURE_HTTP_WM
    case FILE_SOURCE_MN_TYPE_WMV1:
    case FILE_SOURCE_MN_TYPE_WMV2:
    case FILE_SOURCE_MN_TYPE_WMV3:
      videoCoding = OMX_VIDEO_CodingWMV;
      break;
#endif
    case FILE_SOURCE_MN_TYPE_RV30:
    case FILE_SOURCE_MN_TYPE_RV40:
      videoCoding = OMX_VIDEO_CodingRV;
      break;
    case FILE_SOURCE_MN_TYPE_MPEG2:
      videoCoding = OMX_VIDEO_CodingMPEG2;
      break;
    case FILE_SOURCE_MN_TYPE_VC1:
      videoCoding = (OMX_VIDEO_CODINGTYPE) QOMX_VIDEO_CodingVC1;
      break;
    case FILE_SOURCE_MN_TYPE_DIVX311:
    case FILE_SOURCE_MN_TYPE_DIVX40:
    case FILE_SOURCE_MN_TYPE_DIVX50_60:
      videoCoding = (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingDivX;
      break;
   case FILE_SOURCE_MN_TYPE_VP6F:
      videoCoding = (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingVP;
      break;
   case FILE_SOURCE_MN_TYPE_SORENSON_SPARK:
      videoCoding = (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingSpark;
      break;
   case FILE_SOURCE_MN_TYPE_STILL_IMAGE:
      videoCoding = OMX_VIDEO_CodingMPEG4;
      break;
   default:
     break;
  }
  return videoCoding;
}

/** @brief Convert the File Source minor type to minor type.
*
* @param[in] minorType - File Source minor type
* @param[out] pMimeMinorType - Reference to minor type
*/
QOMX_OTHER_CODINGTYPE HTTPSourceMMITrackHandler::GetOmxOtherMinorType
(
 const FileSourceMnMediaType minorType
)
{
  QOMX_OTHER_CODINGTYPE otherCoding = QOMX_OTHER_CodingUnused;

  switch(minorType)
  {
  case FILE_SOURCE_MN_TYPE_SMPTE_TIMED_TEXT:
     otherCoding = QOMX_OTHER_CodingSMPTETT;
     break;

  default:
    break;
  }
  return otherCoding;
}

/** @brief Convert the File Source minor type to minor type.
*
* @param[in] minorType - File Source minor type
* @param[out] pMimeMinorType - Reference to minor type
*/
OMX_IMAGE_CODINGTYPE  HTTPSourceMMITrackHandler::GetOmxImageMinorType
(
  const FileSourceMnMediaType minorType
)
{
  OMX_IMAGE_CODINGTYPE imageEnoding = OMX_IMAGE_CodingUnused;

  switch(minorType)
  {
  case FILE_SOURCE_MN_TYPE_PNG:
    imageEnoding = OMX_IMAGE_CodingPNG;
    break;

  default:
    break;
  }
  return imageEnoding;
}

/** @brief Check for download completion.
*
* @return
* TRUE - Download complete
* FALSE - Otherwise
*/
bool HTTPSourceMMITrackHandler::IsDownloadComplete(HTTPMediaType mediaType)
{
  bool bDownloadComplete = false;
  uint32 currStartOffset;
  uint32 downloadOffset;

  (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
  if (m_pHTTPController &&
      m_pHTTPController->GetDownloadProgress(mediaType,
                              currStartOffset,
                              downloadOffset,
                                             HTTPCommon::HTTP_DOWNLOADPROGRESS_UNITS_DATA,
                                             bDownloadComplete))
  {
    m_bEOS = bDownloadComplete;
  }
  (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);

  return bDownloadComplete;
}

/** @brief Get number of tracks.
*
* @param[in] portIdx - port index
* @return number of tracks
*/
uint32 HTTPSourceMMITrackHandler::GetNumberOfTracks(OMX_U32 portIdx)
{
  uint32 num = 0;

  //Loop through the track list and update the track state if found
  for (ListPair<TrackDescription>* it = m_trackList.iterator();
       it != NULL; it = it->tail())
  {
    TrackDescription& trackDescription = it->head();
    if ((portIdx == MMI_HTTP_AUDIO_PORT_INDEX) &&
        (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_AUDIO))
    {
      num++;
    }
    else if ((portIdx == MMI_HTTP_VIDEO_PORT_INDEX) &&
        (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_VIDEO))
    {
      num++;
    }
    else if ((portIdx == MMI_HTTP_OTHER_PORT_INDEX) &&
        (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_TEXT))
    {
      num++;
    }
  }
  return num;
}

/** @brief Check (over all tracks) if buffering.
*
* @return
* TRUE - Buffering
* FALSE - Otherwise (means Playing)
*/
bool HTTPSourceMMITrackHandler::IsBuffering()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMITrackHandler::IsBuffering" );
  bool bBuffering = true;

  //Return buffering if atleast one track is buffering
  //If seek is pending then it is by default in buffering state
  TrackDescription trackDescription;
  if((!IsSeekPending()) && m_trackList.size() > 0)
  {
    bBuffering = false;
    (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
    for (int index = 0; index < m_trackList.size(); index++)
    {
      (void)m_trackList.get(index, trackDescription);
      if (trackDescription.selectionState.isSelected &&
          trackDescription.playState == BUFFERING)
      {
        bBuffering = true;
        break;
      }
    }
    (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
  }

  return bBuffering;
}

/** @brief Find if given track exists in the track list and return track description.
*
* @param[in] trackIdentifier - Track ID
* @param[in]  majorType - HTTP media major type
* @param[out] ppTrackDescription - Reference to track description
* @return
* TRUE - Track found
* FALSE - Otherwise
*/
bool HTTPSourceMMITrackHandler::FindTrack
(
 const int32 trackIdentifier,
 HTTPMediaType majorType,
 TrackDescription** ppTrackDescription
 )
{
  bool bFound = false;
  FileSourceMjMediaType fsMajorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;

  HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType( majorType, fsMajorType );

  if (ppTrackDescription)
  {
    //Loop through the track list and return trackDescription if found
    (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
    for (ListPair<TrackDescription>* it = m_trackList.iterator();
         it != NULL; it = it->tail())
    {
      *ppTrackDescription = &it->head();
      if (it->head().trackID == trackIdentifier &&
          it->head().majorType == fsMajorType )
      {
        bFound = true;
        break;
      }
    }
    (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
  }

  return bFound;
}

/** @brief Find if given media type exists in the track list and return the first
*   selected track description.
*
* @param[in]  majorType - HTTP media major type
* @param[out] ppTrackDescription - Reference to track description
* @return
* TRUE - Track found
* FALSE - Otherwise
*/
bool HTTPSourceMMITrackHandler::FindMediaType
(
 const HTTPMediaType majorType,
 TrackDescription** ppTrackDescription
 )
{
  bool bFound = false;
  FileSourceMjMediaType fsMajorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;

  HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType( majorType, fsMajorType );

  if (ppTrackDescription)
  {
    //Loop through the track list and return trackDescription if found
    (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
    for (ListPair<TrackDescription>* it = m_trackList.iterator();
         it != NULL; it = it->tail())
    {
      if (it->head().majorType == fsMajorType &&
          it->head().selectionState.isSelected)
      {
        bFound = true;
        *ppTrackDescription = &it->head();
        break;
      }
    }
    (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);
  }

  return bFound;
}

/**
* @brief: Get the track duration in millisec
*
* @param[out] uint32 duration
*
* @return true or false indicating the success of operation
*/
bool HTTPSourceMMITrackHandler::GetMediaDuration
(
  OMX_U32  portIdx,
  OMX_U32  &duration
 )
{
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMITrackHandler::GetMediaduration for port:%u",
               (uint32) portIdx );

  duration = 0;
  bool bOK = false;

  if(m_pDataInterface)
  {
    HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
    MapPortIDToMediaType(portIdx,mediaType);
    HTTPCommon::HTTPAttrVal attr_dur;
    if (m_pDataInterface->GetConfig(mediaType,HTTPCommon::HTTP_ATTR_DURATION,attr_dur))
        {
      duration = attr_dur.int_attr_val;
          bOK = true;
    }
  }

  return bOK;
}

/**
* @brief Get the selected trackID for given port
*
* @param portIdx
* @param trackID
*
* @return true indicating success; false otherwise
*/
bool HTTPSourceMMITrackHandler::GetTrackID
(
  OMX_U32 portIdx,
  uint32 &trackID
 )
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
  "HTTPSourceMMITrackHandler::GetTrackID for port:%u",(uint32)portIdx );

  TrackDescription trackDescription;
  bool bOK = false;

  for (int index = 0; index < m_trackList.size(); index++)
  {
    (void)m_trackList.get(index, trackDescription);
    if (trackDescription.selectionState.isSelected)
    {
      if (((portIdx == MMI_HTTP_AUDIO_PORT_INDEX) &&
         (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_AUDIO)) ||
         ((portIdx == MMI_HTTP_VIDEO_PORT_INDEX) &&
         (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_VIDEO)))
      {
        trackID = trackDescription.trackID;
        bOK = true;
        break;
      }
    }
  }
  return bOK;
}

/**
 * @brief
 *  Returns the state of the track buffering or playing.
 *
 * @param trackID
 * @param majorType
 *
 * @return HTTPSourceMMITrackHandler::TrackPlayState
 */
HTTPSourceMMITrackHandler::TrackPlayState HTTPSourceMMITrackHandler::GetTrackPlayState
(
  uint32 trackID,
  HTTPMediaType majorType
)
{
  TrackPlayState trackPlayState = UNKNOWN;

  TrackDescription* pTrackDescription = NULL;

  (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
  if (FindTrack(trackID, majorType, &pTrackDescription))
  {
    if (pTrackDescription)
    {
      trackPlayState = pTrackDescription->playState;
    }
  }
  (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);

  return trackPlayState;
}

/**
 * @brief This Method maps the portid to corresponding
 *        media major type
 *
 * @param portIdx
 * @param mediaType
 */
void HTTPSourceMMITrackHandler::MapPortIDToMediaType
(
  OMX_U32 portIdx,
  HTTPMediaType &mediaType
)
{
  mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;

  if(portIdx == MMI_HTTP_AUDIO_PORT_INDEX )
  {
    mediaType = HTTPCommon::HTTP_AUDIO_TYPE;
  }
  else if(portIdx == MMI_HTTP_VIDEO_PORT_INDEX)
  {
    mediaType = HTTPCommon::HTTP_VIDEO_TYPE;
  }
}

/**
 * @brief This method maps HTTPMediaMinorType to Filesource
 *        minor media type
 *
 * @param[in] httpMinorType the HTTP Media Minor Type
 * @param[out] fsMinorType populated Filesource Media Minor Type
 */
void HTTPSourceMMITrackHandler::MapHTTPMinorTypeToFileSourceMinorType
(
  HTTPMediaMinorType httpMinorType,
  FileSourceMnMediaType& fsMinorType
)
{
  fsMinorType = FILE_SOURCE_MN_TYPE_UNKNOWN;

  if(httpMinorType == HTTP_MINOR_TYPE_AAC_ADTS)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_AAC_ADTS;
  }
  if(httpMinorType == HTTP_MINOR_TYPE_AAC_ADIF)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_AAC_ADIF;
  }
  if(httpMinorType == HTTP_MINOR_TYPE_AAC_LOAS)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_AAC_LOAS;
  }
  if(httpMinorType == HTTP_MINOR_TYPE_AAC )
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_AAC;
  }
  if(httpMinorType == HTTP_MINOR_TYPE_AC3)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_AC3;
  }
  if(httpMinorType == HTTP_MINOR_TYPE_EAC3)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_EAC3;
  }
  if(httpMinorType == HTTP_MINOR_TYPE_MP2)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_MP2;
  }
  if(httpMinorType == HTTP_MINOR_TYPE_MPEG2)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_MPEG2;
  }
  else if (httpMinorType == HTTP_MINOR_TYPE_H264)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_H264;
  }
  else if (httpMinorType == HTTP_MINOR_TYPE_HVC)
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_HEVC;
  }
  else if(httpMinorType == HTTP_MINOR_TYPE_SMPTETT )
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_SMPTE_TIMED_TEXT;
  }
  else if(httpMinorType == HTTP_MINOR_TYPE_PNG )
  {
    fsMinorType = FILE_SOURCE_MN_TYPE_PNG;
  }
}

/*
 * Mapping Media type to Port ID
 *
 * @param[out] portIdx - port ID
 * @param[in]  mediaType - HTTP media type
 * @return - None
 */
void HTTPSourceMMITrackHandler::MapMediaTypeToPortID
(
  OMX_U32 &portIdx,
  HTTPMediaType mediaType
)
{
  portIdx = MAX_UINT32_VAL;

  if(mediaType == HTTPCommon::HTTP_AUDIO_TYPE )
  {
    portIdx = MMI_HTTP_AUDIO_PORT_INDEX;
  }
  else if(mediaType == HTTPCommon::HTTP_VIDEO_TYPE)
  {
    portIdx = MMI_HTTP_VIDEO_PORT_INDEX;
  }
}

/**
 * @brief This Method fills in the extra data info to text
 *        sample buffer
 *
 * @param nPortIndex
 * @param pBuffHdr
 * @param pTrackDescription
 * @param httpSampleInfo
 *
 * @return   MMI_S_COMPLETE on success otherwise appropriate
 *           error code
 */
OMX_U32 HTTPSourceMMITrackHandler::FillExtraDataForSubTitles
(
  OMX_U32 /*nPortIndex*/,
  OMX_BUFFERHEADERTYPE *pBuffHdr,
  TrackDescription* pTrackDescription,
  HTTPSampleInfo &httpSampleInfo
)
{
  OMX_U32 ret = MMI_S_COMPLETE;

  if (pBuffHdr && pTrackDescription)
  {
    OMX_OTHER_EXTRADATATYPE *pSubTitleExtraData = NULL;
    OMX_OTHER_EXTRADATATYPE *pNoMoreExtraData = NULL;
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "FillExtraSampleInfoData::pBufHdr->pBuffer[%p] FilledLen[%d]",
                  pBuffHdr->pBuffer,
                  (int)pBuffHdr->nFilledLen);
    /* Filling up OMX_EXTRA_SAMPLE_INFO */
    size_t ulAddr =  (size_t)(pBuffHdr->pBuffer +  pBuffHdr->nFilledLen);
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "FillExtraDataForSubTitles::ulAddr[%ux]", ulAddr);

    uint32 requiredExtraDataLen  = (OMX_U32)(MMI_HTTP_TEXT_EXTRADATA_BUFFER_SIZE);

    if (requiredExtraDataLen <= (pBuffHdr->nAllocLen - pBuffHdr->nFilledLen))
    {
      /* Aligned address to DWORD boundary */
      ulAddr = (ulAddr + 0x3) & (~0x3);
      pSubTitleExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
      /*Setting OMX_BUFFERFLAG_EXTRADATA flag*/
      pBuffHdr->nFlags |= OMX_BUFFERFLAG_EXTRADATA;

      /* fill in the sample dimension info */
      pSubTitleExtraData->nPortIndex = MMI_HTTP_OTHER_PORT_INDEX;
      pSubTitleExtraData->nVersion = pBuffHdr->nVersion;
      pSubTitleExtraData->eType = (OMX_EXTRADATATYPE)QOMX_HTTP_IndexParamSMPTETimeTextDimensionExtraData;
      pSubTitleExtraData->nSize = (OMX_U32)sizeof(OMX_OTHER_EXTRADATATYPE)+
                                  (OMX_U32)(sizeof(OMX_U8)* sizeof(QOMX_SUBTITILE_DIMENSIONS)) - 4;
      pSubTitleExtraData->nDataSize = (OMX_U32)sizeof(QOMX_SUBTITILE_DIMENSIONS);
      union
      {
        OMX_U8* in;
        QOMX_SUBTITILE_DIMENSIONS* out;
      }u;
      u.in = pSubTitleExtraData->data;
      QOMX_SUBTITILE_DIMENSIONS *dimensionsOffset = u.out;
      dimensionsOffset->nHeight = pTrackDescription->frameHeight;
      dimensionsOffset->nWidth = pTrackDescription->frameWidth;
      dimensionsOffset->nDuration = (uint32)STD_MAX(0,(int64)(httpSampleInfo.endTime - httpSampleInfo.startTime));
      dimensionsOffset->nStartOffset = httpSampleInfo.sSubInfo.periodStartTime;

      ulAddr += (size_t)pSubTitleExtraData->nSize;


      /* Aligned address to DWORD boundary */
      ulAddr = (ulAddr + 0x3) & (~0x3);

      /* Fill in the subs info associated with the sample */
      if (httpSampleInfo.sSubInfo.sSubTitle.subSampleCount > 0)
      {
        FileSourceMnMediaType fsMinorType = FILE_SOURCE_MN_TYPE_UNKNOWN;
        MapHTTPMinorTypeToFileSourceMinorType(httpSampleInfo.sSubInfo.sSubTitle.eSubMnType,fsMinorType);
        OMX_IMAGE_CODINGTYPE eCoding = GetOmxImageMinorType(fsMinorType);
        if (eCoding != OMX_IMAGE_CodingUnused)
        {
          OMX_OTHER_EXTRADATATYPE *pSubTitleSubExtraData  = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
          if (pSubTitleSubExtraData)
          {
            pSubTitleSubExtraData->nPortIndex = MMI_HTTP_OTHER_PORT_INDEX;
            pSubTitleSubExtraData->nVersion = pBuffHdr->nVersion;
            pSubTitleSubExtraData->eType = (OMX_EXTRADATATYPE)QOMX_HTTP_IndexParamSMPTETimeTextSubInfoExtraData;
            pSubTitleSubExtraData->nSize = (OMX_U32)(sizeof(OMX_OTHER_EXTRADATATYPE) +
                                           (sizeof(OMX_U8) * sizeof(QOMX_SUBTITLE_SUB_INFOTYPE)) - 4);
            pSubTitleSubExtraData->nDataSize = (OMX_U32)sizeof(QOMX_SUBTITLE_SUB_INFOTYPE);
            union
            {
               OMX_U8* in;
               QOMX_SUBTITLE_SUB_INFOTYPE* out;
            }u;
            u.in =pSubTitleSubExtraData->data;
            QOMX_SUBTITLE_SUB_INFOTYPE *subsOffset  = u.out;
            subsOffset->subSampleCount = httpSampleInfo.sSubInfo.sSubTitle.subSampleCount;
            subsOffset->eSubCodingType = eCoding;
            subsOffset->nSubtitleSubInfoSize = httpSampleInfo.sSubInfo.sSubTitle.subtitleSubInfoSize;
            memcpy(subsOffset->cSubtitleSubInfo,httpSampleInfo.sSubInfo.sSubTitle.cSubtitleSubInfo,
                   subsOffset->nSubtitleSubInfoSize);
            ulAddr += (size_t)pSubTitleSubExtraData->nSize;
            /* Aligned address to DWORD boundary */
            ulAddr = (ulAddr + 0x3) & (~0x3);
          }
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,"no Subs for SubTitles");
      }

      /* Append OMX_ExtraDataNone to indicate end of extra data*/
      pNoMoreExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
      if(pNoMoreExtraData)
      {
        pNoMoreExtraData->nSize = (OMX_U32)sizeof(OMX_OTHER_EXTRADATATYPE);
        pNoMoreExtraData->nVersion = pBuffHdr->nVersion;
        pNoMoreExtraData->nPortIndex = MMI_HTTP_OTHER_PORT_INDEX;
        pNoMoreExtraData->eType = OMX_ExtraDataNone;
        pNoMoreExtraData->nDataSize = 0;
        ulAddr += (size_t)pNoMoreExtraData->nSize;
      }

      /* Aligned address to DWORD boundary */
      ulAddr = (ulAddr + 0x3) & (~0x3);
    }
    else
    {
       QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "InSufficient Buffer for Subtitile Extra Data, required(%u), provided(%u)",
                     requiredExtraDataLen,(uint32)pBuffHdr->nSize );
       ret = MMI_S_EBADPARAM;
    }
  }
  else
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Either buffer(%p) or TrackDescription(%p) is NULL",
                  pBuffHdr, pTrackDescription);
    ret = MMI_S_EBADPARAM;
  }
  return ret;
}
/*
 * Filling up extra sample info and PSSH info as extra data
 * @param[in]  nPortIndex - port index
 * @param[out] pBuffHdr - buffer header
 * @param[in]  pExtraSampleInfo - extra sample info
 * @param[in]  pPsshInfo       - pssh infor
 & @return - OMX_ERRORTYPE
 */

OMX_U32 HTTPSourceMMITrackHandler::FillExtraDataForExtraSampleInfo
(
  OMX_U32 nPortIndex,
  OMX_BUFFERHEADERTYPE *pBuffHdr,
  QOMX_PARAM_STREAMING_PSSHINFO *pPsshInfo,
  QOMX_EXTRA_SAMPLE_INFO *pExtraSampleInfo
)
{
  OMX_U32 eError = MMI_S_EBADPARAM;
  size_t ulAddr;
  OMX_OTHER_EXTRADATATYPE *pExtraSampleInfoData = NULL;
  OMX_OTHER_EXTRADATATYPE *pPSSHInfoData = NULL;
  OMX_OTHER_EXTRADATATYPE *pNoMoreExtraData = NULL;
  if((NULL != pBuffHdr ) && (NULL != pPsshInfo))
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
        "FillExtraDataForExtraSampleInfo::pBufHdr->nAllocLen[%u] FilledLen[%u]",
        (uint32)pBuffHdr->nAllocLen,
        (uint32)pBuffHdr->nFilledLen);


    if (MMI_HTTP_MAX_PORT_ENC_BUFFER_SIZE <= (pBuffHdr->nAllocLen - pBuffHdr->nFilledLen))
    {
      /* Filling up OMX_EXTRA_SAMPLE_INFO */
      ulAddr = (size_t)pBuffHdr->pBuffer +  (size_t)pBuffHdr->nFilledLen;

      /* Aligned address to DWORD boundary */
      ulAddr = (ulAddr + 0x3) & (~0x3);
      pPSSHInfoData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
       /*Setting OMX_BUFFERFLAG_EXTRADATA flag*/
      pBuffHdr->nFlags |= OMX_BUFFERFLAG_EXTRADATA;
      pPSSHInfoData->nSize = (OMX_U32)(sizeof(OMX_OTHER_EXTRADATATYPE) +
                             sizeof(OMX_U8)* sizeof(QOMX_PARAM_STREAMING_PSSHINFO) -4);
      pPSSHInfoData->nVersion = pBuffHdr->nVersion;
      pPSSHInfoData->nPortIndex = nPortIndex;
      pPSSHInfoData->nDataSize = (OMX_U32)sizeof(QOMX_PARAM_STREAMING_PSSHINFO);
      pPSSHInfoData->eType = (OMX_EXTRADATATYPE)QOMX_HTTP_IndexParamPsshInfo;
      /* Fill QOMX_PARAM_STREAMING_PSSHINFO data into data*/
      memcpy(pPSSHInfoData->data, pPsshInfo, sizeof(QOMX_PARAM_STREAMING_PSSHINFO));

      ulAddr += (size_t)pPSSHInfoData->nSize;

      if (pExtraSampleInfo)
      {
        /* Filling up OMX_EXTRA_SAMPLE_INFO */
        /* Aligned address to DWORD boundary */
        ulAddr = (ulAddr + 0x3) & (~0x3);
         /*Setting OMX_BUFFERFLAG_EXTRADATA flag*/
        pBuffHdr->nFlags |= OMX_BUFFERFLAG_EXTRADATA;
          pExtraSampleInfoData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
        pExtraSampleInfoData->nSize = (OMX_U32)(sizeof(OMX_OTHER_EXTRADATATYPE) +
                                      sizeof(OMX_U8)* sizeof(QOMX_EXTRA_SAMPLE_INFO) -4);
        pExtraSampleInfoData->nVersion = pBuffHdr->nVersion;
        pExtraSampleInfoData->nPortIndex = nPortIndex;
        pExtraSampleInfoData->nDataSize = (OMX_U32)sizeof(QOMX_EXTRA_SAMPLE_INFO);
        pExtraSampleInfoData->eType = (OMX_EXTRADATATYPE)QOMX_HTTP_IndexParamExtraSampleInfo;
        /* Fill OMX_EXTRA_SAMPLE_INFO data into data*/
        memcpy(pExtraSampleInfoData->data, pExtraSampleInfo, sizeof(QOMX_EXTRA_SAMPLE_INFO));
        ulAddr += (size_t)pExtraSampleInfoData->nSize;
      }

      /* Append OMX_ExtraDataNone to indicate end of extra data*/
      pNoMoreExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;
      pNoMoreExtraData->nSize = (OMX_U32)sizeof(OMX_OTHER_EXTRADATATYPE);
      pNoMoreExtraData->nVersion = pBuffHdr->nVersion;
      pNoMoreExtraData->nPortIndex = nPortIndex;
      pNoMoreExtraData->eType = OMX_ExtraDataNone;
      pNoMoreExtraData->nDataSize = 0;
      eError = MMI_S_COMPLETE;
    }
    else
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                "InSufficient Buffer for PSSH and ExtraSampleInfo, required(%d), provided(%u)",
                MMI_HTTP_MAX_PORT_ENC_BUFFER_SIZE,
                (uint32)pBuffHdr->nSize );
    }
  }
  return eError;
}

int HTTPSourceMMITrackHandler::GenerateUniqueID()
{
  ++m_nUniqueID;
  return (m_nUniqueID);
}

void HTTPSourceMMITrackHandler::SetLastPsshUniqueIdQueried(FileSourceMjMediaType majorType, int uniqId)
{
  if (majorType == FILE_SOURCE_MJ_TYPE_AUDIO)
  {
    m_nLastAudioPsshUniqId = uniqId;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Set:LastAudioPsshUniqId %d",m_nLastAudioPsshUniqId);
  }
  else if (majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
  {
    m_nLastVideoPsshUniqId = uniqId;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Set:LastVideoPsshUniqId %d",m_nLastVideoPsshUniqId);
  }
  return;
}

int HTTPSourceMMITrackHandler::GetLastPsshUniqueIdQueried(FileSourceMjMediaType majorType)
{
  int uniqueId = 0;
  if (majorType == FILE_SOURCE_MJ_TYPE_AUDIO)
  {
    uniqueId = m_nLastAudioPsshUniqId;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Get:LastAudioPsshUniqId %d",m_nLastAudioPsshUniqId);
  }
  else if (majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
  {
    uniqueId = m_nLastVideoPsshUniqId;
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Get:LastVideoPsshUniqId %d",m_nLastVideoPsshUniqId);
  }
  return uniqueId;
}

/** @brief Selects or mutes (or deselects or unmutes) a given track.
*
* @param[out] uniqID - unique ID for querying PSSH info
* @param[in] majorType - File source major type
* @return
* TRUE - Job accepted successfully
* FALSE - Otherwise
*/

int HTTPSourceMMITrackHandler::GetPsshUniqueId
(
  FileSourceMjMediaType majorType
)
{
  int uniqID = -1;
  bool eRet = false;
  (void)MM_CriticalSection_Enter(m_pTrackHandlerDataLock);
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
               "lastPSSHIndexQueried %d",m_lastPSSHIndexQueried);
  if (m_lastPSSHIndexQueried <= MAX_PSSH_INFO_ENTRY)
  {
    for (int id = m_lastPSSHIndexQueried + 1; id < MAX_PSSH_INFO_ENTRY ; id++)
    {
      if (TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NEW &&
          TrackDrmInfo[id].majorType == majorType &&
          TrackDrmInfo[id].nUniqueID > GetLastPsshUniqueIdQueried(majorType))
      {
        uniqID  = TrackDrmInfo[id].nUniqueID;
        m_lastPSSHIndexQueried = id;
        eRet = true;
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "Starting from lastPSSHIndexQueried %d, unique ID %d",m_lastPSSHIndexQueried, uniqID );
        break;
      }
    }
    if (!eRet)
    {
      for (int id = 0; (id < MAX_PSSH_INFO_ENTRY) && (id < m_lastPSSHIndexQueried + 1) ; id++)
      {
        if (TrackDrmInfo[id].ePsshInfoStatus == PSSH_INFO_NEW &&
            TrackDrmInfo[id].majorType == majorType &&
            TrackDrmInfo[id].nUniqueID > GetLastPsshUniqueIdQueried(majorType))
        {
          uniqID  = TrackDrmInfo[id].nUniqueID;
          m_lastPSSHIndexQueried = id;
          eRet = true;
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
               "Starting before lastPSSHIndexQueried %d, unique ID %d",m_lastPSSHIndexQueried, uniqID );
          break;
        }
      }
    }
  }
  else
  {
     QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
               "Sanity failed : lastPSSHIndexQueried %d",m_lastPSSHIndexQueried );
  }
  (void)MM_CriticalSection_Leave(m_pTrackHandlerDataLock);

  if (uniqID == -1)
  {
    uniqID = GetLastPsshUniqueIdQueried(majorType);
  }
  return uniqID;
}


void HTTPSourceMMITrackHandler::PrintTrackDRMInfo()
{
  #if 0
  for (int id = 0; id < MAX_PSSH_INFO_ENTRY; id++)
  {
    QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
             "TrackDrmInfo [%d] ePsshInfoStatus %d majorType %d nUniqueID %d",
             id, TrackDrmInfo[id].ePsshInfoStatus,
             TrackDrmInfo[id].majorType, TrackDrmInfo[id].nUniqueID);
  }
  #endif
  return;

}

}/* namespace video */
