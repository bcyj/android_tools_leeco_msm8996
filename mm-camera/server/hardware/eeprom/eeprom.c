/*==========================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "camera_dbg.h"
#include "tgtcommon.h"
#include <media/msm_camera.h>
#include "eeprom_interface.h"
#include "sensor_interface.h"
#include "eeprom.h"

struct msm_camera_eeprom_info_t eeprom_calib_info;
eeprom_calib_2d_data eeprom_calib_data;

struct pixel_t *eprom_dpc_data[SENSOR_MODE_INVALID] = {
  &eeprom_calib_data.dpc.snapshot_coord[0],/*SENSOR_MODE_SNAPSHOT*/
  &eeprom_calib_data.dpc.snapshot_coord[0],/*SENSOR_MODE_RAW_SNAPSHOT*/
  &eeprom_calib_data.dpc.preview_coord[0],/*SENSOR_MODE_PREVIEW*/
  &eeprom_calib_data.dpc.preview_coord[0],/*SENSOR_MODE_VIDEO*/
  &eeprom_calib_data.dpc.video_coord[0],/*SENSOR_MODE_VIDEO HD*/
  NULL,/*SENSOR_MODE_HFR_60FPS*/
  NULL,/*SENSOR_MODE_HFR_90FPS*/
  NULL,/*SENSOR_MODE_HFR_120FPS*/
  NULL,/*SENSOR_MODE_HFR_150FPS*/
  &eeprom_calib_data.dpc.snapshot_coord[0],/*SENSOR_MODE_ZSL*/
  };
/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
void eeprom_autofocus_calibration(void *ctrl)
{
  eeprom_ctrl_t *ectrl = (eeprom_ctrl_t *) ctrl;
  actuator_tuned_params_t *aftune = &ectrl->aftune_ptr->actuator_tuned_params;

    aftune->region_params[0].code_per_step = eeprom_calib_data.afc.inf_dac /
      (aftune->region_params[0].step_bound[0] -
      aftune->region_params[0].step_bound[1]);

    aftune->region_params[1].code_per_step =
      (eeprom_calib_data.afc.macro_dac - eeprom_calib_data.afc.inf_dac) /
      (aftune->region_params[1].step_bound[0] -
      aftune->region_params[1].step_bound[1]);
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
void eeprom_lensshading_calibration(void *ctrl)
{
  int i, j, index;
  eeprom_ctrl_t *ectrl = (eeprom_ctrl_t *) ctrl;
  chromatix_parms_type *chromatix = ectrl->chromatixPtr;

  for (j = 0; j < ROLLOFF_MAX_LIGHT; j++) {
    for (i = 0; i < MESH_ROLLOFF_SIZE; i++) {
      chromatix->chromatix_mesh_rolloff_table[j].r_gain[i] =
      chromatix->chromatix_mesh_rolloff_table[j].r_gain[i] *
      (chromatix->chromatix_rolloff_goldenmod.r_gain[i] /
        ((float)eeprom_calib_data.lsc.r_gain[i] /
        eeprom_calib_info.lsc.qvalue));

      chromatix->chromatix_mesh_rolloff_table[j].b_gain[i] =
      chromatix->chromatix_mesh_rolloff_table[j].b_gain[i] *
      (chromatix->chromatix_rolloff_goldenmod.b_gain[i] /
        ((float)eeprom_calib_data.lsc.b_gain[i] /
        eeprom_calib_info.lsc.qvalue));

      chromatix->chromatix_mesh_rolloff_table[j].gr_gain[i] =
      chromatix->chromatix_mesh_rolloff_table[j].gr_gain[i] *
      (chromatix->chromatix_rolloff_goldenmod.gr_gain[i] /
        ((float)eeprom_calib_data.lsc.gr_gain[i] /
        eeprom_calib_info.lsc.qvalue));

      chromatix->chromatix_mesh_rolloff_table[j].gb_gain[i] =
      chromatix->chromatix_mesh_rolloff_table[j].gb_gain[i] *
      (chromatix->chromatix_rolloff_goldenmod.gb_gain[i] /
        ((float)eeprom_calib_data.lsc.gb_gain[i] /
        eeprom_calib_info.lsc.qvalue));
    }
  }

  CDBG("%s ChromatixLSC R Gain   Gr Gain   Gb Gain   B Gain\n", __func__);
  for (i = 0; i < MESH_ROLLOFF_SIZE; i++)
    CDBG("calibdata[%d],    %f    %f    %f    %f\n", i,
    chromatix->chromatix_mesh_rolloff_table[0].r_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].gr_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].gb_gain[i],
    chromatix->chromatix_mesh_rolloff_table[0].b_gain[i]);
}

