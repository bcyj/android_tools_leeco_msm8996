#ifndef __SEGMENTDOWNLOADER_H__
#define __SEGMENTDOWNLOADER_H__
/************************************************************************* */
/**
 * SegmentDownloader.h
 * @brief Defines the SegmentDownloader interface.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/SegmentDownloader.h#24 $
$DateTime: 2013/08/14 18:14:12 $
$Change: 4277631 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "HTTPCommon.h"
#include "HTTPBandwidthEstimator.h"
#include "StreamSourceClock.h"

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

  #define UNDEFINED_VAL_REQUESTID 0xFFFFFFFF

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class iSegmentNotifier
{
public:
  virtual ~iSegmentNotifier() { };

  virtual int GetBufferedDurationFromNotifier() = 0;
};

class SegmentDownloader
{
public:
  SegmentDownloader();
  ~SegmentDownloader();

  bool Init(HTTPSessionInfo* pSessionInfo,
            HTTPStackInterface* pHTTPStack);

  bool Start(const char *pUrl,
             const char* pByteRangeUrl,
             const uint64 nKey = 0,
             const int64 nStartOffset = 0,
             const int64 nEndOffset = 0,
             const int64 nDurationMs = -1,
             const uint32 nHTTPRequestID = UNDEFINED_VAL_REQUESTID)
  {
    m_nKey = nKey;
    DownloaderBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->Open(pUrl, pByteRangeUrl, nStartOffset, nEndOffset, nDurationMs, nHTTPRequestID) :
            false);
  };

  bool IsInUse()
  {
    DownloaderBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->IsInUse() : false);
  };

  HTTPCommon::HTTPDownloadStatus Read(byte* pBuffer,
                                      const int64 nSizeToRead,
                                      int64& nSizeRead)
  {
    DownloaderBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ?
            pCurrentStateHandler->Read(pBuffer, nSizeToRead, nSizeRead) :
            HTTPCommon::HTTPDL_ERROR_ABORT);
  };

  bool Stop()
  {
    DownloaderBaseStateHandler* pCurrentStateHandler = GetStateHandler();
    return (pCurrentStateHandler ? pCurrentStateHandler->Close() : false);
  };

  uint64 GetNumBytesReceived()
  {
    uint64 nNumBytesReceived;
    MM_CriticalSection_Enter(m_pDataLock);
    nNumBytesReceived = m_nNumBytesReceived;
    MM_CriticalSection_Leave(m_pDataLock);
    return nNumBytesReceived;
  };

  int64 GetContentLength()
  {
    int64 nContentLength;
    MM_CriticalSection_Enter(m_pDataLock);
    nContentLength = m_nContentLength;
    MM_CriticalSection_Leave(m_pDataLock);
    return nContentLength;
  };

  uint64 GetStartOffset()
  {
    uint64 nStartOffset;
    MM_CriticalSection_Enter(m_pDataLock);
    nStartOffset = m_nStartOffset;
    MM_CriticalSection_Leave(m_pDataLock);
    return nStartOffset;
  };

  uint64 GetEndOffset()
  {
    uint64 nEndOffset;
    MM_CriticalSection_Enter(m_pDataLock);
    nEndOffset = m_nEndOffset;
    MM_CriticalSection_Leave(m_pDataLock);
    return nEndOffset;
  };

  int64 GetDuration()
  {
    int64 nDuration;
    MM_CriticalSection_Enter(m_pDataLock);
    nDuration = m_nDuration;
    MM_CriticalSection_Leave(m_pDataLock);
    return nDuration;
  };

  uint64 GetKey()
  {
    uint64 nKey;
    MM_CriticalSection_Enter(m_pDataLock);
    nKey = m_nKey;
    MM_CriticalSection_Leave(m_pDataLock);
    return nKey;
  };

  /**
   * Sets the iSegmentNotifier.
   */
  void SetSegNotifier(iSegmentNotifier *pSegNotifier);

  /**
   * Check if the DOWNLOAD_TOO_SLOW needs to be notified for this
   * data unit.
   */
  bool CheckDownloadTooSlow();

  /**
   * Disable or re-enable socket reads on this download unit.
   */
  void DisableSocketReads(bool bIsDisabled);

  /**
   * Increase the ref count of the shared timer
   */
  void StartSharedTimer();

  /**
   * Decrease the ref count of the shared timer.
   */
  void StopSharedTimer();

  /**
   * Decrease the refcnt of the shared timer by the amount it was
   * increased.
   */
  void ReleaseSharedTimerRefCnt();

  /*
   * Sets the shared bandwidth estimator
   */
  void SetSharedBwEstimator(HTTPBandwidthEstimator *pSharedBandwidthEstimator)
  {
    m_pSharedBandwidthEstimator = pSharedBandwidthEstimator;
  }

  char* GetByteRangeURL()
  {
    char *pByteRangeUrl;
    MM_CriticalSection_Enter(m_pDataLock);
    pByteRangeUrl = m_pByteRangeUrl;
    MM_CriticalSection_Leave(m_pDataLock);
    return pByteRangeUrl;
  }

  void SetByteRangeURL(const char* pByteRangeUrl)
  {
    MM_CriticalSection_Enter(m_pDataLock);
    if(m_pByteRangeUrl)
    {
      QTV_Free(m_pByteRangeUrl);
      m_pByteRangeUrl = NULL;
    }

    if(pByteRangeUrl)
    {
      size_t len = std_strlen(pByteRangeUrl);
      m_pByteRangeUrl = (char *)QTV_Malloc((len + 1) * sizeof(char));
      if(m_pByteRangeUrl)
      {
        std_strlcpy(m_pByteRangeUrl, pByteRangeUrl, len + 1);
      }
    }
    MM_CriticalSection_Leave(m_pDataLock);
  }

  bool IsByteRangeUrlRequested()
  {
    bool bSentByteRangeURL;
    MM_CriticalSection_Enter(m_pDataLock);
    bSentByteRangeURL = m_bSentByteRangeUrl;
    MM_CriticalSection_Leave(m_pDataLock);
    return bSentByteRangeURL;
  }

  void SetByteRangeUrlRequested(bool bSentByteRangeURL)
  {
    MM_CriticalSection_Enter(m_pDataLock);
    m_bSentByteRangeUrl = bSentByteRangeURL;
    MM_CriticalSection_Leave(m_pDataLock);
  }

  bool IsByteRangeUrlInUse()
  {
    bool bIsByteRangeUrlInUse;
    MM_CriticalSection_Enter(m_pDataLock);
    bIsByteRangeUrlInUse = m_bIsByteRangeUrlInUse;
    MM_CriticalSection_Leave(m_pDataLock);
    return bIsByteRangeUrlInUse;
  }

  void SetByteRangeUrlInUse(bool bIsByteRangeUrlInUse)
  {
    MM_CriticalSection_Enter(m_pDataLock);
    m_bIsByteRangeUrlInUse = bIsByteRangeUrlInUse;
    MM_CriticalSection_Leave(m_pDataLock);
  }

