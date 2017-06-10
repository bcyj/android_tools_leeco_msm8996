/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "mesh_rolloff44.h"
#include "../mlro_to_plro/mlro.h"
#include "isp_log.h"

#ifdef ROLLOFF_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

// #define ROLLOFF_TBL_DEBUG  /*print out the tbl value*/

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#define NUM_OF_SUB_GRID 4 /* init 0 or 64? compute by system team code*/
#define INTERP_FACTOR 3 /* 2^INTERP_FACTOR = NUM_OF_SUB_GRID */

static void mesh_rolloff_V4_ScaleMesh(float *Mesh, uint16_t *meshOut);

/** mesh_rolloff_mesh_table_debug:
 *    @meshtbl: input 17x13 table to be printed
 *
 *  Dump the mesh_rolloff table of size 17x13
 *
 *
 *  Return void
 **/
static void mesh_rolloff_mesh_table_debug(mesh_rolloff_array_type *meshtbl)
{
  int i, j;

  ISP_DBG(ISP_MOD_ROLLOFF , "%s: 17x13 Rolloff Tbl R\n", __func__);
  for (i = 0; i < CHROMATIX_MESH_ROLL_NUM_COL; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \n",
      __func__, meshtbl->r_gain[(i*17)+0], meshtbl->r_gain[(i*17)+1],
      meshtbl->r_gain[(i*17)+2], meshtbl->r_gain[(i*17)+3],
      meshtbl->r_gain[(i*17)+4], meshtbl->r_gain[(i*17)+5],
      meshtbl->r_gain[(i*17)+6], meshtbl->r_gain[(i*17)+7],
      meshtbl->r_gain[(i*17)+8], meshtbl->r_gain[(i*17)+9],
      meshtbl->r_gain[(i*17)+10], meshtbl->r_gain[(i*17)+11],
      meshtbl->r_gain[(i*17)+12], meshtbl->r_gain[(i*17)+13],
      meshtbl->r_gain[(i*17)+14], meshtbl->r_gain[(i*17)+15],
      meshtbl->r_gain[(i*17)+16]);
  }
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: 17x13 Rolloff Tbl GR\n", __func__);
  for (i = 0; i < CHROMATIX_MESH_ROLL_NUM_COL; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \n",
      __func__, meshtbl->gr_gain[(i*17)+0], meshtbl->gr_gain[(i*17)+1],
      meshtbl->gr_gain[(i*17)+2], meshtbl->gr_gain[(i*17)+3],
      meshtbl->gr_gain[(i*17)+4], meshtbl->gr_gain[(i*17)+5],
      meshtbl->gr_gain[(i*17)+6], meshtbl->gr_gain[(i*17)+7],
      meshtbl->gr_gain[(i*17)+8], meshtbl->gr_gain[(i*17)+9],
      meshtbl->gr_gain[(i*17)+10], meshtbl->gr_gain[(i*17)+11],
      meshtbl->gr_gain[(i*17)+12], meshtbl->gr_gain[(i*17)+13],
      meshtbl->gr_gain[(i*17)+14], meshtbl->gr_gain[(i*17)+15],
      meshtbl->gr_gain[(i*17)+16]);
  }

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: 17x13 Rolloff Tbl GB\n", __func__);
  for (i = 0; i < CHROMATIX_MESH_ROLL_NUM_COL; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \n",
      __func__, meshtbl->gb_gain[(i*17)+0], meshtbl->gb_gain[(i*17)+1],
      meshtbl->gb_gain[(i*17)+2], meshtbl->gb_gain[(i*17)+3],
      meshtbl->gb_gain[(i*17)+4], meshtbl->gb_gain[(i*17)+5],
      meshtbl->gb_gain[(i*17)+6], meshtbl->gb_gain[(i*17)+7],
      meshtbl->gb_gain[(i*17)+8], meshtbl->gb_gain[(i*17)+9],
      meshtbl->gb_gain[(i*17)+10], meshtbl->gb_gain[(i*17)+11],
      meshtbl->gb_gain[(i*17)+12], meshtbl->gb_gain[(i*17)+13],
      meshtbl->gb_gain[(i*17)+14], meshtbl->gb_gain[(i*17)+15],
      meshtbl->gb_gain[(i*17)+16]);
  }

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: 17x13 Rolloff Tbl B\n", __func__);
  for (i = 0; i < CHROMATIX_MESH_ROLL_NUM_COL; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \n",
      __func__, meshtbl->b_gain[(i*17)+0], meshtbl->b_gain[(i*17)+1],
      meshtbl->b_gain[(i*17)+2], meshtbl->b_gain[(i*17)+3],
      meshtbl->b_gain[(i*17)+4], meshtbl->b_gain[(i*17)+5],
      meshtbl->b_gain[(i*17)+6], meshtbl->b_gain[(i*17)+7],
      meshtbl->b_gain[(i*17)+8], meshtbl->b_gain[(i*17)+9],
      meshtbl->b_gain[(i*17)+10], meshtbl->b_gain[(i*17)+11],
      meshtbl->b_gain[(i*17)+12], meshtbl->b_gain[(i*17)+13],
      meshtbl->b_gain[(i*17)+14], meshtbl->b_gain[(i*17)+15],
      meshtbl->b_gain[(i*17)+16]);
  }

  return;
} /* mesh_rolloff_mesh_table_debug */

/** mesh_rolloff_mesh_downscaled_table_debug:
 *    @meshtbl: Input 13x10 table to be printed
 *
 *  Print the 13x10 Rolloff table
 *
 *
 *  Return void
 **/
static void mesh_rolloff_mesh_downscaled_table_debug(float *meshtbl)
{
  int i;
  for (i = 0; i < HW_MESH_ROLL_NUM_COL; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: %f %f %f %f %f %f %f %f %f %f %f %f %f \n",
      __func__, meshtbl[(i*13)+0], meshtbl[(i*13)+1],
      meshtbl[(i*13)+2], meshtbl[(i*13)+3],
      meshtbl[(i*13)+4], meshtbl[(i*13)+5],
      meshtbl[(i*13)+6], meshtbl[(i*13)+7],
      meshtbl[(i*13)+8], meshtbl[(i*13)+9],
      meshtbl[(i*13)+10], meshtbl[(i*13)+11],
      meshtbl[(i*13)+12]);
  }

  return;
} /* mesh_rolloff_mesh_downscaled_table_debug */

/** mesh_rolloff_downscale_rolloff_table:
 *    @tableIn: Input 17x13 table
 *    @tableOut: Output 13x10 table
 *
 *  Convert 17x13 table to 13x10 table if size is >130. Then convert 13x10 float
 *  table to Q10
 *
 *
 *  Return void
 **/
static void mesh_rolloff_downscale_rolloff_table (mesh_rolloff_array_type *tableIn,
  MESH_RollOffTable_V4 *tableOut)
{
  int i =0;

  /* Down scale the mesh table from Chromatix header 17x13 to HW 13x10 size*/
  if (tableIn->mesh_rolloff_table_size > MESH_ROLL_OFF_V4_TABLE_SIZE) {
    ISP_DBG(ISP_MOD_ROLLOFF, "\n\n\n\n%s:Bicubuc downscale 17x13 Chromatix mesh to 13x10 mesh table\n",
      __func__);
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: R table 13 x 10\n", __func__);
    mesh_rolloff_V4_ScaleMesh(tableIn->r_gain, tableOut->TableR);

    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Gr table 13 x 10\n", __func__);
    mesh_rolloff_V4_ScaleMesh(tableIn->gr_gain, tableOut->TableGr);

    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Gb table 13 x 10\n", __func__);
    mesh_rolloff_V4_ScaleMesh(tableIn->gb_gain, tableOut->TableGb);

    ISP_DBG(ISP_MOD_ROLLOFF, "%s: B table 13 x 10\n", __func__);
    mesh_rolloff_V4_ScaleMesh(tableIn->b_gain, tableOut->TableB);
  }
  else {
    for (i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
      tableOut->TableR[i] = FLOAT_TO_Q(10, tableIn->r_gain[i]);
      tableOut->TableGr[i] = FLOAT_TO_Q(10, tableIn->gr_gain[i]);
      tableOut->TableGb[i] = FLOAT_TO_Q(10, tableIn->gb_gain[i]);
      tableOut->TableB[i] = FLOAT_TO_Q(10, tableIn->b_gain[i]);
    }
  }
} /* mesh_rolloff_downscale_rolloff_table */

