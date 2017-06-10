#ifndef SLIM_CORE_H
#define SLIM_CORE_H
/*============================================================================
  @file slim_core.h

  This module initiates all requests to sensor services.

  SLIM core maintains all SLIM related data and provides services for both
  handling of SLIM client requests and provider data/responses.

               Copyright (c) 2013-2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/core/inc/slim_core.h#9 $ */

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "slim_provider_data.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

/**
@brief SLIM core initialization.

This function initializes SLIM module and configures providers for different
SLIM services. No SLIM services are available before this function
is called.
*/
void slim_CoreInit
(
  void
);

/**
@brief SLIM core configuration.

This function configures SLIM service providers using the given provider
setting.

@param e_ProviderSetting : Provider setting that should be used to configure
provider services.
*/
void slim_CoreUpdateConfiguration
(
  slim_ProviderSettingEnumType e_ProviderSetting
);

/**
@brief Engine function for opening client connection.

SLIM core implementation for opening client connection.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreOpenClientConnection
(
  const slim_OpenTxnStructType *pz_Txn
);

/**
@brief Engine function for closing client connection.

SLIM core implementation for closing client connection.

@param  p_Handle: Pointer to the client handle
*/
void slim_CoreCloseClientConnection
(
  slimClientHandleT p_Handle
);

/**
@brief Engine function for forwarding message to provider implementation.

SLIM core implementation for forwarding message to provider implementation.

@param  e_Provider: Provider type.
@param  q_MessageId: Message id.
@param  q_MessageSize: Payload size.
@param  p_Message: Pointer to the payload.
*/
void slim_CoreHandleProviderMessage
(
  slim_ProviderEnumType e_Provider,
  uint32 q_MessageId,
  uint32 q_MessageSize,
  const void *p_Message
);

/**
@brief Engine function for enabling sensor reporting.

SLIM core implementation for enabling sensor reporting.
Engine forwards the sensor reporting request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnableSensorDataRequest
(
  const slim_EnableSensorDataTxnStructType *pz_Txn
);

/**
@brief Engine function for enabling motion data reporting.

SLIM core implementation for enabling motion data reporting.
Engine forwards the sensor reporting request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnableMotionDataRequest
(
  const slim_EnableMotionDataTxnStructType *pz_Txn
);

/**
@brief Engine function for enabling pedometer reporting.

SLIM core implementation for enabling pedometer reporting.
Engine forwards the sensor reporting request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnablePedometerRequest
(
  const slim_EnablePedometerTxnStructType *pz_Txn
);

/**
@brief Engine function for enabling QMD data reporting.

SLIM core implementation for enabling QMD data reporting.
Engine forwards the QMD data reporting request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnableQmdDataRequest
(
  const slim_EnableQmdDataTxnStructType *pz_Txn
);

/**
@brief Engine function for enabling SMD data reporting.

SLIM core implementation for enabling SMD data reporting.
Engine forwards the SMD data reporting request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnableSmdDataRequest
(
  const slim_EnableSmdDataTxnStructType *pz_Txn
);

/**
@brief Engine function for enable distance bound request.

SLIM core implementation for making distance bound request.
Engine forwards the distance bound request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnableDistanceBoundRequest
(
  const slim_EnableDistanceBoundTxnStructType *pz_Txn
);

/**
@brief Engine function for set distance bound request.

SLIM core implementation for making distance bound request.
Engine forwards the distance bound request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreSetDistanceBoundRequest
(
  const slim_SetDistanceBoundTxnStructType *pz_Txn
);

/**
@brief Engine function for get distance bound request.

SLIM core implementation for making distance bound request.
Engine forwards the distance bound request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreGetDistanceBoundRequest
(
  const slim_ClientTxnDataStructType *pz_Txn
);

/**
@brief Engine function for enabling vehicle data reporting.

SLIM core implementation for enabling vehicle data reporting.
Engine forwards the vehicle data reporting request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnableVehicleDataRequest
(
  const slim_EnableVehicleDataTxnStructType *pz_Txn
);

/**
@brief Engine function for enabling pedestrian alignment data reporting.

SLIM core implementation for enabling pedestrian alignment data reporting.
Engine forwards the pedestrian alignment data reporting request to currently
active sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnablePedestrianAlignmentRequest
(
  const slim_EnablePedestrianAlignmentTxnStructType *pz_Txn
);

/**
@brief Engine function for enabling magnetic field data reporting.

SLIM core implementation for enabling magnetic field data reporting.
Engine forwards the reporting request to currently active
sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreEnableMagneticFieldDataRequest
(
  const slim_EnableMagneticFieldDataTxnStructType *pz_Txn
);

/**
@brief Engine function for getting provider time.

SLIM core implementation for getting provider time.
Engine forwards the time request to the requested sensor service provider.

@param  pz_Txn: Pointer to the request transaction data.
*/
void slim_CoreGetProviderTimeRequest
(
  const slim_GetProviderTimeRequestTxnStructType *pz_Txn
);

