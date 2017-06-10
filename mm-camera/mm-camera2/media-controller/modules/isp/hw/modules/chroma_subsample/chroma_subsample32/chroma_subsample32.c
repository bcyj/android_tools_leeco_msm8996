/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "chroma_subsample32.h"
#include "isp_log.h"

#ifdef ENABLE_CHROMA_SUBSAMPLE_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

/** chroma_subsample_cmd_debug:
 *
 *    @mod:
 *
 **/
static void chroma_subsample_cmd_debug(isp_chroma_ss_mod_t *mod)
{
  ISP_DBG(ISP_MOD_CHROMA_SS, "%s: E", __func__);

  ISP_DBG(ISP_MOD_CHROMA_SS, "%s: hCositedPhase = %d, vCositedPhase = %d", __func__,
    mod->reg_cmd.hCositedPhase, mod->reg_cmd.vCositedPhase);
  ISP_DBG(ISP_MOD_CHROMA_SS, "%s: hCosited = %d, vCosited = %d", __func__,
    mod->reg_cmd.hCosited, mod->reg_cmd.vCosited);
  ISP_DBG(ISP_MOD_CHROMA_SS, "%s: hsubSampleEnable = %d, vsubSampleEnable = %d cropEnable = %d",
    __func__, mod->reg_cmd.hsubSampleEnable, mod->reg_cmd.vsubSampleEnable,
    mod->reg_cmd.cropEnable);
  ISP_DBG(ISP_MOD_CHROMA_SS, "%s: cropWidthLastPixel = %d, cropWidthFirstPixel = %d", __func__,
    mod->reg_cmd.cropWidthLastPixel, mod->reg_cmd.cropWidthFirstPixel);
  ISP_DBG(ISP_MOD_CHROMA_SS, "%s: cropHeightLastLine = %d, cropHeightFirstLine = %d", __func__,
    mod->reg_cmd.cropHeightLastLine, mod->reg_cmd.cropHeightFirstLine);
} /* chroma_subsample_cmd_debug */

/** chroma_subsample_cmd_debug:
 *
 *    @mod_ctrl:
 *    @in_params:
 *    @notify_ops
 *
 * init chroma_subsample module
 *
 **/
static int chroma_subsample_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_chroma_ss_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  return 0;
} /* chroma_subsample_init */

/** chroma_subsample_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_params_size:
 *
 * enable module
 *
 **/
static int chroma_subsample_enable(isp_chroma_ss_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->enable = enable->enable;
  mod->hw_update_pending = mod->enable;

  return 0;
} /* chroma_subsample_enable */

/** chroma_subsample_trigger_enable:
 *
 *    @mod:
 *    @enable:
 *    @in_params_size:
 *
 * tigger module
 *
 **/
static int chroma_subsample_trigger_enable(isp_chroma_ss_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  /* TODO, Implement */
  return 0;
} /*chroma_subsample_trigger_enable */

/** chroma_subsample_config:
 *
 *    @mod:
 *    @in_params:
 *    @in_params_size:
 *
 **/
static int chroma_subsample_config(isp_chroma_ss_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  if (sizeof(isp_hw_pix_setting_params_t) != in_param_size) {
    CDBG_ERROR("%s: in_params size mismatch\n", __func__);
    return -1;
  }

  mod->reg_cmd.hCositedPhase       = 0;
  mod->reg_cmd.vCositedPhase       = 0;
  mod->reg_cmd.cropEnable          =  0;
  mod->reg_cmd.cropWidthFirstPixel =  0;
  mod->reg_cmd.cropWidthLastPixel  =  0;
  mod->reg_cmd.cropHeightFirstLine =  0;
  mod->reg_cmd.cropHeightLastLine  =  0;
  mod->reg_cmd.hCosited            = 0;
  mod->reg_cmd.vCosited            = 0;
  mod->reg_cmd.hsubSampleEnable    =  1;
  mod->reg_cmd.vsubSampleEnable    =  1;

  if (!mod->enable) {
    ISP_DBG(ISP_MOD_CHROMA_SS, "%s: Chroma SS not enabled %d", __func__, mod->enable);
    return -EAGAIN;
  }

  if ((in_params->outputs[ISP_PIX_PATH_ENCODER].stream_param.fmt ==
         CAM_FORMAT_YUV_422_NV61) ||
      (in_params->outputs[ISP_PIX_PATH_ENCODER].stream_param.fmt ==
         CAM_FORMAT_YUV_422_NV16)) {
    mod->reg_cmd.hCosited = 1;
    mod->reg_cmd.vsubSampleEnable = 0;
  } else {
    mod->reg_cmd.hCosited = 0;
    mod->reg_cmd.vsubSampleEnable = 1;
  }

  mod->hw_update_pending = TRUE;

  return 0;
} /* chroma_subsample_config */

