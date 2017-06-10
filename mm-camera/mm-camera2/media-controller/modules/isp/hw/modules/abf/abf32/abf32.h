/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC_ABF32_H__
#define __DEMOSAIC_ABF32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "abf32_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "../common/abf_common.h"
#include "chromatix.h"

typedef struct {
  float table_pos[16];
  float table_neg[8];
} abf2_table_t;

typedef struct {
  abf2_table_t r_table;
  abf2_table_t g_table;
  abf2_table_t b_table;
  chromatix_adaptive_bayer_filter_data_type2 data;
  int8_t table_updated;
} abf2_parms_t;

typedef struct {
  /* ISP Related*/
  int fd; //handle for module in session
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
  cam_streaming_mode_t old_streaming_mode;

  /* Module Params*/
  ISP_DemosaicABF_CmdType RegCmd;
  ISP_DemosaicABF_CmdType applied_RegCmd;
  trigger_ratio_t aec_ratio;
  abf2_parms_t abf2_parms;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag from PIX*/
  uint8_t skip_trigger; /* skip the trigger update */
  uint8_t enable;         /* enable flag from PIX */
} isp_abf_mod_t;

#endif //__DEMOSAIC_ABF32_H__
