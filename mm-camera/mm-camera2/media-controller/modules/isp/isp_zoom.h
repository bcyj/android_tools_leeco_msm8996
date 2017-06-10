/*============================================================================
Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ISP_ZOOM_H__
#define __ISP_ZOOM_H__

#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"
#include "isp_ops.h"
#include "isp_hw.h"
#include "q3a_stats_hw.h"

#define MAX_ZOOM_STEPS      182
#define ZOOM_TABLE_MAX_DEF  182

typedef struct {
  uint32_t *ext_zoom_table;
  uint32_t *zoom_table;
  uint32_t zoom_table_bump[ZOOM_TABLE_MAX_DEF];
  int zoom_table_size;
  uint32_t resize_factor;
  uint32_t isp_version;
  int maximum_value;
  int minimum_value;
  uint32_t zoom_step_size;
  int32_t step_value;
} isp_zoom_data_t;

typedef struct {
  void *pzoom;
  int num_fovs;
  uint32_t session_id;
  isp_zoom_scaling_param_t zoom_scaling;
  int32_t zoom_val;
} isp_zoom_session_t;

typedef struct {
  isp_zoom_data_t zoom_data;
  isp_zoom_session_t sessions[ISP_MAX_SESSIONS];
} isp_zoom_t;

isp_zoom_t *isp_zoom_create(uint32_t isp_version);
void isp_zoom_destroy(isp_zoom_t *zoom_root);
int isp_zoom_get_ratio_table(isp_zoom_t *pzoom, int32_t *num, int *buf);
uint32_t isp_zoom_get_max_zoom(isp_zoom_t *pzoom);
uint32_t isp_zoom_get_min_zoom(isp_zoom_t *pzoom);
isp_zoom_session_t *isp_zoom_open_session(isp_zoom_t *pzoom,
  uint32_t session_id);
void isp_zoom_close_session(isp_zoom_session_t *session);
int isp_zoom_get_crop_factor(isp_zoom_session_t *session, int zoom_val,
  uint32_t *crop_factor);
int isp_set_zoom_scaling_parm(isp_zoom_session_t *session,
  isp_hw_zoom_param_t *crop_info);

int isp_zoom_get_scaling_param(isp_zoom_session_t *session,
  isp_zoom_scaling_param_t *scaling_param);

uint32_t isp_zoom_calc_dim(isp_zoom_session_t *session, uint32_t dim,
  uint32_t crop_factor);
#endif /* __ISP_ZOOM_H__ */
