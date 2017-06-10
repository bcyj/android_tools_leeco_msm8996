/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_UTIL_H__
#define __ISP_UTIL_H__

#include "isp_channel_util.h"

#define ISP_DEFAULT_EFFECT      (CAM_EFFECT_MODE_OFF)
#define ISP_DEFAULT_SCE_FACTOR  (0)
#define ISP_DEFAULT_CONTRAST    (5)
#define ISP_DEFAULT_SATURATION  (5)
#define ISP_DEFAULT_SHARPNESS   (12)
#define ISP_DEFAULT_BESTSHOT    (CAM_SCENE_MODE_OFF)

/** isp_util_find_stream
 *    @isp: ISP root
 *    @session_id: session ID
 *    @stream_id: stream ID
 *
 *  Finds a required stream
 **/
isp_stream_t *isp_util_find_stream(isp_t *isp, uint32_t session_id,
  uint32_t stream_id);

isp_port_t *isp_util_find_sink_port(
  isp_t *isp,
  ispif_src_port_caps_t *ispif_src_cap);
isp_stream_t *isp_util_add_stream(isp_t *isp, uint32_t session_id,
                                      uint32_t stream_id,
                                      mct_stream_info_t *stream_info);
int isp_util_add_stream_to_sink_port(
  isp_t *isp,
  isp_port_t *isp_sink_port,
  isp_stream_t *stream);
int isp_util_del_stream_from_sink_port(
   isp_t *isp,
   isp_port_t *isp_sink_port,
   isp_stream_t *stream);
int isp_util_add_stream_to_src_port(
   isp_t *isp,
   isp_port_t *isp_src_port,
   isp_stream_t *stream);
int isp_util_del_stream_from_src_port(
   isp_t *isp,
   isp_port_t *isp_src_port,
   isp_stream_t *stream);
int isp_util_del_stream(
  isp_t *isp,
  isp_stream_t *stream);
isp_stream_t *isp_util_find_stream_from_sink_port(
     isp_port_t *isp_sink_port, uint32_t session_id, uint32_t stream_id);
isp_stream_t *isp_util_find_stream_from_src_port(
     isp_port_t *isp_src_port, uint32_t session_id, uint32_t stream_id);

boolean isp_util_need_pix(isp_t *isp, uint32_t vfe_output_mask);
isp_session_t *isp_util_find_session(isp_t *isp, uint32_t session_id);
boolean isp_util_is_stream_in_sink_port(
   isp_t *isp,
   isp_port_t *isp_sink_port,
   isp_stream_t *stream);
isp_stream_t * isp_util_find_stream_in_session(isp_session_t *sess, uint32_t stream_id);
isp_port_t *isp_util_get_matched_src_port(isp_t *isp,
  mct_port_caps_type_t cap_type, isp_port_t *isp_sink_port,
  isp_stream_t *stream);
int isp_util_select_pipeline_streams(isp_t *isp, isp_session_t *session);
int isp_util_map_image_buf(isp_t *isp,
                                  isp_session_t *session,
                                  isp_stream_t *stream, mct_list_t *img_buf_list);
int isp_util_alloc_native_buf(isp_t *isp,
                         isp_session_t *session,
                         isp_stream_t *stream);
void isp_util_release_native_buf(isp_t *isp,
                         isp_session_t *session,
                         isp_stream_t *stream);
int isp_util_create_hw(isp_t *isp, int hw_idx, int num_streams);
int isp_util_gen_hws_caps(isp_t *isp);
void isp_util_destroy_hw(isp_t *isp, int hw_idx, int num_streams);
int isp_util_stream_config_check_pix_caps(isp_t *isp,
                                         isp_port_t *isp_port,
                                         isp_stream_t *stream);

boolean isp_util_check_yuv_sensor_from_stream(isp_t *isp, uint32_t session_id,
  uint32_t stream_id);
int isp_util_do_pproc_zoom(isp_t *isp, uint32_t session_id, int32_t* zoom_val);

/** isp_util_set_bestshot_param
 *    @isp: isp root
 *    @session: isp session
 *    @bracket: bracketing control data
 *
 *  Sets  bracketing control parameter
 **/
int isp_util_set_cam_bracketing_ctrl_param(isp_t *isp, isp_session_t *session,
  mct_bracket_ctrl_t bracket);

