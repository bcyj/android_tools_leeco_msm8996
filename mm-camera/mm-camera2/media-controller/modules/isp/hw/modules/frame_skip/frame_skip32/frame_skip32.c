/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "frame_skip32.h"
#include "isp_log.h"


/*===========================================================================
 * FUNCTION    - frame_skip_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int frame_skip_config(isp_frame_skip_mod_t *mod, isp_hw_pix_setting_params_t *in_params,
                     uint32_t in_param_size)
{
  int  rc = 0;
  uint32_t i;
  ISP_DBG(ISP_MOD_FRAME_SKIP, "%s\n",__func__);

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  /* if no frame skip needed teh period is zero */
  if (in_params->outputs[ISP_PIX_PATH_ENCODER].frame_skip_period == 0) {
     mod->frame_skip_cmd.output2YPeriod     = 31;
     mod->frame_skip_cmd.output2CbCrPeriod  = 31;
     mod->frame_skip_cmd.output2YPattern    = 0xffffffff;
     mod->frame_skip_cmd.output2CbCrPattern = 0xffffffff;
  } else {
     mod->frame_skip_cmd.output2YPeriod     = in_params->outputs[ISP_PIX_PATH_ENCODER].frame_skip_period;
     mod->frame_skip_cmd.output2CbCrPeriod  = in_params->outputs[ISP_PIX_PATH_ENCODER].frame_skip_period;
     mod->frame_skip_cmd.output2YPattern    = in_params->outputs[ISP_PIX_PATH_ENCODER].frame_skip_pattern;
     mod->frame_skip_cmd.output2CbCrPattern = in_params->outputs[ISP_PIX_PATH_ENCODER].frame_skip_pattern;
  }
  if (in_params->outputs[ISP_PIX_PATH_VIEWFINDER].frame_skip_period == 0) {
     mod->frame_skip_cmd.output1YPeriod     = 31;
     mod->frame_skip_cmd.output1CbCrPeriod  = 31;
     mod->frame_skip_cmd.output1YPattern    = 0xffffffff;
     mod->frame_skip_cmd.output1CbCrPattern = 0xffffffff;
  } else {
     mod->frame_skip_cmd.output1YPeriod     = in_params->outputs[ISP_PIX_PATH_VIEWFINDER].frame_skip_period;
     mod->frame_skip_cmd.output1CbCrPeriod  = in_params->outputs[ISP_PIX_PATH_VIEWFINDER].frame_skip_period;
     mod->frame_skip_cmd.output1YPattern    = in_params->outputs[ISP_PIX_PATH_VIEWFINDER].frame_skip_pattern;
     mod->frame_skip_cmd.output1CbCrPattern = in_params->outputs[ISP_PIX_PATH_VIEWFINDER].frame_skip_pattern;
  }

  return rc;
}

/* ============================================================
 * function name: fs_enable
 * description: enable fs
 * ============================================================*/
static int frame_skip_enable(isp_frame_skip_mod_t *fs,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  fs->fs_enable = enable->enable;

  return 0;
}

/* ============================================================
 * function name: frame_skip_trigger_enable
 * description: enable trigger update feature
 * ============================================================*/
static int frame_skip_trigger_enable(isp_frame_skip_mod_t *fs,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  fs->fs_trigger_enable = enable->enable;

  return 0;
}

/* ============================================================
 * function name: frame_skip_destroy
 * description: close fs
 * ============================================================*/
static int frame_skip_destroy (void *mod_ctrl)
{
  isp_frame_skip_mod_t *fs = mod_ctrl;

  memset(fs,  0,  sizeof(isp_frame_skip_mod_t));
  free(fs);
  return 0;
}

/* ============================================================
 * function name: frame_skip_set_params
 * description: set parameters
 * ============================================================*/
static int frame_skip_set_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_frame_skip_mod_t *fs = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = frame_skip_enable(fs, (isp_mod_set_enable_t *)in_params,
                     in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = frame_skip_config(fs, (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = frame_skip_trigger_enable(fs, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    //rc = frame_skip_trigger_update(fs, (isp_pix_trigger_update_input_t *)in_params, in_param_size);
    break;
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    break;
  }
  return rc;
}

/* ============================================================
 * function name: frame_skip_get_params
 * description: get parameters
 * ============================================================*/
static int frame_skip_get_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_frame_skip_mod_t *fs = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = fs->fs_enable;
    break;
  }
  default:
    rc = -EPERM;
    break;
  }
  return rc;
}

/* ============================================================
 * function name: frame_skip_do_hw_update
 * description: frame_skip_do_hw_update
 * ============================================================*/
static int frame_skip_do_hw_update(isp_frame_skip_mod_t *fs)
{
  int i, rc = 0;

  return rc;
}

/* ============================================================
 * function name: frame_skip_action
 * description: processing the action
 * ============================================================*/
static int frame_skip_action (void *mod_ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_frame_skip_mod_t *fs = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = frame_skip_do_hw_update(fs);
    break;
  default:
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
    break;
  }
  return rc;
}

/* ============================================================
 * function name: frame_skip32_open
 * description: open frame skip
 * ============================================================*/
isp_ops_t *frame_skip32_open(uint32_t version)
{
  isp_frame_skip_mod_t *fs = malloc(sizeof(isp_frame_skip_mod_t));

  if (!fs) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(fs,  0,  sizeof(isp_frame_skip_mod_t));
  fs->ops.ctrl = (void *)fs;
  fs->ops.init = NULL; //frame_skip_init;
  /* destroy the module object */
  fs->ops.destroy = frame_skip_destroy;
  /* set parameter */
  fs->ops.set_params = frame_skip_set_params;
  /* get parameter */
  fs->ops.get_params = frame_skip_get_params;
  fs->ops.action = frame_skip_action;
  return &fs->ops;
}
