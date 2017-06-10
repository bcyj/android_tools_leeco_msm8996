/************************************************************************* */
/**
 * HTTPController.cpp
 * @brief Implementation of HTTPController.
 *  HTTPController class controls the HTTP session. It maintains a command
 *  queue and executes the command in the HTTP thread context. It relies
 *  on HTTPDownloader for the command execution and notifies HTTP source
 *  filter of the execution status upon completion.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPController.cpp#37 $
$DateTime: 2013/09/20 11:38:26 $
$Change: 4469780 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPController.cpp
** ======================================================================= */
#include "HTTPStackInterface.h"
#include "HTTPController.h"
#include "HTTPDownloader.h"

#include <SourceMemDebug.h>

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
/**
                       HTTP Controller Command State Matrix
                       (A, B) - A: Is state good to execute HTTP cmd
                                B: Notification sent or suppressed

                  | IDLE    CONNECTING    CONNECTED   DOWNLOADING   DOWNLOAD_DONE   CLOSING
 ------------------------------------------------------------------------------------------
  OPEN            |(Y, N)     (N, Y)        (N, Y)       (N, Y)        (N, Y)        (N, N)
                  |
  CLOSE           |(Y, N)     (Y, N)        (Y, N)       (Y, N)        (Y, N)        (N, N)
                  |
  START           |(N, Y)     (N, Y)        (Y, N)       (N, Y)        (N, Y)        (N, N)
                  |
  STOP            |(N, Y)     (N, Y)        (Y, Y)       (Y, Y)        (Y, Y)        (N, N)
                  |
  PLAY            |(N, Y)     (N, Y)        (Y, Y)       (Y, Y)        (Y, Y)        (N, N)
                  |
  PAUSE           |(N, Y)     (N, Y)        (Y, Y)       (Y, Y)        (Y, Y)        (N, N)
                  |
  DOWNLOAD        |(N, Y)     (N, Y)        (Y, N)       (N, N)        (N, N)        (N, N)
                  |
  SEEK            |(N, Y)     (N, Y)        (Y, Y)       (Y, Y)        (Y, Y)        (N, N)
                  |
  FETCH_POSITIONS |(N, Y)     (N, Y)        (N, Y)       (Y, Y)        (Y, Y)        (N, N)
                  |
  GET_TRACKS      |(N, Y)     (N, Y)        (N, Y)       (Y, Y)        (Y, Y)        (N, N)
                  |
  COMPARE_TRACKS  |(N, Y)     (N, Y)        (Y, Y)       (Y, Y)        (Y, Y)        (N, N)

  SET_TRACK_STATE |(N, Y)     (N, Y)        (N, Y)       (Y, Y)        (Y, Y)        (N, N)

  CHECK_DATA_AVAIL_FOR_SEEK  |(N, Y)     (N, Y)        (Y, Y)       (Y, Y)        (Y, Y)        (N, N)
 ------------------------------------------------------------------------------------------
 */
const bool HTTPController::CommandStateMatrix
[HTTPCommon::CMD_MAX][HTTPController::STATE_MAX][2] =
{
  {
    {1, 0}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 0}
  },
  {
    {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {1, 0}, {0, 1}, {0, 1}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {1, 1}, {1, 1}, {1, 1}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {1, 1}, {1, 1}, {1, 1}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {1, 1}, {1, 1}, {1, 1}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {1, 0}, {0, 0}, {0, 0}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {1, 1}, {1, 1}, {1, 1}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {0, 1}, {1, 1}, {1, 1}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {0, 1}, {1, 1}, {1, 1}, {0, 0}
  },
  {
    {0, 1}, {0, 1}, {1, 1}, {1, 1}, {1, 1}, {0, 0}
  },

  {
    {0, 1}, {0, 1}, {0, 1}, {1, 1}, {1, 1}, {0, 0}
  },

  {
    {0, 1}, {0, 1}, {1, 1}, {1, 1}, {1, 1}, {0, 0}
  },
};

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief HTTPController Constructor.
  *
  * @param[in] pHTTPNotification - HTTP stream notification
  * @param[in] pHTTPStackStatusHandler - HTTP status handler IQI (passed to HTTP stack)
  * @param[out] bOk - Status of initialization
  */
HTTPController::HTTPController
(
 iHTTPNotificationHandler* pHTTPNotification,
 void* pHTTPStackStatusHandler,
 bool& bOk
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPController::HTTPController" );
  bOk = false;

  //Initialize class member data (for each create)
  Reset();

  //Register the HTTP source notification handler (for event notification)
  m_pHTTPSourceNotificationHandler = pHTTPNotification;

  m_pStackNotificationHandler = reinterpret_cast<HTTPStatusHandlerInterface *>(pHTTPStackStatusHandler);

  //Create the HTTP streamer CS
  bOk = (MM_CriticalSection_Create(&m_pHTTPStreamerCS) == 0 && m_pHTTPStreamerCS);

  //Create the HTTP downloader
  if (bOk)
  {
    //Pass pStatusHandlerIQI to HTTPDownloader (and eventually to HTTP stack)
    m_pDownloader = QTV_New_Args(HTTPDownloader, (this, bOk));
  }

  StreamQ_init(&m_PendingAuthQueue);
}

/** @brief Initialize member data.
*
*/
void HTTPController::Reset()
{
  m_state = IDLE;
  m_bHTTPStreamerRunning = false;
  m_pDownloader = NULL;

  m_pHTTPStreamerThread = NULL;
  m_pHTTPStreamerCS = NULL;

  m_pHTTPSourceNotificationHandler = NULL;

  //Setting defaults
  m_dataStorageOption = iHTTPAPI::DEFAULT_STORAGE;
  m_nHeapStorageLimit = HTTP_HEAP_SIZE_UNKNOWN;

  m_nHttpRequestsLimit = HTTP_MAX_CONNECTIONS;
}

/** @brief HTTPController Destructor.
*
*/
HTTPController::~HTTPController()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPController::~HTTPController" );

  //Release the HTTPStreamerThread
  ReleaseThread();

  while (StreamQ_cnt(&m_PendingAuthQueue) > 0)
  {
    PendingAuthElem *elem = (PendingAuthElem *)StreamQ_get(&m_PendingAuthQueue);

    if (elem)
    {
      QTV_Delete(elem);
    }
  }

  //Release the Critical Section(s)
  if (m_pHTTPStreamerCS)
  {
    (void)MM_CriticalSection_Release(m_pHTTPStreamerCS);
    m_pHTTPStreamerCS = NULL;
  }

  if (m_pDownloader)
  {
    QTV_Delete(m_pDownloader);
    m_pDownloader = NULL;
  }
}

/**
 * @brief
 *  Stop the HTTPStreamer thread and wait for the thread to exit
 */
void HTTPController::ReleaseThread()
{
  if (m_pHTTPStreamerThread)
  {
    //Unconditional wait till HTTP streamer thread exits
    int nExitCode;
    (void)MM_Thread_Join(m_pHTTPStreamerThread, &nExitCode);
    (void)MM_Thread_Release(m_pHTTPStreamerThread);
    m_pHTTPStreamerThread = NULL;
  }
}

/**
 * @brief
 *  This traps the notification from httpstack to serialize auth
 *    notification to MMI and also caches the http-stack ptr on
 *    which to invoke setHeader for auth. <<< NOte that
 *    implementation needs to be such that any instance of
 *    httpstack should never be deleted otherwise the
 *    httpstack-ptr will become invalid (dangling) >>> This was
 *    done to add fewer changes to the code instead of caching a
 *    pointer that will always remain in scope.
 *
 * @param httpNotifyCode
 * @param pCbData
 *
 * @return bool
 */