/** rolloff_normalize_table:
 *    @mesh_mod: Pointer to rolloff module struct
 *    @pix_setting: Pointer to pipeline settings
 *
 *  Get the tables from the chromatix pointer and normalize the values to ensure
 *  all values are >1
 *
 *
 *  Return void
 **/
static void rolloff_normalize_table(isp_mesh_rolloff_mod_t *mesh_mod,
  isp_hw_pix_setting_params_t *pix_setting)
{
  int i, j, k;
  float min_value = 1.0, scaling_val;
  chromatix_VFE_common_type *chrComPtr = NULL;
  mesh_rolloff_array_type inTbl;
  MESH_RollOffTable_V4 downscaledTable;
  MESH_RollOffTable_V4 *outTbl = NULL;
  MESH_RollOffTable_V4 *deltaTbl = NULL;
  chromatix_rolloff_type *chromatix_rolloff;
  chromatix_rolloff_type *inf_tables = NULL;
  chromatix_VFE_common_type *infTblPtr = NULL;

  chrComPtr = pix_setting->chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  if (pix_setting->af_rolloff_info.rolloff_tables_macro != NULL) {
    infTblPtr =
      (chromatix_VFE_common_type *)pix_setting->af_rolloff_info.rolloff_tables_macro;
    inf_tables = &infTblPtr->chromatix_rolloff;
  }

  for (k = 0, min_value = 1.0; k < ISP_ROLLOFF_LENS_POSITION_MAX; k++) {
    if (mesh_mod->rolloff_calibration_table.rolloff_tableset[k] != NULL) {
      for (i = ISP_ROLLOFF_TL84_LIGHT; i < ISP_ROLLOFF_MAX_LIGHT; i++) {
        outTbl = &(mesh_mod->rolloff_tbls.rolloff_tableset[k]->left[i]);
        deltaTbl = &(mesh_mod->rolloff_calibration_table.rolloff_tableset[k]->left[i]);

        /*input table from chromatix*/
        if (k == ISP_ROLLOFF_LENS_POSITION_INF) {
          if (i == ISP_ROLLOFF_LED_FLASH)
            inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_LED);
          else if (i == ISP_ROLLOFF_STROBE_FLASH)
            inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_Strobe);
          else if (i < ROLLOFF_MAX_LIGHT)
            inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table[i]);
          else
            inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_lowlight[i % ROLLOFF_MAX_LIGHT]);
        } else if (k == ISP_ROLLOFF_LENS_POSITION_MACRO) {
          if (!inf_tables) // if not supported skip this normalization
            break;
          // get the table from sensor. Currently copied the above code
          if (i == ISP_ROLLOFF_LED_FLASH)
            inTbl = (inf_tables->chromatix_mesh_rolloff_table_LED);
          else if (i == ISP_ROLLOFF_STROBE_FLASH)
            inTbl = (inf_tables->chromatix_mesh_rolloff_table_Strobe);
          else if (i < ROLLOFF_MAX_LIGHT)
            inTbl = (inf_tables->chromatix_mesh_rolloff_table[i]);
          else
            inTbl = (inf_tables->chromatix_mesh_rolloff_table_lowlight[i % ROLLOFF_MAX_LIGHT]);
        }

        /* normalize initial mesh rolloff table*/
        for (j = 0; j < MESH_ROLLOFF_SIZE; j++) {
          /* RED Channel */
          if (inTbl.r_gain[j] < min_value)
            min_value = inTbl.r_gain[j];
          /* GR Channel */
          if (inTbl.gr_gain[j] < min_value)
            min_value = inTbl.gr_gain[j];
          /* BLUE Channel */
          if (inTbl.b_gain[j] < min_value)
            min_value = inTbl.b_gain[j];
          /* GB Channel */
          if (inTbl.gb_gain[j] < min_value)
            min_value = inTbl.gb_gain[j];
        }
        if (min_value < 1.0) {
          scaling_val = 1.0 / min_value;

          for (j = 0; j < MESH_ROLLOFF_SIZE; j++) {
            /* RED Channel */
            inTbl.r_gain[j]  *= scaling_val;
            /* GR Channel */
            inTbl.gr_gain[j] *= scaling_val;
            /* BLUE Channel */
            inTbl.b_gain[j]  *= scaling_val;
            /* GB Channel */
            inTbl.gb_gain[j] *= scaling_val;
          }
        }

        mesh_rolloff_downscale_rolloff_table(&inTbl, &downscaledTable);

        for (j = 0; j < MESH_ROLL_OFF_V4_TABLE_SIZE; j++) {
          /* RED Channel */
          outTbl->TableR[j] = downscaledTable.TableR[j] * deltaTbl->TableR[j];
          /* GR Channel */
          outTbl->TableGr[j] = downscaledTable.TableGr[j] * deltaTbl->TableGr[j];
          /* BLUE Channel */
          outTbl->TableB[j] = downscaledTable.TableB[j] * deltaTbl->TableB[j];
          /* GB Channel */
          outTbl->TableGb[j] = downscaledTable.TableGb[j] * deltaTbl->TableGb[j];
        }
      }
    }
  }
} /* rolloff_normalize_table */

/** rolloff_prepare_tables
 *    @mesh_mod: Pointer to rolloff module struct
 *    @pix_settings: Pointer to pipeline settings
 *
 *  Allocate and initialize rolloff tables
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int rolloff_prepare_tables(isp_mesh_rolloff_mod_t *mesh_mod,
  isp_hw_pix_setting_params_t *pix_settings)
{
  int i, j, k;

  if(mesh_mod->rolloff_tbls.rolloff_tableset[0] == NULL) {
    mesh_mod->rolloff_tbls.rolloff_tableset[0] =
      (isp_rolloff_info_t*)malloc(sizeof(isp_rolloff_info_t) );
    mesh_mod->rolloff_calibration_table.rolloff_tableset[0] =
      (isp_rolloff_info_t*)malloc(sizeof(isp_rolloff_info_t) );
  }

  if (pix_settings->af_rolloff_info.rolloff_tables_macro != NULL &&
      mesh_mod->rolloff_calibration_table.rolloff_tableset[1] == NULL) {
    /* */
    mesh_mod->rolloff_calibration_table.rolloff_tableset[1] =
      (isp_rolloff_info_t*)malloc(sizeof(isp_rolloff_info_t));
    /* point to the table from sensor since its not changing */
    mesh_mod->rolloff_tbls.rolloff_tableset[1] =
      (isp_rolloff_info_t*)malloc(sizeof(isp_rolloff_info_t));
  }

  /* Todo: Remove this once Sensor Provides Delta Tables */
  for (j = 0; j < ISP_ROLLOFF_LENS_POSITION_MAX; j++) {
    for (k = 0; k < ISP_ROLLOFF_MAX_LIGHT; k++) {
      for (i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
        if (mesh_mod->rolloff_calibration_table.rolloff_tableset[j] != NULL) {
          mesh_mod->rolloff_calibration_table.rolloff_tableset[j]->left[k].TableR[i] = 1.0;
          mesh_mod->rolloff_calibration_table.rolloff_tableset[j]->left[k].TableB[i] = 1.0;
          mesh_mod->rolloff_calibration_table.rolloff_tableset[j]->left[k].TableGr[i] = 1.0;
          mesh_mod->rolloff_calibration_table.rolloff_tableset[j]->left[k].TableGb[i] = 1.0;
        }
      }
    }
  }

  for (j = 0; j < ISP_ROLLOFF_LENS_POSITION_MAX; j++) {
    if (mesh_mod->rolloff_calibration_table.rolloff_tableset[j] != NULL) {
      memset(mesh_mod->rolloff_tbls.rolloff_tableset[j], 0x0, sizeof(isp_rolloff_info_t));
    }
  }
  /* Left frame tables */
  rolloff_normalize_table(mesh_mod, pix_settings);

  return 0;
} /* rolloff_prepare_tables */

