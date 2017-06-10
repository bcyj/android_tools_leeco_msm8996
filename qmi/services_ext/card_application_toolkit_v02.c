/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        C A R D _ A P P L I C A T I O N _ T O O L K I T _ V 0 2  . C

GENERAL DESCRIPTION
  This is the file which defines the cat service Data structures.

  Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/cat/main/latest/src/card_application_toolkit_v02.c#24 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It requires encode/decode library version 5 or later
   It was generated on: Thu Jun 27 2013 (Spin 1)
   From IDL File: card_application_toolkit_v02.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "card_application_toolkit_v02.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t cat_display_text_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_display_text_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_display_text_event_type_v02, pc_display_text),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_display_text_event_type_v02, pc_display_text) - QMI_IDL_OFFSET8(cat_display_text_event_type_v02, pc_display_text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_get_inkey_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_get_inkey_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_get_inkey_event_type_v02, pc_get_inkey),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_get_inkey_event_type_v02, pc_get_inkey) - QMI_IDL_OFFSET8(cat_get_inkey_event_type_v02, pc_get_inkey_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_get_input_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_get_input_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_get_input_event_type_v02, pc_get_input),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_get_input_event_type_v02, pc_get_input) - QMI_IDL_OFFSET8(cat_get_input_event_type_v02, pc_get_input_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_setup_menu_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_setup_menu_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_setup_menu_event_type_v02, pc_setup_menu),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_setup_menu_event_type_v02, pc_setup_menu) - QMI_IDL_OFFSET8(cat_setup_menu_event_type_v02, pc_setup_menu_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_select_item_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_select_item_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_select_item_event_type_v02, pc_select_item),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_select_item_event_type_v02, pc_select_item) - QMI_IDL_OFFSET8(cat_select_item_event_type_v02, pc_select_item_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_alpha_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_alpha_id_type_v02, pc_cmd_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_alpha_id_type_v02, alpha_identifier),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_alpha_id_type_v02, alpha_identifier) - QMI_IDL_OFFSET8(cat_alpha_id_type_v02, alpha_identifier_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_setup_evt_list_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_setup_evt_list_type_v02, pc_setup_evt_list),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_setup_idle_mode_text_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_setup_idle_mode_text_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_setup_idle_mode_text_event_type_v02, pc_setup_idle_mode_text),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_setup_idle_mode_text_event_type_v02, pc_setup_idle_mode_text) - QMI_IDL_OFFSET8(cat_setup_idle_mode_text_event_type_v02, pc_setup_idle_mode_text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_lang_notification_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_lang_notification_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_lang_notification_event_type_v02, pc_lang_notification),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_lang_notification_event_type_v02, pc_lang_notification) - QMI_IDL_OFFSET8(cat_lang_notification_event_type_v02, pc_lang_notification_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_refresh_event_type_data_v02[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_refresh_event_type_v02, refresh_mode),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_refresh_event_type_v02, refresh_stage),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_proactive_session_end_type_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_proactive_session_end_type_type_v02, proactive_session_end_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_decoded_header_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_decoded_header_type_v02, command_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_decoded_header_type_v02, uim_ref_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_decoded_header_type_v02, command_number),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_dcs_encoded_text_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_dcs_encoded_text_type_v02, dcs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_dcs_encoded_text_type_v02, text),
  QMI_CAT_DCS_ENCODED_STRING_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_dcs_encoded_text_type_v02, text) - QMI_IDL_OFFSET8(cat_dcs_encoded_text_type_v02, text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_high_priority_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_high_priority_type_v02, high_priority),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_user_control_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_user_control_type_v02, user_control),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_icon_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_icon_type_v02, qualifier),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_icon_type_v02, height),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_icon_type_v02, width),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_icon_type_v02, ics),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_icon_type_v02, rec_num),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_icon_type_v02, data),
  ((QMI_CAT_ICON_DATA_SIZE_MAX_V02) & 0xFF), ((QMI_CAT_ICON_DATA_SIZE_MAX_V02) >> 8),
  QMI_IDL_OFFSET8(cat_icon_type_v02, data) - QMI_IDL_OFFSET8(cat_icon_type_v02, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_duration_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_duration_type_v02, units),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_duration_type_v02, interval),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_response_format_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_response_format_type_v02, response_format),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_help_available_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_help_available_type_v02, help_available),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_response_packing_format_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_response_packing_format_type_v02, response_packing_format),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_response_length_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_response_length_type_v02, maximum_user_input),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_response_length_type_v02, minimum_user_input),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_show_user_input_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_show_user_input_type_v02, show_user_input),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_tone_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_tone_type_v02, tone),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_softkey_selection_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_softkey_selection_type_v02, softkey_selection),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_single_item_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_single_item_type_v02, item_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_single_item_type_v02, item_text),
  QMI_CAT_ITEM_TEXT_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_single_item_type_v02, item_text) - QMI_IDL_OFFSET8(cat_single_item_type_v02, item_text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_items_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_items_type_v02, items),
  QMI_CAT_NUMBER_OF_ITEMS_MAX_V02,
  QMI_IDL_OFFSET8(cat_items_type_v02, items) - QMI_IDL_OFFSET8(cat_items_type_v02, items_len),
 24, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_single_item_with_dcs_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_single_item_with_dcs_type_v02, item_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_single_item_with_dcs_type_v02, dcs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_single_item_with_dcs_type_v02, item_text),
  QMI_CAT_ITEM_TEXT_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_single_item_with_dcs_type_v02, item_text) - QMI_IDL_OFFSET8(cat_single_item_with_dcs_type_v02, item_text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_items_with_dcs_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_items_with_dcs_type_v02, items),
  QMI_CAT_NUMBER_OF_ITEMS_MAX_V02,
  QMI_IDL_OFFSET8(cat_items_with_dcs_type_v02, items) - QMI_IDL_OFFSET8(cat_items_with_dcs_type_v02, items_len),
 26, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_default_item_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_default_item_type_v02, default_item),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_next_action_indicator_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_next_action_indicator_type_v02, next_action_list),
  QMI_CAT_ACTION_LIST_MAX_V02,
  QMI_IDL_OFFSET8(cat_next_action_indicator_type_v02, next_action_list) - QMI_IDL_OFFSET8(cat_next_action_indicator_type_v02, next_action_list_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_icon_id_list_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_icon_id_list_type_v02, display_icon_only),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_icon_id_list_type_v02, icon_list),
  QMI_CAT_NUMBER_OF_ICONS_MAX_V02,
  QMI_IDL_OFFSET8(cat_icon_id_list_type_v02, icon_list) - QMI_IDL_OFFSET8(cat_icon_id_list_type_v02, icon_list_len),
 15, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_presentation_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_presentation_type_v02, presentation),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_packing_required_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_packing_required_type_v02, packing_required),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_sms_tpdu_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_sms_tpdu_type_v02, sms_tpdu),
  QMI_CAT_SMS_TPDU_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_sms_tpdu_type_v02, sms_tpdu) - QMI_IDL_OFFSET8(cat_sms_tpdu_type_v02, sms_tpdu_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_is_cdma_sms_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_is_cdma_sms_type_v02, is_cdma_sms),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_address_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_address_type_v02, ton),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_address_type_v02, npi),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_address_type_v02, address_data),
  QMI_CAT_ADDRESS_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_address_type_v02, address_data) - QMI_IDL_OFFSET8(cat_address_type_v02, address_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_call_setup_requirement_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_call_setup_requirement_type_v02, call_setup_requirement),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_redial_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_redial_type_v02, redial_necessary),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_redial_type_v02, units),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_redial_type_v02, interval),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_subaddress_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_subaddress_type_v02, subaddress),
  QMI_CAT_SUBADDRESS_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_subaddress_type_v02, subaddress) - QMI_IDL_OFFSET8(cat_subaddress_type_v02, subaddress_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_capability_config_data_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_capability_config_data_type_v02, capability_config_data),
  QMI_CAT_CAPABILITY_CONFIG_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_capability_config_data_type_v02, capability_config_data) - QMI_IDL_OFFSET8(cat_capability_config_data_type_v02, capability_config_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_dtmf_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_dtmf_type_v02, dtmf_data),
  QMI_CAT_DTMF_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_dtmf_type_v02, dtmf_data) - QMI_IDL_OFFSET8(cat_dtmf_type_v02, dtmf_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_spec_lang_notify_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_spec_lang_notify_type_v02, spec_lang_notify),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_language_type_data_v02[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_language_type_v02, language),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_launch_mode_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_launch_mode_type_v02, launch_mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_url_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_url_type_v02, url_data),
  QMI_CAT_URL_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_url_type_v02, url_data) - QMI_IDL_OFFSET8(cat_url_type_v02, url_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_browser_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_browser_id_type_v02, browser_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_bearer_list_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_bearer_list_type_v02, bearer_list),
  ((QMI_CAT_BEARER_LIST_MAX_V02) & 0xFF), ((QMI_CAT_BEARER_LIST_MAX_V02) >> 8),
  QMI_IDL_OFFSET8(cat_bearer_list_type_v02, bearer_list) - QMI_IDL_OFFSET8(cat_bearer_list_type_v02, bearer_list_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_file_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_file_type_v02, path),
  QMI_CAT_FILE_PATH_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_file_type_v02, path) - QMI_IDL_OFFSET8(cat_file_type_v02, path_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_prov_file_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_FIRST_EXTENDED |  QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_SZ_IS_32 ,
  QMI_IDL_OFFSET8(cat_prov_file_type_v02, file),
  ((QMI_CAT_NUMBER_OF_PROV_FILES_MAX_V02) & 0xFF), ((QMI_CAT_NUMBER_OF_PROV_FILES_MAX_V02) >> 8), 0, 0,
  QMI_IDL_OFFSET8(cat_prov_file_type_v02, file) - QMI_IDL_OFFSET8(cat_prov_file_type_v02, file_len),
 47, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_ussd_string_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_ussd_string_type_v02, orig_dcs_from_sim),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_ussd_string_type_v02, dcs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_ussd_string_type_v02, text),
  QMI_CAT_USSD_STRING_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_ussd_string_type_v02, text) - QMI_IDL_OFFSET8(cat_ussd_string_type_v02, text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_immediate_response_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_immediate_response_type_v02, immediate_resp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_notification_required_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_notification_required_type_v02, notification_required),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_play_tone_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_play_tone_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_play_tone_event_type_v02, pc_play_tone),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_play_tone_event_type_v02, pc_play_tone) - QMI_IDL_OFFSET8(cat_play_tone_event_type_v02, pc_play_tone_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_setup_call_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_setup_call_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_setup_call_event_type_v02, pc_setup_call),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_setup_call_event_type_v02, pc_setup_call) - QMI_IDL_OFFSET8(cat_setup_call_event_type_v02, pc_setup_call_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_send_dtmf_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_send_dtmf_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_send_dtmf_event_type_v02, pc_send_dtmf),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_send_dtmf_event_type_v02, pc_send_dtmf) - QMI_IDL_OFFSET8(cat_send_dtmf_event_type_v02, pc_send_dtmf_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_launch_browser_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_launch_browser_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_launch_browser_event_type_v02, pc_launch_browser),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_launch_browser_event_type_v02, pc_launch_browser) - QMI_IDL_OFFSET8(cat_launch_browser_event_type_v02, pc_launch_browser_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_send_sms_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_send_sms_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_send_sms_event_type_v02, pc_send_sms),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_send_sms_event_type_v02, pc_send_sms) - QMI_IDL_OFFSET8(cat_send_sms_event_type_v02, pc_send_sms_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_send_ss_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_send_ss_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_send_ss_event_type_v02, pc_send_ss),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_send_ss_event_type_v02, pc_send_ss) - QMI_IDL_OFFSET8(cat_send_ss_event_type_v02, pc_send_ss_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_send_ussd_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_send_ussd_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_send_ussd_event_type_v02, pc_send_ussd),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_send_ussd_event_type_v02, pc_send_ussd) - QMI_IDL_OFFSET8(cat_send_ussd_event_type_v02, pc_send_ussd_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_provide_local_info_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_provide_local_info_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_provide_local_info_event_type_v02, pc_provide_local_info),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_provide_local_info_event_type_v02, pc_provide_local_info) - QMI_IDL_OFFSET8(cat_provide_local_info_event_type_v02, pc_provide_local_info_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_setup_event_list_raw_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_setup_event_list_raw_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_setup_event_list_raw_event_type_v02, pc_setup_event_list),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_setup_event_list_raw_event_type_v02, pc_setup_event_list) - QMI_IDL_OFFSET8(cat_setup_event_list_raw_event_type_v02, pc_setup_event_list_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_slot_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_slot_type_v02, slot),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_open_channel_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_open_channel_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_open_channel_event_type_v02, pc_open_channel),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_open_channel_event_type_v02, pc_open_channel) - QMI_IDL_OFFSET8(cat_open_channel_event_type_v02, pc_open_channel_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_close_channel_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_close_channel_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_close_channel_event_type_v02, pc_close_channel),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_close_channel_event_type_v02, pc_close_channel) - QMI_IDL_OFFSET8(cat_close_channel_event_type_v02, pc_close_channel_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_send_data_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_send_data_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_send_data_event_type_v02, pc_send_data),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_send_data_event_type_v02, pc_send_data) - QMI_IDL_OFFSET8(cat_send_data_event_type_v02, pc_send_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_receive_data_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_receive_data_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_receive_data_event_type_v02, pc_receive_data),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_receive_data_event_type_v02, pc_receive_data) - QMI_IDL_OFFSET8(cat_receive_data_event_type_v02, pc_receive_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_on_demand_link_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_on_demand_link_type_v02, on_demand_link_est),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_csd_bearer_description_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_csd_bearer_description_type_v02, speed),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_csd_bearer_description_type_v02, name),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_csd_bearer_description_type_v02, connection_element),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_gprs_bearer_description_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_gprs_bearer_description_type_v02, precedence_cls),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_gprs_bearer_description_type_v02, delay_cls),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_gprs_bearer_description_type_v02, reliability_cls),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_gprs_bearer_description_type_v02, peak_throughput),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_gprs_bearer_description_type_v02, mean_throughput),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_gprs_bearer_description_type_v02, pkt_data_protocol),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_eutran_ext_param_bearer_description_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, traffic_class),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, max_bitrate_ul),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, max_bitrate_dl),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, guaranteed_bitrate_ul),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, guaranteed_bitrate_dl),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, delivery_order),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, max_sdu_size),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, max_sdu_err_ratio),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, residual_bit_err_ratio),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, delivery_of_err_sdu),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, transfer_delay),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, traffic_handling_pri),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_eutran_ext_param_bearer_description_type_v02, pdp_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_eutran_ext_mapped_utran_ps_bearer_description_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, qci),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, max_bitrate_ul),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, max_bitrate_dl),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, guaranteed_bitrate_ul),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, guaranteed_bitrate_dl),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, max_bitrate_ul_ext),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, max_bitrate_dl_ext),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, guaranteed_bitrate_ul_ext),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, guaranteed_bitrate_dl_ext),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02, pdp_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_buffer_size_type_data_v02[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_buffer_size_type_v02, buffer_size),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_network_access_name_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_network_access_name_type_v02, text),
  QMI_CAT_NETWORK_ACCESS_NAME_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_network_access_name_type_v02, text) - QMI_IDL_OFFSET8(cat_network_access_name_type_v02, text_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_ip_address_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_ip_address_type_v02, address_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_ip_address_type_v02, address_data),
  QMI_CAT_NETWORK_ADDRESS_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_ip_address_type_v02, address_data) - QMI_IDL_OFFSET8(cat_ip_address_type_v02, address_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_transport_level_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_transport_level_type_v02, transport_protocol),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_transport_level_type_v02, port_number),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_channel_data_lenght_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_channel_data_lenght_type_v02, ch_data_length),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_send_data_immediately_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_send_data_immediately_type_v02, send_data_immediately),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_channel_data_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_channel_data_type_v02, channel_data_string),
  ((QMI_CAT_CHANNEL_DATA_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_CHANNEL_DATA_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_channel_data_type_v02, channel_data_string) - QMI_IDL_OFFSET8(cat_channel_data_type_v02, channel_data_string_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_channel_id_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_channel_id_type_v02, ch_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_activate_event_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_activate_event_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_activate_event_type_v02, pc_activate),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_activate_event_type_v02, pc_activate) - QMI_IDL_OFFSET8(cat_activate_event_type_v02, pc_activate_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_activate_descriptor_target_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_activate_descriptor_target_type_v02, target),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_service_state_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_service_state_type_v02, cat_common_evt_reg_state_mask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_service_state_type_v02, pc_evt_report_mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_decoded_service_state_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_decoded_service_state_type_v02, cat_common_evt_reg_state_mask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_decoded_service_state_type_v02, pc_evt_report_mask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_terminal_response_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_terminal_response_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_terminal_response_type_v02, terminal_response),
  ((QMI_CAT_TERMINAL_RESPONSE_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_TERMINAL_RESPONSE_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_terminal_response_type_v02, terminal_response) - QMI_IDL_OFFSET8(cat_terminal_response_type_v02, terminal_response_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_terminal_resp_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_terminal_resp_type_v02, sw1),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_terminal_resp_type_v02, sw2),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_terminal_resp_type_v02, tr_response),
  QMI_CAT_TERMINAL_RESPONSE_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_terminal_resp_type_v02, tr_response) - QMI_IDL_OFFSET8(cat_terminal_resp_type_v02, tr_response_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_envelope_cmd_type_data_v02[] = {
  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_envelope_cmd_type_v02, env_cmd_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_envelope_cmd_type_v02, envelope_data),
  ((QMI_CAT_ENVELOPE_DATA_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_ENVELOPE_DATA_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_envelope_cmd_type_v02, envelope_data) - QMI_IDL_OFFSET8(cat_envelope_cmd_type_v02, envelope_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_envelope_resp_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_envelope_resp_type_v02, sw1),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_envelope_resp_type_v02, sw2),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_envelope_resp_type_v02, env_resp_data),
  QMI_CAT_RAW_ENV_RSP_DATA_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_envelope_resp_type_v02, env_resp_data) - QMI_IDL_OFFSET8(cat_envelope_resp_type_v02, env_resp_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_proactive_command_input_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_proactive_command_input_type_v02, cmd_ref_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_proactive_command_input_type_v02, format),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_terminal_response_command_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_terminal_response_command_type_v02, uim_ref_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_terminal_response_command_type_v02, command_number),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_terminal_response_command_type_v02, response_cmd),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_terminal_response_command_type_v02, general_result),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_terminal_response_command_type_v02, tr_additional_info),
  QMI_CAT_TR_ADDITIONAL_INFO_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_terminal_response_command_type_v02, tr_additional_info) - QMI_IDL_OFFSET8(cat_terminal_response_command_type_v02, tr_additional_info_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_tr_item_identifier_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_tr_item_identifier_type_v02, identifier),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_tr_get_inkey_extra_info_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_tr_get_inkey_extra_info_type_v02, unit),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_tr_get_inkey_extra_info_type_v02, interval),

   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_tr_get_inkey_extra_info_type_v02, get_inkey_text),
 12, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_tr_get_inkey_yes_no_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_tr_get_inkey_yes_no_type_v02, unit),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_tr_get_inkey_yes_no_type_v02, interval),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_tr_get_inkey_yes_no_type_v02, get_inkey_yes_no),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_decoded_envelope_cmd_type_data_v02[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_decoded_envelope_cmd_type_v02, env_cmd_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_decoded_envelope_identifier_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_decoded_envelope_identifier_type_v02, identifier),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_decoded_envelope_help_request_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_decoded_envelope_help_request_type_v02, help_request),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_pdp_context_act_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_pdp_context_act_type_v02, pdp_context_act_data),
  QMI_CAT_PDP_CONTEXT_ACT_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_pdp_context_act_type_v02, pdp_context_act_data) - QMI_IDL_OFFSET8(cat_pdp_context_act_type_v02, pdp_context_act_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_eps_pdn_connect_act_type_data_v02[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_eps_pdn_connect_act_type_v02, eps_pdn_connect_act_data),
  QMI_CAT_EPS_PDN_CONNECT_ACT_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_eps_pdn_connect_act_type_v02, eps_pdn_connect_act_data) - QMI_IDL_OFFSET8(cat_eps_pdn_connect_act_type_v02, eps_pdn_connect_act_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_user_confirm_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_user_confirm_type_v02, confirm),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_user_icon_display_type_data_v02[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_user_icon_display_type_v02, display),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_scws_channel_status_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_scws_channel_status_type_v02, ch_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_scws_channel_status_type_v02, state),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_scws_open_channel_info_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_info_type_v02, ch_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_info_type_v02, port),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_info_type_v02, buffer_size),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_scws_close_channel_info_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_scws_close_channel_info_type_v02, ch_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_scws_close_channel_info_type_v02, state),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_scws_send_data_result_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_scws_send_data_result_type_v02, ch_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_scws_send_data_result_type_v02, result),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_scws_send_data_info_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_scws_send_data_info_type_v02, ch_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_scws_send_data_info_type_v02, total_packets),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_scws_send_data_info_type_v02, current_packet),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_scws_send_data_info_type_v02, data),
  ((QMI_CAT_SCWS_DATA_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_SCWS_DATA_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_scws_send_data_info_type_v02, data) - QMI_IDL_OFFSET8(cat_scws_send_data_info_type_v02, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_scws_data_available_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_scws_data_available_type_v02, ch_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_scws_data_available_type_v02, data),
  ((QMI_CAT_SCWS_DATA_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_SCWS_DATA_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_scws_data_available_type_v02, data) - QMI_IDL_OFFSET8(cat_scws_data_available_type_v02, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_bip_status_type_data_v02[] = {
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_bip_status_type_v02, channel_id),
 78, 0,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_bip_status_type_v02, status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cat_refresh_alpha_evt_type_data_v02[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_refresh_alpha_evt_type_v02, uim_ref_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_refresh_alpha_evt_type_v02, pc_refresh_alpha),
  ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) & 0xFF), ((QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02) >> 8),
  QMI_IDL_OFFSET8(cat_refresh_alpha_evt_type_v02, pc_refresh_alpha) - QMI_IDL_OFFSET8(cat_refresh_alpha_evt_type_v02, pc_refresh_alpha_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * cat_reset_req_msg is empty
 * static const uint8_t cat_reset_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t cat_reset_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_reset_resp_msg_v02, resp),
  0, 1
};

