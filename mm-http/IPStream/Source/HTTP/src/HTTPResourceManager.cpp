/************************************************************************* */
/**
 * HTTPResourceManager.cpp
 * @brief Implments the HTTP Resources Manager
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPResourceManager.cpp#46 $
$DateTime: 2013/08/30 09:26:33 $
$Change: 4360822 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "HTTPResourceManager.h"
#include "HTTPResource.h"
#include "SourceMemDebug.h"

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
#define GET_RESOURCE_FROM_LOCAL_CACHE(localCache, majorType, pResource) \
{ \
  int32 nMediaIndex = 0; \
  if (GetResourceIndex( majorType, nMediaIndex)) \
  { \
    pResource = localCache[nMediaIndex]; \
  } \
} \

#define UPDATE_RESOURCE_IN_LOCAL_CACHE(localCache, majorType, pResource) \
{ \
  int32 nMediaIndex = 0; \
  if (GetResourceIndex( majorType, nMediaIndex)) \
  { \
    localCache[nMediaIndex] = pResource; \
  } \
} \

#define INVALIDATE_MEDIATYPE_IN_LOCAL_CACHE(localCache, majorType) \
{ \
  int32 nMediaIndex = 0; \
  if (GetResourceIndex( majorType, nMediaIndex)) \
  { \
    localCache[nMediaIndex] = NULL; \
  } \
} \

#define REMOVE_RESOURCE(pResourceElement) \
{ \
  ordered_StreamList_pop_item(&m_resourcesInUseList, &pResourceElement->link); \
  if (pResourceElement->pResource) \
  { \
    (void)pResourceElement->pResource->Close(); \
  } \
  QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, \
                 "resource with key [0x%08x%08x] removed", \
                 (uint32)(pResourceElement->nKey >> 32), (uint32)pResourceElement->nKey ); \
  pResourceElement->nKey = HTTP_RESOURCE_RESERVED_KEY; \
  ordered_StreamList_push(&m_resourcesFreeList, &pResourceElement->link, pResourceElement->nKey); \
} \

#define MAX_SEGMENT_OPEN_COUNT 2
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
void HTTPResourceElement::Reset()
{
  std_memset(&link, 0x0, sizeof(ordered_StreamList_link_type));
  nKey = HTTP_RESOURCE_RESERVED_KEY;
  pResource = NULL;
  bInitialize = false;
}

/*
 * Constructor
 */
HTTPResourceManager::HTTPResourceManager( ) :
  m_resourcesLock(NULL),
  m_bEndOfSession(false),
  m_eState(RESOURCE_MGR_STATE_INIT),
  m_nEndTime(MAX_UINT64)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "created HTTP resource manager" );

  if (MM_CriticalSection_Create(&m_resourcesLock) != 0)
  {
    QTV_MSG_PRIO1(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                  "Unable to create a critical section: %p",
                  (void *)m_resourcesLock);
    m_resourcesLock = NULL;
  }

  ordered_StreamList_init(&m_resourcesFreeList,
                          ORDERED_STREAMLIST_ASCENDING,
                          ORDERED_STREAMLIST_PUSH_SLT);
  ordered_StreamList_init(&m_resourcesInUseList,
                          ORDERED_STREAMLIST_ASCENDING,
                          ORDERED_STREAMLIST_PUSH_SLT);

  m_resources = (HTTPResourceElement*)QTV_Malloc(HTTP_MAX_RESOURCES_POOL * sizeof(HTTPResourceElement));
  if(m_resources)
  {
    // populate the free resources
    for( int i = 0; i < HTTP_MAX_RESOURCES_POOL; i++ )
    {
      m_resources[i].Reset();
      ordered_StreamList_push(&m_resourcesFreeList, &m_resources[i].link, i);
    }
  }

  std_memset(&m_readCache, 0, sizeof(m_readCache));
}

/*
 * Destructor
 */
HTTPResourceManager::~HTTPResourceManager()
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "HTTPResourceManager destructor with %lu pending resources",
                  ordered_StreamList_size(&m_resourcesInUseList) );

  MM_CriticalSection_Enter(m_resourcesLock);

  HTTPResourceElement *pResourceElement =
      (HTTPResourceElement*)ordered_StreamList_pop_front(&m_resourcesInUseList);
  while ( pResourceElement )
  {
    HTTPResource *pResource = pResourceElement->pResource;
    if( pResource )
    {
      (void)pResource->Close();
      QTV_Delete(pResource);
      pResource = pResourceElement->pResource = NULL;
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                     "resource with key [0x%08x%08x] deleted",
                     (uint32)(pResourceElement->nKey >> 32), (uint32)pResourceElement->nKey );
      pResourceElement->nKey = HTTP_RESOURCE_RESERVED_KEY;
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid resource in the list for for key [0x%08x%08x]",
                     (uint32)(pResourceElement->nKey >> 32), (uint32)pResourceElement->nKey );
    }
    pResourceElement =
      (HTTPResourceElement*)ordered_StreamList_pop_front(&m_resourcesInUseList);
  }

  pResourceElement =
      (HTTPResourceElement*)ordered_StreamList_pop_front(&m_resourcesFreeList);
  while ( pResourceElement )
  {
    if( pResourceElement->pResource )
    {
      QTV_Delete(pResourceElement->pResource);
      pResourceElement->pResource = NULL;
    }
    pResourceElement =
      (HTTPResourceElement*)ordered_StreamList_pop_front(&m_resourcesFreeList);
  }

  if(m_resources)
  {
    QTV_Free(m_resources);
    m_resources = NULL;
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  if (m_resourcesLock)
  {
    MM_CriticalSection_Release(m_resourcesLock);
    m_resourcesLock = NULL;
  }
}

/*
 * checks if any resource is readable
 *
 * @return status
 */
