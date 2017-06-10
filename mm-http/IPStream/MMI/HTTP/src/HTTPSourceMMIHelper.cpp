/************************************************************************* */
/**
 * HTTPSourceMMIHelper.cpp
 * @brief Implementation of HTTPSourceMMIHelper.
 *  HTTPSourceMMIHelper is a helper class to HTTPSourceMMI, that mainly
 *  handles HTTP event notifications. It also implements iHTTPPlaybackHandler
 *  iface for HTTP streamer library to interact with HTTPSourceMMI during
 *  run-time.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIHelper.cpp#36 $
$DateTime: 2013/10/04 14:49:35 $
$Change: 4547816 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIHelper.cpp
** ======================================================================= */
#include "HTTPSourceMMIHelper.h"
#include "HTTPSourceMMI.h"
#include "HTTPSourceMMITrackHandler.h"
#include "HTTPSourceMMIStreamPortHandler.h"
#include "mmiDeviceApi.h"
#include "HTTPController.h"

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
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
**                        Class & Function Definitions
** ======================================================================= */
/** @brief Process HTTP event notification from HTTP controller.
  *
  * @param[in] HTTPCommand - HTTP controller command
  * @param[in] HTTPStatus - Command execution status
  * @param[in] pCbData - Reference to the registered MMI callback iface
  */
void HTTPSourceMMIHelper::NotifyHTTPEvent
(
 const HTTPControllerCommand HTTPCommand,
 const HTTPDownloadStatus HTTPStatus,
 void* pCbData
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPSourceMMIHelper::NotifyHTTPEvent" );

  //Convert HTTP event/status to MMI event/status
  switch (HTTPCommand)
  {
  case HTTPCommon::OPEN:
    ProcessOpenStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::CLOSE:
    ProcessCloseStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::START:
    ProcessStartStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::STOP:
    ProcessGenericCmdStatus(HTTPStatus, pCbData);

    break;

  case HTTPCommon::PLAY:
    ProcessPlayStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::PAUSE:
    ProcessPauseStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::DOWNLOAD:
    ProcessDownloadStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::SEEK:
    ProcessSeekStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::GET_TRACKS:
    ProcessGetTracksStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::SET_TRACK_STATE:
    ProcessSetTrackStateStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::WAIT_FOR_RESOURCES:
    ProcessWaitForResourcesStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::NOTIFY_CURRENT_WATERMARK_STATUS:
    ProcessNotifyWatermarkEventStatus(HTTPStatus, pCbData);
    break;

  case HTTPCommon::SELECT_REPRESENTATIONS:
    ProcessSelectRepresentationsStatus(HTTPStatus, pCbData);
    break;

  default:
    //Ignore other events
    break;
  }
}


/** @brief Process HTTP event notification from HTTPDataRequestHandler.
  *
  * @param[in] HTTPCommand - HTTP data command
  * @param[in] HTTPStatus - Command execution status
  * @param[in] pCbData - Reference to the registered MMI callback iface
  */
void HTTPSourceMMIHelper::NotifyHTTPEvent
(
 const HTTPDataCommand HTTPCommand,
 const HTTPDownloadStatus HTTPStatus,
 void* pCbData
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPSourceMMIHelper::NotifyHTTPEvent" );

  //Convert HTTP event/status to MMI event/status
  switch (HTTPCommand)
  {
    case HTTPCommon::FLUSH:
      ProcessFlushStatus(HTTPStatus, pCbData);
      break;


    default:
      //Ignore other events
      break;
  }
}

/** @brief Convert the HTTP status code to MMI status code for
  * Set track state command.
  *
  * @param[in] HTTPStatus - HTTP streamer status code
  * @param[in] pUserData - Reference to the registered MMI callback iface
  */
int HTTPSourceMMIHelper::NotifyDataEvent
(
 const HTTPDataCommand HTTPCommand,
 int32 portIdx,
 void *pBuffHdrParam
 )
{
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPSourceMMIHelper::NotifyDataEvent" );
  int ret = -1;

  //Convert HTTP event/status to MMI event/status
  switch (HTTPCommand)
  {
  case HTTPCommon::DATA_REQUEST:
    ret = ProcessDataRequestStatus(portIdx, pBuffHdrParam);
    break;

  case HTTPCommon::FLUSH:
    ret = ProcessFlushDataStatus(portIdx, pBuffHdrParam);
    break;

  default:
    break;
  }

  return ret;
}

