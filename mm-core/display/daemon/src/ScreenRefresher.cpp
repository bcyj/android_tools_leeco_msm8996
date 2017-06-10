/*
 * DESCRIPTION
 * This file contains a helper class for sending display commits or screen
 * refreshes when needed for postprocessing features.
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "ScreenRefresher.h"
#include <time.h>
#include <dlfcn.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

ScreenRefresher* ScreenRefresher::pRefresher = NULL;
uint32_t ScreenRefresher::mInstanceCount = 0;
uint32_t ScreenRefresher::mRefCount = 0;
void* ScreenRefresher::pLibHandle = NULL;

bool ScreenRefresher::isActive() {
    bool ret = false;
    pthread_mutex_lock(&lock);
    if (mInstanceCount > 0)
        ret = true;
    pthread_mutex_unlock(&lock);
    return ret;
}

ScreenRefresher* ScreenRefresher::getInstance() {
    pthread_mutex_lock(&lock);
    if (pLibHandle == NULL) {
        /*
         * Get handle to the libqdutils library even though
         * utility is part of libqservice. This is because
         * the utility is an inline function in QserviceUtils.h
         * header file that is not included in libqservice but
         * in libqdutils only
         */
        pLibHandle = dlopen("libqdutils.so", RTLD_NOW);
        if (pLibHandle)
            LOGD("%s: Successfully loaded libqdutils.so", __func__);
        else
            LOGE("%s: Not able to load libqdutils.so", __func__);
    }
    if (pRefresher == NULL)
        pRefresher = new ScreenRefresher();
    if ((pRefresher) && (pLibHandle)) {
        mInstanceCount++;
        LOGD_IF(pRefresher->mDebug, "%s: Instance count increased to = %d",
            __func__, mInstanceCount);
    }
    pthread_mutex_unlock(&lock);
    return pRefresher;
}

void ScreenRefresher::deleteInstance() {
    pthread_mutex_lock(&lock);
    if (pRefresher) {
        if (mInstanceCount) {
            mInstanceCount--;
            LOGD_IF(pRefresher->mDebug, "%s: Instance count decrease to = %d",
                __func__, mInstanceCount);
        } else {
            LOGE("%s: Trying to delete instance without getting", __func__);
            pthread_mutex_unlock(&lock);
            return;
        }
        if (mInstanceCount == 0) {
            /* Close handle to the library */
            if (pLibHandle)
                dlclose(pLibHandle);
            pLibHandle = NULL;
            LOGD("%s: Closed the handle to libqdutils", __func__);
            delete pRefresher;
            pRefresher = NULL;
        }
    }
    pthread_mutex_unlock(&lock);
}

void ScreenRefresher::ProcessRefreshWork() {
    int32_t ret =-1;
    int32_t (*refresh)() = NULL;
    struct timespec wait_time;

    LOGD_IF(mDebug, "%s() Entering, mState = %d", __func__, mState);
    pthread_mutex_lock(&lock);
    mState |= REFRESHER_STATE_ENABLE;
    pthread_mutex_unlock(&lock);
    pthread_mutex_lock(&lock);

    if (pLibHandle)
        *(void **)&refresh = dlsym(pLibHandle, "refreshScreen");
    if (refresh == NULL) {
        LOGE("%s: Could not find symbol for refreshScreen", __func__);
        goto exit;
    }

    while (mState & REFRESHER_STATE_ENABLE) {
        if (!(mState & REFRESHER_STATE_CONFIG)) {
            LOGD_IF(mDebug, " %s(): Wait for mWaitCond!", __func__);
            pthread_cond_wait(&mWaitCond, &lock); //Wait for Refresh()
            if (!(mState & REFRESHER_STATE_ENABLE)) {
                LOGD_IF(mDebug, " %s(): ScreenRefresh disabled.", __func__);
                goto exit;
            }
        }

        mPendingFrames = mFrames;

        LOGD_IF(mDebug, " %s(): mState = %d, mFrames = %d", __func__, mState, mFrames);
        while (mFrames && mPendingFrames ) {
            LOGD_IF(mDebug, "%s(): mFrames = %d, count = %d", __func__, mFrames,
                mFrames - mPendingFrames);
            if (pLibHandle) {
                ret = refresh();
                if(ret < 0) {
                    LOGE("%s: Failed to signal HWC", strerror(errno));
                } else {
                    LOGD_IF(mDebug, "%s: screenRefresh successful!", __func__);
                }
            } else {
                LOGE("%s: Handle for calling refresh is not there!", __func__);
                goto exit;
            }
            mPendingFrames--;
            pthread_mutex_unlock(&lock);

            clock_gettime(CLOCK_REALTIME, &wait_time);
            wait_time.tv_nsec += mMS*1000000;

            pthread_mutex_lock(&lock);
            ret = pthread_cond_timedwait(&mWaitCond, &lock, &wait_time);
            if (!(mState & REFRESHER_STATE_ENABLE)) {
                LOGD_IF(mDebug, " %s(): ScreenRefresh disabled.", __func__);
                goto exit;
            }
            if (mState & REFRESHER_STATE_SUSPEND)
                break;
            if (ret == ETIMEDOUT) {
                continue;
            } else if (ret) {
                LOGE("%s: pthread_cond_timedwait failed. err: %s", __func__, strerror(errno));
            }
        }
        mState &= ~REFRESHER_STATE_CONFIG;
    }
exit:
    mState &= ~REFRESHER_STATE_CONFIG;
    pthread_mutex_unlock(&lock);
    LOGD_IF(mDebug, "%s() Exiting!", __func__);
    pthread_exit(NULL);
    return;
}

