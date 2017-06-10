#ifndef __HTTPSOURCEMMIHELPER_H__
#define __HTTPSOURCEMMIHELPER_H__
/************************************************************************* */
/**
 * HTTPSourceMMIHelper.h
 * @brief Header file for HTTPSourceMMIHelper.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIHelper.h#15 $
$DateTime: 2013/05/12 09:54:28 $
$Change: 3751774 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIHelper.h
** ======================================================================= */
#include "HTTPController.h"
#include "HTTPSourceMMI.h"
#include <HTTPStackInterface.h>
#include <HTTPSource.h>

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

  /* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPSourceAPI;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class HTTPSourceMMIHelper
  : public iHTTPNotificationHandler,
    public iHTTPDataNotificationHandler,
    public iHTTPPlaybackHandler,
    public HTTPStatusHandlerInterface
{
public:
  HTTPSourceMMIHelper(HTTPSourceMMI* pMMI)
      : m_bOpenCompleted(false),
        m_pHTTPSourceMMI(pMMI){ };
  virtual ~HTTPSourceMMIHelper(){ };

  friend class HTTPSourceMMI;

  //iHTTPNotificationHandler methods - Handling HTTP streamer notifications
  virtual void NotifyHTTPEvent(const HTTPControllerCommand HTTPCommand,
                               const HTTPDownloadStatus HTTPStatus,
                               void* pMMICbData);

  //iHTTPDataNotificationHandler methods - Handling HTTPDataRequestHandler notifications
  virtual void NotifyHTTPEvent(const HTTPDataCommand HTTPCommand,
                               const HTTPDownloadStatus HTTPStatus,
                               void* pMMICbData);

  virtual int NotifyDataEvent(const HTTPDataCommand HTTPCommand,
                                   int32 portIdx, void *pBuffHdrParam);

  virtual bool IsBuffering();

  //HTTPStatusHandlerInterface methods - Handling HTTP stack notifications
  virtual bool Notify(HTTPStackNotifyCode HTTPStackStatus,void *pCbData);

  //HTTPSourceMMIHelper methods
  void NotifyFileSourceEvent(FileSourceCallBackStatus status);

  // Notify MMI Events to IL client
  void NotifyMmi(OMX_U32 nEvtCode,
                 OMX_U32 nEvtStatus,
                 size_t nPayloadLen,
                 void *pEvtData,
                 void *pClientData);

  bool GetDurationBuffered(int32 portIdx, uint64 &duration, uint64 &curPos);

  void GetQOEData(uint32& bandwidth,uint32& reBufCount,
                  char* pVideoURL,size_t& nURLSize,
                  char* pIpAddr,size_t& nIPAddrSize,
                  char* pStopPhrase,size_t& nStopPhraseSize);

  void UpdateRebufCount(int nCount)
  {
    if(m_pHTTPSourceMMI)
    {
      m_pHTTPSourceMMI->UpdateRebufCount(nCount);
    }
  }

  void ProcessAuthHandlingDiscarded();

private:
  void ResetSession();
  bool Download(void* pUserData);
  void ProcessOpenStatus(const HTTPDownloadStatus HTTPStatus,
                         void* pUserData);
  void ProcessStartStatus(const HTTPDownloadStatus HTTPStatus,
                         void* pUserData);
  void ProcessCloseStatus(const HTTPDownloadStatus HTTPStatus,
                          void* pUserData);
  void ProcessGenericCmdStatus(const HTTPDownloadStatus HTTPStatus,
                               void* pUserData);
  void ProcessDownloadStatus(const HTTPDownloadStatus HTTPStatus,
                             void* pUserData);
  void ProcessPauseStatus(const HTTPDownloadStatus HTTPStatus,
                          void* pUserData);
  void ProcessPlayStatus(const HTTPDownloadStatus HTTPStatus,
                         void* pUserData);
  void ProcessSeekStatus(const HTTPDownloadStatus HTTPStatus,
                         void* pUserData);
  void ProcessSelectRepresentationsStatus(const HTTPDownloadStatus HTTPStatus,
                                          void* pUserData);
  void ProcessGetTracksStatus(const HTTPDownloadStatus HTTPStatus,
                              void* pUserData);
  void ProcessQOENotification(const uint32 eventID);
  void ProcessSetTrackStateStatus(const HTTPDownloadStatus HTTPStatus,
                                  void* pUserData);
  void ProcessWaitForResourcesStatus(const HTTPDownloadStatus HTTPStatus,
                                     void* pUserData);
  void ProcessFlushStatus(const HTTPDownloadStatus HTTPStatus,
                          void* pUserData);
  void ProcessNotifyWatermarkEventStatus(const HTTPDownloadStatus HTTPStatus,
                                         void* pUserData);
  int ProcessDataRequestStatus(int32 portIdx,
                                 void *pBuffHdrParam);
  int ProcessFlushDataStatus(int32 portIdx,
                               void *pBuffHdrParam);
private:
  bool m_bOpenCompleted;
  HTTPSourceMMI* m_pHTTPSourceMMI;
};

}/* namespace video */

#endif /* __HTTPSOURCEMMIHELPER_H__ */
