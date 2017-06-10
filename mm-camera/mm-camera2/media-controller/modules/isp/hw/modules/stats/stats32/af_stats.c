/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include <unistd.h>
#include "af_stats.h"
#include "isp_log.h"

#if 0 /*TODO */
#ifdef ENABLE_AF_STATS_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
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
    ISP_DBG(ISP_MOD_STATS, "%s: AF not enabled", __func__);
    return status;
  }

  mod->af_stats.rgnHOffset = params->af_params.rgn_hoffset;
  mod->af_stats.rgnVOffset = params->af_params.rgn_voffset;
  mod->af_stats.shiftBits = params->af_params.shift_bits;
  mod->af_stats.rgnHeight = params->af_params.rgn_height;
  mod->af_stats.rgnWidth = params->af_params.rgn_width;
  mod->af_stats.rgnHNum = params->af_params.rgn_hnum;
  mod->af_stats.rgnVNum = params->af_params.rgn_vnum;

  ISP_DBG(ISP_MOD_STATS, "%s:\n",__func__);
  ISP_DBG(ISP_MOD_STATS, "rgnHOffset=%d",mod->af_stats.rgnHOffset);
  ISP_DBG(ISP_MOD_STATS, "rgnVOffset=%d",mod->af_stats.rgnVOffset);
  ISP_DBG(ISP_MOD_STATS, "shiftBits=%d",mod->af_stats.shiftBits);
  ISP_DBG(ISP_MOD_STATS, "rgnHeight=%d",mod->af_stats.rgnHeight);
  ISP_DBG(ISP_MOD_STATS, "rgnWidth=%d",mod->af_stats.rgnWidth);
  ISP_DBG(ISP_MOD_STATS, "rgnHNum=%d",mod->af_stats.rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "rgnVNum=%d",mod->af_stats.rgnVNum);

  ISP_DBG(ISP_MOD_STATS, "a00=%d",mod->af_stats.a00);
  ISP_DBG(ISP_MOD_STATS, "a02=%d",mod->af_stats.a02);
  ISP_DBG(ISP_MOD_STATS, "a04=%d",mod->af_stats.a04);
  ISP_DBG(ISP_MOD_STATS, "a20=%d",mod->af_stats.a20);
  ISP_DBG(ISP_MOD_STATS, "a21=%d",mod->af_stats.a21);
  ISP_DBG(ISP_MOD_STATS, "a22=%d",mod->af_stats.a22);
  ISP_DBG(ISP_MOD_STATS, "a23=%d",mod->af_stats.a23);
  ISP_DBG(ISP_MOD_STATS, "a24=%d",mod->af_stats.a24);
  ISP_DBG(ISP_MOD_STATS, "fvMin=%d",mod->af_stats.fvMin);
  ISP_DBG(ISP_MOD_STATS, "fvMode=%d",mod->af_stats.fvMode);

  ISP_DBG(ISP_MOD_STATS, "%s: Sending VFE command for type =%d", __func__,
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
  ISP_DBG(ISP_MOD_STATS, "%s: %d", __func__, enable);

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

  ISP_DBG(ISP_MOD_STATS, "vfe_af_stats_stop\n");
  status = vfe_util_write_hw_cmd(params->camfd,
    CMD_GENERAL, 0, 0, VFE_CMD_STATS_AF_STOP);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}
#endif

