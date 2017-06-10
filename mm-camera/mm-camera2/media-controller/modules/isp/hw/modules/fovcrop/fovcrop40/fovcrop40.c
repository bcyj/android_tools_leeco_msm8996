/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "camera_dbg.h"
#include "fovcrop40.h"
#include "isp_log.h"

#ifdef ENABLE_FOV_CROP_LOGGING
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE


/*#define USE_FOVCROP_FOR_SMALL_SCALE_FACTOR*/
#define MIN_DOWNSCALE_FACTOR 1.03125
#define PREVIEW_FORMAT CAMERA_YUV_420_NV12

#define CHECK_ASPECT_RATIO(x) ((1.3 <= x) && (x <= 1.34))
#define CHECK_DEFAULT_CROP_FACTOR(x) (x == (1<<12))
#define CHECK_DOWNSCALE_FACTOR(x) ((1.0 < x) && (x < MIN_DOWNSCALE_FACTOR))

typedef struct {
  uint32_t width;
  uint32_t height;
} dim_t;

typedef struct {
  dim_t y;
  dim_t cbcr;
} img_desc_t;

typedef struct {
  uint32_t left;
  uint32_t right;
  uint32_t top;
  uint32_t bottom;
} crop_desc_t;

/** vfe_fov_cmd_debug
 *    @cmd: bcc config cmd
 *    @index: pix path index
 *
 * This function dumps the bcc module register settings set to
 * hw
 *
 * Return: nothing
 **/
static void vfe_fov_cmd_debug(ISP_FOV_CropConfigCmdType cmd ,int index)
{
  ISP_DBG(ISP_MOD_FOV, "%s: FOV[%d]\n", __func__, index);

  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP[%d] Y firstPixel %d\n",
    __func__, index, cmd.y_crop_cfg.firstPixel );
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP[%d] Y lastPixel %d\n",
    __func__, index, cmd.y_crop_cfg.lastPixel );
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP[%d] Y firstLine %d\n",
    __func__, index, cmd.y_crop_cfg.firstLine );
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP[%d] Y lastLine %d\n",
    __func__, index, cmd.y_crop_cfg.lastLine );
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP[%d] CbCr firstPixel %d\n",
    __func__, index, cmd.cbcr_crop_cfg.firstPixel );
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP[%d] CbCr lastPixel %d\n",
    __func__, index, cmd.cbcr_crop_cfg.lastPixel );
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP[%d] CbCr firstLine %d\n",
    __func__, index, cmd.cbcr_crop_cfg.firstLine );
  ISP_DBG(ISP_MOD_FOV, "%s: FOV_CROP[%d] CbCr lastLine %d\n",
    __func__, index, cmd.cbcr_crop_cfg.lastLine );
} /* isp_fov_cmd_debug */

/** fov_calculate_scaler_output
 *    @M:       output
 *    @N:       input
 *    @offset:  offset
 *    @actual_input_width: actual input width
 *    @y_scaler: luma scaler
 *
 * bcc module disable,release reg settings and strcuts
 *
 * Return: Scaler putput value
 **/
uint32_t fov_calculate_scaler_output(uint32_t  M, uint32_t  N,
  uint32_t  offset, uint32_t  actual_input_width, boolean   y_scaler)
{
  uint32_t mn_init, phase_init;
  uint32_t ratio = N / M;
  uint32_t interp_reso = 3;
  if (ratio >= 16)     interp_reso = 0;
  else if (ratio >= 8) interp_reso = 1;
  else if (ratio >= 4) interp_reso = 2;

  mn_init = offset * M % N;
  phase_init = (mn_init << (13 + interp_reso)) / M;
  if (y_scaler && ((phase_init >> 13) != 0)) {
    mn_init = (offset + 1) * M % N;
    actual_input_width--;
  }
  return (mn_init + actual_input_width * M) / N;
}

