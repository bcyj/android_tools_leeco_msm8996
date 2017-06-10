/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <dlfcn.h>
#include <math.h>
#include <stdlib.h>
#include "chromatix.h"
#include "eeprom.h"
#include "sensor_common.h"

#define SUNNY_Q13V06K_DEBUG
#undef DEBUG_INFO
#ifdef SUNNY_Q13V06K_DEBUG
#define DEBUG_INFO(fmt, args...) SERR(fmt, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif

#define RG_TYPICAL_VALUE             0x128
#define BG_TYPICAL_VALUE             0x131

#define ABS(x)                      (((x) < 0) ? -(x) : (x))

#define OTP_DRV_START_ADDR          (0x7220)
#define OTP_DRV_END_ADDR            (0x73BA)
#define ADDR_BASIG_INFO_FLAG        (0x7220)
#define BASIC_INFO_GRP_SIZE         (0x7228 - 0x7220)
#define ADDR_LSC_INFO_FLAG          (0x7231)
#define LSC_INFO_GRP_SIZE           (0x72EE - 0x7231)
#define ADDR_VCM_INFO_FLAG          (0x73AC)
#define VCM_INFO_GRP_SIZE           (0x73AF - 0x73AC)

#define OTP_DRV_LSC_SIZE 186
#define AWB_REG_SIZE 6
#define LSC_REG_SIZE 360
#define OTP_REG_ARRAY_SIZE (AWB_REG_SIZE + LSC_REG_SIZE)

struct otp_struct {
  uint16_t flag; /*01:awb,10:lsc;100;af*/
  uint16_t module_integrator_id;
  uint16_t lens_id;
  uint16_t production_year;
  uint16_t production_month;
  uint16_t production_day;
  uint16_t rg_ratio;
  uint16_t bg_ratio;
  uint16_t checksumLSC;
  uint16_t checksumOTP;
  uint16_t checksumTotal;
  uint8_t lenc[OTP_DRV_LSC_SIZE];
  uint16_t VCM_start;
  uint16_t VCM_end;
  uint16_t VCM_dir;
} otp_data;

struct msm_camera_i2c_reg_array g_reg_array[OTP_REG_ARRAY_SIZE];
struct msm_camera_i2c_reg_setting g_reg_setting;

/** sunny_q13v06k_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_q13v06k_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = TRUE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = TRUE;
  e_items->is_lsc = TRUE;
  e_items->is_dpc = FALSE;
}
static uint8_t get_data_from_buffer(uint16_t addr,
  sensor_eeprom_data_t *e_ctrl)
{
  if(!e_ctrl){
    SERR("%s:null pointer exception", __func__);
    return 0;
  }
  if(addr > OTP_DRV_END_ADDR){
    SERR("%s: addr accessed is overflowed", __func__);
    return 0 ;
  }
  return (uint8_t)(e_ctrl->eeprom_params.buffer[addr-OTP_DRV_START_ADDR]);
}
/** sunny_q13v06k_update_awb:
 *
 * Calculate and apply white balance calibration data
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/

static void sunny_q13v06k_update_awb()
{
  uint32_t rg, bg, R_gain, G_gain, B_gain, Base_gain;
  if(otp_data.flag & 0x01){
    rg = otp_data.rg_ratio;
  bg = otp_data.bg_ratio;

  R_gain = (RG_TYPICAL_VALUE*1000) / rg;
  B_gain = (BG_TYPICAL_VALUE*1000) / bg;
  G_gain = 1000;

  if(R_gain < 1000 || B_gain < 1000){
    if (R_gain < B_gain)
      Base_gain = R_gain;
    else
      Base_gain = B_gain;
  }else
    Base_gain = G_gain;

  R_gain = 0x400 * R_gain / (Base_gain);
  B_gain = 0x400 * B_gain / (Base_gain);
  G_gain = 0x400 * G_gain / (Base_gain);

  if (R_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5056;
    g_reg_array[g_reg_setting.size].reg_data = R_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5057;
    g_reg_array[g_reg_setting.size].reg_data = R_gain & 0xff;
    g_reg_setting.size++;
  }
  if (G_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5058;
    g_reg_array[g_reg_setting.size].reg_data = G_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x5059;
    g_reg_array[g_reg_setting.size].reg_data = G_gain & 0xff;
    g_reg_setting.size++;
  }
  if (B_gain > 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x505a;
    g_reg_array[g_reg_setting.size].reg_data = B_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x505b;
    g_reg_array[g_reg_setting.size].reg_data = B_gain & 0xff;
    g_reg_setting.size++;
  }
  }
}

/** sunny_q13v06k_read_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate read white balance success or not.
 **/
