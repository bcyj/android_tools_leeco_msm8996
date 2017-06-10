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
 * SoundModel helper functions used by ListenSoundModel JNI functions
 *
 * Listen SoundModel is a collection of data that defines the
 *     characteristics of the sound that is to be detected.
 * The detection algorithm uses the registered sound model
 *     to decide if the sound it is given to evaluate matches
 *     these sound model characteristics.
 *
 * None of these methods call Native ListenReceiver C++ methods
 */
// enable verbose logs
#define LOG_NDEBUG  0
#define LOG_TAG "SoundModel-helper-JNI"
#include <utils/Log.h>

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#include <jni.h>
#include "JNIHelp.h"  // for NELEM sizeof()/sizeof(x[0])

#include <ListenSoundModelLib_v2.h>
#include <ListenSoundModelLib.h>
#include <SoundModel.h>

namespace android {

/*
 * Check validity of SoundModel
 *
 * param [in]  pKeywordModel - ptr to LSMLib KeywordModel object
 * param [out] pModelInfo - ptr to SoundModel info struct
 *
 * Return - status
 */
listen_status_enum_t checkSoundModel( listen_model_type       * pKeywordModel,
                                      listen_sound_model_info * pModelInfo)
{
   listen_status_enum   status = kSucess;
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   ALOGV("checkSoundModel entered");
   // Call ListenSoundModel library function to get info about soundmodel

   ALOGD("checkSoundModel: NEW querySoundModel() called");
   // querySoundModel() does not zero out elements of listen_sound_model_info
   memset(pModelInfo, 0, sizeof(listen_sound_model_info));
   status = ListenSoundModelLibV2::querySoundModel(pKeywordModel, pModelInfo);

   if (kSucess != status) {
      ALOGE("checkSoundModel: ERROR querySoundModel() failed with %d", status);
      MAP_ERR(status, eStatus);
   } else {
      ALOGI("querySoundModel() SM returned type %d: (Keyword=1, User=2, Mult-KW=4)",
         (int)pModelInfo->type);
      ALOGI("                                returned version %d, size %d",
         pModelInfo->version, pModelInfo->size);
      // ListenSoundModelLib code crashes when old SM used - avoid using older versions of SM
      if (pModelInfo->version < 4) {
         ALOGE("checkSoundModel: ERROR soundModel Version %d is NOT supported", pModelInfo->version);
         eStatus = LISTEN_EBAD_PARAM;
      }
      // This code explicitly supports Keyword SoundModels types:
      if ( !( (pModelInfo->type == kKeywordModel) ||
              (pModelInfo->type == kUserKeywordModel) ||
              (pModelInfo->type == kMultiUserKeywordModel) ) ) {
         ALOGE("checkSoundModel: ERROR soundModel Type %d is NOT supported", pModelInfo->type);
         eStatus = LISTEN_EBAD_PARAM;
      }
   }
   ALOGV("checkSoundModel returns %d", (int)eStatus);
   return eStatus;
}

/*
 * Gets SoundModel Data From ByteBuffer
 *
 * Given a Java ByteBuffer jobject, extracts the C++ soundModel data
 *
 * param [in]  env - ptr to JNI getSMFromByteBuff
 * param [in]  byteBuffObj - java ByteBuffer object
 * param [out] pSize - size of SoundModel contained in byteBuffer
 * param [out] ppData - ptr to byte array containing SoundModel data
 * param [out] pModelInfo - ptr to struct containing SoundModel info from header
 *
 * Return - status
 */
// %%% TODO modelInfo should be taken out of getSMFromByteBuff
// %%%      allow this function to execute ONLY if AppType == SVA and modelInfo returns
listen_status_enum_t getSMFromByteBuff(JNIEnv *env,
                                       jobject byteBuffObj,
                                       uint32_t *pSize, uint8_t **ppData,
                                       listen_sound_model_info *pModelInfo)
{
   ALOGV("getSMFromByteBuff entered");
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   // initialize output parameter values
    jclass byteBuffClazz = env->FindClass("java/nio/ByteBuffer");
   if (NULL == byteBuffClazz) {
      ALOGE("FindClass java/nio/ByteBuffer failed");
      return LISTEN_EFAILURE;
   }
   if (NULL == byteBuffObj) {
      ALOGE("byteBuffObj NULL");
      return LISTEN_EBAD_PARAM;
   }
   if (NULL == pSize ||  NULL == ppData ) {
      ALOGE("byteBuffObj NULL");
      return LISTEN_EBAD_PARAM;
   }
   *pSize = 0;
   *ppData = (uint8_t *)NULL;

   // get the ByteBuffer methods we need to call to Get the byte array
   jmethodID hasArrId   = env->GetMethodID(byteBuffClazz, "hasArray", "()Z");
   jmethodID getArrId   = env->GetMethodID(byteBuffClazz, "array", "()[B");
   if ( (NULL == hasArrId) || (NULL == getArrId) ) {
      ALOGE("getSMFromByteBuff: ERROR GetFieldId ByteBuffer class failed");
      return LISTEN_EFAILURE;
   }

   ALOGV("Call ByteBuffer.hasArray()...");
   bool bBuffHasArr = env->CallBooleanMethod(byteBuffObj, hasArrId);
   if (bBuffHasArr) {
       ALOGV("... returns true");
   } else  {
       ALOGE("getSMFromByteBuff: ERROR ... returns false - no array");
       return LISTEN_EBAD_PARAM;
   }

   jmethodID offsetId   = env->GetMethodID(byteBuffClazz, "arrayOffset", "()I");
   if ( (NULL == offsetId) ) {
     ALOGE("getSMFromByteBuff: ERROR additional GetFieldId ByteBuffer class failed");
     return LISTEN_EFAILURE;
   }
   ALOGV("Call CallObjectMethod -> ByteBuffer.arrayOffset()...");
   jint iOffset = env->CallIntMethod(byteBuffObj, offsetId);
   ALOGV("ByteBuffer.arrayOffset() returned %d", (int)iOffset);

   ALOGV("Call CallObjectMethod -> ByteBuffer.array()...");
   // extract the byte array from Java ByteBuffer into store as C++ ptr to byte[]
   jobject jArrayObj = env->CallObjectMethod(byteBuffObj, getArrId);
   if (NULL == jArrayObj) {
       ALOGE("getSMFromByteBuff: ERROR ... array return NULL");
       return LISTEN_EBAD_PARAM;
   }
   ALOGV("cast jArrayObj = %p to byteArray", jArrayObj);
   jbyteArray byteArray = (jbyteArray)jArrayObj;
   ALOGV("GetArrayLength");
   jsize arrayLen = env->GetArrayLength(byteArray);
   *pSize = (uint32_t)arrayLen;
   ALOGV("Allocate byte array of size %d",*pSize);
   *ppData = (uint8_t *)malloc(*pSize);
   ALOGV("GetByteArrayRegion w/ byteArray=%p, len=%d, into ppData=%p",
      byteArray, arrayLen, *ppData );
   env->GetByteArrayRegion(byteArray, (jsize)iOffset, arrayLen-iOffset, (jbyte *)*ppData);
   ALOGV("... ByteBuffer.array() returns size, data (%d, %p)", *pSize, *ppData);

   // Call ListenSoundModel library function to get info about soundmodel
   listen_model_type    keywordModel;
   keywordModel.size = *pSize;
   keywordModel.data = *ppData;
   eStatus = checkSoundModel(&keywordModel, pModelInfo);
   if (LISTEN_SUCCESS != eStatus) {
       ALOGE("ERROR checkSoundModel() failed");
       if (*ppData != NULL)  {
          free((void *)*ppData);
          *ppData = (uint8_t *)NULL;
       }
       return eStatus;
   }
   ALOGV("getSMFromByteBuff returns type = %d, version = %d, size = %d",
              pModelInfo->type, pModelInfo->version, pModelInfo->size);
   return LISTEN_SUCCESS;
}



/*
 * Gets UserRecording Data From ShortBuffer
 *
 * Given a Java ShortBuffer jobject, extracts the C++ UserRecording data
 * Also calls ListenSoundModel library function to get info about soundmodel
 *
 * param [in]  env - ptr to JNI environment
 * param [in]  shortBuffObj - java ShortBuffer object
 * param [out] pNumSamples - size of UserRecording contained in shortBuffer
 * param [out] ppData - ptr to short array containing UserRecording data
 *
 * Return - status
 */
listen_status_enum_t getRecFromShortBuff(JNIEnv   *env,
                                         jobject  shortBuffObj,
                                         uint32_t *pNumSamples,
                                         int16_t  **ppData)
{
   ALOGV("getRecFromShortBuff entered");
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   // initialize output parameter values
   *pNumSamples = 0;
   *ppData = (int16_t *)NULL;
   jclass shortBuffClazz = env->FindClass("java/nio/ShortBuffer");
   if (shortBuffClazz == NULL) {
      ALOGE("FindClass java/nio/ShortBuffer failed");
      return LISTEN_EFAILURE;
   }
   // get the ShortBuffer methods we need to call to Get the short array
   jmethodID hasArrId   = env->GetMethodID(shortBuffClazz, "hasArray", "()Z");
   jmethodID getArrId   = env->GetMethodID(shortBuffClazz, "array", "()[S");
   if ( (NULL == hasArrId) || (NULL == getArrId) ) {
      ALOGE("getRecFromShortBuff: ERROR GetMethodID for ShortBuffer failed");
      return LISTEN_EFAILURE;
   }

   ALOGV("Call ShortBuffer.hasArray()...");
   bool bBuffHasArr = env->CallBooleanMethod(shortBuffObj, hasArrId);
   if (bBuffHasArr) {
       ALOGV("... returns true");
   } else  {
       ALOGE("getRecFromShortBuff: ERROR ... returns false - no array");
       return LISTEN_EBAD_PARAM;
   }
   ALOGV("Call ShortBuffer.array()...");
   // extract the short array from Java ShortBuffer into store as C++ ptr to short[]
   jobject jArrayObj = env->CallObjectMethod(shortBuffObj, getArrId);
   ALOGV("cast jArrayObj = %p to shortArray", jArrayObj);
   jshortArray shortArray = (jshortArray)jArrayObj;
   ALOGV("GetArrayLength");
   jsize arrayLen = env->GetArrayLength(shortArray);
   *pNumSamples = arrayLen;

   ALOGV("Allocate short array of size %d",arrayLen);
   *ppData = (int16_t *)malloc((arrayLen*2));
   ALOGV("GetShortArrayRegion w/ shortArray=%p, len=%d, into ppData=%p",
      shortArray, arrayLen, *ppData );
   env->GetShortArrayRegion(shortArray, (jsize)0, arrayLen, (jshort *)*ppData);
   ALOGV("... ShortBuffer.array() returns size, data (%d, %p)", *pNumSamples, *ppData);

   ALOGV("getRecFromShortBuff returns");
   return LISTEN_SUCCESS;
}

/*
 * Gets size Capacity of ByteBuffer object
 *
 * param [in] smObj - ByteBuffer object to get size for
 * param [out] pByteBuffSize - pointer to capacity of Byte Buffer
 *
 * Return - status
 */
listen_status_enum_t getByteBufferCapacity(JNIEnv *env,
                                             jobject smObj,
                                             uint32_t *pByteBuffSize)
{
   listen_status_enum_t        retStatus = LISTEN_SUCCESS;
   jclass                      byteBuffClazz;
   jmethodID                   capacityId;
   int                         capacity;

   ALOGV("getByteBufferCapacity entered");
   if (pByteBuffSize == NULL) {
       ALOGE("getByteBufferCapacity: ERROR pByteBuffSize is null");
       retStatus = LISTEN_EBAD_PARAM;
       return retStatus;
   }
   byteBuffClazz = env->FindClass("java/nio/ByteBuffer");
   if (byteBuffClazz == NULL) {
       ALOGE("getByteBufferCapacity: ERROR FindClass java/nio/ByteBuffer failed");
       retStatus = LISTEN_EFAILURE;
       return retStatus;
   }
   // get the ByteBuffer methods we need to call to get the byte array
   capacityId = env->GetMethodID(byteBuffClazz, "capacity", "()I");
   if (NULL == capacityId) {
       ALOGE("getByteBufferCapacity: ERROR GetMethodId capacity of ByteBuffer class failed");
       retStatus = LISTEN_EFAILURE;
       return retStatus;
   }
   ALOGV("getByteBufferCapacity: Call ByteBuffer.capacity()");
   capacity = env->CallIntMethod(smObj, capacityId);
   *pByteBuffSize = (uint32_t)capacity;
   ALOGV("getByteBufferCapacity: ByteBuffer.capacity() returned %d", *pByteBuffSize);

   ALOGV("getByteBufferCapacity returns status %d", (int)retStatus);
   return retStatus;
}
/*
 * Checks ByteBuffer Capacity against required size
 *
 * Verifies that capacity of input ByteBuffer is not less than the
 * size this ByteBuffer needs to be.
 *
 * param [in] reqdSize - size the new Byte Buffer needs to be
 * param [in] smObj - ByteBuffer object that needs to be this size
 *
 * Return - status
 */
listen_status_enum_t checkByteBufferCapacity(JNIEnv *env,
                                             uint32_t reqdSize,
                                             jobject smObj)
{
   listen_status_enum_t        retStatus = LISTEN_SUCCESS;
   jclass                      byteBuffClazz;
   jmethodID                   capacityId;
   uint32_t                    capacity = 0;

   ALOGV("checkByteBufferCapacity entered");
   retStatus = getByteBufferCapacity(env, smObj, &capacity);

   if (retStatus != LISTEN_SUCCESS) {
      ALOGE("checkByteBufferCapacity: ERROR could not get the byteBuffer capacity, check not done");
   } else {
      // test the capacity of ByteBuffer against the required size
      if (capacity < reqdSize) {
         ALOGE("checkByteBufferCapacity: ERROR output byteBuffer not large enough to hold %d bytes of SM",reqdSize);
         retStatus = LISTEN_EBAD_PARAM;
      }
   }
   ALOGV("checkByteBufferCapacity returns status %d", (int)retStatus);
   return retStatus;
}


/*
 * Puts SoundModel array byte buffer into Java ByteBuffer object
 *
 * Copies SoundModel data from pByteBuff into byte array within
 * byteBuffObj
 *
 * param [in] byteBuffObj - ByteBuffer object that byte buffer to be placed in
 * param [in] size - size the byte buffer
 * param [in] byteArr - ptr to byte array containing soundmodel
 *
 * Return - status
 */
listen_status_enum_t putSoundModelInByteBuffer(JNIEnv *  env,
                                               jobject   byteBuffObj,
                                               uint32_t  size,
                                               uint8_t * pByteBuff)
{
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   jclass                      byteBuffClazz;
   jmethodID                   putId;
   jsize                       smLen = 0;
   jbyteArray                  smByteArr = NULL;   // SM data copied into this

   ALOGV("putSoundModelInByteBuffer entered");
   // Put content of outputModel into ByteBuffer byteBuffObj
   byteBuffClazz = env->FindClass("java/nio/ByteBuffer");
   // get the ByteBuffer methods we need to call to Get the byte array
   putId = env->GetMethodID(byteBuffClazz, "put", "([B)Ljava/nio/ByteBuffer;");
   if ( NULL == putId ) {
     ALOGE("putSoundModelInByteBuffer: ERROR GetMethodID put for ByteBuffer class failed");
     eStatus = LISTEN_EFAILURE;
   }
   smLen = (jsize)size;
   smByteArr = env->NewByteArray(smLen);
   // byte array Inside ByteBuffer is GC'ed by Java
   ALOGV("putSoundModelInByteBuffer SetByteArrayRegion");
   env->SetByteArrayRegion(smByteArr, (jsize)0, smLen, (jbyte *)pByteBuff );

   ALOGV("putSoundModelInByteBuffer call put()");
   byteBuffObj = env->CallObjectMethod(byteBuffObj, putId, smByteArr);

   if (smByteArr)
      env->DeleteLocalRef(smByteArr);
   ALOGV("putSoundModelInByteBuffer returned");
   return eStatus;
}

/*
 * Tests if Keyword and/or User are in Model
 *
 * Gets the keyword and user names from SM.
 * Makes sure that keyword and user names strings are contained in SM.
 *
 * param [in] pModel - ptr to model structure
 * param [in] keyword - string containing keyword that should be in given model
 * param [in] user - string containing keyword that should be in given model
 *
 * Return - status
 *       if LISTEN_SUCCESS returned then both keyword and user are in Model
 */
listen_status_enum_t   isKeywordUserInModel(listen_model_type * pModel,
                             const char *        keywordStr,
                             const char *        userStr )
{
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   bool                        areInModel = false;
   listen_sound_model_header	 modelHeader;
   int                         i = 0;
   char ** keywordPhrases = NULL;
   char ** userNames = NULL;
   int userStrLen;
   int keywordStrLen;
   uint16_t numKWs;       /* total number of keywords  */
   uint16_t numUsers;      /* total number of users  */

   ALOGV("isKeywordUserInModel entered");
   if ( (NULL == keywordStr) && (NULL == userStr) ) {
       ALOGV("keyword and user string are both null");
       return LISTEN_EBAD_PARAM;
   }

   // call SVA 2.0 functions to get number of keywords, users, user pairs,...
   // then convert char strings to Unicode jstrings
   memset(&modelHeader, 0, sizeof(listen_sound_model_header));
   status = ListenSoundModelLibV2::getSoundModelHeader(pModel, &modelHeader);
   if (kSucess != status) {
      ALOGE("isKeywordUserInModel: ERROR getSoundModelHeader() failed");
      MAP_ERR(status, eStatus);
      goto cleanup_for_isKeywordUserInModel;
   }
   numKWs = modelHeader.numKeywords;       /* total number of keywords  */
   numUsers = modelHeader.numUsers;      /* total number of users  */
   if ( 0 == numKWs )  {
      ALOGE("isKeywordUserInModel: ERROR number of KWs is zero !" );
      eStatus = LISTEN_EUNSUPPORTED_SOUNDMODEL;
      goto cleanup_for_isKeywordUserInModel;
   }
   if ( (0 == numUsers) && (userStr != NULL) )  {
      ALOGE("isKeywordUserInModel: ERROR number of users is zero but User string is non-null" );
      eStatus = LISTEN_EUSER_NOT_IN_SOUNDMODEL;
      goto cleanup_for_isKeywordUserInModel;
   }
   if (keywordStr != NULL) {
      // get strings for Keyword and compare with give keyword string
      ALOGD("isKeywordUserInModel getKeywordPhrases ");
      eStatus = allocStrArr(numKWs, &keywordPhrases);
      if (eStatus != LISTEN_SUCCESS) {
         ALOGE("isKeywordUserInModel: ERROR could not alloc memory for String array");
         goto cleanup_for_isKeywordUserInModel;
      }
      status = ListenSoundModelLibV2::getKeywordPhrases(pModel, &numKWs, keywordPhrases);
      if (kSucess != status) {
         ALOGE("isKeywordUserInModel: ERROR getKeywordPhrases() failed");
         MAP_ERR(status, eStatus);
         goto cleanup_for_isKeywordUserInModel;
      }
      ALOGD("isKeywordUserInModel  getKeywordPhrase output %d KW strings", numKWs );
      keywordStrLen = strlen(keywordStr);
      for (i = 0; i < numKWs; i++) {
         // compare strings from getKeywordPhrases against keywordStr
         if (0 == strncmp(keywordPhrases[i], keywordStr, keywordStrLen)) {
            ALOGD("isKeywordUserInModel keyword string equals %d-th keyword", i );
            areInModel = true;
            break;
         }
      }
      if (i >= numKWs) {
         ALOGE("isKeywordUserInModel Keyword %s not found", keywordStr );
         eStatus = LISTEN_EKEYWORD_NOT_IN_SOUNDMODEL;
         goto cleanup_for_isKeywordUserInModel;
      }
   }

   if (userStr != NULL) {
      // get strings for Users and compare with given user string
      ALOGD("isKeywordUserInModel getKeywordPhrases ");
      eStatus = allocStrArr(numUsers, &userNames);
      if (eStatus != LISTEN_SUCCESS) {
         ALOGE("isKeywordUserInModel: ERROR could not alloc memory for String arrayu");
         goto cleanup_for_isKeywordUserInModel;
      }
      status = ListenSoundModelLibV2::getUserNames(pModel, &numUsers, userNames);
      if (kSucess != status) {
         ALOGE("isKeywordUserInModel: ERROR getUserNames() failed");
         MAP_ERR(status, eStatus);
         goto cleanup_for_isKeywordUserInModel;
      }
      ALOGD("isKeywordUserInModel  getUserNames output %d User strings", numUsers );
      userStrLen = strlen(userStr);
      for (i = 0; i < numUsers; i++) {
         // compare strings from getKeywordPhrases against keywordStr
         if (0 == strncmp(userNames[i], userStr, userStrLen)) {
            ALOGD("isKeywordUserInModel  user string equals %d-th keyword", i);
            areInModel = true;
            break;
         }
      }
      if (i >= numUsers) {
         ALOGE("isKeywordUserInModel Keyword %s not found", keywordStr );
         eStatus = LISTEN_EUSER_NOT_IN_SOUNDMODEL;
         goto cleanup_for_isKeywordUserInModel;
      }
   }
   ALOGV("isKeywordUserInModel returned areInModel = %d", (int)areInModel);

cleanup_for_isKeywordUserInModel:
   status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
   if (kSucess != status) {
      ALOGE("isKeywordUserInModel: ERROR releaseSoundModelHeader() failed with %d", status);
      // do not pass this error up to Java
   }
   freeStrArr(numKWs, keywordPhrases); // silently ignore errors
   freeStrArr(numUsers, userNames); // silently ignore errors
   ALOGV("isKeywordUserInModel returned status %d", (int)eStatus);
   return eStatus;
}

/*
 * Gets Keyword Index
 *
 * Gets list of keyword strings.
 * Compares input string against this list and output index of keyword that matches.
 *
 * param [in] pModel - ptr to model structure
 * param [in] keywordStr - string containing keyword that should be in given model
 * param [out] pKeywordIndex
 *
 * Return - status
 */
listen_status_enum_t getKeywordIndex(listen_model_type * pModel,
                                     const char *        keywordStr,
                                     uint16_t *          pKeywordIndex)
{
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   listen_sound_model_header	 modelHeader;
   uint16_t                    numKWs = 0;       /* total number of keywords  */
   bool                        bKWMatched = false;
   uint16_t                    i;
   char **                     keywordPhrases = NULL;

   // get number of keywords, users, user pairs,...
   memset(&modelHeader, 0, sizeof(listen_sound_model_header));
   status = ListenSoundModelLibV2::getSoundModelHeader(pModel, &modelHeader);
   if (kSucess != status) {
      ALOGE("getKeywordIndex: ERROR getSoundModelHeader() failed");
      MAP_ERR(status, eStatus);
      goto cleanup_for_getKWIndex;
   }
   numKWs = modelHeader.numKeywords;       /* total number of keywords  */
   if ( 0 == numKWs )  {
      ALOGE("getKeywordIndex: ERROR number of KWs is zero !" );
      eStatus = LISTEN_EBAD_PARAM;
      goto cleanup_for_getKWIndex;
   }
   if (keywordStr != NULL) {
      // get strings for Keyword and compare with give keyword string
      ALOGD("getKeywordIndex: getKeywordPhrases ");
      eStatus = allocStrArr(numKWs, &keywordPhrases);
      if (eStatus != LISTEN_SUCCESS) {
         ALOGE("getKeywordIndex: ERROR could not alloc memory for String arrayu");
         goto cleanup_for_getKWIndex;
      }
      status = ListenSoundModelLibV2::getKeywordPhrases(pModel, &numKWs, keywordPhrases);
      if (kSucess != status) {
         ALOGE("getKeywordIndex: ERROR getKeywordPhrases() failed");
         eStatus = LISTEN_EBAD_PARAM;
         goto cleanup_for_getKWIndex;
      }
      ALOGD("getKeywordIndex:  getKeywordPhrase output %d KW strings", numKWs );
      int keywordStrLen = strlen(keywordStr);
      for (i = 0; i < numKWs; i++) {
         // compare strings from getKeywordPhrases against keywordStr
         if (0 == strncmp(keywordPhrases[i], keywordStr, keywordStrLen)) {
            ALOGD("getKeywordIndex: keyword string equals %d-th keyword", i );
            bKWMatched = true;
            *pKeywordIndex = i;
            break;
         }
      }
      if (i >= numKWs) {
         ALOGE("getKeywordIndex: ERROR Keyword %s not found", keywordStr );
         eStatus = LISTEN_EKEYWORD_NOT_IN_SOUNDMODEL;
         goto cleanup_for_getKWIndex;
      }
   }

cleanup_for_getKWIndex:
   status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
   if (kSucess != status) {
      ALOGE("getKeywordIndex: ERROR releaseSoundModelHeader() failed with %d", status);
      // do not pass this error up to Java
   }
   ALOGV("getKeywordIndex returns %d", eStatus );
   return eStatus;
}


/*
 * Gets User Index
 *
 * Gets list of user strings.
 * Compares input string against this list and output index of user that matches.
 *
 * param [in] pModel - ptr to model structure
 * param [in] userStr - string containing user that should be in given model
 * param [out] pUserIndex
 *
 * Return - status
 */
listen_status_enum_t getUserIndex(listen_model_type * pModel,
                                  const char *        userStr,
                                  uint16_t *          pUserIndex)
{
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   listen_sound_model_header	 modelHeader;
   uint16_t                    numUsers = 0;       /* total number of users  */
   bool                        bUserMatched = false;
   uint16_t                    i;
   char **                     userNames = NULL;

   // get number of keywords, users, user pairs,...
   memset(&modelHeader, 0, sizeof(listen_sound_model_header));
   status = ListenSoundModelLibV2::getSoundModelHeader(pModel, &modelHeader);
   if (kSucess != status) {
      ALOGE("getUserIndex: ERROR getSoundModelHeader() failed");
      MAP_ERR(status, eStatus);
      goto cleanup_for_getUserIndex;
   }
   numUsers = modelHeader.numUsers;       /* total number of users  */
   if ( 0 == numUsers )  {
      ALOGE("getUserIndex: ERROR number of KWs is zero !" );
      eStatus = LISTEN_EBAD_PARAM;
      goto cleanup_for_getUserIndex;
   }
   if (userStr != NULL) {
      // get strings for user and compare with give user string
      ALOGD("getUserIndex: getUserNames ");
      eStatus = allocStrArr(numUsers, &userNames);
      if (eStatus != LISTEN_SUCCESS) {
         ALOGE("getUserIndex: ERROR could not alloc memory for String arrayu");
         goto cleanup_for_getUserIndex;
      }
      status = ListenSoundModelLibV2::getUserNames(pModel, &numUsers, userNames);
      if (kSucess != status) {
         ALOGE("getUserIndex: ERROR getUserNames() failed");
         MAP_ERR(status, eStatus);
         goto cleanup_for_getUserIndex;
      }
      ALOGD("getUserIndex: getUserNames output %d User strings", numUsers );
      int userStrLen = strlen(userStr);
      for (i = 0; i < numUsers; i++) {
         // compare strings from getUserNames against userStr
         if (0 == strncmp(userNames[i], userStr, userStrLen)) {
            ALOGD("getUserIndex: user string equals %d-th user", i );
            bUserMatched = true;
            *pUserIndex = i;
            break;
         }
      }
      if (i >= numUsers) {
         ALOGE("getUserIndex: ERROR User %s not found", userStr );
         eStatus = LISTEN_EUSER_NOT_IN_SOUNDMODEL;
         goto cleanup_for_getUserIndex;
      }
   }

cleanup_for_getUserIndex:
   status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
   if (kSucess != status) {
      ALOGE("getUserIndex: ERROR releaseSoundModelHeader() failed with %d", status);
      // do not pass this error up to Java
   }
   freeStrArr(numUsers, userNames); // silently ignore errors
   ALOGV("getUserIndex returns %d", eStatus );
   return eStatus;
}


/*
 * Gets active UserKeyword pair Index
 *
 * Gets list of keyword strings
 *    compare input keyword string against this list and output index of keyword that matches
 * Gets list of users strings
 *    compare input user string against this list and output index of user that matches
 * now compare the indices against the active pair truth table for SM:
 *   is if pair is active - error otherwise
 *   get the number order in this truth table for the given index pair
 *
 *
 * param [in] pModel - ptr to model structure
 * param [in] keywordStr - string containing keyword that should be in given model
 * param [in] userStr - string containing keyword that should be in given model
 * param [out] pPairIndex

 * Return - status
 */
listen_status_enum_t getUserKeywordPairIndex(listen_model_type * pModel,
                                             const char *        keywordStr,
                                             const char *        userStr,
                                             uint16_t *          pPairIndex )
{
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   listen_sound_model_header	 modelHeader;
   uint16_t                    keywordIndex = 0;
   uint16_t                    userIndex = 0;
   int                         iActivePairs = 0;  // active pairs encountered so far
   bool                        bActive;
   uint16_t                    numKeywords = 0;    /* total number of keywords  */
   uint16_t                    numUsers = 0;       /* total number of users  */
   uint16_t *                  pUserActiveFlags;

   ALOGD("getUserKeywordPairIndex: find indices for keyword %s and user %s",
           keywordStr, userStr);
   // get keyword index
   eStatus = getKeywordIndex(pModel,keywordStr, &keywordIndex);
   if (eStatus != LISTEN_SUCCESS) {
      ALOGE("getUserKeywordPairIndex: getKeywordIndex failed with %d", eStatus );
      return eStatus;
   }

   // get user index
   eStatus = getUserIndex(pModel, userStr, &userIndex);
   if (eStatus != LISTEN_SUCCESS) {
      ALOGE("getUserKeywordPairIndex: getUserIndex failed with %d", eStatus );
      return eStatus;
   }
   ALOGD("getUserKeywordPairIndex: keywordIndex %d, userIndex %d", keywordIndex, userIndex );

   // now compare the indices against the active pair truth table from SM:
   //    is pair to be tested active?
   //    output ordered number within the truth table for the given pair to be tested
   // get truth table from SM
   memset(&modelHeader, 0, sizeof(listen_sound_model_header));
   status = ListenSoundModelLibV2::getSoundModelHeader(pModel, &modelHeader);
   if (kSucess != status) {
      ALOGE("getUserKeywordPairIndex: ERROR getSoundModelHeader() failed");
      MAP_ERR(status, eStatus);
      goto cleanup_for_getUserKWPairIndex;
   }
   numKeywords = modelHeader.numKeywords;       /* total number of users  */
   numUsers = modelHeader.numUsers;             /* total number of users  */

   // need to march thru truth table to count number of active pair elements
   //    before out the specific pair to be tested
   for (int u=0; u < numUsers; u++) {
      pUserActiveFlags = modelHeader.userKeywordPairFlags[u];
      for (int k=0; k < numKeywords; k++) {
         bActive = false;
         // check if pair is active; increment current count
         if (pUserActiveFlags[k] != 0) {
            bActive = true;
         }
         if ( (u==userIndex) && (k==keywordIndex) ) {
            // this pair is the one that we want to test
            if (bActive) {
               *pPairIndex = iActivePairs;
               goto cleanup_for_getUserKWPairIndex; // found index so jump to clean up code
            } else {
               ALOGE("getUserKeywordPairIndex: ERROR User %s not found", userStr );
               eStatus = LISTEN_EKEYWORD_USER_PAIR_NOT_IN_SOUNDMODEL;
               goto cleanup_for_getUserKWPairIndex;
            }
         }
         if (bActive) {
            iActivePairs++;  // since active increment count of active pairs
         }
      }
   }

cleanup_for_getUserKWPairIndex:
   status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
   if (kSucess != status) {
      ALOGE("getUserKeywordPairIndex: ERROR releaseSoundModelHeader() failed with %d", status);
      // do not pass this error up to Java
   }
   ALOGV("getUserKeywordPairIndex returns %d", eStatus );
   return eStatus;
}


/*
 * Get Info about Version 1.0 SoundModel
 *
 * Parsers SVA 1.0 SoundModel information from SoundModel.
 * This method assumes that common soundModelInfo fields are already filled.
 *
 * Param [in]   env - ptr to JNIEnv
 * Param [in]   clazz - java class (not an object instance) of this static method
 * Param [in]   soundModel - SoundModel to query
 * Param [out]  soundModelInfo - ref to ListenTypes$SVASoundModelInfo instance
 *                if this parameter is null, then soundModelInfo if not filled
 *
 * Return - status - LISTEN_SUCCESS or LISTEN_EBAD_PARAM
 */
listen_status_enum_t   getSMInfoV1 (
                 JNIEnv *env, jclass clazz,      // ListenSoundModel
                 listen_model_type *       pModel,
                 jobject                   soundModelInfo)         // ListenTypes$SVASoundModelInfo
{
   ALOGV("getSMInfoV1 entered");
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   char *                      keywordStr = NULL;
   jstring                     keywordStrObj;

   uint16_t     numKWs;        /* total number of keywords  */
   uint16_t     numUsers;      /* total number of users  */
   uint16_t     numActivePairs; /* total number of active user+keyword pairs in SM */
   jclass       smInfoClazz = NULL;
   jclass       kwInfoClazz = NULL;
   jclass       countClazz = NULL;
   jclass       strClazz = NULL;
   jfieldID     kwInfoId;
   jfieldID     userNamesId;
   jfieldID     countsId;
   jfieldID     kwPhraseId;
   jfieldID     activeUsersId;
   jobject      tmpKWInfo = NULL;
   jobjectArray kwInfoArr = NULL;
   jmethodID    countsConstructorId;
   jobject      counts = NULL;
   jmethodID    kwInfoConstructorId;
   jfieldID     numKWsId;
   jfieldID     numUserId;
   jfieldID     numPairsId;
   jobject      iKWInfo = NULL;
   jfieldID     smKWPhraseId = NULL;

   int numUserForThisKW = 0;

#ifdef SVA1_SUPPORTED

   // fill structure after calling Query...
   // put fields into ListenTypes$SVASoundModelInfo jobject
   smInfoClazz = env->FindClass("com/qualcomm/listen/ListenTypes$SVASoundModelInfo");
   if (NULL == smInfoClazz) {
      ALOGE("getSMInfoV1: ERROR SoundModelInfo class not found");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV1;
   }
   // push fields from modelInfo acquire from LSMLib query() into soundModelInfo
   smKWPhraseId = env->GetFieldID(smInfoClazz, "keywordPhrase", "Ljava/lang/String;");
   if ( NULL==smKWPhraseId )  {
      ALOGE("getSMInfoV1: ERROR keywordPhrase from SoundModelInfo class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV1;
   }
   // Call SVA 1.0 function to return Keyword string
   keywordStr = (char *)calloc(MAX_KEYWORD, 1);
   if (NULL == keywordStr) {
      ALOGE("getSMInfoV1: ERROR malloc'ing keywordStr failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_for_getSMInfoV1;
   }
   ALOGD("getSMInfoV1: ListenSoundModelLib::getKeywordStr() called");
   status = ListenSoundModelLib::getKeywordStr(pModel, keywordStr);
   if (kSucess != status) {
      ALOGE("getSMInfoV1: ERROR verifyUserRecording() failed");
      MAP_ERR(status, eStatus);
      goto cleanup_for_getSMInfoV1;
   }
   keywordStrObj = env->NewStringUTF(keywordStr);   // GC'ed

	numKWs = 1;
	numUsers = 0;
	numActivePairs =  1;

   ALOGD("getSMInfoV1: numKWs = %d, numUsers = %d, numPairs = %d", numKWs, numUsers, numActivePairs);

   ALOGD("getSMInfoV1: getKeywordPhrase output 1 KW string %s", keywordStr );
   keywordStrObj = env->NewStringUTF(keywordStr);   // GC'ed

   // push fields from modelInfo acquire from LSMLib query() into VWUSoundModelInfo
   smInfoClazz = env->FindClass("com/qualcomm/listen/ListenTypes$SVASoundModelInfo");
   kwInfoClazz = env->FindClass("com/qualcomm/listen/ListenTypes$KeywordInfo");
   countClazz  = env->FindClass("com/qualcomm/listen/ListenTypes$KeywordUserCounts");
   strClazz    = env->FindClass("java/lang/String");
   if ( (NULL == smInfoClazz) || (NULL == kwInfoClazz) ||
        (NULL == countClazz)  || (NULL == strClazz)) {
      ALOGE("getSMInfoV1: ERROR SoundModelInfo or KeywordInfo class not found");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV1;
   }
   kwInfoId = env->GetFieldID(smInfoClazz, "keywordInfo", "[Lcom/qualcomm/listen/ListenTypes$KeywordInfo;");
   userNamesId = env->GetFieldID(smInfoClazz, "userNames", "[Ljava/lang/String;");
   countsId = env->GetFieldID(smInfoClazz, "counts", "Lcom/qualcomm/listen/ListenTypes$KeywordUserCounts;");
   if ( (NULL==kwInfoId) || (NULL==userNamesId) || (NULL==countsId))  {
      ALOGE("getSMInfoV1: ERROR field from SVASoundModelInfo class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV1;
   }

   kwPhraseId = env->GetFieldID(kwInfoClazz, "keywordPhrase", "Ljava/lang/String;");
   activeUsersId = env->GetFieldID(kwInfoClazz, "activeUsers", "[Ljava/lang/String;");
   if ( (NULL==kwPhraseId) || (NULL==activeUsersId) )  {
      ALOGE("getSMInfoV1: ERROR field from KeywordInfo class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV1;
   }

   kwInfoConstructorId   = env->GetMethodID(kwInfoClazz, "<init>", "()V");
   countsConstructorId   = env->GetMethodID(countClazz, "<init>", "()V");
   if ( (NULL==kwInfoConstructorId) || (NULL==countsConstructorId) )  {
      ALOGE("getSMInfoV1: ERROR field from KeywordInfo class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV1;
   }
   tmpKWInfo = env->NewObject(kwInfoClazz, kwInfoConstructorId);
   ALOGD("getSMInfoV1: allocate arrays");
   kwInfoArr = env->NewObjectArray(1, kwInfoClazz, tmpKWInfo);
   counts = env->NewObject(countClazz, countsConstructorId);

   // fill phrase string and activeUsers for this KW
   ALOGD("getSMInfoV1: fill %d KWs", numKWs);
   // create a new KWInfo obj for a single (0th) element of array
   iKWInfo = env->NewObject(kwInfoClazz, kwInfoConstructorId);
   // fill keywordPhrase
   env->SetObjectField(iKWInfo, kwPhraseId , keywordStrObj);
   env->SetObjectField(iKWInfo, activeUsersId , NULL);
   env->SetObjectArrayElement(kwInfoArr, 0 , iKWInfo);

   // fill KeywordUserCounts counts struct
   numKWsId = env->GetFieldID(countClazz, "numKeywords", "S");
   numUserId = env->GetFieldID(countClazz, "numUsers", "S");
   numPairsId = env->GetFieldID(countClazz, "numUserKWPairs", "S");
   if ( (NULL==numKWsId) || (NULL==numUserId) || (NULL==numPairsId))  {
      ALOGE("getSMInfoV1: ERROR field from KeywordUserCounts class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV1;
   }
   env->SetShortField(counts, numKWsId , numKWs);
   env->SetShortField(counts, numUserId , numUsers);
   env->SetShortField(counts, numPairsId , numActivePairs);

   ALOGD("getSMInfoV1: place NULL userNameArr within soundModelInfo");
   env->SetObjectField(soundModelInfo, userNamesId , NULL);
   ALOGD("getSMInfoV1: place kwInfoArr within soundModelInfo");
   // place newly allocated arrays within sound model info struct
   env->SetObjectField(soundModelInfo, kwInfoId , kwInfoArr);
   ALOGD("getSMInfoV1: place counts within soundModelInfo");
   env->SetObjectField(soundModelInfo, countsId , counts);

cleanup_for_getSMInfoV1:
   if (keywordStr)
      free((void *)keywordStr);
   if (tmpKWInfo)
      env->DeleteLocalRef(tmpKWInfo);
   if (kwInfoArr)
      env->DeleteLocalRef(kwInfoArr);
   if (counts)
      env->DeleteLocalRef(counts);
   if (iKWInfo)
      env->DeleteLocalRef(iKWInfo);

   ALOGV("getSMInfoV1 returns %d", (jint)eStatus);
   return eStatus;
#else
   ALOGE("getSMInfoV1: SVA 1.0 SoundModel format not supported !");
   return LISTEN_EFAILURE;
#endif
}


/*
 * Gets Info about Version 2.x SoundModel
 *
 * Parsers SVA 2.0 and above SoundModel information from SoundModel.
 * This method assumes that common soundModelInfo fields are already filled.
 * Gets and fills detailed information about keyword and user in SoundModel.
 * Allocates and fills VWUSoundModelInfo includes String  keywordPhrase.
 *
 * Param [in]   soundModel - SoundModel to query
 * Param [in]   env - ptr to JNIEnv
 * Param [in]   clazz - java class (not an object instance) of this static method
 * Param [in]   soundModel - SoundModel to query
 * Param [out]  soundModelInfo - ref to ListenTypes$SVASoundModelInfo instance
 *                if this parameter is null, then soundModelInfo if not filled
 * Return - status - LISTEN_SUCCESS or LISTEN_EBAD_PARAM
 */
listen_status_enum_t  getSMInfoV2(
                 JNIEnv *env,
                 jclass                    clazz,           // ListenSoundModel
                 listen_model_type *       pModel,
                 jobject                   soundModelInfo)  // ListenTypes$SVASoundModelInfo
{
   ALOGV("getSMInfoV2 entered");
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   listen_status_enum          status = kSucess;
   listen_sound_model_header   modelHeader;

   uint16_t     numKeyphrasesHeader = 0;        /* total number of keywords from SM header */
   uint16_t     numUsersHeader = 0;      /* total number of users from SM header */
   uint16_t     numKeyphrases = 0;        /* total number of keywords from SML getter */
   uint16_t     numUsers = 0;      /* total number of users from SML getter */
   uint16_t     numActivePairs = 0; /* total number of active user+keyword pairs in SM */
   jclass       smInfoClazz = NULL;
   jclass       kwInfoClazz = NULL;
   jclass       countClazz = NULL;
   jclass       strClazz = NULL;
   jfieldID     kwInfoId;
   jfieldID     userNamesId;
   jfieldID     countsId;
   jfieldID     kwPhraseId;
   jfieldID     activeUsersId;
   jobject      tmpKWInfo = NULL;
   jstring      tmpName;
   jobjectArray kwInfoArr = NULL;
   jobjectArray userNameArr = NULL;
   jmethodID    countsConstructorId;
   jobject      counts = NULL;
   jobject      iKWInfo = NULL;
   jobjectArray actUserArr = NULL;
   jmethodID    kwInfoConstructorId;
   jfieldID     numKWsId;
   jfieldID     numUserId;
   jfieldID     numPairsId;
   uint16_t *   pUserActiveFlag;
   char **      keywordPhrases = NULL;
   char **      userNames = NULL;
   jstring      *users = NULL;
   jstring      *keywords = NULL;

   // call SVA V1 functions to get number of keywords, users, user pairs,...
   // then convert char strings to Unicode jstrings
   memset(&modelHeader, 0, sizeof(listen_sound_model_header));
   status = ListenSoundModelLibV2::getSoundModelHeader(pModel, &modelHeader);
   // error checking on getSoundModelHeader
   if (kSucess != status) {
      ALOGE("getSMInfoV2: ERROR getSoundModelHeader() failed");
      MAP_ERR(status, eStatus);
      goto cleanup_for_getSMInfoV2;
   } else if (0 == modelHeader.numKeywords) {
      ALOGE("getSMInfoV2: ERROR getSoundModelHeader() returned numKeywords zero");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   } else if (NULL == modelHeader.numUsersSetPerKw) {
      ALOGE("getSMInfoV2: ERROR modelHeader returned numUsersSetPerKw ptr as NULL");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   } else if ( (modelHeader.numUsers > 0) && (NULL == modelHeader.userKeywordPairFlags) ) {
      ALOGE("getSMInfoV2: ERROR modelHeader returned userKeywordPairFlags ptr is NULL");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   }
   // We can assume below that there is always at least one keyword

   numKeyphrasesHeader = modelHeader.numKeywords;
   numUsersHeader = modelHeader.numUsers;
   numActivePairs =  modelHeader.numActiveUserKeywordPairs;
   ALOGD("getSMInfoV2: numKeyphrasesHeader = %d, numUsersHeader = %d, numPairs = %d",
           numKeyphrasesHeader, numUsersHeader, numActivePairs);
   keywords = new jstring[numKeyphrasesHeader];
   users = new jstring[numUsersHeader];

   ALOGD("getSMInfoV2: getKeywordPhrases ");
   eStatus = allocStrArr(numKeyphrasesHeader, &keywordPhrases);
   if (eStatus != LISTEN_SUCCESS) {
      ALOGE("getUserIndex: ERROR could not alloc memory for String array");
      goto cleanup_for_getSMInfoV2;
   }
   numKeyphrases = numKeyphrasesHeader;
   status = ListenSoundModelLibV2::getKeywordPhrases(pModel, &numKeyphrases, keywordPhrases);
   if (kSucess != status) {
      ALOGE("getSMInfoV2: ERROR getKeywordPhrases() failed");
      MAP_ERR(status, eStatus);
      goto cleanup_for_getSMInfoV2;
   } else if (numKeyphrases != numKeyphrasesHeader) {
       ALOGE("getSMInfoV2: ERROR getKeywordPhrases() returned numKeyphrases=%d which is not "
               "equal to the number returned by the SML header: %d.", numKeyphrases,
               numKeyphrasesHeader);
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   }
   ALOGD("getSMInfoV2: getKeywordPhrase output %d KW strings", numKeyphrases);
   for (int i = 0; i < numKeyphrases; i++) {
       ALOGD("getSMInfoV2: KeywordPhrase[%d] = %s", i, keywordPhrases[i] );
       keywords[i] = env->NewStringUTF(keywordPhrases[i]);  // GC'ed
   }
   numUsers = numUsersHeader;
   if (numUsersHeader > 0) {
      eStatus = allocStrArr(numUsersHeader, &userNames);
      if (eStatus != LISTEN_SUCCESS) {
         ALOGE("getSMInfoV2: ERROR could not alloc memory for String array");
         goto cleanup_for_getSMInfoV2;
      }
      status = ListenSoundModelLibV2::getUserNames(pModel, &numUsers, userNames);
      if (kSucess != status) {
         ALOGE("getSMInfoV2: ERROR getUserNames() failed");
         MAP_ERR(status, eStatus);
         goto cleanup_for_getSMInfoV2;
      } else if (numUsers != numUsersHeader) {
          ALOGE("getSMInfoV2: ERROR getUserNames() returned numUsers=%d which is not equal to "
                  "the number returned by the SML header: %d.", numUsers, numUsersHeader);
          eStatus = LISTEN_EFAILURE;
          goto cleanup_for_getSMInfoV2;
       }
      ALOGD("getSMInfoV2: getUserNames output %d user strings", numUsers);
      for (int i = 0; i < numUsers; i++) {
          ALOGD("getSMInfoV2: UserName[%d] = %s", i, userNames[i] );
          users[i] = env->NewStringUTF(userNames[i]);   // GC'ed
      }
   }
   // push fields from modelInfo acquire from LSMLib query() into VWUSoundModelInfo
   smInfoClazz = env->FindClass("com/qualcomm/listen/ListenTypes$SVASoundModelInfo");
   kwInfoClazz = env->FindClass("com/qualcomm/listen/ListenTypes$KeywordInfo");
   countClazz  = env->FindClass("com/qualcomm/listen/ListenTypes$KeywordUserCounts");
   strClazz    = env->FindClass("java/lang/String");
   if ( (NULL == smInfoClazz) || (NULL == kwInfoClazz) ||
        (NULL == countClazz)  || (NULL == strClazz)) {
      ALOGE("getSMInfoV2: ERROR SoundModelInfo or KeywordInfo class not found");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   }
   kwInfoId = env->GetFieldID(smInfoClazz, "keywordInfo", "[Lcom/qualcomm/listen/ListenTypes$KeywordInfo;");
   userNamesId = env->GetFieldID(smInfoClazz, "userNames", "[Ljava/lang/String;");
   countsId = env->GetFieldID(smInfoClazz, "counts", "Lcom/qualcomm/listen/ListenTypes$KeywordUserCounts;");
   if ( (NULL==kwInfoId) || (NULL==userNamesId) || (NULL==countsId))  {
      ALOGE("getSMInfoV2: ERROR field from SVASoundModelInfo class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   }

   kwPhraseId = env->GetFieldID(kwInfoClazz, "keywordPhrase", "Ljava/lang/String;");
   activeUsersId = env->GetFieldID(kwInfoClazz, "activeUsers", "[Ljava/lang/String;");
   if ( (NULL==kwPhraseId) || (NULL==activeUsersId) )  {
      ALOGE("getSMInfoV2: ERROR field from KeywordInfo class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   }
   kwInfoConstructorId   = env->GetMethodID(kwInfoClazz, "<init>", "()V");
   countsConstructorId   = env->GetMethodID(countClazz, "<init>", "()V");
   if ( (NULL==kwInfoConstructorId) || (NULL==countsConstructorId) )  {
      ALOGE("getSMInfoV1: ERROR field from KeywordInfo class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   }
   tmpKWInfo = env->NewObject(kwInfoClazz, kwInfoConstructorId);
   tmpName =  env->NewStringUTF("#UNKNOWN#");  // gc'ed
   ALOGD("getSMInfoV2: allocate arrays");
   kwInfoArr = env->NewObjectArray(numKeyphrases, kwInfoClazz, tmpKWInfo);
   userNameArr = env->NewObjectArray(numUsers, strClazz, tmpName);
   counts = env->NewObject(countClazz, countsConstructorId);
   // global objects not needed since these are stored in soundModelInfo
   ALOGD("getSMInfoV2: place userNameArr within soundModelInfo");
   env->SetObjectField(soundModelInfo, userNamesId , userNameArr); // userNameArr kept/gc'ed when soundModelInfo freed
   ALOGD("getSMInfoV2: place kwInfoArr within soundModelInfo");
   // place newly allocated arrays within sound model info struct
   env->SetObjectField(soundModelInfo, kwInfoId , kwInfoArr); // kwInfoArr kept/gc'ed when soundModelInfo freed
   ALOGD("getSMInfoV2: place counts within soundModelInfo");
   env->SetObjectField(soundModelInfo, countsId , counts);  // counts kept/gc'ed when soundModelInfo freed

   // fill phrase string and activeUsers for this KW
   // Active Users class will be left NULL if number users is 0
   ALOGD("getSMInfoV2: fill %d KWs", numKeyphrases);
   kwInfoConstructorId   = env->GetMethodID(kwInfoClazz, "<init>", "()V");
   for (int kw=0; kw < numKeyphrases; kw++) {
      // create a new KWInfo obj for each element of array
      iKWInfo = env->NewObject(kwInfoClazz, kwInfoConstructorId);
      env->SetObjectArrayElement(kwInfoArr, kw , iKWInfo);
      // fill keywordPhrase
      env->SetObjectField(iKWInfo, kwPhraseId , keywords[kw]);
      int numUserForThisKW = modelHeader.numUsersSetPerKw[kw];
      actUserArr = env->NewObjectArray(numUserForThisKW, strClazz, tmpName);
      env->SetObjectField(iKWInfo, activeUsersId , actUserArr);

      if (numUserForThisKW > 0) {
         // check the truth table for which users should be put into activeUser array
         ALOGD("getSMInfoV2: try to fill users string");
         for (int u=0, iActiveUserForKW = 0; u < numUsers; u++) {
            pUserActiveFlag = modelHeader.userKeywordPairFlags[u];
            if (NULL == pUserActiveFlag) {
               ALOGE("getSMInfoV2: ERROR modelHeader.userKeywordPairFlags[%d] is NULL", u);
               eStatus = LISTEN_EFAILURE;
               goto cleanup_for_getSMInfoV2;
            }
            if (iActiveUserForKW >= numUserForThisKW) {
               break;  // done filling the users strings for this keyword
            }
            // pair index for double dimensioned truth table:
            //    major index user, minor index keyword
            ALOGD("getSMInfoV2: test active pair[%d][%d]", u, kw);
            ALOGD("            flag is %d", pUserActiveFlag[kw]);
            if (pUserActiveFlag[kw]) {
               // %%% debug
               const char *kwChar = env->GetStringUTFChars(keywords[kw], 0);
               const char *userChar = env->GetStringUTFChars(users[u], 0);
               ALOGD("getSMInfoV2: %s user + keyword %s active",
                       userChar, kwChar);
               env->ReleaseStringUTFChars(keywords[kw], kwChar);
               env->ReleaseStringUTFChars(users[u], userChar);
               env->SetObjectArrayElement(actUserArr, iActiveUserForKW , users[u]);
               iActiveUserForKW++;
            }
         }
      }
      if (iKWInfo)
         env->DeleteLocalRef(iKWInfo);
      if (actUserArr)
         env->DeleteLocalRef(actUserArr);
   }

   ALOGD("getSMInfoV2 fill %d users", numUsers);
   for (int i=0; i < numUsers; i++) {
      // fill unique User names
      env->SetObjectArrayElement(userNameArr, i , users[i]);
   }

   // fill KeywordUserCounts counts struct
   numKWsId = env->GetFieldID(countClazz, "numKeywords", "S");
   numUserId = env->GetFieldID(countClazz, "numUsers", "S");
   numPairsId = env->GetFieldID(countClazz, "numUserKWPairs", "S");
   if ( (NULL==numKWsId) || (NULL==numUserId) || (NULL==numPairsId))  {
      ALOGE("getSMInfoV2: ERROR field from KeywordUserCounts class not acquired");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_for_getSMInfoV2;
   }
   env->SetShortField(counts, numKWsId , numKeyphrases);
   env->SetShortField(counts, numUserId , numUsers);
   env->SetShortField(counts, numPairsId , numActivePairs);

cleanup_for_getSMInfoV2:
   status = ListenSoundModelLibV2::releaseSoundModelHeader(&modelHeader);
   if (kSucess != status) {
      ALOGE("getSMInfoV2: ERROR releaseSoundModelHeader() failed with %d", status);
      // do not pass this error up to Java
   }
   if (keywords) {
       delete [] keywords;
   }
   if (users) {
       delete [] users;
   }
   freeStrArr(numKeyphrasesHeader, keywordPhrases); // silently ignore errors
   freeStrArr(numUsersHeader, userNames); // silently ignore errors
   if (tmpKWInfo)
      env->DeleteLocalRef(tmpKWInfo);
   if (kwInfoArr)
      env->DeleteLocalRef(kwInfoArr);
   if (userNameArr)
      env->DeleteLocalRef(userNameArr);
   if (counts)
      env->DeleteLocalRef(counts);

   ALOGV("getSMInfoV2 returns %d", (jint)eStatus);
   return eStatus;
}

/*
 * Allocates an array of strings for LSMLib query functions
 *
 * Allocates an array of char * that point to strings.
 * Use this before calling LSMLib getKeywordPhrase() or getUserName().
 *
 * param [in] number of elements
 * param [in] ptr to double dimensional (char **) array memory is saved to
 *
 * return status
 */
listen_status_enum_t allocStrArr(uint16_t size, char *** pppArr)
{
   char **ppArr;
   ALOGV("allocStrArr enters - size %d, write into %p", size, pppArr);
   ppArr = (char **)calloc(size, sizeof(char *));
   if (NULL == ppArr) {
      ALOGE("allocStrArr: ERROR malloc ppArr failed");
      return LISTEN_ENO_MEMORY;
   }

   ALOGV("allocStrArr malloc's each element in Array");
   for (int i=0; i<size ; i++) {
      ppArr[i] = (char *)calloc(MAX_STRING_LEN, 1);
      if (NULL == ppArr[i]) {
         ALOGE("allocStrArr: ERROR malloc ppArr[%d] failed", i);
         return LISTEN_ENO_MEMORY;
      }
   }
   *pppArr = ppArr;
   ALOGV("allocStrArr returns");
   return LISTEN_SUCCESS;
}


/*
 * free all memory allocated for string array
 *
 * Allocate an array of strings created by allocStrArr()
 * Use when finished with this array with
 *
 * param [in] number of elements
 * param [in] ptr to double dimensional (char **) array memory is saved to
 *
 * return status
 */
listen_status_enum_t freeStrArr(uint16_t size, char ** ppArr)
{
   if (0 == size)
      return LISTEN_SUCCESS;  // silently do nothing

   ALOGV("freeStrArr %p", ppArr);
   if (NULL == ppArr)
      return LISTEN_SUCCESS;  // silently do nothing

   ALOGV("freeStrArr free each element in Array size %d", size);
   for (int i=0; i<size ; i++) {
      if (ppArr[i])
         free ((void *)ppArr[i]);
   }
   if (ppArr)
      free ((void *)ppArr);
   ALOGV("freeStrArr returns");
   return LISTEN_SUCCESS;
}

/*
 * Puts audio samples array short buffer into Java ShortBuffer object
 *
 * param [in] shortBuffObj - ByteBuffer object that byte buffer to be placed in
 * param [in] size - size the byte buffer
 * param [in] pShortBuff - ptr to short array containing soundmodel
 *
 * Return - status
 */
listen_status_enum_t putAudioInShortBuffer(JNIEnv *  env,
                                               jobject   shortBuffObj,
                                               uint32_t  size,
                                               int16_t * pShortBuff)
{
   listen_status_enum_t        eStatus = LISTEN_SUCCESS;
   jclass                      shortBuffClazz;
   jmethodID                   putId;
   jsize                       smLen = 0;
   jshortArray                 sampleArr = NULL;

   ALOGV("putAudioInShortBuffer entered");
   // Put audio samples into ShortBuffer shortBuffObj
   shortBuffClazz = env->FindClass("java/nio/ShortBuffer");
   // get the ShortBuffer methods we need to call to Get the short array
   putId = env->GetMethodID(shortBuffClazz, "put", "([S)Ljava/nio/ShortBuffer;");
   if ( NULL == putId ) {
     ALOGE("putAudioInShortBuffer: ERROR GetMethodID put for ShortBuffer class failed");
     eStatus = LISTEN_EFAILURE;
   }
   smLen = (jsize)size;
   sampleArr = env->NewShortArray(smLen);
   // short array Inside ShortBuffer is GC'ed by Java
   ALOGV("putSoundModelInByteBuffer SetShortArrayRegion");
   env->SetShortArrayRegion(sampleArr, (jsize)0, smLen, (jshort *)pShortBuff );

   ALOGV("putAudioInShortBuffer call put()");
   shortBuffObj = env->CallObjectMethod(shortBuffObj, putId, sampleArr);

   if (sampleArr)
      env->DeleteLocalRef(sampleArr);
   ALOGV("putAudioInShortBuffer returned");
   return eStatus;
}

/*
 * Gets language model data from the passed in ByteBuffer.
 *
 * Given a Java ByteBuffer jobject extracts the data into a C++ object.
 *
 * param [in]  env - ptr to JNI environment
 * param [in]  byteBuffObj - java ByteBuffer object containing language model
 * param [out] pSize - size of the language model contained in the ByteBuffer
 * param [out] ppData - ptr to byte array containing language model data
 *
 * Return - status
 */
listen_status_enum_t getLanguageModelFromByteBuff(JNIEnv *env,
                                       jobject byteBuffObj,
                                       uint32_t *pSize, uint8_t **ppData)
{
   ALOGV("getLanguageModelFromByteBuff entered");
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   // initialize output parameter values
    jclass byteBuffClazz = env->FindClass("java/nio/ByteBuffer");
   if (NULL == byteBuffClazz) {
      ALOGE("FindClass java/nio/ByteBuffer failed");
      return LISTEN_EFAILURE;
   }
   if (NULL == byteBuffObj) {
      ALOGE("byteBuffObj NULL");
      return LISTEN_EBAD_PARAM;
   }
   if (NULL == pSize ||  NULL == ppData ) {
      ALOGE("byteBuffObj NULL");
      return LISTEN_EBAD_PARAM;
   }
   *pSize = 0;
   *ppData = (uint8_t *)NULL;

   // get the ByteBuffer methods we need to call to Get the byte array
   jmethodID hasArrId   = env->GetMethodID(byteBuffClazz, "hasArray", "()Z");
   jmethodID getArrId   = env->GetMethodID(byteBuffClazz, "array", "()[B");
   if ( (NULL == hasArrId) || (NULL == getArrId) ) {
      ALOGE("getLanguageModelFromByteBuff: ERROR GetFieldId ByteBuffer class failed");
      return LISTEN_EFAILURE;
   }

   ALOGV("Call ByteBuffer.hasArray()...");
   bool bBuffHasArr = env->CallBooleanMethod(byteBuffObj, hasArrId);
   if (bBuffHasArr) {
       ALOGV("... returns true");
   } else  {
       ALOGE("getLanguageModelFromByteBuff: ERROR ... returns false - no array");
       return LISTEN_EBAD_PARAM;
   }

   jmethodID offsetId   = env->GetMethodID(byteBuffClazz, "arrayOffset", "()I");
   if ( (NULL == offsetId) ) {
     ALOGE("getLanguageModelFromByteBuff: ERROR additional GetFieldId ByteBuffer class failed");
     return LISTEN_EFAILURE;
   }
   ALOGV("getLanguageModelFromByteBuff: Call CallObjectMethod -> ByteBuffer.arrayOffset()...");
   jint iOffset = env->CallIntMethod(byteBuffObj, offsetId);
   ALOGV("getLanguageModelFromByteBuff: ByteBuffer.arrayOffset() returned %d", (int)iOffset);

   ALOGV("getLanguageModelFromByteBuff: Call CallObjectMethod -> ByteBuffer.array()...");
   // extract the byte array from Java ByteBuffer into store as C++ ptr to byte[]
   jobject jArrayObj = env->CallObjectMethod(byteBuffObj, getArrId);
   if (NULL == jArrayObj) {
       ALOGE("getLanguageModelFromByteBuff: ERROR ... array return NULL");
       return LISTEN_EBAD_PARAM;
   }
   ALOGV("cast jArrayObj = %p to byteArray", jArrayObj);
   jbyteArray byteArray = (jbyteArray)jArrayObj;
   ALOGV("GetArrayLength");
   jsize arrayLen = env->GetArrayLength(byteArray);
   *pSize = (uint32_t)arrayLen;
   ALOGV("Allocate byte array of size %d",*pSize);
   *ppData = (uint8_t *)malloc(*pSize);
   ALOGV("GetByteArrayRegion w/ byteArray=%p, len=%d, into ppData=%p",
      byteArray, arrayLen, *ppData );
   env->GetByteArrayRegion(byteArray, (jsize)iOffset, arrayLen-iOffset, (jbyte *)*ppData);
   ALOGV("... ByteBuffer.array() returns size, data (%d, %p)", *pSize, *ppData);

   return LISTEN_SUCCESS;
}
}; // namespace android
