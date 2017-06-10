/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <string.h>
#include <stdlib.h>
#include "camera_dbg.h"
#include <inttypes.h>
#include "pca_rolloff32.h"
#include "../mlro_to_plro/mlro.h"
#include "isp_log.h"

#define PCA_ROLLOFF_TABLE_DEBUG 0
#define PCA_ROLLOFF_TINTLESS_CONVERT_TABLE_SIZE 1
#define TINTLESS_TEMPORAL_RATIO 0.4
#define USE_FIXED_TAB 0
#if USE_FIXED_TAB
static float fixed_rolloff[4][13][8] = {
{ {3.4714,   0.0426,   0.1007,   0.0118,  -0.0201,   0.0076,  -0.0032,  -0.0080},
  {3.0947,  -0.0093,   0.0955,   0.0313,   0.0075,   0.0004,   0.0005,  -0.0046},
  {2.7984,  -0.0442,   0.0671,   0.0092,   0.0165,   0.0020,   0.0027,  -0.0023},
  {2.5954,  -0.0920,   0.0474,  -0.0075,   0.0114,   0.0098,   0.0011,   0.0047},
  {2.4285,  -0.1273,   0.0290,  -0.0108,   0.0029,   0.0034,  -0.0006,   0.0056},
  {2.3278,  -0.1432,   0.0191,  -0.0091,  -0.0087,   0.0009,  -0.0072,   0.0013},
  {2.3192,  -0.1509,   0.0147,  -0.0063,  -0.0149,  -0.0028,  -0.0065,   0.0054},
  {2.3862,  -0.1452,   0.0144,  -0.0098,  -0.0108,  -0.0072,  -0.0048,   0.0036},
  {2.4931,  -0.1336,   0.0184,  -0.0033,   0.0033,  -0.0066,  -0.0014,   0.0048},
  {2.6560,  -0.0985,   0.0293,   0.0026,   0.0094,  -0.0051,  -0.0002,   0.0101},
  {2.9191,  -0.0608,   0.0457,   0.0196,   0.0154,  -0.0033,  -0.0016,  -0.0010},
  {3.3066,  -0.0201,   0.0680,   0.0377,   0.0155,   0.0005,  -0.0085,   0.0062},
  {3.6942,   0.0543,   0.0809,   0.0083,  -0.0272,  -0.0046,   0.0012,   0.0042}},
{ {3.5319,   0.0961,  -0.0358,   0.0267,  -0.0243,  -0.0001,   0.0033,  -0.0022},
  {3.1748,   0.0346,  -0.0369,   0.0480,   0.0090,   0.0018,   0.0024,   0.0002},
  {2.8720,  -0.0178,  -0.0415,   0.0176,   0.0144,  -0.0027,   0.0039,  -0.0048},
  {2.6607,  -0.0796,  -0.0411,  -0.0041,   0.0072,  -0.0084,   0.0027,  -0.0086},
  {2.4943,  -0.1295,  -0.0474,  -0.0059,   0.0018,   0.0015,   0.0016,  -0.0039},
  {2.3830,  -0.1464,  -0.0445,  -0.0074,  -0.0082,   0.0026,   0.0013,  -0.0026},
  {2.3697,  -0.1534,  -0.0412,  -0.0076,  -0.0144,   0.0009,  -0.0031,  -0.0079},
  {2.4344,  -0.1450,  -0.0463,  -0.0039,  -0.0097,   0.0053,  -0.0003,  -0.0042},
  {2.5482,  -0.1205,  -0.0407,  -0.0068,  -0.0018,  -0.0020,   0.0045,  -0.0094},
  {2.7259,  -0.0736,  -0.0409,   0.0089,   0.0131,  -0.0001,   0.0013,  -0.0109},
  {2.9969,  -0.0186,  -0.0311,   0.0314,   0.0172,  -0.0020,   0.0010,  -0.0023},
  {3.3819,   0.0365,  -0.0242,   0.0589,   0.0012,  -0.0031,  -0.0022,   0.0023},
  {3.7626,   0.1098,  -0.0192,   0.0261,  -0.0272,  -0.0135,   0.0046,  -0.0002}},
{ {3.5255,   0.0928,   0.0801,   0.0157,  -0.0121,  -0.0037,  -0.0070,  -0.0039},
  {3.1649,   0.0312,   0.0754,   0.0285,   0.0163,  -0.0073,  -0.0103,  -0.0050},
  {2.8707,  -0.0224,   0.0574,   0.0073,   0.0241,  -0.0082,   0.0009,  -0.0017},
  {2.6546,  -0.0820,   0.0436,  -0.0171,   0.0174,  -0.0069,  -0.0010,  -0.0081},
  {2.4855,  -0.1295,   0.0369,  -0.0284,   0.0056,  -0.0101,   0.0019,   0.0016},
  {2.3826,  -0.1561,   0.0273,  -0.0267,  -0.0099,  -0.0042,  -0.0006,  -0.0036},
  {2.3647,  -0.1653,   0.0291,  -0.0301,  -0.0102,  -0.0076,  -0.0050,  -0.0008},
  {2.4295,  -0.1660,   0.0268,  -0.0279,  -0.0061,  -0.0043,  -0.0003,   0.0018},
  {2.5399,  -0.1520,   0.0367,  -0.0270,   0.0015,  -0.0071,   0.0046,  -0.0008},
  {2.7166,  -0.1104,   0.0511,  -0.0170,   0.0120,  -0.0050,   0.0046,  -0.0031},
  {2.9842,  -0.0563,   0.0692,   0.0019,   0.0195,  -0.0050,   0.0012,  -0.0005},
  {3.3694,  -0.0022,   0.0946,   0.0252,   0.0135,  -0.0073,  -0.0013,  -0.0019},
  {3.7499,   0.0708,   0.1029,  -0.0135,  -0.0275,  -0.0022,   0.0068,   0.0049}},
{ {3.2856,   0.1237,  -0.0410,   0.0336,  -0.0244,  -0.0085,   0.0004,   0.0037},
  {2.9511,   0.0744,  -0.0312,   0.0429,   0.0038,  -0.0086,  -0.0056,  -0.0014},
  {2.6769,   0.0350,  -0.0213,   0.0249,   0.0122,  -0.0121,   0.0041,   0.0098},
  {2.5105,  -0.0187,  -0.0130,   0.0123,   0.0191,  -0.0043,  -0.0035,   0.0074},
  {2.3736,  -0.0545,  -0.0137,  -0.0017,   0.0081,  -0.0042,  -0.0042,   0.0010},
  {2.2686,  -0.0671,  -0.0071,  -0.0089,  -0.0015,  -0.0039,  -0.0015,   0.0088},
  {2.2458,  -0.0850,  -0.0049,  -0.0041,  -0.0061,   0.0048,  -0.0025,   0.0195},
  {2.3120,  -0.0898,  -0.0029,  -0.0070,  -0.0024,  -0.0066,  -0.0027,  -0.0025},
  {2.4202,  -0.0807,  -0.0047,  -0.0036,   0.0006,   0.0003,   0.0041,   0.0045},
  {2.5635,  -0.0512,   0.0093,   0.0027,   0.0103,  -0.0097,   0.0037,   0.0025},
  {2.7760,  -0.0116,   0.0116,   0.0197,   0.0165,  -0.0022,   0.0012,   0.0054},
  {3.1165,   0.0233,   0.0225,   0.0316,  -0.0010,  -0.0053,   0.0042,   0.0090},
  {3.4782,   0.0769,   0.0150,   0.0045,  -0.0388,   0.0001,   0.0159,   0.0043}}
};
#endif

#define MESH_MATRIX_DIFF(M1,M2) ({ \
  int i,j; \
  for(i=0; i<13; i++) { \
      CDBG_ERROR("%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ", \
        M1[i*17]-M2[i][0], M1[i*17+1]-M2[i][1], M1[i*17+2]-M2[i][2], M1[i*17+3]-M2[i][3], \
        M1[i*17+4]-M2[i][4], M1[i*17+5]-M2[i][5], M1[i*17+6]-M2[i][6], M1[i*17+7]-M2[i][7], \
        M1[i*17+8]-M2[i][8], M1[i*17+9]-M2[i][9], M1[i*17+10]-M2[i][10], M1[i*17+11]-M2[i][11], \
        M1[i*17+12]-M2[i][12], M1[i*17+13]-M2[i][13], M1[i*17+14]-M2[i][14], M1[i*17+15]-M2[i][15], \
        M1[i*17+16]-M2[i][16]); \
  } \
})

#define CONV_STATIC_TAB(X, tab_i) ({ \
  int i = 0, j = 0; \
  for (i=0; i<13; i++) { \
    for (j=0; j<8; j++) { \
      X[i][j] = fixed_rolloff[tab_i][i][j]; \
    } \
  } \
})

#ifdef ENABLE_PCA_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#define PCA_TBL_INTERPOLATE(in1, in2, out, ratio, isize, i, jsize, j) \
({ \
  for (i=0; i<isize; i++) \
    for (j=0; j<jsize; j++) \
      out[i][j] = LINEAR_INTERPOLATION(in1[i][j], in2[i][j], ratio); \
})

