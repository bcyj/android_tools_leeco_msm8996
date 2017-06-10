/*============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_LINEAR_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

static const char * const aec_debug_str[] = {
  "LINEAR_AEC_BRIGHT",
  "LINEAR_AEC_BRIGHT_NORMAL",
  "LINEAR_AEC_NORMAL",
  "LINEAR_AEC_NORMAL_LOW",
  "LINEAR_AEC_LOW",
  "LINEAR_AEC_LUX_MAX",
};

static const char * const awb_debug_str[] = {
  "AWB_CCT_TYPE_D65",
  "AWB_CCT_TYPE_D65_TL84",
  "AWB_CCT_TYPE_TL84",
  "AWB_CCT_TYPE_TL84_A",
  "AWB_CCT_TYPE_A",
  "AWB_CCT_TYPE_MAX",
};
/*===========================================================================
 * FUNCTION    - print_vfe_linearization_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void print_vfe_linearization_config(VFE_LinearizationCmdType *cmd)
{
  int i;
  uint32_t *ptr;

  CDBG("%s: Linearization configurations\n", __func__);

  CDBG("pointSlopeR.kneePoint_P0 = %d\n",
    cmd->CfgParams.pointSlopeR.kneePoint_P0);
  CDBG("pointSlopeR.kneePoint_P1 = %d\n",
    cmd->CfgParams.pointSlopeR.kneePoint_P1);
  CDBG("pointSlopeR.kneePoint_P2 = %d\n",
    cmd->CfgParams.pointSlopeR.kneePoint_P2);
  CDBG("pointSlopeR.kneePoint_P3 = %d\n",
    cmd->CfgParams.pointSlopeR.kneePoint_P3);
  CDBG("pointSlopeR.kneePoint_P4 = %d\n",
    cmd->CfgParams.pointSlopeR.kneePoint_P4);
  CDBG("pointSlopeR.kneePoint_P5 = %d\n",
    cmd->CfgParams.pointSlopeR.kneePoint_P5);
  CDBG("pointSlopeR.kneePoint_P6 = %d\n",
    cmd->CfgParams.pointSlopeR.kneePoint_P6);
  CDBG("pointSlopeR.kneePoint_P7 = %d\n",
    cmd->CfgParams.pointSlopeR.kneePoint_P7);

  CDBG("pointSlopeGb.kneePoint_P0 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P0);
  CDBG("pointSlopeGb.kneePoint_P1 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P1);
  CDBG("pointSlopeGb.kneePoint_P2 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P2);
  CDBG("pointSlopeGb.kneePoint_P3 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P3);
  CDBG("pointSlopeGb.kneePoint_P4 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P4);
  CDBG("pointSlopeGb.kneePoint_P5 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P5);
  CDBG("pointSlopeGb.kneePoint_P6 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P6);
  CDBG("pointSlopeGb.kneePoint_P7 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P7);

  CDBG("pointSlopeB.kneePoint_P0 = %d\n",
    cmd->CfgParams.pointSlopeB.kneePoint_P0);
  CDBG("pointSlopeB.kneePoint_P1 = %d\n",
    cmd->CfgParams.pointSlopeB.kneePoint_P1);
  CDBG("pointSlopeB.kneePoint_P2 = %d\n",
    cmd->CfgParams.pointSlopeB.kneePoint_P2);
  CDBG("pointSlopeB.kneePoint_P3 = %d\n",
    cmd->CfgParams.pointSlopeB.kneePoint_P3);
  CDBG("pointSlopeB.kneePoint_P4 = %d\n",
    cmd->CfgParams.pointSlopeB.kneePoint_P4);
  CDBG("pointSlopeB.kneePoint_P5 = %d\n",
    cmd->CfgParams.pointSlopeB.kneePoint_P5);
  CDBG("pointSlopeB.kneePoint_P6 = %d\n",
    cmd->CfgParams.pointSlopeB.kneePoint_P6);
  CDBG("pointSlopeB.kneePoint_P7 = %d\n",
    cmd->CfgParams.pointSlopeB.kneePoint_P7);

  CDBG("pointSlopeGr.kneePoint_P0 = %d\n",
    cmd->CfgParams.pointSlopeGr.kneePoint_P0);
  CDBG("pointSlopeGr.kneePoint_P1 = %d\n",
    cmd->CfgParams.pointSlopeGr.kneePoint_P1);
  CDBG("pointSlopeGr.kneePoint_P2 = %d\n",
    cmd->CfgParams.pointSlopeGr.kneePoint_P2);
  CDBG("pointSlopeGr.kneePoint_P3 = %d\n",
    cmd->CfgParams.pointSlopeGr.kneePoint_P3);
  CDBG("pointSlopeGr.kneePoint_P4 = %d\n",
    cmd->CfgParams.pointSlopeGr.kneePoint_P4);
  CDBG("pointSlopeGr.kneePoint_P5 = %d\n",
    cmd->CfgParams.pointSlopeGr.kneePoint_P5);
  CDBG("pointSlopeGr.kneePoint_P6 = %d\n",
    cmd->CfgParams.pointSlopeGr.kneePoint_P6);
  CDBG("pointSlopeGr.kneePoint_P7 = %d\n",
    cmd->CfgParams.pointSlopeGr.kneePoint_P7);

} /* print_vfe_linearization_config */

