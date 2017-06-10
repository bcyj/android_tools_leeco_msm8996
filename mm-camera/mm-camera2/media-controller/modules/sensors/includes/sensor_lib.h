/* sensor_common.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_LIB_H_
#define __SENSOR_LIB_H_

#include <media/msm_cam_sensor.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Non HFR mode for normal camera, camcorder usecases */
#define SENSOR_DEFAULT_MODE (1 << 0)
/* HFR mode used to capture slow motion video */
#define SENSOR_HFR_MODE (1 << 1)
/* HDR mode used to High Dynamic Range imaging */
#define SENSOR_HDR_MODE (1 << 2)
/* MOUNT ANGLE > = to this value is considered invalid in sensor lib */
#define SENSOR_MOUNTANGLE_360 360
/* Sensor mount angle. */
#define SENSOR_MOUNTANGLE_0 0
#define SENSOR_MOUNTANGLE_90 90
#define SENSOR_MOUNTANGLE_180 180
#define SENSOR_MOUNTANGLE_270 270

#define SENSOR_FLASH_SUPPORTED 1
#define SENSOR_FLASH_NOT_SUPPORTED 0
/* OEM's can extend these modes */

typedef enum {
  CAMIF_BAYER_G_B,
  CAMIF_BAYER_B_G,
  CAMIF_BAYER_G_R,
  CAMIF_BAYER_R_G,
  CAMIF_YCBCR_Y_CB_Y_CR,
  CAMIF_YCBCR_Y_CR_Y_CB,
  CAMIF_YCBCR_CB_Y_CR_Y,
  CAMIF_YCBCR_CR_Y_CB_Y,
  CAMIF_YCBCR_4_2_2_LINEPACKED, /* Only valid for Offline Snapshot */
  CAMIF_YCBCR_4_2_0_LINEPACKED, /* Only valid for Offline Snapshot */
  CAMIF_NUMBEROF_INPUTFORMATTYPE  /* Used for count purposes only */
} sensor_camif_inputformat_t;

typedef enum {
  SENSOR_BAYER,
  SENSOR_YCBCR
} sensor_output_format_t;

typedef enum {
  SENSOR_PARALLEL,
  SENSOR_MIPI_CSI,
  SENSOR_MIPI_CSI_1,
  SENSOR_MIPI_CSI_2,
} sensor_connection_mode_t;

typedef enum {
  SENSOR_8_BIT_DIRECT,
  SENSOR_10_BIT_DIRECT,
  SENSOR_12_BIT_DIRECT
} sensor_raw_output_t;

typedef enum {
  SENSOR_MODE_SNAPSHOT,
  SENSOR_MODE_RAW_SNAPSHOT,
  SENSOR_MODE_PREVIEW,
  SENSOR_MODE_VIDEO,
  SENSOR_MODE_VIDEO_HD,
  SENSOR_MODE_HFR_60FPS,
  SENSOR_MODE_HFR_90FPS,
  SENSOR_MODE_HFR_120FPS,
  SENSOR_MODE_HFR_150FPS,
  SENSOR_MODE_ZSL,
  SENSOR_MODE_INVALID,
} sensor_mode_t;

typedef enum {
  SENSOR_LOAD_CHROMATIX_PREVIEW,
  SENSOR_LOAD_CHROMATIX_VIDEO_DEFAULT,
  SENSOR_LOAD_CHROMATIX_VIDEO_HD,
  SENSOR_LOAD_CHROMATIX_VIDEO_HFR_60FPS,
  SENSOR_LOAD_CHROMATIX_VIDEO_HFR_90FPS,
  SENSOR_LOAD_CHROMATIX_VIDEO_HFR_120FPS,
  SENSOR_LOAD_CHROMATIX_VIDEO_HFR_150FPS,
  SENSOR_LOAD_CHROMATIX_AR,
  SENSOR_LOAD_CHROMATIX_ZSL,
  SENSOR_LOAD_CHROMATIX_MAX,
} sensor_load_chromatix_t;

typedef enum {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_START_STREAM,
  SENSOR_SET_NEW_RESOLUTION,
  SENSOR_SEND_EVENT,
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX,
} sensor_res_cfg_type_t;

typedef enum {
  SENSOR_SMETHOD_NOT_DEFINED = 1,
  SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR=2,
  SENSOR_SMETHOD_TWO_CHIP_COLOR_AREA_SENSOR=3,
  SENSOR_SMETHOD_THREE_CHIP_COLOR_AREA_SENSOR=4,
  SENSOR_SMETHOD_COLOR_SEQ_AREA_SENSOR=5,
  SENSOR_SMETHOD_TRILINEAR_SENSOR=7,
  SENSOR_SMETHOD_COLOR_SEQ_LINEAR_SENSOR=8
}sensor_sensing_method_type_t;

struct sensor_crop_parms_t {
  uint16_t top_crop;
  uint16_t bottom_crop;
  uint16_t left_crop;
  uint16_t right_crop;
} ;

