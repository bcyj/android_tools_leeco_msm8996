
/*===========================================================================

  Copyright (c) 2010-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/micheleb/ril/qcril_uim.c#8 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/23/15   vr      SIMLOCK Temperory unlock status indication
03/18/15   at      Perso reactivation support
12/01/14   hh      Support for get MCC and MNC
11/12/14   at      Support for RIL SAP
09/12/14   at      Change the Vcc property name to persist
08/20/14   at      Support for graceful UICC Voltage supply deactivation
06/18/14   at      Support for SelectNext using reselect QMI command
06/12/14   at      Support for auto device configuration
06/11/14   at      Support for open logical channel API
06/10/14   tl      Removed array structures for slot specific parameters
06/05/14   tl      Add support for recovery indication
05/14/14   yt      Support for STATUS command as part of SIM_IO request
04/18/14   tkl     Added support for RIL_REQUEST_SIM_AUTHENTICATION
01/21/14   at      Added support for getSelectResponse()
01/17/14   at      Changed the feature checks for RIL_REQUEST_SIM_GET_ATR
01/15/14   at      Added QMI UIM initialization retry mechanism
12/23/13   at      Support for Fusion 4.5 device configuration
12/10/13   at      Updated feature checks with new ones for APDU APIs
11/19/13   at      Changed the feature checks for streaming APDU APIs
12/11/13   at      Switch to new QCCI framework
09/11/13   yt      Initialize slot in encrypted PIN info structure
08/12/13   at      Added support for Long APDU indication
02/28/13   at      Support for SGLTE2 device configuration
02/07/13   at      Support for DSDA2 device configuration
01/29/13   yt      Support for third SIM card slot
01/14/13   yt      Fix critical KW errors
12/19/12   at      Move to newer qcril_event_queue API
11/15/12   at      Support for DSDA device configuration
10/31/12   at      Explicit query for card status during PIN responses
10/08/12   at      Support for ISIM Authentication API
07/18/12   at      Check for sglte modem type for QMI port
05/15/12   at      Support for card status validity TLV
04/09/12   at      Added support for RIL_REQUEST_SIM_GET_ATR
03/22/12   at      Replacing malloc()/free() with qcril_malloc()/qcril_free()
03/20/12   tl      Transition to QCCI
10/13/11   yt      Added PIN response cases to qcril_uim_copy_callback
09/20/11   at      Fix for adding 2 responses in qcril_uim_copy_callback
04/11/11   yt      Support for modem restart and silent PIN1 verification
03/30/11   at      Support for logical channel & send apdu commands
02/10/11   at      Find modem type by reading ro.baseband property
01/28/11   at      Refresh registration for Phonebook files by reading PBR
01/27/11   at      Calling qmi_uim_event_reg before qmi_uim_get_card_status
                   to eliminate any race condition in modem
01/18/11   at      Removed reference to RIL_MAX_CARDS
11/12/10   at      Added support for UIM queue implementation, removed seperate
                   IMSI callbacks
10/06/10   at      Support for handling instance_id passed in requests
08/30/10   at      Added handling for internal pin verification callback, removed
                   handling of QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE
08/25/10   at      Fixed proper calling function for read record rsp callback
08/19/10   at      Updated handling for refresh indication parameters
07/13/10   at      Added support for enable/disable FDN
07/02/10   at      Featurization for appropriate QMI ports
06/29/10   at      Added function call for QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS
05/13/10   at      Clean up for merging with mainline
04/25/10   at      Added handling for card status indications from QMI_UIM
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
#include "qcril_uim_card.h"
#include "qcril_uim_file.h"
#include "qcril_uim_security.h"
#include "qcril_uim_queue.h"
#include "qcril_uim_refresh.h"
#include "qcril_uim_util.h"
#include "qcril_uim_restart.h"
#include "qcril_uim_qcci.h"
#include "qmi_client_instance_defs.h"
#include "qmi_cci_target_ext.h"
#include "qcril_uim_sap.h"
#include <string.h>
#include <cutils/properties.h>

/* Android system property for fetching the modem type */
#define QCRIL_UIM_PROPERTY_BASEBAND               "ro.baseband"

/* Android property to enable supply voltage feature */
#define QCRIL_UIM_PROPERTY_FEATURE_VCC                 "persist.qcril_uim_vcc_feature"
#define QCRIL_UIM_PROP_FEATURE_VCC_ENABLED_VALUE       "1"

/* Android system property values for various modem types */
#define QCRIL_UIM_PROP_BASEBAND_VALUE_SVLTE_1     "svlte1"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_SVLTE_2A    "svlte2a"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_CSFB        "csfb"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_SGLTE       "sglte"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_SGLTE2      "sglte2"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_MSM         "msm"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_APQ         "apq"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_MDMUSB      "mdm"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_DSDA        "dsda"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_DSDA_2      "dsda2"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_FUSION_4_5  "mdm2"
#define QCRIL_UIM_PROP_BASEBAND_VALUE_AUTO        "auto"

/* QMI init retry related defines */
#define QCRIL_UIM_QMI_INIT_MAX_RETRIES            10
#define QCRIL_UIM_QMI_INIT_RETRY_INTERVAL          1

/* Global variable with QCRIL status */
qcril_uim_struct_type   qcril_uim;

/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_response

===========================================================================*/
/*!
    @brief
    Send a response to framework for a specific request.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_response
(
  qcril_instance_id_e_type instance_id,
  RIL_Token                token,
  RIL_Errno                result,
  void*                    rsp_ptr,
  size_t                   rsp_len,
  boolean                  remove_entry,
  char*                    logstr
)
{
  qcril_reqlist_public_type      info;
  qcril_request_resp_params_type resp;

  if (qcril_reqlist_query(instance_id, token, &info) == E_SUCCESS)
  {
    qcril_default_request_resp_params(instance_id,
                                      token,
                                      info.request,
                                      result,
                                      &resp);

    if (rsp_ptr != NULL)
    {
      resp.resp_pkt = rsp_ptr;
      resp.resp_len = rsp_len;
    }

    if (logstr != NULL)
    {
      resp.logstr = logstr;
    }

    qcril_send_request_response(&resp);
  }
} /* qcril_uim_response */