/** @brief Check (over all tracks) if buffering.
*
* @return
* TRUE - Buffering
* FALSE - Otherwise (means Playing)
*/
bool HTTPSourceMMIHelper::IsBuffering()
{
  return ((m_pHTTPSourceMMI && m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
    ? m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->IsBuffering()
    : false);
}

/** @brief Notification from HTTP stack (e.g. redirect notification).
*
* @param[in] HTTPStackStatus - HTTP stack notification
* @return
*/
bool HTTPSourceMMIHelper::Notify
(
 HTTPStackNotifyCode HTTPStackStatus,
 void *pCbData
 )
{
  bool ret = false;

  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "HTTPSourceMMIHelper::Notify - status %d", HTTPStackStatus );

  HTTPStackNotificationCbData *protocolData = (HTTPStackNotificationCbData*)pCbData;

  if (protocolData->m_protocolHeaders)
  {
    if (m_pHTTPSourceMMI &&
    m_pHTTPSourceMMI->m_pHTTPSourceMMIExtensionHandler.SetHTTPProtocolHeadersEvent(pCbData) == MMI_S_COMPLETE)
    {
      ret = true;
    }
  }
  if (m_pHTTPSourceMMI &&
    m_pHTTPSourceMMI->m_pHTTPSourceMMIExtensionHandler.SetHTTPProtocolEvent(pCbData) == MMI_S_COMPLETE)
  {
    ret = true;
  }

  return ret;
}

/** @brief Reset the current HTTP session.
*
*/
void HTTPSourceMMIHelper::ResetSession()
{
  if (m_pHTTPSourceMMI)
  {
    m_pHTTPSourceMMI->SetOpenComplete(false);
    m_pHTTPSourceMMI->SetClosePending(false);
    m_pHTTPSourceMMI->SetSeekPending(false);
    if (m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
    {
      m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->Close();
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler is NULL" );
    }
    m_pHTTPSourceMMI->SetStartTimeBufferFlag();
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: m_pHTTPSourceMMI is NULL" );
  }
}

/** @brief Starts downloading the content (Download request is posted to
* HTTPController and result of Download operation is notified later).
*
* @param[in] pUserData - Reference to the registered MMI callback iface
*
* @return
* TRUE - Download successfully posted to HTTP controller
* FALSE - Otherwise
*/
bool HTTPSourceMMIHelper::Download
(
 void* pUserData
 )
{
  bool bOk = false;

  if (m_pHTTPSourceMMI && m_pHTTPSourceMMI->IsHTTPStreamerInitialized() &&
    !m_pHTTPSourceMMI->IsClosePending())
  {
    HTTPController* pHTTPController = m_pHTTPSourceMMI->m_pHTTPController;
    if (!pHTTPController->IsHTTPStreamerRunning())
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: HTTP streamer thread inactive - Download returns" );
    }
    else
    {
      //Queue the DOWNLOAD request on HTTPController
      if (pHTTPController->Download(pUserData))
      {
        //DOWNLOAD request queued, notification will be sent later
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "HTTP DOWNLOAD request queued" );
        bOk = true;
      }
      else
      {
        //Failed to queue DOWNLOAD request
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: Failed to queue HTTP DOWNLOAD request" );
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Either m_pHTTPSourceMMI is NULL or "
      "HTTP streamer uninitialized or "
      "earlier Close pending" );
  }

  return bOk;
}

