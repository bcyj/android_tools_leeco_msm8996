#ifndef SLIM_SERVICE_STATUS_H
#define SLIM_SERVICE_STATUS_H
/*============================================================================
  @file slim_service_status.h

  This module is an utility tool for SLIM providers to keep track of service
  status.

               Copyright (c) 2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/core/inc/slim_service_status.h#2 $ */


#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "slim_provider_data.h"
#include "slim_utils.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
/*! Enumeration for services that support multiplexing and define a rate in the
reporting request.
*/
typedef enum
{
  eSLIM_SERVICE_STATUS_RATE_NONE = -1,
  /**< Invalid */

  eSLIM_SERVICE_STATUS_RATE_ACCEL = 0,
  /**< Accelerometer */
  eSLIM_SERVICE_STATUS_RATE_ACCEL_TEMP,
  /**< Accelerometer temperature */
  eSLIM_SERVICE_STATUS_RATE_GYRO,
  /**< Gyro */
  eSLIM_SERVICE_STATUS_RATE_GYRO_TEMP,
  /**< Gyro temperature */
  eSLIM_SERVICE_STATUS_RATE_MAG_CALIB,
  /**< Calibrated magnetometer */
  eSLIM_SERVICE_STATUS_RATE_MAG_UNCALIB,
  /**< Uncalibrated magnetometer */
  eSLIM_SERVICE_STATUS_RATE_BARO,
  /**< Barometer */

  eSLIM_SERVICE_STATUS_RATE_COUNT,
  /**< Service count */

  eSLIM_SERVICE_STATUS_RATE_MAX = 2147483647 /* Force 32bit */
} slim_ServiceStatusRateEnumType;

/*! Struct for service rate data */
typedef struct
{
  uint16_t w_SampleCount;
  /**< Samples per batch */

  uint16_t w_ReportRate;
  /**< Batches per second */
} slim_ServiceStatusRateStructType;

/*! Struct for service status data */
typedef struct slim_ServiceStatusStructType
{
  slimServiceEnumT e_Service;              /**< SLIM Service */
  int8 b_ClientCount;                      /**< Current client count for the service. */
  slim_ServiceMtDataStructType z_MtOffset; /**< Mt offset for service time-sync. */

  /* These configuration parameters are used only by device sensor data */
  uint16 w_ReportRateHz;                  /**< Current configuration for sample rate */
  uint16 w_SampleCount;                   /**< Current configuration for batch count */

} slim_ServiceStatusStructType;

/*! Struct for provider client data */
typedef struct slim_ProviderClientStatusStructType
{
  slimClientHandleT p_Handle;                     /**< Client handle */
  boolean b_Open;                                 /**< TRUE if connection is open. */
  slimAvailableServiceMaskT q_EnabledServiceMask; /**< Mask of enabled services. */

  slim_ServiceStatusRateStructType z_ServiceRates[eSLIM_SERVICE_STATUS_RATE_COUNT];
  /**< Service rates requested by client. */

} slim_ProviderClientStatusStructType;

/*! Struct for provider service status data */
typedef struct slim_ProviderStatusStructType
{
  slim_ServiceStatusStructType *pz_StatusArray; /**< Pointer to the array of status objects. */
  uint32 q_ServiceCount;                        /**< Count of the status array objects. */
  slim_ProviderClientStatusStructType z_ClientData[SLIM_CLIENT_MAX_COUNT]; /**< Client status. */

} slim_ProviderStatusStructType;

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/
/**
@brief Initializes provider service status struct.

Function initializes provider service status struct.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  q_ServiceCount: Count of services supported by the provider.
@param  q_SupportedServiceMask: Mask of supported services.
*/
void slim_ServiceStatusInitialize
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  uint32 q_ServiceCount,
  slimAvailableServiceMaskT q_SupportedServiceMask
);

/**
@brief Adds client to the provider status client registry.

Function adds client to the provider status client registry. Should be called
when SLIM client opens a provider connection.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  p_Handle: SLIM client handle.
*/
void slim_ServiceStatusAddClient
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  slimClientHandleT p_Handle
);

/**
@brief Removes client from the provider status client registry.

Function removes client from the provider status client registry. Should be called
when SLIM client closes a provider connection.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  p_Handle: SLIM client handle.
*/
void slim_ServiceStatusRemoveClient
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  slimClientHandleT p_Handle
);

/**
@brief Enables service for the given client.

Function sets service enabled for the given client.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  e_Service: SLIM service.
@param  b_Enable: TRUE if service was enabled.
@param  p_Handle: SLIM client handle.
*/
void slim_ServiceStatusEnable
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  slimServiceEnumT e_Service,
  boolean b_Enable,
  slimClientHandleT p_Handle
);