/** fov_config_entry
 *    @mod:       fov module control
 *    @entry_idx: Pix path idx
 *    @pix_setting:  pix path settings
 *
 * Update entry strcuture of Fov module with first/last
 * pixel/line based on Y, CbCr format and scling factor
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_config_entry(isp_fov_mod_t *mod, int entry_idx,
  isp_hw_pix_setting_params_t *pix_setting)
{
  int rc = 0;
  isp_win_t fov_win_y;
  isp_win_t fov_win_cbcr;
  uint32_t scale_w = 1;
  uint32_t scale_h = 1;
  uint32_t camif_w, camif_h;
  cam_format_t format = pix_setting->outputs[entry_idx].stream_param.fmt;
  isp_fov_entry_t *entry = &mod->fov[entry_idx];

  if (pix_setting->outputs[entry_idx].stream_param.width == 0) {
    entry->is_used = 0;
    mod->hw_update_pending = 0;
    ISP_DBG(ISP_MOD_FOV, "%s: FOV entry %d not used", __func__, entry_idx);
    return 0;
  }

  ISP_DBG(ISP_MOD_FOV, "%s: fov_entry[%d] input from scaler: width %d, height %d\n",
    __func__, entry_idx, pix_setting->outputs[entry_idx].stream_param.width,
       pix_setting->outputs[entry_idx].stream_param.height);

  entry->is_used = 1;
  rc = mod->notify_ops->notify(mod->notify_ops->parent, mod->notify_ops->handle,
    ISP_HW_MOD_NOTIFY_FETCH_SCALER_OUTPUT,
    mod->scaler_output, sizeof(mod->scaler_output));
  if (rc < 0) {
    CDBG_ERROR("%s: no scaler output window available, rc = %d\n",
               __func__, rc);
    return rc;
  }

  /*first pixel , first line , last pix, last line*/
  entry->reg_cmd.y_crop_cfg.firstPixel =
    (mod->scaler_output[entry_idx].width -
    pix_setting->outputs[entry_idx].stream_param.width) / 2;

  entry->reg_cmd.y_crop_cfg.firstLine =
    (mod->scaler_output[entry_idx].height -
    pix_setting->outputs[entry_idx].stream_param.height) / 2;

  entry->reg_cmd.y_crop_cfg.lastPixel =
    entry->reg_cmd.y_crop_cfg.firstPixel +
    pix_setting->outputs[entry_idx].stream_param.width - 1;

  entry->reg_cmd.y_crop_cfg.lastLine =
    entry->reg_cmd.y_crop_cfg.firstLine +
    pix_setting->outputs[entry_idx].stream_param.height - 1;

  if (pix_setting->camif_cfg.is_bayer_sensor)
    camif_w = pix_setting->demosaic_output.last_pixel -
        pix_setting->demosaic_output.first_pixel + 1;
  else
    camif_w = (pix_setting->demosaic_output.last_pixel -
        pix_setting->demosaic_output.first_pixel + 1) >> 1;

  camif_h = pix_setting->demosaic_output.last_line -
      pix_setting->demosaic_output.first_line + 1;


  rc = mod->notify_ops->notify(mod->notify_ops->parent, mod->notify_ops->handle,
    ISP_HW_MOD_NOTIFY_FETCH_SCALER_CROP_REQUEST,
    mod->scaler_crop_request, sizeof(mod->scaler_crop_request));
  if (rc < 0) {
    CDBG_ERROR("%s: no scaler output window available, rc = %d\n",
               __func__, rc);
    return rc;
  }

  if (mod->scaler_crop_request[entry_idx] == 0) {
    /* Within ISP scaling range so that no extra cropping needed for zoom */
    memset(&entry->crop_window, 0, sizeof(entry->crop_window));
  } else {
    /* ISP hits scaling limitation needs extra cropping for zoom */
    uint32_t crop_width = pix_setting->outputs[entry_idx].stream_param.width;
    uint32_t crop_height = pix_setting->outputs[entry_idx].stream_param.height;
    uint32_t h_factor = mod->scaler_output[entry_idx].width * Q12;
    uint32_t v_factor = mod->scaler_output[entry_idx].height * Q12;
    uint32_t scaler_factor;

    h_factor /= crop_width;
    v_factor /= crop_height;
    scaler_factor = (h_factor > v_factor)? v_factor : h_factor;

    entry->crop_window.crop_out_x = crop_width * scaler_factor /
      pix_setting->crop_factor;
    if (entry->crop_window.crop_out_x > crop_width)
      entry->crop_window.crop_out_x = crop_width;

    entry->crop_window.crop_out_y = crop_height * scaler_factor /
      pix_setting->crop_factor;
    if (entry->crop_window.crop_out_y > crop_height)
      entry->crop_window.crop_out_y = crop_height;

    entry->crop_window.x = (crop_width - entry->crop_window.crop_out_x) >> 1;
    entry->crop_window.y = (crop_height - entry->crop_window.crop_out_y) >> 1;
  }

  /* CbCr Cropping */
  /* YUV format logic */
  switch (format) {
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_420_YV12: {
    scale_w = 2;
    scale_h = 2;
  }
    break;

  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_422_NV16: {
    scale_w = 2;
    scale_h = 1;
  }
    break;

  default: {
    CDBG_HIGH("%s: Incompatible Format: %d", __func__, format);
  }
    break;
  }

  if (pix_setting->outputs[entry_idx].need_uv_subsample) {
    scale_w *= 2;
    scale_h *= 2;
  }

  entry->reg_cmd.cbcr_crop_cfg.firstPixel =
    entry->reg_cmd.y_crop_cfg.firstPixel / scale_w;
  entry->reg_cmd.cbcr_crop_cfg.firstLine =
    entry->reg_cmd.y_crop_cfg.firstLine / scale_h;

  entry->reg_cmd.cbcr_crop_cfg.lastPixel =
    entry->reg_cmd.cbcr_crop_cfg.firstPixel +
      (pix_setting->outputs[entry_idx].stream_param.width / scale_w) -1;
  entry->reg_cmd.cbcr_crop_cfg.lastLine =
    entry->reg_cmd.cbcr_crop_cfg.firstLine +
      (pix_setting->outputs[entry_idx].stream_param.height / scale_h) -1;
  entry->hw_update_pending = TRUE;

  return rc;
} /* fov_config_entry */

