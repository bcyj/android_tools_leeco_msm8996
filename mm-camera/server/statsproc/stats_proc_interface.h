/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __STATS_PROC_INTERFACE_H__
#define __STATS_PROC_INTERFACE_H__

#include "chromatix.h"
#include "af_tuning.h"
#include "camera.h"
#include "tgtcommon.h"
#include "intf_comm_data.h"

/********************************
     STATS_PROC EXPORTED ENUMS
*********************************/
typedef enum {
  AWB_DEFAULT_CC = 0,
  AWB_SPECIAL_EFFECTS_CC
} stats_proc_color_conv_state_type;

typedef enum {
  STATS_PROC_STATE_NONE,
  STATS_PROC_STATE_INIT,
  STATS_PROC_STATE_PREVIEW,
  STATS_PROC_STATE_SNAPSHOT,
  STATS_PROC_STATE_CAMCORDER,
  STATS_PROC_STATE_INVALID,
} stats_proc_state_type;

typedef enum {
  STATS_PROC_UNDEF = -1,
  STATS_PROC_4_TO_3,
  STATS_PROC_16_TO_9,
} stats_proc_aspect_ratio_t;

typedef enum {
  STATS_PROC_MODE_2D_NORM,
  STATS_PROC_MODE_2D_ZSL,
  STATS_PROC_MODE_3D_NORM,
  STATS_PROC_MODE_3D_ZSL,
} stats_proc_mode_type;

/*todo need to revisit VFE is settign values */
typedef enum {
  STROBE_NOT_NEEDED,
  STROBE_TO_BE_CONFIGURED,
  STROBE_PRE_CONFIGURED,
  STROBE_PRE_FIRED,
  STROBE_SNAP_CONFIGURED,
} stats_proc_strobe_confg_st_type;

typedef enum {
  AF_ABORT = -2,
  AF_FAILED = -1,
  AF_DONE = 0,
  // AF_MAX,
}stats_proc_af_done_type;

typedef struct {
  uint32_t   roi_pixels;
  uint32_t   *bin;
} stats_proc_hist_t;

typedef struct {
  int min_value;
  int max_value;
  int step_val;
  int default_val;
  int current_val;
} stats_proc_cam_parm_info;

typedef struct {
  roi_t      roi[MAX_ROI];
  stats_proc_hist_t     hist[MAX_ROI];
  stats_proc_roi_type_t type;
  uint32_t         frm_id;
  uint32_t         num_roi;
  uint32_t         frm_width;
  uint32_t         frm_height;
  uint32_t         roi_updated;
} stats_proc_roi_info_t;

typedef struct {
  int               num_area;
  roi_t             mtr_area[MAX_ROI];
  int               weight[MAX_ROI];
}stats_proc_mtr_area_t;

/********************************
  STATS_PROC INIT Interface Data
*********************************/
/* sensor provided informaton */
typedef struct {
  uint32_t current_fps;
  uint32_t preview_fps;
  uint32_t snapshot_fps;
  uint32_t video_fps; // hardcode it to 30
  uint32_t max_preview_fps;
  uint32_t preview_linesPerFrame;
  uint32_t snap_linesPerFrame;
  uint32_t snap_max_line_cnt;
  float max_gain;
  uint32_t pixel_clock;
  uint32_t pixel_clock_per_line;
  uint32_t af_is_supported;
} stats_proc_sensor_info_t;

typedef struct {
  float focal_length;    /* focal length of lens (mm) */
  float af_f_num;        /* numeric aperture of lens (mm) */
  float af_f_pix;        /* pixel size (microns) */
  float af_total_f_dist; /* Lens distance (microns) */
  float hor_view_angle;  /* Sensor's horizontal view angle */
  float ver_view_angle;  /* Sensor's vertical view angle */
} stats_proc_actuator_info_t;

typedef struct {
  int pixelsPerRegion;
  int zero_hregions;
  int zero_vregions;
  int trigger_CAF;
  int video_rec_width;
  int video_rec_height;
  int output2_width;
  int output2_height;
  int preview_width;
  int preview_height;

  uint32_t numRegions;
  uint16_t vfe_af_WinHorWidth;
  int  rgnVnum;
  int  rgnHnum;
  vfe_stats_output_t *vfe_stats_out;
  stats_proc_type_t          type;
  stats_proc_mode_type       opt_mode;
  stats_proc_state_type      opt_state;
  float vfe_dig_gain;
} stats_proc_mctl_info_t;

