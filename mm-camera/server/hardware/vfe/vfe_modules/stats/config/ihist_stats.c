/*============================================================================
   Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_IHIST_STATS_LOGGING
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_ihist_stats_get_shiftbits -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_ihist_stats_get_shiftbits(ihist_stats_t *ihist_stats,
  vfe_params_t *vfe_params, uint32_t *p_shiftbits)
{
  CDBG("%s: %d", __func__, ihist_stats->ihist_stats_cmd.shiftBits);
  *p_shiftbits = ihist_stats->ihist_stats_cmd.shiftBits;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_stats_ihist_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_ihist_stats_config(int mod_id, void *stats_ih, void *vparams)
{
  ihist_stats_t *ihist_stats = (ihist_stats_t *)stats_ih;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  uint32_t window_w_t, window_h_t, total_pixels;
  int32_t shift_bits;

  if (!ihist_stats->enable) {
    CDBG("%s: not enabled", __func__);
    return VFE_SUCCESS;
  }

  ihist_stats->ihist_stats_cmd.channelSelect = 0;
  ihist_stats->ihist_stats_cmd.rgnHNum =
    FLOOR16((vfe_params->output2w)/2)-1;

  ihist_stats->ihist_stats_cmd.rgnVNum =
    FLOOR16((vfe_params->output2h)/2)-1;
  /* calculate shift bits */

  window_w_t = vfe_params->output2w;
  window_h_t = vfe_params->output2h;
  total_pixels = (float)(window_w_t * window_h_t)/8.0;
  shift_bits = CEIL_LOG2(total_pixels);
  shift_bits -= 16;
  shift_bits = MAX(0, shift_bits);
  shift_bits = MIN(4, shift_bits);
  CDBG("%s: tot %d shift %d", __func__, total_pixels, shift_bits);

  ihist_stats->ihist_stats_cmd.shiftBits = shift_bits;
  ihist_stats->ihist_stats_cmd.siteSelect = 0;

  CDBG("IHIST statsconfig shiftBits %d\n",
    ihist_stats->ihist_stats_cmd.shiftBits);
  CDBG("IHIST statsconfig channelSelect  %d\n",
    ihist_stats->ihist_stats_cmd.channelSelect);
  CDBG("IHIST statsconfig siteSelect %d\n",
    ihist_stats->ihist_stats_cmd.siteSelect);
  CDBG("IHIST statsconfig rgnHNum   %d\n",
    ihist_stats->ihist_stats_cmd.rgnHNum);
  CDBG("IHIST statsconfig rgnVNum   %d\n",
    ihist_stats->ihist_stats_cmd.rgnVNum);

  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL, &(ihist_stats->ihist_stats_cmd),
    sizeof(ihist_stats->ihist_stats_cmd), VFE_CMD_STATS_IHIST_START);

  //vfeCtrl->stats_state = VFE_STATS_ENABLING;
  return VFE_SUCCESS;
} /* vfe_stats_ihist_init */

/*===========================================================================
 * FUNCTION    - vfe_ihist_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_ihist_stats_enable(int mod_id, void *stats_ih, void *vparams,
  int8_t enable, int8_t hw_write)
{
  ihist_stats_t *mod = (ihist_stats_t *)stats_ih;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  CDBG("%s: %d", __func__, enable);

  mod->enable = enable;
  params->moduleCfg->statsIhistEnable = enable;

  return status;
}/*vfe_ihist_stats_enable*/
