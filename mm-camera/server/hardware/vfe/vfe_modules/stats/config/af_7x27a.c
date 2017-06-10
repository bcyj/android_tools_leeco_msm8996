/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_AF_STATS_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_af_stats_init -
 *
 * DESCRIPTION:  Initialize the config command with defaults
 *==========================================================================*/
 vfe_status_t vfe_af_stats_init(af_stats_t *stats, vfe_params_t *params)
 {
    CDBG("vfe_af_stats_init\n");
   stats->cmd.af_enable = VFE_DISABLE_AUTOFOCUS_STATISTICS;
   stats->cmd.af_busprioritysel =
     VFE_HARDWARE_BUFFER_LEVEL_STATISTICS_BUS_PRIORITY;
   stats->cmd.af_buspriorityval = 7;
   stats->cmd.af_winmode = VFE_SINGLE_AUTOFOCUS_WINDOW_MODE;
   stats->cmd.af_singglewinvh =
     (params->active_crop_info.lastLine -
      params->active_crop_info.firstLine) >> 1;
   stats->cmd.af_singlewinhw =
     (params->active_crop_info.lastPixel -
      params->active_crop_info.firstPixel) >> 1;
   stats->cmd.af_singlewinvoffset = stats->cmd.af_singglewinvh >> 1;
   stats->cmd.af_singlewinhoffset = stats->cmd.af_singlewinhw >> 1;
   stats->cmd.af_multiwingrid[0] = 0;
   stats->cmd.af_multiwingrid[1] = 25;
   stats->cmd.af_multiwingrid[2] = 34;
   stats->cmd.af_multiwingrid[3] = 59;
   stats->cmd.af_multiwingrid[4] = 68;
   stats->cmd.af_multiwingrid[5] = 93;
   stats->cmd.af_multiwingrid[6] = 102;
   stats->cmd.af_multiwingrid[7] = 127;
   stats->cmd.af_multiwingrid[8] = 136;
   stats->cmd.af_multiwingrid[9] = 145;
   stats->cmd.af_multiwingrid[10] = 170;
   stats->cmd.af_multiwingrid[11] = 179;
   stats->cmd.af_multiwingrid[12] = 204;
   stats->cmd.af_multiwingrid[13] = 213;
   stats->cmd.af_multiwingrid[14] = 238;
   stats->cmd.af_multiwingrid[14] = 247;
   stats->cmd.af_metrichpfcoefa00 = -2;
   stats->cmd.af_metrichpfcoefa04 = -2;
   stats->cmd.af_metricmaxval = 0x7FF;
   stats->cmd.af_metricsel =
     VFE_CALCULATE_SUM_OF_AUTOFOCUS_METRICS_IN_ROW;
   stats->cmd.af_metrichpfcoefa20 = -1;
   stats->cmd.af_metrichpfcoefa21 = -1;
   stats->cmd.af_metrichpfcoefa22 = 8;
   stats->cmd.af_metrichpfcoefa23 = -1;
   stats->cmd.af_metrichpfcoefa24 = -1;
   stats->cmd.af_metrichp = (unsigned int)0xFF;
   stats->cmd.af_outbuf[0] = NULL;
   stats->cmd.af_outbuf[1] = NULL;
   stats->cmd.af_outbuf[2] = NULL;

   return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_af_stats_config -
 *
 * DESCRIPTION:  Initialize the config command with defaults
 *==========================================================================*/
vfe_status_t vfe_af_stats_config(void *ctrl, vfe_stats_af_params_t *mctl_af)
{
  vfe_status_t status = VFE_SUCCESS;
  uint8_t *autofocusMultiWindowGrid = NULL;
  vfe_ctrl_info_t *p_obj = (vfe_ctrl_info_t *)ctrl;
  af_stats_t *stats = &(p_obj->vfe_module.stats.af_stats);
  vfe_af_params_t *af_params = &(p_obj->vfe_params.af_params);

  stats->cmd.af_singlewinhoffset = af_params->rgn_hoffset;
  stats->cmd.af_singlewinvoffset = af_params->rgn_voffset;
  stats->cmd.af_singglewinvh = af_params->rgn_height;
  stats->cmd.af_singlewinhw = af_params->rgn_width;

  CDBG("vfe_af_stats_config\n");
  if (af_params->multi_roi_win) {
    stats->cmd.af_winmode = VFE_MULTIPLE_AUTOFOCUS_WINDOWS_MODE;
    memcpy(&(stats->cmd.af_multiwingrid),af_params->multi_roi_win,
      NUM_AF_MULTI_WINDOW_GRIDS);
  }
  stats->cmd.af_enable = VFE_ENABLE_AUTOFOCUS_STATISTICS;

  status = vfe_util_write_hw_cmd(p_obj->vfe_params.camfd,
    CMD_STATS_AF_ENABLE, &(stats->cmd), sizeof(stats->cmd),
    VFE_CMD_STATS_AF_START);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
    return VFE_ERROR_GENERAL;
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
  stats->cmd.af_enable = VFE_DISABLE_AUTOFOCUS_STATISTICS;

  CDBG("vfe_af_stats_stop\n");
  status = vfe_util_write_hw_cmd(params->camfd,
    CMD_STATS_AF_ENABLE, &(stats->cmd), sizeof(stats->cmd),
    VFE_CMD_STATS_AF_STOP);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_af_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_af_stats_enable(af_stats_t* mod, vfe_params_t *params,
  int enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;

  CDBG("vfe_af_stats_enable\n");
  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  CDBG("%s: %d", __func__, enable);

  mod->enable = enable;

  if (!mod->enable)
    return VFE_SUCCESS;
  return status;
}/*vfe_af_stats_enable*/

/*===========================================================================
 * FUNCTION    - vfe_af_stats_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_af_stats_update(af_stats_t *mod, vfe_params_t* parms)
{
   CDBG("vfe_af_stats_update\n");
  if (mod->af_update) {
    parms->update |= VFE_MOD_AF_STATS;
    mod->af_update = FALSE;
  }
  return VFE_SUCCESS;
}