bool HTTPController::Notify(HTTPStackNotifyCode httpNotifyCode,void *pCbData)
{
  bool rslt = false;
  HTTPStackNotificationCbData *cb = (HTTPStackNotificationCbData *)pCbData;

  if (m_pStackNotificationHandler && cb)
  {
    if (cb->m_protocolHeaders && m_pDownloader)
    {
      m_pDownloader->DisableTaskTimeout(true);

      // if protocolheader is not null, currently it means it contains
      // authorization (or proxy authorization) header and no other headers!!!
      PendingAuthElem *pAuthElem = QTV_New(PendingAuthElem);
      if (pAuthElem)
      {
        StreamQ_link(pAuthElem, &(pAuthElem->link));
        pAuthElem->m_HTTPNotifyCode = httpNotifyCode;

        HTTPStackNotificationCbDataForAuth& rHTTPStackNotificationData =
          pAuthElem->m_HTTPStackNotificationData;
        rHTTPStackNotificationData.m_serverCode = cb->m_serverCode;

        rHTTPStackNotificationData.m_serverMessage = NULL;
        size_t bufSize = (cb->m_serverMessage ? 1 + std_strlen(cb->m_serverMessage) : 0);
        if (bufSize > 0)
        {
          rHTTPStackNotificationData.m_serverMessage = (char *)QTV_Malloc(bufSize * sizeof(char));
          if (rHTTPStackNotificationData.m_serverMessage)
          {
            std_strlcpy(rHTTPStackNotificationData.m_serverMessage, cb->m_serverMessage, bufSize);
          }
        }

        rHTTPStackNotificationData.m_entityBody = NULL;
        bufSize = (cb->m_entityBody ? 1 + std_strlen(cb->m_entityBody) : 0);
        if (bufSize > 0)
        {
          rHTTPStackNotificationData.m_entityBody = (char *)QTV_Malloc(bufSize * sizeof(char));
          if (rHTTPStackNotificationData.m_entityBody)
          {
            std_strlcpy(rHTTPStackNotificationData.m_entityBody, cb->m_entityBody, bufSize);
          }
        }

        rHTTPStackNotificationData.m_msgType = cb->m_msgType;

        rHTTPStackNotificationData.m_method = NULL;
        bufSize = (cb->m_method ? 1 + std_strlen(cb->m_method) : 0);
        if (bufSize > 0)
        {
          rHTTPStackNotificationData.m_method = (char *)QTV_Malloc(bufSize * sizeof(char));
          if (rHTTPStackNotificationData.m_method)
          {
            std_strlcpy(rHTTPStackNotificationData.m_method, cb->m_method, bufSize);
          }
        }

        rHTTPStackNotificationData.m_protocolHeaders = NULL;
        bufSize = (cb->m_protocolHeaders ? 1 + std_strlen(cb->m_protocolHeaders) : 0);
        if (bufSize)
        {
          rHTTPStackNotificationData.m_protocolHeaders = (char *)QTV_Malloc(bufSize * sizeof(char));
          if (rHTTPStackNotificationData.m_protocolHeaders)
          {
            std_strlcpy(rHTTPStackNotificationData.m_protocolHeaders, cb->m_protocolHeaders, bufSize);
          }
        }

        rHTTPStackNotificationData.m_pHTTPStack = cb->m_pHTTPStack;

        (void)MM_CriticalSection_Enter(m_pHTTPStreamerCS);
        int pendingAuthQSize = StreamQ_cnt(&m_PendingAuthQueue);
        StreamQ_put(&m_PendingAuthQueue, &(pAuthElem->link));
        (void)MM_CriticalSection_Leave(m_pHTTPStreamerCS);

        if (pendingAuthQSize > 0)
        {
          QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
            "Auth (or proxy-auth) header received, but notification to MMI "
            "skipped as there are %d outstanding SetHeaders for auth",
            pendingAuthQSize);
        }
        else
        {
          NotifyProtocolEvent();
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HTTPDownloader::Notify Failed to allocate pAuthElem");
      }
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "HTTPDownloader::Notify Notify protocolEvent with code %d to MMI", cb->m_serverCode);
      m_pStackNotificationHandler->Notify(httpNotifyCode, pCbData);
    }

    rslt = true;
  }

  return rslt;
}

/** @brief Check if HTTP streamer thread is active.
*
* @return
* TRUE - If thread is around
* FALSE - Otherwise
*/
bool HTTPController::IsHTTPStreamerRunning()
{
  bool bHTTPStreamerRunning = false;

  (void)MM_CriticalSection_Enter(m_pHTTPStreamerCS);
  bHTTPStreamerRunning = m_bHTTPStreamerRunning;
  (void)MM_CriticalSection_Leave(m_pHTTPStreamerCS);

  return bHTTPStreamerRunning;
}

/** @brief This method sets the flag that indicates whether or not HTTP
* streamer thread is active.
*
* @param[in] bHTTPStreamerRunning - Flag to indicate if HTTP streamer thread
*                                   is running
*/
void HTTPController::SetHTTPStreamerRunning
(
 bool bHTTPStreamerRunning
 )
{
  (void)MM_CriticalSection_Enter(m_pHTTPStreamerCS);
  m_bHTTPStreamerRunning = bHTTPStreamerRunning;
  (void)MM_CriticalSection_Leave(m_pHTTPStreamerCS);
}

/** @brief Adds a new scheduler task - delegates to HTTPDownloader.
*
* @param[in] pTask - Task ptr for the new scheduler task
* @param[in] pTaskParam - Task parameter
* @return
* TRUE - Scheduler task successfully added
* FALSE - Error in adding scheduler task
*/
bool HTTPController::AddSchedulerTask
(
 SchedulerTask pTask,
 void* pTaskParam
 )
{
  bool bOk = false;

  if (m_pDownloader)
  {
    bOk = m_pDownloader->AddSchedulerTask(pTask, pTaskParam);
  }

  return bOk;
}

/** @brief Removes a scheduler task - delegates to HTTPDownloader.
*
* @param[in] taskID - Scheduler task ID for the task to be deleted
* @return
* TRUE - Scheduler task successfully deleted
* FALSE - Error in deleting scheduler task
*/
bool HTTPController::DeleteSchedulerTask
(
 const int taskID
 )
{
  bool bOk = false;

  if (m_pDownloader)
  {
    bOk = m_pDownloader->DeleteSchedulerTask(taskID);
  }

  return bOk;
}