/** mesh_rolloff_table_debug:
 *    @meshtbl: Table to be printed
 *
 *  Print the R, Gr, Gb, B values from input 13x10 table
 *
 *
 *  Return void
 **/
static void mesh_rolloff_table_debug(MESH_RollOffTable_V4 *meshtbl)
{
  int i;
#ifdef ROLLOFF_TBL_DEBUG
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: Mesh Rolloff table\n", __func__);
  for (i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: %u %u %u %u\n", __func__, meshtbl->TableR[i], meshtbl->TableGr[i],
    meshtbl->TableGb[i], meshtbl->TableB[i]);
  }
#endif
  return;
} /* mesh_rolloff_table_debug */

/** mesh_rolloff_V4_ScaleMesh:
 *    @Mesh: Pointer to 17x13 float rolloff table
 *    @meshOut: Pointer to output 13x10 table
 *
 *  Downscale the 17x13 float table to 13x10 float table
 *
 *
 *  Return void
 **/
static void mesh_rolloff_V4_ScaleMesh(float *Mesh, uint16_t *meshOut)
{
  float tempMesh[MESH_ROLL_OFF_V4_TABLE_SIZE];
  double cxm, cx0, cx1, cx2, cym, cy0, cy1, cy2;
  double am, a0, a1, a2, bm, b0, b1, b2;
  double tx , ty;
  int ix, iy;
  int i, j;

  /* Down scale mesh table by bicubic interpolation
     x ratio: 16/MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS
     y ratio: 12/MESH_ROLL_OFF_V4_VERTICAL_GRIDS  */
  for (i = 0; i < (MESH_ROLL_OFF_V4_VERTICAL_GRIDS + 1); i++) {
    for (j = 0; j < (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1); j++) {
      tx = (double)j * 16.0 / 12.0;
      ix = (int)tx;
      tx -= (double)ix;

      ty = (double)i * 12.0 / 9.0;
      iy = (int)ty;
      ty -= (double)iy;

      /*get x direction coeff and y direction coeff*/
      CUBIC_F(tx, cxm, cx0, cx1, cx2);
      CUBIC_F(ty, cym, cy0, cy1, cy2);

      if (ty == 0 && tx == 0)
        tempMesh[(i * (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1)) + j] =
          Mesh[(iy * 17) + ix];
      else if (ty == 0) {
        am = Mesh[(iy * 17) + (ix - 1)];
        a0 = Mesh[(iy * 17) + ix];
        a1 = Mesh[(iy * 17) + (ix + 1)];
        a2 = Mesh[(iy * 17) + (ix + 2)];
        tempMesh[i * (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1) + j] =
          (float)((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));
      }
      else if (tx == 0) {
        bm = Mesh[((iy - 1) * 17) + ix];
        b0 = Mesh[(iy * 17) + ix];
        b1 = Mesh[((iy + 1) * 17) + ix];
        b2 = Mesh[((iy + 2) * 17) + ix];
        tempMesh[i * (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1) + j] =
          (float)((cym * bm) + (cy0 * b0) + (cy1 * b1) + (cy2 * b2));
      }
      else {
        am = Mesh[((iy-1) * 17) + (ix - 1)];
        a0 = Mesh[((iy-1) * 17) + ix];
        a1 = Mesh[((iy-1) * 17) + (ix + 1)];
        a2 = Mesh[((iy-1) * 17) + (ix + 2)];
        bm = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Mesh[(iy * 17) + (ix-1)];
        a0 = Mesh[(iy * 17) + ix];
        a1 = Mesh[(iy * 17) + (ix + 1)];
        a2 = Mesh[(iy * 17) + (ix + 2)];
        b0 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Mesh[((iy + 1) * 17) + (ix-1)];
        a0 = Mesh[((iy + 1) * 17) + ix];
        a1 = Mesh[((iy + 1) * 17) + (ix + 1)];
        a2 = Mesh[((iy + 1) * 17) + (ix + 2)];
        b1 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Mesh[((iy + 2) * 17) + (ix - 1)];
        a0 = Mesh[((iy + 2) * 17) + ix];
        a1 = Mesh[((iy + 2) * 17) + (ix + 1)];
        a2 = Mesh[((iy + 2) * 17) + (ix + 2)];
        b2 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));
        tempMesh[(i * (MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS + 1)) + j] =
          (float)((cym * bm) + (cy0 * b0) + (cy1 * b1) + (cy2 * b2));
      }
    }
  }
  mesh_rolloff_mesh_downscaled_table_debug(tempMesh);
  /* Fill out Q10 tables*/
  for (i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
    meshOut[i] = FLOAT_TO_Q(10, tempMesh[i]);
  }

}/*mesh_rolloff_V4_ScaleMesh*/

/** mesh_rolloff_table_interpolate:
 *    @in1: input table 1
 *    @in2: input table 2
 *    @out: output interpolated table
 *    ratio: ratio to be used for interpolation
 *
 *  Interpolate table 1 and table 2 using input ratio
 *
 *
 *  Return void
 **/
static void mesh_rolloff_table_interpolate(MESH_RollOffTable_V4 *in1,
  MESH_RollOffTable_V4 *in2, MESH_RollOffTable_V4 *out, float ratio)
{
  int i = 0;
  int mesh_rolloff_table_size = MESH_ROLL_OFF_V4_TABLE_SIZE;
  TBL_INTERPOLATE(in1->TableR, in2->TableR, out->TableR, ratio,
    mesh_rolloff_table_size, i);
  TBL_INTERPOLATE(in1->TableGb, in2->TableGb, out->TableGb, ratio,
    mesh_rolloff_table_size, i);
  TBL_INTERPOLATE(in1->TableGr, in2->TableGr, out->TableGr, ratio,
    mesh_rolloff_table_size, i);
  TBL_INTERPOLATE(in1->TableB, in2->TableB, out->TableB, ratio,
    mesh_rolloff_table_size, i);
} /* mesh_rolloff_table_interpolate */

/** mesh_rolloff_calc_flash_trigger:
 *    @tblNormalLight: Input table with normal light
 *    @tblOut: output table after interpolation
 *    @mesh_tbls: Pointer to set of different light tables
 *    @in_params: Pipeline trigger update params
 *
 *  If Flash is ON, use flash_sensitivity values from aec update as ratio
 *  to interpolate between normal light table and flash table
 *
 *
 *  Return void
 **/
static void mesh_rolloff_calc_flash_trigger(MESH_RollOffTable_V4 *tblNormalLight,
  MESH_RollOffTable_V4 *tblOut, isp_rolloff_info_t *mesh_tbls,
  isp_pix_trigger_update_input_t *in_params)
{
  float ratio;
  float flash_start, flash_end;
  MESH_RollOffTable_V4 *tblFlash = NULL;
  isp_flash_params_t *flash_params = &(in_params->cfg.flash_params);
  cam_flash_mode_t *flash_mode = &(in_params->trigger_input.flash_mode);
  chromatix_VFE_common_type *chrComPtr =
    (chromatix_VFE_common_type *)in_params->cfg.chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  if ((int)flash_params->flash_type == CAMERA_FLASH_STROBE) {
    tblFlash = &(mesh_tbls->left[ISP_ROLLOFF_STROBE_FLASH]);
    flash_start = chromatix_rolloff->rolloff_Strobe_start;
    flash_end = chromatix_rolloff->rolloff_Strobe_end;
  } else {
    tblFlash = &(mesh_tbls->left[ISP_ROLLOFF_LED_FLASH]);
    flash_start = chromatix_rolloff->rolloff_LED_start;
    flash_end = chromatix_rolloff->rolloff_LED_end;
  }

  if (*flash_mode == CAM_FLASH_MODE_ON)
    ratio = flash_params->sensitivity_led_off / flash_params->sensitivity_led_hi;
  else //assume flash off. To be changed when AUTO mode is added
    ratio = flash_start;

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: flash_start %5.2f flash_end %5.2f \n", __func__, flash_start,
    flash_end);

  if (ratio >= flash_end)
    *tblOut = *tblFlash;
  else if (ratio <= flash_start)
    *tblOut = *tblNormalLight;
  else
    mesh_rolloff_table_interpolate(tblNormalLight, tblFlash, tblOut,
        ratio/(flash_end - flash_start));
} /* mesh_rolloff_calc_flash_trigger */