/** isp_util_set_bestshot
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @bestshot: bestshot data
 *
 *  Sets bestshot parameter.
 **/
int isp_util_set_bestshot(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, cam_scene_mode_type *bestshot);

/** isp_util_set_skin_color_enhance
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @sce_factor: sce_factor data
 *
 *  Sets sce_factor parameter.
 **/
int isp_util_set_skin_color_enhance(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, int32_t *sce_factor);

/** isp_util_get_rolloff_table
 *    @isp: isp root
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @rolloff_table: rolloff table data
 *
 *  Gets the rolloff table.
 **/
int isp_util_get_rolloff_table(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, mct_event_stats_isp_rolloff_t *rolloff_table);
int isp_util_check_stream_aspect_ratio(
  isp_t *isp,
  isp_stream_t *stream1,
  isp_stream_t *stream2);
int isp_util_set_bundle(isp_t *isp,
                        isp_port_t *isp_sink_port,
                        uint32_t session_id,
                        uint32_t stream_id,
                        cam_bundle_config_t *bundle_param);
int isp_util_get_stream_ids_by_mask(isp_session_t *session,
                                      uint32_t stream_idx_mask,
                                      int *num_streams,
                                      uint32_t *stream_ids);
isp_stream_t * isp_util_find_3a_stream(isp_session_t *sess);
int isp_util_set_frame_skip(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  int32_t *skip_pattern);
int isp_util_set_effect(isp_t *isp,
                        isp_port_t *isp_sink_port,
                        uint32_t session_id,
                        uint32_t stream_id,
                        int32_t *effect);
int isp_util_set_contrast(isp_t *isp,
                        isp_port_t *isp_sink_port,
                        uint32_t session_id,
                        uint32_t stream_id,
                        int32_t *contrast);

int isp_util_set_video_hdr(isp_t *isp, uint32_t session_id, uint32_t stream_id,
  uint32_t *vhdr_enable);
int isp_util_set_sharpness(isp_t *isp,
  isp_port_t *isp_sink_port,
  uint32_t session_id,
  uint32_t stream_id,
  int32_t *sharpness);
int isp_util_set_saturation(isp_t *isp,
  isp_port_t *isp_sink_port,
  uint32_t session_id,
  uint32_t stream_id,
  int32_t *saturation,
  boolean is_init_setting);
int isp_util_gen_init_stats_cfg(
  isp_session_t *isp_session,
  isp_stream_t *stream);
int isp_util_set_hfr(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  int32_t *hfr_mode);
int isp_util_set_stats_bf_filter_size(
   isp_t *isp,
   uint32_t session_id,
   uint32_t stream_id);
int isp_util_set_dis(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  int32_t *dis_enable);
int isp_util_set_vt(
  isp_t *isp,
  uint32_t session_id,
  int32_t *vt_enable);
int isp_util_set_longshot(
  isp_t *isp,
  uint32_t session_id,
  int32_t *longshot_enable);
int isp_util_set_lowpowermode(
  isp_t *isp,
  uint32_t session_id,
  boolean *lowpowermode_enable);
int isp_util_ihist_la_trigger_update(
   isp_t *isp,
   isp_session_t *session);
int isp_util_send_dis_config_to_stats(isp_t *isp, isp_session_t *session);
int isp_util_set_param_zoom(
   isp_t *isp,
   isp_session_t *session,
   isp_hw_params_t *new_params);
void isp_util_send_hw_stream_output_dim_downstream(
  isp_t *isp,
  isp_session_t *session,
  int32_t num_streams,
  uint32_t *stream_ids);

int isp_util_set_tintless(isp_t *isp, uint32_t session_id,
  isp_tintless_data_t *tintless_data);

int isp_util_get_user_streams(
  isp_session_t *session,
  uint32_t stream_id,
  uint32_t *stream_ids);

/** isp_util_set_cds_mode
 *    @isp: Isp handler
 *    @session_id: session id
 *    @cds_mode: camera cds mode
 *
 * Return: 0- success and negative value - failure
 **/
int isp_util_set_cds_mode(isp_t *isp, uint32_t session_id,
  cam_cds_mode_type_t *cds_mode);

/** isp_util_do_zoom_at_streamon
 *    @isp: isp root
 *    @session: isp session
 *
 *  Aplies the requred zoom at stream on.
 **/
void isp_util_do_zoom_at_streamon(isp_t *isp, isp_session_t *session);

