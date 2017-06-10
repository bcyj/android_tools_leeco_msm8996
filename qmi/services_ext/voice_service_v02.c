/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        V O I C E _ S E R V I C E _ V 0 2  . C

GENERAL DESCRIPTION
  This is the file which defines the voice service Data structures.

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.6 
   It was generated on: Fri Nov 15 2013 (Spin 0)
   From IDL File: voice_service_v02.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "voice_service_v02.h"
#include "common_v01.h"
#include "voice_service_common_v02.h"


/*Type Definitions*/
static const uint8_t voice_uus_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_uus_type_v02, uus_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_uus_type_v02, uus_dcs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_uus_type_v02, uus_data),
  QMI_VOICE_UUS_DATA_MAX_V02,
  QMI_IDL_OFFSET8(voice_uus_type_v02, uus_data) - QMI_IDL_OFFSET8(voice_uus_type_v02, uus_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_cug_type_data_v02[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_cug_type_v02, cug_index),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_cug_type_v02, suppress_pref_cug),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_cug_type_v02, suppress_oa),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_alpha_ident_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_alpha_ident_type_v02, alpha_dcs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_alpha_ident_type_v02, alpha_text),
  QMI_VOICE_ALPHA_TEXT_MAX_V02,
  QMI_IDL_OFFSET8(voice_alpha_ident_type_v02, alpha_text) - QMI_IDL_OFFSET8(voice_alpha_ident_type_v02, alpha_text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_cc_sups_result_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_cc_sups_result_type_v02, service_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_cc_sups_result_type_v02, reason),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_call_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_info_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info_type_v02, call_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info_type_v02, call_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info_type_v02, direction),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info_type_v02, mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_remote_party_number_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_remote_party_number_type_v02, pi),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_remote_party_number_type_v02, number),
  QMI_VOICE_NUMBER_MAX_V02,
  QMI_IDL_OFFSET8(voice_remote_party_number_type_v02, number) - QMI_IDL_OFFSET8(voice_remote_party_number_type_v02, number_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_remote_party_name_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_remote_party_name_type_v02, name_pi),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_remote_party_name_type_v02, coding_scheme),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_remote_party_name_type_v02, caller_name),
  QMI_VOICE_CALLER_NAME_MAX_V02,
  QMI_IDL_OFFSET8(voice_remote_party_name_type_v02, caller_name) - QMI_IDL_OFFSET8(voice_remote_party_name_type_v02, caller_name_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_num_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_info_type_v02, pi),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_info_type_v02, si),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_info_type_v02, num_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_info_type_v02, num_plan),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_num_info_type_v02, num),
  QMI_VOICE_CALLER_ID_MAX_V02,
  QMI_IDL_OFFSET8(voice_num_info_type_v02, num) - QMI_IDL_OFFSET8(voice_num_info_type_v02, num_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_otasp_status_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_otasp_status_info_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_otasp_status_info_type_v02, otasp_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_signal_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_signal_info_type_v02, signal_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_signal_info_type_v02, alert_pitch),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_signal_info_type_v02, signal),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_caller_id_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_caller_id_info_type_v02, pi),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_caller_id_info_type_v02, caller_id),
  QMI_VOICE_CALLER_ID_MAX_V02,
  QMI_IDL_OFFSET8(voice_caller_id_info_type_v02, caller_id) - QMI_IDL_OFFSET8(voice_caller_id_info_type_v02, caller_id_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_redirecting_num_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_redirecting_num_info_type_v02, pi),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_redirecting_num_info_type_v02, si),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_redirecting_num_info_type_v02, num_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_redirecting_num_info_type_v02, num_plan),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_redirecting_num_info_type_v02, reason),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_redirecting_num_info_type_v02, num),
  QMI_VOICE_CALLER_ID_MAX_V02,
  QMI_IDL_OFFSET8(voice_redirecting_num_info_type_v02, num) - QMI_IDL_OFFSET8(voice_redirecting_num_info_type_v02, num_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_nss_audio_control_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_nss_audio_control_info_type_v02, up_link),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_nss_audio_control_info_type_v02, down_link),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_line_control_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_line_control_info_type_v02, polarity_included),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_line_control_info_type_v02, toggle_mode),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_line_control_info_type_v02, reverse_polarity),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_line_control_info_type_v02, power_denial_time),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_burst_dtmf_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_info_type_v02, call_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_info_type_v02, digit_buffer),
  QMI_VOICE_DIGIT_BUFFER_MAX_V02,
  QMI_IDL_OFFSET8(voice_burst_dtmf_info_type_v02, digit_buffer) - QMI_IDL_OFFSET8(voice_burst_dtmf_info_type_v02, digit_buffer_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_dtmf_lengths_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_dtmf_lengths_type_v02, dtmf_onlength),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_dtmf_lengths_type_v02, dtmf_offlength),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_cont_dtmf_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_cont_dtmf_info_type_v02, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_cont_dtmf_info_type_v02, digit),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_dtmf_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dtmf_info_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_dtmf_info_type_v02, dtmf_event),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dtmf_info_type_v02, digit_buffer),
  QMI_VOICE_DIALED_DIGIT_BUFFER_MAX_V02,
  QMI_IDL_OFFSET8(voice_dtmf_info_type_v02, digit_buffer) - QMI_IDL_OFFSET8(voice_dtmf_info_type_v02, digit_buffer_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_privacy_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_privacy_info_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_privacy_info_type_v02, voice_privacy),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_call_info2_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_info2_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info2_type_v02, call_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info2_type_v02, call_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info2_type_v02, direction),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info2_type_v02, mode),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_info2_type_v02, is_mpty),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_info2_type_v02, als),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_remote_party_number2_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_remote_party_number2_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_remote_party_number2_type_v02, number_pi),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_remote_party_number2_type_v02, number),
  QMI_VOICE_NUMBER_MAX_V02,
  QMI_IDL_OFFSET8(voice_remote_party_number2_type_v02, number) - QMI_IDL_OFFSET8(voice_remote_party_number2_type_v02, number_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_remote_party_name2_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_remote_party_name2_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_remote_party_name2_type_v02, name_pi),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_remote_party_name2_type_v02, coding_scheme),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_remote_party_name2_type_v02, name),
  QMI_VOICE_CALLER_NAME_MAX_V02,
  QMI_IDL_OFFSET8(voice_remote_party_name2_type_v02, name) - QMI_IDL_OFFSET8(voice_remote_party_name2_type_v02, name_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_alerting_type_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_alerting_type_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_alerting_type_type_v02, alerting_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_uus_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_uus_info_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_uus_info_type_v02, uus_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_uus_info_type_v02, uus_dcs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_uus_info_type_v02, uus_data),
  QMI_VOICE_UUS_DATA_MAX_V02,
  QMI_IDL_OFFSET8(voice_uus_info_type_v02, uus_data) - QMI_IDL_OFFSET8(voice_uus_info_type_v02, uus_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_srv_opt_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_srv_opt_type_v02, call_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_srv_opt_type_v02, srv_opt),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_call_end_reason_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_end_reason_type_v02, call_id),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_end_reason_type_v02, call_end_reason),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_alpha_ident_with_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_alpha_ident_with_id_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_alpha_ident_with_id_type_v02, alpha_dcs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_alpha_ident_with_id_type_v02, alpha_text),
  QMI_VOICE_ALPHA_TEXT_MAX_V02,
  QMI_IDL_OFFSET8(voice_alpha_ident_with_id_type_v02, alpha_text) - QMI_IDL_OFFSET8(voice_alpha_ident_with_id_type_v02, alpha_text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_conn_num_with_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_conn_num_with_id_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_conn_num_with_id_type_v02, conn_num_pi),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_conn_num_with_id_type_v02, conn_num_si),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_conn_num_with_id_type_v02, conn_num_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_conn_num_with_id_type_v02, conn_num_plan),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_conn_num_with_id_type_v02, conn_num),
  QMI_VOICE_CALLER_ID_MAX_V02,
  QMI_IDL_OFFSET8(voice_conn_num_with_id_type_v02, conn_num) - QMI_IDL_OFFSET8(voice_conn_num_with_id_type_v02, conn_num_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_diagnostic_info_with_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_diagnostic_info_with_id_type_v02, call_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_diagnostic_info_with_id_type_v02, diagnostic_info),
  QMI_VOICE_DIAGNOSTIC_INFO_MAX_V02,
  QMI_IDL_OFFSET8(voice_diagnostic_info_with_id_type_v02, diagnostic_info) - QMI_IDL_OFFSET8(voice_diagnostic_info_with_id_type_v02, diagnostic_info_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_num_with_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_num_with_id_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_with_id_type_v02, num_pi),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_with_id_type_v02, num_si),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_with_id_type_v02, num_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_with_id_type_v02, num_plan),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_num_with_id_type_v02, num),
  QMI_VOICE_NUMBER_MAX_V02,
  QMI_IDL_OFFSET8(voice_num_with_id_type_v02, num) - QMI_IDL_OFFSET8(voice_num_with_id_type_v02, num_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_notification_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_notification_info_type_v02, call_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_notification_info_type_v02, notification_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_ect_number_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_ect_number_type_v02, ect_call_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_ect_number_type_v02, pi),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_ect_number_type_v02, number),
  QMI_VOICE_NUMBER_MAX_V02,
  QMI_IDL_OFFSET8(voice_ect_number_type_v02, number) - QMI_IDL_OFFSET8(voice_ect_number_type_v02, number_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_supplementary_service_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_supplementary_service_info_type_v02, voice_service),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_supplementary_service_info_type_v02, reason),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_num_type_plan_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_type_plan_type_v02, num_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_num_type_plan_type_v02, num_plan),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_clip_response_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_clip_response_type_v02, active_status),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_clip_response_type_v02, provision_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_clir_response_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_clir_response_type_v02, active_status),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_clir_response_type_v02, provision_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_get_call_forwarding_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_type_v02, service_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_type_v02, service_class),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_type_v02, number),
  QMI_VOICE_NUMBER_MAX_V02,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_type_v02, number) - QMI_IDL_OFFSET8(voice_get_call_forwarding_info_type_v02, number_len),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_type_v02, no_reply_timer),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_get_call_forwarding_info_exten_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_exten_type_v02, service_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_exten_type_v02, service_class),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_exten_type_v02, no_reply_timer),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_exten_type_v02, cfw_num_info),
  QMI_IDL_TYPE88(0, 7),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_call_barring_password_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_call_barring_password_info_type_v02, reason),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_barring_password_info_type_v02, old_password),
  4,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_barring_password_info_type_v02, new_password),
  4,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_barring_password_info_type_v02, new_password_again),
  4,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_uss_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_uss_info_type_v02, uss_dcs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_uss_info_type_v02, uss_data),
  QMI_VOICE_USS_DATA_MAX_V02,
  QMI_IDL_OFFSET8(voice_uss_info_type_v02, uss_data) - QMI_IDL_OFFSET8(voice_uss_info_type_v02, uss_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_air_timer_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_air_timer_type_v02, nam_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_air_timer_type_v02, air_timer),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_roam_timer_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_roam_timer_type_v02, nam_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_roam_timer_type_v02, roam_timer),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_preferred_voice_so_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_preferred_voice_so_type_v02, nam_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_preferred_voice_so_type_v02, evrc_capability),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_preferred_voice_so_type_v02, home_page_voice_so),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_preferred_voice_so_type_v02, home_orig_voice_so),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_preferred_voice_so_type_v02, roam_orig_voice_so),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_arm_config_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_arm_config_type_v02, gsm_amr_status),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_arm_config_type_v02, wcdma_amr_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_supp_service_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_supp_service_info_type_v02, service_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_supp_service_info_type_v02, is_modified_by_call_control),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_new_password_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_new_password_type_v02, new_password),
  4,

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_new_password_type_v02, new_password_again),
  4,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_ss_status_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_ss_status_type_v02, active_status),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_ss_status_type_v02, provision_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_subaddress_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_subaddress_type_v02, extension_bit),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_subaddress_type_v02, subaddress_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_subaddress_type_v02, odd_even_ind),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_subaddress_type_v02, subaddress),
  QMI_VOICE_SUBADDRESS_LEN_MAX_V02,
  QMI_IDL_OFFSET8(voice_subaddress_type_v02, subaddress) - QMI_IDL_OFFSET8(voice_subaddress_type_v02, subaddress_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_alerting_pattern_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_alerting_pattern_type_v02, call_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_alerting_pattern_type_v02, alerting_pattern),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_ext_display_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_ext_display_info_type_v02, display_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_ext_display_info_type_v02, ext_display_info),
  QMI_VOICE_EXT_DISPLAY_RECORD_LEN_MAX_V02,
  QMI_IDL_OFFSET8(voice_ext_display_info_type_v02, ext_display_info) - QMI_IDL_OFFSET8(voice_ext_display_info_type_v02, ext_display_info_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_call_attributes_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_attributes_type_v02, call_id),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_call_attributes_type_v02, call_attributes),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_get_call_forwarding_info_exten2_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_exten2_type_v02, service_status),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_exten2_type_v02, service_class_ext),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_exten2_type_v02, no_reply_timer),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_info_exten2_type_v02, cfw_num_info),
  QMI_IDL_TYPE88(0, 7),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_usr_uri_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_usr_uri_type_v02, uri_name),
  QMI_VOICE_CONF_URI_MAX_LEN_V02,
  QMI_IDL_OFFSET8(voice_usr_uri_type_v02, uri_name) - QMI_IDL_OFFSET8(voice_usr_uri_type_v02, uri_name_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_usr_uri_type_v02, uri_description),
  QMI_VOICE_CONF_DISPLAY_TEXT_MAX_LEN_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_usr_uri_type_v02, uri_description) - QMI_IDL_OFFSET16RELATIVE(voice_usr_uri_type_v02, uri_description_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_ext_brst_intl_type_data_v02[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_ext_brst_intl_type_v02, mcc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_ext_brst_intl_type_v02, db_subtype),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_ext_brst_intl_type_v02, chg_ind),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_ext_brst_intl_type_v02, sub_unit),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_ext_brst_intl_type_v02, unit),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_videoshare_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_videoshare_type_v02, vs_variant),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_videoshare_type_v02, file_attributes),
  ((QMI_VOICE_VS_FILE_ATTRIBUTES_MAX_V02) & 0xFF), ((QMI_VOICE_VS_FILE_ATTRIBUTES_MAX_V02) >> 8),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_vs_variant_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_vs_variant_type_v02, call_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_vs_variant_type_v02, vs_variant),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_sip_uri_with_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_sip_uri_with_id_type_v02, call_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_sip_uri_with_id_type_v02, sip_uri),
  QMI_VOICE_SIP_URI_MAX_V02,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_is_srvcc_call_with_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_is_srvcc_call_with_id_type_v02, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_is_srvcc_call_with_id_type_v02, is_srvcc_call),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_srvcc_parent_call_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_srvcc_parent_call_id_type_v02, call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_srvcc_parent_call_id_type_v02, parent_call_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_srvcc_parent_call_id_type_v02, is_parent_id_cleared),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_conf_participant_call_info_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_conf_participant_call_info_type_v02, user_uri),
  QMI_VOICE_CONF_URI_MAX_LEN_V02,
  QMI_IDL_OFFSET8(voice_conf_participant_call_info_type_v02, user_uri) - QMI_IDL_OFFSET8(voice_conf_participant_call_info_type_v02, user_uri_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_conf_participant_call_info_type_v02, status),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_conf_participant_call_info_type_v02, audio_attributes),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_conf_participant_call_info_type_v02, video_attributes),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_conf_participant_call_info_type_v02, disconnection_method),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_conf_participant_call_info_type_v02, disconnection_info),
  QMI_VOICE_CONF_DISPLAY_TEXT_MAX_LEN_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_conf_participant_call_info_type_v02, disconnection_info) - QMI_IDL_OFFSET16RELATIVE(voice_conf_participant_call_info_type_v02, disconnection_info_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_conference_call_info_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_conference_call_info_type_v02, update_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_conference_call_info_type_v02, conf_participant_info),
  QMI_VOICE_CONF_PARTICIPANT_INFO_ARRAY_MAX_V02,
  QMI_IDL_OFFSET8(voice_conference_call_info_type_v02, conf_participant_info) - QMI_IDL_OFFSET8(voice_conference_call_info_type_v02, conf_participant_info_len),
  QMI_IDL_TYPE88(0, 59),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_ip_call_capabilities_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_ip_call_capabilities_info_type_v02, call_id),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_ip_call_capabilities_info_type_v02, audio_attrib),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_ip_call_capabilities_info_type_v02, audio_cause),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_ip_call_capabilities_info_type_v02, video_attrib),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_ip_call_capabilities_info_type_v02, video_cause),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_child_number_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_child_number_info_type_v02, call_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_child_number_info_type_v02, number),
  QMI_VOICE_SIP_URI_MAX_V02,
  QMI_IDL_OFFSET8(voice_child_number_info_type_v02, number) - QMI_IDL_OFFSET8(voice_child_number_info_type_v02, number_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t voice_display_text_info_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_display_text_info_type_v02, call_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_display_text_info_type_v02, display_text),
  QMI_VOICE_DISPLAY_TEXT_MAX_LEN_V02,
  QMI_IDL_OFFSET8(voice_display_text_info_type_v02, display_text) - QMI_IDL_OFFSET8(voice_display_text_info_type_v02, display_text_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t voice_indication_register_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, reg_dtmf_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, reg_dtmf_events_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, reg_dtmf_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, reg_voice_privacy_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, reg_voice_privacy_events_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, reg_voice_privacy_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, supps_notification_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, supps_notification_events_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, supps_notification_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, call_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, call_events_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, call_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, handover_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, handover_events_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, handover_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, speech_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, speech_events_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, speech_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, ussd_notification_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, ussd_notification_events_valid)),
  0x16,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, ussd_notification_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, sups_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, sups_events_valid)),
  0x17,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, sups_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, modification_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, modification_events_valid)),
  0x18,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, modification_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, uus_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, uus_events_valid)),
  0x19,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, uus_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, aoc_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, aoc_events_valid)),
  0x1A,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, aoc_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, conference_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, conference_events_valid)),
  0x1B,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, conference_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, ext_brst_intl_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, ext_brst_intl_events_valid)),
  0x1C,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, ext_brst_intl_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, page_miss_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, page_miss_events_valid)),
  0x1D,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, page_miss_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, cc_result_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, cc_result_events_valid)),
  0x1E,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, cc_result_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, conf_participants_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, conf_participants_events_valid)),
  0x1F,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, conf_participants_events),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, tty_info_events) - QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, tty_info_events_valid)),
  0x20,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_indication_register_req_msg_v02, tty_info_events)
};

