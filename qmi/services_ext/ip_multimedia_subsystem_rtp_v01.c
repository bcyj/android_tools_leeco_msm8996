/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        I P _ M U L T I M E D I A _ S U B S Y S T E M _ R T P _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the imsrtp service Data structures.

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //source/qcom/qct/interfaces/qmi/imsrtp/main/latest/src/ip_multimedia_subsystem_rtp_v01.c#26 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.2 
   It requires encode/decode library version 5 or later
   It was generated on: Tue Jun  4 2013 (Spin 0)
   From IDL File: ip_multimedia_subsystem_rtp_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "ip_multimedia_subsystem_rtp_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t imsrtp_ip_addr_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_ip_addr_type_v01, ipaddr_type),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsrtp_ip_addr_type_v01, ipaddr),
  IMSRTP_IP_STR_LEN_V01,

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_rtcp_params_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsrtp_rtcp_params_type_v01, cname),
  ((IMSRTP_RTCP_SDES_ITEM_LEN_V01) & 0xFF), ((IMSRTP_RTCP_SDES_ITEM_LEN_V01) >> 8),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_rtcp_params_type_v01, rtcp_ib_port),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_rtcp_params_type_v01, rtcp_ob_port),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_rtcp_params_type_v01, rtcp_tx_enabled),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_codec_params_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_codec_params_type_v01, codec),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_codec_params_type_v01, codec_mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_amr_config_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_amr_config_type_v01, octet_tx_align),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_amr_config_type_v01, octet_rx_align),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_amr_config_type_v01, mode_map_valid),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_amr_config_type_v01, mode_map),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_amr_config_type_v01, max_tx_red),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_amr_config_type_v01, max_rx_red),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_amr_config_type_v01, tx_cmr),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_h264_config_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_h264_config_type_v01, packetization_mode_valid),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_h264_config_type_v01, packetization_mode),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_session_params_type_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_params_type_v01, rx_pt_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_params_type_v01, tx_pt_type),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_params_type_v01, ptime),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_params_type_v01, maxptime),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_params_type_v01, dtmf_clock_rate),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_params_type_v01, dtmf_pt_type),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_rtcp_sr_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sr_type_v01, ssrc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sr_type_v01, hi_ntp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sr_type_v01, lo_ntp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sr_type_v01, tstamp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sr_type_v01, num_pkts),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sr_type_v01, num_bytes),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_rtcp_rr_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, ssrc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, s_ssrc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, frac_lost),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, tot_lost),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, ext_seq),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, est_jitter),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, lsr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, dslr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_rr_type_v01, rtt),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_rtcp_sdes_item_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sdes_item_type_v01, ssrc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sdes_item_type_v01, subtype),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsrtp_rtcp_sdes_item_type_v01, data),
  ((IMSRTP_RTCP_SDES_ITEM_LEN_V01) & 0xFF), ((IMSRTP_RTCP_SDES_ITEM_LEN_V01) >> 8),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_video_config_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, codec),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, width),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, height),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, bit_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, frame_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, clock_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, lip_sync_drop_upper_limit),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, lip_sync_drop_lower_limit),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, lip_sync_enable),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, lip_sync_audio_clock_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_config_type_v01, lip_sync_audio_packet_interval),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_video_h264_config_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_h264_config_v01, profile_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_h264_config_v01, profile_level),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_h264_config_v01, nalheader),
  IMS_RTP_NAL_HEADER_MAX_V01,
  QMI_IDL_OFFSET8(imsrtp_video_h264_config_v01, nalheader) - QMI_IDL_OFFSET8(imsrtp_video_h264_config_v01, nalheader_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_video_mp4_config_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_mp4_config_v01, profile_level_id),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_mp4_config_v01, volheader),
  IMS_RTP_VOL_HEADER_MAX_V01,
  QMI_IDL_OFFSET8(imsrtp_video_mp4_config_v01, volheader) - QMI_IDL_OFFSET8(imsrtp_video_mp4_config_v01, volheader_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_video_h263_config_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_h263_config_v01, profile_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_h263_config_v01, profile_level),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_video_time_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_time_info_type_v01, n_imsntp_time),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_time_info_type_v01, n_ilsntp_time),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_time_info_type_v01, n_irtp_time_stamp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_rtcp_xr_voip_metrics_report_block_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, ssrc),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, loss_rate),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, discard_rate),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, burst_density),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, gap_density),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, burst_duration),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, gap_duration),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, round_trip_delay),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, end_system_delay),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, signal_level),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, noise_level),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, residual_echo_return_loss),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, gap_threshhold),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, r_factor),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, ext_r_factor),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, mos_lq),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, mos_cq),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, packet_loss_concealment),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, jitter_buffer_adaptive),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, jitter_buffer_rate),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, jitter_buffer_nominal),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, jitter_buffer_maximum),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01, jitter_buffer_abs_max),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_rtcp_xr_dllr_report_block_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_dllr_report_block_type_v01, ssrc),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_dllr_report_block_type_v01, last_rr_timestamp),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_dllr_report_block_type_v01, delay_since_last_rr),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_dllr_report_block_type_v01, rtt),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_rtcp_xr_statistics_summary_report_block_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, ssrc),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, begin_seq),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, end_seq),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, lost_packets),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, dup_packets),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, min_jitter),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, max_jitter),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, mean_jitter),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, dev_jitter),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, ipaddr_type),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, min_ttl_or_hl),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, max_ttl_or_hl),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, mean_ttl_or_hl),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01, dev_ttl_or_hl),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_video_capability_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_capability_v01, codec),

  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_capability_v01, resolutions_supported),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_capability_v01, min_bit_rate),

  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_capability_v01, min_frame_rate),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_text_config_params_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_text_config_params_v01, cps),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_text_config_params_v01, num_red_pkts),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_text_config_params_v01, red_payload),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_text_config_params_v01, t140_payload),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsrtp_srtp_info_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_STRING,
  QMI_IDL_OFFSET8(imsrtp_srtp_info_v01, master_key),
  IMSRTP_MASTERKEY_STR_LEN_V01,

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_srtp_info_v01, sec_algo),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_srtp_info_v01, hash_algo),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_srtp_info_v01, is_authentication_enabled),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t qvp_rtcp_fb_messages_data_v01[] = {
  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qvp_rtcp_fb_messages_v01, rtcp_nack_fb_enabled),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qvp_rtcp_fb_messages_v01, rtcp_pli_fb_enabled),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qvp_rtcp_fb_messages_v01, rtcp_fir_fb_enabled),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(qvp_rtcp_fb_messages_v01, rtcp_tmmbr_fb_enabled),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t imsrtp_initialize_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_initialize_req_msg_v01, profile_id)
};

