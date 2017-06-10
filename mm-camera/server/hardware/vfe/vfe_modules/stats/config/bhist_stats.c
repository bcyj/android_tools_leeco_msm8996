/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_BAYER_HIST_LOGGING
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_bhist_stats_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_bhist_stats_debug(bhist_stats_t *bhist_stats)
{
  CDBG("%s:Bayer Histogram Stats Configurations\n", __func__);
  CDBG("%s:rgnHOffset %d\n", __func__, bhist_stats->bhist_stats_cmd.rgnHOffset);
  CDBG("%s:rgnVOffset %d\n", __func__, bhist_stats->bhist_stats_cmd.rgnVOffset);
  CDBG("%s:rgnHNum    %d\n", __func__, bhist_stats->bhist_stats_cmd.rgnHNum);
  CDBG("%s:rgnVNum    %d\n", __func__, bhist_stats->bhist_stats_cmd.rgnVNum);
} /* vfe_bhist_stats_debug */

/*===========================================================================
 * FUNCTION    - vfe_bf_stats_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bhist_stats_init(int mod_id, void *stats, void *vparams)
{
  bhist_stats_t *bhist_stats = (bhist_stats_t *)stats;

  bhist_stats->bhist_stats_cmd.rgnHOffset = 0;
  bhist_stats->bhist_stats_cmd.rgnVOffset = 0;
  bhist_stats->bhist_stats_cmd.rgnHNum    = 0;
  bhist_stats->bhist_stats_cmd.rgnVNum    = 0;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_bhist_stats_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bhist_stats_config(int mod_id, void *stats, void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  bhist_stats_t *mod = (bhist_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;
  uint32_t camif_window_w_t, camif_window_h_t;
  if (!mod->enable) {
    CDBG("%s: Bhist stats not enabled", __func__);
    return status;
  }

  camif_window_w_t = params->sensor_parms.lastPixel -
    params->sensor_parms.firstPixel + 1;
  camif_window_h_t = params->sensor_parms.lastLine -
    params->sensor_parms.firstLine + 1;

  CDBG("%s:\n",__func__);
  CDBG("camif_window_w_t : %u\n", camif_window_w_t);
  CDBG("camif_window_h_t : %u\n", camif_window_h_t);
  mod->bhist_stats_cmd.rgnHOffset = FLOOR2(camif_window_w_t%2);
  mod->bhist_stats_cmd.rgnVOffset = FLOOR2(camif_window_h_t%2);
  mod->bhist_stats_cmd.rgnHNum    = FLOOR2(camif_window_w_t/2) - 1;
  mod->bhist_stats_cmd.rgnVNum    = FLOOR2(camif_window_h_t/2) - 1;
  vfe_bhist_stats_debug(mod);

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    &(mod->bhist_stats_cmd),
    sizeof(mod->bhist_stats_cmd), VFE_CMD_STATS_BHIST_START);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
    return status;
  }
  return status;
} /* vfe_bhist_stats_config */

/*===========================================================================
 * FUNCTION    - vfe_bhist_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_bhist_stats_enable(int mod_id, void *stats, void *vparams,
  int8_t enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  bhist_stats_t* bhist_stats = (bhist_stats_t *)stats;
  vfe_params_t *params = (vfe_params_t *)vparams;

  CDBG("%s: %d", __func__, enable);

  bhist_stats->enable = enable;
  params->moduleCfg->statsSkinBhistEnable = enable;
  return status;
}/*vfe_bhist_stats_enable*/

