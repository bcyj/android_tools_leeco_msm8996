/******************************************************************************
  @file:  jni_bridge.cpp
  @brief: File for implementing the native code assoicated with VZW Native
  GPS Location Provider

  DESCRIPTION
    This file registers to JNI and translate request from Java to C


  INITIALIZATION AND SEQUENCING REQUIREMENTS

  Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  This file contains proprietary and confidential information of Qualcomm Technologies, Inc.
  Use, modification or distribution is prohibited without a pre-existing license from
  Qualcomm Technologies, Inc. THE MODIFICATIONS ARE NOT MADE AVAILABLE UNDER THE APACHE LICENSE.

  This file has been modified by Qualcomm Technologies, Inc. This file contains code which is derived from
  code subject to the following license:

  Copyright (C) 2008 The Android Open Source Project

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include <stdio.h>

#include <jni.h>
#include <cutils/properties.h>
#include <string.h>
#include <pthread.h>

#define LOG_NDDEBUG 0
#define LOG_TAG "VzwNativeGpsProvider"
#include <utils/Log.h>

#include <hardware/gps.h>
#include <loc_eng.h>
#include "loc_vzw.h"


#define DOP_VALID_THRESHOLD 0.1f

static bool sEventCond_signaled = false;
static pthread_mutex_t sEventMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sEventCond = PTHREAD_COND_INITIALIZER;
static jmethodID method_reportLocation;
static jmethodID method_reportStatus;
static jmethodID method_reportSvStatus;
static jmethodID method_reportVzwEvent;

// data written to by GPS callbacks
static GpsLocation       sGpsLocation;
static VzwGpsLocationExt sGpsLocationExt;
static GpsStatus       sGpsStatus;
static GpsSvStatus     sGpsSvStatus;
static GpsDop          sGpsSvDop;

static GpsLocation       sGpsLocationCopy;
static VzwGpsLocationExt sGpsLocationExtCopy;
static GpsStatus       sGpsStatusCopy;
static GpsSvStatus     sGpsSvStatusCopy;
static GpsDop          sGpsSvDopCopy;

VzwGpsEventValue sVzwSpecificEvent;

enum CallbackType {
    kLocation = 1,
    kStatus = 2,
    kSvStatus = 4,
    kVzwEvent = 8,
    kDisableRequest = 32,
};

static int sPendingCallbacks;

void location_callback(GpsLocation* location, VzwGpsLocationExt* ext)
{
    pthread_mutex_lock(&sEventMutex);

    sPendingCallbacks |= kLocation;
    memcpy(&sGpsLocation, location, sizeof(sGpsLocation));
    memcpy(&sGpsLocationExt, ext, sizeof(sGpsLocationExt));

    sEventCond_signaled = true;
    pthread_cond_signal(&sEventCond);
    pthread_mutex_unlock(&sEventMutex);
}

void status_callback(GpsStatus* status)
{
    pthread_mutex_lock(&sEventMutex);

    sPendingCallbacks |= kStatus;
    memcpy(&sGpsStatus, status, sizeof(sGpsStatus));

    sEventCond_signaled = true;
    pthread_cond_signal(&sEventCond);
    pthread_mutex_unlock(&sEventMutex);
}

void sv_status_callback(GpsSvStatus* sv_status, GpsDop* sv_dop)
{
    pthread_mutex_lock(&sEventMutex);

    sPendingCallbacks |= kSvStatus;
    memcpy(&sGpsSvStatus, sv_status, sizeof(sGpsSvStatus));
    memcpy(&sGpsSvDop, sv_dop, sizeof(sGpsSvDop));

    sEventCond_signaled = true;
    pthread_cond_signal(&sEventCond);
    pthread_mutex_unlock(&sEventMutex);
}

void vzw_event_callback(VzwGpsEventValue * pEvent)
{
    pthread_mutex_lock(&sEventMutex);

    sPendingCallbacks |= kVzwEvent;
    memcpy(&sVzwSpecificEvent, pEvent, sizeof(VzwGpsEventValue));

    sEventCond_signaled = true;
    pthread_cond_signal(&sEventCond);
    pthread_mutex_unlock(&sEventMutex);
}

void nmea_callback(GpsUtcTime timestamp, const char* nmea, int length)
{
   // place holder
}

namespace vzw
{
static void class_init_native(JNIEnv* env,jclass clazz)
{
    ALOGD("Class init native, pid %d, uid %d, gid %d", getpid(), getuid(), getgid());

    method_reportLocation = env->GetMethodID(clazz, "reportLocation", "(IDDDDFFFFFFFFFFFFJ)V");
    if(!method_reportLocation)
    {
        ALOGE("cannot find callback reportLocation");
    }

    method_reportStatus = env->GetMethodID(clazz, "reportStatus", "(I)V");
    if(!method_reportStatus)
    {
        ALOGE("cannot find callback reportStatus");
    }

    method_reportSvStatus = env->GetMethodID(clazz, "reportSvStatus", "()V");
    if(!method_reportSvStatus)
    {
        ALOGE("cannot find callback reportSvStatus");
    }

    method_reportVzwEvent = env->GetMethodID(clazz, "reportVzwEvent", "(I)V");
    if(!method_reportVzwEvent)
    {
        ALOGE("cannot find callback reportVzwEvent");
    }

    loc_eng_read_config();
}

static jboolean native_init(JNIEnv* env, jobject obj) {
    ALOGD("native_init");

    sPendingCallbacks = 0;
    if(loc_ext_init() != 0)
    {
        return true;
    }

    return false;
}

static void native_disable(JNIEnv* env, jobject obj) {
    ALOGD("native_disable");
    pthread_mutex_lock(&sEventMutex);
    sPendingCallbacks |= kDisableRequest;

    sEventCond_signaled = true;
    pthread_cond_signal(&sEventCond);
    pthread_mutex_unlock(&sEventMutex);
}

static void native_cleanup(JNIEnv* env, jobject obj) {
    ALOGD("native_cleanup");
    loc_ext_cleanup();
}

static jboolean native_start(JNIEnv* env, jobject obj, jint positionMode,
                             jboolean singleFix, jint fixFrequency,
                             jint timout_sec, jstring credentials)
{
    const char* appCredentials = NULL;
    jboolean isCopy = JNI_FALSE;

    if (NULL != credentials)
        appCredentials = env->GetStringUTFChars(credentials, &isCopy);

    int result = loc_ext_set_position_mode(positionMode, singleFix, fixFrequency,
                                           timout_sec, appCredentials);
    if (isCopy)
        env->ReleaseStringUTFChars(credentials, appCredentials);

    if (result) {
        return false;
    }

    return (loc_ext_start() == 0);
}


static jboolean native_stop(JNIEnv* env, jobject obj)
{
    return (loc_ext_stop() == 0);
}

static void native_wait_for_event(JNIEnv* env, jobject obj)
{
    pthread_mutex_lock(&sEventMutex);

    if(!sEventCond_signaled)
    {
        pthread_cond_wait(&sEventCond, &sEventMutex);
    }
    sEventCond_signaled = false;

    // copy and clear the callback flags
    int pendingCallbacks = sPendingCallbacks;
    sPendingCallbacks = 0;

    // copy everything and unlock the mutex before calling into Java code to avoid the possibility
    // of timeouts in the GPS engine.
    memcpy(&sGpsLocationCopy, &sGpsLocation, sizeof(sGpsLocationCopy));
    memcpy(&sGpsLocationExtCopy, &sGpsLocationExt, sizeof(sGpsLocationExtCopy));
    memcpy(&sGpsStatusCopy, &sGpsStatus, sizeof(sGpsStatusCopy));
    memcpy(&sGpsSvStatusCopy, &sGpsSvStatus, sizeof(sGpsSvStatusCopy));
    memcpy(&sGpsSvDopCopy, &sGpsSvDop, sizeof(sGpsSvDopCopy));
    pthread_mutex_unlock(&sEventMutex);

    // the order or reporting is not based on the order we get from the lower layer, because we do not have a
    // message queue to preserve the order.
    // In absence of a queue, we try to re-order the reports so a location report will always be processed
    // after a satellite report, and before status transition notifications
    if (pendingCallbacks & kSvStatus) {
        env->CallVoidMethod(obj, method_reportSvStatus);
    }
    if (pendingCallbacks & kLocation) {

        env->CallVoidMethod(obj, method_reportLocation, (sGpsLocationCopy.flags | sGpsLocationExtCopy.flags),
                (jdouble)sGpsLocationCopy.latitude, (jdouble)sGpsLocationCopy.longitude,
                (jdouble)sGpsLocationCopy.altitude, (jdouble)sGpsLocationExtCopy.altitude_sea_level,
                (jfloat)sGpsLocationCopy.speed, (jfloat)sGpsLocationCopy.bearing,
                (jfloat)sGpsLocationCopy.accuracy,
                (jfloat)sGpsLocationExtCopy.accuracy_hor_ellipse_semi_major,
                (jfloat)sGpsLocationExtCopy.accuracy_hor_ellipse_semi_minor,
                (jfloat)sGpsLocationExtCopy.accuracy_hor_ellipse_angle,
                (jfloat)sGpsLocationExtCopy.accuracy_vertical,
                (jfloat)sGpsLocationExtCopy.confidence_horizontal,
                (jfloat)sGpsLocationExtCopy.magnetic_variation,
                (jfloat)sGpsLocationExtCopy.pdop,
                (jfloat)sGpsLocationExtCopy.hdop,
                (jfloat)sGpsLocationExtCopy.vdop,
                (jlong)sGpsLocationCopy.timestamp);
    }
    if (pendingCallbacks & kStatus) {
        env->CallVoidMethod(obj, method_reportStatus, sGpsStatusCopy.status);
    }
    if (pendingCallbacks & kVzwEvent) {
        env->CallVoidMethod(obj, method_reportVzwEvent, (jint)sVzwSpecificEvent);
    }
    if (pendingCallbacks & kDisableRequest) {
        // don't need to do anything - we are just poking so wait_for_event will return.
    }
}

static jint native_read_sv_status(JNIEnv* env, jobject obj,
        jintArray flagsArray,
        jintArray prnArray, jfloatArray snrArray, jfloatArray elevArray, jfloatArray azumArray,
        jintArray maskArray, jfloatArray dopArray)
{
    // this should only be called from within a call to reportStatus, so we don't need to lock here

    const int INDEX_VALID_MASK = 0;
    const int INDEX_PDOP = 0;
    const int INDEX_HDOP = 1;
    const int INDEX_VDOP = 2;

    jint* flags = env->GetIntArrayElements(flagsArray, 0);
    jint* prns = env->GetIntArrayElements(prnArray, 0);
    jfloat* snrs = env->GetFloatArrayElements(snrArray, 0);
    jfloat* elev = env->GetFloatArrayElements(elevArray, 0);
    jfloat* azim = env->GetFloatArrayElements(azumArray, 0);
    jint* mask = env->GetIntArrayElements(maskArray, 0);
    jfloat* dop = env->GetFloatArrayElements(dopArray, 0);

    // note: loc_ext doesn't have valididty bits for these, so we have to assume they are all valid
    flags[INDEX_VALID_MASK] = GPS_SV_STATUS_VALID_SV_LIST
                              | GPS_SV_STATUS_VALID_EPH_MASK
                              | GPS_SV_STATUS_VALID_ALM_MASK
                              | GPS_SV_STATUS_VALID_USE_MASK;

    int num_svs = sGpsSvStatusCopy.num_svs;
    for (int i = 0; i < num_svs; i++) {
        prns[i] = sGpsSvStatusCopy.sv_list[i].prn;
        snrs[i] = sGpsSvStatusCopy.sv_list[i].snr;
        elev[i] = sGpsSvStatusCopy.sv_list[i].elevation;
        azim[i] = sGpsSvStatusCopy.sv_list[i].azimuth;
    }
    mask[0] = sGpsSvStatusCopy.ephemeris_mask;
    mask[1] = sGpsSvStatusCopy.almanac_mask;
    mask[2] = sGpsSvStatusCopy.used_in_fix_mask;

    if(sGpsSvDopCopy.PDOP > DOP_VALID_THRESHOLD)
    {
        dop[INDEX_PDOP] = sGpsSvDopCopy.PDOP;
        flags[INDEX_VALID_MASK] |= GPS_SV_STATUS_VALID_PDOP;
    }

    if(sGpsSvDopCopy.HDOP > DOP_VALID_THRESHOLD)
    {
        dop[INDEX_HDOP] = sGpsSvDopCopy.HDOP;
        flags[INDEX_VALID_MASK] |= GPS_SV_STATUS_VALID_HDOP;
    }

    if(sGpsSvDopCopy.VDOP > DOP_VALID_THRESHOLD)
    {
        dop[INDEX_VDOP] = sGpsSvDopCopy.VDOP;
        flags[INDEX_VALID_MASK] |= GPS_SV_STATUS_VALID_VDOP;
    }

    env->ReleaseIntArrayElements(flagsArray, flags, 0);
    env->ReleaseIntArrayElements(prnArray, prns, 0);
    env->ReleaseFloatArrayElements(snrArray, snrs, 0);
    env->ReleaseFloatArrayElements(elevArray, elev, 0);
    env->ReleaseFloatArrayElements(azumArray, azim, 0);
    env->ReleaseIntArrayElements(maskArray, mask, 0);
    env->ReleaseFloatArrayElements(dopArray, dop, 0);
    return num_svs;
}

static void native_set_agps_server(JNIEnv* env, jobject obj,
        jint type, jstring hostname, jint port)
{
    const char *c_hostname = env->GetStringUTFChars(hostname, NULL);
    loc_ext_set_server(type, c_hostname, port);
    env->ReleaseStringUTFChars(hostname, c_hostname);
}

static void native_reset_gps(JNIEnv* env, jobject obj, jint bits)
{
    loc_ext_delete_aiding_data(bits);
}

} // namespace vzw


/* Class path of the Java file this native code interfaces with*/
static const char *classPathName = "com/qualcomm/location/vzw_library/imp/NativeMethods";

