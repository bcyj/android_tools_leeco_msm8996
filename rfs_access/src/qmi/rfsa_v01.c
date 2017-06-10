/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        R E M O T E _ F I L E S Y S T E M _ A C C E S S _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the rfsa service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/rfsa/main/latest/src/remote_filesystem_access_v01.c#7 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2
   It was generated on: Wed Jul 24 2013 (Spin 4)
   From IDL File: remote_filesystem_access_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "rfsa_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t rfsa_file_content_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_file_content_v01, count),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(rfsa_file_content_v01, buffer),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t rfsa_iovec_desc_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_iovec_desc_type_v01, file_offset),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_iovec_desc_type_v01, buff_addr_offset),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_iovec_desc_type_v01, size),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t rfsa_file_stat_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(rfsa_file_stat_req_msg_v01, filename),
  RFSA_MAX_FILE_PATH_V01
};

static const uint8_t rfsa_file_stat_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfsa_file_stat_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x03,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(rfsa_file_stat_resp_msg_v01, flags),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rfsa_file_stat_resp_msg_v01, size) - QMI_IDL_OFFSET8(rfsa_file_stat_resp_msg_v01, size_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_file_stat_resp_msg_v01, size)
};

static const uint8_t rfsa_file_create_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(rfsa_file_create_req_msg_v01, filename),
  RFSA_MAX_FILE_PATH_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(rfsa_file_create_req_msg_v01, flags)
};

static const uint8_t rfsa_file_create_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfsa_file_create_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t rfsa_get_buff_addr_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_get_buff_addr_req_msg_v01, client_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_get_buff_addr_req_msg_v01, size)
};

static const uint8_t rfsa_get_buff_addr_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfsa_get_buff_addr_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rfsa_get_buff_addr_resp_msg_v01, address) - QMI_IDL_OFFSET8(rfsa_get_buff_addr_resp_msg_v01, address_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(rfsa_get_buff_addr_resp_msg_v01, address)
};

static const uint8_t rfsa_release_buff_addr_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_release_buff_addr_req_msg_v01, client_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(rfsa_release_buff_addr_req_msg_v01, address)
};

static const uint8_t rfsa_release_buff_addr_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfsa_release_buff_addr_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t rfsa_file_read_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_file_read_req_msg_v01, client_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(rfsa_file_read_req_msg_v01, filename),
  RFSA_MAX_FILE_PATH_V01,

  0x03,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(rfsa_file_read_req_msg_v01, offset),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(rfsa_file_read_req_msg_v01, size)
};