/**
@brief Enables service for the given client.

Function sets service enabled for the given client. Stores also service
reporting rates.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  e_Service: SLIM service.
@param  b_Enable: TRUE if service was enabled.
@param  w_BatchRateHz: Used batch rate (for device sensor data).
@param  w_SampleCount: Used sample count (for device sensor data).
@param  w_ClientBatchRateHz: Client batch rate (for device sensor data).
@param  w_ClientSampleCount: Client sample count (for device sensor data).
@param  p_Handle: SLIM client handle.
*/
void slim_ServiceStatusEnableForRate
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  slimServiceEnumT e_Service,
  boolean b_Enable,
  uint16 w_BatchRateHz,
  uint16 w_SampleCount,
  uint16 w_ClientBatchRateHz,
  uint16 w_ClientSampleCount,
  slimClientHandleT p_Handle
);

/**
@brief Checks if service is enabled currently for the given client.

Function checks if service is enabled currently for the given client.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  p_Handle: SLIM client handle.
@param  e_Service: SLIM service.
@return TRUE if service is enabled for the given client. Otherwise FALSE.
*/
boolean slim_ServiceStatusIsEnabled
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  slimClientHandleT p_Handle,
  slimServiceEnumT e_Service
);

/**
@brief Checks if requested service can be enabled or disabled for the given client.

Function checks if requested service can be enabled or disabled for the given client.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  p_Handle: SLIM client handle.
@param  e_Service: SLIM service.
@param  b_Enable: TRUE, if service should be enabled.
@param  pb_SendRequest: To this variable is stored whether the request should be
sent to the remote provider or not.
@return eSLIM_SUCCESS if enable request can be done. Otherwise SLIM error code.
*/
slimErrorEnumT slim_ServiceStatusCanEnable
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  slimClientHandleT p_Handle,
  slimServiceEnumT e_Service,
  boolean b_Enable,
  boolean *pb_SendRequest
);

/**
@brief Checks if requested service can be enabled or disabled for the given client.

Function checks if requested service can be enabled or disabled for the given client.
Checks also the if the given reporting rate exceeds the current one.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  p_Handle: SLIM client handle.
@param  e_Service: SLIM service.
@param  b_Enable: TRUE, if service should be enabled.
@param  w_BatchRateHz: Requested batch rate (for device sensor data).
@param  w_SampleCount: Requested sample count (for device sensor data).
@param  pb_SendRequest: To this variable is stored whether the request should be
sent to the remote provider or not.
@param  pb_SendEnableRequest: To this variable is stored whether the request
should be sent as an enable request to the remote provider or not.
@return eSLIM_SUCCESS if enable request can be done. Otherwise SLIM error code.
*/
slimErrorEnumT slim_ServiceStatusCanEnableForRate
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  slimClientHandleT p_Handle,
  slimServiceEnumT e_Service,
  boolean b_Enable,
  uint16 w_BatchRateHz,
  uint16 w_SampleCount,
  boolean *pb_SendRequest,
  boolean *pb_SendEnableRequest
);

/**
@brief Adjusts sample rate and batch count for the given service.

Function adjusts sample rate and batch count for the given service.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  e_Service: SLIM service.
@param  p_Handle: SLIM client handle.
@param  pw_ReportRateHz: New sample rate.
@param  pw_SampleCount: New batch count.
*/
void slim_ServiceStatusGetSampleRate
(
  const slim_ProviderStatusStructType *pz_ProviderStatus,
  slimServiceEnumT e_Service,
  slimClientHandleT p_Handle,
  uint16 *pw_ReportRateHz,
  uint16 *pw_SampleCount
);

/**
@brief Provides timesync mt offset for the given SLIM service.

Function provides timesync mt offset for the given SLIM service.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  e_Service : SLIM service.
@param  pz_MtOffset : Pointer to mt offset. Data is stored to this variable if
found.
@return TRUE if mt data was found. FALSE otherwise.
*/
boolean slim_ServiceStatusGetMtOffset
(
  const slim_ProviderStatusStructType *pz_ProviderStatus,
  slimServiceEnumT e_Service,
  slim_ServiceMtDataStructType *pz_MtOffset
);

/**
@brief Sets timesync mt offset for the given SLIM service.

Function sets timesync mt offset for the given SLIM service.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  e_Service : SLIM service. If eSLIM_SERVICE_NONE, data is applied to all
services.
@param  pz_MtOffset : Pointer to mt offset to be set.
*/
void slim_ServiceStatusSetMtOffset
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  slimServiceEnumT e_Service,
  const slim_ServiceMtDataStructType *pz_MtOffset
);

/**
@brief Dispatches data indication to all registered clients.

Function dispatches data indication to all registered clients.

@param  pz_ProviderStatus: Pointer to the provider status.
@param  e_Provider: SLIM provider.
@param  e_Service: SLIM service.
@param  e_Error: SLIM error.
@param  e_MessageId: SLIM message id.
@param  w_DataSize : Size of the payload.
@param  p_Data : Pointer to the payload.
*/
void slim_ServiceStatusDispatchDataToRegistered
(
  const slim_ProviderStatusStructType *pz_ProviderStatus,
  slim_ProviderEnumType e_Provider,
  slimServiceEnumT e_Service,
  slimErrorEnumT e_Error,
  slimMessageIdEnumT e_MessageId,
  uint16 w_DataSize,
  void *p_Data
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SLIM_SERVICE_STATUS_H */
