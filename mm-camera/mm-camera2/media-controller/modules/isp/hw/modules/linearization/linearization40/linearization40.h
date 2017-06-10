/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __LINEARIZATION40_H__
#define __LINEARIZATION40_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "isp_pipeline40.h"
#include "linearization40_reg.h"
#include "chromatix.h"
#include "chromatix_common.h"


#define ISP32_LINEARIZATON_TABLE_LENGTH 36

#define CALC_SLOPE(x1,x2,y1,y2) \
  ((float)(y2 - y1) /(float)(x2 -x1))

typedef struct ISP_LinearizationRightCfgParams {
  /* Knee points for R channel Right */
  ISP_PointSlopeData pointSlopeR;

  /* Knee points for Gr channel Right */
  ISP_PointSlopeData pointSlopeGb;

  /* Knee points for B channel Right */
  ISP_PointSlopeData pointSlopeB;

  /* Knee points for Gb channel Right */
  ISP_PointSlopeData pointSlopeGr;
}__attribute__((packed, aligned(4))) ISP_LinearizationRightCfgParams;

typedef struct VFE_LinearizationCfgTable {
  uint32_t Lut[ISP32_LINEARIZATON_TABLE_LENGTH];
}ISP_LinearizationCfgTable;

typedef struct ISP_LinearizationCmdType {
  uint32_t dmi_set[2];
  ISP_LinearizationCfgTable CfgTbl;
  uint32_t dmi_reset[2];
  ISP_LinearizationCfgParams CfgParams;
}ISP_LinearizationCmdType;

typedef struct ISP_LinearizationRightCmdType {
  ISP_LinearizationRightCfgParams CfgParams;
  ISP_LinearizationCfgTable CfgTbl;
}ISP_LinearizationRightCmdType;

typedef struct
{
  unsigned short r_lut_p[8]; // 12uQ0
  unsigned short gr_lut_p[8]; // 12uQ0
  unsigned short gb_lut_p[8]; // 12uQ0
  unsigned short b_lut_p[8]; // 12uQ0

  unsigned short r_lut_base[9]; // 12uQ0
  unsigned short gr_lut_base[9]; // 12uQ0
  unsigned short gb_lut_base[9]; // 12uQ0
  unsigned short b_lut_base[9]; // 12uQ0

  unsigned int r_lut_delta[9]; // 18uQ9
  unsigned int gr_lut_delta[9]; // 18uQ9
  unsigned int gb_lut_delta[9]; // 18uQ9
  unsigned int b_lut_delta[9]; // 18uQ9
}ISP_LinearizationLut;

typedef enum {
  LINEAR_AEC_BRIGHT = 0,
  LINEAR_AEC_BRIGHT_NORMAL,
  LINEAR_AEC_NORMAL,
  LINEAR_AEC_NORMAL_LOW,
  LINEAR_AEC_LOW,
  LINEAR_AEC_LUX_MAX,
} isp_linear_lux_t;

typedef struct {
  ISP_LinearizationCmdType  linear_cmd;
  cct_trigger_info trigger_info;
  ISP_LinearizationLut linear_lut;
  ISP_LinearizationLut applied_linear_lut;
  int fd;
  uint8_t linear_trigger_enable;
  uint8_t linear_enable;
  awb_cct_type prev_cct_type;
  isp_linear_lux_t prev_lux;
  float blk_inc_comp;
  uint8_t hw_update_pending;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
}isp_linear_mod_t;


#endif //__LINEARIZATION40_H__
