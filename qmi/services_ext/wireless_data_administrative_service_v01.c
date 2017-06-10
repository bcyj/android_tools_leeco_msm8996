/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        W I R E L E S S _ D A T A _ A D M I N I S T R A T I V E _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the wda service Data structures.

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.1
   It was generated on: Thu Oct 31 2013 (Spin 0)
   From IDL File: wireless_data_administrative_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "wireless_data_administrative_service_v01.h"
#include "common_v01.h"
#include "data_common_v01.h"


/*Type Definitions*/
static const uint8_t wda_packet_filter_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_type_v01, pattern),
  QMI_WDA_PACKET_FILTER_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(wda_packet_filter_type_v01, pattern) - QMI_IDL_OFFSET8(wda_packet_filter_type_v01, pattern_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_type_v01, mask),
  QMI_WDA_PACKET_FILTER_SIZE_MAX_V01,
  QMI_IDL_OFFSET8(wda_packet_filter_type_v01, mask) - QMI_IDL_OFFSET8(wda_packet_filter_type_v01, mask_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t wda_set_data_format_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, qos_format) - QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, qos_format_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, qos_format),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, link_prot) - QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, link_prot_valid)),
  0x11,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, link_prot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ul_data_aggregation_protocol) - QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ul_data_aggregation_protocol_valid)),
  0x12,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ul_data_aggregation_protocol),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_protocol) - QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_protocol_valid)),
  0x13,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_protocol),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ndp_signature) - QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ndp_signature_valid)),
  0x14,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ndp_signature),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_max_datagrams) - QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_max_datagrams_valid)),
  0x15,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_max_datagrams),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_max_size) - QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_max_size_valid)),
  0x16,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, dl_data_aggregation_max_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ep_id) - QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ep_id_valid)),
  0x17,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_set_data_format_req_msg_v01, ep_id),
  QMI_IDL_TYPE88(2, 0)
};

static const uint8_t wda_set_data_format_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, qos_format) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, qos_format_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, qos_format),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, link_prot) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, link_prot_valid)),
  0x11,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, link_prot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_protocol) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_protocol_valid)),
  0x12,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_protocol),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_protocol) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_protocol_valid)),
  0x13,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_protocol),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ndp_signature) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ndp_signature_valid)),
  0x14,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ndp_signature),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_max_datagrams) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_max_datagrams_valid)),
  0x15,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_max_datagrams),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_max_size) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_max_size_valid)),
  0x16,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, dl_data_aggregation_max_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_max_datagrams) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_max_datagrams_valid)),
  0x17,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_max_datagrams),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_max_size) - QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_max_size_valid)),
  0x18,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_set_data_format_resp_msg_v01, ul_data_aggregation_max_size)
};

static const uint8_t wda_get_data_format_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_req_msg_v01, ep_id) - QMI_IDL_OFFSET8(wda_get_data_format_req_msg_v01, ep_id_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_get_data_format_req_msg_v01, ep_id),
  QMI_IDL_TYPE88(2, 0)
};

static const uint8_t wda_get_data_format_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, qos_format) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, qos_format_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, qos_format),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, link_prot) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, link_prot_valid)),
  0x11,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, link_prot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_protocol) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_protocol_valid)),
  0x12,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_protocol),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_protocol) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_protocol_valid)),
  0x13,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_protocol),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ndp_signature) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ndp_signature_valid)),
  0x14,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ndp_signature),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_max_datagrams) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_max_datagrams_valid)),
  0x15,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_max_datagrams),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_max_size) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_max_size_valid)),
  0x16,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, dl_data_aggregation_max_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_max_datagrams) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_max_datagrams_valid)),
  0x17,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_max_datagrams),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_max_size) - QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_max_size_valid)),
  0x18,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_get_data_format_resp_msg_v01, ul_data_aggregation_max_size)
};

static const uint8_t wda_packet_filter_enable_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_enable_req_msg_v01, filter_is_restrictive),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_enable_req_msg_v01, ips_id) - QMI_IDL_OFFSET8(wda_packet_filter_enable_req_msg_v01, ips_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_enable_req_msg_v01, ips_id)
};

static const uint8_t wda_packet_filter_enable_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_enable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wda_packet_filter_disable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_disable_req_msg_v01, ips_id) - QMI_IDL_OFFSET8(wda_packet_filter_disable_req_msg_v01, ips_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_disable_req_msg_v01, ips_id)
};