/** @brief Convert the HTTP status code to MMI status code for Open.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessOpenStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
 )
{
  OMX_U32 ret = MMI_S_EFAIL;

  switch (HTTPStatus)
  {
  case HTTPCommon::HTTPDL_ERROR_ABORT:
    ret = MMI_S_EFAIL;
    break;
  case HTTPCommon::HTTPDL_TIMEOUT:
    ret = MMI_S_ETIMEOUT;
    break;
  case HTTPCommon::HTTPDL_OUT_OF_MEMORY:
    ret = MMI_S_ENOSWRES;
    break;
  case HTTPCommon::HTTPDL_UNSUPPORTED:
    ret = MMI_S_ENOTIMPL;
    break;
  case HTTPCommon::HTTPDL_INTERRUPTED:
    //Open failed
    break;
  case HTTPCommon::HTTPDL_SUCCESS:
    {
      ret = MMI_S_COMPLETE;
      // Get data interface from the controller
      HTTPDataInterface *pDataInterface = NULL;
      m_pHTTPSourceMMI->m_pHTTPController->GetDataInterface( pDataInterface );
      m_pHTTPSourceMMI->SetDataInterface( pDataInterface );
      m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->SetDataInterface(
        pDataInterface );
    }
    break;
  default:
    //Ignore other events
    break;
  }

  if (ret != MMI_S_PENDING)
  {
    //NotifyMmi(MMI_RESP_START, ret, 0, NULL, pUserData);
    NotifyMmi(MMI_RESP_LOAD_RESOURCES,
              ret,
              0,
              NULL,
              pUserData);
  }
}



/** @brief Convert the HTTP status code to MMI status code for start.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessStartStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
 )
{
  OMX_U32 ret = MMI_S_EFAIL;

  switch (HTTPStatus)
  {
  case HTTPCommon::HTTPDL_ERROR_ABORT:
    ret = MMI_S_EFAIL;
    break;
  case HTTPCommon::HTTPDL_TIMEOUT:
    ret = MMI_S_ETIMEOUT;
    break;
  case HTTPCommon::HTTPDL_OUT_OF_MEMORY:
    ret = MMI_S_ENOSWRES;
    break;
  case HTTPCommon::HTTPDL_UNSUPPORTED:
    ret = MMI_S_ENOTIMPL;
    break;
  case HTTPCommon::HTTPDL_INTERRUPTED:
    //Open failed
    break;
  case HTTPCommon::HTTPDL_SUCCESS:
    {
      //Start succeeded, post DOWNLOAD. Since DOWNLOAD
      //is an internal cmd, notify ONLY failure in
      //posting the cmd
      if (Download(pUserData))
      {
        ret = MMI_S_PENDING;
      }
   }
   break;
  default:
    //Ignore other events
    break;
  }

  if (ret != MMI_S_PENDING)
  {
    NotifyMmi(MMI_RESP_START, ret, 0, NULL, pUserData);
  }
}

/** @brief Convert the HTTP status code to MMI status code for Close.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessCloseStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
 )
{
  bool bOk = true;
  OMX_U32 ret = MMI_S_EBADPARAM;

  switch (HTTPStatus)
  {
  case HTTPCommon::HTTPDL_ERROR_ABORT:
    ret = MMI_S_EFAIL;
    break;
  case HTTPCommon::HTTPDL_TIMEOUT:
    ret = MMI_S_ETIMEOUT;
    break;
  case HTTPCommon::HTTPDL_OUT_OF_MEMORY:
    ret = MMI_S_ENOSWRES;
    break;
  case HTTPCommon::HTTPDL_UNSUPPORTED:
    ret = MMI_S_ENOTIMPL;
    break;
  case HTTPCommon::HTTPDL_SUCCESS:
    //Close successful
    ret = MMI_S_COMPLETE;
    break;
  default:
    //Ignore other events
    bOk = false;
    break;
  }

  m_pHTTPSourceMMI->m_pHTTPSourceMMIExtensionHandler.NotifyQOEEvent(QOMX_HTTP_IndexParamQOEStop);
  if (bOk)
  {
    ResetSession();
  }

  NotifyMmi(MMI_RESP_STOP, ret, 0, NULL, pUserData);
  return;
}

/** @brief Convert the HTTP status code to MMI status code for
* a generic command.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessGenericCmdStatus
(
 const HTTPDownloadStatus /*HTTPStatus*/,
 void* /*pUserData*/
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
    "generic MMI notification suppressed");
  return;
}