/** mesh_rolloff_calc_awb_trigger:
 *    @mod: Pointer to rolloff module struct
 *    @tblOut: output table after interpolation
 *    @mesh_tbls: Pointer to set of different light tables
 *    @in_params: Pipeline trigger update params
 *
 *  Use color temp and cct_type from awb_update to interpolate between different
 *  lighting conditions
 *
 *
 *  Return void
 **/
static void mesh_rolloff_calc_awb_trigger(isp_mesh_rolloff_mod_t *mod,
  MESH_RollOffTable_V4 *tblOut, isp_rolloff_info_t *mesh_tbls,
  isp_pix_trigger_update_input_t *in_params)
{
  float ratio = 0.0;
  cct_trigger_info trigger_info;
  awb_cct_type cct_type;
  chromatix_parms_type *chrPtr =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_VFE_common_type *chrComPtr =
    (chromatix_VFE_common_type *)in_params->cfg.chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  trigger_info.mired_color_temp =
    MIRED(in_params->trigger_input.stats_update.awb_update.color_temp);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
    chromatix_rolloff->rolloff_A_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
    chromatix_rolloff->rolloff_D65_trigger);

  cct_type = isp_util_get_awb_cct_type(mod->notify_ops->parent, &trigger_info, chrPtr);

  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      *tblOut = mesh_tbls->left[ISP_ROLLOFF_A_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84_A:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);
      mesh_rolloff_table_interpolate(&(mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT]),
        &(mesh_tbls->left[ISP_ROLLOFF_A_LIGHT]), tblOut, ratio);
      break;
    case AWB_CCT_TYPE_D65_TL84:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_d65.mired_end,
        trigger_info.trigger_d65.mired_start);
      mesh_rolloff_table_interpolate(&(mesh_tbls->left[ISP_ROLLOFF_D65_LIGHT]),
        &(mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT]), tblOut, ratio);
      break;
    case AWB_CCT_TYPE_D65:
      *tblOut = mesh_tbls->left[ISP_ROLLOFF_D65_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84:
    default:
      *tblOut = mesh_tbls->left[ISP_ROLLOFF_TL84_LIGHT];
      break;
  }
} /* mesh_rolloff_calc_awb_trigger */

/** mesh_rolloff_calc_awb_trigger_lowLight:
 *    @mod: Pointer to rolloff module struct
 *    @tblOut: output table after interpolation
 *    @mesh_tbls: Pointer to set of different light tables
 *    @in_params: Pipeline trigger update params
 *
 *  Use color temp and cct_type from awb_update to interpolate between different
 *  low light tables
 *
 *
 *  Return void
 **/
static void mesh_rolloff_calc_awb_trigger_lowLight(isp_mesh_rolloff_mod_t *mod,
  MESH_RollOffTable_V4 *tblOut, isp_rolloff_info_t *mesh_tbls,
  isp_pix_trigger_update_input_t *in_params)
{
  float ratio = 0.0;
  cct_trigger_info trigger_info;
  awb_cct_type cct_type;
  chromatix_parms_type *chrPtr =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  chromatix_VFE_common_type *chrComPtr =
    (chromatix_VFE_common_type *)in_params->cfg.chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  trigger_info.mired_color_temp =
    MIRED(in_params->trigger_input.stats_update.awb_update.color_temp);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A,
    chromatix_rolloff->rolloff_A_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65,
    chromatix_rolloff->rolloff_D65_trigger);

  cct_type = isp_util_get_awb_cct_type(mod->notify_ops->parent, &trigger_info, chrPtr);

  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      *tblOut = mesh_tbls->left[ISP_ROLLOFF_A_LOW_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84_A:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_A.mired_start, trigger_info.trigger_A.mired_end);
      mesh_rolloff_table_interpolate(&(mesh_tbls->left[ISP_ROLLOFF_TL84_LOW_LIGHT]),
        &(mesh_tbls->left[ISP_ROLLOFF_A_LOW_LIGHT]), tblOut, ratio);
      break;
    case AWB_CCT_TYPE_D65_TL84:
      ratio = GET_INTERPOLATION_RATIO(trigger_info.mired_color_temp,
        trigger_info.trigger_d65.mired_end,
        trigger_info.trigger_d65.mired_start);
      mesh_rolloff_table_interpolate(&(mesh_tbls->left[ISP_ROLLOFF_D65_LOW_LIGHT]),
        &(mesh_tbls->left[ISP_ROLLOFF_TL84_LOW_LIGHT]), tblOut, ratio);
      break;
    case AWB_CCT_TYPE_D65:
      *tblOut = mesh_tbls->left[ISP_ROLLOFF_D65_LOW_LIGHT];
      break;
    case AWB_CCT_TYPE_TL84:
    default:
      *tblOut = mesh_tbls->left[ISP_ROLLOFF_TL84_LOW_LIGHT];
      break;
  }
} /* mesh_rolloff_calc_awb_trigger */

/** mesh_rolloff_prepare_hw_table:
 *    @pIn: Input rolloff table to be written to hw
 *    @cmd: output rolloff config cmd
 *
 *  Prepare the config cmd that will be written to hw from the input table
 *
 *
 *  Return void
 **/
static void mesh_rolloff_prepare_hw_table(const MESH_RollOffTable_V4 *pIn,
  MESH_RollOff_V4_ConfigCmdType *cmd)
{
  uint16_t i;
  uint32_t data;

  const uint16_t* Gr = pIn->TableGr;
  const uint16_t* Gb = pIn->TableGb;
  const uint16_t* B =  pIn->TableB;
  const uint16_t* R =  pIn->TableR;

    for (i = 0; i < MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: i=%d, R=%d, Gr=%d\n",
        __func__, i, *R, *Gr);
      cmd->Table.Table[i] = (((uint32_t)(*R)) & 0x00001FFF) |
        (((uint32_t)(*Gr))<<13);
      R++;
      Gr++;
    }
    for (i = MESH_ROLL_OFF_V4_TABLE_SIZE; i < MESH_ROLL_OFF_V4_TABLE_SIZE * 2; i++) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: i=%d, B=%d, Gb=%d\n",
        __func__, i, *B, *Gb);

      cmd->Table.Table[i] = (((uint32_t)(*B)) &0x00001FFF) |
        (((uint32_t)(*Gb))<<13);
      B++;
      Gb++;
    }
} /* mesh_rolloff_prepare_hw_table */

/** mesh_rolloff_cmd_debug:
 *    @cmd: Input cmd that is to be printed
 *
 *  Print the parameters in the rolloff config cmd
 *
 *
 *  Return void
 **/
static void mesh_rolloff_cmd_debug(MESH_RollOff_V4_ConfigCmdType *cmd)
{
  int i;
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: blockWidth = %d, blockHeight = %d, interp_factor = %d", __func__,
    cmd->CfgParams.blockWidth, cmd->CfgParams.blockHeight,
    cmd->CfgParams.interpFactor);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: subGridWidth=%d, subGridHeight=%d\n", __func__,
    cmd->CfgParams.subGridWidth, cmd->CfgParams.subGridHeight);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: subGridXDelta = %d,subGridYDelta=%d\n", __func__,
    cmd->CfgParams.subGridXDelta, cmd->CfgParams.subGridYDelta);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: BlockXIndex=%d, BlockYIndex=%d\n", __func__,
    cmd->CfgParams.blockXIndex, cmd->CfgParams.blockYIndex);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: PixelXIndex=%d, PixelYIndex=%d \n", __func__,
    cmd->CfgParams.PixelXIndex, cmd->CfgParams.PixelYIndex);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: subGridXIndex = %d, subGridYIndex = %d\n", __func__,
    cmd->CfgParams.subGridXIndex, cmd->CfgParams.subGridYIndex);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: yDeltaAccum=%d, pixelOffset = %d pcaLutBankSel =%d\n", __func__,
    cmd->CfgParams.yDeltaAccum, cmd->CfgParams.pixelOffset,
    cmd->CfgParams.pcaLutBankSel);

} /* mesh_rolloff_cmd_debug */

