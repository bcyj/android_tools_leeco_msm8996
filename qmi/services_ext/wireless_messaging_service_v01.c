/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        W I R E L E S S _ M E S S A G I N G _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the wms service Data structures.

  Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/wms/main/latest/src/wireless_messaging_service_v01.c#25 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Thu Jul 25 2013 (Spin 1)
   From IDL File: wireless_messaging_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "wireless_messaging_service_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t wms_mt_message_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_mt_message_type_v01, storage_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_mt_message_type_v01, storage_index),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_transfer_route_mt_message_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_transfer_route_mt_message_type_v01, ack_indicator),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_transfer_route_mt_message_type_v01, transaction_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_transfer_route_mt_message_type_v01, format),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_transfer_route_mt_message_type_v01, data),
  ((WMS_MESSAGE_LENGTH_MAX_V01) & 0xFF), ((WMS_MESSAGE_LENGTH_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_transfer_route_mt_message_type_v01, data) - QMI_IDL_OFFSET8(wms_transfer_route_mt_message_type_v01, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_etws_message_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_etws_message_type_v01, notification_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_etws_message_type_v01, data),
  ((WMS_ETWS_MESSAGE_LENGTH_MAX_V01) & 0xFF), ((WMS_ETWS_MESSAGE_LENGTH_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_etws_message_type_v01, data) - QMI_IDL_OFFSET8(wms_etws_message_type_v01, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_etws_plmn_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wms_etws_plmn_info_type_v01, mobile_country_code),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wms_etws_plmn_info_type_v01, mobile_network_code),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_mt_message_smsc_address_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_mt_message_smsc_address_type_v01, data),
  WMS_SMSC_ADDRESS_LENGTH_MAX_V01,
  QMI_IDL_OFFSET8(wms_mt_message_smsc_address_type_v01, data) - QMI_IDL_OFFSET8(wms_mt_message_smsc_address_type_v01, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_send_raw_message_data_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_send_raw_message_data_type_v01, format),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_send_raw_message_data_type_v01, raw_message),
  ((WMS_MESSAGE_LENGTH_MAX_V01) & 0xFF), ((WMS_MESSAGE_LENGTH_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_send_raw_message_data_type_v01, raw_message) - QMI_IDL_OFFSET8(wms_send_raw_message_data_type_v01, raw_message_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_force_on_dc_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_force_on_dc_type_v01, force_on_dc),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_force_on_dc_type_v01, so),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_gw_cause_info_type_data_v01[] = {
  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_gw_cause_info_type_v01, rp_cause),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_gw_cause_info_type_v01, tp_cause),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_call_control_modified_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_call_control_modified_info_type_v01, alpha_id),
  WMS_ALPHA_ID_LENGTH_MAX_V01,
  QMI_IDL_OFFSET8(wms_call_control_modified_info_type_v01, alpha_id) - QMI_IDL_OFFSET8(wms_call_control_modified_info_type_v01, alpha_id_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_raw_message_write_data_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_raw_message_write_data_type_v01, storage_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_raw_message_write_data_type_v01, format),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_raw_message_write_data_type_v01, raw_message),
  ((WMS_MESSAGE_LENGTH_MAX_V01) & 0xFF), ((WMS_MESSAGE_LENGTH_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_raw_message_write_data_type_v01, raw_message) - QMI_IDL_OFFSET8(wms_raw_message_write_data_type_v01, raw_message_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_message_memory_storage_identification_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_message_memory_storage_identification_type_v01, storage_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_message_memory_storage_identification_type_v01, storage_index),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_read_raw_message_data_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_read_raw_message_data_type_v01, tag_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_read_raw_message_data_type_v01, format),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_read_raw_message_data_type_v01, data),
  ((WMS_MESSAGE_LENGTH_MAX_V01) & 0xFF), ((WMS_MESSAGE_LENGTH_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_read_raw_message_data_type_v01, data) - QMI_IDL_OFFSET8(wms_read_raw_message_data_type_v01, data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_message_tag_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_message_tag_type_v01, storage_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_message_tag_type_v01, storage_index),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_message_tag_type_v01, tag_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_message_tuple_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_message_tuple_type_v01, message_index),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_message_tuple_type_v01, tag_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_set_route_list_tuple_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_route_list_tuple_type_v01, message_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_route_list_tuple_type_v01, message_class),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_route_list_tuple_type_v01, route_storage),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_route_list_tuple_type_v01, receipt_action),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_get_route_list_tuple_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_route_list_tuple_type_v01, route_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_route_list_tuple_type_v01, route_class),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_route_list_tuple_type_v01, route_memory),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_route_list_tuple_type_v01, route_value),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_smsc_address_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_smsc_address_type_v01, smsc_address_type),
  WMS_ADDRESS_TYPE_MAX_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_smsc_address_type_v01, smsc_address_digits),
  WMS_ADDRESS_DIGIT_MAX_V01,
  QMI_IDL_OFFSET8(wms_smsc_address_type_v01, smsc_address_digits) - QMI_IDL_OFFSET8(wms_smsc_address_type_v01, smsc_address_digits_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_ack_information_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_ack_information_type_v01, transaction_id),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_ack_information_type_v01, message_protocol),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_ack_information_type_v01, success),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_3gpp2_failure_information_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_3gpp2_failure_information_type_v01, error_class),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_3gpp2_failure_information_type_v01, tl_status),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_3gpp_failure_information_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_3gpp_failure_information_type_v01, rp_cause),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_3gpp_failure_information_type_v01, tp_cause),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_broadcast_activation_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_broadcast_activation_info_type_v01, message_mode),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_broadcast_activation_info_type_v01, bc_activate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_3gpp_broadcast_config_info_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wms_3gpp_broadcast_config_info_type_v01, from_service_id),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wms_3gpp_broadcast_config_info_type_v01, to_service_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_3gpp_broadcast_config_info_type_v01, selected),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_3gpp2_broadcast_config_info_type_data_v01[] = {
  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_3gpp2_broadcast_config_info_type_v01, service_category),

  QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_3gpp2_broadcast_config_info_type_v01, language),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_3gpp2_broadcast_config_info_type_v01, selected),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_3gpp_broadcast_info_config2_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_3gpp_broadcast_info_config2_type_v01, activated_ind),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_3gpp_broadcast_info_config2_type_v01, wms_3gpp_broadcast_config_info),
  ((WMS_3GPP_BROADCAST_CONFIG_MAX_V01) & 0xFF), ((WMS_3GPP_BROADCAST_CONFIG_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_3gpp_broadcast_info_config2_type_v01, wms_3gpp_broadcast_config_info) - QMI_IDL_OFFSET8(wms_3gpp_broadcast_info_config2_type_v01, wms_3gpp_broadcast_config_info_len),
  QMI_IDL_TYPE88(0, 21),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_3gpp2_broadcast_info_config2_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_3gpp2_broadcast_info_config2_type_v01, activated_ind),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_3gpp2_broadcast_info_config2_type_v01, wms_3gpp2_broadcast_config_info),
  ((WMS_3GPP2_BROADCAST_CONFIG_MAX_V01) & 0xFF), ((WMS_3GPP2_BROADCAST_CONFIG_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_3gpp2_broadcast_info_config2_type_v01, wms_3gpp2_broadcast_config_info) - QMI_IDL_OFFSET8(wms_3gpp2_broadcast_info_config2_type_v01, wms_3gpp2_broadcast_config_info_len),
  QMI_IDL_TYPE88(0, 22),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_memory_full_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_memory_full_info_type_v01, storage_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_memory_full_info_type_v01, message_mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_message_memory_storage_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_message_memory_storage_info_type_v01, storage_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_message_memory_storage_info_type_v01, storage_index),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_message_memory_storage_info_type_v01, message_mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_message_waiting_information_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_message_waiting_information_type_v01, message_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_message_waiting_information_type_v01, active_ind),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_message_waiting_information_type_v01, message_count),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_transport_layer_info_type_data_v01[] = {
  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_transport_layer_info_type_v01, transport_type),

  QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_transport_layer_info_type_v01, transport_cap),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t wms_call_control_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_call_control_info_type_v01, mo_control_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_call_control_info_type_v01, alpha_id),
  WMS_ALPHA_ID_LENGTH_MAX_V01,
  QMI_IDL_OFFSET8(wms_call_control_info_type_v01, alpha_id) - QMI_IDL_OFFSET8(wms_call_control_info_type_v01, alpha_id_len),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * wms_reset_req_msg is empty
 * static const uint8_t wms_reset_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_reset_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_reset_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_event_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_mt_message) - QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_mt_message_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_mt_message),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_call_control_info) - QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_call_control_info_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_call_control_info),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_mwi_message) - QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_mwi_message_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_event_report_req_msg_v01, report_mwi_message)
};

