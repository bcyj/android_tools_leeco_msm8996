/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "colorxform44.h"
#include "isp_log.h"

#ifdef ENABLE_COLOR_XFROM_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

/** color_xform_config_601_to_709
 *    @mod: color xform module control pointer
 *    @stream_index: enc or viewfinder stream index
 *
 * This function sets hw registers for 601 to 709 color xform
 * Pix range: (0-255 YCbCr:601) to (16-235 Y, 16-240 CbCr:709
 * HDTV standard)
 *
 * Return: nothing
 **/
/* Encoder 601 to 709 coefficients
 * from system team
 * 0x071c = 0x1f98036f, 0xffffffff
 * 0x0720 = 0x000d5f44, 0xffffffff
 * 0x0724 = 0x03980000, 0xffffffff
 * 0x0728 = 0x00000068, 0xffffffff
 * 0x072c = 0x00440000, 0xffffffff
 * 0x0730 = 0x0000039e, 0xffffffff
 * 0x0734 = 0x00f0f0eb, 0xffffffff
 * 0x0738 = 0x00101010, 0xffffffff
 */
static void color_xform_config_601_to_709(isp_color_xform_mod_t *mod,
  isp_color_xform_path_t stream_index)
{
  ISP_colorXformCfgCmdType *reg_cmd = &mod->reg_cmd[stream_index];

  CDBG_HIGH("%s for stream %s", __FUNCTION__,
       (stream_index == ISP_COLOR_XFORM_ENC)?"Encoder":"Viewfinder");
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_0 0x071c = 0x1f98036f */
  reg_cmd->m00 = 0x36f;
  reg_cmd->m01 = 0x1f98;
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_1 0x0720 = 0x000d5f44 */
  reg_cmd->m02 = 0x1f44;
  reg_cmd->o0 = 0x35;
  reg_cmd->S0 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_0 0x0724 = 0x03980000 */
  reg_cmd->m10 = 0x0;
  reg_cmd->m11 = 0x398;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_1 0x0728 = 0x00000068 */
  reg_cmd->m12 = 0x68;
  reg_cmd->o1 = 0x0;
  reg_cmd->s1 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_0 0x072c = 0x00440000 */
  reg_cmd->m20 = 0x0;
  reg_cmd->m21 = 0x44;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_1 0x0730 = 0x0000039e */
  reg_cmd->m22 = 0x39e;
  reg_cmd->o2 = 0x0;
  reg_cmd->s2 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_HI 0x0734 = 0x00f0f0eb */
  reg_cmd->c01 = 0xeb;
  reg_cmd->c11 = 0xf0;
  reg_cmd->c21 = 0xf0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_LO 0x0738 = 0x00101010 */
  reg_cmd->c00 = 0x10;
  reg_cmd->c10 = 0x10;
  reg_cmd->c20 = 0x10;
}

/** color_xform_config_601_to_601
 *    @mod: color xform module control pointer
 *    @stream_index: enc or viewfinder stream index
 *
 * This function sets hw registers for 601 to 601 color xform
 * Pix range: (0-255 YCbCr:601) to (16-235 Y, 16-240 CbCr:601
 * SDTV standard)
 *
 * Return: nothing
 **/
/* Encoder 601 to 601 coefficients
 * from system team
 * 0x071c = 0x00000400, 0xffffffff
 * 0x0720 = 0000000000, 0xffffffff
 * 0x0724 = 0x04000000, 0xffffffff
 * 0x0728 = 0000000000, 0xffffffff
 * 0x072c = 0000000000, 0xffffffff
 * 0x0730 = 0x00000400, 0xffffffff
 * 0x0734 = 0x00ffffff, 0xffffffff
 * 0x0738 = 0000000000, 0xffffffff
 */
