/************************************************************************* */
/**
 * HTTPSegmentDataStoreContainer.cpp
 * @brief implementation of the HTTPSegmentDataStoreContainer.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPSegmentDataStoreContainer.cpp#29 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPHeapManager.h"
#include "HTTPSegmentDataStoreContainer.h"
#include "SourceMemDebug.h"

#include "string.h"

namespace video {

/**
 * @brief
 *  c'tor Initialize lists for
 *    (i) swapped elements
 *    (ii) in-use elements
 *
 * @param rslt
 *  true, on success
 *  false, on failure
 *
 * @param pHeapManager pointer to the HTTPHeapManager instance.
 * @param segmentPurgedHandler callback handler to send segment
 *                             removal notifications
 * @param pSegmentPurgedHandlerData data assocaited with the call
 *                                  back handler
 */
HttpSegmentDataStoreContainer::HttpSegmentDataStoreContainer(
  bool& rslt, int maxSwapSegments, HTTPHeapManager *pHeapManager,
  SegmentPurgedHandler segmentPurgedHandler,
  void *pSegmentPurgedHandlerData) :
    m_MaxSwapSegments(maxSwapSegments),
    m_CachedMaxDownloadedOffset(0),
    m_pHeapMgr(pHeapManager),
    m_segmentPurgedHandler(segmentPurgedHandler),
    m_pSegmentPurgedHandlerData(pSegmentPurgedHandlerData)
{
  rslt = true;

  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HttpSegmentDataStoreContainer::ctor this %p maxSwap segments %d",
    (void*)this, m_MaxSwapSegments);

  (void)StreamQ_init(&m_DataStoreSegmentSwappedList);
  (void)StreamQ_init(&m_DataStoreSegmentInUseList);

}

/**
 * @brief
 *  d'tor Release all resources.
 */
HttpSegmentDataStoreContainer::~HttpSegmentDataStoreContainer()
{
  // Release all segments from inuse list and swapped list
  // so that allocated heap memory is free'd.

  // flush the inuse list
  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_get(
      &m_DataStoreSegmentInUseList);

  if (pSegment)
  {
    do
    {
      if (pSegment->bIsMemAllocatedLocally)
      {
        pSegment->Reset();
        if(pSegment->m_pHttpSegmentDataStoreBase)
        {
          QTV_Delete(pSegment->m_pHttpSegmentDataStoreBase);
          pSegment->m_pHttpSegmentDataStoreBase = NULL;
        }
      }
      QTV_Delete(pSegment);
      pSegment =NULL;

      pSegment = (HttpSegmentDataStoreListElement *)StreamQ_get(
        &m_DataStoreSegmentInUseList);

    } while (pSegment);
  }

  // flush the swapped list
  pSegment = (HttpSegmentDataStoreListElement *)StreamQ_get(
    &m_DataStoreSegmentSwappedList);

  if (pSegment)
  {
    do
    {
      if (pSegment->bIsMemAllocatedLocally)
      {
        pSegment->Reset();
        if (pSegment->m_pHttpSegmentDataStoreBase)
        {
          QTV_Delete(pSegment->m_pHttpSegmentDataStoreBase);
          pSegment->m_pHttpSegmentDataStoreBase = NULL;
        }
      }
      QTV_Delete(pSegment);
      pSegment = NULL;

      pSegment = (HttpSegmentDataStoreListElement *)StreamQ_get(
        &m_DataStoreSegmentSwappedList);

    } while (pSegment);
  }
}
/**
 * Move elements from in-use list to swapped-list or release the
 * segment depending on whether the download for the element is fully
 * completed or not. Reset the start byte offset, and cached
 * info.
 */
void HttpSegmentDataStoreContainer::ResetOffset(uint64 nKey)
{
  int64 newStartOffset = 0;
  bool bUpdateStartOffset = false;
  // Reset the start offsets of elements in in-use list
  // starting from segment which has key as nKey
  // Segment's startoffset is prevsegmentstartoffset+
  // prevsegmentnumbytesdownloaded
  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_check(
      &m_DataStoreSegmentInUseList);

  if (pSegment)
  {
    do
    {
      HttpSegmentDataStoreBase *pHttpSegment =
        pSegment->m_pHttpSegmentDataStoreBase;
      if (NULL == pHttpSegment)
      {
        // Should never reach here.
        QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
          "HttpDataStoreSegmentManager::Flush() sanity check failed");
        break;
      }
      int64 nHTTPSegKey =  pHttpSegment->GetKey();
      if(!bUpdateStartOffset && ((uint64)nHTTPSegKey >= nKey))
      {
         QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
             "HttpDataStoreSegmentManager::ResetOffset(%llu),Resetting from Key %lld",
              nKey,nHTTPSegKey);
         bUpdateStartOffset = true;
      }
      if(!bUpdateStartOffset && (uint64)nHTTPSegKey != nKey)
      {
        if(pHttpSegment->IsFullyDownloaded())
        {
          newStartOffset = pHttpSegment->GetStartOffset() + pHttpSegment->GetNumBytesDownloaded();
        }
        else
        {
          //Should never reach here.
          QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
             "HttpDataStoreSegmentManager::ResetOffset() prev segment not fully downloaded"
             "Can not reset offset, prev segment key %d",(int)nHTTPSegKey);
          break;
        }
      }
      if(bUpdateStartOffset)
      {
        pHttpSegment->SetStartOffset(newStartOffset);
        if(pHttpSegment->IsFullyDownloaded())
        {
          newStartOffset+= pHttpSegment->GetNumBytesDownloaded();
        }
        else
        {
          QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                       "HttpDataStoreSegmentManager::ResetOffset() segment %d not"
                       "fully downloaded break the loop",(int)nHTTPSegKey);
          break;

        }
      }
      pSegment = (HttpSegmentDataStoreListElement *)StreamQ_next(
                     &m_DataStoreSegmentInUseList, &pSegment->m_link);
    } while (pSegment);
  }
  //This will take care of updating max download offset
  UpdateCachedInUseListInfo();
}

