/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q M I _ I M S _ V T _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the ims_qmi service Data structures.

  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.7
   It was generated on: Fri Sep 23 2011
   From IDL File: qmi_ims_vt_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "qmi_ims_vt_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t ims_vt_call_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_call_info_type_v01, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(ims_vt_call_info_type_v01, call_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(ims_vt_call_info_type_v01, call_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(ims_vt_call_info_type_v01, direction),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t ims_vt_remote_party_number_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_remote_party_number_type_v01, number),
  QMI_IMS_VT_NUMBER_MAX_V01,
  QMI_IDL_OFFSET8(ims_vt_remote_party_number_type_v01, number) - QMI_IDL_OFFSET8(ims_vt_remote_party_number_type_v01, number_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t ims_vt_remote_party_name_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_remote_party_name_type_v01, caller_name),
  QMI_IMS_VT_CALLER_NAME_MAX_V01,
  QMI_IDL_OFFSET8(ims_vt_remote_party_name_type_v01, caller_name) - QMI_IDL_OFFSET8(ims_vt_remote_party_name_type_v01, caller_name_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t ims_vt_num_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_num_info_type_v01, num),
  QMI_IMS_VT_CALLER_ID_MAX_V01,
  QMI_IDL_OFFSET8(ims_vt_num_info_type_v01, num) - QMI_IDL_OFFSET8(ims_vt_num_info_type_v01, num_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t ims_vt_call_end_reason_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_call_end_reason_type_v01, call_id),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(ims_vt_call_end_reason_type_v01, call_end_reason),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t ims_vt_dial_call_req_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_vt_dial_call_req_v01, calling_number),
  QMI_IMS_VT_NUMBER_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_dial_call_req_v01, call_type) - QMI_IDL_OFFSET8(ims_vt_dial_call_req_v01, call_type_valid)),
  0x10,
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(ims_vt_dial_call_req_v01, call_type)
};

static const uint8_t ims_vt_dial_call_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_dial_call_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_dial_call_resp_v01, call_id) - QMI_IDL_OFFSET8(ims_vt_dial_call_resp_v01, call_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_dial_call_resp_v01, call_id)
};

static const uint8_t ims_vt_end_call_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_end_call_req_v01, call_id)
};

static const uint8_t ims_vt_end_call_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_end_call_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_end_call_resp_v01, call_id) - QMI_IDL_OFFSET8(ims_vt_end_call_resp_v01, call_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_end_call_resp_v01, call_id)
};

static const uint8_t ims_vt_answer_call_req_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_answer_call_req_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(ims_vt_answer_call_req_v01, answer)
};

static const uint8_t ims_vt_answer_call_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_answer_call_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_answer_call_resp_v01, call_id) - QMI_IDL_OFFSET8(ims_vt_answer_call_resp_v01, call_id_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_answer_call_resp_v01, call_id)
};

static const uint8_t ims_vt_get_call_info_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_vt_get_call_info_req_v01, call_id)
};

static const uint8_t ims_vt_get_call_info_resp_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, call_info) - QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, call_info_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, call_info),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, remote_party_number) - QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, remote_party_number_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, remote_party_number),
  1, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, remote_party_name) - QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, remote_party_name_valid)),
  0x12,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_get_call_info_resp_v01, remote_party_name),
  2, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ims_vt_get_call_info_resp_v01, conn_num_info) - QMI_IDL_OFFSET16RELATIVE(ims_vt_get_call_info_resp_v01, conn_num_info_valid)),
  0x13,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ims_vt_get_call_info_resp_v01, conn_num_info),
  3, 0
};

static const uint8_t ims_vt_call_status_ind_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_call_status_ind_v01, call_info),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_call_status_ind_v01, remote_party_number) - QMI_IDL_OFFSET8(ims_vt_call_status_ind_v01, remote_party_number_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_call_status_ind_v01, remote_party_number),
  1, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_vt_call_status_ind_v01, remote_party_name) - QMI_IDL_OFFSET8(ims_vt_call_status_ind_v01, remote_party_name_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_vt_call_status_ind_v01, remote_party_name),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ims_vt_call_status_ind_v01, conn_num_info) - QMI_IDL_OFFSET16RELATIVE(ims_vt_call_status_ind_v01, conn_num_info_valid)),
  0x12,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ims_vt_call_status_ind_v01, conn_num_info),
  3, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(ims_vt_call_status_ind_v01, call_end_reason) - QMI_IDL_OFFSET16RELATIVE(ims_vt_call_status_ind_v01, call_end_reason_valid)),
  0x13,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(ims_vt_call_status_ind_v01, call_end_reason),
  4, 0
};

