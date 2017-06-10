/*===========================================================================

  Copyright (c) 2010, 2014 - 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/micheleb/ril/qcril_uim_security.c#2 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/23/15   vr      SIMLOCK Temperory unlock status indication
03/18/15   at      Perso reactivation support
08/07/14   yt      Retrieve perso retries using QMI_UIM_GET_CONFIGURATION
01/13/14   at      Send remaining retries for PIN APIs with empty PIN string
11/06/13   rak     Support change in ME Deperso request params.
01/29/13   yt      Support for third SIM card slot
01/14/13   yt      Fix critical KW errors
10/31/12   at      Explicit query for card status during PIN responses
03/22/12   at      Replacing malloc()/free() with qcril_malloc()/qcril_free()
08/19/11   yt      Fixed Klocwork errors
07/14/11   yt      Support for returning number of perso retries
04/11/11   yt      Support for silent PIN1 verification
03/22/11   at      Support for ril.h v6
01/18/11   at      Removed slot id parameter from all requests
11/12/10   at      Added support for UIM queue implementation
10/06/10   at      Support for handling instance_id passed in requests
09/28/10   at      Changes to pass correct error code for change pin when
                   disabled case
09/21/10   at      Support for UPIN in all PIN related interfaces
09/09/10   at      Changed the way sessions are fetched
06/29/10   at      Changes to support pin verification APIs
05/13/10   at      Fixed compile errors & clean up for merging with mainline
04/13/10   mib     Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#if defined (FEATURE_QCRIL_UIM_QMI)

#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_reqlist.h"
#include "qcril_uim.h"
#include "qcril_uim_util.h"
#include "qcril_uim_queue.h"
#include "qcril_uim_restart.h"
#include "qcril_uim_card.h"
#include "qcril_uim_qcci.h"
#include "qcril_uim_security.h"
#include <string.h>


/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_fetch_retries

===========================================================================*/
/*!
    @brief
    Finds the app related to the passed session type & fetches its remaining
    retry attempts depending upon the request type - PIN1/PIN2/PUK1/PUK2.

    @return
    RIL_E_SUCCESS if successfully found, else RIL_E_GENERIC_FAILURE
*/
/*=========================================================================*/
static RIL_Errno qcril_uim_fetch_retries
(
  int                           request_id,
  qmi_uim_session_type          session_type,
  int                         * num_retries_ptr
)
{
  uint8                  index     = 0;
  uint8                  slot      = 0;
  qmi_uim_app_state_type app_state = QMI_UIM_APP_STATE_UNKNOWN;

  if (num_retries_ptr == NULL)
  {
    return RIL_E_GENERIC_FAILURE;
  }

  /* Fetch approriate App index to extract the retries */
  if (qcril_uim_extract_index(&index, &slot, session_type) != RIL_E_SUCCESS)
  {
    return RIL_E_GENERIC_FAILURE;
  }

  if ((index >= QMI_UIM_MAX_APP_PER_CARD_COUNT) || (slot >= QMI_UIM_MAX_CARD_COUNT))
  {
    QCRIL_LOG_INFO("Invalid indexes for retries: slot_index: 0x%x, app_index: 0x%x",
                   slot, index);
    return RIL_E_GENERIC_FAILURE;
  }

  app_state = qcril_uim.card_status.card[slot].application[index].app_state;

  /* Also check if the retry info is relevant based on the App state */
  if ((app_state == QMI_UIM_APP_STATE_UNKNOWN) || (app_state == QMI_UIM_APP_STATE_DETECTED))
  {
    QCRIL_LOG_ERROR("Invalid App state for retries: 0x%x, slot_index: 0x%x, app_index: 0x%x",
                    app_state, slot, index);
    return RIL_E_REQUEST_NOT_SUPPORTED;
  }

  /* Fetch retries based on the request type */
  switch (request_id)
  {
    case RIL_REQUEST_ENTER_SIM_PIN:
      *num_retries_ptr = qcril_uim.card_status.card[slot].application[index].pin1_num_retries;
      break;
    case RIL_REQUEST_ENTER_SIM_PIN2:
      *num_retries_ptr = qcril_uim.card_status.card[slot].application[index].pin2_num_retries;
      break;
    case RIL_REQUEST_ENTER_SIM_PUK:
      *num_retries_ptr = qcril_uim.card_status.card[slot].application[index].puk1_num_retries;
      break;
    case RIL_REQUEST_ENTER_SIM_PUK2:
      *num_retries_ptr = qcril_uim.card_status.card[slot].application[index].puk2_num_retries;
      break;
    default:
      QCRIL_LOG_ERROR( "Unsupported Request ID 0x%x\n", request_id);
      return RIL_E_GENERIC_FAILURE;
  }

  QCRIL_LOG_INFO( "Remaining retries: 0x%x, slot: 0x%x, app_index: 0x%x)\n",
                 *num_retries_ptr, slot, index);

  return RIL_E_SUCCESS;
} /* qcril_uim_fetch_retries */


