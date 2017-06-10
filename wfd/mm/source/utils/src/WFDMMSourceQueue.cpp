/* =======================================================================
                              WFDMMSourceQueue.cpp
DESCRIPTION

Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/utils/src/WFDMMSourceQueue.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$

========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include "WFDMMSourceQueue.h"
#include "wfd_util_queue.h"
#include "MMDebugMsg.h"

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Queue::Queue()
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"default constructor should not be here (private)");
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Queue::Queue(OMX_S32 nMaxQueueSize,
      OMX_S32 nMaxDataSize)
    :  m_pHandle(NULL)
  {
    MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_MEDIUM,"constructor %ld %ld", nMaxQueueSize, nMaxDataSize);
    if (venc_queue_create((void**) &m_pHandle, (int) nMaxQueueSize, (int) nMaxDataSize) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"failed to create queue");
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Queue::~Queue()
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_MEDIUM,"destructor");
    if (venc_queue_destroy((void*) m_pHandle) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"failed to delete queue");
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Queue::Pop(OMX_PTR pData,
      OMX_S32 nDataSize)
  {
    OMX_ERRORTYPE result = Peek(pData, nDataSize);

    if (result == OMX_ErrorNone)
    {
      if (venc_queue_pop((void*) m_pHandle, pData, (int) nDataSize) != 0)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"failed to pop queue");
        result = OMX_ErrorUndefined;
      }
    }

    return result;

  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Queue::Push(OMX_PTR pData,
      OMX_S32 nDataSize)
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (venc_queue_push((void*) m_pHandle, (void*) pData, (int) nDataSize) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"failed to push onto queue");
      result = OMX_ErrorUndefined;
    }

    return result;
  }

  OMX_ERRORTYPE Queue::Peek(OMX_PTR pData,
      OMX_S32 nDataSize)
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (venc_queue_peek((void*) m_pHandle, (void*) pData, (int) nDataSize) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"failed to peek into queue");
      result = OMX_ErrorUndefined;
    }

    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_S32 Queue::GetSize()
  {
    return (OMX_S32) venc_queue_size((void*) m_pHandle);
  }


