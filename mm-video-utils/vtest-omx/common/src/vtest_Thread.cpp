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

#include <pthread.h>
#include <stdlib.h>
#include "vtest_Debug.h"
#include "vtest_Thread.h"

namespace vtest {

typedef struct vt_thread_type
{
   thread_fn_type pfn;
   int priority;
   pthread_t thread;
   void* thread_data;
} vt_thread_type;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void* vt_thread_entry(void* thread_data)
{
   vt_thread_type* thread = (vt_thread_type*) thread_data;
   return (void*) thread->pfn(thread->thread_data);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static long vt_thread_create(void** handle,
                       thread_fn_type pfn,
                       void* thread_data,
                       int priority)
{
   long result = 0;
   vt_thread_type* thread = (vt_thread_type*) malloc(sizeof(vt_thread_type));
   *handle = thread;

   if (thread)
   {
      int create_result;
      thread->pfn = pfn;
      thread->thread_data = thread_data;
      thread->priority = priority;
      create_result = pthread_create(&thread->thread,
                                     NULL,
                                     vt_thread_entry,
                                     (void*) thread);
      if (create_result != 0)
      {
         VTEST_MSG_ERROR("failed to create thread");
         result = 1;
      }
   }
   else
   {
      VTEST_MSG_ERROR("failed to allocate thread");
      result = 1;
   }
   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int vt_thread_destroy(void* handle, int* thread_result)
{
   int result = 0;
   vt_thread_type* thread = (vt_thread_type*) handle;

   if (thread)
   {
      if (pthread_join(thread->thread, (void**) thread_result) != 0)
      {
         VTEST_MSG_ERROR("failed to join thread");
         result = 1;
        }
      free(thread);
   }
   else
   {
      VTEST_MSG_ERROR("handle is null");
      result = 1;
   }
   return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Thread::Thread()
    : m_pThreaded(NULL),
      m_pFn(NULL),
      m_nPriority(0),
      m_pThread(NULL),
      m_pThreadArgs(NULL),
      m_bStarted(OMX_FALSE),
      m_nThreadStatus(OMX_ErrorNone) {
    VTEST_MSG_LOW("constructor");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Thread::~Thread() {
    VTEST_MSG_LOW("destructor");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Thread::Start(StartFnType pFn,
                            OMX_PTR pThreadArgs,
                            OMX_S32 nPriority) {
    VTEST_MSG_LOW("Start");
    if (m_bStarted) {
        VTEST_MSG_ERROR("error: thread already started");
        return OMX_ErrorUndefined;
    }

    m_nThreadStatus = OMX_ErrorNone;
    m_pThreadArgs = pThreadArgs;
    m_pFn = pFn;
    m_pThreaded = NULL;

    if (vt_thread_create(&m_pThread, ThreadEntry, this, (int)nPriority) != 0) {
        VTEST_MSG_ERROR("failed to create thread");
        return OMX_ErrorUndefined;
    }

    m_bStarted = OMX_TRUE;
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Thread::Start(IThreaded *pThreaded,
                            OMX_PTR pThreadArgs,
                            OMX_S32 nPriority) {
    VTEST_MSG_LOW("Start");
    if (m_bStarted) {
        VTEST_MSG_ERROR("error: thread already started");
        return OMX_ErrorUndefined;
    }

    m_nThreadStatus = OMX_ErrorNone;
    m_pThreadArgs = pThreadArgs;
    m_pFn = NULL;
    m_pThreaded = pThreaded;

    if (vt_thread_create(&m_pThread, ThreadEntry, this, (int)nPriority) != 0) {
        VTEST_MSG_ERROR("failed to create thread");
        return OMX_ErrorUndefined;
    }

    m_bStarted = OMX_TRUE;
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_BOOL Thread::Started() {
    return m_bStarted;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Thread::Join(OMX_ERRORTYPE *pThreadResult) {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    int thread_result;

    VTEST_MSG_LOW("Join");

    if (vt_thread_destroy(m_pThread, &thread_result) != 0) {
        VTEST_MSG_ERROR("failed to destroy thread");
    }

    if (pThreadResult != NULL) {
        *pThreadResult = (OMX_ERRORTYPE)thread_result;
    }

    m_bStarted = OMX_FALSE;
    m_pThread = NULL;

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
 OMX_ERRORTYPE Thread::GetStatus() {
     return m_nThreadStatus;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
long Thread::ThreadEntry(void *pThreadData) {
    Thread *pThread = (Thread *)pThreadData;
    VTEST_MSG_LOW("ThreadEntry");
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!pThread)
        return OMX_ErrorBadParameter;

    if (pThread->m_pFn != NULL) {
        result = pThread->m_pFn(pThread->m_pThreadArgs);
    }
    else if (pThread->m_pThreaded != NULL) {
        result = pThread->m_pThreaded->ThreadRun(pThread->m_pThreadArgs);
    }
    else {
        VTEST_MSG_ERROR("failed to create thread");
        result = OMX_ErrorUndefined;
    }

    pThread->m_nThreadStatus = result;
    pThread->m_bStarted = OMX_FALSE;
    return (long)result;
}

} // namespace vtest
