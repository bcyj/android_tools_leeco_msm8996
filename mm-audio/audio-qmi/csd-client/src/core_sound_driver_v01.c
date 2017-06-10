/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        C O R E _ S O U N D _ D R I V E R _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the csd service Data structures.

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.7
   It was generated on: Wed Jun 25 2014 (Spin 0)
   From IDL File: core_sound_driver_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "core_sound_driver_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t qmi_csd_vs_open_full_control_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(qmi_csd_vs_open_full_control_type_v01, session_name),
  SESSION_NAME_MAX_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(qmi_csd_vs_open_full_control_type_v01, direction),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(qmi_csd_vs_open_full_control_type_v01, enc_media_type),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(qmi_csd_vs_open_full_control_type_v01, dec_media_type),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(qmi_csd_vs_open_full_control_type_v01, network_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_open_full_control_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(qmi_csd_vc_open_full_control_type_v01, session_name),
  SESSION_NAME_MAX_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(qmi_csd_vc_open_full_control_type_v01, direction),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(qmi_csd_vc_open_full_control_type_v01, network_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_open_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(qmi_csd_vm_open_type_v01, session_name),
  SESSION_NAME_MAX_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_attrib_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_attrib_v01, sample_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_attrib_v01, bits_per_sample),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_entry_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_entry_v01, dev_id),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_dev_entry_v01, dev_attrib),
  QMI_IDL_TYPE88(0, 3),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_enable_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_dev_enable_v01, devs),
  QMI_CSD_DEVICE_MAX_ENTRIES_V01,
  QMI_IDL_OFFSET8(qmi_csd_dev_enable_v01, devs) - QMI_IDL_OFFSET8(qmi_csd_dev_enable_v01, devs_len),
  QMI_IDL_TYPE88(0, 4),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_disable_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_disable_v01, dev_ids),
  QMI_CSD_DEVICE_MAX_ENTRIES_V01,
  QMI_IDL_OFFSET8(qmi_csd_dev_disable_v01, dev_ids) - QMI_IDL_OFFSET8(qmi_csd_dev_disable_v01, dev_ids_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_afe_loopback_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_afe_loopback_v01, tx_dev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_afe_loopback_v01, rx_dev_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_afe_loopback_v01, enable),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_afe_loopback_v01, afe_mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_connect_device_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_connect_device_v01, connect_flag),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_connect_device_v01, source_dev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_connect_device_v01, sink_dev_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_anc_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_anc_v01, rx_dev_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_anc_v01, enable),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_companding_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_companding_v01, rx_dev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_companding_v01, qmi_csd_comp_options),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_caps_entry_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_caps_entry_v01, dev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_caps_entry_v01, sr_bitmask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_caps_entry_v01, bps_bitmask),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_caps_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_caps_v01, num_devs),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_dev_caps_v01, qmi_csd_dev_caps_list),
  ((QMI_CSD_MAX_NUM_DEV_SUPPORTED_V01) & 0xFF), ((QMI_CSD_MAX_NUM_DEV_SUPPORTED_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_csd_dev_caps_v01, qmi_csd_dev_caps_list) - QMI_IDL_OFFSET8(qmi_csd_dev_caps_v01, qmi_csd_dev_caps_list_len),
  QMI_IDL_TYPE88(0, 11),
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_dtmf_ctrl_data_v01[] = {
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_dtmf_ctrl_v01, dtmf_duration_in_ms),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_dtmf_ctrl_v01, dtmf_high_freq),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_dtmf_ctrl_v01, dtmf_low_freq),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_dtmf_ctrl_v01, dtmf_gain),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_dtmf_ctrl_v01, dev_ids),
  ((QMI_CSD_MAX_NUM_DEV_SUPPORTED_V01) & 0xFF), ((QMI_CSD_MAX_NUM_DEV_SUPPORTED_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_csd_dev_dtmf_ctrl_v01, dev_ids) - QMI_IDL_OFFSET8(qmi_csd_dev_dtmf_ctrl_v01, dev_ids_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_sidetone_ctrl_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_sidetone_ctrl_v01, sidetone_ctrl),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_sidetone_ctrl_v01, rx_dev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_sidetone_ctrl_v01, tx_dev_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_rt_proxy_port_cfg_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_rt_proxy_port_cfg_v01, cfg_hdr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_rt_proxy_port_cfg_v01, num_channels),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_rt_proxy_port_cfg_v01, interleaved),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_rt_proxy_port_cfg_v01, frame_size),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_rt_proxy_port_cfg_v01, jitter_allowance),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_rt_proxy_port_cfg_v01, low_water_mark),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_rt_proxy_port_cfg_v01, high_water_mark),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_set_device_config_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_device_config_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_device_config_v01, tx_dev_num),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_device_config_v01, rx_dev_num),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_device_config_v01, tx_dev_sr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_device_config_v01, rx_dev_sr),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_set_rx_volume_index_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_rx_volume_index_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_rx_volume_index_v01, vol_index),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_set_number_of_volume_steps_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_number_of_volume_steps_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_number_of_volume_steps_v01, value),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_set_rx_volume_step_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_rx_volume_step_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_rx_volume_step_v01, vol_step),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_rx_volume_step_v01, ramp_duration),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_set_mute_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_mute_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_mute_v01, direction),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_mute_v01, mute_flag),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_tx_dtmf_detect_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_tx_dtmf_detect_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_tx_dtmf_detect_v01, enable),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_set_ui_property_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_ui_property_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_ui_property_v01, module_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_ui_property_v01, param_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_ui_property_v01, param_data),
  ((MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01) & 0xFF), ((MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_ui_property_v01, param_data) - QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_set_ui_property_v01, param_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_get_ui_property_req_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_req_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_req_v01, module_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_req_v01, param_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_ioctl_get_ui_property_resp_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_resp_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_resp_v01, module_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_resp_v01, param_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_resp_v01, param_data),
  ((MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01) & 0xFF), ((MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_resp_v01, param_data) - QMI_IDL_OFFSET8(qmi_csd_vc_ioctl_get_ui_property_resp_v01, param_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_media_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_media_type_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_media_type_v01, rx_media_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_media_type_v01, tx_media_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_mute_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_mute_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_mute_v01, direction),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_mute_v01, mute_flag),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_encoder_dtx_mode_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_encoder_dtx_mode_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_encoder_dtx_mode_v01, enable),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_dec_timewarp_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dec_timewarp_v01, cmd_token),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dec_timewarp_v01, enable_time_warp),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dec_timewarp_v01, factor),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dec_timewarp_v01, enable_phase_match),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dec_timewarp_v01, run_length),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dec_timewarp_v01, phase_offset),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_enc_minmax_rate_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_enc_minmax_rate_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_enc_minmax_rate_v01, min_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_enc_minmax_rate_v01, max_rate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_enc_rate_mod_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_enc_rate_mod_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_enc_rate_mod_v01, mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_v01, rate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_voc_4gvnb_set_rate_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_4gvnb_set_rate_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_4gvnb_set_rate_v01, rate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_voc_4gvwb_set_rate_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_4gvwb_set_rate_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_4gvwb_set_rate_v01, rate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_voc_amr_set_enc_rate_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_amr_set_enc_rate_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_amr_set_enc_rate_v01, mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_v01, mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_dtmf_generation_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dtmf_generation_v01, cmd_token),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dtmf_generation_v01, direction),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dtmf_generation_v01, mix_flag),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dtmf_generation_v01, tone_1),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dtmf_generation_v01, tone_2),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dtmf_generation_v01, gain),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_dtmf_generation_v01, duration),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_rx_dtmf_detection_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_rx_dtmf_detection_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_rx_dtmf_detection_v01, enable),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_set_ui_property_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_ui_property_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_ui_property_v01, module_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_ui_property_v01, param_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_ui_property_v01, param_data),
  ((MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01) & 0xFF), ((MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_ui_property_v01, param_data) - QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_set_ui_property_v01, param_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_get_ui_property_req_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_req_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_req_v01, module_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_req_v01, param_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vs_ioctl_get_ui_property_resp_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_resp_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_resp_v01, module_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_resp_v01, param_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_resp_v01, param_data),
  ((MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01) & 0xFF), ((MAX_UI_CONTROLLED_PARAM_SIZE_IN_BYTE_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_resp_v01, param_data) - QMI_IDL_OFFSET8(qmi_csd_vs_ioctl_get_ui_property_resp_v01, param_data_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_ioctl_attach_stream_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_attach_stream_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_attach_stream_v01, stream_handle),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_ioctl_detach_stream_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_detach_stream_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_detach_stream_v01, stream_handle),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_ioctl_attach_context_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_attach_context_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_attach_context_v01, context_handle),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_ioctl_detach_context_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_detach_context_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_detach_context_v01, context_handle),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_ioctl_set_network_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_network_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_network_v01, network_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_ioctl_set_voice_timing_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_voice_timing_v01, cmd_token),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_voice_timing_v01, mode),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_voice_timing_v01, enc_offset),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_voice_timing_v01, dec_req_offset),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_voice_timing_v01, dec_offset),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_ioctl_set_tty_mode_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_tty_mode_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_tty_mode_v01, mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vm_ioctl_set_widevoice_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_widevoice_v01, cmd_token),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vm_ioctl_set_widevoice_v01, enable),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_as_open_t_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_open_t_v01, op_code),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_open_t_v01, data_mode),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_open_t_v01, format_type_rx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_open_t_v01, format_type_tx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_open_t_v01, open_mask),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_open_t_v01, session_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_open_t_v01, frames_per_buf),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_ac_open_t_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_open_t_v01, ac_category),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_open_t_v01, ac_mode),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_open_t_v01, dev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_open_t_v01, sample_rate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_as_ts_t_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_ts_t_v01, ts_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_ts_t_v01, ts_high),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_ts_t_v01, ts_low),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_as_stream_fmt_rx_wmav9_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, tag),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, channels),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, sample_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, byte_per_second),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, block_align),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, valid_bit_per_sample),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, channel_mask),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, encode_opt),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, drc_peak_ref),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, drc_peak_target),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, drc_average_ref),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, drc_average_target),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, version_num),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav9_v01, virtual_pkt_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_as_stream_fmt_rx_wmav10_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, tag),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, channels),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, sample_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, byte_per_second),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, block_align),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, valid_bit_per_sample),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, channel_mask),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, encode_opt),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, adv_encode_opt),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, adv_encode_opt2),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, drc_peak_ref),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, drc_peak_target),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, drc_average_ref),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, drc_average_target),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, version_num),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_wmav10_v01, virtual_pkt_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_as_stream_fmt_rx_aac_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, sample_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, channels),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, format),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, aud_obj_type),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, ep_cfg),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, section_DRF),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, scale_factor_DRF),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, spectral_DRF),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, sbr_on_flag),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, sbr_ps_flag),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_aac_v01, bit_rate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_as_stream_fmt_rx_flac_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, strm_info_present),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, min_blk_size),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, max_blk_size),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, channels),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, sample_size),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, sample_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, min_frame_size),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, max_frame_size),

  QMI_IDL_FLAGS_IS_ARRAY |QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_stream_fmt_rx_flac_v01, md5_sum),
  8,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_aud_pp_vol_multi_ch_gain_entry_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_vol_multi_ch_gain_entry_v01, ch_type),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_vol_multi_ch_gain_entry_v01, gain_idx),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_aud_pp_eq_subband_t_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_eq_subband_t_v01, band_idx),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_eq_subband_t_v01, filter_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_eq_subband_t_v01, center_freq_in_hz),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_eq_subband_t_v01, filter_gain),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_eq_subband_t_v01, lq_factor),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_aud_pp_qcpr_config_t_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_qcpr_config_t_v01, preset),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_qcpr_config_t_v01, strength),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_aud_pp_spa_config_t_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_spa_config_t_v01, sample_interval),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_aud_pp_spa_config_t_v01, sample_points),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_dev_restart_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_restart_v01, tx_dev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_restart_v01, rx_dev_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_dev_restart_v01, sample_rate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_cal_feature_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_cal_feature_v01, feature_id),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_cal_feature_v01, feature_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qmi_csd_vc_set_cal_feature_id_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_v01, cmd_token),

  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_v01, cal_feature_id),
  QMI_IDL_TYPE88(0, 61),
  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
/*
 * qmi_csd_query_driver_version_req_msg is empty
 * static const uint8_t qmi_csd_query_driver_version_req_msg_data_v01[] = {
 * };
 */

static const uint8_t qmi_csd_query_driver_version_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_query_driver_version_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_query_driver_version_resp_msg_v01, csd_version) - QMI_IDL_OFFSET8(qmi_csd_query_driver_version_resp_msg_v01, csd_version_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_query_driver_version_resp_msg_v01, csd_version)
};

/*
 * qmi_csd_init_req_msg is empty
 * static const uint8_t qmi_csd_init_req_msg_data_v01[] = {
 * };
 */

static const uint8_t qmi_csd_init_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_init_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_init_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_init_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_init_resp_msg_v01, qmi_csd_status_code)
};

/*
 * qmi_csd_deinit_req_msg is empty
 * static const uint8_t qmi_csd_deinit_req_msg_data_v01[] = {
 * };
 */

static const uint8_t qmi_csd_deinit_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_deinit_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_deinit_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_deinit_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_deinit_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_open_passive_control_voice_stream_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(qmi_csd_open_passive_control_voice_stream_req_msg_v01, session_name),
  SESSION_NAME_MAX_LEN_V01
};

static const uint8_t qmi_csd_open_passive_control_voice_stream_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_passive_control_voice_stream_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_passive_control_voice_stream_resp_msg_v01, open_status) - QMI_IDL_OFFSET8(qmi_csd_open_passive_control_voice_stream_resp_msg_v01, open_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_passive_control_voice_stream_resp_msg_v01, open_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_passive_control_voice_stream_resp_msg_v01, qmi_csd_vs_passive_control_handle) - QMI_IDL_OFFSET8(qmi_csd_open_passive_control_voice_stream_resp_msg_v01, qmi_csd_vs_passive_control_handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_passive_control_voice_stream_resp_msg_v01, qmi_csd_vs_passive_control_handle)
};