/** mesh_rolloff_calc_sub_grid:
 *    @camif_width:
 *    @camif_height:
 *    @interp_factor: output interpolation factor
 *    @block_width: output block width
 *    @block height: output block height
 *    @sub_grid_width: output sub grid width
 *    @sub_grid_height: output sub grid height
 *
 *  Calculate parameters related to sub grid
 *
 *
 *  Return void
 **/
static void mesh_rolloff_calc_sub_grid(uint16_t camif_width,
  uint16_t camif_height, uint32_t *interp_factor, uint16_t *block_width,
  uint16_t *block_height, uint16_t *sub_grid_width, uint16_t *sub_grid_height)
{
  int SGwidth, BlockWidth, MeshOverWidth;
  int SGheight, BlockHeight, MeshOverHeight;
  int iFactor = INTERP_FACTOR + 1;  // Initial bilinear interpolation
                        //factor: 1 more than maximum 3
  do {
    iFactor--;
    SGwidth = (camif_width + MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS - 1)
      / MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS;  // Ceil
    SGwidth = (SGwidth + (1 << iFactor) - 1) >> iFactor;
    SGwidth = (SGwidth + 1) >> 1;  // Bayer SG width
    BlockWidth = SGwidth << iFactor;  // Bayer block width
    MeshOverWidth = BlockWidth * MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS
      - (camif_width >> 1);
    SGheight = (camif_height + MESH_ROLL_OFF_V4_VERTICAL_GRIDS - 1)
      / MESH_ROLL_OFF_V4_VERTICAL_GRIDS;  // Ceil
    SGheight = (SGheight + (1 << iFactor) - 1) >> iFactor;
    SGheight = (SGheight + 1) >> 1;  // Bayer SG height
    BlockHeight = SGheight << iFactor;  // Bayer block height
    MeshOverHeight = BlockHeight * MESH_ROLL_OFF_V4_VERTICAL_GRIDS
      - (camif_height >> 1);
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Bicubic Size Iteration- \ninterp_factor=%d, SGwidth=%d, SGheight=%d, BlockWidth=%d, BlockHeight=%d, MeshOverWidth=%d, MeshOverHeight=%d\n",
       __func__, iFactor, SGwidth, SGheight, BlockWidth, BlockHeight, MeshOverWidth, MeshOverHeight);
  } while ((iFactor > 0) &&   // Interpolation factor must be >= 0
          ((MeshOverWidth >= BlockWidth) || (SGwidth < 9) ||   // SW & HW constraints
          (MeshOverHeight >= BlockHeight) || (SGheight < 9)));  // SW & HW constraints
  *interp_factor = iFactor;
  *block_width = BlockWidth;
  *block_height = BlockHeight;
  *sub_grid_width = SGwidth;
  *sub_grid_height = SGheight;
}

/** mesh_rolloff_update_hw_table:
 *    @cmd: input config cmd that is to be written to hw
 *    @tableIn: final rolloff table
 *    @pix_settings: pipeline settings
 *
 *  Method invoked at SOF to write config and table to hw
 *
 *
 *  Return void
 **/
static void mesh_rolloff_update_hw_table(MESH_RollOff_V4_ConfigCmdType *cmd,
  MESH_RollOffTable_V4 *tableIn, isp_hw_pix_setting_params_t *pix_settings)
{
  uint32_t row, col, i, camif_width, camif_height;
  uint16_t block_width = 0, block_height = 0;
  uint16_t sub_grid_width = 0, sub_grid_height = 0;
  uint32_t interp_factor = 0;

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: sensor_parms.lastPixel=%d sensor_parms.firstPixel=%d\n", __func__,
    pix_settings->camif_cfg.sensor_out_info.request_crop.last_pixel,
    pix_settings->camif_cfg.sensor_out_info.request_crop.first_pixel);
  ISP_DBG(ISP_MOD_ROLLOFF, "%s: sensor_parms.lastLine=%d sensor_parms.firstLine=%d\n", __func__,
    pix_settings->camif_cfg.sensor_out_info.request_crop.last_line,
    pix_settings->camif_cfg.sensor_out_info.request_crop.first_line);

  camif_width =
     pix_settings->camif_cfg.sensor_out_info.request_crop.last_pixel
       - pix_settings->camif_cfg.sensor_out_info.request_crop.first_pixel + 1;
  camif_height =
     pix_settings->camif_cfg.sensor_out_info.request_crop.last_line
       - pix_settings->camif_cfg.sensor_out_info.request_crop.first_line + 1;

  /* Update RollOffTableConfig command and send command */
  /* Note: Bank selection will be handled in the kernel. */
  cmd->CfgParams.pcaLutBankSel = 0;
  cmd->CfgParams.pixelOffset = 0;

  /* ROLLOFF STRIPE CFG: in non-striping mode, these registers programmed 0*/
  cmd->CfgParams.blockXIndex = 0;
  cmd->CfgParams.blockYIndex = 0;
  cmd->CfgParams.PixelXIndex = 0;
  cmd->CfgParams.PixelYIndex = 0;
  cmd->CfgParams.subGridXIndex = 0;
  cmd->CfgParams.subGridYIndex = 0;

  /*the sub grid here is bayer channel based in stead of pixel base
    bayer_width(height) = pixel_Width(Height)/2, the calc_sub_grid system algo
    already calculate the subgrid based on bayer channel.
    the configuration are all based on Bayer width and height*/
  mesh_rolloff_calc_sub_grid(camif_width, camif_height, &interp_factor,
    &block_width, &block_height, &sub_grid_width, &sub_grid_height);

  cmd->CfgParams.blockWidth = block_width -1;
  cmd->CfgParams.blockHeight = block_height -1;
  cmd->CfgParams.interpFactor = interp_factor;
  cmd->CfgParams.subGridXDelta = (1 << 20) / sub_grid_width;
  cmd->CfgParams.subGridYDelta = (1 << 13) / sub_grid_height;
  cmd->CfgParams.subGridWidth = sub_grid_width - 1;
  cmd->CfgParams.subGridHeight = sub_grid_height - 1;

    /* calculate right stripe settings if needed based on the whole frame setting */
  if (pix_settings->camif_cfg.ispif_out_info.is_split &&
      pix_settings->outputs[0].isp_out_info.stripe_id == ISP_STRIPE_RIGHT) {
    uint32_t block_w      = (cmd->CfgParams.blockWidth + 1) * 2;
    uint32_t subgrid_w    = (cmd->CfgParams.subGridWidth + 1) * 2;
    uint32_t right_offset = cmd->CfgParams.PixelXIndex +
      block_w * cmd->CfgParams.blockXIndex +
      sub_grid_width * cmd->CfgParams.subGridXIndex +
      pix_settings->outputs[0].isp_out_info.right_stripe_offset;

    cmd->CfgParams.blockXIndex   = right_offset / block_w;
    cmd->CfgParams.subGridXIndex =
      (right_offset - cmd->CfgParams.blockXIndex * block_w) / subgrid_w;
    cmd->CfgParams.PixelXIndex   =
      right_offset - cmd->CfgParams.blockXIndex * block_w -
      cmd->CfgParams.subGridXIndex * subgrid_w;
  }
  cmd->CfgParams.yDeltaAccum =
    cmd->CfgParams.PixelYIndex * cmd->CfgParams.subGridYDelta;

  mesh_rolloff_prepare_hw_table(tableIn, cmd);
  mesh_rolloff_cmd_debug(cmd);
} /* mesh_rolloff_update_hw_table */

/** mesh_rolloff_calc_awb_trigger:
 *    @mod: Pointer to rolloff module struct
 *    @tblNormalLight: Normal Light rolloff table
 *    @tblLowLight: Low light rolloff table
 *    @tblOut: output table after interpolation
 *    @trigger_params: Pipeline trigger update params
 *
 *  Use aec ratio from aec_update to interpolate between normal light and low
 *  light tables
 *
 *
 *  Return void
 **/
