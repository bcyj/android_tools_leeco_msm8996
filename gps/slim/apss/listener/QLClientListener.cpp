/*============================================================================
  FILE:          QLClientListener.cpp

  OVERVIEW:      QMI LOC client listener is interface to modem sensor clients
                 (SAP & SAMLITE) Sensor data requests and sensor data
                 injection is through QMI LOC transport.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/* LIBC header control macros. */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <log_util.h>
#include <QLClientListener.h>
#include "slim_processor.h"
#include "slim_core.h"
#include <location_service_v02.h>
#include <loc_api_sync_req.h>
#include <LocDualContext.h>

#undef LOG_TAG
#define LOG_TAG "Slim_QLClientListener"

namespace ql_client_listener {

QLClientListener* QLClientListener::mMe = NULL;
LOC_API_ADAPTER_EVENT_MASK_T QLClientListener::mMask =
    LOC_API_ADAPTER_SENSOR_STATUS |
    LOC_API_ADAPTER_REQUEST_TIME_SYNC |
    LOC_API_ADAPTER_MOTION_CTRL |
    LOC_API_ADAPTER_PEDOMETER_CTRL;//LOC_API_ADAPTER_REPORT_SPI//TODO:

/**
@brief Constructor

Function class initialization

@param  tCreator: Msgtask creator
@param  pMsgQ: Message queue
@param  provider: Preferred provider
*/
QLClientListener::QLClientListener(
    MsgTask::tCreate tCreator, void* pMsgQ,
    slimServiceProviderEnumT provider):
    LBSAdapterBase(
        QLClientListener::mMask,
        LocDualContext::getLocFgContext(
            tCreator,
            LocDualContext::mLocationHalName)),
    ClientListener(pMsgQ), mClientId(SLIM_CLIENT_MPSS),
    mClientTxnId(1), mProvider(eSLIM_SERVICE_PROVIDER_DEFAULT)
{
    LOC_LOGD("%s:%d]: QLClientListener created. QLClientListener: %p\n",
             __func__, __LINE__, this);

    memset(mSensorSessionInfo, 0, sizeof(mSensorSessionInfo));

    //TODO: Do we still need this provider info after updated core?
    QLClientListener::mProvider = provider;

    slim_OpenTxnStructType z_Txn;
    memset(&z_Txn, 0, sizeof(z_Txn));

    z_Txn.p_Handle = &mClientId;
    z_Txn.fn_Callback = QLClientListener::forwardCallbackData;
    SLIM_MASK_SET(z_Txn.q_OpenFlags, eSLIM_OPEN_FLAGS_NO_TIME_CONVERSION);
    z_Txn.t_CallbackData = 0;
    processClientRegister(z_Txn);

}

/**
@brief Destructor

Function class deinit

@param  pMsgQ: msg q data
*/
QLClientListener::~QLClientListener()
{
    slim_GenericTxnStructType z_Txn;

    LOC_LOGD( "Calling slim_Close");
    memset(&z_Txn, 0, sizeof(z_Txn));
    z_Txn.e_Service = eSLIM_SERVICE_NONE;
    z_Txn.e_Type = SLIM_REQUEST_CLOSE;
    z_Txn.z_TxnData.p_Handle = &mClientId;

    processClientDeRegister(z_Txn);
}


/**
@brief Get class object

Function to get class object. Class is a singleton.

@param  tCreator: Msgtask creator
@param  pMsgQ: Message queue
@param  provider: Preferred provider, this information is required
so as to send as parameter for time sync request
*/
QLClientListener* QLClientListener::get(void* pMsgQ,
                                        slimServiceProviderEnumT provider)
{
    ENTRY_LOG();
    if (NULL == mMe) {
        mMe = new QLClientListener(NULL, pMsgQ, provider);
    }
    return mMe;
}

/**
@brief Called after sub system restart for session recovery

Function Basic handling after SSR if there is an active session and
         sensor providers are engaged.
*/
void QLClientListener::handleEngineUpEvent()
{

    ENTRY_LOG_CALLFLOW();
    //If any sensor session in progress send stop request to
    //the sensor providers

    for(int i=0;i<=eSLIM_SERVICE_LAST;i++)
    {
        LOC_LOGD("%s: mSensorSessionInfo[%d]=%d", __func__,i,mSensorSessionInfo[i]);
        if(mSensorSessionInfo[i] == 1)
        {
            switch(i)
            {
            case eSLIM_SERVICE_SENSOR_ACCEL:
            case eSLIM_SERVICE_SENSOR_GYRO:
            case eSLIM_SERVICE_SENSOR_ACCEL_TEMP:
            case eSLIM_SERVICE_SENSOR_GYRO_TEMP:
            case eSLIM_SERVICE_SENSOR_MAG_CALIB:
            case eSLIM_SERVICE_SENSOR_MAG_UNCALIB:
                SensorRequest sensorRequest;
                memset(&sensorRequest, 0, sizeof(sensorRequest));
                sensorRequest.requestType = (SensorType)i;
                sensorRequest.requestValue = REQUEST_STOP;
                requestSensorData(sensorRequest);
                mSensorSessionInfo[i] = 0;
                break;
            case eSLIM_SERVICE_PEDOMETER:
                PedometerRequest pedometerRequest;
                memset(&pedometerRequest, 0, sizeof(pedometerRequest));
                pedometerRequest.requestValue = REQUEST_STOP;
                requestPedometerData(pedometerRequest);
                mSensorSessionInfo[i] = 0;
                break;
            case eSLIM_SERVICE_MOTION_DATA :
                SensorRequest motionDataRequest;
                memset(&motionDataRequest, 0, sizeof(motionDataRequest));
                motionDataRequest.requestType = (SensorType)i;
                motionDataRequest.requestValue = REQUEST_STOP;
                requestMotionData(motionDataRequest);
                mSensorSessionInfo[i] = 0;
                break;
            default:
                break;
            }
        }
    }
}

/**
@brief Enabling or disabling of sensor data streaming for MPSS client

Function handles enabling or disabling of sensor data
streaming for MPSS client

@param  request: Request parameters for sensor streaming
*/
bool QLClientListener::requestSensorData(const SensorRequest &request)
{
    ENTRY_LOG();
    slim_EnableSensorDataTxnStructType z_Txn;
    memset(&z_Txn, 0, sizeof(z_Txn));

    //TODO: Have a list of sensor requests to help handle SSR
    bool returnValue = true;
    z_Txn.z_TxnData.p_Handle = &mClientId;
    z_Txn.z_TxnData.u_ClientTxnId = mClientTxnId;
    SLIM_MASK_SET(z_Txn.z_Request.enableConf.providerFlags,
                  QLClientListener::mProvider);

    switch(request.requestType)
    {
    case ACCEL:
        z_Txn.z_Request.sensor = eSLIM_SERVICE_SENSOR_ACCEL;
        break;
     case GYRO:
        z_Txn.z_Request.sensor = eSLIM_SERVICE_SENSOR_GYRO;
        break;
    case ACCEL_TEMPERATURE:
        z_Txn.z_Request.sensor = eSLIM_SERVICE_SENSOR_ACCEL_TEMP;
        break;
    case GYRO_TEMPERATURE:
        z_Txn.z_Request.sensor = eSLIM_SERVICE_SENSOR_GYRO_TEMP;
        break;
    case MAG_CALIB:
        z_Txn.z_Request.sensor = eSLIM_SERVICE_SENSOR_MAG_CALIB;
        break;
    case MAG_UNCALIB:
        z_Txn.z_Request.sensor = eSLIM_SERVICE_SENSOR_MAG_UNCALIB;
        break;
    default:
        returnValue = false;
        LOC_LOGI("%s: Request for unknown sensor type", __func__);
        break;
    }

    if(returnValue)
    {
        z_Txn.z_Request.enableConf.enable = request.requestValue;
        z_Txn.z_Request.sampleCount = request.requestRate.samplesPerBatch;
        z_Txn.z_Request.reportRate = request.requestRate.batchesPerSecond;

        mSensorSessionInfo[z_Txn.z_Request.sensor] = z_Txn.z_Request.enableConf.enable;

        ClientListener::processSensorDataRequest(z_Txn);
    }
    EXIT_LOG(%d, 0);
    return returnValue;
}

/**
@brief Enabling or disabling of pedometer data streaming for MPSS client

Function handles enabling or disabling of pedometer data
streaming for MPSS client

@param  request: Request parameters for pedometer streaming
*/
bool QLClientListener::requestPedometerData(const PedometerRequest &request)
{
    ENTRY_LOG();
    slim_EnablePedometerTxnStructType z_Txn;
    memset(&z_Txn, 0, sizeof(z_Txn));

    z_Txn.z_TxnData.p_Handle = &mClientId;
    z_Txn.z_TxnData.u_ClientTxnId = mClientTxnId;

    SLIM_MASK_SET(z_Txn.z_Request.enableConf.providerFlags,
                  QLClientListener::mProvider);

    z_Txn.z_Request.enableConf.enable = request.requestValue;
    z_Txn.z_Request.resetStepCount_valid = request.resetStepCountValid;
    z_Txn.z_Request.resetStepCount = request.resetStepCount;
    z_Txn.z_Request.stepCountThreshold_valid = request.stepCountThresholdValid;
    z_Txn.z_Request.stepCountThreshold = request.stepCountThreshold;

    mSensorSessionInfo[eSLIM_SERVICE_PEDOMETER] = z_Txn.z_Request.enableConf.enable;

    ClientListener::processPedometerDataRequest(z_Txn);

    EXIT_LOG(%d, 0);
    return true;
}

/**
@brief Enabling or disabling of motion data streaming for MPSS client

Function handles enabling or disabling of motion data
streaming for MPSS client

@param  request: Request parameters for motion data streaming
*/
bool QLClientListener::requestMotionData(const SensorRequest &request)
{
    ENTRY_LOG();
    slim_EnableMotionDataTxnStructType z_Txn;
    memset(&z_Txn, 0, sizeof(z_Txn));

    z_Txn.z_TxnData.p_Handle = &mClientId;
    z_Txn.z_TxnData.u_ClientTxnId = mClientTxnId;

    //TODO:
    SLIM_MASK_SET(z_Txn.z_Request.enableConf.providerFlags,
                  QLClientListener::mProvider);

    z_Txn.z_Request.enableConf.enable = request.requestValue;

    mSensorSessionInfo[eSLIM_SERVICE_MOTION_DATA] = z_Txn.z_Request.enableConf.enable;

    ClientListener::processMotionDataRequest(z_Txn);
    EXIT_LOG(%d, 0);
    return true;
}

/**
@brief Request time sync data for MPSS client

Function requests time sync data for MPSS client

@param  request: Request data for time sync
*/
bool QLClientListener::requestTimeData(const TimeRequest &timeRequest)
{
    ENTRY_LOG();
    slim_GetProviderTimeRequestTxnStructType z_Txn;
    memset(&z_Txn, 0, sizeof(z_Txn));

    z_Txn.z_TxnData.p_Handle = &mClientId;
    z_Txn.z_Request.referenceTimeMs = timeRequest.clientTxTime;
    z_Txn.z_Request.provider = QLClientListener::mProvider;
    // We are always interested in the time of the accel provider
    z_Txn.z_Request.service = eSLIM_SERVICE_SENSOR_ACCEL;

    ClientListener::processTimeRequest(z_Txn);
    EXIT_LOG(%d, 0);
    return true;
}

/**
@brief Handle sensor data injection

Function handles sensor data injection

@param dataBundle:  Sensor data
*/
bool QLClientListener::injectSensorData(SensorDataBundle &dataBundle)
{

        struct QLInjectSensorData: public LocMsg {
            LBSApiBase* mLBSApi;
            SensorDataBundle mDataBundle;
            inline QLInjectSensorData(LBSApiBase* lbsApi,
                                      SensorDataBundle &dataBundle) :
                LocMsg(), mLBSApi(lbsApi), mDataBundle(dataBundle) {}
            inline virtual void proc() const {
                LOC_LOGV("Calling mLBSApi->injectSensorData");
                mLBSApi->injectSensorData(mDataBundle);
            }
        };
        sendMsg(new QLInjectSensorData(mLBSApi, dataBundle));

    return true;
}

/**
@brief Handle pedometer data injection

Function handles pedometer data injection

@param pedometerData:  Pedometer data
*/
bool QLClientListener::injectPedometerData(PedometerData &pedometerData)
{
        struct QLInjectPedometerData: public LocMsg {
            LBSApiBase* mLBSApi;
            PedometerData mPedometerData;
            inline QLInjectPedometerData(LBSApiBase* lbsApi,
                                         PedometerData &pedometerData) :
                LocMsg(), mLBSApi(lbsApi), mPedometerData(pedometerData) {}
            inline virtual void proc() const {
                LOC_LOGV("Calling mLBSApi->injectPedometerData");
                mLBSApi->injectPedometerData(mPedometerData);
            }
        };
        sendMsg(new QLInjectPedometerData(mLBSApi, pedometerData));

    return true;
}

/**
@brief Handle motion data injection

Function handles motion data injection

@param motionData:  Motion data
*/
bool QLClientListener::injectMotionData(MotionData &motionData)
{
        struct QLInjectMotionData: public LocMsg {
            LBSApiBase* mLBSApi;
            MotionData mMotionData;
            inline QLInjectMotionData(LBSApiBase* lbsApi, MotionData &motionData) :
                LocMsg(), mLBSApi(lbsApi), mMotionData(motionData) {}
            inline virtual void proc() const {
                LOC_LOGV("Calling mLBSApi->injectMotionData");
                mLBSApi->injectMotionData(mMotionData);
            }
        };
        sendMsg(new QLInjectMotionData(mLBSApi, motionData));

    return true;
}

/**
@brief Handle time sync data injection

Function handles time sync data injection

@param timeData:  Time sync data
*/
bool QLClientListener::injectTimeData(TimeData &timeData)
{
        struct QLInjectTimeData: public LocMsg {
            LBSApiBase* mLBSApi;
            TimeData mTimeData;
            inline QLInjectTimeData(LBSApiBase* lbsApi, TimeData &timeData) :
                LocMsg(), mLBSApi(lbsApi), mTimeData(timeData) {}
            inline virtual void proc() const {
                LOC_LOGV("Calling mLBSApi->injectTimeData");
                mLBSApi->injectTimeData(mTimeData);
            }
        };
        sendMsg(new QLInjectTimeData(mLBSApi, timeData));

    return true;
}

/**
@brief Local function that handles callback data

Functionis called to handle indication/response routing

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
void QLClientListener::callbackHandler(
    uint64 t_CallbackData,
    const slimMessageHeaderStructT *pz_MessageHeader,
    void *p_Message)
{
    ENTRY_LOG();
    if (NULL == pz_MessageHeader)
    {
        return;
    }

    if (eSLIM_MESSAGE_ID_PROVIDER_TIME_GET_RESP == (int32)pz_MessageHeader->msgId)
    {
      handleTimeService(t_CallbackData, pz_MessageHeader, p_Message);
      return;
    }

    switch (pz_MessageHeader->service)
    {
    case eSLIM_SERVICE_SENSOR_ACCEL:
    case eSLIM_SERVICE_SENSOR_ACCEL_TEMP:
    case eSLIM_SERVICE_SENSOR_GYRO:
    case eSLIM_SERVICE_SENSOR_GYRO_TEMP:
    case eSLIM_SERVICE_SENSOR_MAG_CALIB:
    case eSLIM_SERVICE_SENSOR_MAG_UNCALIB:
        handleSensorService(t_CallbackData, pz_MessageHeader, p_Message);
        break;
    case eSLIM_SERVICE_MOTION_DATA:
        handleMotionDataService(t_CallbackData, pz_MessageHeader, p_Message);
        break;
    case eSLIM_SERVICE_PEDOMETER:
        handlePedometerService(t_CallbackData, pz_MessageHeader, p_Message);
        break;
        //TODO: SPI
    default:
        break;
    }
}

/**
@brief Handler for all sensor service messages

Function is handler for all sensor service messages

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
bool QLClientListener::handleSensorService(
    uint64 t_CallbackData,
    const slimMessageHeaderStructT *pz_MessageHeader,
    void *p_Message)
{
    SLIM_UNUSED(t_CallbackData);

    if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_SENSOR_DATA_IND)
    {
        if (eSLIM_SUCCESS == pz_MessageHeader->msgError &&
            NULL != p_Message)
        {

            slimSensorDataStructT *pz_SensorData =
                (slimSensorDataStructT*)p_Message;

            SensorDataBundle dataBundle;
            memset(&dataBundle, 0, sizeof(dataBundle));

            LOC_LOGI("%s: Received sensor data", __func__);
            switch (pz_MessageHeader->service)
            {
            case eSLIM_SERVICE_SENSOR_ACCEL:
                dataBundle.sensorType = ACCEL;
                break;
            case eSLIM_SERVICE_SENSOR_ACCEL_TEMP:
                dataBundle.sensorType = ACCEL_TEMPERATURE;
                break;
            case eSLIM_SERVICE_SENSOR_GYRO:
                dataBundle.sensorType = GYRO;
                break;
            case eSLIM_SERVICE_SENSOR_GYRO_TEMP:
                dataBundle.sensorType = GYRO_TEMPERATURE;
                break;
            case eSLIM_SERVICE_SENSOR_MAG_CALIB:
                dataBundle.sensorType = MAG_CALIB;
                dataBundle.flags |= CALIBRATED_DATA;
                break;
            case eSLIM_SERVICE_SENSOR_MAG_UNCALIB:
                dataBundle.sensorType = MAG_UNCALIB;
                break;
            default:
                break;
            }
            dataBundle.timeOfFirstSample = pz_SensorData->timeBase;

            /*Time source is set to unspecified for modem clients so that time sync
              can be done, cannot be assumed to be common*/
            switch (pz_SensorData->timeSource)
            {
            case eSLIM_TIME_SOURCE_UNSPECIFIED:
            case eSLIM_TIME_SOURCE_COMMON:
                dataBundle.timeSource = UNSPECIFIED;
                break;
            default:
                dataBundle.timeSource = UNSPECIFIED;
                break;
            }

