/*============================================================================

   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <string.h>
#include <stdlib.h>
#include "stats_proc.h"
#include "awb.h"

static awb_t *awbCtrlObj[MAX_INSTANCES];

/*==========================================================================
 * FUNCTION    - awb_process_eztune_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void awb_process_eztune_data(stats_proc_t *sproc, awb_t *awb)
{
  sproc->share.awb_ext.eztune.shifted_d50_rg     = awb->agw.shifted_d50_rg;
  sproc->share.awb_ext.eztune.shifted_d50_bg     = awb->agw.shifted_d50_bg;
  sproc->share.awb_ext.eztune.ymin_pct           = awb->white_y_min_percent;
  sproc->share.awb_ext.eztune.reg_ave_rg_ratio   = awb->regular_ave_rg_ratio;
  sproc->share.awb_ext.eztune.reg_ave_bg_ratio   = awb->regular_ave_bg_ratio;
  sproc->share.awb_ext.eztune.white_ave_rg_ratio = awb->white_ave_rg_ratio;
  sproc->share.awb_ext.eztune.white_ave_bg_ratio = awb->white_ave_bg_ratio;
  sproc->share.awb_ext.eztune.compact_cluster    = awb->agw_d.compact_cluster;
  sproc->share.awb_ext.eztune.outdoor_grn_cnt    = awb->agw_d.green_cnt;
  sproc->share.awb_ext.eztune.indoor_grn_cnt     = awb->agw_d.indoor_green_cnt;
  sproc->share.awb_ext.eztune.current_stat_config =
    awb->current_awb_stat_config;
  sproc->share.awb_ext.eztune.sgw_rg_ratio       = awb->agw_d.simple_rg_ratio;
  sproc->share.awb_ext.eztune.sgw_bg_ratio       = awb->agw_d.simple_bg_ratio;

  if ((!awb->eztune.lock_output) ||
      (sproc->input.mctl_info.opt_state == STATS_PROC_STATE_SNAPSHOT)) {
      /* Save output for EZ Tune*/
    memcpy((void *)&(awb->eztune.stored_gains),
      (const void *)&(sproc->share.awb_ext.curr_gains),
      sizeof(chromatix_manual_white_balance_type));
  } else {
    /* Restore output */
    memcpy((void *)&(sproc->share.awb_ext.curr_gains),
      (const void *)&(awb->eztune.stored_gains),
      sizeof(chromatix_manual_white_balance_type));
  }
} /* awb_process_eztune_data */

/*==========================================================================
 * FUNCTION    - awb_process_mobicat_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void awb_process_mobicat_data(stats_proc_t *sproc, awb_t *awb)
{
  sproc->share.awb_ext.mobicat_awb.colorTemp = sproc->share.awb_ext.color_temp;
  if (sproc->input.mctl_info.opt_mode == STATS_PROC_MODE_2D_ZSL) {
    sproc->share.awb_ext.mobicat_awb.wbGain.rGain =
      sproc->share.awb_ext.curr_gains.r_gain;
    sproc->share.awb_ext.mobicat_awb.wbGain.bGain =
      sproc->share.awb_ext.curr_gains.b_gain;
    sproc->share.awb_ext.mobicat_awb.wbGain.gGain =
      sproc->share.awb_ext.curr_gains.g_gain;
  }
  else {
    sproc->share.awb_ext.mobicat_awb.wbGain.rGain =
      sproc->share.awb_ext.snapshot_wb.r_gain;
    sproc->share.awb_ext.mobicat_awb.wbGain.bGain =
      sproc->share.awb_ext.snapshot_wb.b_gain;
    sproc->share.awb_ext.mobicat_awb.wbGain.gGain =
      sproc->share.awb_ext.snapshot_wb.g_gain;
  }

  CDBG_AWB("%s: colorTemp: %d rGain: %f bGain: %f gGain: %f",
    __func__, sproc->share.awb_ext.mobicat_awb.colorTemp,
    sproc->share.awb_ext.mobicat_awb.wbGain.rGain,
    sproc->share.awb_ext.mobicat_awb.wbGain.bGain,
    sproc->share.awb_ext.mobicat_awb.wbGain.gGain);
} /* awb_process_mobicat_data */

/*===========================================================================
 * FUNCTION    - awb_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
int awb_init(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return -1;
  awbCtrlObj[index] = malloc(sizeof(awb_t));
  if (!awbCtrlObj[index])
    return -1;
  memset(awbCtrlObj[index], 0, sizeof(awb_t));
  awb_t *awb = awbCtrlObj[index];

  awb_settings_init(sproc, awb);
  awb_self_cal_init(sproc, awb);
  return 0;
} /* awb_init */

