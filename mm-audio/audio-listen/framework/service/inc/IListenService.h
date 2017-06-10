/*
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution, Apache license notifications and
 * license are retained for attribution purposes only.
 */
/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_ILISTENSERVICE_H
#define ANDROID_ILISTENSERVICE_H

#include <utils/Errors.h>  // for status_t
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <IListenReceiver.h>
#include <ListenNativeTypes.h>

namespace android {

class IListenService: public IInterface
{
public:
    DECLARE_META_INTERFACE(ListenService);
    /*
     * Methods executed thru BpListenService onTransact
     * These virtual functions must be implemented in ListenService
     */
    virtual bool initMasterControl(
                   pid_t pid,
                   const sp<IListenReceiver>& client) = 0;

    virtual listen_status_enum_t releaseMasterControl() = 0;

    virtual listen_session_id_t initSession(
                   listen_session_enum_t sessionType,
                   pid_t pid,
                   const sp<IListenReceiver>& client) = 0;

    virtual listen_status_enum_t destroySession(
            listen_session_id_t mSessionId) = 0;

    virtual listen_status_enum_t setParameter(
            listen_param_enum_t eParamType,
            int32_t             value) = 0;

    virtual int32_t getParameter(
            listen_param_enum_t eParamType) = 0;

    virtual listen_status_enum_t registerSoundModel(
            listen_session_id_t          sessionId,
            listen_sound_model_params_t* pSoundModelParams) = 0;

    virtual listen_status_enum_t deregisterSoundModel(
            listen_session_id_t mSessionId ) = 0;

    virtual listen_status_enum_t readBuffer(
            listen_session_id_t          sessionId,
            int32_t                      numSamples,
            int16_t*                     pSampleBuff,
            int32_t*                     pSamplesRead   ) = 0;

    virtual listen_status_enum_t stopBuffering(
            listen_session_id_t mSessionId ) = 0;

    virtual listen_status_enum_t stopDetection(
            listen_session_id_t mSessionId ) = 0;

    virtual listen_status_enum_t startDetection(
            listen_session_id_t mSessionId ) = 0;

};

// ----------------------------------------------------------------------------

class BnListenService: public BnInterface<IListenService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif // ANDROID_ILISTENSERVICE_H
