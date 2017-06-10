/*
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution, Apache license notifications and
 * license are retained for attribution purposes only.
 */
/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * This file contains Listen Native Receiver class and all all classes used
 * to communicate to and from Listen Native Service from Listen Receiver:
 *        ListenReceiver
 *        ListenMasterControl - inherit ListenReceive base class
 *        ListenVWUSession - inherit ListenReceive base class
 *        ListenReceiverListener - communicates with JNI from ListenReceiver methods
 */
#ifndef ANDROID_LISTENRECEIVER_H
#define ANDROID_LISTENRECEIVER_H

#include <arpa/inet.h>

#include <binder/IMemory.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>

#include <ListenNativeTypes.h>
#include <IListenReceiver.h>
#include <IListenDeathNotifier.h>

namespace android {

class ListenService; // forward declarations
class ListenReceiverListener; // declared at end of file

// ----------------------------------------------------------------------------
//
// ListenReceiver
//
// ----------------------------------------------------------------------------
class ListenReceiver : public BnListenReceiver,
                       public virtual IListenDeathNotifier
{
protected:
    ListenReceiver();
public:
    ~ListenReceiver();

// IListenDeathNotifier
    void            died();

// IListenReceiver
    void            notify(int msg, const Parcel *obj = NULL);

    void                         setListener(ListenReceiverListener * listener);
    void                         clearListener();
    ListenReceiverListener *     getListener();

//
// ListenReceiver
//
   /*
    * Initializes ListenReceiver object with the ListenEngine (managed by ListenService)
    * Each receiver is of a particular type on parent class this Receiver
    *      this can be either MasterControl or some type of Session
    * Creates a native receiver object managed by the Listen Native Service
    *      Creation of native receiver will fail
    *          MasterControl is already granted to another app, or
    *          maximum number of Sessions already active.
    *
    * Param [in]  version - two MSBytes Major number, two LSBytes Minor number
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - maximum available sessions already instanced
    */
   virtual listen_status_enum_t init(uint32_t appType) = 0;

   /* Release Receiver
    *
    * Releases this Listen Receiver (based on type) within the Listen native Service
    * Destroys the native receiver objects associated with this
    * Params - none
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - maximum available sessions already instanced
    */
   virtual listen_status_enum_t release() = 0;

    /*
     * Get Listen parameter
     *
     * The form of this function accepts a single parameter type and return value as int32
     * All receivers may query the any parameter
     *
     * Gets a parcel back from Framework getParameter(int key, Parcel* reply)
     *   and converts that to integer return value
     *
     * Param [in]  eParamType - one of Listen listen_param_enum_t
     * Return - current value of parameter
     */
     int32_t  getParam(listen_param_enum_t eParamType);


    /*
     * Member variables
     */
public:
    listen_receiver_enum_t      mReceiverType; // Some receivers are session, some masterCntl
protected:
    ListenReceiverListener *    mListener;  // ptr to JNI class that processes notify method

    thread_id_t                 mLockThreadId;
    Mutex                       mLock;        // used by this classes methods
    Mutex                       mNotifyLock;  // used when ListenReceiverListener notify method is call by this class
public:
    uint32_t                    mAppType;      // 4 bytes - %%% will likely be converted to Java UUID
    uint16_t                    mAlgoVersion;  // optional algorithm version:
                                               //    2 MSBytes: Major number, 2 LSBytes: Minor number)
};


// ----------------------------------------------------------------------------
//
// ListenMasterControl
//
//    child class of ListenReceiver base class for MasterControl
//
// ----------------------------------------------------------------------------
//
class ListenMasterControl : public ListenReceiver
{
public:
    ListenMasterControl();
    ~ListenMasterControl();

   /*
    * Creates a new Listen MasterControl receiver within the Listen native Service
    *
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - maximum available sessions already instanced
    */
   virtual listen_status_enum_t init(uint32_t appType = APP_TYPE_UNKNOWN);

   /* Release MasterControl receiver
    *
    * Releases this receiver within the Listen native Service
    *
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - maximum available sessions already instanced
    */
   virtual listen_status_enum_t release();

