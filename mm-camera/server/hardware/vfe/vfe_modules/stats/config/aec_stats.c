/*============================================================================
   Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_AEC_STATS_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_aec_stats_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_aec_stats_debug(aec_stats_t *aec_stats)
{
  CDBG("AEC statsconfig rgnHOffset %d\n",
    aec_stats->aec_stats_cmd.rgnHOffset);
  CDBG("AEC statsconfig rgnVOffset %d\n",
    aec_stats->aec_stats_cmd.rgnVOffset);
  CDBG("AEC statsconfig shiftBits  %d\n",
    aec_stats->aec_stats_cmd.shiftBits);
  CDBG("AEC statsconfig rgnWidth   %d\n",
    aec_stats->aec_stats_cmd.rgnWidth);
  CDBG("AEC statsconfig rgnHeight  %d\n",
    aec_stats->aec_stats_cmd.rgnHeight);
  CDBG("AEC statsconfig rgnHNum    %d\n",
    aec_stats->aec_stats_cmd.rgnHNum);
  CDBG("AEC statsconfig rgnVNum    %d\n",
    aec_stats->aec_stats_cmd.rgnVNum);
  return VFE_SUCCESS;;
} /* vfe_aec_stats_debug */


/*===========================================================================
 * FUNCTION    - vfe_aec_stats_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_aec_stats_init(int mod_id, void *stats_aec,
  void *vparams)
{
  aec_stats_t *aec_stats = (aec_stats_t *)stats_aec;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  aec_stats->aec_stats_cmd.rgnHOffset = 0;
  aec_stats->aec_stats_cmd.rgnVOffset = 0;
  aec_stats->aec_stats_cmd.shiftBits = 5;
  aec_stats->aec_stats_cmd.rgnWidth = 79;
  aec_stats->aec_stats_cmd.rgnHeight = 59;
  aec_stats->aec_stats_cmd.rgnHNum = 15;
  aec_stats->aec_stats_cmd.rgnVNum = 15;

  return VFE_SUCCESS;;
} /* vfe_aec_stats_init */


/*===========================================================================
 * FUNCTION    - vfe_aec_stats_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_aec_stats_config(int mod_id, void *stats_aec,
  void *vparams)
{
  aec_stats_t *mod = (aec_stats_t *)stats_aec;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;
  uint32_t camif_window_w_t, camif_window_h_t;
  if (!mod->enable) {
    CDBG("%s: AF not enabled", __func__);
    return status;
  }

  camif_window_w_t = params->demosaic_op_params.last_pixel - params->demosaic_op_params.first_pixel + 1;
  camif_window_h_t = params->demosaic_op_params.last_line - params->demosaic_op_params.first_line + 1;

  mod->aec_stats_cmd.rgnWidth =
    ((camif_window_w_t / (mod->aec_stats_cmd.rgnHNum + 1)) - 1);

  mod->aec_stats_cmd.rgnHeight =
    ((camif_window_h_t / (mod->aec_stats_cmd.rgnVNum + 1)) -1);

  mod->aec_stats_cmd.shiftBits = vfe_util_calculate_shift_bits(
    (mod->aec_stats_cmd.rgnWidth + 1) *
    (mod->aec_stats_cmd.rgnHeight + 1));

  vfe_aec_stats_debug(mod);
  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    &(mod->aec_stats_cmd),
    sizeof(mod->aec_stats_cmd), VFE_CMD_STATS_AE_START);

  return status;
} /* vfe_aec_stats_config */

/*===========================================================================
 * FUNCTION    - vfe_aec_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_aec_stats_enable(int mod_id, void *stats_aec,
  void *vparams, int8_t enable, int8_t hw_write)
{
  aec_stats_t *mod = (aec_stats_t *)stats_aec;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;

  CDBG("%s: %d", __func__, enable);

  mod->enable = enable;
  params->moduleCfg->statsAeBgEnable = enable;

  if (!mod->enable)
    return VFE_SUCCESS;
  return status;
}/*vfe_aec_stats_enable*/
