/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __ASF32_H__
#define __ASF32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "asf32_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "chromatix.h"

typedef struct {
  chromatix_asf_5_5_type data;
  asf_setting_type settings;
}asf_params_t;


typedef struct {
  /* ISP Related*/
  int fd; //handle for module in session
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params*/
  ISP_AdaptiveFilterConfigCmdType RegCmd;
  ISP_AdaptiveFilterConfigCmdType applied_RegCmd;
  asf_params_t param;
  isp_sharpness_info_t in_sharpness_info;  // ryan - PIX will set this
  isp_sharpness_info_t out_sharpness_info;
  trigger_ratio_t aec_ratio;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag */
  uint8_t skip_trigger;
  uint8_t enable;         /* enable flag from PIX */
  uint32_t sp_effect_HW_enable;
  /* Driven by UI, BST/ASD soft_focus_degree and VFE downscale factor */
  uint32_t sharpness_update;

} isp_asf_mod_t;

typedef enum ISP_ASF_ModeType {
  ASF_MODE_NONE,
  ASF_MODE_SINGLE_FILTER,
  ASF_MODE_DUAL_FILTER,
  ASF_MODE_SMART_FILTER,
  ASF_MODE_ENUM = ASF_MODE_SMART_FILTER,
} ISP_ASF_ModeType;

#endif //__ASF32_H__

