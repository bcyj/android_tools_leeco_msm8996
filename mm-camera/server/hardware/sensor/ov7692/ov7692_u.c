/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "camera_dbg.h"
#include "sensor_util.h"
#include "sensor_util_bayer.h"
#include "ov7692_u.h"

/*============================================================================
                        EXTERNAL VARIABLES DECLARATIONS
============================================================================*/
/* r/w copy of chromatix data. Must be initialized with
 * active sensor chromatix data everytime camera is started.
 */
/* Sensor model number used in camear for this sensor */
#define SENSOR_MODEL_NO_OV7692 "ov7692"
#define OV7692_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV7692"_"#n".so"

/* Types of shared object available for this sensor to load dynamically */
char *ov7692_load_chromatix[SENSOR_LOAD_CHROMATIX_MAX] = {
//  OV7692_LOAD_CHROMATIX(preview), /* camera / camcorder preview */
  //OV7692_LOAD_CHROMATIX(default_video), /* Non HD Video recording */
  NULL,
  NULL,
  NULL, /* HD video recording */
  NULL, /* HFR video recording */
  NULL,/* AR */
};
/*****************************************************************************
 *                          RUN TIME VARIABLES
 ****************************************************************************/

static sensor_camif_inputformat_t ov7692_inputformat[] = {
    CAMIF_YCBCR_Y_CB_Y_CR,/*RES0*/
    CAMIF_YCBCR_Y_CB_Y_CR,/*RES1*/
};

static sensor_crop_parms_t ov7692_cropinfo[] = {
/*top, down, left, right*/
#if 0
  {24, 24, 56, 56},/*FULL*/
  {12, 12, 28, 28},/*QTR*/
#endif
  {0, 0, 0, 0},/*FULL*/
  {0, 0, 0, 0},/*QTR*/

};

static uint32_t ov7692_mode_res[SENSOR_MODE_INVALID] = {
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_RAW_SNAPSHOT*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_PREVIEW*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_VIDEO*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_60FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_90FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_120FPS*/
  MSM_SENSOR_INVALID_RES,/*SENSOR_MODE_HFR_150FPS*/
  MSM_SENSOR_RES_FULL,/*SENSOR_MODE_VIDEO*/
};

static struct msm_camera_csi_params ov7692_csi_params = {
  .data_format = CSI_8BIT,
  .lane_cnt    = 1,
  .lane_assign = 0xe4,
  .dpcm_scheme = 0,
  .settle_cnt  = 0x14,
};

static struct msm_camera_csi_params *ov7692_csi_params_array[] = {
  &ov7692_csi_params,/*FULL*/
  &ov7692_csi_params,/*QTR*/
};

/*===========================================================================
 * FUNCTION    - ov7692_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov7692_real_to_register_gain(float gain)
{
    return 0;
}/* ov7692_real_to_register_gain */

/*===========================================================================
 * FUNCTION    - ov7692_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
float ov7692_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;

  real_gain = (float)((((float)(reg_gain >> 4) + 1.0) * \
  (16.0 + (float)(reg_gain % 16))) / 16.0);
  return real_gain;
}/* ov7692_register_to_real_gain */

static uint8_t ov7692_set_saturation(void *sctrl, int32_t saturation)
{
  struct v4l2_control v4l2_ctrl;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  if (ctrl->sfd <= 0)
    return FALSE;

  v4l2_ctrl.id = V4L2_CID_SATURATION;
  v4l2_ctrl.value = saturation;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_V4l2_S_CTRL, &v4l2_ctrl) < 0) {
    CDBG_ERROR("ov7692 failed %d\n", __LINE__);
    return FALSE;
  }
  return TRUE;
}

static uint8_t ov7692_set_contrast(void *sctrl, int32_t contrast)
{
  struct v4l2_control v4l2_ctrl;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;

  if (ctrl->sfd <= 0)
    return FALSE;

  v4l2_ctrl.id = V4L2_CID_CONTRAST;
  v4l2_ctrl.value = contrast;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_V4l2_S_CTRL,
   &v4l2_ctrl) < 0) {
    CDBG_ERROR("ov7692 failed %d\n", __LINE__);
    return FALSE;
  }
  return TRUE;
}

static uint8_t ov7692_set_sharpness(void *sctrl, int32_t sharpness)
{
  struct v4l2_control v4l2_ctrl;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  if (ctrl->sfd <= 0)
    return FALSE;

  v4l2_ctrl.id = V4L2_CID_SHARPNESS;
  v4l2_ctrl.value = sharpness/5;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_V4l2_S_CTRL,
   &v4l2_ctrl) < 0) {
    CDBG_ERROR("ov7692 failed %d\n", __LINE__);
    return FALSE;
  }
  return TRUE;
}

static uint8_t ov7692_set_exposure_compensation(void *sctrl, int32_t exposure)
{
  short x;
  struct v4l2_control v4l2_ctrl;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  if (ctrl->sfd <= 0)
    return FALSE;

  v4l2_ctrl.id = V4L2_CID_EXPOSURE;
  x = ((exposure>>16)/6) + 2 ;
  v4l2_ctrl.value = x;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_V4l2_S_CTRL,
   &v4l2_ctrl) < 0) {
    CDBG_ERROR("ov7692 failed %d\n", __LINE__);
    return FALSE;
  }
  return TRUE;
}

static uint8_t ov7692_set_iso(void *sctrl, int32_t iso)
{
  struct v4l2_control v4l2_ctrl;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  if (ctrl->sfd <= 0)
    return FALSE;

  v4l2_ctrl.id = MSM_V4L2_PID_ISO;
  v4l2_ctrl.value = iso;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_V4l2_S_CTRL,
   &v4l2_ctrl) < 0) {
    CDBG_ERROR("ov7692 failed %d\n", __LINE__);
    return FALSE;
  }
  return TRUE;
}