HTTPDownloadStatus HTTPResourceManager::IsReadable(const HTTPMediaType majorType)
{
  HTTPResource *pResource = NULL;

  MM_CriticalSection_Enter(m_resourcesLock);
  HTTPDownloadStatus status = GetReadableResource(majorType, pResource);
  if (status == HTTPCommon::HTTPDL_SUCCESS ||
      status == HTTPCommon::HTTPDL_DATA_END)
  {
    if(pResource)
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                     "resource with key [0x%08x%08x] readable",
                     (uint32)(pResource->GetKey() >> 32), (uint32)pResource->GetKey() );
    }
  }
  MM_CriticalSection_Leave(m_resourcesLock);

  return status;
}
/*
 * @brief - Marks read complete for given media type in all resources.
 *          If all media types are done reading resource is removed.
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPDownloadStatus HTTPResourceManager::Close(HTTPMediaType majorType)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPResourceElement *pResourceElement = NULL;

  MM_CriticalSection_Enter(m_resourcesLock);

  pResourceElement = (HTTPResourceElement *)
                        ordered_StreamList_peek_front(&m_resourcesInUseList);
  while (pResourceElement)
  {
    if(pResourceElement->pResource)
    {
      status = pResourceElement->pResource->Close(majorType);
      if(status == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        //Error while closing resource, return here
        break;
      }
    }
    pResourceElement = (HTTPResourceElement *)
                        ordered_StreamList_peek_next(&pResourceElement->link );
  }
  if(status != HTTPCommon::HTTPDL_WAITING)
  {
    m_eState = RESOURCE_MGR_STATE_INIT;
  }
  else
  {
    //Close is not yet called on all media types. Move it to closing so that
    //Open command is not given to resources
    m_eState = RESOURCE_MGR_STATE_CLOSING;
  }
  INVALIDATE_MEDIATYPE_IN_LOCAL_CACHE( m_readCache, majorType );

  MM_CriticalSection_Leave(m_resourcesLock);

  return status;

}
/*
 * adds the resource to the list of resources that need to be read
 *
 * @param[in] nResourceKey key that identifies the resource
 * @param[in] pResource the resource that needs to be added
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPCommon::HTTPDownloadStatus HTTPResourceManager::AddResource
(
  uint64 nResourceKey,
  HTTPResource *pResource
)
{
  HTTPCommon::HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_WAITING;
  MM_CriticalSection_Enter(m_resourcesLock);

  if (!IsEndOfSession())
  {
    HTTPResourceElement *pResourceElement = NULL;
    while (eReturn == HTTPCommon::HTTPDL_WAITING)
    {
      //Pop the resource passed in from free list, if it's not found it's a newly created resource!
      bool bFound = false;
      pResourceElement =
        (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesFreeList);
      while (pResourceElement)
      {
        if (pResourceElement->pResource == pResource)
        {
          ordered_StreamList_pop_item(&m_resourcesFreeList, &pResourceElement->link);
          bFound = true;
          break;
        }
        pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                                          &pResourceElement->link );
      }
      if (!bFound)
      {
        pResourceElement =
          (HTTPResourceElement *)ordered_StreamList_pop_front(&m_resourcesFreeList);
      }

      if ( pResourceElement )
      {
        pResourceElement->pResource = pResource;
        pResourceElement->nKey = nResourceKey;
        //to indicate that this is a reusable resource when transferred back to the free list.
        pResourceElement->bInitialize = true;
        // put the element to the inuse list, note that insertion can happen either if
        // resource is removed and being added back or if seeking to an already played
        // position and all resources are not flushed out before seek. Even then the
        // inserted resource will be OPENed in a) Select() or b) GetReadableResource()
        ordered_StreamList_push(&m_resourcesInUseList,
                                &pResourceElement->link,
                                nResourceKey);
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                       "added resource with key [0x%08x%08x] to the active list",
                       (uint32)(nResourceKey >> 32), (uint32)nResourceKey );
        eReturn = HTTPCommon::HTTPDL_SUCCESS;
      }
      else
      {
        //add more resources if the max resource limit has been reached.
        StreamList_size_type inUseListSize = ordered_StreamList_size(&m_resourcesInUseList);
        if (inUseListSize >= HTTP_MAX_RESOURCES_FACTOR * HTTP_MAX_RESOURCES_POOL)
        {
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "%d resources in use, waiting for resource(s) to free up!",
                         (int)inUseListSize );
          eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
        else
        {
          uint32 nNewPoolSize = (uint32)inUseListSize + HTTP_MAX_RESOURCES_POOL;
          HTTPResourceElement* pNewPool =
            (HTTPResourceElement*)QTV_Realloc(m_resources, nNewPoolSize * sizeof(HTTPResourceElement));
          if (pNewPool)
          {
            //Create a new batch of resources and move the existing ones to the new pool
            m_resources = pNewPool;
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "No more free resources available, creating new batch of %d resources",
                           HTTP_MAX_RESOURCES_POOL );
            ordered_StreamList_init(&m_resourcesFreeList,
              ORDERED_STREAMLIST_ASCENDING,
              ORDERED_STREAMLIST_PUSH_SLT);
            ordered_StreamList_init(&m_resourcesInUseList,
              ORDERED_STREAMLIST_ASCENDING,
              ORDERED_STREAMLIST_PUSH_SLT);
            for( uint32 i = 0; i < inUseListSize; i++ )
            {
              ordered_StreamList_push(&m_resourcesInUseList, &m_resources[i].link, m_resources[i].nKey);
            }
            for( uint32 i = (uint32)inUseListSize; i < nNewPoolSize; i++ )
            {
              m_resources[i].Reset();
              ordered_StreamList_push(&m_resourcesFreeList, &m_resources[i].link, i);
            }
          }
          else
          {
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Could not create new batch of %d resources, waiting for resource(s) to free up!",
                           HTTP_MAX_RESOURCES_POOL );
            eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
          }
        }
      }
    }

    if (eReturn == HTTPCommon::HTTPDL_SUCCESS)
    {
      int nResourceCnt = (int)ordered_StreamList_size(&m_resourcesInUseList);
      //Issue a open on resource only if resource manager is in opening
      //state else wait for open command on resource manager
      if (nResourceCnt <= MAX_SEGMENT_OPEN_COUNT && m_eState == RESOURCE_MGR_STATE_OPENING)
      {
        //queue the open command
        (void)pResourceElement->pResource->OpenSegment();
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "HTTPResourceManager end of session set" );
    eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  return eReturn;
}
/*
 * gets the resource from the list of resources
 *
 * @param[in] nResourceKey key that identifies the resource
 * @param[out] pResource the populated resource on success
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPDownloadStatus HTTPResourceManager::GetFirstResource
(
  HTTPResource **pResource
)
{
  // This function does not take resources lock, as the caller has to take it
  // as pResource points to an element in the queue.
  HTTPCommon::HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_WAITING;
  if(pResource)
  {
    HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
    if ( pResourceElement )
    {
      *pResource = pResourceElement->pResource;
      eReturn = HTTPCommon::HTTPDL_SUCCESS;
    }
    else if(IsEndOfSession())
    {
      *pResource = NULL;
      eReturn = HTTPCommon::HTTPDL_DATA_END;
    }
  }
  return eReturn;
}

/*
 * gets the resource from the list of resources
 *
 * @param[in] nResourceKey key that identifies the resource
 * @param[out] pResource the populated resource on success
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPDownloadStatus HTTPResourceManager::GetResource
(
  uint64 nResourceKey,
  HTTPResource **pResource
)
{
  // This does not take resources lock, as the caller must take the resources
  // lock when accessing *pResource.
  HTTPCommon::HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
  if(pResource)
  {
    HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
    while ( pResourceElement )
    {
      if( pResourceElement->nKey == nResourceKey )
      {
        *pResource = pResourceElement->pResource;
        eReturn = HTTPCommon::HTTPDL_SUCCESS;
        break;
      }
      pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                                        &pResourceElement->link );
    }
  }
  return eReturn;
}

/*
 * gets the resource with specified start time
 *
 * @param[in] nStartTime - Request start time (in msec)
 * @param[out] pResourceHead - Link to first resource inside the requested range
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPDownloadStatus HTTPResourceManager::GetResource
(
  const uint64 nStartTime,
  HTTPResource*& pResourceHead
)
{
  // This does not take resources lock, as the caller must take the resources
  // lock when accessing *pResource.
  HTTPCommon::HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_SUCCESS;
  HTTPResourceElement *pResourceElement =
            (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
  uint64 nSegStart, nSegDuration;

  while ( pResourceElement )
  {
    if (pResourceElement->pResource &&
          pResourceElement->pResource->GetSegmentRange(nSegStart, nSegDuration))
    {
      if (nStartTime < (nSegStart+nSegDuration))
      {
        pResourceHead = pResourceElement->pResource;
        eReturn = HTTPCommon::HTTPDL_SUCCESS;
        break;
      }
    }
    pResourceElement =
          (HTTPResourceElement *)ordered_StreamList_peek_next(&pResourceElement->link );
  }
  if(pResourceElement == NULL && IsEndOfSession())
  {
    eReturn = HTTPCommon::HTTPDL_DATA_END;
  }
  return eReturn;
}
/*
 * gets the first resource in the specified time range
 *
 * @param[in] nStartTime - Request start time (in msec)
 * @param[in] nDuration - Request duration (in msec)
 * @param[out] pResource - First resource inside the requested range
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPDownloadStatus HTTPResourceManager::GetResource
(
  const uint64 nStartTime,
  const uint64 nDuration,
  HTTPResource*& pResource
)
{
  // This does not take resources lock, as the caller must take the resources
  // lock when accessing *pResource.
  HTTPCommon::HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
  uint64 nSegStart, nSegDuration;
  pResource = NULL;

  while ( pResourceElement )
  {
    if (pResourceElement->pResource &&
        pResourceElement->pResource->GetSegmentRange(nSegStart, nSegDuration))
    {
      if (((nSegStart <= nStartTime) && (nStartTime < (nSegStart + nSegDuration))) ||
          ((nSegStart > nStartTime) && (nSegStart < nStartTime + nDuration)))
      {
        pResource = pResourceElement->pResource;
        eReturn = HTTPCommon::HTTPDL_SUCCESS;
        break;
      }
    }
    pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_next(&pResourceElement->link );
  }

  return eReturn;
}

/*
 * gets the resource from the list of free resources
 *
 * @param[in] nResourceKey key that identifies the resource
 * @param[out] pResource the populated resource on success
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPDownloadStatus HTTPResourceManager::GetFreeResource
(
HTTPResource **pResource
)
{
  // This function does not take the resources lock, as the caller has
  // to take the lock as *pResource points to an element on the queue.
  HTTPCommon::HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;

  if(pResource)
  {
    HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesFreeList);
    while ( pResourceElement )
    {
      if( pResourceElement->bInitialize)
      {
        *pResource = pResourceElement->pResource;
        eReturn = HTTPCommon::HTTPDL_SUCCESS;
        break;
      }
      pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                                        &pResourceElement->link );
    }
  }

  return eReturn;
}

/*
 * Sets the base time on all the readable resources in the resource manager
 *
 * @param[in] baseTime - baseTime to be set.
 *
 * @return
 */
