/************************************************************************* */
/**
 * HTTPResource.cpp
 * @brief Implements the HTTP Resource
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPResource.cpp#38 $
$DateTime: 2013/05/12 09:54:28 $
$Change: 3751774 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "HTTPResource.h"
#include "SourceMemDebug.h"
#include "HTTPDataManager.h"

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
 * Constructor
 *
 * @param[in] nResourceKey, key for the resource
 * @param[out] bResult, TRUE on success else failure
 */
HTTPResource::HTTPResource(bool& bResult, MPDProfileType profileType) :
  m_nSeekTime(-1),
  m_nKey(0),
  m_pFileSource(NULL),
  m_pDataManager(NULL),
  m_pSidxDataManager(NULL),
  m_resourceDataLock(NULL),
  m_pTrackMap(NULL),
  m_nNumOfTracks(0),
  m_nFlushTime(0),
  m_nProfileType(profileType)
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "creating HTTP resource");
  bResult = (MM_CriticalSection_Create(&m_resourceDataLock) == 0);

  if (bResult)
  {
    m_pFileSource = QTV_New_Args( ::FileSource,
                                  ( HTTPResource::_ProcessFileSourceEvent,
                                  reinterpret_cast<void *> (this) ) );
    if ( m_pFileSource == NULL )
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "unable to create filesource");
      bResult = false;
    }
    else
    {
      m_pDataManager = QTV_New_Args( HTTPDataManager, ( bResult) );
      if ( m_pDataManager == NULL )
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "unable to create data manager");
        bResult = false;
      }

      if(bResult)
      {
        m_pSidxDataManager = QTV_New_Args( HTTPDataManager, ( bResult) );
        if ( m_pSidxDataManager == NULL )
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "unable to create sidx data manager");
          bResult = false;
        }
      }
    }
  }
}

/*
 * Destructor
 */
HTTPResource::~HTTPResource()
{
  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                 "HTTPResource [%d %d] being destroyed",
                 (int)(m_nKey >> 32), (int)m_nKey );

  if ( m_pTrackMap )
  {
    QTV_Free(m_pTrackMap);
    m_pTrackMap = NULL;
  }



  if ( m_pFileSource )
  {
    QTV_Delete(m_pFileSource);
    m_pFileSource = NULL;
  }
  if ( m_pDataManager )
  {
    QTV_Delete(m_pDataManager);
    m_pDataManager = NULL;
  }
  if ( m_pSidxDataManager )
  {
    QTV_Delete(m_pSidxDataManager);
    m_pSidxDataManager = NULL;
  }
  if (m_resourceDataLock)
  {
    (void)MM_CriticalSection_Release(m_resourceDataLock);
    m_resourceDataLock = NULL;
  }
}

/*
 * Process the file source events
 *
 * @param[in] status - File source event/status
 * @param[in] pCbData - Reference to callback data
 *
 */
void HTTPResource::_ProcessFileSourceEvent
(
  FileSourceCallBackStatus status,
  void* pCbData
)
{
  HTTPResource *pResource = (HTTPResource *)pCbData;
  if (pResource)
  {
    pResource->ProcessFileSourceEvent( status );
  }
}

/**
 * Check to see if data segment with key 'nKey' is present in
 * this resource.
 */
bool HTTPResource::IsSegmentPresent(const uint64 nKey)
{
  bool rslt = false;
  uint64 nStartTimeForFirstCancellableUnit = MAX_UINT64_VAL;
  HTTPSegmentsInfo availiableSegmentsInfo;
  HTTPSegmentInfo segmentArray[HTTP_MAX_NUMBER_OF_SEGMENTS];

  availiableSegmentsInfo.maxSegments = HTTP_MAX_NUMBER_OF_SEGMENTS;
  availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray = segmentArray;

  const int64 offset = 0;

  HTTPCommon::HTTPDownloadStatus tmpStat =
    m_pDataManager->GetAvailableSegments(availiableSegmentsInfo, offset);

  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "IsSegmentPresent frag %d, numInUse %d, status %d",
     (int)nKey, (int)availiableSegmentsInfo.m_NumSegmentsInUse, (int)tmpStat);

  for (int i = 0; i < availiableSegmentsInfo.m_NumSegmentsInUse; i++)
  {
    if (nKey == availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_Key)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "IsSegmentPresent Frag found!");
      rslt = true;
      break;
    }
  }

  return rslt;
}

/**
 * Gets the data unit key for the data segment following
 * 'nTooSlowDataUnitKey' if any. If not, return MAX_UNIT64_VAL.
 */
void HTTPResource::GetFirstCancellableDataUnit(
    const uint64 nTooSlowDataUnitKey,
    uint64& nFirstCancellableDataUnitKey)
{
  uint64 nStartTimeForFirstCancellableUnit = MAX_UINT64_VAL;
  HTTPSegmentsInfo availiableSegmentsInfo;
  HTTPSegmentInfo segmentArray[HTTP_MAX_NUMBER_OF_SEGMENTS];

  availiableSegmentsInfo.maxSegments = HTTP_MAX_NUMBER_OF_SEGMENTS;
  availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray = segmentArray;

  const int64 offset = 0;
  (void)m_pDataManager->GetAvailableSegments(availiableSegmentsInfo, offset);

  for (int i = 0; i < availiableSegmentsInfo.m_NumSegmentsInUse; i++)
  {
    //Ignore the init segment(s)
    uint64 nCurrKey = availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_Key;
    if (nCurrKey == nTooSlowDataUnitKey)
    {
      if (i < availiableSegmentsInfo.m_NumSegmentsInUse - 1)
      {
        nFirstCancellableDataUnitKey = availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i+1].m_Key;
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "Found dataunit key to cancel in same resource as tooslowunit (%d,%d)",
          (int)GetKey(),(int)nFirstCancellableDataUnitKey);
      }
      break;
    }
  }
}

/**
 * Find the data unit key associated with the first data segment
 * in the resource. If none found then return MAX_UINT64_VAL.
 */
