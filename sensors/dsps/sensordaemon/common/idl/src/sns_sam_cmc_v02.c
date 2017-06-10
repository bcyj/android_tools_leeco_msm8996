/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ S A M _ C M C _ V 0 2  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_SAM_CMC_SVC service Data structures.

  
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13 
   It was generated on: Tue Sep 23 2014 (Spin 0)
   From IDL File: sns_sam_cmc_v02.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_sam_cmc_v02.h"
#include "sns_sam_common_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
static const uint8_t sns_sam_cmc_report_data_s_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_report_data_s_v02, motion_state),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_report_data_s_v02, motion_state_probability),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_sam_cmc_batch_item_s_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_item_s_v02, timestamp),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_item_s_v02, report),
  QMI_IDL_TYPE88(0, 0),
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_item_s_v02, ms_event),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t sns_sam_cmc_enable_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_enable_req_msg_v02, notify_suspend) - QMI_IDL_OFFSET8(sns_sam_cmc_enable_req_msg_v02, notify_suspend_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_enable_req_msg_v02, notify_suspend),
  QMI_IDL_TYPE88(2, 1)
};

static const uint8_t sns_sam_cmc_enable_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_enable_resp_msg_v02, resp),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_enable_resp_msg_v02, instance_id) - QMI_IDL_OFFSET8(sns_sam_cmc_enable_resp_msg_v02, instance_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_enable_resp_msg_v02, instance_id)
};

static const uint8_t sns_sam_cmc_disable_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_disable_req_msg_v02, instance_id)
};

static const uint8_t sns_sam_cmc_disable_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_disable_resp_msg_v02, resp),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_disable_resp_msg_v02, instance_id) - QMI_IDL_OFFSET8(sns_sam_cmc_disable_resp_msg_v02, instance_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_disable_resp_msg_v02, instance_id)
};

static const uint8_t sns_sam_cmc_get_report_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_get_report_req_msg_v02, instance_id)
};

static const uint8_t sns_sam_cmc_get_report_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, resp),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, instance_id) - QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, instance_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, instance_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, timestamp) - QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, timestamp_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, timestamp),
  SNS_SAM_CMC_MS_NUM_V02,
  QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, timestamp) - QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, timestamp_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, report_data) - QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, report_data_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, report_data),
  SNS_SAM_CMC_MS_NUM_V02,
  QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, report_data) - QMI_IDL_OFFSET8(sns_sam_cmc_get_report_resp_msg_v02, report_data_len),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t sns_sam_cmc_report_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_report_ind_msg_v02, instance_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_report_ind_msg_v02, timestamp),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_report_ind_msg_v02, report_data),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_report_ind_msg_v02, ms_event) - QMI_IDL_OFFSET8(sns_sam_cmc_report_ind_msg_v02, ms_event_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_report_ind_msg_v02, ms_event)
};

static const uint8_t sns_sam_cmc_error_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_error_ind_msg_v02, instance_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_error_ind_msg_v02, timestamp),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_error_ind_msg_v02, error)
};

static const uint8_t sns_sam_cmc_update_reporting_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_req_msg_v02, instance_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_req_msg_v02, enable),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_req_msg_v02, report_ms_type),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_req_msg_v02, report_event_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_req_msg_v02, report_motion_state)
};

static const uint8_t sns_sam_cmc_update_reporting_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_resp_msg_v02, resp),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_resp_msg_v02, instance_id) - QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_resp_msg_v02, instance_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_reporting_resp_msg_v02, instance_id)
};

static const uint8_t sns_sam_cmc_batch_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_req_msg_v02, instance_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_req_msg_v02, batch_period),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_batch_req_msg_v02, req_type) - QMI_IDL_OFFSET8(sns_sam_cmc_batch_req_msg_v02, req_type_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_req_msg_v02, req_type)
};

static const uint8_t sns_sam_cmc_batch_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, resp),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, instance_id) - QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, instance_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, instance_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, max_batch_size) - QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, max_batch_size_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, max_batch_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, timestamp) - QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, timestamp_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_resp_msg_v02, timestamp)
};

