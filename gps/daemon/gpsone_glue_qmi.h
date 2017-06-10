/*   Copyright (c) 2012 Qualcomm Atheros, Inc.
     All Rights Reserved.
     Qualcomm Atheros Confidential and Proprietary
 */

#ifndef GPSONE_GLUE_QMI_H
#define GPSONE_GLUE_QMI_H

#ifdef __cplusplus
extern "C" {
#endif


/*=============================================================================
 *
 *                             DATA DECLARATION
 *
 *============================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bearer_independent_transport_v01.h"  //QMI BIT Service data types definitions
/******************************************************************************
 *  Constants and configuration
 *****************************************************************************/

/** Specific value of userHandle, indicating an invalid handle. */
#define BIT_CLIENT_INVALID_HANDLE_VALUE (NULL)

typedef enum
{
  eBIT_CLIENT_SUCCESS                              = 0,
  /**< Request was successful. */

  eBIT_CLIENT_FAILURE_GENERAL                      = 1,
  /**< Failed because of a general failure. */

  eBIT_CLIENT_FAILURE_UNSUPPORTED                  = 2,
  /**< Failed because the service does not support the command. */

  eBIT_CLIENT_FAILURE_INVALID_PARAMETER            = 3,
  /**< Failed because the request contained invalid parameters. */

  eBIT_CLIENT_FAILURE_ENGINE_BUSY                  = 4,
  /**< Failed because the engine is busy. */

  eBIT_CLIENT_FAILURE_PHONE_OFFLINE                = 5,
  /**< Failed because the phone is offline. */

  eBIT_CLIENT_FAILURE_TIMEOUT                      = 6,
  /**< Failed because of a timeout. */

  eBIT_CLIENT_FAILURE_SERVICE_NOT_PRESENT          = 7,
  /**< Failed because the service is not present. */

  eBIT_CLIENT_FAILURE_SERVICE_VERSION_UNSUPPORTED  = 8,
  /**< Failed because the service version is unsupported. */

  eBIT_CLIENT_FAILURE_CLIENT_VERSION_UNSUPPORTED  =  9,
  /**< Failed because the service does not support client version. */

  eBIT_CLIENT_FAILURE_INVALID_HANDLE               = 10,
  /**< Failed because an invalid handle was specified. */

  eBIT_CLIENT_FAILURE_INTERNAL                     = 11,
  /**< Failed because of an internal error in the service. */

  eBIT_CLIENT_FAILURE_NOT_INITIALIZED              = 12,
  /**< Failed because the service has not been initialized. */

  eBIT_CLIENT_FAILURE_NOT_ENOUGH_MEMORY             = 13
  /**< Failed because not rnough memory to do the operation.*/

}bitClientStatusEnumType;

/** @brief Request messages the BIT client can send to the BIT service
*/
typedef union
{
   const bit_data_received_ind_msg_v01* pDataReceivedInd;

   const bit_open_status_ind_msg_v01* pOpenStatusInd;

   const bit_connect_status_ind_msg_v01* pConnectStatusInd;

   const bit_send_status_ind_msg_v01* pSendStatusInd;

   const bit_get_local_host_info_status_ind_msg_v01* pGetLocalHostInfoInd;

   const bit_set_dormancy_status_ind_msg_v01* pSetDormancyStatusInd;

   const bit_disconnect_status_ind_msg_v01* pDisconnectStatusInd;

   const bit_close_status_ind_msg_v01* pCloseStatusInd;

   const bit_resp_msg_v01* pReqAck;

   const bit_session_resp_msg_v01* pReqSessionAck;

}bitClientReqUnionType;


/*===========================================================================
 *
 *                          FUNCTION DECLARATION
 *
 *==========================================================================*/

/*
  gpsone_glue_qmi_init

  Connects a BIT client to BIT service. If the connection is successful, this function
  returns a success value

  @param[in]  bit_forward_qmi_msgqid  MsgQId for the BIT QMI Client Shim Thread
  @return
  One of the following error codes:
  - eBIT_CLIENT_SUCCESS -- If the connection is opened.
  - Non-zero error code (see \ref bitClientStatusEnumType) -- On failure.
*/
extern bitClientStatusEnumType gpsone_glue_qmi_init (int bit_forward_qmi_msgqid);


/*=============================================================================
  bitClientSendReq

  Sends a message to BIT service.If this function is successful it returns a success value

  @param[in] reqId         QMI_BIT service message ID of the request.
  @param[in] pReqPayload   Payload of the request. This can be NULL if the request
                           has no payload.

  @return
  One of the following error codes:
  - 0 (eBIT_CLIENT_SUCCESS) -- On success.
  - Non-zero error code (see \ref bitClientStatusEnumType) -- On failure.
*/
extern bitClientStatusEnumType gpsone_glue_qmi_send_req(
     uint32_t                  reqId,
     bitClientReqUnionType     reqPayload
);
/** gpsone_one_qmi_close
  @brief Disconnects a client from the BIT service on the modem

  @return
  One of the following error codes:
  - 0 (eBIT_CLIENT_SUCCESS) - On success.
  - non-zero error code(see bitClientStatusEnumType) - On failure.
*/

bitClientStatusEnumType gpsone_glue_qmi_close(void);

#ifdef __cplusplus
}
#endif

#endif /* GPSONE_GLUE_QMI_H*/