/*===========================================================================

  FUNCTION:  qcril_uim_is_pin_disabled

===========================================================================*/
/*!
    @brief
    Checks if the passed app's PIN1 is disabled

    @return
    TRUE if disabled, else FALSE
*/
/*=========================================================================*/
static boolean qcril_uim_is_pin_disabled
(
  qmi_uim_session_type              session_type
)
{
  uint8   index   = 0;
  uint8   slot    = 0;

  if (qcril_uim_extract_index(&index, &slot, session_type) == RIL_E_SUCCESS)
  {
    if ((index < QMI_UIM_MAX_APP_PER_CARD_COUNT) && (slot < QMI_UIM_MAX_CARD_COUNT))
    {
      /* Check appropriate pin state */
      if (qcril_uim.card_status.card[slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        if (qcril_uim.card_status.card[slot].upin_state ==
            QMI_UIM_PIN_STATE_DISABLED)
        {
          return TRUE;
        }
      }
      else if (qcril_uim.card_status.card[slot].application[index].pin1_state ==
               QMI_UIM_PIN_STATE_DISABLED)
      {
        return TRUE;
      }
    }
  }
  return FALSE;
} /* qcril_uim_is_pin_disabled */


/*===========================================================================

  FUNCTION:  qcril_uim_pin_resp

===========================================================================*/
/*!
    @brief
    Handle PIN operation confirmations

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_pin_resp
(
  qcril_uim_callback_params_type             * params_ptr,
  qcril_request_return_type            * const ret_ptr
)
{
  RIL_Token                                 token;
  RIL_Errno                                 ril_err;
  qmi_uim_pin_id_type                       pin_id;
  int                                       num_retries;
  qcril_uim_original_request_type         * original_request_ptr = NULL;
  qcril_uim_indication_params_type        * card_status_ind_ptr  = NULL;
  qmi_uim_rsp_data_type                     card_status_rsp;
  int                                       qmi_err_code         = 0;

  if(params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  /* retreive original request */
  original_request_ptr = (qcril_uim_original_request_type*)params_ptr->orig_req_data;
  if(original_request_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  /* Query card status before sending the response. This is needed to comply to certain
     race condition scenarios. An exmaple is when the get sim status is called soon after the
     PIN response - the PIN status is actually changed but QCRIL hasnt gotten the indication yet */
  card_status_ind_ptr = qcril_malloc(sizeof(qcril_uim_indication_params_type));
  if (card_status_ind_ptr != NULL)
  {
    memset(card_status_ind_ptr, 0x00, sizeof(qcril_uim_indication_params_type));
    memset(&card_status_rsp, 0x00, sizeof(qmi_uim_rsp_data_type));
    qmi_err_code = qcril_qmi_uim_get_card_status(qcril_uim.qmi_handle,
                                                 NULL,
                                                 NULL,
                                                 &card_status_rsp);
    if (qmi_err_code < 0)
    {
      QCRIL_LOG_ERROR( "Get card status failed, qmi_err_code: 0x%x\n", qmi_err_code);
    }
    else
    {
      /* Proceed with updating the global using the common function.
         It takes care of sending the UNSOL before the PIN response */
      card_status_ind_ptr->instance_id = original_request_ptr->instance_id;
      card_status_ind_ptr->modem_id    = original_request_ptr->modem_id;
      card_status_ind_ptr->ind_id      = QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG;
      memcpy(&card_status_ind_ptr->ind_data.status_change_ind,
             &card_status_rsp.rsp_data.get_card_status_rsp.card_status,
             sizeof(qmi_uim_card_status_type));
      qcril_uim_process_status_change_ind(card_status_ind_ptr, ret_ptr);
    }

    /* Clean up the allocated memory */
    qcril_free(card_status_ind_ptr);
    card_status_ind_ptr = NULL;
  }

  /* Token */
  token = (RIL_Token)original_request_ptr->token;

  QCRIL_LOG_DEBUG( "qcril_uim_pin_resp: token=%d qmi_err_code=%d \n",
                    qcril_log_get_token_id(token),
                    params_ptr->qmi_rsp_data.qmi_err_code);

  /* Retrieve the PIN id */
  pin_id = QMI_UIM_PIN_ID_PIN1;
  if (original_request_ptr->request_id == RIL_REQUEST_ENTER_SIM_PIN2 ||
      original_request_ptr->request_id == RIL_REQUEST_ENTER_SIM_PUK2 ||
      original_request_ptr->request_id == RIL_REQUEST_CHANGE_SIM_PIN2)
  {
    pin_id = QMI_UIM_PIN_ID_PIN2;
  }

  /* Response code for RIL */
  switch(params_ptr->qmi_rsp_data.qmi_err_code)
  {
    case 0:  /* QMI_ERR_NONE */
      ril_err = RIL_E_SUCCESS;
      break;

    case 12: /* QMI_ERR_INCORRECT_PIN */
    case 36: /* QMI_ERR_PIN_PERM_BLOCKED */
      ril_err = RIL_E_PASSWORD_INCORRECT;
      break;

    case 35: /* QMI_ERR_PIN_BLOCKED */
      ril_err = RIL_E_PASSWORD_INCORRECT;
      if (pin_id == QMI_UIM_PIN_ID_PIN2)
      {
        ril_err = RIL_E_SIM_PUK2;
      }
      break;

    default:
      ril_err = RIL_E_GENERIC_FAILURE;
      /* Pass the correct error on "change PIN1 when PIN1 is disabled" case */
      if ((params_ptr->qmi_rsp_data.rsp_id == QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG) &&
          (original_request_ptr->request_id == RIL_REQUEST_CHANGE_SIM_PIN) &&
          qcril_uim_is_pin_disabled(original_request_ptr->session_type))
      {
        ril_err = RIL_E_REQUEST_NOT_SUPPORTED;
        QCRIL_LOG_DEBUG( "PIN disabled, change sim pin not supported, qmi_err_code:0x%x \n",
                         params_ptr->qmi_rsp_data.qmi_err_code);
      }
      break;
  }

  /* Compose number of retries */
  switch(params_ptr->qmi_rsp_data.rsp_id)
  {
    case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
      num_retries = params_ptr->qmi_rsp_data.rsp_data.set_pin_protection_rsp.num_retries;
      break;
    case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
      num_retries = params_ptr->qmi_rsp_data.rsp_data.verify_pin_rsp.num_retries;
      break;
    case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
      num_retries = params_ptr->qmi_rsp_data.rsp_data.unblock_pin_rsp.num_unblock_retries;
      break;
    case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
      num_retries = params_ptr->qmi_rsp_data.rsp_data.change_pin_rsp.num_retries;
      break;
    default:
      /* This case should never happen!! */
      ril_err = RIL_E_GENERIC_FAILURE;
      num_retries = 0;
      break;
  }

  if(pin_id == QMI_UIM_PIN_ID_PIN1)
  {
    if (ril_err == RIL_E_SUCCESS)
    {
      qcril_uim_store_encrypted_pin(&params_ptr->qmi_rsp_data,
                                    original_request_ptr->session_type);
    }
    else
    {
      /* Reset PIN1 data if PIN1 operation was unsuccessful */
      qcril_uim_clear_encrypted_pin(original_request_ptr->session_type);
    }
  }

  /* Generate response */
  qcril_uim_response(original_request_ptr->instance_id,
                     token,
                     ril_err,
                     (void*)&num_retries,
                     sizeof(int),
                     TRUE,
                     NULL);

  /* Free memory allocated originally in the request */
  qcril_free(original_request_ptr);
  original_request_ptr = NULL;

} /* qcril_uim_pin_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_deperso_resp

===========================================================================*/
/*!
    @brief
    Handle deperso operation confirmation

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_deperso_resp
(
  qcril_uim_callback_params_type  * params_ptr
)
{
  RIL_Token                             token;
  RIL_Errno                             ril_err;
  int                                   num_retries;
  qcril_uim_original_request_type     * original_request_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  /* retreive original request */
  original_request_ptr = (qcril_uim_original_request_type*)params_ptr->orig_req_data;
  if(original_request_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }
  /* Token and response code for RIL */
  token   = (RIL_Token)original_request_ptr->token;
  ril_err = (params_ptr->qmi_rsp_data.qmi_err_code == 0) ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_DEBUG( "qcril_uim_deperso_resp: token=%d qmi_err_code=%d \n",
                    qcril_log_get_token_id(token),
                    params_ptr->qmi_rsp_data.qmi_err_code);

  /* Number of retries: Android RIL currently supports only the possibility
     to deactivate the perso feature. */
  num_retries = params_ptr->qmi_rsp_data.rsp_data.depersonalization_rsp.num_retries;

  /* Generate response */
  qcril_uim_response(original_request_ptr->instance_id,
                     token,
                     ril_err,
                     (void*)&num_retries,
                     sizeof(int),
                     TRUE,
                     NULL);

  /* Free memory allocated originally in the request */
  qcril_free(original_request_ptr);
  original_request_ptr = NULL;

} /* qcril_uim_deperso_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_perso_resp

===========================================================================*/
/*!
    @brief
    Handle perso operation confirmation

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_perso_resp
(
  qcril_uim_callback_params_type  * params_ptr
)
{
  RIL_Token                             token;
  RIL_Errno                             ril_err;
  int                                   num_retries          = -1;
  qcril_uim_original_request_type     * original_request_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    return;
  }

  /* retreive original request */
  original_request_ptr = (qcril_uim_original_request_type*)params_ptr->orig_req_data;
  if(original_request_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    return;
  }

  /* Update Token */
  token   = (RIL_Token)original_request_ptr->token;

  /* Update response error code */
  switch(params_ptr->qmi_rsp_data.qmi_err_code)
  {
    case 0:  /* QMI_ERR_NONE */
      ril_err = RIL_E_SUCCESS;
      break;

    case 12: /* QMI_ERR_INCORRECT_PIN */
    case 36: /* QMI_ERR_PIN_PERM_BLOCKED */
      ril_err = RIL_E_PASSWORD_INCORRECT;
      break;

    case 35: /* QMI_ERR_PIN_BLOCKED */
      ril_err = RIL_E_PASSWORD_INCORRECT;
      break;

    default:
      ril_err = RIL_E_GENERIC_FAILURE;
      break;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_perso_resp: token=%d qmi_err_code=%d \n",
                    qcril_log_get_token_id(token),
                    params_ptr->qmi_rsp_data.qmi_err_code);

  /* Update number of retries */
  if (params_ptr->qmi_rsp_data.rsp_data.personalization_rsp.num_retries_valid)
  {
    num_retries = params_ptr->qmi_rsp_data.rsp_data.personalization_rsp.num_retries;
  }

  /* Generate response */
  qcril_uim_response(original_request_ptr->instance_id,
                     token,
                     ril_err,
                     (void*)&num_retries,
                     sizeof(int),
                     TRUE,
                     NULL);

  /* Free memory allocated originally in the request */
  qcril_free(original_request_ptr);
  original_request_ptr = NULL;
} /* qcril_uim_perso_resp */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_pin

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_SIM_PIN/RIL_REQUEST_ENTER_SIM_PIN2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_pin
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_modem_id_e_type                     modem_id = QCRIL_MAX_MODEM_ID - 1;
  int                                       res;
  RIL_Errno                                 err;
  uint16                                    aid_size             = 0;
  uint16                                    first_level_df_path  = 0;
  uint8                                     slot                 = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  uint8                                     index                = 0;
  uint8                                     prov_slot            = 0;
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
  qmi_uim_verify_pin_params_type            pin_params;
  uint8                                  ** in_ptr    = NULL;
  qcril_uim_original_request_type         * original_request_ptr = NULL;

  if(params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  /* Parse input pin info */
  in_ptr = (uint8 **)(params_ptr->data);
  /* Sanity checks
     in_ptr[0]: PIN value
     in_ptr[1]: AID vaule */
  if(in_ptr == NULL || in_ptr[0] == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0,
                       TRUE, "NULL pointer in PIN data");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_enter_pin(%s, %s)\n",
                  in_ptr[0],
                  (in_ptr[1] != NULL) ? (const char *)in_ptr[1] : "NULL" );

  memset(&pin_params, 0, sizeof(qmi_uim_verify_pin_params_type));

  /* Add entry to ReqList */
  QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr);

  /* Update the file path based on passed aid pointer */
  aid_size = (in_ptr[1] == NULL) ? 0 : strlen((const char*)in_ptr[1]);
  first_level_df_path = (aid_size == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Fetch slot info */
  slot = qcril_uim_instance_id_to_slot(params_ptr->instance_id);
  QCRIL_ASSERT(slot < QMI_UIM_MAX_CARD_COUNT);

  /* Extract session type, we need a provisioning session for pin operations */
  err = qcril_uim_extract_session_type(slot,
                                       (const char *)in_ptr[1],
                                       first_level_df_path,
                                       &pin_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_E_SUCCESS) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(slot,
                                           (const char *)in_ptr[1],
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &pin_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_E_SUCCESS) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0,
                         TRUE, "error extracting session info");
      return;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", pin_params.session_info.session_type);

  /* PIN id */
  switch (params_ptr->event_id)
  {
    case RIL_REQUEST_ENTER_SIM_PIN:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN1;
      break;
    case RIL_REQUEST_ENTER_SIM_PIN2:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;
      break;
    default:
      /* Invalid PIN ID */
      QCRIL_LOG_ERROR( " Invalid Pin from RIL Request ID 0x%x\n", params_ptr->event_id);
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0, TRUE, NULL);
      return;
  }