/** isp_util_get_user_streams
 *    @session: isp session
 *    @stream_id: Stream ID
 *    @stream_ids: Stream ID
 *
 *  Gets IDs of streams from same HAL bunfle as given stream.
 **/
int isp_util_get_user_streams(isp_session_t *session, uint32_t stream_id,
  uint32_t *stream_ids);

int isp_util_request_image_buf(
  isp_t *isp,
  isp_session_t *session,
  int num_streams, uint32_t *stream_ids);
int isp_util_release_image_buf(
  isp_t *isp,
  isp_session_t *session,
  int num_streams,
  uint32_t *stream_ids);

boolean isp_util_is_burst_streaming(isp_session_t *session);
int isp_util_get_rolloff_grid_info(
  isp_t *isp,
  isp_session_t *session,
  uint32_t *grid_width);
int isp_util_compute_stripe_info(
  isp_t *isp,
  isp_session_t *session,
  isp_stream_t *stream);
int isp_util_update_hal_image_buf_to_channel(
  isp_session_t *session,
  isp_stream_t *stream);
int isp_util_config_for_streamon(
  isp_t *isp,
  isp_session_t *session);
int isp_util_gen_channel_streamon_list(
  isp_t *isp,
  isp_session_t *session,
  int num_streams,
  uint32_t *streamids,
  int *num_hw_channels,
  uint32_t *hw_channel_ids);
int isp_util_streamon(
  isp_t *isp,
  isp_session_t *session,
  int num_streams,
  uint32_t *stream_ids);
int isp_util_streamoff(
  isp_t *isp,
  isp_session_t *session,
  int num_streams,
  uint32_t *stream_ids,
  boolean stop_immediately);
int isp_util_proc_restart(
   isp_t *isp,
   uint32_t session_id,
   uint32_t *vfe_mask);
void isp_util_broadcast_zoom_crop(
  isp_t *isp,
  uint32_t session_id,
  int num_streams,
  uint32_t *streamids,
  uint32_t frame_id,
  struct timeval *timestamp);
void isp_util_broadcast_sof_msg_to_modules(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  mct_bus_msg_isp_sof_t *sof_event);
void isp_util_dump_stream_planes(isp_stream_t *stream);
void isp_util_dump_stats_config(stats_config_t *stats_data);
int isp_util_unconfig_stream(
  isp_t *isp,
  isp_session_t *session,
  isp_stream_t *stream);
int isp_util_unconfig_stream_by_sink_port(
  isp_t *isp,
  isp_session_t *session,
  isp_port_t *isp_sink_port);
void isp_util_send_initial_zoom_crop_to_3a(
  isp_t *isp,
  uint32_t session_id,
  int num_streams,
  uint32_t *streamids);
int isp_util_set_recording_hint(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  int32_t *recording_hint);
int isp_util_set_eztune_diagnostics(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  int32_t *set_param);
int isp_util_set_module_trigger(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  isp_mod_trigger_t *set_param);
int isp_util_set_module_enable(
  isp_t *isp,
  uint32_t session_id,
  uint32_t stream_id,
  isp_mod_trigger_t *set_param);
int isp_util_send_buffered_hw_params_to_hw(
  isp_t *isp,
  isp_session_t *session);
int isp_util_buffered_set_param_zoom(
   isp_t *isp,
   uint32_t session_id,
   uint32_t stream_id,
   int32_t *zoom_val);
int isp_util_set_param_tintless(isp_t *isp, uint32_t session_id,
  uint8_t *enabled);
boolean isp_util_stream_use_pipeline(isp_t *isp,  isp_stream_t *stream);
int isp_util_send_uv_subsample_cmd(isp_t *isp, isp_session_t *session,
  uint32_t uv_subsample);
void isp_util_broadcast_pproc_zoom_crop (isp_t *isp, uint32_t session_id,
  int num_streams, uint32_t *streamids, uint32_t frame_id,
  struct timeval *timestamp);
boolean isp_util_is_4k2k_resolution_set(cam_dimension_t dim);
uint32_t isp_util_is_lowpowermode_feature_enable(isp_t *isp, uint32_t session_id);
int isp_util_wm_bus_overflow_recovery(isp_t *isp,
   isp_hw_t *isp_hw,
   isp_hw_session_t *hw_session);
#endif /* __ISP_UTIL_H__ */