uint64 HTTPResource::GetFirstDataUnitKey()
{
  uint64 nFirstDataUnitKey = MAX_UINT64_VAL;

  uint64 nStartTimeForFirstCancellableUnit = MAX_UINT64_VAL;
  HTTPSegmentsInfo availiableSegmentsInfo;
  HTTPSegmentInfo segmentArray[HTTP_MAX_NUMBER_OF_SEGMENTS];

  availiableSegmentsInfo.maxSegments = HTTP_MAX_NUMBER_OF_SEGMENTS;
  availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray = segmentArray;

  const int64 offset = 0;
  HTTPCommon::HTTPDownloadStatus tmpStatus =
    m_pDataManager->GetAvailableSegments(availiableSegmentsInfo, offset);

  for (int i = 0; i < availiableSegmentsInfo.m_NumSegmentsInUse; ++i)
  {
    uint64 nCurDataUnitKey = availiableSegmentsInfo.m_pHTTPSegmentInUseInfoArray[i].m_Key;
    if (IsDataUnit(nCurDataUnitKey))
    {
      nFirstDataUnitKey = nCurDataUnitKey;
      break;
    }
  }

  if (nFirstDataUnitKey < MAX_UINT64_VAL)
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "HTTPResource::GetFirstDataUnitKey (%d,%d)",
                  (int)m_nKey, (int)(nFirstDataUnitKey));
  }

  return nFirstDataUnitKey;
}

/*
 * setup tracks
 *
 * @return HTTPDL_SUCCESS on success and HTTPDL_ERROR_ABORT on failure
 */
HTTPDownloadStatus HTTPResource::SetupTracks()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  FileSourceTrackIdInfoType *pTrackList = NULL;
  uint32 nNumTracks;
  uint32 nNumSelectedTracks = 0;
  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Setting up tracks for resource [%u %u] filesource %p",
                 (uint32)(m_nKey >> 32), (uint32)m_nKey,(void *) m_pFileSource );

  MM_CriticalSection_Enter(m_resourceDataLock);

  /* get the track list */
  nNumTracks = m_pFileSource->GetWholeTracksIDList(pTrackList);
  pTrackList =
    (FileSourceTrackIdInfoType *)QTV_Malloc( nNumTracks *
                                             sizeof(FileSourceTrackIdInfoType) );
  if ( pTrackList )
  {
    nNumTracks = m_pFileSource->GetWholeTracksIDList(pTrackList);
    /* get the number of selected tracks */
    for (uint32 index = 0; index < nNumTracks; index++)
    {
      if ( pTrackList[index].selected )
      {
        nNumSelectedTracks++;
      }
    }

    if ( nNumSelectedTracks > 0 )
    {

      if ( m_pTrackMap )
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "invalid track list removing it" );
        /* this should not be the case */
        QTV_Free(m_pTrackMap);
        m_pTrackMap = NULL;
      }

      m_pTrackMap =
        (HTTPResourceTrackMap *)QTV_Malloc(  nNumSelectedTracks *
                                             sizeof(HTTPResourceTrackMap) );
    }
    if ( m_pTrackMap )
    {
      std_memset( m_pTrackMap, 0,
                  nNumSelectedTracks * sizeof(HTTPResourceTrackMap) );

      nNumSelectedTracks = 0;
      for ( uint32 index = 0; index < nNumTracks; index++  )
      {
        if ( pTrackList[index].selected )
        {
          m_pTrackMap[nNumSelectedTracks].m_nTrackID = pTrackList[index].id;
          m_pTrackMap[nNumSelectedTracks].m_majorType =
            GetHTTPMediaMajorType(pTrackList[index].majorType);
          m_pTrackMap[nNumSelectedTracks].m_nPrevSampleTS = 0;
          m_pTrackMap[nNumSelectedTracks].m_nMediaTimeScale = getTimeScale(m_pTrackMap[nNumSelectedTracks].m_majorType,
                                                              m_pTrackMap[nNumSelectedTracks].m_nTrackID);
          if(0 == m_pTrackMap[nNumSelectedTracks].m_nMediaTimeScale)
          {
            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                          "Wrong MediaTimeScale value" );
          }

          nNumSelectedTracks++;
        }
      }
      m_nNumOfTracks = nNumSelectedTracks;

      status = HTTPCommon::HTTPDL_SUCCESS;
    }
    QTV_Free(pTrackList);
    pTrackList= NULL;
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to allocate track data structures" );
  }

  MM_CriticalSection_Leave(m_resourceDataLock);

  if((DASH_PROFILE_MP2T_MAIN == m_nProfileType) || (DASH_PROFILE_MP2T_SIMPLE == m_nProfileType))
  {
    // Check if audio is present in MP2T &
    // set parser in strip header and frame by frame mode for aac adts
    if(HTTPCommon::HTTPDL_SUCCESS == status)
    {
      status = SetAudioTrackProperties();
    }

    // Set base time for the segment as zero to get absolute timestamp from parser
    SetBaseTime(0);
  }

  return status;
}

/*
 * setup audio track properties
 *
 * @return HTTPDL_SUCCESS on success and HTTPDL_ERROR_ABORT on failure
 */
HTTPDownloadStatus HTTPResource::SetAudioTrackProperties()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  HTTPMediaTrackInfo tInfo;
  FileSource *pFileSource = GetFileSource();
  if ( pFileSource && GetSelectedMediaTrackInfo(HTTPCommon::HTTP_AUDIO_TYPE, tInfo))
  {
    if(HTTP_MINOR_TYPE_AAC == tInfo.minorType      ||
       HTTP_MINOR_TYPE_AAC_ADTS == tInfo.minorType ||
       HTTP_MINOR_TYPE_AAC_ADIF == tInfo.minorType ||
       HTTP_MINOR_TYPE_AAC_LOAS == tInfo.minorType)
    {
      AacCodecData aacCodecData;
      if ( pFileSource->GetAACCodecData( tInfo.nTrackID,
                                        &aacCodecData ) )
      {
        if(HTTP_AAC_FORMAT_ADTS == (HTTPAACStreamFormat)aacCodecData.eAACStreamFormat)
        {
          FileSourceConfigItem pItem;
          FileSourceStatus configStatus;

          configStatus = pFileSource->SetConfiguration(tInfo.nTrackID, &pItem, FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER);
          if(configStatus != FILE_SOURCE_SUCCESS)
          {
            status = HTTPCommon::HTTPDL_ERROR_ABORT;
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "SetConfiguration  STRIP_AUDIO_HEADER failed for track %u", tInfo.nTrackID );
          }
          else
          {
            configStatus = pFileSource->SetConfiguration(tInfo.nTrackID, &pItem, FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
            if(configStatus != FILE_SOURCE_SUCCESS)
            {
              status = HTTPCommon::HTTPDL_ERROR_ABORT;
              QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "SetConfiguration  OUTPUT_SINGLE_AUDIO_FRAME failed for track %u", tInfo.nTrackID );
            }
          }
        }
      }
      else
      {
           QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                          "Get AAC code info failed for track %u", tInfo.nTrackID );
      }
    }
    else if (HTTP_MINOR_TYPE_MP2 == tInfo.minorType)
    {
      FileSourceConfigItem pItem;
      FileSourceStatus configStatus;

      configStatus = pFileSource->SetConfiguration(tInfo.nTrackID, &pItem, FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
      if(configStatus != FILE_SOURCE_SUCCESS)
      {
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "SetConfiguration  OUTPUT_SINGLE_AUDIO_FRAME failed for track %u", tInfo.nTrackID );
      }
    }
  }
  else
  {
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Get HTTP_AUDIO_TYPE trackInfo failed or pFileSource = NULL");
  }

  return status;
}

