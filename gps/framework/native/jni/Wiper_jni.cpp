/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  Copyright (c) 2012-2014 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/

#define LOG_TAG "Wiper_jni"
#define LOG_NDEBUG 0

#define LAT_SCALE_FACTOR 23860929.4222
#define LONG_SCALE_FACTOR 11930464.7111
#define LAT_SCALE_UP(x) ((double)(x * LAT_SCALE_FACTOR))
#define LONG_SCALE_UP(x) ((double)(x * LONG_SCALE_FACTOR))

#define WIPER_FEATURE_ENABLE_MASK 0x1
#define SUPL_WIFI_FEATURE_ENABLE_MASK 0x4

#ifndef IZAT_CONF_FILE
#define IZAT_CONF_FILE "/etc/izat.conf"
#endif

#include "JNIHelp.h"
#include "jni.h"
#include "loc_cfg.h"
#include "LBSAdapter.h"
#include "android_runtime/AndroidRuntime.h"

using namespace android;

static LBSAdapter* nLBSAdapter = NULL;
static jobject jWiper = NULL;

static jmethodID method_wifiRequestEvent;
static jmethodID method_wifiApDataRequestEvent;

static int listenerMode;
static int wifi_wait_timeout_select = 0;

static loc_param_s_type wiper_parameter_table[] =
{
  {"WIFI_WAIT_TIMEOUT_SELECT",&wifi_wait_timeout_select, NULL,'n'},
};

static void wifiRequestEventCb(int requestType) {
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    ALOGD ("wifiRequestEventCb invoked");
    if(jWiper && method_wifiRequestEvent){
        env->CallVoidMethod(jWiper, method_wifiRequestEvent, requestType);

        if (env->ExceptionCheck()) {
            ALOGE("Exception in %s", __FUNCTION__);
            env->ExceptionClear();
        }
    }
    else {
        ALOGE("Error in %s:jWiper= %p,method_wifiRequestEvent=%p",
         __FUNCTION__,jWiper,method_wifiRequestEvent);
    }
}

static void wifiApDataRequestEventCb() {
    JNIEnv* env = AndroidRuntime::getJNIEnv();
    ALOGD ("wifiDataRequestEventCb invoked");
    if(jWiper && method_wifiApDataRequestEvent){
        env->CallVoidMethod(jWiper, method_wifiApDataRequestEvent);

        if (env->ExceptionCheck()) {
            ALOGE("Exception in %s", __FUNCTION__);
            env->ExceptionClear();
        }
    }
    else {
        ALOGE("Error in %s:jWiper= %p,method_wifiApDataRequestEvent=%p",
         __FUNCTION__,jWiper,method_wifiApDataRequestEvent);
    }
}

void WiperSsrInform::proc() const {
    if (nLBSAdapter){
        ALOGD("calling  nLBSAdapter->handleSSR()\n");
        if(listenerMode & WIPER_FEATURE_ENABLE_MASK == WIPER_FEATURE_ENABLE_MASK)
            nLBSAdapter->wifiStatusInform();
    }
}

void WiperRequest::proc() const {
    wifiRequestEventCb(mType);
}

void WiperApDataRequest::proc() const {
    wifiApDataRequestEventCb();
}

static void classInit(JNIEnv* env, jclass clazz) {
    method_wifiRequestEvent = env->GetMethodID(clazz, "wifiRequestEvent", "(I)V");
    method_wifiApDataRequestEvent = env->GetMethodID(clazz, "wifiApDataRequestEvent", "()V");
}

static void instanceInit(JNIEnv *env, jobject obj, jint listener_mode) {
    if (NULL == jWiper) {
        jWiper = env->NewGlobalRef(obj);
    }
    if (NULL == nLBSAdapter) {
        bool isSuplWifiEnabled = false;
        listenerMode = listener_mode;
        LOC_API_ADAPTER_EVENT_MASK_T mask = LOC_API_ADAPTER_BIT_REQUEST_WIFI;

        /*Check if SUPL WIFI is enabled as well as whether the timeout value is greater than 0*/
        if(listenerMode & SUPL_WIFI_FEATURE_ENABLE_MASK == SUPL_WIFI_FEATURE_ENABLE_MASK) {
            UTIL_READ_CONF(IZAT_CONF_FILE, wiper_parameter_table);
            if(wifi_wait_timeout_select > 0) {
                isSuplWifiEnabled = true;
                ALOGD("Subscribe for supl wifi mask.\n");
                mask = mask | LOC_API_ADAPTER_BIT_REQUEST_WIFI_AP_DATA;
            }
        }

        nLBSAdapter = LBSAdapter::get(mask,
                                      (loc_core::MsgTask::tCreate)
                                      AndroidRuntime::createJavaThread,
                                      NULL);

        if(listenerMode & WIPER_FEATURE_ENABLE_MASK == WIPER_FEATURE_ENABLE_MASK)
        {
            ALOGD("Inform wiper status.\n");
            nLBSAdapter->wifiStatusInform();
        }

        if(isSuplWifiEnabled)
        {
            ALOGD("Inform wifi timout value for supl.\n");
            nLBSAdapter->setWifiWaitTimeoutValue(wifi_wait_timeout_select);
        }

    }
}

