/*
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _SCREENREFRESHER_H
#define _SCREENREFRESHER_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#include "common_log.h"
#include <cutils/sockets.h>
#include <linux/msm_mdp.h>
#include <linux/fb.h>

#include "PPDaemonUtils.h"
#include "lib-postproc.h"

#define REFRESHER_STATE_ENABLE 0x1
#define REFRESHER_STATE_CONFIG 0x2
#define REFRESHER_STATE_SUSPEND 0x4

class ScreenRefresher {
    uint32_t mState;
    uint32_t mFrames;
    uint32_t mPendingFrames;
    uint32_t mMS;
    pthread_t mThread;
    pthread_cond_t  mWaitCond;
    void ProcessRefreshWork();
    static ScreenRefresher* pRefresher;
    static uint32_t mInstanceCount;
    static uint32_t mRefCount;
    static void* pLibHandle;
    static void *refresher_thrd_func(void *obj) {
        reinterpret_cast<ScreenRefresher *>(obj)->ProcessRefreshWork();
        return NULL;
    }
    ScreenRefresher(): mState(0), mFrames(0), mPendingFrames(0), mMS(0),
                mDebug(false){
        pthread_cond_init(&mWaitCond, NULL);
        /* set the flags based on property */
        char property[PROPERTY_VALUE_MAX];
        if (property_get("debug.refresh.logs", property, 0) > 0 && (atoi(property) > 0 )) {
            mDebug = true;
        }
    }

public:
    bool mDebug;
    ~ScreenRefresher() {
        pthread_cond_destroy(&mWaitCond);
    }

    int Control(bool bEnable);
    int Refresh(uint32_t nFrames, uint32_t nMS);
    int Notify(uint32_t notification_type);

    static bool isActive();
    static ScreenRefresher* getInstance();
    static void deleteInstance();
};

#endif /* _SCREENREFRESHER_H */
