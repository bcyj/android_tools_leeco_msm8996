/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_TAG "RilInfoMonitor_jni"
#define LOG_NDEBUG 0

#include "JNIHelp.h"
#include "jni.h"
#include "loc_cfg.h"
#include "LBSAdapter.h"
#include "android_runtime/AndroidRuntime.h"

using namespace android;

static LBSAdapter* nLBSAdapter = NULL;
static jobject jRilInfoMonitor = NULL;

static void classInit(JNIEnv* env, jclass clazz) {
}

static void instanceInit(JNIEnv *env, jobject obj) {
    if (NULL == nLBSAdapter) {
        nLBSAdapter = LBSAdapter::get(0,
                                      (loc_core::MsgTask::tCreate)
                                      AndroidRuntime::createJavaThread,
                                      NULL);
    }
    if (NULL == jRilInfoMonitor) {
        jRilInfoMonitor = env->NewGlobalRef(obj);
    }
}

static void cinfoInject(JNIEnv *env, jobject obj,
                        jint cid, jint lac, jint mnc,
                        jint mcc, jboolean roaming) {
    if (nLBSAdapter)
        nLBSAdapter->cinfoInject(cid, lac, mnc, mcc, roaming);
}

static void oosInform(JNIEnv *env, jobject obj) {
    if (nLBSAdapter)
        nLBSAdapter->oosInform();
}

static void niSuplInit(JNIEnv *env, jobject obj,
                       jbyteArray data, jint length) {
    if (nLBSAdapter) {
        jbyte* bytes = (jbyte *)env->GetPrimitiveArrayCritical(data, 0);
        nLBSAdapter->niSuplInit((char *)bytes, length);
        env->ReleasePrimitiveArrayCritical(data, bytes, JNI_ABORT);
    }
}

static void chargerStatusInject(JNIEnv *env, jobject obj,
                                jint status) {
    if (nLBSAdapter) {
        nLBSAdapter->chargerStatusInject(status);
    }
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
    {"native_rm_class_init", "()V", (void *)classInit},
    {"native_rm_init", "()V", (void *)instanceInit},
    {"native_rm_cinfo_inject", "(IIIIZ)V", (void *)cinfoInject},
    {"native_rm_oos_inform", "()V", (void *)oosInform},
    {"native_rm_ni_supl_init", "([BI)V", (void *)niSuplInit},
};

int register_RilInfoMonitor(JNIEnv* env) {
    return jniRegisterNativeMethods(env, "com/qualcomm/location/RilInfoMonitor",
                                    sMethods, NELEM(sMethods));
}
