/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/29/14   at      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_uim.h"
#include "qcril_uim_util.h"
#include "qcril_uim_qcci.h"
#include "qcril_qmi_client.h"
#include "qcril_uim_queue.h"
#include "user_identity_module_v01.h"
#if defined FEATURE_QCRIL_UIM_REMOTE_SERVER
  #include "sap-api.pb.h"
#elif defined FEATURE_QCRIL_UIM_SAP_SERVER_MODE
  #include <hardware/ril/librilutils/sap-api.pb.h>
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER */
#if defined (FEATURE_QCRIL_UIM_REMOTE_SERVER) || defined (FEATURE_QCRIL_UIM_SAP_SERVER_MODE)
#include "pb_decode.h"
#include "pb_encode.h"
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER || FEATURE_QCRIL_UIM_SAP_SERVER_MODE */

#define QCRIL_UIM_SAP_FREE_PTR(ptr)                                         \
  if (ptr != NULL)                                                          \
  {                                                                         \
    qcril_free(ptr);                                                        \
    ptr = NULL;                                                             \
  }                                                                         \

#define QCRIL_UIM_SAP_ALLOC_PTR_RETURN_IF_ERR(ptr,len)                      \
  ptr = qcril_malloc(len);                                                  \
  if (ptr == NULL)                                                          \
  {                                                                         \
    QCRIL_LOG_ERROR("%s","Error allocating request_ptr, cannot proceed");   \
    return NULL;                                                            \
  }                                                                         \

/* Forward declarations */
static const char *   qcril_uim_sap_get_version(void);
static void           qcril_uim_sap_on_cancel(RIL_Token token);
static int            qcril_uim_sap_on_supports(int request_id);
static RIL_RadioState qcril_uim_sap_on_state_request(void);
static void           qcril_uim_sap_on_request(int       request,
                                               void    * data,
                                               size_t    datalen,
                                               RIL_Token t);

/* Global variables */
uim_sap_status_enum_v01     sap_state = UIM_SAP_STATE_NOT_ENABLED_V01;
struct RIL_Env            * qcril_sap_response_api[QCRIL_MAX_INSTANCE_ID];
const RIL_RadioFunctions    qcril_sap_request_api[] =
{
  { RIL_VERSION,
    qcril_uim_sap_on_request,
    qcril_uim_sap_on_state_request,
    qcril_uim_sap_on_supports,
    qcril_uim_sap_on_cancel,
    qcril_uim_sap_get_version
  }
};


/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_sap_convert_slot_id_to_slot_type

===========================================================================*/
/*!
    @brief
    Routine to convert a RIL slot id to QMI slot enum type.

    @return
    Boolean indicating the outcome of the request.
*/
/*=========================================================================*/
static boolean qcril_uim_sap_convert_slot_id_to_slot_type
(
  uint32_t              slot_id,
  qmi_uim_slot_type   * qmi_slot_type_ptr
)
{
  if (qmi_slot_type_ptr == NULL)
  {
    return FALSE;
  }

  switch (slot_id)
  {
     case 0:
       *qmi_slot_type_ptr = QMI_UIM_SLOT_1;
       break;

     case 1:
       *qmi_slot_type_ptr = QMI_UIM_SLOT_2;
       break;

    case 2:
      *qmi_slot_type_ptr = QMI_UIM_SLOT_3;
      break;

    default:
      QCRIL_LOG_ERROR( "Invalid slot_id returned: 0x%x\n", slot_id);
      return FALSE;
  }

  return TRUE;
} /* qcril_uim_sap_convert_slot_id_to_slot_type */


#if defined (FEATURE_QCRIL_UIM_REMOTE_SERVER) || defined (FEATURE_QCRIL_UIM_SAP_SERVER_MODE)
/*=========================================================================

  FUNCTION:  qcril_uim_sap_check_supported_messages

===========================================================================*/
/*!
    @brief
    Checks if the particualr MsgId is supported by the RIL implementation.

    @return
    1 if supported, 0 otherwise.
*/
/*=========================================================================*/
static int qcril_uim_sap_check_supported_messages
(
  int     request_id
)
{
  int result = 0;

  switch ((MsgId)request_id)
  {
    /* Supported */
    case MsgId_RIL_SIM_SAP_CONNECT:
    case MsgId_RIL_SIM_SAP_DISCONNECT:
    case MsgId_RIL_SIM_SAP_APDU:
    case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
    case MsgId_RIL_SIM_SAP_POWER:
    case MsgId_RIL_SIM_SAP_RESET_SIM:
    case MsgId_RIL_SIM_SAP_STATUS:
    case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
    case MsgId_RIL_SIM_SAP_ERROR_RESP:
      result = 1;
      break;

    /* Unsupported */
    case MsgId_UNKNOWN_REQ:
    case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
    default:
      break;
  }
  return result;
} /* qcril_uim_sap_check_supported_messages */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_create_payload

===========================================================================*/
/*!
    @brief
    Allocated the buffer for the variable payload dynamically and copies the
    data that needs to sent in the response.
    Note that the memory allocated here needs to be freed by the caller.

    @return
    Pointer to valid buffer if successful, NULL otherwise.
*/
/*=========================================================================*/
static pb_bytes_array_t  * qcril_uim_sap_create_payload
(
  uint8   * data_ptr,
  uint16    data_len
)
{
  pb_bytes_array_t   * payload_data_ptr = NULL;
  uint16               payload_data_len = 0;

  /* Sanity check */
  if ((data_ptr == NULL) || (data_len == 0))
  {
    return NULL;
  }

  payload_data_len = sizeof(pb_bytes_array_t) + data_len;
  payload_data_ptr = (pb_bytes_array_t *)qcril_malloc(payload_data_len);
  if (payload_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "payload_data_ptr alloc failed");
    return NULL;
  }

  /* Copy the data */
  memset(payload_data_ptr, 0, payload_data_len);
  payload_data_ptr->size = (size_t)data_len;
  memcpy(payload_data_ptr->bytes, data_ptr, data_len);

  return payload_data_ptr;
} /* qcril_uim_sap_create_payload */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_unpack_message