/*===========================================================================
 * FUNCTION    - rolloff_normalize_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void rolloff_normalize_table(isp_pca_rolloff_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, int is_left_table)
{
  int i, j;
  float min_value = 1.0, scaling_val;
  chromatix_VFE_common_type *chrComPtr = NULL;
  mesh_rolloff_array_type *inTbl = NULL;
  mesh_rolloff_array_type *outTbl = NULL;
  mesh_rolloff_array_type *deltaTbl = NULL;

  chrComPtr = in_params->chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *rolloffPtr = &chrComPtr->chromatix_rolloff;

  for (i = ISP_ROLLOFF_TL84_LIGHT; i < ISP_ROLLOFF_MAX_LIGHT; i++) {
    for (j = 0; j < MESH_ROLLOFF_SIZE; j++) {
      if (is_left_table) {
        outTbl = &(mod->rolloff_tbls.left[i]);
        deltaTbl = &(mod->rolloff_calibration_table.left[i]);
      } else {
        outTbl = &(mod->rolloff_tbls.right[i]);
        deltaTbl = &(mod->rolloff_calibration_table.right[i]);
      }

      if (i == ISP_ROLLOFF_LED_FLASH) {
        inTbl = &(rolloffPtr->chromatix_mesh_rolloff_table_LED);
      } else if (i == ISP_ROLLOFF_STROBE_FLASH) {
        inTbl = &(rolloffPtr->chromatix_mesh_rolloff_table_Strobe);
      } else if (i == ISP_ROLLOFF_TL84_LOW_LIGHT) {
        inTbl = &(rolloffPtr->chromatix_mesh_rolloff_table_lowlight[ROLLOFF_TL84_LIGHT]);
      } else if (i == ISP_ROLLOFF_A_LOW_LIGHT) {
        inTbl = &(rolloffPtr->chromatix_mesh_rolloff_table_lowlight[ROLLOFF_A_LIGHT]);
      } else if (i == ISP_ROLLOFF_D65_LOW_LIGHT) {
        inTbl = &(rolloffPtr->chromatix_mesh_rolloff_table_lowlight[ROLLOFF_D65_LIGHT]);
      } else {
        inTbl = &(rolloffPtr->chromatix_mesh_rolloff_table[i]);
      }
      /* RED Channel */
      outTbl->r_gain[j] = inTbl->r_gain[j] * deltaTbl->r_gain[j];
      if (outTbl->r_gain[j] < min_value)
        min_value = outTbl->r_gain[j];
      /* GR Channel */
      outTbl->gr_gain[j] = inTbl->gr_gain[j] * deltaTbl->gr_gain[j];
      if (outTbl->gr_gain[j] < min_value)
        min_value = outTbl->gr_gain[j];
      /* BLUE Channel */
      outTbl->b_gain[j] = inTbl->b_gain[j] * deltaTbl->b_gain[j];
      if (outTbl->b_gain[j] < min_value)
        min_value = outTbl->b_gain[j];
      /* GB Channel */
      outTbl->gb_gain[j] = inTbl->gb_gain[j] * deltaTbl->gb_gain[j];
      if (outTbl->gb_gain[j] < min_value)
        min_value = outTbl->gb_gain[j];
    }
    if (min_value >= 1.0)
      continue;

    scaling_val = 1.0 / min_value;

    for (j = 0; j < MESH_ROLLOFF_SIZE; j++) {
      /* RED Channel */
      outTbl->r_gain[j]  *= scaling_val;
      /* GR Channel */
      outTbl->gr_gain[j] *= scaling_val;
      /* BLUE Channel */
      outTbl->b_gain[j]  *= scaling_val;
      /* GB Channel */
      outTbl->gb_gain[j] *= scaling_val;
    }
    min_value = 1.0;
  }
} /* rolloff_normalize_table */

/*===========================================================================
 * FUNCTION    - rolloff_prepare_tables -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int rolloff_tintless_prepare_tables(isp_pca_rolloff_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, isp_tintless_mesh_config_t *isp_mesh_cfg)
{
  int i, k;
  tintless_mesh_rolloff_array_t *low_light_table;
  chromatix_VFE_common_type *chrComPtr = NULL;
  chrComPtr = in_params->chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *chromatix_rolloff =
    &chrComPtr->chromatix_rolloff;

  /* Todo: Remove this once Sensor Provides Delta Tables */
    for (k = 0; k < ISP_ROLLOFF_MAX_LIGHT; k++) {
      for (i = 0; i < 221; i++) {
          mod->rolloff_calibration_table.left[k].r_gain[i] = 1.0;
          mod->rolloff_calibration_table.left[k].b_gain[i] = 1.0;
          mod->rolloff_calibration_table.left[k].gr_gain[i] = 1.0;
          mod->rolloff_calibration_table.left[k].gb_gain[i] = 1.0;
      }
  }
  /* Left frame tables */
  rolloff_normalize_table(mod, in_params, TRUE);
  if (sizeof(tintless_mesh_rolloff_array_t) == sizeof(mesh_rolloff_array_type))
    memcpy(&isp_mesh_cfg->mesh_fixed, &mod->rolloff_tbls.left[ISP_ROLLOFF_TL84_LIGHT], sizeof(mesh_rolloff_array_type));
  else {
    CDBG_ERROR("%s: Error in copying", __func__);
    return -1;
  }

  low_light_table = &chromatix_rolloff->chromatix_mesh_rolloff_table_lowlight[ROLLOFF_TL84_LIGHT];

  for (i = 0; i < TINTLESS_ROLLOFF_TABLE_SIZE; i++) {
     if (isp_mesh_cfg->mesh_fixed.gr_gain[i] != 0) {
       mod->tintless_lowlight_adjust[i] =
         low_light_table->gr_gain[i] / isp_mesh_cfg->mesh_fixed.gr_gain[i];
     } else {
       CDBG_ERROR("%s: normal light ratio = 0! rc = -1\n", __func__);
       return -1;
     }
  }

  for (i = 0; i < TINTLESS_ROLLOFF_TABLE_SIZE; i++) {
    mod->tintless_current_adjust[i] = 1.0;
  }

  isp_mesh_cfg->mesh_hw = isp_mesh_cfg->mesh_fixed;

  return 0;
}
/*===========================================================================
 * FUNCTION    - rolloff_prepare_tables -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int rolloff_prepare_tables(isp_pca_rolloff_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params)
{
  int i, k;

#if 1
  /* Todo: Remove this once Sensor Provides Delta Tables */
  for (k = 0; k < ISP_ROLLOFF_MAX_LIGHT; k++) {
    for (i = 0; i < 221; i++) {
      mod->rolloff_calibration_table.left[k].r_gain[i] = 1.0;
      mod->rolloff_calibration_table.left[k].b_gain[i] = 1.0;
      mod->rolloff_calibration_table.left[k].gr_gain[i] = 1.0;
      mod->rolloff_calibration_table.left[k].gb_gain[i] = 1.0;
    }
  }
#endif

  memset(&mod->rolloff_tbls, 0x0, sizeof(mod->rolloff_tbls));

  /* Left frame tables */
  rolloff_normalize_table(mod, in_params, TRUE);

  return 0;
} /* rolloff_prepare_tables */

/*==============================================================================
 * Function:           pca_rolloff_table_debug
 *
 * Description:
 *============================================================================*/
static void pca_rolloff_table_debug(PCA_RolloffStruct *in, int compare,
  mesh_rolloff_array_type *orig_mesh)
{
  int i;
  float mesh[13][17];

#if USE_FIXED_TAB
  CONV_STATIC_TAB(in->coeff_table_R, 0);
  CONV_STATIC_TAB(in->coeff_table_Gr, 1);
  CONV_STATIC_TAB(in->coeff_table_Gb, 2);
  CONV_STATIC_TAB(in->coeff_table_B, 3);
#endif

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: PCA Bases Table\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_BASE; i++)
    ISP_DBG(ISP_MOD_ROLLOFF, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      in->PCA_basis_table[i][0], in->PCA_basis_table[i][1],
      in->PCA_basis_table[i][2], in->PCA_basis_table[i][3],
      in->PCA_basis_table[i][4], in->PCA_basis_table[i][5],
      in->PCA_basis_table[i][6], in->PCA_basis_table[i][7],
      in->PCA_basis_table[i][8], in->PCA_basis_table[i][9],
      in->PCA_basis_table[i][10], in->PCA_basis_table[i][11],
      in->PCA_basis_table[i][12], in->PCA_basis_table[i][13],
      in->PCA_basis_table[i][14], in->PCA_basis_table[i][15],
      in->PCA_basis_table[i][16]);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: PCA Channel R Coeff tables\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%f %f %f %f %f %f %f %f\n",
      in->coeff_table_R[i][0], in->coeff_table_R[i][1],
      in->coeff_table_R[i][2], in->coeff_table_R[i][3],
      in->coeff_table_R[i][4], in->coeff_table_R[i][5],
      in->coeff_table_R[i][6], in->coeff_table_R[i][7]);
  }
  if (compare) {
    MATRIX_MULT(in->coeff_table_R, in->PCA_basis_table, mesh, 13, 8, 17);
    MESH_MATRIX_DIFF(orig_mesh->r_gain, mesh);
  }

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: PCA Channel GR Coeff tables\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%f %f %f %f %f %f %f %f\n",
      in->coeff_table_Gr[i][0], in->coeff_table_Gr[i][1],
      in->coeff_table_Gr[i][2], in->coeff_table_Gr[i][3],
      in->coeff_table_Gr[i][4], in->coeff_table_Gr[i][5],
      in->coeff_table_Gr[i][6], in->coeff_table_Gr[i][7]);
  }
  if (compare) {
    MATRIX_MULT(in->coeff_table_Gr, in->PCA_basis_table, mesh, 13, 8, 17);
    MESH_MATRIX_DIFF(orig_mesh->gr_gain, mesh);
  }

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: PCA Channel GB Coeff tables\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%f %f %f %f %f %f %f %f\n",
      in->coeff_table_Gb[i][0], in->coeff_table_Gb[i][1],
      in->coeff_table_Gb[i][2], in->coeff_table_Gb[i][3],
      in->coeff_table_Gb[i][4], in->coeff_table_Gb[i][5],
      in->coeff_table_Gb[i][6], in->coeff_table_Gb[i][7]);
  }
  if (compare) {
    MATRIX_MULT(in->coeff_table_Gb, in->PCA_basis_table, mesh, 13, 8, 17);
    MESH_MATRIX_DIFF(orig_mesh->gb_gain, mesh);
  }

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: PCA Channel B Coeff tables\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%f %f %f %f %f %f %f %f\n",
      in->coeff_table_B[i][0], in->coeff_table_B[i][1],
      in->coeff_table_B[i][2], in->coeff_table_B[i][3],
      in->coeff_table_B[i][4], in->coeff_table_B[i][5],
      in->coeff_table_B[i][6], in->coeff_table_B[i][7]);
  }
  if (compare) {
    MATRIX_MULT(in->coeff_table_B, in->PCA_basis_table, mesh, 13, 8, 17);
    MESH_MATRIX_DIFF(orig_mesh->b_gain, mesh);
  }
} /* pca_rolloff_table_debug */

