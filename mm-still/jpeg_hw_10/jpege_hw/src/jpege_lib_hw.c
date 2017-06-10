/*******************************************************************************
*                                                                         .
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential                      .
*                                                                         .
*******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <media/msm_jpeg.h>
#include "jpege_lib_hw.h"
#include <jpeg_lib_hw_reg.h>

#include <jpeg_hw_dbg.h>

#define HW_BUF_SIZE_TBL_ENTRIES 15

static struct msm_jpeg_hw_cmds *jpege_lib_hw_cmd_malloc_and_copy (uint16_t
                     size,
                     struct
                     msm_jpeg_hw_cmd
                     *p_hw_cmd)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;

  p_hw_cmds =
    malloc (sizeof (struct msm_jpeg_hw_cmds) -
      sizeof (struct msm_jpeg_hw_cmd) + size);
  if (p_hw_cmds) {
    p_hw_cmds->m = size / sizeof (struct msm_jpeg_hw_cmd);
    if (p_hw_cmd)
      memcpy (p_hw_cmds->hw_cmd, p_hw_cmd, size);
  }
  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_get_version = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  MSM_JPEG_HW_CMD_TYPE_READ, 1, JPEG_HW_VERSION_ADDR,
    JPEG_HW_VERSION_BMSK, {0},
};

void jpege_lib_hw_get_version (struct msm_jpeg_hw_cmd *p_hw_cmd)
{
  memcpy (p_hw_cmd, &hw_cmd_get_version, sizeof (hw_cmd_get_version));
  return;
}


struct msm_jpeg_hw_cmd hw_cmd_stop_offline[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_IRQ_CLEAR_ADDR,
   JPEG_IRQ_CLEAR_BMSK, {0x10}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_IRQ_CLEAR_ADDR,
   JPEG_IRQ_CLEAR_BMSK, {0x20}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_IRQ_CLEAR_ADDR,
   JPEG_IRQ_CLEAR_BMSK, {0x01}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_stop_offline (void)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_stop_offline),
               hw_cmd_stop_offline);
  if (!p_hw_cmds) {
    return NULL;
  }

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmds *jpege_lib_hw_stop ()
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;

  p_hw_cmds = jpege_lib_hw_stop_offline ();

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_start[] = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */

  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_IRQ_MASK_ADDR,
   JPEG_IRQ_MASK_BMSK, {JPEG_IRQ_ALLSOURCES_ENABLE}},
   //Update mask after updating the kernel
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_CMD_ADDR,
   0xFFFFFFFF, {JPEG_OFFLINE_CMD_START}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_start()
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_start),
               hw_cmd_start);
  if (!p_hw_cmds) {
    return NULL;
  }

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_core_cfg[] ={

  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_JPEG_CORE_CFG_ADDR,//core cfg
     JPEG_CORE_CFG_BMSK, {0x106005b}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_IRQ_MASK_ADDR,
     JPEG_IRQ_MASK_BMSK, {0x6455ca9c}},
};

struct msm_jpeg_hw_cmds *jpege_cmd_core_cfg (jpege_cmd_scale_cfg *
                                               p_scale_cfg)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;
  uint32_t reg_val = 0;
  uint32_t scale_enabled = JPEG_CORE_CFG__SCALE_ENABLE__SCALE_DISABLED;

  if (NULL == p_scale_cfg) {
    JPEG_HW_PR_ERR("%s %d: Bad parameter", __func__, __LINE__);
    return NULL;
  }
  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_core_cfg),
               hw_cmd_core_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  if(p_scale_cfg->scale_enable) {
    JPEG_HW_DBG("%s %d:Scale Enabled", __func__, __LINE__);
    scale_enabled = JPEG_CORE_CFG__SCALE_ENABLE__SCALE_ENABLED;
  }

  reg_val = ((JPEG_CORE_CFG__MODE__OFFLINE_JPEG_ENCODE<< JPEG_CORE_CFG__MODE_SHFT)|
             (JPEG_CORE_CFG__TESTBUS_ENABLE__TESTBUS_ENABLED<< JPEG_CORE_CFG__TESTBUS_ENABLE_SHFT)|
             (JPEG_CORE_CFG__BLOCK_FORMATTER_OUTPUT_ENABLED << JPEG_CORE_CFG__BLOCK_FORMATTER_OUTPUT_SHFT)|
             (scale_enabled << JPEG_CORE_CFG__SCALE_ENABLE_SHFT)|
             (JPEG_CORE_CFG__BLOCK_FORMATTER_ENABLE__BF_ENABLED<< JPEG_CORE_CFG__BLOCK_FORMATTER_ENABLE_SHFT)|
             (JPEG_CORE_CFG__JPEG_ENCODE_ENABLE__JPEG_ENCODE_ENABLED<< JPEG_CORE_CFG__JPEG_ENCODE_ENABLE_SHFT)|
             (JPEG_CORE_CFG__BRIDGE_ENABLE__BRIDGE_INTERFACE_ENABLED<< JPEG_CORE_CFG__BRIDGE_ENABLE_SHFT)|
             (JPEG_CORE_CFG__WE_ENABLE__WRITE_ENGINE_ENABLED<< JPEG_CORE_CFG__WE_ENABLE_SHFT)|
             (JPEG_CORE_CFG__FE_ENABLE__FETCH_ENGINE_ENABLED<< JPEG_CORE_CFG__FE_ENABLE_SHFT));

  JPEG_HW_DBG("jpege_cmd_core_cfg: core cfg value = %x\n",reg_val);
  p_hw_cmd->data = reg_val;
  p_hw_cmd++;
  p_hw_cmd++;

  return p_hw_cmds;
}