===========================================================================*/
/*!
    @brief
    Unpacks specific messages by passing the incoming raw byte stream through
    the nanopb's decode API. Also note that the memory allocated for the
    request pointer must be freed by the caller.

    @return
    Valid pointer if supported, NULL otherwise.
*/
/*=========================================================================*/
static void * qcril_uim_sap_unpack_message
(
  MsgId                         message_id,
  uint8                       * payload_ptr,
  size_t                        payload_len
)
{
  void          * request_ptr   = NULL;
  boolean         decode_status = FALSE;
  pb_istream_t    stream;

  /* Create the stream */
  stream = pb_istream_from_buffer((uint8_t *)payload_ptr, payload_len);

  /* Decode based on the message id */
  switch (message_id)
  {
    case MsgId_RIL_SIM_SAP_CONNECT:
      QCRIL_UIM_SAP_ALLOC_PTR_RETURN_IF_ERR(request_ptr, sizeof(RIL_SIM_SAP_CONNECT_REQ));
      if (!pb_decode(&stream, RIL_SIM_SAP_CONNECT_REQ_fields, request_ptr))
      {
        QCRIL_LOG_ERROR("%s","Error allocating request_ptr, cannot proceed");
        return NULL;
      }
      break;

    case MsgId_RIL_SIM_SAP_DISCONNECT:
      QCRIL_UIM_SAP_ALLOC_PTR_RETURN_IF_ERR(request_ptr, sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
      if (!pb_decode(&stream, RIL_SIM_SAP_DISCONNECT_REQ_fields, request_ptr))
      {
        QCRIL_LOG_ERROR("%s","Error allocating request_ptr, cannot proceed");
        return NULL;
      }
      break;

    case MsgId_RIL_SIM_SAP_APDU:
      QCRIL_UIM_SAP_ALLOC_PTR_RETURN_IF_ERR(request_ptr, sizeof(RIL_SIM_SAP_APDU_REQ));
      if (!pb_decode(&stream, RIL_SIM_SAP_APDU_REQ_fields, request_ptr))
      {
        QCRIL_LOG_ERROR("%s","Error allocating request_ptr, cannot proceed");
        return NULL;
      }
      break;

    case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
      QCRIL_UIM_SAP_ALLOC_PTR_RETURN_IF_ERR(request_ptr, sizeof(RIL_SIM_SAP_TRANSFER_ATR_REQ));
      if (!pb_decode(&stream, RIL_SIM_SAP_TRANSFER_ATR_REQ_fields, request_ptr))
      {
        QCRIL_LOG_ERROR("%s","Error allocating request_ptr, cannot proceed");
        return NULL;
      }
      break;

    case MsgId_RIL_SIM_SAP_POWER:
      QCRIL_UIM_SAP_ALLOC_PTR_RETURN_IF_ERR(request_ptr, sizeof(RIL_SIM_SAP_POWER_REQ));
      if (!pb_decode(&stream, RIL_SIM_SAP_POWER_REQ_fields, request_ptr))
      {
        QCRIL_LOG_ERROR("%s","Error allocating request_ptr, cannot proceed");
        return NULL;
      }
      break;

    case MsgId_RIL_SIM_SAP_RESET_SIM:
      QCRIL_UIM_SAP_ALLOC_PTR_RETURN_IF_ERR(request_ptr, sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
      if (!pb_decode(&stream, RIL_SIM_SAP_RESET_SIM_REQ_fields, request_ptr))
      {
        QCRIL_LOG_ERROR("%s","Error allocating request_ptr, cannot proceed");
        return NULL;
      }
      break;

    case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
      QCRIL_UIM_SAP_ALLOC_PTR_RETURN_IF_ERR(request_ptr, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ));
      if (!pb_decode(&stream, RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ_fields, request_ptr))
      {
        QCRIL_LOG_ERROR("%s","Error allocating request_ptr, cannot proceed");
        return NULL;
      }
      break;

    /* Unsupported */
    case MsgId_UNKNOWN_REQ:
    case MsgId_RIL_SIM_SAP_ERROR_RESP:
    case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
    default:
      break;
  }
  return request_ptr;
} /* qcril_uim_sap_unpack_message */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_dispatch_unsol_response

===========================================================================*/
/*!
    @brief
    Handles the SAP connection request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    Note that the userdata pointer & the QMI response pointer is freed in
    the caller - the main response callback.

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_sap_dispatch_unsol_response
(
  qcril_instance_id_e_type        instance_id,
  MsgId                           message_id,
  void                          * response_ptr,
  uint16                          response_len
)
{
  QCRIL_LOG_INFO( "Sending OnUnsolicitedResponse data 0x%x bytes on instance_id: 0x%x",
                  response_len, instance_id);

  /* Send the data using OnUnsolicitedResponse */
  qcril_sap_response_api[instance_id]->OnUnsolicitedResponse(
                                         message_id,
                                         response_ptr,
                                         response_len);
} /* qcril_uim_sap_dispatch_unsol_response */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_dispatch_response

===========================================================================*/
/*!
    @brief
    Handles the SAP connection request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    Note that the userdata pointer & the QMI response pointer is freed in
    the caller - the main response callback.

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_sap_dispatch_response
(
  qcril_instance_id_e_type        instance_id,
  RIL_Token                       token,
  RIL_Errno                       ril_err,
  void                          * response_ptr,
  uint16                          response_len
)
{
  QCRIL_LOG_INFO( "Sending OnRequestComplete data 0x%x bytes on instance_id: 0x%x",
                  response_len, instance_id);

  /* Send the data using onRequestComplete */
  qcril_sap_response_api[instance_id]->OnRequestComplete(
                                         token,
                                         ril_err,
                                         response_ptr,
                                         response_len);
} /* qcril_uim_sap_dispatch_response */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_qmi_handle_sap_ind