/*==============================================================================
 * FUNCTION    - pca_rolloff_cmd_debug -
 *
 * DESCRIPTION:
 *============================================================================*/
static void pca_rolloff_cmd_debug(PCA_RollOffConfigCmdType *cmd)
{
  int i;

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: pixelOffset=0x%x, pcaLutBankSel=0x%x\n", __func__,
    cmd->CfgParams.pixelOffset, cmd->CfgParams.pcaLutBankSel);
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: xDelta=0x%x, yDelta=0x%x\n", __func__, cmd->CfgParams.xDelta,
    cmd->CfgParams.yDelta);
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: gridWidth=0x%x, gridHeight=0x%x\n", __func__,
    cmd->CfgParams.gridWidth, cmd->CfgParams.gridHeight);
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: xDeltaRight=0x%x, yDeltaRight=0x%x\n", __func__,
    cmd->CfgParams.xDeltaRight, cmd->CfgParams.yDeltaRight);
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: gridWidthRight=0x%x, gridHeightRight=0x%x\n", __func__,
    cmd->CfgParams.gridWidthRight, cmd->CfgParams.gridHeightRight);
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: gridXIndex=0x%x, gridYIndex=0x%x, gridPixelXIndex=0x%x, "
    "gridPixelYIndex=0x%x\n", __func__, cmd->CfgParams.gridXIndex,
    cmd->CfgParams.gridYIndex, cmd->CfgParams.gridPixelXIndex,
    cmd->CfgParams.gridPixelYIndex);
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: yDeltaAccum=0x%x\n", __func__, cmd->CfgParams.yDeltaAccum);

  if (PCA_ROLLOFF_TABLE_DEBUG) {
    for (i=0; i<(PCA_ROLLOFF_BASIS_TABLE_SIZE); i++) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: ram0_bases[%d]=0x%016llx\n", __func__, i,
        cmd->ram0.basisTable[i]);
    }
    for (i=0; i<(PCA_ROLLOFF_COEFF_TABLE_SIZE); i++) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: ram0_coeff[%d]=0x%016llx\n", __func__, i,
        cmd->ram0.coeffTable[i]);
    }
    for (i=0; i<(PCA_ROLLOFF_BASIS_TABLE_SIZE); i++) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: ram1_bases[%d]=0x%016llx\n", __func__, i,
        cmd->ram1.basisTable[i]);
    }
    for (i=0; i<(PCA_ROLLOFF_COEFF_TABLE_SIZE); i++) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: ram1_coeff[%d]=0x%016llx\n", __func__, i,
        cmd->ram1.coeffTable[i]);
    }
  }
} /* pca_rolloff_cmd_debug */

/*==============================================================================
 * Function:           pca_rolloff_table_interpolate
 *
 * Description:
 *============================================================================*/
static void pca_rolloff_table_interpolate(PCA_RolloffStruct *in1,
  PCA_RolloffStruct *in2, PCA_RolloffStruct *out, float ratio)
{
  int i, j;
  /* Basis table interpolation */
  PCA_TBL_INTERPOLATE(in1->PCA_basis_table, in2->PCA_basis_table,
    out->PCA_basis_table, ratio, PCA_ROLLOFF_NUMBER_BASE, i,
    PCA_ROLLOFF_NUMBER_COLS, j);

  /* Coeff table interpolation */
  PCA_TBL_INTERPOLATE(in1->coeff_table_R, in2->coeff_table_R,
    out->coeff_table_R, ratio, PCA_ROLLOFF_NUMBER_ROWS, i,
    PCA_ROLLOFF_NUMBER_BASE, j);
  PCA_TBL_INTERPOLATE(in1->coeff_table_Gr, in2->coeff_table_Gr,
    out->coeff_table_Gr, ratio, PCA_ROLLOFF_NUMBER_ROWS, i,
    PCA_ROLLOFF_NUMBER_BASE, j);
  PCA_TBL_INTERPOLATE(in1->coeff_table_B, in2->coeff_table_B,
    out->coeff_table_B, ratio, PCA_ROLLOFF_NUMBER_ROWS, i,
    PCA_ROLLOFF_NUMBER_BASE, j);
  PCA_TBL_INTERPOLATE(in1->coeff_table_Gb, in2->coeff_table_Gb,
    out->coeff_table_Gb, ratio, PCA_ROLLOFF_NUMBER_ROWS, i,
    PCA_ROLLOFF_NUMBER_BASE, j);
} /* pca_rolloff_table_interpolate */

/*===========================================================================
 * FUNCTION    - pca_rolloff_update_basis_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void pca_rolloff_update_basis_table(PCA_RolloffStruct *in,
  PCA_RollOffBasisData *out)
{
  uint32_t col;
  for (col = 0; col < PCA_ROLLOFF_NUMBER_COLS; col++) {
    out->v0[col] = FLOAT_TO_Q(11, in->PCA_basis_table[0][col]);
    out->v1[col] = FLOAT_TO_Q(7, in->PCA_basis_table[1][col]);
    out->v2[col] = FLOAT_TO_Q(7, in->PCA_basis_table[2][col]);
    out->v3[col] = FLOAT_TO_Q(7, in->PCA_basis_table[3][col]);
    out->v4[col] = FLOAT_TO_Q(7, in->PCA_basis_table[4][col]);
    out->v5[col] = FLOAT_TO_Q(7, in->PCA_basis_table[5][col]);
    out->v6[col] = FLOAT_TO_Q(7, in->PCA_basis_table[6][col]);
    out->v7[col] = FLOAT_TO_Q(7, in->PCA_basis_table[7][col]);
  }
} /* pca_rolloff_update_basis_table */

/*===========================================================================
 * FUNCTION    - pca_rolloff_update_coeff_table -
 *
 * DESCRIPTION:
 *
 * Note: In the HW DMI memory color channel order is R, Gr, B & Gb. So we
 *       have to follow the same order.
 *==========================================================================*/
static void pca_rolloff_update_coeff_table(PCA_RolloffStruct *in,
  PCA_RollOffCoeffData *out)
{
  uint32_t row;
  /* Prepare Red CoEff Table */
  for (row = 0; row < PCA_ROLLOFF_NUMBER_ROWS;  row++) {
    out->a0[ISP_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][0]);
    out->a1[ISP_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][1]);
    out->a2[ISP_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][2]);
    out->a3[ISP_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][3]);
    out->a4[ISP_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][4]);
    out->a5[ISP_ROLLOFF_CH_R][row] = FLOAT_TO_Q(9, in->coeff_table_R[row][5]);
    out->a6[ISP_ROLLOFF_CH_R][row] = FLOAT_TO_Q(9, in->coeff_table_R[row][6]);
    out->a7[ISP_ROLLOFF_CH_R][row] = FLOAT_TO_Q(10, in->coeff_table_R[row][7]);
  }

  /* Prepare Green-Red CoEff Table */
  for (row = 0; row < PCA_ROLLOFF_NUMBER_ROWS; row++) {
    out->a0[ISP_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][0]);
    out->a1[ISP_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][1]);
    out->a2[ISP_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][2]);
    out->a3[ISP_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][3]);
    out->a4[ISP_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][4]);
    out->a5[ISP_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(9, in->coeff_table_Gr[row][5]);
    out->a6[ISP_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(9, in->coeff_table_Gr[row][6]);
    out->a7[ISP_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(10, in->coeff_table_Gr[row][7]);
  }

  /* Prepare Blue CoEff Table */
  for (row = 0; row < PCA_ROLLOFF_NUMBER_ROWS; row++) {
    out->a0[ISP_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][0]);
    out->a1[ISP_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][1]);
    out->a2[ISP_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][2]);
    out->a3[ISP_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][3]);
    out->a4[ISP_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][4]);
    out->a5[ISP_ROLLOFF_CH_B][row] = FLOAT_TO_Q(9, in->coeff_table_B[row][5]);
    out->a6[ISP_ROLLOFF_CH_B][row] = FLOAT_TO_Q(9, in->coeff_table_B[row][6]);
    out->a7[ISP_ROLLOFF_CH_B][row] = FLOAT_TO_Q(10, in->coeff_table_B[row][7]);
  }

  /* Prepare Green-Blue CoEff Table */
  for (row = 0; row < PCA_ROLLOFF_NUMBER_ROWS; row++) {
    out->a0[ISP_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][0]);
    out->a1[ISP_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][1]);
    out->a2[ISP_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][2]);
    out->a3[ISP_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][3]);
    out->a4[ISP_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][4]);
    out->a5[ISP_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(9, in->coeff_table_Gb[row][5]);
    out->a6[ISP_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(9, in->coeff_table_Gb[row][6]);
    out->a7[ISP_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(10, in->coeff_table_Gb[row][7]);
  }
} /* pca_rolloff_update_coeff_table */

/*===========================================================================
 * FUNCTION    - pca_rolloff_prepare_basis_hw_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void pca_rolloff_prepare_basis_hw_table(const PCA_RollOffTable *in,
  PCA_RollOffConfigCmdType *cmd)
{
  uint16_t i;
  uint64_t *outRam0 = NULL, *outRam1 = NULL;

  outRam0 = cmd->ram0.basisTable;
  outRam1 = cmd->ram1.basisTable;

  for (i = 0; i < PCA_ROLLOFF_BASIS_TABLE_SIZE; i++) {
    /* pack and write ram0 basis table */
    *outRam0 = (((uint64_t)(in->basisData.v0[i]) & 0x00000000000007FF) << 26);
    *outRam0 |= (((uint64_t)(in->basisData.v1[i]) & 0x00000000000000FF) << 17);
    *outRam0 |= (((uint64_t)(in->basisData.v2[i]) & 0x00000000000000FF) << 8);
    *outRam0 |= (((uint64_t)(in->basisData.v3[i]) & 0x00000000000000FF) << 0);
    outRam0++;

    /* pack and write ram1 basis table */
    *outRam1 = (((uint64_t)(in->basisData.v4[i]) & 0x00000000000000FF) << 24);
    *outRam1 |= (((uint64_t)(in->basisData.v5[i]) & 0x00000000000000FF) << 16);
    *outRam1 |= (((uint64_t)(in->basisData.v6[i]) & 0x00000000000000FF) << 8);
    *outRam1 |= (((uint64_t)(in->basisData.v7[i]) & 0x00000000000000FF) << 0);
    outRam1++;
  }
} /* pca_rolloff_prepare_basis_hw_table */

