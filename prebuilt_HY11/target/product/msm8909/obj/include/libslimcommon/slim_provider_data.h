#ifndef SLIM_PROVIDER_DATA_H
#define SLIM_PROVIDER_DATA_H
/*============================================================================
  @file slim_provider_data.h

  Structures and definitions to store provider data.

               Copyright (c) 2013-2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/core/inc/slim_provider_data.h#9 $ */


#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "slim_utils.h"
#include "slim_timesync.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*! Function table for SLIM Service Provider object */
typedef struct slim_ServiceProviderInterfaceStruct slim_ServiceProviderInterfaceStructType;
/*! SLIM service provider is an object with function table reference as first
    member*/
typedef const slim_ServiceProviderInterfaceStructType *const slim_ServiceProviderInterfaceType;

/** Enum for provider setting. Preferred provider can be set from APSS using
    QMI_LOC. If preferred provider is native, SSC should not be used.
*/
typedef enum
{
  SLIM_PROVIDER_SETTING_NONE = -1,
  SLIM_PROVIDER_SETTING_SSC  = 0, /**< Sensors data provider is Snapdragon Sensor
                                       Core (SSC). This is the default. */
  SLIM_PROVIDER_SETTING_NATIVE,   /**< Sensors data provider is on the host
                                       processor. */
  SLIM_PROVIDER_SETTING_COUNT,
  SLIM_PROVIDER_SETTING_MAX = 2147483647 /**< To force a 32 bit signed enum. Do not change or use */
} slim_ProviderSettingEnumType;

/*! Struct for time sync service monotonous data */
typedef struct slim_ServiceMtDataStructType
{
  uint32 q_LastSampleTime; /*!< Time of latest sample */
  /* Monotonous offset.
     Offset changes at a rate of
     maximum of passed millisecond time.
     Can be used to convert from remote to
     local times so that monotonous counter in
     remote times is also monotonous in local
     times. */
  slim_TimeSyncMtOffsetStructType z_MtOffset;
  boolean b_OffsetReset; /*!< TRUE if offset has been reset */
} slim_ServiceMtDataStructType;

/* Function prototypes for sensor provider "interface" */
/*! Sensor provider prototype: query for the service support status */
typedef boolean (*slim_SupportsServiceFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  slimServiceEnumT e_Service
);

/*! Sensor provider prototype: open client connection */
typedef slimErrorEnumT (*slim_OpenClientConnectionFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  slimClientHandleT p_Handle
);

/*! Sensor provider prototype: close client connection */
typedef slimErrorEnumT (*slim_CloseClientConnectionFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  slimClientHandleT p_Handle
);

/*! Sensor provider prototype: mt offset getter */
typedef boolean (*slim_GetMtOffsetForService)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  slimClientHandleT p_Handle,
  slimServiceEnumT e_Service,
  slim_ServiceMtDataStructType *pz_MtOffset
);

/*! Sensor provider prototype: mt offset setter */
typedef void (*slim_SetMtOffsetForService)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  slimClientHandleT p_Handle,
  slimServiceEnumT e_Service,
  const slim_ServiceMtDataStructType *pz_MtOffset,
  boolean b_ForAllClients
);

/*! Sensor provider prototype: provider IPC message handler */
typedef void (*slim_HandleProviderMessage)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  uint32 q_MessageId,
  uint32 q_MsgPayloadSize,
  const void *p_MsgPayload
);

/*! Sensor provider prototype: get time request */
typedef slimErrorEnumT (*slim_GetTimeRequestFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable sensor reporting */
typedef slimErrorEnumT (*slim_EnableSensorDataFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnableSensorDataTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable motion data reporting */
typedef slimErrorEnumT (*slim_EnableMotionDataFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnableMotionDataTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable pedometer reporting */
typedef slimErrorEnumT (*slim_EnablePedometerFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnablePedometerTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable QMD data reporting */
typedef slimErrorEnumT (*slim_EnableQmdDataFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnableQmdDataTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable SMD data reporting */
typedef slimErrorEnumT (*slim_EnableSmdDataFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnableSmdDataTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable distance bound reporting */
typedef slimErrorEnumT (*slim_EnableDistanceBoundFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnableDistanceBoundTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: set distance bound */
typedef slimErrorEnumT (*slim_SetDistanceBoundFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_SetDistanceBoundTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: get distance bound */
typedef slimErrorEnumT (*slim_GetDistanceBoundFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_ClientTxnDataStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable vehicle data reporting */
typedef slimErrorEnumT (*slim_EnableVehicleDataFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnableVehicleDataTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable pedestrian alignment data reporting */
typedef slimErrorEnumT (*slim_EnablePedestrianAlignmentFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnablePedestrianAlignmentTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Sensor provider prototype: enable magnetic field data reporting */
typedef slimErrorEnumT (*slim_EnableMagneticFieldDataFunctionType)
(
  slim_ServiceProviderInterfaceType *pz_Interface,
  const slim_EnableMagneticFieldDataTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
);

/*! Struct for sensor provider "interface" to be implemented by
    provider modules */
struct slim_ServiceProviderInterfaceStruct
{
  boolean b_TimeCommonWithSlim;
  /**< If TRUE, provider has common time with SLIM and no time sync is needed. */

  slim_SupportsServiceFunctionType fn_SupportsService;
  /**< Returns TRUE, if provider supports the specified service. */

  slim_OpenClientConnectionFunctionType fn_OpenClientConnection;
  /**< Called when new SLIM client opens a connection. */

  slim_CloseClientConnectionFunctionType fn_CloseClientConnection;
  /**< Called when existing SLIM client closes a connection. */

  slim_GetMtOffsetForService fn_GetMtOffset;
  /**< Called when SLIM core needs a monotonous offset for the specified service. */

  slim_SetMtOffsetForService fn_SetMtOffset;
  /**< Called when SLIM core set a monotonous offset for the specified service. */

  slim_HandleProviderMessage fn_HandleMessage;
  /**< Called when SLIM core gets a message targeted for provider. */

  slim_GetTimeRequestFunctionType fn_GetTimeRequest;
  /**< Get request for provider time. */

  slim_EnableSensorDataFunctionType fn_EnableSensorDataRequest;
  /**< Enable request for device sensor data. */

  slim_EnableMotionDataFunctionType fn_EnableMotionDataRequest;
  /**< Enable request for motion data. */

  slim_EnablePedometerFunctionType fn_EnablePedometerRequest;
  /**< Enable request for pedometer data. */

  slim_EnableQmdDataFunctionType fn_EnableQmdDataRequest;
  /**< Enable request for AMD/RMD data. */

  slim_EnableSmdDataFunctionType fn_EnableSmdDataRequest;
  /**< Enable request for SMD data. */

  slim_EnableDistanceBoundFunctionType fn_EnableDistanceBoundRequest;
  /**< Enable request for distance bound data. */

  slim_SetDistanceBoundFunctionType fn_SetDistanceBoundRequest;
  /**< Set request for distance bound data. */

  slim_GetDistanceBoundFunctionType fn_GetDistanceBoundRequest;
  /**< Get request for distance bound data. */

  slim_EnableVehicleDataFunctionType fn_EnableVehicleDataRequest;
  /**< Enable request for vehicle data. */

  slim_EnablePedestrianAlignmentFunctionType fn_EnablePedestrianAlignmentRequest;
  /**< Enable request for pedestrian alignment data. */

  slim_EnableMagneticFieldDataFunctionType fn_EnableMagneticFieldDataRequest;
  /**< Enable request for magnetic field data. */
};

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SLIM_PROVIDER_DATA_H */
