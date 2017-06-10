/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ T I M E _ A P I _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_TIME_SVC service Data structures.

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
   From IDL File: sns_time_api_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_time_api_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
/* 
 * sns_time_timestamp_req_msg is empty
 * static const uint8_t sns_time_timestamp_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t sns_time_timestamp_resp_msg_data_v01[] = {
  2,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, timestamp_dsps) - QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, timestamp_dsps_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, timestamp_dsps),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, timestamp_apps) - QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, timestamp_apps_valid)),
  0x11,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, timestamp_apps),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, error_code) - QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, error_code_valid)),
  0x12,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v01, error_code)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_TIME_SVC_message_table_v01[] = {
  {0, 0},
  {sizeof(sns_time_timestamp_resp_msg_v01), sns_time_timestamp_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_TIME_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_TIME_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_TIME_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_TIME_SVC_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(SNS_TIME_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_TIME_SVC_message_table_v01,
  SNS_TIME_SVC_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_TIME_SVC_service_command_messages_v01[] = {
  {SNS_TIME_TIMESTAMP_REQ_V01, TYPE16(0, 0), 0}
};

static const qmi_idl_service_message_table_entry SNS_TIME_SVC_service_response_messages_v01[] = {
  {SNS_TIME_TIMESTAMP_RESP_V01, TYPE16(0, 1), 30}
};

/*Service Object*/
const struct qmi_idl_service_object SNS_TIME_SVC_qmi_idl_service_object_v01 = {
  0x04,
  0x01,
  SNS_QMI_SVC_ID_22_V01,
  30,
  { sizeof(SNS_TIME_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_TIME_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { SNS_TIME_SVC_service_command_messages_v01, SNS_TIME_SVC_service_response_messages_v01, NULL},
  &SNS_TIME_SVC_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_TIME_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_TIME_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_TIME_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_TIME_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_TIME_SVC_qmi_idl_service_object_v01;
}

