/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "vfe.h"

#ifdef ENABLE_FOV_LOGGING
  #undef CDBG
  #define CDBG LOGE
#endif

//#define USE_FOVCROP_FOR_SMALL_SCALE_FACTOR
#define MIN_DOWNSCALE_FACTOR 1.03125
#define PREVIEW_FORMAT CAMERA_YUV_420_NV12

#define CHECK_ASPECT_RATIO(x) ((1.3 <= x) && (x <= 1.34))
#define CHECK_DEFAULT_CROP_FACTOR(x) (x == (1<<12))
#define CHECK_DOWNSCALE_FACTOR(x) ((1.0 < x) && (x < MIN_DOWNSCALE_FACTOR))

/*===========================================================================
 * FUNCTION    - vfe_fov_cmd_debug -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_cmd_debug(VFE_FOV_CropConfigCmdType cmd ,int type)
{
  CDBG("%s: FOV cmd is type : %d ", __func__, type);

  CDBG("%s: type %d y_crop_cfg.firstPixel %d ",
    __func__, type, cmd.y_crop_cfg.firstPixel );
  CDBG("%s: type %d y_crop_cfg.lastPixel %d ",
    __func__, type, cmd.y_crop_cfg.lastPixel );
  CDBG("%s: type %d y_crop_cfg.firstLine %d ",
    __func__, type, cmd.y_crop_cfg.firstLine );
  CDBG("%s: type %d y_crop_cfg.lastLine %d ",
    __func__, type, cmd.y_crop_cfg.lastLine );
  CDBG("%s: type %d cbcr_crop_cfg.firstPixel %d ",
    __func__, type, cmd.cbcr_crop_cfg.firstPixel );
  CDBG("%s: type %d cbcr_crop_cfg.lastPixel %d ",
    __func__, type, cmd.cbcr_crop_cfg.lastPixel );
  CDBG("%s: type %d cbcr_crop_cfg.firstLine %d ",
    __func__, type, cmd.cbcr_crop_cfg.firstLine );
  CDBG("%s: type %d cbcr_crop_cfg.lastLine %d ",
    __func__, type, cmd.cbcr_crop_cfg.lastLine );

  return VFE_SUCCESS;
} /* vfe_fov_cmd_debug */

