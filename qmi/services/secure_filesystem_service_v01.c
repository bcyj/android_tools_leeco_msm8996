/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S E C U R E _ F I L E S Y S T E M _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the sfs service Data structures.

  Copyright (c) 2014 Qualcomm Technologies, Inc. All rights reserved.
Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.2 
   It was generated on: Thu Nov 13 2014 (Spin 0)
   From IDL File: secure_filesystem_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "secure_filesystem_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t qmi_sfs_tz_app_write_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_req_msg_v01, payload_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_req_msg_v01, sec_write_msg),
  ((QMI_SFS_TZ_APP_WRITE_REQ_MSG_SIZE_V01) & 0xFF), ((QMI_SFS_TZ_APP_WRITE_REQ_MSG_SIZE_V01) >> 8)
};

static const uint8_t qmi_sfs_tz_app_write_rsp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_rsp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_rsp_msg_v01, payload_size) - QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_rsp_msg_v01, payload_size_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_rsp_msg_v01, payload_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_rsp_msg_v01, sec_write_msg) - QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_rsp_msg_v01, sec_write_msg_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_write_rsp_msg_v01, sec_write_msg),
  QMI_SFS_TZ_APP_WRITE_RSP_MSG_SIZE_V01
};

static const uint8_t qmi_sfs_tz_app_read_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_req_msg_v01, payload_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_req_msg_v01, sec_read_msg),
  QMI_SFS_TZ_APP_READ_REQ_MSG_SIZE_V01
};

static const uint8_t qmi_sfs_tz_app_read_rsp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_rsp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_rsp_msg_v01, payload_size) - QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_rsp_msg_v01, payload_size_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_rsp_msg_v01, payload_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_rsp_msg_v01, sec_read_msg) - QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_rsp_msg_v01, sec_read_msg_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_sfs_tz_app_read_rsp_msg_v01, sec_read_msg),
  ((QMI_SFS_TZ_APP_READ_RSP_MSG_SIZE_V01) & 0xFF), ((QMI_SFS_TZ_APP_READ_RSP_MSG_SIZE_V01) >> 8)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry sfs_message_table_v01[] = {
  {sizeof(qmi_sfs_tz_app_write_req_msg_v01), qmi_sfs_tz_app_write_req_msg_data_v01},
  {sizeof(qmi_sfs_tz_app_write_rsp_msg_v01), qmi_sfs_tz_app_write_rsp_msg_data_v01},
  {sizeof(qmi_sfs_tz_app_read_req_msg_v01), qmi_sfs_tz_app_read_req_msg_data_v01},
  {sizeof(qmi_sfs_tz_app_read_rsp_msg_v01), qmi_sfs_tz_app_read_rsp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object sfs_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *sfs_qmi_idl_type_table_object_referenced_tables_v01[] =
{&sfs_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object sfs_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(sfs_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  sfs_message_table_v01,
  sfs_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry sfs_service_command_messages_v01[] = {
  {QMI_SFS_TZ_APP_WRITE_REQ_V01, QMI_IDL_TYPE16(0, 0), 626},
  {QMI_SFS_TZ_APP_READ_REQ_V01, QMI_IDL_TYPE16(0, 2), 114}
};

static const qmi_idl_service_message_table_entry sfs_service_response_messages_v01[] = {
  {QMI_SFS_TZ_APP_WRITE_RSP_V01, QMI_IDL_TYPE16(0, 1), 125},
  {QMI_SFS_TZ_APP_READ_RSP_V01, QMI_IDL_TYPE16(0, 3), 637}
};

/*Service Object*/
struct qmi_idl_service_object sfs_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x3D,
  637,
  { sizeof(sfs_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(sfs_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { sfs_service_command_messages_v01, sfs_service_response_messages_v01, NULL},
  &sfs_qmi_idl_type_table_object_v01,
  0x00,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type sfs_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SFS_V01_IDL_MAJOR_VERS != idl_maj_version || SFS_V01_IDL_MINOR_VERS != idl_min_version 
       || SFS_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&sfs_qmi_idl_service_object_v01;
}