===========================================================================*/
/*!
    @brief
    Handles the SAP indication callback. Based on the indication received
    from the modem, respective packed unsol response types are constructed
    and the OnUnsolicitedResponse is called.

    Note that the qmi_indication_ptr pointer is freed in the caller - the
    main indication callback.

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_sap_qmi_handle_sap_ind
(
  qcril_instance_id_e_type          instance_id,
  uim_sap_connection_ind_msg_v01  * qmi_indication_ptr
)
{
  MsgId                           message_id       = 0;
  void                          * response_ptr     = NULL;
  uint16                          response_len     = 0;
  pb_ostream_t                    stream;

  /* Sanity check */
  if ((qmi_indication_ptr == NULL) ||
      (qmi_indication_ptr->sap_connection_event_valid == FALSE))
  {
    return;
  }

  QCRIL_LOG_DEBUG("qcril_uim_sap_qmi_handle_sap_ind, instance_id: 0x%x, slot: 0x%x, sap_state: 0x%x ",
                  instance_id,
                  qmi_indication_ptr->sap_connection_event.slot,
                  qmi_indication_ptr->sap_connection_event.sap_state);

  /* Only 2 unsol responses are send from the QMI IND & it depends on the sap_status:
     1. RIL_SIM_SAP_STATUS_IND, upon status for connection establishment/disconnection
     2. RIL_SIM_SAP_DISCONNECT_IND, upon disconnection request only */
  sap_state = qmi_indication_ptr->sap_connection_event.sap_state;
  switch (sap_state)
  {
    case UIM_SAP_STATE_DISCONNECTED_SUCCESSFULLY_V01:
    {
      RIL_SIM_SAP_DISCONNECT_IND   disconnect_ind;

      response_ptr = qcril_malloc(sizeof(RIL_SIM_SAP_DISCONNECT_IND));
      if (response_ptr == NULL)
      {
        QCRIL_LOG_ERROR("%s","Error allocating response_ptr");
        return;
      }

      memset(&disconnect_ind, 0, sizeof(RIL_SIM_SAP_DISCONNECT_IND));
      memset(response_ptr, 0, sizeof(RIL_SIM_SAP_DISCONNECT_IND));

      message_id = MsgId_RIL_SIM_SAP_DISCONNECT;
      disconnect_ind.disconnectType =
        RIL_SIM_SAP_DISCONNECT_IND_DisconnectType_RIL_S_DISCONNECT_TYPE_GRACEFUL;

      /* Create an output stream & encode the outgoing message */
      stream = pb_ostream_from_buffer(response_ptr, sizeof(RIL_SIM_SAP_DISCONNECT_IND));

      if (!pb_encode(&stream, RIL_SIM_SAP_DISCONNECT_IND_fields, &disconnect_ind))
      {
        QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_DISCONNECT_IND");
        QCRIL_UIM_SAP_FREE_PTR(response_ptr);
        return;
      }
      response_len = stream.bytes_written;
    }
    break;

    case UIM_SAP_STATE_CONNECTION_ERROR_V01:
    case UIM_SAP_STATE_NOT_ENABLED_V01:
    {
      RIL_SIM_SAP_STATUS_IND   msg_status_ind;

      response_ptr = qcril_malloc(sizeof(RIL_SIM_SAP_STATUS_IND));
      if (response_ptr == NULL)
      {
        QCRIL_LOG_ERROR("%s","Error allocating response_ptr");
        return;
      }

      memset(&msg_status_ind, 0, sizeof(RIL_SIM_SAP_STATUS_IND));
      memset(response_ptr, 0, sizeof(RIL_SIM_SAP_STATUS_IND));

      message_id = MsgId_RIL_SIM_SAP_STATUS;

      msg_status_ind.statusChange =
          RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_NOT_ACCESSIBLE;

      /* Create an output stream & encode the outgoing message */
      stream = pb_ostream_from_buffer(response_ptr, sizeof(RIL_SIM_SAP_STATUS_IND));

      if (!pb_encode(&stream, RIL_SIM_SAP_STATUS_IND_fields, &msg_status_ind))
      {
        QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_STATUS_IND");
        QCRIL_UIM_SAP_FREE_PTR(response_ptr);
        return;
      }
      response_len = stream.bytes_written;
    }
    break;

    case UIM_SAP_STATE_CONNECTED_SUCCESSFULLY_V01:
    case UIM_SAP_STATE_CONNECTING_V01:
    case UIM_SAP_STATE_DISCONNECTING_V01:
    default:
      /* Note that for a succesfully connected case, if the connection response
           isnt sent yet, the STATUS_IND is sent after response callback comes */
      QCRIL_LOG_DEBUG("Skipping SAP UNSOL response for sap_state: 0x%x", sap_state);
      return;
  }

  /* Send the UNSOL Response for valid cases */
  if ((response_ptr != NULL) && (response_len != 0))
  {
    qcril_uim_sap_dispatch_unsol_response(instance_id,
                                          message_id,
                                          response_ptr,
                                          response_len);
  }

  /* Clear allocated response pointer */
  QCRIL_UIM_SAP_FREE_PTR(response_ptr);
} /* qcril_uim_sap_qmi_handle_sap_ind */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_qmi_handle_sap_connection_resp