#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  /* Check if PIN1 is replaced by UPIN for the specified App */
  if ((pin_params.pin_id == QMI_UIM_PIN_ID_PIN1) &&
      (qcril_uim_extract_index(&index,
                               &prov_slot,
                               pin_params.session_info.session_type) == RIL_E_SUCCESS))
  {
    if ((index >= 0) && (index < QMI_UIM_MAX_APP_PER_CARD_COUNT) &&
        (prov_slot == slot) &&
        (prov_slot >= 0) && (prov_slot < QMI_UIM_MAX_CARD_COUNT))
    {
      if (qcril_uim.card_status.card[prov_slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        pin_params.pin_id = QMI_UIM_PIN_ID_UPIN;
        QCRIL_LOG_INFO("PIN1 replaced by UPIN for card[%d].application[%d]",
                       prov_slot, index);
      }
    }
  }
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */

  /* Special case for empty PIN string. In this case the PIN retries left is returned
     immediately from the global card status */
  if (strlen((const char*)in_ptr[0]) == 0)
  {
    int num_retries = 0;

    QCRIL_LOG_INFO( "Empty PIN string, fetching retries. Request_id: %d", params_ptr->event_id);

    err = qcril_uim_fetch_retries(params_ptr->event_id,
                                  pin_params.session_info.session_type,
                                  &num_retries);

    /* Generate response */
    qcril_uim_response(params_ptr->instance_id, params_ptr->t, err,
                       (void*)&num_retries, sizeof(int), TRUE, NULL);
    return;
  }

  /* Allocate original request */
  original_request_ptr = qcril_uim_allocate_orig_request(params_ptr->instance_id,
                                                         modem_id,
                                                         (RIL_Token)params_ptr->t,
                                                         params_ptr->event_id,
                                                         pin_params.session_info.session_type);
  if (original_request_ptr == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE,
                       "error allocating memory for original_request_ptr");
    return;
  }

  /* PIN value */
  pin_params.pin_data.data_ptr = in_ptr[0];
  pin_params.pin_data.data_len = strlen((const char*)in_ptr[0]);

  /* Proceed with verify pin */
  QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "verify pin" );
  res = qcril_uim_queue_send_request(QCRIL_UIM_REQUEST_VERIFY_PIN,
                                     qcril_uim.qmi_handle,
                                     &pin_params,
                                     qmi_uim_callback,
                                     (void*)original_request_ptr);
  if (res < 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, NULL);
    /* Clean up any original request if allocated */
    if (original_request_ptr)
    {
      qcril_free(original_request_ptr);
      original_request_ptr = NULL;
    }
  }
} /* qcril_uim_request_enter_pin */


