/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        Q M I _ E M B M S _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the embms service Data structures.

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.



  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.4
   It was generated on: Thu Mar 26 2015 (Spin 0)
   From IDL File: qmi_embms_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "qmi_embms_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t embms_tmgi_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_tmgi_type_v01, tmgi),
  TMGI_LENGTH_MAX_V01,
  QMI_IDL_OFFSET8(embms_tmgi_type_v01, tmgi) - QMI_IDL_OFFSET8(embms_tmgi_type_v01, tmgi_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t embms_content_desc_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_content_desc_type_v01, paramCode),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_content_desc_type_v01, paramValue),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t embms_plmn_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(embms_plmn_type_v01, mcc),
  MCC_MNC_MAX_V01,

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(embms_plmn_type_v01, mnc),
  MCC_MNC_MAX_V01,

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t embms_enable_embms_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_enable_embms_req_msg_v01, dbg_trace_id)
};

static const uint8_t embms_enable_embms_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, call_id) - QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, interface_name) - QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, interface_name_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, interface_name),
  NUMBER_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, if_index) - QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, if_index_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_enable_embms_resp_msg_v01, if_index)
};

static const uint8_t embms_disable_embms_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_disable_embms_req_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_disable_embms_req_msg_v01, call_id)
};

static const uint8_t embms_disable_embms_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_disable_embms_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_disable_embms_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_disable_embms_resp_msg_v01, call_id) - QMI_IDL_OFFSET8(embms_disable_embms_resp_msg_v01, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_disable_embms_resp_msg_v01, call_id)
};

static const uint8_t embms_activate_tmgi_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, call_id),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, tmgi_info),
  QMI_IDL_TYPE88(0, 0),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, preemption_priority),

  0x05,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, earfcnlist),
  EARFCNLIST_MAX_V01,
  QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, earfcnlist) - QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, earfcnlist_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, saiList) - QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, saiList_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, saiList),
  SAI_PER_FREQ_MAX_V01,
  QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, saiList) - QMI_IDL_OFFSET8(embms_activate_tmgi_req_msg_v01, saiList_len)
};

static const uint8_t embms_activate_tmgi_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_activate_tmgi_resp_msg_v01, call_id) - QMI_IDL_OFFSET8(embms_activate_tmgi_resp_msg_v01, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_activate_tmgi_resp_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_activate_tmgi_resp_msg_v01, tmgi_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_activate_tmgi_resp_msg_v01, tmgi_info),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t embms_deactivate_tmgi_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_deactivate_tmgi_req_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_deactivate_tmgi_req_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_deactivate_tmgi_req_msg_v01, tmgi_info),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t embms_deactivate_tmgi_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_deactivate_tmgi_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_deactivate_tmgi_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_deactivate_tmgi_resp_msg_v01, call_id) - QMI_IDL_OFFSET8(embms_deactivate_tmgi_resp_msg_v01, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_deactivate_tmgi_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_deactivate_tmgi_resp_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_deactivate_tmgi_resp_msg_v01, tmgi_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_deactivate_tmgi_resp_msg_v01, tmgi_info),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t embms_activate_deactivate_tmgi_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, call_id),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, act_tmgi_info),
  QMI_IDL_TYPE88(0, 0),

  0x04,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, deact_tmgi_info),
  QMI_IDL_TYPE88(0, 0),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, preemption_priority),

  0x06,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, earfcnlist),
  EARFCNLIST_MAX_V01,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, earfcnlist) - QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, earfcnlist_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, saiList) - QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, saiList_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, saiList),
  SAI_PER_FREQ_MAX_V01,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, saiList) - QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_req_msg_v01, saiList_len)
};

static const uint8_t embms_activate_deactivate_tmgi_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, act_resp_code),

  0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, deact_resp_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, call_id) - QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, call_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, call_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, act_tmgi_info) - QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, act_tmgi_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, act_tmgi_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, deact_tmgi_info) - QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, deact_tmgi_info_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_activate_deactivate_tmgi_resp_msg_v01, deact_tmgi_info),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t embms_get_available_tmgi_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_available_tmgi_req_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_get_available_tmgi_req_msg_v01, call_id)
};