static void mesh_rolloff_calc_aec_trigger(isp_mesh_rolloff_mod_t *mod,
  MESH_RollOffTable_V4 *tblNormalLight, MESH_RollOffTable_V4 *tblLowLight,
  MESH_RollOffTable_V4 *tblOut, isp_pix_trigger_update_input_t *trigger_params)
{
  float aec_ratio = 0.0;
  int is_burst = IS_BURST_STREAMING((&trigger_params->cfg));
  chromatix_VFE_common_type *chrComPtr =
     (chromatix_VFE_common_type *)
       trigger_params->cfg.chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff_type *chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  aec_ratio = isp_util_get_aec_ratio(mod->notify_ops->parent,
                chromatix_rolloff->control_rolloff,
                &(chromatix_rolloff->rolloff_lowlight_trigger),
                &trigger_params->trigger_input.stats_update.aec_update, is_burst);
  if (F_EQUAL(aec_ratio, 0.0)) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Low Light \n", __func__);
    *tblOut = *tblLowLight;
  } else if (F_EQUAL(aec_ratio, 1.0)) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Bright Light \n", __func__);
    *tblOut = *tblNormalLight;
  } else {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Interpolate between Normal and Low Light \n", __func__);
    mesh_rolloff_table_interpolate(tblNormalLight,
      tblLowLight, tblOut, aec_ratio);
  }
} /* mesh_rolloff_calc_aec-trigger */

/** mesh_rolloff_trigger_update:
 *    @mod: Pointer to rolloff module
 *    @trigger_params: pipeline trigger update params
 *    @in_param_size:
 *
 *  Method invoked when awb, aec, af or flash trigger changes to calculate
 *  new rolloff table
 *
 *
 * return 0 on Success, negative on ERROR
 **/
static int mesh_rolloff_trigger_update(isp_mesh_rolloff_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  int rc = 0;
  float new_real_gain;
  float new_lux_idx;
  float new_mired_color_temp;
  int is_burst = IS_BURST_STREAMING((&trigger_params->cfg));
  cam_flash_mode_t new_flash_mode;

  MESH_RollOffTable_V4 *meshRolloffTableFinal = NULL;
  MESH_RollOffTable_V4 meshRolloffTableNormalLight;
  MESH_RollOffTable_V4 meshRolloffTableLowLight;
  MESH_RollOffTable_V4 meshRolloffTableAEC;
  MESH_RollOffTable_V4 meshRolloffTableInf;

  isp_rolloff_info_t *mesh_tbls = NULL;
  MESH_RollOff_V4_ConfigCmdType *cmd = NULL;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
  return -1;
  }

  if (!mod->mesh_rolloff_enable || !mod->mesh_rolloff_trigger_enable) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: skip trigger update! rolloff enable %d, trigger_enable %d\n",
      __func__, mod->mesh_rolloff_enable, mod->mesh_rolloff_trigger_enable);
    return 0;
  }

  if (trigger_params->trigger_input.stats_update.awb_update.color_temp == 0) {
    CDBG_ERROR("%s: Skip trigger update, Color Temperature is 0.\n", __func__);
    return 0;
  }

  if (!is_burst) {
    if (!isp_util_aec_check_settled(&(trigger_params->trigger_input.stats_update.aec_update))) {
      ISP_DBG(ISP_MOD_ROLLOFF, "%s: AEC is not setteled. Skip the trigger\n", __func__);
      return 0;
    }
  }

  meshRolloffTableFinal =
      &(mod->mesh_rolloff_param.input_table);

  new_real_gain = trigger_params->trigger_input.stats_update.aec_update.real_gain;
  new_lux_idx = trigger_params->trigger_input.stats_update.aec_update.lux_idx;
  new_flash_mode = trigger_params->trigger_input.flash_mode;
  new_mired_color_temp = MIRED(trigger_params->trigger_input.stats_update.awb_update.color_temp);

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: CURRENT gain %f, flash %d, lux %f, NEW gain %f flash %d lux%f\n",
    __func__, mod->cur_real_gain,
    mod->cur_flash_mode,
    mod->cur_lux,
    new_real_gain, new_flash_mode, new_lux_idx);

  if ((F_EQUAL(mod->cur_real_gain, new_real_gain)) &&
    (mod->cur_lux == new_lux_idx) &&
    (mod->cur_flash_mode == new_flash_mode) &&
    (!mod->mesh_rolloff_reload_params) &&
    (mod->cur_mired_color_temp == new_mired_color_temp)&&
    (mod->old_streaming_mode == trigger_params->cfg.streaming_mode)) {
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: No change in trigger. Nothing to update\n", __func__);
    return 0;
  } else {
    mod->cur_real_gain = new_real_gain;
    mod->cur_lux = new_lux_idx;
    mod->cur_flash_mode = new_flash_mode;
    mod->cur_mired_color_temp = new_mired_color_temp;
    ISP_DBG(ISP_MOD_ROLLOFF, "%s: Change in trigger. Update roll-off tables.\n", __func__);
    mod->old_streaming_mode = trigger_params->cfg.streaming_mode;
  }

  mod->hw_update_pending = TRUE;

  mesh_tbls = mod->rolloff_tbls.rolloff_tableset[0];
  /* Note: AWB's CCT interpolated tables are used regardeless Flash
   *       is on or not. So derive them before checking Flash on or not. */
  mesh_rolloff_calc_awb_trigger(mod, &meshRolloffTableNormalLight,
    mesh_tbls, trigger_params);
  mesh_rolloff_calc_awb_trigger_lowLight(mod, &meshRolloffTableLowLight,
    mesh_tbls, trigger_params);

  mesh_rolloff_calc_aec_trigger(mod, &meshRolloffTableNormalLight,
    &meshRolloffTableLowLight, &meshRolloffTableAEC, trigger_params);
  if (new_flash_mode != CAM_FLASH_MODE_OFF) {
    mesh_rolloff_calc_flash_trigger(&meshRolloffTableAEC, &meshRolloffTableInf,
      mesh_tbls, trigger_params);
  } else {
    meshRolloffTableInf = meshRolloffTableAEC;
  }

  mesh_rolloff_table_debug(&meshRolloffTableInf);

  /* Interpolate with AF tbls, Rolloff Macro and Infinity tbls*/
  if (mod->rolloff_tbls.rolloff_tableset[1] == NULL) {
    *meshRolloffTableFinal = meshRolloffTableInf;
  } else {
    MESH_RollOffTable_V4 meshRolloffTableMacro;
    float af_ratio = 0.0;
    int af_start, af_end, af_value;

    mesh_tbls = mod->rolloff_tbls.rolloff_tableset[1];

    mesh_rolloff_calc_awb_trigger(mod, &meshRolloffTableNormalLight,
      mesh_tbls, trigger_params);
    mesh_rolloff_calc_awb_trigger_lowLight(mod, &meshRolloffTableLowLight,
      mesh_tbls, trigger_params);

    mesh_rolloff_calc_aec_trigger(mod, &meshRolloffTableNormalLight,
      &meshRolloffTableLowLight, &meshRolloffTableAEC, trigger_params);
    if (new_flash_mode != CAM_FLASH_MODE_OFF) {
      mesh_rolloff_calc_flash_trigger(&meshRolloffTableAEC, &meshRolloffTableMacro,
        mesh_tbls, trigger_params);
    } else {
      meshRolloffTableMacro = meshRolloffTableAEC;
    }

    mesh_rolloff_table_debug(&meshRolloffTableMacro);

    af_start = mod->af_infinity;
    af_end = mod->af_macro;
    af_value = trigger_params->trigger_input.lens_position_current_step;
    af_ratio = 1.0 - isp_util_calc_interpolation_weight(af_value, af_start, af_end);

    mesh_rolloff_table_interpolate(&meshRolloffTableInf, &meshRolloffTableMacro, meshRolloffTableFinal, af_ratio);
    mesh_rolloff_table_debug(meshRolloffTableFinal);
  }

  cmd = &(mod->mesh_rolloff_cmd);
  mesh_rolloff_prepare_hw_table(meshRolloffTableFinal, cmd);

  return rc;
} /* mesh_rolloff_trigger_update */

