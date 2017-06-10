/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <string.h>
#include <stdlib.h>
#include "camera_dbg.h"
#include <inttypes.h>
#include "vfe_tgtcommon.h"
#include "pca_rolloff.h"
#include "mlro.h"

#define PCA_ROLLOFF_TABLE_DEBUG 0

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
  #undef CDBG
  #define CDBG LOGE
#endif

#define PCA_TBL_INTERPOLATE(in1, in2, out, ratio, isize, i, jsize, j) \
({ \
  for (i=0; i<isize; i++) \
    for (j=0; j<jsize; j++) \
      out[i][j] = LINEAR_INTERPOLATION(in1[i][j], in2[i][j], ratio); \
})

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

  CDBG("%s: PCA Bases Table\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_BASE; i++)
    CDBG("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      in->PCA_basis_table[i][0], in->PCA_basis_table[i][1],
      in->PCA_basis_table[i][2], in->PCA_basis_table[i][3],
      in->PCA_basis_table[i][4], in->PCA_basis_table[i][5],
      in->PCA_basis_table[i][6], in->PCA_basis_table[i][7],
      in->PCA_basis_table[i][8], in->PCA_basis_table[i][9],
      in->PCA_basis_table[i][10], in->PCA_basis_table[i][11],
      in->PCA_basis_table[i][12], in->PCA_basis_table[i][13],
      in->PCA_basis_table[i][14], in->PCA_basis_table[i][15],
      in->PCA_basis_table[i][16]);

  CDBG("%s: PCA Channel R Coeff tables\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    CDBG("%f %f %f %f %f %f %f %f\n",
      in->coeff_table_R[i][0], in->coeff_table_R[i][1],
      in->coeff_table_R[i][2], in->coeff_table_R[i][3],
      in->coeff_table_R[i][4], in->coeff_table_R[i][5],
      in->coeff_table_R[i][6], in->coeff_table_R[i][7]);
  }
  if (compare) {
    MATRIX_MULT(in->coeff_table_R, in->PCA_basis_table, mesh, 13, 8, 17);
    MESH_MATRIX_DIFF(orig_mesh->r_gain, mesh);
  }

  CDBG("%s: PCA Channel GR Coeff tables\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    CDBG("%f %f %f %f %f %f %f %f\n",
      in->coeff_table_Gr[i][0], in->coeff_table_Gr[i][1],
      in->coeff_table_Gr[i][2], in->coeff_table_Gr[i][3],
      in->coeff_table_Gr[i][4], in->coeff_table_Gr[i][5],
      in->coeff_table_Gr[i][6], in->coeff_table_Gr[i][7]);
  }
  if (compare) {
    MATRIX_MULT(in->coeff_table_Gr, in->PCA_basis_table, mesh, 13, 8, 17);
    MESH_MATRIX_DIFF(orig_mesh->gr_gain, mesh);
  }

  CDBG("%s: PCA Channel GB Coeff tables\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    CDBG("%f %f %f %f %f %f %f %f\n",
      in->coeff_table_Gb[i][0], in->coeff_table_Gb[i][1],
      in->coeff_table_Gb[i][2], in->coeff_table_Gb[i][3],
      in->coeff_table_Gb[i][4], in->coeff_table_Gb[i][5],
      in->coeff_table_Gb[i][6], in->coeff_table_Gb[i][7]);
  }
  if (compare) {
    MATRIX_MULT(in->coeff_table_Gb, in->PCA_basis_table, mesh, 13, 8, 17);
    MESH_MATRIX_DIFF(orig_mesh->gb_gain, mesh);
  }

  CDBG("%s: PCA Channel B Coeff tables\n", __func__);
  for (i = 0; i < PCA_ROLLOFF_NUMBER_ROWS; i++) {
    CDBG("%f %f %f %f %f %f %f %f\n",
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

  CDBG("%s: tableOffset = %d\n", __func__, cmd->tableOffset);
  CDBG("%s: pixelOffset=0x%x, pcaLutBankSel=0x%x\n", __func__,
    cmd->CfgParams.pixelOffset, cmd->CfgParams.pcaLutBankSel);
  CDBG("%s: xDelta=0x%x, yDelta=0x%x\n", __func__, cmd->CfgParams.xDelta,
    cmd->CfgParams.yDelta);
  CDBG("%s: gridWidth=0x%x, gridHeight=0x%x\n", __func__,
    cmd->CfgParams.gridWidth, cmd->CfgParams.gridHeight);
  CDBG("%s: xDeltaRight=0x%x, yDeltaRight=0x%x\n", __func__,
    cmd->CfgParams.xDeltaRight, cmd->CfgParams.yDeltaRight);
  CDBG("%s: gridWidthRight=0x%x, gridHeightRight=0x%x\n", __func__,
    cmd->CfgParams.gridWidthRight, cmd->CfgParams.gridHeightRight);
  CDBG("%s: gridXIndex=0x%x, gridYIndex=0x%x, gridPixelXIndex=0x%x, "
    "gridPixelYIndex=0x%x\n", __func__, cmd->CfgParams.gridXIndex,
    cmd->CfgParams.gridYIndex, cmd->CfgParams.gridPixelXIndex,
    cmd->CfgParams.gridPixelYIndex);
  CDBG("%s: yDeltaAccum=0x%x\n", __func__, cmd->CfgParams.yDeltaAccum);

  if (PCA_ROLLOFF_TABLE_DEBUG) {
    for (i=0; i<(PCA_ROLLOFF_BASIS_TABLE_SIZE); i++) {
      CDBG("%s: ram0_bases[%d]=0x%016llx\n", __func__, i,
        cmd->Table.ram0.basisTable[i]);
    }
    for (i=0; i<(PCA_ROLLOFF_BASIS_TABLE_SIZE); i++) {
      CDBG("%s: ram0_coeff[%d]=0x%016llx\n", __func__, i,
        cmd->Table.ram0.coeffTable[i]);
    }
    for (i=0; i<(PCA_ROLLOFF_BASIS_TABLE_SIZE); i++) {
      CDBG("%s: ram1_bases[%d]=0x%016llx\n", __func__, i,
        cmd->Table.ram1.basisTable[i]);
    }
    for (i=0; i<(PCA_ROLLOFF_BASIS_TABLE_SIZE); i++) {
      CDBG("%s: ram1_coeff[%d]=0x%016llx\n", __func__, i,
        cmd->Table.ram1.coeffTable[i]);
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
    out->a0[VFE_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][0]);
    out->a1[VFE_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][1]);
    out->a2[VFE_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][2]);
    out->a3[VFE_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][3]);
    out->a4[VFE_ROLLOFF_CH_R][row] = FLOAT_TO_Q(8, in->coeff_table_R[row][4]);
    out->a5[VFE_ROLLOFF_CH_R][row] = FLOAT_TO_Q(9, in->coeff_table_R[row][5]);
    out->a6[VFE_ROLLOFF_CH_R][row] = FLOAT_TO_Q(9, in->coeff_table_R[row][6]);
    out->a7[VFE_ROLLOFF_CH_R][row] = FLOAT_TO_Q(10, in->coeff_table_R[row][7]);
  }

  /* Prepare Green-Red CoEff Table */
  for (row = 0; row < PCA_ROLLOFF_NUMBER_ROWS; row++) {
    out->a0[VFE_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][0]);
    out->a1[VFE_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][1]);
    out->a2[VFE_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][2]);
    out->a3[VFE_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][3]);
    out->a4[VFE_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(8, in->coeff_table_Gr[row][4]);
    out->a5[VFE_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(9, in->coeff_table_Gr[row][5]);
    out->a6[VFE_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(9, in->coeff_table_Gr[row][6]);
    out->a7[VFE_ROLLOFF_CH_GR][row] = FLOAT_TO_Q(10, in->coeff_table_Gr[row][7]);
  }

  /* Prepare Blue CoEff Table */
  for (row = 0; row < PCA_ROLLOFF_NUMBER_ROWS; row++) {
    out->a0[VFE_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][0]);
    out->a1[VFE_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][1]);
    out->a2[VFE_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][2]);
    out->a3[VFE_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][3]);
    out->a4[VFE_ROLLOFF_CH_B][row] = FLOAT_TO_Q(8, in->coeff_table_B[row][4]);
    out->a5[VFE_ROLLOFF_CH_B][row] = FLOAT_TO_Q(9, in->coeff_table_B[row][5]);
    out->a6[VFE_ROLLOFF_CH_B][row] = FLOAT_TO_Q(9, in->coeff_table_B[row][6]);
    out->a7[VFE_ROLLOFF_CH_B][row] = FLOAT_TO_Q(10, in->coeff_table_B[row][7]);
  }

  /* Prepare Green-Blue CoEff Table */
  for (row = 0; row < PCA_ROLLOFF_NUMBER_ROWS; row++) {
    out->a0[VFE_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][0]);
    out->a1[VFE_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][1]);
    out->a2[VFE_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][2]);
    out->a3[VFE_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][3]);
    out->a4[VFE_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(8, in->coeff_table_Gb[row][4]);
    out->a5[VFE_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(9, in->coeff_table_Gb[row][5]);
    out->a6[VFE_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(9, in->coeff_table_Gb[row][6]);
    out->a7[VFE_ROLLOFF_CH_GB][row] = FLOAT_TO_Q(10, in->coeff_table_Gb[row][7]);
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

  outRam0 = cmd->Table.ram0.basisTable;
  outRam1 = cmd->Table.ram1.basisTable;

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
    case VFE_ROLLOFF_CH_R:
      ch_offset = 0 * PCA_ROLLOFF_NUMBER_ROWS;
      break;
    case VFE_ROLLOFF_CH_GR:
      ch_offset = 1 * PCA_ROLLOFF_NUMBER_ROWS;
      break;
    case VFE_ROLLOFF_CH_B:
      ch_offset = 2 * PCA_ROLLOFF_NUMBER_ROWS;
      break;
    case VFE_ROLLOFF_CH_GB:
      ch_offset = 3 * PCA_ROLLOFF_NUMBER_ROWS;
      break;
    default:
      CDBG_ERROR("%s: Invalid chIndex = %d\n", __func__, chIndex);
      return;
  }
  outRam0 = (cmd->Table.ram0.coeffTable) + ch_offset;
  outRam1 = (cmd->Table.ram1.coeffTable) + ch_offset;

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
  pca_rolloff_prepare_coeff_hw_table(in, cmd, VFE_ROLLOFF_CH_R);
  pca_rolloff_prepare_coeff_hw_table(in, cmd, VFE_ROLLOFF_CH_GR);
  pca_rolloff_prepare_coeff_hw_table(in, cmd, VFE_ROLLOFF_CH_B);
  pca_rolloff_prepare_coeff_hw_table(in, cmd, VFE_ROLLOFF_CH_GB);
} /* pca_rolloff_prepare_hw_table */

/*===========================================================================
 * FUNCTION    - pca_rolloff_update_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void pca_rolloff_update_table(PCA_RollOffConfigCmdType *cmd,
  PCA_RolloffStruct *tableIn, vfe_params_t *vfe_params, int is_left_frame)
{
  uint32_t row, col, camif_width, camif_height;
  uint16_t grid_width, grid_height;
  PCA_RollOffTable Tblcfg;

  camif_width = vfe_params->vfe_input_win.width;
  camif_height = vfe_params->vfe_input_win.height;

  if (vfe_params->cam_mode == CAM_MODE_3D)
    /* TODO: Currently we assume that 3D packing will be Side by Side */
    camif_width /= 2;

  grid_width = (camif_width + 31) / 32;
  grid_height = (camif_height + 23) / 24;

  /* Apply persist config first and then update. */
  memset(&cmd->CfgParams, 0, sizeof(PCA_RollOffConfigParams));

  /* VFE_ROLLOFF_CONFIG */
  cmd->CfgParams.pixelOffset = 0;
  /* Note: Bank selection will be handled in the kernel. */
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
    cmd->CfgParams.yDeltaRight = (1 << 13)/ grid_height;

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

  cmd->tableOffset = is_left_frame ? 0 : 68;

  pca_rolloff_update_basis_table(tableIn, &Tblcfg.basisData);
  pca_rolloff_update_coeff_table(tableIn, &Tblcfg.coeffData);

  pca_rolloff_prepare_hw_table(&(Tblcfg), cmd);
} /* pca_rolloff_update_table */

