/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        V O I C E _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the voice service Data structures.

 Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.  
 Qualcomm Technologies Proprietary and Confidential.

 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 02.01 
   It was generated on: Fri Oct 15 2010
   From IDL File: */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "voice_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t voice_call_status_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_status_type_v01, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_status_type_v01, call_event),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_call_information_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_information_type_v01, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_information_type_v01, call_state),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_information_type_v01, call_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_information_type_v01, direction),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_information_type_v01, mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_number_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_number_type_v01, pi),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_number_type_v01, number),
  VOICE_NUMBER_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_otasp_status_information_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_otasp_status_information_type_v01, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_otasp_status_information_type_v01, otasp_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_signal_information_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_signal_information_type_v01, signal_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_signal_information_type_v01, alert_pitch),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_signal_information_type_v01, signal),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_caller_id_information_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_caller_id_information_type_v01, pi),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_caller_id_information_type_v01, caller_id),
  VOICE_CALLER_ID_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_burst_dtmf_information_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_information_type_v01, call_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_burst_dtmf_information_type_v01, digit_buffer),
  VOICE_DIGIT_BUFFER_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_cont_dtmf_information_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_cont_dtmf_information_type_v01, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_cont_dtmf_information_type_v01, digit),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_dtmf_information_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dtmf_information_type_v01, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dtmf_information_type_v01, dtmf_event),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_dtmf_information_type_v01, digit_buffer),
  VOICE_DIGIT_BUFFER_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_privacy_information_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_privacy_information_type_v01, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_privacy_information_type_v01, voice_privacy),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t voice_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v01, reg_dtmf_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v01, reg_dtmf_events_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v01, reg_dtmf_events),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v01, reg_voice_privacy_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v01, reg_voice_privacy_events_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v01, reg_voice_privacy_events)
};

static const uint8_t voice_indication_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_indication_register_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_dial_call_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_dial_call_req_msg_v01, calling_number),
  VOICE_NUMBER_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dial_call_req_msg_v01, call_type) - QMI_IDL_OFFSET8(voice_dial_call_req_msg_v01, call_type_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dial_call_req_msg_v01, call_type)
};

static const uint8_t voice_dial_call_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_end_call_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_end_call_req_msg_v01, call_id)
};

static const uint8_t voice_end_call_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_end_call_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_end_call_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_answer_call_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_answer_call_req_msg_v01, call_id)
};

static const uint8_t voice_answer_call_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_answer_call_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_answer_call_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_call_status_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_call_status_ind_msg_v01, call_status),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_call_status_ind_msg_v01, call_end_reason) - QMI_IDL_OFFSET8(voice_call_status_ind_msg_v01, call_end_reason_valid)),
  0x10,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_call_status_ind_msg_v01, call_end_reason),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_call_status_ind_msg_v01, call_type) - QMI_IDL_OFFSET8(voice_call_status_ind_msg_v01, call_type_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_status_ind_msg_v01, call_type)
};

static const uint8_t voice_get_call_info_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_info_req_msg_v01, call_id)
};

static const uint8_t voice_get_call_info_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, call_information),
  1, 0,

  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, number) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, number_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, number),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, srv_opt) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, srv_opt_valid)),
  0x11,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, srv_opt),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, voice_privacy) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, voice_privacy_valid)),
  0x12,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, voice_privacy),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, otasp_status) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, otasp_status_valid)),
  0x13,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v01, otasp_status)
};

static const uint8_t voice_otasp_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_otasp_status_ind_msg_v01, otasp_status_information),
  3, 0
};

static const uint8_t voice_info_rec_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, signal_information) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, signal_information_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, signal_information),
  4, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, caller_id_information) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, caller_id_information_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, caller_id_information),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, display_buffer) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, display_buffer_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, display_buffer),
  VOICE_DISPLAY_BUFFER_MAX_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, ext_display_buffer) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, ext_display_buffer_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, ext_display_buffer),
  VOICE_DISPLAY_BUFFER_MAX_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, caller_name) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, caller_name_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, caller_name),
  VOICE_CALLER_NAME_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, call_waiting) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, call_waiting_valid)),
  0x15,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v01, call_waiting)
};

static const uint8_t voice_send_flash_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_send_flash_req_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_send_flash_req_msg_v01, flash_payload) - QMI_IDL_OFFSET8(voice_send_flash_req_msg_v01, flash_payload_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_send_flash_req_msg_v01, flash_payload),
  ((VOICE_FLASH_PAYLOAD_MAX_V01) & 0xFF), ((VOICE_FLASH_PAYLOAD_MAX_V01) >> 8)
};