static const uint8_t sns_sam_cmc_batch_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_ind_msg_v02, instance_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_ind_msg_v02, items),
  SNS_SAM_CMC_MAX_ITEMS_IN_BATCH_V02,
  QMI_IDL_OFFSET8(sns_sam_cmc_batch_ind_msg_v02, items) - QMI_IDL_OFFSET8(sns_sam_cmc_batch_ind_msg_v02, items_len),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(sns_sam_cmc_batch_ind_msg_v02, ind_type) - QMI_IDL_OFFSET16RELATIVE(sns_sam_cmc_batch_ind_msg_v02, ind_type_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(sns_sam_cmc_batch_ind_msg_v02, ind_type)
};

static const uint8_t sns_sam_cmc_update_batch_period_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_req_msg_v02, instance_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_req_msg_v02, active_batch_period)
};

static const uint8_t sns_sam_cmc_update_batch_period_resp_msg_data_v02[] = {
  2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_resp_msg_v02, resp),
  QMI_IDL_TYPE88(2, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_resp_msg_v02, instance_id) - QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_resp_msg_v02, instance_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_resp_msg_v02, instance_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_resp_msg_v02, timestamp) - QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_resp_msg_v02, timestamp_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_sam_cmc_update_batch_period_resp_msg_v02, timestamp)
};

/* Type Table */
static const qmi_idl_type_table_entry  SNS_SAM_CMC_SVC_type_table_v02[] = {
  {sizeof(sns_sam_cmc_report_data_s_v02), sns_sam_cmc_report_data_s_data_v02},
  {sizeof(sns_sam_cmc_batch_item_s_v02), sns_sam_cmc_batch_item_s_data_v02}
};

