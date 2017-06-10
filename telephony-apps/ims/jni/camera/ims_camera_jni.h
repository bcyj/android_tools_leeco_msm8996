/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef IMS_CAMERA_JNI_H
#define IMS_CAMERA_JNI_H

#include <jni.h>
#include <cutils/log.h>
#include <utils/String16.h>

typedef int16_t (*ImsCameraOpenFunc)(uint32_t);
typedef int16_t (*ImsCameraOpenFuncWithPackage)(uint32_t, android::String16*);
typedef int16_t (*ImsCamImplInt16VoidFunc)();
typedef int16_t (*ImsCameraSetPreviewSurface)(JNIEnv *, jobject);
typedef int16_t (*ImsCameraSetDisplayOrientation)(uint32_t);

typedef bool (*ImsCameraIsZoomSupported)();
typedef void (*ImsCameraSetZoom)(uint32_t);

typedef struct {
        int width;
        int height;
} Resolution;

typedef enum {
        INVALID_PARAM = 0,
        SET_FPS,
        SET_RESOLUTION
} eParamType;

typedef union {
        int fps;
        Resolution cameraResolution;
} CameraParams;

typedef struct {
        eParamType type;
        CameraParams params;
} CameraParamContainer;


typedef int16_t (*ImsCameraSetParameter)(CameraParamContainer);
typedef CameraParams (*ImsCameraGetParameter)(jobject);

struct ImsCameraApis {
        ImsCameraOpenFunc cameraOpen;
        ImsCameraOpenFuncWithPackage cameraOpenWithPackage;
        ImsCamImplInt16VoidFunc cameraRelease;
        ImsCamImplInt16VoidFunc startCameraPreview;
        ImsCamImplInt16VoidFunc stopCameraPreview;
        ImsCamImplInt16VoidFunc startCameraRecording;
        ImsCamImplInt16VoidFunc stopCameraRecording;
        ImsCameraSetPreviewSurface setPreviewTexture;
        ImsCameraSetDisplayOrientation setDisplayOrientation;
        ImsCameraSetParameter setCameraParameter;
        ImsCameraGetParameter getCameraParameter;
        ImsCameraIsZoomSupported isZoomSupported;
        ImsCamImplInt16VoidFunc getMaxZoom;
        ImsCameraSetZoom setZoom;
};

#endif // IMS_CAMERA_JNI_H