struct msm_jpeg_hw_cmd hw_cmd_fe_cfg[] = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
    //fe CFG
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_FE_CFG_ADDR,
   JPEG_FE_CFG_BMSK, {0xa01a070}},//cbcr , PLANAR and pln0 enabled as default.
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_FE_QOS_CFG_ADDR, //FE_QOS_CFG
   JPEG_FE_QOS_CFG_BMSK, {JPEG_FE_QOS_CFG_ADDR_VALUE}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_FE_QOS_LAT_HIGH_ADDR,//FE_QOS_LAT_HIGH
     JPEG_FE_QOS_LAT_HIGH_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_FE_QOS_LAT_MID_ADDR, //FE_QOS_LAT_MID
     JPEG_FE_QOS_LAT_MID_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_FE_QOS_LAT_LOW_ADDR, //FE_QOS_LAT_LOW
     JPEG_FE_QOS_LAT_LOW_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_fe_cfg(jpege_cmd_input_cfg *p_input_cfg,
  uint8_t mcus_per_blk)
{
  uint32_t reg_val = 0;
  uint8_t memory_format =0;
  uint8_t pln0_enable = 0, pln1_enable = 0, pln2_enable = 0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_fe_cfg),
               hw_cmd_fe_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  //fe CFG
  reg_val = p_hw_cmd->data;

  if(p_input_cfg->input_cbcr_order) {
  JPEG_HW_DBG("Input cbcr order = %d\n",
    p_input_cfg->input_cbcr_order);
  reg_val = (p_input_cfg->input_cbcr_order <<
            JPEG_FE_CFG__CBCR_ORDER_SHFT) | reg_val;
  }

  switch (p_input_cfg->num_of_input_plns) {
  case 1:
    memory_format = JPEG_FE_CFG__MEMORY_FORMAT__MONOCHROME;
    pln0_enable = 1;
    break;
  case 2:
    memory_format = JPEG_FE_CFG__MEMORY_FORMAT__PSEUDO_PLANAR;
    pln0_enable = pln1_enable = 1;
    break;
  case 3:
    memory_format = JPEG_FE_CFG__MEMORY_FORMAT__PLANAR;
    pln0_enable = pln1_enable = pln2_enable = 1;
    break;
  }

  JPEG_HW_DBG("FE_CFG: Num of input planes = %d, Memory format = %d\n",
    p_input_cfg->num_of_input_plns, memory_format);

  reg_val = (memory_format <<
    JPEG_FE_CFG__MEMORY_FORMAT_SHFT) |
    pln0_enable << JPEG_FE_CFG__PLN0_EN_SHFT |
    pln1_enable << JPEG_FE_CFG__PLN1_EN_SHFT |
    pln2_enable << JPEG_FE_CFG__PLN2_EN_SHFT |
    reg_val;

  reg_val |= (mcus_per_blk << JPEG_FE_CFG__MCU_BOUNDARY_SHFT);

  JPEG_HW_DBG("FE_CFG = %x\n",reg_val);
  p_hw_cmd->data = reg_val;
  p_hw_cmd++;

  return p_hw_cmds;

}

struct msm_jpeg_hw_cmd hw_cmd_fe_buffer_cfg[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN0_RD_BUFFER_SIZE_ADDR,
     JPEG_PLN0_RD_BUFFER_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN0_RD_STRIDE_ADDR, //PLN0_RD_STRIDE
     JPEG_PLN0_RD_STRIDE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN1_RD_BUFFER_SIZE_AADR,
     JPEG_PLN1_RD_BUFFER_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN1_RD_STRIDE_ADDR, //PLN1_RD_STRIDE
     JPEG_PLN1_RD_STRIDE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN2_RD_BUFFER_SIZE_ADDR,
     JPEG_PLN2_RD_BUFFER_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN2_RD_STRIDE_ADDR, //PLN1_RD_STRIDE
     JPEG_PLN2_RD_STRIDE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN0_RD_HINIT_ADDR, //PLN0_RD_HINIT
     JPEG_PLN0_RD_HINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN1_RD_HINIT_ADDR, //PLN1_RD_HINIT
     JPEG_PLN1_RD_HINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN2_RD_HINIT_ADDR, //PLN1_RD_HINIT
     JPEG_PLN2_RD_HINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN0_RD_VINIT_ADDR, //PLN0_RD_VINIT
     JPEG_PLN0_RD_VINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN1_RD_HINIT_ADDR, //PLN1_RD_HINIT
     JPEG_PLN1_RD_HINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN2_RD_HINIT_ADDR, //PLN1_RD_HINIT
     JPEG_PLN2_RD_HINIT_BMSK, {0}},
};


