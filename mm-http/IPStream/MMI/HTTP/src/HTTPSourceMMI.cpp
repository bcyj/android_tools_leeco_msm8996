/************************************************************************* */
/**
 * HTTPSourceMMI.cpp
 * @brief Implementation of HTTPSourceMMI.
 *  HTTPSourceMMI is the entry point for the HTTP component functionality.
 *  It talks to HTTPController for the download of HTTP data and File Source
 *  module for the playback.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMI.cpp#67 $
$DateTime: 2013/09/25 17:37:20 $
$Change: 4496018 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMI.cpp
** ======================================================================= */
#include "HTTPSourceMMI.h"
#include "HTTPSourceMMIHelper.h"
#include "HTTPSourceMMIPropertiesHandler.h"
#include "HTTPSourceMMIStreamPortHandler.h"
#include "HTTPSourceMMITrackHandler.h"
#include "HTTPController.h"
#include "HTTPSourceMMIExtensionHandler.h"
#include <SourceMemDebug.h>
#ifdef _ANDROID_
#include <time_genoff.h>
#endif

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */
#define COMPONENT_ROLE "container_streaming.http"
const uint32 TIME_BASE = 1000;
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

/** @brief This function will create a Http Source instance and update handle.
*
*@param[out] : pHTTPSourceHandle handle for http source instance
*@return : open status
*/
OMX_U32 HTTPSourceMMI::HTTPSourceMMIOpen(OMX_HANDLETYPE *pHTTPSourceHandle)
{
  int32  ret = MMI_S_EFAIL;
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "HTTPSourceMMI::HTTPSourceMMIOpen");
  /* VOSAL Initialization*/
  MM_Memory_InitializeCheckPoint();
  (void)MM_Debug_Initialize();

  #ifdef _ANDROID_
  /*
   * Call time_control_operations with T_DISABLE to disable time_services logging
   */
  uint64 timeval = 0;
  time_genoff_info_type timeGenOffInfo;
  timeGenOffInfo.base = ATS_UTC;
  timeGenOffInfo.unit = TIME_MSEC;
  timeGenOffInfo.operation = T_DISABLE;
  timeGenOffInfo.ts_val = &timeval;

  if(time_control_operations(&timeGenOffInfo) != 0)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPSourceMMI::HTTPSourceMMIOpen time_services logging not disabled");
  }
  #endif

  HTTPSourceMMI  *pHttpSource = QTV_New(HTTPSourceMMI);
  if ( NULL == pHttpSource )
  {
    return MMI_S_ENOSWRES;
  }

  *pHTTPSourceHandle = (OMX_HANDLETYPE)pHttpSource;

  pHttpSource->InitPorts();

  if (pHttpSource->Create())
  {
    ret = MMI_S_COMPLETE;
  }

  return ret;
}

/** @brief Closes Http Source instance
*
*@param[in]  :pHTTPSourceHandle handle of http source instance
*@return :close status
*/
OMX_U32 HTTPSourceMMI::HTTPSourceMMIClose(OMX_HANDLETYPE pHTTPSourceHandle)
{
  OMX_U32  ret = MMI_S_PENDING;

  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "HTTPSourceMMI::HTTPSourceMMIClose");

  HTTPSourceMMI  *pHttpSource  =  NULL;
  pHttpSource  =  (HTTPSourceMMI *)pHTTPSourceHandle;
  if ( NULL == pHttpSource)
  {
     return MMI_S_ENOSWRES;
  }

  pHttpSource->SetShutDownInProgress(true);
  ret = pHttpSource->Close();

  QTV_Delete(pHttpSource);

  /* VOSAL Deinitialization */
  (void)MM_Debug_Deinitialize();
  MM_Memory_ReleaseCheckPoint();

  return ret;
}

/** @brief This function will create a Http Source instance and update handle.
*
*@param[in]  :pHTTPSourceHandle - source handle,
              nCode - command code,
              pData - userData associated with command
*@return Command status
*/
OMX_U32 HTTPSourceMMI::HTTPSourceMMICmd(OMX_HANDLETYPE pHTTPSourceHandle,
                                        OMX_U32 nCode, OMX_PTR pData)
{
  OMX_U32  ret = MMI_S_COMPLETE;
  HTTPSourceMMI  *pHttpSource = NULL;

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "HTTPSourceMMI::HTTPSourceMMICmd, code (relative to MMI_CMD_BASE) = %u",
               (uint32)(nCode - MMI_CMD_BASE));

  pHttpSource = (HTTPSourceMMI *)pHTTPSourceHandle;
  if ( NULL == pHttpSource)
  {
   return MMI_S_EFATAL;
  }

  switch (nCode)
  {
    case MMI_CMD_SET_STD_OMX_PARAM :
      ret = pHttpSource->SetParam(pData);
      break;
    case MMI_CMD_GET_STD_OMX_PARAM:
      ret = pHttpSource->GetParam(pData);
      break;
    case MMI_CMD_SET_CUSTOM_PARAM:
      ret = pHttpSource->SetCustomParam(pData);
      break;
    case MMI_CMD_GET_CUSTOM_PARAM:
      ret = pHttpSource->GetCustomParam(pData);
      break;
    case MMI_CMD_FILL_THIS_BUFFER:
      ret = pHttpSource->FillThisBuffer(pData);
      break;
    case MMI_CMD_ALLOC_BUFFER:
      ret = pHttpSource->AllocBuffer(pData);
      break;
    case MMI_CMD_FREE_BUFFER:
      ret = pHttpSource->FreeBuffer(pData);
      break;
    case MMI_CMD_START:
        ret = pHttpSource->Start(pData);
      break;
    case MMI_CMD_STOP:
      if(pHttpSource->m_pStopPhrase && (std_strlen(pHttpSource->m_pStopPhrase) == 0))
      {
        pHttpSource->UpdateStopPhrase((char*)STOP_STRING);
      }
      pHttpSource->Stop(pData);
      ret = pHttpSource->Close(pData);
      break;
    case MMI_CMD_PAUSE:
      ret = pHttpSource->Pause(pData);
      break;
    case MMI_CMD_RESUME:
      ret = pHttpSource->Play(pData);
      break;
    case MMI_CMD_FLUSH:
      ret = pHttpSource->Flush(pData);
      break;
    case MMI_CMD_LOAD_RESOURCES:
      ret = pHttpSource->LoadResources(pData);
      break;
    case MMI_CMD_RELEASE_RESOURCES:
      ret = pHttpSource->ReleaseResources();
      break;
    case MMI_CMD_WAIT_FOR_RESOURCES:
      ret = pHttpSource->WaitForResources(pData);
      break;
    case MMI_CMD_RELEASE_WAIT_ON_RESOURCES:
      ret = pHttpSource->ReleaseWaitForResources();
      break;
    case MMI_CMD_GET_EXTENSION_INDEX:
        ret = pHttpSource->m_pHTTPSourceMMIExtensionHandler.ProcessMMIGetExtensionCommand(
                                                         (MMI_GetExtensionCmdType *)pData);
      break;
    default:
      ret = MMI_S_EBADPARAM;
      break;
  }
  return ret;
}

/** @brief This function will create a Http Source instance and update handle.
*
*@param[in]  :pHTTPSourceHandle - source handle
              pfnEvtHdlr - event handler notification callback
              pClientData - client data associated with command
*@param[out]
*@return : event handler status
*/
OMX_U32 HTTPSourceMMI::HTTPSourceMMIRegisterEventHandler(OMX_HANDLETYPE pHTTPSourceHandle,
                                                      MMI_CmpntEvtHandlerType pfnEvtHdlr,
                                                      void *pClientData)
{
  int ret = MMI_S_COMPLETE;
  HTTPSourceMMI  *pHttpSource = NULL;

  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "HTTPSourceMMI::HTTPSourceMMIRegisterEventHandler");

  if( NULL == pfnEvtHdlr )
  {
    return MMI_S_EBADPARAM;
  }

  pHttpSource = (HTTPSourceMMI *)pHTTPSourceHandle;
  if ( NULL == pHttpSource)
  {
    return MMI_S_EFATAL;
  }

  pHttpSource->m_HTTPSourceMMIAsync_CB = pfnEvtHdlr;
  pHttpSource->m_pClientData = pClientData;
  return ret;
}

/** @brief   Constructor of Http Source MMI Object.
@detail  This creates MMI interface, initializes  all ports based on the
         implementation details.
@return
@note
*/
HTTPSourceMMI::HTTPSourceMMI()
{
  uint16 i = 0;
  m_pURL = NULL;
  m_pRole = QTV_New_Args(OSCL_STRING, (COMPONENT_ROLE));
  m_pHTTPController = NULL;
  m_pHTTPSourceMMIHelper = NULL;
  m_pHTTPSourceMMIPropertiesHandler = NULL;
  m_pHTTPSourceMMIStreamPortHandler = NULL;
  m_pHTTPSourceMMITrackHandler = NULL;
  m_bHTTPStreamerInitialized = false;
  m_bIsShutdownInProgress = false;
  m_bClosePending = false;
  m_nCurrentSeekTime = -1;
  m_nPendingSeekTime = -1;
  m_bSeekPending = false;
  m_pHTTPSourceMMIDataLock = NULL;
  m_bOpenCompleted = false;
  m_pHTTPDataInterface = NULL;
  m_pHTTPDataReqHandler = NULL;
  m_nRebufCount = 0;
  m_pStopPhrase = NULL;

  m_bIsSelectRepresentationsPending = false;
  m_pCachedSelectedRepresentations = NULL;

  //initialize ports
  for (i=0; i<MMI_HTTP_NUM_VIDEO_PORTS; i++)
  {
    memset(&m_portVideo[i], 0x00, sizeof(HttpSourceMmiPortInfo));
  }

  for (i=0; i<MMI_HTTP_NUM_AUDIO_PORTS; i++)
  {
    memset(&m_portAudio[i], 0x00, sizeof(HttpSourceMmiPortInfo));
  }

  for (i=0; i<MMI_HTTP_NUM_IMAGE_PORTS; i++)
  {
    memset(&m_portImage[i], 0x00, sizeof(HttpSourceMmiPortInfo));
  }

  for (i=0; i<MMI_HTTP_NUM_OTHER_PORTS; i++)
  {
    memset(&m_portOther[i], 0x00, sizeof(HttpSourceMmiPortInfo));
  }

  m_pStopPhrase = (char*)QTV_Malloc(MAX_STOP_PHRASE_LENGTH*sizeof(char));
  if(m_pStopPhrase)
  {
    memset(m_pStopPhrase,0,MAX_STOP_PHRASE_LENGTH*sizeof(char));
  }
}

/** @brief This Destructor of Http Source MMI.
*
*@param[in]
*@param[out]
*@return
*/
HTTPSourceMMI::~HTTPSourceMMI()
{
  Destroy();
}

/** @brief Open the network connection for the specified URL (Open request
  * is posted to the HTTPController and MMI source media module is notified
  * later through the registered iResultRecipient interface.
  *
  * @param[in] pData - Client data pertaining to command
  *
  * @return open status
  */
OMX_U32 HTTPSourceMMI::Open(void * /*pData*/)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::Open" );

  OMX_U32 ret = MMI_S_EINVALSTATE;

  if (m_pURL == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Invalid/Empty input URL" );
  }
  else if (IsHTTPStreamerInitialized() && !IsClosePending())
  {
    if (m_pHTTPController->IsHTTPStreamerRunning())
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: HTTP streamer thread already active" );
    }
    else
    {
      if (InitOpen())
      {
        if (m_pHTTPController->Create())
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                        "HTTP streamer thread created" );

          //Queue the OPEN request on HTTPController
          if (m_pHTTPController->Open(m_pURL->get_cstr(),
              static_cast<iHTTPPlaybackHandler *> (m_pHTTPSourceMMIHelper),
              m_pClientData))
          {
            //OPEN request queued, notification will be sent later
            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                          "HTTP OPEN request queued" );
            ret = MMI_S_PENDING;
          }
          else
          {
            //Failed to queue OPEN request
            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                          "Error: Failed to queue HTTP OPEN request" );
          }
        }
        else
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "Error: HTTP streamer thread creation failed" );
        }
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: Open initialization failed" );
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either HTTP streamer uninitialized or "
                  "earlier Close pending" );
  }

  return ret;
}

/** @brief Close the ongoing session. Close request is posted to the
  * HTTPController and MMI source media module is notified later
  * through the registered iResultRecipient interface.
  *
  * @param[in] pResultRecipient - Reference to the callback handler for
  *                               job result notification
  *
  * @return close status
  */
OMX_U32 HTTPSourceMMI::Close(void *pData)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::Close" );

  OMX_U32 ret = MMI_S_EINVALSTATE;

  if (IsHTTPStreamerInitialized() && !IsClosePending())
  {
    if (!m_pHTTPController->IsHTTPStreamerRunning())
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: HTTP streamer thread inactive - Close returns" );
      ret = MMI_S_COMPLETE;
    }
    else
    {
      //Queue the CLOSE request on HTTPController
      //Due to possible race condition, set the flag to true before posting CLOSE
      //and unset it if needed (for failure)
      SetClosePending(true);
      m_pHTTPSourceMMIExtensionHandler.Reset();
      if(m_pHTTPDataReqHandler)
      {
        //Request handler thread exits. Instance deleted in destructor sequence.
        m_pHTTPDataReqHandler->Close();
      }
      if (m_pHTTPController->Close(pData))
      {
        //CLOSE request queued, notification will be sent later
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "HTTP CLOSE request queued" );
        ret = MMI_S_PENDING;
      }
      else
      {
        //Failed to queue CLOSE request
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: Failed to queue HTTP CLOSE request" );
        SetClosePending(false);
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either HTTP streamer uninitialized or "
                  "earlier Close pending" );
  }

  return ret;
}

/** @brief Start track setup for the HTTP session, which is a NOOP for
  * HTTP source. This is ONLY for sources that might involve server
  * interaction for track setup. However accept the job and fake success
  * later through the registered iResultRecipient interface.
  *
  * @param[in] pData - Reference to the client data of command
  *
  * @return start status
  */
OMX_U32 HTTPSourceMMI::Start(void * /*pData*/)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::Start" );

  OMX_U32 ret = MMI_S_EINVALSTATE;

  if (IsHTTPStreamerInitialized() && !IsClosePending())
  {
    if (!m_pHTTPController->IsHTTPStreamerRunning())
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: HTTP streamer thread inactive - Start returns" );
    }
    else
    {
      //Queue the START request on HTTPController
      if (m_pHTTPController->Start(m_pClientData))
      {
        //START request queued, notification will be sent later
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "HTTP START request queued" );
          ret = MMI_S_PENDING;
      }
      else
      {
        //Failed to queue START request
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: Failed to queue HTTP START request" );
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either HTTP streamer uninitialized or "
                  "earlier Close pending" );
  }

  return ret;
}

/** @brief Stop the ongoing HTTP session, which is typically done to
  * alter the track selection and restart the session with a new set
  * of tracks. This is a NOOP for HTTP source, so accept the job and fake
  * success later through the registered iResultRecipient interface.
  * Stop and Start is transparent to the File Source, and the only
  * interaction with File Source is the dynamic track switching in the
  * middle of playback which should theoretically work!!
  *
  * @param[in] pData - Reference to the client data of command
  *
  * @return stop status
  */
OMX_U32 HTTPSourceMMI::Stop(void *pData)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::Stop" );

  OMX_U32 ret = MMI_S_EINVALSTATE;
  this->m_pHTTPSourceMMIHelper->ProcessQOENotification(QOMX_HTTP_IndexParamQOEStop);
  if (IsHTTPStreamerInitialized() && !IsClosePending())
  {
    if (!m_pHTTPController->IsHTTPStreamerRunning())
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: HTTP streamer thread inactive - Stop returns" );
      //stop was issued when streamer was inactive, return immediately
      ret = MMI_S_COMPLETE;
    }
    else
    {
      //Queue the STOP request on HTTPController
      if (m_pHTTPController->Stop(pData))
      {
        //STOP request queued, notification will be sent later
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "HTTP STOP request queued" );
          ret = MMI_S_PENDING;
      }
      else
      {
        //Failed to queue STOP request
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: Failed to queue HTTP STOP request" );
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either HTTP streamer uninitialized or "
                  "earlier Close pending" );
  }

  return ret;
}

/** @brief Start/resume playback for the HTTP session, which is a NOOP for
  * HTTP source. This is ONLY for sources that might involve server
  * interaction for starting playback. However accept the job and
  * fake success later through the registered iResultRecipient interface.
  *
  * @param[in] pData - Reference to the client data of command
  *
  * @return play status
  */
OMX_U32 HTTPSourceMMI::Play(void *pData)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::Play" );

  OMX_U32 ret = MMI_S_EINVALSTATE;

  if (IsHTTPStreamerInitialized() && !IsClosePending())
  {
    if (!m_pHTTPController->IsHTTPStreamerRunning())
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: HTTP streamer thread inactive - Play returns" );
    }
    else
    {
      //Queue the PLAY request on HTTPController
      if (m_pHTTPController->Play(pData))
      {
        //PLAY request queued, notification will be sent later
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "HTTP PLAY request queued" );
        //Resume data command processing
        if(m_pHTTPDataReqHandler)
        {
          m_pHTTPDataReqHandler->Resume();
        }
        ret = MMI_S_PENDING;
      }
      else
      {
        //Failed to queue PLAY request
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: Failed to queue HTTP PLAY request" );
        ret = MMI_S_EFAIL;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either HTTP streamer uninitialized or "
                  "earlier Close pending" );
  }

  return ret;
}

/** @brief Pause playback for the HTTP session, which is a NOOP for
  * HTTP source. This is ONLY for sources that might involve server
  * interaction for pausing playback. However accept the job and fake
  * success later through the registered iResultRecipient interface.
  *
  * @param[in] pData - Reference to the client data of command
  *
  * @return pause status
  */
OMX_U32 HTTPSourceMMI::Pause(void *pData)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::Pause" );

  OMX_U32 ret = MMI_S_EINVALSTATE;

  if (IsHTTPStreamerInitialized() && !IsClosePending())
  {
    if (!m_pHTTPController->IsHTTPStreamerRunning())
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: HTTP streamer thread inactive - Pause returns" );
      //seems we got pause on idle state, so return immediately with success
      ret = MMI_S_COMPLETE;
    }
    else
    {
      //Queue the PAUSE request on HTTPController
      if (m_pHTTPController->Pause(pData))
      {
        //PAUSE request queued, notification will be sent later
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "HTTP PAUSE request queued" );
        //Pause data command processing
        if(m_pHTTPDataReqHandler)
        {
          m_pHTTPDataReqHandler->Pause();
        }
        ret = MMI_S_PENDING;
      }
      else
      {
        //Failed to queue PAUSE request
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: Failed to queue HTTP PAUSE request" );
      }
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either HTTP streamer uninitialized or "
                  "earlier Close pending" );
  }

  return ret;
}

/** @brief Seeks to the specifed position in the clip. Seek request is posted
  * to HTTPController and result of Seek operation is notifed later through the
  * registered iResultRecipient interface.
  *
  * @param[in] timeToSeek - Seek start time (in milliseconds)
  * @param[in] pResultRecipient - Reference to client data of command
  * @return seek status
  */
