/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <jni.h>
#include <utils/Log.h>
#include <ims_camera_jni.h>

jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *e;

    ALOGD("%s\n", __func__);
    if (jvm->GetEnv((void **) &e, JNI_VERSION_1_6)) {
        return JNI_ERR;
    }

    if (register_videophone_ims_camera(e)) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
