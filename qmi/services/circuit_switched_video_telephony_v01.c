/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        C I R C U I T _ S W I T C H E D _ V I D E O _ T E L E P H O N Y _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the csvt service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It was generated on: Sat Aug 24 2013 (Spin 0)
   From IDL File: circuit_switched_video_telephony_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "circuit_switched_video_telephony_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t csvt_port_id_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_port_id_type_v01, port_family),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_port_id_type_v01, port_number),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t csvt_uus_id_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_uus_id_v01, uus_id_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(csvt_uus_id_v01, uus_id),
  CSVT_MAX_UUS_ID_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t rlp_params_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rlp_params_v01, rlp_version),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rlp_params_v01, rlp_tx_window_size),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rlp_params_v01, rlp_rx_window_size),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rlp_params_v01, rlp_ack_timer),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rlp_params_v01, rlp_retrans_attempts),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(rlp_params_v01, rlp_reseq_timer),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t v42_params_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(v42_params_v01, v42_direction),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(v42_params_v01, v42_negotiation),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(v42_params_v01, v42_max_dict),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(v42_params_v01, v42_max_string),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/* 
 * csvt_reset_req_msg is empty
 * static const uint8_t csvt_reset_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t csvt_reset_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_reset_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t csvt_set_event_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_set_event_report_req_msg_v01, report_call_events) - QMI_IDL_OFFSET8(csvt_set_event_report_req_msg_v01, report_call_events_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_set_event_report_req_msg_v01, report_call_events),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_set_event_report_req_msg_v01, call_types) - QMI_IDL_OFFSET8(csvt_set_event_report_req_msg_v01, call_types_valid)),
  0x11,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(csvt_set_event_report_req_msg_v01, call_types)
};

static const uint8_t csvt_set_event_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_set_event_report_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t csvt_event_report_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, instance_id),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, event_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, call_type) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, call_type_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, call_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, synchronous_call) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, synchronous_call_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, synchronous_call),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, transparent_call) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, transparent_call_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, transparent_call),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, network_type) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, network_type_valid)),
  0x13,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, network_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, network_speed) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, network_speed_valid)),
  0x14,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, network_speed),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, max_frame_size) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, max_frame_size_valid)),
  0x15,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, max_frame_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, incoming_number) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, incoming_number_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, incoming_number),
  CSVT_MAX_INCOM_NUM_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, uus_id) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, uus_id_valid)),
  0x17,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, uus_id),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, modify_allowed) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, modify_allowed_valid)),
  0x18,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, modify_allowed),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, call_end_cause) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, call_end_cause_valid)),
  0x19,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, call_end_cause),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, port_data) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, port_data_valid)),
  0x1A,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, port_data),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, incoming_number_length) - QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, incoming_number_length_valid)),
  0x1B,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_event_report_ind_msg_v01, incoming_number_length)
};

static const uint8_t csvt_originate_call_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, instance_id),

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, call_mode),

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, dial_string),
  CSVT_MAX_DIAL_STRING_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, network_datarate) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, network_datarate_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, network_datarate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, air_interface_datarate) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, air_interface_datarate_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, air_interface_datarate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, synchronous_call) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, synchronous_call_valid)),
  0x12,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, synchronous_call),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, transparent_call) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, transparent_call_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, transparent_call),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cli_enabled) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cli_enabled_valid)),
  0x14,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cli_enabled),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cug_enabled) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cug_enabled_valid)),
  0x15,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cug_enabled),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cug_index) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cug_index_valid)),
  0x16,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, cug_index),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, supress_preferred_cug) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, supress_preferred_cug_valid)),
  0x17,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, supress_preferred_cug),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, supress_outgoing_access) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, supress_outgoing_access_valid)),
  0x18,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, supress_outgoing_access),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, uus_id) - QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, uus_id_valid)),
  0x19,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_originate_call_req_msg_v01, uus_id),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t csvt_originate_call_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_originate_call_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t csvt_originate_call_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_ind_msg_v01, instance_id),

  0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_ind_msg_v01, request_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_originate_call_ind_msg_v01, call_end_cause) - QMI_IDL_OFFSET8(csvt_originate_call_ind_msg_v01, call_end_cause_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_originate_call_ind_msg_v01, call_end_cause)
};

static const uint8_t csvt_confirm_call_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_confirm_call_req_msg_v01, instance_id),

  0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_confirm_call_req_msg_v01, confirm_call),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_confirm_call_req_msg_v01, reject_value) - QMI_IDL_OFFSET8(csvt_confirm_call_req_msg_v01, reject_value_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_confirm_call_req_msg_v01, reject_value)
};

static const uint8_t csvt_confirm_call_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_confirm_call_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t csvt_confirm_call_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_confirm_call_ind_msg_v01, instance_id),

  0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_confirm_call_ind_msg_v01, request_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_confirm_call_ind_msg_v01, call_end_cause) - QMI_IDL_OFFSET8(csvt_confirm_call_ind_msg_v01, call_end_cause_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_confirm_call_ind_msg_v01, call_end_cause)
};

static const uint8_t csvt_answer_call_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_answer_call_req_msg_v01, instance_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_answer_call_req_msg_v01, answer_call),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_answer_call_req_msg_v01, reject_value) - QMI_IDL_OFFSET8(csvt_answer_call_req_msg_v01, reject_value_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_answer_call_req_msg_v01, reject_value)
};

static const uint8_t csvt_answer_call_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_answer_call_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t csvt_answer_call_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_answer_call_ind_msg_v01, instance_id),

  0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_answer_call_ind_msg_v01, request_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_answer_call_ind_msg_v01, call_end_cause) - QMI_IDL_OFFSET8(csvt_answer_call_ind_msg_v01, call_end_cause_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_answer_call_ind_msg_v01, call_end_cause)
};

static const uint8_t csvt_end_call_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_end_call_req_msg_v01, instance_id)
};

static const uint8_t csvt_end_call_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_end_call_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t csvt_end_call_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_end_call_ind_msg_v01, instance_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_end_call_ind_msg_v01, request_status)
};

static const uint8_t csvt_modify_call_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_modify_call_req_msg_v01, instance_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_modify_call_req_msg_v01, new_call_type)
};

static const uint8_t csvt_modify_call_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_modify_call_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t csvt_modify_call_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_modify_call_ind_msg_v01, instance_id),

  0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_modify_call_ind_msg_v01, request_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_modify_call_ind_msg_v01, call_end_cause) - QMI_IDL_OFFSET8(csvt_modify_call_ind_msg_v01, call_end_cause_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_modify_call_ind_msg_v01, call_end_cause)
};

static const uint8_t csvt_ack_call_modify_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_ack_call_modify_req_msg_v01, instance_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_ack_call_modify_req_msg_v01, accept_request)
};

static const uint8_t csvt_ack_call_modify_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_ack_call_modify_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t csvt_ack_call_modify_ind_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_ack_call_modify_ind_msg_v01, instance_id),

  0x02,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_ack_call_modify_ind_msg_v01, request_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_ack_call_modify_ind_msg_v01, call_end_cause) - QMI_IDL_OFFSET8(csvt_ack_call_modify_ind_msg_v01, call_end_cause_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_ack_call_modify_ind_msg_v01, call_end_cause)
};

/* 
 * csvt_get_rlp_params_req_msg is empty
 * static const uint8_t csvt_get_rlp_params_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t csvt_get_rlp_params_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp1_parameters) - QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp1_parameters_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp1_parameters),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, v42_parameters) - QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, v42_parameters_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, v42_parameters),
  QMI_IDL_TYPE88(0, 3),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp2_parameters) - QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp2_parameters_valid)),
  0x12,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp2_parameters),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp3_parameters) - QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp3_parameters_valid)),
  0x13,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_rlp_params_resp_msg_v01, rlp3_parameters),
  QMI_IDL_TYPE88(0, 2)
};

static const uint8_t csvt_set_rlp_params_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_set_rlp_params_req_msg_v01, rlp_parameters) - QMI_IDL_OFFSET8(csvt_set_rlp_params_req_msg_v01, rlp_parameters_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_set_rlp_params_req_msg_v01, rlp_parameters),
  QMI_IDL_TYPE88(0, 2),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_set_rlp_params_req_msg_v01, v42_parameters) - QMI_IDL_OFFSET8(csvt_set_rlp_params_req_msg_v01, v42_parameters_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_set_rlp_params_req_msg_v01, v42_parameters),
  QMI_IDL_TYPE88(0, 3)
};

static const uint8_t csvt_set_rlp_params_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_set_rlp_params_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * csvt_get_active_call_list_req_msg is empty
 * static const uint8_t csvt_get_active_call_list_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t csvt_get_active_call_list_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_active_call_list_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_active_call_list_resp_msg_v01, active_call_inst_id) - QMI_IDL_OFFSET8(csvt_get_active_call_list_resp_msg_v01, active_call_inst_id_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_active_call_list_resp_msg_v01, active_call_inst_id),
  CSVT_MAX_ACTIVE_CALL_V01,
  QMI_IDL_OFFSET8(csvt_get_active_call_list_resp_msg_v01, active_call_inst_id) - QMI_IDL_OFFSET8(csvt_get_active_call_list_resp_msg_v01, active_call_inst_id_len)
};

static const uint8_t csvt_get_call_info_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_req_msg_v01, instance_id)
};

static const uint8_t csvt_get_call_info_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, call_type) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, call_type_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, call_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, synchronous_call) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, synchronous_call_valid)),
  0x11,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, synchronous_call),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, transparent_call) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, transparent_call_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, transparent_call),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, network_type) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, network_type_valid)),
  0x13,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, network_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, network_speed) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, network_speed_valid)),
  0x14,
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, network_speed),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, max_frame_size) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, max_frame_size_valid)),
  0x15,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, max_frame_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, incoming_number) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, incoming_number_valid)),
  0x16,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_STRING,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, incoming_number),
  CSVT_MAX_INCOM_NUM_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, uus_id) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, uus_id_valid)),
  0x17,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, uus_id),
  QMI_IDL_TYPE88(0, 1),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, modify_allowed) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, modify_allowed_valid)),
  0x18,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, modify_allowed),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, call_end_cause) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, call_end_cause_valid)),
  0x19,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, call_end_cause),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, port_data) - QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, port_data_valid)),
  0x1A,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_call_info_resp_msg_v01, port_data),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t csvt_get_call_stats_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_stats_req_msg_v01, instance_id)
};

static const uint8_t csvt_get_call_stats_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, call_active) - QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, call_active_valid)),
  0x10,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, call_active),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, tx_counter) - QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, tx_counter_valid)),
  0x11,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, tx_counter),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, rx_counter) - QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, rx_counter_valid)),
  0x12,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_call_stats_resp_msg_v01, rx_counter)
};

static const uint8_t csvt_set_subscription_binding_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_set_subscription_binding_req_msg_v01, bind_subs)
};

static const uint8_t csvt_set_subscription_binding_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_set_subscription_binding_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* 
 * csvt_get_bind_subscription_req_msg is empty
 * static const uint8_t csvt_get_bind_subscription_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t csvt_get_bind_subscription_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(csvt_get_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(csvt_get_bind_subscription_resp_msg_v01, bind_subscription) - QMI_IDL_OFFSET8(csvt_get_bind_subscription_resp_msg_v01, bind_subscription_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(csvt_get_bind_subscription_resp_msg_v01, bind_subscription)
};

/* Type Table */
static const qmi_idl_type_table_entry  csvt_type_table_v01[] = {
  {sizeof(csvt_port_id_type_v01), csvt_port_id_type_data_v01},
  {sizeof(csvt_uus_id_v01), csvt_uus_id_data_v01},
  {sizeof(rlp_params_v01), rlp_params_data_v01},
  {sizeof(v42_params_v01), v42_params_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry csvt_message_table_v01[] = {
  {sizeof(csvt_reset_req_msg_v01), 0},
  {sizeof(csvt_reset_resp_msg_v01), csvt_reset_resp_msg_data_v01},
  {sizeof(csvt_set_event_report_req_msg_v01), csvt_set_event_report_req_msg_data_v01},
  {sizeof(csvt_set_event_report_resp_msg_v01), csvt_set_event_report_resp_msg_data_v01},
  {sizeof(csvt_event_report_ind_msg_v01), csvt_event_report_ind_msg_data_v01},
  {sizeof(csvt_originate_call_req_msg_v01), csvt_originate_call_req_msg_data_v01},
  {sizeof(csvt_originate_call_resp_msg_v01), csvt_originate_call_resp_msg_data_v01},
  {sizeof(csvt_originate_call_ind_msg_v01), csvt_originate_call_ind_msg_data_v01},
  {sizeof(csvt_confirm_call_req_msg_v01), csvt_confirm_call_req_msg_data_v01},
  {sizeof(csvt_confirm_call_resp_msg_v01), csvt_confirm_call_resp_msg_data_v01},
  {sizeof(csvt_confirm_call_ind_msg_v01), csvt_confirm_call_ind_msg_data_v01},
  {sizeof(csvt_answer_call_req_msg_v01), csvt_answer_call_req_msg_data_v01},
  {sizeof(csvt_answer_call_resp_msg_v01), csvt_answer_call_resp_msg_data_v01},
  {sizeof(csvt_answer_call_ind_msg_v01), csvt_answer_call_ind_msg_data_v01},
  {sizeof(csvt_end_call_req_msg_v01), csvt_end_call_req_msg_data_v01},
  {sizeof(csvt_end_call_resp_msg_v01), csvt_end_call_resp_msg_data_v01},
  {sizeof(csvt_end_call_ind_msg_v01), csvt_end_call_ind_msg_data_v01},
  {sizeof(csvt_modify_call_req_msg_v01), csvt_modify_call_req_msg_data_v01},
  {sizeof(csvt_modify_call_resp_msg_v01), csvt_modify_call_resp_msg_data_v01},
  {sizeof(csvt_modify_call_ind_msg_v01), csvt_modify_call_ind_msg_data_v01},
  {sizeof(csvt_ack_call_modify_req_msg_v01), csvt_ack_call_modify_req_msg_data_v01},
  {sizeof(csvt_ack_call_modify_resp_msg_v01), csvt_ack_call_modify_resp_msg_data_v01},
  {sizeof(csvt_ack_call_modify_ind_msg_v01), csvt_ack_call_modify_ind_msg_data_v01},
  {sizeof(csvt_get_rlp_params_req_msg_v01), 0},
  {sizeof(csvt_get_rlp_params_resp_msg_v01), csvt_get_rlp_params_resp_msg_data_v01},
  {sizeof(csvt_set_rlp_params_req_msg_v01), csvt_set_rlp_params_req_msg_data_v01},
  {sizeof(csvt_set_rlp_params_resp_msg_v01), csvt_set_rlp_params_resp_msg_data_v01},
  {sizeof(csvt_get_active_call_list_req_msg_v01), 0},
  {sizeof(csvt_get_active_call_list_resp_msg_v01), csvt_get_active_call_list_resp_msg_data_v01},
  {sizeof(csvt_get_call_info_req_msg_v01), csvt_get_call_info_req_msg_data_v01},
  {sizeof(csvt_get_call_info_resp_msg_v01), csvt_get_call_info_resp_msg_data_v01},
  {sizeof(csvt_get_call_stats_req_msg_v01), csvt_get_call_stats_req_msg_data_v01},
  {sizeof(csvt_get_call_stats_resp_msg_v01), csvt_get_call_stats_resp_msg_data_v01},
  {sizeof(csvt_set_subscription_binding_req_msg_v01), csvt_set_subscription_binding_req_msg_data_v01},
  {sizeof(csvt_set_subscription_binding_resp_msg_v01), csvt_set_subscription_binding_resp_msg_data_v01},
  {sizeof(csvt_get_bind_subscription_req_msg_v01), 0},
  {sizeof(csvt_get_bind_subscription_resp_msg_v01), csvt_get_bind_subscription_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object csvt_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *csvt_qmi_idl_type_table_object_referenced_tables_v01[] =
{&csvt_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object csvt_qmi_idl_type_table_object_v01 = {
  sizeof(csvt_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(csvt_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  csvt_type_table_v01,
  csvt_message_table_v01,
  csvt_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry csvt_service_command_messages_v01[] = {
  {QMI_CSVT_RESET_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_CSVT_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_CSVT_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_CSVT_SET_EVENT_REPORT_REQ_V01, QMI_IDL_TYPE16(0, 2), 15},
  {QMI_CSVT_ORIGINATE_CALL_REQ_V01, QMI_IDL_TYPE16(0, 5), 176},
  {QMI_CSVT_CONFIRM_CALL_REQ_V01, QMI_IDL_TYPE16(0, 8), 15},
  {QMI_CSVT_ANSWER_CALL_REQ_V01, QMI_IDL_TYPE16(0, 11), 15},
  {QMI_CSVT_END_CALL_REQ_V01, QMI_IDL_TYPE16(0, 14), 7},
  {QMI_CSVT_MODIFY_CALL_REQ_V01, QMI_IDL_TYPE16(0, 17), 14},
  {QMI_CSVT_ACK_CALL_MODIFY_REQ_V01, QMI_IDL_TYPE16(0, 20), 11},
  {QMI_CSVT_GET_RLP_PARAMS_REQ_V01, QMI_IDL_TYPE16(0, 23), 0},
  {QMI_CSVT_SET_RLP_PARAMS_REQ_V01, QMI_IDL_TYPE16(0, 25), 46},
  {QMI_CSVT_GET_ACTIVE_CALL_LIST_REQ_V01, QMI_IDL_TYPE16(0, 27), 0},
  {QMI_CSVT_GET_CALL_INFO_REQ_V01, QMI_IDL_TYPE16(0, 29), 7},
  {QMI_CSVT_GET_CALL_STATS_REQ_V01, QMI_IDL_TYPE16(0, 31), 7},
  {QMI_CSVT_SET_SUBSCRIPTION_BINDING_REQ_V01, QMI_IDL_TYPE16(0, 33), 7},
  {QMI_CSVT_GET_BIND_SUBSCRIPTION_REQ_V01, QMI_IDL_TYPE16(0, 35), 0}
};

static const qmi_idl_service_message_table_entry csvt_service_response_messages_v01[] = {
  {QMI_CSVT_RESET_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_CSVT_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_CSVT_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_CSVT_SET_EVENT_REPORT_RESP_V01, QMI_IDL_TYPE16(0, 3), 7},
  {QMI_CSVT_ORIGINATE_CALL_RESP_V01, QMI_IDL_TYPE16(0, 6), 7},
  {QMI_CSVT_CONFIRM_CALL_RESP_V01, QMI_IDL_TYPE16(0, 9), 7},
  {QMI_CSVT_ANSWER_CALL_RESP_V01, QMI_IDL_TYPE16(0, 12), 7},
  {QMI_CSVT_END_CALL_RESP_V01, QMI_IDL_TYPE16(0, 15), 7},
  {QMI_CSVT_MODIFY_CALL_RESP_V01, QMI_IDL_TYPE16(0, 18), 7},
  {QMI_CSVT_ACK_CALL_MODIFY_RESP_V01, QMI_IDL_TYPE16(0, 21), 7},
  {QMI_CSVT_GET_RLP_PARAMS_RESP_V01, QMI_IDL_TYPE16(0, 24), 107},
  {QMI_CSVT_SET_RLP_PARAMS_RESP_V01, QMI_IDL_TYPE16(0, 26), 7},
  {QMI_CSVT_GET_ACTIVE_CALL_LIST_RESP_V01, QMI_IDL_TYPE16(0, 28), 19},
  {QMI_CSVT_GET_CALL_INFO_RESP_V01, QMI_IDL_TYPE16(0, 30), 122},
  {QMI_CSVT_GET_CALL_STATS_RESP_V01, QMI_IDL_TYPE16(0, 32), 25},
  {QMI_CSVT_SET_SUBSCRIPTION_BINDING_RESP_V01, QMI_IDL_TYPE16(0, 34), 7},
  {QMI_CSVT_GET_BIND_SUBSCRIPTION_RESP_V01, QMI_IDL_TYPE16(0, 36), 14}
};

static const qmi_idl_service_message_table_entry csvt_service_indication_messages_v01[] = {
  {QMI_CSVT_EVENT_REPORT_IND_V01, QMI_IDL_TYPE16(0, 4), 136},
  {QMI_CSVT_ORIGINATE_CALL_IND_V01, QMI_IDL_TYPE16(0, 7), 18},
  {QMI_CSVT_CONFIRM_CALL_IND_V01, QMI_IDL_TYPE16(0, 10), 18},
  {QMI_CSVT_ANSWER_CALL_IND_V01, QMI_IDL_TYPE16(0, 13), 18},
  {QMI_CSVT_END_CALL_IND_V01, QMI_IDL_TYPE16(0, 16), 11},
  {QMI_CSVT_MODIFY_CALL_IND_V01, QMI_IDL_TYPE16(0, 19), 18},
  {QMI_CSVT_ACK_CALL_MODIFY_IND_V01, QMI_IDL_TYPE16(0, 22), 18}
};

/*Service Object*/
struct qmi_idl_service_object csvt_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x1D,
  8204,
  { sizeof(csvt_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(csvt_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(csvt_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { csvt_service_command_messages_v01, csvt_service_response_messages_v01, csvt_service_indication_messages_v01},
  &csvt_qmi_idl_type_table_object_v01,
  0x06,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type csvt_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( CSVT_V01_IDL_MAJOR_VERS != idl_maj_version || CSVT_V01_IDL_MINOR_VERS != idl_min_version 
       || CSVT_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&csvt_qmi_idl_service_object_v01;
}