===========================================================================*/
/*!
    @brief
    Handles the SAP connection request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sap_qmi_handle_sap_connection_resp
(
  const qcril_uim_callback_params_type * const params_ptr
)
{
  MsgId                                message_id         = MsgId_UNKNOWN_REQ;
  RIL_Token                            token              = NULL;
  RIL_Errno                            ril_err;
  qcril_instance_id_e_type             instance_id        = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_uim_original_request_type    * orig_request_ptr   = NULL;
  void                               * response_ptr       = NULL;
  uint16                               response_len       = 0;
  void                               * unsol_response_ptr = NULL;
  uint16                               unsol_response_len = 0;
  pb_ostream_t                         stream;

  /* Sanity check */
  if(params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL params_ptr");
    return;
  }

  /* Retrieve original request */
  orig_request_ptr = (qcril_uim_original_request_type*)params_ptr->orig_req_data;
  if(orig_request_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL original_request_ptr");
    return;
  }

  /* Get parameters from original request param */
  token       = orig_request_ptr->token;
  instance_id = orig_request_ptr->instance_id;
  message_id  = orig_request_ptr->request_id;

  /* Free orig_request_ptr allocated in the request */
  QCRIL_UIM_SAP_FREE_PTR(orig_request_ptr);

  QCRIL_LOG_DEBUG("handle_sap_connection_resp, token: %d, MsgId: 0x%x, qmi_err_code: 0x%x",
                  qcril_log_get_token_id(token),
                  message_id,
                  params_ptr->qmi_rsp_data.qmi_err_code);

  ril_err = (params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR) ?
               RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;

  /* Update the parameters for respective responses */
  if (message_id == MsgId_RIL_SIM_SAP_CONNECT)
  {
    RIL_SIM_SAP_CONNECT_RSP resp;

    response_ptr = qcril_malloc(sizeof(RIL_SIM_SAP_CONNECT_RSP));
    if (response_ptr == NULL)
    {
      QCRIL_LOG_ERROR("%s","Error allocating response_ptr");
      return;
    }

    /* Encode the outgoing message */
    memset(&resp, 0, sizeof(RIL_SIM_SAP_CONNECT_RSP));
    memset(response_ptr, 0, sizeof(RIL_SIM_SAP_CONNECT_RSP));

    stream = pb_ostream_from_buffer(response_ptr, sizeof(RIL_SIM_SAP_CONNECT_RSP));

    resp.response = (params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR) ?
                       RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SUCCESS :
                       RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;

    if (!pb_encode(&stream, RIL_SIM_SAP_CONNECT_RSP_fields, &resp))
    {
      QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_CONNECT_RSP");
      QCRIL_UIM_SAP_FREE_PTR(response_ptr);
      return;
    }
    response_len = stream.bytes_written;

    /* Now send UNSOL response if needed */
    if ((params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR) &&
        (sap_state == UIM_SAP_STATE_CONNECTED_SUCCESSFULLY_V01))
    {
      RIL_SIM_SAP_STATUS_IND   msg_status_ind;

      unsol_response_ptr = qcril_malloc(sizeof(RIL_SIM_SAP_STATUS_IND));
      if (unsol_response_ptr == NULL)
      {
        QCRIL_LOG_ERROR("%s","Error allocating unsol_response_ptr");
        return;
      }

      memset(&msg_status_ind, 0, sizeof(RIL_SIM_SAP_STATUS_IND));
      memset(unsol_response_ptr, 0, sizeof(RIL_SIM_SAP_STATUS_IND));

      message_id = MsgId_RIL_SIM_SAP_STATUS;

      msg_status_ind.statusChange =
          RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_RESET;

      /* Create an output stream & encode the outgoing message */
      stream = pb_ostream_from_buffer(unsol_response_ptr, sizeof(RIL_SIM_SAP_STATUS_IND));

      if (!pb_encode(&stream, RIL_SIM_SAP_STATUS_IND_fields, &msg_status_ind))
      {
        QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_STATUS_IND");
        QCRIL_UIM_SAP_FREE_PTR(unsol_response_ptr);
        return;
      }
      unsol_response_len = stream.bytes_written;
    }
  }
  else if (message_id == MsgId_RIL_SIM_SAP_DISCONNECT)
  {
    RIL_SIM_SAP_DISCONNECT_RSP resp;

    /* Nothing to update in the response struct */
    response_ptr = qcril_malloc(sizeof(RIL_SIM_SAP_DISCONNECT_RSP));
    if (response_ptr == NULL)
    {
      QCRIL_LOG_ERROR("%s","Error allocating response_ptr, cannot proceed");
      return;
    }

    /* Encode the outgoing message */
    memset(&resp, 0, sizeof(RIL_SIM_SAP_DISCONNECT_RSP));
    memset(response_ptr, 0, sizeof(RIL_SIM_SAP_DISCONNECT_RSP));

    stream = pb_ostream_from_buffer(response_ptr, sizeof(RIL_SIM_SAP_DISCONNECT_RSP));

    /* Encode the message */
    if (!pb_encode(&stream, RIL_SIM_SAP_DISCONNECT_RSP_fields, &resp))
    {
      QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_CONNECT_RSP");
      QCRIL_UIM_SAP_FREE_PTR(response_ptr);
      return;
    }
    response_len = stream.bytes_written;
  }
  else
  {
    QCRIL_LOG_ERROR("Unsupported MsgId: 0x%x", message_id);
    return;
  }

  /* Send the response */
  qcril_uim_sap_dispatch_response(instance_id,
                                  token,
                                  ril_err,
                                  response_ptr,
                                  response_len);

  /* Send the UNSOL Response if needed */
  if ((unsol_response_ptr != NULL) && (unsol_response_len != 0))
  {
    qcril_uim_sap_dispatch_unsol_response(instance_id,
                                          message_id,
                                          unsol_response_ptr,
                                          unsol_response_len);
  }

  /* Clear allocated response pointer */
  QCRIL_UIM_SAP_FREE_PTR(response_ptr);
  QCRIL_UIM_SAP_FREE_PTR(unsol_response_ptr);
} /* qcril_uim_sap_qmi_handle_sap_connection_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_qmi_handle_sap_request_resp

===========================================================================*/
/*!
    @brief
    Handles the SAP request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sap_qmi_handle_sap_request_resp
(
  const qcril_uim_callback_params_type * const params_ptr
)
{
  MsgId                                message_id       = MsgId_UNKNOWN_REQ;
  RIL_Token                            token            = NULL;
  RIL_Errno                            ril_err;
  qcril_instance_id_e_type             instance_id      = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_uim_original_request_type    * orig_request_ptr = NULL;
  void                               * response_ptr     = NULL;
  uint16                               response_len     = 0;
  pb_bytes_array_t                   * payload_data_ptr = NULL;
  size_t                               encoded_size     = 0;
  pb_ostream_t                         stream;


  /* Sanity check */
  if(params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL params_ptr");
    return;
  }

  /* Retrieve original request */
  orig_request_ptr = (qcril_uim_original_request_type*)params_ptr->orig_req_data;
  if(orig_request_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL original_request_ptr");
    return;
  }

  ril_err = (params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR) ?
              RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;

  /* Get parameters from original request param */
  token       = orig_request_ptr->token;
  instance_id = orig_request_ptr->instance_id;
  message_id  = orig_request_ptr->request_id;

  /* Free orig_request_ptr allocated in the request */
  QCRIL_UIM_SAP_FREE_PTR(orig_request_ptr);

  QCRIL_LOG_DEBUG("handle_sap_request_resp, token: %d, MsgId: 0x%x, qmi_err_code: 0x%x",
                  qcril_log_get_token_id(token),
                  message_id,
                  params_ptr->qmi_rsp_data.qmi_err_code);

  /* Update the parameters for respective responses */
  switch (message_id)
  {
    case MsgId_RIL_SIM_SAP_APDU:
      payload_data_ptr = qcril_uim_sap_create_payload(
        params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr,
        params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_len);
      if (payload_data_ptr)
      {
        RIL_SIM_SAP_APDU_RSP   resp;

        /* Update the outgoing message */
        memset(&resp, 0, sizeof(RIL_SIM_SAP_APDU_RSP));
        resp.apduResponse = (pb_bytes_array_t*)payload_data_ptr;
        resp.type         = RIL_SIM_SAP_APDU_RSP_Type_RIL_TYPE_APDU;
        resp.response     = (params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR) ?
                               RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SUCCESS :
                               RIL_SIM_SAP_APDU_RSP_Response_RIL_E_GENERIC_FAILURE;

        /* Encode the message. For variable pb byte array, we need to
           first find the encoded size */
        if (!pb_get_encoded_size(&encoded_size,
                                 RIL_SIM_SAP_APDU_RSP_fields,
                                 &resp))
        {
          QCRIL_UIM_SAP_FREE_PTR(payload_data_ptr);
          QCRIL_LOG_ERROR("%s","Error in pb_get_encoded_size");
          return;
        }

        response_ptr = qcril_malloc(encoded_size);
        if (response_ptr == NULL)
        {
          QCRIL_UIM_SAP_FREE_PTR(payload_data_ptr);
          QCRIL_LOG_ERROR("%s","Error allocating response_ptr");
          return;
        }

        memset(response_ptr, 0, encoded_size);
        stream = pb_ostream_from_buffer(response_ptr, encoded_size);
        if (!pb_encode(&stream, RIL_SIM_SAP_APDU_RSP_fields, &resp))
        {
          QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_APDU_RSP");
          QCRIL_UIM_SAP_FREE_PTR(payload_data_ptr);
          QCRIL_UIM_SAP_FREE_PTR(response_ptr);
          return;
        }
        response_len = stream.bytes_written;
      }
      break;

     case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
       payload_data_ptr = qcril_uim_sap_create_payload(
         params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr,
         params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_len);
       if (payload_data_ptr)
       {
         RIL_SIM_SAP_TRANSFER_ATR_RSP resp;

         /* Update the outgoing message */
         memset(&resp, 0, sizeof(RIL_SIM_SAP_TRANSFER_ATR_RSP));
         resp.atr = (pb_bytes_array_t*)payload_data_ptr;
         resp.response = (params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR) ?
                            RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SUCCESS :
                            RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_GENERIC_FAILURE;

         /* Encode the message. For variable pb byte array, we need to
           first find the encoded size */
         if (!pb_get_encoded_size(&encoded_size,
                                  RIL_SIM_SAP_TRANSFER_ATR_RSP_fields,
                                  &resp))
         {
           QCRIL_UIM_SAP_FREE_PTR(payload_data_ptr);
           QCRIL_LOG_ERROR("%s","Error in pb_get_encoded_size");
           return;
         }

         response_ptr = qcril_malloc(encoded_size);
         if (response_ptr == NULL)
         {
           QCRIL_UIM_SAP_FREE_PTR(payload_data_ptr);
           QCRIL_LOG_ERROR("%s","Error allocating response_ptr");
           return;
         }

         memset(response_ptr, 0, encoded_size);
         stream = pb_ostream_from_buffer(response_ptr, encoded_size);
         if (!pb_encode(&stream, RIL_SIM_SAP_TRANSFER_ATR_RSP_fields, &resp))
         {
           QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_TRANSFER_ATR_RSP");
           QCRIL_UIM_SAP_FREE_PTR(payload_data_ptr);
           QCRIL_UIM_SAP_FREE_PTR(response_ptr);
           return;
         }
         response_len = stream.bytes_written;
       }
       break;

     case MsgId_RIL_SIM_SAP_POWER:
       {
         RIL_SIM_SAP_POWER_RSP resp;

         response_ptr = qcril_malloc(sizeof(RIL_SIM_SAP_POWER_RSP));
         if (response_ptr == NULL)
         {
           QCRIL_LOG_ERROR("%s","Error allocating response_ptr");
           return;
         }

         memset(&resp, 0, sizeof(RIL_SIM_SAP_POWER_RSP));
         memset(response_ptr, 0, sizeof(RIL_SIM_SAP_POWER_RSP));

         resp.response = (params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR) ?
                            RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SUCCESS :
                            RIL_SIM_SAP_POWER_RSP_Response_RIL_E_GENERIC_FAILURE;

         /* Encode the outgoing message */
         stream = pb_ostream_from_buffer(response_ptr, sizeof(RIL_SIM_SAP_POWER_RSP));
         if (!pb_encode(&stream, RIL_SIM_SAP_POWER_RSP_fields, &resp))
         {
           QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_POWER_RSP");
           QCRIL_UIM_SAP_FREE_PTR(response_ptr);
           return;
         }
         response_len = stream.bytes_written;
       }
       break;

     case MsgId_RIL_SIM_SAP_RESET_SIM:
       {
         RIL_SIM_SAP_RESET_SIM_RSP resp;

         response_ptr = qcril_malloc(sizeof(RIL_SIM_SAP_RESET_SIM_RSP));
         if (response_ptr == NULL)
         {
           QCRIL_LOG_ERROR("%s","Error allocating response_ptr");
           return;
         }

         memset(&resp, 0, sizeof(RIL_SIM_SAP_RESET_SIM_RSP));
         memset(response_ptr, 0, sizeof(RIL_SIM_SAP_RESET_SIM_RSP));

         resp.response = (params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR)?
                            RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SUCCESS :
                            RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_GENERIC_FAILURE;

         /* Encode the outgoing message */
         stream = pb_ostream_from_buffer(response_ptr, sizeof(RIL_SIM_SAP_RESET_SIM_RSP));
         if (!pb_encode(&stream, RIL_SIM_SAP_RESET_SIM_RSP_fields, &resp))
         {
           QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_RESET_SIM_RSP");
           QCRIL_UIM_SAP_FREE_PTR(response_ptr);
           return;
         }
         response_len = stream.bytes_written;
       }
       break;

     case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
       {
         RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP resp;

         response_ptr = qcril_malloc(sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP));
         if (response_ptr == NULL)
         {
           QCRIL_LOG_ERROR("%s","Error allocating response_ptr");
           return;
         }

         memset(response_ptr, 0, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP));
         memset(&resp, 0, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP));
         resp.response = (params_ptr->qmi_rsp_data.qmi_err_code == QMI_NO_ERR) ?
                           RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_SUCCESS :
                           RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_Response_RIL_E_GENERIC_FAILURE;
         if ((params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr != NULL) &&
             (params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_len > 0) &&
             (params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_len < sizeof(int32_t)))
         {
           resp.has_CardReaderStatus = TRUE;
           memcpy(&resp.CardReaderStatus,
                  params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr,
                  params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_len);
         }

         /* Encode the outgoing message */
         stream = pb_ostream_from_buffer(response_ptr, sizeof(RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP));
         if (!pb_encode(&stream, RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_fields, &resp))
         {
           QCRIL_LOG_ERROR("%s","Error encoding RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP");
           QCRIL_UIM_SAP_FREE_PTR(response_ptr);
           return;
         }
         response_len = stream.bytes_written;
       }
       break;

     default:
       QCRIL_LOG_ERROR("Unsupported MsgId: 0x%x", message_id);
       return;
  }

  /* Send the response */
  qcril_uim_sap_dispatch_response(instance_id,
                                  token,
                                  ril_err,
                                  response_ptr,
                                  response_len);

  /* Free up any memory allocated */
  QCRIL_UIM_SAP_FREE_PTR(payload_data_ptr);
  QCRIL_UIM_SAP_FREE_PTR(response_ptr);
} /* qcril_uim_sap_qmi_handle_sap_request_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_handle_qmi_sap_request

===========================================================================*/
/*!
    @brief
    Handler function to send QMI_UIM_SAP_REQUEST to the modem.

    @return
    Boolean indicating the outcome of the request.
*/
/*=========================================================================*/
static boolean qcril_uim_sap_handle_qmi_sap_request
(
  qcril_instance_id_e_type     instance_id,
  RIL_Token                    token,
  MsgId                        message_id,
  uint8                      * payload_ptr,
  size_t                       payload_len
)
{
  int                                 qmi_result       = 0;
  size_t                              apdu_len         = 0;
  qmi_uim_slot_type                   qmi_slot         = QMI_UIM_SLOT_1;
  void                              * message_ptr      = NULL;
  qcril_uim_original_request_type   * original_req_ptr = NULL;
  qmi_uim_sap_request_params_type     sap_request_params;

  if (!qcril_uim_sap_convert_slot_id_to_slot_type(qmi_ril_get_sim_slot(), &qmi_slot))
  {
    return FALSE;
  }

  /* Unpack incoming data based on the message id */
  message_ptr = qcril_uim_sap_unpack_message(message_id, payload_ptr, payload_len);
  if (message_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Failed to unpack payload, message_id: 0x%x", message_id);
    return FALSE;
  }

  memset(&sap_request_params, 0, sizeof(qmi_uim_sap_request_params_type));

  /* Update QMI parameters from protobuf request & dispatch it to modem */
  sap_request_params.slot = qmi_slot;

  switch (message_id)
  {
    case MsgId_RIL_SIM_SAP_APDU:
      if (((RIL_SIM_SAP_APDU_REQ *)message_ptr)->command == NULL)
      {
        QCRIL_LOG_ERROR( "%s", "Null apdu ptr, cannot proceed");
        goto send_error;
      }

      if ((((RIL_SIM_SAP_APDU_REQ *)message_ptr)->command->size <= 0) ||
          (((RIL_SIM_SAP_APDU_REQ *)message_ptr)->command->size > QMI_UIM_APDU_DATA_MAX_V01))
      {
        QCRIL_LOG_ERROR("invalid command->size: 0x%x, cannot proceed",
                        ((RIL_SIM_SAP_APDU_REQ *)message_ptr)->command->size);
        goto send_error;
      }

      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_SEND_APDU;
      sap_request_params.apdu.data_len = ((RIL_SIM_SAP_APDU_REQ *)message_ptr)->command->size;
      sap_request_params.apdu.data_ptr = ((RIL_SIM_SAP_APDU_REQ *)message_ptr)->command->bytes;
      break;

     case MsgId_RIL_SIM_SAP_POWER:
       if (((RIL_SIM_SAP_POWER_REQ *)message_ptr)->state)
       {
         sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_POWER_SIM_ON;
       }
       else
       {
         sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_POWER_SIM_OFF;
       }
      break;

    case MsgId_RIL_SIM_SAP_RESET_SIM:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_RESET_SIM;
      break;

    case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_GET_ATR;
      break;

    case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_READER_STATUS;
      break;

    default:
      QCRIL_LOG_ERROR( "Unspported message_id: 0x%x\n", message_id);
      goto send_error;
  }

  /* Create userdata, it is freed in the callback */
  original_req_ptr = qcril_uim_allocate_orig_request(instance_id,
                                                     QCRIL_DEFAULT_MODEM_ID,
                                                     token,
                                                     (int)message_id,
                                                     0);
  if (original_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for original_request_ptr!");
    goto send_error;
  }

  /* Dispatch request to modem. In case of error, clean up */
  qmi_result = qcril_uim_queue_send_request(QCRIL_UIM_REQUEST_SAP_REQUEST,
                                            qcril_uim.qmi_handle,
                                            &sap_request_params,
                                            qmi_uim_callback,
                                            (void*)original_req_ptr);
  if (qmi_result >= 0)
  {
    QCRIL_UIM_SAP_FREE_PTR(message_ptr);
    return TRUE;
  }

  QCRIL_LOG_ERROR("SAP request failed, qmi_result: 0x%x", qmi_result);

send_error:
  /* Clear allocated message pointer */
  QCRIL_UIM_SAP_FREE_PTR(message_ptr);
  return FALSE;
} /* qcril_uim_sap_handle_qmi_sap_request */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_handle_qmi_sap_connection

