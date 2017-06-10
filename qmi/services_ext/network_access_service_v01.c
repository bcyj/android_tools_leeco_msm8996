/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        N E T W O R K _ A C C E S S _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the nas service Data structures.

  Copyright (c) 2006-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/nas/main/latest/src/network_access_service_v01.c#94 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Wed Jul 31 2013 (Spin 0)
   From IDL File: network_access_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "network_access_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t nas_signal_stregth_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_signal_stregth_indicator_type_v01, report_signal_strength),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_signal_stregth_indicator_type_v01, report_signal_strength_threshold_list),
  NAS_SIG_STRENGTH_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_signal_stregth_indicator_type_v01, report_signal_strength_threshold_list) - QMI_IDL_OFFSET8(nas_signal_stregth_indicator_type_v01, report_signal_strength_threshold_list_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_rssi_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rssi_indicator_type_v01, report_rssi),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rssi_indicator_type_v01, rssi_delta),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_ecio_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ecio_indicator_type_v01, report_ecio),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ecio_indicator_type_v01, ecio_delta),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_io_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_io_indicator_type_v01, report_io),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_io_indicator_type_v01, io_delta),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_sinr_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_sinr_indicator_type_v01, report_sinr),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_sinr_indicator_type_v01, sinr_delta),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_rsrq_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rsrq_indicator_type_v01, report_rsrq),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rsrq_indicator_type_v01, rsrq_delta),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_ecio_indicator_threshold_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ecio_indicator_threshold_type_v01, report_ecio),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_ecio_indicator_threshold_type_v01, threshold_list),
  NAS_ECIO_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_ecio_indicator_threshold_type_v01, threshold_list) - QMI_IDL_OFFSET8(nas_ecio_indicator_threshold_type_v01, threshold_list_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_sinr_indicator_threshold_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_sinr_indicator_threshold_type_v01, report_sinr),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_sinr_indicator_threshold_type_v01, threshold_list),
  NAS_SINR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_sinr_indicator_threshold_type_v01, threshold_list) - QMI_IDL_OFFSET8(nas_sinr_indicator_threshold_type_v01, threshold_list_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_rsrp_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rsrp_indicator_type_v01, report_lte_rsrp),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rsrp_indicator_type_v01, lte_rsrp_delta),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_snr_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_snr_indicator_type_v01, report_lte_snr),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_snr_indicator_type_v01, lte_snr_delta),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_signal_strength_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_signal_strength_type_v01, sig_strength),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_signal_strength_type_v01, radio_if),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_rf_band_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_rf_band_info_type_v01, radio_if),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_rf_band_info_type_v01, active_band),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_rf_band_info_type_v01, active_channel),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_registration_reject_reason_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_registration_reject_reason_type_v01, service_domain),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_registration_reject_reason_type_v01, reject_cause),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_rssi_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rssi_type_v01, rssi),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_rssi_type_v01, radio_if),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_ecio_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ecio_type_v01, ecio),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_ecio_type_v01, radio_if),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_error_rate_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_error_rate_type_v01, error_rate),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_error_rate_type_v01, radio_if),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_rsrq_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rsrq_type_v01, rsrq),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rsrq_type_v01, radio_if),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_reg_network_reject_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_reg_network_reject_v01, reg_network_reject),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_reg_network_reject_v01, suppress_sys_info),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_signal_strength_list_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_signal_strength_list_type_v01, sig_strength),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_signal_strength_list_type_v01, radio_if),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_network_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_network_info_type_v01, mobile_country_code),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_network_info_type_v01, mobile_network_code),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_network_info_type_v01, network_status),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(nas_3gpp_network_info_type_v01, network_description),
  NAS_NETWORK_DESCRIPTION_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_network_radio_access_technology_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_network_radio_access_technology_type_v01, mcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_network_radio_access_technology_type_v01, mnc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_network_radio_access_technology_type_v01, rat),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_mnc_pcs_digit_include_status_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_mnc_pcs_digit_include_status_type_v01, mcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_mnc_pcs_digit_include_status_type_v01, mnc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_mnc_pcs_digit_include_status_type_v01, mnc_includes_pcs_digit),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_csg_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_csg_info_type_v01, id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_csg_info_type_v01, name),
  NAS_CSG_NAME_MAX_V01,
  QMI_IDL_OFFSET8(nas_csg_info_type_v01, name) - QMI_IDL_OFFSET8(nas_csg_info_type_v01, name_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_csg_nw_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_info_type_v01, mcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_info_type_v01, mnc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_info_type_v01, csg_list_cat),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_csg_nw_info_type_v01, csg_info),
  QMI_IDL_TYPE88(0, 22),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_manual_network_register_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_manual_network_register_info_type_v01, mobile_country_code),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_manual_network_register_info_type_v01, mobile_network_code),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_manual_network_register_info_type_v01, radio_access_technology),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_serving_system_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_serving_system_type_v01, registration_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_serving_system_type_v01, cs_attach_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_serving_system_type_v01, ps_attach_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_serving_system_type_v01, selected_network),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_serving_system_type_v01, radio_if),
  NAS_RADIO_IF_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_serving_system_type_v01, radio_if) - QMI_IDL_OFFSET8(nas_serving_system_type_v01, radio_if_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_plmn_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_type_v01, mobile_country_code),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_type_v01, mobile_network_code),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(nas_plmn_type_v01, network_description),
  NAS_NETWORK_DESCRIPTION_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_system_id_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_system_id_type_v01, sid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_system_id_type_v01, nid),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_base_station_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_base_station_info_type_v01, base_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_base_station_info_type_v01, base_lat),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_base_station_info_type_v01, base_long),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_roaming_indicator_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_roaming_indicator_type_v01, radio_if),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_roaming_indicator_type_v01, roaming_indicator),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_time_zone_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_time_zone_type_v01, lp_sec),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_time_zone_type_v01, ltm_offset),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_time_zone_type_v01, daylt_savings),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_ss_detailed_service_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ss_detailed_service_info_type_v01, srv_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ss_detailed_service_info_type_v01, srv_capability),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ss_detailed_service_info_type_v01, hdr_srv_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ss_detailed_service_info_type_v01, hdr_hybrid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ss_detailed_service_info_type_v01, is_sys_forbidden),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_system_id_ext_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_system_id_ext_type_v01, mcc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_system_id_ext_type_v01, imsi_11_12),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_gw_sys_info3_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_gw_sys_info3_type_v01, cs_bar_status),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_gw_sys_info3_type_v01, ps_bar_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_universal_time_and_local_time_zone_3gpp_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_universal_time_and_local_time_zone_3gpp_type_v01, year),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_universal_time_and_local_time_zone_3gpp_type_v01, month),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_universal_time_and_local_time_zone_3gpp_type_v01, day),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_universal_time_and_local_time_zone_3gpp_type_v01, hour),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_universal_time_and_local_time_zone_3gpp_type_v01, minute),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_universal_time_and_local_time_zone_3gpp_type_v01, second),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_universal_time_and_local_time_zone_3gpp_type_v01, time_zone),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_plmn_id_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_id_type_v01, mcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_id_type_v01, mnc),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp2_home_network_ext_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_3gpp2_home_network_ext_type_v01, mcc_mnc),
  QMI_IDL_TYPE88(0, 35),
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp2_home_network_ext_type_v01, network_desc_display),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp2_home_network_ext_type_v01, network_desc_encoding),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_home_network_ext_type_v01, network_desc),
  NAS_NETWORK_DESCRIPTION_MAX_V01,
  QMI_IDL_OFFSET8(nas_3gpp2_home_network_ext_type_v01, network_desc) - QMI_IDL_OFFSET8(nas_3gpp2_home_network_ext_type_v01, network_desc_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_mcs_digit_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_mcs_digit_type_v01, is_3gpp_network),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_mcs_digit_type_v01, mnc_includes_pcs_digit),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_preferred_networks_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_preferred_networks_type_v01, mobile_country_code),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_preferred_networks_type_v01, mobile_network_code),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_preferred_networks_type_v01, radio_access_technology),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_forbidden_networks_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_forbidden_networks_type_v01, mobile_country_code),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_forbidden_networks_type_v01, mobile_network_code),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_accolc_set_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_accolc_set_type_v01, spc),
  NAS_SPC_MAX_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_accolc_set_type_v01, accolc),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_registration_parameters_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_registration_parameters_type_v01, reg_home_sid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_registration_parameters_type_v01, reg_foreign_sid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_registration_parameters_type_v01, reg_foreign_nid),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_hdr_scp_config_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_scp_config_type_v01, state),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_scp_config_type_v01, protocol_mask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_scp_config_type_v01, broadcast_mask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_scp_config_type_v01, application_mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_net_sel_pref_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_net_sel_pref_type_v01, net_sel_pref),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_net_sel_pref_type_v01, mcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_net_sel_pref_type_v01, mnc),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_csg_nw_iden_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_iden_type_v01, mcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_iden_type_v01, mnc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_iden_type_v01, mnc_includes_pcs_digit),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_iden_type_v01, id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_csg_nw_iden_type_v01, rat),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_ddtm_preference_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_ddtm_preference_type_v01, ddtm_pref),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_ddtm_preference_type_v01, ddtm_action),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_ddtm_preference_type_v01, so_list_action),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_ddtm_preference_type_v01, so),
  NAS_SO_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_ddtm_preference_type_v01, so) - QMI_IDL_OFFSET8(nas_ddtm_preference_type_v01, so_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_ddtm_settings_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_ddtm_settings_type_v01, curr_ddtm_status),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_ddtm_settings_type_v01, ddtm_pref),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_ddtm_settings_type_v01, ddtm_action),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_ddtm_settings_type_v01, so_list_action),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_ddtm_settings_type_v01, so),
  NAS_SO_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_ddtm_settings_type_v01, so) - QMI_IDL_OFFSET8(nas_ddtm_settings_type_v01, so_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_service_provider_name_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_service_provider_name_type_v01, display_cond),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_service_provider_name_type_v01, spn),
  NAS_SERVICE_PROVIDER_NAME_MAX_V01,
  QMI_IDL_OFFSET8(nas_service_provider_name_type_v01, spn) - QMI_IDL_OFFSET8(nas_service_provider_name_type_v01, spn_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_operator_plmn_list_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_operator_plmn_list_type_v01, mcc),
  NAS_MCC_MNC_MAX_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_operator_plmn_list_type_v01, mnc),
  NAS_MCC_MNC_MAX_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_operator_plmn_list_type_v01, lac1),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_operator_plmn_list_type_v01, lac2),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_operator_plmn_list_type_v01, pnn_rec_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_plmn_network_name_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_plmn_network_name_type_v01, coding_scheme),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_plmn_network_name_type_v01, ci),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_plmn_network_name_type_v01, long_name_spare_bits),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_plmn_network_name_type_v01, short_name_spare_bits),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_network_name_type_v01, long_name),
  NAS_LONG_NAME_MAX_V01,
  QMI_IDL_OFFSET8(nas_plmn_network_name_type_v01, long_name) - QMI_IDL_OFFSET8(nas_plmn_network_name_type_v01, long_name_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_plmn_network_name_type_v01, short_name),
  NAS_SHORT_NAME_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_plmn_network_name_type_v01, short_name) - QMI_IDL_OFFSET16RELATIVE(nas_plmn_network_name_type_v01, short_name_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp2_home_sid_nid_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_home_sid_nid_info_type_v01, sid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_home_sid_nid_info_type_v01, nid),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp2_min_based_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_min_based_info_type_v01, mcc_m),
  NAS_MCC_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_min_based_info_type_v01, imsi_m_11_12),
  NAS_IMSI_11_12_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_min_based_info_type_v01, imsi_m_s1),
  NAS_IMSI_MIN1_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_min_based_info_type_v01, imsi_m_s2),
  NAS_IMSI_MIN2_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp2_true_imsi_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_true_imsi_info_type_v01, mcc_t),
  NAS_MCC_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_true_imsi_info_type_v01, imsi_t_11_12),
  NAS_IMSI_11_12_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_true_imsi_info_type_v01, imsi_t_s1),
  NAS_IMSI_MIN1_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_true_imsi_info_type_v01, imsi_t_s2),
  NAS_IMSI_MIN2_LEN_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_true_imsi_info_type_v01, imsi_t_addr_num),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_channel_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_channel_info_type_v01, pri_ch_a),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_channel_info_type_v01, pri_ch_b),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_channel_info_type_v01, sec_ch_a),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_channel_info_type_v01, sec_ch_b),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_nmr_cell_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_type_v01, nmr_cell_id),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_type_v01, nmr_plmn),
  NAS_PLMN_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_type_v01, nmr_lac),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_type_v01, nmr_arfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_type_v01, nmr_bsic),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_type_v01, nmr_rx_lev),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_geran_cell_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, cell_id),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, plmn),
  NAS_PLMN_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, lac),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, arfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, bsic),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, timing_advance),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, rx_lev),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, nmr_cell_info),
  NAS_NMR_MAX_NUM_V01,
  QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, nmr_cell_info) - QMI_IDL_OFFSET8(nas_geran_cell_info_type_v01, nmr_cell_info_len),
  QMI_IDL_TYPE88(0, 54),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_umts_monitored_cell_set_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_info_type_v01, umts_uarfcn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_info_type_v01, umts_psc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_info_type_v01, umts_rscp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_info_type_v01, umts_ecio),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_umts_geran_nbr_cell_set_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_info_type_v01, geran_arfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_info_type_v01, geran_bsic_ncc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_info_type_v01, geran_bsic_bcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_info_type_v01, geran_rssi),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_umts_lte_nbr_cell_set_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_lte_nbr_cell_set_info_type_v01, earfcn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_lte_nbr_cell_set_info_type_v01, pci),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_umts_lte_nbr_cell_set_info_type_v01, rsrp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_umts_lte_nbr_cell_set_info_type_v01, rsrq),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_lte_nbr_cell_set_info_type_v01, srxlev),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_umts_lte_nbr_cell_set_info_type_v01, cell_is_tdd),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_umts_cell_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, cell_id),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, plmn),
  NAS_PLMN_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, lac),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, uarfcn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, psc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, rscp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, ecio),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, umts_monitored_cell),
  NAS_UMTS_MAX_MONITORED_CELL_SET_NUM_V01,
  QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, umts_monitored_cell) - QMI_IDL_OFFSET8(nas_umts_cell_info_type_v01, umts_monitored_cell_len),
  QMI_IDL_TYPE88(0, 56),
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_umts_cell_info_type_v01, umts_geran_nbr_cell),
  NAS_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_umts_cell_info_type_v01, umts_geran_nbr_cell) - QMI_IDL_OFFSET16RELATIVE(nas_umts_cell_info_type_v01, umts_geran_nbr_cell_len),
  QMI_IDL_TYPE88(0, 57),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_cell_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_cell_info_type_v01, sid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_cell_info_type_v01, nid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_cell_info_type_v01, base_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_cell_info_type_v01, refpn),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_cell_info_type_v01, base_lat),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_cell_info_type_v01, base_long),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_ngbr_cell_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_cell_type_v01, pci),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_cell_type_v01, rsrq),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_cell_type_v01, rsrp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_cell_type_v01, rssi),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_cell_type_v01, srxlev),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_intra_freq_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, ue_in_idle),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, plmn),
  NAS_PLMN_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, tac),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, global_cell_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, earfcn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, serving_cell_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, cell_resel_priority),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, s_non_intra_search),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, thresh_serving_low),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, s_intra_search),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, cells),
  NAS_MAX_LTE_NGBR_NUM_CELLS_V01,
  QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, cells) - QMI_IDL_OFFSET8(nas_lte_intra_freq_type_v01, cells_len),
  QMI_IDL_TYPE88(0, 61),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_inter_freq_freqs_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_freqs_type_v01, earfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_freqs_type_v01, threshX_low),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_freqs_type_v01, threshX_high),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_freqs_type_v01, cell_resel_priority),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_freqs_type_v01, cells),
  NAS_MAX_LTE_NGBR_NUM_CELLS_V01,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_freqs_type_v01, cells) - QMI_IDL_OFFSET8(nas_lte_inter_freq_freqs_type_v01, cells_len),
  QMI_IDL_TYPE88(0, 61),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_inter_freq_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_type_v01, ue_in_idle),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_type_v01, freqs),
  NAS_MAX_LTE_NGBR_NUM_FREQS_V01,
  QMI_IDL_OFFSET8(nas_lte_inter_freq_type_v01, freqs) - QMI_IDL_OFFSET8(nas_lte_inter_freq_type_v01, freqs_len),
  QMI_IDL_TYPE88(0, 63),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_ngbr_gsm_cell_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_cell_type_v01, arfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_cell_type_v01, band_1900),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_cell_type_v01, cell_id_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_cell_type_v01, bsic_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_cell_type_v01, rssi),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_cell_type_v01, srxlev),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_ngbr_gsm_freq_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_freq_type_v01, cell_resel_priority),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_freq_type_v01, thresh_gsm_high),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_freq_type_v01, thresh_gsm_low),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_freq_type_v01, ncc_permitted),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_freq_type_v01, cells),
  NAS_MAX_LTE_NGBR_GSM_NUM_CELLS_V01,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_freq_type_v01, cells) - QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_freq_type_v01, cells_len),
  QMI_IDL_TYPE88(0, 65),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_ngbr_gsm_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_type_v01, ue_in_idle),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_type_v01, freqs),
  NAS_MAX_LTE_NGBR_GSM_NUM_FREQS_V01,
  QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_type_v01, freqs) - QMI_IDL_OFFSET8(nas_lte_ngbr_gsm_type_v01, freqs_len),
  QMI_IDL_TYPE88(0, 66),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_ngbr_wcdma_cell_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_cell_type_v01, psc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_cell_type_v01, cpich_rscp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_cell_type_v01, cpich_ecno),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_cell_type_v01, srxlev),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_ngbr_wcdma_freq_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_freq_type_v01, uarfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_freq_type_v01, cell_resel_priority),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_freq_type_v01, thresh_Xhigh),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_freq_type_v01, thresh_Xlow),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_freq_type_v01, cells),
  NAS_MAX_LTE_NGBR_WCDMA_NUM_CELLS_V01,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_freq_type_v01, cells) - QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_freq_type_v01, cells_len),
  QMI_IDL_TYPE88(0, 68),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_ngbr_wcdma_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_type_v01, ue_in_idle),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_type_v01, freqs),
  NAS_MAX_LTE_NGBR_WCDMA_NUM_FREQS_V01,
  QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_type_v01, freqs) - QMI_IDL_OFFSET8(nas_lte_ngbr_wcdma_type_v01, freqs_len),
  QMI_IDL_TYPE88(0, 69),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_wcdma_ngbr_lte_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_ngbr_lte_type_v01, wcdma_rrc_state),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_wcdma_ngbr_lte_type_v01, umts_lte_nbr_cell),
  NAS_UMTS_LTE_MAX_NBR_CELL_SET_NUM_V01,
  QMI_IDL_OFFSET8(nas_wcdma_ngbr_lte_type_v01, umts_lte_nbr_cell) - QMI_IDL_OFFSET8(nas_wcdma_ngbr_lte_type_v01, umts_lte_nbr_cell_len),
  QMI_IDL_TYPE88(0, 58),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_rx_power_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_rx_power_type_v01, rx0_agc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_rx_power_type_v01, rx1_agc),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_gsm_cell_info_ext_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_gsm_cell_info_ext_type_v01, g_ta),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_gsm_cell_info_ext_type_v01, g_bcch),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_wcdma_cell_info_ext_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_cell_info_ext_type_v01, w_agc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_cell_info_ext_type_v01, w_txagc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_cell_info_ext_type_v01, w_dl_bler),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_wcdma_active_set_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_info_type_v01, psc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_info_type_v01, cell_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_info_type_v01, rscp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_info_type_v01, ecio),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_info_type_v01, uarfcn),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_wcdma_active_set_reference_rl_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_reference_rl_type_v01, cell_id),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_reference_rl_type_v01, plmn),
  NAS_PLMN_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_reference_rl_type_v01, lac),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_reference_rl_type_v01, uarfcn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_reference_rl_type_v01, psc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_reference_rl_type_v01, rac),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_eons_plmn_name_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, spn_enc),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, spn),
  NAS_SPN_LEN_MAX_V01,
  QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, spn) - QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, spn_len),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, plmn_short_name_enc),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, plmn_short_name_ci),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, plmn_short_spare_bits),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, plmn_short_name),
  NAS_PLMN_NAME_MAX_V01,
  QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, plmn_short_name) - QMI_IDL_OFFSET8(nas_3gpp_eons_plmn_name_type_v01, plmn_short_name_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_3gpp_eons_plmn_name_type_v01, plmn_long_name_enc),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_3gpp_eons_plmn_name_type_v01, plmn_long_name_ci),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_3gpp_eons_plmn_name_type_v01, plmn_long_spare_bits),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_3gpp_eons_plmn_name_type_v01, plmn_long_name),
  NAS_PLMN_NAME_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_3gpp_eons_plmn_name_type_v01, plmn_long_name) - QMI_IDL_OFFSET16RELATIVE(nas_3gpp_eons_plmn_name_type_v01, plmn_long_name_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_display_bit_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_display_bit_type_v01, is_spn_set),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_display_bit_type_v01, is_plmn_set),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_standby_pref_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_standby_pref_type_v01, standby_pref),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_standby_pref_type_v01, priority_subs),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_standby_pref_type_v01, active_subs),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_standby_pref_type_v01, default_data_subs),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_technology_pref_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_technology_pref_type_v01, technology_pref),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_technology_pref_type_v01, duration),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_active_technology_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_active_technology_type_v01, technology_pref),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_active_technology_type_v01, duration),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_julian_time_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_julian_time_type_v01, year),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_julian_time_type_v01, month),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_julian_time_type_v01, day),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_julian_time_type_v01, hour),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_julian_time_type_v01, minute),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_julian_time_type_v01, second),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_julian_time_type_v01, day_of_week),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_reg_reject_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_reg_reject_info_type_v01, reject_srv_domain),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_reg_reject_info_type_v01, rej_cause),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp2_srv_status_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp2_srv_status_info_type_v01, srv_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp2_srv_status_info_type_v01, is_pref_data_path),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_srv_status_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp_srv_status_info_type_v01, srv_status),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_3gpp_srv_status_info_type_v01, true_srv_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_srv_status_info_type_v01, is_pref_data_path),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_common_network_id_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_common_network_id_type_v01, mcc),
  NAS_MCC_MNC_MAX_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_common_network_id_type_v01, mnc),
  NAS_MCC_MNC_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_common_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_common_sys_info_type_v01, srv_domain_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_common_sys_info_type_v01, srv_domain),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_common_sys_info_type_v01, srv_capability_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_common_sys_info_type_v01, srv_capability),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_common_sys_info_type_v01, roam_status_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_common_sys_info_type_v01, roam_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_common_sys_info_type_v01, is_sys_forbidden_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_common_sys_info_type_v01, is_sys_forbidden),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_only_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, p_rev_in_use_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, p_rev_in_use),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, bs_p_rev_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, bs_p_rev),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, ccs_supported_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, ccs_supported),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, cdma_sys_id_valid),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, cdma_sys_id),
  QMI_IDL_TYPE88(0, 27),
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, bs_info_valid),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, bs_info),
  QMI_IDL_TYPE88(0, 28),
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, packet_zone_valid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, packet_zone),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, network_id_valid),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_cdma_only_sys_info_type_v01, network_id),
  QMI_IDL_TYPE88(0, 86),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_hdr_only_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_only_sys_info_type_v01, hdr_personality_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_hdr_only_sys_info_type_v01, hdr_personality),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_only_sys_info_type_v01, hdr_active_prot_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_hdr_only_sys_info_type_v01, hdr_active_prot),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_only_sys_info_type_v01, is856_sys_id_valid),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_only_sys_info_type_v01, is856_sys_id),
  NAS_IS_856_MAX_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_gsm_only_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_gsm_only_sys_info_type_v01, egprs_supp_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_gsm_only_sys_info_type_v01, egprs_supp),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_gsm_only_sys_info_type_v01, dtm_supp_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_gsm_only_sys_info_type_v01, dtm_supp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_wcdma_only_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_only_sys_info_type_v01, hs_call_status_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_wcdma_only_sys_info_type_v01, hs_call_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_only_sys_info_type_v01, hs_ind_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_wcdma_only_sys_info_type_v01, hs_ind),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_only_sys_info_type_v01, psc_valid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_only_sys_info_type_v01, psc),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_only_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_only_sys_info_type_v01, tac_valid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_only_sys_info_type_v01, tac),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_hdr_only_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_hdr_only_sys_info_type_v01, is_sys_prl_match_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_hdr_only_sys_info_type_v01, is_sys_prl_match),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_only_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_only_sys_info_type_v01, lac_valid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_only_sys_info_type_v01, lac),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_only_sys_info_type_v01, cell_id_valid),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_only_sys_info_type_v01, cell_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_only_sys_info_type_v01, reg_reject_info_valid),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_3gpp_only_sys_info_type_v01, reg_reject_info),
  QMI_IDL_TYPE88(0, 83),
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_only_sys_info_type_v01, network_id_valid),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_3gpp_only_sys_info_type_v01, network_id),
  QMI_IDL_TYPE88(0, 86),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_sys_info_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_cdma_sys_info_type_v01, common_sys_info),
  QMI_IDL_TYPE88(0, 87),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_cdma_sys_info_type_v01, cdma_hdr_only_sys_info),
  QMI_IDL_TYPE88(0, 93),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_cdma_sys_info_type_v01, cdma_specific_sys_info),
  QMI_IDL_TYPE88(0, 88),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_hdr_sys_info_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_hdr_sys_info_type_v01, common_sys_info),
  QMI_IDL_TYPE88(0, 87),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_hdr_sys_info_type_v01, cdma_hdr_only_sys_info),
  QMI_IDL_TYPE88(0, 93),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_hdr_sys_info_type_v01, hdr_specific_sys_info),
  QMI_IDL_TYPE88(0, 89),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_gsm_sys_info_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_gsm_sys_info_type_v01, common_sys_info),
  QMI_IDL_TYPE88(0, 87),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_gsm_sys_info_type_v01, threegpp_specific_sys_info),
  QMI_IDL_TYPE88(0, 94),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_gsm_sys_info_type_v01, gsm_specific_sys_info),
  QMI_IDL_TYPE88(0, 90),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_wcdma_sys_info_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_wcdma_sys_info_type_v01, common_sys_info),
  QMI_IDL_TYPE88(0, 87),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_wcdma_sys_info_type_v01, threegpp_specific_sys_info),
  QMI_IDL_TYPE88(0, 94),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_wcdma_sys_info_type_v01, wcdma_specific_sys_info),
  QMI_IDL_TYPE88(0, 91),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_sys_info_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_sys_info_type_v01, common_sys_info),
  QMI_IDL_TYPE88(0, 87),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_sys_info_type_v01, threegpp_specific_sys_info),
  QMI_IDL_TYPE88(0, 94),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_sys_info_type_v01, lte_specific_sys_info),
  QMI_IDL_TYPE88(0, 92),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_tdscdma_only_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, hs_call_status_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, hs_call_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, hs_ind_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, hs_ind),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, cell_parameter_id_valid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, cell_parameter_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, cell_broadcast_cap_valid),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, cell_broadcast_cap),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, cs_bar_status_valid),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, cs_bar_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, ps_bar_status_valid),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, ps_bar_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, cipher_domain_valid),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_tdscdma_only_sys_info_type_v01, cipher_domain),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_tdscdma_sys_info_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_tdscdma_sys_info_type_v01, common_sys_info),
  QMI_IDL_TYPE88(0, 87),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_tdscdma_sys_info_type_v01, threegpp_specific_sys_info),
  QMI_IDL_TYPE88(0, 94),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_tdscdma_sys_info_type_v01, tdscdma_specific_sys_info),
  QMI_IDL_TYPE88(0, 100),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_sys_info2_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_sys_info2_type_v01, geo_sys_idx),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_sys_info2_type_v01, reg_prd),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_hdr_sys_info2_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_sys_info2_type_v01, geo_sys_idx),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_gsm_sys_info2_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_gsm_sys_info2_type_v01, geo_sys_idx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_gsm_sys_info2_type_v01, cell_broadcast_cap),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_wcdma_sys_info2_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_sys_info2_type_v01, geo_sys_idx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_sys_info2_type_v01, cell_broadcast_cap),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_sys_info2_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sys_info2_type_v01, geo_sys_idx),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_common_sig_info_param_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_common_sig_info_param_type_v01, rssi),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_common_sig_info_param_type_v01, ecio),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_hdr_sig_info_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_hdr_sig_info_type_v01, common_sig_str),
  QMI_IDL_TYPE88(0, 107),
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_hdr_sig_info_type_v01, sinr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_sig_info_type_v01, io),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_sig_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sig_info_type_v01, rssi),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sig_info_type_v01, rsrq),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sig_info_type_v01, rsrp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sig_info_type_v01, snr),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_tdscdma_sig_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_sig_info_type_v01, rssi),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_sig_info_type_v01, rscp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_sig_info_type_v01, ecio),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tdscdma_sig_info_type_v01, sinr),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_sig_rpt_config_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_lte_sig_rpt_config_type_v01, rpt_rate),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_lte_sig_rpt_config_type_v01, avg_period),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_set_rx_diversity_req_param_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_rx_diversity_req_param_type_v01, radio_if),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_rx_diversity_req_param_type_v01, rx_chain_bitmask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_rx_chain_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_rx_chain_info_type_v01, is_radio_tuned),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_rx_chain_info_type_v01, rx_pwr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_rx_chain_info_type_v01, ecio),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_rx_chain_info_type_v01, rscp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_rx_chain_info_type_v01, rsrp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_rx_chain_info_type_v01, phase),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_tx_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tx_info_type_v01, is_in_traffic),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tx_info_type_v01, tx_pwr),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_akey_with_spc_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_akey_with_spc_type_v01, spc),
  NAS_SPC_MAX_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_akey_with_spc_type_v01, akey),
  QMI_NAS_AKEY_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_plmn_id_ext_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_id_ext_type_v01, mcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_id_ext_type_v01, mnc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_id_ext_type_v01, mnc_includes_pcs_digit),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_spn_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_spn_type_v01, spn_enc),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_spn_type_v01, spn),
  NAS_SPN_LEN_MAX_V01,
  QMI_IDL_OFFSET8(nas_spn_type_v01, spn) - QMI_IDL_OFFSET8(nas_spn_type_v01, spn_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_plmn_name_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_plmn_name_type_v01, plmn_name_enc),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_plmn_name_type_v01, plmn_name_ci),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_plmn_name_type_v01, plmn_spare_bits),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_plmn_name_type_v01, plmn_name),
  NAS_PLMN_NAME_MAX_V01,
  QMI_IDL_OFFSET8(nas_plmn_name_type_v01, plmn_name) - QMI_IDL_OFFSET8(nas_plmn_name_type_v01, plmn_name_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_bs_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, pilot_type),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, sid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, nid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, base_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, pilot_pn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, pilot_strength),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, base_lat),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, base_long),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_bs_info_type_v01, time_stamp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_position_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_position_info_type_v01, ue_in_idle),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_cdma_position_info_type_v01, bs),
  NAS_CDMA_POSITION_INFO_MAX_V01,
  QMI_IDL_OFFSET8(nas_cdma_position_info_type_v01, bs) - QMI_IDL_OFFSET8(nas_cdma_position_info_type_v01, bs_len),
  QMI_IDL_TYPE88(0, 119),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_tds_cell_info_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_tds_cell_info_type_v01, plmn),
  QMI_IDL_TYPE88(0, 116),
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_tds_cell_info_type_v01, lac),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_tds_cell_info_type_v01, uarfcn),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tds_cell_info_type_v01, cell_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tds_cell_info_type_v01, cell_parameter_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tds_cell_info_type_v01, pathloss),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tds_cell_info_type_v01, timing_advance),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tds_cell_info_type_v01, rscp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_tds_nbr_cell_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_tds_nbr_cell_info_type_v01, uarfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_tds_nbr_cell_info_type_v01, cell_parameter_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_tds_nbr_cell_info_type_v01, rscp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_cphy_mbsfn_area_signal_strength_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_cphy_mbsfn_area_signal_strength_type_v01, area_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_lte_cphy_mbsfn_area_signal_strength_type_v01, snr),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_cphy_mbsfn_area_signal_strength_type_v01, signal_level),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_ims_registration_state_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_ims_registration_state_type_v01, call_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_ims_registration_state_type_v01, is_registered),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_ims_pref_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_ims_pref_type_v01, sys_mode),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_ims_pref_type_v01, ims_pref_call_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_cdma_avoid_sys_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_avoid_sys_info_type_v01, sid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_avoid_sys_info_type_v01, nid),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_avoid_sys_info_type_v01, mnc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_avoid_sys_info_type_v01, mcc),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_network_time_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_network_time_type_v01, universal_time),
  QMI_IDL_TYPE88(0, 82),
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_network_time_type_v01, time_zone),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_network_time_type_v01, daylt_sav_adj),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_network_time_type_v01, radio_if),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_sib16_julian_time_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_julian_time_type_v01, year),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_julian_time_type_v01, month),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_julian_time_type_v01, day),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_julian_time_type_v01, hour),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_julian_time_type_v01, minute),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_julian_time_type_v01, second),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_julian_time_type_v01, millisecond),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_julian_time_type_v01, day_of_week),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_embms_tmgi_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_embms_tmgi_type_v01, mrb_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_embms_tmgi_type_v01, session_id_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_embms_tmgi_type_v01, session_id),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_embms_tmgi_type_v01, tmgi_identifier),
  NAS_TMGI_IDENTIFIER_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_embms_signal_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_embms_signal_type_v01, area_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_lte_embms_signal_type_v01, snr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_lte_embms_signal_type_v01, excess_snr),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_embms_signal_type_v01, tmgi_info),
  NAS_TMGI_BEARER_INFO_MAX_V01,
  QMI_IDL_OFFSET8(nas_lte_embms_signal_type_v01, tmgi_info) - QMI_IDL_OFFSET8(nas_lte_embms_signal_type_v01, tmgi_info_len),
  QMI_IDL_TYPE88(0, 129),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lte_cphy_ca_indicator_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_cphy_ca_indicator_type_v01, pci),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lte_cphy_ca_indicator_type_v01, freq),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_lte_cphy_ca_indicator_type_v01, scell_state),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_oplmn_entry_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_oplmn_entry_type_v01, plmn),
  3,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_oplmn_entry_type_v01, access_tech),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_oplmn_list_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_oplmn_list_type_v01, list_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_oplmn_list_type_v01, total_list_entries),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_oplmn_list_type_v01, oplmn),
  ((NAS_MAX_BUILTIN_OPLMN_ENTRIES_V01) & 0xFF), ((NAS_MAX_BUILTIN_OPLMN_ENTRIES_V01) >> 8),
  QMI_IDL_OFFSET8(nas_oplmn_list_type_v01, oplmn) - QMI_IDL_OFFSET8(nas_oplmn_list_type_v01, oplmn_len),
  QMI_IDL_TYPE88(0, 132),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_3gpp_network_scan_result_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_network_scan_result_type_v01, mobile_country_code),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_network_scan_result_type_v01, mobile_network_code),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_network_scan_result_type_v01, network_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_network_scan_result_type_v01, rat),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_3gpp_network_scan_result_type_v01, mnc_includes_pcs_digit),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(nas_3gpp_network_scan_result_type_v01, network_description),
  NAS_NETWORK_DESCRIPTION_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_lang_plmn_names_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lang_plmn_names_type_v01, plmn_long_name),
  NAS_ALT_LANG_NAME_LEN_MAX_V01,
  QMI_IDL_OFFSET8(nas_lang_plmn_names_type_v01, plmn_long_name) - QMI_IDL_OFFSET8(nas_lang_plmn_names_type_v01, plmn_long_name_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_lang_plmn_names_type_v01, plmn_short_name),
  NAS_ALT_LANG_NAME_LEN_MAX_V01,
  QMI_IDL_OFFSET8(nas_lang_plmn_names_type_v01, plmn_short_name) - QMI_IDL_OFFSET8(nas_lang_plmn_names_type_v01, plmn_short_name_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_lang_plmn_names_type_v01, lang_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_csg_nw_signal_strength_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_signal_strength_info_type_v01, mcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_signal_strength_info_type_v01, mnc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_signal_strength_info_type_v01, csg_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_csg_nw_signal_strength_info_type_v01, signal_strength),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_umts_monitored_cell_set_ext_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_ext_info_type_v01, umts_uarfcn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_ext_info_type_v01, umts_psc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_ext_info_type_v01, umts_rscp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_ext_info_type_v01, umts_ecio),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_ext_info_type_v01, umts_squal),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_ext_info_type_v01, umts_srxlev),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_ext_info_type_v01, umts_rank),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_umts_monitored_cell_set_ext_info_type_v01, umts_set),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_umts_geran_nbr_cell_set_ext_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_ext_info_type_v01, geran_arfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_ext_info_type_v01, geran_bsic_ncc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_ext_info_type_v01, geran_bsic_bcc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_ext_info_type_v01, geran_rssi),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_geran_nbr_cell_set_ext_info_type_v01, geran_rank),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_umts_cell_ext_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, cell_id),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, plmn),
  NAS_PLMN_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, lac),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, uarfcn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, psc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, rscp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, ecio),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, squal),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, srxlev),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, umts_monitored_ext_ext_cell),
  NAS_UMTS_MAX_MONITORED_CELL_SET_NUM_V01,
  QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, umts_monitored_ext_ext_cell) - QMI_IDL_OFFSET8(nas_umts_cell_ext_info_type_v01, umts_monitored_ext_ext_cell_len),
  QMI_IDL_TYPE88(0, 137),
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_umts_cell_ext_info_type_v01, umts_geran_ext_nbr_cell),
  NAS_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_umts_cell_ext_info_type_v01, umts_geran_ext_nbr_cell) - QMI_IDL_OFFSET16RELATIVE(nas_umts_cell_ext_info_type_v01, umts_geran_ext_nbr_cell_len),
  QMI_IDL_TYPE88(0, 138),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_wcdma_active_set_ext_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, psc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, cell_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, rscp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, ecio),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, uarfcn),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, sf),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, phy_chan_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, slot_format),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_wcdma_active_set_ext_info_type_v01, is_compressed_mode_on),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_nmr_cell_info_ext_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_cell_id),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_plmn),
  NAS_PLMN_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_lac),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_arfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_bsic),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_rx_lev),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_c1),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_c2),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_c31),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_nmr_cell_info_ext_type_v01, nmr_c32),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_geran_cell_info_ext_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, cell_id),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, plmn),
  NAS_PLMN_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, lac),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, arfcn),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, bsic),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, timing_advance),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, rx_lev),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, nmr_cell_info),
  NAS_NMR_MAX_NUM_V01,
  QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, nmr_cell_info) - QMI_IDL_OFFSET8(nas_geran_cell_info_ext_type_v01, nmr_cell_info_len),
  QMI_IDL_TYPE88(0, 141),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t nas_geran_eng_mode_scell_config_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_geran_eng_mode_scell_config_type_v01, pbcch_present),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_geran_eng_mode_scell_config_type_v01, gprs_rxlev_access_min),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_geran_eng_mode_scell_config_type_v01, gprs_ms_txpwr_max_cch),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * nas_reset_req_msg is empty
 * static const uint8_t nas_reset_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_reset_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_reset_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_abort_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_abort_req_msg_v01, tx_id)
};