typedef struct {
  sensor_output_format_t output_format;
  sensor_connection_mode_t connection_mode;
  sensor_raw_output_t raw_output;
} sensor_output_t;

typedef struct {
  sensor_mode_t op_mode;
  uint32_t pixels_per_line;
  uint32_t lines_per_frame;
  uint32_t pclk;
  uint32_t max_fps;
  float digital_gain;
  float stored_digital_gain;
  float max_gain;
  uint32_t max_linecount;
} sensor_aec_data_t;

typedef struct {
  uint64_t min_exposure_time;
  uint64_t max_exposure_time;
  uint32_t min_iso;
  uint32_t max_iso;
} sensor_manual_exposure_info_t;

typedef struct {
  float focal_length;
  float pix_size;
  float f_number;
  float total_f_dist;
  float hor_view_angle;
  float ver_view_angle;
  float um_per_dac;
  int   dac_offset;
  uint32_t near_end_distance;
  sensor_sensing_method_type_t sensing_method;
  float crop_factor; //depends on sensor physical dimentions
} sensor_lens_info_t;

/* sensor_lib_out_info_t: store information about different resolution
 * supported by sensor
 *
 * x_output: sensor output width (pixels)
 * y_output: sensor output height (pixels)
 * line_length_pclk: number of pixels per line
 * frame_length_lines: number of lines per frame
 * vt_pixel_clk: sensor scanning rate (cycles / sec)
 * op_pixel_clk: actual sensor output rate (cycles / sec)
 * binning_factor: 1 if average is taken, >1 if sum is taken (applies only
 *                 for if this resolution has binning)
 * min_fps: minumim fps that can be supported for this resolution
 * max_fps: maximum fps that can be supported for this sensor
 * mode: mode / modes for which this resolution can be used
 *       SENSOR_DEFAULT_MODE / SENSOR_HFR_MODE*/
struct sensor_lib_out_info_t {
  uint16_t x_output;
  uint16_t y_output;
  uint16_t line_length_pclk;
  uint16_t frame_length_lines;
  uint32_t vt_pixel_clk;
  uint32_t op_pixel_clk;
  uint16_t binning_factor;
  float    min_fps;
  float    max_fps;
  uint32_t mode;
};

struct sensor_lib_out_info_array {
  /* sensor output for each resolutions */
  struct sensor_lib_out_info_t *out_info;

  /* Number of valid entries in out_info array */
  uint16_t size;
};

struct sensor_lib_reg_settings_array {
  struct msm_camera_i2c_reg_setting *reg_settings;
  uint32_t                          size;
};

struct sensor_lib_csi_params_array {
  struct msm_camera_csi2_params **csi2_params;
  uint16_t size;
};

struct sensor_lib_crop_params_array{
  struct sensor_crop_parms_t *crop_params;
  uint16_t size;
};

struct sensor_lib_chromatix_t {
  char *common_chromatix;
  char *camera_preview_chromatix;
  char *camera_snapshot_chromatix;
  char *camcorder_chromatix;
  char *liveshot_chromatix;
};

struct sensor_lib_chromatix_array {
  struct sensor_lib_chromatix_t *sensor_lib_chromatix;
  uint16_t size;
};

struct sensor_res_cfg_table_t {
  /* Resolution configuration */
  sensor_res_cfg_type_t *res_cfg_type;

  /* Resolution configuration size */
  uint16_t size;
};

typedef struct {
  uint16_t reg_gain;
  uint16_t line_count;
  float digital_gain;
  float sensor_real_gain;
  float sensor_digital_gain;
  uint16_t use_long_exposure;
  uint32_t long_line_count;
} sensor_exposure_info_t;

struct sensor_pix_fmt_info_t {
  uint32_t fourcc;
};

typedef struct _sensor_stream_info_t {
  uint16_t vc_cfg_size;
  struct msm_camera_csid_vc_cfg *vc_cfg;
  struct sensor_pix_fmt_info_t *pix_fmt_fourcc;
} sensor_stream_info_t;

typedef struct _sensor_stream_info_array_t {
  sensor_stream_info_t *sensor_stream_info;
  uint16_t size;
} sensor_stream_info_array_t;

struct msm_sensor_output_info_t {
  uint16_t x_output;
  uint16_t y_output;
  uint16_t line_length_pclk;
  uint16_t frame_length_lines;
  uint32_t vt_pixel_clk;
  uint32_t op_pixel_clk;
  uint16_t binning_factor;
  uint16_t fps;
};

struct msm_sensor_output_reg_addr_t {
  uint16_t x_output;
  uint16_t y_output;
  uint16_t line_length_pclk;
  uint16_t frame_length_lines;
};

struct msm_sensor_exp_gain_info_t {
  uint16_t coarse_int_time_addr;
  uint16_t global_gain_addr;
  uint16_t vert_offset;
};

struct sensor_meta_data_out_info_t {
  int32_t width;
  int32_t height;
  int32_t fmt;
  int32_t ebd;
  enum sensor_stats_type stats_type;
};

struct sensor_lib_meta_data_info_array {
  /* meta data output */
  struct sensor_meta_data_out_info_t *meta_data_out_info;

