/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        U S E R _ I D E N T I T Y _ M O D U L E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the uim service Data structures.

  Copyright (c) 2010-2015 Qualcomm Technologies, Inc. All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.5
   It was generated on: Fri Jul 24 2015 (Spin 0)
   From IDL File: user_identity_module_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "user_identity_module_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t uim_session_information_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_session_information_type_v01, session_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_session_information_type_v01, aid),
  QMI_UIM_AID_MAX_V01,
  QMI_IDL_OFFSET8(uim_session_information_type_v01, aid) - QMI_IDL_OFFSET8(uim_session_information_type_v01, aid_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_file_id_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_id_type_v01, file_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_file_id_type_v01, path),
  QMI_UIM_PATH_MAX_V01,
  QMI_IDL_OFFSET8(uim_file_id_type_v01, path) - QMI_IDL_OFFSET8(uim_file_id_type_v01, path_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_read_transparent_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_type_v01, offset),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_type_v01, length),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_card_result_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_card_result_type_v01, sw1),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_card_result_type_v01, sw2),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_read_transparent_result_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_result_type_v01, content),
  ((QMI_UIM_CONTENT_TRANSPARENT_MAX_V01) & 0xFF), ((QMI_UIM_CONTENT_TRANSPARENT_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_read_transparent_result_type_v01, content) - QMI_IDL_OFFSET8(uim_read_transparent_result_type_v01, content_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_read_record_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_read_record_type_v01, record),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_read_record_type_v01, length),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_read_record_result_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_read_record_result_type_v01, content),
  ((QMI_UIM_CONTENT_RECORD_MAX_V01) & 0xFF), ((QMI_UIM_CONTENT_RECORD_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_read_record_result_type_v01, content) - QMI_IDL_OFFSET8(uim_read_record_result_type_v01, content_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_read_additional_record_result_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_read_additional_record_result_type_v01, additional_record),
  ((QMI_UIM_CONTENT_ADDITIONAL_RECORD_MAX_V01) & 0xFF), ((QMI_UIM_CONTENT_ADDITIONAL_RECORD_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_read_additional_record_result_type_v01, additional_record) - QMI_IDL_OFFSET8(uim_read_additional_record_result_type_v01, additional_record_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_write_transparent_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_write_transparent_type_v01, offset),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_write_transparent_type_v01, data),
  ((QMI_UIM_CONTENT_TRANSPARENT_MAX_V01) & 0xFF), ((QMI_UIM_CONTENT_TRANSPARENT_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_write_transparent_type_v01, data) - QMI_IDL_OFFSET8(uim_write_transparent_type_v01, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_write_record_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_write_record_type_v01, record),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_write_record_type_v01, data),
  ((QMI_UIM_CONTENT_RECORD_MAX_V01) & 0xFF), ((QMI_UIM_CONTENT_RECORD_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_write_record_type_v01, data) - QMI_IDL_OFFSET8(uim_write_record_type_v01, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_file_attributes_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, file_size),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, file_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, file_type),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, rec_size),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, rec_count),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_read),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_read_mask),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_write),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_write_mask),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_increase),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_increase_mask),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_deactivate),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_deactivate_mask),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_activate),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, sec_activate_mask),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, raw_value),
  ((QMI_UIM_FILE_ATTR_RAW_VALUE_MAX_V01) & 0xFF), ((QMI_UIM_FILE_ATTR_RAW_VALUE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_file_attributes_type_v01, raw_value) - QMI_IDL_OFFSET8(uim_file_attributes_type_v01, raw_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_register_refresh_file_id_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_register_refresh_file_id_type_v01, file_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_register_refresh_file_id_type_v01, path),
  QMI_UIM_PATH_MAX_V01,
  QMI_IDL_OFFSET8(uim_register_refresh_file_id_type_v01, path) - QMI_IDL_OFFSET8(uim_register_refresh_file_id_type_v01, path_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_register_refresh_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_register_refresh_type_v01, register_flag),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_register_refresh_type_v01, vote_for_init),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_register_refresh_type_v01, files),
  ((QMI_UIM_REFRESH_FILES_MAX_V01) & 0xFF), ((QMI_UIM_REFRESH_FILES_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_register_refresh_type_v01, files) - QMI_IDL_OFFSET8(uim_register_refresh_type_v01, files_len),
  QMI_IDL_TYPE88(0, 11),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_refresh_event_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_refresh_event_type_v01, stage),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_refresh_event_type_v01, mode),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_refresh_event_type_v01, session_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_refresh_event_type_v01, aid),
  QMI_UIM_AID_MAX_V01,
  QMI_IDL_OFFSET8(uim_refresh_event_type_v01, aid) - QMI_IDL_OFFSET8(uim_refresh_event_type_v01, aid_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_event_type_v01, files),
  ((QMI_UIM_REFRESH_FILES_MAX_V01) & 0xFF), ((QMI_UIM_REFRESH_FILES_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_refresh_event_type_v01, files) - QMI_IDL_OFFSET8(uim_refresh_event_type_v01, files_len),
  QMI_IDL_TYPE88(0, 11),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_set_pin_protection_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_set_pin_protection_type_v01, pin_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_set_pin_protection_type_v01, pin_operation),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_type_v01, pin_value),
  QMI_UIM_PIN_MAX_V01,
  QMI_IDL_OFFSET8(uim_set_pin_protection_type_v01, pin_value) - QMI_IDL_OFFSET8(uim_set_pin_protection_type_v01, pin_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_pin_retries_left_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_pin_retries_left_type_v01, verify_left),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_pin_retries_left_type_v01, unblock_left),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_encrypted_pin_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_encrypted_pin_type_v01, enc_pin1_value),
  QMI_UIM_ENCRYPTED_PIN_MAX_V01,
  QMI_IDL_OFFSET8(uim_encrypted_pin_type_v01, enc_pin1_value) - QMI_IDL_OFFSET8(uim_encrypted_pin_type_v01, enc_pin1_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_verify_pin_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_verify_pin_type_v01, pin_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_verify_pin_type_v01, pin_value),
  QMI_UIM_PIN_MAX_V01,
  QMI_IDL_OFFSET8(uim_verify_pin_type_v01, pin_value) - QMI_IDL_OFFSET8(uim_verify_pin_type_v01, pin_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_unblock_pin_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_unblock_pin_type_v01, pin_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_unblock_pin_type_v01, puk_value),
  QMI_UIM_PIN_MAX_V01,
  QMI_IDL_OFFSET8(uim_unblock_pin_type_v01, puk_value) - QMI_IDL_OFFSET8(uim_unblock_pin_type_v01, puk_value_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_unblock_pin_type_v01, new_pin_value),
  QMI_UIM_PIN_MAX_V01,
  QMI_IDL_OFFSET8(uim_unblock_pin_type_v01, new_pin_value) - QMI_IDL_OFFSET8(uim_unblock_pin_type_v01, new_pin_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_change_pin_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_change_pin_type_v01, pin_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_change_pin_type_v01, old_pin_value),
  QMI_UIM_PIN_MAX_V01,
  QMI_IDL_OFFSET8(uim_change_pin_type_v01, old_pin_value) - QMI_IDL_OFFSET8(uim_change_pin_type_v01, old_pin_value_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_change_pin_type_v01, new_pin_value),
  QMI_UIM_PIN_MAX_V01,
  QMI_IDL_OFFSET8(uim_change_pin_type_v01, new_pin_value) - QMI_IDL_OFFSET8(uim_change_pin_type_v01, new_pin_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_depersonalization_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_depersonalization_type_v01, feature),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_depersonalization_type_v01, operation),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_depersonalization_type_v01, ck_value),
  QMI_UIM_CK_MAX_V01,
  QMI_IDL_OFFSET8(uim_depersonalization_type_v01, ck_value) - QMI_IDL_OFFSET8(uim_depersonalization_type_v01, ck_value_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_perso_retries_left_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_perso_retries_left_type_v01, verify_left),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_perso_retries_left_type_v01, unblock_left),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_pin_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_pin_info_type_v01, pin_state),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_pin_info_type_v01, pin_retries),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_pin_info_type_v01, puk_retries),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t app_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(app_info_type_v01, app_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(app_info_type_v01, app_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(app_info_type_v01, perso_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(app_info_type_v01, perso_feature),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(app_info_type_v01, perso_retries),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(app_info_type_v01, perso_unblock_retries),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(app_info_type_v01, aid_value),
  QMI_UIM_AID_MAX_V01,
  QMI_IDL_OFFSET8(app_info_type_v01, aid_value) - QMI_IDL_OFFSET8(app_info_type_v01, aid_value_len),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(app_info_type_v01, univ_pin),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(app_info_type_v01, pin1),
  QMI_IDL_TYPE88(0, 22),
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(app_info_type_v01, pin2),
  QMI_IDL_TYPE88(0, 22),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t card_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(card_info_type_v01, card_state),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(card_info_type_v01, upin),
  QMI_IDL_TYPE88(0, 22),
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(card_info_type_v01, error_code),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(card_info_type_v01, app_info),
  QMI_UIM_APPS_MAX_V01,
  QMI_IDL_OFFSET8(card_info_type_v01, app_info) - QMI_IDL_OFFSET8(card_info_type_v01, app_info_len),
  QMI_IDL_TYPE88(0, 23),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_card_status_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_card_status_type_v01, index_gw_pri),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_card_status_type_v01, index_1x_pri),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_card_status_type_v01, index_gw_sec),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_card_status_type_v01, index_1x_sec),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_card_status_type_v01, card_info),
  QMI_UIM_CARDS_MAX_V01,
  QMI_IDL_OFFSET8(uim_card_status_type_v01, card_info) - QMI_IDL_OFFSET8(uim_card_status_type_v01, card_info_len),
  QMI_IDL_TYPE88(0, 24),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t extended_card_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(extended_card_info_type_v01, card_state),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(extended_card_info_type_v01, upin),
  QMI_IDL_TYPE88(0, 22),
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(extended_card_info_type_v01, error_code),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(extended_card_info_type_v01, app_info),
  QMI_UIM_EXTENDED_APPS_MAX_V01,
  QMI_IDL_OFFSET8(extended_card_info_type_v01, app_info) - QMI_IDL_OFFSET8(extended_card_info_type_v01, app_info_len),
  QMI_IDL_TYPE88(0, 23),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_extended_card_status_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, index_gw),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, index_gw) - QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, index_gw_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, index_1x),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, index_1x) - QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, index_1x_len),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, card_info),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, card_info) - QMI_IDL_OFFSET8(uim_extended_card_status_type_v01, card_info_len),
  QMI_IDL_TYPE88(0, 26),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_authentication_data_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_authentication_data_type_v01, context),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_authentication_data_type_v01, data),
  ((QMI_UIM_AUTHENTICATE_DATA_MAX_V01) & 0xFF), ((QMI_UIM_AUTHENTICATE_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_authentication_data_type_v01, data) - QMI_IDL_OFFSET8(uim_authentication_data_type_v01, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_session_change_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_session_change_type_v01, session_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_session_change_type_v01, activate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_application_information_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_application_information_type_v01, slot),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_application_information_type_v01, aid),
  QMI_UIM_AID_MAX_V01,
  QMI_IDL_OFFSET8(uim_application_information_type_v01, aid) - QMI_IDL_OFFSET8(uim_application_information_type_v01, aid_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_personalization_status_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_personalization_status_type_v01, feature),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_personalization_status_type_v01, verify_left),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_personalization_status_type_v01, unblock_left),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_personalization_status_other_slots_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_personalization_status_other_slots_type_v01, personalization_status),
  QMI_UIM_PERSO_FEATURE_MAX_V01,
  QMI_IDL_OFFSET8(uim_personalization_status_other_slots_type_v01, personalization_status) - QMI_IDL_OFFSET8(uim_personalization_status_other_slots_type_v01, personalization_status_len),
  QMI_IDL_TYPE88(0, 31),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_send_apdu_long_response_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_send_apdu_long_response_type_v01, total_length),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_send_apdu_long_response_type_v01, token),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_send_apdu_long_response_chunk_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_send_apdu_long_response_chunk_type_v01, token),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_send_apdu_long_response_chunk_type_v01, total_length),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_send_apdu_long_response_chunk_type_v01, offset),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_send_apdu_long_response_chunk_type_v01, apdu),
  ((QMI_UIM_APDU_DATA_MAX_V01) & 0xFF), ((QMI_UIM_APDU_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_send_apdu_long_response_chunk_type_v01, apdu) - QMI_IDL_OFFSET8(uim_send_apdu_long_response_chunk_type_v01, apdu_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_sap_connect_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_sap_connect_type_v01, connect),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_sap_connect_type_v01, slot),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_sap_request_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_sap_request_type_v01, sap_request),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_sap_request_type_v01, slot),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_sap_indication_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_sap_indication_type_v01, sap_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_sap_indication_type_v01, slot),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_physical_slot_status_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_physical_slot_status_type_v01, physical_card_status),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_physical_slot_status_type_v01, physical_slot_state),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_physical_slot_status_type_v01, logical_slot),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_physical_slot_status_type_v01, iccid),
  QMI_UIM_ICCID_LEN_MAX_V01,
  QMI_IDL_OFFSET8(uim_physical_slot_status_type_v01, iccid) - QMI_IDL_OFFSET8(uim_physical_slot_status_type_v01, iccid_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_read_transparent_long_response_chunk_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_long_response_chunk_type_v01, long_data_token),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_long_response_chunk_type_v01, total_length),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_long_response_chunk_type_v01, offset),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_long_response_chunk_type_v01, read_result),
  QMI_IDL_TYPE88(0, 4),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_plmn_name_release_date_info_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_plmn_name_release_date_info_type_v01, month),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_plmn_name_release_date_info_type_v01, day),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_plmn_name_release_date_info_type_v01, year),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_network_perso_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_network_perso_type_v01, mcc),
  QMI_UIM_MCC_LEN_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_network_perso_type_v01, mnc),
  QMI_UIM_MNC_MAX_V01,
  QMI_IDL_OFFSET8(uim_network_perso_type_v01, mnc) - QMI_IDL_OFFSET8(uim_network_perso_type_v01, mnc_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_gw_network_subset_perso_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_gw_network_subset_perso_type_v01, network),
  QMI_IDL_TYPE88(0, 41),
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gw_network_subset_perso_type_v01, digit6),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gw_network_subset_perso_type_v01, digit7),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_gw_sp_perso_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_gw_sp_perso_type_v01, network),
  QMI_IDL_TYPE88(0, 41),
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gw_sp_perso_type_v01, gid1),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_gw_corporate_perso_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_gw_corporate_perso_type_v01, network),
  QMI_IDL_TYPE88(0, 41),
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gw_corporate_perso_type_v01, gid1),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gw_corporate_perso_type_v01, gid2),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_sim_perso_type_data_v01[] = {
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_sim_perso_type_v01, network),
  QMI_IDL_TYPE88(0, 41),
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_sim_perso_type_v01, msin),
  QMI_UIM_MSIN_MAX_V01,
  QMI_IDL_OFFSET8(uim_sim_perso_type_v01, msin) - QMI_IDL_OFFSET8(uim_sim_perso_type_v01, msin_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_1x_network_type2_perso_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_1x_network_type2_perso_type_v01, irm_code),
  QMI_UIM_IRM_CODE_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t profile_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(profile_info_type_v01, profile_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(profile_info_type_v01, profile_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_naf_id_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_naf_id_type_v01, fqdn),
  QMI_UIM_FQDN_MAX_V01,
  QMI_IDL_OFFSET8(uim_naf_id_type_v01, fqdn) - QMI_IDL_OFFSET8(uim_naf_id_type_v01, fqdn_len),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_naf_id_type_v01, security_protocol_id),
  QMI_UIM_SECURITY_PROTOCOL_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_additional_search_data_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_additional_search_data_type_v01, search_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_additional_search_data_type_v01, first_record),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_temporary_unlock_status_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_temporary_unlock_status_type_v01, feature),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_temporary_unlock_status_type_v01, temporary_unlock_time_left),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_temporary_depersonalization_status_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_temporary_depersonalization_status_type_v01, temporary_unlock_status),
  QMI_UIM_PERSO_FEATURE_MAX_V01,
  QMI_IDL_OFFSET8(uim_temporary_depersonalization_status_type_v01, temporary_unlock_status) - QMI_IDL_OFFSET8(uim_temporary_depersonalization_status_type_v01, temporary_unlock_status_len),
  QMI_IDL_TYPE88(0, 50),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_perso_reactivation_status_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_perso_reactivation_status_type_v01, slot),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_perso_reactivation_status_type_v01, personalization_status),
  QMI_IDL_TYPE88(0, 31),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_temporary_unlock_feature_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_temporary_unlock_feature_type_v01, feature),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_temporary_unlock_feature_type_v01, duration),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t uim_temporary_unlock_slot_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_temporary_unlock_slot_info_type_v01, slot_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_temporary_unlock_slot_info_type_v01, feature_info),
  QMI_UIM_PERSO_FEATURE_MAX_V01,
  QMI_IDL_OFFSET8(uim_temporary_unlock_slot_info_type_v01, feature_info) - QMI_IDL_OFFSET8(uim_temporary_unlock_slot_info_type_v01, feature_info_len),
  QMI_IDL_TYPE88(0, 53),
  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/*
 * uim_reset_req_msg is empty
 * static const uint8_t uim_reset_req_msg_data_v01[] = {
 * };
 */