static const uint8_t nas_abort_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_abort_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_set_event_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, signal_strength) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, signal_strength_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, signal_strength),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_rf_band_info) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_rf_band_info_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_rf_band_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_reg_reject) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_reg_reject_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_reg_reject),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, rssi_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, rssi_indicator_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, rssi_indicator),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, ecio_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, ecio_indicator_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, ecio_indicator),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, io_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, io_indicator_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, io_indicator),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, sinr_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, sinr_indicator_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, sinr_indicator),
  QMI_IDL_TYPE88(0, 4),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_error_rate) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_error_rate_valid)),
  0x17,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, report_error_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, rsrq_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, rsrq_indicator_valid)),
  0x18,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, rsrq_indicator),
  QMI_IDL_TYPE88(0, 5),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, ecio_threshold_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, ecio_threshold_indicator_valid)),
  0x19,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, ecio_threshold_indicator),
  QMI_IDL_TYPE88(0, 6),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, sinr_threshold_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, sinr_threshold_indicator_valid)),
  0x1A,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, sinr_threshold_indicator),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, lte_snr_delta_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, lte_snr_delta_indicator_valid)),
  0x1B,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, lte_snr_delta_indicator),
  QMI_IDL_TYPE88(0, 9),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, lte_rsrp_delta_indicator) - QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, lte_rsrp_delta_indicator_valid)),
  0x1C,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_req_msg_v01, lte_rsrp_delta_indicator),
  QMI_IDL_TYPE88(0, 8)
};

static const uint8_t nas_set_event_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_event_report_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_event_report_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_event_report_ind_msg_v01, signal_strength) - QMI_IDL_OFFSET8(nas_event_report_ind_msg_v01, signal_strength_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_event_report_ind_msg_v01, signal_strength),
  QMI_IDL_TYPE88(0, 10),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_event_report_ind_msg_v01, rf_band_info_list) - QMI_IDL_OFFSET8(nas_event_report_ind_msg_v01, rf_band_info_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_event_report_ind_msg_v01, rf_band_info_list),
  NAS_RADIO_IF_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_event_report_ind_msg_v01, rf_band_info_list) - QMI_IDL_OFFSET8(nas_event_report_ind_msg_v01, rf_band_info_list_len),
  QMI_IDL_TYPE88(0, 11),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, registration_reject_reason) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, registration_reject_reason_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, registration_reject_reason),
  QMI_IDL_TYPE88(0, 12),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, rssi) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, rssi_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, rssi),
  QMI_IDL_TYPE88(0, 13),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, ecio) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, ecio_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, ecio),
  QMI_IDL_TYPE88(0, 14),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, io) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, io_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, io),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, sinr) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, sinr_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, sinr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, error_rate) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, error_rate_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, error_rate),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, rsrq) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, rsrq_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, rsrq),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, snr) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, snr_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, snr),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, rsrp) - QMI_IDL_OFFSET16RELATIVE(nas_event_report_ind_msg_v01, rsrp_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_event_report_ind_msg_v01, rsrp)
};

static const uint8_t nas_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_sys_sel_pref) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_sys_sel_pref_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_sys_sel_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_ddtm_events) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_ddtm_events_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_ddtm_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, req_serving_system) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, req_serving_system_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, req_serving_system),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, dual_standby_pref) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, dual_standby_pref_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, dual_standby_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, subscription_info) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, subscription_info_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, subscription_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_network_time) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_network_time_valid)),
  0x17,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_network_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, sys_info) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, sys_info_valid)),
  0x18,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, sys_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, sig_info) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, sig_info_valid)),
  0x19,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, sig_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, err_rate) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, err_rate_valid)),
  0x1A,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, err_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_hdr_uati) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_hdr_uati_valid)),
  0x1B,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_hdr_uati),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_hdr_session_close) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_hdr_session_close_valid)),
  0x1C,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_hdr_session_close),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_managed_roaming) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_managed_roaming_valid)),
  0x1D,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_managed_roaming),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_current_plmn_name) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_current_plmn_name_valid)),
  0x1E,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_current_plmn_name),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_embms_status) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_embms_status_valid)),
  0x1F,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_embms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_rf_band_info) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_rf_band_info_valid)),
  0x20,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_rf_band_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, network_reject) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, network_reject_valid)),
  0x21,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, network_reject),
  QMI_IDL_TYPE88(0, 17),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_operator_name_data) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_operator_name_data_valid)),
  0x22,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_operator_name_data),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_csp_plmn_mode_bit) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_csp_plmn_mode_bit_valid)),
  0x23,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_csp_plmn_mode_bit),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_rtre_cfg) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_rtre_cfg_valid)),
  0x24,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_rtre_cfg),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_ims_pref_status) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_ims_pref_status_valid)),
  0x25,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_ims_pref_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_e911_state_ready_status) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_e911_state_ready_status_valid)),
  0x26,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_e911_state_ready_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_lte_sib16_network_time) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_lte_sib16_network_time_valid)),
  0x27,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_lte_sib16_network_time),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_lte_cphy_ca) - QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_lte_cphy_ca_valid)),
  0x28,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_indication_register_req_msg_v01, reg_lte_cphy_ca)
};

static const uint8_t nas_indication_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_get_signal_strength_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_signal_strength_req_msg_v01, request_mask) - QMI_IDL_OFFSET8(nas_get_signal_strength_req_msg_v01, request_mask_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_signal_strength_req_msg_v01, request_mask)
};

static const uint8_t nas_get_signal_strength_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, signal_strength),
  QMI_IDL_TYPE88(0, 10),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, signal_strength_list) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, signal_strength_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, signal_strength_list),
  ((NAS_SIG_STRENGTH_LIST_MAX_V01) & 0xFF), ((NAS_SIG_STRENGTH_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, signal_strength_list) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, signal_strength_list_len),
  QMI_IDL_TYPE88(0, 18),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, rssi) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, rssi_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, rssi),
  ((NAS_RSSI_LIST_MAX_V01) & 0xFF), ((NAS_RSSI_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, rssi) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, rssi_len),
  QMI_IDL_TYPE88(0, 13),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, ecio) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, ecio_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, ecio),
  ((NAS_ECIO_LIST_MAX_V01) & 0xFF), ((NAS_ECIO_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, ecio) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, ecio_len),
  QMI_IDL_TYPE88(0, 14),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, io) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, io_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, io),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, sinr) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, sinr_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, sinr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, error_rate) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, error_rate_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, error_rate),
  ((NAS_ERROR_RATE_LIST_MAX_V01) & 0xFF), ((NAS_ERROR_RATE_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, error_rate) - QMI_IDL_OFFSET8(nas_get_signal_strength_resp_msg_v01, error_rate_len),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_signal_strength_resp_msg_v01, rsrq) - QMI_IDL_OFFSET16RELATIVE(nas_get_signal_strength_resp_msg_v01, rsrq_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_signal_strength_resp_msg_v01, rsrq),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_signal_strength_resp_msg_v01, snr) - QMI_IDL_OFFSET16RELATIVE(nas_get_signal_strength_resp_msg_v01, snr_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_signal_strength_resp_msg_v01, snr),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_signal_strength_resp_msg_v01, lte_rsrp) - QMI_IDL_OFFSET16RELATIVE(nas_get_signal_strength_resp_msg_v01, lte_rsrp_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_signal_strength_resp_msg_v01, lte_rsrp)
};

