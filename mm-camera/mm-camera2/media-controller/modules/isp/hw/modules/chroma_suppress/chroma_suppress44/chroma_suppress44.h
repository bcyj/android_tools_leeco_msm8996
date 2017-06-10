/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CHROMA_SUPPRESS44_H__
#define __CHROMA_SUPPRESS44_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "chroma_suppress44_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "chromatix.h"

#define Clamp(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))

typedef struct {
  uint32_t hw_mask;
  ISP_ChromaSuppress_Mix1_ConfigCmdType reg_cmd;
}isp_chroma_suppress_mix_1_cmd_t;

typedef struct {
  uint32_t hw_mask;
  ISP_ChromaSuppress_Mix2_ConfigCmdType reg_cmd;
}isp_chroma_suppress_mix_2_cmd_t;

typedef struct {
  /* ISP Related*/
  int fd; //handle for module in session
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params*/
  ISP_ChromaSuppress_ConfigCmdType reg_cmd;
  isp_chroma_suppress_mix_1_cmd_t reg_mix_cmd_1;
  isp_chroma_suppress_mix_2_cmd_t reg_mix_cmd_2;
  trigger_ratio_t aec_ratio;
  cs_luma_threshold_type thresholds;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag from PIX*/
  uint8_t skip_trigger;
  uint8_t enable;         /* enable flag from PIX */
  uint8_t bestshot_enable;         /* enable flag from PIX */
} isp_chroma_suppress_mod_t;
#endif //__CHROMA_SUPPRESS44_H__
