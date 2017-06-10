/* eis2_interface.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "is_interface.h"
#include "eis2_interface.h"
#include "camera_dbg.h"
#include <time.h>

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#define USEC_PER_SEC     1000000.0
#define Q16              (1 << 16)


/** eis2_align_gyro_to_camera:
 *    @gyro_position: gyro integrated angle aligned with camera coordinate
 *       system
 *    @gyro_angle: gyro integrated angle
 *    @camera_mount_angle: camera mount angle (0, 90, 180, 270 degrees)
 *    @camera_posistion: camera position (front or back)
 *
 * This function aligns the gyro data to match the camera coordinate system.
 *
 * Returns 0 on success, -1 otherwise.
 **/
static int eis2_align_gyro_to_camera(gyro_data_s gyro_data,
  unsigned int camera_mount_angle, enum camb_position_t camera_position)
{
  int err = 0;
  unsigned int i;
  long temp;

  if (camera_position == BACK_CAMERA_B) {
    switch (camera_mount_angle) {
    case 270:
      /* No Negation.  No axes swap */
      break;

    case 180:
      /* Negate y.  Swap x, y axes */
      for (i = 0; i < gyro_data.num_elements; i++) {
        temp = gyro_data.gyro_data[i].data[0];
        gyro_data.gyro_data[i].data[0] = -gyro_data.gyro_data[i].data[1];
        gyro_data.gyro_data[i].data[1] = temp;
      }
      break;

    case 90:
      /* Negate x, y.  No axes swap */
      for (i = 0; i < gyro_data.num_elements; i++) {
        gyro_data.gyro_data[i].data[0] = -gyro_data.gyro_data[i].data[0];
        gyro_data.gyro_data[i].data[1] = -gyro_data.gyro_data[i].data[1];
      }
      break;

    case 0:
      /* Negate x.  Swap x, y axes */
      for (i = 0; i < gyro_data.num_elements; i++) {
        temp = gyro_data.gyro_data[i].data[0];
        gyro_data.gyro_data[i].data[0] = gyro_data.gyro_data[i].data[1];
        gyro_data.gyro_data[i].data[1] = -temp;
      }
      break;

    default:
      err = -1;
    }
  } else if (camera_position == FRONT_CAMERA_B) {
    switch (camera_mount_angle) {
    case 270:
      /* Negate y, z.  No axes swap */
      for (i = 0; i < gyro_data.num_elements; i++) {
        gyro_data.gyro_data[i].data[1] = -gyro_data.gyro_data[i].data[1];
        gyro_data.gyro_data[i].data[2] = -gyro_data.gyro_data[i].data[2];
      }
      break;

    case 180:
      /* Negate z.  Swap x, y axes */
      for (i = 0; i < gyro_data.num_elements; i++) {
        temp = gyro_data.gyro_data[i].data[0];
        gyro_data.gyro_data[i].data[0] = gyro_data.gyro_data[i].data[1];
        gyro_data.gyro_data[i].data[1] = temp;
        gyro_data.gyro_data[i].data[2] = -gyro_data.gyro_data[i].data[2];
      }
      break;

    case 90:
      /* Negate x, z.  No axes swap */
      for (i = 0; i < gyro_data.num_elements; i++) {
        gyro_data.gyro_data[i].data[0] = -gyro_data.gyro_data[i].data[0];
        gyro_data.gyro_data[i].data[2] = -gyro_data.gyro_data[i].data[2];
      }
      break;

    case 0:
      /* Negate x, y, z.  Swap x, y axes */
      for (i = 0; i < gyro_data.num_elements; i++) {
        temp = gyro_data.gyro_data[i].data[0];
        gyro_data.gyro_data[i].data[0] = -gyro_data.gyro_data[i].data[1];
        gyro_data.gyro_data[i].data[1] = -temp;
        gyro_data.gyro_data[i].data[2] = -gyro_data.gyro_data[i].data[2];
      }
      break;

    default:
      err = -1;
    }
  } else {
    err = -1;
  }

  return err;
}