/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_puk

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_SIM_PUK/RIL_REQUEST_ENTER_SIM_PUK2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_puk
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_modem_id_e_type                     modem_id = QCRIL_MAX_MODEM_ID - 1;
  int                                       res;
  RIL_Errno                                 err;
  uint16                                    aid_size             = 0;
  uint16                                    first_level_df_path  = 0;
  uint8                                     slot                 = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  uint8                                     index                = 0;
  uint8                                     prov_slot            = 0;
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
  qmi_uim_unblock_pin_params_type           pin_params;
  uint8                                  ** in_ptr    = NULL;
  qcril_uim_original_request_type         * original_request_ptr = NULL;

  if(params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  /* Parse input pin info */
  in_ptr = (uint8 **)(params_ptr->data);
  /* Sanity checks
     in_ptr[0]: PUK value
     in_ptr[1]: new PIN value
     in_ptr[2]: AID vaule */
  if(in_ptr == NULL || in_ptr[0] == NULL || in_ptr[1] == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0,
                       TRUE, "NULL pointer in PIN data");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_enter_puk(%s, %s, %s)\n",
                  in_ptr[0],
                  in_ptr[1],
                  (in_ptr[2] != NULL) ? (const char *)in_ptr[2] : "NULL" );

  memset(&pin_params, 0, sizeof(qmi_uim_unblock_pin_params_type));

  /* Add entry to ReqList */
  QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr);

  /* Update the file path based on passed aid pointer */
  aid_size = (in_ptr[2] == NULL) ? 0 : strlen((const char*)in_ptr[2]);
  first_level_df_path = (aid_size == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Fetch slot info */
  slot = qcril_uim_instance_id_to_slot(params_ptr->instance_id);
  QCRIL_ASSERT(slot < QMI_UIM_MAX_CARD_COUNT);

  /* Extract session type, we need a provisioning session for pin operations */
  err = qcril_uim_extract_session_type(slot,
                                       (const char *)in_ptr[2],
                                       first_level_df_path,
                                       &pin_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_E_SUCCESS) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(slot,
                                           (const char *)in_ptr[2],
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &pin_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_E_SUCCESS) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0,
                         TRUE, "error extracting session info");
      return;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", pin_params.session_info.session_type);

  /* PIN id */
  switch (params_ptr->event_id)
  {
    case RIL_REQUEST_ENTER_SIM_PUK:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN1;
      break;
    case RIL_REQUEST_ENTER_SIM_PUK2:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;
      break;
    default:
      /* Invalid PIN ID */
      QCRIL_LOG_ERROR( " Invalid Pin from RIL Request ID 0x%x\n", params_ptr->event_id);
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0, TRUE, NULL);
      return;
  }

#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  /* Check if PIN1 is replaced by UPIN for the specified App */
  if ((pin_params.pin_id == QMI_UIM_PIN_ID_PIN1) &&
      (qcril_uim_extract_index(&index,
                               &prov_slot,
                               pin_params.session_info.session_type) == RIL_E_SUCCESS))
  {
    if ((index >= 0) && (index < QMI_UIM_MAX_APP_PER_CARD_COUNT) &&
        (prov_slot == slot) &&
        (prov_slot >= 0) && (prov_slot < QMI_UIM_MAX_CARD_COUNT))
    {
      if (qcril_uim.card_status.card[prov_slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        pin_params.pin_id = QMI_UIM_PIN_ID_UPIN;
        QCRIL_LOG_INFO("PIN1 replaced by UPIN for card[%d].application[%d]",
                       prov_slot, index);
      }
    }
  }
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */

  /* Special case for empty PUK string. In this case the PUK retries left is returned
     immediately from the global card status */
  if (strlen((const char*)in_ptr[0]) == 0)
  {
    int num_retries = 0;

    QCRIL_LOG_INFO( "Empty PUK string, fetching retries. Request_id: %d", params_ptr->event_id);

    err = qcril_uim_fetch_retries(params_ptr->event_id,
                                  pin_params.session_info.session_type,
                                  &num_retries);

    /* Generate response */
    qcril_uim_response(params_ptr->instance_id, params_ptr->t, err,
                       (void*)&num_retries, sizeof(int), TRUE, NULL);
    return;
  }

  /* Allocate original request */
  original_request_ptr = qcril_uim_allocate_orig_request(params_ptr->instance_id,
                                                         modem_id,
                                                         (RIL_Token)params_ptr->t,
                                                         params_ptr->event_id,
                                                         pin_params.session_info.session_type);
  if (original_request_ptr == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE,
                       "error allocating memory for original_request_ptr");
    return;
  }

  /* PUK value */
  pin_params.puk_data.data_ptr = in_ptr[0];
  pin_params.puk_data.data_len = strlen((const char*)in_ptr[0]);

  /* New PIN value */
  pin_params.new_pin_data.data_ptr = in_ptr[1];
  pin_params.new_pin_data.data_len = strlen((const char*)in_ptr[1]);

  /* Proceed with unblock pin */
  QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "unblock pin" );
  res = qcril_uim_queue_send_request(QCRIL_UIM_REQUEST_UNBLOCK_PIN,
                                     qcril_uim.qmi_handle,
                                     &pin_params,
                                     qmi_uim_callback,
                                     (void*)original_request_ptr);
  if (res < 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, NULL);
    /* Clean up any original request if allocated */
    if (original_request_ptr)
    {
      qcril_free(original_request_ptr);
      original_request_ptr = NULL;
    }
  }
} /* qcril_uim_request_enter_puk */


/*===========================================================================

  FUNCTION:  qcril_uim_request_change_pin

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CHANGE_SIM_PIN/RIL_REQUEST_CHANGE_SIM_PIN2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_change_pin
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_modem_id_e_type                     modem_id = QCRIL_MAX_MODEM_ID - 1;
  int                                       res;
  RIL_Errno                                 err;
  uint16                                    aid_size             = 0;
  uint16                                    first_level_df_path  = 0;
  uint8                                     slot                 = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  uint8                                     index                = 0;
  uint8                                     prov_slot            = 0;
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
  qmi_uim_change_pin_params_type            pin_params;
  uint8                                  ** in_ptr    = NULL;
  qcril_uim_original_request_type         * original_request_ptr = NULL;

  if(params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  /* Parse input pin info */
  in_ptr = (uint8 **)(params_ptr->data);
  /* Sanity checks
     in_ptr[0]: old PIN value
     in_ptr[1]: new PIN value
     in_ptr[2]: AID vaule */
  if(in_ptr == NULL || in_ptr[0] == NULL || in_ptr[1] == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0,
                       TRUE, "NULL pointer in PIN data");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_change_pin(%s, %s, %s)\n",
                  in_ptr[0],
                  in_ptr[1],
                  (in_ptr[2] != NULL) ? (const char *)in_ptr[2] : "NULL" );

  memset(&pin_params, 0, sizeof(qmi_uim_change_pin_params_type));

  /* Add entry to ReqList */
  QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr);

  /* Update the file path based on passed aid pointer */
  aid_size = (in_ptr[2] == NULL) ? 0 : strlen((const char*)in_ptr[2]);
  first_level_df_path = (aid_size == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Fetch slot info */
  slot = qcril_uim_instance_id_to_slot(params_ptr->instance_id);
  QCRIL_ASSERT(slot < QMI_UIM_MAX_CARD_COUNT);

  /* Extract session type, we need a provisioning session for pin operations */
  err = qcril_uim_extract_session_type(slot,
                                       (const char *)in_ptr[2],
                                       first_level_df_path,
                                       &pin_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_E_SUCCESS) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(slot,
                                           (const char *)in_ptr[2],
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &pin_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_E_SUCCESS) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0,
                         TRUE, "error extracting session info");
      return;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", pin_params.session_info.session_type);

  /* PIN id */
  switch (params_ptr->event_id)
  {
    case RIL_REQUEST_CHANGE_SIM_PIN:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN1;
      break;
    case RIL_REQUEST_CHANGE_SIM_PIN2:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;
      break;
    default:
      /* Invalid PIN ID */
      QCRIL_LOG_ERROR( " Invalid Pin from RIL Request ID 0x%x\n", params_ptr->event_id);
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0, TRUE, NULL);
      return;
  }