/*=========================================================================

  FUNCTION:  qcril_uim_copy_indication

===========================================================================*/
/*!
    @brief
    Makes a copy copy of the indication received from QMI.

    @return
    Pointer to copy of the indication
*/
/*=========================================================================*/
static qcril_uim_indication_params_type* qcril_uim_copy_indication
(
  qmi_uim_indication_id_type           ind_id,
  const qmi_uim_indication_data_type * ind_data_ptr,
  const void                         * user_data,
  int                                * total_size_ptr
)
{
  qcril_uim_indication_params_type* out_ptr  = NULL;

  /* Sanity checks */
  if ((ind_data_ptr == NULL) || (total_size_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return NULL;
  }

  /* Calculate buffer size */
  *total_size_ptr = sizeof(qcril_uim_indication_params_type);

  /* Refresh indication is currently the only one that requires a deep copy */
  if(ind_id == QMI_UIM_SRVC_REFRESH_IND_MSG)
  {
    *total_size_ptr += ind_data_ptr->refresh_ind.refresh_event.num_files * sizeof(qmi_uim_refresh_file_id_type);
  }
  else if (ind_id == QMI_UIM_SRVC_SEND_APDU_IND_MSG)
  {
    *total_size_ptr += ind_data_ptr->send_apdu_ind.apdu.data_len;
  }

  /* Alocate memory to copy the indication */
  out_ptr = qcril_malloc(*total_size_ptr);
  if (out_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "out_ptr alloc failed");
    *total_size_ptr = 0;
    return NULL;
  }

  memset(out_ptr, 0, *total_size_ptr);

  out_ptr->ind_id = ind_id;
  memcpy(&out_ptr->ind_data, ind_data_ptr, sizeof(qmi_uim_indication_data_type));
  out_ptr->user_data = (void*)user_data;

  /* Deep copy */
  if(ind_id == QMI_UIM_SRVC_REFRESH_IND_MSG)
  {
    out_ptr->ind_data.refresh_ind.refresh_event.files_ptr = (qmi_uim_refresh_file_id_type*)&out_ptr->payload;
    memcpy(out_ptr->ind_data.refresh_ind.refresh_event.files_ptr,
           ind_data_ptr->refresh_ind.refresh_event.files_ptr,
           ind_data_ptr->refresh_ind.refresh_event.num_files * sizeof(qmi_uim_refresh_file_id_type));
  }
  else if (ind_id == QMI_UIM_SRVC_SEND_APDU_IND_MSG)
  {
    out_ptr->ind_data.send_apdu_ind.apdu.data_ptr = (unsigned char *)&out_ptr->payload;
    memcpy(out_ptr->ind_data.send_apdu_ind.apdu.data_ptr,
           ind_data_ptr->send_apdu_ind.apdu.data_ptr,
           ind_data_ptr->send_apdu_ind.apdu.data_len);
  }

  /* out_ptr will be freed after processing the event in
     qcril_uim_process_qmi_indication */

  return out_ptr;
} /* qcril_uim_copy_indication */