/** mesh_rolloff_reset:
 *    @mod: Pointer to rolloff module
 *
 *  Reset the rolloff module
 *
 *
 *  Return void
 **/
static void mesh_rolloff_reset(isp_mesh_rolloff_mod_t *mod)
{
  int i;

  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mod->hw_update_pending = 0;
  memset(&mod->mesh_rolloff_trigger_ratio, 0, sizeof(mod->mesh_rolloff_trigger_ratio));
  memset(&mod->mesh_rolloff_cmd, 0, sizeof(mod->mesh_rolloff_cmd));
  memset(&mod->mesh_rolloff_param, 0, sizeof(mod->mesh_rolloff_param));
  mod->mesh_rolloff_enable = 0;
  mod->mesh_rolloff_update = 0;
  mod->mesh_rolloff_trigger_enable = 0;
  mod->mesh_rolloff_reload_params = 0;
  mod->cur_flash_mode = CAMERA_FLASH_NONE;
  mod->cur_real_gain = 1.0;
  mod->cur_lux = 1.0;
  mod->cur_mired_color_temp = 1.0;
  for(i = 0; i < ISP_ROLLOFF_LENS_POSITION_MAX; i++) {
    if (mod->rolloff_tbls.rolloff_tableset[i]) {
      free(mod->rolloff_tbls.rolloff_tableset[i]);
      mod->rolloff_tbls.rolloff_tableset[i] = NULL;
    }
    if (mod->rolloff_calibration_table.rolloff_tableset[i]) {
      free(mod->rolloff_calibration_table.rolloff_tableset[i]);
      mod->rolloff_calibration_table.rolloff_tableset[i] = NULL;
    }
  }
}

/** mesh_rolloff_init:
 *    @mod_ctrl: Pointer to rolloff module
 *    @in_params: HW module Initialization params
 *    @notify_ops: funtion pointer to notify ops
 *
 *  Initialize the rolloff module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int mesh_rolloff_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_mesh_rolloff_mod_t *mesh_rolloff = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  int i;

  mesh_rolloff->fd = init_params->fd;
  mesh_rolloff->notify_ops = notify_ops;
  mesh_rolloff->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mesh_rolloff->hw_update_pending = FALSE;
  mesh_rolloff_reset(mesh_rolloff);
  return 0;
}/* mesh_rolloff_init */

/** mesh_rolloff_config:
 *    @mesh_mod: Pointer to the rolloff module
 *    @pix_settings: Pipeline settings
 *    @in_param_size:
 *
 *  Allocate the rolloff tables and configure the rolloff module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int mesh_rolloff_config(isp_mesh_rolloff_mod_t *mesh_mod,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  int  rc = 0;
  uint32_t i;
  chromatix_parms_type *chrPtr =
   (chromatix_parms_type *)pix_settings->chromatix_ptrs.chromatixPtr;

  MESH_RollOff_V4_ConfigCmdType *cmd = NULL;

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: E\n",__func__);

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
  return -1;
  }

  /*Get all rolloff tables from chromatix and normalize them*/
  if (0 != rolloff_prepare_tables(mesh_mod, pix_settings)) {
    CDBG_ERROR("%s: rolloff prepare initial table failed\n", __func__);
    return -1;
  }

  ISP_DBG(ISP_MOD_ROLLOFF, "%s: prepare tables done\n", __func__);

  if (!pix_settings->camif_cfg.is_bayer_sensor) {
    CDBG_HIGH("%s: not Bayer Format, not support rolloff\n", __func__);
    return 0;
  }

  /*rollff initial table from chromatix normalized TL84 table */
  cmd = &(mesh_mod->mesh_rolloff_cmd);
  mesh_mod->mesh_rolloff_param.input_table =
    mesh_mod->rolloff_tbls.rolloff_tableset[0]->left[ISP_ROLLOFF_TL84_LIGHT];

  mesh_mod->mesh_rolloff_trigger_ratio.ratio = 0;
  mesh_mod->mesh_rolloff_trigger_ratio.lighting = TRIGGER_NORMAL;

  if (pix_settings->af_rolloff_info.rolloff_tables_macro != NULL) {
    mesh_mod->af_macro = pix_settings->af_rolloff_info.af_macro;
    mesh_mod->af_infinity = pix_settings->af_rolloff_info.af_infinity;
  } else {
    mesh_mod->af_macro = 0;
    mesh_mod->af_infinity = 0;
  }

  mesh_rolloff_table_debug(&mesh_mod->mesh_rolloff_param.input_table);
  mesh_rolloff_update_hw_table(cmd, &(mesh_mod->mesh_rolloff_param.input_table), pix_settings);

  mesh_mod->hw_update_pending = TRUE;

  return rc;
}

/** mesh_rolloff_enable:
 *    @mesh_rolloff: Pointer to rolloff module
 *    @enable: enable flag
 *    @in_param_size:
 *
 *  Enable rolloff module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int mesh_rolloff_enable(isp_mesh_rolloff_mod_t *mesh_rolloff,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  mesh_rolloff->mesh_rolloff_enable = enable->enable;

  return 0;
}

/** mesh_rolloff_trigger_enable:
 *    @mesh_rolloff: Pointer to rolloff module
 *    @enable: enable flag
 *    @in_param_size:
 *
 *  Enable trigger update for rolloff module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int mesh_rolloff_trigger_enable(isp_mesh_rolloff_mod_t *mesh_rolloff,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  mesh_rolloff->mesh_rolloff_trigger_enable = enable->enable;

  return 0;
}

/** mesh_rolloff_destroy:
 *    @mod_ctrl: Pointer to rolloff module
 *
 *  Deallocate and destroy rolloff module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int mesh_rolloff_destroy(void *mod_ctrl)
{
  isp_mesh_rolloff_mod_t *mesh_rolloff = mod_ctrl;
  int i;

  for (i = 0; i < ISP_ROLLOFF_LENS_POSITION_MAX; i++) {
    if (mesh_rolloff->rolloff_tbls.rolloff_tableset[i]) {
      free(mesh_rolloff->rolloff_tbls.rolloff_tableset[i]);
      mesh_rolloff->rolloff_tbls.rolloff_tableset[i] = NULL;
    }
    if (mesh_rolloff->rolloff_calibration_table.rolloff_tableset[i]) {
      free(mesh_rolloff->rolloff_calibration_table.rolloff_tableset[i]);
      mesh_rolloff->rolloff_calibration_table.rolloff_tableset[i] = NULL;
    }
  }
  memset(mesh_rolloff, 0, sizeof(isp_mesh_rolloff_mod_t));
  free(mesh_rolloff);
  return 0;
}

/** mesh_rolloff_set_params:
 *    @mod_ctrl: Pointer to rolloff module struct
 *    @param_id: event id indicating what value is set
 *    @in_params: input event params
 *    @in_param_size: size of input params
 *
 *  Set value for parameter given by param id and pass input params
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int mesh_rolloff_set_params (void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_mesh_rolloff_mod_t *mesh_rolloff = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = mesh_rolloff_enable(mesh_rolloff, (isp_mod_set_enable_t *)in_params,
                     in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = mesh_rolloff_config(mesh_rolloff, (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = mesh_rolloff_trigger_enable(mesh_rolloff, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = mesh_rolloff_trigger_update(mesh_rolloff, (isp_pix_trigger_update_input_t *)in_params, in_param_size);
    break;
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    break;
  }
  return rc;
}

/** mesh_rolloff_get_params:
 *    @mod_ctrl: Pointer to rolloff module struct
 *    @param_id: event id indicating what param to get
 *    @in_params: input params
 *    @in_param_size: Size of Input Params
 *    @out_params: output params
 *    @out_param_size: size of output params
 *
 *  Get value of parameter given by param id
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int mesh_rolloff_get_params (void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size, void *out_params,
  uint32_t out_param_size)
{
  isp_mesh_rolloff_mod_t *mesh_rolloff = mod_ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = mesh_rolloff->mesh_rolloff_enable;
    break;
  }
  case ISP_HW_MOD_GET_ROLLOFF_GRID_INFO:{
    uint32_t *horizontal_grids = out_params;
    if (sizeof(uint32_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }

    *horizontal_grids = MESH_ROLL_OFF_V4_HORIZONTAL_GRIDS;
    break;
  }

  case ISP_HW_MOD_GET_ROLLOFF_TABLE: {
      mct_event_stats_isp_rolloff_t *rolloff_table =
        (mct_event_stats_isp_rolloff_t *)out_params;
      int i;
      if (sizeof(mct_event_stats_isp_rolloff_t) != out_param_size) {
        CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                   __func__, param_id);
        break;
      }
      for(i = 0; i < MCT_MESH_ROLL_OFF_V4_TABLE_SIZE; i++) {
        rolloff_table->TableR[i] =
          mesh_rolloff->mesh_rolloff_param.input_table.TableR[i];
        rolloff_table->TableGr[i] =
          mesh_rolloff->mesh_rolloff_param.input_table.TableGr[i];
        rolloff_table->TableB[i] =
          mesh_rolloff->mesh_rolloff_param.input_table.TableB[i];
        rolloff_table->TableGb[i] =
          mesh_rolloff->mesh_rolloff_param.input_table.TableGb[i];
      }
    }
    break;

  case ISP_HW_MOD_GET_TABLE_SIZE: {
    isp_hw_read_info *read_info = out_params;

    read_info->read_type = VFE_READ_DMI_32BIT;
    read_info->read_bank = ROLLOFF_RAM0_BANK0;
    read_info->bank_idx = 0;

     /* HW pack for R+Gr & Gb+B to be 2 13x10 tbl, uint32_t*/
    read_info->read_lengh =sizeof(uint32_t) * MESH_ROLL_OFF_V4_TABLE_SIZE * 2;
  }
    break;
  case ISP_HW_MOD_GET_DMI_DUMP_USER: {
    isp_hw_read_info *read_info = in_params;
    uint32_t *dmi_dump = (uint32_t *) out_params;

    memcpy(out_params, &mesh_rolloff->applied_table, read_info->read_lengh);
  }
    break;
  default:
    rc = -EPERM;
    break;
  }
  return rc;
}