/*==============================================================================
 * Function:           pca_rolloff_calc_awb_trigger
 *
 * Description:
 *============================================================================*/
static void pca_rolloff_calc_awb_trigger(PCA_RolloffStruct *tblOut,
  vfe_params_t *vfe_params, pca_rolloff_mod_t* pca_rolloff_ctrl,
  int is_left_frame)
{
  float ratio = 0.0;
  cct_trigger_info trigger_info;
  awb_cct_type cct_type;
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  trigger_info.mired_color_temp = MIRED(vfe_params->awb_params.color_temp);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
    chrPtr->rolloff_A_trigger_snapshot);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
    chrPtr->rolloff_D65_trigger_snapshot);

  cct_type = vfe_util_get_awb_cct_type(&trigger_info, vfe_params);

  CDBG("%s: cct_type = %d\n", __func__, cct_type);
  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      if (is_left_frame)
        *tblOut = pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_A_LIGHT];
      else
        *tblOut = pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_A_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84_A:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);

      if (is_left_frame)
        pca_rolloff_table_interpolate(
          &(pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_TL84_LIGHT]),
          &(pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_A_LIGHT]),
          tblOut, ratio);
      else
        pca_rolloff_table_interpolate(
          &(pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_TL84_LIGHT]),
          &(pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_A_LIGHT]),
          tblOut, ratio);
      break;
    case AWB_CCT_TYPE_D65_TL84:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_d65.mired_end,
        trigger_info.trigger_d65.mired_start);

      if (is_left_frame)
        pca_rolloff_table_interpolate(
          &(pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_D65_LIGHT]),
          &(pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_TL84_LIGHT]),
          tblOut, ratio);
      else
        pca_rolloff_table_interpolate(
          &(pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_D65_LIGHT]),
          &(pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_TL84_LIGHT]),
          tblOut, ratio);
      break;
    case AWB_CCT_TYPE_D65:
      if (is_left_frame)
        *tblOut = pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_D65_LIGHT];
      else
        *tblOut = pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_D65_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84:
    default:
      if (is_left_frame)
        *tblOut = pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_TL84_LIGHT];
      else
        *tblOut = pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_TL84_LIGHT];
      break;
  }
} /* pca_rolloff_calc_awb_trigger */

