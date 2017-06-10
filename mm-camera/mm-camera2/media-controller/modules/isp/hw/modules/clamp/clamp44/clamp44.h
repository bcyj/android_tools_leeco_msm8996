/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CLAMP44_H__
#define __CLAMP44_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "clamp44_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "chromatix.h"

typedef enum {
  ISP_CLAMP_ENCODER = 0,
  ISP_CLAMP_VIEWFINDER = 1,
  ISP_CLAMP_MAX_NUM = 2
} isp_clamp_type;

typedef struct {
  ISP_OutputClampConfigCmdType reg_cmd;
  uint8_t hw_update_pending;
}isp_clamp_entry_t;

typedef struct {
  /* ISP Related*/
  int fd;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;

  /* Module Params*/
  isp_clamp_entry_t clamp[ISP_CLAMP_MAX_NUM];

  /* Module Control */
  uint8_t enable;         /* enable flag from PIX */
}isp_clamp_mod_t;

#endif// __CLAMP44_H__