/** fov_config_entry_split
 *    @mod:       fov module control
 *    @entry_idx: Pix path idx
 *    @pix_setting:  pix path settings
 *
 * Update entry strcuture of Fov module with first/last
 * pixel/line based on Y, CbCr format and scling factor
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_config_entry_split(isp_fov_mod_t *mod, int entry_idx,
  isp_hw_pix_setting_params_t *pix_setting)
{
  int rc = 0;
  isp_win_t fov_win_y;
  isp_win_t fov_win_cbcr;
  uint32_t scale_w = 1;
  uint32_t scale_h = 1;
  uint32_t sensor_out_w, sensor_out_h;
  cam_format_t format = pix_setting->outputs[entry_idx].stream_param.fmt;
  isp_fov_entry_t *entry = &mod->fov[entry_idx];

  /* for split case*/
  dim_t       apparent_in;
  img_desc_t  apparent_out;
  img_desc_t  actual_out;
  img_desc_t  num_pixels_to_crop;
  crop_desc_t crop_y;
  crop_desc_t crop_cbcr;
  uint32_t    scaler_M, scaler_N;
  uint32_t    cbcr_subsample = 2;
  uint32_t    stripe_offset =
    pix_setting->outputs[entry_idx].isp_out_info.right_stripe_offset;

  if (pix_setting->outputs[entry_idx].stream_param.width == 0) {
    entry->is_used = 0;
    mod->hw_update_pending = 0;
    ISP_DBG(ISP_MOD_FOV, "%s: FOV entry %d not used", __func__, entry_idx);
    return 0;
  }

  entry->is_used = 1;
  sensor_out_w =
    pix_setting->camif_cfg.sensor_out_info.request_crop.last_pixel -
      pix_setting->camif_cfg.sensor_out_info.request_crop.first_pixel + 1;
  sensor_out_h =
    pix_setting->camif_cfg.sensor_out_info.request_crop.last_line -
      pix_setting->camif_cfg.sensor_out_info.request_crop.first_line + 1;

  if (!pix_setting->camif_cfg.is_bayer_sensor)
    sensor_out_w >>= 1;

  /* apparent input is imaginary; it is as if FOV is cropped performed
     before scaler to achieve the zoom scaling */
  apparent_in.width = sensor_out_w * Q12 / pix_setting->crop_factor;
  apparent_in.height = sensor_out_h * Q12 / pix_setting->crop_factor;

  /* apparent output is the desired final output */
  apparent_out.y.width = pix_setting->outputs[entry_idx].stream_param.width;
  apparent_out.y.height = pix_setting->outputs[entry_idx].stream_param.height;

  switch (pix_setting->outputs[entry_idx].stream_param.fmt) {
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_422_NV16: {
    apparent_out.cbcr.width = apparent_out.y.width / 2;
    apparent_out.cbcr.height = apparent_out.y.height;
  }
    break;
  default: {
    apparent_out.cbcr.width = apparent_out.y.width / 2;
    apparent_out.cbcr.height = apparent_out.y.height / 2;
  }
    break;
  }

  if (pix_setting->outputs[entry_idx].need_uv_subsample) {
    cbcr_subsample = 4;
    apparent_out.cbcr.width /= 2;
    apparent_out.cbcr.height /= 2;
  }

  /* crop to match input and output aspect ratio */
  if (apparent_out.y.width  * apparent_in.height <
      apparent_out.y.height * apparent_in.width) {
    scaler_M = apparent_out.y.height;
    scaler_N = apparent_in.height;
    apparent_in.width = apparent_out.y.width * apparent_in.height /
      apparent_out.y.height;
  } else {
    scaler_M = apparent_out.y.width;
    scaler_N = apparent_in.width;
    apparent_in.height = apparent_out.y.height * apparent_in.width /
      apparent_out.y.width;
  }

  /* maximum zoom by ISP reached */
  if (apparent_in.width < apparent_out.y.width) {
    entry->crop_window.crop_out_x = apparent_in.width;
    entry->crop_window.crop_out_y = apparent_in.height;
    entry->crop_window.x = (apparent_out.y.width  - apparent_in.width) >> 1;
    entry->crop_window.y = (apparent_out.y.height - apparent_in.height) >> 1;
    apparent_in.width  = apparent_out.y.width;
    apparent_in.height = apparent_out.y.height;
    scaler_N = scaler_M;
  } else {
    memset(&entry->crop_window, 0, sizeof(crop_window_info_t));
  }
  actual_out.y.width = sensor_out_w * apparent_out.y.width /
    apparent_in.width;
  actual_out.y.height = sensor_out_h * apparent_out.y.height /
    apparent_in.height;
  actual_out.cbcr.width = sensor_out_w * apparent_out.cbcr.width /
    apparent_in.width;
  actual_out.cbcr.height = sensor_out_h * apparent_out.cbcr.height /
    apparent_in.height;
  num_pixels_to_crop.y.width = actual_out.y.width - apparent_out.y.width;
  num_pixels_to_crop.y.height = actual_out.y.height - apparent_out.y.height;
  num_pixels_to_crop.cbcr.width = actual_out.cbcr.width -
    apparent_out.cbcr.width;
  num_pixels_to_crop.cbcr.height = actual_out.cbcr.height -
    apparent_out.cbcr.height;
  crop_y.left = num_pixels_to_crop.y.width >> 1;
  crop_y.right = num_pixels_to_crop.y.width - crop_y.left;
  crop_y.top = num_pixels_to_crop.y.height >> 1;
  crop_cbcr.left = num_pixels_to_crop.cbcr.width >> 1;
  crop_cbcr.right = num_pixels_to_crop.cbcr.width - crop_cbcr.left;
  crop_cbcr.top = num_pixels_to_crop.cbcr.height >> 1;

  if (pix_setting->outputs[entry_idx].isp_out_info.stripe_id ==
    ISP_STRIPE_LEFT) {
    entry->reg_cmd.y_crop_cfg.firstPixel = crop_y.left;
    entry->reg_cmd.y_crop_cfg.lastPixel = crop_y.left +
      pix_setting->outputs[entry_idx].isp_out_info.left_output_width - 1;
    entry->reg_cmd.cbcr_crop_cfg.firstPixel = crop_cbcr.left;
    entry->reg_cmd.cbcr_crop_cfg.lastPixel = crop_cbcr.left +
      pix_setting->outputs[entry_idx].isp_out_info.left_output_width /
      cbcr_subsample - 1;
  } else {
    uint32_t actual_out_y =
      fov_calculate_scaler_output(scaler_M, scaler_N,
        stripe_offset, sensor_out_w - stripe_offset, 1);
    uint32_t actual_out_cbcr =
      fov_calculate_scaler_output(scaler_M / cbcr_subsample, scaler_N,
        stripe_offset, sensor_out_w - stripe_offset, 0);

    entry->reg_cmd.y_crop_cfg.lastPixel =
      actual_out_y - crop_y.right - 1;
    entry->reg_cmd.y_crop_cfg.firstPixel =
      entry->reg_cmd.y_crop_cfg.lastPixel -
        pix_setting->outputs[entry_idx].isp_out_info.right_output_width + 1;
    entry->reg_cmd.cbcr_crop_cfg.lastPixel =
      actual_out_cbcr - crop_cbcr.right - 1;
    entry->reg_cmd.cbcr_crop_cfg.firstPixel =
      entry->reg_cmd.cbcr_crop_cfg.lastPixel -
        pix_setting->outputs[entry_idx].isp_out_info.right_output_width /
        cbcr_subsample + 1;
  }

  entry->reg_cmd.y_crop_cfg.firstLine = crop_y.top;
  entry->reg_cmd.y_crop_cfg.lastLine =
    crop_y.top + apparent_out.y.height - 1;
  entry->reg_cmd.cbcr_crop_cfg.firstLine  = crop_cbcr.top;
  entry->reg_cmd.cbcr_crop_cfg.lastLine =
    crop_cbcr.top + apparent_out.cbcr.height - 1;

  entry->hw_update_pending = TRUE;
  return rc;
}

