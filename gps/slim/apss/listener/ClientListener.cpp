/*============================================================================
  FILE:          ClientListener.cpp

  OVERVIEW:      Client listener routes sensor data requests from various
                 sensor clients to the SLIM daemon task by adding message
                 queue.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_NDDEBUG 0
#define LOG_TAG "Slim_ClientListener"
#include <log_util.h>
#include <ClientListener.h>

/**
@brief Constructor

Function class initialization

@param  pMsgQ: msg q data
*/
ClientListener::ClientListener(void* pMsgQ)
{
    LOC_LOGD("%s:%d]: ClientListener created. ClientListener: %p\n",
             __func__, __LINE__, this);
    mMsgQ = pMsgQ;

}

/**
@brief Routes client register to the SLIM daemon task

Function routes client register to the SLIM daemon task.

@param  request: Open request transaction data.
*/
void ClientListener::processClientRegister(slim_OpenTxnStructType &request)
{
    ENTRY_LOG();
    SlimDaemonMessage slimApMessage;
    memset(&slimApMessage, 0, sizeof(slimApMessage));
    SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

    pSlimDaemonMessage->msgHeader.msgSize = sizeof(*pSlimDaemonMessage);
    pSlimDaemonMessage->msgHeader.msgType = SLIM_DAEMON_CLIENT_OPEN_REQUEST;

    pSlimDaemonMessage->msgData.request.data.openRequest = request;

    bool rv = CLMsgQSnd(pSlimDaemonMessage,
                        sizeof(*pSlimDaemonMessage));
    if(!rv)
    {
        LOC_LOGE("%s: Failed sending sensor request", __FUNCTION__);
    }
    EXIT_LOG(%d, rv);
}

/**
@brief Routes client de-register to the SLIM daemon task

Function routes client register to the SLIM daemon task.

@param  request: Generic request transaction data.
*/
void ClientListener::processClientDeRegister(slim_GenericTxnStructType &request)
{
    ENTRY_LOG();
    SlimDaemonMessage slimApMessage;
    memset(&slimApMessage, 0, sizeof(slimApMessage));
    SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

    pSlimDaemonMessage->msgHeader.msgSize = sizeof(*pSlimDaemonMessage);
    pSlimDaemonMessage->msgHeader.msgType = SLIM_DAEMON_CLIENT_CLOSE_REQUEST;

    pSlimDaemonMessage->msgData.request.data.closeRequest = request;

    bool rv = CLMsgQSnd(pSlimDaemonMessage,
                        sizeof(*pSlimDaemonMessage));
    if(!rv)
    {
        LOC_LOGE("%s: Failed sending sensor request", __FUNCTION__);
    }
    EXIT_LOG(%d, rv);

}

/**
@brief Routes time request to the SLIM daemon task

Function routes time request to the SLIM daemon task.

@param  request: Time request transaction data.
*/
void ClientListener::processTimeRequest(
        slim_GetProviderTimeRequestTxnStructType &request)
{
    ENTRY_LOG();
    SlimDaemonMessage slimApMessage;
    memset(&slimApMessage, 0, sizeof(slimApMessage));
    SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

    pSlimDaemonMessage->msgHeader.msgSize = sizeof(*pSlimDaemonMessage);
    pSlimDaemonMessage->msgHeader.msgType = SLIM_DAEMON_TIME_SYNC_DATA_REQUEST;

    pSlimDaemonMessage->msgData.request.data.timeRequest = request;

    bool rv = CLMsgQSnd(pSlimDaemonMessage,
                        sizeof(*pSlimDaemonMessage));
    if(!rv)
    {
        LOC_LOGE("%s: Failed sending time request", __FUNCTION__);
    }
    EXIT_LOG(%d, rv);
}

