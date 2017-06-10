/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        D A T A _ F I L T E R _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the dfs service Data structures.

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.



  $Header: //source/qcom/qct/interfaces/qmi/dfs/main/latest/src/data_filter_service_v01.c#12 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.5 
   It was generated on: Fri Jun 19 2015 (Spin 0)
   From IDL File: data_filter_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "data_filter_service_v01.h"
#include "common_v01.h"
#include "data_common_v01.h"


/*Type Definitions*/
static const uint8_t dfs_filter_capability_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_filter_capability_type_v01, max_filters_supported),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_filter_capability_type_v01, max_filters_supported_per_add),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_ipv6_addr_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv6_addr_type_v01, ipv6_address),
  QMI_DFS_IPV6_ADDR_LEN_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv6_addr_type_v01, prefix_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_ipv4_addr_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv4_addr_type_v01, ipv4_addr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv4_addr_type_v01, subnet_mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_ipv4_tos_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv4_tos_type_v01, val),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv4_tos_type_v01, mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_ipv4_info_type_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv4_info_type_v01, valid_params),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_ipv4_info_type_v01, src_addr),
  QMI_IDL_TYPE88(0, 2),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_ipv4_info_type_v01, dest_addr),
  QMI_IDL_TYPE88(0, 2),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_ipv4_info_type_v01, tos),
  QMI_IDL_TYPE88(0, 3),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_ipv6_trf_cls_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv6_trf_cls_type_v01, val),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv6_trf_cls_type_v01, mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_ipv6_info_type_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv6_info_type_v01, valid_params),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_ipv6_info_type_v01, src_addr),
  QMI_IDL_TYPE88(0, 1),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_ipv6_info_type_v01, dest_addr),
  QMI_IDL_TYPE88(0, 1),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_ipv6_info_type_v01, trf_cls),
  QMI_IDL_TYPE88(0, 5),
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_ipv6_info_type_v01, flow_label),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_ip_header_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dfs_ip_header_type_v01, ip_version),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_ip_header_type_v01, v4_info),
  QMI_IDL_TYPE88(0, 4),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_ip_header_type_v01, v6_info),
  QMI_IDL_TYPE88(0, 6),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_port_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dfs_port_type_v01, port),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dfs_port_type_v01, range),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_port_info_type_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dfs_port_info_type_v01, valid_params),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_port_info_type_v01, src_port_info),
  QMI_IDL_TYPE88(0, 8),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_port_info_type_v01, dest_port_info),
  QMI_IDL_TYPE88(0, 8),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_icmp_info_type_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dfs_icmp_info_type_v01, valid_params),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_icmp_info_type_v01, type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_icmp_info_type_v01, code),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_ipsec_info_type_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dfs_ipsec_info_type_v01, valid_params),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_ipsec_info_type_v01, spi),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_xport_header_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_xport_header_type_v01, xport_protocol),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_xport_header_type_v01, tcp_info),
  QMI_IDL_TYPE88(0, 9),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_xport_header_type_v01, udp_info),
  QMI_IDL_TYPE88(0, 9),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_xport_header_type_v01, icmp_info),
  QMI_IDL_TYPE88(0, 10),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_xport_header_type_v01, esp_info),
  QMI_IDL_TYPE88(0, 11),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_xport_header_type_v01, ah_info),
  QMI_IDL_TYPE88(0, 11),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_filter_rule_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_filter_rule_type_v01, ip_info),
  QMI_IDL_TYPE88(0, 7),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_filter_rule_type_v01, xport_info),
  QMI_IDL_TYPE88(0, 12),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_request_socket_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dfs_request_socket_info_type_v01, ip_family),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_request_socket_info_type_v01, xport_prot),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dfs_request_socket_info_type_v01, port_no),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t dfs_allocated_socket_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_allocated_socket_info_type_v01, status),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_allocated_socket_info_type_v01, socket_handle),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_allocated_socket_info_type_v01, is_ephemeral),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_allocated_socket_info_type_v01, socket_info),
  QMI_IDL_TYPE88(0, 14),
  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/*
 * dfs_get_filter_capability_req_msg is empty
 * static const uint8_t dfs_get_filter_capability_req_msg_data_v01[] = {
 * };
 */