static const uint8_t voice_indication_register_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_indication_register_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t voice_dial_call_req_msg_data_v02[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, calling_number),
  QMI_VOICE_NUMBER_MAX_V02,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, call_type) - QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, call_type_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, call_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, clir_type) - QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, clir_type_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, clir_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, uus) - QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, uus_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_dial_call_req_msg_v02, uus),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, cug) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, cug_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, cug),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, emer_cat) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, emer_cat_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, emer_cat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, called_party_subaddress) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, called_party_subaddress_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, called_party_subaddress),
  QMI_IDL_TYPE88(0, 47),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, service_type) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, service_type_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, service_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, sip_uri_overflow) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, sip_uri_overflow_valid)),
  0x17,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, sip_uri_overflow),
  QMI_VOICE_SIP_URI_OVERFLOW_MAX_V02,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, audio_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, audio_attrib_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, audio_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, video_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, video_attrib_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, video_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, pi) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, pi_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, pi),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, videoshare_call_attribs) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, videoshare_call_attribs_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, videoshare_call_attribs),
  QMI_IDL_TYPE88(0, 54),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, ecall_variant) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, ecall_variant_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, ecall_variant),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, conf_uri_list) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, conf_uri_list_valid)),
  0x1D,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, conf_uri_list),
  ((QMI_VOICE_CONF_URI_LIST_MAX_LEN_V02) & 0xFF), ((QMI_VOICE_CONF_URI_LIST_MAX_LEN_V02) >> 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, display_text) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, display_text_valid)),
  0x1E,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_req_msg_v02, display_text),
  QMI_VOICE_DISPLAY_TEXT_MAX_LEN_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, display_text) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_req_msg_v02, display_text_len)
};

static const uint8_t voice_dial_call_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v02, alpha_ident) - QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v02, alpha_ident_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_dial_call_resp_msg_v02, alpha_ident),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_resp_msg_v02, cc_result_type_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_resp_msg_v02, cc_sups_result_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_dial_call_resp_msg_v02, end_reason) - QMI_IDL_OFFSET16RELATIVE(voice_dial_call_resp_msg_v02, end_reason_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_dial_call_resp_msg_v02, end_reason)
};

static const uint8_t voice_end_call_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_end_call_req_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_end_call_req_msg_v02, end_cause) - QMI_IDL_OFFSET8(voice_end_call_req_msg_v02, end_cause_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_end_call_req_msg_v02, end_cause)
};

static const uint8_t voice_end_call_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_end_call_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_end_call_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_end_call_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_end_call_resp_msg_v02, call_id)
};

static const uint8_t voice_answer_call_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, call_type) - QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, call_type_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, call_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, audio_attrib) - QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, audio_attrib_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, audio_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, video_attrib) - QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, video_attrib_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, video_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, pi) - QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, pi_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, pi),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, file_attributes) - QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, file_attributes_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_answer_call_req_msg_v02, file_attributes),
  ((QMI_VOICE_VS_FILE_ATTRIBUTES_MAX_V02) & 0xFF), ((QMI_VOICE_VS_FILE_ATTRIBUTES_MAX_V02) >> 8),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_answer_call_req_msg_v02, reject_call) - QMI_IDL_OFFSET16RELATIVE(voice_answer_call_req_msg_v02, reject_call_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_answer_call_req_msg_v02, reject_call),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_answer_call_req_msg_v02, reject_cause) - QMI_IDL_OFFSET16RELATIVE(voice_answer_call_req_msg_v02, reject_cause_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_answer_call_req_msg_v02, reject_cause)
};

static const uint8_t voice_answer_call_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_answer_call_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_answer_call_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_answer_call_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_answer_call_resp_msg_v02, call_id)
};

static const uint8_t voice_get_call_info_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_info_req_msg_v02, call_id)
};

static const uint8_t voice_get_call_info_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, call_info) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, call_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, call_info),
  QMI_IDL_TYPE88(0, 4),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, remote_party_number) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, remote_party_number_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, remote_party_number),
  QMI_IDL_TYPE88(0, 5),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, srv_opt) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, srv_opt_valid)),
  0x12,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, srv_opt),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, voice_privacy) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, voice_privacy_valid)),
  0x13,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, voice_privacy),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, otasp_status) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, otasp_status_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, otasp_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, remote_party_name) - QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, remote_party_name_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_info_resp_msg_v02, remote_party_name),
  QMI_IDL_TYPE88(0, 6),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, uus) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, uus_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, uus),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, alerting_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, alerting_type_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, alerting_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, alpha_ident) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, alpha_ident_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, alpha_ident),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, conn_num_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, conn_num_info_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, conn_num_info),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, diagnostic_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, diagnostic_info_valid)),
  0x1A,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, diagnostic_info),
  QMI_VOICE_DIAGNOSTIC_INFO_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, diagnostic_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, diagnostic_info_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, alerting_pattern) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, alerting_pattern_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, alerting_pattern),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, audio_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, audio_attrib_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, audio_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, video_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, video_attrib_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, video_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, vs_variant) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, vs_variant_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, vs_variant),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, sip_uri) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, sip_uri_valid)),
  0x1F,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, sip_uri),
  QMI_VOICE_SIP_URI_MAX_V02,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, is_srvcc_call) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_info_resp_msg_v02, is_srvcc_call_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_info_resp_msg_v02, is_srvcc_call)
};

