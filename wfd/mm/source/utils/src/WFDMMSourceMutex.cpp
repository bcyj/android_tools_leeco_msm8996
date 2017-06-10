/* =======================================================================
                             WFDMMSourceMutex.cpp
DESCRIPTION

Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/utils/src/WFDMMSourceMutex.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$

========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include "WFDMMSourceMutex.h"
#include "WFDMMSourceMutex.h"
#include "wfd_util_mutex.h"
#include "MMDebugMsg.h"



  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Mutex::Mutex() :
    m_pMutex(NULL)
  {
    if (venc_mutex_create(&m_pMutex) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "failed to init mutex");
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Mutex::~Mutex()
  {
    if (venc_mutex_destroy(m_pMutex) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"failed to destroy mutex");
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Mutex::Lock()
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    if (venc_mutex_lock(m_pMutex) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"failed to lock mutex");
      result = OMX_ErrorUndefined;
    }
    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Mutex::UnLock()
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    if (venc_mutex_unlock(m_pMutex) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"failed to unlock mutex");
      result = OMX_ErrorUndefined;
    }
    return result;
  }