static const uint8_t nas_perform_network_scan_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, network_type) - QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, network_type_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, network_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, scan_type) - QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, scan_type_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, scan_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, band_pref) - QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, band_pref_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, lte_band_pref) - QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, lte_band_pref_valid)),
  0x13,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, lte_band_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, tdscdma_band_pref) - QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, tdscdma_band_pref_valid)),
  0x14,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_perform_network_scan_req_msg_v01, tdscdma_band_pref)
};

static const uint8_t nas_perform_network_scan_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_perform_network_scan_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_network_scan_resp_msg_v01, nas_3gpp_network_info) - QMI_IDL_OFFSET8(nas_perform_network_scan_resp_msg_v01, nas_3gpp_network_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_perform_network_scan_resp_msg_v01, nas_3gpp_network_info),
  ((NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) & 0xFF), ((NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_perform_network_scan_resp_msg_v01, nas_3gpp_network_info) - QMI_IDL_OFFSET8(nas_perform_network_scan_resp_msg_v01, nas_3gpp_network_info_len),
  QMI_IDL_TYPE88(0, 19),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, nas_network_radio_access_technology) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, nas_network_radio_access_technology_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_perform_network_scan_resp_msg_v01, nas_network_radio_access_technology),
  ((NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) & 0xFF), ((NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, nas_network_radio_access_technology) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, nas_network_radio_access_technology_len),
  QMI_IDL_TYPE88(0, 20),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, mnc_includes_pcs_digit_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_perform_network_scan_resp_msg_v01, mnc_includes_pcs_digit),
  ((NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) & 0xFF), ((NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, mnc_includes_pcs_digit_len),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, scan_result) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, scan_result_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_perform_network_scan_resp_msg_v01, scan_result),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, csg_info) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, csg_info_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_perform_network_scan_resp_msg_v01, csg_info),
  NAS_3GPP_NETWORK_INFO_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, csg_info) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, csg_info_len),
  QMI_IDL_TYPE88(0, 23),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, csg_sig_info) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, csg_sig_info_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_perform_network_scan_resp_msg_v01, csg_sig_info),
  NAS_3GPP_NETWORK_INFO_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, csg_sig_info) - QMI_IDL_OFFSET16RELATIVE(nas_perform_network_scan_resp_msg_v01, csg_sig_info_len),
  QMI_IDL_TYPE88(0, 136)
};

static const uint8_t nas_initiate_network_register_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, register_action),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, manual_network_register_info) - QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, manual_network_register_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, manual_network_register_info),
  QMI_IDL_TYPE88(0, 24),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, change_duration) - QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, change_duration_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, change_duration),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, mnc_includes_pcs_digit) - QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, mnc_includes_pcs_digit_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_initiate_network_register_req_msg_v01, mnc_includes_pcs_digit)
};

static const uint8_t nas_initiate_network_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_initiate_network_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_initiate_attach_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_initiate_attach_req_msg_v01, ps_attach_action) - QMI_IDL_OFFSET8(nas_initiate_attach_req_msg_v01, ps_attach_action_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_initiate_attach_req_msg_v01, ps_attach_action)
};

static const uint8_t nas_initiate_attach_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_initiate_attach_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_serving_system_req_msg is empty
 * static const uint8_t nas_get_serving_system_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_serving_system_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_serving_system_resp_msg_v01, serving_system),
  QMI_IDL_TYPE88(0, 25),

  0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, roaming_indicator) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, roaming_indicator_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, roaming_indicator),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, data_capabilities) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, data_capabilities_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, data_capabilities),
  NAS_DATA_CAPABILITIES_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, data_capabilities) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, data_capabilities_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, current_plmn) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, current_plmn_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, current_plmn),
  QMI_IDL_TYPE88(0, 26),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, cdma_system_id) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, cdma_system_id_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, cdma_system_id),
  QMI_IDL_TYPE88(0, 27),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, cdma_base_station_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, cdma_base_station_info_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, cdma_base_station_info),
  QMI_IDL_TYPE88(0, 28),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, roaming_indicator_list) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, roaming_indicator_list_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, roaming_indicator_list),
  NAS_ROAMING_INDICATOR_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, roaming_indicator_list) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, roaming_indicator_list_len),
  QMI_IDL_TYPE88(0, 29),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, def_roam_ind) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, def_roam_ind_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, def_roam_ind),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, nas_3gpp_time_zone) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, nas_3gpp_time_zone_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, nas_3gpp_time_zone),
  QMI_IDL_TYPE88(0, 30),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, p_rev_in_use) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, p_rev_in_use_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, p_rev_in_use),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, time_zone) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, time_zone_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, time_zone),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, adj) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, adj_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, adj),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, lac) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, lac_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, lac),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, cell_id) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, cell_id_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, cell_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, ccs) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, ccs_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, ccs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, prl_ind) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, prl_ind_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, prl_ind),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, dtm_ind) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, dtm_ind_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, dtm_ind),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, detailed_service_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, detailed_service_info_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, detailed_service_info),
  QMI_IDL_TYPE88(0, 31),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, cdma_system_id_ext) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, cdma_system_id_ext_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, cdma_system_id_ext),
  QMI_IDL_TYPE88(0, 32),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, hdr_personality) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, hdr_personality_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, hdr_personality),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, tac) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, tac_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, tac),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, call_barring_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, call_barring_status_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, call_barring_status),
  QMI_IDL_TYPE88(0, 33),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, umts_psc) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, umts_psc_valid)),
  0x26,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, umts_psc),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, mnc_includes_pcs_digit_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, mnc_includes_pcs_digit),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, hs_call_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_serving_system_resp_msg_v01, hs_call_status_valid)),
  0x28,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_serving_system_resp_msg_v01, hs_call_status)
};

static const uint8_t nas_serving_system_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_serving_system_ind_msg_v01, serving_system),
  QMI_IDL_TYPE88(0, 25),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, roaming_indicator) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, roaming_indicator_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, roaming_indicator),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, data_capabilities) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, data_capabilities_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, data_capabilities),
  NAS_DATA_CAPABILITIES_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, data_capabilities) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, data_capabilities_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, current_plmn) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, current_plmn_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, current_plmn),
  QMI_IDL_TYPE88(0, 26),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, cdma_system_id) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, cdma_system_id_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, cdma_system_id),
  QMI_IDL_TYPE88(0, 27),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, cdma_base_station_info) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, cdma_base_station_info_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, cdma_base_station_info),
  QMI_IDL_TYPE88(0, 28),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, roaming_indicator_list) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, roaming_indicator_list_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, roaming_indicator_list),
  NAS_ROAMING_INDICATOR_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, roaming_indicator_list) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, roaming_indicator_list_len),
  QMI_IDL_TYPE88(0, 29),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, def_roam_ind) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, def_roam_ind_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, def_roam_ind),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, nas_3gpp_time_zone) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, nas_3gpp_time_zone_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, nas_3gpp_time_zone),
  QMI_IDL_TYPE88(0, 30),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, p_rev_in_use) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, p_rev_in_use_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, p_rev_in_use),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, plmn_description_changed) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, plmn_description_changed_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, plmn_description_changed),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, time_zone) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, time_zone_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, time_zone),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, adj) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, adj_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, adj),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, universal_time_and_local_time_3gpp_zone) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, universal_time_and_local_time_3gpp_zone_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, universal_time_and_local_time_3gpp_zone),
  QMI_IDL_TYPE88(0, 34),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, lac) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, lac_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, lac),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, cell_id) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, cell_id_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, cell_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, ccs) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, ccs_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, ccs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, prl_ind) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, prl_ind_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, prl_ind),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, dtm_ind) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, dtm_ind_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, dtm_ind),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, detailed_service_info) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, detailed_service_info_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, detailed_service_info),
  QMI_IDL_TYPE88(0, 31),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, cdma_system_id_ext) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, cdma_system_id_ext_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, cdma_system_id_ext),
  QMI_IDL_TYPE88(0, 32),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, hdr_personality) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, hdr_personality_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, hdr_personality),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, tac) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, tac_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, tac),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, call_barring_status) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, call_barring_status_valid)),
  0x26,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, call_barring_status),
  QMI_IDL_TYPE88(0, 33),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, srv_sys_no_change) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, srv_sys_no_change_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, srv_sys_no_change),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, umts_psc) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, umts_psc_valid)),
  0x28,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, umts_psc),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, mnc_includes_pcs_digit_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, mnc_includes_pcs_digit),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, hs_call_status) - QMI_IDL_OFFSET16RELATIVE(nas_serving_system_ind_msg_v01, hs_call_status_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_serving_system_ind_msg_v01, hs_call_status)
};

