/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved
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

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/25/13   tl      Initial version

===========================================================================*/

/*===========================================================================

                            INCLUDE FILES

===========================================================================*/

#include "cri_uim_core.h"
#include "cri_uim_utils.h"
#include "cri_core.h"
#include "cri_rule_handler.h"
#include "common_v01.h"

/*===========================================================================

                           GLOBALS

===========================================================================*/

uim_card_status_type_v01      uim_card_status;
int                           uim_client_id;

/*===========================================================================

                           FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  cri_uim_reset_card_status

===========================================================================*/
/*
    @brief
    Initalizes the cached card status.

    @return
    None
*/
/*=========================================================================*/
static void cri_uim_reset_card_status
(
  void
)
{
  int i = 0;
  int j = 0;

  memset(&uim_card_status, 0x00, sizeof(uim_card_status_type_v01));

  uim_card_status.index_gw_pri = MCM_UIM_INVALID_SESSION_VALUE;
  uim_card_status.index_1x_pri = MCM_UIM_INVALID_SESSION_VALUE;
  uim_card_status.index_gw_sec = MCM_UIM_INVALID_SESSION_VALUE;
  uim_card_status.index_1x_sec = MCM_UIM_INVALID_SESSION_VALUE;

  for (i = 0; i< QMI_UIM_CARDS_MAX_V01; i++)
  {
    uim_card_status.card_info[i].card_state      = UIM_CARD_STATE_ABSENT_V01;
    uim_card_status.card_info[i].upin.pin_state  = UIM_PIN_STATE_UNKNOWN_V01;
    uim_card_status.card_info[i].error_code      = UIM_CARD_ERROR_CODE_UNKNOWN_V01;

    for (j = 0; j < QMI_UIM_APPS_MAX_V01; j++)
    {
      uim_card_status.card_info[i].app_info[j].app_type       = UIM_APP_TYPE_UNKNOWN_V01;
      uim_card_status.card_info[i].app_info[j].app_state      = UIM_APP_STATE_UNKNOWN_V01;
      uim_card_status.card_info[i].app_info[j].perso_state    = UIM_PERSO_STATE_UNKNOWN_V01;
      uim_card_status.card_info[i].app_info[j].perso_feature  = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
      uim_card_status.card_info[i].app_info[j].univ_pin       = UIM_UNIV_PIN_PIN1_USED_V01;
      uim_card_status.card_info[i].app_info[j].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
      uim_card_status.card_info[i].app_info[j].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
    }
  }

  UTIL_LOG_MSG("\ncard status reset SUCCESSFUL\n");
} /* cri_uim_reset_card_status */


/*===========================================================================

  FUNCTION:  cri_uim_card_status_event_reg

===========================================================================*/
/*
    @brief
    Function registers card status indication events for the MCM UIM
    service.

    Function makes synchronous call to the modem.

    @return
    None
*/
/*=========================================================================*/
static qmi_error_type_v01 cri_uim_card_status_event_reg
(
  void
)
{
  qmi_error_type_v01                 cri_status        = QMI_ERR_NONE_V01;
  uim_event_reg_req_msg_v01          qmi_request;
  uim_event_reg_resp_msg_v01         qmi_response;

  UTIL_LOG_MSG("\nregister for card status indications START\n");

  memset(&qmi_request, 0x00, sizeof(uim_event_reg_req_msg_v01));
  memset(&qmi_response, 0x00, sizeof(uim_event_reg_resp_msg_v01));

  /* Bit 0 of event mask - Card status */
  qmi_request.event_mask = 1;

  cri_status = cri_core_qmi_send_msg_sync(uim_client_id,
                                          QMI_UIM_EVENT_REG_REQ_V01,
                                          (void*) &qmi_request,
                                          sizeof(qmi_request),
                                          (void*) &qmi_response,
                                          sizeof(qmi_response),
                                          CRI_CORE_MINIMAL_TIMEOUT);

  if(cri_status == QMI_ERR_NONE_V01)
  {
    if (qmi_response.resp.result == QMI_RESULT_SUCCESS_V01)
    {
      cri_status = QMI_ERR_NONE_V01;
    }
    else
    {
      UTIL_LOG_MSG("\ncri_uim_card_status_event_reg: ERROR: 0x%x\n", qmi_response.resp.error);
      return qmi_response.resp.error;
    }
  }
  else
  {
    UTIL_LOG_MSG("\ncri_uim_card_status_event_reg: failed to send QMI message: 0x%x\n", cri_status);
    return QMI_ERR_INTERNAL_V01;
  }

  return cri_status;
} /* cri_uim_card_status_event_reg */