/*
 * Checks if the reads may be made on the resource.
 *
 * @param[in]  majorType the media type for the request.
 *
 * @return HTTPDL_SUCCESS on success.
 *         HTTPDL_WAITING resource is not ready.
 *         HTTPDL_DATA_END reads have been completed on this resource
 *         HTTPDL_UNSUPPORTED the resource does not support the operation
 */
HTTPDownloadStatus HTTPResource::IsResourceReadable
(
  HTTPMediaType majorType
)
{
  HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_DATA_END;

  if ( majorType != HTTPCommon::HTTP_UNKNOWN_TYPE )
  {
    bool isMajorTypeFound = false;
    MM_CriticalSection_Enter(m_resourceDataLock);
    for ( uint32 i = 0; i < m_nNumOfTracks; i++ )
    {
      if ( m_pTrackMap[i].m_majorType == majorType  )
      {
        isMajorTypeFound = true;

        if ( m_pTrackMap[i].m_bEndOfStream == false )
        {
          eReturn = HTTPCommon::HTTPDL_SUCCESS;
        }
        break;
      }
    }
    MM_CriticalSection_Leave(m_resourceDataLock);

    if (false == isMajorTypeFound)
    {
      // major type does not exist in this resource. Eg, looking for a
      // video track in audio only clip. Return HTTPDL_UNSUPPORTED
      // so caller will try to iterate furthur.
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "Did not find major type %d in resource %u",
                    majorType, (uint32)m_nKey);
      eReturn = HTTPCommon::HTTPDL_UNSUPPORTED;
    }
  }
  else
  {
    eReturn = HTTPCommon::HTTPDL_SUCCESS;
  }

  return eReturn;
}
/* @brief - Sets end of stream for the given media type.
 * @param - major type for which end of stream should be set
 *          If major type is unknown set end of stream on all
 *          tracks
 */
void HTTPResource::SetEndOfStream(HTTPMediaType majorType)
{
  MM_CriticalSection_Enter(m_resourceDataLock);
  if(majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
  {
    for ( uint32 i = 0; i < m_nNumOfTracks; i++ )
    {
      m_pTrackMap[i].m_bEndOfStream = true;
    }

  }
  else
  {
    HTTPResourceTrackMap *pTrackMap = GetTrackMapByMediaType(majorType);
    if(pTrackMap)
    {
      pTrackMap->m_bEndOfStream = true;
    }
  }
  MM_CriticalSection_Leave(m_resourceDataLock);
  return;
}

/* @brief - Sets closed flag for the given media type.
 * @param - major type for which close flaf should be set
 *          If major type is unknown set closeflag on all
 *          tracks
 */
void HTTPResource::SetTrackClosed(HTTPMediaType majorType)
{
  MM_CriticalSection_Enter(m_resourceDataLock);
  if(majorType == HTTPCommon::HTTP_UNKNOWN_TYPE)
  {
    for ( uint32 i = 0; i < m_nNumOfTracks; i++ )
    {
      m_pTrackMap[i].m_bClosed = true;
    }

  }
  else
  {
    HTTPResourceTrackMap *pTrackMap = GetTrackMapByMediaType(majorType);
    if(pTrackMap)
    {
      pTrackMap->m_bClosed = true;
    }
  }
  MM_CriticalSection_Leave(m_resourceDataLock);
  return;
}


/*
 * flushes the data
 *
 * @param[in] mediaType the media type for which the data needs to be
              flushed
 * @param[in] starttime  time before which data needs to be flushed
 *
 * @return HTTPDL_SUCCESS on success
 */
uint64 HTTPResource::GetMinFlushTime()
{
  uint64 minFlushTime = (uint64)MAX_UINT64_VAL;
  if(m_pTrackMap)
  {
    for(uint32 i=0 ;i< m_nNumOfTracks; i++)
    {
      if(m_pTrackMap[i].m_nFlushTime < minFlushTime)
      {
        minFlushTime = m_pTrackMap[i].m_nFlushTime;
      }
    }
  }
  if(minFlushTime == (uint64)MAX_UINT64_VAL)
  {
    minFlushTime = m_nFlushTime;
  }

  return minFlushTime;
}
void HTTPResource::SetFlushTime(HTTPMediaType majorType,int64 nStartTime)
{
  if(m_pTrackMap)
  {
    m_nFlushTime = 0;
    for(uint32 i=0 ;i< m_nNumOfTracks; i++)
    {
      if(majorType == HTTPCommon::HTTP_UNKNOWN_TYPE ||
        m_pTrackMap[i].m_majorType == majorType)
      {
        m_pTrackMap[i].m_nFlushTime = nStartTime;
      }
    }
  }
  else
  {
    m_nFlushTime = nStartTime;
  }
}
/*
 * cleans up the resource, brings it back to init state
 *
 * @return HTTPDL_SUCCESS on success else failure
 *
 */
HTTPDownloadStatus HTTPResource::Close
(
  HTTPMediaType /* majorType */
)
{
  HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_SUCCESS;
  if ( m_pFileSource )
  {
    //close the file
    FileSourceStatus eStatus = m_pFileSource->CloseFile();
    if ( FILE_SOURCE_FAILED(eStatus) )
    {
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HTTPResource [%llu %llu] close file failed %p",
                     m_nKey >> 32, m_nKey, (uint32 *)eStatus );
      eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }

  if ( m_pTrackMap )
  {
    QTV_Free(m_pTrackMap);
    m_pTrackMap = NULL;
  }


  m_nNumOfTracks = 0;
  m_nFlushTime = 0;

  return eReturn;
}


/*
 * cleans up the resource, brings it back to init state
 *
 * @return HTTPDL_SUCCESS on success else failure
 *
 */
HTTPDownloadStatus HTTPResource::Close
(
  void
)
{
  HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_SUCCESS;
  if ( m_pFileSource )
  {
    //close the file
    FileSourceStatus eStatus = m_pFileSource->CloseFile();
    if ( FILE_SOURCE_FAILED(eStatus) )
    {
      QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HTTPResource [%llu %llu] close file failed %p",
                     m_nKey >> 32, m_nKey, (uint32 *)eStatus );
      eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }

  if ( m_pDataManager )
  {
    //flush the data and close data manager
    if ( m_pDataManager->Flush() != HTTPCommon::HTTPDL_SUCCESS)
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HTTPResource [%llu %llu] flush data manager failed",
                     m_nKey >> 32, m_nKey );
      eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
    (void)m_pDataManager->Close();
  }

  if ( m_pTrackMap )
  {
    QTV_Free(m_pTrackMap);
    m_pTrackMap = NULL;
  }



  m_nNumOfTracks = 0;
  m_nFlushTime = 0;

  return eReturn;
}

