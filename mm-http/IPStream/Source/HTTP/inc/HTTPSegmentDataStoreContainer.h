#ifndef _HTTP_SEGMENT_CONTAINER_H_
#define _HTTP_SEGMENT_CONTAINER_H_

/************************************************************************* */
/**
 * HTTPSegmentDataStoreContainer.h
 * @brief Header file for HTTPSegmentDataStoreContainer. The data structures
 *  to maintain the elements free, swapped, in-use are chosen by this class.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPSegmentDataStoreContainer.h#13 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSegmentDataStoreContainer.h
** ======================================================================= */

#include "HTTPSegmentDataStoreStructs.h"

/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

namespace video {

class HttpSegmentDataStoreContainer
{
public:
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
   * @param segmentPurgedHandler callback handler when a segment is removed
   * @param pSegmentPurgedHandlerData callback data for the handler
   */
  HttpSegmentDataStoreContainer(bool& rslt,
                                int maxSwapSegments,
                                HTTPHeapManager *pHeapManager,
                                SegmentPurgedHandler segmentPurgedHandler,
                                void *pSegmentPurgedHandlerData);

  /**
   * @brief
   *  d'tor Release all resources.
   */
  ~HttpSegmentDataStoreContainer();

  /**
   * Move elements from in-use list to swapped-list or release the
   * segment depending on whether the download for the element is fully
   * completed or not. Reset the start byte offset, and cached
   * info.
   */
  void Reset();

  void ResetOffset(uint64 nKey);

  /**
   * @brief
   *  Reset internal data structures to initial state and update
   *  the start byte offset to nOffet.
   *
   * @param nOffset
   */
  void SetWriteOffset(int64 nOffset);

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
  HTTPDownloadStatus SetSegmentComplete(uint64 nKey, int64 nEndOffset = -1);

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
  HTTPDownloadStatus AbortSegment(uint64 nKey);

  /**
   * Gets the total space occupied by the segments in use.
   */
  int GetSpaceInUse();

  void SetMaxSwapSegments(int numSwapSegments);

  /**
   * Populate segments info into client owned structure.
   *
   * @param rHTTPSegmentsInfo client owned structure.
   * @param offset Offset from which segments are considered.
   *  The member variable 'maxSegments' should be populated by the
   *  client to specify the segmentInfo array size.
   * if the client specifies maxsegments as 0,maxsegments is populated
   * to number of segments in in-use list and status insufficient buffer
   * is returned
   * @return - HTTPDL_INSUFFICIENT_BUFFER if maxsegments < numsegments
   *           HTTPDL_SUCCESS - success case
   *           HTTPDL_ERROR_ABORT - otherwise
   */
  HTTPDownloadStatus GetAvailableSegments(HTTPSegmentsInfo& rHTTPSegmentsInfo, int64 offset);

  /**
   * @brief
   *  Get a buffer for segment with key 'nKey'. This is optimized
   *  for sequential downloads where the last element in the
   *  in-use list should be the onle associated with nKey.
   *
   * @param pBuf
   * @param nBufSize
   * @param nKey
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus GetBuffer(byte* &pBuf, int64& nBufSize, int64 nKey);

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
  HTTPDownloadStatus CommitBuffer(byte* pBuf, int64 nBytes, int64 nKey);

  /**
   * @brief
   *  Read at most 'nBufSize' bytes into 'pBuf' at read offset
   *  'readByteOffset'.
   *
   * @param readByteOffset
   * @param pBuf
   * @param nBufSize
   * @param pnRead
   *
   * @return DataSourceReturnCode
   */
  DataSourceReturnCode Read(
    int64 readByteOffset, byte *pBuf, int nBufSize, int *pnRead);

  /**
   * @brief
   *  Return the byte offset of one past the last readable byte
   *  offset.
   *
   * @return int64
   */
  int64 GetMaxDownloadOffset() const;

  /**
   * Count the number of fully downloaded segments starting with
   * the one associated with byteOffset
   *
   * @param byteOffset
   *
   * @return int32
   */
  int32 GetNumAvailableSegments(int64 byteOffset);

  /**
   * @brief
   *  Return the byte offset represented by the first in-use
   *  element.
   *
   * @return int64
   */
  int64 GetStartByteOffset() ;

  /**
   * @brief
   *  Move segments from inuse list to swapped list till the
   *  element associated with 'minReadByteOffset' is reached.
   *
   * @param minReadByteOffset
   */
  void PurgeSegmentsForReadOffset(int64 minReadByteOffset);

  /**
   * @brief
   *  Get a free data store element based on the choice of
   *  storage. The queue element is allocated and returned.
   *
   * @param eDataStorage
   *
   * @return HttpSegmentDataStoreListElement*
   */
  HttpSegmentDataStoreListElement *GetFreeSegment(
    iHTTPAPI::SegmentStorageType eDataStorage,
    HTTPDownloadStatus& result,
    HttpSegmentDataStoreBase  *pInitDataUnit = NULL);

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
  HttpSegmentDataStoreListElement* GetSwappedSegment(int64 nKey);

  void ReleaseSegment(HttpSegmentDataStoreListElement *pSegment);
  void PutSegmentInSwappedList(HttpSegmentDataStoreListElement *pSegment);
  void PutSegmentInInUseList(HttpSegmentDataStoreListElement *pSegment);
  bool SetStartOffsetForSegment(HttpSegmentDataStoreListElement *pSegment);


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
  HttpSegmentDataStoreListElement *PeekInUseSegment(uint64 key);

  /**
   * @brief
   *  Find a segment to release from swapped list.
   *  Currently it swaps out the oldest element in the swapped
   *  list. But can be made more sophisticated later eg swaps out
   *  the element furthest away in the backward directed (or fwd
   *  direction if does not exist) from the current write pointer.
   */
  void ReleaseSwappedSegment();

  /**
   * @brief
   *  Return the number of segments in the swapped list.
   *
   * @return int
   */
  int GetNumSwappedSegments();

  /**
   * @brief
   *  Return the number of segments in the inuse list.
   *
   * @return int
   */
  int GetNumInUseSegments();

  /**
   * @brief
   *  Update cached values of maxDownloadedOffset and number of
   *  fully downloaded contigious segments in in-use list. Should
   *  be called whenever a segment is added or removed from the
   *  inuse list.
   */
  void UpdateCachedInUseListInfo();

  void PrintLists();

private:
  void PrintList(StreamQ_type* q);

  StreamQ_type m_DataStoreSegmentSwappedList;
  StreamQ_type m_DataStoreSegmentInUseList;

  int m_MaxSwapSegments;

  int64 m_CachedMaxDownloadedOffset;

  HTTPHeapManager *m_pHeapMgr;
  SegmentPurgedHandler m_segmentPurgedHandler;
  void *m_pSegmentPurgedHandlerData;
};

} // end namespace video

#endif