/*===========================================================================

  FUNCTION:  cri_uim_refresh_event_reg

===========================================================================*/
/*
    @brief
    Function registers refresh indication events for the MCM UIM
    service.

    Function makes synchronous call to the modem.

    @return
    None
*/
/*=========================================================================*/
static void cri_uim_refresh_event_reg
(
  void
)
{
  qmi_error_type_v01                       cri_status       = QMI_ERR_NONE_V01;
  uim_refresh_register_all_req_msg_v01     qmi_request;
  uim_refresh_register_all_resp_msg_v01    qmi_response;
  int                                      session_index    = 0;
  uim_session_type_enum_v01                session_table[] = {UIM_SESSION_TYPE_PRIMARY_GW_V01,
                                                              UIM_SESSION_TYPE_PRIMARY_1X_V01,
                                                              UIM_SESSION_TYPE_SECONDARY_GW_V01,
                                                              UIM_SESSION_TYPE_SECONDARY_1X_V01};

  UTIL_LOG_MSG("\nregister for refresh START\n");

  for (session_index = 0; session_index < (sizeof(session_table)/sizeof(uim_session_type_enum_v01)); session_index++)
  {
    memset(&qmi_request, 0x00, sizeof(uim_refresh_register_all_req_msg_v01));
    memset(&qmi_response, 0x00, sizeof(uim_refresh_register_all_resp_msg_v01));

    qmi_request.session_information.session_type = session_table[session_index];

    /* indicate that we are registering for refresh */
    qmi_request.register_for_refresh = 1;

    cri_status = cri_core_qmi_send_msg_sync( uim_client_id,
                                             QMI_UIM_REFRESH_REGISTER_ALL_REQ_V01,
                                             (void*) &qmi_request,
                                             sizeof(qmi_request),
                                             (void*) &qmi_response,
                                             sizeof(qmi_response),
                                             CRI_CORE_MINIMAL_TIMEOUT);

    if(cri_status == QMI_ERR_NONE_V01)
    {
      if (qmi_response.resp.result != QMI_RESULT_SUCCESS_V01)
      {
        UTIL_LOG_MSG("\nFailed to regiester for refresh for session: 0x%x error: 0x%x\n", session_index,
                                                                                          qmi_response.resp.error);
        continue;
      }
    }
    else
    {
      UTIL_LOG_MSG("\nError sending refresh registeration QMI message: 0x%x\n", cri_status);
      continue;
    }
  }
} /* cri_uim_refresh_event_reg */