static void sendNetworkLocation(JNIEnv* env, jobject obj, jint position_valid,
                                jdouble latitude, jdouble longitude, jfloat accuracy,
                                jint hor_confidence, jint apinfo_valid,
                                jbyteArray mac_array, jintArray rssi_array,
                                jintArray channel_array,
                                jint num_aps_used, jint ap_len,
                                jobjectArray ssid_array)
{
    ALOGD("Send Network Location.\n");

    jint * rssi_arr = 0;
    jint * channel_arr = 0;
    jbyte * mac_arr = 0;

    WifiLocation wifiInfo;
    memset(&wifiInfo, 0, sizeof(wifiInfo));

    rssi_arr = env->GetIntArrayElements(rssi_array, 0);
    channel_arr = env->GetIntArrayElements(channel_array, 0);
    mac_arr = env->GetByteArrayElements(mac_array, 0);

    int ssid_len = env->GetArrayLength(ssid_array);
    LOC_LOGV("ssid_len is %d", ssid_len);

    wifiInfo.positionValid = position_valid;
    wifiInfo.latitude = LAT_SCALE_UP(latitude);
    wifiInfo.longitude = LONG_SCALE_UP(longitude);
    wifiInfo.accuracy = accuracy;
    wifiInfo.horConfidence = hor_confidence;
    wifiInfo.numApsUsed = num_aps_used;
    wifiInfo.fixError = 0;
    wifiInfo.wifiApSsidValid = apinfo_valid;
    wifiInfo.wifiApSsidInfoLen = ap_len;

    if(apinfo_valid) {
        if(rssi_arr != NULL && channel_arr != NULL && mac_arr != NULL) {
            wifiInfo.apInfoValid = 1;
            wifiInfo.apInfo.apLen = ap_len;

            for(int i=0;i<MAX_REPORTED_APS;i++) {
                wifiInfo.apInfo.rssi[i] = rssi_arr[i];
                wifiInfo.apInfo.channel[i] = channel_arr[i];
            }
            for(int j=0;j<(MAC_ADDRESS_LENGTH * MAX_REPORTED_APS);j++){
                wifiInfo.apInfo.mac_address[j] = mac_arr[j];
            }
        }

        for (int i=0;i<ap_len;i++)
        {
            int j=0;
            jstring string = (jstring)env->GetObjectArrayElement(ssid_array, i);
            const char* ssid = env->GetStringUTFChars(string, 0);
            for(j=0;ssid[j]!='\0';j++) {
                wifiInfo.wifiApSsidInfo[i].ssid[j] = ssid[j];
            }
            wifiInfo.wifiApSsidInfo[i].ssid[j] = '\0';
            env->ReleaseStringUTFChars(string, ssid);
            env->DeleteLocalRef(string);
        }
    }
    if (nLBSAdapter) {
        nLBSAdapter->injectWifiPosition(wifiInfo);
    }
    if(NULL != rssi_arr) {
        env->ReleaseIntArrayElements(rssi_array, rssi_arr, 0);
    }
    if(NULL != channel_arr) {
        env->ReleaseIntArrayElements(channel_array, channel_arr, 0);
    }
    if(NULL != mac_arr) {
        env->ReleaseByteArrayElements(mac_array, mac_arr, 0);
    }
}

static void sendPassiveLocation(JNIEnv* env, jobject obj, jint position_valid,
                                jdouble latitude, jdouble longitude,
                                jfloat accuracy, jint hor_confidence)
{
    ALOGD("Send Passive Location.\n");

    jint * rssi_arr = 0;
    jint * channel_arr = 0;
    jbyte * mac_arr = 0;
    CoarsePositionInfo cpInfo;
    memset(&cpInfo, 0, sizeof(cpInfo));

    cpInfo.latitudeValid = position_valid;
    cpInfo.latitude = latitude;
    cpInfo.longitudeValid = position_valid;
    cpInfo.longitude = longitude;
    cpInfo.horUncCircularValid = position_valid;
    cpInfo.horUncCircular = accuracy;
    cpInfo.horConfidenceValid = position_valid;
    cpInfo.horConfidence = hor_confidence;

    if (nLBSAdapter) {
        nLBSAdapter->injectCoarsePosition(cpInfo);
    }
}

