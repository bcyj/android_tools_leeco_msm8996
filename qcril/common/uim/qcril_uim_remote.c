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
12/14/14   at      Init QMI UIM REMOTE only if feature is defined
09/29/14   at      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_uim_util.h"
#include "qcril_uim_remote.h"
#include "qmi_cci_target_ext.h"
#include "qmi_client.h"
#include "user_identity_module_remote_v01.h"
#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
#include "qcril_uim_remote_client_misc.h"
#include "uim_remote_client.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#endif /* FEATURE_QCRIL_UIM_REMOTE_CLIENT */

/* Global QMI Remote handle */
qmi_client_type            qmi_remote_handle      = NULL;
uint32_t                   qmi_uim_remote_apdu_id = 0;

/*===========================================================================
                        INTERNAL DEFINITIONS AND TYPES
===========================================================================*/

/* QMI init retry related defines */
#define QCRIL_UIM_REMOTE_QMI_INIT_MAX_RETRIES      10
#define QCRIL_UIM_REMOTE_QMI_SVC_TIMEOUT         5000
#define QCRIL_QMI_UIM_REMOTE_FREE_PTR(ptr)                                  \
  if (ptr != NULL)                                                          \
  {                                                                         \
    qcril_free(ptr);                                                        \
    ptr = NULL;                                                             \
  }                                                                         \

/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_remote_set_apdu_id

===========================================================================*/
/*!
    @brief
    Utility routine to store a passed APDU id.

    @return
    void
*/
/*=========================================================================*/
static void qcril_uim_remote_set_apdu_id
(
  uint32_t                          in_apdu_id
)
{
  /* Update the global APDU id */
  QCRIL_LOG_INFO( "Storing apdu_id: 0x%x", in_apdu_id);
  qmi_uim_remote_apdu_id = in_apdu_id;
} /* qcril_uim_remote_set_apdu_id */


/*=========================================================================

  FUNCTION:  qcril_uim_remote_get_apdu_id

===========================================================================*/
/*!
    @brief
    Utility routine to fetch the stored APDU id.

    @return
    void
*/
/*=========================================================================*/
static uint32_t qcril_uim_remote_get_apdu_id
(
  void
)
{
  QCRIL_LOG_INFO( "Returning apdu_id: 0x%x", qmi_uim_remote_apdu_id);
  return qmi_uim_remote_apdu_id;
} /* qcril_uim_remote_get_apdu_id */


/*=========================================================================

  FUNCTION:  qcril_uim_remote_convert_slot_id_to_slot_type

===========================================================================*/
/*!
    @brief
    Routine to convert a RIL slot id to QMI slot enum type.

    @return
    Boolean indicating the outcome of the request.
*/
/*=========================================================================*/
static boolean qcril_uim_remote_convert_slot_id_to_slot_type
(
  uint32_t                          slot_id,
  uim_remote_slot_type_enum_v01   * qmi_slot_type_ptr
)
{
  if (qmi_slot_type_ptr == NULL)
  {
    return FALSE;
  }

  switch (slot_id)
  {
     case 0:
       *qmi_slot_type_ptr = UIM_REMOTE_SLOT_1_V01;
       break;

     case 1:
       *qmi_slot_type_ptr = UIM_REMOTE_SLOT_2_V01;
       break;

    case 2:
      *qmi_slot_type_ptr = UIM_REMOTE_SLOT_3_V01;
      break;

    default:
      QCRIL_LOG_ERROR( "Invalid slot_id returned: 0x%x\n", slot_id);
      return FALSE;
  }

  return TRUE;
} /* qcril_uim_remote_convert_slot_id_to_slot_type */


/*=========================================================================

  FUNCTION:  qcril_uim_remote_copy_indication

===========================================================================*/
/*!
    @brief
    Makes a copy of the indication received from QMI UIM REMOTE.

    @return
    Pointer to copy of the indication
*/
/*=========================================================================*/
static qcril_uim_remote_ind_params_type  * qcril_uim_remote_copy_indication
(
  qmi_client_type                user_handle_ptr,
  unsigned int                   msg_id,
  unsigned char                * qmi_rmt_ind_ptr,
  unsigned int                   qmi_rmt_ind_len,
  uint32_t                     * out_len_ptr
)
{
  void                             * decoded_payload_ptr  = NULL;
  uint32_t                           decoded_payload_len  = 0;
  qcril_uim_remote_ind_params_type * out_ptr              = NULL;
  qmi_client_error_type              qmi_err              = QMI_INTERNAL_ERR;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if ((user_handle_ptr == NULL) ||
      (qmi_rmt_ind_ptr == NULL) ||
      (out_len_ptr     == NULL))
  {
    QCRIL_LOG_ERROR( "%s\n", "Invalid input, cannot proceed");
    return NULL;
  }

  /* First decode the message payload from QCCI */
  qmi_idl_get_message_c_struct_len(uim_remote_get_service_object_v01(),
                                   QMI_IDL_INDICATION,
                                   msg_id,
                                   &decoded_payload_len);
  if (decoded_payload_len == 0)
  {
    QCRIL_LOG_ERROR("%s: Failed to find decoded_payload_len");
    return NULL;
  }

  /* Allocate decoded payload buffer */
  decoded_payload_ptr = qcril_malloc(decoded_payload_len);
  if (decoded_payload_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Failed to allocate payload ptr, payload len: 0x%x\n",
                    decoded_payload_len);
    return NULL;
  }

  /* Decode the Indication payload */
  qmi_err = qmi_client_message_decode(user_handle_ptr,
                                      QMI_IDL_INDICATION,
                                      msg_id,
                                      qmi_rmt_ind_ptr,
                                      qmi_rmt_ind_len,
                                      decoded_payload_ptr,
                                      decoded_payload_len);
  if (qmi_err != QMI_NO_ERR)
  {
    QCRIL_LOG_ERROR("Failed to decode Indication: 0x%x, qmi_err: 0x%x", msg_id, qmi_err);
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(decoded_payload_ptr);
    return NULL;
  }

  /* Note: out_ptr and decoded_payload_ptr will be freed after
     processing the event in qcril_gstk_qmi_process_qmi_indication */
  out_ptr = qcril_malloc(sizeof(qcril_uim_remote_ind_params_type));
  if (out_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "out_ptr alloc failed");
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(decoded_payload_ptr);
    return NULL;
  }

  /* Initialize the payload data & assign the message data pointer */
  *out_len_ptr = sizeof(qcril_uim_remote_ind_params_type);
  memset(out_ptr, 0, sizeof(qcril_uim_remote_ind_params_type));
  out_ptr->handle      = user_handle_ptr;
  out_ptr->msg_id      = msg_id;
  out_ptr->msg_ptr     = decoded_payload_ptr;
  out_ptr->modem_id    = QCRIL_DEFAULT_MODEM_ID;
  out_ptr->instance_id = qmi_ril_get_process_instance_id();

  return out_ptr;
} /* qcril_uim_remote_copy_indication */