static const uint8_t uim_reset_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_reset_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_read_transparent_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, file_id),
  QMI_IDL_TYPE88(0, 1),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, read_transparent),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, indication_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, encryption) - QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, encryption_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, encryption),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, encryption_ext) - QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, encryption_ext_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_req_msg_v01, encryption_ext)
};

static const uint8_t uim_read_transparent_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_transparent_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_read_transparent_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_transparent_resp_msg_v01, read_result) - QMI_IDL_OFFSET8(uim_read_transparent_resp_msg_v01, read_result_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_resp_msg_v01, read_result),
  QMI_IDL_TYPE88(0, 4),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_resp_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, encryption) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, encryption_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_resp_msg_v01, encryption),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, file_length) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, file_length_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_resp_msg_v01, file_length),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, long_data_token) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, long_data_token_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_resp_msg_v01, long_data_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, read_result_ext) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, read_result_ext_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_resp_msg_v01, read_result_ext),
  ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, read_result_ext) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_resp_msg_v01, read_result_ext_len)
};

static const uint8_t uim_read_transparent_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_read_transparent_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_transparent_ind_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_read_transparent_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_transparent_ind_msg_v01, read_result) - QMI_IDL_OFFSET8(uim_read_transparent_ind_msg_v01, read_result_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_ind_msg_v01, read_result),
  QMI_IDL_TYPE88(0, 4),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, encryption) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, encryption_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_ind_msg_v01, encryption),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, file_length) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, file_length_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_ind_msg_v01, file_length),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, long_data_token) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, long_data_token_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_ind_msg_v01, long_data_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, read_result_ext) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, read_result_ext_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_ind_msg_v01, read_result_ext),
  ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, read_result_ext) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_ind_msg_v01, read_result_ext_len)
};

static const uint8_t uim_read_record_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, file_id),
  QMI_IDL_TYPE88(0, 1),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, read_record),
  QMI_IDL_TYPE88(0, 5),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, last_record) - QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, last_record_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, last_record),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, indication_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_read_record_req_msg_v01, indication_token)
};

static const uint8_t uim_read_record_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_record_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_read_record_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_record_resp_msg_v01, read_result) - QMI_IDL_OFFSET8(uim_read_record_resp_msg_v01, read_result_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_resp_msg_v01, read_result),
  QMI_IDL_TYPE88(0, 6),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_record_resp_msg_v01, additional_read_result) - QMI_IDL_OFFSET16RELATIVE(uim_read_record_resp_msg_v01, additional_read_result_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_read_record_resp_msg_v01, additional_read_result),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_record_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_read_record_resp_msg_v01, indication_token_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_record_resp_msg_v01, indication_token)
};

static const uint8_t uim_read_record_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_read_record_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_record_ind_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_read_record_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_read_record_ind_msg_v01, read_result) - QMI_IDL_OFFSET8(uim_read_record_ind_msg_v01, read_result_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_record_ind_msg_v01, read_result),
  QMI_IDL_TYPE88(0, 6),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_record_ind_msg_v01, additional_read_result) - QMI_IDL_OFFSET16RELATIVE(uim_read_record_ind_msg_v01, additional_read_result_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_read_record_ind_msg_v01, additional_read_result),
  QMI_IDL_TYPE88(0, 7)
};

static const uint8_t uim_write_transparent_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_transparent_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_transparent_req_msg_v01, file_id),
  QMI_IDL_TYPE88(0, 1),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_transparent_req_msg_v01, write_transparent),
  QMI_IDL_TYPE88(0, 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_write_transparent_req_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_write_transparent_req_msg_v01, indication_token_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_write_transparent_req_msg_v01, indication_token)
};

static const uint8_t uim_write_transparent_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_transparent_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_write_transparent_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_write_transparent_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_transparent_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_write_transparent_resp_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_write_transparent_resp_msg_v01, indication_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_write_transparent_resp_msg_v01, indication_token)
};

static const uint8_t uim_write_transparent_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_transparent_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_write_transparent_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_write_transparent_ind_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_write_transparent_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_transparent_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_write_record_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_record_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_record_req_msg_v01, file_id),
  QMI_IDL_TYPE88(0, 1),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_record_req_msg_v01, write_record),
  QMI_IDL_TYPE88(0, 9),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_write_record_req_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_write_record_req_msg_v01, indication_token_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_write_record_req_msg_v01, indication_token)
};

static const uint8_t uim_write_record_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_record_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_write_record_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_write_record_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_record_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_write_record_resp_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_write_record_resp_msg_v01, indication_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_write_record_resp_msg_v01, indication_token)
};

static const uint8_t uim_write_record_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_record_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_write_record_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_write_record_ind_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_write_record_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_write_record_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_get_file_attributes_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_req_msg_v01, file_id),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_file_attributes_req_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_get_file_attributes_req_msg_v01, indication_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_req_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_file_attributes_req_msg_v01, uicc_security_attributes) - QMI_IDL_OFFSET8(uim_get_file_attributes_req_msg_v01, uicc_security_attributes_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_req_msg_v01, uicc_security_attributes)
};

static const uint8_t uim_get_file_attributes_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_file_attributes_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_get_file_attributes_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_file_attributes_resp_msg_v01, file_attributes) - QMI_IDL_OFFSET8(uim_get_file_attributes_resp_msg_v01, file_attributes_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_resp_msg_v01, file_attributes),
  QMI_IDL_TYPE88(0, 10),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_file_attributes_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_get_file_attributes_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_get_file_attributes_resp_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_file_attributes_resp_msg_v01, file_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_file_attributes_resp_msg_v01, file_status_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_get_file_attributes_resp_msg_v01, file_status)
};

static const uint8_t uim_get_file_attributes_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_file_attributes_ind_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_get_file_attributes_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_file_attributes_ind_msg_v01, file_attributes) - QMI_IDL_OFFSET8(uim_get_file_attributes_ind_msg_v01, file_attributes_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_file_attributes_ind_msg_v01, file_attributes),
  QMI_IDL_TYPE88(0, 10),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_file_attributes_ind_msg_v01, file_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_file_attributes_ind_msg_v01, file_status_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_get_file_attributes_ind_msg_v01, file_status)
};

static const uint8_t uim_refresh_register_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_register_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_register_req_msg_v01, register_refresh),
  QMI_IDL_TYPE88(0, 12)
};

static const uint8_t uim_refresh_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_refresh_ok_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_ok_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_refresh_ok_req_msg_v01, ok_to_refresh)
};

static const uint8_t uim_refresh_ok_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_ok_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_refresh_complete_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_complete_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_refresh_complete_req_msg_v01, refresh_success)
};

static const uint8_t uim_refresh_complete_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_complete_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_refresh_get_last_event_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_get_last_event_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t uim_refresh_get_last_event_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_get_last_event_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_refresh_get_last_event_resp_msg_v01, refresh_event) - QMI_IDL_OFFSET8(uim_refresh_get_last_event_resp_msg_v01, refresh_event_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_get_last_event_resp_msg_v01, refresh_event),
  QMI_IDL_TYPE88(0, 13)
};

static const uint8_t uim_set_pin_protection_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_req_msg_v01, set_pin_protection),
  QMI_IDL_TYPE88(0, 14),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_set_pin_protection_req_msg_v01, pin_key_ref_id) - QMI_IDL_OFFSET8(uim_set_pin_protection_req_msg_v01, pin_key_ref_id_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_set_pin_protection_req_msg_v01, pin_key_ref_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_set_pin_protection_req_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_set_pin_protection_req_msg_v01, indication_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_req_msg_v01, indication_token)
};

static const uint8_t uim_set_pin_protection_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_set_pin_protection_resp_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_set_pin_protection_resp_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_resp_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_set_pin_protection_resp_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_set_pin_protection_resp_msg_v01, encrypted_pin1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_resp_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_set_pin_protection_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_set_pin_protection_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_set_pin_protection_resp_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_set_pin_protection_resp_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_set_pin_protection_resp_msg_v01, card_result_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_set_pin_protection_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_set_pin_protection_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_set_pin_protection_ind_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_set_pin_protection_ind_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_ind_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_set_pin_protection_ind_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_set_pin_protection_ind_msg_v01, encrypted_pin1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_pin_protection_ind_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_set_pin_protection_ind_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_set_pin_protection_ind_msg_v01, card_result_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_set_pin_protection_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_verify_pin_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_req_msg_v01, verify_pin),
  QMI_IDL_TYPE88(0, 17),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_verify_pin_req_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_verify_pin_req_msg_v01, encrypted_pin1_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_req_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_req_msg_v01, pin_key_ref_id) - QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_req_msg_v01, pin_key_ref_id_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(uim_verify_pin_req_msg_v01, pin_key_ref_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_req_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_req_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_verify_pin_req_msg_v01, indication_token)
};

static const uint8_t uim_verify_pin_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_verify_pin_resp_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_verify_pin_resp_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_resp_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_verify_pin_resp_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_verify_pin_resp_msg_v01, encrypted_pin1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_resp_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_verify_pin_resp_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_resp_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_resp_msg_v01, card_result_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_verify_pin_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_verify_pin_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_verify_pin_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_verify_pin_ind_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_verify_pin_ind_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_ind_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_verify_pin_ind_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_verify_pin_ind_msg_v01, encrypted_pin1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_pin_ind_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_ind_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_verify_pin_ind_msg_v01, card_result_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_verify_pin_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_unblock_pin_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_unblock_pin_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_unblock_pin_req_msg_v01, unblock_pin),
  QMI_IDL_TYPE88(0, 18),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_unblock_pin_req_msg_v01, pin_key_ref_id) - QMI_IDL_OFFSET8(uim_unblock_pin_req_msg_v01, pin_key_ref_id_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_unblock_pin_req_msg_v01, pin_key_ref_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_unblock_pin_req_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_unblock_pin_req_msg_v01, indication_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_unblock_pin_req_msg_v01, indication_token)
};

