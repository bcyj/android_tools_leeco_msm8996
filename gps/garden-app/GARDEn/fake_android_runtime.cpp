/* Copyright (c) 2011-2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <android_runtime/AndroidRuntime.h>
#include "fake_logd_write.h"

namespace android {

pthread_t AndroidRuntime::createJavaThread(const char* name,
    void (*start)(void *), void* arg)
{
    pthread_t threadId = 0;
    pthread_create(&threadId, NULL, (void *(*)(void*))start, arg);
    return threadId;
}
}
