#ifndef SLIM_UTILS_H
#define SLIM_UTILS_H
/*============================================================================
  @file slim_utils.h

  Utility functions and types for different SLIM modules.

               Copyright (c) 2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/core/inc/slim_utils.h#3 $ */

#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <stringl.h>
#include "slim_internal.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
/*! Sensor service count */
#define SLIM_SERVICE_COUNT (eSLIM_SERVICE_LAST + 1)

/*! Get client id value */
#define SLIM_CLIENT_ID(p_Handle) (NULL != p_Handle ? *(int8*)p_Handle : -1)

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
/** Enum for generic SLIM requests. These requests has a similar payload
    when sending the request to IPC queue.
*/
typedef enum
{
  SLIM_REQUEST_CLOSE = 0,         /**< Close client connection */
  SLIM_REQUEST_GET,              /**< Get report by request */

  SLIM_ERROR_MAX    = 2147483647 /**< To force a 32 bit signed enum. Do not change or use */
} slim_GenericRequestEnumType;

/*! Struct for SLIM client transaction data */
typedef struct slim_ClientTxnDataStructType
{
  slimClientHandleT p_Handle; /*!< Client handle */
  uint8 u_ClientTxnId;        /*!< Transaction id provided by client */
} slim_ClientTxnDataStructType;

/*! Struct for generic SLIM API request */
typedef struct slim_GenericTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;            /*!< Transaction data */
  slim_GenericRequestEnumType e_Type;                /*!< Request type */
  slimServiceEnumT e_Service;                    /*!< Service to make the request for. */
} slim_GenericTxnStructType;

/*! Struct for SLIM API open request */
typedef struct
{
  slimClientHandleT p_Handle;              /*!< Client handle */
  slimNotifyCallbackFunctionT fn_Callback; /*!< Client callback */
  slimOpenFlagsT q_OpenFlags;              /*!< Client flags */
  uint64 t_CallbackData;                   /*!< Client callback data */
} slim_OpenTxnStructType;

/*! Struct for SLIM enable sensor data transaction */
typedef struct slim_EnableSensorDataTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;       /*!< Transaction data */
  slimEnableSensorDataRequestStructT z_Request; /*!< Sensor configuration data */
} slim_EnableSensorDataTxnStructType;

/*! Struct for SLIM enable motion data transaction */
typedef struct slim_EnableMotionDataTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;        /*!< Transaction data */
  slimEnableMotionDataRequestStructT z_Request;  /*!< Motion data configuration */
} slim_EnableMotionDataTxnStructType;

/*! Struct for SLIM enable pedometer transaction */
typedef struct slim_EnablePedometerTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;       /*!< Transaction data */
  slimEnablePedometerRequestStructT z_Request;  /*!< Pedometer configuration */
} slim_EnablePedometerTxnStructType;

/*! Struct for SLIM enable AMD/RMD data transaction */
typedef struct slim_EnableQmdDataTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;        /*!< Transaction data */
  slimEnableQmdDataRequestStructT z_Request;     /*!< QMD data configuration */
} slim_EnableQmdDataTxnStructType;

/*! Struct for SLIM enable SMD data transaction */
typedef struct slim_EnableSmdDataTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;        /*!< Transaction data */
  slimEnableSmdDataRequestStructT z_Request;     /*!< SMD data configuration */
} slim_EnableSmdDataTxnStructType;

/*! Struct for SLIM enable distance bound data transaction */
typedef struct slim_EnableDistanceBoundTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;           /*!< Transaction data */
  slimEnableDistanceBoundRequestStructT z_Request;  /*!< DB configuration */
} slim_EnableDistanceBoundTxnStructType;

/*! Struct for SLIM set distance bound data transaction */
typedef struct slim_SetDistanceBoundTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;           /*!< Transaction data */
  slimSetDistanceBoundRequestStructT z_Request;     /*!< DB configuration */
} slim_SetDistanceBoundTxnStructType;

/*! Struct for SLIM enable vehicle data transaction */
typedef struct slim_EnableVehicleDataTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;        /*!< Transaction data */
  slimEnableVehicleDataRequestStructT z_Request; /*!< Vehicle data configuration */
} slim_EnableVehicleDataTxnStructType;

/*! Struct for SLIM enable pedestrian alignment transaction */
typedef struct slim_EnablePedestrianAlignmentTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;                /*!< Transaction data */
  slimEnablePedestrianAlignmentRequestStructT z_Request; /*!< Pedestrian alignment
                                                              configuration */
} slim_EnablePedestrianAlignmentTxnStructType;

/*! Struct for SLIM enable magnetic field data transaction */
typedef struct slim_EnableMagneticFieldDataTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;               /*!< Transaction data */
  slimEnableMagneticFieldDataRequestStructT z_Request;  /*!< Magnetic field data configuration */
} slim_EnableMagneticFieldDataTxnStructType;

/*! Struct for SLIM get provider time request transaction */
typedef struct slim_GetProviderTimeRequestTxnStructType
{
  slim_ClientTxnDataStructType z_TxnData;           /*!< Transaction data */
  slimGetProviderTimeRequestStructT z_Request;      /*!< Time request */
} slim_GetProviderTimeRequestTxnStructType;

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/
/**
@brief Converts slow clock ticks to milliseconds.

Function converts given slow clock ticks value to milliseconds. Rounds the
result to the nearest integer.

@param  t_TimeInClockTicks: Slow clock value in ticks.
@return Clock value in milliseconds.
*/
uint64 slim_TimeToMilliseconds
(
  uint64 t_TimeInClockTicks
);

/**
@brief Converts slow clock ticks to milliseconds.

Function converts given slow clock ticks value to milliseconds. Rounds the
result to the floor integer.

@param  t_TimeInClockTicks: Slow clock value in ticks.
@return Clock value in milliseconds.
*/
uint64 slim_TimeToMillisecondsFloor
(
  uint64 t_TimeInClockTicks
);

/**
@brief Converts milliseconds to slow clock ticks.

Function converts given milliseconds value to slow clock ticks.

@param  t_TimeInMSecs: Clock value in milliseconds.
@return Clock value in slow clock ticks.
*/
uint32 slim_TimeToClockTicks
(
  uint64 t_TimeInMSecs
);

#ifdef __cplusplus
}
#endif

#endif /* SLIM_UTILS_H_ */
