/************************************************************************* */
/**
 * HTTPCmdQueue.cpp
 * @brief implementation of HTTPCmdQueue.
 *  HTTPCmdQueue provides controll and data command queue functionality
 *  for the HTTP module.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPCmdQueue.cpp#11 $
$DateTime: 2013/05/12 09:54:28 $
$Change: 3751774 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPCmdQueue.cpp
** ======================================================================= */
#include "HTTPCmdQueue.h"
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

  /* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief inititalizes the command queues
  *
  */
HttpCmdQueue::HttpCmdQueue()
{
  m_pCmdLock = NULL;
  // Initialize the command queue and the free command queue, and link the
  // command items onto the free command queue.
  (void)StreamQ_init( &m_cmdQueue );
  (void)StreamQ_init( &m_cmdFreeQueue );

  for( int i = 0; i < HTTP_CONTROLLER_CMD_BUFFER_COUNT; i++ )
  {
    (void)StreamQ_link( &m_cmdBuffers[i], &m_cmdBuffers[i].m_baseCmd.m_link );
    StreamQ_put( &m_cmdFreeQueue, &m_cmdBuffers[i].m_baseCmd.m_link );
  }
 MM_CriticalSection_Create(&m_pCmdLock);

}

/** @brief destructor of the command queues
  *
  */
HttpCmdQueue::~HttpCmdQueue()
{
  if(m_pCmdLock)
  {
    MM_CriticalSection_Release(m_pCmdLock);
    m_pCmdLock = NULL;
  }

}

/** @brief gets a free command buffer
  * @param[in] cmdType - type of the command
  * @param[in] pUserData - command data
  * @return Returns a pointer to a command buffer, if available,
  * or NULL if no command buffers are available
  */
ControllerCmd *HttpCmdQueue::GetCmd(HTTPControllerCommand cmdType, void *pUserData)
{
  ControllerCmd *pControllerCmd;

  MM_CriticalSection_Enter(m_pCmdLock);

  // Get a command buffer from the free command queue.
  pControllerCmd = (ControllerCmd *)StreamQ_get( &m_cmdFreeQueue );
  if( pControllerCmd == NULL )
  {
    // No free command buffers available, log an error.
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "No items on Controller cmd queue" );
  }
  else
  {
    // populate the m_baseCmd parameters and initialize all the others to zero
    // pControllerCmd->m_baseCmd.m_link is ini'ted at the begining
    pControllerCmd->m_baseCmd.m_cmdType = cmdType;
    pControllerCmd->m_baseCmd.m_pUserData = pUserData;
    memset(((byte *)pControllerCmd) + sizeof(ControllerBaseCmd), 0,
           sizeof(ControllerCmd) - sizeof(ControllerBaseCmd));
  }
  MM_CriticalSection_Leave(m_pCmdLock);

  return pControllerCmd;
}

/** @brief puts the command into command queue
  *
  * The caller of this function should have previously allocated a command
  * by calling GetCmd().
  *
  * @param[in] The command that needs to be queued
  */
void HttpCmdQueue::QueueCmd(ControllerCmd *pControllerCmd)
{
  MM_CriticalSection_Enter(m_pCmdLock);
  // Get a command buffer from the free command queue.
  if ( pControllerCmd == NULL )
  {
    // No free command buffers available, log an error.
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "NULL command to queue in Controller cmd queue" );
  }
  else
  {
    StreamQ_put( &m_cmdQueue, &pControllerCmd->m_baseCmd.m_link );
  }
  MM_CriticalSection_Leave(m_pCmdLock);

}


/** @brief inititalizes the control command queue
  *
  */
HTTPCtrlCmdQueue::HTTPCtrlCmdQueue()
{
}

/** @brief destructs the control command queue
  *
  */
HTTPCtrlCmdQueue::~HTTPCtrlCmdQueue()
{
}

/** @brief de-queues and process the control command command queue
  *
  * The Commands are de-queued until the control command queue is empty.
  * @param[in] pSelf - Self pointer of object on which the commands processed
  * @return status code
  */