/*=========================================================================

  FUNCTION:  qcril_uim_remote_copy_callback

===========================================================================*/
/*!
    @brief
    Makes a copy of the response received from QMI UIM REMOTE.

    @return
    Pointer to copy of the response
*/
/*=========================================================================*/
static qcril_uim_remote_cb_params_type * qcril_uim_remote_copy_callback
(
  unsigned int                         msg_id,
  void                               * resp_data_ptr,
  void                               * resp_cb_data_ptr,
  qmi_client_error_type                transp_err,
  uint32_t                           * out_len_ptr
)
{
  qcril_uim_remote_cb_params_type   * out_ptr  = NULL;

  /* Sanity check */
  if ((resp_data_ptr    == NULL) ||
      (resp_cb_data_ptr == NULL) ||
      (out_len_ptr      == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot copy");
    return NULL;
  }

  /* Update size & allocate buffer */
  *out_len_ptr = sizeof(qcril_uim_remote_cb_params_type);
  out_ptr = (qcril_uim_remote_cb_params_type*)qcril_malloc(*out_len_ptr);
  if (out_ptr == NULL)
  {
    return NULL;
  }

  memset(out_ptr, 0, *out_len_ptr);

  /* Copy the response parameters */
  out_ptr->msg_id        = msg_id;
  out_ptr->msg_ptr       = resp_data_ptr;
  out_ptr->transp_err    = transp_err;
  out_ptr->orig_req_ptr = (qcril_uim_original_request_type*)resp_cb_data_ptr;

  return out_ptr;
} /* qcril_uim_remote_copy_callback */


#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
/*===========================================================================

  FUNCTION:  qcril_uim_remote_event_resp

===========================================================================*/
/*!
    @brief
    Handles the response of event request

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_event_resp
(
  qcril_uim_original_request_type      * original_req_ptr,
  uim_remote_event_resp_msg_v01        * resp_msg_ptr
)
{
  RIL_Token                                         token;
  com_qualcomm_uimremoteclient_UimRemoteEventResp   event_resp;

  if ((original_req_ptr == NULL) || (resp_msg_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "NULL original_req_ptr or resp_msg_ptr");
    return;
  }

  /* Retreive token */
  token = (RIL_Token)original_req_ptr->token;

  QCRIL_LOG_DEBUG( "qcril_uim_remote_event_resp: token=%d, result=0x%x, error=0x%x",
                    qcril_log_get_token_id(token),
                    resp_msg_ptr->resp.result,
                    resp_msg_ptr->resp.error);

  /* Update the response error code */
  memset(&event_resp, 0, sizeof(com_qualcomm_uimremoteclient_UimRemoteEventResp));
  event_resp.response = (resp_msg_ptr->resp.result == QMI_RESULT_SUCCESS_V01) ?
                          com_qualcomm_uimremoteclient_UimRemoteEventResp_Status_UIM_REMOTE_SUCCESS :
                          com_qualcomm_uimremoteclient_UimRemoteEventResp_Status_UIM_REMOTE_FAILURE;

  /* Generate response */
  qcril_uim_remote_client_socket_send(TRUE,
                                      token,
                                      com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_RESPONSE,
                                      qcril_uim_remote_client_map_event_to_request(original_req_ptr->request_id),
                                      TRUE,
                                      com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_SUCCESS,
                                      &event_resp,
                                      sizeof(com_qualcomm_uimremoteclient_UimRemoteEventResp));
} /* qcril_uim_remote_event_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_apdu_resp

===========================================================================*/
/*!
    @brief
    Handles the response of APDU request

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_apdu_resp
(
  qcril_uim_original_request_type      * original_req_ptr,
  uim_remote_event_resp_msg_v01        * resp_msg_ptr
)
{
  RIL_Token                                       token;
  com_qualcomm_uimremoteclient_UimRemoteApduResp  apdu_resp;

  if ((original_req_ptr == NULL) || (resp_msg_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "NULL original_req_ptr or resp_msg_ptr");
    return;
  }

  /* Retreive token */
  token = (RIL_Token)original_req_ptr->token;

  QCRIL_LOG_DEBUG( "qcril_uim_remote_event_resp: token=%d, result=0x%x, error=0x%x",
                    qcril_log_get_token_id(token),
                    resp_msg_ptr->resp.result,
                    resp_msg_ptr->resp.error);

  /* Update the response error code */
  memset(&apdu_resp, 0, sizeof(com_qualcomm_uimremoteclient_UimRemoteApduResp));
  apdu_resp.status = (resp_msg_ptr->resp.result == QMI_RESULT_SUCCESS_V01) ?
                        com_qualcomm_uimremoteclient_UimRemoteApduResp_Status_UIM_REMOTE_SUCCESS :
                        com_qualcomm_uimremoteclient_UimRemoteApduResp_Status_UIM_REMOTE_FAILURE;

  /* Generate response */
  qcril_uim_remote_client_socket_send(TRUE,
                                      token,
                                      com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_RESPONSE,
                                      qcril_uim_remote_client_map_event_to_request(original_req_ptr->request_id),
                                      TRUE,
                                      com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_SUCCESS,
                                      &apdu_resp,
                                      sizeof(com_qualcomm_uimremoteclient_UimRemoteApduResp));
} /* qcril_uim_remote_apdu_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_handle_apdu_ind

===========================================================================*/
/*!
    @brief
    Handles the APDU indication

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_handle_apdu_ind
(
  uim_remote_apdu_ind_msg_v01        * ind_msg_ptr
)
{
  qcril_binary_data_type                           apdu_data;
  com_qualcomm_uimremoteclient_UimRemoteApduInd    apdu_ind;

  if (ind_msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL ind_msg_ptr");
    return;
  }

  if (ind_msg_ptr->command_apdu_len == 0)
  {
    QCRIL_LOG_ERROR("%s", "Zero command_apdu_len");
    return;
  }

  QCRIL_LOG_INFO( "%s slot: 0x%x, apdu_id:0x%x",
                  __FUNCTION__,
                  ind_msg_ptr->slot,
                  ind_msg_ptr->apdu_id);

  /* Store the APDU ID */
  qcril_uim_remote_set_apdu_id(ind_msg_ptr->apdu_id);

  /* Update the indication data */
  memset(&apdu_ind, 0, sizeof(com_qualcomm_uimremoteclient_UimRemoteApduInd));
  memset(&apdu_data, 0, sizeof(qcril_binary_data_type));
  if (ind_msg_ptr->command_apdu_len > 0)
  {
    apdu_data.len  = ind_msg_ptr->command_apdu_len;
    apdu_data.data = ind_msg_ptr->command_apdu;
  }

  apdu_ind.apduCommand.arg = &apdu_data;

  qcril_uim_remote_client_socket_send(FALSE,
                                      0,
                                      com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION,
                                      com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_APDU,
                                      FALSE,
                                      com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_SUCCESS,
                                      &apdu_ind,
                                      sizeof(com_qualcomm_uimremoteclient_UimRemoteApduInd));
} /* qcril_uim_remote_handle_apdu_ind */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_handle_connect_ind

