#ifndef HTTPRESOURCESMANAGER_H
#define HTTPRESOURCESMANAGER_H
/************************************************************************* */
/**
 * HTTPResourceManager.h
 * @brief Defines the HTTP Resource Manager
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPResourceManager.h#32 $
$DateTime: 2013/08/30 09:26:33 $
$Change: 4360822 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "Streamlist.h"
#include "HTTPCommon.h"
#include "HTTPDataInterface.h"
#include "QsmTypes.h"

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
#define HTTP_MAX_RESOURCES_POOL   10
#define HTTP_MAX_RESOURCES_FACTOR 50
#define HTTP_RESOURCE_RESERVED_KEY 0xFFFFFFFFFFFFFFFFULL

#define HTTP_RESOURCE_INDEX_AUDIO  0
#define HTTP_RESOURCE_INDEX_VIDEO  1
#define HTTP_RESOURCE_INDEX_TEXT   2
#define HTTP_RESOURCE_INDEX_MAX    3


/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
// forward declarations
class HTTPResource;

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

enum HTTPResourceState{
  RESOURCE_STATE_INIT,
  RESOURCE_STATE_OPENING,
  RESOURCE_STATE_OPEN
};

enum HTTPResourceMgrState{
  RESOURCE_MGR_STATE_INIT,
  RESOURCE_MGR_STATE_OPENING,
  RESOURCE_MGR_STATE_CLOSING
};

// A queue element (stored in Queue) for a data resource
struct HTTPResourceElement
{
  void Reset();
  ordered_StreamList_link_type link;
  uint64 nKey;
  HTTPResource *pResource;
  bool bInitialize;
};


/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class HTTPResourceManager
{
public:
  HTTPResourceManager( );

  virtual ~HTTPResourceManager();

  virtual HTTPDownloadStatus GetSegmentInfo(uint64 nStartTime,
                                            uint64 nDuration) = 0;

  virtual HTTPDownloadStatus FillDataUnitInfo(uint64 nStartTime,
                                              uint64 nDuration,
                                              QSM::CDataUnitInfo* pDataUnitInfo,
                                              uint32 nSizeOfDataUnitInfo,
                                              uint32& nNumDataUnitInfo) = 0;

  virtual HTTPDownloadStatus GetSegmentData(uint64 nFragKey) = 0;

  virtual HTTPDownloadStatus CancelSegmentData(uint64 nFragKey) = 0;

  virtual HTTPDownloadStatus Seek(uint64 nSeekTime) = 0;

  virtual HTTPDownloadStatus Select(uint64 nDataUnitKey, uint64& nPbTime) = 0;

  HTTPDownloadStatus Flush(HTTPMediaType majorType,int64 nStartTime);

  void ClearBufferedData(HTTPCommon::HTTPMediaType majorType, uint64 nTime);

  virtual HTTPDownloadStatus Open(uint64 nDataUnitKey,int64 nStartTime,bool bSeek=false) =0;

  virtual bool IsClosing()
  {
     bool bRet = false;
     if(m_eState == RESOURCE_MGR_STATE_CLOSING)
     {
       bRet = true;
     }
     return bRet;
  }
  virtual bool IsOpened()
  {
    bool bRet = false;
    if(m_eState == RESOURCE_MGR_STATE_OPENING)
    {
      bRet = true;
    }
    return bRet;
  }
  virtual HTTPDownloadStatus Open()
  {
    HTTPDownloadStatus status = HTTPCommon::HTTPDL_WAITING;
    // Move to open only when resource manager is in init
    //state. If on one of the media type close is called but
    // on other media type reads are still going on than we
    //will wait for other media type to finish reading.
    if(m_eState == RESOURCE_MGR_STATE_INIT)
    {
      m_eState = RESOURCE_MGR_STATE_OPENING;
      status =  HTTPCommon::HTTPDL_SUCCESS;
    }
    else if(m_eState == RESOURCE_MGR_STATE_OPENING)
    {
      status = HTTPCommon::HTTPDL_EXISTS;
    }
    return status;
  }

  bool IsEndOfSession()
  {
    bool bEndOfSession = false;
    MM_CriticalSection_Enter(m_resourcesLock);
    bEndOfSession = m_bEndOfSession;
    MM_CriticalSection_Leave(m_resourcesLock);
    return bEndOfSession;
  }
  virtual void SetEndOfSession(bool bEndOfSession)
  {
    MM_CriticalSection_Enter(m_resourcesLock);
    m_bEndOfSession = bEndOfSession;
    MM_CriticalSection_Leave(m_resourcesLock);
  }

  virtual HTTPDownloadStatus IsReadable(const HTTPMediaType majorType = HTTPCommon::HTTP_UNKNOWN_TYPE);

  virtual HTTPDownloadStatus Close();

  virtual HTTPDownloadStatus Close(HTTPMediaType majorType);

  virtual bool CanPlaybackUninterrupted();

// implementation for HTTP Data Interface methods
public:
  virtual uint32 GetMediaTrackInfo(HTTPMediaTrackInfo *pTrackInfo );

  virtual HTTPDownloadStatus GetSelectedMediaTrackInfo(HTTPCommon::HTTPMediaType majorType,
                                                       HTTPMediaTrackInfo &TrackInfo );

  virtual HTTPDownloadStatus GetCodecData(uint32 nTrackID,
                                          HTTPMediaMinorType minorType,
                                          HTTPCodecData &CodecData);

  virtual HTTPDownloadStatus GetFormatBlock(HTTPCommon::HTTPMediaType majorType,
                                            uint8* pBuffer,
                                            uint32 &nbufSize);

  virtual HTTPCommon::HTTPDownloadStatus GetNextMediaSample( HTTPCommon::HTTPMediaType majorType,
                                                             uint8 *pBuffer,
                                                             uint32 &nSize,
                                                             HTTPSampleInfo &sampleInfo);

  virtual void GetVideoInfo(HTTPCommon::HTTPMediaType majorType, char* pVideoURL,size_t& nURLSize, char* pIPAddr, size_t& nIPAddrSize);

  virtual bool GetCurrentPlaybackPosition( HTTPCommon::HTTPMediaType mediaType,
                                           uint64 &nPlaybackPosition);

  virtual bool GetDownloadPosition( HTTPCommon::HTTPMediaType majorType,
                                    uint64& nDownloadPosition);

  virtual void SetBaseTime(uint64 baseTime);

  virtual bool GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime);
  virtual void SetEndTime(uint64 nEndTime);

protected:
  virtual HTTPCommon::HTTPDownloadStatus AddResource(uint64 nResourceKey,
                                                     HTTPResource *pResource);

  virtual HTTPCommon::HTTPDownloadStatus GetResource(uint64 nResourceKey,
                                                     HTTPResource **pResource);

  virtual HTTPCommon::HTTPDownloadStatus GetResource(const uint64 nStartTime,
                                                     HTTPResource*& pResource);

  virtual HTTPDownloadStatus GetResource(const uint64 nStartTime,
                                         const uint64 nDuration,
                                         HTTPResource*& pResource);

  virtual HTTPDownloadStatus GetReadableResource( HTTPMediaType majorType,
                                                  HTTPResource *&pResource );

  /**
   * Find the first cancellable data unit following the data unit
   * detected as too slow. If not found, then the out params have
   * a value of MAX_UINT64_VAL.
   */
  virtual void GetFirstCancellableDataUnit(
    const uint32 nTooSlowSegKey, const uint64 nTooSlowDataUnitKey,
    uint64& nFirstCancellableUnitSegKey, uint64& nFirstCancellableDataUnitKey);

  virtual bool GetFirstResourceStartTime(uint64 &nStartTime);

  HTTPDownloadStatus GetFirstResource(HTTPResource **pResource);

  virtual HTTPDownloadStatus RemoveResource(const uint64 nResourceKey);

  /**
   * Remove any resource which is in error state in inuselist.
   */
  virtual void RemoveResourcesInErrorState();

  virtual HTTPDownloadStatus GetFreeResource(HTTPResource **pResource);

protected:
    // lock to control the access for the resources
  MM_HANDLE  m_resourcesLock;

private:

  bool GetResourceIndex( HTTPCommon::HTTPMediaType majorType,
                         int32 &localCacheIndex );

  HTTPDownloadStatus RemoveResources();

  // free resources
  ordered_StreamList_type m_resourcesFreeList;
  // in use resources
  ordered_StreamList_type m_resourcesInUseList;
  // the data resources
  HTTPResourceElement *m_resources;

  HTTPResource *m_readCache[HTTP_RESOURCE_INDEX_MAX];

  bool m_bEndOfSession;
  HTTPResourceMgrState m_eState;
  uint64 m_nEndTime;
};

} // namespace video

#endif // HTTPRESOURCESMANAGER_H
