
/*===========================================================================

  Copyright (c) 2010-2015 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_uim_queue.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/18/15   at      Perso reactivation support
12/01/14   hh      Support for get MCC and MNC
11/12/14   at      QCRIL UIM SAP support
08/20/14   at      Support for graceful UICC Voltage supply deactivation
06/18/14   at      Support for SelectNext using reselect QMI command
06/11/14   at      Support for open logical channel API
05/14/14   yt      Support for STATUS command as part of SIM_IO request
01/17/14   at      Changed the feature checks for RIL_REQUEST_SIM_GET_ATR
12/11/13   at      Switch to new QCCI framework
11/19/13   at      Changed the feature checks for streaming APDU APIs
01/14/13   yt      Fix critical KW errors
10/08/12   at      Support for ISIM Authentication API
05/29/12   at      Reset the pending request count during SS Restart
04/09/12   at      Added support for RIL_REQUEST_SIM_GET_ATR
03/22/12   at      Replacing malloc()/free() with qcril_malloc()/qcril_free()
03/20/12   tl      Transition to QCCI
08/30/11   yt      Fixed Klocwork errors
04/11/11   yt      Support for modem restart and silent PIN1 verification
03/30/11   at      Support for logical channel & send apdu commands
01/11/11   at      Added support for QCRIL_UIM_REQUEST_REFRESH_REGISTER
12/03/10   at      Added qcril_uim_queue_mutex for protecting queue params
11/11/10   at      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#if defined (FEATURE_QCRIL_UIM_QMI)

#include "qcrili.h"
#include "qcril_uim.h"
#include "qcril_log.h"
#include "qcril_uim_util.h"
#include "qcril_uim_queue.h"
#include "qcril_uim_qcci.h"
#include <pthread.h>


/*===========================================================================

                           GLOBALS

===========================================================================*/

static uint8                                qcril_uim_pending_qmi_requests = 0;
static qcril_uim_queue_request_entry_type * qcril_uim_queue_head_ptr       = NULL;
static pthread_mutex_t                      qcril_uim_queue_mutex          = PTHREAD_MUTEX_INITIALIZER;

