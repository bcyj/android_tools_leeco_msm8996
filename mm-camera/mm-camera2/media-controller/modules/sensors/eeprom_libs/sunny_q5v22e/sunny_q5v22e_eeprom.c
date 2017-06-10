/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <dlfcn.h>
#include <math.h>
#include <stdlib.h>
#include "chromatix.h"
#include "eeprom.h"
#include "sensor_common.h"

#define AWB_REG_SIZE 6

#define RG_TYPICAL 0x177
#define BG_TYPICAL 0x154

struct otp_struct {
  uint16_t module_integrator_id;
  uint16_t lens_id;
  uint16_t rg_ratio;
  uint16_t bg_ratio;
  uint16_t user_data[2];
  uint16_t light_rg;
  uint16_t light_bg;
} otp_data;

struct msm_camera_i2c_reg_array g_reg_array[AWB_REG_SIZE];
struct msm_camera_i2c_reg_setting g_reg_setting;

/** sunny_q5v22e_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void
 **/
void sunny_q5v22e_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = FALSE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = TRUE;
  e_items->is_lsc = FALSE;
  e_items->is_dpc = FALSE;
}

/** sunny_q5v22e_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void
 **/
static void sunny_q5v22e_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  int grp_off[3] = { 0x05, 0x0E, 0x17 };
  int i = 0;
  int index = 0;
  uint8_t mid, flag, wb_mix;
  uint16_t rg, bg;
  int r_gain, g_gain, b_gain, g_gain_b, g_gain_r;

  for (i = 0; i < 3; i++) {
    mid = e_ctrl->eeprom_params.buffer[grp_off[i]];
    rg = e_ctrl->eeprom_params.buffer[grp_off[i] + 2];
    bg = e_ctrl->eeprom_params.buffer[grp_off[i] + 3];
    wb_mix = e_ctrl->eeprom_params.buffer[grp_off[i] + 6];

    rg = (rg << 2) | (wb_mix >> 6);
    bg = (bg << 2) | ((wb_mix & 0x30) >> 4);
    flag = mid & 0x80;

    if (!flag && rg && bg) {
      /* current group has valid calibration data */
      otp_data.module_integrator_id = mid & 0x7f;
      otp_data.lens_id = e_ctrl->eeprom_params.buffer[grp_off[i] + 1];
      otp_data.rg_ratio = rg;
      otp_data.bg_ratio = bg;
      otp_data.user_data[0] = e_ctrl->eeprom_params.buffer[grp_off[i] + 4];
      otp_data.user_data[1] = e_ctrl->eeprom_params.buffer[grp_off[i] + 5];
      otp_data.light_rg = e_ctrl->eeprom_params.buffer[grp_off[i] + 7];
      otp_data.light_rg = (otp_data.light_rg << 2) | ((wb_mix & 0x0c) >> 2);
      otp_data.light_bg = e_ctrl->eeprom_params.buffer[grp_off[i] + 8];
      otp_data.light_bg = (otp_data.light_bg << 2) | (wb_mix & 0x03);
      break;
    }
  }

  if (3 == i) {
    SERR("No WB calibration data valid\n");
    return;
  }

  if (otp_data.light_rg)
    otp_data.rg_ratio = otp_data.rg_ratio * (otp_data.light_rg + 512) / 1024;

  if (otp_data.light_bg)
    otp_data.bg_ratio = otp_data.bg_ratio * (otp_data.light_bg + 512) / 1024;

  for ( index = 0; index < AGW_AWB_MAX_LIGHT; index++ ) {

    e_ctrl->eeprom_data.wbc.r_over_g[index] =
      (float) (otp_data.rg_ratio);

    e_ctrl->eeprom_data.wbc.b_over_g[index] =
      (float) (otp_data.bg_ratio);
  }
  e_ctrl->eeprom_data.wbc.gr_over_gb = 1.0f;
#if 0
  if (otp_data.bg_ratio < BG_TYPICAL) {
    if (otp_data.rg_ratio < RG_TYPICAL) {
      g_gain = 0x400;
      b_gain = 0x400 * BG_TYPICAL / otp_data.bg_ratio;
      r_gain = 0x400 * RG_TYPICAL / otp_data.rg_ratio;
    } else {
      r_gain = 0x400;
      g_gain = 0x400 * otp_data.rg_ratio / RG_TYPICAL;
      b_gain = g_gain * BG_TYPICAL / otp_data.bg_ratio;
    }
  } else {
    if (otp_data.rg_ratio < RG_TYPICAL) {
      b_gain = 0x400;
      g_gain = 0x400 * otp_data.bg_ratio / BG_TYPICAL;
      r_gain = g_gain * RG_TYPICAL / otp_data.rg_ratio;
    } else {
      g_gain_b = 0x400 * otp_data.bg_ratio / BG_TYPICAL;
      g_gain_r = 0x400 * otp_data.rg_ratio / RG_TYPICAL;

      if (g_gain_b > g_gain_r) {
        b_gain = 0x400;
        g_gain = g_gain_b;
        r_gain = g_gain * RG_TYPICAL / otp_data.rg_ratio;
      } else {
        r_gain = 0x400;
        g_gain = g_gain_r;
        b_gain = g_gain * BG_TYPICAL / otp_data.bg_ratio;
      }
    }
  }
  SERR("R_gain=0x%x,G_gain=0x%x,B_gain=0x%x",r_gain,g_gain,b_gain) ;

  if (r_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5186;
    g_reg_array[g_reg_setting.size].reg_data = r_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5187;
    g_reg_array[g_reg_setting.size].reg_data = r_gain & 0x00ff;
    g_reg_setting.size++;
  }
  if (g_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5188;
    g_reg_array[g_reg_setting.size].reg_data = g_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5189;
    g_reg_array[g_reg_setting.size].reg_data = g_gain & 0x00ff;
    g_reg_setting.size++;
  }
  if (b_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x518a;
    g_reg_array[g_reg_setting.size].reg_data = b_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x518b;
    g_reg_array[g_reg_setting.size].reg_data = b_gain & 0x00ff;
    g_reg_setting.size++;
  }
#endif
}

/** sunny_q5v22e_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void
 **/
void sunny_q5v22e_format_calibration_data(void *e_ctrl) {
  SLOW("Enter");

  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = MSM_CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;
  sunny_q5v22e_format_wbdata(ectrl);

  SLOW("Exit");
}

/** sunny_q5v22e_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * Return: void
 **/
void sunny_q5v22e_get_raw_data(void *e_ctrl, void *data) {
  if (e_ctrl && data)
    memcpy(data, &g_reg_setting, sizeof(g_reg_setting));
  else
    SERR("failed Null pointer");
  return;
}

static eeprom_lib_func_t sunny_q5v22e_lib_func_ptr = {
  .get_calibration_items = sunny_q5v22e_get_calibration_items,
  .format_calibration_data = sunny_q5v22e_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = NULL,
  .do_dpc_calibration = NULL,
  .get_dpc_calibration_info = NULL,
  .get_raw_data = sunny_q5v22e_get_raw_data,
};

/** sunny_q5v22e_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: function pointer of eeprom_lib_func_t.
 **/
void* sunny_q5v22e_eeprom_open_lib(void) {
  return &sunny_q5v22e_lib_func_ptr;
}
