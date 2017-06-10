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

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>  // for status_t
#include <binder/Parcel.h>
#include <binder/IMemory.h>

#include <IListenService.h>
#include <ListenService.h>

#define LOG_TAG "IListenService"
#define LOG_NDEBUG 0

namespace android {

enum {
    SET_MASTER_CONTROL_RECEIVER = IBinder::FIRST_CALL_TRANSACTION,
    CLEAR_MASTER_CONTROL_RECEIVER,
    CREATE_SESSION_RECEIVER,
    DESTROY_SESSION_RECEIVER,
    SET_PARAMETER,
    GET_PARAMETER,
    REGISTER_SOUNDMODEL,
    DEREGISTER_SOUNDMODEL,
    READ_BUFFER,
    STOP_BUFFERING,
    STOP_DETECTION,
    START_DETECTION,
};

class BpListenService: public BpInterface<IListenService>
{
public:
    BpListenService(const sp<IBinder>& impl)
        : BpInterface<IListenService>(impl)
    {
       ALOGD("BpListenService::constructor- impl=%p, this=%p", impl.get(), this);
    }
    ~BpListenService()
    {
       ALOGD("BpListenService::destructor, this=%p", this);
    }

    virtual bool initMasterControl(
            pid_t pid, const sp<IListenReceiver>& client) {
        Parcel data, reply;
        bool retVal;
        ALOGD("BpListenService::initMasterControl entered");
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(pid);
        data.writeStrongBinder(client->asBinder());
        remote()->transact(SET_MASTER_CONTROL_RECEIVER, data, &reply);
        retVal = (bool)(reply.readInt32());
        ALOGD("BpListenService::initMasterControl returns %d", retVal);
        return retVal;
    }

    virtual listen_status_enum_t releaseMasterControl() {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        remote()->transact(CLEAR_MASTER_CONTROL_RECEIVER, data, &reply);
        return (listen_status_enum_t)(reply.readInt32());
    }

    virtual listen_session_id_t initSession(
            listen_session_enum_t sessionType,
            pid_t pid, const sp<IListenReceiver>& client) {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(sessionType);
        data.writeInt32(pid);
        data.writeStrongBinder(client->asBinder());

        remote()->transact(CREATE_SESSION_RECEIVER, data, &reply);
        return (listen_session_id_t)(reply.readInt32());
    }

    virtual listen_status_enum_t destroySession(
            listen_session_id_t mSessionId ) {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(mSessionId);
        remote()->transact(DESTROY_SESSION_RECEIVER, data, &reply);
        return (listen_status_enum_t)(reply.readInt32());
    }

    virtual listen_status_enum_t setParameter(
            listen_param_enum_t eParamType,
            int32_t             value) {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(eParamType);
        data.writeInt32(value);
        remote()->transact(SET_PARAMETER, data, &reply);
        return (listen_status_enum_t)(reply.readInt32());
    }

    virtual int32_t getParameter(
            listen_param_enum_t eParamType) {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(eParamType);
        remote()->transact(GET_PARAMETER, data, &reply);
        return (int32_t)(reply.readInt32());
    }

    virtual listen_status_enum_t registerSoundModel(
            listen_session_id_t mSessionId,
            listen_sound_model_params_t* pSoundModelParams) {
        Parcel data, reply;
        listen_status_enum_t listenStatus;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(mSessionId);
        data.writeInt32((int32_t)pSoundModelParams->sound_model_type);
        ALOGI("registerSoundModel: model type = %d", pSoundModelParams->sound_model_type);
        int arraySize = pSoundModelParams->sound_model_data->size;
        data.writeInt32(arraySize);
        data.write(pSoundModelParams->sound_model_data->p_data, arraySize );
        data.writeInt32((int32_t)pSoundModelParams->detection_mode);
        data.writeInt32((int32_t)pSoundModelParams->detect_failure);
        data.writeInt32((int32_t)pSoundModelParams->num_keywords);
        data.writeInt32((int32_t)pSoundModelParams->num_user_pairings);
        data.write(pSoundModelParams->min_conf_levels,
           (pSoundModelParams->num_keywords +  pSoundModelParams->num_user_pairings) );
        data.writeInt32((int32_t)pSoundModelParams->enable_buffering);
        ALOGI("transact(REGISTER_SOUNDMODEL)");
        remote()->transact(REGISTER_SOUNDMODEL, data, &reply);
        listenStatus = (listen_status_enum_t)(reply.readInt32());
        ALOGI("registerSoundModel returns %d", listenStatus);
        return (listenStatus);
    }
    virtual listen_status_enum_t deregisterSoundModel(
            listen_session_id_t mSessionId ) {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(mSessionId);
        remote()->transact(DEREGISTER_SOUNDMODEL, data, &reply);
        return (listen_status_enum_t)(reply.readInt32());
    }