static void swap(struct pixel_t *m, struct pixel_t *n)
{
  struct pixel_t temp;
  temp.x = m->x;
  temp.y = m->y;
  *m = *n;
  n->x = temp.x;
  n->y = temp.y;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
void eeprom_defectpixcel_calibration(void *ctrl)
{
  struct msm_calib_dpc *dpc = &eeprom_calib_data.dpc;
  int count = dpc->validcount;
  int i, j;
  eeprom_ctrl_t *ectrl = (eeprom_ctrl_t *) ctrl;

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
  CDBG("%s Accending order\n", __func__);
  for (i = 0; i < count; i++)
    CDBG("dpc->snapshot_coord[%d] X= %d Y = %d\n",
      i, dpc->snapshot_coord[i].x, dpc->snapshot_coord[i].y);
  for (i = 0; i < count; i++)
    CDBG("dpc->preview_coord[%d] X= %d Y = %d\n",
      i, dpc->preview_coord[i].x, dpc->preview_coord[i].y);
  for (i = 0; i < count; i++)
    CDBG("dpc->video_coord[%d] X= %d Y = %d\n",
      i, dpc->video_coord[i].x, dpc->video_coord[i].y);
}

void eeprom_get_dpc_info(void *ctrl, int type, void *info)
{
  eeprom_ctrl_t *ectrl = (eeprom_ctrl_t *) ctrl;
  pixels_array_t *dpc_info = (pixels_array_t *)info;
  if (eeprom_calib_info.dpc.is_supported) {
    dpc_info->count = eeprom_calib_data.dpc.validcount;
    dpc_info->pix = (pixel_t *)eprom_dpc_data[type];
  }
}

static float wbgain_calibration(float gain, float calib_factor)
{
  /* gain = 1 / [(1/gain) * calibration_r_over_g_factor] */
  return gain / calib_factor;
}


/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
void eeprom_whitebalance_calibration(void *ctrl)
{
  eeprom_ctrl_t *ectrl = (eeprom_ctrl_t *) ctrl;
  CDBG("%s getting  in\n", __func__);

  wbcalib_data *calib_info = &(ectrl->calibrated_data.wbc);
  float r_gain, b_gain, g_gain, min_gain, gr_over_gb;
  chromatix_parms_type *chromatix = ectrl->chromatixPtr;
  uint32_t indx, light_indx = AGW_AWB_MAX_LIGHT-1;
  calib_info->r_over_g = ((float)eeprom_calib_data.wbc.r_over_g /
    eeprom_calib_info.wb.qvalue);
  calib_info->b_over_g = ((float)eeprom_calib_data.wbc.b_over_g /
    eeprom_calib_info.wb.qvalue);
  calib_info->gr_over_gb = ((float)eeprom_calib_data.wbc.gr_over_gb /
    eeprom_calib_info.wb.qvalue);
  float r_over_g_calib_factor = calib_info->r_over_g;
  CDBG("calib_info->r_over_g = %f\n", calib_info->r_over_g);
  CDBG("calib_info->b_over_g = %f\n", calib_info->b_over_g);
  CDBG("calib_info->gr_over_gb = %f\n", calib_info->gr_over_gb);

  r_over_g_calib_factor /= chromatix->AWB_golden_module_R_Gr_ratio;
  float b_over_g_calib_factor = (float)calib_info->b_over_g;
    b_over_g_calib_factor /= chromatix->AWB_golden_module_B_Gr_ratio;

  /* Calibrate the AWB in chromatix based on color measurement read */
  for (indx = 0; indx < light_indx; indx++) {
    chromatix->awb_reference_hw_rolloff[indx].red_gain *=
      r_over_g_calib_factor;
    chromatix->awb_reference_hw_rolloff[indx].blue_gain *=
      b_over_g_calib_factor;
  }

  /* Calibrate the MWB in chromatix based on color measurement read */
  /* MWB TL84 */
  r_gain = wbgain_calibration(
    chromatix->chromatix_tl84_white_balance.r_gain,
    r_over_g_calib_factor);
  b_gain = wbgain_calibration(
    chromatix->chromatix_tl84_white_balance.b_gain,
    b_over_g_calib_factor);
  g_gain = chromatix->chromatix_tl84_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_tl84_white_balance.r_gain = r_gain / min_gain;
  chromatix->chromatix_tl84_white_balance.g_gain = g_gain / min_gain;
  chromatix->chromatix_tl84_white_balance.b_gain = b_gain / min_gain;

  /* MWB D50 */
  r_gain = wbgain_calibration(
    chromatix->chromatix_d50_white_balance.r_gain,
    r_over_g_calib_factor);
  b_gain = wbgain_calibration(
    chromatix->chromatix_d50_white_balance.b_gain,
    b_over_g_calib_factor);
  g_gain = chromatix->chromatix_d50_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_d50_white_balance.r_gain = r_gain / min_gain;
  chromatix->chromatix_d50_white_balance.g_gain = g_gain / min_gain;
  chromatix->chromatix_d50_white_balance.b_gain = b_gain / min_gain;

  /* MWB Incandescent */
  r_gain = wbgain_calibration(
    chromatix->chromatix_incandescent_white_balance.r_gain,
    r_over_g_calib_factor);
  b_gain = wbgain_calibration(
    chromatix->chromatix_incandescent_white_balance.b_gain,
    b_over_g_calib_factor);
  g_gain = chromatix->chromatix_incandescent_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_incandescent_white_balance.r_gain =
    r_gain / min_gain;
  chromatix->chromatix_incandescent_white_balance.g_gain =
    g_gain / min_gain;
  chromatix->chromatix_incandescent_white_balance.b_gain =
    b_gain / min_gain;

  /* MWB D65 */
  r_gain = wbgain_calibration(
    chromatix->chromatix_d65_white_balance.r_gain, r_over_g_calib_factor);
  b_gain = wbgain_calibration(
    chromatix->chromatix_d65_white_balance.b_gain, b_over_g_calib_factor);
  g_gain = chromatix->chromatix_d65_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->chromatix_d65_white_balance.r_gain = r_gain / min_gain;
  chromatix->chromatix_d65_white_balance.g_gain = g_gain / min_gain;
  chromatix->chromatix_d65_white_balance.b_gain = b_gain / min_gain;

  /* MWB Strobe */
  r_gain = wbgain_calibration(
    chromatix->strobe_flash_white_balance.r_gain, r_over_g_calib_factor);
  b_gain = wbgain_calibration(
    chromatix->strobe_flash_white_balance.b_gain, b_over_g_calib_factor);
  g_gain = chromatix->strobe_flash_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->strobe_flash_white_balance.r_gain = r_gain / min_gain;
  chromatix->strobe_flash_white_balance.g_gain = g_gain / min_gain;
  chromatix->strobe_flash_white_balance.b_gain = b_gain / min_gain;

  /* MWB LED flash */
  r_gain = wbgain_calibration(
    chromatix->led_flash_white_balance.r_gain, r_over_g_calib_factor);
  b_gain = wbgain_calibration(
    chromatix->led_flash_white_balance.b_gain, b_over_g_calib_factor);
  g_gain = chromatix->led_flash_white_balance.g_gain;
  min_gain = MIN(r_gain, g_gain);
  min_gain = MIN(b_gain, min_gain);
  chromatix->led_flash_white_balance.r_gain = r_gain / min_gain;
  chromatix->led_flash_white_balance.g_gain = g_gain / min_gain;
  chromatix->led_flash_white_balance.b_gain = b_gain / min_gain;

  /* Gr/Gb channel balance */
  gr_over_gb = (float)calib_info->gr_over_gb;
  /* To compensate invert the measured gains */
  b_gain = gr_over_gb;
  r_gain = 1;
  CDBG("%s: gb_gain: %f, gr_gain: %f\n", __func__, b_gain, r_gain);
  min_gain = MIN(r_gain, b_gain);
  CDBG("%s: min_gain: %f\n", __func__, min_gain);
  chromatix->chromatix_channel_balance_gains.green_even = r_gain/min_gain;
  chromatix->chromatix_channel_balance_gains.green_odd = b_gain/min_gain;
  return;
}

/*===========================================================================
 * FUNCTION    - eeprom_get_data-
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t eeprom_get_data(void *ctrl, uint16_t index)
{
  struct msm_eeprom_cfg_data cfg;
  eeprom_ctrl_t *ectrl = (eeprom_ctrl_t *) ctrl;
  if (ectrl->fd <= 0)
    return FALSE;
  CDBG("%s getting eeprom data in index =%d\n", __func__, index);
  cfg.cfgtype = CFG_GET_EEPROM_DATA;
  cfg.cfg.get_data.index = index;
  if (ectrl->cali_data == NULL) {
    CDBG_ERROR("%s: Invalid paramter\n", __func__);
    return FALSE;
  }
  cfg.cfg.get_data.eeprom_data = ectrl->cali_data;
  if (ioctl(ectrl->fd, MSM_CAM_IOCTL_EEPROM_IO_CFG, &cfg) < 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return FALSE;
  }
  CDBG("%s exit\n", __func__);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - eeprom_get_info-
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t eeprom_get_info(void *ctrl)
{
  struct msm_eeprom_cfg_data cfg;
  eeprom_ctrl_t *ectrl = (eeprom_ctrl_t *) ctrl;
  if (ectrl->fd <= 0)
    return FALSE;

  cfg.cfgtype = CFG_GET_EEPROM_INFO;
  if (ioctl(ectrl->fd, MSM_CAM_IOCTL_EEPROM_IO_CFG, &cfg) < 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return FALSE;
  }
  memcpy(&eeprom_calib_info, &cfg.cfg.get_info,
    sizeof(struct msm_camera_eeprom_info_t));
  ectrl->is_eeprom_supported = cfg.is_eeprom_supported;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
void eeprom_do_calibration(void *ctrl)
{
  if (eeprom_calib_info.af.is_supported)
    eeprom_autofocus_calibration(ctrl);

  if (eeprom_calib_info.wb.is_supported)
    eeprom_whitebalance_calibration(ctrl);

  if (eeprom_calib_info.lsc.is_supported)
    eeprom_lensshading_calibration(ctrl);

  if (eeprom_calib_info.dpc.is_supported)
    eeprom_defectpixcel_calibration(ctrl);

}

/*===========================================================================
 * FUNCTION    - eeprom_get_raw_data -
 *
 * DESCRIPTION: Pass raw data to caller
 *==========================================================================*/
static int32_t eeprom_get_raw_data(void *ctrl, void *data)
{
  eeprom_ctrl_t *ectrl = (eeprom_ctrl_t *) ctrl;
  CDBG("%s called\n", __func__);
  if (eeprom_calib_info.raw.is_supported)
    data = (void *)&eeprom_calib_data.raw;
  else
    data = NULL;
  CDBG("%s exit\n", __func__);
  return TRUE;
}

/*==========================================================
* FUNCTION    - eeprom_init -
*
* DESCRIPTION: Intializes the eeprom object with function pointers.
*
*==========================================================*/
void eeprom_init(eeprom_ctrl_t *ectrl)
{
  if (ectrl == NULL) {
    CDBG_ERROR("%s: Invalid Argument - ectrl\n", __func__);
    return;
  }

  ectrl->is_eeprom_supported = 0;
  ectrl->fn_table.dpc_calibration_info = NULL;
  ectrl->fn_table.eeprom_get_raw_data = eeprom_get_raw_data;

  eeprom_get_info(ectrl);

  if (eeprom_calib_info.af.is_supported) {
    ectrl->cali_data = &(eeprom_calib_data.afc);
    eeprom_get_data(ectrl, eeprom_calib_info.af.index);
  }
  if (eeprom_calib_info.wb.is_supported) {
    ectrl->cali_data = &(eeprom_calib_data.wbc);
    eeprom_get_data(ectrl, eeprom_calib_info.wb.index);
  }
  if (eeprom_calib_info.lsc.is_supported) {
    ectrl->cali_data = &(eeprom_calib_data.lsc);
    eeprom_get_data(ectrl, eeprom_calib_info.lsc.index);
  }
  if (eeprom_calib_info.dpc.is_supported) {
    ectrl->cali_data = &(eeprom_calib_data.dpc);
    eeprom_get_data(ectrl, eeprom_calib_info.dpc.index);
    ectrl->fn_table.dpc_calibration_info = eeprom_get_dpc_info;
  }
  if (eeprom_calib_info.raw.is_supported) {
    eeprom_calib_data.raw.data = malloc(eeprom_calib_info.raw.size *
      sizeof(uint8_t));
    if (eeprom_calib_data.raw.data) {
       uint32_t index = 0;
       eeprom_calib_data.raw.size = eeprom_calib_info.raw.size;
       ectrl->cali_data = eeprom_calib_data.raw.data;
       eeprom_get_data(ectrl, eeprom_calib_info.raw.index);
       CDBG("%s raw data size %d\n", __func__, eeprom_calib_data.raw.size);
       for (index = 0; index < eeprom_calib_data.raw.size; index++) {
         CDBG("%s raw data[%d] %x\n", __func__, index,
           eeprom_calib_data.raw.data[index]);
       }
    } else {
       CDBG_ERROR("%s:%d malloc failed\n", __func__, __LINE__);
       eeprom_calib_info.raw.is_supported = 0;
    }
  }

  /* Initialize the function pointer table with appropriate functions */
  ectrl->fn_table.do_calibration = eeprom_do_calibration;
  return;
}
/*==========================================================
* FUNCTION    - eeprom_destroy -
*
* DESCRIPTION: Dentializes the eeprom object, deallocate memory.
*
*==========================================================*/
void eeprom_destroy(eeprom_ctrl_t *ectrl)
{
  if (eeprom_calib_info.raw.is_supported)
    free(eeprom_calib_data.raw.data);
  return;
}
