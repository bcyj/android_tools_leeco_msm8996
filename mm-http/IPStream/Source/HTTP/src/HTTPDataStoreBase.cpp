/************************************************************************* */
/**
 * HTTPDataStoreBase.cpp
 * @brief implementation of the HttpDataStoreBase. This class contains
 *   pure virtual functions to be defined by HttpDataStoreHeap
 *   and HttpDataStoreFileSystem.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPDataStoreBase.cpp#8 $
$DateTime: 2012/08/01 18:57:11 $
$Change: 2653906 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

// HttpDownload
#include "HTTPDataStoreBase.h"
#include "HTTPHeapManager.h"
#include "HTTPSegmentDataStoreManager.h"
#include "MMFile.h"
#include <SourceMemDebug.h>

namespace video {

/**
 * @brief ctor
 *
 * @param result
 * @param pEnv
 */
HttpDataStoreBase::HttpDataStoreBase
(
  HTTPDownloadStatus &result
 ) :
  m_pBuffer(NULL),
  m_nBufferSize(0),
  m_nWriteOffset(0),
  m_nReadOffset(0),
  m_fEndofFile(false),
  m_nFileSize(0),
  m_pHTTPReadableDataInfo(NULL),
  m_iReadNotificationHandler(NULL),
  m_pCriticalSection(NULL)
{
  result = HTTPCommon::HTTPDL_SUCCESS;
  if (MM_CriticalSection_Create(&m_pCriticalSection) || m_pCriticalSection == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "HttpDataStoreBase lock creation failed" );
    result = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
}

/**
 * @brief dtor
 */
HttpDataStoreBase::~HttpDataStoreBase()
{
  if (m_pBuffer)
  {
    QTV_Free(m_pBuffer);
    m_pBuffer = NULL;
  }
  if (m_pCriticalSection)
  {
    (void)MM_CriticalSection_Release(m_pCriticalSection);
    m_pCriticalSection = NULL;
  }
}

/**
 * @brief Get the handle to the proper type of derived class
 *
 * @param pHttpDataStore
 * @param eDataStorage
 * @param nContentLength
 * @param buffParams
 * @param pHTTPFileSourceHelper
 * @param pFilepath
 * @param pIStreamPort
 * @param segmentPurgedHandler
 * @param pSegmentPurgedHandlerData
 *
 * @return HTTPDownloadStatus
 */
HTTPDownloadStatus
HttpDataStoreBase::SetSessionStorageOption(HttpDataStoreBase *& pHttpDataStore,
                                           iHTTPAPI::DataStorageType eDataStorage,
                                           int64 /*nContentLength */,
                                           tBufferReuseParams& /* buffParams */,
                                           iHTTPFileSourceHelper *pHTTPFileSourceHelper,
                                           const char* /* pFilepath */,
                                           iStreamPort* /* pIStreamPort */,
                                           SegmentPurgedHandler segmentPurgedHandler,
                                           void *pSegmentPurgedHandlerData,
                                           HTTPHeapManager *pHeapManager,
                                           int numSwapSegments)
{
  HTTPDownloadStatus result = HTTPCommon::HTTPDL_SUCCESS;

  pHttpDataStore = NULL;
  switch (eDataStorage)
  {
    case (iHTTPAPI::SEGMENT_STORE):
      {

        pHttpDataStore = QTV_New_Args(HttpDataStoreSegmentManager,
                                      (result,
                                       numSwapSegments,
                                       pHeapManager,
                                       pHTTPFileSourceHelper, segmentPurgedHandler,
                                       pSegmentPurgedHandlerData));
      }
      break;

    default:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "SetSessionStorageOption invalid store type" );
      result = HTTPCommon::HTTPDL_ERROR_ABORT;
      break;
  }

  if (NULL == pHttpDataStore)
  {
    result = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
  }

  return result;
}


/**
 * @brief SetEndofFile
 */
HTTPDownloadStatus
HttpDataStoreBase::SetEndofFile()
{
  AcquireCriticalSection();
  m_fEndofFile = true;
  ReleaseCriticalSection();
  return HTTPCommon::HTTPDL_SUCCESS;
}