typedef struct {
  float digital_gain_adj;
  float blk_inc_comp;
} stats_proc_isp_info_t;

/* postproc information */
typedef struct {
  int wd_enabled;
} stats_proc_postproc_info_t;

/* flash information */
typedef struct {
  int led_state;
  int led_mode;
  uint32_t strobe_chrg_ready;
  int strobe_cfg_st;
} stats_proc_flash_info_t;

typedef struct {
  /* Gyro data in float */
  int float_ready;
  float flt[3];
  /* Gyro data in Q16 */
  int q16_ready;
  long q16[3];
} stats_proc_gyro_info_t;

/********************************
  STATS_PROC Shared Data
*********************************/
/* EZ TUNE Data */
typedef struct {
  uint32_t touch_roi_luma;
  float preview_exp_time;
} stats_proc_aec_eztune_t;

typedef struct {
  int   outdoor_grn_cnt;
  int   indoor_grn_cnt;
  int   compact_cluster;
  int   ymin_pct;
  int   current_stat_config;
  float reg_ave_rg_ratio;
  float reg_ave_bg_ratio;
  float white_ave_rg_ratio;
  float white_ave_bg_ratio;
  float shifted_d50_rg;
  float shifted_d50_bg;
  float sgw_rg_ratio;
  float sgw_bg_ratio;
} stats_proc_awb_eztune_t;

typedef struct {
  int peakpos_index;
  int tracing_index;
  int tracing_stats[AF_COLLECTION_POINTS];
  int tracing_pos[AF_COLLECTION_POINTS];
} stats_proc_af_eztune_t;

typedef struct {
  int flicker_freq;
  int actual_peaks;
  int multiple_peak_algo;
  int std_width;
  int status;
  int flicker_detect;
} stats_proc_afd_eztune_t;

typedef struct {
  uint32_t mixed_light;
  uint32_t histo_backlight_detected;
} stats_proc_asd_eztune_t;
/* End of EZ TUNE Data */

typedef struct {
  uint32_t luma_target;
  uint32_t line_count;
  uint32_t redeye_led_on_skip_frm;
  uint32_t redeye_led_off_skip_frm;
  uint32_t auto_mode;
  /* exif tag data */
  uint32_t exp_mode;
  uint32_t exp_program;
  float    real_gain;
  float    exp_time;
} stats_proc_aec_snapshot_t;

typedef struct {
  float off;
  float low;
  float high;
} stats_proc_flash_sensitivity_t;

typedef struct {
  /* FPS */
  int preview_fps;
  int afr_enable;
  camera_auto_exposure_mode_type  metering_type;
  int iso;

  /* Index */
  int   exp_index;
  int   indoor_index;
  int   outdoor_index;
  float lux_idx;

  int aec_settled;

  /* digital gain */
  float stored_digital_gain;

  /* Luma */
  uint32_t target_luma;
  uint32_t cur_luma;
  int high_luma_region_count;

  uint32_t cur_line_cnt;
  float cur_real_gain;
  float prev_sensitivity;

  uint32_t exp_tbl_val;
  int max_line_cnt;
  int sof_update;

  int  comp_luma;

  /* LED */
  stats_proc_flash_sensitivity_t flash_si;
  int led_state;
  int use_led_estimation;
  stats_proc_strobe_confg_st_type strobe_cfg_st;
  uint32_t stobe_len;
  int led_frame_skip_cnt;
  int aec_flash_settled;
  int use_strobe;

  /* AFD */
  float band_50hz_gap;
  /* HJR */
  int hjr_snap_frame_cnt;
  uint32_t hjr_dig_gain;
  stats_proc_aec_snapshot_t    snap;
  stats_proc_aec_eztune_t   eztune;
  int asd_extreme_green_cnt;
  int asd_extreme_blue_cnt;
  /* total bayer stat regions 64x48  used to get ratio of extreme stats.*/
  int asd_extreme_tot_regions;
  QAECInfo_t mobicat_aec;
} stats_proc_aec_data_t;

/* AWB DATA */
typedef struct {
  float redGain;
  float greenGain;
  float blueGain;
  int8_t update;
} stats_proc_wb_gains_type;