/*==============================================================================
 * FUNCTION    - pca_rolloff_calc_flash_trigger -
 *
 * DESCRIPTION:
 *============================================================================*/
static void pca_rolloff_calc_flash_trigger(PCA_RolloffStruct *tblCCT,
  PCA_RolloffStruct *tblOut, vfe_params_t *vfe_params,
  pca_rolloff_mod_t *pca_rolloff_ctrl, int is_left_frame)
{
  float ratio;
  float flash_start, flash_end;
  PCA_RolloffStruct *tblFlash = NULL;
  vfe_flash_parms_t *flash_params = &(vfe_params->flash_params);
  chromatix_parms_type *chrPtr = vfe_params->chroma3a;

  ratio = flash_params->sensitivity_led_off / flash_params->sensitivity_led_hi;

  if ((int)flash_params->flash_mode == VFE_FLASH_STROBE) {
    flash_start = chrPtr->rolloff_Strobe_start;
    flash_end = chrPtr->rolloff_Strobe_end;

    if (is_left_frame)
      tblFlash =
        &(pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_STROBE_FLASH]);
    else
      tblFlash =
        &(pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_STROBE_FLASH]);
  } else {
    flash_start = chrPtr->rolloff_LED_start;
    flash_end = chrPtr->rolloff_LED_end;

    if (is_left_frame)
      tblFlash =
        &(pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_LED_FLASH]);
    else
      tblFlash =
        &(pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_LED_FLASH]);
  }

  CDBG("%s: flash_start %5.2f flash_end %5.2f \n", __func__, flash_start,
    flash_end);

  if (ratio >= flash_end)
    *tblOut = *tblFlash;
  else if (ratio <= flash_start)
    *tblOut = *tblCCT;
  else
    pca_rolloff_table_interpolate(tblCCT, tblFlash, tblOut,
       ratio/(flash_end - flash_start));
} /* pca_rolloff_calc_flash_trigger */

/*==============================================================================
 * FUNCTION    - pca_rolloff_allocate_scratch_mem -
 *
 * DESCRIPTION:
 *============================================================================*/
static vfe_status_t pca_rolloff_allocate_scratch_mem(double ***out,
  double **temp, int x, int y)
{
  int i;
  *out = (double **)malloc(x * sizeof(double *));
  if (!(*out)) {
    CDBG_ERROR("%s: Not enough memory for out\n", __func__);
    return VFE_ERROR_GENERAL;
  }
  *temp = (double *)malloc((x * y) * sizeof(double));
  if (!(*temp)) {
    CDBG_ERROR("%s: Not enough memory for temp \n", __func__);
    free(*out);
    return VFE_ERROR_GENERAL;
  }
  for (i = 0; i < x; i++)
    (*out)[i] = (*temp) + (i * y);

  return VFE_SUCCESS;
} /* pca_rolloff_allocate_scratch_mem */

