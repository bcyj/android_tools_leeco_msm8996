/*==============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
 *============================================================================*/
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tgtcommon.h"
#include "stats_proc_interface.h"
#include "stats_proc.h"
#include "dis_interface.h"
#include "camera_dbg.h"

#if 0
#undef CDBG
#define CDBG LOGE
#undef LOG_TAG
#define LOG_TAG "DISINFO"
#endif

static dis_context_type *disCtrl[MAX_INSTANCES];

/*==============================================================================
 * FUNCTION    - dis_if_init -
 *
 * DESCRIPTION:
 *============================================================================*/
int dis_if_init(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return -1;
  disCtrl[index] = malloc(sizeof(dis_context_type));
  if (!disCtrl[index])
    return -1;
  memset(disCtrl[index], 0, sizeof(dis_context_type));
  return 0;
} /* dis_if_init */

/*==============================================================================
 * FUNCTION    - dis_if_deinit -
 *
 * DESCRIPTION:
 *============================================================================*/
void dis_if_deinit(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index < MAX_INSTANCES) {
    if (disCtrl[index])
      free(disCtrl[index]);
      disCtrl[index] = NULL;
  }
} /* dis_if_deinit */

/*==============================================================================
 * FUNCTION    - dis_normalize_offset -
 *
 * DESCRIPTION:
 *============================================================================*/
void dis_normalize_offset(dis_context_type *dis,
  dis_position_type *offset)
{
  int32_t vfe_out_w, vfe_out_h;
  uint32_t original_x_margin, original_y_margin;

  uint32_t stats_in_w = dis->init_data.input_frame_width;
  uint32_t stats_in_h = dis->init_data.input_frame_height;

  mctl_dis_frame_cfg_type_t *frame_cfg = &dis->input_data.d.init_data.frame_cfg;

  original_x_margin =
    (frame_cfg->vfe_output_width - frame_cfg->dis_frame_width) / 2;
  original_y_margin =
    (frame_cfg->vfe_output_height - frame_cfg->dis_frame_height) / 2;

  vfe_out_w = frame_cfg->vfe_output_width;
  vfe_out_h = frame_cfg->vfe_output_height;

  CDBG("%s: offset in DIS frame domain: x=%d y=%d, Range:(-ve(margin/2) to "
    "+ve(margin/2))", __func__, offset->x, offset->y);
  if (offset->x != 0)
    offset->x = offset->x + dis->init_data.margin_x;
  if (offset->y != 0)
    offset->y = offset->y + dis->init_data.margin_y;
  CDBG("%s: offset in DIS frame domain: x=%d y=%d, Range:(0 to +ve(2*margin))",
    __func__, offset->x, offset->y);

  if (offset->x != 0)
    offset->x = (int32_t) roundf((float)offset->x * (float)vfe_out_w /
      (float)stats_in_w);
  else
    offset->x = offset->x + original_x_margin;

  if (offset->y != 0)
    offset->y = (int32_t) roundf((float)offset->y * (float)vfe_out_h /
      (float)stats_in_h);
  else
    offset->y = offset->y + original_y_margin;

  CDBG("%s: offset in VFE_OP frame domain: x=%d y=%d, Range:(0 to "
    "+ve(2*margin))", __func__, offset->x, offset->y);

  offset->x = MAX(MIN((int32_t)(2 * original_x_margin), offset->x), 0);
  offset->y = MAX(MIN((int32_t)(2 * original_y_margin), offset->y), 0);

  CDBG("%s, after we clamp offset x = %d, y = %d.\n", __func__, offset->x,
    offset->y);
} /* dis_normalize_offset */

static void dis_detect_motion(stats_proc_gyro_info_t *gyro,
  dis_context_type *dis)
{
  float gyro_x_sqr;
  float gyro_y_sqr;

  gyro_x_sqr = gyro->flt[0] * gyro->flt[0];
  gyro_y_sqr = gyro->flt[1] * gyro->flt[1];

  CDBG("%s: Gyro X Sqr: %f, Y Sqr: %f\n", __func__, gyro_x_sqr, gyro_y_sqr);

  dis->gyro_motion_x = 0;
  dis->gyro_motion_y = 0;

  if (gyro_x_sqr >= MOTION_THRESHOLD)
    dis->gyro_motion_x = 1;

  if (gyro_y_sqr >= MOTION_THRESHOLD)
    dis->gyro_motion_y = 1;
}

/*==============================================================================
 * FUNCTION    - dis_process -
 *
 * DESCRIPTION:
 *============================================================================*/
