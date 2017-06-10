/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       GeoFence service module

GENERAL DESCRIPTION
  GeoFence service module

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
=============================================================================*/

#undef com_qualcomm_location_GeoFenceKeeper_TIME_UNIT_IN_MS
#define com_qualcomm_location_GeoFenceKeeper_TIME_UNIT_IN_MS 65536L

#define LOG_TAG "GeoFenceKeep_jni"
#define LOG_NDEBUG 0
#define GPS_CONF_FILE "/etc/gps.conf"

#include "JNIHelp.h"
#include "jni.h"
#include "loc_cfg.h"
#include "GeoFencerProxyMsgs.h"
#include "android_runtime/AndroidRuntime.h"
#include "hardware/gps.h"
#include <dlfcn.h>
#include <log_util.h>

using namespace android;

static jobject mGeoFenceKeeper = NULL;
static const GpsGeofencingInterface* sGpsGeofencingInterface = NULL;
static jmethodID method_breachEvent;

static void breachEventCb(int engID, GpsLocation* location, int32_t transition, GpsUtcTime timestamp) {
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    int whichWay = -1;

    if (mGeoFenceKeeper == NULL) {
        ALOGE(" mGeoFenceKeeper == NULL ", __FUNCTION__);
        return;
    }

    switch (transition) {
        case GPS_GEOFENCE_ENTERED:
            whichWay = 0;
            break;
        case GPS_GEOFENCE_EXITED:
            whichWay = 1;
            break;
        case GPS_GEOFENCE_UNCERTAIN:
            whichWay = 2;
            break;
        default:
            LOC_LOGE("unknown transition ");
            break;
    }

    env->CallVoidMethod(mGeoFenceKeeper, method_breachEvent,
                        engID, whichWay, (jdouble)location->latitude, (jdouble)location->longitude);
    if (env->ExceptionCheck()) {
        ALOGE("Exception in %s", __FUNCTION__);
        env->ExceptionClear();
    }
}

static void geofence_status_callback(int32_t status, GpsLocation* last_location) {

    LOC_LOGW("geofence_add_callback -- EVENT STATUS status=%d", status);
}

static void geofence_add_callback(int32_t geofence_id, int32_t status) {

    LOC_LOGW("EVENT ADD geofence_id=%d status=%d", geofence_id, status);
}

static void geofence_remove_callback(int32_t geofence_id, int32_t status) {

    LOC_LOGW("EVENT REMOVE geofence_id=%d status=%d", geofence_id, status);
}

static void geofence_pause_callback(int32_t geofence_id, int32_t status) {

    LOC_LOGW("EVENT PAUSE geofence_id=%d status=%d", geofence_id, status);
}

static void geofence_resume_callback(int32_t geofence_id, int32_t status) {

    LOC_LOGW("EVENT RESUME geofence_id=%d status=%d", geofence_id, status);
}

static pthread_t create_thread_callback(const char* name, void (*start)(void *), void* arg)
{
    return (pthread_t)AndroidRuntime::createJavaThread(name, start, arg);
}

GpsGeofenceCallbacks sGpsGeofenceCallbacks = {
    breachEventCb,
    geofence_status_callback,
    geofence_add_callback,
    geofence_remove_callback,
    geofence_pause_callback,
    geofence_resume_callback,
    create_thread_callback
};

/*
 * Class:     com_qualcomm_location_GeoFenceKeeper
 * Method:    startGeoFence
 * Signature: (IDDF)I
 */
extern "C" JNIEXPORT jlong JNICALL
Java_com_qualcomm_location_GeoFenceKeeper_startGeoFence
(JNIEnv *env, jobject obj, jint txID, jdouble latitude, jdouble longitude, jfloat radius)
{
    if (NULL == mGeoFenceKeeper) {
        mGeoFenceKeeper = env->NewGlobalRef(obj);
    }

    if (sGpsGeofencingInterface != NULL) {
        sGpsGeofencingInterface->add_geofence_area(txID,
                                                  latitude,
                                                  longitude,
                                                  radius,
                                                  GPS_GEOFENCE_UNCERTAIN,
                                                  GPS_GEOFENCE_ENTERED|GPS_GEOFENCE_EXITED,
                                                  60000,
                                                  0);
        return 0;
    } else {
        ALOGE("Geofence interface not available");
    }
    return -1;
}

/*
 * Class:     com_qualcomm_location_GeoFenceKeeper
 * Method:    stopGenFence
 * Signature: (I)V
 */
extern "C" JNIEXPORT void JNICALL
Java_com_qualcomm_location_GeoFenceKeeper_stopGenFence
(JNIEnv *env, jobject obj, jint engID)
{
    if (sGpsGeofencingInterface != NULL) {
        sGpsGeofencingInterface->remove_geofence_area(engID);
    } else {
        ALOGE("Geofence interface remove_geofence_area is not available");
    }
}

/*
 * Class:     com_qualcomm_location_GeoFenceKeeper
 * Method:    classInit
 * Signature: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_com_qualcomm_location_GeoFenceKeeper_classInit
(JNIEnv *env, jclass clazz)
{
    method_breachEvent = env->GetMethodID(clazz, "breachEvent", "(IIDD)V");
    sGpsGeofencingInterface = gps_geofence_get_interface_ext();

    // init the interface
    if (sGpsGeofencingInterface) {
        sGpsGeofencingInterface->init(&sGpsGeofenceCallbacks);
    } else {
        LOC_LOGE ("get geofence interface failed.");
    }
}

/*
 * Class:     com_qualcomm_location_GeoFenceKeeper
 * Method:    testBreach
 * Signature: (I)V
 */
extern "C" JNIEXPORT void JNICALL
Java_com_qualcomm_location_GeoFenceKeeper_testBreach
(JNIEnv *env, jclass clazz, jint breacherID)
{
#ifdef ON_TARGET_TEST
    GpsLocation location;
    location.latitude = 37;
    location.longitude = -120;
    breachEventCb(breacherID, &location, GPS_GEOFENCE_EXITED, 0);
#endif
}