#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  /* Check if PIN1 is replaced by UPIN for the specified App */
  if ((pin_params.pin_id == QMI_UIM_PIN_ID_PIN1) &&
      (qcril_uim_extract_index(&index,
                               &prov_slot,
                               pin_params.session_info.session_type) == RIL_E_SUCCESS))
  {
    if ((index >= 0) && (index < QMI_UIM_MAX_APP_PER_CARD_COUNT) &&
        (prov_slot == slot) &&
        (prov_slot >= 0) && (prov_slot < QMI_UIM_MAX_CARD_COUNT))
    {
      if (qcril_uim.card_status.card[prov_slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        pin_params.pin_id = QMI_UIM_PIN_ID_UPIN;
        QCRIL_LOG_INFO("PIN1 replaced by UPIN for card[%d].application[%d]",
                       prov_slot, index);
      }
    }
  }
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN*/

  /* Allocate original request */
  original_request_ptr = qcril_uim_allocate_orig_request(params_ptr->instance_id,
                                                         modem_id,
                                                         (RIL_Token)params_ptr->t,
                                                         params_ptr->event_id,
                                                         pin_params.session_info.session_type);
  if (original_request_ptr == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE,
                       "error allocating memory for original_request_ptr");
    return;
  }

  /* Old PIN value */
  pin_params.old_pin_data.data_ptr = in_ptr[0];
  pin_params.old_pin_data.data_len = strlen((const char*)in_ptr[0]);

  /* New PIN value */
  pin_params.new_pin_data.data_ptr = in_ptr[1];
  pin_params.new_pin_data.data_len = strlen((const char*)in_ptr[1]);

  /* Proceed with change pin */
  QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "change pin" );
  res = qcril_uim_queue_send_request(QCRIL_UIM_REQUEST_CHANGE_PIN,
                                     qcril_uim.qmi_handle,
                                     &pin_params,
                                     qmi_uim_callback,
                                     (void*)original_request_ptr);
  if (res < 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, NULL);
    /* Clean up any original request if allocated */
    if (original_request_ptr)
    {
      qcril_free(original_request_ptr);
      original_request_ptr = NULL;
    }
  }
} /* qcril_uim_request_change_pin */


/*===========================================================================

  FUNCTION:  qcril_uim_request_set_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS request from
    the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_set_pin_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_modem_id_e_type                     modem_id  = QCRIL_MAX_MODEM_ID - 1;
  uint8                                  ** in_ptr    = NULL;
  int                                       res;
  RIL_Errno                                 err;
  uint8                                     slot       = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  uint16                                    aid_size   = 0;
  uint16                                    first_level_df_path = 0;
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  uint8                                     index      = 0;
  uint8                                     prov_slot  = 0;
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
  qcril_uim_callback_params_type            callback_params;
  qmi_uim_set_pin_protection_params_type    pin_params;
  qcril_uim_original_request_type         * original_request_ptr = NULL;

  if(params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  memset(&pin_params, 0, sizeof(qmi_uim_set_pin_protection_params_type));
  memset(&callback_params, 0, sizeof(qcril_uim_callback_params_type));

  /* Parse input pin info */
  in_ptr = (uint8 **)(params_ptr->data);

  /* Sanity checks
     in_ptr[0]: facility string code
     in_ptr[1]: lock/unlock
     in_ptr[2]: password
	 in_ptr[3]: service class bit (unused)
     in_ptr[4]: AID value */
  if(in_ptr == NULL || in_ptr[0] == NULL || in_ptr[1] == NULL || in_ptr[2] == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0,
                       TRUE, "NULL pointer in PIN data");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_set_pin_status(%s, %s, %s, %s)\n",
                  in_ptr[0],
                  in_ptr[1],
                  in_ptr[2],
                  (in_ptr[4] != NULL) ? (const char *)in_ptr[4] : "NULL" );

  /* Add entry to ReqList */
  QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr);

  /* Check facility string */
  if (memcmp(in_ptr[0], "SC", 2) != 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0,
                       TRUE, "unsupported facilty string" );
    return;
  }

  /* Fetch slot info */
  slot = qcril_uim_instance_id_to_slot(params_ptr->instance_id);
  QCRIL_ASSERT(slot < QMI_UIM_MAX_CARD_COUNT);

  /* Update the file path based on passed aid pointer */
  aid_size = (in_ptr[4] == NULL) ?  0 : strlen((const char *)in_ptr[4]);
  first_level_df_path = (aid_size == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Extract session type, we need a provisioning session for pin operations */
  err = qcril_uim_extract_session_type(slot,
                                       (const char *)in_ptr[4],
                                       first_level_df_path,
                                       &pin_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_E_SUCCESS) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(slot,
                                           (const char *)in_ptr[4],
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &pin_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_E_SUCCESS) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0,
                         TRUE, "error extracting session info");
      return;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", pin_params.session_info.session_type);

  /* pin parameters */
  if (*in_ptr[1] == '0')
  {
    pin_params.pin_operation = QMI_UIM_PIN_OP_DISABLE;
  }
  else if (*in_ptr[1] == '1')
  {
    pin_params.pin_operation = QMI_UIM_PIN_OP_ENABLE;
  }
  else
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t, err, NULL, 0,
                       TRUE, "invalid input paramter data[1]");
    return;
  }

  pin_params.pin_id = QMI_UIM_PIN_ID_PIN1;
  pin_params.pin_data.data_ptr = (unsigned char*)in_ptr[2];
  pin_params.pin_data.data_len = strlen((const char*)in_ptr[2]);

#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  /* In case of UPIN, only disable is currently supported */
  if ((pin_params.pin_operation == QMI_UIM_PIN_OP_DISABLE) &&
      (qcril_uim_extract_index(&index,
                               &prov_slot,
                               pin_params.session_info.session_type) == RIL_E_SUCCESS))
  {
    if ((index >= 0) && (index < QMI_UIM_MAX_APP_PER_CARD_COUNT) &&
        (prov_slot == slot) &&
        (prov_slot >= 0) && (prov_slot < QMI_UIM_MAX_CARD_COUNT))
    {
      if (qcril_uim.card_status.card[prov_slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        pin_params.pin_id = QMI_UIM_PIN_ID_UPIN;
        QCRIL_LOG_INFO("PIN1 replaced by UPIN for card[%d].application[%d]",
                       prov_slot, index);
      }
    }
  }
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */

  /* Allocate original request */
  original_request_ptr = qcril_uim_allocate_orig_request(params_ptr->instance_id,
                                                         modem_id,
                                                         (RIL_Token)params_ptr->t,
                                                         params_ptr->event_id,
                                                         pin_params.session_info.session_type);
  if (original_request_ptr == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE,
                       "error allocating memory for original_request_ptr");
    return;
  }

  /* Proceed with set pin protection */
  QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "set pin protection" );
  res = qcril_uim_queue_send_request(QCRIL_UIM_REQUEST_SET_PIN,
                                     qcril_uim.qmi_handle,
                                     &pin_params,
                                     qmi_uim_callback,
                                     (void*)original_request_ptr);
  if (res < 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t, RIL_E_GENERIC_FAILURE,
                       NULL, 0, TRUE, "error in qmi_uim_set_pin_protection");
    /* Clean up any original request if allocated */
    if (original_request_ptr)
    {
      qcril_free(original_request_ptr);
      original_request_ptr = NULL;
    }
  }
} /* qcril_uim_request_set_pin_status */