static void color_xform_config_601_to_601(isp_color_xform_mod_t *mod,
  isp_color_xform_path_t stream_index)
{
  ISP_colorXformCfgCmdType *reg_cmd = &mod->reg_cmd[stream_index];
  CDBG_HIGH("%s for stream %s", __FUNCTION__,
       (stream_index == ISP_COLOR_XFORM_ENC)?"Encoder":"Viewfinder");

  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_0 0x071c = 0x00000400 */
  reg_cmd->m00 = 0x400;
  reg_cmd->m01 = 0x0;
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_1 0x0720 = 0000000000 */
  reg_cmd->m02 = 0x0;
  reg_cmd->o0 = 0x0;
  reg_cmd->S0 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_0 0x0724 = 0x04000000 */
  reg_cmd->m10 = 0x0;
  reg_cmd->m11 = 0x400;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_1 0x0728 = 0000000000 */
  reg_cmd->m12 = 0x0;
  reg_cmd->o1 = 0x0;
  reg_cmd->s1 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_0 0x072c = 0000000000 */
  reg_cmd->m20 = 0x0;
  reg_cmd->m21 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_1 0x0730 = 0x00000400 */
  reg_cmd->m22 = 0x400;
  reg_cmd->o2 = 0x0;
  reg_cmd->s2 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_HI 0x0734 = 0x00ffffff */
  reg_cmd->c01 = 0xff;
  reg_cmd->c11 = 0xff;
  reg_cmd->c21 = 0xff;
  /* VFE_COLOR_XFORM_ENC_CLAMP_LO 0x0738 = 0000000000 */
  reg_cmd->c00 = 0x0;
  reg_cmd->c10 = 0x0;
  reg_cmd->c20 = 0x0;
}

/** color_xform_config_601_to_601_sdtv
 *    @mod: color xform module control pointer
 *    @stream_index: enc or viewfinder stream index
 *
 * This function sets hw registers for 601 to 601 color xform
 * Pix range: (0-255 YCbCr:601) to (0-255 YCbCr:601) no change
 *
 * Return: nothing
 **/
/* Encoder 601 to 601_sdtv coefficients
 * from system team
 * 0x071c = 0x00000370
 * 0x0720 = 0x00040000
 * 0x0724 = 0x03870000
 * 0x0728 = 0x0003c000
 * 0x072c = 0x00010000
 * 0x0730 = 0x0003c387
 * 0x0734 = 0x00f0f0eb
 * 0x0738 = 0x00101010
 */
static void color_xform_config_601_to_601_sdtv(isp_color_xform_mod_t *mod,
  isp_color_xform_path_t stream_index)
{
  ISP_colorXformCfgCmdType *reg_cmd = &mod->reg_cmd[stream_index];
  CDBG_HIGH("%s for stream %s", __FUNCTION__,
       (stream_index == ISP_COLOR_XFORM_ENC)?"Encoder":"Viewfinder");

  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_0 0x071c = 0x00000370 */
  reg_cmd->m00 = 0x0370;
  reg_cmd->m01 = 0x0;
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_1 0x0720 = 0x00040000 */
  reg_cmd->m02 = 0x0;
  reg_cmd->o0  = 0x10;
  reg_cmd->S0  = 0x0;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_0 0x0724 = 0x03870000 */
  reg_cmd->m10 = 0x0;
  reg_cmd->m11 = 0x387;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_1 0x0728 = 0x0003c000 */
  reg_cmd->m12 = 0x0;
  reg_cmd->o1  = 0x0f;
  reg_cmd->s1  = 0x0;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_0 0x072c = 0x00010000 */
  reg_cmd->m20 = 0x0;
  reg_cmd->m21 = 0x1;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_1 0x0730 = 0x0003c387 */
  reg_cmd->m22 = 0x387;
  reg_cmd->o2  = 0x0f;
  reg_cmd->s2  = 0x0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_HI 0x0734 = 0x00f0f0eb */
  reg_cmd->c01 = 0xeb;
  reg_cmd->c11 = 0xf0;
  reg_cmd->c21 = 0xf0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_LO 0x0738 = 0x00101010 */
  reg_cmd->c00 = 0x10;
  reg_cmd->c10 = 0x10;
  reg_cmd->c20 = 0x10;
}