===========================================================================*/
/*!
    @brief
    Handler function to send QMI_UIM_SAP_CONNECTION to the modem.

    @return
    Boolean indicating the outcome of the request.
*/
/*=========================================================================*/
static boolean qcril_uim_sap_handle_qmi_sap_connection
(
  qcril_instance_id_e_type     instance_id,
  RIL_Token                    token,
  MsgId                        message_id,
  uint8                      * payload_ptr,
  size_t                       payload_len
)
{
  int                                   qmi_result       = 0;
  qmi_uim_slot_type                     qmi_slot         = QMI_UIM_SLOT_1;
  void                                * message_ptr      = NULL;
  qcril_uim_original_request_type     * original_req_ptr = NULL;
  qmi_uim_sap_connection_params_type    sap_request_params;

  if (!qcril_uim_sap_convert_slot_id_to_slot_type(qmi_ril_get_sim_slot(), &qmi_slot))
  {
    return FALSE;
  }

  /* Unpack incoming data based on the message id */
  message_ptr = qcril_uim_sap_unpack_message(message_id, payload_ptr, payload_len);
  if (message_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Failed to unpack payload, message_id: 0x%x", message_id);
    return FALSE;
  }

  memset(&sap_request_params, 0, sizeof(qmi_uim_sap_connection_params_type));

  /* Update QMI parameters from protobuf request & dispatch it to modem */
  sap_request_params.slot           = qmi_slot;
  sap_request_params.conn_condition = QMI_UIM_SAP_CONN_COND_BLOCK_VOICE;

  if (message_id == MsgId_RIL_SIM_SAP_CONNECT)
  {
    sap_request_params.operation_type = QMI_UIM_SAP_CONNECTION_CONNECT;
  }
  else if (message_id == MsgId_RIL_SIM_SAP_DISCONNECT)
  {
    /* Note - for disconnect req, there is no mode passed from client */
    sap_request_params.operation_type  = QMI_UIM_SAP_CONNECTION_DISCONNECT;
    sap_request_params.disconnect_mode = QMI_UIM_SAP_DISCONNECT_MODE_GRACEFUL;
  }
  else
  {
    QCRIL_LOG_ERROR( "Unspported message_id: 0x%x\n", message_id);
    QCRIL_UIM_SAP_FREE_PTR(message_ptr);
    return FALSE;
  }

  QCRIL_UIM_SAP_FREE_PTR(message_ptr);

  /* Create userdata, it is freed in the callback */
  original_req_ptr = qcril_uim_allocate_orig_request(instance_id,
                                                     QCRIL_DEFAULT_MODEM_ID,
                                                     token,
                                                     (int)message_id,
                                                     0);
  if (original_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for original_request_ptr!");
    return FALSE;
  }

  /* Dispatch request to modem. In case of error, clean up */
  qmi_result = qcril_uim_queue_send_request(QCRIL_UIM_REQUEST_SAP_CONNECTION,
                                            qcril_uim.qmi_handle,
                                            &sap_request_params,
                                            qmi_uim_callback,
                                            (void*)original_req_ptr);
  if (qmi_result < 0)
  {
    QCRIL_LOG_ERROR("SAP connection request failed, qmi_result: 0x%x", qmi_result);
    QCRIL_UIM_SAP_FREE_PTR(original_req_ptr);
    return FALSE;
  }

  return TRUE;
} /* qcril_uim_sap_handle_qmi_sap_connection */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_process_message

