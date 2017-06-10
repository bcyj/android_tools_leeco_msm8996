/* =======================================================================
                              WFDMMSourceSignalQueue.cpp
DESCRIPTION

Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/utils/src/WFDMMSourceSignalQueue.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$
========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include "WFDMMSourceMutex.h"
#include "WFDMMSourceQueue.h"
#include "WFDMMSourceSignal.h"
#include "WFDMMSourceSignalQueue.h"
#include "MMDebugMsg.h"

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  SignalQueue::SignalQueue()
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"default constructor should not be here (private)");
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  SignalQueue::SignalQueue(OMX_S32 nMaxQueueSize,
      OMX_S32 nMaxDataSize)
    :  m_pSignal(new Signal()),
    m_pMutex(new Mutex()),
    m_pQueue(new Queue(nMaxQueueSize, nMaxDataSize))
  {
    MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_MEDIUM,"constructor %ld %ld", nMaxQueueSize, nMaxDataSize);
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  SignalQueue::~SignalQueue()
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_MEDIUM,"destructor");
    if (m_pMutex != NULL)
      delete m_pMutex;
    if (m_pSignal != NULL)
      delete m_pSignal;
    if (m_pQueue)
      delete m_pQueue;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE SignalQueue::Pop(OMX_PTR pData,
      OMX_S32 nDataSize,
      OMX_S32 nTimeoutMillis)
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;

    // wait for signal or for data to come into queue
    while ( (GetSize() == 0) && (result == OMX_ErrorNone) && (nTimeoutMillis > 0) )
    {
      result = m_pSignal->Wait(nTimeoutMillis);
    }

    // did we timeout?
    if (result == OMX_ErrorNone)
    {
      // lock mutex
      m_pMutex->Lock();

      result = m_pQueue->Pop(pData, nDataSize);

      // unlock mutex
      m_pMutex->UnLock();
    }
    else if (result != OMX_ErrorTimeout)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Error waiting for signal");
      result = OMX_ErrorUndefined;
    }

    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE SignalQueue::Push(OMX_PTR pData,
      OMX_S32 nDataSize)
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;

    // lock mutex
    m_pMutex->Lock();

    result = m_pQueue->Push(pData, nDataSize);

    // unlock mutex
    m_pMutex->UnLock();


    if (result == OMX_ErrorNone)
    {
      m_pSignal->Set();
    }

    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE SignalQueue::Peek(OMX_PTR pData,
      OMX_S32 nDataSize)
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_LOW,"Peek");
    OMX_ERRORTYPE result = OMX_ErrorNone;

    // lock mutex
    m_pMutex->Lock();

    result = m_pQueue->Peek(pData, nDataSize);

    // unlock mutex
    m_pMutex->UnLock();

    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_S32 SignalQueue::GetSize()
  {
    return m_pQueue->GetSize();
  }
