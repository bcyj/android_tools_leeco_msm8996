/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        I P _ M U L T I M E D I A _ S U B S Y S T E M _ V I D E O _ T E L E P H O N Y _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the imsvt service Data structures.

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved. 
 Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/interfaces/qmi/imsvt/main/latest/src/ip_multimedia_subsystem_video_telephony_v01.c#6 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====* 
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY 
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 5.5 
   It was generated on: Thu Sep 27 2012
   From IDL File: ip_multimedia_subsystem_video_telephony_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "ip_multimedia_subsystem_video_telephony_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t imsvt_h264_parameters_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_h264_parameters_v01, profile_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_h264_parameters_v01, profile_level),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_mp4_parameters_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_mp4_parameters_v01, profile_level_id),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_config_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, codec),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, width),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, height),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, bit_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, frame_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, clock_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, lipsynch_drop_upper_limit),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, lipsynch_drop_lower_limit),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsvt_config_type_v01, lip_sync_enable),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_h264_config_type_data_v01[] = {
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_h264_config_type_v01, h264_config),
 0, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_mp4_config_type_data_v01[] = {
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_mp4_config_type_v01, mp4_config),
 1, 0,
  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_time_info_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_time_info_type_v01, n_imsntp_time),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_time_info_type_v01, n_ilsntp_time),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_time_info_type_v01, n_irtp_time_stamp),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_vol_header_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsvt_vol_header_info_type_v01, volheader),
  IMSVT_VOL_HEADER_MAX_V01,
  QMI_IDL_OFFSET8(imsvt_vol_header_info_type_v01, volheader) - QMI_IDL_OFFSET8(imsvt_vol_header_info_type_v01, volheader_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_nal_header_info_type_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsvt_nal_header_info_type_v01, nalheader),
  IMSVT_NAL_HEADER_MAX_V01,
  QMI_IDL_OFFSET8(imsvt_nal_header_info_type_v01, nalheader) - QMI_IDL_OFFSET8(imsvt_nal_header_info_type_v01, nalheader_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_addr_type_data_v01[] = {
  QMI_IDL_GENERIC_2_BYTE,
  QMI_IDL_OFFSET8(imsvt_addr_type_v01, port),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_addr_type_v01, iptype),

  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(imsvt_addr_type_v01, ipaddr),
  IMSVT_IP_ADDR_MAX_V01,
  QMI_IDL_OFFSET8(imsvt_addr_type_v01, ipaddr) - QMI_IDL_OFFSET8(imsvt_addr_type_v01, ipaddr_len),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_audio_ls_parameters_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_audio_ls_parameters_v01, audio_clock_rate),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_audio_ls_parameters_v01, audio_packet_interval),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_h263_config_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_h263_config_type_v01, profile_type),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_h263_config_type_v01, profile_level),

  QMI_IDL_FLAG_END_VALUE
};

static const uint8_t imsvt_video_codec_error_type_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_video_codec_error_type_v01, error_value),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t imsvt_ack_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_ack_msg_v01, resp),
  0, 1
};

static const uint8_t imsvt_initialize_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, video_codec_config),
  2, 0,

  0x02,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, video_device),

  0x03,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, ip_addr),
  8, 0,

  0x04,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, audio_param),
  9, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, h264_config) - QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, h264_config_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, h264_config),
  3, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, mp4_config) - QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, mp4_config_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, mp4_config),
  4, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, nal_header) - QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, nal_header_valid)),
  0x12,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_initialize_req_msg_v01, nal_header),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsvt_initialize_req_msg_v01, vol_header) - QMI_IDL_OFFSET16RELATIVE(imsvt_initialize_req_msg_v01, vol_header_valid)),
  0x13,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsvt_initialize_req_msg_v01, vol_header),
  6, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsvt_initialize_req_msg_v01, h263_config) - QMI_IDL_OFFSET16RELATIVE(imsvt_initialize_req_msg_v01, h263_config_valid)),
  0x14,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsvt_initialize_req_msg_v01, h263_config),
  10, 0
};

static const uint8_t imsvt_initialize_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_initialize_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_initialize_resp_msg_v01, video_device) - QMI_IDL_OFFSET8(imsvt_initialize_resp_msg_v01, video_device_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_initialize_resp_msg_v01, video_device),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_initialize_resp_msg_v01, ip_addr) - QMI_IDL_OFFSET8(imsvt_initialize_resp_msg_v01, ip_addr_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_initialize_resp_msg_v01, ip_addr),
  8, 0
};