int dis_process(stats_proc_t *sproc)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  dis_context_type *dis = disCtrl[index];
  dis_position_type offset;
  dis_1d_proj_type* curr_ptr;
  vfe_stats_output_t *vfe_out = sproc->input.mctl_info.vfe_stats_out;

  if (dis == NULL)
    return -1;

  if (sproc->input.gyro_info.float_ready)
    dis_detect_motion(&(sproc->input.gyro_info), dis);

  CDBG("%s: E", __func__);
  curr_ptr = dis->vfe_context.curr_frame_1d_proj;

  memcpy(curr_ptr->row_sum, vfe_out->vfe_stats_struct.rs_op.row_sum,
    sizeof(curr_ptr->row_sum));
  memcpy(curr_ptr->col_sum, vfe_out->vfe_stats_struct.cs_op.col_sum,
    sizeof(curr_ptr->col_sum));
  memset(&offset,  0, sizeof(dis_position_type));

  offset.frame_id = vfe_out->vfe_stats_struct.cs_op.frame_id;
  rc = dis_stabilize_frame_with_1d_proj(dis, &offset);
  if(rc == DIS_SUCCESS) {
    /* Store unnormalized DIS offsets */
    sproc->share.dis_ext.prev_dis_output_x = dis->inst_shake.x;
    sproc->share.dis_ext.prev_dis_output_y = dis->inst_shake.y;

    /* Check if we have valid EIS Offsets */
    if (sproc->share.dis_ext.eis_output_valid) {
      /* Use EIS Offsets for this frame */
      offset.x = sproc->share.dis_ext.eis_output_x;
      offset.y = sproc->share.dis_ext.eis_output_y;
      sproc->share.dis_ext.has_output = 1;
    }

    dis_normalize_offset(dis, &offset);
    sproc->share.dis_ext.dis_pos.frame_id = offset.frame_id;
    sproc->share.dis_ext.dis_pos.x = offset.x;
    sproc->share.dis_ext.dis_pos.y = offset.y;
    sproc->share.dis_ext.has_output = 1;
  } else {
    rc = -1;
    sproc->share.dis_ext.has_output = 0;
  }

  CDBG("%s: X, rc = %d", __func__, rc);
  return rc;
} /* dis_process */

/*==============================================================================
 * FUNCTION    - dis_get_params -
 *
 * DESCRIPTION:
 *============================================================================*/
int dis_get_params(stats_proc_t *sproc, stats_proc_get_dis_data_t *get_dis)
{
  uint32_t index = sproc->handle & 0xFF;
  switch (get_dis->type) {
    default:
      CDBG_ERROR("%s: Invalid DIS Get Params Type=%d", __func__, get_dis->type);
      return -1;
  }
  return 0;
} /* dis_get_params */

/*==============================================================================
 * FUNCTION    - dis_set_params -
 *
 * DESCRIPTION:
 *============================================================================*/
int dis_set_params(stats_proc_t *sproc, stats_proc_set_dis_data_t *data)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  dis_context_type *dis = disCtrl[index];
  dis_init_type *init_param = &dis->init_data;

  CDBG("%s: type = %d", __func__, data->type);

  switch(data->type) {
    case DIS_S_INIT_DATA: {
      mctl_dis_frame_cfg_type_t *frame_cfg = &data->d.init_data.frame_cfg;
      vfe_stats_rs_cs_params *rs_cs_parm = &data->d.init_data.rs_cs_parm;

      dis->input_data = *data;

      CDBG("%s: Input Frame Cfg VFE WxH=%dx%d DIS WxH=%dx%d", __func__,
        frame_cfg->vfe_output_width, frame_cfg->vfe_output_height,
        frame_cfg->dis_frame_width, frame_cfg->dis_frame_height);
      CDBG("%s: Input Rs_Cs RS#=%d CS#=%d, RS_VSR=%d, CS_HSR=%d", __func__,
        rs_cs_parm->rs_num_rgns, rs_cs_parm->cs_num_rgns,
        rs_cs_parm->config.row_sum_V_subsample_ratio,
        rs_cs_parm->config.col_sum_H_subsample_ratio);

      init_param->frame_rate = frame_cfg->frame_fps;
      init_param->input_frame_width = rs_cs_parm->cs_num_rgns;
      init_param->input_frame_height = rs_cs_parm->rs_num_rgns;

      init_param->margin_x = (frame_cfg->vfe_output_width -
        frame_cfg->dis_frame_width) / 2;
      init_param->margin_y = (frame_cfg->vfe_output_height -
        frame_cfg->dis_frame_height) / 2;

      CDBG("%s: Before normalize Input margin x=%d y=%d", __func__,
        init_param->margin_x, init_param->margin_y);

      init_param->margin_x = (uint16_t) roundf((float)(init_param->margin_x *
        rs_cs_parm->cs_num_rgns) / (float)frame_cfg->vfe_output_width);
      init_param->margin_y = (uint16_t) roundf((float)(init_param->margin_y *
        rs_cs_parm->rs_num_rgns) / (float)frame_cfg->vfe_output_height);

      CDBG("%s: After normalize Input margin x=%d y=%d", __func__,
        init_param->margin_x, init_param->margin_y);

      // stabilization search window in x, y direction (pixels)
      init_param->search_x = init_param->margin_x;
      init_param->search_y = init_param->margin_y;

      // scale factor to down sample input image horizaontally, vertically
      init_param->num_row_sum = rs_cs_parm->rs_num_rgns;
      init_param->num_col_sum = rs_cs_parm->cs_num_rgns;

      CDBG("%s: Input Search x=%d y=%d", __func__, init_param->search_x,
        init_param->search_y);

      if (dis_init(init_param, dis) > 0) {
        CDBG_HIGH("%s: dis_init failed \n", __func__);
        rc = -1;
      }
      break;
    }
    case DIS_S_DEINIT_DATA:
      if (dis_exit(dis) > 0) {
        CDBG_HIGH("%s: dis_exit failed \n", __func__);
        rc = -1;
      }
      break;
    default:
      CDBG_ERROR("%s: Invalid type = %d", __func__, data->type);
      rc = -1;
      break;
  }
  return rc;
} /* dis_set_params */