/*===========================================================================
 * FUNCTION    - vfe_fov_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_fov_config(int mod_id, void *mod, void *params)
{
  fov_mod_t *fov_mod = (fov_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  if (!fov_mod->fov_enc_enable && !fov_mod->fov_view_enable) {
    CDBG("%s: fov not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!fov_mod->fov_enc_enable)
    CDBG_HIGH("%s: fov_enc not enable, not write to HW", __func__);
  else {
    if(VFE_SUCCESS != vfe_crop_enc_config(params))
      CDBG_ERROR("%s: cannot config Enc cropping", __func__);

    fov_mod->fov_enc_cmd.y_crop_cfg.firstPixel =
      vfe_params->crop_ops_params[OUT_ENCODER].y.crop_first_pixel;
    fov_mod->fov_enc_cmd.y_crop_cfg.lastPixel =
      vfe_params->crop_ops_params[OUT_ENCODER].y.crop_last_pixel;
    fov_mod->fov_enc_cmd.y_crop_cfg.firstLine =
      vfe_params->crop_ops_params[OUT_ENCODER].y.crop_first_line;
    fov_mod->fov_enc_cmd.y_crop_cfg.lastLine =
      vfe_params->crop_ops_params[OUT_ENCODER].y.crop_last_line;

    fov_mod->fov_enc_cmd.cbcr_crop_cfg.firstPixel =
      vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_first_pixel;
    fov_mod->fov_enc_cmd.cbcr_crop_cfg.lastPixel =
      vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_last_pixel;
    fov_mod->fov_enc_cmd.cbcr_crop_cfg.firstLine =
      vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_first_line;
    fov_mod->fov_enc_cmd.cbcr_crop_cfg.lastLine =
      vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_last_line;

    vfe_fov_cmd_debug(fov_mod->fov_enc_cmd , ENCODER);

    if(VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *)&fov_mod->fov_enc_cmd, sizeof(fov_mod->fov_enc_cmd),
      VFE_CMD_FOV_ENC_CFG)){
      CDBG_HIGH("%s: fov_enc config for operation mode = %d failed\n", __func__,
        vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
    }
  }

  if (!fov_mod->fov_view_enable)
    CDBG_HIGH("%s: fov_view not enable, not write to HW", __func__);
  else {
    if(VFE_SUCCESS != vfe_crop_view_config(params))
      CDBG_ERROR("%s: cannot config Enc cropping", __func__);

    fov_mod->fov_view_cmd.y_crop_cfg.firstPixel =
      vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_first_pixel;
    fov_mod->fov_view_cmd.y_crop_cfg.lastPixel =
      vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_last_pixel;
    fov_mod->fov_view_cmd.y_crop_cfg.firstLine =
      vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_first_line;
    fov_mod->fov_view_cmd.y_crop_cfg.lastLine =
      vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_last_line;

    fov_mod->fov_view_cmd.cbcr_crop_cfg.firstPixel =
      vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_first_pixel;
    fov_mod->fov_view_cmd.cbcr_crop_cfg.lastPixel =
      vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_last_pixel;
    fov_mod->fov_view_cmd.cbcr_crop_cfg.firstLine =
      vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_first_line;
    fov_mod->fov_view_cmd.cbcr_crop_cfg.lastLine =
      vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_last_line;

    vfe_fov_cmd_debug(fov_mod->fov_view_cmd , VIEWFINER);

    if(VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *)&fov_mod->fov_view_cmd, sizeof(fov_mod->fov_view_cmd),
      VFE_CMD_FOV_VIEW_CFG)) {
        CDBG_HIGH("%s: fov_view config for operation mode = %d failed\n", __func__,
        vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL; ;;
    }
  }
  return VFE_SUCCESS;
} /* vfe_fov_config */

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

  if ((!fov_mod->fov_enc_enable) && (!fov_mod->fov_view_enable) ) {
    CDBG("%s: fov not enabled", __func__);
    return VFE_SUCCESS;
  }

  if (!fov_mod->fov_update) {
    CDBG("%s: fov no need to update", __func__);
    return VFE_SUCCESS;
  }

  if (!fov_mod->fov_enc_enable)
    CDBG_HIGH("%s: fov_enc not enable, not write to HW", __func__);
  else {
    if(VFE_SUCCESS != vfe_crop_enc_config(params))
      CDBG_ERROR("%s: cannot config Enc cropping", __func__);

    fov_mod->fov_enc_cmd.y_crop_cfg.firstPixel =
      vfe_params->crop_ops_params[OUT_ENCODER].y.crop_first_pixel;
    fov_mod->fov_enc_cmd.y_crop_cfg.lastPixel =
      vfe_params->crop_ops_params[OUT_ENCODER].y.crop_last_pixel;
    fov_mod->fov_enc_cmd.y_crop_cfg.firstLine =
      vfe_params->crop_ops_params[OUT_ENCODER].y.crop_first_line;
    fov_mod->fov_enc_cmd.y_crop_cfg.lastLine =
      vfe_params->crop_ops_params[OUT_ENCODER].y.crop_last_line;

    fov_mod->fov_enc_cmd.cbcr_crop_cfg.firstPixel =
      vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_first_pixel;
    fov_mod->fov_enc_cmd.cbcr_crop_cfg.lastPixel =
      vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_last_pixel;
    fov_mod->fov_enc_cmd.cbcr_crop_cfg.firstLine =
      vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_first_line;
    fov_mod->fov_enc_cmd.cbcr_crop_cfg.lastLine =
      vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_last_line;

    vfe_fov_cmd_debug(fov_mod->fov_enc_cmd , ENCODER);

    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *)&fov_mod->fov_enc_cmd, sizeof(fov_mod->fov_enc_cmd),
      VFE_CMD_FOV_ENC_CFG)) {
      CDBG_HIGH("%s: fov_enc config for operation mode = %d failed\n", __func__,
        vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
    }
  }

  if (!fov_mod->fov_view_enable)
    CDBG_HIGH("%s: fov_view not enable, not write to HW", __func__);
  else {
    if(VFE_SUCCESS != vfe_crop_view_config(params))
      CDBG_ERROR("%s: cannot config Enc cropping", __func__);

    fov_mod->fov_view_cmd.y_crop_cfg.firstPixel =
      vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_first_pixel;
    fov_mod->fov_view_cmd.y_crop_cfg.lastPixel =
      vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_last_pixel;
    fov_mod->fov_view_cmd.y_crop_cfg.firstLine =
      vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_first_line;
    fov_mod->fov_view_cmd.y_crop_cfg.lastLine =
      vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_last_line;

    fov_mod->fov_view_cmd.cbcr_crop_cfg.firstPixel =
      vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_first_pixel;
    fov_mod->fov_view_cmd.cbcr_crop_cfg.lastPixel =
      vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_last_pixel;
    fov_mod->fov_view_cmd.cbcr_crop_cfg.firstLine =
      vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_first_line;
    fov_mod->fov_view_cmd.cbcr_crop_cfg.lastLine =
      vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_last_line;

    vfe_fov_cmd_debug(fov_mod->fov_view_cmd , VIEWFINER);

    if (VFE_SUCCESS != vfe_util_write_hw_cmd(vfe_params->camfd, CMD_GENERAL,
      (void *)&fov_mod->fov_view_cmd, sizeof(fov_mod->fov_view_cmd),
      VFE_CMD_FOV_VIEW_CFG)) {
      CDBG_HIGH("%s: fov_view config for operation mode = %d failed\n", __func__,
        vfe_params->vfe_op_mode);
      return VFE_ERROR_GENERAL;
    }
  }

  vfe_params->update |= VFE_MOD_FOV;
  fov_mod->fov_update = FALSE;

  return VFE_SUCCESS;
} /* vfe_fov_update */