/*=========================================================================

  FUNCTION:  qcril_uim_copy_callback

===========================================================================*/
/*!
    @brief
    Makes a copy copy of the callback received from QMI.

    @return
    Pointer to copy of the callback
*/
/*=========================================================================*/
static qcril_uim_callback_params_type* qcril_uim_copy_callback
(
  const qmi_uim_rsp_data_type        * rsp_data_ptr,
  const void                         * user_data,
  uint32                             * total_size_ptr
)
{
  qcril_uim_callback_params_type* out_ptr  = NULL;

  /* Sanity checks */
  if ((rsp_data_ptr == NULL) || (total_size_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return NULL;
  }

  *total_size_ptr = sizeof(qcril_uim_callback_params_type);

  /* Following messages are the only responses that require a deep copy and
     additional memory for it */
  switch(rsp_data_ptr->rsp_id)
  {
    case QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.read_transparent_rsp.content.data_len;
      break;

    case QMI_UIM_SRVC_READ_RECORD_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.read_record_rsp.content.data_len;
      break;

    case QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.get_file_attributes_rsp.raw_value.data_len;
      break;

    case QMI_UIM_SRVC_SEND_APDU_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.send_apdu_rsp.apdu_response.data_len;
      break;

    case QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.authenticate_rsp.auth_response.data_len;
      break;

    case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.set_pin_protection_rsp.encr_pin_data.data_len;
      break;

    case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.verify_pin_rsp.encr_pin_data.data_len;
      break;

    case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.unblock_pin_rsp.encr_pin_data.data_len;
      break;

    case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.change_pin_rsp.encr_pin_data.data_len;
      break;

    case QMI_UIM_SRVC_GET_ATR_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.get_atr_rsp.atr_response.data_len;
      break;

    case QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG:
    case QMI_UIM_SRVC_OPEN_LOGICAL_CHANNEL_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.logical_channel_rsp.select_response.data_len;
      break;

    case QMI_UIM_SRVC_SEND_STATUS_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.send_status_rsp.status_response.data_len;
      break;

    case QMI_UIM_SRVC_RESELECT_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.reselect_rsp.select_response.data_len;
      break;

    case QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG:
      *total_size_ptr += rsp_data_ptr->rsp_data.sap_response_rsp.sap_response.data_len;
      break;

    default:
      break;
  }

  out_ptr = (qcril_uim_callback_params_type*)qcril_malloc(*total_size_ptr);
  if (out_ptr == NULL)
  {
    return NULL;
  }

  memset(out_ptr, 0, *total_size_ptr);

  /* Copy the response parameters */
  memcpy(&out_ptr->qmi_rsp_data, rsp_data_ptr, sizeof(qmi_uim_rsp_data_type));

  /* Update original request data */
  out_ptr->orig_req_data = (qcril_uim_original_request_type*)user_data;

  /* Deep copy */
  switch(rsp_data_ptr->rsp_id)
  {
    case QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG:
      out_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr =
        (uint8*)&out_ptr->payload;
      memcpy(out_ptr->qmi_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr,
             rsp_data_ptr->rsp_data.read_transparent_rsp.content.data_ptr,
             rsp_data_ptr->rsp_data.read_transparent_rsp.content.data_len);
      break;

    case QMI_UIM_SRVC_READ_RECORD_RSP_MSG:
      out_ptr->qmi_rsp_data.rsp_data.read_record_rsp.content.data_ptr =
        (uint8*)&out_ptr->payload;
      memcpy(out_ptr->qmi_rsp_data.rsp_data.read_record_rsp.content.data_ptr,
             rsp_data_ptr->rsp_data.read_record_rsp.content.data_ptr,
             rsp_data_ptr->rsp_data.read_record_rsp.content.data_len);
      break;

    case QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG:
      out_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_ptr =
        (uint8*)&out_ptr->payload;
      memcpy(out_ptr->qmi_rsp_data.rsp_data.get_file_attributes_rsp.raw_value.data_ptr,
             rsp_data_ptr->rsp_data.get_file_attributes_rsp.raw_value.data_ptr,
             rsp_data_ptr->rsp_data.get_file_attributes_rsp.raw_value.data_len);
      break;

    case QMI_UIM_SRVC_SEND_APDU_RSP_MSG:
      out_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_ptr =
        (uint8*)&out_ptr->payload;
      memcpy(out_ptr->qmi_rsp_data.rsp_data.send_apdu_rsp.apdu_response.data_ptr,
             rsp_data_ptr->rsp_data.send_apdu_rsp.apdu_response.data_ptr,
             rsp_data_ptr->rsp_data.send_apdu_rsp.apdu_response.data_len);
      break;

    case QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG:
      out_ptr->qmi_rsp_data.rsp_data.authenticate_rsp.auth_response.data_ptr =
        (uint8*)&out_ptr->payload;
      memcpy(out_ptr->qmi_rsp_data.rsp_data.authenticate_rsp.auth_response.data_ptr,
             rsp_data_ptr->rsp_data.authenticate_rsp.auth_response.data_ptr,
             rsp_data_ptr->rsp_data.authenticate_rsp.auth_response.data_len);
      break;

    case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
      if(rsp_data_ptr->rsp_data.set_pin_protection_rsp.encr_pin_data.data_len > 0)
      {
        out_ptr->qmi_rsp_data.rsp_data.set_pin_protection_rsp.encr_pin_data.data_ptr =
          (uint8*)&out_ptr->payload;
        memcpy(out_ptr->qmi_rsp_data.rsp_data.set_pin_protection_rsp.encr_pin_data.data_ptr,
               rsp_data_ptr->rsp_data.set_pin_protection_rsp.encr_pin_data.data_ptr,
               rsp_data_ptr->rsp_data.set_pin_protection_rsp.encr_pin_data.data_len);
      }
      break;

    case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
      if(rsp_data_ptr->rsp_data.verify_pin_rsp.encr_pin_data.data_len > 0)
      {
        out_ptr->qmi_rsp_data.rsp_data.verify_pin_rsp.encr_pin_data.data_ptr =
          (uint8*)&out_ptr->payload;
        memcpy(out_ptr->qmi_rsp_data.rsp_data.verify_pin_rsp.encr_pin_data.data_ptr,
               rsp_data_ptr->rsp_data.verify_pin_rsp.encr_pin_data.data_ptr,
               rsp_data_ptr->rsp_data.verify_pin_rsp.encr_pin_data.data_len);
      }
      break;

    case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
      if(rsp_data_ptr->rsp_data.unblock_pin_rsp.encr_pin_data.data_len > 0)
      {
        out_ptr->qmi_rsp_data.rsp_data.unblock_pin_rsp.encr_pin_data.data_ptr =
          (uint8*)&out_ptr->payload;
        memcpy(out_ptr->qmi_rsp_data.rsp_data.unblock_pin_rsp.encr_pin_data.data_ptr,
               rsp_data_ptr->rsp_data.unblock_pin_rsp.encr_pin_data.data_ptr,
               rsp_data_ptr->rsp_data.unblock_pin_rsp.encr_pin_data.data_len);
      }
      break;

    case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
      if(rsp_data_ptr->rsp_data.change_pin_rsp.encr_pin_data.data_len > 0)
      {
        out_ptr->qmi_rsp_data.rsp_data.change_pin_rsp.encr_pin_data.data_ptr =
          (uint8*)&out_ptr->payload;
        memcpy(out_ptr->qmi_rsp_data.rsp_data.change_pin_rsp.encr_pin_data.data_ptr,
               rsp_data_ptr->rsp_data.change_pin_rsp.encr_pin_data.data_ptr,
               rsp_data_ptr->rsp_data.change_pin_rsp.encr_pin_data.data_len);
      }
      break;

    case QMI_UIM_SRVC_GET_ATR_RSP_MSG:
      out_ptr->qmi_rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr =
        (uint8*)&out_ptr->payload;
      memcpy(out_ptr->qmi_rsp_data.rsp_data.get_atr_rsp.atr_response.data_ptr,
             rsp_data_ptr->rsp_data.get_atr_rsp.atr_response.data_ptr,
             rsp_data_ptr->rsp_data.get_atr_rsp.atr_response.data_len);
      break;

    case QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG:
    case QMI_UIM_SRVC_OPEN_LOGICAL_CHANNEL_RSP_MSG:
      out_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr =
        (uint8*)&out_ptr->payload;
      memcpy(out_ptr->qmi_rsp_data.rsp_data.logical_channel_rsp.select_response.data_ptr,
             rsp_data_ptr->rsp_data.logical_channel_rsp.select_response.data_ptr,
             rsp_data_ptr->rsp_data.logical_channel_rsp.select_response.data_len);
      break;

    case QMI_UIM_SRVC_SEND_STATUS_RSP_MSG:
      if (rsp_data_ptr->rsp_data.send_status_rsp.status_response.data_len > 0)
      {
        out_ptr->qmi_rsp_data.rsp_data.send_status_rsp.status_response.data_ptr =
          (uint8*)&out_ptr->payload;
        memcpy(out_ptr->qmi_rsp_data.rsp_data.send_status_rsp.status_response.data_ptr,
               rsp_data_ptr->rsp_data.send_status_rsp.status_response.data_ptr,
               rsp_data_ptr->rsp_data.send_status_rsp.status_response.data_len);
      }
      break;

    case QMI_UIM_SRVC_RESELECT_RSP_MSG:
      if (rsp_data_ptr->rsp_data.reselect_rsp.select_response.data_len > 0)
      {
        out_ptr->qmi_rsp_data.rsp_data.reselect_rsp.select_response.data_ptr =
          (uint8*)&out_ptr->payload;
        memcpy(out_ptr->qmi_rsp_data.rsp_data.reselect_rsp.select_response.data_ptr,
               rsp_data_ptr->rsp_data.reselect_rsp.select_response.data_ptr,
               rsp_data_ptr->rsp_data.reselect_rsp.select_response.data_len);
      }
      break;

    case QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG:
      out_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr =
        (uint8*)&out_ptr->payload;
      memcpy(out_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr,
             rsp_data_ptr->rsp_data.sap_response_rsp.sap_response.data_ptr,
             rsp_data_ptr->rsp_data.sap_response_rsp.sap_response.data_len);
      break;

    default:
      break;
  }

  return out_ptr;
} /* qcril_uim_copy_callback */


/*=========================================================================

  FUNCTION:  qcril_uim_indication_cb

===========================================================================*/
/*!
    @brief
    Callback for QMI indications.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_indication_cb
(
  void                         * user_data,
  qmi_uim_indication_id_type     ind_id,
  qmi_uim_indication_data_type * ind_data_ptr
)
{
  qcril_uim_indication_params_type *ind_params_ptr      = NULL;
  int                               ind_params_tot_size = 0;
  qcril_instance_id_e_type          instance_id         = QCRIL_DEFAULT_INSTANCE_ID;
  IxErrnoType                       result              = E_FAILURE;

  QCRIL_LOG_INFO("%s: ind = 0x%x",
                 __FUNCTION__, ind_id);

#ifndef FEATURE_QCRIL_UIM_QMI_RPC_QCRIL
  if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDA) ||
      qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDA2))
  {
    instance_id = qmi_ril_get_process_instance_id();
  }
#endif /* FEATURE_QCRIL_UIM_QMI_RPC_QCRIL */

  switch(ind_id)
  {
    case QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG:
    case QMI_UIM_SRVC_REFRESH_IND_MSG:
    case QMI_UIM_SRVC_SEND_APDU_IND_MSG:
    case QMI_UIM_SRVC_RECOVERY_IND_MSG:
    case QMI_UIM_SRVC_SUPPLY_VOLTAGE_IND_MSG:
    case QMI_UIM_SRVC_SIMLOCK_TEMP_UNLOCK_IND_MSG:
      ind_params_ptr =
          qcril_uim_copy_indication(ind_id,
                                    ind_data_ptr,
                                    user_data,
                                    &ind_params_tot_size);
      break;

    default:
      QCRIL_LOG_ERROR("Invalid indication type: 0x%x", ind_id);
      break;
  }

  if (ind_params_ptr == NULL || ind_params_tot_size == 0)
  {
    QCRIL_LOG_ERROR("%s", "Error copying the indication");
    return;
  }

  QCRIL_LOG_INFO( "%s qcril_event_queue\n", __FUNCTION__);
  result = qcril_event_queue( instance_id,
                              QCRIL_DEFAULT_MODEM_ID,
                              QCRIL_DATA_NOT_ON_STACK,
                              QCRIL_EVT_UIM_QMI_INDICATION,
                              (void *)ind_params_ptr,
                              ind_params_tot_size,
                              NULL);
  if (result != E_SUCCESS)
  {
    QCRIL_LOG_ERROR( " qcril_event_queue failed, result: 0x%x\n", result);
    /* Free allocated memory in case event queueing fails */
    qcril_free(ind_params_ptr);
    ind_params_ptr = NULL;
  }

} /* qcril_uim_indication_cb */


