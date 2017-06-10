/*==============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
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
#include "eis_interface.h"
#include "camera_dbg.h"

static eis_context_type *eisCtrl[MAX_INSTANCES];

/*==============================================================================
 * FUNCTION    - eis_if_init -
 *
 * DESCRIPTION:
 *============================================================================*/
int eis_if_init(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return -1;
  eisCtrl[index] = malloc(sizeof(eis_context_type));
  if (!eisCtrl[index])
    return -1;
  memset(eisCtrl[index], 0, sizeof(eis_context_type));
  return 0;
} /* eis_if_init */

/*==============================================================================
 * FUNCTION    - eis_if_deinit -
 *
 * DESCRIPTION:
 *============================================================================*/
void eis_if_deinit(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index < MAX_INSTANCES) {
    if (eisCtrl[index])
      free(eisCtrl[index]);
      eisCtrl[index] = NULL;
  }
} /* eis_if_deinit */

/*==============================================================================
 * FUNCTION    - eis_process -
 *
 * DESCRIPTION:
 *============================================================================*/
int eis_process(stats_proc_t *sproc)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  eis_context_type *eis = eisCtrl[index];
  eis_position_type dis_position;
  eis_position_type gyro_data;
  eis_position_type eis_output;

  if (eis == NULL)
    return -1;

  sproc->share.dis_ext.eis_output_valid = 0;

  gyro_data.x = sproc->input.gyro_info.q16[1];
  gyro_data.y = sproc->input.gyro_info.q16[0];


  dis_position.x = (long)sproc->share.dis_ext.prev_dis_output_x;
  dis_position.y = (long)sproc->share.dis_ext.prev_dis_output_y;


  rc = eis_stabilize_frame(eis, &gyro_data, &dis_position,
      &eis_output, &sproc->share.dis_ext.eis_output_valid);
  if (rc)
    return rc;

  if (sproc->share.dis_ext.eis_output_valid) {
    sproc->share.dis_ext.eis_output_x = (int32_t) eis_output.x;
    sproc->share.dis_ext.eis_output_y = (int32_t) eis_output.y;
    rc = 0;

  } else {
    CDBG_ERROR("%s: Invalid EIS Output\n", __func__);
    rc = -1;
  }

  return rc;
}

/*==============================================================================
 * FUNCTION    - eis_set_params -
 *
 * DESCRIPTION:
 *============================================================================*/
int eis_set_params(stats_proc_t *sproc, stats_proc_set_dis_data_t *data)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  eis_context_type *eis = eisCtrl[index];
  eis_init_type *init_param = &eis->init_data;

  CDBG("%s: type = %d", __func__, data->type);

  switch(data->type) {
    case DIS_S_INIT_DATA: {
      mctl_dis_frame_cfg_type_t *frame_cfg = &data->d.init_data.frame_cfg;
      vfe_stats_rs_cs_params *rs_cs_parm = &data->d.init_data.rs_cs_parm;

      init_param->width = frame_cfg->dis_frame_width;
      init_param->height = frame_cfg->dis_frame_height;

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

      if (eis_init(init_param, eis) > 0) {
        CDBG_HIGH("%s: eis_init failed \n", __func__);
        rc = -1;
      }
      break;
    }
    case DIS_S_DEINIT_DATA:
      if (eis_exit(eis) > 0) {
        CDBG_HIGH("%s: eis_exit failed \n", __func__);
        rc = -1;
      }
      break;
    default:
      CDBG_ERROR("%s: Invalid type = %d", __func__, data->type);
      rc = -1;
      break;
  }
  return rc;
} /* eis_set_params */