/*===========================================================================
 * FUNCTION    - vfe_crop_enc_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_crop_enc_config(void *params)
{
  crop_window_info_t fov_win_y;
  crop_window_info_t fov_win_cbcr;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  cam_format_t format = vfe_params->enc_format;

  uint32_t first_px = vfe_params->scaler_op_params[OUT_ENCODER].first_pixel;
  uint32_t last_px = vfe_params->scaler_op_params[OUT_ENCODER].last_pixel;
  uint32_t first_ln = vfe_params->scaler_op_params[OUT_ENCODER].first_line;
  uint32_t last_ln = vfe_params->scaler_op_params[OUT_ENCODER].last_line;
  uint32_t first_px_cbcr, last_px_cbcr, first_ln_cbcr, last_ln_cbcr;
  uint32_t scale_w = 1;
  uint32_t scale_h = 1;

  CDBG("%s: ENC Y input: firstpx: %d, lastpx: %d, firstln: %d, lastln: %d",
    __func__, first_px, last_px, first_ln, last_ln);

  /* crop window for y channel*/
  fov_win_y.crop_out_x = vfe_params->output2w;
  fov_win_y.crop_out_y = vfe_params->output2h;
  /*first pixel , first line */
  fov_win_y.x = ((last_px - first_px + 1) - fov_win_y.crop_out_x) / 2;
  fov_win_y.y = ((last_ln - first_ln + 1) - fov_win_y.crop_out_y) / 2;

  /* need to edit vfe param, add crop info for ycbcr for enc and view*/
  vfe_params->crop_ops_params[OUT_ENCODER].y.crop_first_pixel = fov_win_y.x;
  vfe_params->crop_ops_params[OUT_ENCODER].y.crop_last_pixel = fov_win_y.x +
    fov_win_y.crop_out_x - 1;
  vfe_params->crop_ops_params[OUT_ENCODER].y.crop_first_line = fov_win_y.y;
  vfe_params->crop_ops_params[OUT_ENCODER].y.crop_last_line = fov_win_y.y +
    fov_win_y.crop_out_y - 1;

  vfe_params->crop_info.crop_out_x = fov_win_y.crop_out_x;
  vfe_params->crop_info.crop_out_y = fov_win_y.crop_out_y;

  /* CbCr Cropping */
  /* YUV format logic */
  switch(format) {
    case(CAMERA_YUV_420_NV12):
    case(CAMERA_YUV_420_NV21):
    case(CAMERA_YUV_420_NV21_ADRENO):
    case(CAMERA_YUV_420_YV12):
      scale_w = 2;
      scale_h = 2;
      break;
    case(CAMERA_YUV_422_NV61):
    case(CAMERA_YUV_422_NV16):
      scale_w = 2;
      scale_h = 1;
      break;
    default:
      CDBG_HIGH("%s: Incompatible Format: %d", __func__, format);
  }
  /* crop window for cbcr channel */
  fov_win_cbcr.crop_out_x = vfe_params->output2w / scale_w;
  fov_win_cbcr.crop_out_y = vfe_params->output2h / scale_h;

  fov_win_cbcr.x = fov_win_y.x / scale_w;
  fov_win_cbcr.y = fov_win_y.y / scale_h;

  vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_first_pixel =
    fov_win_cbcr.x;
  vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_last_pixel =
    fov_win_cbcr.x + fov_win_cbcr.crop_out_x - 1;
  vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_first_line =
    fov_win_cbcr.y;
  vfe_params->crop_ops_params[OUT_ENCODER].cbcr.crop_last_line =
    fov_win_cbcr.y + fov_win_cbcr.crop_out_y - 1;

  return VFE_SUCCESS;
} /* vfe_crop_enc_config */