void HTTPResourceManager::SetBaseTime(uint64 baseTime)
{
  MM_CriticalSection_Enter(m_resourcesLock);
  /* iterate the resources and set base time on all the opened resources */
   HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);

  HTTPResource *pResource = NULL;
  while ( pResourceElement )
  {
    pResource = pResourceElement->pResource;
    if(pResource && pResource->IsReadable(HTTPCommon::HTTP_UNKNOWN_TYPE)==HTTPCommon::HTTPDL_SUCCESS)
    {
      pResource->SetBaseTime(baseTime);
    }
   pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                              &pResourceElement->link );
  }
  MM_CriticalSection_Leave(m_resourcesLock);
  return;
}
/*
 * Gets the base time from the first readable resources in the resource manager
 *
 * @param[out] baseTime - baseTime of the first readable resource. basetime is
 * returned 0 if first resource is not readable.
 *
 * @return
 */
bool HTTPResourceManager::GetBaseTime(uint64 &segMediaTime, uint64 &segMPDTime)
{
  bool bRet = false;
  segMediaTime = 0;
  segMPDTime = 0;
  HTTPResource *pResource = NULL;
  MM_CriticalSection_Enter(m_resourcesLock);
  /* Get base time from the first resource in resource list */
   HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
  if ( pResourceElement )
  {
    pResource = pResourceElement->pResource;
    if(pResource && pResource->IsReadable(HTTPCommon::HTTP_UNKNOWN_TYPE) == HTTPCommon::HTTPDL_SUCCESS)
    {
      bRet = pResource->GetBaseTime(segMediaTime, segMPDTime);
    }
  }
  MM_CriticalSection_Leave(m_resourcesLock);

  return bRet;
}
/*
 * Sets the end time for resourcemanager/rephandler. Also check if data is already
 * downloaded upto end time which is being set. In that case mark the last segment
 * as complete.
 *
 * @param[in] nEndTime - end time to be set
 * @return
 */
void HTTPResourceManager::SetEndTime(uint64 nEndTime)
{
  MM_CriticalSection_Enter(m_resourcesLock);
  if(nEndTime < m_nEndTime)
  {
    HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);

    HTTPResource *pResource = NULL;

    while ( pResourceElement )
    {
      pResource = pResourceElement->pResource;
      HTTPResourceElement *pNextResourceElement = NULL;
      pNextResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                                &pResourceElement->link );
      if (pResource)
      {
        uint64 nSegStart, nSegDuration;
        pResource->GetSegmentRange(nSegStart,nSegDuration);
        if (nSegStart >= nEndTime)
        {
          uint64 nResourceKey = pResource->GetKey();
          RemoveResource(nResourceKey);
        }
      }
      pResourceElement = pNextResourceElement;
    }

    //Modify rep end time only if it is less than current set value
    m_nEndTime = nEndTime;
    pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_back(&m_resourcesInUseList);
    if(pResourceElement && pResourceElement->pResource)
    {
      uint64 nPos = 0;
      //If the data upto end time of representation is already downloaded mark the last segment
      //as complete.
      bool bIsPartiallyDownloaded = false;
      if(pResourceElement->pResource->GetDownloadPosition(
           HTTPCommon::HTTP_UNKNOWN_TYPE, nPos, bIsPartiallyDownloaded))
      {
        if(nPos >= m_nEndTime)
        {
          pResourceElement->pResource->MarkSegmentComplete();
        }
      }
    }
  }
  MM_CriticalSection_Leave(m_resourcesLock);
}

