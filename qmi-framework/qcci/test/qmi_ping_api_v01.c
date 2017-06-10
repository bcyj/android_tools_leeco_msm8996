/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q M I _ P I N G _ A P I _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the ping service Data structures.

  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.5
   It was generated on: Fri May 13 2011
   From IDL File: qmi_ping_api_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "qmi_ping_api_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t ping_name_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ping_name_type_v01, name),
  ((PING_MAX_NAME_SIZE_V01) & 0xFF), ((PING_MAX_NAME_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(ping_name_type_v01, name) - QMI_IDL_OFFSET8(ping_name_type_v01, name_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t ping_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ping_req_msg_v01, ping),
  4,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_req_msg_v01, client_name) - QMI_IDL_OFFSET8(ping_req_msg_v01, client_name_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_req_msg_v01, client_name),
  0, 0
};

static const uint8_t ping_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ping_resp_msg_v01, pong),
  4,

  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_resp_msg_v01, service_name) - QMI_IDL_OFFSET8(ping_resp_msg_v01, service_name_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_resp_msg_v01, service_name),
  0, 0
};

static const uint8_t ping_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ping_ind_msg_v01, indication),
  5,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_ind_msg_v01, service_name) - QMI_IDL_OFFSET8(ping_ind_msg_v01, service_name_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_ind_msg_v01, service_name),
  0, 0
};

static const uint8_t ping_data_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ping_data_req_msg_v01, data),
  ((PING_MAX_DATA_SIZE_V01) & 0xFF), ((PING_MAX_DATA_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(ping_data_req_msg_v01, data) - QMI_IDL_OFFSET8(ping_data_req_msg_v01, data_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ping_data_req_msg_v01, client_name) - QMI_IDL_OFFSET16RELATIVE(ping_data_req_msg_v01, client_name_valid)),
  0x10,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ping_data_req_msg_v01, client_name),
  0, 0
};

static const uint8_t ping_data_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ping_data_resp_msg_v01, data),
  ((PING_MAX_DATA_SIZE_V01) & 0xFF), ((PING_MAX_DATA_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(ping_data_resp_msg_v01, data) - QMI_IDL_OFFSET8(ping_data_resp_msg_v01, data_len),

  0x02,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ping_data_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ping_data_resp_msg_v01, service_name) - QMI_IDL_OFFSET16RELATIVE(ping_data_resp_msg_v01, service_name_valid)),
  0x10,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ping_data_resp_msg_v01, service_name),
  0, 0
};

static const uint8_t ping_data_ind_reg_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, num_inds) - QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, num_inds_valid)),
  0x10,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, num_inds),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, ind_size) - QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, ind_size_valid)),
  0x11,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, ind_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, ind_delay) - QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, ind_delay_valid)),
  0x12,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, ind_delay),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, num_reqs) - QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, num_reqs_valid)),
  0x13,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, num_reqs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, client_name) - QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, client_name_valid)),
  0x14,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_data_ind_reg_req_msg_v01, client_name),
  0, 0
};

static const uint8_t ping_data_ind_reg_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_data_ind_reg_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_data_ind_reg_resp_msg_v01, service_name) - QMI_IDL_OFFSET8(ping_data_ind_reg_resp_msg_v01, service_name_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_data_ind_reg_resp_msg_v01, service_name),
  0, 0
};

static const uint8_t ping_data_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ping_data_ind_msg_v01, data),
  ((PING_MAX_DATA_SIZE_V01) & 0xFF), ((PING_MAX_DATA_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(ping_data_ind_msg_v01, data) - QMI_IDL_OFFSET8(ping_data_ind_msg_v01, data_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ping_data_ind_msg_v01, service_name) - QMI_IDL_OFFSET16RELATIVE(ping_data_ind_msg_v01, service_name_valid)),
  0x10,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ping_data_ind_msg_v01, service_name),
  0, 0
};

static const uint8_t ping_get_service_name_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ping_get_service_name_req_msg_v01, client_name) - QMI_IDL_OFFSET8(ping_get_service_name_req_msg_v01, client_name_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_get_service_name_req_msg_v01, client_name),
  0, 0
};

