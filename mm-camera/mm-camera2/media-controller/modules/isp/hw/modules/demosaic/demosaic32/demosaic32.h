/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC32_H__
#define __DEMOSAIC32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "demosaic32_reg.h"
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
  ISP_Demosaic32ConfigCmdType reg_cmd;
  ISP_Demosaic32ConfigCmdType applied_RegCmd;
  ISP_Demosaic32MixConfigCmdType mix_reg_cmd;

  trigger_ratio_t ratio;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag , master*/
  uint8_t skip_trigger; /* skip trigger update feature flag , secondary*/
  uint8_t enable;         /* enable flag from PIX */
  int classifier_cfg_done;
} isp_demosaic_mod_t;
#endif /* __DEMOSAIC32_H__ */