static const uint8_t qmi_csd_open_full_control_voice_stream_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_full_control_voice_stream_req_msg_v01, qmi_csd_vs_open_payload),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t qmi_csd_open_full_control_voice_stream_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_full_control_voice_stream_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_full_control_voice_stream_resp_msg_v01, open_status) - QMI_IDL_OFFSET8(qmi_csd_open_full_control_voice_stream_resp_msg_v01, open_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_full_control_voice_stream_resp_msg_v01, open_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_full_control_voice_stream_resp_msg_v01, qmi_csd_vs_handle) - QMI_IDL_OFFSET8(qmi_csd_open_full_control_voice_stream_resp_msg_v01, qmi_csd_vs_handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_full_control_voice_stream_resp_msg_v01, qmi_csd_vs_handle)
};

static const uint8_t qmi_csd_open_voice_context_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_voice_context_req_msg_v01, qmi_csd_vc_open_payload),
  QMI_IDL_TYPE88(0, 1)
};

static const uint8_t qmi_csd_open_voice_context_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_voice_context_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_voice_context_resp_msg_v01, open_status) - QMI_IDL_OFFSET8(qmi_csd_open_voice_context_resp_msg_v01, open_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_voice_context_resp_msg_v01, open_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_voice_context_resp_msg_v01, qmi_csd_vc_handle) - QMI_IDL_OFFSET8(qmi_csd_open_voice_context_resp_msg_v01, qmi_csd_vc_handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_voice_context_resp_msg_v01, qmi_csd_vc_handle)
};

static const uint8_t qmi_csd_open_voice_manager_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_voice_manager_req_msg_v01, qmi_csd_vm_open_payload),
  QMI_IDL_TYPE88(0, 2)
};

static const uint8_t qmi_csd_open_voice_manager_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_voice_manager_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_voice_manager_resp_msg_v01, open_status) - QMI_IDL_OFFSET8(qmi_csd_open_voice_manager_resp_msg_v01, open_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_voice_manager_resp_msg_v01, open_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_voice_manager_resp_msg_v01, qmi_csd_vm_handle) - QMI_IDL_OFFSET8(qmi_csd_open_voice_manager_resp_msg_v01, qmi_csd_vm_handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_voice_manager_resp_msg_v01, qmi_csd_vm_handle)
};

/*
 * qmi_csd_open_device_control_req_msg is empty
 * static const uint8_t qmi_csd_open_device_control_req_msg_data_v01[] = {
 * };
 */

static const uint8_t qmi_csd_open_device_control_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_device_control_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_device_control_resp_msg_v01, open_status) - QMI_IDL_OFFSET8(qmi_csd_open_device_control_resp_msg_v01, open_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_device_control_resp_msg_v01, open_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_device_control_resp_msg_v01, qmi_csd_device_handle) - QMI_IDL_OFFSET8(qmi_csd_open_device_control_resp_msg_v01, qmi_csd_device_handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_device_control_resp_msg_v01, qmi_csd_device_handle)
};

static const uint8_t qmi_csd_close_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_close_req_msg_v01, handle)
};

static const uint8_t qmi_csd_close_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_close_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_close_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_close_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_close_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_close_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_close_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_close_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_enable_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_req_msg_v01, handle),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_req_msg_v01, qmi_csd_dev_enable_cmd_payload),
  QMI_IDL_TYPE88(0, 5),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_req_msg_v01, extn_devices) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_req_msg_v01, extn_devices_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_req_msg_v01, extn_devices),
  QMI_CSD_EXTENDED_DEVICES_MAX_ENTRIES_V01,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_req_msg_v01, extn_devices) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_req_msg_v01, extn_devices_len),
  QMI_IDL_TYPE88(0, 4)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_enable_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_enable_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_disable_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_req_msg_v01, handle),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_req_msg_v01, qmi_csd_dev_disable_cmd_payload),
  QMI_IDL_TYPE88(0, 6),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_req_msg_v01, extn_devices) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_req_msg_v01, extn_devices_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_req_msg_v01, extn_devices),
  QMI_CSD_EXTENDED_DEVICES_MAX_ENTRIES_V01,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_req_msg_v01, extn_devices) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_req_msg_v01, extn_devices_len)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_disable_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_disable_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_afe_loopback_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_afe_loopback_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_afe_loopback_req_msg_v01, qmi_csd_dev_afe_loopback_cmd_payload),
  QMI_IDL_TYPE88(0, 7)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_connect_device_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_connect_device_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_connect_device_req_msg_v01, qmi_csd_dev_connect_device_cmd_payload),
  QMI_IDL_TYPE88(0, 8)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_anc_control_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_anc_control_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_anc_control_req_msg_v01, qmi_csd_dev_anc_cmd_payload),
  QMI_IDL_TYPE88(0, 9)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_companding_control_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_companding_control_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_companding_control_req_msg_v01, qmi_csd_dev_companding_cmd_payload),
  QMI_IDL_TYPE88(0, 10)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_get_max_device_nums_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_max_device_nums_req_msg_v01, handle)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01, max_num_devices) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01, max_num_devices_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01, max_num_devices)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_get_dev_caps_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_dev_caps_req_msg_v01, handle)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01, qmi_csd_dev_caps_payload) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01, qmi_csd_dev_caps_payload_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01, qmi_csd_dev_caps_payload),
  QMI_IDL_TYPE88(0, 12)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_dtmf_control_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_dtmf_control_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_dtmf_control_req_msg_v01, qmi_csd_dev_dtmf_cmd_payload),
  QMI_IDL_TYPE88(0, 13)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_sidetone_control_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_sidetone_control_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_sidetone_control_req_msg_v01, qmi_csd_dev_sidetone_cmd_payload),
  QMI_IDL_TYPE88(0, 14)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_configure_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_configure_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_configure_req_msg_v01, dev_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_configure_req_msg_v01, qmi_csd_dev_rt_proxy_port_cfg_payload),
  QMI_IDL_TYPE88(0, 15)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_configure_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_configure_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_configure_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_configure_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_configure_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_v01, handle),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_v01, qmi_csd_vc_ioctl_set_device_config_payload),
  QMI_IDL_TYPE88(0, 16),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_v01, ec_ref_dev_num) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_v01, ec_ref_dev_num_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_v01, ec_ref_dev_num)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_enable_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_req_msg_v01, cmd_token)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_enable_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_disable_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_req_msg_v01, cmd_token)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_disable_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_rx_volume_index_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_req_msg_v01, qmi_csd_vc_ioctl_set_rx_volume_index_payload),
  QMI_IDL_TYPE88(0, 17)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_req_msg_v01, qmi_csd_vc_ioctl_set_number_of_volume_steps_payload),
  QMI_IDL_TYPE88(0, 18)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_rx_volume_step_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_req_msg_v01, qmi_csd_vc_ioctl_set_rx_volume_step_payload),
  QMI_IDL_TYPE88(0, 19)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_mute_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_req_msg_v01, handle),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_req_msg_v01, qmi_csd_vc_ioctl_set_mute_payload),
  QMI_IDL_TYPE88(0, 20),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_req_msg_v01, ramp_duration) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_req_msg_v01, ramp_duration_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_req_msg_v01, ramp_duration)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_req_msg_v01, qmi_csd_vc_ioctl_tx_dtmf_detect_payload),
  QMI_IDL_TYPE88(0, 21)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_tx_dtmf_detected_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_tx_dtmf_detected_ind_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_tx_dtmf_detected_ind_msg_v01, dtmf_low_freq),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_tx_dtmf_detected_ind_msg_v01, dtmf_high_freq)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_ui_property_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_req_msg_v01, qmi_csd_vc_ioctl_set_ui_property_payload),
  QMI_IDL_TYPE88(0, 22)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_v01, handle),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_v01, qmi_csd_vc_ioctl_get_ui_property_req_payload),
  QMI_IDL_TYPE88(0, 23),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_v01, param_size) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_v01, param_size_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_v01, param_size)
};

static const uint8_t qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, qmi_csd_vc_ioctl_get_ui_property_resp_payload) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, qmi_csd_vc_ioctl_get_ui_property_resp_payload_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01, qmi_csd_vc_ioctl_get_ui_property_resp_payload),
  QMI_IDL_TYPE88(0, 24)
};

static const uint8_t qmi_csd_ioctl_vc_state_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_state_ind_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vc_state_ind_msg_v01, qmi_csd_ioctl_vc_state_id)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_media_type_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_req_msg_v01, qmi_csd_vs_ioctl_set_media_type_payload),
  QMI_IDL_TYPE88(0, 25)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_mute_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_req_msg_v01, handle),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_req_msg_v01, qmi_csd_vs_ioctl_set_mute_payload),
  QMI_IDL_TYPE88(0, 26),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_req_msg_v01, ramp_duration) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_req_msg_v01, ramp_duration_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_req_msg_v01, ramp_duration)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_req_msg_v01, qmi_csd_vs_ioctl_set_encoder_dtx_mode_payload),
  QMI_IDL_TYPE88(0, 27)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_dec_timewarp_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_req_msg_v01, qmi_csd_vs_ioctl_set_dec_timewarp_payload),
  QMI_IDL_TYPE88(0, 28)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_req_msg_v01, qmi_csd_vs_ioctl_set_enc_minmax_rate_payload),
  QMI_IDL_TYPE88(0, 29)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_req_msg_v01, qmi_csd_vs_ioctl_set_enc_rate_mod_payload),
  QMI_IDL_TYPE88(0, 30)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_req_msg_v01, qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_payload),
  QMI_IDL_TYPE88(0, 31)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_req_msg_v01, qmi_csd_vs_ioctl_voc_4gvnb_set_rate_payload),
  QMI_IDL_TYPE88(0, 32)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_req_msg_v01, qmi_csd_vs_ioctl_voc_4gvwb_set_rate_payload),
  QMI_IDL_TYPE88(0, 33)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_req_msg_v01, qmi_csd_vs_ioctl_voc_amr_set_enc_rate_payload),
  QMI_IDL_TYPE88(0, 34)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_req_msg_v01, qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_payload),
  QMI_IDL_TYPE88(0, 35)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_dtmf_generation_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_req_msg_v01, qmi_csd_vs_ioctl_set_dtmf_generation_payload),
  QMI_IDL_TYPE88(0, 36)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_dtmf_generation_ended_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_dtmf_generation_ended_ind_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_dtmf_generation_ended_ind_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_dtmf_generation_ended_ind_msg_v01, direction)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_req_msg_v01, qmi_csd_vs_ioctl_set_rx_dtmf_detection_payload),
  QMI_IDL_TYPE88(0, 37)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_rx_dtmf_detected_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_rx_dtmf_detected_ind_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_rx_dtmf_detected_ind_msg_v01, dtmf_low_freq),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_rx_dtmf_detected_ind_msg_v01, dtmf_high_freq)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_ui_property_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_req_msg_v01, qmi_csd_vs_ioctl_set_ui_property_payload),
  QMI_IDL_TYPE88(0, 38)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_v01, handle),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_v01, qmi_csd_vs_ioctl_get_ui_property_req_payload),
  QMI_IDL_TYPE88(0, 39),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_v01, param_size) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_v01, param_size_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_v01, param_size)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, qmi_csd_vs_ioctl_get_ui_property_resp_payload) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, qmi_csd_vs_ioctl_get_ui_property_resp_payload_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01, qmi_csd_vs_ioctl_get_ui_property_resp_payload),
  QMI_IDL_TYPE88(0, 40)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_start_record_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, cmd_token),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, rx_tap_point),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, tx_tap_point),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, dev_id) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, dev_id_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, dev_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, mode) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, mode_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01, mode)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_start_record_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_stop_record_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_req_msg_v01, cmd_token)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_start_playback_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_req_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_req_msg_v01, dev_id)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_stop_playback_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_req_msg_v01, cmd_token)
};

static const uint8_t qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vs_state_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_state_ind_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_state_ind_msg_v01, qmi_csd_ioctl_vc_state_id)
};

static const uint8_t qmi_csd_ioctl_vs_enc_buffer_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_enc_buffer_ind_msg_v01, handle)
};

static const uint8_t qmi_csd_ioctl_vs_dec_buffer_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vs_dec_buffer_ind_msg_v01, handle)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_attach_stream_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_req_msg_v01, qmi_csd_vm_ioctl_attach_stream_payload),
  QMI_IDL_TYPE88(0, 41)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_detach_stream_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_req_msg_v01, qmi_csd_vm_ioctl_detach_stream_payload),
  QMI_IDL_TYPE88(0, 42)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_attach_context_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_req_msg_v01, qmi_csd_vm_ioctl_attach_context_payload),
  QMI_IDL_TYPE88(0, 43)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_detach_context_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_req_msg_v01, qmi_csd_vm_ioctl_detach_context_payload),
  QMI_IDL_TYPE88(0, 44)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_start_voice_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_req_msg_v01, cmd_token)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_standby_voice_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_req_msg_v01, cmd_token)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_stop_voice_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_req_msg_v01, cmd_token)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_set_network_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_req_msg_v01, qmi_csd_vm_ioctl_set_network_payload),
  QMI_IDL_TYPE88(0, 45)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_set_network_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_set_voice_timing_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_req_msg_v01, qmi_csd_vm_ioctl_set_voice_timing_payload),
  QMI_IDL_TYPE88(0, 46)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_set_tty_mode_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_req_msg_v01, qmi_csd_vm_ioctl_set_tty_mode_payload),
  QMI_IDL_TYPE88(0, 47)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_set_widevoice_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_req_msg_v01, qmi_csd_vm_ioctl_set_widevoice_payload),
  QMI_IDL_TYPE88(0, 48)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_open_audio_stream_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_audio_stream_req_msg_v01, qmi_csd_as_open_payload),
  QMI_IDL_TYPE88(0, 49)
};