/** @brief Queue up HTTPController Open command.
*
* @param[in] pURN - URN to use to connect to the server
* @param[in] pPlaybackHandler - Reference to HTTP source playback handler
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::Open
(
 const char* pURN,
 void* pPlaybackHandler,
 void* pUserData
 )
{
  bool result = false;
  URL *pUrl = NULL;

  //Get and queue OPEN command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::OPEN, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller open" );
  }
  else
  {
    pUrl = QTV_New_Args(URL, (pURN));
    pControllerCmd->m_openCmd.m_pURN = pUrl;
    pControllerCmd->m_openCmd.m_pPlaybackHandler = pPlaybackHandler;
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController Close command.
*
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::Close
(
 void* pUserData
 )
{
  bool result = false;

  //Get and queue CLOSE command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::CLOSE, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller CLOSE" );
  }
  else
  {
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController Start command.
*
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::Start
(
 void* pUserData
 )
{
  bool result = false;

  //Get and queue START command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::START, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller START" );
  }
  else
  {
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController Stop command.
*
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::Stop
(
 void* pUserData
 )
{
  bool result = false;

  //Get and queue STOP command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::STOP, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller STOP" );
  }
  else
  {
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController Play command.
*
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::Play
(
 void* pUserData
 )
{
  bool result = false;

  //Get and queue PLAY command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::PLAY, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller PLAY" );
  }
  else
  {
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController Pause command.
*
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::Pause
(
 void* pUserData
 )
{
  bool result = false;

  //Get and queue PAUSE command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::PAUSE, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller PAUSE" );
  }
  else
  {
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController Download command.
*
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::Download
(
 void* pUserData
 )
{
  bool result = false;

  //Get and queue DOWNLOAD command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::DOWNLOAD, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller DOWNLOAD" );
  }
  else
  {
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController Seek command.
*
* @param[in] seekTime - Seek start time
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::Seek
(
 const int64 seekTime,
 void* pUserData
 )
{
  bool result = false;

  //Get and queue SEEK command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::SEEK, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller SEEK" );
  }
  else
  {
    pControllerCmd->m_seekCmd.m_nSeekTime = seekTime;
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController GetTracks command.
*
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::GetTracks
(
 void* pUserData
 )
{
  bool result = false;

  //Get and queue GET_TRACKS command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::GET_TRACKS, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller GET_TRACKS" );
  }
  else
  {
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController SetTrackState command.
*
* @param[in] trackIndex - Index of track to set state for
* @param[in] bSelected - Flag to indicate the selection state for the track
* @param[in] pUserData - Reference to MMI callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::SetTrackState
(
 const int trackIndex,
 const bool bSelected,
 void* pUserData
 )
{
  bool result = false;

  //Get and queue SET_TRACK_STATE command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::SET_TRACK_STATE,
                                                   pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller SET_TRACK_STATE" );
  }
  else
  {
    pControllerCmd->m_setTrackStateCmd.m_trackIndex = trackIndex;
    pControllerCmd->m_setTrackStateCmd.m_bSelected = bSelected;
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

/** @brief Queue up HTTPController WaitForResources command.
*
* @param[in] pUserData - Reference to callback data
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPController::WaitForResources
(
 void* pUserData
 )
{
  bool result = false;

  //Get and queue WAIT_FOR_RESOURCES command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::WAIT_FOR_RESOURCES, pUserData);
  if (pControllerCmd == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller WAIT_FOR_RESOURCES" );
  }
  else
  {
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    result = true;
  }

  return result;
}

bool HTTPController::NotifyWaterMarkStatus
(
  uint32 portIDAndWatermarkType
)
{
  bool result = true;

  //Get and queue NOTIFY_CURRENT_WATERMARK_STATUS command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(
    HTTPCommon::NOTIFY_CURRENT_WATERMARK_STATUS, NULL);
  if (pControllerCmd == NULL)
  {
     QTV_MSG_PRIO( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                   "Unable to get command for HTTP controller to notify watermark status" );
    result = false;
  }
  else
  {
    pControllerCmd->m_notifyWatermarkEventCmd.m_portIdxAndWatermarkType = portIDAndWatermarkType;
    m_ctrlCmdQueue.QueueCmd(pControllerCmd);
    QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                 "Cmd to notify watermark status queued");
  }

  return result;

}

bool HTTPController::SetAuthorization(const char *authKey,
                                      const char *authValue)
{
  bool result = false;

  //Get and queue SET_AUTHORIZATION command
  ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(
    HTTPCommon::SET_AUTHORIZATION, NULL);
  if (pControllerCmd == NULL)
  {
     QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                  "Unable to get command for HTTP controller to set authorization");
  }
  else
  {
    if (authKey && authValue)
    {
      size_t authKeyBufSize = 1 + std_strlen(authKey);
      size_t authValueBufSize = 1 + std_strlen(authValue);

      pControllerCmd->m_setAuthenticationCmd.m_AuthHeader =
        (char *)QTV_Malloc(authKeyBufSize * sizeof(char));

      pControllerCmd->m_setAuthenticationCmd.m_AuthValue =
        (char *)QTV_Malloc(authValueBufSize * sizeof(char));

      if (pControllerCmd->m_setAuthenticationCmd.m_AuthHeader &&
          pControllerCmd->m_setAuthenticationCmd.m_AuthValue)
      {
        std_strlcpy(pControllerCmd->m_setAuthenticationCmd.m_AuthHeader,
                    authKey, authKeyBufSize);

        std_strlcpy(pControllerCmd->m_setAuthenticationCmd.m_AuthValue,
                    authValue, authValueBufSize);

        m_ctrlCmdQueue.QueueCmd(pControllerCmd);

        QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
                     "Cmd to SetAuthorization queued");

        result = true;
      }
      else
      {
        if (pControllerCmd->m_setAuthenticationCmd.m_AuthHeader)
        {
          QTV_Free(pControllerCmd->m_setAuthenticationCmd.m_AuthHeader);
        }

        if (pControllerCmd->m_setAuthenticationCmd.m_AuthValue)
        {
          QTV_Free(pControllerCmd->m_setAuthenticationCmd.m_AuthValue);
        }
      }
    }
    else
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "SetAuthorization failed. AuthKey %p or AuthValue %p is NULL",
                    (void*)authKey, (void*)authValue);

      result = false;
    }
  }

  return result;
}

/** @brief Starts connecting to the server and downloads the header if successfully
* connected (For FastTrack this also includes getting SDP and setting up selected
* tracks).
*
* @param[in] pUrn - URN to be opened
* @param[in] pPlaybackHandler - Reference to MMI playback handler
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecuteOpen
(
 URL* pUrn,
 void* pPlaybackHandler,
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecuteOpen - Executing OPEN, "
                 "HTTPController State = %d", GetState());

  bool bError = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::OPEN, this);

  //ToDo: Clear out net-abort signal??
  //ResetNetAbort();

  //Check if Open is allowed
  if (pUrn && m_pDownloader && IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bError))
  {
    //Set callback info
    HTTPControllerCbData cbData = {this, HTTP_CONNECT_TIMEOUT_MSEC,
                                   pUserData};

    tOpenParams params = {m_dataStorageOption, m_nHeapStorageLimit};
    status = m_pDownloader->StartSession(*pUrn,
                                         params,
                                         pPlaybackHandler,
                                         ExecuteOpenCallback,
                                         cbData);

    if (HTTPSUCCEEDED(status))
    {
      //Set state to CONNECTING
      SetState(CONNECTING);
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING ,QTVDIAG_PRIO_HIGH,
                    "HTTPController::ExecuteOpen successful" );
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: ExecuteOpen failed" );
      bError = true;
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: m_pDownloader/Url is NULL or not a good state for OPEN" );
  }// if (m_pDownloader && IsStateGood2ExecuteCmd())

  if (pUrn)
  {
    QTV_Delete(pUrn);
  }

  //Notify any failure - command execution status is notified later
  cmdExecHelper.Notify(bError, status, pUserData);
}

/** @brief Stops the HTTP session - brings down TCP.
*
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecuteClose
(
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecuteClose - Executing CLOSE, "
                 "HTTPController State = %d", GetState());

  bool bError = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::CLOSE, this);

  //Check if close is allowed
  if (m_pDownloader && IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bError))
  {
    status = HTTPCommon::HTTPDL_SUCCESS;

    //Set callback info
    HTTPControllerCbData cbData = {this, HTTP_CLOSE_TIMEOUT_MSEC,
                                   pUserData};
    State currState = GetState();

    if (currState == IDLE)
    {
      DestroySession(status, pUserData);
    }
    else
    {
      if (currState == CONNECTING)
      {
        m_pDownloader->SetNetAbort();
      }
      status = m_pDownloader->CloseSession(ExecuteCloseCallback,
                                           cbData);
      if (HTTPSUCCEEDED(status))
      {
        //Set state to CLOSING
        SetState(CLOSING);
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING ,QTVDIAG_PRIO_HIGH,
                      "HTTPController::ExecuteClose successful" );
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: ExecuteClose failed" );
        bError = true;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either m_pDownloader is NULL or not a good state for CLOSE" );
  }// if (m_pDownloader && IsStateGood2ExecuteCmd())

  //Notify any failure - command execution status is notified later
  cmdExecHelper.Notify(bError, status, pUserData);
}

/** @brief START marks the completion of track selection, initialize data download.
*
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecuteStart
(
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecuteStart - Executing START, "
                 "HTTPController State = %d", GetState());

  bool bError = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::START, this);

  //Check if start is allowed
  if (IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bError))
  {
    //Set callback info
    HTTPControllerCbData cbData = {this, HTTP_CONNECT_TIMEOUT_MSEC,
                                   pUserData};
    status = m_pDownloader->InitializeDownloader(ExecuteStartCallback,
                                                 cbData);
    if (HTTPSUCCEEDED(status))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING ,QTVDIAG_PRIO_HIGH,
                    "HTTPController::ExecuteStart successful" );
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: ExecuteStart failed" );
      bError = true;
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Not a good state for START" );
  }// if (IsStateGood2ExecuteCmd())

  //Notify command execution status
  cmdExecHelper.Notify(bError, status, pUserData);
}

/** @brief Stop the ongoing HTTP session, which is typically done to
* alter the track selection and restart the session with a new set
* of tracks - NOOP.
*
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecuteStop
(
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecuteStop - Executing STOP, "
                 "HTTPController State = %d", GetState());

  bool bNotify = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::STOP, this);

  //Check if stop is allowed
  if (IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bNotify))
  {
    status = HTTPCommon::HTTPDL_SUCCESS;
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Not a good state for STOP" );
  }// if (IsStateGood2ExecuteCmd())

  //Notify command execution status
  cmdExecHelper.Notify(bNotify, status, pUserData);
}

/** @brief Start/resume playback for the HTTP session - NOOP.
*
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecutePlay
(
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecutePlay - Executing PLAY, "
                 "HTTPController State = %d", GetState() );

  bool bNotify = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::PLAY, this);

  //Check if play is allowed
  if(m_pDownloader && IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bNotify))
  {
    //Resume data request servicing task
    status = m_pDownloader->ResumeSession();
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either m_pDownloader is NULL or "
                  "not a good state for RESUME" );
  }// if (m_pDownloader && IsStateGood2ExecuteCmd())

  //Notify any failure - command execution status is notified later
  cmdExecHelper.Notify(bNotify, status, pUserData);
}

/** @brief Pause playback for the HTTP session - NOOP.
*
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecutePause
(
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecutePause - Executing PAUSE, "
                 "HTTPController State = %d", GetState());

  bool bNotify = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::PAUSE, this);

  //Check if pause is allowed
  if(m_pDownloader && IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bNotify))
  {
    //Pause data request servicing task
    status = m_pDownloader->PauseSession();
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either m_pDownloader is NULL or "
                  "not a good state for PAUSE" );
  }// if (m_pDownloader && IsStateGood2ExecuteCmd())

  //Notify command execution status
  cmdExecHelper.Notify(bNotify, status, pUserData);
}

/** @brief Starts downloading data and writing it into data manager buffer.
*
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecuteDownload
(
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecuteDownload - Executing DOWNLOAD, "
                 "HTTPController State = %d", GetState());

  bool bError = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::DOWNLOAD, this);

  //Check if download is allowed
  if (m_pDownloader && IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bError))
  {
    //Set callback info
    HTTPControllerCbData cbData = {this, HTTP_DATAINACTIVITY_TIMEOUT_MSEC,
                                   pUserData};

    status = m_pDownloader->StartDownload(ExecuteDownloadCallback,
                                          cbData);

    if (HTTPSUCCEEDED(status))
    {
      //Set state to DOWNLOADING
      SetState(DOWNLOADING);
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "HTTPController::ExecuteDownload successful" );
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: ExecuteDownload failed" );
      bError = true;
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either m_pDownloader is NULL or not a good state for DOWNLOAD" );
  }// if (m_pDownloader && IsStateGood2ExecuteCmd())

  //Notify any failure - command execution status is notified later
  cmdExecHelper.Notify(bError, status, pUserData);
}

/** @brief Seeks to the specified position.
*
* @param[in] seekTime - Seek start time
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecuteSeek
(
 const int64 seekTime,
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecuteSeek - Executing SEEK, "
                 "HTTPController State = %d", GetState());

  bool bError = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::SEEK, this);

  //Check if seek is allowed
  if (m_pDownloader && IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bError))
  {
    //Set callback info and post SEEK into downloader
    HTTPControllerCbData cbData = {this, HTTP_SEEK_TIMEOUT_MSEC,
                                   pUserData};
    status = m_pDownloader->SeekSession(seekTime,
                                        ExecuteSeekCallback,
                                        cbData);
    if (HTTPSUCCEEDED(status))
    {
      bError = false;
    }

    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "HTTPController::ExecuteSeek status %d, seekTime %lld",
                   status, seekTime );
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either m_pDownloader is NULL or "
                  "not a good state for SEEK" );
  }// if (m_pDownloader && IsStateGood2ExecuteCmd())

  //Notify any failure - command execution status is notified later
  cmdExecHelper.Notify(bError, status, pUserData);
}

/** @brief Get the track list for the clip - notify success and the
* track list is obtained by MMITrackHandler.
*
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecuteGetTracks
(
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecuteGetTracks - Executing GET_TRACKS, "
                 "HTTPController State = %d", GetState());

  bool bNotify = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::GET_TRACKS, this);

  //Check if get tracks is allowed
  if (IsStateGood2ExecuteCmd(cmdExecHelper.GetCmd(), bNotify))
  {
    status = HTTPCommon::HTTPDL_SUCCESS;
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Not a good state for GET_TRACKS" );
  }// if (IsStateGood2ExecuteCmd())

  //Notify command execution status
  cmdExecHelper.Notify(bNotify, status, pUserData);
}

/** @brief Set the track state - notify success and the track state
* is set by MMITrackHandler.
*
* @param[in] trackIndex - Index of track to set state for
* @param[in] bSelected - Flag to indicate the selection state for the track
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::ExecuteSetTrackState
(
 const int32 /* trackIndex */,
 const bool /* bSelected */,
 void* pUserData
)
{
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HTTPController::ExecuteSetTrackState - Executing SET_TRACK_STATE, "
                 "HTTPController State = %d", GetState());

  bool bNotify = true;
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::SET_TRACK_STATE, this);
  //Notify command execution status
  cmdExecHelper.Notify(bNotify, status, pUserData);
}

