/* dis_interface.c
 *
 * Copyright (c) 2012 - 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dis_interface.h"
#include "is_sns_lib.h"
#include "camera_dbg.h"

#if 0
#undef CDBG
#define CDBG ALOGE
#undef LOG_TAG
#define LOG_TAG "DISINFO"
#endif

#undef CDBG_HIGH
#if 1
#define CDBG_HIGH ALOGE
#else
#define CDBG_HIGH
#endif


#define USEC_PER_SEC    (1000000L)
#define ONE_Q16         (1 << 16)
#define MAX_INSTANCES     8
#ifndef MIN
#define MIN(a,b)     (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)     (((a) > (b)) ? (a) : (b))
#endif


/** dis_normalize_offset:
 *    @dis: DIS context
 *    @offset: x, y IS offsets
 *
 * This function normalizes the offsets from the RS/CS stats window domain to
 * the VFE dimensions domain.  It caps the resulting offsets to ensure that
 * they don't exceed the actual margins.
 **/
void dis_normalize_offset(dis_context_type *dis,
  dis_position_type *offset)
{
  int32_t vfe_out_w, vfe_out_h;
  uint32_t original_x_margin, original_y_margin;

  uint32_t stats_in_w = dis->init_data.input_frame_width;
  uint32_t stats_in_h = dis->init_data.input_frame_height;
  frame_cfg_t *frame_cfg = &dis->input_data.frame_cfg;

  original_x_margin =
    (frame_cfg->vfe_output_width - frame_cfg->dis_frame_width) / 2;
  original_y_margin =
    (frame_cfg->vfe_output_height - frame_cfg->dis_frame_height) / 2;

  vfe_out_w = frame_cfg->vfe_output_width;
  vfe_out_h = frame_cfg->vfe_output_height;

  CDBG("%s: offset in DIS frame domain: x=%d y=%d, Range:(-ve(margin/2) to "
    "+ve(margin/2))", __func__, offset->x, offset->y);

  offset->x = offset->x + dis->init_data.margin_x;
  offset->y = offset->y + dis->init_data.margin_y;

  CDBG("%s: offset in DIS frame domain: x=%d y=%d, Range:(0 to +ve(2*margin))",
    __func__, offset->x, offset->y);

  if (offset->x != 0)
    offset->x = (int32_t) roundf((float)offset->x * (float)vfe_out_w /
      (float)stats_in_w);
  if (offset->y != 0)
    offset->y = (int32_t) roundf((float)offset->y * (float)vfe_out_h /
      (float)stats_in_h);

  CDBG("%s: offset in VFE_OP frame domain: x=%d y=%d, Range:(0 to "
    "+ve(2*margin))", __func__, offset->x, offset->y);

  offset->x = MAX(MIN((int32_t)(2 * original_x_margin), offset->x), 0);
  offset->y = MAX(MIN((int32_t)(2 * original_y_margin), offset->y), 0);

  CDBG("%s, after we clamp offset x = %d, y = %d.\n", __func__, offset->x,
    offset->y);
} /* dis_normalize_offset */


/** dis_detect_motion:
 *    @frame_times: times associated with the current frame (SOF, exposure
 *                  time, etc.)
 *    @dis: DIS context
 *
 * This function is used for gyro-assisted DIS.  It looks at the frame's gyro
 * angles and compares them to a threshold.  If either angle (x or y) exceeds
 * the threshold, there is motion for this frame.
 **/
static void dis_detect_motion(frame_times_t *frame_times,
  dis_context_type *dis)
{
  float gyro_x_sqr;
  float gyro_y_sqr;
  time_interval_s t;
  unsigned long long eof;
  gyro_sample_s gyro_samples_1[64];
  gyro_data_s gyro_data;
  int angle[3];

  gyro_data.gyro_data = gyro_samples_1;
  eof = frame_times->sof + frame_times->frame_time;
  t.t_end = (frame_times->sof + eof) / 2 -
    frame_times->exposure_time * USEC_PER_SEC / 2;
  t.t_start = dis->prev_frame_time ? dis->prev_frame_time : t.t_end - 15000;
  dis->prev_frame_time = t.t_end;

  get_gyro_samples(&t, &gyro_data);
  get_integrated_gyro_data(&gyro_data, &t, angle);
  CDBG("%s: t0 = %llu, t1 = %llu, x = %d, y = %d", __func__, t.t_start,
    t.t_end, angle[0], angle[1]);

  gyro_x_sqr = ((float)angle[0] / ONE_Q16) * ((float)angle[0] / ONE_Q16);
  gyro_y_sqr = ((float)angle[1] / ONE_Q16) * ((float)angle[1] / ONE_Q16);

  CDBG("%s: Gyro X Sqr: %f, Y Sqr: %f\n", __func__, gyro_x_sqr, gyro_y_sqr);

  dis->gyro_motion_x = 0;
  dis->gyro_motion_y = 0;

  if (gyro_x_sqr >= MOTION_THRESHOLD)
    dis->gyro_motion_x = 1;

  if (gyro_y_sqr >= MOTION_THRESHOLD)
    dis->gyro_motion_y = 1;
}

/** dis_process:
 *    @dis: DIS context
 *    @rs_cs_data: RS/CS stats
 *    @frame_times: times associated with the current frame (SOF, exposure
 *                  time, etc.)
 *    @is_output: IS algorithm output
 *
 * This function runs the DIS algorithm.
 *
 * Returns 0 on success.
 **/
