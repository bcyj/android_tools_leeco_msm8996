/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        C O M M O N _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the common service Data structures.

  Copyright (c) 2006-2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Thu Nov 14 2013 (Spin 0)
   From IDL File: common_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t qmi_response_type_data_v01[] = {
  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(qmi_response_type_v01, result),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(qmi_response_type_v01, error),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * qmi_get_supported_msgs_req is empty
 * static const uint8_t qmi_get_supported_msgs_req_data_v01[] = {
 * };
 */
  
static const uint8_t qmi_get_supported_msgs_resp_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_get_supported_msgs_resp_v01, resp),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_get_supported_msgs_resp_v01, supported_msgs) - QMI_IDL_OFFSET8(qmi_get_supported_msgs_resp_v01, supported_msgs_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_get_supported_msgs_resp_v01, supported_msgs),
  ((QMI_GET_SUPPORTED_MESSAGES_ARRAY_MAX_V01) & 0xFF), ((QMI_GET_SUPPORTED_MESSAGES_ARRAY_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_get_supported_msgs_resp_v01, supported_msgs) - QMI_IDL_OFFSET8(qmi_get_supported_msgs_resp_v01, supported_msgs_len)
};

static const uint8_t qmi_get_supported_fields_req_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_get_supported_fields_req_v01, msg_id)
};

static const uint8_t qmi_get_supported_fields_resp_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, resp),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, request_fields) - QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, request_fields_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, request_fields),
  QMI_GET_SUPPORTED_FIELDS_ARRAY_MAX_V01,
  QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, request_fields) - QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, request_fields_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, response_fields) - QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, response_fields_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, response_fields),
  QMI_GET_SUPPORTED_FIELDS_ARRAY_MAX_V01,
  QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, response_fields) - QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, response_fields_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, indication_fields) - QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, indication_fields_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, indication_fields),
  QMI_GET_SUPPORTED_FIELDS_ARRAY_MAX_V01,
  QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, indication_fields) - QMI_IDL_OFFSET8(qmi_get_supported_fields_resp_v01, indication_fields_len)
};

/* Type Table */
static const qmi_idl_type_table_entry  common_type_table_v01[] = {
  {sizeof(qmi_response_type_v01), qmi_response_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry common_message_table_v01[] = {
  {sizeof(qmi_get_supported_msgs_req_v01), 0},
  {sizeof(qmi_get_supported_msgs_resp_v01), qmi_get_supported_msgs_resp_data_v01},
  {sizeof(qmi_get_supported_fields_req_v01), qmi_get_supported_fields_req_data_v01},
  {sizeof(qmi_get_supported_fields_resp_v01), qmi_get_supported_fields_resp_data_v01}
};

/* Range Table */
/* Predefine the Type Table Object */
const qmi_idl_type_table_object common_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *common_qmi_idl_type_table_object_referenced_tables_v01[] =
{&common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
const qmi_idl_type_table_object common_qmi_idl_type_table_object_v01 = {
  sizeof(common_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(common_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  common_type_table_v01,
  common_message_table_v01,
  common_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