typedef struct {
  uint32_t    t1;  /**< Green zone boundary 1 parameter, unsigned Q6.*/
  uint32_t    t2;  /**< Green zone boundary 2 parameter, unsigned Q6.*/
  uint32_t    t3;  /**< Green zone boundary 3 parameter, unsigned Q6.*/
  uint32_t    t4;  /**< Green zone slope boundary 1 parameter, unsigned Q6.*/
  uint32_t    t5;  /**< Extreme blue color boundary parameter, unsigned Q6.*/
  uint32_t    t6;  /**< Extreme red color boundary parameter, unsigned Q6. */
  uint32_t    mg;  /**< Green zone slope boundary 2 parameter, unsigned Q6.*/
} awb_extreme_stats_t;

typedef struct {
  int      awb_update;
  int      decision;
  uint32_t color_temp;

  chromatix_rolloff_light_type         rolloff_tbl;
  chromatix_manual_white_balance_type  curr_gains;
  chromatix_manual_white_balance_type  snapshot_wb;
  chromatix_wb_exp_stats_type          bounding_box;
  awb_extreme_stats_t                  estats; /*Extreme stats */
  config3a_wb_t                        current_wb_type;
  stats_proc_awb_eztune_t              eztune;
  QAWBInfo_t                           mobicat_awb;
} stats_proc_awb_data_t;
/* AF DATA  */

typedef struct {
  int active;
  int reset_lens;
  int stop_af;
  int done_flag;
  int done_status;
  int direction;
  int num_of_steps;
  int move_lens_status;
  int cont_af_enabled;
  int touch_af_enabled;
  int af_led_on_skip_frm;
  int cur_af_mode;
  int cur_focusrect_value;
  roi_t af_roi[MAX_ROI];
  int af_stats_config_mode;
  uint8_t *af_multi_roi_window;
  unsigned int af_multi_nfocus;
  stats_proc_roi_info_t roiInfo;
  stats_proc_roi_info_t fdInfo;
  stats_proc_af_eztune_t eztune;
  QAFInfo_t mobicat_af;
} stats_proc_af_data_t;

typedef struct {
  /* ASD DATA */
  uint32_t backlight_detected;
  uint32_t backlight_scene_severity;
  uint32_t snow_or_cloudy_scene_detected;
  uint32_t landscape_severity;
  uint32_t portrait_severity;
  float soft_focus_dgr;
  stats_proc_asd_eztune_t  eztune;
} stats_proc_asd_data_t;

typedef struct {
  int has_output;
  cam_dis_info_t dis_pos;
  int32_t prev_dis_output_x;
  int32_t prev_dis_output_y;
  int32_t eis_output_x;
  int32_t eis_output_y;
  unsigned long eis_output_valid;
} stats_proc_dis_data_t;

typedef struct {
  stats_proc_afd_eztune_t  eztune;
} stats_proc_afd_data_t;

typedef struct {
  stats_proc_mctl_info_t     mctl_info;     /* mctl provided information */
  chromatix_parms_type       *chromatix;    /* chromatix header pointer */
  af_tune_parms_t            *af_tune_ptr; /* focus  header pointer */
  stats_proc_sensor_info_t   sensor_info;   /* sensor provided informaton */
  stats_proc_actuator_info_t actr_info;     /* lens actuator information */
  stats_proc_isp_info_t      isp_info;      /* isp provided information */
  stats_proc_postproc_info_t postproc_info; /* PostProc provided information */
  stats_proc_flash_info_t    flash_info;    /* Flash provided information */
  stats_proc_gyro_info_t     gyro_info;     /* GYRO provided information */
  mctl_ops_t                 ops;           /* mctl ops table */
  //void                       *ops_cookie;   /* mctl ops cookie */
} stats_proc_interface_input_t;

typedef struct {
  stats_proc_aec_data_t  aec_d;
  stats_proc_awb_data_t  awb_d;
  stats_proc_af_data_t   af_d;
  stats_proc_asd_data_t  asd_d;
  stats_proc_dis_data_t  dis_d;
  stats_proc_afd_data_t  afd_d;
} stats_proc_interface_output_t;

typedef struct {
  stats_proc_interface_output_t output;
  stats_proc_interface_input_t  input;
} stats_proc_interface_t;

