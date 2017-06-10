/*
**
** Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights ReservedA
** Qualcomm Technologies Confidential and Proprietary.
**
** Not a Contribution, Apache license notifications and
** license are retained for attribution purposes only.
*/
/*
**
** Copyright 2008, The Android Open Source Project
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
 * ListenService
 *
 * Manages multiple ListenSessions
 * This is implemented as a Singleton Class !
 * First session that requests to be initialized, instantiates ListenService
 */

#define LOG_TAG "ListenService"
#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>

#include <cutils/atomic.h>
#include <cutils/properties.h> // for property_get

#include <utils/misc.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>
#include <utils/Errors.h>  // for status_t
#include <utils/String8.h>
#include <utils/SystemClock.h>
#include <utils/Vector.h>
#include <cutils/properties.h>

#include <hardware/audio.h>

#include <ListenService.h>
#include <ListenReceiver.h>

#define TIMES_TO_TRY_LOCK 5
#define TIME_BETWEEN_TRY_LOCK_ATTEMPTS 20000 // 20 ms

namespace { // anonymous namespace
using android::status_t;
using android::OK;
using android::BAD_VALUE;
using android::NOT_ENOUGH_DATA;
using android::Parcel;
}

namespace android {

// Static vars
uint16_t       ListenService::mNumInstances = 0;
Mutex          ListenService::mInstanceLock;
ListenService* ListenService::mSelf = (ListenService*)NULL;

/*
 * Constructor
 */
ListenService::ListenService()
     :  mHwDevice(NULL),
        mbListenFeatureEnabled(false),
        mbVoiceWakeupFeatureEnabled(false),
        mbNumSubFeaturesEnabled(0),
        mMadObserverSet(false),
        mMADEnabled(false)
{
    ALOGV("constructor entered, this = %p", this);
    status_t status = OK;

    // cap maximum sessions that ListenService can store
    // to MAX_LISTEN_SESSIONS - size of session array
    mMaxSysCaps.num_sessions = MAX_LISTEN_SESSIONS;

    // mLock is initialized when  Mutex::Autolock called
    memset(&mMasterCntrl, 0, sizeof(listen_master_cntrl_data_t) );
    for (int i=0; i < MAX_LISTEN_SESSIONS; i++) {
        memset(&mSessions[i], 0, sizeof(listen_session_data_t) );
    }
    memset(&mCurUsage, 0, sizeof(listen_resources_t) );

    ALOGD("setObitRecipient with ListenService ptr");
    setObitRecipient(this);
    ALOGV("constructor end");
}

/*
 * Destructor
 */
ListenService::~ListenService()
{
    ALOGV("destructor entered, this = %p", this);
    ALOGD("call clearObitRecipient");
    clearObitRecipient();
    status_t status = OK;
    status = unloadAudioInterface();
    if (status) {
       ALOGE("~ListenService: ERROR unloadAudioInterface() failed");
    }

    // currently nothing to clean up - SessionData[] is static array
    ALOGV("deconstructor end");
}

/*
 * Load Audio Interface
 *
 * Open Audio HW device that will be used for making HAL calls
 *
 * Return - HAL return status
 */
status_t ListenService::loadAudioInterface() {
    status_t status = OK;
    const hw_module_t *mod;
    mHwDevice = NULL;  // clear in case of error
    ALOGV("loadAudioInterface enter");
    if ( NULL != mHwDevice ) {
        ALOGV("Openning HW Device skipped because it is already open");
        return OK;
    }

    status = hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID, AUDIO_HARDWARE_MODULE_ID_PRIMARY, &mod);
    if (OK != status) {
        ALOGE("loadAudioInterface: ERROR hw_get_module_by_class() failed with %d", status);
        return status;
    }
    // Get the pointer to HAL
    // Requests that HAL be loaded if it is not already
    status = audio_hw_device_open(mod, &mHwDevice);
    if (OK != status) {
       ALOGE("loadAudioInterface: ERROR audio_hw_device_open() failed with %d",status);
        return status;
    }
    ALOGV("loadAudioInterface returned status %d", status);
    return status;
}

/*
 * Unload Audio Interface
 *
 * Release active (open) HAL Listen Sessions
 * Release active MAD Observer object
 * Close Audio HW device
 *
 * Return - HAL return status
 */
status_t ListenService::unloadAudioInterface()
{
    status_t status = OK;
    ALOGV("unloadAudioInterface enter");
    if ( NULL == mHwDevice ) {
        ALOGV("unloadAudioInterface: HW Device not open - nothing to do - return");
        return OK;
    }
    // close all open AudioHAL sessions
    for (int i=0; i < mMaxSysCaps.num_sessions; i++) {
       listen_session_t *iSession = mSessions[i].pAudioHALSession;
       if (NULL != iSession) {
          status = mHwDevice->close_listen_session(mHwDevice, iSession);
          if (OK != status) {
             ALOGE("unloadAudioInterface: ERROR close_listen_session(session=%p) failed, status %d", iSession, status);
          }
          freeSessionId(i);  // clears all mSessions[i] elements
       }
    }
    // release MAD observer
    if (mMadObserverSet) {
       ALOGV("releaseMasterControl: clear MAD Observer callback");
       status = mHwDevice->set_mad_observer(mHwDevice, NULL);
       mMadObserverSet = false;
       if (OK != status) {
          ALOGE("unloadAudioInterface: ERROR set_mad_observer(NULL) failed, status %d", status);
       }
    }
    // vote to close audio device
    status = audio_hw_device_close(mHwDevice);
    if (OK != status) {
       ALOGE("unloadAudioInterface: ERROR audio_hw_device_close() failed, status %d", status);
        return status;
    }
    ALOGV("unloadAudioInterface returned status %d", status);
    return status;
}

/*
 * Get reference to ListenService singleton
 *
 * Return - ptr to ListenService class object
 */
ListenService* ListenService::getInstance()
{
    status_t status = OK;
    mInstanceLock.lock();
    ALOGV("getInstance entered, mNumInstances currently %d", mNumInstances );
    if (0 == mNumInstances) {
       mSelf = new ListenService();
       if (NULL == mSelf) {
          ALOGE("getInstance: ERROR new ListenService() failed");
          return NULL;
       }
       ALOGV("getInstance ListenService created");
       // load HW Device interface to AudioHAL
       status = mSelf->loadAudioInterface();
       if (status) {
          ALOGE("getInstance: ERROR loadAudioInterface() failed, status %d", status);
          return NULL;
       }
       ALOGV("getInstance loadAudioInterface successful");
    }
    mNumInstances++;
    ALOGV("getInstance returns %p, mNumInstances set to %d", mSelf, mNumInstances);
    mInstanceLock.unlock();
    return mSelf;
}

/*
 * Release an instance of ListenService
 */
void ListenService::releaseInstance()
{
    mInstanceLock.lock();
    ALOGV("releaseInstance entered, mNumInstances currently %d", mNumInstances );
    mNumInstances--;
    if (0 == mNumInstances) {
        delete mSelf;
    }
    ALOGV("releaseInstance returns, mNumInstances set to %d", mNumInstances);
    mInstanceLock.unlock();
    return;
}

/*
 * Create ListenService
 *
 * Called by Server Manager to instatiate singleton ListenService during boot-up.
 * Adds "listen.service" as a client of Server Manager
 */
void ListenService::instantiate()
{
    ALOGV("instantiate entered");
    ListenService* singleton = ListenService::getInstance();
    // Add this Listen Service as client to Service Manager
    ALOGV("instantiate: call addService");
    defaultServiceManager()->addService(
            String16("listen.service"), singleton);
    ALOGV("instantiate returns");
    return;
}

/*
 * Initialize the MasterControl receiver client object maintained in Service
 *
 * Only one MasterControl object is maintained by ListenService for all applications.
 *
 * Param [in] pid - processId of app thread ListenReceiver created under
 *            setting this to non-zero is a prerequist to ListenService send event notification to client.
 * Param [in] client - ptr back to IListenReceiver for particular ListerReceiver client
 *            events are to be sent to.
 *
 * Return - returns true if no other application is using the single MasterControl instance
 *          returns false if request for MasterControl fails.
 */
