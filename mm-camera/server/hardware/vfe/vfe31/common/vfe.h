/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __VFE_H__
#define __VFE_H__

#include "vfe_interface.h"
#include "vfe_util_common.h"
#include "vfe_tgtcommon.h"
#include "vfe_module_ops.h"
#include "fovcrop.h"
#include "scaler.h"
#include "demosaic.h"
#include "chroma_enhan.h"
#include "chroma_suppress.h"
#include "chroma_subsample.h"
#include "bpc.h"
#include "mce.h"
#include "demux.h"
#include "wb.h"
#include "abf.h"
#include "colorcorrect.h"
#include "gamma.h"
#include "clamp.h"
#include "frame_skip.h"
#include "asf.h"
#include "rolloff.h"
#include "black_level.h"
#include "sce.h"
#include "luma_adaptation.h"
#include "aec_stats.h"
#include "af_stats.h"
#include "ihist_stats.h"
#include "rs_cs_stats.h"
#include "awb_stats.h"
#include "eztune_vfe_util.h"
#include "vfe_test_vector.h"
#include "bg_stats.h"
#include "bf_stats.h"
#include "bhist_stats.h"
#include "stats_parser.h"

typedef struct {
  aec_stats_t aec_stats;
  af_stats_t af_stats;
  ihist_stats_t ihist_stats;
  rs_stats_t rs_stats;
  cs_stats_t cs_stats;
  awb_stats_t awb_stats;
  bg_stats_t bg_stats;
  bf_stats_t bf_stats;
  bhist_stats_t bhist_stats;
  stats_parser_mod_t parser_mod;
  uint8_t use_bayer_stats;
}stats_mod_t;

typedef struct {
  fov_mod_t fov_mod;
  scaler_mod_t scaler_mod;
  demosaic_mod_t demosaic_mod;
  chroma_enhan_mod_t chroma_enhan_mod;
  demux_mod_t demux_mod;
  chroma_supp_mod_t chroma_supp_mod;
  clamp_mod_t clamp_mod;
  bpc_mod_t bpc_mod;
  mce_mod_t mce_mod;
  wb_mod_t wb_mod;
  color_correct_mod_t color_correct_mod;
  abf_mod_t abf_mod;
  gamma_mod_t gamma_mod;
  frame_skip_mod_t frame_skip_mod;
  chroma_ss_mod_t chroma_ss_mod;
  asf_mod_t asf_mod;
  rolloff_mod_t rolloff_mod;
  black_level_mod_t linear_mod;
  sce_mod_t sce_mod;
  la_mod_t la_mod;
  stats_mod_t stats;
} vfe_module_t;

typedef struct VFE_OperationConfigCmdType {
  /* 0 - preview, 1 - snapshot, 2 - video */
  uint32_t                    operationMode;
  /* 0 = stats are reported through individual messages.
   * 1 = stats are reported by a combo stats message. */
  uint32_t                    statisticsComposite;
  uint32_t                    hfrMode;

  VFE_CFGPacked               vfeCfg;
  VFE_ModuleCfgPacked         moduleCfg;
 VFE_RealignConfigCmdType     realignBufCfg;
  VFE_ChromaUpsampleCfgPacked chromaUpsampleCfg;
  VFE_StatsCfg                vfeStatsCfg;
} VFE_OperationConfigCmdType;

typedef struct {
  VFE_OperationConfigCmdType op_cmd;
}vfe_operation_cfg_t;

typedef struct {
  vfe_module_t vfe_module;
  vfe_params_t vfe_params;
  vfe_diagnostics_t vfe_diag;
  vfe_operation_cfg_t vfe_op_cfg;
  vfe_test_vector_t test_vector;
  int test_vector_get_dump;
  dis_ctrl_info_t dis_ctrl;
  current_output_info_t vfe_out;
  isp_stats_t vfe_stats_struct;
  mctl_ops_t *ops;
}vfe_ctrl_info_t;

#define VFE_MAX_OBJS 2
#define VFE_MAX_CLIENT_NUM 2

typedef struct {
  uint32_t obj_idx_mask;
  uint8_t client_idx;
  uint32_t handle;
  uint8_t my_comp_id;
  uint32_t vfe_version;
  mctl_ops_t *ops;
  vfe_ops_t vfe_ops;
  int sdev_fd;
} vfe_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t vfe_handle_cnt;
  vfe_client_t client[VFE_MAX_CLIENT_NUM];
  vfe_ctrl_info_t vfe_obj[VFE_MAX_OBJS];
} vfe_comp_root_t;

typedef struct {
  vfe_ctrl_info_t vfe_obj[VFE_MAX_OBJS];
} vfe_comp_objs_t;

