/*============================================================================
   Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/* #include "vpe_api.h" */
#include "vpe_util.h"
#include "vpe_proc.h"
/* -----------------------------------------------------------------------
** Static Variable
** ----------------------------------------------------------------------- */

/* -------- Downscale, ranging from 0.8x to 20.0x of original size -------- */
int16_t my_vpe_scale_0p8_to_20p0_C0[MM_VPE_SCALE_COEFF_NUM] =
{
  0,   -7,  -13, -19, -24, -28, -32, -34, -37, -39,
  -40, -41, -41, -41, -40, -40, -38, -37, -35, -33,
  -31, -29, -26, -24, -21, -18, -15, -13, -10, -7,
  -5,  -2
};

int16_t my_vpe_scale_0p8_to_20p0_C1[MM_VPE_SCALE_COEFF_NUM] =
{
  511, 507, 501, 494, 485, 475, 463, 450, 436, 422,
  405, 388, 370, 352, 333, 314, 293, 274, 253, 233,
  213, 193, 172, 152, 133, 113, 95,  77,  60,  43,
  28,  13
};

int16_t my_vpe_scale_0p8_to_20p0_C2[MM_VPE_SCALE_COEFF_NUM] =
{
  0,   13,  28,  43,  60,  77,  95,  113, 133, 152,
  172, 193, 213, 233, 253, 274, 294, 314, 333, 352,
  370, 388, 405, 422, 436, 450, 463, 475, 485, 494,
  501, 507,
};

int16_t my_vpe_scale_0p8_to_20p0_C3[MM_VPE_SCALE_COEFF_NUM] =
{
  0,   -2,  -5,  -7,  -10, -13, -15, -18, -21, -24,
  -26, -29, -31, -33, -35, -37, -38, -40, -40, -41,
  -41, -41, -40, -39, -37, -34, -32, -28, -24, -19,
  -13, -7
};

/* -------- Downscale, ranging from 0.6x to 0.8x of original size -------- */
int16_t my_vpe_scale_0p6_to_0p8_C0[MM_VPE_SCALE_COEFF_NUM] =
{
  104, 96,  89,  82,  75,  68,  61,  55,  49,  43,
  38,  33,  28,  24,  20,  16,  12,  9,   6,   4,
  2,   0,   -2,  -4,  -5,  -6,  -7,  -7,  -8,  -8,
  -8,  -8
};

int16_t my_vpe_scale_0p6_to_0p8_C1[MM_VPE_SCALE_COEFF_NUM] =
{
  303, 303, 302, 300, 298, 296, 293, 289, 286, 281,
  276, 270, 265, 258, 252, 245, 238, 230, 223, 214,
  206, 197, 189, 180, 172, 163, 154, 145, 137, 128,
  120, 112
};

int16_t my_vpe_scale_0p6_to_0p8_C2[MM_VPE_SCALE_COEFF_NUM] =
{
  112, 120, 128, 137, 145, 154, 163, 172, 180, 189,
  197, 206, 214, 223, 230, 238, 245, 252, 258, 265,
  270, 276, 281, 286, 289, 293, 296, 298, 300, 302,
  303, 303
};

int16_t my_vpe_scale_0p6_to_0p8_C3[MM_VPE_SCALE_COEFF_NUM] =
{
  -8,  -8,  -8,  -8,  -7,  -7,  -6,  -5,  -4,  -2,
  0,   2,   4,   6,   9,   12,  16,  20,  24,  28,
  33,  38,  43,  49,  55,  61,  68,  75,  82,  89,
  96,  104
};

/* -------- Downscale, ranging from 0.4x to 0.6x of original size -------- */
int16_t my_vpe_scale_0p4_to_0p6_C0[MM_VPE_SCALE_COEFF_NUM] =
{
  136, 132, 128, 123, 119, 115, 111, 107, 103, 98,
  95,  91,  87,  84,  80,  76,  73,  69,  66,  62,
  59,  57,  54,  50,  47,  44,  41,  39,  36,  33,
  32,  29
};

int16_t my_vpe_scale_0p4_to_0p6_C1[MM_VPE_SCALE_COEFF_NUM] =
{
  206, 205, 204, 204, 201, 200, 199, 197, 196, 194,
  191, 191, 189, 185, 184, 182, 180, 178, 176, 173,
  170, 168, 165, 162, 160, 157, 155, 152, 148, 146,
  142, 140
};

int16_t my_vpe_scale_0p4_to_0p6_C2[MM_VPE_SCALE_COEFF_NUM] =
{
  140, 142, 146, 148, 152, 155, 157, 160, 162, 165,
  168, 170, 173, 176, 178, 180, 182, 184, 185, 189,
  191, 191, 194, 196, 197, 199, 200, 201, 204, 204,
  205, 206
};

