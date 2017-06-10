/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SCE32_H__
#define __SCE32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "sce32_reg.h"
#include "chromatix.h"


/** ISP_Skin_enhan_line
 *    @point0: origin point
 *    @shift_cr: cb axis coefficient
 *    @shift_cb: cr axis coefficient
 *
 * parametric and equation of the SCE triangels common point
 * interpolation line:
 * cr = cr0 + shift_cr * t
 * cb = cb0 + shift_cb * t
 **/
typedef struct ISP_Skin_enhan_line {
  cr_cb_point point0;
  double shift_cr;
  double shift_cb;
} ISP_Skin_enhan_line;

/** ISP_Skin_enhan_line
 *    @interpolation_line: line along which the common vertex is shifted
 *    @shift_cr: cb axis boundary
 *    @shift_cb: cr axis boundary
 *
 * Range in which the common triangles vertex can be shifted defined by a line
 * and boundaries
 **/
typedef struct ISP_Skin_enhan_range {
  ISP_Skin_enhan_line interpolation_line;
  double pos_step;
  double neg_step;
}ISP_Skin_enhan_range;

typedef struct ISP_Skin_enhan_ConfigCmdType {
  ISP_Skin_Enhan_Coordinates crcoord;
  ISP_Skin_Enhan_Coordinates cbcoord;
  ISP_Skin_Enhan_Coeff       crcoeff;
  ISP_Skin_Enhan_Coeff       cbcoeff;
  ISP_Skin_Enhan_Offset      croffset;
  ISP_Skin_Enhan_Offset      cboffset;
}__attribute__((packed, aligned(4))) ISP_Skin_enhan_ConfigCmdType;

typedef struct {
  ISP_Skin_enhan_ConfigCmdType sce_cmd;
  ISP_Skin_enhan_ConfigCmdType applied_sce_cmd;
  sce_cr_cb_triangle_set origin_triangles_A;
  sce_cr_cb_triangle_set origin_triangles_D65;
  sce_cr_cb_triangle_set origin_triangles_TL84;
  sce_cr_cb_triangle_set destination_triangles_A;
  sce_cr_cb_triangle_set destination_triangles_D65;
  sce_cr_cb_triangle_set destination_triangles_TL84;
  sce_cr_cb_triangle_set *orig;
  sce_cr_cb_triangle_set *dest;
  sce_shift_vector interp_vector;
  ISP_Skin_enhan_range interp_range;
  double sce_adjust_factor;
  cct_trigger_info trigger_info;
  int fd;
  uint8_t sce_enable;
  uint8_t sce_trigger_enable;
  uint8_t sce_update;
  awb_cct_type prev_cct_type;
  double prev_sce_adj;
  float prev_aec_ratio;
  uint8_t hw_update_pending;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
}isp_sce_mod_t;

#endif //__SCE32_H__
