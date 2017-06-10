/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ F I L E _ I N T E R N A L _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_FILE_INTERNAL_SVC service Data structures.

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
   From IDL File: sns_file_internal_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_file_internal_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sns_file_open_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_file_open_req_msg_v01, path_name),
  ((SNS_FILE_MAX_FILENAME_SIZE_V01) & 0xFF), ((SNS_FILE_MAX_FILENAME_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(sns_file_open_req_msg_v01, path_name) - QMI_IDL_OFFSET8(sns_file_open_req_msg_v01, path_name_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_file_open_req_msg_v01, mode),
  SNS_FILE_MAX_MODE_SIZE_V01,
  QMI_IDL_OFFSET16RELATIVE(sns_file_open_req_msg_v01, mode) - QMI_IDL_OFFSET16RELATIVE(sns_file_open_req_msg_v01, mode_len)
};

static const uint8_t sns_file_open_resp_msg_data_v01[] = {
  2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_file_open_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_file_open_resp_msg_v01, fildes) - QMI_IDL_OFFSET8(sns_file_open_resp_msg_v01, fildes_valid)),
  0x10,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_file_open_resp_msg_v01, fildes)
};

static const uint8_t sns_file_write_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_file_write_req_msg_v01, fildes),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_file_write_req_msg_v01, buf),
  ((SNS_FILE_MAX_BUF_SIZE_V01) & 0xFF), ((SNS_FILE_MAX_BUF_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(sns_file_write_req_msg_v01, buf) - QMI_IDL_OFFSET8(sns_file_write_req_msg_v01, buf_len)
};

static const uint8_t sns_file_write_resp_msg_data_v01[] = {
  2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_file_write_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_file_write_resp_msg_v01, bytes_written) - QMI_IDL_OFFSET8(sns_file_write_resp_msg_v01, bytes_written_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_file_write_resp_msg_v01, bytes_written)
};

static const uint8_t sns_file_close_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_file_close_req_msg_v01, fildes)
};

static const uint8_t sns_file_close_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_file_close_resp_msg_v01, resp),
  0, 1
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_FILE_INTERNAL_SVC_message_table_v01[] = {
  {sizeof(sns_file_open_req_msg_v01), sns_file_open_req_msg_data_v01},
  {sizeof(sns_file_open_resp_msg_v01), sns_file_open_resp_msg_data_v01},
  {sizeof(sns_file_write_req_msg_v01), sns_file_write_req_msg_data_v01},
  {sizeof(sns_file_write_resp_msg_v01), sns_file_write_resp_msg_data_v01},
  {sizeof(sns_file_close_req_msg_v01), sns_file_close_req_msg_data_v01},
  {sizeof(sns_file_close_resp_msg_v01), sns_file_close_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_FILE_INTERNAL_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_FILE_INTERNAL_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_FILE_INTERNAL_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_FILE_INTERNAL_SVC_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(SNS_FILE_INTERNAL_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_FILE_INTERNAL_SVC_message_table_v01,
  SNS_FILE_INTERNAL_SVC_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_FILE_INTERNAL_SVC_service_command_messages_v01[] = {
  {SNS_FILE_INTERNAL_CANCEL_REQ_V01, TYPE16(1, 0), 0},
  {SNS_FILE_INTERNAL_VERSION_REQ_V01, TYPE16(1, 2), 0},
  {SNS_FILE_INTERNAL_OPEN_REQ_V01, TYPE16(0, 0), 525},
  {SNS_FILE_INTERNAL_WRITE_REQ_V01, TYPE16(0, 2), 528},
  {SNS_FILE_INTERNAL_CLOSE_REQ_V01, TYPE16(0, 4), 11}
};

static const qmi_idl_service_message_table_entry SNS_FILE_INTERNAL_SVC_service_response_messages_v01[] = {
  {SNS_FILE_INTERNAL_CANCEL_RESP_V01, TYPE16(1, 1), 5},
  {SNS_FILE_INTERNAL_VERSION_RESP_V01, TYPE16(1, 3), 17},
  {SNS_FILE_INTERNAL_OPEN_RESP_V01, TYPE16(0, 1), 16},
  {SNS_FILE_INTERNAL_WRITE_RESP_V01, TYPE16(0, 3), 12},
  {SNS_FILE_INTERNAL_CLOSE_RESP_V01, TYPE16(0, 5), 5}
};

/*Service Object*/
const struct qmi_idl_service_object SNS_FILE_INTERNAL_SVC_qmi_idl_service_object_v01 = {
  0x04,
  0x01,
  SNS_QMI_SVC_ID_36_V01,
  528,
  { sizeof(SNS_FILE_INTERNAL_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_FILE_INTERNAL_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { SNS_FILE_INTERNAL_SVC_service_command_messages_v01, SNS_FILE_INTERNAL_SVC_service_response_messages_v01, NULL},
  &SNS_FILE_INTERNAL_SVC_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_FILE_INTERNAL_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_FILE_INTERNAL_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_FILE_INTERNAL_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_FILE_INTERNAL_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_FILE_INTERNAL_SVC_qmi_idl_service_object_v01;
}

