/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stddef.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_AWB_STATS_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

#define AWB_SHIFT_BITS(n) ({ \
  uint32_t s_bits; \
  s_bits = CEIL_LOG2(n); \
  s_bits = (s_bits > 8) ? (s_bits-8) : 0; \
  s_bits;})

/* Stats AWB Config Command */
#ifndef VFE_31
/* 1296 * 972 camif size.  16x16 regions. */
const struct VFE_StatsAwb_CfgCmdType VFE_DefaultStatsAwb_ConfigCmd = {
  /* reserved */
  0,   /* rgnHOffset */
  0,   /* rgnVOffset */
  1,   /* shiftBits */
  79,  /* rgnWidth  */
  59,  /* rgnHeight */
  /* reserved */
  15,  /*  rgnHNum */
  15,  /*  rgnVNum */
  241, /*  yMax    */
  10,  /*  yMin    */
  /* reserved */
  114,  /* c1  */
  /* reserved */
  136, /* c2  */
  /* reserved */
  -34, /* c3  */
  /* reserved */
  257, /* c4  */
  /* reserved */
  2, /* m1 */
  -16, /* m2 */
  16, /* m3 */
  -16, /* m4 */
  61,   /* t1 */
  32,   /* t2 */
  33,   /* t3 */
  64,   /* t6 */
  130,  /* t4 */
  /* reserved */
  157,  /* mg */
  /* reserved */
  64,  /* t5  */
};
#else
const struct VFE_StatsAwb_CfgCmdType VFE_DefaultStatsAwb_ConfigCmd = {
  /* reserved */
  5,   /* shiftBits */
  79,  /* rgnWidth  */
  59,  /* rgnHeight */
  /* reserved */
  15,  /*  rgnHNum */
  15,  /*  rgnVNum */
  241, /*  yMax    */
  10,  /*  yMin    */
  /* reserved */
  90,  /* c1  */
  /* reserved */
  152, /* c2  */
  /* reserved */
  -99, /* c3  */
  /* reserved */
  257, /* c4  */
  /* reserved */
  0x10, /* m1 */
  0xf0, /* m2 */
  0x10, /* m3 */
  0xf0, /* m4 */
  61,   /* t1 */
  32,   /* t2 */
  33,   /* t3 */
  64,   /* t6 */
  130,  /* t4 */
  /* reserved */
  157,  /* mg */
  /* reserved */
  64,  /* t5  */
};
#endif

/*===========================================================================
 * FUNCTION    - vfe_awb_stats_get_shiftbits -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_awb_stats_get_shiftbits(awb_stats_t *mod,
  vfe_params_t *vfe_params, uint32_t *p_shiftbits)
{
  CDBG("%s: %d", __func__, mod->VFE_StatsAwb_ConfigCmd.shiftBits);
  *p_shiftbits = mod->VFE_StatsAwb_ConfigCmd.shiftBits;
  return VFE_SUCCESS;
}/*vfe_awb_stats_get_shiftbits*/