/** color_xform_reset
 *    @mod: color_xform module struct data
 *
 * color_xform module disable,release reg settings and strcuts
 *
 * Return: nothing
 **/
static void color_xform_reset(isp_color_xform_mod_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));
  mod->hw_update_pending = 0;
  mod->trigger_enable = 0; /* enable trigger update feature flag from PIX */
  mod->skip_trigger = 0;
  mod->enable = 0;         /* enable flag from PIX */
}

/** color_xform_init
 *    @mod_ctrl: color xform module control strcut
 *    @in_params: color xform hw module init params
 *    @notify_ops: fn pointer to notify other modules
 *
 *  color xform module data struct initialization
 *
 * Return: 0 always
 **/
static int color_xform_init (void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_color_xform_mod_t *mod = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  mod->fd = init_params->fd;
  mod->notify_ops = notify_ops;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  color_xform_reset(mod);
  return 0;
} /* color_xform_init */

/** color_xform_config
 *    @mod: color_xform module struct data
 *    @in_params : input params
 *    @in_param_size: size of struct
 *
 * Copy from mod->chromatix params to reg cmd then configure
 *
 * Return: nothing
 **/
static int color_xform_config(isp_color_xform_mod_t *mod,
  isp_hw_pix_setting_params_t *in_params, uint32_t in_param_size)
{
  int rc = 0;
  color_xform_type_t color_xform = XFORM_MAX;

  ISP_DBG(ISP_MOD_COLOR_XFORM, "%s: E", __func__);

  if (!mod->enable) {
    CDBG_ERROR("%s: module not enabled %d", __func__, mod->enable);
    return rc;
  }

  /* set old cfg to invalid value to trigger the first trigger update */
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mod->hw_update_pending = TRUE;
  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));

  color_xform =
          (in_params->recording_hint)?XFORM_601_601_SDTV:XFORM_601_601;

  ISP_DBG(ISP_MOD_COLOR_XFORM, "%s: recording_hint: %d set_color_xform: %d",
    __func__, in_params->recording_hint, color_xform);
  switch(color_xform) {
  case XFORM_601_601: {
    color_xform_config_601_to_601(mod, ISP_COLOR_XFORM_ENC);
    color_xform_config_601_to_601(mod, ISP_COLOR_XFORM_VIEWFINDER);
  }
    break;

  case XFORM_601_601_SDTV: {
    if(in_params->outputs[0].stream_param.streaming_mode ==
        CAM_STREAMING_MODE_BURST) /* live shot stream */
      color_xform_config_601_to_601(mod, ISP_COLOR_XFORM_ENC);
    else
      color_xform_config_601_to_601_sdtv(mod, ISP_COLOR_XFORM_ENC);

    if (in_params->outputs[1].stream_param.streaming_mode ==
        CAM_STREAMING_MODE_BURST)/* live shot stream */
      color_xform_config_601_to_601(mod, ISP_COLOR_XFORM_VIEWFINDER);
    else
      color_xform_config_601_to_601_sdtv(mod, ISP_COLOR_XFORM_VIEWFINDER);
  }
    break;

  case XFORM_601_709: {
    if (in_params->outputs[0].stream_param.streaming_mode ==
         CAM_STREAMING_MODE_BURST) /* live shot stream */
      color_xform_config_601_to_601(mod, ISP_COLOR_XFORM_ENC);
    else
      color_xform_config_601_to_709(mod, ISP_COLOR_XFORM_ENC);
    if (in_params->outputs[1].stream_param.streaming_mode ==
       CAM_STREAMING_MODE_BURST)/* live shot stream */
      color_xform_config_601_to_601(mod, ISP_COLOR_XFORM_VIEWFINDER);
    else
      color_xform_config_601_to_709(mod, ISP_COLOR_XFORM_VIEWFINDER);
  }
    break;

  case XFORM_MAX:
  default: {
    CDBG_ERROR("%s: unsupported color xform: %d",
       __func__, color_xform);
    rc = -1;
  }
    break;
  }

  return rc;
} /* color_xform_config */