bool ListenService::initMasterControl(
                   pid_t pid,
                   const sp<IListenReceiver>& client)
{
   ALOGV("initMasterControl entered, pid %d, client %p",  pid, client.get());
   listen_status_enum_t  status =  LISTEN_SUCCESS;
   status = requestMasterControl();
   if (LISTEN_SUCCESS != status){
      ALOGE("initMasterControl: ERROR requestMasterControl() failed, status %d", status);
       return false;
   }
   mMasterCntrl.receiver.pid = pid;
   ALOGV("initMasterControl sets client %p", client.get());
   mMasterCntrl.receiver.client = client;
   setListenClientContext(client);
   ALOGV("initMasterControl returns true");
   return true;
}

/*
 * Initialize a Session receiver client
 *
 * Param [in] sessionType - one of Listen typeslisten_status_enum_t types
 * Param [in] pid - processId of app thread native ListenReceiver object created under.
 *            Setting this to non-zero will ensure that event notification is sent to client.
 * Param [in] client - ptr back to IListenReceiver for particular ListerReceiver
 *
 * Return - status
 */
listen_session_id_t ListenService::initSession(
                   listen_session_enum_t sessionType,
                   pid_t pid,
                   const sp<IListenReceiver>& client )
{
   ALOGV("initSession entered, pid %d, client %p",  pid, client.get());
   listen_status_enum_t  status = LISTEN_SUCCESS;
   listen_session_id_t nListenSessionId = UNDEFINED;
   switch(sessionType) {
      case LISTEN_VOICE_WAKEUP_SESSION:
         status = addSession(&nListenSessionId);
         if (LISTEN_SUCCESS != status){
            ALOGE("initSession: ERROR add session failed, status %d", status);
            return UNDEFINED;
         }
         break;
      default:
         ALOGE("initSession: session type %d unknown", sessionType);
         break;
   }
   if ((UNDEFINED == nListenSessionId) || (nListenSessionId >= mMaxSysCaps.num_sessions) ) {
      ALOGE("initSession: ERROR bad SessionId (%d)", nListenSessionId);
      return UNDEFINED;
   }
   mSessions[nListenSessionId].receiver.pid = pid;
   ALOGV("initSession sets client %p", client.get());
   mSessions[nListenSessionId].receiver.client = client;
   setListenClientContext(client);
   ALOGV("initSession returns sessionId %d", nListenSessionId);
   return nListenSessionId;
}

/*
 * Set Listen Parameter
 *
 * The function accepts a single parameter type and its data value(s).
 * Currently the only parameters that can be set are global parameters
 * Only clients with MasterControl permission can set global parameters
 * This sets the parameters with ListenService.
 * Enabling a feature parameter does NOT cause MAD to be enabled.
 *     That will be done only after both features are enabled and
 *     a soundModel is registered
 * Disabling a feature will cause MAD to be disabled.
 * Simply enabling feature should not cause MAD to be enabled;
 *     that will be triggered by registerSoundModel under the right conditions.
 * Param [in]  type - one of Listen of type listen_param_enum_t
 * Param [in]  request - parcel data structure containing input data field(s)
 * Return - errors
 *      LISTEN_ENO_GLOBAL_CONTROL - returned if session does not have permission to set global params
 *      LISTEN_BAD_PARAM
 */
listen_status_enum_t ListenService::setParameter(listen_param_enum_t  eParamType,
                                     int32_t              value)
{
    ALOGV("setParameter(%d, %d) entered", eParamType, value);
    listen_status_enum_t  status = LISTEN_SUCCESS;
    status_t aHALstatus;

    Mutex::Autolock lock(mLock);
    switch(eParamType) {
       case LISTEN_PARAM_LISTEN_FEATURE_ENABLE:
       {
          // used to enable or disable Listen Feature
          ALOGV("setParameter(LISTEN_PARAM_LISTEN_FEATURE_ENABLE, %d)", value);
          notifyOfFeatureChange(eParamType, value);
          if (DISABLE == value)  {
             ALOGD("setParameter: Disable ListenFeature because DISABLE=value");
             // Disabling ListenFeature will cause MAD to be disabled
             if (mMADEnabled) {
                // If HW MAD is current enabled, when
                //    client requested Listen Feature to be disabled
                //    all SoundModels must be deregistered, then
                //    MAD should be explicitly disabled
                ALOGD("setParameter: call deregisterAll");
                status = deregisterAll();
                if (LISTEN_SUCCESS != status) {
                   ALOGE("setParameter: ERROR deregisterAll failed w/%d", status);
                }
                ALOGD("setParameter: mMADEnabled so call disableMAD");
                aHALstatus = disableMAD();
                if (OK != aHALstatus) {
                   ALOGE("setParameter: ERROR disableMAD failed w/%d", aHALstatus);
                } else {
                   mMADEnabled = false;
                   ALOGV("setParameter: MAD disable, flag set");
                }
             }
          }
          mbListenFeatureEnabled = (bool)value;
          ALOGD("setParameter: mbListenFeatureEnabled set to %d", (int)mbListenFeatureEnabled);
       } break;

       case LISTEN_PARAM_VOICE_WAKEUP_FEATURE_ENABLE:
       {
          // used to enable or disable VoiceWakeup Feature
          ALOGV("setParameter(LISTEN_PARAM_VOICE_WAKEUP_FEATURE_ENABLE)");
          notifyOfFeatureChange(eParamType, value);
          if ( DISABLE == value ) {
             ALOGV("setParameter: Disable VWUFeature because DISABLE=value");
             if (mbVoiceWakeupFeatureEnabled) {
                mbNumSubFeaturesEnabled--;
             }
             // Deregister all currently registered soundmodels
             ALOGD("setParameter: call deregisterAll");
             status = deregisterAll();
             if (LISTEN_SUCCESS != status) {
                ALOGE("setParameter: ERROR deregisterAll failed w/%d", status);
             }
          } else {
             if (!mbVoiceWakeupFeatureEnabled) {
                mbNumSubFeaturesEnabled++;
             }
          }
          mbVoiceWakeupFeatureEnabled = (bool)value;
          ALOGD("setParameter: mbVoiceWakeupFeatureEnabled set to %d",
                      (int)mbVoiceWakeupFeatureEnabled);
       } break;

       default:
          ALOGE("setParameter: ERROR unknown key (%d)", eParamType);
          return LISTEN_EBAD_PARAM;
          break;
    }
    if ( mMADEnabled && (0==mbNumSubFeaturesEnabled) )
    {
       // If HW MAD is current enabled, yet all the sub-features are disabled
       //    MAD should be disabled
       ALOGV("setParameter: MAD disable because all sub-features disabled");
       aHALstatus = disableMAD();
       if (OK != aHALstatus) {
          ALOGE("setParameter: ERROR disableMAD failed w/%d", aHALstatus);
       } else {
          mMADEnabled = false;
          ALOGV("setParameter: MAD disable, flag set");
       }
    }
    ALOGV("setParameter returns %d", status);
    return status;
}

/*
 * Get Listen Parameter
 *
 * The form of this function accepts a single parameter type and outputs data field(s)
 * All clients may query the any parameter
 * This does NOT call AudioHAL::getListenParameters()
  *
 * Param [in] eParamType - one of Listen listen_param_enum_t types
 * Return - value
 */
int32_t ListenService::getParameter(listen_param_enum_t eParamType) {
    ALOGV("getParameter(%d) enter", eParamType);
    int32_t retVal = UNDEFINED;
    switch(eParamType) {
       case LISTEN_PARAM_LISTEN_FEATURE_ENABLE:
          // place flag into reply parcel
          retVal = (int32_t)mbListenFeatureEnabled;
          break;
       case LISTEN_PARAM_VOICE_WAKEUP_FEATURE_ENABLE:
          // place flag into reply parcel
          retVal = (int32_t)mbVoiceWakeupFeatureEnabled;
          break;
       default:
          ALOGE("getParameter: ERROR unknown key (%d)", eParamType);
          retVal = UNDEFINED;
          break;
    }
    ALOGV("getParameter(%d) returned %d", eParamType, retVal);
    return retVal;
}

/* Request Master Control
 *
 * Acquire the ability to set master control parameters
 * The first client that has permission to requests Master Control
 *     gets to set Listen global parameters
 * These parameters are global not session based
 *
 * Return - errors
 *       LISTEN_ERESOURCE_NOT_AVAILABLE  - global control already acquired by another client
 */
