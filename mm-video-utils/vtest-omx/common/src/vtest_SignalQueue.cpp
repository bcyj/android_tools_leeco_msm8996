/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential

Copyright (c) 2010 The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#include "vtest_Debug.h"
#include "vtest_Mutex.h"
#include "vtest_Queue.h"
#include "vtest_Signal.h"
#include "vtest_SignalQueue.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
SignalQueue::SignalQueue(OMX_S32 nMaxQueueSize, OMX_S32 nMaxDataSize)
    :  m_pSignal(new Signal()),
      m_pMutex(new Mutex()),
      m_pQueue(new Queue(nMaxQueueSize, nMaxDataSize)),
      m_bBroadcast(OMX_FALSE) {

    VTEST_MSG_LOW("constructor %u %u", (unsigned int)nMaxQueueSize, (unsigned int)nMaxDataSize);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
SignalQueue::~SignalQueue() {

    VTEST_MSG_LOW("destructor");
    if (m_pMutex != NULL) delete m_pMutex;
    if (m_pSignal != NULL) delete m_pSignal;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE SignalQueue::Pop(OMX_PTR pData,
        OMX_S32 nDataSize, OMX_S32 nTimeoutMillis) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    // wait for signal or for data to come into queue
    while (GetSize() == 0 && result == OMX_ErrorNone) {
        result = m_pSignal->Wait(nTimeoutMillis);
        /* The Signal class and vt_signal.c is not written properly,
         * there is a condition in which wait can come out even
         * without anyone calling set. Hence there is a while loop
         * here to avoid that condition. Fixing of that would require
         * squashing all the layers into one with a single lock.*/
        if (m_bBroadcast) {
            result = OMX_ErrorTimeout;
            m_bBroadcast = OMX_FALSE;
        }
    }

    // did we timeout?
    if (result == OMX_ErrorNone) {
        // lock mutex
        m_pMutex->Lock();

        result = m_pQueue->Pop(pData, nDataSize);

        // unlock mutex
        m_pMutex->UnLock();
    } else if (result != OMX_ErrorTimeout) {
        VTEST_MSG_ERROR("Error waiting for signal");
        result = OMX_ErrorUndefined;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE SignalQueue::Push(OMX_PTR pData, OMX_S32 nDataSize) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    // lock mutex
    m_pMutex->Lock();

    result = m_pQueue->Push(pData, nDataSize);

    // unlock mutex
    m_pMutex->UnLock();


    if (result == OMX_ErrorNone) {
        m_pSignal->Set();
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE SignalQueue::Peek(OMX_PTR pData, OMX_S32 nDataSize) {

    VTEST_MSG_LOW("Peek");
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
OMX_S32 SignalQueue::GetSize() {

    return m_pQueue->GetSize();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_S32 SignalQueue::WakeAll() {

    m_bBroadcast = OMX_TRUE;
    return m_pSignal->Broadcast();
}

} // namespace vtest