static const uint8_t embms_get_available_tmgi_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_available_tmgi_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_available_tmgi_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_available_tmgi_resp_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_get_available_tmgi_resp_msg_v01, tmgi_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_get_available_tmgi_resp_msg_v01, tmgi_info),
  NUMBER_MAX_V01,
  QMI_IDL_OFFSET8(embms_get_available_tmgi_resp_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_get_available_tmgi_resp_msg_v01, tmgi_info_len),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t embms_get_active_tmgi_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_active_tmgi_req_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_get_active_tmgi_req_msg_v01, call_id)
};

static const uint8_t embms_get_active_tmgi_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_active_tmgi_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_active_tmgi_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_active_tmgi_resp_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_get_active_tmgi_resp_msg_v01, tmgi_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_get_active_tmgi_resp_msg_v01, tmgi_info),
  NUMBER_MAX_V01,
  QMI_IDL_OFFSET8(embms_get_active_tmgi_resp_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_get_active_tmgi_resp_msg_v01, tmgi_info_len),
  QMI_IDL_TYPE88(0, 0)
};

/*
 * embms_enable_rssi_req_msg is empty
 * static const uint8_t embms_enable_rssi_req_msg_data_v01[] = {
 * };
 */

/*
 * embms_enable_rssi_resp_msg is empty
 * static const uint8_t embms_enable_rssi_resp_msg_data_v01[] = {
 * };
 */

/*
 * embms_disable_rssi_req_msg is empty
 * static const uint8_t embms_disable_rssi_req_msg_data_v01[] = {
 * };
 */

/*
 * embms_disable_rssi_resp_msg is empty
 * static const uint8_t embms_disable_rssi_resp_msg_data_v01[] = {
 * };
 */

static const uint8_t embms_get_coverage_state_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_coverage_state_req_msg_v01, dbg_trace_id)
};

static const uint8_t embms_get_coverage_state_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_coverage_state_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_coverage_state_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_coverage_state_resp_msg_v01, coverage_state) - QMI_IDL_OFFSET8(embms_get_coverage_state_resp_msg_v01, coverage_state_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_coverage_state_resp_msg_v01, coverage_state)
};

static const uint8_t embms_get_rssi_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_rssi_req_msg_v01, dbg_trace_id)
};

static const uint8_t embms_get_rssi_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, area_id) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, area_id_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, area_id),
  SIG_MAX_MBSFN_AREA_V01,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, area_id) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, area_id_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, sig_noise_ratio) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, sig_noise_ratio_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, sig_noise_ratio),
  SIG_MAX_MBSFN_AREA_V01,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, sig_noise_ratio) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, sig_noise_ratio_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, excess_snr) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, excess_snr_valid)),
  0x12,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, excess_snr),
  SIG_MAX_MBSFN_AREA_V01,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, excess_snr) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, excess_snr_len),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, number_of_tmgi_per_mbsfn) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, number_of_tmgi_per_mbsfn_valid)),
  0x13,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, number_of_tmgi_per_mbsfn),
  SIG_MAX_MBSFN_AREA_V01,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, number_of_tmgi_per_mbsfn) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, number_of_tmgi_per_mbsfn_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, active_tgmi) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, active_tgmi_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, active_tgmi),
  ((SIG_MAX_TMGI_V01) & 0xFF), ((SIG_MAX_TMGI_V01) >> 8),
  QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, active_tgmi) - QMI_IDL_OFFSET8(embms_get_rssi_resp_msg_v01, active_tgmi_len),
  QMI_IDL_TYPE88(0, 0)
};

/*
 * embms_get_embms_service_state_req_msg is empty
 * static const uint8_t embms_get_embms_service_state_req_msg_data_v01[] = {
 * };
 */

static const uint8_t embms_get_embms_service_state_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_embms_service_state_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_embms_service_state_resp_msg_v01, embms_service_state) - QMI_IDL_OFFSET8(embms_get_embms_service_state_resp_msg_v01, embms_service_state_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_embms_service_state_resp_msg_v01, embms_service_state)
};

static const uint8_t embms_unsol_embms_service_state_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_embms_service_state_ind_msg_v01, state),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(embms_unsol_embms_service_state_ind_msg_v01, interface_name),
  NUMBER_MAX_V01
};

