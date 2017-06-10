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

#define RG_TYPICAL_VALUE 0x130
#define BG_TYPICAL_VALUE 0x135

#define SUNNY_Q13V04B_EEPROM_AWB_ENABLE 1
#define SUNNY_Q13V04B_EEPROM_LSC_ENABLE 1

#define ABS(x)            (((x) < 0) ? -(x) : (x))

#define OTP_DRV_START_ADDR          0x7220
#define OTP_DRV_INFO_GROUP_COUNT    3
#define OTP_DRV_INFO_SIZE           5
#define OTP_DRV_AWB_GROUP_COUNT     3
#define OTP_DRV_AWB_SIZE            5
#define OTP_DRV_VCM_GROUP_COUNT     3
#define OTP_DRV_VCM_SIZE            3
#define OTP_DRV_LSC_GROUP_COUNT     3
#define OTP_DRV_LSC_SIZE            62
#define OTP_DRV_LSC_REG_ADDR        0x5200

#define DRV_INFO_FLAG_OFFSET 0
#define AWB_FLAG_OFFSET (DRV_INFO_FLAG_OFFSET \
                        + OTP_DRV_INFO_GROUP_COUNT * OTP_DRV_INFO_SIZE \
                        + 1)
#define VCM_FLAG_OFFSET (AWB_FLAG_OFFSET \
                        + OTP_DRV_AWB_GROUP_COUNT * OTP_DRV_AWB_SIZE \
                        + 1)
#define LSC_FLAG_OFFSET (VCM_FLAG_OFFSET \
                        + OTP_DRV_VCM_GROUP_COUNT * OTP_DRV_VCM_SIZE \
                        + 1)

#define LSC_ENABLE_REG_DEFAULT_VALUE 0x0e
#define LSC_ENABLE_REG_ADDR 0x5000

#define AWB_REG_SIZE 6
#define LSC_ENABLE_REG_SIZE 1
#define LSC_REG_SIZE 62
#define OTP_REG_ARRAY_SIZE (AWB_REG_SIZE + LSC_ENABLE_REG_SIZE + LSC_REG_SIZE)

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
  uint16_t lenc[OTP_DRV_LSC_SIZE];
  uint16_t VCM_start;
  uint16_t VCM_end;
  uint16_t VCM_dir;
} otp_data;

struct msm_camera_i2c_reg_array g_reg_array[OTP_REG_ARRAY_SIZE];
struct msm_camera_i2c_reg_setting g_reg_setting;

/** sunny_q13v04b_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_q13v04b_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = TRUE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = TRUE;
  e_items->is_lsc = TRUE;
  e_items->is_dpc = FALSE;
}

/** sunny_q13v04b_calc_otp:
 *
 * Calculate R/B target value for otp white balance data format
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 */
static void sunny_q13v04b_calc_otp(uint16_t r_ratio, uint16_t b_ratio,
  uint16_t *r_target, uint16_t *b_target, uint16_t r_offset, uint16_t b_offset)
{
  SLOW("Before AWB OTP target calibration");
  SLOW("r_ratio:0x%0x", r_ratio);
  SLOW("b_ratio:0x%0x", b_ratio);
  SLOW("*r_target:0x%0x", *r_target);
  SLOW("*b_target:0x%0x", *b_target);
  SLOW("r_offset:0x%0x", r_offset);
  SLOW("b_offset:0x%0x", b_offset);
  if ((b_offset * ABS(RG_TYPICAL_VALUE - r_ratio))
    < (r_offset * ABS(BG_TYPICAL_VALUE - b_ratio))) {
    if (b_ratio < BG_TYPICAL_VALUE)
      *b_target = BG_TYPICAL_VALUE - b_offset;
    else
      *b_target = BG_TYPICAL_VALUE + b_offset;

    if (r_ratio < RG_TYPICAL_VALUE) {
      *r_target = RG_TYPICAL_VALUE
        - ABS(BG_TYPICAL_VALUE - *b_target)
        * ABS(RG_TYPICAL_VALUE - r_ratio)
        / ABS(BG_TYPICAL_VALUE - b_ratio);
    } else {
      *r_target = RG_TYPICAL_VALUE
        + ABS(BG_TYPICAL_VALUE - *b_target)
        * ABS(RG_TYPICAL_VALUE - r_ratio)
        / ABS(BG_TYPICAL_VALUE - b_ratio);
    }
  } else {
    if (r_ratio < RG_TYPICAL_VALUE)
      *r_target = RG_TYPICAL_VALUE - r_offset;
    else
      *r_target = RG_TYPICAL_VALUE + r_offset;

    if (b_ratio < BG_TYPICAL_VALUE) {
      *b_target = BG_TYPICAL_VALUE
        - ABS(RG_TYPICAL_VALUE - *r_target)
        * ABS(BG_TYPICAL_VALUE - b_ratio)
        / ABS(RG_TYPICAL_VALUE - r_ratio);
    } else {
      *b_target = BG_TYPICAL_VALUE
        + ABS(RG_TYPICAL_VALUE - *r_target)
        * ABS(BG_TYPICAL_VALUE - b_ratio)
        / ABS(RG_TYPICAL_VALUE - r_ratio);
    }
  }
  SLOW("After AWB OTP target calibration");
  SLOW("r_ratio:0x%0x", r_ratio);
  SLOW("b_ratio:0x%0x", b_ratio);
  SLOW("*r_target:0x%0x", *r_target);
  SLOW("*b_target:0x%0x", *b_target);
  SLOW("r_offset:0x%0x", r_offset);
  SLOW("b_offset:0x%0x", b_offset);
}

