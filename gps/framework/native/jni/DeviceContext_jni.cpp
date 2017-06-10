/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_TAG "DeviceContext_jni"
#define LOG_NDEBUG 0

#include "JNIHelp.h"
#include "jni.h"
#include "loc_cfg.h"
#include "LBSAdapter.h"
#include "LocDualContext.h"
#include "android_runtime/AndroidRuntime.h"

using namespace android;
using namespace loc_core;

struct LbsShutDownMsg : public LocMsg {
    ContextBase* mContext;
    inline LbsShutDownMsg(ContextBase* context) :
        LocMsg(), mContext(context) {}
    inline ~LbsShutDownMsg() {
        delete mContext->getMsgTask();
        delete mContext;
    }
    inline void proc() const {
        LBSApiBase* lbsApi =
            (LBSApiBase*)mContext->getLocApi()->getSibling();
        lbsApi->shutdown();
    }
};

class LBSShutDownAdapter : public LBSAdapterBase {
public:
    inline LBSShutDownAdapter(ContextBase* context) :
        LBSAdapterBase(0, context) {}
};

static LBSAdapter* nLBSAdapter = NULL;
static jobject jDeviceContext = NULL;

static void classInit(JNIEnv* env, jclass clazz) {
}

static void instanceInit(JNIEnv *env, jobject obj) {
    if (NULL == nLBSAdapter) {
        nLBSAdapter = LBSAdapter::get(0,
                                      (loc_core::MsgTask::tCreate)
                                      AndroidRuntime::createJavaThread,
                                      NULL);
    }
    if (NULL == jDeviceContext) {
        jDeviceContext = env->NewGlobalRef(obj);
    }
}

static void chargerStatusInject(JNIEnv *env, jobject obj,
                                jint status) {
    if (nLBSAdapter) {
        nLBSAdapter->chargerStatusInject(status);
    }
}

static void deviceShutdown(JNIEnv *env, jobject obj) {
    const MsgTask* msgTask(new MsgTask((MsgTask::tCreate)
                                       AndroidRuntime::createJavaThread,
                                       "LBS_shutdown"));
    ContextBase* context(new ContextBase(msgTask, 0,
                                         LocDualContext::mLBSLibName));
    LBSShutDownAdapter adapter(context);
    adapter.sendMsg(new LbsShutDownMsg(context));
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
    {"native_dc_class_init", "()V", (void *)classInit},
    {"native_dc_init", "()V", (void *)instanceInit},
    {"native_dc_shutdown", "()V", (void *)deviceShutdown},
    {"native_dc_charger_status_inject", "(I)V", (void *)chargerStatusInject}
};

static int register_DeviceContext(JNIEnv* env) {
    return jniRegisterNativeMethods(env, "com/qualcomm/location/DeviceContext",
                                    sMethods, NELEM(sMethods));
}

extern int register_UlpEngine(JNIEnv* env);
extern int register_LocationService(JNIEnv* env);
extern int register_Wiper(JNIEnv* env);
extern int register_RilInfoMonitor(JNIEnv* env);

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed!");
        return result;
    }
    ALOG_ASSERT(env, "Could not retrieve the env!");

    register_DeviceContext(env);
    register_UlpEngine(env);
    register_LocationService(env);
    register_Wiper(env);
    register_RilInfoMonitor(env);

    return JNI_VERSION_1_4;
}