/* Type Table */
static const qmi_idl_type_table_entry  ims_qmi_type_table_v01[] = {
  {sizeof(ims_vt_call_info_type_v01), ims_vt_call_info_type_data_v01},
  {sizeof(ims_vt_remote_party_number_type_v01), ims_vt_remote_party_number_type_data_v01},
  {sizeof(ims_vt_remote_party_name_type_v01), ims_vt_remote_party_name_type_data_v01},
  {sizeof(ims_vt_num_info_type_v01), ims_vt_num_info_type_data_v01},
  {sizeof(ims_vt_call_end_reason_type_v01), ims_vt_call_end_reason_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry ims_qmi_message_table_v01[] = {
  {sizeof(ims_vt_dial_call_req_v01), ims_vt_dial_call_req_data_v01},
  {sizeof(ims_vt_dial_call_resp_v01), ims_vt_dial_call_resp_data_v01},
  {sizeof(ims_vt_end_call_req_v01), ims_vt_end_call_req_data_v01},
  {sizeof(ims_vt_end_call_resp_v01), ims_vt_end_call_resp_data_v01},
  {sizeof(ims_vt_answer_call_req_v01), ims_vt_answer_call_req_data_v01},
  {sizeof(ims_vt_answer_call_resp_v01), ims_vt_answer_call_resp_data_v01},
  {sizeof(ims_vt_get_call_info_req_v01), ims_vt_get_call_info_req_data_v01},
  {sizeof(ims_vt_get_call_info_resp_v01), ims_vt_get_call_info_resp_data_v01},
  {sizeof(ims_vt_call_status_ind_v01), ims_vt_call_status_ind_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object ims_qmi_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *ims_qmi_qmi_idl_type_table_object_referenced_tables_v01[] =
{&ims_qmi_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object ims_qmi_qmi_idl_type_table_object_v01 = {
  sizeof(ims_qmi_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(ims_qmi_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  ims_qmi_type_table_v01,
  ims_qmi_message_table_v01,
  ims_qmi_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry ims_qmi_service_command_messages_v01[] = {
  {IMS_VT_DIAL_CALL_REQ_V01, TYPE16(0, 0), 88},
  {IMS_VT_END_CALL_REQ_V01, TYPE16(0, 2), 4},
  {IMS_VT_ANSWER_CALL_REQ_V01, TYPE16(0, 4), 8},
  {IMS_VT_GET_CALL_INFO_REQ_V01, TYPE16(0, 6), 4}
};

static const qmi_idl_service_message_table_entry ims_qmi_service_response_messages_v01[] = {
  {IMS_VT_DIAL_CALL_RESP_V01, TYPE16(0, 1), 11},
  {IMS_VT_END_CALL_RESP_V01, TYPE16(0, 3), 11},
  {IMS_VT_ANSWER_CALL_RESP_V01, TYPE16(0, 5), 11},
  {IMS_VT_GET_CALL_INFO_RESP_V01, TYPE16(0, 7), 370}
};

static const qmi_idl_service_message_table_entry ims_qmi_service_indication_messages_v01[] = {
  {IMS_VT_CALL_STATUS_IND_V01, TYPE16(0, 8), 369}
};

/*Service Object*/
const struct qmi_idl_service_object ims_qmi_qmi_idl_service_object_v01 = {
  0x02,
  0x01,
  0x0013,
  370,
  { sizeof(ims_qmi_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ims_qmi_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ims_qmi_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { ims_qmi_service_command_messages_v01, ims_qmi_service_response_messages_v01, ims_qmi_service_indication_messages_v01},
  &ims_qmi_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type ims_qmi_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( IMS_QMI_V01_IDL_MAJOR_VERS != idl_maj_version || IMS_QMI_V01_IDL_MINOR_VERS != idl_min_version 
       || IMS_QMI_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&ims_qmi_qmi_idl_service_object_v01;
}

