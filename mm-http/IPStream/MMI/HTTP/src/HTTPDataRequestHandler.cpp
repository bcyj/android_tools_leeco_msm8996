/************************************************************************* */
/**
 * HTTPDataRequestHandler.cpp
 * @brief Implementation of HTTPDataRequestHandler.
 *  HTTPDataRequestHandler class handles the processing of data request commands.
 *  It creates a seperate thread for processing of fillthisbuffer commands.
 *
 * COPYRIGHT 2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPDataRequestHandler.cpp#6 $
$DateTime: 2013/05/31 18:11:40 $
$Change: 3851777 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDataRequestHandler.cpp
** ======================================================================= */

#include "HTTPDataRequestHandler.h"
namespace video{
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
const uint32 HTTPDataRequestHandler::PROCESS_PAUSE_EVENT = 0;
const uint32 HTTPDataRequestHandler::PROCESS_RESUME_EVENT = 1;
const uint32 HTTPDataRequestHandler::PROCESS_FLUSH_SIGNALS[] ={2,3,4,5};
const uint32 HTTPDataRequestHandler::PROCESS_EXIT_EVENT = 6;
const uint32 HTTPDataRequestHandler::PROCESS_FTB_EVENT = 7;
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
/** @brief HTTPDataRequestHandler Constructor.
  *
  * @param[in] pHTTPNotification - HTTP stream notification handler
  * @param[out] bOk - Status of initialization
  */
HTTPDataRequestHandler::HTTPDataRequestHandler
(
 iHTTPDataNotificationHandler* pHTTPNotification,
 bool& bOk
 ) : m_pProcessFTBSignal(NULL),
   m_pProcessPauseSignal(NULL),
   m_pProcessResumeSignal(NULL),
   m_pProcessExitSignal(NULL),
   m_pSignalQ(NULL),
   mhTimer(NULL),
   m_pHTTPDataRequestThread(NULL),
   m_pDataRequestLock(NULL),
   m_cIdleStateHandler(this),
   m_cPausedStateHandler(this),
   m_cRunningStateHandler(this),
   m_pCurrentStateHandler(NULL),
   m_pHTTPSourceNotificationHandler(pHTTPNotification)
{
  //Start with idle state
  SetStateHandler(&m_cIdleStateHandler);

  //Create data lock
  bOk = (MM_CriticalSection_Create(&m_pDataRequestLock) == 0);

  //Initialize cmd queue for queuing FTB calls
  if (bOk)
  {
    for(int i = 0; i <  MMI_HTTP_NUM_MAX_PORTS; i++)
    {
      bOk = (m_cDataInfo[i].m_cDataCmdQ).Init(MAX_DATA_COMMANDS);
      m_cDataInfo[i].m_pProcessFlushSignal = NULL;
    }
  }

  bOk = StartDataRequestThread();
}
/** @brief HTTPDataRequestHandler destructor
*
*/
 HTTPDataRequestHandler::~HTTPDataRequestHandler()
 {
   QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "~HTTPDataRequestHandler start");

   if(m_pHTTPDataRequestThread)
   {
     Close();
     int exitCode = 0;
     MM_Thread_Join( m_pHTTPDataRequestThread, &exitCode );
     MM_Thread_Release( m_pHTTPDataRequestThread );
     m_pHTTPDataRequestThread = NULL;
   }

   if(mhTimer)
   {
     MM_Timer_Stop(mhTimer);
     MM_Timer_Release(mhTimer);
     mhTimer = NULL;
   }

   if(m_pDataRequestLock)
   {
     MM_CriticalSection_Release(m_pDataRequestLock);
     m_pDataRequestLock = NULL;
   }

   if(m_pProcessPauseSignal)
   {
     MM_Signal_Release(m_pProcessPauseSignal);
     m_pProcessPauseSignal = NULL;
   }

   if(m_pProcessResumeSignal)
   {
     MM_Signal_Release(m_pProcessResumeSignal);
     m_pProcessResumeSignal = NULL;
   }

   if(m_pProcessExitSignal)
   {
     MM_Signal_Release(m_pProcessExitSignal);
     m_pProcessExitSignal = NULL;
   }

   if(m_pProcessFTBSignal)
   {
     MM_Signal_Release(m_pProcessFTBSignal);
     m_pProcessFTBSignal = NULL;
   }