/** sunny_q13v04b_calc_gain:
 *
 * Calculate white balance gain
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_q13v04b_calc_gain(uint16_t R_target, uint16_t B_target,
  uint16_t *R_gain, uint16_t *B_gain, uint16_t *G_gain)
{
  /* 0x400 = 1x gain */
  if (otp_data.bg_ratio < B_target) {
    if (otp_data.rg_ratio < R_target) {
      *G_gain = 0x400;
      *B_gain = 0x400 * B_target / otp_data.bg_ratio;
      *R_gain = 0x400 * R_target / otp_data.rg_ratio;
    } else {
      *R_gain = 0x400;
      *G_gain = 0x400 * otp_data.rg_ratio / R_target;
      *B_gain = 0x400 * otp_data.rg_ratio * B_target
        / (otp_data.bg_ratio * R_target);
    }
  } else {
    if (otp_data.rg_ratio < R_target) {
      *B_gain = 0x400;
      *G_gain = 0x400 * otp_data.bg_ratio / B_target;
      *R_gain = 0x400 * otp_data.bg_ratio * R_target
        / (otp_data.rg_ratio * B_target);
    } else {
      if (B_target * otp_data.rg_ratio < R_target * otp_data.bg_ratio) {
        *B_gain = 0x400;
        *G_gain = 0x400 * otp_data.bg_ratio / B_target;
        *R_gain = 0x400 * otp_data.bg_ratio * R_target
        / (otp_data.rg_ratio * B_target);
      } else {
        *R_gain = 0x400;
        *G_gain = 0x400 * otp_data.rg_ratio / R_target;
        *B_gain = 0x400 * otp_data.rg_ratio * B_target
        / (otp_data.bg_ratio * R_target);
      }
    }
  }
}

/** sunny_q13v04b_update_awb:
 *
 * Calculate and apply white balance calibration data
 *
 * This function executes in eeprom module context
 *
 * Return: void.

 ^ B/G
 |
 |
 |         |----------|<----b---->|      a = R_offset_inside
 |         |          |           |      b = R_offset_outside
 |         |     |----|<-a->|     |      c = B_offset_inside
 |         |     |    |     |     |      d = B_offset_outside
 |         |     |    x-----|-----|
 |         |     |          c     |
 |         |     |----------|     d
 |         |                      |
 |         |----------------------|
 |
 |
 |                                           R/G
 o----------------------------------------------->

 **/