OMX_U32 HTTPSourceMMI::Seek
(
 const int64 timeToSeek,
 void* pResultRecipient
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::Seek" );

  OMX_U32 ret = MMI_S_EINVALSTATE;

  if (IsHTTPStreamerInitialized() && !IsClosePending())
  {
    //Check if repositioning is allowed
    HTTPDownloadStatus status = IsRepositioningAllowed(timeToSeek);
    if (status == HTTPCommon::HTTPDL_SUCCESS)
    {
      if (!m_pHTTPController->IsHTTPStreamerRunning())
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: HTTP streamer thread inactive - Seek returns" );
      }
      else
      {
        if (IsSeekPending())
        {
          m_nPendingSeekTime = timeToSeek;
          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Another SEEK %lld already in progress, %lld will be processed later",
                         m_nCurrentSeekTime, m_nPendingSeekTime );
          ret = MMI_S_COMPLETE;
        }
        else
        {
          //Queue the SEEK request on HTTPController
          if (m_pHTTPController->Seek(timeToSeek, pResultRecipient))
          {
            //SEEK request queued, notification will be sent later
            QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                           "HTTP SEEK request queued, timeToSeek %lld", timeToSeek );
            //Pause data command processing
            if(m_pHTTPDataReqHandler)
            {
              m_pHTTPDataReqHandler->Pause();
            }
            ret = MMI_S_COMPLETE;
            SetSeekPending(true);
            m_nPendingSeekTime = m_nCurrentSeekTime = timeToSeek;
            int portIdx = 0;

            //Loop through the track list and send underrun event to all the valid tracks
            for (ListPair<HTTPSourceMMITrackHandler::TrackDescription>* it = m_pHTTPSourceMMITrackHandler->m_trackList.iterator();
                 it != NULL; it = it->tail())
            {
              HTTPSourceMMITrackHandler::TrackDescription& trackDescription = it->head();

              if((trackDescription.majorType == FILE_SOURCE_MJ_TYPE_AUDIO))
              {
                portIdx = MMI_HTTP_AUDIO_PORT_INDEX;
              }
              else if ((trackDescription.majorType == FILE_SOURCE_MJ_TYPE_VIDEO))
              {
                portIdx = MMI_HTTP_VIDEO_PORT_INDEX;
              }
              else if((trackDescription.majorType == FILE_SOURCE_MJ_TYPE_TEXT))
              {
                portIdx = MMI_HTTP_OTHER_PORT_INDEX;
              }

              // Process and Notify under run Buffering events to client
              m_pHTTPSourceMMIExtensionHandler.SetHTTPBufferingStatus(portIdx,false);
            }
          }
          else
          {
            //Failed to queue SEEK request
            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                          "Error: Failed to queue HTTP SEEK request" );
            ret = MMI_S_EFAIL;
          }
        }

        //Clear EOS flag for outgoing OMX buffers if SEEK is taken
        if (ret == MMI_S_COMPLETE)
        {
          ClearEOSBufferFlag();
        }
      }
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: Reposition not allowed" );
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Either HTTP streamer uninitialized or "
                  "earlier Close pending" );
  }

  return ret;
}

