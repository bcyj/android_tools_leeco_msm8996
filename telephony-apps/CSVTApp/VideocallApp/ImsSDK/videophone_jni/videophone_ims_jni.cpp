/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "videocall_jni"
#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0
#include <cutils/log.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "videophone_impl.h"

extern JavaVM *jvmPtr;
static VtImplApis *vt_apis = NULL;
static jobject mediaHandlerObject = NULL;

static jint dpl_init(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);
    jint error = -1;
    if (vt_apis && vt_apis->initImsThinClient) {
        error = vt_apis->initImsThinClient();
    }
    return error;
}

static void dpl_deinit(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);
    if (vt_apis && vt_apis->deInitImsThinClient) {
        vt_apis->deInitImsThinClient();
    }
}

static jint dpl_handleRawFrame(JNIEnv *e, jobject o, jbyteArray frame) {
    jint ret = 0;
    jsize size;
    jbyte *bytes;
    uint8_t *nativeframe;

    if (!vt_apis || !vt_apis->frameToEncode) return ret;
    if (frame != NULL) {
        jsize size = e->GetArrayLength(frame);
        bytes = e->GetByteArrayElements(frame, JNI_FALSE);
        vt_apis->frameToEncode((unsigned short *)bytes,(int)size);
        e->ReleaseByteArrayElements(frame, bytes, JNI_ABORT);
    } else {
        ALOGD("%s: Received a null frame", __func__);
        ret = -1;
    }
    return ret;
}

static int dpl_setSurface(JNIEnv *e, jobject o, jobject osurface) {
    ALOGD("%s", __func__);
    if (vt_apis && vt_apis->setFarEndSurface) {
        vt_apis->setFarEndSurface(e, osurface);
    }
    return 0;
}

static void dpl_setDeviceOrientation(JNIEnv *e, jobject o, jint orientation) {
    ALOGD("%s", __func__);
    if (vt_apis && vt_apis->setDeviceOrientation) {
        vt_apis->setDeviceOrientation(orientation);
    }
}

static short dpl_getNegotiatedFPS(JNIEnv *e, jobject o) {
    short def = 20;
    ALOGD("%s", __func__);

    if (vt_apis && vt_apis->getNegotiatedFPS) {
        return vt_apis->getNegotiatedFPS();
    }
    return def;
}

static int dpl_getNegotiatedHeight(JNIEnv *e, jobject o) {
    int def = 240;
    ALOGD("%s", __func__);

    if (vt_apis && vt_apis->getNegotiatedHeight) {
        return vt_apis->getNegotiatedHeight();
    }
    return def;
}

static int dpl_getNegotiatedWidth(JNIEnv *e, jobject o) {
    int def = 320;
    ALOGD("%s", __func__);

    if(vt_apis && vt_apis->getNegotiatedWidth) {
        return vt_apis->getNegotiatedWidth();
    }
    return def;
}

static int dpl_getUIOrientationMode(JNIEnv *e, jobject o) {
    int def = -1;
    ALOGD("%s", __func__);

    if(vt_apis && vt_apis->getUIOrientationMode) {
        return vt_apis->getUIOrientationMode();
    }
    return def;
}

static void onMediaEvent(uint16_t eventId) {
    bool threadAttached = false;

    ALOGD("%s", __func__);
    JNIEnv *e;
    if (jvmPtr->GetEnv((void **) &e, JNI_VERSION_1_6) != JNI_OK) {
        ALOGD("Attaching IMS thread");
        if (jvmPtr->AttachCurrentThread(&e, NULL) != JNI_OK) {
            ALOGE("videocall callback: AttachCurrentThread error");
            return;
        }
        threadAttached = true;
    }

    if(gClassMediaHandler) {
        jmethodID mid = e->GetMethodID(gClassMediaHandler, "onMediaEvent", "(I)V");
        if (mid == 0) {
            ALOGE("videocall callback: GetMethodID error");
            return;
        }

        e->CallVoidMethod(mediaHandlerObject, mid, (int) eventId);
    }
    else {
        ALOGE("Error: gClassMediaHandler is null.");
    }

    if (threadAttached) {
        jvmPtr->DetachCurrentThread();
        ALOGD("Detached current thread");
    }
}

static void dpl_registerForImsEvent(JNIEnv *e, jobject o, jobject objMediaHandler) {
    ALOGD("%s", __func__);
    if (vt_apis && vt_apis->registerAppEventCallback) {
        vt_apis->registerAppEventCallback(&onMediaEvent);
    }

    //Cache the MediaHandler instance
    mediaHandlerObject = (jclass) e->NewGlobalRef(objMediaHandler);
}

