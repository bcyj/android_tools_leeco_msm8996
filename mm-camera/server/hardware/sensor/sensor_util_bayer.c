/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "sensor_util_bayer.h"
#include "camera_dbg.h"

#ifdef SENSOR_DEBUG
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - sensor_util_power_up1 -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_power_up(void *sctrl)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  CDBG("%s enter\n", __func__);

  cfg.cfgtype = CFG_POWER_UP;
  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return -EIO;
  }

  return 0;
}

/*===========================================================================
 * FUNCTION    - sensor_util_get_output_csi_info -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_output_csi_info(void *sctrl)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  struct csi_lane_params_t *csi_lane_params = NULL;
  int index = 0;
  if (ctrl->sfd <= 0)
    return FALSE;

  cfg.cfgtype = CFG_GET_CSI_PARAMS;
  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
    CDBG("%s failed %d\n", __func__, __LINE__);
    return 0;
  }

  ctrl->sensor.sensor_csi_params.csi_lane_params = cfg.cfg.csi_lane_params;
  csi_lane_params = &ctrl->sensor.sensor_csi_params.csi_lane_params;
  CDBG("%s csi lane params, lane assign = 0x%x, lane mask = 0x%x, csi = %d\n",
    __func__, csi_lane_params->csi_lane_assign, csi_lane_params->csi_lane_mask,
    csi_lane_params->csi_if);
    for (index = 0; index < csi_lane_params->csi_if; index ++)
      CDBG("%s csi_lane_params, csid_core[%d] = %d\n", __func__,
        index, csi_lane_params->csid_core[index]);

  CDBG("%s exit\n", __func__);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_util_config -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_config(void *sctrl)
{
  int32_t rc = 0;
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  uint16_t index = 0;
  CDBG("%s enter\n", __func__);

  if (ctrl->sfd <= 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  for (index = 0; index < ctrl->driver_params->init_settings_size; index++) {
    cfg.cfg.setting = &ctrl->driver_params->init_settings[index];
    cfg.mode = ctrl->sensor.cam_mode;
    if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
      CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
      return -EIO;
    }
  }
  CDBG("%s exit\n", __func__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_set_frame_rate(void *sctrl, uint16_t fps)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  struct sensor_cfg_data cfg;
  uint32_t fps_divider;
  CDBG("%s enter\n", __func__);

  if (ctrl->sfd <= 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  fps = fps * Q8;
  fps_divider =	sensor_util_bayer_get_max_fps(sctrl,
    ctrl->sensor.op_mode) * Q10 / fps;

  if (fps_divider < Q10) {
    fps_divider = Q10;
  }

  CDBG("%s: fps %d\n", __func__, fps);
  CDBG("%s: fps_divider %d\n", __func__, fps_divider);

  ctrl->sensor.current_fps = fps;
  ctrl->sensor.current_fps_div = fps_divider;
  CDBG("%s exit\n", __func__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - sensor_util_get_snapshot_fps -
 *
 * DESCRIPTION:
 *==========================================================================*/
