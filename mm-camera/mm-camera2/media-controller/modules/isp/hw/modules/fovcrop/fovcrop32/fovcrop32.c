/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "fovcrop32.h"
#include "isp_log.h"

#ifdef ENABLE_FOV_LOGGING
  #undef ISP_DBG
  #define ISP_DBG LOGE
#endif

#define MIN_DOWNSCALE_FACTOR 1.03125

#define CHECK_ASPECT_RATIO(x) ((1.3 <= x) && (x <= 1.34))
#define CHECK_DEFAULT_CROP_FACTOR(x) (x == (1<<12))
#define CHECK_DOWNSCALE_FACTOR(x) ((1.0 < x) && (x < MIN_DOWNSCALE_FACTOR))

/*===========================================================================
 * FUNCTION    - vfe_fov_cmd_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void vfe_fov_cmd_debug(ISP_FOV_CropConfigCmdType *cmd)
{
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP firstPixel %d\n", __func__, cmd->firstPixel);
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP lastPixel %d\n", __func__, cmd->lastPixel);
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP firstLine %d\n", __func__, cmd->firstLine);
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP lastLine %d\n", __func__, cmd->lastLine);
} /* isp_fov_cmd_debug */

/*===========================================================================
 * FUNCTION    - zoom_vfe -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_calc_fov(
  isp_hw_pix_setting_params_t *pix_setting,
  uint32_t crop_factor,
  crop_window_info_t *fov_win)
{
  /* todo_bug_fix? hardcoding of index of 0 or 1 is wrong */
  uint32_t output2Height = pix_setting->outputs[0].stream_param.height;
  uint32_t output2Width  = pix_setting->outputs[0].stream_param.width;
  uint32_t output2_aspect_ratio = 0;
  uint32_t input_aspect_ratio = 0;

  CDBG_HIGH("%s: out1w = %d, out1h = %d, out2w = %d, out2h = %d", __func__,
    pix_setting->outputs[0].stream_param.width,
    pix_setting->outputs[0].stream_param.height,
    pix_setting->outputs[1].stream_param.width,
    pix_setting->outputs[1].stream_param.height);

  if(!output2Height) {
    CDBG_ERROR("%s: output2 height = 0", __func__);
    return -EINVAL;
  }

  memset(fov_win, 0, sizeof(crop_window_info_t));
  /* check how the output aspect ratio compares to the camsensor aspect ratio.
   * Once we have this information we can use vfe_camera_crop_factor to scale
   * one direction and calculate the crop in the other direction based of the
   * output aspect ratio. If output is wider than camsensor we want to use the
   * full width of the output and reduce the height. If output is narrower we
   * want to use the full height and reduce the width.*/

  uint32_t  crop_input_w;

  if (pix_setting->camif_cfg.is_bayer_sensor) {
    crop_input_w = pix_setting->demosaic_output.last_pixel -
      pix_setting->demosaic_output.first_pixel + 1;
  } else {
    crop_input_w = (pix_setting->demosaic_output.last_pixel -
      pix_setting->demosaic_output.first_pixel + 1)/2;
  }

  uint32_t  crop_input_h = pix_setting->demosaic_output.last_line -
    pix_setting->demosaic_output.first_line + 1;

  input_aspect_ratio = (crop_input_w * Q12) /(crop_input_h);

  if(output2Height)
    output2_aspect_ratio = (output2Width * Q12) / output2Height;

  /* if the narrowest output is narrower than the camsensor output */
  if (output2_aspect_ratio < input_aspect_ratio) {
    /* Calculate Crop in the Y Direction using vfe_camera_crop_factor */
    fov_win->crop_out_y =
      (crop_input_h * Q12) / crop_factor;
    fov_win->crop_out_x =
      (fov_win->crop_out_y * output2_aspect_ratio) / Q12;
    if (fov_win->crop_out_x > crop_input_w)
       fov_win->crop_out_x = crop_input_w;
  } else {// output2_aspect_ratio >= input_aspect_ratio
    /* if output is wider than the cam-sensor output */
    fov_win->crop_out_x =
      (crop_input_w * Q12) / crop_factor;
    fov_win->crop_out_y =
      (fov_win->crop_out_x * Q12) / (output2_aspect_ratio);
    if (fov_win->crop_out_y > crop_input_h)
       fov_win->crop_out_y = crop_input_h;
  }

  CDBG_HIGH("%s: demosaic first_p = %d, last_p = %d, first_l = %d, last_l = %d",
    __func__, pix_setting->demosaic_output.first_pixel,
    pix_setting->demosaic_output.last_pixel,
    pix_setting->demosaic_output.first_line,
    pix_setting->demosaic_output.last_line);

  CDBG_HIGH("%s: crop_factor = %d, x=%d, y=%d, crop_out_x=%d, crop_out_y=%d",
    __func__, crop_factor, fov_win->x, fov_win->y, fov_win->crop_out_x,
    fov_win->crop_out_y);

  return 0;
}

