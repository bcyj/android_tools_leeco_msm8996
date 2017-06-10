/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

  Not a Contribution, Apache license notifications and
  license are retained for attribution purposes only.
=============================================================================*/

/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2011,2012, The Linux Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "LocSvc_jni"
#define LOG_NDEBUG 0

#include "JNIHelp.h"
#include "jni.h"
#include "hardware/hardware.h"
#include "hardware/gps.h"
#include "hardware_legacy/power.h"
#include "utils/Log.h"
#include "utils/misc.h"
#include "android_runtime/AndroidRuntime.h"
#include "loc_extended.h"

#include <string.h>
#include <pthread.h>

static jmethodID method_reportAGpsStatus;
static jmethodID method_setEngineCapabilities;
static jmethodID method_xtraDownloadRequest;
static jmethodID method_reportXtraServer;
static jmethodID method_reportNiNotification;
static jmethodID method_requestUtcTime;

static jobject mCallbacksObj = NULL;

#define WAKE_LOCK_NAME  "LocationService"

static void checkAndClearExceptionFromCallback(JNIEnv* env, const char* methodName) {
    if (env->ExceptionCheck()) {
        ALOGE("An exception was thrown by callback '%s'.", methodName);
        env->ExceptionClear();
    }
}

static void set_capabilities_callback(uint32_t capabilities)
{
    ALOGD("set_capabilities_callback: %ld\n", (long ) capabilities);
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_setEngineCapabilities, capabilities);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void acquire_wakelock_callback()
{
    acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKE_LOCK_NAME);
}

static void release_wakelock_callback()
{
    release_wake_lock(WAKE_LOCK_NAME);
}

static void request_utc_time_callback()
{
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_requestUtcTime);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static pthread_t create_thread_callback(const char* name, void (*start)(void *), void* arg)
{
    return (pthread_t)android::AndroidRuntime::createJavaThread(name, start, arg);
}

GpsExtCallbacks sGpsExtCallbacks = {
    sizeof(GpsExtCallbacks),
    set_capabilities_callback,
    acquire_wakelock_callback,
    release_wakelock_callback,
    create_thread_callback,
    request_utc_time_callback,
};