/**
 * Move elements from in-use list to swapped-list or release the
 * segment depending on whether the download for the element is fully
 * completed or not. Reset the start byte offset, and cached
 * info.
 */
void HttpSegmentDataStoreContainer::Reset()
{
  m_CachedMaxDownloadedOffset = 0;
  int numSegmentsInQ=StreamQ_cnt(&m_DataStoreSegmentInUseList);
  // flush the inuse list
  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_check(
      &m_DataStoreSegmentInUseList);

  if (pSegment)
  {
    do
    {
      HttpSegmentDataStoreBase *pHttpSegment =
        pSegment->m_pHttpSegmentDataStoreBase;

      if (NULL == pHttpSegment)
      {
        // Should never reach here.
        QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
          "HttpDataStoreSegmentManager::ClearInUseList() sanity check failed");
        break;
      }


      if(pHttpSegment->m_bIsPurgable)
      {
        HttpSegmentDataStoreListElement* pNextSegment = (HttpSegmentDataStoreListElement *)StreamQ_next(
                                      &m_DataStoreSegmentInUseList,&pSegment->m_link);

        //Remove the element from the inuse list
        StreamQ_delete(&pSegment->m_link);
        if (m_segmentPurgedHandler )
        {
          m_segmentPurgedHandler(pHttpSegment->GetKey(), m_pSegmentPurgedHandlerData);
        }

        if (true == pHttpSegment->m_bIsFullyDownloaded)
        {
          PutSegmentInSwappedList(pSegment);
        }
        else
        {
          ReleaseSegment(pSegment);
        }
        pSegment = pNextSegment;

      }
      else
      {
        m_CachedMaxDownloadedOffset = pSegment->GetStartOffset() +
                                          pSegment->GetNumBytesDownloaded();
        pSegment = (HttpSegmentDataStoreListElement *)StreamQ_next(
                     &m_DataStoreSegmentInUseList, &pSegment->m_link);

      }
      numSegmentsInQ--;

    } while (pSegment && numSegmentsInQ);
  }

  UpdateCachedInUseListInfo();
}

/**
 * @brief
 *  Reset internal data structures to initial state and update
 *  the start byte offset to nOffet.
 *
 * @param nOffset
 */
void HttpSegmentDataStoreContainer::SetWriteOffset(int64 /* nOffset */)
{
  Reset();
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "SetWriteOffset() not supported for segment manager");
}

/**
 * @brief
 *  Mark segment as download completed. Optimize for the current
 *  use-case where only one segment is downloaded at a time. In
 *  this case, the segment marked completed should be the last
 *  element in the in-use list.
 *
 * @param nKey
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS  on success
 *  HTTPDL_ERROR_ABORT on failure segment not found in in-use
 *                    list.
 */
HTTPDownloadStatus HttpSegmentDataStoreContainer::SetSegmentComplete(uint64 nKey, int64 nEndOffset)
{
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;

  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_last_check(&m_DataStoreSegmentInUseList);

  if (pSegment)
  {
    if ((uint64)pSegment->GetKey() != nKey)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "SetSegmentComplete() key '%d' not last element in-use", (int)nKey);
      pSegment = PeekInUseSegment(nKey);
    }

    if (pSegment)
    {
      pSegment->SetSegmentComplete(nEndOffset);
      UpdateCachedInUseListInfo();
      result = HTTPCommon::HTTPDL_SUCCESS;
      QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "SetSegmentComplete() result %d for key '%d', nOffset %lld, this %p",
                  result, (int)pSegment->GetKey(), nEndOffset, (void*)this);
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "SetSegmentComplete() key '%d' not found in inuse list", (int)nKey);
    }
  }

  return result;
}

/**
 * @brief
 *  Abort a segment.
 *
 * @param nKey
 *
 * @return HTTPDownloadStatus
 *  HTTPCommon::HTTPDL_SUCCESS on success
 *  HTTPCommon::HTTPDL_ERROR_ABORT on failure.
 */
