/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q U A L C O M M _ M O B I L E _ A C C E S S _ P O I N T _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the qcmap service Data structures.

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //components/rel/qmimsgs.mpss/2.3/qcmap/src/qualcomm_mobile_access_point_v01.c#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.1 
   It requires encode/decode library version 5 or later
   It was generated on: Wed Mar 27 2013 (Spin 0)
   From IDL File: qualcomm_mobile_access_point_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "qualcomm_mobile_access_point_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t qcmap_get_firewall_list_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_list_type_v01, firewall_handle),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_list_type_v01, start_dest_port),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_list_type_v01, end_dest_port),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_list_type_v01, protocol),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_snat_entry_config_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_snat_entry_config_v01, private_ip_addr),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_snat_entry_config_v01, private_port),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_snat_entry_config_v01, global_port),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_snat_entry_config_v01, protocol),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_ipv4_info_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ipv4_info_v01, subnet_mask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ipv4_info_v01, nat_ip_addr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ipv4_info_v01, nat_dns_addr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ipv4_info_v01, usb_rmnet_ip_addr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ipv4_info_v01, usb_rmnet_gateway_addr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ipv4_info_v01, apps_rmnet_ip_addr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ipv4_info_v01, apps_rmnet_gateway_addr),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_net_policy_info_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(qcmap_net_policy_info_v01, tech_pref),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_net_policy_info_v01, profile_id_3gpp2),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_net_policy_info_v01, profile_id_3gpp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_firewall_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_firewall_type_v01, start_dest_port),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_firewall_type_v01, end_dest_port),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_firewall_type_v01, protocol),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_ip4_addr_subnet_mask_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ip4_addr_subnet_mask_v01, addr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_ip4_addr_subnet_mask_v01, subnet_mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_ip6_addr_prefix_len_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_ip6_addr_prefix_len_v01, addr),
  QCMAP_IPV6_ADDR_LEN_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_ip6_addr_prefix_len_v01, prefix_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_tcp_udp_port_range_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_tcp_udp_port_range_v01, port),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_tcp_udp_port_range_v01, range),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_ip4_tos_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_ip4_tos_v01, value),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_ip4_tos_v01, mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qcmap_ip6_traffic_class_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_ip6_traffic_class_v01, value),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_ip6_traffic_class_v01, mask),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t qcmap_mobile_ap_enable_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, ip_family),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, ip_addr_info) - QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, ip_addr_info_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, ip_addr_info),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, net_policy_info) - QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, net_policy_info_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, net_policy_info),
  3, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, ssid2_ip_addr_info) - QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, ssid2_ip_addr_info_valid)),
  0x12,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, ssid2_ip_addr_info),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, qcmap_nat_type_info) - QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, qcmap_nat_type_info_valid)),
  0x13,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, qcmap_nat_type_info),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, dun_client_ip_addr) - QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, dun_client_ip_addr_valid)),
  0x14,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_req_msg_v01, dun_client_ip_addr)
};

static const uint8_t qcmap_mobile_ap_enable_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_resp_msg_v01, mobile_ap_handle) - QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_resp_msg_v01, mobile_ap_handle_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_enable_resp_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_mobile_ap_disable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_disable_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_mobile_ap_disable_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_mobile_ap_disable_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_bring_up_wwan_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_bring_up_wwan_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_bring_up_wwan_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_bring_up_wwan_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_bring_up_wwan_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_bring_up_wwan_ind_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_bring_up_wwan_ind_msg_v01, ip_family)
};

static const uint8_t qcmap_tear_down_wwan_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_tear_down_wwan_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_tear_down_wwan_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_tear_down_wwan_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_tear_down_wwan_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_tear_down_wwan_ind_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_tear_down_wwan_ind_msg_v01, ip_family)
};

static const uint8_t qcmap_get_wwan_status_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_status_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_wwan_status_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, call_end_reason) - QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, call_end_reason_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, call_end_reason),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, verbose_call_end_reason) - QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, verbose_call_end_reason_valid)),
  0x11,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, verbose_call_end_reason),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, wwan_status) - QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, wwan_status_valid)),
  0x12,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_status_resp_msg_v01, wwan_status)
};

static const uint8_t qcmap_wwan_status_ind_register_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_register_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_register_req_msg_v01, register_indication)
};

static const uint8_t qcmap_wwan_status_ind_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_register_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_wwan_status_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, mobile_ap_handle),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, ip_family),

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, wwan_status),

  0x04,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, reconfig_required),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, call_end_reason) - QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, call_end_reason_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, call_end_reason),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, verbose_call_end_reason) - QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, verbose_call_end_reason_valid)),
  0x11,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_wwan_status_ind_msg_v01, verbose_call_end_reason)
};

static const uint8_t qcmap_set_ipsec_vpn_pass_through_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_ipsec_vpn_pass_through_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_ipsec_vpn_pass_through_req_msg_v01, vpn_pass_through_value)
};

