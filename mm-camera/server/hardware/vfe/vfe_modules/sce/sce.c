/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "vfe.h"

#define SWAP(a,b) ({\
  a[0]^=b[0];\
  b[0]^=a[0];\
  a[0]^=b[0];\
  a[1]^=b[1];\
  b[1]^=a[1];\
  a[1]^=b[1];\
})

#ifdef ENABLE_SCE_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

static sce_cr_cb_triangle_set acc_color_green = {
/* SCE Origin Triangle 1 */
  {
    {-64, -64},
    {-10, 0},
    {-128, 0},
  },
/* SCE Origin Triangle 2 */
  {
    {-64, -64},
    {0, -10},
    {-10, 0},
  },
/* SCE Origin Triangle 3 */
  {
    {-64, -64},
    {0, -128},
    {0, -10},
  },
/* SCE Origin Triangle 4 */
  {
    {-64, -64},
    {-128, -128},
    {0, -128},
  },
/* SCE Origin Triangle 5 */
  {
    {-64, -64},
    {-128, 0},
    {-128, -128},
  }
};

static sce_cr_cb_triangle_set acc_color_blue = {
/* SCE Origin Triangle 1 */
  {
    {-64, 64},
    {0, 127},
    {-128, 127},
  },
/* SCE Origin Triangle 2 */
  {
    {-64, 64},
    {0, 15},
    {0, 127},
  },
/* SCE Origin Triangle 3 */
  {
    {-64, 64},
    {-10, 15},
    {0, 15},
  },
/* SCE Origin Triangle 4 */
  {
    {-64, 64},
    {-128, 25},
    {-10, 15},
  },
/* SCE Origin Triangle 5 */
  {
    {-64, 64},
    {-128,127},
    {-128, 25},
  }
};

static sce_cr_cb_triangle_set acc_color_orange = {
/* SCE Origin Triangle 1 */
  {
    {100, -90},
    {30, -10},
    {20, -80},
  },
/* SCE Origin Triangle 2 */
  {
    {100, -90},
    {127, -10},
    {30, -10},
  },
/* SCE Origin Triangle 3 */
  {
    {100, -90},
    {20, -80},
    {75, -128},
  },
/* SCE Origin Triangle 4 */
  {
    {100, -90},
    {75, -128},
    {127, -128},
  },
/* SCE Origin Triangle 5 */
  {
    {100, -90},
    {127, -128},
    {127, -10},
  }
};

/*===========================================================================
 * FUNCTION    - vfe_sce_reorder_vertices -
 *
 * DESCRIPTION: This function updates the mce trigger enable flag
 *==========================================================================*/
void vfe_sce_reorder_vertices(cr_cb_triangle *t)
{
  int a[3][2];

  if((t->point1.cr == t->point2.cr) &&(t->point2.cr == t->point3.cr)) {
    CDBG_ERROR("Points are colinear\n");
  }

  if((t->point1.cb == t->point2.cb) && (t->point2.cb == t->point3.cb)) {
    CDBG_ERROR("Points are colinear\n");
  }

  a[0][0] = t->point1.cr;
  a[0][1] = t->point1.cb;
  a[1][0] = t->point2.cr;
  a[1][1] = t->point2.cb;
  a[2][0] = t->point3.cr;
  a[2][1] = t->point3.cb;

  int i,j = 3;
  int re_order = 0;

  while(j > 0)
  {
    for (i = 1; i < j ; i++ )
    {
      if(a[i-1][1] > a[i][1])
        SWAP(a[i-1], a[i]);
    }
    j--;
  }

  if(a[0][1] == a[1][1]){
    if(a[0][0] > a[1][0]) {
      SWAP(a[0], a[1]);
    }
  }else if(a[1][0] < a[2][0]) {
    SWAP(a[1], a[2]);
  }
  else if(a[1][0] == a[2][0] && a[1][1] > a[2][1])
    SWAP(a[1], a[2]);
  else
    re_order = 1;

  t->point1.cr = a[0][0];
  t->point1.cb = a[0][1];
  t->point2.cr = a[1][0];
  t->point2.cb = a[1][1];
  t->point3.cr = a[2][0];
  t->point3.cb = a[2][1];
}

/*===========================================================================
 * Function:           vfe_reorder_sce_matrix
 *
 * Description:
 *=========================================================================*/
void vfe_sce_reorder_triangles(sce_mod_t *sce_mod, vfe_params_t* params)
{
  chromatix_parms_type *p_chx = params->chroma3a;
  sce_mod->origin_triangles_A = p_chx->origin_triangles_A;
  sce_mod->destination_triangles_A = p_chx->destination_triangles_A;
  sce_mod->origin_triangles_D65 = p_chx->origin_triangles_D65;
  sce_mod->destination_triangles_D65 = p_chx->destination_triangles_D65;
  sce_mod->origin_triangles_TL84 = p_chx->origin_triangles_TL84;
  sce_mod->destination_triangles_TL84 = p_chx->destination_triangles_TL84;

  CDBG("%s:\n", __func__);
#ifdef FEATURE_VFE_TEST_VECTOR
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_A.triangle1));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_A.triangle2));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_A.triangle3));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_A.triangle4));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_A.triangle5));

  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_A.triangle1));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_A.triangle2));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_A.triangle3));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_A.triangle4));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_A.triangle5));

  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_D65.triangle1));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_D65.triangle2));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_D65.triangle3));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_D65.triangle4));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_D65.triangle5));

  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_D65.triangle1));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_D65.triangle2));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_D65.triangle3));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_D65.triangle4));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_D65.triangle5));

  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_TL84.triangle1));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_TL84.triangle2));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_TL84.triangle3));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_TL84.triangle4));
  vfe_sce_reorder_vertices(&(sce_mod->origin_triangles_TL84.triangle5));

  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_TL84.triangle1));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_TL84.triangle2));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_TL84.triangle3));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_TL84.triangle4));
  vfe_sce_reorder_vertices(&(sce_mod->destination_triangles_TL84.triangle5));
#endif
}

/*===========================================================================
 * FUNCTION    - vfe_calc_sce_shiftbits -
 *
 * DESCRIPTION: calculate the right shift bits for the transformation matrix and
 *              offset operation for triangle n
 *==========================================================================*/
