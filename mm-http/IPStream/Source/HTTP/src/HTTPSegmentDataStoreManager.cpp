/************************************************************************* */
/**
 * HTTPSegmentDataStoreManager.cpp
 * @brief implementation of the HttpDataStoreSegmentManager.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPSegmentDataStoreManager.cpp#22 $
$DateTime: 2013/03/29 03:23:21 $
$Change: 3547279 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPSegmentDataStoreContainer.h"
#include "HTTPSegmentDataStoreManager.h"
#include "SourceMemDebug.h"

namespace video {

/**
 *  c'tor Create an instance with maxSegments that will be
 *  tracked.
 *
 * @param result          HTTPDL_SUCCESS on success,
 *                        HTTPDL_ERROR_ABORT on failure
 * @param numMaxSegments  Max segments that may be in use.
 * @param pHTTPFileSourceHelper file source helper interface
 * @param segmentPurgedHandler callback handler to send segment
 *                             removal notifications
 * @param pSegmentPurgedHandlerData data assocaited with the call
 *                                  back handler
 */
HttpDataStoreSegmentManager::HttpDataStoreSegmentManager(
  HTTPDownloadStatus& result,
  int maxSwapSegments,
  HTTPHeapManager *pHeapManager,
  iHTTPFileSourceHelper *pHTTPFileSourceHelper,
  SegmentPurgedHandler segmentPurgedHandler,
  void *pSegmentPurgedHandlerData) :
    HttpDataStoreBase(result),
    m_pSegmentContainer(NULL),
    m_pHTTPFileSourceHelper(pHTTPFileSourceHelper),
    m_PrevMinReadByteOffset(0)
#ifdef HTTP_STREAMING_TEST_SEGMENTDATASTORE_MANAGER
    ,m_pTestHttpDataStoreSegmentManager(NULL)
#endif
{
  bool rslt = false;

  if (NULL == m_pHTTPFileSourceHelper)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "HttpDataStoreSegmentManager c'tor: NULL pHTTPFileSourceHelper");
  }
  else
  {
    m_pSegmentContainer = QTV_New_Args(
      HttpSegmentDataStoreContainer, (rslt, maxSwapSegments, pHeapManager,
                                      segmentPurgedHandler,
                                      pSegmentPurgedHandlerData ));

    result = ((m_pSegmentContainer && true == rslt)
              ? HTTPCommon::HTTPDL_SUCCESS
              : HTTPCommon::HTTPDL_ERROR_ABORT);
  }

  QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_LOW,
    "HttpDataStoreSegmentManager c'tor: result %d",
    result);
}

/**
 * d'tor
 */
HttpDataStoreSegmentManager::~HttpDataStoreSegmentManager()
{
  if (m_pSegmentContainer)
  {
    QTV_Delete(m_pSegmentContainer);
    m_pSegmentContainer = NULL;
  }

  QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
               "HttpDataStoreSegmentManager destroyed");
}

/**
 * @brief
 *  Flush all data in-use. Segments that are fully downloaded
 *  may be cached for efficiency.
 *
 * @param offset
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS  on success
 *  HTTPDL_ERROR_ABORT on failure
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::Flush(int32 /* offset */)
{
  HTTPDownloadStatus result = video::HTTPCommon::HTTPDL_SUCCESS;

  if (m_pSegmentContainer)
  {
    AcquireCriticalSection();
    m_PrevMinReadByteOffset = 0;
    m_pSegmentContainer->Reset();
    m_fEndofFile=false;
    ReleaseCriticalSection();
  }

  return result;
}
/**
 * @brief
 *  Flushes all data from in-use list before the given key.
 *
 * @param nKey
 * @param bResetOffset - if true offset for segments after nKey
 *                       is reset to end of first non purgable segment or 0.
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS  on success
 *  HTTPDL_ERROR_ABORT on failure
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::ResetOffset(uint64 nKey)
 {
    HTTPDownloadStatus result = video::HTTPCommon::HTTPDL_SUCCESS;

  if (m_pSegmentContainer)
  {
    AcquireCriticalSection();
    m_PrevMinReadByteOffset = 0;
    m_pSegmentContainer->ResetOffset(nKey);
    ReleaseCriticalSection();
  }

  return result;
 }
/**
 * @brief
 *  Not implemented initially.
 *
 * @param nOffset
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS, on success
 *  HTTPDL_ERROR_ABORT, on failure
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::SetWriteOffset(int64 /* nOffset */)
{
  return HTTPCommon::HTTPDL_ERROR_ABORT;
}

