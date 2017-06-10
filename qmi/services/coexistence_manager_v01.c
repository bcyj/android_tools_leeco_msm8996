/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        C O E X I S T E N C E _ M A N A G E R _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the cxm service Data structures.

  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/modem/arch/qmi_cxm/coexistence_manager_v01.c#3 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 2.8
   It was generated on: Wed Oct 26 2011
   From IDL File: coexistence_manager_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "coexistence_manager_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t cxm_lte_ml1_state_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, is_connected),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, is_high_priority),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, mask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, start_time),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, end_time),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, dl_earfcn),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, ul_earfcn),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, dl_bandwidth),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, ul_bandwidth),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, frame_structure),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, tdd_config),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, ssp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, dl_cp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_state_s_v01, ul_cp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cxm_lte_ml1_coex_snr_info_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_snr_info_s_v01, snr_is_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_snr_info_s_v01, subframe),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_snr_info_s_v01, snr),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cxm_lte_ml1_coex_tx_event_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, subframe),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, tx_is_scheduled),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, channel_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, priority),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, slot0_power),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, slot0_first_rb),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, slot0_last_rb),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, slot1_power),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, slot1_first_rb),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, slot1_last_rb),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, srs_power),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, srs_first_rb),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_tx_event_s_v01, srs_last_rb),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cxm_lte_ml1_coex_ustmr_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_ustmr_s_v01, dl_subframe),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_ustmr_s_v01, dl_frame_time),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_ustmr_s_v01, ul_subframe),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_ustmr_s_v01, ul_frame_time),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cxm_lte_ml1_coex_frame_timing_s_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_frame_timing_s_v01, report_type),

   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_lte_ml1_coex_frame_timing_s_v01, timing),
 3, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cxm_lte_phr_backoff_req_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_phr_backoff_req_s_v01, backoff_mtpl),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_phr_backoff_req_s_v01, priority_threshold),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t cxm_lte_phr_less_backoff_req_s_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_phr_less_backoff_req_s_v01, backoff_mtpl),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_phr_less_backoff_req_s_v01, priority_threshold),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_phr_less_backoff_req_s_v01, starting_time),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_lte_phr_less_backoff_req_s_v01, num_of_subframes),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t cxm_unused_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_unused_resp_msg_v01, resp),
  0, 1
};

static const uint8_t cxm_activate_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(cxm_activate_req_msg_v01, data_plane_enable)
};

/* 
 * cxm_deactivate_req_msg is empty
 * static const uint8_t cxm_deactivate_req_msg_data_v01[] = {
 * };
 */
  