static void vfe_calc_sce_shiftbits(sce_affine_transform_2d *coeff,
  uint32_t *matrix_shift, uint32_t *offset_shift)
{
  double temp1, temp2;
  double A,B,C,D,E,F;
  uint32_t nMaxQ2, nMaxQ12;
  /* A, B, D, E is 12S in Hw, so the bits for abs of affine transform is 11 */
  uint32_t nBitsForRotScaleInHw = 11;
  /* C, F is 17S in Hw, so the bits for abs of offset shift(Q2) is 16 */
  uint32_t nBitsForTranslationInHw = 16;

  A = fabs(coeff->a);
  B = fabs(coeff->b);
  C = fabs(coeff->c);
  D = fabs(coeff->d);
  E = fabs(coeff->e);
  F = fabs(coeff->f);

  /* Calculate Q2 */
  temp1 = (C >= F) ? C : F;
  /* number of bits required to store the integer part of max(C,F) =
     log2(ceil(max(C,F)))+1 */
  /* nMaxQ2 is the number of bits to store the decimal part */
  if ((int32_t)temp1 == 0)
    nMaxQ2 = nBitsForTranslationInHw;
  else
    nMaxQ2 = nBitsForTranslationInHw - 1 - (uint32_t)(log(temp1) / log(2.0));

  /* temp1 = max(A, B, D, E) */
  temp1 = (A >= B) ? A : B;
  temp2 = (D >= E) ? D : E;
  temp1 = (temp1 >= temp2) ? temp1 : temp2;

  /* number of bits required to store the integer part of max(A,B,D,E) =
     log2(ceil(max(A,B,D,E)))+1 */

  if ((int32_t)temp1 == 0)
    nMaxQ12 = nBitsForRotScaleInHw;
  else
    nMaxQ12 =  nBitsForRotScaleInHw - 1 - (uint32_t)(log(temp1) / log(2.0));

  /* matrixShift[i] is Q1 */
  /* offsetShift[i] is Q2, the value of Q2 is restricted by
     the overall shift (Q1+Q2) */
  if ( nMaxQ12 <= nMaxQ2) {
    *offset_shift = nMaxQ12;
    *matrix_shift = 0;
  } else {
    *offset_shift = nMaxQ2;
    *matrix_shift = nMaxQ12 - nMaxQ2;
  }
  return;
}

/*===========================================================================
 * FUNCTION    - vfe_calc_sce_newendpoint -
 *
 * DESCRIPTION:  Calculate the new vertex based on the control factor
 *==========================================================================*/
static void vfe_calc_sce_newendpoint(double *rEnd_Cr, double *rEnd_Cb,
  const double Start_Cr, const double Start_Cb, double adj_fac)
{
  CDBG("%s:\n",__func__);
  CDBG("adj_fac : %lf\n",adj_fac);
  CDBG("rEnd_cr : %lf, start_cr : %lf \n",*rEnd_Cr,Start_Cr);
  CDBG("rEnd_cb : %lf, start_cb : %lf \n",*rEnd_Cb,Start_Cb);

  *rEnd_Cr = adj_fac * (*rEnd_Cr - Start_Cr) + Start_Cr;
  *rEnd_Cb = adj_fac * (*rEnd_Cb - Start_Cb) + Start_Cb;

  *rEnd_Cr = MAX(*rEnd_Cr,-128);
  *rEnd_Cr = MIN(*rEnd_Cr, 127);

  *rEnd_Cb = MAX(*rEnd_Cb,-128);
  *rEnd_Cb = MIN(*rEnd_Cb, 127);

  CDBG("Final rEnd_cr : %lf, rEnd_cb : %lf\n",*rEnd_Cr,*rEnd_Cb);
}

/*===========================================================================
 * FUNCTION    - vfe_calc_sce_transform -
 *
 * DESCRIPTION: calculate the new co-efficents and the offset bits
 *              for the new traingle
 *==========================================================================*/
static int32_t vfe_calc_sce_transform(cr_cb_triangle *pDestVert,
  cr_cb_triangle *pOrigVert, sce_affine_transform_2d *pTransform,double val)
{
  double M1[9], M2[9], InvM2[9], Tx[9];
  int32_t rc = TRUE, i;

  if (pDestVert == NULL || pOrigVert == NULL || pTransform == NULL){
    CDBG(" Null pointer in vfe_util_sce_transform\n");
    return FALSE;
  }

  /* fill in M1, the dest triangle */
  M1[0] = pDestVert->point1.cr;
  M1[3] = pDestVert->point1.cb;
  M1[1] = pDestVert->point2.cr;
  M1[4] = pDestVert->point2.cb;
  M1[2] = pDestVert->point3.cr;
  M1[5] = pDestVert->point3.cb;
  M1[6] = M1[7] = M1[8] = 1.0;

  /* fill in M2 the original three vertex */
  M2[0] = pOrigVert->point1.cr;
  M2[3] = pOrigVert->point1.cb;
  M2[1] = pOrigVert->point2.cr;
  M2[4] = pOrigVert->point2.cb;
  M2[2] = pOrigVert->point3.cr;
  M2[5] = pOrigVert->point3.cb;
  M2[6] = M2[7] = M2[8] = 1.0;

  /* Calculate the new vertex */
  vfe_calc_sce_newendpoint(&M1[0], &M1[3], M2[0], M2[3], val);

  MATRIX_INVERSE_3x3(M2, InvM2);

  Tx[8] = M1[6]*InvM2[2] + M1[7]*InvM2[5] + M1[8]*InvM2[8];

  if ( Tx [8] != 0 ){
     Tx[0] = (M1[0]*InvM2[0] + M1[1]*InvM2[3] + M1[2]*InvM2[6]) / Tx[8];
     Tx[1] = (M1[0]*InvM2[1] + M1[1]*InvM2[4] + M1[2]*InvM2[7]) / Tx[8];
     Tx[2] = (M1[0]*InvM2[2] + M1[1]*InvM2[5] + M1[2]*InvM2[8]) / Tx[8];
     Tx[3] = (M1[3]*InvM2[0] + M1[4]*InvM2[3] + M1[5]*InvM2[6]) / Tx[8];
     Tx[4] = (M1[3]*InvM2[1] + M1[4]*InvM2[4] + M1[5]*InvM2[7]) / Tx[8];
     Tx[5] = (M1[3]*InvM2[2] + M1[4]*InvM2[5] + M1[5]*InvM2[8]) / Tx[8];
     Tx[6] = (M1[6]*InvM2[0] + M1[7]*InvM2[3] + M1[8]*InvM2[6]) / Tx[8];
     Tx[7] = (M1[6]*InvM2[1] + M1[7]*InvM2[4] + M1[8]*InvM2[7]) / Tx[8];
     Tx[8] = 1;

    for (i = 0; i < 6; i++)
      *((float *)pTransform + i) = (float)(Tx[i]);
  } else
      rc = FALSE;

  return rc;
}