listen_status_enum_t ListenService::requestMasterControl()
{
    ALOGV("requestMasterControl entered, mMasterCntrl.bGranted currently %d", mMasterCntrl.bGranted);
    status_t aHALstatus;

    Mutex::Autolock lock(mLock);
    if ( mMasterCntrl.bGranted ) {
       ALOGE("requestMasterControl: ERROR MasterControl priviledge already granted, return RESOURCE_NOT_AVAILABLE");
       return LISTEN_ERESOURCE_NOT_AVAILABLE;
    }
    if (NULL == mHwDevice) {
       // HW Device did not be initialized during getInstance
       ALOGE("requestMasterControl: ERROR audioHAL device not initialized, return RESOURCE_NOT_AVAILABLE");
       return LISTEN_ERESOURCE_NOT_AVAILABLE;
    }

    if (!mMadObserverSet) {
       ALOGV("requestMasterControl: set MAD Observer mHwDevice %p, CB %p", mHwDevice, eventCallback);
       aHALstatus = mHwDevice->set_mad_observer(mHwDevice, eventCallback);
       if (OK != aHALstatus) {
          ALOGE("requestMasterControl: ERROR Set MAD Observer failed, status %d",aHALstatus);
          return LISTEN_EFAILURE;
       }
       mMadObserverSet = true; // set only if no error occurred
    }
    mMasterCntrl.bGranted = true;
    ALOGV("requestMasterControl returns SUCCESS");
    return LISTEN_SUCCESS;
}

/*
 * Release Master Control
 *
 * Return - status
 */
listen_status_enum_t ListenService::releaseMasterControl()
{
    ALOGV("releaseMasterControl entered");
    Mutex::Autolock lock(mLock);
    status_t aHALstatus;
    mMasterCntrl.bGranted = false;

    // clear MasterControl parameters
    ALOGD("call clearListenClientContext");
    clearListenClientContext(mMasterCntrl.receiver.client);
    memset(&mMasterCntrl, 0, sizeof(listen_master_cntrl_data_t) );

    if (NULL == mHwDevice) {
       // HW Device was not initialized during getInstance
       ALOGE("releaseMasterControl: ERROR audioHAL device not initialized");
       return LISTEN_ERESOURCE_NOT_AVAILABLE;
    }
    // if no sessions are active, then clear MAD Observer callback
    // Since there are no active session, MAD should already have be disabled
    if ( mMadObserverSet && (0 == mCurUsage.num_sessions) ) {
       ALOGV("releaseMasterControl: clear MAD Observer callback");
       aHALstatus = mHwDevice->set_mad_observer(mHwDevice, NULL);
       mMadObserverSet = false;
       if (OK != aHALstatus) {
          ALOGE("releaseMasterControl: ERROR Clearing MAD Observer failed");
          return LISTEN_EFAILURE;
       }
    }
    ALOGV("releaseMasterControl returns");
    return LISTEN_SUCCESS;
}

// ----------------------------------------------------------------------------
//  Session specific Methods
// ----------------------------------------------------------------------------

/*
 * Creates a new Listen Native Session managed by this Listen Native Service
 *
 * If no sessions are currently active, instantiate ListenService
 * Limit the number of sessions that can be active to a maximum number.
 *
 * Param [out] sessionId - returns unique session handle return event with option data to client
 * Return - errors
 *       LISTEN_ERESOURCE_NOT_AVAILABLE  - maximum available sessions already instanced
 */
listen_status_enum_t ListenService::addSession(listen_session_id_t* sessionId)
{
   ALOGV("addSession entered");
   listen_session_id_t     freeId = UNDEFINED;
   *sessionId = UNDEFINED;  // set in case it fails
   status_t aHALstatus;

   if (mMaxSysCaps.num_sessions <= mCurUsage.num_sessions) {
       // maximum number of sessions already reached
       ALOGE("addSession: ERROR maximum number of sessions (%d) already active",
                    mCurUsage.num_sessions);
       return LISTEN_ERESOURCE_NOT_AVAILABLE;
   }
   if (NULL == mHwDevice) {
       // HW Device did not be initialized during getInstance
       ALOGE("addSession: ERROR audioHAL not initialized");
       return LISTEN_ERESOURCE_NOT_AVAILABLE;
   }

   Mutex::Autolock lock(mLock);
   freeId = getAvailableSessionId();
   if ( (UNDEFINED == freeId) || (freeId >= mMaxSysCaps.num_sessions) ){
      ALOGE("addSession: ERROR all available session(s) already used");
       return LISTEN_ERESOURCE_NOT_AVAILABLE;
   }
   // For now, since only one session type is supported, this is hardcoded
   mSessions[freeId].type = LISTEN_VOICE_WAKEUP_SESSION;
   *sessionId = freeId; // return valid session handle

   if (!mMadObserverSet) {
      ALOGV("addSession: set MAD Observer mHwDevice %p, CB %p", mHwDevice, eventCallback);
      aHALstatus = mHwDevice->set_mad_observer(mHwDevice, eventCallback);
      if (OK != aHALstatus) {
         ALOGE("addSession: ERROR Set MAD Observer failed, status %d",aHALstatus);
         return LISTEN_EFAILURE;
      }
      mMadObserverSet = true;  // only if no error occurred
   }
   ALOGV("addSession:... uses and returns sessionId %d", freeId);
   return LISTEN_SUCCESS;
}

/*
 * Remove session from Listen Native Service
 *
 * Param [in]  session id - handle of session
 * Return - status
 *       LISTEN_EBAD_PARAM if id invalid
 *       silently return success if session with id not active
 */
listen_status_enum_t ListenService::destroySession(listen_session_id_t sessionId)
{
    ALOGV("destroySession entered, sessionId %d", sessionId);
    status_t             aHALstatus;
    listen_status_enum_t eStatus = LISTEN_SUCCESS;
    if (sessionId >= mMaxSysCaps.num_sessions) {
        return LISTEN_EBAD_PARAM;
    }
    Mutex::Autolock lock(mLock);
    // if this function had to waited for release lock then check
    //    that this session hasn't been destroyed by another thread
    if (!mSessions[sessionId].bActive) {
        ALOGV("destroySession: session likely just destroyed by another thread");
        return LISTEN_SUCCESS; // silently ignore request
    }

    // closes this listen session within AudioHAL
    if (NULL != mSessions[sessionId].pAudioHALSession) {
        if (mSessions[sessionId].bSoundModelRegistered ) {
            aHALstatus = mSessions[sessionId].pAudioHALSession->deregister_sound_model(mSessions[sessionId].pAudioHALSession);
            if (OK != aHALstatus) {
                ALOGE("destroySession: ERROR audio HAL deregister_sound_model failed");
            }
        }
       aHALstatus = mHwDevice->close_listen_session(mHwDevice, mSessions[sessionId].pAudioHALSession);
       if (OK != aHALstatus) {
          ALOGE("destroySession: ERROR close_listen_session failed");
          eStatus = LISTEN_EFAILURE;
          // drop down to clear session entry in table
       }
    }

    ALOGD("call clearListenClientContext");
    clearListenClientContext(mSessions[sessionId].receiver.client);
    freeSessionId(sessionId);  // clears all mSessions[sessionId] elements

    // if this session being destroyed is the last one active...
    if (0 == mCurUsage.num_sessions) {
       // Master Control is not active disable, clear observer CB ptr
       if (mMadObserverSet && !mMasterCntrl.bGranted) {
          ALOGV("destroySession: clear MAD Observer callback");
          aHALstatus = mHwDevice->set_mad_observer(mHwDevice, NULL);
          if (OK != aHALstatus) {
             ALOGE("destroySession: ERROR Clearing MAD Observer failed w/%d", aHALstatus);
          }
          mMadObserverSet = false;
       }
       if (mMADEnabled) {
          // disable MAD since no sessions are active
          aHALstatus = disableMAD();
          if (OK != aHALstatus) {
             ALOGE("destroySession: ERROR disableMAD failed w/%d", aHALstatus);
          }
          mMADEnabled = false;
       }
    }
    ALOGV("destroySession returns %d", eStatus);
    return eStatus;
}

/*
 * registerSoundModel
 *
 * Specifies the sound model that Listen engine uses to detect desired keyword/sound
 *    for this session.
 * Only one model may be registed per session
 *
 * SM Registration can occur if Listen Feature and associated Session feature are enabled
 * When the first SM is registered, only then is MAD enabled.
 * Due to design of ListenDriver, MAD must be enabled before a Session can be openned.
 *
 * If no error conditions exist this method makes calls to HAL to:
 *    Enabled MAD by setting HW Device "mad_on" parameter
 *    Open new Listen Session HAL object
 *    Set Session observer for receiving HAL Listen events
 *    Register SoundModel
 *
 * Param [in]  session id - handle of session
 * Param [in]  pSoundModelParams - pointer to struct with all sound model parameters
 *
 * Return - status
 */