struct msm_jpeg_hw_cmds *jpege_lib_hw_fe_buffer_cfg
  (jpege_cmd_input_cfg * p_input_cfg,
  jpege_cmd_scale_cfg *p_scale_cfg){

  uint32_t reg_val = 0;
  uint32_t n_block_pattern = 0;
  uint32_t chroma_width =0;
  uint32_t chroma_height =0;
  int i = 0;
  uint32_t y_stride;
  uint32_t y_scanline;
  uint32_t c_stride;
  uint32_t c_scanline;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_fe_buffer_cfg),
    hw_cmd_fe_buffer_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  JPEG_HW_PR_ERR("%s:%d] w %d h %d stride %d scanline %d",
    __func__, __LINE__, p_input_cfg->image_width, p_input_cfg->image_height,
    p_input_cfg->stride, p_input_cfg->scanline);

  if (p_input_cfg->stride > p_input_cfg->image_width) {
    y_stride = p_input_cfg->stride;
    c_stride = p_input_cfg->stride;
  } else {
    y_stride = p_input_cfg->image_width;
    c_stride = p_input_cfg->image_width;
  }
  p_input_cfg->image_width = p_input_cfg->stride;

  if (p_input_cfg->scanline > p_input_cfg->image_height) {
    y_scanline = p_input_cfg->scanline;
    c_scanline = p_input_cfg->scanline;
  } else {
    y_scanline = p_input_cfg->image_height;
    c_scanline = p_input_cfg->image_height;
  }

  //PLN0_RD_BUFFER_SIZE
  reg_val = (((p_input_cfg->image_height -1)
            <<JPEG_PLN0_RD_BUFFER_SIZE__HEIGHT_SHFT)|
            (p_input_cfg->image_width -1));
  p_hw_cmd->data = reg_val;
  p_hw_cmd++;

  //PLN0_RD_STRIDE
  reg_val = y_stride;
  p_hw_cmd->data = reg_val;
  p_hw_cmd++;

  //PLN1_RD_BUFFER_SIZE

  switch(p_input_cfg->inputFormat) {
    case JPEGE_INPUT_H2V1:
          chroma_width = p_input_cfg->image_width;
          chroma_height = p_input_cfg->image_height;
          break;
    case JPEGE_INPUT_H2V2:
          chroma_width = p_input_cfg->image_width;
          chroma_height = floor(p_input_cfg->image_height/2);
          break;
    case JPEGE_INPUT_H1V2:
          chroma_width = p_input_cfg->image_width *2;
          chroma_height = floor(p_input_cfg->image_height/2);
          c_stride = c_stride * 2;
          break;
    case JPEGE_INPUT_H1V1:
          chroma_width = p_input_cfg->image_width *2;
          chroma_height = p_input_cfg->image_height;
          c_stride = c_stride * 2;
          break;
    case JPEGE_INPUT_MONOCHROME:
          chroma_width = 0;
          chroma_height = 0;
          c_stride = 0;
          break;
    case JPEGE_INPUT_FORMAT_MAX:
         JPEG_HW_PR_ERR("Invalid Color format = %d\n",
           p_input_cfg->inputFormat);
          return NULL;
    }
  if (p_input_cfg->num_of_input_plns == 3) {
      chroma_height = floor(chroma_height);
      chroma_width = floor(chroma_width/2);
      c_stride /= 2;
  }
  JPEG_HW_DBG("%s: numofplanes = %d, chroma height = %d, chroma width = %d\n",
     __func__, p_input_cfg->num_of_input_plns, chroma_height, chroma_width);
    reg_val = (((chroma_height -1)<<JPEG_PLN1_RD_BUFFER_SIZE__HEIGHT_SHFT)|
            (chroma_width -1));
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //PLN1_RD_STRIDE
    reg_val = c_stride;
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //PLN1_BUFFER_ADDR
    reg_val = (((chroma_height -1)<<JPEG_PLN2_RD_BUFFER_SIZE__HEIGHT_SHFT)|
            (chroma_width -1));
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //PLN2_RD_STRIDE
    reg_val = c_stride;
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_crop_cfg[] = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN0_RD_HINIT_INT_ADDR,
      JPEG_PLN0_RD_HINIT_INT_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN1_RD_HINIT_INT_ADDR,
      JPEG_PLN1_RD_HINIT_INT_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN2_RD_HINIT_INT_ADDR,
      JPEG_PLN2_RD_HINIT_INT_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN0_RD_VINIT_INT_ADDR,
      JPEG_PLN0_RD_VINIT_INT_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN1_RD_VINIT_INT_ADDR,
      JPEG_PLN1_RD_VINIT_INT_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_PLN2_RD_VINIT_INT_ADDR,
      JPEG_PLN2_RD_VINIT_INT_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_crop_cfg(jpege_cmd_scale_cfg *scale_cfg,
                                               jpege_cmd_input_cfg *p_input_cfg)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;
  uint32_t pln1_hoffset = 0, pln1_voffset =0;

   p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_crop_cfg),
               hw_cmd_crop_cfg);
    if (!p_hw_cmds) {
      return NULL;
    }
     p_hw_cmd = &p_hw_cmds->hw_cmd[0];

     switch(p_input_cfg->inputFormat) {
       case JPEGE_INPUT_H2V2:
         pln1_hoffset = floor(scale_cfg->h_offset/2);
         pln1_voffset = floor(scale_cfg->v_offset/2);
         break;
       case JPEGE_INPUT_H2V1:
         pln1_hoffset = floor(scale_cfg->h_offset/2);
         pln1_voffset = scale_cfg->v_offset;
         break;
       case JPEGE_INPUT_H1V1:
         pln1_hoffset = scale_cfg->h_offset;
         pln1_voffset = scale_cfg->v_offset;
         break;
       case JPEGE_INPUT_H1V2:
         pln1_hoffset = scale_cfg->h_offset;
         pln1_voffset = floor(scale_cfg->v_offset/2);
         break;
       case JPEGE_INPUT_MONOCHROME:
         pln1_hoffset = floor(scale_cfg->h_offset/2);
         pln1_voffset = scale_cfg->v_offset;
         break;
       case JPEGE_INPUT_FORMAT_MAX:
         JPEG_HW_PR_ERR("Invalid Color Format : %d\n",p_input_cfg->inputFormat);
         return NULL;
   }

     /*Set the hinit value to the horizontal offset.
       This takes care of cropping on the left*/
     p_hw_cmd->data = scale_cfg->h_offset;
     p_hw_cmd++;

     p_hw_cmd->data = pln1_hoffset;
     p_hw_cmd++;

     p_hw_cmd->data = pln1_hoffset;
     p_hw_cmd++;

     /*Set the vinit value to the horizontal offset
       This takes care of cropping on the top*/
     p_hw_cmd->data = scale_cfg->v_offset;
     p_hw_cmd++;

     p_hw_cmd->data = pln1_voffset;
     p_hw_cmd++;

     p_hw_cmd->data = pln1_voffset;
     p_hw_cmd++;

     return p_hw_cmds;
}