            dataBundle.sensorDataLength = pz_SensorData->samples_len;

            LOC_LOGV("%s: Received sensor-%d data with len-%d", __func__,
                     pz_SensorData->sensorType, pz_SensorData->samples_len);

            for(uint32_t i=0;i<pz_SensorData->samples_len;i++)
            {

                dataBundle.samples[i].timeOffset =
                        slim_TimeToMilliseconds(pz_SensorData->samples[i].sampleTimeOffset);
                dataBundle.samples[i].xAxis = pz_SensorData->samples[i].sample[0];
                dataBundle.samples[i].yAxis= pz_SensorData->samples[i].sample[1];
                dataBundle.samples[i].zAxis = pz_SensorData->samples[i].sample[2];

                LOC_LOGV("%s: Received data with time offset-%d, data-(%f,%f,%f)",
                         __func__,
                         dataBundle.samples[i].timeOffset,
                         dataBundle.samples[i].xAxis,
                         dataBundle.samples[i].yAxis,
                         dataBundle.samples[i].zAxis);
            }

            LOC_LOGI("%s: Sending Sensor Data to LocApi.",__func__);
            injectSensorData(dataBundle);
        }
    }
    else if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_SENSOR_DATA_ENABLE_RESP)
    {
        LOC_LOGI("%s: Received sensor enable response", __func__);
    }
    else
    {
         LOC_LOGI("%s: Received invalid message", __func__);
    }
    return true;
}