static const uint8_t qcmap_set_ipsec_vpn_pass_through_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_set_ipsec_vpn_pass_through_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_ipsec_vpn_pass_through_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_ipsec_vpn_pass_through_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_ipsec_vpn_pass_through_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_ipsec_vpn_pass_through_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_ipsec_vpn_pass_through_resp_msg_v01, vpn_pass_through_value) - QMI_IDL_OFFSET8(qcmap_get_ipsec_vpn_pass_through_resp_msg_v01, vpn_pass_through_value_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_ipsec_vpn_pass_through_resp_msg_v01, vpn_pass_through_value)
};

static const uint8_t qcmap_set_pptp_vpn_pass_through_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_pptp_vpn_pass_through_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_pptp_vpn_pass_through_req_msg_v01, vpn_pass_through_value)
};

static const uint8_t qcmap_set_pptp_vpn_pass_through_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_set_pptp_vpn_pass_through_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_pptp_vpn_pass_through_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_pptp_vpn_pass_through_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_pptp_vpn_pass_through_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_pptp_vpn_pass_through_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_pptp_vpn_pass_through_resp_msg_v01, vpn_pass_through_value) - QMI_IDL_OFFSET8(qcmap_get_pptp_vpn_pass_through_resp_msg_v01, vpn_pass_through_value_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_pptp_vpn_pass_through_resp_msg_v01, vpn_pass_through_value)
};

static const uint8_t qcmap_set_l2tp_vpn_pass_through_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_l2tp_vpn_pass_through_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_l2tp_vpn_pass_through_req_msg_v01, vpn_pass_through_value)
};

static const uint8_t qcmap_set_l2tp_vpn_pass_through_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_set_l2tp_vpn_pass_through_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_l2tp_vpn_pass_through_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_l2tp_vpn_pass_through_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_l2tp_vpn_pass_through_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_l2tp_vpn_pass_through_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_l2tp_vpn_pass_through_resp_msg_v01, vpn_pass_through_value) - QMI_IDL_OFFSET8(qcmap_get_l2tp_vpn_pass_through_resp_msg_v01, vpn_pass_through_value_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_l2tp_vpn_pass_through_resp_msg_v01, vpn_pass_through_value)
};

static const uint8_t qcmap_set_dynamic_nat_entry_timeout_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_dynamic_nat_entry_timeout_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_dynamic_nat_entry_timeout_req_msg_v01, timeout)
};

static const uint8_t qcmap_set_dynamic_nat_entry_timeout_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_set_dynamic_nat_entry_timeout_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_dynamic_nat_entry_timeout_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_dynamic_nat_entry_timeout_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_dynamic_nat_entry_timeout_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_dynamic_nat_entry_timeout_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_dynamic_nat_entry_timeout_resp_msg_v01, timeout) - QMI_IDL_OFFSET8(qcmap_get_dynamic_nat_entry_timeout_resp_msg_v01, timeout_valid)),
  0x10,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_dynamic_nat_entry_timeout_resp_msg_v01, timeout)
};

static const uint8_t qcmap_add_static_nat_entry_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_static_nat_entry_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_static_nat_entry_req_msg_v01, snat_entry_config),
  1, 0
};

static const uint8_t qcmap_add_static_nat_entry_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_static_nat_entry_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_delete_static_nat_entry_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_delete_static_nat_entry_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_delete_static_nat_entry_req_msg_v01, snat_entry_config),
  1, 0
};

static const uint8_t qcmap_delete_static_nat_entry_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_delete_static_nat_entry_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_static_nat_entries_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_static_nat_entries_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_static_nat_entries_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_static_nat_entries_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_static_nat_entries_resp_msg_v01, snat_config) - QMI_IDL_OFFSET8(qcmap_get_static_nat_entries_resp_msg_v01, snat_config_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_static_nat_entries_resp_msg_v01, snat_config),
  QCMAP_MAX_SNAT_ENTRIES_V01,
  QMI_IDL_OFFSET8(qcmap_get_static_nat_entries_resp_msg_v01, snat_config) - QMI_IDL_OFFSET8(qcmap_get_static_nat_entries_resp_msg_v01, snat_config_len),
  1, 0
};

static const uint8_t qcmap_set_dmz_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_dmz_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_set_dmz_req_msg_v01, dmz_ip_addr)
};

static const uint8_t qcmap_set_dmz_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_set_dmz_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_dmz_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_dmz_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_dmz_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_dmz_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_dmz_resp_msg_v01, dmz_ip_addr) - QMI_IDL_OFFSET8(qcmap_get_dmz_resp_msg_v01, dmz_ip_addr_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_dmz_resp_msg_v01, dmz_ip_addr)
};

static const uint8_t qcmap_delete_dmz_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_delete_dmz_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_delete_dmz_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_delete_dmz_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_wwan_config_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_req_msg_v01, addr_type_op)
};