/**
@brief Engine function for getting provider time.

SLIM core implementation for getting provider time.
Engine forwards the time request to the requested sensor service provider.

@param  e_Provider: Service provider
@param  e_Service: Sensor service
@param  t_ReferenceTime: Reference time for the request.
*/
void slim_CoreUpdateProviderTime
(
  slim_ProviderEnumType e_Provider,
  slimServiceEnumT e_Service,
  uint64 t_ReferenceTime
);

/**
@brief Notify function for provider configuration change.

SLIM providers should use this function to notify SLIM core when there has been
changes in the supported services by the provider.

@param  e_Provider : Sensor service provider the notification is originated from.
@param  q_ServiceMask : Services that were changed.
*/
void slim_CoreNotifyConfigurationChange
(
  slim_ProviderEnumType e_Provider,
  slimAvailableServiceMaskT q_ServiceMask
);

/**
@brief Notify function for request responses

SLIM providers should use this function to notify of request results. This
function should be used when the response has no payload.

@param  l_ServiceTxnId: SLIM service transaction id.
@param  e_Provider : Sensor service provider the response is originated from.
@param  e_MsgError : SLIM error code
*/
void slim_CoreNotifyResponseNoPayload
(
  int32 l_ServiceTxnId,
  slim_ProviderEnumType e_Provider,
  slimErrorEnumT e_MsgError
);

/**
@brief Notify function for request responses

SLIM providers should use this function to notify of request results.

@param  l_ServiceTxnId: SLIM service transaction id.
@param  e_Provider : Sensor service provider the response is originated from.
@param  e_MsgError : SLIM error code
@param  w_DataSize : Size of the payload.
@param  p_Data : Pointer to the payload.
*/
void slim_CoreNotifyResponse
(
  int32 l_ServiceTxnId,
  slim_ProviderEnumType e_Provider,
  slimErrorEnumT e_MsgError,
  uint16 w_DataSize,
  void *p_Data
);

/**
@brief Notify function for time request response

SLIM providers should use this function to notify of time request results.

@param  l_ServiceTxnId: SLIM service transaction id.
@param  e_Provider: Sensor data provider sending the time data.
@param  e_MsgError : SLIM error code
@param  q_RemoteRxTimeMsec: Remote receive time.
@param  q_RemoteTxTimeMsec: Remote transmit time.
*/
void slim_CoreNotifyTimeResponse
(
  int32 l_ServiceTxnId,
  slim_ProviderEnumType e_Provider,
  slimErrorEnumT e_MsgError,
  uint32 q_RemoteRxTimeMsec,
  uint32 q_RemoteTxTimeMsec
);

/**
@brief Function for dispatching sensor data.

SLIM providers should use this function to send sensor data indications
to SLIM clients.

@param  p_Handle: Pointer to the client handle.
@param  e_Provider: Sensor data provider.
@param  e_Service: SLIM service.
@param  e_Error: SLIM error.
@param  e_MessageId: SLIM message id.
@param  w_DataSize : Size of the payload.
@param  p_Data : Pointer to the payload.
*/
void slim_CoreDispatchData
(
  slimClientHandleT p_Handle,
  slim_ProviderEnumType e_Provider,
  slimServiceEnumT e_Service,
  slimErrorEnumT e_Error,
  slimMessageIdEnumT e_MessageId,
  uint16 w_DataSize,
  void *p_Data
);

/**
@brief Provides client handle for a pending transaction id.

Function provides client handle for a pending transaction id (if transaction
is found).

@param  l_ServiceTxnId: Transaction id.
@param  pp_Handle: Pointer to the SLIM client handle is stored to this variable
if the function succeeds.
@return TRUE if the transaction is found.
*/
boolean slim_CoreClientHandleForTransactionId
(
  int32 l_ServiceTxnId,
  slimClientHandleT *pp_Handle
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SLIM_CORE_H */
