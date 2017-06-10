/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        D E V I C E _ M A N A G E M E N T _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the dms service Data structures.

  Copyright (c) 2006-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Fri Oct 25 2013 (Spin 0)
   From IDL File: device_management_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "device_management_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t dms_battery_lvl_limits_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_battery_lvl_limits_type_v01, battery_lvl_lower_limit),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_battery_lvl_limits_type_v01, battery_lvl_upper_limit),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_power_state_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_power_state_type_v01, power_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_power_state_type_v01, battery_lvl),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_pin_status_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_pin_status_type_v01, status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_status_type_v01, verify_retries_left),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_status_type_v01, unblock_retries_left),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_subs_config_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_subs_config_type_v01, max_active),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_subs_config_type_v01, subscription_list),
  QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01,
  QMI_IDL_OFFSET8(dms_subs_config_type_v01, subscription_list) - QMI_IDL_OFFSET8(dms_subs_config_type_v01, subscription_list_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_multisim_capability_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_multisim_capability_type_v01, max_subscriptions),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_multisim_capability_type_v01, subscription_config_list),
  QMI_DMS_MAX_CONFIG_LIST_LEN_V01,
  QMI_IDL_OFFSET8(dms_multisim_capability_type_v01, subscription_config_list) - QMI_IDL_OFFSET8(dms_multisim_capability_type_v01, subscription_config_list_len),
  QMI_IDL_TYPE88(0, 3),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_device_capabilities_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_device_capabilities_type_v01, max_tx_channel_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_device_capabilities_type_v01, max_rx_channel_rate),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_device_capabilities_type_v01, data_service_capability),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_device_capabilities_type_v01, sim_capability),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_device_capabilities_type_v01, radio_if_list),
  QMI_DMS_RADIO_IF_LIST_MAX_V01,
  QMI_IDL_OFFSET8(dms_device_capabilities_type_v01, radio_if_list) - QMI_IDL_OFFSET8(dms_device_capabilities_type_v01, radio_if_list_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_pin_protection_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_pin_protection_info_type_v01, pin_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_protection_info_type_v01, protection_setting_enabled),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_protection_info_type_v01, pin_value),
  QMI_DMS_PIN_VALUE_MAX_V01,
  QMI_IDL_OFFSET8(dms_pin_protection_info_type_v01, pin_value) - QMI_IDL_OFFSET8(dms_pin_protection_info_type_v01, pin_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_pin_retries_status_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_retries_status_type_v01, verify_retries_left),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_retries_status_type_v01, unblock_retries_left),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_pin_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_pin_info_type_v01, pin_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_info_type_v01, pin_value),
  QMI_DMS_PIN_VALUE_MAX_V01,
  QMI_IDL_OFFSET8(dms_pin_info_type_v01, pin_value) - QMI_IDL_OFFSET8(dms_pin_info_type_v01, pin_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_pin_unblock_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_pin_unblock_info_type_v01, unblock_pin_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_unblock_info_type_v01, puk_value),
  QMI_DMS_PUK_VALUE_MAX_V01,
  QMI_IDL_OFFSET8(dms_pin_unblock_info_type_v01, puk_value) - QMI_IDL_OFFSET8(dms_pin_unblock_info_type_v01, puk_value_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_unblock_info_type_v01, new_pin_value),
  QMI_DMS_PUK_VALUE_MAX_V01,
  QMI_IDL_OFFSET8(dms_pin_unblock_info_type_v01, new_pin_value) - QMI_IDL_OFFSET8(dms_pin_unblock_info_type_v01, new_pin_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_pin_change_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_pin_change_info_type_v01, pin_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_change_info_type_v01, old_pin_value),
  QMI_DMS_PIN_VALUE_MAX_V01,
  QMI_IDL_OFFSET8(dms_pin_change_info_type_v01, old_pin_value) - QMI_IDL_OFFSET8(dms_pin_change_info_type_v01, old_pin_value_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pin_change_info_type_v01, new_pin_value),
  QMI_DMS_PIN_VALUE_MAX_V01,
  QMI_IDL_OFFSET8(dms_pin_change_info_type_v01, new_pin_value) - QMI_IDL_OFFSET8(dms_pin_change_info_type_v01, new_pin_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_device_time_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_device_time_type_v01, time_count),
  6,

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_device_time_type_v01, time_source),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_activation_code_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_activation_code_type_v01, act_code),
  QMI_DMS_ACTIVATION_CODE_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_manual_act_data_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_manual_act_data_type_v01, spc),
  QMI_DMS_SPC_LEN_V01,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dms_manual_act_data_type_v01, sid),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_manual_act_data_type_v01, mdn),
  QMI_DMS_MDN_MAX_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_manual_act_data_type_v01, min),
  QMI_DMS_MIN_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_mn_ha_key_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_mn_ha_key_type_v01, mn_ha_key),
  QMI_DMS_HA_KEY_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_mn_aaa_key_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_mn_aaa_key_type_v01, mn_aaa_key),
  QMI_DMS_AAA_KEY_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_pref_roaming_list_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dms_pref_roaming_list_type_v01, prl_total_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_FLAGS_FIRST_EXTENDED |  QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_FLAGS_ARRAY_LENGTH_ONLY,
  QMI_IDL_OFFSET8(dms_pref_roaming_list_type_v01, prl),
  ((QMI_DMS_PRL_DATA_MAX_V01) & 0xFF), ((QMI_DMS_PRL_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(dms_pref_roaming_list_type_v01, prl) - QMI_IDL_OFFSET8(dms_pref_roaming_list_type_v01, prl_len),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_pref_roaming_list_type_v01, prl_seg_num),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_FLAGS_FIRST_EXTENDED |  QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_FLAGS_ARRAY_DATA_ONLY,
  QMI_IDL_OFFSET8(dms_pref_roaming_list_type_v01, prl),
  ((QMI_DMS_PRL_DATA_MAX_V01) & 0xFF), ((QMI_DMS_PRL_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(dms_pref_roaming_list_type_v01, prl) - QMI_IDL_OFFSET8(dms_pref_roaming_list_type_v01, prl_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_user_lock_state_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_user_lock_state_info_type_v01, lock_state),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_user_lock_state_info_type_v01, lock_code),
  QMI_DMS_LOCK_CODE_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_user_lock_set_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_user_lock_set_info_type_v01, cur_code),
  QMI_DMS_LOCK_CODE_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_user_lock_set_info_type_v01, new_code),
  QMI_DMS_LOCK_CODE_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_user_data_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_user_data_type_v01, data),
  ((QMI_DMS_USER_DATA_MAX_V01) & 0xFF), ((QMI_DMS_USER_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(dms_user_data_type_v01, data) - QMI_IDL_OFFSET8(dms_user_data_type_v01, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_eri_data_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_eri_data_type_v01, eri_data),
  ((QMI_DMS_ERI_DATA_MAX_V01) & 0xFF), ((QMI_DMS_ERI_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(dms_eri_data_type_v01, eri_data) - QMI_IDL_OFFSET8(dms_eri_data_type_v01, eri_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_facility_state_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_facility_state_info_type_v01, facility_state),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_facility_state_info_type_v01, verify_reties_left),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_facility_state_info_type_v01, unblock_retries_left),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_facility_set_ck_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_facility_set_ck_info_type_v01, facility),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_facility_set_ck_info_type_v01, facility_state),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_facility_set_ck_info_type_v01, facility_ck),
  QMI_DMS_FACILITY_CK_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dms_facility_unblock_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_facility_unblock_info_type_v01, facility),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_facility_unblock_info_type_v01, facility_unblock_ck),
  QMI_DMS_FACILITY_UNBLOCK_CK_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * dms_reset_req_msg is empty
 * static const uint8_t dms_reset_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_reset_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_reset_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_set_event_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_power_state) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_power_state_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_power_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, lvl_limits) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, lvl_limits_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, lvl_limits),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_pin_state) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_pin_state_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_pin_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_activation_state) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_activation_state_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_activation_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_oprt_mode_state) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_oprt_mode_state_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_oprt_mode_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_uim_state) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_uim_state_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_uim_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_wireless_disable_state) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_wireless_disable_state_valid)),
  0x16,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_wireless_disable_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_prl_init) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_prl_init_valid)),
  0x17,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_prl_init),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_cdma_lock_mode) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_cdma_lock_mode_valid)),
  0x18,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_cdma_lock_mode),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_device_multisim_info) - QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_device_multisim_info_valid)),
  0x19,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_event_report_req_msg_v01, report_device_multisim_info)
};