vfe_ctrl_info_t *get_vfe_ctrl_info(uint32_t handle);
vfe_ctrl_info_t *vfe_get_obj(vfe_client_t *vfe_client);
vfe_status_t vfe_modules_init(vfe_module_t *vfe_module, vfe_params_t *params);
vfe_status_t vfe_stats_init(stats_mod_t *stats, vfe_params_t *params);
vfe_status_t vfe_params_init(vfe_ctrl_info_t *vfe_obj, vfe_params_t *params);
vfe_status_t vfe_modules_deinit(vfe_module_t *vfe_module, vfe_params_t *params);
vfe_status_t vfe_config_mode(vfe_op_mode_t mode, vfe_ctrl_info_t *vfe_ctrl_obj);
vfe_status_t vfe_trigger_update_for_aec(vfe_ctrl_info_t *vfe_ctrl_obj);
vfe_status_t vfe_trigger_update_for_awb(vfe_ctrl_info_t *vfe_ctrl_obj);
vfe_status_t vfe_trigger_update_for_ihist(vfe_ctrl_info_t *vfe_ctrl_obj,
  uint32_t *ihist_stats_buf);
vfe_status_t vfe_set_hue(vfe_ctrl_info_t* vfe_ctrl_obj, void *parm1);
vfe_status_t vfe_cmd_hw_reg_update(vfe_ctrl_info_t *vfe_ctrl_obj, int* p_update);
vfe_status_t vfe_query_shift_bits(vfe_ctrl_info_t *p_obj, vfe_get_parm_t type,
  void *parm);
vfe_status_t vfe_query_rs_cs_parm(vfe_ctrl_info_t *p_obj, vfe_get_parm_t type,
  void *parm);
vfe_status_t vfe_query_fov_crop(vfe_ctrl_info_t *p_obj, vfe_get_parm_t type,
  void *parm);
vfe_status_t vfe_set_effect(vfe_ctrl_info_t *vfe_ctrl_obj,
  vfe_effects_type_t type, vfe_effects_params_t *params);
vfe_status_t vfe_set_bestshot(vfe_ctrl_info_t *vfe_ctrl_obj,
  camera_bestshot_mode_type mode);
vfe_status_t vfe_config_autofocus(vfe_ctrl_info_t *vfe_ctrl_obj,
  vfe_stats_af_params_t *p_obj);
vfe_status_t vfe_set_manual_wb(vfe_ctrl_info_t *vfe_ctrl_obj);
vfe_status_t vfe_update_crop(vfe_ctrl_info_t *vfe_ctrl_obj,
  crop_window_info_t *fov_params);
vfe_status_t vfe_calc_pixel_crop_info(vfe_ctrl_info_t *p_obj, void *parm);
vfe_status_t vfe_calc_pixel_skip_info(vfe_ctrl_info_t *p_obj, void *parm);
vfe_status_t vfe_set_pixel_crop_info(vfe_ctrl_info_t *p_obj, void *parm);
vfe_status_t vfe_pp_info(vfe_ctrl_info_t *p_obj, vfe_pp_params_t *pp_info);
vfe_status_t vfe_set_mce(vfe_ctrl_info_t *vfe_ctrl_obj, uint32_t enable);
vfe_status_t vfe_set_sce(vfe_ctrl_info_t *vfe_ctrl_obj, int32_t sce_factor);
vfe_status_t vfe_set_output_info(vfe_ctrl_info_t *p_obj, void *parm);
vfe_status_t vfe_query_pre_fov_parm(vfe_ctrl_info_t *p_obj, void *parm);
vfe_status_t vfe_color_modules_enable(vfe_ctrl_info_t *vfe_ctrl_obj,
  uint8_t enable);
vfe_status_t vfe_blk_inc_comp(vfe_ctrl_info_t *p_obj, void *parm);
vfe_status_t vfe_set_frame_skip_pattern(vfe_ctrl_info_t *p_obj,
  vfe_frame_skip *ext);
vfe_status_t vfe_set_hfr_mode(vfe_ctrl_info_t *p_obj,
  vfe_hfr_info_t *hfr_info);
vfe_status_t vfe_is_stats_conf_enabled(vfe_ctrl_info_t *p_obj,
  void *parm);
vfe_status_t vfe_command_ops(vfe_ctrl_info_t *p_obj, void *parm);
vfe_status_t vfe_ops_init(vfe_client_t *vfe_client);
vfe_status_t vfe_ops_deinit(vfe_client_t* vfe_client);
vfe_status_t is_vfe_asf_reconfig(vfe_ctrl_info_t *p_obj, void *parm);
vfe_status_t vfe_query_stats_buf_ptr(vfe_ctrl_info_t *p_obj,
  vfe_get_parm_t type, void *parm);
vfe_status_t vfe_stats_parser_get_buf_ptr(stats_parser_mod_t *mod,
  vfe_params_t *params, stats_buffers_type_t **pp_stats_bufs);
int vfe_stats_parsing(vfe_ctrl_info_t *p_obj,
int isp_started, stats_type_t type, void *stats, void *stats_output);
vfe_status_t vfe_release_stats(vfe_ctrl_info_t *vfe_ctrl_obj);
void vfe_release_all_stats_buff(vfe_ctrl_info_t *p_obj);
vfe_status_t vfe_stop_autofocus(vfe_ctrl_info_t *vfe_ctrl_obj);

#endif /* __VFE_H__ */