static const uint8_t imsrtp_initialize_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_initialize_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_initialize_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_initialize_ind_v01, status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_initialize_ind_v01, app_id) - QMI_IDL_OFFSET8(imsrtp_initialize_ind_v01, app_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_initialize_ind_v01, app_id)
};

static const uint8_t imsrtp_uninitialize_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_uninitialize_req_msg_v01, app_id)
};

static const uint8_t imsrtp_uninitialize_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_uninitialize_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_uninitialize_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_uninitialize_ind_v01, status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_uninitialize_ind_v01, app_id)
};

static const uint8_t imsrtp_session_initialize_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, app_id),

  0x02,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, rtp_ib_port),

  0x03,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, ipaddr_type),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, rtp_ob_local_port) - QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, rtp_ob_local_port_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, rtp_ob_local_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, remote_ip) - QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, remote_ip_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, remote_ip),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, remote_port) - QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, remote_port_valid)),
  0x12,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, remote_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, codec_param) - QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, codec_param_valid)),
  0x13,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, codec_param),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, session_param) - QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, session_param_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, session_param),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, amr_config_type) - QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, amr_config_type_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, amr_config_type),
  3, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, h264_config_type) - QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, h264_config_type_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, h264_config_type),
  4, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, rtcp_param) - QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, rtcp_param_valid)),
  0x17,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_req_msg_v01, rtcp_param),
  1, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, cookie) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, cookie_valid)),
  0x18,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, cookie),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, video_codec_config) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, video_codec_config_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, video_codec_config),
  9, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, h264_specific_config) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, h264_specific_config_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, h264_specific_config),
  10, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, mp4_specific_config) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, mp4_specific_config_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, mp4_specific_config),
  11, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, h263_specific_config) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, h263_specific_config_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, h263_specific_config),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_dtx) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_dtx_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, enable_dtx),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, dcm_rat) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, dcm_rat_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, dcm_rat),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, apn_name) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, apn_name_valid)),
  0x1F,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, apn_name),
  IMSRTP_APN_NAME_LEN_V01,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, apn_type) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, apn_type_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, apn_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, apn_addr_type) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, apn_addr_type_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, apn_addr_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_rtcp_xr) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_rtcp_xr_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, enable_rtcp_xr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtcp_remote_port) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtcp_remote_port_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, rtcp_remote_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, direction) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, direction_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, direction),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_reliable_transmission) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_reliable_transmission_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, enable_reliable_transmission),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_file_streaming) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_file_streaming_valid)),
  0x26,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, enable_file_streaming),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, stream_source_id) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, stream_source_id_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, stream_source_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtp_mtu_size) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtp_mtu_size_valid)),
  0x28,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, rtp_mtu_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtp_txt_params) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtp_txt_params_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, rtp_txt_params),
  18, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtp_remote_txt_params) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtp_remote_txt_params_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, rtp_remote_txt_params),
  18, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_cvo) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_cvo_valid)),
  0x2B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, enable_cvo),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_srtp) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, enable_srtp_valid)),
  0x2C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, enable_srtp),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, tx_srtp_info) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, tx_srtp_info_valid)),
  0x2D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, tx_srtp_info),
  19, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rx_srtp_info) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rx_srtp_info_valid)),
  0x2E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, rx_srtp_info),
  19, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtcp_tx_fb_enable) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, rtcp_tx_fb_enable_valid)),
  0x2F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, rtcp_tx_fb_enable),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, fb_messages) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_initialize_req_msg_v01, fb_messages_valid)),
  0x30,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_initialize_req_msg_v01, fb_messages),
  20, 0
};

static const uint8_t imsrtp_session_initialize_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_initialize_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_sesion_initialize_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, status),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, app_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, session_id) - QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, session_id_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, session_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, cookie) - QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, cookie_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, cookie),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, direction) - QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, direction_valid)),
  0x12,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_sesion_initialize_ind_v01, direction)
};

static const uint8_t imsrtp_session_uninitialize_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_uninitialize_req_msg_v01, session_id)
};

static const uint8_t imsrtp_session_uninitialize_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_uninitialize_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_session_uninitialize_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_uninitialize_ind_v01, status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_uninitialize_ind_v01, session_id)
};

static const uint8_t imsrtp_session_configure_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, session_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtp_ob_local_port) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtp_ob_local_port_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtp_ob_local_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtp_ib_port) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtp_ib_port_valid)),
  0x11,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtp_ib_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, remote_ip) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, remote_ip_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, remote_ip),
  0, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, remote_port) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, remote_port_valid)),
  0x13,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, remote_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, codec_param) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, codec_param_valid)),
  0x14,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, codec_param),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, session_param) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, session_param_valid)),
  0x15,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, session_param),
  5, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, amr_config_type) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, amr_config_type_valid)),
  0x16,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, amr_config_type),
  3, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, h264_config_type) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, h264_config_type_valid)),
  0x17,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, h264_config_type),
  4, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtcp_param) - QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtcp_param_valid)),
  0x18,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_req_msg_v01, rtcp_param),
  1, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, video_codec_config) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, video_codec_config_valid)),
  0x19,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, video_codec_config),
  9, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, h264_specific_config) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, h264_specific_config_valid)),
  0x1A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, h264_specific_config),
  10, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, mp4_specific_config) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, mp4_specific_config_valid)),
  0x1B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, mp4_specific_config),
  11, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, h263_specific_config) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, h263_specific_config_valid)),
  0x1C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, h263_specific_config),
  12, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rs_bandwidth) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rs_bandwidth_valid)),
  0x1D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rs_bandwidth),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rr_bandwidth) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rr_bandwidth_valid)),
  0x1E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rr_bandwidth),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_dtx) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_dtx_valid)),
  0x1F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, enable_dtx),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_rtcp_xr) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_rtcp_xr_valid)),
  0x20,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, enable_rtcp_xr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtcp_remote_port) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtcp_remote_port_valid)),
  0x21,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rtcp_remote_port),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, direction) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, direction_valid)),
  0x22,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, direction),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, cookie) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, cookie_valid)),
  0x23,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, cookie),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_reliable_transmission) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_reliable_transmission_valid)),
  0x24,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, enable_reliable_transmission),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_file_streaming) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_file_streaming_valid)),
  0x25,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, enable_file_streaming),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, stream_source_id) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, stream_source_id_valid)),
  0x26,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, stream_source_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtp_mtu_size) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtp_mtu_size_valid)),
  0x27,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rtp_mtu_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtp_txt_params) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtp_txt_params_valid)),
  0x28,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rtp_txt_params),
  18, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtp_remote_txt_params) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtp_remote_txt_params_valid)),
  0x29,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rtp_remote_txt_params),
  18, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, ui_orientation) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, ui_orientation_valid)),
  0x2A,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, ui_orientation),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, tx_gbr) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, tx_gbr_valid)),
  0x2B,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, tx_gbr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, tx_mbr) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, tx_mbr_valid)),
  0x2C,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, tx_mbr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rx_gbr) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rx_gbr_valid)),
  0x2D,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rx_gbr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rx_mbr) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rx_mbr_valid)),
  0x2E,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rx_mbr),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_cvo) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_cvo_valid)),
  0x2F,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, enable_cvo),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_srtp) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_srtp_valid)),
  0x30,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, enable_srtp),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, tx_srtp_info) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, tx_srtp_info_valid)),
  0x31,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, tx_srtp_info),
  19, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rx_srtp_info) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rx_srtp_info_valid)),
  0x32,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rx_srtp_info),
  19, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtcp_tx_fb_enable) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, rtcp_tx_fb_enable_valid)),
  0x33,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, rtcp_tx_fb_enable),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, fb_messages) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, fb_messages_valid)),
  0x34,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, fb_messages),
  20, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, cvo_tag_id) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, cvo_tag_id_valid)),
  0x35,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, cvo_tag_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_early_media_blocking) - QMI_IDL_OFFSET16RELATIVE(imsrtp_session_configure_req_msg_v01, enable_early_media_blocking_valid)),
  0x36,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_session_configure_req_msg_v01, enable_early_media_blocking)
};