/** @brief Convert the HTTP status code to MMI status code for Download.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessDownloadStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
 )
{
  bool bSuppressNotification = false;
  bool result = false;
  unsigned int ret = MMI_S_EFAIL;
  bool bOpenComplete = false;
  OMX_U32 nEvtCode = MMI_RESP_START;
  void *pEventData = NULL;
  MMI_ResourceLostMsgType resourceLostMsg;
  uint32 nEventDataSize = 0;
  if (m_pHTTPSourceMMI == NULL)
  {
    return;
  }

  bOpenComplete = m_pHTTPSourceMMI->IsOpenComplete();

  switch (HTTPStatus)
  {
  case HTTPCommon::HTTPDL_ERROR_ABORT:
  case HTTPCommon::HTTPDL_TIMEOUT:
  case HTTPCommon::HTTPDL_OUT_OF_MEMORY:
  case HTTPCommon::HTTPDL_UNSUPPORTED:
    ret = MMI_S_EFAIL;
    break;
  case HTTPCommon::HTTPDL_INTERRUPTED:
  case HTTPCommon::HTTPDL_DATA_END:
    //Download interrupted - don't notify if Open is complete,
    //because being here means a user-ininitated stop
    if (bOpenComplete)
    {
      bSuppressNotification = true;
      break;
    }
    ret = MMI_S_EFAIL;
    break;
  case HTTPCommon::HTTPDL_SUCCESS:
    //Download succeeded
    bSuppressNotification = true;
    result = true;
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "Download complete" );
    break;
  case HTTPCommon::HTTPDL_TRACKS_AVALIABLE:
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "Tracks available" );

    if (!m_pHTTPSourceMMI->IsOpenComplete())
    {
      m_pHTTPSourceMMI->SetOpenComplete(true);
      bOpenComplete = true;
    }
    //Create the thread to process FTB calls
    m_pHTTPSourceMMI->StartDataRequestProcessing();

    ProcessGetTracksStatus(HTTPCommon::HTTPDL_SUCCESS, pUserData);

    bSuppressNotification = true;
    break;
  default:
    //Ignore other events
    bSuppressNotification = true;
    break;
  }

  if (bOpenComplete && !bSuppressNotification)
  {
    //if open complete, i.e. we'd sent start response
    //so send an resource lost event for any error now
    nEvtCode = MMI_EVT_RESOURCES_LOST;
    ret = MMI_S_COMPLETE;
    resourceLostMsg.bSuspendable = OMX_FALSE;
    pEventData = &resourceLostMsg;
    nEventDataSize = (uint32)sizeof(MMI_ResourceLostMsgType);
    m_pHTTPSourceMMI->UpdateStopPhrase((char*)STOP_ERROR_STRING);
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "HttpMmi download error, status %d",
      HTTPStatus);
  }

  if (!bSuppressNotification && !bOpenComplete)
  {
    //set open complete if we encounter any timeout,
    //any other error while downloading
    m_pHTTPSourceMMI->SetOpenComplete(true);
  }

  if (!bSuppressNotification)
  {
    NotifyMmi(nEvtCode, ret, nEventDataSize, pEventData, pUserData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
      "HTTP MMI notification suppressed" );
  }
}

/** @brief Convert the HTTP status code to MMI status code for Download.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessPauseStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
 )
{
  OMX_U32 ret = MMI_S_EFAIL;

  switch (HTTPStatus)
  {
  case HTTPCommon::HTTPDL_ERROR_ABORT:
    ret = MMI_S_EFAIL;
    break;
  case HTTPCommon::HTTPDL_TIMEOUT:
    ret = MMI_S_ETIMEOUT;
    break;
  case HTTPCommon::HTTPDL_OUT_OF_MEMORY:
    ret = MMI_S_ENOSWRES;
    break;
  case HTTPCommon::HTTPDL_UNSUPPORTED:
    ret = MMI_S_ENOTIMPL;
    break;
  case HTTPCommon::HTTPDL_SUCCESS:
    ret = MMI_S_COMPLETE;
    break;
  default:
    //send generic failure on any other events
    break;
  }

  NotifyMmi(MMI_RESP_PAUSE, ret, 0, NULL, pUserData);
  return;
}

/** @brief Convert the HTTP status code to MMI status code for Download.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessPlayStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
 )
{
  OMX_U32 ret = MMI_S_EFAIL;
  bool bNotify = true;
  if (m_pHTTPSourceMMI == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "MMI notification suppressed since m_pHTTPSourceMMI is NULL" );
    return;
  }

    switch (HTTPStatus)
    {
    case HTTPCommon::HTTPDL_ERROR_ABORT:
      ret = MMI_S_EFAIL;
      break;
    case HTTPCommon::HTTPDL_TIMEOUT:
      ret = MMI_S_ETIMEOUT;
      break;
    case HTTPCommon::HTTPDL_OUT_OF_MEMORY:
      ret = MMI_S_ENOSWRES;
      break;
    case HTTPCommon::HTTPDL_UNSUPPORTED:
      ret = MMI_S_ENOTIMPL;
      break;
    case HTTPCommon::HTTPDL_SUCCESS:
    case HTTPCommon::HTTPDL_DATA_END:
      ret = MMI_S_COMPLETE;
      break;
    default:
      //send generic failure on any other events
      break;
    }

  if ((ret != MMI_S_COMPLETE) && (ret != MMI_S_PENDING))
  {
    //Notify error on resume
    if (m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "MMI_EVT_RESOURCES_LOST to be sent, clearing track desc" );
      (void)m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->ResetPortInfo();
  }
    MMI_ResourceLostMsgType resourceLostMsg;
    resourceLostMsg.bSuspendable = OMX_FALSE;
    m_pHTTPSourceMMI->UpdateStopPhrase((char*)STOP_ERROR_STRING);
    NotifyMmi(MMI_EVT_RESOURCES_LOST, MMI_S_EFAIL,
      sizeof(MMI_ResourceLostMsgType), (void *)&resourceLostMsg, pUserData);
  }

  if(bNotify)
  {
    NotifyMmi(MMI_RESP_RESUME, ret, 0, NULL, pUserData);
  }
  return;
}

/** @brief Convert the HTTP status code to MMI status code for Seek.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessSeekStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
 )
{
  bool bNotify = true;

  if (m_pHTTPSourceMMI == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "MMI notification suppressed since m_pHTTPSourceMMI is NULL" );
    return;
  }

  (void)MM_CriticalSection_Enter(m_pHTTPSourceMMI->m_pHTTPSourceMMIDataLock);
  if (HTTPSUCCEEDED(HTTPStatus) || HTTPStatus == HTTPCommon::HTTPDL_DATA_END)
  {
    if(m_pHTTPSourceMMI->m_pHTTPDataInterface)
    {

      OMX_U32 ret = MMI_S_COMPLETE;
      //Seek is complete, adjust OMX buffer flags, notify track handler and check if
      //there is another pending seek
      int64 currSeekTime = m_pHTTPSourceMMI->m_nCurrentSeekTime;
      int64 pendingSeekTime = m_pHTTPSourceMMI->m_nPendingSeekTime;
      m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->SetAbsoluteSeekOffset(currSeekTime);
      m_pHTTPSourceMMI->SetStartTimeBufferFlag();
      if(HTTPStatus == HTTPCommon::HTTPDL_DATA_END)
      {
      //Return seek success. Later when FTBs come, if EOSbuffer flag is set, return EOS sample from MMI directly
      m_pHTTPSourceMMI->SetEOSBufferFlag();
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "recvd HTTPDL_DATA_END" );
      }
      if (pendingSeekTime != currSeekTime)
      {
        m_pHTTPSourceMMI->SetSeekPending(false);
        ret = m_pHTTPSourceMMI->Seek(pendingSeekTime, NULL);
      }
      else if(ret == MMI_S_COMPLETE)
      {
        bool isAudioPortConfigPending = false;
        bool isVideoPortConfigPending = false;
        bool isOtherPortConfigPending = false;
        if(HTTPStatus != HTTPCommon::HTTPDL_DATA_END)
        {
          //After seek is completed compare and update all the tracks and port information.
          //Delay port config change event for the track untill seekpending flag
          //becomes false, so that AAL would get only updated port information
          //TODO: Delaying of port config change event would be avoided if AAL
          //issue discontinuety to framwork upon getting port setting change event
          //and framwork start querying the scan source for the port

          if(m_pHTTPSourceMMI->CompareAndUpdatePorts(isAudioPortConfigPending,
                                                     isVideoPortConfigPending,
                                                     isOtherPortConfigPending)
                                            == HTTPCommon::HTTPDL_ERROR_ABORT)
          {
            ret = MMI_S_EFAIL;
          }
        }

        m_pHTTPSourceMMI->SetSeekPending(false);

        // Check and send port config event after seek pernding flag becomes false
        if((HTTPStatus != HTTPCommon::HTTPDL_DATA_END) && (MMI_S_EFAIL != ret))
        {
          m_pHTTPSourceMMI->CheckAndSendPortConfigChangeEvent(HTTPCommon::HTTP_AUDIO_TYPE, 0, isAudioPortConfigPending);
          m_pHTTPSourceMMI->CheckAndSendPortConfigChangeEvent(HTTPCommon::HTTP_VIDEO_TYPE, 0, isVideoPortConfigPending);
          m_pHTTPSourceMMI->CheckAndSendPortConfigChangeEvent(HTTPCommon::HTTP_TEXT_TYPE, 0, isOtherPortConfigPending);
        }

        //Resume data command processing
        if(m_pHTTPSourceMMI->m_pHTTPDataReqHandler)
        {
          m_pHTTPSourceMMI->m_pHTTPDataReqHandler->Resume();
        }

        const char *pCachedSelectRepresentations =
          m_pHTTPSourceMMI->GetCachedSelectedRepresentations();

        if (pCachedSelectRepresentations)
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "ProcessSeekStatus: Queue pending AdaptationSet change cmd");
          if(m_pHTTPSourceMMI->m_pHTTPController->SelectRepresentations(pCachedSelectRepresentations))
          {
            ret = MMI_S_COMPLETE;
            m_pHTTPSourceMMI->SetSelectRepresentationsPending(true);
            m_pHTTPSourceMMI->ClearCachedSelectRepresentions();
          }
        }
      }
      if(ret == MMI_S_COMPLETE)
      {
        bNotify = false;
      }
    }
  }

  //Only failure before posting a SEEK on File Source is captured here.
  //Seek status is notified by File Source if it has been posted
  if (bNotify)
  {
    if (m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "MMI_EVT_RESOURCES_LOST to be sent, clearing track desc" );
      (void)m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->ResetPortInfo();
    }
    m_pHTTPSourceMMI->SetSeekPending(false);
    MMI_ResourceLostMsgType resourceLostMsg;
    resourceLostMsg.bSuspendable = OMX_FALSE;
    m_pHTTPSourceMMI->UpdateStopPhrase((char*)STOP_ERROR_STRING);
    NotifyMmi(MMI_EVT_RESOURCES_LOST, MMI_S_EFAIL,
      sizeof(MMI_ResourceLostMsgType), (void *)&resourceLostMsg, pUserData);
  }
  (void)MM_CriticalSection_Leave(m_pHTTPSourceMMI->m_pHTTPSourceMMIDataLock);
}

void HTTPSourceMMIHelper::ProcessSelectRepresentationsStatus(const HTTPDownloadStatus /*HTTPStatus*/,
                                          void*  /*pUserData*/)
{
  // check to see if there is a pending seek or select reps that needs to happen.
  const char *pSelectString = m_pHTTPSourceMMI->GetCachedSelectedRepresentations();

  if (pSelectString)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Adaptationset change: ProcessSelectRepresentationsStatus process b2b");

    if (m_pHTTPSourceMMI->m_pHTTPController->SelectRepresentations(pSelectString) == false)
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "SelectRepresentations failed");
    }

    m_pHTTPSourceMMI->ClearCachedSelectRepresentions();
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Adaptationset change: SetSelectRepresentationsPending(false)");
    m_pHTTPSourceMMI->SetSelectRepresentationsPending(false);
  }
}