/**
 * @brief
 *  Create a segment. The key must not be in use.
 *
 * @param nKey          The unique key corresponding to a segment.
 *        eDataStorage  Choice of segment store.
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS        Segment created
 *  HTTPDL_EALREADY       Segment with index nOffset already
 *                        exists.
 *  HTTPDL_WAITING        Max segments are in use.
 *  HTTPDL_ERROR_ABORT    Failed to create segment
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::CreateSegment(
  uint64 nKey, iHTTPAPI::SegmentStorageType eDataStorage,
  int64 /* nStartOffset */, bool bPurge, HttpSegmentDataStoreBase *pInitDataUnit)
{
  QTV_NULL_PTR_CHECK(m_pSegmentContainer, HTTPCommon::HTTPDL_ERROR_ABORT);
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
  HttpSegmentDataStoreListElement *pSegment = NULL;

  AcquireCriticalSection();

  if (NULL != m_pSegmentContainer->PeekInUseSegment(nKey))
  {
    QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "CreateSegment() segment with key '%d' in use", (int)nKey);
  }
  else
  {
    // try to find a cached segment
    pSegment = m_pSegmentContainer->GetSwappedSegment(nKey);

    if (pSegment)
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
        "CreateSegment() Reuse segment with key '%d' from swapped list", (int)nKey);

      result = HTTPCommon::HTTPDL_EXISTS;
    }
    else
    {
      pSegment = m_pSegmentContainer->GetFreeSegment(eDataStorage,result, pInitDataUnit);
      int numSwappedSegments = m_pSegmentContainer->GetNumSwappedSegments();
      if (NULL == pSegment && numSwappedSegments > 0)
      {
        m_pSegmentContainer->ReleaseSwappedSegment();
        pSegment = m_pSegmentContainer->GetFreeSegment(eDataStorage,result, pInitDataUnit);
      }
      if(pSegment == NULL)
      {
        QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
          "CreateSegment() Failed to get a free segment");
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
          "CreateSegment() Segment for key '%d' created", (int)nKey);

      }
    }
  }

  if (pSegment)
  {
    if (HTTPCommon::HTTPDL_EXISTS != result)
    {
      pSegment->SetKey(nKey);
    }
    pSegment->SetPurgeFlag(bPurge);
    if(m_pSegmentContainer->SetStartOffsetForSegment(pSegment))
    {
      m_pSegmentContainer->PutSegmentInInUseList(pSegment);

      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
        "CreateSegment() Moved a segment to inUseList for key '%d'", (int)nKey);
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
        "CreateSegment() Error in setting startoffset for key '%d'", (int)nKey);
      m_pSegmentContainer->ReleaseSegment(pSegment);
      result = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }

  ReleaseCriticalSection();

  QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                "CreateSegment() key '%d' result '%d'", (int)nKey, result);

  return result;
}