/*===========================================================================
 * FUNCTION    - vfe_crop_view_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_crop_view_config(void *params)
{
  crop_window_info_t fov_win_y;
  crop_window_info_t fov_win_cbcr;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  cam_format_t format;

  uint32_t first_px = vfe_params->scaler_op_params[OUT_PREVIEW].first_pixel;
  uint32_t last_px = vfe_params->scaler_op_params[OUT_PREVIEW].last_pixel;
  uint32_t first_ln = vfe_params->scaler_op_params[OUT_PREVIEW].first_line;
  uint32_t last_ln = vfe_params->scaler_op_params[OUT_PREVIEW].last_line;
  uint32_t first_px_cbcr, last_px_cbcr, first_ln_cbcr, last_ln_cbcr ;
  uint32_t scale_w = 1;
  uint32_t scale_h = 1;

  CDBG("%s: VIEW Y input: firstpx: %d, lastpx: %d, firstln: %d, lastln: %d",
    __func__, first_px, last_px, first_ln, last_ln);

  /* crop window for y channel */
  fov_win_y.crop_out_x = vfe_params->output1w;
  fov_win_y.crop_out_y = vfe_params->output1h;
  /*first pixel , first line */
  fov_win_y.x = ((last_px - first_px + 1) - fov_win_y.crop_out_x) / 2;
  fov_win_y.y = ((last_ln - first_ln + 1) - fov_win_y.crop_out_y) / 2;

  /* need to edit vfe param, add crop info for ycbcr for enc and view */
  vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_first_pixel = fov_win_y.x;
  vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_last_pixel = fov_win_y.x +
    fov_win_y.crop_out_x - 1;
  vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_first_line = fov_win_y.y;
  vfe_params->crop_ops_params[OUT_PREVIEW].y.crop_last_line = fov_win_y.y +
    fov_win_y.crop_out_y - 1;

  /* CbCr Cropping*/
  /* YUV format logic */
  /*view finder always use 420s */
  format = PREVIEW_FORMAT;

  switch(format) {
    case(CAMERA_YUV_420_NV12):
    case(CAMERA_YUV_420_NV21):
    case(CAMERA_YUV_420_NV21_ADRENO):
    case(CAMERA_YUV_420_YV12):
      scale_w = 2;
      scale_h = 2;
      break;
    case(CAMERA_YUV_422_NV61):
    case(CAMERA_YUV_422_NV16):
      scale_w = 1;
      scale_h = 2;
      break;
    default:
      CDBG_HIGH("%s: Incompatible Format: %d", __func__, format);
  }
  /* crop window for CbCr channel */
  fov_win_cbcr.crop_out_x = vfe_params->output1w / scale_w;
  fov_win_cbcr.crop_out_y = vfe_params->output1h / scale_h;

  fov_win_cbcr.x = fov_win_y.x / scale_w;
  fov_win_cbcr.y = fov_win_y.y / scale_h;

  vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_first_pixel =
    fov_win_cbcr.x;
  vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_last_pixel =
    fov_win_cbcr.x + fov_win_cbcr.crop_out_x - 1;
  vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_first_line =
    fov_win_cbcr.y;
  vfe_params->crop_ops_params[OUT_PREVIEW].cbcr.crop_last_line =
    fov_win_cbcr.y + fov_win_cbcr.crop_out_y - 1;

  return VFE_SUCCESS;
} /* vfe_crop_view_config */

