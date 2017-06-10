/*============================================================================
  FILE:          SlimSensor1ProviderWrapper.cpp

  OVERVIEW:      Implementation of wrapper for Sensor1 provider handling of
                 sensor data requests.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/* LIBC header control macros. */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <log_util.h>

#include "SlimSensor1ProviderWrapper.h"
#include "slim_service_status.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#undef LOG_TAG
#define LOG_TAG "Slim_Sensor1ProviderWrapper"

/* Total service count supported by Sensor1 */
#define SLIM_SENSOR1_SERVICE_COUNT 9

/* Supported services by Sensor1 */
#define SLIM_SENSOR1_SERVICE_MASK ((1 <<  eSLIM_SERVICE_SENSOR_ACCEL) |\
                                   (1 <<  eSLIM_SERVICE_SENSOR_ACCEL_TEMP) |\
                                   (1 <<  eSLIM_SERVICE_SENSOR_GYRO) |\
                                   (1 <<  eSLIM_SERVICE_SENSOR_GYRO_TEMP) |\
                                   (1 <<  eSLIM_SERVICE_MOTION_DATA) |\
                                   (1 <<  eSLIM_SERVICE_PEDOMETER) |\
                                   (1 <<  eSLIM_SERVICE_SENSOR_MAG_CALIB) |\
                                   (1 <<  eSLIM_SERVICE_SENSOR_MAG_UNCALIB) |\
                                   (1 <<  eSLIM_SERVICE_SENSOR_BARO))

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*! Struct for SLIM provider data (Sensor1) */
typedef struct slim_Sensor1ProviderStructType
{
  /*! Provider interface function table */
  const slim_ServiceProviderInterfaceStructType *pz_Vtbl;

  /*! Service status keeps track of service state. */
  slim_ServiceStatusStructType z_ServiceStatus[SLIM_SENSOR1_SERVICE_COUNT];
  slim_ProviderStatusStructType z_ProviderStatus;

} slim_Sensor1ProviderStructType;

/*! Data for SENSOR1 provider */
static slim_Sensor1ProviderStructType z_Sensor1Data;

/*! Provider implementation */
static const slim_ServiceProviderInterfaceStructType sensor1Provider =
{
    FALSE,
    slim_Sensor1SupportsService,
    slim_Sensor1OpenClientConnection,
    slim_Sensor1CloseClientConnection,
    slim_Sensor1GetMtOffsetForService,
    slim_Sensor1SetMtOffsetForService,
    slim_Sensor1HandleProviderMessage,
    slim_Sensor1GetTimeUpdate,
    slim_Sensor1EnableSensorDataRequest,
    slim_Sensor1EnableMotionDataRequest,
    slim_Sensor1EnablePedometerRequest,
    NULL /* No support for QMD request */,
    NULL /* No support for SMD request */,
    NULL /* No support for distance bound request */,
    NULL /* No support for distance bound request */,
    NULL /* No support for distance bound request */,
    NULL /* No support for vehicle data */,
    NULL /* No support for pedestrian alignment data */,
    NULL /* No support for magnetic field data */
};


/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/
/**
@brief Updates client registration status and notifies SLIM core of the
successfull enable request.

Function updates client registration status and notifies SLIM core of the
successfull enable request.

@param  pz_Sensor1Data: Pointer to the Sensor1 provider data structure.
@param  p_Handle: SLIM client handle.
@param  b_Enable: TRUE if service was enabled.
@param  w_BatchRateHz: Used batch rate (for device sensor data).
@param  w_SampleCount: Used sample count (for device sensor data).
@param  w_ClientBatchRateHz: Client batch rate (for device sensor data).
@param  w_ClientSampleCount: Client sample count (for device sensor data).
@param  l_ServiceTxnId: Service transaction id.
@param  e_Service: SLIM service.
*/
static void NotifySuccesfullEnableRequestForRate
(
  slim_Sensor1ProviderStructType *pz_Sensor1Data,
  slimClientHandleT p_Handle,
  boolean b_Enable,
  uint16 w_BatchRateHz,
  uint16 w_SampleCount,
  uint16 w_ClientBatchRateHz,
  uint16 w_ClientSampleCount,
  int32 l_ServiceTxnId,
  slimServiceEnumT e_Service
)
{
  /* Set new service status */
  slim_ServiceStatusEnableForRate(
    &pz_Sensor1Data->z_ProviderStatus,
    e_Service,
    b_Enable,
    w_BatchRateHz,
    w_SampleCount,
    w_ClientBatchRateHz,
    w_ClientSampleCount,
    p_Handle);

  /* Send response to client */
  slim_ProviderRouteGenericResponse(
    l_ServiceTxnId,
    SLIM_PROVIDER_SENSOR1,
    eSLIM_SUCCESS);
}

