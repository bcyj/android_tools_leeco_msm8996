/******************************************************************************
  @file    qcril_qmi_nas.h
  @brief   qcril qmi - NAS

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI NAS.

  ---------------------------------------------------------------------------

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_QMI_NAS_H
#define QCRIL_QMI_NAS_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "qmi_client.h"
#include "voice_service_v02.h"
#include "device_management_service_v01.h"
#include "network_access_service_v01.h"
#include "qcril_arb.h"
#include "qcril_data_client.h"

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define QMI_CODING_SCHEME_CELL_BROADCAST_DATA 0x00
#define QMI_CODING_SCHEME_UCS2                0x01

#define NAS_NMR_MAX_NUM_V01  6
#define QCRIL_MCC_MNC_MAX_SIZE     (4)

typedef enum
{
    QMI_RIL_RTE_NONE = 0,
    QMI_RIL_RTE_FIRST,
    QMI_RIL_RTE_1x = QMI_RIL_RTE_FIRST,
    QMI_RIL_RTE_GSM,
    QMI_RIL_RTE_WCDMA,
    QMI_RIL_RTE_TDSCDMA,
    QMI_RIL_RTE_SUB_LTE,
    QMI_RIL_RTE_SUB_DO,
    QMI_RIL_RTE_LAST = QMI_RIL_RTE_SUB_DO,
    QMI_RIL_RTE_CAP
} qmi_ril_nw_reg_rte_type;

typedef enum
{
    QMI_RIL_RTE_KIND_NONE = 0,
    QMI_RIL_RTE_KIND_VOICE,
    QMI_RIL_RTE_KIND_DATA,
    QMI_RIL_RTE_KIND_IMS,
    QMI_RIL_RTE_KIND_CAP
} qmi_ril_nw_reg_rte_kind_type;

typedef enum
{
    QMI_RIL_RAT_CONFIDENCE_UNKNOWN = 0,
    QMI_RIL_RAT_CONFIDENCE_FULL_SVC,
    QMI_RIL_RAT_CONFIDENCE_LTD_SVC,
    QMI_RIL_RAT_CONFIDENCE_ANTICIPATED,
    QMI_RIL_RAT_CONFIDENCE_NO_SVC
} qmi_ril_nw_reg_rat_confidence_tag_type;

typedef struct
{
  qmi_ril_nw_reg_rte_type                   voice_rte;
  qmi_ril_nw_reg_rte_type                   data_rte;
  qmi_ril_nw_reg_rte_type                   ims_rte;
  qmi_ril_nw_reg_rat_confidence_tag_type    voice_rte_confidence_tag;
  qmi_ril_nw_reg_rat_confidence_tag_type    data_rte_confidence_tag;
  qmi_ril_nw_reg_rat_confidence_tag_type    ims_rte_confidence_tag;
} qmi_ril_nw_reg_rte_snapshot_type;

typedef enum
{
    QMI_RIL_EME_CBM_INVALID,
    QMI_RIL_EME_CBM_NOT_ACTIVE,
    QMI_RIL_EME_CBM_ACTIVE
} qmi_ril_emergency_callback_mode_state_type;

typedef enum
{

    QMI_RIL_NW_REG_FULL_SERVICE             = 1,
    QMI_RIL_NW_REG_LIMITED_SERVICE          = 2,
    QMI_RIL_NW_REG_VOICE_CALLS_AVAILABLE    = 4,
    QMI_RIL_NW_REG_VOIP_CALLS_AVAILABLE     = 8
} qmi_ril_nw_reg_status_overview_item_type;

typedef enum
{
    QMI_RIL_DMS_RADIO_PWR_CANCEL_NONE = 0,
    QMI_RIL_DMS_RADIO_PWR_CANCEL_SSR,
} qmi_ril_dms_radio_pwr_cancel_e_type;

typedef enum
{
    SUBSCRIPTION_SOURCE_RUIM_SIM = 0,
    SUBSCRIPTION_SOURCE_NV,
    SUBSCRIPTION_SOURCE_RUIM_IF_AVAIL
}oem_hook_cdma_subscription_source_e_type;

typedef enum
{
    FTM_SUBSCRIPTION_INVALID = -1,
    FTM_SUBSCRIPTION_CDMA,
    FTM_SUBSCRIPTION_GSM,
    FTM_SUBSCRIPTION_WCDMA
}oem_hook_ftm_subscription_source_e_type;

typedef enum
{
    QMI_RIL_NAS_INIT_APN_ATTCH_NONE   = 0,
    QMI_RIL_NAS_INIT_APN_ATTCH_DETACH,
    QMI_RIL_NAS_INIT_APN_ATTCH_ATTACH,
    QMI_RIL_NAS_INIT_ALLOW_DATA_ATTCH_ATTACH,
    QMI_RIL_NAS_INIT_ALLOW_DATA_ATTCH_ON_DEMAND_ATTACH,
    QMI_RIL_NAS_INIT_ALLOW_DATA_ATTCH_WAIT_FOR_RAT_EXP,
    QMI_RIL_NAS_INIT_ALLOW_DATA_ATTCH_DETACH
} qmi_ril_nas_init_apn_attch_state_e_type;

typedef enum
{
    QCRIL_QMI_ACQ_ORDER_NONE = 0,
    QCRIL_QMI_ACQ_ORDER_LTE_TDS_GSM,
    QCRIL_QMI_ACQ_ORDER_TDS_GSM_LTE,
    QCRIL_QMI_ACQ_ORDER_LTE_UMTS_GSM
} qcril_qmi_acq_order_e_type;

typedef enum
{
    QCRIL_QMI_BAND_PREF_NONE = 0,
    QCRIL_QMI_BAND_PREF_LTE_FULL,
    QCRIL_QMI_BAND_PREF_TDD_LTE,
    QCRIL_QMI_BAND_PREF_FDD_LTE
} qcril_qmi_band_pref_e_type;

typedef enum
{
    QCRIL_QMI_RAT_BAND_NONE = 1,
    QCRIL_QMI_LTE_BAND,
} qcril_qmi_rat_band_e_type;

struct ftm_subs_status
{
  oem_hook_ftm_subscription_source_e_type subs_type;
  int is_enable;
};

struct cdma_ftm_data
{
    uint32_t srv_status;
    uint32_t srv_domain;
    uint32_t system_mode;
    uint32_t roam_status;
    uint32_t mcc;
    uint32_t mnc;
    int8_t rssi;
    int16_t ecio;
    float cdma_1x_rx0_agc;
    float cdma_1x_rx1_agc;
    float cdma_evdo_rx0_agc;
    float cdma_evdo_rx1_agc;
};

struct gsm_neigh_cell_info
{
    uint32_t cell_id;
    uint16_t bcch;
    uint8_t bsic;
    uint16_t rx_level;
};

struct gsm_ftm_data
{
    uint32_t srv_status;
    uint32_t srv_domain;
    uint32_t system_mode;
    uint32_t roam_status;
    uint32_t mcc;
    uint32_t mnc;
    uint16_t lac;
    int8_t rssi;
    uint16_t bcch;
    uint8_t bsic;
    uint16_t rx_level;
    uint16_t rx_qual_full;
    uint16_t rx_qual_sub;
    uint16_t ta;
    uint32_t no_of_neigh_cell_info_len;
    struct gsm_neigh_cell_info gsm_neigh_cell[ NAS_NMR_MAX_NUM_V01 ];
};

struct wcdma_neigh_cell_info
{
    uint32_t cell_id;
    uint16_t uarfcn;
    uint16_t psc;
    int16_t rscp;
    int16_t ecio;
};

struct wcdma_ftm_data
{
    uint32_t srv_status;
    uint32_t srv_domain;
    uint32_t system_mode;
    uint32_t roam_status;
    uint32_t mcc;
    uint32_t mnc;
    uint16_t lac;
    uint16_t bler;
    int16_t ecio;
    int16_t rscp;
    float rx_agc;
    float tx_agc;
    uint16_t uarfcn;
    uint16_t psc;
    uint32_t no_of_neigh_cell_info_len;
    struct wcdma_neigh_cell_info wcdma_neigh_cell[ NAS_NMR_MAX_NUM_V01 ];
};

void qcril_qmi_dms_unsolicited_indication_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);


typedef enum
{
    QCRIL_CELL_LOCATION_INCREASE_PERIOD,
    QCRIL_CELL_LOCATION_REDUCE_PERIOD,
    QCRIL_CELL_LOCATION_RESET_PERIOD,
} qmi_ril_cell_location_period_action;

void qcril_qmi_dms_unsolicited_indication_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
);

void qcril_qmi_nas_unsolicited_indication_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_async_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_unsolicited_indication_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
);

void qcril_qmi_nas_init();
void qcril_qmi_dms_init();
void qcril_qmi_nas_multi_sim_init();

RIL_Errno qcril_qmi_nas_get_device_capability(int need_msim_capability,
                                              int need_sub_capability);

void qcril_qmi_nas_get_subscription_info();

void qcril_qmi_nas_update_modem_stack_id
(
uint8_t stack_id
);

void qcril_qmi_util_decode_operator_name
(
  char *dest,
  uint16 max_dest_length,
  int coding_scheme,
  const uint8 *src,
  uint16 src_length
);

void qcril_qmi_util_decode_operator_name_in_little_endian
(
  char *dest,
  uint16 max_dest_length,
  int coding_scheme,
  const uint8 *src,
  uint16 src_length
);

uint32_t qmi_ril_nw_reg_get_status_overview( void );

int qcril_qmi_nas_is_in_service_in( uint8_t radio_if1, uint8_t radio_if2 );

void qcril_qmi_nas_request_signal_strength
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_request_data_registration_state
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_request_registration_state
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_request_operator
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_query_network_selection_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_request_power
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_request_screen_state
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

RIL_RadioState qcril_qmi_nas_dms_get_current_power_state ( qcril_instance_id_e_type instance_id );

boolean qcril_qmi_nas_dms_is_in_online_mode();
boolean qcril_qmi_nas_dms_is_in_ftm_mode();
boolean qcril_qmi_nas_is_radio_power_check_enabled();

void qcril_qmi_nas_exit_emergency_callback_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_set_network_selection_automatic
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_set_network_selection_manual
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_query_available_networks
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_set_band_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_query_available_band_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_exit_emergency_callback_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_request_radio_tech
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_request_get_dc_rt_info
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_get_neighboring_cell_ids
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_set_location_updates
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_request_cdma_subscription
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_set_subscription_source
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_set_subscription_source_with_spc
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_get_subscription_source
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_cdma_validate_and_write_key
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_dms_request_device_identity
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_dms_request_baseband_version
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_dms_commmon_pre_init();
void qcril_qmi_nas_dms_commost_post_cleanup();

int qcril_qmi_ril_domestic_service_is_screen_off();

void qcril_qmi_nas_dms_event_card_status_updated
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

void qcril_qmi_nas_dms_event_update_fdn_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

void qcril_qmi_dms_fusion_csfb_unsolicited_indication_cb
(
  qmi_client_type                user_handle,
  unsigned long                  message_id,
  unsigned char                  * ind_buf,
  int                            ind_buf_len,
  void                           *ind_cb_data
);

void qcril_qmi_nas_cleanup();


void qcril_qmi_nas_minority_command_cb
(
  qmi_client_type              user_handle,
  unsigned int                 msg_id,
  void                        *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
);

void qcril_qmi_nas_dsds_request_set_subscription_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_dsds_request_get_uicc_subscription
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_event_deactivate_provision_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_event_activate_provision_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_set_uicc_subscription
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_dsds_request_set_tune_away
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_dsds_request_get_tune_away
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_dsds_request_set_paging_priority
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_dsds_request_get_paging_priority
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_dsds_request_set_data_subscription
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_dsds_request_set_default_voice_sub
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_event_subs_followup
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_event_subs_deactivate_followup
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_event_card_status_update
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_event_app_status_update
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_get_modem_capability
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_update_sub_binding
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_request_allow_data
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_trigger_propagate_known_signal_strength_ind
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_dms_request_imei
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_dms_request_imeisv
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_bootup_kick
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_nw_select_handle_total_cleanup
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_send_unsol_radio_state_changed
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_connected_emergency_call_end_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_emergency_mode_on_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_emergency_mode_off_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

RIL_Errno qcril_qmi_util_enable_networking_indications( int enable, int force_update );

void qmi_ril_initiate_bootup_kick(void);

int qmi_ril_is_qmi_sys_info_available(void);
int qmi_ril_is_qmi_tdscdma_support_available(void);

qmi_ril_emergency_callback_mode_state_type qmi_ril_nwr_get_eme_cbm();
void qmi_ril_nwr_set_eme_cbm(qmi_ril_emergency_callback_mode_state_type new_mode);

void qcril_qmi_nas_initiate_radio_state_changed_ind(void);

void qcril_qmi_nas_dms_handle_fetch_prl_request
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_dms_handle_check_prl_ver_change
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas_set_registered_on_ims( int registered );

void qcril_qmi_nas_embms_requst_enable
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_enable_data_con
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_requst_disable
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_disable_data_ind
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_request_get_coverage_state
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_get_rssi
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_get_sib16_coverage
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_set_sntp_time
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_get_utc_time
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_send_radio_state_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_get_active_log_packet_ids
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_deliver_log_packet
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_get_e911_state
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_get_sib_plmn
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_get_embms_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_post_voice_rte_change_ind_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_perform_network_scan_command_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_set_nw_selection_command_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_set_rfm_scenario_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_get_rfm_scenario_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_get_provisioned_table_revision_req
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_embms_set_enable_cmd_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_dms_update_card_status
(
  qcril_instance_id_e_type instance_id,
  uint8 slot,
  boolean pwr_oprt_in_progress,
  qcril_card_status_e_type card_status
);


void qcril_qmi_nas_set_reported_voice_radio_tech(RIL_RadioTechnology voice_radio_tech);
RIL_RadioTechnology qcril_qmi_nas_get_reported_voice_radio_tech();

int qcril_qmi_nas_is_using_rte_helper(qmi_ril_nw_reg_rte_type rte,
                                      qmi_ril_nw_reg_rte_kind_type rte_kind);

int qcril_qmi_dms_is_prl_info_available(uint16_t * prl_version);

void qcril_qmi_nas_initiate_voice_rte_change_propagation( void );

void qcril_qmi_nas_reevaluate_vrte_helper (qcril_timed_callback_handler_params_type *param);

void qcril_qmi_nas_control_signal_nas_on_current_calls_change();

void qcril_qmi_nas_dms_commmon_post_init();

void qmi_ril_nw_reg_snapshot_init( qmi_ril_nw_reg_rte_snapshot_type* snapshot );
void qmi_ril_nw_reg_snapshot_cur_rte( qmi_ril_nw_reg_rte_snapshot_type* snapshot );
int qmi_ril_nw_reg_snapshot_is_same_as( qmi_ril_nw_reg_rte_snapshot_type* snapshot1, qmi_ril_nw_reg_rte_snapshot_type* snapshot2 );

int qmi_ril_nw_reg_voice_is_voice_call_mode_reasonable_against_dev_cfg( call_mode_enum_v02 call_mode );
int qmi_ril_nwreg_is_vrte_post_change_window( void );
void qcril_qmi_nas_evaluate_data_rte_on_pref_data_tech_change();

void qmi_ril_nw_reg_data_pref_changed_action();
int  qcril_qmi_nas_check_power_save_and_screen_off_status();

const char* qcril_qmi_util_retrieve_technology_name(int technology);
const char* qcril_qmi_util_retrieve_pref_data_tech_name(int pref_data_tech);
void qcril_qmi_nas_retrieve_rf_band_info();

int qcril_qmi_data_nas_control_get_current_calls_number(qcril_data_active_call_info_t *call_list);

inline boolean qcril_qmi_nas_get_mode_pref_from_nv_10();
int qmi_ril_nwreg_mode_pref_enforcement_deferred_ncl( int android_mode_pref );
int qmi_ril_nwreg_mode_pref_is_pending_deferred_enforcement_ncl ( int * android_mode_pref );

void qmi_ril_nwreg_mode_pref_enforce_deferred_op_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

qmi_ril_nw_reg_rte_type qmi_ril_nw_reg_get_current_rte_of_kind( qmi_ril_nw_reg_rte_kind_type rte_kind );

void qmi_ril_nw_reg_initiate_post_cfg_ban_for_data_reg_extrapolation_ncl();
RIL_Errno qcril_qmi_fetch_system_selection_preference(void);
inline uint8_t qcril_qmi_nas_get_mode_pref(uint16_t *mode_pref);

void qcril_qmi_nas_request_shutdown
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type       *const ret_ptr
);

void qcril_qmi_nas_csg_handle_oem_hook_perform_network_scan
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_csg_handle_oem_hook_set_sys_selection
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_nw_select_dedicated_unsolicited_indicaton_event_thrd_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qmi_ril_nw_reg_data_sys_update_pre_update_action( void );

void qmi_ril_nwreg_common_ind_subscribe_consider_action_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);
void qcril_qmi_nas_cancel_radio_power_process(qmi_ril_dms_radio_pwr_cancel_e_type cancel_reason);

void qmi_ril_nwreg_post_oprt_online_action_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);


int qcril_qmi_nas_retrieve_reported_data_technology();
dms_simul_voice_and_data_capability_mask_v01 qcril_qmi_nas_retrieve_sv_capability();

void qcril_qmi_nas_assess_emergency_number_list_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

voice_dial_call_service_type_enum_v02 qcril_qmi_nas_setting_srv_type_based_on_elaboration_and_rat
(
  uint64_t ela
);

typedef enum
{
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_NONE = 0,
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MCC,
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_VOICE,
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_HARD_MCC,
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_HARD,
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_NW,
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_ESCV_IIN,
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_ESCV_NW,
    QMI_RIL_CUSTOM_EMERGENCY_NUMBERS_SOURCE_MAX
} qmi_ril_custom_emergency_numbers_source_type;

boolean is_operator_name_empty_or_white_space ( char * str, int max_len);
int qmi_ril_nwreg_is_designated_number_enforcable_ncl(char * emergency_number);
int qmi_ril_nwreg_convert_emergency_number_to_ims_address(char* emergency_number);
int qmi_ril_nwreg_is_designated_number_emergency_for_display_purposes_only(char * emergency_number);
void qmi_ril_nwreg_designated_number_ensure_fresh_check_ncl(void);

RIL_Errno qcril_qmi_nas_voice_move_device_to_online_for_emer_call_conditionally();
RIL_Errno qcril_qmi_nas_voice_move_device_to_lpm_after_emer_call_conditionally();

void qcril_qmi_nas_start_wait_for_pbm_ind_timer();
void qcril_qmi_nas_cancel_wait_for_pbm_ind_timer();

void qcril_qmi_nas_embms_send_radio_state(radio_state_enum_v01 radio_state);

void qcril_qmi_nas_cdma_avoid_system
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_cdma_clear_avoid_list
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_get_cdma_avoid_system_list
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_enable_engineer_mode
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_set_builtin_plmn_list
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_perform_incremental_network_scan
(
const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);
void qcril_qmi_nas_get_cell_info_list_ncl

(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);
void qcril_qmi_nas_set_cell_info_list_rate
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);
void qcril_qmi_nas_poll_cell_info_list
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);
void qcril_qmi_nas_cell_info_list_changed
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

int qcril_qmi_nas_get_escv_type
(
    char *emergency_number
);

uint8_t qcril_qmi_nas_get_lte_disable_cause
(
    nas_lte_disable_cause_enum_type_v01 *lte_disable_cause
);

void qcril_qmi_voice_ims_send_unsol_radio_state_change_helper();

void qcril_qmi_nas_cancel_srv_domain_camped_timer_helper();
void qcril_qmi_nas_modem_power_set_boot_in_apm();
void qcril_qmi_nas_modem_power_ril_resumed();
boolean qcril_qmi_nas_modem_power_is_mdm_shdn_in_apm();
void qcril_qmi_nas_modem_power_load_apm_mdm_not_pwdn();

void qcril_qmi_drop_sig_info_cache(void);
void qcril_qmi_nas_sys_sel_pref_validity_tmr_expry_handler(void * param);
int qcril_qmi_nas_sys_sel_pref_setup_timed_callback ();
void qcril_qmi_nas_initialize_is_indication_received();
void qcril_qmi_nas_cancel_sys_sel_pref_tmr();
RIL_Errno qcril_qmi_nas_mode_pref_request_response_helper(const qcril_request_params_type *const params_ptr, uint8 *is_change);
int qcril_qmi_is_srlte_supported();
void qcril_qmi_nas_fetch_lte_sms_status ( uint8_t *lte_sms_status_valid, nas_sms_status_enum_type_v01 *lte_sms_status );

void qcril_qmi_nas_set_is_data_enabled
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type       *const ret_ptr
);

void qcril_qmi_nas_set_is_data_roaming_enabled
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type       *const ret_ptr
);

void qcril_qmi_nas_set_apn_info
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type       *const ret_ptr
);

void qcril_qmi_nas_dsds_request_set_lte_tune_away
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_dsds_lte_tune_away_cb ( RIL_Errno resp_res );

int qcril_qmi_nas_get_sim_mcc_mnc(char mcc[QCRIL_MCC_MNC_MAX_SIZE],
                                  char mnc[QCRIL_MCC_MNC_MAX_SIZE]);

void qcril_qmi_nas_update_sim_mcc_mnc(boolean valid,
                                      char    mcc[QCRIL_MCC_MNC_MAX_SIZE],
                                      char    mnc[QCRIL_MCC_MNC_MAX_SIZE]);

void qcril_uim_process_mcc_mnc_info ( const qcril_request_params_type *const params_ptr,
                                      qcril_request_return_type       *const ret_ptr);

boolean qcril_qmi_nas_did_rat_expansion_occur
(
    void
);

qcril_modem_stack_id_e_type qcril_qmi_nas_get_modem_stack_id();

void qcril_qmi_nas_embms_send_embms_status (uint8_t is_avaliable);

void qcril_qmi_nas_populate_sig_config_delta_info(void);

RIL_Errno qcril_qmi_nas_configure_sig_info2(void);

uint8_t qcril_qmi_nas_get_embms_status(void);

void qcril_qmi_nas_update_embms_status(void);

#endif /* QCRIL_QMI_NAS_H */