/*===========================================================================
 * FUNCTION    - vfe_convert_linearization_tbl -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_convert_linearization_tbl(chromatix_linearization_type *tbl,
  VFE_LinearizationLut *pBlkTbl)
{
  uint16_t i = 0;

  for ( i = 0 ; i < 8 ; i++ ) {
    pBlkTbl->r_lut_p_l[i] = tbl->r_lut_p_l[i];
    pBlkTbl->gr_lut_p[i] = tbl->gr_lut_p[i];
    pBlkTbl->gb_lut_p[i] = tbl->gb_lut_p[i];
    pBlkTbl->b_lut_p[i] = tbl->b_lut_p[i];
  }

  for ( i = 0 ; i < 9 ; i++ ) {
    pBlkTbl->r_lut_base[i] = tbl->r_lut_base[i];
    pBlkTbl->gr_lut_base[i] = tbl->gr_lut_base[i];
    pBlkTbl->gb_lut_base[i] = tbl->gb_lut_base[i];
    pBlkTbl->b_lut_base[i] = tbl->b_lut_base[i];

    pBlkTbl->r_lut_delta[i] = FLOAT_TO_Q(9,tbl->r_lut_delta[i]);
    pBlkTbl->gr_lut_delta[i] = FLOAT_TO_Q(9,tbl->gr_lut_delta[i]);
    pBlkTbl->gb_lut_delta[i] = FLOAT_TO_Q(9,tbl->gb_lut_delta[i]);
    pBlkTbl->b_lut_delta[i] = FLOAT_TO_Q(9,tbl->b_lut_delta[i]);
  }
} /* vfe_convert_linearization_tbl */

/*===========================================================================
 * FUNCTION    - vfe_write_linearization_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_write_linearization_table(const VFE_LinearizationLut *pIn,
  VFE_LinearizationCmdType *blkConfigCmd)
{
  uint16_t i,j;
  uint32_t data = 0;

  const uint16_t* baseR = pIn->r_lut_base;
  const uint16_t* baseGr = pIn->gr_lut_base;
  const uint16_t* baseGb = pIn->gb_lut_base;
  const uint16_t* baseB = pIn->b_lut_base;

  const uint32_t* sR = pIn->r_lut_delta;
  const uint32_t* sGr = pIn->gr_lut_delta;
  const uint32_t* sGb = pIn->gb_lut_delta;
  const uint32_t* sB = pIn->b_lut_delta;

  /* pack the table in HW specific arrangement */
  for (i=0, j=0; i<VFE32_LINEARIZATON_TABLE_LENGTH/4; i++, j=j+4) {
    blkConfigCmd->CfgTbl.Lut[j] = ((((uint32_t)(*baseR)) & 0x00000FFF) |
      ((*sR & 0x0003FFFF) << 12));
    baseR++;
    sR++;

    blkConfigCmd->CfgTbl.Lut[j+1] = ((((uint32_t)(*baseGr)) & 0x00000FFF) |
      ((*sGr & 0x0003FFFF) << 12));
    baseGr++;
    sGr++;

    blkConfigCmd->CfgTbl.Lut[j+2] = ((((uint32_t)(*baseGb)) & 0x00000FFF) |
      ((*sGb & 0x0003FFFF) << 12));
    baseGb++;
    sGb++;

    blkConfigCmd->CfgTbl.Lut[j+3] = ((((uint32_t)(*baseB)) & 0x00000FFF) |
      ((*sB & 0x0003FFFF) << 12));
    baseB++;
    sB++;
  }
}/* vfe_write_linearization_table */

/*===========================================================================
 * FUNCTION    - vfe_config_linearization_cmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_config_linearization_cmd(
  linear_mod_t *mod,
  chromatix_linearization_type *pTbl,
  int8_t is_snapmode)
{
  int i;
  uint32_t *ptr;
  VFE_LinearizationCmdType *cmd;

  vfe_convert_linearization_tbl(pTbl,&(mod->linear_lut));

  if(is_snapmode)
    cmd = &(mod->snapshot_linear_cmd);
  else
    cmd =&( mod->video_linear_cmd);

  /* Configure the Linearization */
  cmd->CfgParams.pointSlopeR.kneePoint_P0 = mod->linear_lut.r_lut_p_l[0];
  cmd->CfgParams.pointSlopeR.kneePoint_P1 = mod->linear_lut.r_lut_p_l[1];
  cmd->CfgParams.pointSlopeR.kneePoint_P2 = mod->linear_lut.r_lut_p_l[2];
  cmd->CfgParams.pointSlopeR.kneePoint_P3 = mod->linear_lut.r_lut_p_l[3];
  cmd->CfgParams.pointSlopeR.kneePoint_P4 = mod->linear_lut.r_lut_p_l[4];
  cmd->CfgParams.pointSlopeR.kneePoint_P5 = mod->linear_lut.r_lut_p_l[5];
  cmd->CfgParams.pointSlopeR.kneePoint_P6 = mod->linear_lut.r_lut_p_l[6];
  cmd->CfgParams.pointSlopeR.kneePoint_P7 = mod->linear_lut.r_lut_p_l[7];

  cmd->CfgParams.pointSlopeGb.kneePoint_P0 = mod->linear_lut.gb_lut_p[0];
  cmd->CfgParams.pointSlopeGb.kneePoint_P1 = mod->linear_lut.gb_lut_p[1];
  cmd->CfgParams.pointSlopeGb.kneePoint_P2 = mod->linear_lut.gb_lut_p[2];
  cmd->CfgParams.pointSlopeGb.kneePoint_P3 = mod->linear_lut.gb_lut_p[3];
  cmd->CfgParams.pointSlopeGb.kneePoint_P4 = mod->linear_lut.gb_lut_p[4];
  cmd->CfgParams.pointSlopeGb.kneePoint_P5 = mod->linear_lut.gb_lut_p[5];
  cmd->CfgParams.pointSlopeGb.kneePoint_P6 = mod->linear_lut.gb_lut_p[6];
  cmd->CfgParams.pointSlopeGb.kneePoint_P7 = mod->linear_lut.gb_lut_p[7];

  cmd->CfgParams.pointSlopeB.kneePoint_P0 = mod->linear_lut.b_lut_p[0];
  cmd->CfgParams.pointSlopeB.kneePoint_P1 = mod->linear_lut.b_lut_p[1];
  cmd->CfgParams.pointSlopeB.kneePoint_P2 = mod->linear_lut.b_lut_p[2];
  cmd->CfgParams.pointSlopeB.kneePoint_P3 = mod->linear_lut.b_lut_p[3];
  cmd->CfgParams.pointSlopeB.kneePoint_P4 = mod->linear_lut.b_lut_p[4];
  cmd->CfgParams.pointSlopeB.kneePoint_P5 = mod->linear_lut.b_lut_p[5];
  cmd->CfgParams.pointSlopeB.kneePoint_P6 = mod->linear_lut.b_lut_p[6];
  cmd->CfgParams.pointSlopeB.kneePoint_P7 = mod->linear_lut.b_lut_p[7];

  cmd->CfgParams.pointSlopeGr.kneePoint_P0 = mod->linear_lut.gr_lut_p[0];
  cmd->CfgParams.pointSlopeGr.kneePoint_P1 = mod->linear_lut.gr_lut_p[1];
  cmd->CfgParams.pointSlopeGr.kneePoint_P2 = mod->linear_lut.gr_lut_p[2];
  cmd->CfgParams.pointSlopeGr.kneePoint_P3 = mod->linear_lut.gr_lut_p[3];
  cmd->CfgParams.pointSlopeGr.kneePoint_P4 = mod->linear_lut.gr_lut_p[4];
  cmd->CfgParams.pointSlopeGr.kneePoint_P5 = mod->linear_lut.gr_lut_p[5];
  cmd->CfgParams.pointSlopeGr.kneePoint_P6 = mod->linear_lut.gr_lut_p[6];
  cmd->CfgParams.pointSlopeGr.kneePoint_P7 = mod->linear_lut.gr_lut_p[7];

  vfe_write_linearization_table(&(mod->linear_lut), cmd);

  print_vfe_linearization_config(cmd);
}/* vfe_config_linearization_cmd */

