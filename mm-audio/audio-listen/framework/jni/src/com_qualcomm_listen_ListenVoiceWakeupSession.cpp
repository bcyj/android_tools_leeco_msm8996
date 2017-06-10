/*
 *  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
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
** See the License for the specific language governing pfermissions and
** limitations under the License.
*/

/*
 * Listen VoiceWakeup Session
 *
 * VoiceWakeup Session is a particular type of ListenReceiver used to
 * perform voice wakeup detection.
 *
 * includes implimentation of
 *     com_qualcomm_listen_ListenVoiceWakeupSession JNI class methods
 *
 * In general all methods do the following 3 basic things:
 *    i)   extract fields from jobject input parameters and store into C++ variables
 *    ii)  call Native ListenReceiver C++ methods
 *    iii) insert output parameters and return values from native function into
 *         jobjects
 *
 * Implementation Notes:
 *
 * 1) NULL ptr checks for StrongPointer objects sp<Xyz> pXyz must be of the form
 *       if (pXyz == NULL) { }
 *    Attempting to use "if (NULL == pXyz)" results in compilation error:
 *       "no match for 'operator==' in '0 == pXyz'"
 * 2) It is expected that Java will Garbage Collect all allocated variables:
 *       a) are returned by JNI methods
 *       b) are contained within other Java objects
 *    Code allocating this objects are marked with a comment "GC'ed"
 */

// enable verbose logs
#define LOG_NDEBUG  0
#define LOG_TAG "ListenVWUSession-JNI"
#include <utils/Log.h>

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <jni.h>
#include "JNIHelp.h"  // for NELEM sizeof()/sizeof(x[0])

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <ListenNativeTypes.h>
#include <ListenReceiver.h>

#include <com_qualcomm_listen_ListenVoiceWakeupSession.h>

#include <ListenSoundModelLib_v2.h>
#include <ListenSoundModelLib.h>
#include <SoundModel.h>