static const uint8_t ping_get_service_name_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_get_service_name_resp_msg_v01, service_name),
  0, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ping_get_service_name_resp_msg_v01, resp),
  0, 1
};

/*
 * ping_null_req_msg is empty
 * static const uint8_t ping_null_req_msg_data_v01[] = {
 * };
 */

static const uint8_t ping_null_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ping_null_resp_msg_v01, service_name),
  0, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ping_null_resp_msg_v01, resp),
  0, 1
};

/*
 * ping_null_ind_msg is empty
 * static const uint8_t ping_null_ind_msg_data_v01[] = {
 * };
 */

/* Type Table */
static const qmi_idl_type_table_entry  ping_type_table_v01[] = {
  {sizeof(ping_name_type_v01), ping_name_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry ping_message_table_v01[] = {
  {sizeof(ping_req_msg_v01), ping_req_msg_data_v01},
  {sizeof(ping_resp_msg_v01), ping_resp_msg_data_v01},
  {sizeof(ping_ind_msg_v01), ping_ind_msg_data_v01},
  {sizeof(ping_data_req_msg_v01), ping_data_req_msg_data_v01},
  {sizeof(ping_data_resp_msg_v01), ping_data_resp_msg_data_v01},
  {sizeof(ping_data_ind_reg_req_msg_v01), ping_data_ind_reg_req_msg_data_v01},
  {sizeof(ping_data_ind_reg_resp_msg_v01), ping_data_ind_reg_resp_msg_data_v01},
  {sizeof(ping_data_ind_msg_v01), ping_data_ind_msg_data_v01},
  {sizeof(ping_get_service_name_req_msg_v01), ping_get_service_name_req_msg_data_v01},
  {sizeof(ping_get_service_name_resp_msg_v01), ping_get_service_name_resp_msg_data_v01},
  {0, 0},
  {sizeof(ping_null_resp_msg_v01), ping_null_resp_msg_data_v01},
  {0, 0}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object ping_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *ping_qmi_idl_type_table_object_referenced_tables_v01[] =
{&ping_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object ping_qmi_idl_type_table_object_v01 = {
  sizeof(ping_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(ping_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  ping_type_table_v01,
  ping_message_table_v01,
  ping_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry ping_service_command_messages_v01[] = {
  {QMI_PING_REQ_V01, TYPE16(0, 0), 266},
  {QMI_PING_DATA_REQ_V01, TYPE16(0, 3), 8456},
  {QMI_PING_DATA_IND_REG_REQ_V01, TYPE16(0, 5), 279},
  {QMI_PING_GET_SERVICE_NAME_REQ_V01, TYPE16(0, 8), 259},
  {QMI_PING_NULL_REQ_V01, TYPE16(0, 10), 0}
};

static const qmi_idl_service_message_table_entry ping_service_response_messages_v01[] = {
  {QMI_PING_RESP_V01, TYPE16(0, 1), 273},
  {QMI_PING_DATA_RESP_V01, TYPE16(0, 4), 8463},
  {QMI_PING_DATA_IND_REG_RESP_V01, TYPE16(0, 6), 266},
  {QMI_PING_GET_SERVICE_NAME_RESP_V01, TYPE16(0, 9), 266},
  {QMI_PING_NULL_RESP_V01, TYPE16(0, 11), 266}
};

static const qmi_idl_service_message_table_entry ping_service_indication_messages_v01[] = {
  {QMI_PING_IND_V01, TYPE16(0, 2), 267},
  {QMI_PING_DATA_IND_V01, TYPE16(0, 7), 8456},
  {QMI_PING_NULL_IND_V01, TYPE16(0, 12), 0}
};

/*Service Object*/
const struct qmi_idl_service_object ping_qmi_idl_service_object_v01 = {
  0x02,
  0x01,
  0x0F,
  8463,
  { sizeof(ping_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ping_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ping_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { ping_service_command_messages_v01, ping_service_response_messages_v01, ping_service_indication_messages_v01},
  &ping_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type ping_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( PING_V01_IDL_MAJOR_VERS != idl_maj_version || PING_V01_IDL_MINOR_VERS != idl_min_version
       || PING_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&ping_qmi_idl_service_object_v01;
}

