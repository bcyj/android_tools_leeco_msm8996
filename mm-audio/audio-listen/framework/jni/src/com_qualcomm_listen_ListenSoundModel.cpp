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
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
 * Listen SoundModel
 *
 * Listen SoundModel is a collection of data that defines the
 *     characteristics of the sound that is to be detected.
 * The detection algorithm uses the registered sound model
 *     to decide if the sound it is given to evaluate matches
 *     these sound model characteristics.
 *
 * No events are generated due to calls to these SoundModel methods.
 * Sound Model methods call Listen SoundModel Library functions directly.
 *
 * includes implimentation of
 *     com_qualcomm_listen_ListenSoundModel JNI class methods
 *
 * In general all methods do the following 3 basic things:
 *    i)   extract fields from jobject input parameters and store into C++ variables
 *    ii)  call ListenSoundModelLib C++  functions
 *    iii) insert output parameters and return values from native function into
 *         jobjects
 *
 * None of these methods call Native ListenReceiver C++ methods
 *
 * Each method will check the version of the SoundModel given and decide which version
 * of the Listen SoundModel Library functions are called (e.g. SVA 1.0 or SVA 2.0).
 */
// enable verbose logs
#define LOG_NDEBUG  0
#define LOG_TAG "ListenSM-JNI"
#include <utils/Log.h>

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#include <jni.h>
#include "JNIHelp.h"  // for NELEM sizeof()/sizeof(x[0])

#ifdef LISTEN_SERVICE_USED
#include <ListenNativeTypes.h>
#endif

#include <ListenSoundModelLib_v2.h>
#include <ListenSoundModelLib.h>

#include <SoundModel.h>

#include <com_qualcomm_listen_ListenSoundModel.h>

