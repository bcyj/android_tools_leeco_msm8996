/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __COLOR_XFORM44_H__
#define __COLOR_XFORM44_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "colorxform44_reg.h"
#include "chromatix.h"

typedef enum isp_color_xform_path{
  ISP_COLOR_XFORM_ENC,
  ISP_COLOR_XFORM_VIEWFINDER,
  ISP_COLOR_XFORM_MAX = 2
} isp_color_xform_path_t;

typedef struct {
  /* ISP Related*/
  int fd; //handle for module in session
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params: index 0 for encoder and index 1 for view finder*/
  ISP_colorXformCfgCmdType reg_cmd[ISP_COLOR_XFORM_MAX];

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag , master*/
  uint8_t skip_trigger; /* skip trigger update feature flag , secondary*/
  uint8_t enable;         /* enable flag from PIX */
  color_xform_type_t xform;
} isp_color_xform_mod_t;

#endif //__COLOR_XFORM44_H__