static const uint8_t cat_set_event_report_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_evt_report_req_mask) - QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_evt_report_req_mask_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_evt_report_req_mask),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_dec_evt_report_req_mask) - QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_dec_evt_report_req_mask_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_dec_evt_report_req_mask),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, slot_mask) - QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, slot_mask_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, slot_mask),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_full_func_evt_report_req_mask) - QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_full_func_evt_report_req_mask_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_set_event_report_req_msg_v02, pc_full_func_evt_report_req_mask)
};

static const uint8_t cat_set_event_report_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_evt_report_req_err_mask) - QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_evt_report_req_err_mask_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_evt_report_req_err_mask),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_dec_evt_report_req_err_mask) - QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_dec_evt_report_req_err_mask_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_dec_evt_report_req_err_mask),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_full_func_evt_report_err_mask) - QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_full_func_evt_report_err_mask_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_set_event_report_resp_msg_v02, pc_full_func_evt_report_err_mask)
};

static const uint8_t cat_event_report_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_event_report_ind_msg_v02, display_text) - QMI_IDL_OFFSET8(cat_event_report_ind_msg_v02, display_text_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_event_report_ind_msg_v02, display_text),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, get_inkey) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, get_inkey_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, get_inkey),
  1, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, get_input) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, get_input_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, get_input),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_menu) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_menu_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, setup_menu),
  3, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, select_item) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, select_item_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, select_item),
  4, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, pc_alpha_id_available) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, pc_alpha_id_available_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, pc_alpha_id_available),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, pc_setup_evt_list) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, pc_setup_evt_list_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, pc_setup_evt_list),
  6, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, idle_mode_text) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, idle_mode_text_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, idle_mode_text),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, lang_notification) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, lang_notification_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, lang_notification),
  8, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, refresh) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, refresh_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, refresh),
  9, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, proactive_session_end_type) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, proactive_session_end_type_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, proactive_session_end_type),
  10, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, decoded_header) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, decoded_header_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, decoded_header),
  11, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, text_string) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, text_string_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, text_string),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, high_priority) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, high_priority_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, high_priority),
  13, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_control) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_control_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, user_control),
  14, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, icon) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, icon_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, icon),
  15, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, duration) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, duration_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, duration),
  16, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, response_format) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, response_format_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, response_format),
  17, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, help_available) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, help_available_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, help_available),
  18, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, response_packing_format) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, response_packing_format_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, response_packing_format),
  19, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, response_length) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, response_length_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, response_length),
  20, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, show_user_input) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, show_user_input_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, show_user_input),
  21, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, tone) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, tone_valid)),
  0x26,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, tone),
  22, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, softkey_selection) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, softkey_selection_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, softkey_selection),
  23, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, items) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, items_valid)),
  0x28,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, items),
  25, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, default_item) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, default_item_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, default_item),
  28, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, next_action_list) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, next_action_list_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, next_action_list),
  29, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, icon_id_list) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, icon_id_list_valid)),
  0x2B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_event_report_ind_msg_v02, icon_id_list),
  30, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, presentation) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, presentation_valid)),
  0x2C,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, presentation),
  31, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, packing_required) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, packing_required_valid)),
  0x2D,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, packing_required),
  32, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, sms_tpdu) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, sms_tpdu_valid)),
  0x2E,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, sms_tpdu),
  33, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, is_cdma_sms) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, is_cdma_sms_valid)),
  0x2F,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, is_cdma_sms),
  34, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, address) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, address_valid)),
  0x30,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, address),
  35, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, call_setup_requirement) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, call_setup_requirement_valid)),
  0x31,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, call_setup_requirement),
  36, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, redial) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, redial_valid)),
  0x32,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, redial),
  37, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, subaddress) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, subaddress_valid)),
  0x33,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, subaddress),
  38, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, capability_config_data) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, capability_config_data_valid)),
  0x34,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, capability_config_data),
  39, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, dtmf_data) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, dtmf_data_valid)),
  0x35,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, dtmf_data),
  40, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, spec_lang_notify) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, spec_lang_notify_valid)),
  0x36,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, spec_lang_notify),
  41, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, language) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, language_valid)),
  0x37,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, language),
  42, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, launch_mode) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, launch_mode_valid)),
  0x38,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, launch_mode),
  43, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, url) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, url_valid)),
  0x39,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, url),
  44, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, browswer_id) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, browswer_id_valid)),
  0x3A,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, browswer_id),
  45, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, bearer_list) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, bearer_list_valid)),
  0x3B,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, bearer_list),
  46, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, prov_files) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, prov_files_valid)),
  0x3C,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, prov_files),
  48, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, ussd_string) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, ussd_string_valid)),
  0x3D,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, ussd_string),
  49, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, default_text) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, default_text_valid)),
  0x3E,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, default_text),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, immediate_resp) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, immediate_resp_valid)),
  0x3F,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, immediate_resp),
  50, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_conf_alpha) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_conf_alpha_valid)),
  0x40,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, user_conf_alpha),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_call_disp_alpha) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_call_disp_alpha_valid)),
  0x41,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, setup_call_disp_alpha),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_conf_icon) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_conf_icon_valid)),
  0x42,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, user_conf_icon),
  15, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_call_disp_icon) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_call_disp_icon_valid)),
  0x43,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, setup_call_disp_icon),
  15, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, gateway_proxy) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, gateway_proxy_valid)),
  0x44,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, gateway_proxy),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, alpha) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, alpha_valid)),
  0x45,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, alpha),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, notification_required) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, notification_required_valid)),
  0x46,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, notification_required),
  51, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, play_tone) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, play_tone_valid)),
  0x47,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, play_tone),
  52, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_call) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_call_valid)),
  0x48,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, setup_call),
  53, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_dtmf) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_dtmf_valid)),
  0x49,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, send_dtmf),
  54, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, launch_browser) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, launch_browser_valid)),
  0x4A,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, launch_browser),
  55, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_sms) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_sms_valid)),
  0x4B,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, send_sms),
  56, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_ss) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_ss_valid)),
  0x4C,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, send_ss),
  57, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_ussd) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_ussd_valid)),
  0x4D,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, send_ussd),
  58, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, provide_local_info) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, provide_local_info_valid)),
  0x4E,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, provide_local_info),
  59, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_event_list_raw) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, setup_event_list_raw_valid)),
  0x4F,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, setup_event_list_raw),
  60, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, slot) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, slot_valid)),
  0x50,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, slot),
  61, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, open_channel) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, open_channel_valid)),
  0x51,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, open_channel),
  62, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, close_channel) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, close_channel_valid)),
  0x52,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, close_channel),
  63, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_data) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_data_valid)),
  0x53,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, send_data),
  64, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, receive_data) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, receive_data_valid)),
  0x54,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, receive_data),
  65, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, on_demand_link) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, on_demand_link_valid)),
  0x55,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, on_demand_link),
  66, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, csd_bearer_description) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, csd_bearer_description_valid)),
  0x56,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, csd_bearer_description),
  67, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, gprs_bearer_description) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, gprs_bearer_description_valid)),
  0x57,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, gprs_bearer_description),
  68, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, eutran_ext_param_bearer_description) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, eutran_ext_param_bearer_description_valid)),
  0x58,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, eutran_ext_param_bearer_description),
  69, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, eutran_ext_mapped_bearer_description) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, eutran_ext_mapped_bearer_description_valid)),
  0x59,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, eutran_ext_mapped_bearer_description),
  70, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, buffer_size) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, buffer_size_valid)),
  0x5A,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, buffer_size),
  71, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, network_access_name) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, network_access_name_valid)),
  0x5B,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, network_access_name),
  72, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, other_address) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, other_address_valid)),
  0x5C,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, other_address),
  73, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_login) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_login_valid)),
  0x5D,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, user_login),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_password) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, user_password_valid)),
  0x5E,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, user_password),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, transport_level) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, transport_level_valid)),
  0x5F,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, transport_level),
  74, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, data_destination_address) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, data_destination_address_valid)),
  0x60,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, data_destination_address),
  73, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, channel_data_length) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, channel_data_length_valid)),
  0x61,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, channel_data_length),
  75, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_data_immediately) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, send_data_immediately_valid)),
  0x62,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, send_data_immediately),
  76, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, channel_data) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, channel_data_valid)),
  0x63,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, channel_data),
  77, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, channel_id) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, channel_id_valid)),
  0x64,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, channel_id),
  78, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, items_with_dcs) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, items_with_dcs_valid)),
  0x65,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, items_with_dcs),
  27, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, activate) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, activate_valid)),
  0x66,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, activate),
  79, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, activate_target) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, activate_target_valid)),
  0x67,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, activate_target),
  80, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, rsp_type) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, rsp_type_valid)),
  0x68,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, rsp_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, bip_status) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, bip_status_valid)),
  0x69,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, bip_status),
  105, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, refresh_alpha) - QMI_IDL_OFFSET16RELATIVE(cat_event_report_ind_msg_v02, refresh_alpha_valid)),
  0x6A,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_event_report_ind_msg_v02, refresh_alpha),
  106, 0
};