listen_status_enum_t ListenService::registerSoundModel(
                                listen_session_id_t          sessionId,
                                listen_sound_model_params_t* pSoundModelParams)
{
    ALOGV("registerSoundModel entered, session Id = %d", sessionId);
    listen_status_enum_t      eStatus = LISTEN_SUCCESS;
    status_t                  aHALstatus;
    listen_session_data_t *   pSession;
    listen_sound_model_enum_t soundModelType;
    bool                      bMADEnabled = false;
    // temp ptr to session to use for registeration
    listen_session_t *        pListenHALSession = NULL;
    // ptr to new session created by this method
    listen_session_t *        pNewAudioHALSession = NULL;
    listen_open_params_t      open_session_params;
    int                       num_conf_levels;
    if ( NULL == pSoundModelParams ) {
       ALOGE("registerSoundModel - ERROR soundModelParams ptr NULL");
       return LISTEN_EBAD_PARAM;
    } else {
        soundModelType = pSoundModelParams->sound_model_type;
        ALOGD("registerSoundModel: pSoundModelParams: ");
        ALOGD("    soundModel type %d", (int)pSoundModelParams->sound_model_type);
        ALOGD("    detectionMode %d", (int)pSoundModelParams->detection_mode);
        if (pSoundModelParams->enable_buffering)
            ALOGD("    enable buffering flag true");
        else
            ALOGD("    enable buffering flag false");
        ALOGD("    num_keywords %d ", (int)pSoundModelParams->num_keywords);
        ALOGD("    num_user_pairings %d ", (int)pSoundModelParams->num_user_pairings);
        ALOGD("    detectFailure %d ", (int)pSoundModelParams->detect_failure);
        if (NULL != pSoundModelParams->sound_model_data) {
           ALOGD("    pSoundModelData %p ", pSoundModelParams->sound_model_data);
           ALOGD("        SoundModel data %p, size %d,",
                  pSoundModelParams->sound_model_data->p_data,
                  pSoundModelParams->sound_model_data->size );
        } else {
            ALOGE("registerSoundModel: pSoundModelData is NULL");
            return LISTEN_EBAD_PARAM;
        }
    }
    if ( sessionId >= mMaxSysCaps.num_sessions ) {
       ALOGE("registerSoundModel - ERROR session id %d not valid", sessionId);
       return LISTEN_EBAD_PARAM;
    }
    if ( NULL == pSoundModelParams ) {
       ALOGE("registerSoundModel - ERROR soundModelParams ptr NULL");
       return LISTEN_EBAD_PARAM;
    }

    // check sound model data structure
    if (NULL == pSoundModelParams->sound_model_data->p_data) {
       ALOGE("ERROR: sound model data is NULL");
       return LISTEN_EBAD_PARAM;
    }
    if (0 == pSoundModelParams->sound_model_data->size) {
       ALOGE("ERROR: sound model size is 0");
       return LISTEN_EBAD_PARAM;
    }

    // check Mode is ok
    if ( ( LISTEN_MODE_KEYWORD_ONLY_DETECTION != pSoundModelParams->detection_mode) &&
         ( LISTEN_MODE_USER_KEYWORD_DETECTION != pSoundModelParams->detection_mode)  ) {
            ALOGE("ERROR: detectionMode %d is not valid", (int)pSoundModelParams->detection_mode);
       return LISTEN_EBAD_PARAM;
    }

    // check each unsigned int Confidence level in the range 0 - 100
    num_conf_levels = pSoundModelParams->num_keywords + pSoundModelParams->num_user_pairings;

    for (int i = 0; i < num_conf_levels; i++) {
       // unsigned percent values already forced to be > 0
       if (pSoundModelParams->min_conf_levels[i] > 100) {
          ALOGE("Confidence level %d value %d exceeds range between 0 and 100",
                  i, pSoundModelParams->min_conf_levels[i]);
          return LISTEN_EBAD_PARAM;
       }
    }

    Mutex::Autolock lock(mLock);

    // in order for SoundModel to be registered,
    //    both Listen and VoiceWakeupSession Feature must be enabled
    if ( !mbListenFeatureEnabled || !mbVoiceWakeupFeatureEnabled) {
       ALOGE("ListenService:registerSoundModel - ERROR both Listen and VoiceWakeup Feature must be enabled");
       return LISTEN_EFEATURE_NOT_ENABLED;
    }


    pSession = &mSessions[sessionId];
    if ( ! pSession->bActive )  {
       ALOGE("ListenService:registerSoundModel - ERROR session id %d not active", sessionId);
       return LISTEN_ESESSION_NOT_ACTIVE;
    }
    if ( pSession->bSoundModelRegistered ) {
       ALOGE("ListenService:registerSoundModel - ERROR a SoundModel already registered for this Session");
       return LISTEN_ESOUNDMODEL_ALREADY_REGISTERED;
    }

    // %%% TODO need to CHECK that number of KW and User-pairings in SM will not exceed system wide limits


    // Enable MAD if it is not already enabled
    if (! mMADEnabled) {
       ALOGV("ListenService:registerSoundModel explicitly enable MAD");
       aHALstatus = enableMAD();
       if (OK != aHALstatus) {
          ALOGE("ListenService:registerSoundModel - ERROR Set MAD Enable failed");
          return LISTEN_EFAILURE;
       }
       bMADEnabled = true;
    }

    // create a ListenSession within AudioHAL if not already done
    //    for this Service session object
    if ( NULL == pSession->pAudioHALSession )  {
       ALOGV("Call open_listen_session (%p)", mHwDevice);
       if (LISTEN_SM_TYPE_SVA == soundModelType) {
           open_session_params.app_id = LISTEN_APP_TYPE_SVA;
       } else if (LISTEN_SM_TYPE_SVA_V2 == soundModelType) {
           open_session_params.app_id = LISTEN_APP_TYPE_SVA_V2;
       } else {
           ALOGV("SM Type %d is not recognized", soundModelType);
           return LISTEN_EUNSUPPORTED_SOUNDMODEL;
       }
       open_session_params.num_keywords = pSoundModelParams->num_keywords;
       open_session_params.num_user_pairings = pSoundModelParams->num_user_pairings;
       aHALstatus = mHwDevice->open_listen_session(mHwDevice,
                      &open_session_params, &pNewAudioHALSession);
       if (OK != aHALstatus) {
          ALOGE("registerSoundModel: ERROR open_listen_session failed");
          goto cleanupHALObjects;
       }
       if (NULL == pNewAudioHALSession) {
          ALOGE("registerSoundModel: ERROR open_listen_session output NULL ptr");
          goto cleanupHALObjects;
       }
       ALOGV("save HAL session ptr %p in mSessions[%d]", pNewAudioHALSession, sessionId) ;
       // Client sets the callback function for listen session
       aHALstatus = pNewAudioHALSession->set_session_observer(pNewAudioHALSession,
                        eventCallback, pNewAudioHALSession);
       if (OK != aHALstatus) {
          ALOGE("registerSoundModel: ERROR set_session_observer failed");
          goto cleanupHALObjects;
       }
       // new session successfully created and initialized
       pListenHALSession = pNewAudioHALSession;
    } else {
       // use previous created Session
       pListenHALSession = pSession->pAudioHALSession;
    }

    // save the number of keyword and active user+keyword pairs of the SM being registered
    pSession->num_keywords = open_session_params.num_keywords =
                pSoundModelParams->num_keywords;
    pSession->num_user_kw_pairings = open_session_params.num_user_pairings =
                pSoundModelParams->num_user_pairings;

    // Now ask audioHAL to register the sound model
    ALOGV("ListenService:registerSoundModel - call HAL:register_sound_model()");
    aHALstatus = pListenHALSession->register_sound_model(
                           pListenHALSession, pSoundModelParams);
    if (OK != aHALstatus) {
       ALOGE("ListenService:registerSoundModel - ERROR audio HAL register_sound_model failed");
       // we have already checked above that soundmodel has not been registered for this session
       //     so if register_sound_model fails, we should release the session - it can't be used
       if (aHALstatus == -EINVAL) {
           // ListenHAL based on platform capabilities checks that the number of resources
           //   (keywords/active pairs/...).  If this error returned, then the resources require
           //   to register this SM exceeds platform limits
           eStatus = LISTEN_ERESOURCE_NOT_AVAILABLE;
       }
       goto cleanupHALObjects;
    }

    // everything successful so set flags denote MAD enabled and SM registered
    if (bMADEnabled) {
       mMADEnabled = true;
       // send an Listen "running" event to App after both MAD enabled and SM register successfully
       ALOGV("registerSoundModel: notifyOfMADChange");
       notifyOfMADChange(true);
    }
    pSession->bSoundModelRegistered = true;
    if (pNewAudioHALSession) {
       // save ptr to newly created session
       pSession->pAudioHALSession = pNewAudioHALSession;
    }
    ALOGV("registerSoundModel successful");
    return LISTEN_SUCCESS;

cleanupHALObjects:
    // Something failed at HAL level so clean up all created/reserved objects
    if (pNewAudioHALSession) {
       mHwDevice->close_listen_session(mHwDevice, pNewAudioHALSession);
       // closing session object will clear session observer if set
       // pSession->pAudioHALSession set before register not clear
    }
    if (bMADEnabled && !mMADEnabled) {
       // if MAD was enabled due to this SM registeration (it was not registered before)
       //    but some error occurred, disable MAD
       disableMAD();
    }
    freeSessionId(sessionId);  // clears all fields in mSessions[sessionId] entry
    if (eStatus != LISTEN_SUCCESS) {
        ALOGE("registerSoundModel failed with status ", eStatus);
        return eStatus;
    } else {
    ALOGE("registerSoundModel failed");
    return LISTEN_EFAILURE;
}
}