===========================================================================*/
/*!
    @brief
    Processes incoming SAP messages. The expectation is that when this function
    is called, it is passed the data of entire message along with the header.
    Upon successful unpacking of the header & message, the request is sent to
    the modem.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_sap_process_message
(
  qcril_instance_id_e_type    instance_id,
  MsgId                       message_id,
  RIL_Token                   token,
  uint8                     * data_ptr,
  size_t                      data_len
)
{
  boolean                 result      = FALSE;
  IxErrnoType             evt_result  = E_FAILURE;

  QCRIL_LOG_INFO( "Received data 0x%x bytes on instance_id: 0x%x",
                  data_len, instance_id);

  /* Sanity check - only on data ptr, length could be 0 for certain messages */
  if (data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot proceed");
    return;
  }

  QCRIL_LOG_INFO( "Request details: MsgId: 0x%x, packed payload length: 0x%x",
                  message_id,
                  data_len);

  /* Pass it on the main request handling. Upon successful call to the modem,
     we expect the callback to respond back with the result */
  switch (message_id)
  {
    case MsgId_RIL_SIM_SAP_CONNECT:
    case MsgId_RIL_SIM_SAP_DISCONNECT:
      result = qcril_uim_sap_handle_qmi_sap_connection(instance_id,
                                                       token,
                                                       message_id,
                                                       data_ptr,
                                                       data_len);
      break;

    case MsgId_RIL_SIM_SAP_APDU:
    case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
    case MsgId_RIL_SIM_SAP_POWER:
    case MsgId_RIL_SIM_SAP_RESET_SIM:
    case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
      result = qcril_uim_sap_handle_qmi_sap_request(instance_id,
                                                    token,
                                                    message_id,
                                                    data_ptr,
                                                    data_len);
      break;

    case MsgId_RIL_SIM_SAP_ERROR_RESP:
    case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
    case MsgId_RIL_SIM_SAP_STATUS:
    default:
      result = FALSE;
      QCRIL_LOG_ERROR("Unhandled message, id: 0x%x", message_id);
      break;
  }

  if (result == FALSE)
  {
    /* If the QMI API failed, post an error response via an internal event since
       we cannot call onRequestComplete before the completion of onRequest API.
       Create a userdata to retrieve certain params and is freed in callback */
    qcril_uim_original_request_type   * original_req_ptr =
      qcril_uim_allocate_orig_request(instance_id,
                                      QCRIL_DEFAULT_MODEM_ID,
                                      (RIL_Token)token,
                                      (int)message_id,
                                      0);
    if (original_req_ptr != NULL)
    {
      QCRIL_LOG_INFO( "%s qcril_event_queue\n", __FUNCTION__);
      evt_result = qcril_event_queue(instance_id,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_DATA_NOT_ON_STACK,
                                     QCRIL_EVT_INTERNAL_UIM_SAP_RESP,
                                     (void *)original_req_ptr,
                                     sizeof(qcril_uim_original_request_type),
                                     NULL);
      if (evt_result != E_SUCCESS)
      {
        QCRIL_LOG_ERROR( " qcril_event_queue failed, result: 0x%x\n", evt_result);
        /* Free allocated memory in case event queueing fails */
        QCRIL_UIM_SAP_FREE_PTR(original_req_ptr);
      }
    }
  }
} /* qcril_uim_sap_process_message */
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER || FEATURE_QCRIL_UIM_SAP_SERVER_MODE */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_sap_process_response

