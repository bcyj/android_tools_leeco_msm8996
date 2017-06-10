#ifndef HTTPRESOURCE_H
#define HTTPRESOURCE_H
/************************************************************************* */
/**
 * HTTPResource.h
 * @brief Defines the HTTP Resource
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPResource.h#38 $
$DateTime: 2013/07/30 18:45:48 $
$Change: 4190515 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "HTTPCommon.h"
#include "HTTPDataInterface.h"
#include "filesource.h"
#include "QsmTypes.h"
#include "MPDParser.h"

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

#define WRAP_AROUND_THREASHOLD 0x80000000
#define DASH_TIMESCALE 1000

// forward declarations
class HTTPDataManager;
class TrackList;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
// map between the track ID from FileSource and the state of the track
struct HTTPResourceTrackMap
{
  int32 m_nTrackID;
  HTTPCommon::HTTPMediaType m_majorType;
  bool m_bEndOfStream;
  bool m_bClosed;
  uint64 m_nPrevSampleTS;
  uint64 m_nFlushTime;
  uint64 m_nMediaTimeScale;
};



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

class HTTPResource : public iHTTPFileSourceHelper
{
public:
  HTTPResource(bool& bResult, MPDProfileType profileType);

  virtual ~HTTPResource();

  virtual HTTPDownloadStatus GetSegmentInfo(uint64 nStartTime,
                                            uint64 nDuration) = 0;

  virtual HTTPDownloadStatus OpenSegment() = 0;

  virtual HTTPDownloadStatus FillDataUnitInfo(uint64 nStartTime,
                                              uint64 nDuration,
                                              QSM::CDataUnitInfo* pDataUnitInfo,
                                              uint32 nNumDataUnits,
                                              uint32& nNumDataUnitsFilled) = 0;

  virtual HTTPDownloadStatus GetDataUnitDownloadInfo(QSM::CDataUnitDownloadInfo *pDownloadInfo,
                                                     uint32 nSize,
                                                     uint32 &nFilled,
                                                     uint64 nStartTime) = 0;

  virtual HTTPDownloadStatus GetSegmentData(uint64 nDataUnitKey) = 0;

  virtual HTTPDownloadStatus ContinueDownloadDataUnit(uint64 nDataUnitKey) = 0;

  virtual HTTPDownloadStatus GetDataUnitInfoByKey(uint64 nDataUnitKey,
                                                  uint64 &nStartTime,
                                                  uint64 &nDuration) = 0;

  virtual HTTPDownloadStatus Seek(uint64 nSeekTime) = 0;

  virtual HTTPDownloadStatus Select(uint64 nDataUnitKey, uint64& nPbTime) = 0;

  virtual HTTPDownloadStatus CancelSegmentData(uint64 nDataUnitKey) = 0;

  virtual HTTPDownloadStatus Flush(HTTPMediaType majorType,int64 nStartTime)=0;

  virtual void ClearBufferedData(uint64 nStartTime) = 0;

  virtual uint64 GetMinFlushTime();

  virtual HTTPDownloadStatus Open(uint64 /* nDataUnitKey */,int64 /* nStartTime */,bool /* bSeek */)
  {
    return HTTPCommon::HTTPDL_SUCCESS;
  }

  virtual HTTPDownloadStatus Close();

  virtual HTTPDownloadStatus Close(HTTPMediaType majorType);

  virtual bool GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime);

  virtual void SetBaseTime(uint64 baseTime);

  virtual void SetEndOfStream(HTTPMediaType majorType);

  virtual void SetTrackClosed(HTTPMediaType majorType);

  virtual void SetFlushTime(HTTPMediaType majorType,int64 nStartTime);

  virtual HTTPDownloadStatus IsReadable(HTTPMediaType majorType)
  {
    return IsResourceReadable(majorType);
  };

  virtual bool GetSegmentRange(uint64& nStartTime, uint64& nDuration) = 0;

  bool StoreSample(HTTPCommon::HTTPMediaType majorType,uint8 *pBuffer,
                   uint32 nSize, HTTPSampleInfo sampleInfo);

  void static _ProcessFileSourceEvent( FileSourceCallBackStatus status,
                                       void* pCbData );

  bool ReadComplete();

  bool CloseComplete();

  bool StoreDRMInfo(FileSource *pFileSource, uint32 trackId, HTTPDrmInfo *drmInfo);

   virtual void SetSegmentInfo(const SegmentInfo &cSegInfo ,const uint64 nStartTime,
                               const uint64 nDuration,const uint64 nKey, const char *url) = 0;

  virtual bool IsDownloading() = 0;

  virtual bool IsDownloadComplete() = 0;
  virtual bool IsSegErrorHappened() = 0;

  virtual bool GetStartTimeForLastDataUnit(uint64& nStartTime, double nRepEndTime,  bool &bIsLmsgSet) = 0;

  virtual void MarkSegmentComplete() = 0;

  /**
   * Mark that the resource reads have started on in. This means
   * that the fragment which has been detected as too slow cannot
   * be cancelled.
   */
  virtual void MarkResourceReadsStarted() = 0;

  /**
   * Check if reads have started on the resource. If not, then
   * disable the resource for sample reads and return true.
   *
   * @return bool
   *  true: if resource is disabled for sample reads.
   *  false: if resource is not disabled for sample reads.
   */
  virtual bool CheckReadsStartedAndDisableReads() = 0;

  /**
   * Check if this resource is disabled for sample reads.
   */
  virtual bool IsResourceReadDisabled() const = 0;

  /**
   * Re-enable sample reads on the resource in case it was
   * disabled for sample reads.
   */
  virtual void ReEnableResourceReads() = 0;

  /**
   * Disable socket reads on the data unit depending on
   * 'bIsDisabled'.
   */
  virtual bool DisableSocketReads(const uint64 nDataUnitKey, bool bIsDisabled) = 0;

  /**
   * Check to see if data segment with key 'nKey' is present in
   * this resource.
   */
  virtual bool IsDataUnit(uint64 nKey) const = 0;

  /**
   * Check to see if this data segment is available for download.
   */
  virtual bool IsSegmentAvailable() = 0;

