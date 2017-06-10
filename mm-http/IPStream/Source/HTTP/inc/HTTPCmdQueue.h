#ifndef HTTP_CMD_QUEUE_H_
#define HTTP_CMD_QUEUE_H_
/************************************************************************* */
/**
 * HTTPCmdQueue.h
 * @brief Header file for HTTPCmdQueue.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPCmdQueue.h#10 $
$DateTime: 2013/05/12 09:54:28 $
$Change: 3751774 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPCmdQueue.h
** ======================================================================= */
#include "MMCriticalSection.h"
#include "StreamQueue.h"
#include "HTTPCommon.h"
#include "Url.h"

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
#define HTTP_CONTROLLER_CMD_BUFFER_COUNT 20

  /* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPController;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
struct ControllerBaseCmd
{
  StreamQ_link_type m_link;
  HTTPControllerCommand m_cmdType;
  void *m_pUserData;
};

struct ControllerOpenCmd : public ControllerBaseCmd
{
  URL* m_pURN;
  void* m_pPlaybackHandler;
};

struct ControllerSeekCmd : public ControllerBaseCmd
{
  int64 m_nSeekTime;
};

struct SetTrackStateCmd : public ControllerBaseCmd
{
  int32 m_trackIndex;
  bool m_bSelected;
};

struct FlushRequestCmd : public ControllerBaseCmd
{
  int32 m_portId;
  bool m_bNotify;
};

struct DataRequestCmd : public ControllerBaseCmd
{
  int32 m_portId;
};

struct NotifyWaterMarkEventCmd : public ControllerBaseCmd
{
  uint32 m_portIdxAndWatermarkType;
};

struct SetAuthenticationCmd : public ControllerBaseCmd
{
  char *m_AuthHeader;
  char *m_AuthValue;
};

struct SelectRepresentationsCmd : public ControllerBaseCmd
{
  char *m_pSelectionsXML;
};

union ControllerCmd
{
  ControllerBaseCmd m_baseCmd;
  ControllerOpenCmd m_openCmd;
  ControllerSeekCmd m_seekCmd;
  SetTrackStateCmd m_setTrackStateCmd;
  FlushRequestCmd m_flushCmd;
  DataRequestCmd m_dataRequestCmd;
  NotifyWaterMarkEventCmd m_notifyWatermarkEventCmd;
  SetAuthenticationCmd m_setAuthenticationCmd;
  SelectRepresentationsCmd m_SelectRepresentationsCmd;
};

class HttpCmdQueue
{
public:
  HttpCmdQueue();
  virtual ~HttpCmdQueue();

  ControllerCmd *GetCmd(HTTPControllerCommand cmdType, void *pUserData);
  void QueueCmd(ControllerCmd *pControllerCmd);
  virtual int32 ProcessAllCmds(HTTPController *pSelf) = 0;

protected:
  ControllerCmd m_cmdBuffers[HTTP_CONTROLLER_CMD_BUFFER_COUNT];
  StreamQ_type m_cmdQueue;     // Queue of active commands that need to be processed
  StreamQ_type m_cmdFreeQueue; // Queue for storing free command buffers
  MM_HANDLE m_pCmdLock;
};

class HTTPCtrlCmdQueue: public HttpCmdQueue
{
public:
  HTTPCtrlCmdQueue();
  virtual ~HTTPCtrlCmdQueue();

  virtual int32 ProcessAllCmds(HTTPController *pSelf);
  void FlushCmds();
};


} //namespace
#endif
