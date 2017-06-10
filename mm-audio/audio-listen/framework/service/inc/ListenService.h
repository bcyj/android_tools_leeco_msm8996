/*
**
** Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
** Qualcomm Technologies Confidential and Proprietary.
**
** Not a Contribution, Apache license notifications and
** license are retained for attribution purposes only.
**/
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

#ifndef ANDROID_LISTENSERVICE_H
#define ANDROID_LISTENSERVICE_H

#include <arpa/inet.h>

#include <utils/Log.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <binder/Parcel.h>

#include <hardware/audio.h>

#include <ListenNativeTypes.h>
#include <IListenService.h>
#include <IListenClientDeathNotifier.h>

namespace android {

typedef struct{
      listen_receiver_enum_t  type;
      // Only if is set non-NULL will event notification to client be attempted.
      pid_t                   pid;                   // process id of client
      sp<IListenReceiver>     client;
} listen_receiver_data_t;

typedef struct{
      listen_session_enum_t   type;
      bool                    bActive;               // set true when session added to Listen Native Service
      listen_receiver_data_t  receiver;              // ListenReceiver attributes
      listen_session_t *      pAudioHALSession;
      bool                    bSoundModelRegistered; // true iff SM is currently registered
      uint16_t                num_keywords;
      uint16_t                num_user_kw_pairings;
} listen_session_data_t;

typedef struct{
      bool                    bGranted;              // set true when the Listen Native Service currently active
      listen_receiver_data_t  receiver;              // ListenReceiver attributes
} listen_master_cntrl_data_t;

/*
 * Listen Native Service
 *
 * Manages multiple Listen Native Sessions
 * Calls ListenSoundModel library functions
 * Calls AudioHAL for Listen requests to be sent to QDSP
 * Directs Events from AudioHAL to appropriate session callback
 * This is implemented as a Singleton Class !
 * First session that requests to be initialized, instantiates ListenService
 * The first session that has permission and requests Global Control gets to set these parameters
 * All methods are synchronous
 */
class ListenService : public BnListenService,
                      public virtual IListenClientDeathNotifier
{

  protected:
    ListenService();
  public:
    virtual   ~ListenService();

  private:
    status_t  loadAudioInterface();
    status_t  unloadAudioInterface();

// IListenClientDeathNotifier
public:
    virtual void      died() {} //nothing special done in this virtual method
protected:
    virtual status_t  setListenClientContext(const sp<IListenReceiver>& receiver);
    virtual status_t  clearListenClientContext(const sp<IListenReceiver>& receiver);

//
// ListenService
//
public:
    /*
     * Called by Server Manager process to instatiate single ListenService
     * Add "listen.service" as a client of Server Manager
     */
    static  void          instantiate();
    /*
     * Get this singleton instance of ListenService
     * load HW Device interface to AudioHAL
     */
    static ListenService* getInstance();

    /*
     * Release instance of ListenService
     */
    void releaseInstance();

    // ----------------------------------------------------------------------------
    //  Master Control specific Methods
    // ----------------------------------------------------------------------------
    /*
     * Associate a ListenReceiver MasterControl object with the ListenService
     * Returns true if MasterControl has not be previous acquired.
     */
    virtual bool  initMasterControl(
                   pid_t pid,
                   const sp<IListenReceiver>& client );

   /* Request Master Control
    *
    * Acquire the ability to set master control parameters
    * The first client that has permission to requests Master Control
    *     gets to set Listen global parameters
    * These parameters are global not session based
    *
    * Param - none
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - global control already acquired by another client
    */
    listen_status_enum_t requestMasterControl();

   /* Release Master Control
    *
    * Acquire the ability to set master control parameters
    * The first client that has permission to requests Master Control
    *     gets to set Listen global parameters
    * These parameters are global not session based
    *
    * Param [in]  ptr to java receiver object
    *
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - global control already acquired by another client
    */
   virtual listen_status_enum_t releaseMasterControl();

    // ----------------------------------------------------------------------------
    //  Session specific Methods
    // ----------------------------------------------------------------------------
    /*
     * Associate a ListenReceiver Session object with the ListenService
     * Acquires an available ListenSession
     * Save infomation about IListenSevice receiver object requesting
     * Returns valid session id if free session available
     */
    virtual listen_session_id_t initSession(
                   listen_session_enum_t sessionType,
                   pid_t pid,
                   const sp<IListenReceiver>& client );

   /*
    * Creates a new Listen Native Session managed by the Listen Native Service
    *
    * If no sessions are currently active, instantiate ListenNativeService
    * Limit the number of sessions that can be active to a maximum number.
    *
    * Current only one type of session is supported - VoiceWakeupSessions
    * When more than one type of session needs to be supported, added session type parameter.
    *
    * Param [out] returns unique session handle return event with option data to client
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - maximum available sessions already instanced
    */
    listen_status_enum_t addSession(listen_session_id_t* sessionId);

   /*
    *
    * Remove session from Listen Native Service
    *
    * Param [in]  session id - handle of session
    */
    virtual listen_status_enum_t destroySession(listen_session_id_t sessionId);

    /*
     * Set Listen parameter
     *
     * The function accepts a single parameter type and its data value(s).
     * Currently the only parameters that can be set are global parameters
     * Only clients with MasterControl permission can set global parameters
     *
     * Param [in]  eParamType - one of Listen listen_param_enum_t
     * Param [in]  request - parcel data structure containing input data field(s)
     * Return - errors
     *      LISTEN_ENO_GLOBAL_CONTROL - returned if session does not have permission to set global params
     *      LISTEN_BAD_PARAM
     */
     virtual listen_status_enum_t setParameter(listen_param_enum_t   eParamType,
                                       int32_t               value);

    /*
     * Get Listen parameter
     *
     * The form of this function accepts a single parameter type and outputs data field(s)
     * All clients may query the any parameter
     *
     * Param [in]  eParamType - one of Listen listen_param_enum_t
     * Param [out] reply - parcel data structure containing output data field(s)
     * Return - status
     *      LISTEN_BAD_PARAM
     */
     virtual int32_t getParameter(listen_param_enum_t eParamType);

    /*
     * registerSoundModel
     *
     * Specifies the sound model that Listen engine uses to detect desired keyword/sound
     * for this session.
     * Only one model may be registed per session.
     * If a model was previous register for this session, it must be explicitly deregistered.
     *
     * Session must be initialized before calling
     *
     * Param [in]  session id - handle of Listen Native Session
     * Param [in]  pSoundModelParams - ptr to listen_sound_model_params_t
     *
     * Return - error
     *     LISTEN_EBAD_PARAM
     *     LISTEN_ESESSION_NOT_INITIALIZED
     */
     virtual listen_status_enum_t  registerSoundModel(
                                    listen_session_id_t          sessionId,
                                    listen_sound_model_params_t* pSoundModelParams);

    /*
     * deregisterSoundModel
     *
     * Clears SoundModel registeration for a given session
     *
     * Param [in]  session id - handle of Listen Native Session
     *
     * Return - error
     *      LISTEN_ESOUNDMODEL_NOT_REGISTERED
     */
     virtual listen_status_enum_t deregisterSoundModel(listen_session_id_t sessionId);

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
    virtual listen_status_enum_t readBuffer(
            listen_session_id_t          sessionId,
            int32_t                      numSamples,
            int16_t*                     pSampleBuff,
            int32_t*                     pSamplesRead);

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
    virtual listen_status_enum_t stopBuffering(
            listen_session_id_t sessionId);

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
    virtual listen_status_enum_t stopDetection(
            listen_session_id_t sessionId);
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
    virtual listen_status_enum_t startDetection(
            listen_session_id_t sessionId);

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
     status_t  cleanupDeadClient(const sp<IListenReceiver>& client);

     /*
      * determine system resources
      *
      * based on target determine the maximum number of each type of listen resource
      * (e.g. session, keywords, user-keywords pairings)
      * This function is Hardcoded to fixed values.
      */
     listen_status_enum_t determine_system_resources(
                 listen_resources_t *p_system_caps);

private:
    ListenService(const ListenService&);                 // Prevent copy-construction
    ListenService& operator=(const ListenService&);      // Prevent assignment

    listen_session_id_t  getAvailableSessionId();
    listen_status_enum_t freeSessionId(listen_session_id_t sessionId);

    status_t             enableMAD();
    status_t             disableMAD();
    listen_status_enum_t deregisterAll();

    inline
    listen_status_enum_t sendFeatureNotification(sp<IListenReceiver>  client,
                                                 listen_param_enum_t  eParamType,
                                                 int32_t              value);
    listen_status_enum_t notifyOfFeatureChange(listen_param_enum_t  eParamType,
                                               int32_t              value);
    listen_status_enum_t notifyOfMADChange(bool bEnable);

   /*
    * Variables
    */
  private:
    static      uint16_t       mNumInstances;
    static      Mutex          mInstanceLock;
    static      ListenService* mSelf;

    audio_hw_device_t *        mHwDevice;
    // Overall Listen Feature enable flag
    //   All sub-features are implicitly disabled when ListenFeature disabled
    bool                       mbListenFeatureEnabled;
    // Sub-Features: VoiceWakeup, TargetSound,...
    //   Each sub-feature can be enabled/disabled separately
    bool                       mbVoiceWakeupFeatureEnabled;
    uint16_t                   mbNumSubFeaturesEnabled;

    bool                       mMadObserverSet;

  public:
    // need to be accessed by eventCallback function
    bool                       mMADEnabled;
    // Mutex lock is public so that static eventCallback() can access it
    mutable     Mutex          mLock;

    // Data structures for containing information about:
    //    single MasterControl receiver
    //    MAX_LISTEN_SESSIONS concurrent Session receivers
    listen_master_cntrl_data_t mMasterCntrl;
    // Session specific members
    // %%% replace static array with list
    listen_session_data_t      mSessions[MAX_LISTEN_SESSIONS];

    // Overall system resources are determined based on target
    listen_resources_t         mMaxSysCaps;
    // Keep accumulated counts of each resource currently being used across all sessions
    listen_resources_t         mCurUsage;
};

/* ListenService's Event callback function of type listen_callback_t
 * Executed by AudioHAL to send an event to ListenService
 */
static void eventCallback(listen_event_enum_t eventType,
                          listen_event_data_t *payload,
                          void *clientData);

}; // namespace android

#endif // ANDROID_LISTENSERVICE_H
