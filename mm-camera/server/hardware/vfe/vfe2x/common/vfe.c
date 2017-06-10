/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vfe.h"
#include "camera_dbg.h"
#include "vfe_config.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_VFE_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

static vfe_comp_objs_t vfe_objs;
static vfe_status_t vfe_enable_modules(vfe_ctrl_info_t *vfe_ctrl_obj,
  int hw_write);

/*===========================================================================
FUNCTION    - vfe_state_swap_dimensions -

DESCRIPTION
===========================================================================*/
static void vfe_state_swap_dimensions(uint16_t * x, uint16_t * y)
{
  *x ^= *y;
  *y ^= *x;
  *x ^= *y;
} /* vfe_state_swap_dimensions */

/*===========================================================================
 * FUNCTION    - vfe_operation_config -
 * DESCRIPTION:
 * DEPENDENCY:
 *==========================================================================*/
static void vfe_operation_config(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  vfe_params_t *params = (vfe_params_t *)&(vfe_ctrl_obj->vfe_params);

  vfe_ctrl_obj->vfe_op_cfg.startCmd.inputSource = VFE_INPUT_SOURCE_CAMIF;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.snapshotNumber = 8;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.chromaSubsampleEnable =
    VFE_ENABLE_CHROMA_SUBSAMPLE;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.blackLevelCorrectionEnable =
    (params->moduleCfg->blackLevelCorrectionEnable) ?
    VFE_ENABLE_BLACK_LEVEL_CORRECTION :
    VFE_DISABLE_BLACK_LEVEL_CORRECTION;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.lensRollOffCorrectionEnable =
    (params->moduleCfg->lensRollOffEnable) ?
    VFE_ENABLE_LENS_ROLL_OFF_CORRECTION :
    VFE_DISABLE_LENS_ROLL_OFF_CORRECTION;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.whiteBalanceEnable =
    (params->moduleCfg->whiteBalanceEnable) ?
    VFE_ENABLE_WHITE_BALANCE :
    VFE_DISABLE_WHITE_BALANCE;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.RGB_GammaEnable =
    (params->moduleCfg->rgbLUTEnable) ?
    VFE_ENABLE_RGB_GAMMA :
    VFE_DISABLE_RGB_GAMMA;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.lumaNoiseReductionPathEnable =
    VFE_DISABLE_LUMA_NOISE_REDUCTION_PATH;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.adaptiveSpatialFilterEnable =
    (params->moduleCfg->asfEnable) ?
    VFE_ENABLE_ADAPTIVE_SPATIAL_FILTER :
    VFE_DISABLE_ADAPTIVE_SPATIAL_FILTER;

  vfe_ctrl_obj->vfe_op_cfg.startCmd.lastPixel =
    params->active_crop_info.lastPixel;
  vfe_ctrl_obj->vfe_op_cfg.startCmd.lastLine =
    params->active_crop_info.lastLine;

  CDBG_HIGH("%s: format %d", __func__,
    vfe_ctrl_obj->vfe_params.sensor_parms.vfe_camif_in_fmt);

  /* operationMode */
  switch (vfe_ctrl_obj->vfe_params.vfe_op_mode) {
    case VFE_OP_MODE_PREVIEW:
    case VFE_OP_MODE_ZSL:
      vfe_ctrl_obj->vfe_op_cfg.startCmd.modeOfOperation =
        OPERATION_CONTINUOUS;
      break;
    case VFE_OP_MODE_SNAPSHOT:
      vfe_ctrl_obj->vfe_op_cfg.startCmd.modeOfOperation =
        OPERATION_SNAPSHOT;
      vfe_ctrl_obj->vfe_op_cfg.startCmd.snapshotNumber =
        vfe_ctrl_obj->vfe_out.num_output;
      CDBG("%s: set snapshot number here for burst mode %d", __func__,
        vfe_ctrl_obj->vfe_op_cfg.startCmd.snapshotNumber);
      break;
    case VFE_OP_MODE_VIDEO:
      vfe_ctrl_obj->vfe_op_cfg.startCmd.modeOfOperation =
        OPERATION_CONTINUOUS;
      break;
    case VFE_OP_MODE_RAW_SNAPSHOT:
      vfe_ctrl_obj->vfe_op_cfg.startCmd.modeOfOperation =
        OPERATION_SNAPSHOT;
      vfe_ctrl_obj->vfe_op_cfg.startCmd.chromaSubsampleEnable =
        VFE_DISABLE_CHROMA_SUBSAMPLE;
      vfe_ctrl_obj->vfe_op_cfg.startCmd.snapshotNumber = 1;
      break;
    default:
      CDBG_ERROR("%s: Invalid vfe_op_mode %d\n", __func__,
        vfe_ctrl_obj->vfe_params.vfe_op_mode);
      break;
  }

  CDBG("%s:\n", __func__);
  CDBG("inputSource: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.inputSource);
  CDBG("modeOfOperation: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.modeOfOperation);
  CDBG("snapshotNumber: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.snapshotNumber);
  CDBG("blackLevelCorrectionEnable: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.blackLevelCorrectionEnable);
  CDBG("lensRollOffCorrectionEnable: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.lensRollOffCorrectionEnable);
  CDBG("whiteBalanceEnable: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.whiteBalanceEnable);
  CDBG("RGB_GammaEnable: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.RGB_GammaEnable);
  CDBG("lumaNoiseReductionPathEnable: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.lumaNoiseReductionPathEnable);
  CDBG("adaptiveSpatialFilterEnable: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.adaptiveSpatialFilterEnable);
  CDBG("chromaSubsampleEnable: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.chromaSubsampleEnable);
  CDBG("lastPixel: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.lastPixel);
  CDBG("lastLine: %d\n",
    vfe_ctrl_obj->vfe_op_cfg.startCmd.lastLine);

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_ctrl_obj->vfe_params.camfd,
    CMD_GENERAL, (void *)&vfe_ctrl_obj->vfe_op_cfg.startCmd,
    sizeof(vfe_ctrl_obj->vfe_op_cfg.startCmd), VFE_CMD_OPERATION_CFG))
    CDBG_HIGH("%s: Start config command failed\n", __func__);
} /* vfe_operation_config */
/*===========================================================================
 * FUNCTION    - vfe_set_hfr_mode -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 * Also sets the hfr mode and frame skip pattern
 *==========================================================================*/
vfe_status_t vfe_set_hfr_mode(vfe_ctrl_info_t *p_obj,
  vfe_hfr_info_t *hfr_info)
{
  return VFE_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - vfe_set_frame_skip_pattern -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 * The pattern can be set by HDR or HFR modes
 *==========================================================================*/
vfe_status_t vfe_set_frame_skip_pattern(vfe_ctrl_info_t *p_obj,
  vfe_frame_skip *frm_skip)
{
  vfe_status_t status = VFE_SUCCESS;
  status = vfe_frame_skip_config_pattern(&(p_obj->vfe_module.frame_skip_mod),
    frm_skip);
  if(status != VFE_SUCCESS) {
    CDBG_ERROR("vfe_frame_skip_config_pattern failed\n");
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_stats_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function initializes all the vfe modules.
 *==========================================================================*/
vfe_status_t vfe_stats_init(stats_mod_t *stats, vfe_params_t *params)
{
  if (VFE_SUCCESS != vfe_aecawb_stats_init(&(stats->aecawb_stats),
    params))
    return VFE_ERROR_GENERAL;

  if (VFE_SUCCESS != vfe_af_stats_init(&(stats->af_stats),
    params))
    return VFE_ERROR_GENERAL;

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_modules_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This fucnction initializes all the vfe modules.
 *==========================================================================*/
vfe_status_t vfe_modules_init(vfe_module_t *vfe_module,
  vfe_params_t *params)
{
  int mod_id = 0;
  vfe_status_t status = VFE_SUCCESS;
  vfe_module->stats.use_bayer_stats = 0;

  CDBG("%s: enter config %u\n", __func__, params->current_config);

  if ((params->current_config & VFE_MOD_LINEARIZATION)) {
    status = vfe_black_level_init(&(vfe_module->linear_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: black_level init failed\n", __func__);
      return status;
    }
  }
#if 1
  if ((params->current_config & VFE_MOD_COLOR_PROC)) {
    status = vfe_color_proc_init(&(vfe_module->color_proc_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: color proc init failed\n", __func__);
      return status;
    }
  }
#endif
  if ((params->current_config & VFE_MOD_CLAMP)) {
    status = vfe_clamp_init(mod_id, &(vfe_module->clamp_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: clamp init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_WB)) {
    status = vfe_wb_init(mod_id, &(vfe_module->wb_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: white balance init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_DEMOSAIC)) {
    status = vfe_demosaic_init(mod_id, &(vfe_module->demosaic_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: demosaic init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_BPC)) {
    status = vfe_demosaic_bpc_init(&(vfe_module->bpc_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: demosaic bpc init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_CHROMA_SS)) {
    status = vfe_chroma_subsample_init(mod_id, &(vfe_module->chroma_ss_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: chroma subsample init failed\n", __func__);
      return status;
    }
  }
  if((params->current_config & VFE_MOD_FRAME_SKIP)) {
    status = vfe_frame_skip_init(&(vfe_module->frame_skip_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: frame skip init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_ASF)) {
    status = vfe_asf_init(mod_id, &vfe_module->asf_mod, params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: asf init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_DEMUX)) {
    status = vfe_demux_init(mod_id, &(vfe_module->demux_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: demux init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_ABF)) {
    status = vfe_abf_init(mod_id, &vfe_module->abf_mod, params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: ABF2 init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_GAMMA)) {
    status = vfe_gamma_init(mod_id, &vfe_module->gamma_mod, params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: gamma init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_ROLLOFF)) {
    status = vfe_rolloff_init(mod_id, &vfe_module->rolloff_mod, params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: rolloff init failed\n", __func__);
      return status;
    }
  }
  if ((params->current_config & VFE_MOD_ACTIVE_CROP)) {
    status = vfe_active_crop_init(&(vfe_module->active_crop_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: Active crop init failed\n", __func__);
      return status;
    }
  }
  return status;
} /* vfe_modules_init */

/*===========================================================================
 * FUNCTION    - vfe_params_init -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This fucnction initializes all the default vfe params.
 *==========================================================================*/
vfe_status_t vfe_params_init(vfe_ctrl_info_t *vfe_obj, vfe_params_t *params)
{
  params->prev_spl_effect = CAMERA_EFFECT_OFF;
  params->bs_mode = CAMERA_BESTSHOT_OFF;
  params->cam_mode = CAM_MODE_2D;
  params->moduleCfg = &vfe_obj->vfe_op_cfg.moduleCfg;
  params->vfe_op_mode = VFE_OP_MODE_INVALID;
  params->flash_params.flash_mode = VFE_FLASH_NONE;
  params->awb_params.color_temp = 4000;
  params->wb = CAMERA_WB_AUTO;
  params->eztune_status = FALSE;
  params->use_default_config = TRUE;
  params->current_config = (IS_BAYER_FORMAT(params)) ? VIDEO_CONFIG :
    YUV_VIDEO_CONFIG;
  params->stats_config = (params->current_config & ~STATS_ALL_MASK);
  params->prev_mode = VFE_OP_MODE_INVALID;
  params->demosaic_wb_not_present = FALSE;
  params->use_cc_for_dig_gain = FALSE;

  params->sharpness_info.asd_soft_focus_dgr = 1.0;
  params->sharpness_info.bst_soft_focus_dgr = 1.0;
  params->sharpness_info.ui_sharp_ctrl_factor = 1.0;
  params->sharpness_info.portrait_severity = 0.0;

  return VFE_SUCCESS;
}
/*===========================================================================
FUNCTION     - vfe_modules_deinit -

DESCRIPTION
===========================================================================*/
vfe_status_t vfe_modules_deinit(vfe_module_t *vfe_module, vfe_params_t *params)
{
  int mod_id = 0;
  if (VFE_SUCCESS != vfe_asf_deinit(mod_id, &vfe_module->asf_mod, params))
    return VFE_ERROR_GENERAL;
  if (VFE_SUCCESS != vfe_rolloff_deinit(mod_id, &vfe_module->rolloff_mod,
                       params))
    return VFE_ERROR_GENERAL;

  return VFE_SUCCESS;
} /* vfe_modules_deinit */

/*===========================================================================
 * FUNCTION    - vfe_cmd_hw_reg_update -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_cmd_hw_reg_update(vfe_ctrl_info_t *vfe_ctrl_obj,
  int* p_update)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_module_t *mod_obj = (vfe_module_t *)&vfe_ctrl_obj->vfe_module;
  vfe_params_t *params = (vfe_params_t *)&vfe_ctrl_obj->vfe_params;
  stats_mod_t *stats = (stats_mod_t *)&mod_obj->stats;
  current_output_info_t *vfe_out = (current_output_info_t *)&vfe_ctrl_obj->vfe_out;
  int mod_id = 0;
  params->update = 0;
  *p_update = FALSE;
  CDBG("vfe_cmd_hw_reg_update Entered");
  status = vfe_black_level_update(&mod_obj->linear_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: black_level update failed", __func__);
    return status;
  }
  status = vfe_asf_update(mod_id, &mod_obj->asf_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: asf update failed", __func__);
    return status;
  }
  status = vfe_wb_update(mod_id, &mod_obj->wb_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: wb update failed", __func__);
    return status;
  }

  if(params->current_config & VFE_MOD_COLOR_PROC) {
    status = vfe_color_proc_update(&mod_obj->color_proc_mod, params);
    if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: CC update failed", __func__);
    return status;
    }
  }

  status = vfe_gamma_update(mod_id, &mod_obj->gamma_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Gamma update failed", __func__);
    return status;
  }
  status = vfe_demosaic_bpc_update(&mod_obj->bpc_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: demosaic bpc update failed", __func__);
    return status;
  }
  status = vfe_abf_update(mod_id, &mod_obj->abf_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: ABF update failed", __func__);
    return status;
  }
#if 1
  status = vfe_aecawb_stats_update(&stats->aecawb_stats, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: wb trigger update failed", __func__);
    return status;
  }
#endif
#if 1
  status = vfe_af_stats_update(&stats->af_stats, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: af trigger update failed", __func__);
    return status;
  }
#endif
  status = vfe_fov_update(mod_id, &mod_obj->fov_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: FOV update failed", __func__);
    return status;
  }
  status = vfe_scaler_update(&mod_obj->scaler_mod, params, vfe_out);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: SCALER update failed", __func__);
    return status;
  }
  status = vfe_demosaic_update(mod_id, &mod_obj->demosaic_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: demosaic update failed", __func__);
    return status;
  }
  if (params->update) {
    if(params->eztune_status)
       ez_vfe_diagnostics_update((void *)vfe_ctrl_obj);
    status = vfe_util_write_hw_cmd (params->camfd, CMD_GENERAL, NULL, 0,
      VFE_CMD_UPDATE);
    if (VFE_SUCCESS == status)
      *p_update = TRUE;
  }
  CDBG("%s: status %d mod 0x%x", __func__, status, params->update);
  return VFE_SUCCESS;
} /* vfe_cmd_hw_reg_update */

/*===========================================================================
 * FUNCTION    - vfe_trigger_update_for_aec -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_trigger_update_for_aec(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  /* Note: Since AWB is dependent on AEC, trigger update for AWB always
   *       comes after trigger update of AEC. So move all the vfe module
   *       triggers to AWB.
   */
  return VFE_SUCCESS;
} /* vfe_trigger_update_for_aec */

/*===========================================================================
 * FUNCTION    - vfe_trigger_update_for_awb -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_trigger_update_for_awb(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_module_t *mod_obj = (vfe_module_t *)&vfe_ctrl_obj->vfe_module;
  vfe_params_t *params = (vfe_params_t *)&vfe_ctrl_obj->vfe_params;
  stats_mod_t *stats = (stats_mod_t *)&mod_obj->stats;
  int mod_id = 0;

  CDBG("%s: enter", __func__);
#if 1
  status = vfe_black_level_trigger_update(&mod_obj->linear_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: black_level trigger update failed", __func__);
    return status;
  }
  if (vfe_ctrl_obj->vfe_params.vfe_op_mode == VFE_OP_MODE_SNAPSHOT) {
    status = vfe_rolloff_trigger_update(mod_id, &mod_obj->rolloff_mod, params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: rolloff trigger update failed", __func__);
      return status;
    }
  }
  status = vfe_wb_trigger_update(mod_id, &mod_obj->wb_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: wb trigger update failed", __func__);
    return status;
  }
  status = vfe_demosaic_trigger_update (mod_id, &mod_obj->demosaic_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Demosaic trigger update failed", __func__);
    return status;
  }
  status = vfe_demosaic_bpc_trigger_update(&mod_obj->bpc_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Demosaic BPC trigger update failed", __func__);
    return status;
  }
  status = vfe_abf_trigger_update(mod_id, &mod_obj->abf_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: ABF trigger update failed", __func__);
    return status;
  }
#if 1
  status = vfe_asf_trigger_update(mod_id, &mod_obj->asf_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: asf trigger update failed", __func__);
    return status;
  }
#endif

  if(params->current_config & VFE_MOD_COLOR_PROC) {
    status = vfe_color_proc_trigger_update(&mod_obj->color_proc_mod,
      params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: CC trigger update failed", __func__);
      return status;
    }
  }

#endif
  return VFE_SUCCESS;
} /* vfe_trigger_update_for_awb */

/*===========================================================================
 * FUNCTION    - vfe_trigger_update_for_ihist -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_trigger_update_for_ihist(vfe_ctrl_info_t *vfe_ctrl_obj,
  uint32_t *ihist_stats_buf)
{
  CDBG_ERROR("%s: not supported", __func__);
  return VFE_SUCCESS;
} /* vfe_trigger_update_for_ihist */

/*===========================================================================
 * FUNCTION    - vfe_stats_config -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY:
 *==========================================================================*/
static vfe_status_t vfe_stats_config(stats_mod_t *stats,
  vfe_params_t *params)
{
  int8_t enable;
  enable = ((params->current_config & VFE_MOD_AEC_AWB_STATS) != 0);
  if (VFE_SUCCESS != vfe_aecawb_stats_enable(&(stats->aecawb_stats),
    params, enable, FALSE))
    return VFE_ERROR_GENERAL;

  if (VFE_SUCCESS != vfe_aecawb_stats_config(&(stats->aecawb_stats),
    params))
    return VFE_ERROR_GENERAL;

  enable = ((params->current_config & VFE_MOD_AF_STATS) != 0);
  if (VFE_SUCCESS != vfe_af_stats_enable(&(stats->af_stats),
    params, enable, FALSE))
    return VFE_ERROR_GENERAL;


  return VFE_SUCCESS;
} /* vfe_stats_config */
/*===========================================================================
 * FUNCTION    - vfe_set_output_info -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_set_output_info(vfe_ctrl_info_t *vfe_ctrl_obj,
                                 void *parm)
{
  vfe_status_t status = VFE_SUCCESS;
  current_output_info_t *vfe_out = (current_output_info_t *)parm;
  CDBG("%s: vfe output mode %d \n", __func__, vfe_out->vfe_operation_mode);

  if (vfe_out != NULL)
    vfe_ctrl_obj->vfe_out = *vfe_out;
  else
    status = VFE_ERROR_GENERAL;
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_enable_modules -
 *
 * DESCRIPTION:
 *==========================================================================*/
static vfe_status_t vfe_enable_modules(vfe_ctrl_info_t *vfe_ctrl_obj,
  int hw_write)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_module_t *mod_obj = &(vfe_ctrl_obj->vfe_module);
  stats_mod_t *stats = &mod_obj->stats;
  vfe_params_t* params = &vfe_ctrl_obj->vfe_params;
  current_output_info_t *vfe_out = &vfe_ctrl_obj->vfe_out;
  vfe_diagnostics_t *diagnostics = &(vfe_ctrl_obj->vfe_diag);
  int8_t enable = FALSE;
  int8_t ez_gamma_diag_enable = TRUE, ez_rolloff_diag_enable = TRUE;
  int8_t ez_blacklevel_diag_enable = TRUE, ez_asf_diag_enable = TRUE;
  int mod_id = 0;

  CDBG("vfe_enable_modules Entered");
  enable = ((params->current_config & VFE_MOD_LINEARIZATION) != 0);
  if (params->eztune_status)
    ez_blacklevel_diag_enable = diagnostics->control_blacklevel.enable;
  if (!ez_blacklevel_diag_enable)
    enable = FALSE;
  status = vfe_black_level_enable(&mod_obj->linear_mod, params, enable,
    hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: black_level enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_FOV) != 0);
  status = vfe_fov_enable(mod_id, &mod_obj->fov_mod, params, enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: fov enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_SCALER) != 0);
  status = vfe_scaler_enable(&mod_obj->scaler_mod, params, vfe_out,
             enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: scaler enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_ROLLOFF) != 0);
  if (params->eztune_status)
    ez_rolloff_diag_enable = diagnostics->control_rolloff.enable;
  if (!ez_rolloff_diag_enable)
    enable = FALSE;
  status = vfe_rolloff_enable(mod_id, &mod_obj->rolloff_mod, params,
             enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: rolloff enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_DEMOSAIC) != 0);
  status = vfe_demosaic_enable(mod_id, &mod_obj->demosaic_mod, params, enable,
    hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Demosaic enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_BPC) != 0);
  status = vfe_demosaic_bpc_enable(&mod_obj->bpc_mod, params, enable,
    hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Demosaic BPC enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_ASF) != 0);
  if (params->eztune_status)
    ez_asf_diag_enable = diagnostics->control_asfsharp.enable;
  if (!ez_asf_diag_enable)
    enable = FALSE;
  status = vfe_asf_enable(mod_id, &mod_obj->asf_mod, params, enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: asf enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_DEMUX) != 0);
  status = vfe_demux_enable(mod_id, &mod_obj->demux_mod, params, enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: demux enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_WB) != 0);
  status = vfe_wb_enable(mod_id, &mod_obj->wb_mod, params, enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: wb enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_COLOR_PROC) != 0);
  if(params->current_config & VFE_MOD_COLOR_PROC) {
    status = vfe_color_proc_enable(&mod_obj->color_proc_mod, params,
      enable, hw_write);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: CV enable failed", __func__);
      return status;
    }
  }

  enable = ((params->current_config & VFE_MOD_GAMMA) != 0);
  if(params->eztune_status)
    ez_gamma_diag_enable = diagnostics->control_gamma.enable;
  if(!ez_gamma_diag_enable)
    enable = FALSE;
  status = vfe_gamma_enable(mod_id, &mod_obj->gamma_mod, params, enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: gamma enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_ABF) != 0);
  status = vfe_abf_enable(mod_id, &mod_obj->abf_mod, params, enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: abf enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_ACTIVE_CROP) != 0);
  status = vfe_active_crop_enable(&mod_obj->active_crop_mod, params,
    enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: la enable failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_CHROMA_SS) != 0);
  status = vfe_chroma_subsample_enable(mod_id, &mod_obj->chroma_ss_mod, params,
    enable, hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Chroma SS enable failed", __func__);
    return status;
  }
  enable = ((params->current_config & VFE_MOD_CLAMP) != 0);
  status = vfe_clamp_enable(mod_id, &mod_obj->clamp_mod, params, enable,
    hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Frame skip config failed", __func__);
    return status;
  }

  enable = ((params->current_config & VFE_MOD_FRAME_SKIP) != 0);
  status = vfe_frame_skip_enable(&mod_obj->frame_skip_mod, params, enable,
    hw_write);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Frame skip config failed", __func__);
    return status;
  }

  return status;
} /* vfe_enable_modules */

/*===========================================================================
 * FUNCTION    - vfe_config_pipeline -
 *
 * DESCRIPTION:
 *==========================================================================*/
static vfe_status_t vfe_config_pipeline(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_module_t *mod_obj = &(vfe_ctrl_obj->vfe_module);
  vfe_params_t* params = &vfe_ctrl_obj->vfe_params;
  current_output_info_t *vfe_out = (current_output_info_t *)&vfe_ctrl_obj->vfe_out;
  int mod_id = 0;
  /*enable the modules*/
  status = vfe_enable_modules(vfe_ctrl_obj, FALSE);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: enable modules failed", __func__);
    return status;
  }
  /*trigger update for 3A*/
  if ((params->vfe_op_mode == VFE_OP_MODE_SNAPSHOT) &&
       IS_BAYER_FORMAT(params)) {
    status = vfe_trigger_update_for_aec(vfe_ctrl_obj);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: AWB trigger failed", __func__);
      return status;
    }
    status = vfe_trigger_update_for_awb(vfe_ctrl_obj);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: AWB trigger failed", __func__);
      return status;
    }
  }
  vfe_update_crop(vfe_ctrl_obj, NULL);
  status = vfe_active_crop_config(&mod_obj->active_crop_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Active crop config failed", __func__);
    return status;
  }
  status = vfe_demux_config(mod_id, &mod_obj->demux_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: demux config failed", __func__);
    return status;
  }
  status = vfe_black_level_config(&(mod_obj->linear_mod), params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: black_level config failed", __func__);
    return status;
  }

  if(params->current_config & VFE_MOD_COLOR_PROC) {
  	status = vfe_color_proc_config(&(mod_obj->color_proc_mod), params);
  	if (VFE_SUCCESS != status) {
    	CDBG_HIGH("%s: color proc config failed", __func__);
    	return status;
  	}
  }
  status = vfe_wb_config(mod_id, &mod_obj->wb_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: wb config failed", __func__);
    return status;
  }
  status = vfe_asf_config(mod_id, &mod_obj->asf_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: asf config failed", __func__);
    return status;
  }
  status = vfe_demosaic_bpc_config(&(mod_obj->bpc_mod), params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: demosaic BPC config failed", __func__);
    return status;
  }
  status = vfe_abf_config(mod_id, &mod_obj->abf_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: abf2 config failed", __func__);
    return status;
  }
  status = vfe_rolloff_config(mod_id, &mod_obj->rolloff_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: rolloff config failed", __func__);
    return status;
  }
  if (((params->current_config & VFE_MOD_DEMOSAIC) != 0)) {
    status = vfe_demosaic_config(mod_id, &(mod_obj->demosaic_mod), params);
    if (VFE_SUCCESS != status) {
      CDBG_HIGH("%s: demosaic config failed", __func__);
      return status;
    }
  }
  status = vfe_fov_config(mod_id, &(mod_obj->fov_mod), params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: FOV config failed", __func__);
    return status;
  }

  status = vfe_scaler_config(&(mod_obj->scaler_mod), params, vfe_out);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: SCALER config failed", __func__);
    return status;
  }
  status = vfe_stats_config(&(vfe_ctrl_obj->vfe_module.stats), params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: stats config failed", __func__);
    return status;
  }
  status = vfe_gamma_config(mod_id, &mod_obj->gamma_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: gamma config failed", __func__);
    return status;
  }
  status = vfe_clamp_config(mod_id, &mod_obj->clamp_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: clamp config failed", __func__);
    return status;
  }
  status = vfe_frame_skip_config(&mod_obj->frame_skip_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Frame skip config failed", __func__);
    return status;
  }
  mod_obj->chroma_ss_mod.main_format =
    vfe_ctrl_obj->vfe_out.output[PRIMARY].format;
  status = vfe_chroma_subsample_config(mod_id, &mod_obj->chroma_ss_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Chroma SS config failed", __func__);
    return status;
  }
  vfe_operation_config(vfe_ctrl_obj);
  if(params->eztune_status)
    ez_vfe_diagnostics((void *)vfe_ctrl_obj);
  return VFE_SUCCESS;
} /* vfe_config_video_pipe */

/*===========================================================================
 * FUNCTION    - vfe_config_raw_snapshot_pipe -
 *
 * DESCRIPTION:
 *==========================================================================*/
static vfe_status_t vfe_config_raw_snapshot_pipe(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  /*VFE config not required*/
  vfe_operation_config(vfe_ctrl_obj);
  return VFE_SUCCESS;
} /* vfe_config_raw_snapshot_pipe */

/*===========================================================================
 * FUNCTION    - vfe_config_mode -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_config_mode(vfe_op_mode_t mode, vfe_ctrl_info_t *vfe_ctrl_obj)
{
  uint32_t clip_width, clip_height;
  vfe_status_t status = VFE_SUCCESS;
  vfe_params_t *params = &vfe_ctrl_obj->vfe_params;
  if(!vfe_ctrl_obj)
    return VFE_ERROR_GENERAL;

  vfe_ctrl_obj->vfe_params.prev_mode = vfe_ctrl_obj->vfe_params.vfe_op_mode;
  vfe_ctrl_obj->vfe_params.vfe_op_mode = mode;

  switch (mode) {
    case VFE_OP_MODE_PREVIEW:
      vfe_ctrl_obj->vfe_params.output2w = vfe_ctrl_obj->vfe_out.output[PRIMARY].image_width;
      vfe_ctrl_obj->vfe_params.output2h = vfe_ctrl_obj->vfe_out.output[PRIMARY].image_height;
      params->current_config = (IS_BAYER_FORMAT(params)) ? VIDEO_CONFIG :
        YUV_VIDEO_CONFIG;
      if (vfe_config_pipeline(vfe_ctrl_obj) != VFE_SUCCESS)
        goto error;
      break;

    case VFE_OP_MODE_VIDEO:
      vfe_ctrl_obj->vfe_params.output1w = vfe_ctrl_obj->vfe_out.output[SECONDARY].image_width;
      vfe_ctrl_obj->vfe_params.output1h = vfe_ctrl_obj->vfe_out.output[SECONDARY].image_height;
      vfe_ctrl_obj->vfe_params.output2w = vfe_ctrl_obj->vfe_out.output[PRIMARY].image_width;
      vfe_ctrl_obj->vfe_params.output2h = vfe_ctrl_obj->vfe_out.output[PRIMARY].image_height;
      if(params->use_default_config != TRUE)
        params->current_config |= params->stats_config;
      else
        params->current_config = (IS_BAYER_FORMAT(params)) ? VIDEO_CONFIG :
          YUV_VIDEO_CONFIG;
      CDBG_HIGH("%s: VideoCFg config %x", __func__, params->current_config);
      if (vfe_config_pipeline(vfe_ctrl_obj) != VFE_SUCCESS)
        goto error;
      break;

    case VFE_OP_MODE_SNAPSHOT:
      clip_width = vfe_ctrl_obj->vfe_params.sensor_parms.sensor_width;
      clip_height = vfe_ctrl_obj->vfe_params.sensor_parms.sensor_height;
      if ((vfe_ctrl_obj->vfe_out.output[SECONDARY].image_width +
           vfe_ctrl_obj->vfe_out.output[SECONDARY].extra_pad_width) > clip_width)
        vfe_ctrl_obj->vfe_params.output1w =
          clip_width & 0xFFF0;
      else
        vfe_ctrl_obj->vfe_params.output1w =
        vfe_ctrl_obj->vfe_out.output[SECONDARY].image_width +
        vfe_ctrl_obj->vfe_out.output[SECONDARY].extra_pad_width;
      if ((vfe_ctrl_obj->vfe_out.output[SECONDARY].image_height +
            vfe_ctrl_obj->vfe_out.output[SECONDARY].extra_pad_height) > clip_height)
        vfe_ctrl_obj->vfe_params.output1h = clip_height;
      else
        vfe_ctrl_obj->vfe_params.output1h =
        vfe_ctrl_obj->vfe_out.output[SECONDARY].image_height +
        vfe_ctrl_obj->vfe_out.output[SECONDARY].extra_pad_height;

      /* Output2 dimensions */
      if ((vfe_ctrl_obj->vfe_out.output[PRIMARY].image_width +
           vfe_ctrl_obj->vfe_out.output[PRIMARY].extra_pad_width) > clip_width)
        vfe_ctrl_obj->vfe_params.output2w = clip_width & 0xFFF0;
      else
        vfe_ctrl_obj->vfe_params.output2w = vfe_ctrl_obj->vfe_out.output[PRIMARY].image_width +
           vfe_ctrl_obj->vfe_out.output[PRIMARY].extra_pad_width;
      if ((vfe_ctrl_obj->vfe_out.output[PRIMARY].image_height +
           vfe_ctrl_obj->vfe_out.output[PRIMARY].extra_pad_height) > clip_height)
        vfe_ctrl_obj->vfe_params.output2h = clip_height;
      else
        vfe_ctrl_obj->vfe_params.output2h = vfe_ctrl_obj->vfe_out.output[PRIMARY].image_height +
           vfe_ctrl_obj->vfe_out.output[PRIMARY].extra_pad_height;


      if(params->use_default_config != TRUE)
        params->current_config &= STATS_ALL_MASK;
      else
        params->current_config = (IS_BAYER_FORMAT(params)) ? SNAPSHOT_CONFIG :YUV_VIDEO_CONFIG;
          /*YUV_SNAPSHOT_CONFIG;*/

      CDBG_HIGH("%s: SnapshotCfg config %x", __func__, params->current_config);
      if (vfe_config_pipeline(vfe_ctrl_obj) != VFE_SUCCESS)
        goto error;

      break;

    case VFE_OP_MODE_RAW_SNAPSHOT:
      vfe_ctrl_obj->vfe_params.output2w = params->vfe_input_win.width;
      vfe_ctrl_obj->vfe_params.output2h = params->vfe_input_win.height;

      CDBG("raw output w=%d h=%d\n", vfe_ctrl_obj->vfe_params.output2w,
        vfe_ctrl_obj->vfe_params.output2h);
      if (vfe_config_raw_snapshot_pipe(vfe_ctrl_obj) != VFE_SUCCESS)
        goto error;
      break;

    case VFE_OP_MODE_ZSL:
      clip_width = vfe_ctrl_obj->vfe_params.sensor_parms.sensor_width;
      clip_height = vfe_ctrl_obj->vfe_params.sensor_parms.sensor_height;
      /* Output1 dimensions */
      /* Todo: If Snapshot size is smaller than preview */
      vfe_ctrl_obj->vfe_params.output1w =
        vfe_ctrl_obj->vfe_out.output[SECONDARY].image_width +
        vfe_ctrl_obj->vfe_out.output[SECONDARY].extra_pad_width;
      vfe_ctrl_obj->vfe_params.output1h =
        vfe_ctrl_obj->vfe_out.output[SECONDARY].image_height +
        vfe_ctrl_obj->vfe_out.output[SECONDARY].extra_pad_height;

      /* Output2 dimensions */
      if ((vfe_ctrl_obj->vfe_out.output[PRIMARY].image_width +
           vfe_ctrl_obj->vfe_out.output[PRIMARY].extra_pad_width) > clip_width)
        vfe_ctrl_obj->vfe_params.output2w = clip_width & 0xFFF0;
      else
        vfe_ctrl_obj->vfe_params.output2w =
          vfe_ctrl_obj->vfe_out.output[PRIMARY].image_width +
          vfe_ctrl_obj->vfe_out.output[PRIMARY].extra_pad_width;

      if ((vfe_ctrl_obj->vfe_out.output[PRIMARY].image_height +
           vfe_ctrl_obj->vfe_out.output[PRIMARY].extra_pad_height) > clip_height)
        vfe_ctrl_obj->vfe_params.output2h = clip_height;
      else
        vfe_ctrl_obj->vfe_params.output2h =
          vfe_ctrl_obj->vfe_out.output[PRIMARY].image_height +
          vfe_ctrl_obj->vfe_out.output[PRIMARY].extra_pad_height;

      params->current_config = (IS_BAYER_FORMAT(params)) ? ZSL_CONFIG :
        YUV_ZSL_CONFIG;
      if (vfe_config_pipeline(vfe_ctrl_obj) != VFE_SUCCESS)
        goto error;
      break;

    default:
      CDBG_ERROR("%s: Invalid mode \n", __func__);
      return VFE_ERROR_GENERAL;
  }
  CDBG("%s: Success fully Configured mode = %d \n", __func__, mode);
  return VFE_SUCCESS;

error:
  CDBG_ERROR("%s: Error while configuring mode = %d \n", __func__, mode);
  return VFE_ERROR_GENERAL;
} /* vfe_config_mode */

/*===========================================================================
 * FUNCTION    - vfe_query_stats_buf_ptr -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after init.
 *==========================================================================*/
vfe_status_t vfe_query_stats_buf_ptr(vfe_ctrl_info_t *p_obj,
  vfe_get_parm_t type, void *parm)
{
  vfe_status_t status = VFE_SUCCESS;
  stats_buffers_type_t *output = (stats_buffers_type_t *)parm;

  status = vfe_stats_parser_get_buf_ptr(
    &(p_obj->vfe_module.stats.parser_mod), &p_obj->vfe_params,
    &output);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: failed", __func__);
    return VFE_ERROR_GENERAL;
  }
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_query_shift_bits -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_query_shift_bits(vfe_ctrl_info_t *p_obj, vfe_get_parm_t type,
  void *parm)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_stats_output_t *output = (vfe_stats_output_t *)parm;

  switch (type) {
    case VFE_GET_AEC_SHIFT_BITS:
      output->aec_params.rgn_height =
        p_obj->vfe_module.stats.aecawb_stats.rgn_height;
      output->aec_params.rgn_width =
        p_obj->vfe_module.stats.aecawb_stats.rgn_width;
      CDBG("%s: Width : %d Height : %d\n", __func__, output->aec_params.rgn_width,
        output->aec_params.rgn_height);
      break;

    case VFE_GET_IHIST_SHIFT_BITS:
      status = VFE_ERROR_NOT_SUPPORTED;
      break;

  case VFE_GET_AWB_SHIFT_BITS:
#if 0
      status = vfe_awb_stats_get_shiftbits(
        &p_obj->vfe_module.stats.awb_stats, &p_obj->vfe_params,
        &output->awb_shift_bits);
#endif
      break;

    default:
      CDBG_ERROR("%s: Invalid mode \n", __func__);
      return VFE_ERROR_GENERAL;
      break;
  }
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_query_fov_crop -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_query_fov_crop(vfe_ctrl_info_t *p_obj, vfe_get_parm_t type,
  void *parm)
{

  switch (type) {
    case VFE_GET_FOV_CROP_PARM: {
      vfe_fov_crop_params_t *fov_crop = (vfe_fov_crop_params_t *)parm;
      fov_crop->first_pixel =
        p_obj->vfe_module.fov_mod.fov_cmd.firstPixel;
      fov_crop->last_pixel =
        p_obj->vfe_module.fov_mod.fov_cmd.lastPixel;
      fov_crop->first_line =
        p_obj->vfe_module.fov_mod.fov_cmd.firstLine;
      fov_crop->last_line =
        p_obj->vfe_module.fov_mod.fov_cmd.lastLine;
      break;
    }
    case VFE_GET_ZOOM_CROP_INFO: {
      vfe_zoom_crop_info_t *zoom_crop = (vfe_zoom_crop_info_t *)parm;
      zoom_crop->output1h = p_obj->vfe_params.output1h;
      zoom_crop->output2h = p_obj->vfe_params.output2h;
      zoom_crop->output1w = p_obj->vfe_params.output1w;
      zoom_crop->output2w = p_obj->vfe_params.output2w;
      zoom_crop->crop_win.x = p_obj->vfe_params.crop_info.crop_first_pixel;
      zoom_crop->crop_win.y = p_obj->vfe_params.crop_info.crop_first_line;
      zoom_crop->crop_win.crop_out_x = p_obj->vfe_params.crop_info.crop_out_x;
      zoom_crop->crop_win.crop_out_y = p_obj->vfe_params.crop_info.crop_out_y;
      break;
    }
    default:
      CDBG_ERROR("%s: Invalid mode \n", __func__);
      return VFE_ERROR_GENERAL;
      break;
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_apply_hue_saturation -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t vfe_apply_hue_saturation(vfe_effects_params_t *params)
{
  switch (params->spl_effect) {
    case CAMERA_EFFECT_SEPIA:
    case CAMERA_EFFECT_MONO:
    case CAMERA_EFFECT_NEGATIVE:
    case CAMERA_EFFECT_AQUA:
    case CAMERA_EFFECT_EMBOSS:
    case CAMERA_EFFECT_SKETCH:
      return FALSE;
    default:;
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - vfe_apply_contrast -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t vfe_apply_contrast(vfe_effects_params_t *params)
{
  switch (params->spl_effect) {
    case CAMERA_EFFECT_POSTERIZE:
    case CAMERA_EFFECT_SOLARIZE:
      return FALSE;
    default:;
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - vfe_set_spl_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_set_spl_effect(vfe_ctrl_info_t *vfe_ctrl_obj,
  vfe_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;

  /* reset the old values */
  switch (params->prev_spl_effect) {
    case CAMERA_EFFECT_OFF: {
      break;
    }

    case CAMERA_EFFECT_EMBOSS:
    case CAMERA_EFFECT_SKETCH: {
      status = vfe_color_conversion_set_spl_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.color_proc_mod.cv_mod, params,
        CAMERA_EFFECT_OFF);
      if (VFE_SUCCESS != status) {
        CDBG_HIGH("%s: CV set %d failed", __func__,
          params->effects_params.spl_effect);
        return VFE_ERROR_GENERAL;
      }
      status = vfe_asf_set_special_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.asf_mod, params, CAMERA_EFFECT_OFF);
      if (VFE_SUCCESS != status) {
        CDBG_HIGH("%s: ASF set %d failed", __func__,
          params->effects_params.spl_effect);
        return VFE_ERROR_GENERAL;
      }
      break;
    }

    case CAMERA_EFFECT_NEON: {
      status = vfe_asf_set_special_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.asf_mod, params, CAMERA_EFFECT_OFF);
      if (VFE_SUCCESS != status) {
        CDBG_HIGH("%s: ASF reset %d failed", __func__, params->prev_spl_effect);
        return VFE_ERROR_GENERAL;
      }
      break;
    }

    case CAMERA_EFFECT_SEPIA:
    case CAMERA_EFFECT_MONO:
    case CAMERA_EFFECT_NEGATIVE:
    case CAMERA_EFFECT_AQUA: {
      status = vfe_color_conversion_set_spl_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.color_proc_mod.cv_mod, params,
        CAMERA_EFFECT_OFF);
      if (VFE_SUCCESS != status) {
        CDBG_HIGH("%s: CV reset %d failed", __func__, params->prev_spl_effect);
        return VFE_ERROR_GENERAL;
      }
      break;
    }

    case CAMERA_EFFECT_POSTERIZE:
    case CAMERA_EFFECT_SOLARIZE:
    case CAMERA_EFFECT_BLACKBOARD:
    case CAMERA_EFFECT_WHITEBOARD: {
      status = vfe_gamma_set_spl_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.gamma_mod,
        params,
        CAMERA_EFFECT_OFF);
      if (VFE_SUCCESS != status) {
        CDBG_HIGH("%s: Gamma reset %d failed", __func__,
          params->prev_spl_effect);
        return VFE_ERROR_GENERAL;
      }
      break;
    }
    default:;
  }

  /* apply the new effects */
  switch (params->effects_params.spl_effect) {
    case CAMERA_EFFECT_OFF:
      break;

    case CAMERA_EFFECT_EMBOSS:
    case CAMERA_EFFECT_SKETCH:
      break;

    case CAMERA_EFFECT_NEON:
      break;

    case CAMERA_EFFECT_SEPIA:
    case CAMERA_EFFECT_MONO:
    case CAMERA_EFFECT_NEGATIVE:
    case CAMERA_EFFECT_AQUA: {
      status = vfe_color_conversion_set_spl_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.color_proc_mod.cv_mod, params,
        params->effects_params.spl_effect);
      if (VFE_SUCCESS != status) {
        CDBG_HIGH("%s: CV set %d failed", __func__,
          params->effects_params.spl_effect);
        return VFE_ERROR_GENERAL;
      }
      params->effects_params.hue = 0;
      params->effects_params.saturation = .5;
      break;
    }

    case CAMERA_EFFECT_POSTERIZE:
    case CAMERA_EFFECT_SOLARIZE:
    case CAMERA_EFFECT_BLACKBOARD:
    case CAMERA_EFFECT_WHITEBOARD: {
      status = vfe_gamma_set_spl_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.gamma_mod,
        params,
        params->effects_params.spl_effect);
      if (VFE_SUCCESS != status) {
        CDBG_HIGH("%s: Gamma set %d failed", __func__,
          params->effects_params.spl_effect);
        return VFE_ERROR_GENERAL;
      }
      break;
    }
    default:;
  }
  params->prev_spl_effect = params->effects_params.spl_effect;

  CDBG("%s: return status =%d", __func__, status);
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_set_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_set_bestshot(vfe_ctrl_info_t *vfe_ctrl_obj,
  camera_bestshot_mode_type mode)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_params_t *params = &vfe_ctrl_obj->vfe_params;
  vfe_module_t *module = &vfe_ctrl_obj->vfe_module;
  int mod_id = 0;

  if (mode == params->bs_mode) {
    CDBG_HIGH("%s: Bestshot mode not changed", __func__);
    return status;
  }
  params->bs_mode = mode;
  status = vfe_color_conversion_set_bestshot(mod_id,
    &module->color_proc_mod.cv_mod, params, mode);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: CV set bestshot failed", __func__);
    return status;
  }
  status = vfe_color_correct_set_bestshot(mod_id,
    &module->color_proc_mod.cc_mod, params, mode);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: CC set bestshot failed", __func__);
    return status;
  }
  status = vfe_gamma_set_bestshot(mod_id,
    &module->gamma_mod, params, mode);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: Gamma set bestshot failed", __func__);
    return status;
  }
  status = vfe_wb_set_bestshot(mod_id,&module->wb_mod, params, mode);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: CV set bestshot failed", __func__);
    return status;
  }
  status = vfe_asf_set_bestshot(mod_id,&module->asf_mod, params, mode);
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: ASF set bestshot failed", __func__);
    return status;
  }

  return status;
} /* vfe_set_bestshot */

