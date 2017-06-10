/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_chroma_subsample_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_subsample_init(int mod_id, void *mod_cs,
  void* vparms)
{
  chroma_ss_mod_t *mod = (chroma_ss_mod_t *)mod_cs;
  vfe_params_t *parms = (vfe_params_t *)vparms;
#ifndef VFE_2X
  mod->Chroma_ss_cmd.hCositedPhase       = 0;
  mod->Chroma_ss_cmd.vCositedPhase       = 0;
  mod->Chroma_ss_cmd.cropEnable          =  0;
  mod->Chroma_ss_cmd.cropWidthFirstPixel =  0;
  mod->Chroma_ss_cmd.cropWidthLastPixel  =  0;
  mod->Chroma_ss_cmd.cropHeightFirstLine =  0;
  mod->Chroma_ss_cmd.cropHeightLastLine  =  0;
#endif
  mod->Chroma_ss_cmd.hCosited            = 0;
  mod->Chroma_ss_cmd.vCosited            = 0;
  mod->Chroma_ss_cmd.hsubSampleEnable    =  1;
  mod->Chroma_ss_cmd.vsubSampleEnable    =  1;

  return VFE_SUCCESS;
} /* vfe_chroma_subsample_init */

/*===========================================================================
 * FUNCTION.   - vfe_chroma_subsample_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_chroma_subsample_enable(int mod_id, void *mod_cs,
  void *vparm, int8_t enable, int8_t hw_write)
{
  chroma_ss_mod_t *mod = (chroma_ss_mod_t *)mod_cs;
  vfe_params_t *p_obj = (vfe_params_t *)vparm;
  CDBG("%s, enable/disable css module = %d",__func__, enable);
  if (hw_write && (mod->css_enable == enable))
    return VFE_SUCCESS;

  mod->css_enable = enable;
  if (hw_write) {
    p_obj->current_config = (enable) ?
      (p_obj->current_config | VFE_MOD_CHROMA_SS)
      : (p_obj->current_config & ~VFE_MOD_CHROMA_SS);
  }
  return VFE_SUCCESS;
} /* vfe_chroma_subsample_enable */

/*===========================================================================
 * FUNCTION    - vfe_chroma_subsample_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_subsample_config(int mod_id, void *mod_cs,
  void *vparm)
{
  chroma_ss_mod_t *mod = (chroma_ss_mod_t *)mod_cs;
  vfe_params_t *p_obj = (vfe_params_t *)vparm;
  if (!mod->css_enable) {
    CDBG("%s: Chroma SS not enabled %d", __func__, mod->css_enable);
    return VFE_SUCCESS;
  }

#ifndef VFE_2X
  mod->Chroma_ss_cmd.hCositedPhase       = 0;
  mod->Chroma_ss_cmd.vCositedPhase       = 0;
  if ((p_obj->vfe_op_mode == VFE_OP_MODE_SNAPSHOT ||
    p_obj->vfe_op_mode == VFE_OP_MODE_ZSL) &&
    (mod->main_format == CAMERA_YUV_422_NV61 ||
     mod->main_format == CAMERA_YUV_422_NV16))
    mod->Chroma_ss_cmd.hCosited            = 1;
  else
    mod->Chroma_ss_cmd.hCosited            = 0;
  mod->Chroma_ss_cmd.vCosited            = 0;
  mod->Chroma_ss_cmd.hsubSampleEnable    =  1;
  if ((p_obj->vfe_op_mode == VFE_OP_MODE_SNAPSHOT ||
    p_obj->vfe_op_mode == VFE_OP_MODE_ZSL) &&
    (mod->main_format == CAMERA_YUV_422_NV61 ||
    mod->main_format == CAMERA_YUV_422_NV16))
    mod->Chroma_ss_cmd.vsubSampleEnable    =  0;
  else
    mod->Chroma_ss_cmd.vsubSampleEnable    =  1;
  mod->Chroma_ss_cmd.cropEnable          =  0;
  mod->Chroma_ss_cmd.cropWidthFirstPixel =  0;
  mod->Chroma_ss_cmd.cropWidthLastPixel  =  0;
  mod->Chroma_ss_cmd.cropHeightFirstLine =  0;
  mod->Chroma_ss_cmd.cropHeightLastLine  =  0;
#else
  mod->Chroma_ss_cmd.hCosited            = 1;
  mod->Chroma_ss_cmd.vCosited            = 0;
  mod->Chroma_ss_cmd.hsubSampleEnable    =  0;
  mod->Chroma_ss_cmd.vsubSampleEnable    =  0;
#endif

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(p_obj->camfd,CMD_GENERAL,
    (void *) &(mod->Chroma_ss_cmd), sizeof(mod->Chroma_ss_cmd),
    VFE_CMD_CHROMA_SUBS_CFG)) {
    CDBG_HIGH("%s: Chroma SS config for operation mode = %d \
      failed\n", __func__, p_obj->vfe_op_mode);
    return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_chroma_subsample_config */

/*===========================================================================
 * FUNCTION    - vfe_chroma_subsample_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_subsample_deinit(int mod_id, void *mod,
  void *parm)
{
  return VFE_SUCCESS;
}

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_chroma_subsample_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_chroma_subsample_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  chroma_ss_module_t *cmd = (chroma_ss_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->Chroma_ss_cmd),
     sizeof(VFE_ChromaSubsampleConfigCmdType),
     VFE_CMD_CHROMA_SUBS_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
}
#endif