/*===========================================================================
 * FUNCTION    - vfe_stats_awb_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_awb_stats_init(int mod_id, void *stats_wb, void *vparams)
{
  awb_stats_t* mod = (awb_stats_t *)stats_wb;
  vfe_params_t *params = (vfe_params_t *)vparams;
  int is_vfe3_3 = false;
  VFE_StatsAwb_CfgCmdType *pcmd = &mod->VFE_StatsAwb_ConfigCmd;
  *pcmd = VFE_DefaultStatsAwb_ConfigCmd;
  mod->enable = 0;
#ifndef VFE_31
  if (is_vfe3_3) {
    mod->VFE_StatsAwb_ConfigCmd.rgnHOffset = 1;
    mod->VFE_StatsAwb_ConfigCmd.rgnVOffset = 1;
  }
#endif
  params->awb_params.bounding_box.c1 = pcmd->c1;
  params->awb_params.bounding_box.c2 = pcmd->c2;
  params->awb_params.bounding_box.c3 = pcmd->c3;
  params->awb_params.bounding_box.c4 = pcmd->c4;
  params->awb_params.bounding_box.m1 = pcmd->m1;
  params->awb_params.bounding_box.m2 = pcmd->m2;
  params->awb_params.bounding_box.m3 = pcmd->m3;
  params->awb_params.bounding_box.m4 = pcmd->m4;
  params->awb_params.exterme_col_param.mg = pcmd->mg;
  params->awb_params.exterme_col_param.t1 = pcmd->t1;
  params->awb_params.exterme_col_param.t2 = pcmd->t2;
  params->awb_params.exterme_col_param.t3 = pcmd->t3;
  params->awb_params.exterme_col_param.t4 = pcmd->t4;
  params->awb_params.exterme_col_param.t5 = pcmd->t5;
  params->awb_params.exterme_col_param.t6 = pcmd->t6;
  params->awb_params.bounding_box.y_max = pcmd->yMax;
  params->awb_params.bounding_box.y_min = pcmd->yMin;
  return VFE_SUCCESS;
} /* vfe_awb_stats_init */

/*===========================================================================
 * FUNCTION    - vfe_awb_stats_enable -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_awb_stats_enable(int mod_id, void *stats_wb, void *vparams,
  int8_t enable, int8_t hw_write)
{
  awb_stats_t* mod = (awb_stats_t *)stats_wb;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;

  if (!IS_BAYER_FORMAT(params))
    enable = FALSE;

  CDBG("%s: %d", __func__, enable);
  mod->enable = enable;
  params->moduleCfg->statsAwbEnable = enable;

  if (!mod->enable)
    return VFE_SUCCESS;

  return status;
} /* vfe_awb_stats_enable */

/*===========================================================================
 * FUNCTION    - vfe_awb_stats_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_awb_stats_debug(VFE_StatsAwb_CfgCmdType *pcmd)
{
  CDBG("AWB statsconfig shiftBits %d\n", pcmd->shiftBits);
  CDBG("AWB statsconfig rgnWidth  %d\n", pcmd->rgnWidth);
  CDBG("AWB statsconfig rgnHeight %d\n", pcmd->rgnHeight);
#ifndef VFE_31
  CDBG("AWB statsconfig rgnHOffset  %d\n", pcmd->rgnHOffset);
  CDBG("AWB statsconfig rgnVOffset %d\n", pcmd->rgnVOffset);
#endif
  CDBG("AWB statsconfig rgnHNum   %d\n", pcmd->rgnHNum);
  CDBG("AWB statsconfig rgnVNum   %d\n", pcmd->rgnVNum);
  CDBG("AWB statsconfig yMax      %d\n", pcmd->yMax);
  CDBG("AWB statsconfig yMin      %d\n", pcmd->yMin);

  CDBG("AWB statsconfig t1 %d\n", pcmd->t1);
  CDBG("AWB statsconfig t2 %d\n", pcmd->t2);
  CDBG("AWB statsconfig t3 %d\n", pcmd->t3);
  CDBG("AWB statsconfig t4 %d\n", pcmd->t4);
  CDBG("AWB statsconfig mg %d\n", pcmd->mg);
  CDBG("AWB statsconfig t5 %d\n", pcmd->t5);
  CDBG("AWB statsconfig t6 %d\n", pcmd->t6);

  CDBG("AWB statsconfig m1 %d\n", pcmd->m1);
  CDBG("AWB statsconfig m2 %d\n", pcmd->m2);
  CDBG("AWB statsconfig m3 %d\n", pcmd->m3);
  CDBG("AWB statsconfig m4 %d\n", pcmd->m4);

  CDBG("AWB statsconfig c1 %d\n", pcmd->c1);
  CDBG("AWB statsconfig c2 %d\n", pcmd->c2);
  CDBG("AWB statsconfig c3 %d\n", pcmd->c3);
  CDBG("AWB statsconfig c4 %d\n", pcmd->c4);
}/* vfe_awb_stats_debug */

