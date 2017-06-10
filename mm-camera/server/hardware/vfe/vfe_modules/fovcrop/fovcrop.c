/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

/*#define ENABLE_FOV_LOGGING 1*/
#ifdef ENABLE_FOV_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

//#define USE_FOVCROP_FOR_SMALL_SCALE_FACTOR
#define MIN_DOWNSCALE_FACTOR 1.03125

#define CHECK_ASPECT_RATIO(x) ((1.3 <= x) && (x <= 1.34))
#define CHECK_DEFAULT_CROP_FACTOR(x) (x == (1<<12))
#define CHECK_DOWNSCALE_FACTOR(x) ((1.0 < x) && (x < MIN_DOWNSCALE_FACTOR))

/*===========================================================================
 * FUNCTION    - vfe_fov_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_config(int mod_id, void *mod, void *params)
{
  fov_mod_t *fov_mod = (fov_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  if (!fov_mod->fov_enable) {
    CDBG("%s: fov not enabled", __func__);
    return VFE_SUCCESS;
  }

#ifdef USE_FOVCROP_FOR_SMALL_SCALE_FACTOR
  int is_snap_mode = IS_SNAP_MODE(vfe_params);
  camera_size_t *camif_size = &vfe_params->vfe_input_win;
  uint32_t input_width, input_height, output_width, output_height;
  int update_fov = FALSE;
  float scale_factor_horiz, scale_factor_vert;
  float camif_scale_x, camif_scale_y;
  input_width = vfe_params->crop_info.crop_out_x;
  input_height = vfe_params->crop_info.crop_out_y;
  float aspect_ratio = (float)input_width / (float)input_height;
  output_width = vfe_params->output2w;
  output_height = vfe_params->output2h;
  scale_factor_horiz = (float)input_width / (float)output_width;
  scale_factor_vert = (float)input_height / (float)output_height;
  camif_scale_x = (float)camif_size->width / (float)input_width;
  camif_scale_y = (float)camif_size->height / (float)input_height;
  CDBG("%s: horiz %f vert %f aspect_ratio %f", __func__,
    scale_factor_horiz, scale_factor_vert, aspect_ratio);
  CDBG("%s: camif_scale_x %f camif_scale_y %f crop_f %d", __func__,
    camif_scale_x, camif_scale_y, vfe_params->crop_factor);
  update_fov = (is_snap_mode) &&
     CHECK_DOWNSCALE_FACTOR(scale_factor_vert) &&
     CHECK_DOWNSCALE_FACTOR(scale_factor_horiz) &&
     CHECK_DEFAULT_CROP_FACTOR(vfe_params->crop_factor) &&
     CHECK_ASPECT_RATIO(aspect_ratio);
  CDBG("%s: update_fov %d", __func__, update_fov);

  fov_mod->fov_cmd.firstPixel = vfe_params->crop_info.crop_first_pixel;
  fov_mod->fov_cmd.lastPixel  = vfe_params->crop_info.crop_last_pixel;
  fov_mod->fov_cmd.firstLine  = vfe_params->crop_info.crop_first_line;
  fov_mod->fov_cmd.lastLine   = vfe_params->crop_info.crop_last_line;
  CDBG("%s: dim %d %d %d %d", __func__,
    fov_mod->fov_cmd.firstPixel, fov_mod->fov_cmd.lastPixel,
    fov_mod->fov_cmd.firstLine, fov_mod->fov_cmd.lastLine);
  if (update_fov) {
    int x_delta = (camif_size->width - output_width)/2;
    int y_delta = (camif_size->height - output_height)/2;
    vfe_params->crop_info.crop_first_pixel =
      fov_mod->fov_cmd.firstPixel = x_delta;
    vfe_params->crop_info.crop_last_pixel =
      fov_mod->fov_cmd.lastPixel  = output_width + x_delta - 1;
    vfe_params->crop_info.crop_first_line =
      fov_mod->fov_cmd.firstLine = y_delta;
    vfe_params->crop_info.crop_last_line =
      fov_mod->fov_cmd.lastLine   = output_height + y_delta - 1;
    vfe_params->crop_info.crop_out_x = output_width;
    vfe_params->crop_info.crop_out_y = output_height;

    CDBG("%s: new dim %d %d %d %d", __func__,
      fov_mod->fov_cmd.firstPixel, fov_mod->fov_cmd.lastPixel,
      fov_mod->fov_cmd.firstLine, fov_mod->fov_cmd.lastLine);
  }
#else
  fov_mod->fov_cmd.firstPixel = vfe_params->crop_info.crop_first_pixel;
  fov_mod->fov_cmd.lastPixel  = vfe_params->crop_info.crop_last_pixel;
  fov_mod->fov_cmd.firstLine  = vfe_params->crop_info.crop_first_line;
  fov_mod->fov_cmd.lastLine   = vfe_params->crop_info.crop_last_line;
#endif

  vfe_util_write_hw_cmd(vfe_params->camfd,CMD_GENERAL,
    (void *)&fov_mod->fov_cmd, sizeof(fov_mod->fov_cmd),
     VFE_CMD_FOV_CFG);
  CDBG("%s:VFE_CMD_FOV_CFG, enable = %d, update = %d, firstPixel : %u, lastPixel : %u, firstLine : %u, lastLine : %u",
    __func__, fov_mod->fov_enable, fov_mod->fov_update, fov_mod->fov_cmd.firstPixel, fov_mod->fov_cmd.lastPixel,
    fov_mod->fov_cmd.firstLine, fov_mod->fov_cmd.lastLine);

  return VFE_SUCCESS;
} /* vfe_config_fov */