/** @breif SetParam sets the omx parameters.
*
*@param[in]  :parameter - user data of SetParam command
*@param[out] :
*@return SetParam Command status
*/
OMX_U32 HTTPSourceMMI::SetParam(void* parameter)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  OMX_INDEXTYPE index = OMX_IndexComponentStartUnused;
  MMI_OmxParamCmdType *param = NULL;

  if ( NULL == parameter )
  {
    return ret;
  }

  param = (MMI_OmxParamCmdType *)parameter;
  index = param->nParamIndex;

  switch (index)
  {
    case OMX_IndexParamAudioPortFormat:
    {
      OMX_AUDIO_PARAM_PORTFORMATTYPE*  pAudFmt =
                       (OMX_AUDIO_PARAM_PORTFORMATTYPE*)param->pParamStruct;
      if (!pAudFmt || !IsValidPort(pAudFmt->nPortIndex, OMX_PortDomainAudio))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamAudioPortFormat: pAudFmt/Port index is invalid" );
        break;
      }
      ret = MMI_S_COMPLETE;
      if(pAudFmt->eEncoding == OMX_AUDIO_CodingAutoDetect)
      {
        ret = SetPortToAutoDetect(pAudFmt->nPortIndex);
      }
    }
    break;

    case OMX_IndexParamVideoPortFormat:
    {
      OMX_VIDEO_PARAM_PORTFORMATTYPE *pVidFmt =
                   (OMX_VIDEO_PARAM_PORTFORMATTYPE*)param->pParamStruct;
      if (!pVidFmt || !IsValidPort(pVidFmt->nPortIndex, OMX_PortDomainVideo))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamVideoPortFormat: pVidFmt/Port index is invalid" );
        break;
      }
      ret = MMI_S_COMPLETE;
      if (pVidFmt->eCompressionFormat == OMX_VIDEO_CodingAutoDetect)
      {
        ret = SetPortToAutoDetect(pVidFmt->nPortIndex);
      }
    }
    break;

    case OMX_IndexParamImagePortFormat:
    {
      ret = MMI_S_ENOTIMPL;
    }
    break;

    case OMX_IndexParamOtherPortFormat:
    {
      OMX_OTHER_PARAM_PORTFORMATTYPE *pOtherFmt =
                   (OMX_OTHER_PARAM_PORTFORMATTYPE*)param->pParamStruct;
      if (!pOtherFmt || !IsValidPort(pOtherFmt->nPortIndex, OMX_PortDomainOther))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamOtherPortFormat: pOtherFmt/Port index is invalid" );
        break;
      }
      ret = MMI_S_COMPLETE;
      if ((QOMX_OTHER_CODINGTYPE)pOtherFmt->eFormat == QOMX_OTHER_CodingAutoDetect)
      {
        ret = SetPortToAutoDetect(pOtherFmt->nPortIndex);
      }
    }
    break;

    case OMX_IndexParamContentURI:
    {
      OMX_PARAM_CONTENTURITYPE*  pContentUri =
                     (OMX_PARAM_CONTENTURITYPE*)param->pParamStruct;
      if (!pContentUri)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamContentURI: pContentUri is invalid" );
        break;
      }
      SetURL((const char*)pContentUri->contentURI);
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexParamStandardComponentRole:
    {
      OMX_PARAM_COMPONENTROLETYPE* pCompRole = (OMX_PARAM_COMPONENTROLETYPE*)param->pParamStruct;
      if (!pCompRole || !pCompRole->cRole || !m_pRole ||
          (sizeof(OMX_PARAM_COMPONENTROLETYPE) != pCompRole->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamStandardComponentRole: pCompRole/m_pRole/pCompRole->cRole is invalid" );
        break;
      }
      //as a component we know our role, somebody need not set our role
      //if setparam comes on role, lets just compare and return back
      if (!std_strcmp((char*)pCompRole->cRole, m_pRole->get_cstr()))
      {
        ret = MMI_S_COMPLETE;
      }
    }
    break;

    case OMX_IndexParamActiveStream:
    {
      OMX_PARAM_U32TYPE* pU32 = (OMX_PARAM_U32TYPE*)param->pParamStruct;

      if (!pU32 || !IsValidPort(pU32->nPortIndex) ||
          (sizeof(OMX_PARAM_U32TYPE) != pU32->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamActiveStream: pU32/port index is invalid" );
        break;
      }
      ret = SelectStream(pU32->nPortIndex, pU32->nU32);
    }
    break;

    case OMX_IndexConfigTimeSeekMode:
    {
      OMX_TIME_CONFIG_SEEKMODETYPE* pSeekMode =
                 (OMX_TIME_CONFIG_SEEKMODETYPE*)param->pParamStruct;

      if (!pSeekMode || (sizeof(OMX_TIME_CONFIG_SEEKMODETYPE) != pSeekMode->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexConfigTimeSeekMode: pSeekMode/port index is invalid" );
        ret = MMI_S_EBADPARAM;
        break;
      }

      m_seekMode = pSeekMode->eType;
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexConfigTimePosition:
    {
      OMX_TIME_CONFIG_TIMESTAMPTYPE* pTimestamp =
            (OMX_TIME_CONFIG_TIMESTAMPTYPE*)param->pParamStruct;

      if (!pTimestamp ||
          (sizeof(OMX_TIME_CONFIG_TIMESTAMPTYPE) != pTimestamp->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexConfigTimePosition: pTimestamp/port is invalid" );
        ret = MMI_S_EBADPARAM;
        break;
      }

      (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
      ret = Seek(pTimestamp->nTimestamp/TIME_BASE, NULL);
      (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);
    }
    break;

    default:
    {
      ret = m_pHTTPSourceMMIExtensionHandler.ProcessMMISetStdExtnParam(param);
    }
      break;
  }

  return ret;
}

/** @breif GetParam: gets the omx param.
*
*@param[in]  :parameter
*@param[out] :
*@return
*/
OMX_U32 HTTPSourceMMI::GetParam(void* parameter)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  OMX_INDEXTYPE nOmxIndex = OMX_IndexComponentStartUnused;
  MMI_OmxParamCmdType* pOmxParam = NULL;
  HttpSourceMmiPortInfo* pPortInfo = NULL;

  if ( NULL == parameter )
  {
     return ret;
  }

  pOmxParam = (MMI_OmxParamCmdType *)parameter;
  nOmxIndex = pOmxParam->nParamIndex;
  switch (nOmxIndex)
  {
    case OMX_IndexParamAudioInit:
    {
      OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE*)pOmxParam->pParamStruct;
      if(pPortParam)
      {
        pPortParam->nStartPortNumber = MMI_HTTP_AUDIO_PORT_INDEX;
        pPortParam->nPorts = MMI_HTTP_NUM_AUDIO_PORTS;
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "OMX_IndexParamAudioInit: pPortParam is invalid" );
      }
      break;
    }

    case OMX_IndexParamVideoInit:
    {
      OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE*)pOmxParam->pParamStruct;
      if(pPortParam)
      {
        pPortParam->nStartPortNumber = MMI_HTTP_VIDEO_PORT_INDEX;
        pPortParam->nPorts = MMI_HTTP_NUM_VIDEO_PORTS;
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "OMX_IndexParamAudioInit: pPortParam is invalid" );
      }
      break;
    }

    case OMX_IndexParamImageInit:
    {
      ret = MMI_S_ENOTIMPL;
      break;
    }

    case OMX_IndexParamOtherInit:
    {
      OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE*)pOmxParam->pParamStruct;
      if(pPortParam)
      {
        pPortParam->nStartPortNumber = MMI_HTTP_OTHER_PORT_INDEX;
        pPortParam->nPorts = MMI_HTTP_NUM_OTHER_PORTS;
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "OMX_IndexParamAudioInit: pPortParam is invalid" );
      }
      break;
    }


    case OMX_IndexParamAudioPortFormat:
    {
      OMX_AUDIO_PARAM_PORTFORMATTYPE* pPortFormat =
                  (OMX_AUDIO_PARAM_PORTFORMATTYPE*)pOmxParam->pParamStruct;
      if(pPortFormat)
      {
        if (!IsValidPort(pPortFormat->nPortIndex, OMX_PortDomainAudio)&&
            (sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE) != pPortFormat->nSize))
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "OMX_IndexParamAudioPortFormat: pPortFormat/port index is invalid" );
          break;
        }
        else if(IsSeekPending())
        {
          ret = MMI_S_EFAIL;
          break;
        }
        pPortInfo = &m_portAudio[ pPortFormat->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
        pPortFormat->eEncoding = pPortInfo->m_portDef.format.audio.eEncoding;
        ret = MMI_S_COMPLETE;
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "OMX_IndexParamAudioPortFormat: pPortFormat/port index is invalid" );
      }
    }
    break;

    case OMX_IndexParamAudioAac:
    {
      OMX_AUDIO_PARAM_AACPROFILETYPE *pAacFmt =
                      (OMX_AUDIO_PARAM_AACPROFILETYPE*)pOmxParam->pParamStruct;
      if (!pAacFmt || !IsValidPort(pAacFmt->nPortIndex, OMX_PortDomainAudio) ||
          (sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE) != pAacFmt->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamAudioAac: pAacFmt/port index is invalid" );
          break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }
      pPortInfo = &m_portAudio[ pAacFmt->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
      ret = MMI_S_COMPLETE;
      QOMX_STRUCT_INIT(*pAacFmt, OMX_AUDIO_PARAM_AACPROFILETYPE);
      if (pPortInfo->m_pPortSpecificData)
      {
        memcpy(pAacFmt, pPortInfo->m_pPortSpecificData, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
      }
    }
    break;

#ifdef FEATURE_HTTP_AMR
    case OMX_IndexParamAudioAmr:
    {
      ret = MMI_S_EBADPARAM;
      OMX_AUDIO_PARAM_AMRTYPE* pAmrFormat;
      pAmrFormat = (OMX_AUDIO_PARAM_AMRTYPE*)pOmxParam->pParamStruct;
      if (!pAmrFormat || !IsValidPort(pAmrFormat->nPortIndex, OMX_PortDomainAudio) ||
          (sizeof(OMX_AUDIO_PARAM_AMRTYPE) != pAmrFormat->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamAudioAmr: pAmrFormat/port index is invalid" );
        break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }
      pPortInfo = &m_portAudio[pAmrFormat->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
      if (pPortInfo && pPortInfo->m_pPortSpecificData)
      {
        memcpy(pAmrFormat, pPortInfo->m_pPortSpecificData, sizeof(OMX_AUDIO_PARAM_AMRTYPE));
      }
      else
      {
        QOMX_STRUCT_INIT(*pAmrFormat, OMX_AUDIO_PARAM_AMRTYPE);
      }
      ret = MMI_S_COMPLETE;
    }
    break;
#endif // FEATURE_HTTP_AMR

    case OMX_IndexParamAudioMp3:
    {
      ret = MMI_S_EBADPARAM;
      OMX_AUDIO_PARAM_MP3TYPE* pMp3Format;
      pMp3Format = (OMX_AUDIO_PARAM_MP3TYPE*)pOmxParam->pParamStruct;
      if (!pMp3Format || !IsValidPort(pMp3Format->nPortIndex, OMX_PortDomainAudio) ||
          (sizeof(OMX_AUDIO_PARAM_MP3TYPE) != pMp3Format->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamAudioMp3: pMp3Format/port index is invalid" );
        break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }
      pPortInfo = &m_portAudio[pMp3Format->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
      QOMX_STRUCT_INIT(*pMp3Format, OMX_AUDIO_PARAM_MP3TYPE);
      if (pPortInfo && pPortInfo->m_pPortSpecificData)
      {
        memcpy(pMp3Format, pPortInfo->m_pPortSpecificData, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
      }
      ret = MMI_S_COMPLETE;
    }
    break;

#ifdef FEATURE_HTTP_WM
    case OMX_IndexParamAudioWma:
    {
      OMX_AUDIO_PARAM_WMATYPE* pWmaFormat;
      pWmaFormat = (OMX_AUDIO_PARAM_WMATYPE*)pOmxParam->pParamStruct;
      if (!pWmaFormat || !IsValidPort(pWmaFormat->nPortIndex, OMX_PortDomainAudio))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamAudioWma: pWmaFormat/port index is invalid" );
        break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }
      pPortInfo = &m_portAudio[pWmaFormat->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];

      //Get the track info
      HTTPSourceMMITrackHandler::TrackDescription* pTrackDescription = NULL;
      if (!m_pHTTPSourceMMITrackHandler ||
          !m_pHTTPSourceMMITrackHandler->FindTrack(pPortInfo->m_trackId,
                                                   HTTPCommon::HTTP_AUDIO_TYPE,
                                                   &pTrackDescription) ||
          !pTrackDescription)
      {
        break;
      }

      if (pTrackDescription->minorType == FILE_SOURCE_MN_TYPE_WMA)
      {
        QOMX_STRUCT_INIT(*pWmaFormat, OMX_AUDIO_PARAM_WMATYPE);
        if (pPortInfo && pPortInfo->m_pPortSpecificData)
        {
          memcpy(pWmaFormat, pPortInfo->m_pPortSpecificData, sizeof(OMX_AUDIO_PARAM_WMATYPE));
        }
        ret = MMI_S_COMPLETE;
        break;
      }
      else
      {
        ret = m_pHTTPSourceMMIExtensionHandler.ProcessMMIGetStdExtnParam(pOmxParam);
      }
    }
    break;
#endif /*FEATURE_HTTP_WM*/

    case OMX_IndexParamVideoMpeg4:
    {
      OMX_VIDEO_PARAM_MPEG4TYPE* pMp4Format;
      pMp4Format = (OMX_VIDEO_PARAM_MPEG4TYPE*)pOmxParam->pParamStruct;
      if (!pMp4Format || !IsValidPort(pMp4Format->nPortIndex, OMX_PortDomainVideo) ||
          (sizeof(OMX_VIDEO_PARAM_MPEG4TYPE) != pMp4Format->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamVideoMpeg4: pMp4Format/port index is invalid" );
        break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }
      pPortInfo = &m_portVideo[pMp4Format->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
      QOMX_STRUCT_INIT(*pMp4Format, OMX_VIDEO_PARAM_MPEG4TYPE);
      if (pPortInfo && pPortInfo->m_pPortSpecificData)
      {
        memcpy(pMp4Format, pPortInfo->m_pPortSpecificData, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
      }
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexParamVideoAvc:
    {
      ret = MMI_S_EBADPARAM;
      OMX_VIDEO_PARAM_AVCTYPE* pAvcFmt;
      pAvcFmt = (OMX_VIDEO_PARAM_AVCTYPE*)pOmxParam->pParamStruct;
      if (!pAvcFmt || !IsValidPort(pAvcFmt->nPortIndex, OMX_PortDomainVideo) ||
          (sizeof(OMX_VIDEO_PARAM_AVCTYPE) != pAvcFmt->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamVideoAvc: pAvcFmt/port index is invalid" );
        break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }
      pPortInfo = &m_portVideo[pAvcFmt->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
      QOMX_STRUCT_INIT(*pAvcFmt, OMX_VIDEO_PARAM_AVCTYPE);
      if (pPortInfo && pPortInfo->m_pPortSpecificData)
      {
        memcpy(pAvcFmt, pPortInfo->m_pPortSpecificData, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
      }
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexParamVideoMpeg2:
    {
      OMX_VIDEO_PARAM_MPEG2TYPE* pMpeg2Format;
      pMpeg2Format = (OMX_VIDEO_PARAM_MPEG2TYPE*)pOmxParam->pParamStruct;
      ret = MMI_S_EBADPARAM;

      if (!pMpeg2Format || !IsValidPort(pMpeg2Format->nPortIndex, OMX_PortDomainVideo) ||
          (sizeof(OMX_VIDEO_PARAM_MPEG2TYPE) != pMpeg2Format->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamVideoMpeg2: pMpeg2Format/port index is invalid" );
        break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }
      pPortInfo = &m_portVideo[pMpeg2Format->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
      QOMX_STRUCT_INIT(*pMpeg2Format, OMX_VIDEO_PARAM_MPEG2TYPE);
      if (pPortInfo && pPortInfo->m_pPortSpecificData)
      {
        memcpy(pMpeg2Format, pPortInfo->m_pPortSpecificData, sizeof(OMX_VIDEO_PARAM_MPEG2TYPE));
      }
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexParamVideoPortFormat:
    {
      OMX_VIDEO_PARAM_PORTFORMATTYPE* pVideoPortFormat =
                   (OMX_VIDEO_PARAM_PORTFORMATTYPE*)pOmxParam->pParamStruct;
      if (!pVideoPortFormat ||
          !IsValidPort(pVideoPortFormat->nPortIndex, OMX_PortDomainVideo) ||
          (sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE) != pVideoPortFormat->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamVideoPortFormat: pVideoPortFormat/port index is invalid" );
        ret = MMI_S_EBADPARAM;
        break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }

      pPortInfo = &m_portVideo[pVideoPortFormat->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
      pVideoPortFormat->eCompressionFormat = pPortInfo->m_portDef.format.video.eCompressionFormat;
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexParamImagePortFormat:
    {
      ret = MMI_S_ENOTIMPL;
    }
    break;

    case OMX_IndexParamOtherPortFormat:
    {
      OMX_OTHER_PARAM_PORTFORMATTYPE* pOtherPortFormat =
                   (OMX_OTHER_PARAM_PORTFORMATTYPE*)pOmxParam->pParamStruct;
      if (!pOtherPortFormat ||
          !IsValidPort(pOtherPortFormat->nPortIndex, OMX_PortDomainOther) ||
          (sizeof(OMX_OTHER_PARAM_PORTFORMATTYPE) != pOtherPortFormat->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamOtherPortFormat: pOtherPortFormat/port index is invalid" );
        ret = MMI_S_EBADPARAM;
        break;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }

      pPortInfo = &m_portOther[pOtherPortFormat->nPortIndex - MMI_HTTP_OTHER_PORT_INDEX];
      pOtherPortFormat->eFormat = pPortInfo->m_portDef.format.other.eFormat;
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexParamContentURI:
    {
      OMX_PARAM_CONTENTURITYPE* pUri;
      pUri = (OMX_PARAM_CONTENTURITYPE*)pOmxParam->pParamStruct;
      if (!pUri)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamContentURI: Uri is invalid" );
        break;
      }
      pUri->contentURI[0] = '\0';
      if (pUri->nSize <= (uint32)m_pURL->size())
      {
        (void) std_strlcpy((char *)pUri->contentURI, m_pURL->get_cstr(), m_pURL->size());
      }
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexParamStandardComponentRole:
    {
      OMX_PARAM_COMPONENTROLETYPE* pCompRole = (OMX_PARAM_COMPONENTROLETYPE*)pOmxParam->pParamStruct;

      if (!pCompRole || !pCompRole->cRole || !m_pRole ||
          (sizeof(OMX_PARAM_COMPONENTROLETYPE) != pCompRole->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamStandardComponentRole: pCompRole/m_pRole/pCompRole->cRole is invalid" );
        break;
      }
      (void) std_strlcpy((char*)pCompRole->cRole, m_pRole->get_cstr(), m_pRole->size());
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexConfigTimeSeekMode:
    {
      OMX_TIME_CONFIG_SEEKMODETYPE* pSeekMode = (OMX_TIME_CONFIG_SEEKMODETYPE*)pOmxParam->pParamStruct;
      if (!pSeekMode || (sizeof(OMX_TIME_CONFIG_SEEKMODETYPE) != pSeekMode->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexConfigTimeSeekMode: pSeekMode is invalid" );
        ret = MMI_S_EBADPARAM;
        break;
      }
      pSeekMode->eType = (OMX_TIME_SEEKMODETYPE) m_seekMode;
      ret = MMI_S_COMPLETE;
    }
    break;

    case OMX_IndexConfigTimePosition:
    {
      ret = MMI_S_ENOTIMPL;
    }
    break;

    case OMX_IndexParamNumAvailableStreams:
    {
      OMX_PARAM_U32TYPE* pU32 = (OMX_PARAM_U32TYPE*)pOmxParam->pParamStruct;
      if (!pU32 || !IsValidPort(pU32->nPortIndex) || (sizeof(OMX_PARAM_U32TYPE) != pU32->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamNumAvailableStreams: port index or pU32 is invalid" );
        ret = MMI_S_EBADPARAM;
      }
      else if((IsSeekPending()) || (!IsOpenComplete()))
      {
        pU32->nU32 = 0;
        ret = MMI_S_EFAIL;
      }
      else
      {
        pU32->nU32 = m_pHTTPSourceMMITrackHandler->GetNumberOfTracks(pU32->nPortIndex);
        ret = MMI_S_COMPLETE;
      }
    }
    break;

    case OMX_IndexParamActiveStream:
    {
      OMX_PARAM_U32TYPE* pU32 = (OMX_PARAM_U32TYPE*)pOmxParam->pParamStruct;
      if (!pU32 || !IsValidPort(pU32->nPortIndex) || (sizeof(OMX_PARAM_U32TYPE) != pU32->nSize))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "OMX_IndexParamActiveStream: port index or pU32 is invalid" );
        ret = MMI_S_EBADPARAM;
      }
      else
      {
        pU32->nU32 = 0;
        ret = MMI_S_COMPLETE;
      }
    }
    break;

    default:
    {
      ret = m_pHTTPSourceMMIExtensionHandler.ProcessMMIGetStdExtnParam(pOmxParam);
    }
    break;
  }

  return ret;
}
/* Compares the port definition to be set with the
 * existing port definition
 * return MMI_S_COMPLETE if port definiton matches
 */
OMX_U32 HTTPSourceMMI::ComparePortDefinition(MMI_ParamDomainDefType *pDomainDef)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  if(pDomainDef != NULL)
  {
    HttpSourceMmiPortInfo* pPortInfo = NULL;
    if(pDomainDef->nPortIndex == MMI_HTTP_VIDEO_PORT_INDEX)
    {
      OMX_VIDEO_PORTDEFINITIONTYPE pVideoPortDef,pVideoPortDef_old;
      pPortInfo = &m_portVideo[pDomainDef->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
      pVideoPortDef_old = (OMX_VIDEO_PORTDEFINITIONTYPE)pPortInfo->m_portDef.format.video;
      pVideoPortDef = (OMX_VIDEO_PORTDEFINITIONTYPE)pDomainDef->format.video;
      if(pVideoPortDef.eCompressionFormat == pVideoPortDef_old.eCompressionFormat  &&
        pVideoPortDef.eColorFormat == pVideoPortDef_old.eColorFormat &&
        pVideoPortDef.nBitrate == pVideoPortDef_old.nBitrate &&
        pVideoPortDef.xFramerate == pVideoPortDef_old.xFramerate)
      {
        ret = MMI_S_COMPLETE;
      }
    }
  }
  return ret;
}
/**@brief SetCustomParam
*@param[in] parameter - command data
*@param[out]
*@return success status
*/
OMX_U32 HTTPSourceMMI::SetCustomParam(void* parameter)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  MMI_IndexType nMMIIndex = MMI_IndexInvalid;
  MMI_CustomParamCmdType* pCustParam;
  HttpSourceMmiPortInfo* pPortInfo = NULL;

  if ( NULL == parameter )
  {
    return ret;
  }

  pCustParam = (MMI_CustomParamCmdType*)parameter;
  nMMIIndex = pCustParam->nParamIndex;

  switch (nMMIIndex)
  {
    case MMI_IndexDomainDef:
    {
      MMI_ParamDomainDefType* pDomainDef = (MMI_ParamDomainDefType*)pCustParam->pParamStruct;
      if (!pDomainDef || !IsValidPort(pDomainDef->nPortIndex))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "SetCustomParam: port index or pDomainDef is invalid" );
        ret = MMI_S_EBADPARAM;
        break;
      }

      switch ( pDomainDef->nPortIndex )
      {
        case MMI_HTTP_AUDIO_PORT_INDEX:
        {
          switch ((uint32)pDomainDef->format.audio.eEncoding)
          {
#ifdef FEATURE_HTTP_AMR
            case OMX_AUDIO_CodingAMR:
#endif // FEATURE_HTTP_AMR
            case OMX_AUDIO_CodingAAC:
            case OMX_AUDIO_CodingMP3:
            case (OMX_AUDIO_CODINGTYPE) QOMX_EXT_AUDIO_CodingAC3:
            case (OMX_AUDIO_CODINGTYPE) QOMX_EXT_AUDIO_CodingMP2:
#ifdef FEATURE_HTTP_WM
            case OMX_AUDIO_CodingWMA:
#endif
            case OMX_AUDIO_CodingEVRC:
            {
              ret = MMI_S_EBADPARAM;
            }
            break;
            case OMX_AUDIO_CodingAutoDetect:
            {
              ret = SetPortToAutoDetect(pDomainDef->nPortIndex);
            }
            break;

            default:
            {
              ret = MMI_S_ENOTIMPL;
            }
            break;
          }
        }
        break;

        case MMI_HTTP_VIDEO_PORT_INDEX:
        {
          switch ( pDomainDef->format.video.eCompressionFormat)
          {
            case OMX_VIDEO_CodingH263:
            case OMX_VIDEO_CodingMPEG4:
#ifdef FEATURE_HTTP_WM
            case OMX_VIDEO_CodingWMV:
#endif
            case OMX_VIDEO_CodingRV:
            case OMX_VIDEO_CodingAVC:
            case OMX_VIDEO_CodingMPEG2:
            {
              ret=ComparePortDefinition(pDomainDef);
            }
            break;

            case OMX_VIDEO_CodingAutoDetect:
            {
              ret = SetPortToAutoDetect(pDomainDef->nPortIndex);
            }
            break;

            default:
            {
              ret = MMI_S_ENOTIMPL;
            }
            break;
          }
        }
        break;

        case MMI_HTTP_IMAGE_PORT_INDEX:
        {
          ret = MMI_S_ENOTIMPL;
        }
        break;

        case MMI_HTTP_OTHER_PORT_INDEX:
        {
          if (pDomainDef->format.other.eFormat == OMX_OTHER_FormatTime)
          {
          }
          else if (pDomainDef->format.other.eFormat == (OMX_OTHER_FORMATTYPE)QOMX_OTHER_CodingSMPTETT)
          {
            ret = MMI_S_EBADPARAM;
          }
          else if (pDomainDef->format.other.eFormat == (OMX_OTHER_FORMATTYPE)QOMX_OTHER_CodingAutoDetect)
          {
            ret = SetPortToAutoDetect(pDomainDef->nPortIndex);
          }
          else
          {
            ret = MMI_S_ENOTIMPL;
          }
        }
        break;

        default:
        {
          ret = MMI_S_ENOTIMPL;
        }
        break;
      }
    }
    break;

    case MMI_IndexBuffersReq:
    {
      MMI_ParamBuffersReqType*  pParamBuffReq = (MMI_ParamBuffersReqType*)pCustParam->pParamStruct;
      if (!pParamBuffReq || !IsValidPort(pParamBuffReq->nPortIndex))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "SetCustomParam: port index or pParamBuffReq is invalid" );
        ret = MMI_S_EBADPARAM;
        break;
      }

      switch( pParamBuffReq->nPortIndex)
      {
        case MMI_HTTP_AUDIO_PORT_INDEX:
        {
          if(pParamBuffReq->nCount < pParamBuffReq->nMinCount)
          {
            ret = MMI_S_EBADPARAM;
          }
          else
          {
            pPortInfo = &m_portAudio[pParamBuffReq->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
            pPortInfo->m_portDef.nBufferCountActual = pParamBuffReq->nCount;
            ret = MMI_S_COMPLETE;
          }
        }
        break;

        case MMI_HTTP_VIDEO_PORT_INDEX:
        {
          if(pParamBuffReq->nCount < pParamBuffReq->nMinCount)
          {
            ret = MMI_S_EBADPARAM;
          }
          else
          {
            pPortInfo = &m_portVideo[pParamBuffReq->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
            pPortInfo->m_portDef.nBufferCountActual = pParamBuffReq->nCount;
            ret = MMI_S_COMPLETE;
          }
        }
        break;

        case MMI_HTTP_IMAGE_PORT_INDEX:
        {
          ret = MMI_S_ENOTIMPL;
        }
        break;

        case MMI_HTTP_OTHER_PORT_INDEX:
        {
          if(pParamBuffReq->nCount < pParamBuffReq->nMinCount)
          {
            ret = MMI_S_EBADPARAM;
          }
          else
          {
            pPortInfo = &m_portOther[pParamBuffReq->nPortIndex - MMI_HTTP_OTHER_PORT_INDEX];
            pPortInfo->m_portDef.nBufferCountActual = pParamBuffReq->nCount;
            ret = MMI_S_COMPLETE;
          }
        }
        break;

        default:
        {
          ret = MMI_S_ENOTIMPL;
        }
        break;
      }
    }
    break;

    default:
    {
      ret = MMI_S_ENOTIMPL;
    }
    break;
  }
  return ret;
}

/**@brief GetCustomParam
*@param[in] parameter - command data
*@param[out]
*@return success status
*/
OMX_U32 HTTPSourceMMI::GetCustomParam(void* parameter)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  MMI_IndexType nMMIIndex;
  MMI_CustomParamCmdType* pCustParam;
  HttpSourceMmiPortInfo* pPortInfo = NULL;

  if ( NULL == parameter )
  {
    return ret;
  }

  pCustParam = (MMI_CustomParamCmdType*)parameter;
  nMMIIndex = pCustParam->nParamIndex;
  switch (nMMIIndex)
  {
    case MMI_IndexDomainDef:
    {
      MMI_ParamDomainDefType*  pDomainDef = (MMI_ParamDomainDefType*)pCustParam->pParamStruct;
      if (!pDomainDef || !IsValidPort(pDomainDef->nPortIndex))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "GetCustomParam: port index or pDomainDef is invalid" );
        return MMI_S_EBADPARAM;
      }
      else if(IsSeekPending())
      {
        ret = MMI_S_EFAIL;
        break;
      }

      switch (pDomainDef->nPortIndex)
      {
        case MMI_HTTP_AUDIO_PORT_INDEX:
        {
          OMX_AUDIO_PORTDEFINITIONTYPE* pAudioPortDef;
          pAudioPortDef = (OMX_AUDIO_PORTDEFINITIONTYPE*)&pDomainDef->format.audio;
          pPortInfo = &m_portAudio[ pDomainDef->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
          // If no track available
          pAudioPortDef->cMIMEType = '\0';
          pAudioPortDef->pNativeRender = NULL;
          pAudioPortDef->bFlagErrorConcealment = pPortInfo->m_portDef.format.audio.bFlagErrorConcealment;
          pAudioPortDef->eEncoding = pPortInfo->m_portDef.format.audio.eEncoding;
          ret = MMI_S_COMPLETE;
        }
        break;

        case MMI_HTTP_VIDEO_PORT_INDEX:
        {
          OMX_VIDEO_PORTDEFINITIONTYPE* pVideoPortDef;
          pVideoPortDef = (OMX_VIDEO_PORTDEFINITIONTYPE*)&pDomainDef->format.video;
          pPortInfo = &m_portVideo[ pDomainDef->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
          //Default value stored in file demux context
          pVideoPortDef->cMIMEType = '\0';
          pVideoPortDef->pNativeRender = NULL;
          pVideoPortDef->bFlagErrorConcealment = OMX_FALSE;
          pVideoPortDef->eColorFormat = pPortInfo->m_portDef.format.video.eColorFormat;
          pVideoPortDef->pNativeWindow = NULL;
          pVideoPortDef->nFrameWidth = pPortInfo->m_portDef.format.video.nFrameWidth;
          pVideoPortDef->nFrameHeight = pPortInfo->m_portDef.format.video.nFrameHeight;
          pVideoPortDef->nBitrate = pPortInfo->m_portDef.format.video.nBitrate;
          pVideoPortDef->xFramerate = pPortInfo->m_portDef.format.video.xFramerate;
          pVideoPortDef->eCompressionFormat = pPortInfo->m_portDef.format.video.eCompressionFormat;
          ret = MMI_S_COMPLETE;
        }
        break;

        case MMI_HTTP_IMAGE_PORT_INDEX:
        {
          ret = MMI_S_EBADPARAM;
        }
        break;

        case MMI_HTTP_OTHER_PORT_INDEX:
        {
          OMX_OTHER_PORTDEFINITIONTYPE* pOtherPortDef;
          pOtherPortDef = (OMX_OTHER_PORTDEFINITIONTYPE*)&pDomainDef->format.other;
          pPortInfo = &m_portOther[pDomainDef->nPortIndex - MMI_HTTP_OTHER_PORT_INDEX];
          pOtherPortDef->eFormat = pPortInfo->m_portDef.format.other.eFormat;
          ret = MMI_S_COMPLETE;
        }
        break;

        default:
        {
          ret = MMI_S_EBADPARAM;
        }
        break;
      }
    }
    break;

    case MMI_IndexBuffersReq:
    {
      MMI_ParamBuffersReqType*  pParamBuffReq = (MMI_ParamBuffersReqType*)pCustParam->pParamStruct;
      if (!pParamBuffReq || !IsValidPort(pParamBuffReq->nPortIndex))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "GetCustomParam: port index or pParamBuffReq is invalid" );
        ret = MMI_S_EBADPARAM;
        break;
      }

      switch ( pParamBuffReq->nPortIndex )
      {
        case MMI_HTTP_AUDIO_PORT_INDEX:
        {
          pPortInfo = &m_portAudio[ pParamBuffReq->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
          pParamBuffReq->nMinCount = pPortInfo->m_portDef.nBufferCountMin;
          pParamBuffReq->nCount = pPortInfo->m_portDef.nBufferCountActual;
          pParamBuffReq->nAlignment = pPortInfo->m_portDef.nBufferAlignment;
          pParamBuffReq->bBuffersContiguous = pPortInfo->m_portDef.bBuffersContiguous;
          pParamBuffReq->nSuffixSize = 0;
          pParamBuffReq->nBufferPoolId = 0;
          pParamBuffReq->nDataSize = pPortInfo->m_portDef.nBufferSize;
          ret = MMI_S_COMPLETE;
        }
        break;

        case MMI_HTTP_VIDEO_PORT_INDEX:
        {
          pPortInfo = &m_portVideo[ pParamBuffReq->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
          pParamBuffReq->nMinCount = pPortInfo->m_portDef.nBufferCountMin;
          pParamBuffReq->nCount = pPortInfo->m_portDef.nBufferCountActual;
          pParamBuffReq->nAlignment = pPortInfo->m_portDef.nBufferAlignment;
          pParamBuffReq->bBuffersContiguous = pPortInfo->m_portDef.bBuffersContiguous;
          pParamBuffReq->nSuffixSize = 0;
          pParamBuffReq->nBufferPoolId = 0;
          pParamBuffReq->nDataSize = pPortInfo->m_portDef.nBufferSize;
          QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "GetCustomParam: Video Port Buffer Size %u",(uint32)pParamBuffReq->nDataSize );
          ret = MMI_S_COMPLETE;
        }
        break;

        case MMI_HTTP_IMAGE_PORT_INDEX:
        {
          ret = MMI_S_ENOTIMPL;
        }
        break;

        case MMI_HTTP_OTHER_PORT_INDEX:
        {
           pPortInfo = &m_portOther[ pParamBuffReq->nPortIndex - MMI_HTTP_OTHER_PORT_INDEX];
           pParamBuffReq->nMinCount = pPortInfo->m_portDef.nBufferCountMin;
           pParamBuffReq->nCount = pPortInfo->m_portDef.nBufferCountActual;
           pParamBuffReq->nAlignment = pPortInfo->m_portDef.nBufferAlignment;
           pParamBuffReq->bBuffersContiguous = pPortInfo->m_portDef.bBuffersContiguous;
           pParamBuffReq->nSuffixSize = 0;
           pParamBuffReq->nBufferPoolId = 0;
           pParamBuffReq->nDataSize = pPortInfo->m_portDef.nBufferSize;
           ret = MMI_S_COMPLETE;
        }
        break;

        default:
        {
          ret = MMI_S_ENOTIMPL;
        }
        break;
      }
    }
    break;

    default:
    {
      ret = MMI_S_EBADPARAM;
    }
    break;
  }
  return ret;
}


/**@brief Allocate Buffer
*@param[in] parameter - command data
*@param[out]
*@return success status
*/
OMX_U32 HTTPSourceMMI::AllocBuffer(void* parameter)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  HttpSourceMmiPortInfo* pPortInfo = NULL;

  if ( NULL == parameter )
  {
    return ret;
  }

  MMI_AllocBufferCmdType* pAllocBufferCmd = (MMI_AllocBufferCmdType*)parameter;
  switch( pAllocBufferCmd->nPortIndex)
  {
    case MMI_HTTP_AUDIO_PORT_INDEX:
    {
      if (IsValidPort(pAllocBufferCmd->nPortIndex, OMX_PortDomainAudio))
      {
        pPortInfo = &m_portAudio[pAllocBufferCmd->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
        OMX_U32 nSize = pPortInfo->m_portDef.nBufferSize;
        if(nSize == pAllocBufferCmd->nSize)
        {
          pAllocBufferCmd->pBuffer = (OMX_U8*)QTV_Malloc(nSize);
          if(pAllocBufferCmd->pBuffer)
          {
            memset(pAllocBufferCmd->pBuffer,0,nSize);
            ret = MMI_S_COMPLETE;
          }
        }
      }
    }
    break;

    case MMI_HTTP_VIDEO_PORT_INDEX:
    {
      if (IsValidPort(pAllocBufferCmd->nPortIndex, OMX_PortDomainVideo))
      {
        pPortInfo = &m_portVideo[pAllocBufferCmd->nPortIndex - MMI_HTTP_VIDEO_PORT_INDEX];
        OMX_U32 nSize = pPortInfo->m_portDef.nBufferSize;
        ret = MMI_S_EBADPARAM;
        if(nSize == pAllocBufferCmd->nSize)
        {
          pAllocBufferCmd->pBuffer = (OMX_U8*)QTV_Malloc(nSize);
          if(pAllocBufferCmd->pBuffer)
          {
            memset(pAllocBufferCmd->pBuffer,0,nSize);
            ret = MMI_S_COMPLETE;
          }
        }
      }
    }
    break;

    case MMI_HTTP_IMAGE_PORT_INDEX:
    {
      ret = MMI_S_ENOTIMPL;
    }
    break;

    case MMI_HTTP_OTHER_PORT_INDEX:
    {
      if (IsValidPort(pAllocBufferCmd->nPortIndex, OMX_PortDomainOther))
      {
        pPortInfo = &m_portOther[pAllocBufferCmd->nPortIndex - MMI_HTTP_OTHER_PORT_INDEX];
        OMX_U32 nSize = pPortInfo->m_portDef.nBufferSize;
        ret = MMI_S_EBADPARAM;
        if(nSize == pAllocBufferCmd->nSize)
        {
          pAllocBufferCmd->pBuffer = (OMX_U8*)QTV_Malloc(nSize);
          if(pAllocBufferCmd->pBuffer)
          {
            memset(pAllocBufferCmd->pBuffer,0,nSize);
            ret = MMI_S_COMPLETE;
          }
        }
      }
    }
    break;

    default:
    {
      ret = MMI_S_EBADPARAM;
    }
    break;
  }
  return ret;
}

/**@brief Free Buffer
*@param[in] parameter - command data
*@param[out]
*@return success status
*/
OMX_U32 HTTPSourceMMI::FreeBuffer(void* parameter)
{
  OMX_U32 ret = MMI_S_EBADPARAM;

  if (NULL == parameter)
  {
    return ret;
  }

  MMI_FreeBufferCmdType *pFreeBufferCmd = (MMI_FreeBufferCmdType*)parameter;
  switch ( pFreeBufferCmd->nPortIndex)
  {
    case MMI_HTTP_AUDIO_PORT_INDEX:
    {
      if (IsValidPort(pFreeBufferCmd->nPortIndex, OMX_PortDomainAudio))
      {
        if(pFreeBufferCmd->pBuffer)
        {
          QTV_Free(pFreeBufferCmd->pBuffer);
          ret = MMI_S_COMPLETE;
        }
      }
    }
    break;

    case MMI_HTTP_VIDEO_PORT_INDEX:
    {
      if (IsValidPort(pFreeBufferCmd->nPortIndex, OMX_PortDomainVideo))
      {
        if(pFreeBufferCmd->pBuffer)
        {
          QTV_Free(pFreeBufferCmd->pBuffer);
          ret = MMI_S_COMPLETE;
        }
      }
    }
    break;

    case MMI_HTTP_IMAGE_PORT_INDEX:
    {
      ret = MMI_S_ENOTIMPL;
    }
    break;

    case MMI_HTTP_OTHER_PORT_INDEX:
    {
      if (IsValidPort(pFreeBufferCmd->nPortIndex, OMX_PortDomainOther))
      {
        if(pFreeBufferCmd->pBuffer)
        {
          QTV_Free(pFreeBufferCmd->pBuffer);
          ret = MMI_S_COMPLETE;
        }
      }
    }
    break;

    default:
    {
      ret = MMI_S_EBADPARAM;
    }
    break;
  }

  return ret;
}

bool HTTPSourceMMI::StartDataRequestProcessing()
{
  bool bOk = false;

  if(m_pHTTPDataReqHandler)
  {
    QTV_Delete(m_pHTTPDataReqHandler);
    m_pHTTPDataReqHandler = NULL;
  }

  m_pHTTPDataReqHandler = QTV_New_Args(HTTPDataRequestHandler, (m_pHTTPSourceMMIHelper,
                                                  bOk));
  return bOk;
}
/**@brief Gets a sample synchromously if available
          otherwise gives it asynchornously later
*@param[in] parameter - command data
*@param[out]
*@return success status
*/
OMX_U32 HTTPSourceMMI::FillThisBuffer(void* parameter)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  if ( NULL == parameter )
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "FillThisBuffer Error: Param NULL" );

  }
  else
  {
    MMI_BufferCmdType* pBuffCmd = (MMI_BufferCmdType*)parameter;
    OMX_BUFFERHEADERTYPE *pBuffHdr = (OMX_BUFFERHEADERTYPE*)pBuffCmd->pBufferHdr;
    if (!pBuffHdr || (pBuffHdr->nSize != sizeof(OMX_BUFFERHEADERTYPE)) ||
        (!IsValidPort(pBuffCmd->nPortIndex)))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "FillThisBuffer: port/buffer size/buffer ptr is invalid" );
    }
    else
    {
      ret = MMI_S_EFAIL;
      OMX_U32 portIdx=pBuffHdr->nOutputPortIndex;

      if(m_pHTTPDataReqHandler && m_pHTTPDataReqHandler->DataRequest(portIdx, (void *)pBuffHdr))
      {
        ret = MMI_S_PENDING;
      }
      QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "HTTPMMIFillThisBuffer for port %u pBufferHdr 0x%p pBuffer 0x%p ret %d",
                     (uint32)portIdx,
                    (void *)pBuffHdr,
                    (void *)(pBuffHdr->pBuffer),
                    (int)ret);
    }
  }
  return ret;
}
/**@brief Gets a sample for the port
*@param[in] portidx - port id
*@param[in] pbuffHdr - buffer header
*@param[out]
*@return success status
*/

OMX_U32 HTTPSourceMMI::GetSample(int32 portIdx, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
  OMX_U32 ret = MMI_S_PENDING;
  HttpSourceMmiPortInfo* pPortInfo = NULL;
  int32 offset = 0;
  bool bSwitch = false;
  int32 targetIndex = 0;
  int32 trackId=0;
  HTTPMediaType majorType = HTTPCommon::HTTP_UNKNOWN_TYPE;
  if(IsSeekPending())
  {
    return MMI_S_PENDING;
  }
  else if(pBuffHdr->nOutputPortIndex == MMI_HTTP_AUDIO_PORT_INDEX)
  {
    targetIndex = pBuffHdr->nOutputPortIndex - MMI_HTTP_AUDIO_PORT_INDEX;
    pPortInfo = &m_portAudio[targetIndex];
    trackId=  pPortInfo->m_trackId;
    pBuffHdr->nFlags = 0;
    majorType = HTTPCommon::HTTP_AUDIO_TYPE;
  }
  else if(pBuffHdr->nOutputPortIndex == MMI_HTTP_VIDEO_PORT_INDEX)
  {
    targetIndex = pBuffHdr->nOutputPortIndex - MMI_HTTP_VIDEO_PORT_INDEX;
    pPortInfo = &m_portVideo[targetIndex];
    trackId=  pPortInfo->m_trackId;
    pBuffHdr->nFlags = 0;
    majorType = HTTPCommon::HTTP_VIDEO_TYPE;
  }
  else if(pBuffHdr->nOutputPortIndex == MMI_HTTP_OTHER_PORT_INDEX)
  {
    targetIndex = pBuffHdr->nOutputPortIndex - MMI_HTTP_OTHER_PORT_INDEX;
    pPortInfo = &m_portOther[targetIndex];
    trackId=  pPortInfo->m_trackId;
    pBuffHdr->nFlags = 0;
    majorType = HTTPCommon::HTTP_TEXT_TYPE;
  }
  else
  {
    return MMI_S_ENOTIMPL;
  }
  bool bPortSwitching = IsPortSwitching(pPortInfo);
  if (!bPortSwitching && !IsEndOfStream(portIdx))
  {
    ret = m_pHTTPSourceMMITrackHandler->Read(trackId, majorType, 0, pBuffHdr,
                                             offset, bPortSwitching);
  }
  else if(IsEndOfStream(portIdx))
  {
    ret = MMI_S_COMPLETE;
  }

  if(ret==MMI_S_COMPLETE)
  {
   // Process and Notify Buffering events to client
    m_pHTTPSourceMMIExtensionHandler.SetHTTPBufferingStatus(portIdx,true);

    pBuffHdr->nFlags |= pPortInfo->m_nBufferFlags;
    if(pPortInfo->m_nBufferFlags & OMX_BUFFERFLAG_STARTTIME)
    {
      //clear Start Time flag
      pPortInfo->m_nBufferFlags &= ~OMX_BUFFERFLAG_STARTTIME;
    }
    if (pBuffHdr->nFlags & OMX_BUFFERFLAG_EOS)
    {
      //set eos flag on port
      pPortInfo->m_nBufferFlags |= OMX_BUFFERFLAG_EOS;
      //Check if we need to set EOS flag on other ports
      //Set EOS flag on all the other ports which are uninitialized
      HttpSourceMmiPortInfo *pOtherPort = NULL;
      for(int i = 0;i<MMI_HTTP_NUM_VIDEO_PORTS;i++)
      {
        pOtherPort = &m_portVideo[i];
        if(pOtherPort && pOtherPort->m_portDef.format.video.eCompressionFormat == OMX_VIDEO_CodingUnused)
        {
          pOtherPort->m_nBufferFlags |= OMX_BUFFERFLAG_EOS;
        }
      }
      for(int i = 0;i<MMI_HTTP_NUM_AUDIO_PORTS;i++)
      {
        pOtherPort = &m_portAudio[i];
        if(pOtherPort && pOtherPort->m_portDef.format.audio.eEncoding == OMX_AUDIO_CodingUnused)
        {
          pOtherPort->m_nBufferFlags |= OMX_BUFFERFLAG_EOS;
        }
      }
      for(int i = 0;i<MMI_HTTP_NUM_IMAGE_PORTS;i++)
      {
        pOtherPort = &m_portImage[i];
        if(pOtherPort && pOtherPort->m_portDef.format.image.eCompressionFormat == OMX_IMAGE_CodingUnused)
        {
          pOtherPort->m_nBufferFlags |= OMX_BUFFERFLAG_EOS;
        }
      }
    }
  }
  else if(ret==MMI_S_PENDING)
  {
    if (bPortSwitching)
    {
      bool isPortConfigPending = false;
      //Handle switch for the media type (compare codec info and update port info for now)
      uint32 nCurrNumTracks = m_pHTTPSourceMMITrackHandler->m_trackList.size();
      HTTPDownloadStatus eStatus = HandleSwitch(majorType, isPortConfigPending);
      CheckAndSendPortConfigChangeEvent(majorType, 0, isPortConfigPending);
      if (eStatus == HTTPCommon::HTTPDL_SUCCESS)
      {
        //Check if new tracks are added, if so notify player of the same so that
        //the new tracks are set up and reads will begin
        HTTPMediaTrackInfo* pTrackInfo = NULL;
        uint32 nNewNumTracks = 0;
        if (m_pHTTPDataInterface)
        {
          nNewNumTracks = m_pHTTPDataInterface->GetMediaTrackInfo(pTrackInfo);
        }

        QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                       "Switched streams on track %d mediaType %d, %d/%d tracks",
                       trackId, majorType, nCurrNumTracks, nNewNumTracks );

        // Removing this check, as this piece of code must be executed even when
        // the old and new number of track are same but the new track type added is different.
        // Example is transition from Period1(Audio only) to Period2(Video only).
        // TODO: Cleanup the nNewNumTracks/nCurrNumTracks code.
        {
          pTrackInfo = (HTTPMediaTrackInfo *) (QTV_Malloc(nNewNumTracks * sizeof(HTTPMediaTrackInfo)));
          if (pTrackInfo && m_pHTTPDataInterface)
          {
            nNewNumTracks = m_pHTTPDataInterface->GetMediaTrackInfo(pTrackInfo);
            for (uint32 index = 0; index < nNewNumTracks; index++)
            {
              if (pTrackInfo + index == NULL)
              {
                QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                               "Error: Track info for track %u is NULL", index );
                continue;
              }
              else if ( pTrackInfo[index].bSelected )
              {
                if ( pTrackInfo[index].majorType != majorType )
                {
                  isPortConfigPending = false;
                  eStatus = HandleSwitch(pTrackInfo[index].majorType, isPortConfigPending);
                  CheckAndSendPortConfigChangeEvent(pTrackInfo[index].majorType, 0, isPortConfigPending);
                  if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
                  {
                    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                                   "Switch failed on track %u mediaType %d",
                                   pTrackInfo[index].nTrackID, pTrackInfo[index].majorType );
                    break;
                  }
                }
              }
            }
          }
          if(pTrackInfo)
          {
            QTV_Free(pTrackInfo);
            pTrackInfo = NULL;
          }
        }
      }

      if (eStatus == HTTPCommon::HTTPDL_SUCCESS)
      {
        SetPortSwitchingFlag(pPortInfo,false);
      }
      else if (eStatus == HTTPCommon::HTTPDL_WAITING)
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                       "Switching streams on track %d, waiting!", trackId );
      }
      else
      {
        SetPortSwitchingFlag(pPortInfo,false);
        ret = MMI_S_EFATAL;
        if (m_pHTTPSourceMMIHelper)
        {
          MMI_ResourceLostMsgType resourceLostMsg;
          resourceLostMsg.bSuspendable = OMX_FALSE;
          UpdateStopPhrase((char*)STOP_ERROR_STRING);
          m_pHTTPSourceMMIHelper->NotifyMmi(MMI_EVT_RESOURCES_LOST,
                                            MMI_S_EFAIL,
                                            sizeof(MMI_ResourceLostMsgType),
                                            (void *)&resourceLostMsg, m_pClientData);

          QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Switch stream failed %d on track %d, aborting!",
                         eStatus, trackId );
        }
      }
    }

    HTTPSourceMMITrackHandler::TrackDescription* pTrackDescription = NULL;
    if (m_pHTTPSourceMMITrackHandler->GetTrackPlayState(trackId, majorType ) ==
        HTTPSourceMMITrackHandler::PLAYING)
    {
      // suppress the watermark event
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                    "Suppress watermark for track %d", trackId);
    }
    // Send buffering status only if no representation/period switching
    else if((!bPortSwitching) && (m_pHTTPSourceMMITrackHandler->FindTrack(trackId, majorType, &pTrackDescription)))
    {
      // Process and Notify under run Buffering events to client
      m_pHTTPSourceMMIExtensionHandler.SetHTTPBufferingStatus(portIdx,false);
    }
  }
  else if(MMI_S_EFATAL == ret)
  {
    if (m_pHTTPSourceMMIHelper)
    {
      MMI_ResourceLostMsgType resourceLostMsg;
      resourceLostMsg.bSuspendable = OMX_FALSE;
      UpdateStopPhrase((char*)STOP_ERROR_STRING);
      m_pHTTPSourceMMIHelper->NotifyMmi(MMI_EVT_RESOURCES_LOST,
                                        MMI_S_EFAIL,
                                        sizeof(MMI_ResourceLostMsgType),
                                        (void *)&resourceLostMsg, m_pClientData);

      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "MMI_EVT_RESOURCES_LOST due to Download failure detected in Read path, aborting!");
    }
  }
  return ret;
}
/**@brief Flushes all ports and send buffer done notifications to mmi
*@param[in] parameter - command data
*@param[out]
*@return success status
*/
OMX_U32 HTTPSourceMMI::Flush(void* pData)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  int32 portIdx = -1;

  if (NULL != pData)
  {
    portIdx = ((MMI_PortCmdType *)pData)->nPortIndex;
  }

  if (m_pHTTPDataReqHandler && ((portIdx == -1) || IsValidPort(portIdx)))
  {
    ret = MMI_S_PENDING;
    m_pHTTPDataReqHandler->DataFlush(portIdx);
  }

  return ret;
}


/**@brief Sets a port to auto detect mode
*@param[in] portIdx - port to be set in auto detect
*@param[out]
*@return success status
*/
OMX_U32 HTTPSourceMMI::SetPortToAutoDetect(OMX_U32 portIdx)
{
  OMX_U32 ret = MMI_S_EBADPARAM;
  HttpSourceMmiPortInfo* pPortInfo = NULL;

  if(portIdx == MMI_HTTP_VIDEO_PORT_INDEX)
  {
    pPortInfo = &m_portVideo[portIdx - MMI_HTTP_VIDEO_PORT_INDEX];
    pPortInfo->m_portDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAutoDetect;
    pPortInfo->m_portDef.nBufferCountMin = MMI_HTTP_VIDEO_PORT_MIN_BUFFER_COUNT;
    pPortInfo->m_portDef.nBufferCountActual = pPortInfo->m_portDef.nBufferCountMin;
    ret = MMI_S_COMPLETE;
  }
  else if(portIdx == MMI_HTTP_AUDIO_PORT_INDEX)
  {
    pPortInfo = &m_portAudio[portIdx - MMI_HTTP_AUDIO_PORT_INDEX];
    pPortInfo->m_portDef.format.audio.eEncoding = OMX_AUDIO_CodingAutoDetect;
    pPortInfo->m_portDef.nBufferCountMin = MMI_HTTP_AUDIO_PORT_MIN_BUFFER_COUNT;
    pPortInfo->m_portDef.nBufferCountActual = pPortInfo->m_portDef.nBufferCountMin;
    ret = MMI_S_COMPLETE;
  }
  else if(portIdx == MMI_HTTP_OTHER_PORT_INDEX)
  {
    pPortInfo = &m_portOther[portIdx - MMI_HTTP_OTHER_PORT_INDEX];
    pPortInfo->m_portDef.format.other.eFormat = (OMX_OTHER_FORMATTYPE)QOMX_OTHER_CodingAutoDetect;
    pPortInfo->m_portDef.nBufferCountMin = MMI_HTTP_OTHER_PORT_MIN_BUFFER_COUNT;
    pPortInfo->m_portDef.nBufferCountActual = pPortInfo->m_portDef.nBufferCountMin;
    ret = MMI_S_COMPLETE;
  }

  return ret;
}

/**@brief Selects a stream from tracklist
*@param[in] portIdx - port index
            streamNo - stream number
*@param[out]
*@return status of select stream
*/
OMX_U32 HTTPSourceMMI::SelectStream (OMX_U32 portIdx, OMX_U32 streamNo)
{
   OMX_U32 ret = MMI_S_EBADPARAM;
   int32 trk = 0;
   HTTPMediaType majorType = HTTPCommon::HTTP_UNKNOWN_TYPE;
   HTTPSourceMMITrackHandler::tTrackState tState = {false, false};

   if(m_pHTTPSourceMMITrackHandler &&
      m_pHTTPSourceMMITrackHandler->MapStreamNumberToTrackID(portIdx, streamNo,
                                                             trk, majorType))
   {
     tState.isMuted = false;
     tState.isSelected = true;
     m_pHTTPSourceMMITrackHandler->SetTrackState((int)trk, majorType, tState, NULL);
     ret = MMI_S_COMPLETE;
   }
   return ret;
}

/**@brief LoadResources
*@param[in]
*@param[out]
*@return
*/
OMX_U32 HTTPSourceMMI::LoadResources(void *pData)
{
  //Component state changes from LOADED->IDLE
  OMX_U32 ret = MMI_S_COMPLETE;
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
               "HTTPSourceMMI::LoadResources");
  ret = Open(pData);
  return ret;
}