/*==============================================================================
 * FUNCTION    - pca_rolloff_convert_tables -
 *
 * DESCRIPTION: This routine genrerates PCA bases and coefficients.
 *============================================================================*/
static vfe_status_t pca_rolloff_convert_tables(vfe_rolloff_info_t *mesh_tbls,
  pca_rolloff_mod_t* pca_rolloff_ctrl, int is_left_frame)
{
  int i, j, k, x, y;
  int w = 17, h = 13, nbases = 8, nch = 4, ntbl = 6;
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

  if (VFE_SUCCESS != pca_rolloff_allocate_scratch_mem(&illu_tbls, &scratch1,
    x, y)) {
    CDBG_ERROR("%s: pca_rolloff_allocate_scratch_mem for illu_tbls failed.",
      __func__);
    return VFE_ERROR_GENERAL;
  }

  for(i = 0; i < (ntbl); i++) {
    if (is_left_frame)
      temp = &(mesh_tbls->left[i]);
    else
      temp = &(mesh_tbls->right[i]);
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
  if (VFE_SUCCESS != pca_rolloff_allocate_scratch_mem(&bases, &scratch2, x, y)) {
    CDBG_ERROR("%s: pca_rolloff_allocate_scratch_mem for bases failed.",
      __func__);
    free(scratch1);
    free(illu_tbls);
    return VFE_ERROR_GENERAL;
  }

  /* allocate memory for TL84, D65, A, Low light, LED, Strobe PCA coefficients */
  x = nbases;
  y = h * nch * (ntbl);
  if (VFE_SUCCESS != pca_rolloff_allocate_scratch_mem(&illu_coeffs, &scratch3,
    x, y)) {
    CDBG_ERROR("%s: pca_rolloff_allocate_scratch_mem for illu_coeffs failed.",
      __func__);
    free(scratch1);
    free(illu_tbls);
    free(scratch2);
    free(bases);
    return VFE_ERROR_GENERAL;
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
    return VFE_ERROR_GENERAL;
  }

  /* Write PCA bases and coefficients of TL84, D65, A, Low light, LED, Strobe */
  for (i = 0; i < (ntbl); i++) {
    if (is_left_frame)
      dest = &(pca_rolloff_ctrl->pca_tbls.left_table[i]);
    else
      dest = &(pca_rolloff_ctrl->pca_tbls.right_table[i]);

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

  return VFE_SUCCESS;
} /* pca_rolloff_convert_tables */

/*==============================================================================
 * Function:           pca_rolloff_init
 *
 * Description:
 *============================================================================*/
vfe_status_t pca_rolloff_init(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_rolloff_info_t *mesh_tbls, vfe_params_t* vfe_params)
{
  int i;
  chromatix_parms_type *chrPtr = NULL;

  pca_rolloff_ctrl->pca_rolloff_enable = TRUE;
  pca_rolloff_ctrl->pca_rolloff_update = TRUE;
  pca_rolloff_ctrl->pca_rolloff_trigger_enable = TRUE;
  pca_rolloff_ctrl->pca_rolloff_reload_params = FALSE;

  if (VFE_SUCCESS != pca_rolloff_convert_tables(mesh_tbls, pca_rolloff_ctrl,
    TRUE)) {
    CDBG_HIGH("%s: Left Mesh to PCA failed. Disable rollOff\n", __func__);
    pca_rolloff_ctrl->pca_rolloff_enable = FALSE;
    return VFE_ERROR_GENERAL;
  }
  if (vfe_params->cam_mode == CAM_MODE_3D) {
    if (VFE_SUCCESS != pca_rolloff_convert_tables(mesh_tbls, pca_rolloff_ctrl,
      FALSE)) {
      CDBG_HIGH("%s: Right Mesh to PCA failed. Disable rollOff\n", __func__);
      pca_rolloff_ctrl->pca_rolloff_enable = FALSE;
      return VFE_ERROR_GENERAL;
    }
  }

  if (PCA_ROLLOFF_TABLE_DEBUG) {
    for (i = VFE_ROLLOFF_TL84_LIGHT; i < VFE_ROLLOFF_MAX_LIGHT; i++) {
      CDBG("%s: PCA Rolloff left table %d values\n", __func__, i);
      pca_rolloff_table_debug(&(pca_rolloff_ctrl->pca_tbls.left_table[i]),
        1, &(mesh_tbls->left[i]));
    }
    if (vfe_params->cam_mode == CAM_MODE_3D) {
      for (i = VFE_ROLLOFF_TL84_LIGHT; i < VFE_ROLLOFF_MAX_LIGHT; i++) {
        CDBG("%s: PCA Rolloff right table %d values\n", __func__, i);
        pca_rolloff_table_debug(&(pca_rolloff_ctrl->pca_tbls.right_table[i]),
          0, NULL);
      }
    }
  }

  pca_rolloff_ctrl->pca_rolloff_prev_param.left_input_table =
    pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_TL84_LIGHT];
  pca_rolloff_ctrl->pca_rolloff_prev_param.right_input_table =
    pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_TL84_LIGHT];
  pca_rolloff_ctrl->pca_rolloff_snap_param.left_input_table =
    pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_TL84_LIGHT];
  pca_rolloff_ctrl->pca_rolloff_snap_param.right_input_table =
    pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_TL84_LIGHT];

  return VFE_SUCCESS;
} /* pca_rolloff_init */

/*==============================================================================
 * Function:           pca_rolloff_config
 *
 * Description:
 *============================================================================*/