/*
 * method to retrieve all valid track information. Pass NULL for trackIdInfo
 * to get total number of valid tracks. Allocate the memory and call again
 * with non NULL pointer.
 *
 * @param[out] pTrackInfo Pointer to HTTPMediaTrackInfo,
 *
 * @return  number of valid tracks.
 */
uint32 HTTPResource::GetMediaTrackInfo
(
  HTTPMediaTrackInfo *pTrackInfo
)
{
  uint32 nNumTracks = 0;
  if ( pTrackInfo == NULL )
  {
    // query for the size
    nNumTracks = m_nNumOfTracks;
  }
  else
  {
    for ( uint32 i = 0; i < m_nNumOfTracks; i++ )
    {
      if ( pTrackInfo + i )
      {
        if ( GetSelectedMediaTrackInfo(m_pTrackMap[i].m_majorType,
                                       pTrackInfo[i]) != true )
        {
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Get trackInfo for index %u id %u failed", i,
                         m_pTrackMap[i].m_nTrackID );
          break;
        }
        nNumTracks++;
      }
      else
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Invalid track info failed for index %u", i );
      }
    }
  }
  return nNumTracks;
}

/*
 * method to retrieve track information for particular track.
 *
 * @param[in]  majorType the media type for the request
 * @param[out] TrackInfo populated the track information on success
 *
 * @return true is successful in retrieving the track information
 * else returns false
 */
bool HTTPResource::GetSelectedMediaTrackInfo
(
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaTrackInfo &TrackInfo
)
{
  bool bResult = false;
  FileSource *pFileSource = GetFileSource();
  HTTPResourceTrackMap *pTrackMap = GetTrackMapByMediaType(majorType);
  if ( pTrackMap && pFileSource )
  {
    MediaTrackInfo mediaTrackInfo;
    // found track with matching track Id
    TrackInfo.majorType = pTrackMap->m_majorType;
    TrackInfo.bSelected = true;
    TrackInfo.nTrackID = pTrackMap->m_nTrackID;
    if ( pFileSource->GetMediaTrackInfo( pTrackMap->m_nTrackID, &mediaTrackInfo ) ==
         FILE_SOURCE_SUCCESS )
    {
      bResult = true;
      if ( pTrackMap->m_majorType == HTTPCommon::HTTP_AUDIO_TYPE )
      {
        TrackInfo.minorType =
          GetHTTPMediaMinorType(mediaTrackInfo.audioTrackInfo.audioCodec);
        TrackInfo.audioTrackInfo.bitRate =
          mediaTrackInfo.audioTrackInfo.bitRate;
        TrackInfo.audioTrackInfo.duration =
          mediaTrackInfo.audioTrackInfo.duration;
        TrackInfo.audioTrackInfo.maxBitRate =
          mediaTrackInfo.audioTrackInfo.maxBitRate;
        TrackInfo.audioTrackInfo.minBitRate =
          mediaTrackInfo.audioTrackInfo.minBitRate;
        TrackInfo.audioTrackInfo.nBitsPerSample =
          mediaTrackInfo.audioTrackInfo.nBitsPerSample;
        TrackInfo.audioTrackInfo.nBlockAlign =
          mediaTrackInfo.audioTrackInfo.nBlockAlign;
        TrackInfo.audioTrackInfo.numChannels =
          mediaTrackInfo.audioTrackInfo.numChannels;
        TrackInfo.audioTrackInfo.samplingRate =
          mediaTrackInfo.audioTrackInfo.samplingRate;
        TrackInfo.audioTrackInfo.timeScale =
          mediaTrackInfo.audioTrackInfo.timeScale;
        TrackInfo.audioTrackInfo.maxSampleSize =
          pFileSource->GetTrackMaxFrameBufferSize(pTrackMap->m_nTrackID);
        StoreDRMInfo(pFileSource, TrackInfo.nTrackID, &TrackInfo.audioTrackInfo.drmInfo);
      }
      else if ( pTrackMap->m_majorType == HTTPCommon::HTTP_VIDEO_TYPE )
      {
        TrackInfo.minorType =
          GetHTTPMediaMinorType(mediaTrackInfo.videoTrackInfo.videoCodec);
        TrackInfo.videoTrackInfo.bitRate =
          mediaTrackInfo.videoTrackInfo.bitRate;
        TrackInfo.videoTrackInfo.duration =
          mediaTrackInfo.videoTrackInfo.duration;
        TrackInfo.videoTrackInfo.frameHeight =
          mediaTrackInfo.videoTrackInfo.frameHeight;
        TrackInfo.videoTrackInfo.frameRate =
          mediaTrackInfo.videoTrackInfo.frameRate;
        TrackInfo.videoTrackInfo.frameWidth =
          mediaTrackInfo.videoTrackInfo.frameWidth;
        TrackInfo.videoTrackInfo.layer =
          mediaTrackInfo.videoTrackInfo.layer;
        TrackInfo.videoTrackInfo.timeScale =
          mediaTrackInfo.videoTrackInfo.timeScale;
        TrackInfo.videoTrackInfo.maxSampleSize =
          pFileSource->GetTrackMaxFrameBufferSize(pTrackMap->m_nTrackID);
        StoreDRMInfo(pFileSource, TrackInfo.nTrackID,  &TrackInfo.videoTrackInfo.drmInfo);
      }
      else if ( pTrackMap->m_majorType == HTTPCommon::HTTP_TEXT_TYPE )
      {
        TrackInfo.minorType =
          GetHTTPMediaMinorType(mediaTrackInfo.textTrackInfo.textCodec);
        TrackInfo.textTrackInfo.layer = mediaTrackInfo.textTrackInfo.layer;
        TrackInfo.textTrackInfo.timeScale = mediaTrackInfo.textTrackInfo.timeScale;
        TrackInfo.textTrackInfo.duration = mediaTrackInfo.textTrackInfo.duration;
        TrackInfo.textTrackInfo.nHeight = mediaTrackInfo.textTrackInfo.frameHeight;
        TrackInfo.textTrackInfo.nWidth = mediaTrackInfo.textTrackInfo.frameWidth;
        TrackInfo.textTrackInfo.maxSampleSize =
           pFileSource->GetTrackMaxFrameBufferSize(pTrackMap->m_nTrackID);
        StoreDRMInfo(pFileSource, TrackInfo.nTrackID,  &TrackInfo.textTrackInfo.drmInfo);
      }
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Get track info failed for track %u", pTrackMap->m_nTrackID );
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid resource state %p %p", (void *)pTrackMap,
                   (void *)pFileSource );
  }
  return bResult;
}