static const uint8_t qmi_csd_open_audio_stream_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_audio_stream_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_audio_stream_resp_msg_v01, open_status) - QMI_IDL_OFFSET8(qmi_csd_open_audio_stream_resp_msg_v01, open_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_audio_stream_resp_msg_v01, open_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_audio_stream_resp_msg_v01, qmi_csd_as_handle) - QMI_IDL_OFFSET8(qmi_csd_open_audio_stream_resp_msg_v01, qmi_csd_as_handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_audio_stream_resp_msg_v01, qmi_csd_as_handle)
};

static const uint8_t qmi_csd_open_audio_context_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_audio_context_req_msg_v01, qmi_csd_ac_open_payload),
  QMI_IDL_TYPE88(0, 50)
};

static const uint8_t qmi_csd_open_audio_context_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_open_audio_context_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_audio_context_resp_msg_v01, open_status) - QMI_IDL_OFFSET8(qmi_csd_open_audio_context_resp_msg_v01, open_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_audio_context_resp_msg_v01, open_status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_open_audio_context_resp_msg_v01, qmi_csd_ac_handle) - QMI_IDL_OFFSET8(qmi_csd_open_audio_context_resp_msg_v01, qmi_csd_ac_handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_open_audio_context_resp_msg_v01, qmi_csd_ac_handle)
};

static const uint8_t qmi_csd_as_cmd_start_session_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_req_msg_v01, qmi_csd_as_ts_payload),
  QMI_IDL_TYPE88(0, 51)
};

static const uint8_t qmi_csd_as_cmd_start_session_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_start_session_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_stop_session_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_stop_session_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_stop_session_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_stop_session_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_stop_session_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_stop_session_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_stop_session_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_stop_session_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_stop_session_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_stop_session_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_flush_stream_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_flush_stream_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_flush_stream_tx_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_tx_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_flush_stream_tx_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_get_vol_levels_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_get_vol_levels_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, num_levels) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, num_levels_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01, num_levels)
};

static const uint8_t qmi_csd_as_cmd_get_dsp_clk_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_get_dsp_clk_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, qmi_csd_as_ts_payload) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, qmi_csd_as_ts_payload_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01, qmi_csd_as_ts_payload),
  QMI_IDL_TYPE88(0, 51)
};

static const uint8_t qmi_csd_as_cmd_get_rendered_time_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_get_rendered_time_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, qmi_csd_as_ts_payload) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, qmi_csd_as_ts_payload_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01, qmi_csd_as_ts_payload),
  QMI_IDL_TYPE88(0, 51)
};

static const uint8_t qmi_csd_as_cmd_get_session_id_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_get_session_id_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, qmi_csd_session_id) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, qmi_csd_session_id_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_session_id_resp_msg_v01, qmi_csd_session_id)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_v01, sample_rate),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_v01, channels),

  0x04,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_v01, bit_per_sample),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_v01, sign_flag),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x06,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_v01, interleave_flag)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01, channels),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01, bit_per_sample),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01, sample_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01, nBlockSize) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01, nBlockSize_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01, nBlockSize)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_midi_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_req_msg_v01, mode)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_req_msg_v01, qmi_csd_as_stream_fmt_rx_wmav9_payload),
  QMI_IDL_TYPE88(0, 52)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_req_msg_v01, qmi_csd_as_stream_fmt_rx_wmav10_payload),
  QMI_IDL_TYPE88(0, 53)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_aac_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_req_msg_v01, qmi_csd_as_stream_fmt_rx_aac_payload),
  QMI_IDL_TYPE88(0, 54)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_g711_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_req_msg_v01, sample_rate)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_flac_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_req_msg_v01, qmi_csd_as_stream_fmt_rx_flac_payload),
  QMI_IDL_TYPE88(0, 55)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01, sample_rate),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01, channels),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01, bit_rate),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01, min_bit_rate),

  0x06,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01, max_bit_rate),

  0x07,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01, bits_per_sample),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x08,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01, bit_stream_fmt)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_v01, channels),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_v01, amr_band_mode),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_v01, amr_dtx_mode),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_v01, amr_frame_fmt),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x06,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_v01, amr_lsf_idx)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_v01, sample_rate),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_v01, channels),

  0x04,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_v01, bit_per_sample),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_v01, sign_flag),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x06,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_v01, interleave_flag)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_v01, sample_rate),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_v01, channels),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_v01, format),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_v01, bit_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x06,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_v01, encoder_mode)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_g711_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_req_msg_v01, sample_rate)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_req_msg_v01, encoder_mode),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_req_msg_v01, dtx_mode)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_req_msg_v01, encoder_mode),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_req_msg_v01, dtx_mode)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_v01, min_frame_rate),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_v01, max_frame_rate),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_v01, reduce_rate_cmd),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_v01, rate_mod_cmd)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_v01, bit_rate_ctrl),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_v01, min_frame_rate),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_v01, max_frame_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_v01, dtx_mode)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_v01, bit_rate_ctrl),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_v01, min_frame_rate),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_v01, max_frame_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_v01, dtx_mode)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_v01, bit_rate_ctrl),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_v01, min_frame_rate),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_v01, max_frame_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_v01, dtx_mode)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01, sub_bands),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01, block_len),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01, channel_mode),

  0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01, alloc_method),

  0x06,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01, bit_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x07,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01, sample_rate)
};

static const uint8_t qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_eos_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_eos_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_set_stream_eos_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_eos_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_eos_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_eos_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_eos_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_eos_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_eos_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_eos_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_config_pp_vol_master_gain_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_req_msg_v01, master_gain_step)
};

static const uint8_t qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_config_pp_vol_stereo_gain_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_req_msg_v01, left_ch_gain_step),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_req_msg_v01, right_ch_gain_step)
};

static const uint8_t qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_config_pp_vol_multichannel_gain_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_req_msg_v01, multi_ch_gain),
  QMI_CSD_AUD_PP_MULTI_CH_NUM_V01,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_req_msg_v01, multi_ch_gain) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_req_msg_v01, multi_ch_gain_len),
  QMI_IDL_TYPE88(0, 56)
};

static const uint8_t qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_config_pp_vol_mute_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_req_msg_v01, mute)
};

static const uint8_t qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_config_pp_eq_enable_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_req_msg_v01, eq_bands) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_req_msg_v01, eq_bands_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_req_msg_v01, eq_bands),
  QMI_CSD_AUD_PP_EQ_SUB_BAND_MAX_NUM_V01,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_req_msg_v01, eq_bands) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_req_msg_v01, eq_bands_len),
  QMI_IDL_TYPE88(0, 57)
};

static const uint8_t qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_config_pp_qcpr_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_req_msg_v01, config) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_req_msg_v01, config_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_req_msg_v01, config),
  QMI_IDL_TYPE88(0, 58)
};

static const uint8_t qmi_csd_as_cmd_config_pp_qcpr_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_config_pp_spa_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_req_msg_v01, config) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_req_msg_v01, config_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_req_msg_v01, config),
  QMI_IDL_TYPE88(0, 59)
};

static const uint8_t qmi_csd_as_cmd_config_pp_spa_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_spa_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_config_pp_tsm_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_req_msg_v01, tsm_factor) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_req_msg_v01, tsm_factor_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_req_msg_v01, tsm_factor)
};

static const uint8_t qmi_csd_as_cmd_config_pp_tsm_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_get_spa_data_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_spa_data_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_get_spa_data_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_spa_data_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_spa_data_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_spa_data_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_spa_data_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_spa_data_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_spa_data_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_spa_data_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_set_dualmono_remap_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_req_msg_v01, remap_type)
};

static const uint8_t qmi_csd_as_cmd_set_dualmono_remap_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_adjust_session_clock_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_req_msg_v01, time_high),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_req_msg_v01, time_low)
};

static const uint8_t qmi_csd_as_cmd_adjust_session_clock_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, estimated_processing_time_low) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, estimated_processing_time_low_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, estimated_processing_time_low),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, estimated_processing_time_high) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, estimated_processing_time_high_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01, estimated_processing_time_high)
};

static const uint8_t qmi_csd_as_cmd_set_aac_sbr_ps_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_req_msg_v01, type)
};

static const uint8_t qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_dtmf_ctl_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_req_msg_v01, tone_1),

  0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_req_msg_v01, tone_2),

  0x04,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_req_msg_v01, gain_index),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_req_msg_v01, duration)
};

static const uint8_t qmi_csd_as_cmd_dtmf_ctl_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_set_stream_info_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_req_msg_v01, max_buf_size)
};

static const uint8_t qmi_csd_as_cmd_set_stream_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_set_stream_info_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_as_cmd_get_rendered_byte_offset_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, offset) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, offset_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01, offset)
};

static const uint8_t qmi_csd_as_cmd_get_midi_sequence_id_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_req_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, sequence_id) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, sequence_id_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01, sequence_id)
};

static const uint8_t qmi_csd_as_cmd_encoder_bit_rate_update_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_req_msg_v01, fmt_type),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_req_msg_v01, bit_rate)
};

static const uint8_t qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_req_msg_v01, channel_mask)
};

static const uint8_t qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_as_cmd_config_decoder_multi_channel_eac3_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_req_msg_v01, num_channels),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_req_msg_v01, channel_config),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_req_msg_v01, channel_mapping),
  8
};

static const uint8_t qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_as_evt_eos_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_evt_eos_ind_msg_v01, handle)
};

static const uint8_t qmi_csd_as_evt_sr_cm_change_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_evt_sr_cm_change_ind_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_evt_sr_cm_change_ind_msg_v01, sample_rate),

  0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_evt_sr_cm_change_ind_msg_v01, num_channels),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_evt_sr_cm_change_ind_msg_v01, channel_mapping),
  8
};

static const uint8_t qmi_csd_as_evt_spa_buf_ready_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_evt_spa_buf_ready_ind_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_as_evt_spa_buf_ready_ind_msg_v01, spa_data),
  ((QMI_CSD_SPA_DATA_BUF_SIZE_V01) & 0xFF), ((QMI_CSD_SPA_DATA_BUF_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_csd_as_evt_spa_buf_ready_ind_msg_v01, spa_data) - QMI_IDL_OFFSET8(qmi_csd_as_evt_spa_buf_ready_ind_msg_v01, spa_data_len)
};

static const uint8_t qmi_csd_ac_cmd_as_attach_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_req_msg_v01, ac_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_req_msg_v01, as_handles),
  QMI_CSD_MAX_NUM_AUDIO_STREAM_FOR_ONE_AC_V01,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_req_msg_v01, as_handles) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_req_msg_v01, as_handles_len)
};

static const uint8_t qmi_csd_ac_cmd_as_attach_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_attach_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_ac_cmd_as_detach_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_req_msg_v01, ac_handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_req_msg_v01, as_handles),
  QMI_CSD_MAX_NUM_AUDIO_STREAM_FOR_ONE_AC_V01,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_req_msg_v01, as_handles) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_req_msg_v01, as_handles_len)
};

static const uint8_t qmi_csd_ac_cmd_as_detach_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_as_detach_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_ac_cmd_set_device_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_req_msg_v01, ac_handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_req_msg_v01, sample_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_req_msg_v01, dev_id)
};

static const uint8_t qmi_csd_ac_cmd_set_device_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_set_device_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_ac_cmd_enable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_enable_req_msg_v01, ac_handle)
};

static const uint8_t qmi_csd_ac_cmd_enable_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_enable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_enable_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_enable_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_enable_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_enable_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_enable_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_enable_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_ac_cmd_disable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_disable_req_msg_v01, ac_handle)
};

static const uint8_t qmi_csd_ac_cmd_disable_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_disable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_disable_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_disable_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_disable_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_disable_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_disable_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_disable_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_vol_master_gain_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_req_msg_v01, master_gain_step)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_vol_stereo_gain_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_req_msg_v01, left_ch_gain_step),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_req_msg_v01, right_ch_gain_step)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_req_msg_v01, multi_ch_gain),
  QMI_CSD_AUD_PP_MULTI_CH_NUM_V01,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_req_msg_v01, multi_ch_gain) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_req_msg_v01, multi_ch_gain_len),
  QMI_IDL_TYPE88(0, 56)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_vol_mute_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_req_msg_v01, mute)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_v01, eq_bands) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_v01, eq_bands_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_v01, eq_bands),
  QMI_CSD_AUD_PP_EQ_SUB_BAND_MAX_NUM_V01,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_v01, eq_bands) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_v01, eq_bands_len),
  QMI_IDL_TYPE88(0, 57)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_qcpr_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_req_msg_v01, config) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_req_msg_v01, config_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_req_msg_v01, config),
  QMI_IDL_TYPE88(0, 58)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_spa_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_req_msg_v01, config) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_req_msg_v01, config_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_req_msg_v01, config),
  QMI_IDL_TYPE88(0, 59)
};

static const uint8_t qmi_csd_ac_cmd_config_pp_spa_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01, qmi_csd_status_code_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ac_cmd_get_spa_data_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_get_spa_data_req_msg_v01, handle)
};

static const uint8_t qmi_csd_ac_cmd_get_spa_data_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_get_spa_data_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_get_spa_data_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_get_spa_data_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_get_spa_data_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_get_spa_data_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_get_spa_data_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_get_spa_data_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_ac_cmd_config_multi_channel_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_req_msg_v01, num_channels),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
  QMI_IDL_FLAGS_IS_ARRAY |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_req_msg_v01, channel_mapping),
  8
};

static const uint8_t qmi_csd_ac_cmd_config_multi_channel_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01, qmi_csd_status_code),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01, handle_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01, handle)
};