bool HTTPResourceManager::GetFirstResourceStartTime(uint64 &nStartTime)
{
  nStartTime = 0;
  bool bRet = false;
  MM_CriticalSection_Enter(m_resourcesLock);
  HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
  if(pResourceElement && pResourceElement->pResource)
  {
      uint64 nDuration = 0;
      if(pResourceElement->pResource->GetSegmentRange(nStartTime,nDuration))
      {
        bRet = true;
      }

  }
  MM_CriticalSection_Leave(m_resourcesLock);

  return bRet;
}
/*
 * removes the specified resource from the list of resources
 *
 * @param[in] nResourceKey key that identifies the resource
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPDownloadStatus HTTPResourceManager::RemoveResource
(
  const uint64 nResourceKey
)
{
  HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;

  MM_CriticalSection_Enter(m_resourcesLock);

  HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
  while (pResourceElement)
  {
    if (pResourceElement->nKey == nResourceKey && pResourceElement->pResource)
    {
      // delete from the in use list and queue it back in the free list
      eReturn = HTTPCommon::HTTPDL_SUCCESS;
      REMOVE_RESOURCE(pResourceElement);
      break;
    }
    pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_next(&pResourceElement->link);
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  return eReturn;
}

/*
 * removes all the active resources
 *
 * @return HTTPDL_SUCCESS on success else failure
 */
HTTPDownloadStatus HTTPResourceManager::Close
(
)
{
  HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_SUCCESS;

  MM_CriticalSection_Enter(m_resourcesLock);

  HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_pop_front(&m_resourcesInUseList);
  while ( pResourceElement )
  {
    HTTPResource *pResource = pResourceElement->pResource;
    if( pResource )
    {
      eReturn = pResource->Close();
      // move the element from the inuse list to the free list
      if ( eReturn ==  HTTPCommon::HTTPDL_SUCCESS )
      {
        QTV_Delete(pResource);
        pResource = pResourceElement->pResource = NULL;
        QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                       "resource with key [0x%08x%08x] deleted",
                       (uint32)(pResourceElement->nKey >> 32), (uint32)pResourceElement->nKey );
        pResourceElement->nKey = HTTP_RESOURCE_RESERVED_KEY;
        // put it back to the free list
        ordered_StreamList_push(&m_resourcesFreeList,
                                &pResourceElement->link,
                                pResourceElement->nKey);
      }
      else
      {
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MED,
                       "close resource with key [0x%08x%08x] failed with %d",
                       (uint32)(pResourceElement->nKey >> 32),
                       (uint32)pResourceElement->nKey, (int)eReturn );
        break;
      }
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid resource in the list for key [0x%08x%08x]",
                     (uint32)(pResourceElement->nKey >> 32), (uint32)pResourceElement->nKey );
      eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
    pResourceElement = (HTTPResourceElement *)ordered_StreamList_pop_front(
      &m_resourcesInUseList);
  }

  SetEndOfSession(false);
  m_eState = RESOURCE_MGR_STATE_INIT;
  m_nEndTime = MAX_UINT64;
  INVALIDATE_MEDIATYPE_IN_LOCAL_CACHE( m_readCache, HTTPCommon::HTTP_AUDIO_TYPE );
  INVALIDATE_MEDIATYPE_IN_LOCAL_CACHE( m_readCache, HTTPCommon::HTTP_VIDEO_TYPE );

  MM_CriticalSection_Leave(m_resourcesLock);

  return eReturn;
}

/*
 * Given the HTTPMediaType populates the local resource cache index
 *
 * @param[in]      majorType the media type for which index needs to be retrived.
 * @param[out]     localCacheIndex the index on success.
 *
 * @return true on success else false
 */
bool HTTPResourceManager::GetResourceIndex
(
  HTTPCommon::HTTPMediaType majorType,
  int32 &localCacheIndex
)
{
  bool bReturn = true;

  switch (majorType)
  {
  case HTTPCommon::HTTP_AUDIO_TYPE:
    localCacheIndex = HTTP_RESOURCE_INDEX_AUDIO;
    break;
  case HTTPCommon::HTTP_VIDEO_TYPE:
    localCacheIndex = HTTP_RESOURCE_INDEX_VIDEO;
    break;
  case HTTPCommon::HTTP_TEXT_TYPE:
    localCacheIndex = HTTP_RESOURCE_INDEX_TEXT;
    break;
  default:
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "unknown media type %d", majorType);
    bReturn = false;
    break;
  }

  return bReturn;
}
/*
 * flushes the data - iterate on the inuse list and flushes the data on all
 * resources before start time. When a resource returns data end it
 * indicates that the end time for the resource is greater than
 * given time so it continues flushing the resources else it stops
 * @param[in] mediaType the media type for which the data needs to be
              flushed
 * @param[in] starttime  time before which data needs to be flushed
 *
 * @return HTTPDL_SUCCESS on success
 */
HTTPDownloadStatus HTTPResourceManager::Flush
(
  HTTPMediaType majorType,
  int64 nStartTime
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
  MM_CriticalSection_Enter(m_resourcesLock);
  HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
  do
  {
    if( pResourceElement && pResourceElement->pResource)
    {
      // Flush on a resource will return status HTTPDL_DATA_END and sets
      // end of stream for the resource when the starttime passed is greater
      //than end time of resource.
      //So RemoveResources call at the end of function will remove all the
      //resources before nStartTime
      //If the resource was not readable(i.e not opened) then resource can
      //be removed when Flush returns data end.
      bool bRemove = false;
      HTTPResourceElement* pResToRemove = pResourceElement;
      uint64 nKey = pResourceElement->pResource->GetKey();
      if(pResourceElement->pResource->IsReadable(
            HTTPCommon::HTTP_UNKNOWN_TYPE) != HTTPCommon::HTTPDL_SUCCESS)
      {
        bRemove = true;
      }

      eStatus = pResourceElement->pResource->Flush(majorType,nStartTime);
      pResourceElement =
        (HTTPResourceElement *)ordered_StreamList_peek_next(&pResourceElement->link);
      if(bRemove &&  eStatus == HTTPCommon::HTTPDL_DATA_END)
      {
        if (pResToRemove)
        {
          // remove from the in use list and queue it back in the free list
          REMOVE_RESOURCE(pResToRemove);
        }
      }
    }
    else
    {
      eStatus = HTTPCommon::HTTPDL_SUCCESS;
      break;
    }

  }while(eStatus==HTTPCommon::HTTPDL_DATA_END);

  (void)RemoveResources();

  MM_CriticalSection_Leave(m_resourcesLock);

  return eStatus;
}

