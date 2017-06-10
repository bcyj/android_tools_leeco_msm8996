/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        D A T A _ S Y S T E M _ D E T E R M I N A T I O N _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the dsd service Data structures.

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.2 
   It was generated on: Fri Dec 12 2014 (Spin 0)
   From IDL File: data_system_determination_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "data_system_determination_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t dsd_system_status_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_system_status_info_type_v01, technology),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_system_status_info_type_v01, rat_value),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dsd_system_status_info_type_v01, so_mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dsd_apn_avail_sys_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_apn_avail_sys_info_type_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_apn_avail_sys_info_type_v01, apn_avail_sys),
  QMI_DSD_MAX_AVAIL_SYS_V01,
  QMI_IDL_OFFSET8(dsd_apn_avail_sys_info_type_v01, apn_avail_sys) - QMI_IDL_OFFSET8(dsd_apn_avail_sys_info_type_v01, apn_avail_sys_len),
  QMI_IDL_TYPE88(0, 0),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dsd_ipv6_addr_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_ipv6_addr_type_v01, ipv6_address),
  QMI_DSD_IPV6_ADDR_LEN_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_ipv6_addr_type_v01, prefix_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dsd_apn_pref_sys_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_apn_pref_sys_type_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01,

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_apn_pref_sys_type_v01, pref_sys),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dsd_apn_name_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_apn_name_type_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dsd_apn_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_apn_info_type_v01, apn_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_apn_info_type_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * dsd_get_system_status_req_msg is empty
 * static const uint8_t dsd_get_system_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dsd_get_system_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_system_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_system_status_resp_msg_v01, avail_sys) - QMI_IDL_OFFSET8(dsd_get_system_status_resp_msg_v01, avail_sys_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_system_status_resp_msg_v01, avail_sys),
  QMI_DSD_MAX_AVAIL_SYS_V01,
  QMI_IDL_OFFSET8(dsd_get_system_status_resp_msg_v01, avail_sys) - QMI_IDL_OFFSET8(dsd_get_system_status_resp_msg_v01, avail_sys_len),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dsd_get_system_status_resp_msg_v01, apn_avail_sys_info) - QMI_IDL_OFFSET16RELATIVE(dsd_get_system_status_resp_msg_v01, apn_avail_sys_info_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dsd_get_system_status_resp_msg_v01, apn_avail_sys_info),
  QMI_DSD_MAX_APNS_V01,
  QMI_IDL_OFFSET16RELATIVE(dsd_get_system_status_resp_msg_v01, apn_avail_sys_info) - QMI_IDL_OFFSET16RELATIVE(dsd_get_system_status_resp_msg_v01, apn_avail_sys_info_len),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t dsd_system_status_change_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_system_status_change_req_msg_v01, limit_so_mask_change_ind) - QMI_IDL_OFFSET8(dsd_system_status_change_req_msg_v01, limit_so_mask_change_ind_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_system_status_change_req_msg_v01, limit_so_mask_change_ind)
};

static const uint8_t dsd_system_status_change_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_system_status_change_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dsd_system_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_system_status_ind_msg_v01, avail_sys) - QMI_IDL_OFFSET8(dsd_system_status_ind_msg_v01, avail_sys_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_system_status_ind_msg_v01, avail_sys),
  QMI_DSD_MAX_AVAIL_SYS_V01,
  QMI_IDL_OFFSET8(dsd_system_status_ind_msg_v01, avail_sys) - QMI_IDL_OFFSET8(dsd_system_status_ind_msg_v01, avail_sys_len),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dsd_system_status_ind_msg_v01, apn_avail_sys_info) - QMI_IDL_OFFSET16RELATIVE(dsd_system_status_ind_msg_v01, apn_avail_sys_info_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(dsd_system_status_ind_msg_v01, apn_avail_sys_info),
  QMI_DSD_MAX_APNS_V01,
  QMI_IDL_OFFSET16RELATIVE(dsd_system_status_ind_msg_v01, apn_avail_sys_info) - QMI_IDL_OFFSET16RELATIVE(dsd_system_status_ind_msg_v01, apn_avail_sys_info_len),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t dsd_bind_subscription_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_bind_subscription_req_msg_v01, bind_subs)
};