namespace android {

#ifdef __cplusplus
extern "C" {
#endif
/*
 * verifyUserRecording
 *
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    verifyUserRecording
 * Signature: (Ljava/nio/ByteBuffer;Ljava/lang/String;Ljava/nio/ShortBuffer;)I
 *
 * Given User-Independent keyword model data this method returns the
 *     detection algorithm's confidence that the given user-specific recordings
 *     matches keyword associated with this given model data
 * Calls ListenSoundModel library function to determine how closely the recording
 *     matches given keyword contained in user-independent keyword-only SoundModel
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  clazz - java class (not an object instance) of this static method
 * Param [in]  inputModel - contains User-Independent model data
 * Param [in]  keywordPhrase - name of keyword in SM to be extended
 *             Null String can be passed if SM is SVA 1.0 format
 * Param [in]  recording  - a single recording of user speaking keyword
 *
 * Return - percent confidence level that user recording matches keyword
 *          Negative error number is returned if error occurred.
 *          LISTEN_EBAD_PARAM
 *          LISTEN_EFAILURE
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_verifyUserRecording
                 (JNIEnv *env, jclass clazz,  // ListenSoundModel
                  jobject inputModel,       // ByteBuffer
                  jstring keywordPhrase,
                  jobject rec )               // ShortBuffer
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_verifyUserRecording entered");
   listen_model_type        keywordModel;
   listen_user_recording    userRecording;
   int16_t                  confidenceLevel = 0;
   jint                     iLevel = 0;
   listen_status_enum       status = kSucess;
   listen_status_enum_t     eStatus = LISTEN_SUCCESS;
   jint                     retStatus = (jint)LISTEN_SUCCESS;
   listen_sound_model_info  modelInfo;
   keywordId_t              kwString = NULL;

   // Check for NULL jobject parameters
   if ( (NULL == inputModel) || (NULL == rec) ) {
      ALOGE("_verifyUserRecording: ERROR Null ptr passed to ListenSoundModel.verifyUserRecording");
      return LISTEN_EBAD_PARAM;
   }
   userRecording.data = NULL;
   userRecording.n_samples = 0;
   keywordModel.data = NULL;
   keywordModel.size = 0;

   // extract data and size from userIndependentData jobject
   //    and place into keywordModel struct
   eStatus = getSMFromByteBuff(env, inputModel, &keywordModel.size, &keywordModel.data, &modelInfo);
   if ( LISTEN_SUCCESS != eStatus) {
       ALOGE("_verifyUserRecording: ERROR getSMFromByteBuff returned %d", eStatus);
       goto cleanup_for_verify_rec;
   }
   ALOGV("_verifyUserRecording: user independent SM (ptr,size)=(%p,%d)",keywordModel.data,keywordModel.size);

   // KW phrase can be NULL if SVA 1.0
   if ( NULL != keywordPhrase) {
      // convert jstring keywordPhrase to char *
      kwString = (keywordId_t)env->GetStringUTFChars(keywordPhrase, 0);
   }
#ifdef SVA1_SUPPORTED
   if (modelInfo.version == SM_VERSION_1p0) {
      // SM version is 1.0 - allow a NULL keyword string param to be given
      char keywordStr[MAX_KEYWORD];
      memset(&keywordStr, 0, MAX_KEYWORD);
      ALOGD("_verifyUserRecording: ListenSoundModelLib::getKeywordStr() called");
      status = ListenSoundModelLib::getKeywordStr(&keywordModel, keywordStr);
      if (kSucess != status) {
         ALOGE("_verifyUserRecording: ERROR verifyUserRecording() failed status %d",
                (int)status);
         MAP_ERR(status, eStatus);
         goto cleanup_for_verify_rec;
      }
      if ( NULL == keywordPhrase) {
         // since keywordPhrase is NULL use Keyword stored in SM
         kwString = keywordStr;
      } else {
          // compare given keyword input param against keyword named in SM
          if (strncmp(kwString, keywordStr, strlen(keywordStr)) != 0) {
             ALOGE("_verifyUserRecording: Keyword param %s does not match Keyword %s in SM",
                   kwString, keywordStr);
             eStatus = LISTEN_EBAD_PARAM;
             goto cleanup_for_verify_rec;
         }
      }
   }
   else
#endif // SVA1_SUPPORTED
   {
      // if SM version is 2.0 or above
      //    make sure that keywordPhrase not NULL - otherwise throw error
      if ( NULL == keywordPhrase) {
          ALOGE("_verifyUserRecording: Null KeywordPhrase can not be passed for SVA 2.0 or above SoundModels");
          eStatus = LISTEN_EBAD_PARAM;
          goto cleanup_for_verify_rec;
      }
   }

   // extract data and numsamples from recording jobject & put in recording struct
   eStatus = getRecFromShortBuff(env, rec, &userRecording.n_samples, &userRecording.data);
   if ( LISTEN_SUCCESS != eStatus) {
       ALOGE("_verifyUserRecording: ERROR getSMFromShortBuff returned %d", eStatus);
       goto cleanup_for_verify_rec;
   }
   ALOGV("_verifyUserRecording: user recording (ptr,size)=(%p,%d)",userRecording.data,userRecording.n_samples);

   // Call ListenSoundModel library function to determine how closely the recording
   //    matches given keyword contained in user-independent keyword-only SoundModel
   if (modelInfo.version >= SM_VERSION_2p0) {
      ALOGV("call NEW verifyUserRecording() with %s", kwString);
      status = ListenSoundModelLibV2::verifyUserRecording(&keywordModel, kwString,
                  NULL, &userRecording, &confidenceLevel); // %% EPD parameters NOT sent
   } else {
#ifdef SVA1_SUPPORTED
      ALOGV("call old verifyUserRecording()");
      status = ListenSoundModelLib::verifyUserRecording(&keywordModel, &userRecording, &confidenceLevel);
#else
      ALOGE("SVA 1.0 SoundModels not supported");
      eStatus = LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION;
      goto cleanup_for_verify_rec;
#endif
  }
   if (kSucess != status) {
      ALOGE("_verifyUserRecording: ERROR verifyUserRecording() failed status %d", (int)status);
      MAP_ERR(status, eStatus);
      iLevel =  (jint)0;
   } else {
      iLevel = confidenceLevel;
   }
   ALOGI("_ListenSoundModel_verifyUserRecording returned confidence %d", (int)iLevel);

cleanup_for_verify_rec:
   if (keywordModel.data != NULL)
       free((void *) keywordModel.data);
   if (userRecording.data != NULL)
       free((void *) userRecording.data);
   if (kwString != NULL)
       env->ReleaseStringUTFChars(keywordPhrase, kwString);
   // return error is error occurred otherwise return confidence level
   if (LISTEN_SUCCESS != eStatus) {
      ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenExtended returns status %d", eStatus);
      return eStatus;
    } else {
      return (jint) iLevel;
    }
 }

/*
 * getSizeWhenExtended
 *
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    getSizeWhenExtended
 * Signature: (Ljava/nio/ByteBuffer;Ljava/lang/String;Ljava/lang/String;)I
 *
 * Gets the total size of bytes required to hold a SoundModel that
 *    containing both User-Independent and User-Dependent model data.
 * Application should call this and create a byte array with this return
 *    size, and use this to hold the 'combined_data' parameter
 *    when extend() method called.
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  clazz - java class (not an object instance) of this static method
 * Param [in]  inputModel - contains SoundMoodel data
 * Param [in]  keywordPhrase - keyword that user data will be added for.
 *        Can be null only if input model is SVA 1.0 format
 * Param [in]  userName - name of user training the keyword.
 *        Can be null only if input model is SVA 1.0 format
 *
 * Return - total size of unified User Keyword sound model will be when 'extended'
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenExtended
                (JNIEnv *env, jclass clazz,  // ListenSoundModel
                 jobject inputModel,       // ByteBuffer
                 jstring keywordPhrase,
                 jstring userName)
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenExtended entered");
   listen_model_type        keywordModel;
   uint32_t                 nUserKeywordModelSize = 0;
   jint                     modelSize = 0;
   listen_status_enum       status = kSucess;
   listen_status_enum_t     eStatus = LISTEN_SUCCESS;
   jint                     retStatus = (jint)LISTEN_SUCCESS;
   listen_sound_model_info  modelInfo;
   keywordId_t              kwString = NULL;
   userId_t                 userString = NULL;
   listen_sound_model_header  modelHeader;

   // Check for NULL jobject parameter
   if ( NULL == inputModel ) {
      ALOGE("_getSizeWhenExtended: ERROR Null ptr passed to ListenSoundModel.getSizeWhenExtended");
      return LISTEN_EBAD_PARAM;
   }

   keywordModel.data = NULL;
   keywordModel.size = 0;
   // extract data and size from userIndependentData jobject
   //    and place into keywordModel struct
   eStatus = getSMFromByteBuff(env, inputModel, &keywordModel.size, &keywordModel.data, &modelInfo);
   if ( LISTEN_SUCCESS != eStatus) {
       ALOGE("_getSizeWhenExtended: ERROR getSMFromByteBuff returned %d", eStatus);
       goto cleanup_for_get_size_when_extended;
   }
   ALOGV("_getSizeWhenExtended: user independent SM (ptr,size)=(%p,%d)", keywordModel.data, keywordModel.size);

   // Call ListenSoundModel library function to calculate the size of an extend
   //    User-dependent SoundModel appended to given keyword-only SoundModel
   if (modelInfo.version >= SM_VERSION_2p0) {
      memset(&modelHeader, 0, sizeof(listen_sound_model_header));
      status = ListenSoundModelLibV2::getSoundModelHeader(&keywordModel, &modelHeader);
      if (kSucess != status) {
          ALOGE("_getSizeWhenExtended: ERROR getSoundModelHeader() failed status %d",
              (int)status);
          MAP_ERR(status, eStatus);
          goto cleanup_for_get_size_when_extended;
      }

      // Stripped models can not be trained
      if ( modelHeader.isStripped ) {
          ALOGE("_getSizeWhenExtended: ERROR stripped sound model can not be trained");
          eStatus = LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION;
          goto cleanup_for_get_size_when_extended;
      } else {
          ALOGV("_getSizeWhenExtended: Model is NOT stripped");
      }

      // Neither keywordPhrase nor userName can be null if SVA 2.0 SoundModel being extended
      if ( (NULL == keywordPhrase) || (NULL == userName) ) {
         ALOGE("_getSizeWhenExtended: ERROR both keyword and user params must be non-null");
         eStatus = LISTEN_EBAD_PARAM;
         goto cleanup_for_get_size_when_extended;
      }
      // convert jstring keywordPhrase to char *
      kwString = (keywordId_t)env->GetStringUTFChars(keywordPhrase, 0);
      userString = (keywordId_t)env->GetStringUTFChars(userName, 0);
      ALOGV("call NEW getUserKeywordModelSize() for %s, %s", kwString, userString);
      status = ListenSoundModelLibV2::getUserKeywordModelSize(&keywordModel,
                kwString, userString, &nUserKeywordModelSize);
   } else {
#ifdef SVA1_SUPPORTED
      ALOGV("call old getUserKeywordModelSize() ");
      status = ListenSoundModelLib::getUserKeywordModelSize(&keywordModel, &nUserKeywordModelSize);
#else
      ALOGE("SVA 1.0 SoundModels not supported");
      eStatus = LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION;
      goto cleanup_for_get_size_when_extended;
#endif
   }
   if (kSucess != status) {
       ALOGE("_getSizeWhenExtended: ERROR getUserKeywordModelSize() failed status %d", (int)status);
       MAP_ERR(status, eStatus);
   } else {
       modelSize = (jint)nUserKeywordModelSize;
   }
   ALOGI("_ListenSoundModel_getSizeWhenExtended returns %d", (int)modelSize);

cleanup_for_get_size_when_extended:
   if (keywordModel.data != NULL)
       free((void *) keywordModel.data);
   if (kwString != NULL)
       env->ReleaseStringUTFChars(keywordPhrase, kwString);
   if (userString != NULL)
       env->ReleaseStringUTFChars(userName, userString);
   if (modelInfo.version >= SM_VERSION_2p0) {
      status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
      if (kSucess != status) {
         ALOGE("_getSizeAfterDelete: ERROR releaseSoundModelHeader() failed status %d", (int)status);
         MAP_ERR(status, eStatus);
      }
   }
   // return error is error occurred otherwise return modelsize
   if (LISTEN_SUCCESS != eStatus) {
      ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenExtended returns status %d", eStatus);
      return eStatus;
   } else {
      ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenExtended returns size %d", modelSize);
      return modelSize;
   }
}

/*
 * extend
 *
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    extend
 * Signature: (Ljava/nio/ByteBuffer;Ljava/lang/String;Ljava/lang/String;
 *             I[Ljava/nio/ShortBuffer;Ljava/nio/ByteBuffer;
 *             Lcom/qualcomm/listen/ListenTypes/ConfidenceData;)I
 *
 * Extends given User-Independent keyword model data by combining it with
 *    User-Dependent keyword model data created from user recordings
 *    into a single SoundModel
 *
 * The sound model is copied into a memory block pointed to combined_data parameter.
 * Application is responsible for creating a byte array large enough to hold this data
 * Size that this returned SoundModel can be queried using getSizeWhenExtended().
 * At least 5 user recordings should be passed to this method.
 * The more user recordings passed as input, the greater the likelihood
 *     of getting a higher quality SoundModel made.
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  clazz - java class (not an object instance) of this static method
 * Param [in]  inputModel - contains user independent Keyword specific model data
 * Param [in]  keywordPhrase - name of keyword in SM to be extended
 *             Can be null only if SM is SVA 1.0 format
 * Param [in]  userName - name of user created these trainging recordings
 *             Can be null only if SM is SVA 1.0 format
 * Param [in]  numRecs - number of recordings of a user speaking the keyword
 * Param [in]  recs  - array of N user recordings
 * Param [out] combinedData  - sound model containing user-independent and user-dependent data
 *             It is assumed that the user has created an ByteByffer for 'combinedData'
 *             whose capacity is large enough to hold byte buffer of copied soundmodel
 * Param [out] qualityData  - structure containing 'quality' level of created SoundModel
 *
 * Return - status
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_extend
                (JNIEnv *env, jclass clazz,  // ListenSoundModel
                 jobject inputModel,       // ByteBuffer
                 jstring keywordPhrase,
                 jstring userName,
                 jint numRecs,
                 jobjectArray recs,          // array of ShortBuffers
                 jobject combinedData,       // ByteBuffer
                 jobject qualityData)        // ListenTypes$ConfidenceData
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_extend entered");
   listen_model_type           keywordModel;
   listen_user_recording **    userRecsArray = NULL;   // array of ptrs
   listen_model_type           outputModel;
   int16_t                     userMatchingScore = 0 ;
   listen_status_enum          status = kSucess;
   jint                        retStatus = (jint)LISTEN_SUCCESS;
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   uint32_t                    nRecs = (uint32_t)numRecs;
   uint32_t                    smMax = 0;
   jclass                      byteBuffClazz;
   jmethodID                   capacityId;
   int                         capacity = 0;
   jclass                      shortBufferClazz;
   jmethodID                   putId;
   jsize                       smLen = 0;
   jclass                      confidenceDataClazz;
   jfieldID                    userMatchId;
   jint                        iUserMatch = 0;
   listen_sound_model_info     modelInfo;
   keywordId_t                 kwString = NULL;
   userId_t                    userString = NULL;
   listen_sound_model_header   modelHeader;

   // Check for NULL jobject parameters
   if ( (NULL == inputModel) || (NULL == recs) ||
        (NULL == combinedData) ||  (NULL == qualityData) ) {
      ALOGE("_extend: ERROR Null ptr passed to ListenSoundModel.extend");
      return LISTEN_EBAD_PARAM;
   }

   keywordModel.data = NULL;
   keywordModel.size = 0;
   outputModel.data = NULL;
   outputModel.size = 0;
   // extract data and size from userIndependentData jobject & put in keywordModel struct
   eStatus = getSMFromByteBuff(env, inputModel, &keywordModel.size, &keywordModel.data, &modelInfo);
   if ( LISTEN_SUCCESS != eStatus) {
       ALOGE("_extend: ERROR getSMFromByteBuff returned %d", eStatus);
       retStatus = eStatus;
       goto cleanup_for_extend;
   }

   ALOGV("_extend: user independent SM (ptr,size)=(%p,%d)", keywordModel.data, keywordModel.size);
   ALOGV("_extend: ref to Java user SM %p", combinedData);

   //
   // should check capacity of combinedData byteBuffer before length before doing Put !!!!!
   //
   if (modelInfo.version >= SM_VERSION_2p0) {
      memset(&modelHeader, 0, sizeof(listen_sound_model_header));
      status = ListenSoundModelLibV2::getSoundModelHeader(&keywordModel, &modelHeader);
      if (kSucess != status) {
         ALOGE("_extend: ERROR getSoundModelHeader() failed status %d",
              (int)status);
         MAP_ERR(status, eStatus);
         goto cleanup_for_extend;
      }

      // Stripped models can not be trained
      if ( modelHeader.isStripped ) {
         ALOGE("_extend: ERROR stripped sound model can not be trained");
         eStatus = LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION;
         goto cleanup_for_extend;
      } else {
        ALOGV("_extend: Model is NOT stripped");
      }
      // Neither keywordPhrase nor userName can be null if SVA 2.0 SoundModel being extended
      if ( (NULL == keywordPhrase) || (NULL == userName) ) {
          ALOGE("_extend: ERROR both keyword and user params must be non-null");
          eStatus = LISTEN_EBAD_PARAM;
          goto cleanup_for_extend;
      }

      // convert jstring keywordPhrase to char *
      kwString = (keywordId_t)env->GetStringUTFChars(keywordPhrase, 0);
      userString = (keywordId_t)env->GetStringUTFChars(userName, 0);
      ALOGV("_extend: call NEW getSize() for %s, %s", kwString, userString);
      status = ListenSoundModelLibV2::getUserKeywordModelSize(&keywordModel,
                kwString, userString, &smMax);
   } else {
#ifdef SVA1_SUPPORTED
      status = ListenSoundModelLib::getUserKeywordModelSize(&keywordModel, &smMax);
#else
      ALOGE("SVA 1.0 SoundModels not supported");
      eStatus = LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION;
      goto cleanup_for_extend;
#endif
   }
   if (kSucess != status) {
       ALOGE("_extend: ERROR getUserKeywordModelSize() failed status %d", (int)status);
       MAP_ERR(status, eStatus);
       goto cleanup_for_extend;
   }

   // verify the combinedData ByteBuffer is big enough
   eStatus = checkByteBufferCapacity(env, smMax, combinedData);
   if ( eStatus != LISTEN_SUCCESS) {
      retStatus = LISTEN_EBAD_PARAM;
      goto cleanup_for_extend;
   }

   // extract data and size from each recording in ShortBuffer jobject array
   //    malloc array of listen_user_recording userRecsArray[]
   //    then place into i-th element of userRecordings struct array
   userRecsArray = (listen_user_recording **)malloc((int)(nRecs * (sizeof(listen_user_recording *))));
   if (NULL == userRecsArray) {
      ALOGE("_extend: ERROR malloc for listen_user_recording[] failed");
      retStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_extend;
   }

   // check if recs is an instance of ShortBuffer[]
   ALOGV("FindClass of ShortBuffer[] as [Ljava/nio/ShortBuffer;");
   shortBufferClazz = env->FindClass("[Ljava/nio/ShortBuffer;");
   if (NULL == shortBufferClazz) {
      ALOGE("_extend: ERROR FindClass([java/nio/ShortBuffer) failed");
      retStatus = LISTEN_EFAILURE;
      goto cleanup_for_extend;
   }
   for (uint32_t i=0; i < nRecs; i++) {
       // get the i-th jobject from recs param
       ALOGV("recording[%d] ptr %p", i, userRecsArray[i]);
       userRecsArray[i] = (listen_user_recording *)malloc((int)(sizeof(listen_user_recording)));
       if (NULL == userRecsArray[i]) {
          ALOGE("_extend: ERROR malloc for listen_user_recording failed");
          retStatus = LISTEN_ENO_MEMORY;
          goto cleanup_for_extend;
       }
       ALOGV("call IsInstanceOf(recs, shortBufferClazz)");
       if( !(env->IsInstanceOf(recs, shortBufferClazz))){
          ALOGE("_extend: ERROR recs is not an ShortBuffer[] instance");
          retStatus = LISTEN_EBAD_PARAM;
          goto cleanup_for_extend;
       }

       ALOGV("call GetObjectArrayElement for element %d of array ptr %p", i, recs );
       jobject userRecObj =  env->GetObjectArrayElement(recs, i); // get i-th jobject recording
       if (NULL == userRecObj) {
          ALOGE("_extend: ERROR GetObjectArrayElement of recs failed");
          retStatus = LISTEN_EBAD_PARAM;
          goto cleanup_for_extend;
       }

       // copy contents of byteArray into byte array
       // extract data and numsamples from recording jobject & put in recording struct
       eStatus = getRecFromShortBuff(env, userRecObj, &userRecsArray[i]->n_samples, &userRecsArray[i]->data);
       if ( LISTEN_SUCCESS != eStatus) {
           ALOGE("_extend: ERROR getRecFromShortBuff returned %d", eStatus);
           retStatus = LISTEN_EFAILURE;
           goto cleanup_for_extend;
       }
       ALOGV("user recording[%d] (ptr,size)=(%p,%d)",
          i, userRecsArray[i]->data, userRecsArray[i]->n_samples);
   }

   // allocate
   ALOGV("Allocate memory for output parameter outputModel, max size %d", smMax);
   outputModel.size = smMax;
   outputModel.data = (uint8_t *)malloc(smMax);
   if (NULL == outputModel.data) {
      ALOGE("ERROR malloc'ing outputModel data failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_extend;
   }
   ALOGV("ptr to userKeyword Model memory is %p", outputModel.data);

   if (modelInfo.version >= SM_VERSION_2p0) {
      ALOGV("_extend: call NEW createUserKeywordModel() for %s, %s", kwString, userString);
      status = ListenSoundModelLibV2::createUserKeywordModel(&keywordModel,
               kwString, userString,
               NULL, // EPD parameters are left as default
               nRecs, userRecsArray,
               &outputModel, &userMatchingScore);
   } else {
#ifdef SVA1_SUPPORTED
      ALOGV("_extend: call old createUserKeywordModel()");
      status = ListenSoundModelLib::createUserKeywordModel(&keywordModel,
               (int32_t)nRecs, userRecsArray,
               &outputModel, &userMatchingScore);
#else
      ALOGE("SVA 1.0 SoundModels not supported");
      eStatus = LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION;
      goto cleanup_for_extend;
#endif
   }
   if (kSucess != status) {
      ALOGE("_extend: ERROR createUserKeywordModel() failed status %d", (int)status);
      MAP_ERR(status, retStatus);
      goto cleanup_for_extend;
      // don't return; drop down to clean up memory
   } else {
      ALOGV("createUserKeywordModel output userMatchingScore = %d", (int)userMatchingScore);
      eStatus = putSoundModelInByteBuffer(env, combinedData, outputModel.size, outputModel.data);
      if ( eStatus != LISTEN_SUCCESS) {
         ALOGE("_extend: ERROR putSoundModelInByteBuffer() failed with %d", (int)eStatus );
         retStatus = eStatus;
         goto cleanup_for_extend;
      }

      // Fill return confidence structure field into ListenTypes$ConfidenceData that is output
      confidenceDataClazz = env->FindClass("com/qualcomm/listen/ListenTypes$ConfidenceData");
      if (NULL == confidenceDataClazz) {
         ALOGE("_extend: ERROR confidenceDataClazz class not found");
         retStatus = LISTEN_EFAILURE;
         goto cleanup_for_extend;
      }
      // get ID of the field ListenTypes$ConfidenceData
      userMatchId = env->GetFieldID(confidenceDataClazz, "userMatch", "I");
      if ( NULL==userMatchId )
      {
         ALOGE("userMatch from DetectionData class not acquired");
         retStatus = LISTEN_EFAILURE;
         goto cleanup_for_extend;
      }

      // place user match score into qualityData output parameter object
      iUserMatch = (jint)userMatchingScore;  // convert short to jint
      ALOGV("createUserKeywordModel set matchingScore %d in to qualityData", (int)iUserMatch);
      env->SetIntField(qualityData, userMatchId, userMatchingScore);

      ALOGV("createUserKeywordModel() returns newSM(data,size)= (%p,%d), score %d",
               outputModel.data, outputModel.size, userMatchingScore);
   }

cleanup_for_extend:
   // free userRecsArray array
   if (userRecsArray) {
      for (uint32_t i=0; i < nRecs; i++) {
         if (userRecsArray[i]) {
             if (userRecsArray[i]->data != NULL)
                free((void *) userRecsArray[i]->data);
             free((void *)userRecsArray[i]);
         }
      }
      free((void *)userRecsArray);
   }
   // clean up keyword model data structures
   if (keywordModel.data != NULL)
       free((void *) keywordModel.data);
   if (outputModel.data != NULL)
      free((void *)outputModel.data);
   if (kwString != NULL)
       env->ReleaseStringUTFChars(keywordPhrase, kwString);
   if (userString != NULL)
       env->ReleaseStringUTFChars(userName, userString);

   if (modelInfo.version >= SM_VERSION_2p0) {
      status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
      if (kSucess != status) {
         ALOGE("_getSizeAfterDelete: ERROR releaseSoundModelHeader() failed status %d", (int)status);
         MAP_ERR(status, eStatus);
      }
   }
   ALOGV("_ListenSoundModel_extend returns %d", retStatus);

   return retStatus;
}

/*
 * verifyUdkRecording
 *
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    verifyUdkRecording
 * Signature: (Ljava/nio/ByteBuffer;Ljava/nio/ShortBuffer;)I
 *
 * Returns the detection algorithm's confidence that the given user recording
 * is viable for creating a user-defined keyphrase sound model.
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  clazz - java class (not an object instance) of this static method
 * Param [in]  jLanguageModel - contains language data for a specific language
 * Param [in]  jUserRecording  - a single recording of user speaking keyword
 *
 * Return - percent confidence level that user recording matches keyword
 *          Negative error number is returned if error occurred.
 *          LISTEN_EBAD_PARAM
 *          LISTEN_ENO_SPEACH_IN_RECORDING
 *          LISTEN_ETOO_MUCH_NOISE_IN_RECORDING
 *          LISTEN_EFAILURE
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_verifyUdkRecording
    (JNIEnv *env, jclass clazz,
            jobject jLanguageModel, //byte buffer
            jobject jUserRecording) //short buffer
{
    listen_language_model_type      languageModel;
    listen_user_recording           userRecording;
    float                           snr;
    jint                            jSnr = 0;
    listen_status_enum              status = kSucess;
    listen_status_enum_t            eStatus = LISTEN_EFAILURE;
    jint                            retStatus = (jint)LISTEN_SUCCESS;

    // Check for NULL jobject parameters
    if ( (NULL == jLanguageModel) || (NULL == jUserRecording) ) {
      ALOGE("_verifyUdkRecording: ERROR Null ptr passed to ListenSoundModel.verifyUdkRecording");
      return LISTEN_EBAD_PARAM;
    }

    ALOGV("_verifyUdkRecording entered");

    languageModel.data = NULL;
    languageModel.size = 0;
    userRecording.data = NULL;
    userRecording.n_samples = 0;

    // Convert jLanguageModel to listen_language_model_type by extracting the byte array from the
    // ByteBuffer jObject.
    eStatus = getLanguageModelFromByteBuff(env, jLanguageModel, &languageModel.size, &languageModel.data);
    if ( LISTEN_SUCCESS != eStatus) {
       ALOGE("_verifyUdkRecording: ERROR getLanguageModelFromByteBuff returned %d", eStatus);
       goto cleanup_for_verify_rec;
    }
    ALOGV("_verifyUdkRecording: user independent SM (ptr,size)=(%p,%d)",
            languageModel.data, languageModel.size);

    // Extract data and numsamples from recording jobject & put in userRecording struct.
    eStatus = getRecFromShortBuff(env, jUserRecording, &userRecording.n_samples, &userRecording.data);
    if ( LISTEN_SUCCESS != eStatus) {
       ALOGE("_verifyUdkRecording: ERROR getSMFromShortBuff returned %d", eStatus);
       goto cleanup_for_verify_rec;
    }
    ALOGV("_verifyUdkRecording: user recording (ptr,size)=(%p,%d)",
           userRecording.data, userRecording.n_samples);

    // Call SML function equivalent checkUserRecording
    ALOGV("call ListenSoundModelLibV2::checkUserRecording");
    status = ListenSoundModelLibV2::checkUserRecording(&languageModel, NULL,
            &userRecording, &snr); // Passing null for EPD uses default EPD params.
    if (kSucess != status) {
        ALOGE("_verifyUdkRecording: ERROR verifyUserRecording() failed status %d", (int)status);
        MAP_ERR(status, eStatus);
    } else {
        jSnr = (jint)snr;
        ALOGI("_ListenSoundModel_verifyUserRecording returned snr %d", (int)jSnr);
    }

cleanup_for_verify_rec:
    if (languageModel.data != NULL)
       free((void *) languageModel.data);
    if (userRecording.data != NULL)
        free((void *) userRecording.data);
    // return error is error occurred otherwise return confidence level
    if (LISTEN_SUCCESS != eStatus) {
        ALOGV("_ListenSoundModel_verifyUdkRecording returns status %d", eStatus);
        return eStatus;
    } else {
        return (jint) jSnr;
    }
}

/*
 * getUdkSmSize
 *
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    getUdkSmSize
 * Signature: (Ljava/lang/String;Ljava/lang/String;[[Ljava/nio/ShortBuffer;Ljava/nio/ByteBuffer;)I
 *
 * Get the total size of bytes required to hold a UDK sound model.
 * The app should call this, create a byte array with the size equal to the
 *    value of this function's return value, and use this byte array to hold
 *    the 'combined_data' parameter when the createUdkSm() method is called.
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  clazz - ptr to java class (not an object instance) of this static method
 * Param [in]  jKeyphrase - keyphrase that user data will be added for.
 * Param [in]  jUsername - name of user training the keyphrase.
 * Param [in]  jUserRecording - an array of verified recording of user speaking the UDK.
 * Param [in]  jLanguageModel - contains language data for a specific language.
 *
 * Return - the total size the UDK sound model will be when 'created'
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_getUdkSmSize
    (JNIEnv *env, jclass clazz,
          jstring jKeyphrase,
          jstring jUsername,
          jobjectArray jUserRecordings, //ShortBuffer
          jobject jLanguageModel) // ByteBuffer
{
    ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getUdkSmSize entered");

    listen_language_model_type  languageModel;
    listen_user_recording       userRecording;
    uint32_t                    nUserKeywordModelSize = -1;
    uint32_t                    numberUdkRecordings = 5; //QUESTION how should this be obtained programmatically, pass in from java API
    jint                        modelSize = 0;
    listen_status_enum          status = kSucess;
    listen_status_enum_t        eStatus = LISTEN_EFAILURE;
    jint                        retStatus = (jint)LISTEN_EFAILURE;
    keywordId_t                 kwString = NULL;
    userId_t                    userString = NULL;
    listen_user_recording **    userRecsArray = NULL;   // array of ptrs
    jclass                      shortBufferArrayClazz;

    // Check for NULL jobject parameter
    if (NULL == jLanguageModel || NULL == jUserRecordings) {
        ALOGE("_getUdkSmSize: ERROR Null ptr passed to ListenSoundModel.getSizeWhenExtended");
        return LISTEN_EBAD_PARAM;
    }
    // Neither jKeyphrase nor jUsername can be null if SVA 2.0 SoundModel being extended
    if ((NULL == jKeyphrase) || (NULL == jUsername)) {
        ALOGE("_getUdkSmSize: ERROR both keyword and user params must be non-null");
        eStatus = LISTEN_EBAD_PARAM;
        goto cleanup_for_get_udk_sm_size;
    }

    languageModel.data = NULL;
    languageModel.size = 0;
    userRecording.data = NULL;
    userRecording.n_samples = 0;

    // Convert jLanguageModel to listen_language_model_type by extracting the byte array from the
    // ByteBuffer jObject.
    eStatus = getLanguageModelFromByteBuff(env, jLanguageModel, &languageModel.size, &languageModel.data);
    if (LISTEN_SUCCESS != eStatus) {
        ALOGE("_getUdkSmSize: ERROR getLanguageModelFromByteBuff returned %d", eStatus);
        goto cleanup_for_get_udk_sm_size;
    }
    ALOGV("_getUdkSmSize: user independent SM (ptr,size)=(%p,%d)",
            languageModel.data, languageModel.size);

    // convert jstring keywordPhrase to char *
    kwString = (keywordId_t)env->GetStringUTFChars(jKeyphrase, 0);
    userString = (keywordId_t)env->GetStringUTFChars(jUsername, 0);
    ALOGV("_getUdkSmSize: call NEW getUserKeywordModelSize() for %s, %s", kwString, userString);

    // Can be optimized.
    // Extract data and size from each recording in ShortBuffer jobject array
    //    malloc array of listen_user_recording userRecsArray[]
    //    then place into i-th element of userRecordings struct array
    userRecsArray = (listen_user_recording **)malloc((int)(numberUdkRecordings * (sizeof(listen_user_recording *))));
    if (NULL == userRecsArray) {
      ALOGE("_getUdkSmSize: ERROR malloc for listen_user_recording[] failed");
      retStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_get_udk_sm_size;
    }

    // check if recs is an instance of ShortBuffer[]
    ALOGV("_getUdkSmSize: FindClass of ShortBuffer[] as [Ljava/nio/ShortBuffer;");
    shortBufferArrayClazz = env->FindClass("[Ljava/nio/ShortBuffer;");
    if (NULL == shortBufferArrayClazz) {
      ALOGE("_getUdkSmSize: ERROR FindClass([java/nio/ShortBuffer) failed");
      retStatus = LISTEN_EFAILURE;
      goto cleanup_for_get_udk_sm_size;
    }
    ALOGV("call IsInstanceOf(recs, shortBufferArrayClazz)");
    if( !(env->IsInstanceOf(jUserRecordings, shortBufferArrayClazz))){
        ALOGE("_getUdkSmSize: ERROR recs is not an ShortBuffer[] instance");
        retStatus = LISTEN_EBAD_PARAM;
        goto cleanup_for_get_udk_sm_size;
    }

    for (uint32_t i=0; i < numberUdkRecordings; i++) {
        // get the i-th jobject from recs param
        ALOGV("_getUdkSmSize: recording[%d] ptr %p", i, userRecsArray[i]);
        userRecsArray[i] = (listen_user_recording *)malloc((int)(sizeof(listen_user_recording)));
        if (NULL == userRecsArray[i]) {
            ALOGE("_getUdkSmSize: ERROR malloc for listen_user_recording failed");
            retStatus = LISTEN_ENO_MEMORY;
            goto cleanup_for_get_udk_sm_size;
        }

        ALOGV("call GetObjectArrayElement for element %d of array ptr %p", i, jUserRecordings);
        jobject userRecObj =  env->GetObjectArrayElement(jUserRecordings, i); // get i-th jobject recording
        if (NULL == userRecObj) {
            ALOGE("_getUdkSmSize: ERROR GetObjectArrayElement of recs failed");
            retStatus = LISTEN_EBAD_PARAM;
            goto cleanup_for_get_udk_sm_size;
        }

        // copy contents of byteArray into byte array
        // extract data and numsamples from recording jobject & put in recording struct
        eStatus = getRecFromShortBuff(env, userRecObj, &userRecsArray[i]->n_samples, &userRecsArray[i]->data);
        if ( LISTEN_SUCCESS != eStatus) {
            ALOGE("_getUdkSmSize: ERROR getRecFromShortBuff returned %d", eStatus);
            retStatus = LISTEN_EFAILURE;
            goto cleanup_for_get_udk_sm_size;
        }
        ALOGV("_getUdkSmSize: user recording[%d] (ptr,size)=(%p,%d)",
                i, userRecsArray[i]->data, userRecsArray[i]->n_samples);
    }

    // Not passing pUserDefinedKeyword, nor EPD params.
    status = ListenSoundModelLibV2::getUserDefinedKeywordSize(NULL, kwString, userString,
          NULL, numberUdkRecordings, userRecsArray, &languageModel, &nUserKeywordModelSize);
    if (kSucess != status) {
       ALOGE("_getUdkSmSize: ERROR getUserKeywordModelSize() failed status %d", (int)status);
       MAP_ERR(status, eStatus);
    } else {
       modelSize = (jint)nUserKeywordModelSize;
    }

cleanup_for_get_udk_sm_size:
   if (languageModel.data != NULL)
       free((void *) languageModel.data);
   // free userRecsArray array
   if (userRecsArray) {
       for (uint32_t i=0; i < numberUdkRecordings; i++) {
           if (userRecsArray[i]) {
               if (userRecsArray[i]->data != NULL)
                   free((void *) userRecsArray[i]->data);
               free((void *)userRecsArray[i]);
           }
       }
       free((void *)userRecsArray);
   }
   if (kwString != NULL)
       env->ReleaseStringUTFChars(jKeyphrase, kwString);
   if (userString != NULL)
       env->ReleaseStringUTFChars(jUsername, userString);
   // return error is error occurred otherwise return modelsize
   if (LISTEN_SUCCESS != eStatus) {
      ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenExtended returns status %d", eStatus);
      return eStatus;
   } else {
      ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenExtended returns size %d", modelSize);
      return modelSize;
   }
}

/*
 * createUdkSm
 *
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    createUdkSm
 * Signature: (Ljava/lang/String;
 *             Ljava/lang/String;
 *             I[[Ljava/nio/ShortBuffer;
 *             Ljava/nio/ByteBuffer;
 *             Ljava/nio/ByteBuffer;
 *             Lcom/qualcomm/listen/ListenTypes/ConfidenceData;)I
 *
 * Creates a user-defined keyphrase sound model.
 * The sound model data is copied into a memory block pointed to combined_data
 * parameter.
 *
 * The app is responsible for creating a byte array large enough to hold this data
 * Size that this returned SoundModel can be queried using getUdkSmSize().
 * At least 5 user recordings should be passed to this method.
 * The more user recordings passed as input, the greater the likelihood
 *     of getting a higher quality SoundModel made.
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  clazz - java class (not an object instance) of this static method
 * Param [in]  jKeyphrase - name of keyword as input by the user
 * Param [in]  jUsername - name of user that created the training recordings
 * Param [in]  jNumberUserRecordings - number of recordings of a user speaking the keyphrase (N)
 * Param [in]  jUserRecordings  - array of N user recordings
 * Param [in]  jLanguageModel - contains language-specific data for the UDK keyphrase
 * Param [out] jCreatedUdkSm  - sound model containing the UDK data
 *             It is assumed that the app has created a byte array for this parameter
 *             whose capacity is large enough to hold the paramter.
 * Param [out] jConfidenceData  - structure containing the quality level of the created UDK SM
 *
 * Return - status
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_createUdkSm
    (JNIEnv *env, jclass clazz,
          jstring        jKeyphrase,
          jstring        jUsername,
          jint           jNumberUserRecordings,
          jobjectArray   jUserRecordings, //ShortBuffer Array
          jobject        jLanguageModel, // ByteBuffer
          jobject        jCreatedUdkSm, // ByteBuffer
          jobject        jConfidenceData) // ListenTypes$ConfidenceData
{
    ALOGV("Java_com_qualcomm_listen_ListenSoundModel_createUdkSm entered");
    listen_language_model_type  languageModel;
    listen_user_recording **    userRecsArray = NULL;   // array of ptrs
    listen_model_type           outputModel;
    int16_t                     userMatchingScore = 0;
    listen_status_enum          status = kSucess;
    jint                        retStatus = (jint)LISTEN_SUCCESS;
    listen_status_enum_t        eStatus = LISTEN_SUCCESS;
    uint32_t                    nRecs = (uint32_t)jNumberUserRecordings;
    uint32_t                    smMax = 0;
    jclass                      shortBufferArrayClazz;
    jclass                      confidenceDataClazz;
    jfieldID                    userMatchId;
    jint                        jUserMatchScore = 0;
    keywordId_t                 kwString = NULL;
    userId_t                    userString = NULL;

    // Check for NULL jobject parameters
    if ((NULL == jUserRecordings) || (NULL == jCreatedUdkSm) || (NULL == jLanguageModel) ||
            (NULL == jConfidenceData) ) {
      ALOGE("_createUdkSm: ERROR Null ptr passed");
      return LISTEN_EBAD_PARAM;
    }
    // Neither keywordPhrase nor userName can be null
    if ((NULL == jKeyphrase) || (NULL == jUsername)) {
      ALOGE("_createUdkSm: ERROR both keyword and user params must be non-null");
      return LISTEN_EBAD_PARAM;
    }

    languageModel.data = NULL;
    languageModel.size = 0;
    outputModel.data = NULL;
    outputModel.size = 0;

    // Convert jLanguageModel to listen_language_model_type by extracting the byte array from the
    // ByteBuffer jObject.
    eStatus = getLanguageModelFromByteBuff(env, jLanguageModel, &languageModel.size, &languageModel.data);
        if (LISTEN_SUCCESS != eStatus) {
           ALOGE("_getUdkSmSize: ERROR getLanguageModelFromByteBuff returned %d", eStatus);
           goto cleanup_for_create_udk_sm;
        }
    ALOGV("_createUdkSm: user independent SM (ptr,size)=(%p,%d)",
            languageModel.data, languageModel.size);
    ALOGV("_createUdkSm: ref to Java user SM %p", jCreatedUdkSm);

    // extract data and size from each recording in ShortBuffer jobject array
    //    malloc array of listen_user_recording userRecsArray[]
    //    then place into i-th element of userRecordings struct array
    userRecsArray = (listen_user_recording **)malloc((int)(nRecs * (sizeof(listen_user_recording *))));
    if (NULL == userRecsArray) {
      ALOGE("_createUdkSm: ERROR malloc for listen_user_recording[] failed");
      retStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_create_udk_sm;
    }

    // check if recs is an instance of ShortBuffer[]
    ALOGV("_createUdkSm: FindClass of ShortBuffer[] as [Ljava/nio/ShortBuffer;");
    shortBufferArrayClazz = env->FindClass("[Ljava/nio/ShortBuffer;");
    if (NULL == shortBufferArrayClazz) {
      ALOGE("_createUdkSm: ERROR FindClass([java/nio/ShortBuffer) failed");
      retStatus = LISTEN_EFAILURE;
      goto cleanup_for_create_udk_sm;
    }
    ALOGV("_createUdkSm: call IsInstanceOf(jUserRecordings, shortBufferArrayClazz)");
    if( !(env->IsInstanceOf(jUserRecordings, shortBufferArrayClazz))){
       ALOGE("_createUdkSm: ERROR recs is not an ShortBuffer[] instance");
       retStatus = LISTEN_EBAD_PARAM;
       goto cleanup_for_create_udk_sm;
    }

    for (uint32_t i=0; i < nRecs; i++) {
       // get the i-th jobject from recs param
       ALOGV("_createUdkSm: recording[%d] ptr %p", i, userRecsArray[i]);
       userRecsArray[i] = (listen_user_recording *)malloc((int)(sizeof(listen_user_recording)));
       if (NULL == userRecsArray[i]) {
          ALOGE("_createUdkSm: ERROR malloc for listen_user_recording failed");
          retStatus = LISTEN_ENO_MEMORY;
          goto cleanup_for_create_udk_sm;
       }

       ALOGV("_createUdkSm: call GetObjectArrayElement for element %d of array ptr %p", i, jUserRecordings);
       jobject userRecObj =  env->GetObjectArrayElement(jUserRecordings, i); // get i-th jobject recording
       if (NULL == userRecObj) {
          ALOGE("_createUdkSm: ERROR GetObjectArrayElement of jUserRecordings failed");
          retStatus = LISTEN_EBAD_PARAM;
          goto cleanup_for_create_udk_sm;
       }

       // copy contents of byteArray into byte array
       // extract data and numsamples from recording jobject & put in recording struct
       eStatus = getRecFromShortBuff(env, userRecObj, &userRecsArray[i]->n_samples, &userRecsArray[i]->data);
       if (LISTEN_SUCCESS != eStatus) {
           ALOGE("_createUdkSm: ERROR getRecFromShortBuff returned %d", eStatus);
           retStatus = LISTEN_EFAILURE;
           goto cleanup_for_create_udk_sm;
       }
       ALOGV("_createUdkSm: user recording[%d] (ptr,size)=(%p,%d)",
          i, userRecsArray[i]->data, userRecsArray[i]->n_samples);
    }

    // convert jstring keywordPhrase and userName to char *
    kwString = (keywordId_t)env->GetStringUTFChars(jKeyphrase, 0);
    userString = (keywordId_t)env->GetStringUTFChars(jUsername, 0);

    retStatus = getByteBufferCapacity(env, jCreatedUdkSm, &smMax);
    if (LISTEN_SUCCESS != eStatus) {
       ALOGE("_createUdkSm: ERROR getByteBufferCapacity returned %d", retStatus);
       goto cleanup_for_create_udk_sm;
    }
    ALOGV("_createUdkSm: size of output buffer is %d", smMax);

    // allocate
    ALOGV("_createUdkSm: Allocate memory for output parameter outputModel, max size %d", smMax);
    outputModel.size = smMax;
    outputModel.data = (uint8_t *)malloc(smMax);
    if (NULL == outputModel.data) {
      ALOGE("_createUdkSm: ERROR malloc'ing outputModel data failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_create_udk_sm;
    }
    ALOGV("_createUdkSm: ptr to userKeyword Model memory is %p", outputModel.data);

    ALOGV("_createUdkSm: call NEW createUserKeywordModel() for %s, %s", kwString, userString);
    status = ListenSoundModelLibV2::createUserDefinedKeywordModel(NULL, kwString, userString,
            NULL, jNumberUserRecordings, userRecsArray, &languageModel, &outputModel, &userMatchingScore);
    if (kCannotCreateUserUDK == status) {
      // createUserDefinedKeywordModel tests that size of output UDK SM is big enough for resulting output
      ALOGE("_createUdkSm: size of output ByteBuffer %d is not big enough for output UDK SM", smMax);
      retStatus = LISTEN_EBAD_PARAM;
      goto cleanup_for_create_udk_sm;
    } else if (kSucess != status) {
      ALOGE("_createUdkSm: ERROR createUserDefinedKeywordModel() failed status %d", (int)status);
      MAP_ERR(status, retStatus);
      goto cleanup_for_create_udk_sm;
      // don't return; drop down to clean up memory
    } else {
        ALOGV("_createUdkSm: createUserKeywordModel output userMatchingScore = %d", (int)userMatchingScore);
      eStatus = putSoundModelInByteBuffer(env, jCreatedUdkSm, outputModel.size, outputModel.data);
      if (eStatus != LISTEN_SUCCESS) {
         ALOGE("_createUdkSm: ERROR putSoundModelInByteBuffer() failed with %d", (int)eStatus );
         retStatus = eStatus;
         goto cleanup_for_create_udk_sm;
      }

      // Fill confidence structure field into ListenTypes$ConfidenceData for output.
      confidenceDataClazz = env->FindClass("com/qualcomm/listen/ListenTypes$ConfidenceData");
      if (NULL == confidenceDataClazz) {
         ALOGE("_createUdkSm: ERROR confidenceDataClazz class not found");
         retStatus = LISTEN_EFAILURE;
         goto cleanup_for_create_udk_sm;
      }
      // get ID of the field ListenTypes$ConfidenceData
      userMatchId = env->GetFieldID(confidenceDataClazz, "userMatch", "I");
      if (NULL == userMatchId) {
         ALOGE("userMatch from DetectionData class not acquired");
         retStatus = LISTEN_EFAILURE;
         goto cleanup_for_create_udk_sm;
      }

      // place user match score into qualityData output parameter object
      jUserMatchScore = (jint)userMatchingScore;  // convert short to jint
      ALOGV("createUserKeywordModel set matchingScore %d in to qualityData", (int)jUserMatchScore);
      env->SetIntField(jConfidenceData, userMatchId, jUserMatchScore);

      ALOGV("createUserKeywordModel() returns newSM(data,size)= (%p,%d), score %d",
               outputModel.data, outputModel.size, jUserMatchScore);
    }

cleanup_for_create_udk_sm:
   // free userRecsArray array
   if (userRecsArray) {
      for (uint32_t i=0; i < nRecs; i++) {
         if (userRecsArray[i]) {
             if (userRecsArray[i]->data != NULL)
                free((void *) userRecsArray[i]->data);
             free((void *)userRecsArray[i]);
         }
      }
      free((void *)userRecsArray);
   }
   // clean up keyword model data structures
   if (languageModel.data != NULL)
       free((void *) languageModel.data);
   if (outputModel.data != NULL)
      free((void *)outputModel.data);
   if (kwString != NULL)
       env->ReleaseStringUTFChars(jKeyphrase, kwString);
   if (userString != NULL)
       env->ReleaseStringUTFChars(jUsername, userString);

   ALOGV("_createUdkSm returns %d", retStatus);
   return retStatus;
}

/*
 * parseVWUDetectionEventData
 *
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    parseVWUDetectionEventData
 * Signature: (Ljava/nio/ByteBuffer;Lcom/qualcomm/listen/ListenTypes/EventData;
 *             Lcom/qualcomm/listen/ListenTypes/VoiceWakeupDetectionData;)I
 *
 * Parsers generic payload passed to processEvent() for VoiceWakeup-specific Event Data and
 *    fill fields within SVA 1.0 voice wakeup detection event data structure.
 * Requires the SoundModel that was used for detection to be given as input.
 *
 * Param [in]   env - ptr to JNIEnv
 * Param [in]   clazz - java class (not an object instance) of this static method
 * Param [in]   registeredSoundModel - sound model keyword detection was performed with
 * Param [in]   eventPayload - black-box event payload returned by ListenEngine
 * Param [in/out]  outDataStruct - VoiceWakeup detection data structure created in Java
 *              filled by this method
 *
 * Return - status
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_parseVWUDetectionEventData
                (JNIEnv *env, jclass clazz,      // ListenSoundModel
                 jobject registeredSoundModel,   // ByteBuffer
                 jobject eventPayloadObj,        // ListenTypes$EventData
                 jobject outDataStruct)          // ListenTypes$VoiceWakeupDetectionData
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_parseVWUDetectionEventData entered");
   listen_model_type           soundModel;
   listen_event_payload        eventPayload;
   listen_detection_event_type detectionData;
   listen_status_enum          status = kSucess;
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   jclass                      eventDataClazz;
   jfieldID                    dataSizeId;
   jfieldID                    dataArrayId;
   jobject                     jArrayObj;
   jbyteArray                  byteArray;
   int                         arrayLen = 0;
   jclass                      vwuDetectionDataClazz  = NULL;
   jfieldID                    statusId;
   jfieldID                    typeId;
   jfieldID                    keywordId;
   jfieldID                    keywordConfidenceLevelId;
   jfieldID                    userConfidenceLevelId;
   jstring                     typeStr = NULL;
   jstring                     keywordStr = NULL;
   jsize                       strLen = 0;
   listen_detection_event_v1 * vwuDetDataV1 = NULL;
   listen_sound_model_info     modelInfo;

#ifdef SVA1_SUPPORTED
   if ( (NULL == registeredSoundModel) || (NULL == eventPayloadObj)  ) {
      ALOGE("_parseVWUDetectionEventData: ERROR Null ptr passed to ListenSoundModel.parseDetectionEventData");
      return (jint)LISTEN_EBAD_PARAM;
   }
   eventPayload.data = NULL;
   eventPayload.size = 0;
   soundModel.data = NULL;
   soundModel.size = 0;
   ALOGV("_parseVWUDetectionEventData: ref to Java registered SM %p", registeredSoundModel);

   // extract data and size from registeredSoundModel jobject & put in soundModel struct
   eStatus = getSMFromByteBuff(env, registeredSoundModel, &soundModel.size, &soundModel.data, &modelInfo);
   if ( LISTEN_SUCCESS != eStatus) {
      ALOGE("_parseVWUDetectionEventData: ERROR getSMFromByteBuff returned %d", eStatus);
      goto cleanup_for_parse_event;
   }
   ALOGV("registered SoundModel (ptr,size)=(%p,%d)", soundModel.data, soundModel.size);
   // This function should only be called for SVA 1.0 SoundModels
   if (modelInfo.version != SM_VERSION_1p0) {
      ALOGE("_parseVWUDetectionEventData: ERROR SM Version is not 1.0");
      eStatus = LISTEN_EBAD_PARAM;
      goto cleanup_for_parse_event;
   }

   // extract data and size from event_payload jobject
   //    and place into eventPayload struct
   eventDataClazz = env->FindClass("com/qualcomm/listen/ListenTypes$EventData");
   if (NULL == eventDataClazz) {
      ALOGE("_parseVWUDetectionEventData: ERROR EventData class not found");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_parse_event;
   }
   // get size and payload from eventData jObject
   dataSizeId = env->GetFieldID(eventDataClazz, "size", "I");
   dataArrayId  = env->GetFieldID(eventDataClazz, "payload", "[B");
   if ( (NULL==dataSizeId) || (NULL==dataArrayId) ) {
      ALOGE("_parseVWUDetectionEventData: ERROR field from EventData class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_parse_event;
   }
   eventPayload.size = env->GetIntField(eventPayloadObj, dataSizeId);

   //
   // extract byte array containing event payload from jobject input parameter
   // store payload and size into eventPayload C structure
   jArrayObj = env->GetObjectField(eventPayloadObj, dataArrayId);
   if (NULL == jArrayObj) {
      ALOGE("_parseVWUDetectionEventData: ERROR payload array is NULL");
      eStatus = LISTEN_EBAD_PARAM;
      goto cleanup_for_parse_event;
   }
   ALOGV("_parseVWUDetectionEventData: cast jArrayObj = %p to byteArray", jArrayObj);
   byteArray = (jbyteArray)jArrayObj;
   ALOGV("_parseVWUDetectionEventData: GetArrayLength");
   arrayLen = env->GetArrayLength(byteArray);
   ALOGV("_parseVWUDetectionEventData: Allocate byte array of size %d", arrayLen);
   eventPayload.data = (uint8_t *)malloc(arrayLen);
   if (NULL == eventPayload.data) {
      ALOGE("_parseVWUDetectionEventData: ERROR malloc of detection event payload data failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_parse_event;
   }
   ALOGV("_parseVWUDetectionEventData: GetByteArrayRegion w/ byteArray=%p, len=%d, into eventPayload.data=%p",
      byteArray, arrayLen, eventPayload.data );
   env->GetByteArrayRegion(byteArray, (jsize)0, (jsize)arrayLen, (jbyte *)eventPayload.data);

   // Call Listen SoundModelLib function to convert black-box event data into
   //    specific structure with specific, well-understood elements
   ALOGV("parseDetectionEventData() SM (data,size)= (%p,%d),...",
      soundModel.data, soundModel.size);
   ALOGV("parseDetectionEventData() payload (data,size)= (%p,%d),...",
      eventPayload.data, eventPayload.size);
   for (int j=0; j<(int)eventPayload.size ;j++) {
      ALOGV("  payload[%d] = 0x%x", j, eventPayload.data[j] );
   }
   // zero out elements of output structure; makes ptrs NULL
   memset(&detectionData, 0, (int)(sizeof(listen_detection_event_type)));
   status = ListenSoundModelLib::parseDetectionEventData (
                 &soundModel, &eventPayload, &detectionData);
   if (kSucess != status) {
      ALOGE("_parseVWUDetectionEventData: ERROR parseDetectionEventData() returned status %d",
           (int)status);
      MAP_ERR(status, eStatus);
      goto cleanup_for_parse_event;
   }
   ALOGI("parseDetectionEventData() version = SM version = ??? ");

   ALOGI("parseDetectionEventData() returns keyword %s",
      detectionData.event.event_v1.keyword);
   ALOGI("                                  keywordConfidenceLevel %d",
      detectionData.event.event_v1.keywordConfidenceLevel);
   ALOGI("                                  userConfidenceLevel %d",
      detectionData.event.event_v1.userConfidenceLevel  );

   // NOTE: In the future, this code will need to check the detectionData.version and
   //     map this to the correct data structure
   // For now - since there is only one version of detection data - current version of code
   //     maps this to listen_detection_event_v1 without test
   // Fill return detection structure fields into ListenTypes$VoiceWakeDetectionData that is output
   // create a class object of type VoiceWakeupDetectionData
   vwuDetectionDataClazz = env->FindClass("com/qualcomm/listen/ListenTypes$VoiceWakeupDetectionData");
   if (NULL == vwuDetectionDataClazz) {
       ALOGE("_parseVWUDetectionEventData: ERROR FindClass(com/qualcomm/listen/ListenTypes$VoiceWakeupDetectionData) failed !");
       eStatus = LISTEN_EFAILURE;
       goto cleanup_for_parse_event;
   }
   // get all Id of all the common ListenTypes$DetectionData fields
   statusId = env->GetFieldID(vwuDetectionDataClazz, "status", "I");
   typeId  = env->GetFieldID(vwuDetectionDataClazz, "type", "Ljava/lang/String;");

   // get all Id of all the ListenTypes$VoiceWakeDetectionData fields
   keywordId  = env->GetFieldID(vwuDetectionDataClazz, "keyword", "Ljava/lang/String;");
   keywordConfidenceLevelId  = env->GetFieldID(vwuDetectionDataClazz, "keywordConfidenceLevel", "S");
   userConfidenceLevelId  = env->GetFieldID(vwuDetectionDataClazz, "userConfidenceLevel", "S");
   if ( (NULL==statusId) || (NULL==typeId) ||(NULL==keywordId) ||
        (NULL==keywordConfidenceLevelId) || (NULL==userConfidenceLevelId) )
   {
      ALOGE("_parseVWUDetectionEventData: ERROR field from VoiceWakeDetectionData class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_parse_event;
   }

   // place return detection data into jobject outDataStruct
   ALOGV("_parseVWUDetectionEventData: SetIntField status");
   env->SetIntField(outDataStruct, statusId, status);  // output status of LSMLib parseDetEventData

   ALOGV("_parseVWUDetectionEventData: NewStringUTF called");
   typeStr = env->NewStringUTF("VoiceWakeup_DetectionData_v0100");

   // All objects within return class 'outDataStruct' ('typeStr' and 'keywordStr') GC'ed by Java
   ALOGV("_parseVWUDetectionEventData: SetObjectField keyword");
   env->SetObjectField(outDataStruct, typeId, typeStr);
   // If there was an error from parse function, just return status and type now
   if (kSucess != status) {
      ALOGE("_parseVWUDetectionEventData: ERROR parseDetectionEventData() failed status %d",
          (int)status);
      ALOGD("_parseVWUDetectionEventData: only status and type set");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_parse_event;
   }

   keywordStr = env->NewStringUTF( detectionData.event.event_v1.keyword );
   strLen = env->GetStringUTFLength(keywordStr);
   ALOGV("_parseVWUDetectionEventData: new jstring contains keyword %s, length %d", detectionData.event.event_v1.keyword, (int)strLen);
   ALOGV("_parseVWUDetectionEventData: SetObjectField keywordStr");
   env->SetObjectField(outDataStruct, keywordId, keywordStr);

   vwuDetDataV1 = &detectionData.event.event_v1;
   ALOGV("_parseVWUDetectionEventData: SetShortField keywordConfidenceLevel");
   env->SetShortField(outDataStruct, keywordConfidenceLevelId, vwuDetDataV1->keywordConfidenceLevel);
   ALOGV("SetShortField userConfidenceLevel");
   env->SetShortField(outDataStruct, userConfidenceLevelId, vwuDetDataV1->userConfidenceLevel);

cleanup_for_parse_event:
   if (eventPayload.data)
      free((void *)eventPayload.data);
   // detectionData.keyword is a static array within detectionData so does not have to be freed
   if (soundModel.data != NULL)
      free((void *)soundModel.data);

   ALOGV("_ListenSoundModel_parseVWUDetectionEventData returns");
   return (jint)eStatus;
#else
   ALOGE("_ListenSoundModel_parseVWUDetectionEventData should not be called");
   ALOGE("SVA 1.0 SoundModels not supported");
   return LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION;
#endif // SVA1_SUPPORTED
}


/*
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    parseVWUDetectionEventDataV2
 * Signature: (Ljava/nio/ByteBuffer;Lcom/qualcomm/listen/ListenTypes/EventData;
 *             Lcom/qualcomm/listen/ListenTypes/VoiceWakeupDetectionDataV2;)I
 *
 * Parsers generic payload passed to processEvent() for VoiceWakeup-specific Event Data and
 *    fill fields within SVA 2.0 voice wakeup detection event data structure.
 * Requires the SoundModel that was used for detection to be given as input.
 * Before calling app processEvent(), the JNI pre-pends  detection/registeration
 *    fields to the end of the payload data so that this method can get these value.
 *    This additional data (at end of byte event data array) is:
 *        detection status returned by algo: success or failure
 *        detection mode set during time of registeration
 *        all keyword & active pair confidence level values set during registeration
 *        size of this confidence level array
 *    This method extracts each of the fields so that they can be passed to
 *        ListenSoundModelLib::parseDetectionEventDataV2().
 *
 * Param [in]   env - ptr to JNIEnv
 * Param [in]   clazz - java class (not an object instance) of this static method
 * Param [in]   registeredSoundModel - sound model keyword detection was performed with
 * Param [in]   eventPayload - black-box event payload returned by ListenEngine
 * Param [in/out]  outDataStruct - VoiceWakeup detection data structure created in Java
 *              filled by this method
 *
 * Return - status
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_parseVWUDetectionEventDataV2
                (JNIEnv *env, jclass clazz,      // ListenSoundModel
                 jobject registeredSoundModel,   // ByteBuffer
                 jobject eventPayloadObj,        // ListenTypes$EventData
                 jobject outDataStruct)          // ListenTypes$VoiceWakeupDetectionDataV2
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_parseVWUDetectionEventDataV2 entered");

   if ( (NULL == registeredSoundModel) || (NULL == eventPayloadObj) || (NULL == outDataStruct) ){
      ALOGE("_parseVWUDetectionEventDataV2: ERROR param is NULL");
      return (jint)LISTEN_EBAD_PARAM;
   }
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   listen_event_payload        eventPayload;
   listen_detection_event_type detectionEvent;  // contains listen_detection_event_v2 struct
   listen_model_type           soundModel;
   listen_sound_model_info     modelInfo;
   uint8_t *                   payloadData = NULL;
   int                         payloadSize = 0;
   uint8_t                     confLevelSize;

   jclass                      eventDataV2Clazz;
   jobject                     jArrayObj;
   jfieldID                    dataSizeId;
   jfieldID                    dataArrayId; // byte array
   jclass                      vwuDetectionDataV2Clazz  = NULL;
   jfieldID                    statusId;
   jfieldID                    typeId;
   jfieldID                    keywordConfLevelArrId;
   jfieldID                    userConfLevelArrId;

   jbyteArray                  byteArray;
   int                         arrayLen = 0;
   jstring                     typeStr = NULL;
   jsize                       strLen = 0;

   jclass                      vwuKWConfLevelClazz;
   jclass                      vwuUserKWPairConfLevelClazz;
   jfieldID                    kwConfLevelKWStrId;
   jfieldID                    kwConfLevelValId;
   jfieldID                    pairConfLevelKWStrId;
   jfieldID                    pairConfLevelUserStrId;
   jfieldID                    pairConfLevelValId;
   listen_sound_model_header	 modelHeader;
   uint16_t                    numKWs;
   uint16_t                    numUsers;
   uint16_t                    numActivePairs;
   int                         numKWsToOutput;
   int                         numPairsToOutput = 0;
   int                         kwsOutputCount;
   int                         pairsOutputCount;
   int                         iActivePairs;
   jstring                     keyword;
   jstring                     user;
   jobject                     tmpKWConfLevel = NULL;
   jobject                     tmpPairConfLevel = NULL;
   jobjectArray                kwConfLevelArr = NULL;
   jobjectArray                pairConfLevelArr = NULL;
   jmethodID                   vwuKWConfLevelConstructorId;
   jmethodID                   vwuUserKWPairConfLevelConstructorId;
   uint16_t *                  pUserActiveFlags;
   char **                     keywordPhrases = NULL;
   char **                     userNames = NULL;

   eventPayload.data = NULL;
   eventPayload.size = 0;
   soundModel.data = NULL;
   soundModel.size = 0;
   detectionEvent.event.event_v2.pairConfidenceLevels.pConfLevels = NULL;
   detectionEvent.event.event_v2.pairConfidenceLevels.size = 0;

   ALOGV("_parseVWUDetectionEventDataV2: ref to Java registered SM %p", registeredSoundModel);

   // extract data and size from registeredSoundModel jobject & put in soundModel struct
   eStatus = getSMFromByteBuff(env, registeredSoundModel, &soundModel.size, &soundModel.data, &modelInfo);
   if ( LISTEN_SUCCESS != eStatus) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR getSMFromByteBuff returned %d", eStatus);
      if (soundModel.data) {
         free((void *)soundModel.data);
      }
      return eStatus;
   }

   memset(&modelHeader, 0, sizeof(listen_sound_model_header));
   status = ListenSoundModelLibV2::getSoundModelHeader(&soundModel, &modelHeader);
   if (kSucess != status) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR getSoundModelHeader() status %d",
               (int)status);
      MAP_ERR(status, eStatus);
      if (soundModel.data) {
         free((void *)soundModel.data);
      }
      return eStatus;
   }
   numKWs = modelHeader.numKeywords;
   numUsers = modelHeader.numUsers;
   numActivePairs = modelHeader.numActiveUserKeywordPairs;
   ALOGD("_parseVWUDetectionEventDataV2: getSoundModelHeader returns numKWs,numActivePairs %d,%d",
                numKWs, numActivePairs);
   ALOGV("registered SoundModel (ptr,size)=(%p,%d)", soundModel.data, soundModel.size);

   // extract fields and byte arrays from eventPayload
   eventDataV2Clazz = env->FindClass("com/qualcomm/listen/ListenTypes$EventData");
   if (NULL == eventDataV2Clazz) {
      ALOGE("_parseVWUDetectionEventData: ERROR EventData class not found");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_parse_event_v2;
   }
   ALOGV("_parseVWUDetectionEventDataV2 GetFieldID(gEventDataClazz,...");
   dataSizeId = env->GetFieldID(eventDataV2Clazz, "size", "I");
   dataArrayId  = env->GetFieldID(eventDataV2Clazz, "payload", "[B"); // byte array
   if ( (dataSizeId == NULL) || (dataArrayId == NULL) ) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR field(s) from EventData class not acquired");
      goto cleanup_for_parse_event_v2;
   }
   payloadSize = env->GetIntField(eventPayloadObj, dataSizeId);

   // extract byte array containing event payload from jobject input parameter
   // store payload and size into detectionEvent C structure
   jArrayObj = env->GetObjectField(eventPayloadObj, dataArrayId);
   if (NULL == jArrayObj) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR payload array is NULL");
      eStatus = LISTEN_EBAD_PARAM;
      goto cleanup_for_parse_event_v2;
   }
   ALOGV("_parseVWUDetectionEventDataV2: cast jArrayObj = %p to byteArray", jArrayObj);
   byteArray = (jbyteArray)jArrayObj;
   ALOGV("_parseVWUDetectionEventDataV2: GetArrayLength");
   arrayLen = env->GetArrayLength(byteArray);
   // compare payload size in eventPayloadObj to actual length of array in same object
   if (payloadSize != arrayLen) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR payload size %d != actual array length %d!",
                payloadSize, arrayLen);
      goto cleanup_for_parse_event_v2;
   }

   ALOGV("_parseVWUDetectionEventDataV2: Allocate byte array of size %d", arrayLen);
   payloadData = (uint8_t *)malloc(arrayLen);
   if (NULL == payloadData) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR malloc of extended detection event payload data failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_parse_event_v2;
   }
   ALOGV("_parseVWUDetectionEventDataV2: GetByteArrayRegion w/ byteArray=%p, len=%d, into payloadData=%p",
                     byteArray, arrayLen, payloadData );
   env->GetByteArrayRegion(byteArray, (jsize)0, (jsize)arrayLen, (jbyte *)payloadData);
   confLevelSize = payloadSize;
   ALOGD("_parseVWUDetectionEventDataV2: confidence level size = %d", (int)confLevelSize);
   eventPayload.size = (uint32_t )confLevelSize;
   eventPayload.data = (uint8_t *)malloc(confLevelSize);
   if (NULL == eventPayload.data) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR malloc'ing eventPayload.data failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_parse_event_v2;
   }

   memcpy(eventPayload.data, &payloadData[0], (int)confLevelSize);

   // need to malloc space for the confidence levels returned in detectionEvent
   detectionEvent.event.event_v2.pairConfidenceLevels.pConfLevels = (uint8_t *)malloc(confLevelSize);
   if (NULL == detectionEvent.event.event_v2.pairConfidenceLevels.pConfLevels) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR malloc'ing detectionEvent.event.event_v2.pairConfidenceLevels.pConfLevels failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_parse_event_v2;
   }
   // Call Listen SoundModelLib function to convert black-box event data into
   //    specific structure with specific, well-understood elements
   ALOGD("_parseVWUDetectionEventDataV2: call parseDetectionEventData()");
   status = ListenSoundModelLibV2::parseDetectionEventData (
                 &soundModel, &eventPayload,
                 &detectionEvent);
   if (kSucess != status) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR parseDetectionEventData() status %d",
           (int)status);
      MAP_ERR(status, eStatus);
      goto cleanup_for_parse_event_v2;
   }
   ALOGD("_parseVWUDetectionEventDataV2 parseDetectionEventData() outputs:");
   ALOGD("              type %d = ",
            (int)detectionEvent.detection_data_type);
   ALOGD("              keyword %s, user %s",
         detectionEvent.event.event_v2.keywordPhrase, detectionEvent.event.event_v2.userName);
   ALOGD("              keyword,user conf levels as %d,%d",
         detectionEvent.event.event_v2.highestKeywordConfidenceLevel,
         detectionEvent.event.event_v2.highestUserConfidenceLevel );
   ALOGD("              conf levels size %d at %p",
         detectionEvent.event.event_v2.pairConfidenceLevels.size,
         detectionEvent.event.event_v2.pairConfidenceLevels.pConfLevels );

   // NOTE: In the future, this code will need to check the detectionData version and
   //     map this to the correct data structure
   // For now - since there is only one version of detection data
   //     current SVA 2.0 version of code maps this to listen_detection_event_v2 without test
   // Fill return detection structure fields into ListenTypes$VoiceWakeDetectionData that is output
   // create a class object of type VoiceWakeupDetectionData
   vwuDetectionDataV2Clazz = env->FindClass("com/qualcomm/listen/ListenTypes$VoiceWakeupDetectionDataV2");
   if (NULL == vwuDetectionDataV2Clazz) {
       ALOGE("_parseVWUDetectionEventDataV2: ERROR FindClass(com/qualcomm/listen/ListenTypes$VoiceWakeupDetectionDataV2) failed !");
       eStatus = LISTEN_EFAILURE;
       goto cleanup_for_parse_event_v2;
   }
   // get all Id of all the ListenTypes$VoiceWakeDetectionDataV2 fields
   statusId = env->GetFieldID(vwuDetectionDataV2Clazz, "status", "I");
   typeId  = env->GetFieldID(vwuDetectionDataV2Clazz, "type", "Ljava/lang/String;");
   // get all Id of all the ListenTypes$VoiceWakeDetectionData fields
   keywordConfLevelArrId  = env->GetFieldID(vwuDetectionDataV2Clazz, "nonzeroKWConfLevels", "[Lcom/qualcomm/listen/ListenTypes$VWUKeywordConfLevel;");
   userConfLevelArrId  = env->GetFieldID(vwuDetectionDataV2Clazz, "nonzeroUserKWPairConfLevels", "[Lcom/qualcomm/listen/ListenTypes$VWUUserKeywordPairConfLevel;");
   if ( (NULL==statusId) || (NULL==typeId) ||
        (NULL==keywordConfLevelArrId) || (NULL==userConfLevelArrId) )
   {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR field from VoiceWakeDetectionData class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_parse_event_v2;
   }

   // place return detection data into jobject outDataStruct
   ALOGV("_parseVWUDetectionEventDataV2: SetIntField status");
   env->SetIntField(outDataStruct, statusId, status);  // output status of LSMLib parseDetEventData

   ALOGV("_parseVWUDetectionEventDataV2: NewStringUTF called");
   typeStr = env->NewStringUTF("VoiceWakeup_DetectionData_v0105");

   // All objects within return class 'outDataStruct' (e.g. 'typeStr') GC'ed by Java
   ALOGV("_parseVWUDetectionEventDataV2: SetObjectField keyword");
   env->SetObjectField(outDataStruct, typeId, typeStr);
   // If there was an error from parse function, just return status and type now

   vwuKWConfLevelClazz = env->FindClass("com/qualcomm/listen/ListenTypes$VWUKeywordConfLevel");
   vwuUserKWPairConfLevelClazz = env->FindClass("com/qualcomm/listen/ListenTypes$VWUUserKeywordPairConfLevel");
   if ( (NULL == vwuKWConfLevelClazz) || (NULL == vwuUserKWPairConfLevelClazz) ) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR VWUKeywordConfLevel or VWUUserKeywordPairConfLevel class not found");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_parse_event_v2;
   }
   kwConfLevelKWStrId = env->GetFieldID(vwuKWConfLevelClazz, "keyword", "Ljava/lang/String;");
   kwConfLevelValId = env->GetFieldID(vwuKWConfLevelClazz, "confLevel", "S"); // short
   pairConfLevelKWStrId = env->GetFieldID(vwuUserKWPairConfLevelClazz, "keyword", "Ljava/lang/String;");
   pairConfLevelUserStrId = env->GetFieldID(vwuUserKWPairConfLevelClazz, "user", "Ljava/lang/String;");
   pairConfLevelValId = env->GetFieldID(vwuUserKWPairConfLevelClazz, "confLevel", "S");  // short

   if ( (NULL==kwConfLevelKWStrId) || (NULL==kwConfLevelValId) ||
        (NULL==pairConfLevelKWStrId) || (NULL==pairConfLevelUserStrId) || (NULL==pairConfLevelValId))  {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR field(s) from VWUKeywordConfLevel or VWUUserKeywordPairConfLevel not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_parse_event_v2;
   }

   // get the strings for kw and users
   ALOGD("_parseVWUDetectionEventDataV2: getKeywordPhrases ");
   eStatus = allocStrArr(numKWs, &keywordPhrases);
   if (eStatus != LISTEN_SUCCESS) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR could not alloc memory for String arrayu");
      goto cleanup_for_parse_event_v2;
   }

   status = ListenSoundModelLibV2::getKeywordPhrases(&soundModel, &numKWs, keywordPhrases);
   if (kSucess != status) {
      ALOGE("_parseVWUDetectionEventDataV2: ERROR getKeywordPhrases() status %d", (int)status);
      MAP_ERR(status, eStatus);
      goto cleanup_for_parse_event_v2;
   }
   if (numUsers > 0) {
      ALOGD("_parseVWUDetectionEventDataV2: getUserNames ");
      eStatus = allocStrArr(numUsers, &userNames);
      if (eStatus != LISTEN_SUCCESS) {
         ALOGE("_parseVWUDetectionEventDataV2: ERROR could not alloc memory for String arrayu");
         goto cleanup_for_parse_event_v2;
      }
      status = ListenSoundModelLibV2::getUserNames(&soundModel, &numUsers, userNames);
      if (kSucess != status) {
         ALOGE("_parseVWUDetectionEventDataV2: ERROR getUserNames() status %d", (int)status);
         MAP_ERR(status, eStatus);
         goto cleanup_for_parse_event_v2;
      }
   }

   // find how many keyword conf levels need to be output in array nonzeroKWConfLevels[]
   // find how many user+keyword pair conf levels need to be output in the array nonzeroUserKWPairConfLevels[]
   if (kFailure == eventPayload.status) {
      // when detection was failure output confidence level for ALL keywordsa and all active pairs
      numKWsToOutput = numKWs;
      numPairsToOutput = numActivePairs;
   } else {
      numKWsToOutput = 0;
      for (int i=0; i < numKWs; i++) {
         if (eventPayload.data[i] > 0)  {
            numKWsToOutput++;
         }
      }
      for (int i=0; i < numActivePairs; i++) {
         if (eventPayload.data[numKWs+i] > 0)  {
            numPairsToOutput++;
         }
      }
   }
   vwuKWConfLevelConstructorId = env->GetMethodID(vwuKWConfLevelClazz, "<init>", "()V");
   tmpKWConfLevel = env->NewObject(vwuKWConfLevelClazz, vwuKWConfLevelConstructorId);  // cleanup at end

   vwuUserKWPairConfLevelConstructorId  = env->GetMethodID(vwuUserKWPairConfLevelClazz, "<init>", "()V");
   tmpPairConfLevel = env->NewObject(vwuUserKWPairConfLevelClazz, vwuUserKWPairConfLevelConstructorId);  // cleanup at end

   kwConfLevelArr = env->NewObjectArray(numKWsToOutput, vwuKWConfLevelClazz, tmpKWConfLevel);
   pairConfLevelArr = env->NewObjectArray(numPairsToOutput, vwuUserKWPairConfLevelClazz, tmpPairConfLevel);

   ALOGD("_parseVWUDetectionEventDataV2: place kwConfLevelArr within VoiceWakeupDetectionDataV2");
   env->SetObjectField(outDataStruct, keywordConfLevelArrId , kwConfLevelArr);
   ALOGD("_parseVWUDetectionEventDataV2: place pairConfLevelArr within VoiceWakeupDetectionDataV2");
   // place newly allocated arrays within parserDetectionEventData object
   env->SetObjectField(outDataStruct, userConfLevelArrId , pairConfLevelArr);

   // fill kwConfLevelArr array
   ALOGD("_parseVWUDetectionEventDataV2: fill %d KWs", numKWsToOutput);
   kwsOutputCount = 0;
   for (int kw=0; kwsOutputCount < numKWsToOutput; kw++) {
      if ( (eventPayload.data[kw] > 0) || (numKWsToOutput == numKWs) ) {
         // create a new KWInfo obj for each element of array
         jobject iKWConfLevel = env->NewObject(vwuKWConfLevelClazz, vwuKWConfLevelConstructorId);
         env->SetObjectArrayElement(kwConfLevelArr, kwsOutputCount , iKWConfLevel);
         // fill keywordPhrase, confidence level value if non-zero or if all values to be output
         keyword = env->NewStringUTF(keywordPhrases[kw]);  // GC'ed
         env->SetObjectField(iKWConfLevel, kwConfLevelKWStrId , keyword);
         env->SetShortField(iKWConfLevel, kwConfLevelValId , eventPayload.data[kw]);
         kwsOutputCount++;
         if (iKWConfLevel)
             env->DeleteLocalRef(iKWConfLevel); // local object not GC'ed by Java
      }
   }

   // fill pairConfLevelArr array
   ALOGD("_parseVWUDetectionEventDataV2: fill %d pairs", numPairsToOutput);
   pairsOutputCount = 0;
   iActivePairs = numKWs;  // offset to first active pair after keyword confidence level values
   for (int u=0; u < numUsers; u++) {
      pUserActiveFlags = modelHeader.userKeywordPairFlags[u];
      for (int k=0; k < numKWs; k++) {
         // test if pair is active by looking at truth table [user][kw] flags
         if (pUserActiveFlags[k]) {
            // if confidence level for active pair is > 0 output it
            if ( (eventPayload.data[iActivePairs] > 0) || (numPairsToOutput == numActivePairs) ) {
               // create a new KWInfo obj for each element of array
               jobject iPairConfLevel = env->NewObject(vwuUserKWPairConfLevelClazz,
                           vwuUserKWPairConfLevelConstructorId);
               env->SetObjectArrayElement(pairConfLevelArr, pairsOutputCount , iPairConfLevel);
               // fill keywordPhrase, usersName, confidence level value if non-zero or if all values to be output
               keyword = env->NewStringUTF(keywordPhrases[k]);  // GC'ed
               env->SetObjectField(iPairConfLevel, pairConfLevelKWStrId , keyword);
               user = env->NewStringUTF(userNames[u]);   // GC'ed
               env->SetObjectField(iPairConfLevel, pairConfLevelUserStrId , user);
               env->SetShortField(iPairConfLevel, pairConfLevelValId , eventPayload.data[iActivePairs]);
               pairsOutputCount++;
               if (iPairConfLevel)
                  env->DeleteLocalRef(iPairConfLevel); // local object not GC'ed by Java
            }
            if (pairsOutputCount >= numPairsToOutput) {
               // break out of loops
               k = numKWs;
               u = numUsers;
               break;

            }
            iActivePairs++;
         }
      }
   }

cleanup_for_parse_event_v2:
   if (payloadData)
      free( (void *)payloadData );
   if (eventPayload.data)
      free( (void *)eventPayload.data );
   if (soundModel.data != NULL)
      free((void *)soundModel.data);
   if (detectionEvent.event.event_v2.pairConfidenceLevels.pConfLevels)
      free( (void *)(detectionEvent.event.event_v2.pairConfidenceLevels.pConfLevels) );
   if (tmpKWConfLevel)
      env->DeleteLocalRef(tmpKWConfLevel);
   if (tmpPairConfLevel)
      env->DeleteLocalRef(tmpPairConfLevel);
   if (kwConfLevelArr)
      env->DeleteLocalRef(kwConfLevelArr);
   if (pairConfLevelArr)
      env->DeleteLocalRef(pairConfLevelArr);
   freeStrArr(numKWs, keywordPhrases); // silently ignore errors
   if (numUsers > 0)
      freeStrArr(numUsers, userNames); // silently ignore errors

   status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
   if (kSucess != status) {
      ALOGE("_getSizeAfterDelete: ERROR releaseSoundModelHeader() failed status %d", (int)status);
      MAP_ERR(status, eStatus);
   }

   ALOGV("_ListenSoundModel_parseVWUDetectionEventDataV2 returns");
   return (jint)eStatus;
}
/*
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    getTypeVersion
 * Signature: (Ljava/nio/ByteBuffer;Lcom/qualcomm/listen/ListenTypes/SoundModelInfo;)I
 *
 * Parsers generic information from SoundModel header common to
 * all SVA SoundModel types and versions.
 * Fill the fields in the simple ListenTypes.SoundModelInfo base class.
 *
 * Param [in]   soundModel - SoundModel to query
 * Param [in]   soundModelInfo - ref to ListenTypes$SoundModelInfo instance created in app
 * Return - status - LISTEN_SUCCESS or LISTEN_EBAD_PARAM
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_getTypeVersion
                (JNIEnv *env, jclass clazz,      // ListenSoundModel
                 jobject soundModel,             // ByteBuffer
                 jobject soundModelInfo)         // ListenTypes$SoundModelInfo
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getTypeVersion entered");
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_model_type           model;
   listen_sound_model_info     modelInfo;
	listen_model_enum           type;	// model type: Keyword, User, TargetSound
	uint32_t                    version;	// model version/
	uint32_t                    size;	// total size of the model: header + payload size
   jclass                      smInfoClazz = NULL;
   jfieldID                    smTypeId = NULL;
   jfieldID                    smVersionId = NULL;
   jfieldID                    smSizeId = NULL;

   if ( (NULL == soundModel) || (NULL == soundModelInfo)  ) {
      ALOGE("_getTypeVersion: ERROR Null ptr passed to ListenSoundModel.getTypeVersion");
      return (jint)LISTEN_EBAD_PARAM;
   }
   // extract data and size from SoundModel jobject & and call query
   eStatus = getSMFromByteBuff(env, soundModel, &model.size, &model.data, &modelInfo);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGV("_ListenSoundModel_getTypeVersion getSMFromByteBuff returns error %d", eStatus);
      goto cleanup_for_get_type_version;
   }

   // fill structure after calling Query...
   // put fields into ListenTypes$SoundModelInfo jobject
   smInfoClazz = env->FindClass("com/qualcomm/listen/ListenTypes$SoundModelInfo");
   if (NULL == smInfoClazz) {
      ALOGE("_getTypeVersion: ERROR SoundModelInfo class not found");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_get_type_version;
   }
   // push fields from modelInfo acquire from LSMLib query() into soundModelInfo
   smTypeId = env->GetFieldID(smInfoClazz, "type", "I");
   smVersionId  = env->GetFieldID(smInfoClazz, "version", "I");
   smSizeId  = env->GetFieldID(smInfoClazz, "size", "I");
   if ( (NULL==smTypeId) || (NULL==smVersionId) || (NULL==smSizeId)) {
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_get_type_version;
   }

   if (modelInfo.type == 4) {
      // SVA 2.0 VoiceWakeup_MultiKeyword model
      version = VERSION_0200;
#ifdef SVA1_SUPPORTED
   } else if ( (modelInfo.type == 1) || (modelInfo.type == 2) ) {
      // SVA 1.0 VoiceWakeup_Keyword or VoiceWakeup_UserKeyword model
      version = VERSION_0100;
#endif
   } else {
      ALOGE("_getTypeVersion: type %d is unsupported", modelInfo.type);
      eStatus = LISTEN_EUNSUPPORTED_SOUNDMODEL;
      goto cleanup_for_get_type_version;
   }
   ALOGV("_getTypeVersion: SetIntField type");
   env->SetIntField(soundModelInfo, smTypeId, (jint)SVA_SOUNDMODEL_TYPE);
   ALOGV("_getTypeVersion: SetIntField version");
   env->SetIntField(soundModelInfo, smVersionId, (jint)version);
   ALOGV("_getTypeVersion: SetIntField size");
   env->SetIntField(soundModelInfo, smSizeId, modelInfo.size);

cleanup_for_get_type_version:
   if (model.data)
       free((void *)model.data);

   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getTypeVersion returns %d", (jint)eStatus);
   return (jint)eStatus;
}


/*
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    getInfoV1
 * Signature: (Ljava/nio/ByteBuffer;Lcom/qualcomm/listen/ListenTypes$SVASoundModelInfo;)I
 *
 * Parsers information from SVA SoundModel.
 * Depending on the SVA SM version, SoundModel::getSMInfoV1 or ::getSMInfoV2 called.
 * This method assumes that soundModelInfo fields common to both SVA 1.0 and 2.0
 * have already been filled by the calling method.
 *
 * Param [in]   env - ptr to JNIEnv
 * Param [in]   clazz - java class (not an object instance) of this static method
 * Param [in]   soundModel - SoundModel to query
 * Param [in]   soundModelInfo - ref to ListenTypes$SVASoundModelInfo instance
 * Return - status - LISTEN_SUCCESS or LISTEN_EBAD_PARAM
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_getInfo
                (JNIEnv *env, jclass clazz,      // ListenSoundModel
                 jobject soundModel,             // ByteBuffer
                 jobject soundModelInfo)         // ListenTypes$SVASoundModelInfo

{
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_model_type           model;
   listen_sound_model_info     modelInfo;

   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getInfo entered");
   if ( (NULL == soundModel) || (NULL == soundModelInfo)  ) {
      ALOGE("_getInfo: ERROR Null ptr passed to ListenSoundModel.getTypeVersion");
      return (jint)LISTEN_EBAD_PARAM;
   }
   // extract data and size from SoundModel jobject & and call query
   eStatus = getSMFromByteBuff(env, soundModel, &model.size, &model.data, &modelInfo);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("_getInfo: getSMFromByteBuff returns error %d", eStatus);
   }

   // get type version
   if (modelInfo.version == SM_VERSION_1p0) {
       eStatus =  getSMInfoV1(env,  clazz, &model, soundModelInfo);
   } else {
       eStatus =  getSMInfoV2(env,  clazz, &model, soundModelInfo);
   }
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("_getInfo: getSMInfoVx returns error %d", eStatus);
   }
   if (model.data)
      free((void *)model.data);

   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getInfo returns %d", (jint)eStatus);
   return (jint)eStatus;
}

/*
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    getSizeWhenMerged
 * Signature: ([Ljava/nio/ByteBuffer;)I
 *
 * Calculate the size of a soundmodel when array of input sound models are merged.
 * This can be called for SVA 2.0 and above SMs.
 *
 * Param [in]    soundModelsArr - array of SoundModels
 *
 * Return - size if value is greated than 0
 *          status otherwise
 *            LISTEN_EBAD_PARAM
 *            LISTEN_ESOUNDMODELS_WITH_SAME_KEYWORD_CANNOT_BE_MERGED
 *            LISTEN_ESOUNDMODELS_WITH_SAME_USER_KEYWORD_PAIR_CANNOT_BE_MERGED
 *            LISTEN_STATUS_EMAX_KEYWORDS_EXCEEDED
 *            LISTEN_EMAX_USERS_EXCEEDED
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenMerged
                (JNIEnv *env, jclass clazz,      // ListenSoundModel
                 jobjectArray soundModelsArr)    // array of ByteBuffer
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenMerged entered");
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   uint32_t                    sizeOfMergedModel = 0;
   listen_sound_model_info     modelInfo;
   jint                        modelSize = 0;
   if ( NULL == soundModelsArr ) {
      ALOGE("_getSizeWhenMerged: ERROR Null soundmodel Array passed to ListenSoundModel.getSizeWhenMerged");
      return LISTEN_EBAD_PARAM;
   }
   uint16_t numSoundModels = (uint16_t)env->GetArrayLength(soundModelsArr);
   jobject iByteBuffer;
   ALOGV("_getSizeWhenMerged array contains %d SMs", numSoundModels);
   listen_model_type   inputModels[numSoundModels];
   listen_model_type * pInputModels[numSoundModels];
   memset(inputModels, 0, (int)(sizeof(listen_model_type) * numSoundModels)); // zero out data pts and sizes
   for (int i=0; i < numSoundModels; i++) {
      pInputModels[i] = &inputModels[i];
      // extract individual sound model from ByteBuffer and store into array of listen_model_type
      // to pass to LSMLib
      iByteBuffer = env->GetObjectArrayElement(soundModelsArr, i );
      ALOGV("_getSizeWhenMerged SM[%d] = %p", i, iByteBuffer);
      if (NULL == iByteBuffer) {
         ALOGE("_getSizeWhenMerged: ERROR Null soundmodel passed as array element");
         eStatus = LISTEN_EBAD_PARAM;
         goto clean_up_getSizeWhenMerged;
      }

      // extract data and size from SoundModel jobject & and call query
      eStatus = getSMFromByteBuff(env, iByteBuffer, &inputModels[i].size, &inputModels[i].data, &modelInfo);
      if (LISTEN_SUCCESS != eStatus) {
           ALOGV("_getSizeWhenMerged getSMFromByteBuff returns error %d", eStatus);
           goto clean_up_getSizeWhenMerged;
      }
   }

   status = ListenSoundModelLibV2::getMergedModelSize(numSoundModels, pInputModels, &sizeOfMergedModel );
   if (kSucess != status) {
      ALOGE("_getSizeWhenMerged: ERROR getMergedModelSize() status %d", (int)status);
      MAP_ERR(status, eStatus);
   } else {
      modelSize = (jint)sizeOfMergedModel;
   }

clean_up_getSizeWhenMerged:
   for (int i=0; i < numSoundModels; i++) {
      if (inputModels[i].data)
          free ((void *)inputModels[i].data);
   }

   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenMerged returns error %d", eStatus);
      return eStatus;
   } else {
      ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeWhenMerged returns %d", (int)modelSize);
      return modelSize;
   }
}

/*
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    merge
 * Signature: ([Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;)I
 *
 * Merges a number of SoundModels into a single SM.
 * SM can contain any mixture of KW-only, UserTrained,... model data.
 * This can be called for SVA 2.0 and above SMs.
 *
 * Param [in]    soundModelsArr - array of SoundModels to be merged
 * Param [out]   mergedModel - output merged sound model
 *
 * Return - status
 *            LISTEN_SUCCESS
 *            LISTEN_EBAD_PARAM
 *            LISTEN_ESOUNDMODELS_WITH_SAME_KEYWORD_CANNOT_BE_MERGED
 *            LISTEN_ESOUNDMODELS_WITH_SAME_USER_KEYWORD_PAIR_CANNOT_BE_MERGED
 *            LISTEN_STATUS_EMAX_KEYWORDS_EXCEEDED
 *            LISTEN_EMAX_USERS_EXCEEDED
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_merge
                (JNIEnv *env, jclass clazz,      // ListenSoundModel
                 jobjectArray soundModelsArr,    // array of ByteBuffer
                 jobject      mergedModel)       // ByteBuffer
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_merge entered");
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   uint32_t                    sizeOfMergedModel = 0;
   listen_sound_model_info     modelInfo;
   jint                        modelSize = 0;
   jobject                     iByteBuffer;

   if ( (NULL == soundModelsArr) ||  (NULL == mergedModel)) {
      ALOGE("_merge: ERROR Null soundmodel Array passed to func");
      return (jint)LISTEN_EBAD_PARAM;
   }
   uint16_t                    numSoundModels;
   numSoundModels = (uint16_t)env->GetArrayLength(soundModelsArr);
   listen_model_type           inputModels[numSoundModels];
   listen_model_type *         pInputModels[numSoundModels];
   listen_model_type           outputModel;

   outputModel.data = NULL;

   // prepare input model data for LSMLib call
   for (int i=0; i < numSoundModels; i++) {
      pInputModels[i] = &inputModels[i];
      // extract individual sound model from ByteBuffer and store into array of listen_model_type
      // to pass to LSMLib
      iByteBuffer = env->GetObjectArrayElement(soundModelsArr, i );

      // extract data and size from SoundModel jobject & and call query
      eStatus = getSMFromByteBuff(env, iByteBuffer, &inputModels[i].size, &inputModels[i].data, &modelInfo);
      if (LISTEN_SUCCESS != eStatus) {
           ALOGV("_merge getSMFromByteBuff returns error %d", eStatus);
           goto clean_up_merged;
      }
   }

   status = ListenSoundModelLibV2::getMergedModelSize(numSoundModels, pInputModels, &sizeOfMergedModel );
   if (kSucess != status) {
      ALOGE("_merge: ERROR getMergedModelSize() status %d", (int)status);
      MAP_ERR(status, eStatus);
      modelSize = 0;
      goto clean_up_merged;
   } else {
      modelSize = (jint)sizeOfMergedModel;
   }

   // should check capacity of mergedModel byteBuffer before length before doing Put !
   // verify the mergedModel ByteBuffer is big enough
   eStatus = checkByteBufferCapacity(env, modelSize, mergedModel);
   if ( eStatus != LISTEN_SUCCESS) {
      ALOGE("_merge: ERROR checkByteBufferCapacity() failed with status %d", eStatus);
      goto clean_up_merged;
   }

   // prepare input model data for LSMLib call
   // allocate
   ALOGV("Allocate memory for output parameter userKeyword Model, max size %d", modelSize);
   outputModel.size = modelSize;
   outputModel.data = (uint8_t *)malloc(modelSize);
   if (NULL == outputModel.data) {
      ALOGE("_merge: ERROR malloc'ing outputModel.data failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto clean_up_merged;
   }
   ALOGV("ptr to userKeyword Model memory is %p", outputModel.data);

   status = ListenSoundModelLibV2::mergeModels(numSoundModels, pInputModels, &outputModel );
   if (kSucess != status) {
      ALOGE("_merge: ERROR mergeModels() failed status %d", (int)status);
      MAP_ERR(status, eStatus);
      goto clean_up_merged;
   } else {
      ALOGD("_merge: mergeModels() returned success");
   }

   eStatus = putSoundModelInByteBuffer(env, mergedModel, outputModel.size, outputModel.data);
   if ( eStatus != LISTEN_SUCCESS) {
      ALOGE("_merge: ERROR putSoundModelInByteBuffer() failed with %d", (int)eStatus );
      goto clean_up_merged;
   }

clean_up_merged:
   if (NULL != outputModel.data) {
      free((void *)outputModel.data);
   }
   for (int i=0; i < numSoundModels; i++) {
      if (inputModels[i].data) {
         free((void *)inputModels[i].data);
      }
   }

   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_merge returns %d", (int)eStatus);
   return (jint)eStatus;
}


/*
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    getSizeAfterDelete
 * Signature: (Ljava/nio/ByteBuffer;Ljava/lang/String;Ljava/lang/String;)I
 *
 * Get the size required to hold sound model that would be created by
 * deleteFromModel() executed for a keyword, a user, or a user+keyword pair
 * This can be called for SVA 2.0 and above SMs.
 *
 * Param [in] inputByteBuff - sound model to have data deleted from
 * Param [in] keyword - string of keyword to be deleted
 * Param [in] user - string of user to be deleted
 *
 * Return - size of model that would be returned by DeleteData() after data is deleted
 *             using the same parameters
 *          negative return value indicates error
 *             LISTEN_EBAD_PARAM
 *             LISTEN_EKEYWORD_NOT_IN_SOUNDMODEL
 *             LISTEN_EUSER_NOT_IN_SOUNDMODEL
 *             LISTEN_EKEYWORD_USER_PAIR_NOT_IN_SOUNDMODEL
 *             LISTEN_ECANNOT_DELETE_LAST_KEYWORD
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_getSizeAfterDelete
                (JNIEnv *env, jclass clazz,      // ListenSoundModel
                 jobject         inputByteBuff,     // ByteBuffer
                 jstring         keyword,
                 jstring         user)
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeAfterDelete entered");
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;

   listen_model_type           model;
   listen_sound_model_info     modelInfo;
   listen_sound_model_header	 modelHeader;
   uint32_t					       nOutputModelSize = 0;
   jint                        modelSize = 0;
   bool                        isInModel = false;
   char *                      keywordStr = NULL;
   char *                      userStr = NULL;

   if ( NULL == inputByteBuff )  {
      ALOGE("_getSizeAfterDelete: ERROR Null ByteBuffer ptr as input");
      return (jint)LISTEN_EBAD_PARAM;
   }
   if ( (NULL == keyword) && (NULL == user) ) {
      ALOGE("_getSizeAfterDelete: ERROR keyword and user strings can't both be null");
      return (jint)LISTEN_EBAD_PARAM;
   }
   // extract data and size from SoundModel jobject & and call query
   eStatus = getSMFromByteBuff(env, inputByteBuff, &model.size, &model.data, &modelInfo);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGV("_getSizeAfterDelete getSMFromByteBuff returns error %d", eStatus);
      return eStatus;
   }

   keywordStr = (char *)env->GetStringUTFChars(keyword, 0);
   userStr = (char *)env->GetStringUTFChars(user, 0);
   ALOGD("_getSizeAfterDelete: KeywordString %s, UserString %s", keywordStr, userStr);

   eStatus = isKeywordUserInModel(&model, keywordStr, userStr);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("_getSizeAfterDelete: ERROR either %s or %s are not present in SM",
                  keywordStr, userStr);
      goto cleanup_for_get_size_after_delete;
   }

   // should not allow keyword to be deleted if there is only one keyword
   //
   memset(&modelHeader, 0, sizeof(listen_sound_model_header));
   status = ListenSoundModelLibV2::getSoundModelHeader(&model, &modelHeader);
   if (kSucess != status) {
      ALOGE("_getSizeAfterDelete: ERROR getSoundModelHeader() failed status %d",
           (int)status);
      MAP_ERR(status, eStatus);
      goto cleanup_for_get_size_after_delete;
   }

   // should not allow keyword to be deleted if there is only one keyword
   if ( (modelHeader.numKeywords <= 1) && (NULL == user) ) {
      ALOGE("_getSizeAfterDelete: ERROR keyword can not be deleted from sound model with only one keyword");
      eStatus = LISTEN_EFAILURE;  // don't have a specific error for this
      goto cleanup_for_get_size_after_delete;
   }

   // call LSMLib function
   status = ListenSoundModelLibV2::getSizeAfterDeleting(&model,
               (keywordId_t)keywordStr, (userId_t)userStr, &nOutputModelSize);
   if (kSucess != status) {
      ALOGE("_getSizeAfterDelete: ERROR getSizeAfterDelete() failed status %d", (int)status);
      MAP_ERR(status, eStatus);
   } else {
      modelSize = (jint)nOutputModelSize;
   }

cleanup_for_get_size_after_delete:
   if (model.data)
      free ((void *)(model.data));
   if (keywordStr != NULL)
       env->ReleaseStringUTFChars(keyword, keywordStr);
   if (userStr != NULL)
       env->ReleaseStringUTFChars(user, userStr);

   status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
   if (kSucess != status) {
      ALOGE("_getSizeAfterDelete: ERROR releaseSoundModelHeader() failed status %d", (int)status);
      MAP_ERR(status, eStatus);
   }

   if (LISTEN_SUCCESS != eStatus) {
      ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeAfterDelete returns %d", (int)modelSize);
      return eStatus;
   } else {
      ALOGV("Java_com_qualcomm_listen_ListenSoundModel_getSizeAfterDelete returns %d", (int)modelSize);
      return modelSize;
   }
}

/*
 * Class:     com_qualcomm_listen_ListenSoundModel
 * Method:    deleteData
 * Signature: (Ljava/nio/ByteBuffer;Ljava/lang/String;Ljava/lang/String;
 *             Ljava/nio/ByteBuffer;)I
 * Deletes specific data from a given SoundModel
 * This can be called for SVA 2.0 and above SMs.
 *
 * Return a new sound model after removing data from a given SM for one of the following:
 *    a keyword,
 *    a user, or
 *    a specific user+keyword pair depending
 * on keywordPhrase and userName.
 *
 * If keywordPhrase is non-null, but userName is null then all data associated
 * with a particular keyword is removed.
 *
 * If keywordPhrase is null, but userName is non-null then all data associated
 * with a particular user is removed
 *
 * If both keywordPhrase & userName are non-null then all data associated
 * with a particular user+keyword pair is removed
 *
 * Param [in] inputByteBuff - sound model data is to be removed from
 * Param [in] keyword - string of keyword to be deleted
 * Param [in] user - string of user to be deleted
 * Param [out] outSoundModel - new output sound delete after data deleted
 *
 * Return - status - LISTEN_SUCCESS or LISTEN_EBAD_PARAM
 *             LISTEN_SUCCESS
 *             LISTEN_EBAD_PARAM
 *             LISTEN_EKEYWORD_NOT_IN_SOUNDMODEL
 *             LISTEN_EUSER_NOT_IN_SOUNDMODEL
 *             LISTEN_EKEYWORD_USER_PAIR_NOT_IN_SOUNDMODEL
 *             LISTEN_ECANNOT_DELETE_LAST_KEYWORD
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenSoundModel_deleteData
                (JNIEnv *env, jclass clazz,       // ListenSoundModel
                 jobject         inputByteBuff,   // ByteBuffer
                 jstring         keyword,
                 jstring         user,
                 jobject         outSoundModel )  // ByteBuffer
{
   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_deleteData entered");
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   bool                        isInModel = false;
   uint32_t                    sizeOfDeletedModel = 0;
   listen_sound_model_info     modelInfo;
   listen_sound_model_header	 modelHeader;
   jint                        modelSize = 0;
   jobject                     iByteBuffer;
   listen_model_type           inputModel;
   listen_model_type           outputModel;
   char *                      keywordChar = NULL;
   char *                      userChar = NULL;

   if ( NULL == inputByteBuff )  {
      ALOGE("_deleteData: ERROR Null ByteBuffer ptr as input");
      return LISTEN_EBAD_PARAM;
   }
   if ( (NULL == keyword) && (NULL == user) ) {
      ALOGE("_deleteData: ERROR keyword and user strings can't both be null");
      return LISTEN_EBAD_PARAM;
   }
   // extract data and size from SoundModel jobject & and call query
   eStatus = getSMFromByteBuff(env, inputByteBuff, &inputModel.size, &inputModel.data, &modelInfo);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("_deleteData getSMFromByteBuff returns error %d", eStatus);
      goto cleanup_delete_data;
   }

   keywordChar = (char *)env->GetStringUTFChars(keyword, 0);
   userChar = (char *)env->GetStringUTFChars(user, 0);
   outputModel.data = NULL;

   ALOGD("_deleteData: call isKeywordUserInModel for keyword,user %s,%s", keywordChar, userChar);
   eStatus = isKeywordUserInModel(&inputModel, keywordChar, userChar);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("_deleteData: ERROR either %s or %s could not be found in SM",
                  keywordChar, userChar);
      goto cleanup_delete_data;
   }

   memset(&modelHeader, 0, sizeof(listen_sound_model_header));
   status = ListenSoundModelLibV2::getSoundModelHeader(&inputModel, &modelHeader);
   if (kSucess != status) {
      ALOGE("_deleteData: ERROR getSoundModelHeader() failed status %d", (int)status);
      MAP_ERR(status, eStatus);
      goto cleanup_delete_data;
   }
   ALOGD("_deleteData: getSoundModelHeader returns counts %d,%d",
                 modelHeader.numKeywords, modelHeader.numUsers);

   if ( (modelHeader.numKeywords <= 1) && (NULL == user) ) {
      ALOGE("_deleteData: ERROR keyword can not be deleted from sound model with only one keyword");
      eStatus = LISTEN_ECANNOT_DELETE_LAST_KEYWORD;
      goto cleanup_delete_data;
   }

   ALOGD("_deleteData: call getSizeAfterDeleting ");
   status = ListenSoundModelLibV2::getSizeAfterDeleting(&inputModel,
               (keywordId_t)keywordChar, (userId_t)userChar, &sizeOfDeletedModel);
   if (kSucess != status) {
      ALOGE("_deleteData: ERROR getMergedModelSize() failed with status %d", (int)status);
      MAP_ERR(status, eStatus);
      goto cleanup_delete_data;
   } else {
      ALOGD("_deleteData: getSizeAfterDeleting successful");
      modelSize = (jint)sizeOfDeletedModel;
   }

   // should check capacity of mergedModel byteBuffer before length before doing Put !!!!!
   // verify the mergedModel ByteBuffer is big enough
   ALOGD("_deleteData: call checkByteBufferCapacity ");
   eStatus = checkByteBufferCapacity(env, sizeOfDeletedModel, outSoundModel);
   if ( eStatus != LISTEN_SUCCESS) {
      ALOGE("_deleteData: ERROR checkByteBufferCapacity() failed with status %d", eStatus);
      goto cleanup_delete_data;
   }

   // prepare input model data for LSMLib call
   // allocate
   ALOGV("Allocate memory for output parameter userKeyword Model, max size %d", modelSize);
   outputModel.size = sizeOfDeletedModel;
   outputModel.data = (uint8_t *)malloc(sizeOfDeletedModel);
   if (NULL == outputModel.data) {
      ALOGE("_deleteData: ERROR malloc'ing outputModel.data failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_delete_data;
   }
   ALOGV("ptr to userKeyword Model memory is %p", outputModel.data);

   // call LSMLib function
   status = ListenSoundModelLibV2::deleteFromModel(&inputModel,
               (keywordId_t)keywordChar, (userId_t)userChar, &outputModel);
   if (kSucess != status) {
      ALOGE("_deleteData: ERROR merge() failed with status %d", (int)status);
      MAP_ERR(status, eStatus);
      goto cleanup_delete_data;
   }

   eStatus = putSoundModelInByteBuffer(env, outSoundModel, outputModel.size, outputModel.data);
   if ( eStatus != LISTEN_SUCCESS) {
      ALOGE("_deleteData: ERROR putSoundModelInByteBuffer() failed with %d", (int)eStatus );
      goto cleanup_delete_data;
   }

cleanup_delete_data:
   if (outputModel.data)
      free((void *)outputModel.data);
   if (inputModel.data)
      free((void *)inputModel.data);
   if (keywordChar)
       env->ReleaseStringUTFChars(keyword, keywordChar);
   if (userChar)
       env->ReleaseStringUTFChars(user, userChar);

   status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
   if (kSucess != status) {
      ALOGE("_getSizeAfterDelete: ERROR releaseSoundModelHeader() failed status %d", (int)status);
      MAP_ERR(status, eStatus);
   }

   ALOGV("Java_com_qualcomm_listen_ListenSoundModel_deleteData returns %d", eStatus);
   return (jint)eStatus;
}

#ifdef __cplusplus
}
#endif // __cplusplus

}; // namespace android
