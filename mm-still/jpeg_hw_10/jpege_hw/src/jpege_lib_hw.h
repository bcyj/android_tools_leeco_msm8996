/*******************************************************************************
*                                                                         .
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential                      .
*                                                                         .
*******************************************************************************/

#ifndef JPEGE_LIB_HW_H
#define JPEGE_LIB_HW_H

#include <unistd.h>
#include <media/msm_jpeg.h>
#include "jpege_hw_core.h"

#define JPEG_8974_V1 0x10000000
#define JPEG_8974_V2 0x10010000

#define IS_8974_V2(v) ((v) == JPEG_8974_V2)

#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)
#define FLOOR16(X) ((X) & 0xFFF0)
#define FLOOR8(X)  ((X) & 0xFFF8)
#define FLOAT_TO_Q(exp, f) \
  ((int32_t)(((f)*(1<<(exp))) + (((f)<0) ? -0.5 : 0.5)))

void jpege_lib_hw_get_version(struct msm_jpeg_hw_cmd *p_hw_cmd);

struct msm_jpeg_hw_cmds *jpege_lib_hw_stop();

struct msm_jpeg_hw_cmds *jpege_lib_hw_start();

struct msm_jpeg_hw_cmds *jpege_lib_hw_fe_cfg(
  jpege_cmd_input_cfg *p_input_cfg, uint8_t mcus_per_blk);

struct msm_jpeg_hw_cmds *jpege_lib_hw_fe_buffer_cfg(
   jpege_cmd_input_cfg *p_input_cfg, jpege_cmd_scale_cfg *scale_cfg);

struct msm_jpeg_hw_cmds *jpege_lib_hw_encode_cfg(
  jpege_cmd_input_cfg * pIn,
  jpege_cmd_scale_cfg *scale_cfg);

struct msm_jpeg_hw_cmds *jpege_lib_hw_we_cfg();

uint32_t jpege_lib_hw_lookup_we_buf_height(uint32_t buf_size);

struct msm_jpeg_hw_cmds *jpege_lib_hw_we_bffr_cfg(
  jpege_cmd_input_cfg * pIn,
  jpege_cmd_scale_cfg *scale_cfg);

struct msm_jpeg_hw_cmds *jpege_lib_hw_default_scale_cfg();

struct msm_jpeg_hw_cmds *jpege_lib_hw_scale_cfg(
  jpege_cmd_scale_cfg *scale_cfg,
  jpege_cmd_input_cfg *pIn);

struct msm_jpeg_hw_cmds *jpege_lib_hw_scale_reg_cfg(
  jpege_cmd_scale_cfg *scale_cfg, jpege_cmd_input_cfg *pIn);

struct msm_jpeg_hw_cmds *jpege_cmd_core_cfg(jpege_cmd_scale_cfg *p_scale_cfg);

struct msm_jpeg_hw_cmds *jpege_lib_hw_encode_state();

struct msm_jpeg_hw_cmds *jpege_lib_hw_crop_cfg(jpege_cmd_scale_cfg *scale_cfg,
  jpege_cmd_input_cfg *p_input_cfg);

struct msm_jpeg_hw_cmds *jpege_lib_hw_restart_marker_set(
  uint32_t restartInterval);

struct msm_jpeg_hw_cmds *jpege_lib_hw_read_quant_tables(void);
struct msm_jpeg_hw_cmds *jpege_lib_hw_set_quant_tables(
  jpege_quantTable *pY, jpege_quantTable *pChroma);

#endif /* JPEGE_LIB_HW_H */
