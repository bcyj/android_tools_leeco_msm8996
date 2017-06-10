/*
 * DESCRIPTION
 * This file contains the dummy light sensor functionalities
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */
#include "DummySensor.h"

#define LOG_TAG "DummySensor"
/* Returns -1 on error, 0 on success */
int DummySensor::ALSRegister()
{
    FILE *dummy = NULL;
    char *line = NULL;
    int ret = -1;
    char property[PROPERTY_VALUE_MAX];
    char al_dummy[PROPERTY_VALUE_MAX] = {'\0'};
    int i = 0;
    // find dummy file in property
    if (property_get(ALS_DUMMY_FILE_PATH_PROP, al_dummy, NULL) > 0) {
        if (-1 == access(&al_dummy[0], R_OK|F_OK)) {
            return ret;
        }
    }

    LOGD_IF(mDebug, "=== Beginning Reading Dummy ALS file %s ===", al_dummy);
    // open dummy file
    dummy = fopen(al_dummy, "r");
    if (!dummy)
        return ret;

    // read dummy file to array
    line = (char *)malloc(MAX_DUMMY_LINE_LEN * sizeof(char));
    if (!line){
        LOGE("Can't alloc line mem!");
        goto err_mem;
    }
    pthread_mutex_lock(&mALSLock);
    while (fgets(line, MAX_DUMMY_LINE_LEN, dummy) &&
                i < MAX_DUMMY_ALS) {
        als_vals[i] = atoi(line);
        LOGD_IF(mDebug, "line %d = %d", i, als_vals[i]);
        i++;
    }
    max_als_vals = i;
    pthread_mutex_unlock(&mALSLock);
    free(line);

    ret = 0;
    LOGD_IF(mDebug, "=== Finished Reading Dummy ALS file ===");
err_mem:
    //close dummy file
    fclose(dummy);

    return ret;
}

int DummySensor::ALSDeRegister()
{
    //remove timer
    pthread_mutex_lock(&mALSLock);
    pthread_cond_signal(&mALSCond);
    int ret = timer_delete(timer_id);
    if (ret) {
        LOGE("timer delete failed");
    }
    pthread_mutex_unlock(&mALSLock);
    return ret;
}

void DummySensor::processDummyWork(union sigval sigev_value)
{
    DummySensor *sensor = (DummySensor *) sigev_value.sival_ptr;
    pthread_mutex_lock(&sensor->mALSLock);
    sensor->mALSValue = sensor->als_vals[sensor->als_idx];
    LOGD_IF(mDebug, "sensor->mALSValue = %d", sensor->mALSValue);
    sensor->mALSReady = 1;
    pthread_cond_signal(&sensor->mALSCond);
    sensor->als_idx = (sensor->als_idx + 1) % sensor->max_als_vals;
    int ret = timer_settime(sensor->timer_id, 0, &sensor->interval, NULL);
    if (ret) {
        LOGE("reset Dummy settimer failed");
    }
    pthread_mutex_unlock(&sensor->mALSLock);
}

int DummySensor::ALSRun(bool isFirstRun)
{
    struct sigevent ev;
    struct itimerspec new_value;
    int ret = -1;

    ev.sigev_notify = SIGEV_THREAD;
    ev.sigev_notify_function = dummy_cb_func;
    ev.sigev_notify_attributes = NULL;
    ev.sigev_value.sival_ptr = this;

    //start timer
    pthread_mutex_lock(&mALSLock);
    ret = timer_create(CLOCK_REALTIME, &ev, &timer_id);
    if (!ret) {
        ret = timer_settime(timer_id, 0, &interval, NULL);
        if (ret) {
            LOGE("create Dummy settimer failed");
            goto err;
        }
    } else {
        LOGE("Dummy timer failed");
        goto err;
    }
err:
    pthread_mutex_unlock(&mALSLock);
    return ret;
}

int DummySensor::ALSCleanup()
{
    return 0;
}

int DummySensor::ALSReadSensor()
{
    //wait for timer to signal
    pthread_mutex_lock(&mALSLock);
    if(!mALSReady)
        pthread_cond_wait(&mALSCond, &mALSLock);
    mALSReady = 0;
    //return with als value from array
    pthread_mutex_unlock(&mALSLock);

    return mALSValue;

}