static const uint8_t uim_unblock_pin_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_unblock_pin_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_unblock_pin_resp_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_unblock_pin_resp_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_unblock_pin_resp_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_unblock_pin_resp_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_unblock_pin_resp_msg_v01, encrypted_pin1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_unblock_pin_resp_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_unblock_pin_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_unblock_pin_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_unblock_pin_resp_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_unblock_pin_resp_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_unblock_pin_resp_msg_v01, card_result_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_unblock_pin_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_unblock_pin_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_unblock_pin_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_unblock_pin_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_unblock_pin_ind_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_unblock_pin_ind_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_unblock_pin_ind_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_unblock_pin_ind_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_unblock_pin_ind_msg_v01, encrypted_pin1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_unblock_pin_ind_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_unblock_pin_ind_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_unblock_pin_ind_msg_v01, card_result_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_unblock_pin_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_change_pin_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_pin_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_pin_req_msg_v01, change_pin),
  QMI_IDL_TYPE88(0, 19),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_change_pin_req_msg_v01, pin_key_ref_id) - QMI_IDL_OFFSET8(uim_change_pin_req_msg_v01, pin_key_ref_id_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_change_pin_req_msg_v01, pin_key_ref_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_change_pin_req_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_change_pin_req_msg_v01, indication_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_change_pin_req_msg_v01, indication_token)
};

static const uint8_t uim_change_pin_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_pin_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_change_pin_resp_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_change_pin_resp_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_pin_resp_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_change_pin_resp_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_change_pin_resp_msg_v01, encrypted_pin1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_pin_resp_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_change_pin_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_change_pin_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_change_pin_resp_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_change_pin_resp_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_change_pin_resp_msg_v01, card_result_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_change_pin_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_change_pin_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_pin_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_change_pin_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_change_pin_ind_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_change_pin_ind_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_pin_ind_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 15),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_change_pin_ind_msg_v01, encrypted_pin1) - QMI_IDL_OFFSET8(uim_change_pin_ind_msg_v01, encrypted_pin1_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_pin_ind_msg_v01, encrypted_pin1),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_change_pin_ind_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_change_pin_ind_msg_v01, card_result_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_change_pin_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_depersonalization_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_depersonalization_req_msg_v01, depersonalization),
  QMI_IDL_TYPE88(0, 20),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_depersonalization_req_msg_v01, slot) - QMI_IDL_OFFSET8(uim_depersonalization_req_msg_v01, slot_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_depersonalization_req_msg_v01, slot)
};

static const uint8_t uim_depersonalization_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_depersonalization_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_depersonalization_resp_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_depersonalization_resp_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_depersonalization_resp_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 21)
};

static const uint8_t uim_power_down_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_power_down_req_msg_v01, slot)
};

static const uint8_t uim_power_down_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_power_down_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_power_up_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_power_up_req_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_power_up_req_msg_v01, ignore_hotswap_switch) - QMI_IDL_OFFSET8(uim_power_up_req_msg_v01, ignore_hotswap_switch_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_power_up_req_msg_v01, ignore_hotswap_switch)
};

static const uint8_t uim_power_up_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_power_up_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_get_card_status_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_card_status_req_msg_v01, extended_card_status) - QMI_IDL_OFFSET8(uim_get_card_status_req_msg_v01, extended_card_status_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_card_status_req_msg_v01, extended_card_status)
};

static const uint8_t uim_get_card_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_card_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_card_status_resp_msg_v01, card_status) - QMI_IDL_OFFSET8(uim_get_card_status_resp_msg_v01, card_status_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_card_status_resp_msg_v01, card_status),
  QMI_IDL_TYPE88(0, 25),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, hot_swap) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, hot_swap_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(uim_get_card_status_resp_msg_v01, hot_swap),
  QMI_UIM_CARDS_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, hot_swap) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, hot_swap_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, card_status_validity) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, card_status_validity_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_get_card_status_resp_msg_v01, card_status_validity),
  QMI_UIM_CARDS_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, card_status_validity) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, card_status_validity_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, extended_card_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, extended_card_status_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_get_card_status_resp_msg_v01, extended_card_status),
  QMI_IDL_TYPE88(0, 27),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, extended_hot_swap) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, extended_hot_swap_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(uim_get_card_status_resp_msg_v01, extended_hot_swap),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, extended_hot_swap) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, extended_hot_swap_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, sim_busy_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, sim_busy_status_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_get_card_status_resp_msg_v01, sim_busy_status),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, sim_busy_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_card_status_resp_msg_v01, sim_busy_status_len)
};

static const uint8_t uim_event_reg_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_event_reg_req_msg_v01, event_mask)
};

static const uint8_t uim_event_reg_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_event_reg_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_event_reg_resp_msg_v01, event_mask) - QMI_IDL_OFFSET8(uim_event_reg_resp_msg_v01, event_mask_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_event_reg_resp_msg_v01, event_mask)
};

static const uint8_t uim_status_change_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_status_change_ind_msg_v01, card_status) - QMI_IDL_OFFSET8(uim_status_change_ind_msg_v01, card_status_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_status_change_ind_msg_v01, card_status),
  QMI_IDL_TYPE88(0, 25),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_status_change_ind_msg_v01, card_status_validity) - QMI_IDL_OFFSET16RELATIVE(uim_status_change_ind_msg_v01, card_status_validity_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_status_change_ind_msg_v01, card_status_validity),
  QMI_UIM_CARDS_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_status_change_ind_msg_v01, card_status_validity) - QMI_IDL_OFFSET16RELATIVE(uim_status_change_ind_msg_v01, card_status_validity_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_status_change_ind_msg_v01, extended_card_status) - QMI_IDL_OFFSET16RELATIVE(uim_status_change_ind_msg_v01, extended_card_status_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_status_change_ind_msg_v01, extended_card_status),
  QMI_IDL_TYPE88(0, 27)
};

static const uint8_t uim_refresh_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_refresh_ind_msg_v01, refresh_event) - QMI_IDL_OFFSET8(uim_refresh_ind_msg_v01, refresh_event_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_ind_msg_v01, refresh_event),
  QMI_IDL_TYPE88(0, 13)
};

static const uint8_t uim_authenticate_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_authenticate_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_authenticate_req_msg_v01, authentication_data),
  QMI_IDL_TYPE88(0, 28),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_authenticate_req_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_authenticate_req_msg_v01, indication_token_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_authenticate_req_msg_v01, indication_token)
};

static const uint8_t uim_authenticate_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, content) - QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, content_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, content),
  ((QMI_UIM_AUTHENTICATE_RESULT_MAX_V01) & 0xFF), ((QMI_UIM_AUTHENTICATE_RESULT_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, content) - QMI_IDL_OFFSET8(uim_authenticate_resp_msg_v01, content_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_authenticate_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_authenticate_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_authenticate_resp_msg_v01, indication_token)
};

static const uint8_t uim_authenticate_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, content) - QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, content_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, content),
  ((QMI_UIM_AUTHENTICATE_RESULT_MAX_V01) & 0xFF), ((QMI_UIM_AUTHENTICATE_RESULT_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, content) - QMI_IDL_OFFSET8(uim_authenticate_ind_msg_v01, content_len)
};

static const uint8_t uim_close_session_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_close_session_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_close_session_req_msg_v01, terminate_application) - QMI_IDL_OFFSET8(uim_close_session_req_msg_v01, terminate_application_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_close_session_req_msg_v01, terminate_application)
};

static const uint8_t uim_close_session_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_close_session_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_get_service_status_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_service_status_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_get_service_status_req_msg_v01, mask),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_service_status_req_msg_v01, service_number) - QMI_IDL_OFFSET8(uim_get_service_status_req_msg_v01, service_number_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_service_status_req_msg_v01, service_number)
};

static const uint8_t uim_get_service_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, fdn_status) - QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, fdn_status_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, fdn_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, hidden_key_status) - QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, hidden_key_status_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, hidden_key_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, index) - QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, index_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, index),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, esn_status) - QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, esn_status_valid)),
  0x13,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, esn_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, acl_status) - QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, acl_status_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, acl_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, service_status) - QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, service_status_valid)),
  0x15,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_service_status_resp_msg_v01, service_status)
};

static const uint8_t uim_set_service_status_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_service_status_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_set_service_status_req_msg_v01, fdn_status) - QMI_IDL_OFFSET8(uim_set_service_status_req_msg_v01, fdn_status_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_set_service_status_req_msg_v01, fdn_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_set_service_status_req_msg_v01, enable_acl) - QMI_IDL_OFFSET8(uim_set_service_status_req_msg_v01, enable_acl_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_set_service_status_req_msg_v01, enable_acl)
};

static const uint8_t uim_set_service_status_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_service_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_change_provisioning_session_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_provisioning_session_req_msg_v01, session_change),
  QMI_IDL_TYPE88(0, 29),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_change_provisioning_session_req_msg_v01, application_information) - QMI_IDL_OFFSET8(uim_change_provisioning_session_req_msg_v01, application_information_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_provisioning_session_req_msg_v01, application_information),
  QMI_IDL_TYPE88(0, 30)
};

static const uint8_t uim_change_provisioning_session_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_change_provisioning_session_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_get_label_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_label_req_msg_v01, application_information),
  QMI_IDL_TYPE88(0, 30)
};

static const uint8_t uim_get_label_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_label_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_label_resp_msg_v01, label) - QMI_IDL_OFFSET8(uim_get_label_resp_msg_v01, label_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_label_resp_msg_v01, label),
  QMI_UIM_LABEL_MAX_V01,
  QMI_IDL_OFFSET8(uim_get_label_resp_msg_v01, label) - QMI_IDL_OFFSET8(uim_get_label_resp_msg_v01, label_len)
};

static const uint8_t uim_get_configuration_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_configuration_req_msg_v01, configuration_mask) - QMI_IDL_OFFSET8(uim_get_configuration_req_msg_v01, configuration_mask_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_get_configuration_req_msg_v01, configuration_mask)
};

static const uint8_t uim_get_configuration_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, automatic_selection) - QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, automatic_selection_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, automatic_selection),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, personalization_status) - QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, personalization_status_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, personalization_status),
  QMI_UIM_PERSO_FEATURE_MAX_V01,
  QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, personalization_status) - QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, personalization_status_len),
  QMI_IDL_TYPE88(0, 31),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, halt_subscription) - QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, halt_subscription_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, halt_subscription),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, extended_personalization_status) - QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, extended_personalization_status_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, extended_personalization_status),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, extended_personalization_status) - QMI_IDL_OFFSET8(uim_get_configuration_resp_msg_v01, extended_personalization_status_len),
  QMI_IDL_TYPE88(0, 32),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, usb_uicc_supported) - QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, usb_uicc_supported_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_get_configuration_resp_msg_v01, usb_uicc_supported),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, sap_client_supported) - QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, sap_client_supported_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_get_configuration_resp_msg_v01, sap_client_supported),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, temporary_depersonalization_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, temporary_depersonalization_status_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_get_configuration_resp_msg_v01, temporary_depersonalization_status),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, temporary_depersonalization_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, temporary_depersonalization_status_len),
  QMI_IDL_TYPE88(0, 51),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, perso_reactivation_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, perso_reactivation_status_valid)),
  0x17,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_get_configuration_resp_msg_v01, perso_reactivation_status),
  QMI_UIM_EXTENDED_PERSO_FEATURE_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, perso_reactivation_status) - QMI_IDL_OFFSET16RELATIVE(uim_get_configuration_resp_msg_v01, perso_reactivation_status_len),
  QMI_IDL_TYPE88(0, 52)
};

static const uint8_t uim_send_apdu_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_send_apdu_req_msg_v01, slot),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_send_apdu_req_msg_v01, apdu),
  ((QMI_UIM_APDU_DATA_MAX_V01) & 0xFF), ((QMI_UIM_APDU_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_send_apdu_req_msg_v01, apdu) - QMI_IDL_OFFSET8(uim_send_apdu_req_msg_v01, apdu_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_send_apdu_req_msg_v01, channel_id) - QMI_IDL_OFFSET16RELATIVE(uim_send_apdu_req_msg_v01, channel_id_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_send_apdu_req_msg_v01, channel_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_send_apdu_req_msg_v01, procedure_bytes) - QMI_IDL_OFFSET16RELATIVE(uim_send_apdu_req_msg_v01, procedure_bytes_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(uim_send_apdu_req_msg_v01, procedure_bytes)
};

static const uint8_t uim_send_apdu_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_send_apdu_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_send_apdu_resp_msg_v01, apdu) - QMI_IDL_OFFSET8(uim_send_apdu_resp_msg_v01, apdu_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_send_apdu_resp_msg_v01, apdu),
  ((QMI_UIM_APDU_DATA_MAX_V01) & 0xFF), ((QMI_UIM_APDU_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_send_apdu_resp_msg_v01, apdu) - QMI_IDL_OFFSET8(uim_send_apdu_resp_msg_v01, apdu_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_send_apdu_resp_msg_v01, long_response) - QMI_IDL_OFFSET16RELATIVE(uim_send_apdu_resp_msg_v01, long_response_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_send_apdu_resp_msg_v01, long_response),
  QMI_IDL_TYPE88(0, 33)
};