static void sunny_q13v04b_update_awb()
{
  uint16_t R_gain = 0;
  uint16_t G_gain = 0;
  uint16_t B_gain = 0;
  uint16_t R_offset_inside = 0;
  uint16_t B_offset_inside = 0;
  uint16_t R_offset_outside = 0;
  uint16_t B_offset_outside = 0;
  uint16_t R_target = 0;
  uint16_t B_target = 0;

  R_offset_inside = RG_TYPICAL_VALUE / 100; // ¡À1%
  R_offset_outside = RG_TYPICAL_VALUE * 3 / 100; // ¡À3%
  B_offset_inside = BG_TYPICAL_VALUE / 100;
  B_offset_outside = BG_TYPICAL_VALUE * 3 / 100;

  if(0 == otp_data.light_rg){
      otp_data.rg_ratio = otp_data.rg_ratio;
  } else {
    otp_data.rg_ratio = otp_data.rg_ratio * (otp_data.light_rg +512) / 1024;
  }
  if(0 == otp_data.light_bg){
      otp_data.bg_ratio = otp_data.bg_ratio;
  } else {
    otp_data.bg_ratio = otp_data.bg_ratio * (otp_data.light_bg +512) / 1024;
  }

  if ((ABS(otp_data.rg_ratio - RG_TYPICAL_VALUE)
    < R_offset_inside)
    && (ABS(otp_data.bg_ratio - BG_TYPICAL_VALUE)
    < B_offset_inside)) {
    R_gain = 0x400;
    G_gain = 0x400;
    B_gain = 0x400;
  } else {
    if ((ABS(otp_data.rg_ratio - RG_TYPICAL_VALUE) < R_offset_outside)
      && (ABS(otp_data.bg_ratio - BG_TYPICAL_VALUE) < B_offset_outside)) {
      sunny_q13v04b_calc_otp(otp_data.rg_ratio, otp_data.bg_ratio,
        &R_target, &B_target, R_offset_inside, B_offset_inside);
    } else {
      sunny_q13v04b_calc_otp(otp_data.rg_ratio, otp_data.bg_ratio,
        &R_target, &B_target, R_offset_outside, B_offset_outside);
    }
    sunny_q13v04b_calc_gain(R_target,B_target,&R_gain,&B_gain,&G_gain);
  }

#if SUNNY_Q13V04B_EEPROM_AWB_ENABLE
  SLOW("AWB REG UPDATE START");
  SLOW("R_gain:0x%0x", R_gain);
  SLOW("G_gain:0x%0x", G_gain);
  SLOW("B_gain:0x%0x", B_gain);
  if (R_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5056;
    g_reg_array[g_reg_setting.size].reg_data = R_gain >> 8;
    SLOW("0x%0x:0x%0x", g_reg_array[g_reg_setting.size].reg_addr,
      g_reg_array[g_reg_setting.size].reg_data);
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5057;
    g_reg_array[g_reg_setting.size].reg_data = R_gain & 0xff;
    SLOW("0x%0x:0x%0x", g_reg_array[g_reg_setting.size].reg_addr,
      g_reg_array[g_reg_setting.size].reg_data);
    g_reg_setting.size++;
  }
  if (G_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5058;
    g_reg_array[g_reg_setting.size].reg_data = G_gain >> 8;
    SLOW("0x%0x:0x%0x", g_reg_array[g_reg_setting.size].reg_addr,
      g_reg_array[g_reg_setting.size].reg_data);
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5059;
    g_reg_array[g_reg_setting.size].reg_data = G_gain & 0xff;
    SLOW("0x%0x:0x%0x", g_reg_array[g_reg_setting.size].reg_addr,
      g_reg_array[g_reg_setting.size].reg_data);
    g_reg_setting.size++;
  }
  if (B_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x505a;
    g_reg_array[g_reg_setting.size].reg_data = B_gain >> 8;
    SLOW("0x%0x:0x%0x", g_reg_array[g_reg_setting.size].reg_addr,
      g_reg_array[g_reg_setting.size].reg_data);
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x505b;
    g_reg_array[g_reg_setting.size].reg_data = B_gain & 0xff;
    SLOW("0x%0x:0x%0x", g_reg_array[g_reg_setting.size].reg_addr,
      g_reg_array[g_reg_setting.size].reg_data);
    g_reg_setting.size++;
  }
  SLOW("AWB REG UPDATE END");
#endif
}

/** sunny_q13v04b_read_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate read white balance success or not.
 **/