/*===========================================================================
 * FUNCTION    - vfe_set_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_set_effect(vfe_ctrl_info_t *vfe_ctrl_obj,
  vfe_effects_type_t type,
  vfe_effects_params_t *params)
{
  vfe_status_t status = VFE_SUCCESS;
  int mod_id = 0;

  switch (type) {
    case VFE_CONTRAST: {
      if (!vfe_apply_contrast(params)) {
        status = VFE_ERROR_INVALID_OPERATION;
        break;
      }
      status = vfe_gamma_set_contrast(mod_id,
        &vfe_ctrl_obj->vfe_module.gamma_mod,
        &vfe_ctrl_obj->vfe_params,
        (params->contrast * 10));
      if (VFE_SUCCESS != status)
        CDBG_HIGH("%s: Invalid operation", __func__);
      break;
    }
    case VFE_HUE: {
      if (!vfe_apply_hue_saturation(params)) {
        status = VFE_ERROR_INVALID_OPERATION;
        break;
      }
      status = vfe_color_conversion_set_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.color_proc_mod.cv_mod,
        &vfe_ctrl_obj->vfe_params,
        type);
      if (status != VFE_SUCCESS) {
        CDBG_ERROR("%s: cannot apply hue", __func__);
        return status;
      }
      status = vfe_color_correct_set_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.color_proc_mod.cc_mod,
        &vfe_ctrl_obj->vfe_params,
        type);
      break;
    }
    case VFE_SATURATION: {
      if (!vfe_apply_hue_saturation(params)) {
        status = VFE_ERROR_INVALID_OPERATION;
        break;
      }
      status = vfe_color_conversion_set_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.color_proc_mod.cv_mod,
        &vfe_ctrl_obj->vfe_params,
        type);
      if (status != VFE_SUCCESS) {
        CDBG_ERROR("%s: cannot apply saturation", __func__);
        return status;
      }
      status = vfe_color_correct_set_effect(mod_id,
        &vfe_ctrl_obj->vfe_module.color_proc_mod.cc_mod,
        &vfe_ctrl_obj->vfe_params,
        type);
      break;
    }
    case VFE_SPL_EFFECT: {
      status = vfe_set_spl_effect(vfe_ctrl_obj, &vfe_ctrl_obj->vfe_params);
      break;
    }
    default:
      break;
  }
  return status;
} /* vfe_set_effect */