static const uint8_t voice_otasp_status_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_otasp_status_ind_msg_v02, otasp_status_info),
  QMI_IDL_TYPE88(0, 8)
};

static const uint8_t voice_info_rec_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, signal_info) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, signal_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, signal_info),
  QMI_IDL_TYPE88(0, 9),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, caller_id_info) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, caller_id_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, caller_id_info),
  QMI_IDL_TYPE88(0, 10),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, display_buffer) - QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, display_buffer_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_info_rec_ind_msg_v02, display_buffer),
  QMI_VOICE_DISPLAY_BUFFER_MAX_V02,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, ext_display_buffer) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, ext_display_buffer_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, ext_display_buffer),
  QMI_VOICE_DISPLAY_BUFFER_MAX_V02,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, caller_name) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, caller_name_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, caller_name),
  QMI_VOICE_CALLER_NAME_MAX_V02,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, call_waiting) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, call_waiting_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, call_waiting),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, conn_num_info) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, conn_num_info_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, conn_num_info),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, calling_party_info) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, calling_party_info_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, calling_party_info),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, called_party_info) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, called_party_info_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, called_party_info),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, redirecting_num_info) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, redirecting_num_info_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, redirecting_num_info),
  QMI_IDL_TYPE88(0, 11),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, clir_cause) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, clir_cause_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, clir_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, audio_control) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, audio_control_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, audio_control),
  QMI_IDL_TYPE88(0, 12),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, nss_release) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, nss_release_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, nss_release),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, line_control) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, line_control_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, line_control),
  QMI_IDL_TYPE88(0, 13),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, ext_display_record) - QMI_IDL_OFFSET16RELATIVE(voice_info_rec_ind_msg_v02, ext_display_record_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_info_rec_ind_msg_v02, ext_display_record),
  QMI_IDL_TYPE88(0, 49)
};

static const uint8_t voice_send_flash_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_send_flash_req_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_send_flash_req_msg_v02, flash_payload) - QMI_IDL_OFFSET8(voice_send_flash_req_msg_v02, flash_payload_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_send_flash_req_msg_v02, flash_payload),
  QMI_VOICE_FLASH_PAYLOAD_MAX_V02,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_send_flash_req_msg_v02, flash_type) - QMI_IDL_OFFSET8(voice_send_flash_req_msg_v02, flash_type_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_send_flash_req_msg_v02, flash_type)
};

static const uint8_t voice_send_flash_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_send_flash_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_send_flash_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_send_flash_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_send_flash_resp_msg_v02, call_id)
};

static const uint8_t voice_burst_dtmf_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_req_msg_v02, burst_dtmf_info),
  QMI_IDL_TYPE88(0, 14),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_burst_dtmf_req_msg_v02, dtmf_lengths) - QMI_IDL_OFFSET8(voice_burst_dtmf_req_msg_v02, dtmf_lengths_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_req_msg_v02, dtmf_lengths),
  QMI_IDL_TYPE88(0, 15)
};

static const uint8_t voice_burst_dtmf_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_burst_dtmf_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_burst_dtmf_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_burst_dtmf_resp_msg_v02, call_id)
};

static const uint8_t voice_start_cont_dtmf_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_start_cont_dtmf_req_msg_v02, cont_dtmf_info),
  QMI_IDL_TYPE88(0, 16)
};

static const uint8_t voice_start_cont_dtmf_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_start_cont_dtmf_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_start_cont_dtmf_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_start_cont_dtmf_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_start_cont_dtmf_resp_msg_v02, call_id)
};

static const uint8_t voice_stop_cont_dtmf_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_stop_cont_dtmf_req_msg_v02, call_id)
};

static const uint8_t voice_stop_cont_dtmf_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_stop_cont_dtmf_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_stop_cont_dtmf_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_stop_cont_dtmf_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_stop_cont_dtmf_resp_msg_v02, call_id)
};

static const uint8_t voice_dtmf_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v02, dtmf_info),
  QMI_IDL_TYPE88(0, 17),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v02, on_length) - QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v02, on_length_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v02, on_length),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v02, off_length) - QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v02, off_length_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_dtmf_ind_msg_v02, off_length)
};

static const uint8_t voice_set_preferred_privacy_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_set_preferred_privacy_req_msg_v02, privacy_pref)
};

static const uint8_t voice_set_preferred_privacy_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_preferred_privacy_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t voice_privacy_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_privacy_ind_msg_v02, voice_privacy_info),
  QMI_IDL_TYPE88(0, 18)
};

static const uint8_t voice_all_call_status_ind_msg_data_v02[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_all_call_status_ind_msg_v02, call_info),
  QMI_VOICE_CALL_INFO_MAX_V02,
  QMI_IDL_OFFSET8(voice_all_call_status_ind_msg_v02, call_info) - QMI_IDL_OFFSET8(voice_all_call_status_ind_msg_v02, call_info_len),
  QMI_IDL_TYPE88(0, 19),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, remote_party_number) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, remote_party_number_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, remote_party_number),
  QMI_VOICE_REMOTE_PARTY_NUMBER_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, remote_party_number) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, remote_party_number_len),
  QMI_IDL_TYPE88(0, 20),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, remote_party_name) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, remote_party_name_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, remote_party_name),
  QMI_VOICE_REMOTE_PARTY_NAME_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, remote_party_name) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, remote_party_name_len),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alerting_type) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alerting_type_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, alerting_type),
  QMI_VOICE_ALERTING_TYPE_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alerting_type) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alerting_type_len),
  QMI_IDL_TYPE88(0, 22),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, srv_opt) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, srv_opt_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, srv_opt),
  QMI_VOICE_SRV_OPT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, srv_opt) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, srv_opt_len),
  QMI_IDL_TYPE88(0, 24),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, call_end_reason) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, call_end_reason_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, call_end_reason),
  QMI_VOICE_CALL_END_REASON_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, call_end_reason) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, call_end_reason_len),
  QMI_IDL_TYPE88(0, 25),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alpha_id) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alpha_id_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, alpha_id),
  QMI_VOICE_ALPHA_IDENT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alpha_id) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alpha_id_len),
  QMI_IDL_TYPE88(0, 26),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, conn_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, conn_party_num_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, conn_party_num),
  QMI_VOICE_CONNECTED_PARTY_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, conn_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, conn_party_num_len),
  QMI_IDL_TYPE88(0, 27),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, diagnostic_info) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, diagnostic_info_valid)),
  0x17,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, diagnostic_info),
  QMI_VOICE_DIAGNOSTIC_INFO_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, diagnostic_info) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, diagnostic_info_len),
  QMI_IDL_TYPE88(0, 28),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, called_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, called_party_num_valid)),
  0x18,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, called_party_num),
  QMI_VOICE_CALLED_PARTY_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, called_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, called_party_num_len),
  QMI_IDL_TYPE88(0, 29),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, redirecting_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, redirecting_party_num_valid)),
  0x19,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, redirecting_party_num),
  QMI_VOICE_REDIRECTING_PARTY_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, redirecting_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, redirecting_party_num_len),
  QMI_IDL_TYPE88(0, 29),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alerting_pattern) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alerting_pattern_valid)),
  0x1A,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, alerting_pattern),
  QMI_VOICE_ALERTING_PATTERN_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alerting_pattern) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, alerting_pattern_len),
  QMI_IDL_TYPE88(0, 48),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, audio_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, audio_attrib_valid)),
  0x1B,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, audio_attrib),
  QMI_VOICE_CALL_ATTRIBUTES_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, audio_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, audio_attrib_len),
  QMI_IDL_TYPE88(0, 50),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, video_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, video_attrib_valid)),
  0x1C,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, video_attrib),
  QMI_VOICE_CALL_ATTRIBUTES_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, video_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, video_attrib_len),
  QMI_IDL_TYPE88(0, 50),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, vs_variant) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, vs_variant_valid)),
  0x1D,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, vs_variant),
  QMI_VOICE_VS_CALL_VARIANT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, vs_variant) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, vs_variant_len),
  QMI_IDL_TYPE88(0, 55),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, sip_uri) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, sip_uri_valid)),
  0x1E,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, sip_uri),
  QMI_VOICE_VS_CALL_VARIANT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, sip_uri) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, sip_uri_len),
  QMI_IDL_TYPE88(0, 56),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, is_srvcc) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, is_srvcc_valid)),
  0x1F,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, is_srvcc),
  QMI_VOICE_IS_SRVCC_CALL_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, is_srvcc) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, is_srvcc_len),
  QMI_IDL_TYPE88(0, 57),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, srvcc_parent_call_info) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, srvcc_parent_call_info_valid)),
  0x20,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, srvcc_parent_call_info),
  QMI_VOICE_SRVCC_PARENT_CALL_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, srvcc_parent_call_info) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, srvcc_parent_call_info_len),
  QMI_IDL_TYPE88(0, 58),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, local_call_capabilities_info) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, local_call_capabilities_info_valid)),
  0x21,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, local_call_capabilities_info),
  QMI_VOICE_CALL_CAPABILITIES_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, local_call_capabilities_info) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, local_call_capabilities_info_len),
  QMI_IDL_TYPE88(0, 61),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, peer_call_capabilities_info) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, peer_call_capabilities_info_valid)),
  0x22,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, peer_call_capabilities_info),
  QMI_VOICE_CALL_CAPABILITIES_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, peer_call_capabilities_info) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, peer_call_capabilities_info_len),
  QMI_IDL_TYPE88(0, 61),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, child_number) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, child_number_valid)),
  0x23,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, child_number),
  QMI_VOICE_CHILD_NUMBER_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, child_number) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, child_number_len),
  QMI_IDL_TYPE88(0, 62),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, display_text) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, display_text_valid)),
  0x24,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_all_call_status_ind_msg_v02, display_text),
  QMI_VOICE_DISPLAY_TEXT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, display_text) - QMI_IDL_OFFSET16RELATIVE(voice_all_call_status_ind_msg_v02, display_text_len),
  QMI_IDL_TYPE88(0, 63)
};

