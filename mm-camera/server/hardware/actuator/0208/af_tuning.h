/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __AF_TUNING_H__
#define __AF_TUNING_H__

#include <media/msm_camera.h>


/******************************************************************************
AF data types
******************************************************************************/
typedef struct
{
  unsigned short      minimumY;
  unsigned short      maximumY;
  float       horizontalOffsetRatio;
  float       verticalOffsetRatio;
  float       horizontalClipRatio;
  float       verticalClipRatio;
} af_vfe_config_t;

typedef struct
{
  char      a00;
  char      a02;
  char      a04;
  char      a20;
  char      a21;
  char      a22;
  char      a23;
  char      a24;
} af_vfe_hpf_t;

typedef struct
{
  int AF_shake_resistant_enable;
  float AF_max_gain;
  unsigned char AF_min_frame_luma ;
  float AF_tradeoff_ratio;
  unsigned char AF_shake_resistante_toggle_frame_skip;
}AF_shake_resistant_t;

typedef struct {
  // def: 0.9 - threshold to determine if FV curve is flat
  float af_fv_curve_flat_threshold;
  // default: 0.9
  float af_slope_threshold1;
  // default: 1.1
  float af_slope_threshold2;
  // default: 0.5
  float af_slope_threshold3;
  // default: 3
  float af_slope_threshold4;
  // Lens poisiton when the object is at 3m
  unsigned int af_lens_pos_0;
  // Lens poisiton when the object is at 70cm
  unsigned int af_lens_pos_1;
  // Lens poisiton when the object is at 30cm
  unsigned int af_lens_pos_2;
  // Lens poisiton when the object is at 20cm
  unsigned int af_lens_pos_3;
  // Lens poisiton when the object is at 10cm
  unsigned int af_lens_pos_4;
  // Lens poisiton Macro
  unsigned int af_lens_pos_5;
  // default: 1
  unsigned int af_sp_frame_delay;
  // max number of consecutive downhill in the first 4 or 6 samples
  int af_downhill_allowance;
  // max number of consecutive downhill in 2 or 3 round
  int af_downhill_allowance_1;
} AF_slope_predictive_t;

struct msm_actuator_reg_tbl_t {
  uint8_t reg_tbl_size;
  struct msm_actuator_reg_params_t reg_params[MAX_ACTUATOR_REG_TBL_SIZE];
};

struct damping_t {
  struct damping_params_t ringing_params[MAX_ACTUATOR_REGION];
};

typedef struct {
  uint16_t scenario_size[NUM_ACTUATOR_DIR];
  uint16_t ringing_scenario[NUM_ACTUATOR_DIR][MAX_ACTUATOR_SCENARIO];
  int16_t initial_code;
  uint16_t region_size;
  struct region_params_t region_params[MAX_ACTUATOR_REGION];
  struct damping_t damping[NUM_ACTUATOR_DIR][MAX_ACTUATOR_SCENARIO];
} actuator_tuned_params_t;

typedef struct {
  uint32_t i2c_addr;
  enum msm_actuator_data_type i2c_data_type;
  enum msm_actuator_addr_type i2c_addr_type;
  enum actuator_type act_type;
  uint16_t data_size;
  uint8_t af_restore_pos;
  struct msm_actuator_reg_tbl_t reg_tbl;
  uint16_t init_setting_size;
  struct reg_settings_t init_settings[MAX_ACTUATOR_INIT_SET];
} actuator_params_t;

typedef struct
{
  int CAF_enable;
  unsigned char   af_scene_change_detection_ratio;
  float           af_panning_stable_fv_change_trigger;
  float           af_panning_stable_fvavg_to_fv_change_trigger;
  unsigned short  af_panning_unstable_trigger_cnt;
  unsigned short  af_scene_change_trigger_cnt;
  unsigned long   af_downhill_allowance;
  unsigned short  af_cont_base_frame_delay;
  unsigned short  af_cont_lux_index_change_threshold;
  float           af_cont_threshold_in_noise;
  unsigned short  af_cont_search_step_size;
  unsigned short  af_search_mode;
}AF_CAF_t;

typedef struct{
    float af_gyro_trigger;
    float af_accelerometer_trigger;
    float af_magnetometer_trigger;
    float af_DIS_motion_vector_trigger;
}AF_motion_sensor_t;

typedef struct{
  uint16_t header_version;
  enum af_camera_name cam_name;
  char module_name[MAX_ACT_MOD_NAME_SIZE];
  char actuator_name[MAX_ACT_NAME_SIZE];
}AF_header_info;

typedef struct {
  AF_header_info af_header_info;
  unsigned short num_gross_steps_between_stat_points;
  unsigned short num_fine_steps_between_stat_points;
  unsigned short num_fine_search_points;
  unsigned short af_process_type;
  unsigned short position_near_end;
  unsigned short position_default_in_macro;
  unsigned short position_boundary;
  unsigned short position_default_in_normal;
  unsigned short position_far_end;
  unsigned short undershoot_protect;
  unsigned short undershoot_adjust;
  unsigned short reset_lens_after_snap;
  af_vfe_config_t af_config;
  unsigned short  fv_metric; // 0 = Sum of FV, 1 = Max of FV. Default = 1
  af_vfe_hpf_type af_vfe_hpf;
  AF_shake_resistant_t AF_shake_resistant;
  unsigned char AF_scene_change_detection_ratio;
  float AF_peak_drop_down_factor;
  AF_CAF_t af_CAF;
  int basedelay_snapshot_AF;
  AF_motion_sensor_t AF_motion_sensor;
  int led_af_assist_enable;
  long led_af_assist_trigger_idx;

  AF_slope_predictive_t AF_slope_predictive;

  actuator_params_t actuator_params;

  actuator_tuned_params_t actuator_tuned_params;
}   af_tune_parms_t;

#endif /* __AF_TUNING_H__ */