    virtual listen_status_enum_t readBuffer(
            listen_session_id_t  mSessionId,
            int32_t              numSamples,
            int16_t*             pSampleBuff,
            int32_t*             pSamplesRead)
    {
        Parcel data, reply;
        if (NULL == pSampleBuff || NULL == pSamplesRead) {
            ALOGE("readBuffer: a ptr param (%p, %p) is null", pSampleBuff, pSamplesRead);
            return LISTEN_EBAD_PARAM;
        }
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(mSessionId);
        data.writeInt32(numSamples);
        remote()->transact(READ_BUFFER, data, &reply);

        listen_status_enum_t status = (listen_status_enum_t)reply.readInt32();
        if (LISTEN_ENO_MEMORY == status) {
           ALOGE("readBuffer: no memory error turned transact; don't copy bytes read %d", *pSamplesRead);
           *pSamplesRead = 0;  //force sample read to zero
           // donot bother copying bytes from parcel
        } else {
           ALOGD("readBuffer: *pSamplesRead %d after transact", *pSamplesRead);
           *pSamplesRead = (int32_t)reply.readInt32();
           // ListenService is checking for *pSamplesRead > numSamples
           if (*pSamplesRead > 0) {
              // %%% copy buffer from reply parcel
              reply.read((uint8_t*)pSampleBuff, (*pSamplesRead*2)); // read short[] from parcel
           }
        }
        return status;
    }

    virtual listen_status_enum_t stopBuffering(
            listen_session_id_t mSessionId ) {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(mSessionId);
        remote()->transact(STOP_BUFFERING, data, &reply);
        return (listen_status_enum_t)(reply.readInt32());
    }

    virtual listen_status_enum_t stopDetection(
            listen_session_id_t mSessionId ) {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(mSessionId);
        remote()->transact(STOP_DETECTION, data, &reply);
        return (listen_status_enum_t)(reply.readInt32());
    }