===========================================================================*/
/*!
    @brief
    This is the SAP response handling in RIL event thread context upon
    receiving QCRIL_EVT_INTERNAL_UIM_SAP_RESP. This is mainly used when an
    error response needs to be sent directly from onRequest().

    @return
    None.

*/
/*=========================================================================*/
void qcril_uim_sap_process_response
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_uim_original_request_type   * orig_request_ptr = NULL;
  qcril_uim_callback_params_type      callback_params;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    return;
  }

  orig_request_ptr = (qcril_uim_original_request_type *)params_ptr->data;
  if (orig_request_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL orig_request_ptr");
    return;
  }

  /* Update callback params */
  memset(&callback_params, 0, sizeof(qcril_uim_callback_params_type));

  callback_params.orig_req_data = orig_request_ptr;
  callback_params.qmi_rsp_data.qmi_err_code = QMI_SERVICE_ERR_INTERNAL;

#if defined (FEATURE_QCRIL_UIM_REMOTE_SERVER) || defined (FEATURE_QCRIL_UIM_SAP_SERVER_MODE)
  /* Send error message back to client */
  switch (orig_request_ptr->request_id)
  {
    case MsgId_RIL_SIM_SAP_CONNECT:
    case MsgId_RIL_SIM_SAP_DISCONNECT:
      callback_params.qmi_rsp_data.rsp_id = QMI_UIM_SRVC_SAP_CONNECTION_RSP_MSG;
      qcril_uim_sap_qmi_handle_sap_connection_resp(&callback_params);
      break;

    case MsgId_RIL_SIM_SAP_APDU:
    case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
    case MsgId_RIL_SIM_SAP_POWER:
    case MsgId_RIL_SIM_SAP_RESET_SIM:
    case MsgId_RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS:
    case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
      callback_params.qmi_rsp_data.rsp_id = QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG;
      qcril_uim_sap_qmi_handle_sap_request_resp(&callback_params);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported MsgId: 0x%x", orig_request_ptr->request_id);
      break;
  }
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER || FEATURE_QCRIL_UIM_SAP_SERVER_MODE */
} /* qcril_uim_sap_process_response */


/*===========================================================================

  FUNCTION:  qcril_qmi_sap_ind_hdlr

===========================================================================*/
/*!
    @brief
    This is the SAP indication callback implementation for the QMI interface.

    @return
    None.

*/
/*=========================================================================*/
void qcril_qmi_sap_ind_hdlr
(
  uim_sap_connection_ind_msg_v01   * ind_data_ptr
)
{
  qcril_instance_id_e_type   instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  if (ind_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL ind_data_ptr");
    return;
  }

  instance_id = qmi_ril_get_process_instance_id();
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR("Invalid value, instance_id: 0x%x", instance_id);
    return;
  }

