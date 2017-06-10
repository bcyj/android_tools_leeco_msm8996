/*
 *  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 */
/*
 * Not a Contribution, Apache license notifications and license are retained
 *  for attribution purposes only.
 */
/*
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
 * Listen Master Control
 *
 * Master Control allows setting global control parameters
 *    such as LISTEN_FEATURE_ENABLE and VOICE_WAKEUP_ENABLE
 *    via setParam() method.
 *
 * Includes implimentation of com_qualcomm_listen_ListenMasterControl
 *    JNI class methods
 *
 * In general all methods do the following 3 basic things:
 *    i)   extract fields from jobject input parameters and store into C++ variables
 *    ii)  call Native ListenReceiver C++ methods
 *    iii) insert output parameters and return values from native function into
 *         jobjects
 */

// enable verbose logs
#define LOG_NDEBUG  0
#define LOG_TAG "ListenMstrCntrl-JNI"
#include <utils/Log.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <jni.h>
#include "JNIHelp.h"  // for NELEM sizeof()/sizeof(x[0])

#include <ListenNativeTypes.h>
#include <ListenReceiver.h>

#include <com_qualcomm_listen_ListenMasterControl.h>

namespace android {

// com_qualcomm_listen_ListenReceiver.cpp function
//    to get ptr to ListenReceiver C++ object from ListenReceiver Java object
extern sp<ListenReceiver> getPtrFromJObject(JNIEnv* env, jobject thiz);

// ----------------------------------------------------------------------------
//    Native Methods
// ----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Set a Listen Parameter
 *
 * Class:     com_qualcomm_listen_ListenMasterControl
 * Method:    setParam
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 * Set a Master Control global parameter value
 * Types of parameters include
 *     LISTEN_FEATURE_ENABLE
 *     VOICEWAKEUP_FEATURE_ENABLE
 * The callback for this object must be set BEFORE this is called
 *    because once a feature is enabled the client must be ready to
 *    receive LISTEN_FEATURE events from ListenEngine.
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Param [in]  sParamType - parameter type expressed as String
 * Param [in]  sValue - value expressed as String
 * Return - status
 *      ENO_GLOBAL_CONTROL - permission to set global parameters denied
 *      ENOT_INITIALIZED - callback is not set
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenMasterControl_setParam
  (JNIEnv *env, jobject thiz, jstring sParamType, jstring sValue)
{
   ALOGV("Java_com_qualcomm_listen_ListenMasterControl_setParam entered");
   sp<ListenReceiver>   pListenReceiver = getPtrFromJObject(env, thiz);
   if (pListenReceiver == NULL) {
      ALOGE("_setParam: ERROR listener not set for MasterControl object");
      return (jint)LISTEN_ECALLBACK_NOT_SET;
   }
   ListenMasterControl *pMstrCtrlReceiver = (ListenMasterControl *)pListenReceiver.get();
   if (NULL == pMstrCtrlReceiver) {
      ALOGE("_setParam: ERROR pMstrCtrlReceiver NULL");
      return (jint)LISTEN_EFAILURE;
   }
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   listen_param_enum_t  eParamType;
   int32_t              iValue;

   // listener callback must be set before this is called
   jclass clazz = env->GetObjectClass(thiz);
   jfieldID listenerId = env->GetFieldID(clazz, "listener", "Lcom/qualcomm/listen/IListenEventProcessor;");
   if (listenerId == NULL) {
      ALOGE("_setParam: ERROR listener member var not found in MasterControl class");
        return (jint)LISTEN_EFAILURE;
   }
   void * pListener = (void*)env->GetObjectField(thiz, listenerId);
   if (NULL == pListener) {
      ALOGE("_setParam: ERROR listener not set for MasterControl object");
      return (jint)LISTEN_ECALLBACK_NOT_SET;
   }

   // convert param type string parameter to enum
   const char *ntvParamType =  env->GetStringUTFChars(sParamType, 0);
   if ( !(strncmp("ListenFeature", ntvParamType, sizeof("ListenFeature")) ) ) {
      eParamType = LISTEN_PARAM_LISTEN_FEATURE_ENABLE;
   } else if (!(strncmp("VoiceWakeupFeature", ntvParamType, sizeof("VoiceWakeupFeature")) ) )  {
      eParamType = LISTEN_PARAM_VOICE_WAKEUP_FEATURE_ENABLE;
   } else {
      ALOGE("_setParam: ERROR type %s not recognized", ntvParamType);
      return LISTEN_EBAD_PARAM;
   }

   // convert value string parameter to int
   const char *ntvValue =  env->GetStringUTFChars(sValue, 0);
   if (!(strncmp("enable",ntvValue, sizeof("enable")) ) ) {
      iValue = 1;
   } else if (!(strncmp("disable",ntvValue, sizeof("disable")) ) ) {
      iValue = 0;
   } else {
      ALOGE("_setParam: ERROR value %s not recognized", ntvValue);
      return LISTEN_EBAD_PARAM;
   }
   eStatus = pMstrCtrlReceiver->setParam(eParamType, iValue);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("_setParam(%d, %d): ERROR failed with %d", (int)eParamType, (int)iValue, (int)eStatus);
   }
   // release native strings
   env->ReleaseStringUTFChars(sParamType, ntvParamType);
   env->ReleaseStringUTFChars(sValue, ntvValue);
   ALOGV("_ListenMasterControl_setParam(%d, %d) returned %d", (int)eParamType, (int)iValue, eStatus);
   return (jint)eStatus;
}