/*===========================================================================
 * FUNCTION    - vfe_awb_stats_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_awb_stats_config(int mod_id, void *stats_wb, void *vparams)
{
  awb_stats_t* mod = (awb_stats_t *)stats_wb;
  vfe_params_t *params = (vfe_params_t *)vparams;
  VFE_StatsAwb_CfgCmdType *pcmd =
    (VFE_StatsAwb_CfgCmdType *)&(mod->VFE_StatsAwb_ConfigCmd);
  uint32_t pix_per_region;
  vfe_status_t status = VFE_SUCCESS;
  int is_vfe3_3 = false;
  uint32_t width, height;

  if (!mod->enable) {
    CDBG("%s: AWB stats not enabled", __func__);
    return VFE_SUCCESS;
  }

  width= params->demosaic_op_params.last_pixel - params->demosaic_op_params.first_pixel + 1;
  pcmd->rgnWidth = ((width/(pcmd->rgnHNum + 1)) - 1);

  height = params->demosaic_op_params.last_line - params->demosaic_op_params.first_line + 1;
  pcmd->rgnHeight = ((height/(pcmd->rgnVNum + 1)) - 1);

#ifndef VFE_31
  if (is_vfe3_3) {
    uint32_t temp = pcmd->rgnWidth/pcmd->rgnHNum;
    pcmd->rgnHOffset =
      MIN(temp, (params->awb_params.region_info.regionHOffset+1));
    temp = width - ((pcmd->rgnWidth+1) * (pcmd->rgnHNum+1));
    pcmd->rgnHOffset = MIN(pcmd->rgnHOffset, temp);
    pcmd->rgnHOffset = MAX(1, pcmd->rgnHOffset);

    temp = pcmd->rgnHeight/pcmd->rgnVNum;
    pcmd->rgnVOffset =
      MIN(temp, (params->awb_params.region_info.regionVOffset+1));
    temp = height - ((pcmd->rgnHeight+1) * (pcmd->rgnVNum+1));
    pcmd->rgnVOffset = MIN(pcmd->rgnVOffset, temp);
    pcmd->rgnVOffset = MAX(1, pcmd->rgnVOffset);
  }
#endif

  pix_per_region = (pcmd->rgnWidth+1) * (pcmd->rgnHeight+1);
  CDBG("%s: pix_per_region %d %dx%d", __func__, pix_per_region,
    pcmd->rgnWidth, pcmd->rgnHeight);
#ifndef VFE_31
  pcmd->shiftBits = 0;
#else
  pcmd->shiftBits = AWB_SHIFT_BITS(pix_per_region);
#endif
  pcmd->yMax = params->awb_params.bounding_box.y_max;
  pcmd->yMin = params->awb_params.bounding_box.y_min;
  pcmd->t1 = params->awb_params.exterme_col_param.t1;
  pcmd->t2 = params->awb_params.exterme_col_param.t2;
  pcmd->t3 = params->awb_params.exterme_col_param.t3;
  pcmd->t4 = params->awb_params.exterme_col_param.t4;
  pcmd->t5 = params->awb_params.exterme_col_param.t5;
  pcmd->t6 = params->awb_params.exterme_col_param.t6;
  pcmd->mg = params->awb_params.exterme_col_param.mg;

  pcmd->c1 = params->awb_params.bounding_box.c1;
  pcmd->c2 = params->awb_params.bounding_box.c2;
  pcmd->c3 = params->awb_params.bounding_box.c3;
  pcmd->c4 = params->awb_params.bounding_box.c4;
  pcmd->m1 = params->awb_params.bounding_box.m1;
  pcmd->m2 = params->awb_params.bounding_box.m2;
  pcmd->m3 = params->awb_params.bounding_box.m3;
  pcmd->m4 = params->awb_params.bounding_box.m4;
  vfe_awb_stats_debug(pcmd);

  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, (void *) pcmd,
    sizeof(VFE_StatsAwb_CfgCmdType), VFE_CMD_STATS_AWB_START);

  if (status == VFE_SUCCESS)
    mod->update = FALSE;
  return status;
}/*vfe_awb_stats_config*/