static const uint8_t imsvt_uninitialize_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_uninitialize_req_msg_v01, video_device)
};

static const uint8_t imsvt_uninitialize_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_uninitialize_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_uninitialize_resp_msg_v01, video_device) - QMI_IDL_OFFSET8(imsvt_uninitialize_resp_msg_v01, video_device_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_uninitialize_resp_msg_v01, video_device)
};

static const uint8_t imsvt_codec_config_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, video_device),

  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, video_codec_config),
  2, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, h264_config) - QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, h264_config_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, h264_config),
  3, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, mp4_config) - QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, mp4_config_valid)),
  0x11,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, mp4_config),
  4, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, nal_header) - QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, nal_header_valid)),
  0x12,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_codec_config_req_msg_v01, nal_header),
  7, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsvt_codec_config_req_msg_v01, vol_header) - QMI_IDL_OFFSET16RELATIVE(imsvt_codec_config_req_msg_v01, vol_header_valid)),
  0x13,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsvt_codec_config_req_msg_v01, vol_header),
  6, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsvt_codec_config_req_msg_v01, h263_config) - QMI_IDL_OFFSET16RELATIVE(imsvt_codec_config_req_msg_v01, h263_config_valid)),
  0x14,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsvt_codec_config_req_msg_v01, h263_config),
  10, 0,

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsvt_codec_config_req_msg_v01, max_nalu_size) - QMI_IDL_OFFSET16RELATIVE(imsvt_codec_config_req_msg_v01, max_nalu_size_valid)),
  0x15,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET16ARRAY(imsvt_codec_config_req_msg_v01, max_nalu_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET16RELATIVE(imsvt_codec_config_req_msg_v01, audio_param) - QMI_IDL_OFFSET16RELATIVE(imsvt_codec_config_req_msg_v01, audio_param_valid)),
  0x16,
  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET16ARRAY(imsvt_codec_config_req_msg_v01, audio_param),
  9, 0
};

static const uint8_t imsvt_codec_config_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_codec_config_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_codec_config_resp_msg_v01, video_device) - QMI_IDL_OFFSET8(imsvt_codec_config_resp_msg_v01, video_device_valid)),
  0x10,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_codec_config_resp_msg_v01, video_device)
};

/* 
 * imsvt_record_stop_req_msg is empty
 * static const uint8_t imsvt_record_stop_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsvt_record_stop_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_record_stop_resp_msg_v01, resp),
  0, 1
};

/* 
 * imsvt_record_start_req_msg is empty
 * static const uint8_t imsvt_record_start_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsvt_record_start_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_record_start_resp_msg_v01, resp),
  0, 1
};

/* 
 * imsvt_play_stop_req_msg is empty
 * static const uint8_t imsvt_play_stop_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsvt_play_stop_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_play_stop_resp_msg_v01, resp),
  0, 1
};

/* 
 * imsvt_play_start_req_msg is empty
 * static const uint8_t imsvt_play_start_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsvt_play_start_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_play_start_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsvt_last_audio_play_time_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_last_audio_play_time_req_msg_v01, last_play_time_stamp)
};

static const uint8_t imsvt_last_audio_play_time_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_last_audio_play_time_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsvt_audio_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_audio_report_req_msg_v01, time_info),
  5, 0
};

static const uint8_t imsvt_audio_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_audio_report_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsvt_video_report_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_video_report_req_msg_v01, time_info),
  5, 0
};

static const uint8_t imsvt_video_report_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_video_report_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsvt_gen_h264_sps_pps_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_gen_h264_sps_pps_req_msg_v01, video_codec_config),
  2, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_gen_h264_sps_pps_req_msg_v01, h264_config),
  3, 0
};

static const uint8_t imsvt_gen_h264_sps_pps_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_gen_h264_sps_pps_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_gen_h264_sps_pps_resp_msg_v01, nal_header) - QMI_IDL_OFFSET8(imsvt_gen_h264_sps_pps_resp_msg_v01, nal_header_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_gen_h264_sps_pps_resp_msg_v01, nal_header),
  7, 0
};

static const uint8_t imsvt_gen_mp4_vol_header_req_msg_data_v01[] = {
  0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_gen_mp4_vol_header_req_msg_v01, video_codec_config),
  2, 0,

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_gen_mp4_vol_header_req_msg_v01, mp4_config),
  4, 0
};

static const uint8_t imsvt_gen_mp4_vol_header_resp_msg_data_v01[] = {
  0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_gen_mp4_vol_header_resp_msg_v01, resp),
  0, 1,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(imsvt_gen_mp4_vol_header_resp_msg_v01, vol_header) - QMI_IDL_OFFSET8(imsvt_gen_mp4_vol_header_resp_msg_v01, vol_header_valid)),
  0x10,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_gen_mp4_vol_header_resp_msg_v01, vol_header),
  6, 0
};

static const uint8_t imsvt_video_bitrate_adapt_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(imsvt_video_bitrate_adapt_req_msg_v01, bit_rate)
};

static const uint8_t imsvt_video_bitrate_adapt_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_video_bitrate_adapt_resp_msg_v01, resp),
  0, 1
};

/* 
 * imsvt_h264_idr_generate_req_msg is empty
 * static const uint8_t imsvt_h264_idr_generate_req_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsvt_h264_idr_generate_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_h264_idr_generate_resp_msg_v01, resp),
  0, 1
};

static const uint8_t imsvt_av_sync_feed_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(imsvt_av_sync_feed_req_msg_v01, n_iavsync_time)
};

static const uint8_t imsvt_av_sync_feed_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_av_sync_feed_resp_msg_v01, resp),
  0, 1
};

/* 
 * imsvt_service_available_ind_msg is empty
 * static const uint8_t imsvt_service_available_ind_msg_data_v01[] = {
 * };
 */
  
