/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __CAMERA_PLUGIN_H__
#define __CAMERA_PLUGIN_H__

/*===========================================================================
 *                         INCLUDE FILES
 *===========================================================================*/
#include "camera.h"
#include "camera_plugin_intf.h"
#include "vfe_modules.h"

typedef enum {
  PLGN_SUCCESS = 0,
  PLGN_ERROR_GENERAL,
  PLGN_ERROR_INVALID_OPERATION,
  PLGN_ERROR_NO_MEMORY,
  PLGN_ERROR_NOT_SUPPORTED,
}plugin_status_t;

typedef struct {
  fov_module_t reg_update_fov;
  scaler_module_t reg_update_scaler;
  demosaic_module_t reg_update_demosaic;
  chroma_enhan_module_t reg_update_ce;
  demux_module_t reg_update_demux;
  chroma_supp_module_t reg_update_chrom_supp;
  clamp_module_t reg_update_clamp;
  bcc_module_t reg_update_bcc;
  bpc_module_t reg_update_bpc;
  mce_module_t reg_update_mce;
  wb_module_t reg_update_wb;
  color_correct_module_t reg_update_cc;
  abf_module_t reg_update_abf;
  clf_module_t reg_update_clf;
  gamma_module_t reg_update_gamma;
  frame_skip_module_t reg_update_fs;
  chroma_ss_module_t reg_update_ss;
  asf_module_t reg_update_asf;
  rolloff_module_t reg_update_roll_off;
  linear_module_t reg_update_linear;
  sce_module_t reg_update_sce;
  la_module_t reg_update_la;
  awb_stats_module_t reg_update_awb;
  camera_plugin_vfe_reg_update_data_t entries[CAMERA_PLUGIN_VFE_REG_UPDATE_MAX_ENTRIES];
} camera_plugin_client_vfe_reg_update_data_t;

typedef struct {
  pthread_mutex_t client_mutex;
  uint32_t client_handle;
  uint32_t feature_mask;
  void *plugin_root;
  camera_plugin_client_ops_t client_ops;
  camera_plugin_mctl_process_ops_t mctl_ops;
  camera_plugin_client_vfe_reg_update_data_t vfe_reg_update_data;
  uint8_t used;
} camera_plugin_client_t;

#define CAMERA_PLUGIN_CLIENT_MAX 8

typedef struct {
  pthread_mutex_t mutex;
  uint8_t support_simultaneous_camera;
  uint8_t diff_clk_for_rdi_pix;
  uint32_t open_cnt;
  uint32_t handle_cnt;
  camera_plugin_client_t client[CAMERA_PLUGIN_CLIENT_MAX];
  camera_plugin_ops_t my_ops;
} camera_plugin_root_t;

#endif /* __CAMERA_PLUGIN_H__ */
