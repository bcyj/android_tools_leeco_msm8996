/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "clamp40.h"
#include "isp_log.h"

#ifdef ENABLE_CLAMP_LOGGING
  #undef ISP_DBG
  #define ISP_DBG ALOGE
#endif

/** vfe_clamp_cmd_debug:
 *
 *    @cmd:
 *    @index:
 *
 **/
static void vfe_clamp_cmd_debug(ISP_OutputClampConfigCmdType cmd, int index)
{
  ISP_DBG(ISP_MOD_CLAMP, "%s: CLAMP[%d]\n", __func__, index);

  ISP_DBG(ISP_MOD_CLAMP, "%s: CLAMP[%d]  %d\n",
    __func__, index, cmd.yChanMax);
  ISP_DBG(ISP_MOD_CLAMP, "%s: CLAMP[%d] %d\n",
    __func__, index, cmd.cbChanMax);
  ISP_DBG(ISP_MOD_CLAMP, "%s: CLAMP[%d] %d\n",
    __func__, index, cmd.crChanMax);

  ISP_DBG(ISP_MOD_CLAMP, "%s: CLAMP[%d] %d\n",
    __func__, index, cmd.yChanMin);
  ISP_DBG(ISP_MOD_CLAMP, "%s: CLAMP[%d] %d\n",
    __func__, index, cmd.cbChanMin);
  ISP_DBG(ISP_MOD_CLAMP, "%s: CLAMP[%d] %d\n",
    __func__, index, cmd.crChanMin);

} /* isp_clamp_cmd_debug */

/** clamp_reset:
 *
 *    @mod:
 *
 **/
static void clamp_reset(isp_clamp_mod_t *mod)
{
  memset(&mod->clamp, 0, sizeof(mod->clamp));
  mod->enable = 0;
}

/** clamp_init:
 *
 *    @mod:
 *
 * init abf module
 *
 **/
static int clamp_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_clamp_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;

  clamp_reset(mod);

  return 0;
} /* clamp_init */

/** clamp_destroy:
 *
 *    @mod_ctrl:
 *
 * close abf mod
 *
 **/
static int clamp_destroy(void *mod_ctrl)
{
  isp_clamp_mod_t *mod = mod_ctrl;

  memset(mod,  0,  sizeof(isp_clamp_mod_t));
  free(mod);

  return 0;
} /* clamp_destroy */

/** clamp_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_param_size:
 *
 * close abf mod
 *
 **/
static int clamp_enable(isp_clamp_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->enable = enable->enable;
  if (!mod->enable){
    mod->clamp[ISP_CLAMP_ENCODER].hw_update_pending = 0;
    mod->clamp[ISP_CLAMP_VIEWFINDER].hw_update_pending = 0;
  }

  return 0;
} /* clamp_enable */

/** clamp_do_hw_update:
 *
 *    @clamp_mod:
 *
 * update module register to kernel
 *
 **/
