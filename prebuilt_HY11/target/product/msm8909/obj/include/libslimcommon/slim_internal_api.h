#ifndef SLIM_INTERNAL_API_H
#define SLIM_INTERNAL_API_H
/*============================================================================
@file slim_internal_api.h

  SLIM internal API header file

  SLIM internal API is extension for the SLIM public API. Internal API is
  intended only for use of internal SLIM clients.

               Copyright (c) 2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/osal/inc/slim_internal_api.h#1 $ */

/*----------------------------------------------------------------------------
* Include Files
* -------------------------------------------------------------------------*/
#include "slim_api.h"
#include "slim_internal_client_types.h"

/*----------------------------------------------------------------------------
* Function Declarations and Documentation
* -------------------------------------------------------------------------*/
/**
@brief Get request for provider time.

Specific provider time can be gotten by request. Time request need to contain
the service and the provider information client is interested in.
Result to the time request is provided to client via registered callback
function. @see slimNotifyCallbackFunctionT
 - Service: eSLIM_SERVICE_NONE
 - Type:    eSLIM_MESSAGE_TYPE_RESPONSE
 - Error:   eSLIM_SUCCESS if request was successfull, otherwise SLIM error code.
 - ID/Payload:
    - eSLIM_MESSAGE_ID_PROVIDER_TIME_GET_RESP/slimGetProviderTimeResponseStructT


@param  p_Handle: The opaque handle used to identify this client.
@param  pz_Request : Data for time request.
@param  u_TxnId : A transaction ID for the service request. This id is
provided back to client when a response to this request is sent.
@return eSLIM_SUCCESS if request was sent successfully. Otherwise SLIM error code.
*/
extern slimErrorEnumT slim_GetProviderTime
(
  slimClientHandleT p_Handle,
  const slimGetProviderTimeRequestStructT *pz_Request,
  uint8 u_TxnId
);
#endif /* #ifndef SLIM_INTERNAL_API_H */
