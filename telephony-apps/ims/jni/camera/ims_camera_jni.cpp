/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "ims_camera_jni"
#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0

#include "ims_camera_jni.h"

#include <cutils/log.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include <stdlib.h>


#define IMS_API_ACCESS_ERROR   -10
#define DEFAULT_IMPL_LIB_PATH   "lib-imscamera.so"
#define IMPL_LIB_PROPERTY_NAME  "imscamera.impl.lib"

static ImsCameraApis *ims_cam_apis = NULL;

static jint dpl_cameraOpen(JNIEnv *e, jobject o, jint cameraId) {
    ALOGD("%s", __func__);
    if (ims_cam_apis && ims_cam_apis->cameraOpen && cameraId >= 0) {
        return ims_cam_apis->cameraOpen(cameraId);
    }
    return IMS_API_ACCESS_ERROR;
}

static jint dpl_cameraOpenWithPackage(JNIEnv *e, jobject o, jint cameraId, jstring packageName) {
    ALOGD("%s", __func__);

    jint status = IMS_API_ACCESS_ERROR;
    if (e && ims_cam_apis && ims_cam_apis->cameraOpenWithPackage && cameraId >= 0) {

        const char16_t* buffer = e->GetStringChars(packageName, NULL);
        if (buffer != NULL) {
            const jsize bufferLength = e->GetStringLength(packageName);
            android::String16 pn(buffer, bufferLength);
            e->ReleaseStringChars(packageName, buffer);

            status = ims_cam_apis->cameraOpenWithPackage(cameraId, &pn);
        } else {
            ALOGD("%s: GetStringUTFChars failed", __func__);
        }
    }
    return status;
}

static short dpl_cameraRelease(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->cameraRelease) {
        return ims_cam_apis->cameraRelease();
    }
    return IMS_API_ACCESS_ERROR;
}

static short dpl_cameraStartPreview(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->startCameraPreview) {
        return ims_cam_apis->startCameraPreview();
    }
    return IMS_API_ACCESS_ERROR;
}

static short dpl_cameraStopPreview(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->stopCameraPreview) {
        return ims_cam_apis->stopCameraPreview();
    }
    return IMS_API_ACCESS_ERROR;
}

static short dpl_cameraStartRecording(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->startCameraRecording) {
        return ims_cam_apis->startCameraRecording();
    }
    return IMS_API_ACCESS_ERROR;
}

static short dpl_cameraStopRecording(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->stopCameraRecording) {
        return ims_cam_apis->stopCameraRecording();
    }
    return IMS_API_ACCESS_ERROR;
}

static int dpl_setPreviewTexture(JNIEnv *e, jobject o, jobject osurface) {
    ALOGD("%s", __func__);
    if (ims_cam_apis && ims_cam_apis->setPreviewTexture) {
        return ims_cam_apis->setPreviewTexture(e, osurface);
    }
    return IMS_API_ACCESS_ERROR;
}

static short dpl_cameraSetDisplayOrientation(JNIEnv *e, jobject o, jint rotation) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->setDisplayOrientation && rotation >= 0) {
        return ims_cam_apis->setDisplayOrientation(rotation);
    }
    return IMS_API_ACCESS_ERROR;
}

static bool dpl_cameraIsZoomSupported(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->isZoomSupported) {
        return ims_cam_apis->isZoomSupported();
    }
    return IMS_API_ACCESS_ERROR;
}

static int dpl_cameraGetMaxZoom(JNIEnv *e, jobject o) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->getMaxZoom) {
        return ims_cam_apis->getMaxZoom();
    }
    return IMS_API_ACCESS_ERROR;
}


static void dpl_cameraSetZoom(JNIEnv *e, jobject o, jint zoomValue) {
    ALOGD("%s", __func__);
    if (ims_cam_apis && ims_cam_apis->setZoom && zoomValue >= 0) {
        ims_cam_apis->setZoom(zoomValue);
    }
}

static short dpl_cameraSetPreviewSize(JNIEnv *e, jobject o, jint width, jint height) {
    ALOGD("%s", __func__);
    if (ims_cam_apis && ims_cam_apis->setCameraParameter) {
        Resolution res;
        res.width = width;
        res.height = height;
        CameraParams cp;
        cp.cameraResolution = res;
        CameraParamContainer cpc;
        cpc.type = SET_RESOLUTION;
        cpc.params = cp;
        return ims_cam_apis->setCameraParameter(cpc);
    }
    return IMS_API_ACCESS_ERROR;
}

static short dpl_cameraSetFpsRange(JNIEnv *e, jobject o, jint fps) {
    ALOGD("%s", __func__);

    if (ims_cam_apis && ims_cam_apis->setCameraParameter) {
        CameraParams cp;
        cp.fps = fps;
        CameraParamContainer cpc;
        cpc.type = SET_FPS;
        cpc.params = cp;
        return ims_cam_apis->setCameraParameter(cpc);
    }
    return IMS_API_ACCESS_ERROR;
}