===========================================================================*/
/*!
    @brief
    Handles the Connect indication

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_handle_connect_ind
(
  uim_remote_connect_ind_msg_v01        * ind_msg_ptr
)
{
  QCRIL_LOG_INFO( "%s slot: 0x%x", __FUNCTION__, ind_msg_ptr->slot);

  qcril_uim_remote_client_socket_send_empty_payload_unsol_resp(
     com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_CONNECT);
} /* qcril_uim_remote_handle_connect_ind */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_handle_disconnect_ind

===========================================================================*/
/*!
    @brief
    Handles the Disconnect indication

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_handle_disconnect_ind
(
  uim_remote_disconnect_ind_msg_v01        * ind_msg_ptr
)
{
  QCRIL_LOG_INFO( "%s slot: 0x%x", __FUNCTION__, ind_msg_ptr->slot);

  qcril_uim_remote_client_socket_send_empty_payload_unsol_resp(
   com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_DISCONNECT);
} /* qcril_uim_remote_handle_disconnect_ind */



/*===========================================================================

  FUNCTION:  qcril_uim_remote_handle_pup_ind

===========================================================================*/
/*!
    @brief
    Handles the power up indication

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_handle_pup_ind
(
  uim_remote_card_power_up_ind_msg_v01        * ind_msg_ptr
)
{
  com_qualcomm_uimremoteclient_UimRemotePowerUpInd    pup_ind;

  if (ind_msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL ind_msg_ptr");
    return;
  }

  QCRIL_LOG_INFO( "%s slot: 0x%x", __FUNCTION__, ind_msg_ptr->slot);

  /* Update the indication data */
  memset(&pup_ind, 0, sizeof(com_qualcomm_uimremoteclient_UimRemotePowerUpInd));

  if (ind_msg_ptr->response_timeout_valid)
  {
    pup_ind.timeout = (int32_t)ind_msg_ptr->response_timeout;
    pup_ind.has_timeout = TRUE;
  }

  if (ind_msg_ptr->voltage_class_valid)
  {
    pup_ind.has_voltageclass = TRUE;
    switch (ind_msg_ptr->voltage_class)
    {
      case UIM_REMOTE_VOLTAGE_CLASS_C_LOW_V01:
        pup_ind.voltageclass = com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_C_LOW;
        break;

      case UIM_REMOTE_VOLTAGE_CLASS_C_V01:
        pup_ind.voltageclass = com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_C;
        break;

      case UIM_REMOTE_VOLTAGE_CLASS_C_HIGH_V01:
        pup_ind.voltageclass = com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_C_HIGH;
        break;

      case UIM_REMOTE_VOLTAGE_CLASS_B_LOW_V01:
        pup_ind.voltageclass = com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_B_LOW;
        break;

      case UIM_REMOTE_VOLTAGE_CLASS_B_V01:
        pup_ind.voltageclass = com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_B;
        break;

      case UIM_REMOTE_VOLTAGE_CLASS_B_HIGH_V01:
        pup_ind.voltageclass = com_qualcomm_uimremoteclient_UimRemotePowerUpInd_VoltageClass_UIM_REMOTE_VOLTAGE_CLASS_B_HIGH;
        break;

      default:
        /* Skip this field for unsupported enums */
        pup_ind.has_voltageclass = FALSE;
        break;
    }
  }

  /* Send the unsol response */
  qcril_uim_remote_client_socket_send(FALSE,
                                      0,
                                      com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION,
                                      com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_POWER_UP,
                                      FALSE,
                                      com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_SUCCESS,
                                      &pup_ind,
                                      sizeof(com_qualcomm_uimremoteclient_UimRemotePowerUpInd));
} /* qcril_uim_remote_handle_pup_ind */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_handle_pdown_ind

===========================================================================*/
/*!
    @brief
    Handles the power down indication

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_handle_pdown_ind
(
  uim_remote_card_power_down_ind_msg_v01        * ind_msg_ptr
)
{
  com_qualcomm_uimremoteclient_UimRemotePowerDownInd    pdown_ind;

  if (ind_msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL ind_msg_ptr");
    return;
  }

  QCRIL_LOG_INFO( "%s slot: 0x%x", __FUNCTION__, ind_msg_ptr->slot);

  /* Update the indication data */
  memset(&pdown_ind, 0, sizeof(com_qualcomm_uimremoteclient_UimRemotePowerDownInd));

  if (ind_msg_ptr->mode_valid)
  {
    pdown_ind.has_mode = TRUE;
    switch (ind_msg_ptr->mode)
    {
      case UIM_REMOTE_POWER_DOWN_TELECOM_INTERFACE_V01:
        pdown_ind.mode = com_qualcomm_uimremoteclient_UimRemotePowerDownInd_PowerDownMode_UIM_REMOTE_PDOWN_TELECOM_INTERFACE;
        break;

      case UIM_REMOTE_POWER_DOWN_CARD_V01:
        pdown_ind.mode = com_qualcomm_uimremoteclient_UimRemotePowerDownInd_PowerDownMode_UIM_REMOTE_PDOWN_CARD;
        break;

      default:
        /* Skip this field for unsupported enums */
        pdown_ind.has_mode = FALSE;
        break;
    }
  }

  /* Send the unsol response */
  qcril_uim_remote_client_socket_send(FALSE,
                                      0,
                                      com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_INDICATION,
                                      com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_POWER_DOWN,
                                      FALSE,
                                      com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_SUCCESS,
                                      &pdown_ind,
                                      sizeof(com_qualcomm_uimremoteclient_UimRemotePowerDownInd));
} /* qcril_uim_remote_handle_pdown_ind */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_handle_reset_ind