static uint8_t ov7692_set_special_effect(void *sctrl, int32_t effect)
{
  struct v4l2_control v4l2_ctrl;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  if (ctrl->sfd <= 0)
    return FALSE;

  v4l2_ctrl.id = V4L2_CID_SPECIAL_EFFECT;
  v4l2_ctrl.value = effect;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_V4l2_S_CTRL,
   &v4l2_ctrl) < 0) {
    CDBG_ERROR("ov7692 failed %d\n", __LINE__);
    return FALSE;
  }
  return TRUE;
}

static uint8_t ov7692_set_antibanding(void *sctrl, int32_t antibanding)
{
  struct v4l2_control v4l2_ctrl;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  if (ctrl->sfd <= 0)
    return FALSE;
  v4l2_ctrl.id = V4L2_CID_POWER_LINE_FREQUENCY;
  v4l2_ctrl.value = antibanding;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_V4l2_S_CTRL,
   &v4l2_ctrl) < 0) {
    CDBG_ERROR("ov7692 failed %d\n", __LINE__);
    return FALSE;
  }
  return TRUE;
}

static uint8_t ov7692_set_wb_oem(void *sctrl, int32_t wb_oem)
{
  struct v4l2_control v4l2_ctrl;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *) sctrl;
  if (ctrl->sfd <= 0)
    return FALSE;

  v4l2_ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;
  v4l2_ctrl.value = wb_oem;

  if (ioctl(ctrl->sfd, MSM_CAM_IOCTL_SENSOR_V4l2_S_CTRL,
    &v4l2_ctrl) < 0) {
    CDBG_ERROR("ov7692 failed %d\n", __LINE__);
    return FALSE;
  }
  return TRUE;
}

static sensor_function_table_t ov7692_func_tbl = {
  .sensor_set_op_mode = sensor_util_set_op_mode,
  .sensor_get_mode_aec_info = sensor_util_get_mode_aec_info,
  .sensor_get_dim_info = sensor_util_get_dim_info,

  .sensor_set_frame_rate = sensor_util_set_frame_rate,
  .sensor_get_snapshot_fps = sensor_util_get_snapshot_fps,

  .sensor_get_preview_fps_range = sensor_util_get_preview_fps_range,
  .sensor_set_exposure_gain = sensor_util_set_exposure_gain,
  .sensor_set_snapshot_exposure_gain = sensor_util_set_snapshot_exposure_gain,
  .sensor_register_to_real_gain = ov7692_register_to_real_gain,
  .sensor_real_to_register_gain = ov7692_real_to_register_gain,
  .sensor_get_lens_info = sensor_get_lens_info,
  .sensor_set_saturation = ov7692_set_saturation,
  .sensor_set_contrast   = ov7692_set_contrast,
  .sensor_set_sharpness = ov7692_set_sharpness,
  .sensor_set_exposure_compensation  = ov7692_set_exposure_compensation,
  .sensor_set_iso = ov7692_set_iso,
  .sensor_set_special_effect = ov7692_set_special_effect,
  .sensor_set_antibanding = ov7692_set_antibanding,
  .sensor_set_wb_oem = ov7692_set_wb_oem,
  .sensor_get_max_supported_hfr_mode = sensor_util_get_max_supported_hfr_mode,
  .sensor_get_csi_params = sensor_util_get_csi_params,
  .sensor_set_start_stream = sensor_util_set_start_stream,
  .sensor_set_stop_stream = sensor_util_set_stop_stream,
  .sensor_get_camif_cfg = sensor_util_bayer_get_camif_cfg,
  .sensor_get_output_cfg = sensor_util_bayer_get_output_cfg,
  .sensor_get_digital_gain = sensor_util_bayer_get_digital_gain,
  .sensor_get_cur_res = sensor_util_bayer_get_cur_res,
};

int8_t ov7692_process_start(void *ctrl)
{
  sensor_ctrl_t *sctrl = (sensor_ctrl_t *) ctrl;
  sctrl->fn_table = &ov7692_func_tbl;
  sctrl->sensor.inputformat = ov7692_inputformat;
  sctrl->sensor.crop_info = ov7692_cropinfo;
  sctrl->sensor.mode_res = ov7692_mode_res;
  sctrl->sensor.sensor_csi_params.csic_params = &ov7692_csi_params_array[0];

  sensor_util_get_output_info(sctrl);

  sctrl->sensor.op_mode = SENSOR_MODE_VIDEO;

  sctrl->sensor.out_data.sensor_output.connection_mode = SENSOR_MIPI_CSI_1;
  sctrl->sensor.out_data.sensor_output.output_format = SENSOR_YCBCR;
  sctrl->sensor.out_data.sensor_output.raw_output = SENSOR_8_BIT_DIRECT;

  sctrl->sensor.out_data.aec_info.max_gain = 16.0;
  sctrl->sensor.out_data.aec_info.max_linecount =
  sctrl->sensor.output_info[sctrl->sensor.
    mode_res[SENSOR_MODE_PREVIEW]].frame_length_lines * 24;
  sctrl->sensor.snapshot_exp_wait_frames = 1;

  sctrl->sensor.out_data.lens_info.focal_length = 4.6;
  sctrl->sensor.out_data.lens_info.pix_size = 1.4;
  sctrl->sensor.out_data.lens_info.f_number = 2.65;
  sctrl->sensor.out_data.lens_info.total_f_dist = 1.97;
  sctrl->sensor.out_data.lens_info.hor_view_angle = 54.8;
  sctrl->sensor.out_data.lens_info.ver_view_angle = 42.5;

  sensor_util_config(sctrl);
  return TRUE;
}