/** @brief Convert the HTTP status code to MMI status code for
* Get tracks command.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessGetTracksStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
)
{
  OMX_U32 ret = MMI_S_EFAIL;

  if (HTTPSUCCEEDED(HTTPStatus))
  {
    if (m_pHTTPSourceMMI && m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
    {
      m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->ProcessGetTracksStatus();
      ret = MMI_S_COMPLETE;
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "NULL objects HTTPSourceMMI %p or track handler",
                     (void *)m_pHTTPSourceMMI );
    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "ProcessGetTracksStatus failed with error %d", HTTPStatus );
    ret = MMI_S_EFAIL;
  }

  NotifyMmi(MMI_RESP_START, ret, 0, NULL, pUserData);

  if (m_pHTTPSourceMMI && ret == MMI_S_COMPLETE )
  {
    m_pHTTPSourceMMI->UpdatePorts();
  }
  ProcessQOENotification(QOMX_HTTP_IndexParamQOEPlay);
  return;
}

/** @brief .
*     Process QOE notification to be sent for eventID passed
*
* @param[in] eventID - id of event for which we need to notify QOE event
*                      after updating data structure related to that event
*/
void HTTPSourceMMIHelper::ProcessQOENotification(const uint32 eventID)
{
  m_pHTTPSourceMMI->m_pHTTPSourceMMIExtensionHandler.UpdateQOEData(eventID);
  /*Incase of stop, notification is sent while sending response*/
  if(eventID != QOMX_HTTP_IndexParamQOEStop)
  {
    bool status = m_pHTTPSourceMMI->m_pHTTPSourceMMIExtensionHandler.NotifyQOEEvent(eventID);
  }
}
/** @brief Convert the HTTP status code to MMI status code for
* Set track state command.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessSetTrackStateStatus
(
 const HTTPDownloadStatus HTTPStatus,
 void* pUserData
 )
{
  if (m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
  {
    m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->ProcessSetTrackStateStatus(
      HTTPStatus, pUserData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "MMI notification suppressed since m_pHTTPSourceMMITrackHelper is NULL" );
  }
}

/** @brief Convert the HTTP status code to MMI status code for
* Set track state command.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessWaitForResourcesStatus
(
 const HTTPDownloadStatus /*HTTPStatus*/,
 void* pUserData
 )
{
  NotifyMmi(MMI_RESP_WAIT_FOR_RESOURCES, MMI_S_COMPLETE, 0, NULL, pUserData);
  return;
}

