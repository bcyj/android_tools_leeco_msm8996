/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "sce44.h"
#include "isp_pipeline_util.h"
#include "isp_log.h"

#if 0
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#define SWAP(a,b) ({\
  a[0]^=b[0];\
  b[0]^=a[0];\
  a[0]^=b[0];\
  a[1]^=b[1];\
  b[1]^=a[1];\
  a[1]^=b[1];\
})

#ifdef ENABLE_SCE_LOGGING
  #undef ISP_DBG
  #define ISP_DBG ALOGE
#endif

#define FLOAT_EPS       (0.0000001)
#define FLOAT_EQ(x, y)  (fabs((x) - (y)) < FLOAT_EPS)

/** sce_find_line_by_vector:
 *    @line: result line
 *    @vector: line direction vector
 *    @c_point: point belonging to line
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function finds a line parametric equation by vector and point.
 *
 *  Return:  None
 *
 **/
static void sce_find_line_by_vector(ISP_Skin_enhan_line *line,
  sce_shift_vector *vector, cr_cb_point *c_point)
{
  line->point0  = *c_point;
  line->shift_cr = vector->cr;
  line->shift_cb = vector->cb;
}

/** sce_find_line_by_two_points:
 *    @line: result line
 *    @point1: point belonging to line
 *    @point2: point belonging to line
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function finds a line parametric equation by two points.
 *
 *  Return:  None
 *
 **/
static void sce_find_line_by_two_points(ISP_Skin_enhan_line *line,
  cr_cb_point *point1, cr_cb_point *point2)
{
  line->point0  = *point1;
  line->shift_cr = (point2->cr - point1->cr);
  line->shift_cb = (point2->cb - point1->cb);
}

/** sce_find_intersection:
 *    @line1: first line
 *    @line2: second line
 *    @t: parameter for intersection point in first line
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function finds a intersection between two lines and return its position
 *  in first line by parameter.
 *
 *  Return:  TRUE  - lines are intersecting
 *           FALSE - lines are parallel
 *
 **/
static boolean sce_find_intersection(ISP_Skin_enhan_line *line1,
  ISP_Skin_enhan_line *line2, double* t)
{
  double t2;
  /* check if one of the lines is either vertical or horizontal */
  /* Assumptions according SCE documentation:
     - lines are not matching:
        One of the line is crossing center of pentagon and other is one
        of its sides
     - none of the points belong to neither axis (cb and cr does not equal 0)
     - no line can have both shift_cr and shift_cb equal to 0
   */
  if ((FLOAT_EQ((line2->shift_cb * line1->shift_cr -
      line1->shift_cb * line2->shift_cr), 0)) ||
      ((FLOAT_EQ(line1->shift_cr, 0) && FLOAT_EQ(line2->shift_cr, 0)))){
    /* Lines are parallel */

    ISP_DBG(ISP_MOD_SCE, "%s: parallel lines", __func__);
    return FALSE;
  }

  if(FLOAT_EQ(line1->shift_cr, 0)) {
    *t =  ((line1->point0.cr - line2->point0.cr) * line2->shift_cb +
          (line2->point0.cb - line1->point0.cb) * line2->shift_cr) /
          (line1->shift_cb * line2->shift_cr - line2->shift_cb * line1->shift_cr);
    t2  = (line1->point0.cr - line2->point0.cr + line1->shift_cr * *t) /
          line2->shift_cr;
  } else {
    t2 =  ((line2->point0.cr - line1->point0.cr) * line1->shift_cb +
          (line1->point0.cb - line2->point0.cb) * line1->shift_cr) /
          (line2->shift_cb * line1->shift_cr - line1->shift_cb * line2->shift_cr);
    *t =  (line2->point0.cr - line1->point0.cr + line2->shift_cr * t2) /
          line1->shift_cr;
  }

  if ((t2 < 0) || (t2 > 1)){
    /* Lines intersect outside poligon */
    ISP_DBG(ISP_MOD_SCE, "%s: outside intersection", __func__);
    return FALSE;
  }

  return TRUE;
}

/** sce_found_boundaries:
 *    @triangles: set of SCE triangles
 *    @line: line for SCE interpolation
 *    @t_pos boundary in positive direction of shift vector
 *    @t_neg boundary in negative direction of shift vector
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function finds boundaries in both directions from origin point between
 *  which point position can be interpolated according system team documentation
 *
 *  Return:   0 - Success
 *           -1 - Numbers from chromatix are inconsistent: triangles
 *                don't form proper poligon.
 **/