static const uint8_t qmi_csd_ac_evt_spa_buf_ready_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_evt_spa_buf_ready_ind_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 |   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ac_evt_spa_buf_ready_ind_msg_v01, spa_data),
  ((QMI_CSD_SPA_DATA_BUF_SIZE_V01) & 0xFF), ((QMI_CSD_SPA_DATA_BUF_SIZE_V01) >> 8),
  QMI_IDL_OFFSET8(qmi_csd_ac_evt_spa_buf_ready_ind_msg_v01, spa_data) - QMI_IDL_OFFSET8(qmi_csd_ac_evt_spa_buf_ready_ind_msg_v01, spa_data_len)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_v01, handle),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_v01, tx_dev_id),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_v01, rx_dev_id),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_v01, ref_dev_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x05,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_v01, aanc_ctrl)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_aanc_control_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_aanc_control_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_pause_voice_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_req_msg_v01, cmd_token)
};

static const uint8_t qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_restart_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_restart_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_restart_req_msg_v01, qmi_csd_dev_restart_payload),
  QMI_IDL_TYPE88(0, 60)
};

static const uint8_t qmi_csd_ioctl_dev_cmd_restart_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_restart_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_restart_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_restart_resp_msg_v01, qmi_csd_status_code_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_ioctl_dev_cmd_restart_resp_msg_v01, qmi_csd_status_code)
};

static const uint8_t qmi_csd_vc_set_cal_feature_id_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_req_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_req_msg_v01, qmi_csd_vc_set_cal_feature_id_payload),
  QMI_IDL_TYPE88(0, 62)
};

static const uint8_t qmi_csd_vc_set_cal_feature_id_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, handle) - QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, handle_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, handle),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, cmd_token) - QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, cmd_token_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, cmd_token),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, qmi_csd_status_code) - QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, qmi_csd_status_code_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(qmi_csd_vc_set_cal_feature_id_resp_msg_v01, qmi_csd_status_code)
};