static const uint8_t uim_send_apdu_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_send_apdu_ind_msg_v01, long_response_chunk),
  QMI_IDL_TYPE88(0, 34)
};

static const uint8_t uim_sap_connection_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, sap_connect),
  QMI_IDL_TYPE88(0, 35),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, disconnect_mode) - QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, disconnect_mode_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, disconnect_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, intermediate_get_response) - QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, intermediate_get_response_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, intermediate_get_response),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, connection_condition) - QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, connection_condition_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_sap_connection_req_msg_v01, connection_condition)
};

static const uint8_t uim_sap_connection_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_sap_connection_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_sap_connection_resp_msg_v01, sap_state) - QMI_IDL_OFFSET8(uim_sap_connection_resp_msg_v01, sap_state_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_sap_connection_resp_msg_v01, sap_state)
};

static const uint8_t uim_sap_request_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_sap_request_req_msg_v01, sap_request),
  QMI_IDL_TYPE88(0, 36),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_sap_request_req_msg_v01, apdu) - QMI_IDL_OFFSET8(uim_sap_request_req_msg_v01, apdu_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_sap_request_req_msg_v01, apdu),
  ((QMI_UIM_APDU_DATA_MAX_V01) & 0xFF), ((QMI_UIM_APDU_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_sap_request_req_msg_v01, apdu) - QMI_IDL_OFFSET8(uim_sap_request_req_msg_v01, apdu_len)
};

static const uint8_t uim_sap_request_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_sap_request_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_sap_request_resp_msg_v01, ATR_value) - QMI_IDL_OFFSET8(uim_sap_request_resp_msg_v01, ATR_value_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_sap_request_resp_msg_v01, ATR_value),
  QMI_UIM_ATR_DATA_MAX_V01,
  QMI_IDL_OFFSET8(uim_sap_request_resp_msg_v01, ATR_value) - QMI_IDL_OFFSET8(uim_sap_request_resp_msg_v01, ATR_value_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_sap_request_resp_msg_v01, apdu) - QMI_IDL_OFFSET16RELATIVE(uim_sap_request_resp_msg_v01, apdu_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_sap_request_resp_msg_v01, apdu),
  ((QMI_UIM_APDU_DATA_MAX_V01) & 0xFF), ((QMI_UIM_APDU_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(uim_sap_request_resp_msg_v01, apdu) - QMI_IDL_OFFSET16RELATIVE(uim_sap_request_resp_msg_v01, apdu_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_sap_request_resp_msg_v01, status) - QMI_IDL_OFFSET16RELATIVE(uim_sap_request_resp_msg_v01, status_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_sap_request_resp_msg_v01, status),
  QMI_UIM_CARD_READER_DATA_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_sap_request_resp_msg_v01, status) - QMI_IDL_OFFSET16RELATIVE(uim_sap_request_resp_msg_v01, status_len)
};

static const uint8_t uim_sap_connection_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_sap_connection_ind_msg_v01, sap_connection_event) - QMI_IDL_OFFSET8(uim_sap_connection_ind_msg_v01, sap_connection_event_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_sap_connection_ind_msg_v01, sap_connection_event),
  QMI_IDL_TYPE88(0, 37)
};

static const uint8_t uim_logical_channel_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, aid) - QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, aid_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, aid),
  QMI_UIM_AID_MAX_V01,
  QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, aid) - QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, aid_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, channel_id) - QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, channel_id_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, channel_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, file_control_information) - QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, file_control_information_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, file_control_information),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, terminate_application) - QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, terminate_application_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_logical_channel_req_msg_v01, terminate_application)
};

static const uint8_t uim_logical_channel_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, channel_id) - QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, channel_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, channel_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, card_result_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, select_response) - QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, select_response_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, select_response),
  QMI_UIM_SELECT_RESPONSE_MAX_V01,
  QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, select_response) - QMI_IDL_OFFSET8(uim_logical_channel_resp_msg_v01, select_response_len)
};

static const uint8_t uim_subscription_ok_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_subscription_ok_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_subscription_ok_req_msg_v01, ok_for_subscription),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_subscription_ok_req_msg_v01, encrypted_imsi) - QMI_IDL_OFFSET8(uim_subscription_ok_req_msg_v01, encrypted_imsi_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_subscription_ok_req_msg_v01, encrypted_imsi),
  QMI_UIM_ENCRYPTED_IMSI_MAX_V01,
  QMI_IDL_OFFSET8(uim_subscription_ok_req_msg_v01, encrypted_imsi) - QMI_IDL_OFFSET8(uim_subscription_ok_req_msg_v01, encrypted_imsi_len)
};

static const uint8_t uim_subscription_ok_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_subscription_ok_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_get_atr_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_get_atr_req_msg_v01, slot)
};

static const uint8_t uim_get_atr_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_atr_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_atr_resp_msg_v01, atr_value) - QMI_IDL_OFFSET8(uim_get_atr_resp_msg_v01, atr_value_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_atr_resp_msg_v01, atr_value),
  QMI_UIM_ATR_MAX_V01,
  QMI_IDL_OFFSET8(uim_get_atr_resp_msg_v01, atr_value) - QMI_IDL_OFFSET8(uim_get_atr_resp_msg_v01, atr_value_len)
};

static const uint8_t uim_open_logical_channel_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, aid) - QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, aid_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, aid),
  QMI_UIM_AID_MAX_V01,
  QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, aid) - QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, aid_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, file_control_information) - QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, file_control_information_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_open_logical_channel_req_msg_v01, file_control_information)
};

static const uint8_t uim_open_logical_channel_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, channel_id) - QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, channel_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, channel_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, card_result_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, select_response) - QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, select_response_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, select_response),
  QMI_UIM_SELECT_RESPONSE_MAX_V01,
  QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, select_response) - QMI_IDL_OFFSET8(uim_open_logical_channel_resp_msg_v01, select_response_len)
};

static const uint8_t uim_session_closed_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, aid) - QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, aid_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, aid),
  QMI_UIM_AID_MAX_V01,
  QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, aid) - QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, aid_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, channel_id) - QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, channel_id_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, channel_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, provisioning_session) - QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, provisioning_session_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, provisioning_session),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, cause) - QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, cause_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, cause),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, file_id) - QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, file_id_valid)),
  0x14,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(uim_session_closed_ind_msg_v01, file_id)
};

static const uint8_t uim_refresh_register_all_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_register_all_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_refresh_register_all_req_msg_v01, register_for_refresh),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_refresh_register_all_req_msg_v01, vote_for_init) - QMI_IDL_OFFSET8(uim_refresh_register_all_req_msg_v01, vote_for_init_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_refresh_register_all_req_msg_v01, vote_for_init)
};

static const uint8_t uim_refresh_register_all_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_refresh_register_all_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_set_file_status_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_file_status_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_file_status_req_msg_v01, file_id),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_set_file_status_req_msg_v01, file_status)
};

static const uint8_t uim_set_file_status_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_file_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_switch_slot_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_switch_slot_req_msg_v01, logical_slot),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_switch_slot_req_msg_v01, physical_slot)
};

static const uint8_t uim_switch_slot_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_switch_slot_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * uim_get_slots_status_req_msg is empty
 * static const uint8_t uim_get_slots_status_req_msg_data_v01[] = {
 * };
 */

static const uint8_t uim_get_slots_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_slots_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_slots_status_resp_msg_v01, physical_slot_status) - QMI_IDL_OFFSET8(uim_get_slots_status_resp_msg_v01, physical_slot_status_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_slots_status_resp_msg_v01, physical_slot_status),
  QMI_UIM_PHYSICAL_SLOTS_MAX_V01,
  QMI_IDL_OFFSET8(uim_get_slots_status_resp_msg_v01, physical_slot_status) - QMI_IDL_OFFSET8(uim_get_slots_status_resp_msg_v01, physical_slot_status_len),
  QMI_IDL_TYPE88(0, 38)
};

static const uint8_t uim_slot_status_change_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_slot_status_change_ind_msg_v01, physical_slot_status) - QMI_IDL_OFFSET8(uim_slot_status_change_ind_msg_v01, physical_slot_status_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_slot_status_change_ind_msg_v01, physical_slot_status),
  QMI_UIM_PHYSICAL_SLOTS_MAX_V01,
  QMI_IDL_OFFSET8(uim_slot_status_change_ind_msg_v01, physical_slot_status) - QMI_IDL_OFFSET8(uim_slot_status_change_ind_msg_v01, physical_slot_status_len),
  QMI_IDL_TYPE88(0, 38)
};

static const uint8_t uim_read_transparent_long_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_read_transparent_long_ind_msg_v01, long_response_chunk),
  QMI_IDL_TYPE88(0, 39),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_long_ind_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_long_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_long_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_long_ind_msg_v01, encryption) - QMI_IDL_OFFSET16RELATIVE(uim_read_transparent_long_ind_msg_v01, encryption_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_read_transparent_long_ind_msg_v01, encryption)
};

static const uint8_t uim_sim_busy_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_sim_busy_status_ind_msg_v01, sim_busy_status) - QMI_IDL_OFFSET8(uim_sim_busy_status_ind_msg_v01, sim_busy_status_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_sim_busy_status_ind_msg_v01, sim_busy_status),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET8(uim_sim_busy_status_ind_msg_v01, sim_busy_status) - QMI_IDL_OFFSET8(uim_sim_busy_status_ind_msg_v01, sim_busy_status_len)
};

/*
 * uim_get_plmn_name_table_info_req_msg is empty
 * static const uint8_t uim_get_plmn_name_table_info_req_msg_data_v01[] = {
 * };
 */

static const uint8_t uim_get_plmn_name_table_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_plmn_name_table_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_plmn_name_table_info_resp_msg_v01, release_date) - QMI_IDL_OFFSET8(uim_get_plmn_name_table_info_resp_msg_v01, release_date_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_plmn_name_table_info_resp_msg_v01, release_date),
  QMI_IDL_TYPE88(0, 40),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_plmn_name_table_info_resp_msg_v01, table_source) - QMI_IDL_OFFSET8(uim_get_plmn_name_table_info_resp_msg_v01, table_source_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_get_plmn_name_table_info_resp_msg_v01, table_source)
};

static const uint8_t uim_personalization_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_personalization_req_msg_v01, ck_value),
  QMI_UIM_CK_MAX_V01,
  QMI_IDL_OFFSET8(uim_personalization_req_msg_v01, ck_value) - QMI_IDL_OFFSET8(uim_personalization_req_msg_v01, ck_value_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_personalization_req_msg_v01, feature_gw_network_perso) - QMI_IDL_OFFSET8(uim_personalization_req_msg_v01, feature_gw_network_perso_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_personalization_req_msg_v01, feature_gw_network_perso),
  QMI_UIM_PERSO_NUM_NW_MAX_V01,
  QMI_IDL_OFFSET8(uim_personalization_req_msg_v01, feature_gw_network_perso) - QMI_IDL_OFFSET8(uim_personalization_req_msg_v01, feature_gw_network_perso_len),
  QMI_IDL_TYPE88(0, 41),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_network_subset_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_network_subset_perso_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, feature_gw_network_subset_perso),
  QMI_UIM_PERSO_NUM_NS_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_network_subset_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_network_subset_perso_len),
  QMI_IDL_TYPE88(0, 42),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_sp_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_sp_perso_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, feature_gw_sp_perso),
  QMI_UIM_PERSO_NUM_GW_SP_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_sp_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_sp_perso_len),
  QMI_IDL_TYPE88(0, 43),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_corporate_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_corporate_perso_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, feature_gw_corporate_perso),
  QMI_UIM_PERSO_NUM_GW_CP_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_corporate_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_corporate_perso_len),
  QMI_IDL_TYPE88(0, 44),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_sim_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_sim_perso_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, feature_gw_sim_perso),
  QMI_UIM_PERSO_NUM_SIM_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_sim_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_gw_sim_perso_len),
  QMI_IDL_TYPE88(0, 45),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_network1_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_network1_perso_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, feature_1x_network1_perso),
  QMI_UIM_PERSO_NUM_NW_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_network1_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_network1_perso_len),
  QMI_IDL_TYPE88(0, 41),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_network2_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_network2_perso_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, feature_1x_network2_perso),
  QMI_UIM_PERSO_NUM_1X_NW2_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_network2_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_network2_perso_len),
  QMI_IDL_TYPE88(0, 46),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_ruim_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_ruim_perso_valid)),
  0x17,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, feature_1x_ruim_perso),
  QMI_UIM_PERSO_NUM_SIM_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_ruim_perso) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, feature_1x_ruim_perso_len),
  QMI_IDL_TYPE88(0, 45),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, slot) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, slot_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, num_iterations) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, num_iterations_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, num_iterations),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, num_retries) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, num_retries_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, num_retries),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, reactivate_feature) - QMI_IDL_OFFSET16RELATIVE(uim_personalization_req_msg_v01, reactivate_feature_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(uim_personalization_req_msg_v01, reactivate_feature)
};

static const uint8_t uim_personalization_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_personalization_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_personalization_resp_msg_v01, retries_left) - QMI_IDL_OFFSET8(uim_personalization_resp_msg_v01, retries_left_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_personalization_resp_msg_v01, retries_left),
  QMI_IDL_TYPE88(0, 21)
};

