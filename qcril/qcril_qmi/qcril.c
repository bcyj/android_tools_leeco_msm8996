/******************************************************************************
  @file    qcril.c
  @brief   qcril qmi core

  DESCRIPTION

----------------------------------------------------------------------------
  Copyright (c) 2008-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

---------------------------------------------------------------------------

******************************************************************************/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <signal.h>

#include <cutils/properties.h>
#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "qcril_arb.h"
#include "qcril_log.h"
#include "qcril_reqlist.h"
#include "qcril_other.h"
#include "qcrilhook_oem.h"
#include "qcril_qmi_client.h"
#include "qcril_db.h"

#include "qcril_pbm.h"
#include "qcril_qmi_nas.h"
#include "qcril_qmi_nas2.h"
#include "qcril_qmi_voice.h"
#include "qcril_qmi_imsa.h"
#include "qcril_qmi_pdc.h"

#include "qcril_qmi_ims_socket.h"
#include "qcril_qmi_ims_flow_control.h"
#include "qcril_qmi_ims_misc.h"

#include "qcril_qmi_oem_socket.h"

#include "qcril_qmi_imss.h"

#include "core_handler.h"
#include "cri_core.h"
#include "mdm_detect.h"
#include "qcril_am.h"
#include "qmi_ril_peripheral_mng.h"

#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
  #include "qcril_uim_remote_client_socket.h"
  #include "qcril_uim_remote.h"
#endif
#ifdef FEATURE_QCRIL_UIM_REMOTE_SERVER
  #include "qcril_uim_remote_server_socket.h"
#endif

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*! Hash table size, should be roughly equal to number of QCRIL events */
#define QCRIL_HT_ENTRIES_MAX 100

/* Bitmask represents the radio states
   Bit 5  - SIM or RUIM ready
   Bit 3  - Modem On
   Bit 2  - Modem Unavailable
   Bit 1  - Modem Off
*/
#define QCRIL_STATE_MASK_ALL_STATES                    0x07
#define QCRIL_STATE_MASK_ALL_ACTIVE_STATES             0x05
#define QCRIL_STATE_MASK_SIM_OR_RUIM_READY_STATES      0x10

#define QCRIL_REG_ALL_STATES( event_id, handler )          \
  NULL, (uint32) event_id, handler, QCRIL_STATE_MASK_ALL_STATES

#define QCRIL_REG_ALL_ACTIVE_STATES( event_id, handler ) \
  NULL, (uint32) event_id, handler, QCRIL_STATE_MASK_ALL_ACTIVE_STATES

#define QCRIL_REG_SIM_OR_RUIM_READY_STATES( event_id, handler ) \
  NULL, (uint32) event_id, handler, QCRIL_STATE_MASK_SIM_OR_RUIM_READY_STATES

static qcril_dispatch_table_entry_type qcril_event_table[] =
{
  /**********************************************
   *                SIM (MMGSDI)                *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/


  /* QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK, qcril_uim_process_qmi_callback ) },

  /* QCRIL_EVT_UIM_QMI_INDICATION */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_UIM_QMI_INDICATION, qcril_uim_process_qmi_indication ) },

  /* QCRIL_EVT_UIM_MCC_MNC_INFO */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_UIM_MCC_MNC_INFO, qcril_uim_process_mcc_mnc_info ) },

  /* QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUBS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUBS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUBS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUBS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_UIM_GET_MCC_MNC */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_UIM_GET_MCC_MNC, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_UIM_SAP_RESP */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_UIM_SAP_RESP, qcril_uim_sap_process_response ) },

  /* QCRIL_EVT_UIM_RMT_QMI_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_UIM_RMT_QMI_COMMAND_CALLBACK, qcril_uim_remote_process_qmi_callback ) },

  /* QCRIL_EVT_UIM_RMT_QMI_INDICATION_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_UIM_RMT_QMI_INDICATION_CALLBACK, qcril_uim_remote_process_qmi_indication ) },

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/


  /* 1 - RIL_REQUEST_GET_SIM_STATUS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_SIM_STATUS, qcril_uim_request_get_sim_status ) },

  /* 2 - RIL_REQUEST_ENTER_SIM_PIN */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PIN, qcril_uim_request_enter_pin ) },

  /* 3 - RIL_REQUEST_ENTER_SIM_PUK */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PUK, qcril_uim_request_enter_puk ) },

  /* 4 - RIL_REQUEST_ENTER_SIM_PIN2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PIN2, qcril_uim_request_enter_pin ) },

  /* 5 - RIL_REQUEST_ENTER_SIM_PUK2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PUK2, qcril_uim_request_enter_puk ) },

  /* 6 - RIL_REQUEST_CHANGE_SIM_PIN */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CHANGE_SIM_PIN, qcril_uim_request_change_pin ) },

  /* 7 - RIL_REQUEST_CHANGE_SIM_PIN2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CHANGE_SIM_PIN2, qcril_uim_request_change_pin ) },

  /* 8 - RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE, qcril_uim_request_enter_perso_key ) },

  /* 11 - RIL_REQUEST_GET_IMSI */
  { QCRIL_REG_SIM_OR_RUIM_READY_STATES( RIL_REQUEST_GET_IMSI, qcril_uim_request_get_imsi ) },

  /* 28 - RIL_REQUEST_SIM_IO */
  { QCRIL_REG_SIM_OR_RUIM_READY_STATES( RIL_REQUEST_SIM_IO, qcril_uim_request_sim_io ) },

  /* 105 - RIL_REQUEST_ISIM_AUTHENTICATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ISIM_AUTHENTICATION, qcril_uim_request_isim_authenticate ) },

#ifdef RIL_REQUEST_SIM_APDU
  /* 104 - RIL_REQUEST_SIM_APDU */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_APDU, qcril_uim_request_send_apdu ) },
#endif /* RIL_REQUEST_SIM_APDU */

#ifdef RIL_REQUEST_SIM_OPEN_CHANNEL
  /* 105 - RIL_REQUEST_SIM_OPEN_CHANNEL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_OPEN_CHANNEL, qcril_uim_request_logical_channel ) },
#endif /* RIL_REQUEST_SIM_OPEN_CHANNEL */

#ifdef RIL_REQUEST_SIM_CLOSE_CHANNEL
  /* 106 - RIL_REQUEST_SIM_CLOSE_CHANNEL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_CLOSE_CHANNEL, qcril_uim_request_logical_channel ) },
#endif /* RIL_REQUEST_SIM_CLOSE_CHANNEL */

#ifdef RIL_REQUEST_SIM_TRANSMIT_CHANNEL
  /* 107 - RIL_REQUEST_SIM_TRANSMIT_CHANNEL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_TRANSMIT_CHANNEL, qcril_uim_request_send_apdu ) },
#endif /* RIL_REQUEST_SIM_TRANSMIT_CHANNEL */

#ifdef RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC
  /* 114 - RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC, qcril_uim_request_send_apdu ) },
#endif /* RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC */

#ifdef RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL
  /* 117 - RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL, qcril_uim_request_send_apdu ) },
#endif /* RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL */

#if defined RIL_REQUEST_SIM_GET_ATR
  /* 125 - RIL_REQUEST_SIM_GET_ATR */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_GET_ATR, qcril_uim_request_get_atr ) },
#endif /* RIL_REQUEST_SIM_GET_ATR */

#ifdef RIL_REQUEST_SIM_AUTHENTICATION
  /* TBD - RIL_REQUEST_SIM_AUTHENTICATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_AUTHENTICATION, qcril_uim_request_sim_authenticate ) },
#endif /* RIL_REQUEST_SIM_AUTHENTICATION */

  /* QCRIL_EVT_HOOK_UICC_VOLTAGE_STATUS_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_UICC_VOLTAGE_STATUS_REQ, qcril_uim_request_voltage_status ) },

  /* QCRIL_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ, qcril_uim_request_perso_reactivate ) },

  /* QCRIL_EVT_HOOK_PERSONALIZATION_STATUS_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_PERSONALIZATION_STATUS_REQ, qcril_uim_request_perso_status ) },

  /**********************************************
   *                  SIM (GSTK)                *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/
  /* QCRIL_EVT_GSTK_QMI_CAT_INDICATION */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_CAT_INDICATION, qcril_gstk_qmi_process_qmi_indication ) },

  /* QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY, qcril_gstk_qmi_process_notify_ril_is_ready ) },

  /* QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK, qcril_gstk_qmi_process_raw_command_callback ) },

  /* QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK, qcril_gstk_qmi_process_qmi_response ) },

  /* QCRIL_EVT_GSTK_QMI_NOTIFY_CARD_ERROR */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_NOTIFY_CARD_ERROR, qcril_gstk_qmi_process_card_error ) },

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/
  /* 67 - RIL_REQUEST_STK_GET_PROFILE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_GET_PROFILE, qcril_gstk_qmi_request_stk_get_profile ) },

  /* 68 - RIL_REQUEST_STK_SET_PROFILE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SET_PROFILE, qcril_gstk_qmi_request_stk_set_profile ) },

  /* 69 - RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, qcril_gstk_qmi_request_stk_send_envelope_command ) },

  /* 70 - RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, qcril_gstk_qmi_request_stk_send_terminal_response ) },

  /* 71 - RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim ) },

  /* 103 - RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING, qcril_gstk_qmi_request_stk_service_is_running ) },

  /* 107 - RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS, qcril_gstk_qmi_request_stk_send_envelope_with_status ) },

  /**********************************************
   *                    SMS                     *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/

  /* QCRIL_EVT_SMS_COMMAND_CALLBACK */

  /* QCRIL_EVT_SMS_SUBSCRIPTION_PREF_INFO */

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 25 - RIL_REQUEST_SEND_SMS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SEND_SMS, qcril_sms_request_send_sms ) },

  /* 26 - RIL_REQUEST_SEND_SMS_EXPECT_MORE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SEND_SMS_EXPECT_MORE, qcril_sms_request_send_sms_expect_more ) },

  /* 37 - RIL_REQUEST_SMS_ACKNOWLEDGE  */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SMS_ACKNOWLEDGE , qcril_sms_request_sms_acknowledge ) },

  /* 63 - RIL_REQUEST_WRITE_SMS_TO_SIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_WRITE_SMS_TO_SIM, qcril_sms_request_write_sms_to_sim ) },

  /* 64 - RIL_REQUEST_DELETE_SMS_ON_SIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DELETE_SMS_ON_SIM, qcril_sms_request_delete_sms_on_sim ) },

  /* 100 - RIL_REQUEST_GET_SMSC_ADDRESS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_SMSC_ADDRESS, qcril_sms_request_get_smsc_address ) },

  /* 101 - RIL_REQUEST_SET_SMSC_ADDRESS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_SMSC_ADDRESS, qcril_sms_request_set_smsc_address ) },

  /* 102 - RIL_REQUEST_REPORT_SMS_MEMORY_STATUS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, qcril_sms_request_report_sms_memory_status ) },

  /* 89 - RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG, qcril_sms_request_gsm_get_broadcast_sms_config ) },

  /* 90 - RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, qcril_sms_request_gsm_set_broadcast_sms_config ) },

  /* 91 - RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, qcril_sms_request_gsm_sms_broadcast_activation ) },

  /* 87 - RIL_REQUEST_CDMA_SEND_SMS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SEND_SMS, qcril_sms_request_cdma_send_sms ) },

  /* 88 - RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, qcril_sms_request_cdma_sms_acknowledge ) },

  /* 96 - RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, qcril_sms_request_cdma_write_sms_to_ruim ) },

  /* 97 - RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, qcril_sms_request_cdma_delete_sms_on_ruim ) },

  /* 92 - RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, qcril_sms_request_cdma_get_broadcast_sms_config ) },

  /* 93 - RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, qcril_sms_request_cdma_set_broadcast_sms_config ) },

  /* 94 - RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, qcril_sms_request_cdma_sms_broadcast_activation ) },

  /* 108 - RIL_REQUEST_IMS_REGISTRATION_STATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_IMS_REGISTRATION_STATE, qcril_sms_request_ims_registration_state ) },

  /* 109 - RIL_REQUEST_IMS_SEND_SMS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_IMS_SEND_SMS, qcril_sms_request_ims_send_sms ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/
  /* QCRIL_EVT_SMS_RAW_READ_HANDLER */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_RAW_READ, qcril_sms_request_raw_read ) },

  /* QCRIL_EVT_QMI_SMS_PERFORM_INITIAL_CONFIGURATION_HANDLER */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_PERFORM_INITIAL_CONFIGURATION, qcril_sms_perform_initial_configuration_evt_handler ) },

  /* QCRIL_EVT_QMI_NAS_DSDS_SUBS_FOLLOWUP */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_NAS_DSDS_SUBS_FOLLOWUP, qcril_qmi_nas_event_subs_followup ) },

  /*QCRIL_EVT_QMI_NAS_DSDS_SUBS_DEACTIVATE_FOLLOWUP */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_NAS_DSDS_SUBS_DEACTIVATE_FOLLOWUP, qcril_qmi_nas_event_subs_deactivate_followup ) },

  /*QCRIL_EVT_QMI_NAS_CARD_STATUS_UPDATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_NAS_CARD_STATUS_UPDATE, qcril_qmi_nas_event_card_status_update ) },

  /* QCRIL_EVT_QMI_NAS_SIG_STRENGTH_UPDATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_NAS_SIG_STRENGTH_UPDATE, qcril_qmi_trigger_propagate_known_signal_strength_ind ) },

  /* QCRIL_EVT_QMI_NAS_CLEANUP_NW_SEL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_NAS_CLEANUP_NW_SEL, qcril_qmi_nas_nw_select_handle_total_cleanup ) },

  /* QCRIL_EVT_QMI_NAS_PASSOVER_NW_SEL_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_NAS_PASSOVER_NW_SEL_IND, qcril_qmi_nas_nw_select_dedicated_unsolicited_indicaton_event_thrd_handler ) },

  /* QCRIL_EVT_QMI_RIL_POST_VOICE_RTE_CHANGE_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_RIL_POST_VOICE_RTE_CHANGE_IND, qcril_qmi_nas_post_voice_rte_change_ind_handler ) },

  /* QCRIL_EVT_QMI_RIL_ASSESS_EMRGENCY_NUMBER_LIST_DESIGNATED_COUNTRY */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_RIL_ASSESS_EMRGENCY_NUMBER_LIST_DESIGNATED_COUNTRY, qcril_qmi_nas_assess_emergency_number_list_handler ) },

  // QCRIL_EVT_QMI_RIL_ENFORCE_DEFERRED_MODE_PREF_SET
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_RIL_ENFORCE_DEFERRED_MODE_PREF_SET, qmi_ril_nwreg_mode_pref_enforce_deferred_op_handler ) },

  // QCRIL_EVT_QMI_RIL_COMMON_IND_SUBSCRIBE_CONSIDER_ACTION
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_RIL_COMMON_IND_SUBSCRIBE_CONSIDER_ACTION, qmi_ril_nwreg_common_ind_subscribe_consider_action_handler ) },

  // QCRIL_EVT_QMI_RIL_POST_OPRT_ONLINE_ACTION
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_RIL_POST_OPRT_ONLINE_ACTION, qmi_ril_nwreg_post_oprt_online_action_handler ) },

  /**********************************************
   *                     DATA                   *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/
#ifndef QMI_RIL_UTF
  /* QCRIL_EVT_DATA_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES(  QCRIL_EVT_DATA_COMMAND_CALLBACK, qcril_data_command_hdlr ) },

  /* QCRIL_EVT_DATA_EVENT_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_DATA_EVENT_CALLBACK, qcril_data_event_hdlr ) },

#ifdef FEATURE_QCRIL_USE_NETCTRL
  /* QCRIL_EVT_DATA_WDS_EVENT_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_DATA_WDS_EVENT_CALLBACK, qcril_data_wds_event_hdlr ) },

  /* QCRIL_EVT_DATA_DSD_EVENT_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_DATA_DSD_EVENT_CALLBACK, qcril_data_dsd_event_hdlr ) },
#endif /*FEATURE_QCRIL_USE_NETCTRL*/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/
  /* 27 - RIL_REQUEST_SETUP_DATA_CALL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SETUP_DATA_CALL, qcril_data_request_setup_data_call ) },

  /* 41 - RIL_REQUEST_DEACTIVATE_DATA_CALL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DEACTIVATE_DATA_CALL, qcril_data_request_deactivate_data_call ) },

  /* The following event can come after switched to LPM. So honor it even Modem is in OFF state */
  /* 56 - RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, qcril_data_request_last_data_call_fail_cause ) },

  /* 57 - RIL_REQUEST_DATA_CALL_LIST */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DATA_CALL_LIST, qcril_data_request_data_call_list ) },

  /* 109 - RIL_REQUEST_GET_DATA_CALL_PROFILE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_DATA_CALL_PROFILE, qcril_data_request_omh_profile_info ) },


  /* 128 - RIL_REQUEST_SET_DATA_PROFILE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_DATA_PROFILE, qcril_data_request_set_data_profile ) },


  /* - RIL_REQUEST_DATA_GO_DORMANT*/
  { QCRIL_REG_ALL_ACTIVE_STATES(QCRIL_EVT_HOOK_DATA_GO_DORMANT, qcril_data_process_qcrilhook_go_dormant)},

#if (RIL_QCOM_VERSION >= 2)
  /* - RIL_REQUEST_SETUP_QOS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SETUP_QOS, qcril_data_request_setup_qos ) },

  /* - RIL_REQUEST_RELEASE_QOS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_RELEASE_QOS, qcril_data_request_release_qos ) },

  /* - RIL_REQUEST_GET_QOS_STATUS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_QOS_STATUS, qcril_data_request_get_qos_status ) },

  /* - RIL_REQUEST_MODIFY_QOS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_MODIFY_QOS, qcril_data_request_modify_qos ) },

  /* - RIL_REQUEST_SUSPEND_QOS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SUSPEND_QOS, qcril_data_request_suspend_qos ) },

  /* - RIL_REQUEST_RESUME_QOS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_RESUME_QOS, qcril_data_request_resume_qos ) },
#endif /* RIL_QCOM_VERSION >= 2 */

#endif
  /**********************************************
   *                    modem reset              *
   **********************************************/

  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_REQ, qmi_ril_core_pre_suspend_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_REQ, qmi_ril_core_final_suspend_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_CON, qmi_ril_suspending_con_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_CON, qmi_ril_suspending_con_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_CON, qmi_ril_suspending_con_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_CON, qmi_ril_suspending_con_handler ) },

  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_REQ, qmi_ril_core_pre_resume_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_REQ, qmi_ril_core_final_resume_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_CON, qmi_ril_resuming_con_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_CON, qmi_ril_resuming_con_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_CON, qmi_ril_resuming_con_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_CON, qmi_ril_resuming_con_handler ) },

  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_REQ, qmi_ril_stub_data_suspend_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_REQ, qmi_ril_stub_data_resume_handler ) },

  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_REQ, qmi_ril_route_uim_suspend_handler ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_REQ, qmi_ril_route_uim_resume_handler ) },

  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_MODEM_RESTART_CHECK_IF_SERVICE_UP,
                                                 qcril_qmi_check_if_service_is_up) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_SERVICE_DOWN, qcril_qmi_handle_event_service_down) },
  /* -------------------*
   *        EVENTS      *
   * -------------------*/
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_SEND_UNSOL_RADIO_STATE_CHANGED, qcril_qmi_nas_send_unsol_radio_state_changed ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_CONNECTED_EMEGENCY_CALL_END, qcril_qmi_nas_connected_emergency_call_end_hdlr ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_NAS_EMERGENCY_MODE_ON,   qcril_qmi_nas_emergency_mode_on_hdlr ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_NAS_EMERGENCY_MODE_OFF,  qcril_qmi_nas_emergency_mode_off_hdlr ) },

  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_PRL_VER_FETCH_ATTEMPT, qcril_qmi_nas_dms_handle_fetch_prl_request ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_CHECK_PRL_VER_CHANGE, qcril_qmi_nas_dms_handle_check_prl_ver_change ) },

  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_REQUEST_NW_SCAN, qcril_qmi_nas_perform_network_scan_command_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_REQUEST_NW_SELECT, qcril_qmi_nas_set_nw_selection_command_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_REQUEST_EMBMS_SET_ENABLE, qcril_qmi_nas_embms_set_enable_cmd_cb_helper ) },


  /* ---------------------------*
   *      RESPONSE HANDLERS     *
   * ---------------------------*/

  /* ---------------------------*
   *      INDICATION HANDLERS   *
   * ---------------------------*/

  /* QCRIL_EVT_QMI_VOICE_BURST_STOP_CONT_DTMF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_VOICE_BURST_STOP_CONT_DTMF, qcril_qmi_voice_request_dtmf_stop ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_VOICE_GET_WAITING_CALL, qcril_qmi_voice_waiting_call_handler ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_VOICE_EMERGENCY_CALL_PENDING, qcril_qmi_voice_emergency_call_pending_handler ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_VOICE_HANDLE_INDICATIONS, qcril_qmi_voice_unsol_ind_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_VOICE_HANDLE_COMM_CALLBACKS, qcril_qmi_voice_command_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_IMSA_HANDLE_INDICATIONS, qcril_qmi_imsa_unsol_ind_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_IMSA_HANDLE_COMM_CALLBACKS, qcril_qmi_imsa_command_cb_helper ) },

  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_IMSS_HANDLE_COMM_CALLBACKS, qcril_qmi_imss_command_cb_helper ) },

  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_NAS_HANDLE_INDICATIONS, qcril_qmi_nas_unsolicited_indication_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_NAS_HANDLE_ASYNC_CB, qcril_qmi_nas_async_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_DMS_HANDLE_INDICATIONS, qcril_qmi_dms_unsolicited_indication_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_PBM_HANDLE_INDICATIONS, qcril_qmi_pbm_unsolicited_indication_cb_helper ) },
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_QMI_SMS_HANDLE_INDICATIONS, qcril_qmi_sms_unsolicited_indication_cb_helper ) },

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/
  /* 9 - RIL_REQUEST_GET_CURRENT_CALLS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_CURRENT_CALLS, qcril_qmi_voice_request_get_current_atel_calls ) },

  /* 10 - RIL_REQUEST_DIAL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DIAL, qcril_qmi_voice_request_dial ) },

  /* 12 - RIL_REQUEST_HANGUP */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_HANGUP, qcril_qmi_voice_request_hangup ) },

  /* 18 - RIL_REQUEST_LAST_CALL_FAIL_CAUSE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_LAST_CALL_FAIL_CAUSE, qcril_qmi_voice_request_last_call_fail_cause ) },

  /* 40 - RIL_REQUEST_ANSWER */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ANSWER, qcril_qmi_voice_request_answer ) },

  /* 24 - RIL_REQUEST_DTMF */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DTMF, qcril_qmi_voice_request_dtmf ) },

  /* 49 - RIL_REQUEST_DTMF_START */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DTMF_START, qcril_qmi_voice_request_dtmf_start) },

  /* 50 - RIL_REQUEST_DTMF_STOP */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DTMF_STOP, qcril_qmi_voice_request_dtmf_stop) },

  /* 84 - RIL_REQUEST_CDMA_FLASH */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_FLASH, qcril_qmi_voice_request_cdma_flash) },

  /* 85 - RIL_REQUEST_CDMA_BURST_DTMF */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_BURST_DTMF,  qcril_qmi_voice_request_cdma_burst_dtmf ) },

  /* 124 - RIL_REQUEST_MODIFY_CALL_INITIATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_MODIFY_CALL_INITIATE,  qcril_qmi_voice_request_modify_call_initiate ) },

  /* 125 - RIL_REQUEST_MODIFY_CALL_CONFIRM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_MODIFY_CALL_CONFIRM,  qcril_qmi_voice_request_modify_call_confirm ) },

  /**********************************************
   *                    eMBMS                   *
   **********************************************/
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_ENABLE,  qcril_qmi_nas_embms_requst_enable ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_CON,  qcril_qmi_nas_embms_enable_data_con ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_DISABLE,  qcril_qmi_nas_embms_requst_disable ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_EMBMS_DISABLE_DATA_IND,  qcril_qmi_nas_embms_disable_data_ind ) },
#ifndef QMI_RIL_UTF
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ,  qcril_data_embms_enable_data_req ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI,  qcril_data_embms_activate_tmgi) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI,  qcril_data_embms_deactivate_tmgi) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI,  qcril_data_embms_activate_deactivate_tmgi) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_AVAILABLE_TMGI, qcril_data_embms_get_available_tmgi) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_TMGI, qcril_data_embms_get_active_tmgi) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_CONTENT_DESC_UPDATE, qcril_data_embms_content_desc_update) },
#endif
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_COVERAGE_STATE,  qcril_qmi_nas_embms_request_get_coverage_state) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_RSSI,  qcril_qmi_nas_embms_get_rssi ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_SET_SNTP_TIME,  qcril_qmi_nas_embms_set_sntp_time ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_SIB16_COVERAGE,  qcril_qmi_nas_embms_get_sib16_coverage ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_UTC_TIME,  qcril_qmi_nas_embms_get_utc_time ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_EMBMS_SEND_UNSOL_RADIO_STATE,  qcril_qmi_nas_embms_send_radio_state_helper ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_LOG_PACKET_IDS,  qcril_qmi_nas_embms_get_active_log_packet_ids ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_DELIVER_LOG_PACKET,  qcril_qmi_nas_embms_deliver_log_packet ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_E911_STATE,  qcril_qmi_nas_embms_get_e911_state ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_SIB_PLMN,    qcril_qmi_nas_embms_get_sib_plmn ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_STATUS,    qcril_qmi_nas_embms_get_embms_status ) },

  /**********************************************
   *                    QTuner (RFPE)           *
   **********************************************/
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_RFM_SCENARIO_REQ ,  qcril_qmi_nas_set_rfm_scenario_req) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_RFM_SCENARIO_REQ ,  qcril_qmi_nas_get_rfm_scenario_req) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_PROVISIONED_TABLE_REVISION_REQ ,  qcril_qmi_nas_get_provisioned_table_revision_req) },

  /**********************************************
   *           CM                               *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/


  /**********************************************
   *           CM - Phone Services              *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 23 - RIL_REQUEST_RADIO_POWER */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_RADIO_POWER, qcril_qmi_nas_request_power ) },

  /* 45 - RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, qcril_qmi_nas_query_network_selection_mode ) },

  /* 46 - RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, qcril_qmi_nas_set_network_selection_automatic ) },

  /* 47 - RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL, qcril_qmi_nas_set_network_selection_manual ) },

  /* 48 - RIL_REQUEST_QUERY_AVAILABLE_NETWORKS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_AVAILABLE_NETWORKS, qcril_qmi_nas_query_available_networks ) },

  /* 58 - RIL_REQUEST_RESET_RADIO - Deprecated per ril.h */

  /* 65 - RIL_REQUEST_SET_BAND_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_BAND_MODE, qcril_qmi_nas_set_band_mode ) },

  /* 66 - RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, qcril_qmi_nas_query_available_band_mode ) },

  /* 73 - RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, qcril_qmi_nas_request_set_preferred_network_type ) },

  /* 74 - RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, qcril_qmi_nas_request_get_preferred_network_type ) },

  /* 77 - RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, qcril_qmi_nas_set_subscription_source ) },

  /* 106 - RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE, qcril_qmi_nas_get_subscription_source ) },

  /* 78 - RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, qcril_qmi_nas_request_set_roaming_preference ) },

  /* 79 - RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE, qcril_qmi_nas_request_query_roaming_preference ) },

  /* 103 - RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, qcril_qmi_nas_exit_emergency_callback_mode ) },


  /* 109 - RIL_REQUEST_SET_UICC_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_UICC_SUBSCRIPTION, qcril_qmi_nas_set_uicc_subscription ) },

  /* 111 - RIL_REQUEST_GET_UICC_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_UICC_SUBSCRIPTION, qcril_qmi_nas_dsds_request_get_uicc_subscription ) },

  /* 113 - RIL_REQUEST_SET_SUBSCRIPTION_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_SUBSCRIPTION_MODE, qcril_qmi_nas_dsds_request_set_subscription_mode ) },

  /* QCRIL_EVT_HOOK_SET_TRANSMIT_POWER */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_TRANSMIT_POWER, qcril_qmi_nas2_set_max_transmit_power ) },

  /* 111 - RIL_REQUEST_SET_INITIAL_ATTACH_APN*/
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_INITIAL_ATTACH_APN, qcril_qmi_nas_set_initial_attach_apn ) },

  /* 123 - RIL_REQUEST_ALLOW_DATA */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ALLOW_DATA, qcril_qmi_nas_request_allow_data ) },

  /* 129 - RIL_REQUEST_SHUTDOWN */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SHUTDOWN, qcril_qmi_nas_request_shutdown) },
  /* -------------------*
   *        EVENTS      *
   * -------------------*/

  /* QCRIL_EVT_CM_UPDATE_FDN_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_UPDATE_FDN_STATUS, qcril_qmi_nas_dms_event_update_fdn_status ) },

  /* QCRIL_EVT_CM_CARD_STATUS_UPDATED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CARD_STATUS_UPDATED, qcril_qmi_nas_dms_event_card_status_updated ) },

  /* QCRIL_EVT_CM_CARD_APP_STATUS_CHANGED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CARD_APP_STATUS_CHANGED, qcril_qmi_nas_event_app_status_update) },

  /* QCRIL_EVT_CM_ACTIVATE_PROVISION_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_ACTIVATE_PROVISION_STATUS, qcril_qmi_nas_event_activate_provision_status ) },

  /* QCRIL_EVT_CM_DEACTIVATE_PROVISION_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_DEACTIVATE_PROVISION_STATUS, qcril_qmi_nas_event_deactivate_provision_status ) },

  /**********************************************
   *       CM - Serving System Management       *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 104 - RIL_REQUEST_VOICE_RADIO_TECH */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_VOICE_RADIO_TECH, qcril_qmi_nas_request_radio_tech ) },

  /* 109 - RIL_REQUEST_GET_DC_RT_INFO */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_DC_RT_INFO, qcril_qmi_nas_request_get_dc_rt_info ) },

  /* 19 - RIL_REQUEST_SIGNAL_STRENGTH */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIGNAL_STRENGTH, qcril_qmi_nas_request_signal_strength ) },

  /* 20 - RIL_REQUEST_REGISTRATION_STATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_REGISTRATION_STATE, qcril_qmi_nas_request_registration_state ) },

  /* 21 - RIL_REQUEST_DATA_REGISTRATION_STATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DATA_REGISTRATION_STATE, qcril_qmi_nas_request_data_registration_state ) },

  /* 22 - RIL_REQUEST_OPERATOR */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_OPERATOR,  qcril_qmi_nas_request_operator  ) },

  /* 61 - RIL_REQUEST_SCREEN_STATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SCREEN_STATE, qcril_qmi_nas_request_screen_state ) },

  /* 75 - RIL_REQUEST_GET_NEIGHBORING_CELL_IDS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, qcril_qmi_nas_get_neighboring_cell_ids ) },

  /* 76 - RIL_REQUEST_SET_LOCATION_UPDATES */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_LOCATION_UPDATES, qcril_qmi_nas_set_location_updates ) },

  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_CELL_INFO_LIST, qcril_qmi_nas_get_cell_info_list_ncl ) },

  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, qcril_qmi_nas_set_cell_info_list_rate ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/

  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_POLL_CELL_INFO_LIST, qcril_qmi_nas_poll_cell_info_list ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_CELL_INFO_LIST_CHANGED_IND, qcril_qmi_nas_cell_info_list_changed ) },

  /**********************************************
   *             CM - Call Services             *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/


  /* 80 - RIL_REQUEST_SET_TTY_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_TTY_MODE, qcril_qmi_voice_request_set_tty_mode ) },

  /* 81 - RIL_REQUEST_QUERY_TTY_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_TTY_MODE, qcril_voice_query_tty_mode ) },

  /* 82 - RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, qcril_qmi_voice_request_set_preferred_voice_privacy_mode ) },

  /* 83 - RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, qcril_qmi_voice_request_query_preferred_voice_privacy_mode ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/


  /**********************************************
   *           CM - In Band Services            *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/



  /* -------------------*
   *        EVENTS      *
   * -------------------*/


  /**********************************************
   *          CM - Supplemental Services        *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 13 - RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, qcril_qmi_voice_request_manage_calls_hangup_waiting_or_background ) },

  /* 14 - RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, qcril_qmi_voice_request_manage_calls_hangup_foreground_resume_background ) },

  /* 15 - RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, qcril_qmi_voice_request_manage_calls_switch_waiting_or_holding_and_active) },

  /* 16 - RIL_REQUEST_CONFERENCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CONFERENCE, qcril_qmi_voice_request_manage_calls_conference ) },

  /* 17 - RIL_REQUEST_UDUB */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_UDUB, qcril_qmi_voice_request_manage_calls_udub ) },

  /* 29 - RIL_REQUEST_SEND_USSD */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SEND_USSD, qcril_qmi_voice_supsvc_request_send_ussd ) },

  /* 30 - RIL_REQUEST_CANCEL_USSD */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CANCEL_USSD, qcril_qmi_voice_supsvc_request_cancel_ussd ) },

  /* 31 - RIL_REQUEST_GET_CLIR */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_CLIR, qcril_qmi_voice_request_get_clir ) },

  /* 32 - RIL_REQUEST_SET_CLIR */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_CLIR, qcril_qmi_voice_request_set_clir ) },

  /* 33 - RIL_REQUEST_QUERY_CALL_FORWARD_STATUS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, qcril_qmi_voice_request_query_call_forward_status ) },

  /* 34 - RIL_REQUEST_SET_CALL_FORWARD */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_CALL_FORWARD, qcril_qmi_voice_request_set_call_forward ) },

  /* 35 - RIL_REQUEST_QUERY_CALL_WAITING */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_CALL_WAITING, qcril_qmi_voice_request_query_call_waiting ) },

  /* 36 - RIL_REQUEST_SET_CALL_WAITING */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_CALL_WAITING, qcril_qmi_voice_request_set_call_waiting ) },

  /* 42 - RIL_REQUEST_QUERY_FACILITY_LOCK */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_FACILITY_LOCK, qcril_qmi_voice_request_query_facility_lock ) },

  /* 43 - RIL_REQUEST_SET_FACILITY_LOCK */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_FACILITY_LOCK, qcril_qmi_voice_request_set_facility_lock ) },

  /* 44 - RIL_REQUEST_CHANGE_BARRING_PASSWORD */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CHANGE_BARRING_PASSWORD, qcril_qmi_voice_request_change_barring_password ) },

  /* 52 - RIL_REQUEST_SEPARATE_CONNECTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SEPARATE_CONNECTION, qcril_qmi_voice_request_manage_calls_seperate_connection ) },

  /* 55 - RIL_REQUEST_QUERY_CLIP */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_CLIP, qcril_qmi_voice_request_query_clip ) },

  /* 62 - RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, qcril_qmi_voice_request_set_supp_svc_notification ) },

  /* 72 - RIL_REQUEST_EXPLICIT_CALL_TRANSFER */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_EXPLICIT_CALL_TRANSFER, qcril_qmi_voice_request_manage_calls_explicit_call_transfer ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/


  /**********************************************
   *          CM - Stats Services               *
   **********************************************/


   /**********************************************
   *                  Other                     *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 38 - RIL_REQUEST_GET_IMEI */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_IMEI, qcril_qmi_nas_dms_request_imei ) },

  /* 39 - RIL_REQUEST_GET_IMEISV */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_IMEISV, qcril_qmi_nas_dms_request_imeisv ) },

  /* 51 - RIL_REQUEST_BASEBAND_VERSION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_BASEBAND_VERSION, qcril_qmi_nas_dms_request_baseband_version ) },

  /* 53 - RIL_REQUEST_SET_MUTE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_MUTE, qcril_other_request_set_mute ) },

  /* 54 - RIL_REQUEST_GET_MUTE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_MUTE, qcril_other_request_get_mute ) },

  /* 59 - RIL_REQUEST_OEM_HOOK_RAW */
  /* RIL_REQUEST_OEM_HOOK_RAW is handled explicity in onRequest() and  appropriately dispatched */

  /* 60 - RIL_REQUEST_OEM_HOOK_STRINGS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_OEM_HOOK_STRINGS, qcril_other_request_oem_hook_strings ) },

  /* 0x80001 - QCRIL_EVT_HOOK_NV_READ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_NV_READ, qcril_other_request_oem_hook_nv_read ) },

  /* 0x80002 - QCRIL_EVT_HOOK_NV_WRITE */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_NV_WRITE, qcril_other_request_oem_hook_nv_write ) },

  /* 0x80005 - QCRIL_EVT_HOOK_SET_TUNE_AWAY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_TUNE_AWAY, qcril_qmi_nas_dsds_request_set_tune_away ) },

  /* 0x80006 - QCRIL_EVT_HOOK_GET_TUNE_AWAY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_TUNE_AWAY, qcril_qmi_nas_dsds_request_get_tune_away ) },

  /* 0x80007 - QCRIL_EVT_HOOK_SET_PAGING_PRIORITY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_PAGING_PRIORITY, qcril_qmi_nas_dsds_request_set_paging_priority ) },

  /* 0x80008 - QCRIL_EVT_HOOK_GET_PAGING_PRIORITY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_PAGING_PRIORITY, qcril_qmi_nas_dsds_request_get_paging_priority ) },

  /* 0x80009 - QCRIL_EVT_HOOK_GET_NEIGHBORING_CELLS_INFO */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_NEIGHBORING_CELLS_INFO, qcril_other_request_oem_hook_neighboring_cells_info ) },

  /* 0x8000b - QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC,
        qcril_qmi_nas_set_subscription_source_with_spc ) },
  /* 0x8000c - QCRIL_EVT_HOOK_SET_DEFAULT_VOICE_SUB */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_DEFAULT_VOICE_SUB, qcril_qmi_nas_dsds_request_set_default_voice_sub ) },

  /* 0x8000d - QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD, qcril_qmi_voice_request_set_local_call_hold ) },

  /* - QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION, qcril_qmi_nas_dsds_request_set_data_subscription ) },

  /* QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY, qcril_qmi_nas_dsds_request_set_lte_tune_away ) },

  /* 0x80050 - QCRIL_EVT_HOOK_CSG_PERFORM_NW_SCAN */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_CSG_PERFORM_NW_SCAN, qcril_qmi_nas_csg_handle_oem_hook_perform_network_scan ) },

  /* 0x80051 - QCRIL_EVT_HOOK_CSG_SET_SYS_SEL_PREF */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_CSG_SET_SYS_SEL_PREF, qcril_qmi_nas_csg_handle_oem_hook_set_sys_selection ) },

  /* 0x80052 - QCRIL_EVT_HOOK_CSG_GET_SYS_INFO */
  //  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_CSG_GET_SYS_INFO, qcril_qmi_nas_csg_handle_oem_hook_get_sys_info ) },

  /* 0x86001 - QCRIL_EVT_HOOK_VT_DIAL_CALL */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_VT_DIAL_CALL, qcril_qmi_ims_vt_dial_call ) },

  /* 0x86002 - QCRIL_EVT_HOOK_VT_END_CALL */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_VT_END_CALL, qcril_qmi_ims_vt_end_call ) },

  /* 0x86003 - QCRIL_EVT_HOOK_VT_ANSWER_CALL */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_VT_ANSWER_CALL, qcril_qmi_ims_vt_answer_call ) },

  /* 0x86004 - QCRIL_EVT_HOOK_VT_GET_CALL_INFO */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_VT_GET_CALL_INFO, qcril_qmi_ims_vt_get_call_info ) },

  /* 0x80051 - QCRIL_EVT_HOOK_IMS_ENABLER_STATE_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_ENABLER_STATE_REQ, qcril_qmi_ims_presence_enabler_state_req ) },

  /* 0x80052 - QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_REQ, qcril_qmi_ims_presence_send_publish_req ) },

  /* 0x80053 - QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_XML_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_XML_REQ, qcril_qmi_ims_presence_send_publish_xml_req ) },

  /* 0x80054 - QCRIL_EVT_HOOK_IMS_SEND_UNPUBLISH_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_SEND_UNPUBLISH_REQ, qcril_qmi_ims_presence_send_unpublish_req ) },

  /* 0x80055 - QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_REQ, qcril_qmi_ims_presence_send_subscribe_req ) },

  /* 0x80056 - QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_XML_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_XML_REQ, qcril_qmi_ims_presence_send_subscribe_xml_req ) },

  /* 0x80057 - QCRIL_EVT_HOOK_IMS_SEND_UNSUBSCRIBE_REQ */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_SEND_UNSUBSCRIBE_REQ, qcril_qmi_ims_presence_send_unsubscribe_req ) },

  /* 0x80058 - QCRIL_EVT_HOOK_IMS_SET_NOTIFY_FMT_REQ_V01  */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_SET_NOTIFY_FMT_REQ_V01 , qcril_qmi_ims_presence_set_notify_fmt_req ) },

  /* 0x80059 - QCRIL_EVT_HOOK_IMS_GET_NOTIFY_FMT_REQ_V01   */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_GET_NOTIFY_FMT_REQ_V01  , qcril_qmi_ims_presence_get_notify_fmt_req ) },

  /* 0x8005A - QCRIL_EVT_HOOK_IMS_SET_EVENT_REPORT_REQ_V01   */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_SET_EVENT_REPORT_REQ_V01  , qcril_qmi_ims_presence_set_event_report_req ) },

  /* 0x8005B - QCRIL_EVT_HOOK_IMS_GET_NOTIFY_FMT_REQ_V01   */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_IMS_GET_EVENT_REPORT_REQ_V01  , qcril_qmi_ims_presence_get_event_report_req ) },

  /* 76 - RIL_REQUEST_DEVICE_IDENTITY */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DEVICE_IDENTITY, qcril_qmi_nas_dms_request_device_identity ) },

  /* 86 - RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY, qcril_qmi_nas_cdma_validate_and_write_key ) },

  /* 99 - RIL_REQUEST_CDMA_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SUBSCRIPTION, qcril_qmi_nas_request_cdma_subscription ) },

  /* QCRIL_EVT_HOOK_CDMA_AVOID_CUR_NWK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_HOOK_CDMA_AVOID_CUR_NWK, qcril_qmi_nas_cdma_avoid_system ) },

  /* QCRIL_EVT_HOOK_CDMA_CLEAR_AVOIDANCE_LIST */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_HOOK_CDMA_CLEAR_AVOIDANCE_LIST, qcril_qmi_nas_cdma_clear_avoid_list ) },

  /* QCRIL_EVT_HOOK_CDMA_GET_AVOIDANCE_LIST */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_CDMA_GET_AVOIDANCE_LIST,  qcril_qmi_nas_get_cdma_avoid_system_list) },

  /* QCRIL_EVT_HOOK_GET_SAR_REV_KEY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_SAR_REV_KEY,  qcril_qmi_nas2_get_sar_rev_key) },

  /* QCRIL_EVT_HOOK_ENABLE_ENGINEER_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_ENABLE_ENGINEER_MODE, qcril_qmi_nas_enable_engineer_mode ) },

  /* QCRIL_EVT_HOOK_SET_BUILTIN_PLMN_LIST */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_BUILTIN_PLMN_LIST, qcril_qmi_nas_set_builtin_plmn_list) },

  /* QCRIL_EVT_HOOK_PERFORM_INCREMENTAL_NW_SCAN */
  { QCRIL_REG_ALL_ACTIVE_STATES(QCRIL_EVT_HOOK_PERFORM_INCREMENTAL_NW_SCAN, qcril_qmi_nas_perform_incremental_network_scan) },

  /* QCRIL_EVT_HOOK_REJECT_INCOMING_CALL_CAUSE_21 */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_REJECT_INCOMING_CALL_CAUSE_21, qcril_qmi_voice_oem_hook_reject_incoming_call_cause_21 ) },

  /* QCRIL_EVT_HOOK_SET_MODEM_TEST_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_MODEM_TEST_MODE, qcril_qmi_pdc_set_modem_test_mode) },

  /* QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE, qcril_qmi_pdc_query_modem_test_mode) },

  /* QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS, qcril_qmi_pdc_get_available_configs) },

 /* QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_ACQ_ORDER*/
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_ACQ_ORDER, qcril_qmi_nas_request_set_preferred_network_acq_order ) },

 /* QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_ACQ_ORDER*/
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_ACQ_ORDER, qcril_qmi_nas_request_get_preferred_network_acq_order ) },

  /* QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS, qcril_qmi_voice_get_current_setup_calls ) },

  /* QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER, qcril_qmi_voice_request_setup_answer ) },

  /* QCRIL_EVT_HOOK_CLEANUP_LOADED_CONFIGS */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_CLEANUP_LOADED_CONFIGS, qcril_qmi_pdc_cleanup_loaded_configs ) },

  /* QCRIL_EVT_HOOK_GET_MODEM_CAPABILITY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_MODEM_CAPABILITY, qcril_qmi_nas_get_modem_capability ) },

  /* QCRIL_EVT_HOOK_UPDATE_SUB_BINDING */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_UPDATE_SUB_BINDING, qcril_qmi_nas_update_sub_binding ) },

  /* QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_BAND_PREF*/
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_BAND_PREF, qcril_qmi_nas_request_set_preferred_network_band_pref) },

  /* QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_BAND_PREF*/
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_BAND_PREF, qcril_qmi_nas_request_get_preferred_network_band_pref) },

  /* QCRIL_EVT_HOOK_SEL_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SEL_CONFIG, qcril_qmi_pdc_select_configs ) },

  /* QCRIL_EVT_HOOK_GET_META_INFO */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_META_INFO, qcril_qmi_pdc_get_meta_info_of_config ) },

  /* QCRIL_EVT_HOOK_SET_IS_DATA_ENABLED */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_IS_DATA_ENABLED, qcril_qmi_nas_set_is_data_enabled ) },

  /* QCRIL_EVT_HOOK_SET_IS_DATA_ROAMING_ENABLED */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_IS_DATA_ROAMING_ENABLED, qcril_qmi_nas_set_is_data_roaming_enabled ) },

  /* QCRIL_EVT_HOOK_SET_APN_INFO */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_APN_INFO , qcril_qmi_nas_set_apn_info ) },

  /* QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS, qcril_qmi_pdc_deactivate_configs ) },

  /* QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE, qcril_qmi_pdc_get_qc_version_of_file ) },

  /* QCRIL_EVT_HOOK_VALIDATE_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_VALIDATE_CONFIG, qcril_qmi_pdc_validate_config ) },

  /* QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID, qcril_qmi_pdc_get_qc_version_of_configid ) },

  /* QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE, qcril_qmi_pdc_get_oem_version_of_file ) },

  /* QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID, qcril_qmi_pdc_get_oem_version_of_configid ) },

  /* QCRIL_EVT_HOOK_ACTIVATE_CONFIGS */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_ACTIVATE_CONFIGS, qcril_qmi_pdc_activate_configs ) },

  /* -------------------*
   *      CALLBACKS     *
   * -------------------*/

  /* -------------------*
   *        EVENTS      *
   * -------------------*/
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION, qcril_qmi_pdc_load_configuration) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION, qcril_qmi_pdc_select_configuration) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_PDC_DELETE_CONFIGURATION, qcril_qmi_pdc_delete_configuration) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION, qcril_qmi_pdc_list_configuration) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION, qcril_qmi_pdc_deactivate_configuration) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_QMI_RIL_PDC_PARSE_DIFF_RESULT, qcril_qmi_pdc_parse_diff_result) },

  /* -------------------*
   *     UNSOLICITED    *
   * -------------------*/

  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_IMS_REGISTRATION_STATE, qcril_qmi_imsa_request_ims_registration_state ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_DIAL, qcril_qmi_voice_request_dial ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_ANSWER, qcril_qmi_voice_request_answer ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_HANGUP, qcril_qmi_voice_request_hangup ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_LAST_CALL_FAIL_CAUSE, qcril_qmi_voice_request_last_call_fail_cause ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_GET_CURRENT_CALLS, qcril_qmi_voice_send_current_ims_calls ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_WAITING_OR_BACKGROUND, qcril_qmi_voice_request_manage_calls_hangup_waiting_or_background ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND, qcril_qmi_voice_request_manage_calls_hangup_foreground_resume_background ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, qcril_qmi_voice_request_manage_calls_switch_waiting_or_holding_and_active ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE, qcril_qmi_voice_request_manage_calls_conference ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_EXIT_ECBM, qcril_qmi_nas_exit_emergency_callback_mode ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_DTMF, qcril_qmi_voice_request_dtmf ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START, qcril_qmi_voice_request_dtmf_start ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP, qcril_qmi_voice_request_dtmf_stop ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE, qcril_qmi_voice_request_modify_call_initiate ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM, qcril_qmi_voice_request_modify_call_confirm ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CLIP, qcril_qmi_voice_request_query_clip ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_GET_CLIR, qcril_qmi_voice_request_get_clir ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_CLIR, qcril_qmi_voice_request_set_clir ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_FORWARD_STATUS, qcril_qmi_voice_request_query_call_forward_status ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS, qcril_qmi_voice_request_set_call_forward ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_WAITING, qcril_qmi_voice_request_query_call_waiting ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_WAITING, qcril_qmi_voice_request_set_call_waiting ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_IMS_REG_STATE_CHANGE, qcril_qmi_imss_request_set_ims_registration ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_SUPP_SVC_NOTIFICATION, qcril_qmi_voice_request_set_supp_svc_notification ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_ADD_PARTICIPANT, qcril_qmi_voice_request_add_participant ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_QUERY_SERVICE_STATUS, qcril_qmi_imsa_request_query_ims_srv_status ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_SERVICE_STATUS, qcril_qmi_imss_request_set_ims_srv_status ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_STOP_CONT_DTMF, qcril_qmi_voice_request_dtmf_stop ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS, qcril_qmi_voice_request_ims_set_supp_srv_status ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_CALL_DEFLECTION, qcril_qmi_voice_request_call_deflection ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_QUERY_VT_CALL_QUALITY, qcril_qmi_imss_request_query_vt_call_quality ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_VT_CALL_QUALITY, qcril_qmi_imss_request_set_vt_call_quality ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR, qcril_qmi_voice_request_get_colr ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_COLR, qcril_qmi_voice_request_set_colr ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_HOLD, qcril_qmi_voice_request_manage_calls_hold_resume ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_RESUME, qcril_qmi_voice_request_manage_calls_hold_resume ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_TTY_MODE, qcril_qmi_voice_request_set_tty_mode ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_STATISTICS, qcril_qmi_imsa_request_get_rtp_statistics ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_ERROR_STATISTICS, qcril_qmi_imsa_request_get_rtp_statistics ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_GET_WIFI_CALLING_STATUS, qcril_qmi_imss_request_get_wifi_calling_status ) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_IMS_SOCKET_REQ_SET_WIFI_CALLING_STATUS, qcril_qmi_imss_request_set_wifi_calling_status ) },

#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_EVENT, qcril_uim_remote_client_request_event) },
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_APDU, qcril_uim_remote_client_request_apdu) },
#endif
#ifdef FEATURE_QCRIL_UIM_REMOTE_SERVER
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_UIM_REMOTE_SERVER_SOCKET_REQ_DISPATCH, qcril_uim_remote_server_socket_dispatch_request ) },
#endif
};

static int qmi_ril_fw_dedicated_thrd_exec_android_requests_set[] =
  {
    RIL_REQUEST_VOICE_RADIO_TECH,
    RIL_REQUEST_SIGNAL_STRENGTH,
    RIL_REQUEST_REGISTRATION_STATE,
    RIL_REQUEST_DATA_REGISTRATION_STATE,
    RIL_REQUEST_IMS_REGISTRATION_STATE,
    RIL_REQUEST_OPERATOR,
    RIL_REQUEST_SET_LOCATION_UPDATES,
    RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE,
    RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC,
    RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL,
    RIL_REQUEST_SET_BAND_MODE,
    RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE,
    RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE,
    RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE,
    RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE,
    RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE,
    RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE,
    RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE,
    RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE,
    RIL_REQUEST_SET_UICC_SUBSCRIPTION,
    RIL_REQUEST_GET_UICC_SUBSCRIPTION,
    RIL_REQUEST_SET_SUBSCRIPTION_MODE,
    RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE,
    RIL_REQUEST_SET_TTY_MODE,
    RIL_REQUEST_QUERY_TTY_MODE,
    RIL_REQUEST_CDMA_SUBSCRIPTION,
    RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY,
    RIL_REQUEST_DEVICE_IDENTITY,
    RIL_REQUEST_BASEBAND_VERSION,
    RIL_REQUEST_GET_IMEI,
    RIL_REQUEST_GET_IMEISV,
    RIL_REQUEST_RADIO_POWER,
    RIL_REQUEST_SET_INITIAL_ATTACH_APN,
    RIL_REQUEST_WRITE_SMS_TO_SIM,
    RIL_REQUEST_GET_CELL_INFO_LIST,

    QMI_RIL_ZERO
  };

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* QCRIL internal info */
static qcril_arb_state_struct_type *qcril_state;

/* QCRIL timer id */
pthread_mutex_t qcril_timer_id_mutex; /*!< Mutex to control access/update of QCRIL Timer ID*/
static uint16 qcril_timer_id;         /*!< Next QCRIL Timer ID */

/* QCRIL timer list */
pthread_mutex_t qcril_timed_callback_list_mutex;
static qcril_timed_callback_info *qcril_timed_callback_list;

/* QCRIL Heap Memory list*/
pthread_mutex_t qcril_heap_memory_list_mutex;
qcril_heap_list_info *qcril_heap_memory_list;
uint32 heap_memory_id;
boolean is_heap_memory_tracked;

/*! Hash table for dispatching inputs to QCRIL handlers */
static qcril_dispatch_table_entry_type *qcril_hash_table[ QCRIL_HT_ENTRIES_MAX ];

/* Time (1 second) to wait for the completion of modem restart before re-initiate QCRIL */
static const struct timeval TIMEVAL_DELAY = {1,0};
static const struct timeval HEAP_MEM_LIST_PRINT_TIMEVAL_DELAY = {60,0};

#define QMI_RIL_SYS_PROP_SGLTE_CSFB_LENGTH       4
#define QMI_RIL_SYS_PROP_RAT_OPTION             "persist.radio.rat_on"
#define QMI_RIL_SYS_PROP_NAME_IMS_RETRY_ON_3GPP "persist.radio.ims_retry_3gpp"
#define QMI_RIL_SYS_PROP_NAME_IMS_RETRY_ON_3GPP2 "persist.radio.ims_retry_3gpp2"
#define QMI_RIL_SYS_PROP_NAME_OEM_SOCKET         "persist.radio.oem_socket"
#define QMI_RIL_SYS_PROP_NAME_OEM_IND_TO_BOTH    "persist.radio.oem_ind_to_both"
#define QMI_RIL_SYS_PROP_NAME_SUPPRESS_REQ       "persist.radio.suppress_req"

#define QCRIL_REQUEST_SUPPRESS_MAX_LEN 4
static int qcril_request_suppress_list[QCRIL_REQUEST_SUPPRESS_MAX_LEN];

/* QCRIL request supress list mutex. */
pthread_mutex_t      qcril_request_supress_list_mutex;
pthread_mutexattr_t  qcril_request_supress_list_mutex_attr;

typedef enum
{
  QMI_RIL_FTR_MULTI_SIM_UNKNOWN = 0,
  QMI_RIL_FTR_MULTI_SIM_DISABLED,
  QMI_RIL_FTR_DSDS_ENABLED,
  QMI_RIL_FTR_TSTS_ENABLED,
  QMI_RIL_FTR_DSDA_ENABLED
} qcril_qmi_ftr_multi_sim_info_type;

typedef enum
{
  QMI_RIL_FEATURE_UNKNOWN = 0,
  QMI_RIL_FEATURE_DISABLED,
  QMI_RIL_FEATURE_ENABLED
} qcril_qmi_feature_type;

typedef enum
{
  QMI_RIL_KDDI_HOLD_ANSWER_UNKNOWN = 0,
  QMI_RIL_KDDI_HOLD_ANSWER_DISABLED,
  QMI_RIL_KDDI_HOLD_ANSWER_ENABLED
} qcril_qmi_kddi_hold_answer_info_type;

#define QMI_RIL_SYS_PROP_NAME_BASEBAND    "ro.baseband"

#define QMI_RIL_SYS_PROP_NAME_SGLTE_CSFB  "persist.radio.sglte_csfb"

#define QMI_RIL_INI_RETRY_GAP_SEC     5

typedef enum
{
  QMI_RIL_FTR_BASEBAND_UNKNOWN = 0,
  QMI_RIL_FTR_BASEBAND_NONE,
  QMI_RIL_FTR_BASEBAND_APQ,
  QMI_RIL_FTR_BASEBAND_CSFB,
  QMI_RIL_FTR_BASEBAND_MSM,
  QMI_RIL_FTR_BASEBAND_SVLTE2,
  QMI_RIL_FTR_BASEBAND_SGLTE,
  QMI_RIL_FTR_BASEBAND_SGLTE2,
  QMI_RIL_FTR_BASEBAND_DSDA,
  QMI_RIL_FTR_BASEBAND_DSDA2,
  QMI_RIL_FTR_BASEBAND_MDM2,
} qcril_qmi_ftr_baseband_info_type;

typedef enum
{
  QMI_RIL_FTR_SGLTE_CSFB_UNKNOWN = 0,
  QMI_RIL_FTR_SGLTE_CSFB_YES,
  QMI_RIL_FTR_SGLTE_CSFB_NO
} qcril_qmi_ftr_sglte_csfb_info_type;

typedef enum
{
  QMI_RIL_FTR_RAT_UNKNOWN = 0,
  QMI_RIL_FTR_RAT_DISBLE,
  QMI_RIL_FTR_RAT_LEGACY,
  QMI_RIL_FTR_RAT_COMBINE
} qcril_qmi_ftr_rat_enable_option;

static qcril_qmi_feature_type            qmi_ril_oem_ind_to_both;
static qcril_qmi_feature_type            qmi_ril_ftr_suppress_req;
static qcril_qmi_ftr_multi_sim_info_type qmi_ril_multi_sim_ftr_info;
static qcril_qmi_ftr_rat_enable_option   qmi_ril_rat_enable_option;
static qcril_qmi_kddi_hold_answer_info_type qmi_ril_kddi_hold_answer;
static qcril_qmi_kddi_hold_answer_info_type qmi_ril_kddi_hold_answer;
static qcril_qmi_ftr_baseband_info_type qmi_ril_baseband_ftr_info; // inits to QMI_RIL_FTR_BASEBAND_UNKNOWN
static qcril_qmi_ftr_baseband_info_type qmi_ril_sglte_ftr_info; // inits to QMI_RIL_FTR_BASEBAND_UNKNOWN
static qcril_qmi_ftr_sglte_csfb_info_type  qmi_ril_sglte_csfb_ftr_info;
static qmi_ril_gen_operational_status_type qmi_ril_gen_operational_status;
static qcril_instance_id_e_type            qmi_ril_process_instance_id;
static uint32_t                            qmi_ril_sim_slot; ///< sim slot associated w/this ril instance
static pthread_t                           qmi_ril_init_retry_thread_pid;

typedef enum
{
  QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID,
  QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT,
  QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY,
  QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION
} qmi_ril_fw_android_param_copy_approach_type;


#define OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE    4
#define OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE       4
#define OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE             2
#define OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE             2

#define OEM_HOOK_QMI_TUNNELING_REQ_MODEM_ID_SIZE       8
#define OEM_HOOK_QMI_TUNNELING_REQ_MSG_SIZE            4
#define OEM_HOOK_QMI_TUNNELING_ERROR_CODE_SIZE 2

#define OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE      ( OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE +              \
                                                         OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE +                    \
                                                         OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE +                               \
                                                         OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE +                               \
                                                         OEM_HOOK_QMI_TUNNELING_ERROR_CODE_SIZE )

#define OEM_HOOK_QMI_TUNNELING_RESULT_SIZE   7

#define OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE      (QCRIL_OTHER_OEM_NAME_LENGTH)
#define OEM_HOOK_QMI_TUNNELING_IND_EVENT_ID_SIZE       4
#define OEM_HOOK_QMI_TUNNELING_IND_PLD_LEN_SIZE        4

#define OEM_HOOK_QMI_TUNNELING_IND_OVERHEAD_SIZE       ( OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE + OEM_HOOK_QMI_TUNNELING_IND_EVENT_ID_SIZE + \
                                                         OEM_HOOK_QMI_TUNNELING_IND_PLD_LEN_SIZE + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE + OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE )

#define MEM_PADDING "QCRILQMIqcrilqmi"
#define MEM_PADDING_SIZE 16

typedef struct
{
  pthread_mutex_t                             lock_mutex;
  pthread_mutexattr_t                         lock_mtx_atr;
} qmi_ril_atomic_lock_info_type;
static qmi_ril_atomic_lock_info_type qmi_ril_common_critical_section;

typedef struct
{
  int                       event_id;
  RIL_Token                 token;
  size_t                    original_data_len;

  qmi_ril_fw_android_param_copy_approach_type
                            param_copy_arrron;
  union
  {
    uint32        four_bytes;
    void*         dynamic_copy;
  } copied_params;
} qmi_ril_dedicated_request_exec_params_type;

#define QMI_RIL_FW_ANDROID_REQUEST_INFO_NO_PAYLOAD      ( 1 << 31 )
#define QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_INT     ( 1 << 30 )
#define QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_PTR     ( 1 << 29 )
#define QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE  ( 1 << 28 )

#define QMI_RIL_FW_ANDROID_REQUEST_INFO_SZ_MASK        ( (uint32_t)0xFFFFFFFF & ( ~( QMI_RIL_FW_ANDROID_REQUEST_INFO_NO_PAYLOAD |  \
                                                                                     QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_INT | \
                                                                                     QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_PTR | \
                                                                                     QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE | \
                                                                                     QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK \
                                                                                      ) ) )


#define QMI_RIL_FW_ANDROID_REQUEST_INFO_COMPOSE(desc, sz) ( ((uint32_t)sz & QMI_RIL_FW_ANDROID_REQUEST_INFO_SZ_MASK) | desc )

#define QMI_RIL_FW_ANDROID_REQUEST_INFO_DECOMPOSE_SZ(x) ( (uint32_t)x & QMI_RIL_FW_ANDROID_REQUEST_INFO_SZ_MASK )

typedef struct
{
  RIL_Token                 token;
  union
  {
    int                     param_int;
    void*                   param_ptr;
  } param_payload_holder;
  uint32_t                  param_info_and_len;
  uint32_t                  moniker;
} qmi_ril_fw_android_request_holder_type;



struct qmi_ril_fw_android_request_kind_execution_overview_type;

typedef struct qmi_ril_fw_android_request_kind_execution_overview_type
{
   RIL_Token                                token_under_execution;
   union
   {
     qmi_ril_fw_android_request_holder_type   local_holder;
     qmi_ril_fw_android_request_holder_type*  extra_holders;
   } holders;
   struct qmi_ril_fw_android_request_kind_execution_overview_type *
                                              family_ring;
   uint32_t                                   nof_extra_holders_and_caps_and_dynamics;
   uint32_t                                   chief_moniker;
   int                                        original_android_request_id;
   int                                        family_pair_android_request_id;
} qmi_ril_fw_android_request_kind_execution_overview_type;

/* QCRIL android pending unsol resp */
typedef struct qmi_ril_android_pending_unsol_resp_type
{
  boolean valid;
  const char *logstr;
  uint8 instance_id;

  qmi_ril_fw_android_param_copy_approach_type param_copy_arrron;
  void* copied_params;
  size_t copied_param_len;

  int8 num_of_locker;
} qmi_ril_android_pending_unsol_resp_type;

typedef struct
{
  pthread_mutex_t pending_unsol_resp_mutex;
  qmi_ril_android_pending_unsol_resp_type* pending_unsol_resp_list;
} qmi_ril_android_pending_unsol_resp_overview_type;

typedef struct esoc_mdm_info {
    boolean                             pm_feature_supported;
    boolean                             esoc_feature_supported;
    int                                 esoc_fd;
    int                                 voting_state; // 0 - vote released; 1 - vote activated
    char                                link_name[MAX_NAME_LEN];
    char                                modem_name[MAX_NAME_LEN];
    char                                esoc_node[MAX_NAME_LEN];
    pthread_mutex_t                     mdm_mutex;
    pthread_mutexattr_t                 mdm_attr;
    MdmType                             type;
} qcril_mdm_info;

static qmi_ril_android_pending_unsol_resp_overview_type qmi_ril_android_pending_unsol_resp;
extern char *qcril_qmi_get_esoc_link_name();
extern char *qcril_qmi_get_esoc_modem_name();
extern void qcril_qmi_load_esoc_info();

static void qcril_request_suppress_list_init();

#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY                      ( (uint32_t)1 << 31 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE                         ( (uint32_t)1 << 30 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FLOW_CONTROL_EXEMPT                    ( (uint32_t)1 << 29 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE                       ( (uint32_t)1 << 28 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING                         ( (uint32_t)1 << 27 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_MULTIPLE_AUTO_DROP_ON_DIFF_PARAMS      ( (uint32_t)1 << 26 )
#define QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK       ( (uint32_t)1 << 25 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF                        ( (uint32_t)1 << 24 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_VOICE_CALL_SPECIFIC_DROP_OFF    ( (uint32_t)1 << 23 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FAMILY_RING_DEFINED_PAIR               ( (uint32_t)1 << 22 )
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF                        ( (uint32_t)1 << 21 )

#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_QUEUE_SZ_MASK        ( (uint32_t)0xFFFFFFFF & \
                                                                              ( ~( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FLOW_CONTROL_EXEMPT | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_VOICE_CALL_SPECIFIC_DROP_OFF | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_MULTIPLE_AUTO_DROP_ON_DIFF_PARAMS | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FAMILY_RING_DEFINED_PAIR | \
                                                                                   QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF \
                                                                                   ) ) )



#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ(x)   ( ((uint32_t)x & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_QUEUE_SZ_MASK) )

#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( caps_mask, queue_sz )   ( ((uint32_t)queue_sz & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_QUEUE_SZ_MASK) | caps_mask )

#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_SINGLE (1)
#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF (16)

#define QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_OLDER  (1)
#define QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_NEWER  (-1)
#define QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_EQUAL  (0)

static uint32_t qmi_ril_fw_android_request_get_handling_capabilities( int android_request_id );

static void qmi_ril_fw_android_request_flow_control_info_lock( void );
static void qmi_ril_fw_android_request_flow_control_info_unlock( void );

#define QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID    (256)
#define QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID             (1043) // start from RIL_UNSOL_RESPONSE_BASE

#define QCRIL_MDM_LOCK()      pthread_mutex_lock(&esoc_info.mdm_mutex);
#define QCRIL_MDM_UNLOCK()    pthread_mutex_unlock(&esoc_info.mdm_mutex);

static void qmi_ril_fw_android_request_flow_control_init( void );

typedef struct
{
   qmi_ril_atomic_lock_info_type                            guard;
   qmi_ril_fw_android_request_kind_execution_overview_type* android_request_kind_info;
   int                                                      in_exec_on_main_thread;
   uint32_t                                                 common_moniker_book;
} qmi_ril_fw_android_request_flow_control_overview_type;

static void qmi_ril_fw_android_request_flow_control_trigger_remains(qcril_timed_callback_handler_params_type * handler_params);

static qmi_ril_fw_android_request_flow_control_overview_type qmi_ril_fw_android_request_flow_control_overview;

static RIL_Errno qmi_ril_fw_android_request_render_execution( RIL_Token token,
                                                       int android_request_id,
                                                       void * android_request_data,
                                                       int android_request_data_len,
                                                       qcril_instance_id_e_type  instance_id,
                                                       int * is_dedicated_thread);

static qmi_ril_fw_android_request_holder_type * qmi_ril_fw_android_request_flow_control_find_outstanding_request_within_kind( qmi_ril_fw_android_request_kind_execution_overview_type* origin );
static int qmi_ril_fw_android_request_flow_control_find_request_for_execution( qmi_ril_fw_android_request_kind_execution_overview_type* origin,
                                                                        qmi_ril_fw_android_request_kind_execution_overview_type** exec_overview,
                                                                        qmi_ril_fw_android_request_holder_type ** exec_req_holder );
static qmi_ril_fw_android_request_kind_execution_overview_type* qmi_ril_fw_android_request_flow_control_find_busy_kind( qmi_ril_fw_android_request_kind_execution_overview_type* origin );
static void qmi_ril_fw_android_request_flow_control_abandon_requests_local_and_ring_zoned( qmi_ril_fw_android_request_kind_execution_overview_type* origin, RIL_Errno cause, int is_unbind_cleanup );
static void qmi_ril_fw_android_request_flow_control_abandon_requests_local_only( qmi_ril_fw_android_request_kind_execution_overview_type* origin, RIL_Errno cause, int is_unbind_cleanup );
static void qmi_ril_fw_android_request_flow_control_release_holder_info_bookref( qmi_ril_fw_android_request_holder_type* request_holder_org, int android_request_id );
static int qmi_ril_fw_android_request_flow_control_request_holders_have_same_param( qmi_ril_fw_android_request_holder_type* origin, qmi_ril_fw_android_request_holder_type* peer );
static void qmi_ril_fw_android_request_flow_control_declare_family_ring( const int * android_request_ids );
static int qmi_ril_fw_android_request_flow_control_moniker_compare( uint32_t moniker1, uint32_t moniker2 ) ;
static void qmi_ril_fw_android_request_flow_control_overview_request_review_holders( qmi_ril_fw_android_request_kind_execution_overview_type* origin );

static void * qmi_ril_fw_dedicated_request_exec_thread(void * param);


static qmi_ril_fw_android_param_copy_approach_type qmi_ril_fw_create_android_live_params_copy(int android_request_id,
                                               void * android_request_data,
                                               int android_request_data_len,
                                               void* four_byte_storage,
                                               int* four_byte_storage_occupied,
                                               void** sub_created_custom_storage,
                                               int* custom_storage_len);

static void qmi_ril_fw_destroy_android_live_params_copy(qmi_ril_fw_android_param_copy_approach_type used_approach,
                                               int android_request_id,
                                               void* four_byte_storage,
                                               void* sub_created_custom_storage);

static boolean qmi_ril_check_android_unsol_resp_dispatchable(int resp_id);
static void qmi_ril_init_android_unsol_resp_pending_list();

static void qmi_ril_android_pending_unsol_resp_lock( void );
static void qmi_ril_android_pending_unsol_resp_unlock( void );

static void qmi_ril_add_unsol_resp_to_pending_list(qcril_unsol_resp_params_type *param_ptr);
static qmi_ril_android_pending_unsol_resp_type* qmi_ril_get_unsol_resp_from_pending_list(int resp_id);
static void qmi_ril_free_pending_unsol_resp(qmi_ril_android_pending_unsol_resp_type* resp, int resp_id);
static void qmi_ril_bootup_actition_on_rild_atel_link_connect(void * params);
static boolean qcril_qmi_is_pm_voting_feature_supported();
static boolean qcril_qmi_is_esoc_voting_feature_supported();

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES


===========================================================================*/

void RIL_removeTimedCallback(void *param);
int qcril_qmi_modem_power_voting_state();

boolean qcril_request_check_if_suppressed(int event_id);
int qcril_request_suppress_request(int event_id);
void qcril_request_clean_up_suppress_list();


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES


===========================================================================*/

static void onRequest_rid( int request, void *data, size_t datalen, RIL_Token t );
static RIL_RadioState currentState_rid();
static int onSupports_rid( int request );
static void onCancel_rid( RIL_Token t );
static const char *getVersion_rid( void );

static RIL_Errno qmi_ril_core_init(void);
static RIL_Errno qmi_ril_initiate_core_init_retry(void);
static void * qmi_ril_core_init_kicker_thread_proc(void* empty_param);
static void qmi_ril_initiate_bootup(void);
static void qmi_ril_bootup_perform_core_or_start_polling(void * params);
static void qmi_ril_core_init_kicker_main_threaded_proc(void* empty_param);
static void qcril_free_request_list_entry_deferred( qcril_timed_callback_handler_params_type * handler_params );
static void qcril_print_heap_memory_list();

pthread_t qmi_ril_fw_get_main_thread_id();

static void qmi_ril_oem_hook_init();

boolean qcril_qmi_modem_power_is_voting_feature_supported();

RIL_Errno qmi_ril_oem_hook_get_request_id
(
  uint16   service_id,
  uint16   message_id,
  uint32   *request_id,
  char **  log_ind
);

static const RIL_RadioFunctions qcril_request_api[] = {
  { RIL_VERSION, onRequest_rid, currentState_rid, onSupports_rid, onCancel_rid, getVersion_rid }
};

static pthread_t qmi_ril_main_thread_id;

/* esoc info */
qcril_mdm_info                          esoc_info;

#ifdef RIL_SHLIB
struct RIL_Env *qcril_response_api[ QCRIL_MAX_INSTANCE_ID ]; /*!< Functions for ril to call */
#endif /* RIL_SHLIB */


/*===========================================================================

                                FUNCTIONS

===========================================================================*/


/*===========================================================================

  FUNCTION:  qcril_get_baseband_name

===========================================================================*/
/*!
    @brief
    retrieve baseband info

    @return
    None
*/
/*=========================================================================*/
void qcril_get_baseband_name(char *prop_str)
{
    char *link_name;
    if (prop_str)
    {
        property_get( QMI_RIL_SYS_PROP_NAME_BASEBAND, prop_str, "" );

        if (!strcmp("mdm", prop_str) || !strcmp("mdm2", prop_str))
        {
            link_name = qcril_qmi_get_esoc_link_name();
            if (link_name)
            {
                QCRIL_LOG_INFO("link name %s ", link_name);
                if (!strcmp("HSIC", link_name))
                {
                    strlcpy(prop_str, "mdm", PROPERTY_VALUE_MAX);
                }
                else
                {
                    strlcpy(prop_str, "mdm2", PROPERTY_VALUE_MAX);
                }
            }
        }
    }

    return;
}

//===========================================================================
// qmi_ril_clear_timed_callback_list
//===========================================================================
void qmi_ril_clear_timed_callback_list()
{
  qcril_timed_callback_info *cur;
  qcril_timed_callback_info *next;
  QCRIL_MUTEX_LOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");
  cur = qcril_timed_callback_list;

  while ( NULL != cur )
  {
    if (cur->need_free && NULL != cur->extra_params)
    {
      qcril_free(cur->extra_params);
    }
    next = cur->next;
    qcril_free(cur);
    cur = next;
  }

  qcril_timed_callback_list = NULL;
  QCRIL_MUTEX_UNLOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");

  // add heap memory print callback back
  if ( 1 == is_heap_memory_tracked )
  {
    qcril_setup_timed_callback(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                               qcril_print_heap_memory_list, &HEAP_MEM_LIST_PRINT_TIMEVAL_DELAY, NULL );
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_clear_timed_callback_list

//===========================================================================
// qmi_ril_is_multi_sim_feature_supported
//===========================================================================
int qmi_ril_is_multi_sim_feature_supported()
{
   return ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDS) ||
            qmi_ril_is_feature_supported(QMI_RIL_FEATURE_TSTS) );
} // qmi_ril_is_multi_sim_feature_supported

//===========================================================================
// qmi_ril_is_feature_supported
//===========================================================================
int qmi_ril_is_feature_supported(int feature)
{
  int res = FALSE;
  char prop_str[ PROPERTY_VALUE_MAX ];

  switch ( feature )
  {
    case QMI_RIL_FEATURE_FUSION_CSFB:
    case QMI_RIL_FEATURE_APQ:
    case QMI_RIL_FEATURE_MSM:
    case QMI_RIL_FEATURE_SVLTE2:
    case QMI_RIL_FEATURE_SGLTE:
    case QMI_RIL_FEATURE_DSDA:
    case QMI_RIL_FEATURE_DSDA2:
    case QMI_RIL_FEATURE_PCI:
      if ( QMI_RIL_FTR_BASEBAND_UNKNOWN ==  qmi_ril_baseband_ftr_info)
      {
        *prop_str = 0;

        qcril_get_baseband_name(prop_str);

        if ( strcmp(prop_str, "apq") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_APQ;
        }
        else if ( strcmp(prop_str, "mdm") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_APQ;
        }
        else if ( strcmp(prop_str, "mdm2") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_MDM2;
        }
        else if ( strcmp(prop_str, "msm") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_MSM;
        }
        else if ( strcmp(prop_str, "csfb") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_CSFB;
        }
        else if ( strcmp(prop_str, "svlte2a") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_SVLTE2;
        }
        else if ( strcmp(prop_str, "sglte2") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_SGLTE;
        }
        else if ( strcmp(prop_str, "sglte") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_SGLTE;
        }
        else if ( strcmp(prop_str, "dsda") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_DSDA;
        }
        else if ( strcmp(prop_str, "dsda2") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_DSDA2;
        }
        else if ( strcmp(prop_str, "auto") == 0)
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_APQ;
        }
        else
        {
          qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_NONE;
        }
      }

      if ( QMI_RIL_FEATURE_FUSION_CSFB == feature && QMI_RIL_FTR_BASEBAND_CSFB == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else if ( QMI_RIL_FEATURE_APQ == feature && QMI_RIL_FTR_BASEBAND_APQ == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else if ( QMI_RIL_FEATURE_APQ == feature && QMI_RIL_FTR_BASEBAND_MDM2 == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else if ( QMI_RIL_FEATURE_PCI == feature && QMI_RIL_FTR_BASEBAND_MDM2 == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else if ( QMI_RIL_FEATURE_MSM == feature && QMI_RIL_FTR_BASEBAND_MSM == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else if ( QMI_RIL_FEATURE_SVLTE2 == feature && QMI_RIL_FTR_BASEBAND_SVLTE2 == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else if ( QMI_RIL_FEATURE_SGLTE == feature && QMI_RIL_FTR_BASEBAND_SGLTE == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else if ( QMI_RIL_FEATURE_DSDA == feature && QMI_RIL_FTR_BASEBAND_DSDA == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else if ( QMI_RIL_FEATURE_DSDA2 == feature && QMI_RIL_FTR_BASEBAND_DSDA2 == qmi_ril_baseband_ftr_info)
      {
        res = TRUE;
      }
      else
      {
        res = FALSE;
      }
      break;

    case QMI_RIL_FEATURE_SGLTE2:
      if ( QMI_RIL_FTR_BASEBAND_UNKNOWN == qmi_ril_sglte_ftr_info)
      {
        *prop_str = 0;
        property_get( QMI_RIL_SYS_PROP_NAME_BASEBAND, prop_str, "" );
        if ( strcmp(prop_str, "sglte2") == 0)
        {
          qmi_ril_sglte_ftr_info = QMI_RIL_FTR_BASEBAND_SGLTE2;
        }
      }

      switch ( qmi_ril_sglte_ftr_info )
      {
        case QMI_RIL_FTR_BASEBAND_SGLTE2:
          res = TRUE;
          break;

        default:
          res = FALSE;
          break;
      }

      break;
    case QMI_RIL_FEATURE_SGLTE_CSFB:
      if ( QMI_RIL_FTR_SGLTE_CSFB_UNKNOWN == qmi_ril_sglte_csfb_ftr_info )
      {
        *prop_str = 0;
        property_get( QMI_RIL_SYS_PROP_NAME_SGLTE_CSFB, prop_str, "" );
        if ( strncmp(prop_str, "true", QMI_RIL_SYS_PROP_SGLTE_CSFB_LENGTH ) == 0)
        {
          qmi_ril_sglte_csfb_ftr_info = QMI_RIL_FTR_SGLTE_CSFB_YES;
        }
        else
        {
          qmi_ril_sglte_csfb_ftr_info = QMI_RIL_FTR_SGLTE_CSFB_NO;
        }
      }

      switch ( qmi_ril_sglte_csfb_ftr_info )
      {
        case QMI_RIL_FTR_SGLTE_CSFB_YES:
          res = TRUE;
          break;

        default:
          res = FALSE;
          break;
      }

      break;

    case QMI_RIL_FEATURE_DSDS:
    case QMI_RIL_FEATURE_TSTS:
      if ( QMI_RIL_FTR_MULTI_SIM_UNKNOWN == qmi_ril_multi_sim_ftr_info )
      {
        *prop_str = 0;
        property_get( QMI_RIL_SYS_PROP_NAME_MULTI_SIM, prop_str, "" );
        if ( strncmp(prop_str, "dsds", QMI_RIL_SYS_PROP_LENGTH_MULTI_SIM ) == 0)
        {
          qmi_ril_multi_sim_ftr_info = QMI_RIL_FTR_DSDS_ENABLED;
        }
        else if ( strncmp(prop_str, "tsts", QMI_RIL_SYS_PROP_LENGTH_MULTI_SIM ) == 0)
        {
          qmi_ril_multi_sim_ftr_info = QMI_RIL_FTR_TSTS_ENABLED;
        }
        else if ( strncmp(prop_str, "dsda", QMI_RIL_SYS_PROP_LENGTH_MULTI_SIM ) == 0)
        {
          qmi_ril_multi_sim_ftr_info = QMI_RIL_FTR_DSDA_ENABLED;
        }
        else
        {
          qmi_ril_multi_sim_ftr_info = QMI_RIL_FTR_MULTI_SIM_DISABLED;
        }
      }

      if ( (feature == QMI_RIL_FEATURE_DSDS) &&
           ( ( qmi_ril_multi_sim_ftr_info == QMI_RIL_FTR_DSDS_ENABLED) ||
             ( qmi_ril_multi_sim_ftr_info == QMI_RIL_FTR_DSDA_ENABLED) ) )
      {
          // DSDS/DSDA is enabled
          res = TRUE;
      }
      else if ( (feature == QMI_RIL_FEATURE_TSTS) &&
                (qmi_ril_multi_sim_ftr_info == QMI_RIL_FTR_TSTS_ENABLED) )
      {
          // TSTS is enabled
          res = TRUE;
      }
      else
      {

          res = FALSE;
      }
      break;

    case QMI_RIL_FEATURE_LEGACY_RAT:
    case QMI_RIL_FEATURE_COMBINE_RAT:
      if ( QMI_RIL_FTR_RAT_UNKNOWN == qmi_ril_rat_enable_option)
      {
        *prop_str = 0;
        property_get( QMI_RIL_SYS_PROP_RAT_OPTION, prop_str, "" );
        if ( !strncmp(prop_str, "legacy", 6) )
        {
          qmi_ril_rat_enable_option = QMI_RIL_FTR_RAT_LEGACY;
        }
        else if ( !strncmp(prop_str, "combine", 7) )
        {
          qmi_ril_rat_enable_option = QMI_RIL_FTR_RAT_COMBINE;
        }
        else
        {
          qmi_ril_rat_enable_option = QMI_RIL_FTR_RAT_DISBLE;
        }
      }

      if ( (feature == QMI_RIL_FEATURE_LEGACY_RAT) &&
            (qmi_ril_rat_enable_option == QMI_RIL_FTR_RAT_LEGACY) )
      {
        res = TRUE;
      }
      else if ( (feature == QMI_RIL_FEATURE_COMBINE_RAT) &&
            (qmi_ril_rat_enable_option == QMI_RIL_FTR_RAT_COMBINE) )
      {
        res = TRUE;
      }
      else
      {
        res = FALSE;
      }
      break;

    case QMI_RIL_FEATURE_KDDI_HOLD_ANSWER:
      res = FALSE;
      char prop[ PROPERTY_VALUE_MAX ];

      if ( QMI_RIL_KDDI_HOLD_ANSWER_UNKNOWN == qmi_ril_kddi_hold_answer)
      {
        property_get( QCRIL_FEATURE_KDDI_HOLD_ANSWER_ON, prop, "" );
        if ((strlen(prop) > 0) && atoi(prop))
        {
          qmi_ril_kddi_hold_answer = QMI_RIL_KDDI_HOLD_ANSWER_ENABLED;
        }
        else
        {
          qmi_ril_kddi_hold_answer = QMI_RIL_KDDI_HOLD_ANSWER_DISABLED;
        }
      }

      switch ( qmi_ril_kddi_hold_answer )
      {
        case QMI_RIL_KDDI_HOLD_ANSWER_ENABLED:
          res = TRUE;
          break;

        case QMI_RIL_KDDI_HOLD_ANSWER_DISABLED:
        default:
          res = FALSE;
          break;
      }
      break;

    case QMI_RIL_FEATURE_8960:
#ifdef FEATURE_QCRIL_8960
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_7627A:
#ifdef FEATURE_QCRIL_7627A
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_8974:
#ifdef FEATURE_QCRIL_8974
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_8610:
#ifdef FEATURE_QCRIL_8610
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_8916:
#ifdef FEATURE_QCRIL_8916
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_8994:
#ifdef FEATURE_QCRIL_8994
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_8909:
#ifdef FEATURE_QCRIL_8909
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_8084:
#ifdef FEATURE_QCRIL_8084
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_8226:
#ifdef FEATURE_QCRIL_8226
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_SHM:
#ifdef FEATURE_QCRIL_SHM
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_ICS:
#ifdef QMI_RIL_IS_ICS
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_JB:
#ifdef QMI_RIL_IS_JB
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_KK:
#ifdef QMI_RIL_IS_KK
      res = TRUE;
#else
      res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_IMS:
    /* SMS over IMS is supported only if FEATURE_QCRIL_IMS is defined */
#ifdef FEATURE_QCRIL_IMS
      res = TRUE;
#else
      res = FALSE;
#endif
    break;

    case QMI_RIL_FEATURE_TDSCDMA_UI:
    /* TDSCDMA option in UI is supported only if FEATURE_TDSCDMA_UI is defined */
#ifdef FEATURE_TDSCDMA_UI
      res = TRUE;
#else
      res = FALSE;
#endif
    break;

    case QMI_RIL_FEATURE_VOIP_VT:
     /* only if modify call initiate is defined */
    #if !defined(QMI_RIL__RIL_VT_VOLTE_EMULATED)
    res = TRUE;
    #endif
    break;

    case QMI_RIL_FEATURE_IMS_RETRY_3GPP:
      // feature is enabled (true) by default, eg. if property set to anything
      // other than "false" or "0", including if its missing (unset).
      *prop_str = 0;
      property_get(QMI_RIL_SYS_PROP_NAME_IMS_RETRY_ON_3GPP, prop_str, "");
      res = ( (strcmp(prop_str, "false") != 0) && (strcmp(prop_str, "0") != 0) );
      QCRIL_LOG_DEBUG("property %s = \"%s\", %d",
                       QMI_RIL_SYS_PROP_NAME_IMS_RETRY_ON_3GPP, prop_str, res);
      break;

    case QMI_RIL_FEATURE_IMS_RETRY_3GPP2:
      // feature is enabled (true) by default, eg. if property set to anything
      // other than "false" or "0", including if its missing (unset).
      *prop_str = 0;
      property_get(QMI_RIL_SYS_PROP_NAME_IMS_RETRY_ON_3GPP2, prop_str, "");
      res = ( (strcmp(prop_str, "false") != 0) && (strcmp(prop_str, "0") != 0) );
      QCRIL_LOG_DEBUG("property %s = \"%s\", %d",
                       QMI_RIL_SYS_PROP_NAME_IMS_RETRY_ON_3GPP2, prop_str, res);
      break;

    case QMI_RIL_FEATURE_PLAIN_ANDROID:
#if defined(QMI_RIL_UNDER_PLAIN_ANDROID)
    res = TRUE;
#else
    res = FALSE;
#endif
      break;

    case QMI_RIL_FEATURE_OEM_IND_TO_BOTH:
      // feature is enabled (true) by default, eg. if property set to anything
      // other than "false" or "0", including if its missing (unset).
      if (QMI_RIL_FEATURE_UNKNOWN == qmi_ril_oem_ind_to_both)
      {
        *prop_str = 0;
        property_get(QMI_RIL_SYS_PROP_NAME_OEM_IND_TO_BOTH, prop_str, "");
        if ( (strcmp(prop_str, "false") != 0) && (strcmp(prop_str, "0") != 0) )
        {
          qmi_ril_oem_ind_to_both = QMI_RIL_FEATURE_ENABLED;
        }
        else
        {
          qmi_ril_oem_ind_to_both = QMI_RIL_FEATURE_DISABLED;
        }
      }

      if (QMI_RIL_FEATURE_ENABLED == qmi_ril_oem_ind_to_both)
      {
        res = TRUE;
      }
      else
      {
        res = FALSE;
      }
      break;

    case QMI_RIL_FEATURE_OEM_SOCKET:
      // feature is enabled (true) by default, eg. if property set to anything
      // other than "false" or "0", including if its missing (unset).
      *prop_str = 0;
      property_get(QMI_RIL_SYS_PROP_NAME_OEM_SOCKET, prop_str, "");
      res = ( (strcmp(prop_str, "false") != 0) && (strcmp(prop_str, "0") != 0) );
      break;

    case QMI_RIL_FEATURE_POSIX_CLOCKS:
#ifdef HAVE_POSIX_CLOCKS
      res = TRUE;
#else
      res = FALSE;
#endif
    break;

    case QMI_RIL_FEATURE_SAP_SILENT_PIN_VERIFY:
      /* feature is disabled by default, eg. if property set to anything
         other than "true" or "1", including if its missing (unset). */
      *prop_str = 0;
      property_get(QCRIL_FEATURE_SAP_SILENT_PIN_VERIFY, prop_str, "");
      res = ( (strcmp(prop_str, "true") == 0) || (strcmp(prop_str, "1") == 0) );
      break;

    case QMI_RIL_FEATURE_SRLTE:
      res = qcril_qmi_is_srlte_supported();
      break;

    case QMI_RIL_FEATURE_ATEL_STKCC:
#if RIL_UNSOL_ON_SS == 11038
      res = FALSE;
#else
      res = TRUE;
#endif
      break;

    case QMI_RIL_FEATURE_SUPPRESS_REQ:
      /* feature is disabled by default, eg. if property set to anything
         other than "true" or "1", including if its missing (unset). */
      if (QMI_RIL_FEATURE_UNKNOWN == qmi_ril_ftr_suppress_req)
      {
        *prop_str = 0;
        property_get(QMI_RIL_SYS_PROP_NAME_SUPPRESS_REQ, prop_str, "");
        if ((strcmp(prop_str, "true") == 0) || (strcmp(prop_str, "1") == 0))
        {
          qmi_ril_ftr_suppress_req = QMI_RIL_FEATURE_ENABLED;
        }
        else
        {
          qmi_ril_ftr_suppress_req = QMI_RIL_FEATURE_DISABLED;
        }
      }

      switch ( qmi_ril_ftr_suppress_req )
      {
        case QMI_RIL_FEATURE_ENABLED:
          res = TRUE;
          break;

        default:
          res = FALSE;
          break;
      }
      break;

    default:
      res = FALSE;
      break;
  }

  return res;
} // qmi_ril_is_feature_supported

//===========================================================================
// qmi_ril_is_qcom_ril_version_supported
//===========================================================================
int qmi_ril_is_qcom_ril_version_supported(int version)
{
  int res = FALSE;

  #if defined(RIL_QCOM_VERSION)
  if( version > QMI_RIL_ZERO && RIL_QCOM_VERSION >= version )
  {
    res = TRUE;
  }
  #else
  QCRIL_NOTUSED(version);
  #endif

  return res;
} //qmi_ril_is_qcom_ril_version_supported

/*=========================================================================
  FUNCTION:  qcril_qmi_print_hex

===========================================================================*/
/*!
    @brief
    Print specified buffer context in hexadecimal format.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_print_hex
(
  unsigned char *msg,
  int            msg_len
)
{
  #define QCRIL_PRINT_MAX_BYTES_PER_LINE 16
  const char hex_chart[] = {'0', '1', '2', '3',
                            '4', '5', '6', '7',
                            '8', '9', 'A', 'B',
                            'C', 'D', 'E', 'F'}; // Do not change this array
  unsigned char buf[QCRIL_PRINT_MAX_BYTES_PER_LINE * 3 + 1];
  unsigned int buf_idx = 0;                 // 0 to QCRIL_PRINT_MAX_BYTES_PER_LINE * 3
  unsigned int msg_idx = 0;                 // 0 to msg_len - 1
  unsigned int bytes_per_line_idx = 0;      // 0 to QCRIL_PRINT_MAX_BYTES_PER_LINE - 1
  do
  {
    if (NULL == msg || msg_len <= 0)
    {
      break;
    }
    while (msg_idx < msg_len)
    {
      for (bytes_per_line_idx = 0, buf_idx = 0;
          (bytes_per_line_idx < QCRIL_PRINT_MAX_BYTES_PER_LINE) && (msg_idx < msg_len);
           bytes_per_line_idx++, msg_idx++)
      {
        buf[buf_idx] = hex_chart[(msg[msg_idx] >> 4) & 0x0F];
        buf_idx++;
        buf[buf_idx] = hex_chart[msg[msg_idx] & 0x0F];
        buf_idx++;
        buf[buf_idx] = ' ';
        buf_idx++;
      }
      buf[buf_idx] = '\0';
      QCRIL_LOG_DEBUG("%s", buf);
    }
  } while (FALSE);
} /* qcril_qmi_print_hex */

//===========================================================================
// ril_to_uim_is_tsts_enabled
//===========================================================================
int ril_to_uim_is_tsts_enabled(void)
{
  return qmi_ril_is_feature_supported( QMI_RIL_FEATURE_TSTS );
} // ril_to_uim_is_tsts_enabled

//===========================================================================
// ril_to_uim_is_dsds_enabled
//===========================================================================
int ril_to_uim_is_dsds_enabled(void)
{
  return (qmi_ril_is_feature_supported( QMI_RIL_FEATURE_DSDS ));
} // ril_to_uim_is_dsds_enabled

//===========================================================================
// qmi_ril_is_multi_sim_oem_hook_request
//===========================================================================
/*!
    @brief
    Checks if it is a multi sim OEM_HOOK request that
    is supposed to be responded on rild socket.

    @return
    TRUE, if multi sim OEM_HOOK request.
    FALSE, otherwise.
*/
/*=========================================================================*/
int qmi_ril_is_multi_sim_oem_hook_request (int req_res_id)
{
  int res = FALSE;
  switch (req_res_id)
  {
      case QCRIL_EVT_HOOK_UNSOL_VOICE_SYSTEM_ID:      // fall through
      case QCRIL_EVT_HOOK_UNSOL_MODEM_CAPABILITY:     // fall through
      case QCRIL_EVT_HOOK_SET_DEFAULT_VOICE_SUB:      // fall through
      case QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD:        // fall through
      case QCRIL_EVT_HOOK_GET_MODEM_CAPABILITY:
      case QCRIL_EVT_HOOK_UPDATE_SUB_BINDING:
      res = TRUE;
      break;

      default:
      res = FALSE;
      break;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
} // qmi_ril_is_multi_sim_oem_hook_request

qcril_timed_callback_info **_qcril_find_timed_callback_locked(uint32 timer_id)
{
  qcril_timed_callback_info **i;

  for (i = &qcril_timed_callback_list; *i ; i = &((*i)->next)) {
    if ((*i)->timer_id == timer_id) {
      break;
    }
  }

  return i;

}
qcril_timed_callback_info *qcril_find_and_remove_timed_callback(uint32 timer_id)
{
  qcril_timed_callback_info **i, *ret;
  QCRIL_MUTEX_LOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");

  i = _qcril_find_timed_callback_locked(timer_id);

  ret = *i;
  if ( NULL != ret )
  {
    if ( NULL != ret->next )
    {
      ret->next->prev = ret->prev;
    }
    *i = ret->next;

    ret->next = NULL;
    ret->prev = NULL;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");
  return ret;

}

void qcril_add_timed_callback(qcril_timed_callback_info *info)
{
  qcril_timed_callback_info **i;
  qcril_timed_callback_info *prev = NULL;
  QCRIL_MUTEX_LOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");

  for (i = &qcril_timed_callback_list; *i; i = &((*i)->next))
  {
    prev = *i;
  }
  *i = info;
  info->next = NULL;
  info->prev = prev;
  QCRIL_MUTEX_UNLOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");
}

//===========================================================================
// qcril_print_heap_memory_list
//===========================================================================
void qcril_print_heap_memory_list()
{
  qcril_heap_list_info *i;
  QCRIL_LOG_INFO(" ************ print heap memory list ************");
  QCRIL_MUTEX_LOCK( &qcril_heap_memory_list_mutex, "heap memory list mutex");
  for (i = qcril_heap_memory_list; i; i = i->next)
  {
    QCRIL_LOG_INFO("\tid: %u, func: %s, line: %u, size: %u", i->mem_id, i->func_name, i->line_number, i->size);
  }
  QCRIL_MUTEX_UNLOCK( &qcril_heap_memory_list_mutex, "heap memory list mutex");
  qcril_setup_timed_callback(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                             qcril_print_heap_memory_list, &HEAP_MEM_LIST_PRINT_TIMEVAL_DELAY, NULL );
} // qcril_print_heap_memory_list

//===========================================================================
// qcril_find_heap_memory_locked
//===========================================================================
qcril_heap_list_info **qcril_find_heap_memory_locked(void* addr)
{
  qcril_heap_list_info **i;

  for (i = &qcril_heap_memory_list; *i ; i = &((*i)->next)) {
    if ((*i)->addr == addr) {
      break;
    }
  }

  return i;
} // qcril_find_heap_memory_locked

//===========================================================================
// qcril_remove_heap_memory_from_list
//===========================================================================
qcril_heap_list_info *qcril_remove_heap_memory_from_list(void* addr)
{
  qcril_heap_list_info **i, *ret;
  QCRIL_MUTEX_LOCK( &qcril_heap_memory_list_mutex, "heap memory list mutex");

  i = qcril_find_heap_memory_locked(addr);

  ret = *i;
  if ( NULL != ret )
  {
    if ( NULL != ret->next )
    {
      ret->next->prev = ret->prev;
    }
    *i = ret->next;

    ret->next = NULL;
    ret->prev = NULL;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_heap_memory_list_mutex, "heap memory list mutex");
  return ret;
} // qcril_remove_heap_memory_from_list

//===========================================================================
// qcril_add_heap_memory_to_list
//===========================================================================
void qcril_add_heap_memory_to_list(qcril_heap_list_info *info)
{
  qcril_heap_list_info **i;
  qcril_heap_list_info *prev = NULL;
  QCRIL_MUTEX_LOCK( &qcril_heap_memory_list_mutex, "heap memory list mutex");

  for (i = &qcril_heap_memory_list; *i; i = &((*i)->next))
  {
    prev = *i;
  }
  *i = info;
  info->next = NULL;
  info->prev = prev;
  QCRIL_MUTEX_UNLOCK( &qcril_heap_memory_list_mutex, "heap memory list mutex");
} // qcril_add_heap_memory_to_list

/*=========================================================================
  FUNCTION:  qcril_timed_callback_dispatch

===========================================================================*/
/*!
    @brief
    Dispatch function for all timed callbacks

    @return
    void
*/
/*=========================================================================*/
void qcril_timed_callback_dispatch
(
  void *param
)
{
  uint32            timer_id = (uintptr_t) param;
  RIL_TimedCallback cb;
  qcril_timed_callback_info *info = qcril_find_and_remove_timed_callback(timer_id);

  if (info)
  {
    cb = info->callback;
    cb((void *)(uintptr_t)timer_id);
    qcril_free(info);
  }
} /* qcril_timed_callback_dispatch */

/*=========================================================================
  FUNCTION:  qcril_setup_timed_callback

===========================================================================*/
/*!
    @brief
    Setup RIL callback timer

    @return
    0 on success.
*/
/*=========================================================================*/
int qcril_setup_timed_callback
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  RIL_TimedCallback callback,
  const struct timeval *relativeTime,
  uint32 *timer_id
)
{
  qcril_timed_callback_info *tcbinfo;
  int ret = -1;
  uint32 the_timer_id;


  if ((instance_id < QCRIL_MAX_INSTANCE_ID) && (modem_id < QCRIL_MAX_MODEM_ID))
  {
      tcbinfo = qcril_malloc(sizeof(qcril_timed_callback_info));
      if (tcbinfo)
      {

        QCRIL_MUTEX_LOCK( &qcril_timer_id_mutex, "qcril_timer_id_mutex" );

        /* Most Significant 16 bits are the Instance ID + Modem ID and Least Significant 16 bits are the QCRIL Timer ID */
        the_timer_id = ( uint32 ) QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, qcril_timer_id );
        qcril_timer_id++;
        if( 0 == qcril_timer_id )
        {
            qcril_timer_id = 1;
        }

        QCRIL_MUTEX_UNLOCK( &qcril_timer_id_mutex, "qcril_timer_id_mutex" );

        tcbinfo->timer_id = the_timer_id;
        tcbinfo->callback = callback;

        qcril_add_timed_callback(tcbinfo);

        qcril_response_api[ instance_id ]->RequestTimedCallback( qcril_timed_callback_dispatch,
                                                                 (void *)(uintptr_t)the_timer_id, relativeTime );

        QCRIL_LOG_DEBUG( "Set timer with ID %d", the_timer_id );

        if (timer_id)
        {
          *timer_id = the_timer_id;
        }
        ret = 0;
      }
  }

  return ret;
} /* qcril_setup_timed_callback */

//===========================================================================
// qcril_timed_callback_dispatch_expra_params
//===========================================================================
static void qcril_timed_callback_dispatch_expra_params
(
  void *param
)
{
  uint32                                    timer_id = (uint32)(uintptr_t) param;
  qcril_timed_callback_type                 cb;
  qcril_timed_callback_info *               info = qcril_find_and_remove_timed_callback(timer_id);
  qcril_timed_callback_handler_params_type  handler_params;

  if (info)
  {
    memset( &handler_params, 0, sizeof( handler_params ) );
    handler_params.timer_id     = timer_id;
    handler_params.custom_param = info->extra_params;

    cb                          = info->callback;

    cb( &handler_params );

    qcril_free( info );
  }
} // qcril_timed_callback_dispatch_expra_params

//===========================================================================
// qcril_setup_timed_callback_ex_params
//===========================================================================
int qcril_setup_timed_callback_ex_params
(
  qcril_instance_id_e_type      instance_id,
  qcril_modem_id_e_type         modem_id,
  qcril_timed_callback_type     callback,
  void*                         extra_params,
  const struct timeval *        relativeTime,
  uint32 *                      timer_id
)
{
  return qcril_setup_timed_callback_ex_params_adv(instance_id, modem_id, callback, extra_params, FALSE, relativeTime, timer_id);
}

//===========================================================================
// qcril_setup_timed_callback_ex_params_adv
//===========================================================================
int qcril_setup_timed_callback_ex_params_adv
(
  qcril_instance_id_e_type      instance_id,
  qcril_modem_id_e_type         modem_id,
  qcril_timed_callback_type     callback,
  void*                         extra_params,
  boolean                       need_free,
  const struct timeval *        relativeTime,
  uint32 *                      timer_id
)
{
  qcril_timed_callback_info *tcbinfo;
  int ret = -1;
  uint32 the_timer_id;


  if ((instance_id < QCRIL_MAX_INSTANCE_ID) && (modem_id < QCRIL_MAX_MODEM_ID))
  {
      tcbinfo = qcril_malloc(sizeof(qcril_timed_callback_info));
      if (tcbinfo)
      {

        QCRIL_MUTEX_LOCK( &qcril_timer_id_mutex, "qcril_timer_id_mutex" );

        /* Most Significant 16 bits are the Instance ID + Modem ID and Least Significant 16 bits are the QCRIL Timer ID */
        the_timer_id = ( uint32 ) QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, qcril_timer_id );
        qcril_timer_id++;
        if( 0 == qcril_timer_id )
        {
            qcril_timer_id = 1;
        }

        QCRIL_MUTEX_UNLOCK( &qcril_timer_id_mutex, "qcril_timer_id_mutex" );

        tcbinfo->timer_id     = the_timer_id;
        tcbinfo->callback     = (void*)callback;
        tcbinfo->extra_params = extra_params;
        tcbinfo->need_free    = need_free;

        qcril_add_timed_callback(tcbinfo);

        qcril_response_api[ instance_id ]->RequestTimedCallback( qcril_timed_callback_dispatch_expra_params,
                                                                 (void *)(uintptr_t)the_timer_id,
                                                                 relativeTime );

        QCRIL_LOG_DEBUG( "Set timer with ID %d", the_timer_id );

        if (timer_id)
        {
          *timer_id = the_timer_id;
        }
        ret = 0;
      }
  }

  return ret;
} // qcril_setup_timed_callback_ex_params_adv


/*=========================================================================
  FUNCTION:  qcril_cancel_timed_callback

===========================================================================*/
/*!
    @brief
    Cancel RIL callback timer

    @return
    0 on success.
*/
/*=========================================================================*/
int qcril_cancel_timed_callback
(
  void *param
)
{
  uint32 timer_id = (uint32)(uintptr_t) param;
  qcril_timed_callback_info *info = qcril_find_and_remove_timed_callback(timer_id);
  int ret = -1;
  /*-----------------------------------------------------------------------*/

  if (info)
  {
    ret = 0;

    if (info->need_free && NULL != info->extra_params)
    {
      qcril_free(info->extra_params);
    }

    QCRIL_LOG_DEBUG( "Cancel timer with ID %d", info->timer_id );
    qcril_free(info);
  }

  return ret;
} /* qcril_cancel_timed_callback */


/*=========================================================================
  FUNCTION:  qcril_timed_callback_active

===========================================================================*/
/*!
    @brief
    Query state of the timed callback

    @return
    0 if timer is inactive. Non-zero Otherwise
*/
/*=========================================================================*/
int qcril_timed_callback_active
(
  uint32 timer_id
)
{
  /*-----------------------------------------------------------------------*/
  qcril_timed_callback_info **info = NULL;

  QCRIL_ASSERT( info );

  QCRIL_MUTEX_LOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex" );

  info = _qcril_find_timed_callback_locked(timer_id);

  QCRIL_MUTEX_UNLOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex" );

  QCRIL_ASSERT( info != NULL );

  return *info !=NULL;
} /* qcril_timed_callback_active */


/*=========================================================================
  FUNCTION:  qcril_default_request_resp_params

===========================================================================*/
/*!
    @brief
    Set default values for parameters in RIL request's response

    @return
    None
*/
/*=========================================================================*/
void qcril_default_request_resp_params
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request_id,
  RIL_Errno ril_err_no,
  qcril_request_resp_params_type *param_ptr
)
{
    if(instance_id < QCRIL_MAX_INSTANCE_ID && param_ptr != NULL)
    {
        param_ptr->instance_id        = instance_id;
        param_ptr->t                  = t;
        param_ptr->request_id         = request_id;
        param_ptr->android_request_id = request_id;
        param_ptr->ril_err_no         = ril_err_no;
        param_ptr->resp_pkt           = NULL;
        param_ptr->resp_len           = 0;
        param_ptr->logstr             = NULL;
        param_ptr->rild_sock_oem_req  = 0;
    }
    else
    {
        QCRIL_LOG_FATAL("CHECK FAILED");
    }

} /* qcril_default_request_resp_params */

//===========================================================================
// qcril_free_request_and_dispatch_follower_request_cb
//===========================================================================
static void qcril_free_request_and_dispatch_follower_request_cb(qcril_timed_callback_handler_params_type * handler_params)
{
  qcril_free_req_and_dispatch_follower_req_payload_type *payload;
  qcril_request_resp_params_type                         resp_local;
  RIL_Token                                              follower_token;

  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL != handler_params)
  {
    payload = handler_params->custom_param;

    if ( NULL != payload )
    {
      follower_token = qcril_reqlist_get_follower_token( payload->instance_id, payload->t );
      if ( QMI_RIL_ZERO != follower_token && qcril_reqlist_is_auto_respond_duplicate( payload->instance_id, follower_token ) )
      {
        // drop org
        qcril_reqlist_free( payload->instance_id, payload->t );
        // auto respond for duplicate
        resp_local          = *payload->data;
        resp_local.t        = follower_token; // substitute token
        resp_local.logstr   = NULL;
        qcril_send_request_response( &resp_local );
      }
      else
      {
        qcril_reqlist_free_and_dispatch_follower_req(payload->t, payload->instance_id, NULL, QMI_RIL_ZERO );
      }

      if ( NULL != payload->data)
      {
        if (NULL != payload->data->resp_pkt)
        {
          qcril_free(payload->data->resp_pkt);
        }
        qcril_free(payload->data);
      }
      qcril_free(payload);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_free_request_and_dispatch_follower_request_cb

//===========================================================================
// qcril_free_request_list_entry_deferred
//===========================================================================
void qcril_free_request_list_entry_deferred( qcril_timed_callback_handler_params_type * handler_params )
{
  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL != handler_params )
  {
    qcril_reqlist_free( QCRIL_DEFAULT_INSTANCE_ID, (RIL_Token) handler_params->custom_param ); // QCRIL_DEFAULT_INSTANCE_ID as only QMI RIL may be using this extended req list functionality
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_free_request_list_entry_deferred

void qcril_send_empty_payload_request_response(qcril_instance_id_e_type instance_id, RIL_Token t, int request_id, RIL_Errno ril_err_no)
{
  qcril_request_resp_params_type resp_param;

  if ( request_id > QCRIL_EVT_IMS_SOCKET_REQ_BASE && request_id < QCRIL_EVT_IMS_SOCKET_REQ_MAX )
  {
    qcril_qmi_ims_socket_send(t, IMS__MSG_TYPE__RESPONSE, qcril_qmi_ims_map_event_to_request(request_id), qcril_qmi_ims_map_ril_error_to_ims_error(ril_err_no), NULL, 0);
  }
  else
  {
    qcril_default_request_resp_params( instance_id, t, request_id, ril_err_no, &resp_param );
    qcril_send_request_response( &resp_param );
  }
}

//===========================================================================
//qcril_send_request_response
//===========================================================================
void qcril_send_request_response
(
  qcril_request_resp_params_type *param_ptr
)
{
  qcril_instance_id_e_type instance_id;
  char label[ 512 ];
  char *ril_errno_name[] = { "Success",
                             "Radio Not Available",
                             "Generic Failure",
                             "Password Incorrect",
                             "SIM Pin2",
                             "SIM Puk2",
                             "Request Not Supported",
                             "Cancell`ed", "OP Not Allowed During Voice Call",
                             "OP Not Allowed Before Reg To NW",
                             "SMS Send Fail Retry",
                             "SIM Absent",
                             "Subscription Not Available",
                             "Mode Not Supported",
                             "FDN Check Failure",
                             "Illegal SIM or ME",
                             "Setup Data Call failure",
                             "Dial modified to USSD",
                             "Dial modified to SS",
                             "Dial modified to Dial",
                             "USSD modified to Dial",
                             "USSD modified to SS",
                             "USSD modified to USSD",
                             "SS modified to Dial",
                             "SS modified to USSD",
                             "SS modified to SS",
                             "Subscription not supported",
                             "Missing resource",
                             "No such element",
    };

    qmi_ril_oem_hook_response_context_type * oem_hook_response_ctx = NULL;
    qmi_ril_oem_hook_response_context_type * prev_iter = NULL;
    int                                      is_qmi_hook_idl_tunneling_response;
    RIL_Token                                token_param;
    char*                                    substituted_data = NULL;
    char*                                    orig_substituted_data = NULL;
    uint32_t                                 substituted_data_len = 0;
    qmi_idl_service_object_type              qmi_idl_tunneling_service_object;
    qmi_client_error_type                    idl_err = QMI_NO_ERR;
    int                                      successfully_substituted;
    void *                                   actual_resp_pkt;
    size_t                                   actual_resp_len;
    uint32_t                                 encoded_fact = 0;

    uint32_t *                               int32_param;
    uint16_t *                               int16_param;
    uint32                                   log_request_id;
    char*                                    log_evt_name;
    boolean                                  is_oem_response = FALSE;
    int                                      android_request_id_for_response;


    if (param_ptr != NULL && (param_ptr->instance_id < QCRIL_MAX_INSTANCE_ID) )
    {
          actual_resp_pkt = param_ptr->resp_pkt;
          actual_resp_len = param_ptr->resp_len;

          substituted_data = NULL;

          switch ( param_ptr->request_id )
          {
            case QCRIL_EVT_HOOK_NV_READ:                    // fall through
            case QCRIL_EVT_HOOK_NV_WRITE:                   // fall through
            case QCRIL_EVT_HOOK_DATA_GO_DORMANT:            // fall through
            case QCRIL_EVT_HOOK_ME_DEPERSONALIZATION:       // fall through
            case QCRIL_EVT_HOOK_REQ_GENERIC:
            case QCRIL_EVT_HOOK_CSG_PERFORM_NW_SCAN:
            case QCRIL_EVT_HOOK_CSG_SET_SYS_SEL_PREF:
            case QCRIL_EVT_HOOK_CSG_GET_SYS_INFO:
            case QCRIL_EVT_HOOK_UICC_VOLTAGE_STATUS_REQ:
            case QCRIL_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ:
            case QCRIL_EVT_HOOK_PERSONALIZATION_STATUS_REQ:
            // DSDS/DSDA/TSTS requests
            case QCRIL_EVT_HOOK_SET_TUNE_AWAY:
            case QCRIL_EVT_HOOK_GET_TUNE_AWAY:
            case QCRIL_EVT_HOOK_SET_PAGING_PRIORITY:
            case QCRIL_EVT_HOOK_GET_PAGING_PRIORITY:
            case QCRIL_EVT_HOOK_SET_DEFAULT_VOICE_SUB:
            case QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD:
            case QCRIL_EVT_HOOK_GET_MODEM_CAPABILITY:
            case QCRIL_EVT_HOOK_UPDATE_SUB_BINDING:
            case QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY:
                android_request_id_for_response = RIL_REQUEST_OEM_HOOK_RAW;
              break;
            default:
              android_request_id_for_response = param_ptr->request_id;
              break;
          }

          if ( ( ( param_ptr->request_id > QCRIL_EVT_HOOK_BASE ) && ( param_ptr->request_id < QCRIL_EVT_HOOK_MAX ) ) ||
               ( ( param_ptr->request_id > QCRIL_EVT_OEM_BASE ) && ( param_ptr->request_id < QCRIL_EVT_OEM_MAX ) ) ||
               ( param_ptr->android_request_id == RIL_REQUEST_OEM_HOOK_RAW ) )
          {
            is_oem_response = TRUE;
          }

          // oem hook qmi idl tunneling
          token_param                        = param_ptr->t;
          is_qmi_hook_idl_tunneling_response = FALSE;
          oem_hook_response_ctx              = NULL;
          prev_iter                          = NULL;

          pthread_mutex_lock( &qmi_ril_oem_hook_overview.overview_lock_mutex );
          oem_hook_response_ctx = qmi_ril_oem_hook_overview.qmi_ril_oem_hook_qmi_tunneling_reqs_root;

          while ( NULL != oem_hook_response_ctx && !is_qmi_hook_idl_tunneling_response )
          {
            QCRIL_LOG_DEBUG("buffer token = %d, received token = %d", oem_hook_response_ctx->original_token, token_param);
            if ( token_param == oem_hook_response_ctx->original_token )
            {  // match
              is_qmi_hook_idl_tunneling_response = TRUE;
              if ( NULL != prev_iter )
              {
                prev_iter->next = oem_hook_response_ctx->next;
              }
              else
              { // top
                qmi_ril_oem_hook_overview.qmi_ril_oem_hook_qmi_tunneling_reqs_root = oem_hook_response_ctx->next;
              }
              break;
            }
            else
            {
              prev_iter = oem_hook_response_ctx;
              oem_hook_response_ctx = oem_hook_response_ctx->next;
            }
          }

          pthread_mutex_unlock( &qmi_ril_oem_hook_overview.overview_lock_mutex );

          if ( is_qmi_hook_idl_tunneling_response )
          {
            QCRIL_LOG_DEBUG("qmi_idl_tunneling: responding to req_id = %d, srv_id = %d, msg_id = %d",
                                              oem_hook_response_ctx->ril_request_id,
                                              oem_hook_response_ctx->ril_idl_service_id,
                                              oem_hook_response_ctx->ril_idl_message_id );

            log_evt_name      = NULL;
            log_request_id    = QMI_RIL_ZERO;
            qmi_ril_oem_hook_get_request_id( oem_hook_response_ctx->ril_idl_service_id, oem_hook_response_ctx->ril_idl_message_id, &log_request_id, &log_evt_name );
            if ( NULL != log_evt_name )
            {
              QCRIL_LOG_DEBUG(".. responding to request %s", log_evt_name);
            }
            else
            {
              QCRIL_LOG_DEBUG(".. responding to request with unknown name");
            }

            do
            {
                successfully_substituted = FALSE;
                qmi_idl_tunneling_service_object = qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object( oem_hook_response_ctx->ril_idl_service_id );

                if ( NULL != qmi_idl_tunneling_service_object )
                {
                  idl_err = qmi_idl_get_max_message_len( qmi_idl_tunneling_service_object, QMI_IDL_RESPONSE, oem_hook_response_ctx->ril_idl_message_id, &substituted_data_len  );

                  if ( QMI_NO_ERR == idl_err )
                  {
                    substituted_data = qcril_malloc( substituted_data_len + OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE );

                    /* keep copy of orginal substituted data for freeing it properly */
                    orig_substituted_data = substituted_data;

                    if ( NULL != substituted_data )
                    {
                      encoded_fact = QMI_RIL_ZERO;

                      if ( param_ptr->resp_len > 0 )
                      {
                         idl_err = qmi_idl_message_encode( qmi_idl_tunneling_service_object,
                                                 QMI_IDL_RESPONSE,
                                                 oem_hook_response_ctx->ril_idl_message_id,
                                                 param_ptr->resp_pkt,
                                                 param_ptr->resp_len,
                                                 substituted_data + OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE,
                                                 substituted_data_len,
                                                 &encoded_fact );

                         if( idl_err == QMI_NO_ERR )
                         {
                            /* for VT service, skip the result part, as RIL<-->Telphony interface does not expect result field */
                            if( ( oem_hook_response_ctx->ril_idl_service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_VT ) ||
                                ( oem_hook_response_ctx->ril_idl_service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE ) )
                            {
                               substituted_data = substituted_data + OEM_HOOK_QMI_TUNNELING_RESULT_SIZE;
                               encoded_fact = encoded_fact - OEM_HOOK_QMI_TUNNELING_RESULT_SIZE;
                            }

                            QCRIL_LOG_DEBUG("TLV response message size = %d", encoded_fact);
                            qcril_qmi_print_hex(  (unsigned char*)substituted_data + OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE, encoded_fact);
                         }
                         else
                         {
                            QCRIL_LOG_ERROR( "qcril_send_request_response() QMI IDL - decode failed to decode buf with err %d, factual len %d ", (int) idl_err, (int) encoded_fact   );
                            break;
                         }
                      }
                      else
                      {
                         QCRIL_LOG_ERROR( "qcril_send_request_response() QMI IDL - skip decode due to error = %d in response or zero length =%d ", (int) param_ptr->ril_err_no, (int) param_ptr->resp_len   );
                         break;
                      }
                    }
                    else
                    {
                       QCRIL_LOG_ERROR( "qcril_send_request_response() QMI IDL - response decode failed to allocate substitute buf len %d", (int) substituted_data_len   );
                       break;
                    }
                  }
                  else
                  {
                    QCRIL_LOG_ERROR( "qcril_send_request_response() QMI IDL - response decode could not get length for message id %d, err code %d", (int) oem_hook_response_ctx->ril_idl_message_id, (int) idl_err   );
                    break;
                  }
                }
                else
                {
                  QCRIL_LOG_ERROR( "qcril_send_request_response() QMI IDL - response decode not found service object for service id error %d", (int) oem_hook_response_ctx->ril_idl_service_id );
                  break;
                }

                // fill up the tunneling header
                // request id
                if ( NULL != substituted_data )
                {
                  int32_param = (uint32_t*)substituted_data;
                  *int32_param = QCRIL_EVT_HOOK_REQ_GENERIC;


                  // response size
                  int32_param = (uint32_t*) (substituted_data + OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE);
                  *int32_param = encoded_fact + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE +
                                                OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE +
                                                OEM_HOOK_QMI_TUNNELING_ERROR_CODE_SIZE;

                  // service id
                  int16_param = (uint16_t*) ( substituted_data +
                                              OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE +
                                              OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE );
                  *int16_param = oem_hook_response_ctx->ril_idl_service_id;

                  // message id
                  int16_param = (uint16_t*) ( substituted_data +
                                              OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE +
                                              OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE +
                                              OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE );

                  *int16_param = oem_hook_response_ctx->ril_idl_message_id;

                  //error code
                  int16_param = (uint16_t*) ( substituted_data + OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE +
                                                                                       OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE +
                                                                                       OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE +
                                                                                       OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE );
                }

                /* If there was an internal error send internal error, else send the error cause received in the response request */
                if( idl_err == QMI_NO_ERR )
                {
                  // fill up the tunneling header
                  // request id
                  if ( NULL != substituted_data )
                  {
                    int32_param = (uint32_t*)substituted_data;
                    *int32_param = QCRIL_EVT_HOOK_REQ_GENERIC;


                    // response size
                    int32_param = (uint32_t*) (substituted_data + OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE);
                    *int32_param = encoded_fact + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE +
                                                  OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE +
                                                  OEM_HOOK_QMI_TUNNELING_ERROR_CODE_SIZE;

                    // service id
                    int16_param = (uint16_t*) ( substituted_data +
                                                OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE +
                                                OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE );
                    *int16_param = oem_hook_response_ctx->ril_idl_service_id;

                    // message id
                    int16_param = (uint16_t*) ( substituted_data +
                                                OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE +
                                                OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE +
                                                OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE );

                    *int16_param = oem_hook_response_ctx->ril_idl_message_id;

                    //error code
                    int16_param = (uint16_t*) ( substituted_data + OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE +
                                                                                         OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE +
                                                                                         OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE +
                                                                                         OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE );

                    /* If there was an internal error send internal error, else send the error cause received in the response request */
                    if( idl_err == QMI_NO_ERR )
                    {
                      *int16_param = param_ptr->ril_err_no;
                    }
                    else
                    {
                      *int16_param = idl_err;
                    }

                    // for oem hook tunneling requests, always send success, error code will actually indicate success or faliure
                    if( param_ptr->ril_err_no != RIL_E_SUCCESS )
                    {
                       QCRIL_LOG_DEBUG("error_cause received from qmi = %d", param_ptr->ril_err_no);
                       param_ptr->ril_err_no = RIL_E_SUCCESS;
                    }
                  }

                  // finally
                  successfully_substituted = TRUE;
                }

            } while ( FALSE );

            if ( successfully_substituted  )
            {
              actual_resp_pkt = substituted_data;
              QCRIL_LOG_DEBUG("allocated memory size = %d, final msg size = %d", substituted_data_len, encoded_fact + OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE);

              actual_resp_len = encoded_fact + OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE;
              QCRIL_LOG_DEBUG( "qcril_send_request_response() QMI IDL - substitted payload with qmi encoded stream len %d", (int) actual_resp_len   );

              qcril_qmi_print_hex( actual_resp_pkt, actual_resp_len);
            }
            else
            {
              is_qmi_hook_idl_tunneling_response = FALSE;
            }
          }


          // do respond
          instance_id = param_ptr->instance_id;

          if (!qmi_ril_is_multi_sim_oem_hook_request(param_ptr->request_id) &&
              qmi_ril_is_feature_supported(QMI_RIL_FEATURE_OEM_SOCKET) &&
              is_oem_response && !param_ptr->rild_sock_oem_req)
          {
            qcril_qmi_oem_socket_send(instance_id,
                                      param_ptr->t,
                                      param_ptr->request_id,
                                      param_ptr->ril_err_no,
                                      actual_resp_pkt,
                                      actual_resp_len);
          }
          else
          {
            uint16_t err_name_idx = param_ptr->ril_err_no;

#if defined(RIL_REQUEST_SIM_OPEN_CHANNEL) || defined(RIL_REQUEST_SIM_CLOSE_CHANNEL) || \
    defined(RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC) || defined(RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL)
            switch ( param_ptr->ril_err_no )
            {
               case RIL_E_MISSING_RESOURCE:
                 err_name_idx = RIL_E_SUBSCRIPTION_NOT_SUPPORTED + 1;
                 break;
               case RIL_E_NO_SUCH_ELEMENT:
                 err_name_idx = RIL_E_SUBSCRIPTION_NOT_SUPPORTED + 2;
                 break;
               default:
                 err_name_idx = param_ptr->ril_err_no;
                 break;
            }
#endif /* RIL_REQUEST_SIM_OPEN_CHANNEL || RIL_REQUEST_SIM_CLOSE_CHANNEL ||
          RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC || RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL */

            // Log the event packet for the response to RIL request
            if ( param_ptr->logstr != NULL )
            {
              QCRIL_SNPRINTF( label, sizeof( label ), "%s - %s, RID %d, Token id %d, %s",
                              qcril_log_lookup_event_name( param_ptr->request_id ),
                              param_ptr->logstr, param_ptr->instance_id,
                              qcril_log_get_token_id( param_ptr->t ),
                              ril_errno_name[ err_name_idx ] );
            }
            else
            {
              QCRIL_SNPRINTF( label, sizeof( label ), "%s - RID %d, Token id %d, %s",
                              qcril_log_lookup_event_name( param_ptr->request_id ),
                              param_ptr->instance_id,
                              qcril_log_get_token_id( param_ptr->t ),
                              ril_errno_name[ err_name_idx ] );
            }

            QCRIL_LOG_CF_PKT_RIL_RES( instance_id, label );

            // Send response to the RIL request
            QCRIL_LOG_DEBUG( "UI <--- %s (%d) Complete --- RIL [RID %d, Token id %d, %s, Len %d %s]\n",
                           qcril_log_lookup_event_name( param_ptr->request_id ),
                           param_ptr->request_id,
                           param_ptr->instance_id,
                           qcril_log_get_token_id( param_ptr->t ),
                           ril_errno_name[ err_name_idx ],
                           actual_resp_len,
                           ( param_ptr->logstr == NULL )? "" : param_ptr->logstr );

            qmi_ril_fw_send_request_response_epilog(
                                                    instance_id,
                                                    param_ptr->t,
                                                    android_request_id_for_response,
                                                    param_ptr->ril_err_no,
                                                    actual_resp_pkt,
                                                    actual_resp_len,
                                                    FALSE,
                                                    param_ptr
                                                    );
          }
          if ( orig_substituted_data )
          {
            qcril_free( orig_substituted_data );
          }

          if ( oem_hook_response_ctx )
          {
            qcril_free( oem_hook_response_ctx );
          }
    }
    else
    {
      QCRIL_LOG_FATAL("%s","FATAL : CHECK FAILED");
    }

} // qcril_send_request_response

//===========================================================================
// qmi_ril_fw_send_request_response_epilog
//===========================================================================
void qmi_ril_fw_send_request_response_epilog( qcril_instance_id_e_type instance_id,
                                              RIL_Token token,
                                              int android_request_id,
                                              RIL_Errno resp_cause,
                                              void* resp_data,
                                              uint32 resp_data_len,
                                              int is_abnormal_drop,
                                              qcril_request_resp_params_type *param_ptr_ref )
{
  RIL_Token following_token;
  RIL_Token prev_token;
  int       go_on;

  qcril_free_req_and_dispatch_follower_req_payload_type *payload;

  qmi_ril_fw_android_request_kind_execution_overview_type*
                                            android_request_kind_execution_overview;
  uint32_t                                  android_request_handling_caps;
  qmi_ril_fw_android_request_holder_type*   android_request_param_holder;
  qmi_ril_fw_android_request_holder_type*   android_request_prm_hldr_iter;
  uint32_t                                  cap_max_queue_sz;
  uint32_t                                  cap_queue_sz_iter;
  void*                                     android_request_param_clone;
  int                                       do_flow_control_follow_up;
  int                                       need_trigger_address_remains;
  int                                       regarded_same;
  uint32_t                                  payload_sz_cur;
  int                                       need_protection;
  int                                       any_oustanding_requests;
  int                                       final_android_request_id;
  int                                       iter_android_request_id;
  int                                       ok_compensated_req_id;

  QCRIL_LOG_FUNC_ENTRY();

  final_android_request_id = android_request_id;

  if ( qcril_reqlist_has_follower( instance_id, token ) && !is_abnormal_drop && NULL != param_ptr_ref )
  {
    if ( !is_abnormal_drop && NULL != param_ptr_ref  )
    {
      payload = qcril_malloc(sizeof(*payload));
      if ( NULL != payload )
      {
        go_on = FALSE;

        payload->t = token;
        payload->instance_id = instance_id;
        payload->data = qcril_malloc(sizeof(*param_ptr_ref)); // malloc for qcril_request_resp_params_type
        if ( NULL != payload->data )
        {
          *payload->data = *param_ptr_ref;

          go_on = FALSE;
          if (QMI_RIL_ZERO != resp_data_len )
          {
            payload->data->resp_pkt = qcril_malloc( resp_data_len );
            if ( NULL != payload->data->resp_pkt )
            {
              memcpy(payload->data->resp_pkt, resp_data, resp_data_len);
              go_on = TRUE;
            }
          }
          else
          {
            payload->data->resp_pkt = NULL;
            go_on = TRUE;
          }

          if ( go_on )
          {
            qcril_setup_timed_callback_ex_params( QCRIL_DEFAULT_INSTANCE_ID,
                                                  QCRIL_DEFAULT_MODEM_ID,
                                                  qcril_free_request_and_dispatch_follower_request_cb,
                                                  payload,
                                                  NULL,   // immediate
                                                  NULL );
          }
        }
        if ( NULL != payload && !go_on )
        {
          if ( NULL != payload->data )
          {
            qcril_free( payload->data );
          }
          qcril_free( payload );
          payload = NULL;
        }
      }
    }
    else
    { // follow line
      prev_token = token;
      do
      {
        following_token = qcril_reqlist_get_follower_token( instance_id, prev_token );
        ( void ) qcril_reqlist_free( instance_id , prev_token );
        prev_token = following_token;
      } while ( QMI_RIL_ZERO != prev_token );
    }
  }
  else if (qcril_reqlist_under_follower_handler_exec( instance_id, token )  )
  {
    qcril_setup_timed_callback_ex_params( QCRIL_DEFAULT_INSTANCE_ID,
                                          QCRIL_DEFAULT_MODEM_ID,
                                          qcril_free_request_list_entry_deferred,
                                          (void*) token,
                                          NULL,   // immediate
                                          NULL );
  }
  else
  { // Remove entry from Reqlist if applicable
    ( void ) qcril_reqlist_free( instance_id , token );
  }

  // flow control s
  QCRIL_LOG_DEBUG("main trd %x, cur trd %x", qmi_ril_fw_get_main_thread_id(), pthread_self() );
  QCRIL_LOG_DEBUG("eq %d, under mn thrd %d", pthread_equal( qmi_ril_fw_get_main_thread_id(), pthread_self()) , qmi_ril_fw_android_request_flow_control_overview.in_exec_on_main_thread );
  need_protection = !( ( pthread_equal( qmi_ril_fw_get_main_thread_id(), pthread_self() ) ) && qmi_ril_fw_android_request_flow_control_overview.in_exec_on_main_thread );
  QCRIL_LOG_DEBUG("need protection %d ", need_protection );

  if ( need_protection )
  {
    qmi_ril_fw_android_request_flow_control_info_lock();
  }

  QCRIL_LOG_DEBUG( "in protected zone" );

  do_flow_control_follow_up = FALSE;
  need_trigger_address_remains = FALSE;
  do
  {
    if ( android_request_id <= QMI_RIL_ZERO || android_request_id > QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID )
    { // attempt restoring original android event id from token if event id got altered
      ok_compensated_req_id = FALSE;
      android_request_kind_execution_overview = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ QMI_RIL_ZERO ];
      for ( iter_android_request_id = QMI_RIL_ZERO; iter_android_request_id <= QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID && !ok_compensated_req_id; iter_android_request_id++  )
      {
        if ( token == android_request_kind_execution_overview->token_under_execution )
        {
          final_android_request_id = iter_android_request_id;
          ok_compensated_req_id    = TRUE;
          QCRIL_LOG_DEBUG( "restrored org. a-r-id %d", final_android_request_id );
        }
        else
        {
          android_request_kind_execution_overview++;
        }
      }

      if ( !ok_compensated_req_id )
        break; // not expected
    }
    else
    {
      final_android_request_id = android_request_id;
    }

    android_request_kind_execution_overview = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ final_android_request_id ];

    android_request_handling_caps = android_request_kind_execution_overview->nof_extra_holders_and_caps_and_dynamics;
    cap_max_queue_sz = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ( android_request_handling_caps );

    if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FLOW_CONTROL_EXEMPT )
      break; // no flow control action

    QCRIL_LOG_DEBUG( "token under exec %d, completion token id %d", qcril_log_get_token_id( android_request_kind_execution_overview->token_under_execution) , qcril_log_get_token_id (token) ) ;
    if ( android_request_kind_execution_overview->token_under_execution != token )
      break; // not mainstream execution, handler was not invoked

    android_request_kind_execution_overview->token_under_execution = QMI_RIL_ZERO;

    // locate bookkeeping ref
    android_request_param_holder = NULL;
    if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY )
    {
      android_request_param_holder = &android_request_kind_execution_overview->holders.local_holder;
    }
    else if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE )
    {
      android_request_prm_hldr_iter = android_request_kind_execution_overview->holders.extra_holders;
      for ( cap_queue_sz_iter = QMI_RIL_ZERO; cap_queue_sz_iter < cap_max_queue_sz && NULL == android_request_param_holder; cap_queue_sz_iter++ )
      {
        if ( !(android_request_prm_hldr_iter->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE) && android_request_prm_hldr_iter->token == token )
        {
          android_request_param_holder = android_request_prm_hldr_iter;
        }
        else
        {
          android_request_prm_hldr_iter++;
        }
      }
    }
    else // invalid case
      break;

    if ( NULL == android_request_param_holder )
      break;


    do_flow_control_follow_up = TRUE;
  } while (FALSE);

  // do post
  QCRIL_LOG_DEBUG("resp token-id %d, req-id %d(%d)",
                   qcril_log_get_token_id( token ), final_android_request_id, android_request_id );

  /* Print RIL Message */
  qcril_log_print_ril_message(final_android_request_id, RIL__MSG_TYPE__RESPONSE, resp_data,
                              resp_data_len, resp_cause);

  qcril_response_api[ instance_id ]->OnRequestComplete( token,
                                                        resp_cause,
                                                        resp_data,
                                                        resp_data_len );

  if ( do_flow_control_follow_up )
  {
    payload_sz_cur = QMI_RIL_FW_ANDROID_REQUEST_INFO_DECOMPOSE_SZ( android_request_param_holder->param_info_and_len );

    if (  (android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE) &&
         !(android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE) )
    { // auto response check
      android_request_prm_hldr_iter = android_request_kind_execution_overview->holders.extra_holders;
      for ( cap_queue_sz_iter = QMI_RIL_ZERO; cap_queue_sz_iter < cap_max_queue_sz ; cap_queue_sz_iter++  )
      {
        regarded_same = FALSE;
        if ( android_request_prm_hldr_iter->token != token )
        { // not self
          regarded_same = qmi_ril_fw_android_request_flow_control_request_holders_have_same_param( android_request_param_holder, android_request_prm_hldr_iter );
        }

        if ( regarded_same )
        { // do auto response
          QCRIL_LOG_DEBUG("resp token-id %d, req-id %d",
                          qcril_log_get_token_id(android_request_prm_hldr_iter->token),
                          final_android_request_id);

          qcril_response_api[ instance_id ]->OnRequestComplete( android_request_prm_hldr_iter->token,
                                                                resp_cause,
                                                                resp_data,
                                                                resp_data_len );

          // cleanup
          qmi_ril_fw_android_request_flow_control_release_holder_info_bookref( android_request_prm_hldr_iter, final_android_request_id );
        }
        android_request_prm_hldr_iter++;
      }
    }

    // cleanup for just finsihed execution
    qmi_ril_fw_android_request_flow_control_release_holder_info_bookref( android_request_param_holder, final_android_request_id );

    qmi_ril_fw_android_request_flow_control_overview_request_review_holders( android_request_kind_execution_overview );

    // outstanding left for execution?
    any_oustanding_requests = qmi_ril_fw_android_request_flow_control_find_request_for_execution( android_request_kind_execution_overview, NULL, NULL );
    if ( any_oustanding_requests )
    {
      need_trigger_address_remains = TRUE;
    }
  }
  if ( need_protection )
  {
    qmi_ril_fw_android_request_flow_control_info_unlock();
  }
  QCRIL_LOG_DEBUG( "left zone " );

  if ( need_trigger_address_remains )
  {
    qcril_setup_timed_callback_ex_params( QCRIL_DEFAULT_INSTANCE_ID,
                                          QCRIL_DEFAULT_MODEM_ID,
                                          qmi_ril_fw_android_request_flow_control_trigger_remains,
                                          (void *)(intptr_t)final_android_request_id,
                                          NULL,   // immediate
                                          NULL );
  }
  // flow control e

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_fw_send_request_response_epilog

/*=========================================================================
  FUNCTION:  qcril_default_unsol_resp_params

===========================================================================*/
/*!
    @brief
    Set default values for unsolicted response parameters.

    @return
    None
*/
/*=========================================================================*/
void qcril_default_unsol_resp_params
(
  qcril_instance_id_e_type instance_id,
  int response_id,
  qcril_unsol_resp_params_type *param_ptr
)
{
  if(instance_id < QCRIL_MAX_INSTANCE_ID && param_ptr != NULL)
  {
      param_ptr->instance_id = instance_id;
      param_ptr->response_id = response_id;
      param_ptr->resp_pkt = NULL;
      param_ptr->resp_len = 0;
      param_ptr->logstr = NULL;
  }
  else
  {
    QCRIL_LOG_FATAL("CHECK FAILED");
  }

} /* qcril_default_unsol_resp_params */

/*=========================================================================
  FUNCTION:  qcril_send_unsol_response_epilog
===========================================================================*/
static void qcril_send_unsol_response_epilog(qcril_unsol_resp_params_type *param_ptr)
{
  QCRIL_LOG_FUNC_ENTRY();
  qcril_instance_id_e_type instance_id = param_ptr->instance_id;
  char label[ 512 ];

  /* Log event packet for Unsolicited response */
  if ( param_ptr->logstr != NULL)
  {
    QCRIL_SNPRINTF( label, sizeof( label ), "%s, %s", qcril_log_lookup_event_name( param_ptr->response_id ), param_ptr->logstr );
  }
  else
  {
    QCRIL_SNPRINTF( label, sizeof( label ), "%s", qcril_log_lookup_event_name( param_ptr->response_id ) );
  }

  QCRIL_LOG_CF_PKT_RIL_UNSOL_RES( instance_id, label );

  /* Send Unsolicted RIL response */
  QCRIL_LOG_DEBUG( "UI <--- %s (%d) --- RIL [RID %d, Len %d, %s]",
                   qcril_log_lookup_event_name( param_ptr->response_id ), param_ptr->response_id,
                   param_ptr->instance_id, param_ptr->resp_len, param_ptr->logstr );


  /* Print RIL Message */
  qcril_log_print_ril_message(param_ptr->response_id, RIL__MSG_TYPE__UNSOL_RESPONSE,
                               param_ptr->resp_pkt, param_ptr->resp_len, RIL_E_SUCCESS);

  if ( param_ptr->instance_id < QCRIL_MAX_INSTANCE_ID )
  {
    qcril_response_api[ param_ptr->instance_id ]->OnUnsolicitedResponse( param_ptr->response_id, param_ptr->resp_pkt,
                                                                       param_ptr->resp_len );
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_send_unsol_response_epilog */

/*=========================================================================
  FUNCTION:  qcril_send_unsol_response

===========================================================================*/
/*!
    @brief
    Send RIL_onUnsolicitedResponse.

    @return
    None
*/
/*=========================================================================*/
void qcril_send_unsol_response
(
  qcril_unsol_resp_params_type *param_ptr
)
{
  boolean dispachable;
  qmi_ril_gen_operational_status_type cur_status;

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    if ( param_ptr == NULL || param_ptr->instance_id >= QCRIL_MAX_INSTANCE_ID )
    {
      QCRIL_LOG_FATAL("invalid param");
      break;
    }

    qmi_ril_android_pending_unsol_resp_lock();

    // check dispatchable, if not queue the resp
    dispachable = qmi_ril_check_android_unsol_resp_dispatchable(param_ptr->response_id);
    if ( !dispachable )
    {
      qmi_ril_add_unsol_resp_to_pending_list(param_ptr);
    }
    else
    {
       // handling RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED differently
       if ( RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED!= param_ptr->response_id)
       {
          cur_status = qmi_ril_get_operational_status();
          if ( cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_ONGOING ||
                cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED ||
                cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING )
          {
            qcril_send_unsol_response_epilog(param_ptr);
          }
          else
          {
            QCRIL_LOG_INFO("Invalid state (%d), Blocking unsol resp %d", cur_status, param_ptr->response_id);
          }
       }
    }

    qmi_ril_android_pending_unsol_resp_unlock();

    // send RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED after unsol_resp_unlock
    if ( dispachable && (RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED== param_ptr->response_id) )
    {
       qcril_send_unsol_response_epilog(param_ptr);
    }

  } while (FALSE);

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_send_unsol_response */

//===========================================================================
//qcril_hook_unsol_response
//===========================================================================
void qcril_hook_unsol_response
(
  qcril_instance_id_e_type instance_id,
  uint32  unsol_event,
  char *  data,
  uint32  data_len
)
{
  char *payload = NULL;
  uint32 index = 0;
  qcril_unsol_resp_params_type unsol_resp;

  int                                       is_qmi_idl_tunelling;
  uint16                                    message_id;
  uint16                                    service_id;
  uint32_t                                  tlv_stream_len;
  qmi_idl_service_object_type               qmi_idl_tunneling_service_object;
  qmi_client_error_type                     idl_err;
  uint32_t                                  encoded_fact;
  uint32_t *                                int32_param;
  uint16_t *                                int16_param;

  payload = NULL;
  do
  {
    service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_NONE;
    message_id = QMI_RIL_ZERO;

    switch ( unsol_event )
    {
      // * VT section
      case QCRIL_EVT_HOOK_VT_UNSOL_CALL_STATUS_IND:
        is_qmi_idl_tunelling = TRUE;
        service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_VT;
        message_id  = IMS_VT_CALL_STATUS_IND_V01;
        break;

      // * eMBMS section
      case QCRIL_EVT_HOOK_EMBMS_UNSOL_RSSI_IND:
        is_qmi_idl_tunelling = TRUE;
        service_id  = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id  = QMI_EMBMS_UNSOL_RSSI_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_SVC_STATE:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_UNSOL_EMBMS_SERVICE_STATE_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_ACTIVE_TMGI:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_ACTIVE_TMGI_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_COVERAGE:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_UNSOL_BROADCAST_COVERAGE_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_OSS_WARNING:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_OOS_WARNING_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_AVAILABLE_TMGI:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_AVAILABLE_TMGI_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_CELL_INFO_CHANGED:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_CELL_INFO_CHANGED_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_RADIO_STATE_CHANGED:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_RADIO_STATE_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_SAI_LIST:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_SAI_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_SIB16_COVERAGE:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_UNSOL_SIB16_COVERAGE_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_E911_STATE_CHANGED:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_E911_STATE_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_CONTENT_DESC_CONTROL:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_UNSOL_CONTENT_DESC_UPDATE_PER_OBJ_IND_V01;
        break;

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_EMBMS_STATUS:
        is_qmi_idl_tunelling = TRUE;
        service_id           = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
        message_id           = QMI_EMBMS_UNSOL_EMBMS_STATUS_IND_V01;
        break;

      //presence
      case QCRIL_EVT_HOOK_IMS_PUBLISH_TRIGGER_IND_V01:
        is_qmi_idl_tunelling = TRUE;
        service_id  = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE;
        message_id  = QMI_IMSP_PUBLISH_TRIGGER_IND_V01;
        break;
      case QCRIL_EVT_HOOK_IMS_NOTIFY_XML_IND_V01:
        is_qmi_idl_tunelling = TRUE;
        service_id  = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE;
        message_id  = QMI_IMSP_NOTIFY_XML_IND_V01;
        break;

      case QCRIL_EVT_HOOK_IMS_NOTIFY_IND_V01:
        is_qmi_idl_tunelling = TRUE;
        service_id  = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE;
        message_id  = QMI_IMSP_NOTIFY_IND_V01;
        break;

      case QCRIL_EVT_HOOK_IMS_ENABLER_STATUS_IND:
        is_qmi_idl_tunelling = TRUE;
        service_id  = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE;
        message_id  = QMI_IMSP_ENABLER_STATE_IND_V01;
        break;

      default:
        is_qmi_idl_tunelling = FALSE;
        break;
    }


    if ( is_qmi_idl_tunelling )
    {
      qmi_idl_tunneling_service_object = qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object( service_id );

      if ( NULL != qmi_idl_tunneling_service_object )
      {
        idl_err = qmi_idl_get_max_message_len( qmi_idl_tunneling_service_object, QMI_IDL_INDICATION, message_id, &tlv_stream_len  );

        if ( QMI_NO_ERR == idl_err )
        {
          payload = (char *) qcril_malloc( tlv_stream_len + OEM_HOOK_QMI_TUNNELING_IND_OVERHEAD_SIZE );

          if ( NULL != payload )
          {
            QCRIL_LOG_DEBUG("max length = %d, rcvd struc len = %d, msg_id = %d", tlv_stream_len, data_len, message_id );

            encoded_fact = QMI_RIL_ZERO;
            idl_err = qmi_idl_message_encode( qmi_idl_tunneling_service_object,
                                    QMI_IDL_INDICATION,
                                    message_id,
                                    data,
                                    data_len,
                                    payload + OEM_HOOK_QMI_TUNNELING_IND_OVERHEAD_SIZE,
                                    tlv_stream_len,
                                    &encoded_fact );

            if ( QMI_NO_ERR == idl_err )
            {
              // complete the oem hook tunneling header

              // signature
              memcpy( payload, QCRIL_HOOK_OEM_NAME, OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE );

              // event id
              int32_param = (uint32_t*) ( payload + OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE );
              *int32_param = QCRIL_EVT_HOOK_UNSOL_GENERIC;

              // payload length
              int32_param = (uint32_t*) ( payload + OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE + OEM_HOOK_QMI_TUNNELING_IND_EVENT_ID_SIZE );
              *int32_param = encoded_fact + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE + OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE;

              // service id
              int16_param = (uint16_t*) ( payload + OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE + OEM_HOOK_QMI_TUNNELING_IND_EVENT_ID_SIZE + OEM_HOOK_QMI_TUNNELING_IND_PLD_LEN_SIZE );
              *int16_param = service_id;

              // message id
              int16_param = (uint16_t*) ( payload + OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE + OEM_HOOK_QMI_TUNNELING_IND_EVENT_ID_SIZE +
                                         OEM_HOOK_QMI_TUNNELING_IND_PLD_LEN_SIZE + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE );
              *int16_param = message_id;

              // finally
              qcril_default_unsol_resp_params( instance_id,
                                               (int) RIL_UNSOL_OEM_HOOK_RAW,
                                               &unsol_resp );

              unsol_resp.resp_pkt                   = ( void * ) payload;
              unsol_resp.resp_len                   = encoded_fact + OEM_HOOK_QMI_TUNNELING_IND_OVERHEAD_SIZE;
              qcril_qmi_print_hex( unsol_resp.resp_pkt , unsol_resp.resp_len);

              if (!qmi_ril_is_multi_sim_oem_hook_request(unsol_event) &&
                   qmi_ril_is_feature_supported(QMI_RIL_FEATURE_OEM_SOCKET))
              {
                qcril_qmi_oem_socket_send_unsol(unsol_resp.resp_pkt,
                                                unsol_resp.resp_len);
              }
              else
              {
                qcril_send_unsol_response( &unsol_resp );
              }
            }
            else
            {
              QCRIL_LOG_ERROR( "QMI IDL - failed to compose tlv stream err %d, actually encoded len %d ", (int) idl_err, (int) encoded_fact  );
              break;
            }

          }
          else
          {
            QCRIL_LOG_ERROR( "QMI IDL - failed to allocate payload tlv stream buf, size %d ", (int) tlv_stream_len  );
            break;
          }
        }
        else
        {
          QCRIL_LOG_ERROR( "QMI IDL - unsol event decode failed to obtain message len for msg id %d, idl err %d", (int) message_id, (int) idl_err  );
          break;
        }
      }
      else
      {
        QCRIL_LOG_ERROR( "QMI IDL - unsol event decode failed to obtain svc object for svc id %d ", (int) service_id   );
        break;
      }
    }
    else
    { // legacy stream
      payload = (char *) qcril_malloc( QCRIL_OTHER_OEM_NAME_LENGTH + sizeof(unsol_event) + sizeof(data_len) + data_len );
      if ( NULL != payload )
      {
        memcpy( payload, QCRIL_HOOK_OEM_NAME, QCRIL_OTHER_OEM_NAME_LENGTH );
        index += QCRIL_OTHER_OEM_NAME_LENGTH;

        memcpy( &payload[index], &unsol_event, sizeof(unsol_event) );
        index += sizeof(unsol_event);

        memcpy( &payload[index], &data_len, sizeof(data_len) );
        index += sizeof(data_len);

        memcpy( &payload[index], data, data_len );
        index += data_len;

        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_OEM_HOOK_RAW, &unsol_resp );
        unsol_resp.resp_pkt = ( void * ) payload;
        unsol_resp.resp_len = index;

        if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_OEM_IND_TO_BOTH))
        {
          qcril_qmi_oem_socket_send_unsol(unsol_resp.resp_pkt,
                                          unsol_resp.resp_len);
          qcril_send_unsol_response( &unsol_resp );
        }
        else if (!qmi_ril_is_multi_sim_oem_hook_request(unsol_event) &&
             qmi_ril_is_feature_supported(QMI_RIL_FEATURE_OEM_SOCKET))
        {
          qcril_qmi_oem_socket_send_unsol(unsol_resp.resp_pkt,
                                          unsol_resp.resp_len);
        }
        else
        {
          qcril_send_unsol_response( &unsol_resp );
        }
      }
      else
      {
        QCRIL_LOG_ERROR( "qcril_malloc returned NULL" );
        break;
      }
    }

  } while ( FALSE );

  if ( NULL != payload )
  {
    qcril_free( payload );
  }

} // qcril_hook_unsol_response


/*===========================================================================

FUNCTION qcril_hash

DESCRIPTION
  Return a hash of the mobile ESN and other selected inputs ( see CAI 2-216 )
    R = floor( N* (( 40503 * ( L ^ H ^ DECORR )) % 65536 ) / 65536 )

DEPENDENCIES
  None.

RETURN VALUE
  A number between 0 and N-1.  Note that in some cases the CAI requires that
  1 be added to this.  This is up to the caller.

SIDE EFFECTS
  None.

===========================================================================*/
static uint16 qcril_hash
(
  uint32 hash_key,
  uint16  n, /* Range */
  uint16  decorr
)
{
  uint16 temp;

  /*-----------------------------------------------------------------------*/

  temp = (decorr ^ ((uint16)( hash_key & 0xFFFF )) ^ ((uint16)(( hash_key>>16 ) & 0xFFFF )));
  temp = ( temp * 40503L ) & 0xFFFF;
  temp = (uint16)(( (long) temp * n ) >> 16);

  return( temp );

} /* qcril_hash() */

/*===========================================================================

  FUNCTION:  qcril_hash_table_lookup

===========================================================================*/
/*!
    @brief
    Finds the dispatch table entry for a given event_id

    @return
    E_SUCCESS if an entry is found
    E_FAILURE if no entry is found

*/
/*=========================================================================*/
IxErrnoType qcril_hash_table_lookup
(
  uint32 id,
  qcril_dispatch_table_entry_type **entry_ptr_ptr /*!< OUT table entry ptr, if found */
)
{
  uint32 hash_index; /*!< Indicies into hash table */
  IxErrnoType status = E_SUCCESS;
  qcril_dispatch_table_entry_type *temp_entry_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  hash_index = qcril_hash( id, QCRIL_HT_ENTRIES_MAX, 0 );

  if(hash_index < QCRIL_HT_ENTRIES_MAX)
  {
    temp_entry_ptr = (qcril_dispatch_table_entry_type *) qcril_hash_table[hash_index];
  }

  /* Look through the hash table collision list for this entry. */

  while (temp_entry_ptr != NULL)
  {
    if (temp_entry_ptr->event_id == id)
    {
      if ( entry_ptr_ptr != NULL )
      {
        *entry_ptr_ptr = temp_entry_ptr;
      }
      break;
    }
    temp_entry_ptr = temp_entry_ptr->next_ptr;
  }

  if (temp_entry_ptr == NULL)
  {
    status = E_FAILURE;
  }

  return (status);

} /* qcril_hash_table_lookup() */


//===========================================================================
// qcril_qmi_mgr_modem_state_updated
//===========================================================================
void qcril_qmi_mgr_modem_state_updated(qcril_instance_id_e_type instance_id, qcril_modem_state_e_type new_modem_state)
{
  qcril_arb_state_info_struct_type *s_ptr;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  qmi_ril_enter_critical_section();
  s_ptr = &qcril_state->info[ instance_id ];
  s_ptr->modem_state = new_modem_state;
  qmi_ril_leave_critical_section();

  QCRIL_LOG_FUNC_RETURN_WITH_RET(new_modem_state);
} // qcril_qmi_mgr_modem_state_updated

//===========================================================================
// qcril_qmi_mgr_voice_technology_updated
//===========================================================================
void qcril_qmi_mgr_voice_technology_updated(qcril_radio_tech_e_type new_voice_technology)
{
  qcril_arb_state_info_struct_type *s_ptr;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_enter_critical_section();
  s_ptr = &qcril_state->info[ QCRIL_DEFAULT_INSTANCE_ID ];
  s_ptr->voice_radio_tech = new_voice_technology;

  s_ptr = &qcril_state->info[ QCRIL_SECOND_INSTANCE_ID ];
  s_ptr->voice_radio_tech = new_voice_technology;

  // TODO_TSTS: This code seems not required. This info is used no where.
  s_ptr = &qcril_state->info[ QCRIL_THIRD_INSTANCE_ID ];
  s_ptr->voice_radio_tech = new_voice_technology;
  qmi_ril_leave_critical_section();

  QCRIL_LOG_FUNC_RETURN_WITH_RET(new_voice_technology);
} // qcril_qmi_mgr_modem_state_updated

//===========================================================================
//qcril_qmi_mgr_get_known_voice_technology
//===========================================================================
qcril_radio_tech_e_type qcril_qmi_mgr_get_known_voice_technology(void)
{
  qcril_radio_tech_e_type res;
  qcril_arb_state_info_struct_type *s_ptr;

  qmi_ril_enter_critical_section();
  s_ptr = &qcril_state->info[ QCRIL_DEFAULT_INSTANCE_ID ];
  res = s_ptr->voice_radio_tech;
  qmi_ril_leave_critical_section();

  return res;
}

/*===========================================================================

  FUNCTION:  qcril_state_transition

===========================================================================*/
/*!
    @brief
    Called each time an event handler returns, to process the next state
    of qcril.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_state_transition
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  int event_id,
  const qcril_request_return_type *ret_ptr
)
{
  qcril_arb_state_info_struct_type *s_ptr, *sec_s_ptr;
  boolean pri_gw_sim_state_changed = FALSE, pri_cdma_sim_state_changed = FALSE;
  boolean sec_gw_sim_state_changed = FALSE, sec_cdma_sim_state_changed = FALSE;
  qcril_sim_state_e_type current_pri_gw_sim_state, next_pri_gw_sim_state, current_pri_cdma_sim_state, next_pri_cdma_sim_state;
  qcril_sim_state_e_type current_sec_gw_sim_state, next_sec_gw_sim_state, current_sec_cdma_sim_state, next_sec_cdma_sim_state;
  boolean ter_gw_sim_state_changed = FALSE;
  boolean ter_cdma_sim_state_changed = FALSE;
  qcril_sim_state_e_type current_ter_gw_sim_state;
  qcril_sim_state_e_type next_ter_gw_sim_state;
  qcril_sim_state_e_type current_ter_cdma_sim_state;
  qcril_sim_state_e_type next_ter_cdma_sim_state;


  char *sim_state_name[] = { "Absent", "Not ready", "Ready", "PIN", "PUK", "Network personalization", "Error", "Illegal" };

  modem_id = modem_id;

  if(instance_id < QCRIL_MAX_INSTANCE_ID && ret_ptr != NULL)
  {
    /*-----------------------------------------------------------------------*/

    s_ptr = &qcril_state->info[ instance_id ];
    sec_s_ptr = &qcril_state->info[ QCRIL_DSDS_INSTANCE_PAIR( instance_id ) ];

    /*-----------------------------------------------------------------------*/

    QCRIL_MUTEX_LOCK( &qcril_state->mutex, "qcril_state_mutex" );

    current_pri_gw_sim_state = next_pri_gw_sim_state = s_ptr->pri_gw_sim_state;
    current_pri_cdma_sim_state = next_pri_cdma_sim_state = s_ptr->pri_cdma_sim_state;
    current_sec_gw_sim_state = next_sec_gw_sim_state = s_ptr->sec_gw_sim_state;
    current_sec_cdma_sim_state = next_sec_cdma_sim_state = s_ptr->sec_cdma_sim_state;
    current_ter_gw_sim_state = next_ter_gw_sim_state = s_ptr->ter_gw_sim_state;
    current_ter_cdma_sim_state = next_ter_cdma_sim_state = s_ptr->ter_cdma_sim_state;

    /* Check whether the primary GSM/WCDMA SIM state is changed */
    if ( ret_ptr->pri_gw_sim_state_changed && ( ret_ptr->next_pri_gw_sim_state !=  s_ptr->pri_gw_sim_state ) )
    {
      pri_gw_sim_state_changed = TRUE;
      next_pri_gw_sim_state = ret_ptr->next_pri_gw_sim_state;
    }

    /* Check whether the primary CDMA SIM state is changed */
    if ( ret_ptr->pri_cdma_sim_state_changed && ( ret_ptr->next_pri_cdma_sim_state !=  s_ptr->pri_cdma_sim_state ) )
    {
      pri_cdma_sim_state_changed = TRUE;
      next_pri_cdma_sim_state = ret_ptr->next_pri_cdma_sim_state;
    }

    if (qmi_ril_is_feature_supported( QMI_RIL_FEATURE_DSDS ))
    {
      /* Check whether the secondary GSM/WCDMA SIM state is changed */
      if ( ret_ptr->sec_gw_sim_state_changed && ( ret_ptr->next_sec_gw_sim_state !=  s_ptr->sec_gw_sim_state ) )
      {
        sec_gw_sim_state_changed = TRUE;
        next_sec_gw_sim_state = ret_ptr->next_sec_gw_sim_state;
      }

      /* Check whether the secondary CDMA SIM state is changed */
      if ( ret_ptr->sec_cdma_sim_state_changed && ( ret_ptr->next_sec_cdma_sim_state !=  s_ptr->sec_cdma_sim_state ) )
      {
        sec_cdma_sim_state_changed = TRUE;
        next_sec_cdma_sim_state = ret_ptr->next_sec_cdma_sim_state;
      }

      /* Check whether the tertiary GSM/WCDMA SIM state is changed */
      if ( ret_ptr->ter_gw_sim_state_changed && ( ret_ptr->next_ter_gw_sim_state !=  s_ptr->ter_gw_sim_state ) )
      {
        ter_gw_sim_state_changed = TRUE;
        next_ter_gw_sim_state = ret_ptr->next_ter_gw_sim_state;
      }

      /* Check whether the tertiary CDMA SIM state is changed */
      if ( ret_ptr->ter_cdma_sim_state_changed && ( ret_ptr->next_ter_cdma_sim_state !=  s_ptr->ter_cdma_sim_state ) )
      {
        ter_cdma_sim_state_changed = TRUE;
        next_ter_cdma_sim_state = ret_ptr->next_ter_cdma_sim_state;
      }
    }

    QCRIL_LOG_DEBUG( "[%s(%d)] GW SIM(pri): %s --> %s, CDMA SIM(pri): %s --> %s,"
                     "GW SIM(sec): %s --> %s, CDMA SIM(sec): %s --> %s,"
                     "GW SIM(ter): %s --> %s, CDMA SIM(ter): %s --> %s",
                     qcril_log_lookup_event_name( event_id ), event_id,
                     sim_state_name[ s_ptr->pri_gw_sim_state ], sim_state_name[ next_pri_gw_sim_state ],
                     sim_state_name[ s_ptr->pri_cdma_sim_state ], sim_state_name[ next_pri_cdma_sim_state ],
                     sim_state_name[ s_ptr->sec_gw_sim_state ], sim_state_name[ next_sec_gw_sim_state ],
                     sim_state_name[ s_ptr->sec_cdma_sim_state ], sim_state_name[ next_sec_cdma_sim_state ],
                     sim_state_name[ s_ptr->ter_gw_sim_state ], sim_state_name[ next_ter_gw_sim_state ],
                     sim_state_name[ s_ptr->ter_cdma_sim_state ], sim_state_name[ next_ter_cdma_sim_state ] );


    /* Change in Modem State, SIM state or Subscription Config mask. Figure out the states update */
    if ( pri_gw_sim_state_changed || pri_cdma_sim_state_changed
         || sec_gw_sim_state_changed || sec_cdma_sim_state_changed
         || ter_gw_sim_state_changed || ter_cdma_sim_state_changed
       )
    {
      /* Save the Modem State, SIM state */
      s_ptr->pri_gw_sim_state = next_pri_gw_sim_state;
      s_ptr->pri_cdma_sim_state = next_pri_cdma_sim_state;
      s_ptr->sec_gw_sim_state = next_sec_gw_sim_state;
      s_ptr->sec_cdma_sim_state = next_sec_cdma_sim_state;
      s_ptr->ter_gw_sim_state = next_ter_gw_sim_state;
      s_ptr->ter_cdma_sim_state = next_ter_cdma_sim_state;
    }

    QCRIL_MUTEX_UNLOCK( &qcril_state->mutex, "qcril_state_mutex" );
  }
  else
  {
    QCRIL_LOG_FATAL("CHECK FAILED");
  }

} /* qcril_state_transition() */


/*===========================================================================

  FUNCTION:  qcril_dispatch_event

===========================================================================*/
/*!
    @brief
    Does processing common to onRequest and qcril_process_event with
    respect to calling the event handler and processing the return value
    from the event handler.

    @return
    E_SUCCESS of the event was handled normally
    E_NOT_ALLOWED if the event is not supported in the current state

*/
/*=========================================================================*/
IxErrnoType qcril_dispatch_event
(
  qcril_dispatch_table_entry_type *entry_ptr,
  qcril_request_params_type *params_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_arb_state_info_struct_type *s_ptr;
  qcril_request_return_type ret;

  IxErrnoType res = E_NOT_ALLOWED;
  qmi_ril_gen_operational_status_type cur_status = qmi_ril_get_operational_status();

  if(params_ptr != NULL && (params_ptr->instance_id < QCRIL_MAX_INSTANCE_ID) )
  {
    // print the recieved date byte stream
    qcril_qmi_print_hex(params_ptr->data,  params_ptr->datalen);

    instance_id = params_ptr->instance_id;
    s_ptr = &qcril_state->info[ instance_id ];
    modem_id = params_ptr->modem_id;

    switch ( cur_status )
    {
      case QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDING:
      case QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDED:
      case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_PENDING:
      case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_RETRY:
      case QMI_RIL_GEN_OPERATIONAL_STATUS_UNBIND:
        switch (params_ptr->event_id)
          {
          case QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_CON:
          if ( ( cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_PENDING ) ||
               ( cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_RESUME_RETRY ) )
          {
            QCRIL_LOG_DEBUG("Dropping suspend event %d state %d", params_ptr->event_id, cur_status);
            res = E_NOT_ALLOWED;
          }
          else
          {
            res = E_SUCCESS;
          }
          break;

          case QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_CON:
          if ( ( cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDING ) ||
               ( cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDED ) )
          {
            QCRIL_LOG_DEBUG("Dropping resume event %d state %d", params_ptr->event_id, cur_status);
            res = E_NOT_ALLOWED;
          }
          else
          {
            res = E_SUCCESS;
          }
          break;

          case QCRIL_EVT_DATA_EVENT_CALLBACK:
          case QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK:
          case QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK:
          case QCRIL_EVT_INTERNAL_UIM_SAP_RESP:
          case QCRIL_EVT_UIM_RMT_QMI_COMMAND_CALLBACK:
          case QCRIL_EVT_UIM_RMT_QMI_INDICATION_CALLBACK:
          case QCRIL_EVT_QMI_RIL_SEND_UNSOL_RADIO_STATE_CHANGED:
          case QCRIL_EVT_QMI_RIL_EMBMS_SEND_UNSOL_RADIO_STATE:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_CHECK_IF_SERVICE_UP:
          case QCRIL_EVT_QMI_RIL_SERVICE_DOWN:
            res = E_SUCCESS;
            break;

          // allow tunelling specifically for RIL_REQUEST_SCREEN_STATE
          // and RIL_REQUEST_SHUTDOWN
          case RIL_REQUEST_SCREEN_STATE:
          case RIL_REQUEST_SHUTDOWN:
            res = E_SUCCESS;
            break;

          case RIL_REQUEST_RADIO_POWER:
            if ( (qcril_qmi_nas_modem_power_is_mdm_shdn_in_apm() &&
                0 == qcril_qmi_modem_power_voting_state()) ||
                (qmi_ril_get_operational_status() == QMI_RIL_GEN_OPERATIONAL_STATUS_UNBIND) )
            {
                res = E_SUCCESS;
            }
            break;

          case QCRIL_EVT_HOOK_UPDATE_SUB_BINDING:
          case QCRIL_EVT_QMI_REQUEST_POWER_RADIO:
          case QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN:
          case QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP:
          case QCRIL_EVT_CM_CARD_STATUS_UPDATED:
          case QCRIL_EVT_QMI_NAS_CARD_STATUS_UPDATE:
          case QCRIL_EVT_QMI_DMS_HANDLE_INDICATIONS:
          case QCRIL_EVT_QMI_VOICE_HANDLE_INDICATIONS:
          case QCRIL_EVT_QMI_NAS_HANDLE_INDICATIONS:
          case QCRIL_EVT_QMI_VOICE_HANDLE_COMM_CALLBACKS:
          case QCRIL_EVT_QMI_RIL_POST_VOICE_RTE_CHANGE_IND:
          case RIL_REQUEST_DIAL:
          case RIL_REQUEST_SIM_IO:
          case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
            if ( qmi_ril_get_operational_status() == QMI_RIL_GEN_OPERATIONAL_STATUS_UNBIND )
            {
              QCRIL_LOG_DEBUG("Operational status %d, event id %d", qmi_ril_get_operational_status(), params_ptr->event_id);
              res = E_SUCCESS;
            }
            else
            {
              res = E_NOT_ALLOWED;
            }
            break;

          default:
            res = E_NOT_ALLOWED;
            break;
          }
        break;


      case QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_ONGOING: // fallthrough
      case QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED: // fallthrough
      case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING:
        switch (params_ptr->event_id)
        {
          case QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_REQ:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_CON:
          case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_CON:
          if ( cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING )
          {
            QCRIL_LOG_DEBUG("Dropping suspend event %d state %d", params_ptr->event_id, cur_status);
            res = E_NOT_ALLOWED;
          }
          else
          {
            res = E_SUCCESS;
          }
          break;

          default:
          res = E_SUCCESS;
          break;
        }
        break;

      default:
        res = E_NOT_ALLOWED;
        break;
    }
    if (E_SUCCESS == res)
      {
        QCRIL_MUTEX_LOCK( &qcril_state->mutex, "qcril_state_mutex" );

        /* Initialize the structure that the request handler will use to return
           information about the status of the request */
        ret.pri_gw_sim_state_changed = FALSE;
        ret.next_pri_gw_sim_state = s_ptr->pri_gw_sim_state;
        ret.pri_cdma_sim_state_changed = FALSE;
        ret.next_pri_cdma_sim_state = s_ptr->pri_cdma_sim_state;
        ret.sec_gw_sim_state_changed = FALSE;
        ret.next_sec_gw_sim_state = s_ptr->sec_gw_sim_state;
        ret.sec_cdma_sim_state_changed = FALSE;
        ret.next_sec_cdma_sim_state = s_ptr->sec_cdma_sim_state;
        ret.ter_gw_sim_state_changed = FALSE;
        ret.next_ter_gw_sim_state = s_ptr->ter_gw_sim_state;
        ret.ter_cdma_sim_state_changed = FALSE;
        ret.next_ter_cdma_sim_state = s_ptr->ter_cdma_sim_state;

        QCRIL_MUTEX_UNLOCK( &qcril_state->mutex, "qcril_state_mutex" );

        /* Dispatch the request to the appropriate handler */
        (entry_ptr->handler)(params_ptr, &ret);

        /* Handle state transition */
        if ( ret.pri_gw_sim_state_changed || ret.pri_cdma_sim_state_changed ||
             ret.sec_gw_sim_state_changed || ret.sec_cdma_sim_state_changed ||
             ret.ter_gw_sim_state_changed || ret.ter_cdma_sim_state_changed
           )
        {
          qcril_state_transition( instance_id, modem_id, params_ptr->event_id, &ret );
        }
      }
  }
  else
  {
    res = E_NOT_ALLOWED;
    QCRIL_LOG_FATAL("CHECK FAILED");
    if ( NULL != params_ptr )
    {
      QCRIL_LOG_FATAL(".. failure details instance id %d, modem id %d", (int)params_ptr->instance_id, (int)params_ptr->modem_id  );
    }
  }

  return res;

} // qcril_dispatch_event

//=========================================================================
// qmi_ril_oem_hook_get_request_id
//===========================================================================
RIL_Errno qmi_ril_oem_hook_get_request_id
(
  uint16   service_id,
  uint16   message_id,
  uint32   *request_id,
  char **  log_ind
)
{
  RIL_Errno  result = RIL_E_SUCCESS;
  char * msg_returned = NULL;

  if( service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_VT )
  {
     switch (message_id )
     {
        case  IMS_VT_DIAL_CALL_REQ_V01:
            *request_id  = QCRIL_EVT_HOOK_VT_DIAL_CALL;
            msg_returned = "QCRIL_EVT_HOOK_VT_DIAL_CALL";
            break;

        case  IMS_VT_END_CALL_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_VT_END_CALL;
            msg_returned = "QCRIL_EVT_HOOK_VT_END_CALL";
            break;

        case  IMS_VT_ANSWER_CALL_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_VT_ANSWER_CALL;
            msg_returned = "QCRIL_EVT_HOOK_VT_ANSWER_CALL";
            break;

        case  IMS_VT_GET_CALL_INFO_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_VT_GET_CALL_INFO;
            msg_returned = "QCRIL_EVT_HOOK_VT_GET_CALL_INFO";
            break;

        default:
            QCRIL_LOG_ERROR("invalid messsage-id = %d received",  message_id);
            result = RIL_E_GENERIC_FAILURE;
            break;
     }
  }
  else if ( service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE )
  {
     switch (message_id )
     {
        case  QMI_IMSP_GET_ENABLER_STATE_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_ENABLER_STATE_REQ;
            msg_returned = "QCRIL_EVT_HOOK_IMS_ENABLER_STATE_REQ";
            break;

        case  QMI_IMSP_SEND_PUBLISH_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_REQ;
            msg_returned = "QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_REQ";
            break;

        case QMI_IMSP_SEND_PUBLISH_XML_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_XML_REQ;
            msg_returned = "QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_XML_REQ";
            break;

        case  QMI_IMSP_SEND_UNPUBLISH_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_SEND_UNPUBLISH_REQ;
            msg_returned = "QCRIL_EVT_HOOK_IMS_SEND_UNPUBLISH_REQ";
            break;

        case  QMI_IMSP_SEND_SUBSCRIBE_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_REQ;
            msg_returned = "QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_REQ";
            break;

        case  QMI_IMSP_SEND_SUBSCRIBE_XML_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_XML_REQ;
            msg_returned = "QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_XML_REQ";
            break;

        case  QMI_IMSP_SEND_UNSUBSCRIBE_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_SEND_UNSUBSCRIBE_REQ;
            msg_returned = "QCRIL_EVT_HOOK_IMS_SEND_UNSUBSCRIBE_REQ";
            break;

        case  QMI_IMSP_SET_NOTIFY_FMT_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_SET_NOTIFY_FMT_REQ_V01;
            msg_returned = "QCRIL_EVT_HOOK_IMS_SET_NOTIFY_FMT_REQ_V01";
            break;

        case  QMI_IMSP_GET_NOTIFY_FMT_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_GET_NOTIFY_FMT_REQ_V01;
            msg_returned = "QCRIL_EVT_HOOK_IMS_GET_NOTIFY_FMT_REQ_V01";
            break;

        case  QMI_IMSP_SET_EVENT_REPORT_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_SET_EVENT_REPORT_REQ_V01;
            msg_returned = "QCRIL_EVT_HOOK_IMS_SET_EVENT_REPORT_REQ_V01";
            break;

        case  QMI_IMSP_GET_EVENT_REPORT_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_IMS_GET_EVENT_REPORT_REQ_V01;
            msg_returned = "QCRIL_EVT_HOOK_IMS_GET_EVENT_REPORT_REQ_V01";
            break;

        default:
            QCRIL_LOG_ERROR("invalid messsage-id = %d received",  message_id);
            result = RIL_E_GENERIC_FAILURE;
            break;
      }
  }
  else if( service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS )
  {
     switch (message_id )
     {
        case  QMI_EMBMS_ENABLE_EMBMS_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_ENABLE;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_ENABLE";
            break;

        case  QMI_EMBMS_DISABLE_EMBMS_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_DISABLE;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_DISABLE";
            break;

        case  QMI_EMBMS_ACTIVATE_TMGI_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI";
            break;

        case  QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI";
            break;

        case  QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_GET_AVAILABLE_TMGI;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_AVAILABLE_TMGI";
            break;

        case  QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_TMGI;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_TMGI";
            break;

        case  QMI_EMBMS_UPDATE_CONTENT_DESC_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_CONTENT_DESC_UPDATE;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_CONTENT_DESC_UPDATE";
            break;

        case  QMI_EMBMS_ENABLE_RSSI_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_ENABLE_RSSI;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_ENABLE_RSSI";
            break;

        case  QMI_EMBMS_DISABLE_RSSI_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_DISABLE_RSSI;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_DISABLE_RSSI";
            break;

        case  QMI_EMBMS_GET_COVERAGE_STATE_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_GET_COVERAGE_STATE;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_COVERAGE_STATE";
            break;

        case  QMI_EMBMS_GET_RSSI_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_GET_RSSI;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_RSSI";
            break;

        case  QMI_EMBMS_SET_SNTP_TIME_REQ_V01:
         *request_id =  QCRIL_EVT_HOOK_EMBMS_SET_SNTP_TIME;
         msg_returned = "QCRIL_EVT_HOOK_EMBMS_SET_SNTP_TIME";
         break;

        case  QMI_EMBMS_GET_SIB16_COVERAGE_REQ_V01:
           *request_id =  QCRIL_EVT_HOOK_EMBMS_GET_SIB16_COVERAGE;
           msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_SIB16_COVERAGE";
           break;

        case  QMI_EMBMS_GET_UTC_TIME_REQ_V01:
         *request_id =  QCRIL_EVT_HOOK_EMBMS_GET_UTC_TIME;
         msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_UTC_TIME";
         break;

        case  QMI_EMBMS_GET_EMBMS_SERVICE_STATE_REQ_V01:
            *request_id =  QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_SVC_STATE;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_SVC_STATE";
            break;

        case  QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01:
            *request_id = QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI";
            break;

        case  QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_REQ_V01:
            *request_id = QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_LOG_PACKET_IDS;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_LOG_PACKET_IDS";
            break;

        case  QMI_EMBMS_DELIVER_LOG_PACKET_REQ_V01:
            *request_id = QCRIL_EVT_HOOK_EMBMS_DELIVER_LOG_PACKET;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_DELIVER_LOG_PACKET";
            break;

        case  QMI_EMBMS_GET_E911_STATE_REQ_V01:
            *request_id = QCRIL_EVT_HOOK_EMBMS_GET_E911_STATE;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_E911_STATE";
            break;

        case  QMI_EMBMS_GET_SIB_PLMN_REQ_V01:
            *request_id = QCRIL_EVT_HOOK_EMBMS_GET_SIB_PLMN;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_SIB_PLMN";
            break;

        case  QMI_EMBMS_GET_EMBMS_STATUS_REQ_V01:
            *request_id = QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_STATUS;
            msg_returned = "QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_STATUS";
            break;

        default:
            QCRIL_LOG_ERROR("invalid messsage-id = %d received",  message_id);
            result = RIL_E_GENERIC_FAILURE;
            break;
     }
  }
  else if ( service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_QTUNER )
  {
    switch( message_id )
    {
      case QMI_Qtuner_SET_RFM_SCENARIO_REQ_V01:
        *request_id =  QCRIL_EVT_HOOK_SET_RFM_SCENARIO_REQ;
        msg_returned = "QCRIL_EVT_HOOK_SET_RFM_SCENARIO_REQ";
        break;

      case QMI_Qtuner_GET_RFM_SCENARIO_REQ_V01:
        *request_id =  QCRIL_EVT_HOOK_GET_RFM_SCENARIO_REQ;
        msg_returned = "QCRIL_EVT_HOOK_GET_RFM_SCENARIO_REQ";
        break;

      case QMI_Qtuner_GET_PROVISIONED_TABLE_REVISION_REQ_V01:
        *request_id = QCRIL_EVT_HOOK_GET_PROVISIONED_TABLE_REVISION_REQ;
        msg_returned = "QCRIL_EVT_HOOK_GET_PROVISIONED_TABLE_REVISION_REQ";
        break;

      default:
        QCRIL_LOG_ERROR("invalid messsage-id = %d received",  message_id);
        result = RIL_E_GENERIC_FAILURE;
        break;
    }
  }
  else
  {
     QCRIL_LOG_ERROR("invalid service-id = %d received", service_id);
      result = RIL_E_GENERIC_FAILURE;
   }

  *log_ind = msg_returned;

  return result;
}

//=========================================================================
// qmi_ril_parse_oem_hook_header
//===========================================================================
RIL_Errno qmi_ril_parse_oem_hook_header ( unsigned char *data, qmi_ril_oem_hook_request_details_type *outcome )
{
  char oem_name [ QCRIL_OTHER_OEM_NAME_LENGTH + 2 ];

  uint32 index = 0,
  cmd_id = 0;
  RIL_Errno res  = RIL_E_GENERIC_FAILURE;
  qmi_ril_oem_hook_request_details_type   outcome_data;

  memset( &outcome_data, 0, sizeof( outcome_data ) );

  do
  {
    if(data == NULL)
    {
       QCRIL_LOG_ERROR( "data is NULL");
       break;
    }

    // decode the raw string to find out the oem name string data[0 - 7], 8 bytes
    if( strncmp( (char *)data, QCRIL_HOOK_OEM_NAME, QCRIL_OTHER_OEM_NAME_LENGTH ) != 0 )
    {
       memcpy( oem_name, &data[index], QCRIL_OTHER_OEM_NAME_LENGTH );
       oem_name[QCRIL_OTHER_OEM_NAME_LENGTH] = '\0';
       QCRIL_LOG_ERROR( "Mismatch in oem_name between received=%s and expected=%s", oem_name, QCRIL_HOOK_OEM_NAME);
       break;
    }

    // incrementing the index by OEM name size i.e 9 bytes
    index += (uint32)QCRIL_OTHER_OEM_NAME_LENGTH;

    // decode the raw string to find out command id, data[9 - 12], 4 bytes
    memcpy( &cmd_id, &data[index], QCRIL_OTHER_OEM_REQUEST_ID_LEN );

    QCRIL_LOG_DEBUG("command-id = %d", cmd_id);

    if( cmd_id >= QCRIL_EVT_HOOK_MAX )
    {
       QCRIL_LOG_ERROR( "Received un expected command id = %lu", cmd_id );
       break;
    }

    switch ( cmd_id )
    {
      case QCRIL_EVT_HOOK_REQ_GENERIC:
        outcome_data.is_qmi_tunneling = TRUE;         // fall through
      case QCRIL_EVT_HOOK_NV_READ:                    // fall through
      case QCRIL_EVT_HOOK_NV_WRITE:                   // fall through
      case QCRIL_EVT_HOOK_DATA_GO_DORMANT:            // fall through
      case QCRIL_EVT_HOOK_ME_DEPERSONALIZATION:       // fall through
      case QCRIL_EVT_HOOK_CSG_PERFORM_NW_SCAN:
      case QCRIL_EVT_HOOK_CSG_SET_SYS_SEL_PREF:
      case QCRIL_EVT_HOOK_CSG_GET_SYS_INFO:
      case QCRIL_EVT_HOOK_UICC_VOLTAGE_STATUS_REQ:
      case QCRIL_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ:
      case QCRIL_EVT_HOOK_PERSONALIZATION_STATUS_REQ:
      // DSDS/DSDA requests
      case QCRIL_EVT_HOOK_SET_TUNE_AWAY:              // fall through
      case QCRIL_EVT_HOOK_GET_TUNE_AWAY:              // fall through
      case QCRIL_EVT_HOOK_SET_PAGING_PRIORITY:        // fall through
      case QCRIL_EVT_HOOK_GET_PAGING_PRIORITY:        // fall through
      case QCRIL_EVT_HOOK_SET_DEFAULT_VOICE_SUB:      // fall through
      case QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD:        // fall through
      case QCRIL_EVT_HOOK_GET_MODEM_CAPABILITY:
      case QCRIL_EVT_HOOK_UPDATE_SUB_BINDING:
      case QCRIL_EVT_HOOK_REJECT_INCOMING_CALL_CAUSE_21:
      case QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY:
        outcome_data.is_oem_hook    = TRUE;
        outcome_data.hook_req       = cmd_id;
        break;

      default:
        QCRIL_LOG_ERROR( "Received un-handled/oem command id = %lu", cmd_id );
        outcome_data.is_oem_hook    = TRUE;
        outcome_data.hook_req       = cmd_id;
        break;
    }

    // incrementing the index by command id size i.e 4 bytes
    index += (uint32)QCRIL_OTHER_OEM_REQUEST_ID_LEN;

    // decode the raw string to find the length of the payload, data[13 - 16],  4 bytes
    memcpy( &outcome_data.hook_req_len, &data[index], QCRIL_OTHER_OEM_REQUEST_DATA_LEN );
    // finally
    res = RIL_E_SUCCESS;

  } while ( FALSE );

  QCRIL_LOG_DEBUG( "res = %d", res );
  if ( RIL_E_SUCCESS == res )
  {
    *outcome = outcome_data;
    QCRIL_LOG_DEBUG( "is_tunneling = %d, is_oemhook = %d, event_id  = %d",
                     outcome->is_qmi_tunneling, outcome->is_oem_hook, outcome->hook_req );
  }

  return res;
} //  qmi_ril_parse_oem_hook_header


#define LOG_ONREQUEST_NONE                                 (0)
#define LOG_ONREQUEST_DISPATCHED                           (1)
#define LOG_ONREQUEST_FLOWCONTROL_DEFERRED_BUSY            (2)
#define LOG_ONREQUEST_FLOWCONTROL_REJECTED_OVERFLOW        (4)
#define LOG_ONREQUEST_FLOWCONTROL_EXEMPT                   (8)
#define LOG_ONREQUEST_DISPATCHED_CMD_MAIN_THRD             (16)
#define LOG_ONREQUEST_DISPATCHED_CMD_DEDICATED_THRD        (32)

/*===========================================================================

  FUNCTION:  onRequest

===========================================================================*/
/*!
    @brief
    Call from RIL to us to make a RIL_REQUEST
    Must be completed with a call to RIL_onRequestComplete()
    RIL_onRequestComplete() may be called from any thread, before or after
    this function returns.
    Returning from this routine implies the radio is ready to process another
    command (whether or not the previous command has completed).

    @return
    None.
*/
/*=========================================================================*/
static void onRequest
(
  qcril_instance_id_e_type  instance_id,
  int                       request,
  void                      *data,
  size_t                    datalen,
  RIL_Token                 t
)
{
  qcril_request_params_type param;
  int * in_data_ptr,
        in_data_val;

  char                                     label[ 512 ];
  qcril_request_resp_params_type           resp;

  RIL_Errno                                audit_result = RIL_E_GENERIC_FAILURE;
  qmi_ril_oem_hook_request_details_type    oem_hook_req_details;
  int                                      is_oem_hook_qmi_idl_tunneling;
  void *                                   substituted_data;
  uint32_t                                 substituted_data_len;
  qmi_idl_service_object_type              qmi_idl_tunneling_service_object;
  qmi_client_error_type                    idl_err;
  qmi_ril_oem_hook_response_context_type*  oem_hook_qmi_idl_resp_track;
  uint16 *                                 uint16_param;
  char*                                    evt_name;
  RIL_Errno                                info_fetch_result;

  qmi_ril_fw_android_request_kind_execution_overview_type*
                                            android_request_kind_execution_overview;
  uint32_t                                  android_request_handling_caps;
  qmi_ril_fw_android_request_holder_type*   android_request_param_holder;
  qmi_ril_fw_android_request_holder_type*   android_request_prm_hldr_iter;
  uint32_t                                  cap_max_queue_sz;
  uint32_t                                  cap_queue_sz_iter;
  int                                       no_render;
  qmi_ril_fw_android_request_kind_execution_overview_type*
                                            already_pending_request_kind;
  int                                       clean_family;
  qmi_ril_fw_android_request_holder_type *  relative_request_holder;
  qmi_ril_fw_android_request_holder_type    inbound_request_holder_local;

  qmi_ril_fw_android_request_kind_execution_overview_type*
                                            already_awaiting_exec_request_kind;
  int                                       any_already_awaiting_exec;

  void*                                     android_request_param_copy;
  int                                       android_request_param_copy_len;
  uint32                                    android_request_param_4_bytes;
  int                                       android_request_param_4_bytes_len;
  qmi_ril_fw_android_param_copy_approach_type
                                            request_param_method;
  int                                       local_param_copy_success;


  char                                      log_essence[ QCRIL_MAX_LOG_MSG_SIZE ];
  char                                      log_addon[ QCRIL_MAX_LOG_MSG_SIZE ];
  uint32                                    log_flags = LOG_ONREQUEST_NONE;
  int                                       log_android_request_id = 0;
  int                                       log_pending_android_request_id = 0;
  int                                       log_projected_token = 0;
  int                                       log_pending_projected_token = 0;
  qmi_ril_fw_android_request_holder_type*   log_waiting_for_execution_exec_req_holder = NULL;
  int                                       log_dispatch_dedicated_thrd = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( t != (void *) QCRIL_TOKEN_ID_INTERNAL );

  /*-----------------------------------------------------------------------*/

  memset( &oem_hook_req_details, 0, sizeof(oem_hook_req_details) );

  do
  {
          no_render                     = FALSE;

          substituted_data              = NULL;

          log_android_request_id        = request;
          log_projected_token           = qcril_log_get_token_id( t );

          memset ( &inbound_request_holder_local, 0, sizeof( inbound_request_holder_local ) );
          inbound_request_holder_local.param_info_and_len = QMI_RIL_FW_ANDROID_REQUEST_INFO_COMPOSE( QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE, QMI_RIL_ZERO );

          // Initialize the input parameters for the request handler.  This allows for
          //   easy addition or deletion of parameters in the future.
          param.event_id          = request;
          param.data              = data;
          param.datalen           = datalen;
          param.t                 = t;
          param.instance_id       = instance_id;
          param.modem_id          = QCRIL_DEFAULT_MODEM_ID;

          QCRIL_LOG_DEBUG( "UI --- %s (%d) ---> RIL [RID %d, token id %d, data len %d]",
                           qcril_log_lookup_event_name( param.event_id ), param.event_id, param.instance_id,
                           qcril_log_get_token_id( param.t ), param.datalen );
          /* Print RIL Message */
          qcril_log_print_ril_message(param.event_id, RIL__MSG_TYPE__REQUEST, param.data,
                                       param.datalen, info_fetch_result);

          /* check if request is suppressed */
          if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_SUPPRESS_REQ) &&
                  qcril_request_check_if_suppressed(param.event_id))
          {
              audit_result = RIL_E_GENERIC_FAILURE;
              QCRIL_LOG_INFO("RIL Request event id: %d is suppressed", param.event_id);
              break;
          }

          if ( ( param.event_id == RIL_REQUEST_DIAL ) || ( param.event_id == RIL_REQUEST_SETUP_DATA_CALL ) )
          {
            QCRIL_SNPRINTF( label, sizeof( label ), "%s - RID %d, Token id %d", qcril_log_lookup_event_name( param.event_id ),
                            param.instance_id, qcril_log_get_token_id( param.t ) );
            // Use bold arrows for really interesting events
            QCRIL_LOG_CF_PKT_RIL_REQ2( param.instance_id, label );
          }
          else if ( ( ( param.event_id == RIL_REQUEST_RADIO_POWER ) ||
                      ( param.event_id == RIL_REQUEST_SCREEN_STATE ) ||
                      ( param.event_id == RIL_REQUEST_SET_MUTE ) ) && ( param.data != NULL ) )
          {
            in_data_ptr = ( int * ) param.data;
            if ( in_data_ptr != NULL )
            {
              in_data_val = *in_data_ptr;
            }
            else
            {
              in_data_val = 1;
            }

            QCRIL_SNPRINTF( label, sizeof( label ), "%s - %s, Token id %d",
                            qcril_log_lookup_event_name( param.event_id ), ( in_data_val == 0 ) ? "Off" : "On" ,
                            qcril_log_get_token_id( param.t ) );
            QCRIL_LOG_CF_PKT_RIL_REQ( param.instance_id, label );
          }
          else
          {
            QCRIL_SNPRINTF( label, sizeof( label ), "%s - Token id %d",
                            qcril_log_lookup_event_name( param.event_id ), qcril_log_get_token_id( param.t ) );
            QCRIL_LOG_CF_PKT_RIL_REQ( param.instance_id, label );
          }

          if ( param.event_id >= (int) QCRIL_EVT_BASE )
          {
            // The request is out of range
            audit_result = RIL_E_REQUEST_NOT_SUPPORTED;
            break;
          }

          // print the recieved TLV byte stream
          QCRIL_LOG_DEBUG("data stream received from telephony");
          qcril_qmi_print_hex(param.data,  param.datalen);

          if ( param.event_id  == RIL_REQUEST_OEM_HOOK_RAW )
          {
            if (!qmi_ril_get_req_details_from_oem_req(&oem_hook_req_details,
                                                       &audit_result,
                                                        data,
                                                       &param,
                                                        datalen))
            {
              QCRIL_LOG_DEBUG("OEM HOOK RAW request %d not supported.",
                                           oem_hook_req_details.hook_req);
              break;
            }
            if (!qmi_ril_is_feature_supported(QMI_RIL_FEATURE_OEM_SOCKET) ||
                 qmi_ril_is_multi_sim_oem_hook_request(oem_hook_req_details.hook_req))
            {
              // oem hook qmi idl tunneling
              if ( oem_hook_req_details.is_qmi_tunneling )
              {
                if (!qmi_ril_parse_oem_req_tunnelled_message(&oem_hook_req_details,
                                                             &audit_result,
                                                             &param))
                {
                  break;
                }
              }
            }
            else
            {
              QCRIL_LOG_DEBUG("OEM HOOK RAW messages are supported through oem socket, "
                              "not through rild socket");
              audit_result = RIL_E_REQUEST_NOT_SUPPORTED;
              break;
            }
          }

          // flow control s
          qmi_ril_fw_android_request_flow_control_info_lock();
          do
          {
            QCRIL_LOG_DEBUG( "android request id %d", request );
            if ( request <= QMI_RIL_ZERO || request > QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID )
              break;

            android_request_kind_execution_overview = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ request ];

            android_request_handling_caps = android_request_kind_execution_overview->nof_extra_holders_and_caps_and_dynamics;
            QCRIL_LOG_DEBUG( "handling caps %x hex", android_request_handling_caps );

            if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FLOW_CONTROL_EXEMPT )
            {
              audit_result = RIL_E_SUCCESS;
              log_flags |= LOG_ONREQUEST_FLOWCONTROL_EXEMPT;
              break;
            }

            // store param
            request_param_method = qmi_ril_fw_create_android_live_params_copy (
                                                                                  request,
                                                                                  data,
                                                                                  datalen,
                                                                                  (void*)&android_request_param_4_bytes,
                                                                                  &android_request_param_4_bytes_len,
                                                                                  &android_request_param_copy,
                                                                                  &android_request_param_copy_len
                                                                               );
            QCRIL_LOG_DEBUG( "method %d, datalen %d", (int)request_param_method, datalen );
            switch ( request_param_method )
            {
              case QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT:
                inbound_request_holder_local.param_info_and_len = QMI_RIL_FW_ANDROID_REQUEST_INFO_COMPOSE(
                           QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_INT, android_request_param_4_bytes_len );
                memcpy( (void*)&inbound_request_holder_local.param_payload_holder.param_int, (void*)&android_request_param_4_bytes, sizeof(int) );
                local_param_copy_success = TRUE;
                break;

              case QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY:
                inbound_request_holder_local.param_info_and_len = QMI_RIL_FW_ANDROID_REQUEST_INFO_COMPOSE(
                          QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_PTR, android_request_param_copy_len );
                inbound_request_holder_local.param_payload_holder.param_ptr = android_request_param_copy;
                local_param_copy_success = TRUE;
                break;

              case QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION:
                // no payload
                inbound_request_holder_local.param_info_and_len = QMI_RIL_FW_ANDROID_REQUEST_INFO_COMPOSE(
                           QMI_RIL_FW_ANDROID_REQUEST_INFO_NO_PAYLOAD, QMI_RIL_ZERO );
                local_param_copy_success = TRUE;
                break;

              default: // failure
                local_param_copy_success = FALSE;
                break;
            }
            if ( !local_param_copy_success )
            {
              audit_result = RIL_E_GENERIC_FAILURE;
              break;
            }

            // stote attributes
            inbound_request_holder_local.token = t;
            inbound_request_holder_local.moniker = qmi_ril_fw_android_request_flow_control_overview.common_moniker_book;
            qmi_ril_fw_android_request_flow_control_overview.common_moniker_book++;
            if ( QMI_RIL_ZERO == qmi_ril_fw_android_request_flow_control_overview.common_moniker_book )
            {
              qmi_ril_fw_android_request_flow_control_overview.common_moniker_book++;
            }
            // note QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE is now excluded in param_info_and_len


            // flow controlling for what's oustanding
            already_pending_request_kind = qmi_ril_fw_android_request_flow_control_find_busy_kind( android_request_kind_execution_overview );
            clean_family = FALSE;

            if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_MULTIPLE_AUTO_DROP_ON_DIFF_PARAMS &&
                 NULL != already_pending_request_kind )
            {
              QCRIL_LOG_DEBUG( "auto drop check" );
              if ( already_pending_request_kind != android_request_kind_execution_overview )
              { // different android request id
                clean_family = TRUE;
              }
              else
              { // same android request id
                relative_request_holder = qmi_ril_fw_android_request_flow_control_find_outstanding_request_within_kind( android_request_kind_execution_overview );
                if ( NULL != relative_request_holder )
                {
                  if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY )
                  {
                    clean_family = !qmi_ril_fw_android_request_flow_control_request_holders_have_same_param(
                                        &android_request_kind_execution_overview->holders.local_holder, &inbound_request_holder_local );
                  }
                  else if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE )
                  {
                    cap_max_queue_sz = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ( android_request_handling_caps );
                    android_request_prm_hldr_iter = android_request_kind_execution_overview->holders.extra_holders;
                    for ( cap_queue_sz_iter = QMI_RIL_ZERO; cap_queue_sz_iter < cap_max_queue_sz && !clean_family ; cap_queue_sz_iter++ )
                    {
                      if ( !(android_request_prm_hldr_iter->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE) )
                      {
                        if ( !qmi_ril_fw_android_request_flow_control_request_holders_have_same_param( android_request_prm_hldr_iter, &inbound_request_holder_local ) )
                        {
                          clean_family = TRUE;
                        }
                        else
                        {
                          android_request_prm_hldr_iter++;
                        }
                      }
                    }
                  }
                }
              }
            }
            QCRIL_LOG_DEBUG( "clean family %d", clean_family );
            if ( clean_family )
            {
              qmi_ril_fw_android_request_flow_control_abandon_requests_local_and_ring_zoned( android_request_kind_execution_overview, RIL_E_CANCELLED, FALSE );
              already_pending_request_kind = NULL;
            }
            // if nothing pending, anything already ready for exec
            already_awaiting_exec_request_kind = NULL;
            if ( NULL == already_pending_request_kind )
            {
              any_already_awaiting_exec = qmi_ril_fw_android_request_flow_control_find_request_for_execution( android_request_kind_execution_overview,
                                                                                  &already_awaiting_exec_request_kind,
                                                                                  &log_waiting_for_execution_exec_req_holder );
              QCRIL_LOG_DEBUG( "already waiting for exec %d", any_already_awaiting_exec );
            }


            cap_max_queue_sz = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ( android_request_handling_caps );

            // identify slot to store android req data
            android_request_param_holder = NULL;
            if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY )
            { // // single request supported
              if ( QMI_RIL_ZERO != android_request_kind_execution_overview->token_under_execution )
              { // single request supported and one already outstanding
                // action will be taken below on android_request_param_holder validation
              }
              else
              {
                android_request_param_holder = &android_request_kind_execution_overview->holders.local_holder;
              }
            }
            else if ( android_request_handling_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE  )
            {
              android_request_prm_hldr_iter = android_request_kind_execution_overview->holders.extra_holders;

              for ( cap_queue_sz_iter = QMI_RIL_ZERO; cap_queue_sz_iter < cap_max_queue_sz && NULL == android_request_param_holder; cap_queue_sz_iter++ )
              {
                if ( android_request_prm_hldr_iter->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE )
                {
                  android_request_param_holder = android_request_prm_hldr_iter;
                }
                else
                {
                  android_request_prm_hldr_iter++;
                }
              }
            }
            QCRIL_LOG_DEBUG( "android_request_param_holder %x hex", android_request_param_holder );
            if ( NULL == android_request_param_holder )
            { // no resources to accomodate the request
              audit_result = RIL_E_GENERIC_FAILURE;
              log_flags |= LOG_ONREQUEST_FLOWCONTROL_REJECTED_OVERFLOW;
              break;
            }

            // store params and token
            *android_request_param_holder = inbound_request_holder_local;
            qmi_ril_fw_android_request_flow_control_overview_request_review_holders( android_request_kind_execution_overview );

            QCRIL_LOG_DEBUG( "pending req kind 0x%x hex, awaitingexec req kind 0x%x hex", already_pending_request_kind, already_awaiting_exec_request_kind );
            if ( NULL != already_pending_request_kind || NULL != already_awaiting_exec_request_kind )
            { // may not commence execution now, need to wait for completion of already pending requests
              audit_result = RIL_E_SUCCESS;
              no_render    = TRUE;
              if ( NULL != already_pending_request_kind )
              {
                log_flags |= LOG_ONREQUEST_FLOWCONTROL_DEFERRED_BUSY;
                log_pending_android_request_id = already_pending_request_kind->original_android_request_id;
                log_pending_projected_token    = qcril_log_get_token_id ( already_pending_request_kind->token_under_execution );
              }
              else if ( NULL != already_awaiting_exec_request_kind )
              {
                log_flags |= LOG_ONREQUEST_FLOWCONTROL_DEFERRED_BUSY;
                log_pending_android_request_id = already_awaiting_exec_request_kind->original_android_request_id;
                if ( NULL != log_waiting_for_execution_exec_req_holder )
                {
                  log_pending_projected_token    = qcril_log_get_token_id ( log_waiting_for_execution_exec_req_holder->token );
                }
              }
              break;
            }

            // mark commencing of execution
            android_request_kind_execution_overview->token_under_execution = t;

            log_flags |= LOG_ONREQUEST_DISPATCHED;

            QCRIL_LOG_DEBUG( "token under exec %d", qcril_log_get_token_id ( android_request_kind_execution_overview->token_under_execution )  );

            audit_result = RIL_E_SUCCESS;
          } while (FALSE);
          qmi_ril_fw_android_request_flow_control_info_unlock();

          QCRIL_LOG_DEBUG( "pre-exec token id %d, a-r-id %d, audit %d, nrender %d", qcril_log_get_token_id( param.t ),
                                                                                    param.event_id,
                                                                                    (int)audit_result,
                                                                                    (int)no_render );

          if ( no_render )
            break;
          // flow control - e

          if ( RIL_E_SUCCESS == audit_result )
          {
            audit_result = qmi_ril_fw_android_request_render_execution( param.t,
                                                                        param.event_id,
                                                                        param.data,
                                                                        param.datalen,
                                                                        param.instance_id,
                                                                        &log_dispatch_dedicated_thrd );
            if ( log_dispatch_dedicated_thrd )
            {
              log_flags |= LOG_ONREQUEST_DISPATCHED_CMD_DEDICATED_THRD;
            }
            else
            {
              log_flags |= LOG_ONREQUEST_DISPATCHED_CMD_MAIN_THRD;
            }
          }
          else
          {
            if ( !(inbound_request_holder_local.param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE) )
            {
              qmi_ril_fw_android_request_flow_control_release_holder_info_bookref( &inbound_request_holder_local, param.event_id );
            }
          }

    } while ( FALSE );

    if ( RIL_E_SUCCESS != audit_result )
    {
      qcril_default_request_resp_params( param.instance_id, param.t, param.event_id, audit_result, &resp );

      if ( param.event_id == RIL_REQUEST_OEM_HOOK_RAW )
      {
        resp.rild_sock_oem_req = TRUE;
      }

      qcril_send_request_response( &resp );
    } // otherwise it is taken that respective request handler has responded synchronously or will respond asynchronously

    QCRIL_LOG_DEBUG( "dispatch outcome = %d", audit_result );

    if ( substituted_data )
    {
      qcril_free( substituted_data );
    }

    if ( !( log_flags & LOG_ONREQUEST_DISPATCHED ) )
    { // not dispatched
      snprintf( log_essence, QCRIL_MAX_LOG_MSG_SIZE, "cmd %d, t_id %d ",
                              log_android_request_id, log_projected_token);

      if ( log_flags & LOG_ONREQUEST_FLOWCONTROL_DEFERRED_BUSY )
      {
        snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE, "defer exec FC cmd %d t_id %d", log_pending_android_request_id, log_pending_projected_token );
        strlcat( log_essence, log_addon, sizeof( log_essence ) );
      }
      else if ( log_flags & LOG_ONREQUEST_FLOWCONTROL_REJECTED_OVERFLOW )
      {
        strlcat( log_essence, "exec rej no no cap", sizeof( log_essence ) );
      }
      else
      {
        strlcat( log_essence, "exec not performed", sizeof( log_essence ) );
      }
      snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE, ",int_adt %d", (int) audit_result );
      strlcat( log_essence, log_addon, sizeof( log_essence ) );

      QCRIL_LOG_DEBUG( log_essence );
    }
} // onRequest

//===========================================================================
// qmi_ril_fw_dedicated_request_exec_thread
//===========================================================================
void * qmi_ril_fw_dedicated_request_exec_thread(void * param)
{
    pthread_t                                   thread_id;
    qmi_ril_gen_operational_status_type         cur_state;
    char                                        thread_name_buf[ QMI_RIL_THREAD_NAME_MAX_SIZE ];
    qmi_ril_dedicated_request_exec_params_type* exec_params;
    qcril_dispatch_table_entry_type*            dispatch_tbl_entry_ptr;
    RIL_Errno                                   audit_result;
    qcril_request_params_type                   pass_on_params;
    qcril_request_resp_params_type              resp;

    void*                                       param_ptr_to_be_passed;
    int                                         param_len_to_be_passed;

    QCRIL_LOG_FUNC_ENTRY();

    audit_result = RIL_E_GENERIC_FAILURE;

    exec_params = (qmi_ril_dedicated_request_exec_params_type*)param;
    if ( NULL != exec_params )
    {
      thread_id = pthread_self();
      QCRIL_SNPRINTF( thread_name_buf, sizeof(thread_name_buf), "cmd-%d(%d)", (int)exec_params->event_id, ((uint32_t)thread_id) % 1000 );
      qmi_ril_set_thread_name( thread_id, thread_name_buf );

      cur_state = qmi_ril_get_operational_status();
      QCRIL_LOG_INFO( " ..operational state %d", (int) cur_state );

      do
      {
          // lookup executive entry
          if ( qcril_hash_table_lookup( (uint32) exec_params->event_id , &dispatch_tbl_entry_ptr ) != E_SUCCESS )
          { // this shoud not happen as integrity check should have been done in onRequest
            audit_result = RIL_E_REQUEST_NOT_SUPPORTED;
            break;
          }

          // param setup
          switch ( exec_params->param_copy_arrron )
          {
            case QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT:
              param_ptr_to_be_passed  = (void*)&exec_params->copied_params.four_bytes;
              param_len_to_be_passed  = exec_params->original_data_len;
              break;

            case QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY:
              param_ptr_to_be_passed  = exec_params->copied_params.dynamic_copy;
              param_len_to_be_passed  = exec_params->original_data_len;
              break;

            case QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION:  // passthrough
            default:
              param_ptr_to_be_passed  = NULL;
              param_len_to_be_passed  = QMI_RIL_ZERO;
              break;

          }

          memset( &pass_on_params, 0, sizeof( pass_on_params ));
          pass_on_params.instance_id  = QCRIL_DEFAULT_INSTANCE_ID;
          pass_on_params.modem_id     = QCRIL_DEFAULT_MODEM_ID;
          pass_on_params.event_id     = exec_params->event_id;
          pass_on_params.t            = exec_params->token;
          pass_on_params.data         = param_ptr_to_be_passed;
          pass_on_params.datalen      = param_len_to_be_passed;

          // do exec
          if ( qcril_dispatch_event( dispatch_tbl_entry_ptr, &pass_on_params ) == E_NOT_ALLOWED )
          {
            audit_result = RIL_E_RADIO_NOT_AVAILABLE;
            break;
          }
          audit_result = RIL_E_SUCCESS;
      } while (FALSE);
      if ( RIL_E_SUCCESS != audit_result )
      {
        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, exec_params->token, exec_params->event_id, audit_result, &resp );

        if ( exec_params->event_id == RIL_REQUEST_OEM_HOOK_RAW )
        {
            resp.rild_sock_oem_req = TRUE;
        }

        qcril_send_request_response( &resp );
      } // otherwise it is taken that respective request handler has responded synchronously or will respond asynchronously

      // cleanup
      qmi_ril_fw_destroy_android_live_params_copy( exec_params->param_copy_arrron,
                                                   exec_params->event_id,
                                                   (void*)&exec_params->copied_params.four_bytes,
                                                   exec_params->copied_params.dynamic_copy );
      qcril_free( exec_params );

      qmi_ril_clear_thread_name( thread_id );
    }

    QCRIL_LOG_INFO( " ..exec res %d", (int) audit_result );

    QCRIL_LOG_FUNC_RETURN();
    return NULL;
} // qmi_ril_fw_dedicated_request_exec_thread


/*===========================================================================

  FUNCTION:  onRequest_rid

===========================================================================*/
/*!
    @brief
    Call from RIL instance RID0 to us to make a RIL_REQUEST
    Must be completed with a call to RIL_onRequestComplete()
    RIL_onRequestComplete() may be called from any thread, before or after
    this function returns.
    Returning from this routine implies the radio is ready to process another
    command (whether or not the previous command has completed).

    @return
    None.
*/
/*=========================================================================*/
static void onRequest_rid
(
  int request,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
  /*-----------------------------------------------------------------------*/

  onRequest( qmi_ril_process_instance_id, request, data, datalen, t );

} /* onRequest_rid0 */


/*===========================================================================

  FUNCTION:  qcril_process_event

===========================================================================*/
/*!
    @brief
    Dispatches all RIL events except RIL_REQUESTs, which are handled by
    onRequest.  This routine may be called from multiple threads but it
    blocks on a single semaphore that is shared with onRequest such that
    only one event is handled at a time by the RIL.

    @return
    E_SUCCESS of the event was handled normally
    E_NOT_SUPPORTED if the event_id was invalid
    E_NOT_ALLOWED if the event is not supported in the current state

*/
/*=========================================================================*/
IxErrnoType qcril_process_event
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
  IxErrnoType err_no;
  qcril_request_params_type params;
  qcril_dispatch_table_entry_type *entry_ptr; /*!< Dispatch table entry */
  char event_name[ 100 ] = "";
  qcril_request_resp_params_type  resp;

  /*-----------------------------------------------------------------------*/

  do
  {
  QCRIL_SNPRINTF( event_name, sizeof( event_name ), "%s(%d), RID %d, MID %d", qcril_log_lookup_event_name( (int) event_id ),
                  event_id, instance_id, modem_id );

  if (
       ( ( event_id > QCRIL_EVT_MMGSDI_INTERNAL_BASE ) && ( event_id < QCRIL_EVT_MMGSDI_MAX ) ) ||
       ( ( event_id > QCRIL_EVT_GSTK_INTERNAL_BASE ) && ( event_id < QCRIL_EVT_GSTK_MAX ) ) ||
       ( ( event_id > QCRIL_EVT_PBM_INTERNAL_BASE ) && ( event_id < QCRIL_EVT_PBM_MAX ) ) )
  {
    /* Internal events */
    QCRIL_LOG_CF_PKT_RIL_EVT( instance_id, event_name );
    QCRIL_LOG_DEBUG( "RIL --- %s ---> RIL", event_name );
  }
  else
  {
    /* AMSS events or callbacks */
    QCRIL_LOG_CF_PKT_MODEM_EVT( modem_id, event_name );

    QCRIL_LOG_DEBUG( "RIL <--- %s --- AMSS", event_name, event_id, instance_id, modem_id );
  }

  if (event_id <= QCRIL_EVT_BASE)
  {
    /* The event_id is out of range */
      err_no =  (E_NOT_SUPPORTED);
      break;
  }

  /* Do a lookup for the entry */
  if ((qcril_hash_table_lookup( (uint32) event_id, &entry_ptr ) != E_SUCCESS) || (entry_ptr == NULL))
  {
    /* The request is not supported */
      err_no =  (E_NOT_SUPPORTED);
      break;
  }

  /* Initialize the input parameters for the request handler.  This allows for
     easy addition or deletion of parameters in the future. */
  params.instance_id = instance_id;
  params.modem_id = modem_id;
  params.event_id = (int) event_id;
  params.data = data;
  params.datalen = datalen;
  params.t = t;

  err_no = qcril_dispatch_event( entry_ptr, &params );

  QCRIL_LOG_INFO( "Exit %s, err_no %d", qcril_log_lookup_event_name((int) event_id ), err_no );
  if ( E_SUCCESS != err_no )
  {
    if ( params.event_id > QCRIL_EVT_IMS_SOCKET_REQ_BASE && params.event_id < QCRIL_EVT_IMS_SOCKET_REQ_MAX )
    {
      qcril_qmi_ims_socket_send(params.t, IMS__MSG_TYPE__RESPONSE, qcril_qmi_ims_map_event_to_request(params.event_id), qcril_qmi_ims_map_ril_error_to_ims_error(err_no), NULL, 0);
    }
    else if ( params.event_id > QCRIL_EVT_HOOK_BASE && params.event_id < QCRIL_EVT_HOOK_MAX )
    {
      qcril_default_request_resp_params( instance_id, params.t, params.event_id, err_no, &resp );
      qcril_send_request_response( &resp );
    }
  }
  }while(0);

  return err_no;

} /* qcril_process_event() */


//===========================================================================
//qmi_ril_get_operational_status
//===========================================================================
qmi_ril_gen_operational_status_type qmi_ril_get_operational_status(void)
{
  qmi_ril_gen_operational_status_type res;

  qmi_ril_enter_critical_section();
  res = qmi_ril_gen_operational_status;
  qmi_ril_leave_critical_section();

  return res;
} //qmi_ril_get_operational_status
//===========================================================================
//qmi_ril_set_operational_status
//===========================================================================
void qmi_ril_set_operational_status( qmi_ril_gen_operational_status_type new_status )
{
  QCRIL_LOG_INFO( "new_status %d", new_status );

  qmi_ril_enter_critical_section();
  qmi_ril_gen_operational_status = new_status;
  qmi_ril_leave_critical_section();
} // qmi_ril_set_operational_status
/*===========================================================================

  FUNCTION:  currentState

===========================================================================*/
/*!
    @brief
    Return current radio state of an instance.

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static RIL_RadioState currentState
(
  qcril_instance_id_e_type instance_id
)
{
  char label[ 300 ];
  char * radio_state_name;
  RIL_RadioState radio_state;
  qmi_ril_gen_operational_status_type current_state;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  current_state = qmi_ril_get_operational_status();
  switch (current_state)
  {
    case QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_ONGOING: // fallthrough
    case QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED: // fallthrough
    case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING:
      radio_state = qcril_qmi_nas_dms_get_current_power_state( instance_id );
      break;

    default:
      radio_state = RADIO_STATE_UNAVAILABLE;
      break;
  }

  if (qcril_qmi_nas_modem_power_is_mdm_shdn_in_apm() &&
      0 == qcril_qmi_modem_power_voting_state() &&
      qcril_qmi_modem_power_is_voting_feature_supported())
  {
      radio_state = RADIO_STATE_OFF;
      QCRIL_LOG_INFO("setting RADIO STATE OFF");
  }

  switch ( radio_state )
  {
    case RADIO_STATE_OFF:
      radio_state_name = "Radio Off";
      break;

    case RADIO_STATE_UNAVAILABLE:
      radio_state_name = "Radio Unavailable";
      break;

    case RADIO_STATE_ON:
      radio_state_name = "Radio On";
      break;

    default:
      radio_state_name = "<unspecified>";
      break;
  }

  QCRIL_LOG_DEBUG( "currentState() -> %s(%d)", radio_state_name, radio_state );

  QCRIL_SNPRINTF( label, sizeof( label ), "currentState() - %s", radio_state_name );
  QCRIL_LOG_CF_PKT_RIL_FN( instance_id, label );

  return radio_state;

} /* currentState() */


/*===========================================================================

  FUNCTION:  currentState_rid

===========================================================================*/
/*!
    @brief
    Synchronous call from the RIL to us to return current radio state of
    RIL instance RID0.
    RADIO_STATE_UNAVAILABLE should be the initial state.

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static RIL_RadioState currentState_rid
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  return currentState( QCRIL_DEFAULT_INSTANCE_ID );

} /* currentState_rid0() */


/*===========================================================================

  FUNCTION:  onSupports

===========================================================================*/
/*!
    @brief
    Determines if the RIL supports the given RIL_REQUEST by a RIL instance

    @return
    1 if the given RIL_REQUEST is supported
*/
/*=========================================================================*/
static int onSupports
(
  qcril_instance_id_e_type instance_id,
  int request
)
{
  int supported = 1;
  char label[ 80 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  /* Do a lookup for the entry */
  if ( qcril_hash_table_lookup( (uint32) request, NULL ) != E_SUCCESS )
  {
    supported = 0;
  }

  QCRIL_LOG_DEBUG( "onSupports() ->: %s (%d), %s", qcril_log_lookup_event_name( request ), request,
                   (supported>0)?"Supported":"Not Supported");

  QCRIL_SNPRINTF( label, sizeof( label ), "onSupports() -` %s %s",
                  qcril_log_lookup_event_name( request ), ( supported > 0 ) ? "Supported" : "Not Supported" );
  QCRIL_LOG_CF_PKT_RIL_FN( instance_id, label );

  return supported;

} /* onSupports() */


/*===========================================================================

  FUNCTION:  onSupports_rid

===========================================================================*/
/*!
    @brief
    Determines if the RIL supports the given RIL_REQUEST by
    RIL instance RID0.

    @return
    1 if the given RIL_REQUEST is supported
*/
/*=========================================================================*/
static int onSupports_rid
(
  int request
)
{
  /*-----------------------------------------------------------------------*/

  return onSupports( QCRIL_DEFAULT_INSTANCE_ID, request );

} /* onSupports_rid0() */


/*===========================================================================

  FUNCTION:  onCancel

===========================================================================*/
/*!
    @brief
    The RIL is supposed to make a "best effort" to cancel the outstanding
    RIL_REQUEST with the given RIL_Token.  For now the hope is that out
    "best effort" can be "no effort".

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static void onCancel
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_NOTUSED( t );

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_LOG_CF_PKT_RIL_FN( instance_id, "onCancel()" );

} /* onCancel() */


/*===========================================================================

  FUNCTION:  onCancel_rid

===========================================================================*/
/*!
    @brief
    The RIL is supposed to make a "best effort" to cancel the outstanding
    RIL_REQUEST with the given RIL_Token for RIL instance RID0.
    For now the hope is that out "best effort" can be "no effort".

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static void onCancel_rid
(
  RIL_Token t
)
{
  /*-----------------------------------------------------------------------*/

  onCancel( QCRIL_DEFAULT_INSTANCE_ID, t );

} /* onCancel_rid0()*/


/*===========================================================================

  FUNCTION:  getVersion

===========================================================================*/
/*!
    @brief
    Used to query what version of the RIL is present

    @return
    A string describing this RIL version
*/
/*=========================================================================*/
static const char *getVersion
(
  qcril_instance_id_e_type instance_id
)
{
  char *version = "Qualcomm RIL 1.0";

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/


  QCRIL_LOG_DEBUG( "version %s", version );

  QCRIL_LOG_CF_PKT_RIL_FN( instance_id, "getVersion() - Qualcomm RIL 1.0 (QMI)" );

  return version;

} /* getVersion() */


/*===========================================================================

  FUNCTION:  getVersion_rid

===========================================================================*/
/*!
    @brief
    Used to query what version of the RIL is present in RID0

    @return
    A string describing this RIL version
*/
/*=========================================================================*/
static const char *getVersion_rid
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, qmi_ril_bootup_actition_on_rild_atel_link_connect, NULL, NULL );

  return getVersion( QCRIL_DEFAULT_INSTANCE_ID );

} /* getVersion_rid0() */

/*===========================================================================

  FUNCTION:  qcril_init_hash_table

===========================================================================*/
/*!
    @brief
    Initializes the hash table of events

    @return
    None.
*/
/*=========================================================================*/
static void qcril_init_hash_table( void )
{
  uint32 reg_index, hash_index; /*!< index into hash table */
  qcril_dispatch_table_entry_type *temp_entry_ptr;

  /*-----------------------------------------------------------------------*/

  /* Initialize hash table */
  for (reg_index = 0; reg_index < QCRIL_ARR_SIZE( qcril_event_table ); reg_index++)
  {
    hash_index = qcril_hash( qcril_event_table[reg_index].event_id,
                             QCRIL_HT_ENTRIES_MAX, 0 );

    if(hash_index < QCRIL_HT_ENTRIES_MAX)
    {
      if (qcril_hash_table[hash_index] == NULL)
      {
        /* No collision, just assign the new entry */
        qcril_hash_table[hash_index] = &qcril_event_table[reg_index];
      }
      else
      {
        /* Link the entry at the end of the collision list */
        temp_entry_ptr = qcril_hash_table[hash_index];

        while (temp_entry_ptr->next_ptr != NULL)
        {
          temp_entry_ptr = temp_entry_ptr->next_ptr;
        }
        temp_entry_ptr->next_ptr = &qcril_event_table[reg_index];
      }
    }
  }

} /* qcril_init_hash_table() */


/*===========================================================================

  FUNCTION:  qcril_delay_timed_cb

===========================================================================*/
/*!
    @brief
    Handle delay timer expiration.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_delay_timed_cb
(
  void *param
)
{
  QCRIL_LOG_DEBUG( "Delay Timer expired with ID %d", (uint32)(uintptr_t) param );

}; /* qcril_delay_timed_cb */


/*===========================================================================

  FUNCTION:  qcril_init_state

===========================================================================*/
/*!
    @brief
    Initialize states of QCRIL.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_init_state
(
  void
)
{
  qcril_instance_id_e_type instance_id;
  uint32 timer_id;
  qcril_arb_state_info_struct_type *s_ptr;
  int len;
  char args[ PROPERTY_VALUE_MAX ];
  char *end_ptr;
  unsigned long ret_val;

  /*-----------------------------------------------------------------------*/

  /* Initialize TIMER ID */
  qcril_timer_id = 1;

  /* Allow cache */
  qcril_state = (qcril_arb_state_struct_type *) qcril_arb_allocate_cache( QCRIL_ARB_CACHE_STATE );

  if(qcril_state != NULL)
  {
  /* Initialize Timer ID mutex */
  pthread_mutex_init( &qcril_state->mutex, NULL );

  /* initialize Timed Callback list */
  pthread_mutex_init( &qcril_timed_callback_list_mutex, NULL);
  qcril_timed_callback_list = NULL;

  pthread_mutex_init( &qmi_ril_android_pending_unsol_resp.pending_unsol_resp_mutex, NULL);

  /* conditionally initialize heap memory tracker list*/
  is_heap_memory_tracked = 0;
  property_get( QCRIL_TRACK_HEAP_MEM, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "Fail to convert QCRIL_TRACK_HEAP_MEM %s", QCRIL_TRACK_HEAP_MEM );
    }
    else if ( ret_val > 1 )
    {
      QCRIL_LOG_ERROR( "Invalid saved QCRIL_TRACK_HEAP_MEM %ld, use default", ret_val );
    }
    else
    {
      is_heap_memory_tracked = ( uint8 ) ret_val;
    }
  }
  if ( 1 == is_heap_memory_tracked )
  {
    pthread_mutex_init( &qcril_heap_memory_list_mutex, NULL);
    qcril_heap_memory_list = NULL;
    heap_memory_id = 0;
    qcril_setup_timed_callback(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                               qcril_print_heap_memory_list, &HEAP_MEM_LIST_PRINT_TIMEVAL_DELAY, NULL );
  }

  memset( &qmi_ril_common_critical_section, 0, sizeof( qmi_ril_common_critical_section ) );
  pthread_mutexattr_init( &qmi_ril_common_critical_section.lock_mtx_atr );
  pthread_mutex_init( &qmi_ril_common_critical_section.lock_mutex, &qmi_ril_common_critical_section.lock_mtx_atr );

  pthread_mutexattr_init (&esoc_info.mdm_attr);
  pthread_mutex_init (&esoc_info.mdm_mutex, &esoc_info.mdm_attr);

  /* Initialize internal data */
  for ( instance_id = 0; instance_id < QCRIL_MAX_INSTANCE_ID; instance_id++ )
  {
    s_ptr = &qcril_state->info[ instance_id ];


    /* Stay in GW SIM Not Ready State till Modem reports an update on GW SIM State */
    s_ptr->pri_gw_sim_state = QCRIL_SIM_STATE_NOT_READY;
    s_ptr->sec_gw_sim_state = QCRIL_SIM_STATE_NOT_READY;

    /* Stay in CDMA SIM Not Ready State till Modem reports an update on CDMA SIM State */
    s_ptr->pri_cdma_sim_state = QCRIL_SIM_STATE_NOT_READY;
    s_ptr->sec_cdma_sim_state = QCRIL_SIM_STATE_NOT_READY;

    /* No RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED needed to be reported at power-up.
    ** RILD itself will invoke the unsol event when it's done register and query
    ** QCRIL for the Radio State. */
  }

  /* This is a workaround for a bug in ril.cpp where it starts ril_event_loop()
     before adding s_fdlisten to readFds.  When ril_event_loop first calls
     select() it is only waiting on s_fdWakeupRead.  Setting this timer wakes up
     the select, and when it blocks again s_fdlisten is in the fd_set.  Otherwise
     ril_event_loop() is blocked forever, even if Java connects to the socket. */
  qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, qcril_delay_timed_cb,
                                         &TIMEVAL_DELAY, NULL );
  }
  else
  {
    QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
  }

} /* qcril_init_state */


/*===========================================================================

  FUNCTION:  qcril_init

===========================================================================*/
/*!
    @brief
    Initializes all QCRIL modules

    @return
    None.
*/
/*=========================================================================*/
void qcril_init
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  /* Initialize the Arbitration module. Should be done before any other initialization */
  qcril_arb_init();

  /* Initialize QCRIL states */
  qcril_init_state();

  // init oem handling fw
  qmi_ril_oem_hook_init();

  qcril_db_init();

  /* ###############################################################################################
        !!!IMPORTANT!!!
  ##################################################################################################

   (1) Use the state mutex to block QCRIL states update that could possibily triggered by any AMSS
       command callback, or AMSS event before the completion of radio state initialization.

   (2) Don't call qcril_process_event inside this block. Doing so, will end up in mutex deadlock.

  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                                            QCRIL STATES INITIALIZATION BEGIN
  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */


  QCRIL_MUTEX_LOCK( &qcril_state->mutex, "qcril_state_mutex" );

  /* Initialize QCRIL internal data structures */
  qcril_init_hash_table();

  qcril_reqlist_init();

  QCRIL_MUTEX_UNLOCK( &qcril_state->mutex, "qcril_state_mutex" );

  /* initialize supress list */
  qcril_request_suppress_list_init();

  /* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                                         QCRIL STATES INITIALIZATION END
     <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

  qmi_ril_qmi_client_pre_initialization_init();
  qmi_ril_qmi_client_pre_initialization_acquire();

  // per technology pre-init
  qcril_qmi_nas_dms_commmon_pre_init();
  qcril_qmi_voice_pre_init();
  qcril_ims_flow_control_pre_init();
#ifndef QMI_RIL_UTF
  qcril_am_pre_init();
#else
  qmi_ril_rat_enable_option = QMI_RIL_FTR_RAT_UNKNOWN;
  qmi_ril_baseband_ftr_info = QMI_RIL_FTR_BASEBAND_UNKNOWN;
#endif
  qcril_qmi_imsa_pre_init();
  qcril_qmi_sms_pre_init();

#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
  qcril_uim_remote_client_socket_init();
#endif
#ifdef FEATURE_QCRIL_UIM_REMOTE_SERVER
  qcril_uim_remote_server_socket_init();
#endif
  QCRIL_LOG_FUNC_RETURN();

} /* qcril_init() */

//===========================================================================
//qmi_ril_initiate_bootup
//===========================================================================
void qmi_ril_initiate_bootup(void)
{
  qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, qmi_ril_bootup_perform_core_or_start_polling, NULL, NULL );
} // qmi_ril_initiate_bootup

//===========================================================================
// qmi_ril_oem_hook_init
//===========================================================================
void qmi_ril_oem_hook_init()
{
  memset( &qmi_ril_oem_hook_overview, 0, sizeof( qmi_ril_oem_hook_overview ) );

  pthread_mutexattr_init( &qmi_ril_oem_hook_overview.overview_lock_mtx_atr );
  pthread_mutex_init( &qmi_ril_oem_hook_overview.overview_lock_mutex, &qmi_ril_oem_hook_overview.overview_lock_mtx_atr );
  qmi_ril_oem_hook_overview.qmi_ril_oem_hook_qmi_tunneling_reqs_root = NULL;
} // qmi_ril_oem_hook_init

//===========================================================================
//qmi_ril_bootup_perform_core_or_start_polling
//===========================================================================
void qmi_ril_bootup_perform_core_or_start_polling(void * params)
{
  RIL_Errno init_res;

  int                          ril_version;
  qcril_unsol_resp_params_type unsol_resp;

  qmi_ril_main_thread_id = pthread_self();

  qmi_ril_set_thread_name( qmi_ril_fw_get_main_thread_id(), QMI_RIL_QMI_MAIN_THREAD_NAME);
  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( params );

  qmi_ril_wave_modem_status(); // this should result in "modem unavailble" report

  qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_PENDING ); // for consistency

  qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_ONGOING );
  init_res = qmi_ril_core_init();
  QCRIL_LOG_INFO("sees %d from qmi_ril_core_init()", (int)init_res );
  if ( RIL_E_SUCCESS == init_res )
  {
    qmi_ril_wave_modem_status(); // this should trigger reporting of modem state to Android
    qmi_ril_core_init_enter_warp();
  }
  else
  {
    qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_PENDING );
    qmi_ril_initiate_core_init_retry();
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_bootup_perform_core_or_start_polling
//qmi_ril_core_init
//===========================================================================
RIL_Errno qmi_ril_core_init(void)
{
  RIL_Errno res = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_event_suspend(); // to ensure atomic init flow cross sub domains
  do
  {
    res = qcril_qmi_client_init();
    if ( RIL_E_SUCCESS != res )
      break;

    qcril_other_init();

    qcril_uim_init();

    qcril_gstk_qmi_init();

    qcril_uim_remote_init();

#ifndef QMI_RIL_UTF
    qcril_data_init();
#endif

    qcril_qmi_nas_dms_commmon_post_init();

    if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_OEM_SOCKET))
    {
      QCRIL_LOG_INFO( "%s Init OEM socket thread", __FUNCTION__ );
      qcril_qmi_oem_socket_init();
    }

  } while (FALSE);
  qcril_event_resume();

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

  return res;
} // qmi_ril_core_init
//===========================================================================
//qmi_ril_core_init_enter_warp
//===========================================================================
void qmi_ril_core_init_enter_warp()
{
  qmi_ril_gen_operational_status_type cur_status;

  QCRIL_LOG_FUNC_ENTRY();

  if ( !qmi_ril_is_multi_sim_feature_supported() )
  {
      QCRIL_LOG_INFO( "!QMI RIL! 2nd phase init for NON - DSDS" );

      cur_status = qmi_ril_get_operational_status();
      // Do not set the state to UNRESTRICTED if RIL is currently being SUSPENDED.
      if ( !( cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDING ||
                cur_status == QMI_RIL_GEN_OPERATIONAL_STATUS_SUSPENDED ) )
      {
          qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED );
          (void)qcril_sms_perform_initial_configuration(); // SMS initialization done right away

          QCRIL_LOG_INFO( "!QMI RIL! 2nd phase SMS init for NON - DSDS" );
      }
  } // otherwise SMS init will be performed latyer once sub is activated
  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_core_init_enter_warp

//===========================================================================
//qmi_ril_initiate_core_init_retry
//===========================================================================
RIL_Errno qmi_ril_initiate_core_init_retry(void)
{
  RIL_Errno res = RIL_E_GENERIC_FAILURE;
  pthread_attr_t attr;
  int conf;

#ifdef QMI_RIL_UTF
  pthread_attr_init (&attr);
  conf = utf_pthread_create_handler(&qmi_ril_init_retry_thread_pid, &attr, qmi_ril_core_init_kicker_thread_proc, NULL);
#else
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  conf = pthread_create(&qmi_ril_init_retry_thread_pid, &attr, qmi_ril_core_init_kicker_thread_proc, NULL);
#endif
  qmi_ril_set_thread_name(qmi_ril_init_retry_thread_pid, QMI_RIL_CORE_INIT_KICKER_THREAD_NAME);

  pthread_attr_destroy(&attr);

  res =  (conf < 0 ) ? RIL_E_GENERIC_FAILURE : RIL_E_SUCCESS;
  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

  return res;
} // qmi_ril_initiate_core_init_retry

//===========================================================================
//qmi_ril_core_init_kicker_main_threaded_proc
//===========================================================================
void qmi_ril_core_init_kicker_main_threaded_proc(void* empty_param)
{
  RIL_Errno core_init_res = RIL_E_GENERIC_FAILURE;
  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( empty_param );
  qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_ONGOING );
  core_init_res = qmi_ril_core_init();
  QCRIL_LOG_INFO( "iteration - %d", (int) core_init_res );

  if ( RIL_E_SUCCESS == core_init_res )
  {
    qmi_ril_core_init_enter_warp();
    qmi_ril_wave_modem_status(); // this should trigger reporting of modem state to Android
  }
  else
  {
    qmi_ril_set_operational_status( QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_PENDING );
    qmi_ril_initiate_core_init_retry();
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_core_init_kicker_main_threaded_proc
//===========================================================================
//qmi_ril_core_init_kicker_thread_proc
//===========================================================================
void * qmi_ril_core_init_kicker_thread_proc(void* empty_param)
{
  RIL_Errno core_init_res = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( empty_param );
  sleep( QMI_RIL_INI_RETRY_GAP_SEC );
  qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, qmi_ril_core_init_kicker_main_threaded_proc, NULL, NULL );

  QCRIL_LOG_FUNC_RETURN();

  qmi_ril_clear_thread_name(pthread_self());
  return NULL;
}  // qmi_ril_core_init_kicker_thread_proc

/*===========================================================================

  FUNCTION:  qcril_release

===========================================================================*/
/*!
    @brief
    Release AMSS client objects.

    @return
    None.
*/
/*=========================================================================*/
void qcril_release
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  qcril_uim_release();
  qcril_uim_remote_release();

  /* For QMI_VOICE, NAS/DMS, WMS */
  qcril_qmi_client_release();

  if ( qmi_ril_is_multi_sim_feature_supported() )
  {
      qcril_ipc_release();
  }
  qcril_log_cleanup();
  if (qcril_qmi_is_pm_voting_feature_supported())
  {
    qmi_ril_peripheral_mng_deinit();
  }
} /* qcril_release()*/


/*===========================================================================

  FUNCTION:  qcril_malloc_adv

===========================================================================*/
/*!
    @brief
    Allocate memory from heap.

    @return
    Pointer to allocated memory region.
*/
/*=========================================================================*/
void *qcril_malloc_adv
(
  size_t size,
  const char* func_name,
  int line_num
)
{
  void *mem_ptr;

  /*-----------------------------------------------------------------------*/

  if ( 1 == is_heap_memory_tracked )
  {
    mem_ptr = malloc(size + 2 * MEM_PADDING_SIZE);

    if ( NULL != mem_ptr )
    {
      qcril_heap_list_info *heap_memory_info = malloc(sizeof(*heap_memory_info));
      if ( NULL != heap_memory_info )
      {
        memcpy(mem_ptr, MEM_PADDING, MEM_PADDING_SIZE);
        mem_ptr = (void*)((char*)mem_ptr + MEM_PADDING_SIZE);
        memset( mem_ptr, 0, size );
        memcpy((void*)((char*)mem_ptr+size), MEM_PADDING, MEM_PADDING_SIZE);

        heap_memory_info->addr = mem_ptr;
        strlcpy(heap_memory_info->func_name, func_name,
                (sizeof(heap_memory_info->func_name) / sizeof(heap_memory_info->func_name[0])));
        heap_memory_info->line_number = line_num;
        heap_memory_info->size = size;
        QCRIL_MUTEX_LOCK( &qcril_heap_memory_list_mutex, "heap memory list mutex");
        heap_memory_info->mem_id = heap_memory_id++;
        QCRIL_LOG_INFO( "malloc new memory: id: %d, func: %s", heap_memory_info->mem_id, heap_memory_info->func_name );
        QCRIL_MUTEX_UNLOCK( &qcril_heap_memory_list_mutex, "heap memory list mutex");

        QCRIL_LOG_INFO("Adding new addr %p to memory list", mem_ptr);
        qcril_add_heap_memory_to_list(heap_memory_info);
      }
      else
      {
        memset( mem_ptr, 0, size );
        QCRIL_LOG_ERROR( "Fail to allocate memory" );
      }
    }
    else
    {
      QCRIL_LOG_ERROR( "Fail to allocate memory" );
    }
  }
  else
  {
    mem_ptr = malloc( size );
    if ( NULL != mem_ptr )
    {
      memset( mem_ptr, 0, size );
    }
    else
    {
      QCRIL_LOG_ERROR( "Fail to allocate memory" );
    }
  }

  return mem_ptr;

} /* qcril_malloc_adv */


/*===========================================================================

  FUNCTION:  qcril_free_adv

===========================================================================*/
/*!
    @brief
    Free specified memory region.

    @return
    None.
*/
/*=========================================================================*/
void qcril_free_adv
(
  void *mem_ptr,
  const char* func_name,
  int line_num
)
{

  /*-----------------------------------------------------------------------*/

  if ( mem_ptr == NULL )
  {
      return;
  }

  /*-----------------------------------------------------------------------*/

  if ( 1 == is_heap_memory_tracked )
  {
    QCRIL_LOG_INFO("removing heap memory: %p from list", mem_ptr);
    qcril_heap_list_info *heap_mem_info = qcril_remove_heap_memory_from_list(mem_ptr);
    if (NULL == heap_mem_info)
    {
      QCRIL_LOG_ERROR("Memory %p not found in list. Called from %s line %d", mem_ptr, func_name, line_num);
    }
    else
    {
      // check buffer underflow
      if ( 0 != memcmp((void*)(((char*)mem_ptr) - MEM_PADDING_SIZE), (void*)MEM_PADDING, MEM_PADDING_SIZE ) )
      {
        QCRIL_LOG_FATAL("memory underflow!");
      }
      // check buffer overflow
      if ( 0 != memcmp((void*)((char*)(mem_ptr) + heap_mem_info->size), (void*)MEM_PADDING, MEM_PADDING_SIZE) )
      {
        QCRIL_LOG_FATAL("memory overflow!");
      }

      mem_ptr = (void*)(((char*)mem_ptr) - MEM_PADDING_SIZE);
      free(heap_mem_info);
    }
    if (NULL != mem_ptr)
    {
      free( mem_ptr );
    }
    else
    {
      QCRIL_LOG_DEBUG("Memory pointer is NULL after shifting the padding size");
    }
  }
  else
  {
    if (NULL != mem_ptr)
    {
      free( mem_ptr );
    }
  }

  return;
} /* qcril_free_adv */

//===========================================================================
// qcril_get_empty_binary_data_type
//===========================================================================
qcril_binary_data_type qcril_get_empty_binary_data_type()
{
    qcril_binary_data_type res;
    res.len = 0;
    res.data = NULL;
    return res;
} // qcril_get_empty_binary_data_type

//===========================================================================
// qcril_is_binary_data_empty
//===========================================================================
boolean qcril_is_binary_data_empty(qcril_binary_data_type bin_data)
{
    return !bin_data.len || !bin_data.data;
} // qcril_is_binary_data_empty

//===========================================================================
// qcril_find_pattern
//===========================================================================
qcril_binary_data_type qcril_find_pattern(qcril_binary_data_type bin_data, const char *pattern)
{
    int i;
    for (i = 0; i < bin_data.len - strlen(pattern); ++i)
    {
        boolean match = TRUE;
        int j;
        for (j = 0; j < strlen(pattern); ++j)
        {
            if (bin_data.data[i + j] != pattern[j])
            {
                match = FALSE;
                break;
            }
        }
        if (match)
        {
            bin_data.data += i;
            bin_data.len -= i;
            return bin_data;
        }
    }
    return qcril_get_empty_binary_data_type();
} // qcril_find_pattern

//===========================================================================
//qmi_ril_get_process_instance_id
//===========================================================================
qcril_instance_id_e_type qmi_ril_get_process_instance_id(void)
{
  return qmi_ril_process_instance_id; // this is always thread safe as may only be changed upon ril library load
} // qmi_ril_get_process_instance_id

//=============================================================================
// FUNCTION: qmi_ril_get_sim_slot
//
// DESCRIPTION:
// returns the sim card slot index associated with current RIL instance
// Note: thread safe as long as its only set durring ril init
//
// RETURN: 0 | 1 - sim card slot index
//=============================================================================
uint32_t qmi_ril_get_sim_slot(void)
{
  return qmi_ril_sim_slot;
}

//=============================================================================
// FUNCTION: qmi_ril_get_stack_id
//
// DESCRIPTION:
// returns the modem stack id associated with current RIL instance
//
// RETURN: 0 | 1 | 2 - primary | secondary | tertiary stack id
//=============================================================================
qcril_modem_stack_id_e_type qmi_ril_get_stack_id
(
qcril_instance_id_e_type instance_id
)
{
  return qcril_qmi_nas_get_modem_stack_id();
}

//===========================================================================
//qmi_ril_enter_critical_section
//===========================================================================
void qmi_ril_enter_critical_section(void)
{
  pthread_mutex_lock( &qmi_ril_common_critical_section.lock_mutex );
} // qmi_ril_enter_critical_section
//===========================================================================
//qmi_ril_leave_critical_section
//===========================================================================
void qmi_ril_leave_critical_section(void)
{
  pthread_mutex_unlock( &qmi_ril_common_critical_section.lock_mutex );
} // qmi_ril_leave_critical_section

/*===========================================================================

  FUNCTION: qcril_qmi_load_esoc_and_register_with_pm

===========================================================================*/
/*!
    @brief
    Loads esoc info

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_load_esoc_and_register_with_pm
(
    void
)
{
    char* esoc_modem_name;
    qcril_qmi_load_esoc_info();
    esoc_modem_name = qcril_qmi_get_esoc_modem_name();
    QCRIL_LOG_INFO("load_esoc_and_register_with_pm: modem_name = %s",
                   ((NULL != esoc_modem_name)? esoc_modem_name: "NULL"));
    if (NULL != esoc_modem_name && !qmi_ril_peripheral_mng_init(esoc_modem_name))
    {
        QCRIL_LOG_INFO("peripheral manager feature is enabled");
        esoc_info.pm_feature_supported = TRUE;
    }
}

/*===========================================================================

  FUNCTION: qcril_qmi_load_esoc_info

===========================================================================*/
/*!
    @brief
    Loads esoc info

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_load_esoc_info
(
    void
)
{
    struct dev_info devinfo;

    do {
        if (get_system_info(&devinfo) != RET_SUCCESS)
        {
            QCRIL_LOG_ERROR("Could not retrieve esoc info");
            break;
        }

        if (devinfo.num_modems != 1)
        {
            QCRIL_LOG_ERROR("Unexpected number of modems %d",
                             devinfo.num_modems);
            break;
        }

        /* Read esoc node, to be used if
         * peripheral manager is not supported */
        strlcpy(esoc_info.esoc_node,
                devinfo.mdm_list[0].powerup_node,
                sizeof(esoc_info.esoc_node));

        /* Read modem name, to be used to register with
         * peripheral manager */
        strlcpy(esoc_info.modem_name,
                devinfo.mdm_list[0].mdm_name,
                sizeof(esoc_info.modem_name));

        /* Read link name to find out the transport medium
         * to decide on qmi port */
        strlcpy(esoc_info.link_name,
                devinfo.mdm_list[0].mdm_link,
                sizeof(esoc_info.link_name));

        QCRIL_LOG_INFO("Read esoc info: modem name: %s, "
                       "link name: %s, esoc_node: %s",
                       esoc_info.modem_name, esoc_info.link_name,
                       esoc_info.esoc_node);

        esoc_info.type = devinfo.mdm_list[0].type;
        QCRIL_LOG_INFO("Mdm type (0-External, 1-Internal):%d",
                        esoc_info.type);

    } while (0);

    return;
}

/*===========================================================================

  function: qcril_qmi_get_esoc_link_name

===========================================================================*/
/*!
    @brief
    returns esoc mdm link name

    @return
    esoc device node link name
*/
/*=========================================================================*/
char *qcril_qmi_get_esoc_link_name
(
    void
)
{
    char *link_name = NULL;
    if (strlen(esoc_info.link_name) > 0)
    {
        link_name = esoc_info.link_name;
    }

    return link_name;
}
/*===========================================================================

  function: qcril_qmi_get_esoc_modem_name

===========================================================================*/
/*!
    @brief
    returns esoc modem name

    @return
    esoc modem name
*/
/*=========================================================================*/
char *qcril_qmi_get_esoc_modem_name
(
    void
)
{
    char *modem_name= NULL;
    if (strlen(esoc_info.modem_name) > 0)
    {
        modem_name = esoc_info.modem_name;
    }

    return modem_name;
}

/*===========================================================================

  function: qcril_qmi_get_esoc_node_name

===========================================================================*/
/*!
    @brief
    returns esoc node name

    @return
    esoc node name
*/
/*=========================================================================*/
char *qcril_qmi_get_esoc_node_name
(
    void
)
{
    char *modem_name= NULL;
    if (strlen(esoc_info.esoc_node) > 0)
    {
        modem_name = esoc_info.esoc_node;
    }

    return modem_name;
}
/*===========================================================================

  FUNCTION: qcril_qmi_modem_power_process_regular_shutdown

===========================================================================*/
/*!
    @brief
        release vote on modem

    @return
        None
*/
/*=========================================================================*/
void qcril_qmi_modem_power_process_regular_shutdown
(
    void
)
{
    QCRIL_MDM_LOCK();
    if (qcril_qmi_is_pm_voting_feature_supported())
    {
        qmi_ril_peripheral_mng_release_vote();
    }
    else if (qcril_qmi_is_esoc_voting_feature_supported())
    {
        close (esoc_info.esoc_fd);
    }

    QCRIL_MDM_UNLOCK();

    if (qcril_qmi_is_pm_voting_feature_supported())
    {
        QCRIL_LOG_INFO("released vote for modem %s",
                        qcril_qmi_get_esoc_modem_name()?
                            qcril_qmi_get_esoc_modem_name() : "null");
    }
    else if (qcril_qmi_is_esoc_voting_feature_supported())
    {
        QCRIL_LOG_INFO("released vote for  node %s fd %d",
                        qcril_qmi_get_esoc_node_name()?
                            qcril_qmi_get_esoc_node_name() : "null",
                        esoc_info.esoc_fd);
    }

} // qcril_qmi_modem_power_process_regular_shutdown

/*===========================================================================

  FUNCTION: qcril_qmi_vote_for_modem_up_using_pm

===========================================================================*/
/*!
    @brief
        adds vote for modem through peripheral manager,
        so that pil is loaded.

    @return
        None
*/
/*=========================================================================*/
void qcril_qmi_vote_for_modem_up_using_pm
(
    void
)
{
    char   *esoc_modem_name;
    int     ret;

    QCRIL_LOG_FUNC_ENTRY();

    esoc_modem_name = qcril_qmi_get_esoc_modem_name();
    if (esoc_modem_name)
    {
        ret = qmi_ril_peripheral_mng_vote();
        if (ret)
        {
            QCRIL_LOG_ERROR("Could not vote for modem %s", esoc_modem_name);
        }
        else
        {
            esoc_info.voting_state = 1;
            QCRIL_LOG_INFO("vote activated for modem %s", esoc_modem_name);
        }
    }
    else
    {
        QCRIL_LOG_ERROR("ESOC node is not available");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_vote_for_modem_up_using_pm

/*===========================================================================

  FUNCTION: qcril_qmi_vote_for_modem_up_using_esoc

===========================================================================*/
/*!
    @brief
        adds vote for modem using esoc , so that pil is loaded.

    @return
        None
*/
/*=========================================================================*/
void qcril_qmi_vote_for_modem_up_using_esoc
(
    void
)
{
    char *esoc_node_name;

    QCRIL_LOG_FUNC_ENTRY();

    esoc_node_name = qcril_qmi_get_esoc_node_name();
    if (esoc_node_name)
    {
        if (!access (esoc_node_name, F_OK))
        {
            QCRIL_LOG_INFO("esoc feature is enabled");
            esoc_info.esoc_feature_supported = TRUE;
            if(esoc_info.type == MDM_TYPE_EXTERNAL)
            {
                if(NULL != esoc_node_name)
                {
                    esoc_info.esoc_fd = open(esoc_node_name, O_RDONLY);
                }

                if (RIL_VALID_FILE_HANDLE > esoc_info.esoc_fd)
                {
                    esoc_info.esoc_feature_supported = FALSE;
                    QCRIL_LOG_ERROR("Can not open file %s", esoc_node_name);
                }
                else
                {
                    esoc_info.voting_state = 1;
                    QCRIL_LOG_INFO("vote activated for node %s, fd %d",
                                    esoc_node_name, esoc_info.esoc_fd);
                }
            }
            else
            {
                esoc_info.esoc_feature_supported = FALSE;
                QCRIL_LOG_INFO("Internal modem - esoc file open not required");
            }
        }
        else
        {
            QCRIL_LOG_ERROR("ESOC node %s not accessible", esoc_node_name);
        }
    }
    else
    {
        QCRIL_LOG_ERROR("ESOC node is not available");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_vote_for_modem_up_using_esoc

/*===========================================================================

  FUNCTION: qcril_qmi_modem_power_process_bootup

===========================================================================*/
/*!
    @brief
        adds vote for modem, so that pil is loaded.

    @return
        None
*/
/*=========================================================================*/
void qcril_qmi_modem_power_process_bootup
(
    void
)
{
    char   *esoc_modem_name;
    int     ret;

    QCRIL_LOG_FUNC_ENTRY();

    qcril_qmi_nas_modem_power_load_apm_mdm_not_pwdn();
    if (qcril_qmi_is_pm_voting_feature_supported())
    {
        qcril_qmi_vote_for_modem_up_using_pm();
    }
    else
    {
        qcril_qmi_vote_for_modem_up_using_esoc();
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_modem_power_process_bootup

/*===========================================================================

  FUNCTION: qcril_qmi_is_esoc_voting_feature_supported

===========================================================================*/
/*!
    @brief
    Retrieve if esoc voting feature is supported

    @return
    TRUE or FALSE
*/
/*=========================================================================*/
boolean qcril_qmi_is_esoc_voting_feature_supported
(
    void
)
{
    boolean ret;
    ret = esoc_info.esoc_feature_supported;
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
} // qcril_qmi_is_esoc_voting_feature_supported

/*===========================================================================

    FUNCTION: qcril_qmi_modem_power_is_voting_feature_supported

===========================================================================*/
/*!
    @brief
    Retrieve if voting feature is supported

    @return
    TRUE or FALSE
*/
/*=========================================================================*/
boolean qcril_qmi_modem_power_is_voting_feature_supported
(
    void
)
{
    boolean ret;
    ret = (qcril_qmi_is_esoc_voting_feature_supported() ||
            qcril_qmi_is_pm_voting_feature_supported());
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
} // qcril_qmi_modem_power_is_voting_feature_supported

/*===========================================================================

  FUNCTION: qcril_qmi_is_pm_voting_feature_supported

===========================================================================*/
/*!
    @brief
    Retrieve if peripheral manager voting feature is supported

    @return
    TRUE or FALSE
*/
/*=========================================================================*/
boolean qcril_qmi_is_pm_voting_feature_supported
(
    void
)
{
    boolean ret;
    ret = esoc_info.pm_feature_supported;
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
} // qcril_qmi_is_pm_voting_feature_supported

int qcril_qmi_modem_power_voting_state()
{
    int ret;
    QCRIL_MDM_LOCK();
    ret = esoc_info.voting_state;
    QCRIL_MDM_UNLOCK();
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}

void qcril_qmi_modem_power_set_voting_state(int state)
{
    QCRIL_MDM_LOCK();
    esoc_info.voting_state = state;
    QCRIL_MDM_UNLOCK();
    QCRIL_LOG_INFO("voting state set to %d", state);
}

/*===========================================================================

  FUNCTION: qcril_qmi_modem_power_process_apm_off

===========================================================================*/
/*!
    @brief
    vote to power up modem

    @return
    none
*/
/*=========================================================================*/
void qcril_qmi_modem_power_process_apm_off
(
    void
)
{
    QCRIL_LOG_FUNC_ENTRY();
    char *esoc_modem_name = NULL;
    char *esoc_node_name  = NULL;
    int   ret = -1;

    QCRIL_MDM_LOCK();
    if (qcril_qmi_is_pm_voting_feature_supported())
    {
        esoc_modem_name = qcril_qmi_get_esoc_modem_name();
        ret = qmi_ril_peripheral_mng_vote();
        if (!ret)
        {
            esoc_info.voting_state = 1;
        }
    }
    else if (qcril_qmi_is_esoc_voting_feature_supported())
    {
        esoc_node_name = qcril_qmi_get_esoc_node_name();
        if (NULL != esoc_node_name)
        {
            esoc_info.esoc_fd = open(esoc_node_name, O_RDONLY);
            if (esoc_info.esoc_fd >= RIL_VALID_FILE_HANDLE)
            {
                esoc_info.voting_state = 1;
            }
        }
    }

    QCRIL_MDM_UNLOCK();

    if (esoc_info.voting_state)
    {
        QCRIL_LOG_INFO("vote activated for modem: %s, node: %s",
                        esoc_modem_name ? esoc_modem_name : "null",
                        esoc_node_name ? esoc_node_name : " null");
    }
    else
    {
        QCRIL_LOG_ERROR("Could not vote for modem: %s, node: %s",
                        esoc_modem_name ? esoc_modem_name : "null",
                        esoc_node_name ? esoc_node_name : " null");
    }

    QCRIL_LOG_FUNC_RETURN();
}

#ifdef RIL_SHLIB
/*===========================================================================

  FUNCTION:  RIL_Init

===========================================================================*/
/*!
    @brief
    Returns the current state of the RIL

    @return
    The current state of the RIL
*/
/*=========================================================================*/
const RIL_RadioFunctions *RIL_Init
(
  const struct RIL_Env *env,
  int argc,
  char **argv
)
{
  qmi_ril_set_thread_name( pthread_self() , QMI_RIL_QMI_RILD_THREAD_NAME);

  /* Ignore SIGPIPE signal so that rild does not crash
     even if the other end of the socket is closed abruptly. */
  signal(SIGPIPE, SIG_IGN);

  qcril_log_init();

  /* Load eSOC info and register with peripheral manager */
  qcril_qmi_load_esoc_and_register_with_pm();

  int client_id = 0;
#ifdef FEATURE_DSDA_RIL_INSTANCE
   #if (FEATURE_DSDA_RIL_INSTANCE == 2)
      #define RIL_INSTANCE "FEATURE_DSDA_RIL_INSTANCE 2"
   #else
      #define RIL_INSTANCE "FEATURE_DSDA_RIL_INSTANCE 1"
   #endif
  static const char ril_instance_str[] = RIL_INSTANCE;
  client_id = FEATURE_DSDA_RIL_INSTANCE - 1; // compile seperate libs for each instance
#endif
  int opt = -1;
  while ( -1 != (opt = getopt(argc, argv, "p:d:s:c:"))) {
     switch (opt) {
       case 'c':
        client_id = atoi(optarg);
        QCRIL_LOG_INFO( "RIL client opt: %d, running RIL_Init()", client_id);
        break;
       default:
        QCRIL_LOG_INFO("RIL client opt: %d not handled intreseted only -c option", opt);
        break;
     }
  }
  QCRIL_LOG_INFO( "RIL client : %d, running RIL_Init()", client_id);

  qmi_ril_process_instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qmi_ril_sim_slot = 0; // use 1st slot as default

  if ( client_id == 2 )
  { // 2nd RIL instance - 1, only for DSDS or DSDA
    qmi_ril_process_instance_id = QCRIL_SECOND_INSTANCE_ID;
    if ( qmi_ril_is_multi_sim_feature_supported() )
    {
       qmi_ril_sim_slot = QCRIL_SECOND_INSTANCE_ID; // use 2nd slot for DSDS/TSTS
    }
    else if ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDA)  ||
              qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDA2) )
    {
       qmi_ril_sim_slot = 0; // use 1st slot for DSDA
    }
    else
    {
       QCRIL_LOG_ERROR("Usupported configuration, can't determine sim slot");
    }
  }
  else if ( client_id == 3 )
  { // 3rd RIL instance, only for TSTS
    qmi_ril_process_instance_id = QCRIL_THIRD_INSTANCE_ID;
    if ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_TSTS) )
    {
       qmi_ril_sim_slot = QCRIL_THIRD_INSTANCE_ID; // use 3rd slot for TSTS
    }
    else
    {
       QCRIL_LOG_ERROR("Usupported configuration, can't determine sim slot");
    }
  }

  if ( qmi_ril_is_multi_sim_feature_supported() )
  {
      qcril_ipc_init();
  }

  QCRIL_LOG_DEBUG( "RIL %d, running RIL_Init()", qmi_ril_process_instance_id );

  argc = argc;
  argv = argv;

  // o maintain compatibility with data and UIM code which use instance_id and may respond on "second instance" context
  qcril_response_api[ QCRIL_DEFAULT_INSTANCE_ID ] = (struct RIL_Env *) env;
  qcril_response_api[ QCRIL_SECOND_INSTANCE_ID ] = (struct RIL_Env *) env;
  // TODO_TSTS: Check if this is required. Seems not required.
  qcril_response_api[ QCRIL_THIRD_INSTANCE_ID ] = (struct RIL_Env *) env;

  // flow control
  qmi_ril_fw_android_request_flow_control_init();

  // unsol response dispatchable control
  qmi_ril_init_android_unsol_resp_pending_list();

  // Initialize the event thread */
  qcril_event_init();

  // Initialize QCRIL
  qcril_init();

  // Start event thread
  qcril_event_start();

  core_handler_start();
  cri_core_start();
  util_timer_start();

  qcril_log_timer_init();

  // start bootup if applicable
  qmi_ril_initiate_bootup();

  return &qcril_request_api[ QCRIL_DEFAULT_INSTANCE_ID ];

} /* RIL_Init() */
#elif defined (PLATFORM_LTK)
int qcril_main()
{
  const RIL_RadioFunctions *rilapi = &qcril_api;

  /*-----------------------------------------------------------------------*/

  qcril_init();

} /* qcril_main() */
#else
int main(int argc, char *argv[])
{
  const RIL_RadioFunctions *rilapi = &qcril_api;

  /*-----------------------------------------------------------------------*/
  QCRIL_NOTUSED( argv );
  QCRIL_NOTUSED( argc );

  qcril_init();


  /* Verify QCRIL access function */
  qcril_unit_test( rilapi );

  /* Verify QCRIL Powerup Unit Test Suite */
  qcril_powerup_unit_test();

  /* Verify QCRIL State Transition Test Suite */
  qcril_state_transition_unit_test();

  /* Run ReqList Unit Test Suite */
  qcril_reqlist_unit_test();

  /* Run CM CList Unit Test Suite */
  qcril_cm_clist_unit_test();

  /* Run CM Phone Services Unit Test Suite */
  qcril_cm_phonesvc_unit_test( rilapi );

  /* Run CM Serving System Services Unit Test Suite */
  qcril_cm_srvsys_unit_test( rilapi );

  /* Run CM Call Services Unit Test Suite */
  qcril_cm_callsvc_unit_test( rilapi );

  /* Run CM Inband Services Unit Test Suite */
  qcril_cm_inband_unit_test( rilapi );

  /* Run Other Unit Test Suite */
  qcril_other_unit_test( rilapi );

  /* Run Oem Hook Unit Test Suite */
  qcril_oem_hook_unit_test( rilapi );

  return 0;

} /* qcril_main */
#endif /* RIL_SHLIB */

//===========================================================================
// qmi_ril_fw_android_request_get_handling_capabilities
//===========================================================================
uint32_t qmi_ril_fw_android_request_get_handling_capabilities( int android_request_id )
{
  uint32_t res;

  switch ( android_request_id )
  {
    case RIL_REQUEST_VOICE_RADIO_TECH:
    case RIL_REQUEST_SIGNAL_STRENGTH:
    case RIL_REQUEST_REGISTRATION_STATE:
    case RIL_REQUEST_DATA_REGISTRATION_STATE:
    case RIL_REQUEST_IMS_REGISTRATION_STATE:
    case RIL_REQUEST_OPERATOR:
    case RIL_REQUEST_SCREEN_STATE:
    case RIL_REQUEST_SET_LOCATION_UPDATES:
    case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
    case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE:
    case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
    case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE:
    case RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
    case RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE:
    case RIL_REQUEST_QUERY_TTY_MODE:
    case RIL_REQUEST_CDMA_SUBSCRIPTION:
    case RIL_REQUEST_DEVICE_IDENTITY:
    case RIL_REQUEST_BASEBAND_VERSION:
    case RIL_REQUEST_GET_IMEI:
    case RIL_REQUEST_GET_IMEISV:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF,
                                                               QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF );
      break;

    case RIL_REQUEST_GET_NEIGHBORING_CELL_IDS:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF,
                                                               96
                                                               );
      break;

    case RIL_REQUEST_GET_SIM_STATUS:
    case RIL_REQUEST_ENTER_SIM_PIN:
    case RIL_REQUEST_ENTER_SIM_PUK:
    case RIL_REQUEST_ENTER_SIM_PIN2:
    case RIL_REQUEST_ENTER_SIM_PUK2:
    case RIL_REQUEST_CHANGE_SIM_PIN:
    case RIL_REQUEST_CHANGE_SIM_PIN2:
    case RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE:
    case RIL_REQUEST_GET_IMSI:
    case RIL_REQUEST_SIM_IO:
    case RIL_REQUEST_SETUP_DATA_CALL:
    case RIL_REQUEST_DEACTIVATE_DATA_CALL:
    case RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
    case RIL_REQUEST_DATA_CALL_LIST:
    case RIL_REQUEST_GET_DATA_CALL_PROFILE:
    case RIL_REQUEST_SET_DATA_PROFILE:
#if (RIL_QCOM_VERSION >= 2)
    case RIL_REQUEST_SETUP_QOS:
    case RIL_REQUEST_RELEASE_QOS:
    case RIL_REQUEST_GET_QOS_STATUS:
    case RIL_REQUEST_MODIFY_QOS:
    case RIL_REQUEST_SUSPEND_QOS:
    case RIL_REQUEST_RESUME_QOS:
#endif
    case RIL_REQUEST_OEM_HOOK_RAW:
    case RIL_REQUEST_SET_FACILITY_LOCK:
    case RIL_REQUEST_QUERY_FACILITY_LOCK:
    case RIL_REQUEST_STK_GET_PROFILE:
    case RIL_REQUEST_STK_SET_PROFILE:
    case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
    case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
    case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
    case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
    case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS:
#ifdef RIL_REQUEST_SIM_TRANSMIT_CHANNEL
     case RIL_REQUEST_SIM_TRANSMIT_CHANNEL:
#endif /* RIL_REQUEST_SIM_TRANSMIT_CHANNEL */
#ifdef RIL_REQUEST_SIM_CLOSE_CHANNEL
     case RIL_REQUEST_SIM_CLOSE_CHANNEL:
#endif /* RIL_REQUEST_SIM_CLOSE_CHANNEL */
#ifdef RIL_REQUEST_SIM_OPEN_CHANNEL
     case RIL_REQUEST_SIM_OPEN_CHANNEL:
#endif /* RIL_REQUEST_SIM_OPEN_CHANNEL */
#ifdef RIL_REQUEST_SIM_APDU
    case RIL_REQUEST_SIM_APDU:
#endif /* RIL_REQUEST_SIM_APDU */
#ifdef RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC
     case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC:
#endif /* RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC */
#ifdef RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL
     case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL:
#endif /* RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL */
#if defined RIL_REQUEST_SIM_GET_ATR
    case RIL_REQUEST_SIM_GET_ATR:
#endif /* RIL_REQUEST_SIM_GET_ATR */
    case RIL_REQUEST_ISIM_AUTHENTICATION:
#ifdef RIL_REQUEST_SIM_AUTHENTICATION
    case RIL_REQUEST_SIM_AUTHENTICATION:
#endif /* RIL_REQUEST_SIM_AUTHENTICATION */
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FLOW_CONTROL_EXEMPT, QMI_RIL_ZERO );
      break;

    case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC:
    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_MULTIPLE_AUTO_DROP_ON_DIFF_PARAMS,
                                                               QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF );
      break;

    case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
    case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING,
                                                               QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF );
      break;

    case RIL_REQUEST_RADIO_POWER:
    case RIL_REQUEST_SHUTDOWN:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE, QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF );
      break;

    case RIL_REQUEST_SET_UICC_SUBSCRIPTION:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF,
                                                               QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF );
      break;


    case RIL_REQUEST_DTMF_START:
    case RIL_REQUEST_DTMF_STOP:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_VOICE_CALL_SPECIFIC_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FAMILY_RING_DEFINED_PAIR
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING,
                                                               96 );
      break;

    case RIL_REQUEST_DTMF:
    case RIL_REQUEST_CDMA_BURST_DTMF:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_VOICE_CALL_SPECIFIC_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING,
                                                               96 );
      break;

    case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
    case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
    case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
    case RIL_REQUEST_CONFERENCE:
    case RIL_REQUEST_EXPLICIT_CALL_TRANSFER:
    case RIL_REQUEST_SEPARATE_CONNECTION:
    case RIL_REQUEST_UDUB:
    case RIL_REQUEST_HANGUP:
    case RIL_REQUEST_ANSWER:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_VOICE_CALL_SPECIFIC_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING,
                                                               QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF );
      break;

    case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
    case RIL_REQUEST_ALLOW_DATA:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF,
                                                               96 );
      break;

    case RIL_REQUEST_SEND_SMS:
    case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE,
                                                               96 );
      break;

    case RIL_REQUEST_CDMA_SEND_SMS:
    case RIL_REQUEST_IMS_SEND_SMS:
    case RIL_REQUEST_WRITE_SMS_TO_SIM:
    case RIL_REQUEST_SMS_ACKNOWLEDGE:
    case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF,
                                                               96 );
      break;

    case RIL_REQUEST_SEND_USSD:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FLOW_CONTROL_EXEMPT, QMI_RIL_ZERO );
      break;

    case RIL_REQUEST_GET_CLIR:
    case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS:
    case RIL_REQUEST_SET_CALL_FORWARD:
    case RIL_REQUEST_QUERY_CALL_WAITING:
    case RIL_REQUEST_SET_CALL_WAITING:
    case RIL_REQUEST_CHANGE_BARRING_PASSWORD:
    case RIL_REQUEST_QUERY_CLIP:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_NO_AUTO_RESPONSE,
                                                               QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF );
      break;

    case RIL_REQUEST_GET_CELL_INFO_LIST:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF,
                                                               QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_MULTIPLE_DEF );
      break;

    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF
                                                               | QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY,
                                                               QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_SINGLE );
      break;

    default:
      res = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_COMPOSE_CAPS( QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY, QMI_RIL_FW_ANDROID_REQUEST_HNDL_QUEUE_SZ_SINGLE );
      break;
  }

  return res;
} // qmi_ril_fw_android_request_get_handling_capabilities


//===========================================================================
// qmi_ril_fw_android_request_render_execution
//===========================================================================
RIL_Errno qmi_ril_fw_android_request_render_execution( RIL_Token token,
                                                       int android_request_id,
                                                       void * android_request_data,
                                                       int android_request_data_len,
                                                       qcril_instance_id_e_type  instance_id,
                                                       int * is_dedicated_thread )
{

  RIL_Errno                                audit_result;
  int*                                     dedicated_thrd_req_lookup_ptr;
  int                                      dedicated_thrd_req_lookup_val;
  qmi_ril_dedicated_request_exec_params_type*
                                           dedicated_thrd_exec_params;
  pthread_attr_t                           dedicated_thrd_attr;
  pthread_t                                dedicated_thrd_thread_pid;
  int                                      dedicated_thrd_conf;

  qcril_dispatch_table_entry_type *        entry_ptr;
  qcril_request_params_type                param;

  int                                      inbound_data_len_for_cloning;

  qmi_ril_fw_android_param_copy_approach_type
                                           param_copy_approach;

  uint32                                   prepared_four_byte_storage;
  int                                      prepared_four_byte_storage_occupied;
  void*                                    prepared_sub_created_custom_storage;
  int                                      prepared_custom_storage_len;


  QCRIL_LOG_FUNC_ENTRY();

  memset( &param, 0, sizeof( param ) );
  param.event_id          = android_request_id;
  param.data              = android_request_data;
  param.datalen           = android_request_data_len;
  param.t                 = token;
  param.instance_id       = instance_id;
  param.modem_id          = QCRIL_DEFAULT_MODEM_ID;

  audit_result = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_DEBUG( "rendering exec for token id %d", qcril_log_get_token_id( token ) );

  do
  {
      entry_ptr = NULL;
      // do a lookup for the entry
      if ( qcril_hash_table_lookup( (uint32) param.event_id, &entry_ptr ) != E_SUCCESS || NULL == entry_ptr )
      {
        // The request is not supported
        audit_result = RIL_E_REQUEST_NOT_SUPPORTED;
        break;
      }

      // check if request requires dedicated thread for execution
      dedicated_thrd_req_lookup_ptr = qmi_ril_fw_dedicated_thrd_exec_android_requests_set;
      do
      {
        dedicated_thrd_req_lookup_val = *dedicated_thrd_req_lookup_ptr;
        dedicated_thrd_req_lookup_ptr++;
      } while ( QMI_RIL_ZERO != dedicated_thrd_req_lookup_val && param.event_id != dedicated_thrd_req_lookup_val );

      if ( dedicated_thrd_req_lookup_val == param.event_id )
      { // deferred thread exec
        dedicated_thrd_exec_params = qcril_malloc( sizeof(*dedicated_thrd_exec_params) );
        if ( NULL == dedicated_thrd_exec_params )
        {
          audit_result = RIL_E_GENERIC_FAILURE;
          break;
        }

        memset( dedicated_thrd_exec_params, 0, sizeof(*dedicated_thrd_exec_params) );
        dedicated_thrd_exec_params->event_id = param.event_id;
        dedicated_thrd_exec_params->token    = param.t;

        param_copy_approach = qmi_ril_fw_create_android_live_params_copy( android_request_id,
                                                                          android_request_data,
                                                                          android_request_data_len,
                                                                          (void*)&prepared_four_byte_storage,
                                                                          &prepared_four_byte_storage_occupied,
                                                                          &prepared_sub_created_custom_storage,
                                                                          &prepared_custom_storage_len
                                                                          );

        dedicated_thrd_exec_params->param_copy_arrron = param_copy_approach;
        switch ( param_copy_approach )
        {
          case QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT:
            dedicated_thrd_exec_params->copied_params.four_bytes = prepared_four_byte_storage;
            break;

          case QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY:
            dedicated_thrd_exec_params->copied_params.dynamic_copy = prepared_sub_created_custom_storage;
            break;

          case QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION:
            // no action
            break;

          case QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID: // fallthrough
          default:
            qcril_free( dedicated_thrd_exec_params );  // rollback
            dedicated_thrd_exec_params = NULL;
            break;
        }

        if ( NULL == dedicated_thrd_exec_params )
        {
          audit_result = RIL_E_GENERIC_FAILURE;
          break;
        }

        dedicated_thrd_exec_params->original_data_len = android_request_data_len;

#ifdef QMI_RIL_UTF
        pthread_attr_init( &dedicated_thrd_attr );
        dedicated_thrd_conf = utf_pthread_create_handler(&dedicated_thrd_thread_pid, &dedicated_thrd_attr,
                              qmi_ril_fw_dedicated_request_exec_thread, (void*)dedicated_thrd_exec_params );

        pthread_attr_destroy( &dedicated_thrd_attr );
#else
        pthread_attr_init( &dedicated_thrd_attr );
        pthread_attr_setdetachstate( &dedicated_thrd_attr, PTHREAD_CREATE_DETACHED );
        dedicated_thrd_conf = pthread_create( &dedicated_thrd_thread_pid, &dedicated_thrd_attr, qmi_ril_fw_dedicated_request_exec_thread, (void*)dedicated_thrd_exec_params );
        pthread_attr_destroy( &dedicated_thrd_attr );
#endif

        if ( QMI_RIL_ZERO != dedicated_thrd_conf )
        { // failure, rollback
          QCRIL_LOG_ERROR( "dedicated thread launch failure %d", (int)dedicated_thrd_conf );
          qmi_ril_fw_destroy_android_live_params_copy( dedicated_thrd_exec_params->param_copy_arrron,
                                                       android_request_id,
                                                       (void*)(uintptr_t)dedicated_thrd_exec_params->copied_params.four_bytes,
                                                       dedicated_thrd_exec_params->copied_params.dynamic_copy );

          qcril_free( dedicated_thrd_exec_params );
          audit_result = RIL_E_GENERIC_FAILURE;
          break;

        }
        audit_result = RIL_E_SUCCESS;
        QCRIL_LOG_DEBUG( "routed to dedicated thrd status %d", audit_result );

        if ( is_dedicated_thread )
        {
          *is_dedicated_thread = TRUE;
        }
      }
      else
      {

        // do dispatch request internally
        if ( qcril_dispatch_event( entry_ptr, &param ) == E_NOT_ALLOWED )
        {
          audit_result = RIL_E_RADIO_NOT_AVAILABLE;
          break;
        }
        audit_result = RIL_E_SUCCESS;

        if ( is_dedicated_thread )
        {
          *is_dedicated_thread = FALSE;
        }
      }
  } while (FALSE);

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)audit_result );

  return audit_result;
} // qmi_ril_fw_android_request_render_execution
//===========================================================================
// qmi_ril_fw_android_request_flow_control_info_lock
//===========================================================================
void qmi_ril_fw_android_request_flow_control_info_lock( void )
{
  pthread_mutex_lock( &qmi_ril_fw_android_request_flow_control_overview.guard.lock_mutex );
} // qmi_ril_fw_android_request_flow_control_info_lock
//===========================================================================
// qmi_ril_fw_android_request_flow_control_info_unlock
//===========================================================================
void qmi_ril_fw_android_request_flow_control_info_unlock( void )
{
  pthread_mutex_unlock( &qmi_ril_fw_android_request_flow_control_overview.guard.lock_mutex );
} // qmi_ril_fw_android_request_flow_control_info_unlock
//===========================================================================
// qmi_ril_fw_android_request_flow_control_trigger_remains
//===========================================================================
void qmi_ril_fw_android_request_flow_control_trigger_remains(qcril_timed_callback_handler_params_type * handler_params)
{
  int                                       android_request_id;

  qmi_ril_fw_android_request_kind_execution_overview_type*
                                            android_request_kind_execution_overview;
  qmi_ril_fw_android_request_kind_execution_overview_type*
                                            android_request_kind_execution_being_busy;
  uint32_t                                  android_request_handling_caps;
  uint32_t                                  cap_max_queue_sz;
  uint32_t                                  cap_queue_sz_iter;
  void*                                     android_request_param_clone;
  int                                       do_flow_control_follow_up;
  int                                       need_trigger_address_remains;
  int                                       regarded_same;
  uint32_t                                  payload_sz_cur;
  uint32_t                                  payload_sz_iter;

  void*                                     invoke_data;
  int                                       invoke_data_len;

  RIL_Errno                                 audit_result;
  int                                       must_render;

  qmi_ril_fw_android_request_kind_execution_overview_type*
                                            android_request_kind_execution_exec_canidate;
  qmi_ril_fw_android_request_holder_type*   android_request_param_exec_candidate;
  int                                       oustanding_req_lookup_res;

  int                                       android_request_id_exec_candidate;

  int                                       log_android_req_id = 0;
  int                                       log_projected_token_id = 0;
  int                                       log_id_dedicated_thread = FALSE;
  int                                       log_is_commenced = FALSE;


  QCRIL_LOG_FUNC_ENTRY();

  audit_result = RIL_E_GENERIC_FAILURE;

  if ( NULL != handler_params)
  {
    android_request_id = (intptr_t)handler_params->custom_param;
    must_render        = FALSE;

    QCRIL_LOG_INFO( ".. android request id %d", (int)android_request_id );

    qmi_ril_fw_android_request_flow_control_info_lock();

    do
    {
      if ( android_request_id <= QMI_RIL_ZERO || android_request_id > QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID )
        break;

      android_request_kind_execution_overview = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ android_request_id ];
      android_request_handling_caps = android_request_kind_execution_overview->nof_extra_holders_and_caps_and_dynamics;
      cap_max_queue_sz = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ( android_request_handling_caps );

      android_request_kind_execution_being_busy = qmi_ril_fw_android_request_flow_control_find_busy_kind( android_request_kind_execution_overview );
      if ( NULL != android_request_kind_execution_being_busy )
      { // occupied with request, will be retried as soon as that completes
        audit_result = RIL_E_SUCCESS;
        break;
      }

      oustanding_req_lookup_res = qmi_ril_fw_android_request_flow_control_find_request_for_execution(
                                                                                    android_request_kind_execution_overview,
                                                                                    &android_request_kind_execution_exec_canidate,
                                                                                    &android_request_param_exec_candidate
                                                                                    );
      QCRIL_LOG_INFO( ".. oustanding lookup res %d", (int)oustanding_req_lookup_res );
      if ( !oustanding_req_lookup_res )
      { // no request to exec
        audit_result = RIL_E_SUCCESS;
        break;
      }

      android_request_id_exec_candidate = android_request_kind_execution_exec_canidate->original_android_request_id;

      android_request_kind_execution_exec_canidate->token_under_execution = android_request_param_exec_candidate->token;

      QCRIL_LOG_DEBUG( "token under exec %d", qcril_log_get_token_id ( android_request_kind_execution_exec_canidate->token_under_execution )  );

      if ( android_request_param_exec_candidate->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_NO_PAYLOAD )
      {
        invoke_data     = NULL;
        invoke_data_len = QMI_RIL_ZERO;
      }
      else if ( android_request_param_exec_candidate->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_INT  )
      {
        invoke_data     = &android_request_param_exec_candidate->param_payload_holder.param_int;
        invoke_data_len = sizeof(int);
      }
      else if ( android_request_param_exec_candidate->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_PTR )
      {
        invoke_data     = android_request_param_exec_candidate->param_payload_holder.param_ptr;
        invoke_data_len = QMI_RIL_FW_ANDROID_REQUEST_INFO_DECOMPOSE_SZ( android_request_param_exec_candidate->param_info_and_len );
      }
      else // invalid
        break;

      must_render = TRUE;

      log_android_req_id     = android_request_kind_execution_exec_canidate->original_android_request_id;
      log_projected_token_id = qcril_log_get_token_id( android_request_param_exec_candidate->token );

    } while (FALSE);

    qmi_ril_fw_android_request_flow_control_info_unlock();

    if ( must_render )
    {
      audit_result = qmi_ril_fw_android_request_render_execution( android_request_param_exec_candidate->token,
                                                                  android_request_id_exec_candidate,
                                                                  invoke_data,
                                                                  invoke_data_len,
                                                                  qmi_ril_get_process_instance_id(),
                                                                  &log_id_dedicated_thread );

      log_is_commenced = TRUE;
    }

  }

  QCRIL_LOG_INFO( ".. audit res %d", (int)audit_result );

  if ( log_is_commenced )
  {
    QCRIL_LOG_DEBUG( "cmd %d exec t_id %d,thrd: %s,adt: %d",
                         log_android_req_id,
                         log_projected_token_id,
                         log_id_dedicated_thread ? "ddctd" : "main",
                         (int) audit_result
                         );
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_fw_android_request_flow_control_trigger_remains
//===========================================================================
// qmi_ril_fw_get_main_thread_id
//===========================================================================
pthread_t qmi_ril_fw_get_main_thread_id()
{
  return qmi_ril_main_thread_id;
} // qmi_ril_fw_get_main_thread_id
//===========================================================================
// qmi_ril_fw_android_request_flow_control_init
//===========================================================================
void qmi_ril_fw_android_request_flow_control_init( void )
{
  uint32_t  sz;
  uint32_t  android_request_id;
  uint32_t  android_request_caps;
  uint32_t  caps_nof_slots;
  qmi_ril_fw_android_request_kind_execution_overview_type*
            rec_overview;
  qmi_ril_fw_android_request_holder_type*
            holders;
  qmi_ril_fw_android_request_holder_type*
            holder_iter;
  uint32_t  iter;
  int       is_retore_mode_pref_support;

  static const int family_ring_nw_sel[] =
  {
    RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC,
    RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL,
    QMI_RIL_ZERO                                              // must be last one
  };

  static const int family_ring_mode_pref[] =
  {
    RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE,
    RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE,
    QMI_RIL_ZERO                                              // must be last one
  };

  static const int family_ring_nw_sel_and_mode_pref[] =
  {
    RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC,
    RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL,
    RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE,
    RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE,
    QMI_RIL_ZERO                                              // must be last one
  };

  static const int family_ring_dtmf[] =
  {
    RIL_REQUEST_DTMF,
    RIL_REQUEST_DTMF_START,
    RIL_REQUEST_DTMF_STOP,
    RIL_REQUEST_CDMA_BURST_DTMF,
    QMI_RIL_ZERO                                              // must be last one
  };

  static const int family_ring_incall_ss[] =
  {
    RIL_REQUEST_HANGUP,
    RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND,
    RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND,
    RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE,
    RIL_REQUEST_CONFERENCE,
    RIL_REQUEST_EXPLICIT_CALL_TRANSFER,
    RIL_REQUEST_SEPARATE_CONNECTION,
    RIL_REQUEST_UDUB,
    RIL_REQUEST_ANSWER,
    QMI_RIL_ZERO                                              // must be last one
  };

  static const int family_ring_common_ss[] =
  {
    RIL_REQUEST_QUERY_CALL_FORWARD_STATUS,
    RIL_REQUEST_SET_CALL_FORWARD,
    RIL_REQUEST_QUERY_CALL_WAITING,
    RIL_REQUEST_SET_CALL_WAITING,
    RIL_REQUEST_CHANGE_BARRING_PASSWORD,
    RIL_REQUEST_QUERY_CLIP,
    RIL_REQUEST_GET_CLIR,
    QMI_RIL_ZERO                                              // must be last one
  };

  static const int family_ring_send_sms_and_more[] =
  {
    RIL_REQUEST_SEND_SMS,
    RIL_REQUEST_SEND_SMS_EXPECT_MORE,
    QMI_RIL_ZERO                                              // must be last one
  };

  static const int family_ring_ps_attach_detach[] =
  {
    RIL_REQUEST_SET_INITIAL_ATTACH_APN,
    RIL_REQUEST_ALLOW_DATA,
    QMI_RIL_ZERO                                              // must be last one
  };

  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_get_property_value_from_integer(QCRIL_RESTORE_MODE_PREF_SUPPORT,
                                        &is_retore_mode_pref_support, 0);

  memset( &qmi_ril_fw_android_request_flow_control_overview, 0, sizeof( qmi_ril_fw_android_request_flow_control_overview ) );

  holders = NULL;

  // guard
  pthread_mutexattr_init( &qmi_ril_fw_android_request_flow_control_overview.guard.lock_mtx_atr );
  pthread_mutex_init( &qmi_ril_fw_android_request_flow_control_overview.guard.lock_mutex, &qmi_ril_fw_android_request_flow_control_overview.guard.lock_mtx_atr );

  // table
  sz = sizeof(qmi_ril_fw_android_request_kind_execution_overview_type) * (QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID + 1);
  qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info = qcril_malloc( sz );
  if ( NULL != qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info )
  {
    memset( qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info, 0, sz );
    for ( android_request_id = QMI_RIL_ZERO; android_request_id <= QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID; android_request_id++ )
    {
      android_request_caps = qmi_ril_fw_android_request_get_handling_capabilities( android_request_id );
      caps_nof_slots = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ( android_request_caps );

      rec_overview = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ android_request_id ];
      rec_overview->holders.extra_holders = holders;

      if ( android_request_caps & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE )
      {
        sz = sizeof(qmi_ril_fw_android_request_holder_type) * caps_nof_slots;
        holders = qcril_malloc( sz );
        if ( NULL != holders )
        {
          memset( holders, 0, sz );
          rec_overview->holders.extra_holders = holders;
          holder_iter = holders;
          for ( iter = QMI_RIL_ZERO; iter < caps_nof_slots; iter++ )
          {
            holder_iter->param_info_and_len = QMI_RIL_FW_ANDROID_REQUEST_INFO_COMPOSE( QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE, QMI_RIL_ZERO );
            holder_iter++;
          }
        }
      }
      else
      {
        rec_overview->holders.local_holder.param_info_and_len = QMI_RIL_FW_ANDROID_REQUEST_INFO_COMPOSE( QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE, QMI_RIL_ZERO );
      }

      rec_overview->nof_extra_holders_and_caps_and_dynamics = android_request_caps;
      rec_overview->original_android_request_id             = android_request_id;
    }

    // family ring init
    if ( !is_retore_mode_pref_support )
    {
      qmi_ril_fw_android_request_flow_control_declare_family_ring( family_ring_nw_sel );
      qmi_ril_fw_android_request_flow_control_declare_family_ring( family_ring_mode_pref );
    }
    else
    {
      qmi_ril_fw_android_request_flow_control_declare_family_ring( family_ring_nw_sel_and_mode_pref);
    }
    qmi_ril_fw_android_request_flow_control_declare_family_ring( family_ring_dtmf );
    qmi_ril_fw_android_request_flow_control_declare_family_ring( family_ring_incall_ss );
    qmi_ril_fw_android_request_flow_control_declare_family_ring( family_ring_common_ss );
    qmi_ril_fw_android_request_flow_control_declare_family_ring( family_ring_send_sms_and_more );
    qmi_ril_fw_android_request_flow_control_declare_family_ring( family_ring_ps_attach_detach );

    qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ RIL_REQUEST_DTMF_START ].family_pair_android_request_id = RIL_REQUEST_DTMF_STOP;
    qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ RIL_REQUEST_DTMF_STOP ].family_pair_android_request_id = RIL_REQUEST_DTMF_START;

    qmi_ril_fw_android_request_flow_control_overview.common_moniker_book = QMI_RIL_ZERO;
    qmi_ril_fw_android_request_flow_control_overview.common_moniker_book++;
  }

  QCRIL_LOG_FUNC_RETURN();
}
//===========================================================================
// qmi_ril_fw_android_request_flow_control_find_busy_kind
//===========================================================================
qmi_ril_fw_android_request_kind_execution_overview_type* qmi_ril_fw_android_request_flow_control_find_busy_kind( qmi_ril_fw_android_request_kind_execution_overview_type* origin )
{
  qmi_ril_fw_android_request_kind_execution_overview_type* res = NULL;
  qmi_ril_fw_android_request_kind_execution_overview_type* iter;

  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL != origin )
  {
    if ( !( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING )  )
    { // single node
      if ( QMI_RIL_ZERO != origin->token_under_execution )
      {
        res = origin;
      }
    }
    else
    { // ring
      iter = origin;
      do
      {
        QCRIL_LOG_INFO(".. token ptr 0x%x", iter->token_under_execution );
        if ( QMI_RIL_ZERO != iter->token_under_execution )
        {
          res = iter;
        }
        else
        {
          iter->nof_extra_holders_and_caps_and_dynamics |= QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK;
          iter = iter->family_ring;
        }
      } while ( NULL == res && !(QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK & iter->nof_extra_holders_and_caps_and_dynamics) );

      iter = origin;
      while ( QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK & iter->nof_extra_holders_and_caps_and_dynamics )
      {
        iter->nof_extra_holders_and_caps_and_dynamics &= ~QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK;
        iter = iter->family_ring;
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET((intptr_t)res);

  return res;
} // qmi_ril_fw_android_request_flow_control_find_busy_kind
//===========================================================================
// qmi_ril_fw_android_request_flow_control_find_outstanding_request_within_kind
//===========================================================================
qmi_ril_fw_android_request_holder_type * qmi_ril_fw_android_request_flow_control_find_outstanding_request_within_kind( qmi_ril_fw_android_request_kind_execution_overview_type* origin )
{
  qmi_ril_fw_android_request_holder_type *  res = NULL;
  uint32_t                                  idx;
  qmi_ril_fw_android_request_holder_type *  iter;
  uint32_t                                  nof_cap;

  if ( NULL != origin && QMI_RIL_ZERO != origin->chief_moniker )
  {
    if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY )
    { // single
      if ( !(origin->holders.local_holder.param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE) )
      {
        res = &origin->holders.local_holder;
      }
    }
    else if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE )
    {
      iter = origin->holders.extra_holders;
      nof_cap = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ ( origin->nof_extra_holders_and_caps_and_dynamics );
      for ( idx = 0; idx < nof_cap && NULL == res; idx ++ )
      {
        if ( !( iter->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE ) && ( iter->moniker == origin->chief_moniker ) )
        {
          res = iter;
        }
        else
        {
          iter++;
        }
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET((intptr_t)res);

  return res;
}
//===========================================================================
// qmi_ril_fw_android_request_flow_control_find_request_for_execution
//===========================================================================
int qmi_ril_fw_android_request_flow_control_find_request_for_execution( qmi_ril_fw_android_request_kind_execution_overview_type* origin,
                                                                        qmi_ril_fw_android_request_kind_execution_overview_type** exec_overview,
                                                                        qmi_ril_fw_android_request_holder_type ** exec_req_holder )
{
  int res = FALSE;
  qmi_ril_fw_android_request_kind_execution_overview_type*  res_exec_overview = NULL;
  qmi_ril_fw_android_request_holder_type *                  res_exec_req_holder = NULL;
  qmi_ril_fw_android_request_holder_type *                  holder_candidate;
  qmi_ril_fw_android_request_kind_execution_overview_type*  iter;
  uint32_t                                                  chief_of_chiefs_moniker;
  int                                                       moniker_cmp_res;
  qmi_ril_fw_android_request_kind_execution_overview_type*  pair;

  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL != origin )
  {
    chief_of_chiefs_moniker = QMI_RIL_ZERO;

    if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING )
    { // ring case

      // check first if pair is defined
      if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FAMILY_RING_DEFINED_PAIR &&
           QMI_RIL_ZERO != origin->family_pair_android_request_id  )
      {
        pair = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ origin->family_pair_android_request_id ];
        holder_candidate = qmi_ril_fw_android_request_flow_control_find_outstanding_request_within_kind( pair );

        if ( NULL != holder_candidate )
        {
          res_exec_req_holder     = holder_candidate;
          res_exec_overview       = pair;
          res                     = TRUE;
        }
      }

      // look up through whole family ring
      if ( !res )
      {
        iter = origin;
        do
        {
          holder_candidate = qmi_ril_fw_android_request_flow_control_find_outstanding_request_within_kind( iter );

          if ( NULL != holder_candidate )
          {
            if ( QMI_RIL_ZERO == chief_of_chiefs_moniker )
            {
              chief_of_chiefs_moniker = iter->chief_moniker;
              res_exec_req_holder     = holder_candidate;
              res_exec_overview       = iter;
              res                     = TRUE;
            }
            else
            { // non empty chief_of_chiefs_moniker, do election
              moniker_cmp_res = qmi_ril_fw_android_request_flow_control_moniker_compare( iter->chief_moniker, chief_of_chiefs_moniker );
              if ( QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_OLDER == moniker_cmp_res )
              {
                chief_of_chiefs_moniker = iter->chief_moniker;
                res_exec_req_holder     = holder_candidate;
                res_exec_overview       = iter;
                res                     = TRUE;
              }
            }
          }

          iter->nof_extra_holders_and_caps_and_dynamics |= QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK;
          iter = iter->family_ring;
        } while ( !( QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK & iter->nof_extra_holders_and_caps_and_dynamics ) );

        iter = origin;
        while ( QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK & iter->nof_extra_holders_and_caps_and_dynamics )
        {
          iter->nof_extra_holders_and_caps_and_dynamics &= ~QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK;
          iter = iter->family_ring;
        }
      }

    }
    else
    { // one node only
      res_exec_req_holder = qmi_ril_fw_android_request_flow_control_find_outstanding_request_within_kind( origin );
      if ( NULL != res_exec_req_holder )
      {
        res_exec_overview = origin;
        res               = TRUE;
      }
    }
  }

  if ( NULL != exec_overview )
  {
    *exec_overview = res_exec_overview;
  }
  if ( NULL != exec_req_holder )
  {
    *exec_req_holder = res_exec_req_holder;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET((int)res);

  return res;
} // qmi_ril_fw_android_request_flow_control_find_request_for_execution

//===========================================================================
// qmi_ril_fw_android_request_flow_control_release_holder_info_bookref
//===========================================================================
void qmi_ril_fw_android_request_flow_control_release_holder_info_bookref( qmi_ril_fw_android_request_holder_type* request_holder_org, int android_request_id )
{
  qmi_ril_fw_android_param_copy_approach_type
        destroy_prm_used_approach;
  void* destroy_prm_four_byte_storage;
  void* destroy_prm_sub_created_custom_storage;

  QCRIL_LOG_FUNC_ENTRY();

  if ( request_holder_org && !( request_holder_org->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE ) )
  {
    if ( request_holder_org->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_NO_PAYLOAD )
    {
      destroy_prm_used_approach = QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION;
    }
    if ( request_holder_org->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_PTR )
    {
      destroy_prm_used_approach = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
    }
    else if ( request_holder_org->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_INT )
    {
      destroy_prm_used_approach = QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT;
    }
    else
    {
      destroy_prm_used_approach = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
    }

    if ( QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID != destroy_prm_used_approach )
    {
      destroy_prm_four_byte_storage           = (void*)&request_holder_org->param_payload_holder.param_int;
      destroy_prm_sub_created_custom_storage  = request_holder_org->param_payload_holder.param_ptr;

      qmi_ril_fw_destroy_android_live_params_copy( destroy_prm_used_approach,
                                                   android_request_id,
                                                   destroy_prm_four_byte_storage,
                                                   destroy_prm_sub_created_custom_storage );
    }

    request_holder_org->token = QMI_RIL_ZERO;
    request_holder_org->param_payload_holder.param_ptr = NULL;
    request_holder_org->param_payload_holder.param_int = QMI_RIL_ZERO;
    request_holder_org->param_info_and_len = QMI_RIL_FW_ANDROID_REQUEST_INFO_COMPOSE( QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE, QMI_RIL_ZERO );
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_fw_android_request_flow_control_release_holder_info_bookref
//===========================================================================
// qmi_ril_fw_android_request_flow_control_abandon_requests_local_only
//===========================================================================
void qmi_ril_fw_android_request_flow_control_abandon_requests_local_only( qmi_ril_fw_android_request_kind_execution_overview_type* origin, RIL_Errno cause, int is_unbind_cleanup )
{
  uint32_t                                          nof_cap;
  uint32_t                                          idx;
  qmi_ril_fw_android_request_holder_type*           iter;
  RIL_Token                                         cur_token;
  int                                               android_request_id;
  uint32_t cap_mask;

  QCRIL_LOG_FUNC_ENTRY();

  if (is_unbind_cleanup ==  TRUE)
  {
    cap_mask = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF;
  }
  else
  {
    cap_mask = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE;
  }

  if ( origin )
  {
    android_request_id = origin->original_android_request_id;
    if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY )
    {
      if ( !(origin->holders.local_holder.param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE) )
      {
        cur_token = origin->holders.local_holder.token;

        if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF )
        {
          ( void ) qcril_reqlist_free( QCRIL_DEFAULT_INSTANCE_ID , cur_token );
        }

        qmi_ril_fw_android_request_flow_control_release_holder_info_bookref( &origin->holders.local_holder, android_request_id );

        QCRIL_LOG_DEBUG("abandon  token id %d, a-r-id %d", qcril_log_get_token_id( cur_token ), android_request_id );
        qcril_response_api[ QCRIL_DEFAULT_INSTANCE_ID ]->OnRequestComplete( cur_token, // instance id is of relevance in QMI RIL while responding
                                                              cause,
                                                              NULL,
                                                              QMI_RIL_ZERO );
      }
    }
    else if ( origin->nof_extra_holders_and_caps_and_dynamics & cap_mask )
    {
      nof_cap = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ ( origin->nof_extra_holders_and_caps_and_dynamics );
      iter = origin->holders.extra_holders;
      for ( idx = QMI_RIL_ZERO; idx < nof_cap; idx++ )
      {
        if ( !(iter->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE) )
        {
          cur_token = iter->token;

          if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_DROP_OFF )
          {
            ( void ) qcril_reqlist_free( QCRIL_DEFAULT_INSTANCE_ID , cur_token );
          }

          qmi_ril_fw_android_request_flow_control_release_holder_info_bookref( iter, android_request_id );

          QCRIL_LOG_DEBUG("abandon  token id %d, a-r-id %d", qcril_log_get_token_id( cur_token ), android_request_id );
          qcril_response_api[ QCRIL_DEFAULT_INSTANCE_ID ]->OnRequestComplete( cur_token, // instance id is of relevance in QMI RIL while responding
                                                                cause,
                                                                NULL,
                                                                QMI_RIL_ZERO );
        }

        iter++;
      }
    }

    if ( QMI_RIL_ZERO != origin->token_under_execution )
    {
      origin->token_under_execution = QMI_RIL_ZERO;
    }

    qmi_ril_fw_android_request_flow_control_overview_request_review_holders( origin );
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_fw_android_request_flow_control_abandon_requests_local_only
//===========================================================================
// qmi_ril_fw_android_request_flow_control_abandon_requests_local_and_ring_zoned
//===========================================================================
void qmi_ril_fw_android_request_flow_control_abandon_requests_local_and_ring_zoned( qmi_ril_fw_android_request_kind_execution_overview_type* origin, RIL_Errno cause, int is_unbind_cleanup )
{
  qmi_ril_fw_android_request_kind_execution_overview_type* iter;
  QCRIL_LOG_FUNC_ENTRY();

  if ( origin )
  {
    if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_IN_FAMILY_RING )
    {
      iter = origin;

      do
      {
        qmi_ril_fw_android_request_flow_control_abandon_requests_local_only( iter, cause, is_unbind_cleanup );

        iter->nof_extra_holders_and_caps_and_dynamics |= QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK;
        iter = iter->family_ring;

      } while ( !(QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK & iter->nof_extra_holders_and_caps_and_dynamics) );

      iter = origin;
      while ( QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK & iter->nof_extra_holders_and_caps_and_dynamics )
      {
        iter->nof_extra_holders_and_caps_and_dynamics &= ~QMI_RIL_FW_ANDROID_REQUEST_KIND_EXEC_OVERVIEW_DYNAMICS_RING_LOOP_MARK;
        iter = iter->family_ring;
      }

    }
    else
    {
      qmi_ril_fw_android_request_flow_control_abandon_requests_local_only( origin, cause, is_unbind_cleanup );
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_fw_android_request_flow_control_abandon_requests_local_and_ring_zoned
//===========================================================================
// qmi_ril_fw_android_request_flow_control_abandon_requests_family_main_thrd
//===========================================================================
void qmi_ril_fw_android_request_flow_control_abandon_requests_family_main_thrd( int android_request_id, RIL_Errno cause )
{
  qmi_ril_fw_android_request_kind_execution_overview_type* overview;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_fw_android_request_flow_control_info_lock();

  do
  {
    if ( android_request_id <= QMI_RIL_ZERO || android_request_id > QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID )
      break;

    overview = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ android_request_id ];

    if ( overview->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FLOW_CONTROL_EXEMPT )
      break;

    qmi_ril_fw_android_request_flow_control_abandon_requests_local_and_ring_zoned( overview, cause, FALSE );

  } while (FALSE);

  qmi_ril_fw_android_request_flow_control_info_unlock();

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_fw_android_request_flow_control_abandon_requests_family_main_thrd

//===========================================================================
// qmi_ril_abandon_all_ims_requests
//===========================================================================
void qmi_ril_abandon_all_ims_requests( RIL_Errno cause )
{
  qcril_reqlist_public_type request_info;
  IxErrnoType               err_code;
  int                       ims_req_id;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_ims_flow_control_event_queue(QCRIL_QMI_IMS_FLOW_CONTROL_CLEAR_LIST,
                                       QCRIL_DATA_NOT_ON_STACK, QCRIL_EVT_NONE,
                                       NULL, 0, 0);

  for ( ims_req_id = QCRIL_EVT_IMS_SOCKET_REQ_BASE+1;
          ims_req_id < QCRIL_EVT_IMS_SOCKET_REQ_MAX; ims_req_id++ )
  {
    while ( E_SUCCESS == qcril_reqlist_query_by_request( QCRIL_DEFAULT_INSTANCE_ID,
                ims_req_id, &request_info ) )
    {
      qcril_qmi_ims_socket_send( request_info.t, IMS__MSG_TYPE__RESPONSE,
              qcril_qmi_ims_map_event_to_request(ims_req_id),
              qcril_qmi_ims_map_ril_error_to_ims_error(cause), NULL, 0 );
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qmi_ril_fw_android_request_flow_control_abandon_all_requests_main_thrd
//===========================================================================
void qmi_ril_fw_android_request_flow_control_abandon_all_requests_main_thrd( RIL_Errno cause, int is_unbind_cleanup )
{
  int android_request_id;
  qmi_ril_fw_android_request_kind_execution_overview_type* overview;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_fw_android_request_flow_control_info_lock();
  for ( android_request_id = QMI_RIL_ZERO;  android_request_id <= QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID; android_request_id++ )
  {
    overview = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ android_request_id ];

    if ( !(overview->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_FLOW_CONTROL_EXEMPT) )
    {
      qmi_ril_fw_android_request_flow_control_abandon_requests_local_and_ring_zoned( overview, cause, is_unbind_cleanup );
    }
  }
  qmi_ril_fw_android_request_flow_control_info_unlock();

  qmi_ril_abandon_all_ims_requests( cause );

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_fw_android_request_flow_control_abandon_all_requests_main_thrd
//===========================================================================
// qmi_ril_fw_android_request_flow_control_request_holders_have_same_param
//===========================================================================
int qmi_ril_fw_android_request_flow_control_request_holders_have_same_param( qmi_ril_fw_android_request_holder_type* origin, qmi_ril_fw_android_request_holder_type* peer )
{
  int res = FALSE;
  uint32_t payload_sz_iter_org;
  uint32_t payload_sz_iter_peer;

  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL != origin && NULL != peer )
  {
    if ( origin->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_NO_PAYLOAD && peer->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_NO_PAYLOAD )
    {
      res = TRUE;
    }
    else if ( origin->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_INT && peer->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_INT )
    {
      if ( origin->param_payload_holder.param_int == peer->param_payload_holder.param_int )
      {
        res = TRUE;
      }
    }
    else if ( origin->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_PTR && peer->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_PAYLOAD_PTR )
    {
      payload_sz_iter_org = QMI_RIL_FW_ANDROID_REQUEST_INFO_DECOMPOSE_SZ( origin->param_info_and_len );
      payload_sz_iter_peer = QMI_RIL_FW_ANDROID_REQUEST_INFO_DECOMPOSE_SZ( peer->param_info_and_len );
      if ( payload_sz_iter_org == payload_sz_iter_peer &&
           0 == memcmp( origin->param_payload_holder.param_ptr, peer->param_payload_holder.param_ptr, payload_sz_iter_org ) )
      {
        res = TRUE;
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET((int)res);
  return res;
} // qmi_ril_fw_android_request_flow_control_request_holders_have_same_param
//===========================================================================
// qmi_ril_fw_android_request_flow_control_declare_family_ring
//===========================================================================
void qmi_ril_fw_android_request_flow_control_declare_family_ring( const int * android_request_ids )
{
  const int * android_request_ids_iter;
  int cur;
  qmi_ril_fw_android_request_kind_execution_overview_type* overview_cur;
  qmi_ril_fw_android_request_kind_execution_overview_type* overview_prev;
  qmi_ril_fw_android_request_kind_execution_overview_type* overview_first;

  if ( NULL != android_request_ids )
  {
    overview_prev = NULL;
    overview_first = NULL;
    overview_cur = NULL;
    android_request_ids_iter = android_request_ids;

    while ( QMI_RIL_ZERO != ( cur = *android_request_ids_iter ) )
    {
      overview_cur = &qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info[ cur ];

      if ( NULL != overview_prev )
      {
        overview_prev->family_ring = overview_cur;
      }
      else
      {
        overview_first = overview_cur;
      }
      overview_prev = overview_cur;

      android_request_ids_iter++;
    }
    if ( NULL != overview_cur )
    { // non empty
      overview_cur->family_ring = overview_first;
    }

  }
} // qmi_ril_fw_android_request_flow_control_declare_family_ring
//===========================================================================
// qmi_ril_fw_android_request_flow_control_overview_request_review_holders
//===========================================================================
void qmi_ril_fw_android_request_flow_control_overview_request_review_holders( qmi_ril_fw_android_request_kind_execution_overview_type* origin )
{
  uint32_t                                chief_moniker_candidate;
  uint32_t                                idx;
  uint32_t                                nof_cap;
  qmi_ril_fw_android_request_holder_type* iter;
  int                                     cmp_res;

  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL != origin )
  {
    if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_SINGLE_ONLY )
    {
      if ( !(origin->holders.local_holder.param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE) )
      {
        origin->chief_moniker = origin->holders.local_holder.moniker;
      }
      else
      {
        origin->chief_moniker = QMI_RIL_ZERO;
      }
    }
    else if ( origin->nof_extra_holders_and_caps_and_dynamics & QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE )
    {
      chief_moniker_candidate = QMI_RIL_ZERO;
      iter                    = origin->holders.extra_holders;

      nof_cap = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_DECOMPOSE_QUEUE_SZ( origin->nof_extra_holders_and_caps_and_dynamics );
      for ( idx = QMI_RIL_ZERO; idx < nof_cap; idx++ )
      {
        if ( !(iter->param_info_and_len & QMI_RIL_FW_ANDROID_REQUEST_INFO_SLOT_AVAILABLE) )
        {
          if ( QMI_RIL_ZERO != chief_moniker_candidate )
          {
            cmp_res = qmi_ril_fw_android_request_flow_control_moniker_compare( iter->moniker, chief_moniker_candidate );
            if ( QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_OLDER == cmp_res )
            {
              chief_moniker_candidate = iter->moniker;
            }
          }
          else
          {
            chief_moniker_candidate = iter->moniker;
          }
        }
        iter++;
      }

      origin->chief_moniker = chief_moniker_candidate;
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_fw_android_request_flow_control_overview_request_review_holders
//===========================================================================
// qmi_ril_fw_android_request_flow_control_moniker_compare
//===========================================================================
int qmi_ril_fw_android_request_flow_control_moniker_compare( uint32_t moniker1, uint32_t moniker2 )
{
  int res;

  if ( moniker1 == moniker2 )
  {
    res = QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_EQUAL;
  }
  else
  { // non equal
    if ( moniker1 < moniker2 )
    {
      res = QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_OLDER;
    }
    else
    { // moniker2 is arithmetically lesser
      if ( moniker1 > 0xEFFFFFFF )
      {
        res = QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_OLDER;
      }
      else
      {
        res = QMI_RIL_FW_FLOW_CONTROL_CMP_MONIKERS_NEWER;
      }
    }
  }
  return res;
} // qmi_ril_fw_android_request_flow_control_moniker_compare
//===========================================================================
// qmi_ril_fw_android_request_flow_control_drop_legacy_book_records
//===========================================================================
void qmi_ril_fw_android_request_flow_control_drop_legacy_book_records( int voice_calls_related_only, int is_unbind_cleanup )
{
  int                       android_request_id;
  uint32_t                  android_request_caps;
  qcril_reqlist_public_type request_info;
  IxErrnoType               err_code;
  uint32_t                  cmp_mask;

  if ( voice_calls_related_only )
  {
    cmp_mask = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_LEGACY_VOICE_CALL_SPECIFIC_DROP_OFF;
  }
  else
  {
    if (is_unbind_cleanup == TRUE)
    {
      cmp_mask = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_UNBIND_DROP_OFF;
    }
    else
    {
      cmp_mask = QMI_RIL_FW_ANDROID_REQUEST_HNDL_CAPS_ALLOW_MULTIPLE;
    }
  }

  for ( android_request_id = QMI_RIL_ZERO; android_request_id <= QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID; android_request_id++ )
  {
    android_request_caps = qmi_ril_fw_android_request_get_handling_capabilities( android_request_id );
    if ( cmp_mask & android_request_caps )
    {
      err_code = qcril_reqlist_query_by_request( QCRIL_DEFAULT_INSTANCE_ID, android_request_id, &request_info );
      if ( E_SUCCESS == err_code )
      {
        qcril_reqlist_free( QCRIL_DEFAULT_INSTANCE_ID, request_info.t );
      }
    }
  }
} // qmi_ril_fw_android_request_flow_control_drop_legacy_book_records
//===========================================================================
// qmi_ril_fw_create_android_live_params_copy
//===========================================================================
qmi_ril_fw_android_param_copy_approach_type qmi_ril_fw_create_android_live_params_copy(int android_request_id,
                                               void * android_request_data,
                                               int android_request_data_len,
                                               void* four_byte_storage,
                                               int* four_byte_storage_occupied,
                                               void** sub_created_custom_storage,
                                               int* custom_storage_len)
{
  qmi_ril_fw_android_param_copy_approach_type res;
  qmi_ril_fw_android_param_copy_approach_type res_inclanation;
  int len_to_go;
  char* str_access;
  int need_common_clone;

  void * locally_created_custom_storage;
  int locally_created_custom_storage_len;
  int local_ok;

  RIL_IMS_SMS_Message*  android_send_ims_msg_params;
  RIL_IMS_SMS_Message*  copied_android_send_ims_msg_params;
  RIL_CDMA_SMS_Message* copied_android_cdma_send_ims_param;

  char *copied_android_gw_smsc_address;
  char *copied_android_gw_pdu;
  char ** copied_android_gw_sms_ims_params;

  char** android_send_sms_params;

  char** android_cdma_dtmf_holder;
  char** copied_cdma_dtmf_holder;
  char* copied_cdma_dtmf_str;
  char* copied_cdma_dtmf_on;
  char* copied_cdma_dtmf_off;

  char **android_manual_selection_params;
  char *copied_android_manual_selection_mcc_mnc;
  char *copied_android_manual_selection_rat;
  char ** copied_android_manual_selection_params;

  RIL_InitialAttachApn* android_initial_attach_apn_params;
  RIL_InitialAttachApn* copied_android_initial_attach_apn_params=NULL;
  char* copied_android_initial_attach_apn_apn=NULL;
  char* copied_android_initial_attach_apn_protocol=NULL;
  char* copied_android_initial_attach_apn_username=NULL;
  char* copied_android_initial_attach_apn_password=NULL;

  RIL_SMS_WriteArgs*  android_write_sms_to_sim_msg_params;
  RIL_SMS_WriteArgs*  copied_android_write_sms_to_sim_msg_params;
  char *copied_android_write_sms_to_sim_msg_smsc_address;
  char *copied_android_write_sms_to_sim_msg_pdu;

  RIL_CallForwardInfo *android_query_call_fwd_info_params;
  RIL_CallForwardInfo *copied_android_query_call_fwd_info_params;
  char *copied_android_query_call_fwd_info_number;

  int local_four_byte_storage_occupied;

  char* copied_str;

  char** android_change_barring_pwd_params;
  char *android_ch_bar_pwd_faclity;
  char *android_ch_bar_pwd_old_pwd;
  char *android_ch_bar_pwd_new_pwd;

  char** copied_change_barring_pwd_params;
  char *copied_ch_bar_pwd_faclity;
  char *copied_ch_bar_pwd_old_pwd;
  char *copied_ch_bar_pwd_new_pwd;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_DEBUG("a-r-id %d, a-data 0x%x, a-data-len %d", android_request_id, android_request_data, (int)android_request_data_len );


  res               = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;

  len_to_go         = QMI_RIL_ZERO;
  need_common_clone = FALSE;

  local_four_byte_storage_occupied = QMI_RIL_ZERO;

  locally_created_custom_storage_len = QMI_RIL_ZERO;
  locally_created_custom_storage     = NULL;

  switch ( android_request_id )
  {


   case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        android_initial_attach_apn_params = (RIL_InitialAttachApn*)android_request_data;

        local_ok = FALSE;
        do
        {
          copied_android_initial_attach_apn_params = qcril_malloc(
                                                    sizeof(*copied_android_initial_attach_apn_params ) );

          if ( NULL == copied_android_initial_attach_apn_params )
            break;

          *copied_android_initial_attach_apn_params = *android_initial_attach_apn_params;

          //apn
          str_access = android_initial_attach_apn_params->apn;
          copied_android_initial_attach_apn_apn = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_initial_attach_apn_apn && NULL != str_access )
            break;

          //protocol
          str_access = android_initial_attach_apn_params->protocol;
          copied_android_initial_attach_apn_protocol = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_initial_attach_apn_protocol && NULL != str_access )
            break;

          //username
          str_access = android_initial_attach_apn_params->username;
          copied_android_initial_attach_apn_username = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_initial_attach_apn_username && NULL != str_access )
            break;

          //password
          str_access = android_initial_attach_apn_params->password;
          copied_android_initial_attach_apn_password = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_initial_attach_apn_password && NULL != str_access )
            break;


          copied_android_initial_attach_apn_params->apn = copied_android_initial_attach_apn_apn;
          copied_android_initial_attach_apn_params->protocol = copied_android_initial_attach_apn_protocol;
          copied_android_initial_attach_apn_params->username = copied_android_initial_attach_apn_username;
          copied_android_initial_attach_apn_params->password = copied_android_initial_attach_apn_password;

          local_ok = TRUE;
        } while (FALSE);

        if ( local_ok )
        {
          locally_created_custom_storage      = copied_android_initial_attach_apn_params;
          locally_created_custom_storage_len  = sizeof ( *copied_android_initial_attach_apn_params );

          need_common_clone = FALSE;

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
        }
        else
        {  // rollback
          if( NULL != copied_android_initial_attach_apn_params )
          {
              qcril_free( copied_android_initial_attach_apn_params );
              copied_android_initial_attach_apn_params = NULL;
          }

          if( NULL != copied_android_initial_attach_apn_apn )
          {
              qcril_free( copied_android_initial_attach_apn_apn );
              copied_android_initial_attach_apn_apn = NULL;
          }

          if( NULL != copied_android_initial_attach_apn_protocol )
          {
              qcril_free( copied_android_initial_attach_apn_protocol );
              copied_android_initial_attach_apn_protocol = NULL;
          }

          if( NULL != copied_android_initial_attach_apn_password )
          {
              qcril_free( copied_android_initial_attach_apn_password );
              copied_android_initial_attach_apn_password = NULL;
          }

          if( NULL != copied_android_initial_attach_apn_username )
          {
              qcril_free( copied_android_initial_attach_apn_username );
              copied_android_initial_attach_apn_username = NULL;
          }

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }

      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;


    case RIL_REQUEST_IMS_SEND_SMS:
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        android_send_ims_msg_params = (RIL_IMS_SMS_Message*)android_request_data;

        copied_android_send_ims_msg_params  = NULL;
        copied_android_cdma_send_ims_param  = NULL;
        copied_android_gw_smsc_address      = NULL;
        copied_android_gw_pdu               = NULL;
        copied_android_gw_sms_ims_params    = NULL;

        local_ok = FALSE;
        do
        {
          copied_android_send_ims_msg_params = qcril_malloc( sizeof( *copied_android_send_ims_msg_params ) );

          if ( NULL == copied_android_send_ims_msg_params )
            break;
          *copied_android_send_ims_msg_params = *android_send_ims_msg_params;

          if ( RADIO_TECH_3GPP2 == copied_android_send_ims_msg_params->tech )
          { // cdma
            if ( NULL != android_send_ims_msg_params->message.cdmaMessage )
            {
              copied_android_cdma_send_ims_param = qcril_malloc( sizeof( *copied_android_cdma_send_ims_param ) );
              if ( NULL == copied_android_cdma_send_ims_param )
                break;

              *copied_android_cdma_send_ims_param = *android_send_ims_msg_params->message.cdmaMessage;
              copied_android_send_ims_msg_params->message.cdmaMessage = copied_android_cdma_send_ims_param;
            }
            // else nothing - accept params as is
          }
          else
          { // gwl

            // sms sc
            str_access = ( (char **) android_send_ims_msg_params->message.gsmMessage )[ 0 ];
            copied_android_gw_smsc_address = qmi_ril_util_str_clone( str_access );
            if ( NULL == copied_android_gw_smsc_address && NULL != str_access )
              break;

            // pdu
            str_access = ( (char **) android_send_ims_msg_params->message.gsmMessage )[ 1 ];
            copied_android_gw_pdu = qmi_ril_util_str_clone( str_access );
            if ( NULL == copied_android_gw_pdu && NULL != str_access )
              break;

            copied_android_gw_sms_ims_params = qcril_malloc( 2 * sizeof( char* ) ) ;
            if ( NULL == copied_android_gw_sms_ims_params )
              break;

            copied_android_gw_sms_ims_params[0] = copied_android_gw_smsc_address;
            copied_android_gw_sms_ims_params[1] = copied_android_gw_pdu;

            copied_android_send_ims_msg_params->message.gsmMessage = copied_android_gw_sms_ims_params;
          }

          local_ok = TRUE;
        } while (FALSE);

        if ( local_ok )
        {
          locally_created_custom_storage      = copied_android_send_ims_msg_params;
          locally_created_custom_storage_len  = sizeof ( *copied_android_send_ims_msg_params );

          need_common_clone = FALSE; // we already did it

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
        }
        else
        {  // rollback
          if( NULL != copied_android_send_ims_msg_params )
          {
              qcril_free( copied_android_send_ims_msg_params );
              copied_android_send_ims_msg_params = NULL;
          }

          if( NULL != copied_android_cdma_send_ims_param )
          {
              qcril_free( copied_android_cdma_send_ims_param );
              copied_android_cdma_send_ims_param = NULL;
          }

          if( NULL != copied_android_gw_smsc_address )
          {
              qcril_free( copied_android_gw_smsc_address );
              copied_android_gw_smsc_address = NULL;
          }

          if( NULL != copied_android_gw_pdu )
          {
              qcril_free( copied_android_gw_pdu );
              copied_android_gw_pdu = NULL;
          }

          if( NULL != copied_android_gw_sms_ims_params )
          {
              qcril_free( copied_android_gw_sms_ims_params );
              copied_android_gw_sms_ims_params = NULL;
          }

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }

      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_SEND_SMS:              // fallthrough
    case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
      if (  NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO  )
      {
        android_send_sms_params = (char**)android_request_data;

        local_ok = FALSE;

        copied_android_gw_smsc_address    = NULL;
        copied_android_gw_pdu             = NULL;
        copied_android_gw_sms_ims_params  = NULL;

        do
        {
          // sms sc
          str_access = android_send_sms_params[ 0 ];
          copied_android_gw_smsc_address = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_gw_smsc_address && NULL != str_access )
            break;

          // pdu
          str_access = android_send_sms_params[ 1 ];
          copied_android_gw_pdu = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_gw_pdu && NULL != str_access )
            break;

          copied_android_gw_sms_ims_params = qcril_malloc( 2 * sizeof( char* ) ) ;
          if ( NULL == copied_android_gw_sms_ims_params )
            break;

          copied_android_gw_sms_ims_params[0] = copied_android_gw_smsc_address;
          copied_android_gw_sms_ims_params[1] = copied_android_gw_pdu;

          local_ok = TRUE;

        } while ( FALSE );

        if ( local_ok )
        {
          locally_created_custom_storage      = copied_android_gw_sms_ims_params;
          locally_created_custom_storage_len  = 2 * sizeof( char* );

          need_common_clone = FALSE; // we already did it

          QCRIL_LOG_DEBUG("sms allo 0x%x, 0x%x, 0x%x", copied_android_gw_sms_ims_params, copied_android_gw_smsc_address, copied_android_gw_pdu);

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
        }
        else
        {  // rollback
          if( NULL != copied_android_gw_smsc_address )
          {
              qcril_free( copied_android_gw_smsc_address );
              copied_android_gw_smsc_address = NULL;
          }

          if( NULL != copied_android_gw_pdu )
          {
              qcril_free( copied_android_gw_pdu );
              copied_android_gw_pdu = NULL;
          }

          if( NULL != copied_android_gw_sms_ims_params )
          {
              qcril_free( copied_android_gw_sms_ims_params );
              copied_android_gw_sms_ims_params = NULL;
          }

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_CDMA_SEND_SMS:
      if ( NULL != android_request_data )
      {
        len_to_go  = sizeof( RIL_CDMA_SMS_Message );

        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;

        need_common_clone = TRUE;
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_WRITE_SMS_TO_SIM:
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        android_write_sms_to_sim_msg_params = (RIL_SMS_WriteArgs*)android_request_data;

        copied_android_write_sms_to_sim_msg_params  = NULL;
        copied_android_write_sms_to_sim_msg_smsc_address = NULL;
        copied_android_write_sms_to_sim_msg_pdu = NULL;

        local_ok = FALSE;
        do
        {
          copied_android_write_sms_to_sim_msg_params = qcril_malloc( sizeof( *copied_android_write_sms_to_sim_msg_params ) );

          if ( NULL == copied_android_write_sms_to_sim_msg_params )
            break;
          *copied_android_write_sms_to_sim_msg_params = *android_write_sms_to_sim_msg_params;

          // sms sc
          str_access = android_write_sms_to_sim_msg_params->smsc;
          copied_android_write_sms_to_sim_msg_smsc_address = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_write_sms_to_sim_msg_smsc_address && NULL != str_access )
            break;

          // pdu
          str_access = android_write_sms_to_sim_msg_params->pdu;
          copied_android_write_sms_to_sim_msg_pdu = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_write_sms_to_sim_msg_pdu && NULL != str_access )
            break;

          copied_android_write_sms_to_sim_msg_params->smsc = copied_android_write_sms_to_sim_msg_smsc_address;
          copied_android_write_sms_to_sim_msg_params->pdu = copied_android_write_sms_to_sim_msg_pdu;

          local_ok = TRUE;
        } while (FALSE);

        if ( local_ok )
        {
          locally_created_custom_storage      = copied_android_write_sms_to_sim_msg_params;
          locally_created_custom_storage_len  = sizeof ( *copied_android_write_sms_to_sim_msg_params );

          need_common_clone = FALSE; // we already did it

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
        }
        else
        {  // rollback
          if( NULL != copied_android_write_sms_to_sim_msg_params )
          {
              qcril_free( copied_android_write_sms_to_sim_msg_params );
              copied_android_write_sms_to_sim_msg_params = NULL;
          }

          if( NULL != copied_android_write_sms_to_sim_msg_smsc_address )
          {
              qcril_free( copied_android_write_sms_to_sim_msg_smsc_address );
              copied_android_write_sms_to_sim_msg_smsc_address = NULL;
          }

          if( NULL != copied_android_write_sms_to_sim_msg_pdu )
          {
              qcril_free( copied_android_write_sms_to_sim_msg_pdu );
              copied_android_write_sms_to_sim_msg_pdu = NULL;
          }

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }

      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_SET_SMSC_ADDRESS:                    // fallthrough
    case RIL_REQUEST_SEND_USSD:
      // str
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        str_access = (char*)android_request_data;
        len_to_go  = strlen( str_access )  + QMI_RIL_SINGLE;

        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;

        need_common_clone = TRUE;
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL:
      /* only legacy format with RAT needs */
      if ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_LEGACY_RAT) )
      {
        if (  NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO  )
        {
          android_manual_selection_params = (char**)android_request_data;

          local_ok = FALSE;

          copied_android_manual_selection_mcc_mnc     = NULL;
          copied_android_manual_selection_rat         = NULL;
          copied_android_manual_selection_params      = NULL;

          do
          {
            // mcc mnc
            str_access = android_manual_selection_params[ 0 ];
            copied_android_manual_selection_mcc_mnc = qmi_ril_util_str_clone( str_access );
            if ( NULL == copied_android_manual_selection_mcc_mnc && NULL != str_access )
              break;

            // rat
            str_access = android_manual_selection_params[ 1 ];
            copied_android_manual_selection_rat = qmi_ril_util_str_clone( str_access );
            if ( NULL == copied_android_manual_selection_rat && NULL != str_access )
              break;

            copied_android_manual_selection_params = qcril_malloc( 2 * sizeof( char* ) ) ;
            if ( NULL == copied_android_manual_selection_params )
              break;

            copied_android_manual_selection_params[0] = copied_android_manual_selection_mcc_mnc;
            copied_android_manual_selection_params[1] = copied_android_manual_selection_rat;

            local_ok = TRUE;

          } while ( FALSE );

          if ( local_ok )
          {
            locally_created_custom_storage      = copied_android_manual_selection_params;
            locally_created_custom_storage_len  = 2 * sizeof( char* );

            need_common_clone = FALSE; // we already did it

            QCRIL_LOG_DEBUG("manual sel allo 0x%x, 0x%x, 0x%x", copied_android_manual_selection_params, copied_android_manual_selection_mcc_mnc, copied_android_manual_selection_rat);

            res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
          }
          else
          {  // rollback
            if( NULL != copied_android_manual_selection_mcc_mnc )
            {
                qcril_free( copied_android_manual_selection_mcc_mnc );
                copied_android_manual_selection_mcc_mnc = NULL;
            }

            if( NULL != copied_android_manual_selection_rat )
            {
                qcril_free( copied_android_manual_selection_rat );
                copied_android_manual_selection_rat = NULL;
            }

            if( NULL != copied_android_manual_selection_params )
            {
                qcril_free( copied_android_manual_selection_params );
                copied_android_manual_selection_params = NULL;
            }

            res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
          }
        }
        else
        {
          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }
      }
      else
      {
        // str
        if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
        {
          str_access = (char*)android_request_data;
          len_to_go  = strlen( str_access )  + QMI_RIL_SINGLE;

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;

          need_common_clone = TRUE;
        }
        else
        {
          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }
      }
      break;

    case RIL_REQUEST_SET_UICC_SUBSCRIPTION:
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        len_to_go  = sizeof( RIL_SelectUiccSub );

        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;

        need_common_clone = TRUE;
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_QUERY_CALL_WAITING:            // fallthrough
    case RIL_REQUEST_SET_TTY_MODE:                  // fallthrough
    case RIL_REQUEST_SET_SUBSCRIPTION_MODE:         // fallthrough
    case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE:   // fallthrough
    case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE:  // fallthrough
    case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:    // fallthrough
    case RIL_REQUEST_SET_BAND_MODE:                 // fallthrough
    case RIL_REQUEST_SET_LOCATION_UPDATES:          // fallthrough
    case RIL_REQUEST_SCREEN_STATE:                  // fallthrough
    case RIL_REQUEST_SEPARATE_CONNECTION:           // fallthrough
    case RIL_REQUEST_HANGUP:                        // fallthrough
    case RIL_REQUEST_RADIO_POWER:                   // fallthrough
    case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED:
      // int
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        len_to_go  = sizeof( int );

        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;

        need_common_clone = TRUE;
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_DTMF:
    case RIL_REQUEST_DTMF_START:
      // single char
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        len_to_go  = sizeof( char );

        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;

        need_common_clone = TRUE;
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_CDMA_BURST_DTMF:
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        copied_cdma_dtmf_holder  = NULL;
        copied_cdma_dtmf_str     = NULL;
        copied_cdma_dtmf_on      = NULL;
        copied_cdma_dtmf_off     = NULL;
        android_cdma_dtmf_holder = (char**)android_request_data;

        local_ok                 = FALSE;

        do
        {
          copied_cdma_dtmf_holder = qcril_malloc( 3 * sizeof(char*) );
          if ( NULL == copied_cdma_dtmf_holder )
            break;

          copied_cdma_dtmf_str = qmi_ril_util_str_clone( android_cdma_dtmf_holder[0] );
          if ( NULL == copied_cdma_dtmf_str )
            break;

          copied_cdma_dtmf_on = qmi_ril_util_str_clone( android_cdma_dtmf_holder[1] );
          if ( NULL == copied_cdma_dtmf_on )
            break;

          copied_cdma_dtmf_off = qmi_ril_util_str_clone( android_cdma_dtmf_holder[2] );
          if ( NULL == copied_cdma_dtmf_off )
            break;

          copied_cdma_dtmf_holder[0] = copied_cdma_dtmf_str;
          copied_cdma_dtmf_holder[1] = copied_cdma_dtmf_on;
          copied_cdma_dtmf_holder[2] = copied_cdma_dtmf_off;

          local_ok = TRUE;

        } while (FALSE);

        if ( local_ok )
        {
          res_inclanation   = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
          need_common_clone = FALSE;

          locally_created_custom_storage      = copied_cdma_dtmf_holder;
          locally_created_custom_storage_len  = 3 * sizeof( char* );
        }
        else
        {
          if( NULL != copied_cdma_dtmf_off )
          {
              qcril_free( copied_cdma_dtmf_off );
              copied_cdma_dtmf_off = NULL;
          }

          if( NULL != copied_cdma_dtmf_on )
          {
              qcril_free( copied_cdma_dtmf_on );
              copied_cdma_dtmf_on = NULL;
          }

          if( NULL != copied_cdma_dtmf_str )
          {
              qcril_free( copied_cdma_dtmf_str );
              copied_cdma_dtmf_str = NULL;
          }

          if( NULL != copied_cdma_dtmf_holder )
          {
              qcril_free( copied_cdma_dtmf_holder );
              copied_cdma_dtmf_holder = NULL;
          }

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY:
      res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        copied_str = qcril_malloc( 26 + 1 ); // 26 per ril.h
        if ( NULL != copied_str )
        {
          locally_created_custom_storage              = copied_str;
          locally_created_custom_storage_len          = 26 + 1;
          memcpy( locally_created_custom_storage, android_request_data, 26 ); // without + 1

          need_common_clone                           = FALSE;
          res_inclanation                             = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
        }
      }
      break;

    case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS:     // fallthough
    case RIL_REQUEST_SET_CALL_FORWARD:
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        android_query_call_fwd_info_params = (RIL_CallForwardInfo*)android_request_data;

        copied_android_query_call_fwd_info_params = NULL;
        copied_android_query_call_fwd_info_number = NULL;

        local_ok = FALSE;
        do
        {
          copied_android_query_call_fwd_info_params = qcril_malloc( sizeof( *copied_android_query_call_fwd_info_params ) );

          if ( NULL == copied_android_query_call_fwd_info_params )
            break;
          *copied_android_query_call_fwd_info_params = *android_query_call_fwd_info_params;

          // number
          str_access = android_query_call_fwd_info_params->number;
          copied_android_query_call_fwd_info_number = qmi_ril_util_str_clone( str_access );
          if ( NULL == copied_android_query_call_fwd_info_number && NULL != str_access )
            break;

          copied_android_query_call_fwd_info_params->number = copied_android_query_call_fwd_info_number;

          local_ok = TRUE;
        } while (FALSE);

        if ( local_ok )
        {
          locally_created_custom_storage      = copied_android_query_call_fwd_info_params;
          locally_created_custom_storage_len  = sizeof ( *copied_android_query_call_fwd_info_params );

          need_common_clone = FALSE; // we already did it

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
        }
        else
        {  // rollback
          if ( NULL != copied_android_query_call_fwd_info_params )
          {
            qcril_free( copied_android_query_call_fwd_info_params );
            copied_android_query_call_fwd_info_params = NULL;
          }
          if ( NULL != copied_android_query_call_fwd_info_number )
          {
            qcril_free( copied_android_query_call_fwd_info_number );
            copied_android_query_call_fwd_info_number = NULL;
          }

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_CHANGE_BARRING_PASSWORD:
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {

        android_change_barring_pwd_params = (char**)android_request_data;

        local_ok = FALSE;

        copied_change_barring_pwd_params = NULL;
        copied_ch_bar_pwd_faclity        = NULL;
        copied_ch_bar_pwd_old_pwd        = NULL;
        copied_ch_bar_pwd_new_pwd        = NULL;

        do
        {
          // facility
          android_ch_bar_pwd_faclity = android_change_barring_pwd_params[ 0 ];
          copied_ch_bar_pwd_faclity = qmi_ril_util_str_clone( android_ch_bar_pwd_faclity );
          if ( NULL == copied_ch_bar_pwd_faclity && NULL != android_ch_bar_pwd_faclity )
            break;

          // old pwd
          android_ch_bar_pwd_old_pwd = android_change_barring_pwd_params[ 1 ];
          copied_ch_bar_pwd_old_pwd = qmi_ril_util_str_clone( android_ch_bar_pwd_old_pwd );
          if ( NULL == copied_ch_bar_pwd_old_pwd && NULL != android_ch_bar_pwd_old_pwd )
            break;

          // new pwd
          android_ch_bar_pwd_new_pwd = android_change_barring_pwd_params[ 2 ];
          copied_ch_bar_pwd_new_pwd = qmi_ril_util_str_clone( android_ch_bar_pwd_new_pwd );
          if ( NULL == copied_ch_bar_pwd_new_pwd && NULL != android_ch_bar_pwd_new_pwd )
            break;

          copied_change_barring_pwd_params = qcril_malloc( 3 * sizeof( char* ) ) ;
          if ( NULL == copied_change_barring_pwd_params )
            break;

          copied_change_barring_pwd_params[0] = copied_ch_bar_pwd_faclity;
          copied_change_barring_pwd_params[1] = copied_ch_bar_pwd_old_pwd;
          copied_change_barring_pwd_params[2] = copied_ch_bar_pwd_new_pwd;

          local_ok = TRUE;

        } while ( FALSE );

        if ( local_ok )
        {
          locally_created_custom_storage      = copied_change_barring_pwd_params;
          locally_created_custom_storage_len  = 3 * sizeof( char* );

          need_common_clone = FALSE; // we already did it

          QCRIL_LOG_DEBUG("change bar pwd alloc 0x%x", copied_change_barring_pwd_params );

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
        }
        else
        {  // rollback
          if ( NULL != copied_change_barring_pwd_params )
          {
            qcril_free( copied_change_barring_pwd_params );
          }

          if ( NULL != copied_ch_bar_pwd_faclity )
          {
            qcril_free( copied_ch_bar_pwd_faclity );
          }

          if ( NULL != copied_ch_bar_pwd_old_pwd )
          {
            qcril_free( copied_ch_bar_pwd_old_pwd );
          }

          if ( NULL != copied_ch_bar_pwd_new_pwd )
          {
            qcril_free( copied_ch_bar_pwd_new_pwd );
          }

          copied_change_barring_pwd_params = NULL;
          copied_ch_bar_pwd_faclity        = NULL;
          copied_ch_bar_pwd_old_pwd        = NULL;
          copied_ch_bar_pwd_new_pwd        = NULL;

          res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE:
      if ( NULL != android_request_data )
      {
        len_to_go  = sizeof( RIL_CDMA_SMS_Ack );

        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;

        need_common_clone = TRUE;
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    case RIL_REQUEST_SET_CALL_WAITING:
    case RIL_REQUEST_SMS_ACKNOWLEDGE:
      if ( NULL != android_request_data && android_request_data_len > QMI_RIL_ZERO )
      {
        len_to_go  = 2 * sizeof( int );

        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;

        need_common_clone = TRUE;
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      }
      break;

    default:
      if ( NULL == android_request_data )
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION;
        len_to_go       = QMI_RIL_ZERO;
      }
      else if ( android_request_data_len <= QMI_RIL_FOUR_BYTES )
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT;
        len_to_go       = android_request_data_len;
      }
      else
      {
        res_inclanation = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
        len_to_go       = android_request_data_len;
      }
      need_common_clone = TRUE;
      break;
  }

  switch ( res_inclanation )
  {
    case QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY:
      if ( need_common_clone )
      {
        if ( len_to_go > QMI_RIL_ZERO )
        {
          locally_created_custom_storage = qcril_malloc( len_to_go );
          if ( NULL != locally_created_custom_storage )
          {
            memcpy( locally_created_custom_storage, android_request_data, len_to_go );
            locally_created_custom_storage_len = len_to_go;

            res = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
          }
          else
          {
            res = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
          }
        }
        else
        { // unexpected
          res = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
        }
      }
      else
      { // ready to go
        res = QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY;
      }
      break;

    case QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT:
      if ( need_common_clone && len_to_go > QMI_RIL_ZERO && NULL != four_byte_storage)
      {
        memcpy( four_byte_storage, android_request_data, len_to_go );
      }
      local_four_byte_storage_occupied = len_to_go;
      res = QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT;
      break;

    case QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION:
      res = QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION;
      break;

    default:
      res = QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID;
      break;
  }

  if ( NULL != four_byte_storage_occupied )
  {
    *four_byte_storage_occupied = local_four_byte_storage_occupied;
  }
  if ( NULL != sub_created_custom_storage )
  {
    *sub_created_custom_storage = locally_created_custom_storage;
  }
  if ( NULL != custom_storage_len )
  {
    *custom_storage_len = locally_created_custom_storage_len;
  }

  switch ( res )
  {
    case QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT:
      QCRIL_LOG_INFO(".. params for Android request id %d are of plain structure and cloned to preallocated queue buf taking length of %d",
                        android_request_id, (int) local_four_byte_storage_occupied  );
      break;

    case QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY:
      QCRIL_LOG_INFO(".. params for Android request id %d are of complex structure and cloned to 0x%x length %d",
                        android_request_id, locally_created_custom_storage, (int) locally_created_custom_storage_len );
      break;

    case QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION:
      QCRIL_LOG_INFO(".. params for Android request id %d require no copy action (usually means empty payload)", android_request_id);
      break;

    case QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID:  // fallthrough
    default:
      QCRIL_LOG_INFO(".. params for Android request id %d either do no require copying or unsupported case", android_request_id);
      break;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)res );

  return res;
} // qmi_ril_fw_create_android_live_params_copy

//===========================================================================
// qmi_ril_fw_destroy_android_live_params_copy
//===========================================================================
void qmi_ril_fw_destroy_android_live_params_copy(qmi_ril_fw_android_param_copy_approach_type used_approach,
                                               int android_request_id,
                                               void* four_byte_storage,
                                               void* sub_created_custom_storage)
{
  RIL_IMS_SMS_Message*  copied_android_send_ims_msg_params;
  RIL_CDMA_SMS_Message* copied_android_cdma_send_ims_param;
  char *copied_android_gw_smsc_address;
  char *copied_android_gw_pdu;
  char ** copied_android_gw_sms_ims_params;

  char** copied_cdma_dtmf_holder;
  char* copied_cdma_dtmf_str;
  char* copied_cdma_dtmf_on;
  char* copied_cdma_dtmf_off;

  RIL_SMS_WriteArgs*  copied_android_write_sms_to_sim_msg_params;
  char *copied_android_write_sms_to_sim_msg_smsc_address;
  char *copied_android_write_sms_to_sim_msg_pdu;

  RIL_InitialAttachApn* copied_android_initial_attach_apn_params;
  char* copied_android_initial_attach_apn_apn;
  char* copied_android_initial_attach_apn_protocol;
  char* copied_android_initial_attach_apn_username;
  char* copied_android_initial_attach_apn_password;


  char *copied_android_manual_selection_mcc_mnc;
  char *copied_android_manual_selection_rat;
  char ** copied_android_manual_selection_params;

  RIL_CallForwardInfo* copied_android_query_call_fwd_status_params;
  char *copied_android_query_call_fwd_status_number;

  char** copied_change_barring_pwd_params;
  char *copied_ch_bar_pwd_faclity;
  char *copied_ch_bar_pwd_old_pwd;
  char *copied_ch_bar_pwd_new_pwd;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_INFO("action to destroy cloned Android request parameters, a-r-id %d, appron %d", (int)android_request_id, (int)used_approach );


  switch ( android_request_id )
  {
    case RIL_REQUEST_IMS_SEND_SMS:
      if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
      {
        copied_android_send_ims_msg_params = (RIL_IMS_SMS_Message*)sub_created_custom_storage;
        if ( RADIO_TECH_3GPP2 == copied_android_send_ims_msg_params->tech )
        {
          copied_android_cdma_send_ims_param = copied_android_send_ims_msg_params->message.cdmaMessage;
          if ( NULL != copied_android_cdma_send_ims_param )
          {
            qcril_free( copied_android_cdma_send_ims_param );
            copied_android_cdma_send_ims_param = NULL;
          }
        }
        else
        {
          copied_android_gw_sms_ims_params = copied_android_send_ims_msg_params->message.gsmMessage;

          if ( NULL != copied_android_gw_sms_ims_params )
          {
            copied_android_gw_smsc_address = copied_android_gw_sms_ims_params[0];
            copied_android_gw_pdu          = copied_android_gw_sms_ims_params[1];

            if ( NULL != copied_android_gw_smsc_address )
            {
              qcril_free( copied_android_gw_smsc_address );
              copied_android_gw_smsc_address = NULL;
            }
            if ( NULL != copied_android_gw_pdu )
            {
              qcril_free( copied_android_gw_pdu );
              copied_android_gw_pdu = NULL;
            }

            qcril_free( copied_android_gw_sms_ims_params );
            copied_android_gw_sms_ims_params = NULL;
          }


        }

        qcril_free( copied_android_send_ims_msg_params );
        copied_android_send_ims_msg_params = NULL;
      }
      break;

    case RIL_REQUEST_SEND_SMS:              // fallthrough
    case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
      if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
      {
        copied_android_gw_sms_ims_params = (char**)sub_created_custom_storage;
        copied_android_gw_smsc_address   = copied_android_gw_sms_ims_params[0];
        copied_android_gw_pdu            = copied_android_gw_sms_ims_params[1];

        QCRIL_LOG_DEBUG("sms allo 0x%x, 0x%x, 0x%x", copied_android_gw_sms_ims_params, copied_android_gw_smsc_address, copied_android_gw_pdu);

        if( NULL != copied_android_gw_pdu )
        {
            qcril_free( copied_android_gw_pdu );
            copied_android_gw_pdu = NULL;
        }

        if( NULL != copied_android_gw_smsc_address )
        {
            qcril_free( copied_android_gw_smsc_address );
            copied_android_gw_smsc_address = NULL;
        }

        qcril_free( copied_android_gw_sms_ims_params );
        copied_android_gw_sms_ims_params = NULL;
      }
      break;

    case RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL:
      /* only legacy format with RAT needs */
      if ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_LEGACY_RAT) )
      {
        if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
        {
          copied_android_manual_selection_params    = (char**)sub_created_custom_storage;
          copied_android_manual_selection_mcc_mnc   = copied_android_manual_selection_params[0];
          copied_android_manual_selection_rat       = copied_android_manual_selection_params[1];

          QCRIL_LOG_DEBUG("manual sel allo 0x%x, 0x%x, 0x%x", copied_android_manual_selection_params, copied_android_manual_selection_mcc_mnc, copied_android_manual_selection_rat);

          if( NULL != copied_android_manual_selection_mcc_mnc )
          {
              qcril_free( copied_android_manual_selection_mcc_mnc );
              copied_android_manual_selection_mcc_mnc = NULL;
          }

          if( NULL != copied_android_manual_selection_rat )
          {
              qcril_free( copied_android_manual_selection_rat );
              copied_android_manual_selection_rat = NULL;
          }

          qcril_free( copied_android_manual_selection_params );
          copied_android_manual_selection_params = NULL;
        }
      }
      else
      {
        if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
        {
          qcril_free( sub_created_custom_storage );
          sub_created_custom_storage = NULL;
        }
      }
      break;



    case RIL_REQUEST_CDMA_BURST_DTMF:
      if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
      {
        copied_cdma_dtmf_holder = (char**)sub_created_custom_storage;

        copied_cdma_dtmf_str = copied_cdma_dtmf_holder[0];
        copied_cdma_dtmf_on  = copied_cdma_dtmf_holder[1];
        copied_cdma_dtmf_off = copied_cdma_dtmf_holder[2];

        if( NULL != copied_cdma_dtmf_off )
        {
            qcril_free( copied_cdma_dtmf_off );
            copied_cdma_dtmf_off = NULL;
        }

        if( NULL != copied_cdma_dtmf_on )
        {
            qcril_free( copied_cdma_dtmf_on );
            copied_cdma_dtmf_on = NULL;
        }

        if( NULL != copied_cdma_dtmf_str )
        {
            qcril_free( copied_cdma_dtmf_str );
            copied_cdma_dtmf_str = NULL;
        }

        qcril_free( copied_cdma_dtmf_holder );
        copied_cdma_dtmf_holder = NULL;
      }
      break;

    case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
      if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
      {
        copied_android_initial_attach_apn_params = (RIL_InitialAttachApn*)sub_created_custom_storage;

        copied_android_initial_attach_apn_apn = copied_android_initial_attach_apn_params->apn;
        copied_android_initial_attach_apn_protocol = copied_android_initial_attach_apn_params->protocol;
        copied_android_initial_attach_apn_username = copied_android_initial_attach_apn_params->username;
        copied_android_initial_attach_apn_password = copied_android_initial_attach_apn_params->password;

        if ( NULL != copied_android_initial_attach_apn_apn )
        {
          qcril_free( copied_android_initial_attach_apn_apn );
          copied_android_initial_attach_apn_apn = NULL;
        }

        if ( NULL != copied_android_initial_attach_apn_protocol )
        {
          qcril_free( copied_android_initial_attach_apn_protocol );
          copied_android_initial_attach_apn_protocol = NULL;
        }

        if ( NULL != copied_android_initial_attach_apn_username )
        {
          qcril_free( copied_android_initial_attach_apn_username );
          copied_android_initial_attach_apn_username = NULL;
        }


        if ( NULL != copied_android_initial_attach_apn_password )
        {
          qcril_free( copied_android_initial_attach_apn_password );
          copied_android_initial_attach_apn_password = NULL;
        }


        qcril_free( copied_android_initial_attach_apn_params );
        copied_android_initial_attach_apn_params = NULL;
      }
      break;

    case RIL_REQUEST_WRITE_SMS_TO_SIM:
      if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
      {
        copied_android_write_sms_to_sim_msg_params = (RIL_SMS_WriteArgs*)sub_created_custom_storage;

        copied_android_write_sms_to_sim_msg_smsc_address = copied_android_write_sms_to_sim_msg_params->smsc;
        copied_android_write_sms_to_sim_msg_pdu = copied_android_write_sms_to_sim_msg_params->pdu;

        if ( NULL != copied_android_write_sms_to_sim_msg_smsc_address )
        {
          qcril_free( copied_android_write_sms_to_sim_msg_smsc_address );
          copied_android_write_sms_to_sim_msg_smsc_address = NULL;
        }

        if ( NULL != copied_android_write_sms_to_sim_msg_pdu )
        {
          qcril_free( copied_android_write_sms_to_sim_msg_pdu );
          copied_android_write_sms_to_sim_msg_pdu = NULL;
        }

        qcril_free( copied_android_write_sms_to_sim_msg_params );
        copied_android_write_sms_to_sim_msg_params = NULL;
      }
      break;

    case RIL_REQUEST_SET_CALL_FORWARD:            // fallthrough
    case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS:
      if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
      {
        copied_android_query_call_fwd_status_params = (RIL_CallForwardInfo*)sub_created_custom_storage;

        if ( NULL != copied_android_query_call_fwd_status_params )
        {
          copied_android_query_call_fwd_status_number = copied_android_query_call_fwd_status_params->number;

          if ( NULL != copied_android_query_call_fwd_status_number )
          {
            qcril_free( copied_android_query_call_fwd_status_number );
            copied_android_query_call_fwd_status_number = NULL;
          }
          qcril_free( copied_android_query_call_fwd_status_params );
          copied_android_query_call_fwd_status_params = NULL;
        }
      }
      break;

    case RIL_REQUEST_CHANGE_BARRING_PASSWORD:
      if ( NULL != sub_created_custom_storage && QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY == used_approach )
      {
        copied_change_barring_pwd_params = (char**)sub_created_custom_storage;

        if ( NULL != copied_change_barring_pwd_params )
        {
          copied_ch_bar_pwd_faclity = copied_change_barring_pwd_params[0];
          copied_ch_bar_pwd_old_pwd = copied_change_barring_pwd_params[1];
          copied_ch_bar_pwd_new_pwd = copied_change_barring_pwd_params[2];

          if ( NULL != copied_ch_bar_pwd_faclity )
          {
            qcril_free( copied_ch_bar_pwd_faclity );
            copied_ch_bar_pwd_faclity = NULL;
          }

          if ( NULL != copied_ch_bar_pwd_old_pwd )
          {
            qcril_free( copied_ch_bar_pwd_old_pwd );
            copied_ch_bar_pwd_old_pwd = NULL;
          }

          if ( NULL != copied_ch_bar_pwd_new_pwd )
          {
            qcril_free( copied_ch_bar_pwd_new_pwd );
            copied_ch_bar_pwd_new_pwd = NULL;
          }

          qcril_free( copied_change_barring_pwd_params );
          copied_change_barring_pwd_params = NULL;
        }
      }
      break;

    default:
      switch ( used_approach )
      {
        case QMI_RIL_ANDROID_PARAM_CPY_APPRON_4_BYTES_SNAPSHOT:
          if ( NULL != four_byte_storage )
          {
            memset( four_byte_storage, 0, 4 );
          }
          break;

        case QMI_RIL_ANDROID_PARAM_CPY_APPRON_DYNAMIC_COPY:
          if ( NULL != sub_created_custom_storage )
          {
            qcril_free( sub_created_custom_storage );
            sub_created_custom_storage = NULL;
          }
          break;

        case QMI_RIL_ANDROID_PARAM_CPY_APPRON_EMPTY_NO_ACTION: // no action
          break;

        default: // no action
          break;
      }
      break;
  }

  QCRIL_LOG_FUNC_RETURN();

} // qmi_ril_fw_destroy_android_live_params_copy

//===========================================================================
// qmi_ril_util_str_clone
//===========================================================================
char* qmi_ril_util_str_clone( char * org_str )
{
  char* res;
  int   len;

  if ( NULL != org_str )
  {
    len = strlen( org_str );
    res = qcril_malloc( len + 1 );
    if ( NULL != res )
    {
      memcpy( res, org_str, len + 1 );
    }
  }
  else
  {
    res = NULL;
  }

  return res;
} // qmi_ril_util_str_clone

//===========================================================================
// qmi_ril_android_pending_unsol_resp_lock
//===========================================================================
void qmi_ril_android_pending_unsol_resp_lock( void )
{
  pthread_mutex_lock( &qmi_ril_android_pending_unsol_resp.pending_unsol_resp_mutex );
} // qmi_ril_android_pending_unsol_resp_lock

//===========================================================================
// qmi_ril_android_pending_unsol_resp_unlock
//===========================================================================
void qmi_ril_android_pending_unsol_resp_unlock( void )
{
  pthread_mutex_unlock( &qmi_ril_android_pending_unsol_resp.pending_unsol_resp_mutex );
} // qmi_ril_android_pending_unsol_resp_unlock

//===========================================================================
// qmi_ril_init_android_unsol_resp_pending_list
//===========================================================================
void qmi_ril_init_android_unsol_resp_pending_list()
{
  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list
    = qcril_malloc(sizeof(qmi_ril_android_pending_unsol_resp_type) *
                   (QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID - RIL_UNSOL_RESPONSE_BASE + 1) );
  if ( NULL == qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list )
  {
    QCRIL_LOG_ERROR("malloc failed!");
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_init_android_unsol_resp_pending_list

//===========================================================================
// qmi_ril_reset_android_unsol_resp_dispatchable_table
//===========================================================================
void qmi_ril_reset_android_unsol_resp_dispatchable_table()
{
  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_android_pending_unsol_resp_lock();
  int i;
  for ( i=0; i<QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID - RIL_UNSOL_RESPONSE_BASE + 1; i++ )
  {
    qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[i].num_of_locker = 0;
  }
  qmi_ril_android_pending_unsol_resp_unlock();

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_reset_android_unsol_resp_dispatchable_table

//===========================================================================
// qmi_ril_free_pending_unsol_resp
//===========================================================================
void qmi_ril_free_pending_unsol_resp(qmi_ril_android_pending_unsol_resp_type* resp, int resp_id)
{
  QCRIL_LOG_FUNC_ENTRY();

  if ( resp->valid )
  {
    resp->valid = FALSE;
    if ( resp->logstr )
    {
      qcril_free((void*) resp->logstr);
    }
    qmi_ril_fw_destroy_android_live_params_copy(resp->param_copy_arrron, resp_id, resp->copied_params, resp->copied_params);
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_free_pending_unsol_resp

//===========================================================================
// qmi_ril_suppress_android_unsol_resp
//===========================================================================
void qmi_ril_suppress_android_unsol_resp(int resp_id)
{
  QCRIL_LOG_FUNC_ENTRY();
  if ( resp_id < RIL_UNSOL_RESPONSE_BASE || resp_id > QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID)
  {
    QCRIL_LOG_ERROR("invalid android_unsol_resp id %d", resp_id);
  }
  else
  {
    qmi_ril_android_pending_unsol_resp_lock();
    QCRIL_LOG_INFO("suppressing android_unsol_resp %d", resp_id);
    qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[resp_id-RIL_UNSOL_RESPONSE_BASE].num_of_locker++;
    qmi_ril_android_pending_unsol_resp_unlock();
  }
  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_suppress_android_unsol_resp

//===========================================================================
// qmi_ril_unsuppress_android_unsol_resp
//===========================================================================
void qmi_ril_unsuppress_android_unsol_resp(int resp_id)
{
  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( resp_id < RIL_UNSOL_RESPONSE_BASE || resp_id > QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID)
    {
      QCRIL_LOG_ERROR("invalid android_unsol_resp id %d", resp_id);
      break;
    }

    qmi_ril_android_pending_unsol_resp_lock();
    QCRIL_LOG_INFO("unsuppressing android_unsol_resp %d", resp_id);

    if ( qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[resp_id-RIL_UNSOL_RESPONSE_BASE].num_of_locker > 0 )
    {
      QCRIL_LOG_INFO("unsol_resp %d previous locker number %d", resp_id, qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[resp_id-RIL_UNSOL_RESPONSE_BASE].num_of_locker);
      qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[resp_id-RIL_UNSOL_RESPONSE_BASE].num_of_locker--;
      if ( 0 == qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[resp_id-RIL_UNSOL_RESPONSE_BASE].num_of_locker )
      {
        qmi_ril_android_pending_unsol_resp_type* pending_unsol_resp = qmi_ril_get_unsol_resp_from_pending_list(resp_id);
        if ( NULL != pending_unsol_resp )
        {
          qcril_unsol_resp_params_type resp_param;

          qcril_default_unsol_resp_params( pending_unsol_resp->instance_id,
                                           resp_id,
                                           &resp_param );

          resp_param.logstr = pending_unsol_resp->logstr;
          resp_param.resp_pkt = pending_unsol_resp->copied_params;
          resp_param.resp_len = pending_unsol_resp->copied_param_len;
          qcril_send_unsol_response_epilog(&resp_param);

          qmi_ril_free_pending_unsol_resp(pending_unsol_resp, resp_id);
        }
      }
    }
    else
    {
      QCRIL_LOG_DEBUG("Didn't supress android_unsol_resp %d", resp_id);
    }

    qmi_ril_android_pending_unsol_resp_unlock();
  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_unsuppress_android_unsol_resp

//===========================================================================
// qmi_ril_reset_unsol_resp_pending_list
//===========================================================================
void qmi_ril_reset_unsol_resp_pending_list()
{
  int i;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_android_pending_unsol_resp_lock();

  for ( i = RIL_UNSOL_RESPONSE_BASE; i <= QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID; i++)
  {
    qmi_ril_free_pending_unsol_resp(&qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[i-RIL_UNSOL_RESPONSE_BASE], i);
  }

  qmi_ril_android_pending_unsol_resp_unlock();
} // qmi_ril_reset_unsol_resp_pending_list

//===========================================================================
// qmi_ril_add_unsol_resp_to_pending_list
//===========================================================================
void qmi_ril_add_unsol_resp_to_pending_list(qcril_unsol_resp_params_type *param_ptr)
{
  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_android_pending_unsol_resp_type *resp;
  int resp_id = param_ptr->response_id;

  do
  {
    if ( resp_id < RIL_UNSOL_RESPONSE_BASE || resp_id > QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID)
    {
      QCRIL_LOG_ERROR("invalid android_unsol_resp id %d", resp_id);
      break;
    }

    resp = &qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[resp_id-RIL_UNSOL_RESPONSE_BASE];

    qmi_ril_free_pending_unsol_resp(resp, resp_id);

    // copy the unsol resp
    resp->valid = TRUE;
    resp->instance_id = param_ptr->instance_id;
    resp->copied_param_len = param_ptr->resp_len;

    if ( NULL != param_ptr->logstr )
    {
      size_t len = strlen(param_ptr->logstr);
      resp->logstr = qcril_malloc(len+1);
      if ( NULL != resp->logstr )
      {
        memcpy((void*) resp->logstr, (void*) param_ptr->logstr, len);
      }
      else
      {
        QCRIL_LOG_ERROR("malloc failed");
        resp->valid = FALSE;
        break;
      }
    }
    else
    {
      resp->logstr = NULL;
    }

    resp->param_copy_arrron = qmi_ril_fw_create_android_live_params_copy( resp_id,
                                                                          param_ptr->resp_pkt,
                                                                          param_ptr->resp_len,
                                                                          resp->copied_params,
                                                                          NULL,
                                                                          &resp->copied_params,
                                                                          NULL );

    if ( QMI_RIL_ANDROID_PARAM_CPY_APPRON_INVALID == resp->param_copy_arrron )
    {
      resp->valid = FALSE;
      if ( NULL != resp->logstr )
      {
        qcril_free((void*) resp->logstr);
      }
    }

  } while ( FALSE );
  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_add_unsol_resp_to_pending_list

//===========================================================================
// qmi_ril_get_unsol_resp_from_pending_list
//===========================================================================
qmi_ril_android_pending_unsol_resp_type* qmi_ril_get_unsol_resp_from_pending_list(int resp_id)
{
  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_android_pending_unsol_resp_type *resp = NULL;

  do
  {
    if ( resp_id < RIL_UNSOL_RESPONSE_BASE || resp_id > QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID)
    {
      QCRIL_LOG_ERROR("invalid android_unsol_resp id %d", resp_id);
      break;
    }

    resp = &qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[resp_id-RIL_UNSOL_RESPONSE_BASE];
    if ( !resp->valid )
    {
      resp = NULL;
    }

  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();
  return resp;
} // qmi_ril_get_unsol_resp_from_pending_list

//===========================================================================
// qmi_ril_check_android_unsol_resp_dispatchable
//===========================================================================
boolean qmi_ril_check_android_unsol_resp_dispatchable(int resp_id)
{
  boolean ret = FALSE;
  if ( resp_id >= RIL_UNSOL_RESPONSE_BASE && resp_id <= QMI_RIL_ANDROID_UNSOL_RESP_MAX_ID )
  {
    ret = qmi_ril_android_pending_unsol_resp.pending_unsol_resp_list[resp_id-RIL_UNSOL_RESPONSE_BASE].num_of_locker ? FALSE : TRUE;
  }
  else
  {
    QCRIL_LOG_ERROR("invalid android_unsol_resp id %d", resp_id);
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET((int) ret);
  return ret;
} // qmi_ril_check_android_unsol_resp_dispatchable

//===========================================================================
// qmi_ril_reset_multi_sim_ftr_info
//===========================================================================
void qmi_ril_reset_multi_sim_ftr_info()
{
  qmi_ril_multi_sim_ftr_info = QMI_RIL_FTR_MULTI_SIM_UNKNOWN;
} // qmi_ril_reset_multi_sim_ftr_info

//============================================================================
// FUNCTION: qmi_ril_retrieve_number_of_rilds
//
// DESCRIPTION:
// Returns the number of rilds supported on a target that supports mutiple rild scenario
//
// RETURN: number of rilds supported on a target that supports mutiple rild scenario
//============================================================================
int qmi_ril_retrieve_number_of_rilds()
{
    int num_of_rilds = 1;
    if ( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_DSDS) )
    {
        num_of_rilds = 2;
    }
    else if( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_TSTS) )
    {
        num_of_rilds = 3;
    }

    return num_of_rilds;
} //qmi_ril_retrieve_number_of_rilds
//===========================================================================
//qmi_ril_bootup_actition_on_rild_atel_link_connect
//===========================================================================
void qmi_ril_bootup_actition_on_rild_atel_link_connect(void * params)
{
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED( params );
  qcril_sms_post_ready_status_update();

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_bootup_actition_on_rild_atel_link_connect

/*===========================================================================

  FUNCTION  qcril_request_suppress_list_init

===========================================================================*/
/*!
    @brief
    Initialize suppress list and mutex.

    @return
    E_SUCCESS if success
*/
/*=========================================================================*/
void qcril_request_suppress_list_init
(
    void
)
{
    pthread_mutexattr_init(&qcril_request_supress_list_mutex_attr);
    pthread_mutex_init(&qcril_request_supress_list_mutex_attr,
                       &qcril_request_supress_list_mutex_attr);
}

/*===========================================================================

  FUNCTION  qcril_request_check_if_suppressed

===========================================================================*/
/*!
    @brief
    Check if a request id is in suppressed list.

    @return
    TRUE is request is suppressed.
*/
/*=========================================================================*/
boolean qcril_request_check_if_suppressed
(
    int event_id
)
{
    boolean ret = FALSE;
    int     i;

    QCRIL_MUTEX_LOCK(&qcril_request_supress_list_mutex, "supress list mutex");

    do
    {
        for (i = 0; ((i < QCRIL_REQUEST_SUPPRESS_MAX_LEN) &&
                    (qcril_request_suppress_list[i] != 0)); i++)
        {
            if (event_id == qcril_request_suppress_list[i])
            {
                ret = TRUE;
                break;
            }
        }
    } while (FALSE);

    QCRIL_MUTEX_UNLOCK(&qcril_request_supress_list_mutex, "supress list mutex");
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}

/*===========================================================================

  FUNCTION  qcril_request_suppress_request

===========================================================================*/
/*!
    @brief
    Add request id to suppressed list.

    @return
    E_SUCCESS if success.
    E_FAILURE if failure.
*/
/*=========================================================================*/
int qcril_request_suppress_request
(
    int event_id
)
{
    boolean ret = E_FAILURE;
    int i;

    QCRIL_MUTEX_LOCK(&qcril_request_supress_list_mutex, "supress list mutex");

    do
    {
        for (i = 0; i < QCRIL_REQUEST_SUPPRESS_MAX_LEN; i++)
        {
            if (0 == qcril_request_suppress_list[i])
            {
                QCRIL_LOG_DEBUG("Supress %d", event_id);
                qcril_request_suppress_list[i] = event_id;
                ret = E_SUCCESS;
                break;
            }
        }

    } while (FALSE);

    QCRIL_MUTEX_UNLOCK(&qcril_request_supress_list_mutex, "supress list mutex");
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}

/*===========================================================================

  FUNCTION qcril_request_clean_up_suppress_list

===========================================================================*/
/*!
    @brief
    Clean up suppress list.

    @return
    None
*/
/*=========================================================================*/
void qcril_request_clean_up_suppress_list
(
    void
)
{
    int i;
    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_MUTEX_LOCK(&qcril_request_supress_list_mutex, "supress list mutex");
    for (i = 0; i < QCRIL_REQUEST_SUPPRESS_MAX_LEN; i++)
    {
        qcril_request_suppress_list[i] = 0;
    }

    QCRIL_MUTEX_UNLOCK(&qcril_request_supress_list_mutex, "supress list mutex");
    QCRIL_LOG_FUNC_RETURN();
}

#ifdef QMI_RIL_UTF
//============================================================================
// FUNCTION: qmi_ril_thread_shutdown
//
// DESCRIPTION:
// clears all global variables and releases all shared resources for reboot
//
// RETURN:
//============================================================================
int qmi_ril_threads_shutdown()
{
  if (core_shutdown_for_reboot() != 0)
  {
    QCRIL_LOG_ERROR("Could not successfully shutdown thread in core_handler.c");
  }

  return 0;

}

//============================================================================
// FUNCTION: qmi_ril_reboot_cleanup
//
// DESCRIPTION:
// clears all global variables and releases all shared resources for reboot
//
// RETURN:
//============================================================================
int qmi_ril_reboot_cleanup()
{
  // Begin shutdown process
  qmi_ril_fw_android_request_flow_control_drop_legacy_book_records( FALSE, FALSE );
  qmi_ril_fw_android_request_flow_control_abandon_all_requests_main_thrd( RIL_E_CANCELLED, FALSE );
  qmi_ril_clear_timed_callback_list();
  // clean up core clients
  qcril_qmi_client_release();

  qmi_ril_reset_multi_sim_ftr_info();

  if (qcril_db_reset_cleanup() != 0)
  {
    QCRIL_LOG_ERROR("Could not successfully reset resources in qcril_db.c");
  }
  if (qcril_qmi_voice_reboot_cleanup() != 0)
  {
    QCRIL_LOG_ERROR("Could not successfully reset resources in qcril_qmi_voice.c");
  }
  // local file cleanup
  qcril_free(qmi_ril_fw_android_request_flow_control_overview.android_request_kind_info);

  memset(&qcril_hash_table, 0, sizeof(qcril_hash_table));
  unsigned int i;
  for (i = 0; i < QCRIL_ARR_SIZE( qcril_event_table ); ++i)
  {
    qcril_event_table[i].next_ptr = NULL;
  }

  return 0;
}

#endif
