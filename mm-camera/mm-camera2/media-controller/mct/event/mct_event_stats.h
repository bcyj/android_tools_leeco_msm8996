/* mct_event_stats.h
 *                                                           .
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_EVENT_STATS_H__
#define __MCT_EVENT_STATS_H__

#include <media/msmb_isp.h>
#include "media_controller.h"
#include <media/msm_cam_sensor.h>

typedef struct _mct_event_stats_gyro {
} mct_event_stats_gyro_t;

#define MCT_MESH_ROLL_OFF_V4_TABLE_SIZE (13*10)

#ifdef AF_2X13_FILTER_SUPPORT
#define MAX_HPF_COEFF 26
#else
#define MAX_HPF_COEFF 10
#endif

typedef struct _mct_event_stats_isp_data {
  enum msm_isp_stats_type stats_type;
  void *stats_buf;    /* 3A buffer pointer */
  uint32_t buf_size;      /* buffer size */
  uint32_t used_size; /* used size */
} mct_event_stats_isp_data_t;

typedef struct _mct_event_stats_isp_rolloff {
  /* Table for red.  */
  uint16_t TableR[MCT_MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for Gr.   */
  uint16_t TableGr[MCT_MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for blue. */
  uint16_t TableB[MCT_MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for Gb.   */
  uint16_t TableGb[MCT_MESH_ROLL_OFF_V4_TABLE_SIZE];
} mct_event_stats_isp_rolloff_t;

typedef struct {
  uint32_t stats_mask;
}stats_cfg_t;

typedef struct {
  boolean start;      /* enable or disable */
  uint32_t stats_mask; /* which stats need to be enabled */
}stats_start_t;

typedef struct {
  unsigned int id; /* session id + stream id */
  unsigned int frame_id;
  int use_3d;
  float transform_matrix[9];
  unsigned int transform_type;
  int x;
  int y;
  int width;
  int height;
} is_update_t;

typedef struct {
  /* stats to isp */
  uint32_t landscape_severity;
  uint32_t backlight_detected;
  uint32_t backlight_scene_severity;
  boolean snow_or_cloudy_scene_detected;
  uint32_t snow_or_cloudy_luma_target_offset;
  boolean histo_backlight_detected;
  float saturation_adjust;
  uint32_t backlight_luma_target_offset;
  uint32_t portrait_severity;
  float asd_soft_focus_dgr;
  boolean mixed_light;
  boolean asd_enable;

  cam_auto_scene_t scene;
  uint32_t severity[S_MAX];
  uint32_t is_hdr_scene;
  float    hdr_confidence;
} asd_update_t;

typedef enum {
  AFD_TBL_OFF,
  AFD_TBL_60HZ,
  AFD_TBL_50HZ,
  AFD_TBL_MAX,
} afd_tbl_t;

typedef struct {
  /* afd update */
  boolean     afd_enable;
  afd_tbl_t   afd_atb;
  boolean     afd_monitor;
  boolean     afd_exec_once;
}afd_update_t;

typedef enum {
  AEC_EST_OFF,
  AEC_EST_START,
  AEC_EST_DONE,
  AEC_EST_NO_LED_DONE,
  AEC_EST_DONE_FOR_AF,
  AEC_EST_DONE_SKIP,
}aec_led_est_state_t;

typedef struct {
  /* to isp */
  uint32_t stats_frm_id;
  float real_gain;
  uint32_t linecount;
  float dig_gain;
  unsigned int numRegions;
  int pixelsPerRegion;
  int comp_luma;
  uint32_t exp_tbl_val;
  int asd_extreme_green_cnt;
  int asd_extreme_blue_cnt;
  int asd_extreme_tot_regions;
  uint32_t target_luma; /*current luma target*/
  uint32_t cur_luma; /*current vfe luma*/
  int32_t exp_index; /**/
  int32_t exp_index_for_awb;
  float lux_idx;
  int settled;
  int indoor_index;
  int outdoor_index;
  float band_50hz_gap;
  int cur_atb;
  int max_line_cnt;
  float stored_digital_gain;
  int led_state;
  boolean prep_snap_no_led;
  int use_led_estimation;
  int aec_flash_settled;
  unsigned int luma_settled_cnt;
  unsigned int *SY;
  aec_led_est_state_t est_state;
  float exp_time;
  void *flash_sensitivity;
  uint32_t exif_iso;
  unsigned int frame_id;
  uint32_t flash_needed;
  uint32_t luma_delta;
  int preview_fps;
  int preview_linesPerFrame;
  unsigned int sof_id;
  boolean frame_ctrl_en;
  int op_mode;
  boolean led_needed;
  float led_off_gain;
  unsigned int led_off_linecnt;
  boolean nightshot_detected;
  boolean hdr_indoor_detected;
  int metering_type;
  /*TDB : Apex information*/
  float Tv;
  float Sv;
  float Av;
  float Bv;
  float Ev;
}aec_update_t;

typedef struct _af_output_mobicat_data {
  int          peak_location_index;
  uint16_t     roi_left;
  uint16_t     roi_top;
  uint16_t     roi_width;
  uint16_t     roi_height;
  int          grid_info_h_num;
  int          grid_info_v_num;
  uint32_t     r_fv_min;
  uint32_t     gr_fv_min;
  uint32_t     gb_fv_min;
  uint32_t     b_fv_min;
  int          hpf[10];
  int          mode;
  int          status;
  int          far_end;
  int          near_end;
  int          hyp_pos;
  int          state;
  int          stats_index;
  int          stats_pos;
  int          stats_fv[50];
  int          stats_max_fv;
  int          stats_min_fv;
  int          frame_delay;
  int          enable_multiwindow;
  int          Mwin[14];
  int          num_downhill;
  int          caf_state;
  uint32_t     cur_luma;
  int          exp_index;
  unsigned int luma_settled_cnt;
  int          ave_fv;
  int          caf_panning_unstable_cnt;
  int          caf_panning_stable_cnt;
  int          caf_panning_stable;
  int          caf_sad_change;
  int          caf_exposure_change;
  int          caf_luma_chg_during_srch;
  int          caf_trig_refocus;
  int          caf_gyro_assisted_panning;
} af_output_mobicat_data_t;


typedef struct {
  uint32_t h_num;
  uint32_t v_num;
}stats_grid_t;

typedef struct {
  /* ---- To Sensor -----*/
  /* Move Lens*/
  uint32_t stats_frm_id;
  boolean move_lens;
  int direction;
  int num_of_steps;

  boolean use_dac_value;
  int num_of_interval;
  uint16_t pos[MAX_NUMBER_OF_STEPS];
  uint16_t delay[MAX_NUMBER_OF_STEPS];

  /*Reset Lens*/
  boolean reset_lens;
  int reset_pos;

  /* Stop AF */
  boolean stop_af;

  /*----- To AEC ----*/
  boolean check_led;
}af_update_t;

typedef struct {
  int current_step;
} lens_position_update_isp_t;

typedef struct {
  uint32_t t1;
  uint32_t t2;
  uint32_t t3;
  uint32_t t6;
  uint32_t t4;
  uint32_t mg;
  uint32_t t5;
}awb_exterme_color_param_t;

typedef struct {
  float r_gain;
  float g_gain;
  float b_gain;
} awb_gain_t;

typedef struct
{
  unsigned char y_min;                      // LumaMin
  unsigned char y_max;                      // LumaMax

  /* Slope of neutral region line 1 */
  char m1;                          // Slope1
  /* Slope of neutral region line 2 */
  char m2;                          // Slope2

  /* Slope of neutral region line 3 */
  char m3;                          // Slope3

  /* Slope of neutral region line 4 */
  char m4;                          // Slope4

  /* Cb intercept of neutral region line 1 */
  short c1;                         // CbOffset1

  /* Cb intercept of neutral region line 2 */
  short c2;                         // CrOffset2

  /* Cb intercept of neutral region line 3 */
  short c3;                         // CbOffset3

  /* Cb intercept of neutral region line 4 */
  short c4;                         // CrOffset4
} awb_bounding_box_t;

typedef struct {
  uint32_t led1_low_setting;   // led1 current value for pre-flash
  uint32_t led2_low_setting;   // led2 current value for pre-flash
  uint32_t led1_high_setting;  // led1 current value for main-flash
  uint32_t led2_high_setting;  // led2 current value for main-flash
} awb_dual_led_settings_t;

typedef struct {
  /* to isp */
  awb_gain_t gain; /* cast to chromatix_manual_white_balance_type */
  uint32_t color_temp;
  awb_bounding_box_t bounding_box; /* cast to chromatix_wb_exp_stats_type */
  awb_exterme_color_param_t exterme_color_param;
  int wb_mode;
  int best_mode;
  int sample_decision[64];
  boolean grey_world_stats;
  int ccm_flag;
  float cur_ccm[3][3];
  awb_dual_led_settings_t dual_led_setting;
  int decision;
  int gains_restored;
}awb_update_t;

typedef enum {
  STATS_UPDATE_AEC   = (1 << 0),
  STATS_UPDATE_AWB   = (1 << 1),
  STATS_UPDATE_AF    = (1 << 2),
  STATS_UPDATE_ASD   = (1 << 3),
  STATS_UPDATE_AFD   = (1 << 4),
}stats_update_mask_t;

typedef struct {
  union {
    aec_update_t aec_update;
    awb_update_t awb_update;
    af_update_t  af_update;
    asd_update_t asd_update;
    afd_update_t afd_update;
  };
  stats_update_mask_t flag;
}stats_update_t;

typedef struct {
  float real_gain[5];
  uint32_t linecount[5];
  float led_off_gain;
  uint32_t led_off_linecount;
  float led_off_cf_gain;
  uint32_t led_off_cf_linecount;
  int valid_entries;
  int trigger_led;
  float exp_time;
  float lux_idx;
  boolean flash_needed;
  int exposure_index;
}aec_get_t;

typedef struct {
  float r_gain;
  float g_gain;
  float b_gain;
}_awb_get_t;

typedef struct {
  aec_get_t aec_get;
  _awb_get_t awb_get;
  stats_update_mask_t flag;
}stats_get_data_t;

typedef struct {
  stats_grid_t grid_info;
  cam_rect_t   roi;
  uint32_t r_Max;
  uint32_t gr_Max;
  uint32_t b_Max;
  uint32_t gb_Max;
} aec_bg_config_t;

typedef struct {
  stats_grid_t grid_info;
  cam_rect_t roi;
} aec_bhist_config_t;

typedef struct {
  aec_bg_config_t bg_config;
  aec_bhist_config_t bhist_config;
}aec_config_t;

typedef struct {
  stats_grid_t grid_info;
  cam_rect_t   roi;
  awb_update_t awb_init_val;
}awb_config_t;

/* mct_event_stats_config_type:
 * ROI configuration type
 */
typedef enum {
  STATS_CONFIG_MODE_SINGLE,
  STATS_CONFIG_MODE_MULTIPLE,
} mct_event_stats_config_type;

/** mct_stats_hpf_size:
 *  */
typedef enum _mct_stats_hpf_size {
  MCT_EVENT_STATS_HPF_LEGACY,
  MCT_EVENT_STATS_HPF_2X5,
  MCT_EVENT_STATS_HPF_2X13
} mct_stats_hpf_size_type;

/**mct_stats_info_t: Used by ISP to report the supported stats.
 *
 * @kernel_size: isp will tell us which kenrel size to be configured
 *
 *
 **/
typedef struct mct_stats_info_t {
  mct_stats_hpf_size_type kernel_size;
} mct_stats_info_t;

/**af_config_t: Information required to configure AF stats.
 *
 * @roi_type: configure single roi or whole region
 *
 * @roi: roi dimensions
 **/
typedef struct {
  unsigned int stream_id;
  int roi_type;
  stats_grid_t grid_info; /* remove it later */
  cam_rect_t   roi;
  int hpf[MAX_HPF_COEFF];
  unsigned int r_min;
  unsigned int b_min;
  unsigned int gr_min;
  unsigned int gb_min;
}af_config_t;

typedef struct {
  /*stats mask is the one that 3A asks which stats to enable
    since RS, CS, hist stats does not need the configure, so no
    field member is needed for them.
   */
  uint32_t            stats_mask;
  aec_config_t *aec_config; /*include ae or bg*/
  awb_config_t *awb_config;
  af_config_t  *af_config;
}stats_config_t;

typedef struct _mct_event_stats_isp {
  struct timeval timestamp;
  uint32_t frame_id;
  uint32_t stats_mask;
  boolean is_tintless_data;
  mct_event_stats_isp_data_t stats_data[MSM_ISP_STATS_MAX];
  stats_config_t stats_config;
} mct_event_stats_isp_t;

/** mct_event_stats_type
 *
 *
 *
 * 3rd level event type with type of structure mct_event_stats_t
 * Event type used module stats module and sub-modules
 **/
typedef enum _mct_event_stats_type {
  MCT_EVENT_STATS_ISP_STATS_CFG,     /* config stats */
  MCT_EVENT_STATS_ISP_STATS_ENABLE,  /* enable stats */
  MCT_EVENT_STATS_ISP_STATS,         /* isp parsed stats */
  MCT_EVENT_STATS_STATS_MAX
} mct_event_stats_type;

typedef struct _mct_event_stats {
  mct_event_stats_type type;
  union {
    mct_event_stats_isp_t   isp_stats; /*from isp to 3a*/
    stats_start_t           start; /*3a to isp cmd*/
    stats_update_t          stats_update; /*3a to isp update*/
    stats_config_t          stats_cfg; /*3a to isp update*/
  } u;
} mct_event_stats_t;


#endif /* __MCT_EVENT_STATS_H___*/