/* Type Table */
static const qmi_idl_type_table_entry  csd_type_table_v01[] = {
  {sizeof(qmi_csd_vs_open_full_control_type_v01), qmi_csd_vs_open_full_control_type_data_v01},
  {sizeof(qmi_csd_vc_open_full_control_type_v01), qmi_csd_vc_open_full_control_type_data_v01},
  {sizeof(qmi_csd_vm_open_type_v01), qmi_csd_vm_open_type_data_v01},
  {sizeof(qmi_csd_dev_attrib_v01), qmi_csd_dev_attrib_data_v01},
  {sizeof(qmi_csd_dev_entry_v01), qmi_csd_dev_entry_data_v01},
  {sizeof(qmi_csd_dev_enable_v01), qmi_csd_dev_enable_data_v01},
  {sizeof(qmi_csd_dev_disable_v01), qmi_csd_dev_disable_data_v01},
  {sizeof(qmi_csd_dev_afe_loopback_v01), qmi_csd_dev_afe_loopback_data_v01},
  {sizeof(qmi_csd_dev_connect_device_v01), qmi_csd_dev_connect_device_data_v01},
  {sizeof(qmi_csd_dev_anc_v01), qmi_csd_dev_anc_data_v01},
  {sizeof(qmi_csd_dev_companding_v01), qmi_csd_dev_companding_data_v01},
  {sizeof(qmi_csd_dev_caps_entry_v01), qmi_csd_dev_caps_entry_data_v01},
  {sizeof(qmi_csd_dev_caps_v01), qmi_csd_dev_caps_data_v01},
  {sizeof(qmi_csd_dev_dtmf_ctrl_v01), qmi_csd_dev_dtmf_ctrl_data_v01},
  {sizeof(qmi_csd_dev_sidetone_ctrl_v01), qmi_csd_dev_sidetone_ctrl_data_v01},
  {sizeof(qmi_csd_dev_rt_proxy_port_cfg_v01), qmi_csd_dev_rt_proxy_port_cfg_data_v01},
  {sizeof(qmi_csd_vc_ioctl_set_device_config_v01), qmi_csd_vc_ioctl_set_device_config_data_v01},
  {sizeof(qmi_csd_vc_ioctl_set_rx_volume_index_v01), qmi_csd_vc_ioctl_set_rx_volume_index_data_v01},
  {sizeof(qmi_csd_vc_ioctl_set_number_of_volume_steps_v01), qmi_csd_vc_ioctl_set_number_of_volume_steps_data_v01},
  {sizeof(qmi_csd_vc_ioctl_set_rx_volume_step_v01), qmi_csd_vc_ioctl_set_rx_volume_step_data_v01},
  {sizeof(qmi_csd_vc_ioctl_set_mute_v01), qmi_csd_vc_ioctl_set_mute_data_v01},
  {sizeof(qmi_csd_vc_ioctl_tx_dtmf_detect_v01), qmi_csd_vc_ioctl_tx_dtmf_detect_data_v01},
  {sizeof(qmi_csd_vc_ioctl_set_ui_property_v01), qmi_csd_vc_ioctl_set_ui_property_data_v01},
  {sizeof(qmi_csd_vc_ioctl_get_ui_property_req_v01), qmi_csd_vc_ioctl_get_ui_property_req_data_v01},
  {sizeof(qmi_csd_vc_ioctl_get_ui_property_resp_v01), qmi_csd_vc_ioctl_get_ui_property_resp_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_media_type_v01), qmi_csd_vs_ioctl_set_media_type_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_mute_v01), qmi_csd_vs_ioctl_set_mute_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_encoder_dtx_mode_v01), qmi_csd_vs_ioctl_set_encoder_dtx_mode_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_dec_timewarp_v01), qmi_csd_vs_ioctl_set_dec_timewarp_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_enc_minmax_rate_v01), qmi_csd_vs_ioctl_set_enc_minmax_rate_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_enc_rate_mod_v01), qmi_csd_vs_ioctl_set_enc_rate_mod_data_v01},
  {sizeof(qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_v01), qmi_csd_vs_ioctl_voc_qcelp13k_set_rate_data_v01},
  {sizeof(qmi_csd_vs_ioctl_voc_4gvnb_set_rate_v01), qmi_csd_vs_ioctl_voc_4gvnb_set_rate_data_v01},
  {sizeof(qmi_csd_vs_ioctl_voc_4gvwb_set_rate_v01), qmi_csd_vs_ioctl_voc_4gvwb_set_rate_data_v01},
  {sizeof(qmi_csd_vs_ioctl_voc_amr_set_enc_rate_v01), qmi_csd_vs_ioctl_voc_amr_set_enc_rate_data_v01},
  {sizeof(qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_v01), qmi_csd_vs_ioctl_voc_amrwb_set_enc_rate_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_dtmf_generation_v01), qmi_csd_vs_ioctl_set_dtmf_generation_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_rx_dtmf_detection_v01), qmi_csd_vs_ioctl_set_rx_dtmf_detection_data_v01},
  {sizeof(qmi_csd_vs_ioctl_set_ui_property_v01), qmi_csd_vs_ioctl_set_ui_property_data_v01},
  {sizeof(qmi_csd_vs_ioctl_get_ui_property_req_v01), qmi_csd_vs_ioctl_get_ui_property_req_data_v01},
  {sizeof(qmi_csd_vs_ioctl_get_ui_property_resp_v01), qmi_csd_vs_ioctl_get_ui_property_resp_data_v01},
  {sizeof(qmi_csd_vm_ioctl_attach_stream_v01), qmi_csd_vm_ioctl_attach_stream_data_v01},
  {sizeof(qmi_csd_vm_ioctl_detach_stream_v01), qmi_csd_vm_ioctl_detach_stream_data_v01},
  {sizeof(qmi_csd_vm_ioctl_attach_context_v01), qmi_csd_vm_ioctl_attach_context_data_v01},
  {sizeof(qmi_csd_vm_ioctl_detach_context_v01), qmi_csd_vm_ioctl_detach_context_data_v01},
  {sizeof(qmi_csd_vm_ioctl_set_network_v01), qmi_csd_vm_ioctl_set_network_data_v01},
  {sizeof(qmi_csd_vm_ioctl_set_voice_timing_v01), qmi_csd_vm_ioctl_set_voice_timing_data_v01},
  {sizeof(qmi_csd_vm_ioctl_set_tty_mode_v01), qmi_csd_vm_ioctl_set_tty_mode_data_v01},
  {sizeof(qmi_csd_vm_ioctl_set_widevoice_v01), qmi_csd_vm_ioctl_set_widevoice_data_v01},
  {sizeof(qmi_csd_as_open_t_v01), qmi_csd_as_open_t_data_v01},
  {sizeof(qmi_csd_ac_open_t_v01), qmi_csd_ac_open_t_data_v01},
  {sizeof(qmi_csd_as_ts_t_v01), qmi_csd_as_ts_t_data_v01},
  {sizeof(qmi_csd_as_stream_fmt_rx_wmav9_v01), qmi_csd_as_stream_fmt_rx_wmav9_data_v01},
  {sizeof(qmi_csd_as_stream_fmt_rx_wmav10_v01), qmi_csd_as_stream_fmt_rx_wmav10_data_v01},
  {sizeof(qmi_csd_as_stream_fmt_rx_aac_v01), qmi_csd_as_stream_fmt_rx_aac_data_v01},
  {sizeof(qmi_csd_as_stream_fmt_rx_flac_v01), qmi_csd_as_stream_fmt_rx_flac_data_v01},
  {sizeof(qmi_csd_aud_pp_vol_multi_ch_gain_entry_v01), qmi_csd_aud_pp_vol_multi_ch_gain_entry_data_v01},
  {sizeof(qmi_csd_aud_pp_eq_subband_t_v01), qmi_csd_aud_pp_eq_subband_t_data_v01},
  {sizeof(qmi_csd_aud_pp_qcpr_config_t_v01), qmi_csd_aud_pp_qcpr_config_t_data_v01},
  {sizeof(qmi_csd_aud_pp_spa_config_t_v01), qmi_csd_aud_pp_spa_config_t_data_v01},
  {sizeof(qmi_csd_dev_restart_v01), qmi_csd_dev_restart_data_v01},
  {sizeof(qmi_csd_vc_cal_feature_v01), qmi_csd_vc_cal_feature_data_v01},
  {sizeof(qmi_csd_vc_set_cal_feature_id_v01), qmi_csd_vc_set_cal_feature_id_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry csd_message_table_v01[] = {
  {sizeof(qmi_csd_query_driver_version_req_msg_v01), 0},
  {sizeof(qmi_csd_query_driver_version_resp_msg_v01), qmi_csd_query_driver_version_resp_msg_data_v01},
  {sizeof(qmi_csd_init_req_msg_v01), 0},
  {sizeof(qmi_csd_init_resp_msg_v01), qmi_csd_init_resp_msg_data_v01},
  {sizeof(qmi_csd_deinit_req_msg_v01), 0},
  {sizeof(qmi_csd_deinit_resp_msg_v01), qmi_csd_deinit_resp_msg_data_v01},
  {sizeof(qmi_csd_open_passive_control_voice_stream_req_msg_v01), qmi_csd_open_passive_control_voice_stream_req_msg_data_v01},
  {sizeof(qmi_csd_open_passive_control_voice_stream_resp_msg_v01), qmi_csd_open_passive_control_voice_stream_resp_msg_data_v01},
  {sizeof(qmi_csd_open_full_control_voice_stream_req_msg_v01), qmi_csd_open_full_control_voice_stream_req_msg_data_v01},
  {sizeof(qmi_csd_open_full_control_voice_stream_resp_msg_v01), qmi_csd_open_full_control_voice_stream_resp_msg_data_v01},
  {sizeof(qmi_csd_open_voice_context_req_msg_v01), qmi_csd_open_voice_context_req_msg_data_v01},
  {sizeof(qmi_csd_open_voice_context_resp_msg_v01), qmi_csd_open_voice_context_resp_msg_data_v01},
  {sizeof(qmi_csd_open_voice_manager_req_msg_v01), qmi_csd_open_voice_manager_req_msg_data_v01},
  {sizeof(qmi_csd_open_voice_manager_resp_msg_v01), qmi_csd_open_voice_manager_resp_msg_data_v01},
  {sizeof(qmi_csd_open_device_control_req_msg_v01), 0},
  {sizeof(qmi_csd_open_device_control_resp_msg_v01), qmi_csd_open_device_control_resp_msg_data_v01},
  {sizeof(qmi_csd_close_req_msg_v01), qmi_csd_close_req_msg_data_v01},
  {sizeof(qmi_csd_close_resp_msg_v01), qmi_csd_close_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_enable_req_msg_v01), qmi_csd_ioctl_dev_cmd_enable_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_enable_resp_msg_v01), qmi_csd_ioctl_dev_cmd_enable_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_disable_req_msg_v01), qmi_csd_ioctl_dev_cmd_disable_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_disable_resp_msg_v01), qmi_csd_ioctl_dev_cmd_disable_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_afe_loopback_req_msg_v01), qmi_csd_ioctl_dev_cmd_afe_loopback_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_v01), qmi_csd_ioctl_dev_cmd_afe_loopback_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_connect_device_req_msg_v01), qmi_csd_ioctl_dev_cmd_connect_device_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_v01), qmi_csd_ioctl_dev_cmd_connect_device_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_anc_control_req_msg_v01), qmi_csd_ioctl_dev_cmd_anc_control_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_v01), qmi_csd_ioctl_dev_cmd_anc_control_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_companding_control_req_msg_v01), qmi_csd_ioctl_dev_cmd_companding_control_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_v01), qmi_csd_ioctl_dev_cmd_companding_control_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_get_max_device_nums_req_msg_v01), qmi_csd_ioctl_dev_cmd_get_max_device_nums_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_v01), qmi_csd_ioctl_dev_cmd_get_max_device_nums_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_get_dev_caps_req_msg_v01), qmi_csd_ioctl_dev_cmd_get_dev_caps_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_v01), qmi_csd_ioctl_dev_cmd_get_dev_caps_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_dtmf_control_req_msg_v01), qmi_csd_ioctl_dev_cmd_dtmf_control_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_v01), qmi_csd_ioctl_dev_cmd_dtmf_control_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_sidetone_control_req_msg_v01), qmi_csd_ioctl_dev_cmd_sidetone_control_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_v01), qmi_csd_ioctl_dev_cmd_sidetone_control_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_configure_req_msg_v01), qmi_csd_ioctl_dev_cmd_configure_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_configure_resp_msg_v01), qmi_csd_ioctl_dev_cmd_configure_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_v01), qmi_csd_ioctl_vc_cmd_set_device_config_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_v01), qmi_csd_ioctl_vc_cmd_set_device_config_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_enable_req_msg_v01), qmi_csd_ioctl_vc_cmd_enable_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_enable_resp_msg_v01), qmi_csd_ioctl_vc_cmd_enable_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_disable_req_msg_v01), qmi_csd_ioctl_vc_cmd_disable_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_disable_resp_msg_v01), qmi_csd_ioctl_vc_cmd_disable_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_req_msg_v01), qmi_csd_ioctl_vc_cmd_set_rx_volume_index_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_v01), qmi_csd_ioctl_vc_cmd_set_rx_volume_index_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_req_msg_v01), qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_v01), qmi_csd_ioctl_vc_cmd_set_number_of_volume_steps_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_req_msg_v01), qmi_csd_ioctl_vc_cmd_set_rx_volume_step_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_v01), qmi_csd_ioctl_vc_cmd_set_rx_volume_step_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_mute_req_msg_v01), qmi_csd_ioctl_vc_cmd_set_mute_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_v01), qmi_csd_ioctl_vc_cmd_set_mute_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_req_msg_v01), qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_v01), qmi_csd_ioctl_vc_cmd_set_tx_dtmf_detection_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_tx_dtmf_detected_ind_msg_v01), qmi_csd_ioctl_vc_tx_dtmf_detected_ind_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_ui_property_req_msg_v01), qmi_csd_ioctl_vc_cmd_set_ui_property_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_v01), qmi_csd_ioctl_vc_cmd_set_ui_property_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_v01), qmi_csd_ioctl_vc_cmd_get_ui_property_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_v01), qmi_csd_ioctl_vc_cmd_get_ui_property_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vc_state_ind_msg_v01), qmi_csd_ioctl_vc_state_ind_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_media_type_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_media_type_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_media_type_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_mute_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_mute_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_mute_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_encoder_dtx_mode_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_dec_timewarp_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_dec_timewarp_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_enc_minmax_rate_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_enc_rate_modulation_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_req_msg_v01), qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_v01), qmi_csd_ioctl_vs_cmd_voc_qcelp13k_set_rate_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_req_msg_v01), qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_v01), qmi_csd_ioctl_vs_cmd_voc_4gvnb_set_rate_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_req_msg_v01), qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_v01), qmi_csd_ioctl_vs_cmd_voc_4gvwb_set_rate_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_req_msg_v01), qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_v01), qmi_csd_ioctl_vs_cmd_voc_amr_set_enc_rate_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_req_msg_v01), qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_v01), qmi_csd_ioctl_vs_cmd_voc_amrwb_set_enc_rate_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_dtmf_generation_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_dtmf_generation_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_dtmf_generation_ended_ind_msg_v01), qmi_csd_ioctl_vs_dtmf_generation_ended_ind_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_rx_dtmf_detection_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_rx_dtmf_detected_ind_msg_v01), qmi_csd_ioctl_vs_rx_dtmf_detected_ind_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_ui_property_req_msg_v01), qmi_csd_ioctl_vs_cmd_set_ui_property_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_v01), qmi_csd_ioctl_vs_cmd_set_ui_property_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_v01), qmi_csd_ioctl_vs_cmd_get_ui_property_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_v01), qmi_csd_ioctl_vs_cmd_get_ui_property_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_start_record_req_msg_v01), qmi_csd_ioctl_vs_cmd_start_record_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_start_record_resp_msg_v01), qmi_csd_ioctl_vs_cmd_start_record_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_stop_record_req_msg_v01), qmi_csd_ioctl_vs_cmd_stop_record_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_v01), qmi_csd_ioctl_vs_cmd_stop_record_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_start_playback_req_msg_v01), qmi_csd_ioctl_vs_cmd_start_playback_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_v01), qmi_csd_ioctl_vs_cmd_start_playback_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_stop_playback_req_msg_v01), qmi_csd_ioctl_vs_cmd_stop_playback_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_v01), qmi_csd_ioctl_vs_cmd_stop_playback_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_state_ind_msg_v01), qmi_csd_ioctl_vs_state_ind_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_enc_buffer_ind_msg_v01), qmi_csd_ioctl_vs_enc_buffer_ind_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vs_dec_buffer_ind_msg_v01), qmi_csd_ioctl_vs_dec_buffer_ind_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_attach_stream_req_msg_v01), qmi_csd_ioctl_vm_cmd_attach_stream_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_v01), qmi_csd_ioctl_vm_cmd_attach_stream_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_detach_stream_req_msg_v01), qmi_csd_ioctl_vm_cmd_detach_stream_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_v01), qmi_csd_ioctl_vm_cmd_detach_stream_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_attach_context_req_msg_v01), qmi_csd_ioctl_vm_cmd_attach_context_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_v01), qmi_csd_ioctl_vm_cmd_attach_context_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_detach_context_req_msg_v01), qmi_csd_ioctl_vm_cmd_detach_context_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_v01), qmi_csd_ioctl_vm_cmd_detach_context_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_start_voice_req_msg_v01), qmi_csd_ioctl_vm_cmd_start_voice_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_v01), qmi_csd_ioctl_vm_cmd_start_voice_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_standby_voice_req_msg_v01), qmi_csd_ioctl_vm_cmd_standby_voice_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_v01), qmi_csd_ioctl_vm_cmd_standby_voice_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_stop_voice_req_msg_v01), qmi_csd_ioctl_vm_cmd_stop_voice_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_v01), qmi_csd_ioctl_vm_cmd_stop_voice_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_set_network_req_msg_v01), qmi_csd_ioctl_vm_cmd_set_network_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_set_network_resp_msg_v01), qmi_csd_ioctl_vm_cmd_set_network_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_set_voice_timing_req_msg_v01), qmi_csd_ioctl_vm_cmd_set_voice_timing_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_v01), qmi_csd_ioctl_vm_cmd_set_voice_timing_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_set_tty_mode_req_msg_v01), qmi_csd_ioctl_vm_cmd_set_tty_mode_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_v01), qmi_csd_ioctl_vm_cmd_set_tty_mode_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_set_widevoice_req_msg_v01), qmi_csd_ioctl_vm_cmd_set_widevoice_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_v01), qmi_csd_ioctl_vm_cmd_set_widevoice_resp_msg_data_v01},
  {sizeof(qmi_csd_open_audio_stream_req_msg_v01), qmi_csd_open_audio_stream_req_msg_data_v01},
  {sizeof(qmi_csd_open_audio_stream_resp_msg_v01), qmi_csd_open_audio_stream_resp_msg_data_v01},
  {sizeof(qmi_csd_open_audio_context_req_msg_v01), qmi_csd_open_audio_context_req_msg_data_v01},
  {sizeof(qmi_csd_open_audio_context_resp_msg_v01), qmi_csd_open_audio_context_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_start_session_req_msg_v01), qmi_csd_as_cmd_start_session_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_start_session_resp_msg_v01), qmi_csd_as_cmd_start_session_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_stop_session_req_msg_v01), qmi_csd_as_cmd_stop_session_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_stop_session_resp_msg_v01), qmi_csd_as_cmd_stop_session_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_flush_stream_req_msg_v01), qmi_csd_as_cmd_flush_stream_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_flush_stream_resp_msg_v01), qmi_csd_as_cmd_flush_stream_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_flush_stream_tx_req_msg_v01), qmi_csd_as_cmd_flush_stream_tx_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_flush_stream_tx_resp_msg_v01), qmi_csd_as_cmd_flush_stream_tx_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_vol_levels_req_msg_v01), qmi_csd_as_cmd_get_vol_levels_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_vol_levels_resp_msg_v01), qmi_csd_as_cmd_get_vol_levels_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_dsp_clk_req_msg_v01), qmi_csd_as_cmd_get_dsp_clk_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_dsp_clk_resp_msg_v01), qmi_csd_as_cmd_get_dsp_clk_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_rendered_time_req_msg_v01), qmi_csd_as_cmd_get_rendered_time_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_rendered_time_resp_msg_v01), qmi_csd_as_cmd_get_rendered_time_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_session_id_req_msg_v01), qmi_csd_as_cmd_get_session_id_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_session_id_resp_msg_v01), qmi_csd_as_cmd_get_session_id_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_pcm_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_pcm_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_adpcm_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_midi_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_midi_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_midi_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_wmav9_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_wmav10_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_aac_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_aac_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_aac_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_g711_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_g711_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_g711_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_flac_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_flac_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_flac_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_vorbis_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_rx_amrwbplus_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_pcm_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_pcm_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_aac_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_aac_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_g711_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_g711_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_g711_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_amrnb_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_amrwb_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_qcelp13k_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_evrc_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_evrc_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_evrcb_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_evrcwb_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_sbc_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_v01), qmi_csd_as_cmd_set_stream_fmt_tx_sbc_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_eos_req_msg_v01), qmi_csd_as_cmd_set_stream_eos_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_eos_resp_msg_v01), qmi_csd_as_cmd_set_stream_eos_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_vol_master_gain_req_msg_v01), qmi_csd_as_cmd_config_pp_vol_master_gain_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_v01), qmi_csd_as_cmd_config_pp_vol_master_gain_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_vol_stereo_gain_req_msg_v01), qmi_csd_as_cmd_config_pp_vol_stereo_gain_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_v01), qmi_csd_as_cmd_config_pp_vol_stereo_gain_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_req_msg_v01), qmi_csd_as_cmd_config_pp_vol_multichannel_gain_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_v01), qmi_csd_as_cmd_config_pp_vol_multichannel_gain_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_vol_mute_req_msg_v01), qmi_csd_as_cmd_config_pp_vol_mute_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_v01), qmi_csd_as_cmd_config_pp_vol_mute_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_eq_enable_req_msg_v01), qmi_csd_as_cmd_config_pp_eq_enable_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_v01), qmi_csd_as_cmd_config_pp_eq_enable_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_qcpr_req_msg_v01), qmi_csd_as_cmd_config_pp_qcpr_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_qcpr_resp_msg_v01), qmi_csd_as_cmd_config_pp_qcpr_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_spa_req_msg_v01), qmi_csd_as_cmd_config_pp_spa_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_spa_resp_msg_v01), qmi_csd_as_cmd_config_pp_spa_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_tsm_req_msg_v01), qmi_csd_as_cmd_config_pp_tsm_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_pp_tsm_resp_msg_v01), qmi_csd_as_cmd_config_pp_tsm_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_spa_data_req_msg_v01), qmi_csd_as_cmd_get_spa_data_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_spa_data_resp_msg_v01), qmi_csd_as_cmd_get_spa_data_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_dualmono_remap_req_msg_v01), qmi_csd_as_cmd_set_dualmono_remap_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_dualmono_remap_resp_msg_v01), qmi_csd_as_cmd_set_dualmono_remap_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_adjust_session_clock_req_msg_v01), qmi_csd_as_cmd_adjust_session_clock_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_adjust_session_clock_resp_msg_v01), qmi_csd_as_cmd_adjust_session_clock_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_aac_sbr_ps_req_msg_v01), qmi_csd_as_cmd_set_aac_sbr_ps_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_v01), qmi_csd_as_cmd_set_aac_sbr_ps_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_dtmf_ctl_req_msg_v01), qmi_csd_as_cmd_dtmf_ctl_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_dtmf_ctl_resp_msg_v01), qmi_csd_as_cmd_dtmf_ctl_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_info_req_msg_v01), qmi_csd_as_cmd_set_stream_info_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_set_stream_info_resp_msg_v01), qmi_csd_as_cmd_set_stream_info_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_rendered_byte_offset_req_msg_v01), qmi_csd_as_cmd_get_rendered_byte_offset_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_v01), qmi_csd_as_cmd_get_rendered_byte_offset_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_midi_sequence_id_req_msg_v01), qmi_csd_as_cmd_get_midi_sequence_id_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_v01), qmi_csd_as_cmd_get_midi_sequence_id_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_encoder_bit_rate_update_req_msg_v01), qmi_csd_as_cmd_encoder_bit_rate_update_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_v01), qmi_csd_as_cmd_encoder_bit_rate_update_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_req_msg_v01), qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_v01), qmi_csd_as_cmd_config_decoder_multi_channel_wmav10_resp_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_req_msg_v01), qmi_csd_as_cmd_config_decoder_multi_channel_eac3_req_msg_data_v01},
  {sizeof(qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_v01), qmi_csd_as_cmd_config_decoder_multi_channel_eac3_resp_msg_data_v01},
  {sizeof(qmi_csd_as_evt_eos_ind_msg_v01), qmi_csd_as_evt_eos_ind_msg_data_v01},
  {sizeof(qmi_csd_as_evt_sr_cm_change_ind_msg_v01), qmi_csd_as_evt_sr_cm_change_ind_msg_data_v01},
  {sizeof(qmi_csd_as_evt_spa_buf_ready_ind_msg_v01), qmi_csd_as_evt_spa_buf_ready_ind_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_as_attach_req_msg_v01), qmi_csd_ac_cmd_as_attach_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_as_attach_resp_msg_v01), qmi_csd_ac_cmd_as_attach_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_as_detach_req_msg_v01), qmi_csd_ac_cmd_as_detach_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_as_detach_resp_msg_v01), qmi_csd_ac_cmd_as_detach_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_set_device_req_msg_v01), qmi_csd_ac_cmd_set_device_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_set_device_resp_msg_v01), qmi_csd_ac_cmd_set_device_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_enable_req_msg_v01), qmi_csd_ac_cmd_enable_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_enable_resp_msg_v01), qmi_csd_ac_cmd_enable_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_disable_req_msg_v01), qmi_csd_ac_cmd_disable_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_disable_resp_msg_v01), qmi_csd_ac_cmd_disable_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_vol_master_gain_req_msg_v01), qmi_csd_ac_cmd_config_pp_vol_master_gain_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_v01), qmi_csd_ac_cmd_config_pp_vol_master_gain_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_req_msg_v01), qmi_csd_ac_cmd_config_pp_vol_stereo_gain_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_v01), qmi_csd_ac_cmd_config_pp_vol_stereo_gain_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_req_msg_v01), qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_v01), qmi_csd_ac_cmd_config_pp_vol_multichannel_gain_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_vol_mute_req_msg_v01), qmi_csd_ac_cmd_config_pp_vol_mute_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_v01), qmi_csd_ac_cmd_config_pp_vol_mute_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_v01), qmi_csd_ac_cmd_config_pp_eq_enable_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_v01), qmi_csd_ac_cmd_config_pp_eq_enable_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_qcpr_req_msg_v01), qmi_csd_ac_cmd_config_pp_qcpr_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_v01), qmi_csd_ac_cmd_config_pp_qcpr_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_spa_req_msg_v01), qmi_csd_ac_cmd_config_pp_spa_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_pp_spa_resp_msg_v01), qmi_csd_ac_cmd_config_pp_spa_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_get_spa_data_req_msg_v01), qmi_csd_ac_cmd_get_spa_data_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_get_spa_data_resp_msg_v01), qmi_csd_ac_cmd_get_spa_data_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_multi_channel_req_msg_v01), qmi_csd_ac_cmd_config_multi_channel_req_msg_data_v01},
  {sizeof(qmi_csd_ac_cmd_config_multi_channel_resp_msg_v01), qmi_csd_ac_cmd_config_multi_channel_resp_msg_data_v01},
  {sizeof(qmi_csd_ac_evt_spa_buf_ready_ind_msg_v01), qmi_csd_ac_evt_spa_buf_ready_ind_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_v01), qmi_csd_ioctl_dev_cmd_aanc_control_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_aanc_control_resp_msg_v01), qmi_csd_ioctl_dev_cmd_aanc_control_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_pause_voice_req_msg_v01), qmi_csd_ioctl_vm_cmd_pause_voice_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_v01), qmi_csd_ioctl_vm_cmd_pause_voice_resp_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_restart_req_msg_v01), qmi_csd_ioctl_dev_cmd_restart_req_msg_data_v01},
  {sizeof(qmi_csd_ioctl_dev_cmd_restart_resp_msg_v01), qmi_csd_ioctl_dev_cmd_restart_resp_msg_data_v01},
  {sizeof(qmi_csd_vc_set_cal_feature_id_req_msg_v01), qmi_csd_vc_set_cal_feature_id_req_msg_data_v01},
  {sizeof(qmi_csd_vc_set_cal_feature_id_resp_msg_v01), qmi_csd_vc_set_cal_feature_id_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object csd_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *csd_qmi_idl_type_table_object_referenced_tables_v01[] =
{&csd_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object csd_qmi_idl_type_table_object_v01 = {
  sizeof(csd_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(csd_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  csd_type_table_v01,
  csd_message_table_v01,
  csd_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry csd_service_command_messages_v01[] = {
  {QMI_CSD_QUERY_DRIVER_VERSION_REQ_V01, QMI_IDL_TYPE16(0, 0), 0},
  {QMI_CSD_INIT_REQ_V01, QMI_IDL_TYPE16(0, 2), 0},
  {QMI_CSD_DEINIT_REQ_V01, QMI_IDL_TYPE16(0, 4), 0},
  {QMI_CSD_OPEN_PASSIVE_CONTROL_VOICE_STREAM_REQ_V01, QMI_IDL_TYPE16(0, 6), 258},
  {QMI_CSD_OPEN_FULL_CONTROL_VOICE_STREAM_REQ_V01, QMI_IDL_TYPE16(0, 8), 275},
  {QMI_CSD_OPEN_VOICE_CONTEXT_REQ_V01, QMI_IDL_TYPE16(0, 10), 267},
  {QMI_CSD_OPEN_VOICE_MANAGER_REQ_V01, QMI_IDL_TYPE16(0, 12), 259},
  {QMI_CSD_OPEN_DEVICE_CONTROL_REQ_V01, QMI_IDL_TYPE16(0, 14), 0},
  {QMI_CSD_CLOSE_REQ_V01, QMI_IDL_TYPE16(0, 16), 7},
  {QMI_CSD_IOCTL_DEV_CMD_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 18), 111},
  {QMI_CSD_IOCTL_DEV_CMD_DISABLE_REQ_V01, QMI_IDL_TYPE16(0, 20), 47},
  {QMI_CSD_IOCTL_DEV_CMD_AFE_LOOPBACK_REQ_V01, QMI_IDL_TYPE16(0, 22), 21},
  {QMI_CSD_IOCTL_DEV_CMD_ANC_CONTROL_REQ_V01, QMI_IDL_TYPE16(0, 26), 15},
  {QMI_CSD_IOCTL_DEV_CMD_COMPANDING_CONTROL_REQ_V01, QMI_IDL_TYPE16(0, 28), 18},
  {QMI_CSD_IOCTL_DEV_CMD_GET_MAX_DEVICE_NUMS_REQ_V01, QMI_IDL_TYPE16(0, 30), 7},
  {QMI_CSD_IOCTL_DEV_CMD_GET_DEV_CAPS_REQ_V01, QMI_IDL_TYPE16(0, 32), 7},
  {QMI_CSD_IOCTL_DEV_CMD_DTMF_CONTROL_REQ_V01, QMI_IDL_TYPE16(0, 34), 1050},
  {QMI_CSD_IOCTL_DEV_CMD_SIDETONE_CONTROL_REQ_V01, QMI_IDL_TYPE16(0, 36), 22},
  {QMI_CSD_IOCTL_DEV_CMD_CONFIGURE_REQ_V01, QMI_IDL_TYPE16(0, 38), 35},
  {QMI_CSD_IOCTL_VC_CMD_SET_DEVICE_CONFIG_REQ_V01, QMI_IDL_TYPE16(0, 40), 37},
  {QMI_CSD_IOCTL_VC_CMD_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 42), 14},
  {QMI_CSD_IOCTL_VC_CMD_DISABLE_REQ_V01, QMI_IDL_TYPE16(0, 44), 14},
  {QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_INDEX_REQ_V01, QMI_IDL_TYPE16(0, 46), 18},
  {QMI_CSD_IOCTL_VC_CMD_SET_MUTE_REQ_V01, QMI_IDL_TYPE16(0, 52), 27},
  {QMI_CSD_IOCTL_VC_CMD_SET_TX_DTMF_DETECTION_REQ_V01, QMI_IDL_TYPE16(0, 54), 18},
  {QMI_CSD_IOCTL_VC_CMD_SET_UI_PROPERTY_REQ_V01, QMI_IDL_TYPE16(0, 57), 2072},
  {QMI_CSD_IOCTL_VC_CMD_GET_UI_PROPERTY_REQ_V01, QMI_IDL_TYPE16(0, 59), 29},
  {QMI_CSD_IOCTL_VS_CMD_SET_MEDIA_TYPE_REQ_V01, QMI_IDL_TYPE16(0, 62), 22},
  {QMI_CSD_IOCTL_VS_CMD_SET_MUTE_REQ_V01, QMI_IDL_TYPE16(0, 64), 27},
  {QMI_CSD_IOCTL_VS_CMD_SET_ENCODER_DTX_MODE_REQ_V01, QMI_IDL_TYPE16(0, 66), 18},
  {QMI_CSD_IOCTL_VS_CMD_SET_DEC_TIMEWARP_REQ_V01, QMI_IDL_TYPE16(0, 68), 24},
  {QMI_CSD_IOCTL_VS_CMD_SET_ENC_MINMAX_RATE_REQ_V01, QMI_IDL_TYPE16(0, 70), 22},
  {QMI_CSD_IOCTL_VS_CMD_SET_ENC_RATE_MODULATION_REQ_V01, QMI_IDL_TYPE16(0, 72), 18},
  {QMI_CSD_IOCTL_VS_CMD_VOC_QCELP13K_SET_RATE_REQ_V01, QMI_IDL_TYPE16(0, 74), 18},
  {QMI_CSD_IOCTL_VS_CMD_VOC_4GVNB_SET_RATE_REQ_V01, QMI_IDL_TYPE16(0, 76), 18},
  {QMI_CSD_IOCTL_VS_CMD_VOC_4GVWB_SET_RATE_REQ_V01, QMI_IDL_TYPE16(0, 78), 18},
  {QMI_CSD_IOCTL_VS_CMD_VOC_AMR_SET_ENC_RATE_REQ_V01, QMI_IDL_TYPE16(0, 80), 18},
  {QMI_CSD_IOCTL_VS_CMD_VOC_AMRWB_SET_ENC_RATE_REQ_V01, QMI_IDL_TYPE16(0, 82), 18},
  {QMI_CSD_IOCTL_VS_CMD_SET_DTMF_GENERATION_REQ_V01, QMI_IDL_TYPE16(0, 84), 26},
  {QMI_CSD_IOCTL_VS_CMD_SET_RX_DTMF_DETECTION_REQ_V01, QMI_IDL_TYPE16(0, 87), 18},
  {QMI_CSD_IOCTL_VS_CMD_SET_UI_PROPERTY_REQ_V01, QMI_IDL_TYPE16(0, 90), 2072},
  {QMI_CSD_IOCTL_VS_CMD_GET_UI_PROPERTY_REQ_V01, QMI_IDL_TYPE16(0, 92), 29},
  {QMI_CSD_IOCTL_VS_CMD_START_RECORD_REQ_V01, QMI_IDL_TYPE16(0, 94), 42},
  {QMI_CSD_IOCTL_VS_CMD_STOP_RECORD_REQ_V01, QMI_IDL_TYPE16(0, 96), 14},
  {QMI_CSD_IOCTL_VM_CMD_ATTACH_STREAM_REQ_V01, QMI_IDL_TYPE16(0, 105), 18},
  {QMI_CSD_IOCTL_VM_CMD_DETACH_STREAM_REQ_V01, QMI_IDL_TYPE16(0, 107), 18},
  {QMI_CSD_IOCTL_VM_CMD_ATTACH_CONTEXT_REQ_V01, QMI_IDL_TYPE16(0, 109), 18},
  {QMI_CSD_IOCTL_VM_CMD_DETACH_CONTEXT_REQ_V01, QMI_IDL_TYPE16(0, 111), 18},
  {QMI_CSD_IOCTL_VM_CMD_START_VOICE_REQ_V01, QMI_IDL_TYPE16(0, 113), 14},
  {QMI_CSD_IOCTL_VM_CMD_STOP_VOICE_REQ_V01, QMI_IDL_TYPE16(0, 117), 14},
  {QMI_CSD_IOCTL_VM_CMD_SET_NETWORK_REQ_V01, QMI_IDL_TYPE16(0, 119), 18},
  {QMI_CSD_IOCTL_VM_CMD_SET_VOICE_TIMING_REQ_V01, QMI_IDL_TYPE16(0, 121), 22},
  {QMI_CSD_IOCTL_VM_CMD_SET_TTY_MODE_REQ_V01, QMI_IDL_TYPE16(0, 123), 18},
  {QMI_CSD_IOCTL_VM_CMD_SET_WIDEVOICE_REQ_V01, QMI_IDL_TYPE16(0, 125), 18},
  {QMI_CSD_OPEN_AUDIO_STREAM_REQ_V01, QMI_IDL_TYPE16(0, 127), 31},
  {QMI_CSD_OPEN_AUDIO_CONTEXT_REQ_V01, QMI_IDL_TYPE16(0, 129), 19},
  {QMI_CSD_AS_CMD_START_SESSION_REQ_V01, QMI_IDL_TYPE16(0, 131), 22},
  {QMI_CSD_AS_CMD_STOP_SESSION_REQ_V01, QMI_IDL_TYPE16(0, 133), 7},
  {QMI_CSD_AS_CMD_FLUSH_STREAM_REQ_V01, QMI_IDL_TYPE16(0, 135), 7},
  {QMI_CSD_AS_CMD_FLUSH_STREAM_TX_REQ_V01, QMI_IDL_TYPE16(0, 137), 7},
  {QMI_CSD_AS_CMD_GET_VOL_LEVELS_REQ_V01, QMI_IDL_TYPE16(0, 139), 7},
  {QMI_CSD_AS_CMD_GET_DSP_CLK_REQ_V01, QMI_IDL_TYPE16(0, 141), 7},
  {QMI_CSD_AS_CMD_GET_RENDERED_TIME_REQ_V01, QMI_IDL_TYPE16(0, 143), 7},
  {QMI_CSD_AS_CMD_GET_SESSION_ID_REQ_V01, QMI_IDL_TYPE16(0, 145), 7},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_PCM_REQ_V01, QMI_IDL_TYPE16(0, 147), 40},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_ADPCM_REQ_V01, QMI_IDL_TYPE16(0, 149), 35},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_MIDI_REQ_V01, QMI_IDL_TYPE16(0, 151), 14},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_WMAV9_REQ_V01, QMI_IDL_TYPE16(0, 153), 56},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_WMAV10_REQ_V01, QMI_IDL_TYPE16(0, 155), 62},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_AAC_REQ_V01, QMI_IDL_TYPE16(0, 157), 37},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_G711_REQ_V01, QMI_IDL_TYPE16(0, 159), 14},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_FLAC_REQ_V01, QMI_IDL_TYPE16(0, 161), 47},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_VORBIS_REQ_V01, QMI_IDL_TYPE16(0, 163), 56},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_AMRWBPLUS_REQ_V01, QMI_IDL_TYPE16(0, 165), 42},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_PCM_REQ_V01, QMI_IDL_TYPE16(0, 167), 40},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AAC_REQ_V01, QMI_IDL_TYPE16(0, 169), 42},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_G711_REQ_V01, QMI_IDL_TYPE16(0, 171), 14},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AMRNB_REQ_V01, QMI_IDL_TYPE16(0, 173), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AMRWB_REQ_V01, QMI_IDL_TYPE16(0, 175), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_QCELP13K_REQ_V01, QMI_IDL_TYPE16(0, 177), 33},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRC_REQ_V01, QMI_IDL_TYPE16(0, 179), 33},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRCB_REQ_V01, QMI_IDL_TYPE16(0, 181), 35},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRCWB_REQ_V01, QMI_IDL_TYPE16(0, 183), 35},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_SBC_REQ_V01, QMI_IDL_TYPE16(0, 185), 49},
  {QMI_CSD_AS_CMD_SET_STREAM_EOS_REQ_V01, QMI_IDL_TYPE16(0, 187), 7},
  {QMI_CSD_AS_CMD_CONFIG_PP_VOL_MASTER_GAIN_REQ_V01, QMI_IDL_TYPE16(0, 189), 12},
  {QMI_CSD_AS_CMD_CONFIG_PP_VOL_STEREO_GAIN_REQ_V01, QMI_IDL_TYPE16(0, 191), 17},
  {QMI_CSD_AS_CMD_CONFIG_PP_VOL_MULTICHANNEL_GAIN_REQ_V01, QMI_IDL_TYPE16(0, 193), 59},
  {QMI_CSD_AS_CMD_CONFIG_PP_VOL_MUTE_REQ_V01, QMI_IDL_TYPE16(0, 195), 14},
  {QMI_CSD_AS_CMD_CONFIG_PP_EQ_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 197), 251},
  {QMI_CSD_AS_CMD_CONFIG_PP_QCPR_REQ_V01, QMI_IDL_TYPE16(0, 199), 18},
  {QMI_CSD_AS_CMD_CONFIG_PP_SPA_REQ_V01, QMI_IDL_TYPE16(0, 201), 18},
  {QMI_CSD_AS_CMD_CONFIG_PP_TSM_REQ_V01, QMI_IDL_TYPE16(0, 203), 12},
  {QMI_CSD_AS_CMD_GET_SPA_DATA_REQ_V01, QMI_IDL_TYPE16(0, 205), 7},
  {QMI_CSD_AS_CMD_SET_DUAL_MONO_REMAP_REQ_V01, QMI_IDL_TYPE16(0, 207), 14},
  {QMI_CSD_AS_CMD_ADJUST_SESSION_CLOCK_REQ_V01, QMI_IDL_TYPE16(0, 209), 21},
  {QMI_CSD_AS_CMD_SET_AAC_SBR_PS_REQ_V01, QMI_IDL_TYPE16(0, 211), 14},
  {QMI_CSD_AS_CMD_DTMF_CTL_REQ_V01, QMI_IDL_TYPE16(0, 213), 27},
  {QMI_CSD_AS_CMD_SET_STREAM_INFO_REQ_V01, QMI_IDL_TYPE16(0, 215), 14},
  {QMI_CSD_AS_CMD_GET_RENDERED_BYTE_OFFSET_REQ_V01, QMI_IDL_TYPE16(0, 217), 7},
  {QMI_CSD_AS_CMD_GET_MIDI_SEQUENCE_ID_REQ_V01, QMI_IDL_TYPE16(0, 219), 7},
  {QMI_CSD_AS_CMD_ENCODER_BIT_RATE_UPDATE_REQ_V01, QMI_IDL_TYPE16(0, 221), 21},
  {QMI_CSD_AS_CMD_CONFIG_DECODER_MULTI_CHANNEL_WMAV10_REQ_V01, QMI_IDL_TYPE16(0, 223), 14},
  {QMI_CSD_AS_CMD_CONFIG_DECODER_MULTI_CHANNEL_EAC3_REQ_V01, QMI_IDL_TYPE16(0, 225), 56},
  {QMI_CSD_AC_CMD_AS_ATTACH_REQ_V01, QMI_IDL_TYPE16(0, 230), 43},
  {QMI_CSD_AC_CMD_AS_DETACH_REQ_V01, QMI_IDL_TYPE16(0, 232), 43},
  {QMI_CSD_AC_CMD_SET_DEVICE_REQ_V01, QMI_IDL_TYPE16(0, 234), 21},
  {QMI_CSD_AC_CMD_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 236), 7},
  {QMI_CSD_AC_CMD_DISABLE_REQ_V01, QMI_IDL_TYPE16(0, 238), 7},
  {QMI_CSD_AC_CMD_CONFIG_PP_VOL_MASTER_GAIN_REQ_V01, QMI_IDL_TYPE16(0, 240), 12},
  {QMI_CSD_AC_CMD_CONFIG_PP_VOL_STEREO_GAIN_REQ_V01, QMI_IDL_TYPE16(0, 242), 17},
  {QMI_CSD_AC_CMD_CONFIG_PP_VOL_MULTICHANNEL_GAIN_REQ_V01, QMI_IDL_TYPE16(0, 244), 59},
  {QMI_CSD_AC_CMD_CONFIG_PP_VOL_MUTE_REQ_V01, QMI_IDL_TYPE16(0, 246), 14},
  {QMI_CSD_AC_CMD_CONFIG_PP_EQ_ENABLE_REQ_V01, QMI_IDL_TYPE16(0, 248), 251},
  {QMI_CSD_AC_CMD_CONFIG_PP_QCPR_REQ_V01, QMI_IDL_TYPE16(0, 250), 18},
  {QMI_CSD_AC_CMD_CONFIG_PP_SPA_REQ_V01, QMI_IDL_TYPE16(0, 252), 18},
  {QMI_CSD_AC_CMD_GET_SPA_DATA_REQ_V01, QMI_IDL_TYPE16(0, 254), 7},
  {QMI_CSD_AC_CMD_CONFIG_MULTI_CHANNEL_REQ_V01, QMI_IDL_TYPE16(0, 256), 47},
  {QMI_CSD_IOCTL_DEV_CMD_CONNECT_DEVICE_REQ_V01, QMI_IDL_TYPE16(0, 24), 19},
  {QMI_CSD_IOCTL_VC_CMD_SET_NUMBER_OF_VOLUME_STEPS_REQ_V01, QMI_IDL_TYPE16(0, 48), 18},
  {QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_STEP_REQ_V01, QMI_IDL_TYPE16(0, 50), 20},
  {QMI_CSD_IOCTL_VS_CMD_START_PLAYBACK_REQ_V01, QMI_IDL_TYPE16(0, 98), 21},
  {QMI_CSD_IOCTL_VS_CMD_STOP_PLAYBACK_REQ_V01, QMI_IDL_TYPE16(0, 100), 14},
  {QMI_CSD_IOCTL_VM_CMD_STANDBY_VOICE_REQ_V01, QMI_IDL_TYPE16(0, 115), 14},
  {QMI_CSD_IOCTL_DEV_CMD_AANC_CONTROL_REQ_V01, QMI_IDL_TYPE16(0, 259), 35},
  {QMI_CSD_IOCTL_VM_CMD_PAUSE_VOICE_REQ_V01, QMI_IDL_TYPE16(0, 261), 14},
  {QMI_CSD_IOCTL_DEV_CMD_RESTART_REQ_V01, QMI_IDL_TYPE16(0, 263), 22},
  {QMI_CSD_IOCTL_VC_CMD_SET_CAL_FEATURE_ID_REQ_V01, QMI_IDL_TYPE16(0, 265), 22}
};