void HTTPResourceManager::ClearBufferedData(HTTPCommon::HTTPMediaType majorType,
                                            uint64 nStartTime)
{
  MM_CriticalSection_Enter(m_resourcesLock);
  HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_back(&m_resourcesInUseList);
  do
  {
    if( pResourceElement && pResourceElement->pResource)
    {
      HTTPResourceElement* pResToRemove = pResourceElement;
      uint64 nKey = pResourceElement->pResource->GetKey();

      uint64 nResStartTime = 0, nResDuration = 0;
      if (true == pResourceElement->pResource->GetSegmentRange(nResStartTime,nResDuration))
      {
        uint64 nResEndTime = nResStartTime + nResDuration;

        bool bRemove = false;

        if (nResEndTime > nStartTime)
        {
          pResourceElement->pResource->ClearBufferedData(nStartTime);

          if (nResStartTime >= nStartTime)
          {
            pResourceElement =
              (HTTPResourceElement *)ordered_StreamList_peek_prev(&pResourceElement->link);

            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "HTTPResourceManager::ClearBufferedData Remove resource 0x%x%x",
              (unsigned int)(nKey >> 32), (unsigned int)(nKey));

            HTTPResource *pReadResource = NULL;
            GET_RESOURCE_FROM_LOCAL_CACHE(m_readCache, majorType, pReadResource);
            if (pReadResource == pResToRemove->pResource)
            {
              INVALIDATE_MEDIATYPE_IN_LOCAL_CACHE( m_readCache, majorType );
            }

            // remove from the in use list and queue it back in the free list
            REMOVE_RESOURCE(pResToRemove);
          }
          else
          {
            break;
          }
        }
        else
        {
          break;
        }
      }
      else
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "HTTPResourceManager::ClearBufferedData1 Remove resource 0x%x%x",
          (unsigned int)(nKey >> 32), (unsigned int)(nKey));

        HTTPResource *pReadResource = NULL;
        GET_RESOURCE_FROM_LOCAL_CACHE(m_readCache, majorType, pReadResource);
        if (pReadResource == pResToRemove->pResource)
        {
          INVALIDATE_MEDIATYPE_IN_LOCAL_CACHE( m_readCache, majorType );
        }

        REMOVE_RESOURCE(pResToRemove);
        break;
      }
    }

  }while(pResourceElement);

  MM_CriticalSection_Leave(m_resourcesLock);
}

/*
 * Tries to get a resource next in line for the given the media type
 *
 * @param[in]      majorType the media type for the request.
 * @param[out]     pDataResource refernce to resource on success.
 *
 * @return HTTPDL_SUCCESS on success.
 *         HTTPDL_WAITING resource is not ready.
 *         HTTPDL_DATA_END reads have been completed on this resource
 *         HTTPDL_ERROR_ABORT all other failures
 */
HTTPDownloadStatus HTTPResourceManager::GetReadableResource
(
  HTTPMediaType majorType,
  HTTPResource *&pResource
)
{
  // This function does not take resMgr lock, as the caller has a handle to a
  // pointer to the inUseList and must use critical section.
  HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_WAITING;

  /* iterate the resources to find the active resource */
  HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
   while ( pResourceElement )
  {
    pResource = pResourceElement->pResource;
    if ( pResource )
    {
      //OPEN the resource just in case (catch-all for corner scenarios such as
      //the following - OPEN is posted on resources in advance which went into
      //ERROR later and when reading we skip over the ones in ERROR and get stuck
      //on the current resource if not OPENED. So this here will prevent that,
      //though might result in buffering since it's a little late for OPEN!)
      (void)pResource->OpenSegment();

      eReturn = pResource->IsReadable( majorType );
      if ( eReturn == HTTPCommon::HTTPDL_SUCCESS )
      {
        /* resource is ready */
        break;
      }
      else if ( eReturn == HTTPCommon::HTTPDL_WAITING )
      {
        if (!pResource->IsDownloading())
        {
          if (m_bEndOfSession)
          {
            //If EOS is marked by QSM and we're stuck on a resource, remove it and move on!
            QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "resource with key [0x%08x%08x] stuck and EOS is set, move on",
                           (uint32)(pResourceElement->nKey >> 32), (uint32)pResourceElement->nKey );
            REMOVE_RESOURCE(pResourceElement);
            pResourceElement =
              (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
            continue;
          }
          else
          {
            //Find the next readable resource
            HTTPResource* pNextResource = NULL;
            HTTPResourceElement *pNextResourceElement =
              (HTTPResourceElement *)ordered_StreamList_peek_next(&pResourceElement->link);
            while (pNextResourceElement)
            {
              if (pNextResourceElement->pResource &&
                  pNextResourceElement->pResource->IsReadable(majorType) == HTTPCommon::HTTPDL_SUCCESS)
              {
                pNextResource = pNextResourceElement->pResource;
                QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                               "resource with key [0x%08x%08x] readable, moving on",
                               (uint32)(pNextResourceElement->nKey >> 32), (uint32)pNextResourceElement->nKey );
                break;
              }
              pNextResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                                            &pNextResourceElement->link );
            }

            //If download hasn't started on the current resource and a resource ahead is readable,
            //jump over to that resource!
            if (pNextResource)
            {
              HTTPResourceElement *pCurrResourceElement = pResourceElement;
              while (pCurrResourceElement && pCurrResourceElement->pResource != pNextResource)
              {
                REMOVE_RESOURCE(pCurrResourceElement);
                pCurrResourceElement =
                  (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
              }
              pResourceElement = pCurrResourceElement;
              continue;
            }
            else
            {
              //Being here could mean that none of the MAX_SEGMENT_OPEN_COUNT resources are readable,
              //in which case just to make sure OPEN the first resource that is downloading
              HTTPResourceElement *pNextResourceElement =
                (HTTPResourceElement *)ordered_StreamList_peek_next(&pResourceElement->link);
              while (pNextResourceElement)
              {
                if (pNextResourceElement->pResource &&
                    pNextResourceElement->pResource->IsDownloading())
                {
                  (void)pNextResourceElement->pResource->OpenSegment();
                  break;
                }
                pNextResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                                              &pNextResourceElement->link );
              }
            }// if (pNextResource)
          }// if (m_bEndOfSession)
        }// if (!pResource->IsDownloading())

        /* resource is not ready fake an underrun */
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "res not ready fake an underrun for mediatype %d",
                       majorType );
        break;
      }
      else if (eReturn == HTTPCommon::HTTPDL_ERROR_ABORT)
      {
        //In case of error, remove resource and move over
        REMOVE_RESOURCE(pResourceElement);

        //Modify status to WAITING as this might be the last resource in Q
        eReturn = HTTPCommon::HTTPDL_WAITING;

        pResourceElement =
          (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
        continue;
      }
      // else if ( eReturn == HTTPDL_DATA_END or HTTPDL_UNSUPPORTED)
      // done reading the resource, move on to the next one
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "HTTPResourceManager unexpected NULL data resource" );
      eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
      break;
    }
    pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                              &pResourceElement->link );
  }

  if ( eReturn == HTTPCommon::HTTPDL_DATA_END )
  {
    // Is this the last resource that was scanned by the above loop?
    HTTPResourceElement *pResourceLastElement =
      (HTTPResourceElement *)ordered_StreamList_peek_back(&m_resourcesInUseList);
    if ( pResourceLastElement == pResourceElement  || pResourceElement == NULL )
    {
      // potentially the end of session
      if ( m_bEndOfSession == true )
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "trigger an EOS for media type %d", majorType);
        pResource = NULL;
        eReturn = HTTPCommon::HTTPDL_DATA_END;
      }
      else
      {
        eReturn = HTTPCommon::HTTPDL_WAITING;
      }
    }
    else
    {
      eReturn = HTTPCommon::HTTPDL_WAITING;
    }
  }
  else if( eReturn == HTTPCommon::HTTPDL_WAITING)
  {
    if (pResourceElement == NULL)
    {
      pResource = NULL;
      if (m_bEndOfSession == true)
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "trigger an EOS for media type %d", majorType );
        eReturn = HTTPCommon::HTTPDL_DATA_END;
      }
      else
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "no more resources for media type %d", majorType );
        eReturn = HTTPCommon::HTTPDL_NO_MORE_RESOURCES;
      }
    }
  }
  else if (HTTPCommon::HTTPDL_UNSUPPORTED == eReturn)
  {
    // This should mean that the last resource is invalid for this major type.
    pResource = NULL;

    // potentially the end of session
    if ( m_bEndOfSession == true )
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "trigger an EOS for media type %d", majorType);
      eReturn = HTTPCommon::HTTPDL_DATA_END;
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
        "Fake underrun for majorType %d as this type not found", majorType);

      eReturn = HTTPCommon::HTTPDL_WAITING;
    }
  }

  if (pResource)
  {
    if (pResource->IsResourceReadDisabled())
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "GetReadable resource returning waiting as resource with key %d is tooslow",
                    (int)pResource->GetKey());

      eReturn = HTTPCommon::HTTPDL_WAITING;
    }
    else
    {
      pResource->MarkResourceReadsStarted();
    }
  }

  return eReturn;
}