/*=========================================================================

  FUNCTION:  cri_uim_get_card_status

===========================================================================*/
/*
    @brief
    This function sends a synchronous card status request down to the modem
    and initalizes the card status cached in the MCM CRI core context.
    No card status information is returned to the client when calling
    this command.

    Function makes synchronous call to the modem.

    @return
    qmi_error_type_v01
*/
/*=========================================================================*/
static qmi_error_type_v01 cri_uim_get_card_status
(
  void
)
{
  qmi_error_type_v01                  cri_status        = QMI_ERR_NONE_V01;
  uim_get_card_status_resp_msg_v01  * qmi_response_ptr  = NULL;

  UTIL_LOG_MSG("\nget initial card status START\n", cri_status);

  qmi_response_ptr = (uim_get_card_status_resp_msg_v01*)malloc(sizeof(uim_get_card_status_resp_msg_v01));
  if (qmi_response_ptr == NULL)
  {
    UTIL_LOG_MSG("Failed to allocate memory for get card status\n");
    return QMI_ERR_NO_MEMORY_V01;
  }
  memset(qmi_response_ptr, 0x00, sizeof(uim_get_card_status_resp_msg_v01));

  cri_status = cri_core_qmi_send_msg_sync(uim_client_id,
                                          QMI_UIM_GET_CARD_STATUS_REQ_V01,
                                          NULL,
                                          NIL,
                                          (void*) qmi_response_ptr,
                                          sizeof(uim_get_card_status_resp_msg_v01),
                                          CRI_CORE_MINIMAL_TIMEOUT);

  if(cri_status == QMI_ERR_NONE_V01)
  {
    if (qmi_response_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
    {
      cri_status = QMI_ERR_NONE_V01;
    }
    else
    {
      cri_status = qmi_response_ptr->resp.error;
      free(qmi_response_ptr);
      return cri_status;
    }
  }
  else
  {
    UTIL_LOG_MSG("\nError sending message to QMI: 0x%x\n", cri_status);
    free(qmi_response_ptr);
    return QMI_ERR_INTERNAL_V01;
  }

  if(qmi_response_ptr->card_status_valid == TRUE)
  {
    memcpy(&uim_card_status,
           &qmi_response_ptr->card_status,
           sizeof(uim_card_status_type_v01));
  }

  UTIL_LOG_MSG("\nget card status success\n");

  free(qmi_response_ptr);
  return cri_status;
} /* cri_uim_get_card_status */


/*=========================================================================

  FUNCTION:  cri_uim_init_client_state

===========================================================================*/
/*
    @brief


    @return
    None
*/
/*=========================================================================*/
static void cri_uim_init_client_state()
{
  cri_core_error_type    cri_status   = QMI_ERR_NONE_V01;

  /* Register for card status indication from QMI UIM with the server
     Clients will then register for events through the MCM framework */
  cri_status = cri_uim_card_status_event_reg();

  if (cri_status != QMI_ERR_NONE_V01)
  {
    UTIL_LOG_MSG("\nError register with QMI for card status events: 0x%x\n",cri_status);
    return;
  }

  UTIL_LOG_MSG("\ncri_uim_card_status_event_reg SUCCESS: 0x%x\n",cri_status);

  /* Register for refresh indication from QMI UIM with the server
     Clients will then register for events through the MCM framework */
  cri_uim_refresh_event_reg();

  /* Get card status just in case modem is done initializing already */
  cri_status = cri_uim_get_card_status();

  if (cri_status != QMI_ERR_NONE_V01)
  {
    UTIL_LOG_MSG("\nFailed to initalize Card Status globals: %d\n",
                  cri_status);
  }
} /* cri_uim_init_client_state */


/*=========================================================================

  FUNCTION:  cri_uim_init_client

===========================================================================*/
/*
    @brief
    Performs the following task to initalize the UIM service:
      - resets the uim cache
      - registers as a service client of QMI UIM
      - registers for card status and refresh events
      - initalizes the uim cache

    @return
    cri_core_error_type
*/
/*=========================================================================*/
cri_core_error_type cri_uim_init_client
(
  hlos_ind_cb_type hlos_ind_cb
)
{
  cri_core_error_type client_init_error = QMI_ERR_INTERNAL_V01;

  UTIL_LOG_MSG("\nIn cri_uim_utils_init_client\n");

  /* Initalize mcm_uim global structure */
  cri_uim_reset_card_status();

  uim_client_id = cri_core_create_qmi_service_client(QMI_UIM_SERVICE, hlos_ind_cb);

  if(QMI_ERR_INTERNAL_V01 != uim_client_id)
  {
    UTIL_LOG_MSG("\ncri_core_create_qmi_service_client SUCCESS\n");
    client_init_error = QMI_ERR_NONE_V01;
    cri_uim_init_client_state();
  }

  return client_init_error;
} /* cri_uim_init_client */


/*=========================================================================

  FUNCTION:  cri_uim_core_unsol_ind_handler

===========================================================================*/
/*
    @brief
    Retrieves and calls the corresponding HLOS response handler callback function

    @return
    None
*/
/*=========================================================================*/
void cri_uim_core_unsol_ind_handler
(
  int qmi_service_client_id,
  unsigned long message_id,
  void *ind_data,
  int ind_data_len
)
{
  hlos_ind_cb_type hlos_ind_cb;

  hlos_ind_cb = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
  if(hlos_ind_cb )
  {
      (*hlos_ind_cb) (message_id, ind_data, ind_data_len);
  }
} /* cri_uim_core_unsol_ind_handler */


/*=========================================================================

  FUNCTION:  cri_uim_core_async_resp_handler

===========================================================================*/
/*
    @brief
    Calls the corresponding HLOS response handler callback function

    @return
    None
*/
/*=========================================================================*/
void cri_uim_core_async_resp_handler
(
  int qmi_service_client_id,
  unsigned long message_id,
  void *resp_data,
  int resp_data_len,
  cri_core_context_type cri_core_context
)
{
  cri_rule_handler_rule_check(cri_core_context,
                              QMI_ERR_NONE_V01,
                              resp_data);
} /* cri_uim_core_async_resp_handler */