#if defined (FEATURE_QCRIL_UIM_REMOTE_SERVER) || defined (FEATURE_QCRIL_UIM_SAP_SERVER_MODE)
  /* Call the handling function for this IND */
  qcril_uim_sap_qmi_handle_sap_ind(instance_id, ind_data_ptr);
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER || FEATURE_QCRIL_UIM_SAP_SERVER_MODE */

} /* qcril_qmi_sap_ind_hdlr */


/*===========================================================================

  FUNCTION:  qcril_uim_sap_get_version

===========================================================================*/
/*!
    @brief
    This is the QCRIL UIM SAP API that corresponds to getVersion field in
    RIL_RadioFunctions. It is called from RILD to get a version string for
    the vendor RIL implementation .

    @return
    Null terminated string for the RIL implementation.

*/
/*=========================================================================*/
static const char * qcril_uim_sap_get_version
(
  void
)
{
  char *version = "Qualcomm RIL UIM SAP 1.0";

  QCRIL_LOG_DEBUG( "RIL UIM SAP version %s", version );

  return version;
} /* qcril_uim_sap_get_version */


/*===========================================================================

  FUNCTION:  qcril_uim_sap_on_cancel

===========================================================================*/
/*!
    @brief
    This is the QCRIL UIM SAP API that corresponds to onCancel field in
    RIL_RadioFunctions. It is called from RILD to cancel a specific request
    corresponding to the supplied token. The RIL is supposed to make a best
    effort to cancel the outstanding request, the current design is to take
    no effort & let the current command finish.

    @return
    None.

*/
/*=========================================================================*/
static void qcril_uim_sap_on_cancel
(
  RIL_Token token
)
{
  QCRIL_NOTUSED(token);

  QCRIL_LOG_DEBUG( "qcril_uim_sap_on_cancel, token: 0x%x", token);
} /* qcril_uim_sap_on_cancel */


/*===========================================================================

  FUNCTION:  qcril_uim_sap_on_supports

===========================================================================*/
/*!
    @brief
    This is the QCRIL UIM SAP API that corresponds to onSupports field in
    RIL_RadioFunctions. It is called from RILD to get the status if the
    corresponding API is supported on the SAP specific socket.

    Note: request_id here corresponds to MsgId of SAP protobuf interface.

    @return
    1 if specified request code is supported or 0 if it is not.

*/
/*=========================================================================*/
static int qcril_uim_sap_on_supports
(
  int request_id
)
{
  QCRIL_LOG_DEBUG( "qcril_uim_sap_on_supports, request_id: 0x%x", request_id);
#if defined (FEATURE_QCRIL_UIM_REMOTE_SERVER) || defined (FEATURE_QCRIL_UIM_SAP_SERVER_MODE)
  return qcril_uim_sap_check_supported_messages(request_id);
#else
  return 0;
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER || FEATURE_QCRIL_UIM_SAP_SERVER_MODE */
} /* qcril_uim_sap_on_supports */


/*===========================================================================

  FUNCTION:  qcril_uim_sap_on_state_request

===========================================================================*/
/*!
    @brief
    This is the QCRIL UIM SAP API that corresponds to onStateRequest
    field in RIL_RadioFunctions. It is called from RILD to get the current
    radio state on the SAP specific socket.

    @return
    RIL_RadioState.

*/
/*=========================================================================*/
static RIL_RadioState qcril_uim_sap_on_state_request
(
  void
)
{
  /* This API is not currently called by RILD for the SAP socket */
  return RADIO_STATE_ON;
} /* qcril_uim_sap_on_state_request */


/*===========================================================================

  FUNCTION:  qcril_uim_sap_on_request

===========================================================================*/
/*!
    @brief
    This is the QCRIL UIM SAP request callback implementation that was
    reported earlier via the RIL_UIM_SAP_Init API. It corresponds to onRequest
    field in RIL_RadioFunctions. It is called from RILD to make various SAP
    related requests that come as part of the SAP specific socket.
    RIL_onRequestComplete() may be called from any thread, before or after
    this function returns. Returning from this routine implies QCRIL UIM SAP
    is ready to process another command asynchronously.

    @return
    None.

*/
/*=========================================================================*/
static void qcril_uim_sap_on_request
(
  int        request,
  void     * data,
  size_t     datalen,
  RIL_Token  t
)
{
  qcril_instance_id_e_type instance_id = QCRIL_MAX_INSTANCE_ID;

  /* Get the RILD instance ID */
  instance_id = qmi_ril_get_process_instance_id();
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR("Invalid value, instance_id: 0x%x", instance_id);
    return;
  }

  /* Process message - respective request handler is responsible to send the
     response synchronously or asynchronously */
#if defined (FEATURE_QCRIL_UIM_REMOTE_SERVER) || defined (FEATURE_QCRIL_UIM_SAP_SERVER_MODE)
  qcril_uim_sap_process_message(instance_id, request, t, data, datalen);
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER || FEATURE_QCRIL_UIM_SAP_SERVER_MODE */

} /* qcril_uim_sap_on_request */


/*=========================================================================

  FUNCTION:  RIL_SAP_Init

===========================================================================*/
/*!
    @brief
    Initializes QMI_UIM service for SAP interface. It is called whenever
    RILD starts or modem restarts. The RIL_Env is stored based on instance id.

    @return
    Pointer to RIL_RadioFunctions.
*/
/*=========================================================================*/
const RIL_RadioFunctions     * RIL_SAP_Init
(
  const struct RIL_Env  * env,
  int                     argc,
  char                 ** argv
)
{
  int                      opt             = -1;
  int                      client_id       = 0;
  qcril_instance_id_e_type ril_instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  opt = getopt(argc, argv, "p:d:s:c:");

  switch (opt)
  {
    case 'c':
     client_id = atoi(optarg);
     QCRIL_LOG_INFO( "RIL client opt: %d, running RIL_Init()", client_id);
     break;
    default:
     break;
  }

  switch (client_id)
  {
    case 1:
      ril_instance_id = QCRIL_SECOND_INSTANCE_ID;
      break;
    case 2:
      ril_instance_id = QCRIL_THIRD_INSTANCE_ID;
      break;
    case 0:
    default:
      ril_instance_id = QCRIL_DEFAULT_INSTANCE_ID;
      break;
  }

  QCRIL_LOG_DEBUG( "RILD %d, running RIL_SAP_Init()", ril_instance_id);
  if (ril_instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    return NULL;
  }

  /* Store the pointer based on the instance_id */
  qcril_sap_response_api[ril_instance_id] = (struct RIL_Env *) env;

  return &qcril_sap_request_api[0];
} /* RIL_SAP_Init */