/*=========================================================================

  FUNCTION:  qmi_uim_callback

===========================================================================*/
/*!
    @brief
    Callback for QMI commands.

    @return
    None
*/
/*=========================================================================*/
void qmi_uim_callback
(
  qmi_uim_rsp_data_type        * rsp_data_ptr,
  void                         * user_data
)
{
  uint32                           size       = 0;
  qcril_uim_callback_params_type * params_ptr = NULL;
  IxErrnoType                      result     = E_FAILURE;

  if (rsp_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL rsp_data_ptr, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  params_ptr = qcril_uim_copy_callback(rsp_data_ptr,
                                       user_data,
                                       &size);

  if ((params_ptr == NULL) || (params_ptr->orig_req_data == NULL))
  {
    QCRIL_LOG_ERROR("%s", "NULL params_ptr or orig_req_data, cannot proceed");
    if (params_ptr != NULL)
    {
      qcril_free(params_ptr);
    }
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "%s qcril_event_queue\n", __FUNCTION__);
  result = qcril_event_queue( params_ptr->orig_req_data->instance_id,
                              params_ptr->orig_req_data->modem_id,
                              QCRIL_DATA_NOT_ON_STACK,
                              QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK,
                              (void *)params_ptr,
                              size,
                              NULL);
  if (result != E_SUCCESS)
  {
    QCRIL_LOG_ERROR( " qcril_event_queue failed, result: 0x%x\n", result);
    /* Free allocated memory in case event queueing fails */
    qcril_free(params_ptr);
    params_ptr = NULL;
  }
} /* qmi_uim_callback */


/*=========================================================================

  FUNCTION:  qmi_uim_card_init_callback

===========================================================================*/
/*!
    @brief
    Special case callback for the intial card status QMI commands.

    @return
    None
*/
/*=========================================================================*/
void qmi_uim_card_init_callback
(
  qmi_uim_rsp_data_type        * rsp_data_ptr,
  void                         * user_data
)
{
  uint8                            i          = 0;
  uint32                           size       = 0;
  qmi_uim_indication_data_type     ind_data;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity check */
  if (rsp_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL rsp_data_ptr, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  memset(&ind_data, 0, sizeof(qmi_uim_indication_data_type));

  if(rsp_data_ptr->qmi_err_code >= 0)
  {
      QCRIL_LOG_QMI( QCRIL_DEFAULT_MODEM_ID, "qmi_uim_card_init_callback", "inside" );
      ind_data.status_change_ind.card_status = rsp_data_ptr->rsp_data.get_card_status_rsp.card_status;

      ind_data.status_change_ind.card_status_validity = rsp_data_ptr->rsp_data.get_card_status_rsp.card_status_validity;

      qcril_uim_indication_cb(user_data,
                              QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG,
                              &ind_data);
  }
} /* qmi_uim_card_init_callback */


/*=========================================================================

  FUNCTION:  qcril_uim_find_modem_port

===========================================================================*/
/*!
    @brief
    Maps the read Android property value to a QMI port for the modem that
    has the physical connection to the card. If there is no match, a default
    port is returned.

    @return
    Mapped port string value defined by QMI service.
*/
/*=========================================================================*/
static qmi_client_qmux_instance_type qcril_uim_find_modem_port
(
  char   * prop_value_ptr
)
{
  qmi_client_qmux_instance_type qmi_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;

  /* Sanity check */
  if (prop_value_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL prop_value_ptr, using default port");
    QCRIL_ASSERT(0);
    return qmi_modem_port;
  }

  QCRIL_LOG_INFO("Baseband property value read: %s\n", prop_value_ptr);

  /* Map the port based on the read property */
  if ((strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_SVLTE_1)  == 0) ||
      (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_SVLTE_2A) == 0) ||
      (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_CSFB)     == 0))
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_0;
  }
  else if ((strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_MDMUSB) == 0) ||
           (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_SGLTE2) == 0) ||
           (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_AUTO)   == 0))
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
  }
  else if ((strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_MSM)    == 0) ||
           (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_APQ)    == 0) ||
           (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_SGLTE)  == 0))
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
  }
  else if (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_DSDA) == 0)
  {
    /* If it is a DSDA configuration, ports are set based on RILD instance */
    if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
    {
      qmi_modem_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
    }
    else
    {
      qmi_modem_port = QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0;
    }
  }
  else if (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_DSDA_2) == 0)
  {
    /* If it is a DSDA2 configuration, ports are set based on RILD instance.
       Note that there is no support of RMNET2 over USB on mainline since that
       config is not supported. Need to revisit this config for future support */
    if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
    {
      qmi_modem_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
    }
    else
    {
      qmi_modem_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_1;
    }
  }
  else if (strcmp(prop_value_ptr, QCRIL_UIM_PROP_BASEBAND_VALUE_FUSION_4_5) == 0)
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0;
  }
  else
  {
    QCRIL_LOG_ERROR("%s", "Property value does not match, using default port");
  }

  QCRIL_LOG_INFO("QMI port found for modem: 0x%x\n", qmi_modem_port);

  return qmi_modem_port;
} /* qcril_uim_find_modem_port */