static const uint8_t wda_packet_filter_disable_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_disable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wda_packet_filter_get_state_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_get_state_req_msg_v01, ips_id) - QMI_IDL_OFFSET8(wda_packet_filter_get_state_req_msg_v01, ips_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_state_req_msg_v01, ips_id)
};

static const uint8_t wda_packet_filter_get_state_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_get_state_resp_msg_v01, filtering_is_enabled) - QMI_IDL_OFFSET8(wda_packet_filter_get_state_resp_msg_v01, filtering_is_enabled_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_state_resp_msg_v01, filtering_is_enabled),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_get_state_resp_msg_v01, filter_is_restrictive) - QMI_IDL_OFFSET8(wda_packet_filter_get_state_resp_msg_v01, filter_is_restrictive_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_state_resp_msg_v01, filter_is_restrictive)
};

static const uint8_t wda_packet_filter_add_rule_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_add_rule_req_msg_v01, rule),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wda_packet_filter_add_rule_req_msg_v01, ips_id) - QMI_IDL_OFFSET16RELATIVE(wda_packet_filter_add_rule_req_msg_v01, ips_id_valid)),
  0x10,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wda_packet_filter_add_rule_req_msg_v01, ips_id)
};

static const uint8_t wda_packet_filter_add_rule_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_add_rule_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_add_rule_resp_msg_v01, handle) - QMI_IDL_OFFSET8(wda_packet_filter_add_rule_resp_msg_v01, handle_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_add_rule_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_add_rule_resp_msg_v01, rule) - QMI_IDL_OFFSET8(wda_packet_filter_add_rule_resp_msg_v01, rule_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_add_rule_resp_msg_v01, rule),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t wda_packet_filter_delete_rule_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_req_msg_v01, handle) - QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_req_msg_v01, handle_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_req_msg_v01, ips_id) - QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_req_msg_v01, ips_id_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_req_msg_v01, ips_id)
};

static const uint8_t wda_packet_filter_delete_rule_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_resp_msg_v01, handle) - QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_resp_msg_v01, handle_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_delete_rule_resp_msg_v01, handle)
};

static const uint8_t wda_packet_filter_get_rule_handles_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_req_msg_v01, ips_id) - QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_req_msg_v01, ips_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_req_msg_v01, ips_id)
};

static const uint8_t wda_packet_filter_get_rule_handles_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_resp_msg_v01, handle) - QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_resp_msg_v01, handle_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_resp_msg_v01, handle),
  QMI_WDA_PACKET_FILTER_NUM_MAX_V01,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_resp_msg_v01, handle) - QMI_IDL_OFFSET8(wda_packet_filter_get_rule_handles_resp_msg_v01, handle_len)
};

static const uint8_t wda_packet_filter_get_rule_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_get_rule_req_msg_v01, ips_id) - QMI_IDL_OFFSET8(wda_packet_filter_get_rule_req_msg_v01, ips_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_req_msg_v01, ips_id)
};

static const uint8_t wda_packet_filter_get_rule_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_get_rule_resp_msg_v01, handle) - QMI_IDL_OFFSET8(wda_packet_filter_get_rule_resp_msg_v01, handle_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_packet_filter_get_rule_resp_msg_v01, rule) - QMI_IDL_OFFSET8(wda_packet_filter_get_rule_resp_msg_v01, rule_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_packet_filter_get_rule_resp_msg_v01, rule),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t wda_set_loopback_state_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_set_loopback_state_req_msg_v01, loopback_state)
};

static const uint8_t wda_set_loopback_state_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_set_loopback_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * wda_get_loopback_state_req_msg is empty
 * static const uint8_t wda_get_loopback_state_req_msg_data_v01[] = {
 * };
 */

static const uint8_t wda_get_loopback_state_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_get_loopback_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_loopback_state_resp_msg_v01, loopback_state_is_enabled) - QMI_IDL_OFFSET8(wda_get_loopback_state_resp_msg_v01, loopback_state_is_enabled_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_get_loopback_state_resp_msg_v01, loopback_state_is_enabled)
};

static const uint8_t wda_set_qmap_settings_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_qmap_settings_req_msg_v01, in_band_flow_control) - QMI_IDL_OFFSET8(wda_set_qmap_settings_req_msg_v01, in_band_flow_control_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_set_qmap_settings_req_msg_v01, in_band_flow_control),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_qmap_settings_req_msg_v01, ep_id) - QMI_IDL_OFFSET8(wda_set_qmap_settings_req_msg_v01, ep_id_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_set_qmap_settings_req_msg_v01, ep_id),
  QMI_IDL_TYPE88(2, 0)
};

