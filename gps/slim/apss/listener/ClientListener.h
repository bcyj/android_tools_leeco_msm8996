/*============================================================================
@file ClientListener.h

  Client Listener header file

  Client listener routes sensor data requests from various sensor
  clients to the SLIM daemon task.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef CLIENT_LISTENER_H
#define CLIENT_LISTENER_H

/*----------------------------------------------------------------------------
* Include Files
* -------------------------------------------------------------------------*/
#include "msg_q.h"
#include "log_util.h"
#include "slim_utils.h"
#include "SlimDaemonMsg.h"

/*----------------------------------------------------------------------------
* Declarations
* -------------------------------------------------------------------------*/
class ClientListener {
    void* mMsgQ;
    bool CLMsgQSnd(void* msg_obj, uint32_t msg_sz);
protected:
public:
    ClientListener(void* pMsgQ);
    inline virtual ~ClientListener() {}

    virtual void processClientRegister(
        slim_OpenTxnStructType &request);
    virtual void processClientDeRegister(
        slim_GenericTxnStructType &request);
    virtual void processTimeRequest(
        slim_GetProviderTimeRequestTxnStructType &request);
    virtual void processSensorDataRequest(
        slim_EnableSensorDataTxnStructType &request);
    virtual void processPedometerDataRequest(
        slim_EnablePedometerTxnStructType &request);
    virtual void processMotionDataRequest(
        slim_EnableMotionDataTxnStructType &request);
};

#endif //CLIENT_LISTENER_H