/*
 * Query the value for a Master Control global parameter
 *
 * Class:     com_qualcomm_listen_ListenMasterControl
 * Method:    getParam
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Param [in]  sParamType - type specified as string
 * Return - current value for this parameter returned as a string
 */
JNIEXPORT jstring JNICALL Java_com_qualcomm_listen_ListenMasterControl_getParam
  (JNIEnv *env, jobject thiz, jstring sParamType)
{
   ALOGV("Java_com_qualcomm_listen_ListenMasterControl_getParam entered");
   sp<ListenReceiver> pListenReceiver = getPtrFromJObject(env, thiz);
   if (pListenReceiver == NULL) {
      ALOGE("_setParam: ERROR listener not set for MasterControl object");
      return (jstring)NULL;
   }
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   listen_param_enum_t eParamType;

   // convert param type string parameter to enum
   const char *ntvParamType = env->GetStringUTFChars(sParamType, 0);
   if ( !(strncmp("ListenFeature", ntvParamType, sizeof("ListenFeature")) ) ) {
      eParamType = LISTEN_PARAM_LISTEN_FEATURE_ENABLE;
   } else if (!(strncmp("VoiceWakeupFeature", ntvParamType, sizeof("VoiceWakeupFeature")) ) )  {
      eParamType = LISTEN_PARAM_VOICE_WAKEUP_FEATURE_ENABLE;
   } else {
      ALOGE("_getParam: ERROR type %s not recognized", ntvParamType);
      return (jstring)NULL;
   }

   int32_t iValue = pListenReceiver->getParam(eParamType);
   // convert value to string then to jstring
   jstring jstrBuf; // will be GC'ed by Java
   if (iValue == 0) {
      jstrBuf = env->NewStringUTF("disable");
   } else if (iValue == 1) {
      jstrBuf = env->NewStringUTF("enable");
   } else {
      jstrBuf = env->NewStringUTF("invalid");
   }

   const char *pStrBuf = env->GetStringUTFChars(jstrBuf, 0);
   ALOGV("_ListenMasterControl_getParam(%d) returns %d as %s.", (int)eParamType, (int)iValue, pStrBuf);
   // release native strings
   env->ReleaseStringUTFChars(jstrBuf, pStrBuf);
   env->ReleaseStringUTFChars(sParamType, ntvParamType);
   return jstrBuf;
}

#ifdef __cplusplus
}
#endif // __cplusplus


}; // namespace android