/*===========================================================================
 * FUNCTION    - vfe_trigger_interpolate_linear_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_trigger_interpolate_linear_table(
  chromatix_linearization_type *input1, chromatix_linearization_type*input2,
  float ratio, chromatix_linearization_type *output)
{
  int i;

  // Interpolate the knee points (x - cooridnate)
  TBL_INTERPOLATE(input1->r_lut_p_l, input2->r_lut_p_l, output->r_lut_p_l,
    ratio, 8, i);
  TBL_INTERPOLATE(input1->gr_lut_p, input2->gr_lut_p, output->gr_lut_p,
    ratio, 8, i);
  TBL_INTERPOLATE(input1->gb_lut_p, input2->gb_lut_p, output->gb_lut_p,
    ratio, 8, i);
  TBL_INTERPOLATE(input1->b_lut_p, input2->b_lut_p, output->b_lut_p,
    ratio, 8, i);

  // Interpolate the base co-ordinates (y - cooridnate)
  TBL_INTERPOLATE(input1->r_lut_base, input2->r_lut_base, output->r_lut_base,
    ratio, 9, i);
  TBL_INTERPOLATE(input1->gr_lut_base, input2->gr_lut_base, output->gr_lut_base,
    ratio, 9, i);
  TBL_INTERPOLATE(input1->gb_lut_base, input2->gb_lut_base, output->gb_lut_base,
    ratio, 9, i);
  TBL_INTERPOLATE(input1->b_lut_base, input2->b_lut_base, output->b_lut_base,
    ratio, 9, i);

  /* Calculate Slope from knee point co-ordinates, no interpolation
     p[1]-p[0] / base[1] - base[0]
     first knee point = 0
     Last knee point and base(in normal light) = 4095
     first slope = 0
  */

  // All segment zero slopes are zero
  output->r_lut_delta[0] = 0.0;
  output->gr_lut_delta[0] = 0.0;
  output->gb_lut_delta[0] = 0.0;
  output->b_lut_delta[0] = 0.0;

  // calculate slope for segements 1 - 7
  for (i = 1 ; i < 8 ; i++) {
    output->r_lut_delta[i] = CALC_SLOPE(output->r_lut_p_l[i-1],
      output->r_lut_p_l[i], output->r_lut_base[i], output->r_lut_base[i+1]);

    output->gr_lut_delta[i] = CALC_SLOPE(output->gr_lut_p[i-1],
      output->gr_lut_p[i], output->gr_lut_base[i], output->gr_lut_base[i+1]);

    output->gb_lut_delta[i] = CALC_SLOPE(output->gb_lut_p[i-1],
      output->gb_lut_p[i], output->gb_lut_base[i], output->gb_lut_base[i+1]);

    output->b_lut_delta[i] = CALC_SLOPE(output->b_lut_p[i-1],
      output->b_lut_p[i], output->b_lut_base[i], output->b_lut_base[i+1]);
  }

  // calculate slope for  last segements, last base and knee points = 4095
  // R channel
  output->r_lut_delta[8] = CALC_SLOPE(output->r_lut_p_l[7], 4095,
    output->r_lut_base[8], 4095);
  // GR channel
  output->gr_lut_delta[8] = CALC_SLOPE(output->gr_lut_p[7], 4095,
    output->gr_lut_base[8], 4095);
  // GB channel
  output->gb_lut_delta[8] = CALC_SLOPE(output->gb_lut_p[7], 4095,
    output->gb_lut_base[8], 4095);
  // B channel
  output->b_lut_delta[8] = CALC_SLOPE(output->b_lut_p[7], 4095,
    output->b_lut_base[8], 4095);

} /* vfe_trigger_interpolate_linear_table */

