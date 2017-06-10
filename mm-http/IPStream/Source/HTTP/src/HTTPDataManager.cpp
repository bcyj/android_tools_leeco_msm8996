/************************************************************************* */
/**
 * HTTPDataManager.cpp
 * @brief Header file for HTTPDataManager.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPDataManager.cpp#17 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDataManager.cpp
** ======================================================================= */

#include "HTTPDataManager.h"
#include "MMFile.h"
#include <SourceMemDebug.h>

namespace video {
/* =======================================================================
                        Data Declarations
   ======================================================================= */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

  /**
   * @brief HTTPDataManager
   *
   * @param status
   */
  HTTPDataManager::HTTPDataManager(bool& status):
  m_pHttpDataStore(NULL),
  m_pFilepath(NULL),
  m_nContentLength(0),
  m_bDataStorageSwitchInProgress(false)
  {
    status = true;
  }

  /**
   * @brief ~HTTPDataManager
   */
  HTTPDataManager::~HTTPDataManager()
  {
    Delete();
  }

  /**
   * @brief SetSessionStorageOption
   *
   * @param eDataStorage
   * @param nContentLength
   * @param buffParams
   * @param pHTTPFileSourceHelper
   * @param pFilepath
   * @param piStreamPort
   * @param segmentPurgedHandler
   * @param pSegmentPurgedHandlerData
   * @param pHeapManager
   *
   * @return HTTPDownloadStatus
   */

  HTTPDownloadStatus HTTPDataManager::SetSessionStorageOption(const iHTTPAPI::DataStorageType eDataStorage,
                                                              int64 nContentLength,
                                                              tBufferReuseParams& buffParams,
                                                              iHTTPFileSourceHelper *pHTTPFileSourceHelper,
                                                              const char* pFilepath,
                                                              iStreamPort* piStreamPort,
                                                              SegmentPurgedHandler segmentPurgedHandler,
                                                              void *pSegmentPurgedHandlerData,
                                                              HTTPHeapManager *pHeapManager,
                                                              int numSwapSegments)
  {
      HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
      result = HttpDataStoreBase::SetSessionStorageOption(m_pHttpDataStore,
                                                          eDataStorage,
                                                          nContentLength,
                                                          buffParams,
                                                          pHTTPFileSourceHelper,
                                                          pFilepath,
                                                          piStreamPort,
                                                          segmentPurgedHandler,
                                                          pSegmentPurgedHandlerData,
                                                          pHeapManager,
                                                          numSwapSegments);
      if (nContentLength > 0)
      {
        m_nContentLength = nContentLength;
      }

      if (NULL != pFilepath)
      {
        size_t dstSize = std_strlen(pFilepath) + 1;
        if(m_pFilepath)
        {
          QTV_Free(m_pFilepath);
          m_pFilepath = NULL;
        }
        m_pFilepath = (char*)QTV_Malloc(dstSize);
        if (NULL == m_pFilepath)
        {
          result = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
        }
        else
        {
          std_strlcpy(m_pFilepath, pFilepath, dstSize);
        }
      }

      return result;
  }