/*===========================================================================
 * FUNCTION    - vfe_config_autofocus -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_config_autofocus(vfe_ctrl_info_t *vfe_ctrl_obj,
  vfe_stats_af_params_t *af_params)
{
  vfe_status_t status = VFE_SUCCESS;

  status = vfe_af_calc_roi(vfe_ctrl_obj, af_params);
  if(status != VFE_SUCCESS) {
    CDBG_ERROR("%s: failed af_calc_roi \n", __func__);
    return status;
  }

  status = vfe_af_stats_config(vfe_ctrl_obj, af_params);
  if(status != VFE_SUCCESS) {
    CDBG_ERROR("%s: failed af_stats_config \n", __func__);
    return status;
  }
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_set_manual_wb -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_set_manual_wb(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_params_t *params = &vfe_ctrl_obj->vfe_params;
  vfe_module_t *module = &vfe_ctrl_obj->vfe_module;
  int mod_id = 0;

  status = vfe_wb_set_manual_wb(mod_id, &module->wb_mod, params);
  if (VFE_SUCCESS != status) {
    CDBG("%s: Set manual WB failed %d", __func__, status);
    return status;
  }
  status = vfe_color_conversion_set_manual_wb(mod_id,
    &module->color_proc_mod.cv_mod, params);
  return status;
}/*vfe_set_manual_wb*/