vfe_status_t pca_rolloff_config(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  PCA_RollOffConfigCmdType *cmd = NULL;
  PCA_RollOffConfigCmdType cmdRight;
  pca_rolloff_params_t *pcaRolloffTableCur = NULL;

  if (!pca_rolloff_ctrl->pca_rolloff_enable) {
    CDBG("%s: PCA Rolloff is disabled. Skip the config.\n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    if (!pca_rolloff_ctrl->pca_rolloff_trigger_enable) {
      CDBG("%s: Snapshot should have the same config as Preview\n", __func__);
      cmd = &(pca_rolloff_ctrl->pca_rolloff_prev_cmd);
      goto hw_cmd_send;
    }
    if (!pca_rolloff_ctrl->pca_rolloff_update)
      CDBG_HIGH("%s: Trigger should be valid before snapshot config is called"
        "Disabling Roll-off for snapshot\n", __func__);
    cmd = &(pca_rolloff_ctrl->pca_rolloff_snap_cmd);
    pcaRolloffTableCur = &(pca_rolloff_ctrl->pca_rolloff_snap_param);
  } else {
    cmd = &(pca_rolloff_ctrl->pca_rolloff_prev_cmd);
    pcaRolloffTableCur = &(pca_rolloff_ctrl->pca_rolloff_prev_param);
  }

  /* Left frame update */
  pca_rolloff_update_table(cmd, &(pcaRolloffTableCur->left_input_table),
    vfe_params, TRUE);
  /* Right frame update */
  if (vfe_params->cam_mode == CAM_MODE_3D) {
    memcpy(&cmdRight, cmd, sizeof(uint32_t) + sizeof(PCA_RollOffConfigParams));
    pca_rolloff_update_table(&cmdRight, &(pcaRolloffTableCur->right_input_table),
      vfe_params, FALSE);
  }

hw_cmd_send:
  CDBG("%s: Left frame config\n", __func__);
  pca_rolloff_cmd_debug(cmd);
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(PCA_RollOffConfigCmdType),
    VFE_CMD_PCA_ROLL_OFF_CFG)) {
    CDBG_HIGH("%s: L frame config for operation mode = %d failed\n",
      __func__, vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  if (vfe_params->cam_mode == CAM_MODE_3D) {
    CDBG("%s: Right frame config\n", __func__);
    pca_rolloff_cmd_debug(&cmdRight);
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *) &cmdRight, sizeof(PCA_RollOffConfigCmdType),
      VFE_CMD_PCA_ROLL_OFF_CFG)) {
      CDBG_HIGH("%s: R frame config for operation mode = %d failed\n",
        __func__, vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
    }
  }

  return VFE_SUCCESS;
} /* pca_rolloff_config */

/*==============================================================================
 * Function:           pca_rolloff_update
 *
 * Description:
 *============================================================================*/
vfe_status_t pca_rolloff_update(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  PCA_RollOffConfigCmdType *cmd = NULL;
  PCA_RollOffConfigCmdType cmdRight;
  pca_rolloff_params_t *pcaRolloffTableCur = NULL;

  if (!pca_rolloff_ctrl->pca_rolloff_enable) {
    CDBG("%s: PCA Rolloff is disabled. Skip the config.\n", __func__);
    return VFE_SUCCESS;
  }

  if (!pca_rolloff_ctrl->pca_rolloff_update) {
    CDBG("%s: No update required.\n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    CDBG_HIGH("%s: Should not come here\n", __func__);
    cmd = &(pca_rolloff_ctrl->pca_rolloff_snap_cmd);
    pcaRolloffTableCur = &(pca_rolloff_ctrl->pca_rolloff_snap_param);
  } else {
    cmd = &(pca_rolloff_ctrl->pca_rolloff_prev_cmd);
    pcaRolloffTableCur = &(pca_rolloff_ctrl->pca_rolloff_prev_param);
  }

  /* Left frame update */
  pca_rolloff_update_table(cmd, &(pcaRolloffTableCur->left_input_table),
    vfe_params, TRUE);
  /* Right frame update */
  if (vfe_params->cam_mode == CAM_MODE_3D) {
    memcpy(&cmdRight, cmd, sizeof(uint32_t) + sizeof(PCA_RollOffConfigParams));
    pca_rolloff_update_table(&cmdRight, &(pcaRolloffTableCur->right_input_table),
      vfe_params, FALSE);
  }

  CDBG("%s: send the update to RollOff HW\n", __func__);

  CDBG("%s: Left frame config\n", __func__);
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
    (void *) cmd, sizeof(PCA_RollOffConfigCmdType),
    VFE_CMD_PCA_ROLL_OFF_UPDATE)) {
    CDBG_HIGH("%s: L frame update for operation mode = %d failed\n",
      __func__, vfe_params->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }

  if (vfe_params->cam_mode == CAM_MODE_3D) {
    CDBG("%s: Right frame config\n", __func__);
    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *) &cmdRight, sizeof(PCA_RollOffConfigCmdType),
      VFE_CMD_PCA_ROLL_OFF_UPDATE)) {
      CDBG_HIGH("%s: R frame update for operation mode = %d failed\n",
        __func__, vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
    }
  }

  pca_rolloff_ctrl->pca_rolloff_update = FALSE;
  vfe_params->update |= VFE_MOD_ROLLOFF;

  return VFE_SUCCESS;
} /* pca_rolloff_update */

/*==============================================================================
 * Function:           pca_rolloff_trigger_update
 *
 * Description:
 *============================================================================*/
