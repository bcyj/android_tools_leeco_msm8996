/*============================================================================
@file SlimSensor1ProviderWrapper.h

  SLIM Sensor1 Provider Wrapper header file

  Wrapper for Sensor1 provider handling of sensor data requests.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef SENSOR1_PROVIDER_WRAPPER_H
#define SENSOR1_PROVIDER_WRAPPER_H

#include <log_util.h>
#include "SlimSensor1Provider.h"
#include "slim_client_types.h"
#include "slim_provider_data.h"
#include "slim_core.h"
#include "SlimDaemonMsg.h"
#include "SlimProviderCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Opens provider connection for specific client.

Function opens provider connection for a specific client.

@param  pz_Provider : Provider object
@param  p_Handle : Pointer to the client handle.
@return eSLIM_SUCCESS if connection is opened successfully.
*/
slimErrorEnumT slim_Sensor1OpenClientConnection(
    slim_ServiceProviderInterfaceType *pz_Provider,
    const slimClientHandleT p_Handle);


/**
@brief Closes provider connection for specific client.

Function closes sensor data provider connection for a specific client and
frees all allocated resources.

@param  pz_Provider : Provider object
@param  p_Handle : Pointer to the client handle.
@return eSLIM_SUCCESS if connection is closed successfully.
*/
slimErrorEnumT slim_Sensor1CloseClientConnection(
    slim_ServiceProviderInterfaceType *pz_Provider,
    const slimClientHandleT p_Handle);


/**
@brief Initializes Sensor1 provider module.

Function initializes Sensor1 provider module.

@return Pointer to the Sensor1 provider implementation.
*/
void slim_Sensor1Initialize(slim_ServiceProviderInterfaceType **pz_Provider);

/**
@brief Handler for messages targeted for provider.

Function handles messages sent via SLIM core to provider.

@param  pz_Provider : Sensor1 provider object
@param  q_MessageId : Message id.
@param  q_MsgPayloadSize : Message size.
@param  p_MsgPayload : Pointer to the message payload.
*/
void slim_Sensor1HandleProviderMessage
(
  slim_ServiceProviderInterfaceType *pz_Provider,
  uint32 q_MessageId,
  uint32 q_MsgPayloadSize,
  const void *p_MsgPayload
);

/**
@brief Initiates time offset request.

Function for making the time request. Successful response enable SLIM to
calculate the offset between modem time and sensor time.

@param  pz_Provider : Provider object
@param  l_ServiceTxnId : Service transaction id.
@return eSLIM_SUCCESS if time request is made successfully.
*/
slimErrorEnumT slim_Sensor1GetTimeUpdate(
    slim_ServiceProviderInterfaceType *pz_Provider,
    int32 l_ServiceTxnId);

/**
@brief Initiates sensor data request.

Function initiates sensor data request to Sensor1. If request is successfull,
Sensor1 sensor data streaming is either started or stopped.

@param  pz_Provider : Provider object
@param  pz_Txn : Pointer to the transaction data.
@param  l_ServiceTxnId : Service transaction id.
@return eSLIM_SUCCESS if sensor data request is made successfully.
*/
slimErrorEnumT slim_Sensor1EnableSensorDataRequest(
    slim_ServiceProviderInterfaceType *pz_Provider,
    const slim_EnableSensorDataTxnStructType *pz_Txn,
    int32 l_ServiceTxnId);


/**
@brief Initiates pedometer request.

Function initiates pedometer request to Sensor1. If request is successfull,
pedometer streaming via Sensor1 is either started or stopped.

@param  pz_Provider : Provider object
@param  pTxn : Pointer to the transaction data.
@param  l_ServiceTxnId : Service transaction id.
@return eSLIM_SUCCESS if the request is made successfully.
*/
slimErrorEnumT slim_Sensor1EnablePedometerRequest(
    slim_ServiceProviderInterfaceType *pz_Provider,
    const slim_EnablePedometerTxnStructType *pz_Txn,
    int32 l_ServiceTxnId);


/**
@brief Initiates motion data request to Sensor1.

Function initiates motion data request to Sensor1. If request is successfull,
Sensor1 motion data streaming is either started or stopped.

@param  pz_Provider : Provider object
@param  pz_Txn: Pointer to motion data request transaction data.
@param  l_ServiceTxnId : Service transaction id.
@return Sensor1_SERVICE_SUCCESS if request is done successfully. Otherwise Sensor1 error
code.
*/
slimErrorEnumT slim_Sensor1EnableMotionDataRequest(
    slim_ServiceProviderInterfaceType *pz_Provider,
    const slim_EnableMotionDataTxnStructType *pz_Txn,
    int32 l_ServiceTxnId);



/**
@brief Returns TRUE if service is supported by the provider.

Function returns TRUE if service is supported by the provider.

@param  pz_Provider : Provider object
@param  e_Service : SLIM service
@return TRUE if service is supported. FALSE otherwise.
*/

boolean slim_Sensor1SupportsService(
    slim_ServiceProviderInterfaceType *pz_Provider,
    slimServiceEnumT e_Service);

/**
@brief Provides timesync mt offset for the SLIM service.

Function provides timesync mt offset for the SLIM service.

@param  pz_Provider : Provider object
@param  p_Handle : Pointer to the client handle
@param  e_Service : SLIM service.
@param  pz_MtOffset : Pointer to mt offset. Data is stored to this variable if
found.
@return TRUE if mt data was found. FALSE otherwise.
*/
boolean slim_Sensor1GetMtOffsetForService
(
    slim_ServiceProviderInterfaceType *pz_Provider,
    const slimClientHandleT p_Handle,
    slimServiceEnumT e_Service,
    slim_ServiceMtDataStructType *pz_MtOffset);

/**
@brief Sets timesync mt offset for the SLIM service.

Function sets timesync mt offset for the SLIM service.

@param  pz_Provider : Provider object
@param  p_Handle : Pointer to the client handle. If NULL, data is applied to all
clients.
@param  e_Service : SLIM service. If eSLIM_SERVICE_NONE, data is applied to all
services.
@param  pz_MtOffset : Pointer to mt offset to be set.
@param  b_SetForAllClients : TRUE if data should be applied to all clients. In
this case parameter p_Handle is ignored. Otherwise FALSE.
*/
void slim_Sensor1SetMtOffsetForService
(
    slim_ServiceProviderInterfaceType *pz_Provider,
    const slimClientHandleT p_Handle,
    slimServiceEnumT e_Service,
    const slim_ServiceMtDataStructType *pz_MtOffset,
    boolean b_SetForAllClients);

/**
@brief Dispatch function for data indications

Sensor1 modules should use this function to dispatch data indications to SLIM core.

@param  e_Service: SLIM service.
@param  e_Error: Indication error code.
@param  e_MessageId: Indication message id.
@param  p_SlimDaemonMessage: Pointer to the message data.
*/
void slim_Sensor1RouteIndication
(
  slimServiceEnumT e_Service,
  slimErrorEnumT e_Error,
  slimMessageIdEnumT e_MessageId,
  SlimDaemonMessage* pSlimDaemonMessage);

#ifdef __cplusplus
}
#endif
#endif //SENSOR1_PROVIDER_WRAPPER_H