/**
@brief Updates client registration status and notifies SLIM core of the
successfull enable request.

Function updates client registration status and notifies SLIM core of the
successfull enable request.

@param  pz_Sensor1Data: Pointer to the Sensor1 provider data structure.
@param  p_Handle: SLIM client handle.
@param  b_Enable: TRUE if service was enabled.
@param  l_ServiceTxnId: Service transaction id.
@param  e_Service: SLIM service.
*/
static void NotifySuccesfullEnableRequest
(
  slim_Sensor1ProviderStructType *pz_Sensor1Data,
  slimClientHandleT p_Handle,
  boolean b_Enable,
  int32 l_ServiceTxnId,
  slimServiceEnumT e_Service
)
{
  /* Set new service status */
  slim_ServiceStatusEnable(
    &pz_Sensor1Data->z_ProviderStatus,
    e_Service,
    b_Enable,
    p_Handle);

  /* Send response to client */
    slim_ProviderRouteGenericResponse(
    l_ServiceTxnId,
    SLIM_PROVIDER_SENSOR1,
    eSLIM_SUCCESS);
}

/**
@brief Opens provider connection for specific client.

Function opens provider connection for a specific client.

@param  pz_Provider : Provider object
@param  p_Handle : Pointer to the client handle.
@return eSLIM_SUCCESS if connection is opened successfully.
*/
slimErrorEnumT slim_Sensor1OpenClientConnection(
    slim_ServiceProviderInterfaceType *pz_Provider,
    slimClientHandleT p_Handle)
{
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;

    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);

    slim_ServiceStatusAddClient(&pz_Sensor1Data->z_ProviderStatus, p_Handle);

    return eSLIM_SUCCESS;
}

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
    slimClientHandleT p_Handle)
{
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;

    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);

    slim_ServiceStatusRemoveClient(&pz_Sensor1Data->z_ProviderStatus, p_Handle);

    return eSLIM_SUCCESS;
}

/**
@brief Initializes Sensor1 provider module.

Function initializes Sensor1 provider module.

@return Pointer to the Sensor1 provider implementation.
*/
void slim_Sensor1Initialize(slim_ServiceProviderInterfaceType **pz_Provider)
{
    ENTRY_LOG();
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;

    memset(pz_Sensor1Data, 0, sizeof(*pz_Sensor1Data));
    pz_Sensor1Data->pz_Vtbl = &sensor1Provider;
    *pz_Provider = &pz_Sensor1Data->pz_Vtbl;

    /* Initialize service states */
    pz_Sensor1Data->z_ProviderStatus.pz_StatusArray = pz_Sensor1Data->z_ServiceStatus;
    pz_Sensor1Data->z_ProviderStatus.q_ServiceCount = SLIM_SENSOR1_SERVICE_COUNT;
    slim_ServiceStatusInitialize(
            &pz_Sensor1Data->z_ProviderStatus,
            SLIM_SENSOR1_SERVICE_COUNT,
            SLIM_SENSOR1_SERVICE_MASK);

    slim_sensor1_init();
}

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
)
{
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;
    SlimDaemonMessage z_Message;

    SLIM_UNUSED(pz_Provider);

    slim_Memscpy(&z_Message, sizeof(z_Message), p_MsgPayload, q_MsgPayloadSize);

    switch (q_MessageId)
    {
    case SLIM_DAEMON_SENSOR_DATA_INJECT:
    case SLIM_DAEMON_PEDOMETER_INJECT:
    case SLIM_DAEMON_MOTION_DATA_INJECT:
        slim_ProviderDispatchIndication(
                &pz_Sensor1Data->z_ProviderStatus, &z_Message);
        break;
    case SLIM_DAEMON_GENERIC_SERVICE_RESPONSE:
        slim_ProviderDispatchGenericResponse(&z_Message);
        break;
    case SLIM_DAEMON_TIME_SYNC_DATA_INJECT:
        slim_ProviderDispatchTimeResponse(&z_Message);
        break;
    default:
        LOC_LOGE("%s:%d]: Received unsupported message %" PRIu32 ".\n",
                 __func__, __LINE__, (uint32_t)q_MessageId);
        break;
    }
}

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
    int32 l_ServiceTxnId)
{
    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);

    slim_sensor1_get_sensor_time(l_ServiceTxnId);

    return eSLIM_SUCCESS;
}

