/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _ALS_H
#define _ALS_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#include "common_log.h"

#ifndef LOGE_IF
#define LOGE_IF(cond, ...) \
    ( (CONDITION(cond)) ? ((void)LOG(LOG_ERROR, LOG_TAG, __VA_ARGS__)) : (void)0 )
#endif
#ifndef LOGD_IF
#define LOGD_IF(cond, ...) \
    ( (CONDITION(cond)) ? ((void)LOG(LOG_INFO, LOG_TAG, __VA_ARGS__)) : (void)0 )
#endif

#define AMBIENT_LIGHT_MIN 0
#define AMBIENT_LIGHT_MAX 65535
#define AMBIENT_LIGHT_THR 0.5
#define AMBIENT_LIGHT_OFFSET 100
#define ALS_SHIFT_BIT 16

#define DEFAULT_SAMPLE_RATE 5
#define SELECT_SAMPLE_RATE(x) (((x) < DEFAULT_SAMPLE_RATE) ? (x) : DEFAULT_SAMPLE_RATE)

class ALS {
protected:
    bool mDebug;
public:
    bool mALSReady;
    int mALSValue;
    int mALSSampleRate;
    bool mSampleRateSet;
    bool mALSPaused;

    pthread_mutex_t mALSLock;
    pthread_cond_t  mALSCond;

    virtual int ALSInfoQuery() {
        LOGE("%s is not supported", __func__);
        return 0;
    }
    virtual int ALSRegister() {
        LOGE("%s is not supported", __func__);
        return 0;
    }
    virtual int ALSDeRegister() {
        LOGE("%s is not supported", __func__);
        return 0;
    }
    virtual int ALSRun(bool isFirstRun) {
        LOGE("%s is not supported", __func__);
        return 0;
    }
    virtual int ALSCleanup() {
        LOGE("%s is not supported", __func__);
        return 0;
    }
    virtual int ALSPauseControl(bool) {
        LOGE("%s is not supported", __func__);
        return 0;
    }
    virtual int ALSReadSensor() {
        LOGE("%s is not supported", __func__);
        return 0;
    }
    ALS() : mDebug(false), mALSReady(0), mALSValue(0), mALSSampleRate(DEFAULT_SAMPLE_RATE),
            mSampleRateSet(false), mALSPaused(false) {
        char property[PROPERTY_VALUE_MAX];
        if (property_get("debug.als.logs", property, 0) > 0 && (atoi(property) > 0 )) {
            mDebug = true;
        }

        pthread_mutex_init(&mALSLock, NULL);
        pthread_cond_init(&mALSCond, NULL);
    }

    virtual ~ALS() {
        pthread_mutex_destroy(&mALSLock);
        pthread_cond_destroy(&mALSCond);
    }
};

#endif /* _ALS_H */