static const uint8_t dsd_bind_subscription_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dsd_get_bind_subscription_req_msg is empty
 * static const uint8_t dsd_get_bind_subscription_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dsd_get_bind_subscription_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_bind_subscription_resp_msg_v01, bind_subscription) - QMI_IDL_OFFSET8(dsd_get_bind_subscription_resp_msg_v01, bind_subscription_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_bind_subscription_resp_msg_v01, bind_subscription)
};

static const uint8_t dsd_wlan_available_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wlan_ap_mac_address),
  QMI_DSD_MAC_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wlan_ipv4_address) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wlan_ipv4_address_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wlan_ipv4_address),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wlan_ipv6_address) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wlan_ipv6_address_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wlan_ipv6_address),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wqe_status) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wqe_status_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, wqe_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv4_address_1) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv4_address_1_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv4_address_1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv4_address_2) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv4_address_2_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv4_address_2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv6_address_1) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv6_address_1_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv6_address_1),
  QMI_DSD_IPV6_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv6_address_2) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv6_address_2_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, dns_ipv6_address_2),
  QMI_DSD_IPV6_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv4_address_1) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv4_address_1_valid)),
  0x17,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv4_address_1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv4_address_2) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv4_address_2_valid)),
  0x18,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv4_address_2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv6_address_1) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv6_address_1_valid)),
  0x19,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv6_address_1),
  QMI_DSD_IPV6_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv6_address_2) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv6_address_2_valid)),
  0x1A,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, epdg_ipv6_address_2),
  QMI_DSD_IPV6_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, ssid) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, ssid_valid)),
  0x1B,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, ssid),
  QMI_DSD_MAX_SSID_LEN_V01,
  QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, ssid) - QMI_IDL_OFFSET8(dsd_wlan_available_req_msg_v01, ssid_len)
};

static const uint8_t dsd_wlan_available_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_wlan_available_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dsd_wlan_not_available_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_wlan_not_available_req_msg_v01, wqe_status) - QMI_IDL_OFFSET8(dsd_wlan_not_available_req_msg_v01, wqe_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_wlan_not_available_req_msg_v01, wqe_status)
};

static const uint8_t dsd_wlan_not_available_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_wlan_not_available_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dsd_set_wlan_preference_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_set_wlan_preference_req_msg_v01, wlan_preference)
};

static const uint8_t dsd_set_wlan_preference_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_set_wlan_preference_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dsd_get_wlan_preference_req_msg is empty
 * static const uint8_t dsd_get_wlan_preference_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dsd_get_wlan_preference_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_wlan_preference_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_wlan_preference_resp_msg_v01, wlan_preference) - QMI_IDL_OFFSET8(dsd_get_wlan_preference_resp_msg_v01, wlan_preference_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_wlan_preference_resp_msg_v01, wlan_preference)
};

static const uint8_t dsd_set_apn_preferred_system_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_set_apn_preferred_system_req_msg_v01, apn_pref_sys),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t dsd_set_apn_preferred_system_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_set_apn_preferred_system_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dsd_get_modem_power_cost_req_msg is empty
 * static const uint8_t dsd_get_modem_power_cost_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dsd_get_modem_power_cost_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_modem_power_cost_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_modem_power_cost_resp_msg_v01, power_cost) - QMI_IDL_OFFSET8(dsd_get_modem_power_cost_resp_msg_v01, power_cost_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_modem_power_cost_resp_msg_v01, power_cost)
};