===========================================================================*/
/*!
    @brief
    Handles the power down indication

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_handle_reset_ind
(
  uim_remote_card_reset_ind_msg_v01        * ind_msg_ptr
)
{
  QCRIL_LOG_INFO( "%s slot: 0x%x", __FUNCTION__, ind_msg_ptr->slot);

  qcril_uim_remote_client_socket_send_empty_payload_unsol_resp(
    com_qualcomm_uimremoteclient_MessageId_UIM_REMOTE_RESET);
} /* qcril_uim_remote_handle_reset_ind */
#endif /* FEATURE_QCRIL_UIM_REMOTE_CLIENT */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_remote_process_qmi_indication

===========================================================================*/
void qcril_uim_remote_process_qmi_indication
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_uim_remote_ind_params_type  * ind_params_ptr = NULL;

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    return;
  }

  ind_params_ptr = (qcril_uim_remote_ind_params_type*)params_ptr->data;
  if (ind_params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL ind_params_ptr");
    return;
  }

  if (ind_params_ptr->msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL msg_ptr, cannot process response");
    goto clean_up;
  }

  if ((ind_params_ptr->instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (ind_params_ptr->modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, modem_id: 0x%x",
                    ind_params_ptr->instance_id,
                    ind_params_ptr->modem_id);
    goto clean_up;
  }

#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
  /* Process respective indications */
  switch(ind_params_ptr->msg_id)
  {
    case QMI_UIM_REMOTE_APDU_IND_V01:
      qcril_uim_remote_handle_apdu_ind(ind_params_ptr->msg_ptr);
      break;
    case QMI_UIM_REMOTE_CONNECT_IND_V01:
      qcril_uim_remote_handle_connect_ind(ind_params_ptr->msg_ptr);
      break;
    case QMI_UIM_REMOTE_DISCONNECT_IND_V01:
      qcril_uim_remote_handle_disconnect_ind(ind_params_ptr->msg_ptr);
      break;
    case QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01:
      qcril_uim_remote_handle_pup_ind(ind_params_ptr->msg_ptr);
      break;
    case QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01:
      qcril_uim_remote_handle_pdown_ind(ind_params_ptr->msg_ptr);
      break;
    case QMI_UIM_REMOTE_CARD_RESET_IND_V01:
      qcril_uim_remote_handle_reset_ind(ind_params_ptr->msg_ptr);
      break;
    default:
      /* This shouldn't happen since we never post for these msg ids */
      QCRIL_LOG_ERROR("Unsupported QMI UIM REMOTE indication: 0x%x", ind_params_ptr->msg_id);
      break;
  }
#endif /* FEATURE_QCRIL_UIM_REMOTE_CLIENT */

clean_up:
  /* Free memory allocated previously. Note that all the frees are
     performed here rather than at individual handler functions */
  QCRIL_QMI_UIM_REMOTE_FREE_PTR(ind_params_ptr->msg_ptr);
  QCRIL_QMI_UIM_REMOTE_FREE_PTR(ind_params_ptr);
} /* qcril_uim_remote_process_qmi_indication */


/*=========================================================================

  FUNCTION:  qcril_uim_remote_ind_callback

===========================================================================*/
/*!
    @brief
    Callback implementation for the QMI UIM REMOTE indications. This will be
    called by QMI FW when the QMI UIM REMOTE in the modem sends indications.

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_ind_callback
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  unsigned char                * qmi_rmt_ind_ptr,
  unsigned int                   qmi_rmt_ind_len,
  void                         * ind_cb_data_ptr
)
{
  uint32_t                            ind_params_len = 0;
  qcril_uim_remote_ind_params_type  * ind_params_ptr = NULL;
  IxErrnoType                         result         = E_FAILURE;

  QCRIL_NOTUSED(ind_cb_data_ptr);

  QCRIL_LOG_INFO("qcril_uim_remote_ind_callback, msg_id: 0x%x", msg_id);

  if ((qmi_rmt_ind_ptr == NULL) || (qmi_rmt_ind_len == 0))
  {
    QCRIL_LOG_ERROR("%s","NULL ind_buf_ptr or zero qmi_rmt_ind_len");
    return;
  }

  /* Process only the supported IND messages */
  switch (msg_id)
  {
    case QMI_UIM_REMOTE_APDU_IND_V01:
    case QMI_UIM_REMOTE_CONNECT_IND_V01:
    case QMI_UIM_REMOTE_DISCONNECT_IND_V01:
    case QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01:
    case QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01:
    case QMI_UIM_REMOTE_CARD_RESET_IND_V01:
      ind_params_ptr = qcril_uim_remote_copy_indication(user_handle,
                                                        msg_id,
                                                        qmi_rmt_ind_ptr,
                                                        qmi_rmt_ind_len,
                                                        &ind_params_len);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported QMI UIM REMOTE indication: 0x%x", msg_id);
      break;
  }

  if ((ind_params_ptr == NULL) || (ind_params_len == 0))
  {
    QCRIL_LOG_ERROR("Error copying the indication msg_id: 0x%x", msg_id);
    return;
  }

  /* Post the event to process the indication callback.
     Note: msg_ptr will be freed in the indication handler */
  QCRIL_LOG_INFO( "%s qcril_event_queue ", __FUNCTION__);
  result = qcril_event_queue( ind_params_ptr->instance_id,
                              ind_params_ptr->modem_id,
                              QCRIL_DATA_NOT_ON_STACK,
                              QCRIL_EVT_UIM_RMT_QMI_INDICATION_CALLBACK,
                              (void *)ind_params_ptr,
                              ind_params_len,
                              NULL);
  if (result != E_SUCCESS)
  {
    QCRIL_LOG_ERROR( " qcril_event_queue failed, result: 0x%x\n", result);

    /* Free allocated memory in case event queueing fails */
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(ind_params_ptr->msg_ptr);
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(ind_params_ptr);
  }
} /* qcril_uim_remote_ind_callback */