/*
 * deregisterSoundModel
 *
 * Clears SoundModel registeration for this Listen Session
 *
 * When the SM is deregister by the client, we should not disable
 *     MAD unless there are NO other session that have registered SMs
 * Keep the Listen Session active even if MAD disabled
 *
 * NOTE: a notification is NOT sent to client when client calls
 *     deregisterSoundModel itself
 *
 * Param [in]  session id - handle of session
 * Param [in]  soundModelId - sound model id return by ListenSoundModel.create()
 *                 sound model id is not retained by Listen Native Service
 * Return - status
 */
listen_status_enum_t ListenService::deregisterSoundModel(
                               listen_session_id_t sessionId)
{
    ALOGV("deregisterSoundModel entered");
    status_t aHALstatus = OK;
    bool     bKeepMADEnabled = false;

    if ( sessionId >= mMaxSysCaps.num_sessions ) {
       ALOGE("deregisterSoundModel: ERROR session id %d not valid", sessionId);
       return LISTEN_EBAD_PARAM;
    }

    Mutex::Autolock lock(mLock);  // @@@ add 2/13
    listen_session_data_t * pSession = &mSessions[sessionId];
    if ( ! pSession->bActive )  {
       ALOGE("deregisterSoundModel: ERROR session id %d not active", sessionId);
       return LISTEN_ESESSION_NOT_ACTIVE;
    }
    if ( ! pSession->bSoundModelRegistered ) {
       ALOGE("deregisterSoundModel: ERROR SoundModel not registered for this Session");
       return LISTEN_ESOUNDMODEL_NOT_REGISTERED;
    }
    if ( NULL == pSession->pAudioHALSession ) {
       ALOGE("deregisterSoundModel: ERROR pAudioHALSession is NULL");
       return LISTEN_ESESSION_NOT_ACTIVE;
    }

    aHALstatus = pSession->pAudioHALSession->deregister_sound_model(pSession->pAudioHALSession);
    if (OK != aHALstatus) {
       ALOGE("deregisterSoundModel: ERROR audio HAL deregister_sound_model failed");
       // fall thru and mark session as deregistered even if error occurs
    }

    pSession->bSoundModelRegistered = false;

    // Check if MAD can be disabled because there isn't a SoundModel currently registered
    for (int indx=0; indx < mMaxSysCaps.num_sessions; indx++) {
        pSession = &mSessions[indx];
        if (pSession->bActive && pSession->bSoundModelRegistered)  {
           bKeepMADEnabled = true;
           ALOGV("deregisterSoundModel: there will be at least one registered SM after this one deregistered");
           break;
        }
    }
    if ( mMADEnabled && ! bKeepMADEnabled ) {
       ALOGV("deregisterSoundModel: Disable MAD because there are no registered SMs");
       aHALstatus = disableMAD();
       mMADEnabled = false;
    }

    ALOGV("deregisterSoundModel returns");
    return LISTEN_SUCCESS;
}

/*
 * readBuffer
 *
 * Retrieves the speech buffer collected after a keyword is detected
 *
 * If buffering is enabled during SoundModel registration, then audio after
 * the end of the keyword phrase is written to buffer immediate after detection
 * and continues until the buffer is filled or the application explicitly
 * stops buffering.
 *
 * This method is synchronous and can be called after receiving
 * DETECTION_SUCCESS event.
 *
 * This method is blocking - the buffer will be returned when samples available
 * matches samples requested, or end of buffer is reached.
 *
 * Buffer will be held until next detection or until session destroyed.
 *
 * Param [in] sessionId - handle of Listen Native Session
 * Param [in] numSamples - number of requested shorts
 * Param [in] pSampleBuff [out] audio captured after keyword is written to this buffer
 * Param [out] pSamplesRead [out] number of samples written to this buffer
 *
 * Return - error
 *      STATUS_SUCCESS
 *      STATUS_EBUFFERING_NOT_ENABLED
 *      STATUS_EBUFFERED_DATA_INCOMPLETE
 *      STATUS_ENOT_BUFFERING
 *      STATUS_ESOUNDMODEL_NOT_REGISTERED
 */
listen_status_enum_t ListenService::readBuffer(
            listen_session_id_t          sessionId,
            int32_t                      numSamplesToRead,
            int16_t*                     pSampleBuff,
            int32_t*                     pSamplesRead)
{
    status_t aHALstatus = OK;
    uint32_t  bytesRead = 0;

    ALOGV("readBuffer entered for sessionId %d - parameters are: numSample %d, pSampleBuffer %p",
           sessionId, numSamplesToRead, pSampleBuff);
    if ((NULL == pSampleBuff) || (NULL == pSamplesRead)) {
        ALOGE("readBuffer: ERROR ptr param null");
        return LISTEN_EBAD_PARAM;
    }
    // mMaxSysCaps.num_sessions does not change after ListenService is instanciated
    if (sessionId >= mMaxSysCaps.num_sessions) {
        ALOGE("readBuffer: ERROR session Id invalid %d", sessionId);
        return LISTEN_EBAD_PARAM;
    }
    uint8_t * pByteBuff = (uint8_t *)pSampleBuff;
    bytesRead = numSamplesToRead*2;
    mLock.lock();
    listen_session_data_t * pSession = &mSessions[sessionId];
    ALOGV("readBuffer: HAL called, pSes->pAudioHALSession %p, pByteBuff %p, read %d bytes ",
                  pSession->pAudioHALSession, pByteBuff, bytesRead);
    mLock.unlock();  // explicitly unlock Service so stop_buffer, deregisterSM, closeSession
                     // can be called while readBuffer is being performed.
    aHALstatus = pSession->pAudioHALSession->read_buffer(pSession->pAudioHALSession,
                         pByteBuff,
                         &bytesRead);
    ALOGV("readBuffer HAL returned bytes read %d",bytesRead);
    *pSamplesRead = bytesRead / 2;  // convert num bytes to number to 16-bit samples
    if ( *pSamplesRead > numSamplesToRead ) {
        ALOGE("%s: number of samples read > requested", __func__);
        *pSamplesRead = numSamplesToRead;  // clamp output samples read to num samples to read
        return LISTEN_EFAILURE;
    }
    if (-EBUSY == aHALstatus) {
       // SSR or Concurrency has cause ListenEngine (and buffering) to be stopped
       return LISTEN_ENOT_BUFFERING;  // currently not doing buffering
    } else if (-EPIPE == aHALstatus) {
       return LISTEN_EBUFFERING_DATA_INCOMPLETE; // data in buffer is not complete; e.g. overflow occurred
    } else if (OK != aHALstatus) {
       ALOGE("readBuffer: ERROR audio HAL read_buffer failed w/ %d", aHALstatus);
       return LISTEN_EFAILURE;
    }
    ALOGV("readBuffer exit");
    return LISTEN_SUCCESS;
}
/*
 * stopBuffering
 *
 * Stops the Listen Engine buffering
 *
 * Param [in]  session id - handle of Listen Native Session
 *
 * Return - error
 *      LISTEN_ESOUNDMODEL_NOT_REGISTERED
 */
