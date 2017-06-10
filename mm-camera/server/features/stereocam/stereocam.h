/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __STEREO_CAM_H__
#define __STEREO_CAM_H__

#include "tgtcommon.h"
#ifdef MM_STEREO_CAM
  #include "lib3d.h"
#endif

#define ST_PROC_BUFNUM 3

typedef struct {
  uint32_t orig_w;
  uint32_t orig_h;
  uint32_t modified_w;
  uint32_t modified_h;
} st_frame_pack_dim;

typedef struct {
  float left_matrix[3][4];
  float right_matrix[3][4];
} stereo_calibration_matrix;

typedef enum {
  ST_FRAME_D_DONE,
  ST_FRAME_L_DONE,
  ST_FRAME_R_DONE,
  ST_ANALYSIS_START,
  ST_ANALYSIS_END,
} stereo_frame_progress;

typedef enum {
  ST_CONV_AUTO,
  ST_CONV_MANUAL_INIT,
  ST_CONV_MANUAL,
  ST_CONV_MAX,
} stereo_conv_mode;

typedef struct {
  uint32_t buf_size;
  struct msm_frame frame[ST_PROC_BUFNUM];
  st_frame_pack_dim left_pack_dim;
  st_frame_pack_dim right_pack_dim;
  enum msm_st_frame_packing packing;
  int non_zoom_upscale;
  float geo_corr_matrix[9];
  stereo_frame_progress currentFrameProgress;
} stereo_frame_t;

#ifdef MM_STEREO_CAM
typedef struct {
  void *lib3d_obj;
  s3d_t s3d_param;
  s3d_geo_corr_origin_t s3d_geo_corr_origin;
  s3d_state_t s3d_state;
  s3d_config_t s3d_cfg_data;
  s3d_window_attribs_t s3d_active_window;
  uint32_t view_angle;
  float display_distance;
  float pop_out_ratio;
  float quality_indicator;
  int (*s3d_get_version)(float *api_version, float *library_version);
  int (*s3d_create)(s3d_t *p_s3d);
  int (*s3d_config)(s3d_t s3d, s3d_config_t *p_cfg);
  int (*s3d_set_display_angle_of_view)(s3d_t s3d, uint32_t angle_in_degree);
  int (*s3d_set_display_distance)(s3d_t s3d, float distance_in_meters);
  int (*s3d_set_acceptable_angular_disparity)(s3d_t s3d,
    float disparity_in_degree);
  int (*s3d_set_convergence_lpf_weight)(s3d_t s3d, float weight);
  int (*s3d_set_active_analysis_window)(s3d_t s3d,
    s3d_window_attribs_t *p_active_window);
  int (*s3d_get_correction_matrix)(s3d_t s3d, s3d_image_format_t format,
    s3d_geo_corr_origin_t geo_corr_origin,int32_t native_width,
    int32_t height, s3d_matrix_t correction_matrix);
  int (*s3d_reset_history)(s3d_t s3d);
  int (*s3d_run_convergence)(s3d_t s3d, uint8_t *p_left_analysis_img_ptr,
    uint32_t left_analysis_img_stride, uint8_t *p_right_analysis_img_ptr,
    uint32_t right_analysis_img_stride, s3d_state_t *s3d_state,
    float *p_right_image_shift, float *p_3d_quality);
  int (*s3d_adjust_3d_effect)(s3d_t s3d, float pop_out_ratio);
  int (*s3d_destroy)(s3d_t s3d);
  int (*s3d_abort)(s3d_t s3d);
} stereo_lib_data_t;
#endif

typedef struct {
  stereo_frame_t procFrame;
  stereo_frame_t videoFrame;
  stereo_frame_t snapshotMainFrame;
  stereo_frame_t snapshotThumbFrame;
  float right_image_shift;
  float left_image_shift;
  uint32_t manualConvRange;
  uint32_t manualConvValue;
  stereo_conv_mode convMode;
  stereo_calibration_matrix sensorOTPMatrix;
  struct msm_cam_evt_msg bkupEvtMsg;
#ifdef MM_STEREO_CAM
  stereo_lib_data_t lib3d;
#endif
  /* stereocam_analysis thread utility functions */
  int (*st_analysis_launch) (void *data);
  int (*st_analysis_wait_ready) ();
  int (*st_analysis_release) ();

  /* stereocam_dispatch thread utility functions */
  int (*st_dispatch_launch) (void *data);
  int (*st_dispatch_wait_ready) ();
  int (*st_dispatch_release) ();
} stereo_ctrl_t;


#ifdef MM_STEREO_CAM
  int stereocam_get_lib3d_format(int cam_packing);
  int stereocam_get_correction_matrix(stereo_ctrl_t *, stereo_frame_t *);
#endif

void stereocam_init(void *ctrlBlk);
void stereocam_deinit(void *ctrlBlk);
int8_t stereocam_config_proc_ctrl_command(void *ctrlBlk, void *ctrlCmd);
int8_t stereocam_config_proc_vfe_event_message(void *ctrlBlk, void *camMsg);
void stereocam_proc_message(int thread_id, int direction, void *ctrlBlk,
  void *camMsg);

int stereocam_set_display_distance(void *ctrlBlk, void *value);
int stereocam_set_display_view_angle(void *ctrlBlk, void *value);
int stereocam_set_3d_effect(void *ctrlBlk, void *value);
int stereocam_set_manual_conv_value(void *ctrlBlk, void *value);
int stereocam_enable_manual_convergence(void *ctrlBlk, void *value);

uint32_t stereocam_get_display_distance(void *ctrlBlk);
uint32_t stereocam_get_display_view_angle(void *ctrlBlk);
uint32_t stereocam_get_3d_effect(void *ctrlBlk);
uint32_t stereocam_get_manual_conv_range(void *ctrlBlk);

/* stereocam_dispatch thread utility functions */
int launch_stereocam_dispatch_thread(void *data);
int wait_stereocam_dispatch_ready(void);
int release_stereocam_dispatch_thread(void);

/* stereocam_analysis thread utility functions */
int launch_stereocam_analysis_thread(void *data);
int wait_stereocam_analysis_ready(void);
int release_stereocam_analysis_thread(void);

#endif /* __STEREO_CAM_H__ */
