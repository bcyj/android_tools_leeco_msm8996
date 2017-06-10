/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        B E A R E R _ I N D E P E N D E N T _ T R A N S P O R T _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the bit service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/interfaces/qmi/bit/main/latest/src/bearer_independent_transport_v01.c#5 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5
   It was generated on: Thu Aug 23 2012
   From IDL File: bearer_independent_transport_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "bearer_independent_transport_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t bit_host_info_struct_type_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_host_info_struct_type_v01, validity_mask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_host_info_struct_type_v01, ipv4_addr),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(bit_host_info_struct_type_v01, ipv4_port),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(bit_host_info_struct_type_v01, ipv6_addr),
  16,

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(bit_host_info_struct_type_v01, ipv6_port),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(bit_host_info_struct_type_v01, url),
  BIT_CONST_URL_LEN_MAX_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(bit_host_info_struct_type_v01, url_port),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t bit_ack_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_ack_msg_v01, resp),
  0, 1
};

static const uint8_t bit_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_resp_msg_v01, transaction_id)
};

static const uint8_t bit_session_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_session_resp_msg_v01, resp),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_session_resp_msg_v01, transaction_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_session_resp_msg_v01, session_handle)
};

static const uint8_t bit_open_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_open_req_msg_v01, transaction_id)
};

static const uint8_t bit_open_status_ind_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_open_status_ind_msg_v01, status),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_open_status_ind_msg_v01, transaction_id)
};

static const uint8_t bit_close_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_close_req_msg_v01, transaction_id)
};

static const uint8_t bit_close_status_ind_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_close_status_ind_msg_v01, status),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_close_status_ind_msg_v01, transaction_id)
};

static const uint8_t bit_connect_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_connect_req_msg_v01, transaction_id),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_connect_req_msg_v01, link),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(bit_connect_req_msg_v01, protocol) - QMI_IDL_OFFSET8(bit_connect_req_msg_v01, protocol_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_connect_req_msg_v01, protocol),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(bit_connect_req_msg_v01, host_info) - QMI_IDL_OFFSET8(bit_connect_req_msg_v01, host_info_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_connect_req_msg_v01, host_info),
  0, 0
};

static const uint8_t bit_connect_status_ind_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_connect_status_ind_msg_v01, status),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_connect_status_ind_msg_v01, transaction_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(bit_connect_status_ind_msg_v01, session_handle) - QMI_IDL_OFFSET8(bit_connect_status_ind_msg_v01, session_handle_valid)),
  0x10,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_connect_status_ind_msg_v01, session_handle)
};

static const uint8_t bit_disconnect_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_disconnect_req_msg_v01, transaction_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_disconnect_req_msg_v01, session_handle)
};

static const uint8_t bit_disconnect_status_ind_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_disconnect_status_ind_msg_v01, status),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_disconnect_status_ind_msg_v01, transaction_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_disconnect_status_ind_msg_v01, session_handle)
};

static const uint8_t bit_send_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_send_req_msg_v01, transaction_id),

  0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_send_req_msg_v01, session_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(bit_send_req_msg_v01, payload),
  ((BIT_CONST_PAYLOAD_LEN_MAX_V01) & 0xFF), ((BIT_CONST_PAYLOAD_LEN_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(bit_send_req_msg_v01, payload) - QMI_IDL_OFFSET8(bit_send_req_msg_v01, payload_len)
};

static const uint8_t bit_send_status_ind_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_send_status_ind_msg_v01, status),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_send_status_ind_msg_v01, transaction_id),

  0x04,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_send_status_ind_msg_v01, session_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(bit_send_status_ind_msg_v01, bytes_sent) - QMI_IDL_OFFSET8(bit_send_status_ind_msg_v01, bytes_sent_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_send_status_ind_msg_v01, bytes_sent)
};

static const uint8_t bit_ready_to_receive_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_ready_to_receive_req_msg_v01, transaction_id),

  0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_ready_to_receive_req_msg_v01, session_handle),

  0x03,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(bit_ready_to_receive_req_msg_v01, rtr),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(bit_ready_to_receive_req_msg_v01, max_recv_payload_size) - QMI_IDL_OFFSET8(bit_ready_to_receive_req_msg_v01, max_recv_payload_size_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_ready_to_receive_req_msg_v01, max_recv_payload_size)
};

