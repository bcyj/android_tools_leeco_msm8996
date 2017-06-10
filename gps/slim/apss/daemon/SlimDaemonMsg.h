/******************************************************************************
  @file:  SlimDaemonMsg.h
  @brief: Message structure declarations for SLIM daemon

  DESCRIPTION
    This file defines the messages to be used in the messaging queue
    of the daemon data task.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2011, 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
05/01/11   jb       Initial version
08/01/11   jb       Remove custom Sensor Streaming structure as the QMI
                    version has been enhanced with necessary info.
10/24/12   vr       1. Adding temperature streaming.
                    2. Adding motion data streming.
                    3. Adding pedometer streaming.
======================================================================*/

#ifndef __SLIM_DAEMON_MSG_H__
#define __SLIM_DAEMON_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "slim_utils.h"
#include <pthread.h>

typedef enum
{
    SLIM_DAEMON_INVALID = 0,
    SLIM_DAEMON_CLIENT_OPEN_REQUEST,
    SLIM_DAEMON_CLIENT_CLOSE_REQUEST,
    SLIM_DAEMON_SENSOR_DATA_REQUEST,
    SLIM_DAEMON_SENSOR_DATA_INJECT,
    SLIM_DAEMON_TIME_SYNC_DATA_REQUEST,
    SLIM_DAEMON_TIME_SYNC_DATA_INJECT,
    SLIM_DAEMON_SPI_STATUS_REQUEST,
    SLIM_DAEMON_SPI_STATUS_INJECT,
    SLIM_DAEMON_MOTION_DATA_REQUEST,
    SLIM_DAEMON_MOTION_DATA_INJECT,
    SLIM_DAEMON_PEDOMETER_REQUEST,
    SLIM_DAEMON_PEDOMETER_INJECT,
    SLIM_DAEMON_GENERIC_SERVICE_RESPONSE,
} SlimDaemonMessageType;


typedef struct SlimDaemonMsgHeader
{
    size_t msgSize;
    SlimDaemonMessageType msgType;
} SlimDaemonMsgHeader;

typedef struct SlimDaemonMsgRequestData
{
    union {
        slim_OpenTxnStructType openRequest;
        slim_GenericTxnStructType closeRequest;
        slim_GetProviderTimeRequestTxnStructType timeRequest;
        slim_EnableSensorDataTxnStructType sensorDataRequest;
        slim_EnableMotionDataTxnStructType  motionDataRequest;
        slim_EnablePedometerTxnStructType pedometerDataRequest;
    } data;
} SlimDaemonMsgRequestData;


typedef struct SlimDaemonMsgIndicationData
{
    slim_ProviderEnumType eProvider;
    slimServiceEnumT eService;
    slimErrorEnumT eError;
    slimMessageIdEnumT eMessageId;
    union {
        slimSensorDataStructT sensorData;
        slimPedometerDataStructT pedometerData;
        slimMotionDataStructT motionData;
    } data;

} SlimDaemonMsgIndicationData;

typedef struct SlimDaemonMsgResponseData
{
    slim_ProviderEnumType eProvider;
    slimErrorEnumT eError;
    int32 serviceTxnId;
    union {
        uint64 remoteProviderTime;
    } data;

} SlimDaemonMsgResponseData;


typedef struct SlimDaemonMessage {
    SlimDaemonMsgHeader msgHeader;
    union {
        SlimDaemonMsgRequestData request;
        SlimDaemonMsgResponseData response;
        SlimDaemonMsgIndicationData ind;
    } msgData;

} SlimDaemonMessage;

/**
 * Specifies the different inputs for mounted state indicators.
 */
#define SP_MSI_UNMOUNTED      ((uint8_t)0)
#define SP_MSI_MOUNTED        ((uint8_t)1)
#define SP_MSI_UNKNOWN        ((uint8_t)2)
#define SP_MSI_DO_NOT_SEND    ((uint8_t)3)


/**
 * Different data flow paths supported for
 * retrieving sensor data.
 */
typedef enum e_sensor_provider_type {
   MIN_SENSOR_PROVIDER = 0,

   SENSOR1_SENSOR_PROVIDER,
   ANDROID_NDK_SENSOR_PROVIDER,

   MAX_SENSOR_PROVIDER,
} e_sensor_provider_type;


/*mutex/cv to synchronize sensor1 api calls*/
typedef struct sensor1_access_control_t {
    pthread_mutex_t         cb_mutex;                 /* mutex lock for sensor1 callback */
    pthread_cond_t          cb_arrived_cond;          /* cond variable to signal callback has arrived */
} sensor1_access_control_t;

#ifdef __cplusplus
}
#endif

#endif /* __SLIM_DAEMON_MSG_H__ */
