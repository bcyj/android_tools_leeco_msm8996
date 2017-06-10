/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/

#define LOG_TAG "Ulp_jni"
#define LOG_NDEBUG 0

#include "AndroidRuntime.h"
#include "utils/Log.h"
#include "hardware/gps.h"

#include <string.h>
#include <dlfcn.h>

#include "JNIHelp.h"
#include "jni.h"
#include "loc_cfg.h"
#include "ulp_service.h"

const GpsInterface *pGpsInterface = NULL;

static jobject mCallbacksObj = NULL;
static jmethodID method_reportLocation;
static jmethodID method_requestNetworkLocation;
static jmethodID method_requestPhoneContext;
static jmethodID method_reportPVDStatus;

static const UlpEngineInterface* pUlpEngineInterface = NULL;
static const UlpNetworkInterface* pUlpNetworkInterface = NULL;
static const UlpPhoneContextInterface* pUlpPhoneContextInterface = NULL;
static const InjectRawCmdInterface* pUlpInjectRawCmd = NULL;
static const UlpPVDQueryInterface* pUlpPVDQueryInterface = NULL;


/*=============================================================================================
 * Function description:
 *  Helper function to convert a 16 byte array to a 16 byte number in the form of two
 *  longs (most significant 8 bytes, and least significant 8 bytes)
 *
 * Parameters:
 *    bytes, the 16 byte array
 *    most, pointer to the most siginificant 8 bytes
 *    least, pointer to the least siginificant 8 bytes
 *
 * Return value:
 *    error code: 0: success
 =============================================================================================*/
#define UUID_STRING_LENGTH (32+4+1) // 16 * 2 + 4 (-) + 1 null ending
static void convert_uuid_from_byte_array_to_string (unsigned char* uuid_byte_array, char *uuid_string_buf)
{
    memset (uuid_string_buf, 0, UUID_STRING_LENGTH);
    snprintf (uuid_string_buf, UUID_STRING_LENGTH,
              "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
              (*(uuid_byte_array+0) & 0xff),
              (*(uuid_byte_array+1) & 0xff),
              (*(uuid_byte_array+2) & 0xff),
              (*(uuid_byte_array+3) & 0xff),
              (*(uuid_byte_array+4) & 0xff),
              (*(uuid_byte_array+5) & 0xff),
              (*(uuid_byte_array+6) & 0xff),
              (*(uuid_byte_array+7) & 0xff),
              (*(uuid_byte_array+8) & 0xff),
              (*(uuid_byte_array+9) & 0xff),
              (*(uuid_byte_array+10) & 0xff),
              (*(uuid_byte_array+11) & 0xff),
              (*(uuid_byte_array+12) & 0xff),
              (*(uuid_byte_array+13) & 0xff),
              (*(uuid_byte_array+14) & 0xff),
              (*(uuid_byte_array+15) & 0xff));
    ALOGD ("UUID string = %s\n", uuid_string_buf);
}

static void checkAndClearExceptionFromCallback(JNIEnv* env, const char* methodName) {
    if (env->ExceptionCheck()) {
        ALOGE("An exception was thrown by callback '%s'.", methodName);
        env->ExceptionClear();
    }
}

static pthread_t create_thread_callback(const char* name, void (*start)(void *), void* arg)
{
    return (pthread_t)android::AndroidRuntime::createJavaThread(name, start, arg);
}