static const uint8_t uim_increase_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_increase_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_increase_req_msg_v01, file_id),
  QMI_IDL_TYPE88(0, 1),

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_increase_req_msg_v01, increase_data),
  QMI_UIM_INCREASE_DATA_MAX_V01,
  QMI_IDL_OFFSET8(uim_increase_req_msg_v01, increase_data) - QMI_IDL_OFFSET8(uim_increase_req_msg_v01, increase_data_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_increase_req_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_increase_req_msg_v01, indication_token_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_increase_req_msg_v01, indication_token)
};

static const uint8_t uim_increase_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, increase_result) - QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, increase_result_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, increase_result),
  QMI_UIM_INCREASE_DATA_MAX_V01,
  QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, increase_result) - QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, increase_result_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, indication_token) - QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_increase_resp_msg_v01, indication_token)
};

static const uint8_t uim_increase_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, increase_result) - QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, increase_result_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, increase_result),
  QMI_UIM_INCREASE_DATA_MAX_V01,
  QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, increase_result) - QMI_IDL_OFFSET8(uim_increase_ind_msg_v01, increase_result_len)
};

static const uint8_t uim_recovery_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_recovery_req_msg_v01, slot)
};

static const uint8_t uim_recovery_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_recovery_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_reselect_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_reselect_req_msg_v01, slot),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_reselect_req_msg_v01, channel_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_reselect_req_msg_v01, select_mode)
};

static const uint8_t uim_reselect_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, select_response) - QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, select_response_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, select_response),
  QMI_UIM_SELECT_RESPONSE_MAX_V01,
  QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, select_response) - QMI_IDL_OFFSET8(uim_reselect_resp_msg_v01, select_response_len)
};

static const uint8_t uim_recovery_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_recovery_ind_msg_v01, slot)
};

static const uint8_t uim_send_status_cmd_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_send_status_cmd_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_send_status_cmd_req_msg_v01, status_cmd_mode) - QMI_IDL_OFFSET8(uim_send_status_cmd_req_msg_v01, status_cmd_mode_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_send_status_cmd_req_msg_v01, status_cmd_mode),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_send_status_cmd_req_msg_v01, status_cmd_resp) - QMI_IDL_OFFSET8(uim_send_status_cmd_req_msg_v01, status_cmd_resp_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_send_status_cmd_req_msg_v01, status_cmd_resp)
};

static const uint8_t uim_send_status_cmd_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_send_status_cmd_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_send_status_cmd_resp_msg_v01, status_response) - QMI_IDL_OFFSET8(uim_send_status_cmd_resp_msg_v01, status_response_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_send_status_cmd_resp_msg_v01, status_response),
  QMI_UIM_SELECT_RESPONSE_MAX_V01,
  QMI_IDL_OFFSET8(uim_send_status_cmd_resp_msg_v01, status_response) - QMI_IDL_OFFSET8(uim_send_status_cmd_resp_msg_v01, status_response_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_send_status_cmd_resp_msg_v01, card_result) - QMI_IDL_OFFSET16RELATIVE(uim_send_status_cmd_resp_msg_v01, card_result_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_send_status_cmd_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t uim_get_sim_profile_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_get_sim_profile_req_msg_v01, slot)
};

static const uint8_t uim_get_sim_profile_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, profile_info) - QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, profile_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, profile_info),
  QMI_UIM_PROFILES_MAX_V01,
  QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, profile_info) - QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, profile_info_len),
  QMI_IDL_TYPE88(0, 47),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, active_profile_info) - QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, active_profile_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_sim_profile_resp_msg_v01, active_profile_info),
  QMI_IDL_TYPE88(0, 47)
};

static const uint8_t uim_set_sim_profile_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_set_sim_profile_req_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_set_sim_profile_req_msg_v01, profile_id)
};

static const uint8_t uim_set_sim_profile_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_set_sim_profile_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_supply_voltage_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_supply_voltage_req_msg_v01, slot)
};

static const uint8_t uim_supply_voltage_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_supply_voltage_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_supply_voltage_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_supply_voltage_ind_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_supply_voltage_ind_msg_v01, vcc_command)
};

static const uint8_t uim_card_activation_status_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(uim_card_activation_status_ind_msg_v01, slot),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_card_activation_status_ind_msg_v01, card_activation_status)
};

static const uint8_t uim_depersonalization_secure_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_depersonalization_secure_req_msg_v01, encrypted_depersonalization_data),
  ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_depersonalization_secure_req_msg_v01, encrypted_depersonalization_data) - QMI_IDL_OFFSET8(uim_depersonalization_secure_req_msg_v01, encrypted_depersonalization_data_len)
};

static const uint8_t uim_depersonalization_secure_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_depersonalization_secure_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_depersonalization_secure_resp_msg_v01, encrypted_resp) - QMI_IDL_OFFSET8(uim_depersonalization_secure_resp_msg_v01, encrypted_resp_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_depersonalization_secure_resp_msg_v01, encrypted_resp),
  ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_depersonalization_secure_resp_msg_v01, encrypted_resp) - QMI_IDL_OFFSET8(uim_depersonalization_secure_resp_msg_v01, encrypted_resp_len)
};

static const uint8_t uim_personalization_secure_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_personalization_secure_req_msg_v01, encrypted_personalization_data),
  ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_personalization_secure_req_msg_v01, encrypted_personalization_data) - QMI_IDL_OFFSET8(uim_personalization_secure_req_msg_v01, encrypted_personalization_data_len)
};

static const uint8_t uim_personalization_secure_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_personalization_secure_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_personalization_secure_resp_msg_v01, encrypted_resp) - QMI_IDL_OFFSET8(uim_personalization_secure_resp_msg_v01, encrypted_resp_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_personalization_secure_resp_msg_v01, encrypted_resp),
  ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_personalization_secure_resp_msg_v01, encrypted_resp) - QMI_IDL_OFFSET8(uim_personalization_secure_resp_msg_v01, encrypted_resp_len)
};

static const uint8_t uim_emergency_only_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_emergency_only_req_msg_v01, encrypted_emergency_data),
  ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_emergency_only_req_msg_v01, encrypted_emergency_data) - QMI_IDL_OFFSET8(uim_emergency_only_req_msg_v01, encrypted_emergency_data_len)
};

static const uint8_t uim_emergency_only_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_emergency_only_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_emergency_only_resp_msg_v01, encrypted_resp) - QMI_IDL_OFFSET8(uim_emergency_only_resp_msg_v01, encrypted_resp_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_emergency_only_resp_msg_v01, encrypted_resp),
  ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_emergency_only_resp_msg_v01, encrypted_resp) - QMI_IDL_OFFSET8(uim_emergency_only_resp_msg_v01, encrypted_resp_len)
};

static const uint8_t uim_simlock_configuration_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_simlock_configuration_req_msg_v01, encrypted_simlock_config_data),
  ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_simlock_configuration_req_msg_v01, encrypted_simlock_config_data) - QMI_IDL_OFFSET8(uim_simlock_configuration_req_msg_v01, encrypted_simlock_config_data_len)
};

static const uint8_t uim_simlock_configuration_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_simlock_configuration_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_simlock_configuration_resp_msg_v01, encrypted_resp) - QMI_IDL_OFFSET8(uim_simlock_configuration_resp_msg_v01, encrypted_resp_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_simlock_configuration_resp_msg_v01, encrypted_resp),
  ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_RESPONSE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_simlock_configuration_resp_msg_v01, encrypted_resp) - QMI_IDL_OFFSET8(uim_simlock_configuration_resp_msg_v01, encrypted_resp_len)
};

static const uint8_t uim_simlock_configuration_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_simlock_configuration_ind_msg_v01, encrypted_simlock_config_ind),
  ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_REQUEST_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_simlock_configuration_ind_msg_v01, encrypted_simlock_config_ind) - QMI_IDL_OFFSET8(uim_simlock_configuration_ind_msg_v01, encrypted_simlock_config_ind_len)
};

static const uint8_t uim_gba_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_gba_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_gba_req_msg_v01, naf_id),
  QMI_IDL_TYPE88(0, 48),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_req_msg_v01, force_bootstrapping) - QMI_IDL_OFFSET16RELATIVE(uim_gba_req_msg_v01, force_bootstrapping_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_req_msg_v01, force_bootstrapping),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_req_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_gba_req_msg_v01, indication_token_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_req_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_req_msg_v01, apn_name) - QMI_IDL_OFFSET16RELATIVE(uim_gba_req_msg_v01, apn_name_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_req_msg_v01, apn_name),
  QMI_UIM_APN_NAME_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_gba_req_msg_v01, apn_name) - QMI_IDL_OFFSET16RELATIVE(uim_gba_req_msg_v01, apn_name_len)
};

static const uint8_t uim_gba_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, ks_naf) - QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, ks_naf_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, ks_naf),
  QMI_UIM_KS_NAF_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, encrypted_ks_naf) - QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, encrypted_ks_naf_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, encrypted_ks_naf),
  ((QMI_UIM_ENCRYPTED_KS_NAF_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_KS_NAF_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, encrypted_ks_naf) - QMI_IDL_OFFSET8(uim_gba_resp_msg_v01, encrypted_ks_naf_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, btid) - QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, btid_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_resp_msg_v01, btid),
  QMI_UIM_BTID_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, btid) - QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, btid_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, lifetime) - QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, lifetime_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_resp_msg_v01, lifetime),
  QMI_UIM_KS_LIFETIME_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, lifetime) - QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, lifetime_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, impi) - QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, impi_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_resp_msg_v01, impi),
  QMI_UIM_IMPI_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, impi) - QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, impi_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_gba_resp_msg_v01, indication_token_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_resp_msg_v01, indication_token)
};

static const uint8_t uim_gba_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, ks_naf) - QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, ks_naf_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, ks_naf),
  QMI_UIM_KS_NAF_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, encrypted_ks_naf) - QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, encrypted_ks_naf_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, encrypted_ks_naf),
  ((QMI_UIM_ENCRYPTED_KS_NAF_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_KS_NAF_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, encrypted_ks_naf) - QMI_IDL_OFFSET8(uim_gba_ind_msg_v01, encrypted_ks_naf_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, btid) - QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, btid_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_ind_msg_v01, btid),
  QMI_UIM_BTID_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, btid) - QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, btid_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, lifetime) - QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, lifetime_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_ind_msg_v01, lifetime),
  QMI_UIM_KS_LIFETIME_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, lifetime) - QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, lifetime_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, impi) - QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, impi_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_gba_ind_msg_v01, impi),
  QMI_UIM_IMPI_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, impi) - QMI_IDL_OFFSET16RELATIVE(uim_gba_ind_msg_v01, impi_len)
};

static const uint8_t uim_get_gba_impi_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_gba_impi_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_gba_impi_req_msg_v01, encrypt_data) - QMI_IDL_OFFSET8(uim_get_gba_impi_req_msg_v01, encrypt_data_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_gba_impi_req_msg_v01, encrypt_data)
};

static const uint8_t uim_get_gba_impi_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_get_gba_impi_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_get_gba_impi_resp_msg_v01, impi) - QMI_IDL_OFFSET8(uim_get_gba_impi_resp_msg_v01, impi_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_get_gba_impi_resp_msg_v01, impi),
  QMI_UIM_IMPI_MAX_V01,
  QMI_IDL_OFFSET8(uim_get_gba_impi_resp_msg_v01, impi) - QMI_IDL_OFFSET8(uim_get_gba_impi_resp_msg_v01, impi_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_get_gba_impi_resp_msg_v01, encrypted_impi) - QMI_IDL_OFFSET16RELATIVE(uim_get_gba_impi_resp_msg_v01, encrypted_impi_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_get_gba_impi_resp_msg_v01, encrypted_impi),
  ((QMI_UIM_ENCRYPTED_IMPI_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_IMPI_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(uim_get_gba_impi_resp_msg_v01, encrypted_impi) - QMI_IDL_OFFSET16RELATIVE(uim_get_gba_impi_resp_msg_v01, encrypted_impi_len)
};

static const uint8_t uim_search_record_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_search_record_req_msg_v01, session_information),
  QMI_IDL_TYPE88(0, 0),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_search_record_req_msg_v01, file_id),
  QMI_IDL_TYPE88(0, 1),

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_search_record_req_msg_v01, pattern),
  QMI_UIM_CONTENT_RECORD_MAX_V01,
  QMI_IDL_OFFSET8(uim_search_record_req_msg_v01, pattern) - QMI_IDL_OFFSET8(uim_search_record_req_msg_v01, pattern_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_search_record_req_msg_v01, search_data) - QMI_IDL_OFFSET16RELATIVE(uim_search_record_req_msg_v01, search_data_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(uim_search_record_req_msg_v01, search_data),
  QMI_IDL_TYPE88(0, 49),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_search_record_req_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_search_record_req_msg_v01, indication_token_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_search_record_req_msg_v01, indication_token)
};

static const uint8_t uim_search_record_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, records) - QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, records_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, records),
  QMI_UIM_NUM_RECORD_MAX_V01,
  QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, records) - QMI_IDL_OFFSET8(uim_search_record_resp_msg_v01, records_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(uim_search_record_resp_msg_v01, indication_token) - QMI_IDL_OFFSET16RELATIVE(uim_search_record_resp_msg_v01, indication_token_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(uim_search_record_resp_msg_v01, indication_token)
};

static const uint8_t uim_search_record_ind_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, indication_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, card_result) - QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, card_result_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, card_result),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, records) - QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, records_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, records),
  QMI_UIM_NUM_RECORD_MAX_V01,
  QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, records) - QMI_IDL_OFFSET8(uim_search_record_ind_msg_v01, records_len)
};

