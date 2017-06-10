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

#define WB_OFFSET 0x00
#define AWB_REG_SIZE 6
#define LSC_REG_SIZE 62
#define RG_TYPICAL 0x4f
#define BG_TYPICAL 0x4f
#define LSC_OFFSET 0x30

#define RG_RATIO_TYPICAL_VALUE 0x151
#define BG_RATIO_TYPICAL_VALUE 0x15f

#define ABS(x)            (((x) < 0) ? -(x) : (x))

struct otp_struct {
  uint16_t module_integrator_id;
  uint16_t lens_id;
  uint16_t production_year;
  uint16_t production_month;
  uint16_t production_day;
  uint16_t rg_ratio;
  uint16_t bg_ratio;
  uint16_t light_rg;
  uint16_t light_bg;
  uint16_t user_data[5];
  uint16_t lenc[62];
} otp_data;

struct msm_camera_i2c_reg_array g_reg_array[AWB_REG_SIZE+LSC_REG_SIZE];
struct msm_camera_i2c_reg_setting g_reg_setting;

/** sunny_p12v01m_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_p12v01m_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = TRUE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = TRUE;
  e_items->is_lsc = TRUE;
  e_items->is_dpc = FALSE;
}


/** sunny_p12v01m_calc_otp:
 *
 * Calculate R/B target value for otp white balance data format
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 */
static void sunny_p12v01m_calc_otp(uint16_t r_ratio, uint16_t b_ratio,
  uint16_t *r_target, uint16_t *b_target, uint16_t r_offset, uint16_t b_offset)
{
  if ((b_offset * ABS(RG_RATIO_TYPICAL_VALUE - r_ratio))
    < (r_offset * ABS(BG_RATIO_TYPICAL_VALUE - b_ratio))) {
    if (b_ratio < BG_RATIO_TYPICAL_VALUE)
      *b_target = BG_RATIO_TYPICAL_VALUE - b_offset;
    else
      *b_target = BG_RATIO_TYPICAL_VALUE + b_offset;

    if (r_ratio < RG_RATIO_TYPICAL_VALUE) {
      *r_target = RG_RATIO_TYPICAL_VALUE
        - ABS(BG_RATIO_TYPICAL_VALUE - *b_target)
        * ABS(RG_RATIO_TYPICAL_VALUE - r_ratio)
        / ABS(BG_RATIO_TYPICAL_VALUE - b_ratio);
    } else {
      *r_target = RG_RATIO_TYPICAL_VALUE
        + ABS(BG_RATIO_TYPICAL_VALUE - *b_target)
        * ABS(RG_RATIO_TYPICAL_VALUE - r_ratio)
        / ABS(BG_RATIO_TYPICAL_VALUE - b_ratio);
    }
  } else {
    if (r_ratio < RG_RATIO_TYPICAL_VALUE)
      *r_target = RG_RATIO_TYPICAL_VALUE - r_offset;
    else
      *r_target = RG_RATIO_TYPICAL_VALUE + r_offset;

    if (b_ratio < BG_RATIO_TYPICAL_VALUE) {
      *b_target = BG_RATIO_TYPICAL_VALUE
        - ABS(RG_RATIO_TYPICAL_VALUE - *r_target)
        * ABS(BG_RATIO_TYPICAL_VALUE - b_ratio)
        / ABS(RG_RATIO_TYPICAL_VALUE - r_ratio);
    } else {
      *b_target = BG_RATIO_TYPICAL_VALUE
        + ABS(RG_RATIO_TYPICAL_VALUE - *r_target)
        * ABS(BG_RATIO_TYPICAL_VALUE - b_ratio)
        / ABS(RG_RATIO_TYPICAL_VALUE - r_ratio);
    }
  }
}