    /*
     * Set Listen parameter
     *
     * The function accepts a single parameter type and value
     * Currently the only parameters that can be set are global parameters
     * Only receivers with MasterControl can set global parameters
     *
     * Places value into 'parcel' and calls
     *     setParameter(int key, const Parcel& request);
     *
     * Param [in]  key - one of Listen listen_param_enum_t
     * Param [in]  value - generic signed int32 containing value
     * Return - errors
     *      LISTEN_ENO_GLOBAL_CONTROL - returned if session does not have permission to set global params
     */
    listen_status_enum_t setParam(listen_param_enum_t eParamType,
                                  int32_t             value);
    /*
     * Member variables
     */
    bool                        mbMasterControlGranted;
};

// ----------------------------------------------------------------------------
//
// ListenVWUSession
//
//    child class of ListenReceiver base class for VoiceWakeupSession
//
// ----------------------------------------------------------------------------
//
class ListenVWUSession : public ListenReceiver
{
public:
    ListenVWUSession();
    ~ListenVWUSession();

    /*
    * Creates a new Listen VoiceWakeup Session receiver within the Listen Native Service
     *
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - maximum available sessions already instanced
    */
   virtual listen_status_enum_t init(uint32_t = APP_TYPE_UNKNOWN);

   /* Release VoiceWakeup Session receiver
     *
    * Releases this receiver within the Listen native Service
     *
    * Return - errors
    *       LISTEN_ERESOURCE_NOT_AVAILABLE  - maximum available sessions already instanced
     */
   virtual listen_status_enum_t release();


    /*
     * registerSoundModel
     *
     * Specifies the sound model that SVA Listen engine uses to detect desired
     *      keyword for this session.
     * Only one model may be registed per session.
     * If a model was previous register for this session, it must be explicitly deregistered.
     * This form takes both SVA 1.0 and SVA 2.0 and above SoundModels.
     * If SVA 1.0 SM is being registered, the SVA 1.0 parameters were mapped to SVA 1.5 parameters.
     *
     * Client must be initialized before calling
     *
     * Param [in]  pSoundModelParams - pointer to struct with all sound model parameters
     * Return - error
     *     LISTEN_EBAD_PARAM
     *     LISTEN_ESESSION_NOT_INITIALIZED
     */
    listen_status_enum_t registerSoundModel(
                             listen_sound_model_params_t* pSoundModelParams);

    /*
     * deregisterSoundModel
     *
     * Clears SoundModel registeration for this Listen Session
     *
     * Param - none
     *
     * Return - error
     *      LISTEN_ESOUNDMODEL_NOT_REGISTERED
     */
    listen_status_enum_t deregisterSoundModel();

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
    listen_status_enum_t readBuffer(
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
    listen_status_enum_t stopBuffering();


    /*
     * Stop Detection
     *
     * Stops VoiceWakeup Detection for this session without deregistering SoundModel.
     *
     * Registeration must have been successful for this to have any affect.
     *
     * Param - none
     *
     * Return - error
     *     LISTEN_EBAD_PARAM
     *     LISTEN_ESOUND_MODEL_NOT_REGISTERED
     */
    listen_status_enum_t stopDetection();

     /*
     * Start Detection
     *
     * Restarts VoiceWakeup Detection for this session what was previously
     * stopped.
     *
     * Registeration must have been successful for this to have any affect.
     *
     * Param - none
     *
     * Return - error
     *     LISTEN_EBAD_PARAM
     *     LISTEN_ESOUND_MODEL_NOT_REGISTERED
     */
    listen_status_enum_t startDetection();

   /*
     * clean up allocated detection parameters
     *
     * frees memory allocated for listen_detection_params_t elements
     *
     * Param - none
     *
     * Return - none
     */
    void cleanupDetectionParams();

    /*
     * Member variables
     */
    listen_session_id_t          mSessionId;  // handle from session openned by HAL
    bool                         mbSoundModelRegistered;

    listen_detection_params_t    mDetectionParams;
};


// ----------------------------------------------------------------------------
//
// ListenReceiverListener
//
// JNI class inherits this to process event notification from ListenReceiver
// ----------------------------------------------------------------------------
class ListenReceiverListener: virtual public RefBase
{
public:
    virtual void notify(int msg, const Parcel *obj) = 0;
};


#endif // ANDROID_LISTENRECEIVER_H

}; // namespace android
