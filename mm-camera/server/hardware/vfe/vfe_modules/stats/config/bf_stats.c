/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_BAYER_FOCUS_LOGGING
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_bf_stats_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_bf_stats_debug(bf_stats_t *bf_stats)
{
  CDBG("%s:Bayer Focus Stats Configurations\n", __func__);
  CDBG("%s:rgnHOffset %d\n", __func__, bf_stats->bf_stats_cmd.rgnHOffset);
  CDBG("%s:rgnVOffset %d\n", __func__, bf_stats->bf_stats_cmd.rgnVOffset);
  CDBG("%s:rgnWidth   %d\n", __func__, bf_stats->bf_stats_cmd.rgnWidth);
  CDBG("%s:rgnHeight  %d\n", __func__, bf_stats->bf_stats_cmd.rgnHeight);
  CDBG("%s:rgnHNum    %d\n", __func__, bf_stats->bf_stats_cmd.rgnHNum);
  CDBG("%s:rgnVNum    %d\n", __func__, bf_stats->bf_stats_cmd.rgnVNum);
  CDBG("%s:r_fv_min   %d\n", __func__, bf_stats->bf_stats_cmd.r_fv_min);
  CDBG("%s:gr_fv_min  %d\n", __func__, bf_stats->bf_stats_cmd.gr_fv_min);
  CDBG("%s:b_fv_min   %d\n", __func__, bf_stats->bf_stats_cmd.b_fv_min);
  CDBG("%s:gb_fv_min  %d\n", __func__, bf_stats->bf_stats_cmd.gb_fv_min);
  CDBG("%s:a00        %d\n", __func__, bf_stats->bf_stats_cmd.a00);
  CDBG("%s:a01        %d\n", __func__, bf_stats->bf_stats_cmd.a01);
  CDBG("%s:a02        %d\n", __func__, bf_stats->bf_stats_cmd.a02);
  CDBG("%s:a03        %d\n", __func__, bf_stats->bf_stats_cmd.a03);
  CDBG("%s:a04        %d\n", __func__, bf_stats->bf_stats_cmd.a04);
  CDBG("%s:a10        %d\n", __func__, bf_stats->bf_stats_cmd.a10);
  CDBG("%s:a11        %d\n", __func__, bf_stats->bf_stats_cmd.a11);
  CDBG("%s:a12        %d\n", __func__, bf_stats->bf_stats_cmd.a12);
  CDBG("%s:a13        %d\n", __func__, bf_stats->bf_stats_cmd.a13);
  CDBG("%s:a14        %d\n", __func__, bf_stats->bf_stats_cmd.a14);
} /* vfe_bf_stats_debug */

/*===========================================================================
 * FUNCTION    - vfe_bf_stats_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bf_stats_init(int mod_id, void *stats, void *vparams)
{
  bf_stats_t *bf_stats = (bf_stats_t *)stats;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  bf_stats->bf_stats_cmd.rgnHOffset = 0;
  bf_stats->bf_stats_cmd.rgnVOffset = 0;
  bf_stats->bf_stats_cmd.rgnWidth   = 0;
  bf_stats->bf_stats_cmd.rgnHeight  = 0;
  bf_stats->bf_stats_cmd.rgnHNum    = 17;
  bf_stats->bf_stats_cmd.rgnVNum    = 13;
  bf_stats->bf_stats_cmd.r_fv_min   = 0;
  bf_stats->bf_stats_cmd.gr_fv_min  = 0;
  bf_stats->bf_stats_cmd.b_fv_min   = 0;
  bf_stats->bf_stats_cmd.gb_fv_min  = 0;
  bf_stats->bf_stats_cmd.a00        = 0;
  bf_stats->bf_stats_cmd.a01        = 0;
  bf_stats->bf_stats_cmd.a02        = 0;
  bf_stats->bf_stats_cmd.a03        = 0;
  bf_stats->bf_stats_cmd.a04        = 0;
  bf_stats->bf_stats_cmd.a10        = 0;
  bf_stats->bf_stats_cmd.a11        = 0;
  bf_stats->bf_stats_cmd.a12        = 0;
  bf_stats->bf_stats_cmd.a13        = 0;
  bf_stats->bf_stats_cmd.a14        = 0;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_bf_stats_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bf_stats_config(int mod_id, void *stats, void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  bf_stats_t *mod = (bf_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;
  uint32_t camif_window_w_t, camif_window_h_t;
  if (!mod->enable) {
    CDBG("%s: BF not enabled", __func__);
    return status;
  }

  camif_window_w_t = params->sensor_parms.lastPixel -
    params->sensor_parms.firstPixel + 1;
  camif_window_h_t = params->sensor_parms.lastLine -
    params->sensor_parms.firstLine + 1;

  CDBG("%s:\n",__func__);
  CDBG("camif_window_w_t :%u\n", camif_window_w_t);
  CDBG("camif_window_h_t :%u\n", camif_window_h_t);

  mod->bf_stats_cmd.rgnHOffset = 8;//FLOOR2(camif_window_w_t%18);
  mod->bf_stats_cmd.rgnVOffset = 2;//FLOOR2(camif_window_h_t%14);
  mod->bf_stats_cmd.rgnWidth   = FLOOR2((camif_window_w_t - 8 )/18) - 1;
  mod->bf_stats_cmd.rgnHeight  = FLOOR2((camif_window_h_t - 2 )/14) - 1;
  mod->bf_stats_cmd.rgnHNum    = 17;
  mod->bf_stats_cmd.rgnVNum    = 13;
  mod->bf_stats_cmd.r_fv_min   = 10;
  mod->bf_stats_cmd.gr_fv_min  = 10;
  mod->bf_stats_cmd.b_fv_min   = 10;
  mod->bf_stats_cmd.gb_fv_min  = 10;
  mod->bf_stats_cmd.a00        = 0;
  mod->bf_stats_cmd.a01        = 0;
  mod->bf_stats_cmd.a02        = 5;
  mod->bf_stats_cmd.a03        = 0;
  mod->bf_stats_cmd.a04        = 0;
  mod->bf_stats_cmd.a10        = -2;
  mod->bf_stats_cmd.a11        = -2;
  mod->bf_stats_cmd.a12        = 3;
  mod->bf_stats_cmd.a13        = -2;
  mod->bf_stats_cmd.a14        = -2;

  vfe_bf_stats_debug(mod);
  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    &(mod->bf_stats_cmd),
    sizeof(mod->bf_stats_cmd), VFE_CMD_STATS_BF_START);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
    return status;
  }
  return status;
} /* vfe_bf_stats_config */

/*===========================================================================
 * FUNCTION    - vfe_bf_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bf_stats_enable(int mod_id, void *stats, void *vparams,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_params_t *params = (vfe_params_t *)vparams;
  bf_stats_t* bf_stats = (bf_stats_t *)stats;
  CDBG("%s: %d", __func__, enable);

  bf_stats->enable = enable;
  //enabled in kernel during BF_START
  params->moduleCfg->statsAfBfEnable = 0;
  return status;
}/*vfe_bf_stats_enable*/

/*===========================================================================
 * FUNCTION    - vfe_bf_stats_stop -
 *
 * DESCRIPTION:  Disable the af stats
 *==========================================================================*/
vfe_status_t vfe_bf_stats_stop(bf_stats_t* bf_stats, vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;

  CDBG("%s: E\n", __func__);
  status = vfe_util_write_hw_cmd(params->camfd,
    CMD_GENERAL, 0, 0, VFE_CMD_STATS_BF_STOP);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}