static void xtra_download_request_callback()
{
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_xtraDownloadRequest);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void report_xtra_server_callback(const char* server1, const char* server2, const char* server3)
{
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();

    jstring java_string_server1 = env->NewStringUTF(server1);
    jstring java_string_server2 = env->NewStringUTF(server2);
    jstring java_string_server3 = env->NewStringUTF(server3);

    env->CallVoidMethod(mCallbacksObj, method_reportXtraServer,
                        java_string_server1, java_string_server2, java_string_server3);

    if (java_string_server1)
        env->DeleteLocalRef(java_string_server1);
    if (java_string_server2)
        env->DeleteLocalRef(java_string_server2);
    if (java_string_server3)
        env->DeleteLocalRef(java_string_server3);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

GpsXtraExtCallbacks sGpsXtraExtCallbacks = {
    xtra_download_request_callback,
    create_thread_callback,
    report_xtra_server_callback,
};

static void agps_status_extended_callback(AGpsExtStatus* agps_status)
{
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();
    jbyteArray byteArray = NULL;
    jstring ssid_string = NULL;
    jstring password_string = NULL;

    // there is neither ipv4 now ipv6 addresses unless this is true
    if (agps_status->size >
        ((char*)(&(agps_status->ipv4_addr)) - (char*)agps_status)) {
        // if we have a valid ipv4 address
        if (agps_status->ipv4_addr != 0xFFFFFFFF) {
            jbyte ipv4[4];
            byteArray = env->NewByteArray(4);
            ALOG_ASSERT(byteArray, "Native could not create new byte[]");

            //endianess transparent convertion from int to char[]
            ipv4[0] = (jbyte)agps_status->ipv4_addr;
            ipv4[1] = (jbyte)(agps_status->ipv4_addr>>8);
            ipv4[2] = (jbyte)(agps_status->ipv4_addr>>16);
            ipv4[3] = (jbyte)(agps_status->ipv4_addr>>24);

            env->SetByteArrayRegion(byteArray, 0, 4, (const jbyte *)ipv4);
        } else if (agps_status->size >= sizeof(AGpsExtStatus)) {
            // newest version of AGpsStatus struct, which has ipv6
            // address. So we go with that. We won't get here if
            // we have a valid ipv4 address.
            byteArray = env->NewByteArray(16);
            ALOG_ASSERT(byteArray, "Native could not create new byte[]");
            env->SetByteArrayRegion(byteArray, 0, 16,
                                     (const jbyte *)&(agps_status->ipv6_addr));
        }

        if (agps_status->size >= sizeof(AGpsExtStatus)) {
            if (agps_status->ssid[0] != '\0') {
                ssid_string = env->NewStringUTF(agps_status->ssid);
                if (agps_status->password[0] != '\0') {
                    password_string = env->NewStringUTF(agps_status->password);
                }
            }
        }
    }

    env->CallVoidMethod(mCallbacksObj,
                        method_reportAGpsStatus,
                        agps_status->type,
                        agps_status->status,
                        byteArray,
                        ssid_string,
                        password_string);

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
    if (byteArray != NULL) {
        env->DeleteLocalRef(byteArray);
    }
}

AGpsExtCallbacks sAGpsExtCallbacks = {
    agps_status_extended_callback,
    create_thread_callback,
};

static void gps_ni_notify(GpsNiNotification *notification)
{
    ALOGD("gps_ni_notify\n");
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();
    jstring requestor_id = env->NewStringUTF(notification->requestor_id);
    jstring text = env->NewStringUTF(notification->text);
    jstring extras = env->NewStringUTF(notification->extras);

    if (requestor_id && text && extras) {
        env->CallVoidMethod(mCallbacksObj, method_reportNiNotification,
            notification->notification_id, notification->ni_type,
            notification->notify_flags, notification->timeout,
            notification->default_response, requestor_id, text,
            notification->requestor_id_encoding,
            notification->text_encoding, extras);
    } else {
        ALOGE("out of memory in gps_ni_notify\n");
    }

    if (requestor_id)
        env->DeleteLocalRef(requestor_id);
    if (text)
        env->DeleteLocalRef(text);
    if (extras)
        env->DeleteLocalRef(extras);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

GpsNiExtCallbacks sGpsNiExtCallbacks = {
    gps_ni_notify,
    create_thread_callback,
};

static void LocationService_class_init_native(JNIEnv* env, jclass clazz) {
    int err;
    hw_module_t* module;

    method_reportAGpsStatus = env->GetMethodID(clazz, "reportAGpsStatus", "(II[BLjava/lang/String;Ljava/lang/String;)V");
    method_setEngineCapabilities = env->GetMethodID(clazz, "setEngineCapabilities", "(I)V");
    method_xtraDownloadRequest = env->GetMethodID(clazz, "xtraDownloadRequest", "()V");
    method_reportNiNotification = env->GetMethodID(clazz, "reportNiNotification",
            "(IIIIILjava/lang/String;Ljava/lang/String;IILjava/lang/String;)V");
    method_requestUtcTime = env->GetMethodID(clazz,"requestUtcTime","()V");
    method_reportXtraServer = env->GetMethodID(clazz,"reportXtraServer","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

}

static jboolean LocationService_init(JNIEnv* env, jobject obj)
{
    // this must be set before calling into the HAL library
    if (!mCallbacksObj)
        mCallbacksObj = env->NewGlobalRef(obj);

    loc_extended_init(&sGpsExtCallbacks);
    loc_extended_agps_init(&sAGpsExtCallbacks);
    loc_extended_xtra_init(&sGpsXtraExtCallbacks);
    loc_extended_ni_init(&sGpsNiExtCallbacks);

    return true;
}

static void LocationService_cleanup(JNIEnv* env, jobject obj)
{
    loc_extended_cleanup();
}

static void LocationService_inject_time(JNIEnv* env, jobject obj,
        jlong time, jlong timeReference, jint uncertainty)
{
    loc_extended_inject_time(time, timeReference, uncertainty);
}

static void LocationService_inject_xtra_data(JNIEnv* env, jobject obj,
        jbyteArray data, jint length)
{
    jbyte* bytes = (jbyte *)env->GetPrimitiveArrayCritical(data, 0);
    loc_extended_xtra_inject_data((char *)bytes, length);
    env->ReleasePrimitiveArrayCritical(data, bytes, JNI_ABORT);
}

static void LocationService_agps_data_conn_open(JNIEnv* env, jobject obj,
        jint agpsType, jstring apn, jint bearerType)
{
    if (apn == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }
    const char *apnStr = env->GetStringUTFChars(apn, NULL);
    loc_extended_agps_open(agpsType, apnStr, bearerType);
    env->ReleaseStringUTFChars(apn, apnStr);
}

static void LocationService_agps_data_conn_closed(JNIEnv* env, jobject obj,
        jint agpsType)
{
    loc_extended_agps_closed(agpsType);
}

static void LocationService_agps_data_conn_failed(JNIEnv* env, jobject obj,
        jint agpsType)
{
    loc_extended_agps_open_failed(agpsType);
}

static void LocationService_send_ni_response(JNIEnv* env, jobject obj,
      jint notifId, jint response)
{
    loc_extended_ni_respond(notifId, response);
}

static jint LocationService_request_xtra_server(JNIEnv* env, jobject obj)
{
    jint result = -1;

    result = loc_extended_xtra_request_server();

    return result;
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
    {"class_init_native", "()V", (void *)LocationService_class_init_native},
    {"native_init", "()Z", (void*)LocationService_init},
    {"native_cleanup", "()V", (void*)LocationService_cleanup},
    {"native_inject_time", "(JJI)V", (void*)LocationService_inject_time},
    {"native_inject_xtra_data", "([BI)V", (void*)LocationService_inject_xtra_data},
    {"native_agps_data_conn_open", "(ILjava/lang/String;I)V", (void*)LocationService_agps_data_conn_open},
    {"native_agps_data_conn_closed", "(I)V", (void*)LocationService_agps_data_conn_closed},
    {"native_agps_data_conn_failed", "(I)V", (void*)LocationService_agps_data_conn_failed},
    {"native_send_ni_response", "(II)V", (void*)LocationService_send_ni_response},
    {"native_request_xtra_server", "()I", (void*)LocationService_request_xtra_server},
};

int register_LocationService(JNIEnv* env)
{
    return jniRegisterNativeMethods(env, "com/qualcomm/location/LocationService", sMethods, NELEM(sMethods));
}


