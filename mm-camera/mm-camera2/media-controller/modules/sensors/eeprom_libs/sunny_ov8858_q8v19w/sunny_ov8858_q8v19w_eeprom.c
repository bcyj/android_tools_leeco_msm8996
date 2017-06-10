/*============================================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <dlfcn.h>
#include <math.h>
#include <stdlib.h>
#include "chromatix.h"
#include "eeprom.h"
#include "sensor_common.h"

#undef DEBUG_INFO
#ifdef SUNNY_OV8858_Q8V19W_DEBUG
#define DEBUG_INFO(fmt, args...) SERR(fmt, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif


#define BASE_ADDR 0x7010
#define INFO_GROUP_SIZE_R1A 5

#define VCM_FLAG_ADDR 0x7030
#define VCM_GROUP_SIZE 3

#define WB_FLAG_ADDR_R1A 0x7020
#define WB_GROUP_SIZE_R1A 5
#define AWB_REG_SIZE_R1A 6

#define LENS_FLAG_ADDR_R1A 0x703A
#define LENS_GROUP_SIZE_R1A 110
#define LSC_REG_SIZE_R1A 110

/*
Checked golden sample at 20140701
RG: 0x140
BG: 0x132
*/
#define RG_RATIO_TYPICAL_VALUE 0x13e
#define BG_RATIO_TYPICAL_VALUE 0x128

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
  uint16_t VCM_start;
  uint16_t VCM_end;
  uint16_t VCM_dir;
} otp_data;

struct msm_camera_i2c_reg_array g_reg_array[AWB_REG_SIZE_R1A + LSC_REG_SIZE_R1A];
struct msm_camera_i2c_reg_setting g_reg_setting;


/** sunny_ov8858_q8v19w_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_ov8858_q8v19w_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = TRUE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = FALSE;
  e_items->is_lsc = FALSE;
  e_items->is_dpc = FALSE;
}

/** sunny_ov8858_q8v19w_update_awb:
 *
 * Calculate and apply white balance calibration data
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_ov8858_q8v19w_update_awb()
{
  uint16_t R_gain, G_gain, B_gain;
  uint16_t G_gain_R, G_gain_B;
  uint16_t nR_G_gain, nB_G_gain, nG_G_gain;
  uint16_t nBase_gain;

  if (otp_data.light_rg)
    otp_data.rg_ratio = otp_data.rg_ratio * (otp_data.light_rg + 512) / 1024;

  if (otp_data.light_bg)
    otp_data.bg_ratio = otp_data.bg_ratio * (otp_data.light_bg + 512) / 1024;

  DEBUG_INFO(": rg_ratio=0x%x,bg_ratio=0x%x,light_rg=0x%x,light_bg=0x%x",
    otp_data.rg_ratio,otp_data.bg_ratio,otp_data.light_rg,otp_data.light_bg);


  nR_G_gain = (RG_RATIO_TYPICAL_VALUE*1000) / otp_data.rg_ratio;
  nB_G_gain = (BG_RATIO_TYPICAL_VALUE*1000) / otp_data.bg_ratio;
  nG_G_gain = 1000;
  if (nR_G_gain < 1000 || nB_G_gain < 1000)
  {
    if (nR_G_gain < nB_G_gain)
      nBase_gain = nR_G_gain;
    else
      nBase_gain = nB_G_gain;
  }
  else
  {
    nBase_gain = nG_G_gain;
  }
  R_gain = 0x400 * nR_G_gain / (nBase_gain);
  B_gain = 0x400 * nB_G_gain / (nBase_gain);
  G_gain = 0x400 * nG_G_gain / (nBase_gain);

  DEBUG_INFO("R_gain=0x%x,G_gain=0x%x,B_gain=0x%x",R_gain,G_gain,B_gain);

  if (R_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5032;
    g_reg_array[g_reg_setting.size].reg_data = R_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5033;
    g_reg_array[g_reg_setting.size].reg_data = R_gain & 0x00ff;
    g_reg_setting.size++;
  }
  if (G_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5034;
    g_reg_array[g_reg_setting.size].reg_data = G_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5035;
    g_reg_array[g_reg_setting.size].reg_data = G_gain & 0x00ff;
    g_reg_setting.size++;
  }
  if (B_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5036;
    g_reg_array[g_reg_setting.size].reg_data = B_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5037;
    g_reg_array[g_reg_setting.size].reg_data = B_gain & 0x00ff;
    g_reg_setting.size++;
  }
}

/** sunny_ov8858_q8v19w_get_group_index:
 *    @flag: group index register value
 *
 * Get which group is used
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate group index.
 **/
static int sunny_ov8858_q8v19w_get_group_index(uint8_t flag)
{
  int8_t group_index = -1;
  flag = flag & 0xFC;
  if((flag&0xC0) == 0x40){
    group_index = 0;
  }else if((flag&0x30) == 0x10){
    group_index = 1;
  }else if((flag&0x0C) == 0x04){
    group_index = 2;
  }else{
    group_index = -1;
  }

  return group_index;
}

/** sunny_ov8858_q8v19w_read_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read the data structure of product information like product date
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate read information success or not.
 **/
static int sunny_ov8858_q8v19w_read_info(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t flag,reg_val;
  int start_address = 0;
  int i = 0, group_index;
  SLOW("Enter");

  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[BASE_ADDR - BASE_ADDR]);
  if((group_index=sunny_ov8858_q8v19w_get_group_index(flag))==-1){
    SERR("%s:invalid or empty opt data",__func__);
    return -1;
  }
  start_address = (BASE_ADDR - BASE_ADDR )+ (INFO_GROUP_SIZE_R1A*group_index) + 1;

  otp_data.module_integrator_id =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address]);
  otp_data.lens_id =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 1]);
  otp_data.production_year =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 2]);
  otp_data.production_month =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 3]);
  otp_data.production_day =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 4]);

  return 0;
}