/*===========================================================================
 * FUNCTION    - vfe_linear_update_lowlight_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_linear_update_lowlight_table(chromatix_linearization_type *ip,
  chromatix_linearization_type *op, float ratio, uint16_t max_blk_increase)
{
  int i = 0;
  float virtual_slope;
  uint32_t virtual_base;

  for (i = 0; i < 8; i++) {
    op->r_lut_p_l[i] = (ip->r_lut_p_l[i] - (1.0 - ratio) * max_blk_increase);
    op->gr_lut_p[i] = (ip->gr_lut_p[i] - (1.0 - ratio) * max_blk_increase);
    op->gb_lut_p[i] = (ip->gb_lut_p[i] - (1.0 - ratio) * max_blk_increase);
    op->b_lut_p[i] = (ip->b_lut_p[i] - (1.0 - ratio) * max_blk_increase);
  }

  for (i = 0 ; i < 9 ; i++) {
    op->r_lut_base[i] = ip->r_lut_base[i];
    op->gr_lut_base[i] = ip->gr_lut_base[i];
    op->gb_lut_base[i] = ip->gb_lut_base[i];
    op->b_lut_base[i] = ip->b_lut_base[i];
  }

  op->r_lut_delta[0] = 0.0;
  op->gr_lut_delta[0] = 0.0;
  op->gb_lut_delta[0] = 0.0;
  op->b_lut_delta[0] = 0.0;

  for (i = 1 ; i < 8 ; i++) {
    op->r_lut_delta[i] = ip->r_lut_delta[i];
    op->gr_lut_delta[i] = ip->gr_lut_delta[i];
    op->gb_lut_delta[i] = ip->gb_lut_delta[i];
    op->b_lut_delta[i] = ip->b_lut_delta[i];
  }

  /*
    calculate the last segment slope separately, as the curve
    shifts left, there is chance for last base point to exceed
    4095, hence re calculate last segment slope after clamping
    the last base point to 4095
  */
  // calculate the virtual base point
  virtual_base = ip->r_lut_base[8] +
      (ip->r_lut_delta[8]) * (4095 - ip->r_lut_p_l[7]);
  CDBG("R calc_base : %u\n", virtual_base);
  // clamp the virtual base to 4095
  virtual_base = MIN(4095, virtual_base);
  CDBG("R virtual_base : %u, input_slope : %f\n", virtual_base, ip->r_lut_delta[8]);
  // recalculate the new slope
  op->r_lut_delta[8] = CALC_SLOPE(op->r_lut_p_l[7], 4095,
    op->r_lut_base[8], virtual_base);
  CDBG("R virtual_base : %u, output_slope : %f\n", virtual_base, op->r_lut_delta[8]);

  // Gr channel
  virtual_base = ip->gr_lut_base[8] +
      (ip->gr_lut_delta[8]) * (4095 - ip->gr_lut_p[7]);
  CDBG("GR calc_base : %u\n", virtual_base);
  virtual_base = MIN(4095, virtual_base);
  CDBG("GR virtual_base : %u, input_slope : %f\n", virtual_base, ip->gr_lut_delta[8]);
  op->gr_lut_delta[8] = CALC_SLOPE(op->gr_lut_p[7], 4095,
    op->gr_lut_base[8], virtual_base);
  CDBG("GR virtual_base : %u, output_slope : %f\n", virtual_base, op->gr_lut_delta[8]);

  virtual_base = ip->gb_lut_base[8] +
      (ip->gb_lut_delta[8]) * (4095 - ip->gb_lut_p[7]);
  CDBG("GB calc_base : %u\n", virtual_base);
  virtual_base = MIN(4095, virtual_base);
  CDBG("GB virtual_base : %u, input_slope : %f\n", virtual_base, ip->gb_lut_delta[8]);
  op->gb_lut_delta[8] = CALC_SLOPE(op->gb_lut_p[7], 4095,
    op->gb_lut_base[8], virtual_base);
  CDBG("GB virtual_base : %u, output_slope : %f\n", virtual_base, op->gb_lut_delta[8]);

  virtual_base = ip->b_lut_base[8] +
      (ip->b_lut_delta[8]) * (4095 - ip->b_lut_p[7]);
  CDBG("B calc_base : %u\n", virtual_base);
  virtual_base = MIN(4095, virtual_base);
  CDBG("B virtual_base : %u, input_slope : %f\n", virtual_base, ip->b_lut_delta[8]);
  op->b_lut_delta[8] = CALC_SLOPE(op->b_lut_p[7], 4095,
    op->b_lut_base[8], virtual_base);
  CDBG("B virtual_base : %u, output_slope : %f\n", virtual_base, op->b_lut_delta[8]);

} /* vfe_linear_update_lowlight_table */