/** sunny_p12v01m_calc_gain:
 *
 * Calculate white balance gain
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_p12v01m_calc_gain(uint16_t R_target,
  uint16_t B_target, uint16_t *R_gain, uint16_t *B_gain, uint16_t *G_gain)
{
  /* 0x400 = 1x gain */
  if (otp_data.bg_ratio < B_target) {
    if (otp_data.rg_ratio < R_target) {
      *G_gain = 0x400;
      *B_gain = 0x400 *
        B_target /
        otp_data.bg_ratio;
      *R_gain = 0x400 *
        R_target /
        otp_data.rg_ratio;
    } else {
      *R_gain = 0x400;
      *G_gain = 0x400 *
        otp_data.rg_ratio /
        R_target;
      *B_gain = 0x400 * otp_data.rg_ratio * B_target
        / (otp_data.bg_ratio * R_target);
    }
  } else {
    if (otp_data.rg_ratio < R_target) {
      *B_gain = 0x400;
      *G_gain = 0x400 *
        otp_data.bg_ratio /
        B_target;
      *R_gain = 0x400 * otp_data.bg_ratio * R_target
        / (otp_data.rg_ratio * B_target);
    } else {
      if (B_target * otp_data.rg_ratio < R_target * otp_data.bg_ratio) {
        *B_gain = 0x400;
        *G_gain = 0x400 *
          otp_data.bg_ratio /
          B_target;
        *R_gain = 0x400 * otp_data.bg_ratio * R_target
        / (otp_data.rg_ratio * B_target);
      } else {
        *R_gain = 0x400;
        *G_gain = 0x400 *
          otp_data.rg_ratio /
          R_target;
        *B_gain = 0x400 * otp_data.rg_ratio * B_target
        / (otp_data.bg_ratio * R_target);
      }
    }
  }
}

/** sunny_p12v01m_update_awb:
 *
 * Calculate and apply white balance calibration data
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_p12v01m_update_awb()
{
  uint16_t R_gain, G_gain, B_gain;
  uint16_t R_offset_outside, B_offset_outside;
  uint16_t R_offset_inside, B_offset_inside;
  uint16_t R_target, B_target;

  R_offset_inside = RG_RATIO_TYPICAL_VALUE / 100;
  R_offset_outside = RG_RATIO_TYPICAL_VALUE * 3 / 100;
  B_offset_inside = BG_RATIO_TYPICAL_VALUE / 100;
  B_offset_outside = BG_RATIO_TYPICAL_VALUE * 3 / 100;

  if(otp_data.light_rg) {
    otp_data.rg_ratio = otp_data.rg_ratio * (otp_data.light_rg +512) / 1024;
  }
  if(otp_data.light_bg){
    otp_data.bg_ratio = otp_data.bg_ratio * (otp_data.light_bg +512) / 1024;
  }
  if ((ABS(otp_data.rg_ratio - RG_RATIO_TYPICAL_VALUE)
    < R_offset_inside)
    && (ABS(otp_data.bg_ratio - BG_RATIO_TYPICAL_VALUE)
    < B_offset_inside)) {
    R_gain = 0x400;
    G_gain = 0x400;
    B_gain = 0x400;
  } else {
    if ((ABS(otp_data.rg_ratio - RG_RATIO_TYPICAL_VALUE)
      < R_offset_outside)
      && (ABS(otp_data.bg_ratio - BG_RATIO_TYPICAL_VALUE)
      < B_offset_outside)) {
      sunny_p12v01m_calc_otp(otp_data.rg_ratio, otp_data.bg_ratio
        , &R_target, &B_target,
        R_offset_inside, B_offset_inside);
    } else {
      sunny_p12v01m_calc_otp(otp_data.rg_ratio, otp_data.bg_ratio
        , &R_target, &B_target,
        R_offset_outside, B_offset_outside);
    }
    sunny_p12v01m_calc_gain(R_target,B_target,&R_gain,&B_gain,&G_gain);
  }

  if (R_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3400;
    g_reg_array[g_reg_setting.size].reg_data = R_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3401;
    g_reg_array[g_reg_setting.size].reg_data = R_gain & 0xff;
    g_reg_setting.size++;
  }
  if (G_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3402;
    g_reg_array[g_reg_setting.size].reg_data = G_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3403;
    g_reg_array[g_reg_setting.size].reg_data = G_gain & 0xff;
    g_reg_setting.size++;
  }
  if (B_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x3404;
    g_reg_array[g_reg_setting.size].reg_data = B_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x3405;
    g_reg_array[g_reg_setting.size].reg_data = B_gain & 0xff;
    g_reg_setting.size++;
  }
}

/** sunny_p12v01m_read_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate read white balance success or not.
 **/