/** eis2_process:
 *    @p_eis: EIS 2.0 context
 *    @frame_times: times associated with current frame (SOF, exposure, etc.)
 *    @is_output: place to put EIS 2.0 algorithm result
 *
 * Returns 0 on success.
 **/
int eis2_process(eis2_context_type *p_eis, frame_times_t *frame_times,
  is_output_type *is_output)
{
  uint8_t i;
  int rc = 0;
  eis2_position_type dis_position;
  eis2_position_type gyro_data;
  eis_input_type eis_input;
  uint64_t sof, eof;
  struct timespec t_now;

  gyro_data_s eis2_gyro_data[2];
  int idx;

  eis2_gyro_data[0].gyro_data = p_eis->gyro_samples_rs;
  eis2_gyro_data[1].gyro_data = p_eis->gyro_samples_3dr;

  clock_gettime( CLOCK_REALTIME, &t_now );

  if (p_eis == NULL)
    return -1;

  CDBG("%s, time %llu, frame_id %lu", __FUNCTION__,
    (((int64_t)t_now.tv_sec * 1000) + t_now.tv_nsec/1000000),
    is_output->frame_id);

  dis_position.x = is_output->prev_dis_output_x;
  dis_position.y = is_output->prev_dis_output_y;

  /* Fill in EIS input structure */
  eis_input.dis_offset.x = dis_position.x;
  eis_input.dis_offset.y = dis_position.y;
  sof = frame_times->sof;
  eof = sof + frame_times->frame_time;
  eis_input.ts = ((sof + eof) / 2 -
                  frame_times->exposure_time * USEC_PER_SEC / 2) /
                 USEC_PER_SEC * Q16;
  eis_input.crop_factor = 4096.0;
  eis_input.exposure = frame_times->exposure_time;
  eis_input.time_interval[0].t_start = sof -
    eis_input.exposure * USEC_PER_SEC / 2;
  eis_input.time_interval[0].t_end = eof -
    eis_input.exposure * USEC_PER_SEC / 2;

  eis_input.time_interval[1].t_end = (sof + eof) / 2 -
    frame_times->exposure_time * USEC_PER_SEC / 2;
  eis_input.time_interval[1].t_start = p_eis->prev_frame_time ?
    p_eis->prev_frame_time : eis_input.time_interval[1].t_end - 15000;

  p_eis->prev_frame_time = eis_input.time_interval[1].t_end;

  CDBG("%s: frame_id = %lu, sof = %.6f, frame time = %.6f, exposure time, %f, "
    "t0 = %.6f, t1 = %.6f, t2 = %.6f, t3 = %.6f", __func__, is_output->frame_id,
    (double)sof/USEC_PER_SEC, (double)frame_times->frame_time,
    frame_times->exposure_time,
    eis_input.time_interval[0].t_start / USEC_PER_SEC,
    eis_input.time_interval[0].t_end / USEC_PER_SEC,
    eis_input.time_interval[1].t_start / USEC_PER_SEC,
    eis_input.time_interval[1].t_end / USEC_PER_SEC);

  eis2_gyro_data[0].num_elements = 0;
  eis2_gyro_data[1].num_elements = 0;
  get_gyro_samples(&eis_input.time_interval[0], &eis2_gyro_data[0]);
  get_gyro_samples(&eis_input.time_interval[1], &eis2_gyro_data[1]);

  eis_input.gyro_samples_rs = &eis2_gyro_data[0];
  eis_input.gyro_samples_3dr = &eis2_gyro_data[1];

  /* Adjust time interval for rolling shutter if samples are not complete */
  if ((eis2_gyro_data[0].num_elements != 0) &&
      (eis_input.time_interval[0].t_end >
       eis2_gyro_data[0].gyro_data[eis2_gyro_data[0].num_elements-1].ts)) {
    CDBG("%s: Shorten interval, change t_end from %llu to %llu", __func__,
      eis_input.time_interval[0].t_end,
      eis2_gyro_data[0].gyro_data[eis2_gyro_data[0].num_elements-1].ts);
    eis_input.time_interval[0].t_end =
      eis2_gyro_data[0].gyro_data[eis2_gyro_data[0].num_elements-1].ts;
  }

  eis2_align_gyro_to_camera(eis2_gyro_data[0], p_eis->sensor_mount_angle,
    p_eis->camera_position);
  eis2_align_gyro_to_camera(eis2_gyro_data[1], p_eis->sensor_mount_angle,
    p_eis->camera_position);

  CDBG("ggs t0-t1: # elements %d", eis2_gyro_data[0].num_elements);
  for (i = 0; i < eis2_gyro_data[0].num_elements; i++) {
    CDBG("ggs_rs: (x, y, z, ts) = %d, %d, %d, %llu",
      eis2_gyro_data[0].gyro_data[i].data[0],
      eis2_gyro_data[0].gyro_data[i].data[1],
      eis2_gyro_data[0].gyro_data[i].data[2],
      eis2_gyro_data[0].gyro_data[i].ts);
  }
  CDBG("ggs t2-t3: # elements %d", eis2_gyro_data[1].num_elements);
  for (i = 0; i < eis2_gyro_data[1].num_elements; i++) {
    CDBG("ggs_3d: (x, y, z, ts) = %d, %d, %d, %llu",
      eis2_gyro_data[1].gyro_data[i].data[0],
      eis2_gyro_data[1].gyro_data[i].data[1],
      eis2_gyro_data[1].gyro_data[i].data[2],
      eis2_gyro_data[1].gyro_data[i].ts);
  }

  /* Stabilize only when both RS and 3D shake gyro intervals are not empty */
  if (eis2_gyro_data[0].num_elements > 0 &&
    eis2_gyro_data[1].num_elements > 0) {
    rc = eis2_stabilize_frame(p_eis, &eis_input, is_output->transform_matrix);
  } else {
    CDBG_HIGH("%s: Gyro sample interval empty", __func__);
  }

  CDBG("tform mat: %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
    is_output->transform_matrix[0], is_output->transform_matrix[1],
    is_output->transform_matrix[2], is_output->transform_matrix[3],
    is_output->transform_matrix[4], is_output->transform_matrix[5],
    is_output->transform_matrix[6], is_output->transform_matrix[7],
    is_output->transform_matrix[8]);

  return rc;
}


