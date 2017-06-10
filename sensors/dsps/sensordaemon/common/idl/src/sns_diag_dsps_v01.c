/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ D I A G _ D S P S _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_DIAG_DSPS_SVC service Data structures.

  Copyright (c) 2012-14 Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.13
   It was generated on: Wed Sep  3 2014 (Spin 0)
   From IDL File: sns_diag_dsps_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_diag_dsps_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
static const uint8_t sns_diag_mask_s_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_diag_mask_s_v01, mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t sns_diag_debug_options_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_diag_debug_options_v01, option),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(sns_diag_debug_options_v01, value),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t sns_diag_set_debug_options_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_diag_set_debug_options_req_msg_v01, options),
  SNS_DIAG_MAX_OPTIONS_V01,
  QMI_IDL_OFFSET8(sns_diag_set_debug_options_req_msg_v01, options) - QMI_IDL_OFFSET8(sns_diag_set_debug_options_req_msg_v01, options_len),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t sns_diag_set_debug_options_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_diag_set_debug_options_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t sns_diag_set_log_mask_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_diag_set_log_mask_req_msg_v01, log_mask),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sns_diag_set_log_mask_req_msg_v01, log_mask_ext) - QMI_IDL_OFFSET8(sns_diag_set_log_mask_req_msg_v01, log_mask_ext_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_diag_set_log_mask_req_msg_v01, log_mask_ext),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t sns_diag_set_log_mask_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_diag_set_log_mask_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t sns_diag_set_debug_mask_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_diag_set_debug_mask_req_msg_v01, debug_mask),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t sns_diag_set_debug_mask_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 2,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sns_diag_set_debug_mask_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  SNS_DIAG_DSPS_SVC_type_table_v01[] = {
  {sizeof(sns_diag_mask_s_v01), sns_diag_mask_s_data_v01},
  {sizeof(sns_diag_debug_options_v01), sns_diag_debug_options_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry SNS_DIAG_DSPS_SVC_message_table_v01[] = {
  {sizeof(sns_diag_set_debug_options_req_msg_v01), sns_diag_set_debug_options_req_msg_data_v01},
  {sizeof(sns_diag_set_debug_options_resp_msg_v01), sns_diag_set_debug_options_resp_msg_data_v01},
  {sizeof(sns_diag_set_log_mask_req_msg_v01), sns_diag_set_log_mask_req_msg_data_v01},
  {sizeof(sns_diag_set_log_mask_resp_msg_v01), sns_diag_set_log_mask_resp_msg_data_v01},
  {sizeof(sns_diag_set_debug_mask_req_msg_v01), sns_diag_set_debug_mask_req_msg_data_v01},
  {sizeof(sns_diag_set_debug_mask_resp_msg_v01), sns_diag_set_debug_mask_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_DIAG_DSPS_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_DIAG_DSPS_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_DIAG_DSPS_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_DIAG_DSPS_SVC_qmi_idl_type_table_object_v01 = {
  sizeof(SNS_DIAG_DSPS_SVC_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(SNS_DIAG_DSPS_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  SNS_DIAG_DSPS_SVC_type_table_v01,
  SNS_DIAG_DSPS_SVC_message_table_v01,
  SNS_DIAG_DSPS_SVC_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_DIAG_DSPS_SVC_service_command_messages_v01[] = {
  {SNS_DIAG_DSPS_CANCEL_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {SNS_DIAG_DSPS_VERSION_REQ_V01, QMI_IDL_TYPE16(1, 2), 0},
  {SNS_DIAG_SET_LOG_MASK_REQ_V01, QMI_IDL_TYPE16(0, 2), 22},
  {SNS_DIAG_SET_DEBUG_MASK_REQ_V01, QMI_IDL_TYPE16(0, 4), 11},
  {SNS_DIAG_SET_DEBUG_OPTIONS_REQ_V01, QMI_IDL_TYPE16(0, 0), 196}
};

static const qmi_idl_service_message_table_entry SNS_DIAG_DSPS_SVC_service_response_messages_v01[] = {
  {SNS_DIAG_DSPS_CANCEL_RESP_V01, QMI_IDL_TYPE16(1, 1), 5},
  {SNS_DIAG_DSPS_VERSION_RESP_V01, QMI_IDL_TYPE16(1, 3), 17},
  {SNS_DIAG_SET_LOG_MASK_RESP_V01, QMI_IDL_TYPE16(0, 3), 5},
  {SNS_DIAG_SET_DEBUG_MASK_RESP_V01, QMI_IDL_TYPE16(0, 5), 5},
  {SNS_DIAG_SET_DEBUG_OPTIONS_RESP_V01, QMI_IDL_TYPE16(0, 1), 5}
};

/*Service Object*/
struct qmi_idl_service_object SNS_DIAG_DSPS_SVC_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  SNS_QMI_SVC_ID_8_V01,
  196,
  { sizeof(SNS_DIAG_DSPS_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_DIAG_DSPS_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { SNS_DIAG_DSPS_SVC_service_command_messages_v01, SNS_DIAG_DSPS_SVC_service_response_messages_v01, NULL},
  &SNS_DIAG_DSPS_SVC_qmi_idl_type_table_object_v01,
  0x03,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_DIAG_DSPS_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_DIAG_DSPS_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_DIAG_DSPS_SVC_V01_IDL_MINOR_VERS != idl_min_version
       || SNS_DIAG_DSPS_SVC_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&SNS_DIAG_DSPS_SVC_qmi_idl_service_object_v01;
}