/** @brief ExecuteWaitForResources - notify success
*
* @param[in] pUserData - Reference to callback data
*/
void HTTPController::ExecuteWaitForResources
(
 void* pUserData
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::WAIT_FOR_RESOURCES, this);
  //Notify command execution status
  cmdExecHelper.Notify(true, status, pUserData);
}

void HTTPController::ExecuteNotifyWaterMarkStatus
(
 uint32 portIdxAndWatermarkType
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  HTTPControllerCmdExecHelper cmdExecHelper(HTTPCommon::NOTIFY_CURRENT_WATERMARK_STATUS, this);

  cmdExecHelper.Notify(true, status, (void *)&portIdxAndWatermarkType);
}

void HTTPController::ExecuteSetAuthorization
(
 const char *headerName,
 const char *headerValue
)
{
  if(m_pDownloader)
  {
    static const char *authKey = "Authorization";
    static const char *proxyAuthKey = "Proxy-Authorization";

    if (headerName && headerValue)
    {
      bool bIsAuthHeaderSet = false;

      if (0 == std_stricmp(headerName, authKey) ||
          0 == std_stricmp(headerName, proxyAuthKey))
      {
        bIsAuthHeaderSet = true;
      }

      if (true == bIsAuthHeaderSet)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "SetOemHttpHeaders by client");

        PendingAuthElem *pElem =
          (PendingAuthElem *)StreamQ_check(&m_PendingAuthQueue);

        if (pElem)
        {
          pElem = (PendingAuthElem *)StreamQ_get(&m_PendingAuthQueue);
        }

        if (pElem)
        {
          HTTPAuthorizationInterface *pHTTPStack = pElem->m_HTTPStackNotificationData.m_pHTTPStack;

          if (pHTTPStack)
          {
            if (HTTP_SUCCESS == pHTTPStack->SetAuthorization(
              0, headerName, (int)std_strlen(headerName), headerValue, (int)std_strlen(headerValue)))
            {
              // check if there are pending auths which need to be
              // notified to client.
              PendingAuthElem *pHeadElem = (PendingAuthElem *)StreamQ_check(&m_PendingAuthQueue);

              if (pHeadElem)
              {

                QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Notify auth in SetHeader as there are pending notifcations to mmi helper");

                NotifyProtocolEvent();
              }
              else
              {
                // resume timeout logic for tasks.
                m_pDownloader->DisableTaskTimeout(false);
              }
            }
            else
            {
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                           "Failed to set authorization on httpstack");
            }
          }
          else
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Sanity check failed. Invalid auth element");
          }

          QTV_Delete(pElem);
          pElem = NULL;
        }
        else
        {
          QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Failed to find pending auth element");
        }
      }
    }
  }
}