static int sunny_q13v06k_read_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t  flag = get_data_from_buffer(ADDR_BASIG_INFO_FLAG, e_ctrl);
  uint8_t  lsb_awb = 0;
  uint16_t basic_addr = 0;

  if((flag&0xC0) == 0x40)
    basic_addr = ADDR_BASIG_INFO_FLAG;
  else if((flag&0x30) == 0x10)
    basic_addr = ADDR_BASIG_INFO_FLAG + BASIC_INFO_GRP_SIZE;
  else{
    SERR("no valid wbc opt data");
    return -1;
  }

  otp_data.flag = otp_data.flag | 0x01;

  otp_data.module_integrator_id = get_data_from_buffer(basic_addr+1,e_ctrl);
  otp_data.lens_id = get_data_from_buffer(basic_addr+2,e_ctrl);
  otp_data.production_year = get_data_from_buffer(basic_addr+3,e_ctrl) + 2000;
  otp_data.production_month = get_data_from_buffer(basic_addr+4,e_ctrl);
  otp_data.production_day = get_data_from_buffer(basic_addr+5,e_ctrl);
  otp_data.rg_ratio = get_data_from_buffer(basic_addr+6,e_ctrl);
  otp_data.bg_ratio = get_data_from_buffer(basic_addr+7,e_ctrl);
  lsb_awb = get_data_from_buffer(basic_addr+8,e_ctrl);
  otp_data.rg_ratio = (otp_data.rg_ratio << 2) + ((lsb_awb>>6) & 0x03);
  otp_data.bg_ratio = (otp_data.bg_ratio << 2) + ((lsb_awb>>4) & 0x03);

  DEBUG_INFO("vendor_id:%d;lens_id:%d; date of product:%d/%d/%d",
    otp_data.module_integrator_id, otp_data.lens_id,
    otp_data.production_year, otp_data.production_month,
    otp_data.production_day);
  DEBUG_INFO("rg_ratio:0x%0x", otp_data.rg_ratio);
  DEBUG_INFO("bg_ratio:0x%0x", otp_data.bg_ratio);
  return 0;
}