/** color_xform_enable
 *    @mod: color_xform module control struct
 *    @enable: module enable/disable flag
 *
 *  color_xform module enable/disable method
 *
 * Return: 0 - success and negative value - failure
 **/
static int color_xform_enable(isp_color_xform_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mod->enable = enable->enable;
  if (!mod->enable)
    mod->hw_update_pending = 0;
  return 0;
} /* color_xform_enable */

/** color_xform_trigger_enable
 *    @mod: color_xform module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  color_xform module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int color_xform_trigger_enable(isp_color_xform_mod_t *mod,
  isp_mod_set_enable_t *enable, uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->trigger_enable = enable->enable;
  return 0;
} /* color_xform_trigger_enable */

/** color_xform_trigger_update
 *    @mod: color_xform module control struct
 *    @in_params: input config params including chromatix ptr
 *    @in_param_size: input params struct size
 *
 *  color_xform module modify reg settings as per new input
 *  params and trigger hw update
 *
 * Return: 0 - success and negative value - failure
 **/
static int color_xform_trigger_update(isp_color_xform_mod_t *mod,
  isp_pix_trigger_update_input_t *in_params,
  uint32_t in_param_size)
{
  /* no trigger updated needed */
  return 0;
} /* color_xform_trigger_update */

/** util_color_xform_debug
 *    @cmd: color_xform config cmd
 *
 * This function dumps the color xform module configuration set
 * to hw
 *
 * Return: nothing
 **/
static void util_color_xform_debug(isp_color_xform_mod_t *mod)
{
  ISP_colorXformCfgCmdType *reg_cmd;
  int32_t i = 0;
  for(i = 0; i < ISP_COLOR_XFORM_MAX; i++) {
    reg_cmd = &mod->reg_cmd[i];
    ISP_DBG(ISP_MOD_COLOR_XFORM, "%s:%d xform coefficients:\n"
      "m00 = 0x%x, m01 = 0x%x, m02 = 0x%x, o0 = 0x%x, S0 = 0x%x\n"
      "ml0 = 0x%x, ml1 = 0x%x, ml2 = 0x%x, o1 = 0x%x, s1 = 0x%x\n"
      "m20 = 0x%x, m21 = 0x%x, m22 = 0x%x, o2 = 0x%x, s2 = 0x%x\n"
      "c01 = 0x%x, c11 = 0x%x, c21 = 0x%x, c00 = 0x%x, c10 = 0x%x, c20 =0x%x\n",
      __FUNCTION__, i,
    reg_cmd->m00, reg_cmd->m01, reg_cmd->m02, reg_cmd->o0, reg_cmd->S0,
    reg_cmd->m10, reg_cmd->m11, reg_cmd->m12, reg_cmd->o1, reg_cmd->s1,
    reg_cmd->m20, reg_cmd->m21, reg_cmd->m22, reg_cmd->o2, reg_cmd->s2,
    reg_cmd->c01, reg_cmd->c11, reg_cmd->c21, reg_cmd->c00, reg_cmd->c10,
    reg_cmd->c20);
  }
}/* util_color_xform_debug*/

/** color_xform_do_hw_update
 *    @mod:xform module struct data
 *
 * update color_xform module register to kernel
 *
 * Return: nothing
 **/
