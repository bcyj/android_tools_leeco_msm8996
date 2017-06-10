/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#define RS_MAX_V_REGIONS 1024
#define RS_MAX_H_REGIONS 4

#define CS_MAX_V_REGIONS 3
#define CS_MAX_H_REGIONS 1344

#if 0
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - calc_rs_window_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void calc_rs_window_config(pixel_crop_info_t *input_window,
  vfe_params_t *vfe_params)
{
  uint32_t stats_in_height, stats_in_width, remainder_h, remainder_v;
  rscs_stat_config_type *config = &(vfe_params->rs_cs_params.config);

  /* config v */
  stats_in_height = vfe_params->vfe_input_win.height;
  remainder_v = (stats_in_height % RS_MAX_V_REGIONS) / 2;
  vfe_params->rs_cs_params.rs_max_rgns = RS_MAX_V_REGIONS;

  config->row_sum_enable = 1;
  config->row_sum_ver_Toffset_ratio = 0.0;
  config->row_sum_ver_Boffset_ratio = 0.0;
  config->row_sum_V_subsample_ratio = stats_in_height / RS_MAX_V_REGIONS;

  if (remainder_v > 0)
    config->row_sum_V_subsample_ratio += 1;

  /* NEW config h*/
  stats_in_width = vfe_params->vfe_input_win.width;
  remainder_h = (stats_in_width % RS_MAX_H_REGIONS) / 2;
  vfe_params->rs_cs_params.rs_max_h_rgns = RS_MAX_H_REGIONS;

  config->row_sum_hor_Loffset_ratio = 0.0;
  config->row_sum_hor_Roffset_ratio = 0.0;
  config->row_sum_H_subsample_ratio = stats_in_width / RS_MAX_H_REGIONS;

   if (remainder_h > 0)
    config->row_sum_H_subsample_ratio += 1;

  CDBG("%s: RS window config\n", __func__);
  CDBG("%s: RS Hor L_off = %f\n", __func__, config->row_sum_hor_Loffset_ratio);
  CDBG("%s: RS Hor R_off = %f\n", __func__, config->row_sum_hor_Roffset_ratio);
  CDBG("%s: RS Ver T_off = %f\n", __func__, config->row_sum_ver_Toffset_ratio);
  CDBG("%s: RS Ver B_off = %f\n", __func__, config->row_sum_ver_Boffset_ratio);
  CDBG("%s: RS subsample ratio V Ratio = %d\n", __func__, config->row_sum_V_subsample_ratio);
  CDBG("%s: RS subsample ratio H Ratio = %d\n", __func__, config->row_sum_H_subsample_ratio);
} /* calc_rs_window_config */

/*===========================================================================
 * FUNCTION    - calc_cs_window_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void calc_cs_window_config(pixel_crop_info_t *input_window,
  vfe_params_t *vfe_params)
{
  uint32_t stats_in_width, stats_in_height, remainder_h, remainder_v;
  rscs_stat_config_type *config = &(vfe_params->rs_cs_params.config);

  /*config width */
  stats_in_width = vfe_params->vfe_input_win.width;
  remainder_h = (stats_in_width % CS_MAX_H_REGIONS) / 2;
  vfe_params->rs_cs_params.cs_max_rgns = CS_MAX_H_REGIONS;

  config->col_sum_enable = 1;
  config->col_sum_hor_Loffset_ratio = 0.0;
  config->col_sum_hor_Roffset_ratio = 0.0;
  config->col_sum_H_subsample_ratio = stats_in_width / CS_MAX_H_REGIONS;

  if (remainder_h > 0)
    config->col_sum_H_subsample_ratio += 1;


  /*config height*/

  stats_in_height = vfe_params->vfe_input_win.height;
  remainder_v = (stats_in_height % CS_MAX_V_REGIONS) / 2;
  vfe_params->rs_cs_params.cs_max_v_rgns = CS_MAX_V_REGIONS;

  config->col_sum_ver_Toffset_ratio = 0.0;
  config->col_sum_ver_Boffset_ratio = 0.0;
  config->col_sum_V_subsample_ratio = stats_in_height / CS_MAX_V_REGIONS;

  if (remainder_v > 0)
    config->col_sum_V_subsample_ratio += 1;

  CDBG("%s: CS window config\n", __func__);
  CDBG("%s: CS Hor L_off = %f\n", __func__, config->col_sum_hor_Loffset_ratio);
  CDBG("%s: CS Hor R_off = %f\n", __func__, config->col_sum_hor_Roffset_ratio);
  CDBG("%s: CS Ver T_off = %f\n", __func__, config->col_sum_ver_Toffset_ratio);
  CDBG("%s: CS Ver B_off = %f\n", __func__, config->col_sum_ver_Boffset_ratio);
  CDBG("%s: CS subsample_H Ratio = %d\n", __func__, config->col_sum_H_subsample_ratio);
  CDBG("%s: CS subsample_V Ratio = %d\n", __func__, config->col_sum_V_subsample_ratio);
} /* calc_rs_window_config */

