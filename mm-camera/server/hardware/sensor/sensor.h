/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __SENSOR_H__
#define __SENSOR_H__

/*============================================================================
                        INCLUDE FILES
============================================================================*/
#include "tgtcommon.h"
#include "chromatix.h"
#include "sensor_interface.h"

/*============================================================================
                        INTERNAL FEATURES
============================================================================*/
/*============================================================================
                        CONSTANTS
============================================================================*/
#define SENSOR_MODEL_NO 32
#define CLIPI(a) ( ((a<0)||(a>255)) \
                 ? ((a<0)?0:255) \
                 :a);
#undef Q14
#define Q14   0x00004000
#define Q13   0x00002000
#define Q12   0x00001000
#define Q12_2 0x00002000
#define Q12_4 0x00004000
#define Q11   0x00000800
#define Q10   0x00000400
#define Q8    0x00000100
#define Q7    0x00000080
#define Q6    0x00000040
#define Q4    0x00000010

/*============================================================================
                        MACROS
============================================================================*/

/*============================================================================
                        EXTERNAL ABSTRACT DATA TYPES
============================================================================*/
typedef enum {
  SENSOR_RES_0,
  SENSOR_RES_1,
  SENSOR_RES_2,
  SENSOR_RES_3,
} sensor_res_t;


typedef struct {
  chromatix_parms_type *chromatix_ptr;
} sensor_from_chromatix_3a_t;

/*****************************************************
*  enum
*****************************************************/
typedef enum {
  SENSOR_CCD,
  SENSOR_CMOS,
  SENSOR_INVALID_SENSOR
} sensor_sensor_t;

/* remove this after vfe clean up */
typedef struct {
  /* 12-bit, {Q7,Q8,Q9,Q10}, signed */
  /* R */
  int32_t c0;
  int32_t c1;
  int32_t c2;
  /* G */
  int32_t c3;
  int32_t c4;
  int32_t c5;
  /* B */
  int32_t c6;
  int32_t c7;
  int32_t c8;
  /* 11-bit, Q0, signed */
  int16_t k0;
  /* 11-bit, Q0, signed */
  int16_t k1;
  /* 11-bit, Q0, signed */
  int16_t k2;
  /* range 0x0-0x3 maps to Q7-Q10 */
  uint8_t q_factor;
} sensor_color_correction_t;

typedef struct {
  int8_t (*sensor_set_op_mode) (void *, uint8_t);
  int8_t (*sensor_get_mode_aec_info) (void *, void *);
  int8_t (*sensor_get_dim_info) (void *, void *);
  int8_t (*sensor_get_preview_fps_range) (void *, void *);
  int8_t(*sensor_set_frame_rate) (void *, uint16_t);
  uint16_t (*sensor_get_snapshot_fps) (void *, uint16_t);

  int8_t(*sensor_set_exposure_gain) (void *, void *);
  int8_t(*sensor_set_snapshot_exposure_gain) (void *, void *);
  int32_t (*sensor_write_exp_gain)(void *, uint16_t, uint32_t);
  float (*sensor_register_to_real_gain) (uint16_t);
  uint16_t(*sensor_real_to_register_gain) (float);
  int8_t (*sensor_get_max_supported_hfr_mode)(void *, void *);
  int8_t (*sensor_get_cur_fps)(void *, void *);
  int8_t (*sensor_get_lens_info)(void *, void *);
  uint8_t (*sensor_set_saturation) (void * , int32_t);
  uint8_t (*sensor_set_contrast) (void * , int32_t);
  uint8_t (*sensor_set_sharpness) (void * , int32_t);
  uint8_t (*sensor_set_exposure_compensation) (void * , int32_t);
  uint8_t (*sensor_set_iso) (void * , int32_t);
  uint8_t (*sensor_set_special_effect) (void * , int32_t);
  uint8_t (*sensor_set_antibanding) (void * , int32_t);
  uint8_t (*sensor_set_wb_oem) (void * , int32_t);
  int8_t (*sensor_set_start_stream) (void *);
  int8_t (*sensor_set_stop_stream) (void *);
  int8_t (*sensor_get_csi_params)(void *, void *);
  int8_t (*sensor_set_config_setting)(void *, void *);
  int8_t (*sensor_get_camif_cfg) (void *, void *);
  int8_t (*sensor_get_output_cfg) (void *, void *);
  int8_t (*sensor_get_digital_gain) (void *, void *);
  int8_t (*sensor_get_cur_res) (void *, uint16_t, void *);
} sensor_function_table_t;