/********************************
     STATS_PROC SET Interface Data
*********************************/
/* AEC SET DATA */
typedef struct {
  uint8_t enable;
  uint32_t rgn_index;
} stats_proc_interested_region_t;

typedef enum {
  AEC_EXP_COMPENSATION,
  AEC_BRIGHTNESS_LVL,
  AEC_HJR_AF,
  AEC_REDEYE_REDUCTION_MODE,
  AEC_METERING_MODE,
  AEC_ISO_MODE, /* 5 */
  AEC_MOTION_ISO,
  AEC_ANTIBANDING,
  AEC_ANTIBANDING_STATUS,
  AEC_FPS_MODE,
  AEC_LED_RESET,
  AEC_HJR, /* 10 */
  AEC_PREPARE_FOR_SNAPSHOT,
  AEC_STROBE_MODE,
  AEC_PARM_FPS,
  AEC_SET_ROI,
  AEC_SET_MTR_AREA, /* 15 */
  AEC_SOF,
  AEC_BESTSHOT,
  AEC_SET_FD_ROI,
  AEC_STROBE_CFG_ST,
  AEC_EZ_DISABLE, /* 20 */
  AEC_EZ_LOCK_OUTPUT,
  AEC_EZ_FORCE_EXP,
  AEC_EZ_FORCE_LINECOUNT,
  AEC_EZ_FORCE_GAIN,
  AEC_EZ_TEST_ENABLE, /* 25 */
  AEC_EZ_TEST_ROI,
  AEC_EZ_TEST_MOTION,
  AEC_EZ_FORCE_SNAPSHOT,
  AEC_EZ_FORCE_SNAP_LINECOUNT,
  AEC_EZ_FORCE_SNAP_GAIN, /* 30 */
  AEC_SET_LOCK,
  AEC_SET_SENSITIVITY_RATIO,
  AEC_SET_LED_EST,
} stats_proc_aec_set_t;

typedef struct {
  stats_proc_aec_set_t type;
  union {
    uint32_t                        aec_exp_comp;
    uint32_t                        aec_brightness;
    uint32_t                        aec_motion_iso;
    uint32_t                        aec_af_hjr;
    uint32_t                        aec_fps;
    uint32_t                        aec_redeye_reduction_mode;
    camera_auto_exposure_mode_type  aec_metering;
    stats_proc_antibanding_type     aec_atb;
    int                             aec_atb_status;
    int                             aec_iso_mode;
    stats_proc_fps_mode_type        aec_fps_mode;
    stats_proc_interested_region_t  aec_roi;
    int                             aec_strobe_mode;
    stats_proc_strobe_confg_st_type aec_strobe_cfg_st;
    uint32_t                        aec_sensor_update_ok;
    camera_bestshot_mode_type       bestshot_mode;
    stats_proc_roi_info_t           fd_roi;
    stats_proc_mtr_area_t           mtr_area;
    int                             force_aec_lock;
    int                             ez_disable;
    int                             ez_lock_output;
    int                             ez_force_exp;
    uint32_t                        ez_force_linecount;
    float                           ez_force_gain;
    int                             ez_force_snapshot;
    uint32_t                        ez_force_snap_linecount;
    float                           ez_force_snap_gain;
    int                             ez_test_enable;
    int                             ez_test_roi;
    int                             ez_test_motion;
    float                           sensitivity_ratio;
    int                             led_est_state;
  } d;
} stats_proc_set_aec_data_t;

/* AWB SET DATA */
typedef enum {
  AWB_WHITE_BALANCE,
  AWB_RESTORE_LED_GAINS,
  AWB_COLOR_CONV_STATE,  /* todo should be removed*/
  AWB_BESTSHOT,
  AWB_EZ_DISABLE,
  AWB_EZ_LOCK_OUTPUT,
  AWB_LINEAR_GAIN_ADJ,
} awb_set_t;

typedef struct {
  awb_set_t type;
  union {
    uint32_t                          awb_current_wb;
    stats_proc_color_conv_state_type  awb_color_conv_state; /* todo should be removed*/
    camera_bestshot_mode_type         bestshot_mode;
    int                               ez_disable;
    int                               ez_lock_output;
    int                               linear_gain_adj;
  } d;
} stats_proc_set_awb_data_t;