static int sce_found_boundaries(sce_cr_cb_triangle_set *triangles,
  ISP_Skin_enhan_line *line, double *t_pos, double*t_neg)
{
  cr_cb_triangle *array[5];
  double ints_t[2], tmp;
  int idx_t = 0;
  ISP_Skin_enhan_line temp_line;
  int i, j;

  array[0] = &triangles->triangle1;
  array[1] = &triangles->triangle2;
  array[2] = &triangles->triangle3;
  array[3] = &triangles->triangle4;
  array[4] = &triangles->triangle5;

  for(i = 0; i < 5; i++) {
    sce_find_line_by_two_points(&temp_line, &array[i]->point2, &array[i]->point3);
    if(sce_find_intersection(line, &temp_line, &tmp)){
      for(j = 0; j < idx_t; j++)
        if(j<2 && FLOAT_EQ(ints_t[j], tmp))
          /* intersection point is vertex */
          continue;
      if(idx_t > 1){
        /* line intersects pentagon in more than two points  - this should not
           happen unless chromatix is incorrect */
        CDBG_ERROR("%s: Error: too many intersections", __func__);
        return -1;
      }
      ints_t[idx_t] = tmp;
      idx_t++;
    }
  }

  if(idx_t < 2){
    /* line intersects pentagon in less than two points  - this should not
        happen unless chromatix is incorrect */
    CDBG_ERROR("%s: Error: less than two intersections %d", __func__, idx_t);
    return -1;
  }

  if(ints_t[0] > 0){
    *t_pos = ints_t[0];
    *t_neg = ints_t[1];
  } else {
    *t_pos = ints_t[1];
    *t_neg = ints_t[0];
  }

  if((*t_pos < 0) || (*t_neg > 0)) {
    /* intersections are on the same side of central point  - this should not
       happen unless chromatix is incorrect */
    CDBG_ERROR("%s: Error: intersections are on the same side of central point",
               __func__);
    return -1;
  }

  /* According documentation boundaries should be at two thirds of distance
     between central and intersection points. */
  *t_pos *= 2.0/3.0;
  *t_neg *= 2.0/3.0;

  ISP_DBG(ISP_MOD_SCE, "%s: Boundaries %lf,%lf and %lf,%lf",__func__,
    line->point0.cr + line->shift_cr * *t_pos,
    line->point0.cb + line->shift_cb * *t_pos,
    line->point0.cr + line->shift_cr * *t_neg,
    line->point0.cb + line->shift_cb * *t_neg);

  return 0;
}

/** sce_reorder_triangles
 *
 * Description:  reorder the triangles by default chromatix
 *               triangels
 *
 **/
static void sce_reorder_triangles(isp_sce_mod_t *sce_mod, isp_hw_pix_setting_params_t *pix_settings)
{
  chromatix_parms_type *p_chx =
    (chromatix_parms_type *)pix_settings->chromatix_ptrs.chromatixPtr;
  chromatix_SCE_type *p_sce = &p_chx->chromatix_VFE.chromatix_SCE;
  sce_mod->origin_triangles_A = p_sce->origin_triangles_A;
  sce_mod->origin_triangles_D65 = p_sce->origin_triangles_D65;
  sce_mod->origin_triangles_TL84 = p_sce->origin_triangles_TL84;

  ISP_DBG(ISP_MOD_SCE, "%s:\n", __func__);
}

/** calc_sce_shiftbits
 *
 * DESCRIPTION: calculate the right shift bits for the transformation matrix and
 *              offset operation for triangle n
 *
 **/