/*===========================================================================
 * FUNCTION    - vfe_select_linear_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_select_linear_table(linear_mod_t *mod,
  vfe_linear_lux_t lux, awb_cct_type cct, chromatix_linearization_type* output,
  vfe_params_t *vfe_params)
{
  chromatix_linearization_type output1, output2;
  chromatix_parms_type *pchromatix = vfe_params->chroma3a;
  trigger_ratio_t aec_rt;
  float awb_ratio = 0.0;
  uint16_t max_blk_increase;
  trigger_point_type blk_lowlight_trigger;

  aec_rt.ratio = 0.0;
  aec_rt.lighting = TRIGGER_NORMAL;

  if (IS_SNAP_MODE(vfe_params)) {
    max_blk_increase = pchromatix->max_blk_increase_snapshot;
    blk_lowlight_trigger = pchromatix->blk_snapshot_lowlight_trigger;
  }
  else {
    max_blk_increase = pchromatix->max_blk_increase;
    blk_lowlight_trigger = pchromatix->blk_lowlight_trigger;
  }

  aec_rt = vfe_util_get_aec_ratio2(pchromatix->control_linearization,
    &(pchromatix->linearization_bright_trigger), &blk_lowlight_trigger,
    vfe_params);

  if (cct == AWB_CCT_TYPE_TL84_A)
    awb_ratio = GET_INTERPOLATION_RATIO(mod->trigger_info.mired_color_temp,
      mod->trigger_info.trigger_A.mired_start, mod->trigger_info.trigger_A.mired_end);
  else if (cct == AWB_CCT_TYPE_D65_TL84)
    awb_ratio = GET_INTERPOLATION_RATIO(mod->trigger_info.mired_color_temp,
      mod->trigger_info.trigger_d65.mired_end, mod->trigger_info.trigger_d65.mired_start);

  CDBG("%s: aec_ratio : %f :: awb_ratio : %f\n",__func__, aec_rt.ratio, awb_ratio);
  switch (lux) {
  /* Bright */
  case LINEAR_AEC_BRIGHT:
    switch(cct) {
    case AWB_CCT_TYPE_A:
      *output = pchromatix->linear_table_A_bright;
      break;
    case AWB_CCT_TYPE_D65:
      *output = pchromatix->linear_table_Day_bright;
      break;
    case AWB_CCT_TYPE_TL84_A:
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_TL84_bright),
        &(pchromatix->linear_table_A_bright), awb_ratio, output);
      break;
    case AWB_CCT_TYPE_D65_TL84:
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_Day_bright),
        &(pchromatix->linear_table_TL84_bright), awb_ratio, output);
      break;
    case AWB_CCT_TYPE_TL84:
    default:
      *output = pchromatix->linear_table_TL84_bright;
      break;
    }
  break;
  /* b/n Bright and Normal */
  case LINEAR_AEC_BRIGHT_NORMAL:
    switch(cct) {
    case AWB_CCT_TYPE_A:
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_A_normal),
        &(pchromatix->linear_table_A_bright), aec_rt.ratio, output);
      break;
    case AWB_CCT_TYPE_D65:
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_Day_normal),
        &(pchromatix->linear_table_Day_bright), aec_rt.ratio, output);
      break;
    case AWB_CCT_TYPE_TL84_A:
      /* Bilinear Interpolation */
      // Interpolate A bright and TL84 bright
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_TL84_bright),
        &(pchromatix->linear_table_A_bright), awb_ratio, &output1);
      // Interpolate A normal and TL84 normal
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_TL84_normal),
        &(pchromatix->linear_table_A_normal), awb_ratio, &output2);
      // Interpolate b/n the o/p1 and o/p2
      vfe_trigger_interpolate_linear_table(&(output2), &(output1),
        aec_rt.ratio, output);
      break;
    case AWB_CCT_TYPE_D65_TL84:
      // Interpolate TL84 bright and D65 bright
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_Day_bright),
        &(pchromatix->linear_table_TL84_bright), awb_ratio, &output1);
      // Interpolate TL84 normal and D65 normal
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_Day_normal),
        &(pchromatix->linear_table_TL84_normal), awb_ratio, &output2);
      // Interpolate
      vfe_trigger_interpolate_linear_table(&(output2), &(output1),
        aec_rt.ratio, output);
      break;
    case AWB_CCT_TYPE_TL84:
    default:
      vfe_trigger_interpolate_linear_table(
        &(pchromatix->linear_table_TL84_normal),
        &(pchromatix->linear_table_TL84_bright), aec_rt.ratio, output);
      break;
    }
  break;
    case LINEAR_AEC_NORMAL:
      switch(cct) {
      case AWB_CCT_TYPE_A:
        vfe_linear_update_lowlight_table(
          &(pchromatix->linear_table_A_normal), output, aec_rt.ratio,
          max_blk_increase);
        break;
      case AWB_CCT_TYPE_D65:
        vfe_linear_update_lowlight_table(
          &(pchromatix->linear_table_Day_normal), output, aec_rt.ratio,
          max_blk_increase);
          break;
      case AWB_CCT_TYPE_TL84_A:
        /* Bilinear Interpolation */
        // Interpolate TL84 normal and A normal
        vfe_trigger_interpolate_linear_table(
          &(pchromatix->linear_table_TL84_normal),
          &(pchromatix->linear_table_A_normal), awb_ratio, &output1);
        vfe_linear_update_lowlight_table(
          &(output1), output, aec_rt.ratio, max_blk_increase);
        break;
      case AWB_CCT_TYPE_D65_TL84:
        /* Bilinear Interpolation */
        // Interpolate A bright and TL84 bright
        vfe_trigger_interpolate_linear_table(
          &(pchromatix->linear_table_Day_normal),
          &(pchromatix->linear_table_TL84_normal), awb_ratio, &output1);
        vfe_linear_update_lowlight_table(&(output1), output,
          aec_rt.ratio, max_blk_increase);
        break;
      case AWB_CCT_TYPE_TL84:
      default:
        vfe_linear_update_lowlight_table(
          &(pchromatix->linear_table_TL84_normal), output, aec_rt.ratio,
          max_blk_increase);
        break;
    }
    mod->blk_inc_comp = (1.0 - aec_rt.ratio) * max_blk_increase;
  break;
    default:
      CDBG_HIGH("%s: Not Expected: lux : %d :: CCT : %d", __func__, lux, cct);
      break;
  }
} /* vfe_select_linear_table */