/*===========================================================================

  FUNCTION:  qcril_uim_request_get_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_pin_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  uint8                          ** in_ptr      = NULL;
  RIL_Errno                         err;
  uint8                             slot        = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  int                               ret_value   = 0;
  qmi_uim_pin_status_type           pin1_status = QMI_UIM_PIN_STATE_UNKNOWN;

  if(params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  /* Parse input pin info */
  in_ptr = (uint8 **)(params_ptr->data);

  /* Sanity checks
     in_ptr[0]: facility string code
     in_ptr[1]: password
     in_ptr[2]: service class bit (unused)
     in_ptr[3]: AID value */
  if(in_ptr == NULL || in_ptr[0] == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0,
                       TRUE, "NULL pointer in PIN data");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_get_pin_status(%s, %s, %s)\n",
                  in_ptr[0],
                  (in_ptr[1] != NULL) ? (const char *)in_ptr[1] : "NULL",
                  (in_ptr[3] != NULL) ? (const char *)in_ptr[3] : "NULL" );

  /* Add entry to ReqList */
  QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr);

  /* Check facility string */
  if (memcmp(in_ptr[0], "SC", 2) != 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0,
                       TRUE, "unsupported facilty string" );
    return;
  }

  /* Fetch slot info */
  slot = qcril_uim_instance_id_to_slot(params_ptr->instance_id);
  QCRIL_ASSERT(slot < QMI_UIM_MAX_CARD_COUNT);

  /* Fetch PIN1 or UPIN status for the specified App */
  err = qcril_uim_extract_pin1_status(slot,
                                      (const char *)in_ptr[3],
                                      &pin1_status);
  if (err != RIL_E_SUCCESS)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t, err, NULL, 0,
                       TRUE, "error extracting pin1 status");
    return;
  }

  switch(pin1_status)
  {
    case QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED:
    case QMI_UIM_PIN_STATE_ENABLED_VERIFIED:
    case QMI_UIM_PIN_STATE_BLOCKED:
    case QMI_UIM_PIN_STATE_PERM_BLOCKED:
      QCRIL_LOG_INFO( "%s", "PIN enabled\n" );
      ret_value = 1;
      break;
    case QMI_UIM_PIN_STATE_DISABLED:
      QCRIL_LOG_INFO( "%s", "PIN disabled\n" );
      ret_value = 0;
      break;
    default:
      QCRIL_LOG_INFO( "Unknown pin status 0x%x \n", pin1_status);
      err = RIL_E_GENERIC_FAILURE;
      break;
  }

  qcril_uim_response(params_ptr->instance_id, params_ptr->t, err, &ret_value, sizeof(int),
                     TRUE, NULL);
} /* qcril_uim_request_get_pin_status */


/*===========================================================================

  FUNCTION:  qcril_uim_get_perso_retries

===========================================================================*/
/*!
    @brief
    Gets the personalization retries for the feature by sending
    QMI_UIM_GET_CONFIGURATION command to modem.

    @return
    None
*/
/*=========================================================================*/
static RIL_Errno qcril_uim_get_perso_retries
(
  qmi_uim_perso_feature_id_type    perso_feature,
  int                            * num_retries_ptr
)
{
  int                                    ret    = 0;
  uint8                                  i      = 0;
  RIL_Errno                              errval = RIL_E_GENERIC_FAILURE;
  qmi_uim_get_configuration_params_type  get_config_params;
  qmi_uim_rsp_data_type                  rsp_data;

  if (num_retries_ptr == NULL)
  {
    return RIL_E_GENERIC_FAILURE;
  }

  *num_retries_ptr = 0;

  memset(&get_config_params, 0x00, sizeof(get_config_params));
  memset(&rsp_data, 0x00, sizeof(rsp_data));

  get_config_params.perso_status = QMI_UIM_TRUE;
  ret = qcril_qmi_uim_get_configuration(qcril_uim.qmi_handle,
                                        &get_config_params,
                                        NULL,
                                        NULL,
                                        &rsp_data);

  if (ret != 0)
  {
    return RIL_E_GENERIC_FAILURE;
  }

  for (i = 0;
       i < rsp_data.rsp_data.get_configuration_rsp.perso_status_len && i < QMI_UIM_MAX_PERSO_FEATURES;
       i++)
  {
    if (rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == perso_feature)
    {
      *num_retries_ptr = rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
      errval = RIL_E_SUCCESS;
      break;
    }
  }

  return errval;
} /* qcril_uim_get_perso_retries */


/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_perso_key

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_perso_key
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_modem_id_e_type                  modem_id = QCRIL_MAX_MODEM_ID - 1;
  int                                    res;
  qmi_uim_depersonalization_params_type  perso_params;
#ifdef RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE
  const char *                           perso_type = NULL;
#endif
  const char *                           depersonalization_code = NULL;
  qcril_uim_original_request_type      * original_request_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

#ifdef RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE
  perso_type = ((const char **) params_ptr->data)[0];
  depersonalization_code = ((const char **) params_ptr->data)[1];
  if(perso_type == NULL || depersonalization_code == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0,
                       TRUE, "NULL pointer in perso data");
    QCRIL_ASSERT(0);
    return;
  }
#else
  depersonalization_code = ((const char **) params_ptr->data)[0];
  if(depersonalization_code == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0,
                       TRUE, "NULL pointer in perso data");
    QCRIL_ASSERT(0);
    return;
  }
#endif /* RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE */

  /* Add entry to ReqList */
  QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr);

#ifdef RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE
  /* Perso feature */
  switch (atoi(perso_type))
  {
    case RIL_PERSOSUBSTATE_SIM_NETWORK:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_NW;
      break;
    case RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_NS;
      break;
    case RIL_PERSOSUBSTATE_SIM_CORPORATE:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_CP;
      break;
    case RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_SP;
      break;
    case RIL_PERSOSUBSTATE_SIM_SIM:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_SIM;
      break;
    case RIL_PERSOSUBSTATE_RUIM_NETWORK1:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_NW1;
      break;
    case RIL_PERSOSUBSTATE_RUIM_NETWORK2:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_NW2;
      break;
    case RIL_PERSOSUBSTATE_RUIM_HRPD:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_HRPD;
      break;
    case RIL_PERSOSUBSTATE_RUIM_CORPORATE:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_CP;
      break;
    case RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_SP;
      break;
    case RIL_PERSOSUBSTATE_RUIM_RUIM:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_RUIM;
      break;
    default:
      /* Invalid feature ID */
      QCRIL_LOG_ERROR( " Invalid perso feature from RIL Request ID %s\n", perso_type);
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0, TRUE, NULL);
      return;
  }
#else
  perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_NW;
