/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "vfe.h"

/*===========================================================================
 * FUNCTION    - vfe_frame_skip_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_frame_skip_init(int mod_id, void *mod_fs, void *parm)
{
  frame_skip_mod_t *mod = (frame_skip_mod_t *)mod_fs;
  vfe_params_t* parms = (vfe_params_t *)parm;
  mod->frame_skip_cmd.output2YPeriod     = 31;
  mod->frame_skip_cmd.output2CbCrPeriod  = 31;
  mod->frame_skip_cmd.output2YPattern    = 0xffffffff;
  mod->frame_skip_cmd.output2CbCrPattern = 0xffffffff;
  mod->frame_skip_cmd.output1YPeriod     = 31;
  mod->frame_skip_cmd.output1CbCrPeriod  = 31;
  mod->frame_skip_cmd.output1YPattern    = 0xffffffff;
  mod->frame_skip_cmd.output1CbCrPattern = 0xffffffff;

  mod->fs_change = FALSE;
  return VFE_SUCCESS;
} /* vfe_frame_skip_init */

/*===========================================================================
 * FUNCTION.   - vfe_frame_skip_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_frame_skip_enable(int mod_id, void *mod_fs, void *parm,
  int8_t enable, int8_t hw_write)
{
  CDBG("%s, enable/disable frame skip module = %d",__func__, enable);
  frame_skip_mod_t *mod = (frame_skip_mod_t *)mod_fs;
  vfe_params_t* p_obj = (vfe_params_t *)parm;

  if (hw_write && (mod->fs_enable == enable))
    return VFE_SUCCESS;

  mod->fs_enable = enable;
  if (hw_write) {
    p_obj->current_config = (enable) ?
      (p_obj->current_config | VFE_MOD_FRAME_SKIP)
      : (p_obj->current_config & ~VFE_MOD_FRAME_SKIP);
  }
  return VFE_SUCCESS;
} /* vfe_frame_skip_enable */

/*===========================================================================
 * FUNCTION    - vfe_frame_skip_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_frame_skip_config(int mod_id, void *mod_fs, void *parm)
{
  vfe_status_t status = VFE_SUCCESS;
  frame_skip_mod_t *mod = (frame_skip_mod_t *)mod_fs;
  vfe_params_t *p_obj = (vfe_params_t *)parm;
  if (!mod->fs_enable) {
    CDBG("%s: Frameskip not enabled", __func__);
    return status;
  }
  switch(p_obj->cam_mode) {
    case CAM_MODE_2D_HFR:
      break;
    case CAM_MODE_3D:
      break;
    case CAM_MODE_2D:
       break;
    default:
      CDBG_ERROR("%s, Invalid camera mode %d \n",__func__,
        p_obj->cam_mode);
      status = VFE_ERROR_GENERAL;
  }
  if (p_obj->vfe_op_mode == VFE_OP_MODE_ZSL) {
    mod->frame_skip_cmd.output2YPeriod     = 1;
    mod->frame_skip_cmd.output2CbCrPeriod  = 1;
    mod->frame_skip_cmd.output2YPattern    = 1;
    mod->frame_skip_cmd.output2CbCrPattern = 1;
    mod->frame_skip_cmd.output1YPeriod     = 31;
    mod->frame_skip_cmd.output1CbCrPeriod  = 31;
    mod->frame_skip_cmd.output1YPattern    = 0xffffffff;
    mod->frame_skip_cmd.output1CbCrPattern = 0xffffffff;
  } else if(mod->fs_change == TRUE) {
    mod->frame_skip_cmd = mod->ext_frame_skip_cmd;
    mod->fs_change = FALSE;
  } else {
    mod->frame_skip_cmd.output2YPeriod     = 31;
    mod->frame_skip_cmd.output2CbCrPeriod  = 31;
    mod->frame_skip_cmd.output2YPattern    = 0xffffffff;
    mod->frame_skip_cmd.output2CbCrPattern = 0xffffffff;
    mod->frame_skip_cmd.output1YPeriod     = 31;
    mod->frame_skip_cmd.output1CbCrPeriod  = 31;
    mod->frame_skip_cmd.output1YPattern    = 0xffffffff;
    mod->frame_skip_cmd.output1CbCrPattern = 0xffffffff;
  }
  if (VFE_SUCCESS != vfe_util_write_hw_cmd(p_obj->camfd, CMD_GENERAL,
    (void *) &(mod->frame_skip_cmd), sizeof(mod->frame_skip_cmd),
    VFE_CMD_FRAME_SKIP_CFG)) {
    CDBG_HIGH("%s: FRAME_SKIP cmd failed for mode = %d failed\n", __func__,
    p_obj->vfe_op_mode);
    status = VFE_ERROR_GENERAL;
  }
  return status;
} /* vfe_frame_skip_config */

/*===========================================================================
 * FUNCTION    - vfe_frame_skip_config_pattern -
 *
 * DESCRIPTION: This function updates the shadow frame skip command
 *==========================================================================*/
vfe_status_t vfe_frame_skip_config_pattern(int mod_id, void *mod_fs,
  void *parm)
{
  frame_skip_mod_t *mod = (frame_skip_mod_t *)mod_fs;
  vfe_frame_skip *ext = (vfe_frame_skip *)parm;

  mod->fs_change = TRUE;
  //frame skip for video path/ main image path
  mod->ext_frame_skip_cmd.output2YPeriod     = ext->output2period;
  mod->ext_frame_skip_cmd.output2CbCrPeriod  = ext->output2period;
  mod->ext_frame_skip_cmd.output2YPattern    = ext->output2pattern;
  mod->ext_frame_skip_cmd.output2CbCrPattern = ext->output2pattern;
  //frame skip for preview/ thumbnail path
  mod->ext_frame_skip_cmd.output1YPeriod     = ext->output1period;
  mod->ext_frame_skip_cmd.output1CbCrPeriod  = ext->output1period;
  mod->ext_frame_skip_cmd.output1YPattern    = ext->output1pattern;
  mod->ext_frame_skip_cmd.output1CbCrPattern = ext->output1pattern;

  return VFE_SUCCESS;
} /* vfe_frame_skip_set_hdr_pattern */


/*===========================================================================
 * FUNCTION    - vfe_frame_skip_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_frame_skip_deinit(int mod_id, void *mod_fs, void *parm)
{
  return VFE_SUCCESS;
} /* vfe_frame_skip_deinit */

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_frame_skip_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_frame_skip_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  frame_skip_module_t *cmd = (frame_skip_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->frame_skip_cmd),
     sizeof(VFE_FrameSkipConfigCmdType),
     VFE_CMD_FRAME_SKIP_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_frame_skip_plugin_update */
#endif
