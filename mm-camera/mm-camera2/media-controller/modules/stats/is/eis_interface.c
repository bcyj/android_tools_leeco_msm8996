/*==============================================================================
  Copyright (c) 2012 - 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 *============================================================================*/
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "modules.h"
#include "is_interface.h"
#include "eis_interface.h"
#include "camera_dbg.h"

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#undef CDBG_HIGH
#if 1
#define CDBG_HIGH ALOGE
#else
#define CDBG_HIGH
#endif


#define USEC_PER_SEC    (1000000L)


/** eis_align_gyro_to_camera:
 *    @gyro_position: gyro integrated angle aligned with camera coordinate
 *       system
 *    @gyro_angle: gyro integrated angle
 *    @camera_mount_angle: camera mount angle (0, 90, 180, 270 degrees)
 *    @camera_posistion: camera position (front or back)
 *
 * This aligns the gyro data to match the camera coordinate system.
 *
 * Returns 0 on success.
 **/
static int eis_align_gyro_to_camera(eis_position_type *gyro_position,
  int *gyro_angle, unsigned int camera_mount_angle,
  enum camb_position_t camera_position)
{
  int err = 0;

  if (camera_position == BACK_CAMERA_B)
  {
    switch (camera_mount_angle) {
    case 270:
      /* Negate x. Swap x, y axes */
      gyro_position->x = gyro_angle[1];
      gyro_position->y = -gyro_angle[0];
      break;

    case 180:
      /* No Negation.  No axes swap */
      gyro_position->x = gyro_angle[0];
      gyro_position->y = gyro_angle[1];
      break;

    case 90:
      /* Negate y.  Swap x, y axes */
      gyro_position->x = -gyro_angle[1];
      gyro_position->y = gyro_angle[0];
      break;

    case 0:
      /* Negate x, y.  No axes swap */
      gyro_position->x = -gyro_angle[0];
      gyro_position->y = -gyro_angle[1];
      break;

    default:
      err = -1;
    }
  }
  else if (camera_position == FRONT_CAMERA_B)
  {
    switch (camera_mount_angle) {
    case 270:
      /* Negate x, y.  Swap x, y axes */
      gyro_position->x = -gyro_angle[1];
      gyro_position->y = -gyro_angle[0];
      break;

    case 180:
      /* Negate y.  No axes swap */
      gyro_position->x = gyro_angle[0];
      gyro_position->y = -gyro_angle[1];
      break;

    case 90:
      /* No negation.  Swap x, y axes */
      gyro_position->x = gyro_angle[1];
      gyro_position->y = gyro_angle[0];
      break;

    case 0:
      /* Negate x.  No axes swap */
      gyro_position->x = -gyro_angle[0];
      gyro_position->y = gyro_angle[1];
      break;

    default:
      err = -1;
    }
  }
  else
    err = -1;

  if (err != 0) {
    gyro_position->x = 0;
    gyro_position->y = 0;
  }

  return err;
}


/** eis_process:
 *    @eis: EIS context
 *    @frame_times: times associated with the current frame (SOF, exposure
 *                  time, etc.)
 *    @is_output: IS algorithm output
 *
 * This function runs the EIS algorithm.
 *
 * Returns 0 on success.
 **/
int eis_process(eis_context_type *eis, frame_times_t *frame_times,
  is_output_type *is_output)
{
  int err = 0;
  eis_position_type dis_position;
  eis_position_type gyro_position;
  eis_position_type eis_output;

  time_interval_s t;
  unsigned long long eof;
  gyro_sample_s gyro_samples_1[64];
  gyro_data_s gyro_data;
  int angle[3];

  if (eis == NULL)
    return -1;

  is_output->eis_output_valid = 0;

  dis_position.x = is_output->prev_dis_output_x;
  dis_position.y = is_output->prev_dis_output_y;

  eof = frame_times->sof + frame_times->frame_time;
  t.t_end = (frame_times->sof + eof) / 2 -
    (long)(frame_times->exposure_time * USEC_PER_SEC / 2);
  t.t_start = eis->prev_frame_time ? eis->prev_frame_time : t.t_end - 15000;
  eis->prev_frame_time = t.t_end;
  is_output->t_start = t.t_start;
  is_output->t_end = t.t_end;

  gyro_data.gyro_data = gyro_samples_1;
  get_gyro_samples(&t, &gyro_data);
  get_integrated_gyro_data(&gyro_data, &t, angle);
  CDBG("%s: id = %lu, sof = %llu, ftime = %llu, exp_time = %f, t0 = %llu, "
       "t1 = %llu, valt(1) = %llu, valt(end) = %llu, x = %d, y = %d",
    __func__, is_output->frame_id, frame_times->sof, frame_times->frame_time,
    frame_times->exposure_time, t.t_start, t.t_end, gyro_data.gyro_data[0].ts,
    gyro_data.gyro_data[gyro_data.num_elements-1].ts, angle[0], angle[1]);

  err = eis_align_gyro_to_camera(&gyro_position, angle,
    eis->init_data.sensor_mount_angle, eis->init_data.camera_position);

  if (err == 0)
    err = eis_stabilize_frame(eis, &gyro_position, &dis_position,
      &eis_output, &is_output->eis_output_valid);

  if (is_output->eis_output_valid) {
    is_output->eis_output_x = (int32_t) eis_output.x;
    is_output->eis_output_y = (int32_t) eis_output.y;
  } else {
    CDBG_ERROR("%s: Invalid EIS Output\n", __func__);
  }

  return err;
}


/** eis_initialize:
 *    @eis: EIS context
 *    @data: initialization parameters
 *
 * This function initializes the EIS algorithm.
 *
 * Returns 0 on success.
 **/
int eis_initialize(eis_context_type *eis, is_init_data_t *data)
{
  int rc = 0;
  eis_init_type *init_param = &eis->init_data;
  frame_cfg_t *frame_cfg = &data->frame_cfg;
  rs_cs_config_t *rs_cs_config = &data->rs_cs_config;

  init_param->sensor_mount_angle = data->sensor_mount_angle;
  init_param->camera_position = data->camera_position;

  init_param->width = frame_cfg->dis_frame_width;
  init_param->height = frame_cfg->dis_frame_height;

  init_param->margin_x = (frame_cfg->vfe_output_width -
      frame_cfg->dis_frame_width) / 2;
  init_param->margin_y = (frame_cfg->vfe_output_height -
      frame_cfg->dis_frame_height) / 2;

  CDBG_HIGH("%s: Before normalize Input margin x=%d y=%d", __func__,
      init_param->margin_x, init_param->margin_y);

  init_param->margin_x = (uint16_t) roundf((float)(init_param->margin_x *
      rs_cs_config->num_col_sum) / (float)frame_cfg->vfe_output_width);
  init_param->margin_y = (uint16_t) roundf((float)(init_param->margin_y *
      rs_cs_config->num_row_sum) / (float)frame_cfg->vfe_output_height);

  CDBG_HIGH("%s: After normalize Input margin x=%d y=%d", __func__,
      init_param->margin_x, init_param->margin_y);

  CDBG_HIGH("%s: init_param->margin_x = %u", __func__, init_param->margin_x);
  CDBG_HIGH("%s: init_param->margin_y = %u", __func__, init_param->margin_y);

  if (eis_init(init_param, eis) > 0) {
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
int eis_deinitialize(eis_context_type *eis)
{
  int rc;

  rc = eis_exit(eis);
  return rc;
}