/*===========================================================================
 * FUNCTION    - vfe_stats_rs_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rs_stats_config(int mod_id, void *stats_rs, void *vparams)
{
  rs_stats_t *rs_stats = (rs_stats_t *)stats_rs;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  uint32_t stats_input_width, stats_input_height, effective_height, effective_width;
  uint32_t effective_num_v_rgn, effective_num_h_rgn;
  pixel_crop_info_t *input_window = &(vfe_params->demosaic_op_params);
  rscs_stat_config_type  *rscs_config = NULL;

  if (!rs_stats->enable) {
    CDBG("%s: not enabled", __func__);
    return VFE_SUCCESS;
  }

  calc_rs_window_config(input_window, vfe_params);

  rscs_config = &(vfe_params->rs_cs_params.config);

  stats_input_width = vfe_params->vfe_input_win.width;
  stats_input_height = vfe_params->vfe_input_win.height;

  effective_height = stats_input_height *
    (1 - rscs_config->row_sum_ver_Toffset_ratio -
    rscs_config->row_sum_ver_Boffset_ratio);
   effective_width = stats_input_width *
    (1 - rscs_config->row_sum_hor_Roffset_ratio -
    rscs_config->row_sum_hor_Loffset_ratio);

  rscs_config->row_sum_V_subsample_ratio =
    MAX(1, rscs_config->row_sum_V_subsample_ratio);
  rscs_config->row_sum_V_subsample_ratio =
    MIN(4, rscs_config->row_sum_V_subsample_ratio);

  effective_num_v_rgn =
    effective_height / rscs_config->row_sum_V_subsample_ratio - 1;
  effective_num_h_rgn =
    effective_width / rscs_config->row_sum_H_subsample_ratio - 1;

  CDBG("%s Effective Height = %d ", __func__, effective_height);
  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_STATS_RS_ENABLE, 0, 0, 0);

  CDBG("MSM_CAM_IOCTL_CONFIG_VFE CMD_STATS_RS_ENABLE \n");

  rs_stats->rs_stats_cmd.rgnVNum = MIN(RS_MAX_V_REGIONS, effective_num_v_rgn);
  rs_stats->rs_stats_cmd.rgnHNum = MIN(RS_MAX_H_REGIONS, effective_num_h_rgn);
  rs_stats->rs_stats_cmd.rgnHeight = rscs_config->row_sum_V_subsample_ratio -1;
  rs_stats->rs_stats_cmd.rgnHOffset =
    (stats_input_width * rscs_config->row_sum_hor_Loffset_ratio);
  rs_stats->rs_stats_cmd.rgnVOffset =
    (stats_input_height * rscs_config->row_sum_ver_Toffset_ratio);
  rs_stats->rs_stats_cmd.rgnWidth =
    stats_input_width * (1 - rscs_config->row_sum_hor_Loffset_ratio -
    rscs_config->row_sum_hor_Roffset_ratio) - 1;
  rs_stats->rs_stats_cmd.shiftBits =
    vfe_util_calculate_shift_bits(rs_stats->rs_stats_cmd.rgnWidth);

  vfe_params->rs_cs_params.rs_num_rgns = rs_stats->rs_stats_cmd.rgnVNum + 1;
   vfe_params->rs_cs_params.rs_num_h_rgns = rs_stats->rs_stats_cmd.rgnHNum + 1;
  vfe_params->rs_cs_params.rs_shift_bits = rs_stats->rs_stats_cmd.shiftBits;

  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL,
    &(rs_stats->rs_stats_cmd),sizeof(rs_stats->rs_stats_cmd),
    VFE_CMD_STATS_RS_START);

  CDBG("RS Stats Configurations\n");
  CDBG("rgnVNum = %d \n",rs_stats->rs_stats_cmd.rgnVNum);
  CDBG("rgnHNum = %d \n",rs_stats->rs_stats_cmd.rgnHNum);
  CDBG("rgnHeight = %d \n",rs_stats->rs_stats_cmd.rgnHeight);
  CDBG("rgnHOffset = %d \n",rs_stats->rs_stats_cmd.rgnHOffset);
  CDBG("rgnVOffset = %d \n",rs_stats->rs_stats_cmd.rgnVOffset);
  CDBG("rgnWidth = %d \n",rs_stats->rs_stats_cmd.rgnWidth);
  CDBG("shiftBits = %d \n",rs_stats->rs_stats_cmd.shiftBits);

  return VFE_SUCCESS;
} /* vfe_stats_rs_config */

