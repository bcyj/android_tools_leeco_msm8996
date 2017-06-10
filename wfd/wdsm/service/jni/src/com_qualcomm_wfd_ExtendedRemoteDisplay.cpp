/*==============================================================================
*       com_qualcomm_wfd_ExtendedRemoteDisplay.cpp
*
*  DESCRIPTION:
*       Native inteface for ExtendedRemoteDisplay.
*
*  Copyright (c) 2013-2014 by Qualcomm Technologies, Incorporated.
*  All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
*/

#define LOG_TAG "RemoteDisplay"

#include "jni.h"
#include "JNIHelp.h"

#include "ExtendedRemoteDisplay.h"
#include <utils/Log.h>
#ifndef WFD_ICS
#include "common_log.h"
#endif

#include "MMDebugMsg.h"

#include "android_os_Parcel.h"
#include "android_util_Binder.h"
#include "com_qualcomm_wfd_ExtendedRemoteDisplay.h"

#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>

#include <binder/IServiceManager.h>
#include <gui/IGraphicBufferProducer.h>
#include <utils/Log.h>

using namespace android;

#define MSGERR(x) MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, x)
#define MSGHIGH(x) MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, x)
#define MSGMED(x) MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, x)

#define MSGERR1(x,y) MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, x, y)
#define MSGHIGH1(x,y) MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, x, y)
#define MSGMED1(x,y) MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, x, y)

#define MSGHIGH2(x,y,z) MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, x, y, z)

#define UNUSED(x) ((void)x)

static JavaVM *gJavaVM;
static jobject gEXTRDObject;
static jclass  gEXTRDClazz;


/** =======================================================================
**               JNI OnLoad and OnUnload
** ======================================================================= */


JNIEXPORT void JNI_OnUnload
(
    JavaVM *vm,
    void *reserved
)
{
    UNUSED(vm);
    UNUSED(reserved);
    MSGERR("EXTRD JNI On Unload");
    gJavaVM = 0;
}



JNIEXPORT jlong JNICALL Java_com_qualcomm_wfd_ExtendedRemoteDisplay_getNativeObject
(
    JNIEnv* env,
    jclass cls
)
{
    UNUSED(env);
    UNUSED(cls);
    MSGERR("QCRC JNI get native object");
    ExtendedRemoteDisplay* pRD = new ExtendedRemoteDisplay();
    if (!pRD)
    {
        MSGERR("EXTRD JNI Unable to create ExtendedRemoteDisplay");
    }

    return reinterpret_cast<jlong>(pRD);
}
JNIEXPORT jobject JNICALL Java_com_qualcomm_wfd_ExtendedRemoteDisplay_getSurface
(
    JNIEnv* env,
    jclass cls,
    jlong obj,
    jint width,
    jint height
)
{
    UNUSED(cls);
    MSGHIGH2("EXTRD JNI getSurface height = %d, width = %d", height, width);

    if(obj != 0)
    {
        ExtendedRemoteDisplay *pRD = reinterpret_cast<ExtendedRemoteDisplay*>(obj);

        if(pRD)
        {
            sp<IGraphicBufferProducer> pSurface = pRD->createSurface(width, height);

            jobject surfaceObj =
                android_view_Surface_createFromIGraphicBufferProducer(env,
                                                   pSurface);

            if (surfaceObj == NULL)
            {
                MSGERR("getSurface Failed to get Surface from texture");
                return NULL;
            }

            return surfaceObj;
        }
        else
        {
            MSGERR("getSurface Invalid Object Ptr");
            return NULL;
        }

    }
    else
    {
        MSGERR("getSurface NULL Object Ptr");
        return NULL;
    }
}

JNIEXPORT jint JNICALL Java_com_qualcomm_wfd_ExtendedRemoteDisplay_destroySurface
(
    JNIEnv* enc,
    jclass cls,
    jlong obj,
    jobject surface
)
{
    UNUSED(enc);
    UNUSED(cls);
    UNUSED(surface);
    MSGHIGH("EXTRD JNI destroySurface");
    if(obj != 0)
    {
        ExtendedRemoteDisplay *pRD = reinterpret_cast<ExtendedRemoteDisplay*>(obj);

        if(pRD)
        {
            //sp<ISurfaceTexture> surface = reinterpret_cast<sp<ISurfaceTexture>>();
        }
        else
        {
            MSGERR("destroySurface Invalid Object Ptr");
        }

    }
    else
    {
        MSGERR("destroySurface NULL Object Ptr");
    }
    return 0;
}

JNIEXPORT void JNICALL Java_com_qualcomm_wfd_ExtendedRemoteDisplay_destroyNativeObject
(
    JNIEnv* enc,
    jclass cls,
    jlong obj
)
{
    UNUSED(enc);
    UNUSED(cls);
    MSGHIGH("EXTRD JNI destroy native Object");

    if(obj != 0)
    {
        ExtendedRemoteDisplay *pRD = reinterpret_cast<ExtendedRemoteDisplay*>(obj);

        if(pRD)
        {
            delete(pRD);
        }
    }
    else
    {
        MSGERR("destroyNativeObj NULL Object Ptr");
    }
}

//------------------------------------------------------------------------------
//  JNI Methods
//------------------------------------------------------------------------------


static JNINativeMethod gMethods[] = {
 // nameOfNativeMethod, methodSignature, methodPointer
    {"getNativeObject",     "()J",                          (void *)Java_com_qualcomm_wfd_ExtendedRemoteDisplay_getNativeObject},
    {"getSurface",          "(JII)Landroid/view/Surface;",  (void *)Java_com_qualcomm_wfd_ExtendedRemoteDisplay_getSurface},
    {"destroySurface",      "(JLandroid/view/Surface;)I",   (void *)Java_com_qualcomm_wfd_ExtendedRemoteDisplay_destroySurface},
    {"destroyNativeObject", "(J)V",                         (void *)Java_com_qualcomm_wfd_ExtendedRemoteDisplay_destroyNativeObject }
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    UNUSED(reserved);
    JNIEnv *env;

    //store java virtual machine in global variable
    gJavaVM = vm;

    MSGHIGH("JNI_OnLoad called");

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        MSGERR("Failed to get the environment using GetEnv()");
        return JNI_FALSE;
    }



    gEXTRDClazz = env->FindClass("com/qualcomm/wfd/ExtendedRemoteDisplay");
    if(gEXTRDClazz == NULL)
    {
        MSGERR("EXTRD Native cant find class");
        return JNI_FALSE;
    }
 //   if (env->RegisterNatives(gEXTRDClazz, gMethods, 4) < 0) {
 //       MSGERR("EXTRD Native cant register natives");
 //       return JNI_FALSE;
 //   }


    return JNI_VERSION_1_4;
}
