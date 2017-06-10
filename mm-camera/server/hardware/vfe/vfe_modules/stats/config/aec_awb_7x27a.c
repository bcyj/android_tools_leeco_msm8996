/*============================================================================
   Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#define ENABLE_AEC_AWB_STATS_LOGGING 0
#if ENABLE_AEC_AWB_STATS_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - vfe_aecawb_stats_debug -
 *
 * DESCRIPTION:  Initialize the config command with defaults
 *==========================================================================*/
void vfe_aecawb_stats_debug(aecawb_stats_t *stats)
{
  CDBG("AEC AWB Stats Configuration command\n");
  CDBG("cmd.wb_expstatsenable : %d\n", stats->cmd.wb_expstatsenable);
  CDBG("cmd.wb_expstatbuspriorityselection : %d\n",
    stats->cmd.wb_expstatbuspriorityselection);
  CDBG("cmd.wb_expstatbuspriorityvalue : %d\n",
    stats->cmd.wb_expstatbuspriorityvalue);
  CDBG("cmd.exposurestatregions : %d\n", stats->cmd.exposurestatregions);
  CDBG("cmd.exposurestatsubregions : %d\n", stats->cmd.exposurestatsubregions);
  CDBG("cmd.whitebalanceminimumy : %d\n", stats->cmd.whitebalanceminimumy);
  CDBG("cmd.whitebalancemaximumy : %d\n", stats->cmd.whitebalancemaximumy);
  CDBG("cmd.wb_expstatslopeofneutralregionline0 : %d\n",
    stats->cmd.wb_expstatslopeofneutralregionline[0]);
  CDBG("cmd.wb_expstatslopeofneutralregionline1 : %d\n",
    stats->cmd.wb_expstatslopeofneutralregionline[1]);
  CDBG("cmd.wb_expstatslopeofneutralregionline2 : %d\n",
    stats->cmd.wb_expstatslopeofneutralregionline[2]);
  CDBG("cmd.wb_expstatslopeofneutralregionline3 : %d\n",
    stats->cmd.wb_expstatslopeofneutralregionline[3]);
  CDBG("cmd.wb_expstatcbinterceptofneutralregionline1 : %d\n",
    stats->cmd.wb_expstatcbinterceptofneutralregionline1);
  CDBG("cmd.wb_expstatcrinterceptofneutralregionline2 : %d\n",
    stats->cmd.wb_expstatcrinterceptofneutralregionline2);
  CDBG("cmd.wb_expstatcbinterceptofneutralregionline3 : %d\n",
    stats->cmd.wb_expstatcbinterceptofneutralregionline3);
  CDBG("cmd.wb_expstatcrinterceptofneutralregionline4 : %d\n",
    stats->cmd.wb_expstatcrinterceptofneutralregionline4);
  CDBG("cmd.wb_expmetricheaderpattern : %d\n",
    stats->cmd.wb_expmetricheaderpattern);
  CDBG("cmd.wb_expstatoutputbuffer[0] : %p\n",
    stats->cmd.wb_expstatoutputbuffer[0]);
  CDBG("cmd.wb_expstatoutputbuffer[1]: %p\n",
    stats->cmd.wb_expstatoutputbuffer[1]);
  CDBG("cmd.wb_expstatoutputbuffer[2]: %p\n",
    stats->cmd.wb_expstatoutputbuffer[2]);

  CDBG("AEC AWB Update command\n");
  CDBG("cmd.exposurestatregions : %d\n", stats->wb_cmd.exposureRegions);
  CDBG("cmd.exposurestatsubregions : %d\n", stats->wb_cmd.exposureSubRegions);
  CDBG("cmd.whitebalanceminimumy : %d\n", stats->wb_cmd.whiteBalanceMinimumY);
  CDBG("cmd.whitebalancemaximumy : %d\n", stats->wb_cmd.whiteBalanceMaximumY);
  CDBG("cmd.wb_expstatslopeofneutralregionline0 : %d\n",
    stats->wb_cmd.WB_EXPStatSlopeOfNeutralRegionLine[0]);
  CDBG("cmd.wb_expstatslopeofneutralregionline1 : %d\n",
    stats->wb_cmd.WB_EXPStatSlopeOfNeutralRegionLine[1]);
  CDBG("cmd.wb_expstatslopeofneutralregionline2 : %d\n",
    stats->wb_cmd.WB_EXPStatSlopeOfNeutralRegionLine[2]);
  CDBG("cmd.wb_expstatslopeofneutralregionline3 : %d\n",
    stats->wb_cmd.WB_EXPStatSlopeOfNeutralRegionLine[3]);
  CDBG("cmd.WB_EXPStatCbInterceptOfNeutralRegionLine1 : %d\n",
    stats->wb_cmd.WB_EXPStatCbInterceptOfNeutralRegionLine1);
  CDBG("cmd.WB_EXPStatCrInterceptOfNeutralRegionLine2 : %d\n",
    stats->wb_cmd.WB_EXPStatCrInterceptOfNeutralRegionLine2);
  CDBG("cmd.WB_EXPStatCbInterceptOfNeutralRegionLine3 : %d\n",
    stats->wb_cmd.WB_EXPStatCbInterceptOfNeutralRegionLine3);
  CDBG("cmd.WB_EXPStatCrInterceptOfNeutralRegionLine4 : %d\n",
    stats->wb_cmd.WB_EXPStatCrInterceptOfNeutralRegionLine4);
}