listen_status_enum_t ListenService::stopBuffering(listen_session_id_t sessionId)
{
    ALOGV("stopBuffering entered for session id %d", sessionId);
    status_t aHALstatus = OK;
    Mutex::Autolock lock(mLock);
    if (sessionId >= mMaxSysCaps.num_sessions) {
        ALOGE("readBuffer: ERROR session Id invalid %d", sessionId);
        return LISTEN_EBAD_PARAM;
    }
    listen_session_data_t * pSession = &mSessions[sessionId];
    // check ERRORS
    // LISTEN_EBUFFERING_NOT_ENABLED = -8,  // buffering is not enabled for this session
    aHALstatus = pSession->pAudioHALSession->stop_buffering(pSession->pAudioHALSession);
    if (OK != aHALstatus) {
       ALOGE("stopBuffering: ERROR audio HAL stop_buffer failed w/ %d", aHALstatus);
       return LISTEN_EFAILURE;
    }
    ALOGV("stopBuffering exit");
    return LISTEN_SUCCESS;
}

/*
 * stopDetection
 *
 * Stops Detection on this session only
 *
 * Param [in]  session id - handle of Listen Native Session
 *
 * Return - error
 *      LISTEN_ESOUNDMODEL_NOT_REGISTERED
 */
listen_status_enum_t ListenService::stopDetection(listen_session_id_t sessionId)
{
    ALOGV("stopDetection entered for session id %d", sessionId);
    ALOGD("stopDetection STUBBED");
    ALOGV("stopDetection exit");
    return LISTEN_SUCCESS;
}

/*
 * startDetection
 *
 * Re-starts Detection on this session only
 *
 * Param [in]  session id - handle of Listen Native Session
 *
 * Return - error
 *      LISTEN_ESOUNDMODEL_NOT_REGISTERED
 */
listen_status_enum_t ListenService::startDetection(listen_session_id_t sessionId)
{
    ALOGV("startDetection entered for session id %d", sessionId);
    ALOGD("startDetection STUBBED");
    ALOGV("startDetection exit");
    return LISTEN_SUCCESS;
}
//
// Helper Functions
//

/*
 * Get Session Id of an Available Session
 *
 * Return - Id number from zero to (mMaxSysCaps.num_sessions - 1) if not all fixed number of sessions are used.
 *          UNDEFINED if no sessions are available
 */
listen_session_id_t ListenService::getAvailableSessionId()
{
    ALOGV("getAvailableSessionId entered, max_sessions=%d", mMaxSysCaps.num_sessions);
    listen_session_id_t freeId = UNDEFINED;
    listen_session_data_t * pFreeSession = NULL;
    // Note: mutex lock is set in calling function
    for (int i=0; i < mMaxSysCaps.num_sessions; i++) {
       if (! mSessions[i].bActive ) {
          freeId = i;
          break;
       }
    }

    if (UNDEFINED == freeId) {
       ALOGE("getAvailableSessionId: no available sessionId");
       return UNDEFINED;
    }
    // a session is available
    // initialize session data & return valid session id

    pFreeSession = &mSessions[freeId];
    pFreeSession->bActive = true;
    pFreeSession->bSoundModelRegistered = false;
    pFreeSession->num_keywords = 0;
    pFreeSession->num_user_kw_pairings = 0;
    mCurUsage.num_sessions++;
    ALOGV("getAvailableSessionId return %d sessionId, increments numSession to %d",
       freeId, mCurUsage.num_sessions);
    return freeId;
}

/*
 * Marks Session of given Id as "free"
 *
 * Return - status
 */
listen_status_enum_t ListenService::freeSessionId(listen_session_id_t sessionId)
{
    ALOGV("freeSessionId(%d) entered, current numSession = %d",
             sessionId, mCurUsage.num_sessions);
    // Note: mutex lock is set in calling function
    if ( sessionId >= mMaxSysCaps.num_sessions ) {
       ALOGE("freeSessionId: session id %d not valid", sessionId);
       return LISTEN_EFAILURE;  // internal error
    }
    if ( (0 == mCurUsage.num_sessions) ||
         (mSessions[sessionId].bActive == false) ) {
       ALOGV("freeSessionId: session is not active");
       return LISTEN_SUCCESS;
    } else {
       ALOGV("freeSessionId: decrements numSession to %d", mCurUsage.num_sessions);
       mCurUsage.num_sessions--;
    }
    // clear all parameter for this session
    memset(&mSessions[sessionId], 0, sizeof(listen_session_data_t) );
    ALOGV("freeSessionId returns");
    return LISTEN_SUCCESS;
}

/*
 * Enable Listen MAD
 *
 * Return - HAL function status
 */
status_t ListenService::enableMAD()
{
   ALOGV("enableMAD entered");
   status_t aHALstatus = OK;
   if (NULL == mHwDevice) {
      // HW Device did not get initialized during getInstance
      ALOGE("enableMAD - ERROR mHwDevice is NULL");
      return NO_INIT;
   }
   // construct a key=value string to enable MAD 'on'
   const char *madEnabled = "mad=mad_on";
   // call AudioHAL::SetParameter() for MAD_ENABLEd true
   //     int (*set_parameters)(struct audio_hw_device *dev, const char *kv_pairs);
   aHALstatus = mHwDevice->listen_set_parameters(mHwDevice, madEnabled);
   if (OK != aHALstatus) {
      ALOGE("enableMAD: ERROR set_parameters(madEnable) failed w/%d", aHALstatus);
   }
   ALOGV("enableMAD returns status %d", aHALstatus);
   return aHALstatus;
}

/*
 * Disable Listen MAD
 *
 * Return - HAL function status
 */
status_t ListenService::disableMAD()
{
   ALOGV("disableMAD entered");
   status_t aHALstatus = OK;
   if (NULL == mHwDevice) {
      // HW Device did not get initialized during getInstance
      ALOGE("disableMAD: ERROR mHwDevice is NULL");
      return NO_INIT;
   }
   // construct a key=value string to enable MAD 'on'
   const char *madEnabled = "mad=mad_off";
   // call AudioHAL::SetParameter() for MAD_ENABLEd true
   //     int (*set_parameters)(struct audio_hw_device *dev, const char *kv_pairs);
   aHALstatus = mHwDevice->listen_set_parameters(mHwDevice, madEnabled);
   if (OK != aHALstatus) {
      ALOGE("enableMAD: ERROR set_parameters(madEnable) failed w/%d", aHALstatus);
   }

   // send an Listen "running" event to App after both MAD enabled and SM register successfully
   notifyOfMADChange(false);

   ALOGV("disableMAD returns %d", aHALstatus);
   return aHALstatus;
}

/*
 * Send notification event of a feature change to a specific receiver client
 *
 * Param [in]  client - reference to IListenReceiver that is to receiver event
 * Param [in]  eParamType - one of Listen listen_param_enum_t types
 * Param [in]  value - 1 = enable, 0 = disable
 *
 * Return - status
 */
inline listen_status_enum_t ListenService::sendFeatureNotification(
                         sp<IListenReceiver>  client,
                         listen_param_enum_t  eParamType,
                         int32_t              value)
{
   ALOGV("sendFeatureNotification entered");
   Parcel                emptyParcel;  // empty parcel sufficient

   if (client == NULL) {
      ALOGE("sendFeatureNotification: ERROR Could not get IListenReceiver ");
      return LISTEN_EFAILURE;
   }
   if ( LISTEN_PARAM_LISTEN_FEATURE_ENABLE == eParamType ) {
      if (0 == value) {
         client->notify(LISTEN_FEATURE_DISABLED, &emptyParcel);
      } else if (1 == value) {
         client->notify(LISTEN_FEATURE_ENABLED, &emptyParcel);
      } else {
         ALOGE("sendFeatureNotification: ERROR unknown param value %d", value);
         return LISTEN_EBAD_PARAM;
      }
   } else if ( LISTEN_PARAM_VOICE_WAKEUP_FEATURE_ENABLE == eParamType ) {
      if (0 == value) {
         client->notify(VOICE_WAKEUP_FEATURE_DISABLED, &emptyParcel);
      } else if (1 == value) {
         client->notify(VOICE_WAKEUP_FEATURE_ENABLED, &emptyParcel);
      } else {
         ALOGE("sendFeatureNotification: ERROR unknown param value %d", value);
         return LISTEN_EBAD_PARAM;
      }
   } else {
      ALOGE("sendFeatureNotification: ERROR unknown param type %d", eParamType);
      return LISTEN_EBAD_PARAM;
   }
   return LISTEN_SUCCESS;
}
/*
 * Notify all ListenReceiver clients of Application Feature Change
 *
 * Called when either ListenFeature or VoiceWakeupFeature is set by application
 *     to enable or disable.
 *
 * Param [in]  eParamType - one of Listen listen_param_enum_t types
 * Param [in]  value - 1 = enable, 0 = disable
 *
 * Return - status
 */