vfe_status_t pca_rolloff_trigger_update(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params)
{
  float aec_ratio = 0.0;
  static float cur_real_gain = 1.0;
  static float cur_lux_idx = 1.0;
  static float cur_mired_color_temp = 1.0;
  static vfe_flash_type cur_flash_mode = VFE_FLASH_NONE;
  float new_real_gain = cur_real_gain;
  float new_lux_idx;
  float new_mired_color_temp;
  vfe_flash_type new_flash_mode;
  chromatix_parms_type *chrPtr = NULL;
  PCA_RolloffStruct *pcaRolloffLeftTableFinal = NULL;
  PCA_RolloffStruct *pcaRolloffRightTableFinal = NULL;
  PCA_RolloffStruct pcaRolloffLeftTableCCT;
  PCA_RolloffStruct pcaRolloffRightTableCCT;

  pca_rolloff_ctrl->pca_rolloff_update = FALSE;
  if (!pca_rolloff_ctrl->pca_rolloff_enable) {
    CDBG("%s: PCA Rolloff is disabled. Skip the trigger.\n", __func__);
    return VFE_SUCCESS;
  }

  if (!pca_rolloff_ctrl->pca_rolloff_trigger_enable) {
    CDBG("%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return VFE_SUCCESS;
  }

  if (IS_SNAP_MODE(vfe_params)) {
    /* Left frame */
    pcaRolloffLeftTableFinal =
      &(pca_rolloff_ctrl->pca_rolloff_snap_param.left_input_table);
    *pcaRolloffLeftTableFinal =
      pca_rolloff_ctrl->pca_rolloff_prev_param.left_input_table;
    /* Right frame */
    if (vfe_params->cam_mode == CAM_MODE_3D) {
      pcaRolloffRightTableFinal =
        &(pca_rolloff_ctrl->pca_rolloff_snap_param.right_input_table);
      *pcaRolloffRightTableFinal =
        pca_rolloff_ctrl->pca_rolloff_prev_param.right_input_table;
    }
    new_real_gain = vfe_params->aec_params.snapshot_real_gain;
  } else {
    /* Left frame */
    pcaRolloffLeftTableFinal =
      &(pca_rolloff_ctrl->pca_rolloff_prev_param.left_input_table);
    /* Right frame */
    if (vfe_params->cam_mode == CAM_MODE_3D)
      pcaRolloffRightTableFinal =
        &(pca_rolloff_ctrl->pca_rolloff_prev_param.right_input_table);
    if (!vfe_util_aec_check_settled(&(vfe_params->aec_params))) {
      if (!pca_rolloff_ctrl->pca_rolloff_reload_params) {
        CDBG("%s: AEC is not setteled. Skip the trigger\n", __func__);
        return VFE_SUCCESS;
      }
    }
    new_real_gain = vfe_params->aec_params.cur_real_gain;
  }

  chrPtr = vfe_params->chroma3a;
  new_lux_idx = vfe_params->aec_params.lux_idx;
  new_flash_mode = vfe_params->flash_params.flash_mode;
  new_mired_color_temp = MIRED(vfe_params->awb_params.color_temp);

  if ((cur_real_gain == new_real_gain) && (cur_lux_idx == new_lux_idx) &&
    (cur_mired_color_temp == new_mired_color_temp) &&
    (cur_flash_mode == new_flash_mode) &&
    (!pca_rolloff_ctrl->pca_rolloff_reload_params)) {
    CDBG("%s: No change in trigger. Nothing to update\n", __func__);
    return VFE_SUCCESS;
  } else {
    CDBG("%s: trigger is change. Prepare for update.\n", __func__);
    cur_real_gain = new_real_gain;
    cur_lux_idx = new_lux_idx;
    cur_mired_color_temp = new_mired_color_temp;
    cur_flash_mode = new_flash_mode;
    pca_rolloff_ctrl->pca_rolloff_reload_params = FALSE;
  }

  pca_rolloff_ctrl->pca_rolloff_update = TRUE;

  /* Note: AWB's CCT interpolated tables are used regardeless Flash
   *       is on or not. So derive them before checking Flash on or not. */
  /* Left frame */
  pca_rolloff_calc_awb_trigger(&pcaRolloffLeftTableCCT, vfe_params,
    pca_rolloff_ctrl, TRUE);
  /* Right frame */
  if (vfe_params->cam_mode == CAM_MODE_3D)
    pca_rolloff_calc_awb_trigger(&pcaRolloffRightTableCCT, vfe_params,
      pca_rolloff_ctrl, FALSE);

  if (new_flash_mode != VFE_FLASH_NONE) {
    /* Left frame */
    pca_rolloff_calc_flash_trigger(&pcaRolloffLeftTableCCT,
      pcaRolloffLeftTableFinal, vfe_params, pca_rolloff_ctrl, TRUE);
    /* Right frame */
    if (vfe_params->cam_mode == CAM_MODE_3D)
      pca_rolloff_calc_flash_trigger(&pcaRolloffRightTableCCT,
        pcaRolloffRightTableFinal, vfe_params, pca_rolloff_ctrl, FALSE);
  } else {
    aec_ratio = vfe_util_get_aec_ratio(chrPtr->control_rolloff,
      &(chrPtr->rolloff_lowlight_trigger), vfe_params);
    if (F_EQUAL(aec_ratio, 0.0)) {
      CDBG("%s: Low Light \n", __func__);
      /* Left frame */
      *pcaRolloffLeftTableFinal =
        pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_LOW_LIGHT];
      /* Right frame */
      if (vfe_params->cam_mode == CAM_MODE_3D)
        *pcaRolloffLeftTableFinal =
          pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_LOW_LIGHT];
    } else if (F_EQUAL(aec_ratio, 1.0)) {
      CDBG("%s: Bright Light \n", __func__);
      /* Left frame */
      *pcaRolloffLeftTableFinal = pcaRolloffLeftTableCCT;
      /* Right frame */
      if (vfe_params->cam_mode == CAM_MODE_3D)
        *pcaRolloffLeftTableFinal = pcaRolloffRightTableCCT;
    } else {
      CDBG("%s: Interpolate between CCT and Low Light \n", __func__);
      /* Left frame */
      pca_rolloff_table_interpolate(&pcaRolloffLeftTableCCT,
        &(pca_rolloff_ctrl->pca_tbls.left_table[VFE_ROLLOFF_LOW_LIGHT]),
        pcaRolloffLeftTableFinal, aec_ratio);
      /* Right frame */
      if (vfe_params->cam_mode == CAM_MODE_3D)
        pca_rolloff_table_interpolate(&pcaRolloffRightTableCCT,
          &(pca_rolloff_ctrl->pca_tbls.right_table[VFE_ROLLOFF_LOW_LIGHT]),
          pcaRolloffRightTableFinal, aec_ratio);
    }
  }

  return VFE_SUCCESS;
} /* pca_rolloff_trigger_update */

