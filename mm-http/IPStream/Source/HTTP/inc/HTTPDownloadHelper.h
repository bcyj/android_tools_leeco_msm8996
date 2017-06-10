#ifndef __HTTPDOWNLOADHELPER_H__
#define __HTTPDOWNLOADHELPER_H__
/************************************************************************* */
/**
 * HTTPDownloadHelper.h
 * @brief Header file for HTTPDownloadHelper.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPDownloadHelper.h#19 $
$DateTime: 2013/06/28 16:10:11 $
$Change: 4009813 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDownloadHelper.h
** ======================================================================= */
#include "HTTPCommon.h"
#include "HTTPSessionInfo.h"
#include "HTTPDataManager.h"
#include "IPStreamProtocolHeaders.h"
#include <HTTPStackInterface.h>

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
class HTTPDataManager;
class HTTPDataInterface;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class HTTPDownloadHelper : public iHTTPReadable
{
public:
  explicit HTTPDownloadHelper(HTTPSessionInfo& sessionInfo,
                              HTTPStackInterface& pHTTPStack);
  virtual ~HTTPDownloadHelper();

  virtual HTTPDownloadStatus InitiateHTTPConnection() = 0;
  virtual HTTPDownloadStatus GetData() = 0;
  virtual HTTPDownloadStatus CloseHTTPConnection() = 0;

  virtual HTTPDownloadStatus Seek(const int64 /* seekTime */)
  {
    return HTTPCommon::HTTPDL_SUCCESS;
  }
  virtual bool GetTotalDuration(uint32 &duration)
  {
    duration = 0;
    return false;
  }
  virtual bool IsLiveStreamingSession()
  {
    return false;
  }
  virtual void SetSessionInterruptFlag()
  {
    return;
  }
  virtual HTTPDownloadStatus Pause()
  {
    return HTTPCommon::HTTPDL_SUCCESS;
  }
  virtual HTTPDownloadStatus Resume()
  {
    return HTTPCommon::HTTPDL_SUCCESS;
  }

  virtual bool IsEndOfFile()
  {
    bool bEOF = false;
    (void)MM_CriticalSection_Enter(m_pDownloadHelperDataLock);
    bEOF = m_bEndOfFile;
    (void)MM_CriticalSection_Leave(m_pDownloadHelperDataLock);
    return bEOF;
  }

//iHTTPReadable method(s)
  //This is used by DataManager to know the download progress, instead of having
  //to maintain its own statistics. Also for FT broken download, some of already
  //downloaded data might be re-downloaded (incomplete data chunks based on the
  //requested start PB time). This needs to be accounted for and DataManager does
  //not need to have knowledge of any of this.
  virtual uint64 GetTotalBytesReceived()
  {
    uint64 numBytesReceived = 0;
    (void)MM_CriticalSection_Enter(m_pDownloadHelperDataLock);
    numBytesReceived = m_nTotalBytesReceived;
    (void)MM_CriticalSection_Leave(m_pDownloadHelperDataLock);
    return numBytesReceived;
  }
  virtual bool IsReadable(int32 /* readOffset */, int32 /* bufSize */)
  {
    return true;
  }

  virtual bool GetDownloadProgress(HTTPMediaType mediaType,
                                   uint32& currStartOffset,
                                   uint32& downloadOffset,
                                   HTTPDownloadProgressUnitsType eUnitsType,
                                   bool& bEOS) = 0;

  virtual HTTPDataInterface *GetDataInterface()
  {
    return NULL;
  }

  virtual bool GetMediaProperties(char *pPropertiesStr,
                                  uint32 &nPropertiesLen) = 0;

  virtual bool SelectRepresentations(const char* SetSelectionsXML) = 0;

  virtual bool GetMPDText(char *pMPDTextStr, uint32 &mpdSize) = 0;

  virtual void GetQOEData(uint32 &bandwidth, char *pVideoURL, size_t& nURLSize, char *pIpAddr, size_t& nIPAddrSize) = 0;

  virtual bool IsAdaptationSetChangePending()
  {
    return false;
  }

  virtual void UpdateTrackSelections()
  {

  }

protected:
  class BaseStateHandler
  {
  public:
    BaseStateHandler(){ };
    virtual ~BaseStateHandler(){ };

    virtual HTTPDownloadStatus Execute(HTTPDownloadHelper* pDownloadHelper)
    {
      (void)pDownloadHelper;
      return HTTPCommon::HTTPDL_ERROR_ABORT;
    };
  };

  virtual void SetStateHandler(BaseStateHandler* pStatehandler)
  {
    m_pStateHandler = pStatehandler;
  };
  virtual BaseStateHandler* GetStateHandler()
  {
    return m_pStateHandler;
  };
  virtual HTTPDownloadStatus CurrentStateHandler(HTTPDownloadHelper* pDownloadHelper);

  virtual bool ParseURL(const char* pDefaultPort, char*& pLaunchURL);
  virtual HTTPDownloadStatus IsCloseComplete();
  virtual void SetTotalBytesReceived(const int32 totalBytesReceived)
  {
    (void)MM_CriticalSection_Enter(m_pDownloadHelperDataLock);
    m_nTotalBytesReceived = totalBytesReceived;
    (void)MM_CriticalSection_Leave(m_pDownloadHelperDataLock);
  }
  void SetEndOfFile(const bool bEOF)
  {
    (void)MM_CriticalSection_Enter(m_pDownloadHelperDataLock);
    m_bEndOfFile = bEOF;
    (void)MM_CriticalSection_Leave(m_pDownloadHelperDataLock);
  }

  HTTPSessionInfo& m_sessionInfo;
  HTTPStackInterface& m_HTTPStack;
  char* m_pLaunchURL;
  static const int m_nMaxDataChunkLen = HTTP_MAX_DATA_CHUNK_LEN;
  BaseStateHandler* m_pStateHandler;
  MM_HANDLE m_pDownloadHelperDataLock;

private:
  HTTPDownloadHelper();
  HTTPDownloadHelper(const HTTPDownloadHelper&);
  HTTPDownloadHelper& operator=(const HTTPDownloadHelper&);
  int32 m_nTotalBytesReceived;
  bool m_bEndOfFile;
};

}/* namespace video */
#endif /* __HTTPDOWNLOADHELPER_H__ */
