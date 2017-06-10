/*============================================================================
@file SlimProviderCommon.h

  SLIM Provider Common file

  This file defines the implementation of common methods shared
  between sensor providers

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef SLIM_PROVIDER_COMMON_H
#define SLIM_PROVIDER_COMMON_H

#include <stdbool.h>
#include "SlimDaemonMsg.h"
#include "slim_service_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/
/**
@brief Initialize message queue

Initialize message queue

@param p_msg_q: Pointer to message queue
*/
bool slim_message_queue_init(void* p_msg_q);

/**
@brief Route function for data indications

Provider modules should use this function to route data indications through
SLIM-AP daemon task.

@param[in] e_Provider         SLIM provider.
@param[in] e_Service          SLIM service.
@param[in] e_Error            Indication error code.
@param[in] e_MessageId        Indication message id.
@param     pSlimDaemonMessage Pointer to message.
*/
void slim_ProviderRouteIndication
(
  slim_ProviderEnumType e_Provider,
  slimServiceEnumT e_Service,
  slimErrorEnumT e_Error,
  slimMessageIdEnumT e_MessageId,
  SlimDaemonMessage* pSlimDaemonMessage
);

/**
@brief Dispatch function for data indications

Sensor providers should use this function to dispatch data indications
to SLIM core.

@param     pz_ProviderStatus  Pointer to the provider status.
@param[in] pSlimDaemonMessage Message structure.
*/
void slim_ProviderDispatchIndication
(
  slim_ProviderStatusStructType *pz_ProviderStatus,
  const SlimDaemonMessage *pSlimDaemonMessage
);

/**
@brief Route function for time responses

Provider modules should use this function to route time responses through
SLIM-AP daemon task.

@param[in] l_TxnId           SLIM transaction id.
@param[in] e_Provider        SLIM client handle.
@param[in] e_Error           Error code.
@param[in] t_RemoteTimeMsecs Remote time of provider.
*/
void slim_ProviderRouteTimeResponse
(
  int32 l_TxnId,
  slim_ProviderEnumType e_Provider,
  slimErrorEnumT e_Error,
  uint64 t_RemoteTimeMsecs
);

/**
@brief Dispatch function for time response.

AP provider modules should use this function to dispatch time responses
to SLIM core.

@param[in] pSlimDaemonMessage Message structure.
*/
void slim_ProviderDispatchTimeResponse
(
  const SlimDaemonMessage* pSlimDaemonMessage
);

/**
@brief Route function for generic responses

Provider modules should use this function to route generic service responses
through SLIM-AP daemon task.

@param[in] l_TxnId    SLIM transaction id.
@param[in] e_Provider SLIM client handle.
@param[in] e_Error    Error code.
*/
void slim_ProviderRouteGenericResponse
(
  int32 l_TxnId,
  slim_ProviderEnumType e_Provider,
  slimErrorEnumT e_Error
);

/**
@brief Dispatch function for generic responses

Provider modules should use this function to dispatch generic service responses
to SLIM core.

@param[in] pSlimDaemonMessage Message structure.
*/
void slim_ProviderDispatchGenericResponse
(
    const SlimDaemonMessage *pSlimDaemonMessage
);

#ifdef __cplusplus
}
#endif

#endif //SLIM_PROVIDER_COMMON_H