/**
 * Find the first cancellable data unit following the data unit
 * detected as too slow. If not found, then the out params have
 * a value of MAX_UINT64_VAL.
 */
void HTTPResourceManager::GetFirstCancellableDataUnit(
    const uint32 nTooSlowSegKey, const uint64 nTooSlowDataUnitKey,
    uint64& nFirstCancellableUnitSegKey, uint64& nFirstCancellableDataUnitKey)
{
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "HTTPResourceManager: Find first data unit following tooslow unit (seg %d, data-unit %d)",
    (int)nTooSlowSegKey, (int)nTooSlowDataUnitKey);

  nFirstCancellableDataUnitKey = MAX_UINT64_VAL;

  HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);

  while (pResourceElement)
  {
    HTTPResource *pRes = pResourceElement->pResource;

    if (pRes->GetKey() == nTooSlowSegKey &&
        pRes->IsSegmentPresent(nTooSlowDataUnitKey))
    {
      pRes->GetFirstCancellableDataUnit(nTooSlowDataUnitKey, nFirstCancellableDataUnitKey);
      if (MAX_UINT64_VAL == nFirstCancellableDataUnitKey)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "HTTPResourceManager: Did not fine cancellable data unit in the same "
          "resource as the tooslow resource");

        // the too slow data unit must have been the last dataunit in the resource.
        pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
          &pResourceElement->link);

        if (pResourceElement)
        {
          pRes = pResourceElement->pResource;
          nFirstCancellableDataUnitKey = pRes->GetFirstDataUnitKey();
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "HTTPReourceManager: Did not find a resource following the tooSlow resource");
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "HTTPResourceManager: Found cancellable data unit in the same "
          "resource as the tooslow resource");
      }

      if (pResourceElement && nFirstCancellableDataUnitKey < MAX_UINT64_VAL)
      {
        nFirstCancellableUnitSegKey = pResourceElement->nKey;
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "HTTPResourceManager Found cancellable dataunit (seg %d, du %d)",
          (int)nFirstCancellableUnitSegKey, (int)nFirstCancellableDataUnitKey);
      }

      break;
    }

    pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                                        &pResourceElement->link);
  }
}

/*
 * get the information from the first readable resource.
 *
 * @param[out] pTrackInfo Pointer to HTTPMediaTrackInfo,
 *
 * @return  number of valid tracks.
 */
uint32 HTTPResourceManager::GetMediaTrackInfo
(
  HTTPMediaTrackInfo *pTrackInfo
)
{
  uint32 nNumTracks = 0;
  HTTPResource *pResource = NULL;
  MM_CriticalSection_Enter(m_resourcesLock);

  HTTPDownloadStatus eStatus = GetReadableResource(HTTPCommon::HTTP_UNKNOWN_TYPE,
                                                   pResource);
  if ( eStatus == HTTPCommon::HTTPDL_SUCCESS && pResource )
  {
    nNumTracks = pResource->GetMediaTrackInfo( pTrackInfo);
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_DEBUG,
                   "find readable resource %p failed %d", (void *)pResource, eStatus );
  }

  MM_CriticalSection_Leave(m_resourcesLock);
  return nNumTracks;
}