static int sunny_ov8858_q8v19w_read_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t flag ;
  uint8_t temp ;
  uint8_t reg_val  ;
  int group_index  = -1 ;
  int start_address = 0;
  SLOW("Enter");
  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_FLAG_ADDR_R1A - BASE_ADDR]);

  if((group_index=sunny_ov8858_q8v19w_get_group_index(flag))==-1){
    SERR("%s:invalid or empty awb data",__func__) ;
    return -1 ;
  }

  start_address = ( WB_FLAG_ADDR_R1A - BASE_ADDR )+ (group_index*WB_GROUP_SIZE_R1A) + 1;
  temp = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 4]);

  DEBUG_INFO("temp=0x%x ",temp);

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address]);
  DEBUG_INFO("reg_val=0x%x",reg_val);
  otp_data.rg_ratio = (reg_val<<2) + ((temp>>6)&0x03);

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 1]);
  DEBUG_INFO("reg_val=0x%x",reg_val);
  otp_data.bg_ratio = (reg_val<<2) + ((temp>>4)&0x03);

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 2]);
  DEBUG_INFO("reg_val=0x%x",reg_val) ;
  otp_data.light_rg = (reg_val<<2) + ((temp>>2)&0x03)  ;

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 3]);
  DEBUG_INFO("reg_val=0x%x",reg_val) ;
  otp_data.light_bg =  (reg_val<<2) + (temp&0x03) ;

  return 0;
}

/** sunny_ov8858_q8v19w_read_VCMdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate read VCM data success or not.
 **/
static int sunny_ov8858_q8v19w_read_VCMdata(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t flag;
  uint8_t temp;
  uint8_t reg_val;
  int start_address = 0;
  int group_index  = -1;
  SLOW("Enter");
  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[VCM_FLAG_ADDR - BASE_ADDR]);

  if((group_index=sunny_ov8858_q8v19w_get_group_index(flag))==-1){
    SERR("%s:invalid or empty VCM data",__func__);
    return -1;
  }

  start_address = (VCM_FLAG_ADDR - BASE_ADDR)+(group_index*VCM_GROUP_SIZE) +1;
  temp = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 2]);
  DEBUG_INFO("temp=0x%x",temp);

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address]);
  DEBUG_INFO("reg_val=0x%x",reg_val);
  otp_data.VCM_start = (reg_val<<2) + ((temp>>6)&0x03);

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 1]);
  DEBUG_INFO("reg_val=0x%x",reg_val);
  otp_data.VCM_end = (reg_val<<2) + ((temp>>4)&0x03);

  otp_data.VCM_dir = ((temp>>2)&0x03);

  return 0;
}


/** sunny_ov8858_q8v19w_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_ov8858_q8v19w_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  SLOW("Enter");
  int rc = 0;
  rc = sunny_ov8858_q8v19w_read_wbdata(e_ctrl);
  if(rc < 0)
    SERR("read wbdata failed");
  sunny_ov8858_q8v19w_update_awb();

  SLOW("Exit");
}


/** sunny_ov8858_q8v19w_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_ov8858_q8v19w_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  int group_index  = -1;
  int group_offset = 0;
  int j = 0;
  uint8_t flag;
  int lens_offset = 0;
  int start_address = 0;

  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[LENS_FLAG_ADDR_R1A - BASE_ADDR]);

  if((group_index=sunny_ov8858_q8v19w_get_group_index(flag))==-1){
    SERR("%s:invalid or empty lensshading data",__func__);
    return -1;
  }

  start_address = (LENS_FLAG_ADDR_R1A - BASE_ADDR) + (group_index * LENS_GROUP_SIZE_R1A) + 1;
  for(j = 0; j < LSC_REG_SIZE_R1A; j++) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5800 + j;
	g_reg_array[g_reg_setting.size].reg_data =
	  (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + j]);
	g_reg_setting.size++;
    DEBUG_INFO("reg_val=0x%x",
      (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + j]));
  }
  SLOW("Exit");
}

/** sunny_ov8858_q8v19w_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_ov8858_q8v19w_format_calibration_data(void *e_ctrl) {
  uint8_t flag =0;
  int r1a_version = -1;
  SERR("Enter");
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = MSM_CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;
  sunny_ov8858_q8v19w_read_info(ectrl);
  sunny_ov8858_q8v19w_format_wbdata(ectrl);
  sunny_ov8858_q8v19w_format_lensshading(ectrl);

  SERR("Exit");
}

/** sunny_ov8858_q8v19w_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * Return: void.
 **/
void sunny_ov8858_q8v19w_get_raw_data(void *e_ctrl, void *data) {
  if (e_ctrl && data) {
    memcpy(data, &g_reg_setting, sizeof(g_reg_setting));
  } else
    SERR("failed Null pointer");
  return;
}

static eeprom_lib_func_t sunny_ov8858_q8v19w_lib_func_ptr = {
  .get_calibration_items = sunny_ov8858_q8v19w_get_calibration_items,
  .format_calibration_data = sunny_ov8858_q8v19w_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .do_dpc_calibration = NULL,
  .get_dpc_calibration_info = NULL,
  .get_raw_data = sunny_ov8858_q8v19w_get_raw_data,
};

/** sunny_ov8858_q8v19w_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_ov8858_q8v19w_eeprom_open_lib(void) {
  return &sunny_ov8858_q8v19w_lib_func_ptr;
}
