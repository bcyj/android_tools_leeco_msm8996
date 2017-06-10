/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <dlfcn.h>
#include <math.h>
#include <stdlib.h>
#include "chromatix.h"
#include "eeprom.h"
#include "sensor_common.h"

#define WB_OFFSET 0x05
#define AWB_REG_SIZE 6
#define LSC_REG_SIZE 62
#define RG_TYPICAL 0x4f
#define BG_TYPICAL 0x4f
#define LSC_OFFSET 0x20

struct msm_camera_i2c_reg_array g_reg_array[AWB_REG_SIZE+LSC_REG_SIZE];
struct msm_camera_i2c_reg_setting g_reg_setting;

/** truly_cm7700_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * No return value.
 **/
void truly_cm7700_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = TRUE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = TRUE;
  e_items->is_lsc = TRUE;
  e_items->is_dpc = FALSE;
}

/** truly_cm7700_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * No return value.
 **/
static void truly_cm7700_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t light_rg, light_bg, rg, bg, mid;
  int r_gain, g_gain, b_gain, g_gain_b, g_gain_r;
  int i = 0, group_offset = 0;

  SLOW("Enter");
  for (i = 0; i < 3; i++) {
    mid = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET + group_offset]);
    if (mid & 0x80) {
      light_rg = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
      group_offset + 7]);
      light_bg = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
      group_offset + 8]);
      rg = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
      group_offset + 2]);
      bg = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
      group_offset + 3]);
      break;
    }
    group_offset += 9;
  }
  if (group_offset > 0x12) {
    SERR("No WB calibration data valid\n");
    return;
  }
  if (light_rg)
    rg = rg * (light_rg + 128) / 256;
  if (light_bg)
    bg = bg * (light_bg + 128) / 256;
  if (bg < BG_TYPICAL) {
    if (rg < RG_TYPICAL) {
      g_gain = 0x400;
      b_gain = 0x400 * BG_TYPICAL / bg;
      r_gain = 0x400 * RG_TYPICAL / rg;
    } else {
      r_gain = 0x400;
      g_gain = 0x400 * rg / RG_TYPICAL;
      b_gain = g_gain * BG_TYPICAL / bg;
    }
  } else {
    if (rg < RG_TYPICAL) {
      b_gain = 0x400;
      g_gain = 0x400 * BG_TYPICAL / bg;
      r_gain = g_gain * RG_TYPICAL / rg;
    } else {
      g_gain_b = 0x400 * bg / BG_TYPICAL;
      g_gain_r = 0x400 * rg / RG_TYPICAL;
      if (g_gain_b > g_gain_r) {
        b_gain = 0x400;
        g_gain = g_gain_b;
        r_gain = g_gain * RG_TYPICAL / rg;
      } else {
        r_gain = 0x400;
        g_gain = g_gain_r;
        b_gain = g_gain * BG_TYPICAL / bg;
      }
    }
  }
  if (r_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3400;
    g_reg_array[g_reg_setting.size].reg_data = r_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3401;
    g_reg_array[g_reg_setting.size].reg_data = r_gain & 0xff;
    g_reg_setting.size++;
  }
  if (g_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3402;
    g_reg_array[g_reg_setting.size].reg_data = g_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3403;
    g_reg_array[g_reg_setting.size].reg_data = g_gain & 0xff;
    g_reg_setting.size++;
  }
  if (b_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3404;
    g_reg_array[g_reg_setting.size].reg_data = b_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3405;
    g_reg_array[g_reg_setting.size].reg_data = b_gain & 0xff;
    g_reg_setting.size++;
  }
}

/** truly_cm7700_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * No return value.
 **/
void truly_cm7700_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  int i = 0, j = 0, group_offset = 0;
  uint8_t mid;

  for (i = 0; i < 3; i++) {
    mid = (uint8_t)(e_ctrl->eeprom_params.buffer[LSC_OFFSET + group_offset]);
    if (mid == 0x40) {
      for (j = 0; j < LSC_REG_SIZE; j++) {
        g_reg_array[g_reg_setting.size].reg_addr = 0x5800 + j;
        g_reg_array[g_reg_setting.size].reg_data =
          (uint8_t)(e_ctrl->eeprom_params.buffer[LSC_OFFSET +
          group_offset + j + 1]);
        g_reg_setting.size++;
      }
      break;
    }
    group_offset += 64;
  }
  if (group_offset > 0xc8) {
    SERR("lens shading calibration data invalid\n");
    return;
  }
  SLOW("Exit");
}

/** truly_cm7700_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * No return value.
 **/
void truly_cm7700_format_calibration_data(void *e_ctrl) {
  SLOW("Enter");
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = MSM_CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;
  truly_cm7700_format_wbdata(ectrl);
  truly_cm7700_format_lensshading(ectrl);

  SLOW("Exit");
}

/** truly_cm7700_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * No return value.
 **/
void truly_cm7700_get_raw_data(void *e_ctrl, void *data) {
  if (e_ctrl && data)
    memcpy(data, &g_reg_setting, sizeof(g_reg_setting));
  else
    SERR("failed Null pointer");
  return;
}

static eeprom_lib_func_t truly_cm7700_lib_func_ptr = {
  .get_calibration_items = truly_cm7700_get_calibration_items,
  .format_calibration_data = truly_cm7700_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .do_dpc_calibration = NULL,
  .get_dpc_calibration_info = NULL,
  .get_raw_data = truly_cm7700_get_raw_data,
};

/** truly_cm7700_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * return eeprom_lib_func_t point to the function pointer.
 **/
void* truly_cm7700_eeprom_open_lib(void) {
  return &truly_cm7700_lib_func_ptr;
}
