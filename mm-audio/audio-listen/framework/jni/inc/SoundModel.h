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

#include <jni.h>
#ifdef LISTEN_SERVICE_USED
#include <ListenNativeTypes.h>

#else
//
// Constants - duplicated in ListenNativeTypes.h
//
static const uint32_t  UNKNOWN_SOUNDMODEL_TYPE = 0;
// SoundModel is Snapdragon Voice Activation format
static const uint32_t  SVA_SOUNDMODEL_TYPE = 1;

// Version number: 2 MSBytes major number, 2 LSBytes minor number
//    For non-SVA SoundModels version will be unknown
static const uint32_t  VERSION_UNKNOWN    = 0;
static const uint32_t  VERSION_0100       = 0x0100; // Version 1.0
static const uint32_t  VERSION_0200       = 0x0200; // Version 2.0

/* This enum is used to return status of Listen API calls */
typedef enum{
  LISTEN_SUCCESS = 0,                 // must be 0 to match ListenSoundModel lib enums
  // Error numbering that match Java API errors in ListenTypes.java
  LISTEN_EFAILURE = -1,               // Failed for some unspecified reason - generic
  LISTEN_EBAD_PARAM = -2,             // Bad input parameter(s)
  LISTEN_ESOUNDMODEL_NOT_REGISTERED = -3,  // SoundModel is not registered
  LISTEN_ESOUNDMODEL_ALREADY_REGISTERED = -4, // SoundModel should be deregistered before re-registering
  LISTEN_EFEATURE_NOT_ENABLED = -5,   // either Listen or VoiceWakeup Feature not enabled
  LISTEN_ERESOURCE_NOT_AVAILABLE = -6,  // requested object can not be instanced
  LISTEN_ECALLBACK_NOT_SET = -7,      // callback must be set for MasterControl or Session object
  LISTEN_EBUFFERING_NOT_ENABLED = -8,  // buffering is not enabled for this session
  LISTEN_ENOT_BUFFERING = -9,  // currently not doing buffering
  LISTEN_EBUFFERING_DATA_INCOMPLETE = -10, // data in buffer is not complete; e.g. overflow occurred
  LISTEN_EKEYWORD_NOT_IN_SOUNDMODEL = -11, // keyword specified as input parameter not in SoundModel
  LISTEN_EUSER_NOT_IN_SOUNDMODEL = -12, // user specified as input parameter not in SoundModel
  LISTEN_EKEYWORD_USER_PAIR_NOT_IN_SOUNDMODEL = -13, // user+keyword pair specified as input parameter not in SoundModel
  LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION = -14, // feature not supported with given SoundModel type
  LISTEN_EUNSUPPORTED_SOUNDMODEL = -15, // type or version of SoundModel not support by algorithm
  LISTEN_EUSER_NAME_CANNOT_BE_USED = -16, // user specified as input parameter already used or reserved
  LISTEN_ENO_MEMORY = -17, // memory could not be allocated
  LISTEN_EDETECTION_NOT_RUNNING = -18,  // detection not running
  LISTEN_EUSER_KEYWORD_PAIRING_ALREADY_PRESENT = -19, // can't train same KW-user pair if present in SM
  LISTEN_ESOUNDMODELS_WITH_SAME_KEYWORD_CANNOT_BE_MERGED = -20, // merged SMs must have unique KWs
  LISTEN_ESOUNDMODELS_WITH_SAME_USER_KEYWORD_PAIR_CANNOT_BE_MERGED = -21, // merged SMs can't not contain same pair
  LISTEN_EMAX_KEYWORDS_EXCEEDED = -22, // merge would result in SM that exceeds max KW capacity
  LISTEN_EMAX_USERS_EXCEEDED = -23, // merge would result in SM that exceeds max User capacity
  LISTEN_ECANNOT_DELETE_LAST_KEYWORD = -24, // can not delete keyword from soundmodel if it is the only one
  LISTEN_ENO_SPEACH_IN_RECORDING = -25, // no speach detected within recording
  LISTEN_ETOO_MUCH_NOISE_IN_RECORDING = -26, // level of noise in recording to high

  // Error not (yet) exposed to Java
  LISTEN_ENO_GLOBAL_CONTROL = -1001,            // returned if session request to change global param but does not have permission
  LISTEN_ENOT_INITIALIZED = -1002,              // Service object not initialized
  LISTEN_ERECORDINGS_MISMATCH_KEYWORD = -1003,  // Recordings dont match the model
  LISTEN_ESESSION_NOT_ACTIVE = -1004,           // Session was not created/opened successfully
} listen_status_enum_t;

#endif

#include <ListenSoundModelLib_v2.h>
#include <ListenSoundModelLib.h>

// SVA Version number as stored in SM header
#define SM_VERSION_1p0  4
#define SM_VERSION_1p5  5
#define SM_VERSION_2p0  5