int ScreenRefresher::Control(bool bEnable) {
    int ret = 0;

    LOGD_IF(mDebug, "%s(%d) Entering!", __func__, bEnable);
    if( bEnable ) {
        pthread_mutex_lock(&lock);
        mRefCount = mRefCount + 1;
        LOGD_IF(mDebug, "%s: Ref Count increased to %d", __func__, mRefCount);
        if( mState & REFRESHER_STATE_ENABLE ) {
            ret = -1;
            pthread_mutex_unlock(&lock);
            return ret;
        }
        /*start the incalidate thread here*/
        LOGD_IF(mDebug, "Starting the refresh thread!");
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        ret = pthread_create(&mThread, &attr, refresher_thrd_func, this);
        if (ret) {
            LOGE("Failed to create screen refresher thread, error = %s", strerror(ret));
        }
        pthread_attr_destroy(&attr);
        pthread_mutex_unlock(&lock);
    } else {
        pthread_mutex_lock(&lock);
        if (mRefCount) {
            mRefCount = mRefCount - 1;
            LOGD_IF(mDebug, "%s: Ref Count decreased to %d", __func__,
                    mRefCount);
        } else {
            LOGE("%s: Request for decreasing ref count below 0", __func__);
            pthread_mutex_unlock(&lock);
            return -1;
        }
        if (mRefCount == 0) {
            LOGD("%s: Stopping the refresh thread!", __func__);
            pRefresher->mState &= ~REFRESHER_STATE_ENABLE;
            pthread_cond_signal(&pRefresher->mWaitCond);
        }
        pthread_mutex_unlock(&lock);
    }

    LOGD_IF(mDebug, "%s() Exiting, ret = %d!", __func__, ret);
    return ret;
}

int ScreenRefresher::Refresh(uint32_t nFrames, uint32_t nMS) {
    int ret = -1;

    LOGD_IF(mDebug, "%s() Entering!", __func__);
    pthread_mutex_lock(&lock);
    if ((mState & REFRESHER_STATE_ENABLE) &&
                        !(mState & REFRESHER_STATE_SUSPEND)) {
        /* Check if new request is for more refreshes */
        if (nFrames > mPendingFrames) {
            mFrames = nFrames;
            mPendingFrames = nFrames;
        }
        /* Check if new request is for faster refreshes */
        if (nMS < mMS)
            mMS = nMS;
        mState |= REFRESHER_STATE_CONFIG;
        pthread_cond_signal(&mWaitCond);
        LOGD_IF(mDebug, "Sending mWaitCond signal!");
        ret = 0;
    } else {
        ALOGI("%s(): Skipping refresh request! refCount %d, state %d",
                __func__, mRefCount, mState);
    }
    pthread_mutex_unlock(&lock);
    LOGD_IF(mDebug, "%s() Exiting, ret = %d!", __func__, ret);
    return ret;
}

int ScreenRefresher::Notify(uint32_t notification_type) {
    LOGD_IF(mDebug, "%s() Entering!", __func__);
    pthread_mutex_lock(&lock);
    if (notification_type == NOTIFY_TYPE_UPDATE) {
        /* Remove suspend state */
        mState &= ~REFRESHER_STATE_SUSPEND;
    } else if (notification_type == NOTIFY_TYPE_SUSPEND) {
        /* Cancel refreshes */
        mFrames = 0;
        mMS = 0;
        mState |= REFRESHER_STATE_SUSPEND;
    }
    pthread_mutex_unlock(&lock);
    LOGD_IF(mDebug, "%s() Exiting!", __func__);
    return 0;
}