void HTTPController::ExecuteSelectRepresentations(const char *pSelectionsXML)
{
  HTTPControllerCbData cbData = {this, 0, NULL /*pUserData*/};
  HTTPDownloadStatus rslt = m_pDownloader->SelectRepresentations(
     pSelectionsXML, ExecuteSelectRepresentationsCallback, cbData);

  if (HTTPCommon::HTTPDL_SUCCESS != rslt)
  {
    ExecuteSelectRepresentationsCallback(rslt, cbData);
  }
}

/** @brief Gets the http flavor from downloader
* @return - http flavor
*/
HTTPFlavor HTTPController::GetHTTPFlavor()
{
  HTTPFlavor pHTTPFlavor = HTTPCommon::HTTP_NONE;
  if(m_pDownloader)
  {
    pHTTPFlavor=m_pDownloader->GetHTTPFlavor();
  }
  return pHTTPFlavor;
}

void HTTPController::NotifyProtocolEvent()
{
  PendingAuthElem *pElem =
          (PendingAuthElem *)StreamQ_check(&m_PendingAuthQueue);

  if (pElem)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "ExecuteNotifyProtocolEvent httpstackstatus %d", pElem->m_HTTPNotifyCode);

    HTTPStackNotificationCbData httpStackNotificationData;
    httpStackNotificationData.m_serverCode = pElem->m_HTTPStackNotificationData.m_serverCode;
    httpStackNotificationData.m_serverMessage = pElem->m_HTTPStackNotificationData.m_serverMessage;
    httpStackNotificationData.m_entityBody = pElem->m_HTTPStackNotificationData.m_entityBody;
    httpStackNotificationData.m_msgType = pElem->m_HTTPStackNotificationData.m_msgType;
    httpStackNotificationData.m_method = pElem->m_HTTPStackNotificationData.m_method;
    httpStackNotificationData.m_protocolHeaders = pElem->m_HTTPStackNotificationData.m_protocolHeaders;
    httpStackNotificationData.m_pHTTPStack = NULL; // mmi doesnt need it.

    (void)m_pStackNotificationHandler->Notify(
      pElem->m_HTTPNotifyCode, &httpStackNotificationData);
  }
}

/**
 * @brief
 *  Client can call this when auth handling is discarded and so
 *  timeouts need to be re-enabled.
 */
void HTTPController::SetAuthHandlingDiscarded()
{
  if (m_pDownloader)
  {
    m_pDownloader->DisableTaskTimeout(false);
  }
}

/** @brief Destroys the HTTP session - cleans up and exits HTTP thread.
*
* @param[in] HTTPStatus - Close execution status
* @param[in] pUserData - Reference to MMI callback data
*/
void HTTPController::DestroySession
(
 HTTPDownloadStatus HTTPStatus,
 void* pUserData
)
{
  if (m_pDownloader)
  {
    State currState = GetState();

    if (currState == IDLE || currState == CLOSING)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "HTTP session ended - Close complete" );
      (void)m_pDownloader->ResetSession();

      //Stop scheduler (all outstanding sub tasks including the command
      //processing sub task are marked for deletion as part of this)
      m_pDownloader->StopScheduler();

      if (currState == CLOSING)
      {
        SetState(IDLE);
      }

      //Reset session here since stopping the scheduler effectively means
      //that HTTPController is not ready to take any more commands!!
      ResetSession();

      NotifyHTTPEvent(HTTPCommon::CLOSE, HTTPStatus, pUserData);
    }
    else
    {
      //Error - don't notify
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: Not a good state for destroy" );
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: m_pDownloader is NULL" );
  }// if (m_pDownloader)
}

/** @brief Starts the HTTP thread and creates the HTTPController command queue.
*
* @param[out] ppThreadHandle - Reference to thread handle
* @result
* TRUE - Successfully started thread and created command queue
* FALSE - Otherwise
*/
bool HTTPController::Create()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPController::Create" );

  //Kick off the thread
  bool bOk = StartThread();
  if (bOk)
  {
    //Add command processing task
    SchedulerTask pTask = TaskProcessCommands;

    //Fill out the task param structure - to be passed to the task
    HTTPControllerCmdTaskParam* pCmdTaskParam = QTV_New(HTTPControllerCmdTaskParam);
    if (pCmdTaskParam == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: Memory allocation failed for taskParam" );
      bOk = false;
    }
    else
    {
      pCmdTaskParam->pCbSelf = (void*)this;

      if (!AddSchedulerTask(pTask, (void*)pCmdTaskParam))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPControllerCmd task could not be added" );
        bOk = false;
        if (pCmdTaskParam)
        {
          QTV_Delete(pCmdTaskParam);
          pCmdTaskParam = NULL;
        }
      }
    }// if (pCmdTaskParam == NULL)
  }// if (bOk)

  if (!bOk)
  {
    //Create/Start HTTP streamer thread failure
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: HTTPController::Create failed" );
  }
  else
  {
    // HTTP streamer thread is created and added to schedular, so we can turn on the flag
    SetHTTPStreamerRunning(true);
  }

  return bOk;
}

/** @brief Creates and starts the HTTP thread (if not already done).
*
* @result
* TRUE - Successfully started thread
* FALSE - Otherwise
*/
bool HTTPController::StartThread()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPController::StartThread" );

  bool bOk = IsHTTPStreamerRunning();

  //Start the HTTP thread - only one thread can be active at a time
  if (!bOk)
  {
    SetState(IDLE);

    //Release earlier instance of HTTP thread (if any)
    ReleaseThread();

    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "Creating HTTP streamer thread" );

    if (MM_Thread_CreateEx( HTTP_STREAMER_THREAD_PRIORITY, 0, HTTPStreamerThreadEntryFunction,
                            reinterpret_cast<void *> (this), HTTP_STREAMER_STACK_SIZE, "HTTPStreamer",
                            &m_pHTTPStreamerThread) == 0)
    {
      bOk = true;
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: Thread creation failed" );
    }
  }// if (!bOk)
  else
  {
    //HTTP streamer thread already running
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "HTTP streamer thread already running" );
    bOk = false;
  }

  return bOk;
}

/** @brief Process commands task.
*
* @param[in] pTaskParam - Task parameter
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/
int HTTPController::TaskProcessCommands
(
 void* pTaskParam
 )
{
  int result = -1;
  HTTPControllerCmdTaskParam* parameter =
  (HTTPControllerCmdTaskParam*) pTaskParam;

  if (!parameter || !parameter->pCbSelf)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Invalid taskParam" );
  }
  else
  {
    HTTPController* pSelf = (HTTPController*)parameter->pCbSelf;
    pSelf->m_ctrlCmdQueue.ProcessAllCmds(pSelf);
    result = 0;
  }

  return result;
}

/** @brief Entry point routine for HTTP streamer thread.
*
* @param[in] pTaskParam - Task parameter
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/
int HTTPController::HTTPStreamerThreadEntryFunction
(
 void* pTaskParam
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPController::HTTPStreamerThreadEntryFunction" );
  int result = -1;

  HTTPController* pSelf = (HTTPController*) pTaskParam;
  if (pSelf == NULL || pSelf->m_pDownloader == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Invalid taskParam or pDownloader" );
  }
  else
  {
    result = 0;

    pSelf->SetHTTPStreamerRunning(true);

    (void)pSelf->m_pDownloader->StartScheduler();

    //Exit the HTTP streamer thread
    if (pSelf->m_pHTTPStreamerThread)
    {
      MM_Thread_Exit(pSelf->m_pHTTPStreamerThread, 0);
    }
  }

  return result;
}

/** @brief Reset the just-terminated HTTP session.
*
*/
void HTTPController::ResetSession()
{
  //Flush any outstanding controller commands
  m_ctrlCmdQueue.FlushCmds();
  SetHTTPStreamerRunning(false);
}

