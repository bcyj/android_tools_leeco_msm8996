#ifndef __HTTPDATAMANAGER_H__
#define __HTTPDATAMANAGER_H__
/************************************************************************* */
/**
 * HTTPDataManager.h
 * @brief Header file for HTTPDataManager.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPDataManager.h#12 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDataManager.h
** ======================================================================= */
#include "HTTPCommon.h"
#include "HTTPDataStoreBase.h"
#include "DataSourcePort.h"

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

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class HTTPDataManager: public iStreamPort
{
public:
  HTTPDataManager(bool& status);
  virtual ~HTTPDataManager();

  // Interface with HTTPDownloader
  HTTPDownloadStatus SetSessionStorageOption(const iHTTPAPI::DataStorageType eDataStorage,
                                             int64 nContentLength,
                                             tBufferReuseParams& buffParams,
                                             iHTTPFileSourceHelper *pHTTPFileSourceHelper,
                                             const char* pFilepath = NULL,
                                             iStreamPort* piStreamPort = NULL,
                                             SegmentPurgedHandler segmentPurgedHandler = NULL,
                                             void *pSegmentPurgedHandlerData = NULL,
                                             HTTPHeapManager *pHeapManager = NULL,
                                             int numSwapSegments = 0);
  HTTPDownloadStatus SetMaxSwapSegments(int numSwapSegments);
  HTTPDownloadStatus CreateSegment(
    uint64 nKey, iHTTPAPI::SegmentStorageType eDataStorage,
    int64 nStartOffset = -1,bool bPurge = true,
    HttpSegmentDataStoreBase *pInitDataUnit = NULL);
  HTTPDownloadStatus SetSegmentComplete(uint64 nKey, int64 nEndOffset = -1);
  HTTPDownloadStatus AbortSegment(uint64 nKey);
  HTTPDownloadStatus GetAvailableSegments(HTTPSegmentsInfo & rHTTPSegmentsInfo,
                                          int64 offset);
  HTTPDownloadStatus SetWriteOffset(int64 nOffset);
  HTTPDownloadStatus SetHTTPReadableDataInfo(iHTTPReadable* pIHTTPReadableDataInfo);
  HTTPDownloadStatus GetBuffer(byte* &pBuffer, int64& nBufSize, uint64 nKey = 0);
  HTTPDownloadStatus CommitBuffer(byte* pBuffer, int64 nBytes, uint64 nKey = 0);
  HTTPDownloadStatus IsSeekable(const int32 nSeekOffset);
  HTTPDownloadStatus Flush(int32 offset = -1);
  HTTPDownloadStatus ResetOffset(uint64 nKey);
  HTTPDownloadStatus SetEndofFile();
  HTTPDownloadStatus SaveDownloadedContent(const char* pFilePath);
  HTTPDownloadStatus DeleteDownloadedContent();

  //iStreamPort APIs
  virtual DataSourceReturnCode Read(unsigned char* pBuf,
                                    ssize_t nBufSize,
                                    ssize_t* pnRead);
  virtual DataSourceReturnCode Readable(iReadNotificationHandler const* pNotificationHandler);
  virtual DataSourceReturnCode Close();
  virtual DataSourceReturnCode GetContentLength(int64* pContentLength);
  virtual DataSourceReturnCode GetSourceType(DataSourceType* pSourceType);
  virtual DataSourceReturnCode GetNumBytesAvailable(int64* pNumBytesAvailable);
  virtual DataSourceReturnCode GetAvailableOffset(int64* pAvailableOffset,
                                                  bool* pbEOS);
  virtual DataSourceReturnCode GetStartOffset(int64* pStartOffset);
  virtual DataSourceReturnCode Write(const byte *pBuf, ssize_t nBufSize, ssize_t *pnWritten);
  virtual DataSourceReturnCode Seek(const int64 nOffset,
                                    const int nWhence,
                                    int64* pnOutOffset);

  virtual uint32 AddRef(){ return 0; };
  virtual uint32 Release(){ return 0; };
  virtual void* QueryInterface(const AEEIID /* iid */){ return NULL; };

private:
    HttpDataStoreBase * m_pHttpDataStore;
    char* m_pFilepath;
    int64 m_nContentLength;
    bool m_bDataStorageSwitchInProgress;

    void Delete();
};

} // namespace video
#endif // __HTTPDATAMANAGER_H__