/*===========================================================================
 * FUNCTION    - pca_rolloff_prepare_coeff_hw_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void pca_rolloff_prepare_coeff_hw_table(const PCA_RollOffTable *in,
  PCA_RollOffConfigCmdType *cmd, int chIndex)
{
  uint16_t i;
  uint64_t *outRam0 = NULL, *outRam1 = NULL;
  int ch_offset = 0;

  switch (chIndex) {
    case ISP_ROLLOFF_CH_R:
      ch_offset = 0 * PCA_ROLLOFF_NUMBER_ROWS;
      break;
    case ISP_ROLLOFF_CH_GR:
      ch_offset = 1 * PCA_ROLLOFF_NUMBER_ROWS;
      break;
    case ISP_ROLLOFF_CH_B:
      ch_offset = 2 * PCA_ROLLOFF_NUMBER_ROWS;
      break;
    case ISP_ROLLOFF_CH_GB:
      ch_offset = 3 * PCA_ROLLOFF_NUMBER_ROWS;
      break;
    default:
      CDBG_ERROR("%s: Invalid chIndex = %d\n", __func__, chIndex);
      return;
  }
  outRam0 = (cmd->ram0.coeffTable) + ch_offset;
  outRam1 = (cmd->ram1.coeffTable) + ch_offset;

  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    /* pack and write ram0 coeff table */
    *outRam0 =
      (((uint64_t)(in->coeffData.a0[chIndex][i]) & 0x0000000000000FFF) << 26);
    *outRam0 |=
      (((uint64_t)(in->coeffData.a1[chIndex][i]) & 0x00000000000001FF) << 17);
    *outRam0 |=
      (((uint64_t)(in->coeffData.a2[chIndex][i]) & 0x00000000000001FF) << 8);
    *outRam0 |=
      (((uint64_t)(in->coeffData.a3[chIndex][i]) & 0x00000000000000FF) << 0);
    outRam0++;

    /* pack and write ram1 coeff table */
    *outRam1 =
      (((uint64_t)(in->coeffData.a4[chIndex][i]) & 0x00000000000000FF) << 24);
    *outRam1 |=
      (((uint64_t)(in->coeffData.a5[chIndex][i]) & 0x00000000000000FF) << 16);
    *outRam1 |=
      (((uint64_t)(in->coeffData.a6[chIndex][i]) & 0x00000000000000FF) << 8);
    *outRam1 |=
      (((uint64_t)(in->coeffData.a7[chIndex][i]) & 0x00000000000000FF) << 0);
    outRam1++;
  }
} /* pca_rolloff_prepare_coeff_hw_table */

/*===========================================================================
 * FUNCTION    - pca_rolloff_prepare_hw_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void pca_rolloff_prepare_hw_table(const PCA_RollOffTable *in,
  PCA_RollOffConfigCmdType *cmd)
{
  /* Write Basis Table */
  pca_rolloff_prepare_basis_hw_table(in, cmd);

  /* Write Coeff Table */
  pca_rolloff_prepare_coeff_hw_table(in, cmd, ISP_ROLLOFF_CH_R);
  pca_rolloff_prepare_coeff_hw_table(in, cmd, ISP_ROLLOFF_CH_GR);
  pca_rolloff_prepare_coeff_hw_table(in, cmd, ISP_ROLLOFF_CH_B);
  pca_rolloff_prepare_coeff_hw_table(in, cmd, ISP_ROLLOFF_CH_GB);
} /* pca_rolloff_prepare_hw_table */

/*===========================================================================
 * FUNCTION    - pca_rolloff_update_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void pca_rolloff_update_table(PCA_RollOffConfigCmdType *cmd,
  PCA_RolloffStruct *tableIn, isp_hw_pix_setting_params_t *pix_settings,
  int is_left_frame)
{
  uint32_t row, col, camif_width, camif_height;
  uint16_t grid_width, grid_height;
  PCA_RollOffTable Tblcfg;

  camif_width = pix_settings->camif_cfg.sensor_out_info.request_crop.last_pixel
    - pix_settings->camif_cfg.sensor_out_info.request_crop.first_pixel + 1;
  camif_height = pix_settings->camif_cfg.sensor_out_info.request_crop.last_line
    - pix_settings->camif_cfg.sensor_out_info.request_crop.first_line + 1;

  grid_width = (camif_width + 31) / 32;
  grid_height = (camif_height + 23) / 24;

  /* Apply persist config first and then update. */
  memset(&cmd->CfgParams, 0, sizeof(PCA_RollOffConfigParams));

  /* VFE_ROLLOFF_CONFIG */
  cmd->CfgParams.pixelOffset = 0;
  cmd->CfgParams.pcaLutBankSel = 0;

  /* VFE_ROLLOFF_GRID_CFG_0 */
  cmd->CfgParams.xDelta = (1 << 20) / grid_width;
  cmd->CfgParams.yDelta = (1 << 13) / grid_height;

  /* VFE_ROLLOFF_GRID_CFG_1 */
  cmd->CfgParams.gridWidth = grid_width - 1;
  cmd->CfgParams.gridHeight = grid_height - 1;

  if (!is_left_frame) {
    /* VFE_ROLLOFF_RIGHT_GRID_CFG_0 */
    cmd->CfgParams.xDeltaRight = (1 << 20) / grid_width;
    cmd->CfgParams.yDeltaRight = (1 << 13) / grid_height;

    /* VFE_ROLLOFF_RIGHT_GRID_CFG_1 */
    cmd->CfgParams.gridWidthRight = grid_width - 1;
    cmd->CfgParams.gridHeightRight = grid_height - 1;
  }

  /* VFE_ROLLOFF_STRIPE_CFG_0 */
  cmd->CfgParams.gridXIndex = 0;
  cmd->CfgParams.gridYIndex = 0;
  cmd->CfgParams.gridPixelXIndex = 0;
  cmd->CfgParams.gridPixelYIndex = 0;

  /* VFE_ROLLOFF_STRIPE_CFG_1 */
  cmd->CfgParams.yDeltaAccum = 0;

  pca_rolloff_update_basis_table(tableIn, &Tblcfg.basisData);
  pca_rolloff_update_coeff_table(tableIn, &Tblcfg.coeffData);

  pca_rolloff_prepare_hw_table(&(Tblcfg), cmd);
} /* pca_rolloff_update_table */

/*==============================================================================
 * Function:           pca_rolloff_calc_awb_trigger
 *
 * Description:
 *============================================================================*/
static void pca_rolloff_calc_awb_trigger(isp_pca_rolloff_mod_t *mod,
                                         PCA_RolloffStruct *tblOut,
                                         isp_pix_trigger_update_input_t *in_params,
                                         isp_pca_rolloff_mod_t* pca_rolloff_ctrl,
                                         int is_left_frame)
{
  float ratio = 0.0;
  cct_trigger_info trigger_info;
  awb_cct_type cct_type;
  chromatix_parms_type *chrPtr =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_VFE_common_type *chrComPtr =
    (chromatix_VFE_common_type *)in_params->cfg.chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *rolloffPtr = &chrComPtr->chromatix_rolloff;

  trigger_info.mired_color_temp =
    MIRED(in_params->trigger_input.stats_update.awb_update.color_temp);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A, rolloffPtr->rolloff_A_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
    rolloffPtr->rolloff_D65_trigger);

  cct_type = isp_util_get_awb_cct_type(mod->notify_ops->parent, &trigger_info,
    chrPtr);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: cct_type = %d\n", __func__, cct_type);
  switch (cct_type) {
  case AWB_CCT_TYPE_A:
    *tblOut = pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_A_LIGHT];
    break;
  case AWB_CCT_TYPE_TL84_A:
    ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);

    pca_rolloff_table_interpolate(
      &(pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_TL84_LIGHT]),
      &(pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_A_LIGHT]), tblOut,
      ratio);
    break;
  case AWB_CCT_TYPE_D65_TL84:
    ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_d65.mired_end,
        trigger_info.trigger_d65.mired_start);

    pca_rolloff_table_interpolate(
      &(pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_D65_LIGHT]),
      &(pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_TL84_LIGHT]), tblOut,
      ratio);
    break;
  case AWB_CCT_TYPE_D65:
    *tblOut = pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_D65_LIGHT];
    break;
  case AWB_CCT_TYPE_TL84:
  default:
    *tblOut = pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_TL84_LIGHT];
    break;
  }
} /* pca_rolloff_calc_awb_trigger */

/*==============================================================================
 * FUNCTION    - pca_rolloff_calc_flash_trigger -
 *
 * DESCRIPTION:
 *============================================================================*/
