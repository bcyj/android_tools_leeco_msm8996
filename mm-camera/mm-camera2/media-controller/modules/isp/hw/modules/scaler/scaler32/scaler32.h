/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SCALER32_H__
#define __SCALER32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "scaler32_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "chromatix.h"

#define PATH_1 0
#define PATH_2 1

typedef struct {
  ISP_Main_Scaler_ConfigCmdType main_scaler_cmd;
  ISP_Output_YScaleCfgCmdType y_scaler_cmd;
  ISP_Output_CbCrScaleCfgCmdType cbcr_scaler_cmd;
  int fd;
  float scaling_factor;
  uint8_t trigger_update;
  uint8_t enable;
  uint8_t trigger_enable;
  uint8_t main_scaler_hw_update_pending;
  uint8_t Y_scaler_hw_update_pending;
  uint8_t CbCr_scaler_hw_update_pending;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
}isp_scaler_mod_t;

#endif //__SCALER32_H__
