/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S N S _ D E B U G _ I N T E R F A C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the SNS_DEBUG_SVC service Data structures.

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
   From IDL File: sns_debug_interface_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "sns_debug_interface_v01.h"
#include "sns_common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sns_debug_string_id_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, string_identifier),

  0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, module_id),

  0x03,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, str_priority),

  0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, param_values),
  SNS_DEBUG_NUM_PARAMS_ALLWD_V01,
  QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, param_values) - QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, param_values_len),

  0x05,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, line_number),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x06,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, file_name),
  SNS_DEBUG_MAX_FILENAME_SIZE_V01,
  QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, file_name) - QMI_IDL_OFFSET8(sns_debug_string_id_ind_msg_v01, file_name_len)
};

static const uint8_t sns_debug_log_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_debug_log_ind_msg_v01, log_pkt_type),

  0x02,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(sns_debug_log_ind_msg_v01, logpkt_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sns_debug_log_ind_msg_v01, log_pkt_contents),
  SNS_DEBUG_MAX_LOG_SIZE_V01,
  QMI_IDL_OFFSET8(sns_debug_log_ind_msg_v01, log_pkt_contents) - QMI_IDL_OFFSET8(sns_debug_log_ind_msg_v01, log_pkt_contents_len)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry SNS_DEBUG_SVC_message_table_v01[] = {
  {sizeof(sns_debug_string_id_ind_msg_v01), sns_debug_string_id_ind_msg_data_v01},
  {sizeof(sns_debug_log_ind_msg_v01), sns_debug_log_ind_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object SNS_DEBUG_SVC_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *SNS_DEBUG_SVC_qmi_idl_type_table_object_referenced_tables_v01[] =
{&SNS_DEBUG_SVC_qmi_idl_type_table_object_v01, &sns_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object SNS_DEBUG_SVC_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(SNS_DEBUG_SVC_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  SNS_DEBUG_SVC_message_table_v01,
  SNS_DEBUG_SVC_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry SNS_DEBUG_SVC_service_command_messages_v01[] = {
  {SNS_DEBUG_CANCEL_REQ_V01, TYPE16(1, 0), 0},
  {SNS_DEBUG_VERSION_REQ_V01, TYPE16(1, 2), 0}
};

static const qmi_idl_service_message_table_entry SNS_DEBUG_SVC_service_response_messages_v01[] = {
  {SNS_DEBUG_CANCEL_RESP_V01, TYPE16(1, 1), 5},
  {SNS_DEBUG_VERSION_RESP_V01, TYPE16(1, 3), 17}
};

static const qmi_idl_service_message_table_entry SNS_DEBUG_SVC_service_indication_messages_v01[] = {
  {SNS_DEBUG_STRING_ID_IND_V01, TYPE16(0, 0), 70},
  {SNS_DEBUG_LOG_IND_V01, TYPE16(0, 1), 614}
};

/*Service Object*/
const struct qmi_idl_service_object SNS_DEBUG_SVC_qmi_idl_service_object_v01 = {
  0x04,
  0x01,
  SNS_QMI_SVC_ID_7_V01,
  614,
  { sizeof(SNS_DEBUG_SVC_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_DEBUG_SVC_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(SNS_DEBUG_SVC_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { SNS_DEBUG_SVC_service_command_messages_v01, SNS_DEBUG_SVC_service_response_messages_v01, SNS_DEBUG_SVC_service_indication_messages_v01},
  &SNS_DEBUG_SVC_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type SNS_DEBUG_SVC_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SNS_DEBUG_SVC_V01_IDL_MAJOR_VERS != idl_maj_version || SNS_DEBUG_SVC_V01_IDL_MINOR_VERS != idl_min_version 
       || SNS_DEBUG_SVC_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&SNS_DEBUG_SVC_qmi_idl_service_object_v01;
}