/*
 * Query Timescale from file source and update.
 *
 * @return true is successful in retrieving the track information
 * else returns false
 */
uint64 HTTPResource::getTimeScale
(
  HTTPCommon::HTTPMediaType majorType,
  int32 trackID
)
{
  uint64 timescale = 0;
  FileSource *pFileSource = GetFileSource();
  if ( pFileSource )
  {
    MediaTrackInfo mediaTrackInfo;
    if ( pFileSource->GetMediaTrackInfo( trackID, &mediaTrackInfo ) ==
         FILE_SOURCE_SUCCESS )
    {
       if ( majorType == HTTPCommon::HTTP_AUDIO_TYPE )
      {
        timescale = mediaTrackInfo.audioTrackInfo.timeScale;
      }
      else if ( majorType == HTTPCommon::HTTP_VIDEO_TYPE )
      {
        timescale = mediaTrackInfo.videoTrackInfo.timeScale;
      }
      else if ( majorType == HTTPCommon::HTTP_TEXT_TYPE )
      {
        timescale = mediaTrackInfo.textTrackInfo.timeScale;
      }
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "getTimeScale info failed for track %lu", trackID );
    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "getTimeScale Invalid resource state %p", (void *)pFileSource );
  }
  return (uint64)timescale;
}

/*
 * Retrieve the codec specfic data needed to configure decoder
 *
 * @param[in] nTrackID   Identifies the track for which codec data needs to
 * be retrieved
 * @param[in] minorType   media minor type for which the codec info is being
 * requested
 * @param[out] AACCodecData populates the codec data on success
 *
 * @return true is successful in retrieving the codec data else returns false
 */
bool HTTPResource::GetCodecData
(
  uint32 nTrackID,
  HTTPMediaMinorType minorType,
  HTTPCodecData &CodecData
)
{
  bool bResult = false;
  FileSource *pFileSource  = GetFileSource();
  if ( pFileSource )
  {
    // found track with matching track Id
    switch( minorType )
    {
    case HTTP_MINOR_TYPE_AAC:
    case HTTP_MINOR_TYPE_AAC_ADTS:
    case HTTP_MINOR_TYPE_AAC_ADIF:
    case HTTP_MINOR_TYPE_AAC_LOAS:
      {
        AacCodecData aacCodecData;
        if ( pFileSource->GetAACCodecData( nTrackID,
                                           &aacCodecData ) )
        {
          // it is one to one mapping
          CodecData.aacCodecData.eAACProfile =
            (HTTPAACProfile)aacCodecData.ucAACProfile;
          // it is one to one mapping
          CodecData.aacCodecData.eAACStreamFormat =
            (HTTPAACStreamFormat)aacCodecData.eAACStreamFormat;
          bResult =  true;
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Get AAC code info failed for track %u", nTrackID );
        }
      }
      break;
    default:
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "No codec info avaliable for minortype %d track id %u",
                     minorType, nTrackID );
      break;
    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid resource state %p",
                   (void *)pFileSource );
  }
  return bResult;
}

/*
 * Method retrieves the Decoder specific/Format Block information for the
 * given track. If buf = NULL, then the function give the size of the required
 * buffer/format block.
 *
 * Following is an example of retrieving the format block.
 * 1. Invoke getFormatBlock for a given track identifier by passing in
 *    NULL for buf.
 * 2. If a track is valid, *pbufSize will give you the size of format block
 *    that exist.
 * 3  Allocate the memory and invoke getFormatBlock API for a given track
 *    identifier by passing handle to allocated memory.
 *
 * @param[in] majorType media type.
 * @param[out] pBuffer  Buffer provies the format block info to the caller
 * @param[out] pbufSize Size of the FormatBlock buffer
 *
 * @return true if successful in retrieving the format block else failure
 */
bool HTTPResource::GetFormatBlock
(
  HTTPCommon::HTTPMediaType majorType,
  uint8* pBuffer,
  uint32 &nBufSize
)
{
  bool bStatus = false;
  FileSource *pFileSource  = GetFileSource();
  HTTPResourceTrackMap *pTrackMap = GetTrackMapByMediaType(majorType);
  if ( pTrackMap && pFileSource )
  {
    FileSourceStatus fsStatus = pFileSource->GetFormatBlock(
      pTrackMap->m_nTrackID , pBuffer, &nBufSize );
    if ( fsStatus != FILE_SOURCE_SUCCESS )
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Unable to get the format block %d for track %d",
                     fsStatus,  pTrackMap->m_nTrackID );
    }
    else
    {
      bStatus = true;
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid resource state %p %p", (void *)pTrackMap, (void *)pFileSource );
  }
  return bStatus;
}