/** fov_config
 *    @mod: fov module struct data
 *    @pix_setting: hw pixel settings
 *    @in_param_size: input params struct size
 *
 * Based on single / dual vfe need a special algo to
 * calculate/split fov config
 *
 * Return: 0 - sucess and negative value - failure
 **/
static int fov_config(isp_fov_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_setting, uint32_t in_param_size)
{
  int rc = 0;
  int i;

  ISP_DBG(ISP_MOD_FOV, "%s\n",__func__);

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
  return -1;
  }

  if (!mod->fov_enable) {
    /* not enabled no op */
    return rc;
  }

  /*dual vfe need a special algo to calculate/split fov config*/
  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
     if(!pix_setting->camif_cfg.ispif_out_info.is_split)
       rc = fov_config_entry(mod, i, pix_setting);
     else
       rc = fov_config_entry_split(mod, i, pix_setting);

     if (rc < 0) {
       CDBG_ERROR("%s: fov_config_entry error, idx = %d, rc = %d",
         __func__, i, rc);
       return rc;
     }
  }
  mod->old_streaming_mode = pix_setting->streaming_mode;
  mod->hw_update_pending = TRUE;
  /* update fov state for initial configuration.
   * run time trigger update/zoom ratio change
   * also uses this function */

  return rc;
}

