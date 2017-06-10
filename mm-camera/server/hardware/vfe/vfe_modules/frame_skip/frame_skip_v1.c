/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include <math.h>
#include "frame_skip_v1.h"


/*===========================================================================
 * FUNCTION    - vfe_frame_skip_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_frame_skip_init(frame_skip_mod_t *mod, vfe_params_t* parms)
{
  mod->frame_skip_cmd.output2Pattern    = 0x3fffffff;
  mod->frame_skip_cmd.output1Pattern    = 0x3fffffff;
  return VFE_SUCCESS;
} /* vfe_frame_skip_init */

/*===========================================================================
 * FUNCTION.   - vfe_frame_skip_enable -
 *
 * DESCRIPTION
 *========================================================================*/
vfe_status_t vfe_frame_skip_enable(frame_skip_mod_t *mod, vfe_params_t *p_obj,
  int8_t enable, int8_t hw_write)
{
  CDBG("%s, enable/disable frame skip module = %d",__func__, enable);

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
vfe_status_t vfe_frame_skip_config(frame_skip_mod_t *mod, vfe_params_t *p_obj)
{
  vfe_status_t status = VFE_SUCCESS;
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
    mod->frame_skip_cmd.output2Pattern    = 0x3fffffff;
    mod->frame_skip_cmd.output1Pattern    = 0x3fffffff;
  } else if(mod->fs_change == TRUE) {
    CDBG_ERROR("%s:%d] Update frame skip pattern\n",__func__, __LINE__);
    mod->frame_skip_cmd = mod->ext_frame_skip_cmd;
    mod->fs_change = FALSE;
  } else {
    mod->frame_skip_cmd.output2Pattern    = 0x3fffffff;
    mod->frame_skip_cmd.output1Pattern    = 0x3fffffff;
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
vfe_status_t vfe_frame_skip_config_pattern(frame_skip_mod_t *mod,
  vfe_frame_skip *ext)
{
  mod->fs_change = TRUE;
  //frame skip for video path/ main image path
  mod->ext_frame_skip_cmd.output2Pattern    = ext->output2pattern;
  //frame skip for preview/ thumbnail path
  mod->ext_frame_skip_cmd.output1Pattern    = ext->output1pattern;

  return VFE_SUCCESS;
} /* vfe_frame_skip_set_hdr_pattern */