static int clamp_do_hw_update(isp_clamp_mod_t *clamp_mod)
{
  int i, rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  for (i = 0; i< ISP_PIX_PATH_MAX; i++) {
    if (clamp_mod->clamp[i].hw_update_pending) {
      cfg_cmd.cfg_data = (void *) &clamp_mod->clamp[i].reg_cmd;
      cfg_cmd.cmd_len = sizeof(clamp_mod->clamp[i].reg_cmd);
      cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
      cfg_cmd.num_cfg = 1;

    if (i == ISP_PIX_PATH_ENCODER) {
      reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
      reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
      reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CLAMP40_ENC_OFF;
      reg_cfg_cmd[0].u.rw_info.len = ISP_CLAMP40_ENC_LEN * sizeof(uint32_t);
    } else {
      reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
      reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
      reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CLAMP40_VIEW_OFF;
      reg_cfg_cmd[0].u.rw_info.len = ISP_CLAMP40_VIEW_LEN * sizeof(uint32_t);
    }

    vfe_clamp_cmd_debug(clamp_mod->clamp[i].reg_cmd, i);

    rc = ioctl(clamp_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    clamp_mod->clamp[i].hw_update_pending = 0;
    }
  }

  return rc;
} /* clamp_do_hw_update */

/** clamp_config:
 *
 *    @mod:
 *    @in_params:
 *    @in_param_size:
 *
 * configures default values
 *
 **/
static int clamp_config(isp_clamp_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  ISP_DBG(ISP_MOD_CLAMP, "%s: enter, enable = %d", __func__, mod->enable);
  chromatix_parms_type *chroma_ptr =
    (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  int rc = 0;

  if (!mod->enable) { //or enable?
    /* not enabled no op */
    return rc;
  }

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -1;
  }

  mod->clamp[ISP_CLAMP_ENCODER].reg_cmd.cbChanMax  = 255;
  mod->clamp[ISP_CLAMP_ENCODER].reg_cmd.cbChanMin  = 0;
  mod->clamp[ISP_CLAMP_ENCODER].reg_cmd.crChanMax  = 255;
  mod->clamp[ISP_CLAMP_ENCODER].reg_cmd.crChanMin  = 0;
  mod->clamp[ISP_CLAMP_ENCODER].reg_cmd.yChanMax   = 255;
  mod->clamp[ISP_CLAMP_ENCODER].reg_cmd.yChanMin   = 0;

  mod->clamp[ISP_CLAMP_VIEWFINDER].reg_cmd.cbChanMax  = 255;
  mod->clamp[ISP_CLAMP_VIEWFINDER].reg_cmd.cbChanMin  = 0;
  mod->clamp[ISP_CLAMP_VIEWFINDER].reg_cmd.crChanMax  = 255;
  mod->clamp[ISP_CLAMP_VIEWFINDER].reg_cmd.crChanMin  = 0;
  mod->clamp[ISP_CLAMP_VIEWFINDER].reg_cmd.yChanMax   = 255;
  mod->clamp[ISP_CLAMP_VIEWFINDER].reg_cmd.yChanMin   = 0;

  mod->clamp[ISP_CLAMP_ENCODER].hw_update_pending = TRUE;
  mod->clamp[ISP_CLAMP_VIEWFINDER].hw_update_pending = TRUE;

  return rc;
} /* clamp_config */

/** clamp_action:
 *
 *    @mod_ctrl:
 *    @action_code:
 *    @data:
 *    @data_size:
 *
 * configures default values
 *
 **/
static int clamp_action(void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_clamp_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = clamp_do_hw_update(mod);
  }
    break;

  case ISP_HW_MOD_ACTION_RESET: {
    clamp_reset(mod);
  }
    break;

  default: {
    /* no op */
    CDBG_HIGH("%s: action code = %d is not supported. nop", __func__,
      action_code);
  }
    break;
  }

  return rc;
} /* clamp_action */

/** clamp_set_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *    @in_param_size:
 *
 * set parameters from ISP
 *
 **/
static int clamp_set_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_clamp_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = clamp_enable(mod, (isp_mod_set_enable_t *)in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = clamp_config(mod, in_params, in_param_size);
  }
    break;

  default: {
  }
    break;
  }

  return rc;
} /* clamp_set_params */

/** clamp_get_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *    @in_param_size:
 *    @out_params:
 *    @out_param_size:
 *
 * get parameters
 *
 **/
static int clamp_get_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size, void *out_params,
  uint32_t out_param_size)
{
  isp_clamp_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = mod->enable;
  }
    break;

  case ISP_HW_MOD_GET_VFE_DIAG_INFO_USER: {
    vfe_diagnostics_t *vfe_diag = (vfe_diagnostics_t *)out_params;
    if (sizeof(vfe_diagnostics_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
        __func__, param_id);
      break;
    }
    /*Populate vfe_diag data*/
    ISP_DBG(ISP_MOD_CLAMP, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default: {
    rc = -1;
  }
    break;
  }

  return rc;
} /* clamp_get_params */

/** clamp40_open:
 *
 *    @version:
 *
 * open clamp40 mod, create func table
 *
 **/
isp_ops_t *clamp40_open(uint32_t version)
{
  isp_clamp_mod_t *mod = malloc(sizeof(isp_clamp_mod_t));

  ISP_DBG(ISP_MOD_CLAMP, "%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }

  memset(mod,  0,  sizeof(isp_clamp_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = clamp_init;
  mod->ops.destroy = clamp_destroy;
  mod->ops.set_params = clamp_set_params;
  mod->ops.get_params = clamp_get_params;
  mod->ops.action = clamp_action;

  return &mod->ops;
} /* clamp40_open */