/*===========================================================================
 * FUNCTION    - fov_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int fov_config(isp_fov_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_setting, uint32_t in_param_size)
{
  int rc = 0;
  int i;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d", __func__,
      sizeof(isp_hw_pix_setting_params_t), in_param_size);
    return -EINVAL;
  }

  if (!mod->fov_enable)
    return rc;

  /* now determine crop window */
  if(!vfe_calc_fov(pix_setting, pix_setting->crop_factor, &mod->output_res)) {

    mod->fov_win.crop_out_x = mod->output_res.crop_out_x;
    mod->fov_win.crop_out_y = mod->output_res.crop_out_y;
    if(mod->output_res.crop_out_x < pix_setting->outputs[0].stream_param.width)
      mod->output_res.crop_out_x = pix_setting->outputs[0].stream_param.width;

    if(mod->output_res.crop_out_y < pix_setting->outputs[0].stream_param.height)
      mod->output_res.crop_out_y = pix_setting->outputs[0].stream_param.height;

    if (pix_setting->camif_cfg.is_bayer_sensor) {
      mod->fov_cmd.firstPixel = ((pix_setting->demosaic_output.last_pixel -
        pix_setting->demosaic_output.first_pixel + 1)  -
        mod->output_res.crop_out_x) / 2;
    } else {
      mod->fov_cmd.firstPixel = (((pix_setting->demosaic_output.last_pixel -
        pix_setting->demosaic_output.first_pixel + 1) / 2) -
        mod->output_res.crop_out_x) / 2;
    }

    mod->fov_cmd.firstLine= ((pix_setting->demosaic_output.last_line -
      pix_setting->demosaic_output.first_line + 1) -
      mod->output_res.crop_out_y) / 2;

    mod->fov_cmd.lastPixel = mod->fov_cmd.firstPixel +
      mod->output_res.crop_out_x -1;
    mod->fov_cmd.lastLine = mod->fov_cmd.firstLine +
      mod->output_res.crop_out_y -1;
    mod->fov_win.x = mod->fov_cmd.firstPixel;
    mod->fov_win.y = mod->fov_cmd.firstLine;
    pix_setting->crop_info.y.crop_factor = pix_setting->crop_factor;
    pix_setting->crop_info.y.pix_line.first_pixel = mod->fov_cmd.firstPixel;
    pix_setting->crop_info.y.pix_line.last_pixel = mod->fov_cmd.firstPixel +
      mod->output_res.crop_out_x -1;
    pix_setting->crop_info.y.pix_line.first_line = mod->fov_cmd.firstLine;
    pix_setting->crop_info.y.pix_line.last_line = mod->fov_cmd.firstLine +
      mod->output_res.crop_out_y -1;

    ISP_DBG(ISP_MOD_FOV, "%s: crop_factor = %d, fov_out first_pix = %d, first_line = %d,"
      "last_pix = %d, last_line = %d", __func__, pix_setting->crop_factor,
      mod->fov_cmd.firstPixel, mod->fov_cmd.firstLine,
      mod->fov_cmd.lastPixel, mod->fov_cmd.lastLine);
  }

  mod->old_streaming_mode = pix_setting->streaming_mode;

  mod->hw_update_pending = TRUE;
  /* update fov state for initial configuration.
   * run time trigger update/zoom ratio change
   * also uses this function */

  return rc;
}

/*===========================================================================
 * FUNCTION      fov_trigger_update
 *
 * DESCRIPTION:
 *==========================================================================*/
static int fov_trigger_update(isp_fov_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_input, uint32_t in_param_size)
{
  int rc = 0;

  int is_burst = IS_BURST_STREAMING(trigger_input);
  camera_flash_type new_flash_mode;

  chromatix_parms_type *chrPtr =
    (chromatix_parms_type *)trigger_input->cfg.chromatix_ptrs.chromatixPtr;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  if (!mod->fov_enable)
    return 0;

  if (!mod->fov_trigger_enable) {
    ISP_DBG(ISP_MOD_FOV, "%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return 0;
  }

  rc = fov_config(mod, &trigger_input->cfg, sizeof(trigger_input->cfg));

  return rc;
} /* fov_trigger_update */

/*===========================================================================
 * FUNCTION    - fov_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int fov_init (void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_fov_mod_t *fov = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  fov->fd = init_params->fd;
  fov->notify_ops = notify_ops;
  fov->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  fov->hw_update_pending = FALSE;

  return 0;
}/* fov_init */

/*===========================================================================
 * FUNCTION    - fov_set_zoom_ratio -
 *
 * DESCRIPTION: Set new zoom ratio.
 *==========================================================================*/
static int fov_set_zoom_ratio(
  isp_fov_mod_t *fov,
  isp_hw_pix_setting_params_t *pix_setting,
  uint32_t in_param_size)
{
  int rc = 0;
  rc = fov_config(fov, pix_setting, in_param_size);

  return rc;
}

/* ============================================================
 * function name: fov_enable
 * description: enable fov
 * ============================================================*/
static int fov_enable(isp_fov_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d", __func__,
      sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  mod->fov_enable = enable->enable;
  mod->hw_update_pending = mod->fov_enable;

  return 0;
}

/* ============================================================
 * function name: fov_trigger_enable
 * description: enable trigger update feature
 * ============================================================*/
static int fov_trigger_enable(isp_fov_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d", __func__,
      sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mod->fov_trigger_enable = enable->enable;

  return 0;
}

static int fov_get_fov_crop(
  isp_fov_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_setting,
  isp_hw_zoom_param_t *hw_zoom_param)
{
  int i;

  memset(hw_zoom_param, 0, sizeof(isp_hw_zoom_param_t));

  /* the output is used */
  memset(hw_zoom_param,  0,  sizeof(isp_hw_zoom_param_t));
  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
    if (pix_setting->outputs[i].stream_param.width) {
      /* the output is used, fill in stream info */
      hw_zoom_param->entry[hw_zoom_param->num].dim.width =
        pix_setting->outputs[i].stream_param.width;
      hw_zoom_param->entry[hw_zoom_param->num].dim.height =
        pix_setting->outputs[i].stream_param.height;
      hw_zoom_param->entry[hw_zoom_param->num].stream_id =
        pix_setting->outputs[i].stream_param.stream_id;
      /* get fov first pix, first line, delta_x and delta_y*/
      hw_zoom_param->entry[hw_zoom_param->num].crop_win = mod->fov_win;
      hw_zoom_param->num++;
    }
  }
  return 0;
}