static int color_xform_do_hw_update(isp_color_xform_mod_t *mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (mod->hw_update_pending) {
    util_color_xform_debug(mod); /* dump xform coefficient */
    cfg_cmd.cfg_data = (void *) mod->reg_cmd;
    cfg_cmd.cmd_len = sizeof(mod->reg_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_COLOR_XFORM40_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_COLOR_XFORM40_LEN * sizeof(uint32_t);

    util_color_xform_debug(mod);
    rc = ioctl(mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    mod->hw_update_pending = 0;
  }
  return rc;
} /* color_xform_do_hw_update */

/** color_xform_destroy
 *    @mod_ctrl: color_xform module control strcut
 *
 *  Close color_xform module
 *
 * Return: 0 always
 **/
static int color_xform_destroy (void *mod_ctrl)
{
  isp_color_xform_mod_t *mod = mod_ctrl;

  memset(mod,  0,  sizeof(isp_color_xform_mod_t));
  free(mod);
  return 0;
} /* color_xform_destroy */

/** color_xform_set_params
 *    @mod_ctrl: xform module control struct
 *    @param_id : param enum index
 *    @in_params: input config params based on param idex
 *    @in_param_size: input params struct size
 *
 *  set config params utility to update color_xform module
 *
 * Return: 0 - success and negative value - failure
 **/
static int color_xform_set_params (void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_color_xform_mod_t *mod = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = color_xform_enable(mod, (isp_mod_set_enable_t *)in_params,
      in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = color_xform_config(mod, in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = color_xform_trigger_enable(mod, in_params, in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE: {
    rc = color_xform_trigger_update(mod, in_params, in_param_size);
  }
    break;

  default: {
    rc = -EAGAIN;
  }
    break;
  }
  return rc;
} /* color_xform_set_params */

/** color_xform_get_params
 *    @mod_ctrl: color_xform module control struct
 *    @param_id : param enum index
 *    @in_params: input config params based on param idex
 *    @in_param_size: input params struct size
 *    @out_params: struct to return out params
 *    @out_param_size: output params struct size
 *
 *  Get config params utility to fetch config of color_xform
 *  module
 *
 * Return: 0 - success and negative value - failure
 **/
static int color_xform_get_params (void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size, void *out_params,
  uint32_t out_param_size)
{
  int rc = 0;
  isp_color_xform_mod_t *mod = mod_ctrl;

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
} /* color_xform_get_params */

/** color_xform_action
 *    @mod_ctrl: xform module control struct
 *    @action_code : action code
 *    @data: not used
 *    @data_size: not used
 *
 *  processing the hw action like update or reset
 *
 * Return: 0 - success and negative value - failure
 **/
static int color_xform_action (void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_color_xform_mod_t *mod = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = color_xform_do_hw_update(mod);
  }
    break;

  case ISP_HW_MOD_ACTION_RESET: {
    color_xform_reset(mod);
  }
    break;

  default: {
    /* no op */
    ISP_DBG(ISP_MOD_COLOR_XFORM, "%s: action code = %d is not supported. nop",
      __func__, action_code);
    rc = 0;
  }
    break;
  }

  return rc;
} /* color_xform_action */

/** color_xform44_open
 *    @version: hw version
 *
 *  color_xform 40 module open and create func table
 *
 * Return: color_xform module ops struct pointer
 **/
isp_ops_t *color_xform44_open(uint32_t version)
{
  isp_color_xform_mod_t *mod = malloc(sizeof(isp_color_xform_mod_t));

  ISP_DBG(ISP_MOD_COLOR_XFORM, "%s: E", __func__);

  if (!mod) {
    CDBG_ERROR("%s: fail to allocate memory",  __func__);
    return NULL;
  }
  memset(mod,  0,  sizeof(isp_color_xform_mod_t));

  mod->ops.ctrl = (void *)mod;
  mod->ops.init = color_xform_init;
  mod->ops.destroy = color_xform_destroy;
  mod->ops.set_params = color_xform_set_params;
  mod->ops.get_params = color_xform_get_params;
  mod->ops.action = color_xform_action;

  return &mod->ops;
} /* color_xform44_open */