/** eis2_initialize:
 *    @eis: EIS2 context
 *    @data: initialization parameters
 *
 * This function initializes the EIS2 algorithm.
 *
 * Returns 0 on success.
 **/
int eis2_initialize(eis2_context_type *eis, is_init_data_t *data)
{
  int rc = 0;
  eis_init_type init_param;
  frame_cfg_t *frame_cfg = &data->frame_cfg;
  rs_cs_config_t *rs_cs_config = &data->rs_cs_config;

  init_param.sensor_mount_angle = data->sensor_mount_angle;
  init_param.camera_position = data->camera_position;

  init_param.width = frame_cfg->dis_frame_width;
  init_param.height = frame_cfg->dis_frame_height;

  init_param.margin_x = (frame_cfg->vfe_output_width -
      frame_cfg->dis_frame_width) / 2;
  init_param.margin_y = (frame_cfg->vfe_output_height -
      frame_cfg->dis_frame_height) / 2;

  CDBG_HIGH("%s: init_param->margin_x = %u", __func__, init_param.margin_x);
  CDBG_HIGH("%s: init_param->margin_y = %u", __func__, init_param.margin_y);

  if (eis2_init(eis, &init_param) > 0) {
    CDBG_HIGH("%s: eis_init failed \n", __func__);
    rc = -1;
  }
  return rc;
}


/** eis_deinitialize:
 *    @eis: EIS context
 *
 * This function deinits the EIS algorithm.
 *
 * Returns 0 on success.
 **/
int eis2_deinitialize(eis2_context_type *eis)
{
  int rc;

  rc = eis2_exit(eis);
  return rc;
}