/** fov_trigger_update
 *    @mod: fov module control struct
 *    @trigger_input: contains info about demosaic/stats update
 *                  flag,  digital gains etc.
 *    @in_param_size: input params struct size
 *
 *  fov module modify reg settings as per new input params and
 *  trigger hw update
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_trigger_update(isp_fov_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_input, uint32_t in_param_size)
{
  int rc = 0;

  int is_burst = IS_BURST_STREAMING(trigger_input);
  camera_flash_type new_flash_mode;

  chromatix_parms_type *chrPtr =
     (chromatix_parms_type *)trigger_input->cfg.chromatix_ptrs.chromatixPtr;


  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_pix_trigger_update_input_t), in_param_size);
    return -1;
  }

  if (!mod->fov_enable) {
    ISP_DBG(ISP_MOD_FOV, "%s: Pca Rolloff is disabled. Skip the trigger.\n", __func__);
    return 0;
  }

  if (!mod->fov_trigger_enable) {
    ISP_DBG(ISP_MOD_FOV, "%s: Trigger is disable. Skip the trigger update.\n", __func__);
    return 0;
  }

  rc = fov_config(mod, &trigger_input->cfg, sizeof(trigger_input->cfg));

  return rc;
} /* fov_trigger_update */

/** fov_set_zoom_ratio
 *    @fov: bcc module control struct
 *    @pix_setting: contains info about demosaic/stats update
 *                  flag,  digital gains etc.
 *    @in_param_size: input params struct size
 *
 *  Configure Fov module absed on zoom ratio
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_set_zoom_ratio(isp_fov_mod_t *fov,
  isp_hw_pix_setting_params_t *pix_setting, uint32_t in_param_size)
{
  int rc = 0;
  rc = fov_config(fov, pix_setting, in_param_size);

  return rc;
}

/** fov_reset
 *      @mod: bcc module struct data
 *
 * fov module disable hw updates,release reg settings and
 * structs
 *
 * Return: nothing
 **/
static void fov_reset(isp_fov_mod_t *mod)
{
  int i;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->scaler_output, 0, sizeof(mod->scaler_output));
  for (i = 0; i < ISP_PIX_PATH_MAX; i++)
    memset(&mod->fov[i],  0, sizeof(mod->fov[i]));
  mod->hw_update_pending = 0;
  mod->fov_trigger_enable = 0;
  mod->fov_update = 0;
  mod->fov_enable = 0;
}