static const uint8_t embms_unsol_active_tmgi_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_active_tmgi_ind_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_unsol_active_tmgi_ind_msg_v01, tmgi_info),
  NUMBER_MAX_V01,
  QMI_IDL_OFFSET8(embms_unsol_active_tmgi_ind_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_unsol_active_tmgi_ind_msg_v01, tmgi_info_len),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t embms_unsol_broadcast_coverage_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_broadcast_coverage_ind_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_broadcast_coverage_ind_msg_v01, broadcast_coverage)
};

static const uint8_t embms_unsol_rssi_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_rssi_ind_msg_v01, rssi)
};

static const uint8_t embms_unsol_available_tmgi_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_available_tmgi_ind_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_unsol_available_tmgi_ind_msg_v01, tmgi_info),
  NUMBER_MAX_V01,
  QMI_IDL_OFFSET8(embms_unsol_available_tmgi_ind_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_unsol_available_tmgi_ind_msg_v01, tmgi_info_len),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t embms_unsol_oos_warning_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_oos_warning_ind_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_oos_warning_ind_msg_v01, reason),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_unsol_oos_warning_ind_msg_v01, tmgi_info),
  NUMBER_MAX_V01,
  QMI_IDL_OFFSET8(embms_unsol_oos_warning_ind_msg_v01, tmgi_info) - QMI_IDL_OFFSET8(embms_unsol_oos_warning_ind_msg_v01, tmgi_info_len),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t embms_unsol_cell_info_changed_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_cell_info_changed_ind_msg_v01, dbg_trace_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(embms_unsol_cell_info_changed_ind_msg_v01, mcc),
  MCC_MNC_MAX_V01,

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(embms_unsol_cell_info_changed_ind_msg_v01, mnc),
  MCC_MNC_MAX_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_cell_info_changed_ind_msg_v01, cell_id)
};

static const uint8_t embms_unsol_radio_state_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_radio_state_ind_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_radio_state_ind_msg_v01, radio_state)
};

static const uint8_t embms_unsol_sai_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_sai_ind_msg_v01, dbg_trace_id),

  0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_sai_ind_msg_v01, camped_sai_list),
  SAI_PER_FREQ_MAX_V01,
  QMI_IDL_OFFSET8(embms_unsol_sai_ind_msg_v01, camped_sai_list) - QMI_IDL_OFFSET8(embms_unsol_sai_ind_msg_v01, camped_sai_list_len),

  0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(embms_unsol_sai_ind_msg_v01, num_of_sai_per_group),
  FREQ_MAX_V01,
  QMI_IDL_OFFSET16RELATIVE(embms_unsol_sai_ind_msg_v01, num_of_sai_per_group) - QMI_IDL_OFFSET16RELATIVE(embms_unsol_sai_ind_msg_v01, num_of_sai_per_group_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(embms_unsol_sai_ind_msg_v01, available_sai_list),
  ((SAI_MAX_V01) & 0xFF), ((SAI_MAX_V01) >> 8),
  QMI_IDL_OFFSET16RELATIVE(embms_unsol_sai_ind_msg_v01, available_sai_list) - QMI_IDL_OFFSET16RELATIVE(embms_unsol_sai_ind_msg_v01, available_sai_list_len)
};

static const uint8_t embms_get_active_log_packet_ids_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_active_log_packet_ids_req_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_active_log_packet_ids_req_msg_v01, supported_log_packet_id_list),
  ((LOG_PACKET_ID_MAX_V01) & 0xFF), ((LOG_PACKET_ID_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(embms_get_active_log_packet_ids_req_msg_v01, supported_log_packet_id_list) - QMI_IDL_OFFSET8(embms_get_active_log_packet_ids_req_msg_v01, supported_log_packet_id_list_len)
};

static const uint8_t embms_get_active_log_packet_ids_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_active_log_packet_ids_resp_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_active_log_packet_ids_resp_msg_v01, active_log_packet_id_list),
  ((LOG_PACKET_ID_MAX_V01) & 0xFF), ((LOG_PACKET_ID_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(embms_get_active_log_packet_ids_resp_msg_v01, active_log_packet_id_list) - QMI_IDL_OFFSET8(embms_get_active_log_packet_ids_resp_msg_v01, active_log_packet_id_list_len)
};

static const uint8_t embms_deliver_log_packet_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_deliver_log_packet_req_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_deliver_log_packet_req_msg_v01, packet_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_deliver_log_packet_req_msg_v01, log_packet),
  ((LOG_PACKET_SIZE_MAX_V01) & 0xFF), ((LOG_PACKET_SIZE_MAX_V01) >> 8),
  QMI_IDL_OFFSET8(embms_deliver_log_packet_req_msg_v01, log_packet) - QMI_IDL_OFFSET8(embms_deliver_log_packet_req_msg_v01, log_packet_len)
};

