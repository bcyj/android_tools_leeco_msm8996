/*******************************************************************************
@file    TimeServiceNative.cpp
@brief   Provides JNI API to update user time offset when user changes time of
         the day. time-services restores the time of the day after reboot using
         this offset.
---------------------------------------------------------------------------
Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
******************************************************************************/

#include <cutils/log.h>
#include "com_qualcomm_timeservice_TimeServiceBroadcastReceiver.h"

extern "C" {
#include <time_genoff.h>
}

static const char* sLogTag="TimeServiceNative";

static int setTimeServicesTime(time_bases_type base, int64_t millis)
{
    int ret = 0;
    time_genoff_info_type time_set;
    uint64_t value = millis;

    time_set.base = base;
    time_set.unit = TIME_MSEC;
    time_set.operation = T_SET;
    time_set.ts_val = &value;
    ret = time_genoff_operation(&time_set);

    if (ret) {
        ALOGE("%s: Error setting generic offset: %d\n", sLogTag, ret);
    }

    return ret;
}

JNIEXPORT void JNICALL
Java_com_qualcomm_timeservice_TimeServiceBroadcastReceiver_setTimeServicesUserTime
  (JNIEnv *env, jobject obj, jlong millis) {

    ALOGD("%s: User Time to be set is %lld\n", sLogTag, millis);

    setTimeServicesTime(ATS_USER, millis);
}
