/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_BAYER_GRID_LOGGING
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_bg_stats_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_bg_stats_debug(bg_stats_t *bg_stats)
{
  CDBG("%s:Bayer Grid Stats Configurations\n", __func__);
  CDBG("%s:rgnHOffset %d\n", __func__, bg_stats->bg_stats_cmd.rgnHOffset);
  CDBG("%s:rgnVOffset %d\n", __func__, bg_stats->bg_stats_cmd.rgnVOffset);
  CDBG("%s:rgnWidth   %d\n", __func__, bg_stats->bg_stats_cmd.rgnWidth);
  CDBG("%s:rgnHeight  %d\n", __func__, bg_stats->bg_stats_cmd.rgnHeight);
  CDBG("%s:rgnHNum    %d\n", __func__, bg_stats->bg_stats_cmd.rgnHNum);
  CDBG("%s:rgnVNum    %d\n", __func__, bg_stats->bg_stats_cmd.rgnVNum);
  CDBG("%s:gbMax      %d\n", __func__, bg_stats->bg_stats_cmd.gbMax);
  CDBG("%s:grMax      %d\n", __func__, bg_stats->bg_stats_cmd.grMax);
  CDBG("%s:rMax       %d\n", __func__, bg_stats->bg_stats_cmd.rMax);
  CDBG("%s:bMax       %d\n", __func__, bg_stats->bg_stats_cmd.bMax);
} /* vfe_bg_stats_debug */

/*===========================================================================
 * FUNCTION    - vfe_bg_stats_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bg_stats_init(int mod_id, void *stats, void *vparams)
{
  bg_stats_t* bg_stats = (bg_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;

  bg_stats->bg_stats_cmd.rgnHOffset = 0;
  bg_stats->bg_stats_cmd.rgnVOffset = 0;
  bg_stats->bg_stats_cmd.rgnWidth   = 0;
  bg_stats->bg_stats_cmd.rgnHeight  = 0;
  bg_stats->bg_stats_cmd.rgnHNum    = 71;
  bg_stats->bg_stats_cmd.rgnVNum    = 53;
  bg_stats->bg_stats_cmd.rMax       = 255 - 16;
  bg_stats->bg_stats_cmd.grMax      = 255 - 16;
  bg_stats->bg_stats_cmd.bMax       = 255 - 16;
  bg_stats->bg_stats_cmd.gbMax      = 255 - 16;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_bg_stats_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bg_stats_config(int mod_id, void *stats, void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  bg_stats_t* mod = (bg_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;
  uint32_t camif_window_w_t, camif_window_h_t;
  if (!mod->enable) {
    CDBG("%s: BG not enabled", __func__);
    return status;
  }

  camif_window_w_t = params->sensor_parms.lastPixel -
    params->sensor_parms.firstPixel + 1;
  camif_window_h_t = params->sensor_parms.lastLine -
    params->sensor_parms.firstLine + 1;

  mod->bg_stats_cmd.rgnHOffset = FLOOR2(camif_window_w_t%72);
  mod->bg_stats_cmd.rgnVOffset = FLOOR2(camif_window_h_t%54);
  mod->bg_stats_cmd.rgnWidth   = FLOOR2(camif_window_w_t/72) - 1;
  mod->bg_stats_cmd.rgnHeight  = FLOOR2(camif_window_h_t/54) - 1;
  mod->bg_stats_cmd.rgnHNum    = 71;
  mod->bg_stats_cmd.rgnVNum    = 53;
  mod->bg_stats_cmd.rMax       = 255 - 16;
  mod->bg_stats_cmd.grMax      = 255 - 16;
  mod->bg_stats_cmd.bMax       = 255 - 16;
  mod->bg_stats_cmd.gbMax      = 255 - 16;
  vfe_bg_stats_debug(mod);

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    &(mod->bg_stats_cmd),
    sizeof(mod->bg_stats_cmd), VFE_CMD_STATS_BG_START);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
    return status;
  }

  return status;
} /* vfe_bg_stats_config */

/*===========================================================================
 * FUNCTION    - vfe_bg_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bg_stats_enable(int mod_id, void *stats, void *vparams,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  bg_stats_t* bg_stats = (bg_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;
  CDBG("%s: %d", __func__, enable);

  bg_stats->enable = enable;
  params->moduleCfg->statsAeBgEnable = enable;
  return status;
}/*vfe_bg_stats_enable*/
