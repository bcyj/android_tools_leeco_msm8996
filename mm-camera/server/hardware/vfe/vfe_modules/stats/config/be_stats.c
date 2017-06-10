/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_BAYER_EXPOSURE_LOGGING
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_be_stats_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_be_stats_debug(be_stats_t *be_stats)
{
  CDBG("%s:Bayer Exposure Stats Configurations\n", __func__);
  CDBG("%s:rgnHOffset %d\n", __func__, be_stats->be_stats_cmd.rgnHOffset);
  CDBG("%s:rgnVOffset %d\n", __func__, be_stats->be_stats_cmd.rgnVOffset);
  CDBG("%s:rgnWidth   %d\n", __func__, be_stats->be_stats_cmd.rgnWidth);
  CDBG("%s:rgnHeight  %d\n", __func__, be_stats->be_stats_cmd.rgnHeight);
  CDBG("%s:rgnHNum    %d\n", __func__, be_stats->be_stats_cmd.rgnHNum);
  CDBG("%s:rgnVNum    %d\n", __func__, be_stats->be_stats_cmd.rgnVNum);
  CDBG("%s:r_max      %d\n", __func__, be_stats->be_stats_cmd.rMax);
  CDBG("%s:gr_max     %d\n", __func__, be_stats->be_stats_cmd.grMax);
  CDBG("%s:b_max      %d\n", __func__, be_stats->be_stats_cmd.bMax);
  CDBG("%s:gb_max     %d\n", __func__, be_stats->be_stats_cmd.gbMax);
} /* vfe_be_stats_debug */

/*===========================================================================
 * FUNCTION    - vfe_be_stats_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_be_stats_init(int mod_id, void *stats, void *vparams)
{
  be_stats_t* be_stats = (be_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;

  be_stats->be_stats_cmd.rgnHOffset = 0;
  be_stats->be_stats_cmd.rgnVOffset = 0;
  be_stats->be_stats_cmd.rgnWidth   = 0;
  be_stats->be_stats_cmd.rgnHeight  = 0;
  be_stats->be_stats_cmd.rgnHNum    = 31;
  be_stats->be_stats_cmd.rgnVNum    = 23;
  be_stats->be_stats_cmd.rMax      = 254 - 16;
  be_stats->be_stats_cmd.grMax     = 254 - 16;
  be_stats->be_stats_cmd.bMax      = 254 - 16;
  be_stats->be_stats_cmd.gbMax     = 254 - 16;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_be_stats_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_be_stats_config(int mod_id, void *stats, void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  be_stats_t* mod = (be_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;

  uint32_t camif_window_w_t, camif_window_h_t;
  if (!mod->enable) {
    CDBG("%s: BE not enabled", __func__);
    return status;
  }

  camif_window_w_t = params->sensor_parms.lastPixel -
    params->sensor_parms.firstPixel + 1;
  camif_window_h_t = params->sensor_parms.lastLine -
    params->sensor_parms.firstLine + 1;

  CDBG("%s:\n",__func__);

  mod->be_stats_cmd.rgnHOffset = FLOOR2(camif_window_w_t%32);
  mod->be_stats_cmd.rgnVOffset = FLOOR2(camif_window_h_t%24);
  mod->be_stats_cmd.rgnWidth   = FLOOR2(camif_window_w_t/32) - 1;
  mod->be_stats_cmd.rgnHeight  = FLOOR2(camif_window_h_t/24) - 1;
  mod->be_stats_cmd.rgnHNum    = 31;
  mod->be_stats_cmd.rgnVNum    = 23;
  mod->be_stats_cmd.rMax       = 255 - 16;
  mod->be_stats_cmd.grMax      = 255 - 16;
  mod->be_stats_cmd.bMax       = 255 - 16;
  mod->be_stats_cmd.gbMax      = 255 - 16;

  vfe_be_stats_debug(mod);
    /* TODO: VFE_CMD_STATS_BE_START to be defined in kernel
       defined to 0 for compilation*/
  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    &(mod->be_stats_cmd),
    sizeof(mod->be_stats_cmd), VFE_CMD_STATS_BE_START);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
    return status;
  }
  return status;
} /* vfe_be_stats_config */

/*===========================================================================
 * FUNCTION    - vfe_be_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_be_stats_enable(int mod_id, void *stats, void *vparams,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  be_stats_t* be_stats = (be_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;

  CDBG("%s: %d", __func__, enable);

  be_stats->enable = enable;
  params->moduleCfg->statsBeEnable = enable;
  return status;
}/*vfe_be_stats_enable*/