/**
@brief Initiates sensor data request.

Function initiates sensor data request to SSC. If request is successfull,
SSC sensor data streaming is either started or stopped.

@param  pz_Provider : Provider object
@param  pz_Txn : Pointer to the transaction data.
@param  l_ServiceTxnId : Service transaction id.
@return eSLIM_SUCCESS if sensor data request is made successfully.
*/
slimErrorEnumT slim_Sensor1EnableSensorDataRequest(
    slim_ServiceProviderInterfaceType *pz_Provider,
    const slim_EnableSensorDataTxnStructType *pz_Txn, int32 l_ServiceTxnId)
{
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;
    slimErrorEnumT e_RetVal = eSLIM_SUCCESS;
    boolean b_SendRequest = FALSE;
    uint16 w_ReportRateHz = 0;
    uint16 w_SampleCount = 0;
    uint16 w_ClientReportRateHz = 0;
    uint16 w_ClientSampleCount = 0;
    boolean b_SendEnableRequest = pz_Txn->z_Request.enableConf.enable;
    boolean b_Sensor1Success = FALSE;

    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);

    if (pz_Txn->z_Request.enableConf.enable)
    {
      w_ReportRateHz = pz_Txn->z_Request.reportRate;
      w_SampleCount = pz_Txn->z_Request.sampleCount;
      w_ClientReportRateHz = pz_Txn->z_Request.reportRate;
      w_ClientSampleCount = pz_Txn->z_Request.sampleCount;
    }

    /* Always the highest requested rate */
    slim_ServiceStatusGetSampleRate(
      &pz_Sensor1Data->z_ProviderStatus,
      pz_Txn->z_Request.sensor,
      pz_Txn->z_TxnData.p_Handle,
      &w_ReportRateHz,
      &w_SampleCount);

    /* Check if request is valid for this client. */
    e_RetVal = slim_ServiceStatusCanEnableForRate(
      &pz_Sensor1Data->z_ProviderStatus,
      pz_Txn->z_TxnData.p_Handle,
      pz_Txn->z_Request.sensor,
      pz_Txn->z_Request.enableConf.enable,
      w_ReportRateHz,
      w_SampleCount,
      &b_SendRequest,
      &b_SendEnableRequest);

    if (eSLIM_SUCCESS == e_RetVal)
    {
        /* Send request if it is not already sent. */
        if (b_SendRequest)
        {
            b_Sensor1Success = slim_sensor1_update_sensor_status_buffering(
                    pz_Txn->z_Request.enableConf.enable,
                    w_SampleCount,
                    w_ReportRateHz,
                    pz_Txn->z_Request.sensor);
        }
        else
        {
            b_Sensor1Success = TRUE;
        }

        if (b_Sensor1Success)
        {
        /* Error is notified by SLIM core so notify only success to client here.
        */
            NotifySuccesfullEnableRequestForRate(
              pz_Sensor1Data,
              pz_Txn->z_TxnData.p_Handle,
              pz_Txn->z_Request.enableConf.enable,
              w_ReportRateHz,
              w_SampleCount,
              w_ClientReportRateHz,
              w_ClientSampleCount,
              l_ServiceTxnId,
              pz_Txn->z_Request.sensor);
        }
        else
        {
            LOC_LOGE("%s:%d]: Failed enabling sensor service %d.\n",
                     __func__, __LINE__, pz_Txn->z_Request.sensor);
            e_RetVal = eSLIM_ERROR_INTERNAL;
        }
    }

    return e_RetVal;
}

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
    const slim_EnablePedometerTxnStructType *pz_Txn, int32 l_ServiceTxnId)
{
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;
    slimErrorEnumT e_RetVal = eSLIM_SUCCESS;
    boolean b_SendRequest = FALSE;
    boolean b_Sensor1Success = FALSE;

    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);

    /* Check if request is valid for this client. */
    e_RetVal = slim_ServiceStatusCanEnable(
      &pz_Sensor1Data->z_ProviderStatus,
                                     pz_Txn->z_TxnData.p_Handle,
      eSLIM_SERVICE_PEDOMETER,
      pz_Txn->z_Request.enableConf.enable,
      &b_SendRequest);

    if (eSLIM_SUCCESS == e_RetVal)
    {
      if (b_SendRequest)
        {
          uint8_t u_ResetStepCount = pz_Txn->z_Request.resetStepCount_valid ?
                  pz_Txn->z_Request.resetStepCount : (uint8_t)FALSE;
          uint32_t q_StepCountThreshold = pz_Txn->z_Request.stepCountThreshold_valid ?
                  pz_Txn->z_Request.stepCountThreshold : 0;
          b_Sensor1Success = slim_sensor1_update_pedometer(
                  pz_Txn->z_Request.enableConf.enable,
                  u_ResetStepCount,
                  q_StepCountThreshold);
        }
        else
        {
          b_Sensor1Success = TRUE;
        }

      if (b_Sensor1Success)
      {
        /* Error is notified by SLIM core so notify only success to client here.
        */
          NotifySuccesfullEnableRequest(
            pz_Sensor1Data,
                                      pz_Txn->z_TxnData.p_Handle,
                                      pz_Txn->z_Request.enableConf.enable,
                                      l_ServiceTxnId,
                                      eSLIM_SERVICE_PEDOMETER);
    }
      else
      {
          LOC_LOGE("%s:%d]: Failed enabling pedometer data.\n",
                   __func__, __LINE__);
          e_RetVal = eSLIM_ERROR_INTERNAL;
      }
    }
    return e_RetVal;
}