   for(int index = 0; index < MMI_HTTP_NUM_MAX_PORTS; index++)
   {
     if(m_cDataInfo[index].m_pProcessFlushSignal)
     {
       MM_Signal_Release(m_cDataInfo[index].m_pProcessFlushSignal);
       m_cDataInfo[index].m_pProcessFlushSignal = NULL;
     }
   }

   if(m_pSignalQ)
   {
     MM_SignalQ_Release(m_pSignalQ);
     m_pSignalQ = NULL;
   }

   QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "~HTTPDataRequestHandler end");
 }

/** @brief Creates and starts the HTTPDataRequest thread
*
* @result
* TRUE - Successfully started thread
* FALSE - Otherwise
*/
bool HTTPDataRequestHandler::StartDataRequestThread()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPDataRequestHandler::StartDataRequestThread" );

  bool bOk = true;
  //Release earlier instance of HTTPDataRequestThread  (if any)
  if (m_pHTTPDataRequestThread)
  {
    (void)MM_Thread_Release(m_pHTTPDataRequestThread);
    m_pHTTPDataRequestThread = NULL;
  }
  // Create the signal Q for the thread to wait on.
  if( ( 0 != MM_SignalQ_Create( &m_pSignalQ ) ) )
  {
    bOk = false;
  }

  //Create all signals(FTB,flush,pause,resume,exit)
  //for which signal Q will wait.
  //Please note that order of creation of signals decides the priority
  //For eg  Pause signal is created before resume, So if both pause and resume
  //are set on signalQ,MM_SignalQ_Wait will first return pause signal
  if(bOk)
  {
    if (( 0 != MM_Signal_Create( m_pSignalQ,
                                (void *) &PROCESS_PAUSE_EVENT,
                                NULL,
                                &m_pProcessPauseSignal ) ) )
    {
      bOk = false;
    }

    if (bOk)
    {
      if(( 0 != MM_Signal_Create( m_pSignalQ,
                                 (void *) &PROCESS_RESUME_EVENT,
                                 NULL,
                                 &m_pProcessResumeSignal ) ) )
      {
        bOk = false;
      }
      if(bOk)
      {
        for(int index = 0; index < MMI_HTTP_NUM_MAX_PORTS; index++)
        {
          if (( 0 != MM_Signal_Create( m_pSignalQ,
                                   (void *) &PROCESS_FLUSH_SIGNALS[index],
                                   NULL,
                                   &(m_cDataInfo[index].m_pProcessFlushSignal) ) ) )
          {
            bOk = false;
            break;
          }
        }
        if(bOk)
        {

          if ( ( 0 != MM_Signal_Create( m_pSignalQ,
                                 (void *) &PROCESS_EXIT_EVENT,
                                  NULL,
                                  &m_pProcessExitSignal ) ) )
          {
            bOk = false;
          }
          if(bOk)
          {
            if ( ( 0 != MM_Signal_Create( m_pSignalQ,
                                  (void *) &PROCESS_FTB_EVENT,
                                  NULL,
                                  &m_pProcessFTBSignal ) ) )
            {
              bOk = false;
            }
            if(bOk)
            {
              //Create the timer to set FTB signal back in case of buffering
              if ((0 != MM_Timer_CreateEx(0, BufferingCheckTimerCallBack, this, &mhTimer)))
              {
                QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                              "Unable to start timer" );
                bOk =false;
              }

              QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                            "Creating HTTP DataRequest thread" );
              if(bOk)
              {
                if ((MM_Thread_CreateEx( HTTP_STREAMER_THREAD_PRIORITY, 0, HTTPDataRequestThreadEntryFunction,
                                        (void *) (this), HTTP_STREAMER_STACK_SIZE, "HTTPDataRequest",
                                         &m_pHTTPDataRequestThread) == 0))
                {
                  bOk = true;
                }
                else
                {
                  bOk = false;
                  QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                                "Error: HTTPDataRequestThread creation failed" );
                }
              }
            }
          }
        }
      }
    }
  }

  if(bOk)
  {
    //Move to Running state on successful thread creation.
    SetStateHandler(&m_cRunningStateHandler);
  }

  return bOk;
}
/** @brief Entry point routine for HTTP DataRequest thread.
*
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/
int HTTPDataRequestHandler::HTTPDataRequestThreadEntryFunction
(
 void* arg
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPDataRequestHandler::HTTPDataRequestThreadEntryFunction" );
  HTTPDataRequestHandler* pSelf = (HTTPDataRequestHandler*) arg;
  int result = -1;
  if(pSelf)
  {
    result = 0;
    pSelf->DataRequestThread();
  }
  return result;
}

/** @brief HTTPDataRequestThread function which waits for events
*   and processes them.
*
* @return
*/
void HTTPDataRequestHandler::DataRequestThread()
{
  bool bRunning = true;
  while ( bRunning )
  {
    // Wait for a signal to be set on the signal Q.
    uint32 *pEvent = NULL;

    if ( 0 == MM_SignalQ_Wait( m_pSignalQ, (void **) &pEvent ) )
    {
      if( PROCESS_FTB_EVENT == *pEvent )
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "HTTPDataRequest thread received Fillthisbuffer signal" );
        ProcessAllDataCmds();
      }

      else if(PROCESS_FLUSH_SIGNALS[0] <= *pEvent &&
              PROCESS_FLUSH_SIGNALS[MMI_HTTP_NUM_MAX_PORTS-1] >= *pEvent)
      {
        int nPortIndex = MapSignalToPortIndex(*pEvent);
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "HTTPDataRequest thread received Flush signal for %d port",nPortIndex );
        ClearDataCmdQ(nPortIndex - MMI_HTTP_PORT_START_INDEX);
        //Notify flush response
        if(m_pHTTPSourceNotificationHandler)
        {
          m_pHTTPSourceNotificationHandler->NotifyHTTPEvent(HTTPCommon::FLUSH,HTTPCommon::HTTPDL_SUCCESS,(void*)&nPortIndex);
        }
      }

      else if(PROCESS_PAUSE_EVENT == *pEvent)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "HTTPDataRequest thread received Pause request" );
        ProcessPause();
      }

      else if( PROCESS_RESUME_EVENT == *pEvent )
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                      "HTTPDataRequest thread received Resume request" );
        ProcessResume();
      }

      else if( PROCESS_EXIT_EVENT == *pEvent )
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPDataRequest thread received Close request" );
        SetStateHandler(&m_cIdleStateHandler);
        Reset();
        bRunning = false;
      }

      else
      {
        // Not a recognized event, ignore it.
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPDataRequest thread received an unknown event" );

      }
    }
    else
    {
      bRunning = false;
    }
  }
  //Exit the HTTP Data Request thread
  if (m_pHTTPDataRequestThread)
  {
    MM_Thread_Exit(m_pHTTPDataRequestThread, 0);
  }
}
/** @brief Queue up FTB command
*
* @param[in] portIdx - port ID to which this FTB belongs
* @param[in] pBuffHdr - Reference to callback data/buffer header
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPDataRequestHandler::DataRequest
(
 int32 portIdx,
 void* pBuffHdr
 )
{
  bool result = false;
  HTTPDataRequestBaseStateHandler* pCurStateHandler = GetCurrentStateHandler();
  if(pCurStateHandler)
  {
    result = pCurStateHandler->DataRequest(portIdx,pBuffHdr);
  }
  return result;
}
/** @brief Queue up FTB command
*
* @param[in] portIdx - port ID to which this FTB belongs
* @param[in] pBuffHdr - Reference to callback data/buffer header
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPDataRequestHandler::HTTPDataRequestBaseStateHandler::DataRequest
(
 uint32 portIdx,
 void* pBuffHdr
 )
{
  bool result= false;
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,result);
  DataRequestCmdData cmd;
  cmd.eCmd = HTTPCommon::DATA_REQUEST;
  cmd.nPort = portIdx;
  cmd.pBufHdr = pBuffHdr;
  if(m_pDataReqHandler->m_cDataInfo[portIdx - MMI_HTTP_PORT_START_INDEX].m_cDataCmdQ.EnQ(cmd))
  {
    QTV_MSG_PRIO1( QTVDIAG_GENERAL, QTVDIAG_PRIO_MEDIUM,
                  "Data request queued for port %d",(int)portIdx );
    result = true;
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                   "Error in queuing data request for port %d",
                   (int)portIdx );
  }
  return result;
}
/** @brief Queue up FTB command and sets FTB signal
*
* @param[in] portIdx - port ID to which this FTB belongs
* @param[in] pBuffHdr - Reference to callback data/buffer header
* @return
* TRUE - Command successfully queued
* FALSE - Otherwise
*/
bool HTTPDataRequestHandler::HTTPDataRequestRunningStateHandler::DataRequest
(
 uint32 portIdx,
 void* pBuffHdr
)
{
  bool result = false;
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,result);
  result = HTTPDataRequestBaseStateHandler::DataRequest(portIdx,pBuffHdr);
  MM_Signal_Set(m_pDataReqHandler->m_pProcessFTBSignal);
  return result;
}
/** @brief HTTPDataRequestHandler - DataFlush
* Sets flush signal
*
* @return
* TRUE - Flush signal set successfully
* FALSE - Otherwise
*/
bool HTTPDataRequestHandler::DataFlush
(
  int32 portIdx
)
{
  bool result = true;
  HTTPDataRequestBaseStateHandler* pCurStateHandler = GetCurrentStateHandler();
  if(pCurStateHandler)
  {
    pCurStateHandler->DataFlush(portIdx);
  }

  return result;
}
/** @brief HTTPDataRequestHandler - DataFlush
* Sets flush signal
*
* @return
* TRUE - Flush signal set successfully
* FALSE - Otherwise
*/
bool HTTPDataRequestHandler::HTTPDataRequestBaseStateHandler::DataFlush
(
 int32 portIdx
)
{
  bool result = false;
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,result);
  result = true;
  QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
                "HTTPDataRequestHandler::Flush - Flush Signal is set");
  if(portIdx < 0)
  {
    for(int i=0; i < MMI_HTTP_NUM_MAX_PORTS ; i++)
    {
      MM_Signal_Set(m_pDataReqHandler->m_cDataInfo[i].m_pProcessFlushSignal);
    }
  }
  else
  {
    if(portIdx >= MMI_HTTP_PORT_START_INDEX)
    {
      MM_Signal_Set(m_pDataReqHandler->m_cDataInfo[portIdx - MMI_HTTP_PORT_START_INDEX].m_pProcessFlushSignal);
    }
  }

  return result;
}
/** @brief HTTPDataRequestHandler - Close
* Sets exit signal
*
* @return - void
*/
void HTTPDataRequestHandler::Close()
{
  HTTPDataRequestBaseStateHandler* pCurStateHandler = GetCurrentStateHandler();
  if(pCurStateHandler)
  {
    pCurStateHandler->Close();
  }
  return;
}
/** @brief HTTPDataRequestHandler - Close
* Sets exit signal
*
* @return - void
*/
void HTTPDataRequestHandler::HTTPDataRequestBaseStateHandler::Close()
{
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,RETURN_VOID);
  QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
               "HTTPDataRequestHandler::Close - Exit Signal is set");
  MM_Signal_Set(m_pDataReqHandler->m_pProcessExitSignal);
  return;
}
/** @brief HTTPDataRequestHandler - Pause
* Sets pause signal
*
* @return - void
*/
void HTTPDataRequestHandler::Pause()
{
  HTTPDataRequestBaseStateHandler* pCurStateHandler= GetCurrentStateHandler();
  if(pCurStateHandler)
  {
    pCurStateHandler->Pause();
  }
  return;
}
/** @brief HTTPDataRequestHandler - Pause
* Sets pause signal
*
* @return - void
*/
void HTTPDataRequestHandler::HTTPDataRequestBaseStateHandler::Pause()
{
   QTV_NULL_PTR_CHECK(m_pDataReqHandler,RETURN_VOID);
   QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_MEDIUM,
     "HTTPDataRequestHandler::HTTPDataRequestBaseStateHandler::Pause - "
     "Pause signal is Set");
   MM_Signal_Set(m_pDataReqHandler->m_pProcessPauseSignal);
   return;
}
/** @brief HTTPDataRequestHandler - Resume
* Sets resume signal
*
* @return - void
*/
void HTTPDataRequestHandler::Resume()
{
  HTTPDataRequestBaseStateHandler* pCurStateHandler = GetCurrentStateHandler();
  if(pCurStateHandler)
  {
    pCurStateHandler->Resume();
  }
  return;
}
/** @brief HTTPDataRequestHandler - Resume
* Sets resume signal
*
* @return - void
*/
void HTTPDataRequestHandler::HTTPDataRequestBaseStateHandler::Resume()
{
   QTV_NULL_PTR_CHECK(m_pDataReqHandler,RETURN_VOID);
   QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_MEDIUM,
               "HTTPDataRequestHandler::HTTPDataRequestBaseStateHandler::Resume - "
               "Resume signal is Set");
   MM_Signal_Set(m_pDataReqHandler->m_pProcessResumeSignal);
   return;
}
/** @brief HTTPDataRequestHandler - ProcessAllDataCmds
* Processes FTB cmds queued in datacmdqs of all ports
*
* @return - -1 if FTB returns pending 0 otherwise
*/
int HTTPDataRequestHandler::ProcessAllDataCmds()
{
  int ret = -1;
  HTTPDataRequestBaseStateHandler* pCurStateHandler = GetCurrentStateHandler();
  if(pCurStateHandler)
  {
    ret = pCurStateHandler->ProcessAllDataCmds();
  }
  return ret;
}
/** @brief HTTPDataRequestHandler - ProcessAllDataCmds
* Processes FTB cmds queued in datacmdqs of all ports
*
* @return - -1 if FTB returns pending 0 otherwise
*/
int HTTPDataRequestHandler::HTTPDataRequestRunningStateHandler::ProcessAllDataCmds()
{
  int ret = -1;
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,ret);
  for(int i = 0; i < MMI_HTTP_NUM_MAX_PORTS ;i++)
  {
    m_pDataReqHandler->ProcessDataCmds(i);
  }
  return 0;
}
/** @brief HTTPDataRequestHandler - ProcessDataCmds
* Processes FTB cmds queued in datacmdqs of giveb port
* @param - nPort - Port index
* @return - -1 if FTB returns pending 0 otherwise
*/
int HTTPDataRequestHandler::ProcessDataCmds(const int nPort)
{
  int ret = -1;
  DataRequestCmdData cmd;
  void *pIt = NULL;
  uint32 nCmdCount = m_cDataInfo[nPort].m_cDataCmdQ.Count();
  while(nCmdCount > 0 && m_cDataInfo[nPort].m_cDataCmdQ.Next(pIt, cmd))
  {
    if (cmd.eCmd == HTTPCommon::DATA_REQUEST)
    {
       if (m_pHTTPSourceNotificationHandler)
       {
         ret = m_pHTTPSourceNotificationHandler->NotifyDataEvent(HTTPCommon::DATA_REQUEST,
                                                                 cmd.nPort,
                                                                 cmd.pBufHdr);
       }
       //FTB returned pending restart the timer
       if(ret == -1)
       {
         MM_Timer_Stop(mhTimer);
         MM_Timer_Start(mhTimer,DEFAULT_TIMER_DELAY);
         break;
       }
    }
    (void)m_cDataInfo[nPort].m_cDataCmdQ.Remove(pIt);
    nCmdCount--;
  }
  return ret;
}
/** @brief HTTPDataRequestHandler - ClearDataCmdQ
* Clears datacmdq for given port and returns all buffers
* back to client
* @param - nPort - Port index
* @return -  0 if successful -1 otherwise
*/
int HTTPDataRequestHandler::ClearDataCmdQ(const int nPort)
{
  DataRequestCmdData cmd;
  void *pIt = NULL;
  int ret = 0;
  //Clear all cmds for the given port
  while(m_cDataInfo[nPort].m_cDataCmdQ.Next(pIt, cmd))
  {
    if (cmd.eCmd == HTTPCommon::DATA_REQUEST)
    {
       if (m_pHTTPSourceNotificationHandler)
       {
         ret = m_pHTTPSourceNotificationHandler->NotifyDataEvent(HTTPCommon::FLUSH,
                                                                 cmd.nPort,
                                                                 cmd.pBufHdr);
       }
    }
   (void)m_cDataInfo[nPort].m_cDataCmdQ.Remove(pIt);
  }
  m_cDataInfo[nPort].m_cDataCmdQ.Reset();
  return ret;
}
/** @brief HTTPDataRequestHandler - ProcessPause
* Processes pause signal and modifies the state
* @return -  void
*/
void HTTPDataRequestHandler::ProcessPause()
{
  if(m_pCurrentStateHandler)
  {
    m_pCurrentStateHandler->ProcessPause();
  }
}
/** @brief HTTPDataRequestHandler - ProcessPause
* Processes pause signal and modifies the state
* @return -  void
*/
void HTTPDataRequestHandler::HTTPDataRequestRunningStateHandler::ProcessPause()
{
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,RETURN_VOID);
  QTV_MSG_PRIO( QTVDIAG_GENERAL, QTVDIAG_PRIO_MEDIUM,
               "HTTPDataRequestHandler::HTTPDataRequestRunningStateHandler::"
               "ProcessPause - State is updated to Paused" );
  m_pDataReqHandler->SetStateHandler(&m_pDataReqHandler->m_cPausedStateHandler);
  return;
}
/** @brief HTTPDataRequestHandler - ProcessResume
* Processes resume signal and modifies the state
* @return -  void
*/
void HTTPDataRequestHandler::ProcessResume()
{
  if(m_pCurrentStateHandler)
  {
    m_pCurrentStateHandler->ProcessResume();
  }
  return;
}
/** @brief HTTPDataRequestHandler - ProcessResume
* Processes resume signal and modifies the state
* @return -  void
*/
void HTTPDataRequestHandler::HTTPDataRequestPausedStateHandler::ProcessResume()
{
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,RETURN_VOID);
  QTV_MSG_PRIO( QTVDIAG_GENERAL, QTVDIAG_PRIO_MEDIUM,
               "HTTPDataRequestHandler::HTTPDataRequestPausedStateHandler::"
               "ProcessResume - State is updated to Running" );
  m_pDataReqHandler->SetStateHandler(&m_pDataReqHandler->m_cRunningStateHandler);
  MM_Signal_Set(m_pDataReqHandler->m_pProcessFTBSignal);
  return;
}
/** @brief HTTPDataRequestHandler - Reset
* Clears the data cmdq, resets all signals and
* stop the timer
* @return -  void
*/
void HTTPDataRequestHandler::Reset()
{
  if(m_pCurrentStateHandler)
  {
    m_pCurrentStateHandler->Reset();
  }
  return;
}
/** @brief HTTPDataRequestHandler - Reset
* Clears the data cmdq, resets all signals and
* stop the timer
* @return -  void
*/
void HTTPDataRequestHandler::HTTPDataRequestBaseStateHandler::Reset()
{
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,RETURN_VOID);

  for(int i = 0; i < MMI_HTTP_NUM_MAX_PORTS; i++)
  {
    m_pDataReqHandler->ClearDataCmdQ(i);
    MM_Signal_Reset(m_pDataReqHandler->m_cDataInfo[i].m_pProcessFlushSignal);
  }
  MM_Signal_Reset(m_pDataReqHandler->m_pProcessFTBSignal);
  MM_Signal_Reset(m_pDataReqHandler->m_pProcessPauseSignal);
  MM_Signal_Reset(m_pDataReqHandler->m_pProcessResumeSignal);
  MM_Signal_Reset(m_pDataReqHandler->m_pProcessExitSignal);
  MM_Timer_Stop(m_pDataReqHandler->mhTimer);

}
/** @brief Timer expiry callback. Sets the FTB signal on thread
*
* @return -  void
*/
void HTTPDataRequestHandler::BufferingCheckTimerCallBack(void *arg)
{
  HTTPDataRequestHandler* pSelf = (HTTPDataRequestHandler*)arg;

  if(pSelf && pSelf->m_pCurrentStateHandler)
  {
    pSelf->m_pCurrentStateHandler->BufferingCheckTimerCallBack();
  }
  return;
}
/** @brief Timer expiry callback. Sets the FTB signal on thread
*
* @return -  void
*/
void HTTPDataRequestHandler::HTTPDataRequestRunningStateHandler::BufferingCheckTimerCallBack()
{
  QTV_NULL_PTR_CHECK(m_pDataReqHandler,RETURN_VOID);
  QTV_MSG_PRIO( QTVDIAG_GENERAL, QTVDIAG_PRIO_MEDIUM,
                "HTTPDataRequestHandler::HTTPDataRequestRunningStateHandler"
                "::BufferingCheckTimerCallBack()" );
  //Set the FTB signal so that we can check if track is out of buffering
  MM_Signal_Set(m_pDataReqHandler->m_pProcessFTBSignal);
}
uint32 HTTPDataRequestHandler::MapPortIndexToSignal(const int nPortIndex)
{
  uint32 nSignal = (uint32)(-1);
  if(nPortIndex >= MMI_HTTP_PORT_START_INDEX)
  {
    nSignal = PROCESS_FLUSH_SIGNALS[nPortIndex - MMI_HTTP_PORT_START_INDEX];
  }
  return nSignal;
}
int HTTPDataRequestHandler::MapSignalToPortIndex(uint32 nSignal)
{
  int nPortIndex = -1;
  for(int index = 0; index < MMI_HTTP_NUM_MAX_PORTS; index++)
  {
    if(PROCESS_FLUSH_SIGNALS[index] == nSignal)
    {
      nPortIndex = index + MMI_HTTP_PORT_START_INDEX;
      break;
    }
  }
  return nPortIndex;
}

};