static const uint8_t wms_set_event_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_event_report_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_event_report_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_event_report_ind_msg_v01, mt_message) - QMI_IDL_OFFSET8(wms_event_report_ind_msg_v01, mt_message_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_event_report_ind_msg_v01, mt_message),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_event_report_ind_msg_v01, transfer_route_mt_message) - QMI_IDL_OFFSET8(wms_event_report_ind_msg_v01, transfer_route_mt_message_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_event_report_ind_msg_v01, transfer_route_mt_message),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, message_mode) - QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, message_mode_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(wms_event_report_ind_msg_v01, message_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, etws_message) - QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, etws_message_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_event_report_ind_msg_v01, etws_message),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, etws_plmn_info) - QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, etws_plmn_info_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_event_report_ind_msg_v01, etws_plmn_info),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, mt_message_smsc_address) - QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, mt_message_smsc_address_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_event_report_ind_msg_v01, mt_message_smsc_address),
  QMI_IDL_TYPE88(0, 4),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, sms_on_ims) - QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, sms_on_ims_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_event_report_ind_msg_v01, sms_on_ims),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, call_control_info) - QMI_IDL_OFFSET16RELATIVE(wms_event_report_ind_msg_v01, call_control_info_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_event_report_ind_msg_v01, call_control_info),
  QMI_IDL_TYPE88(0, 29)
};

static const uint8_t wms_raw_send_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_raw_send_req_msg_v01, raw_message_data),
  QMI_IDL_TYPE88(0, 5),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, force_on_dc) - QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, force_on_dc_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_raw_send_req_msg_v01, force_on_dc),
  QMI_IDL_TYPE88(0, 6),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, follow_on_dc) - QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, follow_on_dc_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(wms_raw_send_req_msg_v01, follow_on_dc),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, link_timer) - QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, link_timer_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_raw_send_req_msg_v01, link_timer),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, sms_on_ims) - QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, sms_on_ims_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_raw_send_req_msg_v01, sms_on_ims),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, retry_message) - QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, retry_message_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(wms_raw_send_req_msg_v01, retry_message),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, retry_message_id) - QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, retry_message_id_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_raw_send_req_msg_v01, retry_message_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, link_enable_mode) - QMI_IDL_OFFSET16RELATIVE(wms_raw_send_req_msg_v01, link_enable_mode_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_raw_send_req_msg_v01, link_enable_mode)
};

static const uint8_t wms_raw_send_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, message_id),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, cause_code) - QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, cause_code_valid)),
  0x10,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, cause_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, error_class) - QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, error_class_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, error_class),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, gw_cause_info) - QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, gw_cause_info_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, gw_cause_info),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, message_delivery_failure_type) - QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, message_delivery_failure_type_valid)),
  0x13,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, message_delivery_failure_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, message_delivery_failure_cause) - QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, message_delivery_failure_cause_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, message_delivery_failure_cause),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, call_control_modified_info) - QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, call_control_modified_info_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_raw_send_resp_msg_v01, call_control_modified_info),
  QMI_IDL_TYPE88(0, 8)
};

static const uint8_t wms_raw_write_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_raw_write_req_msg_v01, raw_message_write_data),
  QMI_IDL_TYPE88(0, 9),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_raw_write_req_msg_v01, tag_type) - QMI_IDL_OFFSET16RELATIVE(wms_raw_write_req_msg_v01, tag_type_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(wms_raw_write_req_msg_v01, tag_type)
};

static const uint8_t wms_raw_write_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_raw_write_resp_msg_v01, storage_index),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_raw_write_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_raw_read_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_raw_read_req_msg_v01, message_memory_storage_identification),
  QMI_IDL_TYPE88(0, 10),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_raw_read_req_msg_v01, message_mode) - QMI_IDL_OFFSET8(wms_raw_read_req_msg_v01, message_mode_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_raw_read_req_msg_v01, message_mode),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_raw_read_req_msg_v01, sms_on_ims) - QMI_IDL_OFFSET8(wms_raw_read_req_msg_v01, sms_on_ims_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_raw_read_req_msg_v01, sms_on_ims)
};

static const uint8_t wms_raw_read_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_raw_read_resp_msg_v01, raw_message_data),
  QMI_IDL_TYPE88(0, 11),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_raw_read_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_modify_tag_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_modify_tag_req_msg_v01, wms_message_tag),
  QMI_IDL_TYPE88(0, 12),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_modify_tag_req_msg_v01, message_mode) - QMI_IDL_OFFSET8(wms_modify_tag_req_msg_v01, message_mode_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_modify_tag_req_msg_v01, message_mode)
};

static const uint8_t wms_modify_tag_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_modify_tag_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_delete_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_delete_req_msg_v01, storage_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_delete_req_msg_v01, index) - QMI_IDL_OFFSET8(wms_delete_req_msg_v01, index_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_delete_req_msg_v01, index),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_delete_req_msg_v01, tag_type) - QMI_IDL_OFFSET8(wms_delete_req_msg_v01, tag_type_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_delete_req_msg_v01, tag_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_delete_req_msg_v01, message_mode) - QMI_IDL_OFFSET8(wms_delete_req_msg_v01, message_mode_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_delete_req_msg_v01, message_mode)
};

static const uint8_t wms_delete_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_delete_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * wms_get_message_protocol_req_msg is empty
 * static const uint8_t wms_get_message_protocol_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_message_protocol_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_message_protocol_resp_msg_v01, message_protocol),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_message_protocol_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_list_messages_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_list_messages_req_msg_v01, storage_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_list_messages_req_msg_v01, tag_type) - QMI_IDL_OFFSET8(wms_list_messages_req_msg_v01, tag_type_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_list_messages_req_msg_v01, tag_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_list_messages_req_msg_v01, message_mode) - QMI_IDL_OFFSET8(wms_list_messages_req_msg_v01, message_mode_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_list_messages_req_msg_v01, message_mode)
};

static const uint8_t wms_list_messages_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_FIRST_EXTENDED |   QMI_IDL_AGGREGATE,
  QMI_IDL_FLAGS_SZ_IS_32,
  QMI_IDL_OFFSET8(wms_list_messages_resp_msg_v01, message_tuple),
  ((WMS_MESSAGE_TUPLE_MAX_V01) & 0xFF), ((WMS_MESSAGE_TUPLE_MAX_V01) >> 8), 0, 0,
  QMI_IDL_OFFSET8(wms_list_messages_resp_msg_v01, message_tuple) - QMI_IDL_OFFSET8(wms_list_messages_resp_msg_v01, message_tuple_len),
  QMI_IDL_TYPE88(0, 13),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_list_messages_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_routes_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_routes_req_msg_v01, route_list_tuple),
  ((WMS_ROUTE_TUPLE_MAX_V01) & 0xFF), ((WMS_ROUTE_TUPLE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_set_routes_req_msg_v01, route_list_tuple) - QMI_IDL_OFFSET8(wms_set_routes_req_msg_v01, route_list_tuple_len),
  QMI_IDL_TYPE88(0, 14),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_routes_req_msg_v01, transfer_ind) - QMI_IDL_OFFSET8(wms_set_routes_req_msg_v01, transfer_ind_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_routes_req_msg_v01, transfer_ind)
};

static const uint8_t wms_set_routes_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_routes_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * wms_get_routes_req_msg is empty
 * static const uint8_t wms_get_routes_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_routes_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_routes_resp_msg_v01, route_list),
  ((WMS_ROUTE_TUPLE_MAX_V01) & 0xFF), ((WMS_ROUTE_TUPLE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_get_routes_resp_msg_v01, route_list) - QMI_IDL_OFFSET8(wms_get_routes_resp_msg_v01, route_list_len),
  QMI_IDL_TYPE88(0, 15),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_routes_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_routes_resp_msg_v01, transfer_ind) - QMI_IDL_OFFSET8(wms_get_routes_resp_msg_v01, transfer_ind_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_routes_resp_msg_v01, transfer_ind)
};