/* 
 * cat_get_service_state_req_msg is empty
 * static const uint8_t cat_get_service_state_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t cat_get_service_state_resp_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_service_state_resp_msg_v02, cat_service_state),
  81, 0,

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_service_state_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_get_service_state_resp_msg_v02, decoded_cat_service_state) - QMI_IDL_OFFSET8(cat_get_service_state_resp_msg_v02, decoded_cat_service_state_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_service_state_resp_msg_v02, decoded_cat_service_state),
  82, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_get_service_state_resp_msg_v02, pc_full_func_evt_report_mask) - QMI_IDL_OFFSET8(cat_get_service_state_resp_msg_v02, pc_full_func_evt_report_mask_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cat_get_service_state_resp_msg_v02, pc_full_func_evt_report_mask)
};

static const uint8_t cat_send_tr_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_tr_req_msg_v02, terminal_response),
  83, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_tr_req_msg_v02, slot) - QMI_IDL_OFFSET16RELATIVE(cat_send_tr_req_msg_v02, slot_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_tr_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_send_tr_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_tr_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_tr_resp_msg_v02, tr_response_data) - QMI_IDL_OFFSET8(cat_send_tr_resp_msg_v02, tr_response_data_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_tr_resp_msg_v02, tr_response_data),
  84, 0
};

static const uint8_t cat_send_envelope_cmd_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_envelope_cmd_req_msg_v02, envelope_cmd),
  85, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_envelope_cmd_req_msg_v02, slot) - QMI_IDL_OFFSET16RELATIVE(cat_send_envelope_cmd_req_msg_v02, slot_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_envelope_cmd_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_send_envelope_cmd_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_envelope_cmd_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_envelope_cmd_resp_msg_v02, env_resp_data) - QMI_IDL_OFFSET8(cat_send_envelope_cmd_resp_msg_v02, env_resp_data_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_envelope_cmd_resp_msg_v02, env_resp_data),
  86, 0
};

static const uint8_t cat_get_event_report_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_event_report_req_msg_v02, proactive_command_input),
  87, 0
};

static const uint8_t cat_get_event_report_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_event_report_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_get_event_report_resp_msg_v02, display_text) - QMI_IDL_OFFSET8(cat_get_event_report_resp_msg_v02, display_text_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_event_report_resp_msg_v02, display_text),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, get_inkey) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, get_inkey_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, get_inkey),
  1, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, get_input) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, get_input_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, get_input),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_menu) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_menu_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, setup_menu),
  3, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, select_item) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, select_item_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, select_item),
  4, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, pc_alpha_id_available) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, pc_alpha_id_available_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, pc_alpha_id_available),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, pc_setup_evt_list) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, pc_setup_evt_list_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, pc_setup_evt_list),
  6, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, idle_mode_text) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, idle_mode_text_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, idle_mode_text),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, lang_notification) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, lang_notification_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, lang_notification),
  8, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, refresh) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, refresh_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, refresh),
  9, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, proactive_session_end_type) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, proactive_session_end_type_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, proactive_session_end_type),
  10, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, decoded_header) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, decoded_header_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, decoded_header),
  11, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, text_string) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, text_string_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, text_string),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, high_priority) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, high_priority_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, high_priority),
  13, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_control) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_control_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, user_control),
  14, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, icon) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, icon_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, icon),
  15, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, duration) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, duration_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, duration),
  16, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, response_format) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, response_format_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, response_format),
  17, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, help_available) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, help_available_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, help_available),
  18, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, response_packing_format) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, response_packing_format_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, response_packing_format),
  19, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, response_length) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, response_length_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, response_length),
  20, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, show_user_input) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, show_user_input_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, show_user_input),
  21, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, tone) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, tone_valid)),
  0x26,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, tone),
  22, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, softkey_selection) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, softkey_selection_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, softkey_selection),
  23, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, items) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, items_valid)),
  0x28,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, items),
  25, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, default_item) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, default_item_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, default_item),
  28, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, next_action_list) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, next_action_list_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, next_action_list),
  29, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, icon_id_list) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, icon_id_list_valid)),
  0x2B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_get_event_report_resp_msg_v02, icon_id_list),
  30, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, presentation) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, presentation_valid)),
  0x2C,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, presentation),
  31, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, packing_required) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, packing_required_valid)),
  0x2D,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, packing_required),
  32, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, sms_tpdu) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, sms_tpdu_valid)),
  0x2E,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, sms_tpdu),
  33, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, is_cdma_sms) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, is_cdma_sms_valid)),
  0x2F,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, is_cdma_sms),
  34, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, address) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, address_valid)),
  0x30,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, address),
  35, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, call_setup_requirement) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, call_setup_requirement_valid)),
  0x31,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, call_setup_requirement),
  36, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, redial) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, redial_valid)),
  0x32,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, redial),
  37, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, subaddress) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, subaddress_valid)),
  0x33,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, subaddress),
  38, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, capability_config_data) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, capability_config_data_valid)),
  0x34,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, capability_config_data),
  39, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, dtmf_data) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, dtmf_data_valid)),
  0x35,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, dtmf_data),
  40, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, spec_lang_notify) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, spec_lang_notify_valid)),
  0x36,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, spec_lang_notify),
  41, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, language) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, language_valid)),
  0x37,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, language),
  42, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, launch_mode) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, launch_mode_valid)),
  0x38,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, launch_mode),
  43, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, url) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, url_valid)),
  0x39,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, url),
  44, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, browswer_id) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, browswer_id_valid)),
  0x3A,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, browswer_id),
  45, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, bearer_list) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, bearer_list_valid)),
  0x3B,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, bearer_list),
  46, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, prov_files) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, prov_files_valid)),
  0x3C,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, prov_files),
  48, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, ussd_string) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, ussd_string_valid)),
  0x3D,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, ussd_string),
  49, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, default_text) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, default_text_valid)),
  0x3E,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, default_text),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, immediate_resp) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, immediate_resp_valid)),
  0x3F,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, immediate_resp),
  50, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_conf_alpha) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_conf_alpha_valid)),
  0x40,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, user_conf_alpha),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_call_disp_alpha) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_call_disp_alpha_valid)),
  0x41,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, setup_call_disp_alpha),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_conf_icon) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_conf_icon_valid)),
  0x42,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, user_conf_icon),
  15, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_call_disp_icon) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_call_disp_icon_valid)),
  0x43,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, setup_call_disp_icon),
  15, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, gateway_proxy) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, gateway_proxy_valid)),
  0x44,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, gateway_proxy),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, alpha) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, alpha_valid)),
  0x45,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, alpha),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, notification_required) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, notification_required_valid)),
  0x46,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, notification_required),
  51, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, play_tone) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, play_tone_valid)),
  0x47,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, play_tone),
  52, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_call) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_call_valid)),
  0x48,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, setup_call),
  53, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_dtmf) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_dtmf_valid)),
  0x49,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, send_dtmf),
  54, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, launch_browser) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, launch_browser_valid)),
  0x4A,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, launch_browser),
  55, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_sms) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_sms_valid)),
  0x4B,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, send_sms),
  56, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_ss) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_ss_valid)),
  0x4C,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, send_ss),
  57, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_ussd) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_ussd_valid)),
  0x4D,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, send_ussd),
  58, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, provide_local_info) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, provide_local_info_valid)),
  0x4E,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, provide_local_info),
  59, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_event_list_raw) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, setup_event_list_raw_valid)),
  0x4F,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, setup_event_list_raw),
  60, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, slot) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, slot_valid)),
  0x50,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, slot),
  61, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, open_channel) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, open_channel_valid)),
  0x51,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, open_channel),
  62, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, close_channel) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, close_channel_valid)),
  0x52,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, close_channel),
  63, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_data) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_data_valid)),
  0x53,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, send_data),
  64, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, receive_data) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, receive_data_valid)),
  0x54,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, receive_data),
  65, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, on_demand_link) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, on_demand_link_valid)),
  0x55,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, on_demand_link),
  66, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, csd_bearer_description) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, csd_bearer_description_valid)),
  0x56,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, csd_bearer_description),
  67, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, gprs_bearer_description) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, gprs_bearer_description_valid)),
  0x57,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, gprs_bearer_description),
  68, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, eutran_ext_param_bearer_description) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, eutran_ext_param_bearer_description_valid)),
  0x58,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, eutran_ext_param_bearer_description),
  69, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, eutran_ext_mapped_bearer_description) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, eutran_ext_mapped_bearer_description_valid)),
  0x59,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, eutran_ext_mapped_bearer_description),
  70, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, buffer_size) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, buffer_size_valid)),
  0x5A,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, buffer_size),
  71, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, network_access_name) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, network_access_name_valid)),
  0x5B,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, network_access_name),
  72, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, other_address) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, other_address_valid)),
  0x5C,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, other_address),
  73, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_login) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_login_valid)),
  0x5D,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, user_login),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_password) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, user_password_valid)),
  0x5E,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, user_password),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, transport_level) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, transport_level_valid)),
  0x5F,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, transport_level),
  74, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, data_destination_address) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, data_destination_address_valid)),
  0x60,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, data_destination_address),
  73, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, channel_data_length) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, channel_data_length_valid)),
  0x61,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, channel_data_length),
  75, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_data_immediately) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, send_data_immediately_valid)),
  0x62,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, send_data_immediately),
  76, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, channel_data) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, channel_data_valid)),
  0x63,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, channel_data),
  77, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, channel_id) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, channel_id_valid)),
  0x64,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, channel_id),
  78, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, items_with_dcs) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, items_with_dcs_valid)),
  0x65,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, items_with_dcs),
  27, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, activate) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, activate_valid)),
  0x66,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, activate),
  79, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, activate_target) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, activate_target_valid)),
  0x67,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, activate_target),
  80, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, rsp_type) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, rsp_type_valid)),
  0x68,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, rsp_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, bip_status) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, bip_status_valid)),
  0x69,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, bip_status),
  105, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, refresh_alpha) - QMI_IDL_OFFSET16RELATIVE(cat_get_event_report_resp_msg_v02, refresh_alpha_valid)),
  0x6A,
  QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_EXTENDED_OFFSET ,
  QMI_IDL_OFFSET32ARRAY(cat_get_event_report_resp_msg_v02, refresh_alpha),
  106, 0
};

static const uint8_t cat_send_decoded_tr_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_tr_req_msg_v02, terminal_response),
  88, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_tr_req_msg_v02, text_string) - QMI_IDL_OFFSET8(cat_send_decoded_tr_req_msg_v02, text_string_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_tr_req_msg_v02, text_string),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, identifier) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, identifier_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_tr_req_msg_v02, identifier),
  89, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, get_inkey_extra_info) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, get_inkey_extra_info_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_tr_req_msg_v02, get_inkey_extra_info),
  90, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, language) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, language_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_tr_req_msg_v02, language),
  42, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, slot) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, slot_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_tr_req_msg_v02, slot),
  61, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, gst_inkey_yes_no_info) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_tr_req_msg_v02, gst_inkey_yes_no_info_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_tr_req_msg_v02, gst_inkey_yes_no_info),
  91, 0
};

static const uint8_t cat_send_decoded_tr_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_tr_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_tr_resp_msg_v02, tr_response_data) - QMI_IDL_OFFSET8(cat_send_decoded_tr_resp_msg_v02, tr_response_data_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_tr_resp_msg_v02, tr_response_data),
  84, 0
};

static const uint8_t cat_send_decoded_envelope_cmd_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, env_cmd_type),
  92, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, identifier) - QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, identifier_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, identifier),
  93, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, help_request) - QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, help_request_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, help_request),
  94, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, language) - QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, language_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, language),
  42, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, slot) - QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, slot_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, slot),
  61, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, address) - QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, address_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_req_msg_v02, address),
  35, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, sub_address) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, sub_address_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, sub_address),
  38, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, capability_config_param1) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, capability_config_param1_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, capability_config_param1),
  39, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, capability_config_param2) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, capability_config_param2_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, capability_config_param2),
  39, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, ussd_string) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, ussd_string_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, ussd_string),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, pdp_context_act) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, pdp_context_act_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, pdp_context_act),
  95, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, eps_pdn_connect_act) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, eps_pdn_connect_act_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, eps_pdn_connect_act),
  96, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, browser_term_cause) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, browser_term_cause_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, browser_term_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, sms_tpdu) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, sms_tpdu_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, sms_tpdu),
  33, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, is_cdma_sms) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, is_cdma_sms_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, is_cdma_sms),
  34, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, rat) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, rat_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, rat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, call_type) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, call_type_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, call_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, transaction_id) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, transaction_id_valid)),
  0x20,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, transaction_id),
  QMI_CAT_TX_ID_MAX_LENGTH_V02,
  QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, transaction_id) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, transaction_id_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, rp_dest_address) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, rp_dest_address_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, rp_dest_address),
  35, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, tp_dest_address) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, tp_dest_address_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, tp_dest_address),
  35, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, cause) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, cause_valid)),
  0x23,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_req_msg_v02, cause),
  QMI_CAT_CAUSE_MAX_LENGTH_V02,
  QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, cause) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_req_msg_v02, cause_len)
};

static const uint8_t cat_send_decoded_envelope_cmd_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_resp_msg_v02, cc_result) - QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_resp_msg_v02, cc_result_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_resp_msg_v02, cc_result),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_resp_msg_v02, address) - QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_resp_msg_v02, address_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_send_decoded_envelope_cmd_resp_msg_v02, address),
  35, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, sub_address) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, sub_address_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, sub_address),
  38, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, capability_config_param1) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, capability_config_param1_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, capability_config_param1),
  39, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, capability_config_param2) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, capability_config_param2_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, capability_config_param2),
  39, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, ussd_string) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, ussd_string_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, ussd_string),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, pdp_context_act) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, pdp_context_act_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, pdp_context_act),
  95, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, eps_pdn_connect_act) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, eps_pdn_connect_act_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, eps_pdn_connect_act),
  96, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, alpha) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, alpha_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, alpha),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, bc_repeat_ind) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, bc_repeat_ind_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, bc_repeat_ind),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, sms_pp_uicc_acknowledge) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, sms_pp_uicc_acknowledge_valid)),
  0x1A,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, sms_pp_uicc_acknowledge),
  QMI_CAT_SMS_PP_UICC_ACK_MAX_LENGTH_V02,
  QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, sms_pp_uicc_acknowledge) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, sms_pp_uicc_acknowledge_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, rp_dest_address) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, rp_dest_address_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, rp_dest_address),
  35, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, tp_dest_address) - QMI_IDL_OFFSET16RELATIVE(cat_send_decoded_envelope_cmd_resp_msg_v02, tp_dest_address_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_send_decoded_envelope_cmd_resp_msg_v02, tp_dest_address),
  35, 0
};

static const uint8_t cat_event_confirmation_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, confirm) - QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, confirm_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, confirm),
  97, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, display) - QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, display_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, display),
  98, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, slot) - QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, slot_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_event_confirmation_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_event_confirmation_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_event_confirmation_resp_msg_v02, resp),
  0, 1
};

static const uint8_t cat_scws_open_channel_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_req_msg_v02, channel_status),
  99, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_open_channel_req_msg_v02, slot) - QMI_IDL_OFFSET8(cat_scws_open_channel_req_msg_v02, slot_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_scws_open_channel_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_resp_msg_v02, resp),
  0, 1
};

static const uint8_t cat_scws_open_channel_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, open_channel_info) - QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, open_channel_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, open_channel_info),
  100, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, slot) - QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, slot_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, slot),
  61, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, alpha) - QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, alpha_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_open_channel_ind_msg_v02, alpha),
  12, 0
};

static const uint8_t cat_scws_close_channel_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_close_channel_req_msg_v02, channel_status),
  99, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_close_channel_req_msg_v02, slot) - QMI_IDL_OFFSET8(cat_scws_close_channel_req_msg_v02, slot_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_close_channel_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_scws_close_channel_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_close_channel_resp_msg_v02, resp),
  0, 1
};

static const uint8_t cat_scws_close_channel_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_close_channel_ind_msg_v02, close_channel_info) - QMI_IDL_OFFSET8(cat_scws_close_channel_ind_msg_v02, close_channel_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_close_channel_ind_msg_v02, close_channel_info),
  101, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_close_channel_ind_msg_v02, slot) - QMI_IDL_OFFSET8(cat_scws_close_channel_ind_msg_v02, slot_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_close_channel_ind_msg_v02, slot),
  61, 0
};

static const uint8_t cat_scws_send_data_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_send_data_req_msg_v02, result),
  102, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_send_data_req_msg_v02, slot) - QMI_IDL_OFFSET8(cat_scws_send_data_req_msg_v02, slot_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_send_data_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_scws_send_data_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_send_data_resp_msg_v02, resp),
  0, 1
};

static const uint8_t cat_scws_send_data_ind_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_send_data_ind_msg_v02, send_data_info) - QMI_IDL_OFFSET8(cat_scws_send_data_ind_msg_v02, send_data_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_send_data_ind_msg_v02, send_data_info),
  103, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_scws_send_data_ind_msg_v02, slot) - QMI_IDL_OFFSET16RELATIVE(cat_scws_send_data_ind_msg_v02, slot_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_scws_send_data_ind_msg_v02, slot),
  61, 0
};

static const uint8_t cat_scws_data_available_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_data_available_req_msg_v02, result),
  104, 0,

  0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(cat_scws_data_available_req_msg_v02, remaining_data_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(cat_scws_data_available_req_msg_v02, slot) - QMI_IDL_OFFSET16RELATIVE(cat_scws_data_available_req_msg_v02, slot_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(cat_scws_data_available_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_scws_data_available_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_data_available_resp_msg_v02, resp),
  0, 1
};

static const uint8_t cat_scws_channel_status_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_channel_status_req_msg_v02, channel_status),
  99, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_scws_channel_status_req_msg_v02, slot) - QMI_IDL_OFFSET8(cat_scws_channel_status_req_msg_v02, slot_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_channel_status_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_scws_channel_status_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_scws_channel_status_resp_msg_v02, resp),
  0, 1
};

static const uint8_t cat_get_terminal_profile_req_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_get_terminal_profile_req_msg_v02, slot) - QMI_IDL_OFFSET8(cat_get_terminal_profile_req_msg_v02, slot_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_terminal_profile_req_msg_v02, slot),
  61, 0
};

static const uint8_t cat_get_terminal_profile_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_terminal_profile_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_get_terminal_profile_resp_msg_v02, terminal_profile_data) - QMI_IDL_OFFSET8(cat_get_terminal_profile_resp_msg_v02, terminal_profile_data_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_get_terminal_profile_resp_msg_v02, terminal_profile_data),
  QMI_CAT_TERMINAL_PROFILE_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_get_terminal_profile_resp_msg_v02, terminal_profile_data) - QMI_IDL_OFFSET8(cat_get_terminal_profile_resp_msg_v02, terminal_profile_data_len)
};

static const uint8_t cat_set_configuration_req_msg_data_v02[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_set_configuration_req_msg_v02, cat_config_mode),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_set_configuration_req_msg_v02, custom_tp) - QMI_IDL_OFFSET8(cat_set_configuration_req_msg_v02, custom_tp_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_set_configuration_req_msg_v02, custom_tp),
  QMI_CAT_TERMINAL_PROFILE_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_set_configuration_req_msg_v02, custom_tp) - QMI_IDL_OFFSET8(cat_set_configuration_req_msg_v02, custom_tp_len)
};

static const uint8_t cat_set_configuration_resp_msg_data_v02[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_set_configuration_resp_msg_v02, resp),
  0, 1
};

/* 
 * cat_get_configuration_req_msg is empty
 * static const uint8_t cat_get_configuration_req_msg_data_v02[] = {
 * };
 */
  