/** @brief Convert the HTTP status code to MMI status code for
* Set track state command.
*
* @param[in] HTTPStatus - HTTP streamer status code
* @param[in] pUserData - Reference to the registered MMI callback iface
*/
void HTTPSourceMMIHelper::ProcessFlushStatus
(
 const HTTPDownloadStatus /*HTTPStatus*/,
 void* pUserData
 )
{
  MMI_PortCmdType FlushCmdResp;

  if (pUserData)
  {
    FlushCmdResp.nPortIndex =  *((OMX_U32 *)pUserData);
    NotifyMmi(MMI_RESP_FLUSH, MMI_S_COMPLETE, sizeof(FlushCmdResp),
      &FlushCmdResp, m_pHTTPSourceMMI->m_pClientData);
  }

  return;
}

/**
 * @brief
 *  Invoke HTTPSourceMMIExtensionHandler func to check and send
 *  watermark event if needed.
 *
 * @param HTTPStatus
 * @param pUserData
 */
void HTTPSourceMMIHelper::ProcessNotifyWatermarkEventStatus(
  const HTTPDownloadStatus /*HTTPStatus*/,
  void* pUserData)
{
  if (pUserData && m_pHTTPSourceMMI)
  {
    uint32 *portIdxAndWatermarkType = (uint32 *)pUserData;
    m_pHTTPSourceMMI->m_pHTTPSourceMMIExtensionHandler.NotifyWatermarkEvent(*portIdxAndWatermarkType);
  }
}