static const uint8_t qcmap_get_wwan_config_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_addr) - QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_addr_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_addr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_addr) - QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_addr_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_addr),
  QCMAP_IPV6_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_prim_dns_addr) - QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_prim_dns_addr_valid)),
  0x12,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_prim_dns_addr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_sec_dns_addr) - QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_sec_dns_addr_valid)),
  0x13,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v4_sec_dns_addr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_prim_dns_addr) - QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_prim_dns_addr_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_prim_dns_addr),
  QCMAP_IPV6_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_sec_dns_addr) - QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_sec_dns_addr_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_wwan_config_resp_msg_v01, v6_sec_dns_addr),
  QCMAP_IPV6_ADDR_LEN_V01
};

static const uint8_t qcmap_enable_firewall_setting_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_enable_firewall_setting_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_enable_firewall_setting_req_msg_v01, pkts_allowed)
};

static const uint8_t qcmap_enable_firewall_setting_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_enable_firewall_setting_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_firewall_setting_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_setting_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_firewall_setting_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_setting_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_firewall_setting_resp_msg_v01, firewall_enabled) - QMI_IDL_OFFSET8(qcmap_get_firewall_setting_resp_msg_v01, firewall_enabled_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_setting_resp_msg_v01, firewall_enabled),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_firewall_setting_resp_msg_v01, pkts_allowed) - QMI_IDL_OFFSET8(qcmap_get_firewall_setting_resp_msg_v01, pkts_allowed_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_setting_resp_msg_v01, pkts_allowed)
};

static const uint8_t qcmap_disable_firewall_setting_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_disable_firewall_setting_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_disable_firewall_setting_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_disable_firewall_setting_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_add_firewall_config_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_firewall_config_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_firewall_config_req_msg_v01, firewall_config),
  4, 0
};

static const uint8_t qcmap_add_firewall_config_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_firewall_config_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_firewall_config_resp_msg_v01, firewall_handle) - QMI_IDL_OFFSET8(qcmap_add_firewall_config_resp_msg_v01, firewall_handle_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_firewall_config_resp_msg_v01, firewall_handle)
};

static const uint8_t qcmap_delete_firewall_config_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_delete_firewall_config_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_delete_firewall_config_req_msg_v01, firewall_handle)
};

static const uint8_t qcmap_delete_firewall_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_delete_firewall_config_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_firewall_config_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_config_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_firewall_config_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_config_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_firewall_config_resp_msg_v01, firewall_config) - QMI_IDL_OFFSET8(qcmap_get_firewall_config_resp_msg_v01, firewall_config_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_config_resp_msg_v01, firewall_config),
  QCMAP_MAX_FIREWALL_ENTRIES_V01,
  QMI_IDL_OFFSET8(qcmap_get_firewall_config_resp_msg_v01, firewall_config) - QMI_IDL_OFFSET8(qcmap_get_firewall_config_resp_msg_v01, firewall_config_len),
  0, 0
};

static const uint8_t qcmap_station_mode_enable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_station_mode_enable_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_station_mode_enable_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_station_mode_enable_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_station_mode_disable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_station_mode_disable_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_station_mode_disable_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_station_mode_disable_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_station_mode_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_station_mode_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_station_mode_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_station_mode_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_station_mode_resp_msg_v01, station_mode) - QMI_IDL_OFFSET8(qcmap_get_station_mode_resp_msg_v01, station_mode_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_station_mode_resp_msg_v01, station_mode)
};

static const uint8_t qcmap_add_extd_firewall_config_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, mobile_ap_handle),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, next_hdr_prot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, tcp_udp_src) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, tcp_udp_src_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, tcp_udp_src),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, tcp_udp_dst) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, tcp_udp_dst_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, tcp_udp_dst),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, icmp_type) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, icmp_type_valid)),
  0x12,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, icmp_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, icmp_code) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, icmp_code_valid)),
  0x13,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, icmp_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, esp_spi) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, esp_spi_valid)),
  0x14,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, esp_spi),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_src_addr) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_src_addr_valid)),
  0x15,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_src_addr),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_dst_addr) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_dst_addr_valid)),
  0x16,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_dst_addr),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_tos) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_tos_valid)),
  0x17,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip4_tos),
  8, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_src_addr) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_src_addr_valid)),
  0x18,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_src_addr),
  6, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_dst_addr) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_dst_addr_valid)),
  0x19,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_dst_addr),
  6, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_trf_cls) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_trf_cls_valid)),
  0x1A,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_req_msg_v01, ip6_trf_cls),
  9, 0
};

static const uint8_t qcmap_add_extd_firewall_config_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_resp_msg_v01, firewall_handle) - QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_resp_msg_v01, firewall_handle_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_add_extd_firewall_config_resp_msg_v01, firewall_handle)
};

static const uint8_t qcmap_get_extd_firewall_config_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_req_msg_v01, firewall_handle)
};