/*=============================================================================
 * Function:               pca_rolloff_trigger_enable
 *
 * Description:
 *============================================================================*/
vfe_status_t pca_rolloff_trigger_enable(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params, int8_t enable)
{
  CDBG("%s: new trigger enable value = %d\n", __func__, enable);
  pca_rolloff_ctrl->pca_rolloff_trigger_enable = enable;

  return VFE_SUCCESS;
} /* pca_rolloff_trigger_enable */

/*===========================================================================
 * FUNCTION    - pca_rolloff_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t pca_rolloff_reload_params(pca_rolloff_mod_t* pca_rolloff_ctrl,
  vfe_params_t* vfe_params, vfe_rolloff_info_t *mesh_tbls)
{
  int i;

  CDBG("%s: reload the chromatix\n", __func__);
  if (VFE_SUCCESS != pca_rolloff_convert_tables(mesh_tbls, pca_rolloff_ctrl,
    TRUE)) {
    CDBG_HIGH("%s: Left Mesh to PCA failed. Disable rollOff\n", __func__);
    pca_rolloff_ctrl->pca_rolloff_enable = FALSE;
    return VFE_ERROR_GENERAL;
  }
  if (vfe_params->cam_mode == CAM_MODE_3D) {
    if (VFE_SUCCESS != pca_rolloff_convert_tables(mesh_tbls, pca_rolloff_ctrl,
      FALSE)) {
      CDBG_HIGH("%s: Right Mesh to PCA failed. Disable rollOff\n", __func__);
      pca_rolloff_ctrl->pca_rolloff_enable = FALSE;
      return VFE_ERROR_GENERAL;
    }
  }

  if (PCA_ROLLOFF_TABLE_DEBUG) {
    for (i = VFE_ROLLOFF_TL84_LIGHT; i < VFE_ROLLOFF_MAX_LIGHT; i++) {
      CDBG("%s: PCA Rolloff left table %d values\n", __func__, i);
      pca_rolloff_table_debug(&(pca_rolloff_ctrl->pca_tbls.left_table[i]),
        0, NULL);
    }
    if (vfe_params->cam_mode == CAM_MODE_3D) {
      for (i = VFE_ROLLOFF_TL84_LIGHT; i < VFE_ROLLOFF_MAX_LIGHT; i++) {
        CDBG("%s: PCA Rolloff right table %d values\n", __func__, i);
        pca_rolloff_table_debug(&(pca_rolloff_ctrl->pca_tbls.right_table[i]),
          0, NULL);
      }
    }
  }

  pca_rolloff_ctrl->pca_rolloff_reload_params = TRUE;

  return VFE_SUCCESS;
} /* pca_rolloff_reload_params */