namespace android {

// ----------------------------------------------------------------------------
//    External Methods
// ----------------------------------------------------------------------------
// com_qualcomm_listen_ListenReceiver.cpp function
//    to get ptr to ListenReceiver C++ object from ListenReceiver Java object
extern  sp<ListenReceiver> getPtrFromJObject(JNIEnv* env, jobject thiz);

// com_qualcomm_listen_ListenSoundModel.cpp function
//    to get ptr to and size of sound model data from ByteBuffer object
extern listen_status_enum_t getSMFromByteBuff(JNIEnv *env,
                                       jobject byteBuffObj,
                                       uint32_t *pSize, uint8_t **ppData,
                                       listen_sound_model_info  *pModelInfo);


// ----------------------------------------------------------------------------
//    Native Methods
// ----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Get Parameter
 *
 * Class:     com_qualcomm_listen_ListenVoiceWakeupSession
 * Method:    getParam
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 *
 * Query the value of a particular Listen parameter
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Param [in]  sParamType - type specified as string
 * Return - current value for this parameter returned as a string
 *      status is not returned
 */
JNIEXPORT jstring JNICALL Java_com_qualcomm_listen_ListenVoiceWakeupSession_getParam
  (JNIEnv *env, jobject thiz, jstring sParamType)
{
   ALOGV("Java_com_qualcomm_listen_ListenVoiceWakeupSession_getParam entered");
   sp<ListenReceiver> pListenReceiver = getPtrFromJObject(env, thiz);
   if (pListenReceiver == NULL) {
      ALOGE("_getParam: ERROR listener not set for VoiceWakeupSession object");
      return (jstring)NULL;
   }
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   listen_param_enum_t eParamType;

   // convert param type string parameter to enum
   const char *ntvParamType = env->GetStringUTFChars(sParamType, 0);
   if ( !(strncmp("ListenFeature", ntvParamType, sizeof("ListenFeature")) ) ) {
      eParamType = LISTEN_PARAM_LISTEN_FEATURE_ENABLE;
   } else if (!(strncmp("VoiceWakeupFeature", ntvParamType, sizeof("VoiceWakeupFeature")) ) ) {
      eParamType = LISTEN_PARAM_VOICE_WAKEUP_FEATURE_ENABLE;
   } else {
      ALOGE("setParam: ERROR type %s not recognized", ntvParamType);
      env->ReleaseStringUTFChars(sParamType, ntvParamType);
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

   // release native strings
   env->ReleaseStringUTFChars(sParamType, ntvParamType);
   ALOGV("getParam(%d) returns %d",(int)eParamType, (int)iValue);
   return jstrBuf;
}

/*
 * Register SoundModel
 *
 * Class:     com_qualcomm_listen_ListenVoiceWakeupSession
 * Method:    registerSoundModel
 * Signature: (Lcom/qualcomm/listen/ListenTypes/RegisterParams;)I
 *
 * Specifies the sound model data that Listen engine uses to detect desired keyword/sound
 * for this session.
 * This method can be called for SVA 1.0 or 2.0 and above SoundModels
 * Session must be initialized before calling
 *
 * The input jobject contains arrays with {KWStr, confLevel} and {KWStr, UserStr, confLevel}.
 * From these structures this method must construct a byte array with confidence levels
 * for KW then User-Pairings in the exact order these are found in the SM.
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Param [in]  registrationParams - ptr to data and settings required to register a SoundModel
 *            ptr to structure of type ListenTypes.RegisterParams
 *
 * Return - status
 *     EBAD_PARAM
 *     ESESSION_NOT_INITIALIZED
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenVoiceWakeupSession_registerSoundModel
                (JNIEnv *env, jobject thiz,
                 jobject registrationParams)    //  ListenTypes$RegisterParams base class
{
   ALOGV("Java_com_qualcomm_listen_ListenVoiceWakeupSession_registerSoundModel entered");
   listen_status_enum_t  eStatus = LISTEN_SUCCESS;
   listen_status_enum    status = kSucess;

   jfieldID     detectModeId;
   jfieldID     failNotifyId;
   jfieldID     kwMinConfLevelsId;
   jfieldID     userKWPairMinConfLevelsId;
   jobject      jArrayObj;
   uint8_t *    pConfLevels = NULL;
   jobjectArray kwMinConfLevsArr;
   jobjectArray userKWPairMinConfLevsArr;
   short        numKeywords;  // from arrays passed in as input param
   short        numUserKWPairs; // from arrays passed in as input param
   short        totalConfLevelSize;
   jobject      pairConfLevel;
   jobject      kwConfLevel;
   jclass       uKWPairClazz;
   jclass       kwClazz;
   jfieldID     keywordId;
   jfieldID     userId;
   jfieldID     confLevelId;
   uint16_t     numSMKeywords = 0; // queried from the soundmodel
   uint16_t     numSMUsers = 0;     // queried from the soundmodel
   uint16_t     numSMUserKWPairs = 0;   // queried from the soundmodel
   jobject      strObj;
   jstring      keywordStrObj;
   jstring      userStrObj;
   uint16_t     index;
   char *       iKWStr = NULL;
   char *       iUserStr = NULL;
   uint8_t      iConfLevel = 0;
   uint16_t *   pTmpPairPtr = NULL;
   bool         bSvaSoundModel = true;
   char **      keywordPhrases = NULL;
   char **      userNames = NULL;
   uint32_t     strippedModelSize;
   jclass       labClazz;
   jfieldID     buffEnableId;
   jfieldID     labId;
   jobject      jLabObj;
   size_t       strCpyLen;

   if (NULL == registrationParams) {
      ALOGE("_registerSoundModel: ERROR registrationParams is NULL");
      return (jint)LISTEN_EBAD_PARAM;
   }

   // listener callback must be set before this is called
   sp<ListenReceiver>   pListenReceiver = getPtrFromJObject(env, thiz);
   if (pListenReceiver == NULL) {
      ALOGE("_registerSoundModel: ERROR listener not set for VoiceWakeupSession object");
      return (jint)LISTEN_ECALLBACK_NOT_SET;
   }
   ListenVWUSession *pVWUSessionReceiver = (ListenVWUSession *)pListenReceiver.get();
   if (NULL == pVWUSessionReceiver) {
      ALOGE("_registerSoundModel: ERROR pVWUSessionReceiver NULL");
      return (jint)LISTEN_EFAILURE;
   }
   listen_sound_model_params_t nativeSoundModelParams;
   listen_sound_model_data_t  soundModelStruct;
   nativeSoundModelParams.sound_model_data = &soundModelStruct;

   listen_model_type strippedSM;
   strippedSM.size = 0;
   strippedSM.data = NULL;

   jclass regParamsClazz = env->FindClass("com/qualcomm/listen/ListenTypes$RegisterParams");
   if (regParamsClazz == NULL) {
      ALOGE("_registerSoundModel: ERROR FindClass com/qualcomm/listen/ListenTypes$RegisterParams failed");
      return (jint)LISTEN_EFAILURE;
   }
   jfieldID smBuffObjId = env->GetFieldID(regParamsClazz, "soundModelData", "Ljava/nio/ByteBuffer;");  // jobject
   if (NULL == smBuffObjId) {
      ALOGE("_registerSoundModel: GetFieldId soundModelData from RegisterParams class failed");
      return (jint)LISTEN_EFAILURE;
   }
   // soundmodel data is stored as a ByteBuffer jobject, so extract the byte array from it
   ALOGV("_registerSoundModel: GetObjectField smBuffObjId as object");
   jobject smBuffObj = env->GetObjectField(registrationParams, smBuffObjId);
   ALOGV("_registerSoundModel: ref to Java SM to be registered = %p", smBuffObj);
   listen_sound_model_info     modelInfo;
   ALOGV("_registerSoundModel: call getSMFromByteBuff w/ smBuffObj = %p", smBuffObj);
   eStatus = getSMFromByteBuff(env, smBuffObj,
                   &nativeSoundModelParams.sound_model_data->size,
                   &nativeSoundModelParams.sound_model_data->p_data,
                   &modelInfo );
   if ( LISTEN_SUCCESS != eStatus ) {
      ALOGE("getSMFromByteBuff returned %d", eStatus);
      goto clean_up_reg_sm;
   }

   // set type version
   if (modelInfo.version == SM_VERSION_1p0) {
       ALOGD("_registerSoundModel: SVA Version 1.0");
       nativeSoundModelParams.sound_model_type = LISTEN_SM_TYPE_SVA;
       pVWUSessionReceiver->mAlgoVersion =  ALGO_VERS_0100;
   } else {
       ALOGD("_registerSoundModel: SVA Version 2.0");
       nativeSoundModelParams.sound_model_type = LISTEN_SM_TYPE_SVA_V2;
       pVWUSessionReceiver->mAlgoVersion =  ALGO_VERS_0200;
   }

   detectModeId = env->GetFieldID(regParamsClazz, "detectionMode", "I");
   failNotifyId = env->GetFieldID(regParamsClazz, "bFailureNotification", "Z"); // bool
   // get keyword and userKWPair count from arrays in
   kwMinConfLevelsId = env->GetFieldID(regParamsClazz, "keywordMinConfLevels",
                "[Lcom/qualcomm/listen/ListenTypes$VWUKeywordConfLevel;");
   userKWPairMinConfLevelsId = env->GetFieldID(regParamsClazz, "userKWPairMinConfLevels",
                "[Lcom/qualcomm/listen/ListenTypes$VWUUserKeywordPairConfLevel;");
   if ( (NULL == detectModeId) || (NULL == failNotifyId) ||
        (NULL == kwMinConfLevelsId) || (NULL == userKWPairMinConfLevelsId) )
   {
      ALOGE("_registerSoundModel: GetFieldId Listen RegisterParams class failed");
      eStatus = LISTEN_EFAILURE;
      goto clean_up_reg_sm;
   }

   nativeSoundModelParams.detection_mode = (listen_detection_mode_enum_t)env->GetIntField(registrationParams, detectModeId);
   nativeSoundModelParams.detect_failure = (bool)env->GetBooleanField(registrationParams, failNotifyId);

   // check parameters for valid ranges of value
   if ( (LISTEN_MODE_KEYWORD_ONLY_DETECTION != nativeSoundModelParams.detection_mode) &&
        (LISTEN_MODE_USER_KEYWORD_DETECTION != nativeSoundModelParams.detection_mode) ) {
       ALOGE("_registerSoundModel: ERROR: invalid detection mode %d", nativeSoundModelParams.detection_mode);
       eStatus = LISTEN_EBAD_PARAM;
       goto clean_up_reg_sm;
   }

   ALOGV("_registerSoundModel: - SM (ptr,size)=(%p,%d), ...",
      nativeSoundModelParams.sound_model_data->p_data,
      nativeSoundModelParams.sound_model_data->size);
   ALOGV("         ... mode %d, failureFlag %d, ...",
      nativeSoundModelParams.detection_mode,
      nativeSoundModelParams.detect_failure);

   jArrayObj = env->GetObjectField(registrationParams, kwMinConfLevelsId);
   kwMinConfLevsArr = (jobjectArray)jArrayObj;
   if (NULL == kwMinConfLevsArr) {
       ALOGE("_registerSoundModel: ERROR: array of Keyword confidence levels can not be null");
       eStatus = LISTEN_EBAD_PARAM;
       goto clean_up_reg_sm;
   }
   numKeywords = (short)env->GetArrayLength(kwMinConfLevsArr);

   jArrayObj = env->GetObjectField(registrationParams, userKWPairMinConfLevelsId);
   userKWPairMinConfLevsArr = (jobjectArray)jArrayObj;
   if (modelInfo.version >= SM_VERSION_2p0) {
      if (NULL == userKWPairMinConfLevsArr) {
         ALOGD("_registerSoundModel: No user+keyword pair confidence levels given");
         numUserKWPairs = 0;
      } else {
         numUserKWPairs = (short)env->GetArrayLength(userKWPairMinConfLevsArr);
      }
   } else {
      if (numKeywords > 1) {
         ALOGE("_registerSoundModel: ERROR: SVA 1.0 SM have a single Keyword, input array too big");
         eStatus = LISTEN_EBAD_PARAM;
         goto clean_up_reg_sm;
      }
      // SVA 1.0 confidence levels should always be pass down as array of 2 - 1 KW + 1 UserPair
      numUserKWPairs = 1;
   }

   ALOGV("_registerSoundModel: num KW %d, numPairs %d", numKeywords, numUserKWPairs);
   totalConfLevelSize = numKeywords + numUserKWPairs;

   // compare these array sizes with number of keywords and active pairs in SM
   listen_model_type   keywordModel;
   keywordModel.data = nativeSoundModelParams.sound_model_data->p_data;
   keywordModel.size = nativeSoundModelParams.sound_model_data->size;
   listen_sound_model_header modelHeader;

   if (modelInfo.version >= SM_VERSION_2p0) {
      ALOGV("_registerSoundModel: SVA SM Version 2 (%d in header)", modelInfo.version);
      // SoundModel being registered Version 2 or above
      // Call SVA Version 2 functions to get number of keywords, users, user pairs,...
      //    then convert char strings to Unicode jstrings
      memset(&modelHeader, 0, sizeof(listen_sound_model_header));
      status = ListenSoundModelLibV2::getSoundModelHeader(&keywordModel, &modelHeader);
      if (kSucess != status) {
         ALOGE("_registerSoundModel: ERROR getSoundModelHeader() failed");
         MAP_ERR(status, eStatus);
         goto clean_up_reg_sm;
      }
      numSMKeywords = modelHeader.numKeywords;
      numSMUsers = modelHeader.numUsers;
      numSMUserKWPairs =  modelHeader.numActiveUserKeywordPairs;	  // total number of active user+keyword pairs in SM
      ALOGV("_registerSoundModel: num KW %d, num Users %d, numPairs %d from getSoundModelHeader()",
            numSMKeywords, numSMUsers, numSMUserKWPairs);
      if (numKeywords != numSMKeywords) {
         ALOGE("_registerSoundModel: ERROR: number of keyword conf levels %d doesnot match count in SM %d",
            (int)numKeywords, (int)numSMKeywords);
            eStatus = LISTEN_EBAD_PARAM;
            goto clean_up_reg_sm;
      }
      if (numUserKWPairs != numSMUserKWPairs) {
         ALOGE("_registerSoundModel: ERROR: number of pair conf levels %d doesnot match count in SM %d",
            (int)numUserKWPairs, (int)numSMUserKWPairs);
            eStatus = LISTEN_EBAD_PARAM;
            goto clean_up_reg_sm;
      }
   }
   else
   {
      ALOGV("_registerSoundModel: SVA SM Version 1 (%d in header)", modelInfo.version);
      // SoundModel being registered is SVA 1.0
	   numSMKeywords = 1;
      // total number of active user+keyword pairs is at most 1
      if (modelInfo.type == kUserKeywordModel)
         numSMUsers = 1;
      else
         numSMUsers = 0;
   }

   ALOGV("Allocate byte array of size %d for all confidence levels", totalConfLevelSize);
   pConfLevels  = (uint8_t *)calloc(totalConfLevelSize, 1);
   if (NULL == pConfLevels) {
      eStatus = LISTEN_ENO_MEMORY;
      goto clean_up_reg_sm;
   }

   kwClazz = env->FindClass("com/qualcomm/listen/ListenTypes$VWUKeywordConfLevel");
   if (kwClazz == NULL) {
      ALOGE("_registerSoundModel: ERROR FindClass com/qualcomm/listen/ListenTypes$VWUKeywordConfLevel failed");
      eStatus = LISTEN_EFAILURE;
      goto clean_up_reg_sm;
   }
   keywordId = env->GetFieldID(kwClazz, "keyword", "Ljava/lang/String;");  // jobject
   if ( (NULL == keywordId) ) {
      ALOGE("_registerSoundModel: GetFieldId keyword field for Listen VWUKeywordConfLevel failed");
      eStatus = LISTEN_EFAILURE;
      goto clean_up_reg_sm;
   }
   confLevelId = env->GetFieldID(kwClazz, "confLevel", "S"); // short
   if ( (NULL == confLevelId) )  {
      ALOGE("_registerSoundModel: GetFieldId confLevel field for Listen VWUKeywordConfLevel failed");
      eStatus = LISTEN_EFAILURE;
      goto clean_up_reg_sm;
   }

   // get elements from keyword min confidence level array and store into conf level byte array
   ALOGV("GetByteArrayRegion for kwMinConfLevels w/ kwMinConfLevsArr=%p, len=%d, into pConfLevels=%p",
      kwMinConfLevsArr, numKeywords, pConfLevels );
   for (int i=0; i < numKeywords; i++) {
      // extract individual sound model params from jobject into struct listen_sound_model_params_t
      kwConfLevel = env->GetObjectArrayElement(kwMinConfLevsArr, i );
      if ( NULL == kwConfLevel )    {
         ALOGE("_registerSoundModel:  GetObjectArrayElement from KeywordConfLevel array failed");
         eStatus = LISTEN_EFAILURE;
         goto clean_up_reg_sm;
      }
      iConfLevel = (uint8_t)env->GetShortField(kwConfLevel, confLevelId);       // Check that levels are between 0 and 100
      if (iConfLevel > 100) {
         ALOGE("_registerSoundModel: ERROR: min Confidence level %d should be between 0 & 100", pConfLevels[i]);
         eStatus = LISTEN_EBAD_PARAM;
         goto clean_up_reg_sm;
      }
      // find correct keyword index for given input keyword string to place into correct array element
      strObj = (jstring)env->GetObjectField(kwConfLevel, keywordId);
      keywordStrObj = (jstring)strObj;
      // Keyword string must be when registering SVA 2.0 and above SMs
      if ( (modelInfo.version >= SM_VERSION_2p0) && (keywordStrObj == NULL) )  {
         ALOGE("_registerSoundModel: ERROR: Keyword Conf level keyword Str null");
         eStatus = LISTEN_EBAD_PARAM;
         goto clean_up_reg_sm;
      }
      if ( numKeywords > 1 ) {
         iKWStr = (char *)env->GetStringUTFChars(keywordStrObj, 0);
         eStatus = getKeywordIndex(&keywordModel, iKWStr, &index);
         // now check for error from get index
         if ( eStatus != LISTEN_SUCCESS )
         {
            ALOGE("_registerSoundModel: ERROR: given keyword string %s not found", iKWStr);
            eStatus = LISTEN_EBAD_PARAM;
            goto clean_up_reg_sm;
         } else {
            ALOGD("_registerSoundModel: keyword %s is index %d",
                    iKWStr, index);
         }
         // clean up UTF strings immediately
         if (iKWStr)
             env->ReleaseStringUTFChars(keywordStrObj, iKWStr);
         iKWStr = NULL;
      } else {
         index = 0;
      }
      pConfLevels[index] = iConfLevel;
      ALOGD("         ... confidenceLevels[%d] = %d", index, (int)iConfLevel);
   }


   // get elements from User KW Pairs and store into conf level byte array
   ALOGV("GetObjectArrayElement for userKWPairMinConfLevels w/ len=%d", numUserKWPairs );

   for (int i=0; i < numUserKWPairs; i++) {
      pairConfLevel = env->GetObjectArrayElement(userKWPairMinConfLevsArr, i );
      if ( NULL == pairConfLevel )    {
         ALOGE("_registerSoundModel:  GetObjectArrayElement from UserKeywordPairConfLevel array failed");
         eStatus = LISTEN_EFAILURE;
         goto clean_up_reg_sm;
      }

      // extract individual sound model params from jobject into struct listen_sound_model_params_t
      uKWPairClazz = env->FindClass("com/qualcomm/listen/ListenTypes$VWUUserKeywordPairConfLevel");
      if (uKWPairClazz == NULL) {
         ALOGE("_registerSoundModel: ERROR FindClass com/qualcomm/listen/ListenTypes$VWUUserKeywordPairConfLevel failed");
         eStatus = LISTEN_EFAILURE;
         goto clean_up_reg_sm;
      }
      keywordId = env->GetFieldID(uKWPairClazz, "keyword", "Ljava/lang/String;");  // jobject
      userId = env->GetFieldID(uKWPairClazz, "user", "Ljava/lang/String;");
      confLevelId = env->GetFieldID(uKWPairClazz, "confLevel", "S"); // short
      if ( (NULL == keywordId) || (NULL == userId) || (NULL == confLevelId) )    {
         ALOGE("_registerSoundModel: GetFieldId Listen VWUUserKeywordPairConfLevel class failed");
         eStatus = LISTEN_EFAILURE;
         goto clean_up_reg_sm;
      }

      iConfLevel = (uint8_t)env->GetShortField(pairConfLevel, confLevelId);
      if (iConfLevel > 100) {
          ALOGE("_registerSoundModel: ERROR: min Confidence levels should be between 0 & 100, not %d",
                 iConfLevel);
          eStatus = LISTEN_EBAD_PARAM;
          goto clean_up_reg_sm;
      }

      // convert string pairs to correct ordered pair index in array
      strObj = (jstring)env->GetObjectField(pairConfLevel, keywordId);
      keywordStrObj = (jstring)strObj;
      strObj = (jstring)env->GetObjectField(pairConfLevel, userId);
      userStrObj  = (jstring)strObj;
      // Keyword and User strings must be when registering SVA 2.0 and above SMs
      if ( (modelInfo.version >= SM_VERSION_2p0) &&
           ( (NULL == keywordStrObj) || (NULL == keywordStrObj) ) ) {
         ALOGE("_registerSoundModel: ERROR: PairConf level keyword or user Str null");
         eStatus = LISTEN_EBAD_PARAM;
         goto clean_up_reg_sm;
      }
      if ( numUserKWPairs > 1 ) {
         iKWStr = (char *)env->GetStringUTFChars(keywordStrObj, 0);
         iUserStr = (char *)env->GetStringUTFChars(userStrObj, 0);
         eStatus = getUserKeywordPairIndex(&keywordModel, iKWStr, iUserStr, &index);
         // clean up UTF strings immediately
         // now check for error from get index
         if ( eStatus != LISTEN_SUCCESS )
         {
            ALOGE("_registerSoundModel: ERROR: given keyword,user strings %s,%s not found",
                      iKWStr, iUserStr);
            eStatus = LISTEN_EBAD_PARAM;
            goto clean_up_reg_sm;
         } else {
            ALOGD("_registerSoundModel: keyword,user %s,%s is index %d",
                    iKWStr, iUserStr, index);
         }
         if (iKWStr)
             env->ReleaseStringUTFChars(keywordStrObj, iKWStr);
         if (iUserStr)
             env->ReleaseStringUTFChars(userStrObj, iUserStr);
         iKWStr = NULL;
         iUserStr = NULL;
      } else {
         index = 0;
      }
      pConfLevels[numKeywords+index] = iConfLevel;
      ALOGD("         ... confidenceLevels[%d] = %d", (numKeywords+index), (int)iConfLevel);
   }
   nativeSoundModelParams.num_keywords = (uint16_t)numKeywords;
   nativeSoundModelParams.num_user_pairings = (uint16_t)numUserKWPairs;
   nativeSoundModelParams.min_conf_levels = pConfLevels;

   // set Look Ahead Buffer parameter(s)
   if (modelInfo.version >= SM_VERSION_2p0) {
      labId = env->GetFieldID(regParamsClazz, "bufferParams",
                "Lcom/qualcomm/listen/ListenTypes$LookAheadBufferParams;");
      if ( NULL == labId ) {
         ALOGE("_registerSoundModel: get bufferParams fieldId from RegisterParams class failed");
         eStatus = LISTEN_EFAILURE;
         goto clean_up_reg_sm;
      }
      jLabObj = env->GetObjectField(registrationParams, labId);
      if (NULL == jLabObj) {
         ALOGE("_registerSoundModel: get LABParams obj from RegisterParams class failed");
         eStatus = LISTEN_EFAILURE;
         goto clean_up_reg_sm;
      }
      // extract buffer length in samples from LookAheadBuffer class
      labClazz = env->FindClass("com/qualcomm/listen/ListenTypes$LookAheadBufferParams");
      if (labClazz == NULL) {
         ALOGE("_registerSoundModel: ERROR FindClass ListenTypes$LookAheadBufferParams failed");
         eStatus = LISTEN_EFAILURE;
         goto clean_up_reg_sm;
      }
      buffEnableId = env->GetFieldID(labClazz, "enableBuffering", "Z");
      if ( NULL == buffEnableId) {
         ALOGE("_registerSoundModel: get bufferLength FieldId for LookAheadBufferParams class failed");
         eStatus = LISTEN_EFAILURE;
         goto clean_up_reg_sm;
      }
      if ( env->GetBooleanField(jLabObj, buffEnableId) )
          nativeSoundModelParams.enable_buffering = true;  // buffering enabled
      else
          nativeSoundModelParams.enable_buffering = false;  // buffering disabled
   } else {
      nativeSoundModelParams.enable_buffering = false;  // buffering disabled for SVA 1.0
   }
   if (nativeSoundModelParams.enable_buffering)
      ALOGV("_registerSoundModel: enable LAB bufferring true");
   else
      ALOGV("_registerSoundModel: enable LAB bufferring false");

   // when SVA Version 2 or above SoundModels are registered,
   //   save register parameters within VWUSession object used during event detection
   if (modelInfo.version >= SM_VERSION_2p0) {
      ALOGD("save RegSM params in mDetectionParams (%p)", &pVWUSessionReceiver->mDetectionParams);
      pVWUSessionReceiver->cleanupDetectionParams();
      pVWUSessionReceiver->mDetectionParams.detectionMode = (listen_detection_mode_enum_t)nativeSoundModelParams.detection_mode;
      pVWUSessionReceiver->mDetectionParams.numKeywords = (uint16_t)numSMKeywords;
      pVWUSessionReceiver->mDetectionParams.numUsers = (uint16_t)numSMUsers;
      pVWUSessionReceiver->mDetectionParams.numActiveKWUserPairs = (uint16_t)numSMUserKWPairs;
      // Save keywords and user Strings used when processing detection event
      // We can assume that there is always at least one keyword
      ALOGD("_registerSoundModel: getKeywordPhrases & store into VWUSessionReceiver");
      eStatus = allocStrArr(numSMKeywords, &keywordPhrases);
      if (eStatus != LISTEN_SUCCESS) {
         ALOGE("_registerSoundModel: ERROR could not alloc memory for String arrayu");
         goto clean_up_reg_sm;
      }
      status = ListenSoundModelLibV2::getKeywordPhrases(&keywordModel, &numSMKeywords, keywordPhrases);
      if (kSucess != status) {
         ALOGE("_registerSoundModel: ERROR getKeywordPhrases() failed");
         MAP_ERR(status, eStatus);
         goto clean_up_reg_sm;
      }
      pVWUSessionReceiver->mDetectionParams.keywords = (char **)calloc(numSMKeywords, sizeof(char *));
      if (NULL == pVWUSessionReceiver->mDetectionParams.keywords) {
         eStatus = LISTEN_ENO_MEMORY;
         goto clean_up_reg_sm;
      }
      for (int i=0; i < numSMKeywords; i++) {
         pVWUSessionReceiver->mDetectionParams.keywords[i] = (char *)calloc(MAX_STRING_LEN, 1);
         if (NULL == pVWUSessionReceiver->mDetectionParams.keywords[i]) {
            eStatus = LISTEN_ENO_MEMORY;
            goto clean_up_reg_sm;
         }
         // %%% memcpy(pVWUSessionReceiver->mDetectionParams.keywords[i], keywordPhrases[i], MAX_STRING_LEN );
         strCpyLen = strlcpy(pVWUSessionReceiver->mDetectionParams.keywords[i], keywordPhrases[i], MAX_STRING_LEN );
         if (MAX_STRING_LEN < strCpyLen) {
            ALOGE("_registerSoundModel: ERROR strlcpy of keyword[%d] copied more (%d)than buffer size",
                       i, (int)strCpyLen);
            eStatus = LISTEN_EFAILURE;
            goto clean_up_reg_sm;
         }
      }

      // save unpadded min confidence level array within receiver object for use during detection
      pVWUSessionReceiver->mDetectionParams.sizeConfLevels = (uint16_t)totalConfLevelSize;
      pVWUSessionReceiver->mDetectionParams.pMinConfLevels = pConfLevels;

      if (numSMUsers > 0) {
         ALOGD("_registerSoundModel: getuserNames & store into VWUSessionReceiver");
         eStatus = allocStrArr(numSMUsers, &userNames);
         if (eStatus != LISTEN_SUCCESS) {
            ALOGE("_registerSoundModel: ERROR could not alloc memory for String arrayu");
            goto clean_up_reg_sm;
         }
         status = ListenSoundModelLibV2::getUserNames(&keywordModel, &numSMUsers, userNames);
         if (kSucess != status) {
            ALOGE("_registerSoundModel: ERROR getUserNames() failed");
            MAP_ERR(status, eStatus);
            goto clean_up_reg_sm;
         }
         pVWUSessionReceiver->mDetectionParams.users = (char **)calloc(numSMUsers, sizeof(char *));
         if (NULL == pVWUSessionReceiver->mDetectionParams.users) {
            eStatus = LISTEN_ENO_MEMORY;
            goto clean_up_reg_sm;
         }
         for (int i=0; i < numSMUsers; i++) {
            pVWUSessionReceiver->mDetectionParams.users[i] = (char *)calloc(MAX_STRING_LEN, 1);
            if (NULL == pVWUSessionReceiver->mDetectionParams.users[i]) {
               eStatus = LISTEN_ENO_MEMORY;
               goto clean_up_reg_sm;
            }
            strCpyLen = strlcpy(pVWUSessionReceiver->mDetectionParams.users[i], userNames[i], MAX_STRING_LEN );
            if (MAX_STRING_LEN < strCpyLen) {
               ALOGE("_registerSoundModel: ERROR strlcpy of keyword[%d] copied more (%d)than buffer size",
                       i, (int)strCpyLen);
               eStatus = LISTEN_EFAILURE;
               goto clean_up_reg_sm;
            }
         }
         // save the truth table containing [user][keyword] array of active pairs
         pVWUSessionReceiver->mDetectionParams.pUserKeywordPairFlags =
                  (uint16_t *)malloc(sizeof(uint16_t) * numSMUsers * numSMKeywords);
         if (NULL == pVWUSessionReceiver->mDetectionParams.pUserKeywordPairFlags) {
               ALOGE("_registerSoundModel: ERROR malloc'ing pUserKeywordPairFlags failed");
               eStatus = LISTEN_ENO_MEMORY;
               goto clean_up_reg_sm;
         }
         pTmpPairPtr =  pVWUSessionReceiver->mDetectionParams.pUserKeywordPairFlags;
         for (int u=0; u < numSMUsers; u++) {
            for (int k=0; k < numSMKeywords; k++) {
                *pTmpPairPtr = modelHeader.userKeywordPairFlags[u][k];
                ALOGD("_registerSoundModel: userKeywordPairFlags[%d][%d] = %d", u, k, *pTmpPairPtr);
                pTmpPairPtr++;
            }
         }
      }
      // for SVA 2.0 register only stripped sound models
      if ( ! modelHeader.isStripped ) {
         // get size of stripped model, allocate memory for new stripped model, store new model in register params
         ALOGV("_registerSoundModel: strip SoundModel before registering");
         status = ListenSoundModelLibV2::getStrippedUserKeywordModelSize(&keywordModel, &strippedModelSize);
         if ( kSucess != status )  {
            ALOGE("_registerSoundModel: ERROR getStrippedUserKeywordModelSize() failed");
            MAP_ERR(status, eStatus);
            goto clean_up_reg_sm;
         }
         if ( (strippedModelSize >= nativeSoundModelParams.sound_model_data->size) || (0 == strippedModelSize) ) {
            ALOGE("_registerSoundModel: ERROR stripped model size %d is bad (original model size %d)",
                  strippedModelSize, nativeSoundModelParams.sound_model_data->size);
            eStatus = LISTEN_EFAILURE;
            goto clean_up_reg_sm;
         }
         // allocate memory for stripped model
         ALOGV("_registerSoundModel: strip SM size=%d; original SM size=%d",
                 strippedModelSize, nativeSoundModelParams.sound_model_data->size);
         strippedSM.size = strippedModelSize;
         strippedSM.data = (uint8_t *)malloc(strippedModelSize);
         if (NULL == strippedSM.data) {
            ALOGE("_registerSoundModel: ERROR allocating memory for stripped model failed");
            eStatus = LISTEN_ENO_MEMORY;
            goto clean_up_reg_sm;
         }
         // strip out UBM data from SM so that SM is a smaller size
         status = ListenSoundModelLibV2::stripUserKeywordModel(&keywordModel, &strippedSM);
         if (kSucess != status) {
            ALOGE("_registerSoundModel: ERROR stripUserKeywordModel() failed with %d", status);
            MAP_ERR(status, eStatus);
            goto clean_up_reg_sm;
         }
         // replace original model data ptr and size with new stripped mode info in nativeSoundModelParams
         nativeSoundModelParams.sound_model_data->size = strippedSM.size;
         if (nativeSoundModelParams.sound_model_data->p_data)
            // clean up SM malloc'ed in getSMFromByteBuff for original unstripped model
            free( (void *)(nativeSoundModelParams.sound_model_data->p_data) );
         nativeSoundModelParams.sound_model_data->p_data = strippedSM.data;
      }
   }

   eStatus = pVWUSessionReceiver->registerSoundModel(&nativeSoundModelParams);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("ERROR receiver->registerSoundModel failed with %d", (int)eStatus);
      goto clean_up_reg_sm;
   }

clean_up_reg_sm:
   if (LISTEN_SUCCESS != eStatus)
      // if registerSoundModel() failed cleanup detection parameters we set before the call
       pVWUSessionReceiver->cleanupDetectionParams();
   // clean up string used to find keyword and user indices
   if (iKWStr)
      env->ReleaseStringUTFChars(keywordStrObj, iKWStr);
   if (iUserStr)
      env->ReleaseStringUTFChars(userStrObj, iUserStr);
   // clean up SM neither malloc'ed in getSMFromByteBuff, or Stripped model
   if (nativeSoundModelParams.sound_model_data->p_data)
      free( (void *)(nativeSoundModelParams.sound_model_data->p_data) );
   freeStrArr(numSMKeywords, keywordPhrases); // silently ignore errors
   if (numSMUsers > 0)
      freeStrArr(numSMUsers, userNames); // silently ignore errors

   if (modelInfo.version >= SM_VERSION_2p0) {
      status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
      if (kSucess != status) {
         ALOGE("_getSizeAfterDelete: ERROR releaseSoundModelHeader() failed status %d", (int)status);
         MAP_ERR(status, eStatus);
      }
   }
   // pConfLevels stored in ListenVWUSession object cleaned up by ListenVWUSession destructor
   // pVWUSessionReceiver->mDetectionParams.keywords & .users string intentional kept

   ALOGV("_ListenVoiceWakeupSession_registerSoundModel returns %d", eStatus);
   return (jint)eStatus;
}

/*
 * Deregister SoundModel
 *
 * Class:     com_qualcomm_listen_ListenVoiceWakeupSession
 * Method:    deregisterSoundModel
 * Signature: ()I
 *
 * Clears SoundModel registration this session
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Return - status
 *      ESOUNDMODEL_NOT_REGISTERED
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenVoiceWakeupSession_deregisterSoundModel
  (JNIEnv *env, jobject thiz)
{
   ALOGV("Java_com_qualcomm_listen_ListenVoiceWakeupSession_deregisterSoundModel entered");
   sp<ListenReceiver> pListenReceiver = getPtrFromJObject(env, thiz);
   if (pListenReceiver == NULL) {
      ALOGE("_deregisterSoundModel: ERROR listener not set for VoiceWakeupSession object");
      return (jint)LISTEN_ECALLBACK_NOT_SET;
   }
   ListenVWUSession *pVWUSessionReceiver = (ListenVWUSession *)pListenReceiver.get();
   if (NULL == pVWUSessionReceiver) {
      ALOGE("_deregisterSoundModel: ERROR pVWUSessionReceiver NULL");
      return (jint)LISTEN_EFAILURE;
   }
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   eStatus = pVWUSessionReceiver->deregisterSoundModel();
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("ERROR receiver->deregisterSoundModel failed with %d", (int)eStatus);
   }

   ALOGV("_ListenVoiceWakeupSession_deregisterSoundModel returned");
   return (jint)eStatus;
}

/*
 * GetBuffer
 *
 * Class:     com_qualcomm_listen_ListenVoiceWakeupSession
 * Method:    getBuffer
 * Signature: (Ljava/nio/ShortBuffer;ILcom/qualcomm/listen/ListenTypes/ReadResults;)I
 *
 * Gets a buffer containing audio captured after a keyword is detected.
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Param [in]  destBuffer - buffer into which audio data will be filled
 * Param [in]  readSize - number of shorts to read before returning
 * Param [out]  readResults - which failure if any occurred during reading
 * Return - status
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenVoiceWakeupSession_getBuffer
  (JNIEnv *env, jobject thiz,
   jobject destBuffer,  //ShortBuffer
   jint readSize,       //size in shorts
   jobject readResults) //ListenTypes$ReadResults
{
	listen_status_enum_t eStatus = LISTEN_SUCCESS;
	listen_status_enum_t eReadStatus = LISTEN_SUCCESS;
	int32_t numShortsRead;
	int32_t numShortsToRead;
	int16_t* pShortArray = NULL;
	jclass readResultsClazz;
	jfieldID statusFieldId;
	jfieldID writeSizeFieldId;
	ALOGV("Java_com_qualcomm_listen_ListenVoiceWakeupSession_getBuffer entered");
	// Check for NULL jobject parameters
	if ( (NULL == destBuffer) || (NULL == readResults) ) {
	  ALOGE("_getBuffer: ERROR null ptr passed to ListenVoiceWakeupSession.getBuffer");
	  return (jint)LISTEN_EBAD_PARAM;
	}
	sp<ListenReceiver> pListenReceiver = getPtrFromJObject(env, thiz);
	if (pListenReceiver == NULL) {
	  ALOGE("_getBuffer: ERROR listener not set for VoiceWakeupSession object");
	  return (jint)LISTEN_ECALLBACK_NOT_SET;
	}
	ListenVWUSession *pVWUSessionReceiver = (ListenVWUSession *)pListenReceiver.get();
	if (NULL == pVWUSessionReceiver) {
	  ALOGE("_getBuffer: ERROR pVWUSessionReceiver NULL");
	  return (jint)LISTEN_EFAILURE;
	}

	numShortsToRead = (int32_t)readSize;
	numShortsRead = 0;

	jclass shortBuffClazz = env->FindClass("java/nio/ShortBuffer");
	if (shortBuffClazz == NULL) {
	  ALOGE("_getBuffer: FindClass java/nio/ShortBuffer failed");
	  return LISTEN_EFAILURE;
	}
	// get the ShortBuffer methods we need to call to get the short array
	jmethodID hasArrId   = env->GetMethodID(shortBuffClazz, "hasArray", "()Z");
	jmethodID getArrId   = env->GetMethodID(shortBuffClazz, "array", "()[S");
	if ( (NULL == hasArrId) || (NULL == getArrId) ) {
	  ALOGE("_getBuffer: ERROR GetMethodID for ShortBuffer failed");
	  return LISTEN_EFAILURE;
	}
	ALOGV("_getBuffer: call ShortBuffer.hasArray()...");
	bool bBuffHasArr = env->CallBooleanMethod(destBuffer, hasArrId);
	if (bBuffHasArr) {
	   ALOGV("_getBuffer: ShortBuffer.hasArray() is true");
	} else  {
	   ALOGE("_getBuffer: ERROR ... returns false - no array");
	   return LISTEN_EBAD_PARAM;
	}
	// get C++ ptr to short[] in java ShortBuffer
	ALOGV("_getBuffer: Call ShortBuffer.array()...");
	jobject jArrayObj = env->CallObjectMethod(destBuffer, getArrId);
	ALOGV("_getBuffer: cast jArrayObj = %p to shortArray", jArrayObj);
	jshortArray jShortArray = (jshortArray)jArrayObj;
	if (NULL == jArrayObj) {
	   ALOGE("_getBuffer: ERROR array returns NULL");
	   return LISTEN_EBAD_PARAM;
	}
	jsize arrayLen = env->GetArrayLength(jShortArray);
	pShortArray = (int16_t *)malloc(arrayLen*2);
	if (NULL == pShortArray) {
	   ALOGE ("_getBuffer: ERROR malloc for short array failed");
	   return LISTEN_ENO_MEMORY;
	}
	ALOGV("_getBuffer: GetArrayLength= %d", arrayLen);
	/* %%% RM after test
	env->GetShortArrayRegion(jShortArray, (jsize)0, arrayLen, (jshort *)pShortArray);
	ALOGV("_getBuffer: GetShortArrayRegion= %p", pShortArray);
	*/
	ALOGV("_getBuffer: readBuffer called with numShortsToRead %d, pShortArray %p",
        numShortsToRead, pShortArray);
	eReadStatus = pVWUSessionReceiver->readBuffer(numShortsToRead, pShortArray, &numShortsRead);
        // message for error already logged. Drop thru to process return status and size
        ALOGV("_getBuffer: readBuffer returns numShortsToRead %d",numShortsRead);

        eStatus = putAudioInShortBuffer(env, destBuffer, numShortsRead, pShortArray);
        if ( eStatus != LISTEN_SUCCESS) {
           ALOGE("_getBuffer: ERROR putSoundModelInByteBuffer() failed with %d", (int)eStatus );
           goto cleanup_get_buffer;
        }
	//Fill readResults class
	readResultsClazz = env->FindClass("com/qualcomm/listen/ListenTypes$ReadResults");
	// get IDs for the ListenTypes$ReadResults fields (status, writeSize)
	statusFieldId = env->GetFieldID(readResultsClazz, "status", "I");
	writeSizeFieldId = env->GetFieldID(readResultsClazz, "writeSize", "I");
	if (NULL == statusFieldId)
	{
           ALOGE("_getBuffer: ERROR ReadResults.status field could not be acquired");
           eStatus = LISTEN_EFAILURE;
           goto cleanup_get_buffer;
	} else if (NULL == writeSizeFieldId) {
           ALOGE("_getBuffer: ERROR ReadResults.writeSize field could not be acquired");
           eStatus = LISTEN_EFAILURE;
           goto cleanup_get_buffer;
	}

	//jint iEStatus = (jint)eStatus;
	//ALOGV("_getBuffer set eStatus %d in ReadResults", (int)iEStatus);
	ALOGV("_getBuffer: set eStatus %d to ReadResults.status", (int)eStatus);
	env->SetIntField(readResults, statusFieldId, (int)eStatus);

	ALOGV("_getBuffer: set numShortsRead %d to ReadResults.writeSize", (int)eStatus);
	env->SetIntField(readResults, writeSizeFieldId, numShortsRead);

cleanup_get_buffer:
	// If there was an error in readBuffer, that error is more important that
	// any other error so use that errorStatus as readResults' error status.
	if (eReadStatus != LISTEN_SUCCESS) {
		eStatus = eReadStatus;
	}
        if (pShortArray)
           free((void *) pShortArray);

	ALOGV("_ListenVoiceWakeupSession_getBuffer return");
	return (jint)eStatus;
}

/*
 * StopBuffering
 *
 * Class:     com_qualcomm_listen_ListenVoiceWakeupSession
 * Method:    stopBuffering
 * Signature: ()I
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Return - status
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenVoiceWakeupSession_stopBuffering
  (JNIEnv *env, jobject thiz)
{
	listen_status_enum_t eStatus = LISTEN_SUCCESS;
	ALOGV("Java_com_qualcomm_listen_ListenVoiceWakeupSession_stopBuffering entered");
	sp<ListenReceiver> pListenReceiver = getPtrFromJObject(env, thiz);
	if (pListenReceiver == NULL) {
	  ALOGE("_stopBuffering: ERROR listener not set for VoiceWakeupSession object");
	  return (jint)LISTEN_ECALLBACK_NOT_SET;
	}
	ListenVWUSession *pVWUSessionReceiver = (ListenVWUSession *)pListenReceiver.get();
	if (NULL == pVWUSessionReceiver) {
	  ALOGE("_stopBuffering: ERROR pVWUSessionReceiver NULL");
	  return (jint)LISTEN_EFAILURE;
	}
	eStatus = pVWUSessionReceiver->stopBuffering();
	if (LISTEN_SUCCESS != eStatus) {
	  ALOGE("_stopBuffering: ERROR receiver->stopBuffering failed with %d", (int)eStatus);
	}
	ALOGV("_ListenVoiceWakeupSession_stopBuffering returned");
	return (jint)eStatus;
}

#ifdef __cplusplus
}
#endif // __cplusplus

}; // namespace android
