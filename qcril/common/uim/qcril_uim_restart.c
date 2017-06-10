
/*===========================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

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

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/15/15   vdc     Fix silent pin verification during MSSR
06/10/14   tl      Removed array structures for slot specific parameters
06/03/14   tl      Determine slot from instance during modem restart
04/28/14   at      Release QMI CAT client handle upon SSR start
04/08/14   yt      Fix determination of index for encrypted PIN data
01/09/14   yt      Perform silent PIN verification on SAP disconnect
12/11/13   at      Switch to new QCCI framework
09/10/13   yt      Clear encrypted PIN data after card reset
04/10/13   yt      Silent PIN verification support for multi SIM
03/15/13   yt      Report SIM_STATUS only after finishing silent PIN verify
09/27/12   yt      Reset modem_restarted flag if PIN1 verification is not needed
07/27/12   at      Re-initializing QCRIL_GSTK after a modem restart complete
03/22/12   at      Replacing malloc()/free() with qcril_malloc()/qcril_free()
03/20/12   tl      Transition to QCCI
10/26/11   yt      Wait to read EF-ICCID until QMI on modem is ready
06/27/11   yt      Correctly initialize length of EF-ICCID
04/11/11   yt      Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#if defined (FEATURE_QCRIL_UIM_QMI)

#include "qcrili.h"
#include "qcril_uim.h"
#include "qcril_log.h"
#include "qcril_uim_card.h"
#include "qcril_uim_util.h"
#include "qcril_uim_queue.h"
#include "qcril_uim_restart.h"
#include "qcril_uim_qcci.h"
#include "qcril_gstk_qmi.h"


/*===========================================================================

                           DEFINES

===========================================================================*/
/* Duration to wait (in microseconds) before attempting to read ICCID again */
#define QCRIL_UIM_WAIT_FOR_DEVICE_READY 200000

/* Maximum number of attempts to read EF-ICCID on modem restart */
#define QCRIL_UIM_ICCID_READ_ATTEMPTS   10


/*=========================================================================

  FUNCTION:  qcril_uim_verify_pin_on_restart

===========================================================================*/
/*!
    @brief
    Checks ICCID and AID before sending PIN1 for verification on modem
    restart.

    @return
    Status of PIN1 verification.
*/
/*=========================================================================*/
static RIL_Errno qcril_uim_verify_pin_on_restart
(
  uint8                                        slot,
  uint8                                        app_index,
  uint8                                        pin_index
)
{
  qmi_uim_rsp_data_type                     pin_rsp_data;
  qmi_uim_verify_pin_params_type            pin_params;
  int                                       res       = 0;
  uint16                                    slot_app  = 0;
  uint8                                     pin1_data[QMI_UIM_MAX_PIN_LEN];

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  memset(&pin_rsp_data, 0, sizeof(qmi_uim_rsp_data_type));
  memset(&pin_params, 0, sizeof(qmi_uim_verify_pin_params_type));
  memset(pin1_data, 0, sizeof(pin1_data));

  if(pin_index >= QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO ||
     qcril_uim.pin1_info[pin_index].slot != slot)
  {
    QCRIL_LOG_ERROR("Invalid index for PIN1 cache: 0x%x",pin_index);
    return RIL_E_GENERIC_FAILURE;
  }

  if(qcril_uim.pin1_info[pin_index].encrypted_pin1_len == 0)
  {
    QCRIL_LOG_ERROR("%s","Encrypted PIN1 not cached");
    return RIL_E_GENERIC_FAILURE;
  }

  slot_app = slot;
  slot_app = (slot_app << 8) | app_index;
  if(slot_app == qcril_uim.card_status.index_gw_pri_prov)
  {
    pin_params.session_info.session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  }
  else if(slot_app == qcril_uim.card_status.index_1x_pri_prov)
  {
    pin_params.session_info.session_type = QMI_UIM_SESSION_TYPE_PRI_1X_PROV;
  }
  else if(slot_app == qcril_uim.card_status.index_gw_sec_prov)
  {
    pin_params.session_info.session_type = QMI_UIM_SESSION_TYPE_SEC_GW_PROV;
  }
  else if(slot_app == qcril_uim.card_status.index_1x_sec_prov)
  {
    pin_params.session_info.session_type = QMI_UIM_SESSION_TYPE_SEC_1X_PROV;
  }
  else if(slot_app == qcril_uim.card_status.index_gw_ter_prov)
  {
    pin_params.session_info.session_type = QMI_UIM_SESSION_TYPE_TER_GW_PROV;
  }
  else if(slot_app == qcril_uim.card_status.index_1x_ter_prov)
  {
    pin_params.session_info.session_type = QMI_UIM_SESSION_TYPE_TER_1X_PROV;
  }

  pin_params.pin_data.data_ptr = pin1_data;

  /* Copy the cached PIN1 value */
  memcpy(pin_params.pin_data.data_ptr, qcril_uim.pin1_info[pin_index].encrypted_pin1,
         qcril_uim.pin1_info[pin_index].encrypted_pin1_len);

  pin_params.pin_data.data_len = qcril_uim.pin1_info[pin_index].encrypted_pin1_len;
  pin_params.is_pin1_encrypted = QMI_UIM_TRUE;
  pin_params.pin_id            = QMI_UIM_PIN_ID_PIN1;

  memset(&pin_rsp_data, 0, sizeof(qmi_uim_rsp_data_type));

  /* Send verify pin request */
  QCRIL_LOG_DEBUG("%s", "QMI Verify PIN" );
  res = qcril_qmi_uim_verify_pin(qcril_uim.qmi_handle,
                                 &pin_params,
                                 NULL,
                                 NULL,
                                 &pin_rsp_data);

  /* Client needs to free the memory for raw data */
  if(pin_rsp_data.rsp_data.verify_pin_rsp.encr_pin_data.data_ptr != NULL)
  {
    qcril_free(pin_rsp_data.rsp_data.verify_pin_rsp.encr_pin_data.data_ptr);
  }

  if (res < 0)
  {
    QCRIL_LOG_ERROR("Encrypted PIN1 verification failed for slot 0x%x and app 0x%x",
                    slot, app_index);
    /* Reset PIN1 cache. Failure of PIN verification means the cached info
       is invalid */
    memset(&qcril_uim.pin1_info[pin_index], 0x00, sizeof(qcril_uim_pin1_info_type));
    return RIL_E_GENERIC_FAILURE;
  }

  /* Set the silent_pin1_verify_in_progress flag to TRUE. This flag will be
     set to FALSE either after receiving indication with PIN1 enabled and
     verified or an error in PIN1 verification. */
  qcril_uim.pin1_info[pin_index].silent_verify_in_progress = TRUE;

  QCRIL_LOG_DEBUG("Encrypted PIN1 verified for slot 0x%x and app 0x%x",
                    slot, app_index);

  return RIL_E_SUCCESS;
} /* qcril_uim_verify_pin_on_restart */