HTTPDownloadStatus HttpSegmentDataStoreContainer::AbortSegment(uint64 nKey)
{
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
  bool isHeadElement = false;

  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_last_check(&m_DataStoreSegmentInUseList);

  if (pSegment)
  {
    if ((uint64)pSegment->GetKey() != nKey)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "AbortSegment() segment with key '%d' not last in-use element",
        (int)nKey);

      pSegment = PeekInUseSegment(nKey);
    }

    if (pSegment)
    {
      if (m_segmentPurgedHandler )
      {
        m_segmentPurgedHandler(pSegment->GetKey(), m_pSegmentPurgedHandlerData);
      }
      //Release segment
      StreamQ_delete(&pSegment->m_link);
      ReleaseSegment(pSegment);
      result = HTTPCommon::HTTPDL_SUCCESS;
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                    "AbortSegment() segment with key '%llu' aborted", nKey);
      UpdateCachedInUseListInfo();
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                    "AbortSegment() segment with key '%llu' not in-use", nKey);
    }
  }

  QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_LOW,
                "AbortSegment() result %d for key %llu", result, nKey);

  return result;
}

/**
 * Gets the total space occupied by the segments in use.
 */
int HttpSegmentDataStoreContainer::GetSpaceInUse()
{
  // ### TO DO ###.
  // Consider FileSystem as well when filesystem support is added.

  int totalLogicalUnitsHeap = 0;

  // subtract the amount of space in use
  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentInUseList);

  while (pSegment)
  {
    if (iHTTPBase::HEAP == pSegment->GetStorageType())
    {
      int64 numBytesDownloaded = pSegment->GetNumBytesDownloaded();
      int numLogicalUnits = 1 + (int)(numBytesDownloaded / HTTPHeapManager::GetChunkSize());
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "HttpSegmentDataStoreContainer::GetMaxAvailableSpace numDownloaded %lld numLogicalUnits %d",
                    numBytesDownloaded, numLogicalUnits);

      totalLogicalUnitsHeap += numLogicalUnits;
    }
    pSegment = (HttpSegmentDataStoreListElement *)StreamQ_next(
        &m_DataStoreSegmentInUseList, &pSegment->m_link);
  }

  int heapSpaceInUse = totalLogicalUnitsHeap * (int)HTTPHeapManager::GetChunkSize();

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "HttpSegmentDataStoreContainer::GetMaxAvailableSpace inUse %d",
                heapSpaceInUse);

  return heapSpaceInUse;
}

void HttpSegmentDataStoreContainer::SetMaxSwapSegments(
  int numSwapSegments)
{
  numSwapSegments = (numSwapSegments >= 0 ? numSwapSegments : 0);
  m_MaxSwapSegments = numSwapSegments;
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_FATAL,
    "HttpSegmentDataStoreContainer::SetMaxSwapSegments Updated this %p with maxSwap segments %d",
    (void*)this, numSwapSegments);
}

/**
 * Populate segments info into client owned structure.
 *
 * @param rHTTPSegmentsInfo client owned structure.
 * @param offset Offset from which segments are considered.
 *  The member variable 'maxSegments' should be populated by the
 *  client to specify the segmentInfo array size.
 */
HTTPDownloadStatus HttpSegmentDataStoreContainer::GetAvailableSegments(
  HTTPSegmentsInfo& rHTTPSegmentsInfo,
  int64 offset)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  int64 currentOffset = 0;

  if (rHTTPSegmentsInfo.maxSegments > 0)
  {
    memset(rHTTPSegmentsInfo.m_pHTTPSegmentInUseInfoArray,
           0,
           rHTTPSegmentsInfo.maxSegments * sizeof(HTTPSegmentInfo));
  }

  rHTTPSegmentsInfo.m_NumSegmentsInUse = 0;
  rHTTPSegmentsInfo.m_bStartByteOffset = -1;

  HttpSegmentDataStoreListElement *pSegmentIter =
    (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentInUseList);

  int numSegments = 0;

  // this ensures that segments for which the previous segment is not
  // fully downloaded and hence has startOffset of -1 are included as well.
  bool bStartingSegmentFound = false;

  while (pSegmentIter)
  {
    currentOffset = pSegmentIter->GetStartOffset();
    int64 numBytesDownloaded = pSegmentIter->GetNumBytesDownloaded();

    if ((currentOffset >= 0 && offset < (currentOffset + numBytesDownloaded)) ||
        (true == bStartingSegmentFound))
    {
      // include this segment if the 'offset' lies before the end of this segment + 1,
      // or if the startOffset is unknown.
      if(numSegments < rHTTPSegmentsInfo.maxSegments)
      {
        HTTPSegmentInfo& rSegment =
          rHTTPSegmentsInfo.m_pHTTPSegmentInUseInfoArray[numSegments];

        if (-1 == rHTTPSegmentsInfo.m_bStartByteOffset)
        {
          rHTTPSegmentsInfo.m_bStartByteOffset = currentOffset;
        }

        rSegment.m_Key = pSegmentIter->GetKey();
        rSegment.m_NumBytesDownloaded = numBytesDownloaded;
        rSegment.m_bIsFullyDownloaded = pSegmentIter->IsFullyDownloaded();
        rSegment.m_nStartOffset = pSegmentIter->GetStartOffset();
      }
      bStartingSegmentFound = true;
      ++numSegments;
    }

    pSegmentIter = (HttpSegmentDataStoreListElement *)StreamQ_next(
      &m_DataStoreSegmentInUseList, &pSegmentIter->m_link);

  }

  rHTTPSegmentsInfo.m_NumSegmentsInUse = numSegments;

  if(numSegments > rHTTPSegmentsInfo.maxSegments)
  {
    status = HTTPCommon::HTTPDL_INSUFFICIENT_BUFFER;
  }

  return status;

}