static const uint8_t cat_get_configuration_resp_msg_data_v02[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, cat_config_mode) - QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, cat_config_mode_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, cat_config_mode),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, custom_tp) - QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, custom_tp_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, custom_tp),
  QMI_CAT_TERMINAL_PROFILE_MAX_LENGTH_V02,
  QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, custom_tp) - QMI_IDL_OFFSET8(cat_get_configuration_resp_msg_v02, custom_tp_len)
};

/* Type Table */
static const qmi_idl_type_table_entry  cat_type_table_v02[] = {
  {sizeof(cat_display_text_event_type_v02), cat_display_text_event_type_data_v02},
  {sizeof(cat_get_inkey_event_type_v02), cat_get_inkey_event_type_data_v02},
  {sizeof(cat_get_input_event_type_v02), cat_get_input_event_type_data_v02},
  {sizeof(cat_setup_menu_event_type_v02), cat_setup_menu_event_type_data_v02},
  {sizeof(cat_select_item_event_type_v02), cat_select_item_event_type_data_v02},
  {sizeof(cat_alpha_id_type_v02), cat_alpha_id_type_data_v02},
  {sizeof(cat_setup_evt_list_type_v02), cat_setup_evt_list_type_data_v02},
  {sizeof(cat_setup_idle_mode_text_event_type_v02), cat_setup_idle_mode_text_event_type_data_v02},
  {sizeof(cat_lang_notification_event_type_v02), cat_lang_notification_event_type_data_v02},
  {sizeof(cat_refresh_event_type_v02), cat_refresh_event_type_data_v02},
  {sizeof(cat_proactive_session_end_type_type_v02), cat_proactive_session_end_type_type_data_v02},
  {sizeof(cat_decoded_header_type_v02), cat_decoded_header_type_data_v02},
  {sizeof(cat_dcs_encoded_text_type_v02), cat_dcs_encoded_text_type_data_v02},
  {sizeof(cat_high_priority_type_v02), cat_high_priority_type_data_v02},
  {sizeof(cat_user_control_type_v02), cat_user_control_type_data_v02},
  {sizeof(cat_icon_type_v02), cat_icon_type_data_v02},
  {sizeof(cat_duration_type_v02), cat_duration_type_data_v02},
  {sizeof(cat_response_format_type_v02), cat_response_format_type_data_v02},
  {sizeof(cat_help_available_type_v02), cat_help_available_type_data_v02},
  {sizeof(cat_response_packing_format_type_v02), cat_response_packing_format_type_data_v02},
  {sizeof(cat_response_length_type_v02), cat_response_length_type_data_v02},
  {sizeof(cat_show_user_input_type_v02), cat_show_user_input_type_data_v02},
  {sizeof(cat_tone_type_v02), cat_tone_type_data_v02},
  {sizeof(cat_softkey_selection_type_v02), cat_softkey_selection_type_data_v02},
  {sizeof(cat_single_item_type_v02), cat_single_item_type_data_v02},
  {sizeof(cat_items_type_v02), cat_items_type_data_v02},
  {sizeof(cat_single_item_with_dcs_type_v02), cat_single_item_with_dcs_type_data_v02},
  {sizeof(cat_items_with_dcs_type_v02), cat_items_with_dcs_type_data_v02},
  {sizeof(cat_default_item_type_v02), cat_default_item_type_data_v02},
  {sizeof(cat_next_action_indicator_type_v02), cat_next_action_indicator_type_data_v02},
  {sizeof(cat_icon_id_list_type_v02), cat_icon_id_list_type_data_v02},
  {sizeof(cat_presentation_type_v02), cat_presentation_type_data_v02},
  {sizeof(cat_packing_required_type_v02), cat_packing_required_type_data_v02},
  {sizeof(cat_sms_tpdu_type_v02), cat_sms_tpdu_type_data_v02},
  {sizeof(cat_is_cdma_sms_type_v02), cat_is_cdma_sms_type_data_v02},
  {sizeof(cat_address_type_v02), cat_address_type_data_v02},
  {sizeof(cat_call_setup_requirement_type_v02), cat_call_setup_requirement_type_data_v02},
  {sizeof(cat_redial_type_v02), cat_redial_type_data_v02},
  {sizeof(cat_subaddress_type_v02), cat_subaddress_type_data_v02},
  {sizeof(cat_capability_config_data_type_v02), cat_capability_config_data_type_data_v02},
  {sizeof(cat_dtmf_type_v02), cat_dtmf_type_data_v02},
  {sizeof(cat_spec_lang_notify_type_v02), cat_spec_lang_notify_type_data_v02},
  {sizeof(cat_language_type_v02), cat_language_type_data_v02},
  {sizeof(cat_launch_mode_type_v02), cat_launch_mode_type_data_v02},
  {sizeof(cat_url_type_v02), cat_url_type_data_v02},
  {sizeof(cat_browser_id_type_v02), cat_browser_id_type_data_v02},
  {sizeof(cat_bearer_list_type_v02), cat_bearer_list_type_data_v02},
  {sizeof(cat_file_type_v02), cat_file_type_data_v02},
  {sizeof(cat_prov_file_type_v02), cat_prov_file_type_data_v02},
  {sizeof(cat_ussd_string_type_v02), cat_ussd_string_type_data_v02},
  {sizeof(cat_immediate_response_type_v02), cat_immediate_response_type_data_v02},
  {sizeof(cat_notification_required_type_v02), cat_notification_required_type_data_v02},
  {sizeof(cat_play_tone_event_type_v02), cat_play_tone_event_type_data_v02},
  {sizeof(cat_setup_call_event_type_v02), cat_setup_call_event_type_data_v02},
  {sizeof(cat_send_dtmf_event_type_v02), cat_send_dtmf_event_type_data_v02},
  {sizeof(cat_launch_browser_event_type_v02), cat_launch_browser_event_type_data_v02},
  {sizeof(cat_send_sms_event_type_v02), cat_send_sms_event_type_data_v02},
  {sizeof(cat_send_ss_event_type_v02), cat_send_ss_event_type_data_v02},
  {sizeof(cat_send_ussd_event_type_v02), cat_send_ussd_event_type_data_v02},
  {sizeof(cat_provide_local_info_event_type_v02), cat_provide_local_info_event_type_data_v02},
  {sizeof(cat_setup_event_list_raw_event_type_v02), cat_setup_event_list_raw_event_type_data_v02},
  {sizeof(cat_slot_type_v02), cat_slot_type_data_v02},
  {sizeof(cat_open_channel_event_type_v02), cat_open_channel_event_type_data_v02},
  {sizeof(cat_close_channel_event_type_v02), cat_close_channel_event_type_data_v02},
  {sizeof(cat_send_data_event_type_v02), cat_send_data_event_type_data_v02},
  {sizeof(cat_receive_data_event_type_v02), cat_receive_data_event_type_data_v02},
  {sizeof(cat_on_demand_link_type_v02), cat_on_demand_link_type_data_v02},
  {sizeof(cat_csd_bearer_description_type_v02), cat_csd_bearer_description_type_data_v02},
  {sizeof(cat_gprs_bearer_description_type_v02), cat_gprs_bearer_description_type_data_v02},
  {sizeof(cat_eutran_ext_param_bearer_description_type_v02), cat_eutran_ext_param_bearer_description_type_data_v02},
  {sizeof(cat_eutran_ext_mapped_utran_ps_bearer_description_type_v02), cat_eutran_ext_mapped_utran_ps_bearer_description_type_data_v02},
  {sizeof(cat_buffer_size_type_v02), cat_buffer_size_type_data_v02},
  {sizeof(cat_network_access_name_type_v02), cat_network_access_name_type_data_v02},
  {sizeof(cat_ip_address_type_v02), cat_ip_address_type_data_v02},
  {sizeof(cat_transport_level_type_v02), cat_transport_level_type_data_v02},
  {sizeof(cat_channel_data_lenght_type_v02), cat_channel_data_lenght_type_data_v02},
  {sizeof(cat_send_data_immediately_type_v02), cat_send_data_immediately_type_data_v02},
  {sizeof(cat_channel_data_type_v02), cat_channel_data_type_data_v02},
  {sizeof(cat_channel_id_type_v02), cat_channel_id_type_data_v02},
  {sizeof(cat_activate_event_type_v02), cat_activate_event_type_data_v02},
  {sizeof(cat_activate_descriptor_target_type_v02), cat_activate_descriptor_target_type_data_v02},
  {sizeof(cat_service_state_type_v02), cat_service_state_type_data_v02},
  {sizeof(cat_decoded_service_state_type_v02), cat_decoded_service_state_type_data_v02},
  {sizeof(cat_terminal_response_type_v02), cat_terminal_response_type_data_v02},
  {sizeof(cat_terminal_resp_type_v02), cat_terminal_resp_type_data_v02},
  {sizeof(cat_envelope_cmd_type_v02), cat_envelope_cmd_type_data_v02},
  {sizeof(cat_envelope_resp_type_v02), cat_envelope_resp_type_data_v02},
  {sizeof(cat_proactive_command_input_type_v02), cat_proactive_command_input_type_data_v02},
  {sizeof(cat_terminal_response_command_type_v02), cat_terminal_response_command_type_data_v02},
  {sizeof(cat_tr_item_identifier_type_v02), cat_tr_item_identifier_type_data_v02},
  {sizeof(cat_tr_get_inkey_extra_info_type_v02), cat_tr_get_inkey_extra_info_type_data_v02},
  {sizeof(cat_tr_get_inkey_yes_no_type_v02), cat_tr_get_inkey_yes_no_type_data_v02},
  {sizeof(cat_decoded_envelope_cmd_type_v02), cat_decoded_envelope_cmd_type_data_v02},
  {sizeof(cat_decoded_envelope_identifier_type_v02), cat_decoded_envelope_identifier_type_data_v02},
  {sizeof(cat_decoded_envelope_help_request_type_v02), cat_decoded_envelope_help_request_type_data_v02},
  {sizeof(cat_pdp_context_act_type_v02), cat_pdp_context_act_type_data_v02},
  {sizeof(cat_eps_pdn_connect_act_type_v02), cat_eps_pdn_connect_act_type_data_v02},
  {sizeof(cat_user_confirm_type_v02), cat_user_confirm_type_data_v02},
  {sizeof(cat_user_icon_display_type_v02), cat_user_icon_display_type_data_v02},
  {sizeof(cat_scws_channel_status_type_v02), cat_scws_channel_status_type_data_v02},
  {sizeof(cat_scws_open_channel_info_type_v02), cat_scws_open_channel_info_type_data_v02},
  {sizeof(cat_scws_close_channel_info_type_v02), cat_scws_close_channel_info_type_data_v02},
  {sizeof(cat_scws_send_data_result_type_v02), cat_scws_send_data_result_type_data_v02},
  {sizeof(cat_scws_send_data_info_type_v02), cat_scws_send_data_info_type_data_v02},
  {sizeof(cat_scws_data_available_type_v02), cat_scws_data_available_type_data_v02},
  {sizeof(cat_bip_status_type_v02), cat_bip_status_type_data_v02},
  {sizeof(cat_refresh_alpha_evt_type_v02), cat_refresh_alpha_evt_type_data_v02}
};