/* 
 * wms_get_smsc_address_req_msg is empty
 * static const uint8_t wms_get_smsc_address_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_smsc_address_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_smsc_address_resp_msg_v01, smsc_address),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_smsc_address_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_smsc_address_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(wms_set_smsc_address_req_msg_v01, smsc_address_digits),
  WMS_ADDRESS_DIGIT_MAX_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_smsc_address_req_msg_v01, smsc_address_type) - QMI_IDL_OFFSET8(wms_set_smsc_address_req_msg_v01, smsc_address_type_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(wms_set_smsc_address_req_msg_v01, smsc_address_type),
  WMS_ADDRESS_TYPE_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_smsc_address_req_msg_v01, index) - QMI_IDL_OFFSET8(wms_set_smsc_address_req_msg_v01, index_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_smsc_address_req_msg_v01, index)
};

static const uint8_t wms_set_smsc_address_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_smsc_address_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_get_store_max_size_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_store_max_size_req_msg_v01, storage_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_store_max_size_req_msg_v01, message_mode) - QMI_IDL_OFFSET8(wms_get_store_max_size_req_msg_v01, message_mode_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_store_max_size_req_msg_v01, message_mode)
};

static const uint8_t wms_get_store_max_size_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_get_store_max_size_resp_msg_v01, mem_store_max_size),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_store_max_size_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_store_max_size_resp_msg_v01, free_slots) - QMI_IDL_OFFSET8(wms_get_store_max_size_resp_msg_v01, free_slots_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_get_store_max_size_resp_msg_v01, free_slots)
};

static const uint8_t wms_send_ack_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, ack_information),
  QMI_IDL_TYPE88(0, 17),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, wms_3gpp2_failure_information) - QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, wms_3gpp2_failure_information_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, wms_3gpp2_failure_information),
  QMI_IDL_TYPE88(0, 18),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, wms_3gpp_failure_information) - QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, wms_3gpp_failure_information_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, wms_3gpp_failure_information),
  QMI_IDL_TYPE88(0, 19),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, sms_on_ims) - QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, sms_on_ims_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_send_ack_req_msg_v01, sms_on_ims)
};

static const uint8_t wms_send_ack_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_send_ack_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_ack_resp_msg_v01, failure_cause) - QMI_IDL_OFFSET8(wms_send_ack_resp_msg_v01, failure_cause_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_send_ack_resp_msg_v01, failure_cause)
};

static const uint8_t wms_set_retry_period_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_set_retry_period_req_msg_v01, retry_period)
};

static const uint8_t wms_set_retry_period_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_retry_period_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_retry_interval_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_set_retry_interval_req_msg_v01, retry_interval)
};

static const uint8_t wms_set_retry_interval_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_retry_interval_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_dc_disconnect_timer_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_set_dc_disconnect_timer_req_msg_v01, dc_auto_disconn_timer)
};

static const uint8_t wms_set_dc_disconnect_timer_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_dc_disconnect_timer_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_memory_status_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_memory_status_req_msg_v01, memory_available)
};

static const uint8_t wms_set_memory_status_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_memory_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_broadcast_activation_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_broadcast_activation_req_msg_v01, broadcast_activation_info),
  QMI_IDL_TYPE88(0, 20),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_broadcast_activation_req_msg_v01, activate_all) - QMI_IDL_OFFSET8(wms_set_broadcast_activation_req_msg_v01, activate_all_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_broadcast_activation_req_msg_v01, activate_all)
};

static const uint8_t wms_set_broadcast_activation_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_broadcast_activation_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_broadcast_config_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_broadcast_config_req_msg_v01, message_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_broadcast_config_req_msg_v01, wms_3gpp_broadcast_config_info) - QMI_IDL_OFFSET8(wms_set_broadcast_config_req_msg_v01, wms_3gpp_broadcast_config_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_broadcast_config_req_msg_v01, wms_3gpp_broadcast_config_info),
  ((WMS_3GPP_BROADCAST_CONFIG_MAX_V01) & 0xFF), ((WMS_3GPP_BROADCAST_CONFIG_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(wms_set_broadcast_config_req_msg_v01, wms_3gpp_broadcast_config_info) - QMI_IDL_OFFSET8(wms_set_broadcast_config_req_msg_v01, wms_3gpp_broadcast_config_info_len),
  QMI_IDL_TYPE88(0, 21),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_set_broadcast_config_req_msg_v01, wms_3gpp2_broadcast_config_info) - QMI_IDL_OFFSET16RELATIVE(wms_set_broadcast_config_req_msg_v01, wms_3gpp2_broadcast_config_info_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_set_broadcast_config_req_msg_v01, wms_3gpp2_broadcast_config_info),
  ((WMS_3GPP2_BROADCAST_CONFIG_MAX_V01) & 0xFF), ((WMS_3GPP2_BROADCAST_CONFIG_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(wms_set_broadcast_config_req_msg_v01, wms_3gpp2_broadcast_config_info) - QMI_IDL_OFFSET16RELATIVE(wms_set_broadcast_config_req_msg_v01, wms_3gpp2_broadcast_config_info_len),
  QMI_IDL_TYPE88(0, 22)
};

static const uint8_t wms_set_broadcast_config_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_broadcast_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_get_broadcast_config_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_broadcast_config_req_msg_v01, message_mode)
};

static const uint8_t wms_get_broadcast_config_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_broadcast_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_broadcast_config_resp_msg_v01, wms_3gpp_broadcast_info) - QMI_IDL_OFFSET8(wms_get_broadcast_config_resp_msg_v01, wms_3gpp_broadcast_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_broadcast_config_resp_msg_v01, wms_3gpp_broadcast_info),
  QMI_IDL_TYPE88(0, 23),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_get_broadcast_config_resp_msg_v01, wms_3gpp2_broadcast_info) - QMI_IDL_OFFSET16RELATIVE(wms_get_broadcast_config_resp_msg_v01, wms_3gpp2_broadcast_info_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_get_broadcast_config_resp_msg_v01, wms_3gpp2_broadcast_info),
  QMI_IDL_TYPE88(0, 24)
};

static const uint8_t wms_memory_full_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_memory_full_ind_msg_v01, memory_full_info),
  QMI_IDL_TYPE88(0, 25)
};

/* 
 * wms_get_domain_pref_req_msg is empty
 * static const uint8_t wms_get_domain_pref_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_domain_pref_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_domain_pref_resp_msg_v01, domain_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_domain_pref_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_set_domain_pref_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_domain_pref_req_msg_v01, domain_pref)
};

static const uint8_t wms_set_domain_pref_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_domain_pref_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_send_from_mem_store_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_send_from_mem_store_req_msg_v01, message_memory_storage_info),
  QMI_IDL_TYPE88(0, 26),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_from_mem_store_req_msg_v01, sms_on_ims) - QMI_IDL_OFFSET8(wms_send_from_mem_store_req_msg_v01, sms_on_ims_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_send_from_mem_store_req_msg_v01, sms_on_ims)
};

static const uint8_t wms_send_from_mem_store_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, message_id) - QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, message_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, message_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, cause_code) - QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, cause_code_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, cause_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, error_class) - QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, error_class_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, error_class),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, gw_cause_info) - QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, gw_cause_info_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, gw_cause_info),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, message_delivery_failure_type) - QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, message_delivery_failure_type_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_send_from_mem_store_resp_msg_v01, message_delivery_failure_type)
};

/* 
 * wms_get_message_waiting_req_msg is empty
 * static const uint8_t wms_get_message_waiting_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_message_waiting_resp_msg_data_v01[] = {
  0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_message_waiting_resp_msg_v01, message_waiting_info),
  WMS_MESSAGE_TUPLE_MAX_V01,
  QMI_IDL_OFFSET8(wms_get_message_waiting_resp_msg_v01, message_waiting_info) - QMI_IDL_OFFSET8(wms_get_message_waiting_resp_msg_v01, message_waiting_info_len),
  QMI_IDL_TYPE88(0, 27),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_get_message_waiting_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_message_waiting_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_message_waiting_ind_msg_v01, message_waiting_info),
  WMS_MESSAGE_TUPLE_MAX_V01,
  QMI_IDL_OFFSET8(wms_message_waiting_ind_msg_v01, message_waiting_info) - QMI_IDL_OFFSET8(wms_message_waiting_ind_msg_v01, message_waiting_info_len),
  QMI_IDL_TYPE88(0, 27)
};

static const uint8_t wms_set_primary_client_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_primary_client_req_msg_v01, primary_client)
};

static const uint8_t wms_set_primary_client_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_primary_client_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_smsc_address_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_smsc_address_ind_msg_v01, smsc_address),
  QMI_IDL_TYPE88(0, 16)
};

static const uint8_t wms_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_transport_layer_info_events) - QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_transport_layer_info_events_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_transport_layer_info_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_transport_nw_reg_info_events) - QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_transport_nw_reg_info_events_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_transport_nw_reg_info_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_call_status_info_events) - QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_call_status_info_events_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_call_status_info_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_service_ready_events) - QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_service_ready_events_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_service_ready_events),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_broadcast_config_events) - QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_broadcast_config_events_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_indication_register_req_msg_v01, reg_broadcast_config_events)
};

static const uint8_t wms_indication_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * wms_get_transport_layer_req_msg is empty
 * static const uint8_t wms_get_transport_layer_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_transport_layer_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_transport_layer_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_transport_layer_resp_msg_v01, registered_ind) - QMI_IDL_OFFSET8(wms_get_transport_layer_resp_msg_v01, registered_ind_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_transport_layer_resp_msg_v01, registered_ind),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_transport_layer_resp_msg_v01, transport_layer_info) - QMI_IDL_OFFSET8(wms_get_transport_layer_resp_msg_v01, transport_layer_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_transport_layer_resp_msg_v01, transport_layer_info),
  QMI_IDL_TYPE88(0, 28)
};

static const uint8_t wms_transport_layer_info_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_transport_layer_info_ind_msg_v01, registered_ind),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_transport_layer_info_ind_msg_v01, transport_layer_info) - QMI_IDL_OFFSET8(wms_transport_layer_info_ind_msg_v01, transport_layer_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_transport_layer_info_ind_msg_v01, transport_layer_info),
  QMI_IDL_TYPE88(0, 28)
};

/* 
 * wms_get_transport_nw_reg_req_msg is empty
 * static const uint8_t wms_get_transport_nw_reg_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_transport_nw_reg_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_transport_nw_reg_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_transport_nw_reg_resp_msg_v01, transport_nw_reg_status) - QMI_IDL_OFFSET8(wms_get_transport_nw_reg_resp_msg_v01, transport_nw_reg_status_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_transport_nw_reg_resp_msg_v01, transport_nw_reg_status)
};

static const uint8_t wms_transport_nw_reg_info_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_transport_nw_reg_info_ind_msg_v01, transport_nw_reg_status)
};

static const uint8_t wms_bind_subscription_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_bind_subscription_req_msg_v01, subs_type)
};

static const uint8_t wms_bind_subscription_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * wms_get_indication_register_req_msg is empty
 * static const uint8_t wms_get_indication_register_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_indication_register_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_transport_layer_info_events) - QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_transport_layer_info_events_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_transport_layer_info_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_transport_nw_reg_info_events) - QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_transport_nw_reg_info_events_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_transport_nw_reg_info_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_call_status_info_events) - QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_call_status_info_events_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_call_status_info_events),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_service_ready_events) - QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_service_ready_events_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_service_ready_events),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_broadcast_config_events) - QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_broadcast_config_events_valid)),
  0x14,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_indication_register_resp_msg_v01, reg_broadcast_config_events)
};

static const uint8_t wms_get_sms_parameters_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_sms_parameters_req_msg_v01, message_mode)
};

static const uint8_t wms_get_sms_parameters_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, dest_addr) - QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, dest_addr_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, dest_addr),
  WMS_DEST_ADDRESS_LENGTH_MAX_V01,
  QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, dest_addr) - QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, dest_addr_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, pid) - QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, pid_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, pid),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, dcs) - QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, dcs_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, dcs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, validity) - QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, validity_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_sms_parameters_resp_msg_v01, validity)
};

static const uint8_t wms_set_sms_parameters_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, message_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, dest_addr) - QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, dest_addr_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, dest_addr),
  WMS_DEST_ADDRESS_LENGTH_MAX_V01,
  QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, dest_addr) - QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, dest_addr_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, pid) - QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, pid_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, pid),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, dcs) - QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, dcs_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, dcs),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, validity) - QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, validity_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_set_sms_parameters_req_msg_v01, validity)
};

static const uint8_t wms_set_sms_parameters_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_sms_parameters_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_call_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_call_status_ind_msg_v01, call_status)
};

/* 
 * wms_get_domain_pref_config_req_msg is empty
 * static const uint8_t wms_get_domain_pref_config_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_domain_pref_config_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_domain_pref_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_domain_pref_config_resp_msg_v01, lte_domain_pref) - QMI_IDL_OFFSET8(wms_get_domain_pref_config_resp_msg_v01, lte_domain_pref_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_domain_pref_config_resp_msg_v01, lte_domain_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_domain_pref_config_resp_msg_v01, gw_domain_pref) - QMI_IDL_OFFSET8(wms_get_domain_pref_config_resp_msg_v01, gw_domain_pref_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_domain_pref_config_resp_msg_v01, gw_domain_pref)
};

static const uint8_t wms_set_domain_pref_config_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_domain_pref_config_req_msg_v01, lte_domain_pref) - QMI_IDL_OFFSET8(wms_set_domain_pref_config_req_msg_v01, lte_domain_pref_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_domain_pref_config_req_msg_v01, lte_domain_pref),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_domain_pref_config_req_msg_v01, gw_domain_pref) - QMI_IDL_OFFSET8(wms_set_domain_pref_config_req_msg_v01, gw_domain_pref_valid)),
  0x11,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_domain_pref_config_req_msg_v01, gw_domain_pref)
};

static const uint8_t wms_set_domain_pref_config_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_domain_pref_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_domain_pref_config_resp_msg_v01, lte_domain_pref_outcome) - QMI_IDL_OFFSET8(wms_set_domain_pref_config_resp_msg_v01, lte_domain_pref_outcome_valid)),
  0x10,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_domain_pref_config_resp_msg_v01, lte_domain_pref_outcome),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_set_domain_pref_config_resp_msg_v01, gw_domain_pref_outcome) - QMI_IDL_OFFSET8(wms_set_domain_pref_config_resp_msg_v01, gw_domain_pref_outcome_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_set_domain_pref_config_resp_msg_v01, gw_domain_pref_outcome)
};

/* 
 * wms_get_retry_period_req_msg is empty
 * static const uint8_t wms_get_retry_period_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_retry_period_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_retry_period_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_retry_period_resp_msg_v01, retry_period) - QMI_IDL_OFFSET8(wms_get_retry_period_resp_msg_v01, retry_period_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_get_retry_period_resp_msg_v01, retry_period)
};

/* 
 * wms_get_retry_interval_req_msg is empty
 * static const uint8_t wms_get_retry_interval_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_retry_interval_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_retry_interval_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_retry_interval_resp_msg_v01, retry_interval) - QMI_IDL_OFFSET8(wms_get_retry_interval_resp_msg_v01, retry_interval_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_get_retry_interval_resp_msg_v01, retry_interval)
};

/* 
 * wms_get_dc_disconnect_timer_req_msg is empty
 * static const uint8_t wms_get_dc_disconnect_timer_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_dc_disconnect_timer_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_dc_disconnect_timer_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_dc_disconnect_timer_resp_msg_v01, dc_auto_disconn_timer) - QMI_IDL_OFFSET8(wms_get_dc_disconnect_timer_resp_msg_v01, dc_auto_disconn_timer_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_get_dc_disconnect_timer_resp_msg_v01, dc_auto_disconn_timer)
};

/* 
 * wms_get_memory_status_req_msg is empty
 * static const uint8_t wms_get_memory_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_memory_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_memory_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_memory_status_resp_msg_v01, memory_available) - QMI_IDL_OFFSET8(wms_get_memory_status_resp_msg_v01, memory_available_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_memory_status_resp_msg_v01, memory_available)
};

/* 
 * wms_get_primary_client_req_msg is empty
 * static const uint8_t wms_get_primary_client_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_primary_client_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_primary_client_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_primary_client_resp_msg_v01, primary_client) - QMI_IDL_OFFSET8(wms_get_primary_client_resp_msg_v01, primary_client_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_primary_client_resp_msg_v01, primary_client)
};

/* 
 * wms_get_subscription_binding_req_msg is empty
 * static const uint8_t wms_get_subscription_binding_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_subscription_binding_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_subscription_binding_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_subscription_binding_resp_msg_v01, subs_type) - QMI_IDL_OFFSET8(wms_get_subscription_binding_resp_msg_v01, subs_type_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_get_subscription_binding_resp_msg_v01, subs_type)
};

static const uint8_t wms_async_raw_send_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_raw_send_req_msg_v01, raw_message_data),
  QMI_IDL_TYPE88(0, 5),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, force_on_dc) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, force_on_dc_valid)),
  0x10,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_req_msg_v01, force_on_dc),
  QMI_IDL_TYPE88(0, 6),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, follow_on_dc) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, follow_on_dc_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_req_msg_v01, follow_on_dc),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, link_timer) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, link_timer_valid)),
  0x12,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_req_msg_v01, link_timer),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, sms_on_ims) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, sms_on_ims_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_req_msg_v01, sms_on_ims),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, retry_message) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, retry_message_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_req_msg_v01, retry_message),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, retry_message_id) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, retry_message_id_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_req_msg_v01, retry_message_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, user_data) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, user_data_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_req_msg_v01, user_data),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, link_enable_mode) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_req_msg_v01, link_enable_mode_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_req_msg_v01, link_enable_mode)
};

static const uint8_t wms_async_raw_send_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_raw_send_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_async_raw_send_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, send_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_id) - QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, cause_code) - QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, cause_code_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, cause_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, error_class) - QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, error_class_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, error_class),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, gw_cause_info) - QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, gw_cause_info_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, gw_cause_info),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_delivery_failure_type) - QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_delivery_failure_type_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_delivery_failure_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_delivery_failure_cause) - QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_delivery_failure_cause_valid)),
  0x15,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, message_delivery_failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, call_control_modified_info) - QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, call_control_modified_info_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_raw_send_ind_msg_v01, call_control_modified_info),
  QMI_IDL_TYPE88(0, 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_ind_msg_v01, user_data) - QMI_IDL_OFFSET16RELATIVE(wms_async_raw_send_ind_msg_v01, user_data_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_async_raw_send_ind_msg_v01, user_data)
};

static const uint8_t wms_async_send_ack_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, ack_information),
  QMI_IDL_TYPE88(0, 17),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, wms_3gpp2_failure_information) - QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, wms_3gpp2_failure_information_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, wms_3gpp2_failure_information),
  QMI_IDL_TYPE88(0, 18),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, wms_3gpp_failure_information) - QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, wms_3gpp_failure_information_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, wms_3gpp_failure_information),
  QMI_IDL_TYPE88(0, 19),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, sms_on_ims) - QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, sms_on_ims_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, sms_on_ims),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, user_data) - QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, user_data_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_async_send_ack_req_msg_v01, user_data)
};

static const uint8_t wms_async_send_ack_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_send_ack_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_async_send_ack_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_send_ack_ind_msg_v01, ack_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_ack_ind_msg_v01, failure_cause) - QMI_IDL_OFFSET8(wms_async_send_ack_ind_msg_v01, failure_cause_valid)),
  0x10,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_send_ack_ind_msg_v01, failure_cause),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_ack_ind_msg_v01, user_data) - QMI_IDL_OFFSET8(wms_async_send_ack_ind_msg_v01, user_data_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_async_send_ack_ind_msg_v01, user_data)
};

static const uint8_t wms_async_send_from_mem_store_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_req_msg_v01, message_memory_storage_info),
  QMI_IDL_TYPE88(0, 26),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_req_msg_v01, sms_on_ims) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_req_msg_v01, sms_on_ims_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_req_msg_v01, sms_on_ims),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_req_msg_v01, user_data) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_req_msg_v01, user_data_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_req_msg_v01, user_data)
};

static const uint8_t wms_async_send_from_mem_store_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t wms_async_send_from_mem_store_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, send_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_id) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_id_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, cause_code) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, cause_code_valid)),
  0x11,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, cause_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, error_class) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, error_class_valid)),
  0x12,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, error_class),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, gw_cause_info) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, gw_cause_info_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, gw_cause_info),
  QMI_IDL_TYPE88(0, 7),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_delivery_failure_type) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_delivery_failure_type_valid)),
  0x14,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_delivery_failure_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_delivery_failure_cause) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_delivery_failure_cause_valid)),
  0x15,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, message_delivery_failure_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, call_control_modified_info) - QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, call_control_modified_info_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_async_send_from_mem_store_ind_msg_v01, call_control_modified_info),
  QMI_IDL_TYPE88(0, 8),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_async_send_from_mem_store_ind_msg_v01, user_data) - QMI_IDL_OFFSET16RELATIVE(wms_async_send_from_mem_store_ind_msg_v01, user_data_valid)),
  0x17,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(wms_async_send_from_mem_store_ind_msg_v01, user_data)
};

/* 
 * wms_get_service_ready_status_req_msg is empty
 * static const uint8_t wms_get_service_ready_status_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t wms_get_service_ready_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_get_service_ready_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_service_ready_status_resp_msg_v01, registered_ind) - QMI_IDL_OFFSET8(wms_get_service_ready_status_resp_msg_v01, registered_ind_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(wms_get_service_ready_status_resp_msg_v01, registered_ind),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_get_service_ready_status_resp_msg_v01, ready_status) - QMI_IDL_OFFSET8(wms_get_service_ready_status_resp_msg_v01, ready_status_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_get_service_ready_status_resp_msg_v01, ready_status)
};

static const uint8_t wms_service_ready_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(wms_service_ready_ind_msg_v01, ready_status)
};

static const uint8_t wms_broadcast_config_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_1_BYTE_ENUM,
  QMI_IDL_OFFSET8(wms_broadcast_config_ind_msg_v01, message_mode),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(wms_broadcast_config_ind_msg_v01, wms_3gpp_broadcast_info) - QMI_IDL_OFFSET8(wms_broadcast_config_ind_msg_v01, wms_3gpp_broadcast_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_broadcast_config_ind_msg_v01, wms_3gpp_broadcast_info),
  QMI_IDL_TYPE88(0, 23),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(wms_broadcast_config_ind_msg_v01, wms_3gpp2_broadcast_info) - QMI_IDL_OFFSET16RELATIVE(wms_broadcast_config_ind_msg_v01, wms_3gpp2_broadcast_info_valid)),
  0x11,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(wms_broadcast_config_ind_msg_v01, wms_3gpp2_broadcast_info),
  QMI_IDL_TYPE88(0, 24)
};

static const uint8_t wms_set_message_waiting_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_message_waiting_req_msg_v01, message_waiting_info),
  WMS_MESSAGE_TUPLE_MAX_V01,
  QMI_IDL_OFFSET8(wms_set_message_waiting_req_msg_v01, message_waiting_info) - QMI_IDL_OFFSET8(wms_set_message_waiting_req_msg_v01, message_waiting_info_len),
  QMI_IDL_TYPE88(0, 27)
};

static const uint8_t wms_set_message_waiting_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(wms_set_message_waiting_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  wms_type_table_v01[] = {
  {sizeof(wms_mt_message_type_v01), wms_mt_message_type_data_v01},
  {sizeof(wms_transfer_route_mt_message_type_v01), wms_transfer_route_mt_message_type_data_v01},
  {sizeof(wms_etws_message_type_v01), wms_etws_message_type_data_v01},
  {sizeof(wms_etws_plmn_info_type_v01), wms_etws_plmn_info_type_data_v01},
  {sizeof(wms_mt_message_smsc_address_type_v01), wms_mt_message_smsc_address_type_data_v01},
  {sizeof(wms_send_raw_message_data_type_v01), wms_send_raw_message_data_type_data_v01},
  {sizeof(wms_force_on_dc_type_v01), wms_force_on_dc_type_data_v01},
  {sizeof(wms_gw_cause_info_type_v01), wms_gw_cause_info_type_data_v01},
  {sizeof(wms_call_control_modified_info_type_v01), wms_call_control_modified_info_type_data_v01},
  {sizeof(wms_raw_message_write_data_type_v01), wms_raw_message_write_data_type_data_v01},
  {sizeof(wms_message_memory_storage_identification_type_v01), wms_message_memory_storage_identification_type_data_v01},
  {sizeof(wms_read_raw_message_data_type_v01), wms_read_raw_message_data_type_data_v01},
  {sizeof(wms_message_tag_type_v01), wms_message_tag_type_data_v01},
  {sizeof(wms_message_tuple_type_v01), wms_message_tuple_type_data_v01},
  {sizeof(wms_set_route_list_tuple_type_v01), wms_set_route_list_tuple_type_data_v01},
  {sizeof(wms_get_route_list_tuple_type_v01), wms_get_route_list_tuple_type_data_v01},
  {sizeof(wms_smsc_address_type_v01), wms_smsc_address_type_data_v01},
  {sizeof(wms_ack_information_type_v01), wms_ack_information_type_data_v01},
  {sizeof(wms_3gpp2_failure_information_type_v01), wms_3gpp2_failure_information_type_data_v01},
  {sizeof(wms_3gpp_failure_information_type_v01), wms_3gpp_failure_information_type_data_v01},
  {sizeof(wms_broadcast_activation_info_type_v01), wms_broadcast_activation_info_type_data_v01},
  {sizeof(wms_3gpp_broadcast_config_info_type_v01), wms_3gpp_broadcast_config_info_type_data_v01},
  {sizeof(wms_3gpp2_broadcast_config_info_type_v01), wms_3gpp2_broadcast_config_info_type_data_v01},
  {sizeof(wms_3gpp_broadcast_info_config2_type_v01), wms_3gpp_broadcast_info_config2_type_data_v01},
  {sizeof(wms_3gpp2_broadcast_info_config2_type_v01), wms_3gpp2_broadcast_info_config2_type_data_v01},
  {sizeof(wms_memory_full_info_type_v01), wms_memory_full_info_type_data_v01},
  {sizeof(wms_message_memory_storage_info_type_v01), wms_message_memory_storage_info_type_data_v01},
  {sizeof(wms_message_waiting_information_type_v01), wms_message_waiting_information_type_data_v01},
  {sizeof(wms_transport_layer_info_type_v01), wms_transport_layer_info_type_data_v01},
  {sizeof(wms_call_control_info_type_v01), wms_call_control_info_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry wms_message_table_v01[] = {
  {sizeof(wms_reset_req_msg_v01), 0},
  {sizeof(wms_reset_resp_msg_v01), wms_reset_resp_msg_data_v01},
  {sizeof(wms_set_event_report_req_msg_v01), wms_set_event_report_req_msg_data_v01},
  {sizeof(wms_set_event_report_resp_msg_v01), wms_set_event_report_resp_msg_data_v01},
  {sizeof(wms_event_report_ind_msg_v01), wms_event_report_ind_msg_data_v01},
  {sizeof(wms_raw_send_req_msg_v01), wms_raw_send_req_msg_data_v01},
  {sizeof(wms_raw_send_resp_msg_v01), wms_raw_send_resp_msg_data_v01},
  {sizeof(wms_raw_write_req_msg_v01), wms_raw_write_req_msg_data_v01},
  {sizeof(wms_raw_write_resp_msg_v01), wms_raw_write_resp_msg_data_v01},
  {sizeof(wms_raw_read_req_msg_v01), wms_raw_read_req_msg_data_v01},
  {sizeof(wms_raw_read_resp_msg_v01), wms_raw_read_resp_msg_data_v01},
  {sizeof(wms_modify_tag_req_msg_v01), wms_modify_tag_req_msg_data_v01},
  {sizeof(wms_modify_tag_resp_msg_v01), wms_modify_tag_resp_msg_data_v01},
  {sizeof(wms_delete_req_msg_v01), wms_delete_req_msg_data_v01},
  {sizeof(wms_delete_resp_msg_v01), wms_delete_resp_msg_data_v01},
  {sizeof(wms_get_message_protocol_req_msg_v01), 0},
  {sizeof(wms_get_message_protocol_resp_msg_v01), wms_get_message_protocol_resp_msg_data_v01},
  {sizeof(wms_list_messages_req_msg_v01), wms_list_messages_req_msg_data_v01},
  {sizeof(wms_list_messages_resp_msg_v01), wms_list_messages_resp_msg_data_v01},
  {sizeof(wms_set_routes_req_msg_v01), wms_set_routes_req_msg_data_v01},
  {sizeof(wms_set_routes_resp_msg_v01), wms_set_routes_resp_msg_data_v01},
  {sizeof(wms_get_routes_req_msg_v01), 0},
  {sizeof(wms_get_routes_resp_msg_v01), wms_get_routes_resp_msg_data_v01},
  {sizeof(wms_get_smsc_address_req_msg_v01), 0},
  {sizeof(wms_get_smsc_address_resp_msg_v01), wms_get_smsc_address_resp_msg_data_v01},
  {sizeof(wms_set_smsc_address_req_msg_v01), wms_set_smsc_address_req_msg_data_v01},
  {sizeof(wms_set_smsc_address_resp_msg_v01), wms_set_smsc_address_resp_msg_data_v01},
  {sizeof(wms_get_store_max_size_req_msg_v01), wms_get_store_max_size_req_msg_data_v01},
  {sizeof(wms_get_store_max_size_resp_msg_v01), wms_get_store_max_size_resp_msg_data_v01},
  {sizeof(wms_send_ack_req_msg_v01), wms_send_ack_req_msg_data_v01},
  {sizeof(wms_send_ack_resp_msg_v01), wms_send_ack_resp_msg_data_v01},
  {sizeof(wms_set_retry_period_req_msg_v01), wms_set_retry_period_req_msg_data_v01},
  {sizeof(wms_set_retry_period_resp_msg_v01), wms_set_retry_period_resp_msg_data_v01},
  {sizeof(wms_set_retry_interval_req_msg_v01), wms_set_retry_interval_req_msg_data_v01},
  {sizeof(wms_set_retry_interval_resp_msg_v01), wms_set_retry_interval_resp_msg_data_v01},
  {sizeof(wms_set_dc_disconnect_timer_req_msg_v01), wms_set_dc_disconnect_timer_req_msg_data_v01},
  {sizeof(wms_set_dc_disconnect_timer_resp_msg_v01), wms_set_dc_disconnect_timer_resp_msg_data_v01},
  {sizeof(wms_set_memory_status_req_msg_v01), wms_set_memory_status_req_msg_data_v01},
  {sizeof(wms_set_memory_status_resp_msg_v01), wms_set_memory_status_resp_msg_data_v01},
  {sizeof(wms_set_broadcast_activation_req_msg_v01), wms_set_broadcast_activation_req_msg_data_v01},
  {sizeof(wms_set_broadcast_activation_resp_msg_v01), wms_set_broadcast_activation_resp_msg_data_v01},
  {sizeof(wms_set_broadcast_config_req_msg_v01), wms_set_broadcast_config_req_msg_data_v01},
  {sizeof(wms_set_broadcast_config_resp_msg_v01), wms_set_broadcast_config_resp_msg_data_v01},
  {sizeof(wms_get_broadcast_config_req_msg_v01), wms_get_broadcast_config_req_msg_data_v01},
  {sizeof(wms_get_broadcast_config_resp_msg_v01), wms_get_broadcast_config_resp_msg_data_v01},
  {sizeof(wms_memory_full_ind_msg_v01), wms_memory_full_ind_msg_data_v01},
  {sizeof(wms_get_domain_pref_req_msg_v01), 0},
  {sizeof(wms_get_domain_pref_resp_msg_v01), wms_get_domain_pref_resp_msg_data_v01},
  {sizeof(wms_set_domain_pref_req_msg_v01), wms_set_domain_pref_req_msg_data_v01},
  {sizeof(wms_set_domain_pref_resp_msg_v01), wms_set_domain_pref_resp_msg_data_v01},
  {sizeof(wms_send_from_mem_store_req_msg_v01), wms_send_from_mem_store_req_msg_data_v01},
  {sizeof(wms_send_from_mem_store_resp_msg_v01), wms_send_from_mem_store_resp_msg_data_v01},
  {sizeof(wms_get_message_waiting_req_msg_v01), 0},
  {sizeof(wms_get_message_waiting_resp_msg_v01), wms_get_message_waiting_resp_msg_data_v01},
  {sizeof(wms_message_waiting_ind_msg_v01), wms_message_waiting_ind_msg_data_v01},
  {sizeof(wms_set_primary_client_req_msg_v01), wms_set_primary_client_req_msg_data_v01},
  {sizeof(wms_set_primary_client_resp_msg_v01), wms_set_primary_client_resp_msg_data_v01},
  {sizeof(wms_smsc_address_ind_msg_v01), wms_smsc_address_ind_msg_data_v01},
  {sizeof(wms_indication_register_req_msg_v01), wms_indication_register_req_msg_data_v01},
  {sizeof(wms_indication_register_resp_msg_v01), wms_indication_register_resp_msg_data_v01},
  {sizeof(wms_get_transport_layer_req_msg_v01), 0},
  {sizeof(wms_get_transport_layer_resp_msg_v01), wms_get_transport_layer_resp_msg_data_v01},
  {sizeof(wms_transport_layer_info_ind_msg_v01), wms_transport_layer_info_ind_msg_data_v01},
  {sizeof(wms_get_transport_nw_reg_req_msg_v01), 0},
  {sizeof(wms_get_transport_nw_reg_resp_msg_v01), wms_get_transport_nw_reg_resp_msg_data_v01},
  {sizeof(wms_transport_nw_reg_info_ind_msg_v01), wms_transport_nw_reg_info_ind_msg_data_v01},
  {sizeof(wms_bind_subscription_req_msg_v01), wms_bind_subscription_req_msg_data_v01},
  {sizeof(wms_bind_subscription_resp_msg_v01), wms_bind_subscription_resp_msg_data_v01},
  {sizeof(wms_get_indication_register_req_msg_v01), 0},
  {sizeof(wms_get_indication_register_resp_msg_v01), wms_get_indication_register_resp_msg_data_v01},
  {sizeof(wms_get_sms_parameters_req_msg_v01), wms_get_sms_parameters_req_msg_data_v01},
  {sizeof(wms_get_sms_parameters_resp_msg_v01), wms_get_sms_parameters_resp_msg_data_v01},
  {sizeof(wms_set_sms_parameters_req_msg_v01), wms_set_sms_parameters_req_msg_data_v01},
  {sizeof(wms_set_sms_parameters_resp_msg_v01), wms_set_sms_parameters_resp_msg_data_v01},
  {sizeof(wms_call_status_ind_msg_v01), wms_call_status_ind_msg_data_v01},
  {sizeof(wms_get_domain_pref_config_req_msg_v01), 0},
  {sizeof(wms_get_domain_pref_config_resp_msg_v01), wms_get_domain_pref_config_resp_msg_data_v01},
  {sizeof(wms_set_domain_pref_config_req_msg_v01), wms_set_domain_pref_config_req_msg_data_v01},
  {sizeof(wms_set_domain_pref_config_resp_msg_v01), wms_set_domain_pref_config_resp_msg_data_v01},
  {sizeof(wms_get_retry_period_req_msg_v01), 0},
  {sizeof(wms_get_retry_period_resp_msg_v01), wms_get_retry_period_resp_msg_data_v01},
  {sizeof(wms_get_retry_interval_req_msg_v01), 0},
  {sizeof(wms_get_retry_interval_resp_msg_v01), wms_get_retry_interval_resp_msg_data_v01},
  {sizeof(wms_get_dc_disconnect_timer_req_msg_v01), 0},
  {sizeof(wms_get_dc_disconnect_timer_resp_msg_v01), wms_get_dc_disconnect_timer_resp_msg_data_v01},
  {sizeof(wms_get_memory_status_req_msg_v01), 0},
  {sizeof(wms_get_memory_status_resp_msg_v01), wms_get_memory_status_resp_msg_data_v01},
  {sizeof(wms_get_primary_client_req_msg_v01), 0},
  {sizeof(wms_get_primary_client_resp_msg_v01), wms_get_primary_client_resp_msg_data_v01},
  {sizeof(wms_get_subscription_binding_req_msg_v01), 0},
  {sizeof(wms_get_subscription_binding_resp_msg_v01), wms_get_subscription_binding_resp_msg_data_v01},
  {sizeof(wms_async_raw_send_req_msg_v01), wms_async_raw_send_req_msg_data_v01},
  {sizeof(wms_async_raw_send_resp_msg_v01), wms_async_raw_send_resp_msg_data_v01},
  {sizeof(wms_async_raw_send_ind_msg_v01), wms_async_raw_send_ind_msg_data_v01},
  {sizeof(wms_async_send_ack_req_msg_v01), wms_async_send_ack_req_msg_data_v01},
  {sizeof(wms_async_send_ack_resp_msg_v01), wms_async_send_ack_resp_msg_data_v01},
  {sizeof(wms_async_send_ack_ind_msg_v01), wms_async_send_ack_ind_msg_data_v01},
  {sizeof(wms_async_send_from_mem_store_req_msg_v01), wms_async_send_from_mem_store_req_msg_data_v01},
  {sizeof(wms_async_send_from_mem_store_resp_msg_v01), wms_async_send_from_mem_store_resp_msg_data_v01},
  {sizeof(wms_async_send_from_mem_store_ind_msg_v01), wms_async_send_from_mem_store_ind_msg_data_v01},
  {sizeof(wms_get_service_ready_status_req_msg_v01), 0},
  {sizeof(wms_get_service_ready_status_resp_msg_v01), wms_get_service_ready_status_resp_msg_data_v01},
  {sizeof(wms_service_ready_ind_msg_v01), wms_service_ready_ind_msg_data_v01},
  {sizeof(wms_broadcast_config_ind_msg_v01), wms_broadcast_config_ind_msg_data_v01},
  {sizeof(wms_set_message_waiting_req_msg_v01), wms_set_message_waiting_req_msg_data_v01},
  {sizeof(wms_set_message_waiting_resp_msg_v01), wms_set_message_waiting_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object wms_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *wms_qmi_idl_type_table_object_referenced_tables_v01[] =
{&wms_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object wms_qmi_idl_type_table_object_v01 = {
  sizeof(wms_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(wms_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  wms_type_table_v01,
  wms_message_table_v01,
  wms_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry wms_service_command_messages_v01[] = {
  {QMI_WMS_RESET_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_WMS_SET_EVENT_REPORT_REQ_V01, QMI_IDL_TYPE16(0, 2), 12},
  {QMI_WMS_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_WMS_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_WMS_RAW_SEND_REQ_V01, QMI_IDL_TYPE16(0, 5), 293},
  {QMI_WMS_RAW_WRITE_REQ_V01, QMI_IDL_TYPE16(0, 7), 266},
  {QMI_WMS_RAW_READ_REQ_V01, QMI_IDL_TYPE16(0, 9), 16},
  {QMI_WMS_MODIFY_TAG_REQ_V01, QMI_IDL_TYPE16(0, 11), 13},
  {QMI_WMS_DELETE_REQ_V01, QMI_IDL_TYPE16(0, 13), 19},
  {QMI_WMS_GET_MESSAGE_PROTOCOL_REQ_V01, QMI_IDL_TYPE16(0, 15), 0},
  {QMI_WMS_LIST_MESSAGES_REQ_V01, QMI_IDL_TYPE16(0, 17), 12},
  {QMI_WMS_SET_ROUTES_REQ_V01, QMI_IDL_TYPE16(0, 19), 49},
  {QMI_WMS_GET_ROUTES_REQ_V01, QMI_IDL_TYPE16(0, 21), 0},
  {QMI_WMS_GET_SMSC_ADDRESS_REQ_V01, QMI_IDL_TYPE16(0, 23), 0},
  {QMI_WMS_SET_SMSC_ADDRESS_REQ_V01, QMI_IDL_TYPE16(0, 25), 34},
  {QMI_WMS_GET_STORE_MAX_SIZE_REQ_V01, QMI_IDL_TYPE16(0, 27), 8},
  {QMI_WMS_SEND_ACK_REQ_V01, QMI_IDL_TYPE16(0, 29), 23},
  {QMI_WMS_SET_RETRY_PERIOD_REQ_V01, QMI_IDL_TYPE16(0, 31), 7},
  {QMI_WMS_SET_RETRY_INTERVAL_REQ_V01, QMI_IDL_TYPE16(0, 33), 7},
  {QMI_WMS_SET_DC_DISCONNECT_TIMER_REQ_V01, QMI_IDL_TYPE16(0, 35), 7},
  {QMI_WMS_SET_MEMORY_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 37), 4},
  {QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01, QMI_IDL_TYPE16(0, 39), 9},
  {QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 41), 514},
  {QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 43), 4},
  {QMI_WMS_GET_DOMAIN_PREF_REQ_V01, QMI_IDL_TYPE16(0, 46), 0},
  {QMI_WMS_SET_DOMAIN_PREF_REQ_V01, QMI_IDL_TYPE16(0, 48), 4},
  {QMI_WMS_SEND_FROM_MEM_STORE_REQ_V01, QMI_IDL_TYPE16(0, 50), 13},
  {QMI_WMS_GET_MESSAGE_WAITING_REQ_V01, QMI_IDL_TYPE16(0, 52), 0},
  {QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01, QMI_IDL_TYPE16(0, 55), 4},
  {QMI_WMS_INDICATION_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 58), 20},
  {QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01, QMI_IDL_TYPE16(0, 60), 0},
  {QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01, QMI_IDL_TYPE16(0, 63), 0},
  {QMI_WMS_BIND_SUBSCRIPTION_REQ_V01, QMI_IDL_TYPE16(0, 66), 4},
  {QMI_WMS_GET_INDICATION_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 68), 0},
  {QMI_WMS_GET_SMS_PARAMETERS_REQ_V01, QMI_IDL_TYPE16(0, 70), 4},
  {QMI_WMS_SET_SMS_PARAMETERS_REQ_V01, QMI_IDL_TYPE16(0, 72), 32},
  {QMI_WMS_GET_DOMAIN_PREF_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 75), 0},
  {QMI_WMS_SET_DOMAIN_PREF_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 77), 8},
  {QMI_WMS_GET_RETRY_PERIOD_REQ_V01, QMI_IDL_TYPE16(0, 79), 0},
  {QMI_WMS_GET_RETRY_INTERVAL_REQ_V01, QMI_IDL_TYPE16(0, 81), 0},
  {QMI_WMS_GET_DC_DISCONNECT_TIMER_REQ_V01, QMI_IDL_TYPE16(0, 83), 0},
  {QMI_WMS_GET_MEMORY_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 85), 0},
  {QMI_WMS_GET_PRIMARY_CLIENT_REQ_V01, QMI_IDL_TYPE16(0, 87), 0},
  {QMI_WMS_GET_SUBSCRIPTION_BINDING_REQ_V01, QMI_IDL_TYPE16(0, 89), 0},
  {QMI_WMS_ASYNC_RAW_SEND_REQ_V01, QMI_IDL_TYPE16(0, 91), 300},
  {QMI_WMS_ASYNC_SEND_ACK_REQ_V01, QMI_IDL_TYPE16(0, 94), 30},
  {QMI_WMS_ASYNC_SEND_FROM_MEM_STORE_REQ_V01, QMI_IDL_TYPE16(0, 97), 20},
  {QMI_WMS_GET_SERVICE_READY_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 100), 0},
  {QMI_WMS_SET_MESSAGE_WAITING_REQ_V01, QMI_IDL_TYPE16(0, 104), 769}
};

static const qmi_idl_service_message_table_entry wms_service_response_messages_v01[] = {
  {QMI_WMS_RESET_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_WMS_SET_EVENT_REPORT_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_WMS_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_WMS_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_WMS_RAW_SEND_RESP_V01, QMI_IDL_TYPE16(0, 6), 294},
  {QMI_WMS_RAW_WRITE_RESP_V01, QMI_IDL_TYPE16(0, 8), 14},
  {QMI_WMS_RAW_RESP_MSG_V01, QMI_IDL_TYPE16(0, 10), 269},
  {QMI_WMS_MODIFY_TAG_RESP_V01, QMI_IDL_TYPE16(0, 12), 7},
  {QMI_WMS_DELETE_RESP_V01, QMI_IDL_TYPE16(0, 14), 7},
  {QMI_WMS_GET_MESSAGE_PROTOCOL_RESP_V01, QMI_IDL_TYPE16(0, 16), 11},
  {QMI_WMS_LIST_MESSAGES_RESP_V01, QMI_IDL_TYPE16(0, 18), 1289},
  {QMI_WMS_SET_ROUTES_RESP_V01, QMI_IDL_TYPE16(0, 20), 7},
  {QMI_WMS_GET_ROUTES_RESP_V01, QMI_IDL_TYPE16(0, 22), 56},
  {QMI_WMS_GET_SMSC_ADDRESS_RESP_V01, QMI_IDL_TYPE16(0, 24), 35},
  {QMI_WMS_SET_SMSC_ADDRESS_RESP_V01, QMI_IDL_TYPE16(0, 26), 7},
  {QMI_WMS_GET_STORE_MAX_SIZE_RESP_V01, QMI_IDL_TYPE16(0, 28), 21},
  {QMI_WMS_SEND_ACK_RESP_V01, QMI_IDL_TYPE16(0, 30), 11},
  {QMI_WMS_SET_RETRY_PERIOD_RESP_V01, QMI_IDL_TYPE16(0, 32), 7},
  {QMI_WMS_SET_RETRY_INTERVAL_RESP_V01, QMI_IDL_TYPE16(0, 34), 7},
  {QMI_WMS_SET_DC_DISCONNECT_TIMER_RESP_V01, QMI_IDL_TYPE16(0, 36), 7},
  {QMI_WMS_SET_MEMORY_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 38), 7},
  {QMI_WMS_SET_BROADCAST_ACTIVATION_RESP_V01, QMI_IDL_TYPE16(0, 40), 7},
  {QMI_WMS_SET_BROADCAST_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 42), 7},
  {QMI_WMS_GET_BROADCAST_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 44), 519},
  {QMI_WMS_GET_DOMAIN_PREF_RESP_V01, QMI_IDL_TYPE16(0, 47), 11},
  {QMI_WMS_SET_DOMAIN_PREF_RESP_V01, QMI_IDL_TYPE16(0, 49), 7},
  {QMI_WMS_SEND_FROM_MEM_STORE_RESP_V01, QMI_IDL_TYPE16(0, 51), 31},
  {QMI_WMS_GET_MESSAGE_WAITING_RESP_V01, QMI_IDL_TYPE16(0, 53), 776},
  {QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01, QMI_IDL_TYPE16(0, 56), 7},
  {QMI_WMS_INDICATION_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 59), 7},
  {QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01, QMI_IDL_TYPE16(0, 61), 16},
  {QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01, QMI_IDL_TYPE16(0, 64), 11},
  {QMI_WMS_BIND_SUBSCRIPTION_RESP_V01, QMI_IDL_TYPE16(0, 67), 7},
  {QMI_WMS_GET_INDICATION_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 69), 27},
  {QMI_WMS_GET_SMS_PARAMETERS_RESP_V01, QMI_IDL_TYPE16(0, 71), 35},
  {QMI_WMS_SET_SMS_PARAMETERS_RESP_V01, QMI_IDL_TYPE16(0, 73), 7},
  {QMI_WMS_GET_DOMAIN_PREF_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 76), 15},
  {QMI_WMS_SET_DOMAIN_PREF_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 78), 17},
  {QMI_WMS_GET_RETRY_PERIOD_RESP_V01, QMI_IDL_TYPE16(0, 80), 14},
  {QMI_WMS_GET_RETRY_INTERVAL_RESP_V01, QMI_IDL_TYPE16(0, 82), 14},
  {QMI_WMS_GET_DC_DISCONNECT_TIMER_RESP_V01, QMI_IDL_TYPE16(0, 84), 14},
  {QMI_WMS_GET_MEMORY_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 86), 11},
  {QMI_WMS_GET_PRIMARY_CLIENT_RESP_V01, QMI_IDL_TYPE16(0, 88), 11},
  {QMI_WMS_GET_SUBSCRIPTION_BINDING_RESP_V01, QMI_IDL_TYPE16(0, 90), 11},
  {QMI_WMS_ASYNC_RAW_SEND_RESP_V01, QMI_IDL_TYPE16(0, 92), 7},
  {QMI_WMS_ASYNC_SEND_ACK_RESP_V01, QMI_IDL_TYPE16(0, 95), 7},
  {QMI_WMS_ASYNC_SEND_FROM_MEM_STORE_RESP_V01, QMI_IDL_TYPE16(0, 98), 7},
  {QMI_WMS_GET_SERVICE_READY_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 101), 18},
  {QMI_WMS_SET_MESSAGE_WAITING_RESP_V01, QMI_IDL_TYPE16(0, 105), 7}
};

static const qmi_idl_service_message_table_entry wms_service_indication_messages_v01[] = {
  {QMI_WMS_EVENT_REPORT_IND_V01, QMI_IDL_TYPE16(0, 4), 1825},
  {QMI_WMS_MEMORY_FULL_IND_V01, QMI_IDL_TYPE16(0, 45), 5},
  {QMI_WMS_MESSAGE_WAITING_IND_V01, QMI_IDL_TYPE16(0, 54), 769},
  {QMI_WMS_SMSC_ADDRESS_IND_V01, QMI_IDL_TYPE16(0, 57), 28},
  {QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01, QMI_IDL_TYPE16(0, 62), 9},
  {QMI_WMS_TRANSPORT_NW_REG_INFO_IND_V01, QMI_IDL_TYPE16(0, 65), 4},
  {QMI_WMS_CALL_STATUS_IND_V01, QMI_IDL_TYPE16(0, 74), 4},
  {QMI_WMS_ASYNC_RAW_SEND_IND_V01, QMI_IDL_TYPE16(0, 93), 299},
  {QMI_WMS_ASYNC_SEND_ACK_IND_V01, QMI_IDL_TYPE16(0, 96), 16},
  {QMI_WMS_ASYNC_SEND_FROM_MEM_STORE_IND_V01, QMI_IDL_TYPE16(0, 99), 299},
  {QMI_WMS_SERVICE_READY_IND_V01, QMI_IDL_TYPE16(0, 102), 7},
  {QMI_WMS_BROADCAST_CONFIG_IND_V01, QMI_IDL_TYPE16(0, 103), 516}
};

/*Service Object*/
struct qmi_idl_service_object wms_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x05,
  8204,
  { sizeof(wms_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(wms_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(wms_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { wms_service_command_messages_v01, wms_service_response_messages_v01, wms_service_indication_messages_v01},
  &wms_qmi_idl_type_table_object_v01,
  0x14,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type wms_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( WMS_V01_IDL_MAJOR_VERS != idl_maj_version || WMS_V01_IDL_MINOR_VERS != idl_min_version 
       || WMS_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&wms_qmi_idl_service_object_v01;
}