/**@brief ReleaseResources
*@param[in]
*@param[out]
*@return
*/
OMX_U32 HTTPSourceMMI::ReleaseResources(void)
{
   //Component state changes from IDLE->LOADED
   OMX_U32 ret = MMI_S_COMPLETE;
   QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
               "HTTPSourceMMI::ReleaseResources");
   return ret;
}

/**@brief WaitForResources
*@param[in]
*@param[out]
*@return
*/
OMX_U32 HTTPSourceMMI::WaitForResources(void *pData)
{
  //Component state changes from LOADED->WAITFORRESOURCES
   OMX_U32 ret = MMI_S_PENDING;
   QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
               "HTTPSourceMMI::WaitForResources");
   m_pHTTPController->WaitForResources(pData);
   return ret;
}

/**@brief ReleaseWaitForResources
*@param[in]
*@param[out]
*@return
*/
OMX_U32 HTTPSourceMMI::ReleaseWaitForResources(void)
{
   OMX_U32 ret = MMI_S_COMPLETE;

   QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
               "HTTPSourceMMI::ReleaseWaitForResources");
   return ret;
}

/**@brief set default port settings
*@param[in]
*@param[out]
*@return
*/
void HTTPSourceMMI::InitPorts()
{
  int i = 0;
  HttpSourceMmiPortInfo *pPort = NULL;
  for(i = 0;i<MMI_HTTP_NUM_VIDEO_PORTS;i++)
  {
    pPort = &m_portVideo[i];
    pPort->m_bSwitching = false;
    pPort->m_portDef.nSize = (OMX_U32)sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    pPort->m_portDef.bEnabled = OMX_TRUE;
    pPort->m_portDef.bPopulated = OMX_FALSE;
    pPort->m_portDef.eDir = OMX_DirOutput;
    pPort->m_portDef.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX+i;
    pPort->m_portDef.nBufferCountMin = MMI_HTTP_VIDEO_PORT_MIN_BUFFER_COUNT;
    pPort->m_portDef.nBufferCountActual = pPort->m_portDef.nBufferCountMin;
    pPort->m_portDef.nBufferSize = MMI_HTTP_VIDEO_PORT_MAX_BUFFER_SIZE;
    pPort->m_portDef.eDomain = OMX_PortDomainVideo;
    pPort->m_portDef.bBuffersContiguous = OMX_FALSE;
    pPort->m_portDef.nBufferAlignment = 0;
    pPort->m_portDef.format.video.cMIMEType = NULL;
    pPort->m_portDef.format.video.pNativeRender = NULL;
    pPort->m_portDef.format.video.nFrameWidth = 0;
    pPort->m_portDef.format.video.nFrameHeight = 0;
    pPort->m_portDef.format.video.nStride = 0;
    pPort->m_portDef.format.video.nSliceHeight = 0;
    pPort->m_portDef.format.video.nBitrate = 0;
    pPort->m_portDef.format.video.xFramerate = 0;
    pPort->m_portDef.format.video.bFlagErrorConcealment = OMX_TRUE;
    pPort->m_portDef.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
    pPort->m_portDef.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    pPort->m_portDef.format.video.pNativeWindow = NULL;
    pPort->m_nBufferFlags = OMX_BUFFERFLAG_STARTTIME;
    pPort->m_portDef.nBufferSize += (OMX_U32)(MMI_HTTP_MAX_PORT_ENC_BUFFER_SIZE);

  }

  for(i = 0;i<MMI_HTTP_NUM_AUDIO_PORTS;i++)
  {
    pPort = &m_portAudio[i];
    pPort->m_bSwitching = false;
    pPort->m_portDef.nSize = (OMX_U32)sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    pPort->m_portDef.bEnabled = OMX_TRUE;
    pPort->m_portDef.bPopulated = OMX_FALSE;
    pPort->m_portDef.eDir = OMX_DirOutput;
    pPort->m_portDef.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX+i;
    pPort->m_portDef.nBufferCountMin = MMI_HTTP_AUDIO_PORT_MIN_BUFFER_COUNT;
    pPort->m_portDef.nBufferCountActual = pPort->m_portDef.nBufferCountMin;
    pPort->m_portDef.nBufferSize = MMI_HTTP_AUDIO_PORT_DEFAULT_BUFFER_SIZE;
    pPort->m_portDef.eDomain = OMX_PortDomainAudio;
    pPort->m_portDef.bBuffersContiguous = OMX_FALSE;
    pPort->m_portDef.nBufferAlignment = 0;
    pPort->m_portDef.format.audio.cMIMEType = NULL;
    pPort->m_portDef.format.audio.pNativeRender = NULL;
    pPort->m_portDef.format.audio.bFlagErrorConcealment = OMX_TRUE;
    pPort->m_portDef.format.audio.eEncoding = OMX_AUDIO_CodingUnused;
    pPort->m_nBufferFlags = OMX_BUFFERFLAG_STARTTIME ;
    pPort->m_portDef.nBufferSize += (OMX_U32)(MMI_HTTP_MAX_PORT_ENC_BUFFER_SIZE);

  }

  for(i = 0;i<MMI_HTTP_NUM_IMAGE_PORTS;i++)
  {
    pPort = &m_portImage[i];
    pPort->m_bSwitching = false;
    pPort->m_portDef.nSize = (OMX_U32)sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    pPort->m_portDef.bEnabled = OMX_TRUE;
    pPort->m_portDef.bPopulated = OMX_FALSE;
    pPort->m_portDef.eDir = OMX_DirOutput;
    pPort->m_portDef.nPortIndex = MMI_HTTP_IMAGE_PORT_INDEX+i;
    pPort->m_portDef.nBufferCountMin = MMI_HTTP_IMAGE_PORT_MIN_BUFFER_COUNT;
    pPort->m_portDef.nBufferCountActual = pPort->m_portDef.nBufferCountMin;
    pPort->m_portDef.nBufferSize = MMI_HTTP_IMAGE_PORT_DEFAULT_BUFFER_SIZE;
    pPort->m_portDef.eDomain = OMX_PortDomainImage;
    pPort->m_portDef.bBuffersContiguous = OMX_FALSE;
    pPort->m_portDef.nBufferAlignment = 0;
    pPort->m_portDef.format.image.cMIMEType = NULL;
    pPort->m_portDef.format.image.pNativeRender = NULL;
    pPort->m_portDef.format.image.nFrameWidth = 0;
    pPort->m_portDef.format.image.nFrameHeight = 0;
    pPort->m_portDef.format.image.nStride = 0;
    pPort->m_portDef.format.image.nSliceHeight = 0;
    pPort->m_portDef.format.image.bFlagErrorConcealment = OMX_TRUE;
    pPort->m_portDef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
    pPort->m_portDef.format.image.eColorFormat = OMX_COLOR_FormatUnused;
    pPort->m_portDef.format.image.pNativeWindow = NULL;
    pPort->m_nBufferFlags = OMX_BUFFERFLAG_STARTTIME ;
  }

  for(i = 0;i<MMI_HTTP_NUM_OTHER_PORTS;i++)
  {
    pPort = &m_portOther[i];
    pPort->m_bSwitching = false;
    pPort->m_portDef.nSize = (OMX_U32)sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    pPort->m_portDef.bEnabled = OMX_TRUE;
    pPort->m_portDef.bPopulated = OMX_FALSE;
    pPort->m_portDef.eDir = OMX_DirOutput;
    pPort->m_portDef.nPortIndex = MMI_HTTP_OTHER_PORT_INDEX+i;
    pPort->m_portDef.nBufferCountMin = MMI_HTTP_OTHER_PORT_MIN_BUFFER_COUNT;
    pPort->m_portDef.nBufferCountActual = pPort->m_portDef.nBufferCountMin;
    // win32 Conf test doesnt consider the new buffer requirments announced after port config changed
    pPort->m_portDef.nBufferSize =  MMI_HTTP_OTHER_PORT_DEFAULT_BUFFER_SIZE +
                                    (OMX_U32)(MMI_HTTP_TEXT_EXTRADATA_BUFFER_SIZE);
    pPort->m_portDef.eDomain = OMX_PortDomainOther;
    pPort->m_portDef.bBuffersContiguous = OMX_FALSE;
    pPort->m_portDef.nBufferAlignment = 0;
    pPort->m_portDef.format.other.eFormat = (OMX_OTHER_FORMATTYPE)QOMX_OTHER_CodingUnused;;
    pPort->m_nBufferFlags = OMX_BUFFERFLAG_STARTTIME ;
  }
}