/*===========================================================================
 * FUNCTION    - vfe_linearization_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_linearization_init(int module_id, void *mod, void *vparams)
{
  linear_mod_t *linear_mod = (linear_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  CDBG("%s\n",__func__);
  vfe_config_linearization_cmd(linear_mod,
    &(vfe_params->chroma3a->linear_table_TL84_normal), 0);
#if 1
  /* ToDo: Snapshot pipeline is not ready which triggers the Linearization update.
   *       So till it is ready. Hard-code snapshot table to use TL84.
   *       Remove this once available. */
  vfe_config_linearization_cmd(linear_mod,
    &(vfe_params->chroma3a->linear_table_TL84_normal), 1);
#endif
  linear_mod->linear_trigger = TRUE;
  linear_mod->linear_update = FALSE;
  linear_mod->hw_enable = FALSE;
  linear_mod->prev_cct_type = AWB_CCT_TYPE_TL84;
  linear_mod->prev_lux = LINEAR_AEC_NORMAL;
  linear_mod->prev_mode = VFE_OP_MODE_INVALID;

  return VFE_SUCCESS;
}/* vfe_linearization_init */

/*===========================================================================
 * FUNCTION    - vfe_linearization_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_linearization_config(int module_id, void *mod, void *vparams)
{
  linear_mod_t *linear_mod = (linear_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  VFE_LinearizationCmdType *linear_cmd;

  if (!linear_mod->linear_enable) {
    CDBG("%s: linearization not enabled", __func__);
    return VFE_SUCCESS;
  }

  if(IS_SNAP_MODE(vfe_params)) {
    linear_cmd = &(linear_mod->snapshot_linear_cmd);
    CDBG("%s: Sending Snapshot Command \n",__func__);
  } else {
    linear_cmd = &(linear_mod->video_linear_cmd);
    CDBG("%s: Sending Video/Preview/ZSL Command\n",__func__);
  }

  CDBG("%s: mode: %d\n", __func__, vfe_params->vfe_op_mode);

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)linear_cmd, sizeof(*linear_cmd), VFE_CMD_LINEARIZATION_CFG)) {
    CDBG_HIGH("%s: linearization config for op mode = %d failed\n", __func__,
      vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  return VFE_SUCCESS;
} /* vfe_linearization_config */

/*===========================================================================
 * FUNCTION    - vfe_linearization_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_linearization_update(int module_id, void *mod, void *vparams)
{
  vfe_status_t status;
  linear_mod_t *linear_mod = (linear_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  VFE_LinearizationCmdType *linear_cmd;

  if (linear_mod->hw_enable) {
    CDBG("%s: Update hardware", __func__);
    status = vfe_util_write_hw_cmd(vfe_params->camfd,
      CMD_GENERAL, vfe_params->moduleCfg,
      sizeof(VFE_ModuleCfgPacked),
      VFE_CMD_MODULE_CFG);
    if (status != VFE_SUCCESS) {
      CDBG_ERROR("%s: VFE_CMD_MODULE_CFG failed", __func__);
      return status;
    }
    vfe_params->update |= VFE_MOD_LINEARIZATION;
    linear_mod->hw_enable = FALSE;
  }
  if (!linear_mod->linear_enable) {
    CDBG("%s: linearization not enabled", __func__);
    return VFE_SUCCESS;
  }

  if(IS_SNAP_MODE(vfe_params))
    linear_cmd = &(linear_mod->snapshot_linear_cmd);
  else
    linear_cmd = &(linear_mod->video_linear_cmd);

  if(linear_mod->linear_update) {
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *)linear_cmd, sizeof(*linear_cmd), VFE_CMD_LINEARIZATION_UPDATE)) {
       CDBG_HIGH("%s: linearization update for op mode = %d failed\n", __func__,
         vfe_params->vfe_op_mode);
         return VFE_ERROR_GENERAL;
    }
    vfe_params->update |= VFE_MOD_LINEARIZATION;
    linear_mod->linear_update= FALSE;
  } else
      CDBG("%s: not updated\n", __func__);
  return VFE_SUCCESS;
} /* vfe_linearization_update */