/*===========================================================================
 * FUNCTION    - vfe_calc_sce_mapping -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_calc_sce_mapping (sce_affine_transform_2d *Tx,
  int32_t *coeff,uint32_t *matrix_shift, uint32_t *offset_shift)
{
  uint32_t matrixShift, offsetShift;
  int i;
  float *TxElem = (float *)Tx;

  /* calculate the co-efficients */
  vfe_calc_sce_shiftbits(Tx, matrix_shift, offset_shift);

  for (i = 0; i < 6; i++) {
    if (i%3 == 2) {
      coeff[i] = (int32_t)((*TxElem) * (1<<(*offset_shift)));
      if (coeff[i] <= -65536 || coeff[i] > 65536)
        CDBG("ERROR: -65536 < coeffE <= 65536 is violated\n");
    } else {
      coeff[i] = (int32_t)((*TxElem) * (1<<((*matrix_shift)+(*offset_shift))));
      if (coeff[i] <= -2047 || coeff[i] > 2047)
        CDBG("ERROR: -2047 < coeffA <= 2047 is violated\n");
    }
    TxElem++;
  }
  return;
}

/*===========================================================================
 * FUNCTION    - vfe_calc_sce -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_calc_sce_params(cr_cb_triangle *pDestVert,
  cr_cb_triangle *pOrigVert,double val, int32_t *coeff,
  uint32_t *matrix_shift, uint32_t *offset_shift)
{
  sce_affine_transform_2d Tx;
  if(!vfe_calc_sce_transform(pDestVert,pOrigVert, &Tx, val))
    return;

  vfe_calc_sce_mapping(&Tx, coeff, matrix_shift, offset_shift);
}

/*===========================================================================
 * FUNCTION    - vfe_config_sce_cmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_config_sce_cmd(sce_mod_t *sce_mod, vfe_params_t *vfe_params)
{
  int32_t i;
  int32_t coeff[6];
  uint32_t matrix_shift, offset_shift;
  chromatix_parms_type *chroma_ptr = vfe_params->chroma3a;
  float portrait_severity = 0;
  float portrait_sce =
    chroma_ptr->portrait_scene_detect.skin_color_boost_factor;
  double sce_adj_factor;

  // Not considering max severity for portrait/party
  /*if(vfe_params->bs_mode == CAMERA_BESTSHOT_PORTRAIT ||
      vfe_params->bs_mode == CAMERA_BESTSHOT_PARTY)
      portrait_severity = 255; // max severity
     else
   */
  portrait_severity = (float)vfe_params->asd_params.portrait_severity;

  if (portrait_severity != 0) {
    sce_adj_factor = sce_mod->sce_adjust_factor *
      (1.0 - portrait_severity / 255.0) +
      portrait_sce * (portrait_severity / 255.0);
  } else
    sce_adj_factor = sce_mod->sce_adjust_factor;

  CDBG("%s:\n",__func__);
  CDBG("%s: Portrait severity :%f\n",__func__, portrait_severity);
  CDBG("%s: Portrait adj factor :%f\n",__func__, portrait_sce);
  CDBG("%s: sce_adj_factor :%f\n",__func__, sce_adj_factor);
  /* Cr cordintates for Triangle 0-4 */
  sce_mod->sce_cmd.crcoord.vertex00 =
    sce_mod->orig->triangle1.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex01 =
    sce_mod->orig->triangle1.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex02 =
    sce_mod->orig->triangle1.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex10 =
    sce_mod->orig->triangle2.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex11 =
    sce_mod->orig->triangle2.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex12 =
    sce_mod->orig->triangle2.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex20 =
    sce_mod->orig->triangle3.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex21 =
    sce_mod->orig->triangle3.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex22 =
    sce_mod->orig->triangle3.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex30 =
    sce_mod->orig->triangle4.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex31 =
    sce_mod->orig->triangle4.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex32 =
    sce_mod->orig->triangle4.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex40 =
    sce_mod->orig->triangle5.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex41 =
    sce_mod->orig->triangle5.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex42 =
    sce_mod->orig->triangle5.point3.cr;