static const uint8_t qcmap_get_extd_firewall_config_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, next_hdr_prot) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, next_hdr_prot_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, next_hdr_prot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, tcp_udp_src) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, tcp_udp_src_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, tcp_udp_src),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, tcp_udp_dst) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, tcp_udp_dst_valid)),
  0x12,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, tcp_udp_dst),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, icmp_type) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, icmp_type_valid)),
  0x13,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, icmp_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, icmp_code) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, icmp_code_valid)),
  0x14,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, icmp_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, esp_spi) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, esp_spi_valid)),
  0x15,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, esp_spi),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_src_addr) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_src_addr_valid)),
  0x16,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_src_addr),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_dst_addr) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_dst_addr_valid)),
  0x17,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_dst_addr),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_tos) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_tos_valid)),
  0x18,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip4_tos),
  8, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_src_addr) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_src_addr_valid)),
  0x19,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_src_addr),
  6, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_dst_addr) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_dst_addr_valid)),
  0x1A,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_dst_addr),
  6, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_trf_cls) - QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_trf_cls_valid)),
  0x1B,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_extd_firewall_config_resp_msg_v01, ip6_trf_cls),
  9, 0
};

static const uint8_t qcmap_get_firewall_config_handle_list_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_config_handle_list_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_firewall_config_handle_list_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_config_handle_list_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_firewall_config_handle_list_resp_msg_v01, firewall_handle_list) - QMI_IDL_OFFSET8(qcmap_get_firewall_config_handle_list_resp_msg_v01, firewall_handle_list_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_firewall_config_handle_list_resp_msg_v01, firewall_handle_list),
  QCMAP_MAX_FIREWALL_ENTRIES_V01,
  QMI_IDL_OFFSET8(qcmap_get_firewall_config_handle_list_resp_msg_v01, firewall_handle_list) - QMI_IDL_OFFSET8(qcmap_get_firewall_config_handle_list_resp_msg_v01, firewall_handle_list_len)
};

static const uint8_t qcmap_change_nat_type_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_change_nat_type_req_msg_v01, mobile_ap_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_change_nat_type_req_msg_v01, nat_type_option) - QMI_IDL_OFFSET8(qcmap_change_nat_type_req_msg_v01, nat_type_option_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_change_nat_type_req_msg_v01, nat_type_option)
};

static const uint8_t qcmap_change_nat_type_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_change_nat_type_resp_msg_v01, resp),
  0, 1
};

static const uint8_t qcmap_get_nat_type_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_nat_type_req_msg_v01, mobile_ap_handle)
};

static const uint8_t qcmap_get_nat_type_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qcmap_get_nat_type_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qcmap_get_nat_type_resp_msg_v01, nat_type_option) - QMI_IDL_OFFSET8(qcmap_get_nat_type_resp_msg_v01, nat_type_option_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qcmap_get_nat_type_resp_msg_v01, nat_type_option)
};