struct msm_jpeg_hw_cmd hw_cmd_default_scale_cfg[] = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  //core cfg
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN0_HSTEP_ADDR, //SCALE_PLN0_HSTEP
     JPEG_SCALE_PLN0_HSTEP_BMSK, {JPEG_SCALE_DEFAULT_HSTEP_VSTEP}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN1_HSTEP_ADDR, //SCALE_PLN1_HSTEP
     JPEG_SCALE_PLN1_HSTEP_BMSK, {JPEG_SCALE_DEFAULT_HSTEP_VSTEP}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN2_HSTEP_ADDR, //SCALE_PLN2_HSTEP
     JPEG_SCALE_PLN2_HSTEP_BMSK, {JPEG_SCALE_DEFAULT_HSTEP_VSTEP}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN0_VSTEP_ADDR, //SCALE_PLN0_VSTEP
     JPEG_SCALE_PLN0_VSTEP_BMSK, {JPEG_SCALE_DEFAULT_HSTEP_VSTEP}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN1_VSTEP_ADDR, //SCALE_PLN1_VSTEP
     JPEG_SCALE_PLN1_VSTEP_BMSK, {JPEG_SCALE_DEFAULT_HSTEP_VSTEP}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN2_VSTEP_ADDR, //SCALE_PLN1_VSTEP
     JPEG_SCALE_PLN2_VSTEP_BMSK, {JPEG_SCALE_DEFAULT_HSTEP_VSTEP}},
};


struct msm_jpeg_hw_cmds *jpege_lib_hw_default_scale_cfg()
{
  int i=0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_default_scale_cfg),
               hw_cmd_default_scale_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];
  for(i=1;i<=6;i++) {
    p_hw_cmd++;
  }

  return p_hw_cmds;

}

struct msm_jpeg_hw_cmd hw_cmd_scale_cfg[] = {
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_CFG_ADDR,
     JPEG_SCALE_CFG_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_scale_cfg(jpege_cmd_scale_cfg *scale_cfg,
                                                  jpege_cmd_input_cfg * pIn){

  uint32_t reg_val =0;
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;
  uint8_t hor_upscale =0,vert_upscale =0;
  uint8_t hor_downscale =0, vert_downscale =0;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_scale_cfg),
               hw_cmd_scale_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }

  /*Cropping is always done before scaling. If cropping is enables
    the input to the scaler is the cropped image. Use the cropped
    image size as input to the scalar */
  if(scale_cfg->scale_input_width < scale_cfg->output_width) {
    hor_upscale =1;
  }
  else if(scale_cfg->scale_input_width > scale_cfg->output_width) {
   hor_downscale =1;
  }

  if(scale_cfg->scale_input_height < scale_cfg->output_height) {
   vert_upscale=1;
  }
  else if(scale_cfg->scale_input_height > scale_cfg->output_height) {
    vert_downscale =1;
  }

  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  if(vert_upscale) {
    reg_val = (JPEG_SCALE_CFG__V_SCALE_FIR_ALGORITHM__BI_CUBIC
              << JPEG_SCALE_CFG__V_SCALE_FIR_ALGORITHM_SHFT);
    reg_val = reg_val | (JPEG_SCALE_CFG__VSCALE_ALGORITHM__FIR_UPSCALE
              << JPEG_SCALE_CFG__VSCALE_ALGORITHM_SHFT);
    JPEG_HW_DBG("jpege_lib_hw_scale_cfg: vert_upscale true reg_val = %x\n",
                 reg_val);
  }
  if(hor_upscale) {
    reg_val = reg_val | (JPEG_SCALE_CFG__H_SCALE_FIR_ALGORITHM__BI_CUBIC
                        << JPEG_SCALE_CFG__H_SCALE_FIR_ALGORITHM_SHFT);
    reg_val = reg_val |(JPEG_SCALE_CFG__HSCALE_ALGORITHM__FIR_UPSCALE
                       << JPEG_SCALE_CFG__HSCALE_ALGORITHM_SHFT);
    JPEG_HW_DBG("jpege_lib_hw_scale_cfg: hor_upscale true reg_val = %x\n",
                 reg_val);
  }
  if(hor_upscale | hor_downscale) {
    reg_val = reg_val | (JPEG_SCALE_CFG__HSCALE_ENABLED
                       << JPEG_SCALE_CFG__HSCALE_ENABLE_SHFT);
  }
  if(vert_upscale | vert_downscale) {
    reg_val = reg_val | (JPEG_SCALE_CFG__VSCALE_ENABLED
                       << JPEG_SCALE_CFG__VSCALE_ENABLE_SHFT);
  }
  JPEG_HW_DBG("jpege_lib_hw_scale_cfg : scale cfg value  %x\n",reg_val);
  p_hw_cmd->data = reg_val;

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_scale_reg_cfg[] = {
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN0_OUTPUT_CFG_ADDR,
     JPEG_SCALE_PLN0_OUTPUT_CFG_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN1_OUTPUT_CFG_ADDR,
     JPEG_SCALE_PLN1_OUTPUT_CFG_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN2_OUTPUT_CFG_ADDR,
     JPEG_SCALE_PLN2_OUTPUT_CFG_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN0_HSTEP_ADDR,
     JPEG_SCALE_PLN0_HSTEP_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN1_HSTEP_ADDR,
     JPEG_SCALE_PLN1_HSTEP_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN2_HSTEP_ADDR,
     JPEG_SCALE_PLN2_HSTEP_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN0_VSTEP_ADDR,
     JPEG_SCALE_PLN0_VSTEP_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN1_VSTEP_ADDR,
     JPEG_SCALE_PLN1_VSTEP_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_SCALE_PLN2_VSTEP_ADDR,
     JPEG_SCALE_PLN2_VSTEP_BMSK, {0}},
 };


