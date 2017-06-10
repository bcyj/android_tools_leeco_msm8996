/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        R E M O T E _ S T O R A G E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the rmtfs service Data structures.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header: //source/qcom/qct/interfaces/qmi/rmtfs/main/latest/src/remote_storage_v01.c#5 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2
   It was generated on: Wed Jul 24 2013 (Spin 2)
   From IDL File: remote_storage_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "remote_storage_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t rmtfs_iovec_desc_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_iovec_desc_type_v01, sector_addr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_iovec_desc_type_v01, data_phy_addr_offset),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_iovec_desc_type_v01, num_sector),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t rmtfs_open_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(rmtfs_open_req_msg_v01, path),
  RMTFS_MAX_FILE_PATH_V01
};

static const uint8_t rmtfs_open_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rmtfs_open_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rmtfs_open_resp_msg_v01, caller_id) - QMI_IDL_OFFSET8(rmtfs_open_resp_msg_v01, caller_id_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_open_resp_msg_v01, caller_id)
};

static const uint8_t rmtfs_close_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_close_req_msg_v01, caller_id)
};

static const uint8_t rmtfs_close_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rmtfs_close_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t rmtfs_rw_iovec_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_rw_iovec_req_msg_v01, caller_id),

  0x02,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(rmtfs_rw_iovec_req_msg_v01, direction),

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rmtfs_rw_iovec_req_msg_v01, iovec_struct),
  RMTFS_MAX_IOVEC_ENTRIES_V01,
  QMI_IDL_OFFSET8(rmtfs_rw_iovec_req_msg_v01, iovec_struct) - QMI_IDL_OFFSET8(rmtfs_rw_iovec_req_msg_v01, iovec_struct_len),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(rmtfs_rw_iovec_req_msg_v01, is_force_sync)
};

static const uint8_t rmtfs_rw_iovec_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rmtfs_rw_iovec_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t rmtfs_alloc_buff_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_alloc_buff_req_msg_v01, caller_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_alloc_buff_req_msg_v01, buff_size)
};

static const uint8_t rmtfs_alloc_buff_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rmtfs_alloc_buff_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rmtfs_alloc_buff_resp_msg_v01, buff_address) - QMI_IDL_OFFSET8(rmtfs_alloc_buff_resp_msg_v01, buff_address_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(rmtfs_alloc_buff_resp_msg_v01, buff_address)
};

static const uint8_t rmtfs_get_dev_error_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_get_dev_error_req_msg_v01, caller_id)
};

static const uint8_t rmtfs_get_dev_error_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(rmtfs_get_dev_error_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(rmtfs_get_dev_error_resp_msg_v01, status) - QMI_IDL_OFFSET8(rmtfs_get_dev_error_resp_msg_v01, status_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(rmtfs_get_dev_error_resp_msg_v01, status)
};

static const uint8_t rmtfs_force_sync_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rmtfs_force_sync_ind_msg_v01, caller_id),
  RMTFS_MAX_CALLER_ID_V01,
  QMI_IDL_OFFSET8(rmtfs_force_sync_ind_msg_v01, caller_id) - QMI_IDL_OFFSET8(rmtfs_force_sync_ind_msg_v01, caller_id_len)
};

/* Type Table */
static const qmi_idl_type_table_entry  rmtfs_type_table_v01[] = {
  {sizeof(rmtfs_iovec_desc_type_v01), rmtfs_iovec_desc_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry rmtfs_message_table_v01[] = {
  {sizeof(rmtfs_open_req_msg_v01), rmtfs_open_req_msg_data_v01},
  {sizeof(rmtfs_open_resp_msg_v01), rmtfs_open_resp_msg_data_v01},
  {sizeof(rmtfs_close_req_msg_v01), rmtfs_close_req_msg_data_v01},
  {sizeof(rmtfs_close_resp_msg_v01), rmtfs_close_resp_msg_data_v01},
  {sizeof(rmtfs_rw_iovec_req_msg_v01), rmtfs_rw_iovec_req_msg_data_v01},
  {sizeof(rmtfs_rw_iovec_resp_msg_v01), rmtfs_rw_iovec_resp_msg_data_v01},
  {sizeof(rmtfs_alloc_buff_req_msg_v01), rmtfs_alloc_buff_req_msg_data_v01},
  {sizeof(rmtfs_alloc_buff_resp_msg_v01), rmtfs_alloc_buff_resp_msg_data_v01},
  {sizeof(rmtfs_get_dev_error_req_msg_v01), rmtfs_get_dev_error_req_msg_data_v01},
  {sizeof(rmtfs_get_dev_error_resp_msg_v01), rmtfs_get_dev_error_resp_msg_data_v01},
  {sizeof(rmtfs_force_sync_ind_msg_v01), rmtfs_force_sync_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object rmtfs_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *rmtfs_qmi_idl_type_table_object_referenced_tables_v01[] =
{&rmtfs_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object rmtfs_qmi_idl_type_table_object_v01 = {
  sizeof(rmtfs_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(rmtfs_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  rmtfs_type_table_v01,
  rmtfs_message_table_v01,
  rmtfs_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry rmtfs_service_command_messages_v01[] = {
  {QMI_RMTFS_OPEN_REQ_V01, QMI_IDL_TYPE16(0, 0), 258},
  {QMI_RMTFS_CLOSE_REQ_V01, QMI_IDL_TYPE16(0, 2), 7},
  {QMI_RMTFS_RW_IOVEC_REQ_V01, QMI_IDL_TYPE16(0, 4), 3079},
  {QMI_RMTFS_ALLOC_BUFF_REQ_V01, QMI_IDL_TYPE16(0, 6), 14},
  {QMI_RMTFS_GET_DEV_ERROR_REQ_V01, QMI_IDL_TYPE16(0, 8), 7}
};

static const qmi_idl_service_message_table_entry rmtfs_service_response_messages_v01[] = {
  {QMI_RMTFS_OPEN_RESP_V01, QMI_IDL_TYPE16(0, 1), 14},
  {QMI_RMTFS_CLOSE_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_RMTFS_RW_IOVEC_RESP_V01, QMI_IDL_TYPE16(0, 5), 7},
  {QMI_RMTFS_ALLOC_BUFF_RESP_V01, QMI_IDL_TYPE16(0, 7), 18},
  {QMI_RMTFS_GET_DEV_ERROR_RESP_V01, QMI_IDL_TYPE16(0, 9), 11}
};

static const qmi_idl_service_message_table_entry rmtfs_service_indication_messages_v01[] = {
  {QMI_RMTFS_FORCE_SYNC_IND_V01, QMI_IDL_TYPE16(0, 10), 44}
};

/*Service Object*/
struct qmi_idl_service_object rmtfs_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x0E,
  3079,
  { sizeof(rmtfs_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(rmtfs_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(rmtfs_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { rmtfs_service_command_messages_v01, rmtfs_service_response_messages_v01, rmtfs_service_indication_messages_v01},
  &rmtfs_qmi_idl_type_table_object_v01,
  0x02,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type rmtfs_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( RMTFS_V01_IDL_MAJOR_VERS != idl_maj_version || RMTFS_V01_IDL_MINOR_VERS != idl_min_version
       || RMTFS_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&rmtfs_qmi_idl_service_object_v01;
}