/*
* Get the next media sample for the specified media type
*
* @param[in] mediaType media major type.
* @param[out]pBuffer  A pointer to the buffer into which to place the sample.
* @param[out] nSize The size of the data buffer.
* @param[out] sampleInfo Provides information about the sample
*
* @return HTTPDL_SUCCESS if successful in retrieving the format block
* HTTPDL_WAITING the information is not avaliable check back later
* else failure
*/
HTTPCommon::HTTPDownloadStatus HTTPResource::GetNextMediaSample
(
  HTTPCommon::HTTPMediaType mediaType,
  uint8 *pBuffer,
  uint32 &nSize,
  HTTPSampleInfo &httpSampleInfo
)
{
  HTTPCommon::HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

    FileSource *pFileSource  = GetFileSource();
    HTTPResourceTrackMap *pTrackMap = GetTrackMapByMediaType(mediaType);
    if ( pTrackMap && pFileSource && (pTrackMap->m_nMediaTimeScale > 0))
    {
      FileSourceSampleInfo fsSampleInfo;

      //Reset the Sample Info structure
      memset(&fsSampleInfo, 0, sizeof(fsSampleInfo));

      FileSourceMediaStatus fsStatus;
      // found track with matching track Id
      fsStatus = pFileSource->GetNextMediaSample(pTrackMap->m_nTrackID,
                                                 pBuffer, &nSize,
                                                 fsSampleInfo);
      if(pTrackMap->m_nPrevSampleTS > 0 &&
          fsSampleInfo.startTime < pTrackMap->m_nPrevSampleTS &&
         (pTrackMap->m_nPrevSampleTS - fsSampleInfo.startTime) > WRAP_AROUND_THREASHOLD)
      {
         httpSampleInfo.startTime = (uint64)((((uint32)(1<<31) + fsSampleInfo.startTime) * DASH_TIMESCALE) / pTrackMap->m_nMediaTimeScale);
         httpSampleInfo.endTime = (uint64)((((uint32)(1<<31) + fsSampleInfo.endTime)  * DASH_TIMESCALE) / pTrackMap->m_nMediaTimeScale);
      }
      else
      {
        httpSampleInfo.endTime = (fsSampleInfo.endTime * DASH_TIMESCALE) / pTrackMap->m_nMediaTimeScale;
        httpSampleInfo.startTime = (fsSampleInfo.startTime * DASH_TIMESCALE) / pTrackMap->m_nMediaTimeScale;
      }
      pTrackMap->m_nPrevSampleTS = fsSampleInfo.startTime;
      httpSampleInfo.bStartTsValid = fsSampleInfo.bStartTsValid;
      httpSampleInfo.delta = fsSampleInfo.delta;
      httpSampleInfo.sync = fsSampleInfo.sync;
      memset(&httpSampleInfo.sSubInfo.sContentProtection, 0, sizeof(httpSampleInfo.sSubInfo.sContentProtection));
      if (fsSampleInfo.sSubInfo.sCP.ucIsEncrypted)
      {
        memcpy((HTTPContentProtectionType *)&httpSampleInfo.sSubInfo.sContentProtection,
            (FS_CONTENT_PROTECTION_INFOTYPE *)&fsSampleInfo.sSubInfo.sCP,
            sizeof(HTTPContentProtectionType));
      }

      /* For Text Track fill in the subsample info if present */
      if(mediaType == HTTPCommon::HTTP_TEXT_TYPE &&
          fsSampleInfo.sSubInfo.sSubTitle.ulSubSampleCount > 0)
      {
        HTTPMediaMinorType subsMinorType = GetHTTPMediaMinorType(fsSampleInfo.sSubInfo.sSubTitle.eSubMnType);

        if(subsMinorType == HTTP_MINOR_TYPE_PNG)
        {
          httpSampleInfo.sSubInfo.sSubTitle.eSubMnType = subsMinorType;
          httpSampleInfo.sSubInfo.sSubTitle.subSampleCount = fsSampleInfo.sSubInfo.sSubTitle.ulSubSampleCount;
          httpSampleInfo.sSubInfo.sSubTitle.subtitleSubInfoSize = fsSampleInfo.sSubInfo.sSubTitle.usSubtitleSubInfoSize;
          memcpy(httpSampleInfo.sSubInfo.sSubTitle.cSubtitleSubInfo, fsSampleInfo.sSubInfo.sSubTitle.ucSubtitleSubInfo,
                 httpSampleInfo.sSubInfo.sSubTitle.subtitleSubInfoSize);
        }
      }

      switch( fsStatus )
      {
      case FILE_SOURCE_DATA_OK:
        eStatus = video::HTTPCommon::HTTPDL_SUCCESS;
        break;
      case FILE_SOURCE_DATA_END:
        pTrackMap->m_bEndOfStream = true;
        eStatus = video::HTTPCommon::HTTPDL_DATA_END;
        break;
      case FILE_SOURCE_DATA_UNDERRUN:
      case FILE_SOURCE_DATA_INSUFFICIENT:
        eStatus = video::HTTPCommon::HTTPDL_WAITING;
        break;
      case FILE_SOURCE_DATA_FRAGMENT:
        eStatus = HTTPCommon::HTTPDL_SEGMENT_BOUNDARY;
        break;
      default:
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Unhandled error %d for trackID %d", fsStatus,
                       pTrackMap->m_nTrackID );
        break;
    }
  }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid resource state %p %p", (void *)pTrackMap, (void *)pFileSource );
    }

  return eStatus;
}

/*
 * Get the current playback position for the specified media type
 *
 * @param[in] HTTPMediaType media type.
 * @param[out] nPlaybackPosition populates in time uints on success
 *
 * @return true if successful else failure
 */
bool HTTPResource::GetCurrentPlaybackPosition
(
 HTTPCommon::HTTPMediaType mediaType,
 uint64 &nPlaybackPosition
)
{
  bool bStatus = false;
  FileSource *pFileSource  = GetFileSource();
  HTTPResourceTrackMap *pTrackMap = GetTrackMapByMediaType(mediaType);
  if ( pTrackMap && pFileSource && (pTrackMap->m_nMediaTimeScale > 0))
  {
    nPlaybackPosition =
    (pFileSource->GetMediaCurrentPosition(pTrackMap->m_nTrackID) * DASH_TIMESCALE) / pTrackMap->m_nMediaTimeScale;
    nPlaybackPosition = STD_MAX(nPlaybackPosition,(uint64)m_nSeekTime);
    bStatus = true;
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid resource state %p %p",(void *) pTrackMap,(void *) pFileSource );
    // using defaults, since no reads have been done yet
    nPlaybackPosition = STD_MAX(0,m_nSeekTime);
  }

  return bStatus;
}

/*
 * returns if reads have been complete on all tracks on this resource
 *
 * @return true on success else false
 */
bool HTTPResource::ReadComplete
(
  void
)
{
  bool bReturn = false;

  MM_CriticalSection_Enter(m_resourceDataLock);
  if ( m_nNumOfTracks > 0 )
  {
    /* verify if all tracks have been released too */
    int nTracksReleased = 0;
    for ( uint32 i = 0; i < m_nNumOfTracks; i++ )
    {
      if ( m_pTrackMap[i].m_bEndOfStream )
      {
        nTracksReleased++;
      }
    }
    if ( (int)m_nNumOfTracks == nTracksReleased )
    {
      /* the resource can be deleted */
      bReturn = true;
    }
  }
  MM_CriticalSection_Leave(m_resourceDataLock);

  return bReturn;
}


/*
 * returns if close have been complete on all tracks on this resource
 *
 * @return true on success else false
 */