/*=========================================================================

  FUNCTION:  qcril_uim_reset_state

===========================================================================*/
/*!
    @brief
    Reset state of QCRIL_UIM at power up and whenever modem resets.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_reset_state
(
  void
)
{
  /* Clean up refresh info */
  qcril_uim_init_refresh_info();

  /* Clean up Long APDU & select response info, if any */
  qcril_uim_cleanup_long_apdu_info();
  qcril_uim_cleanup_select_response_info();

  /* Initialize global variables */
  memset(&qcril_uim, 0, sizeof(qcril_uim_struct_type));
  qcril_uim.prov_session_info.session_state_gw_indexes[0] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_gw_indexes[1] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_gw_indexes[2] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_1x_indexes[0] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_1x_indexes[1] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;
  qcril_uim.prov_session_info.session_state_1x_indexes[2] =
                                QCRIL_UIM_PROV_SESSION_NOT_ACTIVATED;

  qcril_uim_init_card_status(&qcril_uim.card_status);

  qcril_uim.pin1_info[0].slot = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  qcril_uim.pin1_info[1].slot = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
} /* qcril_uim_reset_state */


/*=========================================================================

  FUNCTION:  qcril_uim_init_state

===========================================================================*/
/*!
    @brief
    Initializes QMI_UIM service and state of QCRIL_UIM. Called during
    initial power up and whenever modem restarts.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_init_state
(
  void
)
{
  qcril_modem_id_e_type          modem_id        = QCRIL_MAX_MODEM_ID - 1;
  int                            qmi_err_code    = QMI_NO_ERR;
  qmi_uim_event_reg_params_type  event_reg_params;
  qmi_uim_rsp_data_type          rsp_data;
  char                           baseband_prop_value[PROPERTY_VALUE_MAX];
  char                           vcc_prop_value[PROPERTY_VALUE_MAX];
  char                           temp_unlock_status[PROPERTY_VALUE_MAX];
  qmi_client_qmux_instance_type  qmi_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
  uint8                          num_retries    = 0;
  char                           temp_buff[100] = {0};

  /* Initialize QMI service */
  QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "init" );

  /* Find out the modem type */
  memset(baseband_prop_value, 0x00, sizeof(baseband_prop_value));
  property_get(QCRIL_UIM_PROPERTY_BASEBAND, baseband_prop_value, "");

  memset(vcc_prop_value, 0x00, sizeof(vcc_prop_value));
  property_get(QCRIL_UIM_PROPERTY_FEATURE_VCC, vcc_prop_value, "");

  /* Get the saved temparory unlock status of the slot */
  (void)snprintf(temp_buff, 99, "%s_%d", QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS, qmi_ril_get_sim_slot());
  memset(temp_unlock_status, 0x00, sizeof(temp_unlock_status));
  property_get(temp_buff, temp_unlock_status, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_FALSE);

  /* Map to a respective QMI port */
  qmi_modem_port = qcril_uim_find_modem_port(baseband_prop_value);
  QCRIL_ASSERT(qmi_modem_port != NULL);

  do
  {
    if (num_retries > 0)
    {
      sleep(QCRIL_UIM_QMI_INIT_RETRY_INTERVAL);
    }

    QCRIL_LOG_INFO("Trying qcril_qmi_uim_srvc_init_client() try # %d", num_retries);
    qcril_uim.qmi_handle = qcril_qmi_uim_srvc_init_client(qmi_modem_port,
                                                          qcril_uim_indication_cb,
                                                          NULL,
                                                          &qmi_err_code);
    num_retries++;
  } while ((qcril_uim.qmi_handle == NULL) &&
           (qmi_err_code != QMI_NO_ERR)   &&
           (num_retries < QCRIL_UIM_QMI_INIT_MAX_RETRIES));

  if ((qcril_uim.qmi_handle == NULL) || (qmi_err_code != QMI_NO_ERR))
  {
    QCRIL_LOG_ERROR("Could not register successfully with QMI UIM Service. Error: %d\n",
                    qmi_err_code);
    return;
  }

  if (strcmp(temp_unlock_status, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_TRUE)  == 0)
  {
    qcril_uim.temp_unlock_status = TRUE;
  }

  /* Register for both legacy and extended card status events to accomodate
     modems supporting either event. */
  memset(&event_reg_params, 0, sizeof(qmi_uim_event_reg_params_type));
  event_reg_params.card_status                = QMI_UIM_TRUE;
  event_reg_params.extended_card_status       = QMI_UIM_TRUE;
  event_reg_params.simlock_temp_unlock_status = QMI_UIM_TRUE;
  if (strcmp(vcc_prop_value, QCRIL_UIM_PROP_FEATURE_VCC_ENABLED_VALUE)  == 0)
  {
    event_reg_params.supply_voltage_status = QMI_UIM_TRUE;
  }