int32 HTTPCtrlCmdQueue::ProcessAllCmds(HTTPController *pSelf)
{
  ControllerCmd  *pControllerCmd = NULL;
  HTTPController *pHttpController = pSelf;
  HTTPControllerCommand cmd = HTTPCommon::CMD_MAX;
  int32 ret = -1;

  QTV_NULL_PTR_CHECK(pHttpController, ret);

  // Get commands from the command queue until the queue is empty. For each
  // command received, execute the right functionality within Controller.
  MM_CriticalSection_Enter(m_pCmdLock);
  int nCommandCount  = StreamQ_cnt(&m_cmdQueue);
  MM_CriticalSection_Leave(m_pCmdLock);

  while (nCommandCount > 0)
  {
    ControllerCmd  *pControllerCmd = NULL;
    MM_CriticalSection_Enter(m_pCmdLock);
    pControllerCmd = (ControllerCmd *)StreamQ_get( &m_cmdQueue );
    MM_CriticalSection_Leave(m_pCmdLock);

    if(NULL == pControllerCmd)
    {
      break;
    }

    cmd = pControllerCmd->m_baseCmd.m_cmdType;

    switch(cmd)
    {
    case HTTPCommon::OPEN:
      pHttpController->ExecuteOpen(pControllerCmd->m_openCmd.m_pURN,
        pControllerCmd->m_openCmd.m_pPlaybackHandler,
        pControllerCmd->m_openCmd.m_pUserData);
      break;
    case HTTPCommon::CLOSE:
      pHttpController->ExecuteClose(pControllerCmd->m_baseCmd.m_pUserData);
      break;
    case HTTPCommon::START:
      pHttpController->ExecuteStart(pControllerCmd->m_baseCmd.m_pUserData);
      break;
    case HTTPCommon::STOP:
      pHttpController->ExecuteStop(pControllerCmd->m_baseCmd.m_pUserData);
      break;
    case HTTPCommon::PLAY:
      pHttpController->ExecutePlay(pControllerCmd->m_baseCmd.m_pUserData);
      break;
    case HTTPCommon::PAUSE:
      pHttpController->ExecutePause(pControllerCmd->m_baseCmd.m_pUserData);
      break;
    case HTTPCommon::DOWNLOAD:
      pHttpController->ExecuteDownload(pControllerCmd->m_baseCmd.m_pUserData);
      break;
    case HTTPCommon::SEEK:
      pHttpController->ExecuteSeek(pControllerCmd->m_seekCmd.m_nSeekTime,
        pControllerCmd->m_seekCmd.m_pUserData);
      break;
    case HTTPCommon::GET_TRACKS:
      pHttpController->ExecuteGetTracks(pControllerCmd->m_baseCmd.m_pUserData);
      break;
    case HTTPCommon::SET_TRACK_STATE:
      pHttpController->ExecuteSetTrackState(pControllerCmd->m_setTrackStateCmd.m_trackIndex,
        pControllerCmd->m_setTrackStateCmd.m_bSelected,
        pControllerCmd->m_setTrackStateCmd.m_pUserData);
      break;
    case HTTPCommon::WAIT_FOR_RESOURCES:
      pHttpController->ExecuteWaitForResources(pControllerCmd->m_baseCmd.m_pUserData);
      break;
    case HTTPCommon::NOTIFY_CURRENT_WATERMARK_STATUS:
      pHttpController->ExecuteNotifyWaterMarkStatus(
        pControllerCmd->m_notifyWatermarkEventCmd.m_portIdxAndWatermarkType);
      break;
    case HTTPCommon::SET_AUTHORIZATION:
      pHttpController->ExecuteSetAuthorization(
        pControllerCmd->m_setAuthenticationCmd.m_AuthHeader,
        pControllerCmd->m_setAuthenticationCmd.m_AuthValue);

      if (pControllerCmd->m_setAuthenticationCmd.m_AuthHeader)
      {
        QTV_Free(pControllerCmd->m_setAuthenticationCmd.m_AuthHeader);
        pControllerCmd->m_setAuthenticationCmd.m_AuthHeader = NULL;
      }

      if (pControllerCmd->m_setAuthenticationCmd.m_AuthValue)
      {
        QTV_Free(pControllerCmd->m_setAuthenticationCmd.m_AuthValue);
        pControllerCmd->m_setAuthenticationCmd.m_AuthValue = NULL;
      }
      break;
    case HTTPCommon::SELECT_REPRESENTATIONS:
      pHttpController->ExecuteSelectRepresentations(
         pControllerCmd->m_SelectRepresentationsCmd.m_pSelectionsXML);
      if (pControllerCmd->m_SelectRepresentationsCmd.m_pSelectionsXML)
      {
        QTV_Free (pControllerCmd->m_SelectRepresentationsCmd.m_pSelectionsXML);
        pControllerCmd->m_SelectRepresentationsCmd.m_pSelectionsXML = NULL;
      }
      break;
    default:
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "Unknown streamer command %d", cmd);
      break;
    }

    // Return the command buffer to the free command queue.
    MM_CriticalSection_Enter(m_pCmdLock);
    StreamQ_put( &m_cmdFreeQueue, &pControllerCmd->m_baseCmd.m_link );
    MM_CriticalSection_Leave(m_pCmdLock);

    nCommandCount--;
  }

  return 0;
}

/** @brief flushs any pending commands in control command queue
  * @param[in] pSelf - Self pointer of object on which the commands processed
  */
void HTTPCtrlCmdQueue::FlushCmds()
{
  ControllerCmd  *pControllerCmd = NULL;
  MM_CriticalSection_Enter(m_pCmdLock);
  // Get commands from the command queue until the queue is empty.
  while( (pControllerCmd = (ControllerCmd *)StreamQ_get( &m_cmdQueue )) != NULL )
  {
    if (pControllerCmd->m_baseCmd.m_cmdType == HTTPCommon::OPEN)
    {
      QTV_Delete(pControllerCmd->m_openCmd.m_pURN);
    }

    // Return the command buffer to the free command queue.
    StreamQ_put( &m_cmdFreeQueue, &pControllerCmd->m_baseCmd.m_link );
  }
  MM_CriticalSection_Leave(m_pCmdLock);
}

}  //namespace