static const uint8_t bit_data_received_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_data_received_ind_msg_v01, session_handle),

  0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_data_received_ind_msg_v01, seq_num),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(bit_data_received_ind_msg_v01, payload),
  ((BIT_CONST_PAYLOAD_LEN_MAX_V01) & 0xFF), ((BIT_CONST_PAYLOAD_LEN_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(bit_data_received_ind_msg_v01, payload) - QMI_IDL_OFFSET8(bit_data_received_ind_msg_v01, payload_len)
};

static const uint8_t bit_data_received_status_req_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_data_received_status_req_msg_v01, resp),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_data_received_status_req_msg_v01, transaction_id),

  0x04,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_data_received_status_req_msg_v01, session_handle),

  0x05,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_data_received_status_req_msg_v01, seq_num),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(bit_data_received_status_req_msg_v01, max_recv_payload_size) - QMI_IDL_OFFSET8(bit_data_received_status_req_msg_v01, max_recv_payload_size_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_data_received_status_req_msg_v01, max_recv_payload_size)
};

static const uint8_t bit_set_dormancy_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_set_dormancy_req_msg_v01, transaction_id),

  0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_set_dormancy_req_msg_v01, session_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(bit_set_dormancy_req_msg_v01, dormancy_state)
};

static const uint8_t bit_set_dormancy_status_ind_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_set_dormancy_status_ind_msg_v01, status),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_set_dormancy_status_ind_msg_v01, transaction_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_set_dormancy_status_ind_msg_v01, session_handle)
};

static const uint8_t bit_get_local_host_info_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_get_local_host_info_req_msg_v01, transaction_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_get_local_host_info_req_msg_v01, session_handle)
};