/** @brief ExecuteOpen callback - invoked by HTTPDownloader.
*
* @param[in] HTTPStatus - Command execution status
* @param[in] callbackData - Reference to callback data
*/
void HTTPController::ExecuteOpenCallback
(
 HTTPDownloadStatus HTTPStatus,
 const HTTPControllerCbData& callbackData
)
{
  HTTPController* pSelf = callbackData.pSelf;
  if (pSelf)
  {
    //Set state to CONNECTED if success, failure is handled by MMI
    if (HTTPSUCCEEDED(HTTPStatus))
    {
      pSelf->SetState(CONNECTED);
    }

    //Notify HTTP status to MMI
    pSelf->NotifyHTTPEvent(HTTPCommon::OPEN, HTTPStatus, callbackData.pCbData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: pSelf is NULL" );
  }
}

/** @brief ExecuteClose callback -  invoked by HTTPDownloader.
*
* @param[in] HTTPStatus - Command execution status
* @param[in] callbackData - Reference to callback data
*/
void HTTPController::ExecuteCloseCallback
(
 HTTPDownloadStatus HTTPStatus,
 const HTTPControllerCbData& callbackData
)
{
  HTTPController* pSelf = callbackData.pSelf;
  HTTPStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
  if (pSelf)
  {
    HTTPStatus = HTTPCommon::HTTPDL_SUCCESS;
    //Destroy session here
    pSelf->DestroySession(HTTPStatus, callbackData.pCbData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: pSelf is NULL" );
  }
}

/** @brief ExecuteStart callback - invoked by HTTPDownloader.
*
* @param[in] HTTPStatus - Command execution status
* @param[in] callbackData - Reference to callback data
*/

void HTTPController::ExecuteStartCallback
(
 HTTPDownloadStatus HTTPStatus,
 const HTTPControllerCbData& callbackData
)
{
  HTTPController* pSelf = callbackData.pSelf;
  if (pSelf)
  {
    pSelf->NotifyHTTPEvent(HTTPCommon::START, HTTPStatus, callbackData.pCbData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: pSelf is NULL" );
  }
}

/** @brief ExecuteDownload callback - invoked by HTTPDownloader.
*
* @param[in] HTTPStatus - Command execution status
* @param[in] callbackData - Reference to callback data
*/
void HTTPController::ExecuteDownloadCallback
(
 HTTPDownloadStatus HTTPStatus,
 const HTTPControllerCbData& callbackData
)
{
  HTTPController* pSelf = callbackData.pSelf;
  if (pSelf)
  {
    //Set state to DOWNLOAD_DONE if success, failure is handled by MMI
    if (HTTPSUCCEEDED(HTTPStatus))
    {
      pSelf->SetState(DOWNLOAD_DONE);
    }

    //Notify HTTP status to MMI
    pSelf->NotifyHTTPEvent(HTTPCommon::DOWNLOAD, HTTPStatus, callbackData.pCbData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: pSelf is NULL" );
  }
}

/** @brief ExecuteSeek callback - invoked by HTTPDownloader.
*
* @param[in] HTTPStatus - Command execution status
* @param[in] callbackData - Reference to callback data
*/
void HTTPController::ExecuteSeekCallback
(
 HTTPDownloadStatus HTTPStatus,
 const HTTPControllerCbData& callbackData
)
{
  HTTPController* pSelf = callbackData.pSelf;
  if (pSelf)
  {
    //Notify HTTP status to MMI
    pSelf->NotifyHTTPEvent(HTTPCommon::SEEK, HTTPStatus, callbackData.pCbData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: pSelf is NULL" );
  }
}

void HTTPController::ExecuteSelectRepresentationsCallback
(
  HTTPDownloadStatus HTTPStatus,
  const HTTPControllerCbData& callbackData
)
{
  HTTPController* pSelf = callbackData.pSelf;
  if (pSelf)
  {
    //Notify HTTP status to HTTPSourceMAPI
    pSelf->NotifyHTTPEvent(HTTPCommon::SELECT_REPRESENTATIONS, HTTPStatus, callbackData.pCbData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: pSelf is NULL" );
  }
}
/** @brief Notify HTTP event along with the associated HTTP command to source.
*
* @param[in] HTTPCommand - HTTP controller command
* @param[in] HTTPStatus - Command execution status
* @param[in] pMAPICbData - Reference to MMI callback data
*/
void HTTPController::NotifyHTTPEvent
(
 const HTTPControllerCommand HTTPCommand,
 const HTTPDownloadStatus HTTPStatus,
 void* pCbData
)
{
  if (GetState() == CLOSING)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "HTTP notification suppressed since HTTP streamer is CLOSING" );
  }
  else if (m_pHTTPSourceNotificationHandler)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                   "NotifyHTTPEvent - Command %d Status %d",
                   HTTPCommand, HTTPStatus );
    m_pHTTPSourceNotificationHandler->NotifyHTTPEvent(HTTPCommand,
                                                      HTTPStatus,
                                                      pCbData);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: m_pNotificationHandler is NULL" );
  }
}

/** @brief Set the HTTP controller state to the new state.
*
* @param[in] newState - New HTTP controller state
*/
void HTTPController::SetState
(
 const State newState
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPController::SetState" );

  //Set current state to new state (if already not in the new state)
  if (m_state != newState)
  {
    m_state = newState;
    switch (m_state)
    {
    case IDLE:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "HTTPController is IDLE" );
      break;
    case CONNECTING:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "HTTPController is CONNECTING" );
      break;
    case CONNECTED:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "HTTPController is CONNECTED" );
      break;
    case DOWNLOADING:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "HTTPController is DOWNLOADING" );
      break;
    case DOWNLOAD_DONE:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "HTTPController is DOWNLOAD_DONE" );
      break;
    case CLOSING:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "HTTPController is CLOSING" );
      break;
    default:
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "Invalid state" );
      break;
    }
  }
}

/** @brief This method checks the HTTPController state to see if the specified
* command can be executed and whether or not to notify the error if not a good
* state to execute the command.
*
* @param[in] cmd - HTTP controller command to be executed
* @param[out] bNotify - Flag indicating whether or not error notification be
*                       sent back
* @return
* TRUE - If the command can be executed
* FALSE - If the command cannot be executed
*/
bool HTTPController::IsStateGood2ExecuteCmd
(
 const HTTPControllerCommand cmd,
 bool& bNotify
)
{
  bool bOk = bNotify = false;
  State state = GetState();

  if ((cmd < HTTPCommon::CMD_MAX) && (state < STATE_MAX))
  {
    bOk = CommandStateMatrix[cmd][state][0];
    bNotify = CommandStateMatrix[cmd][state][1];
  }

  if (!bOk)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "HTTPController NOT in a good state to execute %d command", cmd );
  }

  return bOk;
}

/** @brief Gets the total duration of the clip
*
* @param[out] duration - duration of the clip
* @return
* TRUE - If duration is successfully obtained
* FALSE - Otherwise
*/
bool HTTPController::GetTotalDuration(uint32 &duration)
{
  bool bOk = false;
  if (m_pDownloader)
  {
    bOk = m_pDownloader->GetTotalDuration(duration);
  }
  return bOk;
}
/** @brief Check if session is live streaming session
*
* @param[out] bLiveStreamingSession - true if live streaming session
* @return
* TRUE - Live streaming flag successfully obtained
* FALSE - Otherwise
*/
bool HTTPController::IsLiveStreamingSession
(
 bool& bLiveStreamingSession
)
{
  bool bOk = false;

  if (m_pDownloader)
  {
    bLiveStreamingSession = m_pDownloader->IsLiveStreamingSession();
    bOk = true;
  }

  return bOk;
}