/*
 * get the information from the first readable resource.
 *
 * @param[in]  majorType the media type for the request
 * @param[out] TrackInfo populated the track information on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved track info
 * HTTPDL_WAITING - resource unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus HTTPResourceManager::GetSelectedMediaTrackInfo
(
  HTTPCommon::HTTPMediaType majorType,
  HTTPMediaTrackInfo &TrackInfo
)
{
  HTTPResource *pResource = NULL;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;

  // Try finding the resource in the local cache first, if unavailable
  // get the next readable resource
  MM_CriticalSection_Enter(m_resourcesLock);

  GET_RESOURCE_FROM_LOCAL_CACHE( m_readCache, majorType, pResource );
  if ( pResource == NULL )
  {
    eStatus = GetReadableResource(majorType, pResource);
  }

  if ( eStatus == HTTPCommon::HTTPDL_SUCCESS )
  {
    if (pResource == NULL ||
        !pResource->GetSelectedMediaTrackInfo( majorType, TrackInfo ))
    {
      eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "find readable resource %p failed %d", (void *)pResource, eStatus );
  }

  MM_CriticalSection_Leave(m_resourcesLock);
  return eStatus;
}

/*
 * get the information from the first readable resource.
 *
 * @param[in] nTrackID   Identifies the track for which codec data needs to
 * be retrieved
 * @param[in] minorType   media minor type for which the codec info is being
 * requested
 * @param[out] CodecData populates the codec data on success
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved codec data
 * HTTPDL_WAITING - resource unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus HTTPResourceManager::GetCodecData
(
  uint32 nTrackID,
  HTTPMediaMinorType minorType,
  HTTPCodecData &CodecData
)
{
  HTTPResource *pResource = NULL;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;

  MM_CriticalSection_Enter(m_resourcesLock);

  // Try finding the resource in the local cache first, if unavailable
  // get the next readable resource
  GET_RESOURCE_FROM_LOCAL_CACHE( m_readCache, HTTPCommon::HTTP_UNKNOWN_TYPE, pResource );
  if ( pResource == NULL )
  {
    eStatus = GetReadableResource(HTTPCommon::HTTP_UNKNOWN_TYPE, pResource);
  }

  if ( eStatus == HTTPCommon::HTTPDL_SUCCESS )
  {
    if (pResource == NULL ||
        !pResource->GetCodecData( nTrackID, minorType, CodecData ))
    {
      eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "find readable resource %p failed %d", (void *)pResource, eStatus );
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  return eStatus;
}

/*
 * get the information from the first readable resource.
 *
 * @param[in] majorType media type.
 * @param[out] pBuffer  Buffer provies the format block info to the caller
 * @param[out] nbufSize Size of the FormatBlock buffer
 *
 * @return
 * HTTPDL_ERROR_SUCCESS - successfully retrieved format block
 * HTTPDL_WAITING - resource unavailable, try again
 * HTTPDL_ERROR_ABORT - generic failure
 */