namespace android {

// Maps ListenSoundModelLib error number to ListenNativeType/Java error numbers
// Some errors are being checked for by test scripts.  For these output message
#define MAP_ERR(lsmLibErr, clientErr) \
{ \
    if (lsmLibErr == kSucess) { \
           clientErr = LISTEN_SUCCESS;  \
    } else if (lsmLibErr == kFailed) { \
           clientErr = LISTEN_EFAILURE;  \
    } else if (lsmLibErr == kBadParam) { \
           ALOGE("%s: ERROR bad parameter", __func__); \
           clientErr = LISTEN_EBAD_PARAM;  \
    } else if (lsmLibErr == kKeywordNotFound) { \
           ALOGE("%s: ERROR keyword not found in SoundModel", __func__); \
           clientErr = LISTEN_EKEYWORD_NOT_IN_SOUNDMODEL;  \
    } else if (lsmLibErr == kUserNotFound) { \
           ALOGE("%s: ERROR user not found in SoundModel", __func__); \
           clientErr = LISTEN_EUSER_NOT_IN_SOUNDMODEL;  \
    } else if (lsmLibErr == kUserKwPairNotActive) { \
           ALOGE("%s: ERROR user+keyword pair not found in SoundModel", __func__); \
           clientErr = LISTEN_EKEYWORD_USER_PAIR_NOT_IN_SOUNDMODEL;  \
    } else if (lsmLibErr == kSMVersionUnsupported) { \
           ALOGE("%s: ERROR function not supported for version of SoundModel", __func__); \
           clientErr = LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION;  \
    } else if (lsmLibErr == kUserDataForKwAlreadyPresent) { \
           ALOGE("%s: ERROR user data for keyword already present in SoundModel", __func__); \
           clientErr = LISTEN_EUSER_KEYWORD_PAIRING_ALREADY_PRESENT;  \
    } else if (lsmLibErr == kDuplicateKeyword) { \
           ALOGE("%s: ERROR can not merge SoundModels with same keyword", __func__); \
           clientErr = LISTEN_ESOUNDMODELS_WITH_SAME_KEYWORD_CANNOT_BE_MERGED;  \
    } else if (lsmLibErr == kDuplicateUserKeywordPair) { \
           ALOGE("%s: ERROR can not merge SoundModels with same user+keyword pair", __func__); \
           clientErr = LISTEN_ESOUNDMODELS_WITH_SAME_USER_KEYWORD_PAIR_CANNOT_BE_MERGED;  \
    } else if (lsmLibErr == kMaxKeywordsExceeded) { \
           ALOGE("%s: ERROR maximum number of keywords exceeded", __func__); \
           clientErr = LISTEN_EMAX_KEYWORDS_EXCEEDED;  \
    } else if (lsmLibErr == kMaxUsersExceeded) { \
           ALOGE("%s: ERROR maximum number of users exceeded", __func__); \
           clientErr = LISTEN_EMAX_USERS_EXCEEDED;  \
    } else if (lsmLibErr == kLastKeyword) { \
           ALOGE("%s: ERROR can not delete keyword if it is the only keyword in SoundModel", __func__); \
           clientErr = LISTEN_ECANNOT_DELETE_LAST_KEYWORD;  \
    } else if (lsmLibErr == kNoSignal) { \
           ALOGE("%s: ERROR no speach found in recording", __func__); \
           clientErr = LISTEN_ENO_SPEACH_IN_RECORDING;  \
    } else if (lsmLibErr == kLowSnr) { \
           ALOGE("%s: ERROR too much noise in recording", __func__); \
           clientErr = LISTEN_ETOO_MUCH_NOISE_IN_RECORDING;  \
    } else clientErr = LISTEN_EFAILURE; \
}

/*
 * Checks validity of SoundModel
 *
 * param [in]  pKeywordModel - ptr to LSMLib KeywordModel object
 * param [out] pModelInfo - ptr to SoundModel info struct
 *
 * Return - status
 */
listen_status_enum_t checkSoundModel( listen_model_type       * pKeywordModel,
                                      listen_sound_model_info * pModelInfo);

/*
 * Gets SoundModel Data From ByteBuffer
 *
 * Given a Java ByteBuffer jobject extracts the C++ soundModel data
 *
 * param [in]  env - ptr to JNI environment
 * param [in]  byteBuffObj - java ByteBuffer object
 * param [out] pSize - size of SoundModel contained in byteBuffer
 * param [out] ppData - ptr to byte array containing SoundModel data
 * param [out] pModelInfo - ptr to struct containing SoundModel info from header
 *
 * Return - status
 */
listen_status_enum_t getSMFromByteBuff(JNIEnv *env,
                                       jobject byteBuffObj,
                                       uint32_t *pSize, uint8_t **ppData,
                                       listen_sound_model_info *pModelInfo);

/*
 * Gets language model data from the passed in ByteBuffer.
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
                                       uint32_t *pSize, uint8_t **ppData);

/*
 * Gets UserRecording Data From ShortBuffer
 *
 * Given a Java ShortBuffer jobject extracts the C++ UserRecording data
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
                                         int16_t  **ppData);

/*
 * Checks ByteBuffer Capacity against required size
 *
 * Verifies that capacity of input ByteBuffer is not less than the
 * size this ByteBuffer needs to be.
 *
 * param [in] reqdSize - size the new Byte Buffer needs to be
 * param [out] pByteBuffSize - pointer to capacity of Byte Buffer
 *
 * Return - status
 */
listen_status_enum_t getByteBufferCapacity(JNIEnv *env,
                                           jobject reqdSize,
                                           uint32_t *pByteBuffSize);

/*
 * Checks ByteBuffer Capacity against required size
 *
 * param [in] reqdSize - size the new Byte Buffer needs to be
 * param [in] smObj - ByteBuffer object that needs to be this size
 *
 * Return - status
 */
listen_status_enum_t checkByteBufferCapacity(JNIEnv *env,
                                             uint32_t reqdSize,
                                             jobject smObj);

/*
 * Puts SoundModel array byte buffer into Java ByteBuffer object
 *
 * param [in] byteBuffObj - ByteBuffer object that byte buffer to be placed in
 * param [in] size - size the byte buffer
 * param [in] byteArr - byte buffer containing soundmodel
 *
 * Return - status
 */
listen_status_enum_t putSoundModelInByteBuffer(JNIEnv *  env,
                                               jobject   byteBuffObj,
                                               uint32_t  size,
                                               uint8_t * pByteBuff);

/*
 * Tests if Keyword and/or User are in Model
 *
 * Gets the keyword and user names from SM.
 * Makes sure that keyword and user names strings are contained in SM.
 *
 * param [in] pModel - ptr to model structure
 * param [in] keywordStr - string containing keyword that should be in given model
 * param [in] userStr - string containing keyword that should be in given model
 *
 * Return - status
 */
listen_status_enum_t isKeywordUserInModel(listen_model_type * pModel,
                             const char *        keywordStr,
                             const char *        userStr );

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
                                     uint16_t *          pKeywordIndex);

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
                                     const char *     userStr,
                                     uint16_t *       pUserIndex);

