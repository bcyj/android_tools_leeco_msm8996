/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ T I M E _ A P I _ V 0 2  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_TIME2_SVC service Data structures.

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.6 
   It was generated on: Tue Feb 25 2014 (Spin 0)
   From IDL File: sns_time_api_v02.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_time_api_v02.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sns_time_timestamp_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_req_msg_v02, reg_report) - QMI_IDL_OFFSET8(sns_time_timestamp_req_msg_v02, reg_report_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_req_msg_v02, reg_report)
};

static const uint8_t sns_time_timestamp_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_dsps) - QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_dsps_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_dsps),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_apps) - QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_apps_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_apps),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, error_code) - QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, error_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, error_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, dsps_rollover_cnt) - QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, dsps_rollover_cnt_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, dsps_rollover_cnt),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_apps_boottime) - QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_apps_boottime_valid)),
  0x14,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_resp_msg_v02, timestamp_apps_boottime)
};

static const uint8_t sns_time_timestamp_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_ind_msg_v02, timestamp_dsps),

  0x02,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_ind_msg_v02, timestamp_apps),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_ind_msg_v02, dsps_rollover_cnt),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_time_timestamp_ind_msg_v02, timestamp_apps_boottime) - QMI_IDL_OFFSET8(sns_time_timestamp_ind_msg_v02, timestamp_apps_boottime_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_time_timestamp_ind_msg_v02, timestamp_apps_boottime)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_TIME2_SVC_message_table_v02[] = {
  {sizeof(sns_time_timestamp_req_msg_v02), sns_time_timestamp_req_msg_data_v02},
  {sizeof(sns_time_timestamp_resp_msg_v02), sns_time_timestamp_resp_msg_data_v02},
  {sizeof(sns_time_timestamp_ind_msg_v02), sns_time_timestamp_ind_msg_data_v02}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_TIME2_SVC_qmi_idl_type_table_object_v02;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_TIME2_SVC_qmi_idl_type_table_object_referenced_tables_v02[] =
{&SNS_TIME2_SVC_qmi_idl_type_table_object_v02, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_TIME2_SVC_qmi_idl_type_table_object_v02 = {
  0,
  sizeof(SNS_TIME2_SVC_message_table_v02)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_TIME2_SVC_message_table_v02,
  SNS_TIME2_SVC_qmi_idl_type_table_object_referenced_tables_v02,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_TIME2_SVC_service_command_messages_v02[] = {
  {SNS_TIME_CANCEL_REQ_V02, QMI_IDL_TYPE16(1, 0), 0},
  {SNS_TIME_VERSION_REQ_V02, QMI_IDL_TYPE16(1, 2), 0},
  {SNS_TIME_TIMESTAMP_REQ_V02, QMI_IDL_TYPE16(0, 0), 4}
};

static const qmi_idl_service_message_table_entry SNS_TIME2_SVC_service_response_messages_v02[] = {
  {SNS_TIME_CANCEL_RESP_V02, QMI_IDL_TYPE16(1, 1), 5},
  {SNS_TIME_VERSION_RESP_V02, QMI_IDL_TYPE16(1, 3), 17},
  {SNS_TIME_TIMESTAMP_RESP_V02, QMI_IDL_TYPE16(0, 1), 48}
};

static const qmi_idl_service_message_table_entry SNS_TIME2_SVC_service_indication_messages_v02[] = {
  {SNS_TIME_TIMESTAMP_IND_V02, QMI_IDL_TYPE16(0, 2), 36}
};

/*Service Object*/
struct qmi_idl_service_object SNS_TIME2_SVC_qmi_idl_service_object_v02 = {
  0x06,
  0x02,
  SNS_QMI_SVC_ID_24_V01,
  48,
  { sizeof(SNS_TIME2_SVC_service_command_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_TIME2_SVC_service_response_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_TIME2_SVC_service_indication_messages_v02)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_TIME2_SVC_service_command_messages_v02, SNS_TIME2_SVC_service_response_messages_v02, SNS_TIME2_SVC_service_indication_messages_v02},
  &SNS_TIME2_SVC_qmi_idl_type_table_object_v02,
  0x05,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_TIME2_SVC_get_service_object_internal_v02
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_TIME2_SVC_V02_IDL_MAJOR_VERS != idl_maj_version || SNS_TIME2_SVC_V02_IDL_MINOR_VERS != idl_min_version 
       || SNS_TIME2_SVC_V02_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_TIME2_SVC_qmi_idl_service_object_v02;
}

