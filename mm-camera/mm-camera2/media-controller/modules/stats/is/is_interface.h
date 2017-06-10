/* is_interface.h
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __IS_INTERFACE_H__
#define __IS_INTERFACE_H__


#include "modules.h"


/** is_set_parameter_type:
 * List of IS parameters that can be set by other components.
 **/
typedef enum {
  IS_SET_PARAM_STREAM_CONFIG = 1,
  IS_SET_PARAM_DIS_CONFIG,
  IS_SET_PARAM_OUTPUT_DIM,
  IS_SET_PARAM_IS_ENABLE
} is_set_parameter_type;


/** is_sensor_info:
 *    @sensor_mount_angle: sensor mount angle (0, 90, 180, 270)
 *    @camera_position: camera position (front or back)
 **/
typedef struct _is_sensor_info {
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
} is_sensor_info_t;


/** is_output_dim_info:
 *    @is_mode: Selected IS technology (DIS = 1, Gyro-assisted DIS = 2, etc.)
 *    @vfe_width: VFE width (image width + margin)
 *    @vfe_height: VFE height (image height + margin)
 **/
typedef struct _is_output_dim_info {
  cam_is_type_t is_mode;
  int32_t vfe_width;
  int32_t vfe_height;
} is_output_dim_info_t;

/** _is_set_parameter:
 *    @type: parameter type
 *
 * Used for setting IS parameters
 **/
typedef struct _is_set_parameter {
  is_set_parameter_type type;

  union {
    is_sensor_info_t is_sensor_info;
    isp_dis_config_info_t is_config_info;
    is_output_dim_info_t is_output_dim_info;
    int32_t is_enable;
  } u;
} is_set_parameter_t;


/** rs_cs_config_t
 *    @num_row_sum: number of row sums
 *    @num_col_sum: number of column sums
 */
typedef struct {
  unsigned long num_row_sum;
  unsigned long num_col_sum;
} rs_cs_config_t;

typedef struct {
  int32_t  dis_frame_width;   /* Original */
  int32_t  dis_frame_height;  /* Original */
  int32_t  vfe_output_width;  /* Padded */
  int32_t  vfe_output_height; /* Padded */
  uint16_t frame_fps;         /* image fps */
} frame_cfg_t;

typedef struct {
  unsigned long long sof;
  unsigned long long frame_time;
  float exposure_time;
} frame_times_t;

typedef struct {
  frame_cfg_t frame_cfg;
  rs_cs_config_t rs_cs_config;
  cam_is_type_t is_mode;
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
} is_init_data_t;

typedef struct {
  unsigned long frame_id;
  int has_output;
  int x;
  int y;
  int prev_dis_output_x;
  int prev_dis_output_y;
  unsigned long eis_output_valid;
  int eis_output_x;
  int eis_output_y;

  float transform_matrix[9];

  /* Time interval used to calculate EIS offsets */
  unsigned long long t_start;
  unsigned long long t_end;
} is_output_type;

#endif /* _IS_INTERFACE_H_ */