uint32_t sensor_util_bayer_get_max_fps(void *sctrl, uint8_t mode)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  CDBG("%s enter\n", __func__);

  if (ctrl->sfd <= 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  uint32_t res = ctrl->sensor.mode_res[mode];
  CDBG("%s: pixel_clk: %d\n", __func__,
    ctrl->driver_params->output_info[res].vt_pixel_clk);
  CDBG("%s: frame_length_lines: %d\n", __func__,
    ctrl->driver_params->output_info[res].frame_length_lines);
  CDBG("%s: line_length_pclk: %d\n", __func__,
    ctrl->driver_params->output_info[res].line_length_pclk);
  uint32_t fps =
	    (uint32_t)((float) ctrl->driver_params->output_info[res].vt_pixel_clk /
	   ctrl->driver_params->output_info[res].frame_length_lines /
	   ctrl->driver_params->output_info[res].line_length_pclk * Q8);

  CDBG("%s: fps: %d\n", __func__, fps);

  return fps;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_set_op_mode(void *sctrl, uint8_t mode)
{
  int32_t rc = 0;
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  CDBG("%s enter\n", __func__);
  if (ctrl->sfd <= 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  int32_t res = ctrl->sensor.mode_res[mode];
  if (res == MSM_SENSOR_INVALID_RES) {
    CDBG_ERROR("%s invalid resolution, line %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  ctrl->sensor.current_gain = 0;
  ctrl->sensor.current_linecount = 0;

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.mode = mode;
  cfg.rs = res;

  if (ctrl->sensor.cur_res != res) {
    struct msm_camera_i2c_reg_array *regs = ctrl->driver_params->mode_settings[res].reg_setting;
    int index;
    ctrl->sensor.cur_frame_length_lines =
      ctrl->driver_params->output_info[res].frame_length_lines;
    ctrl->sensor.cur_line_length_pclk =
      ctrl->driver_params->output_info[res].line_length_pclk;

    /*write single array for that mode*/
    for (index = 0; index <  ctrl->driver_params->mode_settings[res].size; index++) {
      CDBG("%s addr %x data %x\n", __func__,
        regs[index].reg_addr, regs[index].reg_data);
    }

    cfg.cfg.setting = &ctrl->driver_params->mode_settings[res];
    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;

    if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
        CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
        return -EIO;
     }

    struct msm_camera_i2c_reg_array dim_settings[] = {
      {ctrl->driver_params->sensor_output_reg_addr->x_output,
        ctrl->driver_params->output_info[res].x_output},
      {ctrl->driver_params->sensor_output_reg_addr->y_output,
        ctrl->driver_params->output_info[res].y_output},
      {ctrl->driver_params->sensor_output_reg_addr->line_length_pclk,
        ctrl->driver_params->output_info[res].line_length_pclk},
      {ctrl->driver_params->sensor_output_reg_addr->frame_length_lines,
        ctrl->driver_params->output_info[res].frame_length_lines},
    };

    struct msm_camera_i2c_reg_setting out_settings = {
      &dim_settings[0], ARRAY_SIZE(dim_settings), MSM_CAMERA_I2C_WORD_ADDR,
      MSM_CAMERA_I2C_WORD_DATA, 0
    };

    cfg.cfg.setting = &out_settings;
    if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
        CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
        return -EIO;
    }

    /*v4l2_subdev_notify*/
    cfg.cfgtype = CFG_PCLK_CHANGE;
    cfg.cfg.pclk = ctrl->driver_params->output_info[res].op_pixel_clk;
    if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
        CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
        return -EIO;
    }
    CDBG("%s: Done mode change\n", __func__);
    ctrl->sensor.cur_res = res;
  }

  ctrl->sensor.prev_op_mode = ctrl->sensor.op_mode;
  ctrl->sensor.op_mode = mode;
  ctrl->sensor.out_data.camif_setting.format = ctrl->sensor.inputformat[res];

  if (ctrl->sensor.out_data.sensor_output.output_format == SENSOR_YCBCR)
   ctrl->sensor.out_data.camif_setting.width =
     ctrl->driver_params->output_info[res].x_output * 2;
  else
   ctrl->sensor.out_data.camif_setting.width =
     ctrl->driver_params->output_info[res].x_output;

  ctrl->sensor.out_data.camif_setting.height =
	ctrl->driver_params->output_info[res].y_output;

  /* CAMIF window */
  if (ctrl->sensor.out_data.sensor_output.output_format == SENSOR_YCBCR) {
    ctrl->sensor.out_data.camif_setting.first_pixel =
      ctrl->sensor.crop_info[res].left_crop * 2;
    ctrl->sensor.out_data.camif_setting.last_pixel =
      (ctrl->driver_params->output_info[res].x_output) * 2 -
      (ctrl->sensor.crop_info[res].right_crop) * 2 - 1;
  } else {
    ctrl->sensor.out_data.camif_setting.first_pixel =
      ctrl->sensor.crop_info[res].left_crop;
    ctrl->sensor.out_data.camif_setting.last_pixel =
      (ctrl->driver_params->output_info[res].x_output) -
      ctrl->sensor.crop_info[res].right_crop - 1;
  }

  ctrl->sensor.out_data.camif_setting.first_line =
	ctrl->sensor.crop_info[res].top_crop;

  ctrl->sensor.out_data.camif_setting.last_line =
	ctrl->driver_params->output_info[res].y_output -
	ctrl->sensor.crop_info[res].bottom_crop - 1;

  /* Set the current dimensions */
  ctrl->sensor.out_data.camif_setting.cropped_width =
    ctrl->sensor.out_data.camif_setting.last_pixel -
    ctrl->sensor.out_data.camif_setting.first_pixel + 1;

  ctrl->sensor.out_data.camif_setting.cropped_height =
    ctrl->sensor.out_data.camif_setting.last_line -
    ctrl->sensor.out_data.camif_setting.first_line + 1;

  CDBG("Mode: %d\n", mode);
  CDBG("Resolution: %d\n", res);
  CDBG("camif window first pixel = %d\n",
    ctrl->sensor.out_data.camif_setting.first_pixel);
  CDBG("camif window last pixel = %d\n",
    ctrl->sensor.out_data.camif_setting.last_pixel);
  CDBG("camif window first line = %d\n",
    ctrl->sensor.out_data.camif_setting.first_line);
  CDBG("camif window last line = %d\n",
    ctrl->sensor.out_data.camif_setting.last_line);
  CDBG("camif window width = %d\n",
    ctrl->sensor.out_data.camif_setting.width);
  CDBG("camif window height = %d\n",
    ctrl->sensor.out_data.camif_setting.height);
  CDBG("camif data format  = %d\n",
    ctrl->sensor.out_data.camif_setting.format);

  CDBG("%s exit\n", __func__);
	return rc;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_dim_info(void *sctrl, void *dim_info)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;

  sensor_dim_info_t *dim_info_ptr =
	(sensor_dim_info_t *) dim_info;
  CDBG("%s enter\n", __func__);
  if (ctrl->sfd <= 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  int32_t res = ctrl->sensor.mode_res[dim_info_ptr->op_mode];
  if (res != MSM_SENSOR_INVALID_RES) {
    dim_info_ptr->width = ctrl->driver_params->output_info[res].x_output -
      ctrl->sensor.crop_info[res].left_crop -
      ctrl->sensor.crop_info[res].right_crop;

    dim_info_ptr->height = ctrl->driver_params->output_info[res].y_output -
      ctrl->sensor.crop_info[res].top_crop -
      ctrl->sensor.crop_info[res].bottom_crop;

    /* CAMIF window */
    dim_info_ptr->first_pixel = ctrl->sensor.crop_info[res].left_crop;

    dim_info_ptr->last_pixel = ctrl->driver_params->output_info[res].x_output -
      ctrl->sensor.crop_info[res].right_crop - 1;

    dim_info_ptr->first_line = ctrl->sensor.crop_info[res].top_crop;

    dim_info_ptr->last_line = ctrl->driver_params->output_info[res].y_output -
      ctrl->sensor.crop_info[res].bottom_crop - 1;
  } else {
    CDBG_ERROR("%s invalid resolution, line %d\n", __func__, __LINE__);
    dim_info_ptr->width = 0;
    dim_info_ptr->height = 0;
    /* CAMIF window */
    dim_info_ptr->first_pixel = 0;
    dim_info_ptr->last_pixel = 0;
    dim_info_ptr->first_line = 0;
    dim_info_ptr->last_line = 0;
  }

  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_preview_fps_range(void *sctrl, void *range)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_fps_range_t *fps_range = (sensor_fps_range_t *) range;

  int32_t res = ctrl->sensor.mode_res[SENSOR_MODE_PREVIEW];
  uint32_t lines_per_frame =
    ctrl->driver_params->output_info[res].frame_length_lines;
  uint32_t max_line_count = 1;
  if (ctrl->sensor.out_data.chromatix_ptr) {
    max_line_count =
      ctrl->sensor.out_data.chromatix_ptr->
      chromatix_exposure_table.exposure_entries
      [ctrl->sensor.out_data.chromatix_ptr->
      chromatix_exposure_table.valid_entries-1].line_count;
    fps_range->max_fps =
      (sensor_util_bayer_get_max_fps(sctrl, SENSOR_MODE_PREVIEW) / 256.0);
    fps_range->min_fps = fps_range->max_fps * lines_per_frame /
      max_line_count;
   } else {
     fps_range->max_fps = 30.0;
     fps_range->min_fps = 30.0;
   }
  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_mode_aec_info(void *sctrl, void *aec_data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_aec_data_t *aec_data_ptr =
    (sensor_aec_data_t *) aec_data;

  int32_t res = ctrl->sensor.mode_res[aec_data_ptr->op_mode];
  if (res != MSM_SENSOR_INVALID_RES) {
    aec_data_ptr->pixels_per_line =
      ctrl->driver_params->output_info[res].line_length_pclk;
    aec_data_ptr->lines_per_frame =
      ctrl->driver_params->output_info[res].frame_length_lines;
    aec_data_ptr->pclk =
      ctrl->driver_params->output_info[res].vt_pixel_clk;
    aec_data_ptr->max_fps =
      sensor_util_bayer_get_max_fps(sctrl, aec_data_ptr->op_mode);
  } else {
    CDBG_ERROR("%s invalid resolution, line %d\n", __func__, __LINE__);
    aec_data_ptr->pixels_per_line = 0;
    aec_data_ptr->lines_per_frame = 0;
    aec_data_ptr->pclk = 0;
    aec_data_ptr->max_fps = 0;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_cur_fps(void *sctrl, void *fps_data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  uint32_t *fps = (uint32_t *) fps_data;

  int32_t res = ctrl->sensor.mode_res[ctrl->sensor.op_mode];
  uint32_t frame_length_lines =
    ctrl->driver_params->output_info[res].frame_length_lines;
  float max_fps =
    (sensor_util_bayer_get_max_fps(sctrl, ctrl->sensor.op_mode) / 256.0);

  float cur_fps = max_fps;
  if (ctrl->sensor.current_linecount > frame_length_lines) {
    cur_fps = max_fps * frame_length_lines /
      ctrl->sensor.current_linecount;
  }
  *fps = cur_fps * Q8;
  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_get_bayer_lens_info(void *sctrl, void *info)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_lens_info_t *lens_info = (sensor_lens_info_t *) info;

  lens_info->focal_length = ctrl->sensor.out_data.lens_info.focal_length;
  lens_info->pix_size = ctrl->sensor.out_data.lens_info.pix_size;
  lens_info->f_number = ctrl->sensor.out_data.lens_info.f_number;
  lens_info->total_f_dist = ctrl->sensor.out_data.lens_info.total_f_dist;
  lens_info->hor_view_angle= ctrl->sensor.out_data.lens_info.hor_view_angle;
  lens_info->ver_view_angle= ctrl->sensor.out_data.lens_info.ver_view_angle;

  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_set_exposure_gain(void *sctrl, void *aec_config)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_set_aec_data_t *aec_config_ptr = (sensor_set_aec_data_t *) aec_config;
  float real_gain = aec_config_ptr->gain;
  uint16_t reg_gain = ctrl->fn_table->sensor_real_to_register_gain(real_gain);
  float sensor_real_gain =
    ctrl->fn_table->sensor_register_to_real_gain(reg_gain);
  float digital_gain = real_gain / sensor_real_gain;
  uint32_t linecount = aec_config_ptr->linecount;

  if ((ctrl->sensor.op_mode == SENSOR_MODE_SNAPSHOT) ||
      (ctrl->sensor.op_mode == SENSOR_MODE_RAW_SNAPSHOT)) {
    ctrl->sensor.out_data.aec_info.digital_gain = digital_gain;
  } else {
    ctrl->sensor.out_data.aec_info.digital_gain =
      ctrl->sensor.out_data.aec_info.stored_digital_gain;
    ctrl->sensor.out_data.aec_info.stored_digital_gain = digital_gain;
  }
  CDBG("%s enter\n", __func__);
  if ((ctrl->sfd <= 0) || (ctrl->sensor.op_mode == SENSOR_MODE_INVALID)) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  if (ctrl->sensor.current_gain == reg_gain &&
      ctrl->sensor.current_linecount == linecount)
    return 0;

  ctrl->sensor.current_gain = reg_gain;
  ctrl->sensor.current_linecount = linecount;
  cfg.cfg.exp_gain.gain = reg_gain;
  cfg.cfg.exp_gain.line = linecount;

  ctrl->fn_table->sensor_write_exp_gain(sctrl, reg_gain, linecount);

  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int32_t sensor_util_bayer_write_exp_gain(void *sctrl,
  uint16_t gain, uint32_t line)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  uint32_t fl_lines;
  uint8_t offset;
  struct sensor_cfg_data cfg;

  fl_lines = ctrl->sensor.cur_frame_length_lines;
  fl_lines = (fl_lines * ctrl->sensor.current_fps_div) / Q10;

  offset = ctrl->driver_params->sensor_exp_gain_info->vert_offset;
  if (line > (fl_lines - offset))
    fl_lines = line + offset;
  /*group hold on*/
  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = ctrl->driver_params->groupon_settings;
  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
      CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
      return -EIO;
  }
  /*Write exp gain settings*/
  struct msm_camera_i2c_reg_array exp_gain_settings[] = {
    {ctrl->driver_params->sensor_output_reg_addr->frame_length_lines,fl_lines},
    {ctrl->driver_params->sensor_exp_gain_info->coarse_int_time_addr,line},
    {ctrl->driver_params->sensor_exp_gain_info->global_gain_addr,gain},
  };

  struct msm_camera_i2c_reg_setting exp_gain={
    &exp_gain_settings[0],
    ARRAY_SIZE(exp_gain_settings),
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_WORD_DATA,
    0
  };

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = &exp_gain;
  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
      CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
      return -EIO;
  }
  /*group hold off */
  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = ctrl->driver_params->groupoff_settings;
  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
      CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
      return -EIO;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_set_snapshot_exposure_gain(void *sctrl, void *aec_config)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_set_aec_data_t *aec_config_ptr =
    (sensor_set_aec_data_t *) aec_config;

  CDBG("%s enter\n", __func__);
  if (ctrl->sfd <= 0) {
    CDBG_ERROR("%s failed %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  uint32_t linecount;
  int32_t res = ctrl->sensor.mode_res[ctrl->sensor.op_mode];
  int32_t prev_res = ctrl->sensor.mode_res[ctrl->sensor.prev_op_mode];
  int32_t prev_bin = ctrl->driver_params->output_info[prev_res].binning_factor;
  int32_t snap_bin = ctrl->driver_params->output_info[res].binning_factor;

  if (prev_res == res)
    return 0;

  if (prev_bin != 0 && snap_bin != 0)
    aec_config_ptr->linecount *= prev_bin / snap_bin;
  else
    CDBG_ERROR("Binning factor not set in kernel\n");

  linecount = aec_config_ptr->linecount;
  ctrl->fn_table->sensor_set_exposure_gain(sctrl, aec_config);

  double frame_time;
  if (ctrl->driver_params->output_info[res].frame_length_lines < linecount) {
    frame_time = Q8 /
      ((float)sensor_util_bayer_get_max_fps(sctrl, ctrl->sensor.op_mode) *
      ctrl->driver_params->output_info[res].frame_length_lines / linecount);
  } else {
    frame_time =
      Q8 / (float)sensor_util_bayer_get_max_fps(sctrl, ctrl->sensor.op_mode);
  }
  usleep(ctrl->sensor.snapshot_exp_wait_frames * frame_time * 1000 * 1000);

  CDBG("FPS: %d\n", ctrl->sensor.current_fps);
  CDBG("%s exit\n", __func__);
  return 0;
}

/*===========================================================================
 * FUNCTION    - x -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_max_supported_hfr_mode(void *sctrl, void *hfr_mode)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  camera_hfr_mode_t *max_hfr_mode = (camera_hfr_mode_t *)hfr_mode;
  sensor_mode_t sensor_mode = SENSOR_MODE_HFR_60FPS;

  CDBG("%s enter\n", __func__);

  *max_hfr_mode = CAMERA_HFR_MODE_OFF;

  while ((sensor_mode <= SENSOR_MODE_HFR_150FPS) &&
       (ctrl->sensor.mode_res[sensor_mode] != MSM_SENSOR_INVALID_RES)) {
    (*max_hfr_mode)++;
    sensor_mode++;
  }

  CDBG("%s max_hfr_mode = %d\n", __func__, *max_hfr_mode);

  return 0;
}

/*===========================================================================
 * FUNCTION    - sensor_util_set_start_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_set_start_stream(void *sctrl)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  CDBG("%s enter\n", __func__);

  if (ctrl->sfd <= 0)
    return FALSE;

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = ctrl->driver_params->start_settings;
  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
    CDBG("%s failed %d\n", __func__, __LINE__);
    return 0;
  }
  CDBG("%s exit\n", __func__);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_util_set_stop_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_set_stop_stream(void *sctrl)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  CDBG("%s enter\n", __func__);

  if (ctrl->sfd <= 0)
    return FALSE;

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = ctrl->driver_params->stop_settings;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_IO_CFG, &cfg) < 0) {
    CDBG("%s failed %d\n", __func__, __LINE__);
    return 0;
  }
  CDBG("%s exit\n", __func__);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_util_bayer_get_csi_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_csi_params(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_csi_params_t **csi_lane_params = (sensor_csi_params_t **)data;
  if (!ctrl || !csi_lane_params) {
    CDBG_ERROR("%s invalid params %p or %p\n", __func__,
      ctrl, csi_lane_params);
    return FALSE;
  }
  *csi_lane_params = &ctrl->sensor.sensor_csi_params;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_util_bayer_config_setting -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_config_setting(void *sctrl, void *data)
{
  struct sensor_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  //CDBG_ERROR("%s:%d called type %d\n", __func__, __LINE__, type);
  if (ctrl->sfd <= 0)
    return FALSE;
  struct sensor_oem_setting *oem_setting = (struct sensor_oem_setting *)data;

  switch(oem_setting->type)
  {
  case I2C_READ:
	cfg.cfgtype = CFG_READ_I2C_ARRAY;
    break;
  case I2C_WRITE:
    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
    break;
  case GPIO_OP:
      cfg.cfgtype = CFG_GPIO_OP;
    break;
  case EEPROM_READ:
      cfg.cfgtype = CFG_GET_EEPROM_DATA;
    break;
  case VREG_SET:
      cfg.cfgtype = CFG_CONFIG_VREG_ARRAY;
    break;
  case CLK_SET:
	  cfg.cfgtype = CFG_CONFIG_CLK_ARRAY;
    break;
  default:
    break;
  };
  cfg.cfg.setting = oem_setting->data;
  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_OEM, &cfg) < 0) {
    CDBG("%s failed %d\n", __func__, __LINE__);
    return 0;
  }
  CDBG("%s exit\n", __func__);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_util_bayer_get_camif_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_camif_cfg(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_camif_setting_t *camif_setting = (sensor_camif_setting_t *)data;
  if (!ctrl || !camif_setting) {
    CDBG_ERROR("%s Invalid params %p or %p\n", __func__, ctrl, camif_setting);
    return FALSE;
  }
  *camif_setting = ctrl->sensor.out_data.camif_setting;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_util_bayer_get_output_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_output_cfg(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_data_t *sensor_data = (sensor_data_t *)data;
  if (!ctrl || !sensor_data) {
    CDBG_ERROR("%s Invalid params %p or %p\n", __func__, ctrl, sensor_data);
    return FALSE;
  }
  sensor_data->sensor_output = ctrl->sensor.out_data.sensor_output;
  sensor_data->pxlcode = ctrl->sensor.out_data.pxlcode;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_util_bayer_get_digital_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_digital_gain(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  float *digital_gain= (float *)data;
  if (!ctrl || !digital_gain) {
    CDBG_ERROR("%s Invalid params %p or %p\n", __func__, ctrl, digital_gain);
    return FALSE;
  }
  *digital_gain = ctrl->sensor.out_data.aec_info.digital_gain;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_util_bayer_get_cur_res -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t sensor_util_bayer_get_cur_res(void *sctrl, uint16_t op_mode, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  enum msm_sensor_resolution_t *cur_res = (enum msm_sensor_resolution_t *)data;
  if (!ctrl || !cur_res) {
    CDBG_ERROR("%s Invalid params %p or %p\n", __func__, ctrl, cur_res);
    return FALSE;
  }
  *cur_res = ctrl->sensor.mode_res[op_mode];
  return TRUE;
}
