/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ R E G _ A P I _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_REG_SVC service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5 
   It requires encode/decode library version 4 or later
   It was generated on: Fri Sep  7 2012
   From IDL File: sns_reg_api_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_reg_api_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sns_reg_single_read_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_reg_single_read_req_msg_v01, item_id)
};

static const uint8_t sns_reg_single_read_resp_msg_data_v01[] = {
  2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_reg_single_read_resp_msg_v01, resp),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_reg_single_read_resp_msg_v01, item_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_reg_single_read_resp_msg_v01, data),
  SNS_REG_MAX_ITEM_BYTE_COUNT_V01,
  QMI_IDL_OFFSET8(sns_reg_single_read_resp_msg_v01, data) - QMI_IDL_OFFSET8(sns_reg_single_read_resp_msg_v01, data_len)
};

static const uint8_t sns_reg_single_write_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_reg_single_write_req_msg_v01, item_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_reg_single_write_req_msg_v01, data),
  SNS_REG_MAX_ITEM_BYTE_COUNT_V01,
  QMI_IDL_OFFSET8(sns_reg_single_write_req_msg_v01, data) - QMI_IDL_OFFSET8(sns_reg_single_write_req_msg_v01, data_len)
};

static const uint8_t sns_reg_single_write_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_reg_single_write_resp_msg_v01, resp),
  0, 1
};

static const uint8_t sns_reg_group_read_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_reg_group_read_req_msg_v01, group_id)
};

static const uint8_t sns_reg_group_read_resp_msg_data_v01[] = {
  2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_reg_group_read_resp_msg_v01, resp),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_reg_group_read_resp_msg_v01, group_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_reg_group_read_resp_msg_v01, data),
  ((SNS_REG_MAX_GROUP_BYTE_COUNT_V01) & 0xFF), ((SNS_REG_MAX_GROUP_BYTE_COUNT_V01) >> 8),
  QMI_IDL_OFFSET8(sns_reg_group_read_resp_msg_v01, data) - QMI_IDL_OFFSET8(sns_reg_group_read_resp_msg_v01, data_len)
};

static const uint8_t sns_reg_group_write_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_reg_group_write_req_msg_v01, group_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_reg_group_write_req_msg_v01, data),
  ((SNS_REG_MAX_GROUP_BYTE_COUNT_V01) & 0xFF), ((SNS_REG_MAX_GROUP_BYTE_COUNT_V01) >> 8),
  QMI_IDL_OFFSET8(sns_reg_group_write_req_msg_v01, data) - QMI_IDL_OFFSET8(sns_reg_group_write_req_msg_v01, data_len)
};

static const uint8_t sns_reg_group_write_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_reg_group_write_resp_msg_v01, resp),
  0, 1
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_REG_SVC_message_table_v01[] = {
  {sizeof(sns_reg_single_read_req_msg_v01), sns_reg_single_read_req_msg_data_v01},
  {sizeof(sns_reg_single_read_resp_msg_v01), sns_reg_single_read_resp_msg_data_v01},
  {sizeof(sns_reg_single_write_req_msg_v01), sns_reg_single_write_req_msg_data_v01},
  {sizeof(sns_reg_single_write_resp_msg_v01), sns_reg_single_write_resp_msg_data_v01},
  {sizeof(sns_reg_group_read_req_msg_v01), sns_reg_group_read_req_msg_data_v01},
  {sizeof(sns_reg_group_read_resp_msg_v01), sns_reg_group_read_resp_msg_data_v01},
  {sizeof(sns_reg_group_write_req_msg_v01), sns_reg_group_write_req_msg_data_v01},
  {sizeof(sns_reg_group_write_resp_msg_v01), sns_reg_group_write_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_REG_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_REG_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_REG_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_REG_SVC_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(SNS_REG_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_REG_SVC_message_table_v01,
  SNS_REG_SVC_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_REG_SVC_service_command_messages_v01[] = {
  {SNS_REG_CANCEL_REQ_V01, TYPE16(1, 0), 0},
  {SNS_REG_VERSION_REQ_V01, TYPE16(1, 2), 0},
  {SNS_REG_SINGLE_READ_REQ_V01, TYPE16(0, 0), 5},
  {SNS_REG_SINGLE_WRITE_REQ_V01, TYPE16(0, 2), 17},
  {SNS_REG_GROUP_READ_REQ_V01, TYPE16(0, 4), 5},
  {SNS_REG_GROUP_WRITE_REQ_V01, TYPE16(0, 6), 266}
};

static const qmi_idl_service_message_table_entry SNS_REG_SVC_service_response_messages_v01[] = {
  {SNS_REG_CANCEL_RESP_V01, TYPE16(1, 1), 5},
  {SNS_REG_VERSION_RESP_V01, TYPE16(1, 3), 17},
  {SNS_REG_SINGLE_READ_RESP_V01, TYPE16(0, 1), 22},
  {SNS_REG_SINGLE_WRITE_RESP_V01, TYPE16(0, 3), 5},
  {SNS_REG_GROUP_READ_RESP_V01, TYPE16(0, 5), 271},
  {SNS_REG_GROUP_WRITE_RESP_V01, TYPE16(0, 7), 5}
};

/*Service Object*/
const struct qmi_idl_service_object SNS_REG_SVC_qmi_idl_service_object_v01 = {
  0x04,
  0x01,
  SNS_QMI_SVC_ID_3_V01,
  271,
  { sizeof(SNS_REG_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_REG_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { SNS_REG_SVC_service_command_messages_v01, SNS_REG_SVC_service_response_messages_v01, NULL},
  &SNS_REG_SVC_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_REG_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_REG_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_REG_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_REG_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_REG_SVC_qmi_idl_service_object_v01;
}