struct msm_jpeg_hw_cmds *jpege_lib_hw_scale_reg_cfg(jpege_cmd_scale_cfg
                                        *scale_cfg, jpege_cmd_input_cfg * pIn){
  uint32_t reg_val =0;
  uint32_t block_height =0, block_width =0;
  uint32_t hor_scale_ratio =0, vert_scale_ratio=0;
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_scale_reg_cfg),
               hw_cmd_scale_reg_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }

   p_hw_cmd = &p_hw_cmds->hw_cmd[0];

   switch(pIn->inputFormat) {
   case JPEGE_INPUT_H2V2:
        block_height = 16;
        block_width = 16;
        break;
   case JPEGE_INPUT_H2V1:
        block_height = 8;
        block_width = 16;
        break;
   case JPEGE_INPUT_H1V1:
        block_height = 8;
        block_width = 8;
        break;
   case JPEGE_INPUT_H1V2:
        block_height = 16;
        block_width = 8;
        break;
   case JPEGE_INPUT_MONOCHROME:
        block_height = 8;
        block_width = 8;
        break;
   case JPEGE_INPUT_FORMAT_MAX:
        JPEG_HW_PR_ERR("Invalid Color Format : %d\n",pIn->inputFormat);
        return NULL;
   }
    reg_val = ((block_height-1)
               << JPEG_SCALE_PLN0_OUTPUT_CFG__BLOCK_HEIGHT_SHFT)
               |(block_width-1);
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //SCALE_PLN1_OUTPUT_CFG - is always 8x8
    block_height =8;
    block_width = 8;

    reg_val = ((block_height-1)
               << JPEG_SCALE_PLN0_OUTPUT_CFG__BLOCK_HEIGHT_SHFT)
              |(block_width-1);
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //SCALE_PLN2_OUTPUT_CFG - is always 8x8
    block_height =8;
    block_width = 8;

    reg_val = ((block_height-1)
              << JPEG_SCALE_PLN0_OUTPUT_CFG__BLOCK_HEIGHT_SHFT)
              |(block_width-1);
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    hor_scale_ratio = FLOAT_TO_Q(21, (double)scale_cfg->scale_input_width /
      (double)scale_cfg->output_width) ;
    vert_scale_ratio = FLOAT_TO_Q(21,(double)scale_cfg->scale_input_height /
      (double)scale_cfg->output_height);

    JPEG_HW_DBG("Scale HSTEP = %x, VSTEP =%x\n",hor_scale_ratio,
                vert_scale_ratio);

    //SCALE_PLN0_HSTEP
    reg_val = hor_scale_ratio;
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //SCALE_PLN1_HSTEP
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //SCALE_PLN2_HSTEP
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //SCALE_PLN0_VSTEP
    reg_val = vert_scale_ratio;
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //SCALE_PLN1_VSTEP
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //SCALE_PLN2_VSTEP
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    return p_hw_cmds;
}


struct msm_jpeg_hw_cmd hw_cmd_encode_state[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_PREDICTION_Y_STATE_ADDR,
   JPEG_ENCODE_PREDICTION_Y_STATE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG__ENCODE_PREDICTION_C_STATE_ADDR,
   JPEG_ENCODE_PREDICTION_C_STATE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_RSM_STATE_ADDR,
   JPEG_ENCODE_RSM_STATE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_PACKER_STATE_ADDR,
   JPEG_ENCODE_PACKER_STATE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_BYTE_PACKER_WORD0_STATE_ADDR,
   JPEG_ENCODE_BYTE_PACKER_WORD0_STATE_BMSK, {0}},
    {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_BYTE_PACKER_WORD1_STATE_ADDR,
   JPEG_ENCODE_BYTE_PACKER_WORD1_STATE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_BYTE_PACKER_WORD2_STATE_ADDR,
   JPEG_ENCODE_BYTE_PACKER_WORD2_STATE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_BYTE_PACKER_WORD3_STATE_ADDR,
   JPEG_ENCODE_BYTE_PACKER_WORD3_STATE_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_encode_state ()
{
  uint32_t nRegVal = 0;
  int i=0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_encode_state),
               hw_cmd_encode_state);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  for(i=0;i<8;i++) {
    p_hw_cmd++;
  }

  return p_hw_cmds;

}

