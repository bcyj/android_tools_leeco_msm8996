/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef VT_JNI_INTERFACE_H
#define VT_JNI_INTERFACE_H

#include <jni.h>
#include <cutils/log.h>

jclass gClassMediaHandler;

typedef void (*IMS_EVENT_NOTIFY_CALLBACK)(uint16_t);
typedef void (*VtImplRegisterCbFun)(IMS_EVENT_NOTIFY_CALLBACK);
typedef int16_t (*VtImplInitFun)(void);
typedef int16_t (*VtImplFrameFun)(uint16_t *, uint32_t);
typedef int16_t (*VtImplSetSurfFun)(JNIEnv *, jobject);
typedef void (*VtImplSetDeviceOrient)(uint32_t);
typedef int16_t (*VtImplDeinitFun)(void);
typedef uint32_t (*VtImplUint32VoidFunc)(void);

struct VtImplApis {
    VtImplRegisterCbFun registerAppEventCallback;
    VtImplInitFun initImsThinClient;
    VtImplFrameFun frameToEncode;
    VtImplSetSurfFun setFarEndSurface;
    VtImplSetDeviceOrient setDeviceOrientation;
    VtImplDeinitFun deInitImsThinClient;
    VtImplUint32VoidFunc getNegotiatedFPS;
    VtImplUint32VoidFunc getNegotiatedHeight;
    VtImplUint32VoidFunc getNegotiatedWidth;
    VtImplUint32VoidFunc getUIOrientationMode;
};

#endif