/*===========================================================================
 * FUNCTION    - vfe_fov_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_update(int mod_id, void *mod, void *params)
{
  vfe_status_t status;
  fov_mod_t *fov_mod = (fov_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  CDBG("%s: enable %d update %d", __func__, fov_mod->fov_enable,
    fov_mod->fov_update);
  if (!fov_mod->fov_enable) {
    CDBG("%s: fov not enabled", __func__);
    return VFE_SUCCESS;
  }

  if(fov_mod->fov_update) {
    status = vfe_fov_config(mod_id, fov_mod,vfe_params);
    if (status != VFE_SUCCESS)
      CDBG_HIGH("%s: Failed\n",__func__);
    else
      vfe_params->update |= VFE_MOD_FOV;
  }
  fov_mod->fov_update = FALSE;
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - zoom_vfe -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int vfe_calc_fov(
  vfe_params_t *vfe_params,
  uint32_t crop_factor,
  crop_window_info_t *fov_win)
{
  uint32_t output2Height = vfe_params->output2h;
  uint32_t output2Width  = vfe_params->output2w;
  uint32_t output2_aspect_ratio = 0;
  uint32_t input_aspect_ratio = 0;

  CDBG("%s: out1w = %d, out1h = %d, out2w = %d, out2h = %d",
    __func__, vfe_params->output1w, vfe_params->output1h, output2Width, output2Height);

  if(!output2Height) {
    CDBG("%s: output2 height = 0", __func__);
    return -1;
  }
  memset(fov_win, 0, sizeof(crop_window_info_t));
  /* check how the output aspect ratio compares to the camsensor aspect ratio.
   * Once we have this information we can use vfe_camera_crop_factor to scale
   * one direction and calculate the crop in the other direction based of the
   * output aspect ratio. If output is wider than camsensor we want to use the
   * full width of the output and reduce the height. If output is narrower we
   * want to use the full height and reduce the width.*/

  uint32_t  crop_input_w;

  if (IS_BAYER_FORMAT(vfe_params)) {
    crop_input_w = vfe_params->demosaic_op_params.last_pixel -
      vfe_params->demosaic_op_params.first_pixel + 1;
  }
  else {
    crop_input_w = (vfe_params->demosaic_op_params.last_pixel -
      vfe_params->demosaic_op_params.first_pixel + 1)/2;
  }

  uint32_t  crop_input_h = vfe_params->demosaic_op_params.last_line -
    vfe_params->demosaic_op_params.first_line + 1;

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
  CDBG("%s: demosaic first_pixel = %d, last_pixel = %d, first_line = %d, last_line = %d",
    __func__, vfe_params->demosaic_op_params.first_pixel, vfe_params->demosaic_op_params.last_pixel,
    vfe_params->demosaic_op_params.first_line, vfe_params->demosaic_op_params.last_line);
  CDBG("%s: crop_factor = %d, x = %d, y = %d, crop_out_x = %d, crop_out_y = %d",
    __func__, crop_factor, fov_win->x, fov_win->y,
   fov_win->crop_out_x, fov_win->crop_out_y);
  return 0;
}