/* ============================================================
 * function name: fov_destroy
 * description: close fov
 * ============================================================*/
static int fov_destroy (void *mod_ctrl)
{
  isp_fov_mod_t *fov = mod_ctrl;

  if (fov) {
    memset(fov, 0, sizeof(isp_fov_mod_t));
    free(fov);
  }

  return 0;
}

/* ============================================================
 * function name: fov_set_params
 * description: set parameters
 * ============================================================*/
static int fov_set_params (void *mod_ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_fov_mod_t *fov = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = fov_enable(fov, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = fov_config(fov, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = fov_trigger_enable(fov, (isp_mod_set_enable_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = fov_trigger_update(fov, (isp_pix_trigger_update_input_t *)in_params,
      in_param_size);
    break;
  case ISP_HW_MOD_SET_ZOOM_RATIO:
    rc = fov_set_zoom_ratio(fov,
      (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  default:
    rc = -EAGAIN;
    break;
  }
  return rc;
}

/* ============================================================
 * function name: fov_get_params
 * description: get parameters
 * ============================================================*/
static int fov_get_params (void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  int rc =0;
  isp_fov_mod_t *fov = mod_ctrl;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d", __func__,
        param_id);
      break;
    }
    enable->enable = fov->fov_enable;
    break;
  }
  case ISP_HW_MOD_GET_FOV:
    rc = fov_get_fov_crop(fov, (isp_hw_pix_setting_params_t *)in_params,
      (isp_hw_zoom_param_t *)out_params);
    break;
  case ISP_PIX_GET_FOV_OUTPUT: {
    isp_pixel_line_info_t *fov_output = out_params;
    fov_output[0].first_pixel = fov->fov_cmd.firstPixel;
    fov_output[0].last_pixel  = fov->fov_cmd.lastPixel;
    fov_output[0].first_line  = fov->fov_cmd.firstLine;
    fov_output[0].last_line   = fov->fov_cmd.lastLine;
    break;
  }
  default:
    rc = -EPERM;
    break;
  }

  return rc;
}

/* ============================================================
 * function name: fov_do_hw_update
 * description: fov_do_hw_update
 * ============================================================*/
static int fov_do_hw_update(isp_fov_mod_t *fov_mod)
{
  int i, rc = 0;

  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  ISP_DBG(ISP_MOD_FOV, "%s: HW_update, = %d\n", __func__, fov_mod->hw_update_pending);

  if (fov_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &fov_mod->fov_cmd;
    cfg_cmd.cmd_len = sizeof(fov_mod->fov_cmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_FOV32_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_FOV32_LEN * sizeof(uint32_t);

    vfe_fov_cmd_debug(&fov_mod->fov_cmd);
    rc = ioctl(fov_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    fov_mod->hw_update_pending = 0;
  }

  return rc;
}

/* ============================================================
 * function name: fov_action
 * description: processing the action
 * ============================================================*/
static int fov_action (void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_fov_mod_t *fov = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = fov_do_hw_update(fov);
    break;
  default:
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop", __func__,
      action_code);
    break;
  }
  return rc;
}

/* ============================================================
 * function name: fov32_open
 * description: open fov
 * ============================================================*/
isp_ops_t *fov32_open(uint32_t version)
{
  isp_fov_mod_t *fov = malloc(sizeof(isp_fov_mod_t));

  if (!fov) {
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }

  memset(fov,  0,  sizeof(isp_fov_mod_t));
  fov->ops.ctrl = (void *)fov;
  fov->ops.init = fov_init;
  fov->ops.destroy = fov_destroy;
  fov->ops.set_params = fov_set_params;
  fov->ops.get_params = fov_get_params;
  fov->ops.action = fov_action;

  return &fov->ops;
}
