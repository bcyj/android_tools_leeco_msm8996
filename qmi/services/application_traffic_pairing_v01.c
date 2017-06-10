/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        A P P L I C A T I O N _ T R A F F I C _ P A I R I N G _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the atp service Data structures.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.6
   It was generated on: Fri Jan 10 2014 (Spin 0)
   From IDL File: application_traffic_pairing_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "application_traffic_pairing_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t atp_policy_info_entry_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(atp_policy_info_entry_type_v01, apn),
  QMI_ATP_MAX_APN_NAME_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_policy_info_entry_type_v01, service_id),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(atp_policy_info_entry_type_v01, package_name),
  ((QMI_ATP_MAX_PACKAGE_NAME_LEN_V01) & 0xFF), ((QMI_ATP_MAX_PACKAGE_NAME_LEN_V01) >> 8),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_policy_info_entry_type_v01, framework_uid),
  QMI_ATP_MAX_UID_V01,
  QMI_IDL_OFFSET16RELATIVE(atp_policy_info_entry_type_v01, framework_uid) - QMI_IDL_OFFSET16RELATIVE(atp_policy_info_entry_type_v01, framework_uid_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_policy_info_entry_type_v01, hash_values),
  QMI_ATP_MAX_HASH_VALUE_V01,
  QMI_IDL_OFFSET16RELATIVE(atp_policy_info_entry_type_v01, hash_values) - QMI_IDL_OFFSET16RELATIVE(atp_policy_info_entry_type_v01, hash_values_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_policy_info_entry_type_v01, max_ack_timeout),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t atp_policy_list_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(atp_policy_list_type_v01, list_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(atp_policy_list_type_v01, total_list_entries),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(atp_policy_list_type_v01, policy_info),
  QMI_ATP_MAX_ENTRIES_PER_IND_V01,
  QMI_IDL_OFFSET8(atp_policy_list_type_v01, policy_info) - QMI_IDL_OFFSET8(atp_policy_list_type_v01, policy_info_len),
  QMI_IDL_TYPE88(0, 0),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t atp_filter_report_src_ipv6_addr_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_filter_report_src_ipv6_addr_type_v01, src_ipv6_addr),
  QMI_ATP_IPV6_ADDR_LEN_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_filter_report_src_ipv6_addr_type_v01, src_ipv6_prefix_length),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t atp_filter_report_dest_ipv6_addr_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_filter_report_dest_ipv6_addr_type_v01, dest_ipv6_addr),
  QMI_ATP_IPV6_ADDR_LEN_V01,

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_filter_report_dest_ipv6_addr_type_v01, dest_ipv6_prefix_length),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t atp_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_indication_register_req_msg_v01, report_activation_status) - QMI_IDL_OFFSET8(atp_indication_register_req_msg_v01, report_activation_status_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_indication_register_req_msg_v01, report_activation_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_indication_register_req_msg_v01, report_policy_info) - QMI_IDL_OFFSET8(atp_indication_register_req_msg_v01, report_policy_info_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_indication_register_req_msg_v01, report_policy_info)
};

static const uint8_t atp_indication_register_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(atp_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_indication_register_resp_msg_v01, activation_status) - QMI_IDL_OFFSET8(atp_indication_register_resp_msg_v01, activation_status_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_indication_register_resp_msg_v01, activation_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_indication_register_resp_msg_v01, policy_info) - QMI_IDL_OFFSET8(atp_indication_register_resp_msg_v01, policy_info_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_indication_register_resp_msg_v01, policy_info)
};

/*
 * atp_activation_status_query_req_msg is empty
 * static const uint8_t atp_activation_status_query_req_msg_data_v01[] = {
 * };
 */

static const uint8_t atp_activation_status_query_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(atp_activation_status_query_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_activation_status_query_resp_msg_v01, is_activated) - QMI_IDL_OFFSET8(atp_activation_status_query_resp_msg_v01, is_activated_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_activation_status_query_resp_msg_v01, is_activated)
};

static const uint8_t atp_activation_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(atp_activation_status_ind_msg_v01, is_activated)
};

/*
 * atp_policy_info_req_msg is empty
 * static const uint8_t atp_policy_info_req_msg_data_v01[] = {
 * };
 */

static const uint8_t atp_policy_info_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(atp_policy_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t atp_policy_info_change_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_policy_info_change_ind_msg_v01, atp_policy_list) - QMI_IDL_OFFSET8(atp_policy_info_change_ind_msg_v01, atp_policy_list_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(atp_policy_info_change_ind_msg_v01, atp_policy_list),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t atp_send_filter_report_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, report_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, action) - QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, action_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, action),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, service_id) - QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, service_id_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, service_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, package_name) - QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, package_name_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(atp_send_filter_report_req_msg_v01, package_name),
  ((QMI_ATP_MAX_PACKAGE_NAME_LEN_V01) & 0xFF), ((QMI_ATP_MAX_PACKAGE_NAME_LEN_V01) >> 8),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, hash_values) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, hash_values_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, hash_values),
  QMI_ATP_MAX_HASH_VALUE_V01,
  QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, hash_values) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, hash_values_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, ip_type) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, ip_type_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, ip_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, dest_ipv4_addr) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, dest_ipv4_addr_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, dest_ipv4_addr),
  QMI_ATP_IPV4_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, dest_ipv6_addr) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, dest_ipv6_addr_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, dest_ipv6_addr),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, dest_port) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, dest_port_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, dest_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, src_ipv4_addr) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, src_ipv4_addr_valid)),
  0x18,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, src_ipv4_addr),
  QMI_ATP_IPV4_ADDR_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, src_ipv6_addr) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, src_ipv6_addr_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, src_ipv6_addr),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, src_port) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, src_port_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, src_port),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, protocol) - QMI_IDL_OFFSET16RELATIVE(atp_send_filter_report_req_msg_v01, protocol_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(atp_send_filter_report_req_msg_v01, protocol)
};

