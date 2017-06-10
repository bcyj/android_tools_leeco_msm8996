#ifndef __HTTPDATASTOREBASE_H__
#define __HTTPDATASTOREBASE_H__
/************************************************************************* */
/**
 * HttpDataStoreBase.h
 * @brief Header file for HttpDataStore.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPDataStoreBase.h#14 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */
/* =======================================================================
**               Include files for HttpDataManagerBase.h
** ======================================================================= */

#include "HTTPCommon.h"
#include "DataSourcePort.h"

namespace video {

class HttpSegmentDataStoreBase;

/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef iSourcePort::DataSourceReturnCode DataSourceReturnCode;
struct tBufferReuseParams
{
  int32 nMaxBufSize;
  bool bInUse;
};

struct HTTPSegmentInfo
{
  // segment key
  uint64 m_Key;
  // num bytes downloaded in segment
  int64 m_NumBytesDownloaded;
  // true if segment was marked completed.
  bool m_bIsFullyDownloaded;
  //startoffset of the segment
  int64 m_nStartOffset;
};

struct HTTPSegmentsInfo
{
  // Populated by client to specify max m_pHTTPSegmentInUseInfoArray size.
  int maxSegments;

  // Pointer to array of size 'maxElements'
  HTTPSegmentInfo *m_pHTTPSegmentInUseInfoArray;

  // Absolute byte offset represented by the first byte of the first
  // segment returned.
  int64 m_bStartByteOffset;

  // if greater than maxSegments, then the array was truncated.
  int m_NumSegmentsInUse;
};

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPHeapManager;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class HttpDataStoreBase
{
public:
  HttpDataStoreBase(HTTPDownloadStatus &result);
  virtual ~HttpDataStoreBase() = 0;

  /**
   * Interface with HttpDownloadManager
   */
  static HTTPDownloadStatus SetSessionStorageOption(HttpDataStoreBase *& pHttpDataStore,
                                                    iHTTPAPI::DataStorageType eDataStorage,
                                                    int64 nContentLength,
                                                    tBufferReuseParams& buffParams,
                                                    iHTTPFileSourceHelper *pHTTPFileSourceHelper,
                                                    const char* pFilepath,
                                                    iStreamPort* piStreamPort,
                                                    SegmentPurgedHandler segmentPurgedHandler,
                                                    void *pSegmentPurgedHandlerData,
                                                    HTTPHeapManager *pHeapManager,
                                                    int numSwapSegments);

  // API's for HttpDownloader
  virtual HTTPDownloadStatus CreateSegment(
    uint64 nKey, iHTTPAPI::SegmentStorageType eDataStorage,
    int64 nStartOffset = -1, bool bPurge = true ,
    HttpSegmentDataStoreBase  *pInitDataUnit = NULL ) = 0;

  virtual HTTPDownloadStatus SetSegmentComplete(uint64 nKey,
                                                int64 nEndOffset = -1) = 0;

  virtual HTTPDownloadStatus AbortSegment(uint64 nKey) = 0;

  virtual HTTPDownloadStatus SetMaxSwapSegments(int /* numSwapSegments */)
  {
    return HTTPCommon::HTTPDL_ERROR_ABORT;
  }

  virtual HTTPDownloadStatus GetNumAvailableSegments(
    int32& nNumSegments, int64 byteOffset) = 0;

  virtual HTTPDownloadStatus GetAvailableSegments(
    HTTPSegmentsInfo & rHTTPSegmentsInfo, int64 offset) = 0;

  virtual HTTPDownloadStatus SetWriteOffset(int64 nOffset) = 0;

  virtual HTTPDownloadStatus GetBuffer(byte* &pBuf,
                                       int64& nBufSize,
                                       uint64 nKey = 0) = 0;
  virtual HTTPDownloadStatus CommitBuffer(byte* pBuf,
                                          int64 nBytes,
                                          uint64 nKey = 0) = 0;
  virtual HTTPDownloadStatus IsSeekable(const int32 /* nSeekOffset */)
  {
    return HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  virtual HTTPDownloadStatus Flush(int32 offset = -1) = 0;
  virtual HTTPDownloadStatus ResetOffset(uint64 /* nKey */)
  {
    return HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  virtual HTTPDownloadStatus SaveDownloadedContent(const char* pFilePath, byte* pOldFilePath) = 0;
  virtual HTTPDownloadStatus DeleteDownloadedContent(byte* pFilepath) = 0;
  HTTPDownloadStatus SetEndofFile();
  virtual DataSourceReturnCode Read(byte *pBuf, int nBufSize, int *pnRead) = 0;
  virtual DataSourceReturnCode Close() = 0;
  virtual DataSourceReturnCode GetMaxDownloadOffset(int64* pMaxDownloadOffset,
                                                    bool* pbEOS) = 0;
  virtual DataSourceReturnCode GetStartOffset(int64* pStartOffset) = 0;

  void SetHTTPReadableDataInfo(iHTTPReadable* pHTTPReadableDataInfo);
  DataSourceReturnCode Readable(iSourcePort::iReadNotificationHandler const* pNotificationHandler);
  DataSourceReturnCode GetNumBytesDownloaded(int64* pNumBytesDownloaded);
  DataSourceReturnCode Seek(const int64 nOffset, const int nWhence, int64* pnOutOffset);

protected:
  void AcquireCriticalSection();
  void ReleaseCriticalSection();

  // Helper functions for pure virtuals
  bool SetWriteOffsetBase(int64 nOffset);
  bool SetReadOffsetBase(int64 absoluteOffset, int64* pnOutOffset);

  bool Delete();
  bool IsEndofFile();
  void SetFileSize();
  int64 GetFileSize();

  /**
   * For Heap, m_pBuffer is the buffer that holds
   * the file. For FileSystem, it is the buffer that
   * is provided to the HttpDownloader to write data
   */
  byte *m_pBuffer;

  int64 m_nBufferSize;
  int64 m_nWriteOffset;
  int64 m_nReadOffset;
  bool m_fEndofFile;
  int64 m_nFileSize;
  iHTTPReadable* m_pHTTPReadableDataInfo;
  iSourcePort::iReadNotificationHandler* m_iReadNotificationHandler;
  MM_HANDLE m_pCriticalSection;
};

} //namespace video

#endif // __HTTPDATASTOREBASE_H__
