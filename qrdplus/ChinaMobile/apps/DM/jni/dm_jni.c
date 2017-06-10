#include <stdlib.h>
#include <string.h>
#include <utils/Log.h>
#include "syncmlcomm.h"
#include "jni.h"
#include "dm_task.h"

static JavaVM* mVM;

#ifndef DEBUG
#define DEBUG   1
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define SML_ERR_OK 0x00
#define RDM_SOCKET_DM_RECEV_BUFFER_LEN (15*1024)

typedef enum {
    invalid_type = 0,
    void_type,
    boolean_type,
    object_type,
    byte_type,
    char_type,
    short_type,
    int_type,
    long_type,
    float_type,
    double_type,
    array_type
} JNIType;

typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jobject mDmJniInterfaceObj;
//jobject mDmAlertDialogObj;
jobject mDmTransactionObj;

int mWorkSpaceId;

int terminalFlag = 0;

extern task_relay_info_type* dm_task_relay_info;
extern char* s_recvBuffer_ptr;
extern uint32 s_recvActualLen;
extern syncml_Comm_type* dm_global_Comm_Ptr;

extern short RDM_parse_syncml(char* infoStr, char** username, char** mac,
        char** algorithm);

JNIEnv* myGetEnv() {
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    JNIEnv* env = NULL;
    if ((*mVM)->GetEnv(mVM, &uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        return NULL ;
    }
    return (uenv.env);
}
jbyteArray createByteArray(char *buffer) {
    LOGE("createByteArray");
    JNIEnv* env = myGetEnv();
    jbyteArray byteArray;
    int size;
    if ( (buffer == NULL ) || (env == NULL)) {
        return NULL ;
    }
    size = strlen(buffer);
    byteArray = (*env)->NewByteArray(env, size);

    (*env)->SetByteArrayRegion(env, byteArray, 0, size, buffer);
    return byteArray;
}

void *callJavaMethod(jobject obj, const char * method_name, JNIType returnType,
        const char * sig, ...) {
    va_list ap;
    void *result = NULL;
    va_start(ap, sig);
    JNIEnv* env = myGetEnv();
    jclass mclas;
    jmethodID mid;

    if(env != NULL)
    {
        mclas = (*env)->GetObjectClass(env, obj);
        mid = (*env)->GetMethodID(env, mclas, method_name, sig);

        switch (returnType) {
        case void_type:
            (*env)->CallVoidMethodV(env, obj, mid, ap);
            break;
        case boolean_type:
            result = (void *) ((*env)->CallBooleanMethodV(env, obj, mid, ap));
            break;
        case object_type:
            result = (void *) ((*env)->CallObjectMethodV(env, obj, mid, ap));
            break;
        case byte_type:
            result = (void *) ((*env)->CallByteMethodV(env, obj, mid, ap));
            break;
        case char_type:
            result = (void *) ((*env)->CallCharMethodV(env, obj, mid, ap));
            break;
        case short_type:
            result = (void *) ((*env)->CallShortMethodV(env, obj, mid, ap));
            break;
        case int_type:
            result = (void *) ((*env)->CallIntMethodV(env, obj, mid, ap));
            break;
        case long_type:
            result = (void *) ((*env)->CallLongMethodV(env, obj, mid, ap));
            break;
        default:
            if (DEBUG)
                LOGE("unknow type: %d", returnType);
            break;

        }
        (*env)->DeleteLocalRef(env, mclas);
    }
    else
        LOGE("myGetEnv() returned NULL");

    va_end(ap);

    return result;
}

char* MMIDM_GetCBFunc(int handler, int offset) {
    jbyteArray byteArray;
    jbyte *body;
    jsize len;

    byteArray = callJavaMethod(mDmJniInterfaceObj, "read_CBFunc", object_type,
            "(II)[B", handler, offset);
    if (NULL == byteArray) {
        LOGE("MMIDM_GetCBFunc byteArray = null  handler='%d'", handler);
        return false;
    }
    myGetByteArrayElements(byteArray, &body, &len);
    if (len > 0) {
        return body;
    } else {
        return NULL ;
    }

}

char* JMMIDM_GetServerNonce() {
    jbyteArray byteArray;
    jbyte *body;
    jsize len;

    byteArray = callJavaMethod(mDmJniInterfaceObj, "getServerNonce",
            object_type, "()[B");
    if (NULL == byteArray) {
        LOGE("JMMIDM_GetServerNonce byteArray = null  ");
        return false;
    }
    myGetByteArrayElements(byteArray, &body, &len);
    if (len > 0) {
        LOGE("JMMIDM_GetServerNonce data = %s", body);
        return body;
    } else {
        return NULL ;
    }

}
char* JMMIDM_GetClientNonce() {
    jbyteArray byteArray;
    jbyte *body;
    jsize len;

    byteArray = callJavaMethod(mDmJniInterfaceObj, "getClientNonce",
            object_type, "()[B");
    if (NULL == byteArray) {
        LOGE("JMMIDM_GetClientNonce byteArray = null");
        return false;
    }
    myGetByteArrayElements(byteArray, &body, &len);
    if (len > 0) {
        LOGE("JMMIDM_GetClientNonce data = %s", body);
        return body;
    } else {
        return NULL ;
    }

}
void JMMIDM_SetServerNonce(char* data) {
    LOGE("JMMIDM_SetServerNonce data = %s", data);
    jbyteArray tmpArray = createByteArray(data);
    callJavaMethod(mDmJniInterfaceObj, "setServerNonce", void_type, "([B)V",
            tmpArray);
    JNIEnv* env = myGetEnv();
    if(env != NULL)
       (*env)->DeleteLocalRef(env, tmpArray);
    else
       LOGE("JMMIDM_SetServerNonce myGetEnv() returned NULL");
}
void JMMIDM_SetClientNonce(char* data) {
    LOGE("JMMIDM_SetClientNonce data = %s", data);
    jbyteArray tmpArray = createByteArray(data);
    callJavaMethod(mDmJniInterfaceObj, "setClientNonce", void_type, "([B)V",
            tmpArray);
    JNIEnv* env = myGetEnv();
    if(env != NULL)
        (*env)->DeleteLocalRef(env, tmpArray);
    else
        LOGE("JMMIDM_SetClientNonce myGetEnv() returned NULL");
}

void MMIDM_ReplaceFunc(int handler, int offset, char* data, int maxSize) {
    if (maxSize > 0) {
        jbyteArray tmpArray = createByteArray(data);
        callJavaMethod(mDmJniInterfaceObj, "write_CBFunc", void_type,
                "(II[BI)V", handler, offset, tmpArray, maxSize);
        JNIEnv* env = myGetEnv();
        if(env != NULL)
            (*env)->DeleteLocalRef(env, tmpArray);
        else
            LOGE("MMIDM_ReplaceFunc myGetEnv() returned NULL");
    } else {
        callJavaMethod(mDmJniInterfaceObj, "write_null_CBFunc", void_type,
                "(I)V", handler);
    }
}

//boolean pimTask_GetVcardItem(int entry_id, jbyte *buffer, int bufferSize)

int JstartPppConnect() {
    int ret = callJavaMethod(mDmJniInterfaceObj, "startPppConnect", int_type,
            "()I");
    LOGE("JstartPppConnect RET: %d", ret);
    return ret;
}
void JstopPppConnect() {
    callJavaMethod(mDmJniInterfaceObj, "stopPppConnect", void_type, "()V");

}

int JgetPppConnectStatus() {
    return callJavaMethod(mDmJniInterfaceObj, "getPppConnectStatus", int_type,
            "()I");
}

//public void displayDialog(int id, String message, int timeout)
void dmTaskComm_displayDialog(int id, char* message, long timeout) {
    LOGE("dmTaskComm_displayDialog id: %d, message: %s", id, message);
    jbyteArray tmpArray = createByteArray(message);
    callJavaMethod(mDmJniInterfaceObj, "displayDialog", void_type, "(I[BI)V",
            id, tmpArray, timeout);
    LOGE("dmTaskComm_displayDialog callJavaMethod end");
    JNIEnv* env = myGetEnv();
    if(env != NULL)
        (*env)->DeleteLocalRef(env, tmpArray);
    else
         LOGE("dmTaskComm_displayDialog myGetEnv() returned NULL");
}

short pimTaskComm_SendData() {
    if (1 == terminalFlag) {
//         return -1;
    }
    jboolean ret = callJavaMethod(mDmTransactionObj, "sendSyncMLData",
            boolean_type, "()Z");
    if (!ret) {
        return -1;
    }
    return 0;
}

short pimTaskComm_RecData() {
    return 0;
}

static void setTerminalFlag() {
    terminalFlag = 1;
}

void dmTaskComm_SetWorkspaceId(int id) {
    mWorkSpaceId = id;
}
/*
 void dmTaskComm_CreateAlertDialog(int id,char *message,long timeout ) {
 callJavaMethod(mDmAlertDialogObj, "CreateAlertDialog", void_type, "(I[BJ)V", id,message,timeout);
 }*/

char* dmGetDmProfile() {
    return NULL ;
}

static short JcopySourceDataToBuffer(JNIEnv *env, jobject thiz, jshort id,
        jbyteArray source, jlong size) {
    long targetSize = 0;
    unsigned char *targetBuf = 0;
    int rc;
    jbyte *body;
    jsize len;
//    rc = dm_smlLockReadBuffer(id, &targetBuf, &targetSize);
    //   rc = dm_smlLockWriteBuffer(id, &targetBuf, &targetSize);

    myGetByteArrayElements(source, &body, &len);
    LOGE("JcopySourceDataToBuffer id: %d, len:%d", id, len);

    //  if(rc == SML_ERR_OK && (targetSize > size)) {
    //  memcpy(targetBuf, body, size);
    memset(s_recvBuffer_ptr, 0, RDM_SOCKET_DM_RECEV_BUFFER_LEN);
    memcpy(s_recvBuffer_ptr, body, len);
    s_recvActualLen = len;

    //   }
//    dm_smlUnlockWriteBuffer(id, size);

    myReleaseByteArrayElements(&source, &body);

    return rc;

}

static jbyteArray JcopyDataForSending(JNIEnv *env, jobject thiz, jshort id) {
    jbyteArray byteArray;
    long size = 0;
    jbyte *buffer = 0;
#if 0
    int rc = dm_smlLockReadBuffer(id, &buffer, &size);
    if(rc != SML_ERR_OK) {
        LOGE("JcopyDataForSending callJavaMethod  rc != SML_ERR_OK");
        dm_smlUnlockReadBuffer(id, size);
        return NULL;
    }

    byteArray = (*env)->NewByteArray(env, size);
    if(byteArray == NULL) {

        dm_smlUnlockReadBuffer(id, size);
        return NULL;
    }

    (*env)->SetByteArrayRegion(env, byteArray, 0, size, buffer);

    dm_smlUnlockReadBuffer(id, size);
#else

    byteArray = (*env)->NewByteArray(env, dm_global_Comm_Ptr->cache_length);
    (*env)->SetByteArrayRegion(env, byteArray, 0,
            dm_global_Comm_Ptr->cache_length, dm_global_Comm_Ptr->cache);
#endif
    return byteArray;
}

static void JstepDataReceive() {
    dm_task_relay_info->comm_observer.notifyTransport(NULL, 0);
}

static void JnotifyCommBroken() {
    dm_task_relay_info->comm_observer.notifyCommBroken(NULL, 0, FALSE);
}

/*
 static void JsaveDmAlertDialogObject(JNIEnv *env, jobject thiz, jobject j) {
 // TODO: remeber to free it using DeleteGlobalRef(mDmAlertDialogObj);
 mDmAlertDialogObj = (*env)->NewGlobalRef(env, j);
 }
 */

static void JsaveDmTransactionObject(JNIEnv *env, jobject thiz, jobject j) {
    // TODO: remeber to free it using DeleteGlobalRef(mDmAlertDialogObj);
    mDmTransactionObj = (*env)->NewGlobalRef(env, j);
}

static int JgetWorkSpaceId(JNIEnv *env, jobject thiz) {
    //return mWorkSpaceId;
    return dm_task_relay_info->workspaceid;
}

static void JsaveDmJniInterfaceObject(JNIEnv *env, jobject thiz, jobject j) {
    // TODO: remeber to free it using DeleteGlobalRef(mDmJniInterfaceObj);
    mDmJniInterfaceObj = (*env)->NewGlobalRef(env, j);
}

static void JsetValueIsAlreadyReceive(JNIEnv *env, jobject thiz, jboolean value) {
//    mPIM.is_alreadyReceive = value;
}

static jbyteArray JgetReplaceServerAddress(JNIEnv *env, jobject thiz) {
    jbyteArray byteArray;
    long size = 0;

    size = strlen(dm_task_relay_info->syncml_connect_addr);
    byteArray = (*env)->NewByteArray(env, size);
    (*env)->SetByteArrayRegion(env, byteArray, 0, size,
            (const char *) dm_task_relay_info->syncml_connect_addr);
    return byteArray;
}

static jboolean JMMIDM_IsDmRun(JNIEnv *env, jobject thiz) {
    return MMIDM_IsDmRun();
}
static jboolean JMMIDM_ExitDM(JNIEnv *env, jobject thiz) {
    return MMIDM_ExitDM();
}
//PUBLIC BOOLEAN MMIDM_StartDm(DM_SESSION_TYPE type,char* msg_body, uint32 msg_size)

static jint JMMIDM_StartVDM(JNIEnv *env, jobject thiz, jint type,
        jbyteArray msg_body, jlong msg_size) {
    jbyte *body;
    jsize len;
    jint result;
    myGetByteArrayElements(msg_body, &body, &len);
    result = (jint) MMIDM_StartDm(type, body, len);
    myReleaseByteArrayElements(&msg_body, &body);
    return result;
}

static void Jhs_dm_mmi_confirmationQuerycb(JNIEnv *env, jobject thiz,
        jboolean iscontinue) {
    return dm_mmi_confirmationQuerycb(iscontinue);
}

static jshort JVDM_notifyNIASessionProceed(JNIEnv *env, jobject thiz) {
    return VDM_notifyNIASessionProceed();
}

static void Jparse_x_syncml_hmac(JNIEnv *env, jobject thiz, jbyteArray headbody) {

    long targetSize = 0;
    unsigned char *targetBuf = 0;
    int rc;
    jbyte *body;
    jsize len;

    myGetByteArrayElements(headbody, &body, &len);
    //LOGE("JcopySourceDataToBuffer id: %d, len:%d", id,len );
    RDM_parse_syncml(body, (char**) &(dm_task_relay_info->comm_hmac.username),
            (char**) &(dm_task_relay_info->comm_hmac.mac),
            (char**) &(dm_task_relay_info->comm_hmac.algorithm));

    myReleaseByteArrayElements(&headbody, &body);
}
static const char *dmClassPathName = "com/android/dm/DMNativeMethod";
static const JNINativeMethod dm_methods[] = { { "JMMIDM_IsDmRun", "()Z",
        (void*) JMMIDM_IsDmRun }, { "JMMIDM_ExitDM", "()Z",
        (void*) JMMIDM_ExitDM }, { "JMMIDM_StartVDM", "(I[BI)I",
        (void*) JMMIDM_StartVDM },
        { "JsaveDmJniInterfaceObject", "(Lcom/android/dm/DmJniInterface;)V",
                (void*) JsaveDmJniInterfaceObject }, {
                "JsaveDmTransactionObject",
                "(Lcom/android/dm/transaction/DMTransaction;)V",
                (void*) JsaveDmTransactionObject }, { "JnotifyCommBroken",
                "()V", (void*) JnotifyCommBroken }, { "JstepDataReceive", "()V",
                (void*) JstepDataReceive }, { "JgetWorkSpaceId", "()I",
                (void*) JgetWorkSpaceId }, { "JcopySourceDataToBuffer",
                "(S[BJ)S", (void*) JcopySourceDataToBuffer }, {
                "JcopyDataForSending", "(S)[B", (void*) JcopyDataForSending }, {
                "JgetReplaceServerAddress", "()[B",
                (void*) JgetReplaceServerAddress }, {
                "Jhs_dm_mmi_confirmationQuerycb", "(Z)V",
                (void*) Jhs_dm_mmi_confirmationQuerycb }, {
                "JVDM_notifyNIASessionProceed", "()S",
                (void*) JVDM_notifyNIASessionProceed }, {
                "Jparse_x_syncml_hmac", "([B)V", (void*) Jparse_x_syncml_hmac },

};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* gMethods, int numMethods) {
    jclass clazz;

    LOGE("registerNativeMethods start");
    clazz = (*env)->FindClass(env, className);
    if (clazz == NULL ) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static int registerNatives(JNIEnv* env) {

    LOGE("registerNatives start");
    if (!registerNativeMethods(env, dmClassPathName, dm_methods,
            sizeof(dm_methods) / sizeof(dm_methods[0]))) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGE("JNI_OnLoad START");
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;
    mVM = vm;
    LOGE("JNI_OnLoad");

    if ((*vm)->GetEnv(vm, &uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;

    bail: return result;
}

void myGetByteArrayElements(jbyteArray byteArray, jbyte **body, jsize *len) {
    JNIEnv* env = myGetEnv();
    if(env != NULL)
    {
        *body = (*env)->GetByteArrayElements(env, byteArray, 0);
        *len = (*env)->GetArrayLength(env, byteArray);
    }
    else
        LOGE("myGetByteArrayElements : env is NULL");
}

void myReleaseByteArrayElements(jbyteArray *byteArray, jbyte **body) {
    JNIEnv* env = myGetEnv();
    if(env != NULL)
    {
        (*env)->ReleaseByteArrayElements(env, *byteArray, *body, 0);
    }
    else
        LOGE("myReleaseByteArrayElements : env is NULL");
}