static const uint8_t imsrtp_session_configure_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_session_configure_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_ind_v01, status),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_ind_v01, session_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_ind_v01, direction) - QMI_IDL_OFFSET8(imsrtp_session_configure_ind_v01, direction_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_ind_v01, direction),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_configure_ind_v01, cookie) - QMI_IDL_OFFSET8(imsrtp_session_configure_ind_v01, cookie_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_configure_ind_v01, cookie)
};

static const uint8_t imsrtp_session_start_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_start_req_msg_v01, session_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_start_req_msg_v01, direction) - QMI_IDL_OFFSET8(imsrtp_session_start_req_msg_v01, direction_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_start_req_msg_v01, direction)
};

static const uint8_t imsrtp_session_start_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_start_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_session_start_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_start_ind_v01, status),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_start_ind_v01, session_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_start_ind_v01, direction) - QMI_IDL_OFFSET8(imsrtp_session_start_ind_v01, direction_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_start_ind_v01, direction)
};

static const uint8_t imsrtp_session_stop_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_stop_req_msg_v01, session_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_stop_req_msg_v01, direction) - QMI_IDL_OFFSET8(imsrtp_session_stop_req_msg_v01, direction_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_stop_req_msg_v01, direction)
};

static const uint8_t imsrtp_session_stop_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_stop_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_session_stop_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_stop_ind_v01, status),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_stop_ind_v01, session_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_stop_ind_v01, direction) - QMI_IDL_OFFSET8(imsrtp_session_stop_ind_v01, direction_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_stop_ind_v01, direction)
};

static const uint8_t imsrtp_session_pause_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_pause_req_msg_v01, session_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_pause_req_msg_v01, direction) - QMI_IDL_OFFSET8(imsrtp_session_pause_req_msg_v01, direction_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_pause_req_msg_v01, direction)
};

static const uint8_t imsrtp_session_pause_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_pause_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_session_resume_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_resume_req_msg_v01, session_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_resume_req_msg_v01, direction) - QMI_IDL_OFFSET8(imsrtp_session_resume_req_msg_v01, direction_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_resume_req_msg_v01, direction)
};

static const uint8_t imsrtp_session_resume_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_resume_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_configure_rtcp_reports_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_reports_req_msg_v01, session_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_reports_req_msg_v01, enable),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_configure_rtcp_reports_req_msg_v01, report_interval) - QMI_IDL_OFFSET8(imsrtp_configure_rtcp_reports_req_msg_v01, report_interval_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_reports_req_msg_v01, report_interval)
};

static const uint8_t imsrtp_configure_rtcp_reports_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_reports_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_configure_rtcp_reports_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_reports_ind_v01, status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_reports_ind_v01, session_id)
};

static const uint8_t imsrtp_configure_rtp_link_monitoring_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtp_link_monitoring_req_msg_v01, session_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtp_link_monitoring_req_msg_v01, enable),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_configure_rtp_link_monitoring_req_msg_v01, interval) - QMI_IDL_OFFSET8(imsrtp_configure_rtp_link_monitoring_req_msg_v01, interval_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtp_link_monitoring_req_msg_v01, interval)
};

static const uint8_t imsrtp_configure_rtp_link_monitoring_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtp_link_monitoring_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_configure_rtp_link_monitoring_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtp_link_monitoring_ind_v01, status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtp_link_monitoring_ind_v01, session_id)
};

static const uint8_t imsrtp_configure_rtcp_link_monitoring_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_link_monitoring_req_msg_v01, session_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_link_monitoring_req_msg_v01, enable),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_configure_rtcp_link_monitoring_req_msg_v01, interval) - QMI_IDL_OFFSET8(imsrtp_configure_rtcp_link_monitoring_req_msg_v01, interval_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_link_monitoring_req_msg_v01, interval)
};

static const uint8_t imsrtp_configure_rtcp_link_monitoring_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_link_monitoring_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_configure_rtcp_link_monitoring_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_link_monitoring_ind_v01, status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_rtcp_link_monitoring_ind_v01, session_id)
};

static const uint8_t imsrtp_send_dtmf_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_send_dtmf_req_msg_v01, session_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_send_dtmf_req_msg_v01, dtmf_digit),

  0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_send_dtmf_req_msg_v01, volume),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x04,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_send_dtmf_req_msg_v01, duration)
};

static const uint8_t imsrtp_send_dtmf_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_send_dtmf_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_start_continuous_dtmf_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_start_continuous_dtmf_req_msg_v01, session_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_start_continuous_dtmf_req_msg_v01, dtmf_digit),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x03,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_start_continuous_dtmf_req_msg_v01, volume)
};

static const uint8_t imsrtp_start_continuous_dtmf_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_start_continuous_dtmf_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_stop_continuous_dtmf_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_stop_continuous_dtmf_req_msg_v01, session_id)
};

static const uint8_t imsrtp_stop_continuous_dtmf_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_stop_continuous_dtmf_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_flush_audio_jitterbuffer_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_flush_audio_jitterbuffer_req_msg_v01, session_id)
};