static int sunny_p12v01m_read_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t mid,temp,reg_val;
  int i = 0, group_offset = 0;

  SLOW("Enter");
  for (i = 0; i < 3; i++) {
    mid = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET + group_offset]);
    if ((mid & 0xc0) == 0x40) {
      otp_data.module_integrator_id = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 1]);
      otp_data.lens_id = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 3]);
      otp_data.production_year = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 3]);
      otp_data.production_month = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 4]);
      otp_data.production_day = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 5]);
      temp = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 10]);
      reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 6]);
      otp_data.rg_ratio = (reg_val << 2) + ((temp >> 6) & 0x03);
      reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 7]);
      otp_data.bg_ratio = (reg_val << 2) + ((temp >> 4) & 0x03);
      reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 8]);
      otp_data.light_rg = (reg_val << 2) + ((temp >> 2) & 0x03);
      reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 9]);
      otp_data.light_bg = (reg_val << 2) + (temp & 0x03);
      otp_data.user_data[0] = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 11]);
      otp_data.user_data[1] = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 12]);
      otp_data.user_data[2] = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 13]);
      otp_data.user_data[3] = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 14]);
      otp_data.user_data[4] = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET +
        group_offset + 15]);
      break;
    }
    group_offset += 16;
  }
  if (group_offset > 0x20) {
    SERR("No WB calibration data valid\n");
    return -1;
  }

  return 0;
}

/** sunny_p12v01m_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_p12v01m_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  SLOW("Enter");
  int rc = 0;

  rc = sunny_p12v01m_read_wbdata(e_ctrl);
  if(rc < 0)
    SERR("read wbdata failed");
  sunny_p12v01m_update_awb();

  SLOW("Exit");
}


/** sunny_p12v01m_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_p12v01m_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  int i = 0, j = 0, group_offset = 0;
  uint8_t mid;

  for (i = 0; i < 3; i++) {
    mid = (uint8_t)(e_ctrl->eeprom_params.buffer[LSC_OFFSET + group_offset]);
    if ((mid & 0xc0) == 0x40) {
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

/** sunny_p12v01m_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_p12v01m_format_calibration_data(void *e_ctrl) {
  SLOW("Enter");
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = MSM_CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;
  sunny_p12v01m_format_wbdata(ectrl);
  sunny_p12v01m_format_lensshading(ectrl);

  SLOW("Exit");
}

/** sunny_p12v01m_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * Return: void.
 **/
void sunny_p12v01m_get_raw_data(void *e_ctrl, void *data) {
  if (e_ctrl && data)
    memcpy(data, &g_reg_setting, sizeof(g_reg_setting));
  else
    SERR("failed Null pointer");
  return;
}

static eeprom_lib_func_t sunny_p12v01m_lib_func_ptr = {
  .get_calibration_items = sunny_p12v01m_get_calibration_items,
  .format_calibration_data = sunny_p12v01m_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .do_dpc_calibration = NULL,
  .get_dpc_calibration_info = NULL,
  .get_raw_data = sunny_p12v01m_get_raw_data,
};

/** sunny_p12v01m_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_p12v01m_eeprom_open_lib(void) {
  return &sunny_p12v01m_lib_func_ptr;
}