/* 
 * voice_get_all_call_info_req_msg is empty
 * static const uint8_t voice_get_all_call_info_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_get_all_call_info_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_all_call_info_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_all_call_info_resp_msg_v02, call_info) - QMI_IDL_OFFSET8(voice_get_all_call_info_resp_msg_v02, call_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_all_call_info_resp_msg_v02, call_info),
  QMI_VOICE_CALL_INFO_MAX_V02,
  QMI_IDL_OFFSET8(voice_get_all_call_info_resp_msg_v02, call_info) - QMI_IDL_OFFSET8(voice_get_all_call_info_resp_msg_v02, call_info_len),
  QMI_IDL_TYPE88(0, 19),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, remote_party_number) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, remote_party_number_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, remote_party_number),
  QMI_VOICE_REMOTE_PARTY_NUMBER_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, remote_party_number) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, remote_party_number_len),
  QMI_IDL_TYPE88(0, 20),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, remote_party_name) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, remote_party_name_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, remote_party_name),
  QMI_VOICE_REMOTE_PARTY_NAME_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, remote_party_name) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, remote_party_name_len),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alerting_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alerting_type_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, alerting_type),
  QMI_VOICE_ALERTING_TYPE_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alerting_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alerting_type_len),
  QMI_IDL_TYPE88(0, 22),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, uus_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, uus_info_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, uus_info),
  QMI_VOICE_UUS_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, uus_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, uus_info_len),
  QMI_IDL_TYPE88(0, 23),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, srv_opt) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, srv_opt_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, srv_opt),
  QMI_VOICE_SRV_OPT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, srv_opt) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, srv_opt_len),
  QMI_IDL_TYPE88(0, 24),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, otasp_status) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, otasp_status_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, otasp_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, voice_privacy) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, voice_privacy_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, voice_privacy),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, call_end_reason) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, call_end_reason_valid)),
  0x18,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, call_end_reason),
  QMI_VOICE_CALL_END_REASON_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, call_end_reason) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, call_end_reason_len),
  QMI_IDL_TYPE88(0, 25),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alpha_id_valid)),
  0x19,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, alpha_id),
  QMI_VOICE_ALPHA_IDENT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alpha_id_len),
  QMI_IDL_TYPE88(0, 26),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, conn_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, conn_party_num_valid)),
  0x1A,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, conn_party_num),
  QMI_VOICE_CONNECTED_PARTY_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, conn_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, conn_party_num_len),
  QMI_IDL_TYPE88(0, 27),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, diagnostic_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, diagnostic_info_valid)),
  0x1B,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, diagnostic_info),
  QMI_VOICE_DIAGNOSTIC_INFO_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, diagnostic_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, diagnostic_info_len),
  QMI_IDL_TYPE88(0, 28),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, called_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, called_party_num_valid)),
  0x1C,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, called_party_num),
  QMI_VOICE_CALLED_PARTY_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, called_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, called_party_num_len),
  QMI_IDL_TYPE88(0, 29),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, redirecting_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, redirecting_party_num_valid)),
  0x1D,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, redirecting_party_num),
  QMI_VOICE_REDIRECTING_PARTY_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, redirecting_party_num) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, redirecting_party_num_len),
  QMI_IDL_TYPE88(0, 29),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alerting_pattern) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alerting_pattern_valid)),
  0x1E,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, alerting_pattern),
  QMI_VOICE_ALERTING_PATTERN_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alerting_pattern) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, alerting_pattern_len),
  QMI_IDL_TYPE88(0, 48),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, audio_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, audio_attrib_valid)),
  0x1F,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, audio_attrib),
  QMI_VOICE_CALL_ATTRIBUTES_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, audio_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, audio_attrib_len),
  QMI_IDL_TYPE88(0, 50),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, video_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, video_attrib_valid)),
  0x20,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, video_attrib),
  QMI_VOICE_CALL_ATTRIBUTES_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, video_attrib) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, video_attrib_len),
  QMI_IDL_TYPE88(0, 50),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, vs_variant) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, vs_variant_valid)),
  0x21,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, vs_variant),
  QMI_VOICE_VS_CALL_VARIANT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, vs_variant) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, vs_variant_len),
  QMI_IDL_TYPE88(0, 55),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, sip_uri) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, sip_uri_valid)),
  0x22,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, sip_uri),
  QMI_VOICE_VS_CALL_VARIANT_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, sip_uri) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, sip_uri_len),
  QMI_IDL_TYPE88(0, 56),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, is_srvcc) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, is_srvcc_valid)),
  0x23,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_all_call_info_resp_msg_v02, is_srvcc),
  QMI_VOICE_IS_SRVCC_CALL_ARRAY_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, is_srvcc) - QMI_IDL_OFFSET16RELATIVE(voice_get_all_call_info_resp_msg_v02, is_srvcc_len),
  QMI_IDL_TYPE88(0, 57)
};

static const uint8_t voice_manage_calls_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_manage_calls_req_msg_v02, sups_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_calls_req_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_manage_calls_req_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_manage_calls_req_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_calls_req_msg_v02, reject_cause) - QMI_IDL_OFFSET8(voice_manage_calls_req_msg_v02, reject_cause_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_manage_calls_req_msg_v02, reject_cause)
};

static const uint8_t voice_manage_calls_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_manage_calls_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_calls_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_manage_calls_resp_msg_v02, failure_cause_valid)),
  0x10,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_manage_calls_resp_msg_v02, failure_cause)
};

static const uint8_t voice_sups_notification_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, notification_info),
  QMI_IDL_TYPE88(0, 30),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, index) - QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, index_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, index),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, ect_number) - QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, ect_number_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, ect_number),
  QMI_IDL_TYPE88(0, 31),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, ss_code) - QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, ss_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_sups_notification_ind_msg_v02, ss_code)
};

static const uint8_t voice_set_sups_service_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, supplementary_service_info),
  QMI_IDL_TYPE88(0, 32),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, service_class) - QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, service_class_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, service_class),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, password) - QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, password_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, password),
  4,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, number) - QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, number_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, number),
  QMI_VOICE_NUMBER_MAX_V02,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, timer_value) - QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, timer_value_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, timer_value),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, num_type_plan) - QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, num_type_plan_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, num_type_plan),
  QMI_IDL_TYPE88(0, 33),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, service_class_ext) - QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, service_class_ext_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_set_sups_service_req_msg_v02, service_class_ext)
};

static const uint8_t voice_set_sups_service_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_sups_service_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_sups_service_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_set_sups_service_resp_msg_v02, failure_cause_valid)),
  0x10,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_set_sups_service_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_sups_service_resp_msg_v02, alpha_ident) - QMI_IDL_OFFSET8(voice_set_sups_service_resp_msg_v02, alpha_ident_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_sups_service_resp_msg_v02, alpha_ident),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, cc_result_type_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_set_sups_service_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, call_id_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_set_sups_service_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, cc_sups_result_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_set_sups_service_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, service_status) - QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, service_status_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_set_sups_service_resp_msg_v02, service_status),
  QMI_IDL_TYPE88(0, 46),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, failure_cause_description) - QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, failure_cause_description_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_set_sups_service_resp_msg_v02, failure_cause_description),
  ((QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02) & 0xFF), ((QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02) >> 8),
  QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, failure_cause_description) - QMI_IDL_OFFSET16RELATIVE(voice_set_sups_service_resp_msg_v02, failure_cause_description_len)
};

static const uint8_t voice_get_call_waiting_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_waiting_req_msg_v02, service_class) - QMI_IDL_OFFSET8(voice_get_call_waiting_req_msg_v02, service_class_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_waiting_req_msg_v02, service_class),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_waiting_req_msg_v02, service_class_ext) - QMI_IDL_OFFSET8(voice_get_call_waiting_req_msg_v02, service_class_ext_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_waiting_req_msg_v02, service_class_ext)
};

static const uint8_t voice_get_call_waiting_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, service_class) - QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, service_class_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, service_class),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, alpha_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_waiting_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_waiting_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_waiting_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_waiting_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_waiting_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_waiting_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_waiting_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_waiting_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_waiting_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_waiting_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_waiting_resp_msg_v02, service_class_ext) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_waiting_resp_msg_v02, service_class_ext_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_waiting_resp_msg_v02, service_class_ext)
};

static const uint8_t voice_get_call_barring_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_barring_req_msg_v02, reason),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_barring_req_msg_v02, service_class) - QMI_IDL_OFFSET8(voice_get_call_barring_req_msg_v02, service_class_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_barring_req_msg_v02, service_class),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_barring_req_msg_v02, service_class_ext) - QMI_IDL_OFFSET8(voice_get_call_barring_req_msg_v02, service_class_ext_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_barring_req_msg_v02, service_class_ext)
};

static const uint8_t voice_get_call_barring_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, service_class) - QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, service_class_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, service_class),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, alpha_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_barring_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_barring_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_barring_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_barring_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_barring_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_barring_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_barring_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_barring_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_barring_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_barring_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_barring_resp_msg_v02, service_class_ext) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_barring_resp_msg_v02, service_class_ext_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_barring_resp_msg_v02, service_class_ext)
};

/* 
 * voice_get_clip_req_msg is empty
 * static const uint8_t voice_get_clip_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_get_clip_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, clip_response) - QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, clip_response_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, clip_response),
  QMI_IDL_TYPE88(0, 34),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, alpha_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_clip_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_clip_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_clip_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_clip_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_clip_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_clip_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_clip_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_clip_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_get_clip_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_clip_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3)
};

/* 
 * voice_get_clir_req_msg is empty
 * static const uint8_t voice_get_clir_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_get_clir_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, clir_response) - QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, clir_response_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, clir_response),
  QMI_IDL_TYPE88(0, 35),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, alpha_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_clir_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_clir_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_clir_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_clir_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_clir_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_clir_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_clir_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_clir_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_get_clir_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_clir_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t voice_get_call_forwarding_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_req_msg_v02, reason),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_forwarding_req_msg_v02, service_class) - QMI_IDL_OFFSET8(voice_get_call_forwarding_req_msg_v02, service_class_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_req_msg_v02, service_class),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_forwarding_req_msg_v02, service_class_ext) - QMI_IDL_OFFSET8(voice_get_call_forwarding_req_msg_v02, service_class_ext_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_req_msg_v02, service_class_ext)
};

static const uint8_t voice_get_call_forwarding_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_info) - QMI_IDL_OFFSET8(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_info),
  GET_CALL_FORWARDING_INFO_MAX_V02,
  QMI_IDL_OFFSET8(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_info) - QMI_IDL_OFFSET8(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_info_len),
  QMI_IDL_TYPE88(0, 36),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_forwarding_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, alpha_id_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_forwarding_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_forwarding_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_forwarding_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_forwarding_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten_info_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten_info),
  GET_CALL_FORWARDING_INFO_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten_info_len),
  QMI_IDL_TYPE88(0, 37),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten2_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten2_info_valid)),
  0x17,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten2_info),
  GET_CALL_FORWARDING_INFO_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten2_info) - QMI_IDL_OFFSET16RELATIVE(voice_get_call_forwarding_resp_msg_v02, get_call_forwarding_exten2_info_len),
  QMI_IDL_TYPE88(0, 51)
};

static const uint8_t voice_set_call_barring_password_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_call_barring_password_req_msg_v02, call_barring_password_info),
  QMI_IDL_TYPE88(0, 38)
};

static const uint8_t voice_set_call_barring_password_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_call_barring_password_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_call_barring_password_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_set_call_barring_password_resp_msg_v02, failure_cause_valid)),
  0x10,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_set_call_barring_password_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_call_barring_password_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_set_call_barring_password_resp_msg_v02, alpha_id_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_call_barring_password_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_set_call_barring_password_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_set_call_barring_password_resp_msg_v02, cc_result_type_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_set_call_barring_password_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_set_call_barring_password_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_set_call_barring_password_resp_msg_v02, call_id_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_set_call_barring_password_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_set_call_barring_password_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_set_call_barring_password_resp_msg_v02, cc_sups_result_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_set_call_barring_password_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t voice_orig_ussd_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_orig_ussd_req_msg_v02, uss_info),
  QMI_IDL_TYPE88(0, 39)
};

static const uint8_t voice_orig_ussd_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_orig_ussd_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_orig_ussd_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_orig_ussd_resp_msg_v02, failure_cause_valid)),
  0x10,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_orig_ussd_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_orig_ussd_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_orig_ussd_resp_msg_v02, alpha_id_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_orig_ussd_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, uss_info) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, uss_info_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_orig_ussd_resp_msg_v02, uss_info),
  QMI_IDL_TYPE88(0, 39),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_orig_ussd_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_orig_ussd_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_orig_ussd_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, uss_info_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, uss_info_utf16_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_orig_ussd_resp_msg_v02, uss_info_utf16),
  QMI_VOICE_USS_DATA_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, uss_info_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_resp_msg_v02, uss_info_utf16_len)
};

static const uint8_t voice_answer_ussd_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_answer_ussd_req_msg_v02, uss_info),
  QMI_IDL_TYPE88(0, 39)
};

static const uint8_t voice_answer_ussd_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_answer_ussd_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * voice_cancel_ussd_req_msg is empty
 * static const uint8_t voice_cancel_ussd_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_cancel_ussd_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_cancel_ussd_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * voice_ussd_release_ind_msg is empty
 * static const uint8_t voice_ussd_release_ind_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_ussd_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_ussd_ind_msg_v02, notification_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_ussd_ind_msg_v02, uss_info) - QMI_IDL_OFFSET8(voice_ussd_ind_msg_v02, uss_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_ussd_ind_msg_v02, uss_info),
  QMI_IDL_TYPE88(0, 39),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_ussd_ind_msg_v02, uss_info_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_ussd_ind_msg_v02, uss_info_utf16_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_ussd_ind_msg_v02, uss_info_utf16),
  QMI_VOICE_USS_DATA_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_ussd_ind_msg_v02, uss_info_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_ussd_ind_msg_v02, uss_info_utf16_len)
};

static const uint8_t voice_uus_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_uus_ind_msg_v02, uus_information),
  QMI_IDL_TYPE88(0, 23)
};

static const uint8_t voice_set_config_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, auto_answer) - QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, auto_answer_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, auto_answer),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, air_timer) - QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, air_timer_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, air_timer),
  QMI_IDL_TYPE88(0, 40),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, roam_timer) - QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, roam_timer_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, roam_timer),
  QMI_IDL_TYPE88(0, 41),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, tty_mode) - QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, tty_mode_valid)),
  0x13,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, tty_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, preferred_voice_so) - QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, preferred_voice_so_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, preferred_voice_so),
  QMI_IDL_TYPE88(0, 42),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, voice_domain) - QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, voice_domain_valid)),
  0x15,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_set_config_req_msg_v02, voice_domain)
};

static const uint8_t voice_set_config_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, auto_answer_outcome) - QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, auto_answer_outcome_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, auto_answer_outcome),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, air_timer_outcome) - QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, air_timer_outcome_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, air_timer_outcome),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, roam_timer_outcome) - QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, roam_timer_outcome_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, roam_timer_outcome),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, tty_mode_outcome) - QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, tty_mode_outcome_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, tty_mode_outcome),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, pref_voice_so_outcome) - QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, pref_voice_so_outcome_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, pref_voice_so_outcome),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, voice_domain_pref_outcome) - QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, voice_domain_pref_outcome_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_set_config_resp_msg_v02, voice_domain_pref_outcome)
};

static const uint8_t voice_get_config_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, auto_answer) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, auto_answer_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, auto_answer),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, air_timer) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, air_timer_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, air_timer),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, roam_timer) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, roam_timer_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, roam_timer),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, tty_mode) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, tty_mode_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, tty_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, pref_voice_so) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, pref_voice_so_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, pref_voice_so),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, amr_status) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, amr_status_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, amr_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, voice_privacy) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, voice_privacy_valid)),
  0x16,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, voice_privacy),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, nam_id) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, nam_id_valid)),
  0x17,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, nam_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, voice_domain_pref) - QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, voice_domain_pref_valid)),
  0x18,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_req_msg_v02, voice_domain_pref)
};

static const uint8_t voice_get_config_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, auto_answer_status) - QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, auto_answer_status_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, auto_answer_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, air_timer_count) - QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, air_timer_count_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, air_timer_count),
  QMI_IDL_TYPE88(0, 40),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, roam_timer_count) - QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, roam_timer_count_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, roam_timer_count),
  QMI_IDL_TYPE88(0, 41),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_tty_mode) - QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_tty_mode_valid)),
  0x13,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_tty_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_preferred_voice_so) - QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_preferred_voice_so_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_preferred_voice_so),
  QMI_IDL_TYPE88(0, 42),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_arm_config) - QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_arm_config_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_arm_config),
  QMI_IDL_TYPE88(0, 43),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_voice_privacy_pref) - QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_voice_privacy_pref_valid)),
  0x16,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, current_voice_privacy_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, voice_domain) - QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, voice_domain_valid)),
  0x17,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_config_resp_msg_v02, voice_domain)
};

static const uint8_t voice_sups_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, supplementary_service_info),
  QMI_IDL_TYPE88(0, 44),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, service_class) - QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, service_class_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, service_class),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, reason) - QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, reason_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, reason),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, number) - QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, number_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, number),
  QMI_VOICE_NUMBER_MAX_V02,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, timer_value) - QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, timer_value_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, timer_value),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, uss_info) - QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, uss_info_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_sups_ind_msg_v02, uss_info),
  QMI_IDL_TYPE88(0, 39),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, call_id_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, alpha_ident) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, alpha_ident_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, alpha_ident),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, password) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, password_valid)),
  0x17,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, password),
  4,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, new_password) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, new_password_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, new_password),
  QMI_IDL_TYPE88(0, 45),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, data_source) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, data_source_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, data_source),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, failure_cause) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, failure_cause_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, call_forwarding_info) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, call_forwarding_info_valid)),
  0x1B,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, call_forwarding_info),
  GET_CALL_FORWARDING_INFO_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, call_forwarding_info) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, call_forwarding_info_len),
  QMI_IDL_TYPE88(0, 36),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, clir_status) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, clir_status_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, clir_status),
  QMI_IDL_TYPE88(0, 35),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, clip_status) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, clip_status_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, clip_status),
  QMI_IDL_TYPE88(0, 34),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, colp_status) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, colp_status_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, colp_status),
  QMI_IDL_TYPE88(0, 46),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, colr_status) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, colr_status_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, colr_status),
  QMI_IDL_TYPE88(0, 46),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, cnap_status) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, cnap_status_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, cnap_status),
  QMI_IDL_TYPE88(0, 46),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, uss_info_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, uss_info_utf16_valid)),
  0x21,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, uss_info_utf16),
  QMI_VOICE_USS_DATA_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, uss_info_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, uss_info_utf16_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, service_class_ext) - QMI_IDL_OFFSET16RELATIVE(voice_sups_ind_msg_v02, service_class_ext_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_sups_ind_msg_v02, service_class_ext)
};

static const uint8_t voice_orig_ussd_no_wait_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_req_msg_v02, uss_info),
  QMI_IDL_TYPE88(0, 39)
};

static const uint8_t voice_orig_ussd_no_wait_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t voice_orig_ussd_no_wait_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, error) - QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, error_valid)),
  0x10,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, error),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, uss_info) - QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, uss_info_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_orig_ussd_no_wait_ind_msg_v02, uss_info),
  QMI_IDL_TYPE88(0, 39),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_no_wait_ind_msg_v02, alpha_id) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_no_wait_ind_msg_v02, alpha_id_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_orig_ussd_no_wait_ind_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_no_wait_ind_msg_v02, uss_info_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_no_wait_ind_msg_v02, uss_info_utf16_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_orig_ussd_no_wait_ind_msg_v02, uss_info_utf16),
  QMI_VOICE_USS_DATA_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_no_wait_ind_msg_v02, uss_info_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_orig_ussd_no_wait_ind_msg_v02, uss_info_utf16_len)
};

static const uint8_t voice_bind_subscription_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_bind_subscription_req_msg_v02, subs_type)
};

static const uint8_t voice_bind_subscription_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_bind_subscription_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t voice_als_set_line_switching_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_als_set_line_switching_req_msg_v02, switch_option)
};

static const uint8_t voice_als_set_line_switching_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_als_set_line_switching_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t voice_als_select_line_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_als_select_line_req_msg_v02, line_value)
};

static const uint8_t voice_als_select_line_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_als_select_line_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * voice_aoc_reset_acm_req_msg is empty
 * static const uint8_t voice_aoc_reset_acm_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_aoc_reset_acm_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_aoc_reset_acm_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t voice_aoc_set_acmmax_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_aoc_set_acmmax_req_msg_v02, acmmax)
};

static const uint8_t voice_aoc_set_acmmax_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_aoc_set_acmmax_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t voice_aoc_get_call_meter_info_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_req_msg_v02, info_mask)
};

static const uint8_t voice_aoc_get_call_meter_info_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, acm) - QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, acm_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, acm),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, acmmax) - QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, acmmax_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, acmmax),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, ccm) - QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, ccm_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_aoc_get_call_meter_info_resp_msg_v02, ccm)
};

/* 
 * voice_aoc_low_funds_ind_msg is empty
 * static const uint8_t voice_aoc_low_funds_ind_msg_data_v02[] = {
 * };
 */
  