/* Array to store all methods
   supported in this JNI file
*/
static JNINativeMethod methods[] = {
  {"class_init_native","()V",(void*)vzw::class_init_native},
  {"native_init", "()Z", (void*)vzw::native_init},
  {"native_disable", "()V", (void*)vzw::native_disable},
  {"native_cleanup", "()V", (void*)vzw::native_cleanup},
  {"native_start", "(IZIILjava/lang/String;)Z", (void*)vzw::native_start},
  {"native_stop", "()Z", (void*)vzw::native_stop},
  {"native_wait_for_event", "()V", (void*)vzw::native_wait_for_event},
  {"native_read_sv_status", "([I[I[F[F[F[I[F)I", (void*)vzw::native_read_sv_status},
  {"native_set_agps_server", "(ILjava/lang/String;I)V", (void*)vzw::native_set_agps_server},
  {"native_reset_gps", "(I)V", (void*)vzw::native_reset_gps}
};


/*===========================================================================
FUNCTION    main

DESCRIPTION
 Register several native methods for one class.

DEPENDENCIES
   N/A

RETURN VALUE
   JNI_TRUE: success
   JNI_FALSE: failure

SIDE EFFECTS
   N/A

===========================================================================*/

static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*===========================================================================
FUNCTION    registerNatives

DESCRIPTION
  Register native methods for all classes we know about.

DEPENDENCIES
   N/A

RETURN VALUE
   JNI_TRUE: success
   JNI_FALSE: failure

SIDE EFFECTS
   N/A

===========================================================================*/
/*
 *
 */
static int registerNatives(JNIEnv* env)
{
  if (!registerNativeMethods(env, classPathName,
                 methods, sizeof(methods) / sizeof(methods[0]))) {
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

/*
  Union to store the native environment variables
*/

typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

/*===========================================================================
FUNCTION    JNI_OnLoad

DESCRIPTION
  Executed when Native library is loaded. Register the native routines

DEPENDENCIES
   N/A

RETURN VALUE
 JNI_VERSION: Success
 -1 : Failure
SIDE EFFECTS
   N/A

===========================================================================*/
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;

    ALOGD (" VzwNativeGpsLocationProvider JNI Onload ");
    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (!registerNatives(env)) {
        ALOGE("registerNatives failed");
    }

    result = JNI_VERSION_1_4;

bail:
    return result;
}




