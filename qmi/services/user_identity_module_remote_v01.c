/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        U S E R _ I D E N T I T Y _ M O D U L E _ R E M O T E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the uim_remote service Data structures.

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/uimrmt/main/latest/src/user_identity_module_remote_v01.c#4 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.7
   It was generated on: Wed May 21 2014 (Spin 0)
   From IDL File: user_identity_module_remote_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "user_identity_module_remote_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t uim_remote_event_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_event_info_type_v01, event),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_event_info_type_v01, slot),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_remote_response_apdu_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_response_apdu_info_type_v01, total_response_apdu_size),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_response_apdu_info_type_v01, response_apdu_segment_offset),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/*
 * uim_remote_reset_req_msg is empty
 * static const uint8_t uim_remote_reset_req_msg_data_v01[] = {
 * };
 */

static const uint8_t uim_remote_reset_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_remote_reset_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_remote_event_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, event_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, atr) - QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, atr_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, atr),
  QMI_UIM_REMOTE_MAX_ATR_LEN_V01,
  QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, atr) - QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, atr_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, wakeup_support) - QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, wakeup_support_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, wakeup_support),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, error_cause) - QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, error_cause_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_event_req_msg_v01, error_cause)
};

static const uint8_t uim_remote_event_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_remote_event_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_remote_apdu_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, apdu_status),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, slot),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, apdu_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, response_apdu_info) - QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, response_apdu_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, response_apdu_info),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, response_apdu_segment) - QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, response_apdu_segment_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, response_apdu_segment),
  ((QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01) & 0xFF), ((QMI_UIM_REMOTE_MAX_RESPONSE_APDU_SEGMENT_LEN_V01) >> 8),
  QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, response_apdu_segment) - QMI_IDL_OFFSET8(uim_remote_apdu_req_msg_v01, response_apdu_segment_len)
};

static const uint8_t uim_remote_apdu_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_remote_apdu_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_remote_apdu_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_apdu_ind_msg_v01, slot),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_apdu_ind_msg_v01, apdu_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_remote_apdu_ind_msg_v01, command_apdu),
  ((QMI_UIM_REMOTE_MAX_COMMAND_APDU_LEN_V01) & 0xFF), ((QMI_UIM_REMOTE_MAX_COMMAND_APDU_LEN_V01) >> 8),
  QMI_IDL_OFFSET8(uim_remote_apdu_ind_msg_v01, command_apdu) - QMI_IDL_OFFSET8(uim_remote_apdu_ind_msg_v01, command_apdu_len)
};

static const uint8_t uim_remote_connect_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_connect_ind_msg_v01, slot)
};

static const uint8_t uim_remote_disconnect_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_disconnect_ind_msg_v01, slot)
};

static const uint8_t uim_remote_card_power_down_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_card_power_down_ind_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_card_power_down_ind_msg_v01, mode) - QMI_IDL_OFFSET8(uim_remote_card_power_down_ind_msg_v01, mode_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_card_power_down_ind_msg_v01, mode)
};

static const uint8_t uim_remote_card_power_up_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_card_power_up_ind_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_card_power_up_ind_msg_v01, response_timeout) - QMI_IDL_OFFSET8(uim_remote_card_power_up_ind_msg_v01, response_timeout_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_card_power_up_ind_msg_v01, response_timeout),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_card_power_up_ind_msg_v01, voltage_class) - QMI_IDL_OFFSET8(uim_remote_card_power_up_ind_msg_v01, voltage_class_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_card_power_up_ind_msg_v01, voltage_class)
};

static const uint8_t uim_remote_card_reset_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_remote_card_reset_ind_msg_v01, slot)
};