struct msm_jpeg_hw_cmd hw_cmd_encode_cfg[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_CFG_ADDR,//ENCODE_CFG
    JPEG_ENCODE_CFG_BMSK, {0}},//0x82 - h2v1
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_IMAGE_SIZE_ADDR,
    JPEG_ENCODE_IMAGE_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_FE_VBPAD_CFG_ADDR,
    JPEG_FE_VBPAD_CFG_BMSK,{0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_STATS_CFG_ADDR,//ENCODE_STATS_CFG
   JPEG_ENCODE_STATS_CFG_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_INDEX_TABLE_CFG_ADDR,
   JPEG_ENCODE_INDEX_TABLE_CFG_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_FSC_REGION_SIZE_ADDR,
   JPEG_ENCODE_FSC_REGION_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_FSC_BUDGET_0_ADDR,
   JPEG_ENCODE_FSC_BUDGET_0_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_FSC_BUDGET_1_ADDR,
   JPEG_ENCODE_FSC_BUDGET_1_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_FSC_BUDGET_2_ADDR,
   JPEG_ENCODE_FSC_BUDGET_2_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_ENCODE_FSC_BUDGET_3_ADDR,
   JPEG_ENCODE_FSC_BUDGET_3_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_encode_cfg (jpege_cmd_input_cfg * pIn,
                                                jpege_cmd_scale_cfg *scale_cfg)
{
  uint32_t nRegVal = 0;
  uint32_t encoded_width, encoded_height, padded_height =0, padded_width =0;
  int hor_subsampling =0, vert_subsampling =0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_encode_cfg),
               hw_cmd_encode_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];
  nRegVal = (JPEG_ENCODE_CFG__APPLY_EOI_ENABLED
             << JPEG_ENCODE_CFG__APPLY_EOI_SHFT)
            | pIn->inputFormat;
  JPEG_HW_DBG("InputFormat =%d, Encode cfg = %x\n",pIn->inputFormat,nRegVal);
  p_hw_cmd->data = nRegVal;
  p_hw_cmd++;

 /*Encoded image size is the image size in MCUs after scaling if scaling is
   enabled or image size in MCUs after crop if cropping is enabled without
   scaling or input size in MCUs otherwise*/

  switch(pIn->inputFormat) {
  case JPEGE_INPUT_H2V2:{
    hor_subsampling = 2;
    vert_subsampling = 2;
    padded_height = CEILING16(scale_cfg->output_height);
    padded_width = CEILING16(scale_cfg->output_width);
    break;
  }
  case JPEGE_INPUT_H2V1:{
    hor_subsampling = 2;
    vert_subsampling = 1;
    padded_height = CEILING8(scale_cfg->output_height);
    padded_width = CEILING16(scale_cfg->output_width);
    break;
  }
  case JPEGE_INPUT_H1V2:{
    hor_subsampling = 1;
    vert_subsampling = 2;
    padded_height = CEILING16(scale_cfg->output_height);
    padded_width = CEILING8(scale_cfg->output_width);
    break;
  }
  case JPEGE_INPUT_H1V1:{
    hor_subsampling = 1;
    vert_subsampling = 1;
    padded_height = CEILING8(scale_cfg->output_height);
    padded_width = CEILING8(scale_cfg->output_width);
    break;
  }
  case JPEGE_INPUT_MONOCHROME:
    hor_subsampling = 1;
    vert_subsampling = 1;
    padded_height = CEILING8(scale_cfg->output_height);
    padded_width = CEILING8(scale_cfg->output_width);
    break;

  case JPEGE_INPUT_FORMAT_MAX:{
          JPEG_HW_PR_ERR("Invalid Color Format : %d\n",pIn->inputFormat);
          return NULL;
   }
  }
  encoded_height = (padded_height / (8*vert_subsampling)) - 1;
  encoded_width = (padded_width /(8 *hor_subsampling)) - 1;
  JPEG_HW_DBG("width = %d, padded_height = %d\n", padded_width,
              padded_height);
  JPEG_HW_DBG("encoded_height = %d , %x, encoded_width = %d, %x\n",
              encoded_height,encoded_height, encoded_width,encoded_width);
  nRegVal = (encoded_height
             << JPEG_ENCODE_IMAGE_SIZE__ENCODE_IMAGE_HEIGHT_SHFT)
             |encoded_width;
  JPEG_HW_DBG("nRegVal = %x\n",nRegVal);
  p_hw_cmd->data = nRegVal;
  p_hw_cmd++;

  //VBPad - same as encoded height
  nRegVal = encoded_height;
  p_hw_cmd->data = nRegVal;
  p_hw_cmd++;

  return p_hw_cmds;
}


struct msm_jpeg_hw_cmd hw_cmd_we_buffer_cfg[] = {

  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_PLN0_WR_BUFFER_SIZE_ADDR,
   JPEG_PLN0_WR_BUFFER_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_PLN0_WR_STRIDE_ADDR,
   JPEG_PLN0_WR_STRIDE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_PLN0_WR_HINIT_ADDR,//PLN0_WR_HINIT
   JPEG_PLN0_WR_HINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_PLN0_WR_VINIT_ADDR,//PLN0_WR_VINIT
   JPEG_PLN0_WR_VINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_PLN0_WR_HSTEP_ADDR,
   JPEG_PLN0_WR_HSTEP_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_PLN0_WR_VSTEP_ADDR,
   JPEG_PLN0_WR_VSTEP_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_PLN0_WR_BLOCK_CFG_ADDR,
   JPEG_PLN0_WR_BLOCK_CFG_BMSK, {0}},
};

