/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_NAS_CORE
#define CRI_NAS_CORE

#include "utils_common.h"
#include "cri_core.h"
#include "network_access_service_v01.h"
#include "cri_nas.h"

#define CRI_NAS_RAT_CDMA_BIT        QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_BIT_V01
#define CRI_NAS_RAT_HRPD_BIT        QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_BIT_V01
#define CRI_NAS_RAT_GSM_BIT         QMI_NAS_RAT_MODE_PREF_GSM_BIT_V01
#define CRI_NAS_RAT_WCDMA_BIT       QMI_NAS_RAT_MODE_PREF_UMTS_BIT_V01
#define CRI_NAS_RAT_LTE_BIT         QMI_NAS_RAT_MODE_PREF_LTE_BIT_V01
#define CRI_NAS_RAT_TDSCDMA_BIT     QMI_NAS_RAT_MODE_PREF_TDSCDMA_BIT_V01
#define CRI_NAS_RAT_PRL_BIT         6

#define CRI_NAS_NW_SCAN_RESP_ENTRY_SIZE 40

typedef struct {
  uint8_t cdma_srv_status_info_valid;
  nas_3gpp2_srv_status_info_type_v01 cdma_srv_status_info;
  uint8_t hdr_srv_status_info_valid;
  nas_3gpp2_srv_status_info_type_v01 hdr_srv_status_info;
  uint8_t gsm_srv_status_info_valid;
  nas_3gpp_srv_status_info_type_v01 gsm_srv_status_info;
  uint8_t wcdma_srv_status_info_valid;
  nas_3gpp_srv_status_info_type_v01 wcdma_srv_status_info;
  uint8_t lte_srv_status_info_valid;
  nas_3gpp_srv_status_info_type_v01 lte_srv_status_info;
  uint8_t cdma_sys_info_valid;
  nas_cdma_sys_info_type_v01 cdma_sys_info;
  uint8_t hdr_sys_info_valid;
  nas_hdr_sys_info_type_v01 hdr_sys_info;
  uint8_t gsm_sys_info_valid;
  nas_gsm_sys_info_type_v01 gsm_sys_info;
  uint8_t wcdma_sys_info_valid;
  nas_wcdma_sys_info_type_v01 wcdma_sys_info;
  uint8_t lte_sys_info_valid;
  nas_lte_sys_info_type_v01 lte_sys_info;
  uint8_t cdma_sys_info2_valid;
  nas_cdma_sys_info2_type_v01 cdma_sys_info2;
  uint8_t hdr_sys_info2_valid;
  nas_hdr_sys_info2_type_v01 hdr_sys_info2;
  uint8_t gsm_sys_info2_valid;
  nas_gsm_sys_info2_type_v01 gsm_sys_info2;
  uint8_t wcdma_sys_info2_valid;
  nas_wcdma_sys_info2_type_v01 wcdma_sys_info2;
  uint8_t lte_sys_info2_valid;
  nas_lte_sys_info2_type_v01 lte_sys_info2;
  uint8_t gsm_sys_info3_valid;
  nas_gw_sys_info3_type_v01 gsm_sys_info3;
  uint8_t wcdma_sys_info3_valid;
  nas_gw_sys_info3_type_v01 wcdma_sys_info3;
  uint8_t voice_support_on_lte_valid;
  uint8_t voice_support_on_lte;
  uint8_t gsm_cipher_domain_valid;
  nas_service_domain_enum_type_v01 gsm_cipher_domain;
  uint8_t wcdma_cipher_domain_valid;
  nas_service_domain_enum_type_v01 wcdma_cipher_domain;
  uint8_t sys_info_no_change_valid;
  uint8_t sys_info_no_change;
  uint8_t tdscdma_srv_status_info_valid;
  nas_3gpp_srv_status_info_type_v01 tdscdma_srv_status_info;
  uint8_t tdscdma_sys_info_valid;
  nas_tdscdma_sys_info_type_v01 tdscdma_sys_info;
  uint8_t lte_embms_coverage_valid;
  uint8_t lte_embms_coverage;
  uint8_t sim_rej_info_valid;
  nas_sim_rej_info_enum_type_v01 sim_rej_info;
  uint8_t wcdma_eutra_status_valid;
  nas_eutra_cell_status_enum_type_v01 wcdma_eutra_status;
  uint8_t lte_ims_voice_avail_valid;
  uint8_t lte_ims_voice_avail;
  uint8_t lte_voice_status_valid;
  nas_lte_voice_status_enum_type_v01 lte_voice_status;
  uint8_t cdma_reg_zone_valid;
  uint16_t cdma_reg_zone;
  uint8_t gsm_rac_valid;
  uint8_t gsm_rac;
  uint8_t wcdma_rac_valid;
  uint8_t wcdma_rac;
  uint8_t cdma_mcc_resolved_via_sid_lookup_valid;
  uint16_t cdma_mcc_resolved_via_sid_lookup;
  uint8_t srv_reg_restriction_valid;
  nas_srv_reg_restriction_enum_v01 srv_reg_restriction;
  uint8_t tdscdma_reg_domain_valid;
  nas_possible_reg_domain_enum_type_v01 tdscdma_reg_domain;
  uint8_t lte_reg_domain_valid;
  nas_possible_reg_domain_enum_type_v01 lte_reg_domain;
  uint8_t wcdma_reg_domain_valid;
  nas_possible_reg_domain_enum_type_v01 wcdma_reg_domain;
  uint8_t gsm_reg_domain_valid;
  nas_possible_reg_domain_enum_type_v01 gsm_reg_domain;
  uint8_t lte_embms_coverage_trace_id_valid;
  int16_t lte_embms_coverage_trace_id;
  uint8_t wcdma_csg_info_valid;
  nas_csg_info_type_v01 wcdma_csg_info;
  uint8_t hdr_voice_status_valid;
  nas_lte_voice_status_enum_type_v01 hdr_voice_status;
  uint8_t hdr_sms_status_valid;
  nas_sms_status_enum_type_v01 hdr_sms_status;
  uint8_t lte_sms_status_valid;
  nas_sms_status_enum_type_v01 lte_sms_status;
  uint8_t lte_is_eb_supported_valid;
  nas_tri_state_boolean_type_v01 lte_is_eb_supported;
  uint8_t gsm_voice_status_valid;
  nas_lte_voice_status_enum_type_v01 gsm_voice_status;
  uint8_t gsm_sms_status_valid;
  nas_sms_status_enum_type_v01 gsm_sms_status;
  uint8_t wcdma_voice_status_valid;
  nas_lte_voice_status_enum_type_v01 wcdma_voice_status;
  uint8_t wcdma_sms_status_valid;
  nas_sms_status_enum_type_v01 wcdma_sms_status;
  uint8_t emergency_access_barred_valid;
  nas_tri_state_boolean_type_v01 emergency_access_barred;
  uint8_t cdma_voice_status_valid;
  nas_lte_voice_status_enum_type_v01 cdma_voice_status;
  uint8_t cdma_sms_status_valid;
  nas_sms_status_enum_type_v01 cdma_sms_status;
  uint8_t tdscdma_voice_status_valid;
  nas_lte_voice_status_enum_type_v01 tdscdma_voice_status;
  uint8_t tdscdma_sms_status_valid;
  nas_sms_status_enum_type_v01 tdscdma_sms_status;
  uint8_t lte_csg_info_valid;
  nas_csg_info_type_v01 lte_csg_info;
  uint8_t lte_cell_status_valid;
  nas_cell_access_status_e_type_v01 lte_cell_status;
  uint8_t hdr_subnet_mask_len_valid;
  uint8_t hdr_subnet_mask_len;
}cri_nas_core_sys_info_type;