  /**
   * Temporary function to set the number of swap segments until
   *  support to be able to calculate free space across multiple
   *  data resources is added.
   *
   * @param numSwapSegments  max swap segments
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus HTTPDataManager::SetMaxSwapSegments(int numSwapSegments)
  {
    QTV_NULL_PTR_CHECK(m_pHttpDataStore, HTTPCommon::HTTPDL_ERROR_ABORT);
    return m_pHttpDataStore->SetMaxSwapSegments(numSwapSegments);
  }

  HTTPDownloadStatus HTTPDataManager::SetHTTPReadableDataInfo(iHTTPReadable*
                                                              pHTTPReadableDataInfo)
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != pHTTPReadableDataInfo)
    {
        m_pHttpDataStore->SetHTTPReadableDataInfo(pHTTPReadableDataInfo);
        result = HTTPCommon::HTTPDL_SUCCESS;
    }
    return result;
  }

  /**
   * @brief
   *  Create a segment with the preferred storage option.
   *
   * @param nKey
   * @param eDataStorage
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus HTTPDataManager::CreateSegment(
    uint64 nKey, iHTTPAPI::SegmentStorageType eDataStorage,
    int64 nStartOffset, bool bPurge, HttpSegmentDataStoreBase *pInitDataUnit)
  {
    return (m_pHttpDataStore
            ? m_pHttpDataStore->CreateSegment(nKey, eDataStorage, nStartOffset, bPurge, pInitDataUnit)
                  : HTTPCommon::HTTPDL_ERROR_ABORT);
  }

  /**
   * @brief
   *  Mark segment as fully downloaded
   *
   * @param nKey
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus HTTPDataManager::SetSegmentComplete(uint64 nKey, int64 nEndOffset)
  {
    return (m_pHttpDataStore
            ? m_pHttpDataStore->SetSegmentComplete(nKey, nEndOffset)
            : HTTPCommon::HTTPDL_ERROR_ABORT);
  }

  /**
   * @brief
   *  Abort download on segment
   *
   * @param nKey
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus HTTPDataManager::AbortSegment(uint64 nKey)
  {
    return (m_pHttpDataStore
            ? m_pHttpDataStore->AbortSegment(nKey)
            : HTTPCommon::HTTPDL_ERROR_ABORT);
  }

  /**
   * @brief
   *  Get the number of segments fully downloaded from byteOffset.
   * @param rHTTPSegmentsInfo  In and Out param. Segments information.
   * @param offset    In param. Byte offset from which the segment info is returned.
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus HTTPDataManager::GetAvailableSegments(
    HTTPSegmentsInfo & rHTTPSegmentsInfo, int64 offset)
  {
    return (m_pHttpDataStore
            ? m_pHttpDataStore->GetAvailableSegments(rHTTPSegmentsInfo, offset)
            : HTTPCommon::HTTPDL_ERROR_ABORT);
  }

  /**
   * @brief SetWriteOffset
   *
   * @param nOffset
   *
   * @return HTTPDownloadStatus
   */

  HTTPDownloadStatus HTTPDataManager::SetWriteOffset(int64 nOffset)
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;

    if (NULL != m_pHttpDataStore)
    {
      result = m_pHttpDataStore->SetWriteOffset(nOffset);
    }