/*
 * Gets active UserKeyword pair Index
 *
 * Gets list of keyword strings.
 * Compares input keyword string against this list and output index of keyword that matches.
 * Gets list of users strings.
 * Compares input user string against this list and output index of user that matches.
 * Then compares the indices against the active pair truth table for SM:
 *   is if pair is active - error otherwise
 *   gets the number order in this truth table for the given index pair
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
                                             uint16_t *          pIndex );

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
 listen_status_enum_t getSMInfoV1(JNIEnv *env,
                 jclass                    clazz,           // ListenSoundModel
                 listen_model_type *       pModel,
                 jobject                   soundModelInfo); // ListenTypes$SVASoundModelInfo

/*
 * Gets Info about Version 2.x SoundModel
 *
 * Parsers SVA 2.0 and above SoundModel information from SoundModel.
 * This method assumes that common soundModelInfo fields are already filled.
 * Gets and fills detailed information about keyword and user in SoundModel.
 * Allocates and fills VWUSoundModelInfo includes String  keywordPhrase.
 *
 * Param [in]    soundModel - SoundModel to query
 * Param [in]   env - ptr to JNIEnv
 * Param [in]   clazz - java class (not an object instance) of this static method
 * Param [in]   soundModel - SoundModel to query
 * Param [out]  soundModelInfo - ref to ListenTypes$SVASoundModelInfo instance
 *                if this parameter is null, then soundModelInfo if not filled
 * Return - status - LISTEN_SUCCESS or LISTEN_EBAD_PARAM
 */
 listen_status_enum_t getSMInfoV2(JNIEnv *env,
                 jclass                    clazz,           // ListenSoundModel
                 listen_model_type *       pModel,
                 jobject                   soundModelInfo); // ListenTypes$SVASoundModelInfo

 /*
 * Allocates an array of strings for LSMLib query functions
 *
 * Allocates an array of char * that point to strings.
 * Use this before calling LSMLib getKeywordPhrase() or getUserName().
 *
 * param [in] number of elements
 * param [in] ptr to double dimensional array (char **) to allocate
 *
 * return status
 */
listen_status_enum_t allocStrArr(uint16_t size, char *** pppArr);

/*
 * Frees all memory allocated for string array
 *
 * Allocates an array of strings created by allocStrArr().
 * Use this when finished with this array.
 *
 * param [in] number of elements
 * param [in] ptr to double dimensional (char **) array memory is saved to
 *
 * return status
 */
listen_status_enum_t freeStrArr(uint16_t size, char ** ppArr);

/*
 * Puts audio samples array short buffer into Java ShortBuffer object
 *
 * param [in] shortBuffObj - ByteBuffer object that byte buffer to be placed in
 * param [in] size - size the byte buffer
 * param [in] pShortBuff - ptr to byte buffer containing soundmodel
 *
 * Return - status
 */
listen_status_enum_t putAudioInShortBuffer(JNIEnv *  env,
                                               jobject   shortBuffObj,
                                               uint32_t  size,
                                               int16_t * pShortBuff);
}; // namespace android