/*===========================================================================
 * FUNCTION    - vfe_stats_cs_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_cs_stats_config(int mod_id, void *stats_cs, void *vparams)
{
  cs_stats_t *cs_stats = (cs_stats_t *)stats_cs;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  uint32_t stats_input_width, stats_input_height, effective_width, effective_height;
  uint32_t effective_num_h_rgn, effective_num_v_rgn;
  pixel_crop_info_t *input_window = &(vfe_params->demosaic_op_params);
  rscs_stat_config_type  *rscs_config = NULL;

  if (!cs_stats->enable) {
    CDBG("%s: not enabled", __func__);
    return VFE_SUCCESS;
  }

  calc_cs_window_config(input_window, vfe_params);

  rscs_config = &(vfe_params->rs_cs_params.config);

  stats_input_width = vfe_params->vfe_input_win.width;
  stats_input_height = vfe_params->vfe_input_win.height;

  /* config Horizon*/
  effective_width = stats_input_width *
    (1 - rscs_config->col_sum_hor_Loffset_ratio -
    rscs_config->col_sum_hor_Roffset_ratio);
  effective_height = stats_input_height*
    (1 - rscs_config->col_sum_ver_Boffset_ratio -
    rscs_config->col_sum_ver_Toffset_ratio);

  rscs_config->col_sum_H_subsample_ratio =
    MAX(2, rscs_config->col_sum_H_subsample_ratio);
  rscs_config->col_sum_H_subsample_ratio =
    MIN(4, rscs_config->col_sum_H_subsample_ratio);

  effective_num_h_rgn =
    effective_width / rscs_config->col_sum_H_subsample_ratio - 1;
  effective_num_v_rgn =
    effective_height / rscs_config->col_sum_V_subsample_ratio - 1;

  CDBG("%s Effective Width = %d ", __func__, effective_width);

  vfe_util_write_hw_cmd(vfe_params->camfd, CMD_STATS_CS_ENABLE, 0, 0, 0);

  CDBG("MSM_CAM_IOCTL_CONFIG_VFE CMD_STATS_CS_ENABLE \n");

  cs_stats->cs_stats_cmd.rgnHNum = MIN(CS_MAX_H_REGIONS, effective_num_h_rgn);
  cs_stats->cs_stats_cmd.rgnVNum = MIN(CS_MAX_V_REGIONS, effective_num_v_rgn);
  cs_stats->cs_stats_cmd.rgnHeight = stats_input_height *
    (1- rscs_config->col_sum_ver_Toffset_ratio -
    rscs_config->col_sum_ver_Boffset_ratio) -1;
  cs_stats->cs_stats_cmd.rgnHOffset =
    stats_input_width * rscs_config->col_sum_hor_Loffset_ratio;
  cs_stats->cs_stats_cmd.rgnVOffset =
    stats_input_height * rscs_config->col_sum_ver_Toffset_ratio;
  cs_stats->cs_stats_cmd.rgnWidth =
    rscs_config->col_sum_H_subsample_ratio -1;
  cs_stats->cs_stats_cmd.shiftBits =
    vfe_util_calculate_shift_bits(cs_stats->cs_stats_cmd.rgnHeight);

  vfe_params->rs_cs_params.cs_num_rgns = cs_stats->cs_stats_cmd.rgnHNum + 1;
  vfe_params->rs_cs_params.cs_num_v_rgns = cs_stats->cs_stats_cmd.rgnVNum + 1;
  vfe_params->rs_cs_params.cs_shift_bits = cs_stats->cs_stats_cmd.shiftBits;

  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL,
    &(cs_stats->cs_stats_cmd),sizeof(cs_stats->cs_stats_cmd),
    VFE_CMD_STATS_CS_START);

  CDBG("CS Stats Configurations\n");
  CDBG("rgnHNum = %d \n",cs_stats->cs_stats_cmd.rgnHNum);
  CDBG("rgnVNum = %d \n",cs_stats->cs_stats_cmd.rgnVNum);
  CDBG("rgnHeight = %d \n",cs_stats->cs_stats_cmd.rgnHeight);
  CDBG("rgnHOffset = %d \n",cs_stats->cs_stats_cmd.rgnHOffset);
  CDBG("rgnVOffset = %d \n",cs_stats->cs_stats_cmd.rgnVOffset);
  CDBG("rgnWidth = %d \n",cs_stats->cs_stats_cmd.rgnWidth);
  CDBG("shiftBits = %d \n",cs_stats->cs_stats_cmd.shiftBits);

  return VFE_SUCCESS;
} /* vfe_stats_cs_config */

/*===========================================================================
 * FUNCTION    - vfe_rs_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_rs_stats_enable(int mod_id, void *stats_rs, void *vparams,
  int8_t enable, int8_t hw_write)
{
  rs_stats_t *mod = (rs_stats_t *)stats_rs;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  CDBG("%s: %d", __func__, enable);

  mod->enable = enable;
  params->moduleCfg->statsRsEnable = enable;

  return status;
}/*vfe_rs_stats_enable*/

/*===========================================================================
 * FUNCTION    - vfe_cs_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_cs_stats_enable(int mod_id, void *stats_cs, void *vparams,
  int8_t enable, int8_t hw_write)
{
  cs_stats_t *mod = (cs_stats_t *)stats_cs;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;
  CDBG("%s: %d", __func__, enable);

  mod->enable = enable;
  params->moduleCfg->statsCsEnable = enable;

  return status;
}/*vfe_cs_stats_enable*/
