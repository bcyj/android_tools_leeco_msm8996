/*
 * DESCRIPTION
 * This file contains the ambient light sensor functionalities read from
 * Android Native API for sensors
 *
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "NativeLightSensor.h"
#include <utils/Timers.h>

#define USEC_SEC 1000000
#define POLL_NO_TIMEOUT -1

NativeLightSensor* NativeLightSensor::pLightSensor = NULL;

int NativeLightSensor::ALSRegister() {
    return 0;
}

int NativeLightSensor::ALSDeRegister() {
    pthread_mutex_lock(&mCtrlLock);
    mSensorThreadStatus = SENSOR_THREAD_SUSPEND;
    pthread_mutex_unlock(&mCtrlLock);

    pthread_mutex_lock(&mALSLock);
    LOGD_IF(mDebug, "%s: Force signalling read", __FUNCTION__);
    bALSReady = true;
    pthread_cond_signal(&mALSCond);
    pthread_mutex_unlock(&mALSLock);

    if (pLooper)
        ALooper_wake(pLooper);

    return 0;
}

int NativeLightSensor::ALSRun(bool isFirstRun) {
    int32_t err;
    pthread_attr_t attr;

    pthread_mutex_lock(&mCtrlLock);
    if (mSensorThreadStatus == SENSOR_THREAD_SUSPEND) {
        mSensorThreadStatus = SENSOR_THREAD_ON;
        pthread_cond_signal(&mCtrlCond);
    } else if ((mSensorThreadStatus == SENSOR_THREAD_OFF) &&
                                                        (!bSensorLoopRunning)) {
        LOGD_IF(mDebug, "%s: Trying to create a new thread", __FUNCTION__);
        mSensorThreadStatus = SENSOR_THREAD_ON;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        err = pthread_create(&mALSSensorThread, &attr, als_sensor_read, this);
        if (err) {
            LOGE_IF(mDebug, "%s: Failed to start the control thread",
                __FUNCTION__);
            mSensorThreadStatus = SENSOR_THREAD_OFF;
        }
        pthread_attr_destroy(&attr);
    }
    pthread_mutex_unlock(&mCtrlLock);
    return 0;
}

int NativeLightSensor::ALSCleanup() {
    void *res;
    pthread_mutex_lock(&mCtrlLock);
    if (bSensorLoopRunning) {
        mSensorThreadStatus = SENSOR_THREAD_OFF;
        pthread_cond_signal(&mCtrlCond);
    }
    pthread_mutex_unlock(&mCtrlLock);
    if (pLooper)
        ALooper_wake(pLooper);
    pthread_join(mALSSensorThread, &res);
    return 0;
}

int NativeLightSensor::ALSReadSensor() {
    int temp_val = 0;

    pthread_mutex_lock(&mALSLock);
    temp_val = mALSValue;
    pthread_mutex_unlock(&mALSLock);

    pthread_mutex_lock(&mCtrlLock);
    if (mSensorThreadStatus != SENSOR_THREAD_ON) {
        pthread_mutex_unlock(&mCtrlLock);
        LOGE_IF(mDebug, "%s: Sensor Thread is not running", __FUNCTION__);
    } else {
        pthread_mutex_unlock(&mCtrlLock);

        pthread_mutex_lock(&mALSLock);
        if (!bALSReady) {
            pthread_cond_wait(&mALSCond, &mALSLock);
            if (bALooperHalt) {
                LOGE_IF(mDebug, "%s: Looper encountered error or exit",
                                                            __FUNCTION__);
                pthread_mutex_unlock(&mALSLock);
                SetLooperHalt(false);
                pthread_mutex_lock(&mALSLock);
            }
        }
        bALSReady = false;
        temp_val = mALSValue;
        pthread_mutex_unlock(&mALSLock);
    }
    return temp_val;
}

bool NativeLightSensor::CheckThreadStatus() {
    bool ret = false;

    pthread_mutex_lock(&mCtrlLock);
    switch (mSensorThreadStatus) {
    case SENSOR_THREAD_ON:
        ret = true;
        break;
    case SENSOR_THREAD_OFF:
        ret = false;
        break;
    case SENSOR_THREAD_SUSPEND:
        SensorControl(OFF);
        pthread_cond_wait(&mCtrlCond, &mCtrlLock);
        if (mSensorThreadStatus == SENSOR_THREAD_ON)
            ret = true;
        else
            ret = false;
        break;
    default:
        LOGE("%s: Invalid thread status!", __FUNCTION__);
    }
    pthread_mutex_unlock(&mCtrlLock);
    return ret;
}

void NativeLightSensor::ProcessNativeLightSensorWork() {
    pthread_mutex_lock(&mCtrlLock);
    if (bSensorLoopRunning ) {
        LOGE_IF(mDebug, "%s: Thread already running", __FUNCTION__);
        pthread_mutex_unlock(&mCtrlLock);
        return;
    }
    bSensorLoopRunning = true;
    pthread_mutex_unlock(&mCtrlLock);

    LOGD_IF(mDebug, "%s Getting manager and light sensor instance",
        __FUNCTION__);
    pSensorManager = ASensorManager_getInstance();
    mLightSensor = ASensorManager_getDefaultSensor(pSensorManager,
        ASENSOR_TYPE_LIGHT);
    if(!mLightSensor) {
        LOGE("%s: Light Sensor was not found on the device", __FUNCTION__);
        goto thread_exit;
    }

    mMinDelay = ASensor_getMinDelay(mLightSensor);
    mMinDelay = (mMinDelay < DEFAULT_DELAY) ? DEFAULT_DELAY : mMinDelay;
    if (mMinDelay)
        LOGD_IF(mDebug, "%s: Sample Rate set = %d per sec", __FUNCTION__,
            (int) (USEC_SEC / mMinDelay));
    else
        LOGD_IF(mDebug, "%s: Light Sensor does not have sample rate",
                __FUNCTION__);

    pLooper = ALooper_forThread();
    if(!pLooper) {
        LOGD_IF(mDebug, "%s: Preparing new Looper for thread", __FUNCTION__);
        pLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    }
    pSensorEventQueue = ASensorManager_createEventQueue(pSensorManager,
        pLooper, ALOOPER_EVENT_INPUT , NULL, NULL);
    SensorControl(ON);

    while(CheckThreadStatus()) {
        SensorControl(ON);

        mIdent = ALooper_pollOnce( POLL_NO_TIMEOUT, NULL, NULL, NULL);

        if (!CheckThreadStatus())
            break;

        switch (mIdent) {

            case ALOOPER_POLL_WAKE :
                LOGD_IF(mDebug, "%s: Interrupt: Poll awoken before timeout",
                    __FUNCTION__);
                SetLooperHalt(true);
                break;

            case ALOOPER_POLL_CALLBACK :
                LOGD_IF(mDebug, "%s: Callback: Callbacks invoked",
                    __FUNCTION__);
                SetLooperHalt(false);
                break;

            case ALOOPER_POLL_TIMEOUT :
                LOGD_IF(mDebug, "%s: Timeout : No data available",
                    __FUNCTION__);
                SetLooperHalt(false);
                break;

            case ALOOPER_POLL_ERROR :
                LOGE("%s: Error: occured while polling", __FUNCTION__);
                SetLooperHalt(true);
                goto thread_exit;

            default:
                SetLooperHalt(false);
                while (ASensorEventQueue_getEvents(pSensorEventQueue,
                    &mEvent, 1) > 0) {

                    pthread_mutex_lock(&mALSLock);
                    mALSValue = (int) (mEvent.light + 0.5);
                    LOGD_IF(mDebug, "%s: ALS Value (NATIVE) = %d", __FUNCTION__,
                        mALSValue);
                    bALSReady = true;
                    pthread_cond_signal(&mALSCond);
                    pthread_mutex_unlock(&mALSLock);

                }
                LOGD_IF(mDebug, "%s: No more events present", __FUNCTION__);
       }
    }

thread_exit:
    LOGD_IF(mDebug, "%s: Exiting the thread", __FUNCTION__);
    SensorControl(OFF);
    ASensorManager_destroyEventQueue(pSensorManager, pSensorEventQueue);

    pthread_mutex_lock(&mCtrlLock);
    bSensorLoopRunning = false;
    pthread_mutex_unlock(&mCtrlLock);

    pLooper = NULL;
    pSensorManager = NULL;

    return;
}

void NativeLightSensor::SensorControl(bool b) {
    if ((bSensorStatus == OFF) && (b)) {
        ASensorEventQueue_enableSensor(pSensorEventQueue, mLightSensor);
        ASensorEventQueue_setEventRate( pSensorEventQueue,
            mLightSensor, mMinDelay);
        bSensorStatus = ON;
    } else if ((bSensorStatus == ON) && (!b)) {
        ASensorEventQueue_disableSensor(pSensorEventQueue, mLightSensor);
        bSensorStatus = OFF;
    }
}

void NativeLightSensor::SetLooperHalt(bool b) {
    pthread_mutex_lock(&mALSLock);
    bALooperHalt = b;
    if (b)
        pthread_cond_signal(&mALSCond);
    pthread_mutex_unlock(&mALSLock);
}

ALS* NativeLightSensor::getInstance() {
    if (pLightSensor == NULL)
        pLightSensor = new NativeLightSensor();
    return pLightSensor;
}

/* Following is for enabling dynamic linking of libmm-als to clients */
extern "C" ALS* getSensor() {
    return NativeLightSensor::getInstance();
}