listen_status_enum_t ListenService::notifyOfFeatureChange(
                         listen_param_enum_t  eParamType,
                         int32_t              value)
{
   ALOGV("notifyOfFeatureChange entered");
   listen_status_enum_t  status = LISTEN_SUCCESS;
   listen_status_enum_t  featNotifyStatus = LISTEN_SUCCESS;
   sp<IListenReceiver> client;

   // locked in parent

   // Sending event to MasterControl seems redundant since it trigger this event
   //    but we're doing it any way
   if ( mMasterCntrl.bGranted ) {
       client = mMasterCntrl.receiver.client;
       status = sendFeatureNotification(client, eParamType, value);
       if (status != LISTEN_SUCCESS) {
          ALOGE("notifyOfFeatureChange: ERROR %d", status);
          return status;
       }
   }

   // Send event to all active Sessions
   for (int i=0; i<mMaxSysCaps.num_sessions; i++) {
       if (mSessions[i].bActive) {
          client = mSessions[i].receiver.client;
          featNotifyStatus = sendFeatureNotification(client, eParamType, value);
          if (featNotifyStatus != LISTEN_SUCCESS) {
             status = featNotifyStatus;
          }
       }
   }
   ALOGV("notifyOfFeatureChange returns status %d", status);
   return status;
}

/*
 * Notify all ListenReceiver clients when MAD is enabled or disabled by HAL
 *
 * Param [in]  eParamType - one of Listen listen_param_enum_t types
 * Param [in]  value - 1 = enable, 0 = disable
 *
 * Param [in]  bEnable - boolean enable flag
 *
 * Return - status
 */
listen_status_enum_t ListenService::notifyOfMADChange(bool bEnable)
{
   ALOGV("notifyOfMADChange entered");
   listen_status_enum_t  status = LISTEN_SUCCESS;
   Parcel emptyParcel;  // empty parcel sufficient
   sp<IListenReceiver> client;

   // locked in parent

   // Sending event to MasterControl seems redundant since it trigger this event
   //    but we're doing it any way
   if ( mMasterCntrl.bGranted ) {
       client = mMasterCntrl.receiver.client;
       if (client == NULL) {
          ALOGE("notifyOfMADChange: ERROR Could not get IListenReceiver ");
          return LISTEN_EFAILURE;
       }
       if (0 == mMasterCntrl.receiver.pid) {
          ALOGE("notifyOfMADChange: Could not get IListenReceiver so skipping notification...");
          ALOGE("... should only be seen when LSNTest app used");
          return LISTEN_SUCCESS;
       }
       if (bEnable) {
          client->notify(LISTEN_ENGINE_STARTED, &emptyParcel);
       } else {
          client->notify(LISTEN_ENGINE_STOPPED, &emptyParcel);
       }
   }

   // Send event to all active Sessions
   for (int i=0; i<mMaxSysCaps.num_sessions; i++) {
       if (mSessions[i].bActive) {
          client = mSessions[i].receiver.client;
          if (client == NULL) {
             ALOGE("notifyOfMADChange: ERROR Could not get IListenReceiver ");
             status = LISTEN_EFAILURE;
             continue;
          }
          if (bEnable) {
             client->notify(LISTEN_ENGINE_STARTED, &emptyParcel);
          } else {
             client->notify(LISTEN_ENGINE_STOPPED, &emptyParcel);
          }
       }
   }
   ALOGV("notifyOfMADChange returns status %d", status);
   return status;
}

/*
 * Deregister SoundModels for All active Sessions
 *
 * Tell HAL to deregister each sound model previously registered for each active session
 * Sends an event to each active session that it's SoundModel has been deregistered.
 *
 * Return - status
 */
listen_status_enum_t ListenService::deregisterAll()
{
   ALOGV("deregisterAll entered");
   listen_status_enum_t status = LISTEN_SUCCESS;
   Parcel emptyParcel;

   // locked in parent

   for (int i=0; i<mMaxSysCaps.num_sessions; i++) {
       // currently only once session type - LISTEN_VOICE_WAKEUP_SESSION - is supported
       if ( (LISTEN_VOICE_WAKEUP_SESSION == mSessions[i].type) &&
            (mSessions[i].bActive) &&
            (mSessions[i].bSoundModelRegistered) )
       {
          // send deregister command to HAL
          if (NULL != mSessions[i].pAudioHALSession ) {
             status_t aHALstatus = OK;
             ALOGV("deregisterAll: calls audio HAL deregister_sound_model()");
             aHALstatus = mSessions[i].pAudioHALSession->deregister_sound_model(mSessions[i].pAudioHALSession);
             if (OK != aHALstatus) {
                ALOGE("deregisterAll: ERROR audio HAL deregister_sound_model() failed");
             }
          }
          // send deregister to this receiver
          sp<IListenReceiver> client = mSessions[i].receiver.client;
          if (client == NULL) {
             ALOGE("deregisterAll: ERROR Could not get IListenReceiver ");
             status = LISTEN_EFAILURE;
             continue;
          }
          ALOGV("deregisterAll: notify client()");
          client->notify(SOUNDMODEL_DEREGISTERED, &emptyParcel);
          mSessions[i].bSoundModelRegistered = false;
       }
   }
   ALOGV("deregisterAll returns %d", status);
   return status;
}

/*
 * cleanupDeadClient
 *
 * called by IListenClientDeathNotifier if a ListenReceiver client dies
 *
 * Clean up MasterControl or VWUSession entrie in ListenService tables
 *     associated with this client
 *
 * Param - strong pointer to ListenReceiver object
 * Return - status OK or otherwise
 */
status_t  ListenService::cleanupDeadClient(const sp<IListenReceiver>& client)
{
    int i;
    status_t status = BAD_VALUE;
    ALOGV("cleanupDeadClient: entered because Listen Reciever died");
    if (client == 0) {
            ALOGE("cleanupDeadClient: ERROR client sp<> parameter NULL!");
       return BAD_VALUE;
    }
    mLock.lock();
    if ( mMasterCntrl.bGranted ) {
       if (mMasterCntrl.receiver.client.get() == client.get()) {
          ALOGV("cleanupDeadClient: client match with MasterControl");
          memset(&mMasterCntrl, 0, sizeof(listen_master_cntrl_data_t) );
          clearListenClientContext(client);
          status = OK;
       } else {
          ALOGV("cleanupDeadClient: given client.ptr %p != MasterControl client.ptr %p",
                        client.get(), mMasterCntrl.receiver.client.get() );
       }
    }
    // client was not MasterControl, so compare give client with each session entry in Service
    for (i=0; i<mMaxSysCaps.num_sessions; i++) {
       if ( mSessions[i].bActive ) {
          if ( mSessions[i].receiver.client.get() == client.get() ) {
             ALOGV("cleanupDeadClient: client match with session %d", i);
             mLock.unlock();
             destroySession(i);  // will also freeSessionId & call clearListenClientContext()
             mLock.lock();
             status = OK;
          } else {
             ALOGV("cleanupDeadClient: given client.ptr %p != Session client.ptr %p",
                        client.get(), mSessions[i].receiver.client.get() );
          }
       }
    }
    mLock.unlock();
    if (status != OK){
    ALOGE("cleanupDeadClient: ERROR client not being managed by service!");
    }
    return status;
}

// establish binder interface to ListenReceiver
status_t ListenService::setListenClientContext(
                         const sp<IListenReceiver>& receiver)
{
    ALOGV("setListenClientContext");
    sp<IBinder> pBinder = NULL;
    if (receiver == 0) {
       ALOGE("setListenClientContext: ERROR no listen receiver associated with binder!?");
       return BAD_VALUE;
    } else {
       ALOGV("setListenClientContext: receiver = %p", receiver.get());
    }
    if (sDeathNotifier == 0) {
       ALOGE("setListenClientContext: ERROR DeathNotifier was not created when ListenService started");
       return UNKNOWN_ERROR;
    }

    Mutex::Autolock _l(sReceiverLock);

    // move creation of DeathNotifier to IListenClientDeathNotifier
    // always set receiver

    pBinder = receiver->asBinder();
    if (pBinder == NULL) {
       ALOGE("setListenClientContext: ERROR binder ptr should not be NULL");
       return UNKNOWN_ERROR;
    }
    pBinder->linkToDeath(sDeathNotifier);
    sListenReceivers.add(receiver);
    return OK;
}