static const uint8_t imsrtp_flush_audio_jitterbuffer_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_flush_audio_jitterbuffer_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_get_session_status_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_get_session_status_req_msg_v01, session_id)
};

static const uint8_t imsrtp_get_session_status_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_get_session_status_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_get_session_status_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_get_session_status_ind_v01, session_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_get_session_status_ind_v01, session_active) - QMI_IDL_OFFSET8(imsrtp_get_session_status_ind_v01, session_active_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_get_session_status_ind_v01, session_active),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_get_session_status_ind_v01, direction) - QMI_IDL_OFFSET8(imsrtp_get_session_status_ind_v01, direction_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_get_session_status_ind_v01, direction)
};

static const uint8_t imsrtp_register_rtp_services_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_register_rtp_services_req_msg_v01, app_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_register_rtp_services_req_msg_v01, rtp_service)
};

static const uint8_t imsrtp_register_rtp_services_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_register_rtp_services_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_error_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_error_ind_v01, status),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_error_ind_v01, session_id)
};

static const uint8_t imsrtp_rtcp_reports_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, status),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, session_id),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, sr_info) - QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, sr_info_valid)),
  0x10,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, sr_info),
  6, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, rr_info) - QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, rr_info_valid)),
  0x11,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, rr_info),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, sdes_info) - QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, sdes_info_valid)),
  0x12,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_rtcp_reports_ind_v01, sdes_info),
  8, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_rtcp_reports_ind_v01, xr_receiver_reference_time) - QMI_IDL_OFFSET16RELATIVE(imsrtp_rtcp_reports_ind_v01, xr_receiver_reference_time_valid)),
  0x13,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_rtcp_reports_ind_v01, xr_receiver_reference_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_rtcp_reports_ind_v01, xr_statistics_summary_report) - QMI_IDL_OFFSET16RELATIVE(imsrtp_rtcp_reports_ind_v01, xr_statistics_summary_report_valid)),
  0x14,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_rtcp_reports_ind_v01, xr_statistics_summary_report),
  16, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_rtcp_reports_ind_v01, xr_dllr_report) - QMI_IDL_OFFSET16RELATIVE(imsrtp_rtcp_reports_ind_v01, xr_dllr_report_valid)),
  0x15,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_rtcp_reports_ind_v01, xr_dllr_report),
  15, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsrtp_rtcp_reports_ind_v01, xr_voip_metrics_report) - QMI_IDL_OFFSET16RELATIVE(imsrtp_rtcp_reports_ind_v01, xr_voip_metrics_report_valid)),
  0x16,
   QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsrtp_rtcp_reports_ind_v01, xr_voip_metrics_report),
  14, 0
};

static const uint8_t imsrtp_last_audio_play_time_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_last_audio_play_time_req_msg_v01, last_play_time_stamp)
};

static const uint8_t imsrtp_last_audio_play_time_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_last_audio_play_time_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_audio_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_audio_report_req_msg_v01, time_info),
  13, 0
};

static const uint8_t imsrtp_audio_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_audio_report_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_video_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_video_report_req_msg_v01, time_info),
  13, 0
};

static const uint8_t imsrtp_video_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_video_report_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_gen_h264_sps_pps_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_gen_h264_sps_pps_req_msg_v01, video_codec_config),
  9, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_gen_h264_sps_pps_req_msg_v01, h264_config),
  10, 0
};

static const uint8_t imsrtp_gen_h264_sps_pps_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_gen_h264_sps_pps_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_gen_h264_sps_pps_resp_msg_v01, nalheader) - QMI_IDL_OFFSET8(imsrtp_gen_h264_sps_pps_resp_msg_v01, nalheader_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_gen_h264_sps_pps_resp_msg_v01, nalheader),
  IMS_RTP_NAL_HEADER_MAX_V01,
  QMI_IDL_OFFSET8(imsrtp_gen_h264_sps_pps_resp_msg_v01, nalheader) - QMI_IDL_OFFSET8(imsrtp_gen_h264_sps_pps_resp_msg_v01, nalheader_len)
};

static const uint8_t imsrtp_gen_mp4_vol_header_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_gen_mp4_vol_header_req_msg_v01, video_codec_config),
  9, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_gen_mp4_vol_header_req_msg_v01, mp4_config),
  11, 0
};

static const uint8_t imsrtp_gen_mp4_vol_header_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_gen_mp4_vol_header_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_gen_mp4_vol_header_resp_msg_v01, volheader) - QMI_IDL_OFFSET8(imsrtp_gen_mp4_vol_header_resp_msg_v01, volheader_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_gen_mp4_vol_header_resp_msg_v01, volheader),
  IMS_RTP_VOL_HEADER_MAX_V01,
  QMI_IDL_OFFSET8(imsrtp_gen_mp4_vol_header_resp_msg_v01, volheader) - QMI_IDL_OFFSET8(imsrtp_gen_mp4_vol_header_resp_msg_v01, volheader_len)
};

static const uint8_t imsrtp_video_bitrate_adapt_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_bitrate_adapt_req_msg_v01, bit_rate)
};

static const uint8_t imsrtp_video_bitrate_adapt_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_video_bitrate_adapt_resp_msg_v01, resp),
  0, 1
};

