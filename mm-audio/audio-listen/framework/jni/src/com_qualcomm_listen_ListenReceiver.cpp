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
 * Listen Receiver
 *
 * Generic class inherited by MasterControl and Session receiver classes.
 *
 * This file includes implementation of com_qualcomm_listen_ListenReceiver JNI class methods
 * These methods call Native ListenReceiver methods to initialize either
 * Listen MasterControl or Session native objects.
 *
 * This file also includes implementation of JNIListenReceiverListener class
 * used to receive and process events from C++ ListenReceiver class.
 * JNIListenReceiverListener deals with sending event notification up to Java
 * ListenReceiver instance associated with this native Listen Receiver.
 * A JNIListenReceiverListener object is created per ListenReceiver and the
 * pointer to this JNIListenReceiverListener object is set in Native ListenReceiver.
 *
 * In general all methods do the following 3 basic things:
 *    i)   extract fields from jobject input parameters and store into C++ variables
 *    ii)  call Native ListenReceiver C++ methods
 *    iii) insert output parameters and return values from native function into
 *         jobjects
 *
 */

// enable verbose logs
#define LOG_NDEBUG  0
#define LOG_TAG "ListenReceiver-JNI"
#include <utils/Log.h>

#include <cutils/properties.h>  // for for getting Listen system property

#include <jni.h>
#include "JNIHelp.h"  // for NELEM sizeof()/sizeof(x[0])

#include <binder/Parcel.h>
#include <utils/Mutex.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <ListenNativeTypes.h>
#include <ListenReceiver.h>

#include <com_qualcomm_listen_ListenReceiver.h>
#include <com_qualcomm_listen_ListenMasterControl.h>
#include <com_qualcomm_listen_ListenVoiceWakeupSession.h>
#include <com_qualcomm_listen_ListenSoundModel.h>

#include <ListenSoundModelLib_v2.h>

namespace android {

// Java field Id to points to Native ListenReceiver object stored within Java ListenReceiver object.
// This is static since it does not change and can be set up manditory init() call.
static jfieldID    gReceiverPtrId = NULL;     // ptr to ListenReceiver hidden away in Java class

// Java class and method Ids used by notify method
// They are static since they do not change and can be set up during JNI_OnLoad()
static jmethodID   gEventCallbackId = NULL;   // callback method in Java class
static jclass      gEventDataClazz = NULL;

// lock used within ListenReceiver native code only
static Mutex       sLock;

static JavaVM*     gvm;

// ----------------------------------------------------------------------------
// ref-counted object for callbacks
class JNIListenReceiverListener: public ListenReceiverListener
{
public:
    JNIListenReceiverListener(JNIEnv* env, jobject thiz);
    ~JNIListenReceiverListener();
    virtual void notify(int msg, const Parcel *obj);
private:
    JNIListenReceiverListener();