/**@brief Check if any of the port is set to auto-detect or not
*@param[in]
*@param[out]
*@return - true if any port is set to auto-detect
* false otherwise
*/
bool HTTPSourceMMI::IsPortSetToAutoDetect()
{
  int i = 0;
  bool ret = false;
  HttpSourceMmiPortInfo *pPort = NULL;
  //Check if any video port is set to auto-detect
  for(i = 0;i<MMI_HTTP_NUM_VIDEO_PORTS;i++)
  {
    pPort = &m_portVideo[i];
    if(pPort->m_portDef.format.video.eCompressionFormat ==
        OMX_VIDEO_CodingAutoDetect)
    {
      ret = true;
      break;
    }
  }
  //If no video port is set to auto-detect check for audio ports
  for(i = 0;i<MMI_HTTP_NUM_AUDIO_PORTS && ret == false;i++)
  {
    pPort = &m_portAudio[i];
    if(pPort->m_portDef.format.audio.eEncoding ==
        OMX_AUDIO_CodingAutoDetect)
    {
      ret = true;
      break;
    }
  }
  return ret;
}
/**@brief Updates all ports
  *@param[in] bSendPortEvents - true if port detection/configuration
  *events should be sent to mmi layer else false
  * @return void
  */
void HTTPSourceMMI::UpdatePorts(bool bSendPortEvents)
{
  uint32 nTracks = 0;
  bool isAudioPortUpdated = false;
  bool isVideoPortUpdated = false;
  bool isOtherPortUpdated = false;
  bool isAudioPortEventSendPending = bSendPortEvents;
  bool isVideoPortEventSendPending = bSendPortEvents;
  bool isOtherPortEventSendPending = bSendPortEvents;
  int audPortIdx = 0;
  int vidPortIdx = 0;
  int OtherPortIdx = 0;
  HTTPSourceMMITrackHandler::TrackDescription unknownTrackDesc;

  memset(&unknownTrackDesc, 0x00,
         sizeof(HTTPSourceMMITrackHandler::TrackDescription));

  if ( m_pHTTPSourceMMIHelper == NULL )
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid HTTPSourceMMIHelper object" );
    return;
  }

  if ( m_pHTTPDataInterface == NULL )
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "Neither of the "
                  "interfaces DataResourceManager and DataInterface exist" );
    return;
  }

  if (m_pHTTPSourceMMITrackHandler)
  {
    nTracks = m_pHTTPSourceMMITrackHandler->m_trackList.size();
  }
  if(bSendPortEvents)
  {
    bool bSendPortDetection = IsPortSetToAutoDetect();
    if(bSendPortDetection)
    {
      if (!nTracks)
      {
        //Notify omx base with event port format not detected
        m_pHTTPSourceMMIHelper->NotifyMmi(MMI_EVT_PORT_DETECTION, MMI_S_EFAIL,
                                          0, NULL,
                                          m_pClientData);
        return;
      }
      else
      {
        //Notify omx base with event port format detected
        m_pHTTPSourceMMIHelper->NotifyMmi(MMI_EVT_PORT_DETECTION, MMI_S_COMPLETE,
                                          0, NULL,
                                          m_pClientData);
      }
    }
  }
  if(!nTracks)
  {
     //Notify omx base with resource lost message if no tracks are there.
    MMI_ResourceLostMsgType resourceLostMsg;
    resourceLostMsg.bSuspendable = OMX_FALSE;
    UpdateStopPhrase((char*)STOP_ERROR_STRING);
    m_pHTTPSourceMMIHelper->NotifyMmi(MMI_EVT_RESOURCES_LOST, MMI_S_EFAIL,
              sizeof(MMI_ResourceLostMsgType), (void *)&resourceLostMsg, m_pClientData);
    return;
  }
  if (m_pHTTPController && m_pHTTPController->IsHTTPStreamerRunning())
  {
    //Loop through the track list and update the track state if found
    for (ListPair<HTTPSourceMMITrackHandler::TrackDescription>* it = m_pHTTPSourceMMITrackHandler->m_trackList.iterator();
         it != NULL; it = it->tail())
    {
      HTTPSourceMMITrackHandler::TrackDescription& trackDescription = it->head();

      if((trackDescription.majorType == FILE_SOURCE_MJ_TYPE_AUDIO) &&
        audPortIdx < MMI_HTTP_NUM_AUDIO_PORTS)
      {
        UpdateAudioPort(audPortIdx, trackDescription, isAudioPortEventSendPending);
        audPortIdx++;
        isAudioPortUpdated = OMX_TRUE;
      }
      else if ((trackDescription.majorType == FILE_SOURCE_MJ_TYPE_VIDEO) &&
        (vidPortIdx < MMI_HTTP_NUM_VIDEO_PORTS ))
      {
        UpdateVideoPort(vidPortIdx, trackDescription, isVideoPortEventSendPending);
        vidPortIdx++;
        isVideoPortUpdated = OMX_TRUE;
      }
      else if((trackDescription.majorType == FILE_SOURCE_MJ_TYPE_TEXT) &&
        (OtherPortIdx < MMI_HTTP_NUM_OTHER_PORTS ))
      {
        UpdateOtherPort(OtherPortIdx, trackDescription, isOtherPortEventSendPending);
        OtherPortIdx++;
        isOtherPortUpdated = OMX_TRUE;
      }
    }
  }

  if (!isAudioPortUpdated)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Audioport updated with unknown codec" );
    InvalidateAudioPort(audPortIdx, isAudioPortEventSendPending);
  }

  if (!isVideoPortUpdated)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Videoport updated with unknown codec" );
    InvalidateVideoPort(vidPortIdx, isVideoPortEventSendPending);
  }

  if(!isOtherPortUpdated)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Otherport updated with unknown codec" );
    InvalidateOtherPort(OtherPortIdx, isOtherPortEventSendPending);
  }

  // Check and send port config events for all the tracks
  CheckAndSendPortConfigChangeEvent(HTTPCommon::HTTP_AUDIO_TYPE, 0, bSendPortEvents);
  CheckAndSendPortConfigChangeEvent(HTTPCommon::HTTP_VIDEO_TYPE, 0, bSendPortEvents);
  CheckAndSendPortConfigChangeEvent(HTTPCommon::HTTP_TEXT_TYPE, 0, bSendPortEvents);

  return;
}

/* Compare the current port setting with the stored one and update the ports
 * required
 *
 * return result of operation
 */
HTTPDownloadStatus HTTPSourceMMI::CompareAndUpdatePorts(bool &isAudioPortConfigPending,
                                                        bool &isVideoPortConfigPending,
                                                        bool &isOtherPortConfigPending)
{
  bool isAudioPortUpdateNeeded = false;
  bool isVideoPortUpdateNeeded = false;
  bool isOtherPortUpdateNeeded = false;

  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
  if ( m_pHTTPDataInterface == NULL )
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR, "DataInterface(NULL) exist");
    return HTTPCommon::HTTPDL_ERROR_ABORT;
  }

  // Traverse throug the existing tracklist to find out the tracks to be
  // updated after seek
  //Loop through the track list and update the track state if found
  for (ListPair<HTTPSourceMMITrackHandler::TrackDescription>* it = m_pHTTPSourceMMITrackHandler->m_trackList.iterator();
    it != NULL; it = it->tail())
  {
    HTTPSourceMMITrackHandler::TrackDescription& trackDescription = it->head();

    if(FILE_SOURCE_MJ_TYPE_AUDIO == trackDescription.majorType)
    {
      isAudioPortUpdateNeeded = true;
    }
    else if (FILE_SOURCE_MJ_TYPE_VIDEO == trackDescription.majorType)
    {
      isVideoPortUpdateNeeded = true;
    }
    else if(FILE_SOURCE_MJ_TYPE_TEXT == trackDescription.majorType)
    {
      isOtherPortUpdateNeeded = true;
    }
  }


  // Check if any new tracks are added after seek

  HTTPMediaTrackInfo* pTrackInfo = NULL;
  uint32 nNewNumTracks = 0;

  if (m_pHTTPDataInterface)
  {
    nNewNumTracks = m_pHTTPDataInterface->GetMediaTrackInfo(pTrackInfo);
  }


  pTrackInfo = (HTTPMediaTrackInfo *) (QTV_Malloc(nNewNumTracks * sizeof(HTTPMediaTrackInfo)));
  if (pTrackInfo && m_pHTTPDataInterface)
  {
    nNewNumTracks = m_pHTTPDataInterface->GetMediaTrackInfo(pTrackInfo);
    for (uint32 index = 0; index < nNewNumTracks; index++)
    {
      if (pTrackInfo + index == NULL)
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: Track info for track %u is NULL", index );
        continue;
      }
      else if ( pTrackInfo[index].bSelected )
      {
        if ((false == isAudioPortUpdateNeeded) && (HTTPCommon::HTTP_AUDIO_TYPE == pTrackInfo[index].majorType))
        {
          isAudioPortUpdateNeeded = true;
        }
        else if((false == isVideoPortUpdateNeeded) && (HTTPCommon::HTTP_VIDEO_TYPE == pTrackInfo[index].majorType))
        {
          isVideoPortUpdateNeeded = true;
        }
        else if((false == isOtherPortUpdateNeeded) && (HTTPCommon::HTTP_TEXT_TYPE == pTrackInfo[index].majorType))
        {
          isOtherPortUpdateNeeded = true;
        }
      }
    }
  }

  if(pTrackInfo)
  {
    QTV_Free(pTrackInfo);
    pTrackInfo = NULL;
  }


  isAudioPortConfigPending = false;
  isVideoPortConfigPending = false;
  isOtherPortConfigPending = false;
  eStatus = HTTPCommon::HTTPDL_SUCCESS;

  //Update tracks and ports after seek and cache the information
  // to send portconfig event for the track after seek completes
  HTTPCommon::HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
  if(isAudioPortUpdateNeeded)
  {
    mediaType = HTTPCommon::HTTP_AUDIO_TYPE;
    eStatus = HandleSwitch(mediaType, isAudioPortConfigPending);
    if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
    {
      eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "CompareAndUpdatePorts: Audio Port updated, isPortConfigpending %d", isAudioPortConfigPending);
    }
  }

  if((isVideoPortUpdateNeeded) && (HTTPCommon::HTTPDL_SUCCESS == eStatus))
  {
    mediaType = HTTPCommon::HTTP_VIDEO_TYPE;
    eStatus = HandleSwitch(mediaType, isVideoPortConfigPending);
    if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
    {
      eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "CompareAndUpdatePorts: Video Port updated, isPortConfigpending %d", isVideoPortConfigPending);
    }
  }

  if((isOtherPortUpdateNeeded) && (HTTPCommon::HTTPDL_SUCCESS == eStatus))
  {
    mediaType = HTTPCommon::HTTP_TEXT_TYPE;
    eStatus = HandleSwitch(mediaType, isOtherPortConfigPending);
    if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
    {
      eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "CompareAndUpdatePorts: Text Port updated, isPortConfigpending %d", isOtherPortConfigPending);
    }
  }

  return eStatus;
}
/* Handles switch for the given media type
 *
 * @param[in] mediaType - HTTP media type
 * @param[out] isPortEventPending - flags indicates if port config change event send is required
 * return result of switch operation
 */