uint32_t jpege_lib_hw_lookup_we_buf_height(uint32_t buf_size)
{
  uint32_t l_hw_buf_height = 0;
  int i;
  uint32_t lBufferRangeTbl [] = {
     1,
     131072,
     262144,
     524288,
     1048576,
     2097152,
     4194304,
     8388608,
     1677216,
     33554432,
     67108864,
     134217728,
     268435456,
     536870912,
     1073741824,
  };

  for (i = 0; i < HW_BUF_SIZE_TBL_ENTRIES - 1; i++) {
     if (buf_size > lBufferRangeTbl[i] &&
       buf_size < (lBufferRangeTbl[i+1]-1)) {
       l_hw_buf_height = pow(2, i+2);
       break;
     }
  }

  if (l_hw_buf_height == 0) {
     JPEG_HW_PR_ERR("%s %d: Error, work buf not within valid range",
       __func__, __LINE__);
  }

  return l_hw_buf_height;
}

struct msm_jpeg_hw_cmds *jpege_lib_hw_we_bffr_cfg (jpege_cmd_input_cfg * pIn,
                                                jpege_cmd_scale_cfg *scale_cfg)
{
  uint32_t l_hw_buf_height = 0;
  uint32_t nRegVal = 0;
  int i=0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds = jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_we_buffer_cfg),
               hw_cmd_we_buffer_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  //get height of wr register based on work buf size
  l_hw_buf_height = jpege_lib_hw_lookup_we_buf_height(pIn->hw_buf_size);

  if (l_hw_buf_height == 0) {
     JPEG_HW_PR_ERR("%s %d: work buf range Error, hw_buf_size=%d",
       __func__, __LINE__, pIn->hw_buf_size );
     return NULL;
  }

  //PLN0_WR_BUFFER_SIZE
  nRegVal = ((l_hw_buf_height
             << JPEG_PLN0_WR_BUFFER_SIZE__HEIGHT_SHFT)
             |(pIn->hw_buf_size / l_hw_buf_height));
  p_hw_cmd->data = nRegVal;
  JPEG_HW_DBG("we_buffer_cfg PLN0_WR_BUFFER_SIZE = %d\n", p_hw_cmd->data);
  p_hw_cmd++;

  //PLN0_WR_STRIDE
  nRegVal = pIn->hw_buf_size / l_hw_buf_height;
  p_hw_cmd->data = nRegVal;
  JPEG_HW_DBG("we_buffer_cfg PLN0_WR_STRIDE = %d\n", p_hw_cmd->data);
  p_hw_cmd++;
  p_hw_cmd++;
  p_hw_cmd++;

  //PLN0_WR_HSTEP
  nRegVal = pIn->hw_buf_size / l_hw_buf_height;
  p_hw_cmd->data = nRegVal;
  JPEG_HW_DBG("we_buffer_cfg PLN0_WR_HSTEP = %d\n", p_hw_cmd->data);
  p_hw_cmd++;

  //PLN0_WR_VSTEP
  nRegVal = l_hw_buf_height;
  p_hw_cmd->data = nRegVal;
  JPEG_HW_DBG("we_buffer_cfg PLN0_WR_VSTEP = %d\n", p_hw_cmd->data);
  p_hw_cmd++;

  p_hw_cmd++;


  return p_hw_cmds;

}

struct msm_jpeg_hw_cmd hw_cmd_we_cfg[] = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_WE_CFG_ADDR,
   JPEG_WE_CFG_BMSK, {0x15011030}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_WE_PATH_CFG_ADDR,
   JPEG_WE_PATH_CFG_BMSK, {0x24}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEG_WE_QOS_CFG_ADDR,
   JPEG_WE_QOS_CFG_BMSK, {JPEG_WE_QOS_CFG_ADDR_VALUE}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_we_cfg ()
{
  uint32_t nRegVal = 0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_we_cfg),
               hw_cmd_we_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  p_hw_cmd++;
  p_hw_cmd++;
  p_hw_cmd++;

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_read_quant_tables[] = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_DMI_ADDR_ADDR,
   JPEG_DMI_ADDR_BMSK, {JPEG_DMI_ADDR_START}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_DMI_CFG_ADDR,
   JPEG_DMI_CFG_BMSK, {JPEG_DMI_CFG_QUANTIZATION}},
  /*
   * Y quantization     : JPEGE_NUM_QUANTIZATION_ENTRIES = 64
   */
  /*
   * Chroma quantization: JPEGE_NUM_QUANTIZATION_ENTRIES = 64
   */
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_DMI_CFG_ADDR,
   JPEG_DMI_CFG_BMSK, {JPEG_DMI_CFG_DISABLE}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_read_quant_tables (void)
{
  uint8_t status = 0;
  uint8_t nIndex = 0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof
               (hw_cmd_read_quant_tables) +
               sizeof (struct
                 msm_jpeg_hw_cmd) *
               GEMINI_NUM_QUANTIZATION_ENTRIES
               * 2, NULL);
  if (!p_hw_cmds) {
    return NULL;
  }
  memcpy (&p_hw_cmds->hw_cmd[0], hw_cmd_read_quant_tables,
    sizeof (hw_cmd_read_quant_tables));
  p_hw_cmd = &p_hw_cmds->hw_cmd[2];

  /*
   * load Y quantization values to lower portion of quantization LUT
   */
  for (nIndex = 0; nIndex < GEMINI_NUM_QUANTIZATION_ENTRIES; nIndex++) {
    p_hw_cmd->type = MSM_JPEG_HW_CMD_TYPE_READ;
    p_hw_cmd->n = 1;
    p_hw_cmd->offset = JPEG_DMI_DATA_ADDR;
    p_hw_cmd->mask = JPEG_DMI_DATA_BMSK;
    p_hw_cmd++;

  }

  /*
   * load Chroma quantization values to upper portion of quantization LUT
   */
  for (nIndex = 0; nIndex < GEMINI_NUM_QUANTIZATION_ENTRIES; nIndex++) {
    p_hw_cmd->type = MSM_JPEG_HW_CMD_TYPE_READ;
    p_hw_cmd->n = 1;
    p_hw_cmd->offset = JPEG_DMI_DATA_ADDR;
    p_hw_cmd->mask = JPEG_DMI_DATA_BMSK;
    p_hw_cmd++;

  }

  /*
   * disable DMI capabilities
   */
  p_hw_cmd->type = hw_cmd_read_quant_tables[2].type;
  p_hw_cmd->n = hw_cmd_read_quant_tables[2].n;
  p_hw_cmd->offset = hw_cmd_read_quant_tables[2].offset;
  p_hw_cmd->mask = hw_cmd_read_quant_tables[2].mask;
  p_hw_cmd->data = hw_cmd_read_quant_tables[2].data;

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_set_quant_tables[] = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_DMI_ADDR_ADDR,
   JPEG_DMI_ADDR_BMSK, {JPEG_DMI_ADDR_START}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_DMI_CFG_ADDR,
   JPEG_DMI_CFG_BMSK, {JPEG_DMI_CFG_QUANTIZATION}},
  /*
   * Y quantization     : JPEGE_NUM_QUANTIZATION_ENTRIES = 64
   */
  /*
   * Chroma quantization: JPEGE_NUM_QUANTIZATION_ENTRIES = 64
   */
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEG_DMI_CFG_ADDR,
   0xffffffff, {JPEG_DMI_CFG_DISABLE}},
};

