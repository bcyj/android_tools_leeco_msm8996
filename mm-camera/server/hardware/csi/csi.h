/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CSI_UTIL_H__
#define __CSI_UTIL_H__

#include "csi_interface.h"
#include "camera_dbg.h"

typedef struct {
  uint32_t fd;
  uint32_t csid_version;
  sensor_csi_params_t *csi_params;
  struct msm_camera_csi_params *curr_csic_params;
  struct msm_camera_csi2_params *curr_csi2_params;
} csi_t;

#define CSI_MAX_CLIENT_NUM 4

typedef struct {
  uint8_t client_idx;
  uint32_t handle;
  mctl_ops_t *ops;
  csi_t csi_ctrl_obj;
} csi_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t csi_handle_cnt;
  csi_client_t client[CSI_MAX_CLIENT_NUM];
} csi_comp_root_t;

int csi_util_process_init(csi_t *csi_obj);
int csi_util_set_cfg(csi_t *csi_obj, csi_set_data_t *csi_set);

#endif