static const uint8_t dms_set_event_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_event_report_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_event_report_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, power_state) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, power_state_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, power_state),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, pin1_status) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, pin1_status_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, pin1_status),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, pin2_status) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, pin2_status_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, pin2_status),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, activation_state) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, activation_state_valid)),
  0x13,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, activation_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, operating_mode) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, operating_mode_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, operating_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, uim_state) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, uim_state_valid)),
  0x15,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, uim_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, wireless_disable_state) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, wireless_disable_state_valid)),
  0x16,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, wireless_disable_state),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, prl_init) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, prl_init_valid)),
  0x17,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, prl_init),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, cdma_lock_mode_state) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, cdma_lock_mode_state_valid)),
  0x18,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, cdma_lock_mode_state),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, multisim_capability) - QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, multisim_capability_valid)),
  0x19,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_event_report_ind_msg_v01, multisim_capability),
  QMI_IDL_TYPE88(0, 4)
};

/* 
 * dms_get_device_cap_req_msg is empty
 * static const uint8_t dms_get_device_cap_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_device_cap_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, device_capabilities),
  QMI_IDL_TYPE88(0, 5),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, device_service_capability) - QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, device_service_capability_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, device_service_capability),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, voice_support_capability) - QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, voice_support_capability_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, voice_support_capability),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, simul_voice_and_data_capability) - QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, simul_voice_and_data_capability_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, simul_voice_and_data_capability),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, multisim_capability) - QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, multisim_capability_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_device_cap_resp_msg_v01, multisim_capability),
  QMI_IDL_TYPE88(0, 4)
};

/* 
 * dms_get_device_mfr_req_msg is empty
 * static const uint8_t dms_get_device_mfr_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_device_mfr_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_device_mfr_resp_msg_v01, device_manufacturer),
  QMI_DMS_DEVICE_MANUFACTURER_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_device_mfr_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_device_model_id_req_msg is empty
 * static const uint8_t dms_get_device_model_id_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_device_model_id_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_device_model_id_resp_msg_v01, device_model_id),
  ((QMI_DMS_DEVICE_MODEL_ID_MAX_V01) & 0xFF), ((QMI_DMS_DEVICE_MODEL_ID_MAX_V01) >> 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dms_get_device_model_id_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_device_rev_id_req_msg is empty
 * static const uint8_t dms_get_device_rev_id_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_device_rev_id_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_device_rev_id_resp_msg_v01, device_rev_id),
  ((QMI_DMS_DEVICE_REV_ID_MAX_V01) & 0xFF), ((QMI_DMS_DEVICE_REV_ID_MAX_V01) >> 8),

  0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dms_get_device_rev_id_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dms_get_device_rev_id_resp_msg_v01, boot_code_rev) - QMI_IDL_OFFSET16RELATIVE(dms_get_device_rev_id_resp_msg_v01, boot_code_rev_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(dms_get_device_rev_id_resp_msg_v01, boot_code_rev),
  QMI_DMS_BOOT_CODE_REV_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dms_get_device_rev_id_resp_msg_v01, pri_rev) - QMI_IDL_OFFSET16RELATIVE(dms_get_device_rev_id_resp_msg_v01, pri_rev_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(dms_get_device_rev_id_resp_msg_v01, pri_rev),
  QMI_DMS_PRI_REV_MAX_V01
};

/* 
 * dms_get_msisdn_req_msg is empty
 * static const uint8_t dms_get_msisdn_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_msisdn_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_msisdn_resp_msg_v01, voice_number),
  QMI_DMS_VOICE_NUMBER_MAX_V01,

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_msisdn_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_msisdn_resp_msg_v01, mobile_id_number) - QMI_IDL_OFFSET8(dms_get_msisdn_resp_msg_v01, mobile_id_number_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_msisdn_resp_msg_v01, mobile_id_number),
  QMI_DMS_MOBILE_ID_NUMBER_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_msisdn_resp_msg_v01, imsi) - QMI_IDL_OFFSET8(dms_get_msisdn_resp_msg_v01, imsi_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_msisdn_resp_msg_v01, imsi),
  QMI_DMS_IMSI_MAX_V01
};

/* 
 * dms_get_device_serial_numbers_req_msg is empty
 * static const uint8_t dms_get_device_serial_numbers_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_device_serial_numbers_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, esn) - QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, esn_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, esn),
  QMI_DMS_ESN_MAX_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, imei) - QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, imei_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, imei),
  QMI_DMS_IMEI_MAX_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, meid) - QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, meid_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, meid),
  QMI_DMS_MEID_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, imeisv_svn) - QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, imeisv_svn_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_device_serial_numbers_resp_msg_v01, imeisv_svn),
  QMI_DMS_IMEISV_MAX_V01
};

/* 
 * dms_get_power_state_req_msg is empty
 * static const uint8_t dms_get_power_state_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_power_state_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_power_state_resp_msg_v01, power_state),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_power_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_uim_set_pin_protection_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_set_pin_protection_req_msg_v01, pin_protection_info),
  QMI_IDL_TYPE88(0, 6)
};

static const uint8_t dms_uim_set_pin_protection_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_set_pin_protection_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_set_pin_protection_resp_msg_v01, pin_retries_status) - QMI_IDL_OFFSET8(dms_uim_set_pin_protection_resp_msg_v01, pin_retries_status_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_set_pin_protection_resp_msg_v01, pin_retries_status),
  QMI_IDL_TYPE88(0, 7)
};

static const uint8_t dms_uim_verify_pin_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_verify_pin_req_msg_v01, pin_info),
  QMI_IDL_TYPE88(0, 8)
};

static const uint8_t dms_uim_verify_pin_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_verify_pin_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_verify_pin_resp_msg_v01, pin_retries_status) - QMI_IDL_OFFSET8(dms_uim_verify_pin_resp_msg_v01, pin_retries_status_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_verify_pin_resp_msg_v01, pin_retries_status),
  QMI_IDL_TYPE88(0, 7)
};

static const uint8_t dms_uim_unblock_pin_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_unblock_pin_req_msg_v01, pin_unblock_info),
  QMI_IDL_TYPE88(0, 9)
};

static const uint8_t dms_uim_unblock_pin_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_unblock_pin_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_unblock_pin_resp_msg_v01, pin_retries_status) - QMI_IDL_OFFSET8(dms_uim_unblock_pin_resp_msg_v01, pin_retries_status_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_unblock_pin_resp_msg_v01, pin_retries_status),
  QMI_IDL_TYPE88(0, 7)
};

static const uint8_t dms_uim_change_pin_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_change_pin_req_msg_v01, pin_change_info),
  QMI_IDL_TYPE88(0, 10)
};

static const uint8_t dms_uim_change_pin_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_change_pin_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_change_pin_resp_msg_v01, pin_retries_status) - QMI_IDL_OFFSET8(dms_uim_change_pin_resp_msg_v01, pin_retries_status_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_change_pin_resp_msg_v01, pin_retries_status),
  QMI_IDL_TYPE88(0, 7)
};

/* 
 * dms_uim_get_pin_status_req_msg is empty
 * static const uint8_t dms_uim_get_pin_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_uim_get_pin_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_get_pin_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_get_pin_status_resp_msg_v01, pin1_status) - QMI_IDL_OFFSET8(dms_uim_get_pin_status_resp_msg_v01, pin1_status_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_get_pin_status_resp_msg_v01, pin1_status),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_get_pin_status_resp_msg_v01, pin2_status) - QMI_IDL_OFFSET8(dms_uim_get_pin_status_resp_msg_v01, pin2_status_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_get_pin_status_resp_msg_v01, pin2_status),
  QMI_IDL_TYPE88(0, 2)
};

/* 
 * dms_get_device_hardware_rev_req_msg is empty
 * static const uint8_t dms_get_device_hardware_rev_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_device_hardware_rev_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_device_hardware_rev_resp_msg_v01, hardware_rev),
  ((QMI_DMS_HARDWARE_REV_MAX_V01) & 0xFF), ((QMI_DMS_HARDWARE_REV_MAX_V01) >> 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dms_get_device_hardware_rev_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_operating_mode_req_msg is empty
 * static const uint8_t dms_get_operating_mode_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_operating_mode_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_get_operating_mode_resp_msg_v01, operating_mode),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_operating_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_operating_mode_resp_msg_v01, offline_reason) - QMI_IDL_OFFSET8(dms_get_operating_mode_resp_msg_v01, offline_reason_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dms_get_operating_mode_resp_msg_v01, offline_reason),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_operating_mode_resp_msg_v01, hardware_controlled_mode) - QMI_IDL_OFFSET8(dms_get_operating_mode_resp_msg_v01, hardware_controlled_mode_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_get_operating_mode_resp_msg_v01, hardware_controlled_mode)
};

static const uint8_t dms_set_operating_mode_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_set_operating_mode_req_msg_v01, operating_mode)
};

static const uint8_t dms_set_operating_mode_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_operating_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_time_req_msg is empty
 * static const uint8_t dms_get_time_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_time_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_time_resp_msg_v01, device_time),
  QMI_IDL_TYPE88(0, 11),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_time_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_time_resp_msg_v01, sys_time_in_ms) - QMI_IDL_OFFSET8(dms_get_time_resp_msg_v01, sys_time_in_ms_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_get_time_resp_msg_v01, sys_time_in_ms),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_time_resp_msg_v01, user_time_in_ms) - QMI_IDL_OFFSET8(dms_get_time_resp_msg_v01, user_time_in_ms_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_get_time_resp_msg_v01, user_time_in_ms)
};

/* 
 * dms_get_prl_ver_req_msg is empty
 * static const uint8_t dms_get_prl_ver_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_prl_ver_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dms_get_prl_ver_resp_msg_v01, prl_version),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_prl_ver_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_prl_ver_resp_msg_v01, prl_only) - QMI_IDL_OFFSET8(dms_get_prl_ver_resp_msg_v01, prl_only_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_get_prl_ver_resp_msg_v01, prl_only)
};

/* 
 * dms_get_activation_state_req_msg is empty
 * static const uint8_t dms_get_activation_state_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_activation_state_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_get_activation_state_resp_msg_v01, activation_state),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_activation_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_activate_automatic_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_activate_automatic_req_msg_v01, activation_code),
  QMI_IDL_TYPE88(0, 12)
};

static const uint8_t dms_activate_automatic_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_activate_automatic_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_activate_manual_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, activation_data),
  QMI_IDL_TYPE88(0, 13),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, mn_ha_key) - QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, mn_ha_key_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, mn_ha_key),
  QMI_IDL_TYPE88(0, 14),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, mn_aaa_key) - QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, mn_aaa_key_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, mn_aaa_key),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, pref_roaming_list) - QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, pref_roaming_list_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_activate_manual_req_msg_v01, pref_roaming_list),
  QMI_IDL_TYPE88(0, 16)
};

static const uint8_t dms_activate_manual_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_activate_manual_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_user_lock_state_req_msg is empty
 * static const uint8_t dms_get_user_lock_state_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_user_lock_state_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_get_user_lock_state_resp_msg_v01, lock_enabled),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_user_lock_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_set_user_lock_state_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_user_lock_state_req_msg_v01, lock_info),
  QMI_IDL_TYPE88(0, 17)
};

static const uint8_t dms_set_user_lock_state_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_user_lock_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_set_user_lock_code_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_user_lock_code_req_msg_v01, lock_info),
  QMI_IDL_TYPE88(0, 18)
};

static const uint8_t dms_set_user_lock_code_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_user_lock_code_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_read_user_data_req_msg is empty
 * static const uint8_t dms_read_user_data_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_read_user_data_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_read_user_data_resp_msg_v01, user_data),
  QMI_IDL_TYPE88(0, 19),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dms_read_user_data_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_write_user_data_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_write_user_data_req_msg_v01, user_data),
  QMI_IDL_TYPE88(0, 19)
};

static const uint8_t dms_write_user_data_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_write_user_data_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_read_eri_file_req_msg is empty
 * static const uint8_t dms_read_eri_file_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_read_eri_file_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_read_eri_file_resp_msg_v01, eri_file),
  QMI_IDL_TYPE88(0, 20),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dms_read_eri_file_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_restore_factory_defaults_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_restore_factory_defaults_req_msg_v01, spc),
  QMI_DMS_SPC_LEN_V01
};

static const uint8_t dms_restore_factory_defaults_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_restore_factory_defaults_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_validate_service_programming_code_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_validate_service_programming_code_req_msg_v01, spc),
  QMI_DMS_SPC_LEN_V01
};

static const uint8_t dms_validate_service_programming_code_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_validate_service_programming_code_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_uim_get_iccid_req_msg is empty
 * static const uint8_t dms_uim_get_iccid_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_uim_get_iccid_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_uim_get_iccid_resp_msg_v01, uim_id),
  QMI_DMS_UIM_ID_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_get_iccid_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_uim_get_ck_status_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_uim_get_ck_status_req_msg_v01, facility)
};

static const uint8_t dms_uim_get_ck_status_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_get_ck_status_resp_msg_v01, facility_info),
  QMI_IDL_TYPE88(0, 21),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_get_ck_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_get_ck_status_resp_msg_v01, operation_blocking) - QMI_IDL_OFFSET8(dms_uim_get_ck_status_resp_msg_v01, operation_blocking_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_uim_get_ck_status_resp_msg_v01, operation_blocking)
};

static const uint8_t dms_uim_set_ck_protection_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_set_ck_protection_req_msg_v01, facility_set_ck_info),
  QMI_IDL_TYPE88(0, 22)
};

static const uint8_t dms_uim_set_ck_protection_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_set_ck_protection_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_set_ck_protection_resp_msg_v01, verify_retries_left) - QMI_IDL_OFFSET8(dms_uim_set_ck_protection_resp_msg_v01, verify_retries_left_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_uim_set_ck_protection_resp_msg_v01, verify_retries_left)
};

static const uint8_t dms_uim_unblock_ck_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_unblock_ck_req_msg_v01, facility_unblock_info),
  QMI_IDL_TYPE88(0, 23)
};

static const uint8_t dms_uim_unblock_ck_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_unblock_ck_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_uim_unblock_ck_resp_msg_v01, unblock_retries_left) - QMI_IDL_OFFSET8(dms_uim_unblock_ck_resp_msg_v01, unblock_retries_left_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_uim_unblock_ck_resp_msg_v01, unblock_retries_left)
};

/* 
 * dms_uim_get_imsi_req_msg is empty
 * static const uint8_t dms_uim_get_imsi_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_uim_get_imsi_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_uim_get_imsi_resp_msg_v01, imsi),
  QMI_DMS_IMSI_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_get_imsi_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_uim_get_state_req_msg is empty
 * static const uint8_t dms_uim_get_state_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_uim_get_state_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dms_uim_get_state_resp_msg_v01, uim_state),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_uim_get_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_band_capability_req_msg is empty
 * static const uint8_t dms_get_band_capability_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_band_capability_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_get_band_capability_resp_msg_v01, band_capability),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_band_capability_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_band_capability_resp_msg_v01, lte_band_capability) - QMI_IDL_OFFSET8(dms_get_band_capability_resp_msg_v01, lte_band_capability_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_get_band_capability_resp_msg_v01, lte_band_capability),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_band_capability_resp_msg_v01, tds_band_capability) - QMI_IDL_OFFSET8(dms_get_band_capability_resp_msg_v01, tds_band_capability_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_get_band_capability_resp_msg_v01, tds_band_capability)
};

/* 
 * dms_get_factory_sku_req_msg is empty
 * static const uint8_t dms_get_factory_sku_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_factory_sku_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_factory_sku_resp_msg_v01, factory_serial_number),
  QMI_DMS_FACTORY_SN_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_factory_sku_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_set_time_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dms_set_time_req_msg_v01, time_in_ms),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_time_req_msg_v01, time_reference_type) - QMI_IDL_OFFSET8(dms_set_time_req_msg_v01, time_reference_type_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_set_time_req_msg_v01, time_reference_type)
};

static const uint8_t dms_set_time_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_time_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_alt_net_config_req_msg is empty
 * static const uint8_t dms_get_alt_net_config_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_alt_net_config_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_get_alt_net_config_resp_msg_v01, alt_net_config),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_alt_net_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_set_alt_net_config_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_alt_net_config_req_msg_v01, alt_net_config)
};

static const uint8_t dms_set_alt_net_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_alt_net_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_sw_version_req_msg is empty
 * static const uint8_t dms_get_sw_version_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_sw_version_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_get_sw_version_resp_msg_v01, sw_version),
  QMI_DMS_SW_VERSION_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_sw_version_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dms_set_spc_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_spc_req_msg_v01, curr_spc),
  QMI_DMS_SPC_LEN_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_set_spc_req_msg_v01, new_spc),
  QMI_DMS_SPC_LEN_V01
};

static const uint8_t dms_set_spc_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_spc_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_current_prl_info_req_msg is empty
 * static const uint8_t dms_get_current_prl_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_current_prl_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_current_prl_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_current_prl_info_resp_msg_v01, prl_version) - QMI_IDL_OFFSET8(dms_get_current_prl_info_resp_msg_v01, prl_version_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dms_get_current_prl_info_resp_msg_v01, prl_version),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_current_prl_info_resp_msg_v01, prl_only) - QMI_IDL_OFFSET8(dms_get_current_prl_info_resp_msg_v01, prl_only_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dms_get_current_prl_info_resp_msg_v01, prl_only)
};

static const uint8_t dms_bind_subscription_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_bind_subscription_req_msg_v01, bind_subs)
};

static const uint8_t dms_bind_subscription_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_bind_subscription_req_msg is empty
 * static const uint8_t dms_get_bind_subscription_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_bind_subscription_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_bind_subscription_resp_msg_v01, bind_subscription) - QMI_IDL_OFFSET8(dms_get_bind_subscription_resp_msg_v01, bind_subscription_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_get_bind_subscription_resp_msg_v01, bind_subscription)
};

static const uint8_t dms_set_ap_sw_version_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dms_set_ap_sw_version_req_msg_v01, ap_sw_version),
  QMI_DMS_SW_VERSION_MAX_V01
};

static const uint8_t dms_set_ap_sw_version_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_ap_sw_version_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_cdma_lock_mode_req_msg is empty
 * static const uint8_t dms_get_cdma_lock_mode_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_cdma_lock_mode_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_cdma_lock_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_cdma_lock_mode_resp_msg_v01, cdma_lock_mode_status) - QMI_IDL_OFFSET8(dms_get_cdma_lock_mode_resp_msg_v01, cdma_lock_mode_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_get_cdma_lock_mode_resp_msg_v01, cdma_lock_mode_status)
};

static const uint8_t dms_set_test_config_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_set_test_config_req_msg_v01, tds_config) - QMI_IDL_OFFSET8(dms_set_test_config_req_msg_v01, tds_config_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_set_test_config_req_msg_v01, tds_config)
};

static const uint8_t dms_set_test_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_set_test_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dms_get_test_config_req_msg is empty
 * static const uint8_t dms_get_test_config_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_get_test_config_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_get_test_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_test_config_resp_msg_v01, active_tds_config) - QMI_IDL_OFFSET8(dms_get_test_config_resp_msg_v01, active_tds_config_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_get_test_config_resp_msg_v01, active_tds_config),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dms_get_test_config_resp_msg_v01, desired_tds_config) - QMI_IDL_OFFSET8(dms_get_test_config_resp_msg_v01, desired_tds_config_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dms_get_test_config_resp_msg_v01, desired_tds_config)
};

/* 
 * dms_clear_test_config_req_msg is empty
 * static const uint8_t dms_clear_test_config_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dms_clear_test_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dms_clear_test_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  dms_type_table_v01[] = {
  {sizeof(dms_battery_lvl_limits_type_v01), dms_battery_lvl_limits_type_data_v01},
  {sizeof(dms_power_state_type_v01), dms_power_state_type_data_v01},
  {sizeof(dms_pin_status_type_v01), dms_pin_status_type_data_v01},
  {sizeof(dms_subs_config_type_v01), dms_subs_config_type_data_v01},
  {sizeof(dms_multisim_capability_type_v01), dms_multisim_capability_type_data_v01},
  {sizeof(dms_device_capabilities_type_v01), dms_device_capabilities_type_data_v01},
  {sizeof(dms_pin_protection_info_type_v01), dms_pin_protection_info_type_data_v01},
  {sizeof(dms_pin_retries_status_type_v01), dms_pin_retries_status_type_data_v01},
  {sizeof(dms_pin_info_type_v01), dms_pin_info_type_data_v01},
  {sizeof(dms_pin_unblock_info_type_v01), dms_pin_unblock_info_type_data_v01},
  {sizeof(dms_pin_change_info_type_v01), dms_pin_change_info_type_data_v01},
  {sizeof(dms_device_time_type_v01), dms_device_time_type_data_v01},
  {sizeof(dms_activation_code_type_v01), dms_activation_code_type_data_v01},
  {sizeof(dms_manual_act_data_type_v01), dms_manual_act_data_type_data_v01},
  {sizeof(dms_mn_ha_key_type_v01), dms_mn_ha_key_type_data_v01},
  {sizeof(dms_mn_aaa_key_type_v01), dms_mn_aaa_key_type_data_v01},
  {sizeof(dms_pref_roaming_list_type_v01), dms_pref_roaming_list_type_data_v01},
  {sizeof(dms_user_lock_state_info_type_v01), dms_user_lock_state_info_type_data_v01},
  {sizeof(dms_user_lock_set_info_type_v01), dms_user_lock_set_info_type_data_v01},
  {sizeof(dms_user_data_type_v01), dms_user_data_type_data_v01},
  {sizeof(dms_eri_data_type_v01), dms_eri_data_type_data_v01},
  {sizeof(dms_facility_state_info_type_v01), dms_facility_state_info_type_data_v01},
  {sizeof(dms_facility_set_ck_info_type_v01), dms_facility_set_ck_info_type_data_v01},
  {sizeof(dms_facility_unblock_info_type_v01), dms_facility_unblock_info_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry dms_message_table_v01[] = {
  {sizeof(dms_reset_req_msg_v01), 0},
  {sizeof(dms_reset_resp_msg_v01), dms_reset_resp_msg_data_v01},
  {sizeof(dms_set_event_report_req_msg_v01), dms_set_event_report_req_msg_data_v01},
  {sizeof(dms_set_event_report_resp_msg_v01), dms_set_event_report_resp_msg_data_v01},
  {sizeof(dms_event_report_ind_msg_v01), dms_event_report_ind_msg_data_v01},
  {sizeof(dms_get_device_cap_req_msg_v01), 0},
  {sizeof(dms_get_device_cap_resp_msg_v01), dms_get_device_cap_resp_msg_data_v01},
  {sizeof(dms_get_device_mfr_req_msg_v01), 0},
  {sizeof(dms_get_device_mfr_resp_msg_v01), dms_get_device_mfr_resp_msg_data_v01},
  {sizeof(dms_get_device_model_id_req_msg_v01), 0},
  {sizeof(dms_get_device_model_id_resp_msg_v01), dms_get_device_model_id_resp_msg_data_v01},
  {sizeof(dms_get_device_rev_id_req_msg_v01), 0},
  {sizeof(dms_get_device_rev_id_resp_msg_v01), dms_get_device_rev_id_resp_msg_data_v01},
  {sizeof(dms_get_msisdn_req_msg_v01), 0},
  {sizeof(dms_get_msisdn_resp_msg_v01), dms_get_msisdn_resp_msg_data_v01},
  {sizeof(dms_get_device_serial_numbers_req_msg_v01), 0},
  {sizeof(dms_get_device_serial_numbers_resp_msg_v01), dms_get_device_serial_numbers_resp_msg_data_v01},
  {sizeof(dms_get_power_state_req_msg_v01), 0},
  {sizeof(dms_get_power_state_resp_msg_v01), dms_get_power_state_resp_msg_data_v01},
  {sizeof(dms_uim_set_pin_protection_req_msg_v01), dms_uim_set_pin_protection_req_msg_data_v01},
  {sizeof(dms_uim_set_pin_protection_resp_msg_v01), dms_uim_set_pin_protection_resp_msg_data_v01},
  {sizeof(dms_uim_verify_pin_req_msg_v01), dms_uim_verify_pin_req_msg_data_v01},
  {sizeof(dms_uim_verify_pin_resp_msg_v01), dms_uim_verify_pin_resp_msg_data_v01},
  {sizeof(dms_uim_unblock_pin_req_msg_v01), dms_uim_unblock_pin_req_msg_data_v01},
  {sizeof(dms_uim_unblock_pin_resp_msg_v01), dms_uim_unblock_pin_resp_msg_data_v01},
  {sizeof(dms_uim_change_pin_req_msg_v01), dms_uim_change_pin_req_msg_data_v01},
  {sizeof(dms_uim_change_pin_resp_msg_v01), dms_uim_change_pin_resp_msg_data_v01},
  {sizeof(dms_uim_get_pin_status_req_msg_v01), 0},
  {sizeof(dms_uim_get_pin_status_resp_msg_v01), dms_uim_get_pin_status_resp_msg_data_v01},
  {sizeof(dms_get_device_hardware_rev_req_msg_v01), 0},
  {sizeof(dms_get_device_hardware_rev_resp_msg_v01), dms_get_device_hardware_rev_resp_msg_data_v01},
  {sizeof(dms_get_operating_mode_req_msg_v01), 0},
  {sizeof(dms_get_operating_mode_resp_msg_v01), dms_get_operating_mode_resp_msg_data_v01},
  {sizeof(dms_set_operating_mode_req_msg_v01), dms_set_operating_mode_req_msg_data_v01},
  {sizeof(dms_set_operating_mode_resp_msg_v01), dms_set_operating_mode_resp_msg_data_v01},
  {sizeof(dms_get_time_req_msg_v01), 0},
  {sizeof(dms_get_time_resp_msg_v01), dms_get_time_resp_msg_data_v01},
  {sizeof(dms_get_prl_ver_req_msg_v01), 0},
  {sizeof(dms_get_prl_ver_resp_msg_v01), dms_get_prl_ver_resp_msg_data_v01},
  {sizeof(dms_get_activation_state_req_msg_v01), 0},
  {sizeof(dms_get_activation_state_resp_msg_v01), dms_get_activation_state_resp_msg_data_v01},
  {sizeof(dms_activate_automatic_req_msg_v01), dms_activate_automatic_req_msg_data_v01},
  {sizeof(dms_activate_automatic_resp_msg_v01), dms_activate_automatic_resp_msg_data_v01},
  {sizeof(dms_activate_manual_req_msg_v01), dms_activate_manual_req_msg_data_v01},
  {sizeof(dms_activate_manual_resp_msg_v01), dms_activate_manual_resp_msg_data_v01},
  {sizeof(dms_get_user_lock_state_req_msg_v01), 0},
  {sizeof(dms_get_user_lock_state_resp_msg_v01), dms_get_user_lock_state_resp_msg_data_v01},
  {sizeof(dms_set_user_lock_state_req_msg_v01), dms_set_user_lock_state_req_msg_data_v01},
  {sizeof(dms_set_user_lock_state_resp_msg_v01), dms_set_user_lock_state_resp_msg_data_v01},
  {sizeof(dms_set_user_lock_code_req_msg_v01), dms_set_user_lock_code_req_msg_data_v01},
  {sizeof(dms_set_user_lock_code_resp_msg_v01), dms_set_user_lock_code_resp_msg_data_v01},
  {sizeof(dms_read_user_data_req_msg_v01), 0},
  {sizeof(dms_read_user_data_resp_msg_v01), dms_read_user_data_resp_msg_data_v01},
  {sizeof(dms_write_user_data_req_msg_v01), dms_write_user_data_req_msg_data_v01},
  {sizeof(dms_write_user_data_resp_msg_v01), dms_write_user_data_resp_msg_data_v01},
  {sizeof(dms_read_eri_file_req_msg_v01), 0},
  {sizeof(dms_read_eri_file_resp_msg_v01), dms_read_eri_file_resp_msg_data_v01},
  {sizeof(dms_restore_factory_defaults_req_msg_v01), dms_restore_factory_defaults_req_msg_data_v01},
  {sizeof(dms_restore_factory_defaults_resp_msg_v01), dms_restore_factory_defaults_resp_msg_data_v01},
  {sizeof(dms_validate_service_programming_code_req_msg_v01), dms_validate_service_programming_code_req_msg_data_v01},
  {sizeof(dms_validate_service_programming_code_resp_msg_v01), dms_validate_service_programming_code_resp_msg_data_v01},
  {sizeof(dms_uim_get_iccid_req_msg_v01), 0},
  {sizeof(dms_uim_get_iccid_resp_msg_v01), dms_uim_get_iccid_resp_msg_data_v01},
  {sizeof(dms_uim_get_ck_status_req_msg_v01), dms_uim_get_ck_status_req_msg_data_v01},
  {sizeof(dms_uim_get_ck_status_resp_msg_v01), dms_uim_get_ck_status_resp_msg_data_v01},
  {sizeof(dms_uim_set_ck_protection_req_msg_v01), dms_uim_set_ck_protection_req_msg_data_v01},
  {sizeof(dms_uim_set_ck_protection_resp_msg_v01), dms_uim_set_ck_protection_resp_msg_data_v01},
  {sizeof(dms_uim_unblock_ck_req_msg_v01), dms_uim_unblock_ck_req_msg_data_v01},
  {sizeof(dms_uim_unblock_ck_resp_msg_v01), dms_uim_unblock_ck_resp_msg_data_v01},
  {sizeof(dms_uim_get_imsi_req_msg_v01), 0},
  {sizeof(dms_uim_get_imsi_resp_msg_v01), dms_uim_get_imsi_resp_msg_data_v01},
  {sizeof(dms_uim_get_state_req_msg_v01), 0},
  {sizeof(dms_uim_get_state_resp_msg_v01), dms_uim_get_state_resp_msg_data_v01},
  {sizeof(dms_get_band_capability_req_msg_v01), 0},
  {sizeof(dms_get_band_capability_resp_msg_v01), dms_get_band_capability_resp_msg_data_v01},
  {sizeof(dms_get_factory_sku_req_msg_v01), 0},
  {sizeof(dms_get_factory_sku_resp_msg_v01), dms_get_factory_sku_resp_msg_data_v01},
  {sizeof(dms_set_time_req_msg_v01), dms_set_time_req_msg_data_v01},
  {sizeof(dms_set_time_resp_msg_v01), dms_set_time_resp_msg_data_v01},
  {sizeof(dms_get_alt_net_config_req_msg_v01), 0},
  {sizeof(dms_get_alt_net_config_resp_msg_v01), dms_get_alt_net_config_resp_msg_data_v01},
  {sizeof(dms_set_alt_net_config_req_msg_v01), dms_set_alt_net_config_req_msg_data_v01},
  {sizeof(dms_set_alt_net_config_resp_msg_v01), dms_set_alt_net_config_resp_msg_data_v01},
  {sizeof(dms_get_sw_version_req_msg_v01), 0},
  {sizeof(dms_get_sw_version_resp_msg_v01), dms_get_sw_version_resp_msg_data_v01},
  {sizeof(dms_set_spc_req_msg_v01), dms_set_spc_req_msg_data_v01},
  {sizeof(dms_set_spc_resp_msg_v01), dms_set_spc_resp_msg_data_v01},
  {sizeof(dms_get_current_prl_info_req_msg_v01), 0},
  {sizeof(dms_get_current_prl_info_resp_msg_v01), dms_get_current_prl_info_resp_msg_data_v01},
  {sizeof(dms_bind_subscription_req_msg_v01), dms_bind_subscription_req_msg_data_v01},
  {sizeof(dms_bind_subscription_resp_msg_v01), dms_bind_subscription_resp_msg_data_v01},
  {sizeof(dms_get_bind_subscription_req_msg_v01), 0},
  {sizeof(dms_get_bind_subscription_resp_msg_v01), dms_get_bind_subscription_resp_msg_data_v01},
  {sizeof(dms_set_ap_sw_version_req_msg_v01), dms_set_ap_sw_version_req_msg_data_v01},
  {sizeof(dms_set_ap_sw_version_resp_msg_v01), dms_set_ap_sw_version_resp_msg_data_v01},
  {sizeof(dms_get_cdma_lock_mode_req_msg_v01), 0},
  {sizeof(dms_get_cdma_lock_mode_resp_msg_v01), dms_get_cdma_lock_mode_resp_msg_data_v01},
  {sizeof(dms_set_test_config_req_msg_v01), dms_set_test_config_req_msg_data_v01},
  {sizeof(dms_set_test_config_resp_msg_v01), dms_set_test_config_resp_msg_data_v01},
  {sizeof(dms_get_test_config_req_msg_v01), 0},
  {sizeof(dms_get_test_config_resp_msg_v01), dms_get_test_config_resp_msg_data_v01},
  {sizeof(dms_clear_test_config_req_msg_v01), 0},
  {sizeof(dms_clear_test_config_resp_msg_v01), dms_clear_test_config_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object dms_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *dms_qmi_idl_type_table_object_referenced_tables_v01[] =
{&dms_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object dms_qmi_idl_type_table_object_v01 = {
  sizeof(dms_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(dms_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  dms_type_table_v01,
  dms_message_table_v01,
  dms_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry dms_service_command_messages_v01[] = {
  {QMI_DMS_RESET_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_DMS_SET_EVENT_REPORT_REQ_V01, QMI_IDL_TYPE16(0, 2), 41},
  {QMI_DMS_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_DMS_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_DMS_GET_DEVICE_CAP_REQ_V01, QMI_IDL_TYPE16(0, 5), 0},
  {QMI_DMS_GET_DEVICE_MFR_REQ_V01, QMI_IDL_TYPE16(0, 7), 0},
  {QMI_DMS_GET_DEVICE_MODEL_ID_REQ_V01, QMI_IDL_TYPE16(0, 9), 0},
  {QMI_DMS_GET_DEVICE_REV_ID_REQ_V01, QMI_IDL_TYPE16(0, 11), 0},
  {QMI_DMS_GET_MSISDN_REQ_V01, QMI_IDL_TYPE16(0, 13), 0},
  {QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01, QMI_IDL_TYPE16(0, 15), 0},
  {QMI_DMS_GET_POWER_STATE_REQ_V01, QMI_IDL_TYPE16(0, 17), 0},
  {QMI_DMS_UIM_SET_PIN_PROTECTION_REQ_V01, QMI_IDL_TYPE16(0, 19), 22},
  {QMI_DMS_UIM_VERIFY_PIN_REQ_V01, QMI_IDL_TYPE16(0, 21), 21},
  {QMI_DMS_UIM_UNBLOCK_PIN_REQ_V01, QMI_IDL_TYPE16(0, 23), 38},
  {QMI_DMS_UIM_CHANGE_PIN_REQ_V01, QMI_IDL_TYPE16(0, 25), 38},
  {QMI_DMS_UIM_GET_PIN_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 27), 0},
  {QMI_DMS_GET_DEVICE_HARDWARE_REV_REQ_V01, QMI_IDL_TYPE16(0, 29), 0},
  {QMI_DMS_GET_OPERATING_MODE_REQ_V01, QMI_IDL_TYPE16(0, 31), 0},
  {QMI_DMS_SET_OPERATING_MODE_REQ_V01, QMI_IDL_TYPE16(0, 33), 4},
  {QMI_DMS_GET_TIME_REQ_V01, QMI_IDL_TYPE16(0, 35), 0},
  {QMI_DMS_GET_PRL_VER_REQ_V01, QMI_IDL_TYPE16(0, 37), 0},
  {QMI_DMS_GET_ACTIVATION_STATE_REQ_V01, QMI_IDL_TYPE16(0, 39), 0},
  {QMI_DMS_ACTIVATE_AUTOMATIC_REQ_V01, QMI_IDL_TYPE16(0, 41), 85},
  {QMI_DMS_ACTIVATE_MANUAL_REQ_V01, QMI_IDL_TYPE16(0, 43), 1627},
  {QMI_DMS_GET_USER_LOCK_STATE_REQ_V01, QMI_IDL_TYPE16(0, 45), 0},
  {QMI_DMS_SET_USER_LOCK_STATE_REQ_V01, QMI_IDL_TYPE16(0, 47), 8},
  {QMI_DMS_SET_USER_LOCK_CODE_REQ_V01, QMI_IDL_TYPE16(0, 49), 11},
  {QMI_DMS_READ_USER_DATA_REQ_V01, QMI_IDL_TYPE16(0, 51), 0},
  {QMI_DMS_WRITE_USER_DATA_REQ_V01, QMI_IDL_TYPE16(0, 53), 517},
  {QMI_DMS_READ_ERI_FILE_REQ_V01, QMI_IDL_TYPE16(0, 55), 0},
  {QMI_DMS_RESTORE_FACTORY_DEFAULTS_REQ_V01, QMI_IDL_TYPE16(0, 57), 9},
  {QMI_DMS_VALIDATE_SERVICE_PROGRAMMING_CODE_REQ_V01, QMI_IDL_TYPE16(0, 59), 9},
  {QMI_DMS_UIM_GET_ICCID_REQ_V01, QMI_IDL_TYPE16(0, 61), 0},
  {QMI_DMS_UIM_GET_CK_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 63), 4},
  {QMI_DMS_UIM_SET_CK_PROTECTION_REQ_V01, QMI_IDL_TYPE16(0, 65), 14},
  {QMI_DMS_UIM_UNBLOCK_CK_REQ_V01, QMI_IDL_TYPE16(0, 67), 13},
  {QMI_DMS_UIM_GET_IMSI_REQ_V01, QMI_IDL_TYPE16(0, 69), 0},
  {QMI_DMS_UIM_GET_STATE_REQ_V01, QMI_IDL_TYPE16(0, 71), 0},
  {QMI_DMS_GET_BAND_CAPABILITY_REQ_V01, QMI_IDL_TYPE16(0, 73), 0},
  {QMI_DMS_GET_FACTORY_SKU_REQ_V01, QMI_IDL_TYPE16(0, 75), 0},
  {QMI_DMS_SET_TIME_REQ_V01, QMI_IDL_TYPE16(0, 77), 18},
  {QMI_DMS_GET_ALT_NET_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 79), 0},
  {QMI_DMS_SET_ALT_NET_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 81), 4},
  {QMI_DMS_GET_SW_VERSION_REQ_V01, QMI_IDL_TYPE16(0, 83), 0},
  {QMI_DMS_SET_SPC_REQ_V01, QMI_IDL_TYPE16(0, 85), 18},
  {QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01, QMI_IDL_TYPE16(0, 87), 0},
  {QMI_DMS_BIND_SUBSCRIPTION_REQ_V01, QMI_IDL_TYPE16(0, 89), 7},
  {QMI_DMS_GET_BIND_SUBSCRIPTION_REQ_V01, QMI_IDL_TYPE16(0, 91), 0},
  {QMI_DMS_SET_AP_SW_VERSION_REQ_V01, QMI_IDL_TYPE16(0, 93), 35},
  {QMI_DMS_GET_CDMA_LOCK_MODE_REQ_V01, QMI_IDL_TYPE16(0, 95), 0},
  {QMI_DMS_SET_TEST_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 97), 7},
  {QMI_DMS_GET_TEST_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 99), 0},
  {QMI_DMS_CLEAR_TEST_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 101), 0}
};

static const qmi_idl_service_message_table_entry dms_service_response_messages_v01[] = {
  {QMI_DMS_RESET_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_DMS_SET_EVENT_REPORT_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_DMS_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_DMS_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_DMS_GET_DEVICE_CAP_RESP_V01, QMI_IDL_TYPE16(0, 6), 1675},
  {QMI_DMS_GET_DEVICE_MFR_RESP_V01, QMI_IDL_TYPE16(0, 8), 138},
  {QMI_DMS_GET_DEVICE_MODEL_ID_RESP_V01, QMI_IDL_TYPE16(0, 10), 266},
  {QMI_DMS_GET_DEVICE_REV_ID_RESP_V01, QMI_IDL_TYPE16(0, 12), 543},
  {QMI_DMS_GET_MSISDN_RESP_V01, QMI_IDL_TYPE16(0, 14), 112},
  {QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_RESP_V01, QMI_IDL_TYPE16(0, 16), 370},
  {QMI_DMS_GET_POWER_STATE_RESP_V01, QMI_IDL_TYPE16(0, 18), 12},
  {QMI_DMS_UIM_SET_PIN_PROTECTION_RESP_V01, QMI_IDL_TYPE16(0, 20), 12},
  {QMI_DMS_UIM_VERIFY_PIN_RESP_V01, QMI_IDL_TYPE16(0, 22), 12},
  {QMI_DMS_UIM_UNBLOCK_PIN_RESP_V01, QMI_IDL_TYPE16(0, 24), 12},
  {QMI_DMS_UIM_CHANGE_PIN_RESP_V01, QMI_IDL_TYPE16(0, 26), 12},
  {QMI_DMS_UIM_GET_PIN_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 28), 19},
  {QMI_DMS_GET_DEVICE_HARDWARE_REV_RESP_V01, QMI_IDL_TYPE16(0, 30), 266},
  {QMI_DMS_GET_OPERATING_MODE_RESP_V01, QMI_IDL_TYPE16(0, 32), 20},
  {QMI_DMS_SET_OPERATING_MODE_RESP_V01, QMI_IDL_TYPE16(0, 34), 7},
  {QMI_DMS_GET_TIME_RESP_V01, QMI_IDL_TYPE16(0, 36), 40},
  {QMI_DMS_GET_PRL_VER_RESP_V01, QMI_IDL_TYPE16(0, 38), 16},
  {QMI_DMS_GET_ACTIVATION_STATE_RESP_V01, QMI_IDL_TYPE16(0, 40), 12},
  {QMI_DMS_ACTIVATE_AUTOMATIC_RESP_V01, QMI_IDL_TYPE16(0, 42), 7},
  {QMI_DMS_ACTIVATE_MANUAL_RESP_V01, QMI_IDL_TYPE16(0, 44), 7},
  {QMI_DMS_GET_USER_LOCK_STATE_RESP_V01, QMI_IDL_TYPE16(0, 46), 11},
  {QMI_DMS_SET_USER_LOCK_STATE_RESP_V01, QMI_IDL_TYPE16(0, 48), 7},
  {QMI_DMS_SET_USER_LOCK_CODE_RESP_V01, QMI_IDL_TYPE16(0, 50), 7},
  {QMI_DMS_READ_USER_DATA_RESP_V01, QMI_IDL_TYPE16(0, 52), 524},
  {QMI_DMS_WRITE_USER_DATA_RESP_V01, QMI_IDL_TYPE16(0, 54), 7},
  {QMI_DMS_READ_ERI_FILE_RESP_V01, QMI_IDL_TYPE16(0, 56), 1036},
  {QMI_DMS_RESTORE_FACTORY_DEFAULTS_RESP_V01, QMI_IDL_TYPE16(0, 58), 7},
  {QMI_DMS_VALIDATE_SERVICE_PROGRAMMING_CODE_RESP_V01, QMI_IDL_TYPE16(0, 60), 7},
  {QMI_DMS_UIM_GET_ICCID_RESP_V01, QMI_IDL_TYPE16(0, 62), 30},
  {QMI_DMS_UIM_GET_CK_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 64), 17},
  {QMI_DMS_UIM_SET_CK_PROTECTION_RESP_V01, QMI_IDL_TYPE16(0, 66), 11},
  {QMI_DMS_UIM_UNBLOCK_CK_RESP_V01, QMI_IDL_TYPE16(0, 68), 11},
  {QMI_DMS_UIM_GET_IMSI_RESP_V01, QMI_IDL_TYPE16(0, 70), 42},
  {QMI_DMS_UIM_GET_STATE_RESP_V01, QMI_IDL_TYPE16(0, 72), 11},
  {QMI_DMS_GET_BAND_CAPABILITY_RESP_V01, QMI_IDL_TYPE16(0, 74), 40},
  {QMI_DMS_GET_FACTORY_SKU_RESP_V01, QMI_IDL_TYPE16(0, 76), 138},
  {QMI_DMS_SET_TIME_RESP_V01, QMI_IDL_TYPE16(0, 78), 7},
  {QMI_DMS_GET_ALT_NET_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 80), 11},
  {QMI_DMS_SET_ALT_NET_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 82), 7},
  {QMI_DMS_GET_SW_VERSION_RESP_V01, QMI_IDL_TYPE16(0, 84), 42},
  {QMI_DMS_SET_SPC_RESP_V01, QMI_IDL_TYPE16(0, 86), 7},
  {QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01, QMI_IDL_TYPE16(0, 88), 16},
  {QMI_DMS_BIND_SUBSCRIPTION_RESP_V01, QMI_IDL_TYPE16(0, 90), 7},
  {QMI_DMS_GET_BIND_SUBSCRIPTION_RESP_V01, QMI_IDL_TYPE16(0, 92), 14},
  {QMI_DMS_SET_AP_SW_VERSION_RESP_V01, QMI_IDL_TYPE16(0, 94), 7},
  {QMI_DMS_GET_CDMA_LOCK_MODE_RESP_V01, QMI_IDL_TYPE16(0, 96), 14},
  {QMI_DMS_SET_TEST_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 98), 7},
  {QMI_DMS_GET_TEST_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 100), 21},
  {QMI_DMS_CLEAR_TEST_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 102), 7}
};

static const qmi_idl_service_message_table_entry dms_service_indication_messages_v01[] = {
  {QMI_DMS_EVENT_REPORT_IND_V01, QMI_IDL_TYPE16(0, 4), 1650}
};

/*Service Object*/
struct qmi_idl_service_object dms_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x02,
  8204,
  { sizeof(dms_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(dms_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(dms_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { dms_service_command_messages_v01, dms_service_response_messages_v01, dms_service_indication_messages_v01},
  &dms_qmi_idl_type_table_object_v01,
  0x1F,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type dms_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( DMS_V01_IDL_MAJOR_VERS != idl_maj_version || DMS_V01_IDL_MINOR_VERS != idl_min_version 
       || DMS_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&dms_qmi_idl_service_object_v01;
}

