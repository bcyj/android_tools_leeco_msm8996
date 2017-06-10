/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_CHANNEL_UTIL_H__
#define __ISP_CHANNEL_UTIL_H__

#define MAX_NUM_CONTINUOS_CH_LIVESHOT 1
isp_channel_t *isp_ch_util_add_channel(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  uint32_t user_stream_idx,
  mct_stream_info_t *stream_info,
  isp_channel_type_t channel_type);
int isp_ch_util_del_channel(
  isp_session_t *session,
  isp_channel_t *channel);
int isp_ch_util_get_channel_idx(
  isp_channel_t *channel);
int isp_ch_util_sync_stream_cfg_to_channel(
  isp_t *isp,
  isp_session_t *session,
  isp_stream_t *stream);
isp_channel_t *isp_ch_util_find_channel_in_session(
  isp_session_t *sess,
  uint32_t channel_id);
isp_channel_t *isp_ch_util_find_channel_in_session_by_idx(
  isp_session_t *sess,
  int idx);
int isp_ch_util_check_min_width_height(
  isp_sink_port_t *sink_port,
  sensor_out_info_t *sensor_output,
  int32_t *width,
  int32_t *height,
  boolean *use_native_buf);
int isp_ch_util_del_channel_by_mask(
  isp_session_t *session,
  uint32_t mask);
boolean isp_ch_util_is_right_stripe_offset_usable(
  uint32_t M,
  uint32_t N,
  uint32_t offset);
int isp_ch_util_adjust_crop_factor(
  isp_session_t *session,
  uint32_t old_crop_factor,
  uint32_t *new_crop_factor);
int isp_ch_util_config_for_yuv_sensor(
  isp_t *isp, isp_session_t *session);
int isp_ch_util_select_pipeline_channel(
  isp_t *isp,
  isp_session_t *session);
int isp_ch_util_compute_stripe_info_of_channel(
  isp_t *isp,
  isp_session_t *session,
  uint32_t initial_overlap_half);
int isp_ch_util_request_channel_image_buf(
  isp_t *isp,
  isp_session_t *session,
  int num_channel,
  isp_channel_t *channel[ISP_MAX_STREAMS]);
void isp_ch_util_release_channel_image_buf(
  isp_t *isp,
  isp_session_t *session,
  int num_channel,
  isp_channel_t *channel[ISP_MAX_STREAMS]);
int isp_ch_util_prepare_hw_config_for_streamon(
  isp_t *isp,
  isp_session_t *session);
int isp_ch_util_streamon(
  isp_t *isp,
  isp_session_t *session,
  int num_channels,
  uint32_t *channel_ids);
int isp_ch_util_streamoff(
  isp_t *isp,
  isp_session_t *session,
  int num_channels,
  uint32_t *channel_ids,
  boolean stop_immediately);
int isp_ch_util_unconfig_channel(
  isp_t *isp,
  int isp_id,
  isp_channel_t *channel);
int isp_ch_util_hw_notify_hw_updating(
  isp_t *isp,
  isp_hw_updating_notify_t *notify_data);
int isp_ch_util_hw_notify_sof(isp_t *isp, mct_bus_msg_t *bus_msg,
  void *ctrl, uint32_t isp_id);
int isp_ch_util_hw_notify_meta_valid(isp_t *isp, mct_bus_msg_t *bus_msg);
int isp_ch_util_buf_divert_notify(
  isp_t *isp,
  isp_frame_divert_notify_t *divert_event);
void isp_ch_util_convert_crop_to_stream(
  isp_session_t *session,
  isp_stream_t *user_stream,
  mct_bus_msg_stream_crop_t *stream_crop,
  isp_zoom_scaling_param_entry_t *entry,isp_t *isp);
isp_channel_t *isp_ch_util_get_image_channel(
  isp_session_t *session,
  uint32_t channel_mask);
void isp_ch_util_dump_channel_planes(isp_channel_t *channel);
int isp_ch_util_divert_ack(
  isp_t *isp,
  isp_session_t *session,
  isp_buf_divert_ack_t *divert_ack);
int isp_ch_util_set_param(
  isp_t *isp,
  isp_session_t *session,
  int stream_id,
  isp_hw_set_params_id_t param_id,
  void *data,
  uint32_t size);
int isp_ch_util_get_param(
  isp_t *isp,
  isp_session_t *session,
  int stream_id,
  isp_hw_set_params_id_t param_id,
  void *in_data,
  uint32_t in_size,
  void *out_data,
  uint32_t out_size);
void isp_ch_util_all_streams_off(
  isp_t *isp,
  isp_session_t *session);
int isp_ch_util_reg_buf_list_update(isp_t *isp, isp_session_t *session,
  int stream_id, mct_stream_map_buf_t * map_buffer_list);
enum msm_vfe_frame_skip_pattern isp_ch_util_get_hfr_skip_pattern(
  isp_session_t *session);
#endif /* __ISP_CHANNEL_UTIL_H__ */