/**
 * @brief
 *  Get a buffer for segment with key 'nKey'. if the previous segment to
 *  segment in question is not yet completly downloaded then HTTP_WAIT is
 *  returned,the caller has to wait till HTTP_success is returned
 *  If the previous is downloaded then the startOffset is set for
 *  the this segment and valid buffer is returned.
 *
 *
 * @param pBuf
 * @param nBufSize
 * @param nKey
 *
 * @return HTTP_SUCCESS indicating success,
 *         HTTP_WAIT indicating that caller has to wait,
 *         as previous segment is not completly downloaded
 *         HTTP_FAILURE indicating failure.
 */
HTTPDownloadStatus HttpSegmentDataStoreContainer::GetBuffer(byte* &pBuf,
                                                            int64& nBufSize,
                                                            int64 nKey)
{
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;

  HttpSegmentDataStoreListElement *pSegment = PeekInUseSegment(nKey);

  if (pSegment)
    {
      if (true == pSegment->IsFullyDownloaded())
      {
        QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                    "GetBuffer() Segment with key '%d' is a completed segment",
                    (int)pSegment->GetKey());
      }
      else
      {
        result = HTTPCommon::HTTPDL_SUCCESS;
        if ( pSegment->GetStartOffset() == -1 )
        {
          result = HTTPCommon::HTTPDL_ERROR_ABORT;
          HttpSegmentDataStoreListElement *pPrevSegment =
          (HttpSegmentDataStoreListElement *)StreamQ_prev(&m_DataStoreSegmentInUseList,&pSegment->m_link);

          if ( pPrevSegment )
          {
            if ( pPrevSegment->IsFullyDownloaded() )
            {
              int64 startOffset = m_CachedMaxDownloadedOffset;
              pSegment->SetStartOffset(startOffset);
              result = HTTPCommon::HTTPDL_SUCCESS;
              QTV_MSG_PRIO4(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                           "GetBuffer: Updated StartOffset: %lld  for segment '%d'"
                           "last segment StartOffset: %lld, numBytesDownloaded: %lld:",
                           pSegment->GetStartOffset(), (int)pSegment->GetKey(),
                           pPrevSegment->GetStartOffset(),pPrevSegment->GetNumBytesDownloaded() );
            }
            else
            {
              QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                            "GetBuffer (%d) : Previous segment is not fully downloaded '%d'",
                            (int)nKey,(int)pPrevSegment->GetKey());
              result = HTTPCommon::HTTPDL_WAITING;
            }
          }
          else
          {
            QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                          "GetBuffer: Weird case Segment '%d' StartOffset not set"
                          "and Previous Segment is NULL",(int)pSegment->GetKey());
          }
        }

        if ( result == HTTPCommon::HTTPDL_SUCCESS )
        {
          result = (true == pSegment->GetBuffer(pBuf, nBufSize)
                    ? HTTPCommon::HTTPDL_SUCCESS
                    : HTTPCommon::HTTPDL_OUT_OF_MEMORY);
        }
      }
    }
  else
  {
    // empty list
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "GetBuffer() Empty in-use list");
  }

  return result;
}

/**
 * @brief
 *  Commit the write into memory for the buffer obtained via
 *  GetBuffer.
 *
 * @param pBuf
 * @param nBytes
 * @param nKey
 *
 * @return HTTPDownloadStatus
 */
HTTPDownloadStatus HttpSegmentDataStoreContainer::CommitBuffer(byte* pBuf,
                                                             int64 nBytes,
                                                             int64 nKey)
{
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;

  HttpSegmentDataStoreListElement *pSegment =  PeekInUseSegment(nKey);

  if (pSegment)
  {
    if (pSegment->GetStartOffset() == -1)
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_FATAL,
        "CommitBuffer Key %d. Concurrent downloads not supported. Download currently "
        "in key %d", (int)nKey, (int)pSegment->GetKey());

      }
      else
      {
        if (true == pSegment->IsFullyDownloaded())
        {
          QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                        "GetBuffer() Segment with key '%d' is a completed segment",
                      (int)pSegment->GetKey());
        }
        else
        {
          // ok
          result = (true == pSegment->CommitBuffer(pBuf, nBytes)
                    ? HTTPCommon::HTTPDL_SUCCESS : HTTPCommon::HTTPDL_ERROR_ABORT);

          if (HTTPCommon::HTTPDL_SUCCESS == result)
          {
              m_CachedMaxDownloadedOffset += nBytes;
          }
      }
    }
  }

  return result;
}