static int sunny_q13v04b_read_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t flag, temp;
  int index, offset;

  flag = e_ctrl->eeprom_params.buffer[AWB_FLAG_OFFSET];
  SLOW("AWB_FLAG = 0x%x",flag);
  if (((flag >> 6) && 0x03) == 1)
    index = 0;
  else if (((flag >> 4) && 0x03) == 1)
    index = 1;
  else if (((flag >> 2) && 0x03) == 1)
    index = 2;
  else
    index = -1;
  SLOW("index = 0x%x",index);
  if (index < 0) {
    SERR("No WB calibration data valid\n");
    return -1;
  }

  offset = AWB_FLAG_OFFSET + index * 5 + 1;
  temp = e_ctrl->eeprom_params.buffer[offset + 4];
  otp_data.rg_ratio = (e_ctrl->eeprom_params.buffer[offset] << 2)
    + ((temp >> 6) & 0x03);
  otp_data.bg_ratio = (e_ctrl->eeprom_params.buffer[offset + 1] << 2)
    + ((temp >> 4) & 0x03);
  otp_data.light_rg = (e_ctrl->eeprom_params.buffer[offset + 2] << 2)
    + ((temp >> 2) & 0x03);
  otp_data.light_bg = (e_ctrl->eeprom_params.buffer[offset + 3] << 2)
    + (temp & 0x03);

  SLOW("rg_ratio:0x%0x", otp_data.rg_ratio);
  SLOW("bg_ratio:0x%0x", otp_data.bg_ratio);
  SLOW("light_rg:0x%0x", otp_data.light_rg);
  SLOW("light_bg:0x%0x", otp_data.light_bg);
  return 0;
}

/** sunny_q13v04b_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_q13v04b_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  int rc = 0;
  rc = sunny_q13v04b_read_wbdata(e_ctrl);
  if(rc < 0)
    SERR("read wbdata failed");
  sunny_q13v04b_update_awb();
}


/** sunny_q13v04b_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_q13v04b_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t flag, temp;
  int index, offset, i;

  flag = e_ctrl->eeprom_params.buffer[LSC_FLAG_OFFSET];
  SLOW("LSC_FLAG = 0x%x",flag);
  if (((flag >> 6) && 0x03) == 1)
    index = 0;
  else if (((flag >> 4) && 0x03) == 1)
    index = 1;
  else if (((flag >> 2) && 0x03) == 1)
    index = 2;
  else
    index = -1;

  SLOW("index = 0x%x",index);
  if (index < 0) {
    SERR("No lens shading calibration data valid\n");
    return -1;
  }

  SLOW("LENC start");
  g_reg_array[g_reg_setting.size].reg_addr = LSC_ENABLE_REG_ADDR;
#if SUNNY_Q13V04B_EEPROM_LSC_ENABLE
  g_reg_array[g_reg_setting.size].reg_data
    = LSC_ENABLE_REG_DEFAULT_VALUE | 0x01;
#else
  g_reg_array[g_reg_setting.size].reg_data
    = LSC_ENABLE_REG_DEFAULT_VALUE & 0xFE;
#endif
  SLOW("0x%0x:0x%0x", g_reg_array[g_reg_setting.size].reg_addr,
    g_reg_array[g_reg_setting.size].reg_data);
  g_reg_setting.size++;
  offset = LSC_FLAG_OFFSET + index * OTP_DRV_LSC_SIZE + 1;
  for (i = 0; i < OTP_DRV_LSC_SIZE; i++) {
    g_reg_array[g_reg_setting.size].reg_addr = OTP_DRV_LSC_REG_ADDR + i;
    g_reg_array[g_reg_setting.size].reg_data =
      (uint8_t)(e_ctrl->eeprom_params.buffer[offset + i]);
    SLOW("0x%0x:0x%0x", g_reg_array[g_reg_setting.size].reg_addr,
      g_reg_array[g_reg_setting.size].reg_data);
    g_reg_setting.size++;
  }
  SLOW("LENC end");
  return 0;
}

/** sunny_q13v04b_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_q13v04b_format_calibration_data(void *e_ctrl) {
  SLOW("Enter %s", __func__);
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = MSM_CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;
  sunny_q13v04b_format_wbdata(ectrl);
  sunny_q13v04b_format_lensshading(ectrl);
  SLOW("Exit %s", __func__);
}

/** sunny_q13v04b_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * Return: void.
 **/
void sunny_q13v04b_get_raw_data(void *e_ctrl, void *data) {
  if (e_ctrl && data)
    memcpy(data, &g_reg_setting, sizeof(g_reg_setting));
  else
    SERR("failed Null pointer");
  return;
}

static eeprom_lib_func_t sunny_q13v04b_lib_func_ptr = {
  .get_calibration_items = sunny_q13v04b_get_calibration_items,
  .format_calibration_data = sunny_q13v04b_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .do_dpc_calibration = NULL,
  .get_dpc_calibration_info = NULL,
  .get_raw_data = sunny_q13v04b_get_raw_data,
};

/** sunny_q13v04b_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_q13v04b_eeprom_open_lib(void) {
  return &sunny_q13v04b_lib_func_ptr;
}
