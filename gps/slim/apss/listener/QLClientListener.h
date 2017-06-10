/*============================================================================
@file QLClientListener.h

  QLClient Listener header file

  QMI LOC client listener is interface to modem sensor clients
  (SAP & SAMLITE) Sensor data requests and sensor data
  injection is through QMI LOC transport.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef QL_CLIENT_LISTENER_H
#define QL_CLIENT_LISTENER_H

#include <LBSAdapterBase.h>
#include <ClientListener.h>
#include "slim_utils.h"
#include <SlimSensor.h>

namespace ql_client_listener {

using namespace loc_core;
using namespace lbs_core;

class QLClientListener :  public LBSAdapterBase, public ClientListener {
    const uint32 mClientId;
    const uint8 mClientTxnId;
    slimServiceProviderEnumT mProvider;
    int mSensorSessionInfo[eSLIM_SERVICE_LAST+1];//keep track of enabled sensors
public:
    static QLClientListener* mMe;
    static LOC_API_ADAPTER_EVENT_MASK_T mMask;
    static QLClientListener* get(void* pMsgQ,
                                 slimServiceProviderEnumT provider);

    static void forwardCallbackData(uint64_t t_CallbackData,
                                    const slimMessageHeaderStructT *pz_MessageHeader,
                                    void *p_Message);

    QLClientListener(MsgTask::tCreate tCreator, void* pMsgQ,
                     slimServiceProviderEnumT provider);
    ~QLClientListener();

    //LBSAdapterBase inherited functions
    virtual bool requestTimeData(const TimeRequest &timeRequest);
    virtual bool requestSensorData(const SensorRequest &request);
    virtual bool requestPedometerData(const PedometerRequest &request);
    virtual bool requestMotionData(const SensorRequest &request);
    virtual void handleEngineUpEvent();

    //class specific functions
    void callbackHandler(uint64 t_CallbackData,
                         const slimMessageHeaderStructT *pz_MessageHeader,
                         void *p_Message);

protected:
    bool handleSensorService(uint64 t_CallbackData,
                             const slimMessageHeaderStructT *pz_MessageHeader,
                             void *p_Message);

    bool handlePedometerService(uint64 t_CallbackData,
                                const slimMessageHeaderStructT *pz_MessageHeader,
                                void *p_Message);

    bool handleMotionDataService(uint64 t_CallbackData,
                                 const slimMessageHeaderStructT *pz_MessageHeader,
                                 void *p_Message);

    bool handleTimeService(uint64 t_CallbackData,
                           const slimMessageHeaderStructT *pz_MessageHeader,
                           void *p_Message);

    virtual bool injectSensorData(SensorDataBundle &dataBundle);
    virtual bool injectMotionData(MotionData &motionData);
    virtual bool injectPedometerData(PedometerData &pedometerData);
    virtual bool injectTimeData(TimeData &timeData);

};

};//namespace
#endif //QL_CLIENT_LISTENER_H