/**
* @breif: Set the ProxyServer info with the HTTP stack
*
* @param[in] Proxyserver address string, includes Port
* @param[in] proxyserver address length
*
* @return 0 if success, -1 otherwise
*/
bool HTTPController::SetProxyServer
(
  const char* ProxyServer,
  size_t ProxyServerLen
)
{
  bool rsltCode = false;

  if(m_pDownloader)
  {
    rsltCode = m_pDownloader->SetProxyServer(ProxyServer,ProxyServerLen);
  }
  return rsltCode;
}

uint32 HTTPController::GetMaxSupportedVideoBufferSize()
{
  return (m_pDownloader ? m_pDownloader->GetMaxSupportedVideoBufferSize() : 0);
}

/**
* @breif: Set the Cookies info
*
* @param[in] url
* @param[in] cookie
*
* @return 0 if success, -1 otherwise
*/
bool HTTPController::SetCookie
(
  const char *url,
  const char *cookie
)
{
  bool rsltCode = false;

  if(m_pDownloader)
  {
    rsltCode = m_pDownloader->SetCookie(url,cookie);
  }
  return rsltCode;
}


/**
* @breif: Get the Cookies info
*
* @param[in] url
* @param[in] cookie
* @param[in/out] cookie len
*
* @return 0 if success, -1 otherwise
*/
bool HTTPController::GetCookies
(
  const char *url,
  const char *cookies,
  size_t &cookiesLen
)
{
  bool rsltCode = false;

  if(m_pDownloader)
  {
    rsltCode = m_pDownloader->GetCookies(url,cookies, cookiesLen);
  }
  return rsltCode;
}

/**
 * @brief Get Proxyserver info
 *
 * @param ProxyServer
 * @param ProxyServerLen
 * @param ProxyServerLenReq
 *
 * @return  < 0  if there is an error .
 *           0   if the proxyServer or ProxyServerlen is
 *               invalid i.e less than the actual proxyServer
 *               size.
 *           > 0 on success.
 */
int32 HTTPController::GetProxyServer
(
 char* ProxyServer,
 size_t ProxyServerLen,
 size_t & ProxyServerLenReq
)
{
  int32 rsltCode = -1;

  if(m_pDownloader)
  {
    rsltCode = m_pDownloader->GetProxyServer(ProxyServer,
                                           ProxyServerLen,
                                           ProxyServerLenReq);
  }
  return rsltCode;
}

/**
* @breif: set network Profile
*
* @param[in] networkProfile number
*
* @return true if success otherwise false
*/
bool HTTPController::SetNetworkProfile(uint32 pdpProfileNo)
{
  bool rsltCode = false;

  if(m_pDownloader)
  {
    rsltCode = m_pDownloader->SetPrimaryPDPProfile(pdpProfileNo);
  }

  return rsltCode;
}

/**
* @breif: Get network Profile
*
* @param[out] Pointer to networkProfile number
*
* @return true if success otherwise false
*/
bool HTTPController::GetNetworkProfile(uint32 &pNetworkProfileId)
{
  bool rsltCode = false;

  if( m_pDownloader)
  {
    int32 networkProfileNo;
    rsltCode = m_pDownloader->GetPrimaryPDPProfile(networkProfileNo);

    if(rsltCode)
    {
      pNetworkProfileId = (uint32)networkProfileNo;
    }
  }
  return rsltCode;
}

/**
* @breif: set network Iface
*
* @param[in] networkIface ID
*
* @return true if success otherwise false
*/
bool HTTPController::SetNetworkInterface
(
  int32 networkIfaceId
)
{
  bool rsltCode = false;

  if(m_pDownloader)
  {
    rsltCode = m_pDownloader->SetNetworkIface(networkIfaceId);
  }

  return rsltCode;
}

/**
* @breif: Get network Iface
*
* @param[out] pointer to networkIface ID
*
* @return true if success otherwise false
*/
bool HTTPController::GetNetworkInterface
(
  int32 &networkIfaceId
)
{
  bool rsltCode = false;

  if( m_pDownloader)
  {
    rsltCode = m_pDownloader->GetNetworkIface(networkIfaceId);
  }
  return rsltCode;
}

/**
* @brief Get Number of bytes downloaded till now
*
* @param[out] nBytesReceived
*
* @return True indicating sucess , False otherwise
*/
bool HTTPController::GetTotalbytesReceived
(
  int64 &nBytesReceived
)
{
  bool rsltCode = false;

  if(m_pDownloader)
  {
    nBytesReceived = m_pDownloader->GetTotalbytesReceived();
    rsltCode = true;
  }
  return rsltCode;
}

/**
* @brief Get the playback handler interface.
*
* @return Playback handler iface
*/
iHTTPPlaybackHandler* HTTPController::GetPlaybackHandler()
{
  return (m_pDownloader ? m_pDownloader->GetPlaybackHandler() : NULL);
}

/**
* @brief Sets the initial preroll
*
* @param [in] preroll - the preroll
*
*/
void HTTPController::SetInitialPreroll(const uint32 preroll)
{
  if ( m_pDownloader )
  {
    m_pDownloader->SetInitialPreroll(preroll);
  }
}

/**
* @brief Gets the initial preroll
*
* @return the preroll
*
*/
uint32 HTTPController::GetInitialPreroll()
{
  if ( m_pDownloader )
  {
    return m_pDownloader->GetInitialPreroll();
  }
  else
  {
    return 0;
  }
}

/**
* @brief Sets the max http Reqs
*
* @param [in] httpReqsLimit - the httpReqsLimit
*
*/
void HTTPController::SetHTTPRequestsLimit(const uint32 httpReqsLimit)
{
  if ( m_pDownloader )
  {
    m_pDownloader->SetHTTPRequestsLimit(httpReqsLimit);
  }
}

/**
* @brief Gets max http Reqs
*
* @return the httpReqsLimit
*
*/
uint32 HTTPController::GetHTTPRequestsLimit()
{
  if ( m_pDownloader )
  {
    return m_pDownloader->GetHTTPRequestsLimit();
  }
  else
  {
    return 0;
  }
}

/**
* @brief Sets the rebuffering preroll
*
* @param [in] preroll - the preroll
*
*/
void HTTPController::SetRebufferPreroll(const uint32 preroll)
{
  if ( m_pDownloader )
  {
    m_pDownloader->SetRebufferPreroll(preroll);
  }
}

/**
* @brief Gets the rebuffering preroll
*
* @return the rebuffering preroll
*
*/
uint32 HTTPController::GetRebufferPreroll()
{
  if ( m_pDownloader )
  {
    return m_pDownloader->GetRebufferPreroll();
  }
  else
  {
    return 0;
  }
}

void HTTPController::SetDataUnitCancellationDisabled(bool bIsDisabled)
{
  if (m_pDownloader)
  {
    m_pDownloader->SetDataUnitCancellationDisabled(bIsDisabled);
  }
}

bool HTTPController::IsDataUnitCancellationDisabled() const
{
  bool rslt = false;

  if (m_pDownloader)
  {
    rslt = m_pDownloader->GetDataUnitCancellationDisabled();
  }

  return rslt;
}

void HTTPController::SetMaxSupportedRepBandwidth(uint32 maxBw)
{
   if (m_pDownloader)
   {
     m_pDownloader->SetMaxSupportedRepBandwidth(maxBw);
   }
}

uint32 HTTPController::GetMaxSupportedRepBandwidth() const
{
  uint32 maxBw = 0;

  if (m_pDownloader)
  {
    maxBw = m_pDownloader->GetMaxSupportedRepBandwidth();
  }

  return maxBw;
}

