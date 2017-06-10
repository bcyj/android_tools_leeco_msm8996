/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CHROMA_ENHAN32_H__
#define __CHROMA_ENHAN32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "chroma_enhan32_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "chromatix.h"

typedef struct {
  /* ISP Related*/
  int fd; //handle for module in session
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params*/
  ISP_Chroma_Enhance_CfgCmdType RegCmd;
  ISP_Chroma_Enhance_CfgCmdType applied_RegCmd;
  chromatix_color_conversion_type cv_data;
  chromatix_color_conversion_type *p_cv;
  float effects_matrix[2][2];
  trigger_ratio_t aec_ratio;
  uint32_t color_temp;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag from PIX*/
  uint8_t skip_trigger;
  uint8_t enable;         /* enable flag from PIX */

} isp_color_conversion_mod_t;

#endif// __CHROMA_ENHAN32_H__