static const uint8_t embms_deliver_log_packet_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_deliver_log_packet_resp_msg_v01, dbg_trace_id)
};

static const uint8_t embms_set_sntp_time_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_set_sntp_time_req_msg_v01, sntp_available),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_set_sntp_time_req_msg_v01, sntp_time_milli_sec) - QMI_IDL_OFFSET8(embms_set_sntp_time_req_msg_v01, sntp_time_milli_sec_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(embms_set_sntp_time_req_msg_v01, sntp_time_milli_sec),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_set_sntp_time_req_msg_v01, time_stamp_milli_sec) - QMI_IDL_OFFSET8(embms_set_sntp_time_req_msg_v01, time_stamp_milli_sec_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(embms_set_sntp_time_req_msg_v01, time_stamp_milli_sec)
};

/*
 * embms_set_sntp_time_resp_msg is empty
 * static const uint8_t embms_set_sntp_time_resp_msg_data_v01[] = {
 * };
 */

/*
 * embms_get_sib16_coverage_req_msg is empty
 * static const uint8_t embms_get_sib16_coverage_req_msg_data_v01[] = {
 * };
 */

static const uint8_t embms_get_sib16_coverage_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_get_sib16_coverage_resp_msg_v01, in_coverage)
};

static const uint8_t embms_unsol_sib16_coverage_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_sib16_coverage_ind_msg_v01, in_coverage)
};

static const uint8_t embms_get_utc_time_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_utc_time_req_msg_v01, dbg_trace_id)
};

static const uint8_t embms_get_utc_time_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, resp_code),

  0x03,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, milli_sec),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, day_light_saving) - QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, day_light_saving_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, day_light_saving),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, leap_seconds) - QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, leap_seconds_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, leap_seconds),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, local_time_offset) - QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, local_time_offset_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_get_utc_time_resp_msg_v01, local_time_offset)
};

static const uint8_t embms_get_e911_state_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_e911_state_req_msg_v01, dbg_trace_id)
};

static const uint8_t embms_get_e911_state_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_e911_state_resp_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_e911_state_resp_msg_v01, resp_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_get_e911_state_resp_msg_v01, e911_state) - QMI_IDL_OFFSET8(embms_get_e911_state_resp_msg_v01, e911_state_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_e911_state_resp_msg_v01, e911_state)
};

static const uint8_t embms_unsol_e911_state_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_e911_state_ind_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_e911_state_ind_msg_v01, e911_state)
};

static const uint8_t embms_update_content_desc_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_update_content_desc_req_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_update_content_desc_req_msg_v01, call_id),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_update_content_desc_req_msg_v01, tmgi_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_update_content_desc_req_msg_v01, content_desc) - QMI_IDL_OFFSET8(embms_update_content_desc_req_msg_v01, content_desc_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_update_content_desc_req_msg_v01, content_desc),
  CONTENT_PARAM_NUM_MAX_V01,
  QMI_IDL_OFFSET8(embms_update_content_desc_req_msg_v01, content_desc) - QMI_IDL_OFFSET8(embms_update_content_desc_req_msg_v01, content_desc_len),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t embms_update_content_desc_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_update_content_desc_resp_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_update_content_desc_resp_msg_v01, resp_code)
};