struct msm_jpeg_hw_cmds *jpege_lib_hw_set_quant_tables (jpege_quantTable *
                 pY,
                 jpege_quantTable *
                 pChroma)
{
  uint8_t status = 0;
  uint8_t nTmpValue = 0;
  uint8_t nIndex = 0;
  uint32_t nInversedQ = 0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpege_lib_hw_cmd_malloc_and_copy (sizeof
               (hw_cmd_set_quant_tables) +
               sizeof (struct
                 msm_jpeg_hw_cmd) *
               GEMINI_NUM_QUANTIZATION_ENTRIES
               * 2, NULL);
  if (!p_hw_cmds) {
    return NULL;
  }
  memcpy (&p_hw_cmds->hw_cmd[0], hw_cmd_set_quant_tables,
    sizeof (hw_cmd_set_quant_tables));
  p_hw_cmd = &p_hw_cmds->hw_cmd[2];

  /*
   * load Y quantization values to lower portion of quantization LUT
   */
  for (nIndex = 0; nIndex < GEMINI_NUM_QUANTIZATION_ENTRIES; nIndex++) {
    nTmpValue = pY->qtbl[nIndex];

    /*
     * since 1/Q value is an unsigned Q16 value in hardware
     */
    /*
     * set 1/Q to max for the case of zero (ref SWI section 2.2.7)
     */
    if (0 == nTmpValue || 1 == nTmpValue) {
      nInversedQ = JPEG_DMI_DATA_MAX;
    } else {
      /*
       * notes: no floating point can be used
       */
      /*
       * notes: this equation is from hw team for 1/Q
       */
      /*
       * notes: nInversedQ = (uint32_t) ((1 << 16) * (1.0/nTmpValue));
       */
      nInversedQ = (uint32_t) (0x10000 / nTmpValue);
    }
    p_hw_cmd->type = MSM_JPEG_HW_CMD_TYPE_WRITE;
    p_hw_cmd->n = 1;
    p_hw_cmd->offset = JPEG_DMI_DATA_ADDR;
    p_hw_cmd->mask = JPEG_DMI_DATA_BMSK;
    p_hw_cmd->data = nInversedQ;
    p_hw_cmd++;

  }

  /*
   * load Chroma quantization values to upper portion of quantization LUT
   */
  for (nIndex = 0; nIndex < GEMINI_NUM_QUANTIZATION_ENTRIES; nIndex++) {
    nTmpValue = pChroma->qtbl[nIndex];

    /*
     * since 1/Q value is an unsigned Q16 value in hardware
     */
    /*
     * set 1/Q to max for the case of zero (ref SWI section 2.2.7)
     */
    if (0 == nTmpValue || 1 == nTmpValue) {
      nInversedQ = JPEG_DMI_DATA_MAX;
    } else {
      /*
       * notes: no floating point can be used
       */
      /*
       * notes: this equation is from hw team for 1/Q
       */
      /*
       * notes: nInversedQ = (uint32_t) ((1 << 16) * (1.0/nTmpValue));
       */
      nInversedQ = (uint32_t) (0x10000 / nTmpValue);
    }
    p_hw_cmd->type = MSM_JPEG_HW_CMD_TYPE_WRITE;
    p_hw_cmd->n = 1;
    p_hw_cmd->offset = JPEG_DMI_DATA_ADDR;
    p_hw_cmd->mask = JPEG_DMI_DATA_BMSK;
    p_hw_cmd->data = nInversedQ;
    p_hw_cmd++;

  }

  /*
   * disable DMI capabilities
   */
  p_hw_cmd->type = hw_cmd_set_quant_tables[2].type;
  p_hw_cmd->n = hw_cmd_set_quant_tables[2].n;
  p_hw_cmd->offset = hw_cmd_set_quant_tables[2].offset;
  p_hw_cmd->mask = hw_cmd_set_quant_tables[2].mask;
  p_hw_cmd->data = hw_cmd_set_quant_tables[2].data;

  return p_hw_cmds;
}