/* tostart at 2 */
/*===========================================================================
 * FUNCTION    - awb_set -
 *
 * DESCRIPTION:
 *==========================================================================*/
int awb_set_params(stats_proc_t *sproc, stats_proc_set_awb_data_t *data)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  awb_t *awb = awbCtrlObj[index];
  stats_proc_awb_data_t *awb_input = &(sproc->share.awb_ext);
  switch (data->type) {
    case AWB_WHITE_BALANCE:
      rc = awb_set_current_wb(sproc, awb, data->d.awb_current_wb);
      break;
    case AWB_RESTORE_LED_GAINS:
      awb_restore_pre_led_settings(sproc, awb);
      break;
    case AWB_BESTSHOT:
      rc = awb_set_bestshot_mode(sproc, awb, data->d.bestshot_mode);
      break;
    case AWB_EZ_DISABLE:
      awb->eztune.disable = data->d.ez_disable;
      break;
    case AWB_EZ_LOCK_OUTPUT:
      awb->eztune.lock_output = data->d.ez_lock_output;
      break;
    case AWB_LINEAR_GAIN_ADJ:
      awb->linear_gain_adj = data->d.linear_gain_adj;
      break;
    default:
      rc = -1;
      break;
  }
  return rc;
} /* awb_set */

/*===========================================================================
 * FUNCTION    - awb_get -
 *
 * DESCRIPTION:
 *==========================================================================*/
int awb_get_params(stats_proc_t *sproc, stats_proc_get_awb_data_t *data)
{
  int rc = 0;
  stats_proc_awb_data_t *awb_output = &(sproc->share.awb_ext);
  uint32_t index = sproc->handle & 0xFF;
  awb_t *awb = awbCtrlObj[index];

  switch (data->type) {
    case AWB_PARMS:
      data->d.awb_params.bounding_box         = awb_output->bounding_box;
      data->d.awb_params.color_temp           = awb_output->color_temp;
      data->d.awb_params.exterme_col_param.t1 = awb_output->estats.t1;
      data->d.awb_params.exterme_col_param.t2 = awb_output->estats.t2;
      data->d.awb_params.exterme_col_param.t3 = awb_output->estats.t3;
      data->d.awb_params.exterme_col_param.t4 = awb_output->estats.t4;
      data->d.awb_params.exterme_col_param.t5 = awb_output->estats.t5;
      data->d.awb_params.exterme_col_param.t6 = awb_output->estats.t6;
      data->d.awb_params.exterme_col_param.mg = awb_output->estats.mg;
      data->d.awb_params.gain                 = awb_output->curr_gains;
      break;
    case AWB_GAINS:
      data->d.awb_gains.curr_gains            = awb_output->curr_gains;
      break;
    default:
      rc = -1;
      break;
  }
  return rc;
} /* awb_get */

/*===========================================================================
 * FUNCTION    - awb_process -
 *
 * DESCRIPTION:
 *==========================================================================*/
int awb_process(stats_proc_t *sproc)
{
  int rc = -1;
  uint32_t index = sproc->handle & 0xFF;
  awb_t *awb = awbCtrlObj[index];

  if (awb->eztune.disable && sproc->share.eztune_enable)
    return 0;

  switch (sproc->input.mctl_info.opt_state) {
    case STATS_PROC_STATE_PREVIEW:
    case STATS_PROC_STATE_CAMCORDER:
      rc = awb_advanced_grey_world_algo_execute(sproc, awb);
      break;
    case STATS_PROC_STATE_SNAPSHOT:
      awb_algo_snapshot(sproc, awb);
      rc = 0;
      break;
    default:
      break;
  }
  if (sproc->share.eztune_enable)
    awb_process_eztune_data(sproc, awb);
  if (sproc->share.mobicat_enable)
    awb_process_mobicat_data(sproc, awb);
  return rc;
} /* awb_execute */

/*===========================================================================
 * FUNCTION    - awb_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_deinit (stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  awb_t *awb = awbCtrlObj[index];
  if (awb){
    free(awb);
    awb = NULL;
  }
} /* awb_deinit */

/*===========================================================================
 * FUNCTION    - awb_chromatix_reload -
 *
 * DESCRIPTION:
 *==========================================================================*/
void awb_chromatix_reload(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  awb_load_chromatix(sproc, awbCtrlObj[index]);
} /* awb_chromatix_reload */