static const uint8_t uim_remote_unlock_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_unlock_req_msg_v01, simlock_data) - QMI_IDL_OFFSET8(uim_remote_unlock_req_msg_v01, simlock_data_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_remote_unlock_req_msg_v01, simlock_data),
  ((QMI_UIM_SIMLOCK_DATA_MAX_V01) & 0xFF), ((QMI_UIM_SIMLOCK_DATA_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_remote_unlock_req_msg_v01, simlock_data) - QMI_IDL_OFFSET8(uim_remote_unlock_req_msg_v01, simlock_data_len)
};

static const uint8_t uim_remote_unlock_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_remote_unlock_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_remote_unlock_resp_msg_v01, encrypted_key) - QMI_IDL_OFFSET8(uim_remote_unlock_resp_msg_v01, encrypted_key_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_remote_unlock_resp_msg_v01, encrypted_key),
  ((QMI_UIM_ENCRYPTED_KEY_MAX_V01) & 0xFF), ((QMI_UIM_ENCRYPTED_KEY_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(uim_remote_unlock_resp_msg_v01, encrypted_key) - QMI_IDL_OFFSET8(uim_remote_unlock_resp_msg_v01, encrypted_key_len)
};

static const uint8_t uim_verify_imsi_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(uim_verify_imsi_req_msg_v01, imsi) - QMI_IDL_OFFSET8(uim_verify_imsi_req_msg_v01, imsi_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(uim_verify_imsi_req_msg_v01, imsi),
  QMI_UIM_IMSI_LEN_MAX_V01,
  QMI_IDL_OFFSET8(uim_verify_imsi_req_msg_v01, imsi) - QMI_IDL_OFFSET8(uim_verify_imsi_req_msg_v01, imsi_len)
};

static const uint8_t uim_verify_imsi_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_verify_imsi_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t uim_temporary_unlock_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(uim_temporary_unlock_ind_msg_v01, temporary_unlock_status),
  QMI_UIM_EXTENDED_CARDS_MAX_V01,
  QMI_IDL_OFFSET8(uim_temporary_unlock_ind_msg_v01, temporary_unlock_status) - QMI_IDL_OFFSET8(uim_temporary_unlock_ind_msg_v01, temporary_unlock_status_len),
  QMI_IDL_TYPE88(0, 54)
};