/* Type Table */
static const qmi_idl_type_table_entry  qcmap_type_table_v01[] = {
  {sizeof(qcmap_get_firewall_list_type_v01), qcmap_get_firewall_list_type_data_v01},
  {sizeof(qcmap_snat_entry_config_v01), qcmap_snat_entry_config_data_v01},
  {sizeof(qcmap_ipv4_info_v01), qcmap_ipv4_info_data_v01},
  {sizeof(qcmap_net_policy_info_v01), qcmap_net_policy_info_data_v01},
  {sizeof(qcmap_firewall_type_v01), qcmap_firewall_type_data_v01},
  {sizeof(qcmap_ip4_addr_subnet_mask_v01), qcmap_ip4_addr_subnet_mask_data_v01},
  {sizeof(qcmap_ip6_addr_prefix_len_v01), qcmap_ip6_addr_prefix_len_data_v01},
  {sizeof(qcmap_tcp_udp_port_range_v01), qcmap_tcp_udp_port_range_data_v01},
  {sizeof(qcmap_ip4_tos_v01), qcmap_ip4_tos_data_v01},
  {sizeof(qcmap_ip6_traffic_class_v01), qcmap_ip6_traffic_class_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry qcmap_message_table_v01[] = {
  {sizeof(qcmap_mobile_ap_enable_req_msg_v01), qcmap_mobile_ap_enable_req_msg_data_v01},
  {sizeof(qcmap_mobile_ap_enable_resp_msg_v01), qcmap_mobile_ap_enable_resp_msg_data_v01},
  {sizeof(qcmap_mobile_ap_disable_req_msg_v01), qcmap_mobile_ap_disable_req_msg_data_v01},
  {sizeof(qcmap_mobile_ap_disable_resp_msg_v01), qcmap_mobile_ap_disable_resp_msg_data_v01},
  {sizeof(qcmap_bring_up_wwan_req_msg_v01), qcmap_bring_up_wwan_req_msg_data_v01},
  {sizeof(qcmap_bring_up_wwan_resp_msg_v01), qcmap_bring_up_wwan_resp_msg_data_v01},
  {sizeof(qcmap_bring_up_wwan_ind_msg_v01), qcmap_bring_up_wwan_ind_msg_data_v01},
  {sizeof(qcmap_tear_down_wwan_req_msg_v01), qcmap_tear_down_wwan_req_msg_data_v01},
  {sizeof(qcmap_tear_down_wwan_resp_msg_v01), qcmap_tear_down_wwan_resp_msg_data_v01},
  {sizeof(qcmap_tear_down_wwan_ind_msg_v01), qcmap_tear_down_wwan_ind_msg_data_v01},
  {sizeof(qcmap_get_wwan_status_req_msg_v01), qcmap_get_wwan_status_req_msg_data_v01},
  {sizeof(qcmap_get_wwan_status_resp_msg_v01), qcmap_get_wwan_status_resp_msg_data_v01},
  {sizeof(qcmap_wwan_status_ind_register_req_msg_v01), qcmap_wwan_status_ind_register_req_msg_data_v01},
  {sizeof(qcmap_wwan_status_ind_register_resp_msg_v01), qcmap_wwan_status_ind_register_resp_msg_data_v01},
  {sizeof(qcmap_wwan_status_ind_msg_v01), qcmap_wwan_status_ind_msg_data_v01},
  {sizeof(qcmap_set_ipsec_vpn_pass_through_req_msg_v01), qcmap_set_ipsec_vpn_pass_through_req_msg_data_v01},
  {sizeof(qcmap_set_ipsec_vpn_pass_through_resp_msg_v01), qcmap_set_ipsec_vpn_pass_through_resp_msg_data_v01},
  {sizeof(qcmap_get_ipsec_vpn_pass_through_req_msg_v01), qcmap_get_ipsec_vpn_pass_through_req_msg_data_v01},
  {sizeof(qcmap_get_ipsec_vpn_pass_through_resp_msg_v01), qcmap_get_ipsec_vpn_pass_through_resp_msg_data_v01},
  {sizeof(qcmap_set_pptp_vpn_pass_through_req_msg_v01), qcmap_set_pptp_vpn_pass_through_req_msg_data_v01},
  {sizeof(qcmap_set_pptp_vpn_pass_through_resp_msg_v01), qcmap_set_pptp_vpn_pass_through_resp_msg_data_v01},
  {sizeof(qcmap_get_pptp_vpn_pass_through_req_msg_v01), qcmap_get_pptp_vpn_pass_through_req_msg_data_v01},
  {sizeof(qcmap_get_pptp_vpn_pass_through_resp_msg_v01), qcmap_get_pptp_vpn_pass_through_resp_msg_data_v01},
  {sizeof(qcmap_set_l2tp_vpn_pass_through_req_msg_v01), qcmap_set_l2tp_vpn_pass_through_req_msg_data_v01},
  {sizeof(qcmap_set_l2tp_vpn_pass_through_resp_msg_v01), qcmap_set_l2tp_vpn_pass_through_resp_msg_data_v01},
  {sizeof(qcmap_get_l2tp_vpn_pass_through_req_msg_v01), qcmap_get_l2tp_vpn_pass_through_req_msg_data_v01},
  {sizeof(qcmap_get_l2tp_vpn_pass_through_resp_msg_v01), qcmap_get_l2tp_vpn_pass_through_resp_msg_data_v01},
  {sizeof(qcmap_set_dynamic_nat_entry_timeout_req_msg_v01), qcmap_set_dynamic_nat_entry_timeout_req_msg_data_v01},
  {sizeof(qcmap_set_dynamic_nat_entry_timeout_resp_msg_v01), qcmap_set_dynamic_nat_entry_timeout_resp_msg_data_v01},
  {sizeof(qcmap_get_dynamic_nat_entry_timeout_req_msg_v01), qcmap_get_dynamic_nat_entry_timeout_req_msg_data_v01},
  {sizeof(qcmap_get_dynamic_nat_entry_timeout_resp_msg_v01), qcmap_get_dynamic_nat_entry_timeout_resp_msg_data_v01},
  {sizeof(qcmap_add_static_nat_entry_req_msg_v01), qcmap_add_static_nat_entry_req_msg_data_v01},
  {sizeof(qcmap_add_static_nat_entry_resp_msg_v01), qcmap_add_static_nat_entry_resp_msg_data_v01},
  {sizeof(qcmap_delete_static_nat_entry_req_msg_v01), qcmap_delete_static_nat_entry_req_msg_data_v01},
  {sizeof(qcmap_delete_static_nat_entry_resp_msg_v01), qcmap_delete_static_nat_entry_resp_msg_data_v01},
  {sizeof(qcmap_get_static_nat_entries_req_msg_v01), qcmap_get_static_nat_entries_req_msg_data_v01},
  {sizeof(qcmap_get_static_nat_entries_resp_msg_v01), qcmap_get_static_nat_entries_resp_msg_data_v01},
  {sizeof(qcmap_set_dmz_req_msg_v01), qcmap_set_dmz_req_msg_data_v01},
  {sizeof(qcmap_set_dmz_resp_msg_v01), qcmap_set_dmz_resp_msg_data_v01},
  {sizeof(qcmap_get_dmz_req_msg_v01), qcmap_get_dmz_req_msg_data_v01},
  {sizeof(qcmap_get_dmz_resp_msg_v01), qcmap_get_dmz_resp_msg_data_v01},
  {sizeof(qcmap_delete_dmz_req_msg_v01), qcmap_delete_dmz_req_msg_data_v01},
  {sizeof(qcmap_delete_dmz_resp_msg_v01), qcmap_delete_dmz_resp_msg_data_v01},
  {sizeof(qcmap_get_wwan_config_req_msg_v01), qcmap_get_wwan_config_req_msg_data_v01},
  {sizeof(qcmap_get_wwan_config_resp_msg_v01), qcmap_get_wwan_config_resp_msg_data_v01},
  {sizeof(qcmap_enable_firewall_setting_req_msg_v01), qcmap_enable_firewall_setting_req_msg_data_v01},
  {sizeof(qcmap_enable_firewall_setting_resp_msg_v01), qcmap_enable_firewall_setting_resp_msg_data_v01},
  {sizeof(qcmap_get_firewall_setting_req_msg_v01), qcmap_get_firewall_setting_req_msg_data_v01},
  {sizeof(qcmap_get_firewall_setting_resp_msg_v01), qcmap_get_firewall_setting_resp_msg_data_v01},
  {sizeof(qcmap_disable_firewall_setting_req_msg_v01), qcmap_disable_firewall_setting_req_msg_data_v01},
  {sizeof(qcmap_disable_firewall_setting_resp_msg_v01), qcmap_disable_firewall_setting_resp_msg_data_v01},
  {sizeof(qcmap_add_firewall_config_req_msg_v01), qcmap_add_firewall_config_req_msg_data_v01},
  {sizeof(qcmap_add_firewall_config_resp_msg_v01), qcmap_add_firewall_config_resp_msg_data_v01},
  {sizeof(qcmap_delete_firewall_config_req_msg_v01), qcmap_delete_firewall_config_req_msg_data_v01},
  {sizeof(qcmap_delete_firewall_config_resp_msg_v01), qcmap_delete_firewall_config_resp_msg_data_v01},
  {sizeof(qcmap_get_firewall_config_req_msg_v01), qcmap_get_firewall_config_req_msg_data_v01},
  {sizeof(qcmap_get_firewall_config_resp_msg_v01), qcmap_get_firewall_config_resp_msg_data_v01},
  {sizeof(qcmap_station_mode_enable_req_msg_v01), qcmap_station_mode_enable_req_msg_data_v01},
  {sizeof(qcmap_station_mode_enable_resp_msg_v01), qcmap_station_mode_enable_resp_msg_data_v01},
  {sizeof(qcmap_station_mode_disable_req_msg_v01), qcmap_station_mode_disable_req_msg_data_v01},
  {sizeof(qcmap_station_mode_disable_resp_msg_v01), qcmap_station_mode_disable_resp_msg_data_v01},
  {sizeof(qcmap_get_station_mode_req_msg_v01), qcmap_get_station_mode_req_msg_data_v01},
  {sizeof(qcmap_get_station_mode_resp_msg_v01), qcmap_get_station_mode_resp_msg_data_v01},
  {sizeof(qcmap_add_extd_firewall_config_req_msg_v01), qcmap_add_extd_firewall_config_req_msg_data_v01},
  {sizeof(qcmap_add_extd_firewall_config_resp_msg_v01), qcmap_add_extd_firewall_config_resp_msg_data_v01},
  {sizeof(qcmap_get_extd_firewall_config_req_msg_v01), qcmap_get_extd_firewall_config_req_msg_data_v01},
  {sizeof(qcmap_get_extd_firewall_config_resp_msg_v01), qcmap_get_extd_firewall_config_resp_msg_data_v01},
  {sizeof(qcmap_get_firewall_config_handle_list_req_msg_v01), qcmap_get_firewall_config_handle_list_req_msg_data_v01},
  {sizeof(qcmap_get_firewall_config_handle_list_resp_msg_v01), qcmap_get_firewall_config_handle_list_resp_msg_data_v01},
  {sizeof(qcmap_change_nat_type_req_msg_v01), qcmap_change_nat_type_req_msg_data_v01},
  {sizeof(qcmap_change_nat_type_resp_msg_v01), qcmap_change_nat_type_resp_msg_data_v01},
  {sizeof(qcmap_get_nat_type_req_msg_v01), qcmap_get_nat_type_req_msg_data_v01},
  {sizeof(qcmap_get_nat_type_resp_msg_v01), qcmap_get_nat_type_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object qcmap_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *qcmap_qmi_idl_type_table_object_referenced_tables_v01[] =
{&qcmap_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object qcmap_qmi_idl_type_table_object_v01 = {
  sizeof(qcmap_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(qcmap_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  qcmap_type_table_v01,
  qcmap_message_table_v01,
  qcmap_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry qcmap_service_command_messages_v01[] = {
  {QMI_QCMAP_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_QCMAP_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_QCMAP_MOBILE_AP_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 0), 76},
  {QMI_QCMAP_MOBILE_AP_DISABLE_REQ_V01, QMI_IDL_TYPE16(0, 2), 7},
  {QMI_QCMAP_BRING_UP_WWAN_REQ_V01, QMI_IDL_TYPE16(0, 4), 7},
  {QMI_QCMAP_TEAR_DOWN_WWAN_REQ_V01, QMI_IDL_TYPE16(0, 7), 7},
  {QMI_QCMAP_GET_WWAN_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 10), 7},
  {QMI_QCMAP_GET_IPSEC_VPN_PASS_THROUGH_REQ_V01, QMI_IDL_TYPE16(0, 17), 7},
  {QMI_QCMAP_SET_IPSEC_VPN_PASS_THROUGH_REQ_V01, QMI_IDL_TYPE16(0, 15), 11},
  {QMI_QCMAP_GET_PPTP_VPN_PASS_THROUGH_REQ_V01, QMI_IDL_TYPE16(0, 21), 7},
  {QMI_QCMAP_SET_PPTP_VPN_PASS_THROUGH_REQ_V01, QMI_IDL_TYPE16(0, 19), 11},
  {QMI_QCMAP_GET_L2TP_VPN_PASS_THROUGH_REQ_V01, QMI_IDL_TYPE16(0, 25), 7},
  {QMI_QCMAP_SET_L2TP_VPN_PASS_THROUGH_REQ_V01, QMI_IDL_TYPE16(0, 23), 11},
  {QMI_QCMAP_GET_DYNAMIC_NAT_ENTRY_TIMEOUT_REQ_V01, QMI_IDL_TYPE16(0, 29), 7},
  {QMI_QCMAP_SET_DYNAMIC_NAT_ENTRY_TIMEOUT_REQ_V01, QMI_IDL_TYPE16(0, 27), 12},
  {QMI_QCMAP_ADD_STATIC_NAT_ENTRY_REQ_V01, QMI_IDL_TYPE16(0, 31), 19},
  {QMI_QCMAP_DELETE_STATIC_NAT_ENTRY_REQ_V01, QMI_IDL_TYPE16(0, 33), 19},
  {QMI_QCMAP_GET_STATIC_NAT_ENTRIES_REQ_V01, QMI_IDL_TYPE16(0, 35), 7},
  {QMI_QCMAP_SET_DMZ_REQ_V01, QMI_IDL_TYPE16(0, 37), 14},
  {QMI_QCMAP_DELETE_DMZ_REQ_V01, QMI_IDL_TYPE16(0, 41), 7},
  {QMI_QCMAP_GET_DMZ_REQ_V01, QMI_IDL_TYPE16(0, 39), 7},
  {QMI_QCMAP_GET_WWAN_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 43), 18},
  {QMI_QCMAP_ENABLE_FIREWALL_SETTING_REQ_V01, QMI_IDL_TYPE16(0, 45), 11},
  {QMI_QCMAP_GET_FIREWALL_SETTING_REQ_V01, QMI_IDL_TYPE16(0, 47), 7},
  {QMI_QCMAP_DISABLE_FIREWALL_SETTING_REQ_V01, QMI_IDL_TYPE16(0, 49), 7},
  {QMI_QCMAP_ADD_FIREWALL_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 51), 15},
  {QMI_QCMAP_GET_FIREWALL_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 55), 7},
  {QMI_QCMAP_DELETE_FIREWALL_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 53), 14},
  {QMI_QCMAP_WWAN_STATUS_IND_REG_REQ_V01, QMI_IDL_TYPE16(0, 12), 11},
  {QMI_QCMAP_STATION_MODE_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 57), 7},
  {QMI_QCMAP_STATION_MODE_DISABLE_REQ_V01, QMI_IDL_TYPE16(0, 59), 7},
  {QMI_QCMAP_GET_STATION_MODE_REQ_V01, QMI_IDL_TYPE16(0, 61), 7},
  {QMI_QCMAP_ADD_EXTD_FIREWALL_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 63), 115},
  {QMI_QCMAP_GET_EXTD_FIREWALL_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 65), 14},
  {QMI_QCMAP_GET_FIREWALL_CONFIG_HANDLE_LIST_REQ_V01, QMI_IDL_TYPE16(0, 67), 7},
  {QMI_QCMAP_CHANGE_NAT_TYPE_REQ_V01, QMI_IDL_TYPE16(0, 69), 14},
  {QMI_QCMAP_GET_NAT_TYPE_REQ_V01, QMI_IDL_TYPE16(0, 71), 7}
};