/** mesh_rolloff_do_hw_update:
 *    @mesh_rolloff_mod: Pointer to rolloff module
 *
 *  Method called at SOF, writes the value in config cmd to HW
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int mesh_rolloff_do_hw_update(isp_mesh_rolloff_mod_t *mesh_rolloff_mod)
{
  int i, rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[6];
  uint32_t rolloff_lut_channel =
    (mesh_rolloff_mod->mesh_rolloff_cmd.CfgParams.pcaLutBankSel == 0)?
      ROLLOFF_RAM0_BANK0 : ROLLOFF_RAM0_BANK1;

  if (mesh_rolloff_mod->hw_update_pending) {
    /* prepare dmi_set and dmi_reset fields */
    mesh_rolloff_mod->mesh_rolloff_cmd.dmi_set[0] =
      ISP40_DMI_CFG_DEFAULT + rolloff_lut_channel;
    mesh_rolloff_mod->mesh_rolloff_cmd.dmi_set[1] = 0;

    mesh_rolloff_mod->mesh_rolloff_cmd.dmi_reset[0] =
      ISP40_DMI_CFG_DEFAULT + ISP40_DMI_NO_MEM_SELECTED;
    mesh_rolloff_mod->mesh_rolloff_cmd.dmi_reset[1] = 0;

    cfg_cmd.cfg_data = (void *) &mesh_rolloff_mod->mesh_rolloff_cmd;
    cfg_cmd.cmd_len = sizeof(mesh_rolloff_mod->mesh_rolloff_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 6;

    /* set dmi to proper linearization bank */
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP40_DMI_CFG_OFF;
    reg_cfg_cmd[0].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[0].u.rw_info.cmd_data_offset +
        reg_cfg_cmd[0].u.rw_info.len;
    reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[1].u.rw_info.reg_offset = ISP40_DMI_ADDR;
    reg_cfg_cmd[1].u.rw_info.len = 1 * sizeof(uint32_t);

    /* send dmi data */
    reg_cfg_cmd[2].cmd_type = VFE_WRITE_DMI_32BIT;
    reg_cfg_cmd[2].u.dmi_info.hi_tbl_offset = 0;
    reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset =
      reg_cfg_cmd[1].u.rw_info.cmd_data_offset +
        reg_cfg_cmd[1].u.rw_info.len;
    reg_cfg_cmd[2].u.dmi_info.len = sizeof(uint32_t) *
        MESH_ROLL_OFF_V4_TABLE_SIZE * 2;

    /* reset dmi to no bank*/
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset +
        reg_cfg_cmd[2].u.dmi_info.len;
    reg_cfg_cmd[3].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[3].u.rw_info.reg_offset = ISP40_DMI_CFG_OFF;
    reg_cfg_cmd[3].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[4].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[3].u.rw_info.cmd_data_offset +
        reg_cfg_cmd[3].u.rw_info.len;
    reg_cfg_cmd[4].cmd_type = VFE_WRITE_MB;
    reg_cfg_cmd[4].u.rw_info.reg_offset = ISP40_DMI_ADDR;
    reg_cfg_cmd[4].u.rw_info.len = 1 * sizeof(uint32_t);

    reg_cfg_cmd[5].u.rw_info.cmd_data_offset =
      reg_cfg_cmd[4].u.rw_info.cmd_data_offset +
        reg_cfg_cmd[4].u.rw_info.len;
    reg_cfg_cmd[5].cmd_type = VFE_WRITE;
    reg_cfg_cmd[5].u.rw_info.reg_offset = ISP_MESH_ROLLOFF40_CFG_OFF;
    reg_cfg_cmd[5].u.rw_info.len = ISP_MESH_ROLLOFF40_CFG_LEN * sizeof(uint32_t);

    mesh_rolloff_cmd_debug(&mesh_rolloff_mod->mesh_rolloff_cmd);
    rc = ioctl(mesh_rolloff_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    mesh_rolloff_mod->applied_table = mesh_rolloff_mod->mesh_rolloff_cmd.Table;
    mesh_rolloff_mod->mesh_rolloff_cmd.CfgParams.pcaLutBankSel ^= 1;
    mesh_rolloff_mod->hw_update_pending = 0;
  }

  return rc;
}

/** mesh_rolloff_action:
 *    @mod_ctrl: Pointer to rolloff module struct
 *    @action_code:  indicates what action is required
 *    @data: input param
 *    @data_size: size of input param
 *
 *  Do the required actions for event given by action code
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int mesh_rolloff_action (void *mod_ctrl, uint32_t action_code,
  void *data, uint32_t data_size)
{
  int rc = 0;
  isp_mesh_rolloff_mod_t *mesh_rolloff = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = mesh_rolloff_do_hw_update(mesh_rolloff);
    break;
  case ISP_HW_MOD_ACTION_RESET:
    mesh_rolloff_reset(mesh_rolloff);
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

/** mesh_rolloff44_open:
 *    @version:
 *
 *  Allocate and Initialize the rolloff module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *mesh_rolloff44_open(uint32_t version)
{
  isp_mesh_rolloff_mod_t *mesh_rolloff = malloc(sizeof(isp_mesh_rolloff_mod_t));

  if (!mesh_rolloff) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(mesh_rolloff,  0,  sizeof(isp_mesh_rolloff_mod_t));
  mesh_rolloff->ops.ctrl = (void *)mesh_rolloff;
  mesh_rolloff->ops.init = mesh_rolloff_init;
  /* destroy the module object */
  mesh_rolloff->ops.destroy = mesh_rolloff_destroy;
  /* set parameter */
  mesh_rolloff->ops.set_params = mesh_rolloff_set_params;
  /* get parameter */
  mesh_rolloff->ops.get_params = mesh_rolloff_get_params;
  mesh_rolloff->ops.action = mesh_rolloff_action;
  return &mesh_rolloff->ops;
}