static void calc_sce_shiftbits(sce_affine_transform_2d *coeff,
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

/** calc_sce_transform
 *
 * DESCRIPTION: calculate the new co-efficents and the offset bits
 *              for the new traingle
 *
 **/
static int32_t calc_sce_transform(ISP_Skin_enhan_range *interp_range,
  cr_cb_triangle *pOrigVert, sce_affine_transform_2d *pTransform,double val)
{
  double M1[9], M2[9], InvM2[9], Tx[9];
  double dest_cr, dest_cb, int_step;
  int32_t rc = TRUE, i;

  if (pOrigVert == NULL || pTransform == NULL){
    ISP_DBG(ISP_MOD_SCE, " Null pointer in vfe_util_sce_transform\n");
    return FALSE;
  }

  /* Calculate the new vertex */
  if (val < 0)
    int_step = interp_range->neg_step * (-val);
  else
    int_step = interp_range->pos_step * val;

  dest_cr = interp_range->interpolation_line.point0.cr +
            interp_range->interpolation_line.shift_cr * int_step;
  dest_cb = interp_range->interpolation_line.point0.cb +
            interp_range->interpolation_line.shift_cb * int_step;

  /* fill in M1, the dest triangle */
  M1[0] = dest_cr;
  M1[3] = dest_cb;
  M1[1] = pOrigVert->point2.cr;
  M1[4] = pOrigVert->point2.cb;
  M1[2] = pOrigVert->point3.cr;
  M1[5] = pOrigVert->point3.cb;
  M1[6] = M1[7] = M1[8] = 1.0;

  /* fill in M2 the original three vertex */
  M2[0] = pOrigVert->point1.cr;
  M2[3] = pOrigVert->point1.cb;
  M2[1] = pOrigVert->point2.cr;
  M2[4] = pOrigVert->point2.cb;
  M2[2] = pOrigVert->point3.cr;
  M2[5] = pOrigVert->point3.cb;
  M2[6] = M2[7] = M2[8] = 1.0;

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

/** calc_sce_mapping
 *
 * DESCRIPTION: calculate the coefficient
 *              updated the config reg struct with the coeff
 *              calculated here
 *
 **/
static void calc_sce_mapping (sce_affine_transform_2d *Tx,
  int32_t *coeff, uint32_t *matrix_shift, uint32_t *offset_shift)
{
  uint32_t matrixShift, offsetShift;
  int i;
  float *TxElem = (float *)Tx;

  /* calculate the co-efficients */
  calc_sce_shiftbits(Tx, matrix_shift, offset_shift);

  for (i = 0; i < 6; i++) {
    if (i%3 == 2) {
      coeff[i] = (int32_t)((*TxElem) * (1<<(*offset_shift)));
      if (coeff[i] <= -65536 || coeff[i] > 65536)
        ISP_DBG(ISP_MOD_SCE, "ERROR: -65536 < coeffE <= 65536 is violated\n");
    } else {
      coeff[i] = (int32_t)((*TxElem) * (1<<((*matrix_shift)+(*offset_shift))));
      if (coeff[i] <= -2047 || coeff[i] > 2047)
        ISP_DBG(ISP_MOD_SCE, "ERROR: -2047 < coeffA <= 2047 is violated\n");
    }
    TxElem++;
  }
  return;
}

/** vfe_calc_sce
 *
 * DESCRIPTION: get the new coeff and Tx
 *
 **/
static void calc_sce_params(ISP_Skin_enhan_range *interp_range,
  cr_cb_triangle *pOrigVert,double val, int32_t *coeff,
  uint32_t *matrix_shift, uint32_t *offset_shift)
{
  sce_affine_transform_2d Tx;
  if(!calc_sce_transform(interp_range,pOrigVert, &Tx, val))
    return;

  calc_sce_mapping(&Tx, coeff, matrix_shift, offset_shift);
}

/** config_sce_cmd
 *
 * DESCRIPTION: update sce coeff, vertex, and etc to reg struct
 *
 **/
static void config_sce_cmd(isp_sce_mod_t *sce_mod,
  isp_pix_trigger_update_input_t *in_params)
{
  int32_t i;
  int32_t coeff[6];
  uint32_t matrix_shift, offset_shift;
  chromatix_parms_type *chroma_ptr = in_params->cfg.chromatix_ptrs.chromatixPtr;
  ASD_struct_type *ASD_algo_data = &chroma_ptr->ASD_algo_data;
  chromatix_SCE_type *chromatix_SCE = &chroma_ptr->chromatix_VFE.chromatix_SCE;
  double sce_adj_factor;
  float portrait_severity = 0;
  float portrait_sce =
    ASD_algo_data->portrait_scene_detect.skin_color_boost_factor;


  portrait_severity =
    (float)in_params->trigger_input.stats_update.asd_update.portrait_severity;

  if (portrait_severity != 0) {
    sce_adj_factor = sce_mod->sce_adjust_factor *
      (1.0 - portrait_severity / 255.0) +
      portrait_sce * (portrait_severity / 255.0);
  } else {
    sce_adj_factor = sce_mod->sce_adjust_factor;
  }

  ISP_DBG(ISP_MOD_SCE, "%s:\n",__func__);
  ISP_DBG(ISP_MOD_SCE, "%s: Portrait severity :%f\n",__func__, portrait_severity);
  ISP_DBG(ISP_MOD_SCE, "%s: Portrait adj factor :%f\n",__func__, portrait_sce);
  ISP_DBG(ISP_MOD_SCE, "%s: sce_adj_factor :%f\n",__func__, sce_adj_factor);
  /* Cr cordintates for Triangle 0-4 */
  sce_mod->sce_cmd.crcoord.vertex00 = sce_mod->orig->triangle1.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex01 = sce_mod->orig->triangle1.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex02 = sce_mod->orig->triangle1.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex10 = sce_mod->orig->triangle2.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex11 = sce_mod->orig->triangle2.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex12 = sce_mod->orig->triangle2.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex20 = sce_mod->orig->triangle3.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex21 = sce_mod->orig->triangle3.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex22 = sce_mod->orig->triangle3.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex30 = sce_mod->orig->triangle4.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex31 = sce_mod->orig->triangle4.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex32 = sce_mod->orig->triangle4.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex40 = sce_mod->orig->triangle5.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex41 = sce_mod->orig->triangle5.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex42 = sce_mod->orig->triangle5.point3.cr;

  /* Cb cordintates for Triangle 0-4 */
  sce_mod->sce_cmd.cbcoord.vertex00 = sce_mod->orig->triangle1.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex01 = sce_mod->orig->triangle1.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex02 = sce_mod->orig->triangle1.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex10 = sce_mod->orig->triangle2.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex11 = sce_mod->orig->triangle2.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex12 = sce_mod->orig->triangle2.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex20 = sce_mod->orig->triangle3.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex21 = sce_mod->orig->triangle3.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex22 = sce_mod->orig->triangle3.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex30 = sce_mod->orig->triangle4.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex31 = sce_mod->orig->triangle4.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex32 = sce_mod->orig->triangle4.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex40 = sce_mod->orig->triangle5.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex41 = sce_mod->orig->triangle5.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex42 = sce_mod->orig->triangle5.point3.cb;

  /* Update VFE with new co-efficients, for triangle 1 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle1,
    sce_adj_factor, coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef00 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef01 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef00 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef01 = coeff[4];
  sce_mod->sce_cmd.croffset.offset0 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset0 = coeff[5];
  sce_mod->sce_cmd.croffset.shift0 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift0 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 2 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle2,
    sce_adj_factor, coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef10 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef11 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef10 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef11 = coeff[4];
  sce_mod->sce_cmd.croffset.offset1 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset1 = coeff[5];
  sce_mod->sce_cmd.croffset.shift1 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift1 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 3 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle3, sce_adj_factor, coeff,
    &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef20 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef21 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef20 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef21 = coeff[4];
  sce_mod->sce_cmd.croffset.offset2 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset2 = coeff[5];
  sce_mod->sce_cmd.croffset.shift2 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift2 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 4 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle4, sce_adj_factor, coeff,
    &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef30 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef31 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef30 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef31 = coeff[4];
  sce_mod->sce_cmd.croffset.offset3 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset3 = coeff[5];
  sce_mod->sce_cmd.croffset.shift3 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift3 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 5 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle5, sce_adj_factor, coeff,
    &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef40 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef41 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef40 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef41 = coeff[4];
  sce_mod->sce_cmd.croffset.offset4 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset4 = coeff[5];
  sce_mod->sce_cmd.croffset.shift4 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift4 = offset_shift;

  /* Update VFE with new co-efficients, for outside region mapping */
  calc_sce_mapping(&chromatix_SCE->outside_region_mapping,
    coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef50 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef51 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef50 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef51 = coeff[4];
  sce_mod->sce_cmd.croffset.offset5 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset5 = coeff[5];
  sce_mod->sce_cmd.croffset.shift5 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift5 = offset_shift;

}

/** trigger_interpolate_sce_triangles
 *
 * DESCRIPTION: interpolate the sce triangles based on awb
 *              trigger
 *
 **/
static void trigger_interpolate_sce_triangles(sce_cr_cb_triangle_set* ip1,
  sce_cr_cb_triangle_set* ip2, sce_cr_cb_triangle_set* op, float ratio)
{
  int *ptrIp1 = (int *)ip1;
  int *ptrIp2 = (int *)ip2;
  int *ptrOp = (int *)op;
  int i, size = sizeof(sce_cr_cb_triangle_set)/ sizeof(int);

  TBL_INTERPOLATE(ptrIp1, ptrIp2, ptrOp, ratio, size, i);
}

static void trigger_interpolate_sce_vectors(sce_shift_vector* ip1,
  sce_shift_vector* ip2, sce_shift_vector* op, float ratio)
{
  op->cr = LINEAR_INTERPOLATION(ip1->cr, ip2->cr, ratio);
  op->cb = LINEAR_INTERPOLATION(ip1->cb, ip2->cb, ratio);
}/* trigger_interpolate_sce_vectors */

/** trigger_sce_get_triangles
 *
 * DESCRIPTION: get triangles baseed on AWB/AEC decision
 *
 **/
static void trigger_sce_get_triangles(isp_sce_mod_t *sce_mod,
  isp_pix_trigger_update_input_t *trigger_params, float aec_ratio, awb_cct_type cct_type)
{
  float awb_ratio = 0.0;
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)trigger_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_SCE_type *p_sce = &chroma_ptr->chromatix_VFE.chromatix_SCE;

  ISP_DBG(ISP_MOD_SCE, "%s:cct type: %d, aec ratio: %f\n",__func__,cct_type, aec_ratio);
  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      sce_mod->orig = &(sce_mod->origin_triangles_A);
      sce_mod->interp_vector = p_sce->shift_vector_A;
    break;

    case AWB_CCT_TYPE_D65:
      sce_mod->orig = &(sce_mod->origin_triangles_D65);
      sce_mod->interp_vector = p_sce->shift_vector_D65;
    break;

    case AWB_CCT_TYPE_TL84_A:
      awb_ratio = GET_INTERPOLATION_RATIO(
        sce_mod->trigger_info.mired_color_temp,
        sce_mod->trigger_info.trigger_A.mired_start,
        sce_mod->trigger_info.trigger_A.mired_end);

      trigger_interpolate_sce_triangles(
        &(sce_mod->origin_triangles_TL84),
        &(sce_mod->origin_triangles_A),
        sce_mod->orig,
        awb_ratio);

      trigger_interpolate_sce_vectors(&p_sce->shift_vector_TL84,
          &p_sce->shift_vector_A, &sce_mod->interp_vector, awb_ratio);
      break;

  case AWB_CCT_TYPE_D65_TL84: {
    awb_ratio = GET_INTERPOLATION_RATIO(
      sce_mod->trigger_info.mired_color_temp,
      sce_mod->trigger_info.trigger_d65.mired_end,
      sce_mod->trigger_info.trigger_d65.mired_start);

    trigger_interpolate_sce_triangles(&(sce_mod->origin_triangles_D65),
      &(sce_mod->origin_triangles_TL84),
      sce_mod->orig,
      awb_ratio);

    trigger_interpolate_sce_vectors(&p_sce->shift_vector_D65,
      &p_sce->shift_vector_TL84, &sce_mod->interp_vector, awb_ratio);
  }
    break;

  case AWB_CCT_TYPE_TL84:
  default:
    sce_mod->orig = &(sce_mod->origin_triangles_TL84);
    sce_mod->interp_vector = p_sce->shift_vector_TL84;
    break;
  }

  sce_find_line_by_vector(&sce_mod->interp_range.interpolation_line,
    &p_sce->shift_vector_A, &sce_mod->orig->triangle1.point1);
  sce_found_boundaries(&sce_mod->origin_triangles_A,
    &sce_mod->interp_range.interpolation_line, &sce_mod->interp_range.pos_step,
    &sce_mod->interp_range.neg_step);
  ISP_DBG(ISP_MOD_SCE, "%s: Interpolation parameter range -  %6.4lf to %6.4lf",
      __func__, sce_mod->interp_range.neg_step, sce_mod->interp_range.pos_step);

} /* trigger_sce_get_triangles */

/** vfe_sce_trigger_update
 *
 * DESCRIPTION: trigger update by aec/awb trigger event
 *
 **/
static int sce_trigger_update( isp_sce_mod_t *sce_mod,
  isp_pix_trigger_update_input_t *trigger_params,
  uint32_t in_param_size)
{
  uint8_t update_sce = FALSE;
  chromatix_parms_type *chroma_ptr =
     (chromatix_parms_type *)trigger_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_SCE_type *chromatix_SCE = &chroma_ptr->chromatix_VFE.chromatix_SCE;
  float aec_ratio = 0.0;
  tuning_control_type *tc = &(chromatix_SCE->control_SCE);
  trigger_point_type  *tp = &(chromatix_SCE->SCE_trigger);
  uint8_t is_burst = IS_BURST_STREAMING((&trigger_params->cfg));

  if (!sce_mod->sce_enable) {
    ISP_DBG(ISP_MOD_SCE, "%s: SCE not enabled", __func__);
    return 0;
  }
  if (!sce_mod->sce_trigger_enable) {
    ISP_DBG(ISP_MOD_SCE, "%s: SCE trigger not enabled", __func__);
    return 0;
  }
  if (trigger_params->trigger_input.stats_update.awb_update.color_temp == 0) {
    ISP_DBG(ISP_MOD_SCE, "%s: SCE zero color temperature", __func__);
    return 0;
  }

  sce_mod->trigger_info.mired_color_temp =
    MIRED(trigger_params->trigger_input.stats_update.awb_update.color_temp);

  CALC_CCT_TRIGGER_MIRED(sce_mod->trigger_info.trigger_A,
    chromatix_SCE->SCE_A_trigger);
  CALC_CCT_TRIGGER_MIRED(sce_mod->trigger_info.trigger_d65,
    chromatix_SCE->SCE_D65_trigger);

  awb_cct_type cct_type = isp_util_get_awb_cct_type(sce_mod->notify_ops->parent,
    &sce_mod->trigger_info, chroma_ptr);
  aec_ratio = isp_util_get_aec_ratio(sce_mod->notify_ops->parent,*tc, tp,
    &trigger_params->trigger_input.stats_update.aec_update, is_burst);

  /* check for trigger updation */
  update_sce = ((sce_mod->old_streaming_mode != trigger_params->cfg.streaming_mode) ||
    (sce_mod->prev_sce_adj != sce_mod->sce_adjust_factor) ||
    !F_EQUAL(sce_mod->prev_aec_ratio, aec_ratio) || !F_EQUAL(sce_mod->prev_cct_type, cct_type));

  if(update_sce) {
    ISP_DBG(ISP_MOD_SCE, "Prev SCE Adj factor: %lf new: %lf", sce_mod->prev_sce_adj,
      sce_mod->sce_adjust_factor);
    ISP_DBG(ISP_MOD_SCE, "Prev mode: %d new: %d", sce_mod->old_streaming_mode, trigger_params->cfg.streaming_mode);
    ISP_DBG(ISP_MOD_SCE, "Prev aec ratio %f new %f", sce_mod->prev_aec_ratio, aec_ratio);
    ISP_DBG(ISP_MOD_SCE, "Prev cct type %d new %d", sce_mod->prev_cct_type, cct_type);
    trigger_sce_get_triangles(sce_mod, trigger_params, aec_ratio, cct_type);
    config_sce_cmd(sce_mod, trigger_params);
    sce_mod->old_streaming_mode = trigger_params->cfg.streaming_mode;
    sce_mod->prev_aec_ratio = aec_ratio;
    sce_mod->prev_cct_type = cct_type;
    sce_mod->prev_sce_adj = sce_mod->sce_adjust_factor;
    sce_mod->hw_update_pending = TRUE;
  } else {
      ISP_DBG(ISP_MOD_SCE, "%s: no updates\n",__func__);
  }

  return 0;
} /* vfe_sce_trigger_update */

/** sce_set_factor
 *    @sce_mod: module instance data
 *    @sce_factor: SCE factor
 *    @in_param_size: parameter size
 *
 *  Configures SCE module according bestshot mode.
 **/
static int sce_set_factor(isp_sce_mod_t *sce_mod,
  int32_t *sce_factor, uint32_t in_param_size)
{
  int rc = 0;

  if (in_param_size != sizeof(int32_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(int32_t), in_param_size);
    return -1;
  }

  if (!sce_mod->sce_enable) {
    ISP_DBG(ISP_MOD_SCE, "%s: SCE not enabled", __func__);
    return 0;
  }

  sce_mod->sce_adjust_factor = (double)(*sce_factor)/100.0;
  ISP_DBG(ISP_MOD_SCE, "%s:UI : %d Adj factor = %lf\n", __func__, *sce_factor,
    sce_mod->sce_adjust_factor);

  return rc;
}

/** sce_enable
 *
 * description: enable sce
 *
 **/
static int sce_enable(isp_sce_mod_t *sce,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  sce->sce_enable = enable->enable;

  return 0;
}

/** sce_trigger_enable
 *
 * description: enable trigger update feature
 *
 **/
static int sce_trigger_enable(isp_sce_mod_t *sce,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  sce->sce_trigger_enable = enable->enable;

  return 0;
}

/** sce_set_bestshot
 *    @mod: module instance data
 *    @pix_settings: pix data
 *    @in_param_size: parameter size
 *
 *  Sets configures SCE module according bestshot mode.
 **/
static int sce_set_bestshot(isp_sce_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  int rc = 0;
  isp_mod_set_enable_t tEnable;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d", __func__,
      sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  ISP_DBG(ISP_MOD_SCE, "%s: Bestshot mode: %d", __func__, pix_settings->bestshot_mode);
  switch (pix_settings->bestshot_mode) {
  case CAM_SCENE_MODE_PORTRAIT:
  case CAM_SCENE_MODE_PARTY:
  case CAM_SCENE_MODE_THEATRE:
  case CAM_SCENE_MODE_AUTO: {
    tEnable.enable = TRUE;
    rc = sce_enable(mod, &tEnable, sizeof(isp_mod_set_enable_t));
    if (rc != 0) {
      ISP_DBG(ISP_MOD_SCE, "%s: cannot enable SCE", __func__);
      break;
    }
    rc = sce_trigger_enable(mod, &tEnable, sizeof(isp_mod_set_enable_t));
    if (rc != 0) {
      ISP_DBG(ISP_MOD_SCE, "%s: cannot enable trigger", __func__);
    }
  }
    break;

  default:
    tEnable.enable = TRUE;

    /* we have to restore the neutral skin factor */
    mod->sce_adjust_factor = 0;
    rc = sce_enable(mod, &tEnable, sizeof(isp_mod_set_enable_t));
    if (rc != 0) {
      ISP_DBG(ISP_MOD_SCE, "%s: cannot disable SCE", __func__);
      break;
    }
    rc = sce_trigger_enable(mod, &tEnable, sizeof(isp_mod_set_enable_t));
    if (rc != 0)
      ISP_DBG(ISP_MOD_SCE, "%s: cannot disable trigger", __func__);

    break;
  }

  return rc;
}
/** sce_reset
 *
 *  reset sce param, config reg struct, etc.
 *
 **/

static void sce_reset(isp_sce_mod_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->sce_cmd, 0, sizeof(mod->sce_cmd));
  mod->sce_adjust_factor = 0.0;
  mod->sce_enable = 0;
  mod->sce_trigger_enable = 0;
  mod->sce_update = 0;
  mod->prev_sce_adj = 0.0;
  mod->prev_aec_ratio = 0.0;
  mod->hw_update_pending = 0;
  memset(&mod->origin_triangles_A, 0, sizeof(mod->origin_triangles_A));
  memset(&mod->origin_triangles_D65, 0, sizeof(mod->origin_triangles_D65));
  memset(&mod->origin_triangles_TL84, 0, sizeof(mod->origin_triangles_TL84));
  memset(&mod->trigger_info, 0, sizeof(mod->trigger_info));
  memset(&mod->origin_triangles_A, 0, sizeof(mod->origin_triangles_A));
}