/* Type Table */
static const qmi_idl_type_table_entry  uim_type_table_v01[] = {
  {sizeof(uim_session_information_type_v01), uim_session_information_type_data_v01},
  {sizeof(uim_file_id_type_v01), uim_file_id_type_data_v01},
  {sizeof(uim_read_transparent_type_v01), uim_read_transparent_type_data_v01},
  {sizeof(uim_card_result_type_v01), uim_card_result_type_data_v01},
  {sizeof(uim_read_transparent_result_type_v01), uim_read_transparent_result_type_data_v01},
  {sizeof(uim_read_record_type_v01), uim_read_record_type_data_v01},
  {sizeof(uim_read_record_result_type_v01), uim_read_record_result_type_data_v01},
  {sizeof(uim_read_additional_record_result_type_v01), uim_read_additional_record_result_type_data_v01},
  {sizeof(uim_write_transparent_type_v01), uim_write_transparent_type_data_v01},
  {sizeof(uim_write_record_type_v01), uim_write_record_type_data_v01},
  {sizeof(uim_file_attributes_type_v01), uim_file_attributes_type_data_v01},
  {sizeof(uim_register_refresh_file_id_type_v01), uim_register_refresh_file_id_type_data_v01},
  {sizeof(uim_register_refresh_type_v01), uim_register_refresh_type_data_v01},
  {sizeof(uim_refresh_event_type_v01), uim_refresh_event_type_data_v01},
  {sizeof(uim_set_pin_protection_type_v01), uim_set_pin_protection_type_data_v01},
  {sizeof(uim_pin_retries_left_type_v01), uim_pin_retries_left_type_data_v01},
  {sizeof(uim_encrypted_pin_type_v01), uim_encrypted_pin_type_data_v01},
  {sizeof(uim_verify_pin_type_v01), uim_verify_pin_type_data_v01},
  {sizeof(uim_unblock_pin_type_v01), uim_unblock_pin_type_data_v01},
  {sizeof(uim_change_pin_type_v01), uim_change_pin_type_data_v01},
  {sizeof(uim_depersonalization_type_v01), uim_depersonalization_type_data_v01},
  {sizeof(uim_perso_retries_left_type_v01), uim_perso_retries_left_type_data_v01},
  {sizeof(uim_pin_info_type_v01), uim_pin_info_type_data_v01},
  {sizeof(app_info_type_v01), app_info_type_data_v01},
  {sizeof(card_info_type_v01), card_info_type_data_v01},
  {sizeof(uim_card_status_type_v01), uim_card_status_type_data_v01},
  {sizeof(extended_card_info_type_v01), extended_card_info_type_data_v01},
  {sizeof(uim_extended_card_status_type_v01), uim_extended_card_status_type_data_v01},
  {sizeof(uim_authentication_data_type_v01), uim_authentication_data_type_data_v01},
  {sizeof(uim_session_change_type_v01), uim_session_change_type_data_v01},
  {sizeof(uim_application_information_type_v01), uim_application_information_type_data_v01},
  {sizeof(uim_personalization_status_type_v01), uim_personalization_status_type_data_v01},
  {sizeof(uim_personalization_status_other_slots_type_v01), uim_personalization_status_other_slots_type_data_v01},
  {sizeof(uim_send_apdu_long_response_type_v01), uim_send_apdu_long_response_type_data_v01},
  {sizeof(uim_send_apdu_long_response_chunk_type_v01), uim_send_apdu_long_response_chunk_type_data_v01},
  {sizeof(uim_sap_connect_type_v01), uim_sap_connect_type_data_v01},
  {sizeof(uim_sap_request_type_v01), uim_sap_request_type_data_v01},
  {sizeof(uim_sap_indication_type_v01), uim_sap_indication_type_data_v01},
  {sizeof(uim_physical_slot_status_type_v01), uim_physical_slot_status_type_data_v01},
  {sizeof(uim_read_transparent_long_response_chunk_type_v01), uim_read_transparent_long_response_chunk_type_data_v01},
  {sizeof(uim_plmn_name_release_date_info_type_v01), uim_plmn_name_release_date_info_type_data_v01},
  {sizeof(uim_network_perso_type_v01), uim_network_perso_type_data_v01},
  {sizeof(uim_gw_network_subset_perso_type_v01), uim_gw_network_subset_perso_type_data_v01},
  {sizeof(uim_gw_sp_perso_type_v01), uim_gw_sp_perso_type_data_v01},
  {sizeof(uim_gw_corporate_perso_type_v01), uim_gw_corporate_perso_type_data_v01},
  {sizeof(uim_sim_perso_type_v01), uim_sim_perso_type_data_v01},
  {sizeof(uim_1x_network_type2_perso_type_v01), uim_1x_network_type2_perso_type_data_v01},
  {sizeof(profile_info_type_v01), profile_info_type_data_v01},
  {sizeof(uim_naf_id_type_v01), uim_naf_id_type_data_v01},
  {sizeof(uim_additional_search_data_type_v01), uim_additional_search_data_type_data_v01},
  {sizeof(uim_temporary_unlock_status_type_v01), uim_temporary_unlock_status_type_data_v01},
  {sizeof(uim_temporary_depersonalization_status_type_v01), uim_temporary_depersonalization_status_type_data_v01},
  {sizeof(uim_perso_reactivation_status_type_v01), uim_perso_reactivation_status_type_data_v01},
  {sizeof(uim_temporary_unlock_feature_type_v01), uim_temporary_unlock_feature_type_data_v01},
  {sizeof(uim_temporary_unlock_slot_info_type_v01), uim_temporary_unlock_slot_info_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry uim_message_table_v01[] = {
  {sizeof(uim_reset_req_msg_v01), 0},
  {sizeof(uim_reset_resp_msg_v01), uim_reset_resp_msg_data_v01},
  {sizeof(uim_read_transparent_req_msg_v01), uim_read_transparent_req_msg_data_v01},
  {sizeof(uim_read_transparent_resp_msg_v01), uim_read_transparent_resp_msg_data_v01},
  {sizeof(uim_read_transparent_ind_msg_v01), uim_read_transparent_ind_msg_data_v01},
  {sizeof(uim_read_record_req_msg_v01), uim_read_record_req_msg_data_v01},
  {sizeof(uim_read_record_resp_msg_v01), uim_read_record_resp_msg_data_v01},
  {sizeof(uim_read_record_ind_msg_v01), uim_read_record_ind_msg_data_v01},
  {sizeof(uim_write_transparent_req_msg_v01), uim_write_transparent_req_msg_data_v01},
  {sizeof(uim_write_transparent_resp_msg_v01), uim_write_transparent_resp_msg_data_v01},
  {sizeof(uim_write_transparent_ind_msg_v01), uim_write_transparent_ind_msg_data_v01},
  {sizeof(uim_write_record_req_msg_v01), uim_write_record_req_msg_data_v01},
  {sizeof(uim_write_record_resp_msg_v01), uim_write_record_resp_msg_data_v01},
  {sizeof(uim_write_record_ind_msg_v01), uim_write_record_ind_msg_data_v01},
  {sizeof(uim_get_file_attributes_req_msg_v01), uim_get_file_attributes_req_msg_data_v01},
  {sizeof(uim_get_file_attributes_resp_msg_v01), uim_get_file_attributes_resp_msg_data_v01},
  {sizeof(uim_get_file_attributes_ind_msg_v01), uim_get_file_attributes_ind_msg_data_v01},
  {sizeof(uim_refresh_register_req_msg_v01), uim_refresh_register_req_msg_data_v01},
  {sizeof(uim_refresh_register_resp_msg_v01), uim_refresh_register_resp_msg_data_v01},
  {sizeof(uim_refresh_ok_req_msg_v01), uim_refresh_ok_req_msg_data_v01},
  {sizeof(uim_refresh_ok_resp_msg_v01), uim_refresh_ok_resp_msg_data_v01},
  {sizeof(uim_refresh_complete_req_msg_v01), uim_refresh_complete_req_msg_data_v01},
  {sizeof(uim_refresh_complete_resp_msg_v01), uim_refresh_complete_resp_msg_data_v01},
  {sizeof(uim_refresh_get_last_event_req_msg_v01), uim_refresh_get_last_event_req_msg_data_v01},
  {sizeof(uim_refresh_get_last_event_resp_msg_v01), uim_refresh_get_last_event_resp_msg_data_v01},
  {sizeof(uim_set_pin_protection_req_msg_v01), uim_set_pin_protection_req_msg_data_v01},
  {sizeof(uim_set_pin_protection_resp_msg_v01), uim_set_pin_protection_resp_msg_data_v01},
  {sizeof(uim_set_pin_protection_ind_msg_v01), uim_set_pin_protection_ind_msg_data_v01},
  {sizeof(uim_verify_pin_req_msg_v01), uim_verify_pin_req_msg_data_v01},
  {sizeof(uim_verify_pin_resp_msg_v01), uim_verify_pin_resp_msg_data_v01},
  {sizeof(uim_verify_pin_ind_msg_v01), uim_verify_pin_ind_msg_data_v01},
  {sizeof(uim_unblock_pin_req_msg_v01), uim_unblock_pin_req_msg_data_v01},
  {sizeof(uim_unblock_pin_resp_msg_v01), uim_unblock_pin_resp_msg_data_v01},
  {sizeof(uim_unblock_pin_ind_msg_v01), uim_unblock_pin_ind_msg_data_v01},
  {sizeof(uim_change_pin_req_msg_v01), uim_change_pin_req_msg_data_v01},
  {sizeof(uim_change_pin_resp_msg_v01), uim_change_pin_resp_msg_data_v01},
  {sizeof(uim_change_pin_ind_msg_v01), uim_change_pin_ind_msg_data_v01},
  {sizeof(uim_depersonalization_req_msg_v01), uim_depersonalization_req_msg_data_v01},
  {sizeof(uim_depersonalization_resp_msg_v01), uim_depersonalization_resp_msg_data_v01},
  {sizeof(uim_power_down_req_msg_v01), uim_power_down_req_msg_data_v01},
  {sizeof(uim_power_down_resp_msg_v01), uim_power_down_resp_msg_data_v01},
  {sizeof(uim_power_up_req_msg_v01), uim_power_up_req_msg_data_v01},
  {sizeof(uim_power_up_resp_msg_v01), uim_power_up_resp_msg_data_v01},
  {sizeof(uim_get_card_status_req_msg_v01), uim_get_card_status_req_msg_data_v01},
  {sizeof(uim_get_card_status_resp_msg_v01), uim_get_card_status_resp_msg_data_v01},
  {sizeof(uim_event_reg_req_msg_v01), uim_event_reg_req_msg_data_v01},
  {sizeof(uim_event_reg_resp_msg_v01), uim_event_reg_resp_msg_data_v01},
  {sizeof(uim_status_change_ind_msg_v01), uim_status_change_ind_msg_data_v01},
  {sizeof(uim_refresh_ind_msg_v01), uim_refresh_ind_msg_data_v01},
  {sizeof(uim_authenticate_req_msg_v01), uim_authenticate_req_msg_data_v01},
  {sizeof(uim_authenticate_resp_msg_v01), uim_authenticate_resp_msg_data_v01},
  {sizeof(uim_authenticate_ind_msg_v01), uim_authenticate_ind_msg_data_v01},
  {sizeof(uim_close_session_req_msg_v01), uim_close_session_req_msg_data_v01},
  {sizeof(uim_close_session_resp_msg_v01), uim_close_session_resp_msg_data_v01},
  {sizeof(uim_get_service_status_req_msg_v01), uim_get_service_status_req_msg_data_v01},
  {sizeof(uim_get_service_status_resp_msg_v01), uim_get_service_status_resp_msg_data_v01},
  {sizeof(uim_set_service_status_req_msg_v01), uim_set_service_status_req_msg_data_v01},
  {sizeof(uim_set_service_status_resp_msg_v01), uim_set_service_status_resp_msg_data_v01},
  {sizeof(uim_change_provisioning_session_req_msg_v01), uim_change_provisioning_session_req_msg_data_v01},
  {sizeof(uim_change_provisioning_session_resp_msg_v01), uim_change_provisioning_session_resp_msg_data_v01},
  {sizeof(uim_get_label_req_msg_v01), uim_get_label_req_msg_data_v01},
  {sizeof(uim_get_label_resp_msg_v01), uim_get_label_resp_msg_data_v01},
  {sizeof(uim_get_configuration_req_msg_v01), uim_get_configuration_req_msg_data_v01},
  {sizeof(uim_get_configuration_resp_msg_v01), uim_get_configuration_resp_msg_data_v01},
  {sizeof(uim_send_apdu_req_msg_v01), uim_send_apdu_req_msg_data_v01},
  {sizeof(uim_send_apdu_resp_msg_v01), uim_send_apdu_resp_msg_data_v01},
  {sizeof(uim_send_apdu_ind_msg_v01), uim_send_apdu_ind_msg_data_v01},
  {sizeof(uim_sap_connection_req_msg_v01), uim_sap_connection_req_msg_data_v01},
  {sizeof(uim_sap_connection_resp_msg_v01), uim_sap_connection_resp_msg_data_v01},
  {sizeof(uim_sap_request_req_msg_v01), uim_sap_request_req_msg_data_v01},
  {sizeof(uim_sap_request_resp_msg_v01), uim_sap_request_resp_msg_data_v01},
  {sizeof(uim_sap_connection_ind_msg_v01), uim_sap_connection_ind_msg_data_v01},
  {sizeof(uim_logical_channel_req_msg_v01), uim_logical_channel_req_msg_data_v01},
  {sizeof(uim_logical_channel_resp_msg_v01), uim_logical_channel_resp_msg_data_v01},
  {sizeof(uim_subscription_ok_req_msg_v01), uim_subscription_ok_req_msg_data_v01},
  {sizeof(uim_subscription_ok_resp_msg_v01), uim_subscription_ok_resp_msg_data_v01},
  {sizeof(uim_get_atr_req_msg_v01), uim_get_atr_req_msg_data_v01},
  {sizeof(uim_get_atr_resp_msg_v01), uim_get_atr_resp_msg_data_v01},
  {sizeof(uim_open_logical_channel_req_msg_v01), uim_open_logical_channel_req_msg_data_v01},
  {sizeof(uim_open_logical_channel_resp_msg_v01), uim_open_logical_channel_resp_msg_data_v01},
  {sizeof(uim_session_closed_ind_msg_v01), uim_session_closed_ind_msg_data_v01},
  {sizeof(uim_refresh_register_all_req_msg_v01), uim_refresh_register_all_req_msg_data_v01},
  {sizeof(uim_refresh_register_all_resp_msg_v01), uim_refresh_register_all_resp_msg_data_v01},
  {sizeof(uim_set_file_status_req_msg_v01), uim_set_file_status_req_msg_data_v01},
  {sizeof(uim_set_file_status_resp_msg_v01), uim_set_file_status_resp_msg_data_v01},
  {sizeof(uim_switch_slot_req_msg_v01), uim_switch_slot_req_msg_data_v01},
  {sizeof(uim_switch_slot_resp_msg_v01), uim_switch_slot_resp_msg_data_v01},
  {sizeof(uim_get_slots_status_req_msg_v01), 0},
  {sizeof(uim_get_slots_status_resp_msg_v01), uim_get_slots_status_resp_msg_data_v01},
  {sizeof(uim_slot_status_change_ind_msg_v01), uim_slot_status_change_ind_msg_data_v01},
  {sizeof(uim_read_transparent_long_ind_msg_v01), uim_read_transparent_long_ind_msg_data_v01},
  {sizeof(uim_sim_busy_status_ind_msg_v01), uim_sim_busy_status_ind_msg_data_v01},
  {sizeof(uim_get_plmn_name_table_info_req_msg_v01), 0},
  {sizeof(uim_get_plmn_name_table_info_resp_msg_v01), uim_get_plmn_name_table_info_resp_msg_data_v01},
  {sizeof(uim_personalization_req_msg_v01), uim_personalization_req_msg_data_v01},
  {sizeof(uim_personalization_resp_msg_v01), uim_personalization_resp_msg_data_v01},
  {sizeof(uim_increase_req_msg_v01), uim_increase_req_msg_data_v01},
  {sizeof(uim_increase_resp_msg_v01), uim_increase_resp_msg_data_v01},
  {sizeof(uim_increase_ind_msg_v01), uim_increase_ind_msg_data_v01},
  {sizeof(uim_recovery_req_msg_v01), uim_recovery_req_msg_data_v01},
  {sizeof(uim_recovery_resp_msg_v01), uim_recovery_resp_msg_data_v01},
  {sizeof(uim_reselect_req_msg_v01), uim_reselect_req_msg_data_v01},
  {sizeof(uim_reselect_resp_msg_v01), uim_reselect_resp_msg_data_v01},
  {sizeof(uim_recovery_ind_msg_v01), uim_recovery_ind_msg_data_v01},
  {sizeof(uim_send_status_cmd_req_msg_v01), uim_send_status_cmd_req_msg_data_v01},
  {sizeof(uim_send_status_cmd_resp_msg_v01), uim_send_status_cmd_resp_msg_data_v01},
  {sizeof(uim_get_sim_profile_req_msg_v01), uim_get_sim_profile_req_msg_data_v01},
  {sizeof(uim_get_sim_profile_resp_msg_v01), uim_get_sim_profile_resp_msg_data_v01},
  {sizeof(uim_set_sim_profile_req_msg_v01), uim_set_sim_profile_req_msg_data_v01},
  {sizeof(uim_set_sim_profile_resp_msg_v01), uim_set_sim_profile_resp_msg_data_v01},
  {sizeof(uim_supply_voltage_req_msg_v01), uim_supply_voltage_req_msg_data_v01},
  {sizeof(uim_supply_voltage_resp_msg_v01), uim_supply_voltage_resp_msg_data_v01},
  {sizeof(uim_supply_voltage_ind_msg_v01), uim_supply_voltage_ind_msg_data_v01},
  {sizeof(uim_card_activation_status_ind_msg_v01), uim_card_activation_status_ind_msg_data_v01},
  {sizeof(uim_depersonalization_secure_req_msg_v01), uim_depersonalization_secure_req_msg_data_v01},
  {sizeof(uim_depersonalization_secure_resp_msg_v01), uim_depersonalization_secure_resp_msg_data_v01},
  {sizeof(uim_personalization_secure_req_msg_v01), uim_personalization_secure_req_msg_data_v01},
  {sizeof(uim_personalization_secure_resp_msg_v01), uim_personalization_secure_resp_msg_data_v01},
  {sizeof(uim_emergency_only_req_msg_v01), uim_emergency_only_req_msg_data_v01},
  {sizeof(uim_emergency_only_resp_msg_v01), uim_emergency_only_resp_msg_data_v01},
  {sizeof(uim_simlock_configuration_req_msg_v01), uim_simlock_configuration_req_msg_data_v01},
  {sizeof(uim_simlock_configuration_resp_msg_v01), uim_simlock_configuration_resp_msg_data_v01},
  {sizeof(uim_simlock_configuration_ind_msg_v01), uim_simlock_configuration_ind_msg_data_v01},
  {sizeof(uim_gba_req_msg_v01), uim_gba_req_msg_data_v01},
  {sizeof(uim_gba_resp_msg_v01), uim_gba_resp_msg_data_v01},
  {sizeof(uim_gba_ind_msg_v01), uim_gba_ind_msg_data_v01},
  {sizeof(uim_get_gba_impi_req_msg_v01), uim_get_gba_impi_req_msg_data_v01},
  {sizeof(uim_get_gba_impi_resp_msg_v01), uim_get_gba_impi_resp_msg_data_v01},
  {sizeof(uim_search_record_req_msg_v01), uim_search_record_req_msg_data_v01},
  {sizeof(uim_search_record_resp_msg_v01), uim_search_record_resp_msg_data_v01},
  {sizeof(uim_search_record_ind_msg_v01), uim_search_record_ind_msg_data_v01},
  {sizeof(uim_remote_unlock_req_msg_v01), uim_remote_unlock_req_msg_data_v01},
  {sizeof(uim_remote_unlock_resp_msg_v01), uim_remote_unlock_resp_msg_data_v01},
  {sizeof(uim_verify_imsi_req_msg_v01), uim_verify_imsi_req_msg_data_v01},
  {sizeof(uim_verify_imsi_resp_msg_v01), uim_verify_imsi_resp_msg_data_v01},
  {sizeof(uim_temporary_unlock_ind_msg_v01), uim_temporary_unlock_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object uim_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *uim_qmi_idl_type_table_object_referenced_tables_v01[] =
{&uim_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object uim_qmi_idl_type_table_object_v01 = {
  sizeof(uim_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(uim_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  uim_type_table_v01,
  uim_message_table_v01,
  uim_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry uim_service_command_messages_v01[] = {
  {QMI_UIM_RESET_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_UIM_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_UIM_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_UIM_READ_TRANSPARENT_REQ_V01, QMI_IDL_TYPE16(0, 2), 75},
  {QMI_UIM_READ_RECORD_REQ_V01, QMI_IDL_TYPE16(0, 5), 72},
  {QMI_UIM_WRITE_TRANSPARENT_REQ_V01, QMI_IDL_TYPE16(0, 8), 4163},
  {QMI_UIM_WRITE_RECORD_REQ_V01, QMI_IDL_TYPE16(0, 11), 322},
  {QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01, QMI_IDL_TYPE16(0, 14), 64},
  {QMI_UIM_SET_PIN_PROTECTION_REQ_V01, QMI_IDL_TYPE16(0, 25), 62},
  {QMI_UIM_VERIFY_PIN_REQ_V01, QMI_IDL_TYPE16(0, 28), 320},
  {QMI_UIM_UNBLOCK_PIN_REQ_V01, QMI_IDL_TYPE16(0, 31), 70},
  {QMI_UIM_CHANGE_PIN_REQ_V01, QMI_IDL_TYPE16(0, 34), 70},
  {QMI_UIM_DEPERSONALIZATION_REQ_V01, QMI_IDL_TYPE16(0, 37), 26},
  {QMI_UIM_REFRESH_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 17), 1344},
  {QMI_UIM_REFRESH_OK_REQ_V01, QMI_IDL_TYPE16(0, 19), 41},
  {QMI_UIM_REFRESH_COMPLETE_REQ_V01, QMI_IDL_TYPE16(0, 21), 41},
  {QMI_UIM_REFRESH_GET_LAST_EVENT_REQ_V01, QMI_IDL_TYPE16(0, 23), 37},
  {QMI_UIM_EVENT_REG_REQ_V01, QMI_IDL_TYPE16(0, 45), 7},
  {QMI_UIM_GET_CARD_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 43), 4},
  {QMI_UIM_POWER_DOWN_REQ_V01, QMI_IDL_TYPE16(0, 39), 4},
  {QMI_UIM_POWER_UP_REQ_V01, QMI_IDL_TYPE16(0, 41), 8},
  {QMI_UIM_AUTHENTICATE_REQ_V01, QMI_IDL_TYPE16(0, 49), 1074},
  {QMI_UIM_CLOSE_SESSION_REQ_V01, QMI_IDL_TYPE16(0, 52), 41},
  {QMI_UIM_GET_SERVICE_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 54), 48},
  {QMI_UIM_SET_SERVICE_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 56), 45},
  {QMI_UIM_CHANGE_PROVISIONING_SESSION_REQ_V01, QMI_IDL_TYPE16(0, 58), 42},
  {QMI_UIM_GET_LABEL_REQ_V01, QMI_IDL_TYPE16(0, 60), 37},
  {QMI_UIM_GET_CONFIGURATION_REQ_V01, QMI_IDL_TYPE16(0, 62), 7},
  {QMI_UIM_SEND_APDU_REQ_V01, QMI_IDL_TYPE16(0, 64), 1041},
  {QMI_UIM_SAP_CONNECTION_REQ_V01, QMI_IDL_TYPE16(0, 67), 20},
  {QMI_UIM_SAP_REQUEST_REQ_V01, QMI_IDL_TYPE16(0, 69), 1034},
  {QMI_UIM_LOGICAL_CHANNEL_REQ_V01, QMI_IDL_TYPE16(0, 72), 52},
  {QMI_UIM_SUBSCRIPTION_OK_REQ_V01, QMI_IDL_TYPE16(0, 74), 300},
  {QMI_UIM_GET_ATR_REQ_V01, QMI_IDL_TYPE16(0, 76), 4},
  {QMI_UIM_OPEN_LOGICAL_CHANNEL_REQ_V01, QMI_IDL_TYPE16(0, 78), 44},
  {QMI_UIM_REFRESH_REGISTER_ALL_REQ_V01, QMI_IDL_TYPE16(0, 81), 45},
  {QMI_UIM_SET_FILE_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 83), 60},
  {QMI_UIM_SWITCH_SLOT_REQ_V01, QMI_IDL_TYPE16(0, 85), 11},
  {QMI_UIM_GET_SLOTS_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 87), 0},
  {QMI_UIM_GET_PLMN_NAME_TABLE_INFO_REQ_V01, QMI_IDL_TYPE16(0, 92), 0},
  {QMI_UIM_PERSONALIZATION_REQ_V01, QMI_IDL_TYPE16(0, 94), 4475},
  {QMI_UIM_INCREASE_REQ_V01, QMI_IDL_TYPE16(0, 96), 191},
  {QMI_UIM_RECOVERY_REQ_V01, QMI_IDL_TYPE16(0, 99), 4},
  {QMI_UIM_RESELECT_REQ_V01, QMI_IDL_TYPE16(0, 101), 15},
  {QMI_UIM_SEND_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 104), 51},
  {QMI_UIM_GET_SIM_PROFILE_REQ_V01, QMI_IDL_TYPE16(0, 106), 4},
  {QMI_UIM_SET_SIM_PROFILE_REQ_V01, QMI_IDL_TYPE16(0, 108), 11},
  {QMI_UIM_SUPPLY_VOLTAGE_REQ_V01, QMI_IDL_TYPE16(0, 110), 4},
  {QMI_UIM_DEPERSONALIZATION_SECURE_REQ_V01, QMI_IDL_TYPE16(0, 114), 1029},
  {QMI_UIM_PERSONALIZATION_SECURE_REQ_V01, QMI_IDL_TYPE16(0, 116), 1029},
  {QMI_UIM_EMERGENCY_ONLY_REQ_V01, QMI_IDL_TYPE16(0, 118), 1029},
  {QMI_UIM_SIMLOCK_CONFIGURATION_REQ_V01, QMI_IDL_TYPE16(0, 120), 1029},
  {QMI_UIM_GBA_REQ_V01, QMI_IDL_TYPE16(0, 123), 416},
  {QMI_UIM_GET_GBA_IMPI_REQ_V01, QMI_IDL_TYPE16(0, 126), 41},
  {QMI_UIM_SEARCH_RECORD_REQ_V01, QMI_IDL_TYPE16(0, 128), 327},
  {QMI_UIM_REMOTE_UNLOCK_REQ_V01, QMI_IDL_TYPE16(0, 131), 1029},
  {QMI_UIM_VERIFY_IMSI_REQ_V01, QMI_IDL_TYPE16(0, 133), 14}
};

static const qmi_idl_service_message_table_entry uim_service_response_messages_v01[] = {
  {QMI_UIM_RESET_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_UIM_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_UIM_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_UIM_READ_TRANSPARENT_RESP_V01, QMI_IDL_TYPE16(0, 3), 5165},
  {QMI_UIM_READ_RECORD_RESP_V01, QMI_IDL_TYPE16(0, 6), 4380},
  {QMI_UIM_WRITE_TRANSPARENT_RESP_V01, QMI_IDL_TYPE16(0, 9), 19},
  {QMI_UIM_WRITE_RECORD_RESP_V01, QMI_IDL_TYPE16(0, 12), 19},
  {QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01, QMI_IDL_TYPE16(0, 15), 1079},
  {QMI_UIM_SET_PIN_PROTECTION_RESP_V01, QMI_IDL_TYPE16(0, 26), 283},
  {QMI_UIM_VERIFY_PIN_RESP_V01, QMI_IDL_TYPE16(0, 29), 283},
  {QMI_UIM_UNBLOCK_PIN_RESP_V01, QMI_IDL_TYPE16(0, 32), 283},
  {QMI_UIM_CHANGE_PIN_RESP_V01, QMI_IDL_TYPE16(0, 35), 283},
  {QMI_UIM_DEPERSONALIZATION_RESP_V01, QMI_IDL_TYPE16(0, 38), 12},
  {QMI_UIM_REFRESH_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 18), 7},
  {QMI_UIM_REFRESH_OK_RESP_V01, QMI_IDL_TYPE16(0, 20), 7},
  {QMI_UIM_REFRESH_COMPLETE_RESP_V01, QMI_IDL_TYPE16(0, 22), 7},
  {QMI_UIM_REFRESH_GET_LAST_EVENT_RESP_V01, QMI_IDL_TYPE16(0, 24), 1348},
  {QMI_UIM_EVENT_REG_RESP_V01, QMI_IDL_TYPE16(0, 46), 14},
  {QMI_UIM_GET_CARD_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 44), 5453},
  {QMI_UIM_POWER_DOWN_RESP_V01, QMI_IDL_TYPE16(0, 40), 7},
  {QMI_UIM_POWER_UP_RESP_V01, QMI_IDL_TYPE16(0, 42), 7},
  {QMI_UIM_AUTHENTICATE_RESP_V01, QMI_IDL_TYPE16(0, 50), 1048},
  {QMI_UIM_CLOSE_SESSION_RESP_V01, QMI_IDL_TYPE16(0, 53), 7},
  {QMI_UIM_GET_SERVICE_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 55), 31},
  {QMI_UIM_SET_SERVICE_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 57), 7},
  {QMI_UIM_CHANGE_PROVISIONING_SESSION_RESP_V01, QMI_IDL_TYPE16(0, 59), 7},
  {QMI_UIM_GET_LABEL_RESP_V01, QMI_IDL_TYPE16(0, 61), 266},
  {QMI_UIM_GET_CONFIGURATION_RESP_V01, QMI_IDL_TYPE16(0, 63), 1309},
  {QMI_UIM_SEND_APDU_RESP_V01, QMI_IDL_TYPE16(0, 65), 1045},
  {QMI_UIM_SAP_CONNECTION_RESP_V01, QMI_IDL_TYPE16(0, 68), 11},
  {QMI_UIM_SAP_REQUEST_RESP_V01, QMI_IDL_TYPE16(0, 70), 1554},
  {QMI_UIM_LOGICAL_CHANNEL_RESP_V01, QMI_IDL_TYPE16(0, 73), 275},
  {QMI_UIM_SUBSCRIPTION_OK_RESP_V01, QMI_IDL_TYPE16(0, 75), 7},
  {QMI_UIM_GET_ATR_RESP_V01, QMI_IDL_TYPE16(0, 77), 44},
  {QMI_UIM_OPEN_LOGICAL_CHANNEL_RESP_V01, QMI_IDL_TYPE16(0, 79), 275},
  {QMI_UIM_REFRESH_REGISTER_ALL_RESP_V01, QMI_IDL_TYPE16(0, 82), 7},
  {QMI_UIM_SET_FILE_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 84), 7},
  {QMI_UIM_SWITCH_SLOT_RESP_V01, QMI_IDL_TYPE16(0, 86), 7},
  {QMI_UIM_GET_SLOTS_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 88), 111},
  {QMI_UIM_GET_PLMN_NAME_TABLE_INFO_RESP_V01, QMI_IDL_TYPE16(0, 93), 21},
  {QMI_UIM_PERSONALIZATION_RESP_V01, QMI_IDL_TYPE16(0, 95), 12},
  {QMI_UIM_INCREASE_RESP_V01, QMI_IDL_TYPE16(0, 97), 150},
  {QMI_UIM_RECOVERY_RESP_V01, QMI_IDL_TYPE16(0, 100), 7},
  {QMI_UIM_RESELECT_RESP_V01, QMI_IDL_TYPE16(0, 102), 271},
  {QMI_UIM_SEND_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 105), 271},
  {QMI_UIM_GET_SIM_PROFILE_RESP_V01, QMI_IDL_TYPE16(0, 107), 86},
  {QMI_UIM_SET_SIM_PROFILE_RESP_V01, QMI_IDL_TYPE16(0, 109), 7},
  {QMI_UIM_SUPPLY_VOLTAGE_RESP_V01, QMI_IDL_TYPE16(0, 111), 7},
  {QMI_UIM_DEPERSONALIZATION_SECURE_RESP_V01, QMI_IDL_TYPE16(0, 115), 1036},
  {QMI_UIM_PERSONALIZATION_SECURE_RESP_V01, QMI_IDL_TYPE16(0, 117), 1036},
  {QMI_UIM_EMERGENCY_ONLY_RESP_V01, QMI_IDL_TYPE16(0, 119), 1036},
  {QMI_UIM_SIMLOCK_CONFIGURATION_RESP_V01, QMI_IDL_TYPE16(0, 121), 1036},
  {QMI_UIM_GBA_RESP_V01, QMI_IDL_TYPE16(0, 124), 1855},
  {QMI_UIM_GET_GBA_IMPI_RESP_V01, QMI_IDL_TYPE16(0, 127), 1295},
  {QMI_UIM_SEARCH_RECORD_RESP_V01, QMI_IDL_TYPE16(0, 129), 277},
  {QMI_UIM_REMOTE_UNLOCK_RESP_V01, QMI_IDL_TYPE16(0, 132), 524},
  {QMI_UIM_VERIFY_IMSI_RESP_V01, QMI_IDL_TYPE16(0, 134), 7}
};