void HTTPController::SetMaxSupportedASCValue(uint32 maxASCVal)
{
   if (m_pDownloader)
   {
     m_pDownloader->SetMaxSupportedASCValue(maxASCVal);
   }
}

uint32 HTTPController::GetMaxSupportedASCValue() const
{
  uint32 maxASCValue = 0;
  if (m_pDownloader)
  {
    maxASCValue = m_pDownloader->GetMaxSupportedASCValue();
  }

  return maxASCValue;
}

void HTTPController::UseTsbForStartupLatencyImprovement()
{
  if (m_pDownloader)
  {
    m_pDownloader->UseTsbForStartupLatencyImprovement();
  }
}

bool HTTPController::IsStartupLatencyImprovementEnabled() const
{
  return m_pDownloader ? m_pDownloader->IsStartupLatencyImprovementEnabled() : false;
}

void HTTPController::SetAudioSwitchingEnabled(bool bIsEnabled)
{
  if (m_pDownloader)
  {
    m_pDownloader->SetAudioSwitchingEnabled(bIsEnabled);
  }
}

bool HTTPController::IsAudioSwitchingEnabled() const
{
  return m_pDownloader ? m_pDownloader->IsAudioSwitchingEnabled() : false;
}

/**
* @brief Edit oem headers.
*
* @return success status of the operation
*/
bool HTTPController::SetOemHttpHeaders(IPStreamProtocolHeaderCommand whatToDo,
                                       uint32 whichMethodsAffected,
                                       const char *headerName,
                                       const char *headerValue)
{
  bool ret = false;
  if(m_pDownloader)
  {
    static const char *authKey = "Authorization";
    static const char *proxyAuthKey = "Proxy-Authorization";

    if (headerName && headerValue)
    {
      bool bIsAuthHeaderSet = false;

      if (0 == std_stricmp(headerName, authKey) ||
          0 == std_stricmp(headerName, proxyAuthKey))
      {
        bIsAuthHeaderSet = true;
      }

      if (true == bIsAuthHeaderSet)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "SetAuthorization by client");
        ret = SetAuthorization(headerName, headerValue);

      }
      else
      {
        ret = m_pDownloader->SetOemHttpHeaders(whatToDo, whichMethodsAffected, headerName, headerValue);
      }
    }
    else
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "HTTPDownloader::SetOemHttpHeaders headerName '%p' or headerValue '%p' is NULL",
        (void*)headerName, (void*)headerValue);
    }
  }

  return ret;
}

/**
* @brief get oem headers.
*
* @return success status of the operation
*/
bool HTTPController::GetOemHttpHeaders(uint32 whichMethodsAffected,
                                       const char *headerName,
                                       char *headerValue,
                                       int& headerValueSize)
{
  bool ret = false;
  if(m_pDownloader)
  {
    ret = m_pDownloader->GetOemHttpHeaders(whichMethodsAffected, headerName, headerValue, headerValueSize);
  }
  return ret;
}

/**
 * @brief Get DownloadProgress from downloadHelpers
 * @param mediaType
 * @param currStartOffset
 * @param downloadOffset
 * @param eUnitsType
 * @param bEOS
 *
 * @return bool
 */
bool HTTPController::GetDownloadProgress
(
 HTTPMediaType mediaType,
 uint32& currStartOffset,
 uint32& downloadOffset,
 HTTPDownloadProgressUnitsType eUnitsType,
 bool& bEOS
)
{
  bool retVal = false;

  if(m_pDownloader)
  {
    retVal = m_pDownloader->GetDownloadProgress(mediaType,currStartOffset,
                                                downloadOffset, eUnitsType, bEOS);

    if(retVal && eUnitsType == HTTPCommon::HTTP_DOWNLOADPROGRESS_UNITS_TIME )
    {
      //convert to micro secs
      currStartOffset *= 1000;
      downloadOffset *= 1000;
    }
  }

  return retVal;
}

/**
 *  To get the data interface for the current session.
 *
  * @param[out] pDataInterface - Reference to data interface
 */
void HTTPController::GetDataInterface
(
 HTTPDataInterface *&pDataInterface
)
{
  pDataInterface = NULL;
  if (m_pDownloader)
  {
    pDataInterface = m_pDownloader->GetDataInterface();
  }
  return;
}


HTTPController::HTTPStackNotificationCbDataForAuth::HTTPStackNotificationCbDataForAuth() :
  m_serverCode(0),
  m_serverMessage(NULL),
  m_entityBody(NULL),
  m_msgType(false),
  m_method(NULL),
  m_protocolHeaders(NULL),
  m_pHTTPStack(NULL)
{

}

HTTPController::HTTPStackNotificationCbDataForAuth::~HTTPStackNotificationCbDataForAuth()
{
  if (m_protocolHeaders)
  {
    QTV_Free(m_protocolHeaders);
    m_protocolHeaders= NULL;
  }

  if (m_method)
  {
    QTV_Free(m_method);
    m_method = NULL;
  }

  if (m_entityBody)
  {
    QTV_Free(m_entityBody);
    m_entityBody = NULL;
  }

  if (m_serverMessage)
  {
    QTV_Free(m_serverMessage);
    m_serverMessage = NULL;
  }
}

/**
 * Retreive the Dash Adapatation Properties
 *
 * @param pPropertiesStr
 * @param nPropertiesLen
 *
 * @return True if success, false otherwise
 */
bool HTTPController::GetMediaProperties
(
 char *pPropertiesStr,
 uint32& nPropertiesLen
)
{
  bool rslt = false;
  if(m_pDownloader)
  {
    rslt = m_pDownloader->GetMediaProperties(pPropertiesStr,nPropertiesLen);
  }
  return rslt;

}

/**
 * Select the representations
 *
 * @param SetSelectionsXML
 *
 * @return True if success, false otherwise
 */
bool HTTPController::SelectRepresentations
(
 const char* SetSelectionsXML
)
{
  bool result = false;

  //Get and queue SetSelections command
  if (SetSelectionsXML)
  {
    ControllerCmd *pControllerCmd = m_ctrlCmdQueue.GetCmd(HTTPCommon::SELECT_REPRESENTATIONS, NULL);
    if (pControllerCmd == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Unable to get command for HTTP controller SELECT_REPRESENTATIONS" );
    }
    else
    {
      size_t reqdBufSize = 1 + std_strlen(SetSelectionsXML);
      pControllerCmd->m_SelectRepresentationsCmd.m_pSelectionsXML =
        (char *)QTV_Malloc(reqdBufSize * sizeof(char));

      if (pControllerCmd->m_SelectRepresentationsCmd.m_pSelectionsXML)
      {
        std_strlcpy(pControllerCmd->m_SelectRepresentationsCmd.m_pSelectionsXML,
                    SetSelectionsXML,
                    reqdBufSize);
        m_ctrlCmdQueue.QueueCmd(pControllerCmd);
        result = true;
      }
    }
  }

  return result;
}

/**
 * Retreive  Dash MPD Text
 *
 * @param pMPDTextStr
 * @param mpdSize
 *
 * @return True if success, false otherwise
 */
bool HTTPController::GetMPDText(char *pMPDTextStr, uint32 &mpdSize)
{
  bool rslt = false;
  if(m_pDownloader)
  {
    rslt = m_pDownloader->GetMPDText(pMPDTextStr,mpdSize);
  }
  return rslt;

}
/**
 * Retreive  QOE data
 *
 * @param bandwidth
 * @param pVideoURL
 * @param nURLSize
 * @param pIpAddr
 * @param nIPAddrSize
 *
 */
void HTTPController::GetQOEData(uint32 &bandwidth, char* pVideoURL, size_t& nURLSize, char* pIpAddr, size_t& nIPAddrSize)
{
  if(m_pDownloader)
  {
    m_pDownloader->GetQOEData(bandwidth,pVideoURL,nURLSize,pIpAddr,nIPAddrSize);
  }
}

}/* namespace video */