HTTPDownloadStatus HTTPResourceManager::GetFormatBlock
(
  HTTPCommon::HTTPMediaType majorType,
  uint8* pBuffer,
  uint32 &nbufSize
)
{
  HTTPResource *pResource = NULL;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;

  MM_CriticalSection_Enter(m_resourcesLock);

  // Try finding the resource in the local cache first, if unavailable
  // get the next readable resource
  GET_RESOURCE_FROM_LOCAL_CACHE( m_readCache, majorType, pResource );
  if ( pResource == NULL )
  {
    eStatus = GetReadableResource(majorType, pResource);
  }

  if ( eStatus == HTTPCommon::HTTPDL_SUCCESS )
  {
    if (pResource == NULL ||
        !pResource->GetFormatBlock(majorType, pBuffer, nbufSize))
    {
      eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "find readable resource %p failed %d", (void *)pResource, eStatus );
  }

  MM_CriticalSection_Leave(m_resourcesLock);

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
HTTPCommon::HTTPDownloadStatus HTTPResourceManager::GetNextMediaSample
(
  HTTPCommon::HTTPMediaType majorType,
  uint8 *pBuffer,
  uint32 &nSize,
  HTTPSampleInfo &sampleInfo
)
{
  uint32 nBackupSize = nSize;
  HTTPResource *pResource = NULL;
  HTTPCommon::HTTPDownloadStatus eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;

  MM_CriticalSection_Enter(m_resourcesLock);

  // find the resource in the local cache
  GET_RESOURCE_FROM_LOCAL_CACHE( m_readCache, majorType, pResource  );

  while (1)
  {
    if (pResource)
    {
      eReturn = pResource->GetNextMediaSample(majorType, pBuffer, nSize,
                                              sampleInfo);

      //OPEN the next MAX_SEGMENT_OPEN_COUNT resources on a successful read.
      //It's done here as an optimization for the following usecase - 0X0X0,
      //where 'X' is a resource in ERROR. If resources are opened in advance
      //only when done reading res 1, res 3 will be opened only when skipping
      //over res 2 and moving onto res 3
      if (eReturn == HTTPCommon::HTTPDL_SUCCESS ||
          eReturn == HTTPCommon::HTTPDL_DATA_END)
      {
        uint32 nResourceCnt = 0;

        HTTPResourceElement *pResourceElement =
          (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
        while (pResourceElement && nResourceCnt < MAX_SEGMENT_OPEN_COUNT)
        {
          HTTPResourceElement *pNextResourceElement = (HTTPResourceElement *)
               ordered_StreamList_peek_next(&pResourceElement->link);

          if (pNextResourceElement)
          {
            HTTPResource *pNextResource = pNextResourceElement->pResource;
            if (pNextResource)
            {
              //Note that if a resource is already in ERROR, we move over that one
              //and OPEN the next one and the one in ERROR will be cleaned up in the
              //read path
              if (pNextResource->OpenSegment() != HTTPCommon::HTTPDL_ERROR_ABORT)
              {
                nResourceCnt++;
              }
            }
            pResourceElement = pNextResourceElement;
          }
          else
          {
            //No more resources to OPEN
            break;
          }
        }

        if (eReturn == HTTPCommon::HTTPDL_DATA_END)
        {
          if (RemoveResources() != HTTPCommon::HTTPDL_SUCCESS)
          {
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Remove Resources failed for major type %x",
                            majorType );
            eReturn = HTTPCommon::HTTPDL_ERROR_ABORT;
            break;
          }
          else
          {
            // done with the resource, null the current
            // resource, this should trigger the outer
            // while loop to look for the next resource
            pResource = NULL;
            INVALIDATE_MEDIATYPE_IN_LOCAL_CACHE( m_readCache, majorType );
            // on data end filesource is setting the size to zero,
            // restore the passed length, because reading the
            // sample from new filesource is attempted
            nSize = nBackupSize;
          }
        }
        else
        {
          break;
        }
      }
      else
      {
        // pass all other returns to the client
        break;
      }
    }
    else
    {
      // cannot find the resource in local cache, look in the resources
      eReturn = GetReadableResource(majorType, pResource);
      if (eReturn == HTTPCommon::HTTPDL_SUCCESS)
      {
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "switching to new resource [0x%08x%08x] for %d",
                       (uint32)(pResource->GetKey()>> 32), (uint32)pResource->GetKey(), (int)majorType );
        // update the local cache
        UPDATE_RESOURCE_IN_LOCAL_CACHE( m_readCache, majorType,
                                        pResource );
      }
      else if (eReturn == HTTPCommon::HTTPDL_WAITING)
      {
        // Next readable resource is not available
        // Return no more resources here. This is
        // similar to SEGMENT_BOUNDARY for VOD profile
        // PlayGroup decides whether or not to switch
        // after inspecting switch Q
        eReturn = HTTPCommon::HTTPDL_NO_MORE_RESOURCES;
        break;
      }
      else
      {
        // report all other problems to the client
         break;
      }
    }
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  return eReturn;
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
void HTTPResourceManager::GetVideoInfo(HTTPCommon::HTTPMediaType majorType, char* pVideoURL,size_t& nURLSize, char* pIPAddr, size_t& nIPAddrSize)
{
  HTTPResource *pResource = NULL;
  GET_RESOURCE_FROM_LOCAL_CACHE( m_readCache, majorType, pResource);
  if(pResource)
  {
    char* pData = pResource->GetVideoURL(nURLSize);
    if(pVideoURL)
    {
      std_strlcpy(pVideoURL,pData,nURLSize+1);
    }
    pData = pResource->GetIPAddr(nIPAddrSize);
    if(pIPAddr)
    {
      std_strlcpy(pIPAddr,pData,nIPAddrSize+1);
    }
  }
};

/*
 * starts from the begining of the inuse list and runs thru the resource list
 * to see any of the them can deleted. A resource may be deleted if reads (on
 * all tracks) and downloads have been complete on that resource. Stops at the
 * first non readable resources
 *
 * @return HTTPDL_SUCCESS if successful else failure
 */
HTTPDownloadStatus HTTPResourceManager::RemoveResources
(
)
{
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;

  MM_CriticalSection_Enter(m_resourcesLock);

  // Delete any resource that can be deleted
  while ( 1 )
  {
    HTTPResourceElement *pResourceElement =
      (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
    if (pResourceElement)
    {
      HTTPResource *pResource = pResourceElement->pResource;
      if ( pResource )
      {
        if (pResource->ReadComplete())
        {
          // remove from the in use list and queue it back in the free list
          REMOVE_RESOURCE(pResourceElement);
        }
        else
        {
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                        "stop remove at resource [0x%08x%08x]",
                        (uint32)(pResource->GetKey()>> 32), (uint32)pResource->GetKey());
          break;
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Unexpected error");
        eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
        break;
      }
    }
    else
    {
       // reached the end of list
       break;
    }
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  return eStatus;
}

/**
 * Remove any resource which is in error state in inuselist.
 */
void HTTPResourceManager::RemoveResourcesInErrorState()
{
  MM_CriticalSection_Enter(m_resourcesLock);

  HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);

  // Delete any resource that can be deleted
  while (pResourceElement)
  {
    HTTPResource *pResource = pResourceElement->pResource;
    if ( pResource )
    {
      if (pResource->IsSegErrorHappened())
      {
        REMOVE_RESOURCE(pResourceElement);

        HTTPResource *pResource = pResourceElement->pResource;
        if (pResource)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "RemoveResourcesInErrorState: removed resource with key %llu",
            pResource->GetKey());
        }

        pResourceElement =
          (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
      }
      else
      {
        pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
           &pResourceElement->link );
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Unexpected error");
      break;
    }
  }

  MM_CriticalSection_Leave(m_resourcesLock);
}

/*
 * redirect to the right resource
 *
 * @param[in] majorType media type.
 * @param[out] nPlaybackPosition populates in time uints on success
 *
 * @return true if successful else failure
 */
bool HTTPResourceManager::GetCurrentPlaybackPosition
(
  HTTPCommon::HTTPMediaType majorType,
  uint64 &nPlaybackPosition
)
{
  bool bResult = false;
  HTTPResource *pResource = NULL;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;

  MM_CriticalSection_Enter(m_resourcesLock);

  // find the resource in the local cache
  GET_RESOURCE_FROM_LOCAL_CACHE( m_readCache, majorType, pResource );
  if ( pResource == NULL )
  {
    eStatus = GetReadableResource(majorType, pResource);
  }

  if ( pResource && eStatus == HTTPCommon::HTTPDL_SUCCESS )
  {
    bResult = pResource->GetCurrentPlaybackPosition( majorType,
                                                     nPlaybackPosition);
  }
  else
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "find readable resource %p failed %d", (void *)pResource, eStatus );
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  return bResult;
}

/*
 * get the current download position for given media type. Redirect to the right
 * resource
 *
 * @param[in] majorType media type.
 * @param[out] nDownloadPosition current download position.
 *
 * @return TRUE - success, FALSE - otherwise
 */
bool HTTPResourceManager::GetDownloadPosition
(
 HTTPCommon::HTTPMediaType majorType,
 uint64& nDownloadPosition
)
{
  bool bResult = true;
  nDownloadPosition = 0;
  MM_CriticalSection_Enter(m_resourcesLock);

  //Get max download position over all readable resources ((ideally if download is
  //underway on all of them this is the same as querying only the last resource)
  HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
  while ( pResourceElement )
  {
    HTTPResource *pResource = pResourceElement->pResource;
    if ( pResource )
    {
      uint64 nPosition = 0;
      bool bIsPartiallyDownloaded = false;
      if (pResource->GetDownloadPosition(majorType, nPosition, bIsPartiallyDownloaded))
      {
        nDownloadPosition = STD_MAX(nDownloadPosition, nPosition);
        if (bIsPartiallyDownloaded)
        {
          break;
        }
      }
      else
      {
        //Some resources can be in error, continue over to next and resource
        //clean up happens in the read path!
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "Couldn't get download position on resource [0x%08x%08x]",
                      (uint32)(pResource->GetKey()>> 32), (uint32)pResource->GetKey());
      }
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Unexpected error");
      bResult = false;
      break;
    }

   pResourceElement = (HTTPResourceElement *)ordered_StreamList_peek_next(
                                            &pResourceElement->link );
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  return bResult;
}

/*
 * Check if playback can continue uninterrupted on the current playing resource.
 *
 * @return
 * true - Can continue uninterrupted
 * false - Cannot continue uninterrupted
 */
bool HTTPResourceManager::CanPlaybackUninterrupted()
{
  bool bRet = false;
  HTTPResource *pResource = NULL;
  MM_CriticalSection_Enter(m_resourcesLock);

  //Check if the first resource in resource list is still available
  HTTPResourceElement *pResourceElement =
    (HTTPResourceElement *)ordered_StreamList_peek_front(&m_resourcesInUseList);
  if (pResourceElement)
  {
    pResource = pResourceElement->pResource;
    if (pResource)
    {
      bRet = pResource->IsSegmentAvailable();
    }
  }

  MM_CriticalSection_Leave(m_resourcesLock);

  return bRet;
}

} // namespace video