HTTPDownloadStatus HTTPSourceMMI::HandleSwitch(const HTTPMediaType mediaType, bool &isPortEventPending)
{
  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMITrackHandler, HTTPCommon::HTTPDL_ERROR_ABORT);
  HTTPDownloadStatus eStatus = m_pHTTPSourceMMITrackHandler->CompareAndUpdateTrack(mediaType);
  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "HandleSwitch %d, %d", mediaType, eStatus);

  if (eStatus == HTTPCommon::HTTPDL_SUCCESS ||
      eStatus == HTTPCommon::HTTPDL_CODEC_INFO_CHANGED)
  {
    FileSourceMjMediaType majorType;
    HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType(mediaType, majorType);
    if(mediaType == HTTPCommon::HTTP_VIDEO_TYPE)
    {
      m_pHTTPSourceMMIHelper->ProcessQOENotification(QOMX_HTTP_IndexParamQOESwitch);
    }
    bool bSendPortEvent = (eStatus == HTTPCommon::HTTPDL_CODEC_INFO_CHANGED)? true:false;

    //Loop through the track list and update the port defn and track state if found
    eStatus = HTTPCommon::HTTPDL_ERROR_ABORT;
    ListPair<HTTPSourceMMITrackHandler::TrackDescription>* it;
    for ( it = m_pHTTPSourceMMITrackHandler->m_trackList.iterator(); it != NULL;
          it = it->tail() )
    {
      HTTPSourceMMITrackHandler::TrackDescription& trackDescription = it->head();
      if( trackDescription.majorType == majorType )
      {
        if( trackDescription.majorType == FILE_SOURCE_MJ_TYPE_AUDIO ||
            trackDescription.majorType == FILE_SOURCE_MJ_TYPE_VIDEO ||
            trackDescription.majorType == FILE_SOURCE_MJ_TYPE_TEXT )
        {
          // does not support storing multiple tracks in ports # audio/video ports is 1
          if (trackDescription.majorType == FILE_SOURCE_MJ_TYPE_AUDIO)
          {
            UpdateAudioPort(0, trackDescription, bSendPortEvent);
          }
          else if(trackDescription.majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
          {
            UpdateVideoPort(0, trackDescription, bSendPortEvent);
          }
          else if(trackDescription.majorType == FILE_SOURCE_MJ_TYPE_TEXT)
          {
            UpdateOtherPort(0,trackDescription,bSendPortEvent);
          }

          //update the flag, so that port config event can be sent after handle switch
          isPortEventPending = bSendPortEvent;
          eStatus = HTTPCommon::HTTPDL_SUCCESS;
          break;
        }
      }
    }
  }
  else if((eStatus == HTTPCommon::HTTPDL_UNSUPPORTED)||
          (eStatus == HTTPCommon::HTTPDL_DATA_END))
  {
    //This is when a track is removed!
    FileSourceMjMediaType majorType;
    HTTPCommon::MapHTTPMediaTypeToFileSourceMajorType(mediaType, majorType);
    HTTPSourceMMITrackHandler::TrackDescription trackDescription;

    if(mediaType == HTTPCommon::HTTP_VIDEO_TYPE)
    {
      m_pHTTPSourceMMIHelper->ProcessQOENotification(QOMX_HTTP_IndexParamQOESwitch);
    }

    isPortEventPending = true;
    if (majorType == FILE_SOURCE_MJ_TYPE_AUDIO)
    {
      InvalidateAudioPort(0, isPortEventPending);
    }
    else if(majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
    {
      InvalidateVideoPort(0, isPortEventPending);
    }
    else if(majorType == FILE_SOURCE_MJ_TYPE_TEXT)
    {
      InvalidateOtherPort(0, isPortEventPending);
    }

    eStatus = HTTPCommon::HTTPDL_SUCCESS;
  }

  return eStatus;
}

/* Updates audio port and send port changed, port detected notifications to mmi.
 *
 * @param[in] PortIndex - port index to be updated
 * @param[in] trackDescription - track description
 * @param[in] bSendPortEvents - true if port config change events should be
 *                              sent to IL client
 *
 * @return void
  */
void HTTPSourceMMI::UpdateAudioPort
(
  OMX_U32 PortIndex,
  HTTPSourceMMITrackHandler::TrackDescription &trackDescription,
  bool &bSendPortEvents
)
{
  HTTPMediaTrackInfo trackInfo;
  MMI_PortMsgType audPort;
  HTTPMediaType httpMediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
  HttpSourceMmiPortInfo* pPortAddr = &m_portAudio[PortIndex];
  QTV_NULL_PTR_CHECK(m_pHTTPDataInterface, RETURN_VOID);
  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMITrackHandler,RETURN_VOID);
  if((m_pHTTPSourceMMITrackHandler->GetOmxAudioMinorType(trackDescription.minorType) ==
      pPortAddr->m_portDef.format.audio.eEncoding) && (bSendPortEvents))
  {
    if(pPortAddr->m_portDef.format.audio.eEncoding == OMX_AUDIO_CodingAAC)
    {
      OMX_AUDIO_PARAM_AACPROFILETYPE *pAacFmt =
           (OMX_AUDIO_PARAM_AACPROFILETYPE*)pPortAddr->m_pPortSpecificData;
      if(pAacFmt->nChannels == trackDescription.numChannels &&
         pAacFmt->nSampleRate == trackDescription.samplingRate &&
         pAacFmt->nBitRate == trackDescription.bitRate)
      {
        bSendPortEvents = false;
      }
    }
    else
    {
      //If codec is same there is no need to send port config change event
      bSendPortEvents = false;
    }
  }
  if (pPortAddr->m_pPortSpecificData)
  {
    QTV_Free(pPortAddr->m_pPortSpecificData);
    pPortAddr->m_pPortSpecificData = NULL;
  }

  memset(&trackInfo, 0x0, sizeof(trackInfo));
  pPortAddr->m_trackId = trackDescription.trackID;
  pPortAddr->m_isTrackValid = OMX_TRUE;
  SetPortSwitchingFlag(pPortAddr,false);

  audPort.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX + PortIndex;

  //Update port domain & port index
  pPortAddr->m_portDef.eDomain    = OMX_PortDomainAudio;
  pPortAddr->m_portDef.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX + PortIndex;

  //Update port buffer minimum requirement Set nBufferCountActual to nBufferCountMin,
  //nBufferCountActual can't be less than nBufferCountMin.
  pPortAddr->m_portDef.nBufferCountMin = MMI_HTTP_AUDIO_PORT_MIN_BUFFER_COUNT;
  pPortAddr->m_portDef.nBufferCountActual = pPortAddr->m_portDef.nBufferCountMin;

  //Get the largest frame size in the audio track
  pPortAddr->m_portDef.nBufferSize = MMI_HTTP_AUDIO_PORT_DEFAULT_BUFFER_SIZE;

  //Get media track information & update the respective eCoding types
  HTTPCommon::MapFileSourceMajorTypeToHTTPMediaType( trackDescription.majorType,
                                                     httpMediaType );
  m_pHTTPDataInterface->GetSelectedMediaTrackInfo(httpMediaType, trackInfo);

  // allocating more memory requird if DRM protected
  if (trackInfo.audioTrackInfo.drmInfo.isDRMProtected)
  {
    pPortAddr->m_portDef.nBufferSize += (OMX_U32)(MMI_HTTP_MAX_PORT_ENC_BUFFER_SIZE);
  }

  pPortAddr->m_portDef.format.audio.eEncoding =
    m_pHTTPSourceMMITrackHandler->GetOmxAudioMinorType(trackDescription.minorType);

  switch ((uint32)pPortAddr->m_portDef.format.audio.eEncoding)
  {
    case OMX_AUDIO_CodingAAC:
    {
      OMX_AUDIO_PARAM_AACPROFILETYPE* pAacParam = NULL;
      pPortAddr->m_pPortSpecificData =
         (OMX_AUDIO_PARAM_AACPROFILETYPE*)QTV_Malloc(sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));

      if (NULL == pPortAddr->m_pPortSpecificData)
      {
        break;
      }
      pAacParam = (OMX_AUDIO_PARAM_AACPROFILETYPE*)pPortAddr->m_pPortSpecificData;
      QOMX_STRUCT_INIT(*pAacParam, OMX_AUDIO_PARAM_AACPROFILETYPE);

      UpdateAacAudioPort(PortIndex, pAacParam, trackInfo);
    }
    break;

    case (OMX_AUDIO_CODINGTYPE) QOMX_EXT_AUDIO_CodingAC3:
    {
      QOMX_AUDIO_PARAM_AC3PROFILETYPE* pAc3Param = NULL;
      pPortAddr->m_pPortSpecificData =
         (QOMX_AUDIO_PARAM_AC3PROFILETYPE*)QTV_Malloc(sizeof(QOMX_AUDIO_PARAM_AC3PROFILETYPE));

      if (NULL == pPortAddr->m_pPortSpecificData)
      {
        break;
      }

      pAc3Param = (QOMX_AUDIO_PARAM_AC3PROFILETYPE*)pPortAddr->m_pPortSpecificData;
      QOMX_STRUCT_INIT(*pAc3Param, QOMX_AUDIO_PARAM_AC3PROFILETYPE);

      UpdateAc3AudioPort(PortIndex, pAc3Param, trackInfo);

    }
    break;

    case (OMX_AUDIO_CODINGTYPE) QOMX_EXT_AUDIO_CodingMP2:
    {
      QOMX_AUDIO_PARAM_MP2PROFILETYPE* pMp2Param = NULL;
      pPortAddr->m_pPortSpecificData =
         (QOMX_AUDIO_PARAM_MP2PROFILETYPE*)QTV_Malloc(sizeof(QOMX_AUDIO_PARAM_MP2PROFILETYPE));

      if (NULL == pPortAddr->m_pPortSpecificData)
      {
        break;
      }

      pMp2Param = (QOMX_AUDIO_PARAM_MP2PROFILETYPE*)pPortAddr->m_pPortSpecificData;
      QOMX_STRUCT_INIT(*pMp2Param, QOMX_AUDIO_PARAM_MP2PROFILETYPE);

      UpdateMp2AudioPort(PortIndex, pMp2Param, trackInfo);

    }
    break;

    default:
    {
      pPortAddr->m_portDef.format.audio.eEncoding = OMX_AUDIO_CodingUnused;
    }
    break;
  }
}

/* Invalidates audio port and send port changed, port detected notifications to mmi.
 *
 * @param[in] PortIndex - port index to be updated
 * @param[in] bSendPortEvents - true if port config change events should be
 *                              sent to IL client
 *
 * @return void
  */
void HTTPSourceMMI::InvalidateAudioPort
(
  OMX_U32 PortIndex,
  bool &bSendPortEvents
)
{
  MMI_PortMsgType audPort;
  HttpSourceMmiPortInfo* pPortAddr = &m_portAudio[PortIndex];
  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMITrackHandler,RETURN_VOID);
  if(pPortAddr->m_portDef.format.audio.eEncoding == OMX_AUDIO_CodingUnused )
  {
    //If codec is unused, not need to send port config event
    bSendPortEvents = false;
  }

  pPortAddr->m_trackId = 0;
  pPortAddr->m_isTrackValid = OMX_FALSE;
  SetPortSwitchingFlag(pPortAddr,false);
  if (pPortAddr->m_pPortSpecificData)
  {
    QTV_Free(pPortAddr->m_pPortSpecificData);
    pPortAddr->m_pPortSpecificData = NULL;
  }
  pPortAddr->m_portDef.format.audio.cMIMEType = NULL;
  pPortAddr->m_portDef.format.audio.pNativeRender = NULL;
  pPortAddr->m_portDef.format.audio.bFlagErrorConcealment = OMX_TRUE;
  pPortAddr->m_portDef.format.audio.eEncoding = OMX_AUDIO_CodingUnused;
  pPortAddr->m_portDef.eDomain    = OMX_PortDomainAudio;
  pPortAddr->m_portDef.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX + PortIndex;
  pPortAddr->m_portDef.nBufferCountMin = MMI_HTTP_AUDIO_PORT_MIN_BUFFER_COUNT;
  pPortAddr->m_portDef.nBufferCountActual = pPortAddr->m_portDef.nBufferCountMin;
  pPortAddr->m_portDef.nBufferSize = MMI_HTTP_AUDIO_PORT_DEFAULT_BUFFER_SIZE;

  audPort.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX + PortIndex;
}

#ifdef FEATURE_HTTP_AMR
/**@brief Updates amr port information
  *@param[in] PortIndex - port index to be updated
              pAmrParam - amr omx audio param
              trackInfo - track info
  *@param[out]
  * @return void
  */
void HTTPSourceMMI::UpdateAmrAudioPort(OMX_U32 audioPortIndex,
                                       QOMX_AUDIO_PARAM_AMRWBPLUSTYPE* pAmrParam,
                                       MediaTrackInfo &trackInfo)
{
  if (NULL == pAmrParam)
  {
    return;
  }

  pAmrParam->nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX + audioPortIndex;
  pAmrParam->eAMRDTXMode  = OMX_AUDIO_AMRDTXModeOff;
  pAmrParam->eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatConformance;
  switch (trackInfo.audioTrackInfo.audioCodec)
  {
    case FILE_SOURCE_MN_TYPE_GSM_AMR:
    case FILE_SOURCE_MN_TYPE_NONMP4_AMR:
    case FILE_SOURCE_MN_TYPE_CONC_AMR:
      pAmrParam->eAMRBandMode = OMX_AUDIO_AMRBandModeNB7;
      break;
    case FILE_SOURCE_MN_TYPE_AMR_WB:
    case FILE_SOURCE_MN_TYPE_AMR_WB_PLUS:
      pAmrParam->eAMRBandMode = (OMX_AUDIO_AMRBANDMODETYPE)QOMX_AUDIO_AMRBandModeWB9;
      break;
    default:
      break;
  }
  pAmrParam->nChannels  = trackInfo.audioTrackInfo.numChannels;
  pAmrParam->nBitRate = trackInfo.audioTrackInfo.bitRate;
  return;
}
#endif // FEATURE_HTTP_AMR

/* @brief Updates aac port information
 * @param[in] PortIndex - port index to be updated
 * @param[in] pAacParam - aac omx audio param
 * @param[in] trackInfo - track info
 *
 * @return void
  */
void HTTPSourceMMI::UpdateAacAudioPort(OMX_U32 audioPortIndex,
                                       OMX_AUDIO_PARAM_AACPROFILETYPE* pAacParam,
                                       HTTPMediaTrackInfo &trackInfo)
{
  HTTPCodecData sCodecData;
  HTTPDownloadStatus eCodecStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

  if (NULL == pAacParam)
  {
    return;
  }
  pAacParam->nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX + audioPortIndex;
  pAacParam->nChannels = trackInfo.audioTrackInfo.numChannels;
  pAacParam->nSampleRate = trackInfo.audioTrackInfo.samplingRate;
  pAacParam->nBitRate = trackInfo.audioTrackInfo.bitRate;
  pAacParam->eChannelMode = (trackInfo.audioTrackInfo.numChannels < 2) ?
                             OMX_AUDIO_ChannelModeMono : OMX_AUDIO_ChannelModeStereo;

  memset(&sCodecData, 0,sizeof(HTTPCodecData));
  eCodecStatus = m_pHTTPDataInterface->GetCodecData(
    trackInfo.nTrackID, trackInfo.majorType, trackInfo.minorType, sCodecData);
  if (eCodecStatus != HTTPCommon::HTTPDL_SUCCESS)
  {
    QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Get Codec data failed for track %u status %d",
                   trackInfo.nTrackID, eCodecStatus );
  }
  else
  {
    //Update AAC profile
    pAacParam->eAACProfile = (OMX_AUDIO_AACPROFILETYPE)sCodecData.aacCodecData.eAACProfile;

    //Update AAC Stream Format
    switch(sCodecData.aacCodecData.eAACStreamFormat)
    {
      case FILE_SOURCE_AAC_FORMAT_UNKNWON:
        pAacParam->eAACStreamFormat = OMX_AUDIO_AACStreamFormatRAW;
        break;
      case FILE_SOURCE_AAC_FORMAT_ADTS:
        pAacParam->eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4ADTS;
        break;
      case FILE_SOURCE_AAC_FORMAT_ADIF:
        pAacParam->eAACStreamFormat = OMX_AUDIO_AACStreamFormatADIF;
        break;
      case FILE_SOURCE_AAC_FORMAT_RAW:
        pAacParam->eAACStreamFormat = OMX_AUDIO_AACStreamFormatRAW;
        break;
      case FILE_SOURCE_AAC_FORMAT_LOAS:
        pAacParam->eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP4LOAS;
        break;
      default:
        pAacParam->eAACStreamFormat = OMX_AUDIO_AACStreamFormatRAW;
        break;
    }
  }
  return;
}


void HTTPSourceMMI::UpdateAc3AudioPort(OMX_U32 audioPortIndex,
                                       QOMX_AUDIO_PARAM_AC3PROFILETYPE* pAc3Param,
                                       HTTPMediaTrackInfo &trackInfo)
{
  HTTPCodecData sCodecData;
  sCodecData.aacCodecData.eAACProfile = HTTP_AAC_PROFILE_NULL;
  sCodecData.aacCodecData.eAACStreamFormat = HTTP_AAC_FORMAT_UNKNWON;

  HTTPDownloadStatus eCodecStatus = HTTPCommon::HTTPDL_ERROR_ABORT;

  if (NULL == pAc3Param)
  {
    return;
  }
  pAc3Param->nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX + audioPortIndex;
  pAc3Param->nChannels = trackInfo.audioTrackInfo.numChannels;
  pAc3Param->nSampleRate = trackInfo.audioTrackInfo.samplingRate;
  pAc3Param->nBitRate = trackInfo.audioTrackInfo.bitRate;
  pAc3Param->eChannelMode = (trackInfo.audioTrackInfo.numChannels < 2) ?
                             OMX_AUDIO_ChannelModeMono : OMX_AUDIO_ChannelModeStereo;


  return;
}

