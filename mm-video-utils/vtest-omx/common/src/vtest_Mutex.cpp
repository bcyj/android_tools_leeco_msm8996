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
#include "vtest_ComDef.h"
#include "vtest_Mutex.h"
#include "vtest_Debug.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int vt_mutex_create(void **handle) {

    int result = 0;
    pthread_mutex_t *mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

    if (!mutex)
       return 1;

    if (handle) {
        if (pthread_mutex_init(mutex, NULL) == 0) {
            *handle = mutex;
        } else {
            VTEST_MSG_ERROR("init mutex failed");
            free(mutex);
            result = 1;
        }
    } else {
        VTEST_MSG_ERROR("handle is null");
        result = 1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int vt_mutex_destroy(void *handle) {

    int result = 0;
    pthread_mutex_t *mutex = (pthread_mutex_t *)handle;

    if (mutex) {
        if (pthread_mutex_destroy(mutex) == 0) {
            free(handle);
        } else {
            VTEST_MSG_ERROR("destroy mutex failed");
            result = 1;
        }
    } else {
        VTEST_MSG_ERROR("handle is null");
        result = 1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int vt_mutex_lock(void *handle) {

    int result = 0;
    pthread_mutex_t *mutex = (pthread_mutex_t *)handle;

    if (mutex) {
        if (pthread_mutex_lock(mutex) != 0) {
            VTEST_MSG_ERROR("lock mutex failed");
        }
    } else {
        VTEST_MSG_ERROR("handle is null");
        result = 1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int vt_mutex_unlock(void *handle) {

    int result = 0;
    pthread_mutex_t *mutex = (pthread_mutex_t *)handle;

    if (mutex) {
        if (pthread_mutex_unlock(mutex) != 0) {
            VTEST_MSG_ERROR("lock mutex failed");
        }
    } else {
        VTEST_MSG_ERROR("handle is null");
        result = 1;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Mutex::Mutex() :
    m_pMutex(NULL) {

    if (vt_mutex_create(&m_pMutex) != 0) {
        VTEST_MSG_ERROR("failed to init mutex");
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Mutex::~Mutex() {

    if (vt_mutex_destroy(m_pMutex) != 0) {
        VTEST_MSG_ERROR("failed to destroy mutex");
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Mutex::Lock() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (vt_mutex_lock(m_pMutex) != 0) {
        VTEST_MSG_ERROR("failed to lock mutex");
        result = OMX_ErrorUndefined;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Mutex::UnLock() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (vt_mutex_unlock(m_pMutex) != 0) {
        VTEST_MSG_ERROR("failed to unlock mutex");
        result = OMX_ErrorUndefined;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Mutex::Autolock::Autolock(Mutex *pMutex) {

    m_pMutex = pMutex;
    m_pMutex->Lock();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Mutex::Autolock::~Autolock() {

    m_pMutex->UnLock();
}

}