static const qmi_idl_service_message_table_entry csd_service_response_messages_v01[] = {
  {QMI_CSD_QUERY_DRIVER_VERSION_RESP_V01, QMI_IDL_TYPE16(0, 1), 14},
  {QMI_CSD_INIT_RESP_V01, QMI_IDL_TYPE16(0, 3), 14},
  {QMI_CSD_DEINIT_RESP_V01, QMI_IDL_TYPE16(0, 5), 14},
  {QMI_CSD_OPEN_PASSIVE_CONTROL_VOICE_STREAM_RESP_V01, QMI_IDL_TYPE16(0, 7), 21},
  {QMI_CSD_OPEN_FULL_CONTROL_VOICE_STREAM_RESP_V01, QMI_IDL_TYPE16(0, 9), 21},
  {QMI_CSD_OPEN_VOICE_CONTEXT_RESP_V01, QMI_IDL_TYPE16(0, 11), 21},
  {QMI_CSD_OPEN_VOICE_MANAGER_RESP_V01, QMI_IDL_TYPE16(0, 13), 21},
  {QMI_CSD_OPEN_DEVICE_CONTROL_RESP_V01, QMI_IDL_TYPE16(0, 15), 21},
  {QMI_CSD_CLOSE_RESP_V01, QMI_IDL_TYPE16(0, 17), 21},
  {QMI_CSD_IOCTL_DEV_CMD_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 19), 14},
  {QMI_CSD_IOCTL_DEV_CMD_DISABLE_RESP_V01, QMI_IDL_TYPE16(0, 21), 14},
  {QMI_CSD_IOCTL_DEV_CMD_AFE_LOOPBACK_RESP_V01, QMI_IDL_TYPE16(0, 23), 14},
  {QMI_CSD_IOCTL_DEV_CMD_ANC_CONTROL_RESP_V01, QMI_IDL_TYPE16(0, 27), 14},
  {QMI_CSD_IOCTL_DEV_CMD_COMPANDING_CONTROL_RESP_V01, QMI_IDL_TYPE16(0, 29), 14},
  {QMI_CSD_IOCTL_DEV_CMD_GET_MAX_DEVICE_NUMS_RESP_V01, QMI_IDL_TYPE16(0, 31), 21},
  {QMI_CSD_IOCTL_DEV_CMD_GET_DEV_CAPS_RESP_V01, QMI_IDL_TYPE16(0, 33), 3095},
  {QMI_CSD_IOCTL_DEV_CMD_DTMF_CONTROL_RESP_V01, QMI_IDL_TYPE16(0, 35), 14},
  {QMI_CSD_IOCTL_DEV_CMD_SIDETONE_CONTROL_RESP_V01, QMI_IDL_TYPE16(0, 37), 14},
  {QMI_CSD_IOCTL_DEV_CMD_CONFIGURE_RESP_V01, QMI_IDL_TYPE16(0, 39), 14},
  {QMI_CSD_IOCTL_VC_CMD_SET_DEVICE_CONFIG_RESP_V01, QMI_IDL_TYPE16(0, 41), 28},
  {QMI_CSD_IOCTL_VC_CMD_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 43), 28},
  {QMI_CSD_IOCTL_VC_CMD_DISABLE_RESP_V01, QMI_IDL_TYPE16(0, 45), 28},
  {QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_INDEX_RESP_V01, QMI_IDL_TYPE16(0, 47), 28},
  {QMI_CSD_IOCTL_VC_CMD_SET_MUTE_RESP_V01, QMI_IDL_TYPE16(0, 53), 28},
  {QMI_CSD_IOCTL_VC_CMD_SET_TX_DTMF_DETECTION_RESP_V01, QMI_IDL_TYPE16(0, 55), 28},
  {QMI_CSD_IOCTL_VC_CMD_SET_UI_PROPERTY_RESP_V01, QMI_IDL_TYPE16(0, 58), 28},
  {QMI_CSD_IOCTL_VC_CMD_GET_UI_PROPERTY_RESP_V01, QMI_IDL_TYPE16(0, 60), 2093},
  {QMI_CSD_IOCTL_VS_CMD_SET_MEDIA_TYPE_RESP_V01, QMI_IDL_TYPE16(0, 63), 28},
  {QMI_CSD_IOCTL_VS_CMD_SET_MUTE_RESP_V01, QMI_IDL_TYPE16(0, 65), 28},
  {QMI_CSD_IOCTL_VS_CMD_SET_ENCODER_DTX_MODE_RESP_V01, QMI_IDL_TYPE16(0, 67), 28},
  {QMI_CSD_IOCTL_VS_CMD_SET_DEC_TIMEWARP_RESP_V01, QMI_IDL_TYPE16(0, 69), 28},
  {QMI_CSD_IOCTL_VS_CMD_SET_ENC_MINMAX_RATE_RESP_V01, QMI_IDL_TYPE16(0, 71), 28},
  {QMI_CSD_IOCTL_VS_CMD_SET_ENC_RATE_MODULATION_RESP_V01, QMI_IDL_TYPE16(0, 73), 28},
  {QMI_CSD_IOCTL_VS_CMD_VOC_QCELP13K_SET_RATE_RESP_V01, QMI_IDL_TYPE16(0, 75), 28},
  {QMI_CSD_IOCTL_VS_CMD_VOC_4GVNB_SET_RATE_RESP_V01, QMI_IDL_TYPE16(0, 77), 28},
  {QMI_CSD_IOCTL_VS_CMD_VOC_4GVWB_SET_RATE_RESP_V01, QMI_IDL_TYPE16(0, 79), 28},
  {QMI_CSD_IOCTL_VS_CMD_VOC_AMR_SET_ENC_RATE_RESP_V01, QMI_IDL_TYPE16(0, 81), 28},
  {QMI_CSD_IOCTL_VS_CMD_VOC_AMRWB_SET_ENC_RATE_RESP_V01, QMI_IDL_TYPE16(0, 83), 28},
  {QMI_CSD_IOCTL_VS_CMD_SET_DTMF_GENERATION_RESP_V01, QMI_IDL_TYPE16(0, 85), 28},
  {QMI_CSD_IOCTL_VS_CMD_SET_RX_DTMF_DETECTION_RESP_V01, QMI_IDL_TYPE16(0, 88), 28},
  {QMI_CSD_IOCTL_VS_CMD_SET_UI_PROPERTY_RESP_V01, QMI_IDL_TYPE16(0, 91), 28},
  {QMI_CSD_IOCTL_VS_CMD_GET_UI_PROPERTY_RESP_V01, QMI_IDL_TYPE16(0, 93), 2093},
  {QMI_CSD_IOCTL_VS_CMD_START_RECORD_RESP_V01, QMI_IDL_TYPE16(0, 95), 28},
  {QMI_CSD_IOCTL_VS_CMD_STOP_RECORD_RESP_V01, QMI_IDL_TYPE16(0, 97), 28},
  {QMI_CSD_IOCTL_VM_CMD_ATTACH_STREAM_RESP_V01, QMI_IDL_TYPE16(0, 106), 28},
  {QMI_CSD_IOCTL_VM_CMD_DETACH_STREAM_RESP_V01, QMI_IDL_TYPE16(0, 108), 28},
  {QMI_CSD_IOCTL_VM_CMD_ATTACH_CONTEXT_RESP_V01, QMI_IDL_TYPE16(0, 110), 28},
  {QMI_CSD_IOCTL_VM_CMD_DETACH_CONTEXT_RESP_V01, QMI_IDL_TYPE16(0, 112), 28},
  {QMI_CSD_IOCTL_VM_CMD_START_VOICE_RESP_V01, QMI_IDL_TYPE16(0, 114), 28},
  {QMI_CSD_IOCTL_VM_CMD_STOP_VOICE_RESP_V01, QMI_IDL_TYPE16(0, 118), 28},
  {QMI_CSD_IOCTL_VM_CMD_SET_NETWORK_RESP_V01, QMI_IDL_TYPE16(0, 120), 28},
  {QMI_CSD_IOCTL_VM_CMD_SET_VOICE_TIMING_RESP_V01, QMI_IDL_TYPE16(0, 122), 28},
  {QMI_CSD_IOCTL_VM_CMD_SET_TTY_MODE_RESP_V01, QMI_IDL_TYPE16(0, 124), 28},
  {QMI_CSD_IOCTL_VM_CMD_SET_WIDEVOICE_RESP_V01, QMI_IDL_TYPE16(0, 126), 28},
  {QMI_CSD_OPEN_AUDIO_STREAM_RESP_V01, QMI_IDL_TYPE16(0, 128), 21},
  {QMI_CSD_OPEN_AUDIO_CONTEXT_RESP_V01, QMI_IDL_TYPE16(0, 130), 21},
  {QMI_CSD_AS_CMD_START_SESSION_RESP_V01, QMI_IDL_TYPE16(0, 132), 21},
  {QMI_CSD_AS_CMD_STOP_SESSION_RESP_V01, QMI_IDL_TYPE16(0, 134), 21},
  {QMI_CSD_AS_CMD_FLUSH_STREAM_RESP_V01, QMI_IDL_TYPE16(0, 136), 21},
  {QMI_CSD_AS_CMD_FLUSH_STREAM_TX_RESP_V01, QMI_IDL_TYPE16(0, 138), 21},
  {QMI_CSD_AS_CMD_GET_VOL_LEVELS_RESP_V01, QMI_IDL_TYPE16(0, 140), 28},
  {QMI_CSD_AS_CMD_GET_DSP_CLK_RESP_V01, QMI_IDL_TYPE16(0, 142), 36},
  {QMI_CSD_AS_CMD_GET_RENDERED_TIME_RESP_V01, QMI_IDL_TYPE16(0, 144), 36},
  {QMI_CSD_AS_CMD_GET_SESSION_ID_RESP_V01, QMI_IDL_TYPE16(0, 146), 28},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_PCM_RESP_V01, QMI_IDL_TYPE16(0, 148), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_ADPCM_RESP_V01, QMI_IDL_TYPE16(0, 150), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_MIDI_RESP_V01, QMI_IDL_TYPE16(0, 152), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_WMAV9_RESP_V01, QMI_IDL_TYPE16(0, 154), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_WMAV10_RESP_V01, QMI_IDL_TYPE16(0, 156), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_AAC_RESP_V01, QMI_IDL_TYPE16(0, 158), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_G711_RESP_V01, QMI_IDL_TYPE16(0, 160), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_FLAC_RESP_V01, QMI_IDL_TYPE16(0, 162), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_VORBIS_RESP_V01, QMI_IDL_TYPE16(0, 164), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_RX_AMRWBPLUS_RESP_V01, QMI_IDL_TYPE16(0, 166), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_PCM_RESP_V01, QMI_IDL_TYPE16(0, 168), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AAC_RESP_V01, QMI_IDL_TYPE16(0, 170), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_G711_RESP_V01, QMI_IDL_TYPE16(0, 172), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AMRNB_RESP_V01, QMI_IDL_TYPE16(0, 174), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_AMRWB_RESP_V01, QMI_IDL_TYPE16(0, 176), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_QCELP13K_RESP_V01, QMI_IDL_TYPE16(0, 178), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRC_RESP_V01, QMI_IDL_TYPE16(0, 180), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRCB_RESP_V01, QMI_IDL_TYPE16(0, 182), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_EVRCWB_RESP_V01, QMI_IDL_TYPE16(0, 184), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_FMT_TX_SBC_RESP_V01, QMI_IDL_TYPE16(0, 186), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_EOS_RESP_V01, QMI_IDL_TYPE16(0, 188), 21},
  {QMI_CSD_AS_CMD_CONFIG_PP_VOL_MASTER_GAIN_RESP_V01, QMI_IDL_TYPE16(0, 190), 21},
  {QMI_CSD_AS_CMD_CONFIG_PP_VOL_STEREO_GAIN_RESP_V01, QMI_IDL_TYPE16(0, 192), 21},
  {QMI_CSD_AS_CMD_CONFIG_PP_VOL_MULTICHANNEL_GAIN_RESP_V01, QMI_IDL_TYPE16(0, 194), 21},
  {QMI_CSD_AS_CMD_CONFIG_PP_VOL_MUTE_RESP_V01, QMI_IDL_TYPE16(0, 196), 21},
  {QMI_CSD_AS_CMD_CONFIG_PP_EQ_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 198), 21},
  {QMI_CSD_AS_CMD_CONFIG_PP_QCPR_RESP_V01, QMI_IDL_TYPE16(0, 200), 21},
  {QMI_CSD_AS_CMD_CONFIG_PP_SPA_RESP_V01, QMI_IDL_TYPE16(0, 202), 21},
  {QMI_CSD_AS_CMD_CONFIG_PP_TSM_RESP_V01, QMI_IDL_TYPE16(0, 204), 21},
  {QMI_CSD_AS_CMD_GET_SPA_DATA_RESP_V01, QMI_IDL_TYPE16(0, 206), 21},
  {QMI_CSD_AS_CMD_SET_DUAL_MONO_REMAP_RESP_V01, QMI_IDL_TYPE16(0, 208), 21},
  {QMI_CSD_AS_CMD_ADJUST_SESSION_CLOCK_RESP_V01, QMI_IDL_TYPE16(0, 210), 35},
  {QMI_CSD_AS_CMD_SET_AAC_SBR_PS_RESP_V01, QMI_IDL_TYPE16(0, 212), 21},
  {QMI_CSD_AS_CMD_DTMF_CTL_RESP_V01, QMI_IDL_TYPE16(0, 214), 21},
  {QMI_CSD_AS_CMD_SET_STREAM_INFO_RESP_V01, QMI_IDL_TYPE16(0, 216), 21},
  {QMI_CSD_AS_CMD_GET_RENDERED_BYTE_OFFSET_RESP_V01, QMI_IDL_TYPE16(0, 218), 32},
  {QMI_CSD_AS_CMD_GET_MIDI_SEQUENCE_ID_RESP_V01, QMI_IDL_TYPE16(0, 220), 25},
  {QMI_CSD_AS_CMD_ENCODER_BIT_RATE_UPDATE_RESP_V01, QMI_IDL_TYPE16(0, 222), 21},
  {QMI_CSD_AS_CMD_CONFIG_DECODER_MULTI_CHANNEL_WMAV10_RESP_V01, QMI_IDL_TYPE16(0, 224), 21},
  {QMI_CSD_AS_CMD_CONFIG_DECODER_MULTI_CHANNEL_EAC3_RESP_V01, QMI_IDL_TYPE16(0, 226), 21},
  {QMI_CSD_AC_CMD_AS_ATTACH_RESP_V01, QMI_IDL_TYPE16(0, 231), 21},
  {QMI_CSD_AC_CMD_AS_DETACH_RESP_V01, QMI_IDL_TYPE16(0, 233), 21},
  {QMI_CSD_AC_CMD_SET_DEVICE_RESP_V01, QMI_IDL_TYPE16(0, 235), 21},
  {QMI_CSD_AC_CMD_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 237), 21},
  {QMI_CSD_AC_CMD_DISABLE_RESP_V01, QMI_IDL_TYPE16(0, 239), 21},
  {QMI_CSD_AC_CMD_CONFIG_PP_VOL_MASTER_GAIN_RESP_V01, QMI_IDL_TYPE16(0, 241), 21},
  {QMI_CSD_AC_CMD_CONFIG_PP_VOL_STEREO_GAIN_RESP_V01, QMI_IDL_TYPE16(0, 243), 21},
  {QMI_CSD_AC_CMD_CONFIG_PP_VOL_MULTICHANNEL_GAIN_RESP_V01, QMI_IDL_TYPE16(0, 245), 21},
  {QMI_CSD_AC_CMD_CONFIG_PP_VOL_MUTE_RESP_V01, QMI_IDL_TYPE16(0, 247), 21},
  {QMI_CSD_AC_CMD_CONFIG_PP_EQ_ENABLE_RESP_V01, QMI_IDL_TYPE16(0, 249), 21},
  {QMI_CSD_AC_CMD_CONFIG_PP_QCPR_RESP_V01, QMI_IDL_TYPE16(0, 251), 21},
  {QMI_CSD_AC_CMD_CONFIG_PP_SPA_RESP_V01, QMI_IDL_TYPE16(0, 253), 21},
  {QMI_CSD_AC_CMD_GET_SPA_DATA_RESP_V01, QMI_IDL_TYPE16(0, 255), 21},
  {QMI_CSD_AC_CMD_CONFIG_MULTI_CHANNEL_RESP_V01, QMI_IDL_TYPE16(0, 257), 21},
  {QMI_CSD_IOCTL_DEV_CMD_CONNECT_DEVICE_RESP_V01, QMI_IDL_TYPE16(0, 25), 14},
  {QMI_CSD_IOCTL_VC_CMD_SET_NUMBER_OF_VOLUME_STEPS_RESP_V01, QMI_IDL_TYPE16(0, 49), 28},
  {QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_STEP_RESP_V01, QMI_IDL_TYPE16(0, 51), 28},
  {QMI_CSD_IOCTL_VS_CMD_START_PLAYBACK_RESP_V01, QMI_IDL_TYPE16(0, 99), 28},
  {QMI_CSD_IOCTL_VS_CMD_STOP_PLAYBACK_RESP_V01, QMI_IDL_TYPE16(0, 101), 28},
  {QMI_CSD_IOCTL_VM_CMD_STANDBY_VOICE_RESP_V01, QMI_IDL_TYPE16(0, 116), 28},
  {QMI_CSD_IOCTL_DEV_CMD_AANC_CONTROL_RESP_V01, QMI_IDL_TYPE16(0, 260), 14},
  {QMI_CSD_IOCTL_VM_CMD_PAUSE_VOICE_RESP_V01, QMI_IDL_TYPE16(0, 262), 28},
  {QMI_CSD_IOCTL_DEV_CMD_RESTART_RESP_V01, QMI_IDL_TYPE16(0, 264), 14},
  {QMI_CSD_IOCTL_VC_CMD_SET_CAL_FEATURE_ID_RESP_V01, QMI_IDL_TYPE16(0, 266), 28}
};