typedef struct {
  uint8_t radio_if_valid;
  nas_radio_if_enum_v01 radio_if;
}cri_nas_core_managed_roaming_info_type;

typedef struct {
  nas_radio_if_enum_v01 radio_if;
  nas_service_domain_enum_type_v01 reject_srv_domain;
  uint8_t rej_cause;
  uint8_t plmn_id_valid;
  nas_mnc_pcs_digit_include_status_type_v01 plmn_id;
  uint8_t csg_id_valid;
  uint32_t csg_id;
}cri_nas_core_nas_reject_info_type;

typedef struct {
    uint8_t mode_pref_valid;
    mode_pref_mask_type_v01 mode_pref;
    uint8_t net_sel_pref_valid;
    nas_net_sel_pref_enum_v01 net_sel_pref;
    uint8_t manual_net_sel_plmn_valid;
    nas_mnc_pcs_digit_include_status_type_v01 manual_net_sel_plmn;
    uint8_t roam_pref_valid;
    nas_roam_pref_enum_v01 roam_pref;
    uint8_t srv_reg_restriction_valid;
    nas_srv_reg_restriction_enum_v01 srv_reg_restriction;
}cri_nas_core_sys_sel_pref_info_type;


typedef struct {
    cri_nas_rte_type cri_nas_voice_rte;
    cri_nas_rte_reg_confidence_type cri_nas_voice_rte_confidence;
    cri_nas_rte_type cri_nas_data_rte;
    cri_nas_rte_reg_confidence_type cri_nas_data_rte_confidence;
}cri_nas_core_rte_info_type;

cri_nas_core_rte_info_type cri_nas_core_rte_info;
cri_nas_core_sys_info_type *cri_nas_core_sys_info;
cri_nas_core_managed_roaming_info_type *cri_nas_core_managed_roaming_info;
cri_nas_core_nas_reject_info_type *cri_nas_core_nas_reject_info;