static const uint8_t bit_get_local_host_info_status_ind_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_get_local_host_info_status_ind_msg_v01, status),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(bit_get_local_host_info_status_ind_msg_v01, transaction_id),

  0x04,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(bit_get_local_host_info_status_ind_msg_v01, session_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(bit_get_local_host_info_status_ind_msg_v01, local_host_info) - QMI_IDL_OFFSET8(bit_get_local_host_info_status_ind_msg_v01, local_host_info_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(bit_get_local_host_info_status_ind_msg_v01, local_host_info),
  0, 0
};

/*
 * bit_service_ready_ind_msg is empty
 * static const uint8_t bit_service_ready_ind_msg_data_v01[] = {
 * };
 */

/* Type Table */
static const qmi_idl_type_table_entry  bit_type_table_v01[] = {
  {sizeof(bit_host_info_struct_type_v01), bit_host_info_struct_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry bit_message_table_v01[] = {
  {sizeof(bit_ack_msg_v01), bit_ack_msg_data_v01},
  {sizeof(bit_resp_msg_v01), bit_resp_msg_data_v01},
  {sizeof(bit_session_resp_msg_v01), bit_session_resp_msg_data_v01},
  {sizeof(bit_open_req_msg_v01), bit_open_req_msg_data_v01},
  {sizeof(bit_open_status_ind_msg_v01), bit_open_status_ind_msg_data_v01},
  {sizeof(bit_close_req_msg_v01), bit_close_req_msg_data_v01},
  {sizeof(bit_close_status_ind_msg_v01), bit_close_status_ind_msg_data_v01},
  {sizeof(bit_connect_req_msg_v01), bit_connect_req_msg_data_v01},
  {sizeof(bit_connect_status_ind_msg_v01), bit_connect_status_ind_msg_data_v01},
  {sizeof(bit_disconnect_req_msg_v01), bit_disconnect_req_msg_data_v01},
  {sizeof(bit_disconnect_status_ind_msg_v01), bit_disconnect_status_ind_msg_data_v01},
  {sizeof(bit_send_req_msg_v01), bit_send_req_msg_data_v01},
  {sizeof(bit_send_status_ind_msg_v01), bit_send_status_ind_msg_data_v01},
  {sizeof(bit_ready_to_receive_req_msg_v01), bit_ready_to_receive_req_msg_data_v01},
  {sizeof(bit_data_received_ind_msg_v01), bit_data_received_ind_msg_data_v01},
  {sizeof(bit_data_received_status_req_msg_v01), bit_data_received_status_req_msg_data_v01},
  {sizeof(bit_set_dormancy_req_msg_v01), bit_set_dormancy_req_msg_data_v01},
  {sizeof(bit_set_dormancy_status_ind_msg_v01), bit_set_dormancy_status_ind_msg_data_v01},
  {sizeof(bit_get_local_host_info_req_msg_v01), bit_get_local_host_info_req_msg_data_v01},
  {sizeof(bit_get_local_host_info_status_ind_msg_v01), bit_get_local_host_info_status_ind_msg_data_v01},
  {0, 0}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object bit_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *bit_qmi_idl_type_table_object_referenced_tables_v01[] =
{&bit_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object bit_qmi_idl_type_table_object_v01 = {
  sizeof(bit_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(bit_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  bit_type_table_v01,
  bit_message_table_v01,
  bit_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry bit_service_command_messages_v01[] = {
  {QMI_BIT_OPEN_RESP_V01, TYPE16(0, 1), 14},
  {QMI_BIT_OPEN_STATUS_IND_V01, TYPE16(0, 4), 14},
  {QMI_BIT_CLOSE_RESP_V01, TYPE16(0, 1), 14},
  {QMI_BIT_CLOSE_STATUS_IND_V01, TYPE16(0, 6), 14},
  {QMI_BIT_CONNECT_RESP_V01, TYPE16(0, 1), 14},
  {QMI_BIT_CONNECT_STATUS_IND_V01, TYPE16(0, 8), 25},
  {QMI_BIT_DISCONNECT_RESP_V01, TYPE16(0, 2), 25},
  {QMI_BIT_DISCONNECT_STATUS_IND_V01, TYPE16(0, 10), 25},
  {QMI_BIT_SEND_RESP_V01, TYPE16(0, 2), 25},
  {QMI_BIT_SEND_STATUS_IND_V01, TYPE16(0, 12), 32},
  {QMI_BIT_READY_TO_RECEIVE_RESP_V01, TYPE16(0, 2), 25},
  {QMI_BIT_DATA_RECEIVED_IND_V01, TYPE16(0, 14), 2075},
  {QMI_BIT_DATA_RECEIVED_STATUS_RESP_V01, TYPE16(0, 2), 25},
  {QMI_BIT_SET_DORMANCY_RESP_V01, TYPE16(0, 2), 25},
  {QMI_BIT_SET_DORMANCY_STATUS_IND_V01, TYPE16(0, 17), 25},
  {QMI_BIT_GET_LOCAL_HOST_INFO_RESP_V01, TYPE16(0, 2), 25},
  {QMI_BIT_GET_LOCAL_HOST_INFO_STATUS_IND_V01, TYPE16(0, 19), 317},
  {QMI_BIT_SERVICE_READY_IND_V01, TYPE16(0, 20), 0}
};

static const qmi_idl_service_message_table_entry bit_service_response_messages_v01[] = {
  {QMI_BIT_OPEN_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_OPEN_STATUS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_CLOSE_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_CLOSE_STATUS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_CONNECT_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_CONNECT_STATUS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_DISCONNECT_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_DISCONNECT_STATUS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_SEND_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_SEND_STATUS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_READY_TO_RECEIVE_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_DATA_RECEIVED_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_DATA_RECEIVED_STATUS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_SET_DORMANCY_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_SET_DORMANCY_STATUS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_GET_LOCAL_HOST_INFO_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_GET_LOCAL_HOST_INFO_STATUS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_BIT_SERVICE_READY_ACK_V01, TYPE16(0, 0), 7}
};

static const qmi_idl_service_message_table_entry bit_service_indication_messages_v01[] = {
  {QMI_BIT_OPEN_REQ_V01, TYPE16(0, 3), 7},
  {QMI_BIT_CLOSE_REQ_V01, TYPE16(0, 5), 7},
  {QMI_BIT_CONNECT_REQ_V01, TYPE16(0, 7), 313},
  {QMI_BIT_DISCONNECT_REQ_V01, TYPE16(0, 9), 18},
  {QMI_BIT_SEND_REQ_V01, TYPE16(0, 11), 2071},
  {QMI_BIT_READY_TO_RECEIVE_REQ_V01, TYPE16(0, 13), 29},
  {QMI_BIT_DATA_RECEIVED_STATUS_REQ_V01, TYPE16(0, 15), 43},
  {QMI_BIT_SET_DORMANCY_REQ_V01, TYPE16(0, 16), 22},
  {QMI_BIT_GET_LOCAL_HOST_INFO_REQ_V01, TYPE16(0, 18), 18}
};

/*Service Object*/
struct qmi_idl_service_object bit_qmi_idl_service_object_v01 = {
  0x05,
  0x01,
  0x27,
  2075,
  { sizeof(bit_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(bit_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(bit_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { bit_service_command_messages_v01, bit_service_response_messages_v01, bit_service_indication_messages_v01},
  &bit_qmi_idl_type_table_object_v01,
  0x01,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type bit_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( BIT_V01_IDL_MAJOR_VERS != idl_maj_version || BIT_V01_IDL_MINOR_VERS != idl_min_version
       || BIT_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&bit_qmi_idl_service_object_v01;
}