/*=========================================================================

  FUNCTION:  qcril_uim_remote_process_qmi_callback

===========================================================================*/
void qcril_uim_remote_process_qmi_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_uim_remote_cb_params_type  * callback_params_ptr = NULL;

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    return;
  }

  callback_params_ptr = (qcril_uim_remote_cb_params_type*)params_ptr->data;
  if (callback_params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL callback_params_ptr");
    return;
  }

  if (callback_params_ptr->msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL msg_ptr, cannot process response");
    goto clean_up;
  }

  if (callback_params_ptr->orig_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL orig_req_data, cannot process response");
    goto clean_up;
  }

  if ((callback_params_ptr->orig_req_ptr->instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (callback_params_ptr->orig_req_ptr->modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, modem_id: 0x%x",
                    callback_params_ptr->orig_req_ptr->instance_id,
                    callback_params_ptr->orig_req_ptr->modem_id);
    goto clean_up;
  }

  QCRIL_LOG_DEBUG("%s: Response for msg_id: 0x%x, token=%d, transp_err: 0x%x",
                  __FUNCTION__,
                  callback_params_ptr->msg_id,
                  qcril_log_get_token_id((RIL_Token)callback_params_ptr->orig_req_ptr->token),
                  callback_params_ptr->transp_err);

#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
  switch(callback_params_ptr->msg_id)
  {
    case QMI_UIM_REMOTE_EVENT_RESP_V01:
      qcril_uim_remote_event_resp(callback_params_ptr->orig_req_ptr,
                                  callback_params_ptr->msg_ptr);
      break;

    case QMI_UIM_REMOTE_APDU_RESP_V01:
      qcril_uim_remote_apdu_resp(callback_params_ptr->orig_req_ptr,
                                 callback_params_ptr->msg_ptr);
      break;

    default:
      /* This shouldn't happen since we never post for these msg ids */
      QCRIL_LOG_ERROR("Unsupported QMI UIM REMOTE response: 0x%x",
                      callback_params_ptr->msg_id);
      break;
  }
#endif /* FEATURE_QCRIL_UIM_REMOTE_CLIENT */

clean_up:
  /* Free memory allocated previously. Note that all the frees are
     performed here rather than at individual handler functions */
  QCRIL_QMI_UIM_REMOTE_FREE_PTR(callback_params_ptr->orig_req_ptr);
  QCRIL_QMI_UIM_REMOTE_FREE_PTR(callback_params_ptr->msg_ptr);
  QCRIL_QMI_UIM_REMOTE_FREE_PTR(callback_params_ptr);
} /* qcril_uim_remote_process_qmi_callback */


/*=========================================================================

  FUNCTION:  qcril_uim_remote_cmd_callback

===========================================================================*/
/*!
    @brief
    Callback implementation for the QMI UIM REMOTE commands.

    @return
    None
*/
/*=========================================================================*/
static void qcril_uim_remote_cmd_callback
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                         * qmi_rmt_rsp_ptr,
  unsigned int                   qmi_rmt_rsp_len,
  void                         * resp_cb_data_ptr,
  qmi_client_error_type          transp_err
)
{
  uint32_t                          params_len = 0;
  qcril_uim_remote_cb_params_type * params_ptr = NULL;
  IxErrnoType                       result     = E_FAILURE;

  QCRIL_NOTUSED(user_handle);
  QCRIL_NOTUSED(qmi_rmt_rsp_len);

  QCRIL_LOG_INFO( "qcril_uim_remote_cmd_callback: msg_id = 0x%x ", msg_id);

  /* Sanity check */
  if ((qmi_rmt_rsp_ptr == NULL) || (resp_cb_data_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "NULL qmi_rmt_rsp_ptr or resp_cb_data_ptr");
    goto clean_up;
  }

  /* Process only the supported RESP messages */
  switch (msg_id)
  {
    case QMI_UIM_REMOTE_EVENT_RESP_V01:
    case QMI_UIM_REMOTE_APDU_RESP_V01:
      params_ptr = qcril_uim_remote_copy_callback(msg_id,
                                                  qmi_rmt_rsp_ptr,
                                                  resp_cb_data_ptr,
                                                  transp_err,
                                                  &params_len);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported QMI UIM REMOTE response: 0x%x", msg_id);
      break;
  }

  if ((params_ptr == NULL) || (params_len == 0))
  {
    QCRIL_LOG_ERROR("Error copying the response msg_id: 0x%x", msg_id);
    goto clean_up;
  }

  /* Post the event to process the response callback.
     Note: Upon successful posting of the event, necessary pointers will
     be freed in the response handler */
  QCRIL_LOG_INFO( "%s qcril_event_queue ", __FUNCTION__);
  result = qcril_event_queue( params_ptr->orig_req_ptr->instance_id,
                              params_ptr->orig_req_ptr->modem_id,
                              QCRIL_DATA_NOT_ON_STACK,
                              QCRIL_EVT_UIM_RMT_QMI_COMMAND_CALLBACK,
                              (void *)params_ptr,
                              params_len,
                              NULL);
  if (result == E_SUCCESS)
  {
    return;
  }

  QCRIL_LOG_ERROR( " qcril_event_queue failed, result: 0x%x\n", result);

clean_up:
  /* Free allocated pointers only if event queueing fails */
  QCRIL_QMI_UIM_REMOTE_FREE_PTR(resp_cb_data_ptr);
  QCRIL_QMI_UIM_REMOTE_FREE_PTR(qmi_rmt_rsp_ptr);
  QCRIL_QMI_UIM_REMOTE_FREE_PTR(params_ptr);
} /* qcril_uim_remote_cmd_callback */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_client_request_event