//iHTTPFileSourceHelper methods
public:
  virtual bool GetMinimumMediaOffset(uint64& nMinOffset)
  {
    return GetMinimumMediaFileOffset(nMinOffset);
  };

  virtual bool GetOffsetForTime(const int64 nTime, int64& nOffset)
  {
    return GetFileOffsetForTime(nTime, nOffset);
  };

  virtual bool GetTimeForOffset(HTTPMediaType eMediaType, uint64 nOffset, uint64& nTimeOffset)
  {
    return GetTimeForFileOffset(eMediaType, nOffset, nTimeOffset);
  };

//iHTTPDataInterface methods
public:
  virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo *pTrackInfo );

  virtual bool GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType majorType,
                                         HTTPMediaTrackInfo &TrackInfo );

  virtual uint64 getTimeScale(HTTPCommon::HTTPMediaType majorType, int32 trackID);

  virtual bool GetCodecData(uint32 nTrackID,
                            HTTPMediaMinorType minorType,
                            HTTPCodecData &CodecData);

  virtual bool GetFormatBlock(HTTPCommon::HTTPMediaType majorType,
                              uint8* pBuffer, uint32 &nbufSize);

  virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample( HTTPCommon::HTTPMediaType majorType,
                                                             uint8 *pBuffer,
                                                             uint32 &nSize,
                                                             HTTPSampleInfo &sampleInfo);

  virtual char* GetVideoURL(size_t& size) = 0;

  virtual char* GetIPAddr(size_t& size) = 0;

  virtual bool GetCurrentPlaybackPosition( HTTPCommon::HTTPMediaType mediaType,
                                           uint64 &nPlaybackPosition);

  virtual bool GetDownloadPosition( HTTPCommon::HTTPMediaType mediaType,
                                    uint64& nDownloadPosition,
                                    bool& bIsPartiallyDownloaded) = 0;