/*===========================================================================
 * FUNCTION    - vfe_aecawb_stats_init -
 *
 * DESCRIPTION:  Initialize the config command with defaults
 *==========================================================================*/
vfe_status_t vfe_aecawb_stats_init(aecawb_stats_t *stats,
  vfe_params_t *vParams)
{
  stats->cmd.wb_expstatsenable = VFE_ENABLE_WB_EXP_STATS;
  stats->cmd.wb_expstatbuspriorityselection =
    VFE_HARDWARE_BUFFER_LEVEL_STATISTICS_BUS_PRIORITY;
  stats->cmd.wb_expstatbuspriorityvalue = 7;
  //TODO: Need to pass the regions to 3a
  stats->cmd.exposurestatregions = VFE_16_X_16_EXPOSURE_STAT_REGIONS;
  stats->cmd.exposurestatsubregions = VFE_8_X_8_EXPOSURE_STAT_SUB_REGIONS;
  stats->cmd.whitebalanceminimumy =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].y_min;
  stats->cmd.whitebalancemaximumy =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].y_max;
  stats->cmd.wb_expstatslopeofneutralregionline[0] =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].m4;
  stats->cmd.wb_expstatslopeofneutralregionline[1] =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].m3;
  stats->cmd.wb_expstatslopeofneutralregionline[2] =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].m2;
  stats->cmd.wb_expstatslopeofneutralregionline[3] =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].m1;
  stats->cmd.wb_expstatcbinterceptofneutralregionline1 =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].c1;
  stats->cmd.wb_expstatcrinterceptofneutralregionline2 =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].c2;
  stats->cmd.wb_expstatcbinterceptofneutralregionline3 =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].c3;
  stats->cmd.wb_expstatcrinterceptofneutralregionline4 =
    vParams->chroma3a->wb_exp_stats_hw_rolloff[AWB_STATS_NORMAL_LIGHT].c4;
  stats->cmd.wb_expmetricheaderpattern = 0xAE;
  stats->cmd.wb_expstatoutputbuffer[0] = NULL;
  stats->cmd.wb_expstatoutputbuffer[1] = NULL;
  stats->cmd.wb_expstatoutputbuffer[2] = NULL;
  stats->update = TRUE;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_aecawb_stats_config -
 *
 * DESCRIPTION:  Initialize the config command with defaults
 *==========================================================================*/
vfe_status_t vfe_aecawb_stats_config(aecawb_stats_t *stats,
  vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  uint32_t num_pixel;
  int32_t shift_region, shift_subregion;

  CDBG("%s: \n", __func__);
  if (!stats->enable) {
    CDBG("%s: not enabled, config failed : %d\n", __func__, stats->enable);
    return VFE_SUCCESS;
  }

  if (stats->cmd.exposurestatregions == VFE_8_X_8_EXPOSURE_STAT_REGIONS)
    shift_region = 3;
  else if (stats->cmd.exposurestatregions == VFE_16_X_16_EXPOSURE_STAT_REGIONS)
    shift_region = 4;
  else
    shift_region = 2;

  if (stats->cmd.exposurestatsubregions == VFE_4_X_4_EXPOSURE_STAT_SUB_REGIONS)
    shift_subregion = 2;
  else if (stats->cmd.exposurestatsubregions == VFE_8_X_8_EXPOSURE_STAT_SUB_REGIONS)
    shift_subregion = 3;

  stats->rgn_width =
    ((params->active_crop_info.lastPixel - params->active_crop_info.firstPixel + 1) >>
     (shift_region + shift_subregion) << shift_subregion);
  stats->rgn_height =
    ((params->active_crop_info.lastLine - params->active_crop_info.firstLine) >>
     (shift_region + shift_subregion) << shift_subregion);

  vfe_aecawb_stats_debug(stats);
  status = vfe_util_write_hw_cmd(params->camfd,
    CMD_STATS_AEC_AWB_ENABLE, &(stats->cmd), sizeof(stats->cmd),VFE_CMD_STATS_WB_AEC_CONFIG);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s: failed %d", __func__, status);
  }
  return status;
}
/*===========================================================================
 * FUNCTION    - vfe_aecawb_stats_enable -
 *
 * DESCRIPTION:  Initialize the config command with defaults
 *==========================================================================*/
