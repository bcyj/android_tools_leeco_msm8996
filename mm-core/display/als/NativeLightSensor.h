/*
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _NATIVELIGHTSENSOR_H
#define _NATIVELIGHTSENSOR_H

#include <jni.h>
#include <android/sensor.h>

#include "als.h"

#define ON true
#define OFF false
#define DEFAULT_DELAY 500000

enum {
    SENSOR_THREAD_OFF = 0,
    SENSOR_THREAD_ON,
    SENSOR_THREAD_SUSPEND,
};

class NativeLightSensor: public ALS {
    ALooper* pLooper;
    ASensorEvent mEvent;
    ASensorEventQueue* pSensorEventQueue;
    ASensorManager* pSensorManager;
    ASensorRef mLightSensor;
    bool bALooperHalt;
    bool bALSReady;
    bool bSensorLoopRunning;
    bool bSensorStatus;
    bool CheckThreadStatus();
    int mALSValue;
    int mIdent;
    int mMinDelay;
    int mSensorThreadStatus;
    pthread_cond_t mALSCond;
    pthread_cond_t mCtrlCond;
    pthread_mutex_t mALSLock;
    pthread_mutex_t mCtrlLock;
    pthread_t mALSSensorThread;
    static NativeLightSensor *pLightSensor;
    static void *als_sensor_read(void *obj) {
        reinterpret_cast<NativeLightSensor *>(obj)->
            ProcessNativeLightSensorWork();
        return NULL;
    }
    void ProcessNativeLightSensorWork();
    void SensorControl(bool b);
    void SetLooperHalt(bool b);

    NativeLightSensor() : pLooper(NULL), pSensorEventQueue(NULL),
        pSensorManager(NULL), mLightSensor(NULL), bALooperHalt(false),
        bALSReady(false), bSensorLoopRunning(false), bSensorStatus(OFF),
        mALSValue(0), mIdent(0), mMinDelay(0),
        mSensorThreadStatus(SENSOR_THREAD_OFF) {

        pthread_cond_init(&mALSCond, NULL);
        pthread_cond_init(&mCtrlCond, NULL);
        pthread_mutex_init(&mALSLock, NULL);
        pthread_mutex_init(&mCtrlLock, NULL);

    }

public:
    ~NativeLightSensor() {
        pthread_cond_destroy(&mALSCond);
        pthread_cond_destroy(&mCtrlCond);
        pthread_mutex_destroy(&mALSLock);
        pthread_mutex_destroy(&mCtrlLock);
    }

    int ALSRegister();
    int ALSDeRegister();
    int ALSRun(bool isFirstRun);
    int ALSCleanup();
    int ALSReadSensor();

    static ALS* getInstance();
};
#endif
