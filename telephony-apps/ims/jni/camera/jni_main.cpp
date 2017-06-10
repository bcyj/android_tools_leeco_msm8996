/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <jni.h>
#include <utils/Log.h>

extern int register_ims_camera(JNIEnv *e);

jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *e;

    ALOGD("%s\n", __func__);
    if (jvm->GetEnv((void **) &e, JNI_VERSION_1_6)) {
        return JNI_ERR;
    }

    if (register_ims_camera(e)) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}