/* 
 * nas_get_home_network_req_msg is empty
 * static const uint8_t nas_get_home_network_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_home_network_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_home_network_resp_msg_v01, home_network),
  QMI_IDL_TYPE88(0, 26),

  0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_home_network_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_home_network_resp_msg_v01, home_system_id) - QMI_IDL_OFFSET16RELATIVE(nas_get_home_network_resp_msg_v01, home_system_id_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_home_network_resp_msg_v01, home_system_id),
  QMI_IDL_TYPE88(0, 27),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_home_network_resp_msg_v01, nas_3gpp2_home_network_ext) - QMI_IDL_OFFSET16RELATIVE(nas_get_home_network_resp_msg_v01, nas_3gpp2_home_network_ext_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_home_network_resp_msg_v01, nas_3gpp2_home_network_ext),
  QMI_IDL_TYPE88(0, 36),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_home_network_resp_msg_v01, nas_3gpp_mcs_include_digit) - QMI_IDL_OFFSET16RELATIVE(nas_get_home_network_resp_msg_v01, nas_3gpp_mcs_include_digit_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_home_network_resp_msg_v01, nas_3gpp_mcs_include_digit),
  QMI_IDL_TYPE88(0, 37)
};

/* 
 * nas_get_preferred_networks_req_msg is empty
 * static const uint8_t nas_get_preferred_networks_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_preferred_networks_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_preferred_networks_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_preferred_networks) - QMI_IDL_OFFSET8(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_preferred_networks_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_preferred_networks),
  ((NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01) & 0xFF), ((NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_preferred_networks) - QMI_IDL_OFFSET8(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_preferred_networks_len),
  QMI_IDL_TYPE88(0, 38),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, static_3gpp_preferred_networks) - QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, static_3gpp_preferred_networks_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_preferred_networks_resp_msg_v01, static_3gpp_preferred_networks),
  ((NAS_STATIC_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01) & 0xFF), ((NAS_STATIC_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, static_3gpp_preferred_networks) - QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, static_3gpp_preferred_networks_len),
  QMI_IDL_TYPE88(0, 38),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_mnc_includes_pcs_digit_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_mnc_includes_pcs_digit),
  NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, nas_3gpp_mnc_includes_pcs_digit_len),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, static_3gpp_mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, static_3gpp_mnc_includes_pcs_digit_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_preferred_networks_resp_msg_v01, static_3gpp_mnc_includes_pcs_digit),
  NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, static_3gpp_mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_get_preferred_networks_resp_msg_v01, static_3gpp_mnc_includes_pcs_digit_len),
  QMI_IDL_TYPE88(0, 21)
};

static const uint8_t nas_set_preferred_networks_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_preferred_networks_req_msg_v01, nas_3gpp_preferred_networks) - QMI_IDL_OFFSET8(nas_set_preferred_networks_req_msg_v01, nas_3gpp_preferred_networks_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_preferred_networks_req_msg_v01, nas_3gpp_preferred_networks),
  ((NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01) & 0xFF), ((NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_set_preferred_networks_req_msg_v01, nas_3gpp_preferred_networks) - QMI_IDL_OFFSET8(nas_set_preferred_networks_req_msg_v01, nas_3gpp_preferred_networks_len),
  QMI_IDL_TYPE88(0, 38),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_set_preferred_networks_req_msg_v01, nas_3gpp_mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_set_preferred_networks_req_msg_v01, nas_3gpp_mnc_includes_pcs_digit_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_set_preferred_networks_req_msg_v01, nas_3gpp_mnc_includes_pcs_digit),
  NAS_3GPP_PREFERRED_NETWORKS_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_set_preferred_networks_req_msg_v01, nas_3gpp_mnc_includes_pcs_digit) - QMI_IDL_OFFSET16RELATIVE(nas_set_preferred_networks_req_msg_v01, nas_3gpp_mnc_includes_pcs_digit_len),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_set_preferred_networks_req_msg_v01, clear_prev_preferred_networks) - QMI_IDL_OFFSET16RELATIVE(nas_set_preferred_networks_req_msg_v01, clear_prev_preferred_networks_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_set_preferred_networks_req_msg_v01, clear_prev_preferred_networks)
};

static const uint8_t nas_set_preferred_networks_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_preferred_networks_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_forbidden_networks_req_msg is empty
 * static const uint8_t nas_get_forbidden_networks_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_forbidden_networks_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_forbidden_networks_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_forbidden_networks_resp_msg_v01, nas_3gpp_forbidden_networks) - QMI_IDL_OFFSET8(nas_get_forbidden_networks_resp_msg_v01, nas_3gpp_forbidden_networks_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_forbidden_networks_resp_msg_v01, nas_3gpp_forbidden_networks),
  ((NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01) & 0xFF), ((NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_get_forbidden_networks_resp_msg_v01, nas_3gpp_forbidden_networks) - QMI_IDL_OFFSET8(nas_get_forbidden_networks_resp_msg_v01, nas_3gpp_forbidden_networks_len),
  QMI_IDL_TYPE88(0, 39)
};

static const uint8_t nas_set_forbidden_networks_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_forbidden_networks_req_msg_v01, nas_3gpp_forbidden_networks) - QMI_IDL_OFFSET8(nas_set_forbidden_networks_req_msg_v01, nas_3gpp_forbidden_networks_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_forbidden_networks_req_msg_v01, nas_3gpp_forbidden_networks),
  ((NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01) & 0xFF), ((NAS_3GPP_FORBIDDEN_NETWORKS_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_set_forbidden_networks_req_msg_v01, nas_3gpp_forbidden_networks) - QMI_IDL_OFFSET8(nas_set_forbidden_networks_req_msg_v01, nas_3gpp_forbidden_networks_len),
  QMI_IDL_TYPE88(0, 39)
};

static const uint8_t nas_set_forbidden_networks_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_forbidden_networks_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_accolc_req_msg is empty
 * static const uint8_t nas_get_accolc_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_accolc_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_accolc_resp_msg_v01, accolc),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_accolc_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_set_accolc_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_accolc_req_msg_v01, accolc_set),
  QMI_IDL_TYPE88(0, 40)
};

static const uint8_t nas_set_accolc_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_accolc_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_device_config_req_msg is empty
 * static const uint8_t nas_get_device_config_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_device_config_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, sci) - QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, sci_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, sci),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, scm) - QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, scm_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, scm),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, registration_parameters) - QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, registration_parameters_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, registration_parameters),
  QMI_IDL_TYPE88(0, 41),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, force_rev0) - QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, force_rev0_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, force_rev0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, hdr_scp_config) - QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, hdr_scp_config_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, hdr_scp_config),
  QMI_IDL_TYPE88(0, 42),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, roam_pref) - QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, roam_pref_valid)),
  0x16,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, roam_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, force_hdrscp_config_at) - QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, force_hdrscp_config_at_valid)),
  0x17,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_device_config_resp_msg_v01, force_hdrscp_config_at)
};

static const uint8_t nas_set_device_config_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, spc) - QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, spc_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, spc),
  NAS_SPC_MAX_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, force_hdr_rev0) - QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, force_hdr_rev0_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, force_hdr_rev0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, hdr_scp_config) - QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, hdr_scp_config_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, hdr_scp_config),
  QMI_IDL_TYPE88(0, 42),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, roam_pref) - QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, roam_pref_valid)),
  0x16,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_device_config_req_msg_v01, roam_pref)
};

static const uint8_t nas_set_device_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_device_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_rf_band_info_req_msg is empty
 * static const uint8_t nas_get_rf_band_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_rf_band_info_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_rf_band_info_resp_msg_v01, rf_band_info_list),
  NAS_RF_BAND_INFO_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_get_rf_band_info_resp_msg_v01, rf_band_info_list) - QMI_IDL_OFFSET8(nas_get_rf_band_info_resp_msg_v01, rf_band_info_list_len),
  QMI_IDL_TYPE88(0, 11),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_rf_band_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_an_aaa_status_req_msg is empty
 * static const uint8_t nas_get_an_aaa_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_an_aaa_status_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_an_aaa_status_resp_msg_v01, an_aaa_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_an_aaa_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_set_system_selection_preference_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, emergency_mode) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, emergency_mode_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, emergency_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, mode_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, mode_pref_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, mode_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, band_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, band_pref_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, prl_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, prl_pref_valid)),
  0x13,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, prl_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, roam_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, roam_pref_valid)),
  0x14,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, roam_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, lte_band_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, lte_band_pref_valid)),
  0x15,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, lte_band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, net_sel_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, net_sel_pref_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, net_sel_pref),
  QMI_IDL_TYPE88(0, 43),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, change_duration) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, change_duration_valid)),
  0x17,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, change_duration),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_domain_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_domain_pref_valid)),
  0x18,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_domain_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, gw_acq_order_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, gw_acq_order_pref_valid)),
  0x19,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, gw_acq_order_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, mnc_includes_pcs_digit) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, mnc_includes_pcs_digit_valid)),
  0x1A,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, mnc_includes_pcs_digit),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_domain_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_domain_pref_valid)),
  0x1B,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_domain_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, gw_acq_order_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, gw_acq_order_pref_valid)),
  0x1C,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, gw_acq_order_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, tdscdma_band_pref) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, tdscdma_band_pref_valid)),
  0x1D,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, tdscdma_band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, acq_order) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, acq_order_valid)),
  0x1E,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, acq_order),
  NAS_ACQ_ORDER_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, acq_order) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, acq_order_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_reg_restriction) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_reg_restriction_valid)),
  0x1F,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, srv_reg_restriction),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, csg_info) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, csg_info_valid)),
  0x20,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, csg_info),
  QMI_IDL_TYPE88(0, 44),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, usage_setting) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, usage_setting_valid)),
  0x21,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, usage_setting),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, rat) - QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, rat_valid)),
  0x22,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_req_msg_v01, rat)
};

static const uint8_t nas_set_system_selection_preference_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_system_selection_preference_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_system_selection_preference_req_msg is empty
 * static const uint8_t nas_get_system_selection_preference_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_system_selection_preference_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, emergency_mode) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, emergency_mode_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, emergency_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, mode_pref) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, mode_pref_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, mode_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, band_pref) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, band_pref_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, prl_pref) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, prl_pref_valid)),
  0x13,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, prl_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, roam_pref) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, roam_pref_valid)),
  0x14,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, roam_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, band_pref_ext) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, band_pref_ext_valid)),
  0x15,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, band_pref_ext),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, net_sel_pref) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, net_sel_pref_valid)),
  0x16,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, net_sel_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, srv_domain_pref) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, srv_domain_pref_valid)),
  0x18,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, srv_domain_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, gw_acq_order_pref) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, gw_acq_order_pref_valid)),
  0x19,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, gw_acq_order_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, tdscdma_band_pref) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, tdscdma_band_pref_valid)),
  0x1A,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, tdscdma_band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, manual_net_sel_plmn) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, manual_net_sel_plmn_valid)),
  0x1B,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, manual_net_sel_plmn),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, acq_order) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, acq_order_valid)),
  0x1C,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, acq_order),
  NAS_ACQ_ORDER_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, acq_order) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, acq_order_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, srv_reg_restriction) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, srv_reg_restriction_valid)),
  0x1D,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, srv_reg_restriction),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, csg_info) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, csg_info_valid)),
  0x1E,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, csg_info),
  QMI_IDL_TYPE88(0, 44),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, usage_setting) - QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, usage_setting_valid)),
  0x1F,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_system_selection_preference_resp_msg_v01, usage_setting)
};

static const uint8_t nas_system_selection_preference_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, emergency_mode) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, emergency_mode_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, emergency_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, mode_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, mode_pref_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, mode_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, band_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, band_pref_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, prl_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, prl_pref_valid)),
  0x13,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, prl_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, roam_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, roam_pref_valid)),
  0x14,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, roam_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, lte_band_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, lte_band_pref_valid)),
  0x15,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, lte_band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, net_sel_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, net_sel_pref_valid)),
  0x16,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, net_sel_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, srv_domain_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, srv_domain_pref_valid)),
  0x18,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, srv_domain_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, gw_acq_order_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, gw_acq_order_pref_valid)),
  0x19,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, gw_acq_order_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, tdscdma_band_pref) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, tdscdma_band_pref_valid)),
  0x1A,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, tdscdma_band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, manual_net_sel_plmn) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, manual_net_sel_plmn_valid)),
  0x1B,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, manual_net_sel_plmn),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, acq_order) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, acq_order_valid)),
  0x1C,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, acq_order),
  NAS_ACQ_ORDER_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, acq_order) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, acq_order_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, srv_reg_restriction) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, srv_reg_restriction_valid)),
  0x1D,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, srv_reg_restriction),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, csg_info) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, csg_info_valid)),
  0x1E,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, csg_info),
  QMI_IDL_TYPE88(0, 44),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, usage_setting) - QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, usage_setting_valid)),
  0x1F,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_system_selection_preference_ind_msg_v01, usage_setting)
};

static const uint8_t nas_set_ddtm_preference_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_ddtm_preference_req_msg_v01, ddtm_preference),
  QMI_IDL_TYPE88(0, 45)
};

static const uint8_t nas_set_ddtm_preference_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_ddtm_preference_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_ddtm_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_ddtm_ind_msg_v01, ddtm_settings),
  QMI_IDL_TYPE88(0, 46)
};

/* 
 * nas_get_operator_name_data_req_msg is empty
 * static const uint8_t nas_get_operator_name_data_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_operator_name_data_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, service_provider_name) - QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, service_provider_name_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, service_provider_name),
  QMI_IDL_TYPE88(0, 47),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, operator_plmn_list) - QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, operator_plmn_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, operator_plmn_list),
  ((NAS_OPERATOR_PLMN_LIST_MAX_V01) & 0xFF), ((NAS_OPERATOR_PLMN_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, operator_plmn_list) - QMI_IDL_OFFSET8(nas_get_operator_name_data_resp_msg_v01, operator_plmn_list_len),
  QMI_IDL_TYPE88(0, 48),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_operator_name_data_resp_msg_v01, plmn_network_name) - QMI_IDL_OFFSET16RELATIVE(nas_get_operator_name_data_resp_msg_v01, plmn_network_name_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_operator_name_data_resp_msg_v01, plmn_network_name),
  NAS_PLMN_NETWORK_NAME_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_operator_name_data_resp_msg_v01, plmn_network_name) - QMI_IDL_OFFSET16RELATIVE(nas_get_operator_name_data_resp_msg_v01, plmn_network_name_len),
  QMI_IDL_TYPE88(0, 49),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_operator_name_data_resp_msg_v01, plmn_name) - QMI_IDL_OFFSET16RELATIVE(nas_get_operator_name_data_resp_msg_v01, plmn_name_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(nas_get_operator_name_data_resp_msg_v01, plmn_name),
  NAS_PLMN_NAME_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_operator_name_data_resp_msg_v01, nitz_information) - QMI_IDL_OFFSET16RELATIVE(nas_get_operator_name_data_resp_msg_v01, nitz_information_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_operator_name_data_resp_msg_v01, nitz_information),
  QMI_IDL_TYPE88(0, 49)
};

static const uint8_t nas_operator_name_data_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_operator_name_data_ind_msg_v01, service_provider_name) - QMI_IDL_OFFSET8(nas_operator_name_data_ind_msg_v01, service_provider_name_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_operator_name_data_ind_msg_v01, service_provider_name),
  QMI_IDL_TYPE88(0, 47),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_operator_name_data_ind_msg_v01, operator_plmn_list) - QMI_IDL_OFFSET8(nas_operator_name_data_ind_msg_v01, operator_plmn_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_operator_name_data_ind_msg_v01, operator_plmn_list),
  ((NAS_OPERATOR_PLMN_LIST_MAX_V01) & 0xFF), ((NAS_OPERATOR_PLMN_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_operator_name_data_ind_msg_v01, operator_plmn_list) - QMI_IDL_OFFSET8(nas_operator_name_data_ind_msg_v01, operator_plmn_list_len),
  QMI_IDL_TYPE88(0, 48),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_operator_name_data_ind_msg_v01, plmn_network_name) - QMI_IDL_OFFSET16RELATIVE(nas_operator_name_data_ind_msg_v01, plmn_network_name_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_operator_name_data_ind_msg_v01, plmn_network_name),
  NAS_PLMN_NETWORK_NAME_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_operator_name_data_ind_msg_v01, plmn_network_name) - QMI_IDL_OFFSET16RELATIVE(nas_operator_name_data_ind_msg_v01, plmn_network_name_len),
  QMI_IDL_TYPE88(0, 49),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_operator_name_data_ind_msg_v01, plmn_name) - QMI_IDL_OFFSET16RELATIVE(nas_operator_name_data_ind_msg_v01, plmn_name_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(nas_operator_name_data_ind_msg_v01, plmn_name),
  NAS_PLMN_NAME_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_operator_name_data_ind_msg_v01, nitz_information) - QMI_IDL_OFFSET16RELATIVE(nas_operator_name_data_ind_msg_v01, nitz_information_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_operator_name_data_ind_msg_v01, nitz_information),
  QMI_IDL_TYPE88(0, 49)
};

/* 
 * nas_get_csp_plmn_mode_bit_req_msg is empty
 * static const uint8_t nas_get_csp_plmn_mode_bit_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_csp_plmn_mode_bit_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_csp_plmn_mode_bit_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_csp_plmn_mode_bit_resp_msg_v01, plmn_mode) - QMI_IDL_OFFSET8(nas_get_csp_plmn_mode_bit_resp_msg_v01, plmn_mode_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_csp_plmn_mode_bit_resp_msg_v01, plmn_mode)
};

static const uint8_t nas_csp_plmn_mode_bit_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_csp_plmn_mode_bit_ind_msg_v01, plmn_mode) - QMI_IDL_OFFSET8(nas_csp_plmn_mode_bit_ind_msg_v01, plmn_mode_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_csp_plmn_mode_bit_ind_msg_v01, plmn_mode)
};

static const uint8_t nas_update_akey_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_update_akey_req_msg_v01, akey),
  26
};

static const uint8_t nas_update_akey_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_update_akey_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_get_3gpp2_subscription_info_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_req_msg_v01, nam_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_req_msg_v01, get_3gpp2_info_mask) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_req_msg_v01, get_3gpp2_info_mask_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_req_msg_v01, get_3gpp2_info_mask)
};

static const uint8_t nas_get_3gpp2_subscription_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, nam_name) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, nam_name_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, nam_name),
  NAS_MAX_NAM_NAME_LEN_V01,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, nam_name) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, nam_name_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, dir_num) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, dir_num_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, dir_num),
  NAS_MAX_3GPP2_SUBS_INFO_DIR_NUM_LEN_V01,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, dir_num) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, dir_num_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, cdma_sys_id) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, cdma_sys_id_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, cdma_sys_id),
  NAS_MAX_3GPP2_HOME_SID_NID_NUM_V01,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, cdma_sys_id) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, cdma_sys_id_len),
  QMI_IDL_TYPE88(0, 50),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, min_based_info) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, min_based_info_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, min_based_info),
  QMI_IDL_TYPE88(0, 51),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, true_imsi) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, true_imsi_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, true_imsi),
  QMI_IDL_TYPE88(0, 52),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, cdma_channel_info) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, cdma_channel_info_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, cdma_channel_info),
  QMI_IDL_TYPE88(0, 53),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, mdn) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, mdn_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, mdn),
  MDN_MAX_LEN_V01,
  QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, mdn) - QMI_IDL_OFFSET8(nas_get_3gpp2_subscription_info_resp_msg_v01, mdn_len)
};

static const uint8_t nas_set_3gpp2_subscription_info_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, nam_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, dir_num) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, dir_num_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, dir_num),
  NAS_MAX_3GPP2_SUBS_INFO_DIR_NUM_LEN_V01,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, dir_num) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, dir_num_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, cdma_sys_id) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, cdma_sys_id_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, cdma_sys_id),
  NAS_MAX_3GPP2_HOME_SID_NID_NUM_V01,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, cdma_sys_id) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, cdma_sys_id_len),
  QMI_IDL_TYPE88(0, 50),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, min_based_info) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, min_based_info_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, min_based_info),
  QMI_IDL_TYPE88(0, 51),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, true_imsi) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, true_imsi_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, true_imsi),
  QMI_IDL_TYPE88(0, 52),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, cdma_channel_info) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, cdma_channel_info_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, cdma_channel_info),
  QMI_IDL_TYPE88(0, 53),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, nam_name) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, nam_name_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, nam_name),
  NAS_MAX_NAM_NAME_LEN_V01,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, nam_name) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, nam_name_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, mdn) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, mdn_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, mdn),
  MDN_MAX_LEN_V01,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, mdn) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, mdn_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, spc) - QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, spc_valid)),
  0x17,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_req_msg_v01, spc),
  NAS_SPC_MAX_V01
};

static const uint8_t nas_set_3gpp2_subscription_info_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_3gpp2_subscription_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_mob_cai_rev_req is empty
 * static const uint8_t nas_get_mob_cai_rev_req_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_mob_cai_rev_resp_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_mob_cai_rev_resp_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_mob_cai_rev_resp_v01, cai_rev) - QMI_IDL_OFFSET8(nas_get_mob_cai_rev_resp_v01, cai_rev_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_mob_cai_rev_resp_v01, cai_rev)
};

/* 
 * nas_get_rtre_config_req is empty
 * static const uint8_t nas_get_rtre_config_req_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_rtre_config_resp_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_rtre_config_resp_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_rtre_config_resp_v01, rtre_cfg) - QMI_IDL_OFFSET8(nas_get_rtre_config_resp_v01, rtre_cfg_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_rtre_config_resp_v01, rtre_cfg),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_rtre_config_resp_v01, rtre_cfg_pref) - QMI_IDL_OFFSET8(nas_get_rtre_config_resp_v01, rtre_cfg_pref_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_rtre_config_resp_v01, rtre_cfg_pref)
};

static const uint8_t nas_set_rtre_config_req_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_rtre_config_req_v01, rtre_cfg_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_rtre_config_req_v01, spc) - QMI_IDL_OFFSET8(nas_set_rtre_config_req_v01, spc_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_set_rtre_config_req_v01, spc),
  NAS_SPC_MAX_V01
};

static const uint8_t nas_set_rtre_config_resp_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_rtre_config_resp_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_cell_location_info_req_msg is empty
 * static const uint8_t nas_get_cell_location_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_cell_location_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_cell_location_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_cell_location_info_resp_msg_v01, geran_info) - QMI_IDL_OFFSET8(nas_get_cell_location_info_resp_msg_v01, geran_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_cell_location_info_resp_msg_v01, geran_info),
  QMI_IDL_TYPE88(0, 55),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_cell_location_info_resp_msg_v01, umts_info) - QMI_IDL_OFFSET8(nas_get_cell_location_info_resp_msg_v01, umts_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_cell_location_info_resp_msg_v01, umts_info),
  QMI_IDL_TYPE88(0, 59),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, cdma_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, cdma_info_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, cdma_info),
  QMI_IDL_TYPE88(0, 60),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, lte_intra) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, lte_intra_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, lte_intra),
  QMI_IDL_TYPE88(0, 62),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, lte_inter) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, lte_inter_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, lte_inter),
  QMI_IDL_TYPE88(0, 64),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, lte_gsm) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, lte_gsm_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, lte_gsm),
  QMI_IDL_TYPE88(0, 67),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, lte_wcdma) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, lte_wcdma_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, lte_wcdma),
  QMI_IDL_TYPE88(0, 70),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, umts_cell_id) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, umts_cell_id_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, umts_cell_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_lte) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_lte_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, wcdma_lte),
  QMI_IDL_TYPE88(0, 71),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, cdma_rx_power) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, cdma_rx_power_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, cdma_rx_power),
  QMI_IDL_TYPE88(0, 72),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, hdr_rx_power) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, hdr_rx_power_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, hdr_rx_power),
  QMI_IDL_TYPE88(0, 72),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, gsm_info_ext) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, gsm_info_ext_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, gsm_info_ext),
  QMI_IDL_TYPE88(0, 73),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_info_ext) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_info_ext_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, wcdma_info_ext),
  QMI_IDL_TYPE88(0, 74),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, gncell_bcch) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, gncell_bcch_valid)),
  0x1D,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, gncell_bcch),
  NAS_UMTS_GERAN_MAX_NBR_CELL_SET_NUM_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, gncell_bcch) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, gncell_bcch_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, timing_advance) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, timing_advance_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, timing_advance),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, cells) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, cells_valid)),
  0x1F,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, cells),
  NAS_UMTS_MAX_ACTIVE_CELL_SET_NUM_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, cells) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, cells_len),
  QMI_IDL_TYPE88(0, 75),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_active_set_reference_rl) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_active_set_reference_rl_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, wcdma_active_set_reference_rl),
  QMI_IDL_TYPE88(0, 76),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, geran_info_ext) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, geran_info_ext_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, geran_info_ext),
  QMI_IDL_TYPE88(0, 142),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, umts_ext_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, umts_ext_info_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, umts_ext_info),
  QMI_IDL_TYPE88(0, 139),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_active_set_cells) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_active_set_cells_valid)),
  0x23,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, wcdma_active_set_cells),
  NAS_UMTS_MAX_ACTIVE_CELL_SET_NUM_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_active_set_cells) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, wcdma_active_set_cells_len),
  QMI_IDL_TYPE88(0, 140),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, scell_geran_config) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, scell_geran_config_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, scell_geran_config),
  QMI_IDL_TYPE88(0, 143),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, current_l1_ts) - QMI_IDL_OFFSET16RELATIVE(nas_get_cell_location_info_resp_msg_v01, current_l1_ts_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_cell_location_info_resp_msg_v01, current_l1_ts)
};

static const uint8_t nas_get_plmn_name_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, plmn),
  QMI_IDL_TYPE88(0, 35),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, suppress_sim_error) - QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, suppress_sim_error_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, suppress_sim_error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, mnc_includes_pcs_digit) - QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, mnc_includes_pcs_digit_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, mnc_includes_pcs_digit),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, always_send_plmn_name) - QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, always_send_plmn_name_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, always_send_plmn_name),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, use_static_table_only) - QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, use_static_table_only_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, use_static_table_only),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, csg_id) - QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, csg_id_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, csg_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, rat) - QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, rat_valid)),
  0x15,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, rat),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, send_all_information) - QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, send_all_information_valid)),
  0x16,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_req_msg_v01, send_all_information)
};

static const uint8_t nas_get_plmn_name_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_plmn_name_resp_msg_v01, eons_plmn_name_3gpp) - QMI_IDL_OFFSET8(nas_get_plmn_name_resp_msg_v01, eons_plmn_name_3gpp_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_plmn_name_resp_msg_v01, eons_plmn_name_3gpp),
  QMI_IDL_TYPE88(0, 77),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_plmn_name_resp_msg_v01, eons_display_bit_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_plmn_name_resp_msg_v01, eons_display_bit_info_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_plmn_name_resp_msg_v01, eons_display_bit_info),
  QMI_IDL_TYPE88(0, 78),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_plmn_name_resp_msg_v01, is_home_network) - QMI_IDL_OFFSET16RELATIVE(nas_get_plmn_name_resp_msg_v01, is_home_network_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_plmn_name_resp_msg_v01, is_home_network),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_plmn_name_resp_msg_v01, lang_plmn_names) - QMI_IDL_OFFSET16RELATIVE(nas_get_plmn_name_resp_msg_v01, lang_plmn_names_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_plmn_name_resp_msg_v01, lang_plmn_names),
  NAS_ALT_LANG_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_plmn_name_resp_msg_v01, lang_plmn_names) - QMI_IDL_OFFSET16RELATIVE(nas_get_plmn_name_resp_msg_v01, lang_plmn_names_len),
  QMI_IDL_TYPE88(0, 135)
};

static const uint8_t nas_bind_subscription_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_bind_subscription_req_msg_v01, subs_type)
};

static const uint8_t nas_bind_subscription_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_set_dual_standby_pref_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, standby_pref) - QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, standby_pref_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, standby_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, priority_subs) - QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, priority_subs_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, priority_subs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, default_data_subs) - QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, default_data_subs_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, default_data_subs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, default_voice_subs) - QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, default_voice_subs_valid)),
  0x13,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, default_voice_subs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, active_subs_mask) - QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, active_subs_mask_valid)),
  0x14,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_set_dual_standby_pref_req_msg_v01, active_subs_mask)
};

static const uint8_t nas_set_dual_standby_pref_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_dual_standby_pref_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_dual_standby_pref_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, standby_pref) - QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, standby_pref_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, standby_pref),
  QMI_IDL_TYPE88(0, 79),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, default_voice_subs) - QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, default_voice_subs_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, default_voice_subs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, active_subs_mask) - QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, active_subs_mask_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_dual_standby_pref_ind_msg_v01, active_subs_mask)
};

static const uint8_t nas_subscription_info_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_priority_subs) - QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_priority_subs_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_priority_subs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_active) - QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_active_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_active),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_default_data_subs) - QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_default_data_subs_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, is_default_data_subs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, voice_system_id) - QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, voice_system_id_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_subscription_info_ind_msg_v01, voice_system_id)
};

/* 
 * nas_get_mode_pref_req_msg is empty
 * static const uint8_t nas_get_mode_pref_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_mode_pref_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx0_mode_pref) - QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx0_mode_pref_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx0_mode_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx1_mode_pref) - QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx1_mode_pref_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx1_mode_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx2_mode_pref) - QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx2_mode_pref_valid)),
  0x12,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_mode_pref_resp_msg_v01, idx2_mode_pref)
};

static const uint8_t nas_set_technology_preference_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_technology_preference_req_msg_v01, technology_pref),
  QMI_IDL_TYPE88(0, 80)
};

static const uint8_t nas_set_technology_preference_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_technology_preference_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_technology_preference_req_type is empty
 * static const uint8_t nas_get_technology_preference_req_type_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_technology_preference_resp_type_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_technology_preference_resp_type_v01, active_technology_pref),
  QMI_IDL_TYPE88(0, 81),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_technology_preference_resp_type_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_technology_preference_resp_type_v01, persistent_technology_pref) - QMI_IDL_OFFSET8(nas_get_technology_preference_resp_type_v01, persistent_technology_pref_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_technology_preference_resp_type_v01, persistent_technology_pref)
};

/* 
 * nas_get_network_system_preference_req is empty
 * static const uint8_t nas_get_network_system_preference_req_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_network_system_preference_resp_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_network_system_preference_resp_v01, system_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_network_system_preference_resp_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_network_time_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, universal_time),
  QMI_IDL_TYPE88(0, 82),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, time_zone) - QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, time_zone_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, time_zone),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, daylt_sav_adj) - QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, daylt_sav_adj_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, daylt_sav_adj),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, radio_if) - QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, radio_if_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_network_time_ind_msg_v01, radio_if)
};

/* 
 * nas_get_sys_info_req_msg is empty
 * static const uint8_t nas_get_sys_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_sys_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, cdma_srv_status_info) - QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, cdma_srv_status_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, cdma_srv_status_info),
  QMI_IDL_TYPE88(0, 84),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, hdr_srv_status_info) - QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, hdr_srv_status_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, hdr_srv_status_info),
  QMI_IDL_TYPE88(0, 84),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, gsm_srv_status_info) - QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, gsm_srv_status_info_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, gsm_srv_status_info),
  QMI_IDL_TYPE88(0, 85),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, wcdma_srv_status_info) - QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, wcdma_srv_status_info_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, wcdma_srv_status_info),
  QMI_IDL_TYPE88(0, 85),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, lte_srv_status_info) - QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, lte_srv_status_info_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, lte_srv_status_info),
  QMI_IDL_TYPE88(0, 85),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, cdma_sys_info) - QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, cdma_sys_info_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, cdma_sys_info),
  QMI_IDL_TYPE88(0, 95),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, hdr_sys_info) - QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, hdr_sys_info_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_resp_msg_v01, hdr_sys_info),
  QMI_IDL_TYPE88(0, 96),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_sys_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_sys_info_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, gsm_sys_info),
  QMI_IDL_TYPE88(0, 97),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_sys_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_sys_info_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_sys_info),
  QMI_IDL_TYPE88(0, 98),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_sys_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_sys_info_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_sys_info),
  QMI_IDL_TYPE88(0, 99),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_sys_info2_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, cdma_sys_info2),
  QMI_IDL_TYPE88(0, 102),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, hdr_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, hdr_sys_info2_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, hdr_sys_info2),
  QMI_IDL_TYPE88(0, 103),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_sys_info2_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, gsm_sys_info2),
  QMI_IDL_TYPE88(0, 104),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_sys_info2_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_sys_info2),
  QMI_IDL_TYPE88(0, 105),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_sys_info2_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_sys_info2),
  QMI_IDL_TYPE88(0, 106),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_sys_info3) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_sys_info3_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, gsm_sys_info3),
  QMI_IDL_TYPE88(0, 33),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_sys_info3) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_sys_info3_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_sys_info3),
  QMI_IDL_TYPE88(0, 33),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, voice_support_on_lte) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, voice_support_on_lte_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, voice_support_on_lte),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_cipher_domain) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_cipher_domain_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, gsm_cipher_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_cipher_domain) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_cipher_domain_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_cipher_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_srv_status_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_srv_status_info_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, tdscdma_srv_status_info),
  QMI_IDL_TYPE88(0, 85),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_sys_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_sys_info_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, tdscdma_sys_info),
  QMI_IDL_TYPE88(0, 101),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_embms_coverage) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_embms_coverage_valid)),
  0x26,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_embms_coverage),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, sim_rej_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, sim_rej_info_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, sim_rej_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_eutra_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_eutra_status_valid)),
  0x28,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_eutra_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_ims_voice_avail) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_ims_voice_avail_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_ims_voice_avail),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_voice_status_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_reg_zone) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_reg_zone_valid)),
  0x2B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, cdma_reg_zone),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_rac) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_rac_valid)),
  0x2C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, gsm_rac),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_rac) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_rac_valid)),
  0x2D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_rac),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_mcc_resolved_via_sid_lookup) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_mcc_resolved_via_sid_lookup_valid)),
  0x2E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, cdma_mcc_resolved_via_sid_lookup),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, srv_reg_restriction) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, srv_reg_restriction_valid)),
  0x2F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, srv_reg_restriction),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_reg_domain) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_reg_domain_valid)),
  0x30,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, tdscdma_reg_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_reg_domain) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_reg_domain_valid)),
  0x31,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_reg_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_reg_domain) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_reg_domain_valid)),
  0x32,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_reg_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_reg_domain) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_reg_domain_valid)),
  0x33,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, gsm_reg_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_embms_coverage_trace_id) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_embms_coverage_trace_id_valid)),
  0x34,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_embms_coverage_trace_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_csg_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_csg_info_valid)),
  0x35,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_csg_info),
  QMI_IDL_TYPE88(0, 22),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, hdr_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, hdr_voice_status_valid)),
  0x36,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, hdr_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, hdr_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, hdr_sms_status_valid)),
  0x37,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, hdr_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_sms_status_valid)),
  0x38,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_is_eb_supported) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_is_eb_supported_valid)),
  0x39,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_is_eb_supported),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_voice_status_valid)),
  0x3A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, gsm_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, gsm_sms_status_valid)),
  0x3B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, gsm_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_voice_status_valid)),
  0x3C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, wcdma_sms_status_valid)),
  0x3D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, wcdma_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, emergency_access_barred) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, emergency_access_barred_valid)),
  0x3E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, emergency_access_barred),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_voice_status_valid)),
  0x3F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, cdma_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, cdma_sms_status_valid)),
  0x40,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, cdma_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_voice_status_valid)),
  0x41,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, tdscdma_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, tdscdma_sms_status_valid)),
  0x42,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, tdscdma_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_csg_info) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_csg_info_valid)),
  0x43,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_csg_info),
  QMI_IDL_TYPE88(0, 22),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_cell_status) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, lte_cell_status_valid)),
  0x44,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, lte_cell_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, hdr_subnet_mask_len) - QMI_IDL_OFFSET16RELATIVE(nas_get_sys_info_resp_msg_v01, hdr_subnet_mask_len_valid)),
  0x45,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_get_sys_info_resp_msg_v01, hdr_subnet_mask_len)
};

static const uint8_t nas_sys_info_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, cdma_srv_status_info) - QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, cdma_srv_status_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, cdma_srv_status_info),
  QMI_IDL_TYPE88(0, 84),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, hdr_srv_status_info) - QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, hdr_srv_status_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, hdr_srv_status_info),
  QMI_IDL_TYPE88(0, 84),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, gsm_srv_status_info) - QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, gsm_srv_status_info_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, gsm_srv_status_info),
  QMI_IDL_TYPE88(0, 85),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, wcdma_srv_status_info) - QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, wcdma_srv_status_info_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, wcdma_srv_status_info),
  QMI_IDL_TYPE88(0, 85),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, lte_srv_status_info) - QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, lte_srv_status_info_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, lte_srv_status_info),
  QMI_IDL_TYPE88(0, 85),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, cdma_sys_info) - QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, cdma_sys_info_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, cdma_sys_info),
  QMI_IDL_TYPE88(0, 95),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, hdr_sys_info) - QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, hdr_sys_info_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sys_info_ind_msg_v01, hdr_sys_info),
  QMI_IDL_TYPE88(0, 96),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_sys_info) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_sys_info_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, gsm_sys_info),
  QMI_IDL_TYPE88(0, 97),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_sys_info) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_sys_info_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_sys_info),
  QMI_IDL_TYPE88(0, 98),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_sys_info) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_sys_info_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_sys_info),
  QMI_IDL_TYPE88(0, 99),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_sys_info2_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, cdma_sys_info2),
  QMI_IDL_TYPE88(0, 102),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, hdr_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, hdr_sys_info2_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, hdr_sys_info2),
  QMI_IDL_TYPE88(0, 103),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_sys_info2_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, gsm_sys_info2),
  QMI_IDL_TYPE88(0, 104),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_sys_info2_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_sys_info2),
  QMI_IDL_TYPE88(0, 105),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_sys_info2) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_sys_info2_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_sys_info2),
  QMI_IDL_TYPE88(0, 106),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_sys_info3) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_sys_info3_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, gsm_sys_info3),
  QMI_IDL_TYPE88(0, 33),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_sys_info3) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_sys_info3_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_sys_info3),
  QMI_IDL_TYPE88(0, 33),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, voice_support_on_lte) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, voice_support_on_lte_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, voice_support_on_lte),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_cipher_domain) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_cipher_domain_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, gsm_cipher_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_cipher_domain) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_cipher_domain_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_cipher_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, sys_info_no_change) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, sys_info_no_change_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, sys_info_no_change),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_srv_status_info) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_srv_status_info_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, tdscdma_srv_status_info),
  QMI_IDL_TYPE88(0, 85),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_sys_info) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_sys_info_valid)),
  0x26,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, tdscdma_sys_info),
  QMI_IDL_TYPE88(0, 101),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_embms_coverage) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_embms_coverage_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_embms_coverage),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, sim_rej_info) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, sim_rej_info_valid)),
  0x28,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, sim_rej_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_eutra_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_eutra_status_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_eutra_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_ims_voice_avail) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_ims_voice_avail_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_ims_voice_avail),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_voice_status_valid)),
  0x2B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_reg_zone) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_reg_zone_valid)),
  0x2C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, cdma_reg_zone),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_rac) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_rac_valid)),
  0x2D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, gsm_rac),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_rac) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_rac_valid)),
  0x2E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_rac),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_mcc_resolved_via_sid_lookup) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_mcc_resolved_via_sid_lookup_valid)),
  0x2F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, cdma_mcc_resolved_via_sid_lookup),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, srv_reg_restriction) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, srv_reg_restriction_valid)),
  0x30,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, srv_reg_restriction),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_reg_domain) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_reg_domain_valid)),
  0x31,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, tdscdma_reg_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_reg_domain) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_reg_domain_valid)),
  0x32,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_reg_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_reg_domain) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_reg_domain_valid)),
  0x33,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_reg_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_reg_domain) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_reg_domain_valid)),
  0x34,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, gsm_reg_domain),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_embms_coverage_trace_id) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_embms_coverage_trace_id_valid)),
  0x35,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_embms_coverage_trace_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_csg_info) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_csg_info_valid)),
  0x36,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_csg_info),
  QMI_IDL_TYPE88(0, 22),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, hdr_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, hdr_voice_status_valid)),
  0x37,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, hdr_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, hdr_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, hdr_sms_status_valid)),
  0x38,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, hdr_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_sms_status_valid)),
  0x39,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_is_eb_supported) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_is_eb_supported_valid)),
  0x3A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_is_eb_supported),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_voice_status_valid)),
  0x3B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, gsm_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, gsm_sms_status_valid)),
  0x3C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, gsm_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_voice_status_valid)),
  0x3D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, wcdma_sms_status_valid)),
  0x3E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, wcdma_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, emergency_access_barred) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, emergency_access_barred_valid)),
  0x3F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, emergency_access_barred),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_voice_status_valid)),
  0x40,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, cdma_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, cdma_sms_status_valid)),
  0x41,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, cdma_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_voice_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_voice_status_valid)),
  0x42,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, tdscdma_voice_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_sms_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, tdscdma_sms_status_valid)),
  0x43,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, tdscdma_sms_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_csg_info) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_csg_info_valid)),
  0x44,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_csg_info),
  QMI_IDL_TYPE88(0, 22),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_cell_status) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, lte_cell_status_valid)),
  0x45,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, lte_cell_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, hdr_subnet_mask_len) - QMI_IDL_OFFSET16RELATIVE(nas_sys_info_ind_msg_v01, hdr_subnet_mask_len_valid)),
  0x46,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_sys_info_ind_msg_v01, hdr_subnet_mask_len)
};

/* 
 * nas_get_sig_info_req_msg is empty
 * static const uint8_t nas_get_sig_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_sig_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, cdma_sig_info) - QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, cdma_sig_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, cdma_sig_info),
  QMI_IDL_TYPE88(0, 107),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, hdr_sig_info) - QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, hdr_sig_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, hdr_sig_info),
  QMI_IDL_TYPE88(0, 108),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, gsm_sig_info) - QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, gsm_sig_info_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, gsm_sig_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, wcdma_sig_info) - QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, wcdma_sig_info_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, wcdma_sig_info),
  QMI_IDL_TYPE88(0, 107),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, lte_sig_info) - QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, lte_sig_info_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, lte_sig_info),
  QMI_IDL_TYPE88(0, 109),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, rscp) - QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, rscp_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, rscp),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, tdscdma_sig_info) - QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, tdscdma_sig_info_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sig_info_resp_msg_v01, tdscdma_sig_info),
  QMI_IDL_TYPE88(0, 110)
};

static const uint8_t nas_config_sig_info_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, rssi_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, rssi_threshold_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, rssi_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, rssi_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, rssi_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, ecio_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, ecio_threshold_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, ecio_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, ecio_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, ecio_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, hdr_sinr_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, hdr_sinr_threshold_list_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, hdr_sinr_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, hdr_sinr_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, hdr_sinr_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_snr_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_snr_threshold_list_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_snr_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_snr_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_snr_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, io_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, io_threshold_list_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, io_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, io_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, io_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrq_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrq_threshold_list_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrq_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrq_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrq_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrp_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrp_threshold_list_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrp_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrp_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info_req_msg_v01, lte_rsrp_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, lte_sig_rpt_config) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, lte_sig_rpt_config_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info_req_msg_v01, lte_sig_rpt_config),
  QMI_IDL_TYPE88(0, 111),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, rscp_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, rscp_threshold_list_valid)),
  0x18,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info_req_msg_v01, rscp_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, rscp_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, rscp_threshold_list_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, tds_sinr_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, tds_sinr_threshold_list_valid)),
  0x19,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info_req_msg_v01, tds_sinr_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, tds_sinr_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info_req_msg_v01, tds_sinr_threshold_list_len)
};

static const uint8_t nas_config_sig_info_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_config_sig_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_sig_info_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, cdma_sig_info) - QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, cdma_sig_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, cdma_sig_info),
  QMI_IDL_TYPE88(0, 107),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, hdr_sig_info) - QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, hdr_sig_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, hdr_sig_info),
  QMI_IDL_TYPE88(0, 108),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, gsm_sig_info) - QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, gsm_sig_info_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, gsm_sig_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, wcdma_sig_info) - QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, wcdma_sig_info_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, wcdma_sig_info),
  QMI_IDL_TYPE88(0, 107),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, lte_sig_info) - QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, lte_sig_info_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, lte_sig_info),
  QMI_IDL_TYPE88(0, 109),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, rscp) - QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, rscp_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, rscp),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, tdscdma_sig_info) - QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, tdscdma_sig_info_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_sig_info_ind_msg_v01, tdscdma_sig_info),
  QMI_IDL_TYPE88(0, 110)
};

/* 
 * nas_get_err_rate_req_msg is empty
 * static const uint8_t nas_get_err_rate_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_err_rate_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, cdma_frame_err_rate) - QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, cdma_frame_err_rate_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, cdma_frame_err_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, hdr_packet_err_rate) - QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, hdr_packet_err_rate_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, hdr_packet_err_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, gsm_bit_err_rate) - QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, gsm_bit_err_rate_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, gsm_bit_err_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, wcdma_block_err_rate) - QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, wcdma_block_err_rate_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, wcdma_block_err_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, tdscdma_block_err_rate) - QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, tdscdma_block_err_rate_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_err_rate_resp_msg_v01, tdscdma_block_err_rate)
};

static const uint8_t nas_err_rate_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, cdma_frame_err_rate) - QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, cdma_frame_err_rate_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, cdma_frame_err_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, hdr_packet_err_rate) - QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, hdr_packet_err_rate_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, hdr_packet_err_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, gsm_bit_err_rate) - QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, gsm_bit_err_rate_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, gsm_bit_err_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, wcdma_block_err_rate) - QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, wcdma_block_err_rate_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, wcdma_block_err_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, tdscdma_block_err_rate) - QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, tdscdma_block_err_rate_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_err_rate_ind_msg_v01, tdscdma_block_err_rate)
};

static const uint8_t nas_hdr_session_close_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_session_close_ind_msg_v01, close_reason)
};

static const uint8_t nas_hdr_uati_update_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_hdr_uati_update_ind_msg_v01, uati),
  QMI_NAS_UATI_LENGTH_V01
};

static const uint8_t nas_get_hdr_subtype_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_hdr_subtype_req_msg_v01, protocol)
};

static const uint8_t nas_get_hdr_subtype_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_hdr_subtype_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_hdr_subtype_resp_msg_v01, subtype) - QMI_IDL_OFFSET8(nas_get_hdr_subtype_resp_msg_v01, subtype_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_hdr_subtype_resp_msg_v01, subtype)
};

/* 
 * nas_get_hdr_color_code_req_msg is empty
 * static const uint8_t nas_get_hdr_color_code_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_hdr_color_code_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_hdr_color_code_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_hdr_color_code_resp_msg_v01, color_code) - QMI_IDL_OFFSET8(nas_get_hdr_color_code_resp_msg_v01, color_code_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_hdr_color_code_resp_msg_v01, color_code)
};

/* 
 * nas_get_current_acq_sys_mode_req_msg is empty
 * static const uint8_t nas_get_current_acq_sys_mode_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_current_acq_sys_mode_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, cdma) - QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, cdma_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, cdma),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, cdma_evdo) - QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, cdma_evdo_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, cdma_evdo),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, gsm) - QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, gsm_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, gsm),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, umts) - QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, umts_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, umts),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, lte) - QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, lte_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, lte),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, tdscdma) - QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, tdscdma_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_current_acq_sys_mode_resp_msg_v01, tdscdma)
};

static const uint8_t nas_set_rx_diversity_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_rx_diversity_req_msg_v01, req_param),
  QMI_IDL_TYPE88(0, 112)
};

static const uint8_t nas_set_rx_diversity_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_rx_diversity_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_get_tx_rx_info_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_tx_rx_info_req_msg_v01, radio_if)
};

static const uint8_t nas_get_tx_rx_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, rx_chain_0) - QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, rx_chain_0_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, rx_chain_0),
  QMI_IDL_TYPE88(0, 113),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, rx_chain_1) - QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, rx_chain_1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, rx_chain_1),
  QMI_IDL_TYPE88(0, 113),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, tx) - QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, tx_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_tx_rx_info_resp_msg_v01, tx),
  QMI_IDL_TYPE88(0, 114)
};

static const uint8_t nas_update_akey_ext_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_update_akey_ext_req_msg_v01, akey_with_spc),
  QMI_IDL_TYPE88(0, 115)
};

static const uint8_t nas_update_akey_ext_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_update_akey_ext_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_managed_roaming_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_managed_roaming_ind_msg_v01, radio_if) - QMI_IDL_OFFSET8(nas_managed_roaming_ind_msg_v01, radio_if_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_managed_roaming_ind_msg_v01, radio_if)
};

/* 
 * nas_get_dual_standby_pref_req_msg is empty
 * static const uint8_t nas_get_dual_standby_pref_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_dual_standby_pref_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, standby_pref) - QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, standby_pref_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, standby_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, priority_subs) - QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, priority_subs_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, priority_subs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, active_subs) - QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, active_subs_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, active_subs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, default_data_subs) - QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, default_data_subs_valid)),
  0x13,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, default_data_subs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, default_voice_subs) - QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, default_voice_subs_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, default_voice_subs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, active_subs_mask) - QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, active_subs_mask_valid)),
  0x15,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_get_dual_standby_pref_resp_msg_v01, active_subs_mask)
};

/* 
 * nas_detach_lte_req_msg is empty
 * static const uint8_t nas_detach_lte_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_detach_lte_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_detach_lte_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_block_lte_plmn_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_block_lte_plmn_req_msg_v01, plmn),
  QMI_IDL_TYPE88(0, 116),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_block_lte_plmn_req_msg_v01, blocking_interval_abs) - QMI_IDL_OFFSET8(nas_block_lte_plmn_req_msg_v01, blocking_interval_abs_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_block_lte_plmn_req_msg_v01, blocking_interval_abs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_block_lte_plmn_req_msg_v01, blocking_interval_mult) - QMI_IDL_OFFSET8(nas_block_lte_plmn_req_msg_v01, blocking_interval_mult_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_block_lte_plmn_req_msg_v01, blocking_interval_mult)
};

static const uint8_t nas_block_lte_plmn_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_block_lte_plmn_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_unblock_lte_plmn_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_unblock_lte_plmn_req_msg_v01, plmn),
  QMI_IDL_TYPE88(0, 116)
};

static const uint8_t nas_unblock_lte_plmn_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_unblock_lte_plmn_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_reset_lte_plmn_blocking_req_msg is empty
 * static const uint8_t nas_reset_lte_plmn_blocking_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_reset_lte_plmn_blocking_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_reset_lte_plmn_blocking_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_current_plmn_name_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, plmn_id) - QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, plmn_id_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, plmn_id),
  QMI_IDL_TYPE88(0, 116),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, spn) - QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, spn_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, spn),
  QMI_IDL_TYPE88(0, 117),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, short_name) - QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, short_name_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_current_plmn_name_ind_msg_v01, short_name),
  QMI_IDL_TYPE88(0, 118),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, long_name) - QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, long_name_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_current_plmn_name_ind_msg_v01, long_name),
  QMI_IDL_TYPE88(0, 118),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, csg_id) - QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, csg_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_current_plmn_name_ind_msg_v01, csg_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, eons_display_bit_info) - QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, eons_display_bit_info_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_current_plmn_name_ind_msg_v01, eons_display_bit_info),
  QMI_IDL_TYPE88(0, 78),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, is_home_network) - QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, is_home_network_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_current_plmn_name_ind_msg_v01, is_home_network),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, rat) - QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, rat_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_current_plmn_name_ind_msg_v01, rat),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, lang_plmn_names) - QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, lang_plmn_names_valid)),
  0x18,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_current_plmn_name_ind_msg_v01, lang_plmn_names),
  NAS_ALT_LANG_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, lang_plmn_names) - QMI_IDL_OFFSET16RELATIVE(nas_current_plmn_name_ind_msg_v01, lang_plmn_names_len),
  QMI_IDL_TYPE88(0, 135)
};

static const uint8_t nas_config_embms_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_config_embms_req_msg_v01, enable),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_embms_req_msg_v01, trace_id) - QMI_IDL_OFFSET8(nas_config_embms_req_msg_v01, trace_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_embms_req_msg_v01, trace_id)
};

static const uint8_t nas_config_embms_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_config_embms_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_embms_resp_msg_v01, trace_id) - QMI_IDL_OFFSET8(nas_config_embms_resp_msg_v01, trace_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_embms_resp_msg_v01, trace_id)
};

/* 
 * nas_get_embms_status_req_msg is empty
 * static const uint8_t nas_get_embms_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_embms_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_embms_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_embms_status_resp_msg_v01, enabled) - QMI_IDL_OFFSET8(nas_get_embms_status_resp_msg_v01, enabled_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_embms_status_resp_msg_v01, enabled),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_embms_status_resp_msg_v01, trace_id) - QMI_IDL_OFFSET8(nas_get_embms_status_resp_msg_v01, trace_id_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_embms_status_resp_msg_v01, trace_id)
};

static const uint8_t nas_embms_status_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_embms_status_ind_msg_v01, enabled),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_embms_status_ind_msg_v01, trace_id) - QMI_IDL_OFFSET8(nas_embms_status_ind_msg_v01, trace_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_embms_status_ind_msg_v01, trace_id)
};

/* 
 * nas_get_cdma_position_info_req_msg is empty
 * static const uint8_t nas_get_cdma_position_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_cdma_position_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_cdma_position_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_cdma_position_info_resp_msg_v01, info) - QMI_IDL_OFFSET8(nas_get_cdma_position_info_resp_msg_v01, info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_cdma_position_info_resp_msg_v01, info),
  QMI_IDL_TYPE88(0, 120)
};

static const uint8_t nas_rf_band_info_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_rf_band_info_ind_msg_v01, rf_band_info),
  QMI_IDL_TYPE88(0, 11)
};

/* 
 * nas_force_network_search_req_msg is empty
 * static const uint8_t nas_force_network_search_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_force_network_search_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_force_network_search_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_network_reject_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, radio_if),

  0x02,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, reject_srv_domain),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, rej_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, plmn_id) - QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, plmn_id_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, plmn_id),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, csg_id) - QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, csg_id_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_network_reject_ind_msg_v01, csg_id)
};

/* 
 * nas_get_managed_roaming_config_req_msg is empty
 * static const uint8_t nas_get_managed_roaming_config_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_managed_roaming_config_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_managed_roaming_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_managed_roaming_config_resp_msg_v01, managed_roaming_supported) - QMI_IDL_OFFSET8(nas_get_managed_roaming_config_resp_msg_v01, managed_roaming_supported_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_managed_roaming_config_resp_msg_v01, managed_roaming_supported)
};

static const uint8_t nas_rtre_cfg_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_rtre_cfg_ind_msg_v01, rtre_cfg) - QMI_IDL_OFFSET8(nas_rtre_cfg_ind_msg_v01, rtre_cfg_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_rtre_cfg_ind_msg_v01, rtre_cfg),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_rtre_cfg_ind_msg_v01, rtre_cfg_pref) - QMI_IDL_OFFSET8(nas_rtre_cfg_ind_msg_v01, rtre_cfg_pref_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_rtre_cfg_ind_msg_v01, rtre_cfg_pref)
};

/* 
 * nas_get_centralized_eons_support_status_req_msg is empty
 * static const uint8_t nas_get_centralized_eons_support_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_centralized_eons_support_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_centralized_eons_support_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_centralized_eons_support_status_resp_msg_v01, centralized_eons_supported) - QMI_IDL_OFFSET8(nas_get_centralized_eons_support_status_resp_msg_v01, centralized_eons_supported_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_centralized_eons_support_status_resp_msg_v01, centralized_eons_supported)
};

static const uint8_t nas_config_sig_info2_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_rssi_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_rssi_threshold_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_rssi_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_rssi_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_rssi_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_rssi_delta) - QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_rssi_delta_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_rssi_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_ecio_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_ecio_threshold_list_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_ecio_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_ecio_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_ecio_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_ecio_delta) - QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_ecio_delta_valid)),
  0x13,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, cdma_ecio_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, hdr_rssi_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, hdr_rssi_threshold_list_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, hdr_rssi_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, hdr_rssi_threshold_list) - QMI_IDL_OFFSET8(nas_config_sig_info2_req_msg_v01, hdr_rssi_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_rssi_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_rssi_delta_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, hdr_rssi_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_ecio_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_ecio_threshold_list_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, hdr_ecio_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_ecio_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_ecio_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_ecio_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_ecio_delta_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, hdr_ecio_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_sinr_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_sinr_threshold_list_valid)),
  0x18,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, hdr_sinr_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_sinr_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_sinr_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_sinr_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_sinr_delta_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, hdr_sinr_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_io_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_io_threshold_list_valid)),
  0x1A,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, hdr_io_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_io_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_io_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_io_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, hdr_io_delta_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, hdr_io_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, gsm_rssi_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, gsm_rssi_threshold_list_valid)),
  0x1C,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, gsm_rssi_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, gsm_rssi_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, gsm_rssi_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, gsm_rssi_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, gsm_rssi_delta_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, gsm_rssi_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_rssi_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_rssi_threshold_list_valid)),
  0x1E,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, wcdma_rssi_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_rssi_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_rssi_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_rssi_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_rssi_delta_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, wcdma_rssi_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_ecio_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_ecio_threshold_list_valid)),
  0x20,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, wcdma_ecio_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_ecio_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_ecio_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_ecio_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, wcdma_ecio_delta_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, wcdma_ecio_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rssi_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rssi_threshold_list_valid)),
  0x22,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_rssi_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rssi_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rssi_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rssi_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rssi_delta_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_rssi_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_snr_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_snr_threshold_list_valid)),
  0x24,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_snr_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_snr_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_snr_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_snr_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_snr_delta_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_snr_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrq_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrq_threshold_list_valid)),
  0x26,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_rsrq_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrq_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrq_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrq_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrq_delta_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_rsrq_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrp_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrp_threshold_list_valid)),
  0x28,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_rsrp_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrp_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrp_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrp_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_rsrp_delta_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_rsrp_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_sig_rpt_config) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, lte_sig_rpt_config_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, lte_sig_rpt_config),
  QMI_IDL_TYPE88(0, 111),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_rscp_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_rscp_threshold_list_valid)),
  0x2B,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, tdscdma_rscp_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_rscp_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_rscp_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_rscp_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_rscp_delta_valid)),
  0x2C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, tdscdma_rscp_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_rssi_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_rssi_threshold_list_valid)),
  0x2D,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, tds_rssi_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_rssi_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_rssi_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_rssi_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_rssi_delta_valid)),
  0x2E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, tdscdma_rssi_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_ecio_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_ecio_threshold_list_valid)),
  0x2F,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, tds_ecio_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_ecio_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_ecio_threshold_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_ecio_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_ecio_delta_valid)),
  0x30,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, tdscdma_ecio_delta),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_sinr_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_sinr_threshold_list_valid)),
  0x31,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, tds_sinr_threshold_list),
  NAS_SIG_STR_THRESHOLD_LIST_MAX2_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_sinr_threshold_list) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tds_sinr_threshold_list_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_sinr_delta) - QMI_IDL_OFFSET16RELATIVE(nas_config_sig_info2_req_msg_v01, tdscdma_sinr_delta_valid)),
  0x32,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_config_sig_info2_req_msg_v01, tdscdma_sinr_delta)
};

static const uint8_t nas_config_sig_info2_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_config_sig_info2_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_tds_cell_and_position_info_req_msg is empty
 * static const uint8_t nas_get_tds_cell_and_position_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_tds_cell_and_position_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, tds_cell_info) - QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, tds_cell_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, tds_cell_info),
  QMI_IDL_TYPE88(0, 121),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, tds_nbr_cell_info) - QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, tds_nbr_cell_info_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, tds_nbr_cell_info),
  NAS_TDS_MAX_NBR_CELL_NUM_V01,
  QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, tds_nbr_cell_info) - QMI_IDL_OFFSET8(nas_get_tds_cell_and_position_info_resp_msg_v01, tds_nbr_cell_info_len),
  QMI_IDL_TYPE88(0, 122)
};

static const uint8_t nas_set_hplmn_irat_search_timer_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_set_hplmn_irat_search_timer_req_msg_v01, timer_value)
};

static const uint8_t nas_set_hplmn_irat_search_timer_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_hplmn_irat_search_timer_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_get_embms_sig_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_embms_sig_req_msg_v01, trace_id) - QMI_IDL_OFFSET8(nas_get_embms_sig_req_msg_v01, trace_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_embms_sig_req_msg_v01, trace_id)
};

static const uint8_t nas_get_embms_sig_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, trace_id) - QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, trace_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, sig_list) - QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, sig_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, sig_list),
  NAS_LTE_EMBMS_MAX_MBSFN_AREA_V01,
  QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, sig_list) - QMI_IDL_OFFSET8(nas_get_embms_sig_resp_msg_v01, sig_list_len),
  QMI_IDL_TYPE88(0, 123)
};

static const uint8_t nas_limit_sys_info_ind_reporting_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_limit_sys_info_ind_reporting_req_msg_v01, limit_sys_info_chg_rpt)
};

static const uint8_t nas_limit_sys_info_ind_reporting_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_limit_sys_info_ind_reporting_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_sys_info_ind_reporting_limit_req_msg is empty
 * static const uint8_t nas_get_sys_info_ind_reporting_limit_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_sys_info_ind_reporting_limit_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_get_sys_info_ind_reporting_limit_resp_msg_v01, limit_sys_info_chg_rpt),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_sys_info_ind_reporting_limit_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_update_ims_status_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_update_ims_status_req_msg_v01, sys_mode),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_update_ims_status_req_msg_v01, registration_state),
  NAS_IMS_REG_STATUS_MAX_V01,
  QMI_IDL_OFFSET8(nas_update_ims_status_req_msg_v01, registration_state) - QMI_IDL_OFFSET8(nas_update_ims_status_req_msg_v01, registration_state_len),
  QMI_IDL_TYPE88(0, 124)
};

static const uint8_t nas_update_ims_status_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_update_ims_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_ims_pref_status_req_msg is empty
 * static const uint8_t nas_get_ims_pref_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_ims_pref_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_ims_pref_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_ims_pref_status_resp_msg_v01, ims_pref) - QMI_IDL_OFFSET8(nas_get_ims_pref_status_resp_msg_v01, ims_pref_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_ims_pref_status_resp_msg_v01, ims_pref),
  QMI_IDL_TYPE88(0, 125)
};

static const uint8_t nas_ims_pref_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_ims_pref_status_ind_msg_v01, ims_pref),
  QMI_IDL_TYPE88(0, 125)
};

static const uint8_t nas_config_plmn_name_ind_reporting_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_config_plmn_name_ind_reporting_req_msg_v01, send_all_information)
};

static const uint8_t nas_config_plmn_name_ind_reporting_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_config_plmn_name_ind_reporting_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_cdma_avoid_system_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_cdma_avoid_system_req_msg_v01, avoid_type)
};

static const uint8_t nas_cdma_avoid_system_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_cdma_avoid_system_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_cdma_avoid_system_list_req_msg is empty
 * static const uint8_t nas_get_cdma_avoid_system_list_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_cdma_avoid_system_list_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_cdma_avoid_system_list_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_cdma_avoid_system_list_resp_msg_v01, nam1_systems) - QMI_IDL_OFFSET8(nas_get_cdma_avoid_system_list_resp_msg_v01, nam1_systems_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_cdma_avoid_system_list_resp_msg_v01, nam1_systems),
  NAS_MAX_CDMA_SYSTEMS_AVOIDED_V01,
  QMI_IDL_OFFSET8(nas_get_cdma_avoid_system_list_resp_msg_v01, nam1_systems) - QMI_IDL_OFFSET8(nas_get_cdma_avoid_system_list_resp_msg_v01, nam1_systems_len),
  QMI_IDL_TYPE88(0, 126)
};

static const uint8_t nas_set_hplmn_search_timer_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_hplmn_search_timer_req_msg_v01, timer_value)
};

static const uint8_t nas_set_hplmn_search_timer_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_hplmn_search_timer_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * nas_get_hplmn_search_timer_req_msg is empty
 * static const uint8_t nas_get_hplmn_search_timer_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_hplmn_search_timer_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_hplmn_search_timer_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_hplmn_search_timer_resp_msg_v01, timer_value) - QMI_IDL_OFFSET8(nas_get_hplmn_search_timer_resp_msg_v01, timer_value_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_hplmn_search_timer_resp_msg_v01, timer_value)
};

/* 
 * nas_get_subscription_info_req_msg is empty
 * static const uint8_t nas_get_subscription_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_subscription_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_priority_subs) - QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_priority_subs_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_priority_subs),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_active) - QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_active_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_active),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_default_data_subs) - QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_default_data_subs_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, is_default_data_subs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, voice_system_id) - QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, voice_system_id_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_subscription_info_resp_msg_v01, voice_system_id)
};

/* 
 * nas_get_network_time_req_msg is empty
 * static const uint8_t nas_get_network_time_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_network_time_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_network_time_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_network_time_resp_msg_v01, nas_3gpp2_time) - QMI_IDL_OFFSET8(nas_get_network_time_resp_msg_v01, nas_3gpp2_time_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_network_time_resp_msg_v01, nas_3gpp2_time),
  QMI_IDL_TYPE88(0, 127),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_network_time_resp_msg_v01, nas_3gpp_time) - QMI_IDL_OFFSET8(nas_get_network_time_resp_msg_v01, nas_3gpp_time_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_network_time_resp_msg_v01, nas_3gpp_time),
  QMI_IDL_TYPE88(0, 127)
};

/* 
 * nas_get_lte_sib16_network_time_req_msg is empty
 * static const uint8_t nas_get_lte_sib16_network_time_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_lte_sib16_network_time_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, lte_sib16_acquired) - QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, lte_sib16_acquired_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, lte_sib16_acquired),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, universal_time) - QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, universal_time_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, universal_time),
  QMI_IDL_TYPE88(0, 128),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, abs_time) - QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, abs_time_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, abs_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, leap_sec) - QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, leap_sec_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, leap_sec),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, time_zone) - QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, time_zone_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, time_zone),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, daylt_sav_adj) - QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, daylt_sav_adj_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_get_lte_sib16_network_time_resp_msg_v01, daylt_sav_adj)
};

static const uint8_t nas_lte_sib16_network_time_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, lte_sib16_acquired) - QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, lte_sib16_acquired_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, lte_sib16_acquired),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, universal_time) - QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, universal_time_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, universal_time),
  QMI_IDL_TYPE88(0, 128),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, abs_time) - QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, abs_time_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, abs_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, leap_sec) - QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, leap_sec_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, leap_sec),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, time_zone) - QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, time_zone_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, time_zone),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, daylt_sav_adj) - QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, daylt_sav_adj_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_lte_sib16_network_time_ind_msg_v01, daylt_sav_adj)
};

static const uint8_t nas_set_lte_band_priority_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_lte_band_priority_req_msg_v01, band_priority_list),
  NAS_LTE_BAND_PRIORITY_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_set_lte_band_priority_req_msg_v01, band_priority_list) - QMI_IDL_OFFSET8(nas_set_lte_band_priority_req_msg_v01, band_priority_list_len)
};

static const uint8_t nas_set_lte_band_priority_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_lte_band_priority_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_get_embms_sig_ext_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_embms_sig_ext_req_msg_v01, trace_id) - QMI_IDL_OFFSET8(nas_get_embms_sig_ext_req_msg_v01, trace_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_embms_sig_ext_req_msg_v01, trace_id)
};

static const uint8_t nas_get_embms_sig_ext_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, trace_id) - QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, trace_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, snr_and_tmgi_list) - QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, snr_and_tmgi_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, snr_and_tmgi_list),
  NAS_LTE_EMBMS_MAX_MBSFN_AREA_V01,
  QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, snr_and_tmgi_list) - QMI_IDL_OFFSET8(nas_get_embms_sig_ext_resp_msg_v01, snr_and_tmgi_list_len),
  QMI_IDL_TYPE88(0, 130)
};

static const uint8_t nas_lte_cphy_ca_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_lte_cphy_ca_ind_msg_v01, cphy_ca),
  QMI_IDL_TYPE88(0, 131)
};

/* 
 * nas_get_lte_band_priority_list_req_msg is empty
 * static const uint8_t nas_get_lte_band_priority_list_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t nas_get_lte_band_priority_list_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_get_lte_band_priority_list_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_get_lte_band_priority_list_resp_msg_v01, configured_band_priority_list) - QMI_IDL_OFFSET8(nas_get_lte_band_priority_list_resp_msg_v01, configured_band_priority_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_get_lte_band_priority_list_resp_msg_v01, configured_band_priority_list),
  NAS_LTE_BAND_PRIORITY_LIST_MAX_V01,
  QMI_IDL_OFFSET8(nas_get_lte_band_priority_list_resp_msg_v01, configured_band_priority_list) - QMI_IDL_OFFSET8(nas_get_lte_band_priority_list_resp_msg_v01, configured_band_priority_list_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_get_lte_band_priority_list_resp_msg_v01, supported_band_priority_list) - QMI_IDL_OFFSET16RELATIVE(nas_get_lte_band_priority_list_resp_msg_v01, supported_band_priority_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(nas_get_lte_band_priority_list_resp_msg_v01, supported_band_priority_list),
  NAS_LTE_BAND_PRIORITY_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_get_lte_band_priority_list_resp_msg_v01, supported_band_priority_list) - QMI_IDL_OFFSET16RELATIVE(nas_get_lte_band_priority_list_resp_msg_v01, supported_band_priority_list_len)
};

static const uint8_t nas_set_builtin_plmn_list_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_req_msg_v01, oplmn_list) - QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_req_msg_v01, oplmn_list_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_req_msg_v01, oplmn_list),
  QMI_IDL_TYPE88(0, 133),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_set_builtin_plmn_list_req_msg_v01, ind_token) - QMI_IDL_OFFSET16RELATIVE(nas_set_builtin_plmn_list_req_msg_v01, ind_token_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(nas_set_builtin_plmn_list_req_msg_v01, ind_token)
};

static const uint8_t nas_set_builtin_plmn_list_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_set_builtin_plmn_list_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, ind_token) - QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, ind_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, ind_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, received_list_entry_count) - QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, received_list_entry_count_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, received_list_entry_count),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, remaining_list_entry_count) - QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, remaining_list_entry_count_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_set_builtin_plmn_list_ind_msg_v01, remaining_list_entry_count)
};

static const uint8_t nas_perform_incremental_network_scan_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, network_type) - QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, network_type_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, network_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, scan_type) - QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, scan_type_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, scan_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, band_pref) - QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, band_pref_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, band_pref),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, lte_band_pref) - QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, lte_band_pref_valid)),
  0x13,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, lte_band_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, tdscdma_band_pref) - QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, tdscdma_band_pref_valid)),
  0x14,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_req_msg_v01, tdscdma_band_pref)
};

static const uint8_t nas_perform_incremental_network_scan_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t nas_perform_incremental_network_scan_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_ind_msg_v01, scan_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_ind_msg_v01, nas_network_scan_info) - QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_ind_msg_v01, nas_network_scan_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_ind_msg_v01, nas_network_scan_info),
  ((NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) & 0xFF), ((NAS_3GPP_NETWORK_INFO_LIST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_ind_msg_v01, nas_network_scan_info) - QMI_IDL_OFFSET8(nas_perform_incremental_network_scan_ind_msg_v01, nas_network_scan_info_len),
  QMI_IDL_TYPE88(0, 134),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(nas_perform_incremental_network_scan_ind_msg_v01, csg_info) - QMI_IDL_OFFSET16RELATIVE(nas_perform_incremental_network_scan_ind_msg_v01, csg_info_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(nas_perform_incremental_network_scan_ind_msg_v01, csg_info),
  NAS_3GPP_NETWORK_INFO_LIST_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(nas_perform_incremental_network_scan_ind_msg_v01, csg_info) - QMI_IDL_OFFSET16RELATIVE(nas_perform_incremental_network_scan_ind_msg_v01, csg_info_len),
  QMI_IDL_TYPE88(0, 23)
};

/* Type Table */
static const qmi_idl_type_table_entry  nas_type_table_v01[] = {
  {sizeof(nas_signal_stregth_indicator_type_v01), nas_signal_stregth_indicator_type_data_v01},
  {sizeof(nas_rssi_indicator_type_v01), nas_rssi_indicator_type_data_v01},
  {sizeof(nas_ecio_indicator_type_v01), nas_ecio_indicator_type_data_v01},
  {sizeof(nas_io_indicator_type_v01), nas_io_indicator_type_data_v01},
  {sizeof(nas_sinr_indicator_type_v01), nas_sinr_indicator_type_data_v01},
  {sizeof(nas_rsrq_indicator_type_v01), nas_rsrq_indicator_type_data_v01},
  {sizeof(nas_ecio_indicator_threshold_type_v01), nas_ecio_indicator_threshold_type_data_v01},
  {sizeof(nas_sinr_indicator_threshold_type_v01), nas_sinr_indicator_threshold_type_data_v01},
  {sizeof(nas_rsrp_indicator_type_v01), nas_rsrp_indicator_type_data_v01},
  {sizeof(nas_snr_indicator_type_v01), nas_snr_indicator_type_data_v01},
  {sizeof(nas_signal_strength_type_v01), nas_signal_strength_type_data_v01},
  {sizeof(nas_rf_band_info_type_v01), nas_rf_band_info_type_data_v01},
  {sizeof(nas_registration_reject_reason_type_v01), nas_registration_reject_reason_type_data_v01},
  {sizeof(nas_rssi_type_v01), nas_rssi_type_data_v01},
  {sizeof(nas_ecio_type_v01), nas_ecio_type_data_v01},
  {sizeof(nas_error_rate_type_v01), nas_error_rate_type_data_v01},
  {sizeof(nas_rsrq_type_v01), nas_rsrq_type_data_v01},
  {sizeof(nas_reg_network_reject_v01), nas_reg_network_reject_data_v01},
  {sizeof(nas_signal_strength_list_type_v01), nas_signal_strength_list_type_data_v01},
  {sizeof(nas_3gpp_network_info_type_v01), nas_3gpp_network_info_type_data_v01},
  {sizeof(nas_network_radio_access_technology_type_v01), nas_network_radio_access_technology_type_data_v01},
  {sizeof(nas_mnc_pcs_digit_include_status_type_v01), nas_mnc_pcs_digit_include_status_type_data_v01},
  {sizeof(nas_csg_info_type_v01), nas_csg_info_type_data_v01},
  {sizeof(nas_csg_nw_info_type_v01), nas_csg_nw_info_type_data_v01},
  {sizeof(nas_manual_network_register_info_type_v01), nas_manual_network_register_info_type_data_v01},
  {sizeof(nas_serving_system_type_v01), nas_serving_system_type_data_v01},
  {sizeof(nas_plmn_type_v01), nas_plmn_type_data_v01},
  {sizeof(nas_cdma_system_id_type_v01), nas_cdma_system_id_type_data_v01},
  {sizeof(nas_cdma_base_station_info_type_v01), nas_cdma_base_station_info_type_data_v01},
  {sizeof(nas_roaming_indicator_type_v01), nas_roaming_indicator_type_data_v01},
  {sizeof(nas_3gpp_time_zone_type_v01), nas_3gpp_time_zone_type_data_v01},
  {sizeof(nas_ss_detailed_service_info_type_v01), nas_ss_detailed_service_info_type_data_v01},
  {sizeof(nas_cdma_system_id_ext_type_v01), nas_cdma_system_id_ext_type_data_v01},
  {sizeof(nas_gw_sys_info3_type_v01), nas_gw_sys_info3_type_data_v01},
  {sizeof(nas_universal_time_and_local_time_zone_3gpp_type_v01), nas_universal_time_and_local_time_zone_3gpp_type_data_v01},
  {sizeof(nas_plmn_id_type_v01), nas_plmn_id_type_data_v01},
  {sizeof(nas_3gpp2_home_network_ext_type_v01), nas_3gpp2_home_network_ext_type_data_v01},
  {sizeof(nas_3gpp_mcs_digit_type_v01), nas_3gpp_mcs_digit_type_data_v01},
  {sizeof(nas_3gpp_preferred_networks_type_v01), nas_3gpp_preferred_networks_type_data_v01},
  {sizeof(nas_3gpp_forbidden_networks_type_v01), nas_3gpp_forbidden_networks_type_data_v01},
  {sizeof(nas_accolc_set_type_v01), nas_accolc_set_type_data_v01},
  {sizeof(nas_registration_parameters_type_v01), nas_registration_parameters_type_data_v01},
  {sizeof(nas_hdr_scp_config_type_v01), nas_hdr_scp_config_type_data_v01},
  {sizeof(nas_net_sel_pref_type_v01), nas_net_sel_pref_type_data_v01},
  {sizeof(nas_csg_nw_iden_type_v01), nas_csg_nw_iden_type_data_v01},
  {sizeof(nas_ddtm_preference_type_v01), nas_ddtm_preference_type_data_v01},
  {sizeof(nas_ddtm_settings_type_v01), nas_ddtm_settings_type_data_v01},
  {sizeof(nas_service_provider_name_type_v01), nas_service_provider_name_type_data_v01},
  {sizeof(nas_operator_plmn_list_type_v01), nas_operator_plmn_list_type_data_v01},
  {sizeof(nas_plmn_network_name_type_v01), nas_plmn_network_name_type_data_v01},
  {sizeof(nas_3gpp2_home_sid_nid_info_type_v01), nas_3gpp2_home_sid_nid_info_type_data_v01},
  {sizeof(nas_3gpp2_min_based_info_type_v01), nas_3gpp2_min_based_info_type_data_v01},
  {sizeof(nas_3gpp2_true_imsi_info_type_v01), nas_3gpp2_true_imsi_info_type_data_v01},
  {sizeof(nas_cdma_channel_info_type_v01), nas_cdma_channel_info_type_data_v01},
  {sizeof(nas_nmr_cell_info_type_v01), nas_nmr_cell_info_type_data_v01},
  {sizeof(nas_geran_cell_info_type_v01), nas_geran_cell_info_type_data_v01},
  {sizeof(nas_umts_monitored_cell_set_info_type_v01), nas_umts_monitored_cell_set_info_type_data_v01},
  {sizeof(nas_umts_geran_nbr_cell_set_info_type_v01), nas_umts_geran_nbr_cell_set_info_type_data_v01},
  {sizeof(nas_umts_lte_nbr_cell_set_info_type_v01), nas_umts_lte_nbr_cell_set_info_type_data_v01},
  {sizeof(nas_umts_cell_info_type_v01), nas_umts_cell_info_type_data_v01},
  {sizeof(nas_cdma_cell_info_type_v01), nas_cdma_cell_info_type_data_v01},
  {sizeof(nas_lte_ngbr_cell_type_v01), nas_lte_ngbr_cell_type_data_v01},
  {sizeof(nas_lte_intra_freq_type_v01), nas_lte_intra_freq_type_data_v01},
  {sizeof(nas_lte_inter_freq_freqs_type_v01), nas_lte_inter_freq_freqs_type_data_v01},
  {sizeof(nas_lte_inter_freq_type_v01), nas_lte_inter_freq_type_data_v01},
  {sizeof(nas_lte_ngbr_gsm_cell_type_v01), nas_lte_ngbr_gsm_cell_type_data_v01},
  {sizeof(nas_lte_ngbr_gsm_freq_type_v01), nas_lte_ngbr_gsm_freq_type_data_v01},
  {sizeof(nas_lte_ngbr_gsm_type_v01), nas_lte_ngbr_gsm_type_data_v01},
  {sizeof(nas_lte_ngbr_wcdma_cell_type_v01), nas_lte_ngbr_wcdma_cell_type_data_v01},
  {sizeof(nas_lte_ngbr_wcdma_freq_type_v01), nas_lte_ngbr_wcdma_freq_type_data_v01},
  {sizeof(nas_lte_ngbr_wcdma_type_v01), nas_lte_ngbr_wcdma_type_data_v01},
  {sizeof(nas_wcdma_ngbr_lte_type_v01), nas_wcdma_ngbr_lte_type_data_v01},
  {sizeof(nas_rx_power_type_v01), nas_rx_power_type_data_v01},
  {sizeof(nas_gsm_cell_info_ext_type_v01), nas_gsm_cell_info_ext_type_data_v01},
  {sizeof(nas_wcdma_cell_info_ext_type_v01), nas_wcdma_cell_info_ext_type_data_v01},
  {sizeof(nas_wcdma_active_set_info_type_v01), nas_wcdma_active_set_info_type_data_v01},
  {sizeof(nas_wcdma_active_set_reference_rl_type_v01), nas_wcdma_active_set_reference_rl_type_data_v01},
  {sizeof(nas_3gpp_eons_plmn_name_type_v01), nas_3gpp_eons_plmn_name_type_data_v01},
  {sizeof(nas_display_bit_type_v01), nas_display_bit_type_data_v01},
  {sizeof(nas_standby_pref_type_v01), nas_standby_pref_type_data_v01},
  {sizeof(nas_technology_pref_type_v01), nas_technology_pref_type_data_v01},
  {sizeof(nas_active_technology_type_v01), nas_active_technology_type_data_v01},
  {sizeof(nas_julian_time_type_v01), nas_julian_time_type_data_v01},
  {sizeof(nas_reg_reject_info_type_v01), nas_reg_reject_info_type_data_v01},
  {sizeof(nas_3gpp2_srv_status_info_type_v01), nas_3gpp2_srv_status_info_type_data_v01},
  {sizeof(nas_3gpp_srv_status_info_type_v01), nas_3gpp_srv_status_info_type_data_v01},
  {sizeof(nas_common_network_id_type_v01), nas_common_network_id_type_data_v01},
  {sizeof(nas_common_sys_info_type_v01), nas_common_sys_info_type_data_v01},
  {sizeof(nas_cdma_only_sys_info_type_v01), nas_cdma_only_sys_info_type_data_v01},
  {sizeof(nas_hdr_only_sys_info_type_v01), nas_hdr_only_sys_info_type_data_v01},
  {sizeof(nas_gsm_only_sys_info_type_v01), nas_gsm_only_sys_info_type_data_v01},
  {sizeof(nas_wcdma_only_sys_info_type_v01), nas_wcdma_only_sys_info_type_data_v01},
  {sizeof(nas_lte_only_sys_info_type_v01), nas_lte_only_sys_info_type_data_v01},
  {sizeof(nas_cdma_hdr_only_sys_info_type_v01), nas_cdma_hdr_only_sys_info_type_data_v01},
  {sizeof(nas_3gpp_only_sys_info_type_v01), nas_3gpp_only_sys_info_type_data_v01},
  {sizeof(nas_cdma_sys_info_type_v01), nas_cdma_sys_info_type_data_v01},
  {sizeof(nas_hdr_sys_info_type_v01), nas_hdr_sys_info_type_data_v01},
  {sizeof(nas_gsm_sys_info_type_v01), nas_gsm_sys_info_type_data_v01},
  {sizeof(nas_wcdma_sys_info_type_v01), nas_wcdma_sys_info_type_data_v01},
  {sizeof(nas_lte_sys_info_type_v01), nas_lte_sys_info_type_data_v01},
  {sizeof(nas_tdscdma_only_sys_info_type_v01), nas_tdscdma_only_sys_info_type_data_v01},
  {sizeof(nas_tdscdma_sys_info_type_v01), nas_tdscdma_sys_info_type_data_v01},
  {sizeof(nas_cdma_sys_info2_type_v01), nas_cdma_sys_info2_type_data_v01},
  {sizeof(nas_hdr_sys_info2_type_v01), nas_hdr_sys_info2_type_data_v01},
  {sizeof(nas_gsm_sys_info2_type_v01), nas_gsm_sys_info2_type_data_v01},
  {sizeof(nas_wcdma_sys_info2_type_v01), nas_wcdma_sys_info2_type_data_v01},
  {sizeof(nas_lte_sys_info2_type_v01), nas_lte_sys_info2_type_data_v01},
  {sizeof(nas_common_sig_info_param_type_v01), nas_common_sig_info_param_type_data_v01},
  {sizeof(nas_hdr_sig_info_type_v01), nas_hdr_sig_info_type_data_v01},
  {sizeof(nas_lte_sig_info_type_v01), nas_lte_sig_info_type_data_v01},
  {sizeof(nas_tdscdma_sig_info_type_v01), nas_tdscdma_sig_info_type_data_v01},
  {sizeof(nas_lte_sig_rpt_config_type_v01), nas_lte_sig_rpt_config_type_data_v01},
  {sizeof(nas_set_rx_diversity_req_param_type_v01), nas_set_rx_diversity_req_param_type_data_v01},
  {sizeof(nas_rx_chain_info_type_v01), nas_rx_chain_info_type_data_v01},
  {sizeof(nas_tx_info_type_v01), nas_tx_info_type_data_v01},
  {sizeof(nas_akey_with_spc_type_v01), nas_akey_with_spc_type_data_v01},
  {sizeof(nas_plmn_id_ext_type_v01), nas_plmn_id_ext_type_data_v01},
  {sizeof(nas_spn_type_v01), nas_spn_type_data_v01},
  {sizeof(nas_plmn_name_type_v01), nas_plmn_name_type_data_v01},
  {sizeof(nas_cdma_bs_info_type_v01), nas_cdma_bs_info_type_data_v01},
  {sizeof(nas_cdma_position_info_type_v01), nas_cdma_position_info_type_data_v01},
  {sizeof(nas_tds_cell_info_type_v01), nas_tds_cell_info_type_data_v01},
  {sizeof(nas_tds_nbr_cell_info_type_v01), nas_tds_nbr_cell_info_type_data_v01},
  {sizeof(nas_lte_cphy_mbsfn_area_signal_strength_type_v01), nas_lte_cphy_mbsfn_area_signal_strength_type_data_v01},
  {sizeof(nas_ims_registration_state_type_v01), nas_ims_registration_state_type_data_v01},
  {sizeof(nas_ims_pref_type_v01), nas_ims_pref_type_data_v01},
  {sizeof(nas_cdma_avoid_sys_info_type_v01), nas_cdma_avoid_sys_info_type_data_v01},
  {sizeof(nas_network_time_type_v01), nas_network_time_type_data_v01},
  {sizeof(nas_lte_sib16_julian_time_type_v01), nas_lte_sib16_julian_time_type_data_v01},
  {sizeof(nas_embms_tmgi_type_v01), nas_embms_tmgi_type_data_v01},
  {sizeof(nas_lte_embms_signal_type_v01), nas_lte_embms_signal_type_data_v01},
  {sizeof(nas_lte_cphy_ca_indicator_type_v01), nas_lte_cphy_ca_indicator_type_data_v01},
  {sizeof(nas_oplmn_entry_type_v01), nas_oplmn_entry_type_data_v01},
  {sizeof(nas_oplmn_list_type_v01), nas_oplmn_list_type_data_v01},
  {sizeof(nas_3gpp_network_scan_result_type_v01), nas_3gpp_network_scan_result_type_data_v01},
  {sizeof(nas_lang_plmn_names_type_v01), nas_lang_plmn_names_type_data_v01},
  {sizeof(nas_csg_nw_signal_strength_info_type_v01), nas_csg_nw_signal_strength_info_type_data_v01},
  {sizeof(nas_umts_monitored_cell_set_ext_info_type_v01), nas_umts_monitored_cell_set_ext_info_type_data_v01},
  {sizeof(nas_umts_geran_nbr_cell_set_ext_info_type_v01), nas_umts_geran_nbr_cell_set_ext_info_type_data_v01},
  {sizeof(nas_umts_cell_ext_info_type_v01), nas_umts_cell_ext_info_type_data_v01},
  {sizeof(nas_wcdma_active_set_ext_info_type_v01), nas_wcdma_active_set_ext_info_type_data_v01},
  {sizeof(nas_nmr_cell_info_ext_type_v01), nas_nmr_cell_info_ext_type_data_v01},
  {sizeof(nas_geran_cell_info_ext_type_v01), nas_geran_cell_info_ext_type_data_v01},
  {sizeof(nas_geran_eng_mode_scell_config_type_v01), nas_geran_eng_mode_scell_config_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry nas_message_table_v01[] = {
  {sizeof(nas_reset_req_msg_v01), 0},
  {sizeof(nas_reset_resp_msg_v01), nas_reset_resp_msg_data_v01},
  {sizeof(nas_abort_req_msg_v01), nas_abort_req_msg_data_v01},
  {sizeof(nas_abort_resp_msg_v01), nas_abort_resp_msg_data_v01},
  {sizeof(nas_set_event_report_req_msg_v01), nas_set_event_report_req_msg_data_v01},
  {sizeof(nas_set_event_report_resp_msg_v01), nas_set_event_report_resp_msg_data_v01},
  {sizeof(nas_event_report_ind_msg_v01), nas_event_report_ind_msg_data_v01},
  {sizeof(nas_indication_register_req_msg_v01), nas_indication_register_req_msg_data_v01},
  {sizeof(nas_indication_register_resp_msg_v01), nas_indication_register_resp_msg_data_v01},
  {sizeof(nas_get_signal_strength_req_msg_v01), nas_get_signal_strength_req_msg_data_v01},
  {sizeof(nas_get_signal_strength_resp_msg_v01), nas_get_signal_strength_resp_msg_data_v01},
  {sizeof(nas_perform_network_scan_req_msg_v01), nas_perform_network_scan_req_msg_data_v01},
  {sizeof(nas_perform_network_scan_resp_msg_v01), nas_perform_network_scan_resp_msg_data_v01},
  {sizeof(nas_initiate_network_register_req_msg_v01), nas_initiate_network_register_req_msg_data_v01},
  {sizeof(nas_initiate_network_register_resp_msg_v01), nas_initiate_network_register_resp_msg_data_v01},
  {sizeof(nas_initiate_attach_req_msg_v01), nas_initiate_attach_req_msg_data_v01},
  {sizeof(nas_initiate_attach_resp_msg_v01), nas_initiate_attach_resp_msg_data_v01},
  {sizeof(nas_get_serving_system_req_msg_v01), 0},
  {sizeof(nas_get_serving_system_resp_msg_v01), nas_get_serving_system_resp_msg_data_v01},
  {sizeof(nas_serving_system_ind_msg_v01), nas_serving_system_ind_msg_data_v01},
  {sizeof(nas_get_home_network_req_msg_v01), 0},
  {sizeof(nas_get_home_network_resp_msg_v01), nas_get_home_network_resp_msg_data_v01},
  {sizeof(nas_get_preferred_networks_req_msg_v01), 0},
  {sizeof(nas_get_preferred_networks_resp_msg_v01), nas_get_preferred_networks_resp_msg_data_v01},
  {sizeof(nas_set_preferred_networks_req_msg_v01), nas_set_preferred_networks_req_msg_data_v01},
  {sizeof(nas_set_preferred_networks_resp_msg_v01), nas_set_preferred_networks_resp_msg_data_v01},
  {sizeof(nas_get_forbidden_networks_req_msg_v01), 0},
  {sizeof(nas_get_forbidden_networks_resp_msg_v01), nas_get_forbidden_networks_resp_msg_data_v01},
  {sizeof(nas_set_forbidden_networks_req_msg_v01), nas_set_forbidden_networks_req_msg_data_v01},
  {sizeof(nas_set_forbidden_networks_resp_msg_v01), nas_set_forbidden_networks_resp_msg_data_v01},
  {sizeof(nas_get_accolc_req_msg_v01), 0},
  {sizeof(nas_get_accolc_resp_msg_v01), nas_get_accolc_resp_msg_data_v01},
  {sizeof(nas_set_accolc_req_msg_v01), nas_set_accolc_req_msg_data_v01},
  {sizeof(nas_set_accolc_resp_msg_v01), nas_set_accolc_resp_msg_data_v01},
  {sizeof(nas_get_device_config_req_msg_v01), 0},
  {sizeof(nas_get_device_config_resp_msg_v01), nas_get_device_config_resp_msg_data_v01},
  {sizeof(nas_set_device_config_req_msg_v01), nas_set_device_config_req_msg_data_v01},
  {sizeof(nas_set_device_config_resp_msg_v01), nas_set_device_config_resp_msg_data_v01},
  {sizeof(nas_get_rf_band_info_req_msg_v01), 0},
  {sizeof(nas_get_rf_band_info_resp_msg_v01), nas_get_rf_band_info_resp_msg_data_v01},
  {sizeof(nas_get_an_aaa_status_req_msg_v01), 0},
  {sizeof(nas_get_an_aaa_status_resp_msg_v01), nas_get_an_aaa_status_resp_msg_data_v01},
  {sizeof(nas_set_system_selection_preference_req_msg_v01), nas_set_system_selection_preference_req_msg_data_v01},
  {sizeof(nas_set_system_selection_preference_resp_msg_v01), nas_set_system_selection_preference_resp_msg_data_v01},
  {sizeof(nas_get_system_selection_preference_req_msg_v01), 0},
  {sizeof(nas_get_system_selection_preference_resp_msg_v01), nas_get_system_selection_preference_resp_msg_data_v01},
  {sizeof(nas_system_selection_preference_ind_msg_v01), nas_system_selection_preference_ind_msg_data_v01},
  {sizeof(nas_set_ddtm_preference_req_msg_v01), nas_set_ddtm_preference_req_msg_data_v01},
  {sizeof(nas_set_ddtm_preference_resp_msg_v01), nas_set_ddtm_preference_resp_msg_data_v01},
  {sizeof(nas_ddtm_ind_msg_v01), nas_ddtm_ind_msg_data_v01},
  {sizeof(nas_get_operator_name_data_req_msg_v01), 0},
  {sizeof(nas_get_operator_name_data_resp_msg_v01), nas_get_operator_name_data_resp_msg_data_v01},
  {sizeof(nas_operator_name_data_ind_msg_v01), nas_operator_name_data_ind_msg_data_v01},
  {sizeof(nas_get_csp_plmn_mode_bit_req_msg_v01), 0},
  {sizeof(nas_get_csp_plmn_mode_bit_resp_msg_v01), nas_get_csp_plmn_mode_bit_resp_msg_data_v01},
  {sizeof(nas_csp_plmn_mode_bit_ind_msg_v01), nas_csp_plmn_mode_bit_ind_msg_data_v01},
  {sizeof(nas_update_akey_req_msg_v01), nas_update_akey_req_msg_data_v01},
  {sizeof(nas_update_akey_resp_msg_v01), nas_update_akey_resp_msg_data_v01},
  {sizeof(nas_get_3gpp2_subscription_info_req_msg_v01), nas_get_3gpp2_subscription_info_req_msg_data_v01},
  {sizeof(nas_get_3gpp2_subscription_info_resp_msg_v01), nas_get_3gpp2_subscription_info_resp_msg_data_v01},
  {sizeof(nas_set_3gpp2_subscription_info_req_msg_v01), nas_set_3gpp2_subscription_info_req_msg_data_v01},
  {sizeof(nas_set_3gpp2_subscription_info_resp_msg_v01), nas_set_3gpp2_subscription_info_resp_msg_data_v01},
  {sizeof(nas_get_mob_cai_rev_req_v01), 0},
  {sizeof(nas_get_mob_cai_rev_resp_v01), nas_get_mob_cai_rev_resp_data_v01},
  {sizeof(nas_get_rtre_config_req_v01), 0},
  {sizeof(nas_get_rtre_config_resp_v01), nas_get_rtre_config_resp_data_v01},
  {sizeof(nas_set_rtre_config_req_v01), nas_set_rtre_config_req_data_v01},
  {sizeof(nas_set_rtre_config_resp_v01), nas_set_rtre_config_resp_data_v01},
  {sizeof(nas_get_cell_location_info_req_msg_v01), 0},
  {sizeof(nas_get_cell_location_info_resp_msg_v01), nas_get_cell_location_info_resp_msg_data_v01},
  {sizeof(nas_get_plmn_name_req_msg_v01), nas_get_plmn_name_req_msg_data_v01},
  {sizeof(nas_get_plmn_name_resp_msg_v01), nas_get_plmn_name_resp_msg_data_v01},
  {sizeof(nas_bind_subscription_req_msg_v01), nas_bind_subscription_req_msg_data_v01},
  {sizeof(nas_bind_subscription_resp_msg_v01), nas_bind_subscription_resp_msg_data_v01},
  {sizeof(nas_set_dual_standby_pref_req_msg_v01), nas_set_dual_standby_pref_req_msg_data_v01},
  {sizeof(nas_set_dual_standby_pref_resp_msg_v01), nas_set_dual_standby_pref_resp_msg_data_v01},
  {sizeof(nas_dual_standby_pref_ind_msg_v01), nas_dual_standby_pref_ind_msg_data_v01},
  {sizeof(nas_subscription_info_ind_msg_v01), nas_subscription_info_ind_msg_data_v01},
  {sizeof(nas_get_mode_pref_req_msg_v01), 0},
  {sizeof(nas_get_mode_pref_resp_msg_v01), nas_get_mode_pref_resp_msg_data_v01},
  {sizeof(nas_set_technology_preference_req_msg_v01), nas_set_technology_preference_req_msg_data_v01},
  {sizeof(nas_set_technology_preference_resp_msg_v01), nas_set_technology_preference_resp_msg_data_v01},
  {sizeof(nas_get_technology_preference_req_type_v01), 0},
  {sizeof(nas_get_technology_preference_resp_type_v01), nas_get_technology_preference_resp_type_data_v01},
  {sizeof(nas_get_network_system_preference_req_v01), 0},
  {sizeof(nas_get_network_system_preference_resp_v01), nas_get_network_system_preference_resp_data_v01},
  {sizeof(nas_network_time_ind_msg_v01), nas_network_time_ind_msg_data_v01},
  {sizeof(nas_get_sys_info_req_msg_v01), 0},
  {sizeof(nas_get_sys_info_resp_msg_v01), nas_get_sys_info_resp_msg_data_v01},
  {sizeof(nas_sys_info_ind_msg_v01), nas_sys_info_ind_msg_data_v01},
  {sizeof(nas_get_sig_info_req_msg_v01), 0},
  {sizeof(nas_get_sig_info_resp_msg_v01), nas_get_sig_info_resp_msg_data_v01},
  {sizeof(nas_config_sig_info_req_msg_v01), nas_config_sig_info_req_msg_data_v01},
  {sizeof(nas_config_sig_info_resp_msg_v01), nas_config_sig_info_resp_msg_data_v01},
  {sizeof(nas_sig_info_ind_msg_v01), nas_sig_info_ind_msg_data_v01},
  {sizeof(nas_get_err_rate_req_msg_v01), 0},
  {sizeof(nas_get_err_rate_resp_msg_v01), nas_get_err_rate_resp_msg_data_v01},
  {sizeof(nas_err_rate_ind_msg_v01), nas_err_rate_ind_msg_data_v01},
  {sizeof(nas_hdr_session_close_ind_msg_v01), nas_hdr_session_close_ind_msg_data_v01},
  {sizeof(nas_hdr_uati_update_ind_msg_v01), nas_hdr_uati_update_ind_msg_data_v01},
  {sizeof(nas_get_hdr_subtype_req_msg_v01), nas_get_hdr_subtype_req_msg_data_v01},
  {sizeof(nas_get_hdr_subtype_resp_msg_v01), nas_get_hdr_subtype_resp_msg_data_v01},
  {sizeof(nas_get_hdr_color_code_req_msg_v01), 0},
  {sizeof(nas_get_hdr_color_code_resp_msg_v01), nas_get_hdr_color_code_resp_msg_data_v01},
  {sizeof(nas_get_current_acq_sys_mode_req_msg_v01), 0},
  {sizeof(nas_get_current_acq_sys_mode_resp_msg_v01), nas_get_current_acq_sys_mode_resp_msg_data_v01},
  {sizeof(nas_set_rx_diversity_req_msg_v01), nas_set_rx_diversity_req_msg_data_v01},
  {sizeof(nas_set_rx_diversity_resp_msg_v01), nas_set_rx_diversity_resp_msg_data_v01},
  {sizeof(nas_get_tx_rx_info_req_msg_v01), nas_get_tx_rx_info_req_msg_data_v01},
  {sizeof(nas_get_tx_rx_info_resp_msg_v01), nas_get_tx_rx_info_resp_msg_data_v01},
  {sizeof(nas_update_akey_ext_req_msg_v01), nas_update_akey_ext_req_msg_data_v01},
  {sizeof(nas_update_akey_ext_resp_msg_v01), nas_update_akey_ext_resp_msg_data_v01},
  {sizeof(nas_managed_roaming_ind_msg_v01), nas_managed_roaming_ind_msg_data_v01},
  {sizeof(nas_get_dual_standby_pref_req_msg_v01), 0},
  {sizeof(nas_get_dual_standby_pref_resp_msg_v01), nas_get_dual_standby_pref_resp_msg_data_v01},
  {sizeof(nas_detach_lte_req_msg_v01), 0},
  {sizeof(nas_detach_lte_resp_msg_v01), nas_detach_lte_resp_msg_data_v01},
  {sizeof(nas_block_lte_plmn_req_msg_v01), nas_block_lte_plmn_req_msg_data_v01},
  {sizeof(nas_block_lte_plmn_resp_msg_v01), nas_block_lte_plmn_resp_msg_data_v01},
  {sizeof(nas_unblock_lte_plmn_req_msg_v01), nas_unblock_lte_plmn_req_msg_data_v01},
  {sizeof(nas_unblock_lte_plmn_resp_msg_v01), nas_unblock_lte_plmn_resp_msg_data_v01},
  {sizeof(nas_reset_lte_plmn_blocking_req_msg_v01), 0},
  {sizeof(nas_reset_lte_plmn_blocking_resp_msg_v01), nas_reset_lte_plmn_blocking_resp_msg_data_v01},
  {sizeof(nas_current_plmn_name_ind_msg_v01), nas_current_plmn_name_ind_msg_data_v01},
  {sizeof(nas_config_embms_req_msg_v01), nas_config_embms_req_msg_data_v01},
  {sizeof(nas_config_embms_resp_msg_v01), nas_config_embms_resp_msg_data_v01},
  {sizeof(nas_get_embms_status_req_msg_v01), 0},
  {sizeof(nas_get_embms_status_resp_msg_v01), nas_get_embms_status_resp_msg_data_v01},
  {sizeof(nas_embms_status_ind_msg_v01), nas_embms_status_ind_msg_data_v01},
  {sizeof(nas_get_cdma_position_info_req_msg_v01), 0},
  {sizeof(nas_get_cdma_position_info_resp_msg_v01), nas_get_cdma_position_info_resp_msg_data_v01},
  {sizeof(nas_rf_band_info_ind_msg_v01), nas_rf_band_info_ind_msg_data_v01},
  {sizeof(nas_force_network_search_req_msg_v01), 0},
  {sizeof(nas_force_network_search_resp_msg_v01), nas_force_network_search_resp_msg_data_v01},
  {sizeof(nas_network_reject_ind_msg_v01), nas_network_reject_ind_msg_data_v01},
  {sizeof(nas_get_managed_roaming_config_req_msg_v01), 0},
  {sizeof(nas_get_managed_roaming_config_resp_msg_v01), nas_get_managed_roaming_config_resp_msg_data_v01},
  {sizeof(nas_rtre_cfg_ind_msg_v01), nas_rtre_cfg_ind_msg_data_v01},
  {sizeof(nas_get_centralized_eons_support_status_req_msg_v01), 0},
  {sizeof(nas_get_centralized_eons_support_status_resp_msg_v01), nas_get_centralized_eons_support_status_resp_msg_data_v01},
  {sizeof(nas_config_sig_info2_req_msg_v01), nas_config_sig_info2_req_msg_data_v01},
  {sizeof(nas_config_sig_info2_resp_msg_v01), nas_config_sig_info2_resp_msg_data_v01},
  {sizeof(nas_get_tds_cell_and_position_info_req_msg_v01), 0},
  {sizeof(nas_get_tds_cell_and_position_info_resp_msg_v01), nas_get_tds_cell_and_position_info_resp_msg_data_v01},
  {sizeof(nas_set_hplmn_irat_search_timer_req_msg_v01), nas_set_hplmn_irat_search_timer_req_msg_data_v01},
  {sizeof(nas_set_hplmn_irat_search_timer_resp_msg_v01), nas_set_hplmn_irat_search_timer_resp_msg_data_v01},
  {sizeof(nas_get_embms_sig_req_msg_v01), nas_get_embms_sig_req_msg_data_v01},
  {sizeof(nas_get_embms_sig_resp_msg_v01), nas_get_embms_sig_resp_msg_data_v01},
  {sizeof(nas_limit_sys_info_ind_reporting_req_msg_v01), nas_limit_sys_info_ind_reporting_req_msg_data_v01},
  {sizeof(nas_limit_sys_info_ind_reporting_resp_msg_v01), nas_limit_sys_info_ind_reporting_resp_msg_data_v01},
  {sizeof(nas_get_sys_info_ind_reporting_limit_req_msg_v01), 0},
  {sizeof(nas_get_sys_info_ind_reporting_limit_resp_msg_v01), nas_get_sys_info_ind_reporting_limit_resp_msg_data_v01},
  {sizeof(nas_update_ims_status_req_msg_v01), nas_update_ims_status_req_msg_data_v01},
  {sizeof(nas_update_ims_status_resp_msg_v01), nas_update_ims_status_resp_msg_data_v01},
  {sizeof(nas_get_ims_pref_status_req_msg_v01), 0},
  {sizeof(nas_get_ims_pref_status_resp_msg_v01), nas_get_ims_pref_status_resp_msg_data_v01},
  {sizeof(nas_ims_pref_status_ind_msg_v01), nas_ims_pref_status_ind_msg_data_v01},
  {sizeof(nas_config_plmn_name_ind_reporting_req_msg_v01), nas_config_plmn_name_ind_reporting_req_msg_data_v01},
  {sizeof(nas_config_plmn_name_ind_reporting_resp_msg_v01), nas_config_plmn_name_ind_reporting_resp_msg_data_v01},
  {sizeof(nas_cdma_avoid_system_req_msg_v01), nas_cdma_avoid_system_req_msg_data_v01},
  {sizeof(nas_cdma_avoid_system_resp_msg_v01), nas_cdma_avoid_system_resp_msg_data_v01},
  {sizeof(nas_get_cdma_avoid_system_list_req_msg_v01), 0},
  {sizeof(nas_get_cdma_avoid_system_list_resp_msg_v01), nas_get_cdma_avoid_system_list_resp_msg_data_v01},
  {sizeof(nas_set_hplmn_search_timer_req_msg_v01), nas_set_hplmn_search_timer_req_msg_data_v01},
  {sizeof(nas_set_hplmn_search_timer_resp_msg_v01), nas_set_hplmn_search_timer_resp_msg_data_v01},
  {sizeof(nas_get_hplmn_search_timer_req_msg_v01), 0},
  {sizeof(nas_get_hplmn_search_timer_resp_msg_v01), nas_get_hplmn_search_timer_resp_msg_data_v01},
  {sizeof(nas_get_subscription_info_req_msg_v01), 0},
  {sizeof(nas_get_subscription_info_resp_msg_v01), nas_get_subscription_info_resp_msg_data_v01},
  {sizeof(nas_get_network_time_req_msg_v01), 0},
  {sizeof(nas_get_network_time_resp_msg_v01), nas_get_network_time_resp_msg_data_v01},
  {sizeof(nas_get_lte_sib16_network_time_req_msg_v01), 0},
  {sizeof(nas_get_lte_sib16_network_time_resp_msg_v01), nas_get_lte_sib16_network_time_resp_msg_data_v01},
  {sizeof(nas_lte_sib16_network_time_ind_msg_v01), nas_lte_sib16_network_time_ind_msg_data_v01},
  {sizeof(nas_set_lte_band_priority_req_msg_v01), nas_set_lte_band_priority_req_msg_data_v01},
  {sizeof(nas_set_lte_band_priority_resp_msg_v01), nas_set_lte_band_priority_resp_msg_data_v01},
  {sizeof(nas_get_embms_sig_ext_req_msg_v01), nas_get_embms_sig_ext_req_msg_data_v01},
  {sizeof(nas_get_embms_sig_ext_resp_msg_v01), nas_get_embms_sig_ext_resp_msg_data_v01},
  {sizeof(nas_lte_cphy_ca_ind_msg_v01), nas_lte_cphy_ca_ind_msg_data_v01},
  {sizeof(nas_get_lte_band_priority_list_req_msg_v01), 0},
  {sizeof(nas_get_lte_band_priority_list_resp_msg_v01), nas_get_lte_band_priority_list_resp_msg_data_v01},
  {sizeof(nas_set_builtin_plmn_list_req_msg_v01), nas_set_builtin_plmn_list_req_msg_data_v01},
  {sizeof(nas_set_builtin_plmn_list_resp_msg_v01), nas_set_builtin_plmn_list_resp_msg_data_v01},
  {sizeof(nas_set_builtin_plmn_list_ind_msg_v01), nas_set_builtin_plmn_list_ind_msg_data_v01},
  {sizeof(nas_perform_incremental_network_scan_req_msg_v01), nas_perform_incremental_network_scan_req_msg_data_v01},
  {sizeof(nas_perform_incremental_network_scan_resp_msg_v01), nas_perform_incremental_network_scan_resp_msg_data_v01},
  {sizeof(nas_perform_incremental_network_scan_ind_msg_v01), nas_perform_incremental_network_scan_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object nas_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *nas_qmi_idl_type_table_object_referenced_tables_v01[] =
{&nas_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object nas_qmi_idl_type_table_object_v01 = {
  sizeof(nas_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(nas_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  nas_type_table_v01,
  nas_message_table_v01,
  nas_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry nas_service_command_messages_v01[] = {
  {QMI_NAS_RESET_REQ_MSG_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_NAS_ABORT_REQ_MSG_V01, QMI_IDL_TYPE16(0, 2), 5},
  {QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01, QMI_IDL_TYPE16(0, 4), 93},
  {QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01, QMI_IDL_TYPE16(0, 7), 93},
  {QMI_NAS_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_NAS_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_NAS_GET_SIGNAL_STRENGTH_REQ_MSG_V01, QMI_IDL_TYPE16(0, 9), 5},
  {QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01, QMI_IDL_TYPE16(0, 11), 44},
  {QMI_NAS_INITIATE_NETWORK_REGISTER_REQ_MSG_V01, QMI_IDL_TYPE16(0, 13), 20},
  {QMI_NAS_INITIATE_ATTACH_REQ_MSG_V01, QMI_IDL_TYPE16(0, 15), 4},
  {QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01, QMI_IDL_TYPE16(0, 17), 0},
  {QMI_NAS_GET_HOME_NETWORK_REQ_MSG_V01, QMI_IDL_TYPE16(0, 20), 0},
  {QMI_NAS_GET_PREFERRED_NETWORKS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 22), 0},
  {QMI_NAS_SET_PREFERRED_NETWORKS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 24), 948},
  {QMI_NAS_GET_FORBIDDEN_NETWORKS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 26), 0},
  {QMI_NAS_SET_FORBIDDEN_NETWORKS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 28), 261},
  {QMI_NAS_SET_TECHNOLOGY_PREFERENCE_REQ_V01, QMI_IDL_TYPE16(0, 80), 6},
  {QMI_NAS_GET_TECHNOLOGY_PREFERENCE_REQ_V01, QMI_IDL_TYPE16(0, 82), 0},
  {QMI_NAS_GET_ACCOLC_REQ_MSG_V01, QMI_IDL_TYPE16(0, 30), 0},
  {QMI_NAS_SET_ACCOLC_REQ_MSG_V01, QMI_IDL_TYPE16(0, 32), 10},
  {QMI_NAS_GET_NETWORK_SYSTEM_PREFERENCE_REQ_V01, QMI_IDL_TYPE16(0, 84), 0},
  {QMI_NAS_GET_DEVICE_CONFIG_REQ_MSG_V01, QMI_IDL_TYPE16(0, 34), 0},
  {QMI_NAS_SET_DEVICE_CONFIG_REQ_MSG_V01, QMI_IDL_TYPE16(0, 36), 33},
  {QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 38), 0},
  {QMI_NAS_GET_AN_AAA_STATUS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 40), 0},
  {QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 42), 133},
  {QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 44), 0},
  {QMI_NAS_SET_DDTM_PREFERENCE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 47), 72},
  {QMI_NAS_GET_OPERATOR_NAME_DATA_REQ_MSG_V01, QMI_IDL_TYPE16(0, 50), 0},
  {QMI_NAS_GET_CSP_PLMN_MODE_BIT_REQ_MSG_V01, QMI_IDL_TYPE16(0, 53), 0},
  {QMI_NAS_UPDATE_AKEY_REQ_MSG_V01, QMI_IDL_TYPE16(0, 56), 29},
  {QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 58), 11},
  {QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 60), 194},
  {QMI_NAS_GET_MOB_CAI_REV_REQ_MSG_V01, QMI_IDL_TYPE16(0, 62), 0},
  {QMI_NAS_GET_RTRE_CONFIG_REQ_MSG_V01, QMI_IDL_TYPE16(0, 64), 0},
  {QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01, QMI_IDL_TYPE16(0, 66), 13},
  {QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 68), 0},
  {QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01, QMI_IDL_TYPE16(0, 70), 38},
  {QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01, QMI_IDL_TYPE16(0, 72), 4},
  {QMI_NAS_GET_MODE_PREF_REQ_MSG_V01, QMI_IDL_TYPE16(0, 78), 0},
  {QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01, QMI_IDL_TYPE16(0, 74), 27},
  {QMI_NAS_GET_SYS_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 87), 0},
  {QMI_NAS_GET_SIG_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 90), 0},
  {QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 92), 329},
  {QMI_NAS_GET_ERR_RATE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 95), 0},
  {QMI_NAS_GET_HDR_SUBTYPE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 100), 7},
  {QMI_NAS_GET_HDR_COLOR_CODE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 102), 0},
  {QMI_NAS_GET_CURRENT_ACQ_SYS_MODE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 104), 0},
  {QMI_NAS_SET_RX_DIVERSITY_REQ_MSG_V01, QMI_IDL_TYPE16(0, 106), 5},
  {QMI_NAS_GET_TX_RX_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 108), 4},
  {QMI_NAS_UPDATE_AKEY_EXT_REQ_MSG_V01, QMI_IDL_TYPE16(0, 110), 35},
  {QMI_NAS_GET_DUAL_STANDBY_PREF_REQ_MSG_V01, QMI_IDL_TYPE16(0, 113), 0},
  {QMI_NAS_DETACH_LTE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 115), 0},
  {QMI_NAS_BLOCK_LTE_PLMN_REQ_MSG_V01, QMI_IDL_TYPE16(0, 117), 22},
  {QMI_NAS_UNBLOCK_LTE_PLMN_REQ_MSG_V01, QMI_IDL_TYPE16(0, 119), 8},
  {QMI_NAS_RESET_LTE_PLMN_BLOCKING_REQ_MSG_V01, QMI_IDL_TYPE16(0, 121), 0},
  {QMI_NAS_CONFIG_EMBMS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 124), 9},
  {QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 126), 0},
  {QMI_NAS_GET_CDMA_POSITION_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 129), 0},
  {QMI_NAS_FORCE_NETWORK_SEARCH_REQ_MSG_V01, QMI_IDL_TYPE16(0, 132), 0},
  {QMI_NAS_GET_MANAGED_ROAMING_CONFIG_REQ_MSG_V01, QMI_IDL_TYPE16(0, 135), 0},
  {QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 138), 0},
  {QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01, QMI_IDL_TYPE16(0, 140), 1444},
  {QMI_NAS_GET_TDS_CELL_AND_POSITION_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 142), 0},
  {QMI_NAS_SET_HPLMN_IRAT_SEARCH_TIMER_REQ_MSG_V01, QMI_IDL_TYPE16(0, 144), 5},
  {QMI_NAS_GET_EMBMS_SIG_REQ_MSG_V01, QMI_IDL_TYPE16(0, 146), 5},
  {QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_REQ_MSG_V01, QMI_IDL_TYPE16(0, 148), 11},
  {QMI_NAS_GET_SYS_INFO_IND_REPORTING_LIMIT_REQ_MSG_V01, QMI_IDL_TYPE16(0, 150), 0},
  {QMI_NAS_UPDATE_IMS_STATUS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 152), 328},
  {QMI_NAS_GET_IMS_PREF_STATUS_REQ_MSG_V01, QMI_IDL_TYPE16(0, 154), 0},
  {QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01, QMI_IDL_TYPE16(0, 157), 4},
  {QMI_NAS_CDMA_AVOID_SYSTEM_REQ_MSG_V01, QMI_IDL_TYPE16(0, 159), 7},
  {QMI_NAS_GET_CDMA_AVOID_SYSTEM_LIST_REQ_MSG_V01, QMI_IDL_TYPE16(0, 161), 0},
  {QMI_NAS_SET_HPLMN_SEARCH_TIME_REQ_MSG_V01, QMI_IDL_TYPE16(0, 163), 7},
  {QMI_NAS_GET_HPLMN_SEARCH_TIME_REQ_MSG_V01, QMI_IDL_TYPE16(0, 165), 0},
  {QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01, QMI_IDL_TYPE16(0, 167), 0},
  {QMI_NAS_GET_NETWORK_TIME_REQ_MSG_V01, QMI_IDL_TYPE16(0, 169), 0},
  {QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01, QMI_IDL_TYPE16(0, 171), 0},
  {QMI_NAS_SET_LTE_BAND_PRIORITY_REQ_MSG_V01, QMI_IDL_TYPE16(0, 174), 260},
  {QMI_NAS_GET_EMBMS_SIG_EXT_REQ_MSG_V01, QMI_IDL_TYPE16(0, 176), 5},
  {QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_REQ_MSG_V01, QMI_IDL_TYPE16(0, 179), 0},
  {QMI_NAS_SET_BUILTIN_PLMN_LIST_REQ_MSG_V01, QMI_IDL_TYPE16(0, 181), 2520},
  {QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_REQ_MSG_V01, QMI_IDL_TYPE16(0, 184), 44}
};