    // Strong Ref to ListenReceiver Java object to make method call to for event processing
    jobject     mLRecvrObj;
    Mutex       mListenerLock; // ensure destructor and notify don't interupt each other
};

/* Constructor
 * thiz - instance of ListenMasterControl or ListenVoiceWakeupSession
 */
JNIListenReceiverListener::JNIListenReceiverListener(JNIEnv* env, jobject thiz)
{
    ALOGV("JNIListenReceiverListener constructor thiz = %p entered", thiz);
    // Hold onto the ListerReceiver java class for use in calling the static method
    // that posts events to the application thread.
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("JNIListenReceiverListener constructor GetObjectClass(thiz) failed");
        jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    // Ensure ListenReceiver object is not garbage collected until we delete the reference
    mLRecvrObj  = env->NewGlobalRef(thiz);
    ALOGV("JNIListenReceiverListener constructor this = %p returns after setting mLRecvrObj = %p", this, mLRecvrObj);
}

/* Destructor
*/
JNIListenReceiverListener::~JNIListenReceiverListener()
{
    ALOGV("JNIListenReceiverListener destructor enters, this = %p, mLRecvrObj = %p", this, mLRecvrObj);
    // remove global references

    ALOGD("JNIListenReceiverListener destructor DeleteGlobalRef %p", mLRecvrObj);
    JNIEnv* env = NULL;
    if (gvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("~JNIListenReceiverListener JNI_OnLoad: ERROR: GetEnv failed\n");
        return;
    }
    assert(env != NULL);

    if (mLRecvrObj)
       env->DeleteGlobalRef(mLRecvrObj);
    mLRecvrObj = NULL;
    ALOGV("JNIListenReceiverListener destructor returns ");
}

// ----------------------------------------------------------------------------

/*
 * set pointer to Native ListenReceiver object into Java ListenReceiver instance
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to Java ListenReceiver class object
 * Param [in]  receiver - ptr to Native ListenReceiver class to be associated with
 *              Java ListenReceiver object
 */
sp<ListenReceiver> setPtrInJObject(JNIEnv* env, jobject thiz, const sp<ListenReceiver>& receiver)
{
    ALOGV("setPtrInJObject entered, thiz = %p", thiz);
    // Not locked here - lock in _init()
    sp<ListenReceiver> old = (ListenReceiver*)env->GetIntField(thiz, gReceiverPtrId);
    if (old != 0) {
       ALOGV("setPtrInJObject: incStrong sp<%p>", old.get());
       old->decStrong(thiz);
    }
    if (receiver == 0) {
       ALOGV("setPtrInJObject: set NULL into receiver field");
       env->SetIntField(thiz, gReceiverPtrId, 0);
    } else {
       if (receiver.get()) {
          ALOGV("setPtrInJObject: incStrong sp<%p>", receiver.get());
          receiver->incStrong(thiz);
       }
       ALOGV("setPtrInJObject: set %p into receiver field", receiver.get());
       env->SetIntField(thiz, gReceiverPtrId, (int)receiver.get());
    }
    ALOGV("setPtrInJObject returns");
    return old;
}

sp<ListenReceiver> getPtrFromJObject(JNIEnv* env, jobject thiz)
{
    ALOGV("getPtrFromJObject entered, thiz = %p", thiz);
    if (NULL == gReceiverPtrId) {
       ALOGE("getPtrFromJObject: ERROR gReceiverPtrId is NULL");
       return NULL;
    }
    ListenReceiver* const p = (ListenReceiver*)env->GetIntField(thiz, gReceiverPtrId);
    if (p) {
       ALOGV("getPtrFromJObject returns %p", p);
       return sp<ListenReceiver>(p);
    } else {
       ALOGV("getPtrFromJObject returns NULL");
       return NULL;
    }
}

// ----------------------------------------------------------------------------
//    Native Methods
// ----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
/*
 * initialize
 *
 * Class:     com_qualcomm_listen_ListenReceiver
 * Method:    init
 * Signature: (I)I
 *
 * Initialize a native ListenReceiver
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Param [in]  type - type of receiver: e.g. MasterControl, VWUSession,...
 * Param [in]  app_type - describes algorithm to use for detection
 *
 * Return - LISTEN_SUCCESS
 *          LISTEN_EFAILURE  - Listen service disabled
 *          LISTEN_EBAD_PARAM
 *          LISTEN_ENO_MEMORY
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenReceiver_init
  (JNIEnv *env, jobject thiz, jint type, jint appType)
{
   ALOGV("Java_com_qualcomm_listen_ListenReceiver_init entered");
   jclass inheritorClazz = NULL;
   jclass receiverClazz = NULL;
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   jint status =  NO_ERROR;
   ListenReceiver *receiver = NULL;
   sp<ListenReceiver> spReceiver = NULL;
   listen_callback_t cb = NULL;
   void * pThiz = NULL;
   bool bSpReceiverInc = false; // was spReceiver use count incremented?
   JNIListenReceiverListener *pListener = NULL;

   char listenPropStr[PROPERTY_VALUE_MAX];
   property_get("listen.enable", listenPropStr, "1");
   if ( (strncmp(listenPropStr, "0", PROPERTY_VALUE_MAX))==0  ||
        (strncmp(listenPropStr, "false", PROPERTY_VALUE_MAX))==0 ) {
      // disable Listen object creation if Listen system property is explicitly disabled
      ALOGE("_init: ERROR Listen feature is disabled in system.prop");
      return (jint)LISTEN_EFAILURE;
   }

   listen_receiver_enum_t receiverType = (listen_receiver_enum_t)type;

   ALOGD("_init: lock sLock");
   Mutex::Autolock l(sLock);

   // Get the class for the receiver object that is being initialized
   if (LISTEN_RECEIVER_MASTER_CONTROL == receiverType) {
       inheritorClazz = env->FindClass("com/qualcomm/listen/ListenMasterControl");
       ALOGV("_init: new ListenMasterControl");
       receiver = (ListenReceiver *)new ListenMasterControl();
   } else if (LISTEN_RECEIVER_VOICE_WAKEUP_SESSION == receiverType) {
       inheritorClazz = env->FindClass("com/qualcomm/listen/ListenVoiceWakeupSession");
       ALOGV("_init: new ListenVWUSession");
       receiver = (ListenReceiver *)new ListenVWUSession();
   } else {
      ALOGE("_init: ERROR receiver type %d invalid", receiverType);
      return (jint)LISTEN_EBAD_PARAM;
   }
   if (receiver == NULL) {
      ALOGE("_init: ERROR new ListenReceiver failed");
      jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
      return (jint)LISTEN_EFAILURE;
   }
   // assign receiver to strongPtr immediately
   spReceiver = receiver;

   if (NULL == inheritorClazz) {
      eStatus = LISTEN_EFAILURE;
      goto cleanup_after_error;
   }

   // create new listener and pass it to newed ListenReceiver C++ instance
   ALOGV("_init: new JNIListenReceiverListener");
   pListener = new JNIListenReceiverListener(env, thiz);
   if (NULL == pListener) {
      ALOGE("_init: ERROR new JNIListenReceiverListener failed");
      jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_after_error;
   }
   // save ptr to JNIListenReceiverListener in native ListenReceiver object
   ALOGV("_init: setListener");
   receiver->setListener(pListener);

   // get the fields
   gReceiverPtrId = env->GetFieldID(inheritorClazz, "nativeClient", "I");
   if (gReceiverPtrId == NULL) {
      ALOGE("_init: ERROR nativeClient member var not found in clazz");
      eStatus = LISTEN_EFAILURE;
      goto cleanup_after_error;
   }

   if (LISTEN_RECEIVER_MASTER_CONTROL == receiverType) {
       ListenMasterControl *mstrCtrlReceiver = (ListenMasterControl *)receiver;
       ALOGV("_init: call mstrCtrlReceiver->init");
       eStatus = mstrCtrlReceiver->init();
   } else if (LISTEN_RECEIVER_VOICE_WAKEUP_SESSION == receiverType){
       ListenVWUSession *vwuSessionReceiver = (ListenVWUSession *)receiver;
       ALOGV("_init: call vwuSessionReceiver->init");
       eStatus = vwuSessionReceiver->init(appType);
   }

   ALOGD("_init: XXX->init returns %d", (int)eStatus);
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("_init: ERROR receiver->init failed with %d", eStatus);
      goto cleanup_after_error;
   } else {
      // Stow our new C++ ListenReceiver in an opaque field in
      //     the Java ListenReceiver object as a strong ptr
      ALOGV("_init: save sp<receiver> in Java object");
      setPtrInJObject(env, thiz, spReceiver);
      // Use count for strong ptr is incremented by setPtrInJObject()
      bSpReceiverInc = true;
   }
   ALOGV("_ListenReceiver_init returns successfully");
   return (jint)LISTEN_SUCCESS;

cleanup_after_error:
   ALOGD("_init cleanup_after_error");
   if (pListener && receiver) {
      ALOGD("_init clearListener");
      receiver->clearListener();
   }
   if ( (spReceiver != NULL) && bSpReceiverInc ) {
      // decrement useCount only if this was incremented before error
      ALOGD("setPtrInJObject: decStrong sp<%p>", spReceiver.get());
      spReceiver->decStrong(thiz);
   }

   ALOGD("_ListenReceiver_init returns after error");
   return (jint)eStatus;
}

/*
 * release
 *
 * Class:     com_qualcomm_listen_ListenReceiver
 * Method:    release
 * Signature: ()I
 *
 * Release ListenReceiver
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Return - errors
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenReceiver_release
  (JNIEnv *env, jobject thiz)
{
   ALOGV("Java_com_qualcomm_listen_ListenReceiver_release entered");
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   ALOGD("_release: lock sLock");
   Mutex::Autolock l(sLock);
   sp<ListenReceiver> spListenReceiver = getPtrFromJObject(env, thiz);
   if (spListenReceiver == NULL) {
      return LISTEN_SUCCESS; // not able to call ListenReceiver native code
   }
   ALOGV("_release: clear listener set in native Listen Receiver");
   spListenReceiver->clearListener();   // free listener newed in ListenReceiver_init
   ALOGV("_release: call ListenReceiver->release");
   eStatus = spListenReceiver->release();
   if (LISTEN_SUCCESS != eStatus) {
      ALOGE("_release: ERROR receiver->release returned %d", eStatus);
   }
   // decrement reference to native Receiver within java instance
   setPtrInJObject(env, thiz, NULL);

   ALOGV("_ListenReceiver_release returned status %d", eStatus);
   return (jint)eStatus;
}

/*
 * getDetectionStrings
 *
 * Class:     com_qualcomm_listen_ListenReceiver
 * Method:    getDetectionStrings
 * Signature: (ILcom/qualcomm/listen/ListenTypes/EventData;)I
 *
 * From the event payload returned by Detection Succeed or Failed
 * determines the highest scoring Keyword detected and User verified.
 * Finds the strings for these detected keyword and verified user.
 * These strings are then stored into the EventData class
 * created by the calling function.
 *
 * This function uses a small set of information about the SM that was saved
 * in the ListenReceiver C++ object during registration to calculate:
 *    a) which Keyword index has the highest score
 *    b) which User index has the highest non-zero score
 *    c) which Keyword phrase string is associated with this KW index
 *    d) which User name string is associated with this User index
 *
 * Param [in]  env - ptr to JNIEnv
 * Param [in]  thiz - ptr to java class object this method belongs to
 * Param [in]  type - event type returned to receiveEvent()
 * Param [in]  eventData - ptr to EventData object with payload, that is to be
 *             filled with keyword and user strings
 * Return - errors
 */
JNIEXPORT jint JNICALL Java_com_qualcomm_listen_ListenReceiver_getDetectionStrings
  (JNIEnv *env, jobject thiz, jint eventType, jobject eventDataObj)
{
   ALOGV("Java_com_qualcomm_listen_ListenReceiver_getDetectionStrings entered");
   listen_status_enum_t eStatus = LISTEN_SUCCESS;
   jfieldID    dataSizeId;
   jfieldID    dataArrayId; // byte array
   jfieldID    keywordStrId; // String
   jfieldID    userStrId; // String
   jstring     keywordStr;
   jstring     userStr;
   uint16_t    kwIndex;
   uint16_t    pairIndex;
   uint16_t    userIndex;
   uint16_t    iActivePairCount;
   uint16_t    numKWs;
   uint16_t    numUsers;
   uint16_t    numPairs;
   uint16_t    hiKWConfLevel;
   uint16_t    hiPairConfLevel;
   uint16_t    iActivePair = 0;
   uint8_t *   eventData = NULL; // block of memory containing data payload
   uint32_t    eventDataSize;    // size in bytes of payload data
   jobject     jArrayObj;
   int         arrayLen = 0;
   jbyteArray  byteArray;
   int         iPairPayloadIndx;
   int         numConfLevels;
   int         iPair = 0;

   ListenReceiver * pListenReceiver = NULL;
   ListenVWUSession * pListenVWUSession = NULL;
   sp<ListenReceiver> spListenReceiver = getPtrFromJObject(env, thiz);
   if (spListenReceiver == NULL) {
      ALOGE("_getDetectionStrings: spListenReceiver null - not able to get ListenReceiver object ");
      return LISTEN_EFAILURE;
   }
   // get the 'detection' parameters saved during last registerSoundModel
   // these can be retrieved from pListenVWUSession
   pListenReceiver = (ListenReceiver *)spListenReceiver.get();
   // Cast ListenReceiver to ListenVoiceWakeup
   pListenVWUSession = (ListenVWUSession *)pListenReceiver;

   ALOGV("_getDetectionStrings GetFieldID(gEventDataClazz,...");
   dataSizeId = env->GetFieldID(gEventDataClazz, "size", "I");
   dataArrayId  = env->GetFieldID(gEventDataClazz, "payload", "[B"); // byte array
   keywordStrId = env->GetFieldID(gEventDataClazz, "keyword", "Ljava/lang/String;"); // String
   userStrId  = env->GetFieldID(gEventDataClazz, "user", "Ljava/lang/String;"); // String
   if ( (keywordStrId == NULL)  || (userStrId == NULL) ||
        (dataSizeId == NULL) || (dataArrayId == NULL) ) {
      ALOGE("_getDetectionStrings: ERROR field(s) from EventData class not acquired");
      return LISTEN_EFAILURE;
   }

   ALOGV("_getDetectionStrings: get payload byte array");
   eventDataSize = env->GetIntField(eventDataObj, dataSizeId);
   //
   // extract byte array from the event payload jobject input parameter
   // Payload contains N Keyword scores and M User-Pairing scores
   //
   jArrayObj = env->GetObjectField(eventDataObj, dataArrayId);
   ALOGV("_getDetectionStrings: cast jArrayObj = %p to byteArray", jArrayObj);
   byteArray = (jbyteArray)jArrayObj;
   ALOGV("_getDetectionStrings: GetArrayLength");
   arrayLen = env->GetArrayLength(byteArray);
   ALOGV("_getDetectionStrings: Allocate byte array of size %d", arrayLen);
   eventData = (uint8_t *)malloc(arrayLen);
   if (NULL == eventData) {
      ALOGE("_getDetectionStrings: ERROR malloc of detection event payload data failed");
      eStatus = LISTEN_ENO_MEMORY;
      goto cleanup_get_det_str;
   }
   ALOGV("_getDetectionStrings: GetByteArrayRegion w/ byteArray=%p, len=%d, into detectionEventPayload.data=%p",
      byteArray, arrayLen, eventData );
   env->GetByteArrayRegion(byteArray, (jsize)0, (jsize)arrayLen, (jbyte *)eventData);

   //
   // finding the index of the keyword with the highest confidence level
   //
   ALOGD("_getDetectionStrings: address of mDetectionParams is %p", &pListenVWUSession->mDetectionParams);
   // extract number of keywords, users and active user-pairings from ListenReceiver object
   numKWs = pListenVWUSession->mDetectionParams.numKeywords;
   numUsers = pListenVWUSession->mDetectionParams.numUsers;
   numPairs = pListenVWUSession->mDetectionParams.numActiveKWUserPairs;
   numConfLevels = numUsers + numPairs;
   ALOGD("_getDetectionStrings: test confidence levels for %d keywords and %d pairs", numKWs,  numPairs);
   ALOGD("_getDetectionStrings: numConfLevels %d", numConfLevels);
   hiKWConfLevel = 0;
   hiPairConfLevel = 0;
   kwIndex = 0xFFFF;
   for (uint16_t k = 0; k < numKWs; k++) {
      if (eventData[k] > hiKWConfLevel) {
          hiKWConfLevel = eventData[k];
          kwIndex = k;
      }
   }
   if (kwIndex == 0xFFFF) {
      ALOGE("_getDetectionStrings: ERROR highest non-zero keyword confidence level not found");
      kwIndex = 0;
   }
   ALOGD("_getDetectionStrings: keyword index with highest confidence level = %d, value is %d", kwIndex,
              (int)eventData[kwIndex]);
   keywordStr = env->NewStringUTF(pListenVWUSession->mDetectionParams.keywords[kwIndex]);  // GC'ed
   ALOGD("_getDetectionStrings: keyword string is %s", pListenVWUSession->mDetectionParams.keywords[kwIndex]);

   if ( (LISTEN_MODE_KEYWORD_ONLY_DETECTION == pListenVWUSession->mDetectionParams.detectionMode) ||
        (0 == numPairs) ) {
      ALOGD("_getDetectionStrings: null output for user string since detection mode 0 or number pairs 0");
      userStr = NULL;
   } else {
      // find the index of active user pairing with the largest confidence level
      iActivePair = 0;
      pairIndex = 0xFFFF; // use this value to signify undefined
      iPairPayloadIndx = numKWs; // index to start of detection levels for pairs
      for (uint16_t p = 0; p < numPairs; p++) {
         if (eventData[iPairPayloadIndx + p] > hiPairConfLevel) {
             hiPairConfLevel = eventData[iPairPayloadIndx + p];
             pairIndex = p;
         }
      }
      userIndex = 0xFFFF; // use this value to signify undefined
      if (pairIndex < 0xFFFF) { // pair with value > 0 index found
         ALOGD("_getDetectionStrings: pair index with highest confidence level = %d, value[%d] is %d", pairIndex,
             numKWs + pairIndex, (int)eventData[iPairPayloadIndx + pairIndex]);
         // find the user string for the [numUser][numKeyword] pair that was verified
         // use UserKeywordPairFlags[] array containing boolean flags (1 active, 0 not active) from ListenReceiver object
         // loop over all pair comparing active pairs only
         iActivePairCount = 0;
         iPair = 0;
         for (int u=0; u < numUsers; u++) {
            for (int k=0; k < numKWs; k++, iPair++) {
               // if pair is active, then check if this is
               ALOGD("getDetectionStrings: userKeywordPairFlags[%d] = %d",
                     iActivePairCount, pListenVWUSession->mDetectionParams.pUserKeywordPairFlags[iPair]);
               if (pListenVWUSession->mDetectionParams.pUserKeywordPairFlags[iPair]) {
                  ALOGD("getDetectionStrings: pairFlags[%d][%d] active", u, k );
                  if (iActivePairCount == pairIndex) {
                     userIndex = u;
                     ALOGD("getDetectionStrings: userKeywordPairFlags[%d][%d] matched", u, k);
                     u = numUsers; // force breaking out of top 'u' loop
                     break;
                  } else {
                     iActivePairCount++;
                     ALOGD("getDetectionStrings: increment iActivePairCount");
                  }
               }
               else {
               }
            }
         }
      }
      if (userIndex == 0xFFFF) {
         ALOGE("_getDetectionStrings: active pair NOT matched, output null for user string");
         userStr = NULL;
      } else {
         userStr = env->NewStringUTF(pListenVWUSession->mDetectionParams.users[userIndex]);
         ALOGD("_getDetectionStrings: user string is %s", pListenVWUSession->mDetectionParams.users[userIndex]);
      }
   } // no non-zero active pairs

   // place keyword and user strings into EventData object
   env->SetObjectField(eventDataObj, keywordStrId , keywordStr);
   env->SetObjectField(eventDataObj, userStrId , userStr);

cleanup_get_det_str:
   if (eventData) {
      free((void *)eventData);
   }

   ALOGV("_ListenReceiver_getDetectionStrings returned status %d", eStatus);
   return (jint)eStatus;
}

/*
 * Send this event to java object waiting on events via ListenReceiver::receiveEvent()
 * If event data in input parcel is not empty, convert to byte array
 * Checks the receiver static and global variables are setup up
 * This method is call for all events
 *
 * param [in]  eventParcel - parcel containing event data
 */
void JNIListenReceiverListener::notify(int msg, const Parcel *eventParcel)
{
   uint8_t *eventData = NULL;
   jbyteArray dataArray = NULL;
   jint eventType = (jint)msg;  // by default msg value is left unchanged
   jint eventTypeVers = (jint)EVENT_DATA_VERSION;  // by default set to version 1
   jobject tmpEventDataObj = NULL;
   jobject eventDataObj = NULL;
   jobject eventDataGblObj = NULL;
   jmethodID eventDataConstructorId;
   int parcelSize = 0;
   int eventDataSize = 0;
   jfieldID dataArrayId;
   jfieldID dataSizeId;
   jfieldID eventTypeId;
   JNIEnv* env = NULL;
   if (gvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
       ALOGE("notify JNI_OnLoad: ERROR: GetEnv failed\n");
       return;
   }
   assert(env != NULL);

   ALOGV("JNIListenReceiverListener::notify(msgType %d) entered", msg);
   if (NULL == eventParcel) {
      ALOGE("JNIListenReceiverListener::notify eventParcel NULL");
      return;
   }
   ALOGD("JNIListenReceiverListener::notify: lock mListenerLock");
   Mutex::Autolock l(mListenerLock);
   if ( (NULL == gEventCallbackId) || (NULL == mLRecvrObj) || (NULL == gEventDataClazz) ) {
      ALOGE("JNIListenReceiverListener::notify gEventCallbackId is %p, mLRecvrObj is %p, gEventDataClazz is %p",
            gEventCallbackId, mLRecvrObj, gEventDataClazz);
      ALOGE("JNIListenReceiverListener::notify: ERROR bad ptr; notify call SKIPPED!");
      return;
   }

   // create a new object of type EventData, clean up after notify is called
   ALOGV("JNIListenReceiverListener::notify: GetMethodID(gEventDataClazz) constructor");
   eventDataConstructorId = env->GetMethodID(gEventDataClazz, "<init>", "()V");
   // create a new obj
   ALOGV("JNIListenReceiverListener::notify: NewObject using gEventDataClazz constructor");
   eventDataObj = env->NewObject(gEventDataClazz, eventDataConstructorId);
   if (eventDataObj == NULL) {
      ALOGE("JNIListenReceiverListener::notify: ERROR NewObject of gEventDataClazz failed");
      goto cleanup_call_receiver_event;
   }
   eventDataGblObj  = env->NewGlobalRef(eventDataObj);
   if (eventDataGblObj == NULL) {
      ALOGE("JNIListenReceiverListener::notify: ERROR NewGlobalRef failed");
      goto cleanup_call_receiver_event;
   }
   // get size from parcel and store into eventData jObject
   ALOGV("JNIListenReceiverListener::notify: GetFieldIDs for gEventDataClazz");
   dataSizeId = env->GetFieldID(gEventDataClazz, "size", "I");
   eventTypeId = env->GetFieldID(gEventDataClazz, "type", "I");
   if ( (dataSizeId == NULL) || (eventTypeId == NULL) ) {
      ALOGE("JNIListenReceiverListener::notify: ERROR get fieldId from EventData class failed");
      goto cleanup_call_receiver_event;
   }

   if (eventParcel) {
      parcelSize = (int)(eventParcel->dataSize());
      ALOGV("JNIListenReceiverListener::notify:  parcel size is %d", parcelSize);
      if (parcelSize > 0) {
         eventParcel->setDataPosition(0);
         ALOGV("JNIListenReceiverListener::notify: position %d reset to start", (int)eventParcel->dataPosition());
         // event data contains a size and a array of bytes
         eventDataSize = eventParcel->readInt32();
      }
   }
   ALOGV("JNIListenReceiverListener::notify:() set eventDataSize %d", eventDataSize);
   env->SetIntField(eventDataGblObj, dataSizeId, eventDataSize);

   if (eventDataSize > 0) {
      ALOGV("JNIListenReceiverListener::notify: GetFieldID payload from gEventDataClazz");
      jfieldID dataArrayId  = env->GetFieldID(gEventDataClazz, "payload", "[B"); // byte array
      if (dataArrayId == NULL) {
         ALOGE("JNIListenReceiverListener::notify: ERROR payload field from EventData class not acquired");
         goto cleanup_call_receiver_event;
      }

      ALOGV("JNIListenReceiverListener::notify: Create a byte array jObject and set into eventData jObject");
      eventData = (uint8_t *)malloc(eventDataSize);
      if (NULL == eventData) {
         ALOGE("JNIListenReceiverListener::notify: ERROR malloc of detection event payload data failed");
         goto cleanup_call_receiver_event;
      } else {
         eventParcel->read((void *)eventData, eventDataSize);
         ALOGV("JNIListenReceiverListener::notify: eventData ptr %p contains:", eventData);
         for (int j=0; j<eventDataSize; j++) {
            ALOGV("   [%d] = 0x%x", j, eventData[j]);
         }
         dataArray = env->NewByteArray(eventDataSize); // Make a byteArray for event data payload
         // copy eventData into byte array
         ALOGV("JNIListenReceiverListener::notify: set eventData byteArrayRegion size %d", eventDataSize);
         env->SetByteArrayRegion(dataArray, (jsize)0, eventDataSize, (jbyte *)eventData);
         ALOGV("JNIListenReceiverListener::notify: SetObjectField dataArrayId from eventDataGblObj");
         env->SetObjectField(eventDataGblObj, dataArrayId, dataArray);
      }
   } else {
      ALOGV("JNIListenReceiverListener::notify: dataArray element left NULL since size is 0");
   }

   // If detection V2 message type being processing, set Event Type to V2 and
   //     change message value to generic detection message constant

   if (msg == LISTEN_DETECT_V2_SUCCEED) {
      ALOGV("JNIListenReceiverListener::notify: set type to V2, msg to LISTEN_DETECT_SUCCEED");
      eventTypeVers = EVENT_DATA_VERSION_V2;
      eventType = LISTEN_DETECT_SUCCEED;
   }
   else if (msg == LISTEN_DETECT_V2_FAILED) {
      ALOGV("JNIListenReceiverListener::notify: set type to V2, msg to LISTEN_DETECT_FAILED");
      eventTypeVers = EVENT_DATA_VERSION_V2;
      eventType = LISTEN_DETECT_FAILED;
   }
   ALOGV("JNIListenReceiverListener::notify:() set eventDataSize %d", eventDataSize);
   env->SetIntField(eventDataGblObj, eventTypeId, eventTypeVers);

   // return event data assumed to be copied by receiver if desired
   // call mLRecvrObj's receiveEvent method
   ALOGD("JNIListenReceiverListener::notify:() callMethod receiveEvent with mLRecvrObj=%p, gEventCallbackId=%p",
           mLRecvrObj, gEventCallbackId);
   env->CallVoidMethod(mLRecvrObj, gEventCallbackId, eventType, eventDataGblObj);
   if (env->ExceptionCheck()) {
      // skip exception in this case
      ALOGE("JNIListenReceiverListener::notify: ERROR an exception occurred while notifying Java app of an event!");
      env->ExceptionDescribe();
      env->ExceptionClear();
   }

cleanup_call_receiver_event:
   if (eventDataGblObj) {
      env->DeleteGlobalRef(eventDataGblObj);
   }
   if (eventDataObj) {
      env->DeleteLocalRef(eventDataObj);
   }
   if (dataArray) {
      env->DeleteLocalRef(dataArray);
   }
   if (eventData) {
      free((void *)eventData);
   }
   ALOGV("JNIListenReceiverListener::notify returns");
   return;
}

/*
 * JNI_OnLoadVM
 *
 * This function is called when Java Listen class explicitly loads Listen JNI liblistenjni.so.
 *
 * To ensure that no classes or object references are stale, various JNI variables required by
 * JNIListenReceiverListener::notify() are retrived at load time and stored in static variables.
 */
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint version = -1; // undefined