/**
 * @brief
 *  Read at most 'nBufSize' bytes into 'pBuf' at read offset
 *  'readByteOffset'. Reads will be allowed upto the end of the
 *  first segment that is not fully downloaded.
 *
 * @param readByteOffset
 * @param pBuf
 * @param nBufSize
 * @param pnRead
 *
 * @return DataSourceReturnCode
 */
DataSourceReturnCode HttpSegmentDataStoreContainer::Read(int64 readByteOffset,
                                                         byte *pBuf,
                                                         int nBufSize,
                                                         int *pnRead)
{
  DataSourceReturnCode result = iSourcePort::DS_FAILURE;

  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentInUseList);
  int64 totalByteOffset = 0;
  while(pSegment)
  {
    int64 numBytesDownloaded = pSegment->GetNumBytesDownloaded();
    int64 startByteOffset = pSegment->GetStartOffset();

    if ((readByteOffset >= startByteOffset) &&
        (readByteOffset < startByteOffset + numBytesDownloaded))

    {
      // found the segment
      *pnRead = (int)pSegment->Read(
        (int)(readByteOffset - startByteOffset), pBuf,  nBufSize);

      if (*pnRead >= 0)
      {
        result = iSourcePort::DS_SUCCESS;
      }

      break;
    }

    totalByteOffset = startByteOffset + numBytesDownloaded;
    if (false == pSegment->IsFullyDownloaded())
    {
      // reached the end of readable data.
      if(readByteOffset < totalByteOffset)
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "HttpSegmentDataStoreContainer::Read readByteOffset %lld < totalByteOffset %lld",
          readByteOffset, totalByteOffset);
      }
      break;
    }

    pSegment = (HttpSegmentDataStoreListElement *)StreamQ_next(
        &m_DataStoreSegmentInUseList, &pSegment->m_link);
  }

  if (iSourcePort::DS_SUCCESS != result &&
      readByteOffset >= totalByteOffset)
  {
    // reading furthur ahead than data is available
    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "HttpSegmentDataStoreContainer Read %d, readByteOffset %lld totalByteOffset %lld",
                  result, readByteOffset, totalByteOffset);
    result = iSourcePort::DS_WAIT;
  }

  return result;
}

/**
 * @brief
 *  Return the byte offset of one past the last readable byte
 *  offset.
 *
 * @return int64
 */
int64 HttpSegmentDataStoreContainer::GetMaxDownloadOffset() const
{
  return m_CachedMaxDownloadedOffset;
}

/**
 * Count the number of fully downloaded segments starting with
 * the one associated with byteOffset.
 *
 * @param byteOffset
 *
 * @return int32
 */
int32 HttpSegmentDataStoreContainer::GetNumAvailableSegments(int64 byteOffset)
{
  int32 numAvail = 0;
  int byteOffsetIndex = 0; // idx of the element in the in-use list

  if (byteOffset >= 0)
  {
    HttpSegmentDataStoreListElement *pSegmentIter =
      (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentInUseList);

    int64 currentOffset = 0;

    while (pSegmentIter)
    {
      currentOffset = pSegmentIter->GetStartOffset();
      int64 numBytesDownloaded = pSegmentIter->GetNumBytesDownloaded();

      if (false == pSegmentIter->IsFullyDownloaded())
      {
        QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
          "GetNumAvailableSegments() Break out at segment with key '%d'",
           (int)pSegmentIter->GetKey());
        break;
      }

      if ((byteOffset >= currentOffset) &&
          (byteOffset < currentOffset + numBytesDownloaded))
      {
        numAvail = GetNumInUseSegments() - byteOffsetIndex;
        break;
      }

      ++byteOffsetIndex;

      pSegmentIter = (HttpSegmentDataStoreListElement *)StreamQ_next(
        &m_DataStoreSegmentInUseList, &pSegmentIter->m_link);
    }
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetNumAvailableSegments() incorrect byteOffset %d < 0",
      (int)byteOffset);
  }

  return numAvail;
}

/**
 * @brief
 *  Return the byte offset represented by the first in-use
 *  element.
 *
 * @return int64
 */
int64 HttpSegmentDataStoreContainer::GetStartByteOffset()
{

  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentInUseList);
  int64 startByteOffset = 0;
  if(pSegment)
  {
    startByteOffset = pSegment->GetStartOffset();
  }
  return startByteOffset;
}

/**
 * @brief
 *  Move segments from inuse list to swapped list till the
 *  element associated with 'minReadByteOffset' is reached.
 *
 * @param minReadByteOffset
 */