static const uint8_t dfs_get_filter_capability_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_media_offload_filters) - QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_media_offload_filters_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_media_offload_filters),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_pdn_sharing_filters) - QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_pdn_sharing_filters_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_pdn_sharing_filters),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_powersave_filters) - QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_powersave_filters_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_powersave_filters),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_low_latency_filters) - QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_low_latency_filters_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_filter_capability_resp_msg_v01, max_low_latency_filters),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t dfs_bind_client_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, data_port) - QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, data_port_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, data_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, ip_preference) - QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, ip_preference_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, ip_preference),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, ep_id) - QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, ep_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, ep_id),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, mux_id) - QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, mux_id_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, mux_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, bind_subs) - QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, bind_subs_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_bind_client_req_msg_v01, bind_subs)
};

static const uint8_t dfs_bind_client_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_bind_client_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * dfs_get_client_binding_req_msg is empty
 * static const uint8_t dfs_get_client_binding_req_msg_data_v01[] = {
 * };
 */

static const uint8_t dfs_get_client_binding_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, data_port) - QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, data_port_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, data_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, ip_preference) - QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, ip_preference_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, ip_preference),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_ep_id) - QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_ep_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_ep_id),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_mux_id) - QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_mux_id_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_mux_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_subs) - QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_subs_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_get_client_binding_resp_msg_v01, bound_subs)
};

static const uint8_t dfs_add_media_offload_filter_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, filter_id) - QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, filter_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, filter_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, ipv4_dest_address) - QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, ipv4_dest_address_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, ipv4_dest_address),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, ipv6_dest_address) - QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, ipv6_dest_address_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, ipv6_dest_address),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, xport_protocol) - QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, xport_protocol_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, xport_protocol),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, udp_dest_port) - QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, udp_dest_port_valid)),
  0x14,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(dfs_add_media_offload_filter_req_msg_v01, udp_dest_port)
};

static const uint8_t dfs_add_media_offload_filter_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_add_media_offload_filter_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_media_offload_filter_resp_msg_v01, filter_handle) - QMI_IDL_OFFSET8(dfs_add_media_offload_filter_resp_msg_v01, filter_handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_add_media_offload_filter_resp_msg_v01, filter_handle)
};

static const uint8_t dfs_remove_media_offload_filter_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_remove_media_offload_filter_req_msg_v01, filter_handle)
};

static const uint8_t dfs_remove_media_offload_filter_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_remove_media_offload_filter_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dfs_get_media_offload_statistics_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_req_msg_v01, filter_handle)
};

static const uint8_t dfs_get_media_offload_statistics_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, bytes_sent) - QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, bytes_sent_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, bytes_sent),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, bytes_received) - QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, bytes_received_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, bytes_received),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, packets_sent) - QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, packets_sent_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, packets_sent),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, packets_received) - QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, packets_received_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_get_media_offload_statistics_resp_msg_v01, packets_received)
};

static const uint8_t dfs_add_pdn_sharing_filters_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_req_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_req_msg_v01, filter_rules_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_req_msg_v01, filter_rules),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_req_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_req_msg_v01, filter_rules_len),
  QMI_IDL_TYPE88(0, 13)
};

static const uint8_t dfs_add_pdn_sharing_filters_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_handles) - QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_handles_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_handles),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_handles) - QMI_IDL_OFFSET8(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_handles_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_rule_error) - QMI_IDL_OFFSET16RELATIVE(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_rule_error_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_rule_error),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET16RELATIVE(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_rule_error) - QMI_IDL_OFFSET16RELATIVE(dfs_add_pdn_sharing_filters_resp_msg_v01, filter_rule_error_len)
};

