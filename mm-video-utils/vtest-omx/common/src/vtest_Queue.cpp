/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "vtest_Queue.h"
#include "vt_queue.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Queue::Queue() {
    VTEST_MSG_ERROR("default constructor should not be here (private)");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Queue::Queue(OMX_S32 nMaxQueueSize,
             OMX_S32 nMaxDataSize)
    :  m_pHandle(NULL) {
    VTEST_MSG_LOW("constructor %u %u", (unsigned int)nMaxQueueSize, (unsigned int)nMaxDataSize);
    if (vt_queue_create((void **)&m_pHandle, (int)nMaxQueueSize, (int)nMaxDataSize) != 0) {
        VTEST_MSG_ERROR("failed to create queue");
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Queue::~Queue() {
    VTEST_MSG_LOW("destructor");
    if (vt_queue_destroy((void *)m_pHandle) != 0) {
        VTEST_MSG_ERROR("failed to create queue");
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Queue::Pop(OMX_PTR pData,
                         OMX_S32 nDataSize) {
//    VTEST_MSG_LOW("Pop");
    OMX_ERRORTYPE result = Peek(pData, nDataSize);

    if (result == OMX_ErrorNone) {
        if (vt_queue_pop((void *)m_pHandle, pData, (int)nDataSize) != 0) {
            VTEST_MSG_ERROR("failed to pop queue");
            result = OMX_ErrorUndefined;
        }
    }

    return result;

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Queue::Push(OMX_PTR pData,
                          OMX_S32 nDataSize) {
//    VTEST_MSG_LOW("Push");
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (vt_queue_push((void *)m_pHandle, (void *)pData, (int)nDataSize) != 0) {
        VTEST_MSG_ERROR("failed to push onto queue");
        result = OMX_ErrorUndefined;
    }

    return result;
}

OMX_ERRORTYPE Queue::Peek(OMX_PTR pData,
                          OMX_S32 nDataSize) {
//    VTEST_MSG_LOW("Peek");
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (vt_queue_peek((void *)m_pHandle, (void *)pData, (int)nDataSize) != 0) {
        VTEST_MSG_ERROR("failed to peek into queue");
        result = OMX_ErrorUndefined;
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_S32 Queue::GetSize() {
    return (OMX_S32)vt_queue_size((void *)m_pHandle);
}

} // namespace vtest