void HttpSegmentDataStoreContainer::PurgeSegmentsForReadOffset(int64 minReadByteOffset)
{
  HttpSegmentDataStoreListElement *pSegment =
    (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentInUseList);

  while(1)
  {
    if (pSegment)
    {
      if ((minReadByteOffset >=
        pSegment->GetStartOffset() + pSegment->GetNumBytesDownloaded()) &&
        (true == pSegment->IsFullyDownloaded()))
      {
        if(pSegment->IsPurgable())
        {
          HttpSegmentDataStoreListElement* pNextSegment = (HttpSegmentDataStoreListElement *)StreamQ_next(
                                      &m_DataStoreSegmentInUseList,&pSegment->m_link);

          QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
            "HttpDataStoreSegmentManager::Read() Remove segment with key '%d' with"
            "startOffset '%lld' from in use list",
            (int)pSegment->GetKey(),pSegment->GetStartOffset());

          if (m_segmentPurgedHandler )
          {
            m_segmentPurgedHandler(pSegment->GetKey(), m_pSegmentPurgedHandlerData);
          }

          // delete the head element
          StreamQ_delete(&pSegment->m_link);
          PutSegmentInSwappedList(pSegment);
          // get the next segment from the in use list
          pSegment = pNextSegment;
        }
        else
        {
          QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
            "HttpDataStoreSegmentManager::Read() Segment with key '%d' with"
            "startOffset '%lld' is non purgable",
            (int)pSegment->GetKey(),pSegment->GetStartOffset());

          // get the next segment from the in use list
          pSegment = (HttpSegmentDataStoreListElement *)StreamQ_next(
            &m_DataStoreSegmentInUseList, &pSegment->m_link);
        }
      }
      else
      {
        // done parsing thru the segments in the inuse queue
        // has data less than minReadByteOffset
        break;
      }
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "PurgeSegmentsForReadOffset() empty inuse list for offset %lld",
        minReadByteOffset);
      break;
    }
  }
}

/**
 * @brief
 *  Get a free data store element based on the choice of
 *  storage. The queue element is allocated and returned.
 *
 * @param eDataStorage
 *
 * @return HttpSegmentDataStoreListElement*
 */
HttpSegmentDataStoreListElement *HttpSegmentDataStoreContainer::GetFreeSegment(
  iHTTPAPI::SegmentStorageType eDataStorage,
  HTTPDownloadStatus& result,
  HttpSegmentDataStoreBase  *pInitDataUnit)
{
  HttpSegmentDataStoreListElement *pSegment = NULL;
  result = HTTPCommon::HTTPDL_ERROR_ABORT;
  if ((iHTTPAPI::SEGMENT_DEFAULT == eDataStorage) ||
      (iHTTPAPI::SEGMENT_HEAP == eDataStorage))
  {
    HttpSegmentDataStoreListElement* pDataStoreHeapElement = QTV_New(HttpSegmentDataStoreListElement);
    QTV_NULL_PTR_CHECK(pDataStoreHeapElement, NULL);
    if (pInitDataUnit)
    {
      pDataStoreHeapElement->m_pHttpSegmentDataStoreBase = pInitDataUnit;
      (void)StreamQ_link( pDataStoreHeapElement,
                          &pDataStoreHeapElement->m_link);

      pSegment = (HttpSegmentDataStoreListElement *)(&pDataStoreHeapElement->m_link);
      pDataStoreHeapElement->bIsMemAllocatedLocally = false;
      result = HTTPCommon::HTTPDL_SUCCESS;
    }
    else
    {
      HttpSegmentDataStoreHeap* pHeapSegment = QTV_New(HttpSegmentDataStoreHeap);
      if(pHeapSegment)
      {
        pHeapSegment->SetHeapManager(m_pHeapMgr);
        pDataStoreHeapElement->m_pHttpSegmentDataStoreBase = pHeapSegment;
        (void)StreamQ_link( pDataStoreHeapElement,
                            &pDataStoreHeapElement->m_link );

        pSegment = (HttpSegmentDataStoreListElement *)(&pDataStoreHeapElement->m_link);
        pDataStoreHeapElement->bIsMemAllocatedLocally = true;
        result = HTTPCommon::HTTPDL_SUCCESS;
      }
      else
      {
        if(pDataStoreHeapElement)
        {
          QTV_Delete(pDataStoreHeapElement);
          pDataStoreHeapElement = NULL;
        }
       QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_FATAL,
                  "GetFreeSegment malloc failed for heapsegment");
      }
    }
  }
  else
  {
    // file system not implemented.
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_FATAL,
                  "GetFreeSegment storageType %d not supported", eDataStorage);
  }

  return pSegment;
}

/**
 * Find element with key 'nKey' in swapped list. If found, deque
 * from swapped-list and return it.
 *
 * @param nKey
 *
 * @return HttpSegment*
 *    ptr to cached element if found in swapped list,
 *    null if not found in swapped list.
 */