/*===========================================================================

  FUNCTION:  qcril_uim_is_pin_enabled_not_verified

===========================================================================*/
/*!
    @brief
    Checks if PIN1 is enabled and not verified for the given application on
    the indicated slot.

    @return
    TRUE if enabled and not verified, else FALSE
*/
/*=========================================================================*/
static boolean qcril_uim_is_pin_enabled_not_verified
(
  uint8                  slot,
  uint8                  app_index
)
{
  if((app_index >= QMI_UIM_MAX_APP_PER_CARD_COUNT) ||
     (slot >= QMI_UIM_MAX_CARD_COUNT))
  {
    QCRIL_LOG_ERROR("Invalid slot or app_index; slot: %d, app_index: %d",
                    slot, app_index);
    return FALSE;
  }

  /* Check appropriate pin state */
  if(qcril_uim.card_status.card[slot].application[app_index].univ_pin ==
     QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
  {
    if (qcril_uim.card_status.card[slot].upin_state ==
        QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED)
    {
      QCRIL_LOG_DEBUG("UPIN is ENABLED_NOT_VERIFIED on slot %d", slot);
      return TRUE;
    }
  }
  else if(qcril_uim.card_status.card[slot].application[app_index].pin1_state ==
     QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED)
  {
    QCRIL_LOG_DEBUG("PIN1 is ENABLED_NOT_VERIFIED on slot %d", slot);
    return TRUE;
  }

  return FALSE;
} /* qcril_uim_is_pin_enabled_not_verified */


/*===========================================================================

  FUNCTION:  qcril_uim_is_pin_verification_needed

===========================================================================*/
/*!
    @brief
    Checks if PIN1 needs to be verified for any application on the given slot.

    @return
    TRUE if verification is needed, else FALSE
*/
/*=========================================================================*/
static boolean qcril_uim_is_pin_verification_needed
(
  uint8                  slot
)
{
  uint8 app_index = 0;

  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot: %d", slot);
    return FALSE;
  }

  for(app_index = 0; app_index < qcril_uim.card_status.card[slot].num_app; app_index++)
  {
    /* Check appropriate pin state */
    if(qcril_uim.card_status.card[slot].application[app_index].univ_pin ==
       QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
    {
      if (qcril_uim.card_status.card[slot].upin_state <=
          QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED)
      {
        QCRIL_LOG_DEBUG("UPIN verification needed for slot %d and app %d",
                        slot, app_index);
        return TRUE;
      }
    }
    else if(((qcril_uim.card_status.card[slot].application[app_index].app_type == QMI_UIM_APP_SIM) ||
             (qcril_uim.card_status.card[slot].application[app_index].app_type == QMI_UIM_APP_USIM) ||
             (qcril_uim.card_status.card[slot].application[app_index].app_type == QMI_UIM_APP_RUIM) ||
             (qcril_uim.card_status.card[slot].application[app_index].app_type == QMI_UIM_APP_CSIM) ) &&
           (qcril_uim.card_status.card[slot].application[app_index].pin1_state <=
             QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED))
    {
      QCRIL_LOG_DEBUG("PIN1 verification needed for slot %d and app %d",
                       slot, app_index);
      return TRUE;
    }
  }

  return FALSE;
} /* qcril_uim_is_pin_verification_needed */


/*===========================================================================

  FUNCTION:  qcril_uim_match_aid_in_encrypted_pin_cache

===========================================================================*/
/*!
    @brief
    Finds index of the encrypted PIN1 data in the cache that matches the
    specified AID.

    @return
    Index of the encrypted pin data if found. Size of the cache otherwise.
*/
/*=========================================================================*/
static uint8 qcril_uim_match_aid_in_encrypted_pin_cache
(
  uint8           aid_len,
  const char    * aid_ptr
)
{
  uint8 i = 0;

  for(i = 0; i < QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO; i++)
  {
    if(aid_len == qcril_uim.pin1_info[i].aid_len)
    {
      if((qcril_uim.pin1_info[i].aid_len > 0) &&
         (qcril_uim.pin1_info[i].aid_len <= QMI_UIM_MAX_AID_LEN))
      {
        if((aid_ptr != NULL) &&
           (memcmp(aid_ptr,
                   qcril_uim.pin1_info[i].aid_value,
                   qcril_uim.pin1_info[i].aid_len) == 0))
        {
          return i;
        }
      }
      else if(qcril_uim.pin1_info[i].aid_len == 0)
      {
        return i;
      }
    }
  }

  return QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO;
} /* qcril_uim_match_aid_in_encrypted_pin_cache */


/*===========================================================================

  FUNCTION:  qcril_uim_is_silent_pin_verification_needed

===========================================================================*/
/*!
    @brief
    Checks if pin needs to be verified silently for any application on the given slot.

    @return
    TRUE if silent pin verification is needed, else FALSE
*/
/*=========================================================================*/
boolean qcril_uim_is_silent_pin_verification_needed
(
  uint8                  slot
)
{
  uint8 app_index = 0;
  uint8 pin_index = QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot: %d", slot);
    return FALSE;
  }

  /* Check whether UPIN/PIN1 state is ENABLED_NOT_VERIFIED */
  for(app_index = 0; app_index < qcril_uim.card_status.card[slot].num_app; app_index++)
  {
    if(qcril_uim.card_status.card[slot].application[app_index].univ_pin ==
         QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
    {
      if (qcril_uim.card_status.card[slot].upin_state ==
            QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED)
      {
        QCRIL_LOG_INFO("Silent UPIN verification needed for slot %d and app %d",
                        slot, app_index);
        break;
      }
    }
    else if(((qcril_uim.card_status.card[slot].application[app_index].app_type == QMI_UIM_APP_SIM) ||
             (qcril_uim.card_status.card[slot].application[app_index].app_type == QMI_UIM_APP_USIM) ||
             (qcril_uim.card_status.card[slot].application[app_index].app_type == QMI_UIM_APP_RUIM) ||
             (qcril_uim.card_status.card[slot].application[app_index].app_type == QMI_UIM_APP_CSIM) ) &&
           (qcril_uim.card_status.card[slot].application[app_index].pin1_state ==
             QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED))
    {
      QCRIL_LOG_INFO("Silent PIN1 verification needed for slot %d and app %d",
                       slot, app_index);
      break;
    }
  }

  /* If app index is equal to number of apps then there is no such app with
     PIN1/UPIN state as ENABLED_NOT_VERIFIED */
  if(app_index == qcril_uim.card_status.card[slot].num_app)
  {
    return FALSE;
  }

  /* If the app details match then pin verification is in progress/yet to be done */
  pin_index = qcril_uim_match_aid_in_encrypted_pin_cache(
                qcril_uim.card_status.card[slot].application[app_index].aid_len,
                qcril_uim.card_status.card[slot].application[app_index].aid_value);

  if(pin_index < QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO)
  {
    return TRUE;
  }

  return FALSE;
} /* qcril_uim_is_silent_pin_verification_needed */


/*===========================================================================

  FUNCTION:  qcril_uim_get_empty_encrypted_pin_index

===========================================================================*/
/*!
    @brief
    Returns first available index in the encrypted PIN cache.

    @return
    Index of the encrypted pin data if found.
*/
/*=========================================================================*/
static uint8 qcril_uim_get_empty_encrypted_pin_index
(
  void
)
{
  uint8 i = 0;

  for(i = 0; i < QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO; i++)
  {
    if(qcril_uim.pin1_info[i].encrypted_pin1_len == 0)
    {
      return i;
    }
  }

  return QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO;
} /* qcril_uim_get_empty_encrypted_pin_index */


/*=========================================================================

  FUNCTION:  qcril_uim_store_encrypted_pin

===========================================================================*/
/*!
    @brief
    Stores encrypted PIN1 along with ICCID and application ID.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_store_encrypted_pin
(
  qmi_uim_rsp_data_type                        *rsp_data_ptr,
  qmi_uim_session_type                         session_type
)
{
  uint8                      slot         = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  uint8                      app_index    = QCRIL_UIM_INVALID_APP_INDEX_VALUE;
  uint8                      pin_index    = QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO;
  int                        i            = 0;
  qmi_uim_data_type        * pin_data_ptr = NULL;
  RIL_Errno                  ril_err      = RIL_E_GENERIC_FAILURE;
  uint8                      iccid_len    = QCRIL_UIM_ICCID_LEN;
  uint8                      iccid_data[QCRIL_UIM_ICCID_LEN];

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  memset(iccid_data, 0x00, sizeof(iccid_data));

  /* Determine index of the app for which PIN operation was performed */
  if(session_type == QMI_UIM_SESSION_TYPE_PRI_GW_PROV)
  {
    app_index = qcril_uim.card_status.index_gw_pri_prov & 0xFF;
    slot = (qcril_uim.card_status.index_gw_pri_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_PRI_1X_PROV)
  {
    app_index = qcril_uim.card_status.index_1x_pri_prov & 0xFF;
    slot = (qcril_uim.card_status.index_1x_pri_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_SEC_GW_PROV)
  {
    app_index = qcril_uim.card_status.index_gw_sec_prov & 0xFF;
    slot = (qcril_uim.card_status.index_gw_sec_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_SEC_1X_PROV)
  {
    app_index = qcril_uim.card_status.index_1x_sec_prov & 0xFF;
    slot = (qcril_uim.card_status.index_1x_sec_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_TER_GW_PROV)
  {
    app_index = qcril_uim.card_status.index_gw_ter_prov & 0xFF;
    slot = (qcril_uim.card_status.index_gw_ter_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_TER_1X_PROV)
  {
    app_index = qcril_uim.card_status.index_1x_ter_prov & 0xFF;
    slot = (qcril_uim.card_status.index_1x_ter_prov >> 8) & 0xFF;
  }
  else
  {
    QCRIL_LOG_ERROR("PIN1 caching not supported for session type: 0x%x",
                    session_type);
    return;
  }

  if((slot >= QMI_UIM_MAX_CARD_COUNT) ||
     (app_index >= QMI_UIM_MAX_APP_PER_CARD_COUNT))
  {
    QCRIL_LOG_ERROR("Invalid slot or app_index; slot: %d, app_index: %d",
                    slot, app_index);
    return;
  }

  /* Check if encrypted PIN1 was previously cached for this session type */
  pin_index = qcril_uim_match_aid_in_encrypted_pin_cache(
                qcril_uim.card_status.card[slot].application[app_index].aid_len,
                qcril_uim.card_status.card[slot].application[app_index].aid_value);

  /* If this is the first time PIN1 is being stored for this session type, find
     an empty element in encrypted PIN1 cache */
  if(pin_index >= QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO)
  {
    pin_index = qcril_uim_get_empty_encrypted_pin_index();

    if(pin_index >= QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO)
    {
      QCRIL_LOG_ERROR("%s", "Index not found in encrypted PIN1 cache");
      return;
    }
  }

  QCRIL_LOG_DEBUG("Found index in encrypted PIN1 cache: 0x%x", pin_index);
  memset(&qcril_uim.pin1_info[pin_index], 0x00,
         sizeof(qcril_uim.pin1_info[pin_index]));

  qcril_uim.pin1_info[pin_index].slot = slot;

  /* Store encrypted PIN1 value in global*/
  switch(rsp_data_ptr->rsp_id)
  {
    case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
      pin_data_ptr = &rsp_data_ptr->rsp_data.set_pin_protection_rsp.encr_pin_data;
      break;
    case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
      pin_data_ptr = &rsp_data_ptr->rsp_data.verify_pin_rsp.encr_pin_data;
      break;
    case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
      pin_data_ptr = &rsp_data_ptr->rsp_data.unblock_pin_rsp.encr_pin_data;
      break;
    case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
      pin_data_ptr = &rsp_data_ptr->rsp_data.change_pin_rsp.encr_pin_data;
      break;
    default:
      /* This case should never happen!! */
      qcril_uim.pin1_info[pin_index].encrypted_pin1_len = 0;
      memset(qcril_uim.pin1_info[pin_index].encrypted_pin1, 0x00, QMI_UIM_MAX_PIN_LEN);
      break;
  }

  if((pin_data_ptr != NULL) &&
     (pin_data_ptr->data_len > 0) &&
     (pin_data_ptr->data_len <= QMI_UIM_MAX_PIN_LEN))
  {
    QCRIL_LOG_DEBUG("Storing encrypted PIN1 for rsp_id 0x%x :",
                    rsp_data_ptr->rsp_id);
    qcril_uim.pin1_info[pin_index].encrypted_pin1_len = pin_data_ptr->data_len;
    memcpy(qcril_uim.pin1_info[pin_index].encrypted_pin1,
           pin_data_ptr->data_ptr,
           pin_data_ptr->data_len);

    for(i = 0; i < qcril_uim.pin1_info[pin_index].encrypted_pin1_len; i++)
    {
      QCRIL_LOG_VERBOSE("0x%x", qcril_uim.pin1_info[pin_index].encrypted_pin1[i] );
    }

    qcril_uim.pin1_info[pin_index].iccid_len = QCRIL_UIM_ICCID_LEN;
    ril_err = qcril_uim_read_iccid(slot,
                                   iccid_data,
                                   &iccid_len);

    if(ril_err != RIL_E_SUCCESS)
    {
      return;
    }
    /* Copy ICCID into PIN1 cache */
    memcpy(qcril_uim.pin1_info[pin_index].iccid_data,
           iccid_data,
           iccid_len);
    qcril_uim.pin1_info[pin_index].iccid_len = iccid_len;

    /* Copy AID into PIN1 cache */
    if(qcril_uim.card_status.card[slot].application[app_index].aid_len > 0 &&
       qcril_uim.card_status.card[slot].application[app_index].aid_len <= QMI_UIM_MAX_AID_LEN)
    {
      memcpy((void *)qcril_uim.pin1_info[pin_index].aid_value,
             (void *)qcril_uim.card_status.card[slot].application[app_index].aid_value,
             qcril_uim.card_status.card[slot].application[app_index].aid_len);
    }
    qcril_uim.pin1_info[pin_index].aid_len =
      qcril_uim.card_status.card[slot].application[app_index].aid_len;

    QCRIL_LOG_DEBUG("%s\n","AID stored after successful PIN operation:");
    for(i = 0; i < qcril_uim.pin1_info[pin_index].aid_len; i++)
    {
      QCRIL_LOG_VERBOSE("0x%x", qcril_uim.pin1_info[pin_index].aid_value[i]);
    }
  }
} /* qcril_uim_store_encrypted_pin */


/*=========================================================================

  FUNCTION:  qcril_uim_try_pin1_verification

===========================================================================*/
/*!
    @brief
    Checks ICCID, AID and PIN1 status before sending PIN1 for verification
    after modem restart.

    @return
    Status of PIN1 verification.
*/
/*=========================================================================*/
RIL_Errno qcril_uim_try_pin1_verification
(
  uint8                                        slot
)
{
  uint8                                  app_index     = 0;
  uint8                                  pin_index     = 0xFF;
  RIL_Errno                              verify_status = RIL_E_GENERIC_FAILURE;
  RIL_Errno                              read_status   = RIL_E_GENERIC_FAILURE;
  boolean                                iccid_match   = FALSE;
  uint8                                  iccid_len     = QCRIL_UIM_ICCID_LEN;
  uint8                                  iccid_data[QCRIL_UIM_ICCID_LEN];
  uint8                                  err_count     = 0;

  QCRIL_LOG_DEBUG("%s\n", __FUNCTION__);

  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot id: %d", slot);
    return RIL_E_GENERIC_FAILURE;
  }

  while(err_count < QCRIL_UIM_ICCID_READ_ATTEMPTS)
  {
    if(err_count > 0)
    {
      QCRIL_LOG_DEBUG("%s", "wait for 200ms then resend request to read iccid");
      usleep(QCRIL_UIM_WAIT_FOR_DEVICE_READY);
    }

    read_status = qcril_uim_read_iccid(slot,
                                       iccid_data,
                                       &iccid_len);
    if(read_status != RIL_E_SUCCESS && read_status != RIL_E_RADIO_NOT_AVAILABLE)
    {
      return RIL_E_GENERIC_FAILURE;
    }
    else if(read_status == RIL_E_SUCCESS)
    {
      break;
    }

    err_count++;
  }

  if(err_count == QCRIL_UIM_ICCID_READ_ATTEMPTS)
  {
    return RIL_E_GENERIC_FAILURE;
  }

  if((qcril_uim.pin1_info[0].iccid_len == iccid_len) &&
     (memcmp(qcril_uim.pin1_info[0].iccid_data,
             iccid_data,
             qcril_uim.pin1_info[0].iccid_len) == 0))
  {
    iccid_match = TRUE;
  }
  else if((qcril_uim.pin1_info[1].iccid_len == iccid_len) &&
          (memcmp(qcril_uim.pin1_info[1].iccid_data,
                  iccid_data,
                  qcril_uim.pin1_info[1].iccid_len) == 0))
  {
    iccid_match = TRUE;
  }

  if(!iccid_match)
  {
    QCRIL_LOG_ERROR( "%s: Stored ICCID did not match with card\n", __FUNCTION__);
    qcril_uim.silent_pin_verify_reqd = FALSE;
    return RIL_E_GENERIC_FAILURE;
  }

  for(app_index = 0; app_index < qcril_uim.card_status.card[slot].num_app; app_index++)
  {
    if(!qcril_uim_is_pin_enabled_not_verified(slot, app_index))
    {
      continue;
    }

     pin_index = qcril_uim_match_aid_in_encrypted_pin_cache(
                   qcril_uim.card_status.card[slot].application[app_index].aid_len,
                   qcril_uim.card_status.card[slot].application[app_index].aid_value);

    if(pin_index >= QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO)
    {
      continue;
    }

    QCRIL_LOG_DEBUG("PIN1 to be verified for index 0x%x in encrypted PIN1 cache",
                    pin_index);

    verify_status = qcril_uim_verify_pin_on_restart(slot, app_index, pin_index);

    /* Reset the flag for silent PIN1 verification to FALSE */
    qcril_uim.silent_pin_verify_reqd = FALSE;

    return verify_status;
  }

  /* Reset the flag for silent PIN1 verification to FALSE only if PIN1
     verification is not required */
  if(!qcril_uim_is_pin_verification_needed(slot))
  {
    qcril_uim.silent_pin_verify_reqd = FALSE;
    return RIL_E_GENERIC_FAILURE;
  }

  return RIL_E_SUCCESS;
} /* qcril_uim_try_pin1_verification */


/*=========================================================================

  FUNCTION:  qcril_uim_clear_encrypted_pin

===========================================================================*/
/*!
    @brief
    Clears cached value of encrypted PIN1.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_clear_encrypted_pin
(
  qmi_uim_session_type                         session_type
)
{
  uint8                      slot         = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  uint8                      app_index    = QCRIL_UIM_INVALID_APP_INDEX_VALUE;
  uint8                      pin_index    = 0xFF;

  /* Determine index of the app for which PIN operation was performed */
  if(session_type == QMI_UIM_SESSION_TYPE_PRI_GW_PROV)
  {
    app_index = qcril_uim.card_status.index_gw_pri_prov & 0xFF;
    slot = (qcril_uim.card_status.index_gw_pri_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_PRI_1X_PROV)
  {
    app_index = qcril_uim.card_status.index_1x_pri_prov & 0xFF;
    slot = (qcril_uim.card_status.index_1x_pri_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_SEC_GW_PROV)
  {
    app_index = qcril_uim.card_status.index_gw_sec_prov & 0xFF;
    slot = (qcril_uim.card_status.index_gw_sec_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_SEC_1X_PROV)
  {
    app_index = qcril_uim.card_status.index_1x_sec_prov & 0xFF;
    slot = (qcril_uim.card_status.index_1x_sec_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_TER_GW_PROV)
  {
    app_index = qcril_uim.card_status.index_gw_ter_prov & 0xFF;
    slot = (qcril_uim.card_status.index_gw_ter_prov >> 8) & 0xFF;
  }
  else if(session_type == QMI_UIM_SESSION_TYPE_TER_1X_PROV)
  {
    app_index = qcril_uim.card_status.index_1x_ter_prov & 0xFF;
    slot = (qcril_uim.card_status.index_1x_ter_prov >> 8) & 0xFF;
  }
  else
  {
    QCRIL_LOG_DEBUG("PIN1 not cached for session type: 0x%x",
                    session_type);
    return;
  }

  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_DEBUG("PIN1 not cached for slot 0x%x", slot);
    return;
  }

   pin_index = qcril_uim_match_aid_in_encrypted_pin_cache(
                qcril_uim.card_status.card[slot].application[app_index].aid_len,
                qcril_uim.card_status.card[slot].application[app_index].aid_value);

  if(pin_index >= QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO ||
     qcril_uim.pin1_info[pin_index].slot != slot)
  {
    QCRIL_LOG_DEBUG("%s\n","Correct index could not be found in PIN1 cache");
    return;
  }

  QCRIL_LOG_DEBUG("Clearing encypted PIN1 from cache for index: 0x%x", pin_index);
  memset(qcril_uim.pin1_info[pin_index].encrypted_pin1, 0x00, QMI_UIM_MAX_PIN_LEN);
  qcril_uim.pin1_info[pin_index].encrypted_pin1_len = 0;
}/* qcril_uim_clear_encrypted_pin */


/*=========================================================================

  FUNCTION:  qcril_uim_check_silent_pin_verify_in_progress

===========================================================================*/
/*!
    @brief
    Function to check if silent pin1 verification is in progress. Resets
    the flag if PIN1 state is ENABLED_VERIFIED.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_check_silent_pin_verify_in_progress
(
  uint8                                        slot
)
{
  uint8 pin_index = 0;
  uint8 app_index = 0;

  for (pin_index = 0; pin_index < QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO; pin_index++)
  {
    if(qcril_uim.pin1_info[pin_index].slot != slot ||
       qcril_uim.pin1_info[pin_index].silent_verify_in_progress == FALSE)
    {
      continue;
    }

    for (app_index = 0; app_index < qcril_uim.card_status.card[slot].num_app; app_index++)
    {
      if((qcril_uim.pin1_info[pin_index].aid_len ==
            qcril_uim.card_status.card[slot].application[app_index].aid_len) &&
           memcmp(qcril_uim.pin1_info[pin_index].aid_value,
                  qcril_uim.card_status.card[slot].application[app_index].aid_value,
                  qcril_uim.pin1_info[pin_index].aid_len) == 0 )
      {
        if (qcril_uim.card_status.card[slot].application[app_index].pin1_state ==
            QMI_UIM_PIN_STATE_ENABLED_VERIFIED)
        {
          qcril_uim.pin1_info[pin_index].silent_verify_in_progress = FALSE;
        }
      }
    }
  }
} /* qcril_uim_check_silent_verify_in_progress */


/*=========================================================================

  FUNCTION:  qcril_uim_clear_encrypted_pin_after_card_reset

===========================================================================*/
/*!
    @brief
    Clears encrypted PIN1 data after card reset.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_clear_encrypted_pin_after_card_reset
(
  uint8                                        slot
)
{
  uint8 pin_index = 0;

  for(pin_index = 0; pin_index < QCRIL_UIM_MAX_ENCRYPTED_PIN_INFO; pin_index++)
  {
    if(slot != qcril_uim.pin1_info[pin_index].slot)
    {
      continue;
    }
    QCRIL_LOG_INFO("clearing encrypted PIN1 data for index 0x%x", pin_index);
    memset(&qcril_uim.pin1_info[pin_index], 0x00, sizeof(qcril_uim_pin1_info_type));
    qcril_uim.pin1_info[pin_index].slot = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  }
} /* qcril_uim_clear_encrypted_pin_after_card_reset */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_process_modem_restart_start

===========================================================================*/
/*!
    @brief
    Resets state of QCRIL_UIM, sends card error events to QCRIL, and sends
    unsolicited response to Android framework for change in card status.
    Called as a result of QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_modem_restart_start
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_uim_pin1_info_type          pin1_info[2];
  qcril_unsol_resp_params_type      unsol_resp;
  int                               slot = 0;
  int                               qmi_err_code = 0;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Send generic error responses for pending commands in QCRIL UIM
     queue */
  qcril_uim_queue_cleanup();

  /* Copy data needed for PIN1 verification */
  memcpy(pin1_info, qcril_uim.pin1_info, sizeof(qcril_uim.pin1_info));

  /* Update card state */
  if(qcril_uim.card_status.index_gw_pri_prov != QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE)
  {
    ret_ptr->pri_gw_sim_state_changed = TRUE;
    ret_ptr->next_pri_gw_sim_state = QCRIL_SIM_STATE_ABSENT;
  }
  if(qcril_uim.card_status.index_1x_pri_prov != QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE)
  {
    ret_ptr->pri_cdma_sim_state_changed = TRUE;
    ret_ptr->next_pri_cdma_sim_state = QCRIL_SIM_STATE_ABSENT;
  }
  if(qcril_uim.card_status.index_gw_sec_prov != QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE)
  {
    ret_ptr->sec_gw_sim_state_changed = TRUE;
    ret_ptr->next_sec_gw_sim_state = QCRIL_SIM_STATE_ABSENT;
  }
  if(qcril_uim.card_status.index_1x_sec_prov != QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE)
  {
    ret_ptr->sec_cdma_sim_state_changed = TRUE;
    ret_ptr->next_sec_cdma_sim_state = QCRIL_SIM_STATE_ABSENT;
  }
  if(qcril_uim.card_status.index_gw_ter_prov != QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE)
  {
    ret_ptr->ter_gw_sim_state_changed = TRUE;
    ret_ptr->next_ter_gw_sim_state = QCRIL_SIM_STATE_ABSENT;
  }
  if(qcril_uim.card_status.index_1x_ter_prov != QCRIL_UIM_INVALID_SLOT_APP_INDEX_VALUE)
  {
    ret_ptr->ter_cdma_sim_state_changed = TRUE;
    ret_ptr->next_ter_cdma_sim_state = QCRIL_SIM_STATE_ABSENT;
  }
  ret_ptr->next_modem_state = QCRIL_MODEM_STATE_UNAVAILABLE;

  slot = qcril_uim_instance_id_to_slot(params_ptr->instance_id);

  if(slot < QMI_UIM_MAX_CARD_COUNT &&
     qcril_uim.card_status.card[slot].card_state == QMI_UIM_CARD_STATE_PRESENT)
  {
    /* Send internal QCRIL events */
    qcril_uim_update_pbm_card_event(params_ptr->instance_id,
                                    params_ptr->modem_id,
                                    slot,
                                    QCRIL_EVT_PBM_CARD_ERROR);

    qcril_uim_update_cm_card_status(params_ptr->instance_id,
                                    params_ptr->modem_id,
                                    slot,
                                    QCRIL_CARD_STATUS_DOWN);
  }

  /* Send unsolicited response to Framework */
  qcril_default_unsol_resp_params(params_ptr->instance_id,
                                  RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
                                  &unsol_resp);
  qcril_send_unsol_response(&unsol_resp);

  /* Also release QMI UIM/CAT client service handles */
  qcril_qmi_uim_srvc_release_client(qcril_uim.qmi_handle, &qmi_err_code);
  qcril_gstk_qmi_srvc_release_client();

  /* Reset global variables */
  qcril_uim_reset_state();

  /* Restore PIN1 data*/
  memcpy(qcril_uim.pin1_info, pin1_info, sizeof(qcril_uim.pin1_info));
}/* qcril_uim_process_modem_restart_start*/


/*=========================================================================

  FUNCTION:  qcril_uim_process_modem_restart_complete

===========================================================================*/
/*!
    @brief
    Initializes QCRIL_UIM and sets flag to indicate that the modem has
    restarted. Called as a result of
    QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_modem_restart_complete
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  uint8     i = 0;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  (void) params_ptr;
  (void) ret_ptr;

  /* Get QMI handle and update card status */
  qcril_uim_init_state();

  /* Re-init QCRIL_GSTK as well */
  qcril_gstk_qmi_init();

  qcril_uim.silent_pin_verify_reqd = TRUE;
}/* qcril_uim_process_modem_restart_complete*/

#endif /* defined (FEATURE_QCRIL_UIM_QMI) */