static const uint8_t embms_unsol_content_desc_update_per_obj_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_content_desc_update_per_obj_ind_msg_v01, dbg_trace_id),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_unsol_content_desc_update_per_obj_ind_msg_v01, tmgi_info),
  QMI_IDL_TYPE88(0, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_unsol_content_desc_update_per_obj_ind_msg_v01, per_object_content_ctrl) - QMI_IDL_OFFSET8(embms_unsol_content_desc_update_per_obj_ind_msg_v01, per_object_content_ctrl_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_content_desc_update_per_obj_ind_msg_v01, per_object_content_ctrl),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(embms_unsol_content_desc_update_per_obj_ind_msg_v01, per_object_status_ctrl) - QMI_IDL_OFFSET8(embms_unsol_content_desc_update_per_obj_ind_msg_v01, per_object_status_ctrl_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_content_desc_update_per_obj_ind_msg_v01, per_object_status_ctrl)
};

static const uint8_t embms_get_sib_plmn_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_sib_plmn_req_msg_v01, dbg_trace_id)
};

static const uint8_t embms_get_sib_plmn_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_sib_plmn_resp_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(embms_get_sib_plmn_resp_msg_v01, plmn),
  PLMN_LIST_MAX_V01,
  QMI_IDL_OFFSET8(embms_get_sib_plmn_resp_msg_v01, plmn) - QMI_IDL_OFFSET8(embms_get_sib_plmn_resp_msg_v01, plmn_len),
  QMI_IDL_TYPE88(0, 2)
};

static const uint8_t embms_unsol_embms_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_unsol_embms_status_ind_msg_v01, is_available)
};

static const uint8_t embms_get_embms_status_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_embms_status_req_msg_v01, dbg_trace_id)
};

static const uint8_t embms_get_embms_status_resp_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(embms_get_embms_status_resp_msg_v01, dbg_trace_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(embms_get_embms_status_resp_msg_v01, is_available)
};