HttpSegmentDataStoreListElement* HttpSegmentDataStoreContainer::GetSwappedSegment(int64 nKey)
{
  HttpSegmentDataStoreListElement *pSegmentIter =
    (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentSwappedList);

  while (pSegmentIter)
  {
    if (pSegmentIter->GetKey() == nKey)
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
        "GetSwappedSegment() Found cached segment with key '%lld'", nKey);
      break;
    }

    pSegmentIter = (HttpSegmentDataStoreListElement *)StreamQ_next(
      &m_DataStoreSegmentSwappedList, &pSegmentIter->m_link);
  }

  if (pSegmentIter)
  {
    StreamQ_delete(&pSegmentIter->m_link);
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
      "GetSwappedSegment() Did not find cached segment with key '%lld'", nKey);
  }

  return pSegmentIter;
}

void HttpSegmentDataStoreContainer::ReleaseSegment(HttpSegmentDataStoreListElement *pSegment)
{
  if (pSegment)
  {
    if(pSegment->bIsMemAllocatedLocally)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "Segment with key '%d' released",(int)pSegment->GetKey());
      pSegment->Reset();
      if(pSegment->m_pHttpSegmentDataStoreBase)
      {
        QTV_Delete(pSegment->m_pHttpSegmentDataStoreBase);
        pSegment->m_pHttpSegmentDataStoreBase = NULL;
      }
    }
    QTV_Delete(pSegment);
    pSegment = NULL;
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "ReleaseSegment NULL pSegment");
  }
}

void HttpSegmentDataStoreContainer::PutSegmentInSwappedList(HttpSegmentDataStoreListElement *pSegment)
{
  if (pSegment)
  {
    int swapListSize = StreamQ_cnt(&m_DataStoreSegmentSwappedList);

    if (swapListSize >= m_MaxSwapSegments)
    {
      // Release the swapped segment
      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "PutSegmentInSwappedList: Release segment with key %d "
        "as swaplistcount %d >= max %d",
        (int)pSegment->GetKey(), swapListSize, m_MaxSwapSegments);

      ReleaseSegment(pSegment);
    }
    else
    {
      // put in the swapped list.
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "PutSegmentInSwappedList: Move segment with key %d to swaplist",
        (int)pSegment->GetKey());

      StreamQ_put(&m_DataStoreSegmentSwappedList, &pSegment->m_link);
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "PutSegmentInSwappedList NULL pSegment");
  }
}
/**
 * Sets start offset for the given segment.
 *
 * @param pSegment - Segment for which start offset needs to be calculated
 *
 * @return -true if start offset can be updated else false
 */
bool HttpSegmentDataStoreContainer::SetStartOffsetForSegment(HttpSegmentDataStoreListElement *pSegment)
{
  bool result = false;
  int64 startOffset = 0;
  if(pSegment)
  {
      //Get the last segment in the inuse list
      HttpSegmentDataStoreListElement *pLastSegment =
                     (HttpSegmentDataStoreListElement *)StreamQ_last_check(&m_DataStoreSegmentInUseList);
      if(pLastSegment)
      {
        if(pLastSegment->IsFullyDownloaded())
        {
          startOffset = m_CachedMaxDownloadedOffset;
          pSegment->SetStartOffset(startOffset);
          result = true;
        }
        else
        {
          result = true;
          QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                        "SetStartOffsetForSegment(): Previous segment is not fully downloaded '%d'"
                        "SetStartOffset is deferred  ", (int)pLastSegment->GetKey());
        }
      }
      else
      {
        pSegment->SetStartOffset(0);
        result = true;
      }
      QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                        "SetStartOffsetForSegment() StartOffset for segment '%d'"
                        "is set to '%d'", (int)pSegment->GetKey(),(int)pSegment->GetStartOffset());
  }
  else
  {
     QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                        "SetStartOffsetForSegment() Unexpected Null Segment");
  }
  return result;
}
void HttpSegmentDataStoreContainer::PutSegmentInInUseList(HttpSegmentDataStoreListElement *pSegment)
{
  if (pSegment)
  {
    if (pSegment->m_pHttpSegmentDataStoreBase)
    {
      StreamQ_put(&m_DataStoreSegmentInUseList, &pSegment->m_link);
    }
    else
    {
      // sanity check failed
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "PutSegmentInInUseList Failed to put segment in inuse list. Null m_pHttpSegmentDataStoreBase");
    }
  }
  else
  {
    // sanity check failed
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "PutSegmentInInUseList Failed to put segment in inuse list. Null pSegment");
  }
}

/**
 * @brief
 *  Find an return segment with 'key' in in-use list if it
 *  exists. The element is not removed from the in-use list.
 *
 * @param key
 *
 * @return HttpSegmentDataStoreListElement*
 *  Ptr to segment if found.
 *  NULL if not found.
 */