/*===========================================================================
 * FUNCTION    - vfe_PCA_Roll_off_test_vector_validation -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_PCA_Roll_off_test_vector_validation(void * test_input,
  void *test_output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)test_output;
  uint i, j;
  int64_t v0_in, v0_out,v1_in, v1_out, v2_in, v2_out;
  int8_t v3_in, v3_out, v4_in, v4_out,v5_in, v5_out,v6_in, v6_out,v7_in, v7_out;
  uint64_t *InTable = NULL;
  uint64_t *OutTable = NULL;

  OutTable = (uint64_t *)(mod_op->rolloff_table);
  InTable = mod_in->pca_rolloff.ram[0].table;

  CDBG("%s: Ram0 basis table verify v0-v3", __func__);
  for (i=0, j=0; i < PCA_ROLLOFF_BASIS_TABLE_SIZE; i++,j++) {
    if(InTable[i] != OutTable[j]){
      v0_in = (int64_t)((InTable[i] & 0x3ffc000000LL) >> 26);
      v0_out = (int64_t)((OutTable[j] & 0x3ffc000000LL) >> 26);
      if (!MATCH(v0_in, v0_out, 2))
        CDBG_HIGH("%s: %dth v0 mismatch in = %lld, out = %lld",
          __func__, i, v0_in, v0_out);

      v1_in = (int64_t)((InTable[i] & 0x3fe0000) >>17);
      v1_out = (int64_t)((OutTable[j] & 0x3fe0000) >>17);
      v1_in = (v1_in << (64 - 9)) >> (64 - 9);
      v1_out = (v1_out << (64 - 9)) >> (64 - 9);
      if (!MATCH(v1_in, v1_out, 2))
        CDBG_HIGH("%s: %dth v1 mismatch in = %lld, out = %lld",
          __func__, i, v1_in, v1_out);

      v2_in = (int64_t)((InTable[i] & 0x1ff00) >>8);
      v2_out = (int64_t)((OutTable[j] & 0x1ff00) >>8);
      v2_in = (v2_in << (64 - 9)) >> (64 - 9);
      v2_out = (v2_out << (64 - 9)) >> (64 - 9);
      if (!MATCH(v2_in, v2_out, 2))
        CDBG_HIGH("%s: %dth v2 mismatch in = %lld, out = %lld",
          __func__, i, v2_in, v2_out);

      v3_in = (int8_t)((InTable[i] & 0xff) >>0);
      v3_out = (int8_t)((OutTable[j] & 0xff) >>0);
      if (!MATCH(v3_in, v3_out, 2))
        CDBG_HIGH("%s: %dth v3 mismatch in = %d, out = %d",
          __func__, i, v3_in, v3_out);
    }
  }

  CDBG("%s: Ram0 coeff table verify a0-a3", __func__);
  for (; i < mod_in->pca_rolloff.ram[0].size; i++,j++) {
    if(InTable[i] != OutTable[j]){
      v0_in = (int64_t)((InTable[i] & 0x3ffc000000LL) >> 26);
      v0_out = (int64_t)((OutTable[j] & 0x3ffc000000LL) >> 26);
      if (!MATCH(v0_in, v0_out, 2))
        CDBG_HIGH("%s: %dth a0 mismatch in = %lld, out = %lld",
          __func__, i, v0_in, v0_out);

      v1_in = (int64_t)((InTable[i] & 0x3fe0000) >>17);
      v1_out = (int64_t)((OutTable[j] & 0x3fe0000) >>17);
      v1_in = (v1_in << (64 - 9)) >> (64 - 9);
      v1_out = (v1_out << (64 - 9)) >> (64 - 9);
      if (!MATCH(v1_in, v1_out, 2))
        CDBG_HIGH("%s: %dth a1 mismatch in = %lld, out = %lld",
          __func__, i, v1_in, v1_out);

      v2_in = (int64_t)((InTable[i] & 0x1ff00) >>8);
      v2_out = (int64_t)((OutTable[j] & 0x1ff00) >>8);
      v2_in = (v2_in << (64 - 9)) >> (64 - 9);
      v2_out = (v2_out << (64 - 9)) >> (64 - 9);
      if (!MATCH(v2_in, v2_out, 2))
        CDBG_HIGH("%s: %dth a2 mismatchs in = %lld, out = %lld",
          __func__, i, v2_in, v2_out);

      v3_in = (int8_t)((InTable[i] & 0xff) >>0);
      v3_out = (int8_t)((OutTable[j] & 0xff) >>0);
      if (!MATCH(v3_in, v3_out, 2))
        CDBG_HIGH("%s: %dth a3 mismatch in = %d, out = %d",
          __func__, i, v3_in, v3_out);
    }
  }

  InTable = mod_in->pca_rolloff.ram[1].table;

  CDBG("%s: Ram1 basis table verify v4-v7",__func__);
  for (i=0; i < PCA_ROLLOFF_BASIS_TABLE_SIZE; i++, j++) {
    if(InTable[i] != OutTable[j]){
      v4_in = (int8_t)((InTable[i] & 0xff000000) >>24);
      v4_out = (int8_t)((OutTable[j] & 0xff000000) >>24);
      if (!MATCH(v4_in, v4_out, 2))
        CDBG_HIGH("%s: %dth v4 mismatch in = %d, out = %d",
          __func__, i, v4_in, v4_out);

      v5_in = (int8_t)((InTable[i] & 0xff0000) >>16);
      v5_out = (int8_t)((OutTable[j] & 0xff0000) >>16);
      if (!MATCH(v5_in, v5_out, 2))
        CDBG_HIGH("%s: %dth v5 mismatch in = %d, out = %d",
          __func__, i, v5_in, v5_out);

      v6_in = (int8_t)((InTable[i] & 0xff00) >>8);
      v6_out = (int8_t)((OutTable[j] & 0xff00) >>8);
      if (!MATCH(v6_in, v6_out, 2))
        CDBG_HIGH("%s: %dth v6 mismatch in = %d, out = %d",
          __func__, i, v6_in, v6_out);

      v7_in = (int8_t)((InTable[i] & 0xff) >>0);
      v7_out = (int8_t)((OutTable[j] & 0xff) >>0);
      if (!MATCH(v7_in, v7_out, 2))
        CDBG_HIGH("%s: %dth v7 mismatch in = %d, out = %d",
          __func__, i, v7_in, v7_out);
    }
  }

  CDBG("%s: Ram1 coeff table verify a4-a7",__func__);
  for (; i < mod_in->pca_rolloff.ram[1].size; i++, j++) {
    if(InTable[i] != OutTable[j]){
      v4_in = (int8_t)((InTable[i] & 0xff000000) >>24);
      v4_out = (int8_t)((OutTable[j] & 0xff000000) >>24);
      if (!MATCH(v4_in, v4_out, 2))
        CDBG_HIGH("%s: %dth a4 mismatch in = %d, out = %d",
          __func__, i, v4_in, v4_out);

      v5_in = (int8_t)((InTable[i] & 0xff0000) >>16);
      v5_out = (int8_t)((OutTable[j] & 0xff0000) >>16);
      if (!MATCH(v5_in, v5_out, 2))
        CDBG_HIGH("%s: %dth a5 mismatch in = %d, out = %d",
          __func__, i, v5_in, v5_out);

      v6_in = (int8_t)((InTable[i] & 0xff00) >>8);
      v6_out = (int8_t)((OutTable[j] & 0xff00) >>8);
      if (!MATCH(v6_in, v6_out, 2))
        CDBG_HIGH("%s: %dth a6 mismatch in = %d, out = %d",
          __func__, i, v6_in, v6_out);

      v7_in = (int8_t)((InTable[i] & 0xff) >>0);
      v7_out = (int8_t)((OutTable[j] & 0xff) >>0);
      if (!MATCH(v7_in, v7_out, 2))
        CDBG_HIGH("%s: %dth a7 mismatch in = %d, out = %d",
          __func__, i, v7_in, v7_out);
    }
  }
  return VFE_SUCCESS;
} /*vfe_PCA_Roll_off_test_vector_validation*/