bool HTTPResource::CloseComplete
(
  void
)
{
  bool bReturn = false;

  MM_CriticalSection_Enter(m_resourceDataLock);
  if ( m_nNumOfTracks > 0 )
  {
    /* verify if all tracks have been closed */
    int nTracksClosed = 0;
    for ( uint32 i = 0; i < m_nNumOfTracks; i++ )
    {
      if ( m_pTrackMap[i].m_bClosed )
      {
        nTracksClosed++;
      }
    }
    if ( (int)m_nNumOfTracks == nTracksClosed )
    {
      /*Close is called for all the tracks, so filesource can
        closed now */
      bReturn = true;
    }
  }
  else
  {
    //Filesource open is not yet called or complete, so number of tracks will be 0.
    //Allow close on filesource to go through.
    bReturn = true;
  }
  MM_CriticalSection_Leave(m_resourceDataLock);

  return bReturn;
}

/*
 * Returns the minimum offset that has been read across all the avaliable
 * media tracks.
 *
 * @param[out] nMediaOffset the minimum media offset on success
 *
 * @return true on success else failure
 */
bool HTTPResource::GetMinimumMediaFileOffset
(
 uint64 &nMediaOffset
)
{
  bool bReturn = false;
  uint64 nMediaOffsetByTrack = 0;

  MM_CriticalSection_Enter(m_resourceDataLock);
  for ( uint32 i = 0; i < m_nNumOfTracks; i++ )
  {
    if(m_pTrackMap[i].m_bEndOfStream == false)
    {
      bool bError = false;
      nMediaOffsetByTrack =
      m_pFileSource->GetLastRetrievedSampleOffset(
        m_pTrackMap[i].m_nTrackID, &bError);

      if ( bError == true )
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "get the last retrieved sample offset failed for %d",
                        m_pTrackMap[i].m_nTrackID );
        break;
      }
      else
      {
        nMediaOffset = (nMediaOffset == 0)? nMediaOffsetByTrack:
                        STD_MIN(nMediaOffset, nMediaOffsetByTrack);
        bReturn = true;
      }
    }
    else
    {
      bReturn = true;
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "EOS detected on track %d ignore the minoffset",
                      m_pTrackMap[i].m_nTrackID );
    }
  }
  MM_CriticalSection_Leave(m_resourceDataLock);
  return bReturn;
}

/*
 * Get the media offset for the given time
 *
 * @param[in] nTime the time for wehihc offset is requested
 * @param[out] nOffset the offset on success
 *
 * @return true on success else failure
 */
bool HTTPResource::GetFileOffsetForTime
(
 const int64 nTime,
 int64& nOffset
)
{
  bool bReturn = false;

  MM_CriticalSection_Enter(m_resourceDataLock);
  bReturn = m_pFileSource->GetOffsetForTime((uint64)nTime, (uint64*)&nOffset);
  MM_CriticalSection_Leave(m_resourceDataLock);

  return bReturn;
}
/*
 * Gets the base time from the filesource
 *
 * @param[out] baseTime - baseTime of the first readable resource. basetime is
 * returned 0 if first resource is not readable.
 *
 * @return
 */
 bool HTTPResource::GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime)
{
  int64 tmpBaseTime = -1;
  bool bRet = false;
  segMediaTime = 0;
  segMPDTime = 0;
  QTV_NULL_PTR_CHECK(m_pFileSource,bRet);
  FileSourceConfigItem fileSourceConfigItem;
  //Get the minimum base time over all the tracks
  for(uint32 i=0;i<m_nNumOfTracks;i++)
  {
    FileSourceStatus fsStatus =  m_pFileSource->GetConfiguration(
                    m_pTrackMap[i].m_nTrackID,&fileSourceConfigItem,FILE_SOURCE_MEDIA_BASETIME);
    if(fsStatus == FILE_SOURCE_SUCCESS)
    {
      tmpBaseTime = tmpBaseTime >= 0 ? STD_MIN(tmpBaseTime, (int64)fileSourceConfigItem.nresult) :
                                        fileSourceConfigItem.nresult;
      bRet = true;
    }
    else
    {
      bRet = false;
      break;
    }
  }
  if(bRet)
  {
    segMediaTime = tmpBaseTime;
    segMPDTime = tmpBaseTime;
  }
  return bRet;
}
 /*
 * Sets the given base time on all filesource
 *
 * @param[in] baseTime - baseTime to be set.
 *
 * @return
 */
void HTTPResource::SetBaseTime(uint64 baseTime)
{
  QTV_NULL_PTR_CHECK(m_pFileSource,RETURN_VOID);
  FileSourceConfigItem fileSourceConfigItem;
  fileSourceConfigItem.nresult = baseTime;
  for(uint32 i=0;i<m_nNumOfTracks;i++)
  {
    m_pFileSource->SetConfiguration(m_pTrackMap[i].m_nTrackID,&fileSourceConfigItem,FILE_SOURCE_MEDIA_BASETIME);
  }
  return;
}

/*
 * Get play time offset for a given byte offset
 *
 * @param[in] majorType the media type for the request.
 * @param[in] nOffset byte offset in the clip
 * @param[out] nTimeOffset returned time offset on success
 *
 * @return true on success else false on failure
 */
bool HTTPResource::GetTimeForFileOffset
(
 HTTPMediaType majorType,
 uint64 nOffset,
 uint64 &nTimeOffset
)
{
  bool bStatus = false;
  nTimeOffset = 0;
  FileSource *pFileSource  = GetFileSource();
  HTTPResourceTrackMap *pTrackMap = GetTrackMapByMediaType(majorType);
  if ( pTrackMap && pFileSource )
  {
    FileSourceStatus fsStatus =
      pFileSource->GetBufferedDuration( pTrackMap->m_nTrackID, (int64)nOffset, &nTimeOffset );
    bStatus = (fsStatus == FILE_SOURCE_SUCCESS);
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid resource state %p %p", (void *)pTrackMap, (void *)pFileSource );
  }

  return bStatus;
}

/*
 * Get the duration buffered for the specified media type
 *
 * @param[in] HTTPMediaType media type
 * @param[out] nDuration populates in time units on success
 *
 * @return true if successful else failure
 */
bool HTTPResource::GetDurationBuffered
(
 HTTPMediaType majorType,
 uint64& nDuration
)
{
  bool bStatus = false;
  nDuration = 0;
  FileSource *pFileSource  = GetFileSource();
  HTTPResourceTrackMap *pTrackMap = GetTrackMapByMediaType(majorType);
  if ( pTrackMap && pFileSource )
  {
    FileSourceStatus fsStatus =
      pFileSource->GetBufferedDuration(pTrackMap->m_nTrackID, &nDuration);
    bStatus = (fsStatus == FILE_SOURCE_SUCCESS);
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid resource state %p %p", (void *)pTrackMap, (void *)pFileSource );
  }

  return bStatus;
}