/**@brief Updates MP2 audio port information
  *@param[in] PortIndex - port index to be updated
              pMp2Param - mp2 omx audio param
              trackInfo - track info
  *@param[out]
  * @return void
  */
void HTTPSourceMMI::UpdateMp2AudioPort(OMX_U32 audioPortIndex,
                                       QOMX_AUDIO_PARAM_MP2PROFILETYPE* pMp2Param,
                                       HTTPMediaTrackInfo &trackInfo)
{
  if (NULL == pMp2Param)
  {
    return;
  }

  pMp2Param->nPortIndex   = MMI_HTTP_AUDIO_PORT_INDEX + audioPortIndex;
  pMp2Param->nChannels    = trackInfo.audioTrackInfo.numChannels;
  pMp2Param->nSampleRate  = trackInfo.audioTrackInfo.samplingRate;
  pMp2Param->nBitRate     = trackInfo.audioTrackInfo.bitRate;
  pMp2Param->eChannelMode = (trackInfo.audioTrackInfo.numChannels < 2) ?
                             OMX_AUDIO_ChannelModeMono : OMX_AUDIO_ChannelModeStereo;
}

/*
 * Updates video port and send port changed, port detected notifications to mmi.
 *
 * @param[in] PortIndex - port index to be updated
 * @param[in] trackDescription - track description
 * @param[in] bSendPortEvents - true if port config change
 */
void HTTPSourceMMI::UpdateVideoPort
(
  OMX_U32 PortIndex,
  HTTPSourceMMITrackHandler::TrackDescription &trackDescription,
  bool &bSendPortEvents
)
{
  HTTPMediaTrackInfo trackInfo;
  MMI_PortMsgType vidPort;
  HTTPMediaType httpMediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
  QTV_NULL_PTR_CHECK(m_pHTTPDataInterface, RETURN_VOID);
  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMITrackHandler,RETURN_VOID);

  HttpSourceMmiPortInfo* pPortAddr = &m_portVideo[PortIndex];

  if((m_pHTTPSourceMMITrackHandler->GetOmxVideoMinorType(trackDescription.minorType) ==
      pPortAddr->m_portDef.format.video.eCompressionFormat) && (bSendPortEvents))
  {
    // If codec is same no need to send port config change event
    bSendPortEvents = false;
  }
  memset(&trackInfo, 0x0, sizeof(trackInfo));

  if (pPortAddr->m_pPortSpecificData)
  {
    QTV_Free(pPortAddr->m_pPortSpecificData);
    pPortAddr->m_pPortSpecificData = NULL;
  }

  pPortAddr->m_trackId = trackDescription.trackID;
  pPortAddr->m_isTrackValid = OMX_TRUE;
  SetPortSwitchingFlag(pPortAddr,false);

  vidPort.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX + PortIndex;
  pPortAddr->m_portDef.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX + PortIndex;
  pPortAddr->m_portDef.eDomain    = OMX_PortDomainVideo;

  pPortAddr->m_portDef.nBufferCountMin    = MMI_HTTP_VIDEO_PORT_MIN_BUFFER_COUNT;
  pPortAddr->m_portDef.nBufferCountActual = pPortAddr->m_portDef.nBufferCountMin;

  //Get the largest frame size in the video track
  pPortAddr->m_portDef.nBufferSize = GetMaxSupportedVideoBufferSize();
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "Video Port allocated Buffer Size %u",
      (uint32)pPortAddr->m_portDef.nBufferSize);

  //Update media track information
  pPortAddr->m_portDef.format.video.eCompressionFormat =
    m_pHTTPSourceMMITrackHandler->GetOmxVideoMinorType(trackDescription.minorType);
  HTTPCommon::MapFileSourceMajorTypeToHTTPMediaType( trackDescription.majorType,
                                                     httpMediaType );

  m_pHTTPDataInterface->GetSelectedMediaTrackInfo(httpMediaType, trackInfo);
  if(trackInfo.videoTrackInfo.frameWidth == 0 ||
        trackInfo.videoTrackInfo.frameHeight == 0)
  {
    trackInfo.videoTrackInfo.frameHeight = 240;
    trackInfo.videoTrackInfo.frameWidth  = 320;
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "Received 0 width/height from filesource. Resetting to QVGA");
  }
  pPortAddr->m_portDef.format.video.nFrameWidth  = trackInfo.videoTrackInfo.frameWidth;
  pPortAddr->m_portDef.format.video.nStride      = trackInfo.videoTrackInfo.frameWidth;
  pPortAddr->m_portDef.format.video.nFrameHeight = trackInfo.videoTrackInfo.frameHeight;
  pPortAddr->m_portDef.format.video.nSliceHeight = trackInfo.videoTrackInfo.frameHeight;
  pPortAddr->m_portDef.format.video.nBitrate     = trackInfo.videoTrackInfo.bitRate;
  pPortAddr->m_portDef.format.video.xFramerate   = (OMX_U32)trackInfo.videoTrackInfo.frameRate;

  // allocating more memory requird if DRM protected
  if (trackInfo.videoTrackInfo.drmInfo.isDRMProtected)
  {
    pPortAddr->m_portDef.nBufferSize += (OMX_U32)(MMI_HTTP_MAX_PORT_ENC_BUFFER_SIZE);
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
      "(DRM Protected)Video Port allocated Buffer Size %u",
      (uint32)pPortAddr->m_portDef.nBufferSize);
  }

  switch ((uint32)pPortAddr->m_portDef.format.video.eCompressionFormat)
  {
    case OMX_VIDEO_CodingAVC:
    {
      pPortAddr->m_pPortSpecificData =
        (OMX_VIDEO_PARAM_AVCTYPE*)QTV_Malloc(sizeof(OMX_VIDEO_PARAM_AVCTYPE));
      if (NULL == pPortAddr->m_pPortSpecificData )
      {
        break;
      }
      OMX_VIDEO_PARAM_AVCTYPE* pAVCParam =
        (OMX_VIDEO_PARAM_AVCTYPE*)pPortAddr->m_pPortSpecificData;
      QOMX_STRUCT_INIT(*pAVCParam, OMX_VIDEO_PARAM_AVCTYPE);
      pAVCParam->nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX + PortIndex;
    }
    break;
    case OMX_VIDEO_CodingMPEG2:
    {
      pPortAddr->m_pPortSpecificData =
        (OMX_VIDEO_PARAM_MPEG2TYPE*)QTV_Malloc(sizeof(OMX_VIDEO_PARAM_MPEG2TYPE));
      if (NULL == pPortAddr->m_pPortSpecificData )
      {
        break;
      }
      OMX_VIDEO_PARAM_MPEG2TYPE* pMPEG2Param =
        (OMX_VIDEO_PARAM_MPEG2TYPE*)pPortAddr->m_pPortSpecificData;
      QOMX_STRUCT_INIT(*pMPEG2Param, OMX_VIDEO_PARAM_MPEG2TYPE);
      pMPEG2Param->nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX + PortIndex;
    }
    break;
     case (OMX_VIDEO_CODINGTYPE)QOMX_OTHER_CodingHevc:
       QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
         "UpdateVideoPort for Hevc");
       break;

    default:
    {
      pPortAddr->m_portDef.format.video.eCompressionFormat =
        OMX_VIDEO_CodingUnused;
    }
    break;
  }
  return;
}

/*
 * Invalidates video port and send port changed, port detected notifications
 * to mmi.
 *
 * @param[in] PortIndex - port index to be updated
 * @param[in] bSendPortEvents - true if port config change
 */
void HTTPSourceMMI::InvalidateVideoPort
(
  OMX_U32 PortIndex,
  bool &bSendPortEvents
)
{
  MMI_PortMsgType vidPort;
  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMITrackHandler,RETURN_VOID);

  HttpSourceMmiPortInfo* pPortAddr = &m_portVideo[PortIndex];

  if(pPortAddr->m_portDef.format.video.eCompressionFormat ==
     OMX_VIDEO_CodingUnused )
  {
    //If codec is unused, not need to send port config event
    bSendPortEvents = false;
  }

  pPortAddr->m_trackId = 0;
  pPortAddr->m_isTrackValid = OMX_FALSE;
  SetPortSwitchingFlag(pPortAddr,false);
  if (pPortAddr->m_pPortSpecificData)
  {
    QTV_Free(pPortAddr->m_pPortSpecificData);
    pPortAddr->m_pPortSpecificData = NULL;
  }
  pPortAddr->m_portDef.format.video.cMIMEType = NULL;
  pPortAddr->m_portDef.format.video.pNativeRender = NULL;
  pPortAddr->m_portDef.format.video.nFrameWidth = 0;
  pPortAddr->m_portDef.format.video.nFrameHeight = 0;
  pPortAddr->m_portDef.format.video.nStride = 0;
  pPortAddr->m_portDef.format.video.nSliceHeight = 0;
  pPortAddr->m_portDef.format.video.nBitrate = 0;
  pPortAddr->m_portDef.format.video.xFramerate = 0;
  pPortAddr->m_portDef.format.video.bFlagErrorConcealment = OMX_TRUE;
  pPortAddr->m_portDef.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
  pPortAddr->m_portDef.format.video.eColorFormat = OMX_COLOR_FormatUnused;
  pPortAddr->m_portDef.format.video.pNativeWindow = NULL;
  pPortAddr->m_portDef.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX + PortIndex;
  pPortAddr->m_portDef.eDomain    = OMX_PortDomainVideo;
  pPortAddr->m_portDef.nBufferCountMin    = MMI_HTTP_VIDEO_PORT_MIN_BUFFER_COUNT;
  pPortAddr->m_portDef.nBufferCountActual = pPortAddr->m_portDef.nBufferCountMin;
  pPortAddr->m_portDef.nBufferSize = MMI_HTTP_VIDEO_PORT_MAX_BUFFER_SIZE;

  vidPort.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX + PortIndex;
  return;
}


/**@brief validating port
*
*@param[in] index - port index
            domain - port domain (video, audio, etc)
*@param[out]
*@return true - valid port, false - invalid port
*/
bool HTTPSourceMMI::IsValidPort(OMX_U32 index, OMX_PORTDOMAINTYPE domain)
{
  bool ret = false;

  ret = ((index >= MMI_HTTP_VIDEO_PORT_INDEX) &&
         (index<MMI_HTTP_VIDEO_PORT_INDEX + MMI_HTTP_NUM_VIDEO_PORTS) &&
         ((domain == OMX_PortDomainVideo) || (domain == OMX_PortDomainMax))) ||
        ((index>=MMI_HTTP_AUDIO_PORT_INDEX) &&
         (index<MMI_HTTP_AUDIO_PORT_INDEX + MMI_HTTP_NUM_AUDIO_PORTS) &&
         ((domain == OMX_PortDomainAudio) || (domain == OMX_PortDomainMax))) ||
        ((index>=MMI_HTTP_IMAGE_PORT_INDEX) &&
         (index<MMI_HTTP_IMAGE_PORT_INDEX + MMI_HTTP_NUM_IMAGE_PORTS) &&
         ((domain == OMX_PortDomainImage) || (domain == OMX_PortDomainMax))) ||
        (((index>=MMI_HTTP_OTHER_PORT_INDEX) &&
         (index<MMI_HTTP_OTHER_PORT_INDEX + MMI_HTTP_NUM_OTHER_PORTS)) &&
         ((domain == OMX_PortDomainOther) || (domain == OMX_PortDomainMax)));

  return ret;
}

/**@brief Detects end of stream on port
*@param[in] parameter - port index
*@param[out]
*@return true if eos detected, false otherwise
*/
bool HTTPSourceMMI::IsEndOfStream(int32 portIdx)
{
  bool ret = false;
  int32 targetIndex = 0;
  HttpSourceMmiPortInfo* pPortInfo = NULL;
  if(portIdx == MMI_HTTP_AUDIO_PORT_INDEX)
  {
    targetIndex = portIdx - MMI_HTTP_AUDIO_PORT_INDEX;
    pPortInfo = &m_portAudio[targetIndex];
  }
  else if(portIdx == MMI_HTTP_VIDEO_PORT_INDEX)
  {
    targetIndex = portIdx - MMI_HTTP_VIDEO_PORT_INDEX;
    pPortInfo = &m_portVideo[targetIndex];
  }
  else if(portIdx == MMI_HTTP_OTHER_PORT_INDEX)//for text
  {
    targetIndex = portIdx - MMI_HTTP_OTHER_PORT_INDEX;
    pPortInfo = &m_portOther[targetIndex];
  }
  else
  {
    return ret;
  }

  if (pPortInfo->m_nBufferFlags & OMX_BUFFERFLAG_EOS)
  {
    //eos already set
    ret = true;
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                 "Eos Detected on port %d", portIdx);
  }
  return ret;
}

/** @brief Creates http mmi.
*
* @param[in]
* @param[out]
* @return true or false depending on success
*/
bool HTTPSourceMMI::Create()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::Create" );
  bool result = true;

  m_pHTTPSourceMMIHelper = QTV_New_Args(HTTPSourceMMIHelper, (this));
  if (m_pHTTPSourceMMIHelper == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Memory allocation failed for HTTP stream MMI helper" );
    result = false;
  }

  if (result)
  {
    //Initialize HTTP streamer library
    if (!InitializeHTTPStreamer())
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: HTTP streamer library initialization failed" );
      result = false;
    }
  }

  if (result)
  {
    m_pHTTPSourceMMIPropertiesHandler = QTV_New_Args(HTTPSourceMMIPropertiesHandler,
                                                      (m_pHTTPController));
    if (m_pHTTPSourceMMIPropertiesHandler == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: Memory allocation failed for HTTP source MMI properties handler" );
      result = false;
    }
  }

  if (result)
  {
    m_pHTTPSourceMMIStreamPortHandler = QTV_New(HTTPSourceMMIStreamPortHandler);
    if (m_pHTTPSourceMMIStreamPortHandler == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: Memory allocation failed for HTTP source MMI stream port handler" );
      result = false;
    }
  }

  if (result)
  {
    m_pHTTPSourceMMITrackHandler = QTV_New_Args(HTTPSourceMMITrackHandler,
                                                (m_pHTTPController,
                                                 m_pHTTPSourceMMIPropertiesHandler,
                                                 result));
    if (m_pHTTPSourceMMITrackHandler == NULL || result == false)
    {
      QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Error: Create HTTP MMI track handler %p failed %d",
                     (void *)m_pHTTPSourceMMITrackHandler, result );
      result = false;
    }
  }

  if (result)
  {
    if (MM_CriticalSection_Create(&m_pHTTPSourceMMIDataLock))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Error: MMI data lock creation failed" );
      result = false;
    }
  }
  return result;
}

/** @brief Destroy all data structres of http mmi.
  *
  * @return void
  */
void HTTPSourceMMI::Destroy()
{
  int16 i = 0;

  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::~HTTPSourceMMI" );
  if(m_pHTTPDataReqHandler)
  {
    QTV_Delete(m_pHTTPDataReqHandler);
    m_pHTTPDataReqHandler = NULL;
  }
  //If HTTP streamer is not in uninitialized state post STOP
  if (IsHTTPStreamerInitialized())
  {
    //Send a close request (if needed)
    if (!IsClosePending())
    {
      (void)Close();
    }

    UninitializeHTTPStreamer();
  }

  //delete port specific data
  for (i=0; i<MMI_HTTP_NUM_VIDEO_PORTS; i++)
  {
    if (m_portVideo[i].m_pPortSpecificData)
    {
      QTV_Free(m_portVideo[i].m_pPortSpecificData);
      m_portVideo[i].m_pPortSpecificData = NULL;
    }
  }

  for (i=0; i<MMI_HTTP_NUM_AUDIO_PORTS; i++)
  {
    if (m_portAudio[i].m_pPortSpecificData)
    {
      QTV_Free(m_portAudio[i].m_pPortSpecificData);
      m_portAudio[i].m_pPortSpecificData = NULL;
    }
  }

  for (i=0; i<MMI_HTTP_NUM_IMAGE_PORTS; i++)
  {
    if (m_portImage[i].m_pPortSpecificData)
    {
      QTV_Free(m_portImage[i].m_pPortSpecificData);
      m_portImage[i].m_pPortSpecificData = NULL;
    }
  }

  for (i=0; i<MMI_HTTP_NUM_OTHER_PORTS; i++)
  {
    if (m_portOther[i].m_pPortSpecificData)
    {
      QTV_Free(m_portOther[i].m_pPortSpecificData);
      m_portOther[i].m_pPortSpecificData = NULL;
    }
  }

  if (m_pURL)
  {
    QTV_Delete(m_pURL);
    m_pURL = NULL;
  }

  if (m_pRole)
  {
    QTV_Delete(m_pRole);
    m_pRole = NULL;
  }

  if (m_pHTTPSourceMMITrackHandler)
  {
    QTV_Delete(m_pHTTPSourceMMITrackHandler);
    m_pHTTPSourceMMITrackHandler = NULL;
  }

  if (m_pHTTPSourceMMIStreamPortHandler)
  {
    QTV_Delete(m_pHTTPSourceMMIStreamPortHandler);
    m_pHTTPSourceMMIStreamPortHandler = NULL;
  }

  if (m_pHTTPSourceMMIPropertiesHandler)
  {
    QTV_Delete(m_pHTTPSourceMMIPropertiesHandler);
    m_pHTTPSourceMMIPropertiesHandler = NULL;
  }

  if (m_pHTTPSourceMMIHelper)
  {
    QTV_Delete(m_pHTTPSourceMMIHelper);
    m_pHTTPSourceMMIHelper = NULL;
  }

  m_pHTTPDataInterface = NULL;

  if (m_pHTTPSourceMMIDataLock)
  {
    (void)MM_CriticalSection_Release(m_pHTTPSourceMMIDataLock);
    m_pHTTPSourceMMIDataLock = NULL;
  }

  if(m_pStopPhrase)
  {
    QTV_Free(m_pStopPhrase);
    m_pStopPhrase = NULL;
  }

  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "HTTPSourceMMI destruction complete" );

  return;
}

/** @brief Initializes the HTTP streamer library.
  *
  * @return
  * TRUE - Initialization successful
  * FALSE - Otherwise
  */
bool HTTPSourceMMI::InitializeHTTPStreamer()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::InitializeHTTPStreamer" );

  bool bOk = false;

  m_pHTTPController = QTV_New_Args(HTTPController,
    (static_cast<iHTTPNotificationHandler *> (m_pHTTPSourceMMIHelper),
     static_cast<HTTPStatusHandlerInterface *> (m_pHTTPSourceMMIHelper),
     bOk));

  if ((m_pHTTPController == NULL) || !bOk)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: HTTP controller initialization failed" );
    return bOk;
  }
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "HTTP controller initialized" );

  bOk = m_pHTTPSourceMMIExtensionHandler.Initialize(this);
  if(!bOk)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: HTTP Extension Handler initialization failed" );
  }

  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "HTTP controller initialized" );
  SetHTTPStreamerInitialized(true);

  return bOk;
}

/** @brief Cleans up the HTTP streamer library.
  *
  */