int HTTPSourceMMIHelper::ProcessDataRequestStatus
(
 int32 portIdx,
 void *pBuffHdrParam
 )
{
  int ret = -1;
  MMI_BufferCmdType FillBuffCmdResp;
  OMX_BUFFERHEADERTYPE *pBuffHdr = (OMX_BUFFERHEADERTYPE *) pBuffHdrParam;

  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMI, ret);
  QTV_NULL_PTR_CHECK(pBuffHdr, ret);

  OMX_U32 retVal = m_pHTTPSourceMMI->GetSample(portIdx, pBuffHdr);
  if (MMI_S_PENDING == retVal)
  {
    return ret;
  }

  //notify fillbufferdone
  FillBuffCmdResp.nPortIndex = pBuffHdr->nOutputPortIndex;
  FillBuffCmdResp.pBufferHdr = pBuffHdr;

  QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
    "HTTPMMIFillThisBufferDone for port %u pBufferHdr 0x%p pBuffer 0x%p",
    (uint32)FillBuffCmdResp.nPortIndex,
    (void *)pBuffHdr,
    (void *)pBuffHdr->pBuffer );
  NotifyMmi(MMI_RESP_FILL_THIS_BUFFER,
    retVal, sizeof(FillBuffCmdResp),
    &FillBuffCmdResp, m_pHTTPSourceMMI->m_pClientData);
  return 0;
}

int HTTPSourceMMIHelper::ProcessFlushDataStatus
(
 int32 /*portIdx*/,
 void *pBuffHdrParam
 )
{
  int ret = -1;
  MMI_BufferCmdType FillBuffCmdResp;
  OMX_BUFFERHEADERTYPE *pBuffHdr = (OMX_BUFFERHEADERTYPE *) pBuffHdrParam;

  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMI, ret);
  QTV_NULL_PTR_CHECK(pBuffHdr, ret);

  pBuffHdr->nFilledLen = 0;
  pBuffHdr->nOffset = 0;
  //notify fillbufferdone
  FillBuffCmdResp.nPortIndex = pBuffHdr->nOutputPortIndex;
  FillBuffCmdResp.pBufferHdr = pBuffHdr;


  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
    "FlushBuffer - portIdx:%u",
    (uint32)FillBuffCmdResp.nPortIndex);

  NotifyMmi(MMI_RESP_FILL_THIS_BUFFER,
    MMI_S_COMPLETE, sizeof(FillBuffCmdResp),
    &FillBuffCmdResp, m_pHTTPSourceMMI->m_pClientData);
  return 0;
}