private:
  enum DownloaderState
  {
    SEG_DNLD_STATE_IDLE,
    SEG_DNLD_STATE_WAITFORDATA,
    SEG_DNLD_STATE_DATAREADY
  };

  //State handlers
  class DownloaderBaseStateHandler
  {
  public:
    DownloaderBaseStateHandler(DownloaderState eState,
                               SegmentDownloader* pDownloader)
      : m_eState(eState),
        m_pDownloader(pDownloader)
    {
    };

    virtual ~DownloaderBaseStateHandler()
    {
    };

    virtual bool Open(const char* /* pURL */, const char* /* pByteRangeUrl */, const int64 /* nStartOffset */, const int64 /* nEndOffset */, const int64 /* nDurationMs = -1 */, const uint32 /* nHTTPRequestID = UNDEFINED_VAL_REQUESTID */)
    {
      return true;
    };

    virtual bool IsInUse()
    {
      return true;
    };

    virtual HTTPCommon::HTTPDownloadStatus Read(byte* /* pBuffer */,
                                                const int64 /* nSizeToRead */,
                                                int64& /* nSizeRead */)
    {
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };

    virtual bool Close();

  protected:
    DownloaderState m_eState;
    SegmentDownloader* m_pDownloader;
  };


  class DownloaderIdleStateHandler : public DownloaderBaseStateHandler
  {
  public:
    DownloaderIdleStateHandler(SegmentDownloader* pDownloader)
      : DownloaderBaseStateHandler(SEG_DNLD_STATE_IDLE, pDownloader)
    {
    };

    virtual ~DownloaderIdleStateHandler()
    {
    };

    virtual bool Open(const char* pURL, const char* pByteRangeUrl, const int64 nStartOffset, const int64 nEndOffset, const int64 nDurationMs = -1, const uint32 nHTTPRequestID = UNDEFINED_VAL_REQUESTID);

    virtual bool IsInUse()
    {
      return false;
    };
  };

  class DownloaderWaitForDataStateHandler : public DownloaderBaseStateHandler
  {
  public:
    DownloaderWaitForDataStateHandler(SegmentDownloader* pDownloader)
      : DownloaderBaseStateHandler(SEG_DNLD_STATE_WAITFORDATA, pDownloader)
    {
    };

    virtual ~DownloaderWaitForDataStateHandler()
    {
    };

    virtual HTTPCommon::HTTPDownloadStatus Read(byte* pBuffer,
                                                const int64 nSizeToRead,
                                                int64& nSizeRead);
  };

  class DownloaderDataReadyStateHandler : public DownloaderBaseStateHandler
  {
  public:
    DownloaderDataReadyStateHandler(SegmentDownloader* pDownloader)
      : DownloaderBaseStateHandler(SEG_DNLD_STATE_DATAREADY, pDownloader)
    {
    };

    virtual ~DownloaderDataReadyStateHandler()
    {
    };

    virtual HTTPCommon::HTTPDownloadStatus Read(byte* pBuffer,
                                                const int64 nSizeToRead,
                                                int64& nSizeRead);
  };

  void SetStateHandler(DownloaderBaseStateHandler* pCurrStateHandler)
  {
    MM_CriticalSection_Enter(m_pDataLock);
    m_pCurrentStateHandler = pCurrStateHandler;
    MM_CriticalSection_Leave(m_pDataLock);
  }

  DownloaderBaseStateHandler* GetStateHandler()
  {
    DownloaderBaseStateHandler* pCurrStateHandler = NULL;
    MM_CriticalSection_Enter(m_pDataLock);
    pCurrStateHandler = m_pCurrentStateHandler;
    MM_CriticalSection_Leave(m_pDataLock);
    return pCurrStateHandler;
  };

  void UpdateNumBytesReceived(const uint64 nSizeRead)
  {
    MM_CriticalSection_Enter(m_pDataLock);
    m_nNumBytesReceived += nSizeRead;
    if(m_pSharedBandwidthEstimator)
    {
      m_pSharedBandwidthEstimator->UpdateSize(nSizeRead);
    }
    MM_CriticalSection_Leave(m_pDataLock);
  };

  void SetContentLength(const int64 nContentLength)
  {
    MM_CriticalSection_Enter(m_pDataLock);
    m_nContentLength = nContentLength;
    MM_CriticalSection_Leave(m_pDataLock);
  };

  void Reset()
  {
    MM_CriticalSection_Enter(m_pDataLock);
    m_nNumBytesReceived = 0;
    m_nContentLength = 0;
    m_nStartOffset = 0;
    m_nDuration = -1;
    m_nKey = -1;
    m_nHTTPRequestID = 0;
    m_pCurrentStateHandler = &m_cIdleStateHandler;

    m_IsDownloadDisabled = false;
    m_bIsDownloadTooSlowProcessed = false;
    m_BufferedDurationCheckCount = 0;

    if (m_pBandwidthEstimator)
    {
      QTV_Delete(m_pBandwidthEstimator);
      m_pBandwidthEstimator = QTV_New(HTTPBandwidthEstimator);
      if (m_pBandwidthEstimator)
      {
        m_pBandwidthEstimator->Initialize(m_pClock);
      }
    }

    ReleaseSharedTimerRefCnt();

    MM_CriticalSection_Leave(m_pDataLock);
  };

  /**
   * Decision function to decide if the download for this unit is
   * too slow.
   */
  bool IsTimeOut();

  HTTPStackInterface* m_pHTTPStack;
  bool m_bHTTPStackLocallyAllocated;
  HTTPSessionInfo* m_pSessionInfo;

  //Lock for data protection
  MM_HANDLE m_pDataLock;

  //Book-keeping
  uint64 m_nNumBytesReceived;
  int64 m_nContentLength;
  uint64 m_nStartOffset;
  uint64 m_nEndOffset;
  int64 m_nDuration;
  uint64 m_nKey;
  uint32 m_nHTTPRequestID;

  //Downloader state handlers
  DownloaderIdleStateHandler m_cIdleStateHandler;
  DownloaderWaitForDataStateHandler m_cWaitForDataStateHandler;
  DownloaderDataReadyStateHandler m_cDataReadyStateHandler;
  DownloaderBaseStateHandler* m_pCurrentStateHandler;

  /**
   * Ensures that only one too-slow notification is sent of a data unit.
   */
  bool m_bIsDownloadTooSlowProcessed;

  /**
   * Skip reading from the soocket if this flag is set.
   */
  bool m_IsDownloadDisabled;

  /**
   * This is used to help decreae the number of times
   * GetBufferedDurationFromNotifier is called as the call is a
   * little expensive.
   */
  uint32 m_BufferedDurationCheckCount;

  /**
   * Handle to get the buffered duration associated with the data
   * unit from playgroup.
   */
  iSegmentNotifier *m_pSegmentNotifier;

  /**
   * For bandwidth estimator object.
   */
  StreamSourceClock *m_pClock;

  /**
   * Used to calculate elasped time of download.
   */
  HTTPBandwidthAggregate *m_pBandwidthEstimator;

  /**
   * true if the ref count on the shared timer has been increased
   */
  bool m_bIsSharedTimerRefCountIncreased;

  /**
   * Pointer to the bandwidth estimator object that is shared
   * across clients of the same http stack.
   */
  HTTPBandwidthEstimator *m_pSharedBandwidthEstimator;

  char* m_pByteRangeUrl;

  bool m_bSentByteRangeUrl;

  bool m_bIsByteRangeUrlInUse;
};

} // namespace video

#endif //__SEGMENTDOWNLOADER_H__