#endif /* RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE */


  /* Return the number of remaining retries if perso key is empty */
  if(strlen(depersonalization_code) == 0)
  {
    uint8 slot        = 0;
    int   num_retries = 0;
    uint8 i           = 0;

    slot = qcril_uim_instance_id_to_slot(params_ptr->instance_id);
    if( slot >= QMI_UIM_MAX_CARD_COUNT )
    {
      QCRIL_LOG_ERROR("Invalid slot 0x%x for instance id 0x%x",
                       slot, params_ptr->instance_id);
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, NULL);
      return;
    }


    for(i = 0; i < qcril_uim.card_status.card[slot].num_app; i++)
    {
      if((i != QCRIL_UIM_INVALID_APP_INDEX_VALUE) &&
         (qcril_uim.card_status.card[slot].application[i].app_state == QMI_UIM_APP_STATE_PERSO) &&
         (qcril_uim.card_status.card[slot].application[i].perso_feature == perso_params.perso_feature) &&
         (qcril_uim.card_status.card[slot].application[i].perso_state == QMI_UIM_PERSO_STATE_CODE_REQUIRED))
      {
         num_retries = qcril_uim.card_status.card[slot].application[i].perso_retries;
         qcril_uim_response(params_ptr->instance_id, params_ptr->t, RIL_E_SUCCESS,
                            (void*)&num_retries, sizeof(int), TRUE, "sending num perso retries");
         return;
      }
    }

    /* If perso retries are not available in QCRIL, retrieve them from modem
       using Get Configuration command. */
    if (qcril_uim_get_perso_retries(perso_params.perso_feature, &num_retries) == RIL_E_SUCCESS)
    {
      qcril_uim_response(params_ptr->instance_id, params_ptr->t, RIL_E_SUCCESS,
                         (void*)&num_retries, sizeof(int), TRUE, "sending num perso retries");
      return;
    }

    QCRIL_LOG_ERROR("Unable to find app that requires key for perso feature 0x%x",
                     perso_params.perso_feature);
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, NULL);
    return;
  }

  /* Perso operation */
  perso_params.perso_operation = QMI_UIM_PERSO_OP_DEACTIVATE;

  /* New PIN value */
  perso_params.ck_data.data_ptr = depersonalization_code;
  perso_params.ck_data.data_len = strlen(depersonalization_code);

  /* Allocate original request */
  original_request_ptr = qcril_uim_allocate_orig_request(params_ptr->instance_id,
                                                         modem_id,
                                                         (RIL_Token)params_ptr->t,
                                                         params_ptr->event_id,
                                                         0);
  if (original_request_ptr == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE,
                       "error allocating memory for original_request_ptr");
    return;
  }

  /* Proceed with depersonalization */
  QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "depersonalization" );
  res = qcril_uim_queue_send_request(QCRIL_UIM_REQUEST_DEPERSO,
                                     qcril_uim.qmi_handle,
                                     &perso_params,
                                     qmi_uim_callback,
                                     (void*)original_request_ptr);
  if (res < 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, NULL);
    /* Clean up any original request if allocated */
    qcril_free(original_request_ptr);
    original_request_ptr = NULL;
  }
} /* qcril_uim_request_enter_perso_key */


/*===========================================================================

  FUNCTION:  qcril_uim_request_perso_reactivate

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_perso_reactivate
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  int                                    res                  = 0;
  qcril_uim_original_request_type      * original_request_ptr = NULL;
  qmi_uim_personalization_params_type    perso_params;
  RIL_PersonalizationReq                 reactivate_req;
  uint8                                  offset = 0;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if (params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    return;
  }

  if (params_ptr->data == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t, RIL_E_GENERIC_FAILURE,
                       NULL, 0, TRUE, "NULL request pointer");
    return;
  }

  /* For OEMHook messages, client handlers need to parse the
     request byte stream themselves */
  memset(&reactivate_req, 0x00, sizeof(RIL_PersonalizationReq));
  reactivate_req.controlKey = (char *)params_ptr->data;
  if (strlen(reactivate_req.controlKey) == 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t, RIL_E_GENERIC_FAILURE,
                       NULL, 0, TRUE, "Empty controlKey not supported");
    return;
  }

  offset = strlen(reactivate_req.controlKey) + 1;
  reactivate_req.persoType = *((uint8*)params_ptr->data + offset);

  QCRIL_LOG_INFO( "qcril_uim_request_perso_reactivate for persoType (0x%x)\n",
                  reactivate_req.persoType );

  /* Add entry to ReqList */
  QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr);

  memset(&perso_params, 0x00, sizeof(qmi_uim_personalization_params_type));

  /* Reactivate Perso feature */
  switch (reactivate_req.persoType)
  {
    case RIL_PERSOSUBSTATE_SIM_NETWORK:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_NW;
      break;
    case RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_NS;
      break;
    case RIL_PERSOSUBSTATE_SIM_CORPORATE:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_CP;
      break;
    case RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_SP;
      break;
    case RIL_PERSOSUBSTATE_SIM_SIM:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_SIM;
      break;
    case RIL_PERSOSUBSTATE_RUIM_NETWORK1:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_NW1;
      break;
    case RIL_PERSOSUBSTATE_RUIM_NETWORK2:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_NW2;
      break;
    case RIL_PERSOSUBSTATE_RUIM_HRPD:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_HRPD;
      break;
    case RIL_PERSOSUBSTATE_RUIM_CORPORATE:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_CP;
      break;
    case RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_SP;
      break;
    case RIL_PERSOSUBSTATE_RUIM_RUIM:
      perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_RUIM;
      break;
    default:
      /* Invalid feature ID */
      QCRIL_LOG_ERROR( " Unsupported perso feature from RIL Request: 0x%x\n", reactivate_req.persoType);
      qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                         RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0, TRUE, NULL);
      return;
  }

  /* CK value */
  perso_params.ck_data.data_ptr = (unsigned char *)reactivate_req.controlKey;
  perso_params.ck_data.data_len = strlen(reactivate_req.controlKey);

  /* Allocate original request */
  original_request_ptr = qcril_uim_allocate_orig_request(params_ptr->instance_id,
                                                         QCRIL_MAX_MODEM_ID - 1,
                                                         (RIL_Token)params_ptr->t,
                                                         params_ptr->event_id,
                                                         0);
  if (original_request_ptr == NULL)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE,
                       "error allocating memory for original_request_ptr");
    return;
  }

  /* Proceed with depersonalization */
  QCRIL_LOG_QMI( QCRIL_MAX_MODEM_ID - 1, "qmi_uim_service", "personalization" );
  res = qcril_uim_queue_send_request(QCRIL_UIM_REQUEST_PERSO,
                                     qcril_uim.qmi_handle,
                                     &perso_params,
                                     qmi_uim_callback,
                                     (void*)original_request_ptr);
  if (res < 0)
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t,
                       RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, NULL);
    /* Clean up any original request if allocated */
    qcril_free(original_request_ptr);
    original_request_ptr = NULL;
  }
} /* qcril_uim_request_perso_reactivate */