===========================================================================*/
/*!
    @brief
    Handles MessageId_UIM_REMOTE_EVENT request from the client.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_remote_client_request_event
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
  qmi_txn_handle                       txn_handle;
  int                                  qmi_err_code    = 0;
  uim_remote_slot_type_enum_v01        uim_remote_slot = UIM_REMOTE_SLOT_NOT_APPLICABLE_V01;
  uim_remote_event_req_msg_v01       * qmi_rmt_req_ptr = NULL;
  uim_remote_event_resp_msg_v01      * qmi_rmt_rsp_ptr = NULL;
  qcril_uim_original_request_type    * orig_req_ptr    = NULL;
  com_qualcomm_uimremoteclient_Error   pb_err_code     = com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_GENERIC_FAILURE;
  com_qualcomm_uimremoteclient_UimRemoteEventReq  * in_ptr = NULL;
  com_qualcomm_uimremoteclient_UimRemoteEventResp   event_resp;

  /* Sanity check */
  if(params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    return;
  }

  /* Parse input info */
  in_ptr = (com_qualcomm_uimremoteclient_UimRemoteEventReq *)(params_ptr->data);
  if(in_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL input pointer");
    goto report_error;
  }

  QCRIL_LOG_INFO("qcril_uim_remote_client_request_event, event: 0x%x, instance_id: 0x%x)\n",
                 in_ptr->event, params_ptr->instance_id);

  /* Find out what slot it is */
  if (!qcril_uim_remote_convert_slot_id_to_slot_type(qmi_ril_get_sim_slot(), &uim_remote_slot))
  {
    return;
  }

  /* Allocate & fill up QMI Remote request */
  qmi_rmt_req_ptr = (uim_remote_event_req_msg_v01 *) qcril_malloc(sizeof(uim_remote_event_req_msg_v01));
  if (qmi_rmt_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for qmi_rmt_req_ptr");
    goto report_error;
  }

  memset(qmi_rmt_req_ptr, 0, sizeof(uim_remote_event_req_msg_v01));
  qmi_rmt_req_ptr->event_info.slot = uim_remote_slot;
  switch (in_ptr->event)
  {
    case com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CONN_UNAVAILABLE:
      qmi_rmt_req_ptr->event_info.event = UIM_REMOTE_CONNECTION_UNAVAILABLE_V01;
      break;
    case com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CONN_AVAILABLE:
       qmi_rmt_req_ptr->event_info.event = UIM_REMOTE_CONNECTION_AVAILABLE_V01;
      break;
    case com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_INSERTED:
      qmi_rmt_req_ptr->event_info.event = UIM_REMOTE_CARD_INSERTED_V01;
      break;
    case com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_REMOVED:
      qmi_rmt_req_ptr->event_info.event = UIM_REMOTE_CARD_REMOVED_V01;
      break;
    case com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_ERROR:
      qmi_rmt_req_ptr->event_info.event = UIM_REMOTE_CARD_ERROR_V01;
      break;
    case com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_RESET:
      qmi_rmt_req_ptr->event_info.event = UIM_REMOTE_CARD_RESET_V01;
      break;
    case com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_WAKEUP:
      qmi_rmt_req_ptr->event_info.event = UIM_REMOTE_CARD_WAKEUP_V01;
      break;
    default:
      QCRIL_LOG_ERROR("Unsupported event: 0x%x", in_ptr->event);
      pb_err_code = com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_NOT_SUPPORTED;
      goto report_error;
  }

  /* Add ATR if needed */
  if ((in_ptr->event == com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_INSERTED)||
      (in_ptr->event == com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_RESET))
  {
    qcril_binary_data_type  * atr_data_ptr = in_ptr->atr.arg;
    if ((atr_data_ptr       != NULL) &&
        (atr_data_ptr->data != NULL) &&
        (atr_data_ptr->len  > 0)     &&
        (atr_data_ptr->len  <= QMI_UIM_REMOTE_MAX_ATR_LEN_V01))
    {
      qmi_rmt_req_ptr->atr_valid = 0x01;
      qmi_rmt_req_ptr->atr_len = atr_data_ptr->len;
      memcpy(qmi_rmt_req_ptr->atr, atr_data_ptr->data, atr_data_ptr->len);
    }
  }

  /* Add wakeup support TLV if needed */
  if (in_ptr->has_wakeup_support)
  {
    qmi_rmt_req_ptr->wakeup_support_valid = 0x01;
    qmi_rmt_req_ptr->wakeup_support = in_ptr->wakeup_support ? 0x01 : 0x00;
  }

  /* Add Error code TLV if needed */
  if ((in_ptr->event == com_qualcomm_uimremoteclient_UimRemoteEventReq_Event_UIM_REMOTE_STATUS_CARD_ERROR) &&
      (in_ptr->has_error_code))
  {
    qmi_rmt_req_ptr->error_cause_valid = 0x01;
    switch (in_ptr->error_code)
    {
      case com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_UNKNOWN:
        qmi_rmt_req_ptr->error_cause = UIM_REMOTE_CARD_ERROR_UNKNOWN_ERROR_V01;
        break;
      case com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_NO_LINK_EST:
         qmi_rmt_req_ptr->error_cause = UIM_REMOTE_CARD_ERROR_NO_LINK_ESTABLISHED_V01;
        break;
      case com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_CMD_TIMEOUT:
        qmi_rmt_req_ptr->error_cause = UIM_REMOTE_CARD_ERROR_COMMAND_TIMEOUT_V01;
        break;
      case com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_POWER_DOWN:
        qmi_rmt_req_ptr->error_cause = UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_V01;
        break;
      case com_qualcomm_uimremoteclient_UimRemoteEventReq_ErrorCause_UIM_REMOTE_CARD_ERROR_POWER_DOWN_TELECOM:
        qmi_rmt_req_ptr->error_cause = UIM_REMOTE_CARD_ERROR_DUE_TO_POWER_DOWN_TELECOM_V01;
        break;
      default:
        QCRIL_LOG_ERROR("Unsupported error_code: 0x%x", in_ptr->error_code);
        pb_err_code = com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_NOT_SUPPORTED;
        goto report_error;
    }
  }

  /* Allocate original request */
  orig_req_ptr = qcril_uim_allocate_orig_request(params_ptr->instance_id,
                                                 QCRIL_MAX_MODEM_ID - 1,
                                                 (RIL_Token)params_ptr->t,
                                                 params_ptr->event_id,
                                                 0);
  if (orig_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for orig_req_ptr");
    goto report_error;
  }

  /* Allocate response pointer since it is an async command */
  qmi_rmt_rsp_ptr = (uim_remote_event_resp_msg_v01 *)
                      qcril_malloc(sizeof(uim_remote_event_resp_msg_v01));
  if (qmi_rmt_rsp_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for qmi_rmt_rsp_ptr");
    goto report_error;
  }

  memset(qmi_rmt_rsp_ptr, 0x00, sizeof(uim_remote_event_resp_msg_v01));

  /* Proceed with Event request */
  QCRIL_LOG_QMI( QCRIL_MAX_MODEM_ID - 1, "qmi_uim_remote_service", "event req" );
  qmi_err_code = qmi_client_send_msg_async(
                   qmi_remote_handle,
                   QMI_UIM_REMOTE_EVENT_REQ_V01,
                   (void *) qmi_rmt_req_ptr,
                   sizeof(uim_remote_event_req_msg_v01),
                   (void *) qmi_rmt_rsp_ptr,
                   sizeof(uim_remote_event_resp_msg_v01),
                   qcril_uim_remote_cmd_callback,
                   orig_req_ptr,
                   &txn_handle);
  /* On successful API call, free only the request */
  if (qmi_err_code == 0)
  {
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(qmi_rmt_req_ptr);
    return;
  }