/** @brief Invoke the registered generic callback for the MMI base interface.
*
* @param[in] eventID - The async event to be reported in the callback
*/
void HTTPSourceMMIHelper::NotifyMmi
(
 OMX_U32 nEvtCode,
 OMX_U32 nEvtStatus,
 size_t nPayloadLen,
 void *pEvtData,
 void * /*pClientData*/
 )
{
  if (m_pHTTPSourceMMI && m_pHTTPSourceMMI->m_HTTPSourceMMIAsync_CB)
  {
    OMX_U32 eventID = 0;
    if(pEvtData)
    {
      MMI_ExtSpecificMsgType* mmiExtMsg;
      mmiExtMsg = (MMI_ExtSpecificMsgType *)pEvtData;
      eventID = mmiExtMsg->nData2;
    }
    if (m_pHTTPSourceMMI->IsShutDownInProgress() && !(eventID == QOMX_HTTP_IndexParamQOEStop))
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "HTTPSourceMMIHelper::NotifyMmi - Shutdown in progress,"
        " not sending any notifications to MMI");
    }
    else
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "HTTPSourceMMIHelper::NotifyMmi - "
        "Async notification [event code - %d :Event Status - %d] "
        "sent to MMI source filter",
        (uint32)(nEvtCode-MMI_MSG_BASE), (uint32)(nEvtStatus-MMI_S_BASE));
      m_pHTTPSourceMMI->m_HTTPSourceMMIAsync_CB(nEvtCode, nEvtStatus,
        (OMX_U32)nPayloadLen, pEvtData,
        m_pHTTPSourceMMI->m_pClientData);
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: MMI async event suppressed - either "
      "m_pHTTPSourceMMI is NULL or CB not registered" );
  }
}

/**
* @brief Get Duration in the buffer for a track
*
* @param[in] portIdx
* @param[out] duration
*
* @return bool
*/
bool HTTPSourceMMIHelper::GetDurationBuffered
(
 int32 portIdx,
 uint64 &duration,
 uint64 &dwldPos
 )
{
  bool ret = false;
  HTTPSourceMMI::HttpSourceMmiPortInfo* pPortInfo = NULL;

  if(m_pHTTPSourceMMI &&
    m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
  {
    int32 trackIdx = -1;
    HTTPMediaType majorType = HTTPCommon::HTTP_UNKNOWN_TYPE;
    if(portIdx == MMI_HTTP_AUDIO_PORT_INDEX)
    {
      pPortInfo = &m_pHTTPSourceMMI->m_portAudio[portIdx - MMI_HTTP_AUDIO_PORT_INDEX];
      trackIdx =  pPortInfo->m_trackId;
      majorType = HTTPCommon::HTTP_AUDIO_TYPE;
    }
    else if(portIdx ==  MMI_HTTP_VIDEO_PORT_INDEX )
    {
      pPortInfo = &m_pHTTPSourceMMI->m_portVideo[portIdx - MMI_HTTP_VIDEO_PORT_INDEX];
      trackIdx = pPortInfo->m_trackId;
      majorType = HTTPCommon::HTTP_VIDEO_TYPE;
    }
    if(trackIdx >= 0)
    {
      ret = m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->GetBufferingProgress(
        trackIdx, majorType, duration, dwldPos);
    }
  }
  return ret;
}

/**
* @brief Get QOE data to populate data structure.
*
* @param[out] bandwidth
* @param[out] reBufCount
* @param[out] pVideoURL
* @param[out] nURLSize
* @param[out] pIpAddr
* @param[out] nIPAddrSize
* @param[out] pStopPhrase
* @param[out] nStopPhraseSize
*
*/
void HTTPSourceMMIHelper::GetQOEData(uint32& bandwidth,uint32& reBufCount,
                                     char* pVideoURL,size_t& nURLSize,
                                     char* pIpAddr,size_t& nIPAddrSize,
                                     char* pStopPhrase,size_t& nStopPhraseSize)
{
  if(m_pHTTPSourceMMI != NULL && m_pHTTPSourceMMI->m_pHTTPController != NULL)
  {
    m_pHTTPSourceMMI->m_pHTTPController->GetQOEData(bandwidth,pVideoURL,nURLSize,pIpAddr,nIPAddrSize);
    m_pHTTPSourceMMI->GetQOEData(reBufCount,pStopPhrase,nStopPhraseSize);
  }
}

void HTTPSourceMMIHelper::ProcessAuthHandlingDiscarded()
{
  if(m_pHTTPSourceMMI != NULL)
  {
    m_pHTTPSourceMMI->m_pHTTPController->SetAuthHandlingDiscarded();
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "m_pHTTPSourceMMI is  NULL ");
  }
  return;
}

}/* namespace video */