static const uint8_t dfs_remove_filters_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_remove_filters_req_msg_v01, filter_handles),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_remove_filters_req_msg_v01, filter_handles) - QMI_IDL_OFFSET8(dfs_remove_filters_req_msg_v01, filter_handles_len)
};

static const uint8_t dfs_remove_filters_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_remove_filters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dfs_add_powersave_filters_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_powersave_filters_req_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_add_powersave_filters_req_msg_v01, filter_rules_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_add_powersave_filters_req_msg_v01, filter_rules),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_add_powersave_filters_req_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_add_powersave_filters_req_msg_v01, filter_rules_len),
  QMI_IDL_TYPE88(0, 13)
};

static const uint8_t dfs_add_powersave_filters_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_add_powersave_filters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_powersave_filters_resp_msg_v01, filter_handles) - QMI_IDL_OFFSET8(dfs_add_powersave_filters_resp_msg_v01, filter_handles_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_add_powersave_filters_resp_msg_v01, filter_handles),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_add_powersave_filters_resp_msg_v01, filter_handles) - QMI_IDL_OFFSET8(dfs_add_powersave_filters_resp_msg_v01, filter_handles_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dfs_add_powersave_filters_resp_msg_v01, filter_rule_error) - QMI_IDL_OFFSET16RELATIVE(dfs_add_powersave_filters_resp_msg_v01, filter_rule_error_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(dfs_add_powersave_filters_resp_msg_v01, filter_rule_error),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET16RELATIVE(dfs_add_powersave_filters_resp_msg_v01, filter_rule_error) - QMI_IDL_OFFSET16RELATIVE(dfs_add_powersave_filters_resp_msg_v01, filter_rule_error_len)
};

static const uint8_t dfs_set_powersave_filter_mode_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_set_powersave_filter_mode_req_msg_v01, powersave_filter_mode)
};

static const uint8_t dfs_set_powersave_filter_mode_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_set_powersave_filter_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * dfs_get_powersave_filter_mode_req_msg is empty
 * static const uint8_t dfs_get_powersave_filter_mode_req_msg_data_v01[] = {
 * };
 */

static const uint8_t dfs_get_powersave_filter_mode_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_powersave_filter_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_powersave_filter_mode_resp_msg_v01, powersave_filter_mode) - QMI_IDL_OFFSET8(dfs_get_powersave_filter_mode_resp_msg_v01, powersave_filter_mode_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_get_powersave_filter_mode_resp_msg_v01, powersave_filter_mode)
};

static const uint8_t dfs_set_autoexit_powersave_filter_mode_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_set_autoexit_powersave_filter_mode_req_msg_v01, autoexit_powersave_filter_mode)
};

static const uint8_t dfs_set_autoexit_powersave_filter_mode_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_set_autoexit_powersave_filter_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dfs_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_powersave_filter_mode_change) - QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_powersave_filter_mode_change_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_powersave_filter_mode_change),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_low_latency_traffic) - QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_low_latency_traffic_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_low_latency_traffic),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_reverse_ip_transport_filters_update) - QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_reverse_ip_transport_filters_update_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, report_reverse_ip_transport_filters_update),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, remote_socket_handling) - QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, remote_socket_handling_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_indication_register_req_msg_v01, remote_socket_handling)
};

static const uint8_t dfs_indication_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dfs_powersave_filter_mode_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_powersave_filter_mode_ind_msg_v01, powersave_filter_mode)
};

/*
 * dfs_remove_all_powersave_filters_req_msg is empty
 * static const uint8_t dfs_remove_all_powersave_filters_req_msg_data_v01[] = {
 * };
 */