report_error:
    /* In any case of error, check & free all the allocated pointers */
    QCRIL_LOG_ERROR("Error in qcril_uim_remote_client_request_event, qmi_err_code: 0x%x", qmi_err_code);
    memset(&event_resp, 0, sizeof(com_qualcomm_uimremoteclient_UimRemoteEventResp));
    event_resp.response = com_qualcomm_uimremoteclient_UimRemoteEventResp_Status_UIM_REMOTE_FAILURE;
    qcril_uim_remote_client_socket_send(TRUE,
                                        (RIL_Token)params_ptr->t,
                                        com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_RESPONSE,
                                        qcril_uim_remote_client_map_event_to_request(params_ptr->event_id),
                                        TRUE,
                                        com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_GENERIC_FAILURE,
                                        &event_resp,
                                        sizeof(com_qualcomm_uimremoteclient_UimRemoteEventResp));

    /* Clean up any original request if allocated */
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(orig_req_ptr);
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(qmi_rmt_req_ptr);
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(qmi_rmt_rsp_ptr);
#endif /* FEATURE_QCRIL_UIM_REMOTE_CLIENT */
} /* qcril_uim_remote_client_request_event */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_client_request_apdu

===========================================================================*/
/*!
    @brief
    Handles MessageId_UIM_REMOTE_APDU request from the client.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_remote_client_request_apdu
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
  qmi_txn_handle                       txn_handle;
  int                                  qmi_err_code    = 0;
  qcril_binary_data_type             * apdu_data_ptr   = NULL;
  uim_remote_slot_type_enum_v01        uim_remote_slot = UIM_REMOTE_SLOT_NOT_APPLICABLE_V01;
  uim_remote_apdu_req_msg_v01        * qmi_rmt_req_ptr = NULL;
  uim_remote_apdu_resp_msg_v01       * qmi_rmt_rsp_ptr = NULL;
  qcril_uim_original_request_type    * orig_req_ptr    = NULL;
  com_qualcomm_uimremoteclient_Error   pb_err_code     = com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_GENERIC_FAILURE;
  com_qualcomm_uimremoteclient_UimRemoteApduReq  * in_ptr = NULL;
  com_qualcomm_uimremoteclient_UimRemoteApduResp   apdu_resp;

  /* Sanity check */
  if(params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    return;
  }

  /* Parse input info */
  in_ptr = (com_qualcomm_uimremoteclient_UimRemoteApduReq *)(params_ptr->data);
  if(in_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL input pointer");
    goto report_error;
  }

  QCRIL_LOG_INFO("qcril_uim_remote_client_request_apdu, instance_id: 0x%x)\n",
                 params_ptr->instance_id);

  /* Find out what slot it is */
  if (!qcril_uim_remote_convert_slot_id_to_slot_type(qmi_ril_get_sim_slot(), &uim_remote_slot))
  {
    return;
  }

  /* Allocate & fill up QMI Remote request */
  qmi_rmt_req_ptr = (uim_remote_apdu_req_msg_v01 *) qcril_malloc(sizeof(uim_remote_apdu_req_msg_v01));
  if (qmi_rmt_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for qmi_rmt_req_ptr");
    goto report_error;
  }

  memset(qmi_rmt_req_ptr, 0, sizeof(uim_remote_apdu_req_msg_v01));
  qmi_rmt_req_ptr->slot = uim_remote_slot;
  qmi_rmt_req_ptr->apdu_status = (in_ptr->status ==
    com_qualcomm_uimremoteclient_UimRemoteApduReq_ApduStatus_UIM_REMOTE_SUCCESS) ?
    QMI_RESULT_SUCCESS_V01 : QMI_RESULT_FAILURE_V01;

  /* Add APDU ID & response (request from client) if needed */
  qmi_rmt_req_ptr->apdu_id = qcril_uim_remote_get_apdu_id();
  apdu_data_ptr = in_ptr->apduResponse.arg;
  if ((apdu_data_ptr  != NULL) &&
      (apdu_data_ptr->data != NULL) &&
      (apdu_data_ptr->len  > 0)     &&
      (apdu_data_ptr->len  <= QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01))
  {
    QCRIL_LOG_INFO("Adding APDU info & Segment TLVs, total_response_apdu_size: 0x%x",
                   apdu_data_ptr->len);
    qmi_rmt_req_ptr->response_apdu_info_valid = 0x01;
    qmi_rmt_req_ptr->response_apdu_info.total_response_apdu_size = apdu_data_ptr->len;
    qmi_rmt_req_ptr->response_apdu_info.response_apdu_segment_offset = 0;
    qmi_rmt_req_ptr->response_apdu_segment_valid = 0x01;
    qmi_rmt_req_ptr->response_apdu_segment_len = apdu_data_ptr->len;
    memcpy(qmi_rmt_req_ptr->response_apdu_segment, apdu_data_ptr->data, apdu_data_ptr->len);
  }

  /* Allocate original request */
  orig_req_ptr = qcril_uim_allocate_orig_request(params_ptr->instance_id,
                                                 QCRIL_MAX_MODEM_ID - 1,
                                                 (RIL_Token)params_ptr->t,
                                                 params_ptr->event_id,
                                                 0);
  if (orig_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for orig_req_ptr");
    goto report_error;
  }

  /* Allocate response pointer since it is an async command */
  qmi_rmt_rsp_ptr = (uim_remote_apdu_resp_msg_v01 *)
                      qcril_malloc(sizeof(uim_remote_apdu_resp_msg_v01));
  if (qmi_rmt_rsp_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for qmi_rmt_rsp_ptr");
    goto report_error;
  }

  memset(qmi_rmt_rsp_ptr, 0x00, sizeof(uim_remote_apdu_resp_msg_v01));

  /* Proceed with Event request */
  QCRIL_LOG_QMI( QCRIL_MAX_MODEM_ID - 1, "qmi_uim_remote_service", "event req" );
  qmi_err_code = qmi_client_send_msg_async(
                   qmi_remote_handle,
                   QMI_UIM_REMOTE_APDU_REQ_V01,
                   (void *) qmi_rmt_req_ptr,
                   sizeof(uim_remote_apdu_req_msg_v01),
                   (void *) qmi_rmt_rsp_ptr,
                   sizeof(uim_remote_apdu_resp_msg_v01),
                   qcril_uim_remote_cmd_callback,
                   orig_req_ptr,
                   &txn_handle);
  /* On successful API call, free only the request */
  if (qmi_err_code == 0)
  {
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(qmi_rmt_req_ptr);
    return;
  }