vfe_status_t vfe_aecawb_stats_enable(aecawb_stats_t *mod, vfe_params_t *params,
  int enable, int8_t hw_write)
{
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;

  CDBG("%s: %d", __func__, enable);

  mod->enable = enable;
  //TODO: enable stats in operation config
  //params->moduleCfg->statsAeBgEnable = enable;
  return VFE_SUCCESS;

}/* vfe_aecawb_stats_enable */

/*===========================================================================
 * FUNCTION    - vfe_awb_stats_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_aecawb_stats_update(aecawb_stats_t *mod,
  vfe_params_t *params)
{
  vfe_statswb_updatecmdtype_t *pcmd =
    (vfe_statswb_updatecmdtype_t *)&(mod->wb_cmd);
  vfe_status_t status = VFE_SUCCESS;

  CDBG("%s: \n", __func__);
  if (!mod->enable) {
    CDBG_HIGH("%s: AWB_AEC stats not enabled", __func__);
    return VFE_SUCCESS;
  }
#if 0
  if (!mod->update) {
    CDBG("%s: not updated", __func__);
    return VFE_SUCCESS;
  }
#endif
  pcmd->exposureRegions = mod->cmd.exposurestatregions;
  pcmd->exposureSubRegions = mod->cmd.exposurestatsubregions;
  pcmd->whiteBalanceMaximumY = params->awb_params.bounding_box.y_max;
  pcmd->whiteBalanceMinimumY = params->awb_params.bounding_box.y_min;

  pcmd->WB_EXPStatCbInterceptOfNeutralRegionLine1 =
    params->awb_params.bounding_box.c1;
  pcmd->WB_EXPStatCrInterceptOfNeutralRegionLine2 =
    params->awb_params.bounding_box.c2;
  pcmd->WB_EXPStatCbInterceptOfNeutralRegionLine3 =
    params->awb_params.bounding_box.c3;
  pcmd->WB_EXPStatCrInterceptOfNeutralRegionLine4 =
    params->awb_params.bounding_box.c4;
  pcmd->WB_EXPStatSlopeOfNeutralRegionLine[3] =
    params->awb_params.bounding_box.m1;
  pcmd->WB_EXPStatSlopeOfNeutralRegionLine[2] =
    params->awb_params.bounding_box.m2;
  pcmd->WB_EXPStatSlopeOfNeutralRegionLine[1] =
    params->awb_params.bounding_box.m3;
  pcmd->WB_EXPStatSlopeOfNeutralRegionLine[0] =
    params->awb_params.bounding_box.m4;
  CDBG("%s: AWB update ymin %d", __func__, pcmd->whiteBalanceMinimumY);
  vfe_aecawb_stats_debug(mod);
  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, (void *) pcmd,
    sizeof(vfe_statswb_updatecmdtype_t), VFE_CMD_STATS_WB_AEC_UPDATE);

  if (status == VFE_SUCCESS) {
    mod->update = FALSE;
    params->update |= VFE_MOD_AEC_AWB_STATS;
  }
  return status;
}/* vfe_aecawb_stats_update */

/*===========================================================================
 * FUNCTION    - vfe_aecawb_stats_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_aecawb_stats_trigger_update(aecawb_stats_t* mod,
  vfe_params_t *params)
{
  if (!mod->enable) {
    CDBG("%s: not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (mod->wb_cmd.whiteBalanceMinimumY != params->awb_params.bounding_box.y_min)
    mod->update = TRUE;
  CDBG("%s: update %d", __func__, mod->update);
  return VFE_SUCCESS;

}/*vfe_aecawb_stats_trigger_update*/