/*===========================================================================
 * FUNCTION    - vfe_update_crop -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_update_crop(vfe_ctrl_info_t *vfe_ctrl_obj,
  crop_window_info_t *fov_params)
{
  /* need more cleanup later to fully remove set_zoom_fov */
  if(vfe_ctrl_obj->vfe_params.output2w > 0) {
    if (VFE_SUCCESS != vfe_crop_config(&(vfe_ctrl_obj->vfe_params)))
      return VFE_ERROR_GENERAL;

    vfe_ctrl_obj->vfe_module.fov_mod.fov_update = TRUE;
    vfe_ctrl_obj->vfe_module.scaler_mod.scaler_update = TRUE;
    CDBG("%s: fov_update = %d, scaler_update = %d", __func__,
      vfe_ctrl_obj->vfe_module.fov_mod.fov_update,
      vfe_ctrl_obj->vfe_module.scaler_mod.scaler_update);
  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_calc_pixel_crop_info -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_calc_pixel_crop_info(vfe_ctrl_info_t *p_obj, void *parm)
{
  pixel_crop_info_t *info = (pixel_crop_info_t *)parm;

  *info = p_obj->vfe_params.demosaic_op_params;

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_calc_pixel_crop_info -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_set_pixel_crop_info(vfe_ctrl_info_t *p_obj, void *parm)
{
  pixel_crop_info_t *info = (pixel_crop_info_t *)parm;
  vfe_sensor_params_t *sparams = &(p_obj->vfe_params.sensor_parms);
  uint32_t delta;

  sparams->firstPixel =  info->first_pixel;
  sparams->lastPixel =  info->last_pixel;
  sparams->firstLine =  info->first_line;
  sparams->lastLine =  info->last_line;

  if ((sparams->lastLine - sparams->firstLine + 1) > MAX_VFE_HEIGHT) {
    delta = (MAX_VFE_HEIGHT - (sparams->lastLine - sparams->firstLine + 1))
      / 2;
    p_obj->vfe_params.demosaic_op_params.first_line =
      sparams->firstLine + delta - 1;
    p_obj->vfe_params.demosaic_op_params.last_line = sparams->lastLine - delta;
  } else {
    p_obj->vfe_params.demosaic_op_params.first_line = sparams->firstLine;
    p_obj->vfe_params.demosaic_op_params.last_line = sparams->lastLine;
  }

  if ((sparams->lastPixel - sparams->firstPixel + 1) > MAX_VFE_WIDTH) {
    delta = (MAX_VFE_WIDTH - (sparams->lastPixel - sparams->firstPixel + 1))
      / 2;
    p_obj->vfe_params.demosaic_op_params.first_pixel =
      sparams->firstPixel + delta - 1;
    p_obj->vfe_params.demosaic_op_params.last_pixel = sparams->lastPixel - delta;
  } else {
    p_obj->vfe_params.demosaic_op_params.first_pixel = sparams->firstPixel;
    p_obj->vfe_params.demosaic_op_params.last_pixel = sparams->lastPixel;
  }

  p_obj->vfe_params.demosaic_op_params.first_pixel += DEMOSAIC_WIDTH_DELTA/2;
  p_obj->vfe_params.demosaic_op_params.last_pixel -= DEMOSAIC_WIDTH_DELTA/2;
  p_obj->vfe_params.demosaic_op_params.first_line += DEMOSAIC_HEIGHT_DELTA/2;
  p_obj->vfe_params.demosaic_op_params.last_line -= DEMOSAIC_HEIGHT_DELTA/2;

  vfe_init_active_crop_info(&(p_obj->vfe_module.active_crop_mod), &(p_obj->vfe_params));

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_calc_pixel_skip_info -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_calc_pixel_skip_info(vfe_ctrl_info_t *p_obj, void *parm)
{
  pixel_skip_info_t *info = (pixel_skip_info_t *)parm;
  info->pixel_skip = 1;
  info->line_skip = 1;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_pp_info -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY:
 *==========================================================================*/
vfe_status_t vfe_pp_info(vfe_ctrl_info_t *p_obj, vfe_pp_params_t *pp_info)
{
  vfe_module_t *mod_obj = &(p_obj->vfe_module);
  vfe_params_t* params = &p_obj->vfe_params;
  vfe_status_t status = VFE_SUCCESS;

  status = vfe_gamma_get_table(&mod_obj->gamma_mod, params,
    &pp_info->gamma_num_entries, &pp_info->gamma_table);
  if(status != VFE_SUCCESS) {
    CDBG_HIGH("%s:GAMMA table update for WD failed \n",__func__);
    return VFE_ERROR_GENERAL;
  }
  pp_info->la_enable = 0;
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_query_rs_cs_parm -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY:
 *==========================================================================*/
vfe_status_t vfe_query_rs_cs_parm(vfe_ctrl_info_t *p_obj, vfe_get_parm_t type,
  void *parm)
{
  vfe_stats_rs_cs_params *out = (vfe_stats_rs_cs_params *)parm;
  vfe_stats_rs_cs_params *in = &(p_obj->vfe_params.rs_cs_params);

  memcpy(out, in, sizeof(vfe_stats_rs_cs_params));

  CDBG("%s: RS Stats Configurations\n", __func__);
  CDBG("%s: rs max rgns = %d \n", __func__, out->rs_max_rgns);
  CDBG("%s: rs num rgns = %d \n", __func__, out->rs_num_rgns);
  CDBG("%s: rs shiftBits = %d \n", __func__, out->rs_shift_bits);
  CDBG("%s: CS Stats Configurations\n", __func__);
  CDBG("%s: cs max rgns = %d \n", __func__, out->cs_max_rgns);
  CDBG("%s: cs num rgns = %d \n", __func__, out->cs_num_rgns);
  CDBG("%s: cs shiftBits = %d \n", __func__, out->cs_shift_bits);

  return VFE_SUCCESS;
} /* vfe_query_rs_cs_parm */

/*===========================================================================
 * FUNCTION    - vfe_set_mce -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_set_mce(vfe_ctrl_info_t *vfe_ctrl_obj, uint32_t enable)
{
  return VFE_ERROR_INVALID_OPERATION;
}

/*===========================================================================
 * FUNCTION    - vfe_set_sce -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_set_sce(vfe_ctrl_info_t *vfe_ctrl_obj, int32_t sce_factor)
{
  return VFE_ERROR_INVALID_OPERATION;
}

/*===========================================================================
 * FUNCTION    - vfe_blk_inc_comp -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_blk_inc_comp(vfe_ctrl_info_t *p_obj, void *parm)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_stats_output_t *output = (vfe_stats_output_t *)parm;

  output->blk_inc_comp = p_obj->vfe_module.linear_mod.blk_inc_comp;
  return VFE_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - vfe_stop_autofocus -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
vfe_status_t vfe_stop_autofocus(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  vfe_status_t status = VFE_SUCCESS;

  status = vfe_af_stats_stop(&(vfe_ctrl_obj->vfe_module.stats.af_stats),
    &(vfe_ctrl_obj->vfe_params));
  if (VFE_SUCCESS != status) {
    CDBG_HIGH("%s: failed", __func__);
    return VFE_ERROR_GENERAL;
  }
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_is_stats_conf_enabled -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function returns the stats module that are enabled
 *==========================================================================*/
vfe_status_t vfe_is_stats_conf_enabled(vfe_ctrl_info_t *p_obj, void *parm)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_params_t *params = &(p_obj->vfe_params);
  vfe_stats_conf_enable_t *p_stats_conf = (vfe_stats_conf_enable_t *)parm;
  p_stats_conf->aec_enabled = p_obj->vfe_module.stats.aecawb_stats.enable;
  p_stats_conf->awb_enabled = p_obj->vfe_module.stats.aecawb_stats.enable;
  p_stats_conf->af_enabled = p_obj->vfe_module.stats.af_stats.enable;
  p_stats_conf->rs_cs_enabled = 0;
  p_stats_conf->hist_enabled = 0;
  return status;
}/*vfe_is_stats_conf_enabled*/

/*===========================================================================
 * FUNCTION    - isp_send_hw_cmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_send_hw_cmd(int fd, int type,
  void *pCmdData, unsigned int messageSize, int cmd_id)
{
  int rc;
  struct msm_vfe_cfg_cmd cfgCmd;
  struct msm_isp_cmd ispcmd;

  ispcmd.id = cmd_id;
  ispcmd.length = messageSize;
  ispcmd.value = pCmdData;

  cfgCmd.cmd_type = type;
  cfgCmd.length = sizeof(struct msm_isp_cmd);
  cfgCmd.value = &ispcmd;

  rc = ioctl(fd, MSM_CAM_IOCTL_CONFIG_VFE, &cfgCmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_CAM_IOCTL_CONFIG_VFE failed %d\n",
      __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION    - vfe_command_ops -
 *
 * DESCRIPTION:
 *
 *==========================================================================*/
vfe_status_t vfe_command_ops(vfe_ctrl_info_t *p_obj, void *parm)
{
  int rc = 0;
  mod_cmd_t cmd = *(mod_cmd_t *)parm;
  CDBG(" %s cmd_type = %d", __func__, cmd);
  switch(cmd.mod_cmd_ops) {
    case MOD_CMD_START:
      rc = isp_send_hw_cmd(p_obj->vfe_params.camfd, CMD_GENERAL, NULL, 0, VFE_CMD_START);
      break;
    case MOD_CMD_STOP:
      rc = isp_send_hw_cmd(p_obj->vfe_params.camfd, CMD_GENERAL, NULL, 0, VFE_CMD_STOP);
      break;
    case MOD_CMD_RESET:
      rc = isp_send_hw_cmd(p_obj->vfe_params.camfd, CMD_GENERAL, NULL, 0, VFE_CMD_RESET);
      break;
    default:
      CDBG_ERROR("cmd_type = %d not supported", cmd.mod_cmd_ops);
      break;
  }
  if (VFE_SUCCESS != rc) {
    CDBG_ERROR("Failed configuring cmd_type = %d", cmd.mod_cmd_ops);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - vfe_get_obj -
 * DESCRIPTION:
 * DEPENDENCY: This function needs to be called in vfe_process
 *==========================================================================*/
vfe_ctrl_info_t *vfe_get_obj(vfe_client_t *vfe_client)
{
  vfe_ctrl_info_t *vfe_obj = NULL;
  /* now we have not implemented the use case of using 2 VFE obj */
  if(vfe_client->obj_idx_mask == 1)
    vfe_obj= &vfe_objs.vfe_obj[0];
  else if(vfe_client->obj_idx_mask == 2)
    vfe_obj= &vfe_objs.vfe_obj[1];
  return vfe_obj;
}
/*===========================================================================
 * FUNCTION    - vfe_init -
 * DESCRIPTION:
 * DEPENDENCY: This function needs to be called in vfe_process
 *==========================================================================*/
static int vfe_init(vfe_client_t *vfe_client, int event,
  void *parm)
{
  vfe_ctrl_info_t *vfe_ctrl_obj;
  sensor_output_format_t *snsr_fmt = parm;
  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    return -1;
  }

  vfe_ctrl_obj = vfe_get_obj(vfe_client);

  CDBG("%s: parm event =%d", __func__, event);
  if(!vfe_ctrl_obj) {
    CDBG_ERROR("%s: no VFE OBJ associated with clientr",  __func__);
    return -1;
  }
  vfe_ctrl_obj->vfe_params.sensor_parms.vfe_snsr_fmt = *snsr_fmt;
  if(VFE_SUCCESS != vfe_params_init(vfe_ctrl_obj, &vfe_ctrl_obj->vfe_params))
    return VFE_ERROR_GENERAL;

  if(VFE_SUCCESS != vfe_modules_init(&(vfe_ctrl_obj->vfe_module),
    &vfe_ctrl_obj->vfe_params))
    return VFE_ERROR_GENERAL;

  if (vfe_ctrl_obj->vfe_params.sensor_parms.vfe_snsr_fmt != SENSOR_YCBCR) {
    if (VFE_SUCCESS != vfe_stats_init(&(vfe_ctrl_obj->vfe_module.stats),
       &vfe_ctrl_obj->vfe_params))
      return VFE_ERROR_GENERAL;
	if(VFE_SUCCESS != vfe_stats_buffer_init(vfe_ctrl_obj)) {
      CDBG_ERROR("%s: vfe_stats_buffer_init error", __func__);
	  return VFE_ERROR_GENERAL;
	 }
  }

#ifdef FEATURE_VFE_TEST_VECTOR
  if (VFE_SUCCESS != vfe_test_vector_init(&vfe_ctrl_obj->test_vector,
    (void *)vfe_ctrl_obj))
    return VFE_ERROR_GENERAL;
#endif
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_add_instance -
 * DESCRIPTION:
 * DEPENDENCY: This function needs to be called in vfe_process
 *==========================================================================*/
static int vfe_add_instance(vfe_client_t *vfe_client, int type,
  void *parm_in, void *parm_out)
{
  vfe_ctrl_info_t *vfe_ctrl_obj;
  int vfe_obj_id = *((int *)parm_in);

  if((vfe_obj_id < 0) || (vfe_obj_id > VFE_MAX_OBJS)) {
    CDBG_ERROR("%s: Wrong ID, cannot support :%d",  __func__, vfe_obj_id);
    return -1;
  }

  if((vfe_client->obj_idx_mask & (1 << vfe_obj_id))) {
    CDBG_ERROR("%s: the object already exists : %d",  __func__, vfe_obj_id);
    return VFE_ERROR_INVALID_OPERATION;
  }

  vfe_client->obj_idx_mask |= (1 << vfe_obj_id);
  vfe_ctrl_obj = &vfe_objs.vfe_obj[vfe_obj_id];
  vfe_ctrl_obj->vfe_params.camfd = vfe_client->ops->fd;
  vfe_ctrl_obj->ops = vfe_client->ops;
  return VFE_SUCCESS;
} /* vfe_add_instance */

/*===========================================================================
 * FUNCTION    - vfe_process -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
static int vfe_process(void *client, int event, void *parm)
{
  vfe_client_t *vfe_client = (vfe_client_t *)client;
  vfe_ctrl_info_t *vfe_ctrl_obj;
  vfe_op_mode_t op_mode;
  int rc = VFE_SUCCESS;

  if(!vfe_client) {
    CDBG_ERROR("%s: vfe client is NULL",  __func__);
    return -1;
  }
  vfe_ctrl_obj = vfe_get_obj(vfe_client);

  CDBG("%s: parm event =%d", __func__, event);
  if(!vfe_ctrl_obj) {
    CDBG_ERROR("%s: no VFE OBJ associated with clientr",  __func__);
    return -1;
  }

  switch(event) {
  case VFE_MODULE_INIT:
    rc = vfe_init(vfe_client, event, parm);
    break;
  case VFE_CONFIG_MODE:
    op_mode = *(vfe_op_mode_t *)parm;
    rc = vfe_config_mode(op_mode, vfe_ctrl_obj);
    break;
  case VFE_TRIGGER_UPDATE_FOR_3A: {
    vfe_3a_parms_udpate_t *vfe_3a_update = (vfe_3a_parms_udpate_t *)parm;
    switch(vfe_3a_update->mask & VFE_3A_MASK) {
    case VFE_TRIGGER_UPDATE_AEC:
      rc = vfe_trigger_update_for_aec(vfe_ctrl_obj);
      break;
    case VFE_TRIGGER_UPDATE_AWB:
      rc = vfe_trigger_update_for_awb(vfe_ctrl_obj);
      break;
    default:
      CDBG_ERROR("Invalid 3A trigger 3a_mask = 0x%x\n", vfe_3a_update->mask);
      rc = VFE_ERROR_GENERAL;
      break;
    }
    break;
  }
  case VFE_SOF_NOTIFY: {
    int *p_update = (int *)parm;
    rc = vfe_cmd_hw_reg_update(vfe_ctrl_obj, p_update);
    break;
  }
  case VFE_CONFIG_AF: {
   vfe_stats_af_params_t *af_params = (vfe_stats_af_params_t *)parm;
   rc = vfe_config_autofocus(vfe_ctrl_obj, af_params);
   break;
  }
  case VFE_STOP_AF:
    rc = vfe_stop_autofocus(vfe_ctrl_obj);
    if(rc != VFE_SUCCESS) {
      CDBG_ERROR("%s: VFE_STOP_AF failed\n", __func__);
    }
   break;
  case VFE_CMD_OPS:
    rc = vfe_command_ops(vfe_ctrl_obj, parm);
  break;
  case VFE_RELEASE_STATS:
   rc = vfe_release_stats(vfe_ctrl_obj);
   if(rc != VFE_SUCCESS)
     CDBG_ERROR("%s: VFE_RELEASE_STATS failed\n", __func__);
   break;

  default:
    CDBG_ERROR("%s: Invalid mode \n", __func__);
    rc = VFE_ERROR_GENERAL;
    break;
  }
  if(rc > 0) {
    rc = -rc;
  }
  return rc;
} /* vfe_process */

/*===========================================================================
 * FUNCTION    - vfe_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_set_params(void *client, int type, void *parm_in,
  void *parm_out)
{
  vfe_client_t *vfe_client = (vfe_client_t *)client;
  vfe_ctrl_info_t *vfe_ctrl_obj;
  cam_ctrl_dimension_t *In_dim, *Op_Dim;
  vfe_params_t *params = NULL;
  int rc = VFE_SUCCESS;

  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    return -1;
  }
  if(type == VFE_PARM_HW_VERSION) {
    vfe_client->vfe_version = *((uint32_t *)parm_in);
    return 0;
  } else if(type == VFE_PARM_ADD_OBJ_ID) {
    rc = vfe_add_instance(vfe_client, type, parm_in, parm_out);
    return rc;
  }
  vfe_ctrl_obj = vfe_get_obj(vfe_client);
  CDBG("%s: parm type =%d", __func__, type);
  if(!vfe_ctrl_obj) {
    CDBG_ERROR("%s: no VFE OBJ associated with clientr",  __func__);
    return -1;
  }
  params = &vfe_ctrl_obj->vfe_params;
  switch(type) {
  case VFE_SET_SENSOR_PARM: {
    vfe_sensor_params_t vfe_sensor_params;
    sensor_get_t sensor_get;

    sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_PREVIEW;
    rc = vfe_client->ops->fetch_params(
                         vfe_client->my_comp_id, vfe_client->ops->parent,
                         ((MCTL_COMPID_SENSOR << 24)|SENSOR_GET_DIM_INFO),
                         &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_HIGH("%s: sensor_get_params failed %d\n", __func__, rc);
      return rc;
    }
    vfe_sensor_params.qtr_size_width  = sensor_get.data.sensor_dim.width;
    vfe_sensor_params.qtr_size_height = sensor_get.data.sensor_dim.height;

    sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_SNAPSHOT;
    rc = vfe_client->ops->fetch_params(
                         vfe_client->my_comp_id, vfe_client->ops->parent,
                         ((MCTL_COMPID_SENSOR << 24)|SENSOR_GET_DIM_INFO),
                         &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_HIGH("%s: sensor_get_params failed %d\n", __func__, rc);
      return rc;
    }
    vfe_sensor_params.full_size_width  = sensor_get.data.sensor_dim.width;
    vfe_sensor_params.full_size_height = sensor_get.data.sensor_dim.height;

    rc = vfe_client->ops->fetch_params(
                         vfe_client->my_comp_id, vfe_client->ops->parent,
                         ((MCTL_COMPID_SENSOR << 24)|SENSOR_GET_OUTPUT_CFG),
                         &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG("%s: sensor_get_params failed %d\n", __func__, rc);
      return rc;
    }
    vfe_sensor_params.vfe_snsr_fmt  =
      sensor_get.data.sensor_output.output_format;
    vfe_sensor_params.vfe_raw_depth =
      sensor_get.data.sensor_output.raw_output;
    vfe_sensor_params.vfe_snsr_conn_mode =
      sensor_get.data.sensor_output.connection_mode;

    rc = vfe_client->ops->fetch_params(
                         vfe_client->my_comp_id, vfe_client->ops->parent,
                         ((MCTL_COMPID_SENSOR << 24)|SENSOR_GET_CAMIF_CFG),
                         &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG("%s: sensor_get_params failed %d\n", __func__, rc);
      return rc;
    }
    vfe_sensor_params.sensor_width     = sensor_get.data.camif_setting.width;
    vfe_sensor_params.sensor_height    = sensor_get.data.camif_setting.height;
    vfe_sensor_params.lastLine         = sensor_get.data.camif_setting.last_line;
    vfe_sensor_params.firstLine        = sensor_get.data.camif_setting.first_line;
    vfe_sensor_params.lastPixel        = sensor_get.data.camif_setting.last_pixel;
    vfe_sensor_params.firstPixel       = sensor_get.data.camif_setting.first_pixel;
    vfe_sensor_params.vfe_camif_in_fmt = sensor_get.data.camif_setting.format;

    vfe_ctrl_obj->vfe_params.sensor_parms = vfe_sensor_params;
    break;
  }
  case VFE_SET_CHROMATIX_PARM:
    vfe_ctrl_obj->vfe_params.chroma3a = (chromatix_parms_type *)parm_in;
    break;
  case VFE_SET_CAMERA_MODE:
    vfe_ctrl_obj->vfe_params.cam_mode = *(cam_mode_t *)parm_in;
    break;

  case VFE_SET_EFFECTS: {
    vfe_effects_type_t *effect_type = (vfe_effects_type_t *)parm_in;
    vfe_effects_params_t *effects_param = (vfe_effects_params_t *)parm_out;
    vfe_ctrl_obj->vfe_params.effects_params = *effects_param;
    rc = vfe_set_effect(vfe_ctrl_obj, *effect_type, effects_param);
    break;
  }

  case VFE_SET_BESTSHOT: {
    camera_bestshot_mode_type *bs_type = (camera_bestshot_mode_type *)parm_in;
    rc = vfe_set_bestshot(vfe_ctrl_obj, *bs_type);
    if(!rc)
      vfe_ctrl_obj->vfe_params.bs_mode = *bs_type;
    break;
  }

  case VFE_SET_AEC_PARAMS: {
#ifdef FEATURE_VFE_TEST_VECTOR
  if (vfe_test_vector_enabled(&vfe_ctrl_obj->test_vector))
    return VFE_SUCCESS;
#endif
    stats_proc_get_t stats_proc_get;
    stats_proc_get.d.get_aec.type = AEC_PARMS;
    rc = vfe_client->ops->fetch_params(
                         vfe_client->my_comp_id, vfe_client->ops->parent,
                         ((MCTL_COMPID_STATSPROC << 24)|STATS_PROC_AEC_TYPE),
                         &stats_proc_get, sizeof(stats_proc_get));
    if (rc < 0) {
      CDBG_HIGH("%s: stats_proc fetch parms failed %d\n", __func__, rc);
      return rc;
    }
    vfe_ctrl_obj->vfe_params.aec_params.cur_luma =
      stats_proc_get.d.get_aec.d.aec_parms.cur_luma;
    vfe_ctrl_obj->vfe_params.aec_params.cur_real_gain =
      stats_proc_get.d.get_aec.d.aec_parms.cur_real_gain;
    vfe_ctrl_obj->vfe_params.aec_params.exp_index =
      stats_proc_get.d.get_aec.d.aec_parms.exp_index;
    vfe_ctrl_obj->vfe_params.aec_params.exp_tbl_val =
      stats_proc_get.d.get_aec.d.aec_parms.exp_tbl_val;
    vfe_ctrl_obj->vfe_params.aec_params.lux_idx =
      stats_proc_get.d.get_aec.d.aec_parms.lux_idx;
    vfe_ctrl_obj->vfe_params.aec_params.snapshot_real_gain =
      stats_proc_get.d.get_aec.d.aec_parms.snapshot_real_gain;
    vfe_ctrl_obj->vfe_params.aec_params.target_luma =
      stats_proc_get.d.get_aec.d.aec_parms.target_luma;
    break;
  }
  case VFE_SET_AWB_PARMS: {
#ifdef FEATURE_VFE_TEST_VECTOR
  if (vfe_test_vector_enabled(&vfe_ctrl_obj->test_vector))
    break; //return VFE_SUCCESS;
#endif
    stats_proc_get_t stats_proc_get;
    uint32_t color_temp = vfe_ctrl_obj->vfe_params.awb_params.color_temp;
    stats_proc_get.d.get_awb.type = AWB_PARMS;
    rc = vfe_client->ops->fetch_params(
                         vfe_client->my_comp_id, vfe_client->ops->parent,
                         ((MCTL_COMPID_STATSPROC << 24)|STATS_PROC_AWB_TYPE),
                         &stats_proc_get, sizeof(stats_proc_get));
    if (rc < 0) {
      CDBG_HIGH("%s: stats_proc fetch parms failed %d\n", __func__, rc);
      return rc;
    }
    vfe_ctrl_obj->vfe_params.awb_params.bounding_box =
       stats_proc_get.d.get_awb.d.awb_params.bounding_box;
    vfe_ctrl_obj->vfe_params.awb_params.color_temp =
       stats_proc_get.d.get_awb.d.awb_params.color_temp;
    vfe_ctrl_obj->vfe_params.awb_params.exterme_col_param.t1 =
       stats_proc_get.d.get_awb.d.awb_params.exterme_col_param.t1;
    vfe_ctrl_obj->vfe_params.awb_params.exterme_col_param.t2 =
       stats_proc_get.d.get_awb.d.awb_params.exterme_col_param.t2;
    vfe_ctrl_obj->vfe_params.awb_params.exterme_col_param.t3 =
       stats_proc_get.d.get_awb.d.awb_params.exterme_col_param.t3;
    vfe_ctrl_obj->vfe_params.awb_params.exterme_col_param.t4 =
       stats_proc_get.d.get_awb.d.awb_params.exterme_col_param.t4;
    vfe_ctrl_obj->vfe_params.awb_params.exterme_col_param.t5 =
       stats_proc_get.d.get_awb.d.awb_params.exterme_col_param.t5;
    vfe_ctrl_obj->vfe_params.awb_params.exterme_col_param.t6 =
       stats_proc_get.d.get_awb.d.awb_params.exterme_col_param.t6;
    vfe_ctrl_obj->vfe_params.awb_params.gain =
       stats_proc_get.d.get_awb.d.awb_params.gain;

    if(vfe_ctrl_obj->vfe_params.awb_params.color_temp == 0) {
      vfe_ctrl_obj->vfe_params.awb_params.color_temp = color_temp;
    }
    break;
  }
  case VFE_SET_ASD_PARMS: {
    stats_proc_get_t stats_proc_get;
    stats_proc_get.d.get_asd.type = ASD_PARMS;
    rc = vfe_client->ops->fetch_params(
                         vfe_client->my_comp_id, vfe_client->ops->parent,
                         ((MCTL_COMPID_STATSPROC << 24)|STATS_PROC_ASD_TYPE),
                         &stats_proc_get, sizeof(stats_proc_get));
    if (rc < 0) {
      CDBG_HIGH("%s: stats_proc fetch parms failed %d\n", __func__, rc);
      return rc;
    }
    vfe_ctrl_obj->vfe_params.asd_params.asd_soft_focus_dgr =
      stats_proc_get.d.get_asd.d.asd_params.asd_soft_focus_dgr;
    vfe_ctrl_obj->vfe_params.asd_params.backlight_detected =
      stats_proc_get.d.get_asd.d.asd_params.backlight_detected;
    vfe_ctrl_obj->vfe_params.asd_params.backlight_scene_severity =
      stats_proc_get.d.get_asd.d.asd_params.backlight_scene_severity;
    vfe_ctrl_obj->vfe_params.asd_params.landscape_severity =
      stats_proc_get.d.get_asd.d.asd_params.landscape_severity;
    vfe_ctrl_obj->vfe_params.asd_params.portrait_severity =
      stats_proc_get.d.get_asd.d.asd_params.portrait_severity;

    vfe_ctrl_obj->vfe_params.sharpness_info.asd_soft_focus_dgr =
      stats_proc_get.d.get_asd.d.asd_params.asd_soft_focus_dgr;
    vfe_ctrl_obj->vfe_params.sharpness_info.portrait_severity =
      (float)stats_proc_get.d.get_asd.d.asd_params.portrait_severity;
    break;
  }
  case VFE_SET_SENSOR_DIG_GAIN: {
#ifdef FEATURE_VFE_TEST_VECTOR
  if (vfe_test_vector_enabled(&vfe_ctrl_obj->test_vector))
    break; //return VFE_SUCCESS;
#endif
    float *p_dig_gain = (float *)parm_in;
    vfe_ctrl_obj->vfe_params.digital_gain = MIN(1.0, *p_dig_gain);
    break;
  }
  case VFE_SET_FLASH_PARMS: {
#ifdef FEATURE_VFE_TEST_VECTOR
  if (vfe_test_vector_enabled(&vfe_ctrl_obj->test_vector))
    break; //return VFE_SUCCESS;
#endif
    stats_proc_get_t stats_proc_get;
    stats_proc_get.d.get_aec.type = AEC_FLASH_DATA;
    rc = vfe_client->ops->fetch_params(
                         vfe_client->my_comp_id, vfe_client->ops->parent,
                         ((MCTL_COMPID_STATSPROC << 24)|STATS_PROC_AEC_TYPE),
                         &stats_proc_get, sizeof(stats_proc_get));
    if (rc < 0) {
      CDBG_HIGH("%s: stats_proc fetch parms failed %d\n", __func__, rc);
      return rc;
    }
    vfe_ctrl_obj->vfe_params.flash_params.flash_mode =
      stats_proc_get.d.get_aec.d.flash_parms.flash_mode;
    vfe_ctrl_obj->vfe_params.flash_params.sensitivity_led_hi =
      stats_proc_get.d.get_aec.d.flash_parms.sensitivity_led_hi;
    vfe_ctrl_obj->vfe_params.flash_params.sensitivity_led_low =
      stats_proc_get.d.get_aec.d.flash_parms.sensitivity_led_low;
    vfe_ctrl_obj->vfe_params.flash_params.sensitivity_led_off =
      stats_proc_get.d.get_aec.d.flash_parms.sensitivity_led_off;
    vfe_ctrl_obj->vfe_params.flash_params.strobe_duration =
      stats_proc_get.d.get_aec.d.flash_parms.strobe_duration;
    vfe_ctrl_obj->vfe_params.flash_params.sensitivity_led_hi =
      MAX(1, vfe_ctrl_obj->vfe_params.flash_params.sensitivity_led_hi);
    CDBG("%s: sensitivity high %d", __func__,
      vfe_ctrl_obj->vfe_params.flash_params.sensitivity_led_hi);
    break;
  }
  case VFE_SET_EZTUNE: {
    ez_vfe_set(vfe_ctrl_obj, parm_in, parm_out);
    break;
  }
  case VFE_SET_SHARPNESS_PARMS: {
    float *sharp_parm = (float *)parm_in;
    vfe_ctrl_obj->vfe_params.sharpness_info.ui_sharp_ctrl_factor = *sharp_parm;
    break;
  }
  case VFE_SET_WB: {
    vfe_ctrl_obj->vfe_params.wb = *((config3a_wb_t *)parm_in);
    if(vfe_ctrl_obj->vfe_params.wb != CAMERA_WB_AUTO) {
      stats_proc_get_t stats_proc_get;
      stats_proc_get.d.get_awb.type = AWB_GAINS;
      rc = vfe_client->ops->fetch_params(
                       vfe_client->my_comp_id, vfe_client->ops->parent,
                       ((MCTL_COMPID_STATSPROC << 24)|STATS_PROC_AWB_TYPE),
                       &stats_proc_get, sizeof(stats_proc_get));
      if (rc < 0) {
        CDBG_HIGH("%s: stats_proc fetch AWB Gains failed %d\n", __func__, rc);
        return rc;
      }
      vfe_ctrl_obj->vfe_params.awb_params.gain =
        stats_proc_get.d.get_awb.d.awb_gains.curr_gains;
      CDBG_HIGH("%s: Whitebalance gains for %d R = %f G = %f B = %f", __func__,
            vfe_ctrl_obj->vfe_params.wb,
            vfe_ctrl_obj->vfe_params.awb_params.gain.r_gain,
            vfe_ctrl_obj->vfe_params.awb_params.gain.g_gain,
            vfe_ctrl_obj->vfe_params.awb_params.gain.b_gain);
    }
    rc = vfe_set_manual_wb(vfe_ctrl_obj);
    break;
  }
  case VFE_SET_FOV_CROP_FACTOR: {
    uint32_t *p_crop_factor = (uint32_t *)parm_in;
    rc = 0;
    vfe_ctrl_obj->vfe_params.crop_factor = *p_crop_factor;
    rc = vfe_update_crop(vfe_ctrl_obj, NULL);
    CDBG("%s: crop_factor %d, rc = %d", __func__, params->crop_factor, rc);
    break;
  }
  case VFE_SET_CAMIF_DIM: {
    pixel_crop_info_t demosaic_crop_info;
    camif_output_t camif_output;
    rc = vfe_client->ops->fetch_params(
                        vfe_client->my_comp_id, vfe_client->ops->parent,
                        ((MCTL_COMPID_CAMIF << 24)|CAMIF_PARAMS_CAMIF_DIMENSION),
                        (void *)&camif_output, sizeof(camif_output));
    if (rc < 0) {
      CDBG_HIGH("%s Fetch params error ", __func__);
      return rc;
    }
    vfe_ctrl_obj->vfe_params.vfe_input_win.first_pixel =
                                camif_output.d.camif_window.first_pixel;
    vfe_ctrl_obj->vfe_params.vfe_input_win.last_pixel  =
                                camif_output.d.camif_window.last_pixel;
    vfe_ctrl_obj->vfe_params.vfe_input_win.first_line  =
                                camif_output.d.camif_window.first_line;
    vfe_ctrl_obj->vfe_params.vfe_input_win.last_line   =
                                camif_output.d.camif_window.last_line;
    vfe_ctrl_obj->vfe_params.vfe_input_win.width =
                   vfe_ctrl_obj->vfe_params.vfe_input_win.last_pixel -
                   vfe_ctrl_obj->vfe_params.vfe_input_win.first_pixel + 1;
    vfe_ctrl_obj->vfe_params.vfe_input_win.height =
                   vfe_ctrl_obj->vfe_params.vfe_input_win.last_line -
                   vfe_ctrl_obj->vfe_params.vfe_input_win.first_line + 1;

    demosaic_crop_info.first_pixel = vfe_ctrl_obj->vfe_params.vfe_input_win.first_pixel;
    demosaic_crop_info.last_pixel  = vfe_ctrl_obj->vfe_params.vfe_input_win.last_pixel;
    demosaic_crop_info.first_line  = vfe_ctrl_obj->vfe_params.vfe_input_win.first_line;
    demosaic_crop_info.last_line   = vfe_ctrl_obj->vfe_params.vfe_input_win.last_line;
    vfe_set_pixel_crop_info(vfe_ctrl_obj, &demosaic_crop_info);
    break;
  }
  case VFE_SET_MCE: {
    uint32_t *mce_enable = (uint32_t *)parm_in;
    rc = vfe_set_mce(vfe_ctrl_obj, *mce_enable);
    break;
  }
  case VFE_SET_SCE: {
    int32_t *sce_factor = (int32_t *)parm_in;
    rc = vfe_set_sce(vfe_ctrl_obj, *sce_factor);
    break;
  }
  case VFE_SET_OUTPUT_INFO: {
    rc = vfe_set_output_info(vfe_ctrl_obj, parm_in);
    break;
  }
  case VFE_SET_FRAME_SKIP: {
    vfe_frame_skip *frm_skip = (vfe_frame_skip *)parm_in;
    rc = vfe_set_frame_skip_pattern(vfe_ctrl_obj, frm_skip);
    break;
  }
  case VFE_SET_HFR_MODE: {
    vfe_hfr_info_t  *hfr_info = (vfe_hfr_info_t *)parm_in;
    rc = vfe_set_hfr_mode(vfe_ctrl_obj, hfr_info);
    break;
  }
  case VFE_SET_STATS_VERSION: {
    break;
  }
  default:
    CDBG_ERROR("Invalid parm");
    rc = -VFE_ERROR_GENERAL;
    break;
  }
  if(rc > 0) {
    rc = -rc;
    CDBG_ERROR("%s: param_type = %d, error = %d",  __func__, type, rc);
  }
  return rc;
} /* vfe_set_params */

/*===========================================================================
 * FUNCTION    - vfe_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_get_params(void *client, int parm_type,
  void *parm, int parm_len)
{
  vfe_client_t *vfe_client = (vfe_client_t *)client;
  vfe_ctrl_info_t *vfe_ctrl_obj;
  int rc = VFE_SUCCESS;

  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    return -1;
  }
  vfe_ctrl_obj = vfe_get_obj(vfe_client);

  CDBG("%s: parm type =%d", __func__, parm_type);
  if(!vfe_ctrl_obj) {
    CDBG_ERROR("%s: no VFE OBJ associated with clientr",  __func__);
    return -1;
  }

  switch(parm_type) {
  case VFE_GET_STATS_BUF_PTR:
    rc = vfe_query_stats_buf_ptr(vfe_ctrl_obj, parm_type, parm);
  break;
  case VFE_GET_ZOOM_CROP_INFO:
  case VFE_GET_FOV_CROP_PARM:
    rc = vfe_query_fov_crop(vfe_ctrl_obj, parm_type, parm);
    break;
  case VFE_GET_CAMIF_PARM:
    break;
  case VFE_GET_OUTPUT_DIMENSION: {
      vfe_axi_output_dim_t *dim = (vfe_axi_output_dim_t *)parm;
      memset(dim, 0, sizeof(vfe_axi_output_dim_t));
      if(vfe_ctrl_obj->vfe_params.vfe_op_mode == VFE_OP_MODE_PREVIEW ||
         vfe_ctrl_obj->vfe_params.vfe_op_mode == VFE_OP_MODE_RAW_SNAPSHOT) {
        dim->output2w = vfe_ctrl_obj->vfe_params.output2w;
        dim->output2h = vfe_ctrl_obj->vfe_params.output2h;
      } else if(vfe_ctrl_obj->vfe_params.vfe_op_mode == VFE_OP_MODE_VIDEO ||
          vfe_ctrl_obj->vfe_params.vfe_op_mode == VFE_OP_MODE_ZSL ||
          vfe_ctrl_obj->vfe_params.vfe_op_mode == VFE_OP_MODE_SNAPSHOT ||
          vfe_ctrl_obj->vfe_params.vfe_op_mode == VFE_OP_MODE_JPEG_SNAPSHOT) {
        dim->output1w = vfe_ctrl_obj->vfe_params.output1w;
        dim->output1h = vfe_ctrl_obj->vfe_params.output1h;
        dim->output2w = vfe_ctrl_obj->vfe_params.output2w;
        dim->output2h = vfe_ctrl_obj->vfe_params.output2h;
      }
    }
    break;
  case VFE_GET_AWB_SHIFT_BITS:
  case VFE_GET_IHIST_SHIFT_BITS:
  case VFE_GET_AEC_SHIFT_BITS:
    rc = vfe_query_shift_bits(vfe_ctrl_obj, parm_type, parm);
    break;
  case VFE_GET_RS_CS_PARM:
    rc = vfe_query_rs_cs_parm(vfe_ctrl_obj, parm_type, parm);
    break;
  case VFE_GET_PIXEL_CROP_INFO:
    rc = vfe_calc_pixel_crop_info(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_PIXEL_SKIP_INFO:
    rc = vfe_calc_pixel_skip_info(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_DIAGNOSTICS_PTR: {
    vfe_eztune_info_t *p_info = (vfe_eztune_info_t *)parm;
    p_info->diag_ptr = (void *)&(vfe_ctrl_obj->vfe_diag);
    break;
    }
  case VFE_GET_PP_INFO:
    rc = vfe_pp_info(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_BLK_INC_COMP:
    rc = vfe_blk_inc_comp(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_GAMMA_RECONFIG_VFE:
    rc = is_vfe_gamma_reconfig(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_ROLLOFF_RECONFIG_VFE:
    rc = is_vfe_rolloff_reconfig(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_BLACK_LEVEL_RECONFIG_VFE:
    rc = is_vfe_black_level_reconfig(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_5X5ASF_RECONFIG_VFE:
    rc = is_vfe_asf_reconfig(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_STATS_CONF_ENABLE:
    rc = vfe_is_stats_conf_enabled(vfe_ctrl_obj, parm);
    break;
  case VFE_GET_RESIDUE_DIG_GAIN: {
    float *p_dig_gain = (float *)parm;
    *p_dig_gain = vfe_ctrl_obj->vfe_params.aec_gain_adj.cc_gain_adj
      * vfe_ctrl_obj->vfe_params.aec_gain_adj.wb_gain_adj;
    *p_dig_gain = MAX(1.0, *p_dig_gain);
    break;
  }
  default:
    CDBG_ERROR("%s: Invalid mode \n", __func__);
    rc = VFE_ERROR_GENERAL;
    break;
  }
  if(rc > 0) {
    rc = -rc;
  }
  return rc;
} /* vfe_get_params */

/*===========================================================================
 * FUNCTION    - vfe_destroy -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_destroy(void *client)
{
  int idx;
  vfe_client_t *vfe_client = (vfe_client_t *)client;
  vfe_ctrl_info_t *vfe_ctrl_obj;
  int rc = VFE_SUCCESS;

  CDBG("%s: E", __func__);
  if(!vfe_client) {
    CDBG_ERROR("%s: null vfe client",  __func__);
    return -1;
  }
  vfe_ctrl_obj = vfe_get_obj(vfe_client);

  if(!vfe_ctrl_obj) {
    CDBG_ERROR("%s: no VFE OBJ associated with clientr",  __func__);
    goto end;
  }

#ifdef FEATURE_VFE_TEST_VECTOR
  if (0 != (rc = vfe_test_vector_deinit(&vfe_ctrl_obj->test_vector)))
    CDBG_ERROR("%s: vfe_test_vector_deinit err = %d", __func__, rc);
#endif

  if (0 != (rc = vfe_modules_deinit(&(vfe_ctrl_obj->vfe_module),
    &vfe_ctrl_obj->vfe_params))) {
    CDBG_ERROR("%s: vfe_modules_deinit err = %d\n", __func__, rc);
  }
  if (vfe_ctrl_obj->vfe_params.sensor_parms.vfe_snsr_fmt != SENSOR_YCBCR)
    if(VFE_SUCCESS != vfe_stats_buffer_free(vfe_ctrl_obj))
      CDBG_ERROR("%s: vfe_stats_buffer_free error", __func__);
end:
  return rc;
} /* vfe_destroy */
/*===========================================================================
 * FUNCTION    - vfe_parse_stats -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_parse_stats(void *client,
  int isp_started, stats_type_t type, void *stats, void *stats_output)
{
  vfe_client_t *vfe_client = (vfe_client_t *)client;
  vfe_ctrl_info_t *p_obj;
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) stats;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);
  vfe_stats_output_t *vfe_output = stats_output;

  int rc = VFE_SUCCESS;

  if(!vfe_client) {
    CDBG_ERROR("%s: vfe client is NULL",  __func__);
    return -1;
  }
  p_obj = vfe_get_obj(vfe_client);

  if(!p_obj) {
    CDBG_ERROR("%s: no VFE OBJ associated with clientr",  __func__);
    return -1;
  }

  CDBG("%s: E", __func__);

  switch (adsp->msg_id) {
    case MSG_ID_STATS_AWB_AEC:
      rc = vfe_stats_proc_MSG_ID_STATS_WB_EXP(p_obj, isp_started,
             type, adsp, vfe_output);
      break;
    case MSG_ID_STATS_AF:
      rc = vfe_stats_proc_MSG_ID_STATS_AF(p_obj, isp_started,
             type, adsp, vfe_output);
      break;
    default:
      rc = -1;
      break;
  } /*switch (adsp->msg_id) */

  CDBG("%s: X", __func__);
  return rc;
}/* vfe_parse_stats */

/*===========================================================================
 * FUNCTION    - vfe_ops_init -
 * DESCRIPTION:
 * DEPENDENCY: This function is called in vfe_client_init.
 *==========================================================================*/
vfe_status_t vfe_ops_init(vfe_client_t* vfe_client)
{
  CDBG_HIGH("%s: E", __func__);
  if(!vfe_client) {
    CDBG_ERROR("%s: vfe client is null", __func__);
    return -1;
  }

  vfe_client->vfe_ops.process = vfe_process;
  vfe_client->vfe_ops.set_param = vfe_set_params;
  vfe_client->vfe_ops.get_param = vfe_get_params;
  vfe_client->vfe_ops.parse_stats = vfe_parse_stats;
  vfe_client->vfe_ops.destroy = vfe_destroy;

  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - vfe_ops_deinit -
 * DESCRIPTION:
 * DEPENDENCY: This function is called in vfe_client_init.
 *==========================================================================*/
vfe_status_t vfe_ops_deinit(vfe_client_t* vfe_client)
{
  CDBG_HIGH("%s: E", __func__);
  if(!vfe_client) {
    CDBG_ERROR("%s: vfe client is null", __func__);
    return -1;
  }
  memset(&(vfe_client->vfe_ops), 0 , sizeof(vfe_ops_t));
  return VFE_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - vfe_release_stats -
 * DESCRIPTION:
 * DEPENDENCY: This function is called in vfe_client_init.
 *==========================================================================*/
vfe_status_t vfe_release_stats(vfe_ctrl_info_t *vfe_ctrl_obj)
{
  vfe_status_t status = VFE_SUCCESS;
  vfe_params_t *params = (vfe_params_t *)&(vfe_ctrl_obj->vfe_params);

  if (IS_BAYER_FORMAT(params))
    vfe_release_all_stats_buff(vfe_ctrl_obj);

  return status;
}
