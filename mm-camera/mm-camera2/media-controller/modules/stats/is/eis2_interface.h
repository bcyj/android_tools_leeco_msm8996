/* eis2_interface.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EIS2_INTERFACE_H__
#define __EIS2_INTERFACE_H__
#include "is_interface.h"
#include "is_sns_lib.h"
#include "eis_common_interface.h"


/* EIS error codes */
#define EIS_SUCCESS             0
#define EIS_ERR_BASE            0x00
#define EIS_ERR_FAILED          (EIS_ERR_BASE + 1)
#define EIS_ERR_NO_MEMORY       (EIS_ERR_BASE + 2)
#define EIS_ERR_BAD_HANDLE      (EIS_ERR_BASE + 3)
#define EIS_ERR_BAD_POINTER     (EIS_ERR_BASE + 4)
#define EIS_ERR_BAD_PARAM       (EIS_ERR_BASE + 5)

/* Number of samples for correlation calculation */
#define CORR2_WINDOW_LEN 16


typedef struct
{
  long x;
  long y;
  long z;
} eis2_position_type;


typedef double mat3x3[3][3];


typedef struct {
  double w;
  double x;
  double y;
  double z;
} quaternion_type;


/** _eis2_context_type:
 *    @frame_num: current frame number
 *    @past_dis_offsets: recent history of DIS offsets
 *    @past_eis_offsets: recent history of EIS offsets
 *    @timestamp: timestamps of recent offsets
 *    @rolling_shutter_tform: rolling shutter correction transform
 *    @Qmat:
 *    @prev_Qnet:
 *    @Qnet:
 *    @projective_tform:
 *    @composite_tform:
 *    @bias: gyro sensor bias
 *    @kalman_params_x: Kalman filter parameters for x axis
 *    @kalman_params_y: Kalman filter parameters for y axis
 *    @alpha: pan filter coefficient
 *    @avg_gyro_stdev: filtered mean (over x, y, z) gyro standard deviation
 *    @crop_factor: zoom
 *    @margin_x:
 *    @margin_y:
 *    @width:
 *    @height:
 *    @sensor_mount_angle: sensor mount angle (0, 90, 180, 270)
 *    @camera_position: camera position (front or back)
 *    @res_scale:
 *    @gyro_samples_rs: gyro samples for calculating rolling shutter correction
 *       matrix
 *    @gyro_samples_3dr: gyro samples for calculating 3D shake rotation
 *       correction matrix
 *    @Rmat: for debug logging
 *    @pan: for debug logging
 *    @del_offset: for debug logging
 *
 * This structure maintains the EIS 2.0 alogirthm context.
 **/
typedef struct
{
  unsigned long frame_num;
  eis2_position_type past_dis_offsets[CORR2_WINDOW_LEN];
  eis2_position_type past_eis_offsets[CORR2_WINDOW_LEN];
  long long timestamp[CORR2_WINDOW_LEN];
  mat3x3 rolling_shutter_tform;
  quaternion_type Qmat;
  quaternion_type prev_Qnet;
  quaternion_type Qnet;
  mat3x3 projective_tform;
  mat3x3 composite_tform;
  int bias[3];
  eis_kalman_type kalman_params_x;
  eis_kalman_type kalman_params_y;
  double alpha;
  double avg_gyro_stdev;
  long crop_factor;
  double margin_x;
  double margin_y;
  unsigned long width;
  unsigned long height;
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
  double res_scale;
  unsigned long long prev_frame_time;
  gyro_sample_s gyro_samples_rs[100];
  gyro_sample_s gyro_samples_3dr[100];
  mat3x3 Rmat;
  double pan[3];
  double del_offset[2];
} eis2_context_type;


/** _eis2_input_type:
 *    @gyro_sample_rs: gyro samples for calculating rolling shutter transform
 *    @gyro_sample_3dr: gyro samples for calculating 3D rotation transform
 *    @time_interval: time intervals for calculating integrated gyro data and
 *      quaternion product
 *    @dis_offset: DIS offset in pixels, integer format
 *    @ts: frame timestamp, Q16 format
 *    @exposure: exposure time
 *    @crop_factor: zoom
 **/
typedef struct {
  gyro_data_s *gyro_samples_rs;
  gyro_data_s *gyro_samples_3dr;
  time_interval_s time_interval[2];
  eis2_position_type dis_offset;
  long long ts;
  double exposure;
  long crop_factor;
} eis_input_type;


int eis2_init (eis2_context_type *p_eis_context,
  const eis_init_type *p_init_param);
int eis2_stabilize_frame (eis2_context_type *p_context, eis_input_type *p_input,
  float *p_matrix);
int eis2_exit (eis2_context_type *p_eis_context);
int eis2_initialize(eis2_context_type *eis, is_init_data_t *data);
int eis2_process(eis2_context_type *eis, frame_times_t *frame_times,
  is_output_type *is_output);
int eis2_deinitialize(eis2_context_type *eis);
#endif