/* Type Table */
static const qmi_idl_type_table_entry  uim_remote_type_table_v01[] = {
  {sizeof(uim_remote_event_info_type_v01), uim_remote_event_info_type_data_v01},
  {sizeof(uim_remote_response_apdu_info_type_v01), uim_remote_response_apdu_info_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry uim_remote_message_table_v01[] = {
  {sizeof(uim_remote_reset_req_msg_v01), 0},
  {sizeof(uim_remote_reset_resp_msg_v01), uim_remote_reset_resp_msg_data_v01},
  {sizeof(uim_remote_event_req_msg_v01), uim_remote_event_req_msg_data_v01},
  {sizeof(uim_remote_event_resp_msg_v01), uim_remote_event_resp_msg_data_v01},
  {sizeof(uim_remote_apdu_req_msg_v01), uim_remote_apdu_req_msg_data_v01},
  {sizeof(uim_remote_apdu_resp_msg_v01), uim_remote_apdu_resp_msg_data_v01},
  {sizeof(uim_remote_apdu_ind_msg_v01), uim_remote_apdu_ind_msg_data_v01},
  {sizeof(uim_remote_connect_ind_msg_v01), uim_remote_connect_ind_msg_data_v01},
  {sizeof(uim_remote_disconnect_ind_msg_v01), uim_remote_disconnect_ind_msg_data_v01},
  {sizeof(uim_remote_card_power_down_ind_msg_v01), uim_remote_card_power_down_ind_msg_data_v01},
  {sizeof(uim_remote_card_power_up_ind_msg_v01), uim_remote_card_power_up_ind_msg_data_v01},
  {sizeof(uim_remote_card_reset_ind_msg_v01), uim_remote_card_reset_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object uim_remote_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *uim_remote_qmi_idl_type_table_object_referenced_tables_v01[] =
{&uim_remote_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object uim_remote_qmi_idl_type_table_object_v01 = {
  sizeof(uim_remote_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(uim_remote_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  uim_remote_type_table_v01,
  uim_remote_message_table_v01,
  uim_remote_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry uim_remote_service_command_messages_v01[] = {
  {QMI_UIM_REMOTE_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_UIM_REMOTE_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_UIM_REMOTE_RESET_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_UIM_REMOTE_EVENT_REQ_V01, QMI_IDL_TYPE16(0, 2), 58},
  {QMI_UIM_REMOTE_APDU_REQ_V01, QMI_IDL_TYPE16(0, 4), 1059}
};

static const qmi_idl_service_message_table_entry uim_remote_service_response_messages_v01[] = {
  {QMI_UIM_REMOTE_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_UIM_REMOTE_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_UIM_REMOTE_RESET_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_UIM_REMOTE_EVENT_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_UIM_REMOTE_APDU_RESP_V01, QMI_IDL_TYPE16(0, 5), 7}
};

static const qmi_idl_service_message_table_entry uim_remote_service_indication_messages_v01[] = {
  {QMI_UIM_REMOTE_APDU_IND_V01, QMI_IDL_TYPE16(0, 6), 280},
  {QMI_UIM_REMOTE_CONNECT_IND_V01, QMI_IDL_TYPE16(0, 7), 7},
  {QMI_UIM_REMOTE_DISCONNECT_IND_V01, QMI_IDL_TYPE16(0, 8), 7},
  {QMI_UIM_REMOTE_CARD_POWER_UP_IND_V01, QMI_IDL_TYPE16(0, 10), 21},
  {QMI_UIM_REMOTE_CARD_POWER_DOWN_IND_V01, QMI_IDL_TYPE16(0, 9), 14},
  {QMI_UIM_REMOTE_CARD_RESET_IND_V01, QMI_IDL_TYPE16(0, 11), 7}
};

/*Service Object*/
struct qmi_idl_service_object uim_remote_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x32,
  8204,
  { sizeof(uim_remote_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(uim_remote_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(uim_remote_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { uim_remote_service_command_messages_v01, uim_remote_service_response_messages_v01, uim_remote_service_indication_messages_v01},
  &uim_remote_qmi_idl_type_table_object_v01,
  0x02,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type uim_remote_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( UIM_REMOTE_V01_IDL_MAJOR_VERS != idl_maj_version || UIM_REMOTE_V01_IDL_MINOR_VERS != idl_min_version
       || UIM_REMOTE_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&uim_remote_qmi_idl_service_object_v01;
}