/* 
 * voice_get_colp_req_msg is empty
 * static const uint8_t voice_get_colp_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_get_colp_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, colp_response) - QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, colp_response_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, colp_response),
  QMI_IDL_TYPE88(0, 46),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, alpha_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_colp_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_colp_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_colp_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_colp_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_colp_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_colp_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_colp_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_colp_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_get_colp_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_colp_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3)
};

/* 
 * voice_get_colr_req_msg is empty
 * static const uint8_t voice_get_colr_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_get_colr_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, colr_response) - QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, colr_response_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, colr_response),
  QMI_IDL_TYPE88(0, 46),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, alpha_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_colr_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_colr_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_colr_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_colr_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_colr_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_colr_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_colr_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_colr_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_get_colr_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_colr_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3)
};

/* 
 * voice_get_cnap_req_msg is empty
 * static const uint8_t voice_get_cnap_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_get_cnap_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, cnap_response) - QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, cnap_response_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, cnap_response),
  QMI_IDL_TYPE88(0, 46),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, alpha_id) - QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, alpha_id_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_get_cnap_resp_msg_v02, alpha_id),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_cnap_resp_msg_v02, cc_result_type) - QMI_IDL_OFFSET16RELATIVE(voice_get_cnap_resp_msg_v02, cc_result_type_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(voice_get_cnap_resp_msg_v02, cc_result_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_cnap_resp_msg_v02, call_id) - QMI_IDL_OFFSET16RELATIVE(voice_get_cnap_resp_msg_v02, call_id_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_get_cnap_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_get_cnap_resp_msg_v02, cc_sups_result) - QMI_IDL_OFFSET16RELATIVE(voice_get_cnap_resp_msg_v02, cc_sups_result_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(voice_get_cnap_resp_msg_v02, cc_sups_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t voice_manage_ip_calls_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, sups_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, call_type) - QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, call_type_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, call_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, audio_attrib) - QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, audio_attrib_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, audio_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, video_attrib) - QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, video_attrib_valid)),
  0x13,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, video_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, sip_uri) - QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, sip_uri_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, sip_uri),
  QMI_VOICE_SIP_URI_MAX_V02,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, reject_cause) - QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, reject_cause_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_req_msg_v02, reject_cause)
};

static const uint8_t voice_manage_ip_calls_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, failure_cause_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, failure_cause),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, num_participants) - QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, num_participants_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_manage_ip_calls_resp_msg_v02, num_participants)
};

/* 
 * voice_als_get_line_switching_status_req_msg is empty
 * static const uint8_t voice_als_get_line_switching_status_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_als_get_line_switching_status_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_als_get_line_switching_status_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_als_get_line_switching_status_resp_msg_v02, switch_value) - QMI_IDL_OFFSET8(voice_als_get_line_switching_status_resp_msg_v02, switch_value_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_als_get_line_switching_status_resp_msg_v02, switch_value)
};

/* 
 * voice_als_get_selected_line_req_msg is empty
 * static const uint8_t voice_als_get_selected_line_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t voice_als_get_selected_line_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_als_get_selected_line_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_als_get_selected_line_resp_msg_v02, line_value) - QMI_IDL_OFFSET8(voice_als_get_selected_line_resp_msg_v02, line_value_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_als_get_selected_line_resp_msg_v02, line_value)
};

static const uint8_t voice_modified_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, call_type) - QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, call_type_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, call_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, audio_attrib) - QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, audio_attrib_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, audio_attrib),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, video_attrib) - QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, video_attrib_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, video_attrib),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, failure_cause) - QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, failure_cause_valid)),
  0x13,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_modified_ind_msg_v02, failure_cause)
};

static const uint8_t voice_modify_accept_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, call_type) - QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, call_type_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, call_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, audio_attrib) - QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, audio_attrib_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, audio_attrib),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, video_attrib) - QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, video_attrib_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(voice_modify_accept_ind_msg_v02, video_attrib)
};

static const uint8_t voice_speech_codec_info_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, network_mode) - QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, network_mode_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, network_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, speech_codec) - QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, speech_codec_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, speech_codec),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, speech_enc_samp_freq) - QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, speech_enc_samp_freq_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, speech_enc_samp_freq),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, call_id_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_speech_codec_info_ind_msg_v02, call_id)
};

static const uint8_t voice_handover_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_handover_ind_msg_v02, ho_state),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_handover_ind_msg_v02, ho_type) - QMI_IDL_OFFSET8(voice_handover_ind_msg_v02, ho_type_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_handover_ind_msg_v02, ho_type)
};

static const uint8_t voice_conference_info_ind_msg_data_v02[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_conference_info_ind_msg_v02, conference_xml),
  ((QMI_VOICE_CONF_XML_MAX_LEN_V02) & 0xFF), ((QMI_VOICE_CONF_XML_MAX_LEN_V02) >> 8),
  QMI_IDL_OFFSET8(voice_conference_info_ind_msg_v02, conference_xml) - QMI_IDL_OFFSET8(voice_conference_info_ind_msg_v02, conference_xml_len),

  0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_conference_info_ind_msg_v02, sequence),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_conference_info_ind_msg_v02, total_size) - QMI_IDL_OFFSET16RELATIVE(voice_conference_info_ind_msg_v02, total_size_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_conference_info_ind_msg_v02, total_size)
};

static const uint8_t voice_conference_join_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_conference_join_ind_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_conference_join_ind_msg_v02, participant_uri),
  QMI_IDL_TYPE88(0, 52)
};

static const uint8_t voice_conference_participant_update_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_conference_participant_update_ind_msg_v02, participant_uri),
  QMI_IDL_TYPE88(0, 52)
};

static const uint8_t voice_ext_brst_intl_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_ext_brst_intl_ind_msg_v02, ext_burst_data),
  QMI_IDL_TYPE88(0, 53)
};

static const uint8_t voice_mt_page_miss_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_mt_page_miss_ind_msg_v02, page_miss_reason)
};

static const uint8_t voice_call_control_result_info_ind_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_call_control_result_info_ind_msg_v02, cc_result),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_call_control_result_info_ind_msg_v02, alpha_presence),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_call_control_result_info_ind_msg_v02, alpha_text_gsm8) - QMI_IDL_OFFSET8(voice_call_control_result_info_ind_msg_v02, alpha_text_gsm8_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_call_control_result_info_ind_msg_v02, alpha_text_gsm8),
  QMI_VOICE_CC_ALPHA_TEXT_MAX_V02,
  QMI_IDL_OFFSET8(voice_call_control_result_info_ind_msg_v02, alpha_text_gsm8) - QMI_IDL_OFFSET8(voice_call_control_result_info_ind_msg_v02, alpha_text_gsm8_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(voice_call_control_result_info_ind_msg_v02, alpha_text_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_call_control_result_info_ind_msg_v02, alpha_text_utf16_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(voice_call_control_result_info_ind_msg_v02, alpha_text_utf16),
  QMI_VOICE_CC_ALPHA_TEXT_MAX_V02,
  QMI_IDL_OFFSET16RELATIVE(voice_call_control_result_info_ind_msg_v02, alpha_text_utf16) - QMI_IDL_OFFSET16RELATIVE(voice_call_control_result_info_ind_msg_v02, alpha_text_utf16_len)
};

static const uint8_t voice_conf_participants_info_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_conf_participants_info_ind_msg_v02, conf_call_info),
  QMI_IDL_TYPE88(0, 60)
};

static const uint8_t voice_setup_answer_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_setup_answer_req_msg_v02, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_setup_answer_req_msg_v02, reject_setup) - QMI_IDL_OFFSET8(voice_setup_answer_req_msg_v02, reject_setup_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_setup_answer_req_msg_v02, reject_setup),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_setup_answer_req_msg_v02, reject_cause) - QMI_IDL_OFFSET8(voice_setup_answer_req_msg_v02, reject_cause_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(voice_setup_answer_req_msg_v02, reject_cause)
};

static const uint8_t voice_setup_answer_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(voice_setup_answer_resp_msg_v02, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(voice_setup_answer_resp_msg_v02, call_id) - QMI_IDL_OFFSET8(voice_setup_answer_resp_msg_v02, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(voice_setup_answer_resp_msg_v02, call_id)
};

static const uint8_t voice_tty_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(voice_tty_ind_msg_v02, tty_mode)
};

/* Type Table */
static const qmi_idl_type_table_entry  voice_type_table_v02[] = {
  {sizeof(voice_uus_type_v02), voice_uus_type_data_v02},
  {sizeof(voice_cug_type_v02), voice_cug_type_data_v02},
  {sizeof(voice_alpha_ident_type_v02), voice_alpha_ident_type_data_v02},
  {sizeof(voice_cc_sups_result_type_v02), voice_cc_sups_result_type_data_v02},
  {sizeof(voice_call_info_type_v02), voice_call_info_type_data_v02},
  {sizeof(voice_remote_party_number_type_v02), voice_remote_party_number_type_data_v02},
  {sizeof(voice_remote_party_name_type_v02), voice_remote_party_name_type_data_v02},
  {sizeof(voice_num_info_type_v02), voice_num_info_type_data_v02},
  {sizeof(voice_otasp_status_info_type_v02), voice_otasp_status_info_type_data_v02},
  {sizeof(voice_signal_info_type_v02), voice_signal_info_type_data_v02},
  {sizeof(voice_caller_id_info_type_v02), voice_caller_id_info_type_data_v02},
  {sizeof(voice_redirecting_num_info_type_v02), voice_redirecting_num_info_type_data_v02},
  {sizeof(voice_nss_audio_control_info_type_v02), voice_nss_audio_control_info_type_data_v02},
  {sizeof(voice_line_control_info_type_v02), voice_line_control_info_type_data_v02},
  {sizeof(voice_burst_dtmf_info_type_v02), voice_burst_dtmf_info_type_data_v02},
  {sizeof(voice_dtmf_lengths_type_v02), voice_dtmf_lengths_type_data_v02},
  {sizeof(voice_cont_dtmf_info_type_v02), voice_cont_dtmf_info_type_data_v02},
  {sizeof(voice_dtmf_info_type_v02), voice_dtmf_info_type_data_v02},
  {sizeof(voice_privacy_info_type_v02), voice_privacy_info_type_data_v02},
  {sizeof(voice_call_info2_type_v02), voice_call_info2_type_data_v02},
  {sizeof(voice_remote_party_number2_type_v02), voice_remote_party_number2_type_data_v02},
  {sizeof(voice_remote_party_name2_type_v02), voice_remote_party_name2_type_data_v02},
  {sizeof(voice_alerting_type_type_v02), voice_alerting_type_type_data_v02},
  {sizeof(voice_uus_info_type_v02), voice_uus_info_type_data_v02},
  {sizeof(voice_srv_opt_type_v02), voice_srv_opt_type_data_v02},
  {sizeof(voice_call_end_reason_type_v02), voice_call_end_reason_type_data_v02},
  {sizeof(voice_alpha_ident_with_id_type_v02), voice_alpha_ident_with_id_type_data_v02},
  {sizeof(voice_conn_num_with_id_type_v02), voice_conn_num_with_id_type_data_v02},
  {sizeof(voice_diagnostic_info_with_id_type_v02), voice_diagnostic_info_with_id_type_data_v02},
  {sizeof(voice_num_with_id_type_v02), voice_num_with_id_type_data_v02},
  {sizeof(voice_notification_info_type_v02), voice_notification_info_type_data_v02},
  {sizeof(voice_ect_number_type_v02), voice_ect_number_type_data_v02},
  {sizeof(voice_supplementary_service_info_type_v02), voice_supplementary_service_info_type_data_v02},
  {sizeof(voice_num_type_plan_type_v02), voice_num_type_plan_type_data_v02},
  {sizeof(voice_clip_response_type_v02), voice_clip_response_type_data_v02},
  {sizeof(voice_clir_response_type_v02), voice_clir_response_type_data_v02},
  {sizeof(voice_get_call_forwarding_info_type_v02), voice_get_call_forwarding_info_type_data_v02},
  {sizeof(voice_get_call_forwarding_info_exten_type_v02), voice_get_call_forwarding_info_exten_type_data_v02},
  {sizeof(voice_call_barring_password_info_type_v02), voice_call_barring_password_info_type_data_v02},
  {sizeof(voice_uss_info_type_v02), voice_uss_info_type_data_v02},
  {sizeof(voice_air_timer_type_v02), voice_air_timer_type_data_v02},
  {sizeof(voice_roam_timer_type_v02), voice_roam_timer_type_data_v02},
  {sizeof(voice_preferred_voice_so_type_v02), voice_preferred_voice_so_type_data_v02},
  {sizeof(voice_arm_config_type_v02), voice_arm_config_type_data_v02},
  {sizeof(voice_supp_service_info_type_v02), voice_supp_service_info_type_data_v02},
  {sizeof(voice_new_password_type_v02), voice_new_password_type_data_v02},
  {sizeof(voice_ss_status_type_v02), voice_ss_status_type_data_v02},
  {sizeof(voice_subaddress_type_v02), voice_subaddress_type_data_v02},
  {sizeof(voice_alerting_pattern_type_v02), voice_alerting_pattern_type_data_v02},
  {sizeof(voice_ext_display_info_type_v02), voice_ext_display_info_type_data_v02},
  {sizeof(voice_call_attributes_type_v02), voice_call_attributes_type_data_v02},
  {sizeof(voice_get_call_forwarding_info_exten2_type_v02), voice_get_call_forwarding_info_exten2_type_data_v02},
  {sizeof(voice_usr_uri_type_v02), voice_usr_uri_type_data_v02},
  {sizeof(voice_ext_brst_intl_type_v02), voice_ext_brst_intl_type_data_v02},
  {sizeof(voice_videoshare_type_v02), voice_videoshare_type_data_v02},
  {sizeof(voice_vs_variant_type_v02), voice_vs_variant_type_data_v02},
  {sizeof(voice_sip_uri_with_id_type_v02), voice_sip_uri_with_id_type_data_v02},
  {sizeof(voice_is_srvcc_call_with_id_type_v02), voice_is_srvcc_call_with_id_type_data_v02},
  {sizeof(voice_srvcc_parent_call_id_type_v02), voice_srvcc_parent_call_id_type_data_v02},
  {sizeof(voice_conf_participant_call_info_type_v02), voice_conf_participant_call_info_type_data_v02},
  {sizeof(voice_conference_call_info_type_v02), voice_conference_call_info_type_data_v02},
  {sizeof(voice_ip_call_capabilities_info_type_v02), voice_ip_call_capabilities_info_type_data_v02},
  {sizeof(voice_child_number_info_type_v02), voice_child_number_info_type_data_v02},
  {sizeof(voice_display_text_info_type_v02), voice_display_text_info_type_data_v02}
};