/*===========================================================================

  FUNCTION:  qcril_uim_request_perso_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_PERSONALIZATION_STATUS_REQ requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_perso_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  int                                    ret      = 0;
  uint8                                  i        = 0;
  qmi_uim_rsp_data_type                  rsp_data;
  RIL_PersonalizationStatusResp          perso_status_resp;
  qmi_uim_get_configuration_params_type  get_config_params;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if (params_ptr == NULL || ret_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    return;
  }

  /* Add entry to ReqList */
  QCRIL_UIM_ADD_ENTRY_TO_REQUEST_LIST(params_ptr);

  /* Check which perso features are enabled & return the number of retries */
  memset(&rsp_data, 0x00, sizeof(qmi_uim_rsp_data_type));
  memset(&perso_status_resp, 0x00, sizeof(RIL_PersonalizationStatusResp));
  memset(&get_config_params, 0x00, sizeof(qmi_uim_get_configuration_params_type));

  get_config_params.perso_status = QMI_UIM_TRUE;
  ret = qcril_qmi_uim_get_configuration(qcril_uim.qmi_handle,
                                        &get_config_params,
                                        NULL,
                                        NULL,
                                        &rsp_data);

  if ((ret != 0) || (!rsp_data.rsp_data.get_configuration_rsp.perso_status_valid))
  {
    qcril_uim_response(params_ptr->instance_id, params_ptr->t, RIL_E_GENERIC_FAILURE,
                       (void*)&perso_status_resp, sizeof(RIL_PersonalizationStatusResp),
                       TRUE, "Unable to retrieve status");
    return ;
  }

  for (i = 0;
       i < rsp_data.rsp_data.get_configuration_rsp.perso_status_len && i < QMI_UIM_MAX_PERSO_FEATURES;
       i++)
  {
    switch (rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature)
    {
      case QMI_UIM_PERSO_FEATURE_GW_NW:
        perso_status_resp.has_gwNWPersoStatus = TRUE;
        perso_status_resp.gwNWPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_NS:
        perso_status_resp.has_gwNWSubsetPersoStatus = TRUE;
        perso_status_resp.gwNWSubsetPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_SP:
        perso_status_resp.has_gwSPPersoStatus = TRUE;
        perso_status_resp.gwSPPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_CP:
        perso_status_resp.has_gwCPPersoStatus = TRUE;
        perso_status_resp.gwCPPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_SIM:
        perso_status_resp.has_gwSIMPersoStatus = TRUE;
        perso_status_resp.gwSIMPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_1X_NW1:
        perso_status_resp.has_gw1xNWType1PersoStatus = TRUE;
        perso_status_resp.gw1xNWType1PersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_1X_NW2:
        perso_status_resp.has_gw1xNWType2PersoStatus = TRUE;
        perso_status_resp.gw1xNWType2PersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_1X_RUIM:
        perso_status_resp.has_gw1xRUIMPersoStatus = TRUE;
        perso_status_resp.gw1xRUIMPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      default:
        QCRIL_LOG_INFO("Unsupported perso feature, skipping: 0x%x",
                         rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature);
        break;
    }
  }

  qcril_uim_response(params_ptr->instance_id, params_ptr->t, RIL_E_SUCCESS,
                       (void*)&perso_status_resp, sizeof(RIL_PersonalizationStatusResp), TRUE,
                     "sending num perso retries");
} /* qcril_uim_request_perso_status */


/*===========================================================================

  FUNCTION:  qcril_uim_process_simlock_temp_unlock_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI SIMLOCK temporary unlock indication.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_simlock_temp_unlock_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr,
  qcril_request_return_type               * const ret_ptr /*!< Output parameter */
)
{
  uint8               slot = 0;
  char                temp_buff[100] = {0};

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if ((ind_param_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid inputs, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  slot = qcril_uim_instance_id_to_slot(ind_param_ptr->instance_id);
  if( slot >= QMI_UIM_MAX_CARD_COUNT )
  {
    QCRIL_LOG_ERROR("Invalid slot 0x%x for instance id 0x%x",
                     slot, ind_param_ptr->instance_id);
    return;
  }

  (void)snprintf(temp_buff, 99, "%s_%d", QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS, slot);

  QCRIL_LOG_INFO( "RSU status:0x%x and received status:0x%x \n",
           qcril_uim.temp_unlock_status,
           ind_param_ptr->ind_data.simlock_temp_unlock_ind.temporay_unlock_status[slot]);

  if (!ind_param_ptr->ind_data.simlock_temp_unlock_ind.temporay_unlock_status[slot] &&
      qcril_uim.temp_unlock_status)
  {
    /* Send OEMHook unsolicited response to framework */
    qcril_hook_unsol_response(slot,
                              (int) QCRIL_EVT_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED,
                              NULL,
                              0);
    QCRIL_LOG_INFO( "QCRIL_EVT_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED is sent \n");
  }
  if (ind_param_ptr->ind_data.simlock_temp_unlock_ind.temporay_unlock_status[slot] !=
      qcril_uim.temp_unlock_status)
  {
    qcril_uim.temp_unlock_status =
      ind_param_ptr->ind_data.simlock_temp_unlock_ind.temporay_unlock_status[slot];
    switch(qcril_uim.temp_unlock_status)
    {
      case TRUE:
        property_set(temp_buff, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_TRUE);
        break;
      case FALSE:
        property_set(temp_buff, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_FALSE);
        break;
    }

  }
} /* qcril_uim_process_simlock_temp_unlock_ind */


/*===========================================================================

  FUNCTION:  qcril_uim_check_and_send_temp_unlock_expiry_ind

===========================================================================*/
/*!
    @brief
    Function to check the temperory unlock status and send expiry indication

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_check_and_send_temp_unlock_expiry_ind
(
  void
)
{

  int                                    ret    = 0;
  qmi_uim_get_configuration_params_type  get_config_params;
  qmi_uim_rsp_data_type                  rsp_data;
  uint8                                  slot   =  (uint8)qmi_ril_get_sim_slot();
  char                                   temp_buff[100] = {0};

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if( slot >= QMI_UIM_MAX_CARD_COUNT )
  {
    QCRIL_LOG_ERROR("Invalid slot 0x%x", slot);
    return;
  }

  (void)snprintf(temp_buff, 99, "%s_%d", QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS, slot);
  memset(&get_config_params, 0x00, sizeof(get_config_params));
  memset(&rsp_data, 0x00, sizeof(rsp_data));

  get_config_params.perso_status = QMI_UIM_TRUE;
  ret = qcril_qmi_uim_get_configuration(qcril_uim.qmi_handle,
                                        &get_config_params,
                                        NULL,
                                        NULL,
                                        &rsp_data);

  if (ret != 0)
  {
    return;
  }

  QCRIL_LOG_INFO( "RSU status:0x%x and received status:0x%x \n",
           qcril_uim.temp_unlock_status,
           rsp_data.rsp_data.get_configuration_rsp.temp_unlock_status[slot]);

  if (!rsp_data.rsp_data.get_configuration_rsp.temp_unlock_status[slot] &&
      qcril_uim.temp_unlock_status)
  {
    /* Send OEMHook unsolicited response to framework */
    qcril_hook_unsol_response(slot,
                              (int) QCRIL_EVT_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED,
                              NULL,
                              0);
    QCRIL_LOG_INFO( "QCRIL_EVT_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED is sent \n");
    qcril_uim.temp_unlock_status = FALSE;
    property_set(temp_buff, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_FALSE);
  }
  else if (!qcril_uim.temp_unlock_status &&
      rsp_data.rsp_data.get_configuration_rsp.temp_unlock_status[slot])
  {
    qcril_uim.temp_unlock_status = TRUE;
    property_set(temp_buff, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_TRUE);
  }
} /* qcril_uim_check_and_send_temp_unlock_expiry_ind */

#endif /* defined (FEATURE_QCRIL_UIM_QMI) */