/**
@brief Handler for all pedometer service messages

Function is handler for all pedometer service messages

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
bool QLClientListener::handlePedometerService(
    uint64 t_CallbackData,
    const slimMessageHeaderStructT *pz_MessageHeader,
    void *p_Message)
{
    SLIM_UNUSED(t_CallbackData);

    if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_PEDOMETER_IND)
    {
        if (eSLIM_SUCCESS == pz_MessageHeader->msgError &&
            NULL != p_Message)
        {
            LOC_LOGI("%s: Received pedometer inject indication ", __func__);
            PedometerData pedometerData;
            memset(&pedometerData, 0, sizeof(pedometerData));

            slimPedometerDataStructT *pz_PedometerData =
                (slimPedometerDataStructT*)p_Message;

            pedometerData.timeSource = (TimeSourceType)pz_PedometerData->timeSource;
            pedometerData.timestamp = pz_PedometerData->timestamp;
            pedometerData.timeInterval = pz_PedometerData->timeInterval;

            pedometerData.stepCount = pz_PedometerData->stepCount;

            pedometerData.stepConfidenceValid = pz_PedometerData->stepConfidence_valid;
            pedometerData.stepConfidence = pz_PedometerData->stepConfidence;

            pedometerData.stepCountUncertaintyValid = pz_PedometerData->stepCountUncertainty_valid;
            pedometerData.stepCountUncertainty = pz_PedometerData->stepCountUncertainty;

            pedometerData.stepRateValid = pz_PedometerData->stepRate_valid;
            pedometerData.stepRate = pz_PedometerData->stepRate;


            LOC_LOGI("%s: Received Pedometer data,"
                     "step_confidence = %u, step_count = %" PRIu32 ", "
                     "step_count_uncertainity = %f, step_rate = %4.2f Hz",
                     __FUNCTION__, pz_PedometerData->stepConfidence,
                     pz_PedometerData->stepCount, pz_PedometerData->stepCountUncertainty,
                     pz_PedometerData->stepRate);

            LOC_LOGI("%s: Sending Pedometer Data to LocApi",
                     __func__);
            injectPedometerData(pedometerData);
        }

    }
    else if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_PEDOMETER_ENABLE_RESP)
    {
        LOC_LOGI("%s: Received pedometer response", __func__);
    }
    else
    {
         LOC_LOGI("%s: Received invalid message", __func__);
    }
    return true;
}

//TODO:
/**
@brief Handler for all SPI service messages

Function is handler for all SPI service messages

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
// bool QLClientListener::handleSPIService(uint64 t_CallbackData,
//                                     const slimMessageHeaderStructT *pz_MessageHeader,
//                                      void *p_Message)
// {
// }

/**
@brief Handler for all motion data service messages

Function is handler for all motion data service messages

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
bool QLClientListener::handleMotionDataService(uint64 t_CallbackData,
                                      const slimMessageHeaderStructT *pz_MessageHeader,
                                      void *p_Message)
{
    SLIM_UNUSED(t_CallbackData);

    if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_MOTION_DATA_IND)
    {
        if (eSLIM_SUCCESS == pz_MessageHeader->msgError &&
            NULL != p_Message)
        {
            LOC_LOGI("%s: Received motion data inject indication", __func__);
            MotionData motionData;
            memset(&motionData, 0, sizeof(motionData));

            slimMotionDataStructT *pz_MotionData =
                (slimMotionDataStructT*)p_Message;

            motionData.state = (MotionStateType)pz_MotionData->motionState;
            motionData.mode = (MotionModeType)pz_MotionData->motionMode;

            motionData.probability = pz_MotionData->probabilityOfState;
            motionData.age = pz_MotionData->age;
            motionData.timeout = pz_MotionData->timeout;

            LOC_LOGI("%s: Motion data update sent with state = %d and probability = %f",
                     __FUNCTION__, pz_MotionData->motionState, pz_MotionData->probabilityOfState);

            LOC_LOGI("%s: Sending Motion Data to LocApi",
                     __func__);
            injectMotionData(motionData);
        }

    }
    else if(pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_MOTION_DATA_ENABLE_RESP)
    {
        LOC_LOGI("%s: Received motion data enable response", __func__);
    }
    else
    {
         LOC_LOGI("%s: Received invalid message", __func__);
    }
    return true;
}

/**
@brief Handler for all time service messages

Function is handler for all time service messages

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
bool QLClientListener::handleTimeService(uint64 t_CallbackData,
                                      const slimMessageHeaderStructT *pz_MessageHeader,
                                      void *p_Message)
{
    SLIM_UNUSED(t_CallbackData);

    if((int32)pz_MessageHeader->msgId == eSLIM_MESSAGE_ID_PROVIDER_TIME_GET_RESP)
    {
         if (eSLIM_SUCCESS == pz_MessageHeader->msgError &&
            NULL != p_Message)
         {
             LOC_LOGI("%s: Received time data inject indication", __func__);
             TimeData timeData;
             memset(&timeData, 0, sizeof(timeData));

             slimGetProviderTimeResponseStructT *pz_TimeData =
                 (slimGetProviderTimeResponseStructT*)p_Message;

             timeData.clientTxTime = pz_TimeData->referenceTimeMs;
             timeData.remoteProcRxTime = pz_TimeData->remoteReceiveTimeMs;
             timeData.remoteProcTxTime = pz_TimeData->remoteTransmitTimeMs;

             LOC_LOGI("%s: Time data update sent with client time = %d and provider time = %d",
                      __FUNCTION__, pz_TimeData->referenceTimeMs, pz_TimeData->remoteTransmitTimeMs);

             LOC_LOGI("%s: Sending Time Data to LocApi",
                      __func__);
             injectTimeData(timeData);

         }
    }
    else
    {
         LOC_LOGI("%s: Received invalid message", __func__);
    }
    return true;
}

/**
@brief Function that handles callback data

Function is a callback called from SLIM CORE with indication/
response information. In turn calls the non-static member function of
the class to route the handling. Needs to be static to adhere to the callback
function signature defined by slim_api.h

@param t_CallbackData  Callback data
@param pz_MessageHeader  Pointer to message header
@param p_Message  Pointer to message payload
*/
void QLClientListener::forwardCallbackData(uint64_t t_CallbackData,
                        const slimMessageHeaderStructT *pz_MessageHeader,
                        void *p_Message)
{
    ENTRY_LOG();
    if(QLClientListener::mMe != NULL)

    {
        (QLClientListener::mMe)->callbackHandler(t_CallbackData, pz_MessageHeader, p_Message);

    }

}

};//namespace