report_error:
    /* In any case of error, check & free all the allocated pointers */
    QCRIL_LOG_ERROR("Error in qcril_uim_remote_client_request_apdu, qmi_err_code: 0x%x", qmi_err_code);
    memset(&apdu_resp, 0, sizeof(com_qualcomm_uimremoteclient_UimRemoteApduResp));
    apdu_resp.status = com_qualcomm_uimremoteclient_UimRemoteApduResp_Status_UIM_REMOTE_FAILURE;
    qcril_uim_remote_client_socket_send(TRUE,
                                        (RIL_Token)params_ptr->t,
                                        com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_RESPONSE,
                                        qcril_uim_remote_client_map_event_to_request(params_ptr->event_id),
                                        TRUE,
                                        com_qualcomm_uimremoteclient_Error_UIM_REMOTE_ERR_GENERIC_FAILURE,
                                        &apdu_resp,
                                        sizeof(com_qualcomm_uimremoteclient_UimRemoteApduResp));

    /* Clean up any original request if allocated */
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(orig_req_ptr);
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(qmi_rmt_req_ptr);
    QCRIL_QMI_UIM_REMOTE_FREE_PTR(qmi_rmt_rsp_ptr);
#endif /* FEATURE_QCRIL_UIM_REMOTE_CLIENT */
} /* qcril_uim_remote_client_request_apdu */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_init

===========================================================================*/
void qcril_uim_remote_init
(
 void
)
{
#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
  uint8                           num_retries  = 0;
  qmi_client_error_type           qmi_err_code = 0;
  qmi_cci_os_signal_type          os_params;
  qmi_service_info                info;
  qmi_idl_service_object_type     client_service_obj;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Get QMI UIM Remote service object */
  client_service_obj = uim_remote_get_service_object_v01();

  qmi_err_code = qmi_client_notifier_init(
                   client_service_obj,
                   &os_params,
                   &qmi_remote_handle);

  if (qmi_err_code != QMI_NO_ERR)
  {
    QCRIL_LOG_ERROR("Notifier init error: 0x&x", qmi_err_code);
    return;
  }

  do
  {
    /* Wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, QCRIL_UIM_REMOTE_QMI_SVC_TIMEOUT);

    if(os_params.timed_out)
    {
      num_retries++;
      QCRIL_LOG_INFO("Timeout, sig wait try # %d", num_retries);
    }
    else
    {
      break;
    }
    QMI_CCI_OS_SIGNAL_CLEAR(&os_params);
  } while (num_retries < QCRIL_UIM_REMOTE_QMI_INIT_MAX_RETRIES);

  /* Return if not ready after max retries */
  if (num_retries >= QCRIL_UIM_REMOTE_QMI_INIT_MAX_RETRIES)
  {
    qmi_client_release(qmi_remote_handle);
    QCRIL_LOG_INFO("Timeout for max retries reached: %d", num_retries);
    return;
  }

  qmi_err_code = qmi_client_get_service_instance(
                  client_service_obj,
                  QMI_CLIENT_INSTANCE_ANY,
                  &info);
  if(qmi_err_code != QMI_NO_ERR)
  {
    QCRIL_LOG_ERROR("Failed to get service instance, qmi_err_code: 0x&x", qmi_err_code);
    qmi_client_release(qmi_remote_handle);
    return;
  }

  /* Initialize connection to QMI subsystem */
  qmi_err_code = qmi_client_init(&info,
                                 client_service_obj,
                                 (qmi_client_ind_cb)qcril_uim_remote_ind_callback,
                                 NULL,
                                 &os_params,
                                 &qmi_remote_handle);

  /* Return error upon port/handle errors */
  if ((qmi_remote_handle == NULL) || (qmi_err_code != 0))
  {
    QCRIL_LOG_ERROR("QMI UIM REMOTE service port open failure, qmi_err_code: 0x%x", qmi_err_code);
    qmi_client_release(qmi_remote_handle);
    qmi_remote_handle = NULL;
    return;
  }

  QCRIL_LOG_INFO("%s","QMI UIM Remote Client init successful");
#endif /* FEATURE_QCRIL_UIM_REMOTE_CLIENT */
} /* qcril_uim_remote_init */


/*===========================================================================

  FUNCTION:  qcril_uim_remote_release

===========================================================================*/
void qcril_uim_remote_release
(
 void
)
{
  int qmi_err_code = -1;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Deinitialize QMI interface */
  if (qmi_remote_handle != NULL)
  {
    QCRIL_LOG_QMI( QCRIL_MAX_MODEM_ID - 1, "qmi_uim_remote_service", "release" );
    qmi_err_code = qmi_client_release(qmi_remote_handle);
    qmi_remote_handle = NULL;
  }
} /* qcril_uim_release */