static const qmi_idl_service_message_table_entry csd_service_indication_messages_v01[] = {
  {QMI_CSD_IOCTL_VC_TX_DTMF_DETECTED_IND_V01, QMI_IDL_TYPE16(0, 56), 17},
  {QMI_CSD_IOCTL_VC_STATE_IND_V01, QMI_IDL_TYPE16(0, 61), 14},
  {QMI_CSD_IOCTL_VS_DTMF_GENERATION_ENDED_IND_V01, QMI_IDL_TYPE16(0, 86), 19},
  {QMI_CSD_IOCTL_VS_RX_DTMF_DETECTED_IND_V01, QMI_IDL_TYPE16(0, 89), 17},
  {QMI_CSD_IOCTL_VS_STATE_IND_V01, QMI_IDL_TYPE16(0, 102), 14},
  {QMI_CSD_IOCTL_VS_ENC_BUFFER_IND_V01, QMI_IDL_TYPE16(0, 103), 7},
  {QMI_CSD_IOCTL_VS_DEC_BUFFER_IND_V01, QMI_IDL_TYPE16(0, 104), 7},
  {QMI_CSD_AS_EVT_EOS_IND_V01, QMI_IDL_TYPE16(0, 227), 7},
  {QMI_CSD_AS_EVT_SPA_BUF_READY_IND_V01, QMI_IDL_TYPE16(0, 229), 4108},
  {QMI_CSD_AS_EVT_SR_CM_CHANGE_IND_V01, QMI_IDL_TYPE16(0, 228), 30},
  {QMI_CSD_AC_EVT_SPA_BUF_READY_IND_V01, QMI_IDL_TYPE16(0, 258), 4108}
};

/*Service Object*/
struct qmi_idl_service_object csd_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x14,
  4108,
  { sizeof(csd_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(csd_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(csd_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { csd_service_command_messages_v01, csd_service_response_messages_v01, csd_service_indication_messages_v01},
  &csd_qmi_idl_type_table_object_v01,
  0x09,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type csd_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( CSD_V01_IDL_MAJOR_VERS != idl_maj_version || CSD_V01_IDL_MINOR_VERS != idl_min_version
       || CSD_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&csd_qmi_idl_service_object_v01;
}