#if defined (FEATURE_QCRIL_UIM_REMOTE_SERVER) || defined (FEATURE_QCRIL_UIM_SAP_SERVER_MODE)
  event_reg_params.sap_connection       = QMI_UIM_TRUE;
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER || FEATURE_QCRIL_UIM_SAP_SERVER_MODE */

  QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "event register" );
  qmi_err_code = qcril_qmi_uim_event_reg(qcril_uim.qmi_handle,
                                         &event_reg_params,
                                         &rsp_data);
  QCRIL_ASSERT(qmi_err_code >= 0);

  /* Get card status just in case modem is done initializing already */
  QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "get card status" );
  qmi_err_code = qcril_qmi_uim_get_card_status(qcril_uim.qmi_handle,
                                               qmi_uim_card_init_callback,
                                               NULL,
                                               NULL);
  QCRIL_ASSERT(qmi_err_code >= 0);
 
  /* Get modem temparory unlock status and send expiry indication if expired */
  qcril_uim_check_and_send_temp_unlock_expiry_ind();
} /* qcril_uim_init_state */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_init

===========================================================================*/
void qcril_uim_init
(
 void
)
{
  int                            qmi_err_code;
  char                           prop_value[PROPERTY_VALUE_MAX];
  qmi_client_qmux_instance_type  qmi_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Reset qcril_uim global structure*/
  qcril_uim_reset_state();

  /* Find out the modem type */
  memset(prop_value, 0x00, sizeof(prop_value));
  property_get(QCRIL_UIM_PROPERTY_BASEBAND, prop_value, "");

  /* Map to a respective QMI port */
  qmi_modem_port = qcril_uim_find_modem_port(prop_value);

  /* Initialize QMI connenction only once during power up */
#ifdef FEATURE_QCRIL_UIM_QMI_RPC_QCRIL
  if (qmi_connection_init(qmi_modem_port, &qmi_err_code) < 0)
  {
    QCRIL_LOG_ERROR("Could not connect with QMI Interface. Error: %d\n", qmi_err_code);
    qcril_uim.qmi_handle = NULL;
  }
#endif /* FEATURE_QCRIL_UIM_QMI_RPC_QCRIL */

  /* Get QMI handle and initialize card status */
  qcril_uim_init_state();
} /* qcril_uim_init */


/*===========================================================================

  FUNCTION:  qcril_uim_release

===========================================================================*/
void qcril_uim_release
(
 void
)
{
  int                   qmi_err_code = 0;
  qcril_modem_id_e_type modem_id     = QCRIL_MAX_MODEM_ID - 1;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Deinitialize QMI interface */
  if (qcril_uim.qmi_handle != NULL)
  {
    QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "release" );
    qcril_qmi_uim_srvc_release_client(qcril_uim.qmi_handle, &qmi_err_code);
  }

  /* Cleanup refresh data */
  qcril_uim_cleanup_refresh_info();

  /* Clean up Long APDU info & select response info, if any */
  qcril_uim_cleanup_long_apdu_info();
  qcril_uim_cleanup_select_response_info();
} /* qcril_uim_release */


