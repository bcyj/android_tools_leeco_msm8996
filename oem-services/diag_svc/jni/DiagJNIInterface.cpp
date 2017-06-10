/*****************************************************************************
  Copyright (C) 2011,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ******************************************************************************/

#define LOG_TAG "diagJNIInterface"

#include "msg.h"
#include "diag_lsm.h"
#include "diagpkt.h"
#include "diagcmd.h"

#include <jni.h>
#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

static JavaVM *jvm;
static jobject gDiagJNIInterfaceObject, gDiagCommandObject;
const char *kInterfacePath = "com/qti/diagservices/DiagJNIInterface";
const char *kDataPath = "com/qti/diagservices/DiagCommand";

#ifdef __cplusplus
extern "C" {
#endif

    pthread_mutex_t commandList_lock;
    sem_t diagCommand_ready;

    static void diag_callback_handler(const char *s) {
        int jniError;
        JNIEnv *env;
        bool threadAttached = false;

        if(jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
            if(jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
                LOGE("diag_callback_handler: AttachCurrentThread error");
                return;
            }
            threadAttached = true;
        }

        jclass cls = env->GetObjectClass(gDiagJNIInterfaceObject);
        if(!cls) {
            if(threadAttached) jvm->DetachCurrentThread();
            LOGE("diag_callback_handler: GetObjectClass error");
            return;
        }

        jmethodID callback = env->GetStaticMethodID(cls, "eventHandler","(Ljava/lang/String;)V");
        if(!callback) {
            if(threadAttached) jvm->DetachCurrentThread();
            LOGE("diag_callback_handler: GetStaticMethodID error");
            return;
        }

        /* Finally call the callback */
        env->CallStaticVoidMethod(cls, callback, env->NewStringUTF(s));
        if(threadAttached) jvm->DetachCurrentThread();
    }

    typedef struct
    {
        void *req_pkt;
        uint16 pkt_len;
    } DiagCommand;

    DiagCommand commandList;

    JNIEXPORT void JNICALL Java_com_qti_diagservices_DiagJNIInterface_getNextCommand
        (JNIEnv *env, jclass cls) {
            jclass dataClass = env->GetObjectClass(gDiagCommandObject);
            sem_wait(&diagCommand_ready);
            pthread_mutex_lock(&commandList_lock);
            pthread_mutex_unlock(&commandList_lock);
            return;
        }

    PACK(void *) android_diag_handler (PACK(void *)req_pkt, uint16 pkt_len)
    {
        LOGD("android_diag_handler called!");

        /* TODO: Decode diag req_pkt */
        void *rsp_pkt = diagpkt_alloc(DIAG_CONTROL_F, pkt_len);
        if(rsp_pkt) {
            memcpy(rsp_pkt, req_pkt, pkt_len);
        }
        diag_callback_handler("reboot");
        return rsp_pkt;
    }

    static const diagpkt_user_table_entry_type android_diag_handler_tbl[] =
    {
        {DIAG_CONTROL_F, DIAG_CONTROL_F, android_diag_handler},
    };

    jint JNI_OnLoad(JavaVM* vm, void* reserved)
    {
        JNIEnv *env;
        jvm = vm;
        if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
            LOGE("Failed to get the environment using GetEnv()");
            return -1;
        }

        jclass diagJNIInterface = env->FindClass(kInterfacePath);
        jmethodID diagJNIInit = env->GetMethodID(diagJNIInterface, "<init>", "()V");
        jobject diagJNIObject = env->NewObject(diagJNIInterface, diagJNIInit);
        gDiagJNIInterfaceObject = env->NewGlobalRef(diagJNIObject);

        jclass diagCommandCls = env->FindClass(kDataPath);
        jmethodID diagCommandInit = env->GetMethodID(diagCommandCls, "<init>", "()V");
        jobject diagCommandObject = env->NewObject(diagCommandCls, diagCommandInit);
        gDiagCommandObject = env->NewGlobalRef(diagCommandObject);

        JNINativeMethod methods[] = {
            {
                "getNextCommand",
                "()V",
                (void *) Java_com_qti_diagservices_DiagJNIInterface_getNextCommand
            },
        };

        if(android::AndroidRuntime::registerNativeMethods(
                    env, kInterfacePath, methods, NELEM(methods)) != JNI_OK) {
            LOGE("Failed to register native methods");
            return -1;
        }

        /* Register for diag packets */
        boolean bInit_Success = FALSE;
        bInit_Success = Diag_LSM_Init(NULL);

        if(!bInit_Success) {
            LOGE("Diag_LSM_Init call failed");
            return -1;
        }

        LOGI("Diag_LSM_Init call succeeded");

        DIAGPKT_DISPATCH_TABLE_REGISTER(DIAGPKT_NO_SUBSYS_ID,
                android_diag_handler_tbl);
        return JNI_VERSION_1_4;
    }
#ifdef __cplusplus
}
#endif