/* Cb cordintates for Triangle 0-4 */
  sce_mod->sce_cmd.cbcoord.vertex00 =
    sce_mod->orig->triangle1.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex01 =
    sce_mod->orig->triangle1.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex02 =
    sce_mod->orig->triangle1.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex10 =
    sce_mod->orig->triangle2.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex11 =
    sce_mod->orig->triangle2.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex12 =
    sce_mod->orig->triangle2.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex20 =
    sce_mod->orig->triangle3.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex21 =
    sce_mod->orig->triangle3.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex22 =
    sce_mod->orig->triangle3.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex30 =
    sce_mod->orig->triangle4.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex31 =
    sce_mod->orig->triangle4.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex32 =
    sce_mod->orig->triangle4.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex40 =
    sce_mod->orig->triangle5.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex41 =
    sce_mod->orig->triangle5.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex42 =
    sce_mod->orig->triangle5.point3.cb;

  /* Update VFE with new co-efficients, for triangle 1 */
  vfe_calc_sce_params(&sce_mod->dest->triangle1,
    &sce_mod->orig->triangle1, sce_adj_factor, coeff, &matrix_shift,
    &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef00 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef01 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef00 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef01 = coeff[4];
  sce_mod->sce_cmd.croffset.offset0 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset0 = coeff[5];
  sce_mod->sce_cmd.croffset.shift0 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift0 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 2 */
  vfe_calc_sce_params(&sce_mod->dest->triangle2,
    &sce_mod->orig->triangle2, sce_adj_factor, coeff, &matrix_shift,
    &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef10 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef11 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef10 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef11 = coeff[4];
  sce_mod->sce_cmd.croffset.offset1 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset1 = coeff[5];
  sce_mod->sce_cmd.croffset.shift1 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift1 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 3 */
  vfe_calc_sce_params(&sce_mod->dest->triangle3,
    &sce_mod->orig->triangle3, sce_adj_factor, coeff, &matrix_shift,
    &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef20 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef21 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef20 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef21 = coeff[4];
  sce_mod->sce_cmd.croffset.offset2 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset2 = coeff[5];
  sce_mod->sce_cmd.croffset.shift2 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift2 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 4 */
  vfe_calc_sce_params(&sce_mod->dest->triangle4,
    &sce_mod->orig->triangle4, sce_adj_factor, coeff, &matrix_shift,
    &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef30 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef31 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef30 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef31 = coeff[4];
  sce_mod->sce_cmd.croffset.offset3 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset3 = coeff[5];
  sce_mod->sce_cmd.croffset.shift3 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift3 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 5 */
  vfe_calc_sce_params(&sce_mod->dest->triangle5,
    &sce_mod->orig->triangle5, sce_adj_factor, coeff, &matrix_shift,
    &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef40 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef41 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef40 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef41 = coeff[4];
  sce_mod->sce_cmd.croffset.offset4 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset4 = coeff[5];
  sce_mod->sce_cmd.croffset.shift4 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift4 = offset_shift;

  /* Update VFE with new co-efficients, for outside region mapping */
  vfe_calc_sce_mapping(&chroma_ptr->outside_region_mapping,
    coeff, &matrix_shift, &offset_shift);
  if((sce_mod->active_spl_effect == CAMERA_EFFECT_ACCENT_BLUE) ||
     (sce_mod->active_spl_effect == CAMERA_EFFECT_ACCENT_GREEN) ||
     (sce_mod->active_spl_effect == CAMERA_EFFECT_ACCENT_ORANGE))  {
     sce_mod->sce_cmd.crcoeff.coef50 =0;
     sce_mod->sce_cmd.crcoeff.coef51 =0;
     sce_mod->sce_cmd.cbcoeff.coef50 =0;
     sce_mod->sce_cmd.cbcoeff.coef51 =0;
     sce_mod->sce_cmd.croffset.offset5 = 0;
     sce_mod->sce_cmd.cboffset.offset5 = 0;
  } else {
     sce_mod->sce_cmd.crcoeff.coef50 = coeff[0];
     sce_mod->sce_cmd.crcoeff.coef51 = coeff[1];
     sce_mod->sce_cmd.cbcoeff.coef50 = coeff[3];
     sce_mod->sce_cmd.cbcoeff.coef51 = coeff[4];
     sce_mod->sce_cmd.croffset.offset5 = coeff[2];
     sce_mod->sce_cmd.cboffset.offset5 = coeff[5];
  }

  sce_mod->sce_cmd.croffset.shift5 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift5 = offset_shift;

} /* vfe_config_sce_cmd */

/*===========================================================================
 * FUNCTION    - vfe_trigger_interpolate_sce->-
 *
 * DESCRIPTION: interpolate the sce triangles based on awb trigger
 *==========================================================================*/
static void vfe_trigger_interpolate_sce_triangles(sce_cr_cb_triangle_set* ip1,
  sce_cr_cb_triangle_set* ip2, sce_cr_cb_triangle_set* op, float ratio)
{
  int *ptrIp1 = (int *)ip1;
  int *ptrIp2 = (int *)ip2;
  int *ptrOp = (int *)op;
  int i, size = sizeof(sce_cr_cb_triangle_set)/ sizeof(int);
  TBL_INTERPOLATE(ptrIp1, ptrIp2, ptrOp, ratio, size, i);
}/* vfe_trigger_interpolate_sce_triangles */

/*===========================================================================
 * FUNCTION    - vfe_trigger_sce_get_triangles -
 *
 * DESCRIPTION: get triangles baseed on AWB/AEC decision
 *==========================================================================*/
static void vfe_trigger_sce_get_triangles(sce_mod_t *sce_mod,
  vfe_params_t *vfe_params, float aec_ratio, awb_cct_type cct_type)
{
  float awb_ratio = 0.0;
  chromatix_parms_type *chroma_ptr = vfe_params->chroma3a;

  CDBG("%s:cct type: %d, aec ratio: %f\n",__func__,cct_type, aec_ratio);
  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      sce_mod->orig = &(sce_mod->origin_triangles_A);
      sce_mod->dest = &(sce_mod->destination_triangles_A);
    break;

    case AWB_CCT_TYPE_D65:
      sce_mod->orig = &(sce_mod->origin_triangles_D65);
      sce_mod->dest = &(sce_mod->destination_triangles_D65);
    break;

    case AWB_CCT_TYPE_TL84_A:
      awb_ratio = GET_INTERPOLATION_RATIO(
        sce_mod->trigger_info.mired_color_temp,
        sce_mod->trigger_info.trigger_A.mired_start,
        sce_mod->trigger_info.trigger_A.mired_end);

      vfe_trigger_interpolate_sce_triangles(
        &(sce_mod->origin_triangles_TL84),
        &(sce_mod->origin_triangles_A),
        sce_mod->orig,
        awb_ratio);
      vfe_trigger_interpolate_sce_triangles(
        &(sce_mod->destination_triangles_TL84),
        &(sce_mod->destination_triangles_A),
        sce_mod->dest,
        awb_ratio);
      break;

  case AWB_CCT_TYPE_D65_TL84:
    awb_ratio = GET_INTERPOLATION_RATIO(
      sce_mod->trigger_info.mired_color_temp,
      sce_mod->trigger_info.trigger_d65.mired_end,
      sce_mod->trigger_info.trigger_d65.mired_start);

    vfe_trigger_interpolate_sce_triangles(&(sce_mod->origin_triangles_D65),
      &(sce_mod->origin_triangles_TL84),
      sce_mod->orig,
      awb_ratio);
    vfe_trigger_interpolate_sce_triangles(
      &(sce_mod->destination_triangles_D65),
      &(sce_mod->destination_triangles_TL84),
      sce_mod->dest,
      awb_ratio);
    break;

    case AWB_CCT_TYPE_TL84:
    default:
    sce_mod->orig = &(sce_mod->origin_triangles_TL84);
    sce_mod->dest = &(sce_mod->destination_triangles_TL84);
    break;
  }

  if (aec_ratio >= 1.0) {
    /*Normal light: do nothing here since destination is already set to
      chromatix destination */
    CDBG("%s: Normal Light\n",__func__);
  } else if (aec_ratio <= 0.0) {
    /*Low light: set destination to origin same*/
    CDBG("%s: Low Light\n",__func__);
    sce_mod->dest = sce_mod->orig;
  } else {
    /*interpolation destination triangles*/
    int *ptrDest = (int *)sce_mod->dest;
    int *ptrOrig = (int *)sce_mod->orig;
    int i, size = (sizeof(sce_cr_cb_triangle_set) / sizeof(int));

    TBL_INTERPOLATE(ptrDest, ptrOrig, ptrDest, aec_ratio, size, i);
  }
} /* vfe_trigger_sce_get_triangles */

/*===========================================================================
 * FUNCTION    - vfe_sce_trigger_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_sce_trigger_update(int mod_id, void *module, void *vparams)
{
  sce_mod_t *sce_mod = (sce_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  uint8_t update_sce = FALSE;
  chromatix_parms_type *chroma_ptr = vfe_params->chroma3a;
  float aec_ratio = 0.0;
  tuning_control_type *tc = &(chroma_ptr->control_SCE);
  trigger_point_type  *tp = &(chroma_ptr->SCE_trigger_point);

  if (!sce_mod->sce_enable) {
    CDBG("%s: SCE not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (!sce_mod->sce_trigger) {
    CDBG("%s: SCE trigger not enabled", __func__);
    return VFE_SUCCESS;
  }
  if (!vfe_util_aec_check_settled(&vfe_params->aec_params)) {
    CDBG("%s: AEC not settled", __func__);
    return VFE_SUCCESS;
  }

  sce_mod->trigger_info.mired_color_temp =
    MIRED(vfe_params->awb_params.color_temp);

  CALC_CCT_TRIGGER_MIRED(sce_mod->trigger_info.trigger_A,
    chroma_ptr->SCE_A_trigger);
  CALC_CCT_TRIGGER_MIRED(sce_mod->trigger_info.trigger_d65,
    chroma_ptr->SCE_D65_trigger);

  awb_cct_type cct_type = vfe_util_get_awb_cct_type(&sce_mod->trigger_info,
                            vfe_params);
  aec_ratio = vfe_util_get_aec_ratio(*tc, tp,vfe_params);

  /* check for trigger updation */
  update_sce = ((sce_mod->prev_mode != vfe_params->vfe_op_mode) ||
    (sce_mod->prev_sce_adj != sce_mod->sce_adjust_factor) ||
    !F_EQUAL(sce_mod->prev_aec_ratio, aec_ratio) || !F_EQUAL(sce_mod->prev_cct_type, cct_type));

  if(update_sce) {
    CDBG("Prev SCE Adj factor: %lf new: %lf", sce_mod->prev_sce_adj,
      sce_mod->sce_adjust_factor);
    CDBG("Prev mode: %d new: %d", sce_mod->prev_mode, vfe_params->vfe_op_mode);
    CDBG("Prev aec ratio %f new %f", sce_mod->prev_aec_ratio, aec_ratio);
    CDBG("Prev cct type %d new %d", sce_mod->prev_cct_type, cct_type);
    vfe_trigger_sce_get_triangles(sce_mod, vfe_params, aec_ratio, cct_type);
    vfe_config_sce_cmd(sce_mod,vfe_params);
    sce_mod->sce_update = TRUE;
    sce_mod->prev_mode = vfe_params->vfe_op_mode;
    sce_mod->prev_aec_ratio = aec_ratio;
    sce_mod->prev_cct_type = cct_type;
    sce_mod->prev_sce_adj = sce_mod->sce_adjust_factor;
  } else
      CDBG("%s: no updates\n",__func__);
  return VFE_SUCCESS;
} /* vfe_sce_trigger_update */

/*===========================================================================
 * FUNCTION    - vfe_sce_update -
 *
 * DESCRIPTION: This function called from UI, sets the tanning/paling factor
 *==========================================================================*/
vfe_status_t vfe_sce_update(int mod_id, void *module, void *vparams)
{
  sce_mod_t *sce_mod = (sce_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t status;

  if (sce_mod->hw_enable) {
    CDBG("%s: Update hardware", __func__);
    status = vfe_util_write_hw_cmd(vfe_params->camfd,
      CMD_GENERAL, vfe_params->moduleCfg,
      sizeof(VFE_ModuleCfgPacked),
      VFE_CMD_MODULE_CFG);
    if (status != VFE_SUCCESS) {
      CDBG_ERROR("%s: VFE_CMD_MODULE_CFG failed", __func__);
      return status;
    }
    vfe_params->update |= VFE_MOD_SCE;
    sce_mod->hw_enable = FALSE;
  }

  if (!sce_mod->sce_enable) {
    CDBG("%s: SCE not enabled", __func__);
    return VFE_SUCCESS;
  }

  CDBG("%s:\n",__func__);
  if(sce_mod->sce_update) {
    sce_mod->sce_update = FALSE;
    status = vfe_sce_config(mod_id,sce_mod,vfe_params);
    if (status != VFE_SUCCESS)
      CDBG_HIGH("%s: Failed\n",__func__);
    else
      vfe_params->update |= VFE_MOD_SCE;
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_sce_config -
 *
 * DESCRIPTION: This function called from pipelines
 *==========================================================================*/
vfe_status_t vfe_sce_config(int mod_id, void *module, void *vparams)
{
  sce_mod_t *sce_mod = (sce_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t status = VFE_SUCCESS;

  if (!sce_mod->sce_enable) {
    CDBG("%s: SCE not enabled", __func__);
    return VFE_SUCCESS;
  }
  CDBG("%s:\n",__func__);

  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL,
    (void *)&(sce_mod->sce_cmd), sizeof(sce_mod->sce_cmd),
     VFE_CMD_SK_ENHAN_CFG);

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_sce_setup -
 *
 * DESCRIPTION: This function called from UI, sets the tanning/paling factor
 *==========================================================================*/
vfe_status_t vfe_sce_setup(sce_mod_t *sce_mod,vfe_params_t *vfe_params,
  int32_t val)
{
  vfe_status_t status = VFE_SUCCESS;
  if (!sce_mod->sce_enable) {
    CDBG("%s: SCE not enabled", __func__);
    return VFE_SUCCESS;
  }

  sce_mod->sce_adjust_factor = (double)val/100.0;
  CDBG("%s:UI : %d Adj factor = %lf\n", __func__, val,
    sce_mod->sce_adjust_factor);
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_sce_enable -
 *
 * DESCRIPTION: This function is called from UI
 *==========================================================================*/
vfe_status_t vfe_sce_enable(int mod_id, void* module, void *vparams,
  int8_t enable, int8_t hw_write)
{
  sce_mod_t *sce_mod = (sce_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  if (!IS_BAYER_FORMAT(vfe_params))
    enable = FALSE;
  CDBG("%s:enable :%d\n",__func__, enable);
  vfe_params->moduleCfg->skinEnhancementEnable = enable;
  sce_mod->sce_enable = enable;  /*enable/disable the SCE in VFE HW*/

  if (hw_write && (sce_mod->hw_enable == enable))
    return VFE_SUCCESS;

  sce_mod->hw_enable = hw_write;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_SCE)
      : (vfe_params->current_config & ~VFE_MOD_SCE);
  }
  return VFE_SUCCESS;
}
/*===========================================================================
 * Function:           vfe_sce_init
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_sce_init(int mod_id, void *module, void *vparams)
{
  sce_mod_t *sce_mod = (sce_mod_t *)module;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  chromatix_parms_type *chroma_ptr = vfe_params->chroma3a;
  CDBG("%s:sce_adjust_factor : %lf\n",__func__,sce_mod->sce_adjust_factor);
  sce_mod->sce_trigger = TRUE;
  sce_mod->sce_update = FALSE;
  sce_mod->sce_adjust_factor = 0.0;
  sce_mod->hw_enable = FALSE;
  sce_mod->prev_cct_type = AWB_CCT_TYPE_TL84;
  sce_mod->prev_mode = VFE_OP_MODE_INVALID;
  sce_mod->prev_sce_adj = 0.0;
  sce_mod->prev_aec_ratio = 0.0;
  vfe_sce_reorder_triangles(sce_mod, vfe_params);
  sce_mod->orig = &(sce_mod->origin_triangles_TL84);
  sce_mod->dest = &(sce_mod->destination_triangles_TL84);
  sce_mod->origin_triangles_ACC_GREEN = acc_color_green;
  sce_mod->origin_triangles_ACC_BLUE = acc_color_blue;
  sce_mod->origin_triangles_ACC_ORANGE = acc_color_orange;
  vfe_config_sce_cmd(sce_mod,vfe_params);
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_sce_trigger_enable -
 *
 * DESCRIPTION: This function updates the mce trigger enable flag
 *==========================================================================*/
vfe_status_t vfe_sce_trigger_enable(int mod_id, void *module, void *vparams,
  int enable)
{
  sce_mod_t *mod = (sce_mod_t *)module;
  CDBG("%s:enable :%d\n",__func__, enable);
  mod->sce_trigger = enable;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_sce_set_spl_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_sce_set_spl_effect(int mod_id, void* mod_sce, void *vparams,
  vfe_spl_effects_type type)
{
  sce_mod_t *sce_mod = (sce_mod_t *)mod_sce;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;
  vfe_status_t rc =  VFE_ERROR_GENERAL;

  if (vfe_params->bs_mode != CAMERA_BESTSHOT_OFF) {
    CDBG("%s: Best shot enabled, skip seteffect", __func__);
    return VFE_SUCCESS;
  }
  CDBG("%s: type %d", __func__, type);

  switch (type) {
    case CAMERA_EFFECT_ACCENT_BLUE:
      sce_mod->orig = &(sce_mod->origin_triangles_ACC_BLUE);
      sce_mod->dest = &(sce_mod->origin_triangles_ACC_BLUE);
      break;
    case CAMERA_EFFECT_ACCENT_GREEN:
      sce_mod->orig = &(sce_mod->origin_triangles_ACC_GREEN);
      sce_mod->dest = &(sce_mod->origin_triangles_ACC_GREEN);
      break;
    case CAMERA_EFFECT_ACCENT_ORANGE:
      sce_mod->orig = &(sce_mod->origin_triangles_ACC_ORANGE);
      sce_mod->dest = &(sce_mod->origin_triangles_ACC_ORANGE);
      break;
    default:
      sce_mod->orig = &(sce_mod->origin_triangles_TL84);
      sce_mod->dest = &(sce_mod->destination_triangles_TL84);
      break;
  }

  rc = vfe_sce_enable(mod_id, mod_sce, vfe_params, TRUE, TRUE);
  if (VFE_SUCCESS != rc) {
    CDBG_ERROR("%s: cannot enable SCE", __func__);
    return rc;
  }

  switch (type) {
    case CAMERA_EFFECT_ACCENT_BLUE:
    case CAMERA_EFFECT_ACCENT_GREEN:
    case CAMERA_EFFECT_ACCENT_ORANGE: {
      rc = vfe_sce_trigger_enable(mod_id, mod_sce, vfe_params, FALSE);
      if (VFE_SUCCESS != rc) {
        CDBG_ERROR("%s: cannot enable trigger", __func__);
        return rc;
      }
      break;
    }
    default:
      rc = vfe_sce_trigger_enable(mod_id, mod_sce, vfe_params, TRUE);
      if (VFE_SUCCESS != rc) {
        CDBG_ERROR("%s: cannot enable trigger", __func__);
      }
    break;
  }
  sce_mod->sce_update = TRUE;
  sce_mod->active_spl_effect = type;
  vfe_config_sce_cmd(sce_mod, vfe_params);
  return rc;
} /* vfe_sce_set_spl_effect */


/*===========================================================================
 * FUNCTION    - vfe_sce_set_bestshot -
 *
 * DESCRIPTION:
 * This function updates the VFE configs based on  the bestshot mode given
 *==========================================================================*/
vfe_status_t vfe_sce_set_bestshot(int mod_id, void *module, void* vparams,
  camera_bestshot_mode_type mode)
{
  sce_mod_t *mod = (sce_mod_t *)module;
  vfe_params_t *params = (vfe_params_t *)vparams;
  vfe_status_t rc =  VFE_ERROR_GENERAL;
  if (!mod || !params) {
    return VFE_ERROR_GENERAL;
  }
  CDBG("%s: mode: %d", __func__,mode);
  switch (mode) {
    case CAMERA_BESTSHOT_PORTRAIT:
    case CAMERA_BESTSHOT_PARTY:
    case CAMERA_BESTSHOT_THEATRE:
    case CAMERA_BESTSHOT_AUTO:
      rc = vfe_sce_enable (mod_id, mod, params, TRUE, TRUE);
      if (VFE_SUCCESS != rc) {
        CDBG("%s: cannot enable SCE", __func__);
        break;
      }
      rc = vfe_sce_trigger_enable (mod_id, mod, params, TRUE);
      if (VFE_SUCCESS != rc) {
        CDBG("%s: cannot enable trigger", __func__);
      }
      break;
    default:
      rc = vfe_sce_enable (mod_id, mod, params, FALSE, TRUE);
      if (VFE_SUCCESS != rc) {
        CDBG("%s: cannot disable SCE", __func__);
        break;
      }
      rc = vfe_sce_trigger_enable (mod_id, mod, params, FALSE);
      if (VFE_SUCCESS != rc) {
        CDBG("%s: cannot disable trigger", __func__);
      }
      break;
  }

  return rc;
}
/*===========================================================================
 * FUNCTION    - vfe_sce_reload_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_sce_reload_params(int mod_id, void* mod_sce, void* vparams)
{

  sce_mod_t *mod = (sce_mod_t *)mod_sce;
  vfe_params_t *params = (vfe_params_t *)vparams;
  CDBG("%s: ", __func__);
  vfe_status_t status = VFE_SUCCESS;
  vfe_sce_reorder_triangles(mod, params);
  mod->orig = &(mod->origin_triangles_TL84);
  mod->dest = &(mod->destination_triangles_TL84);
  vfe_config_sce_cmd(mod, params);
  return status;
} /*vfe_color_conversion_reload_params*/
/*===========================================================================
 * FUNCTION    - vfe_sce_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
vfe_status_t vfe_sce_tv_validate(int mod_id, void *test_input,
  void *test_output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)test_output;
  VFE_Skin_enhan_ConfigCmdType *in, *out;

  CDBG("%s:\n", __func__);
  in = (VFE_Skin_enhan_ConfigCmdType *)(mod_in->reg_dump + (V32_SCE_OFF/4) );
  out = (VFE_Skin_enhan_ConfigCmdType *)(mod_op->reg_dump_data + (V32_SCE_OFF/4));
  //CR Coordinates
  VALIDATE_TST_VEC(in->crcoord.vertex00, out->crcoord.vertex00, 0, "crcoord.vertex00");
  VALIDATE_TST_VEC(in->crcoord.vertex01, out->crcoord.vertex01, 0, "crcoord.vertex01");
  VALIDATE_TST_VEC(in->crcoord.vertex02, out->crcoord.vertex02, 0, "crcoord.vertex02");
  VALIDATE_TST_VEC(in->crcoord.vertex10, out->crcoord.vertex10, 0, "crcoord.vertex10");
  VALIDATE_TST_VEC(in->crcoord.vertex11, out->crcoord.vertex11, 0, "crcoord.vertex11");
  VALIDATE_TST_VEC(in->crcoord.vertex12, out->crcoord.vertex12, 0, "crcoord.vertex12");
  VALIDATE_TST_VEC(in->crcoord.vertex20, out->crcoord.vertex20, 0, "crcoord.vertex20");
  VALIDATE_TST_VEC(in->crcoord.vertex21, out->crcoord.vertex21, 0, "crcoord.vertex21");
  VALIDATE_TST_VEC(in->crcoord.vertex22, out->crcoord.vertex22, 0, "crcoord.vertex22");
  VALIDATE_TST_VEC(in->crcoord.vertex30, out->crcoord.vertex30, 0, "crcoord.vertex30");
  VALIDATE_TST_VEC(in->crcoord.vertex31, out->crcoord.vertex31, 0, "crcoord.vertex31");
  VALIDATE_TST_VEC(in->crcoord.vertex32, out->crcoord.vertex32, 0, "crcoord.vertex32");
  VALIDATE_TST_VEC(in->crcoord.vertex40, out->crcoord.vertex40, 0, "crcoord.vertex40");
  VALIDATE_TST_VEC(in->crcoord.vertex41, out->crcoord.vertex41, 0, "crcoord.vertex41");
  VALIDATE_TST_VEC(in->crcoord.vertex42, out->crcoord.vertex42, 0, "crcoord.vertex42");

  //CB Coordinates
  VALIDATE_TST_VEC(in->cbcoord.vertex00, out->cbcoord.vertex00, 0, "cbcoord.vertex00");
  VALIDATE_TST_VEC(in->cbcoord.vertex01, out->cbcoord.vertex01, 0, "cbcoord.vertex01");
  VALIDATE_TST_VEC(in->cbcoord.vertex02, out->cbcoord.vertex02, 0, "cbcoord.vertex02");
  VALIDATE_TST_VEC(in->cbcoord.vertex10, out->cbcoord.vertex10, 0, "cbcoord.vertex10");
  VALIDATE_TST_VEC(in->cbcoord.vertex11, out->cbcoord.vertex11, 0, "cbcoord.vertex11");
  VALIDATE_TST_VEC(in->cbcoord.vertex12, out->cbcoord.vertex12, 0, "cbcoord.vertex12");
  VALIDATE_TST_VEC(in->cbcoord.vertex20, out->cbcoord.vertex20, 0, "cbcoord.vertex20");
  VALIDATE_TST_VEC(in->cbcoord.vertex21, out->cbcoord.vertex21, 0, "cbcoord.vertex21");
  VALIDATE_TST_VEC(in->cbcoord.vertex22, out->cbcoord.vertex22, 0, "cbcoord.vertex22");
  VALIDATE_TST_VEC(in->cbcoord.vertex30, out->cbcoord.vertex30, 0, "cbcoord.vertex30");
  VALIDATE_TST_VEC(in->cbcoord.vertex31, out->cbcoord.vertex31, 0, "cbcoord.vertex31");
  VALIDATE_TST_VEC(in->cbcoord.vertex32, out->cbcoord.vertex32, 0, "cbcoord.vertex32");
  VALIDATE_TST_VEC(in->cbcoord.vertex40, out->cbcoord.vertex40, 0, "cbcoord.vertex40");
  VALIDATE_TST_VEC(in->cbcoord.vertex41, out->cbcoord.vertex41, 0, "cbcoord.vertex41");
  VALIDATE_TST_VEC(in->cbcoord.vertex42, out->cbcoord.vertex42, 0, "cbcoord.vertex42");

  //CR co-efficients
  VALIDATE_TST_VEC(in->crcoeff.coef00, out->crcoeff.coef00, 0, "crcoeff.coef00");
  VALIDATE_TST_VEC(in->crcoeff.coef01, out->crcoeff.coef01, 0, "crcoeff.coef01");
  VALIDATE_TST_VEC(in->crcoeff.coef10, out->crcoeff.coef10, 0, "crcoeff.coef10");
  VALIDATE_TST_VEC(in->crcoeff.coef11, out->crcoeff.coef11, 0, "crcoeff.coef11");
  VALIDATE_TST_VEC(in->crcoeff.coef20, out->crcoeff.coef20, 0, "crcoeff.coef20");
  VALIDATE_TST_VEC(in->crcoeff.coef21, out->crcoeff.coef21, 0, "crcoeff.coef21");
  VALIDATE_TST_VEC(in->crcoeff.coef30, out->crcoeff.coef30, 0, "crcoeff.coef30");
  VALIDATE_TST_VEC(in->crcoeff.coef31, out->crcoeff.coef31, 0, "crcoeff.coef31");
  VALIDATE_TST_VEC(in->crcoeff.coef40, out->crcoeff.coef40, 0, "crcoeff.coef40");
  VALIDATE_TST_VEC(in->crcoeff.coef41, out->crcoeff.coef41, 0, "crcoeff.coef41");
  VALIDATE_TST_VEC(in->crcoeff.coef50, out->crcoeff.coef50, 0, "crcoeff.coef50");
  VALIDATE_TST_VEC(in->crcoeff.coef51, out->crcoeff.coef51, 0, "crcoeff.coef51");

  //CB co-efficients
  VALIDATE_TST_VEC(in->cbcoeff.coef00, out->cbcoeff.coef00, 0, "cbcoeff.coef00");
  VALIDATE_TST_VEC(in->cbcoeff.coef01, out->cbcoeff.coef01, 0, "cbcoeff.coef01");
  VALIDATE_TST_VEC(in->cbcoeff.coef10, out->cbcoeff.coef10, 0, "cbcoeff.coef10");
  VALIDATE_TST_VEC(in->cbcoeff.coef11, out->cbcoeff.coef11, 0, "cbcoeff.coef11");
  VALIDATE_TST_VEC(in->cbcoeff.coef20, out->cbcoeff.coef20, 0, "cbcoeff.coef20");
  VALIDATE_TST_VEC(in->cbcoeff.coef21, out->cbcoeff.coef21, 0, "cbcoeff.coef21");
  VALIDATE_TST_VEC(in->cbcoeff.coef30, out->cbcoeff.coef30, 0, "cbcoeff.coef30");
  VALIDATE_TST_VEC(in->cbcoeff.coef31, out->cbcoeff.coef31, 0, "cbcoeff.coef31");
  VALIDATE_TST_VEC(in->cbcoeff.coef40, out->cbcoeff.coef40, 0, "cbcoeff.coef40");
  VALIDATE_TST_VEC(in->cbcoeff.coef41, out->cbcoeff.coef41, 0, "cbcoeff.coef41");
  VALIDATE_TST_VEC(in->cbcoeff.coef50, out->cbcoeff.coef50, 0, "cbcoeff.coef50");
  VALIDATE_TST_VEC(in->cbcoeff.coef51, out->cbcoeff.coef51, 0, "cbcoeff.coef51");

  //Cr Offsets
  VALIDATE_TST_VEC(in->croffset.offset0, out->croffset.offset0, 0, "croffset.offset0");
  VALIDATE_TST_VEC(in->croffset.offset1, out->croffset.offset1, 0, "croffset.offset1");
  VALIDATE_TST_VEC(in->croffset.offset2, out->croffset.offset2, 0, "croffset.offset2");
  VALIDATE_TST_VEC(in->croffset.offset3, out->croffset.offset3, 0, "croffset.offset3");
  VALIDATE_TST_VEC(in->croffset.offset4, out->croffset.offset4, 0, "croffset.offset4");
  VALIDATE_TST_VEC(in->croffset.offset5, out->croffset.offset5, 0, "croffset.offset5");

  //Cr Shits
  VALIDATE_TST_VEC(in->croffset.shift0, out->croffset.shift0, 0, "croffset.shift0");
  VALIDATE_TST_VEC(in->croffset.shift1, out->croffset.shift1, 0, "croffset.shift1");
  VALIDATE_TST_VEC(in->croffset.shift2, out->croffset.shift2, 0, "croffset.shift2");
  VALIDATE_TST_VEC(in->croffset.shift3, out->croffset.shift3, 0, "croffset.shift3");
  VALIDATE_TST_VEC(in->croffset.shift4, out->croffset.shift4, 0, "croffset.shift4");
  VALIDATE_TST_VEC(in->croffset.shift5, out->croffset.shift5, 0, "croffset.shift5");

  //Cb Offsets
  VALIDATE_TST_VEC(in->cboffset.offset0, out->cboffset.offset0, 0, "cboffset.offset0");
  VALIDATE_TST_VEC(in->cboffset.offset1, out->cboffset.offset1, 0, "cboffset.offset1");
  VALIDATE_TST_VEC(in->cboffset.offset2, out->cboffset.offset2, 0, "cboffset.offset2");
  VALIDATE_TST_VEC(in->cboffset.offset3, out->cboffset.offset3, 0, "cboffset.offset3");
  VALIDATE_TST_VEC(in->cboffset.offset4, out->cboffset.offset4, 0, "cboffset.offset4");
  VALIDATE_TST_VEC(in->cboffset.offset5, out->cboffset.offset5, 0, "cboffset.offset5");

  //Cb Shits
  VALIDATE_TST_VEC(in->cboffset.shift0, out->cboffset.shift0, 0, "cboffset.shift0");
  VALIDATE_TST_VEC(in->cboffset.shift1, out->cboffset.shift1, 0, "cboffset.shift1");
  VALIDATE_TST_VEC(in->cboffset.shift2, out->cboffset.shift2, 0, "cboffset.shift2");
  VALIDATE_TST_VEC(in->cboffset.shift3, out->cboffset.shift3, 0, "cboffset.shift3");
  VALIDATE_TST_VEC(in->cboffset.shift4, out->cboffset.shift4, 0, "cboffset.shift4");
  VALIDATE_TST_VEC(in->cboffset.shift5, out->cboffset.shift5, 0, "cboffset.shift5");

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_sce_deinit -
 *
 * DESCRIPTION: this function reloads the params
 *==========================================================================*/
vfe_status_t vfe_sce_deinit(int mod_id, void *module, void *params)
{
  sce_mod_t *sce_mod = (sce_mod_t *)module;
  memset(sce_mod, 0 , sizeof(sce_mod_t));
  return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_sce_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_sce_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  sce_module_t *cmd = (sce_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->sce_cmd),
     sizeof(VFE_Skin_enhan_ConfigCmdType),
     VFE_CMD_SK_ENHAN_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_sce_plugin_update */
#endif