/**
 * @brief
 *  Mark the segment as fully downloaded.
 *
 * @param nKey
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS - on success
 *  HTTPDL_ERROR_ABORT - failure
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::SetSegmentComplete(uint64 nKey,int64 nEndOffset)
{
  QTV_NULL_PTR_CHECK(m_pSegmentContainer, HTTPCommon::HTTPDL_ERROR_ABORT);
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;

  AcquireCriticalSection();

  result = m_pSegmentContainer->SetSegmentComplete(nKey,nEndOffset);

  ReleaseCriticalSection();

  return result;
}

/**
 * @brief
 *  Mark the segment as aborted to remove from in-use list.
 *
 * @param nOffset
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS - on success
 *  HTTPDL_ERROR_ABORT - failure
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::AbortSegment(uint64 nKey)
{
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(m_pSegmentContainer, HTTPCommon::HTTPDL_ERROR_ABORT);

  AcquireCriticalSection();
  result = m_pSegmentContainer->AbortSegment(nKey);
  ReleaseCriticalSection();

  return result;
}

/**
 * temporary function to set the number of max swap segments.
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::SetMaxSwapSegments(
  int numSwapSegments)
{
  QTV_NULL_PTR_CHECK(m_pSegmentContainer, HTTPCommon::HTTPDL_ERROR_ABORT);

  AcquireCriticalSection();
  m_pSegmentContainer->SetMaxSwapSegments(numSwapSegments);
  ReleaseCriticalSection();

  return HTTPCommon::HTTPDL_SUCCESS;
}

/**
 *
 * @param rHTTPSegmentsInfo
 *
 * @return HTTPDownloadStatus client owned structure.
 *  The member variable 'maxSegments' should be populated by the
 *  client to specify the segmentInfo array size.
 *
 *  HTTPCommon::HTTPDL_SUCCESS on SUCCESS
 *  HTTPCommon::HTTPDL_ERROR_ABORT on failure.
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::GetAvailableSegments(
  HTTPSegmentsInfo & rHTTPSegmentsInfo,
  int64 offset)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  QTV_NULL_PTR_CHECK(m_pSegmentContainer, status);

  AcquireCriticalSection();
  status = m_pSegmentContainer->GetAvailableSegments(rHTTPSegmentsInfo, offset);
  ReleaseCriticalSection();

  return status;
}

/**
 * Get a buffer to write to for segment with key 'key'. This may
 * result in memory allocation if there are no more bytes to
 * write to.
 *
 * @param pBuf
 * @param nBufSize
 * @param nKey
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS: on success. pBuffer is non-null and nBufSize > 0
 *  HTTPDL_ERROR_ABORT: failure
 *  HTTPDL_OUT_OF_MEMORY: failed to allocate memory.
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::GetBuffer(byte* &pBuf,
                                                          int64& nBufSize,
                                                          uint64 nKey)
{
  pBuf = NULL;
  QTV_NULL_PTR_CHECK(m_pSegmentContainer, HTTPCommon::HTTPDL_ERROR_ABORT);
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
  int64 savedBufSize = nBufSize;

  AcquireCriticalSection();
  result = m_pSegmentContainer->GetBuffer(pBuf, nBufSize, nKey);

  if (HTTPCommon::HTTPDL_OUT_OF_MEMORY == result)
  {
    // Failed to allocate memory from HTTPHeapManager. Try to release swapped
    // segment and retry.
    if (m_pSegmentContainer->GetNumSwappedSegments() > 0)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "GetBuffer()  Key %d. HTTPHeapManager out of memory. "
                    "Release swapped segment ", (int)nKey);
      m_pSegmentContainer->ReleaseSwappedSegment();

      nBufSize = savedBufSize;
      result = m_pSegmentContainer->GetBuffer(pBuf, nBufSize, nKey);
    }
  }

  ReleaseCriticalSection();

  QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "GetBuffer() result %d, key %d, pBuf %p, bufSize %lld",
                result, (int)nKey, (void *)pBuf, nBufSize);

  return result;
}

/**
 * @brief
 *  Called after writing data on buffer obtained from GetBuffer.
 *
 * @param pBuf
 * @param nBytes
 * @param nKey
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS: on success
 *  HTTPDL_ERROR_ABORT: on failure
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::CommitBuffer(byte* pBuf,
                                                           int64 nBytes,
                                                           uint64 nKey)
{
  QTV_NULL_PTR_CHECK(m_pSegmentContainer, HTTPCommon::HTTPDL_ERROR_ABORT);
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;

  AcquireCriticalSection();
  result = m_pSegmentContainer->CommitBuffer(pBuf, nBytes, nKey);
  ReleaseCriticalSection();

  if(NULL != m_iReadNotificationHandler &&
     HTTPCommon::HTTPDL_SUCCESS == result)
  {
    m_iReadNotificationHandler->Notify();
  }

  QTV_MSG_PRIO5(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "CopyBuffer() result %d this %p, key %d, pBuf %p, nBytes %lld",
                result, (void *)this, (int)nKey, (void *)pBuf, nBytes);

  return result;
}

/**
 * @brief
 *  Not implemented initially.
 *
 * @param pFilePath
 * @param pOldFilePath
 *
 * @return HTTPDownloadStatus
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::SaveDownloadedContent(const char* /* pFilePath */, byte* /* pOldFilePath */)
{
  return HTTPCommon::HTTPDL_ERROR_ABORT;
}

/**
 * @brief
 *  Not implemented initially.
 *
 * @param pFilepath
 *
 * @return HTTPDownloadStatus
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::DeleteDownloadedContent(byte* /* pFilepath */)
{
  return HTTPCommon::HTTPDL_ERROR_ABORT;
}

/**
 * @brief
 *  Read a max of 'nBufSize' bytes starting from the read offset
 *  updated by iStreamPort::Seek.
 *
 * @param pBuf      ptr to client buffer
 * @param nBufSize  client buffer size
 * @param pnRead    num bytes read
 *
 * @return DataSourceReturnCode
 *  iSourcePort::DS_SUCCESS, on success
 *  iSourcePort::DS_WAIT, data not available
 *  iSourcePort::DS_FAILURE, on failure
 */