    return result;
  }


  HTTPDownloadStatus HTTPDataManager::SetEndofFile()
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != m_pHttpDataStore)
    {
        result = m_pHttpDataStore->SetEndofFile();
    }

    return result;
  }


  HTTPDownloadStatus HTTPDataManager::SaveDownloadedContent(const char* pFilePath)
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != m_pHttpDataStore)
    {
        result = m_pHttpDataStore->SaveDownloadedContent(pFilePath,
                                                         (byte*)m_pFilepath);
    }
    // if save was successfull, then update the file path m_pFilepath with pFilePath.
    if (HTTPCommon::HTTPDL_SUCCESS == result)
    {
      if(NULL != m_pFilepath)
      {
        QTV_Free(m_pFilepath);
        size_t sourceSize = std_strlen(pFilePath) + 1;
        m_pFilepath = (char*)QTV_Malloc(sourceSize);
        if (NULL == m_pFilepath)
        {
          result = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
        }
        else
        {
          std_strlcpy(m_pFilepath, pFilePath, sourceSize);
        }
      }
    }

    return result;
  }


  HTTPDownloadStatus HTTPDataManager::DeleteDownloadedContent()
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != m_pHttpDataStore)
    {
        result = m_pHttpDataStore->DeleteDownloadedContent((byte*)m_pFilepath);
    }

    Delete();

    return result;
  }

  void HTTPDataManager::Delete()
  {
    if (NULL != m_pHttpDataStore)
    {
      QTV_Delete(m_pHttpDataStore);
      m_pHttpDataStore = NULL;
    }

    if (m_pFilepath)
    {
      QTV_Free(m_pFilepath);
      m_pFilepath = NULL;
    }
  }

  /**
   * @brief GetBuffer
   *
   * @param pBuffer
   * @param nBufSize
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus HTTPDataManager::GetBuffer(byte* &pBuffer,
                                                int64& nBufSize,
                                                uint64 nKey)
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != m_pHttpDataStore)
    {
      result = m_pHttpDataStore->GetBuffer(pBuffer, nBufSize, nKey);
    }

    return result;
  }


  /**
   * @brief CommitBuffer
   *
   * @param pBuffer
   * @param nBytes
   *
   * @return HTTPDownloadStatus
   */
  HTTPDownloadStatus HTTPDataManager::CommitBuffer(byte* pBuffer,
                                                   int64 nBytes,
                                                   uint64 nKey)
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != m_pHttpDataStore)
    {
        result = m_pHttpDataStore->CommitBuffer(pBuffer, nBytes, nKey);
    }

    return result;
  }

  /**
   * @brief IsSeekable
   *
   * @param nSeekOffset
   *
   * @return HTTPDownloadStatus
   */

  HTTPDownloadStatus HTTPDataManager::IsSeekable(const int32 nSeekOffset)
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != m_pHttpDataStore)
    {
        result = m_pHttpDataStore->IsSeekable(nSeekOffset);
    }

    return result;
  }
    /**
   * @brief Flushes
   *
   * @return HTTPDownloadStatus
   */

  HTTPDownloadStatus HTTPDataManager::ResetOffset(uint64 nKey)
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != m_pHttpDataStore)
    {
        result = m_pHttpDataStore->ResetOffset(nKey);
    }

    return result;
  }
  /**
   * @brief Flush
   *
   * @return HTTPDownloadStatus
   */

  HTTPDownloadStatus HTTPDataManager::Flush(int32 offset)
  {
    HTTPDownloadStatus result = HTTPCommon::HTTPDL_ERROR_ABORT;
    if (NULL != m_pHttpDataStore)
    {
        result = m_pHttpDataStore->Flush(offset);
    }

    return result;
  }

  /**
   * @brief Seek
   *
   * @param nOffset
   * @param nWhence
   * @param pnOutOffset
   *
   * @return int
   */

  DataSourceReturnCode HTTPDataManager::Seek(int64 nOffset,
                                             int nWhence,
                                             int64* pnOutOffset)
  {
    DataSourceReturnCode result = DS_FAILURE;

    if ( NULL == m_pHttpDataStore)
    {
       QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
       "Either m_pHttpDataStore is NULL " );
    }
    else if (m_bDataStorageSwitchInProgress)
    {
      result = DS_WAIT;
    }
    else
    {
      result = m_pHttpDataStore->Seek(nOffset, nWhence, pnOutOffset);
    }

    if (result != DS_SUCCESS && result != DS_WAIT)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "HTTP Seek Failure" );
    }

    return result;
  }


  /**
   * @brief GetContentLength
   *
   * @param pContentLength
   *
   * @return int
   */

  DataSourceReturnCode HTTPDataManager::GetContentLength(int64* pContentLength)
  {
    DataSourceReturnCode result = DS_FAILURE;

    if (NULL == pContentLength || m_nContentLength <= 0)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "NULL pContentLength or m_nContentLength <= 0");
    }
    else
    {
      *pContentLength = m_nContentLength;
      result = DS_SUCCESS;
    }

    return result;
  }

  DataSourceReturnCode HTTPDataManager::GetNumBytesAvailable(int64* pNumBytesAvailable)
  {
    DataSourceReturnCode result = DS_FAILURE;

    if ((NULL == m_pHttpDataStore) || (NULL == pNumBytesAvailable))
    {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "NULL: HTTPDataStore or pNumBytesAvailable" );
    }
    else
    {
      result = m_pHttpDataStore->GetNumBytesDownloaded(pNumBytesAvailable);
    }

    if (result != DS_SUCCESS)
    {
       QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HTTP GetNumBytesAvailable Failure" );
    }

    return result;
  }

  DataSourceReturnCode HTTPDataManager::GetAvailableOffset(int64* pAvailableOffset,
                                                           bool* pbEOS)
  {
    DataSourceReturnCode result = DS_FAILURE;

    if ((NULL == m_pHttpDataStore) || (NULL == pAvailableOffset) || (NULL == pbEOS))
    {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "NULL: HTTPDataStore or pAvailableOffset or pbEOS" );
    }
    else
    {
      result = m_pHttpDataStore->GetMaxDownloadOffset(pAvailableOffset, pbEOS);
    }

    if (result != DS_SUCCESS)
    {
       QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HTTP GetAvailableOffset Failure" );
    }

    return result;
  }
  DataSourceReturnCode HTTPDataManager::GetStartOffset(int64* pStartOffset)
  {
    DataSourceReturnCode result = DS_FAILURE;

    if ((NULL == m_pHttpDataStore) || (NULL == pStartOffset))
    {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "NULL: HTTPDataStore or pStartOffset" );
    }
    else
    {
      result = m_pHttpDataStore->GetStartOffset(pStartOffset);
    }

    if (result != DS_SUCCESS)
    {
       QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HTTP GetStartOffset Failure" );
    }

    return result;
  }

  DataSourceReturnCode HTTPDataManager::GetSourceType(DataSourceType* SourceType)
  {
    SourceType = SourceType;
    return DS_FAILURE;
  }

  /**
   * @brief Read
   *
   * @param pBuf
   * @param nBufSize
   * @param pnRead
   *
   * @return int
   */
  DataSourceReturnCode HTTPDataManager::Read(byte *pBuf, ssize_t nBufSize, ssize_t *pnRead)
  {
    DataSourceReturnCode result = DS_FAILURE;

    if (NULL == m_pHttpDataStore)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "NULL HTTPDataStore" );
    }
    else if (m_bDataStorageSwitchInProgress)
    {
      result = DS_WAIT;
    }
    else
    {
      int nBufSize32 = (int)nBufSize; int pnRead32;
      result = m_pHttpDataStore->Read(pBuf,nBufSize32,&pnRead32);
      nBufSize = (ssize_t)nBufSize32;
      *pnRead = (ssize_t)(pnRead32);
    }

    if ((DS_SUCCESS != result) && (DS_WAIT != result))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "HTTP Read Failure" );
    }

    return result;
  }

  /**
   * @brief Close
   *
   * @return int
   */

  DataSourceReturnCode HTTPDataManager::Close()
  {
    DataSourceReturnCode result = DS_FAILURE;

    if (NULL == m_pHttpDataStore)
    {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "NULL HTTPDataStore" );
    }
    else
    {
      result = m_pHttpDataStore->Close();
    }

    if (result != DS_SUCCESS)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "HTTP Close Failure" );
    }

    return result;
  }

  /**
   * @brief Readable
   *
   * @param iReadNotificationHandler
   *
   * @return int
   */

  DataSourceReturnCode HTTPDataManager::Readable(iReadNotificationHandler const* pNotificationHandler)
  {
    DataSourceReturnCode result = DS_FAILURE;

    if (NULL == pNotificationHandler)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "NULL pNotificationHandler" );
    }
    else
    {
      result = m_pHttpDataStore->Readable(pNotificationHandler);
    }

    if (result != DS_SUCCESS)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "HTTP Readable Failure" );
    }

    return result;
  }

  DataSourceReturnCode HTTPDataManager::Write(const byte *pBuf, ssize_t nBufSize, ssize_t *pnWritten)
  {
      pBuf = pBuf;
      nBufSize = nBufSize;
      pnWritten = pnWritten;
      return DS_FAILURE;
  }

} // namespace video
