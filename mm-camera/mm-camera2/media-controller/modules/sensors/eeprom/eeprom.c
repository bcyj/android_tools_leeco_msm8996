/* eeprom.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <dlfcn.h>
#include <math.h>
#include <stdlib.h>

#include "eeprom.h"
#include "sensor_common.h"
#include "server_debug.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif
#define BUFF_SIZE_255 255

#define MIN(a,b)\
  ((a) > (b) ? (b) : (a))

static void swap(pixel_t *m, pixel_t *n)
{
  pixel_t temp;
  temp.x = m->x;
  temp.y = m->y;
  *m = *n;
  n->x = temp.x;
  n->y = temp.y;
}

/** adjust_step_table:
 *    @e_ctrl: address of pointer to
 *                   step_size_t struct
 *             adjusting ratio
 *
 * This function adjust TAF_table and CAF_table
 *
 * Return:
 * void
 **/
void adjust_step_table(const char *str, step_size_t *light, float ratio) {
  SLOW("%s: before adjust %s scan step: %d %d %d %d %d", __func__, str,
  light->rgn_0, light->rgn_1, light->rgn_2, light->rgn_3, light->rgn_4);
  light->rgn_0 =(unsigned short)round(light->rgn_0 * ratio);
  light->rgn_1 =(unsigned short)round(light->rgn_1 * ratio);
  light->rgn_2 =(unsigned short)round(light->rgn_2 * ratio);
  light->rgn_3 =(unsigned short)round(light->rgn_3 * ratio);
  light->rgn_4 =(unsigned short)round(light->rgn_4 * ratio);
  SLOW("%s: after adjust %s scan step: %d %d %d %d %d", __func__, str,
  light->rgn_0, light->rgn_1, light->rgn_2, light->rgn_3, light->rgn_4);
}

/** eeprom_autofocus_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs autofocus calibration assuming 2 regions
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

boolean eeprom_autofocus_calibration(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *) e_ctrl;
  float                   adjust_ratio = 1;
  int32_t                 i = 0, j = 0;
  step_size_table_t       *table;
  actuator_tuned_params_t *af_driver_tune = NULL;
  af_tuning_algo_t        *af_algo_tune = NULL;
  uint32_t                total_steps = 0;

  /* Validate params */
  RETURN_ON_NULL(e_ctrl);
  RETURN_ON_NULL(ectrl->eeprom_afchroma.af_driver_ptr);

  af_driver_tune =
    &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_tuned_params);
  /* Get the total steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
    step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  if (!total_steps) {
    SERR("Invalid total_steps count = 0");
    return FALSE;
  }

  /* Calculation adjust ratio */
  adjust_ratio = (float)(ectrl->eeprom_data.afc.macro_dac -
    ectrl->eeprom_data.afc.starting_dac) / (float)total_steps;

  for (i = 0; i < af_driver_tune->region_size; i++) {
    af_driver_tune->region_params[i].step_bound[0] =
      (unsigned short)round(
      af_driver_tune->region_params[i].step_bound[0] * adjust_ratio);
    af_driver_tune->region_params[i].step_bound[1] =
      (unsigned short)round(
      af_driver_tune->region_params[i].step_bound[1] * adjust_ratio);
  }

  SLOW("adjust_ratio = %f", adjust_ratio);
  /* Get the calibrated steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
      step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  /* adjust af_driver_ptr */
  af_driver_tune->initial_code = ectrl->eeprom_data.afc.starting_dac;

  SLOW("ENTER");
  /* adjust af_algo_ptr */
  /* adjust af_algo_cam and af_algo_camcorder individually */
  for (j = 0; j < 2; j++){
    af_algo_tune = &(ectrl->eeprom_afchroma.af_algo_ptr[j]->af_algo);

    af_algo_tune->position_far_end =
      (unsigned short)round(af_algo_tune->position_far_end * adjust_ratio);
    af_algo_tune->position_near_end =
      (unsigned short)round(af_algo_tune->position_near_end * adjust_ratio);
    af_algo_tune->position_default_in_macro =
      (unsigned short)round(
      af_algo_tune->position_default_in_macro * adjust_ratio);
    af_algo_tune->position_default_in_normal =
      (unsigned short)round(
      af_algo_tune->position_default_in_normal * adjust_ratio);
    af_algo_tune->position_normal_hyperfocal =
      (unsigned short)round(
      af_algo_tune->position_normal_hyperfocal * adjust_ratio);
    af_algo_tune->position_macro_rgn =
      (unsigned short)round(af_algo_tune->position_macro_rgn * adjust_ratio);
    af_algo_tune->position_boundary =
      (unsigned short)round(af_algo_tune->position_boundary * adjust_ratio);
    for (i = SINGLE_NEAR_LIMIT_IDX; i < SINGLE_MAX_IDX; i++){
      SLOW("before adjust %s: step_index[%d]: %d", \
        __func__, i, af_algo_tune->af_single.index[i]);
      af_algo_tune->af_single.index[i] =
        (unsigned short)round(af_algo_tune->af_single.index[i] * adjust_ratio);
      SLOW("after adjust %s: step_index[%d]: %d", \
        __func__, i, af_algo_tune->af_single.index[i]);
    }
    SLOW("%s: TAF_step_table:", __func__);
    table = &(af_algo_tune->af_single.TAF_step_table);
    adjust_step_table("Prescan_normal_light",
      &table->Prescan_normal_light, adjust_ratio);
    adjust_step_table("Prescan_low_light",
      &table->Prescan_low_light, adjust_ratio);
    adjust_step_table("Finescan_normal_light",
      &table->Finescan_normal_light, adjust_ratio);
    adjust_step_table("Finescan_low_light",
      &table->Finescan_low_light, adjust_ratio);
    SLOW("%s: CAF_step_table:", __func__);
    table = &(af_algo_tune->af_single.CAF_step_table);
    adjust_step_table("Prescan_normal_light",
      &table->Prescan_normal_light, adjust_ratio);
    adjust_step_table("Prescan_low_light",
      &table->Prescan_low_light, adjust_ratio);
    adjust_step_table("Finescan_normal_light",
      &table->Finescan_normal_light, adjust_ratio);
    adjust_step_table("Finescan_low_light",
      &table->Finescan_low_light, adjust_ratio);
  }

  SLOW("Exit");
  return TRUE;
}

