/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S P E C I F I C _ A B S O R P T I O N _ R A T E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the sar service Data structures.

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.


  $Header: //components/rel/qmimsgs.mpss/3.4/sar/src/specific_absorption_rate_v01.c#2 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2
   It requires encode/decode library version 5 or later
   It was generated on: Thu May 30 2013 (Spin 0)
   From IDL File: specific_absorption_rate_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "specific_absorption_rate_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t sar_rf_set_state_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_rf_set_state_req_msg_v01, sar_rf_state),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_rf_set_state_req_msg_v01, compatibility_key) - QMI_IDL_OFFSET8(sar_rf_set_state_req_msg_v01, compatibility_key_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_rf_set_state_req_msg_v01, compatibility_key)
};

static const uint8_t sar_rf_set_state_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_rf_set_state_resp_msg_v01, resp),
  0, 1
};

/*
 * sar_rf_get_state_req_msg is empty
 * static const uint8_t sar_rf_get_state_req_msg_data_v01[] = {
 * };
 */

static const uint8_t sar_rf_get_state_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_rf_get_state_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_rf_get_state_resp_msg_v01, sar_rf_state) - QMI_IDL_OFFSET8(sar_rf_get_state_resp_msg_v01, sar_rf_state_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_rf_get_state_resp_msg_v01, sar_rf_state)
};

/*
 * sar_rf_get_compatibility_key_req_msg is empty
 * static const uint8_t sar_rf_get_compatibility_key_req_msg_data_v01[] = {
 * };
 */

static const uint8_t sar_rf_get_compatibility_key_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_rf_get_compatibility_key_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_rf_get_compatibility_key_resp_msg_v01, compatibility_key) - QMI_IDL_OFFSET8(sar_rf_get_compatibility_key_resp_msg_v01, compatibility_key_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_rf_get_compatibility_key_resp_msg_v01, compatibility_key)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry sar_message_table_v01[] = {
  {sizeof(sar_rf_set_state_req_msg_v01), sar_rf_set_state_req_msg_data_v01},
  {sizeof(sar_rf_set_state_resp_msg_v01), sar_rf_set_state_resp_msg_data_v01},
  {0, 0},
  {sizeof(sar_rf_get_state_resp_msg_v01), sar_rf_get_state_resp_msg_data_v01},
  {0, 0},
  {sizeof(sar_rf_get_compatibility_key_resp_msg_v01), sar_rf_get_compatibility_key_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object sar_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *sar_qmi_idl_type_table_object_referenced_tables_v01[] =
{&sar_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object sar_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(sar_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  sar_message_table_v01,
  sar_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry sar_service_command_messages_v01[] = {
  {QMI_SAR_RF_SET_STATE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 0), 14},
  {QMI_SAR_RF_GET_STATE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 2), 0},
  {QMI_SAR_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_SAR_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_SAR_GET_COMPATIBILITY_KEY_REQ_MSG_V01, QMI_IDL_TYPE16(0, 4), 0}
};

static const qmi_idl_service_message_table_entry sar_service_response_messages_v01[] = {
  {QMI_SAR_RF_SET_STATE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_SAR_RF_GET_STATE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 3), 14},
  {QMI_SAR_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_SAR_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_SAR_GET_COMPATIBILITY_KEY_RESP_MSG_V01, QMI_IDL_TYPE16(0, 5), 14}
};

/*Service Object*/
struct qmi_idl_service_object sar_qmi_idl_service_object_v01 = {
  0x05,
  0x01,
  0x11,
  8204,
  { sizeof(sar_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(sar_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    0 },
  { sar_service_command_messages_v01, sar_service_response_messages_v01, NULL},
  &sar_qmi_idl_type_table_object_v01,
  0x03,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type sar_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SAR_V01_IDL_MAJOR_VERS != idl_maj_version || SAR_V01_IDL_MINOR_VERS != idl_min_version
       || SAR_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&sar_qmi_idl_service_object_v01;
}

