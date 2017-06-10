/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __WB44_H__
#define __WB44_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "wb44_reg.h"
#include "chromatix.h"

#define WB_GAIN(x) FLOAT_TO_Q(7, (x))


#define WB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1->r_gain, g2.r_gain) && \
  F_EQUAL(g1->g_gain, g2.g_gain) && \
  F_EQUAL(g1->b_gain, g2.b_gain))

#define WB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1->r_gain, 0.0) || \
  F_EQUAL(g1->g_gain, 0.0) || \
  F_EQUAL(g1->b_gain, 0.0))

typedef struct  {
  ISP_WhiteBalanceConfigCmdType    ISP_WhiteBalanceCfgCmd;
  ISP_WhiteBalanceRightConfigCmdType ISP_WhiteBalanceRightCfgCmd;
  int8_t enable;
  int fd;
  chromatix_manual_white_balance_type awb_gain;
  float dig_gain;
  int trigger_enable;
  uint8_t hw_update_pending;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
} isp_wb_mod_t;

#endif //__WB44_H__