/**
@brief Routes sensor data request to the SLIM daemon task

Function routes sensor data request to the SLIM daemon task.

@param  request: Sensor data request transaction data.
*/
void ClientListener::processSensorDataRequest(
    slim_EnableSensorDataTxnStructType &request)
{
    ENTRY_LOG();
    SlimDaemonMessage slimApMessage;
    memset(&slimApMessage, 0, sizeof(slimApMessage));
    SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

    pSlimDaemonMessage->msgHeader.msgSize = sizeof(*pSlimDaemonMessage);
    pSlimDaemonMessage->msgHeader.msgType = SLIM_DAEMON_SENSOR_DATA_REQUEST;

    pSlimDaemonMessage->msgData.request.data.sensorDataRequest = request;

    bool rv = CLMsgQSnd(pSlimDaemonMessage,
                        sizeof(*pSlimDaemonMessage));
    if(!rv)
    {
        LOC_LOGE("%s: Failed sending sensor request", __FUNCTION__);
    }
    EXIT_LOG(%d, rv);
}

/**
@brief Routes pedometer data request to the SLIM daemon task

Function routes pedometer data request to the SLIM daemon task.

@param  request: Pedometer data request transaction data.
*/
void ClientListener::processPedometerDataRequest(
        slim_EnablePedometerTxnStructType &request)
{
    ENTRY_LOG();
    SlimDaemonMessage slimApMessage;
    memset(&slimApMessage, 0, sizeof(slimApMessage));
    SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

    pSlimDaemonMessage->msgHeader.msgSize = sizeof(*pSlimDaemonMessage);
    pSlimDaemonMessage->msgHeader.msgType = SLIM_DAEMON_PEDOMETER_REQUEST;

    pSlimDaemonMessage->msgData.request.data.pedometerDataRequest = request;

    bool rv = CLMsgQSnd(pSlimDaemonMessage,
                        sizeof(*pSlimDaemonMessage));
    if(!rv)
    {
        LOC_LOGE("%s: Failed sending pedometer request", __FUNCTION__);
    }

    EXIT_LOG(%d, rv);
}

/**
@brief Routes motion data request to the SLIM daemon task

Function routes motion data request to the SLIM daemon task.

@param  request: Motion data request transaction data.
*/
void ClientListener::processMotionDataRequest(
    slim_EnableMotionDataTxnStructType &request)
{
    ENTRY_LOG();
    SlimDaemonMessage slimApMessage;
    memset(&slimApMessage, 0, sizeof(slimApMessage));
    SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

    pSlimDaemonMessage->msgHeader.msgSize = sizeof(*pSlimDaemonMessage);
    pSlimDaemonMessage->msgHeader.msgType = SLIM_DAEMON_MOTION_DATA_REQUEST;

    pSlimDaemonMessage->msgData.request.data.motionDataRequest = request;

    bool rv = CLMsgQSnd(pSlimDaemonMessage,
                        sizeof(*pSlimDaemonMessage));
    if(!rv)
    {
        LOC_LOGE("%s: Failed sending motion data request", __FUNCTION__);
    }

    EXIT_LOG(%d, rv);
}

/**
@brief Adds messages to daemon message queue

Function adds messages to daemon message queue

@param  msg_obj: message.
@param  msg_sz: message size.
*/
bool ClientListener::CLMsgQSnd(void* msg_obj, uint32_t msg_sz)
{
    ENTRY_LOG();
    void (*dealloc)(void*) = free;

    void* msg_obj_cpy = malloc(msg_sz);
    if( msg_obj_cpy == NULL )
    {
        LOC_LOGE("%s: Memory allocation failure", __FUNCTION__);
        return false;
    }

    memcpy(msg_obj_cpy, msg_obj, msg_sz);

    msq_q_err_type rv = msg_q_snd(mMsgQ, msg_obj_cpy, dealloc);
    if ( rv != eMSG_Q_SUCCESS )
    {
        LOC_LOGE("%s: Invalid Msg Queue send!", __FUNCTION__);
        dealloc(msg_obj_cpy);
        return false;
    }
    EXIT_LOG(%d, rv);
    return true;
}