int16_t my_vpe_scale_0p4_to_0p6_C3[MM_VPE_SCALE_COEFF_NUM] =
{
  29,  32,  33,  36,  39,  41,  44,  47,  50,  54,
  57,  59,  62,  66,  69,  73,  76,  80,  84,  87,
  91,  95,  98,  103, 107, 111, 115, 119, 123, 128,
  132, 136
};

/* -------- Downscale, ranging from 0.2x to 0.4x of original size -------- */
int16_t my_vpe_scale_0p2_to_0p4_C0[MM_VPE_SCALE_COEFF_NUM] =
{
  131, 131, 130, 129, 128, 127, 127, 126, 125, 125,
  124, 123, 123, 121, 120, 119, 119, 118, 117, 117,
  116, 115, 115, 114, 113, 112, 111, 110, 109, 109,
  108, 107
};

int16_t my_vpe_scale_0p2_to_0p4_C1[MM_VPE_SCALE_COEFF_NUM] =
{
  141, 140, 140, 140, 140, 139, 138, 138, 138, 137,
  137, 137, 136, 137, 137, 137, 136, 136, 136, 135,
  135, 135, 134, 134, 134, 134, 134, 133, 133, 132,
  132, 132
};

int16_t my_vpe_scale_0p2_to_0p4_C2[MM_VPE_SCALE_COEFF_NUM] =
{
  132, 132, 132, 133, 133, 134, 134, 134, 134, 134,
  135, 135, 135, 136, 136, 136, 137, 137, 137, 136,
  137, 137, 137, 138, 138, 138, 139, 140, 140, 140,
  140, 141
};

int16_t my_vpe_scale_0p2_to_0p4_C3[MM_VPE_SCALE_COEFF_NUM] =
{
  107, 108, 109, 109, 110, 111, 112, 113, 114, 115,
  115, 116, 117, 117, 118, 119, 119, 120, 121, 123,
  123, 124, 125, 125, 126, 127, 127, 128, 129, 130,
  131, 131
};

static void mm_vpe_update_scale_table(int index, int16_t *c0, int16_t *c1,
  int16_t *c2, int16_t *c3,
  mm_vpe_scale_coef_cfg_type *pvpecmd) {
  int32_t i;
  int32_t *pcoef;

  pvpecmd->offset = index;
  pcoef = &pvpecmd->coef[0];

  for (i = 0; i < MM_VPE_SCALE_COEFF_NUM; i++) {
    *pcoef++ = ((MM_VPE_SCALE_COEFF_MASK & c1[i]) << 16) |
      (MM_VPE_SCALE_COEFF_MASK & c0[i]);
    *pcoef++ = ((MM_VPE_SCALE_COEFF_MASK & c3[i]) << 16) |
      (MM_VPE_SCALE_COEFF_MASK & c2[i]);
  }
}

void mm_vpe_init_scale_table(mm_vpe_scale_coef_cfg_type*  pvpecmd,
  MM_VPE_SCALE_TABLE_ENUM index) {
  switch (index) {
    case MM_VPE_SCALE_TABLE_0:
      mm_vpe_update_scale_table (MM_VPE_SCALE_0P2_TO_0P4_INDEX,
        my_vpe_scale_0p2_to_0p4_C0,
        my_vpe_scale_0p2_to_0p4_C1,
        my_vpe_scale_0p2_to_0p4_C2,
        my_vpe_scale_0p2_to_0p4_C3,
        pvpecmd);
      break;

    case MM_VPE_SCALE_TABLE_1:
      mm_vpe_update_scale_table(MM_VPE_SCALE_0P4_TO_0P6_INDEX,
        my_vpe_scale_0p4_to_0p6_C0,
        my_vpe_scale_0p4_to_0p6_C1,
        my_vpe_scale_0p4_to_0p6_C2,
        my_vpe_scale_0p4_to_0p6_C3,
        pvpecmd);
      break;

    case MM_VPE_SCALE_TABLE_2:
      mm_vpe_update_scale_table(MM_VPE_SCALE_0P6_TO_0P8_INDEX,
        my_vpe_scale_0p6_to_0p8_C0,
        my_vpe_scale_0p6_to_0p8_C1,
        my_vpe_scale_0p6_to_0p8_C2,
        my_vpe_scale_0p6_to_0p8_C3,
        pvpecmd);
      break;

    case MM_VPE_SCALE_TABLE_3:
      mm_vpe_update_scale_table(MM_VPE_SCALE_0P8_TO_8P0_INDEX,
        my_vpe_scale_0p6_to_0p8_C0,
        my_vpe_scale_0p6_to_0p8_C1,
        my_vpe_scale_0p6_to_0p8_C2,
        my_vpe_scale_0p6_to_0p8_C3,
        pvpecmd);
      break;
    default:
      break;
  }
}