static void pca_rolloff_calc_flash_trigger(PCA_RolloffStruct *tblCCT,
  PCA_RolloffStruct *tblOut, isp_pix_trigger_update_input_t *trigger_params,
  isp_pca_rolloff_mod_t *pca_rolloff_ctrl, int is_left_frame)
{
  float ratio;
  float flash_start, flash_end;
  PCA_RolloffStruct *tblFlash = NULL;
  isp_flash_params_t *flash_params = &(trigger_params->cfg.flash_params);
  cam_flash_mode_t *flash_mode = &(trigger_params->trigger_input.flash_mode);
  chromatix_parms_type *chrPtr =
   (chromatix_parms_type *)trigger_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_VFE_common_type *chrComPtr =
   (chromatix_VFE_common_type *)
     trigger_params->cfg.chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *rolloffPtr = &chrComPtr->chromatix_rolloff;

  ratio = flash_params->sensitivity_led_off / flash_params->sensitivity_led_hi;

  if ((int)flash_params->flash_type == CAMERA_FLASH_STROBE) {
    flash_start = rolloffPtr->rolloff_Strobe_start;
    flash_end = rolloffPtr->rolloff_Strobe_end;
    tblFlash =
      &(pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_STROBE_FLASH]);
  } else {
    flash_start = rolloffPtr->rolloff_LED_start;
    flash_end = rolloffPtr->rolloff_LED_end;
    tblFlash =
      &(pca_rolloff_ctrl->pca_tbls.left_table[ISP_ROLLOFF_LED_FLASH]);
  }
  /*sanity check, if input is invalid, then use flash table directly*/
  if (*flash_mode == CAM_FLASH_MODE_TORCH) {
    /*estimation default use flash table if input is invalid*/
    if (flash_params->sensitivity_led_low != 0) {
      ratio = flash_params->sensitivity_led_off / flash_params->sensitivity_led_low;
    }
    else {
      ratio = flash_end;
    }
  } else if (*flash_mode == CAM_FLASH_MODE_ON) {
    if (flash_params->sensitivity_led_hi != 0) {
      ratio = flash_params->sensitivity_led_off / flash_params->sensitivity_led_hi;
    }
    else {
      ratio = flash_end;
    }
  } else //assume flash off. To be changed when AUTO mode is added
      ratio = flash_start;
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: flash_start %5.2f flash_end %5.2f \n", __func__, flash_start,
    flash_end);

  if (ratio >= flash_end)
    *tblOut = *tblFlash;
  else if (ratio <= flash_start)
    *tblOut = *tblCCT;
  else {
    ratio = GET_INTERPOLATION_RATIO(ratio, flash_start, flash_end);
    pca_rolloff_table_interpolate(tblCCT, tblFlash, tblOut, ratio);
  }
} /* pca_rolloff_calc_flash_trigger */

/*==============================================================================
 * FUNCTION    - pca_rolloff_allocate_scratch_mem -
 *
 * DESCRIPTION:
 *============================================================================*/
static int pca_rolloff_allocate_scratch_mem(double ***out,
  double **temp, int x, int y)
{
  int i;
  *out = (double **)malloc(x * sizeof(double *));
  if (!(*out)) {
    CDBG_ERROR("%s: Not enough memory for out\n", __func__);
    return -1;
  }
  *temp = (double *)malloc((x * y) * sizeof(double));
  if (!(*temp)) {
    CDBG_ERROR("%s: Not enough memory for temp \n", __func__);
    free(*out);
    return -1;
  }
  for (i = 0; i < x; i++)
    (*out)[i] = (*temp) + (i * y);

  return 0;
} /* pca_rolloff_allocate_scratch_mem */

/*==============================================================================
 * FUNCTION    - pca_rolloff_convert_tables -
 *
 * DESCRIPTION: This routine genrerates PCA bases and coefficients.
 *============================================================================*/
static int pca_rolloff_convert_tables(isp_rolloff_info_t *mesh_tbls,
  isp_pca_rolloff_mod_t* pca_rolloff_ctrl, int is_left_frame, int n_tbl, int n_idxtbl)
{
  int i, j, k, x, y;
  int w = 17, h = 13, nbases = 8, nch = 4, ntbl = n_tbl;
  double **bases = NULL;
  double **illu_tbls = NULL, **illu_coeffs = NULL;
  double **flash_tbls = NULL, **flash_coeffs = NULL;
  double *scratch1 = NULL, *scratch2 = NULL, *scratch3 = NULL,
    *scratch4 = NULL, *scratch5 = NULL;
  mesh_rolloff_array_type *temp;
  PCA_RolloffStruct *dest;

  /* allocate memory for rearranged TL84, D65, A, Low light, LED, Strobe mesh tables. */
  x = w;
  y = h * nch * (ntbl);

  if (0 != pca_rolloff_allocate_scratch_mem(&illu_tbls, &scratch1,
    x, y)) {
    CDBG_ERROR("%s: pca_rolloff_allocate_scratch_mem for illu_tbls failed.",
      __func__);
    return -1;
  }

  for(i = 0; i < (ntbl); i++) {
    if (ntbl == PCA_ROLLOFF_TINTLESS_CONVERT_TABLE_SIZE) {
      if (is_left_frame)
        temp = &(mesh_tbls->left[n_idxtbl]);
      else
        temp = &(mesh_tbls->right[n_idxtbl]);
    } else{
      if (is_left_frame)
        temp = &(mesh_tbls->left[i]);
      else
        temp = &(mesh_tbls->right[i]);
    }
    /* Red channel */
    for(j = 0; j < h; j++)
      for(k = 0; k < w; k++)
        illu_tbls[k][(i + 0*(ntbl))*h + j] = temp->r_gain[j*w + k];
    /* GR channel */
    for(j = 0; j < h; j++)
      for(k = 0; k < w; k++)
        illu_tbls[k][(i + 1*(ntbl))*h + j] = temp->gr_gain[j*w + k];
    /* GB channel */
    for(j = 0; j < h; j++)
      for(k = 0; k < w; k++)
        illu_tbls[k][(i + 2*(ntbl))*h + j] = temp->gb_gain[j*w + k];
    /* Blue channel */
    for(j = 0; j < h; j++)
      for(k = 0; k < w; k++)
        illu_tbls[k][(i + 3*(ntbl))*h + j] = temp->b_gain[j*w + k];
  }

  /* allocate memory for PCA bases */
  x = nbases;
  y = w;
  if (0 != pca_rolloff_allocate_scratch_mem(&bases, &scratch2, x, y)) {
    CDBG_ERROR("%s: pca_rolloff_allocate_scratch_mem for bases failed.",
      __func__);
    free(scratch1);
    free(illu_tbls);
    return -1;
  }

  /* allocate memory for TL84, D65, A, Low light, LED, Strobe PCA coefficients */
  x = nbases;
  y = h * nch * (ntbl);
  if (0 != pca_rolloff_allocate_scratch_mem(&illu_coeffs, &scratch3,
    x, y)) {
    CDBG_ERROR("%s: pca_rolloff_allocate_scratch_mem for illu_coeffs failed.",
      __func__);
    free(scratch1);
    free(illu_tbls);
    free(scratch2);
    free(bases);
    return -1;
  }

  /* Generate PCA bases and coefficients from TL84, D65, A, Low light, LED, Strobe
   * mesh tables.
   */
  if (subspaceoptim(illu_tbls, ntbl, w, h,
    nbases, illu_coeffs, bases, FALSE) != 0) {
    free(scratch1);
    free(scratch2);
    free(scratch3);
    free(illu_tbls);
    free(bases);
    free(illu_coeffs);
    return -1;
  }

  /* Write PCA bases and coefficients of TL84, D65, A, Low light, LED, Strobe */
  for (i = 0; i < (ntbl); i++) {
    if (ntbl == PCA_ROLLOFF_TINTLESS_CONVERT_TABLE_SIZE) {
      if (is_left_frame)
        dest = &(pca_rolloff_ctrl->pca_tbls.left_table[n_idxtbl]);
      else
        dest = &(pca_rolloff_ctrl->pca_tbls.right_table[n_idxtbl]);
      } else {
        if (is_left_frame)
          dest = &(pca_rolloff_ctrl->pca_tbls.left_table[i]);
        else
          dest = &(pca_rolloff_ctrl->pca_tbls.right_table[i]);
      }


    for(j = 0; j < nbases; j++)
      for(k = 0; k < w; k++)
        dest->PCA_basis_table[j][k] = bases[j][k];

    for(j = 0; j < h; j++) {
      for(k = 0; k < nbases; k++) {
        dest->coeff_table_R[j][k]  = illu_coeffs[k][0*ntbl*h + i*h + j];
        dest->coeff_table_Gr[j][k] = illu_coeffs[k][1*ntbl*h + i*h + j];
        dest->coeff_table_Gb[j][k] = illu_coeffs[k][2*ntbl*h + i*h + j];
        dest->coeff_table_B[j][k]  = illu_coeffs[k][3*ntbl*h + i*h + j];
      }
    }
  }
  free(scratch1);
  free(scratch2);
  free(scratch3);
  free(illu_tbls);
  free(illu_coeffs);
  free(bases);

  return 0;
} /* pca_rolloff_convert_tables */

/** pca_rolloff_tintless_lowlight_adjust:
 *
 *    @mod:
 *    @output_mesh_table:
 *
 *  adjust tintless output table
 *
 *  Return 0 on Success, negative on ERROR
 **/
static void pca_rolloff_tintless_lowlight_adjust(isp_pca_rolloff_mod_t *mod,
  tintless_mesh_rolloff_array_t *output_mesh_table)
{
  int i;
  int j;
  float min_gain = 1.0;
  float correction_gain;

  /*adjust rolloff table for low light, also get normalize ratio by min gain*/
  for (i = 0; i < TINTLESS_ROLLOFF_TABLE_SIZE; i++) {
    output_mesh_table->r_gain[i] *= mod->tintless_current_adjust[i];
    min_gain =
      MIN(min_gain, output_mesh_table->r_gain[i]);

    output_mesh_table->gb_gain[i] *= mod->tintless_current_adjust[i];
      min_gain =
        MIN(min_gain, output_mesh_table->gb_gain[i]);

    output_mesh_table->gr_gain[i] *= mod->tintless_current_adjust[i];
    min_gain =
      MIN(min_gain, output_mesh_table->gr_gain[i]);

    output_mesh_table->b_gain[i] *= mod->tintless_current_adjust[i];
    min_gain =
      MIN(min_gain, output_mesh_table->b_gain[i]);
  }

  if (min_gain < 1.0 && min_gain > 0.0) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: min_gain = %f, Normalize rolloff table!\n", __func__, min_gain);
    correction_gain =  1/min_gain;
    for (i = 0; i < TINTLESS_ROLLOFF_TABLE_SIZE; i++) {
      output_mesh_table->r_gain[i] *= correction_gain;
      output_mesh_table->gb_gain[i] *= correction_gain;
      output_mesh_table->gr_gain[i] *= correction_gain;
      output_mesh_table->b_gain[i] *= correction_gain;
    }
  }

} /* mesh_rolloff_tintless_lowlight_adjust */