/* 
 * dsd_pdn_policy_start_txn_req_msg is empty
 * static const uint8_t dsd_pdn_policy_start_txn_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dsd_pdn_policy_start_txn_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_pdn_policy_start_txn_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_pdn_policy_start_txn_resp_msg_v01, txn_id) - QMI_IDL_OFFSET8(dsd_pdn_policy_start_txn_resp_msg_v01, txn_id_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_pdn_policy_start_txn_resp_msg_v01, txn_id)
};

static const uint8_t dsd_add_pdn_policy_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, txn_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01,

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, supported_system_priority_list),
  QMI_DSD_MAX_SYSTEMS_V01,
  QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, supported_system_priority_list) - QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, supported_system_priority_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, is_default) - QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, is_default_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, is_default),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, override_type) - QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, override_type_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_add_pdn_policy_req_msg_v01, override_type)
};

static const uint8_t dsd_add_pdn_policy_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_add_pdn_policy_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dsd_modify_pdn_policy_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, txn_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, supported_system_priority_list) - QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, supported_system_priority_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, supported_system_priority_list),
  QMI_DSD_MAX_SYSTEMS_V01,
  QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, supported_system_priority_list) - QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, supported_system_priority_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, is_default) - QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, is_default_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, is_default),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, override_type) - QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, override_type_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_modify_pdn_policy_req_msg_v01, override_type)
};

static const uint8_t dsd_modify_pdn_policy_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_modify_pdn_policy_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dsd_delete_pdn_policy_by_apn_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_delete_pdn_policy_by_apn_req_msg_v01, txn_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_delete_pdn_policy_by_apn_req_msg_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01
};

static const uint8_t dsd_delete_pdn_policy_by_apn_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_delete_pdn_policy_by_apn_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dsd_get_pdn_policy_apn_list_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_apn_list_req_msg_v01, txn_id)
};

static const uint8_t dsd_get_pdn_policy_apn_list_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_apn_list_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_pdn_policy_apn_list_resp_msg_v01, apn_list) - QMI_IDL_OFFSET8(dsd_get_pdn_policy_apn_list_resp_msg_v01, apn_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_apn_list_resp_msg_v01, apn_list),
  QMI_DSD_MAX_APNS_V01,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_apn_list_resp_msg_v01, apn_list) - QMI_IDL_OFFSET8(dsd_get_pdn_policy_apn_list_resp_msg_v01, apn_list_len),
  QMI_IDL_TYPE88(0, 4)
};

static const uint8_t dsd_get_pdn_policy_settings_for_apn_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_req_msg_v01, txn_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_req_msg_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01
};

static const uint8_t dsd_get_pdn_policy_settings_for_apn_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, apn_name) - QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, apn_name_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, supported_system_priority_list) - QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, supported_system_priority_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, supported_system_priority_list),
  QMI_DSD_MAX_SYSTEMS_V01,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, supported_system_priority_list) - QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, supported_system_priority_list_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, is_default) - QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, is_default_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, is_default),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, override_type) - QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, override_type_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01, override_type)
};

static const uint8_t dsd_pdn_policy_end_txn_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_pdn_policy_end_txn_req_msg_v01, txn_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_pdn_policy_end_txn_req_msg_v01, txn_exec_type)
};

static const uint8_t dsd_pdn_policy_end_txn_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_pdn_policy_end_txn_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dsd_set_apn_info_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_set_apn_info_req_msg_v01, apn_info),
  QMI_IDL_TYPE88(0, 5),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_set_apn_info_req_msg_v01, apn_invalid) - QMI_IDL_OFFSET8(dsd_set_apn_info_req_msg_v01, apn_invalid_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_set_apn_info_req_msg_v01, apn_invalid)
};

static const uint8_t dsd_set_apn_info_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_set_apn_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dsd_get_apn_info_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_apn_info_req_msg_v01, apn_type)
};

static const uint8_t dsd_get_apn_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_apn_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_apn_info_resp_msg_v01, apn_name) - QMI_IDL_OFFSET8(dsd_get_apn_info_resp_msg_v01, apn_name_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(dsd_get_apn_info_resp_msg_v01, apn_name),
  QMI_DSD_MAX_APN_LEN_V01
};

static const uint8_t dsd_notify_data_settings_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_notify_data_settings_req_msg_v01, data_service_switch) - QMI_IDL_OFFSET8(dsd_notify_data_settings_req_msg_v01, data_service_switch_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_notify_data_settings_req_msg_v01, data_service_switch),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_notify_data_settings_req_msg_v01, data_service_roaming_switch) - QMI_IDL_OFFSET8(dsd_notify_data_settings_req_msg_v01, data_service_roaming_switch_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_notify_data_settings_req_msg_v01, data_service_roaming_switch)
};

static const uint8_t dsd_notify_data_settings_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_notify_data_settings_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * dsd_get_data_settings_req_msg is empty
 * static const uint8_t dsd_get_data_settings_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dsd_get_data_settings_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_data_settings_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_data_settings_resp_msg_v01, data_service_switch) - QMI_IDL_OFFSET8(dsd_get_data_settings_resp_msg_v01, data_service_switch_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_get_data_settings_resp_msg_v01, data_service_switch),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_data_settings_resp_msg_v01, data_service_roaming_switch) - QMI_IDL_OFFSET8(dsd_get_data_settings_resp_msg_v01, data_service_roaming_switch_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_get_data_settings_resp_msg_v01, data_service_roaming_switch)
};

static const uint8_t dsd_thermal_info_change_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_thermal_info_change_ind_msg_v01, thermal_action) - QMI_IDL_OFFSET8(dsd_thermal_info_change_ind_msg_v01, thermal_action_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_thermal_info_change_ind_msg_v01, thermal_action)
};

/* 
 * dsd_get_thermal_mitigation_info_req_msg is empty
 * static const uint8_t dsd_get_thermal_mitigation_info_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dsd_get_thermal_mitigation_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_get_thermal_mitigation_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_get_thermal_mitigation_info_resp_msg_v01, thermal_action) - QMI_IDL_OFFSET8(dsd_get_thermal_mitigation_info_resp_msg_v01, thermal_action_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dsd_get_thermal_mitigation_info_resp_msg_v01, thermal_action)
};

static const uint8_t dsd_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dsd_indication_register_req_msg_v01, report_thermal_info_changes) - QMI_IDL_OFFSET8(dsd_indication_register_req_msg_v01, report_thermal_info_changes_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dsd_indication_register_req_msg_v01, report_thermal_info_changes)
};

static const uint8_t dsd_indication_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dsd_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  dsd_type_table_v01[] = {
  {sizeof(dsd_system_status_info_type_v01), dsd_system_status_info_type_data_v01},
  {sizeof(dsd_apn_avail_sys_info_type_v01), dsd_apn_avail_sys_info_type_data_v01},
  {sizeof(dsd_ipv6_addr_type_v01), dsd_ipv6_addr_type_data_v01},
  {sizeof(dsd_apn_pref_sys_type_v01), dsd_apn_pref_sys_type_data_v01},
  {sizeof(dsd_apn_name_type_v01), dsd_apn_name_type_data_v01},
  {sizeof(dsd_apn_info_type_v01), dsd_apn_info_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry dsd_message_table_v01[] = {
  {sizeof(dsd_get_system_status_req_msg_v01), 0},
  {sizeof(dsd_get_system_status_resp_msg_v01), dsd_get_system_status_resp_msg_data_v01},
  {sizeof(dsd_system_status_change_req_msg_v01), dsd_system_status_change_req_msg_data_v01},
  {sizeof(dsd_system_status_change_resp_msg_v01), dsd_system_status_change_resp_msg_data_v01},
  {sizeof(dsd_system_status_ind_msg_v01), dsd_system_status_ind_msg_data_v01},
  {sizeof(dsd_bind_subscription_req_msg_v01), dsd_bind_subscription_req_msg_data_v01},
  {sizeof(dsd_bind_subscription_resp_msg_v01), dsd_bind_subscription_resp_msg_data_v01},
  {sizeof(dsd_get_bind_subscription_req_msg_v01), 0},
  {sizeof(dsd_get_bind_subscription_resp_msg_v01), dsd_get_bind_subscription_resp_msg_data_v01},
  {sizeof(dsd_wlan_available_req_msg_v01), dsd_wlan_available_req_msg_data_v01},
  {sizeof(dsd_wlan_available_resp_msg_v01), dsd_wlan_available_resp_msg_data_v01},
  {sizeof(dsd_wlan_not_available_req_msg_v01), dsd_wlan_not_available_req_msg_data_v01},
  {sizeof(dsd_wlan_not_available_resp_msg_v01), dsd_wlan_not_available_resp_msg_data_v01},
  {sizeof(dsd_set_wlan_preference_req_msg_v01), dsd_set_wlan_preference_req_msg_data_v01},
  {sizeof(dsd_set_wlan_preference_resp_msg_v01), dsd_set_wlan_preference_resp_msg_data_v01},
  {sizeof(dsd_get_wlan_preference_req_msg_v01), 0},
  {sizeof(dsd_get_wlan_preference_resp_msg_v01), dsd_get_wlan_preference_resp_msg_data_v01},
  {sizeof(dsd_set_apn_preferred_system_req_msg_v01), dsd_set_apn_preferred_system_req_msg_data_v01},
  {sizeof(dsd_set_apn_preferred_system_resp_msg_v01), dsd_set_apn_preferred_system_resp_msg_data_v01},
  {sizeof(dsd_get_modem_power_cost_req_msg_v01), 0},
  {sizeof(dsd_get_modem_power_cost_resp_msg_v01), dsd_get_modem_power_cost_resp_msg_data_v01},
  {sizeof(dsd_pdn_policy_start_txn_req_msg_v01), 0},
  {sizeof(dsd_pdn_policy_start_txn_resp_msg_v01), dsd_pdn_policy_start_txn_resp_msg_data_v01},
  {sizeof(dsd_add_pdn_policy_req_msg_v01), dsd_add_pdn_policy_req_msg_data_v01},
  {sizeof(dsd_add_pdn_policy_resp_msg_v01), dsd_add_pdn_policy_resp_msg_data_v01},
  {sizeof(dsd_modify_pdn_policy_req_msg_v01), dsd_modify_pdn_policy_req_msg_data_v01},
  {sizeof(dsd_modify_pdn_policy_resp_msg_v01), dsd_modify_pdn_policy_resp_msg_data_v01},
  {sizeof(dsd_delete_pdn_policy_by_apn_req_msg_v01), dsd_delete_pdn_policy_by_apn_req_msg_data_v01},
  {sizeof(dsd_delete_pdn_policy_by_apn_resp_msg_v01), dsd_delete_pdn_policy_by_apn_resp_msg_data_v01},
  {sizeof(dsd_get_pdn_policy_apn_list_req_msg_v01), dsd_get_pdn_policy_apn_list_req_msg_data_v01},
  {sizeof(dsd_get_pdn_policy_apn_list_resp_msg_v01), dsd_get_pdn_policy_apn_list_resp_msg_data_v01},
  {sizeof(dsd_get_pdn_policy_settings_for_apn_req_msg_v01), dsd_get_pdn_policy_settings_for_apn_req_msg_data_v01},
  {sizeof(dsd_get_pdn_policy_settings_for_apn_resp_msg_v01), dsd_get_pdn_policy_settings_for_apn_resp_msg_data_v01},
  {sizeof(dsd_pdn_policy_end_txn_req_msg_v01), dsd_pdn_policy_end_txn_req_msg_data_v01},
  {sizeof(dsd_pdn_policy_end_txn_resp_msg_v01), dsd_pdn_policy_end_txn_resp_msg_data_v01},
  {sizeof(dsd_set_apn_info_req_msg_v01), dsd_set_apn_info_req_msg_data_v01},
  {sizeof(dsd_set_apn_info_resp_msg_v01), dsd_set_apn_info_resp_msg_data_v01},
  {sizeof(dsd_get_apn_info_req_msg_v01), dsd_get_apn_info_req_msg_data_v01},
  {sizeof(dsd_get_apn_info_resp_msg_v01), dsd_get_apn_info_resp_msg_data_v01},
  {sizeof(dsd_notify_data_settings_req_msg_v01), dsd_notify_data_settings_req_msg_data_v01},
  {sizeof(dsd_notify_data_settings_resp_msg_v01), dsd_notify_data_settings_resp_msg_data_v01},
  {sizeof(dsd_get_data_settings_req_msg_v01), 0},
  {sizeof(dsd_get_data_settings_resp_msg_v01), dsd_get_data_settings_resp_msg_data_v01},
  {sizeof(dsd_thermal_info_change_ind_msg_v01), dsd_thermal_info_change_ind_msg_data_v01},
  {sizeof(dsd_get_thermal_mitigation_info_req_msg_v01), 0},
  {sizeof(dsd_get_thermal_mitigation_info_resp_msg_v01), dsd_get_thermal_mitigation_info_resp_msg_data_v01},
  {sizeof(dsd_indication_register_req_msg_v01), dsd_indication_register_req_msg_data_v01},
  {sizeof(dsd_indication_register_resp_msg_v01), dsd_indication_register_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object dsd_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *dsd_qmi_idl_type_table_object_referenced_tables_v01[] =
{&dsd_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object dsd_qmi_idl_type_table_object_v01 = {
  sizeof(dsd_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(dsd_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  dsd_type_table_v01,
  dsd_message_table_v01,
  dsd_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry dsd_service_command_messages_v01[] = {
  {QMI_DSD_WLAN_AVAILABLE_REQ_V01, QMI_IDL_TYPE16(0, 9), 183},
  {QMI_DSD_WLAN_NOT_AVAILABLE_REQ_V01, QMI_IDL_TYPE16(0, 11), 7},
  {QMI_DSD_SET_WLAN_PREFERENCE_REQ_V01, QMI_IDL_TYPE16(0, 13), 7},
  {QMI_DSD_GET_WLAN_PREFERENCE_REQ_V01, QMI_IDL_TYPE16(0, 15), 0},
  {QMI_DSD_GET_SYSTEM_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_DSD_SYSTEM_STATUS_CHANGE_REQ_V01, QMI_IDL_TYPE16(0, 2), 4},
  {QMI_DSD_BIND_SUBSCRIPTION_REQ_V01, QMI_IDL_TYPE16(0, 5), 7},
  {QMI_DSD_GET_BIND_SUBSCRIPTION_REQ_V01, QMI_IDL_TYPE16(0, 7), 0},
  {QMI_DSD_SET_APN_PREFERRED_SYSTEM_REQ_V01, QMI_IDL_TYPE16(0, 17), 108},
  {QMI_DSD_GET_MODEM_POWER_COST_REQ_V01, QMI_IDL_TYPE16(0, 19), 0},
  {QMI_DSD_PDN_POLICY_START_TXN_REQ_V01, QMI_IDL_TYPE16(0, 21), 0},
  {QMI_DSD_ADD_PDN_POLICY_REQ_V01, QMI_IDL_TYPE16(0, 23), 137},
  {QMI_DSD_MODIFY_PDN_POLICY_REQ_V01, QMI_IDL_TYPE16(0, 25), 137},
  {QMI_DSD_DELETE_PDN_POLICY_BY_APN_REQ_V01, QMI_IDL_TYPE16(0, 27), 110},
  {QMI_DSD_GET_PDN_POLICY_APN_LIST_REQ_V01, QMI_IDL_TYPE16(0, 29), 7},
  {QMI_DSD_GET_PDN_POLICY_SETTINGS_FOR_APN_REQ_V01, QMI_IDL_TYPE16(0, 31), 110},
  {QMI_DSD_PDN_POLICY_END_TXN_REQ_V01, QMI_IDL_TYPE16(0, 33), 14},
  {QMI_DSD_SET_APN_INFO_REQ_V01, QMI_IDL_TYPE16(0, 35), 112},
  {QMI_DSD_GET_APN_INFO_REQ_V01, QMI_IDL_TYPE16(0, 37), 7},
  {QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01, QMI_IDL_TYPE16(0, 39), 8},
  {QMI_DSD_GET_DATA_SETTING_REQ_V01, QMI_IDL_TYPE16(0, 41), 0},
  {QMI_DSD_GET_THERMAL_MITIGATION_INFO_REQ_V01, QMI_IDL_TYPE16(0, 44), 0},
  {QMI_DSD_INDICATION_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 46), 4}
};

static const qmi_idl_service_message_table_entry dsd_service_response_messages_v01[] = {
  {QMI_DSD_WLAN_AVAILABLE_RESP_V01, QMI_IDL_TYPE16(0, 10), 7},
  {QMI_DSD_WLAN_NOT_AVAILABLE_RESP_V01, QMI_IDL_TYPE16(0, 12), 7},
  {QMI_DSD_SET_WLAN_PREFERENCE_RESP_V01, QMI_IDL_TYPE16(0, 14), 7},
  {QMI_DSD_GET_WLAN_PREFERENCE_RESP_V01, QMI_IDL_TYPE16(0, 16), 14},
  {QMI_DSD_GET_SYSTEM_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 1), 5385},
  {QMI_DSD_SYSTEM_STATUS_CHANGE_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_DSD_BIND_SUBSCRIPTION_RESP_V01, QMI_IDL_TYPE16(0, 6), 7},
  {QMI_DSD_GET_BIND_SUBSCRIPTION_RESP_V01, QMI_IDL_TYPE16(0, 8), 14},
  {QMI_DSD_SET_APN_PREFERRED_SYSTEM_RESP_V01, QMI_IDL_TYPE16(0, 18), 7},
  {QMI_DSD_GET_MODEM_POWER_COST_RESP_V01, QMI_IDL_TYPE16(0, 20), 14},
  {QMI_DSD_PDN_POLICY_START_TXN_RESP_V01, QMI_IDL_TYPE16(0, 22), 14},
  {QMI_DSD_ADD_PDN_POLICY_RESP_V01, QMI_IDL_TYPE16(0, 24), 7},
  {QMI_DSD_MODIFY_PDN_POLICY_RESP_V01, QMI_IDL_TYPE16(0, 26), 7},
  {QMI_DSD_DELETE_PDN_POLICY_BY_APN_RESP_V01, QMI_IDL_TYPE16(0, 28), 7},
  {QMI_DSD_GET_PDN_POLICY_APN_LIST_RESP_V01, QMI_IDL_TYPE16(0, 30), 1526},
  {QMI_DSD_GET_PDN_POLICY_SETTINGS_FOR_APN_RESP_V01, QMI_IDL_TYPE16(0, 32), 137},
  {QMI_DSD_PDN_POLICY_END_TXN_RESP_V01, QMI_IDL_TYPE16(0, 34), 7},
  {QMI_DSD_SET_APN_INFO_RESP_V01, QMI_IDL_TYPE16(0, 36), 7},
  {QMI_DSD_GET_APN_INFO_RESP_V01, QMI_IDL_TYPE16(0, 38), 110},
  {QMI_DSD_NOTIFY_DATA_SETTING_RESP_V01, QMI_IDL_TYPE16(0, 40), 7},
  {QMI_DSD_GET_DATA_SETTING_RESP_V01, QMI_IDL_TYPE16(0, 42), 15},
  {QMI_DSD_GET_THERMAL_MITIGATION_INFO_RESP_V01, QMI_IDL_TYPE16(0, 45), 14},
  {QMI_DSD_INDICATION_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 47), 7}
};

static const qmi_idl_service_message_table_entry dsd_service_indication_messages_v01[] = {
  {QMI_DSD_SYSTEM_STATUS_IND_V01, QMI_IDL_TYPE16(0, 4), 5378},
  {QMI_DSD_THERMAL_INFO_CHANGE_IND_V01, QMI_IDL_TYPE16(0, 43), 7}
};

/*Service Object*/
struct qmi_idl_service_object dsd_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x2A,
  5385,
  { sizeof(dsd_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(dsd_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(dsd_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { dsd_service_command_messages_v01, dsd_service_response_messages_v01, dsd_service_indication_messages_v01},
  &dsd_qmi_idl_type_table_object_v01,
  0x0B,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type dsd_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( DSD_V01_IDL_MAJOR_VERS != idl_maj_version || DSD_V01_IDL_MINOR_VERS != idl_min_version 
       || DSD_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&dsd_qmi_idl_service_object_v01;
}

