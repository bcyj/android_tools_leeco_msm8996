#ifndef __HTTPDATAREQUESTHANDLER_H__
#define __HTTPDATAREQUESTHANDLER__H__
/************************************************************************* */
/**
 * HTTPDataRequestHandler.h
 * @brief Header file for HTTPDataRequestHandler.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPDataRequestHandler.h#1 $
$DateTime: 2012/07/11 01:14:00 $
$Change: 2579632 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDataRequestHandler.h
** ======================================================================= */

#if defined (WINCE) || defined (WIN32)
#include <windows.h>
#endif // defined (WINCE) || defined (WIN32)

#include <MMThread.h>
#include <MMCriticalSection.h>
#include "MMTimer.h"
#include "StreamDataQueue.h"
#include "HTTPController.h"
#include "HTTPCommon.h"
#include "MMTimer.h"
#include "MMSignal.h"
#include "httpInternalDefs.h"
#include <OMX_Component.h>
#include "HTTPSourceMMITrackHandler.h"
namespace video {

/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
#define MAX_DATA_COMMANDS 20
#define DEFAULT_TIMER_DELAY 10
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */
class iHTTPDataNotificationHandler
{
public:
  virtual ~iHTTPDataNotificationHandler() {};

  virtual void NotifyHTTPEvent(const HTTPDataCommand HTTPCommand,
                               const HTTPDownloadStatus HTTPStatus,
                               void* pCbData) = 0;
  virtual int NotifyDataEvent(const HTTPDataCommand /*HTTPCommand*/,
                                int32 /*portIdx*/, void * /*pBuffHdrParam*/){return -1;};
};


class HTTPDataRequestHandler
{
public:
  HTTPDataRequestHandler(iHTTPDataNotificationHandler* phttpnotification,bool& bOk);
  virtual ~HTTPDataRequestHandler();
  //This method queues the FTB command. In running state FTB signal is set so that
  //FTBs can be processed by thread
  bool DataRequest(int32 portIdx, void* pBuffHdr);
  //This method sets the flush signal. When thread receives the flush signal it returns
  //all pending FTBs
  bool DataFlush(int32 portIdx);
  //Sets the pause signal. When thread receives pause signal it modifies the state to paused
  void Pause();
  //Sets the resume signal. When thread receives resume signal it modifies the state from paused to running
  void Resume();
  //Sets the exit signal
  void Close();

private:
  static int HTTPDataRequestThreadEntryFunction(void* pTaskParam);
  bool StartDataRequestThread();
  void DataRequestThread();
  static void BufferingCheckTimerCallBack(void *arg);
  //Iterates on all ports and processes FTB commands
  int ProcessAllDataCmds();
  int ProcessDataCmds(const int nPort);
  //Clears data cmdq for given port. bNotify is true if the flush request comes from upper layer.
  //If cmdq is cleared as part of close,bNotify is false so that flush notification is not given.
  int ClearDataCmdQ(const int nPort);
  void ProcessPause();
  void ProcessResume();
  void Reset();
  uint32 MapPortIndexToSignal(const int nPortIndex);
  int MapSignalToPortIndex(const uint32 nSignal);

  //HTTPDataRequestHandler states
  enum HTTPDataRequestState
  {
    HTTP_DATA_REQUEST_STATE_IDLE,
    HTTP_DATA_REQUEST_STATE_PAUSED,
    HTTP_DATA_REQUEST_STATE_RUNNING
  };
  //State handlers
  class HTTPDataRequestBaseStateHandler
  {
  public:
    HTTPDataRequestBaseStateHandler(HTTPDataRequestState eState,
                                   HTTPDataRequestHandler* pDataReqHdlr)
      : m_eState(eState),
        m_pDataReqHandler(pDataReqHdlr)
    {
    };

    virtual ~HTTPDataRequestBaseStateHandler()
    {
    };
    virtual bool DataRequest(uint32 portIdx, void* pBuffHdr);
    virtual bool DataFlush(int32 portIdx);
    virtual void Pause();
    virtual void Resume();
    virtual void Close();
    virtual int ProcessAllDataCmds()
    {
      return -1;
    }
    virtual void BufferingCheckTimerCallBack()
    {
      return;
    }
    virtual void Reset();
    virtual void ProcessPause()
    {
      return;
    }
    virtual void ProcessResume()
    {
      return;
    }

   protected:
    HTTPDataRequestState m_eState;
    HTTPDataRequestHandler* m_pDataReqHandler;
  };
  class HTTPDataRequestIdleStateHandler:public HTTPDataRequestBaseStateHandler
  {
  public:
    public:
    HTTPDataRequestIdleStateHandler(HTTPDataRequestHandler* pDataReqHdlr)
      : HTTPDataRequestBaseStateHandler(HTTP_DATA_REQUEST_STATE_IDLE, pDataReqHdlr)
    {
    };