/** sce_init
 *
 * DESCRIPTION: init module when open hw
 *
 **/
static int sce_init (void *mod_ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_sce_mod_t *sce = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  sce->fd = init_params->fd;
  sce->notify_ops = notify_ops;
  sce->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  sce_reset(sce);
  return 0;
}

/** sce_config
 *
 * DESCRIPTION: initial configuration from header when first
 *              streamon
 *
 **/
static int sce_config(isp_sce_mod_t *sce_mod, isp_hw_pix_setting_params_t *in_params,
                     uint32_t in_param_size)
{
  int  rc = 0;
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_SCE_type *chromatix_SCE =
    &chroma_ptr->chromatix_VFE.chromatix_SCE;
  int32_t coeff[6];
  uint32_t matrix_shift, offset_shift;

  ISP_DBG(ISP_MOD_SCE, "%s:sce_adjust_factor : %lf\n",__func__,sce_mod->sce_adjust_factor);
  sce_set_factor (sce_mod,&(in_params->sce_factor), sizeof(in_params->sce_factor));
  sce_mod->prev_cct_type = AWB_CCT_TYPE_TL84;
  sce_mod->prev_sce_adj = 0.0;
  sce_mod->prev_aec_ratio = 0.0;
  sce_reorder_triangles(sce_mod, in_params);
  sce_mod->orig = &(sce_mod->origin_triangles_TL84);

   /* Cr cordintates for Triangle 0-4 */
  sce_mod->sce_cmd.crcoord.vertex00 = sce_mod->orig->triangle1.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex01 = sce_mod->orig->triangle1.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex02 = sce_mod->orig->triangle1.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex10 = sce_mod->orig->triangle2.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex11 = sce_mod->orig->triangle2.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex12 = sce_mod->orig->triangle2.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex20 = sce_mod->orig->triangle3.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex21 = sce_mod->orig->triangle3.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex22 = sce_mod->orig->triangle3.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex30 = sce_mod->orig->triangle4.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex31 = sce_mod->orig->triangle4.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex32 = sce_mod->orig->triangle4.point3.cr;

  sce_mod->sce_cmd.crcoord.vertex40 = sce_mod->orig->triangle5.point1.cr;
  sce_mod->sce_cmd.crcoord.vertex41 = sce_mod->orig->triangle5.point2.cr;
  sce_mod->sce_cmd.crcoord.vertex42 = sce_mod->orig->triangle5.point3.cr;

/* Cb cordintates for Triangle 0-4 */
  sce_mod->sce_cmd.cbcoord.vertex00 = sce_mod->orig->triangle1.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex01 = sce_mod->orig->triangle1.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex02 = sce_mod->orig->triangle1.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex10 = sce_mod->orig->triangle2.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex11 = sce_mod->orig->triangle2.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex12 = sce_mod->orig->triangle2.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex20 = sce_mod->orig->triangle3.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex21 = sce_mod->orig->triangle3.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex22 = sce_mod->orig->triangle3.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex30 = sce_mod->orig->triangle4.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex31 = sce_mod->orig->triangle4.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex32 = sce_mod->orig->triangle4.point3.cb;

  sce_mod->sce_cmd.cbcoord.vertex40 = sce_mod->orig->triangle5.point1.cb;
  sce_mod->sce_cmd.cbcoord.vertex41 = sce_mod->orig->triangle5.point2.cb;
  sce_mod->sce_cmd.cbcoord.vertex42 = sce_mod->orig->triangle5.point3.cb;

  /* Update VFE with new co-efficients, for triangle 1 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle1, sce_mod->sce_adjust_factor,
    coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef00 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef01 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef00 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef01 = coeff[4];
  sce_mod->sce_cmd.croffset.offset0 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset0 = coeff[5];
  sce_mod->sce_cmd.croffset.shift0 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift0 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 2 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle2, sce_mod->sce_adjust_factor,
    coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef10 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef11 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef10 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef11 = coeff[4];
  sce_mod->sce_cmd.croffset.offset1 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset1 = coeff[5];
  sce_mod->sce_cmd.croffset.shift1 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift1 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 3 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle3, sce_mod->sce_adjust_factor,
    coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef20 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef21 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef20 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef21 = coeff[4];
  sce_mod->sce_cmd.croffset.offset2 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset2 = coeff[5];
  sce_mod->sce_cmd.croffset.shift2 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift2 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 4 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle4, sce_mod->sce_adjust_factor,
    coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef30 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef31 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef30 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef31 = coeff[4];
  sce_mod->sce_cmd.croffset.offset3 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset3 = coeff[5];
  sce_mod->sce_cmd.croffset.shift3 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift3 = offset_shift;

  /* Update VFE with new co-efficients, for triangle 5 */
  calc_sce_params(&sce_mod->interp_range, &sce_mod->orig->triangle5, sce_mod->sce_adjust_factor,
    coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef40 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef41 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef40 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef41 = coeff[4];
  sce_mod->sce_cmd.croffset.offset4 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset4 = coeff[5];
  sce_mod->sce_cmd.croffset.shift4 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift4 = offset_shift;

  /* Update VFE with new co-efficients, for outside region mapping */
  calc_sce_mapping(&chromatix_SCE->outside_region_mapping,
    coeff, &matrix_shift, &offset_shift);

  sce_mod->sce_cmd.crcoeff.coef50 = coeff[0];
  sce_mod->sce_cmd.crcoeff.coef51 = coeff[1];
  sce_mod->sce_cmd.cbcoeff.coef50 = coeff[3];
  sce_mod->sce_cmd.cbcoeff.coef51 = coeff[4];
  sce_mod->sce_cmd.croffset.offset5 = coeff[2];
  sce_mod->sce_cmd.cboffset.offset5 = coeff[5];
  sce_mod->sce_cmd.croffset.shift5 = matrix_shift;
  sce_mod->sce_cmd.cboffset.shift5 = offset_shift;

  sce_mod->hw_update_pending = TRUE;
  return rc;
}

/** sce_destroy
 *
 * description: close sce
 *
 **/
static int sce_destroy (void *mod_ctrl)
{
  isp_sce_mod_t *sce = mod_ctrl;

  memset(sce,  0,  sizeof(isp_sce_mod_t));
  free(sce);
  return 0;
}
/** sce_set_params
 *
 * description: set parameters
 *
 **/
static int sce_set_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_sce_mod_t *sce = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = sce_enable(sce, (isp_mod_set_enable_t *)in_params,
                     in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = sce_config(sce, (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = sce_trigger_enable(sce, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = sce_trigger_update(sce, in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_BESTSHOT:
     rc = sce_set_bestshot(sce, in_params, in_param_size);
     break;
  case ISP_HW_MOD_SET_SCE_FACTOR:
     rc = sce_set_factor(sce, (int32_t *)in_params, in_param_size);
     break;
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    break;
  }
  return rc;
}

/** sce_get_params
 *
 * description: get parameters
 *
 **/
static int sce_get_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_sce_mod_t *sce = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = sce->sce_enable;
    break;
  }
  default:
    rc = -EPERM;
    break;
  }
  return rc;
}

/** sce_do_hw_update
 *
 * description: sce_do_hw_update
 *
 **/
static int sce_do_hw_update(isp_sce_mod_t *sce_mod)
{
  int i, rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  ISP_DBG(ISP_MOD_SCE, "%s: E: hw_update = %d\n", __func__, sce_mod->hw_update_pending);
  if (sce_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &sce_mod->sce_cmd;
    cfg_cmd.cmd_len = sizeof(sce_mod->sce_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_SCE40_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_SCE40_LEN * sizeof(uint32_t);

    //isp_sce_debug(&sce_mod);
    rc = ioctl(sce_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    sce_mod->hw_update_pending = 0;
  }
  return rc;
}

/** sce_action
 *
 * description: processing the action
 *
 **/
static int sce_action (void *mod_ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_sce_mod_t *sce = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = sce_do_hw_update(sce);
    break;
  case ISP_HW_MOD_ACTION_RESET:
    sce_reset(sce);
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

/** sce44_open
 *
 * description: open sce
 *
 **/
isp_ops_t *sce44_open(uint32_t version)
{
  isp_sce_mod_t *sce = malloc(sizeof(isp_sce_mod_t));

  if (!sce) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(sce,  0,  sizeof(isp_sce_mod_t));
  sce->ops.ctrl = (void *)sce;
  sce->ops.init = sce_init;
  /* destroy the module object */
  sce->ops.destroy = sce_destroy;
  /* set parameter */
  sce->ops.set_params = sce_set_params;
  /* get parameter */
  sce->ops.get_params = sce_get_params;
  sce->ops.action = sce_action;
  return &sce->ops;

}