static void location_callback(UlpLocation* location)
{
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();

    jbyteArray byteArray = NULL;
    if (location->rawDataSize > 0) {
       byteArray = env->NewByteArray(location->rawDataSize);
       ALOG_ASSERT(byteArray, "Native could not create new byte[]");
       env->SetByteArrayRegion(byteArray, 0, location->rawDataSize, (const jbyte *) location->rawData );
    }

    jstring java_string_map_url = NULL;
    ALOGD ("Location CB invoked");
    if ((location->gpsLocation.flags & GPS_LOCATION_HAS_MAP_URL) == GPS_LOCATION_HAS_MAP_URL) {
      java_string_map_url = env->NewStringUTF(location->map_url);
    }

    jstring java_string_map_index = NULL;
    if ((location->gpsLocation.flags & GPS_LOCATION_HAS_MAP_INDEX) == GPS_LOCATION_HAS_MAP_INDEX) {

      char uuid_string_buf  [UUID_STRING_LENGTH];
      convert_uuid_from_byte_array_to_string (location->map_index, uuid_string_buf);
      java_string_map_index = env->NewStringUTF(uuid_string_buf);
    }

    env->CallVoidMethod(mCallbacksObj, method_reportLocation, location->gpsLocation.flags,
            (jdouble)location->gpsLocation.latitude, (jdouble)location->gpsLocation.longitude,
            (jdouble)location->gpsLocation.altitude, (jfloat)location->gpsLocation.speed,
            (jfloat)location->gpsLocation.bearing, (jfloat)location->gpsLocation.accuracy,
            (jlong)location->gpsLocation.timestamp, location->position_source,
            byteArray, (jboolean)location->is_indoor, (jfloat)location->floor_number,
            java_string_map_url, java_string_map_index);

    if (byteArray!= NULL) {
        env->DeleteLocalRef(byteArray);
    }

    if (java_string_map_url != NULL) {
        env->DeleteLocalRef(java_string_map_url);
    }

    if (java_string_map_index != NULL) {
        env->DeleteLocalRef(java_string_map_index);
    }

    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

// PVD Status callback from ULPNative
static void pvd_callback (UlpPVDStatus status, void* venueInfo,
    int venueInfoLen)
{
  JNIEnv* env = android::AndroidRuntime::getJNIEnv();
  ALOGD ("PVD CB invoked, status = %d, len = %d", status, venueInfoLen);

  jbyteArray byteArray = NULL;
  if (venueInfoLen > 0 && venueInfo != NULL) {
     byteArray = env->NewByteArray(venueInfoLen);
     ALOG_ASSERT(byteArray, "Native could not create new byte[]");
     env->SetByteArrayRegion(byteArray, 0, venueInfoLen, (const jbyte *) venueInfo);
  }

  env->CallVoidMethod(mCallbacksObj, method_reportPVDStatus, (jint) status,
          byteArray);

  if (byteArray!= NULL) {
      env->DeleteLocalRef(byteArray);
  }

  checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

UlpEngineCallbacks sUlpEngineCallbacks = {
    sizeof(UlpEngineCallbacks),
    location_callback,
    create_thread_callback,
    pvd_callback
};

//ULP Network Location Callback
static void ulp_network_location_request_cb (UlpNetworkRequestPos* req)
{
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();
    ALOGD ("ulp_network_location_request_cb invoked");
    env->CallVoidMethod(mCallbacksObj, method_requestNetworkLocation, req->request_type,req->interval_ms,req->desired_position_source );
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

UlpNetworkLocationCallbacks pUlpNetworkLocationCallbacks = {
    ulp_network_location_request_cb,
};

static void ulp_request_phone_context_cb(UlpPhoneContextRequest *req)
{
    JNIEnv* env = android::AndroidRuntime::getJNIEnv();
    ALOGD ("ulp_request_phone_context_cb invoked");
    env->CallVoidMethod(mCallbacksObj, method_requestPhoneContext,req->context_type ,
                        req->request_type );
    checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

UlpPhoneContextCallbacks pUlpPhoneContextCallbacks = {
    ulp_request_phone_context_cb,
};

static void classInit(JNIEnv* env, jclass clazz) {
    ALOGD ("classInit invoked");

    method_reportLocation = env->GetMethodID(clazz, "reportLocation", "(IDDDFFFJI[BZFLjava/lang/String;Ljava/lang/String;)V");
    method_requestNetworkLocation = env->GetMethodID(clazz, "requestNetworkLocation", "(III)V");
    method_requestPhoneContext = env->GetMethodID(clazz, "requestPhoneContext", "(II)V");
    method_reportPVDStatus = env->GetMethodID(clazz, "reportPVDStatus", "(I[B)V");
}

static void instanceInit(JNIEnv *env, jobject obj) {
    ALOGD ("instanceInit invoked");
    // this must be set before calling into the HAL library
    if (!mCallbacksObj)
        mCallbacksObj = env->NewGlobalRef(obj);
    ulp_init(&sUlpEngineCallbacks, &pUlpNetworkLocationCallbacks, &pUlpPhoneContextCallbacks);
    pUlpEngineInterface =
        (const UlpEngineInterface*)ulp_get_extension(ULP_ENGINE_INTERFACE);
    pUlpNetworkInterface =
        (const UlpNetworkInterface*)ulp_get_extension(ULP_NETWORK_INTERFACE);
    pUlpPhoneContextInterface =
        (const UlpPhoneContextInterface*)ulp_get_extension(ULP_PHONE_CONTEXT_INTERFACE);
    pUlpInjectRawCmd =
        (const InjectRawCmdInterface*)ulp_get_extension(ULP_RAW_CMD_INTERFACE);
    pUlpPVDQueryInterface =
        (const UlpPVDQueryInterface*)ulp_get_extension(ULP_PVD_QUERY_INTERFACE);
    if((NULL == pUlpEngineInterface)||
       (NULL == pUlpNetworkInterface)||
       (NULL == pUlpPhoneContextInterface)) {
        ALOGE("Error in classInit.ulp_get_extension is null ");
    }
}

static jboolean update_criteria(JNIEnv* env, jobject obj,
        jint source, jint action, jlong minTime, jfloat minDistance, jboolean singleShot,
        jint horizontalAccuracy, jint powerRequirement)
{
    UlpLocationCriteria native_criteria;
    ALOGD("JNI:Inupdate_criteria: action:%d, minTime:%ld, minDistance:%f, singleShot:%d, horizontalAccuracy:%d, powerRequirement:%d \n",
         action, minTime,minDistance, singleShot,horizontalAccuracy,powerRequirement );
    native_criteria.valid_mask = (ULP_CRITERIA_HAS_ACTION | ULP_CRITERIA_HAS_PROVIDER_SOURCE | ULP_CRITERIA_HAS_RECURRENCE_TYPE |
                                  ULP_CRITERIA_HAS_MIN_INTERVAL);
    native_criteria.provider_source = source;
    native_criteria.action = action;
    native_criteria.min_interval = minTime;
    native_criteria.min_distance = minDistance;
    native_criteria.recurrence_type = singleShot? ULP_LOC_RECURRENCE_SINGLE:ULP_LOC_RECURRENCE_PERIODIC;
    native_criteria.preferred_horizontal_accuracy = (UlpHorzAccuracyCriteria) horizontalAccuracy;
    native_criteria.preferred_power_consumption = (UlpPowerCriteria)powerRequirement;
    // Criteria will have valid values for accuracy and power requirements only for HybridProvider
    if(source == ULP_PROVIDER_SOURCE_HYBRID)
    {
       native_criteria.valid_mask |= ( ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY
                                       | ULP_CRITERIA_HAS_PREFERRED_POWER_CONSUMPTION );
    }

    ALOGD("JNI:Inupdate_criteria: After translation action:%d, minTime:%lld, minDistance:%f, singleShot:%d, horizontalAccuracy:%d, powerRequirement:%d \n",
         native_criteria.action, native_criteria.min_interval,native_criteria.min_distance,
         native_criteria.recurrence_type,native_criteria.preferred_horizontal_accuracy,native_criteria.preferred_power_consumption );
    if (pUlpEngineInterface){
        ALOGD("JNI:In update_criteria:Before call to interface->update_criteria(native_criteria)");
        return (pUlpEngineInterface->update_criteria(native_criteria) == 0);
    } else
        return false;
}

static jboolean update_settings(JNIEnv* env, jobject obj,
        jint currentContextType, jboolean currentGpsSetting, jboolean currentAgpsSetting,
        jboolean currentNetworkProvSetting, jboolean currentWifiSetting,
        jboolean currentBatteryCharging, jboolean currentEnhLocationServicesSetting,
         jboolean currentPipUserSetting )
{
    ALOGD("JNI:In update_settings:currentContextType %d, currentGpsSetting %d, currentAgpsSetting %d,"
          "currentNetworkProvSetting %d,currentWifiSetting %d,currentBatteryCharging %d,currentEnhLocationServicesSetting %d"
          " currentPipUserSetting %d \n",
          currentContextType, currentGpsSetting, currentAgpsSetting, currentNetworkProvSetting, currentWifiSetting, currentBatteryCharging,
          currentEnhLocationServicesSetting, currentPipUserSetting);
    if (pUlpPhoneContextInterface->ulp_phone_context_settings_update ) {
        UlpPhoneContextSettings settings;
        settings.context_type = currentContextType;
        settings.is_gps_enabled = currentGpsSetting;
        settings.is_agps_enabled = currentAgpsSetting;
        settings.is_network_position_available = currentNetworkProvSetting;
        settings.is_wifi_setting_enabled = currentWifiSetting;
        settings.is_battery_charging = currentBatteryCharging;
        settings.is_enh_location_services_enabled = currentEnhLocationServicesSetting;
        settings.is_pip_user_setting_enabled = currentPipUserSetting;
        return pUlpPhoneContextInterface->ulp_phone_context_settings_update(&settings);
    }
    else
        return false;
}

static void send_network_location(JNIEnv* env, jobject obj,
        jdouble latitude, jdouble longitude, jfloat accuracy, jlong utctime)
{
    ALOGD("send_network_location.\n");
    if(pUlpNetworkInterface != NULL) {
        UlpNetworkPositionReport position_report;
        position_report.valid_flag = (ULP_NETWORK_POSITION_REPORT_HAS_POSITION |
                                      ULP_NETWORK_POSITION_REPORT_HAS_FIX_TIME);
        position_report.position.latitude = latitude;
        position_report.position.longitude = longitude;
        position_report.position.HEPE = accuracy;
        position_report.position.pos_source = ULP_NETWORK_POSITION_SRC_UNKNOWN;
        position_report.fix_time = utctime;
        pUlpNetworkInterface->ulp_send_network_position(&position_report);
    }
}

static jboolean start(JNIEnv* env, jobject obj)
{
    ALOGD("JNI:start");
    if (pUlpEngineInterface)
        return (pUlpEngineInterface->start() == 0);
    else
        return false;
}

static jboolean stop(JNIEnv* env, jobject obj)
{
    ALOGD("JNI:stop");
    if (pUlpEngineInterface)
        return (pUlpEngineInterface->stop() == 0);
    else
        return false;
}

static jboolean system_update(JNIEnv* env, jobject obj, jint systemEvent)
{
    jboolean result = false;
    ALOGD("JNI:system_update. Event-%d", (UlpSystemEvent)systemEvent );

    if (pUlpEngineInterface)
        return (pUlpEngineInterface->system_update((UlpSystemEvent)systemEvent) == 0);
    else
        return false;
}

static void inject_raw_cmd(JNIEnv* env, jobject obj, jstring rawCmd, int len)
{
    if (pUlpInjectRawCmd) {
        const char* rc = env->GetStringUTFChars(rawCmd, NULL);
        pUlpInjectRawCmd->inject_raw_cmd((char*)rc, len);
        env->ReleaseStringUTFChars(rawCmd, rc);
    }
}

static void send_pvd_query(JNIEnv* env, jobject obj,
    jboolean pos_valid, jdouble latitude, jdouble longitude, jfloat accuracy)
{
  ALOGD("send_pvd_query.\n");
  if(pUlpPVDQueryInterface != NULL) {
    UlpPVDQueryRequest query;
    query.valid_flag = ULP_PVD_QUERY_HAS_POSITION;
    query.latitude = latitude;
    query.longitude = longitude;
    query.accuracy = accuracy;
    pUlpPVDQueryInterface->ulp_send_pvd_query(&query);
  }
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
    {"native_ue_class_init", "()V", (void *)classInit},
    {"native_ue_init", "()V", (void *)instanceInit},
    {"native_ue_update_criteria", "(IIJFZII)Z", (void*) update_criteria},
    {"native_ue_update_settings", "(IZZZZZZZ)Z", (void*) update_settings},
    {"native_ue_send_network_location", "(DDFJ)V", (void*)send_network_location},
    {"native_ue_start", "()Z", (void*) start},
    {"native_ue_stop", "()Z", (void*) stop},
    {"native_ue_system_update", "(I)Z", (void*) system_update},
    {"native_ue_inject_raw_cmd", "(Ljava/lang/String;I)Z", (void*) inject_raw_cmd},
    {"native_ue_send_pvd_query", "(ZDDF)V", (void*)send_pvd_query}
};

int register_UlpEngine(JNIEnv* env) {
    return jniRegisterNativeMethods(env, "com/qualcomm/location/ulp/UlpEngine",
                                    sMethods, NELEM(sMethods));
}


