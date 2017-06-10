/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef VT_JNI_INTERFACE_H
#define VT_JNI_INTERFACE_H

#include <jni.h>
#include <cutils/log.h>
#include <gui/IGraphicBufferProducer.h>
#include <utils/StrongPointer.h>
#include <utils/RefBase.h>

jclass gClassImsMedia;

typedef void (*IMS_EVENT_NOTIFY_CALLBACK2)(int32_t, int32_t);
typedef void (*ImsMediaRegisterCbFuncPtr2)(IMS_EVENT_NOTIFY_CALLBACK2);
typedef int16_t (*ImsMediaInitFuncPtr)(void);
typedef int16_t (*ImsMediaFrameFuncPtr)(uint16_t *, uint32_t);
typedef int16_t (*ImsMediaSetSurfFuncPtr)(JNIEnv *, jobject);
typedef void (*ImsMediaSetDeviceOrientFuncPtr)(uint32_t);
typedef int16_t (*ImsMediaDeinitFuncPtr)(void);
typedef uint32_t (*ImsMediaUint32VoidFuncPtr)(void);
typedef int32_t (*ImsMediaInt32Int32FuncPtr)(int32_t);
typedef uint64_t (*ImsMediaDataUsageFuncPtr)(uint16_t, int32_t);
typedef android::sp<android::IGraphicBufferProducer> (*ImsMediaGbpVoidFuncPtr)(void);

struct ImsMediaApis {
    ImsMediaRegisterCbFuncPtr2 registerAppEventCallback2;
    ImsMediaInitFuncPtr initImsThinClient;
    ImsMediaFrameFuncPtr frameToEncode;
    ImsMediaSetSurfFuncPtr setFarEndSurface;
    ImsMediaSetDeviceOrientFuncPtr setDeviceOrientation;
    ImsMediaDeinitFuncPtr deInitImsThinClient;
    ImsMediaUint32VoidFuncPtr getNegotiatedFPS;
    ImsMediaUint32VoidFuncPtr getNegotiatedHeight;
    ImsMediaUint32VoidFuncPtr getNegotiatedWidth;
    ImsMediaUint32VoidFuncPtr getUIOrientationMode;
    ImsMediaUint32VoidFuncPtr getPeerHeight;
    ImsMediaUint32VoidFuncPtr getPeerWidth;
    ImsMediaUint32VoidFuncPtr getVideoQualityIndication;
    ImsMediaDataUsageFuncPtr getRtpDataUsage2;
    ImsMediaInt32Int32FuncPtr requestRtpDataUsage2;
    ImsMediaGbpVoidFuncPtr getRecordingSurface;
};

#endif