/**
 * @brief IsEndofFile
 */
bool
HttpDataStoreBase::IsEndofFile()
{
  bool bEOF = false;
  AcquireCriticalSection();
  bEOF = m_fEndofFile;
  ReleaseCriticalSection();
  return bEOF;
}

/**
 * @brief SetFileSize
 */
void
HttpDataStoreBase::SetFileSize()
{
  AcquireCriticalSection();
  if (m_nFileSize < m_nWriteOffset)
  {
    m_nFileSize = m_nWriteOffset;
  }
  ReleaseCriticalSection();
}

/**
 * @brief GetFileSize
 */
int64
HttpDataStoreBase::GetFileSize()
{
  int64 fileSize = 0;
  AcquireCriticalSection();
  fileSize = m_nFileSize;
  ReleaseCriticalSection();
  return fileSize;
}

/**
 * @brief Update write offset.
 *
 * @param nOffset - Position of the internal write offset.
 */
bool
HttpDataStoreBase::SetWriteOffsetBase(int64 nOffset)
{
  bool bOk = false;

  if (nOffset >= 0)
  {
    m_nWriteOffset = nOffset;
    bOk = true;
  }

  SetFileSize();

  return bOk;
}

bool
HttpDataStoreBase::SetReadOffsetBase(int64 absoluteOffset, int64* pnOutOffset)
{
  bool bOk = false;

  if (absoluteOffset >= 0 && pnOutOffset)
  {
    *pnOutOffset = m_nReadOffset = absoluteOffset;
     bOk = true;
  }

  return bOk;
}

DataSourceReturnCode
HttpDataStoreBase::GetNumBytesDownloaded(int64* pNumBytesDownloaded)
{
  DataSourceReturnCode result = iSourcePort::DS_FAILURE;

  if (pNumBytesDownloaded && m_pHTTPReadableDataInfo)
  {
    *pNumBytesDownloaded = m_pHTTPReadableDataInfo->GetTotalBytesReceived();
    result = iSourcePort::DS_SUCCESS;
  }

  return result;
}

DataSourceReturnCode
HttpDataStoreBase::Seek(const int64 nOffset, const int nWhence, int64* pnOutOffset)
{
  DataSourceReturnCode eStatusCode = iSourcePort::DS_FAILURE;

  if(nWhence == iSourcePort::DS_SEEK_CUR)
  {
    *pnOutOffset = m_nReadOffset;
    eStatusCode = iSourcePort::DS_SUCCESS;
  }
  else if(nWhence == iSourcePort::DS_SEEK_SET)
  {
    //nOffset is absoluteReadOffset
     if(SetReadOffsetBase(nOffset, pnOutOffset))
     {
       eStatusCode = iSourcePort::DS_SUCCESS;
     }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
       "nWhence is neither DS_SEEK_SET nor DS_SEEK_CURR" );
  }

  return eStatusCode;
}

/**
 * @brief acquire critical section for datastore
 */
void
HttpDataStoreBase::AcquireCriticalSection()
{
  (void)MM_CriticalSection_Enter(m_pCriticalSection);
}

/**
 * @brief relase critical section
 */
void
HttpDataStoreBase::ReleaseCriticalSection()
{
 (void)MM_CriticalSection_Leave(m_pCriticalSection);
}

void
HttpDataStoreBase::SetHTTPReadableDataInfo(iHTTPReadable* pHTTPReadableDataInfo)
{
  if (NULL != pHTTPReadableDataInfo)
  {
    m_pHTTPReadableDataInfo = pHTTPReadableDataInfo;
  }
}

DataSourceReturnCode HttpDataStoreBase::Readable(iSourcePort::iReadNotificationHandler const* pNotificationHandler)
{
  m_iReadNotificationHandler = (iSourcePort::iReadNotificationHandler *)pNotificationHandler;
  return iSourcePort::DS_SUCCESS;
}

bool HttpDataStoreBase::Delete()
{
  bool rslt = true;
  if (NULL != m_pBuffer)
  {
    QTV_Free(m_pBuffer);
    m_pBuffer = NULL;
  }
  return rslt;
}

} // namespace video