/*=========================================================================

  FUNCTION:  qcril_uim_process_qmi_callback

===========================================================================*/
void qcril_uim_process_qmi_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_uim_callback_params_type  * callback_params_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  callback_params_ptr = (qcril_uim_callback_params_type*)params_ptr->data;
  if ((callback_params_ptr == NULL) || (callback_params_ptr->orig_req_data == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL callback_params_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  if ((callback_params_ptr->orig_req_data->instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (callback_params_ptr->orig_req_data->modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, modem_id: 0x%x",
                    callback_params_ptr->orig_req_data->instance_id,
                    callback_params_ptr->orig_req_data->modem_id);
    qcril_free(callback_params_ptr->orig_req_data);
    callback_params_ptr->orig_req_data = NULL;
    qcril_free(callback_params_ptr);
    callback_params_ptr = NULL;
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_process_qmi_callback: Response for token=%d \n",
                    qcril_log_get_token_id((RIL_Token)callback_params_ptr->orig_req_data->token) );

  /* Proceed to check pending requests in the queue, if any */
  qcril_uim_queue_complete_request();

  switch(callback_params_ptr->qmi_rsp_data.rsp_id)
  {
    case QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG:
      if (callback_params_ptr->orig_req_data->request_id == RIL_REQUEST_GET_IMSI)
      {
        qcril_uim_get_imsi_resp(callback_params_ptr);
      }
      else if (callback_params_ptr->orig_req_data->request_id == QCRIL_EVT_INTERNAL_UIM_GET_MCC_MNC)
      {
        qcril_uim_get_mcc_mnc_resp(callback_params_ptr);
      }
      else
      {
        qcril_uim_read_binary_resp(callback_params_ptr);
      }
      break;

    case QMI_UIM_SRVC_READ_RECORD_RSP_MSG:
      qcril_uim_read_record_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_WRITE_TRANSPARENT_RSP_MSG:
      qcril_uim_update_binary_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_WRITE_RECORD_RSP_MSG:
      qcril_uim_update_record_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG:
      qcril_uim_get_response_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_REFRESH_REGISTER_RSP_MSG:
      qcril_uim_refresh_register_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
      qcril_uim_pin_resp(callback_params_ptr, ret_ptr);
      break;

    case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
      qcril_uim_pin_resp(callback_params_ptr, ret_ptr);
      break;

    case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
      qcril_uim_pin_resp(callback_params_ptr, ret_ptr);
      break;

    case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
      qcril_uim_pin_resp(callback_params_ptr, ret_ptr);
      break;

    case QMI_UIM_SRVC_DEPERSONALIZATION_RSP_MSG:
      qcril_uim_deperso_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_GET_SERVICE_STATUS_RSP_MSG:
      qcril_uim_get_fdn_status_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_SET_SERVICE_STATUS_RSP_MSG:
      qcril_uim_set_fdn_status_resp(callback_params_ptr);
      break;

     case QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG:
      qcril_uim_sim_authenticate_resp(callback_params_ptr);
      break;

#if defined(RIL_REQUEST_SIM_OPEN_CHANNEL) || defined(RIL_REQUEST_SIM_CLOSE_CHANNEL)
    case QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG:
    case QMI_UIM_SRVC_OPEN_LOGICAL_CHANNEL_RSP_MSG:
      qcril_uim_logical_channel_resp(callback_params_ptr);
      break;
#endif /* RIL_REQUEST_SIM_OPEN_CHANNEL || RIL_REQUEST_SIM_CLOSE_CHANNEL */

#if defined(RIL_REQUEST_SIM_APDU) || defined(RIL_REQUEST_SIM_TRANSMIT_CHANNEL) || \
    defined(RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC) || defined(RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL)
    case QMI_UIM_SRVC_SEND_APDU_RSP_MSG:
      qcril_uim_send_apdu_resp(callback_params_ptr);
      break;
#endif /* RIL_REQUEST_SIM_APDU || RIL_REQUEST_SIM_TRANSMIT_CHANNEL ||
          RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC || RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL */

#if defined RIL_REQUEST_SIM_GET_ATR
    case QMI_UIM_SRVC_GET_ATR_RSP_MSG:
      qcril_uim_get_atr_resp(callback_params_ptr);
      break;
#endif /* RIL_REQUEST_SIM_GET_ATR */

    case QMI_UIM_SRVC_SEND_STATUS_RSP_MSG:
      qcril_uim_send_status_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_RESELECT_RSP_MSG:
      qcril_uim_reselect_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_SUPPLY_VOLTAGE_RSP_MSG:
      qcril_uim_voltage_supply_resp(callback_params_ptr);
      break;

#if defined (FEATURE_QCRIL_UIM_REMOTE_SERVER) || defined (FEATURE_QCRIL_UIM_SAP_SERVER_MODE)
    case QMI_UIM_SRVC_SAP_CONNECTION_RSP_MSG:
      qcril_uim_sap_qmi_handle_sap_connection_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG:
      qcril_uim_sap_qmi_handle_sap_request_resp(callback_params_ptr);
      break;
#endif /* FEATURE_QCRIL_UIM_REMOTE_SERVER || FEATURE_QCRIL_UIM_SAP_SERVER_MODE */

    case QMI_UIM_SRVC_PERSONALIZATION_RSP_MSG:
      qcril_uim_perso_resp(callback_params_ptr);
      break;

    case QMI_UIM_SRVC_POWER_DOWN_RSP_MSG:
    case QMI_UIM_SRVC_POWER_UP_RSP_MSG:
    case QMI_UIM_SRVC_GET_CARD_STATUS_RSP_MSG:
    case QMI_UIM_SRVC_EVENT_REG_RSP_MSG:
    default:
      break;
  }

  /* free cnf callback_params_ptr, allocated in qcril_uim_copy_callback() */
  qcril_free(callback_params_ptr);
  callback_params_ptr = NULL;

} /* qcril_uim_process_qmi_callback */


/*=========================================================================

  FUNCTION:  qcril_uim_process_qmi_indication

===========================================================================*/
void qcril_uim_process_qmi_indication
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_uim_indication_params_type * ind_param_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  if ((params_ptr->instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (params_ptr->modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, modem_id: 0x%x",
                     params_ptr->instance_id, params_ptr->modem_id);
    QCRIL_ASSERT(0);
    return;
  }

  ind_param_ptr = (qcril_uim_indication_params_type*) params_ptr->data;
  if (ind_param_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL ind_param_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  ind_param_ptr->instance_id = params_ptr->instance_id;
  ind_param_ptr->modem_id = params_ptr->modem_id;

  switch(ind_param_ptr->ind_id)
  {
    case QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG:
      qcril_uim_process_status_change_ind(ind_param_ptr, ret_ptr);
      break;

    case QMI_UIM_SRVC_REFRESH_IND_MSG:
      qcril_uim_process_refresh_ind(ind_param_ptr, ret_ptr);
      break;

#if defined(RIL_REQUEST_SIM_APDU) || defined(RIL_REQUEST_SIM_TRANSMIT_CHANNEL) || \
    defined(RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC) || defined(RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL)
    case QMI_UIM_SRVC_SEND_APDU_IND_MSG:
      qcril_uim_process_send_apdu_ind(ind_param_ptr, ret_ptr);
      break;
#endif /* RIL_REQUEST_SIM_APDU || RIL_REQUEST_SIM_TRANSMIT_CHANNEL ||
          RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC || RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL */

    case QMI_UIM_SRVC_RECOVERY_IND_MSG:
      qcril_uim_process_recovery_ind(ind_param_ptr, ret_ptr);
      break;

    case QMI_UIM_SRVC_SUPPLY_VOLTAGE_IND_MSG:
      qcril_uim_process_supply_voltage_ind(ind_param_ptr, ret_ptr);
      break;

    case QMI_UIM_SRVC_SIMLOCK_TEMP_UNLOCK_IND_MSG:
      qcril_uim_process_simlock_temp_unlock_ind(ind_param_ptr, ret_ptr);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported indication! 0x%x\n", ind_param_ptr->ind_id);
      break;
  }

  /* free ind_param_ptr allocated in qcril_uim_indication_cb() */
  qcril_free(ind_param_ptr);
  ind_param_ptr = NULL;

} /* qcril_uim_process_qmi_indication */


/*=========================================================================

  FUNCTION:  qcril_uim_process_internal_command

===========================================================================*/
void qcril_uim_process_internal_command
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type          instance_id;
  qcril_modem_id_e_type             modem_id;
  int                               *slot_ptr;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    return;
  }

  /* Retrieve instance and modem */
  instance_id = params_ptr->instance_id;
  modem_id = params_ptr->modem_id;
  if ((instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, modem_id: 0x%x",
                     instance_id, modem_id);
    return;
  }


  switch (params_ptr->event_id)
  {
    case QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN:
      if( params_ptr->data != NULL )
      {
        slot_ptr = (int *) params_ptr->data;
        if( *slot_ptr >= QMI_UIM_MAX_CARD_COUNT )
        {
          QCRIL_LOG_ERROR("invalid slot value 0x%x : cannot proceed", *slot_ptr);
          QCRIL_ASSERT(0);
          return;
        }
        QCRIL_LOG_DEBUG( "Card power down , slot %d\n", *slot_ptr );
        qcril_uim_process_power_down(instance_id, modem_id, *slot_ptr);
      }
      break;

    case QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP:
      if( params_ptr->data != NULL )
      {
        slot_ptr = (int *) params_ptr->data;
        if( *slot_ptr >= QMI_UIM_MAX_CARD_COUNT )
        {
          QCRIL_LOG_ERROR("invalid slot value 0x%x : cannot proceed", *slot_ptr);
          QCRIL_ASSERT(0);
          return;
        }
        QCRIL_LOG_DEBUG( "Card power up , slot %d\n", *slot_ptr );
        qcril_uim_process_power_up(instance_id, modem_id, *slot_ptr);
      }
      break;

    case QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS:
      qcril_uim_request_get_fdn_status(params_ptr, ret_ptr);
      break;

    case QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS:
      qcril_uim_request_set_fdn_status(params_ptr, ret_ptr);
      break;

    case QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS:
      qcril_uim_request_get_pin_status(params_ptr, ret_ptr);
      break;

    case QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS:
      qcril_uim_request_set_pin_status(params_ptr, ret_ptr);
      break;

    case QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK:
      qcril_uim_process_internal_verify_pin_command_callback(params_ptr, ret_ptr);
      break;

    case QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUBS:
    case QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUBS:
      if ( ril_to_uim_is_dsds_enabled() || ril_to_uim_is_tsts_enabled() )
      {
        qcril_uim_process_change_subscription(params_ptr, ret_ptr);
      }
      break;

    case QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START:
      qcril_uim_process_modem_restart_start(params_ptr, ret_ptr);
      break;

    case QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE:
      qcril_uim_process_modem_restart_complete(params_ptr, ret_ptr);
      break;

    case QCRIL_EVT_INTERNAL_UIM_GET_MCC_MNC:
      qcril_uim_request_get_mcc_mnc(params_ptr, ret_ptr);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported internal event! event_id: 0x%x\n",
                      params_ptr->event_id);
      break;
  }
}  /* qcril_uim_process_internal_command() */

#endif /* defined (FEATURE_QCRIL_UIM_QMI) */