static const uint8_t dfs_remove_all_powersave_filters_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_remove_all_powersave_filters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dfs_add_low_latency_filters_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_low_latency_filters_req_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_add_low_latency_filters_req_msg_v01, filter_rules_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_add_low_latency_filters_req_msg_v01, filter_rules),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_add_low_latency_filters_req_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_add_low_latency_filters_req_msg_v01, filter_rules_len),
  QMI_IDL_TYPE88(0, 13),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_req_msg_v01, pkt_inter_arrival_time) - QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_req_msg_v01, pkt_inter_arrival_time_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(dfs_add_low_latency_filters_req_msg_v01, pkt_inter_arrival_time),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_req_msg_v01, pkt_inter_arrival_time) - QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_req_msg_v01, pkt_inter_arrival_time_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_req_msg_v01, filter_direction) - QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_req_msg_v01, filter_direction_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(dfs_add_low_latency_filters_req_msg_v01, filter_direction)
};

static const uint8_t dfs_add_low_latency_filters_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_add_low_latency_filters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_add_low_latency_filters_resp_msg_v01, filter_handles) - QMI_IDL_OFFSET8(dfs_add_low_latency_filters_resp_msg_v01, filter_handles_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_add_low_latency_filters_resp_msg_v01, filter_handles),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_add_low_latency_filters_resp_msg_v01, filter_handles) - QMI_IDL_OFFSET8(dfs_add_low_latency_filters_resp_msg_v01, filter_handles_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_resp_msg_v01, filter_rule_error) - QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_resp_msg_v01, filter_rule_error_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(dfs_add_low_latency_filters_resp_msg_v01, filter_rule_error),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_resp_msg_v01, filter_rule_error) - QMI_IDL_OFFSET16RELATIVE(dfs_add_low_latency_filters_resp_msg_v01, filter_rule_error_len)
};

static const uint8_t dfs_low_latency_traffic_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_low_latency_traffic_ind_msg_v01, traffic_start),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_low_latency_traffic_ind_msg_v01, filter_handle)
};

/*
 * dfs_get_reverse_ip_transport_filters_req_msg is empty
 * static const uint8_t dfs_get_reverse_ip_transport_filters_req_msg_data_v01[] = {
 * };
 */

static const uint8_t dfs_get_reverse_ip_transport_filters_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_reverse_ip_transport_filters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_reverse_ip_transport_filters_resp_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_get_reverse_ip_transport_filters_resp_msg_v01, filter_rules_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_reverse_ip_transport_filters_resp_msg_v01, filter_rules),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_get_reverse_ip_transport_filters_resp_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_get_reverse_ip_transport_filters_resp_msg_v01, filter_rules_len),
  QMI_IDL_TYPE88(0, 13)
};

static const uint8_t dfs_reverse_ip_transport_filters_updated_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_reverse_ip_transport_filters_updated_ind_msg_v01, filter_action),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_reverse_ip_transport_filters_updated_ind_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_reverse_ip_transport_filters_updated_ind_msg_v01, filter_rules_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_reverse_ip_transport_filters_updated_ind_msg_v01, filter_rules),
  QMI_DFS_MAX_FILTERS_V01,
  QMI_IDL_OFFSET8(dfs_reverse_ip_transport_filters_updated_ind_msg_v01, filter_rules) - QMI_IDL_OFFSET8(dfs_reverse_ip_transport_filters_updated_ind_msg_v01, filter_rules_len),
  QMI_IDL_TYPE88(0, 13)
};

static const uint8_t dfs_remote_socket_request_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_remote_socket_request_ind_msg_v01, request_socket_list),
  QMI_DFS_MAX_ALLOCATED_SOCKETS_V01,
  QMI_IDL_OFFSET8(dfs_remote_socket_request_ind_msg_v01, request_socket_list) - QMI_IDL_OFFSET8(dfs_remote_socket_request_ind_msg_v01, request_socket_list_len),
  QMI_IDL_TYPE88(0, 14)
};

static const uint8_t dfs_remote_socket_allocated_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_remote_socket_allocated_req_msg_v01, socket_list),
  QMI_DFS_MAX_ALLOCATED_SOCKETS_V01,
  QMI_IDL_OFFSET8(dfs_remote_socket_allocated_req_msg_v01, socket_list) - QMI_IDL_OFFSET8(dfs_remote_socket_allocated_req_msg_v01, socket_list_len),
  QMI_IDL_TYPE88(0, 15)
};