static const qmi_idl_service_message_table_entry uim_service_indication_messages_v01[] = {
  {QMI_UIM_READ_TRANSPARENT_IND_V01, QMI_IDL_TYPE16(0, 4), 5165},
  {QMI_UIM_READ_RECORD_IND_V01, QMI_IDL_TYPE16(0, 7), 4380},
  {QMI_UIM_WRITE_TRANSPARENT_IND_V01, QMI_IDL_TYPE16(0, 10), 19},
  {QMI_UIM_WRITE_RECORD_IND_V01, QMI_IDL_TYPE16(0, 13), 19},
  {QMI_UIM_GET_FILE_ATTRIBUTES_IND_V01, QMI_IDL_TYPE16(0, 16), 1079},
  {QMI_UIM_SET_PIN_PROTECTION_IND_V01, QMI_IDL_TYPE16(0, 27), 283},
  {QMI_UIM_VERIFY_PIN_IND_V01, QMI_IDL_TYPE16(0, 30), 283},
  {QMI_UIM_UNBLOCK_PIN_IND_V01, QMI_IDL_TYPE16(0, 33), 283},
  {QMI_UIM_CHANGE_PIN_IND_V01, QMI_IDL_TYPE16(0, 36), 283},
  {QMI_UIM_STATUS_CHANGE_IND_V01, QMI_IDL_TYPE16(0, 47), 5422},
  {QMI_UIM_REFRESH_IND_V01, QMI_IDL_TYPE16(0, 48), 1341},
  {QMI_UIM_AUTHENTICATE_IND_V01, QMI_IDL_TYPE16(0, 51), 1048},
  {QMI_UIM_SEND_APDU_IND_V01, QMI_IDL_TYPE16(0, 66), 1037},
  {QMI_UIM_SAP_CONNECTION_IND_V01, QMI_IDL_TYPE16(0, 71), 5},
  {QMI_UIM_SESSION_CLOSED_IND_V01, QMI_IDL_TYPE16(0, 80), 60},
  {QMI_UIM_SLOT_STATUS_CHANGE_IND_V01, QMI_IDL_TYPE16(0, 89), 104},
  {QMI_UIM_READ_TRANSPARENT_LONG_IND_V01, QMI_IDL_TYPE16(0, 90), 4118},
  {QMI_UIM_SIM_BUSY_STATUS_IND_V01, QMI_IDL_TYPE16(0, 91), 9},
  {QMI_UIM_INCREASE_IND_V01, QMI_IDL_TYPE16(0, 98), 150},
  {QMI_UIM_RECOVERY_IND_V01, QMI_IDL_TYPE16(0, 103), 4},
  {QMI_UIM_SUPPLY_VOLTAGE_IND_V01, QMI_IDL_TYPE16(0, 112), 11},
  {QMI_UIM_CARD_ACTIVATION_STATUS_IND_V01, QMI_IDL_TYPE16(0, 113), 11},
  {QMI_UIM_SIMLOCK_CONFIGURATION_IND_V01, QMI_IDL_TYPE16(0, 122), 1029},
  {QMI_UIM_GBA_IND_V01, QMI_IDL_TYPE16(0, 125), 1855},
  {QMI_UIM_SEARCH_RECORD_IND_V01, QMI_IDL_TYPE16(0, 130), 277},
  {QMI_UIM_TEMPORARY_UNLOCK_STATUS_IND_V01, QMI_IDL_TYPE16(0, 135), 514}
};

/*Service Object*/
struct qmi_idl_service_object uim_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x0B,
  8204,
  { sizeof(uim_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(uim_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(uim_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { uim_service_command_messages_v01, uim_service_response_messages_v01, uim_service_indication_messages_v01},
  &uim_qmi_idl_type_table_object_v01,
  0x36,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type uim_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( UIM_V01_IDL_MAJOR_VERS != idl_maj_version || UIM_V01_IDL_MINOR_VERS != idl_min_version
       || UIM_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&uim_qmi_idl_service_object_v01;
}