static JNINativeMethod sMethods[] =
{
    {"nativeInit", "()I", (void *)dpl_init},
    {"nativeDeInit", "()V", (void *)dpl_deinit},
    {"nativeHandleRawFrame", "([B)V", (void *)dpl_handleRawFrame},
    {"nativeSetSurface", "(Landroid/graphics/SurfaceTexture;)I", (void *)dpl_setSurface},
    {"nativeSetDeviceOrientation", "(I)V", (void *)dpl_setDeviceOrientation},
    {"nativeGetNegotiatedFPS", "()S", (void *)dpl_getNegotiatedFPS},
    {"nativeGetNegotiatedHeight", "()I", (void *)dpl_getNegotiatedHeight},
    {"nativeGetNegotiatedWidth", "()I", (void *)dpl_getNegotiatedWidth},
    {"nativeGetUIOrientationMode", "()I", (void *)dpl_getUIOrientationMode},
    {"nativeRegisterForMediaEvents", "(Lcom/qualcomm/qti/ims/internal/media/MediaHandler;)V"
        , (void *)dpl_registerForImsEvent}
};

#define DEFAULT_IMPL_LIB_PATH "/vendor/lib/lib-imsvt.so"
#define IMPL_LIB_PROPERTY_NAME "vt.impl.lib"

#define IMPL_SYM_INIT        "initImsThinClient"
#define IMPL_SYM_FRAME       "frameToEncode"
#define IMPL_SYM_SET_SURFACE "setFarEndSurface"
#define IMPL_SYM_SET_DEVICE_ORIENTATION "setDeviceOrientation"
#define IMPL_SYM_DEINIT      "deInitImsThinClient"
#define IMPL_SYM_NEG_FPS     "getNegotiatedFPS"
#define IMPL_SYM_NEG_HEIGHT  "getNegotiatedHeight"
#define IMPL_SYM_NEG_WIDTH   "getNegotiatedWidth"
#define IMPL_SYM_UI_ORIENTATION_MODE   "getUIOrientationMode"
#define IMPL_SYM_REGISTER    "registerAppEventCallback"

struct VtImplApis *vt_load_impl_lib(const char *path)
{
    struct VtImplApis *ret = NULL;
    void *handle;

    if (!(handle = dlopen(path, RTLD_NOW))) {
        ALOGE("Error loading library %s: %s\n", path, dlerror());
        goto finish;
    }

    ret = (VtImplApis *)calloc(sizeof(struct VtImplApis), 1);
    if (!ret) goto close_and_finish;

    ret->initImsThinClient = (VtImplInitFun) dlsym(handle,IMPL_SYM_INIT);
    ret->frameToEncode = (VtImplFrameFun) dlsym(handle, IMPL_SYM_FRAME);
    ret->setFarEndSurface = (VtImplSetSurfFun) dlsym(handle, IMPL_SYM_SET_SURFACE);
    ret->setDeviceOrientation = (VtImplSetDeviceOrient) dlsym(handle, IMPL_SYM_SET_DEVICE_ORIENTATION);
    ret->deInitImsThinClient = (VtImplDeinitFun) dlsym(handle, IMPL_SYM_DEINIT);
    ret->getNegotiatedFPS = (VtImplUint32VoidFunc) dlsym(handle, IMPL_SYM_NEG_FPS);
    ret->getNegotiatedHeight = (VtImplUint32VoidFunc) dlsym(handle, IMPL_SYM_NEG_HEIGHT);
    ret->getNegotiatedWidth = (VtImplUint32VoidFunc) dlsym(handle, IMPL_SYM_NEG_WIDTH);
    ret->getUIOrientationMode = (VtImplUint32VoidFunc) dlsym(handle, IMPL_SYM_UI_ORIENTATION_MODE);
    ret->registerAppEventCallback = (VtImplRegisterCbFun) dlsym(handle, IMPL_SYM_REGISTER);

    return ret;
close_and_finish:
    if (dlclose(handle)) {
        ALOGE("Error closing library %s: %s\n", path, dlerror());
    }
finish:
    return ret;
}

#define METHODS_LEN (sizeof(sMethods) / sizeof(sMethods[0]))

int register_videophone_Dpl(JNIEnv *e) {
    char libpath[PROPERTY_VALUE_MAX];

    ALOGD("%s\n", __func__);

    jclass klass;

    klass = e->FindClass("com/qualcomm/qti/ims/internal/media/MediaHandler");
    if (!klass) {
        ALOGE("%s: Unable to find java class com/qualcomm/qti/ims/internal/media/MediaHandler\n",
                 __func__);
        return JNI_ERR;
    }

    //Cache the MediaHandler class reference
    gClassMediaHandler = (jclass) e->NewGlobalRef(klass);

    property_get(IMPL_LIB_PROPERTY_NAME, libpath, DEFAULT_IMPL_LIB_PATH);
    vt_apis = vt_load_impl_lib(libpath);

    return e->RegisterNatives(klass, sMethods, METHODS_LEN);
}