/** sunny_q13v06k_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_q13v06k_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  int rc = 0;
  rc = sunny_q13v06k_read_wbdata(e_ctrl);
  if(rc < 0)
    SERR("read wbdata failed");
  sunny_q13v06k_update_awb();
}
void LumaDecoder(uint8_t *pData, uint8_t *pPara)
{
  uint32_t Offset, Bit, Option;
  uint32_t i, k;
  uint8_t pCenter[16], pMiddle[32], pCorner[72];
  Offset = pData[0];
  Bit = pData[1]>>4;
  Option = pData[1]&0xf;

  if(Bit <= 5){
    for(i=0,k=2; i<120; i+=8,k+=5){
      pPara[i] = pData[k]>>3;
      pPara[i+1] = ((pData[k]&0x7)<<2)|(pData[k+1]>>6);
      pPara[i+2] = (pData[k+1]&0x3e)>>1;
      pPara[i+3] = ((pData[k+1]&0x1)<<4)|(pData[k+2]>>4);
      pPara[i+4] = ((pData[k+2]&0xf)<<1)|(pData[k+3]>>7);
      pPara[i+5] = (pData[k+3]&0x7c)>>2;
      pPara[i+6] = ((pData[k+3]&0x3)<<3)|(pData[k+4]>>5);
      pPara[i+7] = pData[k+4]&0x1f;
    }
  }else{
    for(i=0,k=2; i<48; i+=8,k+=5){
      pPara[i] = pData[k]>>3;
      pPara[i+1] = ((pData[k]&0x7)<<2)|(pData[k+1]>>6);
      pPara[i+2] = (pData[k+1]&0x3e)>>1;
      pPara[i+3] = ((pData[k+1]&0x1)<<4)|(pData[k+2]>>4);
      pPara[i+4] = ((pData[k+2]&0xf)<<1)|(pData[k+3]>>7);
      pPara[i+5] = (pData[k+3]&0x7c)>>2;
      pPara[i+6] = ((pData[k+3]&0x3)<<3)|(pData[k+4]>>5);
      pPara[i+7] = pData[k+4]&0x1f;
    }

    for(i=48,k=32; i<120; i+=4,k+=3){
      pPara[i] = pData[k]>>2;
      pPara[i+1] = ((pData[k]&0x3)<<4)|(pData[k+1]>>4);
      pPara[i+2] = ((pData[k+1]&0xf)<<2)|(pData[k+2]>>6);
      pPara[i+3] = pData[k+2]&0x3f;
    }

    memcpy(pCenter, pPara, 16);
    memcpy(pMiddle, pPara+16, 32);
    memcpy(pCorner, pPara+48, 72);
    for(i=0; i<32; i++){
      pMiddle[i] <<= (Bit-6);
    }
    for(i=0; i<72; i++){
      pCorner[i] <<= (Bit-6);
    }
    if(Option == 0){
      memcpy(pPara, pCorner, 26);
      memcpy(pPara+26, pMiddle, 8);
      memcpy(pPara+34, pCorner+26, 4);
      memcpy(pPara+38, pMiddle+8, 2);
      memcpy(pPara+40, pCenter, 4);
      memcpy(pPara+44, pMiddle+10, 2);
      memcpy(pPara+46, pCorner+30, 4);
      memcpy(pPara+50, pMiddle+12, 2);
      memcpy(pPara+52, pCenter+4, 4);
      memcpy(pPara+56, pMiddle+14, 2);
      memcpy(pPara+58, pCorner+34, 4);
      memcpy(pPara+62, pMiddle+16, 2);
      memcpy(pPara+64, pCenter+8, 4);
      memcpy(pPara+68, pMiddle+18, 2);
      memcpy(pPara+70, pCorner+38, 4);
      memcpy(pPara+74, pMiddle+20, 2);
      memcpy(pPara+76, pCenter+12, 4);
      memcpy(pPara+80, pMiddle+22, 2);
      memcpy(pPara+82, pCorner+42, 4);
      memcpy(pPara+86, pMiddle+24, 8);
      memcpy(pPara+94, pCorner+46, 26);
    }else{
      memcpy(pPara, pCorner, 22);
      memcpy(pPara+22, pMiddle, 6);
      memcpy(pPara+28, pCorner+22, 4);
      memcpy(pPara+32, pMiddle+6, 6);
      memcpy(pPara+38, pCorner+26, 4);
      memcpy(pPara+42, pMiddle+12, 1);
      memcpy(pPara+43, pCenter, 4);
      memcpy(pPara+47, pMiddle+13, 1);
      memcpy(pPara+48, pCorner+30, 4);
      memcpy(pPara+52, pMiddle+14, 1);
      memcpy(pPara+53, pCenter+4, 4);
      memcpy(pPara+57, pMiddle+15, 1);
      memcpy(pPara+58, pCorner+34, 4);
      memcpy(pPara+62, pMiddle+16, 1);
      memcpy(pPara+63, pCenter+8, 4);
      memcpy(pPara+67, pMiddle+17, 1);
      memcpy(pPara+68, pCorner+38, 4);
      memcpy(pPara+72, pMiddle+18, 1);
      memcpy(pPara+73, pCenter+12, 4);
      memcpy(pPara+77, pMiddle+19, 1);
      memcpy(pPara+78, pCorner+42, 4);
      memcpy(pPara+82, pMiddle+20, 6);
      memcpy(pPara+88, pCorner+46, 4);
      memcpy(pPara+92, pMiddle+26, 6);
      memcpy(pPara+98, pCorner+50, 22);
    }
  }

  for(i=0; i<120; i++){
    pPara[i] += Offset;
  }
}


void ColorDecoder(uint8_t *pData, uint8_t *pPara)
{
  uint32_t Offset, Bit, Option;
  uint32_t i, k;
  uint8_t pBase[30];

  Offset = pData[0];
  Bit = pData[1]>>7;
  Option = (pData[1]&0x40)>>6;
  pPara[0] = (pData[1]&0x3e)>>1;
  pPara[1] = ((pData[1]&0x1)<<4)|(pData[2]>>4);
  pPara[2] = ((pData[2]&0xf)<<1)|(pData[3]>>7);
  pPara[3] = (pData[3]&0x7c)>>2;
  pPara[4] = ((pData[3]&0x3)<<3)|(pData[4]>>5);
  pPara[5] = pData[4]&0x1f;

  for(i=6,k=5; i<30; i+=8,k+=5){
    pPara[i] = pData[k]>>3;
    pPara[i+1] = ((pData[k]&0x7)<<2)|(pData[k+1]>>6);
    pPara[i+2] = (pData[k+1]&0x3e)>>1;
    pPara[i+3] = ((pData[k+1]&0x1)<<4)|(pData[k+2]>>4);
    pPara[i+4] = ((pData[k+2]&0xf)<<1)|(pData[k+3]>>7);
    pPara[i+5] = (pData[k+3]&0x7c)>>2;
    pPara[i+6] = ((pData[k+3]&0x3)<<3)|(pData[k+4]>>5);
    pPara[i+7] = pData[k+4]&0x1f;
  }
  memcpy(pBase, pPara, 30);

  for(i=0,k=20; i<120; i+=4,k++){
    pPara[i] = pData[k]>>6;
    pPara[i+1] = (pData[k]&0x30)>>4;
    pPara[i+2] = (pData[k]&0xc)>>2;
    pPara[i+3] = pData[k]&0x3;
  }

  if(Option == 0){
    for(i=0; i<5; i++){
      for(k=0; k<6; k++){
        pPara[i*24+k*2] += pBase[i*6+k];
        pPara[i*24+k*2+1] += pBase[i*6+k];
        pPara[i*24+k*2+12] += pBase[i*6+k];
        pPara[i*24+k*2+13] += pBase[i*6+k];
      }
    }
  }else{
    for(i=0; i<6; i++){
      for(k=0; k<5; k++){
        pPara[i*20+k*2] += pBase[i*5+k];
        pPara[i*20+k*2+1] += pBase[i*5+k];
        pPara[i*20+k*2+10] += pBase[i*5+k];
        pPara[i*20+k*2+11] += pBase[i*5+k];
      }
    }
  }

  for(i=0; i<120; i++){
    pPara[i] = (pPara[i]<<Bit) + Offset;
  }
}

void OV13850_R2A_LENC_Decoder(uint8_t *pData, uint8_t *pPara)
{
  LumaDecoder(pData, pPara);
  ColorDecoder(pData+86, pPara+120);
  ColorDecoder(pData+136, pPara+240);
}

/** sunny_q13v06k_read_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read lens shading calibration data from OTP
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_q13v06k_read_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t  flag = get_data_from_buffer(ADDR_LSC_INFO_FLAG, e_ctrl);
  uint16_t basic_add = -1;
  uint8_t *buffer = 0;
  int32_t  i = 0;

  if((flag & 0xc0) == 0x40) {
    buffer = e_ctrl->eeprom_params.buffer +
      (ADDR_LSC_INFO_FLAG-ADDR_BASIG_INFO_FLAG) + 1;
  }else if((flag & 0x30) == 0x10) {
    buffer = e_ctrl->eeprom_params.buffer +
      (ADDR_LSC_INFO_FLAG-ADDR_BASIG_INFO_FLAG) +
      LSC_INFO_GRP_SIZE + 1;
  }else {
    SERR("no valid lsc otp data");
    return ;
  }
  otp_data.flag = otp_data.flag | 0x02;

  if(buffer != 0){
    for(i = 0; i < OTP_DRV_LSC_SIZE; i++){
      otp_data.lenc[i] = buffer[i];
    }
    otp_data.checksumLSC = buffer[i];
    otp_data.checksumOTP = buffer[i+1];
    otp_data.checksumTotal = buffer[i+2];
  }
}
/** sunny_q13v06k_generate_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * decode len shading data from otp and apply it
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_q13v06k_generate_lensshading()
{
  uint32_t i;
  uint8_t lenc_out[360];
  uint32_t checksumLSC = 0, checksumOTP = 0, checksumTotal = 0;
  uint32_t lenc_flag  = 0;
  uint8_t ret = 0;
  uint32_t addr = 0, otp_flag = 0;
  uint8_t data_in[186];
  if(otp_data.flag & 0x02){
    memset(data_in, 0, sizeof(data_in));
    memset(lenc_out, 0, sizeof(lenc_out));

    for(i = 0; i < 186; i++)
      checksumLSC += otp_data.lenc[i];

    for(i = 0; i < sizeof(data_in); i++)
      data_in[i] = otp_data.lenc[i];

      OV13850_R2A_LENC_Decoder(data_in, lenc_out);

      for(i = 0; i < 360; i++)
        checksumOTP += lenc_out[i];

      checksumLSC = (checksumLSC)%255 + 1;
      checksumOTP = (checksumOTP)%255 + 1;
      checksumTotal = (checksumLSC) ^ (checksumOTP);

      if(otp_data.checksumLSC == checksumLSC && otp_data.checksumOTP == checksumOTP)
        lenc_flag = 1;
      else if(otp_data.checksumTotal == checksumTotal)
        lenc_flag = 1;

      if(lenc_flag){
        for(i=0;i<360 ;i++) {
          g_reg_array[g_reg_setting.size].reg_addr = 0x5200 +i;
          g_reg_array[g_reg_setting.size].reg_data = lenc_out[i];
          g_reg_setting.size++;
        }
        SERR("OTP decode successfully");
      }else{
        SERR("OTP decode failed");
      }
  }
}

/** sunny_q13v06k_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_q13v06k_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  sunny_q13v06k_read_lensshading(e_ctrl);
  sunny_q13v06k_generate_lensshading();
}

/** sunny_q13v06k_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_q13v06k_format_calibration_data(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = MSM_CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;

  if(ectrl->eeprom_params.num_bytes == 0 || !ectrl->eeprom_params.buffer){
    SERR("%s:no OTP data ",__func__);
    return ;
  }
  sunny_q13v06k_format_wbdata(ectrl);
  sunny_q13v06k_format_lensshading(ectrl);
}

/** sunny_q13v06k_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * Return: void.
 **/
void sunny_q13v06k_get_raw_data(void *e_ctrl, void *data) {
  if (e_ctrl && data)
    memcpy(data, &g_reg_setting, sizeof(g_reg_setting));
  else
    SERR("failed Null pointer");
  return;
}

static eeprom_lib_func_t sunny_q13v06k_lib_func_ptr = {
  .get_calibration_items = sunny_q13v06k_get_calibration_items,
  .format_calibration_data = sunny_q13v06k_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .do_dpc_calibration = NULL,
  .get_dpc_calibration_info = NULL,
  .get_raw_data = sunny_q13v06k_get_raw_data,
};

/** sunny_q13v06k_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_q13v06k_eeprom_open_lib(void) {
  return &sunny_q13v06k_lib_func_ptr;
}