//helper functions
public:
  virtual uint64 GetKey()
  {
    return m_nKey;
  };

  virtual void SetKey(const uint64 nKey)
  {
    m_nKey = nKey;
  };

  /**
   * Check to see if data segment with key 'nKey' is present in
   * this resource.
   */
  bool IsSegmentPresent(const uint64 nKey);

  /**
   * Gets the data unit key for the data segment following
   * 'nTooSlowDataUnitKey' if any. If not, return MAX_UNIT64_VAL.
   */
  void GetFirstCancellableDataUnit(
    const uint64 nTooSlowDataUnitKey,
    uint64& nFirstCancellableDataUnitKey);

  /**
   * Find the data unit key associated with the first data segment
   * in the resource. If none found then return MAX_UINT64_VAL.
   */
  uint64 GetFirstDataUnitKey();

protected:
  virtual void ProcessFileSourceEvent( FileSourceCallBackStatus status ) = 0;

  virtual HTTPDownloadStatus SetupTracks();

  virtual HTTPDownloadStatus SetAudioTrackProperties();

  virtual bool GetMinimumMediaFileOffset(uint64& nMediaOffset);

  virtual bool GetFileOffsetForTime(const int64 nTime, int64& nOffset);

  virtual bool GetTimeForFileOffset(HTTPMediaType majorType, uint64 nOffset, uint64& nTimeOffset);

  virtual HTTPDownloadStatus IsResourceReadable(HTTPMediaType majorType);

  virtual bool GetDurationBuffered(HTTPMediaType majorType, uint64& nDuration);

  virtual FileSource* GetFileSource()
  {
    FileSource* pFileSource;
    MM_CriticalSection_Enter(m_resourceDataLock);
    pFileSource = m_pFileSource;
    MM_CriticalSection_Leave(m_resourceDataLock);
    return pFileSource;
  };

  virtual HTTPDataManager* GetDataManager()
  {
    HTTPDataManager* pDataManager;
    MM_CriticalSection_Enter(m_resourceDataLock);
    pDataManager = m_pDataManager;
    MM_CriticalSection_Leave(m_resourceDataLock);
    return pDataManager;
  };

  virtual HTTPDataManager* GetSidxDataManager()
  {
    HTTPDataManager* pSidxDataManager;
    MM_CriticalSection_Enter(m_resourceDataLock);
    pSidxDataManager = m_pSidxDataManager;
    MM_CriticalSection_Leave(m_resourceDataLock);
    return pSidxDataManager;
  };

  HTTPCommon::HTTPMediaType GetHTTPMediaMajorType(FileSourceMjMediaType fsMajorType)
  {
    HTTPCommon::HTTPMediaType httpMajorType;
    switch(fsMajorType)
    {
      case FILE_SOURCE_MJ_TYPE_AUDIO:
        httpMajorType = HTTPCommon::HTTP_AUDIO_TYPE;
        break;
      case FILE_SOURCE_MJ_TYPE_VIDEO:
        httpMajorType = HTTPCommon::HTTP_VIDEO_TYPE;
        break;
      case FILE_SOURCE_MJ_TYPE_TEXT:
        httpMajorType = HTTPCommon::HTTP_TEXT_TYPE;
        break;
      default:
        httpMajorType = HTTPCommon::HTTP_UNKNOWN_TYPE;
        break;
    }
    return httpMajorType;
  };

  HTTPMediaMinorType GetHTTPMediaMinorType(FileSourceMnMediaType fsMinorType)
  {
    HTTPMediaMinorType httpMinorType;
    switch(fsMinorType)
    {
      case FILE_SOURCE_MN_TYPE_AAC:
        httpMinorType = HTTP_MINOR_TYPE_AAC;
        break;
      case FILE_SOURCE_MN_TYPE_AC3:
        httpMinorType = HTTP_MINOR_TYPE_AC3;
        break;
      case FILE_SOURCE_MN_TYPE_EAC3:
        httpMinorType = HTTP_MINOR_TYPE_EAC3;
        break;
      case FILE_SOURCE_MN_TYPE_MP2:
        httpMinorType = HTTP_MINOR_TYPE_MP2;
        break;
      case FILE_SOURCE_MN_TYPE_AAC_ADTS:
        httpMinorType = HTTP_MINOR_TYPE_AAC;
        break;
      case FILE_SOURCE_MN_TYPE_AAC_ADIF:
        httpMinorType = HTTP_MINOR_TYPE_AAC_ADTS;
        break;
      case FILE_SOURCE_MN_TYPE_AAC_LOAS:
        httpMinorType = HTTP_MINOR_TYPE_AAC_ADIF;
        break;
      case FILE_SOURCE_MN_TYPE_H264:
        httpMinorType = HTTP_MINOR_TYPE_H264;
        break;
      case FILE_SOURCE_MN_TYPE_MPEG2:
        httpMinorType = HTTP_MINOR_TYPE_MPEG2;
        break;
       case FILE_SOURCE_MN_TYPE_HEVC:
         httpMinorType = HTTP_MINOR_TYPE_HVC;
         break;
      case FILE_SOURCE_MN_TYPE_SMPTE_TIMED_TEXT:
        httpMinorType = HTTP_MINOR_TYPE_SMPTETT;
        break;
      case FILE_SOURCE_MN_TYPE_PNG:
        httpMinorType = HTTP_MINOR_TYPE_PNG;
        break;
      default:
        httpMinorType = HTTP_MINOR_TYPE_UNKNOWN;
        break;
    }
    return httpMinorType;
  };

  HTTPResourceTrackMap *GetTrackMapByTrackID(int32 nTrackID)
  {
    HTTPResourceTrackMap *pTrackMap = NULL;
    if ( m_pTrackMap && m_nNumOfTracks > 0 )
    {
      uint32 i;
      for ( i = 0; i < m_nNumOfTracks; i++ )
      {
        if ( m_pTrackMap[i].m_nTrackID == nTrackID )
        {
          // found track with matching track Id
          pTrackMap = m_pTrackMap +i;
          break;
        }
      }
      if ( i == m_nNumOfTracks )
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Unable to find the track map for %d", nTrackID );
      }
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid resource state %p %u",
                     (void *)m_pTrackMap, m_nNumOfTracks );
    }
    return pTrackMap;
  };

  HTTPResourceTrackMap *GetTrackMapByMediaType(HTTPCommon::HTTPMediaType majorType)
  {
    HTTPResourceTrackMap *pTrackMap = NULL;
    if ( m_pTrackMap && m_nNumOfTracks > 0 )
    {
      uint32 i;
      for ( i = 0; i < m_nNumOfTracks; i++ )
      {
        if ( m_pTrackMap[i].m_majorType == majorType )
        {
          // found track with matching track Id
          pTrackMap = m_pTrackMap + i;
          break;
        }
      }
      if ( i == m_nNumOfTracks )
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Unable to find the track map for %d", majorType );
      }
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid resource state %p %u",
                     (void *)m_pTrackMap, m_nNumOfTracks );
    }
    return pTrackMap;
  };

   int64 m_nSeekTime;
private:
  // resource key that identifies the resource
  uint64 m_nKey;

  FileSource *m_pFileSource;
  HTTPDataManager *m_pDataManager;
  HTTPDataManager *m_pSidxDataManager;

  // local to protect the data in the resource form multiple access,
  // typical use case is that the calls are execersied in HTTP Source
  // Thread and FileSoruce events come in FileSource  Thread
  MM_HANDLE m_resourceDataLock;

  HTTPResourceTrackMap* m_pTrackMap;
  uint32 m_nNumOfTracks;
  uint64 m_nFlushTime;

  MPDProfileType m_nProfileType;
};

} // namespace video

#endif // HTTPRESOURCE_H