/** chroma_subsample_do_hw_update:
 *
 *    @mod:
 *
 * update module register to kernel
 *
 **/
static int chroma_subsample_do_hw_update(isp_chroma_ss_mod_t *mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &mod->reg_cmd;
    cfg_cmd.cmd_len = sizeof(mod->reg_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CHROMA_SS32_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_CHROMA_SS32_LEN * sizeof(uint32_t);

    chroma_subsample_cmd_debug(mod);

    rc = ioctl(mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    mod->hw_update_pending = 0;
  }

  return rc;
} /* chroma_subsample_do_hw_update */

/** chroma_subsample_destroy:
 *
 *    @mod_ctrl:
 *
 * close chroma subsamle module
 *
 **/
static int chroma_subsample_destroy(void *mod_ctrl)
{
  isp_chroma_ss_mod_t *mod = mod_ctrl;

  memset(mod,  0,  sizeof(isp_chroma_ss_mod_t));
  free(mod);

  return 0;
} /* chroma_subsample_destroy */

/** chroma_subsample_set_params:
 *
 *    @mod_ctrl:
 *    @param_id:
 *    @in_params:
 *    @in_param_size:
 *
 * set parameters from ISP
 *
 **/
static int chroma_subsample_set_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_chroma_ss_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = chroma_subsample_enable(mod, (isp_mod_set_enable_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = chroma_subsample_config(mod, in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = chroma_subsample_trigger_enable(mod,
           (isp_mod_set_enable_t *)in_params, in_param_size);
  }
    break;

  default:
    return -EAGAIN;
    break;
  }

  return rc;
} /* chroma_subsample_set_params */

/** chroma_subsample_get_params:
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
static int chroma_subsample_get_params(void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size, void *out_params,
  uint32_t out_param_size)
{
  isp_chroma_ss_mod_t *mod = mod_ctrl;
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

  default: {
    rc = -1;
  }
    break;
  }

  return rc;
} /* chroma_subsample_get_params */

/** chroma_subsample_action:
 *
 *    @mod_ctrl:
 *    @action_code:
 *    @data:
 *    @data_size:
 *
 * processing the action
 *
 **/
static int chroma_subsample_action(void *mod_ctrl, uint32_t action_code,
  void *data, uint32_t data_size)
{
  int rc = 0;
  isp_chroma_ss_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = chroma_subsample_do_hw_update(mod);
  }
    break;

  default: {
    /* no op */
    CDBG_HIGH("%s: action code = %d is not supported. nop",
      __func__, action_code);
    rc = -EAGAIN;
  }
    break;
  }

  return rc;
} /* chroma_subsample_action */

/** chroma_subsample32_open:
 *
 *    @version:
 *
 * open chroma_subsample32 mod, create func table
 *
 **/
isp_ops_t *chroma_subsample32_open(uint32_t version)
{
  isp_chroma_ss_mod_t *mod = malloc(sizeof(isp_chroma_ss_mod_t));

  ISP_DBG(ISP_MOD_CHROMA_SS, "%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }

  memset(mod,  0,  sizeof(isp_chroma_ss_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = chroma_subsample_init;
  mod->ops.destroy = chroma_subsample_destroy;
  mod->ops.set_params = chroma_subsample_set_params;
  mod->ops.get_params = chroma_subsample_get_params;
  mod->ops.action = chroma_subsample_action;

  return &mod->ops;
} /* chroma_subsample32_open */