/*===========================================================================
 * FUNCTION    - vfe_awb_stats_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_awb_stats_update(int mod_id, void *stats_wb, void *vparams)
{
  awb_stats_t* mod = (awb_stats_t *)stats_wb;
  vfe_params_t *params = (vfe_params_t *)vparams;
  VFE_StatsAwb_CfgCmdType *pcmd =
    (VFE_StatsAwb_CfgCmdType *)&(mod->VFE_StatsAwb_ConfigCmd);
  vfe_status_t status = VFE_SUCCESS;

  if (!mod->enable) {
    CDBG("%s: AWB stats not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!mod->update) {
    CDBG("%s: not updated", __func__);
    return VFE_SUCCESS;
  }

  pcmd->yMax = params->awb_params.bounding_box.y_max;
  pcmd->yMin = params->awb_params.bounding_box.y_min;
  pcmd->t1 = params->awb_params.exterme_col_param.t1;
  pcmd->t2 = params->awb_params.exterme_col_param.t2;
  pcmd->t3 = params->awb_params.exterme_col_param.t3;
  pcmd->t4 = params->awb_params.exterme_col_param.t4;
  pcmd->t5 = params->awb_params.exterme_col_param.t5;
  pcmd->t6 = params->awb_params.exterme_col_param.t6;
  pcmd->mg = params->awb_params.exterme_col_param.mg;

  pcmd->c1 = params->awb_params.bounding_box.c1;
  pcmd->c2 = params->awb_params.bounding_box.c2;
  pcmd->c3 = params->awb_params.bounding_box.c3;
  pcmd->c4 = params->awb_params.bounding_box.c4;
  pcmd->m1 = params->awb_params.bounding_box.m1;
  pcmd->m2 = params->awb_params.bounding_box.m2;
  pcmd->m3 = params->awb_params.bounding_box.m3;
  pcmd->m4 = params->awb_params.bounding_box.m4;
  CDBG("%s: AWB update ymin %d", __func__, pcmd->yMin);
  //vfe_awb_stats_debug(pcmd);
  status = vfe_util_write_hw_cmd(params->camfd, CMD_GENERAL, (void *) pcmd,
    sizeof(VFE_StatsAwb_CfgCmdType), VFE_CMD_STATS_AWB_UPDATE);

  if (status == VFE_SUCCESS) {
    mod->update = FALSE;
    params->update |= VFE_MOD_AWB_STATS;
  }
  return status;
}/*vfe_awb_stats_update*/

/*===========================================================================
 * FUNCTION    - vfe_awb_stats_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_awb_stats_trigger_update(int mod_id, void *stats_wb,
  void *vparams)
{
  awb_stats_t* mod = (awb_stats_t *)stats_wb;
  vfe_params_t *params = (vfe_params_t *)vparams;
  if (!mod->enable) {
    CDBG("%s: not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (mod->VFE_StatsAwb_ConfigCmd.yMin != params->awb_params.bounding_box.y_min)
    mod->update = TRUE;
  CDBG("%s: update %d", __func__, mod->update);
  return VFE_SUCCESS;
}/*vfe_awb_stats_trigger_update*/

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_awb_stats_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_awb_stats_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  awb_stats_module_t *cmd = (awb_stats_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->VFE_StatsAwb_ConfigCmd),
     sizeof(VFE_StatsAwb_CfgCmdType),
     VFE_CMD_STATS_AWB_UPDATE)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_awb_stats_plugin_update */
#endif