static const uint8_t imsvt_video_error_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(imsvt_video_error_ind_msg_v01, video_codec_error),
  11, 0
};

/* Type Table */
static const qmi_idl_type_table_entry  imsvt_type_table_v01[] = {
  {sizeof(imsvt_h264_parameters_v01), imsvt_h264_parameters_data_v01},
  {sizeof(imsvt_mp4_parameters_v01), imsvt_mp4_parameters_data_v01},
  {sizeof(imsvt_config_type_v01), imsvt_config_type_data_v01},
  {sizeof(imsvt_h264_config_type_v01), imsvt_h264_config_type_data_v01},
  {sizeof(imsvt_mp4_config_type_v01), imsvt_mp4_config_type_data_v01},
  {sizeof(imsvt_time_info_type_v01), imsvt_time_info_type_data_v01},
  {sizeof(imsvt_vol_header_info_type_v01), imsvt_vol_header_info_type_data_v01},
  {sizeof(imsvt_nal_header_info_type_v01), imsvt_nal_header_info_type_data_v01},
  {sizeof(imsvt_addr_type_v01), imsvt_addr_type_data_v01},
  {sizeof(imsvt_audio_ls_parameters_v01), imsvt_audio_ls_parameters_data_v01},
  {sizeof(imsvt_h263_config_type_v01), imsvt_h263_config_type_data_v01},
  {sizeof(imsvt_video_codec_error_type_v01), imsvt_video_codec_error_type_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry imsvt_message_table_v01[] = {
  {sizeof(imsvt_ack_msg_v01), imsvt_ack_msg_data_v01},
  {sizeof(imsvt_initialize_req_msg_v01), imsvt_initialize_req_msg_data_v01},
  {sizeof(imsvt_initialize_resp_msg_v01), imsvt_initialize_resp_msg_data_v01},
  {sizeof(imsvt_uninitialize_req_msg_v01), imsvt_uninitialize_req_msg_data_v01},
  {sizeof(imsvt_uninitialize_resp_msg_v01), imsvt_uninitialize_resp_msg_data_v01},
  {sizeof(imsvt_codec_config_req_msg_v01), imsvt_codec_config_req_msg_data_v01},
  {sizeof(imsvt_codec_config_resp_msg_v01), imsvt_codec_config_resp_msg_data_v01},
  {0, 0},
  {sizeof(imsvt_record_stop_resp_msg_v01), imsvt_record_stop_resp_msg_data_v01},
  {0, 0},
  {sizeof(imsvt_record_start_resp_msg_v01), imsvt_record_start_resp_msg_data_v01},
  {0, 0},
  {sizeof(imsvt_play_stop_resp_msg_v01), imsvt_play_stop_resp_msg_data_v01},
  {0, 0},
  {sizeof(imsvt_play_start_resp_msg_v01), imsvt_play_start_resp_msg_data_v01},
  {sizeof(imsvt_last_audio_play_time_req_msg_v01), imsvt_last_audio_play_time_req_msg_data_v01},
  {sizeof(imsvt_last_audio_play_time_resp_msg_v01), imsvt_last_audio_play_time_resp_msg_data_v01},
  {sizeof(imsvt_audio_report_req_msg_v01), imsvt_audio_report_req_msg_data_v01},
  {sizeof(imsvt_audio_report_resp_msg_v01), imsvt_audio_report_resp_msg_data_v01},
  {sizeof(imsvt_video_report_req_msg_v01), imsvt_video_report_req_msg_data_v01},
  {sizeof(imsvt_video_report_resp_msg_v01), imsvt_video_report_resp_msg_data_v01},
  {sizeof(imsvt_gen_h264_sps_pps_req_msg_v01), imsvt_gen_h264_sps_pps_req_msg_data_v01},
  {sizeof(imsvt_gen_h264_sps_pps_resp_msg_v01), imsvt_gen_h264_sps_pps_resp_msg_data_v01},
  {sizeof(imsvt_gen_mp4_vol_header_req_msg_v01), imsvt_gen_mp4_vol_header_req_msg_data_v01},
  {sizeof(imsvt_gen_mp4_vol_header_resp_msg_v01), imsvt_gen_mp4_vol_header_resp_msg_data_v01},
  {sizeof(imsvt_video_bitrate_adapt_req_msg_v01), imsvt_video_bitrate_adapt_req_msg_data_v01},
  {sizeof(imsvt_video_bitrate_adapt_resp_msg_v01), imsvt_video_bitrate_adapt_resp_msg_data_v01},
  {0, 0},
  {sizeof(imsvt_h264_idr_generate_resp_msg_v01), imsvt_h264_idr_generate_resp_msg_data_v01},
  {sizeof(imsvt_av_sync_feed_req_msg_v01), imsvt_av_sync_feed_req_msg_data_v01},
  {sizeof(imsvt_av_sync_feed_resp_msg_v01), imsvt_av_sync_feed_resp_msg_data_v01},
  {0, 0},
  {sizeof(imsvt_video_error_ind_msg_v01), imsvt_video_error_ind_msg_data_v01}
};

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object imsvt_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *imsvt_qmi_idl_type_table_object_referenced_tables_v01[] =
{&imsvt_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object imsvt_qmi_idl_type_table_object_v01 = {
  sizeof(imsvt_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(imsvt_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  imsvt_type_table_v01,
  imsvt_message_table_v01,
  imsvt_qmi_idl_type_table_object_referenced_tables_v01
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry imsvt_service_command_messages_v01[] = {
  {QMI_IMSVT_INITIALIZE_RESP_V01, TYPE16(0, 2), 64},
  {QMI_IMSVT_UNINITIALIZE_RESP_V01, TYPE16(0, 4), 14},
  {QMI_IMSVT_CODEC_CONFIG_RESP_V01, TYPE16(0, 6), 14},
  {QMI_IMSVT_RECORD_STOP_RESP_V01, TYPE16(0, 8), 7},
  {QMI_IMSVT_RECORD_START_RESP_V01, TYPE16(0, 10), 7},
  {QMI_IMSVT_PLAY_STOP_RESP_V01, TYPE16(0, 12), 7},
  {QMI_IMSVT_PLAY_START_RESP_V01, TYPE16(0, 14), 7},
  {QMI_IMSVT_GEN_H264_SPS_PPS_RESP_V01, TYPE16(0, 22), 266},
  {QMI_IMSVT_GEN_MP4_VOL_HEADER_RESP_V01, TYPE16(0, 24), 266},
  {QMI_IMSVT_LAST_AUDIO_PLAY_TIME_RESP_V01, TYPE16(0, 16), 7},
  {QMI_IMSVT_AUDIO_REPORT_RESP_V01, TYPE16(0, 18), 7},
  {QMI_IMSVT_VIDEO_REPORT_RESP_V01, TYPE16(0, 20), 7},
  {QMI_IMSVT_VIDEO_BIT_RATE_ADAPT_RESP_V01, TYPE16(0, 26), 7},
  {QMI_IMSVT_H264_IDR_GENERATE_RESP_V01, TYPE16(0, 28), 7},
  {QMI_IMSVT_AV_SYNC_FEED_RESP_V01, TYPE16(0, 30), 7},
  {QMI_IMSVT_SERVICE_AVAILABLE_IND_V01, TYPE16(0, 31), 0},
  {QMI_IMSVT_VIDEO_ERROR_IND_V01, TYPE16(0, 32), 7}
};

static const qmi_idl_service_message_table_entry imsvt_service_response_messages_v01[] = {
  {QMI_IMSVT_INITIALIZE_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_UNINITIALIZE_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_CODEC_CONFIG_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_RECORD_STOP_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_RECORD_START_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_PLAY_STOP_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_PLAY_START_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_GEN_H264_SPS_PPS_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_GEN_MP4_VOL_HEADER_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_LAST_AUDIO_PLAY_TIME_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_AUDIO_REPORT_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_VIDEO_REPORT_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_VIDEO_BIT_RATE_ADAPT_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_H264_IDR_GENERATE_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_AV_SYNC_FEED_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_SERVICE_AVAILABLE_ACK_V01, TYPE16(0, 0), 7},
  {QMI_IMSVT_VIDEO_ERROR_ACK_V01, TYPE16(0, 0), 7}
};

static const qmi_idl_service_message_table_entry imsvt_service_indication_messages_v01[] = {
  {QMI_IMSVT_INITIALIZE_REQ_V01, TYPE16(0, 1), 651},
  {QMI_IMSVT_UNINITIALIZE_REQ_V01, TYPE16(0, 3), 7},
  {QMI_IMSVT_CODEC_CONFIG_REQ_V01, TYPE16(0, 5), 608},
  {QMI_IMSVT_RECORD_STOP_REQ_V01, TYPE16(0, 7), 0},
  {QMI_IMSVT_RECORD_START_REQ_V01, TYPE16(0, 9), 0},
  {QMI_IMSVT_PLAY_STOP_REQ_V01, TYPE16(0, 11), 0},
  {QMI_IMSVT_PLAY_START_REQ_V01, TYPE16(0, 13), 0},
  {QMI_IMSVT_GEN_H264_SPS_PPS_REQ_V01, TYPE16(0, 21), 47},
  {QMI_IMSVT_GEN_MP4_VOL_HEADER_REQ_V01, TYPE16(0, 23), 43},
  {QMI_IMSVT_LAST_AUDIO_PLAY_TIME_REQ_V01, TYPE16(0, 15), 7},
  {QMI_IMSVT_AUDIO_REPORT_REQ_V01, TYPE16(0, 17), 15},
  {QMI_IMSVT_VIDEO_REPORT_REQ_V01, TYPE16(0, 19), 15},
  {QMI_IMSVT_VIDEO_BIT_RATE_ADAPT_REQ_V01, TYPE16(0, 25), 7},
  {QMI_IMSVT_H264_IDR_GENERATE_REQ_V01, TYPE16(0, 27), 0},
  {QMI_IMSVT_AV_SYNC_FEED_REQ_V01, TYPE16(0, 29), 11}
};

/*Service Object*/
struct qmi_idl_service_object imsvt_qmi_idl_service_object_v01 = {
  0x05,
  0x01,
  0x20,
  651,
  { sizeof(imsvt_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsvt_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsvt_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { imsvt_service_command_messages_v01, imsvt_service_response_messages_v01, imsvt_service_indication_messages_v01},
  &imsvt_qmi_idl_type_table_object_v01,
  0x02,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type imsvt_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( IMSVT_V01_IDL_MAJOR_VERS != idl_maj_version || IMSVT_V01_IDL_MINOR_VERS != idl_min_version 
       || IMSVT_V01_IDL_TOOL_VERS != library_version) 
  {
    return NULL;
  } 
  return (qmi_idl_service_object_type)&imsvt_qmi_idl_service_object_v01;
}