/*===========================================================================
 * Function:           vfe_fov_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_fov_enable(int mod_id, void *fov_mod, void *params,
  int8_t enable, int8_t hw_write)
{
  fov_mod_t *mod = (fov_mod_t *)fov_mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;
  vfe_status_t status = VFE_SUCCESS;
  CDBG("%s: enable = %d", __func__, enable);

  /* TODO : Please Review this logic*/
  switch (vfe_params->vfe_op_mode) {
    case VFE_OP_MODE_VIDEO:
    case VFE_OP_MODE_ZSL:
    case VFE_OP_MODE_SNAPSHOT:
    case VFE_OP_MODE_JPEG_SNAPSHOT:
      status = vfe_fov_enc_enable(mod_id, mod, params, enable, hw_write);
      status = vfe_fov_view_enable(mod_id, mod, params, enable, hw_write);
      break;
    case VFE_OP_MODE_RAW_SNAPSHOT:
      status = vfe_fov_enc_enable(mod_id, mod, params, enable, hw_write);
      break;
    case VFE_OP_MODE_PREVIEW:
      status = vfe_fov_enc_enable(mod_id, mod, params, enable, hw_write);
      break;
    default:
      CDBG("%s: Invalid Mode: %d", __func__, vfe_params->vfe_op_mode);
      break;
  }

  return VFE_SUCCESS;
} /* vfe_fov_enable */

/*===========================================================================
 * Function:           vfe_fov_enc_enable
 *
 * Description:    TODO: need to add different enable function in VFe pipeline
 *=========================================================================*/
vfe_status_t vfe_fov_enc_enable(int mod_id, void* mod, void* params,
  int8_t enable, int8_t hw_write)
{
  fov_mod_t *fov_mod = (fov_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  vfe_params->moduleCfg->cropEncEnable = enable;
  CDBG("%s: enable %d", __func__, enable);

  if (hw_write && (fov_mod->fov_enc_enable == enable))
    return VFE_SUCCESS;

  fov_mod->fov_enc_enable = enable;

  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_FOV_ENC)
      : (vfe_params->current_config & ~VFE_MOD_FOV_ENC);
  }
  return VFE_SUCCESS;
} /* vfe_fov_enc_enable */

/*===========================================================================
 * Function:           vfe_fov_view_enable
 *
 * Description:
 *=========================================================================*/
vfe_status_t vfe_fov_view_enable(int mod_id, void* mod, void* params,
  int8_t enable, int8_t hw_write)
{
  fov_mod_t *fov_mod = (fov_mod_t *)mod;
  vfe_params_t *vfe_params = (vfe_params_t *)params;

  vfe_params->moduleCfg->cropViewEnable = enable;
  CDBG("%s: enable %d", __func__, enable);

  if (hw_write && (fov_mod->fov_view_enable == enable))
    return VFE_SUCCESS;

  fov_mod->fov_view_enable = enable;
  if (hw_write) {
    vfe_params->current_config = (enable) ?
      (vfe_params->current_config | VFE_MOD_FOV)
      : (vfe_params->current_config & ~VFE_MOD_FOV);
  }
  return VFE_SUCCESS;
} /* vfe_fov_view_enable */

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