int dis_process(dis_context_type *dis, rs_cs_data_t *rs_cs_data,
  frame_times_t *frame_times, is_output_type *is_output)
{
  int rc = 0;
  dis_position_type offset;
  dis_1d_proj_type* curr_ptr;
  //vfe_stats_output_t *vfe_out = sproc->input.mctl_info.vfe_stats_out;

  CDBG("%s: E", __func__);
  if (dis == NULL)
    return -1;

  if (dis->input_data.is_mode == IS_TYPE_GA_DIS &&
    frame_times->frame_time != 0) {
    /* Gyro-assisted DIS and gyro data is available for this frame */
    dis_detect_motion(frame_times, dis);
  }
  else {
    dis->gyro_motion_x = 1;
    dis->gyro_motion_y = 1;
  }

  curr_ptr = dis->vfe_context.curr_frame_1d_proj;

  memcpy(curr_ptr->row_sum, rs_cs_data->rs_stats.row_sum,
    sizeof(curr_ptr->row_sum));
  memcpy(curr_ptr->col_sum, rs_cs_data->cs_stats.col_sum,
    sizeof(curr_ptr->col_sum));
  memset(&offset,  0, sizeof(dis_position_type));

  offset.frame_id = is_output->frame_id;
  rc = dis_stabilize_frame_with_1d_proj(dis, &offset);
  if(rc == DIS_SUCCESS) {
    /* Store unnormalized DIS offsets */
    is_output->prev_dis_output_x = dis->inst_shake.x;
    is_output->prev_dis_output_y = dis->inst_shake.y;

    /* Check for EIS 1 and valid EIS 1 offsets */
    if (dis->input_data.is_mode == IS_TYPE_EIS_1_0 &&
      is_output->eis_output_valid) {
      /* Use EIS 1 offsets for this frame */
      offset.x = is_output->eis_output_x;
      offset.y = is_output->eis_output_y;
      CDBG("%s: dx = %d, dy = %d, ex = %d, ey = %d", __func__,
        dis->inst_shake.x, dis->inst_shake.y, -offset.y, offset.x);
    }
    else {
      CDBG("%s: dx = %d, dy = %d", __func__, dis->inst_shake.x,
        dis->inst_shake.y);
    }

    dis_normalize_offset(dis, &offset);
    if (dis->input_data.is_mode != IS_TYPE_EIS_2_0) {
      is_output->x = offset.x;
      is_output->y = offset.y;
    }
    is_output->has_output = 1;
    CDBG("%s: normalize shake: x = %d, y = %d", __func__, offset.x, offset.y);
  } else {
    rc = -1;
    is_output->has_output = 0;
  }

  CDBG("%s: X, rc = %d", __func__, rc);
  return rc;
} /* dis_process */


/** dis_initialize:
 *    @dis: DIS context
 *    @data: initialization parameters
 *
 * This function initializes the DIS algorithm.
 *
 * Returns 0 on success.
 **/
int dis_initialize(dis_context_type *dis, is_init_data_t *data)
{
  int rc = 0;
  dis_init_type *init_param = &dis->init_data;

  frame_cfg_t *frame_cfg = &data->frame_cfg;
  rs_cs_config_t *rs_cs_config = &data->rs_cs_config;

  dis->input_data = *data;

  CDBG_HIGH("%s: Input Frame Cfg VFE WxH=%dx%d DIS WxH=%dx%d", __func__,
    frame_cfg->vfe_output_width, frame_cfg->vfe_output_height,
    frame_cfg->dis_frame_width, frame_cfg->dis_frame_height);
  CDBG_HIGH("%s: Input Rs_Cs RS#=%ld CS#=%ld", __func__,
    rs_cs_config->num_row_sum, rs_cs_config->num_col_sum);

  init_param->frame_rate = frame_cfg->frame_fps;
  init_param->input_frame_width = rs_cs_config->num_col_sum;
  init_param->input_frame_height = rs_cs_config->num_row_sum;

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

  // stabilization search window in x, y direction (pixels)
  init_param->search_x = init_param->margin_x;
  init_param->search_y = init_param->margin_y;

  // scale factor to down sample input image horizaontally, vertically
  init_param->num_row_sum = rs_cs_config->num_row_sum;
  init_param->num_col_sum = rs_cs_config->num_col_sum;

  CDBG_HIGH("%s: init_param->input_frame_width = %u", __func__,
    init_param->input_frame_width);
  CDBG_HIGH("%s: init_param->input_frame_height = %u", __func__,
    init_param->input_frame_height);
  CDBG_HIGH("%s: init_param->margin_x = %u", __func__, init_param->margin_x);
  CDBG_HIGH("%s: init_param->margin_y = %u", __func__, init_param->margin_y);
  CDBG_HIGH("%s: init_param->search_x = %u", __func__, init_param->search_x);
  CDBG_HIGH("%s: init_param->search_y = %u", __func__, init_param->search_y);
  CDBG_HIGH("%s: init_param->frame_rate = %u", __func__,
    init_param->frame_rate);
  CDBG_HIGH("%s: init_param->num_row_sum = %u", __func__,
    init_param->num_row_sum);
  CDBG_HIGH("%s: init_param->num_col_sum = %u", __func__,
    init_param->num_col_sum);

  if (dis_init(init_param, dis) > 0) {
    CDBG_HIGH("%s: dis_init failed \n", __func__);
    rc = -1;
  }
  return rc;
}

