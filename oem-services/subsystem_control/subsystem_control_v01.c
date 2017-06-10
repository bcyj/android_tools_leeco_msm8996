/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S U B S Y S T E M _ C O N T R O L _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the ssctl service Data structures.

  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.0
   It requires encode/decode library version 5 or later
   It was generated on: Mon Dec 24 2012 (Spin )
   From IDL File: subsystem_control_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "subsystem_control_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
/*
 * qmi_ssctl_restart_req_msg is empty
 * static const uint8_t qmi_ssctl_restart_req_msg_data_v01[] = {
 * };
 */

static const uint8_t qmi_ssctl_restart_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_ssctl_restart_resp_msg_v01, resp),
  0, 1
};

/*
 * qmi_ssctl_restart_ind_msg is empty
 * static const uint8_t qmi_ssctl_restart_ind_msg_data_v01[] = {
 * };
 */

/*
 * qmi_ssctl_shutdown_req_msg is empty
 * static const uint8_t qmi_ssctl_shutdown_req_msg_data_v01[] = {
 * };
 */

static const uint8_t qmi_ssctl_shutdown_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_ssctl_shutdown_resp_msg_v01, resp),
  0, 1
};

/*
 * qmi_ssctl_shutdown_ind_msg is empty
 * static const uint8_t qmi_ssctl_shutdown_ind_msg_data_v01[] = {
 * };
 */

/*
 * qmi_ssctl_get_failure_reason_req_msg is empty
 * static const uint8_t qmi_ssctl_get_failure_reason_req_msg_data_v01[] = {
 * };
 */

static const uint8_t qmi_ssctl_get_failure_reason_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_ssctl_get_failure_reason_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_ssctl_get_failure_reason_resp_msg_v01, error_message) - QMI_IDL_OFFSET8(qmi_ssctl_get_failure_reason_resp_msg_v01, error_message_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_ssctl_get_failure_reason_resp_msg_v01, error_message),
  QMI_SSCTL_ERROR_MSG_LENGTH_V01,
  QMI_IDL_OFFSET8(qmi_ssctl_get_failure_reason_resp_msg_v01, error_message) - QMI_IDL_OFFSET8(qmi_ssctl_get_failure_reason_resp_msg_v01, error_message_len)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry ssctl_message_table_v01[] = {
  {0, 0},
  {sizeof(qmi_ssctl_restart_resp_msg_v01), qmi_ssctl_restart_resp_msg_data_v01},
  {0, 0},
  {0, 0},
  {sizeof(qmi_ssctl_shutdown_resp_msg_v01), qmi_ssctl_shutdown_resp_msg_data_v01},
  {0, 0},
  {0, 0},
  {sizeof(qmi_ssctl_get_failure_reason_resp_msg_v01), qmi_ssctl_get_failure_reason_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object ssctl_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *ssctl_qmi_idl_type_table_object_referenced_tables_v01[] =
{&ssctl_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object ssctl_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(ssctl_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  ssctl_message_table_v01,
  ssctl_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry ssctl_service_command_messages_v01[] = {
  {QMI_SSCTL_RESTART_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_SSCTL_SHUTDOWN_REQ_V01, QMI_IDL_TYPE16(0, 3), 0},
  {QMI_SSCTL_GET_FAILURE_REASON_REQ_V01, QMI_IDL_TYPE16(0, 6), 0}
};

static const qmi_idl_service_message_table_entry ssctl_service_response_messages_v01[] = {
  {QMI_SSCTL_RESTART_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_SSCTL_SHUTDOWN_RESP_V01, QMI_IDL_TYPE16(0, 4), 7},
  {QMI_SSCTL_GET_FAILURE_REASON_RESP_V01, QMI_IDL_TYPE16(0, 7), 101}
};

static const qmi_idl_service_message_table_entry ssctl_service_indication_messages_v01[] = {
  {QMI_SSCTL_RESTART_READY_IND_V01, QMI_IDL_TYPE16(0, 2), 0},
  {QMI_SSCTL_SHUTDOWN_READY_IND_V01, QMI_IDL_TYPE16(0, 5), 0}
};

/*Service Object*/
struct qmi_idl_service_object ssctl_qmi_idl_service_object_v01 = {
  0x05,
  0x01,
  0x2B,
  101,
  { sizeof(ssctl_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ssctl_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ssctl_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { ssctl_service_command_messages_v01, ssctl_service_response_messages_v01, ssctl_service_indication_messages_v01},
  &ssctl_qmi_idl_type_table_object_v01,
  0x00,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type ssctl_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SSCTL_V01_IDL_MAJOR_VERS != idl_maj_version || SSCTL_V01_IDL_MINOR_VERS != idl_min_version
       || SSCTL_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&ssctl_qmi_idl_service_object_v01;
}

