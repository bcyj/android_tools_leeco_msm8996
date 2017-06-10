/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S M G R _ I N T E R N A L _ A P I _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_SMGR_INTERNAL_SVC service Data structures.

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It requires encode/decode library version 4 or later
   It was generated on: Fri May 24 2013 (Spin 0)
   From IDL File: sns_smgr_internal_api_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_smgr_internal_api_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sns_smgr_reg_hw_md_int_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_req_msg_v01, ReportId),

  0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_req_msg_v01, Action),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_req_msg_v01, SrcModule) - QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_req_msg_v01, SrcModule_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_req_msg_v01, SrcModule)
};

static const uint8_t sns_smgr_reg_hw_md_int_resp_msg_data_v01[] = {
  2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_resp_msg_v01, Resp),
  0, 1,

  0x03,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_resp_msg_v01, ReportId),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_resp_msg_v01, result)
};

static const uint8_t sns_smgr_reg_hw_md_int_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_ind_msg_v01, ReportId),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_reg_hw_md_int_ind_msg_v01, indication)
};

static const uint8_t sns_smgr_internal_dev_access_read_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_req_msg_v01, SensorId),

  0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_req_msg_v01, Addr),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_req_msg_v01, Bytes_len)
};

static const uint8_t sns_smgr_internal_dev_access_read_resp_msg_data_v01[] = {
  2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Result) - QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Result_valid)),
  0x10,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Result),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, SensorId) - QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, SensorId_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, SensorId),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Addr) - QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Addr_valid)),
  0x12,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Addr),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Bytes) - QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Bytes_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Bytes),
  ((SNS_SMGR_INTERNAL_MAX_DATA_LEN_V01) & 0xFF), ((SNS_SMGR_INTERNAL_MAX_DATA_LEN_V01) >> 8),
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Bytes) - QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_read_resp_msg_v01, Bytes_len)
};

static const uint8_t sns_smgr_internal_dev_access_write_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_req_msg_v01, SensorId),

  0x02,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_req_msg_v01, Addr),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_req_msg_v01, Bytes),
  ((SNS_SMGR_INTERNAL_MAX_DATA_LEN_V01) & 0xFF), ((SNS_SMGR_INTERNAL_MAX_DATA_LEN_V01) >> 8),
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_req_msg_v01, Bytes) - QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_req_msg_v01, Bytes_len)
};

static const uint8_t sns_smgr_internal_dev_access_write_resp_msg_data_v01[] = {
  2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_resp_msg_v01, Resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_resp_msg_v01, Result) - QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_resp_msg_v01, Result_valid)),
  0x10,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_resp_msg_v01, Result),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_resp_msg_v01, SensorId) - QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_resp_msg_v01, SensorId_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_smgr_internal_dev_access_write_resp_msg_v01, SensorId)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_SMGR_INTERNAL_SVC_message_table_v01[] = {
  {sizeof(sns_smgr_reg_hw_md_int_req_msg_v01), sns_smgr_reg_hw_md_int_req_msg_data_v01},
  {sizeof(sns_smgr_reg_hw_md_int_resp_msg_v01), sns_smgr_reg_hw_md_int_resp_msg_data_v01},
  {sizeof(sns_smgr_reg_hw_md_int_ind_msg_v01), sns_smgr_reg_hw_md_int_ind_msg_data_v01},
  {sizeof(sns_smgr_internal_dev_access_read_req_msg_v01), sns_smgr_internal_dev_access_read_req_msg_data_v01},
  {sizeof(sns_smgr_internal_dev_access_read_resp_msg_v01), sns_smgr_internal_dev_access_read_resp_msg_data_v01},
  {sizeof(sns_smgr_internal_dev_access_write_req_msg_v01), sns_smgr_internal_dev_access_write_req_msg_data_v01},
  {sizeof(sns_smgr_internal_dev_access_write_resp_msg_v01), sns_smgr_internal_dev_access_write_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(SNS_SMGR_INTERNAL_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_SMGR_INTERNAL_SVC_message_table_v01,
  SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_SMGR_INTERNAL_SVC_service_command_messages_v01[] = {
  {SNS_SMGR_INTERNAL_CANCEL_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {SNS_SMGR_INTERNAL_VERSION_REQ_V01, QMI_IDL_TYPE16(1, 2), 0},
  {SNS_SMGR_REG_HW_MD_INT_REQ_V01, QMI_IDL_TYPE16(0, 0), 12},
  {SNS_SMGR_INTERNAL_DEV_ACCESS_READ_REQ_V01, QMI_IDL_TYPE16(0, 3), 22},
  {SNS_SMGR_INTERNAL_DEV_ACCESS_WRITE_REQ_V01, QMI_IDL_TYPE16(0, 5), 2068}
};

static const qmi_idl_service_message_table_entry SNS_SMGR_INTERNAL_SVC_service_response_messages_v01[] = {
  {SNS_SMGR_INTERNAL_CANCEL_RESP_V01, QMI_IDL_TYPE16(1, 1), 5},
  {SNS_SMGR_INTERNAL_VERSION_RESP_V01, QMI_IDL_TYPE16(1, 3), 17},
  {SNS_SMGR_REG_HW_MD_INT_RESP_V01, QMI_IDL_TYPE16(0, 1), 13},
  {SNS_SMGR_INTERNAL_DEV_ACCESS_READ_RESP_V01, QMI_IDL_TYPE16(0, 4), 542},
  {SNS_SMGR_INTERNAL_DEV_ACCESS_WRITE_RESP_V01, QMI_IDL_TYPE16(0, 6), 14}
};

static const qmi_idl_service_message_table_entry SNS_SMGR_INTERNAL_SVC_service_indication_messages_v01[] = {
  {SNS_SMGR_REG_HW_MD_INT_IND_V01, QMI_IDL_TYPE16(0, 2), 8}
};

/*Service Object*/
const struct qmi_idl_service_object SNS_SMGR_INTERNAL_SVC_qmi_idl_service_object_v01 = {
  0x04,
  0x01,
  SNS_QMI_SVC_ID_13_V01,
  2068,
  { sizeof(SNS_SMGR_INTERNAL_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SMGR_INTERNAL_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SMGR_INTERNAL_SVC_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_SMGR_INTERNAL_SVC_service_command_messages_v01, SNS_SMGR_INTERNAL_SVC_service_response_messages_v01, SNS_SMGR_INTERNAL_SVC_service_indication_messages_v01},
  &SNS_SMGR_INTERNAL_SVC_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_SMGR_INTERNAL_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_SMGR_INTERNAL_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_SMGR_INTERNAL_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_SMGR_INTERNAL_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_SMGR_INTERNAL_SVC_qmi_idl_service_object_v01;
}