static const uint8_t atp_send_filter_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(atp_send_filter_report_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t atp_send_filter_report_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(atp_send_filter_report_ind_msg_v01, report_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(atp_send_filter_report_ind_msg_v01, filter_report_resp) - QMI_IDL_OFFSET8(atp_send_filter_report_ind_msg_v01, filter_report_resp_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(atp_send_filter_report_ind_msg_v01, filter_report_resp)
};

/* Type Table */
static const qmi_idl_type_table_entry  atp_type_table_v01[] = {
  {sizeof(atp_policy_info_entry_type_v01), atp_policy_info_entry_type_data_v01},
  {sizeof(atp_policy_list_type_v01), atp_policy_list_type_data_v01},
  {sizeof(atp_filter_report_src_ipv6_addr_type_v01), atp_filter_report_src_ipv6_addr_type_data_v01},
  {sizeof(atp_filter_report_dest_ipv6_addr_type_v01), atp_filter_report_dest_ipv6_addr_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry atp_message_table_v01[] = {
  {sizeof(atp_indication_register_req_msg_v01), atp_indication_register_req_msg_data_v01},
  {sizeof(atp_indication_register_resp_msg_v01), atp_indication_register_resp_msg_data_v01},
  {sizeof(atp_activation_status_query_req_msg_v01), 0},
  {sizeof(atp_activation_status_query_resp_msg_v01), atp_activation_status_query_resp_msg_data_v01},
  {sizeof(atp_activation_status_ind_msg_v01), atp_activation_status_ind_msg_data_v01},
  {sizeof(atp_policy_info_req_msg_v01), 0},
  {sizeof(atp_policy_info_resp_msg_v01), atp_policy_info_resp_msg_data_v01},
  {sizeof(atp_policy_info_change_ind_msg_v01), atp_policy_info_change_ind_msg_data_v01},
  {sizeof(atp_send_filter_report_req_msg_v01), atp_send_filter_report_req_msg_data_v01},
  {sizeof(atp_send_filter_report_resp_msg_v01), atp_send_filter_report_resp_msg_data_v01},
  {sizeof(atp_send_filter_report_ind_msg_v01), atp_send_filter_report_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object atp_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *atp_qmi_idl_type_table_object_referenced_tables_v01[] =
{&atp_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object atp_qmi_idl_type_table_object_v01 = {
  sizeof(atp_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(atp_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  atp_type_table_v01,
  atp_message_table_v01,
  atp_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry atp_service_command_messages_v01[] = {
  {QMI_ATP_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_ATP_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_ATP_INDICATION_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 0), 8},
  {QMI_ATP_ACTIVATION_STATUS_QUERY_REQ_V01, QMI_IDL_TYPE16(0, 2), 0},
  {QMI_ATP_POLICY_INFO_QUERY_REQ_V01, QMI_IDL_TYPE16(0, 5), 0},
  {QMI_ATP_SEND_FILTER_REPORT_REQ_V01, QMI_IDL_TYPE16(0, 8), 618}
};

static const qmi_idl_service_message_table_entry atp_service_response_messages_v01[] = {
  {QMI_ATP_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_ATP_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_ATP_INDICATION_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 1), 15},
  {QMI_ATP_ACTIVATION_STATUS_QUERY_RESP_V01, QMI_IDL_TYPE16(0, 3), 11},
  {QMI_ATP_POLICY_INFO_QUERY_RESP_V01, QMI_IDL_TYPE16(0, 6), 7},
  {QMI_ATP_SEND_FILTER_REPORT_RESP_V01, QMI_IDL_TYPE16(0, 9), 7}
};

static const qmi_idl_service_message_table_entry atp_service_indication_messages_v01[] = {
  {QMI_ATP_ACTIVATION_STATUS_IND_V01, QMI_IDL_TYPE16(0, 4), 4},
  {QMI_ATP_POLICY_INFO_CHANGE_IND_V01, QMI_IDL_TYPE16(0, 7), 51812},
  {QMI_ATP_SEND_FILTER_REPORT_IND_V01, QMI_IDL_TYPE16(0, 10), 14}
};

/*Service Object*/
struct qmi_idl_service_object atp_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x2E,
  51812,
  { sizeof(atp_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(atp_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(atp_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { atp_service_command_messages_v01, atp_service_response_messages_v01, atp_service_indication_messages_v01},
  &atp_qmi_idl_type_table_object_v01,
  0x00,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type atp_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( ATP_V01_IDL_MAJOR_VERS != idl_maj_version || ATP_V01_IDL_MINOR_VERS != idl_min_version
       || ATP_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&atp_qmi_idl_service_object_v01;
}