HttpSegmentDataStoreListElement *HttpSegmentDataStoreContainer::PeekInUseSegment(uint64 key)
{
  HttpSegmentDataStoreListElement *pInUseSegment = NULL;

  HttpSegmentDataStoreListElement *pSegmentIter =
    (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentInUseList);

  while (pSegmentIter)
  {
    HttpSegmentDataStoreBase *pHttpSegment =
      pSegmentIter->m_pHttpSegmentDataStoreBase;

    if (NULL == pHttpSegment)
    {
      // sanity check failed
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "PeekInUseSegment() sanity check failed for key '%lld'", key);
      break;
    }

    if ((uint64)pHttpSegment->GetKey() == key)
    {
      pInUseSegment = pSegmentIter;
      break;
    }

    pSegmentIter = (HttpSegmentDataStoreListElement *)StreamQ_next(
      &m_DataStoreSegmentInUseList, &pSegmentIter->m_link);
  }

  return pInUseSegment;
}

/**
 * @brief
 *  Find a segment to release from swapped list.
 *  Currently it swaps out the oldest element in the swapped
 *  list. But can be made more sophisticated later eg swaps out
 *  the element furthest away in the backward directed (or fwd
 *  direction if does not exist) from the current write pointer.
 */
void HttpSegmentDataStoreContainer::ReleaseSwappedSegment()
{
  HttpSegmentDataStoreListElement *pElem =
    (HttpSegmentDataStoreListElement *)StreamQ_get(&m_DataStoreSegmentSwappedList);

  if (NULL == pElem)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "ReleaseSwappedSegment() Empty swapped list");
  }
  else
  {
    ReleaseSegment(pElem);
  }
}


/**
 * @brief
 *  Return the number of segments in the swapped list.
 *
 * @return int
 */
int HttpSegmentDataStoreContainer::GetNumSwappedSegments()
{
  return StreamQ_cnt(&m_DataStoreSegmentSwappedList);
}

/**
 * @brief
 *  Return the number of segments in the inuse list.
 *
 * @return int
 */
int HttpSegmentDataStoreContainer::GetNumInUseSegments()
{
  return StreamQ_cnt(&m_DataStoreSegmentInUseList);
}

/**
 * @brief
 *  Update cached values of maxDownloadedOffset and number of
 *  fully downloaded contigious segments in in-use list. Should
 *  be called whenever a segment is added or removed from the
 *  inuse list.
 */
void HttpSegmentDataStoreContainer::UpdateCachedInUseListInfo()
{
  int numInUseDownloaded = 0;

  m_CachedMaxDownloadedOffset = 0;

  HttpSegmentDataStoreListElement *pSegmentIter =
    (HttpSegmentDataStoreListElement *)StreamQ_check(&m_DataStoreSegmentInUseList);

  while (pSegmentIter)
  {
    if(pSegmentIter->GetStartOffset() > -1)
    {
      m_CachedMaxDownloadedOffset = pSegmentIter->GetStartOffset();
      m_CachedMaxDownloadedOffset += pSegmentIter->GetNumBytesDownloaded();
    }

    if (false == pSegmentIter->IsFullyDownloaded())
    {
      break;
    }

    ++numInUseDownloaded;

    pSegmentIter = (HttpSegmentDataStoreListElement *)StreamQ_next(
      &m_DataStoreSegmentInUseList, &pSegmentIter->m_link);
  }
}

void HttpSegmentDataStoreContainer::PrintLists()
{

  QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
               "PrintLists() SWAPPED_LIST   ################################");
  PrintList(&m_DataStoreSegmentSwappedList);

  QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
               "PrintLists()  INUSE_LIST  **********************************");
  PrintList(&m_DataStoreSegmentInUseList);
  QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH, "PrintLists() Done");
}

void HttpSegmentDataStoreContainer::PrintList(StreamQ_type* q)
{
  QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
    "HttpSegmentDataStoreContainer::PrintList: Queue size %d", q->cnt);
  char strSegment[100];
  char logStr[200];
  const int lenStrSegment = (int)sizeof(strSegment);
  const int lenLogStr = (int)sizeof(logStr);

  HttpSegmentDataStoreListElement *pSegmentIter =
    (HttpSegmentDataStoreListElement *)StreamQ_check(q);

  logStr[0] = '\0';

  while (pSegmentIter)
  {
    pSegmentIter->GetPrintStr(strSegment, lenStrSegment);
    std_strlcat(logStr, strSegment, sizeof(logStr));

    if (std_strlen(logStr) >= lenLogStr - lenStrSegment)
    {
      // print
      QTV_MSG_SPRINTF_1(QTVDIAG_HTTP_STREAMING, "%s", logStr);
      logStr[0] = '\0';
    }

    pSegmentIter =
      (HttpSegmentDataStoreListElement *)StreamQ_next(q, &pSegmentIter->m_link);
  }

  if ('\0' != logStr)
  {
    QTV_MSG_SPRINTF_1(QTVDIAG_HTTP_STREAMING, "%s", logStr);
    logStr[0] = '\0';
  }
}

} // end namespace video