typedef struct {
  uint16_t width;
  uint16_t height;
  uint16_t top_crop;
  uint16_t bottom_crop;
  uint16_t left_crop;
  uint16_t right_crop;
  float    fps;
  float    nightshot_fps;
  sensor_camif_inputformat_t input_format;
} sensor_dimension_t;

typedef struct {
	uint16_t top_crop;
	uint16_t bottom_crop;
	uint16_t left_crop;
	uint16_t right_crop;
} sensor_crop_parms_t;

typedef struct {
  sensor_mode_t prev_op_mode;
  sensor_mode_t op_mode;

  /* Make/model number of sensor */
  char sensor_model_no[SENSOR_MODEL_NO];

  int cam_mode;
  uint32_t current_fps;
  uint32_t current_fps_div;
  uint32_t current_gain;
  uint32_t current_linecount;
  uint32_t snapshot_exp_wait_frames;
  struct msm_sensor_output_info_t output_info[MSM_SENSOR_INVALID_RES];
  sensor_camif_inputformat_t *inputformat;
  char **sensor_load_chromatixfile;
  sensor_crop_parms_t *crop_info;
  sensor_csi_params_t sensor_csi_params;
  uint32_t *mode_res;
  int32_t num_res;
  uint32_t pending_fps;
  int32_t cur_res;
  int32_t cur_frame_length_lines;
  int32_t cur_line_length_pclk;

  sensor_data_t out_data;
} sensor_params_t;

typedef struct {
  const char *sname;
  int8_t (*s_start)(void *);
  char **sensor_load_chromatixfile;
} sensor_proc_start_t;

typedef struct {
    double left_p_matrix[3][4];
    double right_p_matrix[3][4];
    double square_len;
    double focal_len;
    double pixel_pitch;
} sensor_3d_cali_data;

typedef struct {
  uint16_t rave_over_grave;
  uint16_t bave_over_gbave;
  uint16_t grave_over_gbave;
  uint8_t lsc_data[1108];
} sensor_2d_cali_data;

typedef struct {
  int sfd;
  sensor_proc_start_t *start;
  chromatix_parms_type chromatixData;
  sensor_params_t sensor;
  sensor_function_table_t *fn_table;
  sensor_load_chromatix_t chromatixType;
  sensor_3d_cali_data *cali_data_3d;
  sensor_2d_cali_data cali_data_2d;
  struct msm_camsensor_info sinfo;
  struct sensor_driver_params_type *driver_params;
} sensor_ctrl_t;

#if 0
typedef struct {
 sensor_t sensor;
} sensor_intf_t;
#endif
#define SENSOR_MAX_CLIENT_NUM 2

typedef struct {
  sensor_ctrl_t *sensorCtrl;
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint32_t handle;
  uint8_t my_comp_id;
  mctl_ops_t *ops;
} sensor_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t sensor_handle_cnt;
  sensor_client_t client[SENSOR_MAX_CLIENT_NUM];
} sensor_comp_root_t;

int8_t sensor_init(sensor_ctrl_t *);
int8_t sensor_process_start(sensor_ctrl_t *);
int8_t sensor_load_chromatix(sensor_ctrl_t *);
int8_t sensor_re_load_chromatix(sensor_ctrl_t *, sensor_load_chromatix_t);
#endif /* __SENSOR_H__ */
