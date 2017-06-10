/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <unistd.h>
#include "vfe.h"
#include "camera_dbg.h"

#ifdef ENABLE_AF_STATS_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_af_stats_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_af_stats_init(int mod_id, void *af_stats, void *vparams)
{
  af_stats_t *af_stats_mod = (af_stats_t *)af_stats;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  af_stats_mod->af_stats.fvMode = vfe_params->chroma3a->fv_metric;
  af_stats_mod->af_stats.fvMin = 10;

  af_stats_mod->af_stats.a00 = vfe_params->chroma3a->af_vfe_hpf.a00;
  af_stats_mod->af_stats.a02 = vfe_params->chroma3a->af_vfe_hpf.a02;
  af_stats_mod->af_stats.a04 = vfe_params->chroma3a->af_vfe_hpf.a04;

  af_stats_mod->af_stats.a20 = vfe_params->chroma3a->af_vfe_hpf.a20;
  af_stats_mod->af_stats.a21 = vfe_params->chroma3a->af_vfe_hpf.a21;
  af_stats_mod->af_stats.a22 = vfe_params->chroma3a->af_vfe_hpf.a22;
  af_stats_mod->af_stats.a23 = vfe_params->chroma3a->af_vfe_hpf.a23;
  af_stats_mod->af_stats.a24 = vfe_params->chroma3a->af_vfe_hpf.a24;

  return VFE_SUCCESS;
} /* vfe_stats_awb_init */


/*===========================================================================
 * FUNCTION    - vfe_af_stats_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_af_stats_config(int mod_id, void *stats_af, void *vparams)
{
  vfe_status_t status = VFE_SUCCESS;
  af_stats_t *mod = (af_stats_t *)stats_af;
  vfe_params_t *params = (vfe_params_t *)vparams;

  if (!mod->enable) {
    CDBG("%s: AF not enabled", __func__);
    return status;
  }

  mod->af_stats.rgnHOffset = params->af_params.rgn_hoffset;
  mod->af_stats.rgnVOffset = params->af_params.rgn_voffset;
  mod->af_stats.shiftBits = params->af_params.shift_bits;
  mod->af_stats.rgnHeight = params->af_params.rgn_height;
  mod->af_stats.rgnWidth = params->af_params.rgn_width;
  mod->af_stats.rgnHNum = params->af_params.rgn_hnum;
  mod->af_stats.rgnVNum = params->af_params.rgn_vnum;

  CDBG("%s:\n",__func__);
  CDBG("rgnHOffset=%d",mod->af_stats.rgnHOffset);
  CDBG("rgnVOffset=%d",mod->af_stats.rgnVOffset);
  CDBG("shiftBits=%d",mod->af_stats.shiftBits);
  CDBG("rgnHeight=%d",mod->af_stats.rgnHeight);
  CDBG("rgnWidth=%d",mod->af_stats.rgnWidth);
  CDBG("rgnHNum=%d",mod->af_stats.rgnHNum);
  CDBG("rgnVNum=%d",mod->af_stats.rgnVNum);

  CDBG("a00=%d",mod->af_stats.a00);
  CDBG("a02=%d",mod->af_stats.a02);
  CDBG("a04=%d",mod->af_stats.a04);
  CDBG("a20=%d",mod->af_stats.a20);
  CDBG("a21=%d",mod->af_stats.a21);
  CDBG("a22=%d",mod->af_stats.a22);
  CDBG("a23=%d",mod->af_stats.a23);
  CDBG("a24=%d",mod->af_stats.a24);
  CDBG("fvMin=%d",mod->af_stats.fvMin);
  CDBG("fvMode=%d",mod->af_stats.fvMode);

  CDBG("%s: Sending VFE command for type =%d", __func__,
    VFE_CMD_STATS_AF_START);

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL,
    &(mod->af_stats),
    sizeof(mod->af_stats), VFE_CMD_STATS_AF_START);
  if (VFE_SUCCESS == status)
    mod->af_update = TRUE;
  return status;
} /* vfe_af_stats_config */

/*===========================================================================
 * FUNCTION    - vfe_af_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_af_stats_enable(int mod_id, void *stats_af, void *vparams,
  int8_t enable, int8_t hw_write)
{
  af_stats_t *mod = (af_stats_t *)stats_af;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  CDBG("%s: %d", __func__, enable);

  mod->enable = enable;
  /* Don't enable AF in moduleCfg until AF_START.
   * Otherwise composite IRQ won't happen */
  params->moduleCfg->statsAfBfEnable = 0;

  if (!mod->enable)
    return VFE_SUCCESS;

  return status;
}/*vfe_af_stats_enable*/

/*===========================================================================
 * FUNCTION    - vfe_af_stats_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_af_stats_update(int mod_id, void *stats_af, void *vparams)
{
  af_stats_t *mod = (af_stats_t *)stats_af;
  vfe_params_t *parms = (vfe_params_t *)vparams;

  if (mod->af_update) {
    parms->update |= VFE_MOD_AF_STATS;
    mod->af_update = FALSE;
  }
  return VFE_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - vfe_af_stats_stop -
 *
 * DESCRIPTION:  Disable the af stats
 *==========================================================================*/
vfe_status_t vfe_af_stats_stop(af_stats_t *stats, vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;

  CDBG("vfe_af_stats_stop\n");
  status = vfe_util_write_hw_cmd(params->camfd,
    CMD_GENERAL, 0, 0, VFE_CMD_STATS_AF_STOP);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}