static const qmi_idl_service_message_table_entry qcmap_service_response_messages_v01[] = {
  {QMI_QCMAP_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_QCMAP_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_QCMAP_MOBILE_AP_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 1), 14},
  {QMI_QCMAP_MOBILE_AP_DISABLE_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_QCMAP_BRING_UP_WWAN_RESP_V01, QMI_IDL_TYPE16(0, 5), 7},
  {QMI_QCMAP_TEAR_DOWN_WWAN_RESP_V01, QMI_IDL_TYPE16(0, 8), 7},
  {QMI_QCMAP_GET_WWAN_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 11), 28},
  {QMI_QCMAP_GET_IPSEC_VPN_PASS_THROUGH_RESP_V01, QMI_IDL_TYPE16(0, 18), 11},
  {QMI_QCMAP_SET_IPSEC_VPN_PASS_THROUGH_RESP_V01, QMI_IDL_TYPE16(0, 16), 7},
  {QMI_QCMAP_GET_PPTP_VPN_PASS_THROUGH_RESP_V01, QMI_IDL_TYPE16(0, 22), 11},
  {QMI_QCMAP_SET_PPTP_VPN_PASS_THROUGH_RESP_V01, QMI_IDL_TYPE16(0, 20), 7},
  {QMI_QCMAP_GET_L2TP_VPN_PASS_THROUGH_RESP_V01, QMI_IDL_TYPE16(0, 26), 11},
  {QMI_QCMAP_SET_L2TP_VPN_PASS_THROUGH_RESP_V01, QMI_IDL_TYPE16(0, 24), 7},
  {QMI_QCMAP_GET_DYNAMIC_NAT_ENTRY_TIMEOUT_RESP_V01, QMI_IDL_TYPE16(0, 30), 12},
  {QMI_QCMAP_SET_DYNAMIC_NAT_ENTRY_TIMEOUT_RESP_V01, QMI_IDL_TYPE16(0, 28), 7},
  {QMI_QCMAP_ADD_STATIC_NAT_ENTRY_RESP_V01, QMI_IDL_TYPE16(0, 32), 7},
  {QMI_QCMAP_DELETE_STATIC_NAT_ENTRY_RESP_V01, QMI_IDL_TYPE16(0, 34), 7},
  {QMI_QCMAP_GET_STATIC_NAT_ENTRIES_RESP_V01, QMI_IDL_TYPE16(0, 36), 461},
  {QMI_QCMAP_SET_DMZ_RESP_V01, QMI_IDL_TYPE16(0, 38), 7},
  {QMI_QCMAP_DELETE_DMZ_RESP_V01, QMI_IDL_TYPE16(0, 42), 7},
  {QMI_QCMAP_GET_DMZ_RESP_V01, QMI_IDL_TYPE16(0, 40), 14},
  {QMI_QCMAP_GET_WWAN_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 44), 85},
  {QMI_QCMAP_ENABLE_FIREWALL_SETTING_RESP_V01, QMI_IDL_TYPE16(0, 46), 7},
  {QMI_QCMAP_GET_FIREWALL_SETTING_RESP_V01, QMI_IDL_TYPE16(0, 48), 15},
  {QMI_QCMAP_DISABLE_FIREWALL_SETTING_RESP_V01, QMI_IDL_TYPE16(0, 50), 7},
  {QMI_QCMAP_ADD_FIREWALL_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 52), 14},
  {QMI_QCMAP_GET_FIREWALL_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 56), 461},
  {QMI_QCMAP_DELETE_FIREWALL_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 54), 7},
  {QMI_QCMAP_WWAN_STATUS_IND_REG_RESP_V01, QMI_IDL_TYPE16(0, 13), 7},
  {QMI_QCMAP_STATION_MODE_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 58), 7},
  {QMI_QCMAP_STATION_MODE_DISABLE_RESP_V01, QMI_IDL_TYPE16(0, 60), 7},
  {QMI_QCMAP_GET_STATION_MODE_RESP_V01, QMI_IDL_TYPE16(0, 62), 11},
  {QMI_QCMAP_ADD_EXTD_FIREWALL_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 64), 14},
  {QMI_QCMAP_GET_EXTD_FIREWALL_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 66), 115},
  {QMI_QCMAP_GET_FIREWALL_CONFIG_HANDLE_LIST_RESP_V01, QMI_IDL_TYPE16(0, 68), 211},
  {QMI_QCMAP_CHANGE_NAT_TYPE_RESP_V01, QMI_IDL_TYPE16(0, 70), 7},
  {QMI_QCMAP_GET_NAT_TYPE_RESP_V01, QMI_IDL_TYPE16(0, 72), 14}
};

static const qmi_idl_service_message_table_entry qcmap_service_indication_messages_v01[] = {
  {QMI_QCMAP_BRING_UP_WWAN_IND_V01, QMI_IDL_TYPE16(0, 6), 14},
  {QMI_QCMAP_TEAR_DOWN_WWAN_IND_V01, QMI_IDL_TYPE16(0, 9), 14},
  {QMI_QCMAP_WWAN_STATUS_IND_V01, QMI_IDL_TYPE16(0, 14), 39}
};

/*Service Object*/
struct qmi_idl_service_object qcmap_qmi_idl_service_object_v01 = {
  0x05,
  0x01,
  0x1E,
  8204,
  { sizeof(qcmap_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(qcmap_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(qcmap_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { qcmap_service_command_messages_v01, qcmap_service_response_messages_v01, qcmap_service_indication_messages_v01},
  &qcmap_qmi_idl_type_table_object_v01,
  0x06,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type qcmap_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( QCMAP_V01_IDL_MAJOR_VERS != idl_maj_version || QCMAP_V01_IDL_MINOR_VERS != idl_min_version 
       || QCMAP_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&qcmap_qmi_idl_service_object_v01;
}