cri_nas_nw_select_state_type cri_nas_nw_select_state;
cri_nas_nw_selection_info_type cri_nas_nw_selection_info;

cri_nas_core_sys_sel_pref_info_type cri_nas_sys_sel_pref_info;

uint8_t is_screen_state_off;

cri_nas_rte_gsm_reg_info_type *cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_MAX];
cri_nas_rte_wcdma_reg_info_type *cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_MAX];
cri_nas_rte_tdscdma_reg_info_type *cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_MAX];
cri_nas_rte_lte_reg_info_type *cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_MAX];
cri_nas_rte_cdma_reg_info_type *cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_MAX];
cri_nas_rte_hdr_reg_info_type *cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_MAX];

typedef struct
{
    uint8_t service_provider_name_valid;
    nas_service_provider_name_type_v01 * service_provider_name;

    uint8_t nitz_information_valid;
    nas_plmn_network_name_type_v01 * nitz_information;

    uint8_t spn_valid;
    nas_spn_type_v01 * spn;

    uint8_t short_name_valid;
    nas_plmn_name_type_v01 * short_name;

    uint8_t long_name_valid;
    nas_plmn_name_type_v01 * long_name;

    uint8_t mode_pref_valid;
    uint16_t mode_pref;

    uint8_t prl_pref_valid;
    uint16_t prl_pref;

}cri_nas_cached_info_type;


typedef struct
{
    uint32_t nw_scan_info_len;
    char long_eons [ CRI_NAS_3GPP_NETWORK_INFO_LIST_MAX] [ CRI_NAS_OPERATOR_MAX_EONS_LEN ];
    char short_eons [ CRI_NAS_3GPP_NETWORK_INFO_LIST_MAX] [ CRI_NAS_OPERATOR_MAX_EONS_LEN ];
    char mcc[ CRI_NAS_3GPP_NETWORK_INFO_LIST_MAX] [ CRI_NAS_MCC_MNC_MAX+1] ;
    char mnc[ CRI_NAS_3GPP_NETWORK_INFO_LIST_MAX] [ CRI_NAS_MCC_MNC_MAX+1] ;
    uint32_t rat[CRI_NAS_3GPP_NETWORK_INFO_LIST_MAX];
    uint32_t network_status[CRI_NAS_3GPP_NETWORK_INFO_LIST_MAX];
} cri_nas_nw_scan_resp_type;


int cri_nas_core_retrieve_client_id();
qmi_error_type_v01 cri_nas_core_init_client(hlos_ind_cb_type hlos_ind_cb);
void cri_nas_core_release_client(int qmi_service_client_id);
void cri_nas_core_unsol_ind_handler(int qmi_service_client_id,
                                        unsigned long message_id,
                                        void *ind_data,
                                        int ind_data_len);
void cri_nas_core_async_resp_handler(int qmi_service_client_id,
                                     unsigned long message_id,
                                     void *resp_data,
                                     int resp_data_len,
                                     cri_core_context_type cri_core_context);

void cri_nas_core_calculate_reg_status_for_all_rtes();

void cri_nas_send_update_to_clients(int qmi_service_client_id,
                                        unsigned long message_id,
                                        void *ind_data,
                                        int ind_data_len);

uint32_t cri_nas_set_auto_mode(uint32_t pref_mode);

cri_core_error_type cri_nas_get_operator_name_request_handler(char *long_eons,
                                                            char *short_eons,
                                                            char *mcc,
                                                            char *mnc);

cri_core_error_type cri_nas_network_selection_handler(cri_core_context_type cri_core_context,
                                                    const cri_nas_nw_selection_request_type *selection_info,
                                                    void *user_data,
                                                    hlos_resp_cb_type hlos_resp_cb);

int cri_nas_enable_networking_indications(uint8_t is_screen_off,
                                            uint8_t is_nw_sel_in_progress);

int cri_nas_change_screen_state(uint8_t is_screen_off);

cri_core_error_type cri_nas_fetch_system_selection_preference();

cri_core_error_type cri_nas_get_mode_preferrence(uint32_t *mode_pref);

cri_core_error_type cri_nas_get_nw_selection_preferrence(uint32_t *nw_sel_pref);

cri_nas_tech_reg_status_type cri_nas_is_considered_limited_service(void);

void cri_nas_core_update_data_system_status();

cri_core_error_type cri_nas_nw_selection_set_state(cri_nas_nw_select_state_type state);

uint8_t cri_nas_is_nw_selection_in_progress();

uint8_t cri_nas_is_current_plmn(uint32_t mcc, uint32_t mnc);

cri_nas_tech_reg_status_type cri_nas_is_considered_registered();

#endif