/**
@brief Initiates motion data request to SSC.

Function initiates motion data request to SSC. If request is successfull,
SSC motion data streaming is either started or stopped.

@param  pz_Provider : Provider object
@param  pz_Txn: Pointer to motion data request transaction data.
@param  l_ServiceTxnId : Service transaction id.
@return SSC_SERVICE_SUCCESS if request is done successfully. Otherwise SSC error
code.
*/
slimErrorEnumT slim_Sensor1EnableMotionDataRequest
(
  slim_ServiceProviderInterfaceType *pz_Provider,
  const slim_EnableMotionDataTxnStructType *pz_Txn,
  int32 l_ServiceTxnId
)
{
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;
    slimErrorEnumT e_RetVal = eSLIM_SUCCESS;
    boolean b_SendRequest = FALSE;
    boolean b_Sensor1Success = FALSE;

    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);

    /* Check if request is valid for this client. */
    e_RetVal = slim_ServiceStatusCanEnable(
      &pz_Sensor1Data->z_ProviderStatus,
                                     pz_Txn->z_TxnData.p_Handle,
      eSLIM_SERVICE_MOTION_DATA,
      pz_Txn->z_Request.enableConf.enable,
      &b_SendRequest);

    if (eSLIM_SUCCESS == e_RetVal)
    {
      if (b_SendRequest)
        {
          b_Sensor1Success =
                  slim_sensor1_update_motion_data(pz_Txn->z_Request.enableConf.enable);
        }
        else
        {
          b_Sensor1Success = TRUE;
        }

      if (b_Sensor1Success)
      {
        /* Error is notified by SLIM core so notify only success to client here.
        */
          NotifySuccesfullEnableRequest(
            pz_Sensor1Data,
                                      pz_Txn->z_TxnData.p_Handle,
                                      pz_Txn->z_Request.enableConf.enable,
                                      l_ServiceTxnId,
                                      eSLIM_SERVICE_MOTION_DATA);
    }
      else
      {
          LOC_LOGE("%s:%d]: Failed enabling motion data.\n",
                   __func__, __LINE__);
          e_RetVal = eSLIM_ERROR_INTERNAL;
      }
    }
    return e_RetVal;
}

/**
@brief Returns TRUE if service is supported by the provider.

Function returns TRUE if service is supported by the provider.

@param  pz_Provider : Provider object
@param  e_Service : SLIM service
@return TRUE if service is supported. FALSE otherwise.
*/
boolean slim_Sensor1SupportsService
(
  slim_ServiceProviderInterfaceType *pz_Provider,
  slimServiceEnumT e_Service
)
{
    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);

    return SLIM_MASK_IS_SET(SLIM_SENSOR1_SERVICE_MASK, e_Service) ? TRUE : FALSE;
}

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
  slimClientHandleT p_Handle,
  slimServiceEnumT e_Service,
  slim_ServiceMtDataStructType *pz_MtOffset
)
{
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;
    boolean b_Found = FALSE;

    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);
    SLIM_UNUSED(p_Handle);

    if (NULL != pz_MtOffset)
    {
      b_Found = slim_ServiceStatusGetMtOffset(
        &pz_Sensor1Data->z_ProviderStatus, e_Service, pz_MtOffset);
    }

    return b_Found;
}

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
  slimClientHandleT p_Handle,
  slimServiceEnumT e_Service,
  const slim_ServiceMtDataStructType *pz_MtOffset,
  boolean b_SetForAllClients
)
{
    slim_Sensor1ProviderStructType *pz_Sensor1Data = &z_Sensor1Data;

    ENTRY_LOG();

    SLIM_UNUSED(pz_Provider);
    SLIM_UNUSED(p_Handle);
    SLIM_UNUSED(b_SetForAllClients);

    if (NULL != pz_MtOffset)
    {
      slim_ServiceStatusSetMtOffset(
        &pz_Sensor1Data->z_ProviderStatus, e_Service, pz_MtOffset);
    }
}

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
  SlimDaemonMessage* pSlimDaemonMessage
)
{
    ENTRY_LOG();

    slim_ProviderRouteIndication(
            SLIM_PROVIDER_SENSOR1,
            e_Service,
            e_Error,
            e_MessageId,
            pSlimDaemonMessage);
}