/* 
 * imsrtp_h264_idr_generate_req_msg is empty
 * static const uint8_t imsrtp_h264_idr_generate_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsrtp_h264_idr_generate_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_h264_idr_generate_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_av_sync_feed_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsrtp_av_sync_feed_req_msg_v01, n_iavsync_time)
};

static const uint8_t imsrtp_av_sync_feed_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_av_sync_feed_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_video_error_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_error_ind_msg_v01, video_codec_error)
};

static const uint8_t imsrtp_uninitialize_all_rtp_session_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_uninitialize_all_rtp_session_req_msg_v01, app_id)
};

static const uint8_t imsrtp_uninitialize_all_rtp_session_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_uninitialize_all_rtp_session_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_uninitialize_all_rtp_session_ind_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_uninitialize_all_rtp_session_ind_v01, app_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_uninitialize_all_rtp_session_ind_v01, status)
};

static const uint8_t imsrtp_video_idr_generate_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_idr_generate_ind_msg_v01, session_id)
};

static const uint8_t imsrtp_video_bitrate_adapt_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_bitrate_adapt_ind_msg_v01, session_id),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_video_bitrate_adapt_ind_msg_v01, bit_rate)
};

static const uint8_t imsrtp_session_video_capability_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, session_id),

  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, video_capability),
  17, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_bit_rate) - QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_bit_rate_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_bit_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_frame_rate) - QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_frame_rate_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_frame_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_width) - QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_width_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_width),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_height) - QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_height_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsrtp_session_video_capability_req_msg_v01, preferred_height)
};

static const uint8_t imsrtp_session_video_capability_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_session_video_capability_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_configure_nat_over_rtp_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtp_req_msg_v01, session_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtp_req_msg_v01, enable),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtp_req_msg_v01, interval) - QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtp_req_msg_v01, interval_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtp_req_msg_v01, interval)
};

static const uint8_t imsrtp_configure_nat_over_rtp_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtp_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsrtp_configure_nat_over_rtcp_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtcp_req_msg_v01, session_id),

  0x02,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtcp_req_msg_v01, enable),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtcp_req_msg_v01, interval) - QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtcp_req_msg_v01, interval_valid)),
  0x10,
   QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtcp_req_msg_v01, interval)
};

static const uint8_t imsrtp_configure_nat_over_rtcp_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsrtp_configure_nat_over_rtcp_resp_msg_v01, resp),
  0, 1
};

/* Type Table */
static const qmi_idl_type_table_entry  imsrtp_type_table_v01[] = {
  {sizeof(imsrtp_ip_addr_type_v01), imsrtp_ip_addr_type_data_v01},
  {sizeof(imsrtp_rtcp_params_type_v01), imsrtp_rtcp_params_type_data_v01},
  {sizeof(imsrtp_codec_params_type_v01), imsrtp_codec_params_type_data_v01},
  {sizeof(imsrtp_amr_config_type_v01), imsrtp_amr_config_type_data_v01},
  {sizeof(imsrtp_h264_config_type_v01), imsrtp_h264_config_type_data_v01},
  {sizeof(imsrtp_session_params_type_v01), imsrtp_session_params_type_data_v01},
  {sizeof(imsrtp_rtcp_sr_type_v01), imsrtp_rtcp_sr_type_data_v01},
  {sizeof(imsrtp_rtcp_rr_type_v01), imsrtp_rtcp_rr_type_data_v01},
  {sizeof(imsrtp_rtcp_sdes_item_type_v01), imsrtp_rtcp_sdes_item_type_data_v01},
  {sizeof(imsrtp_video_config_type_v01), imsrtp_video_config_type_data_v01},
  {sizeof(imsrtp_video_h264_config_v01), imsrtp_video_h264_config_data_v01},
  {sizeof(imsrtp_video_mp4_config_v01), imsrtp_video_mp4_config_data_v01},
  {sizeof(imsrtp_video_h263_config_v01), imsrtp_video_h263_config_data_v01},
  {sizeof(imsrtp_video_time_info_type_v01), imsrtp_video_time_info_type_data_v01},
  {sizeof(imsrtp_rtcp_xr_voip_metrics_report_block_type_v01), imsrtp_rtcp_xr_voip_metrics_report_block_type_data_v01},
  {sizeof(imsrtp_rtcp_xr_dllr_report_block_type_v01), imsrtp_rtcp_xr_dllr_report_block_type_data_v01},
  {sizeof(imsrtp_rtcp_xr_statistics_summary_report_block_type_v01), imsrtp_rtcp_xr_statistics_summary_report_block_type_data_v01},
  {sizeof(imsrtp_video_capability_v01), imsrtp_video_capability_data_v01},
  {sizeof(imsrtp_text_config_params_v01), imsrtp_text_config_params_data_v01},
  {sizeof(imsrtp_srtp_info_v01), imsrtp_srtp_info_data_v01},
  {sizeof(qvp_rtcp_fb_messages_v01), qvp_rtcp_fb_messages_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry imsrtp_message_table_v01[] = {
  {sizeof(imsrtp_initialize_req_msg_v01), imsrtp_initialize_req_msg_data_v01},
  {sizeof(imsrtp_initialize_resp_msg_v01), imsrtp_initialize_resp_msg_data_v01},
  {sizeof(imsrtp_initialize_ind_v01), imsrtp_initialize_ind_data_v01},
  {sizeof(imsrtp_uninitialize_req_msg_v01), imsrtp_uninitialize_req_msg_data_v01},
  {sizeof(imsrtp_uninitialize_resp_msg_v01), imsrtp_uninitialize_resp_msg_data_v01},
  {sizeof(imsrtp_uninitialize_ind_v01), imsrtp_uninitialize_ind_data_v01},
  {sizeof(imsrtp_session_initialize_req_msg_v01), imsrtp_session_initialize_req_msg_data_v01},
  {sizeof(imsrtp_session_initialize_resp_msg_v01), imsrtp_session_initialize_resp_msg_data_v01},
  {sizeof(imsrtp_sesion_initialize_ind_v01), imsrtp_sesion_initialize_ind_data_v01},
  {sizeof(imsrtp_session_uninitialize_req_msg_v01), imsrtp_session_uninitialize_req_msg_data_v01},
  {sizeof(imsrtp_session_uninitialize_resp_msg_v01), imsrtp_session_uninitialize_resp_msg_data_v01},
  {sizeof(imsrtp_session_uninitialize_ind_v01), imsrtp_session_uninitialize_ind_data_v01},
  {sizeof(imsrtp_session_configure_req_msg_v01), imsrtp_session_configure_req_msg_data_v01},
  {sizeof(imsrtp_session_configure_resp_msg_v01), imsrtp_session_configure_resp_msg_data_v01},
  {sizeof(imsrtp_session_configure_ind_v01), imsrtp_session_configure_ind_data_v01},
  {sizeof(imsrtp_session_start_req_msg_v01), imsrtp_session_start_req_msg_data_v01},
  {sizeof(imsrtp_session_start_resp_msg_v01), imsrtp_session_start_resp_msg_data_v01},
  {sizeof(imsrtp_session_start_ind_v01), imsrtp_session_start_ind_data_v01},
  {sizeof(imsrtp_session_stop_req_msg_v01), imsrtp_session_stop_req_msg_data_v01},
  {sizeof(imsrtp_session_stop_resp_msg_v01), imsrtp_session_stop_resp_msg_data_v01},
  {sizeof(imsrtp_session_stop_ind_v01), imsrtp_session_stop_ind_data_v01},
  {sizeof(imsrtp_session_pause_req_msg_v01), imsrtp_session_pause_req_msg_data_v01},
  {sizeof(imsrtp_session_pause_resp_msg_v01), imsrtp_session_pause_resp_msg_data_v01},
  {sizeof(imsrtp_session_resume_req_msg_v01), imsrtp_session_resume_req_msg_data_v01},
  {sizeof(imsrtp_session_resume_resp_msg_v01), imsrtp_session_resume_resp_msg_data_v01},
  {sizeof(imsrtp_configure_rtcp_reports_req_msg_v01), imsrtp_configure_rtcp_reports_req_msg_data_v01},
  {sizeof(imsrtp_configure_rtcp_reports_resp_msg_v01), imsrtp_configure_rtcp_reports_resp_msg_data_v01},
  {sizeof(imsrtp_configure_rtcp_reports_ind_v01), imsrtp_configure_rtcp_reports_ind_data_v01},
  {sizeof(imsrtp_configure_rtp_link_monitoring_req_msg_v01), imsrtp_configure_rtp_link_monitoring_req_msg_data_v01},
  {sizeof(imsrtp_configure_rtp_link_monitoring_resp_msg_v01), imsrtp_configure_rtp_link_monitoring_resp_msg_data_v01},
  {sizeof(imsrtp_configure_rtp_link_monitoring_ind_v01), imsrtp_configure_rtp_link_monitoring_ind_data_v01},
  {sizeof(imsrtp_configure_rtcp_link_monitoring_req_msg_v01), imsrtp_configure_rtcp_link_monitoring_req_msg_data_v01},
  {sizeof(imsrtp_configure_rtcp_link_monitoring_resp_msg_v01), imsrtp_configure_rtcp_link_monitoring_resp_msg_data_v01},
  {sizeof(imsrtp_configure_rtcp_link_monitoring_ind_v01), imsrtp_configure_rtcp_link_monitoring_ind_data_v01},
  {sizeof(imsrtp_send_dtmf_req_msg_v01), imsrtp_send_dtmf_req_msg_data_v01},
  {sizeof(imsrtp_send_dtmf_resp_msg_v01), imsrtp_send_dtmf_resp_msg_data_v01},
  {sizeof(imsrtp_start_continuous_dtmf_req_msg_v01), imsrtp_start_continuous_dtmf_req_msg_data_v01},
  {sizeof(imsrtp_start_continuous_dtmf_resp_msg_v01), imsrtp_start_continuous_dtmf_resp_msg_data_v01},
  {sizeof(imsrtp_stop_continuous_dtmf_req_msg_v01), imsrtp_stop_continuous_dtmf_req_msg_data_v01},
  {sizeof(imsrtp_stop_continuous_dtmf_resp_msg_v01), imsrtp_stop_continuous_dtmf_resp_msg_data_v01},
  {sizeof(imsrtp_flush_audio_jitterbuffer_req_msg_v01), imsrtp_flush_audio_jitterbuffer_req_msg_data_v01},
  {sizeof(imsrtp_flush_audio_jitterbuffer_resp_msg_v01), imsrtp_flush_audio_jitterbuffer_resp_msg_data_v01},
  {sizeof(imsrtp_get_session_status_req_msg_v01), imsrtp_get_session_status_req_msg_data_v01},
  {sizeof(imsrtp_get_session_status_resp_msg_v01), imsrtp_get_session_status_resp_msg_data_v01},
  {sizeof(imsrtp_get_session_status_ind_v01), imsrtp_get_session_status_ind_data_v01},
  {sizeof(imsrtp_register_rtp_services_req_msg_v01), imsrtp_register_rtp_services_req_msg_data_v01},
  {sizeof(imsrtp_register_rtp_services_resp_msg_v01), imsrtp_register_rtp_services_resp_msg_data_v01},
  {sizeof(imsrtp_error_ind_v01), imsrtp_error_ind_data_v01},
  {sizeof(imsrtp_rtcp_reports_ind_v01), imsrtp_rtcp_reports_ind_data_v01},
  {sizeof(imsrtp_last_audio_play_time_req_msg_v01), imsrtp_last_audio_play_time_req_msg_data_v01},
  {sizeof(imsrtp_last_audio_play_time_resp_msg_v01), imsrtp_last_audio_play_time_resp_msg_data_v01},
  {sizeof(imsrtp_audio_report_req_msg_v01), imsrtp_audio_report_req_msg_data_v01},
  {sizeof(imsrtp_audio_report_resp_msg_v01), imsrtp_audio_report_resp_msg_data_v01},
  {sizeof(imsrtp_video_report_req_msg_v01), imsrtp_video_report_req_msg_data_v01},
  {sizeof(imsrtp_video_report_resp_msg_v01), imsrtp_video_report_resp_msg_data_v01},
  {sizeof(imsrtp_gen_h264_sps_pps_req_msg_v01), imsrtp_gen_h264_sps_pps_req_msg_data_v01},
  {sizeof(imsrtp_gen_h264_sps_pps_resp_msg_v01), imsrtp_gen_h264_sps_pps_resp_msg_data_v01},
  {sizeof(imsrtp_gen_mp4_vol_header_req_msg_v01), imsrtp_gen_mp4_vol_header_req_msg_data_v01},
  {sizeof(imsrtp_gen_mp4_vol_header_resp_msg_v01), imsrtp_gen_mp4_vol_header_resp_msg_data_v01},
  {sizeof(imsrtp_video_bitrate_adapt_req_msg_v01), imsrtp_video_bitrate_adapt_req_msg_data_v01},
  {sizeof(imsrtp_video_bitrate_adapt_resp_msg_v01), imsrtp_video_bitrate_adapt_resp_msg_data_v01},
  {0, 0},
  {sizeof(imsrtp_h264_idr_generate_resp_msg_v01), imsrtp_h264_idr_generate_resp_msg_data_v01},
  {sizeof(imsrtp_av_sync_feed_req_msg_v01), imsrtp_av_sync_feed_req_msg_data_v01},
  {sizeof(imsrtp_av_sync_feed_resp_msg_v01), imsrtp_av_sync_feed_resp_msg_data_v01},
  {sizeof(imsrtp_video_error_ind_msg_v01), imsrtp_video_error_ind_msg_data_v01},
  {sizeof(imsrtp_uninitialize_all_rtp_session_req_msg_v01), imsrtp_uninitialize_all_rtp_session_req_msg_data_v01},
  {sizeof(imsrtp_uninitialize_all_rtp_session_resp_msg_v01), imsrtp_uninitialize_all_rtp_session_resp_msg_data_v01},
  {sizeof(imsrtp_uninitialize_all_rtp_session_ind_v01), imsrtp_uninitialize_all_rtp_session_ind_data_v01},
  {sizeof(imsrtp_video_idr_generate_ind_msg_v01), imsrtp_video_idr_generate_ind_msg_data_v01},
  {sizeof(imsrtp_video_bitrate_adapt_ind_msg_v01), imsrtp_video_bitrate_adapt_ind_msg_data_v01},
  {sizeof(imsrtp_session_video_capability_req_msg_v01), imsrtp_session_video_capability_req_msg_data_v01},
  {sizeof(imsrtp_session_video_capability_resp_msg_v01), imsrtp_session_video_capability_resp_msg_data_v01},
  {sizeof(imsrtp_configure_nat_over_rtp_req_msg_v01), imsrtp_configure_nat_over_rtp_req_msg_data_v01},
  {sizeof(imsrtp_configure_nat_over_rtp_resp_msg_v01), imsrtp_configure_nat_over_rtp_resp_msg_data_v01},
  {sizeof(imsrtp_configure_nat_over_rtcp_req_msg_v01), imsrtp_configure_nat_over_rtcp_req_msg_data_v01},
  {sizeof(imsrtp_configure_nat_over_rtcp_resp_msg_v01), imsrtp_configure_nat_over_rtcp_resp_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object imsrtp_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *imsrtp_qmi_idl_type_table_object_referenced_tables_v01[] =
{&imsrtp_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object imsrtp_qmi_idl_type_table_object_v01 = {
  sizeof(imsrtp_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(imsrtp_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  imsrtp_type_table_v01,
  imsrtp_message_table_v01,
  imsrtp_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry imsrtp_service_command_messages_v01[] = {
  {QMI_IMSRTP_INITIALIZE_REQ_V01, QMI_IDL_TYPE16(0, 0), 7},
  {QMI_IMSRTP_UNINITIALIZE_REQ_V01, QMI_IDL_TYPE16(0, 3), 4},
  {QMI_IMSRTP_SESSION_INITIALIZE_REQ_V01, QMI_IDL_TYPE16(0, 6), 1755},
  {QMI_IMSRTP_SESSION_UNINITIALIZE_REQ_V01, QMI_IDL_TYPE16(0, 9), 4},
  {QMI_IMSRTP_SESSION_CONFIGURE_REQ_V01, QMI_IDL_TYPE16(0, 12), 1628},
  {QMI_IMSRTP_SESSION_START_REQ_V01, QMI_IDL_TYPE16(0, 15), 15},
  {QMI_IMSRTP_SESSION_STOP_REQ_V01, QMI_IDL_TYPE16(0, 18), 15},
  {QMI_IMSRTP_SESSION_PAUSE_REQ_V01, QMI_IDL_TYPE16(0, 21), 15},
  {QMI_IMSRTP_SESSION_RESUME_REQ_V01, QMI_IDL_TYPE16(0, 23), 15},
  {QMI_IMSRTP_CONFIGURE_RTCP_REPORTS_REQ_V01, QMI_IDL_TYPE16(0, 25), 13},
  {QMI_IMSRTP_CONFIGURE_RTP_LINK_MONITOR_REQ_V01, QMI_IDL_TYPE16(0, 28), 13},
  {QMI_IMSRTP_CONFIGURE_RTCP_LINK_MONITOR_REQ_V01, QMI_IDL_TYPE16(0, 31), 13},
  {QMI_IMSRTP_SEND_DTMF_REQ_V01, QMI_IDL_TYPE16(0, 34), 17},
  {QMI_IMSRTP_START_CONTINUOUS_DTMF_REQ_V01, QMI_IDL_TYPE16(0, 36), 12},
  {QMI_IMSRTP_STOP_CONTINUOUS_DTMF_REQ_V01, QMI_IDL_TYPE16(0, 38), 4},
  {QMI_IMSRTP_FLUSH_AUDIO_JITTER_BUFFER_REQ_V01, QMI_IDL_TYPE16(0, 40), 4},
  {QMI_IMSRTP_REGISTER_RTP_SERVICES_REQ_V01, QMI_IDL_TYPE16(0, 45), 15},
  {QMI_IMSRTP_GET_SESSION_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 42), 4},
  {QMI_IMS_RTP_LAST_AUDIO_PLAY_TIME_REQ_V01, QMI_IDL_TYPE16(0, 49), 7},
  {QMI_IMS_RTP_AUDIO_REPORT_REQ_V01, QMI_IDL_TYPE16(0, 51), 15},
  {QMI_IMS_RTP_VIDEO_REPORT_REQ_V01, QMI_IDL_TYPE16(0, 53), 15},
  {QMI_IMS_RTP_VIDEO_BIT_RATE_ADAPT_REQ_V01, QMI_IDL_TYPE16(0, 59), 7},
  {QMI_IMS_RTP_H264_IDR_GENERATE_REQ_V01, QMI_IDL_TYPE16(0, 61), 0},
  {QMI_IMS_RTP_AV_SYNC_FEED_REQ_V01, QMI_IDL_TYPE16(0, 63), 11},
  {QMI_IMS_RTP_GEN_H264_SPS_PPS_REQ_V01, QMI_IDL_TYPE16(0, 55), 311},
  {QMI_IMS_RTP_GEN_MP4_VOL_HEADER_REQ_V01, QMI_IDL_TYPE16(0, 57), 307},
  {QMI_IMSRTP_UNINITIALIZE_ALL_RTP_SESSION_REQ_V01, QMI_IDL_TYPE16(0, 66), 4},
  {QMI_IMSRTP_SESSION_VIDEO_CAPABILITY_REQ_V01, QMI_IDL_TYPE16(0, 71), 46},
  {QMI_IMSRTP_CONFIGURE_NAT_OVER_RTP_REQ_V01, QMI_IDL_TYPE16(0, 73), 13},
  {QMI_IMSRTP_CONFIGURE_NAT_OVER_RTCP_REQ_V01, QMI_IDL_TYPE16(0, 75), 13}
};

static const qmi_idl_service_message_table_entry imsrtp_service_response_messages_v01[] = {
  {QMI_IMSRTP_INITIALIZE_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_IMSRTP_UNINITIALIZE_RESP_V01, QMI_IDL_TYPE16(0, 4), 7},
  {QMI_IMSRTP_SESSION_INITIALIZE_RESP_V01, QMI_IDL_TYPE16(0, 7), 7},
  {QMI_IMSRTP_SESSION_UNINITIALIZE_RESP_V01, QMI_IDL_TYPE16(0, 10), 7},
  {QMI_IMSRTP_SESSION_CONFIGURE_RESP_V01, QMI_IDL_TYPE16(0, 13), 7},
  {QMI_IMSRTP_SESSION_START_RESP_V01, QMI_IDL_TYPE16(0, 16), 7},
  {QMI_IMSRTP_SESSION_STOP_RESP_V01, QMI_IDL_TYPE16(0, 19), 7},
  {QMI_IMSRTP_SESSION_PAUSE_RESP_V01, QMI_IDL_TYPE16(0, 22), 7},
  {QMI_IMSRTP_SESSION_RESUME_RESP_V01, QMI_IDL_TYPE16(0, 24), 7},
  {QMI_IMSRTP_CONFIGURE_RTCP_REPORTS_RESP_V01, QMI_IDL_TYPE16(0, 26), 7},
  {QMI_IMSRTP_CONFIGURE_RTP_LINK_MONITOR_RESP_V01, QMI_IDL_TYPE16(0, 29), 7},
  {QMI_IMSRTP_CONFIGURE_RTCP_LINK_MONITOR_RESP_V01, QMI_IDL_TYPE16(0, 32), 7},
  {QMI_IMSRTP_SEND_DTMF_RESP_V01, QMI_IDL_TYPE16(0, 35), 7},
  {QMI_IMSRTP_START_CONTINUOUS_DTMF_RESP_V01, QMI_IDL_TYPE16(0, 37), 7},
  {QMI_IMSRTP_STOP_CONTINUOUS_DTMF_RESP_V01, QMI_IDL_TYPE16(0, 39), 7},
  {QMI_IMSRTP_FLUSH_AUDIO_JITTER_BUFFER_RESP_V01, QMI_IDL_TYPE16(0, 41), 7},
  {QMI_IMSRTP_REGISTER_RTP_SERVICES_RESP_V01, QMI_IDL_TYPE16(0, 46), 7},
  {QMI_IMSRTP_GET_SESSION_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 43), 7},
  {QMI_IMS_RTP_LAST_AUDIO_PLAY_TIME_RESP_V01, QMI_IDL_TYPE16(0, 50), 7},
  {QMI_IMS_RTP_AUDIO_REPORT_RESP_V01, QMI_IDL_TYPE16(0, 52), 7},
  {QMI_IMS_RTP_VIDEO_REPORT_RESP_V01, QMI_IDL_TYPE16(0, 54), 7},
  {QMI_IMS_RTP_VIDEO_BIT_RATE_ADAPT_RESP_V01, QMI_IDL_TYPE16(0, 60), 7},
  {QMI_IMS_RTP_H264_IDR_GENERATE_RESP_V01, QMI_IDL_TYPE16(0, 62), 7},
  {QMI_IMS_RTP_AV_SYNC_FEED_RESP_V01, QMI_IDL_TYPE16(0, 64), 7},
  {QMI_IMS_RTP_GEN_H264_SPS_PPS_RESP_V01, QMI_IDL_TYPE16(0, 56), 266},
  {QMI_IMS_RTP_GEN_MP4_VOL_HEADER_RESP_V01, QMI_IDL_TYPE16(0, 58), 266},
  {QMI_IMSRTP_UNINITIALIZE_ALL_RTP_SESSION_RESP_V01, QMI_IDL_TYPE16(0, 67), 7},
  {QMI_IMSRTP_SESSION_VIDEO_CAPABILITY_RESP_V01, QMI_IDL_TYPE16(0, 72), 7},
  {QMI_IMSRTP_CONFIGURE_NAT_OVER_RTP_RESP_V01, QMI_IDL_TYPE16(0, 74), 7},
  {QMI_IMSRTP_CONFIGURE_NAT_OVER_RTCP_RESP_V01, QMI_IDL_TYPE16(0, 76), 7}
};

static const qmi_idl_service_message_table_entry imsrtp_service_indication_messages_v01[] = {
  {QMI_IMSRTP_INITIALIZE_IND_V01, QMI_IDL_TYPE16(0, 2), 11},
  {QMI_IMSRTP_UNINITIALIZE_IND_V01, QMI_IDL_TYPE16(0, 5), 11},
  {QMI_IMSRTP_SESSION_INITIALIZE_IND_V01, QMI_IDL_TYPE16(0, 8), 33},
  {QMI_IMSRTP_SESSION_UNINITIALIZE_IND_V01, QMI_IDL_TYPE16(0, 11), 11},
  {QMI_IMSRTP_SESSION_CONFIGURE_IND_V01, QMI_IDL_TYPE16(0, 14), 29},
  {QMI_IMSRTP_SESSION_START_IND_V01, QMI_IDL_TYPE16(0, 17), 22},
  {QMI_IMSRTP_SESSION_STOP_IND_V01, QMI_IDL_TYPE16(0, 20), 22},
  {QMI_IMSRTP_CONFIGURE_RTCP_REPORTS_IND_V01, QMI_IDL_TYPE16(0, 27), 11},
  {QMI_IMSRTP_CONFIGURE_RTP_LINK_MONITORING_IND_V01, QMI_IDL_TYPE16(0, 30), 11},
  {QMI_IMSRTP_CONFIGURE_RTCP_LINK_MONITORING_IND_V01, QMI_IDL_TYPE16(0, 33), 11},
  {QMI_IMSRTP_GET_SESSION_STATUS_IND_V01, QMI_IDL_TYPE16(0, 44), 19},
  {QMI_IMSRTP_ERROR_IND_V01, QMI_IDL_TYPE16(0, 47), 11},
  {QMI_IMSRTP_RTCP_REPORTS_IND_V01, QMI_IDL_TYPE16(0, 48), 458},
  {QMI_IMS_RTP_VIDEO_ERROR_IND_V01, QMI_IDL_TYPE16(0, 65), 7},
  {QMI_IMSRTP_UNINITIALIZE_ALL_RTP_SESSION_IND_V01, QMI_IDL_TYPE16(0, 68), 11},
  {QMI_IMS_RTP_VIDEO_IDR_GENERATE_IND_V01, QMI_IDL_TYPE16(0, 69), 4},
  {QMI_IMS_RTP_VIDEO_BIT_RATE_ADAPT_IND_V01, QMI_IDL_TYPE16(0, 70), 11}
};

/*Service Object*/
struct qmi_idl_service_object imsrtp_qmi_idl_service_object_v01 = {
  0x05,
  0x01,
  0x28,
  1755,
  { sizeof(imsrtp_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsrtp_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsrtp_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { imsrtp_service_command_messages_v01, imsrtp_service_response_messages_v01, imsrtp_service_indication_messages_v01},
  &imsrtp_qmi_idl_type_table_object_v01,
  0x10,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type imsrtp_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( IMSRTP_V01_IDL_MAJOR_VERS != idl_maj_version || IMSRTP_V01_IDL_MINOR_VERS != idl_min_version 
       || IMSRTP_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&imsrtp_qmi_idl_service_object_v01;
}