    ALOGV("JNI_OnLoad entered");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("JNI_OnLoad: ERROR: GetEnv failed\n");
        return version;
    }
    assert(env != NULL);
    gvm = vm;

    jclass eventDataClazz = env->FindClass("com/qualcomm/listen/ListenTypes$EventData");
    if ( NULL == eventDataClazz ) {
       ALOGE("JNI_OnLoad: ERROR EventData class not found");
       return version;
    }
    // saved as global references so it is not stale within notify()
    gEventDataClazz = reinterpret_cast<jclass>(env->NewGlobalRef(eventDataClazz) );
    ALOGV("JNI_OnLoad: save gEventDataClazz = %p as globalRef", gEventDataClazz);

    //
    // Get and save classes and methods needed by return events to Java ListenReceiver.
    // This class handles must be acquired from the app context, and saved.
    //
    jclass receiverClazz = env->FindClass("com/qualcomm/listen/ListenReceiver");
    if (NULL == receiverClazz) {
       ALOGE("JNI_OnLoad: ERROR FindClass(com/qualcomm/listen/ListenReceiver) failed !" );
       return version;
    }


    ALOGV("JNI_OnLoad: GetMethodID for receiveEvent in OnLoad");
    gEventCallbackId = env->GetMethodID(receiverClazz, "receiveEvent", "(ILcom/qualcomm/listen/ListenTypes$EventData;)V");
    if (env->ExceptionCheck()) {
      // skip exception in this case
      ALOGE("JNI_OnLoad: ERROR GetMethodID for receiveEvent threw an exception");
      env->ExceptionDescribe();
      env->ExceptionClear();
      // gEventCallbackId can be acquired later, so don't abort
      gEventCallbackId = NULL;
    }
    ALOGV("JNI_OnLoad: gEventCallbackId save as %p", gEventCallbackId);

    /* success -- return valid version number */
    version = JNI_VERSION_1_4;
    return version;
}

#ifdef __cplusplus
}
#endif


}; // namespace android