/*===========================================================================
 * FUNCTION    - vfe_crop_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_crop_config(void *params)
{
  crop_window_info_t fov_win;
  CDBG("%s\n",__func__);
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  /* now determine crop window */
  if(0 == vfe_calc_fov(vfe_params, vfe_params->crop_factor, &fov_win)) {
    /* note: crop_out_x is the crop win without rounding to main scaler */
    vfe_params->crop_info.crop_out_x = fov_win.crop_out_x;
    vfe_params->crop_info.crop_out_y = fov_win.crop_out_y;
    /* VFE has no upscaling so set the lower bound to out2 w/h */
    if(fov_win.crop_out_x < vfe_params->output2w)
      fov_win.crop_out_x = vfe_params->output2w;
    if(fov_win.crop_out_y < vfe_params->output2h)
      fov_win.crop_out_y = vfe_params->output2h;
    if (IS_BAYER_FORMAT(vfe_params)) {
      fov_win.x = ((vfe_params->demosaic_op_params.last_pixel -
          vfe_params->demosaic_op_params.first_pixel + 1)  -
        fov_win.crop_out_x) / 2;
    } else {
      fov_win.x = (((vfe_params->demosaic_op_params.last_pixel -
          vfe_params->demosaic_op_params.first_pixel + 1) / 2) -
        fov_win.crop_out_x) / 2;
    }
    fov_win.y = ((vfe_params->demosaic_op_params.last_line -
      vfe_params->demosaic_op_params.first_line + 1) -
      fov_win.crop_out_y) / 2;
    vfe_params->crop_info.crop_factor = vfe_params->crop_factor;
    vfe_params->crop_info.crop_first_pixel = fov_win.x;
    vfe_params->crop_info.crop_last_pixel = fov_win.x +
    fov_win.crop_out_x -1;
    vfe_params->crop_info.crop_first_line = fov_win.y;
    vfe_params->crop_info.crop_last_line = fov_win.y +
    fov_win.crop_out_y -1;
    CDBG("%s: crop_factor = %d, x = %d, y = %d, crop_out_x = %d, crop_out_y = %d",
      __func__, vfe_params->crop_factor, fov_win.x, fov_win.y,
      fov_win.crop_out_x, fov_win.crop_out_y);

  }
  return VFE_SUCCESS;
}

/*===========================================================================
 * Function:           vfe_fov_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_fov_enable(int mod_id, void* mod, void* params,
  int8_t enable, int8_t hw_write)
{
  fov_mod_t *fov_mod = (fov_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  vfe_params->moduleCfg->cropEnable = enable;
  CDBG("%s: enable %d", __func__, enable);

  if (hw_write && (fov_mod->fov_enable == enable))
    return VFE_SUCCESS;

  fov_mod->fov_enable = enable;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_FOV)
      : (vfe_params->current_config & ~VFE_MOD_FOV);
  }
  return VFE_SUCCESS;
} /* vfe_fov_enable */

/*===========================================================================
 * FUNCTION    - vfe_fov_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_init(int mod_id, void *mod_fov, void* parm)
{
  return VFE_SUCCESS;
} /* vfe_fov_init */

/*===========================================================================
 * FUNCTION    - vfe_fov_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_deinit(int mod_id, void *mod_fov, void* parm)
{
  return VFE_SUCCESS;
} /* vfe_fov_deinit */

#ifdef VFE_32
/*===========================================================================
 * FUNCTION    - vfe_fov_plugin_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_plugin_update(int module_id, void *mod,
  void *vparams)
{
  vfe_status_t status;
  fov_module_t *cmd = (fov_module_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)vparams;

  if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd,
     CMD_GENERAL, (void *)&(cmd->fov_cmd),
     sizeof(VFE_FOV_CropConfigCmdType),
     VFE_CMD_FOV_CFG)) {
     CDBG_HIGH("%s: Failed for op mode = %d failed\n", __func__,
       vfe_params->vfe_op_mode);
       return VFE_ERROR_GENERAL;
  }
  return VFE_SUCCESS;
} /* vfe_fov_plugin_update */
#endif
