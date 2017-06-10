/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __COLOR_CORRECT32_H__
#define __COLOR_CORRECT32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "colorcorrect32_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "chromatix.h"
#include "chromatix_common.h"

#define CC_COEFF(x, q) (FLOAT_TO_Q((q), (x)))

typedef struct
{
  int32_t  c0;
  int32_t  c1;
  int32_t  c2;
  int32_t  c3;
  int32_t  c4;
  int32_t  c5;
  int32_t  c6;
  int32_t  c7;
  int32_t  c8;
  int32_t  k0;
  int32_t  k1;
  int32_t  k2;

  uint8_t  q_factor;  // QFactor
} color_correct_type;

typedef struct {
  color_correct_type  chromatix_STROBE_color_correction;
  color_correct_type  chromatix_TL84_color_correction;
  color_correct_type  chromatix_yhi_ylo_color_correction;
  color_correct_type  chromatix_outdoor_color_correction;
  color_correct_type  chromatix_LED_color_correction_VF;
  color_correct_type  chromatix_D65_color_correction_VF;
  color_correct_type  chromatix_A_color_correction_VF;
} color_correct_tab_t;

typedef struct {
  /* ISP Related*/
  int fd; //handle for module in session
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params*/
  ISP_ColorCorrectionCfgCmdType RegCmd;
  ISP_ColorCorrectionCfgCmdType applied_RegCmd;
  trigger_ratio_t aec_ratio;
  uint32_t color_temp;
  color_correct_tab_t table;
  float effects_matrix[3][3];
  float dig_gain;
  color_correct_type final_table;

  /* Module Control */
  uint8_t hw_update_pending;
  cam_flash_mode_t prev_flash_mode;
  uint8_t trigger_enable; /* enable trigger update feature flag from PIX*/
  uint8_t skip_trigger;
  uint8_t enable;         /* enable flag from PIX */
  uint8_t bestshot_enable;         /* enable flag from PIX */

} isp_color_correct_mod_t;

#endif //__COLOR_CORRECT32_H__