static const qmi_idl_service_message_table_entry nas_service_response_messages_v01[] = {
  {QMI_NAS_RESET_RESP_MSG_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_NAS_ABORT_RESP_MSG_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01, QMI_IDL_TYPE16(0, 5), 7},
  {QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01, QMI_IDL_TYPE16(0, 8), 7},
  {QMI_NAS_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_NAS_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_NAS_GET_SIGNAL_STRENGTH_RESP_MSG_V01, QMI_IDL_TYPE16(0, 10), 136},
  {QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01, QMI_IDL_TYPE16(0, 12), 15717},
  {QMI_NAS_INITIATE_NETWORK_REGISTER_RESP_MSG_V01, QMI_IDL_TYPE16(0, 14), 7},
  {QMI_NAS_INITIATE_ATTACH_RESP_MSG_V01, QMI_IDL_TYPE16(0, 16), 7},
  {QMI_NAS_GET_SERVING_SYSTEM_RESP_MSG_V01, QMI_IDL_TYPE16(0, 18), 676},
  {QMI_NAS_GET_HOME_NETWORK_RESP_MSG_V01, QMI_IDL_TYPE16(0, 21), 547},
  {QMI_NAS_GET_PREFERRED_NETWORKS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 23), 1625},
  {QMI_NAS_SET_PREFERRED_NETWORKS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 25), 7},
  {QMI_NAS_GET_FORBIDDEN_NETWORKS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 27), 268},
  {QMI_NAS_SET_FORBIDDEN_NETWORKS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 29), 7},
  {QMI_NAS_SET_TECHNOLOGY_PREFERENCE_RESP_V01, QMI_IDL_TYPE16(0, 81), 7},
  {QMI_NAS_GET_TECHNOLOGY_PREFERENCE_RESP_V01, QMI_IDL_TYPE16(0, 83), 18},
  {QMI_NAS_GET_ACCOLC_RESP_MSG_V01, QMI_IDL_TYPE16(0, 31), 11},
  {QMI_NAS_SET_ACCOLC_RESP_MSG_V01, QMI_IDL_TYPE16(0, 33), 7},
  {QMI_NAS_GET_NETWORK_SYSTEM_PREFERENCE_RESP_V01, QMI_IDL_TYPE16(0, 85), 11},
  {QMI_NAS_GET_DEVICE_CONFIG_RESP_MSG_V01, QMI_IDL_TYPE16(0, 35), 49},
  {QMI_NAS_SET_DEVICE_CONFIG_RESP_MSG_V01, QMI_IDL_TYPE16(0, 37), 7},
  {QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 39), 91},
  {QMI_NAS_GET_AN_AAA_STATUS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 41), 11},
  {QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 43), 7},
  {QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 45), 126},
  {QMI_NAS_SET_DDTM_PREFERENCE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 48), 7},
  {QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG_V01, QMI_IDL_TYPE16(0, 51), 36643},
  {QMI_NAS_GET_CSP_PLMN_MODE_BIT_RESP_MSG_V01, QMI_IDL_TYPE16(0, 54), 11},
  {QMI_NAS_UPDATE_AKEY_RESP_MSG_V01, QMI_IDL_TYPE16(0, 57), 7},
  {QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 59), 188},
  {QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 61), 7},
  {QMI_NAS_GET_MOB_CAI_REV_RESP_MSG_V01, QMI_IDL_TYPE16(0, 63), 11},
  {QMI_NAS_GET_RTRE_CONFIG_RESP_MSG_V01, QMI_IDL_TYPE16(0, 65), 15},
  {QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01, QMI_IDL_TYPE16(0, 67), 7},
  {QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 69), 2655},
  {QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01, QMI_IDL_TYPE16(0, 71), 3188},
  {QMI_NAS_BIND_SUBSCRIPTION_RESP_MSG_V01, QMI_IDL_TYPE16(0, 73), 7},
  {QMI_NAS_GET_MODE_PREF_RESP_MSG_V01, QMI_IDL_TYPE16(0, 79), 22},
  {QMI_NAS_DUAL_STANDBY_PREF_RESP_MSG_V01, QMI_IDL_TYPE16(0, 75), 7},
  {QMI_NAS_GET_SYS_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 88), 737},
  {QMI_NAS_GET_SIG_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 91), 66},
  {QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 93), 7},
  {QMI_NAS_GET_ERR_RATE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 96), 29},
  {QMI_NAS_GET_HDR_SUBTYPE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 101), 12},
  {QMI_NAS_GET_HDR_COLOR_CODE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 103), 11},
  {QMI_NAS_GET_CURRENT_ACQ_SYS_MODE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 105), 49},
  {QMI_NAS_SET_RX_DIVERSITY_RESP_MSG_V01, QMI_IDL_TYPE16(0, 107), 7},
  {QMI_NAS_GET_TX_RX_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 109), 63},
  {QMI_NAS_UPDATE_AKEY_EXT_RESP_V01, QMI_IDL_TYPE16(0, 111), 7},
  {QMI_NAS_GET_DUAL_STANDBY_PREF_RESP_MSG_V01, QMI_IDL_TYPE16(0, 114), 38},
  {QMI_NAS_DETACH_LTE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 116), 7},
  {QMI_NAS_BLOCK_LTE_PLMN_RESP_MSG_V01, QMI_IDL_TYPE16(0, 118), 7},
  {QMI_NAS_UNBLOCK_LTE_PLMN_RESP_MSG_V01, QMI_IDL_TYPE16(0, 120), 7},
  {QMI_NAS_RESET_LTE_PLMN_BLOCKING_RESP_MSG_V01, QMI_IDL_TYPE16(0, 122), 7},
  {QMI_NAS_CONFIG_EMBMS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 125), 12},
  {QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 127), 16},
  {QMI_NAS_GET_CDMA_POSITION_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 130), 312},
  {QMI_NAS_FORCE_NETWORK_SEARCH_RESP_MSG_V01, QMI_IDL_TYPE16(0, 133), 7},
  {QMI_NAS_GET_MANAGED_ROAMING_CONFIG_RESP_MSG_V01, QMI_IDL_TYPE16(0, 136), 11},
  {QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 139), 11},
  {QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01, QMI_IDL_TYPE16(0, 141), 7},
  {QMI_NAS_GET_TDS_CELL_AND_POSITION_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 143), 93},
  {QMI_NAS_SET_HPLMN_IRAT_SEARCH_TIMER_RESP_MSG_V01, QMI_IDL_TYPE16(0, 145), 7},
  {QMI_NAS_GET_EMBMS_SIG_RESP_MSG_V01, QMI_IDL_TYPE16(0, 147), 64},
  {QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_RESP_MSG_V01, QMI_IDL_TYPE16(0, 149), 7},
  {QMI_NAS_GET_SYS_INFO_IND_REPORTING_LIMIT_RESP_MSG_V01, QMI_IDL_TYPE16(0, 151), 18},
  {QMI_NAS_UPDATE_IMS_STATUS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 153), 7},
  {QMI_NAS_GET_IMS_PREF_STATUS_RESP_MSG_V01, QMI_IDL_TYPE16(0, 155), 19},
  {QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01, QMI_IDL_TYPE16(0, 158), 7},
  {QMI_NAS_CDMA_AVOID_SYSTEM_RESP_MSG_V01, QMI_IDL_TYPE16(0, 160), 7},
  {QMI_NAS_GET_CDMA_AVOID_SYSTEM_LIST_RESP_MSG_V01, QMI_IDL_TYPE16(0, 162), 91},
  {QMI_NAS_SET_HPLMN_SEARCH_TIME_RESP_MSG_V01, QMI_IDL_TYPE16(0, 164), 7},
  {QMI_NAS_GET_HPLMN_SEARCH_TIME_RESP_MSG_V01, QMI_IDL_TYPE16(0, 166), 14},
  {QMI_NAS_GET_SUBSCRIPTION_INFO_RESP_MSG_V01, QMI_IDL_TYPE16(0, 168), 26},
  {QMI_NAS_GET_NETWORK_TIME_RESP_MSG_V01, QMI_IDL_TYPE16(0, 170), 35},
  {QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01, QMI_IDL_TYPE16(0, 172), 50},
  {QMI_NAS_SET_LTE_BAND_PRIORITY_RESP_MSG_V01, QMI_IDL_TYPE16(0, 175), 7},
  {QMI_NAS_GET_EMBMS_SIG_EXT_RESP_MSG_V01, QMI_IDL_TYPE16(0, 177), 2400},
  {QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_RESP_MSG_V01, QMI_IDL_TYPE16(0, 180), 527},
  {QMI_NAS_SET_BUILTIN_PLMN_LIST_RESP_MSG_V01, QMI_IDL_TYPE16(0, 182), 7},
  {QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_RESP_MSG_V01, QMI_IDL_TYPE16(0, 185), 7}
};