    virtual ~HTTPDataRequestIdleStateHandler()
    {
    };
    virtual bool DataRequest(uint32 /*portIdx*/, void* /*pBuffHdr*/)
    {
      return false;
    }
    virtual bool DataFlush(int32 /*portIdx*/)
    {
      return false;
    }
    virtual void Pause()
    {
      return;
    }
    virtual void Resume()
    {
      return;
    }
    virtual void Close()
    {
      return;
    }
  };

  class HTTPDataRequestPausedStateHandler:public HTTPDataRequestBaseStateHandler
  {
  public:
    public:
    HTTPDataRequestPausedStateHandler(HTTPDataRequestHandler* pDataReqHdlr)
      : HTTPDataRequestBaseStateHandler(HTTP_DATA_REQUEST_STATE_PAUSED, pDataReqHdlr)
    {
    };

    virtual ~HTTPDataRequestPausedStateHandler()
    {
    };
    virtual void ProcessResume();

  };
  class HTTPDataRequestRunningStateHandler:public HTTPDataRequestBaseStateHandler
  {
  public:

    HTTPDataRequestRunningStateHandler(HTTPDataRequestHandler* pDataReqHdlr)
      : HTTPDataRequestBaseStateHandler(HTTP_DATA_REQUEST_STATE_RUNNING, pDataReqHdlr)
    {
    };

    virtual ~HTTPDataRequestRunningStateHandler()
    {
    }
    virtual bool DataRequest(uint32 portIdx, void* pBuffHdr);
    virtual int ProcessAllDataCmds();
    virtual void BufferingCheckTimerCallBack();
    virtual void ProcessPause();

  };

  void SetStateHandler(HTTPDataRequestBaseStateHandler* pCurrStateHandler)
  {
    MM_CriticalSection_Enter(m_pDataRequestLock);
    m_pCurrentStateHandler = pCurrStateHandler;
    MM_CriticalSection_Leave(m_pDataRequestLock);
  }

  HTTPDataRequestBaseStateHandler* GetCurrentStateHandler()
  {
    HTTPDataRequestBaseStateHandler* pCurrStateHandler = NULL;
    MM_CriticalSection_Enter(m_pDataRequestLock);
    pCurrStateHandler = m_pCurrentStateHandler;
    MM_CriticalSection_Leave(m_pDataRequestLock);
    return pCurrStateHandler;
  }

  static const uint32 PROCESS_PAUSE_EVENT ;
  static const uint32 PROCESS_RESUME_EVENT;
  static const uint32 PROCESS_FLUSH_SIGNALS[];
  static const uint32 PROCESS_EXIT_EVENT;
  static const uint32 PROCESS_FTB_EVENT;
  // Signal to process FTB commands on thread
  MM_HANDLE m_pProcessFTBSignal;
  //Signal to process Pause command
  MM_HANDLE m_pProcessPauseSignal;
  //Signal to process Resume command
  MM_HANDLE m_pProcessResumeSignal;
  //Signal to process Close command
  MM_HANDLE m_pProcessExitSignal;

   //The signal Q for the HTTP Data Request thread to wait on.
  MM_HANDLE m_pSignalQ;
  //Timer to set the FTB signal if FTB can not be processed due to buffering
  MM_HANDLE mhTimer;
  //Thread handle
  MM_HANDLE m_pHTTPDataRequestThread;
  MM_HANDLE m_pDataRequestLock;

  //Added typedef HTTPCommon::HTTPControllerCommand HTTPDataCommand; in HTTPCommon.h
  struct DataRequestCmdData
  {
    HTTPDataCommand eCmd;
    int32 nPort;
    void *pBufHdr;
  };
  typedef struct HTTPDataInfo
  {
    //CmdQ for queueing FTB commands
    StreamDataQ<DataRequestCmdData> m_cDataCmdQ;
    // Signal to process Flush commands on thread
    MM_HANDLE m_pProcessFlushSignal;
  }HTTPDataInfo;
  HTTPDataRequestIdleStateHandler m_cIdleStateHandler;
  HTTPDataRequestPausedStateHandler m_cPausedStateHandler;
  HTTPDataRequestRunningStateHandler m_cRunningStateHandler;

  HTTPDataRequestBaseStateHandler* m_pCurrentStateHandler;

  HTTPDataInfo m_cDataInfo[MMI_HTTP_NUM_MAX_PORTS];

  iHTTPDataNotificationHandler *m_pHTTPSourceNotificationHandler;
};
}/* namespace video */
#endif /* __HTTPDATAREQUESTHANDLER_H__ */