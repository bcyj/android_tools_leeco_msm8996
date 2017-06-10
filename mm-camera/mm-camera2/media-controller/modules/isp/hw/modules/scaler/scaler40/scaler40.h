/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SCALER40_H__
#define __SCALER40_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "scaler40_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "chromatix.h"

typedef struct {
  ISP_ScaleCfgCmdType reg_cmd;
  float scaling_factor;
  uint8_t hw_update_pending;
  uint8_t is_right_stripe_config;
  uint8_t is_used; /* is this entry used or not */
}isp_scaler_entry_t;

typedef struct {
  int fd;
  isp_scaler_entry_t scalers[ISP_PIX_PATH_MAX];
  uint8_t trigger_enable; /* enable trigger update feature flag */
  uint8_t enable;         /* enable flag from PIX */
  uint32_t applied_crop_factor;   /* Q12 = 1x */
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  uint32_t max_scaler_out_width;
  uint32_t max_scaler_out_height;
}isp_scaler_mod_t;

#endif /* __SCALER40_H__ */