void HTTPSourceMMI::UninitializeHTTPStreamer()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::UninitializeHTTPStreamer" );

  if (m_pHTTPController)
  {
    QTV_Delete(m_pHTTPController);
    m_pHTTPController = NULL;
  }

  SetHTTPStreamerInitialized(false);
}

/** @brief Initialize data interface (if needed).
  *
  * @return
  * TRUE - Open initialization successful
  * FALSE - Otherwise
  */
bool HTTPSourceMMI::InitOpen()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::InitOpen" );
  bool bOk = false;

  SetDataInterface(NULL);
  m_pHTTPSourceMMITrackHandler->SetDataInterface(NULL);

  //Check if config file exists and if yes try setting session properties
  //from it. Default file name is HTTPPropertiesConfig.cfg and here is a
  //sample of how the entries should look.
  /***********
    <Property Key1> = <Property Value1>
    <Property Key2> = <Property Value2>
    ...
    E.g. HTTP_PROPERTY_DATA_STORAGE = FILE_SYSTEM
         HTTP_PROPERTY_CONTENT_SAVE_LOCATION = ./TestClip.3gp
         HTTP_PROPERTY_DISABLE_BROKEN_DOWNLOAD = 0
         HTTP_PROPERTY_INIT_PREROLL_MSEC = 5000
  ***********/
  if (m_pHTTPSourceMMIPropertiesHandler)
  {
    bOk = m_pHTTPSourceMMIPropertiesHandler->SetPropertiesFromConfigFile();
  }

  return bOk;
}

/** @brief Open HTTP session with the specified URL. This method is used
  * for the DOWNLOAD_ONLY mode.
  *
  * @param[in] pURL - Reference to the URL to be opened
  *
  * @return
  * TRUE - Job accepted successfully
  * FALSE - Otherwise
  */
bool HTTPSourceMMI::OpenSession
(
 const char* pURL
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPSourceMMI::OpenSession" );
  if (m_pURL)
  {
    *m_pURL = pURL;
  }
  return (Open(NULL) == MMI_S_PENDING ? true : false);
}

/**
  * @brief set close pending status.
  *
  * @return void
  */
void HTTPSourceMMI::SetClosePending(const bool bClosePending)
{
 (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
  m_bClosePending = bClosePending;
 (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);

};
void HTTPSourceMMI::SetShutDownInProgress(const bool bIsShutDownInProgress)
{
  (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
  m_bIsShutdownInProgress = bIsShutDownInProgress;
  (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);
}
/**
  * @brief queries close pending status.
  *
  * @return close pedning status
  */
bool HTTPSourceMMI::IsClosePending() const
{
  bool bClosePending;
  (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
  bClosePending = m_bClosePending;
  (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);

  return bClosePending;
};
bool HTTPSourceMMI::IsShutDownInProgress() const
{
  bool bIsShutDownInProgress;
  (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
  bIsShutDownInProgress = m_bIsShutdownInProgress;
  (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);

  return bIsShutDownInProgress;
};

/**
  * @brief set streamer to intialized .
  *
  * @return void
  */
void HTTPSourceMMI::SetHTTPStreamerInitialized(const bool bInitialized)
{
  m_bHTTPStreamerInitialized = bInitialized;
};

/**
  * @brief Queries whether streamer intialized or not.
  *
  * @return streamer intialization status
  * TRUE - intialized
  * FALSE - Otherwise
  */
bool HTTPSourceMMI::IsHTTPStreamerInitialized() const
{
  return m_bHTTPStreamerInitialized;
};

/**
  * @brief Sets seek pending and also notifies HTTPSourceMMITrackHandler.
  *
  * @param[in] bSeekPending - SEEK pending flag
  *
  * @return
  * TRUE - Seek pending notified successfully
  * FALSE - Otherwise
  */
void HTTPSourceMMI::SetSeekPending
(
 const bool bSeekPending
)
{
  (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
  m_bSeekPending = bSeekPending;
  if (m_pHTTPSourceMMITrackHandler)
  {
    m_pHTTPSourceMMITrackHandler->SetSeekPending(bSeekPending);
  }
  (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);
  return;
}
bool HTTPSourceMMI::IsSeekPending()
{
  bool bSeekPending = false;
  (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
  bSeekPending = m_bSeekPending;
  (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);
  return bSeekPending;
}
void HTTPSourceMMI::SetPortSwitchingFlag
(
 HttpSourceMmiPortInfo* pPortInfo,
 const bool bPortSwitching
)
{
  QTV_NULL_PTR_CHECK(pPortInfo,RETURN_VOID);
  (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
  pPortInfo->m_bSwitching = bPortSwitching;
  (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);
  return;
}
bool HTTPSourceMMI::IsPortSwitching(HttpSourceMmiPortInfo* pPortInfo)
{
  bool bPortSwitching = false;
  (void)MM_CriticalSection_Enter(m_pHTTPSourceMMIDataLock);
  if(pPortInfo)
  {
    bPortSwitching = pPortInfo->m_bSwitching;
  }
  (void)MM_CriticalSection_Leave(m_pHTTPSourceMMIDataLock);
  return bPortSwitching;
}

/** @brief Check if repositioning is allowed.
  *
  * @param[in] timeToSeek - Seek start time (in microsec)
  * @return
  * TRUE - Reposition allowed
  * FALSE - Otherwise
  */
HTTPDownloadStatus HTTPSourceMMI::IsRepositioningAllowed
(
 const int64 timeToSeek
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  bool bOk = (timeToSeek >= 0);
  if (bOk)
  {
    //Check if user playback is allowed (valid check for FT, always TRUE for PD)
    if (!IsHTTPStreamerInitialized())
    {
      bOk = false;
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "HTTP streamer uninitialized" );
    }
    else
    {
      bOk = IsOpenComplete();
      if (bOk)
      {
          //Checking against clip duration
          HTTPCommon::HTTPAttrVal val;
          if(m_pHTTPDataInterface->GetConfig(HTTPCommon::HTTP_UNKNOWN_TYPE, HTTPCommon::HTTP_ATTR_REPOSITION_RANGE, val))
          {
            if (timeToSeek > (int64)val.sReposRange.nMax)
            {
              bOk = false;
              status = HTTPCommon::HTTPDL_UNSUPPORTED;
              QTV_MSG_PRIO2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                             "Seek %lld beyond reposition range %lld is not allowed",
                             timeToSeek, (int64)val.sReposRange.nMax );
            }
          }
          else
          {
            bOk = false;
            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "Clip reposition range or duration could not be obtained" );
        }
      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Seek is denied as open is not complete" );
      }
    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "Invalid seek input - timeToSeek %d (msec)",
                   (int)timeToSeek );
  }
  if(bOk)
  {
    status = HTTPCommon::HTTPDL_SUCCESS;
  }

  return status;
}

/**
 * @breif : This Method sets URL
 *
 * @param[in]  pointer to URL string
 *
 * @return NONE
 */
void HTTPSourceMMI::SetURL(const char* Url)
{
  // clear the URL if its already set for conformance test
  if(m_pURL)
  {
    QTV_Delete(m_pURL);
    m_pURL = NULL;
  }

  m_pURL = QTV_New_Args(OSCL_STRING,(Url));
}

/**
 * @breif: This Method sets the Start Time flag for Audio, Video
 *       &image ports when seek is successfull
 *
 * @param[in] NONE
 *
 * @return NONE
 */
void HTTPSourceMMI::SetStartTimeBufferFlag()
{
  int i = 0;
  HttpSourceMmiPortInfo *pPort = NULL;

  //First data buffer going out after seek/resume in livestreaming should carry STARTTIME flag.
  //Also clear the EOS flag to continue processing buffers for a seek after EOS
  for(i = 0;i<MMI_HTTP_NUM_VIDEO_PORTS;i++)
  {
    pPort = &m_portVideo[i];
    pPort->m_nBufferFlags |= OMX_BUFFERFLAG_STARTTIME;
    pPort->m_nBufferFlags &= ~OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_AUDIO_PORTS;i++)
  {
    pPort = &m_portAudio[i];
    pPort->m_nBufferFlags |= OMX_BUFFERFLAG_STARTTIME;
    pPort->m_nBufferFlags &= ~OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_IMAGE_PORTS;i++)
  {
    pPort = &m_portImage[i];
    pPort->m_nBufferFlags |= OMX_BUFFERFLAG_STARTTIME;
    pPort->m_nBufferFlags &= ~OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_OTHER_PORTS;i++)
  {
    pPort = &m_portOther[i];
    pPort->m_nBufferFlags |= OMX_BUFFERFLAG_STARTTIME;
    pPort->m_nBufferFlags &= ~OMX_BUFFERFLAG_EOS;
  }
}
/**
 * @breif: This Method sets the EOS flag for Audio, Video
 *       &image ports
 *
 * @param[in] NONE
 *
 * @return NONE
 */
void HTTPSourceMMI::SetEOSBufferFlag()
{
  int i = 0;
  HttpSourceMmiPortInfo *pPort = NULL;

  //Set EOS flag on all the ports
  for(i = 0;i<MMI_HTTP_NUM_VIDEO_PORTS;i++)
  {
    pPort = &m_portVideo[i];
    pPort->m_nBufferFlags |= OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_AUDIO_PORTS;i++)
  {
    pPort = &m_portAudio[i];
    pPort->m_nBufferFlags |= OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_IMAGE_PORTS;i++)
  {
    pPort = &m_portImage[i];
    pPort->m_nBufferFlags |= OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_OTHER_PORTS;i++)
  {
    pPort = &m_portOther[i];
    pPort->m_nBufferFlags |= OMX_BUFFERFLAG_EOS;
  }
}

/**
 * @breif: This Method clears the EOS flag for Audio, Video
 *       &image ports (used for Seek after EOS)
 *
 * @param[in] NONE
 *
 * @return NONE
 */
void HTTPSourceMMI::ClearEOSBufferFlag()
{
  int i = 0;
  HttpSourceMmiPortInfo *pPort = NULL;

  //Clear EOS flag on all the ports
  for(i = 0;i<MMI_HTTP_NUM_VIDEO_PORTS;i++)
  {
    pPort = &m_portVideo[i];
    pPort->m_nBufferFlags &= ~OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_AUDIO_PORTS;i++)
  {
    pPort = &m_portAudio[i];
    pPort->m_nBufferFlags &= ~OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_IMAGE_PORTS;i++)
  {
    pPort = &m_portImage[i];
    pPort->m_nBufferFlags &= ~OMX_BUFFERFLAG_EOS;
  }
  for(i = 0;i<MMI_HTTP_NUM_OTHER_PORTS;i++)
  {
    pPort = &m_portOther[i];
    pPort->m_nBufferFlags &= ~OMX_BUFFERFLAG_EOS;
  }
}

/**
 * @breif: gets controller interface
 *
 * @param[in] NONE
 *
 * @return HTTPController interface
 */
HTTPController* HTTPSourceMMI::GetController()
{
  return m_pHTTPController;
}

/* Updates Other port and send port changed, port detected notifications to mmi.
 *
 * @param[in] PortIndex - port index to be updated
 * @param[in] trackDescription - track description
 * @param[in] bSendPortEvents - true if port config change events should be
 *                              sent to IL client
 *
 * @return void
  */
void HTTPSourceMMI::UpdateOtherPort
(
  OMX_U32 PortIndex,
  HTTPSourceMMITrackHandler::TrackDescription &trackDescription,
  bool &bSendPortEvents
)
{
  HTTPMediaTrackInfo trackInfo;
  MMI_PortMsgType otherPort;
  HTTPMediaType httpMediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
  HttpSourceMmiPortInfo* pPortAddr = &m_portOther[PortIndex];
  QTV_NULL_PTR_CHECK(m_pHTTPDataInterface, RETURN_VOID);
  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMITrackHandler,RETURN_VOID);
  if(((OMX_OTHER_FORMATTYPE)m_pHTTPSourceMMITrackHandler->GetOmxOtherMinorType(trackDescription.minorType) ==
      pPortAddr->m_portDef.format.other.eFormat) && (bSendPortEvents))
  {
    //If codec is same there is no need to send port config change event
    bSendPortEvents = false;
  }
  if (pPortAddr->m_pPortSpecificData)
  {
    QTV_Free(pPortAddr->m_pPortSpecificData);
    pPortAddr->m_pPortSpecificData = NULL;
  }

  memset(&trackInfo, 0x0, sizeof(trackInfo));
  pPortAddr->m_trackId = trackDescription.trackID;
  pPortAddr->m_isTrackValid = OMX_TRUE;
  pPortAddr->m_bSwitching = false;

  otherPort.nPortIndex = MMI_HTTP_OTHER_PORT_INDEX + PortIndex;

  //Update port domain & port index
  pPortAddr->m_portDef.eDomain    = OMX_PortDomainOther;
  pPortAddr->m_portDef.nPortIndex = MMI_HTTP_OTHER_PORT_INDEX + PortIndex;

  //Update port buffer minimum requirement Set nBufferCountActual to nBufferCountMin,
  //nBufferCountActual can't be less than nBufferCountMin.
  pPortAddr->m_portDef.nBufferCountMin = MMI_HTTP_OTHER_PORT_MIN_BUFFER_COUNT;
  pPortAddr->m_portDef.nBufferCountActual = pPortAddr->m_portDef.nBufferCountMin;

  pPortAddr->m_portDef.nBufferSize = (uint32)trackDescription.maxSampleSize;
  if( pPortAddr->m_portDef.nBufferSize == 0)
  {
    pPortAddr->m_portDef.nBufferSize = MMI_HTTP_OTHER_PORT_DEFAULT_BUFFER_SIZE +
                                      (uint32)(MMI_HTTP_TEXT_EXTRADATA_BUFFER_SIZE);
  }

  //Get media track information & update the respective eCoding types
  HTTPCommon::MapFileSourceMajorTypeToHTTPMediaType( trackDescription.majorType,
                                                     httpMediaType );
  m_pHTTPDataInterface->GetSelectedMediaTrackInfo(httpMediaType, trackInfo);

  pPortAddr->m_portDef.format.other.eFormat =
    (OMX_OTHER_FORMATTYPE)m_pHTTPSourceMMITrackHandler->GetOmxOtherMinorType(trackDescription.minorType);

  if (pPortAddr->m_portDef.format.other.eFormat == (OMX_OTHER_FORMATTYPE)QOMX_OTHER_CodingSMPTETT)
  {
    // Do need to provide metdata ? not neccessary
  }
  else
  {
    pPortAddr->m_portDef.format.other.eFormat = (OMX_OTHER_FORMATTYPE)QOMX_OTHER_CodingUnused;
  }
}

/*
 * Invalidates Other port and send port changed, port detected notifications
 * to mmi.
 *
 * @param[in] PortIndex - port index to be updated
 * @param[in] bSendPortEvents - true if port config change
 */
void HTTPSourceMMI::InvalidateOtherPort
(
  OMX_U32 PortIndex,
  bool &bSendPortEvents
)
{
  MMI_PortMsgType OtherPort;
  QTV_NULL_PTR_CHECK(m_pHTTPSourceMMITrackHandler,RETURN_VOID);

  HttpSourceMmiPortInfo* pPortAddr = &m_portOther[PortIndex];

  if(pPortAddr->m_portDef.format.other.eFormat ==
     (OMX_OTHER_FORMATTYPE) QOMX_OTHER_CodingUnused )
  {
    //If codec is unused, not need to send port config event
    bSendPortEvents = false;
  }

  pPortAddr->m_trackId = 0;
  pPortAddr->m_isTrackValid = OMX_FALSE;
  pPortAddr->m_bSwitching = false;
  if (pPortAddr->m_pPortSpecificData)
  {
    QTV_Free(pPortAddr->m_pPortSpecificData);
    pPortAddr->m_pPortSpecificData = NULL;
  }
  pPortAddr->m_portDef.format.other.eFormat = (OMX_OTHER_FORMATTYPE)QOMX_OTHER_CodingUnused;
  pPortAddr->m_portDef.nPortIndex = MMI_HTTP_OTHER_PORT_INDEX + PortIndex;
  pPortAddr->m_portDef.eDomain    = OMX_PortDomainOther;
  pPortAddr->m_portDef.nBufferCountMin    = MMI_HTTP_OTHER_PORT_MIN_BUFFER_COUNT;
  pPortAddr->m_portDef.nBufferCountActual = pPortAddr->m_portDef.nBufferCountMin;
  pPortAddr->m_portDef.nBufferSize = MMI_HTTP_OTHER_PORT_DEFAULT_BUFFER_SIZE;

  OtherPort.nPortIndex = MMI_HTTP_OTHER_PORT_INDEX + PortIndex;

  return;
}

/*
 * check and Sends the Port config change event if port configuration is changed
 *
 *
 * @param[in] mediaType - Media type for port config change event
 * @param[in] PortIndex - port index to be updated
 * @param[in] bSendPortEvents - flag to indicate port config change
 */
void HTTPSourceMMI::CheckAndSendPortConfigChangeEvent
(
 HTTPCommon::HTTPMediaType mediaType,
 OMX_U32 portIndex,
 bool isSendPortConfig
)
{
  MMI_PortMsgType portType;
  if(isSendPortConfig)
  {
    if(HTTPCommon::HTTP_AUDIO_TYPE == mediaType)
    {
      portType.nPortIndex = portIndex + MMI_HTTP_AUDIO_PORT_INDEX;
    }
    else if(HTTPCommon::HTTP_VIDEO_TYPE == mediaType)
    {
      portType.nPortIndex = portIndex + MMI_HTTP_VIDEO_PORT_INDEX;
    }
    else if(HTTPCommon::HTTP_TEXT_TYPE == mediaType)
  {
      portType.nPortIndex = portIndex + MMI_HTTP_OTHER_PORT_INDEX;
    }

    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                   "sending MMI_EVT_PORT_CONFIG_CHANGED on media type %d ",
                   mediaType );

    //Notify omx base with event MMI_EVT_PORT_CONFIG_CHANGED for port settings change
    m_pHTTPSourceMMIHelper->NotifyMmi(MMI_EVT_PORT_CONFIG_CHANGED, MMI_S_COMPLETE,
                                      sizeof(MMI_PortMsgType), &portType,
                                      m_pClientData);
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "No Need to send MMI_EVT_PORT_CONFIG_CHANGED on media type %d ",
                   mediaType);
  }

  return;
}

void HTTPSourceMMI::SetSelectRepresentationsPending(bool flag)
{
  m_bIsSelectRepresentationsPending = flag;
}

bool HTTPSourceMMI::IsSelectRepresentationsPending() const
{
  return m_bIsSelectRepresentationsPending;
}

bool HTTPSourceMMI::CacheSelectedRepresentations(const char *str)
{
  bool rslt = false;
  if (m_pCachedSelectedRepresentations)
  {
    QTV_Free(m_pCachedSelectedRepresentations);
  }

  if (str)
  {
    size_t bufSize = 1 + std_strlen(str);
    m_pCachedSelectedRepresentations = (char *)QTV_Malloc(bufSize);
    if (m_pCachedSelectedRepresentations)
    {
      std_strlcpy(m_pCachedSelectedRepresentations, str, bufSize);
      rslt = true;
    }
  }

  return rslt;
}

const char *HTTPSourceMMI::GetCachedSelectedRepresentations() const
{
  return m_pCachedSelectedRepresentations;
}

void HTTPSourceMMI::ClearCachedSelectRepresentions()
{
  if (m_pCachedSelectedRepresentations)
  {
    QTV_Free(m_pCachedSelectedRepresentations);
    m_pCachedSelectedRepresentations = NULL;
  }
}

}/* namespace video */