/** fov_init
 *    @mod_ctrl: fov module control strcut
 *    @in_params: fov hw module init params
 *    @notify_ops: fn pointer to notify other modules
 *
 *  fov module data struct initialization
 *
 * Return: 0 always
 **/
static int fov_init (void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_fov_mod_t *fov = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  fov->fd = init_params->fd;
  fov->notify_ops = notify_ops;
  fov->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  fov->hw_update_pending = FALSE;
  fov_reset(fov);
  return 0;
}/* fov_init */

/** fov_enable
 *    @mod: bcc module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input param struct size
 *
 *  fov module enable/disable method
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_enable(isp_fov_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  int i;
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  mod->fov_enable = enable->enable;
  if (!mod->fov_enable) {
    /* set all scaler entries to not used */
    for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
      mod->fov[i].hw_update_pending = 0;
      mod->fov[i].is_used = 0;
    }
  }
  return 0;
}

/** fov_trigger_enable
 *    @mod: fov module control struct
 *    @enable: module enable/disable flag
 *    @in_param_size: input params struct size
 *
 *  fov module enable hw update trigger feature
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_trigger_enable(isp_fov_mod_t *mod, isp_mod_set_enable_t *enable,
  uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }
  mod->fov_trigger_enable = enable->enable;

  return 0;
}

/** fov_get_fov_crop
 *    @mod: fov module control struct
 *    @pix_setting : contains info about demosaic/stats update
 *                  flag,  digital gains etc.
 *    @hw_zoom_param: zoom information
 *
 *  Get/store information about crop window, width, height etc.
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_get_fov_crop(isp_fov_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_setting, isp_hw_zoom_param_t *hw_zoom_param)
{
  int i;
  isp_fov_entry_t *entry;

  memset(hw_zoom_param,  0,  sizeof(isp_hw_zoom_param_t));
  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
    entry = &mod->fov[i];
    if (pix_setting->outputs[i].stream_param.width) {
      /* the output is used, fill in stream info */
      hw_zoom_param->entry[hw_zoom_param->num].dim.width =
        pix_setting->outputs[i].stream_param.width;
      hw_zoom_param->entry[hw_zoom_param->num].dim.height =
        pix_setting->outputs[i].stream_param.height;
      hw_zoom_param->entry[hw_zoom_param->num].stream_id =
        pix_setting->outputs[i].stream_param.stream_id;
      /* get fov first pix, first line, delta_x and delta_y*/
      hw_zoom_param->entry[hw_zoom_param->num].crop_win = entry->crop_window;
      hw_zoom_param->num++;
    }
  }
  return 0;
}

/** fov_destroy
 *    @mod_ctrl: bcc module control strcut
 *
 *  Close fov module
 *
 * Return: 0 always
 **/
static int fov_destroy (void *mod_ctrl)
{
  isp_fov_mod_t *fov = mod_ctrl;

  memset(fov,  0,  sizeof(isp_fov_mod_t));
  free(fov);
  return 0;
}