/*===========================================================================
 * FUNCTION    - vfe_linearization_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_linearization_trigger_update(int module_id, void *mod, void *vparams)
{
  vfe_status_t status;
  linear_mod_t *linear_mod = (linear_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  chromatix_parms_type *chroma3a = vfe_params->chroma3a;
  long bl_lux_index_end, bl_lux_index_start;
  long ll_lux_index_end, ll_lux_index_start;
  vfe_linear_lux_t lux = LINEAR_AEC_NORMAL;
  awb_cct_type cct_type;
  chromatix_linearization_type output;
  uint8_t update_linear = FALSE;

  if (!linear_mod->linear_enable) {
    CDBG("%s: linearization not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!linear_mod->linear_trigger) {
    CDBG("%s: linearization tigger not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!vfe_util_aec_check_settled(&vfe_params->aec_params)) {
    CDBG("%s: AEC not settled", __func__);
    return VFE_SUCCESS;
  }

  linear_mod->trigger_info.mired_color_temp =
    MIRED(vfe_params->awb_params.color_temp);
  CALC_CCT_TRIGGER_MIRED(linear_mod->trigger_info.trigger_A,
    chroma3a->linear_A_trigger);
  CALC_CCT_TRIGGER_MIRED(linear_mod->trigger_info.trigger_d65,
    chroma3a->linear_D65_trigger);

  // get the cct type
  cct_type = vfe_util_get_awb_cct_type(&linear_mod->trigger_info,vfe_params);

  // get the bright light trigger points
  bl_lux_index_end = chroma3a->linearization_bright_trigger.lux_index_end;
  bl_lux_index_start = chroma3a->linearization_bright_trigger.lux_index_start;

  // get the low light trigger points
  if (IS_SNAP_MODE(vfe_params)) {
    ll_lux_index_end = chroma3a->blk_snapshot_lowlight_trigger.lux_index_end;
    ll_lux_index_start =
      chroma3a->blk_snapshot_lowlight_trigger.lux_index_start;
  } else {
    ll_lux_index_end = chroma3a->blk_lowlight_trigger.lux_index_end;
    ll_lux_index_start = chroma3a->blk_lowlight_trigger.lux_index_start;
  }

  /*============================================*/
  /* AEC :Decision */
  /*============================================*/
  float lux_idx = vfe_params->aec_params.lux_idx;
  if (lux_idx <= bl_lux_index_end)
    lux = LINEAR_AEC_BRIGHT;
  else if (lux_idx > bl_lux_index_end  &&  lux_idx <= bl_lux_index_start)
    lux = LINEAR_AEC_BRIGHT_NORMAL;
  else if (lux_idx > bl_lux_index_start)
    lux = LINEAR_AEC_NORMAL;
  else
    CDBG_HIGH("%s: Lux index is invalid\n", __func__);

  /* check for trigger updation */
  update_linear = ((linear_mod->prev_mode != vfe_params->vfe_op_mode) ||
    !F_EQUAL(linear_mod->prev_lux, lux) || !F_EQUAL(linear_mod->prev_cct_type, cct_type));

  if (update_linear) {
    vfe_select_linear_table(linear_mod, lux, cct_type, &output,
      vfe_params);
    int8_t is_snapmode = IS_SNAP_MODE(vfe_params);
    vfe_config_linearization_cmd(linear_mod, &output, is_snapmode);
    linear_mod->linear_update= TRUE;
    CDBG("%s: color temp %d", __func__, vfe_params->awb_params.color_temp);
    CDBG("%s: lux index %f", __func__, vfe_params->aec_params.lux_idx);
    CDBG("%s: AEC type prev %s new %s", __func__, aec_debug_str[linear_mod->prev_lux],
      aec_debug_str[lux]);
    CDBG("%s: AWB type prev %s new %s", __func__, awb_debug_str[linear_mod->prev_cct_type],
      awb_debug_str[cct_type]);
    linear_mod->prev_mode = vfe_params->vfe_op_mode;
    linear_mod->prev_lux = lux;
    linear_mod->prev_cct_type = cct_type;
  }
  return VFE_SUCCESS;
} /* vfe_linearization_trigger_update */

/*===========================================================================
 * Function:           vfe_linearization_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_linearization_enable(int module_id, void *mod, void *params,
  int8_t enable, int8_t hw_write)
{
  linear_mod_t *linear_mod = (linear_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;
  vfe_params->moduleCfg->blackLevelCorrectionEnable = enable;

  if (hw_write && (linear_mod->linear_enable == enable))
    return VFE_SUCCESS;

  linear_mod->linear_enable = enable;
  linear_mod->hw_enable = hw_write;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_LINEARIZATION)
      : (vfe_params->current_config & ~VFE_MOD_LINEARIZATION);
  }
  return VFE_SUCCESS;
} /* vfe_linearization_enable */
/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_trigger_enable -
 *
 * DESCRIPTION: This function updates the mce trigger enable flag
 *==========================================================================*/