DataSourceReturnCode HttpDataStoreSegmentManager::Read(byte *pBuf,
                                                       int nBufSize,
                                                       int *pnRead)
{
  QTV_NULL_PTR_CHECK(pBuf, iSourcePort::DS_FAILURE);
  QTV_NULL_PTR_CHECK(pnRead, iSourcePort::DS_FAILURE);

  DataSourceReturnCode result = iSourcePort::DS_FAILURE;
  *pnRead = 0;

  AcquireCriticalSection();

  result = m_pSegmentContainer->Read(m_nReadOffset, pBuf, nBufSize, pnRead);

  if (iSourcePort::DS_SUCCESS == result)
  {
    if (*pnRead > 0)
    {
      m_nReadOffset += *pnRead;

      // check if one more more inuse head segments need to be swapped out.
      int64 minReadByteOffset = 0;

      if (m_pHTTPFileSourceHelper)
      {
        uint64 currPos = MAX_UINT64_VAL;

        if (false == m_pHTTPFileSourceHelper->GetMinimumMediaOffset(currPos))
        {
          currPos = 0;
          result = iSourcePort::DS_FAILURE;
        }

        minReadByteOffset = (int64)currPos;
      }

      if (iSourcePort::DS_FAILURE != result &&
          ((minReadByteOffset > 0) &&
           (m_nReadOffset >= m_PrevMinReadByteOffset)&&
           (minReadByteOffset >= m_PrevMinReadByteOffset)))
      {
        m_pSegmentContainer->PurgeSegmentsForReadOffset(minReadByteOffset);
        m_PrevMinReadByteOffset = minReadByteOffset;
      }
    }
  }
  else
  {
    if ((iSourcePort::DS_WAIT == result) && (true == m_fEndofFile))
    {
      QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH, "Read() EOF");
      result = video::iSourcePort::DS_SUCCESS;
    }
  }

  ReleaseCriticalSection();

  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "Read() result %d, at readOffset %d, numRead %d",
                result, (int)m_nReadOffset - *pnRead, *pnRead);

  return result;
}

/**
 * @brief
 *  No-op on SegmentManager.
 *
 * @return DataSourceReturnCode
 *  DS_SUCCESS
 */
DataSourceReturnCode HttpDataStoreSegmentManager::Close()
{
  return iSourcePort::DS_SUCCESS;
}

/**
 * @brief
 *  Get the number of fully downloaded segments starting from
 *  the segment containing 'byteOffset'
 *
 * @param nNumSegments
 * @param byteOffset
 *
 * @return HTTPDownloadStatus
 *  HTTPDL_SUCCESS - on success
 *  HTTPDL_ERROR_ABORT - failure
 */
HTTPDownloadStatus HttpDataStoreSegmentManager::GetNumAvailableSegments(
  int32& nNumSegments, int64 byteOffset)
{
  QTV_NULL_PTR_CHECK(m_pSegmentContainer, HTTPCommon::HTTPDL_ERROR_ABORT);

  AcquireCriticalSection();
  nNumSegments = m_pSegmentContainer->GetNumAvailableSegments(byteOffset);
  ReleaseCriticalSection();

  QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                "GetNumAvailableSegments() numAvail %d for byteOffset %lld",
                nNumSegments, byteOffset);

  return HTTPCommon::HTTPDL_SUCCESS;
}

/**
 * @brief
 *  Get the byte offset of the download.
 *
 * @param pMaxDownloadOffset
 * @param pbEOS
 *
 * @return DataSourceReturnCode
 *  iSourcePort::DS_SUCCESS, on success
 *  iSourcePort::DS_FAILURE, on failure
 */
DataSourceReturnCode HttpDataStoreSegmentManager::GetMaxDownloadOffset(
  int64* pMaxDownloadOffset, bool* pbEOS)
{
  QTV_NULL_PTR_CHECK(pMaxDownloadOffset, iSourcePort::DS_FAILURE);
  QTV_NULL_PTR_CHECK(pbEOS, iSourcePort::DS_FAILURE);

  AcquireCriticalSection();

  *pbEOS = m_fEndofFile;
  *pMaxDownloadOffset = m_pSegmentContainer->GetMaxDownloadOffset();

  ReleaseCriticalSection();

  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "GetMaxDownloadOffset() maxOffset %d, isEos %d",
                (int)*pMaxDownloadOffset, *pbEOS);

  return iSourcePort::DS_SUCCESS;
}

/**
 * @brief
 *  Return the byte offset corresponding to the start of the
 *  byte range in-use for iStreamPort::read.
 *
 * @param pStartOffset
 *
 * @return DataSourceReturnCode
 *  iSourcePort::DS_SUCCESS, on success
 *  iSourcePort::DS_FAILURE, on failure
 */
DataSourceReturnCode HttpDataStoreSegmentManager::GetStartOffset(
  int64* pStartOffset)
{
  QTV_NULL_PTR_CHECK(pStartOffset, iSourcePort::DS_FAILURE);

  AcquireCriticalSection();
  *pStartOffset = m_pSegmentContainer->GetStartByteOffset();
  ReleaseCriticalSection();

  return iSourcePort::DS_SUCCESS;
}

void HttpDataStoreSegmentManager::Print()
{
  QTV_MSG_PRIO3(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
    "HttpDataStoreSegmentManager::Print() startByteOffset %d, readOffset %d, "
    "maxDownloadOffset %lld",
    (int)m_pSegmentContainer->GetStartByteOffset(), (int)m_nReadOffset,
    m_pSegmentContainer->GetMaxDownloadOffset());

  m_pSegmentContainer->PrintLists();
}

} // end namespace video