/* AF SET DATA */
typedef enum {
  AF_RESET_LENS,
  AF_SEARCH_MODE,
  AF_METERING_MODE,
  AF_START_PARAMS,
  AF_MOVE_LENS,
  AF_RESET_MOVE_LENS_STATUS,
  AF_LENS_MOVE_DONE,
  AF_CANCEL,
  AF_CONTINUOS_FOCUS,
  AF_STOP,
  AF_BESTSHOT,
  AF_FOCUSRECT,
  AF_MODE,
  AF_ROI,
  AF_FACE_DETECTION_ROI,
  AF_LOCK_CAF,
  AF_SEND_CAF_DONE_EVT_LTR,
  AF_EZ_DISABLE,
  AF_STATE,
} af_set_t;

typedef struct {
  af_set_t type;
  union {
    int                       af_search_mode;
    int                       af_metering_mode;
    int                       af_step;
    int                       af_conti_focus;
    int                       af_lens_move_status;
    int                       af_stop;
    int                       af_state;
    int                       cur_focusrect_value;
    int                       cur_af_mode;
    int                       af_lock_caf;
    int                       af_send_evt_ltr;
    stats_proc_roi_info_t     roiInfo;
    camera_bestshot_mode_type bestshot_mode;
    int                       af_mode;
    int                       ez_disable;
  } d;
} stats_proc_set_af_data_t;

/* ASD SET DATA */
typedef enum {
  ASD_ENABLE,
  ASD_BESTSHOT,
} asd_set_t;

typedef struct {
  asd_set_t type;
  union {
    int                       asd_enable;
    camera_bestshot_mode_type bestshot_mode;
  } d;
} stats_proc_set_asd_data_t;

/* DIS SET DATA */
typedef enum {
  DIS_S_INIT_DATA,
  DIS_S_DEINIT_DATA,
} dis_set_t;

typedef struct {
  mctl_dis_frame_cfg_type_t frame_cfg;
  vfe_stats_rs_cs_params rs_cs_parm;
} stats_proc_dis_init_data_t;

typedef struct {
  dis_set_t type;
  union {
    stats_proc_dis_init_data_t init_data;
  } d;
} stats_proc_set_dis_data_t;

/* AFD SET DATA */
typedef enum {
  AFD_RESET,
  AFD_ENABLE,
} afd_set_t;

typedef struct {
  afd_set_t type;
    int afd_enable;
    camera_antibanding_type afd_mode;
} stats_proc_set_afd_data_t;

typedef enum {
  EZ_TUNE_ENABLE,
} eztune_set_t;

typedef struct {
  eztune_set_t type;
  union {
    int eztune_enable;
  } d;
} stats_proc_set_eztune_data_t;

typedef enum {
  MOBICAT_ENABLE,
} mobicat_set_t;

typedef struct {
  mobicat_set_t type;
  union {
    int mobicat_enable;
  } d;
} stats_proc_set_mobicat_data_t;

typedef struct {
  stats_proc_type_t type;
  union {
    stats_proc_set_aec_data_t     set_aec;
    stats_proc_set_awb_data_t     set_awb;
    stats_proc_set_af_data_t      set_af;
    stats_proc_set_asd_data_t     set_asd;
    stats_proc_set_dis_data_t     set_dis;
    stats_proc_set_afd_data_t     set_afd;
    stats_proc_set_eztune_data_t  set_eztune;
    stats_proc_set_mobicat_data_t set_mobicat;
  } d;
} stats_proc_set_t;

/********************************
     STATS_PROC Interface APIs
*********************************/
//uint32_t stats_proc_interface_create(void);
//void stats_proc_interface_destroy(uint32_t handle);
//int stats_proc_init(uint32_t handle, stats_proc_interface_input_t *init_data);
//int stats_proc_get_params(uint32_t handle, stats_proc_get_t *stats_proc_get);
//int stats_proc_set_params(uint32_t handle, stats_proc_set_t *stats_proc_set,
//  stats_proc_interface_t *stats_proc_intf);
//int stats_proc_process(uint32_t handle,
//  stats_proc_interface_t *stats_proc_intf);

int STATSPROC_comp_create();
uint32_t STATSPROC_client_open(module_ops_t *ops);
int STATSPROC_comp_destroy();

#endif /* __STATS_PROC_INTERFACE_H__ */