static const qmi_idl_service_message_table_entry nas_service_indication_messages_v01[] = {
  {QMI_NAS_EVENT_REPORT_IND_MSG_V01, QMI_IDL_TYPE16(0, 6), 1332},
  {QMI_NAS_SERVING_SYSTEM_IND_MSG_V01, QMI_IDL_TYPE16(0, 19), 688},
  {QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01, QMI_IDL_TYPE16(0, 46), 119},
  {QMI_NAS_DDTM_IND_MSG_V01, QMI_IDL_TYPE16(0, 49), 73},
  {QMI_NAS_OPERATOR_NAME_DATA_IND_MSG_V01, QMI_IDL_TYPE16(0, 52), 36636},
  {QMI_NAS_CSP_PLMN_MODE_BIT_IND_MSG_V01, QMI_IDL_TYPE16(0, 55), 4},
  {QMI_NAS_MANAGED_ROAMING_IND_MSG_V01, QMI_IDL_TYPE16(0, 112), 4},
  {QMI_NAS_DUAL_STANDBY_PREF_IND_MSG_V01, QMI_IDL_TYPE16(0, 76), 22},
  {QMI_NAS_SUBSCRIPTION_INFO_IND_MSG_V01, QMI_IDL_TYPE16(0, 77), 19},
  {QMI_NAS_NETWORK_TIME_IND_MSG_V01, QMI_IDL_TYPE16(0, 86), 23},
  {QMI_NAS_SYS_INFO_IND_MSG_V01, QMI_IDL_TYPE16(0, 89), 734},
  {QMI_NAS_SIG_INFO_IND_MSG_V01, QMI_IDL_TYPE16(0, 94), 59},
  {QMI_NAS_ERR_RATE_IND_MSG_V01, QMI_IDL_TYPE16(0, 97), 22},
  {QMI_NAS_HDR_SESSION_CLOSE_IND_MSG_V01, QMI_IDL_TYPE16(0, 98), 7},
  {QMI_NAS_HDR_UATI_UPDATE_IND_MSG_V01, QMI_IDL_TYPE16(0, 99), 19},
  {QMI_NAS_CURRENT_PLMN_NAME_IND_V01, QMI_IDL_TYPE16(0, 123), 3206},
  {QMI_NAS_EMBMS_STATUS_IND_V01, QMI_IDL_TYPE16(0, 128), 9},
  {QMI_NAS_RF_BAND_INFO_IND_V01, QMI_IDL_TYPE16(0, 131), 8},
  {QMI_NAS_NETWORK_REJECT_IND_V01, QMI_IDL_TYPE16(0, 134), 27},
  {QMI_NAS_RTRE_CONFIG_IND_V01, QMI_IDL_TYPE16(0, 137), 8},
  {QMI_NAS_IMS_PREF_STATUS_IND_V01, QMI_IDL_TYPE16(0, 156), 12},
  {QMI_NAS_LTE_SIB16_NETWORK_TIME_IND_V01, QMI_IDL_TYPE16(0, 173), 43},
  {QMI_NAS_LTE_CPHY_CA_IND_V01, QMI_IDL_TYPE16(0, 178), 11},
  {QMI_NAS_SET_BUILTIN_PLMN_LIST_IND_MSG_V01, QMI_IDL_TYPE16(0, 183), 26},
  {QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_IND_MSG_V01, QMI_IDL_TYPE16(0, 186), 14896}
};

/*Service Object*/
struct qmi_idl_service_object nas_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x03,
  36643,
  { sizeof(nas_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(nas_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(nas_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { nas_service_command_messages_v01, nas_service_response_messages_v01, nas_service_indication_messages_v01},
  &nas_qmi_idl_type_table_object_v01,
  0x5A,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type nas_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( NAS_V01_IDL_MAJOR_VERS != idl_maj_version || NAS_V01_IDL_MINOR_VERS != idl_min_version 
       || NAS_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&nas_qmi_idl_service_object_v01;
}

