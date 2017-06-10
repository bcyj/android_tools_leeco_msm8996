/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "WifiFtm"

#include <jni.h>
#include <ScopedUtfChars.h>
#include <utils/String16.h>
#include <cutils/jstring.h>
#include <stdint.h>
#include "cutils/log.h"
#include "wifi_ftm.h"

#define BUF_SIZE 256

static jint doCommand(const char* fmt, ...) {
    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int byteCount = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (byteCount < 0 || byteCount >= BUF_SIZE) {
        return -1;
    }

    return wifi_ftm_send_cmd(buf);
}

static jint wifi_ftm_start(JNIEnv* env, jobject) {
    ::wifi_ftmd_start();
    return doCommand("START FTM");
}

static jint wifi_ftm_stop(JNIEnv* env, jobject) {
    return doCommand("STOP FTM");
}

static jint wifi_ftm_set_channel(JNIEnv* env, jobject, jint channel) {
    return doCommand("SET CHANNEL %d", channel);
}

static jint wifi_ftm_set_tx_power(JNIEnv* env, jobject, jint tx_pow) {
    return doCommand("SET TX_POWER %d", tx_pow);
}

static jint wifi_ftm_set_power_mode(JNIEnv* env, jobject, jint pwr_mode) {
    return doCommand("SET PWR_MODE %d", pwr_mode);
}

static jint wifi_ftm_set_tx_rate(JNIEnv* env, jobject, jstring tx_rate) {
    ScopedUtfChars rate(env, tx_rate);
    if (rate.c_str() == NULL)
        return -1;

    return doCommand("SET TX_RATE %s", rate.c_str());
}

static jint wifi_ftm_tx_start(JNIEnv* env, jobject) {
    return doCommand("START TX");
}

static jint wifi_ftm_tx_stop(JNIEnv* env, jobject) {
    return doCommand("STOP TX");
}

static jint wifi_ftm_rx_start(JNIEnv* env, jobject) {
    return doCommand("START RX");
}

static jint wifi_ftm_rx_stop(JNIEnv* env, jobject) {
    return doCommand("STOP RX");
}

static jint wifi_ftm_scw_start(JNIEnv* env, jobject) {
    return doCommand("START SCW");
}

static jint wifi_ftm_scw_stop(JNIEnv* env, jobject) {
    return doCommand("STOP SCW");
}

static jstring wifi_ftm_rx_report(JNIEnv* env, jobject) {
    char reply[64];
    size_t reply_len = 63;
    wifi_ftm_send_cmd("REPORT RX", reply, &reply_len);
    reply[reply_len] = '\0';
    android::String16 str((char *) reply);
    return env->NewString((const jchar *) str.string(), str.size());
}

static JNINativeMethod wifiFtmMethods[] = {
        { "StartTest", "()I", (void *) wifi_ftm_start },
        { "StopTest", "()I", (void *) wifi_ftm_stop },
        { "SetChannel", "(I)I", (void *) wifi_ftm_set_channel },
        { "SetTxPower", "(I)I", (void *) wifi_ftm_set_tx_power },
        { "SetTxRate", "(Ljava/lang/String;)I", (void *) wifi_ftm_set_tx_rate },
        { "SetPowerMode", "(I)I", (void *) wifi_ftm_set_power_mode },
        { "StartTx", "()I", (void *) wifi_ftm_tx_start },
        { "StopTx", "()I", (void *) wifi_ftm_tx_stop },
        { "StartRx", "()I", (void *) wifi_ftm_rx_start },
        { "StopRx", "()I", (void *) wifi_ftm_rx_stop },
        { "StartSCW", "()I", (void *) wifi_ftm_scw_start },
        { "StopSCW", "()I", (void *) wifi_ftm_scw_stop },
        { "ReportRx", "()Ljava/lang/String;", (void *) wifi_ftm_rx_report },
};

/*
 * Register the methods.
 */
static int registerMethods(JNIEnv* env, const char* className, JNINativeMethod* methods,
        int numMethods) {
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("register methods, unable to find class '%s'\n", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
        ALOGE("register methods, failed for '%s'\n", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Load the functions.
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        ALOGE("ERROR Message: GetEnv failed\n");
        return -1;
    }

    if (!registerMethods(env, "com/qti/fcctest/FccTestActivity",
            wifiFtmMethods,
            sizeof(wifiFtmMethods) / sizeof(wifiFtmMethods[0]))) {
        ALOGE("ERROR Message: libwifi_ftmd registration failed\n");
        return -1;
    }

    return JNI_VERSION_1_6;
}

