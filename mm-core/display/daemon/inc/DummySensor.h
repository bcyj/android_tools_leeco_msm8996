/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _DUMMYSENSOR_H
#define _DUMMYSENSOR_H

#include <assert.h>
#include "als.h"
#define MAX_DUMMY_ALS 1024
#define MAX_DUMMY_LINE_LEN 32
#define ALS_DUMMY_FILE_PATH_PROP "ro.qcom.lightsensorsim.input"

class DummySensor: public ALS {
public:
    int als_vals[MAX_DUMMY_ALS];
    int max_als_vals;
    int als_idx;
    timer_t timer_id;
    struct itimerspec interval;
    DummySensor() : ALS() {
    //Initialize control structs
        assert(sizeof(char) == 1);
        pthread_mutex_init(&mALSLock, NULL);
        pthread_cond_init(&mALSCond, NULL);
        max_als_vals = 0;
        als_idx=0;
        timer_id = 0;
        interval.it_value.tv_sec = 0;
        interval.it_value.tv_nsec = 200000000; //200ms

        interval.it_interval.tv_sec = 0;
        interval.it_interval.tv_nsec = 0;
        mALSValue = 100;
    }
    ~DummySensor() {
        pthread_mutex_destroy(&mALSLock);
        pthread_cond_destroy(&mALSCond);
    }
    int ALSRegister();
    int ALSDeRegister();
    int ALSRun(bool isFirstRun);
    int ALSCleanup();
    int ALSReadSensor();

    void processDummyWork(union sigval sigev_value);
    static void dummy_cb_func(union sigval sigev_value) {
        reinterpret_cast<DummySensor *>(sigev_value.sival_ptr)->processDummyWork(sigev_value);
    }
};

#endif /* end _DUMMYSENSOR_H */