/* Message Table */
static const qmi_idl_message_table_entry cat_message_table_v02[] = {
  {0, 0},
  {sizeof(cat_reset_resp_msg_v02), cat_reset_resp_msg_data_v02},
  {sizeof(cat_set_event_report_req_msg_v02), cat_set_event_report_req_msg_data_v02},
  {sizeof(cat_set_event_report_resp_msg_v02), cat_set_event_report_resp_msg_data_v02},
  {sizeof(cat_event_report_ind_msg_v02), cat_event_report_ind_msg_data_v02},
  {0, 0},
  {sizeof(cat_get_service_state_resp_msg_v02), cat_get_service_state_resp_msg_data_v02},
  {sizeof(cat_send_tr_req_msg_v02), cat_send_tr_req_msg_data_v02},
  {sizeof(cat_send_tr_resp_msg_v02), cat_send_tr_resp_msg_data_v02},
  {sizeof(cat_send_envelope_cmd_req_msg_v02), cat_send_envelope_cmd_req_msg_data_v02},
  {sizeof(cat_send_envelope_cmd_resp_msg_v02), cat_send_envelope_cmd_resp_msg_data_v02},
  {sizeof(cat_get_event_report_req_msg_v02), cat_get_event_report_req_msg_data_v02},
  {sizeof(cat_get_event_report_resp_msg_v02), cat_get_event_report_resp_msg_data_v02},
  {sizeof(cat_send_decoded_tr_req_msg_v02), cat_send_decoded_tr_req_msg_data_v02},
  {sizeof(cat_send_decoded_tr_resp_msg_v02), cat_send_decoded_tr_resp_msg_data_v02},
  {sizeof(cat_send_decoded_envelope_cmd_req_msg_v02), cat_send_decoded_envelope_cmd_req_msg_data_v02},
  {sizeof(cat_send_decoded_envelope_cmd_resp_msg_v02), cat_send_decoded_envelope_cmd_resp_msg_data_v02},
  {sizeof(cat_event_confirmation_req_msg_v02), cat_event_confirmation_req_msg_data_v02},
  {sizeof(cat_event_confirmation_resp_msg_v02), cat_event_confirmation_resp_msg_data_v02},
  {sizeof(cat_scws_open_channel_req_msg_v02), cat_scws_open_channel_req_msg_data_v02},
  {sizeof(cat_scws_open_channel_resp_msg_v02), cat_scws_open_channel_resp_msg_data_v02},
  {sizeof(cat_scws_open_channel_ind_msg_v02), cat_scws_open_channel_ind_msg_data_v02},
  {sizeof(cat_scws_close_channel_req_msg_v02), cat_scws_close_channel_req_msg_data_v02},
  {sizeof(cat_scws_close_channel_resp_msg_v02), cat_scws_close_channel_resp_msg_data_v02},
  {sizeof(cat_scws_close_channel_ind_msg_v02), cat_scws_close_channel_ind_msg_data_v02},
  {sizeof(cat_scws_send_data_req_msg_v02), cat_scws_send_data_req_msg_data_v02},
  {sizeof(cat_scws_send_data_resp_msg_v02), cat_scws_send_data_resp_msg_data_v02},
  {sizeof(cat_scws_send_data_ind_msg_v02), cat_scws_send_data_ind_msg_data_v02},
  {sizeof(cat_scws_data_available_req_msg_v02), cat_scws_data_available_req_msg_data_v02},
  {sizeof(cat_scws_data_available_resp_msg_v02), cat_scws_data_available_resp_msg_data_v02},
  {sizeof(cat_scws_channel_status_req_msg_v02), cat_scws_channel_status_req_msg_data_v02},
  {sizeof(cat_scws_channel_status_resp_msg_v02), cat_scws_channel_status_resp_msg_data_v02},
  {sizeof(cat_get_terminal_profile_req_msg_v02), cat_get_terminal_profile_req_msg_data_v02},
  {sizeof(cat_get_terminal_profile_resp_msg_v02), cat_get_terminal_profile_resp_msg_data_v02},
  {sizeof(cat_set_configuration_req_msg_v02), cat_set_configuration_req_msg_data_v02},
  {sizeof(cat_set_configuration_resp_msg_v02), cat_set_configuration_resp_msg_data_v02},
  {0, 0},
  {sizeof(cat_get_configuration_resp_msg_v02), cat_get_configuration_resp_msg_data_v02}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object cat_qmi_idl_type_table_object_v02;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *cat_qmi_idl_type_table_object_referenced_tables_v02[] =
{&cat_qmi_idl_type_table_object_v02, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object cat_qmi_idl_type_table_object_v02 = {
  sizeof(cat_type_table_v02)/sizeof(qmi_idl_type_table_entry ),
  sizeof(cat_message_table_v02)/sizeof(qmi_idl_message_table_entry),
  1,
  cat_type_table_v02,
  cat_message_table_v02,
  cat_qmi_idl_type_table_object_referenced_tables_v02
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry cat_service_command_messages_v02[] = {
  {QMI_CAT_RESET_REQ_V02, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_CAT_SET_EVENT_REPORT_REQ_V02, QMI_IDL_TYPE16(0, 2), 25},
  {QMI_CAT_GET_SUPPORTED_MSGS_REQ_V02, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_CAT_GET_SUPPORTED_FIELDS_REQ_V02, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_CAT_GET_SERVICE_STATE_REQ_V02, QMI_IDL_TYPE16(0, 5), 0},
  {QMI_CAT_SEND_TR_REQ_V02, QMI_IDL_TYPE16(0, 7), 268},
  {QMI_CAT_SEND_ENVELOPE_CMD_REQ_V02, QMI_IDL_TYPE16(0, 9), 269},
  {QMI_CAT_GET_EVENT_REPORT_REQ_V02, QMI_IDL_TYPE16(0, 11), 8},
  {QMI_CAT_SEND_DECODED_TR_REQ_V02, QMI_IDL_TYPE16(0, 13), 562},
  {QMI_CAT_SEND_DECODED_ENVELOPE_CMD_REQ_V02, QMI_IDL_TYPE16(0, 15), 2536},
  {QMI_CAT_EVENT_CONFIRMATION_REQ_V02, QMI_IDL_TYPE16(0, 17), 12},
  {QMI_CAT_SCWS_OPEN_CHANNEL_REQ_V02, QMI_IDL_TYPE16(0, 19), 12},
  {QMI_CAT_SCWS_CLOSE_CHANNEL_REQ_V02, QMI_IDL_TYPE16(0, 22), 12},
  {QMI_CAT_SCWS_SEND_DATA_REQ_V02, QMI_IDL_TYPE16(0, 25), 12},
  {QMI_CAT_SCWS_DATA_AVAILABLE_REQ_V02, QMI_IDL_TYPE16(0, 28), 1018},
  {QMI_CAT_SCWS_CHANNEL_STATUS_REQ_V02, QMI_IDL_TYPE16(0, 30), 12},
  {QMI_CAT_GET_TERMINAL_PROFILE_REQ_V02, QMI_IDL_TYPE16(0, 32), 4},
  {QMI_CAT_SET_CONFIGURATION_REQ_V02, QMI_IDL_TYPE16(0, 34), 88},
  {QMI_CAT_GET_CONFIGURATION_REQ_V02, QMI_IDL_TYPE16(0, 36), 0}
};

static const qmi_idl_service_message_table_entry cat_service_response_messages_v02[] = {
  {QMI_CAT_RESET_RESP_V02, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_CAT_SET_EVENT_REPORT_RESP_V02, QMI_IDL_TYPE16(0, 3), 28},
  {QMI_CAT_GET_SUPPORTED_MSGS_RESP_V02, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_CAT_GET_SUPPORTED_FIELDS_RESP_V02, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_CAT_GET_SERVICE_STATE_RESP_V02, QMI_IDL_TYPE16(0, 6), 36},
  {QMI_CAT_SEND_TR_RESP_V02, QMI_IDL_TYPE16(0, 8), 268},
  {QMI_CAT_SEND_EVENLOPE_CMD_RESP_V02, QMI_IDL_TYPE16(0, 10), 268},
  {QMI_CAT_GET_EVENT_REPORT_RESP_V02, QMI_IDL_TYPE16(0, 12), 65535},
  {QMI_CAT_SEND_DECODED_TR_RESP_V02, QMI_IDL_TYPE16(0, 14), 268},
  {QMI_CAT_SEND_DECODED_ENVELOPE_CMD_RESP_V02, QMI_IDL_TYPE16(0, 16), 2345},
  {QMI_CAT_EVENT_CONFIRMATION_RESP_V02, QMI_IDL_TYPE16(0, 18), 7},
  {QMI_CAT_SCWS_OPEN_CHANNEL_RESP_V02, QMI_IDL_TYPE16(0, 20), 7},
  {QMI_CAT_SCWS_CLOSE_CHANNEL_RESP_V02, QMI_IDL_TYPE16(0, 23), 7},
  {QMI_CAT_SCWS_SEND_DATA_RESP_V02, QMI_IDL_TYPE16(0, 26), 7},
  {QMI_CAT_SCWS_DATA_AVAILABLEA_RESP_V02, QMI_IDL_TYPE16(0, 29), 7},
  {QMI_CAT_SCWS_CHANNEL_STATUS_RESP_V02, QMI_IDL_TYPE16(0, 31), 7},
  {QMI_CAT_GET_TERMINAL_PROFILE_RESP_V02, QMI_IDL_TYPE16(0, 33), 91},
  {QMI_CAT_SET_CONFIGURATION_RESP_V02, QMI_IDL_TYPE16(0, 35), 7},
  {QMI_CAT_GET_CONFIGURATION_RESP_V02, QMI_IDL_TYPE16(0, 37), 95}
};

static const qmi_idl_service_message_table_entry cat_service_indication_messages_v02[] = {
  {QMI_CAT_EVENT_REPORT_IND_V02, QMI_IDL_TYPE16(0, 4), 65535},
  {QMI_CAT_SCWS_OPEN_CHANNEL_IND_V02, QMI_IDL_TYPE16(0, 21), 275},
  {QMI_CAT_SCWS_CLOSE_CHANNEL_IND_V02, QMI_IDL_TYPE16(0, 24), 12},
  {QMI_CAT_SCWS_SEND_DATA_IND_V02, QMI_IDL_TYPE16(0, 27), 1015}
};

/*Service Object*/
struct qmi_idl_service_object cat_qmi_idl_service_object_v02 = {
  0x05,
  0x02,
  0x0A,
  65535,
  { sizeof(cat_service_command_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(cat_service_response_messages_v02)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(cat_service_indication_messages_v02)/sizeof(qmi_idl_service_message_table_entry) },
  { cat_service_command_messages_v02, cat_service_response_messages_v02, cat_service_indication_messages_v02},
  &cat_qmi_idl_type_table_object_v02,
  0x17,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type cat_get_service_object_internal_v02
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( CAT_V02_IDL_MAJOR_VERS != idl_maj_version || CAT_V02_IDL_MINOR_VERS != idl_min_version 
       || CAT_V02_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&cat_qmi_idl_service_object_v02;
}