static void sendWifiApInfo(JNIEnv* env, jobject obj,
                           jbyteArray mac_array, jintArray rssi_array,
                           jintArray channel_array, jint ap_len)
{
    ALOGD("Send Wifi Ap info\n");

    jint * rssi_arr = 0;
    jint * channel_arr = 0;
    jbyte * mac_arr = 0;
    WifiApInfo wifiApInfo;

    rssi_arr = env->GetIntArrayElements(rssi_array, 0);
    channel_arr = env->GetIntArrayElements(channel_array, 0);
    mac_arr = env->GetByteArrayElements(mac_array, 0);

    if(rssi_arr != NULL && channel_arr != NULL && mac_arr != NULL) {
        wifiApInfo.apLen = ap_len;
        for(int i=0;i<MAX_REPORTED_APS;i++) {
            wifiApInfo.rssi[i] = rssi_arr[i];
            wifiApInfo.channel[i] = channel_arr[i];
        }
        for(int j=0;j<(MAC_ADDRESS_LENGTH * MAX_REPORTED_APS);j++){
            wifiApInfo.mac_address[j] = mac_arr[j];
        }
    }

    if (nLBSAdapter) {
        nLBSAdapter->injectWifiApInfo(wifiApInfo);
    }
    if(NULL != rssi_arr) {
        env->ReleaseIntArrayElements(rssi_array, rssi_arr, 0);
    }
    if(NULL != channel_arr) {
        env->ReleaseIntArrayElements(channel_array, channel_arr, 0);
    }
    if(NULL != mac_arr) {
        env->ReleaseByteArrayElements(mac_array, mac_arr, 0);
    }
}

static void sendWifiEnabledStatus(JNIEnv *env, jobject obj, jint status) {
    ALOGD("Send Wifi Status info\n");

    if (nLBSAdapter) {
        nLBSAdapter->wifiEnabledStatusInject(status);
    }
}

static void sendWifiAttachmentStatus(JNIEnv *env, jobject obj, jint status,
                                     jint ap_mac_valid,
                                     jbyteArray ap_mac_array,
                                     jint ssid_valid,
                                     jcharArray ssid_array) {
    ALOGD("Send Wifi Supplicant info\n");

    if (nLBSAdapter) {
        WifiSupplicantInfo wifiSupplicantInfo;
        memset(&wifiSupplicantInfo, 0, sizeof(wifiSupplicantInfo));
        jbyte * mac_arr = 0;
        jchar * ssid_arr = 0;
        if(ap_mac_valid)
            mac_arr = env->GetByteArrayElements(ap_mac_array, 0);
        if(ssid_valid)
            ssid_arr = env->GetCharArrayElements(ssid_array, 0);

        wifiSupplicantInfo.attachState = status;
        wifiSupplicantInfo.apMacAddressValid = ap_mac_valid;
        if((mac_arr != NULL) && (wifiSupplicantInfo.apMacAddressValid == 1)) {
            for(int i=0;i<MAC_ADDRESS_LENGTH;i++) {
                wifiSupplicantInfo.apMacAddress[i] = mac_arr[i];
            }
        }
        int i=0;
        wifiSupplicantInfo.wifiApSsidValid = ssid_valid;
        if((ssid_arr != NULL) && (wifiSupplicantInfo.wifiApSsidValid == 1)) {
            for(i=0;i<SSID_LENGTH;i++) {
                if(ssid_arr[i+1] != '"')
                {
                    wifiSupplicantInfo.ssid[i] = ssid_arr[i+1];
                }
                else
                    break;
            }
            wifiSupplicantInfo.ssid[i] = '\0';
            LOC_LOGV("%s:%d] ssid is %s", __func__, __LINE__,
                     wifiSupplicantInfo.ssid);
        }

        nLBSAdapter->wifiAttachmentStatusInject(wifiSupplicantInfo);

        if(NULL != mac_arr) {
            env->ReleaseByteArrayElements(ap_mac_array, mac_arr, 0);
        }

        if(NULL != ssid_arr) {
            env->ReleaseCharArrayElements(ssid_array, ssid_arr, 0);
        }
    }
}

static JNINativeMethod sMethods[] = {
    /* name, signature, funcPtr */
    {"native_wiper_class_init", "()V", (void *)classInit},
    {"native_wiper_init", "(I)V", (void *)instanceInit},
    {"native_wiper_send_network_location",  "(IDDFII[B[I[III[Ljava/lang/String;)V", (void*)sendNetworkLocation},
    {"native_wiper_send_passive_location",  "(IDDFI)V", (void*)sendPassiveLocation},
    {"native_wiper_send_wifi_ap_info",  "([B[I[II)V", (void*)sendWifiApInfo},
    {"native_wiper_send_wifi_enabled_status",  "(I)V", (void *)sendWifiEnabledStatus},
    {"native_wiper_send_wifi_attachment_status",  "(II[BI[C)V", (void *)sendWifiAttachmentStatus},
};

int register_Wiper(JNIEnv* env) {
    return jniRegisterNativeMethods(env, "com/qualcomm/location/Wiper",
                                    sMethods, NELEM(sMethods));
}