static const uint8_t rfsa_file_read_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfsa_file_read_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rfsa_file_read_resp_msg_v01, data) - QMI_IDL_OFFSET8(rfsa_file_read_resp_msg_v01, data_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfsa_file_read_resp_msg_v01, data),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t rfsa_iovec_file_read_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_iovec_file_read_req_msg_v01, client_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(rfsa_iovec_file_read_req_msg_v01, filename),
  RFSA_MAX_FILE_PATH_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(rfsa_iovec_file_read_req_msg_v01, iovec_struct),
  RFSA_MAX_IOVEC_ENTRIES_V01,
  QMI_IDL_OFFSET16RELATIVE(rfsa_iovec_file_read_req_msg_v01, iovec_struct) - QMI_IDL_OFFSET16RELATIVE(rfsa_iovec_file_read_req_msg_v01, iovec_struct_len),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t rfsa_iovec_file_read_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfsa_iovec_file_read_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t rfsa_iovec_file_write_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rfsa_iovec_file_write_req_msg_v01, client_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(rfsa_iovec_file_write_req_msg_v01, filename),
  RFSA_MAX_FILE_PATH_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(rfsa_iovec_file_write_req_msg_v01, iovec_struct),
  RFSA_MAX_IOVEC_ENTRIES_V01,
  QMI_IDL_OFFSET16RELATIVE(rfsa_iovec_file_write_req_msg_v01, iovec_struct) - QMI_IDL_OFFSET16RELATIVE(rfsa_iovec_file_write_req_msg_v01, iovec_struct_len),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t rfsa_iovec_file_write_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rfsa_iovec_file_write_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  rfsa_type_table_v01[] = {
  {sizeof(rfsa_file_content_v01), rfsa_file_content_data_v01},
  {sizeof(rfsa_iovec_desc_type_v01), rfsa_iovec_desc_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry rfsa_message_table_v01[] = {
  {sizeof(rfsa_file_stat_req_msg_v01), rfsa_file_stat_req_msg_data_v01},
  {sizeof(rfsa_file_stat_resp_msg_v01), rfsa_file_stat_resp_msg_data_v01},
  {sizeof(rfsa_file_create_req_msg_v01), rfsa_file_create_req_msg_data_v01},
  {sizeof(rfsa_file_create_resp_msg_v01), rfsa_file_create_resp_msg_data_v01},
  {sizeof(rfsa_get_buff_addr_req_msg_v01), rfsa_get_buff_addr_req_msg_data_v01},
  {sizeof(rfsa_get_buff_addr_resp_msg_v01), rfsa_get_buff_addr_resp_msg_data_v01},
  {sizeof(rfsa_release_buff_addr_req_msg_v01), rfsa_release_buff_addr_req_msg_data_v01},
  {sizeof(rfsa_release_buff_addr_resp_msg_v01), rfsa_release_buff_addr_resp_msg_data_v01},
  {sizeof(rfsa_file_read_req_msg_v01), rfsa_file_read_req_msg_data_v01},
  {sizeof(rfsa_file_read_resp_msg_v01), rfsa_file_read_resp_msg_data_v01},
  {sizeof(rfsa_iovec_file_read_req_msg_v01), rfsa_iovec_file_read_req_msg_data_v01},
  {sizeof(rfsa_iovec_file_read_resp_msg_v01), rfsa_iovec_file_read_resp_msg_data_v01},
  {sizeof(rfsa_iovec_file_write_req_msg_v01), rfsa_iovec_file_write_req_msg_data_v01},
  {sizeof(rfsa_iovec_file_write_resp_msg_v01), rfsa_iovec_file_write_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object rfsa_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *rfsa_qmi_idl_type_table_object_referenced_tables_v01[] =
{&rfsa_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object rfsa_qmi_idl_type_table_object_v01 = {
  sizeof(rfsa_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(rfsa_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  rfsa_type_table_v01,
  rfsa_message_table_v01,
  rfsa_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry rfsa_service_command_messages_v01[] = {
  {QMI_RFSA_FILE_STAT_REQ_MSG_V01, QMI_IDL_TYPE16(0, 0), 258},
  {QMI_RFSA_FILE_CREATE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 2), 269},
  {QMI_RFSA_FILE_READ_REQ_MSG_V01, QMI_IDL_TYPE16(0, 8), 279},
  {QMI_RFSA_GET_BUFF_ADDR_REQ_MSG_V01, QMI_IDL_TYPE16(0, 4), 14},
  {QMI_RFSA_RELEASE_BUFF_ADDR_REQ_MSG_V01, QMI_IDL_TYPE16(0, 6), 18},
  {QMI_RFSA_IOVEC_FILE_READ_REQ_MSG_V01, QMI_IDL_TYPE16(0, 10), 869},
  {QMI_RFSA_IOVEC_FILE_WRITE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 12), 869}
};

static const qmi_idl_service_message_table_entry rfsa_service_response_messages_v01[] = {
  {QMI_RFSA_FILE_STAT_RESP_MSG_V01, QMI_IDL_TYPE16(0, 1), 25},
  {QMI_RFSA_FILE_CREATE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_RFSA_FILE_READ_RESP_MSG_V01, QMI_IDL_TYPE16(0, 9), 22},
  {QMI_RFSA_GET_BUFF_ADDR_RESP_MSG_V01, QMI_IDL_TYPE16(0, 5), 18},
  {QMI_RFSA_RELEASE_BUFF_ADDR_RESP_MSG_V01, QMI_IDL_TYPE16(0, 7), 7},
  {QMI_RFSA_IOVEC_FILE_READ_RESP_MSG_V01, QMI_IDL_TYPE16(0, 11), 7},
  {QMI_RFSA_IOVEC_FILE_WRITE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 13), 7}
};

/*Service Object*/
struct qmi_idl_service_object rfsa_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x1C,
  869,
  { sizeof(rfsa_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(rfsa_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { rfsa_service_command_messages_v01, rfsa_service_response_messages_v01, NULL},
  &rfsa_qmi_idl_type_table_object_v01,
  0x01,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type rfsa_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( RFSA_V01_IDL_MAJOR_VERS != idl_maj_version || RFSA_V01_IDL_MINOR_VERS != idl_min_version
       || RFSA_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&rfsa_qmi_idl_service_object_v01;
}