/** pca_rolloff_tintless_trigger_update:
 *
 *    @mod: Pointer to rolloff module struct
 *    @trigger_params: Pipeline trigger update params
 *
 *  Return 0 on success, -1 on Error
 **/
static int pca_rolloff_tintless_trigger_update(isp_pca_rolloff_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params)
{
  int rc = 0;
  chromatix_VFE_common_type *chrComPtr =
   (chromatix_VFE_common_type *)
    trigger_params->cfg.chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *chromatix_rolloff = &chrComPtr->chromatix_rolloff;
  PCA_RollOffConfigCmdType* cmd = &mod->pca_rolloff_cmd;
  int is_burst = IS_BURST_STREAMING(&(trigger_params->cfg));
  camera_flash_type flash_type = trigger_params->cfg.flash_params.flash_type;
  isp_rolloff_info_t meshtbls;
  int i;
  float aec_ratio = 0.0;

  aec_ratio = isp_util_get_aec_ratio(mod->notify_ops->parent,
    chromatix_rolloff->control_rolloff,
    &(chromatix_rolloff->rolloff_lowlight_trigger),
    &trigger_params->trigger_input.stats_update.aec_update, is_burst);

  /*determine low light condition for low light ratio adjust
    only pure low light will go into tintless lowlight mode*/
  if (F_EQUAL(aec_ratio, 0.0)) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: tintless low light mode\n", __func__);
    mod->tintless_low_light_mode = 1;
  } else if (F_EQUAL(aec_ratio, 1.0)) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: tintless normal light \n", __func__);
    mod->tintless_low_light_mode = 0;
  }

  if (!is_burst) {
    isp_tintless_notify_data_t tintless_data;
    tintless_mesh_rolloff_array_t mesh_hw;

    tintless_data.session_id = mod->session_id;
    tintless_data.notify_data = &mesh_hw;
    tintless_data.notify_data_size = sizeof(tintless_mesh_rolloff_array_t);

    /* get mesh_hw */
    rc = mod->notify_ops->notify(mod->notify_ops->parent,
           mod->notify_ops->handle, ISP_HW_MOD_NOTIFY_GET_ROLLOFF_TABLE,
          &tintless_data, sizeof(isp_tintless_notify_data_t));
    if (rc < 0) {
     CDBG_ERROR("%s: Unable to config tintless rc = %d\n", __func__, rc);
     rc = -1;
    }

    if (mod->tintless_low_light_mode == 1) {
      for (i = 0; i < TINTLESS_ROLLOFF_TABLE_SIZE; i++) {
        mod->tintless_current_adjust[i] =
          mod->tintless_lowlight_adjust[i]* (TINTLESS_TEMPORAL_RATIO)  +
          mod->tintless_current_adjust[i] * (1 - TINTLESS_TEMPORAL_RATIO);
      }
      pca_rolloff_tintless_lowlight_adjust(mod, &mesh_hw);
    } else {
      for (i = 0; i < TINTLESS_ROLLOFF_TABLE_SIZE; i++) {
        mod->tintless_current_adjust[i] =
          1 * TINTLESS_TEMPORAL_RATIO  +
          mod->tintless_current_adjust[i] * (1 - TINTLESS_TEMPORAL_RATIO);
      }
      pca_rolloff_tintless_lowlight_adjust(mod, &mesh_hw);
    }

    /* for tintless, we will have only one output table. */
    memcpy(&mod->rolloff_tbls.left[ISP_ROLLOFF_TL84_LIGHT], &mesh_hw, sizeof (tintless_mesh_rolloff_array_t));

    /* for tintless,we need to convert only one table. */
    pca_rolloff_convert_tables((isp_rolloff_info_t *)&mod->rolloff_tbls, mod, TRUE,
                               PCA_ROLLOFF_TINTLESS_CONVERT_TABLE_SIZE,ISP_ROLLOFF_TL84_LIGHT);
    /* Q10 and packing */
  } else { // burst_mode
     if (flash_type != CAMERA_FLASH_NONE) {
     }
  }

  /* interpolate flash */
    /* TODO: tintless : add flash logic and Snapshot logic
       after flash is available for flash */

  return rc;
} /* pca_rolloff_tintless_trigger_update */
/*===========================================================================
 * FUNCTION      pca_rolloff_trigger_update
 *
 * DESCRIPTION:
 *==========================================================================*/
static int pca_rolloff_tintless_trigger_update(isp_pca_rolloff_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params);
static int pca_rolloff_trigger_update(isp_pca_rolloff_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  int rc = 0;
  float aec_ratio = 0.0;
  static float cur_real_gain = 1.0;
  static float cur_lux_idx = 1.0;
  static float cur_mired_color_temp = 1.0;
  static cam_flash_mode_t cur_flash_mode = CAMERA_FLASH_NONE;
  float new_real_gain = cur_real_gain;
  float new_lux_idx;
  float new_mired_color_temp;
  int is_burst = IS_BURST_STREAMING((&trigger_params->cfg));
  PCA_RollOffTable Tblcfg;
  cam_flash_mode_t new_flash_mode;
  PCA_RolloffStruct *pcaRolloffLeftTableFinal = NULL;
  PCA_RolloffStruct pcaRolloffLeftTableCCT;
  chromatix_parms_type *chrPtr =
     (chromatix_parms_type *)trigger_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_VFE_common_type *chrComPtr = (chromatix_VFE_common_type *)
     trigger_params->cfg.chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *rolloffPtr = &chrComPtr->chromatix_rolloff;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }

  if (!mod->pca_rolloff_enable) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Pca Rolloff is disabled. Skip the trigger.\n", __func__);
    return 0;
  }

  if (!mod->pca_rolloff_trigger_enable) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return 0;
  }

  if (trigger_params->trigger_input.stats_update.awb_update.color_temp == 0) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Skip trigger update, Color Temperature is 0.\n", __func__);
    return 0;
  }
  /*TODO: discuss this logic to check if it is necessory*/
  if (is_burst) {
    new_real_gain = trigger_params->trigger_input.stats_update.aec_update.real_gain;
  } else {
    new_real_gain = trigger_params->trigger_input.stats_update.aec_update.real_gain;
    /*Skip trigger update if AEC is not settled and also tintless is disabled.
      Tintless if enabled needs to be applied to every frame irrespective of
      AEC settle*/
    if (!isp_util_aec_check_settled(&(trigger_params->trigger_input.stats_update.aec_update))
        && !trigger_params->cfg.tintless_data->is_enabled) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: AEC is not setteled. Tintless disabled. Skip Trigger\n",
        __func__);
      return 0;
    }
  }

  pcaRolloffLeftTableFinal = &(mod->pca_rolloff_param.left_input_table);

  new_lux_idx = trigger_params->trigger_input.stats_update.aec_update.lux_idx;
  new_flash_mode = trigger_params->trigger_input.flash_mode;
  new_mired_color_temp = MIRED(trigger_params->trigger_input.stats_update.awb_update.color_temp);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: cur_gain %f new_gain %f cur_flash %d new_flash %d mode %d\n",
    __func__, cur_real_gain, new_real_gain, cur_flash_mode, new_flash_mode,
    mod->old_streaming_mode);

  if ((F_EQUAL(cur_real_gain, new_real_gain)) &&
      (cur_lux_idx == new_lux_idx) &&
      (cur_flash_mode == new_flash_mode) &&
      (!mod->pca_rolloff_reload_params) &&
      (mod->old_streaming_mode == trigger_params->cfg.streaming_mode)) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: No change in trigger. Nothing to update\n", __func__);
    return 0;
  } else {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Change in trigger. Update roll-off tables.\n", __func__);
    cur_real_gain = new_real_gain;
    cur_lux_idx = new_lux_idx;
    cur_mired_color_temp = new_mired_color_temp;
    mod->old_streaming_mode = trigger_params->cfg.streaming_mode;
  }

  mod->hw_update_pending = TRUE;
  if (trigger_params->cfg.tintless_data->is_supported &&
      trigger_params->cfg.tintless_data->is_enabled) {
    /*if TORCH(LED LOW), then save the non flash table before fetch new table*/
    if (cur_flash_mode != CAM_FLASH_MODE_TORCH &&
        new_flash_mode == CAM_FLASH_MODE_TORCH) {
      memcpy(&(mod->last_non_flash_tbl),
        &(mod->pca_rolloff_param.left_input_table),
        sizeof(PCA_RolloffStruct));
    }
    rc = pca_rolloff_tintless_trigger_update(mod, trigger_params);
    if (rc < 0) {
      CDBG_ERROR("%s: error: tintless Trigger update failed", __func__);
      mod->hw_update_pending = FALSE;
      return -1;
    }
  }
  /* Note: Interpolate table based on AWB
           AWB's CCT interpolated tables are used regardeless Flash
   *       is on or not. So derive them before checking Flash on or not. */
  /* Left frame */
  pca_rolloff_calc_awb_trigger(mod,
                               &pcaRolloffLeftTableCCT, trigger_params,
                               mod, TRUE);

  cur_flash_mode = new_flash_mode;
  /* interpolate table based on flash mode */
  if (new_flash_mode != CAM_FLASH_MODE_OFF) {
    /* Left frame */
    if (trigger_params->cfg.tintless_data->is_supported &&
        trigger_params->cfg.tintless_data->is_enabled) {
      pca_rolloff_calc_flash_trigger(&mod->last_non_flash_tbl,
        pcaRolloffLeftTableFinal, trigger_params, mod, TRUE);
    } else
      pca_rolloff_calc_flash_trigger(&pcaRolloffLeftTableCCT,
        pcaRolloffLeftTableFinal, trigger_params, mod, TRUE);
  } else {
    if (trigger_params->cfg.tintless_data->is_supported &&
        trigger_params->cfg.tintless_data->is_enabled) {
      *pcaRolloffLeftTableFinal =
        mod->pca_tbls.left_table[ISP_ROLLOFF_TL84_LIGHT];
    } else {
    aec_ratio = isp_util_get_aec_ratio(mod->notify_ops->parent,
      rolloffPtr->control_rolloff, &(rolloffPtr->rolloff_lowlight_trigger),
      &trigger_params->trigger_input.stats_update.aec_update, is_burst);
    if (F_EQUAL(aec_ratio, 0.0)) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: Low Light \n", __func__);
      /* Left frame */
      *pcaRolloffLeftTableFinal =
        mod->pca_tbls.left_table[ISP_ROLLOFF_TL84_LOW_LIGHT];
    } else if (F_EQUAL(aec_ratio, 1.0)) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: Bright Light \n", __func__);
      /* Left frame */
      *pcaRolloffLeftTableFinal = pcaRolloffLeftTableCCT;
    } else {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: Interpolate between CCT and Low Light \n", __func__);
      /* Left frame */
      pca_rolloff_table_interpolate(&pcaRolloffLeftTableCCT,
        &(mod->pca_tbls.left_table[ISP_ROLLOFF_TL84_LOW_LIGHT]),
        pcaRolloffLeftTableFinal, aec_ratio);
    }
   }
  }
  /* prepare HW pca table by bit packing, left frame only */
  pca_rolloff_update_basis_table(&(mod->pca_rolloff_param.left_input_table),
    &Tblcfg.basisData);
  pca_rolloff_update_coeff_table(&(mod->pca_rolloff_param.left_input_table),
    &Tblcfg.coeffData);
  pca_rolloff_prepare_hw_table(&(Tblcfg), &(mod->pca_rolloff_cmd));

  return rc;
} /* pca_rolloff_trigger_update */