static const uint8_t dfs_remote_socket_allocated_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_remote_socket_allocated_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t dfs_remote_socket_release_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_remote_socket_release_ind_msg_v01, socket_handles) - QMI_IDL_OFFSET8(dfs_remote_socket_release_ind_msg_v01, socket_handles_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_remote_socket_release_ind_msg_v01, socket_handles),
  QMI_DFS_MAX_ALLOCATED_SOCKETS_V01,
  QMI_IDL_OFFSET8(dfs_remote_socket_release_ind_msg_v01, socket_handles) - QMI_IDL_OFFSET8(dfs_remote_socket_release_ind_msg_v01, socket_handles_len)
};

static const uint8_t dfs_remote_socket_set_option_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(dfs_remote_socket_set_option_ind_msg_v01, socket_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_remote_socket_set_option_ind_msg_v01, is_udp_encaps) - QMI_IDL_OFFSET8(dfs_remote_socket_set_option_ind_msg_v01, is_udp_encaps_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_remote_socket_set_option_ind_msg_v01, is_udp_encaps)
};

/* 
 * dfs_get_capability_req_msg is empty
 * static const uint8_t dfs_get_capability_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t dfs_get_capability_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(dfs_get_capability_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(dfs_get_capability_resp_msg_v01, remote_socket_capability) - QMI_IDL_OFFSET8(dfs_get_capability_resp_msg_v01, remote_socket_capability_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(dfs_get_capability_resp_msg_v01, remote_socket_capability)
};

/* Type Table */
static const qmi_idl_type_table_entry  dfs_type_table_v01[] = {
  {sizeof(dfs_filter_capability_type_v01), dfs_filter_capability_type_data_v01},
  {sizeof(dfs_ipv6_addr_type_v01), dfs_ipv6_addr_type_data_v01},
  {sizeof(dfs_ipv4_addr_type_v01), dfs_ipv4_addr_type_data_v01},
  {sizeof(dfs_ipv4_tos_type_v01), dfs_ipv4_tos_type_data_v01},
  {sizeof(dfs_ipv4_info_type_v01), dfs_ipv4_info_type_data_v01},
  {sizeof(dfs_ipv6_trf_cls_type_v01), dfs_ipv6_trf_cls_type_data_v01},
  {sizeof(dfs_ipv6_info_type_v01), dfs_ipv6_info_type_data_v01},
  {sizeof(dfs_ip_header_type_v01), dfs_ip_header_type_data_v01},
  {sizeof(dfs_port_type_v01), dfs_port_type_data_v01},
  {sizeof(dfs_port_info_type_v01), dfs_port_info_type_data_v01},
  {sizeof(dfs_icmp_info_type_v01), dfs_icmp_info_type_data_v01},
  {sizeof(dfs_ipsec_info_type_v01), dfs_ipsec_info_type_data_v01},
  {sizeof(dfs_xport_header_type_v01), dfs_xport_header_type_data_v01},
  {sizeof(dfs_filter_rule_type_v01), dfs_filter_rule_type_data_v01},
  {sizeof(dfs_request_socket_info_type_v01), dfs_request_socket_info_type_data_v01},
  {sizeof(dfs_allocated_socket_info_type_v01), dfs_allocated_socket_info_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry dfs_message_table_v01[] = {
  {sizeof(dfs_get_filter_capability_req_msg_v01), 0},
  {sizeof(dfs_get_filter_capability_resp_msg_v01), dfs_get_filter_capability_resp_msg_data_v01},
  {sizeof(dfs_bind_client_req_msg_v01), dfs_bind_client_req_msg_data_v01},
  {sizeof(dfs_bind_client_resp_msg_v01), dfs_bind_client_resp_msg_data_v01},
  {sizeof(dfs_get_client_binding_req_msg_v01), 0},
  {sizeof(dfs_get_client_binding_resp_msg_v01), dfs_get_client_binding_resp_msg_data_v01},
  {sizeof(dfs_add_media_offload_filter_req_msg_v01), dfs_add_media_offload_filter_req_msg_data_v01},
  {sizeof(dfs_add_media_offload_filter_resp_msg_v01), dfs_add_media_offload_filter_resp_msg_data_v01},
  {sizeof(dfs_remove_media_offload_filter_req_msg_v01), dfs_remove_media_offload_filter_req_msg_data_v01},
  {sizeof(dfs_remove_media_offload_filter_resp_msg_v01), dfs_remove_media_offload_filter_resp_msg_data_v01},
  {sizeof(dfs_get_media_offload_statistics_req_msg_v01), dfs_get_media_offload_statistics_req_msg_data_v01},
  {sizeof(dfs_get_media_offload_statistics_resp_msg_v01), dfs_get_media_offload_statistics_resp_msg_data_v01},
  {sizeof(dfs_add_pdn_sharing_filters_req_msg_v01), dfs_add_pdn_sharing_filters_req_msg_data_v01},
  {sizeof(dfs_add_pdn_sharing_filters_resp_msg_v01), dfs_add_pdn_sharing_filters_resp_msg_data_v01},
  {sizeof(dfs_remove_filters_req_msg_v01), dfs_remove_filters_req_msg_data_v01},
  {sizeof(dfs_remove_filters_resp_msg_v01), dfs_remove_filters_resp_msg_data_v01},
  {sizeof(dfs_add_powersave_filters_req_msg_v01), dfs_add_powersave_filters_req_msg_data_v01},
  {sizeof(dfs_add_powersave_filters_resp_msg_v01), dfs_add_powersave_filters_resp_msg_data_v01},
  {sizeof(dfs_set_powersave_filter_mode_req_msg_v01), dfs_set_powersave_filter_mode_req_msg_data_v01},
  {sizeof(dfs_set_powersave_filter_mode_resp_msg_v01), dfs_set_powersave_filter_mode_resp_msg_data_v01},
  {sizeof(dfs_get_powersave_filter_mode_req_msg_v01), 0},
  {sizeof(dfs_get_powersave_filter_mode_resp_msg_v01), dfs_get_powersave_filter_mode_resp_msg_data_v01},
  {sizeof(dfs_set_autoexit_powersave_filter_mode_req_msg_v01), dfs_set_autoexit_powersave_filter_mode_req_msg_data_v01},
  {sizeof(dfs_set_autoexit_powersave_filter_mode_resp_msg_v01), dfs_set_autoexit_powersave_filter_mode_resp_msg_data_v01},
  {sizeof(dfs_indication_register_req_msg_v01), dfs_indication_register_req_msg_data_v01},
  {sizeof(dfs_indication_register_resp_msg_v01), dfs_indication_register_resp_msg_data_v01},
  {sizeof(dfs_powersave_filter_mode_ind_msg_v01), dfs_powersave_filter_mode_ind_msg_data_v01},
  {sizeof(dfs_remove_all_powersave_filters_req_msg_v01), 0},
  {sizeof(dfs_remove_all_powersave_filters_resp_msg_v01), dfs_remove_all_powersave_filters_resp_msg_data_v01},
  {sizeof(dfs_add_low_latency_filters_req_msg_v01), dfs_add_low_latency_filters_req_msg_data_v01},
  {sizeof(dfs_add_low_latency_filters_resp_msg_v01), dfs_add_low_latency_filters_resp_msg_data_v01},
  {sizeof(dfs_low_latency_traffic_ind_msg_v01), dfs_low_latency_traffic_ind_msg_data_v01},
  {sizeof(dfs_get_reverse_ip_transport_filters_req_msg_v01), 0},
  {sizeof(dfs_get_reverse_ip_transport_filters_resp_msg_v01), dfs_get_reverse_ip_transport_filters_resp_msg_data_v01},
  {sizeof(dfs_reverse_ip_transport_filters_updated_ind_msg_v01), dfs_reverse_ip_transport_filters_updated_ind_msg_data_v01},
  {sizeof(dfs_remote_socket_request_ind_msg_v01), dfs_remote_socket_request_ind_msg_data_v01},
  {sizeof(dfs_remote_socket_allocated_req_msg_v01), dfs_remote_socket_allocated_req_msg_data_v01},
  {sizeof(dfs_remote_socket_allocated_resp_msg_v01), dfs_remote_socket_allocated_resp_msg_data_v01},
  {sizeof(dfs_remote_socket_release_ind_msg_v01), dfs_remote_socket_release_ind_msg_data_v01},
  {sizeof(dfs_remote_socket_set_option_ind_msg_v01), dfs_remote_socket_set_option_ind_msg_data_v01},
  {sizeof(dfs_get_capability_req_msg_v01), 0},
  {sizeof(dfs_get_capability_resp_msg_v01), dfs_get_capability_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object dfs_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *dfs_qmi_idl_type_table_object_referenced_tables_v01[] =
{&dfs_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01, &data_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object dfs_qmi_idl_type_table_object_v01 = {
  sizeof(dfs_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(dfs_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  dfs_type_table_v01,
  dfs_message_table_v01,
  dfs_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry dfs_service_command_messages_v01[] = {
  {QMI_DFS_INDICATION_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 24), 16},
  {QMI_DFS_GET_FILTER_CAPABILITY_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_DFS_BIND_CLIENT_REQ_V01, QMI_IDL_TYPE16(0, 2), 31},
  {QMI_DFS_GET_CLIENT_BINDING_REQ_V01, QMI_IDL_TYPE16(0, 4), 0},
  {QMI_DFS_ADD_MEDIA_OFFLOAD_FILTER_REQ_V01, QMI_IDL_TYPE16(0, 6), 43},
  {QMI_DFS_REMOVE_MEDIA_OFFLOAD_FILTER_REQ_V01, QMI_IDL_TYPE16(0, 8), 7},
  {QMI_DFS_GET_MEDIA_OFFLOAD_STATISTICS_REQ_V01, QMI_IDL_TYPE16(0, 10), 7},
  {QMI_DFS_ADD_PDN_SHARING_FILTERS_REQ_V01, QMI_IDL_TYPE16(0, 12), 36979},
  {QMI_DFS_REMOVE_FILTERS_REQ_V01, QMI_IDL_TYPE16(0, 14), 1024},
  {QMI_DFS_ADD_POWERSAVE_FILTERS_REQ_V01, QMI_IDL_TYPE16(0, 16), 36979},
  {QMI_DFS_SET_POWERSAVE_FILTER_MODE_REQ_V01, QMI_IDL_TYPE16(0, 18), 4},
  {QMI_DFS_GET_POWERSAVE_FILTER_MODE_REQ_V01, QMI_IDL_TYPE16(0, 20), 0},
  {QMI_DFS_SET_AUTOEXIT_POWERSAVE_FILTER_MODE_REQ_V01, QMI_IDL_TYPE16(0, 22), 4},
  {QMI_DFS_REMOVE_ALL_POWERSAVE_FILTERS_REQ_V01, QMI_IDL_TYPE16(0, 27), 0},
  {QMI_DFS_ADD_LOW_LATENCY_FILTERS_REQ_V01, QMI_IDL_TYPE16(0, 29), 38010},
  {QMI_DFS_GET_REVERSE_IP_TRANSPORT_FILTERS_REQ_V01, QMI_IDL_TYPE16(0, 32), 0},
  {QMI_DFS_REMOTE_SOCKET_ALLOCATED_REQ_V01, QMI_IDL_TYPE16(0, 36), 4084},
  {QMI_DFS_GET_CAPABILITY_REQ_V01, QMI_IDL_TYPE16(0, 40), 0}
};

static const qmi_idl_service_message_table_entry dfs_service_response_messages_v01[] = {
  {QMI_DFS_INDICATION_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 25), 7},
  {QMI_DFS_GET_FILTER_CAPABILITY_RESP_V01, QMI_IDL_TYPE16(0, 1), 47},
  {QMI_DFS_BIND_CLIENT_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_DFS_GET_CLIENT_BINDING_RESP_V01, QMI_IDL_TYPE16(0, 5), 38},
  {QMI_DFS_ADD_MEDIA_OFFLOAD_FILTER_RESP_V01, QMI_IDL_TYPE16(0, 7), 14},
  {QMI_DFS_REMOVE_MEDIA_OFFLOAD_FILTER_RESP_V01, QMI_IDL_TYPE16(0, 9), 7},
  {QMI_DFS_GET_MEDIA_OFFLOAD_STATISTICS_RESP_V01, QMI_IDL_TYPE16(0, 11), 43},
  {QMI_DFS_ADD_PDN_SHARING_FILTERS_RESP_V01, QMI_IDL_TYPE16(0, 13), 3075},
  {QMI_DFS_REMOVE_FILTERS_RESP_V01, QMI_IDL_TYPE16(0, 15), 7},
  {QMI_DFS_ADD_POWERSAVE_FILTERS_RESP_V01, QMI_IDL_TYPE16(0, 17), 3075},
  {QMI_DFS_SET_POWERSAVE_FILTER_MODE_RESP_V01, QMI_IDL_TYPE16(0, 19), 7},
  {QMI_DFS_GET_POWERSAVE_FILTER_MODE_RESP_V01, QMI_IDL_TYPE16(0, 21), 11},
  {QMI_DFS_SET_AUTOEXIT_POWERSAVE_FILTER_MODE_RESP_V01, QMI_IDL_TYPE16(0, 23), 7},
  {QMI_DFS_REMOVE_ALL_POWERSAVE_FILTERS_RESP_V01, QMI_IDL_TYPE16(0, 28), 7},
  {QMI_DFS_ADD_LOW_LATENCY_FILTERS_RESP_V01, QMI_IDL_TYPE16(0, 30), 3075},
  {QMI_DFS_GET_REVERSE_IP_TRANSPORT_FILTERS_RESP_V01, QMI_IDL_TYPE16(0, 33), 36986},
  {QMI_DFS_REMOTE_SOCKET_ALLOCATED_RESP_V01, QMI_IDL_TYPE16(0, 37), 7},
  {QMI_DFS_GET_CAPABILITY_RESP_V01, QMI_IDL_TYPE16(0, 41), 11}
};

static const qmi_idl_service_message_table_entry dfs_service_indication_messages_v01[] = {
  {QMI_DFS_POWERSAVE_FILTER_MODE_IND_V01, QMI_IDL_TYPE16(0, 26), 4},
  {QMI_DFS_LOW_LATENCY_TRAFFIC_IND_V01, QMI_IDL_TYPE16(0, 31), 11},
  {QMI_DFS_REVERSE_IP_TRANSPORT_FILTERS_UPDATED_IND_V01, QMI_IDL_TYPE16(0, 34), 36986},
  {QMI_DFS_REMOTE_SOCKET_REQUEST_IND_V01, QMI_IDL_TYPE16(0, 35), 1789},
  {QMI_DFS_REMOTE_SOCKET_RELEASE_IND_V01, QMI_IDL_TYPE16(0, 38), 1024},
  {QMI_DFS_REMOTE_SOCKET_SET_OPTION_IND_V01, QMI_IDL_TYPE16(0, 39), 11}
};

/*Service Object*/
struct qmi_idl_service_object dfs_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x30,
  38010,
  { sizeof(dfs_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(dfs_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(dfs_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { dfs_service_command_messages_v01, dfs_service_response_messages_v01, dfs_service_indication_messages_v01},
  &dfs_qmi_idl_type_table_object_v01,
  0x09,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type dfs_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( DFS_V01_IDL_MAJOR_VERS != idl_maj_version || DFS_V01_IDL_MINOR_VERS != idl_min_version
       || DFS_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&dfs_qmi_idl_service_object_v01;
}