/* 
 * cxm_state_req_msg is empty
 * static const uint8_t cxm_state_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t cxm_state_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_state_ind_msg_v01, lte_ml1_state),
  0, 0
};

static const uint8_t cxm_notify_event_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cxm_notify_event_ind_msg_v01, lte_ml1_tx_event_info) - QMI_IDL_OFFSET8(cxm_notify_event_ind_msg_v01, lte_ml1_tx_event_info_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_notify_event_ind_msg_v01, lte_ml1_tx_event_info),
  2, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cxm_notify_event_ind_msg_v01, lte_ml1_snr_info) - QMI_IDL_OFFSET8(cxm_notify_event_ind_msg_v01, lte_ml1_snr_info_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_notify_event_ind_msg_v01, lte_ml1_snr_info),
  1, 0
};

static const uint8_t cxm_frame_timing_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_frame_timing_ind_msg_v01, frame_timing_report_list),
  CXM_LTE_ML1_FRAME_TIMING_REPORT_LIST_MAX_V01,
  QMI_IDL_OFFSET8(cxm_frame_timing_ind_msg_v01, frame_timing_report_list) - QMI_IDL_OFFSET8(cxm_frame_timing_ind_msg_v01, frame_timing_report_list_len),
  4, 0
};

/* 
 * cxm_stmr_read_req_msg is empty
 * static const uint8_t cxm_stmr_read_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t cxm_stmr_read_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_stmr_read_resp_msg_v01, resp),
  0, 1
};

/* 
 * cxm_stmr_read_complete_req_msg is empty
 * static const uint8_t cxm_stmr_read_complete_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t cxm_phr_backoff_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cxm_phr_backoff_req_msg_v01, phr_backoff) - QMI_IDL_OFFSET8(cxm_phr_backoff_req_msg_v01, phr_backoff_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_phr_backoff_req_msg_v01, phr_backoff),
  5, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(cxm_phr_backoff_req_msg_v01, phr_less_backoff) - QMI_IDL_OFFSET8(cxm_phr_backoff_req_msg_v01, phr_less_backoff_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_phr_backoff_req_msg_v01, phr_less_backoff),
  6, 0
};

static const uint8_t cxm_phr_backoff_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(cxm_phr_backoff_resp_msg_v01, resp),
  0, 1
};

/* Type Table */
static const qmi_idl_type_table_entry  cxm_type_table_v01[] = {
  {sizeof(cxm_lte_ml1_state_s_v01), cxm_lte_ml1_state_s_data_v01},
  {sizeof(cxm_lte_ml1_coex_snr_info_s_v01), cxm_lte_ml1_coex_snr_info_s_data_v01},
  {sizeof(cxm_lte_ml1_coex_tx_event_s_v01), cxm_lte_ml1_coex_tx_event_s_data_v01},
  {sizeof(cxm_lte_ml1_coex_ustmr_s_v01), cxm_lte_ml1_coex_ustmr_s_data_v01},
  {sizeof(cxm_lte_ml1_coex_frame_timing_s_v01), cxm_lte_ml1_coex_frame_timing_s_data_v01},
  {sizeof(cxm_lte_phr_backoff_req_s_v01), cxm_lte_phr_backoff_req_s_data_v01},
  {sizeof(cxm_lte_phr_less_backoff_req_s_v01), cxm_lte_phr_less_backoff_req_s_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry cxm_message_table_v01[] = {
  {sizeof(cxm_unused_resp_msg_v01), cxm_unused_resp_msg_data_v01},
  {sizeof(cxm_activate_req_msg_v01), cxm_activate_req_msg_data_v01},
  {0, 0},
  {0, 0},
  {sizeof(cxm_state_ind_msg_v01), cxm_state_ind_msg_data_v01},
  {sizeof(cxm_notify_event_ind_msg_v01), cxm_notify_event_ind_msg_data_v01},
  {sizeof(cxm_frame_timing_ind_msg_v01), cxm_frame_timing_ind_msg_data_v01},
  {0, 0},
  {sizeof(cxm_stmr_read_resp_msg_v01), cxm_stmr_read_resp_msg_data_v01},
  {0, 0},
  {sizeof(cxm_phr_backoff_req_msg_v01), cxm_phr_backoff_req_msg_data_v01},
  {sizeof(cxm_phr_backoff_resp_msg_v01), cxm_phr_backoff_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object cxm_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *cxm_qmi_idl_type_table_object_referenced_tables_v01[] =
{&cxm_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object cxm_qmi_idl_type_table_object_v01 = {
  sizeof(cxm_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(cxm_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  cxm_type_table_v01,
  cxm_message_table_v01,
  cxm_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry cxm_service_command_messages_v01[] = {
  {QMI_CXM_ACTIVATE_REQ_MSG_V01, TYPE16(0, 1), 4},
  {QMI_CXM_DEACTIVATE_REQ_MSG_V01, TYPE16(0, 2), 0},
  {QMI_CXM_STATE_REQ_MSG_V01, TYPE16(0, 3), 0},
  {QMI_CXM_STMR_READ_REQ_MSG_V01, TYPE16(0, 7), 0},
  {QMI_CXM_STMR_READ_COMPLETE_REQ_MSG_V01, TYPE16(0, 9), 0},
  {QMI_CXM_PHR_BACKOFF_REQ_MSG_V01, TYPE16(0, 10), 15}
};

static const qmi_idl_service_message_table_entry cxm_service_response_messages_v01[] = {
  {QMI_CXM_ACTIVATE_RESP_MSG_V01, TYPE16(0, 0), 7},
  {QMI_CXM_DEACTIVATE_RESP_MSG_V01, TYPE16(0, 0), 7},
  {QMI_CXM_STATE_RESP_MSG_V01, TYPE16(0, 0), 7},
  {QMI_CXM_STMR_READ_RESP_MSG_V01, TYPE16(0, 8), 7},
  {QMI_CXM_STMR_READ_COMPLETE_RESP_MSG_V01, TYPE16(0, 0), 7},
  {QMI_CXM_PHR_BACKOFF_RESP_MSG_V01, TYPE16(0, 11), 7}
};

static const qmi_idl_service_message_table_entry cxm_service_indication_messages_v01[] = {
  {QMI_CXM_STATE_IND_MSG_V01, TYPE16(0, 4), 47},
  {QMI_CXM_NOTIFY_EVENT_IND_V01, TYPE16(0, 5), 28},
  {QMI_CXM_FRAME_TIMING_IND_V01, TYPE16(0, 6), 88}
};

/*Service Object*/
const struct qmi_idl_service_object cxm_qmi_idl_service_object_v01 = {
  0x02,
  0x01,
  0x01,
  88,
  { sizeof(cxm_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(cxm_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(cxm_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { cxm_service_command_messages_v01, cxm_service_response_messages_v01, cxm_service_indication_messages_v01},
  &cxm_qmi_idl_type_table_object_v01
};

/* Service Object Accessor */
qmi_idl_service_object_type cxm_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( CXM_V01_IDL_MAJOR_VERS != idl_maj_version || CXM_V01_IDL_MINOR_VERS != idl_min_version 
       || CXM_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&cxm_qmi_idl_service_object_v01;
}