    virtual listen_status_enum_t startDetection(
            listen_session_id_t mSessionId ) {
        Parcel data, reply;
        data.writeInterfaceToken(IListenService::getInterfaceDescriptor());
        data.writeInt32(mSessionId);
        remote()->transact(START_DETECTION, data, &reply);
        return (listen_status_enum_t)(reply.readInt32());
    }
};

IMPLEMENT_META_INTERFACE(ListenService, "com.qualcomm.listen.IListenService");

// ----------------------------------------------------------------------

status_t BnListenService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    listen_status_enum_t listenStatus = LISTEN_SUCCESS;
    status_t status = NO_ERROR;
    switch (code) {
        case SET_MASTER_CONTROL_RECEIVER:
        {
            ALOGI("BnListenService::onTransact received SET_MASTER_CONTROL_RECEIVER");
            CHECK_INTERFACE(IListenService, data, reply);
            // acquire MasterControl for the given receiver
            // call ListenService.requestMasterControl()
            pid_t pid = data.readInt32();
            sp<IListenReceiver> client =
                interface_cast<IListenReceiver>(data.readStrongBinder());
            ALOGD("BnListenService::onTransact calls initMasterControl");
            bool bGranted = initMasterControl(pid, client);
            ALOGD("BnListenService::onTransact initMasterControl returns %d", bGranted);
            if ( ! bGranted ) {
                ALOGV("initMasterControl request NOT granted");
            } else {
                ALOGV("initMasterControl succeeded");
            }
            reply->writeInt32(bGranted);
        } break;
        case CLEAR_MASTER_CONTROL_RECEIVER:
        {
            ALOGI("BnListenService::onTransact received CLEAR_MASTER_CONTROL_RECEIVER");
            CHECK_INTERFACE(IListenService, data, reply);
            listenStatus = releaseMasterControl();
            reply->writeInt32(listenStatus);
        } break;
        case CREATE_SESSION_RECEIVER:
        {
            ALOGI("BnListenService::onTransact received CREATE_SESSION_RECEIVER");
            listen_session_id_t newSessionId;
            CHECK_INTERFACE(IListenService, data, reply);
            listen_session_enum_t sessionType = (listen_session_enum_t)data.readInt32();
            pid_t pid = data.readInt32();
            sp<IListenReceiver> client =
                interface_cast<IListenReceiver>(data.readStrongBinder());
            newSessionId = initSession(sessionType, pid, client);
            if (UNDEFINED == newSessionId) {
                ALOGV("initSession returned Id UNDEFINED");
            } else {
                ALOGV("initSession succeeded");
            }
            reply->writeInt32(newSessionId);
        } break;
        case DESTROY_SESSION_RECEIVER:
        {
            ALOGI("BnListenService::onTransact received DESTROY_SESSION_RECEIVER");
            CHECK_INTERFACE(IListenService, data, reply);
            listen_session_id_t sessionId = data.readInt32();
            listenStatus = destroySession(sessionId);
            reply->writeInt32(listenStatus);
        } break;
        case SET_PARAMETER :
        {
            ALOGI("BnListenService::onTransact received SET_PARAMETER");
            CHECK_INTERFACE(IListenService, data, reply);
            listen_param_enum_t eParamType = (listen_param_enum_t)data.readInt32();
            int32_t             value = data.readInt32();
            listenStatus = setParameter(eParamType, value);
            reply->writeInt32(listenStatus);
        } break;
        case GET_PARAMETER :
        {
            ALOGI("BnListenService::onTransact received GET_PARAMETER");
            int32_t   value;
            CHECK_INTERFACE(IListenService, data, reply);
            listen_param_enum_t eParamType = (listen_param_enum_t)data.readInt32();
            value = getParameter(eParamType);
            reply->writeInt32(value);
        } break;
        case REGISTER_SOUNDMODEL :
        {
            ALOGI("BnListenService::onTransact received REGISTER_SOUNDMODEL");
            CHECK_INTERFACE(IListenService, data, reply);
            int   num_kws, num_pairs = 0;
            int   confLevelArraySize = 0;
            listen_sound_model_params_t  soundModelParams;
            listen_sound_model_data_t    modelData;
            soundModelParams.sound_model_data = &modelData;
            listen_session_id_t sessionId = (listen_session_id_t)data.readInt32();
            // peal off values from parcel and store them into sound_model_params data struct
            soundModelParams.sound_model_type = (listen_sound_model_enum_t)data.readInt32();
            ALOGI("nListenService::onTransact model type = %d", soundModelParams.sound_model_type);
            int byteArrSize = data.readInt32();
            soundModelParams.sound_model_data->size = byteArrSize;

            // copy SoundModel
            soundModelParams.sound_model_data->p_data = (uint8_t *)malloc(byteArrSize);
            if (NULL == soundModelParams.sound_model_data->p_data) {
               ALOGE("nListenService::onTransact ERROR malloc for SM failed");
               listenStatus = LISTEN_ENO_MEMORY;
               // drop to end
            } else {
               data.read(soundModelParams.sound_model_data->p_data, byteArrSize); // read byte[] from parcel

               soundModelParams.detection_mode = (listen_detection_mode_enum_t)data.readInt32();
               soundModelParams.detect_failure = (bool)data.readInt32();
               num_kws = (int)data.readInt32();
               soundModelParams.num_keywords = (uint16_t)num_kws;
               num_pairs = (int)data.readInt32();
               soundModelParams.num_user_pairings = (uint16_t)num_pairs;
               confLevelArraySize = num_kws + num_pairs;

               // copy array of confidence levels
               soundModelParams.min_conf_levels = (uint8_t *)malloc(confLevelArraySize);
               if (NULL == soundModelParams.min_conf_levels) {
                  ALOGE("nListenService::onTransact ERROR malloc for ConfLevels failed");
                  listenStatus = LISTEN_ENO_MEMORY;
                  // drop to end
               } else {
                  data.read(soundModelParams.min_conf_levels, confLevelArraySize); // read byte[] from parcel
                  soundModelParams.enable_buffering = (bool)data.readInt32();
                  ALOGI("onTransact(REGISTER_SOUNDMODEL) called with Id %d, Params %p", sessionId, &soundModelParams);
                  listenStatus = registerSoundModel(sessionId, &soundModelParams);
                  ALOGI("... returns %d", listenStatus);
               }
            }
            if (soundModelParams.sound_model_data->p_data)
               free((void *)soundModelParams.sound_model_data->p_data);
            if (soundModelParams.min_conf_levels)
               free((void *)soundModelParams.min_conf_levels);
            reply->writeInt32(listenStatus);
        } break;
        case DEREGISTER_SOUNDMODEL :
        {
            ALOGI("BnListenService::onTransact received DEREGISTER_SOUNDMODEL");
            CHECK_INTERFACE(IListenService, data, reply);
            listen_session_id_t sessionId = (listen_session_id_t)data.readInt32();
            listenStatus = deregisterSoundModel(sessionId);
            reply->writeInt32(listenStatus);
        } break;
        case READ_BUFFER :
        {
            ALOGI("BnListenService::onTransact received READ_BUFFER");
            CHECK_INTERFACE(IListenService, data, reply);
            int16_t *pSampleBuff = NULL;
            int32_t samplesRead = 0;

            listen_session_id_t sessionId = (listen_session_id_t)data.readInt32();
            int32_t numSamples = data.readInt32();

            ALOGD("onTransact: malloc bytes for %d samples", numSamples);
            pSampleBuff = (int16_t *)calloc(numSamples, 2);
            if (NULL == pSampleBuff) {
                ALOGE("onTransact: samplesRead %d", samplesRead);
                reply->writeInt32((int32_t)LISTEN_ENO_MEMORY);
            } else {
               listenStatus = readBuffer(sessionId, numSamples, pSampleBuff, &samplesRead);
               if (listenStatus != LISTEN_SUCCESS)
                  ALOGE("onTransact: ERROR - readBuffer returned %d", (int)status);
               reply->writeInt32(listenStatus);
               reply->writeInt32(samplesRead);
               ALOGD("onTransact: samplesRead %d", samplesRead);
               if (samplesRead > 0) {
                  // %%% copy pSampleBuff into parcel
                  status = reply->write(pSampleBuff, (samplesRead * 2));
                  if (status != NO_ERROR)
                     ALOGE("onTransact: ERROR - write byte array to parcel returned %d", (int)status);
               }
               free((void *)pSampleBuff);
           }
        } break;
        case STOP_BUFFERING :
        {
            ALOGI("BnListenService::onTransact received STOP_BUFFERING");
            CHECK_INTERFACE(IListenService, data, reply);
            listen_session_id_t sessionId = (listen_session_id_t)data.readInt32();
            listenStatus = stopBuffering(sessionId);
            reply->writeInt32(listenStatus);
        } break;
        case STOP_DETECTION :
        {
            ALOGI("BnListenService::onTransact received STOP_DETECTION");
            CHECK_INTERFACE(IListenService, data, reply);
            listen_session_id_t sessionId = (listen_session_id_t)data.readInt32();
            listenStatus = stopDetection(sessionId);
            reply->writeInt32(listenStatus);
        } break;
        case START_DETECTION :
        {
            ALOGI("BnListenService::onTransact received START_DETECTION");
            CHECK_INTERFACE(IListenService, data, reply);
            listen_session_id_t sessionId = (listen_session_id_t)data.readInt32();
            listenStatus = startDetection(sessionId);
            reply->writeInt32(listenStatus);
        } break;
        default:
        {
            ALOGI("BnListenService::onTransact received unrecognized msg %d",code);
            return BBinder::onTransact(code, data, reply, flags);
        } break;
    }
    return status;
}

// ----------------------------------------------------------------------------

}; // namespace android
