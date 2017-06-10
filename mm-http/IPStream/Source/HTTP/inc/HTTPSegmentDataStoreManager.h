#ifndef _HTTP_SEGMENTDATASTOREMANAGER_H_
#define _HTTP_SEGMENTDATASTOREMANAGER_H_

/************************************************************************* */
/**
 * HTTPSegmentDataStoreManager.h
 * @brief Header file for HTTPSegmentDataStoreManager.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPSegmentDataStoreManager.h#11 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSegmentDataStoreManager.h
** ======================================================================= */

#include "HTTPCommon.h"
#include "HTTPSegmentDataStoreStructs.h"

namespace video {

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
class HTTPHeapManager;
class HttpSegmentDataStoreContainer;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class HttpDataStoreSegmentManager : public HttpDataStoreBase
{
public:
  /**
   *  c'tor Create an instance of segment manager
   *
   * @param result          HTTPDL_SUCCESS on success,
   *                        HTTPDL_ERROR_ABORT on failure
   */
  HttpDataStoreSegmentManager(HTTPDownloadStatus& result,
                              int maxSwapSegments,
                              HTTPHeapManager *pHeapManager,
                              iHTTPFileSourceHelper *pHTTPFileSourceHelper,
                              SegmentPurgedHandler segmentPurgedHandler,
                              void *pSegmentPurgedHandlerData);

  /**
   * d'tor
   */
  virtual ~HttpDataStoreSegmentManager();

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
  virtual HTTPDownloadStatus Flush(int32 offset = -1);
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
  virtual HTTPDownloadStatus ResetOffset(uint64 nKey);


  /**
   * @brief
   *  Not implemented initially.
   *
   * @param nOffset
   *
   * @return HTTPDownloadStatus
   */
  virtual HTTPDownloadStatus SetWriteOffset(int64 nOffset);

  /**
   * @brief
   *  Create a segment. The key must not be in use.
   *
   * @param nOffset The unique key corresponding to a segment.
   *        eDataStorage  Choice of segment store.
   *
   * @return HTTPDownloadStatus
   *  HTTPDL_SUCCESS        Segment created
   *  HTTPDL_EALREADY       Segment with index nOffset already
   *                        exists.
   *  HTTPDL_WAITING        Max segments are in use.
   *  HTTPDL_ERROR_ABORT    Failed to create segment
   */
  virtual HTTPDownloadStatus CreateSegment(
    uint64 nKey, iHTTPAPI::SegmentStorageType eDataStorage,
    int64 nStartOffset = -1,bool bPurge = true, HttpSegmentDataStoreBase *pInitDataUnit = NULL );

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
  virtual HTTPDownloadStatus SetSegmentComplete(uint64 nKey,
                                                int64 nEndOffset = -1);

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
  virtual HTTPDownloadStatus AbortSegment(uint64 nKey);

  /**
   * temporary function to set the number of max swap segments.
   */
  virtual HTTPDownloadStatus SetMaxSwapSegments(int numSwapSegments);

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
  virtual HTTPDownloadStatus GetAvailableSegments(HTTPSegmentsInfo & rHTTPSegmentsInfo,
                                                  int64 offset);

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
  virtual HTTPDownloadStatus GetBuffer(byte* &pBuf,
                                       int64& nBufSize,
                                       uint64 nKey = 0);

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
  virtual HTTPDownloadStatus CommitBuffer(byte* pBuf,
                                          int64 nBytes,
                                          uint64 nKey = 0);

  /**
   * @brief
   *  Not implemented initially.
   *
   * @param pFilePath
   * @param pOldFilePath
   *
   * @return HTTPDownloadStatus
   */
  virtual HTTPDownloadStatus SaveDownloadedContent(const char* pFilePath, byte* pOldFilePath);

  /**
   * @brief
   *  Not implemented initially.
   *
   * @param pFilepath
   *
   * @return HTTPDownloadStatus
   */
  virtual HTTPDownloadStatus DeleteDownloadedContent(byte* pFilepath);

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
  virtual DataSourceReturnCode Read(byte *pBuf, int nBufSize, int *pnRead);

  /**
   * @brief
   *  No-op on SegmentManager.
   *
   * @return DataSourceReturnCode
   *  DS_SUCCESS
   */
  virtual DataSourceReturnCode Close();

  /**
   * @brief
   *  Get the number of fully downloaded segments starting from
   *  the segment containing 'byteOffset'
   *
   * @param byteOffset
   * @param nNumSegments
   *
   * @return HTTPDownloadStatus
   *  HTTPDL_SUCCESS - on success
   *  HTTPDL_ERROR_ABORT - failure
   */
  virtual HTTPDownloadStatus GetNumAvailableSegments(int32& nNumSegments,
                                                     int64 byteOffset);
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
  virtual DataSourceReturnCode GetMaxDownloadOffset(int64* pMaxDownloadOffset,
                                                    bool* pbEOS);

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
  virtual DataSourceReturnCode GetStartOffset(int64* pStartOffset);

  void Print();

private:
  HttpDataStoreSegmentManager();
  HttpDataStoreSegmentManager(const HttpDataStoreSegmentManager&);
  HttpDataStoreSegmentManager& operator=(const HttpDataStoreSegmentManager&);

  HttpSegmentDataStoreContainer *m_pSegmentContainer;
  iHTTPFileSourceHelper *m_pHTTPFileSourceHelper;
  int64 m_PrevMinReadByteOffset;
};

} // end namespace video

#endif
