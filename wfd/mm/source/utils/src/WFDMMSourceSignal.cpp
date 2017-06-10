/* =======================================================================
                              WFDMMSourceSignal.cpp
DESCRIPTION

Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/src/WFDMMSourceSignal.cpp

========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include "WFDMMSourceSignal.h"
#include "WFDMMSourceSignal.h"
#include "wfd_util_signal.h"
#include "MMDebugMsg.h"
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Signal::Signal()
    : m_pSignal(NULL)
  {
    if (venc_signal_create(&m_pSignal) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"error creating signal");
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Signal::~Signal()
  {
    if (venc_signal_destroy(m_pSignal) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"error destroying signal");
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Signal::Set()
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    if (venc_signal_set(m_pSignal) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"error setting signal");
      result = OMX_ErrorUndefined;
    }
    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Signal::Wait(OMX_S32 nTimeoutMillis)
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;

    int ret = venc_signal_wait(m_pSignal, (int) nTimeoutMillis);

    if (ret == 2)
    {
      result = OMX_ErrorTimeout;
    }
    else if (ret != 0)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"error waiting for signal");
      result = OMX_ErrorUndefined;
    }

    return result;
  }