/* Message Table */
static const qmi_idl_message_table_entry voice_message_table_v02[] = {
  {sizeof(voice_indication_register_req_msg_v02), voice_indication_register_req_msg_data_v02},
  {sizeof(voice_indication_register_resp_msg_v02), voice_indication_register_resp_msg_data_v02},
  {sizeof(voice_dial_call_req_msg_v02), voice_dial_call_req_msg_data_v02},
  {sizeof(voice_dial_call_resp_msg_v02), voice_dial_call_resp_msg_data_v02},
  {sizeof(voice_end_call_req_msg_v02), voice_end_call_req_msg_data_v02},
  {sizeof(voice_end_call_resp_msg_v02), voice_end_call_resp_msg_data_v02},
  {sizeof(voice_answer_call_req_msg_v02), voice_answer_call_req_msg_data_v02},
  {sizeof(voice_answer_call_resp_msg_v02), voice_answer_call_resp_msg_data_v02},
  {sizeof(voice_get_call_info_req_msg_v02), voice_get_call_info_req_msg_data_v02},
  {sizeof(voice_get_call_info_resp_msg_v02), voice_get_call_info_resp_msg_data_v02},
  {sizeof(voice_otasp_status_ind_msg_v02), voice_otasp_status_ind_msg_data_v02},
  {sizeof(voice_info_rec_ind_msg_v02), voice_info_rec_ind_msg_data_v02},
  {sizeof(voice_send_flash_req_msg_v02), voice_send_flash_req_msg_data_v02},
  {sizeof(voice_send_flash_resp_msg_v02), voice_send_flash_resp_msg_data_v02},
  {sizeof(voice_burst_dtmf_req_msg_v02), voice_burst_dtmf_req_msg_data_v02},
  {sizeof(voice_burst_dtmf_resp_msg_v02), voice_burst_dtmf_resp_msg_data_v02},
  {sizeof(voice_start_cont_dtmf_req_msg_v02), voice_start_cont_dtmf_req_msg_data_v02},
  {sizeof(voice_start_cont_dtmf_resp_msg_v02), voice_start_cont_dtmf_resp_msg_data_v02},
  {sizeof(voice_stop_cont_dtmf_req_msg_v02), voice_stop_cont_dtmf_req_msg_data_v02},
  {sizeof(voice_stop_cont_dtmf_resp_msg_v02), voice_stop_cont_dtmf_resp_msg_data_v02},
  {sizeof(voice_dtmf_ind_msg_v02), voice_dtmf_ind_msg_data_v02},
  {sizeof(voice_set_preferred_privacy_req_msg_v02), voice_set_preferred_privacy_req_msg_data_v02},
  {sizeof(voice_set_preferred_privacy_resp_msg_v02), voice_set_preferred_privacy_resp_msg_data_v02},
  {sizeof(voice_privacy_ind_msg_v02), voice_privacy_ind_msg_data_v02},
  {sizeof(voice_all_call_status_ind_msg_v02), voice_all_call_status_ind_msg_data_v02},
  {sizeof(voice_get_all_call_info_req_msg_v02), 0},
  {sizeof(voice_get_all_call_info_resp_msg_v02), voice_get_all_call_info_resp_msg_data_v02},
  {sizeof(voice_manage_calls_req_msg_v02), voice_manage_calls_req_msg_data_v02},
  {sizeof(voice_manage_calls_resp_msg_v02), voice_manage_calls_resp_msg_data_v02},
  {sizeof(voice_sups_notification_ind_msg_v02), voice_sups_notification_ind_msg_data_v02},
  {sizeof(voice_set_sups_service_req_msg_v02), voice_set_sups_service_req_msg_data_v02},
  {sizeof(voice_set_sups_service_resp_msg_v02), voice_set_sups_service_resp_msg_data_v02},
  {sizeof(voice_get_call_waiting_req_msg_v02), voice_get_call_waiting_req_msg_data_v02},
  {sizeof(voice_get_call_waiting_resp_msg_v02), voice_get_call_waiting_resp_msg_data_v02},
  {sizeof(voice_get_call_barring_req_msg_v02), voice_get_call_barring_req_msg_data_v02},
  {sizeof(voice_get_call_barring_resp_msg_v02), voice_get_call_barring_resp_msg_data_v02},
  {sizeof(voice_get_clip_req_msg_v02), 0},
  {sizeof(voice_get_clip_resp_msg_v02), voice_get_clip_resp_msg_data_v02},
  {sizeof(voice_get_clir_req_msg_v02), 0},
  {sizeof(voice_get_clir_resp_msg_v02), voice_get_clir_resp_msg_data_v02},
  {sizeof(voice_get_call_forwarding_req_msg_v02), voice_get_call_forwarding_req_msg_data_v02},
  {sizeof(voice_get_call_forwarding_resp_msg_v02), voice_get_call_forwarding_resp_msg_data_v02},
  {sizeof(voice_set_call_barring_password_req_msg_v02), voice_set_call_barring_password_req_msg_data_v02},
  {sizeof(voice_set_call_barring_password_resp_msg_v02), voice_set_call_barring_password_resp_msg_data_v02},
  {sizeof(voice_orig_ussd_req_msg_v02), voice_orig_ussd_req_msg_data_v02},
  {sizeof(voice_orig_ussd_resp_msg_v02), voice_orig_ussd_resp_msg_data_v02},
  {sizeof(voice_answer_ussd_req_msg_v02), voice_answer_ussd_req_msg_data_v02},
  {sizeof(voice_answer_ussd_resp_msg_v02), voice_answer_ussd_resp_msg_data_v02},
  {sizeof(voice_cancel_ussd_req_msg_v02), 0},
  {sizeof(voice_cancel_ussd_resp_msg_v02), voice_cancel_ussd_resp_msg_data_v02},
  {sizeof(voice_ussd_release_ind_msg_v02), 0},
  {sizeof(voice_ussd_ind_msg_v02), voice_ussd_ind_msg_data_v02},
  {sizeof(voice_uus_ind_msg_v02), voice_uus_ind_msg_data_v02},
  {sizeof(voice_set_config_req_msg_v02), voice_set_config_req_msg_data_v02},
  {sizeof(voice_set_config_resp_msg_v02), voice_set_config_resp_msg_data_v02},
  {sizeof(voice_get_config_req_msg_v02), voice_get_config_req_msg_data_v02},
  {sizeof(voice_get_config_resp_msg_v02), voice_get_config_resp_msg_data_v02},
  {sizeof(voice_sups_ind_msg_v02), voice_sups_ind_msg_data_v02},
  {sizeof(voice_orig_ussd_no_wait_req_msg_v02), voice_orig_ussd_no_wait_req_msg_data_v02},
  {sizeof(voice_orig_ussd_no_wait_resp_msg_v02), voice_orig_ussd_no_wait_resp_msg_data_v02},
  {sizeof(voice_orig_ussd_no_wait_ind_msg_v02), voice_orig_ussd_no_wait_ind_msg_data_v02},
  {sizeof(voice_bind_subscription_req_msg_v02), voice_bind_subscription_req_msg_data_v02},
  {sizeof(voice_bind_subscription_resp_msg_v02), voice_bind_subscription_resp_msg_data_v02},
  {sizeof(voice_als_set_line_switching_req_msg_v02), voice_als_set_line_switching_req_msg_data_v02},
  {sizeof(voice_als_set_line_switching_resp_msg_v02), voice_als_set_line_switching_resp_msg_data_v02},
  {sizeof(voice_als_select_line_req_msg_v02), voice_als_select_line_req_msg_data_v02},
  {sizeof(voice_als_select_line_resp_msg_v02), voice_als_select_line_resp_msg_data_v02},
  {sizeof(voice_aoc_reset_acm_req_msg_v02), 0},
  {sizeof(voice_aoc_reset_acm_resp_msg_v02), voice_aoc_reset_acm_resp_msg_data_v02},
  {sizeof(voice_aoc_set_acmmax_req_msg_v02), voice_aoc_set_acmmax_req_msg_data_v02},
  {sizeof(voice_aoc_set_acmmax_resp_msg_v02), voice_aoc_set_acmmax_resp_msg_data_v02},
  {sizeof(voice_aoc_get_call_meter_info_req_msg_v02), voice_aoc_get_call_meter_info_req_msg_data_v02},
  {sizeof(voice_aoc_get_call_meter_info_resp_msg_v02), voice_aoc_get_call_meter_info_resp_msg_data_v02},
  {sizeof(voice_aoc_low_funds_ind_msg_v02), 0},
  {sizeof(voice_get_colp_req_msg_v02), 0},
  {sizeof(voice_get_colp_resp_msg_v02), voice_get_colp_resp_msg_data_v02},
  {sizeof(voice_get_colr_req_msg_v02), 0},
  {sizeof(voice_get_colr_resp_msg_v02), voice_get_colr_resp_msg_data_v02},
  {sizeof(voice_get_cnap_req_msg_v02), 0},
  {sizeof(voice_get_cnap_resp_msg_v02), voice_get_cnap_resp_msg_data_v02},
  {sizeof(voice_manage_ip_calls_req_msg_v02), voice_manage_ip_calls_req_msg_data_v02},
  {sizeof(voice_manage_ip_calls_resp_msg_v02), voice_manage_ip_calls_resp_msg_data_v02},
  {sizeof(voice_als_get_line_switching_status_req_msg_v02), 0},
  {sizeof(voice_als_get_line_switching_status_resp_msg_v02), voice_als_get_line_switching_status_resp_msg_data_v02},
  {sizeof(voice_als_get_selected_line_req_msg_v02), 0},
  {sizeof(voice_als_get_selected_line_resp_msg_v02), voice_als_get_selected_line_resp_msg_data_v02},
  {sizeof(voice_modified_ind_msg_v02), voice_modified_ind_msg_data_v02},
  {sizeof(voice_modify_accept_ind_msg_v02), voice_modify_accept_ind_msg_data_v02},
  {sizeof(voice_speech_codec_info_ind_msg_v02), voice_speech_codec_info_ind_msg_data_v02},
  {sizeof(voice_handover_ind_msg_v02), voice_handover_ind_msg_data_v02},
  {sizeof(voice_conference_info_ind_msg_v02), voice_conference_info_ind_msg_data_v02},
  {sizeof(voice_conference_join_ind_msg_v02), voice_conference_join_ind_msg_data_v02},
  {sizeof(voice_conference_participant_update_ind_msg_v02), voice_conference_participant_update_ind_msg_data_v02},
  {sizeof(voice_ext_brst_intl_ind_msg_v02), voice_ext_brst_intl_ind_msg_data_v02},
  {sizeof(voice_mt_page_miss_ind_msg_v02), voice_mt_page_miss_ind_msg_data_v02},
  {sizeof(voice_call_control_result_info_ind_msg_v02), voice_call_control_result_info_ind_msg_data_v02},
  {sizeof(voice_conf_participants_info_ind_msg_v02), voice_conf_participants_info_ind_msg_data_v02},
  {sizeof(voice_setup_answer_req_msg_v02), voice_setup_answer_req_msg_data_v02},
  {sizeof(voice_setup_answer_resp_msg_v02), voice_setup_answer_resp_msg_data_v02},
  {sizeof(voice_tty_ind_msg_v02), voice_tty_ind_msg_data_v02}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object voice_qmi_idl_type_table_object_v02;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *voice_qmi_idl_type_table_object_referenced_tables_v02[] =
{&voice_qmi_idl_type_table_object_v02, &common_qmi_idl_type_table_object_v01, &voice_service_common_qmi_idl_type_table_object_v02};

/*Type Table Object*/
static const qmi_idl_type_table_object voice_qmi_idl_type_table_object_v02 = {
  sizeof(voice_type_table_v02)/sizeof(qmi_idl_type_table_entry ),
  sizeof(voice_message_table_v02)/sizeof(qmi_idl_message_table_entry),
  1,
  voice_type_table_v02,
  voice_message_table_v02,
  voice_qmi_idl_type_table_object_referenced_tables_v02,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry voice_service_command_messages_v02[] = {
  {QMI_VOICE_INDICATION_REGISTER_REQ_V02, QMI_IDL_TYPE16(0, 0), 68},
  {QMI_VOICE_GET_SUPPORTED_MSGS_REQ_V02, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_VOICE_GET_SUPPORTED_FIELDS_REQ_V02, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_VOICE_DIAL_CALL_REQ_V02, QMI_IDL_TYPE16(0, 2), 2094},
  {QMI_VOICE_END_CALL_REQ_V02, QMI_IDL_TYPE16(0, 4), 11},
  {QMI_VOICE_ANSWER_CALL_REQ_V02, QMI_IDL_TYPE16(0, 6), 551},
  {QMI_VOICE_GET_CALL_INFO_REQ_V02, QMI_IDL_TYPE16(0, 8), 4},
  {QMI_VOICE_SEND_FLASH_REQ_V02, QMI_IDL_TYPE16(0, 12), 92},
  {QMI_VOICE_BURST_DTMF_REQ_V02, QMI_IDL_TYPE16(0, 14), 42},
  {QMI_VOICE_START_CONT_DTMF_REQ_V02, QMI_IDL_TYPE16(0, 16), 5},
  {QMI_VOICE_STOP_CONT_DTMF_REQ_V02, QMI_IDL_TYPE16(0, 18), 4},
  {QMI_VOICE_SET_PREFERRED_PRIVACY_REQ_V02, QMI_IDL_TYPE16(0, 21), 4},
  {QMI_VOICE_GET_ALL_CALL_INFO_REQ_V02, QMI_IDL_TYPE16(0, 25), 0},
  {QMI_VOICE_MANAGE_CALLS_REQ_V02, QMI_IDL_TYPE16(0, 27), 15},
  {QMI_VOICE_SET_SUPS_SERVICE_REQ_V02, QMI_IDL_TYPE16(0, 30), 116},
  {QMI_VOICE_GET_CALL_WAITING_REQ_V02, QMI_IDL_TYPE16(0, 32), 11},
  {QMI_VOICE_GET_CALL_BARRING_REQ_V02, QMI_IDL_TYPE16(0, 34), 15},
  {QMI_VOICE_GET_CLIP_REQ_V02, QMI_IDL_TYPE16(0, 36), 0},
  {QMI_VOICE_GET_CLIR_REQ_V02, QMI_IDL_TYPE16(0, 38), 0},
  {QMI_VOICE_GET_CALL_FORWARDING_REQ_V02, QMI_IDL_TYPE16(0, 40), 15},
  {QMI_VOICE_SET_CALL_BARRING_PASSWORD_REQ_V02, QMI_IDL_TYPE16(0, 42), 16},
  {QMI_VOICE_ORIG_USSD_REQ_V02, QMI_IDL_TYPE16(0, 44), 187},
  {QMI_VOICE_ANSWER_USSD_REQ_V02, QMI_IDL_TYPE16(0, 46), 187},
  {QMI_VOICE_CANCEL_USSD_REQ_V02, QMI_IDL_TYPE16(0, 48), 0},
  {QMI_VOICE_SET_CONFIG_REQ_V02, QMI_IDL_TYPE16(0, 53), 39},
  {QMI_VOICE_GET_CONFIG_REQ_V02, QMI_IDL_TYPE16(0, 55), 36},
  {QMI_VOICE_ORIG_USSD_NO_WAIT_REQ_V02, QMI_IDL_TYPE16(0, 58), 187},
  {QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02, QMI_IDL_TYPE16(0, 61), 4},
  {QMI_VOICE_ALS_SET_LINE_SWITCHING_REQ_V02, QMI_IDL_TYPE16(0, 63), 4},
  {QMI_VOICE_ALS_SELECT_LINE_REQ_V02, QMI_IDL_TYPE16(0, 65), 4},
  {QMI_VOICE_AOC_RESET_ACM_REQ_V02, QMI_IDL_TYPE16(0, 67), 0},
  {QMI_VOICE_AOC_SET_ACMMAX_REQ_V02, QMI_IDL_TYPE16(0, 69), 7},
  {QMI_VOICE_AOC_GET_CALL_METER_INFO_REQ_V02, QMI_IDL_TYPE16(0, 71), 5},
  {QMI_VOICE_GET_COLP_REQ_V02, QMI_IDL_TYPE16(0, 74), 0},
  {QMI_VOICE_GET_COLR_REQ_V02, QMI_IDL_TYPE16(0, 76), 0},
  {QMI_VOICE_GET_CNAP_REQ_V02, QMI_IDL_TYPE16(0, 78), 0},
  {QMI_VOICE_MANAGE_IP_CALLS_REQ_V02, QMI_IDL_TYPE16(0, 80), 172},
  {QMI_VOICE_ALS_GET_LINE_SWITCHING_STATUS_REQ_V02, QMI_IDL_TYPE16(0, 82), 0},
  {QMI_VOICE_ALS_GET_SELECTED_LINE_REQ_V02, QMI_IDL_TYPE16(0, 84), 0},
  {QMI_VOICE_SETUP_ANSWER_REQ_V02, QMI_IDL_TYPE16(0, 97), 15}
};

static const qmi_idl_service_message_table_entry voice_service_response_messages_v02[] = {
  {QMI_VOICE_INDICATION_REGISTER_RESP_V02, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_VOICE_GET_SUPPORTED_MSGS_RESP_V02, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_VOICE_GET_SUPPORTED_FIELDS_RESP_V02, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_VOICE_DIAL_CALL_RESP_V02, QMI_IDL_TYPE16(0, 3), 212},
  {QMI_VOICE_END_CALL_RESP_V02, QMI_IDL_TYPE16(0, 5), 11},
  {QMI_VOICE_ANSWER_CALL_RESP_V02, QMI_IDL_TYPE16(0, 7), 11},
  {QMI_VOICE_GET_CALL_INFO_RESP_V02, QMI_IDL_TYPE16(0, 9), 918},
  {QMI_VOICE_SEND_FLASH_RESP_V02, QMI_IDL_TYPE16(0, 13), 11},
  {QMI_VOICE_BURST_DTMF_RESP_V02, QMI_IDL_TYPE16(0, 15), 11},
  {QMI_VOICE_START_CONT_DTMF_RESP_V02, QMI_IDL_TYPE16(0, 17), 11},
  {QMI_VOICE_STOP_CONT_DTMF_RESP_V02, QMI_IDL_TYPE16(0, 19), 11},
  {QMI_VOICE_SET_PREFERRED_PRIVACY_RESP_V02, QMI_IDL_TYPE16(0, 22), 7},
  {QMI_VOICE_GET_ALL_CALL_INFO_RESP_V02, QMI_IDL_TYPE16(0, 26), 7349},
  {QMI_VOICE_MANAGE_CALLS_RESP_V02, QMI_IDL_TYPE16(0, 28), 12},
  {QMI_VOICE_SET_SUPS_SERVICE_RSEP_V02, QMI_IDL_TYPE16(0, 31), 734},
  {QMI_VOICE_GET_CALL_WAITING_RESP_V02, QMI_IDL_TYPE16(0, 33), 223},
  {QMI_VOICE_GET_CALL_BARRING_RESP_V02, QMI_IDL_TYPE16(0, 35), 223},
  {QMI_VOICE_GET_CLIP_RESP_V02, QMI_IDL_TYPE16(0, 37), 217},
  {QMI_VOICE_GET_CLIR_RESP_V02, QMI_IDL_TYPE16(0, 39), 217},
  {QMI_VOICE_GET_CALL_FORWARDING_RESP_V02, QMI_IDL_TYPE16(0, 41), 3682},
  {QMI_VOICE_SET_CALL_BARRING_PASSWORD_RESP_V02, QMI_IDL_TYPE16(0, 43), 212},
  {QMI_VOICE_ORIG_USSD_RESP_V02, QMI_IDL_TYPE16(0, 45), 767},
  {QMI_VOICE_ANSWER_USSD_RESP_V02, QMI_IDL_TYPE16(0, 47), 7},
  {QMI_VOICE_CANCEL_USSD_RESP_V02, QMI_IDL_TYPE16(0, 49), 7},
  {QMI_VOICE_SET_CONFIG_RESP_V02, QMI_IDL_TYPE16(0, 54), 31},
  {QMI_VOICE_GET_CONFIG_RESP_V02, QMI_IDL_TYPE16(0, 56), 55},
  {QMI_VOICE_ORIG_USSD_NO_WAIT_RESP_V02, QMI_IDL_TYPE16(0, 59), 7},
  {QMI_VOICE_BIND_SUBSCRIPTION_RESP_V02, QMI_IDL_TYPE16(0, 62), 7},
  {QMI_VOICE_ALS_SET_LINE_SWITCHING_RESP_V02, QMI_IDL_TYPE16(0, 64), 7},
  {QMI_VOICE_ALS_SELECT_LINE_RESP_V02, QMI_IDL_TYPE16(0, 66), 7},
  {QMI_VOICE_AOC_RESET_ACM_RESP_V02, QMI_IDL_TYPE16(0, 68), 7},
  {QMI_VOICE_AOC_SET_ACMMAX_RESP_V02, QMI_IDL_TYPE16(0, 70), 7},
  {QMI_VOICE_AOC_GET_CALL_METER_INFO_RESP_V02, QMI_IDL_TYPE16(0, 72), 28},
  {QMI_VOICE_GET_COLP_RESP_V02, QMI_IDL_TYPE16(0, 75), 217},
  {QMI_VOICE_GET_COLR_RESP_V02, QMI_IDL_TYPE16(0, 77), 217},
  {QMI_VOICE_GET_CNAP_RESP_V02, QMI_IDL_TYPE16(0, 79), 217},
  {QMI_VOICE_MANAGE_IP_CALLS_RESP_V02, QMI_IDL_TYPE16(0, 81), 20},
  {QMI_VOICE_ALS_GET_LINE_SWITCHING_STATUS_RESP_V02, QMI_IDL_TYPE16(0, 83), 11},
  {QMI_VOICE_ALS_GET_SELECTED_LINE_RESP_V02, QMI_IDL_TYPE16(0, 85), 11},
  {QMI_VOICE_SETUP_ANSWER_RESP_V02, QMI_IDL_TYPE16(0, 98), 11}
};

static const qmi_idl_service_message_table_entry voice_service_indication_messages_v02[] = {
  {QMI_VOICE_OTASP_STATUS_IND_V02, QMI_IDL_TYPE16(0, 10), 5},
  {QMI_VOICE_INFO_REC_IND_V02, QMI_IDL_TYPE16(0, 11), 1101},
  {QMI_VOICE_DTMF_IND_V02, QMI_IDL_TYPE16(0, 20), 78},
  {QMI_VOICE_PRIVACY_IND_V02, QMI_IDL_TYPE16(0, 23), 5},
  {QMI_VOICE_ALL_CALL_STATUS_IND_V02, QMI_IDL_TYPE16(0, 24), 9093},
  {QMI_VOICE_SUPS_NOTIFICATION_IND_V02, QMI_IDL_TYPE16(0, 29), 104},
  {QMI_VOICE_USSD_RELEASE_IND_V02, QMI_IDL_TYPE16(0, 50), 0},
  {QMI_VOICE_USSD_IND_V02, QMI_IDL_TYPE16(0, 51), 559},
  {QMI_VOICE_UUS_IND_V02, QMI_IDL_TYPE16(0, 52), 135},
  {QMI_VOICE_SUPS_IND_V02, QMI_IDL_TYPE16(0, 57), 2015},
  {QMI_VOICE_ORIG_USSD_NO_WAIT_IND_V02, QMI_IDL_TYPE16(0, 60), 752},
  {QMI_VOICE_AOC_LOW_FUNDS_IND_V02, QMI_IDL_TYPE16(0, 73), 0},
  {QMI_VOICE_MODIFIED_IND_V02, QMI_IDL_TYPE16(0, 86), 35},
  {QMI_VOICE_MODIFY_ACCEPT_IND_V02, QMI_IDL_TYPE16(0, 87), 30},
  {QMI_VOICE_SPEECH_CODEC_INFO_IND_V02, QMI_IDL_TYPE16(0, 88), 25},
  {QMI_VOICE_HANDOVER_IND_V02, QMI_IDL_TYPE16(0, 89), 14},
  {QMI_VOICE_CONFERENCE_INFO_IND_V02, QMI_IDL_TYPE16(0, 90), 2067},
  {QMI_VOICE_CONFERENCE_JOIN_IND_V02, QMI_IDL_TYPE16(0, 91), 393},
  {QMI_VOICE_CONFERENCE_PARTICIPANT_UPDATE_IND_V02, QMI_IDL_TYPE16(0, 92), 389},
  {QMI_VOICE_EXT_BRST_INTL_IND_V02, QMI_IDL_TYPE16(0, 93), 9},
  {QMI_VOICE_MT_PAGE_MISS_IND_V02, QMI_IDL_TYPE16(0, 94), 5},
  {QMI_VOICE_CALL_CONTROL_RESULT_INFO_IND_V02, QMI_IDL_TYPE16(0, 95), 787},
  {QMI_VOICE_CONFERENCE_PARTICIPANTS_INFO_IND_V02, QMI_IDL_TYPE16(0, 96), 3468},
  {QMI_VOICE_TTY_IND_V02, QMI_IDL_TYPE16(0, 99), 4}
};

/*Service Object*/
struct qmi_idl_service_object voice_qmi_idl_service_object_v02 = {
  0x06,
  0x02,
  0x09,
  9093,
  { sizeof(voice_service_command_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(voice_service_response_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(voice_service_indication_messages_v02)/sizeof(qmi_idl_service_message_table_entry) },
  { voice_service_command_messages_v02, voice_service_response_messages_v02, voice_service_indication_messages_v02},
  &voice_qmi_idl_type_table_object_v02,
  0x25,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type voice_get_service_object_internal_v02
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( VOICE_V02_IDL_MAJOR_VERS != idl_maj_version || VOICE_V02_IDL_MINOR_VERS != idl_min_version 
       || VOICE_V02_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&voice_qmi_idl_service_object_v02;
}