/** fov_set_params
 *    @mod_ctrl: fov module control struct
 *    @param_id : param enum index
 *    @in_params: input config params based on param idex
 *    @in_param_size: input params struct size
 *
 *  set config params like mod enable, zoom, trigger update, mod
 *  config etc. utility to update fov module
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_set_params (void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_fov_mod_t *fov = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE: {
    rc = fov_enable(fov, (isp_mod_set_enable_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_MOD_CONFIG: {
    rc = fov_config(fov, (isp_hw_pix_setting_params_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_ENABLE: {
    rc = fov_trigger_enable(fov, (isp_mod_set_enable_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_TRIGGER_UPDATE: {
    rc = fov_trigger_update(fov, (isp_pix_trigger_update_input_t *)in_params,
           in_param_size);
  }
    break;

  case ISP_HW_MOD_SET_ZOOM_RATIO: {
    rc = fov_set_zoom_ratio(fov, (isp_hw_pix_setting_params_t *)in_params,
           in_param_size);
  }
    break;

  default: {
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
  }
    break;
  }
  return rc;
}

/** fov_get_params
 *    @mod_ctrl: fov module control struct
 *    @param_id : param enum index
 *    @in_params: input config params based on param idex
 *    @in_param_size: input params struct size
 *    @out_params: struct to return out params
 *    @out_param_size: output params struct size
 *
 *  Get config params utility to fetch config of fov module
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_get_params (void *mod_ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  int rc = 0;
  isp_fov_mod_t *fov = mod_ctrl;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = fov->fov_enable;
  }
      break;

  case ISP_HW_MOD_GET_FOV: {
    rc = fov_get_fov_crop(fov, (isp_hw_pix_setting_params_t *)in_params,
      (isp_hw_zoom_param_t *)out_params);
  }
    break;

  case ISP_PIX_GET_FOV_OUTPUT: {
    int i;
    isp_pixel_line_info_t *fov_output = out_params;
    for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
      if (fov->fov[i].is_used) {
        fov_output[i].first_pixel = fov->fov[i].reg_cmd.y_crop_cfg.firstPixel;
        fov_output[i].last_pixel = fov->fov[i].reg_cmd.y_crop_cfg.lastPixel;
        fov_output[i].first_line = fov->fov[i].reg_cmd.y_crop_cfg.firstLine;
        fov_output[i].last_line = fov->fov[i].reg_cmd.y_crop_cfg.lastLine;
        ISP_DBG(ISP_MOD_FOV, "%s:fov[%d]:first_pix %d, last_pix %d, first_ln %d, last_ln %d\n",
           __func__, i, fov_output[i].first_pixel, fov_output[i].last_pixel,
           fov_output[i].first_line, fov_output[i].last_line);
      }
    }
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
    ISP_DBG(ISP_MOD_FOV, "%s: Populating vfe_diag data", __func__);
  }
    break;

  default: {
    rc = -EPERM;
  }
    break;
  }
  return rc;
}

/** fov_do_hw_update
 *    @fov_mod: bcc module struct data
 *
 * update fov module register to kernel
 *
 * Return: nothing
 **/
static int fov_do_hw_update(isp_fov_mod_t *fov_mod)
{
  int i, rc = 0;

  ISP_DBG(ISP_MOD_FOV, "%s: HW_update, FOV[0] = %d, FOV[1] = %d\n", __func__,
    fov_mod->fov[0].hw_update_pending,
    fov_mod->fov[1].hw_update_pending);

  for (i = 0; i< ISP_PIX_PATH_MAX; i++) {
    if (fov_mod->fov[i].hw_update_pending) {
      vfe_fov_cmd_debug(fov_mod->fov[i].reg_cmd, i);

      if (i == ISP_PIX_PATH_ENCODER) {
        rc = isp_pipeline_util_single_HW_write(fov_mod->fd,
          (void *)&fov_mod->fov[i].reg_cmd, sizeof(fov_mod->fov[i].reg_cmd),
          ISP_FOV40_ENC_OFF, ISP_FOV40_ENC_LEN, VFE_WRITE);
      } else {
        rc = isp_pipeline_util_single_HW_write(fov_mod->fd,
          (void *)&fov_mod->fov[i].reg_cmd, sizeof(fov_mod->fov[i].reg_cmd),
          ISP_FOV40_VIEW_OFF, ISP_FOV40_VIEW_LEN, VFE_WRITE);
      }
      fov_mod->fov[i].hw_update_pending = 0;
    }
  }

  return rc;
}

/** fov_action
 *    @mod_ctrl: scaler module control struct
 *    @action_code : action code
 *    @data: not used
 *    @data_size: not used
 *
 *  processing the hw action like update or reset
 *
 * Return: 0 - success and negative value - failure
 **/
static int fov_action (void *mod_ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_fov_mod_t *fov = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE: {
    rc = fov_do_hw_update(fov);
  }
    break;

  case ISP_HW_MOD_ACTION_RESET: {
    fov_reset(fov);
  }
    break;

  default: {
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
  }
    break;
  }
  return rc;
}

/** fov40_open
 *    @version: hw version
 *
 *  fov 40 module open and create func table
 *
 * Return: fov module ops struct pointer
 **/
isp_ops_t *fov40_open(uint32_t version)
{
  isp_fov_mod_t *fov = malloc(sizeof(isp_fov_mod_t));

  if (!fov) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(fov,  0,  sizeof(isp_fov_mod_t));
  fov->ops.ctrl = (void *)fov;
  fov->ops.init = fov_init;
  /* destroy the module object */
  fov->ops.destroy = fov_destroy;
  /* set parameter */
  fov->ops.set_params = fov_set_params;
  /* get parameter */
  fov->ops.get_params = fov_get_params;
  fov->ops.action = fov_action;
  return &fov->ops;
}