static const uint8_t voice_send_flash_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_send_flash_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_send_flash_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_burst_dtmf_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_req_msg_v01, burst_dtmf_information),
  6, 0
};

static const uint8_t voice_burst_dtmf_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_start_cont_dtmf_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_start_cont_dtmf_req_msg_v01, cont_dtmf_information),
  7, 0
};

static const uint8_t voice_start_cont_dtmf_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_start_cont_dtmf_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_start_cont_dtmf_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_stop_cont_dtmf_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_stop_cont_dtmf_req_msg_v01, cont_dtmf_information),
  7, 0
};

static const uint8_t voice_stop_cont_dtmf_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_stop_cont_dtmf_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_stop_cont_dtmf_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_dtmf_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v01, dtmf_information),
  8, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v01, on_length) - QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v01, on_length_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v01, on_length),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v01, off_length) - QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v01, off_length_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v01, off_length)
};

static const uint8_t voice_set_preferred_privacy_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_preferred_privacy_req_msg_v01, privacy_pref)
};

static const uint8_t voice_set_preferred_privacy_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_preferred_privacy_resp_msg_v01, resp),
  0, 1
};

static const uint8_t voice_privacy_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_privacy_ind_msg_v01, voice_privacy_information),
  9, 0
};

/* Type Table */
static const qmi_idl_type_table_entry  voice_type_table_v01[] = {
  {sizeof(voice_call_status_type_v01), voice_call_status_type_data_v01},
  {sizeof(voice_call_information_type_v01), voice_call_information_type_data_v01},
  {sizeof(voice_number_type_v01), voice_number_type_data_v01},
  {sizeof(voice_otasp_status_information_type_v01), voice_otasp_status_information_type_data_v01},
  {sizeof(voice_signal_information_type_v01), voice_signal_information_type_data_v01},
  {sizeof(voice_caller_id_information_type_v01), voice_caller_id_information_type_data_v01},
  {sizeof(voice_burst_dtmf_information_type_v01), voice_burst_dtmf_information_type_data_v01},
  {sizeof(voice_cont_dtmf_information_type_v01), voice_cont_dtmf_information_type_data_v01},
  {sizeof(voice_dtmf_information_type_v01), voice_dtmf_information_type_data_v01},
  {sizeof(voice_privacy_information_type_v01), voice_privacy_information_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry voice_message_table_v01[] = {
  {sizeof(voice_indication_register_req_msg_v01), voice_indication_register_req_msg_data_v01},
  {sizeof(voice_indication_register_resp_msg_v01), voice_indication_register_resp_msg_data_v01},
  {sizeof(voice_dial_call_req_msg_v01), voice_dial_call_req_msg_data_v01},
  {sizeof(voice_dial_call_resp_msg_v01), voice_dial_call_resp_msg_data_v01},
  {sizeof(voice_end_call_req_msg_v01), voice_end_call_req_msg_data_v01},
  {sizeof(voice_end_call_resp_msg_v01), voice_end_call_resp_msg_data_v01},
  {sizeof(voice_answer_call_req_msg_v01), voice_answer_call_req_msg_data_v01},
  {sizeof(voice_answer_call_resp_msg_v01), voice_answer_call_resp_msg_data_v01},
  {sizeof(voice_call_status_ind_msg_v01), voice_call_status_ind_msg_data_v01},
  {sizeof(voice_get_call_info_req_msg_v01), voice_get_call_info_req_msg_data_v01},
  {sizeof(voice_get_call_info_resp_msg_v01), voice_get_call_info_resp_msg_data_v01},
  {sizeof(voice_otasp_status_ind_msg_v01), voice_otasp_status_ind_msg_data_v01},
  {sizeof(voice_info_rec_ind_msg_v01), voice_info_rec_ind_msg_data_v01},
  {sizeof(voice_send_flash_req_msg_v01), voice_send_flash_req_msg_data_v01},
  {sizeof(voice_send_flash_resp_msg_v01), voice_send_flash_resp_msg_data_v01},
  {sizeof(voice_burst_dtmf_req_msg_v01), voice_burst_dtmf_req_msg_data_v01},
  {sizeof(voice_burst_dtmf_resp_msg_v01), voice_burst_dtmf_resp_msg_data_v01},
  {sizeof(voice_start_cont_dtmf_req_msg_v01), voice_start_cont_dtmf_req_msg_data_v01},
  {sizeof(voice_start_cont_dtmf_resp_msg_v01), voice_start_cont_dtmf_resp_msg_data_v01},
  {sizeof(voice_stop_cont_dtmf_req_msg_v01), voice_stop_cont_dtmf_req_msg_data_v01},
  {sizeof(voice_stop_cont_dtmf_resp_msg_v01), voice_stop_cont_dtmf_resp_msg_data_v01},
  {sizeof(voice_dtmf_ind_msg_v01), voice_dtmf_ind_msg_data_v01},
  {sizeof(voice_set_preferred_privacy_req_msg_v01), voice_set_preferred_privacy_req_msg_data_v01},
  {sizeof(voice_set_preferred_privacy_resp_msg_v01), voice_set_preferred_privacy_resp_msg_data_v01},
  {sizeof(voice_privacy_ind_msg_v01), voice_privacy_ind_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object voice_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *voice_qmi_idl_type_table_object_referenced_tables_v01[] =
{&voice_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object voice_qmi_idl_type_table_object_v01 = {
  sizeof(voice_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(voice_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  voice_type_table_v01,
  voice_message_table_v01,
  voice_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry voice_service_command_messages_v01[] = {
  {QMI_VOICE_INDICATION_REGISTER_REQ_V01, TYPE16(0, 0), 8},
  {QMI_VOICE_DIAL_CALL_REQ_V01, TYPE16(0, 2), 262},
  {QMI_VOICE_END_CALL_REQ_V01, TYPE16(0, 4), 4},
  {QMI_VOICE_ANSWER_CALL_REQ_V01, TYPE16(0, 6), 4},
  {QMI_VOICE_GET_CALL_INFO_REQ_V01, TYPE16(0, 9), 4},
  {QMI_VOICE_SEND_FLASH_REQ_V01, TYPE16(0, 13), 4103},
  {QMI_VOICE_BURST_DTMF_REQ_V01, TYPE16(0, 15), 260},
  {QMI_VOICE_START_CONT_DTMF_REQ_V01, TYPE16(0, 17), 5},
  {QMI_VOICE_STOP_CONT_DTMF_REQ_V01, TYPE16(0, 19), 5},
  {QMI_VOICE_SET_PREFERRED_PRIVACY_REQ_V01, TYPE16(0, 22), 4}
};

static const qmi_idl_service_message_table_entry voice_service_response_messages_v01[] = {
  {QMI_VOICE_INDICATION_REGISTER_RESP_V01, TYPE16(0, 1), 7},
  {QMI_VOICE_DIAL_CALL_RESP_V01, TYPE16(0, 3), 11},
  {QMI_VOICE_END_CALL_RESP_V01, TYPE16(0, 5), 11},
  {QMI_VOICE_ANSWER_CALL_RESP_V01, TYPE16(0, 7), 11},
  {QMI_VOICE_GET_CALL_INFO_RESP_V01, TYPE16(0, 10), 288},
  {QMI_VOICE_SEND_FLASH_RESP_V01, TYPE16(0, 14), 11},
  {QMI_VOICE_BURST_DTMF_RESP_V01, TYPE16(0, 16), 11},
  {QMI_VOICE_START_CONT_DTMF_RESP_V01, TYPE16(0, 18), 11},
  {QMI_VOICE_STOP_CONT_DTMF_RESP_V01, TYPE16(0, 20), 11},
  {QMI_VOICE_SET_PREFERRED_PRIVACY_RESP_V01, TYPE16(0, 23), 7}
};

static const qmi_idl_service_message_table_entry voice_service_indication_messages_v01[] = {
  {QMI_VOICE_CALL_STATUS_IND_V01, TYPE16(0, 8), 14},
  {QMI_VOICE_OTASP_STATUS_IND_V01, TYPE16(0, 11), 5},
  {QMI_VOICE_INFO_REC_IND_V01, TYPE16(0, 12), 1048},
  {QMI_VOICE_DTMF_IND_V01, TYPE16(0, 21), 269},
  {QMI_VOICE_PRIVACY_IND_V01, TYPE16(0, 24), 5}
};

/*Service Object*/
const struct qmi_idl_service_object voice_qmi_idl_service_object_v01 = {
  02,
  01,
  0x09,
  0,
  { sizeof(voice_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(voice_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(voice_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { voice_service_command_messages_v01, voice_service_response_messages_v01, voice_service_indication_messages_v01},
  &voice_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type voice_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( VOICE_V01_IDL_MAJOR_VERS != idl_maj_version || VOICE_V01_IDL_MINOR_VERS != idl_min_version 
       || VOICE_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&voice_qmi_idl_service_object_v01;
}