  /* Number of valid entries in meta data array */
  uint16_t size;
};

typedef struct {
  /** Function to calculate exosure based on real gain and
   *  linecount value, 1st param - real gain, 2nd param -
   *  linecount, 3rd param - exposure info output, return staus -
   *  success / failure */
  int32_t (*sensor_calculate_exposure) (float, uint32_t,
    sensor_exposure_info_t *);

  /** Function to create register table from exposure settings
   * input param1 - register gain value
   * input param2 - coarse integration time value
   * input param3 - frame length line value
   * input param3 - register settings
   * return value - 0 for success and negative value for
   * failure **/
  int32_t (*sensor_fill_exposure_array)(uint16_t, uint32_t, uint32_t,
    int32_t, uint32_t, struct msm_camera_i2c_reg_setting *);
} sensor_exposure_table_t;

typedef struct {
  /** Function to create register table from awb settings
   *  success / failure */
  int32_t (*sensor_fill_awb_array)(uint16_t, uint16_t,
    struct msm_camera_i2c_seq_reg_setting *);
  /* sensor awb table size */
  uint16_t awb_table_size;
  /* video_hdr_capability */
  int video_hdr_capability;
} sensor_video_hdr_table_t;

typedef struct {
  int32_t width;
  int32_t height;
} msm_sensor_dimension_t;

typedef struct {
  /* sensor slave info */
  struct msm_camera_sensor_slave_info *sensor_slave_info;

  /* sensor info */
  struct msm_sensor_init_params *sensor_init_params;

  /* name of the AF actuator (if any)*/
  char* actuator_name;

  /* name of the eeprom (if any)*/
  char* eeprom_name;

  /* sensor output settings */
  sensor_output_t *sensor_output;

  /* sensor output register address */
  struct msm_sensor_output_reg_addr_t *output_reg_addr;

  /* sensor exposure gain register address */
  struct msm_sensor_exp_gain_info_t *exp_gain_info;

  /*sensor manual exposure info */
  sensor_manual_exposure_info_t *manual_exp_info;

  /* sensor aec info */
  sensor_aec_data_t *aec_info;

  /* sensor snapshot exposure wait frames info */
  uint16_t snapshot_exp_wait_frames;

  /* number of frames to skip after start stream info */
  uint16_t sensor_num_frame_skip;

  /* number of frames to skip after start HDR stream info */
  uint16_t sensor_num_HDR_frame_skip;

  /* sensor pipeline delay */
  uint32_t sensor_max_pipeline_frame_delay;

  /* sensor exposure table size */
  uint16_t exposure_table_size;

  /* sensor lens info */
  sensor_lens_info_t *default_lens_info;
  
  /* flag to sync exp and gain */
  uint8_t sync_exp_gain;

  /* csi lane params */
  struct csi_lane_params_t *csi_lane_params;

  /* csi cid params */
  struct msm_camera_csid_vc_cfg *csi_cid_params;

  /* sensor port info that consists of cid mask and fourcc mapaping */
  sensor_stream_info_array_t *sensor_stream_info_array;

  /* csi cid params size */
  uint16_t csi_cid_params_size;

  /* init settings */
  struct sensor_lib_reg_settings_array *init_settings_array;

  /* start settings */
  struct msm_camera_i2c_reg_setting *start_settings;

  /* stop settings */
  struct msm_camera_i2c_reg_setting *stop_settings;

  /* group on settings */
  struct msm_camera_i2c_reg_setting *groupon_settings;

  /* group off settings */
  struct msm_camera_i2c_reg_setting *groupoff_settings;

  /* resolution config table */
  struct sensor_res_cfg_table_t *sensor_res_cfg_table;

  /* resolution settings */
  struct sensor_lib_reg_settings_array *res_settings_array;
  struct sensor_lib_out_info_array     *out_info_array;
  struct sensor_lib_csi_params_array   *csi_params_array;
  struct sensor_lib_crop_params_array  *crop_params_array;
  struct sensor_lib_chromatix_array    *chromatix_array;

  /* video_hdr mode info*/
  struct sensor_lib_meta_data_info_array *meta_data_out_info_array;

  /* exposure funtion table */
  sensor_exposure_table_t *exposure_func_table;

  /* exposure info */
  sensor_exposure_info_t exposure_info;

  /* video hdr func table */
  sensor_video_hdr_table_t *video_hdr_awb_lsc_func_table;

  /* scale size tbl count*/
  uint8_t scale_tbl_cnt;
  /* function to get scale size tbl*/
  int32_t (*get_scale_tbl)(msm_sensor_dimension_t *);

  /* supported Scene mode */
  uint32_t *sensor_supported_scene_mode;

  /* supported effect mode */
  uint32_t *sensor_supported_effect_mode;

  /* sensor pipeline immediate delay */
  uint32_t sensor_max_immediate_frame_delay;

  /* library specific data */
  void *data;
} sensor_lib_t;

#endif /* __SENSOR_LIB_H_ */
