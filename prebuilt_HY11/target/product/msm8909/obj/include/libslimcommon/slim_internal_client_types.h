#ifndef SLIM_INTERNAL_CLIENT_TYPES_H
#define SLIM_INTERNAL_CLIENT_TYPES_H
/*============================================================================
@file slim_internal_client_types.h

  SLIM internal API header containing the client types needed for internal
  SLIM API use.

               Copyright (c) 2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/osal/inc/slim_internal_client_types.h#1 $ */

/*----------------------------------------------------------------------------
* Include Files
* -------------------------------------------------------------------------*/
#include "slim_client_types.h"

/*----------------------------------------------------------------------------
* Preprocessor Definitions and Constants
* -------------------------------------------------------------------------*/
/** Value for the first internal message id. Note that this should not
    overlap with SLIM API message ids. */
#define SLIM_FIRST_INTERNAL_MSG_ID (eSLIM_MESSAGE_ID_MAX_VALUE + 1)

/*----------------------------------------------------------------------------
* Type Declarations
* -------------------------------------------------------------------------*/
/** Enum for internal SLIM message ids. Payload is valid only with specific
    message id only if the error code is eSLIM_SUCCESS in the message header. */
typedef enum
{
  /* Response to provider time request */
  eSLIM_MESSAGE_ID_PROVIDER_TIME_GET_RESP = SLIM_FIRST_INTERNAL_MSG_ID,
  /**< slimGetProviderTimeResponseStructT */

  eSLIM_MESSAGE_ID_INTERNAL_MAX = 2147483647 /* Force 32bit */
} slimInternalMessageIdEnumT;

/*! Struct for provider get time response */
typedef struct
{
  slimServiceEnumT         service;
  /**< SLIM service for the time response. */

  slimServiceProviderEnumT provider;
  /**< Time response provider. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  uint32_t                 referenceTimeMs;
  /**< Reference time provided by client in the request. */

  uint32_t                 remoteReceiveTimeMs;
  /**< Provider receive time in milliseconds. */

  uint32_t                 remoteTransmitTimeMs;
  /**< Provider transmit time in milliseconds. */

} slimGetProviderTimeResponseStructT;

/*! Struct for provider get time request parameters*/
typedef struct
{
  slimServiceEnumT service;
  /**< SLIM service to get the time for. */

  slimServiceProviderEnumT provider;
  /**< Provider type to get the time for. One of: \n
         eSLIM_SERVICE_PROVIDER_DEFAULT
         eSLIM_SERVICE_PROVIDER_SSC \n
         eSLIM_SERVICE_PROVIDER_SAMLITE \n
         eSLIM_SERVICE_PROVIDER_NATIVE */

  uint32_t referenceTimeMs;
  /**< Client can use this field to store the local reference time in
       milliseconds. It is echoed back in the response. */

} slimGetProviderTimeRequestStructT;

#endif /* SLIM_INTERNAL_CLIENT_TYPES_H */