/* Type Table */
static const qmi_idl_type_table_entry  embms_type_table_v01[] = {
  {sizeof(embms_tmgi_type_v01), embms_tmgi_type_data_v01},
  {sizeof(embms_content_desc_type_v01), embms_content_desc_type_data_v01},
  {sizeof(embms_plmn_type_v01), embms_plmn_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry embms_message_table_v01[] = {
  {sizeof(embms_enable_embms_req_msg_v01), embms_enable_embms_req_msg_data_v01},
  {sizeof(embms_enable_embms_resp_msg_v01), embms_enable_embms_resp_msg_data_v01},
  {sizeof(embms_disable_embms_req_msg_v01), embms_disable_embms_req_msg_data_v01},
  {sizeof(embms_disable_embms_resp_msg_v01), embms_disable_embms_resp_msg_data_v01},
  {sizeof(embms_activate_tmgi_req_msg_v01), embms_activate_tmgi_req_msg_data_v01},
  {sizeof(embms_activate_tmgi_resp_msg_v01), embms_activate_tmgi_resp_msg_data_v01},
  {sizeof(embms_deactivate_tmgi_req_msg_v01), embms_deactivate_tmgi_req_msg_data_v01},
  {sizeof(embms_deactivate_tmgi_resp_msg_v01), embms_deactivate_tmgi_resp_msg_data_v01},
  {sizeof(embms_activate_deactivate_tmgi_req_msg_v01), embms_activate_deactivate_tmgi_req_msg_data_v01},
  {sizeof(embms_activate_deactivate_tmgi_resp_msg_v01), embms_activate_deactivate_tmgi_resp_msg_data_v01},
  {sizeof(embms_get_available_tmgi_req_msg_v01), embms_get_available_tmgi_req_msg_data_v01},
  {sizeof(embms_get_available_tmgi_resp_msg_v01), embms_get_available_tmgi_resp_msg_data_v01},
  {sizeof(embms_get_active_tmgi_req_msg_v01), embms_get_active_tmgi_req_msg_data_v01},
  {sizeof(embms_get_active_tmgi_resp_msg_v01), embms_get_active_tmgi_resp_msg_data_v01},
  {sizeof(embms_enable_rssi_req_msg_v01), 0},
  {sizeof(embms_enable_rssi_resp_msg_v01), 0},
  {sizeof(embms_disable_rssi_req_msg_v01), 0},
  {sizeof(embms_disable_rssi_resp_msg_v01), 0},
  {sizeof(embms_get_coverage_state_req_msg_v01), embms_get_coverage_state_req_msg_data_v01},
  {sizeof(embms_get_coverage_state_resp_msg_v01), embms_get_coverage_state_resp_msg_data_v01},
  {sizeof(embms_get_rssi_req_msg_v01), embms_get_rssi_req_msg_data_v01},
  {sizeof(embms_get_rssi_resp_msg_v01), embms_get_rssi_resp_msg_data_v01},
  {sizeof(embms_get_embms_service_state_req_msg_v01), 0},
  {sizeof(embms_get_embms_service_state_resp_msg_v01), embms_get_embms_service_state_resp_msg_data_v01},
  {sizeof(embms_unsol_embms_service_state_ind_msg_v01), embms_unsol_embms_service_state_ind_msg_data_v01},
  {sizeof(embms_unsol_active_tmgi_ind_msg_v01), embms_unsol_active_tmgi_ind_msg_data_v01},
  {sizeof(embms_unsol_broadcast_coverage_ind_msg_v01), embms_unsol_broadcast_coverage_ind_msg_data_v01},
  {sizeof(embms_unsol_rssi_ind_msg_v01), embms_unsol_rssi_ind_msg_data_v01},
  {sizeof(embms_unsol_available_tmgi_ind_msg_v01), embms_unsol_available_tmgi_ind_msg_data_v01},
  {sizeof(embms_unsol_oos_warning_ind_msg_v01), embms_unsol_oos_warning_ind_msg_data_v01},
  {sizeof(embms_unsol_cell_info_changed_ind_msg_v01), embms_unsol_cell_info_changed_ind_msg_data_v01},
  {sizeof(embms_unsol_radio_state_ind_msg_v01), embms_unsol_radio_state_ind_msg_data_v01},
  {sizeof(embms_unsol_sai_ind_msg_v01), embms_unsol_sai_ind_msg_data_v01},
  {sizeof(embms_get_active_log_packet_ids_req_msg_v01), embms_get_active_log_packet_ids_req_msg_data_v01},
  {sizeof(embms_get_active_log_packet_ids_resp_msg_v01), embms_get_active_log_packet_ids_resp_msg_data_v01},
  {sizeof(embms_deliver_log_packet_req_msg_v01), embms_deliver_log_packet_req_msg_data_v01},
  {sizeof(embms_deliver_log_packet_resp_msg_v01), embms_deliver_log_packet_resp_msg_data_v01},
  {sizeof(embms_set_sntp_time_req_msg_v01), embms_set_sntp_time_req_msg_data_v01},
  {sizeof(embms_set_sntp_time_resp_msg_v01), 0},
  {sizeof(embms_get_sib16_coverage_req_msg_v01), 0},
  {sizeof(embms_get_sib16_coverage_resp_msg_v01), embms_get_sib16_coverage_resp_msg_data_v01},
  {sizeof(embms_unsol_sib16_coverage_ind_msg_v01), embms_unsol_sib16_coverage_ind_msg_data_v01},
  {sizeof(embms_get_utc_time_req_msg_v01), embms_get_utc_time_req_msg_data_v01},
  {sizeof(embms_get_utc_time_resp_msg_v01), embms_get_utc_time_resp_msg_data_v01},
  {sizeof(embms_get_e911_state_req_msg_v01), embms_get_e911_state_req_msg_data_v01},
  {sizeof(embms_get_e911_state_resp_msg_v01), embms_get_e911_state_resp_msg_data_v01},
  {sizeof(embms_unsol_e911_state_ind_msg_v01), embms_unsol_e911_state_ind_msg_data_v01},
  {sizeof(embms_update_content_desc_req_msg_v01), embms_update_content_desc_req_msg_data_v01},
  {sizeof(embms_update_content_desc_resp_msg_v01), embms_update_content_desc_resp_msg_data_v01},
  {sizeof(embms_unsol_content_desc_update_per_obj_ind_msg_v01), embms_unsol_content_desc_update_per_obj_ind_msg_data_v01},
  {sizeof(embms_get_sib_plmn_req_msg_v01), embms_get_sib_plmn_req_msg_data_v01},
  {sizeof(embms_get_sib_plmn_resp_msg_v01), embms_get_sib_plmn_resp_msg_data_v01},
  {sizeof(embms_unsol_embms_status_ind_msg_v01), embms_unsol_embms_status_ind_msg_data_v01},
  {sizeof(embms_get_embms_status_req_msg_v01), embms_get_embms_status_req_msg_data_v01},
  {sizeof(embms_get_embms_status_resp_msg_v01), embms_get_embms_status_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object embms_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *embms_qmi_idl_type_table_object_referenced_tables_v01[] =
{&embms_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object embms_qmi_idl_type_table_object_v01 = {
  sizeof(embms_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(embms_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  embms_type_table_v01,
  embms_message_table_v01,
  embms_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry embms_service_command_messages_v01[] = {
  {QMI_EMBMS_ENABLE_EMBMS_REQ_V01, QMI_IDL_TYPE16(0, 0), 7},
  {QMI_EMBMS_DISABLE_EMBMS_REQ_V01, QMI_IDL_TYPE16(0, 2), 11},
  {QMI_EMBMS_ACTIVATE_TMGI_REQ_V01, QMI_IDL_TYPE16(0, 4), 420},
  {QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01, QMI_IDL_TYPE16(0, 6), 21},
  {QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01, QMI_IDL_TYPE16(0, 10), 11},
  {QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01, QMI_IDL_TYPE16(0, 12), 11},
  {QMI_EMBMS_ENABLE_RSSI_REQ_V01, QMI_IDL_TYPE16(0, 14), 0},
  {QMI_EMBMS_DISABLE_RSSI_REQ_V01, QMI_IDL_TYPE16(0, 16), 0},
  {QMI_EMBMS_GET_COVERAGE_STATE_REQ_V01, QMI_IDL_TYPE16(0, 18), 7},
  {QMI_EMBMS_GET_RSSI_REQ_V01, QMI_IDL_TYPE16(0, 20), 7},
  {QMI_EMBMS_GET_EMBMS_SERVICE_STATE_REQ_V01, QMI_IDL_TYPE16(0, 22), 0},
  {QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01, QMI_IDL_TYPE16(0, 8), 430},
  {QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_REQ_V01, QMI_IDL_TYPE16(0, 33), 1036},
  {QMI_EMBMS_DELIVER_LOG_PACKET_REQ_V01, QMI_IDL_TYPE16(0, 35), 2067},
  {QMI_EMBMS_SET_SNTP_TIME_REQ_V01, QMI_IDL_TYPE16(0, 37), 26},
  {QMI_EMBMS_GET_SIB16_COVERAGE_REQ_V01, QMI_IDL_TYPE16(0, 39), 0},
  {QMI_EMBMS_GET_UTC_TIME_REQ_V01, QMI_IDL_TYPE16(0, 42), 7},
  {QMI_EMBMS_GET_E911_STATE_REQ_V01, QMI_IDL_TYPE16(0, 44), 7},
  {QMI_EMBMS_UPDATE_CONTENT_DESC_REQ_V01, QMI_IDL_TYPE16(0, 47), 537},
  {QMI_EMBMS_GET_SIB_PLMN_REQ_V01, QMI_IDL_TYPE16(0, 50), 7},
  {QMI_EMBMS_GET_EMBMS_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 53), 7}
};

static const qmi_idl_service_message_table_entry embms_service_response_messages_v01[] = {
  {QMI_EMBMS_ENABLE_EMBMS_RESP_V01, QMI_IDL_TYPE16(0, 1), 92},
  {QMI_EMBMS_DISABLE_EMBMS_RESP_V01, QMI_IDL_TYPE16(0, 3), 18},
  {QMI_EMBMS_ACTIVATE_TMGI_RESP_V01, QMI_IDL_TYPE16(0, 5), 28},
  {QMI_EMBMS_DEACTIVATE_TMGI_RESP_V01, QMI_IDL_TYPE16(0, 7), 28},
  {QMI_EMBMS_GET_AVAILABLE_TMGI_RESP_V01, QMI_IDL_TYPE16(0, 11), 466},
  {QMI_EMBMS_GET_ACTIVE_TMGI_RESP_V01, QMI_IDL_TYPE16(0, 13), 466},
  {QMI_EMBMS_ENABLE_RSSI_RESP_V01, QMI_IDL_TYPE16(0, 15), 0},
  {QMI_EMBMS_DISABLE_RSSI_RESP_V01, QMI_IDL_TYPE16(0, 17), 0},
  {QMI_EMBMS_GET_COVERAGE_STATE_RESP_V01, QMI_IDL_TYPE16(0, 19), 21},
  {QMI_EMBMS_GET_RSSI_RESP_V01, QMI_IDL_TYPE16(0, 21), 1955},
  {QMI_EMBMS_GET_EMBMS_SERVICE_STATE_RESP_V01, QMI_IDL_TYPE16(0, 23), 14},
  {QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_RESP_V01, QMI_IDL_TYPE16(0, 9), 41},
  {QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_RESP_V01, QMI_IDL_TYPE16(0, 34), 1036},
  {QMI_EMBMS_DELIVER_LOG_PACKET_RESP_V01, QMI_IDL_TYPE16(0, 36), 7},
  {QMI_EMBMS_SET_SNTP_TIME_RESP_V01, QMI_IDL_TYPE16(0, 38), 0},
  {QMI_EMBMS_GET_SIB16_COVERAGE_RESP_V01, QMI_IDL_TYPE16(0, 40), 4},
  {QMI_EMBMS_GET_UTC_TIME_RESP_V01, QMI_IDL_TYPE16(0, 43), 37},
  {QMI_EMBMS_GET_E911_STATE_RESP_V01, QMI_IDL_TYPE16(0, 45), 21},
  {QMI_EMBMS_UPDATE_CONTENT_DESC_RESP_V01, QMI_IDL_TYPE16(0, 48), 14},
  {QMI_EMBMS_GET_SIB_PLMN_RESP_V01, QMI_IDL_TYPE16(0, 51), 139},
  {QMI_EMBMS_GET_EMBMS_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 54), 11}
};

static const qmi_idl_service_message_table_entry embms_service_indication_messages_v01[] = {
  {QMI_EMBMS_UNSOL_EMBMS_SERVICE_STATE_IND_V01, QMI_IDL_TYPE16(0, 24), 74},
  {QMI_EMBMS_ACTIVE_TMGI_IND_V01, QMI_IDL_TYPE16(0, 25), 459},
  {QMI_EMBMS_UNSOL_BROADCAST_COVERAGE_IND_V01, QMI_IDL_TYPE16(0, 26), 14},
  {QMI_EMBMS_UNSOL_RSSI_IND_V01, QMI_IDL_TYPE16(0, 27), 7},
  {QMI_EMBMS_AVAILABLE_TMGI_IND_V01, QMI_IDL_TYPE16(0, 28), 459},
  {QMI_EMBMS_OOS_WARNING_IND_V01, QMI_IDL_TYPE16(0, 29), 466},
  {QMI_EMBMS_CELL_INFO_CHANGED_IND_V01, QMI_IDL_TYPE16(0, 30), 26},
  {QMI_EMBMS_RADIO_STATE_IND_V01, QMI_IDL_TYPE16(0, 31), 14},
  {QMI_EMBMS_SAI_IND_V01, QMI_IDL_TYPE16(0, 32), 2616},
  {QMI_EMBMS_UNSOL_SIB16_COVERAGE_IND_V01, QMI_IDL_TYPE16(0, 41), 4},
  {QMI_EMBMS_E911_STATE_IND_V01, QMI_IDL_TYPE16(0, 46), 14},
  {QMI_EMBMS_UNSOL_CONTENT_DESC_UPDATE_PER_OBJ_IND_V01, QMI_IDL_TYPE16(0, 49), 31},
  {QMI_EMBMS_UNSOL_EMBMS_STATUS_IND_V01, QMI_IDL_TYPE16(0, 52), 4}
};

/*Service Object*/
struct qmi_idl_service_object embms_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x0002,
  2616,
  { sizeof(embms_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(embms_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(embms_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { embms_service_command_messages_v01, embms_service_response_messages_v01, embms_service_indication_messages_v01},
  &embms_qmi_idl_type_table_object_v01,
  0x02,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type embms_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( EMBMS_V01_IDL_MAJOR_VERS != idl_maj_version || EMBMS_V01_IDL_MINOR_VERS != idl_min_version
       || EMBMS_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&embms_qmi_idl_service_object_v01;
}