vfe_status_t vfe_linearization_trigger_enable(int module_id, void* mod, void* vparams,
  int enable)
{
  linear_mod_t *linear_mod = (linear_mod_t *)mod;
  CDBG("%s:enable :%d\n",__func__, enable);
  linear_mod->linear_trigger = enable;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_linearization_tv_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
vfe_status_t vfe_linearization_tv_validate(int module_id, void* input, void* output)
{
  uint8_t i;
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;
  uint32_t ip, op;
  VFE_LinearizationCmdType *in, *out;
  VFE_PointSlopeData *ins, *outs;

  CDBG("%s:\n", __func__);
  in = (VFE_LinearizationCmdType *)(mod_in->reg_dump + (V32_LINEAR_OFF1/4) );
  out = (VFE_LinearizationCmdType *)(mod_op->reg_dump_data + (V32_LINEAR_OFF1/4));
  ins = &(in->CfgParams.pointSlopeR);
  outs = &(out->CfgParams.pointSlopeR);

  //R Channel
  VALIDATE_TST_VEC(ins->kneePoint_P0, outs->kneePoint_P0, 0, "R Channel.kneePoint_P0");
  VALIDATE_TST_VEC(ins->kneePoint_P1, outs->kneePoint_P1, 0, "R Channel.kneePoint_P1");
  VALIDATE_TST_VEC(ins->kneePoint_P2, outs->kneePoint_P2, 0, "R Channel.kneePoint_P2");
  VALIDATE_TST_VEC(ins->kneePoint_P3, outs->kneePoint_P3, 0, "R Channel.kneePoint_P3");
  VALIDATE_TST_VEC(ins->kneePoint_P4, outs->kneePoint_P4, 0, "R Channel.kneePoint_P4");
  VALIDATE_TST_VEC(ins->kneePoint_P5, outs->kneePoint_P5, 0, "R Channel.kneePoint_P5");

  in = (VFE_LinearizationCmdType *)(mod_in->reg_dump + (V32_LINEAR_OFF2/4) - 4);
  out = (VFE_LinearizationCmdType *)(mod_op->reg_dump_data + (V32_LINEAR_OFF2/4) - 4);

  ins = &(in->CfgParams.pointSlopeR);
  outs = &(out->CfgParams.pointSlopeR);

  VALIDATE_TST_VEC(ins->kneePoint_P6, outs->kneePoint_P6, 0, "R Channel.kneePoint_P6");
  VALIDATE_TST_VEC(ins->kneePoint_P7, outs->kneePoint_P7, 0, "R Channel.kneePoint_P7");

  ins++;
  outs++;
  //GB Channel
  VALIDATE_TST_VEC(ins->kneePoint_P0, outs->kneePoint_P0, 0, "GB Channel.kneePoint_P0");
  VALIDATE_TST_VEC(ins->kneePoint_P1, outs->kneePoint_P1, 0, "GB Channel.kneePoint_P1");
  VALIDATE_TST_VEC(ins->kneePoint_P2, outs->kneePoint_P2, 0, "GB Channel.kneePoint_P2");
  VALIDATE_TST_VEC(ins->kneePoint_P3, outs->kneePoint_P3, 0, "GB Channel.kneePoint_P3");
  VALIDATE_TST_VEC(ins->kneePoint_P4, outs->kneePoint_P4, 0, "GB Channel.kneePoint_P4");
  VALIDATE_TST_VEC(ins->kneePoint_P5, outs->kneePoint_P5, 0, "GB Channel.kneePoint_P5");
  VALIDATE_TST_VEC(ins->kneePoint_P6, outs->kneePoint_P6, 0, "GB Channel.kneePoint_P6");
  VALIDATE_TST_VEC(ins->kneePoint_P7, outs->kneePoint_P7, 0, "GB Channel.kneePoint_P7");

  ins++;
  outs++;
  //B Channel
  VALIDATE_TST_VEC(ins->kneePoint_P0, outs->kneePoint_P0, 0, "B Channel.kneePoint_P0");
  VALIDATE_TST_VEC(ins->kneePoint_P1, outs->kneePoint_P1, 0, "B Channel.kneePoint_P1");
  VALIDATE_TST_VEC(ins->kneePoint_P2, outs->kneePoint_P2, 0, "B Channel.kneePoint_P2");
  VALIDATE_TST_VEC(ins->kneePoint_P3, outs->kneePoint_P3, 0, "B Channel.kneePoint_P3");
  VALIDATE_TST_VEC(ins->kneePoint_P4, outs->kneePoint_P4, 0, "B Channel.kneePoint_P4");
  VALIDATE_TST_VEC(ins->kneePoint_P5, outs->kneePoint_P5, 0, "B Channel.kneePoint_P5");
  VALIDATE_TST_VEC(ins->kneePoint_P6, outs->kneePoint_P6, 0, "B Channel.kneePoint_P6");
  VALIDATE_TST_VEC(ins->kneePoint_P7, outs->kneePoint_P7, 0, "B Channel.kneePoint_P7");

  ins++;
  outs++;
  //GR Channel
  VALIDATE_TST_VEC(ins->kneePoint_P0, outs->kneePoint_P0, 0, "GR Channel.kneePoint_P0");
  VALIDATE_TST_VEC(ins->kneePoint_P1, outs->kneePoint_P1, 0, "GR Channel.kneePoint_P1");
  VALIDATE_TST_VEC(ins->kneePoint_P2, outs->kneePoint_P2, 0, "GR Channel.kneePoint_P2");
  VALIDATE_TST_VEC(ins->kneePoint_P3, outs->kneePoint_P3, 0, "GR Channel.kneePoint_P3");
  VALIDATE_TST_VEC(ins->kneePoint_P4, outs->kneePoint_P4, 0, "GR Channel.kneePoint_P4");
  VALIDATE_TST_VEC(ins->kneePoint_P5, outs->kneePoint_P5, 0, "GR Channel.kneePoint_P5");
  VALIDATE_TST_VEC(ins->kneePoint_P6, outs->kneePoint_P6, 0, "GR Channel.kneePoint_P6");
  VALIDATE_TST_VEC(ins->kneePoint_P7, outs->kneePoint_P7, 0, "GR Channel.kneePoint_P7");
  // Compare Tables

  for(i = 0; i < mod_in->linearization.size; i++) {
    ip = mod_in->linearization.table[i] & 0x3ffff000;
    op = mod_op->linearization_table[i] & 0x3ffff000;
    ip >>= 12;
    op >>= 12;
    VALIDATE_TST_LUT(ip, op, 3, "slope", i);
    ip = mod_in->linearization.table[i] & 0x00000fff;
    op = mod_op->linearization_table[i] & 0x00000fff;
    VALIDATE_TST_LUT(ip, op, 3, "base", i);
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_linearization_reload_params -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_linearization_reload_params(int module_id, void *mod,
  void* vfe_parms)
{
  CDBG_ERROR("%s: Not implemented\n", __func__);
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_linearization_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_linearization_deinit(int mod_id, void *module, void *params)
{
  linear_mod_t *linear_mod = (linear_mod_t *)module;
  memset(linear_mod, 0 , sizeof(linear_mod_t));
  return VFE_SUCCESS;
}
#ifndef VFE_40
/*===========================================================================
 * FUNCTION    - vfe_linearization_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_linearization_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  linear_module_t *cmd = (linear_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *)&(cmd->linear_cmd), sizeof(VFE_LinearizationCmdType),
    VFE_CMD_LINEARIZATION_UPDATE)) {
      CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
        vfe_params->vfe_op_mode);
        return VFE_ERROR_GENERAL;
  }

  return VFE_SUCCESS;
} /* vfe_linearization_update */
#endif