status_t ListenService::clearListenClientContext(
                         const sp<IListenReceiver>& receiver)
{
   sp<IBinder> pBinder = NULL;
    if (receiver == 0) {
       ALOGE("clearListenClientContext: ERROR no listen receiver associated with binder!?");
       return BAD_VALUE;
    } else {
       ALOGV("clearListenClientContext: receiver = %p", receiver.get());
    }
    Mutex::Autolock _l(sReceiverLock);

    pBinder = receiver->asBinder();
    if (pBinder == NULL) {
       ALOGE("clearListenClientContext: ERROR binder ptr should not be NULL");
       return UNKNOWN_ERROR;
    }
    pBinder->unlinkToDeath(sDeathNotifier);
    sListenReceivers.remove(receiver);
    return OK;
}

/*
 * ListenService's Event callback function
 * Executed by AudioHAL to send an event to ListenService
 * Assumed that AudioHAL always execute this callback one event at a time
 *
 * Param [in] eventType -
 * Param [in] payload - ptr to event data structure containing ptr to and size of event data
 * Param [in] sessionHndl - ptr to AudioHal session that should receives this event
 *                  when set non-NULL, only session client with matching handle is sent event
 *                  if NULL then event should be sent to all receivers
 */
void eventCallback(listen_event_enum_t eventType,
                          listen_event_data_t *payload,
                          void *sessionHndl)
{
   ALOGV("eventCallback(type = %d) entered", (int)eventType);
   listen_receiver_data_t *pReceiverHandle = NULL;
   sp<IListenReceiver> client;
   ListenService * pListenService = ListenService::getInstance();
   Parcel emptyParcel; // left empty, no elements written to it
   bool                     bMCReceiverActive = false;  // only send to MasterControl receiver if active
   status_t parcelStatus = NO_ERROR;
   bool   bLockAcquired = false;
   bool   bTryLocked = false;

   if (!pListenService) {
      ALOGE("eventCallback: get ListenService instance failed!");
     return;
   }

   for (int iTry=0; iTry < TIMES_TO_TRY_LOCK; iTry++) {
      ALOGV("eventCallback: try lock called");
      bTryLocked = pListenService->mLock.tryLock();
      if (bTryLocked) { // "true returned if mutex is already locked"
           ALOGE("eventCallback: lock mLock already used, sleep %d ms", TIME_BETWEEN_TRY_LOCK_ATTEMPTS);
           usleep(TIME_BETWEEN_TRY_LOCK_ATTEMPTS);
      } else {
          ALOGV("eventCallback: lock mLock acquired");
          bLockAcquired = true;
          break;
      }
   }
   if (false == bLockAcquired) {
       // could not acquire lock, drop this event rather than potentionally hang
       ALOGE("eventCallback: could not acquire lock, event type %d dropped", eventType);
       goto cleanupServiceInstance;
   }

   if (NULL == sessionHndl) {
      // when session handle is null then this event is potentially for
      //     MasterControl (if active) and all active Sessions
      if ( pListenService->mMasterCntrl.bGranted )  {
         if ( 0 == pListenService->mMasterCntrl.receiver.pid ) {
            ALOGE("eventCallback: ERROR Could not get IListenReceiver so skipping notification...");
            ALOGE("... should only be seen when LSNTest app used");
            goto cleanupServiceInstance;
         }
         ALOGV("eventCallback: will process MasterControl event");
         pReceiverHandle = &(pListenService->mMasterCntrl.receiver);
         bMCReceiverActive = true;
      } else {
         ALOGV("eventCallback: skipping notification of MasterControl client since it is not active ");
      }
   } else {
      // find the receiver object in array using the ptr to listen_session_t passed into this callback
      listen_session_t * session = (listen_session_t *)sessionHndl;
      int i;
      for (i=0; i < pListenService->mMaxSysCaps.num_sessions; i++) {
         if (pListenService->mSessions[i].pAudioHALSession == session) {
            if (pListenService->mSessions[i].bActive) {
               ALOGV("eventCallback: will process event for %d-th session", i);
            pReceiverHandle = &(pListenService->mSessions[i].receiver);
            } else {
               ALOGE("eventCallback: session associate with id %p passed to eventCallback NOT active !", session);
               goto cleanupServiceInstance;
            }
            break;
         }
      }
      if (i >= pListenService->mMaxSysCaps.num_sessions) {
            ALOGE("eventCallback: session id %p passed to eventCallback NOT in mSession table !", session);
            goto cleanupServiceInstance;
      }
   }

   switch(eventType) {
      case LISTEN_ERROR:
      {
         ALOGE("eventCallback: ERROR Event sent by HAL");
         // do not send event; drop thru to clean up
      } break;

      case LISTEN_EVENT_STARTED:
      case LISTEN_EVENT_STOPPED:
      {
         ALOGI("eventCallback: LISTEN START/STOP EVENT");
         //
         // Send notification all active receivers that Listen engine is 'active' or 'stopped'
         //
         // first send event to master control receiver if active
         if (bMCReceiverActive) {
         client = pListenService->mMasterCntrl.receiver.client;
            if (client != NULL) {
               client->notify((int)eventType, &emptyParcel);
               ALOGV("eventCallback: masterControl->notify returns");
            }
         }
         // then send notification all active session receivers
         for (int i=0; i < pListenService->mMaxSysCaps.num_sessions; i++) {
            if (pListenService->mSessions[i].bActive) {
               client = pListenService->mSessions[i].receiver.client;
               if (client != NULL) {
                  client->notify((int)eventType, &emptyParcel);
                  ALOGV("eventCallback: session[%d]->notify returns", i);
               }
            }
         }
      } break;

      case LISTEN_EVENT_DETECT_SUCCESS:
      case LISTEN_EVENT_DETECT_FAILED:
      {
         ALOGI("eventCallback: DETECTION EVENT");
         // For a detection event, send event to specific session (with it associated registered soundmodel)
         //     that detection occurred for
         //     LISTEN_EVENT_DETECT_SUCCESS sent if minimum keyword (and user) matching confidence levels were met
         //     LISTEN_EVENT_DETECT_FAILED sent when special Failure Detection mode is
         Parcel detectionParcel;
         if (NULL == pReceiverHandle) {
            ALOGE("eventCallback: ERROR handle to requested receiver could not be acquired !");
            goto cleanupServiceInstance;
         }
         client = pReceiverHandle->client;
         if (client == NULL) {
            ALOGE("eventCallback: ERROR client ptr is NULL ");
            goto cleanupServiceInstance;
         }
          // payload contains ptr to black box data that will be passed up to client
         if ( NULL == payload )  {
            ALOGE("eventCallback - Detect Event payload NULL");
            detectionParcel.writeInt32((int32_t)0);
         } else if ( 0 == payload->event_detect.size) {
            ALOGE("eventCallback - Detect Event payload size zero");
            detectionParcel.writeInt32((int32_t)0);
         } else {
            size_t payloadSize = payload->event_detect.size;
            detectionParcel.writeInt32((int32_t)payload->event_detect.size);
            ALOGD("eventCallback: Detect Event (Type %d), payload size %d", eventType, payload->event_detect.size);
            parcelStatus = detectionParcel.write((void *)payload->event_detect.p_data, payloadSize);
            if  (parcelStatus != NO_ERROR) {
                ALOGE("eventCallback - ERROR %d while writing to detectionParcel", parcelStatus);
            }
         }
         // notify only the session that has is associated with the receiverHandle
         client->notify((int)eventType, &detectionParcel);
         ALOGV("eventCallback: client->notify returns");
      }  break;

      default:
         /* None of the other Listen events:
          *    LISTEN_EVENT_LISTEN_FEATURE_DISABLE and _DISABLE
          *    LISTEN_EVENT_VOICE_WAKEUP_FEATURE_ENABLE and _DISABLE
          *    LISTEN_EVENT_DEREGISTERED
          * should be sent to us by AudioHAL
          */
         ALOGE("eventCallback: ERROR event type %d should not be called from HAL", eventType);
         break;
   }
cleanupServiceInstance:
   if (pListenService) {
      ALOGV("eventCallback: releaseInstance");
      pListenService->releaseInstance();
   }
   if (bLockAcquired) {
      ALOGV("eventCallback: unlock mLock");
      pListenService->mLock.unlock();
   }
   ALOGV("eventCallback returns");
   return;
}

} // namespace android