/** wbgain_calibration:
 *    @gain: gain of the whitebalance
 *    @calib_factor: calibration factor for whhitebalance
 *
 * Helper function for whitebalace calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

static float wbgain_calibration(float gain, float calib_factor)
{
  /* gain = 1 / [(1/gain) * calibration_r_over_g_factor] */
  return gain / calib_factor;
}

/** eeprom_whitebalance_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs whitebalace calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

void eeprom_whitebalance_calibration(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  chromatix_parms_type *chromatix =
    ectrl->eeprom_afchroma.chromatix.chromatix_ptr;
  wbcalib_data_t *wbc = &(ectrl->eeprom_data.wbc);
  float r_gain, b_gain, g_gain, min_gain, gr_over_gb;
  uint32_t indx;
  float r_over_g_calib_factor[AGW_AWB_MAX_LIGHT];
  float b_over_g_calib_factor[AGW_AWB_MAX_LIGHT];

  /* Apply calibration if only it is not calibrated already */
  if (!chromatix->AWB_algo_data.enable_AWB_module_cal){

  /* Use unused variables in aec chromatix to send calibration factor to AWBi,
  set to 1.0f */
  chromatix->AEC_algo_data.aec_led_flux_med = 1.0f;
  chromatix->AEC_algo_data.aec_led_flux_low = 1.0f;

  /* Calibrate the AWB in chromatix based on color measurement read */
  for (indx = 0; indx < AGW_AWB_MAX_LIGHT; indx++) {
    r_over_g_calib_factor[indx] = wbc->r_over_g[indx] /
      chromatix->AWB_algo_data.AWB_golden_module_R_Gr_ratio[indx];
    b_over_g_calib_factor[indx] = wbc->b_over_g[indx] /
      chromatix->AWB_algo_data.AWB_golden_module_B_Gr_ratio[indx];

    chromatix->AWB_bayer_algo_data.reference[indx].RG_ratio *=
      r_over_g_calib_factor[indx];
    chromatix->AWB_bayer_algo_data.reference[indx].BG_ratio *=
      b_over_g_calib_factor[indx];
  }

  /* Calibrate the MWB in chromatix based on color measurement read */
  /* MWB TL84 */
  r_gain = wbgain_calibration(
    chromatix->chromatix_MWB.MWB_tl84.r_gain,
    r_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  b_gain = wbgain_calibration(
    chromatix->chromatix_MWB.MWB_tl84.b_gain,
    b_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  g_gain = chromatix->chromatix_MWB.MWB_tl84.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_MWB.MWB_tl84.r_gain = r_gain / min_gain;
  chromatix->chromatix_MWB.MWB_tl84.g_gain = g_gain / min_gain;
  chromatix->chromatix_MWB.MWB_tl84.b_gain = b_gain / min_gain;

  /* MWB D50 */
  r_gain = wbgain_calibration(
    chromatix->chromatix_MWB.MWB_d50.r_gain,
    r_over_g_calib_factor[AGW_AWB_D50]);
  b_gain = wbgain_calibration(
    chromatix->chromatix_MWB.MWB_d50.b_gain,
    b_over_g_calib_factor[AGW_AWB_D50]);
  g_gain = chromatix->chromatix_MWB.MWB_d50.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_MWB.MWB_d50.r_gain = r_gain / min_gain;
  chromatix->chromatix_MWB.MWB_d50.g_gain = g_gain / min_gain;
  chromatix->chromatix_MWB.MWB_d50.b_gain = b_gain / min_gain;

  /* MWB Incandescent */
  r_gain = wbgain_calibration(
    chromatix->chromatix_MWB.MWB_A.r_gain,
    r_over_g_calib_factor[AGW_AWB_A]);
  b_gain = wbgain_calibration(
    chromatix->chromatix_MWB.MWB_A.b_gain,
    b_over_g_calib_factor[AGW_AWB_A]);
  g_gain = chromatix->chromatix_MWB.MWB_A.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_MWB.MWB_A.r_gain =
    r_gain / min_gain;
  chromatix->chromatix_MWB.MWB_A.g_gain =
    g_gain / min_gain;
  chromatix->chromatix_MWB.MWB_A.b_gain =
    b_gain / min_gain;

  /* MWB D65 */
  r_gain = wbgain_calibration(
    chromatix->chromatix_MWB.MWB_d65.r_gain,
     r_over_g_calib_factor[AGW_AWB_D65]);
  b_gain = wbgain_calibration(
    chromatix->chromatix_MWB.MWB_d65.b_gain,
     b_over_g_calib_factor[AGW_AWB_D65]);
  g_gain = chromatix->chromatix_MWB.MWB_d65.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_MWB.MWB_d65.r_gain = r_gain / min_gain;
  chromatix->chromatix_MWB.MWB_d65.g_gain = g_gain / min_gain;
  chromatix->chromatix_MWB.MWB_d65.b_gain = b_gain / min_gain;

  /* MWB Strobe */
  r_gain = wbgain_calibration(
    chromatix->chromatix_MWB.strobe_flash_white_balance.r_gain,
     r_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  b_gain = wbgain_calibration(
    chromatix->chromatix_MWB.strobe_flash_white_balance.b_gain,
     b_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  g_gain = chromatix->chromatix_MWB.strobe_flash_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_MWB.strobe_flash_white_balance.r_gain =
   r_gain / min_gain;
  chromatix->chromatix_MWB.strobe_flash_white_balance.g_gain =
    g_gain / min_gain;
  chromatix->chromatix_MWB.strobe_flash_white_balance.b_gain =
    b_gain / min_gain;

  /* MWB LED flash */
  r_gain = wbgain_calibration(
    chromatix->chromatix_MWB.led_flash_white_balance.r_gain,
    r_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  b_gain = wbgain_calibration(
    chromatix->chromatix_MWB.led_flash_white_balance.b_gain,
   b_over_g_calib_factor[AGW_AWB_WARM_FLO]);
  g_gain = chromatix->chromatix_MWB.led_flash_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_MWB.led_flash_white_balance.r_gain =
    r_gain / min_gain;
  chromatix->chromatix_MWB.led_flash_white_balance.g_gain =
   g_gain / min_gain;
  chromatix->chromatix_MWB.led_flash_white_balance.b_gain =
   b_gain / min_gain;

  /* Gr/Gb channel balance */
  gr_over_gb = wbc->gr_over_gb;
  /* To compensate invert the measured gains */
  b_gain = gr_over_gb;
  r_gain = 1;
  SLOW("%s: gb_gain: %f, gr_gain: %f", __func__, b_gain, r_gain);
  min_gain = MIN(r_gain, b_gain);
  SLOW("%s: min_gain: %f", __func__, min_gain);
  chromatix->chromatix_VFE.chromatix_channel_balance_gains.green_even =
    r_gain/min_gain;
  chromatix->chromatix_VFE.chromatix_channel_balance_gains.green_odd =
    b_gain/min_gain;

    chromatix->AWB_algo_data.enable_AWB_module_cal = 1;
    }
  return;
}

/** eeprom_print_matrix:
 *    @paramlist: address of pointer to
 *                   chromatix struct
 *
 * Prints out debug logs
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
void eeprom_print_matrix(float* paramlist)
{
  int j =0;

  for(j=0; j < MESH_ROLLOFF_SIZE; j = j+17) {
    SLOW("%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, \
%.1f, %.1f, %.1f, %.1f, %.1f, %.1f",
      paramlist[j], paramlist[j+1], paramlist[j+2], paramlist[j+3],
      paramlist[j+4], paramlist[j+5], paramlist[j+6],  paramlist[j+7],
      paramlist[j+8], paramlist[j+9], paramlist[j+10], paramlist[j+11],
      paramlist[j+12], paramlist[j+13], paramlist[j+14], paramlist[j+15],
      paramlist[j+16]);
  }
}

/** eeprom_lensshading_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * Performs lenshading calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

void eeprom_lensshading_calibration(void *e_ctrl)
{
  int i, j, index;
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  chromatix_rolloff_type *chromatix =
    &(ectrl->eeprom_afchroma.chromatix.common_chromatix_ptr->chromatix_rolloff);
  lsccalib_data_t *lsc = &ectrl->eeprom_data.lsc;
  SLOW( ": Enter" );
  for (j = 0; j < ROLLOFF_MAX_LIGHT; j++) {
    for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
      chromatix->chromatix_mesh_rolloff_table[j].r_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].r_gain[i] /
        lsc->lsc_calib[j].r_gain[i]);
      chromatix->chromatix_mesh_rolloff_table[j].b_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].b_gain[i] /
        lsc->lsc_calib[j].b_gain[i]);
      chromatix->chromatix_mesh_rolloff_table[j].gr_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gr_gain[i] /
        lsc->lsc_calib[j].gr_gain[i]);
      chromatix->chromatix_mesh_rolloff_table[j].gb_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gb_gain[i] /
        lsc->lsc_calib[j].gb_gain[i]);


      chromatix->chromatix_mesh_rolloff_table_lowlight[j].r_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].r_gain[i] /
        lsc->lsc_calib[j].r_gain[i]);
      chromatix->chromatix_mesh_rolloff_table_lowlight[j].b_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].b_gain[i] /
        lsc->lsc_calib[j].b_gain[i]);
      chromatix->chromatix_mesh_rolloff_table_lowlight[j].gr_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gr_gain[i] /
        lsc->lsc_calib[j].gr_gain[i]);
      chromatix->chromatix_mesh_rolloff_table_lowlight[j].gb_gain[i] *=
        (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gb_gain[i] /
        lsc->lsc_calib[j].gb_gain[i]);
    }
  }

  j= ROLLOFF_D65_LIGHT;
  for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
    chromatix->chromatix_mesh_rolloff_table_LED.r_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].r_gain[i] /
      lsc->lsc_calib[j].r_gain[i]);
    chromatix->chromatix_mesh_rolloff_table_LED.b_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].b_gain[i] /
      lsc->lsc_calib[j].b_gain[i]);
    chromatix->chromatix_mesh_rolloff_table_LED.gr_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gr_gain[i] /
      lsc->lsc_calib[j].gr_gain[i]);
    chromatix->chromatix_mesh_rolloff_table_LED.gb_gain[i] *=
      (chromatix->chromatix_mesh_rolloff_table_golden_module[j].gb_gain[i] /
      lsc->lsc_calib[j].gb_gain[i]);
  }

  SLOW("CHROMATIX LSC MATRICES");
  for (i = 0; i < ROLLOFF_MAX_LIGHT; i++) {
    SLOW("chromatix_mesh_rolloff_table[%d] FINAL R MATRIX", i);
    eeprom_print_matrix(chromatix->chromatix_mesh_rolloff_table[i].r_gain);

    SLOW("chromatix_mesh_rolloff_table[%d] FINAL GR MATRIX", i);
    eeprom_print_matrix(chromatix->chromatix_mesh_rolloff_table[i].gr_gain);

    SLOW("chromatix_mesh_rolloff_table[%d] FINAL GB MATRIX", i);
    eeprom_print_matrix(chromatix->chromatix_mesh_rolloff_table[i].gb_gain);

    SLOW("chromatix_mesh_rolloff_table[%d] FINAL B MATRIX", i);
    eeprom_print_matrix(chromatix->chromatix_mesh_rolloff_table[i].b_gain);
  }
  SLOW( ": Exit" );
}


/** eeprom_defectpixcel_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * Performs defectpixel calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

void eeprom_defectpixcel_calibration(void *ctrl)
{
  int i, j;
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)ctrl;
  dpccalib_data_t *dpc = &(ectrl->eeprom_data.dpc);

  int count = dpc->validcount;

  for (i = 0; i < count-1; i++)
    for (j = 0; j < (count-(i+1)); j++) {
      if (dpc->snapshot_coord[j].x > dpc->snapshot_coord[j+1].x)
        swap(&dpc->snapshot_coord[j], &dpc->snapshot_coord[j+1]);
      if (dpc->preview_coord[j].x > dpc->preview_coord[j+1].x)
        swap(&dpc->preview_coord[j], &dpc->preview_coord[j+1]);
      if (dpc->video_coord[j].x > dpc->video_coord[j+1].x)
        swap(&dpc->video_coord[j], &dpc->video_coord[j+1]);
    }

  for (i = 0; i < count-1; i++)
    for (j = 0; j < (count-(i+1)); j++) {
      if (dpc->snapshot_coord[j].x == dpc->snapshot_coord[j+1].x)
        if (dpc->snapshot_coord[j].y > dpc->snapshot_coord[j+1].y)
          swap(&dpc->snapshot_coord[j], &dpc->snapshot_coord[j+1]);
      if (dpc->preview_coord[j].x == dpc->preview_coord[j+1].x)
        if (dpc->preview_coord[j].y > dpc->preview_coord[j+1].y)
          swap(&dpc->preview_coord[j], &dpc->preview_coord[j+1]);
      if (dpc->video_coord[j].x == dpc->video_coord[j+1].x)
        if (dpc->video_coord[j].y > dpc->video_coord[j+1].y)
          swap(&dpc->video_coord[j], &dpc->video_coord[j+1]);
    }
  SLOW("%s Accending order\n", __func__);
  for (i = 0; i < count; i++)
    SLOW("dpc->snapshot_coord[%d] X= %d Y = %d\n",
      i, dpc->snapshot_coord[i].x, dpc->snapshot_coord[i].y);
  for (i = 0; i < count; i++)
    SLOW("dpc->preview_coord[%d] X= %d Y = %d\n",
      i, dpc->preview_coord[i].x, dpc->preview_coord[i].y);
  for (i = 0; i < count; i++)
    SLOW("dpc->video_coord[%d] X= %d Y = %d\n",
      i, dpc->video_coord[i].x, dpc->video_coord[i].y);
}


/** eeprom_get_dpc_calibration_info:
 *    @ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *    @type: type of info requested
 *    @info: return address of the dpc info
 *
 * performs defectpixel calibration
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

void eeprom_get_dpc_calibration_info(void *ctrl, int type, void *info)
{
  sensor_eeprom_data_t *e_ctrl = (sensor_eeprom_data_t *)ctrl;
  pixels_array_t *dpc_info = (pixels_array_t *)info;
  dpccalib_data_t *dpc = &e_ctrl->eeprom_data.dpc;
    dpc_info->count = dpc->validcount;
//    dpc_info->pix = (pixel_t *)eeprom_dpc_data[type];
}


/** eeprom_do_chroma_calibration:
 *    @ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * Kicks off the calibration process
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

static void eeprom_do_chroma_calibration(void *ctrl)
{
  /* Selectively access the calibration data by setprop value.
   * As a default, we always access. */
  char                   value[PROPERTY_VALUE_MAX];
  sensor_eeprom_data_t  *e_ctrl = (sensor_eeprom_data_t *)ctrl;
  eeprom_calib_items_t  *e_items = &e_ctrl->eeprom_data.items;
  boolean                access_wb = 1;
  boolean                access_ls = 1;
  boolean                access_dp = 1;

  SLOW("Enter");

  if (property_get("persist.camera.cal.awb", value, "1")) {
    access_wb = (atoi(value) == 1)? TRUE:FALSE;
  }
  if (property_get("persist.camera.cal.ls", value, "1")) {
    access_ls = (atoi(value) == 1)? TRUE:FALSE;
  }
  if (property_get("persist.camera.cal.ls", value, "1")) {
    access_dp = (atoi(value) == 1)? TRUE:FALSE;
  }

  SERR("turn on Cal wb(%d) ls(%d) dp(%d)",
    access_wb, access_ls, access_dp);

  if (e_items->is_wbc &&
      e_ctrl->eeprom_afchroma.chromatix.chromatix_reloaded)
    if (e_ctrl->eeprom_lib.func_tbl->do_wbc_calibration != NULL)
      e_ctrl->eeprom_lib.func_tbl->do_wbc_calibration(e_ctrl);

  if (e_items->is_lsc &&
      e_ctrl->eeprom_afchroma.chromatix.chromatix_common_reloaded)
    if (e_ctrl->eeprom_lib.func_tbl->do_lsc_calibration != NULL)
      e_ctrl->eeprom_lib.func_tbl->do_lsc_calibration(e_ctrl);

  if (e_items->is_dpc)
    if (e_ctrl->eeprom_lib.func_tbl->do_dpc_calibration != NULL)
      e_ctrl->eeprom_lib.func_tbl->do_dpc_calibration(e_ctrl);
  SLOW("Exit");
}

/** eeprom_do_af_calibration:
 *    @ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * Kicks off the calibration process
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

static boolean eeprom_do_af_calibration(void *ctrl)
{
  sensor_eeprom_data_t *e_ctrl = (sensor_eeprom_data_t *)ctrl;
  eeprom_calib_items_t *e_items = &e_ctrl->eeprom_data.items;
  char                  value[PROPERTY_VALUE_MAX];

  /* Selectively access the calibration data by setprop value.
     * As a default, we always access. */
  boolean               access_af = 1;

  /* Validate params */
  RETURN_ON_NULL(e_ctrl);

  SLOW("Enter");

  if (property_get("persist.camera.cal.af", value, "1")) {
    access_af = (atoi(value) == 1)? TRUE:FALSE;
  }
  SERR("turn on Cal af(%d)", access_af);

  if (access_af && e_items->is_afc)
    if (e_ctrl->eeprom_lib.func_tbl->do_af_calibration != NULL)
      e_ctrl->eeprom_lib.func_tbl->do_af_calibration(ctrl);

  SLOW("Exit");
  return TRUE;
}

/** eeprom_get_raw_data:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *    @data: pointer to the raw data
 *
 * get the raw data from eeprom
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t eeprom_get_raw_data(void *e_ctrl, void *data) {
  sensor_eeprom_data_t *ep = (sensor_eeprom_data_t *)e_ctrl;

  SLOW("Enter");
  if(ep->eeprom_lib.func_tbl->get_raw_data != NULL)
    ep->eeprom_lib.func_tbl->get_raw_data(ep, data);
  SLOW("Exit");
  return 0;
}

/** eeprom_load_library:
 *    @name: eeprom device library name
 *    @data: eeprom library parameters
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t eeprom_load_library(sensor_eeprom_data_t *e_ctrl)
{
  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = {0};
  void *(*eeprom_open_lib)(void) = NULL;
  const char *name = e_ctrl->eeprom_params.eeprom_name;

  SLOW("enter");
  snprintf(lib_name, sizeof(lib_name), "libmmcamera_%s_eeprom.so", name);
  SHIGH("%s lib_name %s",__func__, lib_name);
  e_ctrl->eeprom_lib.eeprom_lib_handle = dlopen(lib_name, RTLD_NOW);
  if (!e_ctrl->eeprom_lib.eeprom_lib_handle) {
    SERR("failed");
    return -EINVAL;
  }

  snprintf(open_lib_str, sizeof(open_lib_str), "%s_eeprom_open_lib", name);
  *(void **)&eeprom_open_lib  = dlsym(e_ctrl->eeprom_lib.eeprom_lib_handle,
    open_lib_str);
  if (!eeprom_open_lib) {
    SERR("failed");
    return -EINVAL;
  }

  e_ctrl->eeprom_lib.func_tbl = (eeprom_lib_func_t *)eeprom_open_lib();
  if (!e_ctrl->eeprom_lib.func_tbl) {
    SERR("failed");
    return -EINVAL;
  }

  SERR("e_ctrl->eeprom_lib.func_tbl =%p",e_ctrl->eeprom_lib.func_tbl);

  SLOW("exit");
  return 0;
}

/** eeprom_unload_library:
 *    @eeprom_lib_params: eeprom library parameters
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t eeprom_unload_library(eeprom_lib_params_t *eeprom_lib_params)
{
  if (!eeprom_lib_params) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  if (eeprom_lib_params->eeprom_lib_handle) {
    dlclose(eeprom_lib_params->eeprom_lib_handle);
    eeprom_lib_params->eeprom_lib_handle = NULL;
  }
  return SENSOR_SUCCESS;
}

/** eeprom_get_info:
 *    @ptr: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * 1) Gets the details about the number bytes read in kernel
 * 2) Reads the actual data from the kerenl.
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int eeprom_get_info(void *ptr)
{
  int rc = 0;
  sensor_eeprom_data_t *ep = (sensor_eeprom_data_t *)ptr;
  struct msm_eeprom_cfg_data cfg;
  uint32_t i;
  if (ep == NULL) {
    SERR("Invalid Argument -eeprom data");
    return -EINVAL;
  }

  cfg.cfgtype = CFG_EEPROM_GET_INFO;
  cfg.is_supported = 0;
  rc = ioctl(ep->fd, VIDIOC_MSM_EEPROM_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_EEPROM_CFG(%d) failed!", ep->fd);
    return rc;
  }
  ep->eeprom_params.is_supported = cfg.is_supported;
  memcpy(ep->eeprom_params.eeprom_name, cfg.cfg.eeprom_name,
     sizeof(ep->eeprom_params.eeprom_name));

  if (cfg.is_supported) {
    SLOW("kernel returned eeprom supported name = %s\n", cfg.cfg.eeprom_name);
    cfg.cfgtype = CFG_EEPROM_GET_CAL_DATA;
    rc = ioctl(ep->fd, VIDIOC_MSM_EEPROM_CFG, &cfg);
    if (rc < 0) {
      SERR("VIDIOC_MSM_EEPROM_CFG(%d) failed!", ep->fd);
      return rc;
    }
    SLOW("kernel returned num_bytes =%d\n", cfg.cfg.get_data.num_bytes);
    ep->eeprom_params.num_bytes = cfg.cfg.get_data.num_bytes;
    if (ep->eeprom_params.num_bytes) {
      ep->eeprom_params.buffer = (uint8_t *)malloc(ep->eeprom_params.num_bytes);
      if (!ep->eeprom_params.buffer){
        SERR("%s failed allocating memory\n",__func__);
        rc = -ENOMEM;
        return rc;
      }
    cfg.cfgtype = CFG_EEPROM_READ_CAL_DATA;
    cfg.cfg.read_data.num_bytes = ep->eeprom_params.num_bytes;
    cfg.cfg.read_data.dbuffer = ep->eeprom_params.buffer;
    rc = ioctl(ep->fd, VIDIOC_MSM_EEPROM_CFG, &cfg);
    if (rc < 0) {
      SERR("CFG_EEPROM_READ_CAL_DATA(%d) failed!", ep->fd);
      return rc;
    }
    SLOW("kernel returned read buffer =%p\n", cfg.cfg.read_data.dbuffer);
    }
  }
  return rc;
}

/** eeprom_open:
 *    @eeprom_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *    @subdev_name: EEPROM subdev name
 *
 * 1) Allocates memory for EEPROM control structure
 * 2) Opens EEPROM subdev node
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_open(void **eeprom_ctrl, const char *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_eeprom_data_t *ctrl = NULL;
  char subdev_string[32];

  SLOW("Enter");
  if (!eeprom_ctrl || !subdev_name) {
    SERR("failed sctrl %p subdev name %p",
      eeprom_ctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  ctrl = malloc(sizeof(sensor_eeprom_data_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(sensor_eeprom_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", subdev_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if ((ctrl->fd) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    ctrl->fd = -1;
    goto ERROR;
  }
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR;
  }
  *eeprom_ctrl = (void *)ctrl;
  SLOW("Exit");
  return rc;

ERROR:
  free(ctrl);
  return rc;
}

/** eeprom_open_fd:
 *    @ctrl: EEEPROM control handle
 *    @subdev_name: EEPROM subdev name
 *
 * opens the eeprom fd
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_open_fd( sensor_eeprom_data_t *ctrl, const char *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  char subdev_string[32];

  SLOW("Enter");
  if (!ctrl || !subdev_name) {
    SERR("failed sctrl %p subdev name %p",
      ctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", subdev_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if ((ctrl->fd) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    ctrl->fd = -1;
    rc = SENSOR_FAILURE;
  }
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
  }
  SLOW("Exit");
  return rc;
}

/** eeprom_close_fd:
 *    @e_ctrl: EEEPROM control handle
 *
 * close the eeprom fd
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_close_fd(sensor_eeprom_data_t *ctrl)
{
  int32_t rc = SENSOR_SUCCESS;

  SLOW("Enter");

  /* close subdev */
  close(ctrl->fd);
  SLOW("Exit");
  return rc;
}

/** eeprom_set_bytestream:
 *    @e_ctrl: EEEPROM control handle
 *    @e_params: bytestream pointer
 *
 * Sets the bytestream pointer which has the eeprom data
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_set_bytestream(
   sensor_eeprom_data_t *e_ctrl, eeprom_params_t *e_params) {

  int32_t rc = SENSOR_SUCCESS;
  uint32_t i;
  SLOW("Enter");
  if (!e_params) {
      SERR("failed");
    return SENSOR_FAILURE;
  }
  e_ctrl->eeprom_params = *e_params;

  SLOW("e_ctrl->eeprom_params.num_bytes =%d", e_ctrl->eeprom_params.num_bytes);
  for (i = 0; i < e_ctrl->eeprom_params.num_bytes; i++)
    SLOW("e_ctrl->eeprom_params 0x%X", e_ctrl->eeprom_params.buffer[i]);

  SLOW("Exit");
  return rc;
}

/** eeprom_set_chromatix_af_pointer:
 *    @e_ctrl: EEEPROM control handle
 *    @e_params: chromatix and af tuning pointer
 *
 * Sets the chromatix and af tuning pointers for calibration
 * purpose
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_set_chromatix_af_pointer(
   sensor_eeprom_data_t *e_ctrl, eeprom_set_chroma_af_t *e_params) {

  int32_t rc = SENSOR_SUCCESS;
  int i;
  SLOW("Enter");
  if (!e_params) {
      SERR("failed");
    return SENSOR_FAILURE;
  }
  e_ctrl->eeprom_afchroma = *e_params;
  SLOW("chromatix pointer =%p", e_ctrl->eeprom_afchroma.chromatix.chromatix_ptr);
  SLOW("common chromatix pointer =%p", e_ctrl->eeprom_afchroma.chromatix.common_chromatix_ptr);

  SLOW("Exit");
  return rc;
}

/** eeprom_process:
 *    @eeprom_ctrl: EEEPROM control handle
 *    @event: configuration event type
 *    @data: NULL
 *
 * Handle all EERPOM events
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_process(void *eeprom_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  if (!eeprom_ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  sensor_eeprom_data_t *e_ctrl = (sensor_eeprom_data_t *)eeprom_ctrl;
  switch(event) {
  case EEPROM_OPEN_FD:
      rc = eeprom_open_fd(e_ctrl, data);
      break;
  case EEPROM_CLOSE_FD:
      rc = eeprom_close_fd(e_ctrl);
      break;
  case EEPROM_READ_DATA:
      rc = eeprom_get_info(e_ctrl);
      break;
  case EEPROM_SET_BYTESTREAM:
      rc = eeprom_set_bytestream(e_ctrl, data);
      if(e_ctrl->eeprom_params.is_supported)
        eeprom_load_library(e_ctrl);
      break;
  case EEPROM_SET_FORMAT_DATA:
      if (e_ctrl->eeprom_lib.func_tbl &&
          e_ctrl->eeprom_params.is_supported) {
        if (e_ctrl->eeprom_lib.func_tbl->format_calibration_data != NULL)
          e_ctrl->eeprom_lib.func_tbl->format_calibration_data(e_ctrl);
        if (e_ctrl->eeprom_lib.func_tbl->get_calibration_items != NULL)
          e_ctrl->eeprom_lib.func_tbl->get_calibration_items(e_ctrl);
      }
      break;
  case EEPROM_SET_CALIBRATE_CHROMATIX:
      e_ctrl->eeprom_afchroma = *((eeprom_set_chroma_af_t *)data);
      eeprom_do_chroma_calibration(e_ctrl);
      break;
  case EEPROM_CALIBRATE_FOCUS_DATA:
      e_ctrl->eeprom_afchroma = *((eeprom_set_chroma_af_t *)data);
      eeprom_do_af_calibration(e_ctrl);
      break;
  case EEPROM_GET_ISINSENSOR_CALIB: {
      int32_t *is_insensor = (int32_t *)data;
      if (e_ctrl->eeprom_lib.func_tbl &&
          e_ctrl->eeprom_params.is_supported) {
        if (e_ctrl->eeprom_lib.func_tbl->get_calibration_items != NULL)
          e_ctrl->eeprom_lib.func_tbl->get_calibration_items(e_ctrl);
        *is_insensor = e_ctrl->eeprom_data.items.is_insensor;
      }
      break;
    }
  case EEPROM_GET_RAW_DATA:
      rc = eeprom_get_raw_data(e_ctrl, data);
      break;
  default:
      SERR("Invalid event");
      rc = SENSOR_FAILURE;
      break;
  }
  return rc;
}

/** eeprom_close:
 *    @eeprom_ctrl: ERPOM control handle
 *
 * 2) Close fd
 * 3) Free EEPROM control structure
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_close(void *eeprom_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_eeprom_data_t *ctrl = (sensor_eeprom_data_t *)eeprom_ctrl;

  SLOW("Enter");

  /* Validate input parameters */
  if (!ctrl) {
    SERR("failed: ctrl %p", ctrl);
    return -EINVAL;
  }

  /* Close EEPROM library if it was opened earlier, don't
     * check return status, continue to free */
  eeprom_unload_library(&ctrl->eeprom_lib);

  /* close subdev */
  close(ctrl->fd);
  free(ctrl);
  SLOW("Exit");
  return rc;
}

/** eeprom_sub_module_init:
 *    @func_tbl: pointer to sensor function table
 *
 * Initialize function table for EEPROM device to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t eeprom_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = eeprom_open;
  func_tbl->process = eeprom_process;
  func_tbl->close = eeprom_close;
  return SENSOR_SUCCESS;
}