/*===========================================================================
 * FUNCTION    - pca_rolloff_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int pca_rolloff_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_pca_rolloff_mod_t *pca_rolloff = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  int i = 0;

  pca_rolloff->fd = init_params->fd;
  pca_rolloff->notify_ops = notify_ops;
  pca_rolloff->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  pca_rolloff->hw_update_pending = FALSE;
  pca_rolloff->tintless_low_light_mode = 0;

  for (i = 0; i < TINTLESS_ROLLOFF_TABLE_SIZE; i++) {
    pca_rolloff->tintless_current_adjust[i] = 1.0;
  }
  return 0;
} /* pca_rolloff_init */


/*===========================================================================
 * FUNCTION    - pca_rolloff_tintless_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int pca_rolloff_tintless_config(isp_pca_rolloff_mod_t *mesh_mod,
  isp_hw_pix_setting_params_t *pix_settings)
{
  int rc = TINTLESS_SUCCESS,i;
  sensor_request_crop_t camif =
    pix_settings->camif_cfg.sensor_out_info.request_crop;
  camera_flash_type flash_type = pix_settings->flash_params.flash_type;
  PCA_RollOffConfigCmdType *cmd = NULL;
  isp_tintless_mesh_config_t isp_mesh_cfg;
  tintless_stats_config_t stats_cfg;
  isp_tintless_notify_data_t tintless_data;

  cmd = &(mesh_mod->pca_rolloff_cmd);
  if(mesh_mod->tintless_configured){
    isp_tintless_notify_data_t tintless_data;
    tintless_mesh_rolloff_array_t mesh_hw;

    tintless_data.session_id = mesh_mod->session_id;
    tintless_data.notify_data = &mesh_hw;
    tintless_data.notify_data_size = sizeof(tintless_mesh_rolloff_array_t);

    /* get mesh_hw */
    rc = mesh_mod->notify_ops->notify(mesh_mod->notify_ops->parent,
           mesh_mod->notify_ops->handle, ISP_HW_MOD_NOTIFY_GET_ROLLOFF_TABLE,
          &tintless_data, sizeof(isp_tintless_notify_data_t));
    if (rc < 0) {
     CDBG_ERROR("%s: Unable to config tintless rc = %d\n", __func__, rc);
     rc = -1;
    }
    /* for tintless, we will have only one output table. */
    memcpy(&mesh_mod->rolloff_tbls.left[ISP_ROLLOFF_TL84_LIGHT], &mesh_hw, sizeof (tintless_mesh_rolloff_array_t));

     /* for tintless,we need to convert only one table. */
    pca_rolloff_convert_tables((isp_rolloff_info_t *)&mesh_mod->rolloff_tbls,
                               mesh_mod, TRUE, PCA_ROLLOFF_TINTLESS_CONVERT_TABLE_SIZE,
                               ISP_ROLLOFF_TL84_LIGHT);

    mesh_mod->pca_rolloff_param.left_input_table =
      mesh_mod->pca_tbls.left_table[ISP_ROLLOFF_TL84_LIGHT];
  }
  else {

  /* set tintless tables */
  rc = rolloff_tintless_prepare_tables(mesh_mod, pix_settings,
         &isp_mesh_cfg);
  if (rc < 0) {
    CDBG_ERROR("%s: tintless rolloff prepare table failed\n", __func__);
    return -1;
  }
  mesh_mod->tintless_configured = 1;
  mesh_mod->pca_rolloff_param.left_input_table =
      mesh_mod->pca_tbls.left_table[ISP_ROLLOFF_TL84_LIGHT];


  tintless_data.session_id = mesh_mod->session_id;
  tintless_data.notify_data = &isp_mesh_cfg;
  tintless_data.notify_data_size = sizeof(isp_tintless_mesh_config_t);

  rc = mesh_mod->notify_ops->notify(mesh_mod->notify_ops->parent,
         mesh_mod->notify_ops->handle, ISP_HW_MOD_NOTIFY_ROLL_CONFIG,
         &tintless_data, sizeof(isp_tintless_notify_data_t));
  if (rc < 0) {
    CDBG_ERROR("%s: Unable to config tintless rc = %d\n", __func__, rc);
    rc = -1;
  }
 }
   pca_rolloff_update_table(&(mesh_mod->pca_rolloff_cmd),
   &(mesh_mod->pca_rolloff_param.left_input_table), pix_settings, TRUE);
   mesh_mod->hw_update_pending = TRUE;

  return rc;
} /* mesh_rolloff_tintless_config */


/*===========================================================================
 * FUNCTION    - pca_rolloff_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int pca_rolloff_config(isp_pca_rolloff_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  PCA_RollOffConfigCmdType cmdRight;
  chromatix_parms_type *chrPtr;
  int rc = 0;
  int i,n_tbl = ISP_ROLLOFF_MAX_LIGHT,n_idxtbl = 0;
  int is_burst = IS_BURST_STREAMING(pix_settings);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s\n", __func__);
  chrPtr = (chromatix_parms_type *)pix_settings->chromatix_ptrs.chromatixPtr;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  if (!pix_settings->camif_cfg.is_bayer_sensor) {
    CDBG_HIGH("%s: not Bayer Format, not support rolloff\n", __func__);
    return 0;
  }
  if (pix_settings->tintless_data->is_supported &&
        pix_settings->tintless_data->is_enabled)
  {
    n_tbl = PCA_ROLLOFF_TINTLESS_CONVERT_TABLE_SIZE;
    /* We need to convert only LED flash table.*/
    n_idxtbl = ISP_ROLLOFF_LED_FLASH;
  }

  /*get mesh tables from chromatix*/
  if (0 != rolloff_prepare_tables(mod, pix_settings)) {
    CDBG_ERROR("%s: rolloff prepare initial table failed\n", __func__);
    return -1;
  }

  mod->session_id = pix_settings->outputs->stream_param.session_id;

  /* convert MESH table into PCA table*/
  if (!is_burst) {
    if (0 != pca_rolloff_convert_tables(&mod->rolloff_tbls, mod, TRUE,n_tbl,n_idxtbl)) {
      CDBG_HIGH("%s: Mesh to PCA failed. Disable rollOff\n", __func__);
      mod->pca_rolloff_enable = FALSE;
      return -1;
    }
  }
  mod->pca_rolloff_param.left_input_table =
    mod->pca_tbls.left_table[ISP_ROLLOFF_TL84_LIGHT];

  /*print out pca table content*/
  if (PCA_ROLLOFF_TABLE_DEBUG) {
    for (i = ISP_ROLLOFF_TL84_LIGHT; i < ISP_ROLLOFF_MAX_LIGHT; i++) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: PCA Rolloff left table %d values\n", __func__, i);
      pca_rolloff_table_debug(&(mod->pca_tbls.left_table[i]), 1,
        &(mod->rolloff_tbls.left[i]));
    }
  }

  if (pix_settings->tintless_data->is_supported &&
      pix_settings->tintless_data->is_enabled &&
      (!is_burst)) {
    rc = pca_rolloff_tintless_config(mod, pix_settings);
    if (rc < 0) {
      CDBG_ERROR("%s: unable to config tintless\n", __func__);
    }
  } else {
  /*prepare HW pca table by bit packing, left frame only*/
  pca_rolloff_update_table(&(mod->pca_rolloff_cmd),
    &(mod->pca_rolloff_param.left_input_table), pix_settings, TRUE);
  }
  mod->hw_update_pending = TRUE;

  return rc;
}

/* ============================================================
 * function name: pca_rolloff_enable
 * description: enable pca_rolloff
 * ============================================================*/