JNINativeMethod sMethods[] =
{
    {"native_open", "(I)S", (void *)dpl_cameraOpen},
    {"native_open", "(ILjava/lang/String;)S", (void *)dpl_cameraOpenWithPackage},
    {"native_release", "()S", (void *)dpl_cameraRelease},
    {"native_startPreview", "()S", (void *)dpl_cameraStartPreview},
    {"native_stopPreview", "()S", (void *)dpl_cameraStopPreview},
    {"native_startRecording", "()S", (void *)dpl_cameraStartRecording},
    {"native_stopRecording", "()S", (void *)dpl_cameraStopRecording},
    {"native_setPreviewTexture", "(Landroid/view/Surface;)S",
                                 (void *)dpl_setPreviewTexture},
    {"native_setDisplayOrientation", "(I)S", (void *)dpl_cameraSetDisplayOrientation},
    {"native_isZoomSupported", "()Z", (void *)dpl_cameraIsZoomSupported},
    {"native_getMaxZoom", "()I", (void *)dpl_cameraGetMaxZoom},
    {"native_setZoom", "(I)V", (void *)dpl_cameraSetZoom},
    {"native_setPreviewSize", "(II)S", (void *)dpl_cameraSetPreviewSize},
    {"native_setPreviewFpsRange", "(S)S", (void *)dpl_cameraSetFpsRange}
};

#define IMPL_CAM_SYM_OPEN    "cameraOpen"
#define IMPL_CAM_SYM_OPEN_WITH_PACKAGE   "cameraOpen2"
#define IMPL_CAM_SYM_RELEASE "cameraRelease"
#define IMPL_CAM_SYM_START_PREVIEW "startCameraPreview"
#define IMPL_CAM_SYM_STOP_PREVIEW "stopCameraPreview"
#define IMPL_CAM_SYM_START_RECORDING "startCameraRecording"
#define IMPL_CAM_SYM_STOP_RECORDING "stopCameraRecording"
#define IMPL_CAM_SYM_SET_CAMERA_PARAMETER "setCameraParameter"
#define IMPL_CAM_SYM_GET_CAMERA_PARAMETER "getCameraParameter"
#define IMPL_CAM_SYM_SET_NEAR_END_SURFACE "setNearEndSurface"
#define IMPL_CAM_SYM_SET_PREVIEW_DISPLAY_ORIENTATION "setPreviewDisplayOrientation"
#define IMPL_CAM_SYM_GET_MAX_ZOOM "getMaxZoom"
#define IMPL_CAM_SYM_IS_ZOOM_SUPPORTED "isZoomSupported"
#define IMPL_CAM_SYM_SET_ZOOM "setZoom"

static struct ImsCameraApis *ims_camera_load_lib(const char *path)
{
    struct ImsCameraApis *ret = NULL;
    void *handle;

    if (!(handle = dlopen(path, RTLD_NOW))) {
        ALOGE("Error loading library %s: %s\n", path, dlerror());
        goto finish;
    }

    ret = (ImsCameraApis *)calloc(sizeof(struct ImsCameraApis), 1);
    if (!ret) goto close_and_finish;

    ret->cameraOpen = (ImsCameraOpenFunc) dlsym(handle, IMPL_CAM_SYM_OPEN);
    ret->cameraOpenWithPackage = (ImsCameraOpenFuncWithPackage) dlsym(handle, IMPL_CAM_SYM_OPEN_WITH_PACKAGE);
    ret->cameraRelease = (ImsCamImplInt16VoidFunc) dlsym(handle, IMPL_CAM_SYM_RELEASE);
    ret->startCameraPreview = (ImsCamImplInt16VoidFunc) dlsym(handle, IMPL_CAM_SYM_START_PREVIEW);
    ret->stopCameraPreview = (ImsCamImplInt16VoidFunc) dlsym(handle, IMPL_CAM_SYM_STOP_PREVIEW);
    ret->startCameraRecording = (ImsCamImplInt16VoidFunc) dlsym(handle, IMPL_CAM_SYM_START_RECORDING);
    ret->stopCameraRecording = (ImsCamImplInt16VoidFunc) dlsym(handle, IMPL_CAM_SYM_STOP_RECORDING);
    ret->setCameraParameter = (ImsCameraSetParameter) dlsym(handle, IMPL_CAM_SYM_SET_CAMERA_PARAMETER);
    ret->getCameraParameter = (ImsCameraGetParameter) dlsym(handle, IMPL_CAM_SYM_GET_CAMERA_PARAMETER);
    ret->setPreviewTexture = (ImsCameraSetPreviewSurface) dlsym(handle, IMPL_CAM_SYM_SET_NEAR_END_SURFACE);
    ret->setDisplayOrientation = (ImsCameraSetDisplayOrientation) dlsym(handle, IMPL_CAM_SYM_SET_PREVIEW_DISPLAY_ORIENTATION);
    ret->getMaxZoom = (ImsCamImplInt16VoidFunc) dlsym(handle, IMPL_CAM_SYM_GET_MAX_ZOOM);
    ret->isZoomSupported = (ImsCameraIsZoomSupported) dlsym(handle, IMPL_CAM_SYM_IS_ZOOM_SUPPORTED);
    ret->setZoom = (ImsCameraSetZoom) dlsym(handle, IMPL_CAM_SYM_SET_ZOOM);
    return ret;
close_and_finish:
    if (dlclose(handle)) {
        ALOGE("Error closing library %s: %s\n", path, dlerror());
    }
finish:
    return ret;
}

int register_ims_camera(JNIEnv *e) {
    char libpath[PROPERTY_VALUE_MAX];

    ALOGD("%s\n", __func__);

    jclass klass;

    klass = e->FindClass("com/qualcomm/ims/vt/ImsCamera");
    if (!klass) {
        ALOGE("%s: Unable to find java class com/qualcomm/ims/vt/ImsCamera\n",
                 __func__);
        return JNI_ERR;
    }

    property_get(IMPL_LIB_PROPERTY_NAME, libpath, DEFAULT_IMPL_LIB_PATH);
    ims_cam_apis = ims_camera_load_lib(libpath);

    const jint sMethodsSize = sizeof(sMethods) / sizeof(sMethods[0]);
    return e->RegisterNatives(klass, sMethods, sMethodsSize);
}