/*=========================================================================

  FUNCTION:  qcril_uim_queue_add

===========================================================================*/
/*!
    @brief
    This function performs a deep copy of the request and then adds it
    to the global linked list.

    @return
    Error code, 0 if no error.
*/
/*=========================================================================*/
static int qcril_uim_queue_add
(
  qcril_uim_request_type                     request_type,
  qmi_client_type                            qmi_handle,
  const void                               * param_data_ptr,
  qmi_uim_user_async_cb_type                 callback_function_ptr,
  const qcril_uim_original_request_type    * original_request_ptr
)
{
  qcril_uim_queue_request_entry_type   * request_ptr  = NULL;

  /* Allocate request */
  request_ptr = (qcril_uim_queue_request_entry_type*)qcril_malloc(sizeof(qcril_uim_queue_request_entry_type));
  if (request_ptr == NULL)
  {
    return -1;
  }

  /* Update the parameters */
  memset(request_ptr, 0, sizeof(qcril_uim_queue_request_entry_type));
  request_ptr->request_type          = request_type;
  request_ptr->qmi_handle            = qmi_handle;
  request_ptr->callback_function_ptr = callback_function_ptr;
  request_ptr->original_request_ptr  = (qcril_uim_original_request_type*)original_request_ptr;

  /* Perform a deep copy where needed */
  switch(request_type)
  {
    case QCRIL_UIM_REQUEST_READ_TRANSPARENT:
      memcpy(&request_ptr->params.read_transparent,
             param_data_ptr,
             sizeof(qmi_uim_read_transparent_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.read_transparent.session_info.aid.data_ptr,
                          ((qmi_uim_read_transparent_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_read_transparent_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.read_transparent.file_id.path.data_ptr,
                          ((qmi_uim_read_transparent_params_type*)param_data_ptr)->file_id.path.data_ptr,
                          ((qmi_uim_read_transparent_params_type*)param_data_ptr)->file_id.path.data_len);
      break;

    case QCRIL_UIM_REQUEST_READ_RECORD:
      memcpy(&request_ptr->params.read_record,
             param_data_ptr,
             sizeof(qmi_uim_read_record_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.read_record.session_info.aid.data_ptr,
                          ((qmi_uim_read_record_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_read_record_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.read_record.file_id.path.data_ptr,
                          ((qmi_uim_read_record_params_type*)param_data_ptr)->file_id.path.data_ptr,
                          ((qmi_uim_read_record_params_type*)param_data_ptr)->file_id.path.data_len);
      break;

    case QCRIL_UIM_REQUEST_WRITE_TRANSPARENT:
      memcpy(&request_ptr->params.write_transparent,
             param_data_ptr,
             sizeof(qmi_uim_write_transparent_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.write_transparent.session_info.aid.data_ptr,
                          ((qmi_uim_write_transparent_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_write_transparent_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.write_transparent.file_id.path.data_ptr,
                          ((qmi_uim_write_transparent_params_type*)param_data_ptr)->file_id.path.data_ptr,
                          ((qmi_uim_write_transparent_params_type*)param_data_ptr)->file_id.path.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.write_transparent.data.data_ptr,
                          ((qmi_uim_write_transparent_params_type*)param_data_ptr)->data.data_ptr,
                          ((qmi_uim_write_transparent_params_type*)param_data_ptr)->data.data_len);
      break;

    case QCRIL_UIM_REQUEST_WRITE_RECORD:
      memcpy(&request_ptr->params.write_record,
             param_data_ptr,
             sizeof(qmi_uim_write_record_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.write_record.session_info.aid.data_ptr,
                          ((qmi_uim_write_record_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_write_record_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.write_record.file_id.path.data_ptr,
                          ((qmi_uim_write_record_params_type*)param_data_ptr)->file_id.path.data_ptr,
                          ((qmi_uim_write_record_params_type*)param_data_ptr)->file_id.path.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.write_record.data.data_ptr,
                          ((qmi_uim_write_record_params_type*)param_data_ptr)->data.data_ptr,
                          ((qmi_uim_write_record_params_type*)param_data_ptr)->data.data_len);
      break;

    case QCRIL_UIM_REQUEST_GET_RESPONSE:
      memcpy(&request_ptr->params.get_attributes,
             param_data_ptr,
             sizeof(qmi_uim_get_file_attributes_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.get_attributes.session_info.aid.data_ptr,
                          ((qmi_uim_get_file_attributes_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_get_file_attributes_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.get_attributes.file_id.path.data_ptr,
                          ((qmi_uim_get_file_attributes_params_type*)param_data_ptr)->file_id.path.data_ptr,
                          ((qmi_uim_get_file_attributes_params_type*)param_data_ptr)->file_id.path.data_len);
      break;

    case QCRIL_UIM_REQUEST_REFRESH_REGISTER:
      memcpy(&request_ptr->params.refresh_register,
             param_data_ptr,
             sizeof(qmi_uim_refresh_register_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.refresh_register.session_info.aid.data_ptr,
                          ((qmi_uim_refresh_register_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_refresh_register_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.refresh_register.files_ptr,
                          ((qmi_uim_refresh_register_params_type*)param_data_ptr)->files_ptr,
                          (((qmi_uim_refresh_register_params_type*)param_data_ptr)->num_files) * sizeof(qmi_uim_file_id_type));
      break;

    case QCRIL_UIM_REQUEST_GET_FDN:
      memcpy(&request_ptr->params.get_service_status,
             param_data_ptr,
             sizeof(qmi_uim_get_service_status_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.get_service_status.session_info.aid.data_ptr,
                          ((qmi_uim_get_service_status_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_get_service_status_params_type*)param_data_ptr)->session_info.aid.data_len);
      break;

    case QCRIL_UIM_REQUEST_SET_FDN:
      memcpy(&request_ptr->params.set_service_status,
             param_data_ptr,
             sizeof(qmi_uim_set_service_status_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.set_service_status.session_info.aid.data_ptr,
                          ((qmi_uim_set_service_status_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_set_service_status_params_type*)param_data_ptr)->session_info.aid.data_len);
      break;

    case QCRIL_UIM_REQUEST_VERIFY_PIN:
      memcpy(&request_ptr->params.verify_pin,
             param_data_ptr,
             sizeof(qmi_uim_verify_pin_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.verify_pin.session_info.aid.data_ptr,
                          ((qmi_uim_verify_pin_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_verify_pin_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.verify_pin.pin_data.data_ptr,
                          ((qmi_uim_verify_pin_params_type*)param_data_ptr)->pin_data.data_ptr,
                          ((qmi_uim_verify_pin_params_type*)param_data_ptr)->pin_data.data_len);
      break;

    case QCRIL_UIM_REQUEST_UNBLOCK_PIN:
      memcpy(&request_ptr->params.unblock_pin,
             param_data_ptr,
             sizeof(qmi_uim_unblock_pin_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.unblock_pin.session_info.aid.data_ptr,
                          ((qmi_uim_unblock_pin_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_unblock_pin_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.unblock_pin.puk_data.data_ptr,
                          ((qmi_uim_unblock_pin_params_type*)param_data_ptr)->puk_data.data_ptr,
                          ((qmi_uim_unblock_pin_params_type*)param_data_ptr)->puk_data.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.unblock_pin.new_pin_data.data_ptr,
                          ((qmi_uim_unblock_pin_params_type*)param_data_ptr)->new_pin_data.data_ptr,
                          ((qmi_uim_unblock_pin_params_type*)param_data_ptr)->new_pin_data.data_len);
      break;

    case QCRIL_UIM_REQUEST_CHANGE_PIN:
      memcpy(&request_ptr->params.change_pin,
             param_data_ptr,
             sizeof(qmi_uim_change_pin_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.change_pin.session_info.aid.data_ptr,
                          ((qmi_uim_change_pin_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_change_pin_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.change_pin.old_pin_data.data_ptr,
                          ((qmi_uim_change_pin_params_type*)param_data_ptr)->old_pin_data.data_ptr,
                          ((qmi_uim_change_pin_params_type*)param_data_ptr)->old_pin_data.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.change_pin.new_pin_data.data_ptr,
                          ((qmi_uim_change_pin_params_type*)param_data_ptr)->new_pin_data.data_ptr,
                          ((qmi_uim_change_pin_params_type*)param_data_ptr)->new_pin_data.data_len);
      break;

    case QCRIL_UIM_REQUEST_SET_PIN:
      memcpy(&request_ptr->params.set_pin,
             param_data_ptr,
             sizeof(qmi_uim_set_pin_protection_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.set_pin.session_info.aid.data_ptr,
                          ((qmi_uim_set_pin_protection_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_set_pin_protection_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.set_pin.pin_data.data_ptr,
                          ((qmi_uim_set_pin_protection_params_type*)param_data_ptr)->pin_data.data_ptr,
                          ((qmi_uim_set_pin_protection_params_type*)param_data_ptr)->pin_data.data_len);
      break;

    case QCRIL_UIM_REQUEST_DEPERSO:
      memcpy(&request_ptr->params.deperso,
             param_data_ptr,
             sizeof(qmi_uim_depersonalization_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.deperso.ck_data.data_ptr,
                          ((qmi_uim_depersonalization_params_type*)param_data_ptr)->ck_data.data_ptr,
                          ((qmi_uim_depersonalization_params_type*)param_data_ptr)->ck_data.data_len);
      break;

    case QCRIL_UIM_REQUEST_POWER_UP:
      memcpy(&request_ptr->params.power_up,
             param_data_ptr,
             sizeof(qmi_uim_power_up_params_type));
      break;

    case QCRIL_UIM_REQUEST_POWER_DOWN:
      memcpy(&request_ptr->params.power_down,
             param_data_ptr,
             sizeof(qmi_uim_power_down_params_type));
      break;

    case QCRIL_UIM_REQUEST_CHANGE_PROV_SESSION:
      memcpy(&request_ptr->params.change_prov_session,
             param_data_ptr,
             sizeof(qmi_uim_change_prov_session_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.change_prov_session.app_info.aid.data_ptr,
                          ((qmi_uim_change_prov_session_params_type*)param_data_ptr)->app_info.aid.data_ptr,
                          ((qmi_uim_change_prov_session_params_type*)param_data_ptr)->app_info.aid.data_len);
      break;

    case QCRIL_UIM_REQUEST_AUTHENTICATE:
      memcpy(&request_ptr->params.authenticate,
             param_data_ptr,
             sizeof(qmi_uim_authenticate_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.authenticate.session_info.aid.data_ptr,
                          ((qmi_uim_authenticate_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_authenticate_params_type*)param_data_ptr)->session_info.aid.data_len);
      QCRIL_UIM_DUPLICATE(request_ptr->params.authenticate.auth_data.data_ptr,
                          ((qmi_uim_authenticate_params_type*)param_data_ptr)->auth_data.data_ptr,
                          ((qmi_uim_authenticate_params_type*)param_data_ptr)->auth_data.data_len);
      break;

    case QCRIL_UIM_REQUEST_SAP_CONNECTION:
      memcpy(&request_ptr->params.sap_connection,
             param_data_ptr,
             sizeof(qmi_uim_sap_connection_params_type));
      break;

    case QCRIL_UIM_REQUEST_SAP_REQUEST:
      memcpy(&request_ptr->params.sap_request,
             param_data_ptr,
             sizeof(qmi_uim_sap_request_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.sap_request.apdu.data_ptr,
                          ((qmi_uim_sap_request_params_type*)param_data_ptr)->apdu.data_ptr,
                          ((qmi_uim_sap_request_params_type*)param_data_ptr)->apdu.data_len);
      break;

    case QCRIL_UIM_REQUEST_LOGICAL_CHANNEL:
      memcpy(&request_ptr->params.logical_channel,
             param_data_ptr,
             sizeof(qmi_uim_logical_channel_params_type));
      if (request_ptr->params.logical_channel.operation_type == QMI_UIM_LOGICAL_CHANNEL_OPEN)
      {
        QCRIL_UIM_DUPLICATE(request_ptr->params.logical_channel.channel_data.aid.data_ptr,
                            ((qmi_uim_logical_channel_params_type*)param_data_ptr)->channel_data.aid.data_ptr,
                            ((qmi_uim_logical_channel_params_type*)param_data_ptr)->channel_data.aid.data_len);
      }
      break;

    case QCRIL_UIM_REQUEST_OPEN_LOGICAL_CHANNEL:
      memcpy(&request_ptr->params.open_logical_channel,
             param_data_ptr,
             sizeof(qmi_uim_open_logical_channel_params_type));
      if (((qmi_uim_open_logical_channel_params_type*)param_data_ptr)->aid.data_ptr)
      {
        QCRIL_UIM_DUPLICATE(request_ptr->params.logical_channel.channel_data.aid.data_ptr,
                            ((qmi_uim_open_logical_channel_params_type*)param_data_ptr)->aid.data_ptr,
                            ((qmi_uim_open_logical_channel_params_type*)param_data_ptr)->aid.data_len);
      }
      break;

    case QCRIL_UIM_REQUEST_SEND_APDU:
      memcpy(&request_ptr->params.send_apdu,
             param_data_ptr,
             sizeof(qmi_uim_send_apdu_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.send_apdu.apdu.data_ptr,
                          ((qmi_uim_send_apdu_params_type*)param_data_ptr)->apdu.data_ptr,
                          ((qmi_uim_send_apdu_params_type*)param_data_ptr)->apdu.data_len);
      break;

    case QCRIL_UIM_REQUEST_GET_ATR:
      memcpy(&request_ptr->params.get_atr,
             param_data_ptr,
             sizeof(qmi_uim_get_atr_params_type));
      break;

    case QCRIL_UIM_REQUEST_SEND_STATUS:
      memcpy(&request_ptr->params.send_status,
             param_data_ptr,
             sizeof(qmi_uim_status_cmd_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.send_status.session_info.aid.data_ptr,
                          ((qmi_uim_status_cmd_params_type*)param_data_ptr)->session_info.aid.data_ptr,
                          ((qmi_uim_status_cmd_params_type*)param_data_ptr)->session_info.aid.data_len);
      break;

    case QCRIL_UIM_REQUEST_RESELECT:
      memcpy(&request_ptr->params.reselect,
             param_data_ptr,
             sizeof(qmi_uim_reselect_params_type));
      break;

    case QCRIL_UIM_REQUEST_SUPPLY_VOLTAGE:
      memcpy(&request_ptr->params.supply_voltage,
             param_data_ptr,
             sizeof(qmi_uim_supply_voltage_params_type));
      break;

    case QCRIL_UIM_REQUEST_PERSO:
      memcpy(&request_ptr->params.perso,
             param_data_ptr,
             sizeof(qmi_uim_personalization_params_type));
      QCRIL_UIM_DUPLICATE(request_ptr->params.perso.ck_data.data_ptr,
                          ((qmi_uim_personalization_params_type*)param_data_ptr)->ck_data.data_ptr,
                          ((qmi_uim_personalization_params_type*)param_data_ptr)->ck_data.data_len);
      break;

    default:
      /* This should never happen! */
      qcril_free(request_ptr);
      return -1;
  }

  /* Traverse through the list and add this request entry at the end */
  if (qcril_uim_queue_head_ptr == NULL)
  {
    qcril_uim_queue_head_ptr = request_ptr;
  }
  else
  {
    qcril_uim_queue_request_entry_type * temp = qcril_uim_queue_head_ptr;
    while (temp != NULL && temp->queue_next_ptr != NULL)
    {
      temp = temp->queue_next_ptr;
    }
    temp->queue_next_ptr = request_ptr;
  }

  return 0;
} /* qcril_uim_queue_add */


/*=========================================================================

  FUNCTION:  qcril_uim_queue_remove_head

===========================================================================*/
/*!
    @brief
    This function removes a QMI request from the head of the queue,
    freeing the memory

    @return
    None.
*/
/*=========================================================================*/
static void qcril_uim_queue_remove_head
(
  void
)
{
  qcril_uim_queue_request_entry_type * next_request_ptr = NULL;

  if (qcril_uim_queue_head_ptr == NULL)
  {
    return;
  }

  /* Store the pointer to the next item */
  next_request_ptr = qcril_uim_queue_head_ptr->queue_next_ptr;

  /* Free the request */
  switch(qcril_uim_queue_head_ptr->request_type)
  {
    case QCRIL_UIM_REQUEST_READ_TRANSPARENT:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.read_transparent.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.read_transparent.file_id.path.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_READ_RECORD:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.read_record.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.read_record.file_id.path.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_WRITE_TRANSPARENT:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.write_transparent.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.write_transparent.file_id.path.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.write_transparent.data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_WRITE_RECORD:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.write_record.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.write_record.file_id.path.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.write_record.data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_RESPONSE:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.get_attributes.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.get_attributes.file_id.path.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_REFRESH_REGISTER:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.refresh_register.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.refresh_register.files_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_FDN:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.get_service_status.session_info.aid.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SET_FDN:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.set_service_status.session_info.aid.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_VERIFY_PIN:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.verify_pin.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.verify_pin.pin_data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_UNBLOCK_PIN:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.unblock_pin.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.unblock_pin.puk_data.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.unblock_pin.new_pin_data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_CHANGE_PIN:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.change_pin.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.change_pin.old_pin_data.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.change_pin.new_pin_data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SET_PIN:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.set_pin.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.set_pin.pin_data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_DEPERSO:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.deperso.ck_data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_CHANGE_PROV_SESSION:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.change_prov_session.app_info.aid.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_AUTHENTICATE:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.authenticate.session_info.aid.data_ptr);
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.authenticate.auth_data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SAP_REQUEST:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.sap_request.apdu.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_LOGICAL_CHANNEL:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.logical_channel.channel_data.aid.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_OPEN_LOGICAL_CHANNEL:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.open_logical_channel.aid.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SEND_APDU:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.send_apdu.apdu.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_ATR:
      break;

    case QCRIL_UIM_REQUEST_SEND_STATUS:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.send_status.session_info.aid.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_PERSO:
      QCRIL_UIM_FREE_IF_NOT_NULL(qcril_uim_queue_head_ptr->params.perso.ck_data.data_ptr);
      break;

    case QCRIL_UIM_REQUEST_POWER_UP:
    case QCRIL_UIM_REQUEST_POWER_DOWN:
    case QCRIL_UIM_REQUEST_RESELECT:
    case QCRIL_UIM_REQUEST_SUPPLY_VOLTAGE:
    case QCRIL_UIM_REQUEST_SAP_CONNECTION:
    default:
      break;
  }

  qcril_free(qcril_uim_queue_head_ptr);

  /* Set the new head of the queue */
  qcril_uim_queue_head_ptr = next_request_ptr;

} /* qcril_uim_queue_remove_head */


/*=========================================================================

  FUNCTION:  qcril_uim_queue_execute_request

===========================================================================*/
/*!
    @brief
    This function executes a QMI request, sending it down to the modem
    in an asynchronous way.

    @return
    Error code returned by QMI API.
*/
/*=========================================================================*/
static int qcril_uim_queue_execute_request
(
  qcril_uim_request_type                   request_type,
  qmi_client_type                          qmi_handle,
  const void                             * param_data_ptr,
  qmi_uim_user_async_cb_type               callback_function_ptr,
  const qcril_uim_original_request_type  * original_request_ptr
)
{
  int ret = -1;

  QCRIL_ASSERT( param_data_ptr != NULL );
  QCRIL_ASSERT( callback_function_ptr != NULL );
  QCRIL_ASSERT( original_request_ptr != NULL );

  switch(request_type)
  {
    case QCRIL_UIM_REQUEST_READ_TRANSPARENT:
      ret = qcril_qmi_uim_read_transparent(qmi_handle,
                                          (const qmi_uim_read_transparent_params_type*)param_data_ptr,
                                          callback_function_ptr,
                                          (void*)original_request_ptr,
                                          NULL);
      break;

    case QCRIL_UIM_REQUEST_READ_RECORD:
      ret = qcril_qmi_uim_read_record(qmi_handle,
                                     (const qmi_uim_read_record_params_type*)param_data_ptr,
                                     callback_function_ptr,
                                     (void*)original_request_ptr,
                                     NULL);
      break;

    case QCRIL_UIM_REQUEST_WRITE_TRANSPARENT:
      ret = qcril_qmi_uim_write_transparent(qmi_handle,
                                           (const qmi_uim_write_transparent_params_type*)param_data_ptr,
                                           callback_function_ptr,
                                           (void*)original_request_ptr,
                                           NULL);
      break;

    case QCRIL_UIM_REQUEST_WRITE_RECORD:
      ret = qcril_qmi_uim_write_record(qmi_handle,
                                      (const qmi_uim_write_record_params_type*)param_data_ptr,
                                      callback_function_ptr,
                                      (void*)original_request_ptr,
                                      NULL);
      break;

    case QCRIL_UIM_REQUEST_GET_RESPONSE:
      ret = qcril_qmi_uim_get_file_attributes(qmi_handle,
                                             (const qmi_uim_get_file_attributes_params_type*)param_data_ptr,
                                             callback_function_ptr,
                                             (void*)original_request_ptr,
                                             NULL);
      break;

    case QCRIL_UIM_REQUEST_REFRESH_REGISTER:
      ret = qcril_qmi_uim_refresh_register(qmi_handle,
                                          (const qmi_uim_refresh_register_params_type*)param_data_ptr,
                                          callback_function_ptr,
                                          (void*)original_request_ptr,
                                          NULL);
      break;

    case QCRIL_UIM_REQUEST_GET_FDN:
      ret = qcril_qmi_uim_get_service_status(qmi_handle,
                                            (const qmi_uim_get_service_status_params_type*)param_data_ptr,
                                            callback_function_ptr,
                                            (void*)original_request_ptr,
                                            NULL);
      break;

    case QCRIL_UIM_REQUEST_SET_FDN:
      ret = qcril_qmi_uim_set_service_status(qmi_handle,
                                            (const qmi_uim_set_service_status_params_type*)param_data_ptr,
                                            callback_function_ptr,
                                            (void*)original_request_ptr,
                                            NULL);
      break;

    case QCRIL_UIM_REQUEST_VERIFY_PIN:
      ret = qcril_qmi_uim_verify_pin(qmi_handle,
                                    (const qmi_uim_verify_pin_params_type*)param_data_ptr,
                                    callback_function_ptr,
                                    (void*)original_request_ptr,
                                    NULL);
      break;

    case QCRIL_UIM_REQUEST_UNBLOCK_PIN:
      ret = qcril_qmi_uim_unblock_pin(qmi_handle,
                                     (const qmi_uim_unblock_pin_params_type*)param_data_ptr,
                                     callback_function_ptr,
                                     (void*)original_request_ptr,
                                     NULL);
      break;

    case QCRIL_UIM_REQUEST_CHANGE_PIN:
      ret = qcril_qmi_uim_change_pin(qmi_handle,
                                    (const qmi_uim_change_pin_params_type*)param_data_ptr,
                                    callback_function_ptr,
                                    (void*)original_request_ptr,
                                    NULL);
      break;

    case QCRIL_UIM_REQUEST_SET_PIN:
      ret = qcril_qmi_uim_set_pin_protection(qmi_handle,
                                            (const qmi_uim_set_pin_protection_params_type*)param_data_ptr,
                                            callback_function_ptr,
                                            (void*)original_request_ptr,
                                            NULL);
      break;

    case QCRIL_UIM_REQUEST_DEPERSO:
      ret = qcril_qmi_uim_depersonalization(qmi_handle,
                                           (const qmi_uim_depersonalization_params_type*)param_data_ptr,
                                           callback_function_ptr,
                                           (void*)original_request_ptr,
                                           NULL);
      break;

    case QCRIL_UIM_REQUEST_POWER_UP:
      ret = qcril_qmi_uim_power_up(qmi_handle,
                                  (const qmi_uim_power_up_params_type*)param_data_ptr,
                                  callback_function_ptr,
                                  (void*)original_request_ptr,
                                  NULL);
      break;

    case QCRIL_UIM_REQUEST_POWER_DOWN:
      ret = qcril_qmi_uim_power_down(qmi_handle,
                                    (const qmi_uim_power_down_params_type*)param_data_ptr,
                                    callback_function_ptr,
                                    (void*)original_request_ptr,
                                    NULL);
      break;

    case QCRIL_UIM_REQUEST_CHANGE_PROV_SESSION:
      ret = qcril_qmi_uim_change_provisioning_session(qmi_handle,
                                                     (const qmi_uim_change_prov_session_params_type*)param_data_ptr,
                                                     callback_function_ptr,
                                                     (void*)original_request_ptr,
                                                     NULL);
      break;

    case QCRIL_UIM_REQUEST_AUTHENTICATE:
      ret = qcril_qmi_uim_authenticate(qmi_handle,
                                       (const qmi_uim_authenticate_params_type*)param_data_ptr,
                                       callback_function_ptr,
                                       (void*)original_request_ptr,
                                       NULL);
      break;

    case QCRIL_UIM_REQUEST_SAP_CONNECTION:
      ret = qcril_qmi_uim_sap_connection(qmi_handle,
                                         (const qmi_uim_sap_connection_params_type*)param_data_ptr,
                                         callback_function_ptr,
                                         (void*)original_request_ptr,
                                         NULL);
      break;

    case QCRIL_UIM_REQUEST_SAP_REQUEST:
      ret = qcril_qmi_uim_sap_request(qmi_handle,
                                      (const qmi_uim_sap_request_params_type*)param_data_ptr,
                                      callback_function_ptr,
                                      (void*)original_request_ptr,
                                      NULL);
      break;

    case QCRIL_UIM_REQUEST_LOGICAL_CHANNEL:
      ret = qcril_qmi_uim_logical_channel(qmi_handle,
                                         (const qmi_uim_logical_channel_params_type*)param_data_ptr,
                                         callback_function_ptr,
                                         (void*)original_request_ptr,
                                         NULL);
      break;

    case QCRIL_UIM_REQUEST_OPEN_LOGICAL_CHANNEL:
      ret = qcril_qmi_uim_open_logical_channel(
              qmi_handle,
              (const qmi_uim_open_logical_channel_params_type*)param_data_ptr,
              callback_function_ptr,
              (void*)original_request_ptr,
              NULL);
      break;

    case QCRIL_UIM_REQUEST_SEND_APDU:
      ret = qcril_qmi_uim_send_apdu(qmi_handle,
                                   (const qmi_uim_send_apdu_params_type*)param_data_ptr,
                                   callback_function_ptr,
                                   (void*)original_request_ptr,
                                   NULL);
      break;

    case QCRIL_UIM_REQUEST_GET_ATR:
      ret = qcril_qmi_uim_get_atr(qmi_handle,
                                  (const qmi_uim_get_atr_params_type*)param_data_ptr,
                                  callback_function_ptr,
                                  (void*)original_request_ptr,
                                  NULL);
      break;

    case QCRIL_UIM_REQUEST_SEND_STATUS:
      ret = qcril_qmi_uim_send_status(qmi_handle,
                                      (const qmi_uim_status_cmd_params_type*)param_data_ptr,
                                      callback_function_ptr,
                                      (void*)original_request_ptr,
                                      NULL);
      break;

    case QCRIL_UIM_REQUEST_RESELECT:
      ret = qcril_qmi_uim_reselect(qmi_handle,
                                   (const qmi_uim_reselect_params_type*)param_data_ptr,
                                   callback_function_ptr,
                                   (void*)original_request_ptr,
                                   NULL);
      break;

    case QCRIL_UIM_REQUEST_SUPPLY_VOLTAGE:
      ret = qcril_qmi_uim_supply_voltage(qmi_handle,
                                         (const qmi_uim_supply_voltage_params_type*)param_data_ptr,
                                         callback_function_ptr,
                                         (void*)original_request_ptr,
                                         NULL);
      break;

    case QCRIL_UIM_REQUEST_PERSO:
      ret = qcril_qmi_uim_personalization(qmi_handle,
                                         (const qmi_uim_personalization_params_type*)param_data_ptr,
                                           callback_function_ptr,
                                           (void*)original_request_ptr,
                                           NULL);
      break;

    default:
      ret = -1;
      break;
  }

  return ret;
} /* qcril_uim_queue_execute_request */


/*=========================================================================

  FUNCTION:  qcril_uim_queue_send_request

===========================================================================*/
/*!
    @brief
    This function checks if the incoming request has to be:
    i.   Sent directly to the modem - when it is detemined that queue size
         is low enough to proceed, or,
    ii.  Queued in the global buffer so that further processing will be
         performed - this is when specified queue size -
         QCRIL_UIM_MAX_QMI_QUEUE_SIZE limit is reached.

    @return
    Error code returned by QMI API.
*/
/*=========================================================================*/
int qcril_uim_queue_send_request
(
  qcril_uim_request_type                     request_type,
  qmi_client_type                            qmi_handle,
  const void                               * param_data_ptr,
  qmi_uim_user_async_cb_type                 callback_function_ptr,
  const qcril_uim_original_request_type    * original_request_ptr
)
{
  int ret = -1;

  if (param_data_ptr == NULL ||
      original_request_ptr == NULL ||
      callback_function_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return -1;
  }

  /* Note on the queue mutex: Entire send_request and complete_request is supposed
     to be an atomic operation. Also note that QMI APIs called when locked are aync. */
  QCRIL_MUTEX_LOCK(&qcril_uim_queue_mutex, "qcril_uim_queue_mutex");

  if (qcril_uim_pending_qmi_requests < QCRIL_UIM_MAX_QMI_QUEUE_SIZE)
  {
    QCRIL_LOG_INFO( "Pending QMI commands: %d -> Sending to modem for token: %d",
                    qcril_uim_pending_qmi_requests,
                    qcril_log_get_token_id((RIL_Token)original_request_ptr->token));

    /* If QMI Queue size is under limit, proceed with the call as before */
    ret = qcril_uim_queue_execute_request(request_type,
                                          qmi_handle,
                                          param_data_ptr,
                                          callback_function_ptr,
                                          original_request_ptr);
    if (ret >= 0)
    {
      qcril_uim_pending_qmi_requests++;
    }

    QCRIL_MUTEX_UNLOCK(&qcril_uim_queue_mutex, "qcril_uim_queue_mutex");
    return ret;
  }

  /* Queue the request */
  QCRIL_LOG_INFO( "Pending QMI commands: %d -> Putting in the queue for token: %d",
                  qcril_uim_pending_qmi_requests,
                  qcril_log_get_token_id((RIL_Token)original_request_ptr->token));

  ret = qcril_uim_queue_add(request_type,
                            qmi_handle,
                            param_data_ptr,
                            callback_function_ptr,
                            original_request_ptr);

  QCRIL_MUTEX_UNLOCK(&qcril_uim_queue_mutex, "qcril_uim_queue_mutex");

  return ret;
} /* qcril_uim_queue_send_request */


/*=========================================================================

  FUNCTION:  qcril_uim_queue_complete_request

===========================================================================*/
/*!
    @brief
    This function checks if the response in the callback has to be:
    i.   Only sent to Android FW - when it is detemined that queue is empty
         or,
    ii.  Checked if response was queued initially and hence has to be
         removed from the queue, queue parameters updated.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_queue_complete_request
(
  void
)
{
  int ret = -1;

  QCRIL_MUTEX_LOCK(&qcril_uim_queue_mutex, " qcril_uim_queue_mutex");

  /* Since we have received a callback, there must be a pending request! */
  QCRIL_ASSERT(qcril_uim_pending_qmi_requests > 0);

  /* If the queue is empty, simply decrease the number of pending requests */
  if (qcril_uim_queue_head_ptr == NULL)
  {
    if (qcril_uim_pending_qmi_requests > 0)
    {
      qcril_uim_pending_qmi_requests--;
    }

    QCRIL_LOG_INFO( "Remaining QMI commands: %d", qcril_uim_pending_qmi_requests);

    QCRIL_MUTEX_UNLOCK(&qcril_uim_queue_mutex, "qcril_uim_queue_mutex");

    return;
  }

  if(qcril_uim_queue_head_ptr->original_request_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s","NULL pointer");
    QCRIL_MUTEX_UNLOCK(&qcril_uim_queue_mutex, "qcril_uim_queue_mutex");
    QCRIL_ASSERT(0);
    return;
  }

  /* In this case, the number of pending requests does not decrease */
  QCRIL_LOG_INFO( "Sending new request from the queue for token: %d, pending in queue: %d",
                  qcril_log_get_token_id((RIL_Token)qcril_uim_queue_head_ptr->original_request_ptr->token),
                  qcril_uim_pending_qmi_requests);

  /* Execute the first item in the queue */
  ret = qcril_uim_queue_execute_request(qcril_uim_queue_head_ptr->request_type,
                                        qcril_uim_queue_head_ptr->qmi_handle,
                                        &qcril_uim_queue_head_ptr->params,
                                        qcril_uim_queue_head_ptr->callback_function_ptr,
                                        qcril_uim_queue_head_ptr->original_request_ptr);

  /* In case of error:
     1. Simulate immediate answer
     2. Decrease the number of pending requests by 1 */
  if (ret < 0)
  {
    if (qcril_uim_queue_head_ptr->original_request_ptr != NULL)
    {
      qcril_uim_remove_non_provisioning_session(qcril_uim_queue_head_ptr->original_request_ptr->token);
      qcril_uim_response(qcril_uim_queue_head_ptr->original_request_ptr->instance_id,
                         qcril_uim_queue_head_ptr->original_request_ptr->token,
                         RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, "error in QMI function");
      qcril_free(qcril_uim_queue_head_ptr->original_request_ptr);
      qcril_uim_queue_head_ptr->original_request_ptr = NULL;
    }

    if (qcril_uim_pending_qmi_requests > 0)
    {
      qcril_uim_pending_qmi_requests--;
    }
  }

  /* Remove head of the queue */
  qcril_uim_queue_remove_head();

  QCRIL_MUTEX_UNLOCK(&qcril_uim_queue_mutex, "qcril_uim_queue_mutex");

} /* qcril_uim_queue_complete_request */


/*=========================================================================

  FUNCTION:  qcril_uim_queue_cleanup

===========================================================================*/
/*!
    @brief
    This function checks if there are any requests in the queue. For each
    pending request, it sends generic error response to Android FW and
    removes the request from the queue.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_queue_cleanup
(
  void
)
{
  QCRIL_LOG_DEBUG("%s: Sending error responses for pending commands in QCRIL_UIM queue",
                  __FUNCTION__);

  QCRIL_MUTEX_LOCK(&qcril_uim_queue_mutex, " qcril_uim_queue_mutex");

  while(qcril_uim_queue_head_ptr != NULL)
  {
    if (qcril_uim_queue_head_ptr->original_request_ptr != NULL)
    {
      qcril_uim_response(qcril_uim_queue_head_ptr->original_request_ptr->instance_id,
                         qcril_uim_queue_head_ptr->original_request_ptr->token,
                         RIL_E_GENERIC_FAILURE, NULL, 0, TRUE, "Dropping request");
      qcril_free(qcril_uim_queue_head_ptr->original_request_ptr);
      qcril_uim_queue_head_ptr->original_request_ptr = NULL;
    }
    qcril_uim_queue_remove_head();
  }

  QCRIL_MUTEX_UNLOCK(&qcril_uim_queue_mutex, "qcril_uim_queue_mutex");
} /* qcril_uim_queue_cleanup */

#endif /* defined (FEATURE_QCRIL_UIM_QMI) */