static const uint8_t wda_set_qmap_settings_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_set_qmap_settings_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_set_qmap_settings_resp_msg_v01, in_band_flow_control) - QMI_IDL_OFFSET8(wda_set_qmap_settings_resp_msg_v01, in_band_flow_control_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_set_qmap_settings_resp_msg_v01, in_band_flow_control)
};

static const uint8_t wda_get_qmap_settings_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_qmap_settings_req_msg_v01, ep_id) - QMI_IDL_OFFSET8(wda_get_qmap_settings_req_msg_v01, ep_id_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_get_qmap_settings_req_msg_v01, ep_id),
  QMI_IDL_TYPE88(2, 0)
};

static const uint8_t wda_get_qmap_settings_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wda_get_qmap_settings_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wda_get_qmap_settings_resp_msg_v01, in_band_flow_control) - QMI_IDL_OFFSET8(wda_get_qmap_settings_resp_msg_v01, in_band_flow_control_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wda_get_qmap_settings_resp_msg_v01, in_band_flow_control)
};

/* Type Table */
static const qmi_idl_type_table_entry  wda_type_table_v01[] = {
  {sizeof(wda_packet_filter_type_v01), wda_packet_filter_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry wda_message_table_v01[] = {
  {sizeof(wda_set_data_format_req_msg_v01), wda_set_data_format_req_msg_data_v01},
  {sizeof(wda_set_data_format_resp_msg_v01), wda_set_data_format_resp_msg_data_v01},
  {sizeof(wda_get_data_format_req_msg_v01), wda_get_data_format_req_msg_data_v01},
  {sizeof(wda_get_data_format_resp_msg_v01), wda_get_data_format_resp_msg_data_v01},
  {sizeof(wda_packet_filter_enable_req_msg_v01), wda_packet_filter_enable_req_msg_data_v01},
  {sizeof(wda_packet_filter_enable_resp_msg_v01), wda_packet_filter_enable_resp_msg_data_v01},
  {sizeof(wda_packet_filter_disable_req_msg_v01), wda_packet_filter_disable_req_msg_data_v01},
  {sizeof(wda_packet_filter_disable_resp_msg_v01), wda_packet_filter_disable_resp_msg_data_v01},
  {sizeof(wda_packet_filter_get_state_req_msg_v01), wda_packet_filter_get_state_req_msg_data_v01},
  {sizeof(wda_packet_filter_get_state_resp_msg_v01), wda_packet_filter_get_state_resp_msg_data_v01},
  {sizeof(wda_packet_filter_add_rule_req_msg_v01), wda_packet_filter_add_rule_req_msg_data_v01},
  {sizeof(wda_packet_filter_add_rule_resp_msg_v01), wda_packet_filter_add_rule_resp_msg_data_v01},
  {sizeof(wda_packet_filter_delete_rule_req_msg_v01), wda_packet_filter_delete_rule_req_msg_data_v01},
  {sizeof(wda_packet_filter_delete_rule_resp_msg_v01), wda_packet_filter_delete_rule_resp_msg_data_v01},
  {sizeof(wda_packet_filter_get_rule_handles_req_msg_v01), wda_packet_filter_get_rule_handles_req_msg_data_v01},
  {sizeof(wda_packet_filter_get_rule_handles_resp_msg_v01), wda_packet_filter_get_rule_handles_resp_msg_data_v01},
  {sizeof(wda_packet_filter_get_rule_req_msg_v01), wda_packet_filter_get_rule_req_msg_data_v01},
  {sizeof(wda_packet_filter_get_rule_resp_msg_v01), wda_packet_filter_get_rule_resp_msg_data_v01},
  {sizeof(wda_set_loopback_state_req_msg_v01), wda_set_loopback_state_req_msg_data_v01},
  {sizeof(wda_set_loopback_state_resp_msg_v01), wda_set_loopback_state_resp_msg_data_v01},
  {sizeof(wda_get_loopback_state_req_msg_v01), 0},
  {sizeof(wda_get_loopback_state_resp_msg_v01), wda_get_loopback_state_resp_msg_data_v01},
  {sizeof(wda_set_qmap_settings_req_msg_v01), wda_set_qmap_settings_req_msg_data_v01},
  {sizeof(wda_set_qmap_settings_resp_msg_v01), wda_set_qmap_settings_resp_msg_data_v01},
  {sizeof(wda_get_qmap_settings_req_msg_v01), wda_get_qmap_settings_req_msg_data_v01},
  {sizeof(wda_get_qmap_settings_resp_msg_v01), wda_get_qmap_settings_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object wda_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *wda_qmi_idl_type_table_object_referenced_tables_v01[] =
{&wda_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01, &data_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object wda_qmi_idl_type_table_object_v01 = {
  sizeof(wda_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(wda_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  wda_type_table_v01,
  wda_message_table_v01,
  wda_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry wda_service_command_messages_v01[] = {
  {QMI_WDA_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_WDA_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_WDA_SET_DATA_FORMAT_REQ_V01, QMI_IDL_TYPE16(0, 0), 57},
  {QMI_WDA_GET_DATA_FORMAT_REQ_V01, QMI_IDL_TYPE16(0, 2), 11},
  {QMI_WDA_PACKET_FILTER_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 4), 8},
  {QMI_WDA_PACKET_FILTER_DISABLE_REQ_V01, QMI_IDL_TYPE16(0, 6), 4},
  {QMI_WDA_PACKET_FILTER_GET_STATE_REQ_V01, QMI_IDL_TYPE16(0, 8), 4},
  {QMI_WDA_PACKET_FILTER_ADD_RULE_REQ_V01, QMI_IDL_TYPE16(0, 10), 393},
  {QMI_WDA_PACKET_FILTER_DELETE_RULE_REQ_V01, QMI_IDL_TYPE16(0, 12), 11},
  {QMI_WDA_PACKET_FILTER_GET_RULE_HANDLES_REQ_V01, QMI_IDL_TYPE16(0, 14), 4},
  {QMI_WDA_PACKET_FILTER_GET_RULE_REQ_V01, QMI_IDL_TYPE16(0, 16), 11},
  {QMI_WDA_SET_LOOPBACK_STATE_REQ_V01, QMI_IDL_TYPE16(0, 18), 4},
  {QMI_WDA_GET_LOOPBACK_STATE_REQ_V01, QMI_IDL_TYPE16(0, 20), 0},
  {QMI_WDA_SET_QMAP_SETTINGS_REQ_V01, QMI_IDL_TYPE16(0, 22), 15},
  {QMI_WDA_GET_QMAP_SETTINGS_REQ_V01, QMI_IDL_TYPE16(0, 24), 11}
};

static const qmi_idl_service_message_table_entry wda_service_response_messages_v01[] = {
  {QMI_WDA_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_WDA_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_WDA_SET_DATA_FORMAT_RESP_V01, QMI_IDL_TYPE16(0, 1), 67},
  {QMI_WDA_GET_DATA_FORMAT_RESP_V01, QMI_IDL_TYPE16(0, 3), 67},
  {QMI_WDA_PACKET_FILTER_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 5), 7},
  {QMI_WDA_PACKET_FILTER_DISABLE_RESP_V01, QMI_IDL_TYPE16(0, 7), 7},
  {QMI_WDA_PACKET_FILTER_GET_STATE_RESP_V01, QMI_IDL_TYPE16(0, 9), 15},
  {QMI_WDA_PACKET_FILTER_ADD_RULE_RESP_V01, QMI_IDL_TYPE16(0, 11), 403},
  {QMI_WDA_PACKET_FILTER_DELETE_RULE_RESP_V01, QMI_IDL_TYPE16(0, 13), 14},
  {QMI_WDA_PACKET_FILTER_GET_RULE_HANDLES_RESP_V01, QMI_IDL_TYPE16(0, 15), 139},
  {QMI_WDA_PACKET_FILTER_GET_RULE_RESP_V01, QMI_IDL_TYPE16(0, 17), 403},
  {QMI_WDA_SET_LOOPBACK_STATE_RESP_V01, QMI_IDL_TYPE16(0, 19), 7},
  {QMI_WDA_GET_LOOPBACK_STATE_RESP_V01, QMI_IDL_TYPE16(0, 21), 11},
  {QMI_WDA_SET_QMAP_SETTINGS_RESP_V01, QMI_IDL_TYPE16(0, 23), 11},
  {QMI_WDA_GET_QMAP_SETTINGS_RESP_V01, QMI_IDL_TYPE16(0, 25), 11}
};

/*Service Object*/
struct qmi_idl_service_object wda_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x1A,
  8204,
  { sizeof(wda_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(wda_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { wda_service_command_messages_v01, wda_service_response_messages_v01, NULL},
  &wda_qmi_idl_type_table_object_v01,
  0x0D,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type wda_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( WDA_V01_IDL_MAJOR_VERS != idl_maj_version || WDA_V01_IDL_MINOR_VERS != idl_min_version
       || WDA_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&wda_qmi_idl_service_object_v01;
}