static int pca_rolloff_enable(isp_pca_rolloff_mod_t *pca_rolloff,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  pca_rolloff->pca_rolloff_enable = enable->enable;

  return 0;
}

/* ============================================================
 * function name: pca_rolloff_trigger_enable
 * description: enable trigger update feature
 * ============================================================*/
static int pca_rolloff_trigger_enable(isp_pca_rolloff_mod_t *pca_rolloff,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  pca_rolloff->pca_rolloff_trigger_enable = enable->enable;

  return 0;
}

/* ============================================================
 * function name: pca_rolloff_destroy
 * description: close pca_rolloff
 * ============================================================*/
static int pca_rolloff_destroy(void *mod_ctrl)
{
  isp_pca_rolloff_mod_t *pca_rolloff = mod_ctrl;

  memset(pca_rolloff, 0, sizeof(isp_pca_rolloff_mod_t));
  free(pca_rolloff);
  return 0;
}

/* ============================================================
 * function name: pca_rolloff_set_params
 * description: set parameters
 * ============================================================*/
static int pca_rolloff_set_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_pca_rolloff_mod_t *pca_rolloff = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = pca_rolloff_enable(pca_rolloff, (isp_mod_set_enable_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = pca_rolloff_config(pca_rolloff,
      (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = pca_rolloff_trigger_enable(pca_rolloff,
      (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = pca_rolloff_trigger_update(pca_rolloff,
      (isp_pix_trigger_update_input_t *)in_params, in_param_size);
    break;
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    break;
  }
  return rc;
}

/* ============================================================
 * function name: pca_rolloff_get_params
 * description: get parameters
 * ============================================================*/
static int pca_rolloff_get_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  int rc =0;
  isp_pca_rolloff_mod_t *pca_rolloff = mod_ctrl;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = pca_rolloff->pca_rolloff_enable;
    break;
  }

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    vfe_diag->control_rolloff.enable = pca_rolloff->pca_rolloff_enable;
    vfe_diag->control_rolloff.cntrlenable = pca_rolloff->pca_rolloff_trigger_enable;
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Populating vfe_diag data", __func__);
  }
    break;

  case ISP_HW_MOD_GET_TINTLESS_RO: {
    tintless_mesh_rolloff_array_t *rolloff = (tintless_mesh_rolloff_array_t *)out_params;
    if (sizeof(tintless_mesh_rolloff_array_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    /*Populate rolloff data */
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Populating rolloff data", __func__);
    memcpy(rolloff, &pca_rolloff->mesh_hw, sizeof(tintless_mesh_rolloff_array_t));
  }
    break;

  default:
    rc = -EPERM;
    break;
  }
  return rc;
}

/* ============================================================
 * function name: pca_rolloff_do_hw_update
 * description: pca_rolloff_do_hw_update
 * ============================================================*/
static int pca_rolloff_do_hw_update(isp_pca_rolloff_mod_t *pca_rolloff)
{
  int i, rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[10];
  uint32_t rolloff_lut_channel;

  if (pca_rolloff->hw_update_pending) {
    rolloff_lut_channel =
        (pca_rolloff->pca_rolloff_cmd.CfgParams.pcaLutBankSel == 0) ?
          ROLLOFF_RAM0_BANK0 : ROLLOFF_RAM0_BANK1;

    /* prepare dmi_set for Ram0 */
    pca_rolloff->pca_rolloff_cmd.dmi_set0[0] = ISP32_DMI_CFG_DEFAULT +
        rolloff_lut_channel;
    pca_rolloff->pca_rolloff_cmd.dmi_set0[1] = 0;

    rolloff_lut_channel =
        (pca_rolloff->pca_rolloff_cmd.CfgParams.pcaLutBankSel == 0) ?
          ROLLOFF_RAM1_BANK0 : ROLLOFF_RAM1_BANK1;

    /* prepare dmi_set for Ram1 */
    pca_rolloff->pca_rolloff_cmd.dmi_set1[0] = ISP32_DMI_CFG_DEFAULT +
        rolloff_lut_channel;
    pca_rolloff->pca_rolloff_cmd.dmi_set1[1] = 0;

    /* prepare dmi_reset fields */
    pca_rolloff->pca_rolloff_cmd.dmi_reset[0] = ISP32_DMI_CFG_DEFAULT +
        ISP32_DMI_NO_MEM_SELECTED;
    pca_rolloff->pca_rolloff_cmd.dmi_reset[1] = 0;

    cfg_cmd.cfg_data = (void *)&pca_rolloff->pca_rolloff_cmd;
    cfg_cmd.cmd_len = sizeof(pca_rolloff->pca_rolloff_cmd);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 10;

    /* set dmi to proper lut bank */
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP32_DMI_CFG_OFF;
    reg_cfg_cmd[0].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[0].u.rw_info.cmd_data_offset + reg_cfg_cmd[0].u.rw_info.len;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP32_DMI_ADDR;
    reg_cfg_cmd[1].u.rw_info.len = 1 * sizeof(uint32_t);

    /* send dmi data */
    reg_cfg_cmd[2].cmd_type = VFE_WRITE_DMI_64BIT;
    reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset =
      reg_cfg_cmd[1].u.rw_info.cmd_data_offset + reg_cfg_cmd[1].u.rw_info.len;
    reg_cfg_cmd[2].u.dmi_info.hi_tbl_offset =
      reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset + 4;
    reg_cfg_cmd[2].u.dmi_info.len = sizeof(PCA_RollOffRamTable);

    /* set dmi to proper lut bank */
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset + reg_cfg_cmd[2].u.rw_info.len;
    reg_cfg_cmd[3].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[3].u.rw_info.reg_offset = ISP32_DMI_CFG_OFF;
    reg_cfg_cmd[3].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[4].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[3].u.rw_info.cmd_data_offset + reg_cfg_cmd[3].u.rw_info.len;
    reg_cfg_cmd[4].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[4].u.rw_info.reg_offset = ISP32_DMI_ADDR;
    reg_cfg_cmd[4].u.rw_info.len = 1 * sizeof(uint32_t);

    /* send dmi data */
    reg_cfg_cmd[5].cmd_type = VFE_WRITE_DMI_64BIT;
    reg_cfg_cmd[5].u.dmi_info.lo_tbl_offset =
      reg_cfg_cmd[4].u.rw_info.cmd_data_offset + reg_cfg_cmd[4].u.rw_info.len;
    reg_cfg_cmd[5].u.dmi_info.hi_tbl_offset =
      reg_cfg_cmd[5].u.dmi_info.lo_tbl_offset + 4;
    reg_cfg_cmd[5].u.dmi_info.len = sizeof(PCA_RollOffRamTable);

    /* reset dmi to no bank*/
    reg_cfg_cmd[6].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[5].u.dmi_info.lo_tbl_offset + reg_cfg_cmd[5].u.dmi_info.len;
    reg_cfg_cmd[6].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[6].u.rw_info.reg_offset = ISP32_DMI_CFG_OFF;
    reg_cfg_cmd[6].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[7].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[6].u.rw_info.cmd_data_offset + reg_cfg_cmd[6].u.rw_info.len;
    reg_cfg_cmd[7].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[7].u.rw_info.reg_offset = ISP32_DMI_ADDR;
    reg_cfg_cmd[7].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[8].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[7].u.rw_info.cmd_data_offset + reg_cfg_cmd[7].u.rw_info.len;
    reg_cfg_cmd[8].cmd_type = VFE_WRITE;
    reg_cfg_cmd[8].u.rw_info.reg_offset = ISP_PCA_ROLLOFF32_CFG_OFF_0;
    reg_cfg_cmd[8].u.rw_info.len = ISP_PCA_ROLLOFF32_CFG_LEN_0 * sizeof(uint32_t);

    reg_cfg_cmd[9].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[8].u.rw_info.cmd_data_offset + reg_cfg_cmd[8].u.rw_info.len;
    reg_cfg_cmd[9].cmd_type = VFE_WRITE;
    reg_cfg_cmd[9].u.rw_info.reg_offset = ISP_PCA_ROLLOFF32_CFG_OFF_1;
    reg_cfg_cmd[9].u.rw_info.len = ISP_PCA_ROLLOFF32_CFG_LEN_1 * sizeof(uint32_t);

    pca_rolloff_cmd_debug(&pca_rolloff->pca_rolloff_cmd);
    rc = ioctl(pca_rolloff->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    memcpy(&pca_rolloff->mesh_hw,&pca_rolloff->rolloff_tbls.left[0], sizeof(tintless_mesh_rolloff_array_t));
    pca_rolloff->pca_rolloff_cmd.CfgParams.pcaLutBankSel ^= 1;
    pca_rolloff->hw_update_pending = 0;
  }
  return rc;
}

/* ============================================================
 * function name: pca_rolloff_action
 * description: processing the action
 * ============================================================*/
static int pca_rolloff_action (void *mod_ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_pca_rolloff_mod_t *pca_rolloff = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = pca_rolloff_do_hw_update(pca_rolloff);
    break;
  default:
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
    break;
  }
  return rc;
}

/* ============================================================
 * function name: pca_rolloff32_open
 * description: open pca_rolloff
 * ============================================================*/
isp_ops_t *pca_rolloff32_open(uint32_t version)
{
  isp_pca_rolloff_mod_t *pca_rolloff = malloc(sizeof(isp_pca_rolloff_mod_t));

  if (!pca_rolloff) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(pca_rolloff,  0,  sizeof(isp_pca_rolloff_mod_t));
  pca_rolloff->ops.ctrl = (void *)pca_rolloff;
  pca_rolloff->ops.init = pca_rolloff_init;
  /* destroy the module object */
  pca_rolloff->ops.destroy = pca_rolloff_destroy;
  /* set parameter */
  pca_rolloff->ops.set_params = pca_rolloff_set_params;
  /* get parameter */
  pca_rolloff->ops.get_params = pca_rolloff_get_params;
  pca_rolloff->ops.action = pca_rolloff_action;
  return &pca_rolloff->ops;
}