/* Message Table */
static const qmi_idl_message_table_entry SNS_SAM_CMC_SVC_message_table_v02[] = {
  {sizeof(sns_sam_cmc_enable_req_msg_v02), sns_sam_cmc_enable_req_msg_data_v02},
  {sizeof(sns_sam_cmc_enable_resp_msg_v02), sns_sam_cmc_enable_resp_msg_data_v02},
  {sizeof(sns_sam_cmc_disable_req_msg_v02), sns_sam_cmc_disable_req_msg_data_v02},
  {sizeof(sns_sam_cmc_disable_resp_msg_v02), sns_sam_cmc_disable_resp_msg_data_v02},
  {sizeof(sns_sam_cmc_get_report_req_msg_v02), sns_sam_cmc_get_report_req_msg_data_v02},
  {sizeof(sns_sam_cmc_get_report_resp_msg_v02), sns_sam_cmc_get_report_resp_msg_data_v02},
  {sizeof(sns_sam_cmc_report_ind_msg_v02), sns_sam_cmc_report_ind_msg_data_v02},
  {sizeof(sns_sam_cmc_error_ind_msg_v02), sns_sam_cmc_error_ind_msg_data_v02},
  {sizeof(sns_sam_cmc_update_reporting_req_msg_v02), sns_sam_cmc_update_reporting_req_msg_data_v02},
  {sizeof(sns_sam_cmc_update_reporting_resp_msg_v02), sns_sam_cmc_update_reporting_resp_msg_data_v02},
  {sizeof(sns_sam_cmc_batch_req_msg_v02), sns_sam_cmc_batch_req_msg_data_v02},
  {sizeof(sns_sam_cmc_batch_resp_msg_v02), sns_sam_cmc_batch_resp_msg_data_v02},
  {sizeof(sns_sam_cmc_batch_ind_msg_v02), sns_sam_cmc_batch_ind_msg_data_v02},
  {sizeof(sns_sam_cmc_update_batch_period_req_msg_v02), sns_sam_cmc_update_batch_period_req_msg_data_v02},
  {sizeof(sns_sam_cmc_update_batch_period_resp_msg_v02), sns_sam_cmc_update_batch_period_resp_msg_data_v02}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_SAM_CMC_SVC_qmi_idl_type_table_object_v02;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_SAM_CMC_SVC_qmi_idl_type_table_object_referenced_tables_v02[] =
{&SNS_SAM_CMC_SVC_qmi_idl_type_table_object_v02, &sns_sam_common_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_SAM_CMC_SVC_qmi_idl_type_table_object_v02 = {
  sizeof(SNS_SAM_CMC_SVC_type_table_v02)/sizeof(qmi_idl_type_table_entry ),
  sizeof(SNS_SAM_CMC_SVC_message_table_v02)/sizeof(qmi_idl_message_table_entry),
  1,
  SNS_SAM_CMC_SVC_type_table_v02,
  SNS_SAM_CMC_SVC_message_table_v02,
  SNS_SAM_CMC_SVC_qmi_idl_type_table_object_referenced_tables_v02,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_SAM_CMC_SVC_service_command_messages_v02[] = {
  {SNS_SAM_CMC_CANCEL_REQ_V02, QMI_IDL_TYPE16(2, 0), 0},
  {SNS_SAM_CMC_VERSION_REQ_V02, QMI_IDL_TYPE16(2, 2), 0},
  {SNS_SAM_CMC_ENABLE_REQ_V02, QMI_IDL_TYPE16(0, 0), 8},
  {SNS_SAM_CMC_DISABLE_REQ_V02, QMI_IDL_TYPE16(0, 2), 4},
  {SNS_SAM_CMC_GET_REPORT_REQ_V02, QMI_IDL_TYPE16(0, 4), 4},
  {SNS_SAM_CMC_UPDATE_REPORTING_REQ_V02, QMI_IDL_TYPE16(0, 8), 29},
  {SNS_SAM_CMC_BATCH_REQ_V02, QMI_IDL_TYPE16(0, 10), 18},
  {SNS_SAM_CMC_UPDATE_BATCH_PERIOD_REQ_V02, QMI_IDL_TYPE16(0, 13), 11},
  {SNS_SAM_CMC_GET_ATTRIBUTES_REQ_V02, QMI_IDL_TYPE16(1, 0), 0}
};

static const qmi_idl_service_message_table_entry SNS_SAM_CMC_SVC_service_response_messages_v02[] = {
  {SNS_SAM_CMC_CANCEL_RESP_V02, QMI_IDL_TYPE16(2, 1), 5},
  {SNS_SAM_CMC_VERSION_RESP_V02, QMI_IDL_TYPE16(2, 3), 17},
  {SNS_SAM_CMC_ENABLE_RESP_V02, QMI_IDL_TYPE16(0, 1), 9},
  {SNS_SAM_CMC_DISABLE_RESP_V02, QMI_IDL_TYPE16(0, 3), 9},
  {SNS_SAM_CMC_GET_REPORT_RESP_V02, QMI_IDL_TYPE16(0, 5), 125},
  {SNS_SAM_CMC_UPDATE_REPORTING_RESP_V02, QMI_IDL_TYPE16(0, 9), 9},
  {SNS_SAM_CMC_BATCH_RESP_V02, QMI_IDL_TYPE16(0, 11), 23},
  {SNS_SAM_CMC_UPDATE_BATCH_PERIOD_RESP_V02, QMI_IDL_TYPE16(0, 14), 16},
  {SNS_SAM_CMC_GET_ATTRIBUTES_RESP_V02, QMI_IDL_TYPE16(1, 1), 86}
};

static const qmi_idl_service_message_table_entry SNS_SAM_CMC_SVC_service_indication_messages_v02[] = {
  {SNS_SAM_CMC_REPORT_IND_V02, QMI_IDL_TYPE16(0, 6), 29},
  {SNS_SAM_CMC_ERROR_IND_V02, QMI_IDL_TYPE16(0, 7), 15},
  {SNS_SAM_CMC_BATCH_IND_V02, QMI_IDL_TYPE16(0, 12), 1980}
};

/*Service Object*/
struct qmi_idl_service_object SNS_SAM_CMC_SVC_qmi_idl_service_object_v02 = {
  0x06,
  0x02,
  SNS_QMI_SVC_ID_41_V01,
  1980,
  { sizeof(SNS_SAM_CMC_SVC_service_command_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SAM_CMC_SVC_service_response_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_SAM_CMC_SVC_service_indication_messages_v02)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_SAM_CMC_SVC_service_command_messages_v02, SNS_SAM_CMC_SVC_service_response_messages_v02, SNS_SAM_CMC_SVC_service_indication_messages_v02},
  &SNS_SAM_CMC_SVC_qmi_idl_type_table_object_v02,
  0x02,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_SAM_CMC_SVC_get_service_object_internal_v02
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_SAM_CMC_SVC_V02_IDL_MAJOR_VERS != idl_maj_version || SNS_SAM_CMC_SVC_V02_IDL_MINOR_VERS != idl_min_version 
       || SNS_SAM_CMC_SVC_V02_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_SAM_CMC_SVC_qmi_idl_service_object_v02;
}