/*
 * Storing DRM specific info
 *
 * @param[in] pFileSource - pointer of associated file source
 * @param[in] trackId     - associated track id for query
 * @param[in/out] drmInfo - DRM info structure
 *
 * @return true on success else false on failure
 */

bool HTTPResource::StoreDRMInfo
(
  FileSource *pFileSource, uint32 trackId, HTTPDrmInfo *drmInfo
)
{
  bool bStatus = true;

  if (pFileSource  && drmInfo)
  {
    if (pFileSource->IsDRMProtection())
    {

      FileSourceConfigItem fileSourceConfigItem;
      fileSourceConfigItem.nresult = 0;
      if (drmInfo->eDrmType == FILE_SOURCE_NO_DRM)
      {
        FileSourceStatus fsStatus =  pFileSource->GetConfiguration(
                 trackId ,&fileSourceConfigItem, FILE_SOURCE_MEDIA_NUM_DRM_SYSTEM_SUPPORTED);
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                  "Number of DRM supported %llu", fileSourceConfigItem.nresult );
        if(fsStatus == FILE_SOURCE_SUCCESS)
        {
          FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE *pPSSHInfo
              = (FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE *)
                 QTV_Malloc((size_t)fileSourceConfigItem.nresult * sizeof(FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE));
          if (pPSSHInfo)
          {
            pFileSource->GetProtectionSystemSpecificInfo(trackId, pPSSHInfo);
            for (uint32 drmIndex = 0; drmIndex < fileSourceConfigItem.nresult; drmIndex++)
            {
              if (pPSSHInfo[drmIndex].ePSDRMType == FILE_SOURCE_MARLIN_DRM ||
                  pPSSHInfo[drmIndex].ePSDRMType == FILE_SOURCE_CENC_DRM ||
                  pPSSHInfo[drmIndex].ePSDRMType == FILE_SOURCE_PLAYREADY_DRM )
              {
                if (pPSSHInfo[drmIndex].ucSystemID)
                {
                  memcpy(drmInfo->systemID,
                         (const char*)pPSSHInfo[drmIndex].ucSystemID,
                         MAX_SYSTEMID_SIZE);
                  drmInfo->systemIdBufSize = MAX_SYSTEMID_SIZE;
                }
                drmInfo->isDRMProtected = true;
                drmInfo->ulDRMIndex = pPSSHInfo[drmIndex].ulDRMIndex;
                drmInfo->eDrmType = pPSSHInfo[drmIndex].ePSDRMType;
                drmInfo->kidCount = pPSSHInfo[drmIndex].ulKIDCount;
                drmInfo->kidDataBufSize = pPSSHInfo[drmIndex].ulKIDDataSize;
                drmInfo->psshDataBufSize = pPSSHInfo[drmIndex].ulPSSHDataSize;
                drmInfo->defaultKeyIDSize = 0;
                break;
              }/* if (pPSSHInfo[drmIndex].ePSSDRMType == FILE_SOURCE_MARLIN_DRM)*/
            } /*for */
            QTV_Free(pPSSHInfo);
            pPSSHInfo = NULL;
          }/*if (pPSSHInfo)*/
        }
      }
      else if (drmInfo->eDrmType == FILE_SOURCE_MARLIN_DRM ||
               drmInfo->eDrmType == FILE_SOURCE_CENC_DRM ||
               drmInfo->eDrmType == FILE_SOURCE_PLAYREADY_DRM )
      {
        FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE *pPSSHdata =
                (FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE *)
                 QTV_Malloc(sizeof(FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE));

        if (pPSSHdata)
        {
          pPSSHdata->ulDRMIndex = drmInfo->ulDRMIndex;
          pPSSHdata->ulKIDDataBufSize = drmInfo->kidDataBufSize;
          pPSSHdata->ulPSSHDataBufSize = drmInfo->psshDataBufSize;
          pPSSHdata->pucKIDDataBuf = NULL;
          pPSSHdata->pucPSSHDataBuf = NULL;

          // allocating pucKIDDataBuf
          if (pPSSHdata->ulKIDDataBufSize > 0)
          {
            pPSSHdata->pucKIDDataBuf = (uint8 *)QTV_Malloc(pPSSHdata->ulKIDDataBufSize + 1);
            if (!pPSSHdata->pucKIDDataBuf)
            {
               bStatus = false;
            }
          }

          // allocating pucPSSHDataBuf
          if (pPSSHdata->ulPSSHDataBufSize > 0)
          {
            pPSSHdata->pucPSSHDataBuf = (uint8 *)QTV_Malloc(pPSSHdata->ulPSSHDataBufSize + 1);
            if (!pPSSHdata->pucPSSHDataBuf)
            {
               bStatus = false;
            }
          }

          if (bStatus == true)
          {
            pFileSource->GetProtectionSystemSpecificData(trackId, pPSSHdata);
            if (drmInfo->pKidDataBuf && pPSSHdata->pucKIDDataBuf)
            {
              memset(drmInfo->pKidDataBuf, 0, drmInfo->kidDataBufSize + 1);
              memcpy(drmInfo->pKidDataBuf,
                         (const char*)pPSSHdata->pucKIDDataBuf,
                         drmInfo->kidDataBufSize);
            }


            if (drmInfo->pPsshDataBuf && pPSSHdata->pucPSSHDataBuf)
            {
              memset(drmInfo->pPsshDataBuf, 0,  drmInfo->psshDataBufSize + 1);
              memcpy(drmInfo->pPsshDataBuf,
                          (const char*)pPSSHdata->pucPSSHDataBuf,
                          drmInfo->psshDataBufSize);
            }

            memcpy(drmInfo->defaultKeyID,
                        (const char*)pPSSHdata->ucDefaultKeyID,
                         MAX_KID_SIZE);
            drmInfo->defaultKeyIDSize = MAX_KID_SIZE;
          }
          if (pPSSHdata->pucKIDDataBuf)
          {
            QTV_Free(pPSSHdata->pucKIDDataBuf);
            pPSSHdata->pucKIDDataBuf= NULL;
          }
          if (pPSSHdata->pucPSSHDataBuf)
          {
            QTV_Free(pPSSHdata->pucPSSHDataBuf);
            pPSSHdata->pucPSSHDataBuf= NULL;
          }
          QTV_Free(pPSSHdata);
          pPSSHdata = NULL;
        }
      }
    }
    else
    {
       drmInfo->isDRMProtected = false;
    }
  }
  else
  {
    bStatus = false;
  }

  return bStatus;
}

} // namespace video
