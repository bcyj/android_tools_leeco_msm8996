/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/
#include <sys/time.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>
#include <string.h>
#include <unistd.h>
#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include <inttypes.h>
#include <media/msm_isp.h>
#include "mctl_divert.h"
#include "mctl.h"
#include "mctl_af.h"
#include "config_proc.h"
#include "camera_dbg.h"
#include "mm_camera_interface.h"
#include <dlfcn.h>

#if 0
#undef CDBG
#define CDBG LOGE
#endif

#define SET_PARM_BIT(parm, parm_arr) \
  (parm_arr |= (1<<(parm)))
#define SET_PARM_BIT32(parm, parm_arr) \
  (parm_arr[parm/32] |= (1<<(parm%32)))
#ifndef VFE_2X
#define DEFAULT_PREVIEW_WIDTH 640
#define DEFAULT_PREVIEW_HEIGHT 480
static struct camera_size_type jpeg_thumbnail_sizes[]  = {
    { 512, 288 },
    { 480, 288 },
    { 432, 288 },
    { 512, 384 },
    { 352, 288 },
    { 320, 240 },
    { 176, 144 },
    {0,0}
};
static struct camera_size_type default_preview_size[] = {
    { 1920, 1080}, //1080p
    { 1280, 720}, // 720P, reserved
    { 800, 480}, // WVGA
    { 768, 432},
    { 720, 480},
    { 640, 480}, // VGA
    { 576, 432},
    { 480, 320}, // HVGA
    { 384, 288},
    { 352, 288}, // CIF
    { 320, 240}, // QVGA
    { 240, 160}, // SQVGA
    { 176, 144}, // QCIF
};
static struct camera_size_type supported_video_sizes[] = {
    { 1920, 1080},// 1080p
    { 1280, 720}, // 720p
    { 800, 480},  // WVGA
    { 720, 480},  // 480p
    { 640, 480},  // VGA
    { 480, 320},  // HVGA
    { 352, 288},  // CIF
    { 320, 240},  // QVGA
    { 176, 144},  // QCIF
};
static struct camera_size_type hfr_sizes[] = {
    { 800, 480}, // WVGA
    { 640, 480} // VGA
};
#else
#define DEFAULT_PREVIEW_WIDTH 1280
#define DEFAULT_PREVIEW_HEIGHT 720
static struct camera_size_type jpeg_thumbnail_sizes[]  = {
    { 864, 480}, //FWVGA
    { 800, 480},
    { 768, 432},
    { 720, 480},
    { 640, 480}, // VGA
    { 576, 432},
    { 512, 384},
    { 480, 320}, // HVGA
    { 432, 240}, //WQVGA
    { 384, 288},
    { 352, 288}, // CIF
    { 320, 240}, // QVGA
    { 240, 160}, // SQVGA
    { 176, 144}, // QCIF
    {0,0}
};
static struct camera_size_type default_preview_size[] = {
    { 1280, 720}, // 720p
    { 864, 480}, // FWVGA
    { 800, 480},
    { 768, 432},
    { 720, 480},
    { 640, 480}, // VGA
    { 576, 432},
    { 480, 320}, // HVGA
    { 432, 240}, //WQVGA
    { 384, 288},
    { 352, 288}, // CIF
    { 320, 240}, // QVGA
    { 240, 160}, // SQVGA
    { 176, 144}, // QCIF
};
static struct camera_size_type supported_video_sizes[] = {
    { 1280, 720}, // 720p
    { 864, 480},  // FWVGA
    { 800, 480},
    { 768, 432},
    { 720, 480},  // 480p
    { 640, 480},  // VGA
    { 576, 432},
    { 480, 320},  // HVGA
    { 432, 240}, //WQVGA
    { 384, 288},
    { 352, 288},  // CIF
    { 320, 240},  // QVGA
    { 240, 160}, // SQVGA
    { 176, 144},  // QCIF
};
static struct camera_size_type hfr_sizes[] = {
    { 432, 240}, //WQVGA
    { 320, 240}  //QVGA
};
#endif

extern int parse_mobicat_info(QCameraInfo_t *p_mobicat_info,
  cam_exif_tags_t *p_metadata);

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PREVIEW_HFR -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PREVIEW_HFR(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  uint32_t value = *(uint32_t *)ctrlCmd->value;
  v4l2_video_ctrl *pvideo_ctrl = &ctrl->video_ctrl;
  vfe_hfr_info_t  hfr_info;
  hfr_info.preview_hfr = TRUE;
  hfr_info.hfr_mode = value;

  CDBG("%s: HFR %d", __func__, value);
  pvideo_ctrl->op_mode = MSM_V4L2_CAM_OP_VIDEO;
  ctrl->hfr_mode = value;
  ctrl->preview_hfr = TRUE;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_HFR_MODE, &hfr_info, NULL);
  ctrlCmd->status = (rc == VFE_SUCCESS) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}/*config_proc_CAMERA_SET_PREVIEW_HFR*/

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_MAX_DIMENSION -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_MAX_DIMENSION(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  cam_sensor_dim_t *p_dim = (cam_sensor_dim_t *)ctrlCmd->value;
  camera_hfr_mode_t max_hfr_supported;

  sensor_get_t sensor_get;

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
    SENSOR_GET_MAX_SUPPORTED_HFR_MODE, &sensor_get,
    sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }

  max_hfr_supported = sensor_get.data.max_supported_hfr_mode;

  switch (p_dim->type) {
      break;
    case CAM_SENSOR_MODE_SNAPSHOT:
      sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_SNAPSHOT;
      break;
    default:
    case CAM_SENSOR_MODE_VIDEO:
    case CAM_SENSOR_MODE_PREVIEW: {
      CDBG("%s:%d] req %d max %d\n", __func__, __LINE__,
        p_dim->hfr_mode, max_hfr_supported);
      if (max_hfr_supported < p_dim->hfr_mode) {
        p_dim->hfr_mode = max_hfr_supported;
      }
      switch (p_dim->hfr_mode) {
        case CAMERA_HFR_MODE_60FPS:
          sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_HFR_60FPS;
          break;
        case CAMERA_HFR_MODE_90FPS:
          sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_HFR_90FPS;
          break;
        case CAMERA_HFR_MODE_120FPS:
          sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_HFR_120FPS;
          break;
        default:
        case CAMERA_HFR_MODE_OFF:
          sensor_get.data.sensor_dim.op_mode =
            (p_dim->type == CAM_SENSOR_MODE_PREVIEW) ?
            SENSOR_MODE_PREVIEW : SENSOR_MODE_VIDEO;
          break;
      }
    }
  }
  CDBG("%s:%d] sensor mode %d\n", __func__, __LINE__,
    sensor_get.data.sensor_dim.op_mode);
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
    SENSOR_GET_DIM_INFO, &sensor_get,
    sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
  }
  CDBG("%s:%d] sensor dim %dx%d\n", __func__, __LINE__,
    sensor_get.data.sensor_dim.width,
    sensor_get.data.sensor_dim.height);
  p_dim->width = sensor_get.data.sensor_dim.width;
  p_dim->height = sensor_get.data.sensor_dim.height;

  ctrlCmd->status = (rc == 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}/*config_proc_CAMERA_GET_MAX_DIMENSION*/

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_MAX_DIMENSION -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_MAX_NUM_FACES_DECT(void *parm1, void *parm2)
{

  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  int *num_fd = (int *)(cmd->value);
  void *faceProc_ptr = NULL;
  *num_fd = 0;
#ifdef MM_CAMERA_FD
  *num_fd = MAX_ROI;
#endif
  cmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
} /*config_proc_CAMERA_GET_MAX_NUM_FACES_DECT*/

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_HDR -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_HDR(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  exp_bracketing_t *temp = (exp_bracketing_t *)(cmd->value);

  temp->total_frames = 3;
  char str[32] = "0,-6,6"; /* TODO: Get from frameproc. */
  strlcpy(temp->values, str, sizeof(str));
  cmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_SET_3A_CONVERGENCE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_CAMERA_SET_3A_CONVERGENCE(void *parm1, void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  cam_3a_conv_info_t *p_conv_info = (cam_3a_conv_info_t *)(cmd->value);

  cmd->status = CAM_CTRL_SUCCESS;

  CDBG("%s:  p_conv_info = %p\n", __func__, p_conv_info);
  if (NULL != p_conv_info) {
    ctrl->conv_3a_info_set = TRUE;
    ctrl->conv_3a_info = *p_conv_info;
  } else {
    ctrl->conv_3a_info_set = FALSE;
  }

  return TRUE;
}/*config_CAMERA_SET_3A_CONVERGENCE*/

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_LED_MODE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_LED_MODE(void *parm1,
  void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  led_mode_t led_mode = *(led_mode_t *)(cmd->value);
  int32_t rc = TRUE;
  if (!ctrl->cam_sensor_info.flash_enabled) {
    CDBG("LED flash isn't supported for this sensor!!!\n");
  } else if (led_mode < LED_MODE_MAX) {
    flash_led_set_t led_set_parm;
    led_set_parm.data.led_mode = led_mode;
	ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
	 ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
	 FLASH_SET_MODE, &led_set_parm, NULL);

    stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
    sp_ctrl->intf.input.flash_info.led_mode = led_mode;
    stats_proc_set_t sp_set_param;
    sp_set_param.type = STATS_PROC_AEC_TYPE;
    sp_set_param.d.set_aec.type = AEC_STROBE_MODE;

    flash_strobe_set_t strobe_set_parm;

    if (led_mode == LED_MODE_OFF) {
      sp_set_param.d.set_aec.d.aec_strobe_mode = STROBE_FLASH_MODE_OFF;
      strobe_set_parm.charge_enable = STROBE_FLASH_CHARGE_DISABLE;
    } else if (led_mode == LED_MODE_AUTO) {
      sp_set_param.d.set_aec.d.aec_strobe_mode = STROBE_FLASH_MODE_AUTO;
      strobe_set_parm.charge_enable = STROBE_FLASH_CHARGE_ENABLE;
    } else {
      sp_set_param.d.set_aec.d.aec_strobe_mode = STROBE_FLASH_MODE_ON;
      strobe_set_parm.charge_enable = STROBE_FLASH_CHARGE_ENABLE;
    }

    ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].set_params(
      ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].handle,
      FLASH_SET_STATE, &strobe_set_parm, NULL);

    if (ctrl->comp_mask & (1 << MCTL_COMPID_STATSPROC)) {
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
             ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
             sp_set_param.type, &sp_set_param, &(sp_ctrl->intf));
    }
  }
  cmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_FD_FEATURE_INFO -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_FD_FEATURE_INFO(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;

  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  int32_t *value  = (int32_t *)ctrlCmd->value;
  int val = 0;
#ifdef MM_CAMERA_FD
 SET_PARM_BIT(MM_CAMERA_FACIAL_FEATURE_FD, val);
#endif
 *value = val;
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - camconfig_proc_CAMERA_GET_PARM_MAXZOOM -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_MAXZOOM(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;

  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  int32_t *value  = (int32_t *)ctrlCmd->value;

  *value = ctrl->zoomCtrl.cam_parm_zoom.maximum_value;
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - camconfig_proc_CAMERA_GET_PARM_ZOOMRATIOS -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_ZOOMRATIOS(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("config_proc_CAMERA_GET_PARM_ZOOMRATIOS: E\n");

  rc = zoom_process(&ctrl->zoomCtrl, ZOOM_PROC_CMD_ZOOM_RATIOS, parm2);
  ctrlCmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;

  return TRUE;
}
/*===========================================================================
 * FUNCTION    - camconfig_proc_CAMERA_GET_PARM_DEF_PREVIEW_SIZES -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_DEF_PREVIEW_SIZES(void *parm1,void *parm2)
{
  int8_t rc = FALSE;
  int cnt=0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("config_proc_CAMERA_GET_PARM_DEF_PREVIEW_SIZES: E\n");
  if(ctrlCmd->value)
  {
      memcpy(ctrlCmd->value,&default_preview_size[0],ctrlCmd->length);
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      return TRUE;
  }
  else
      return false;

}
/*===========================================================================
 * FUNCTION    - camconfig_proc_CAMERA_GET_PARM_DEF_VIDEO_SIZES -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_DEF_VIDEO_SIZES(void *parm1,void *parm2)
{
  int8_t rc = FALSE;
  int cnt=0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("config_proc_CAMERA_GET_PARM_DEF_VIDEO_SIZES: E\n");
  if(ctrlCmd->value)
  {
      CDBG("config_proc_CAMERA_GET_PARM_DEF_VIDEO_SIZES: E\n");
      memcpy(ctrlCmd->value,&supported_video_sizes[0],ctrlCmd->length);
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      return TRUE;
  }
  else
      return false;
}
/*===========================================================================
 * FUNCTION    - camconfig_proc_CAMERA_GET_PARM_DEF_THUMB_SIZES -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_DEF_THUMB_SIZES(void *parm1,void *parm2)
{
  int8_t rc = FALSE;
  int cnt=0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("config_proc_CAMERA_GET_PARM_DEF_THUMB_SIZES: E\n");
  if(ctrlCmd->value)
  {
      memcpy(ctrlCmd->value,&jpeg_thumbnail_sizes[0],ctrlCmd->length);
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      return TRUE;
  }
  else
  {
      return false;
  }
}
/*===========================================================================
 * FUNCTION    - camconfig_proc_CAMERA_GET_PARM_DEF_HFR_SIZES -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_DEF_HFR_SIZES(void *parm1,void *parm2)
{
  int8_t rc = FALSE;
  int cnt=0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("config_proc_CAMERA_GET_PARM_DEF_HFR_SIZES: E\n");
  if(ctrlCmd->value)
  {
      memcpy(ctrlCmd->value,&hfr_sizes[0],ctrlCmd->length);
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      return TRUE;
  }
  else
      return false;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_FOCUS_RECT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_FOCUS_RECT(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd =
    (struct msm_ctrl_cmd *)parm2;

  ctrl->afCtrl.parm_focusrect.current_value =
    *(int16_t *)ctrlCmd->value;

  CDBG("%s:parm_focusrect.current_value=%d\n",
    __func__,
    ctrl->afCtrl.parm_focusrect.current_value);
  ctrlCmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_CONTRAST -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_CONTRAST(void *parm1, void *parm2)
{
  int8_t rc = TRUE;

  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  uint8_t bestshotMode = ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

  int8_t current_spl_effect =
    ctrl->effectCtrl.specialEffectsInfo.parm.current_value;

  struct msm_ctrl_cmd *ctrlCmd =
    (struct msm_ctrl_cmd *)parm2;

  uint32_t value =
    *(uint32_t *)ctrlCmd->value;
  if (bestshotMode == CAMERA_BESTSHOT_OFF
     && current_spl_effect != CAMERA_EFFECT_POSTERIZE
     && current_spl_effect != CAMERA_EFFECT_SOLARIZE) {
    rc = effects_set_contrast(ctrl, value);
    CDBG("%s: Setting Contrast!", __func__);
    ctrlCmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  } else {
    CDBG("%s: bestmode or color effect is not off, contrast is not supported", __func__);
    rc = TRUE ;
    ctrlCmd->status = CAM_CTRL_SUCCESS ;
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_BRIGHTNESS -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_BRIGHTNESS(void *parm1, void *parm2)
{
  int8_t rc = TRUE;

  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  uint8_t bestshotMode = ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

  int32_t value = *(int32_t *)cmd->value;

  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR) {
    //call sensor set brightness function
    cmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  } else {
    stats_proc_set_t set_param;
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_BRIGHTNESS_LVL;
    set_param.d.set_aec.d.aec_brightness = value;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf));
    if (rc)
      CDBG_ERROR("FAILED to set AEC_BRIGHTNESS_LVL\n");
    cmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  }

  return TRUE;
}
/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_EXPOSURE_COMPENSATION -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_EXPOSURE_COMPENSATION(void *parm1,
  void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  uint8_t bestshotMode = ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  uint32_t value =  *(uint32_t *)cmd->value;

  CDBG("In config_proc_CAMERA_SET_PARM_EXPOSURE_COMPENSATION\n");

  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR) {
    sensor_get_t sensor_get;
    sensor_set_t sensor_set;
    sensor_set.data.exposure = value;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_SET_EXPOSURE_COMPENSATION, &sensor_set, NULL);
  } else {
    stats_proc_set_t set_param;
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_EXP_COMPENSATION;
    set_param.d.set_aec.d.aec_exp_comp = *(int32_t *)cmd->value;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type,
		&set_param, &(sp_ctrl->intf));
  }
  if (rc < 0)
    CDBG_ERROR("%s FAILED to set AEC_EXP_COMPENSATION\n", __func__);
  cmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;

  CDBG("%s: %p, %d\n", __func__,
  cmd->value, *(int32_t *)cmd->value);

  return TRUE;
}
/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_SHARPNESS -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_SHARPNESS(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  int sharpness = *(int *)ctrlCmd->value;

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    ctrl->ui_sharp_ctrl_factor =
      (float)(sharpness)/(CAMERA_MAX_SHARPNESS - CAMERA_MIN_SHARPNESS);
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle,
               VFE_SET_SHARPNESS_PARMS,
               (void *)&ctrl->ui_sharp_ctrl_factor, NULL);
  } else {
    sensor_get_t sensor_get;
    sensor_set_t sensor_set;
    sensor_set.data.sharpness = sharpness;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_SET_SHARPNESS, &sensor_set, NULL);
  }
  ctrlCmd->status = (rc == VFE_SUCCESS) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_HUE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_HUE(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;

  struct msm_ctrl_cmd *ctrlCmd =
    (struct msm_ctrl_cmd *)parm2;

  uint8_t bestshotMode =
    ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

  int32_t hue =
    (int32_t)*(int32_t *)ctrlCmd->value;

  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_BAYER) {
    if (bestshotMode == CAMERA_BESTSHOT_OFF) {
      rc = effects_set_hue(ctrl, hue);
    } else {
      CDBG("%s: Besthsot mode is set\n", __func__);
      rc = TRUE;
    }
  }

  ctrlCmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_SATURATION -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_SATURATION(void *parm1, void *parm2)
{
  int8_t rc = FALSE;

  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;

  struct msm_ctrl_cmd *ctrlCmd =
    (struct msm_ctrl_cmd *)parm2;

  int8_t current_spl_effect =
    ctrl->effectCtrl.specialEffectsInfo.parm.current_value;
  uint8_t bestshotMode =
    ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

  uint32_t value =
    *(uint32_t *)ctrlCmd->value;

  if (bestshotMode == CAMERA_BESTSHOT_OFF &&
     current_spl_effect == CAMERA_EFFECT_OFF) {
    rc = effects_set_saturation(ctrl, value);
    ctrlCmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  } else {
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_EXPOSURE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_EXPOSURE(void *parm1, void *parm2)
{
  int8_t rc = FALSE;

  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  struct msm_ctrl_cmd *ccmd = (struct msm_ctrl_cmd *)parm2;
  uint32_t value = *(uint32_t *)ccmd->value;
  uint8_t bestshotMode = ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

  if (value < CAMERA_AEC_MAX_MODES) {
    /* Even though exposure metering is controlled by bestshot we will allow
     * user to change value after bestshot mode is applied */
    if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR) {
#if 0 //TODO Enable once sensor supports set_exposure.
      sensor_set_t set_param;
      set_param.type = SENSOR_SET_EXPOSURE;
      rc = ctrl->sensorCtrl.fn_table.sensor_set_exposure_mode((int8_t)value);
      ccmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
#endif
    } else {
      stats_proc_set_t set_param;
      set_param.type = STATS_PROC_AEC_TYPE;
      set_param.d.set_aec.type = AEC_METERING_MODE;
      set_param.d.set_aec.d.aec_metering = *(int32_t *)ccmd->value;
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf));
      if (rc < 0)
        CDBG_ERROR("%s FAILED to set AEC_METERING_MODE\n", __func__);
      ccmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
    }
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_AUTO_FOCUS -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_AUTO_FOCUS(void *parm1, void *parm2)
{
  int8_t rc = FALSE;

  CDBG("%s\n", __func__);
  mmcamera_util_profile("set_parm_AF: ");
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  int32_t value = *(int32_t *)ctrlCmd->value;
  stats_proc_set_t set_param;
  stats_proc_get_t stats_get;

  /* Check underlying AF mode. If AF mode is infinity, we just return.
   True in case like Landscape bestshot mode */
  stats_get.type = STATS_PROC_AF_TYPE;
  stats_get.d.get_af.type = AF_FOCUS_MODE;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].get_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_get.type,
    &stats_get, sizeof(stats_get));
  if (rc < 0) {
    CDBG_ERROR("%s: Getting AF MODE failed\n", __func__);
    return rc;
  }
  CDBG("%s: Received AF MODE: %d", __func__, stats_get.d.get_af.d.af_mode);

  if (stats_get.d.get_af.d.af_mode == AF_MODE_INFINITY) {
    CDBG_ERROR("%s: Focus mode INFINITY. AutoFocus disabled!!!", __func__);
    ctrlCmd->status = CAM_CTRL_SUCCESS;
    /* Send autofocus done event as application might be waiting
     * for af done event*/
    mctl_af_send_focus_done_event(ctrl, CAMERA_EXIT_CB_DONE);
    return TRUE;
  }

  if ((ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value !=
    CAMERA_BESTSHOT_OFF) &&
    ctrl->stats_proc_ctrl.intf.output.af_d.reset_lens) {
    ctrlCmd->status = CAM_CTRL_FAILED;
    return TRUE;
  }

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR &&
    ctrl->afCtrl.af_enable) {
    if (value < AF_MODE_MAX) {
      /* If autoFocus is called when CAF is enabled, we'll need to
         handle it differently */
      if (ctrl->afCtrl.af_cont_enable) {
        /* first get the current CAF status - whether it's already
           Focused or curently Focusing or unknown (may or may not
           be focused). If Focused or Unknown we send back AF
           event with success or failure. If Focusing, we'll send
           event later */
        rc = mctl_af_get_caf_status(ctrl);
        if (rc < 0) {
          CDBG_ERROR("%s: Getting CAF status failed!", __func__);
          ctrlCmd->status = CAM_CTRL_FAILED;
        }
        else {
          /* We'll need to lock the focus unless cancel_autofocus
             is called. */
          set_param.type = STATS_PROC_AF_TYPE;
          set_param.d.set_af.type = AF_LOCK_CAF;
          set_param.d.set_af.d.af_lock_caf = TRUE;
          rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
            ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type,
            &set_param, &(ctrl->stats_proc_ctrl.intf));
          if (rc < 0) {
            CDBG_ERROR("%s: Lock CAF failed %d\n", __func__, rc);
            ctrlCmd->status = CAM_CTRL_FAILED;
          }
        }
        return TRUE;
      }

      CDBG("%s : AF Started, mctl_af_start...", __func__);
      rc = mctl_af_start(ctrl,
        ctrl->afCtrl.parm_focusrect.current_value);

      if (rc < 0) {
        ctrlCmd->status = CAM_CTRL_FAILED;
        CDBG_ERROR("%s: FAILED to set AF STATS...\n", __func__);
        return TRUE;
      }
      /* Set Auto Focus parameters */
      CDBG("%s : AF Started, Set Auto Focus parameters", __func__);
      set_param.type = STATS_PROC_AF_TYPE;
      set_param.d.set_af.type = AF_START_PARAMS;
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param,
        &(ctrl->stats_proc_ctrl.intf));
      if (rc < 0) {
        CDBG_ERROR("%s: FAILED to set AF_SET_START_PARAMS\n", __func__);
        ctrlCmd->status = CAM_CTRL_FAILED;
        return TRUE;
      }

      /*Pre-flash*/
      stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
      stats_proc_get_t stats_flash_get;
      stats_flash_get.type = STATS_PROC_AEC_TYPE;
      stats_flash_get.d.get_aec.type = AEC_QUERY_FLASH_FOR_SNAPSHOT;
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].get_params(
           ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_flash_get.type,
           &stats_flash_get, sizeof(stats_flash_get));
      if (stats_flash_get.d.get_aec.d.query_flash_for_snap == 2) {
        flash_led_set_t led_set_parm;
        led_set_parm.data.led_state = MSM_CAMERA_LED_LOW;
        sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_LOW;
        CDBG("%s: set led to low\n", __func__);
		ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
		 ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
		 FLASH_SET_STATE, &led_set_parm, NULL);
      }

      /* Reset Lens */
      CDBG("%s : AF Started, Reset Lens", __func__);
      set_param.d.set_af.type = AF_RESET_LENS;
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param,
        &(ctrl->stats_proc_ctrl.intf));
      if (rc < 0) {
        CDBG_ERROR("%s: FAILED to set AF_RESET_LENS\n", __func__);
        ctrlCmd->status = CAM_CTRL_FAILED;
        return TRUE;
      }

    if (ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle) {
      rc = ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
        ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
          ACTUATOR_DEF_FOCUS, NULL, NULL);
        if (rc != 0) {
          CDBG_ERROR("%s(%d)Failure:Reset lens failed\n",
            __FILE__, __LINE__);
          ctrlCmd->status = CAM_CTRL_FAILED;
        } else {
          CDBG("%s: Reset lens succeeded\n", __func__);
          ctrlCmd->status = CAM_CTRL_ACCEPTED;
        }
      }
    } else {
      CDBG_ERROR("%s: not supported on this sensor!\n", __func__);
      ctrlCmd->status = CAM_CTRL_FAILED;
    }
  } else {
    CDBG_ERROR("%s: invalid autofocus value %d\n", __func__, value);
    ctrlCmd->status = CAM_CTRL_FAILED;
  }

  CDBG("%s: rc=%d\n", __func__, rc);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_ISO -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_ISO(void *parm1, void *parm2)
{
  int8_t rc = 1;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  uint8_t bestshotMode = ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

  int value = *(int *)cmd->value;

  if (value <= (int)max_camera_iso_type &&
     (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR)) {
    stats_proc_set_t set_param;
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_ISO_MODE;
    set_param.d.set_aec.d.aec_iso_mode = value;

    if (bestshotMode == CAMERA_BESTSHOT_OFF) {
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf));
      if (rc < 0)
        CDBG_ERROR("%s FAILED to set ISO_MODE\n", __func__);
      cmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
    } else {
      cmd->status = CAM_CTRL_SUCCESS;
      rc = FALSE ;
    }
  } else {
    sensor_get_t sensor_get;
    sensor_set_t sensor_set;
    sensor_set.data.iso = value;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_SET_ISO, &sensor_set, NULL);
    cmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  }

  return TRUE;
}

/*===========================================================================
FUNCTION      config_proc_set_wb

DESCRIPTION
===========================================================================*/
static int8_t config_proc_set_wb(mctl_config_ctrl_t *ctrl, uint32_t parm)
{
  int8_t status = FALSE;
  int rc = 0;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_set_t set_param;
  set_param.type = STATS_PROC_AWB_TYPE;
  set_param.d.set_awb.type = AWB_WHITE_BALANCE;
  set_param.d.set_awb.d.awb_current_wb = parm;
  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR) {
    sensor_get_t sensor_get;
    sensor_set_t sensor_set;
    sensor_set.data.whitebalance = parm;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_SET_WHITEBALANCE, &sensor_set, NULL);

  } else {
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
             ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
             set_param.type, &set_param, &(ctrl->stats_proc_ctrl.intf));

    config3a_wb_t wb = parm;

    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_WB,
             (void *)&wb, NULL);
  if (rc) {
    CDBG_ERROR("%s: Manual WB set failed", __func__);
    return FALSE;
   }
  }
  return rc;
} /* config_proc_set_wb */

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_WB -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_WB(void *parm1, void *parm2)
{
  int8_t rc = FALSE;

  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;

  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  int8_t current_spl_effect =
    ctrl->effectCtrl.specialEffectsInfo.parm.current_value;

  uint8_t bestshotMode =
    ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

   CDBG("%s: WB %d", __func__, *(int32_t *)(ctrlCmd->value));
  /* If any of the following four special effects are set,
     then don't apply white balance.*/
  if (current_spl_effect == CAMERA_EFFECT_MONO ||
    current_spl_effect == CAMERA_EFFECT_NEGATIVE ||
    current_spl_effect == CAMERA_EFFECT_SEPIA ||
    current_spl_effect == CAMERA_EFFECT_AQUA ||
    current_spl_effect == CAMERA_EFFECT_EMBOSS ||
    current_spl_effect == CAMERA_EFFECT_SKETCH ||
    current_spl_effect == CAMERA_EFFECT_NEON ||
    current_spl_effect == CAMERA_EFFECT_FADED ||
    current_spl_effect == CAMERA_EFFECT_VINTAGECOOL ||
    current_spl_effect == CAMERA_EFFECT_VINTAGEWARM ||
    current_spl_effect == CAMERA_EFFECT_ACCENT_BLUE ||
    current_spl_effect == CAMERA_EFFECT_ACCENT_GREEN ||
    current_spl_effect == CAMERA_EFFECT_ACCENT_ORANGE ||
    bestshotMode != CAMERA_BESTSHOT_OFF) {
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  } else {
    rc = config_proc_set_wb(ctrl, *(int32_t *)(ctrlCmd->value));
    ctrlCmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  }
  return TRUE;
}
/*===========================================================================
 * FUNCTION    - config_proc_set_parm_effect -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_set_parm_effect(mctl_config_ctrl_t *ctrl, int32_t effect)
{
  int8_t rc = TRUE;

  CDBG("%s: effect =%d", __func__, effect);
    ctrl->effectCtrl.specialEffectsInfo.spl_effects_enabled = TRUE;
    rc = effects_set_special_effect(ctrl, effect);

  if (rc)
    ctrl->effectCtrl.specialEffectsInfo.parm.current_value = effect;

  return rc;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_EFFECT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_EFFECT(void *parm1, void *parm2)
{
  int8_t rc;
  int is_awb = TRUE;

  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;

  struct msm_ctrl_cmd *ctrlCmd =
    (struct msm_ctrl_cmd *)parm2;
  int32_t effect = *(int32_t *)ctrlCmd->value;

  uint8_t bestshotMode = ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

  /* TODO get params from 3A and check WB first and set is_awb */
  // is_awb = ctrl->stats_proc_ctrl.intf.output.awb_d.current_wb_type;
  CDBG("%s, value =%d", __func__, effect);
  if ((ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) &&
    !is_awb &&
    (effect == CAMERA_EFFECT_MONO ||
    effect  == CAMERA_EFFECT_NEGATIVE ||
    effect  == CAMERA_EFFECT_SEPIA ||
    effect  == CAMERA_EFFECT_AQUA ||
    effect == CAMERA_EFFECT_EMBOSS ||
    effect == CAMERA_EFFECT_SKETCH ||
    effect == CAMERA_EFFECT_NEON   ||
    effect == CAMERA_EFFECT_FADED  ||
    effect == CAMERA_EFFECT_VINTAGECOOL ||
    effect == CAMERA_EFFECT_VINTAGEWARM ||
    effect == CAMERA_EFFECT_ACCENT_BLUE ||
    effect == CAMERA_EFFECT_ACCENT_GREEN ||
    effect == CAMERA_EFFECT_ACCENT_ORANGE )) {
    CDBG("%s: cannot apply effect %d wb %d\n", __func__, effect,
      ctrl->stats_proc_ctrl.intf.output.awb_d.current_wb_type);
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  } else if (bestshotMode != CAMERA_BESTSHOT_OFF) {
    CDBG("%s: cannot apply effect since bestshot mode is set\n", __func__);
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  } else {
    rc = config_proc_set_parm_effect(ctrl, effect);
    ctrlCmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  }

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_FPS -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_FPS(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  uint32_t new_fps;
  uint16_t max_fps;

  new_fps = *((uint32_t *)cmd->value);
  max_fps = (new_fps & 0x0000FFFF);

  CDBG("config_proc_CAMERA_SET_PARM_FPS: %d\n", max_fps);
  sensor_get_t sensor_get;
  sensor_set_t sensor_set;
  sensor_set.data.aec_data.fps = max_fps;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
	  ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	  SENSOR_SET_FPS, &sensor_set, NULL);

  ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
	  ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	  SENSOR_GET_OUTPUT_CFG, &sensor_get, sizeof(sensor_get));

  cmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  if (!rc && sensor_get.data.sensor_output.output_format == SENSOR_BAYER) {
    /*notify 3a of the max fps change*/
    stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
    stats_proc_set_t sp_set_param;
    sp_set_param.type = STATS_PROC_AEC_TYPE;
    sp_set_param.d.set_aec.type = AEC_PARM_FPS;
    sp_set_param.d.set_aec.d.aec_fps = new_fps;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, sp_set_param.type, &sp_set_param, &(sp_ctrl->intf));
  }

  return TRUE;
}
/*===========================================================================
     * FUNCTION    - config_proc_CAMERA_SET_PARM_BESTSHOT_MODE -
     *
     * DESCRIPTION:
     *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_BESTSHOT_MODE(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;

  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);

  int8_t current_spl_effect =
    ctrl->effectCtrl.specialEffectsInfo.parm.current_value;

  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR) {
    CDBG("config_proc_CAMERA_SET_PARM_BESTSHOT_MODE: bestshot mode not supported\n");
    rc = FALSE;
  } else {

    stats_proc_set_t set_param;
    CDBG("config_proc_CAMERA_SET_PARM_BESTSHOT_MODE: %d\n",
      *(uint16_t *)ctrlCmd->value);

    rc = bestshot_set_mode(ctrl, &(ctrl->bestshotCtrl), *(uint8_t *)ctrlCmd->value);
    if (*(int32_t *)ctrlCmd->value == CAMERA_BESTSHOT_AUTO) {
      set_param.d.set_asd.d.asd_enable = 1;
    } else {
      set_param.d.set_asd.d.asd_enable = 0;
    }
    set_param.type = STATS_PROC_ASD_TYPE;
    set_param.d.set_asd.type = ASD_ENABLE;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf));

    if (ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value == CAMERA_BESTSHOT_OFF) {
      effects_set_special_effect(ctrl, current_spl_effect);
      if (current_spl_effect != CAMERA_EFFECT_POSTERIZE
          && current_spl_effect != CAMERA_EFFECT_SOLARIZE) {
        effects_set_contrast(ctrl,
                             ctrl->effectCtrl.contrastInfo.current_value);
      }
      if (current_spl_effect == CAMERA_EFFECT_OFF) {
        effects_set_saturation(ctrl,
                               ctrl->effectCtrl.saturationInfo.current_value);
      }
    }
  }

  ctrlCmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  CDBG("%s: rc = %d, status = %d", __func__, rc, ctrlCmd->status);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_ANTIBANDING -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_ANTIBANDING(void *parm1, void *parm2)
{
  int rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  uint8_t bestshotMode = ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;
  camera_antibanding_type value =
    *(camera_antibanding_type *)cmd->value;
  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR) {
    // TO  DO
    cmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  } else {
    stats_proc_set_t set_param;

    set_param.type = STATS_PROC_AFD_TYPE;
    set_param.d.set_afd.type = AFD_RESET;
    if (ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf)) < 0) {
      CDBG("%s Stats proc set param failed for AFD_ENABLE", __func__);
      cmd->status = CAM_CTRL_FAILED;
      return TRUE;
    }
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_ANTIBANDING;
    set_param.d.set_aec.d.aec_atb = value;
    if (bestshotMode != CAMERA_BESTSHOT_LANDSCAPE &&
      bestshotMode!= CAMERA_BESTSHOT_BEACH &&
      bestshotMode != CAMERA_BESTSHOT_SNOW &&
      bestshotMode != CAMERA_BESTSHOT_SPORTS &&
      bestshotMode != CAMERA_BESTSHOT_ACTION) {

      if(ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf)) < 0)
        CDBG_ERROR("FAILED to set AEC_ANTIBANDING\n");
      cmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
    } else {
      CDBG("Bestshotmode caused antibanding to be not set");
      cmd->status = CAM_CTRL_SUCCESS;
    }
  }
  CDBG("%s: %p, %d\n", __func__, cmd->value, *(int32_t *)cmd->value);
  return TRUE;
}
// TODO To create new file -> mctl_util.c and move this func there.
/*===========================================================================
 * FUNCTION    - mctl_util_convert_position_to_index -
 *
 * DESCRIPTION: convert coordinate to aec stats region index
 *==========================================================================*/
static uint32_t mctl_util_convert_position_to_stats_reg_index(void *cctrl,
  uint32_t x, uint32_t y)
{
  uint32_t reg_idx = 0;
  uint32_t region_number;
  int rc = 0;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  zoom_scaling_params_t *zoomscaling = &(ctrl->zoomCtrl.zoomscaling);
  uint32_t num_reg_by_line;
  int32_t sensor_size_w, sensor_size_h; /* stats collection img size */
  int32_t reg_size_w, reg_size_h; /* each stats clooection region size */
  int32_t fov_size_w, fov_size_h; /* fov output size */
  int32_t img_size_w, img_size_h; /* output/display img size */
  int32_t cur_level_x, cur_level_y;
  sensor_get_t sensor_get;

  /*Assuming that output1 is for display*/
  img_size_w = ctrl->dimInfo.display_width;
  img_size_h = ctrl->dimInfo.display_height;
  region_number = ctrl->stats_proc_ctrl.intf.input.mctl_info.numRegions;

  CDBG("%s: location (%d, %d)\n", __func__, x, y);
  CDBG("Display img size w%d x h%d, region_number =%d\n",
    img_size_w, img_size_h, region_number);
  switch (region_number) {
    case  256:
      num_reg_by_line = 16;
      break;

    case 64:
      num_reg_by_line = 8;
      break;

    case 16:
      num_reg_by_line = 4;
      break;

    default:
      num_reg_by_line = 0;
  }
  if (ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
	  ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	  SENSOR_GET_CAMIF_CFG, &sensor_get, sizeof(sensor_get)) < 0) {
    CDBG_ERROR("%s: sensor_get_params failed\n", __func__);
    return 0;
  }

  if (num_reg_by_line != 0) {
    sensor_size_w = sensor_get.data.camif_setting.last_pixel -
      sensor_get.data.camif_setting.first_pixel + 1;
    sensor_size_h = sensor_get.data.camif_setting.last_line -
      sensor_get.data.camif_setting.first_line + 1;

    reg_size_w = sensor_size_w / num_reg_by_line;
    reg_size_h = sensor_size_h / num_reg_by_line;

    CDBG("stats collection size w%d x h%d, region size w%d x h%d \n",
      sensor_size_w, sensor_size_h,reg_size_w, reg_size_h);
    /*check the display level scaling*/
    if (zoomscaling->input1_width !=0 && zoomscaling->output1_width != 0) {
      CDBG("Scaling input w%d x h%d, output w%d x h%d\n",
        zoomscaling->input1_width, zoomscaling->input1_height,
        zoomscaling->output1_width, zoomscaling->output1_height);
      /*there is croping and upscaling in display*/
      cur_level_x = zoomscaling->output1_width -
        zoomscaling->input1_width;
      cur_level_y = zoomscaling->output1_height -
        zoomscaling->input1_height;
      cur_level_x = cur_level_x/2 + x * zoomscaling->input1_width/
        zoomscaling->output1_width;
      cur_level_y = cur_level_y/2 + y * zoomscaling->input1_height/
        zoomscaling->output1_height;

      CDBG("X=%d, Y= %d\n", cur_level_x, cur_level_y);
      /* when display cropping and upscaling is enable, FOV size is the same
       * as output image size */
      cur_level_x += (sensor_size_w - img_size_w)/2;
      cur_level_y += (sensor_size_h - img_size_h)/2;

      CDBG("x=%d, y= %d, num_reg=%d\n", cur_level_x,
        cur_level_y, num_reg_by_line);
      if (reg_size_h > 0)
        reg_idx = (uint32_t) (cur_level_x / reg_size_w) +
          num_reg_by_line * (uint32_t) (cur_level_y / reg_size_h);

    } else {
      vfe_fov_crop_params_t crop_win;
      rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
        ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_FOV_CROP_PARM,
        (void *)&crop_win, sizeof(crop_win));
      /*No display upscaling, consider the down scaling from fov to output*/
      fov_size_w = crop_win.last_pixel - crop_win.first_pixel + 1;
      fov_size_h = crop_win.last_line - crop_win.first_line + 1;
      CDBG("FOV size w%d x h%d\n", fov_size_w, fov_size_h);
      if (fov_size_w != img_size_w || fov_size_h != img_size_h) {
        cur_level_x =  x * fov_size_w / img_size_w;
        cur_level_y =  y * fov_size_h / img_size_h;
      } else {
        cur_level_x =  x;
        cur_level_y =  y;
      }
      cur_level_x += (sensor_size_w - fov_size_w) / 2;
      cur_level_y += (sensor_size_h - fov_size_h) / 2;
      if (reg_size_h > 0)
        reg_idx = (uint32_t)(cur_level_x / reg_size_w) +
          num_reg_by_line * (uint32_t)(cur_level_y / reg_size_h);

    }

    if (reg_idx >= region_number) {
      reg_idx = region_number -1;
    }
  }
  CDBG("reg_idx = %d\n", reg_idx);
  return reg_idx;
} /* mctl_util_convert_position_to_stats_reg_index */

cam_format_t mm_camera_convert_mbusfmt_to_camfmt(uint32_t pxlfmt)
{
  cam_format_t fmt = CAMERA_BAYER_SBGGR10;

  switch(pxlfmt) {
    case V4L2_MBUS_FMT_SBGGR10_1X10:
      fmt = CAMERA_BAYER_SBGGR10;
      break;
    case V4L2_MBUS_FMT_YUYV8_2X8:
      fmt = CAMERA_YUV_422_YUYV;
      break;
  }
  return fmt;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_AEC_LOCK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_AEC_LOCK(void *parm1, void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  int value = *(int *)cmd->value;
  stats_proc_set_t set_param;

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_SET_LOCK;
    set_param.d.set_aec.d.force_aec_lock = value;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
               ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
               set_param.type, &set_param, &(sp_ctrl->intf));
    if (rc < 0)
      CDBG_ERROR("FAILED to set AEC_LOCK \n");
  } else {
    CDBG_HIGH("%s AEC_LOCK not supported on this sensor ", __func__);
  }
  cmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  CDBG("%s: %p, %d\n", __func__, cmd->value, *(int32_t *)cmd->value);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_AEC_ROI -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_AEC_ROI(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  cam_set_aec_roi_t *cam_set_aec_roi;
  uint32_t currROIindx = 0;

  CDBG("%s Sensor output_format=%d \n", __func__,
       ctrl->sensorCtrl.sensor_output.output_format);

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    CDBG("cmd->value %p, length =%d\n", cmd->value, cmd->length);
    cam_set_aec_roi =  (cam_set_aec_roi_t *)cmd->value;
    CDBG("cmd enable %d, type %d, index %d\n",
      cam_set_aec_roi->aec_roi_enable, cam_set_aec_roi->aec_roi_type,
      cam_set_aec_roi->aec_roi_position.aec_roi_idx);
    if (cam_set_aec_roi->aec_roi_enable) {
      if (cam_set_aec_roi->aec_roi_type == AEC_ROI_BY_COORDINATE) {
        currROIindx = mctl_util_convert_position_to_stats_reg_index (ctrl,
          cam_set_aec_roi->aec_roi_position.coordinate.x,
          cam_set_aec_roi->aec_roi_position.coordinate.y);
        CDBG("%s: By location (%d,%d)\n", __func__,
          cam_set_aec_roi->aec_roi_position.coordinate.x,
          cam_set_aec_roi->aec_roi_position.coordinate.y);
      } else if (cam_set_aec_roi->aec_roi_type == AEC_ROI_BY_INDEX) {
        currROIindx = cam_set_aec_roi->aec_roi_position.aec_roi_idx;
        CDBG("%s: By index %d\n", __func__, currROIindx);
      } else {
        rc = FALSE;
        CDBG_ERROR("%s: wrong type\n", __func__);
      }
    } else {
      CDBG("%s: disable AEC_ROI\n", __func__);
      currROIindx =0; /*don't care this value*/
    }

    CDBG("%s ROI index=%d\n", __func__, currROIindx);

    /* The coordinate convert call already takes care of num_regions,
     * no need to consider it here anymore */
    if (rc) {
      int ret = 0;
      stats_proc_set_t set_param;
      set_param.type = STATS_PROC_AEC_TYPE;
      set_param.d.set_aec.type = AEC_SET_ROI;
      set_param.d.set_aec.d.aec_roi.enable =
        cam_set_aec_roi->aec_roi_enable;
      set_param.d.set_aec.d.aec_roi.rgn_index = currROIindx;
      ret = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf));
      rc = (!ret)?TRUE:FALSE;
    }
    if (!rc)
      CDBG_ERROR("%s FAILED to set AEC_ROI\n", __func__);
  } else {
    rc = FALSE;
    CDBG("%s YUV sensor, AEC_INTERESTED_REGION not supported\n", __func__);
  }
  CDBG("%s %p, %d\n", __func__, cmd->value, *(uint32_t *)cmd->value);
  cmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_AEC_MTR_AREA -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_AEC_MTR_AREA(void *parm1, void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_set_t set_param;
  aec_mtr_area_t mtr_info =  *(aec_mtr_area_t *)cmd->value;

  if (ctrl->comp_mask & (1 << MCTL_COMPID_STATSPROC)) {
    memcpy(&(set_param.d.set_aec.d.mtr_area.weight[0]),&(mtr_info.weight[0]),
      MAX_ROI * sizeof(int));
    memcpy(&(set_param.d.set_aec.d.mtr_area.mtr_area[0]),&(mtr_info.mtr_area[0]),
      MAX_ROI * sizeof(roi_t));
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_SET_MTR_AREA;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
           ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
           set_param.type, &set_param, &(sp_ctrl->intf));
  } else {
    CDBG_ERROR("%s StatsProc module not enabled. Metering area not configured",
      __func__);
  }
  if (rc < 0)
    CDBG_ERROR("FAILED to set AEC_MTR_AREA \n");
  cmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  CDBG("%s: %p, %d\n", __func__, cmd->value, *(int32_t *)cmd->value);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_AWB_LOCK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_AWB_LOCK(void *parm1, void *parm2)
{
  int rc = 0;
  int value;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_set_t set_param;

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    value = *(int *)cmd->value;
    set_param.type = STATS_PROC_AWB_TYPE;
    set_param.d.set_awb.type = AWB_EZ_LOCK_OUTPUT;
    set_param.d.set_awb.d.ez_lock_output = value;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
               ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
               set_param.type, &set_param, &(sp_ctrl->intf));
    if (rc < 0)
      CDBG_ERROR("Failed to set AWB Lock\n");
  } else {
    CDBG_HIGH("%s AWB_LOCK not enabled on this sensor ", __func__);
  }
  cmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  CDBG("%s: %p, %d\n", __func__, cmd->value, *(int32_t *)cmd->value);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_SCE_FACTOR -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_SCE_FACTOR(void *parm1, void *parm2)
{
  int rc = -1;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  int32_t sce_factor = *(int32_t *)ctrlCmd->value;

  CDBG("%s: SCE adj factor %d", __func__, sce_factor);

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_SCE,
               (void *)&sce_factor, NULL);
  } else {
    rc = 0;
    CDBG_HIGH("SCE not supported on this sensor\n");
  }

  ctrlCmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_MCE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_MCE(void *parm1, void *parm2)
{
  int rc = -1;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  uint32_t enable = *(uint32_t *)ctrlCmd->value;
  vfe_status_t status = VFE_SUCCESS;

  CDBG("%s: MCE %d", __func__, enable);

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_MCE,
               (void *)&enable, NULL);
  } else {
    rc = 0;
    CDBG_HIGH("MCE not supported on this sensor\n");
  }

  ctrlCmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_REDEYE_REDUCTION -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_REDEYE_REDUCTION(void *parm1,  void *parm2)
{
  int8_t rc = -EINVAL;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);

  uint32_t redeye_reduction_mode = *(uint32_t *)(cmd->value);

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    stats_proc_set_t set_param;
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_REDEYE_REDUCTION_MODE;
    set_param.d.set_aec.d.aec_redeye_reduction_mode = redeye_reduction_mode;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf));
  }

  cmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_HFR -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_SET_PARM_HFR(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  uint32_t value = *(uint32_t *)ctrlCmd->value;
  vfe_hfr_info_t  hfr_info;
  hfr_info.preview_hfr = FALSE;
  hfr_info.hfr_mode = value;

  CDBG("%s: HFR %d", __func__, value);
  ctrl->hfr_mode = value;
  ctrl->preview_hfr = FALSE;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
           ctrl->comp_ops[MCTL_COMPID_VFE].handle,
           VFE_SET_HFR_MODE, &hfr_info, NULL);
  }
  ctrlCmd->status = (rc == VFE_SUCCESS) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_AF_MODE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_SET_PARM_AF_MODE(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  stats_proc_set_t set_param;
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  int32_t af_mode = *(int32_t *)ctrlCmd->value;

  CDBG("%s: Set AF Mode to %d\n", __func__, af_mode);

  if (af_mode < AF_MODE_MAX) {
    if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR &&
      ctrl->afCtrl.af_enable) {
      stats_proc_set_t stats_af_set_data;
      stats_af_set_data.type = STATS_PROC_AF_TYPE;
      stats_af_set_data.d.set_af.type = AF_MODE;
      stats_af_set_data.d.set_af.d.af_mode = af_mode;
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_af_set_data.type,
        &stats_af_set_data,
        &(ctrl->stats_proc_ctrl.intf));
      if (rc < 0) {
        CDBG_ERROR("%s: STATS_PROC_AF_TYPE failed %d\n", __func__, rc);
        ctrlCmd->status = CAM_CTRL_FAILED;
        return TRUE;
      }

      /* Reset lens if we have to */
      if (ctrl->stats_proc_ctrl.intf.output.af_d.reset_lens == TRUE &&
        (ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle)) {
        CDBG("%s Resetting Lens\n", __func__);
        rc = ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
          ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
          ACTUATOR_DEF_FOCUS, NULL, NULL);
        if (rc != 0) {
          CDBG_ERROR("File Name:%s, Line#%d,Failure:Reset lens failed\n",
          __FILE__, __LINE__);
          rc = FALSE;
        } else {
          CDBG("Reset lens succeeded\n");
          rc = TRUE;
        }
        ctrl->stats_proc_ctrl.intf.output.af_d.reset_lens = FALSE;
      }
    }
  }

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_HISTOGRAM -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_HISTOGRAM(void *parm1, void *parm2)
{
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;

  ctrl->vfeData.enable_histogram = *((int8_t *)(ctrlCmd->value));
  CDBG("%s Histogram Enable = %s ", __func__, ctrl->vfeData.enable_histogram ? "Yes" : "No");
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_WAVELET_DENOISE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_WAVELET_DENOISE(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  frame_proc_set_t fp_set_param;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  denoise_param_t denoise_param = *(denoise_param_t *)ctrlCmd->value;
  frame_proc_key_t fp_key;

  fp_set_param.type = FRAME_PROC_WAVELET_DENOISE;
  fp_set_param.d.set_wd.type = WAVELET_DENOISE_ENABLE;
  fp_set_param.d.set_wd.denoise_enable = denoise_param.denoise_enable;
  fp_set_param.d.set_wd.process_planes = denoise_param.process_plates;
  if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle, fp_set_param.type,
    &fp_set_param, &(fp_ctrl->intf))<0 ){
    CDBG_ERROR("%s Frame proc set param failed for Wavelet Denoise",
      __func__);
    ctrlCmd->status = CAM_CTRL_FAILED;
  } else {
    if (fp_ctrl->intf.output.wd_d.denoise_enable &&
      ctrl->ops_mode != CAM_OP_MODE_ZSL)
      fp_key = FP_SNAPSHOT_SET;
    else
      fp_key = FP_SNAPSHOT_RESET;
    rc = mctl_divert_set_key(ctrl, fp_key);
    if (!rc) {
      CDBG_ERROR("%s Error setting postprocessing mode", __func__);
      ctrlCmd->status = CAM_CTRL_FAILED;
    } else
      ctrlCmd->status = CAM_CTRL_SUCCESS;
  }
  ctrl->denoise_enable = fp_set_param.d.set_wd.denoise_enable; // remember denoise flag
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_AUTO_FOCUS_CANCEL -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_AUTO_FOCUS_CANCEL(void *parm1, void *parm2)
{
  int8_t rc = TRUE;

  CDBG("%s\n", __func__);
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  stats_proc_set_t stats_af_set_data;

  /* If cancel autofocus is called while we are in CAF mode,
     we need to unlock CAF. */
  if (ctrl->afCtrl.af_cont_enable) {
    CDBG("%s: Unlock CAF now!", __func__);
    stats_af_set_data.type = STATS_PROC_AF_TYPE;
    stats_af_set_data.d.set_af.type = AF_LOCK_CAF;
    stats_af_set_data.d.set_af.d.af_lock_caf = FALSE;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_af_set_data.type,
        &stats_af_set_data, &(ctrl->stats_proc_ctrl.intf));
    if (rc < 0) {
      CDBG_ERROR("%s: Unlock CAF failed %d\n", __func__, rc);
      ctrlCmd->status = CAM_CTRL_FAILED;
    }

    /* Reset send_event_later flag just in case it's set before */
    stats_af_set_data.type = STATS_PROC_AF_TYPE;
    stats_af_set_data.d.set_af.type = AF_SEND_CAF_DONE_EVT_LTR;
    stats_af_set_data.d.set_af.d.af_send_evt_ltr = FALSE;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_af_set_data.type,
      &stats_af_set_data, &(ctrl->stats_proc_ctrl.intf));
  } else {/* for normal snapshot */
    /* set param AF_CANCEL */
    if ((ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) &&
         ctrl->afCtrl.af_enable) {
      stats_af_set_data.type = STATS_PROC_AF_TYPE;
      stats_af_set_data.d.set_af.type = AF_CANCEL;
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_af_set_data.type,
        &stats_af_set_data, &(ctrl->stats_proc_ctrl.intf));

      flash_led_get_t led_get_parm;
      ctrl->comp_ops[MCTL_COMPID_FLASHLED].get_params(
        ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
        FLASH_GET_MODE, &led_get_parm, sizeof(led_get_parm));
      if (led_get_parm.data.led_mode != LED_MODE_OFF &&
        led_get_parm.data.led_mode != LED_MODE_TORCH) {
        flash_led_set_t led_set_parm;
        led_set_parm.data.led_state = MSM_CAMERA_LED_OFF;
        stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
        sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_OFF;
        ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
          ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
          FLASH_SET_STATE, &led_set_parm, NULL);
      }
      if (rc < 0) {
        CDBG_ERROR("%s: Cancelling AF failed!", __func__);
        ctrlCmd->status = CAM_CTRL_FAILED;
      }
    }
  }

  return rc;

}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_MOTION_ISO -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_MOTION_ISO(void *parm1, void *parm2)
{
  int8_t rc = FALSE;

  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);

  uint32_t motion_iso  = *(uint32_t *)ctrlCmd->value;

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    stats_proc_set_t set_param;
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_MOTION_ISO;
    set_param.d.set_aec.d.aec_motion_iso = motion_iso;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf));
  } else {
    rc = FALSE;
  }
  ctrlCmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_ENABLE_AFD -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_ENABLE_AFD(void *parm1,void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
#ifndef VFE_2X
  stats_proc_ctrl_t  *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_set_t set_param;
#else
  stats_proc_ctrl_t  *sp_ctrl = &(ctrl->stats_proc_ctrl);
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  frame_proc_set_t set_param;
  frame_proc_key_t fp_key;
  stats_proc_set_t s_set_param;
#endif

#ifndef VFE_2X
  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    set_param.type = STATS_PROC_AFD_TYPE;
    set_param.d.set_afd.type = AFD_ENABLE;
    set_param.d.set_afd.afd_enable = 1;
    /*Update this param from HAL */
    set_param.d.set_afd.afd_mode = *(int *)ctrlCmd->value;
    if((ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf))) < 0) {
      CDBG_ERROR("%s Stats proc set param failed for AFD_ENABLE", __func__);
      ctrlCmd->status = CAM_CTRL_FAILED;
      return TRUE;
    }
}
#else
  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    set_param.type = FRAME_PROC_AFD;
    set_param.d.set_afd.type = FRAME_PROC_AFD_ENABLE;
    /*Update this param from HAL */
    set_param.d.set_afd.afd_enable = *(int *)ctrlCmd->value;
    if((ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle, set_param.type, &set_param, &(fp_ctrl->intf))) < 0) {
      CDBG_ERROR("%s Stats proc set param failed for AFD_ENABLE", __func__);
      ctrlCmd->status = CAM_CTRL_FAILED;
      return TRUE;
    }
    /* Start diverting preview frames from kernel */
    fp_key = FP_PREVIEW_SET;
    rc = mctl_divert_set_key(ctrl, fp_key);
    if (!rc)
      CDBG_ERROR("%s Error setting PREVIEW KEY", __func__);
}
#endif
  CDBG("%s: X status %d rc %d", __func__, ctrlCmd->status, rc);
  ctrlCmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}
/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_FD -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_FD (void *parm1, void *parm2)
{
  int32_t rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  fd_set_parm_t parm = *(fd_set_parm_t*)cmd->value;
  int fd_mode = parm.fd_mode;
  if (fd_mode == CLEAR_ALBUM) {
    frame_proc_set_t fd_set_parm;
    fd_set_parm.type = FRAME_PROC_FACE_DETECT;
    fd_set_parm.d.set_fd.type = FACE_CLEAR_ALBUM;
    if (fp_ctrl->lib.frame_proc_set_params(fp_ctrl->handle,
      &fd_set_parm, &(fp_ctrl->intf)) < 0) {
      CDBG_ERROR("%s  Frame proc set param failed for Face Detect",
        __func__);
      return -1;
    }
    return TRUE;
  }
  ctrl->fd_mode =  parm.fd_mode;
  if(parm.num_fd <= 0)
    parm.num_fd = MAX_ROI;
  ctrl->num_fd = MIN(parm.num_fd, MAX_ROI);
  rc = config_proc_face_detection_cmd (parm1, ctrl->fd_mode);
  cmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_PREPARE_SNAPSHOT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_PREPARE_SNAPSHOT(void * parm1,void * parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;

  /*Todo: remove it after 3A 4.0*/
#ifdef VFE_40
  CDBG_HIGH("%s: No 3A for VFE40, return before prepare snapshot\n", __func__);
  cmd->status = CAM_CTRL_SUCCESS;
  return rc;
#endif

  mmcamera_util_profile("Prepare snapshot starts ");
  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    flash_led_get_t led_get_parm;
	ctrl->comp_ops[MCTL_COMPID_FLASHLED].get_params(
	 ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
	 FLASH_GET_STATE, &led_get_parm, sizeof(led_get_parm));

    flash_strobe_get_t strobe_get_parm;
	ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].get_params(
	  ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].handle,
	  FLASH_GET_STATE, &strobe_get_parm, sizeof(strobe_get_parm));

    vfe_stats_conf_enable_t  vfe_stats_info;
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
      ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_STATS_CONF_ENABLE,
      (void *)&vfe_stats_info, sizeof(vfe_stats_info));

    if(!vfe_stats_info.aec_enabled){
     CDBG_ERROR("%s AEC has not been enabled.Hence returning",__func__);
     cmd->status = CAM_CTRL_SUCCESS;
     return TRUE;
    }
    stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
    sp_ctrl->intf.input.flash_info.led_state = led_get_parm.data.led_state;
    sp_ctrl->intf.input.flash_info.strobe_chrg_ready = strobe_get_parm.data.strobe_ready;
    stats_proc_set_t sp_set_param;
    sp_set_param.type = STATS_PROC_AEC_TYPE;
    sp_set_param.d.set_aec.type = AEC_PREPARE_FOR_SNAPSHOT;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, sp_set_param.type, &sp_set_param, &(sp_ctrl->intf));

    if (led_get_parm.data.led_state == MSM_CAMERA_LED_OFF &&
      sp_ctrl->intf.output.aec_d.led_state == MSM_CAMERA_LED_LOW) {
      flash_led_set_t led_set_parm;
      led_set_parm.data.led_state = MSM_CAMERA_LED_LOW;
      sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_LOW;
      CDBG("%s: set led to low\n", __func__);
	  ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
	   ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
	   FLASH_SET_STATE, &led_set_parm, NULL);
    }
    cmd->status = CAM_CTRL_SUCCESS;

    if (rc < 0) {
      CDBG("prepare snapshot: block until aec settle\n");
      return FALSE;
    }
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_FPS_MODE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_FPS_MODE(void * parm1,void * parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  uint8_t bestshotMode = ctrl->bestshotCtrl.bestshotModeInfo.parm.current_value;

  sensor_get_t sensor_get;
  ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
	  ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	  SENSOR_GET_OUTPUT_CFG, &sensor_get, sizeof(sensor_get));

  if (sensor_get.data.sensor_output.output_format == SENSOR_BAYER) {
    stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
    stats_proc_set_t sp_set_param;
    sp_set_param.type = STATS_PROC_AEC_TYPE;
    sp_set_param.d.set_aec.type = AEC_FPS_MODE;
    sp_set_param.d.set_aec.d.aec_fps = *(fps_mode_t *)(cmd->value);
    CDBG("%s: sensor mode =%d", __func__, ctrl->sensor_op_mode);

    if (bestshotMode == CAMERA_BESTSHOT_OFF) {
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, sp_set_param.type, &sp_set_param, &(sp_ctrl->intf));
      if (rc) {
        CDBG_ERROR("FAILED to set AEC_FPS_MODE\n");
      } else {
        /*restore to max_preview_fps, since max_preview_fps is Q8 number,
         convert max_fps to regular number and round it up, latere the
         sensor will cap it back to the real max_preview_fps*/
        sensor_get_t sensor_get;
        if (ctrl->state != CAMERA_STATE_STARTED)
          sensor_get.data.aec_info.op_mode = SENSOR_MODE_VIDEO;
        else
          sensor_get.data.aec_info.op_mode = ctrl->sensor_op_mode;
		ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
	      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	      SENSOR_GET_SENSOR_MODE_AEC_INFO, &sensor_get, sizeof(sensor_get));

        uint32_t max_fps = (sensor_get.data.aec_info.max_fps + 255)>>8;
        max_fps = (max_fps & 0x0000FFFF);  /*set min fps to min*/

        if (max_fps) {
          sensor_set_t sensor_set;
          sensor_set.data.aec_data.fps = max_fps;
          rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
	        ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	        SENSOR_SET_FPS, &sensor_set, NULL);


          sp_set_param.type = STATS_PROC_AEC_TYPE;
          sp_set_param.d.set_aec.type = AEC_PARM_FPS;
          sp_set_param.d.set_aec.d.aec_fps = max_fps;
          ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
            ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, sp_set_param.type, &sp_set_param, &(sp_ctrl->intf));
        }
      }
    }
    cmd->status = (!rc) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_ctrlcmd_CAMERA_SET_CAF -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_ASD_ENABLE(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("In config_proc_CAMERA_SET_ASD_ENABLE\n");

  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_BAYER) {
    stats_proc_set_t set_param;
    set_param.type = STATS_PROC_ASD_TYPE;
    set_param.d.set_asd.type = ASD_ENABLE;
    set_param.d.set_asd.d.asd_enable = *(int32_t *)cmd->value;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type, &set_param, &(sp_ctrl->intf));
    if (rc < 0)
      CDBG_ERROR("%s FAILED to set config_proc_CAMERA_SET_ASD_ENABLE\n", __func__);
    cmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  } else {
    cmd->status = CAM_CTRL_FAILED;
  }

  CDBG("%s: %p, %d\n", __func__,
    cmd->value, *(int32_t *)cmd->value);

  return TRUE;
}


/*===========================================================================
 * FUNCTION    - config_proc_ctrlcmd_CAMERA_SET_CAF -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_CAF(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd =
    (struct msm_ctrl_cmd *)parm2;
  stats_proc_set_t caf_params;

  caf_ctrl_t *caf  = (caf_ctrl_t *)ctrlCmd->value;
  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    caf_params.type = STATS_PROC_AF_TYPE;
    caf_params.d.set_af.type = AF_CONTINUOS_FOCUS;
    caf_params.d.set_af.d.af_conti_focus = *caf;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, caf_params.type,
      &caf_params, &(ctrl->stats_proc_ctrl.intf));
    CDBG("%s: af_conti_focus = %d, rc = %d\n", __func__, *caf, rc);
    if (rc>=0) {
      /* Enable CAF */
      if (*caf) {
        /* Only if CAF is not already enabled set trigger_CAF to TRUE */
        if (!ctrl->afCtrl.af_cont_enable) {
          ctrl->stats_proc_ctrl.intf.input.mctl_info.trigger_CAF = TRUE;
        }
      }
      else {
        ctrl->stats_proc_ctrl.intf.input.mctl_info.trigger_CAF = FALSE;
      }

      /* if CAF was previously enabled and we are disabling it now, we need
       *  to stop AF stats generation too */
      if (ctrl->afCtrl.af_cont_enable && !(*caf)) {
        CDBG("%s: Disabling CAF. Stop AF stats generation!", __func__);
        mctl_af_stop(ctrl);
      }
      ctrl->afCtrl.af_cont_enable = *caf;
    }
  } else {
    rc = -EINVAL;
  }

  ctrlCmd->status = (rc>=0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_ROI -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_PARM_AF_ROI(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  uint32_t num_roi;
  uint32_t i;
  stats_proc_set_t set_param;
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd =
    (struct msm_ctrl_cmd *)parm2;

  CDBG("%s: E\n", __func__);

  roi_info_t *mctl_roi_info = &ctrl->afCtrl.roiInfo;
  roi_info_t *roi_info = (roi_info_t *)ctrlCmd->value;
  memset(&set_param.d.set_af.d.roiInfo, 0, sizeof(stats_proc_roi_info_t));
  memcpy(mctl_roi_info, roi_info, sizeof(roi_info_t));

  num_roi = roi_info->num_roi;

#ifdef DRAW_RECTANGLES
  camframe_roi.num_roi = num_roi;
#endif

  CDBG("%s: num_roi = %d\n", __func__, num_roi);
  for (i = 0; i < num_roi; i++) {
#ifdef DRAW_RECTANGLES
    camframe_roi.roi[i] = isp3a_roi_info->roi[i];
#endif
    set_param.d.set_af.d.roiInfo.roi[i].x = mctl_roi_info->roi[i].x;
    set_param.d.set_af.d.roiInfo.roi[i].dx = mctl_roi_info->roi[i].dx;
    set_param.d.set_af.d.roiInfo.roi[i].y = mctl_roi_info->roi[i].y;
    set_param.d.set_af.d.roiInfo.roi[i].dy = mctl_roi_info->roi[i].dy;
    CDBG("%s: mctl_roi_info: x=%d, y=%d, dx=%d, dy=%d\n", __func__,
      mctl_roi_info->roi[i].x,  mctl_roi_info->roi[i].y,
      mctl_roi_info->roi[i].dx, mctl_roi_info->roi[i].dy);
  }

  set_param.type = STATS_PROC_AF_TYPE;
  set_param.d.set_af.type = AF_ROI;
  set_param.d.set_af.d.roiInfo.num_roi = num_roi;
  set_param.d.set_af.d.roiInfo.roi_updated = TRUE;
  set_param.d.set_af.d.roiInfo.frm_height = ctrl->dimInfo.display_height;
  set_param.d.set_af.d.roiInfo.frm_width = ctrl->dimInfo.display_width;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type,
    &set_param, &(ctrl->stats_proc_ctrl.intf));


  ctrlCmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;

  CDBG("%s: X\n", __func__);

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_RECORDING_HINT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_RECORDING_HINT(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  if (ctrlCmd->value) {
    ctrlCmd->status = CAM_CTRL_SUCCESS;
    ctrl->videoHint = *((uint32_t *)ctrlCmd->value);
  } else {
    ctrlCmd->status = CAM_CTRL_FAILED;
    ctrl->videoHint = 0;
  }
  CDBG_HIGH("%s rc = %d, status = %d, Current usecase : %s \n", __func__,
             rc, ctrlCmd->status, ctrl->videoHint ? "Camcorder" : "Camera");
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_CHANNEL_STREAM -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_CHANNEL_STREAM(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  uint32_t channel_stream_info = STREAM_IMAGE;
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  if (ctrlCmd->value)
    channel_stream_info = *((uint32_t *)ctrlCmd->value);
  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    ispif_set_t ispif_set;
    ispif_get_t ispif_get;
    CDBG("%d", channel_stream_info);
    ispif_set.data.channel_stream_info = channel_stream_info;
    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
      ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
      ISPIF_SET_INTF_PARAMS, &ispif_set, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ispif_set_params failed %d\n", __func__, rc);
      ctrlCmd->status = CAM_CTRL_FAILED;
      return FALSE;
    }
    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
      ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
      ISPIF_PROCESS_CFG, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ispif_process_cfg failed %d\n", __func__, rc);
      ctrlCmd->status = CAM_CTRL_FAILED;
      return FALSE;
    }
    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].get_params(
      ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
      ISPIF_GET_CHANNEL_INFO, &ispif_get, sizeof(ispif_get));
    if (rc < 0) {
      CDBG_ERROR("%s: ispif_get_interface failed %d\n", __func__, rc);
      ctrlCmd->status = CAM_CTRL_FAILED;
      return FALSE;
    }
    ctrl->channel_interface_mask = ispif_get.data.channel_interface_mask;
    ctrl->channel_stream_info = ispif_get.data.channel_stream_info;
  } else {
    ctrl->channel_interface_mask = PIX_0;
    ctrl->channel_stream_info = STREAM_IMAGE;
  }
  CDBG("%s rc = %d, status = %d\n", __func__,
    rc, ctrlCmd->status);
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_LOW_POWER_MODE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_LOW_POWER_MODE(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  if (ctrlCmd->value) {
    ctrl->enableLowPowerMode = *((uint32_t *)ctrlCmd->value);
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  } else {
    ctrlCmd->status = CAM_CTRL_FAILED;
    ctrl->enableLowPowerMode = 0;
  }
  CDBG_HIGH("%s Low power mode enabled? : %s \n", __func__,
             ctrl->enableLowPowerMode ? "Yes" : "No");

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_DIS_ENABLE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_SET_DIS_ENABLE(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  if (ctrlCmd->value) {
    ctrl->video_dis.enable_dis = *((uint32_t *)ctrlCmd->value);
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  } else {
    rc = -EINVAL;
    ctrlCmd->status = CAM_CTRL_FAILED;
    ctrl->video_dis.enable_dis = 0;
  }

  CDBG_HIGH("%s: DIS Enabled? : %s \n", __func__,
        ctrl->video_dis.enable_dis ? "Yes" : "No");

  return rc;
}


/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_SET_PARM_HDR
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_SET_PARM_HDR(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  frame_proc_set_t fp_set_param;
  frame_proc_key_t fp_key;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  exp_bracketing_t *exp_values;
  exp_values = (exp_bracketing_t *)cmd->value;

  switch (exp_values->mode) {
    case HDR_BRACKETING_OFF:
      ctrl->hdrCtrl.exp_bracketing_enable = FALSE;
      ctrl->hdrCtrl.hdr_enable = FALSE;
    break;
    case HDR_MODE:
      ctrl->hdrCtrl.exp_bracketing_enable = FALSE;
      ctrl->hdrCtrl.hdr_enable = TRUE;
      ctrl->hdrCtrl.total_hal_frames = exp_values->total_hal_frames;
    break;
    case EXP_BRACKETING_MODE:
      ctrl->hdrCtrl.exp_bracketing_enable = TRUE;
      ctrl->hdrCtrl.hdr_enable = FALSE;
      ctrl->hdrCtrl.total_frames = exp_values->total_frames;
      ctrl->hdrCtrl.total_hal_frames = exp_values->total_hal_frames;
      ctrl->hdrCtrl.total_mctl_frames = ctrl->hdrCtrl.total_frames
                                         - ctrl->hdrCtrl.total_hal_frames;
      if (strlen(exp_values->values) < MAX_EXP_BRACKETING_LENGTH) {
        strlcpy(ctrl->hdrCtrl.user_exp_values, exp_values->values,
              strlen(exp_values->values) + 1);
      } else {
        CDBG_ERROR("%s exposure values length %d exceeds "
          "MAX_EXP_BRACKETING_LENGTH = %d", __func__,
          strlen(exp_values->values), MAX_EXP_BRACKETING_LENGTH);
        rc = FALSE;
      }
    default:
    break;
  }
  if (rc == FALSE) {
    CDBG_ERROR("%s Not setting HDR Mode to frameproc ", __func__);
    cmd->status = CAM_CTRL_FAILED;
    return TRUE;
  }

  if (ctrl->comp_mask & (1 << MCTL_COMPID_FRAMEPROC)) {
      fp_set_param.type = FRAME_PROC_HDR;
      fp_set_param.d.set_hdr.type = FRAME_PROC_HDR_ENABLE;
      fp_set_param.d.set_hdr.hdr_init_info.hdr_enable = ctrl->hdrCtrl.hdr_enable;
      if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
        ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle, fp_set_param.type,
        &fp_set_param, &(fp_ctrl->intf))<0 ){
        CDBG_ERROR("%s Frame proc set param failed for HDR",
          __func__);
        rc = FALSE;
      }
  }

  if (ctrl->hdrCtrl.exp_bracketing_enable)
    hdr_get (HDR_PARM_FRM_USER, ctrl);
  else if (ctrl->hdrCtrl.hdr_enable)
    hdr_get (HDR_PARM_FRM_EXP, ctrl);

  cmd->status = rc ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  return TRUE;
}

static int config_get_size_with_pad(int width, int height, cam_pad_format_t pad_fmt, int factor)
{
  switch(pad_fmt) {
    case CAMERA_PAD_TO_2K:
      return PAD_TO_2K(height*width/factor);
    case CAMERA_PAD_TO_4K:
      return PAD_TO_4K(height*width/factor);
    case CAMERA_PAD_TO_8K:
      return PAD_TO_8K(height*width/factor);
    case CAMERA_PAD_TO_WORD:
    default:
      return PAD_TO_WORD(height*width/factor);
  }
  return 0;
}

/*===========================================================================
FUNCTION    - config_swap_dimensions -

DESCRIPTION
===========================================================================*/
static void config_swap_dimensions(uint16_t * x, uint16_t * y)
{
  *x ^= *y;
  *y ^= *x;
  *x ^= *y;
} /* vfe_state_swap_dimensions */

/*===========================================================================
 * FUNCTION    - config_validate_dimension -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY: This function needs to be called after set_parms.
 *==========================================================================*/
static int config_validate_dimension(
  mctl_config_ctrl_t *ctrl,
  const cam_ctrl_dimension_t *Indim,
  cam_ctrl_dimension_t *OpDim)
{
  uint32_t initial_downsample_ratio,decimated_input_width;
  uint32_t vert_downsample_ratio = 1;
  uint32_t horz_downsample_ratio = 1;
  uint32_t num_pixels_per_64_bits = 8;
  uint32_t ui_aspect_ratio, thumbnail_aspect_ratio;
  uint32_t mode = 0;

  CDBG("%s: E", __func__);
  *OpDim = *Indim;
  OpDim->rdi0_width = 0;
  OpDim->rdi0_height = 0;
  OpDim->rdi1_width = 0;
  OpDim->rdi1_height = 0;

  /* RAW Snapshot dimensions */
  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR) {
    OpDim->raw_picture_width = ctrl->sensorCtrl.full_size_width * 2;
  } else {
    switch (ctrl->sensorCtrl.sensor_output.raw_output) {
      case SENSOR_8_BIT_DIRECT:
        num_pixels_per_64_bits = 8;
        break;
      case SENSOR_10_BIT_DIRECT:
        num_pixels_per_64_bits = 6;
        break;
      case SENSOR_12_BIT_DIRECT:
        num_pixels_per_64_bits = 5;
        break;
      default:
        CDBG_HIGH("%s Unsupported raw image output type: %d\n", __func__,
                   ctrl->sensorCtrl.sensor_output.raw_output);
        break;
    }

    OpDim->raw_picture_width =
      (ctrl->sensorCtrl.full_size_width + num_pixels_per_64_bits - 1) /
      num_pixels_per_64_bits * 8;
  }
  OpDim->raw_picture_height = ctrl->sensorCtrl.full_size_height;

  /* save the original dimensions from the app */
  OpDim->orig_picture_dx = Indim->picture_width;
  OpDim->orig_picture_dy = Indim->picture_height;

#ifndef VFE_2X
  OpDim->video_width = OpDim->orig_video_width;
  OpDim->video_height = OpDim->orig_video_height;
#else
  OpDim->video_width = OpDim->display_width;
  OpDim->video_height = OpDim->display_height;
#endif
  /* TBD: fix me. vfe_set_dis_dimension(vfe_ctrl_obj, OpDim);*/
  /* swap the dimensions if the width is less than height */
  if (OpDim->picture_width < OpDim->picture_height) {
    /* Landscape from UI */
    config_swap_dimensions(&(OpDim->picture_width),
      &(OpDim->picture_height));
  }
  /* If the display width or height is 0, use the picture width and height) */
  OpDim->display_width = (OpDim->display_width == 0)? OpDim->picture_width :
    OpDim->display_width;
  OpDim->display_height = (OpDim->display_height == 0)? OpDim->picture_height :
    OpDim->display_height;

  /* Delimit the image size to maximum sensor output */
  if ((OpDim->picture_width > ctrl->sensorCtrl.full_size_width) ||
    (OpDim->picture_height > ctrl->sensorCtrl.full_size_height)) {
    OpDim->picture_width = (ctrl->sensorCtrl.full_size_width);
    OpDim->picture_height = (ctrl->sensorCtrl.full_size_height);

    OpDim->display_width = (OpDim->display_width > OpDim->picture_width) ?
      OpDim->picture_width : OpDim->display_width;

    OpDim->display_height = (OpDim->display_height > OpDim->picture_height) ?
      OpDim->picture_height : OpDim->display_height;

    OpDim->orig_picture_dx = OpDim->picture_width;
    OpDim->orig_picture_dy = OpDim->picture_height;
  }

  /* make the image dimensions multiple of 16 - JPEG requirement */
  OpDim->picture_width = CEILING16(OpDim->picture_width);

  /* Make sure thumbnail is not larger than picture */
  if (OpDim->ui_thumbnail_width > OpDim->orig_picture_dx ||
    OpDim->ui_thumbnail_height > OpDim->orig_picture_dy) {
    OpDim->thumbnail_width = OpDim->orig_picture_dx;
    OpDim->thumbnail_height = OpDim->orig_picture_dy;
  } else {
    OpDim->thumbnail_width = OpDim->ui_thumbnail_width;
    OpDim->thumbnail_height = OpDim->ui_thumbnail_height;
  }

  /* match aspect ratio of thumbnail to main image */
  ui_aspect_ratio = (OpDim->picture_width * Q12) / OpDim->picture_height;
  thumbnail_aspect_ratio =
    (OpDim->thumbnail_width * Q12) / OpDim->thumbnail_height;

  if (thumbnail_aspect_ratio < ui_aspect_ratio) {
    /* if thumbnail is narrower than main image, in other words wide and
     * snapshot then we want to adjust the height of the thumbnail to match
     * the main image aspect ratio. */
    OpDim->thumbnail_height = OpDim->ui_thumbnail_height =
      CEILING32((OpDim->thumbnail_width * Q12) / ui_aspect_ratio);
  } else if (thumbnail_aspect_ratio != ui_aspect_ratio) {
    /* if thumbnail is wider than main image we want to adjust width of the
     * thumbnail to match main image aspect ratio
     */
    OpDim->thumbnail_width = OpDim->ui_thumbnail_width =
      CEILING32((OpDim->thumbnail_height * ui_aspect_ratio) / Q12);
  }

  if (ctrl->sensorCtrl.qtr_size_height && ctrl->sensorCtrl.full_size_height)
    vert_downsample_ratio =
      (uint16_t) ((((float) ctrl->sensorCtrl.full_size_height) /
      ((float) ctrl->sensorCtrl.qtr_size_height)) + 0.5);

  if (ctrl->sensorCtrl.qtr_size_width && ctrl->sensorCtrl.full_size_width)
    horz_downsample_ratio =
      (uint16_t) ((((float) ctrl->sensorCtrl.full_size_width) /
      ((float) ctrl->sensorCtrl.qtr_size_width)) + 0.5);

  if (vert_downsample_ratio != horz_downsample_ratio) {
    /* assume camsensor always decimates in dy
     * We need to calculate decimation vfe will perform in dx */
    initial_downsample_ratio = vert_downsample_ratio / horz_downsample_ratio;

    decimated_input_width = ctrl->sensorCtrl.qtr_size_width / initial_downsample_ratio;

    if (OpDim->display_width > FLOOR16(decimated_input_width))
      OpDim->display_width = FLOOR16(decimated_input_width);
  }

  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR)
    OpDim->rdi0_width = ctrl->sensor_stream_fullsize ?
    ctrl->sensorCtrl.full_size_width * 2:ctrl->sensorCtrl.qtr_size_width * 2;
  else
    OpDim->rdi0_width = ctrl->sensor_stream_fullsize ?
    ctrl->sensorCtrl.full_size_width :ctrl->sensorCtrl.qtr_size_width;
  OpDim->rdi0_height = ctrl->sensor_stream_fullsize ?
    ctrl->sensorCtrl.full_size_height:ctrl->sensorCtrl.qtr_size_height;

  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR)
    OpDim->rdi1_width = ctrl->sensor_stream_fullsize ?
    ctrl->sensorCtrl.full_size_width * 2:ctrl->sensorCtrl.qtr_size_width * 2;
  else
    OpDim->rdi1_width = ctrl->sensor_stream_fullsize ?
    ctrl->sensorCtrl.full_size_width :ctrl->sensorCtrl.qtr_size_width;
  OpDim->rdi1_height = ctrl->sensor_stream_fullsize ?
    ctrl->sensorCtrl.full_size_height:ctrl->sensorCtrl.qtr_size_height;

  CDBG("%s: X", __func__);
  return 0;
} /* config_validate_dimension */

/*===========================================================================
 * FUNCTION    - config_calc_dim_offset_app_preview -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_calc_dim_offset_app_preview(mctl_config_ctrl_t *ctrl,
  cam_ctrl_dimension_t *dim)
{
  uint32_t size;
  int chrom_stride;

  if(dim->prev_format == CAMERA_YUV_420_YV12)
    dim->display_frame_offset.num_planes = 3;
  else
    dim->display_frame_offset.num_planes = 2;

  if(!dim->display_frame_offset.num_planes ||
       dim->display_frame_offset.num_planes == 1) {
    dim->display_frame_offset.sp.cbcr_offset =
      config_get_size_with_pad(dim->display_width, dim->display_height,
        dim->prev_padding_format, 1);
    dim->display_frame_offset.sp.len =
      dim->display_frame_offset.sp.cbcr_offset +
      config_get_size_with_pad(dim->display_width, dim->display_height,
        dim->prev_padding_format, 2);
    dim->display_frame_offset.sp.y_offset = 0;
    dim->display_frame_offset.frame_len =
      PAD_TO_4K(dim->display_frame_offset.sp.len);
  } else if (dim->display_frame_offset.num_planes == 3 ){ //NV12
    chrom_stride = ((dim->display_width/2+15)/16)*16;
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_Y].len =
      config_get_size_with_pad(dim->display_width, dim->display_height,
        dim->prev_padding_format, 1);
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_Y].offset = 0;
    /*Chrome 1*/
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CB].len =
      config_get_size_with_pad(chrom_stride, dim->display_height,
        dim->prev_padding_format, 2);
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CB].offset = 0;
    /*Chrome 2*/
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CR].len =
      config_get_size_with_pad(chrom_stride, dim->display_height,
        dim->prev_padding_format, 2);
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CR].offset = 0;

    dim->display_frame_offset.frame_len = PAD_TO_4K(
      dim->display_frame_offset.mp[V4L2_MULTI_PLANE_Y].len +
      dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CB].len +
      dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CR].len);
    CDBG("%s: size =%dx%d, chrom_stride =%d, plane: len0=%d, len1=%d,len2=%d",
        __func__, dim->display_width, dim->display_height, chrom_stride,
        dim->display_frame_offset.mp[V4L2_MULTI_PLANE_Y].len ,
      dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CB].len ,
      dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CR].len);

  } else {
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_Y].len =
      config_get_size_with_pad(dim->display_width, dim->display_height,
        dim->prev_padding_format, 1);
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_Y].offset = 0;
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len =
      config_get_size_with_pad(dim->display_width, dim->display_height,
        dim->prev_padding_format, 2);
    dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset = 0;
    dim->display_frame_offset.frame_len = PAD_TO_4K(
      dim->display_frame_offset.mp[V4L2_MULTI_PLANE_Y].len +
      dim->display_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len);
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_calc_dim_offset_app_video -
 *
 * DESCRIPTION:
 *==========================================================================*/
#ifdef VFE_40
/*Update video buffer size calculation for Venus*/
static int8_t config_calc_dim_offset_app_video(mctl_config_ctrl_t *ctrl,
  cam_ctrl_dimension_t *dim)
{
  uint32_t size;
  dim->video_frame_offset.num_planes= 2;

  if(!dim->video_frame_offset.num_planes ||
     dim->video_frame_offset.num_planes == 1) {
    /* use adjusted video width and height for length */
      dim->video_frame_offset.sp.cbcr_offset =
        PAD_TO_2K(dim->orig_video_height * dim->orig_video_width);
      dim->video_frame_offset.sp.len =
        PAD_TO_2K(dim->video_height * dim->video_width) +
        PAD_TO_2K(dim->video_height * dim->video_width/2);
      dim->video_frame_offset.sp.y_offset = 0;
      dim->video_frame_offset.frame_len =
        PAD_TO_4K(dim->video_frame_offset.sp.len);
  } else {
    uint32_t size, luma_size, chroma_size;
    cal_video_buf_size(dim->video_width, dim->video_height,
                       &luma_size, &chroma_size, &size);

    dim->video_frame_offset.mp[V4L2_MULTI_PLANE_Y].len = luma_size;
    dim->video_frame_offset.mp[V4L2_MULTI_PLANE_Y].offset = 0;
    dim->video_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len = chroma_size;
    dim->video_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset = 0;
    dim->video_frame_offset.frame_len = size;
  }
  return TRUE;
}
#else
static int8_t config_calc_dim_offset_app_video(mctl_config_ctrl_t *ctrl,
  cam_ctrl_dimension_t *dim)
{
  uint32_t size;
  dim->video_frame_offset.num_planes= 2;

  if(!dim->video_frame_offset.num_planes ||
     dim->video_frame_offset.num_planes == 1) {
    /* use adjusted video width and height for length */
      dim->video_frame_offset.sp.cbcr_offset =
        PAD_TO_2K(dim->orig_video_height * dim->orig_video_width);
      dim->video_frame_offset.sp.len =
        PAD_TO_2K(dim->video_height * dim->video_width) +
        PAD_TO_2K(dim->video_height * dim->video_width/2);
      dim->video_frame_offset.sp.y_offset = 0;
      dim->video_frame_offset.frame_len =
        PAD_TO_4K(dim->video_frame_offset.sp.len);
  } else {
    dim->video_frame_offset.mp[V4L2_MULTI_PLANE_Y].len =
      PAD_TO_2K(dim->video_height*dim->video_width);
    dim->video_frame_offset.mp[V4L2_MULTI_PLANE_Y].offset = 0;
    dim->video_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len =
      PAD_TO_2K(dim->video_height*dim->video_width/2);
    dim->video_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset = 0;
    dim->video_frame_offset.frame_len = PAD_TO_4K(
      dim->video_frame_offset.mp[V4L2_MULTI_PLANE_Y].len +
      dim->video_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len);
  }
  return TRUE;
}
#endif
/*===========================================================================
 * FUNCTION    - config_calc_dim_offset_app_snapshot -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_calc_dim_offset_app_snapshot(mctl_config_ctrl_t *ctrl,
  cam_ctrl_dimension_t *dim)
{
  uint32_t size;

  dim->picture_frame_offset.num_planes = 2;
  if(!dim->picture_frame_offset.num_planes ||
      dim->picture_frame_offset.num_planes == 1) {
    dim->picture_frame_offset.sp.cbcr_offset =
      PAD_TO_WORD(CEILING16(dim->picture_height) * dim->picture_width);
    if (ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV61 ||
        ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV16)
      dim->picture_frame_offset.sp.len =
        dim->picture_frame_offset.sp.cbcr_offset +
          PAD_TO_WORD(CEILING16(dim->picture_height) * dim->picture_width);
    else
       dim->picture_frame_offset.sp.len =
        dim->picture_frame_offset.sp.cbcr_offset +
            PAD_TO_WORD(CEILING16(dim->picture_height) * dim->picture_width/2);

    if(dim->rotation == 90 || dim->rotation == 180) {
      dim->picture_frame_offset.sp.y_offset =
        PAD_TO_WORD((CEILING16(dim->picture_height) -
          dim->picture_height) * dim->picture_width);
      dim->picture_frame_offset.sp.cbcr_offset +=
        PAD_TO_WORD((CEILING16(dim->picture_height) -
          dim->picture_height) * dim->picture_width/2);
    } else {
      dim->picture_frame_offset.sp.y_offset = 0;
    }
    dim->picture_frame_offset.frame_len =
      PAD_TO_4K(dim->picture_frame_offset.sp.len );
  } else {
    dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_Y].len =
      PAD_TO_WORD(CEILING16(dim->picture_height)*dim->picture_width);
    if (ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV61 ||
        ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV16)
      dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len =
        PAD_TO_WORD(CEILING16(dim->picture_height) * dim->picture_width);
    else
      dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len =
        PAD_TO_WORD(CEILING16(dim->picture_height) * dim->picture_width/2);

      dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_Y].offset =
        dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_Y].len -
          PAD_TO_WORD(dim->picture_height * dim->picture_width);
      if (ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV61 ||
          ctrl->video_ctrl.main_img_format == CAMERA_YUV_422_NV16)
        dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset =
          dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len -
            PAD_TO_WORD(dim->picture_height * dim->picture_width);
      else
        dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset =
          dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len -
          PAD_TO_WORD(dim->picture_height * dim->picture_width/2);

 /*Always Double padding for snapshot*/
    dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_Y].len +=
      dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_Y].offset;
   dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len +=
      dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset;

    dim->picture_frame_offset.frame_len = PAD_TO_4K(
      dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_Y].len +
      dim->picture_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len );
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_calc_dim_offset_app_thumb -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_calc_dim_offset_app_thumb(mctl_config_ctrl_t *ctrl,
  cam_ctrl_dimension_t *dim)
{
  uint32_t size;

  dim->thumb_frame_offset.num_planes = 2;
  if(!dim->thumb_frame_offset.num_planes ||
     dim->thumb_frame_offset.num_planes == 1) {
    dim->thumb_frame_offset.sp.cbcr_offset =
      PAD_TO_WORD(CEILING16(dim->thumbnail_height) * dim->thumbnail_width);
    if (ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV61 ||
        ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV16)
      dim->thumb_frame_offset.sp.len =
        PAD_TO_WORD(CEILING16(dim->thumbnail_height) * dim->thumbnail_width) +
          PAD_TO_WORD(CEILING16(dim->thumbnail_height) * dim->thumbnail_width);
    else
      dim->thumb_frame_offset.sp.len =
        PAD_TO_WORD(CEILING16(dim->thumbnail_height) * dim->thumbnail_width) +
        PAD_TO_WORD(CEILING16(dim->thumbnail_height) * dim->thumbnail_width/2);

    if(dim->rotation == 90 || dim->rotation == 180) {
      dim->thumb_frame_offset.sp.y_offset =
        dim->thumb_frame_offset.sp.cbcr_offset -
          PAD_TO_WORD(dim->thumbnail_height * dim->thumbnail_width);
      dim->thumb_frame_offset.sp.cbcr_offset +=
        PAD_TO_WORD((CEILING16(dim->thumbnail_height) -
          dim->thumbnail_height) * dim->thumbnail_width/2);
    } else {
      dim->thumb_frame_offset.sp.y_offset = 0;
    }
    dim->thumb_frame_offset.frame_len =
      PAD_TO_4K(dim->thumb_frame_offset.sp.len);
  } else {
    dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_Y].len =
      PAD_TO_WORD(CEILING16(dim->thumbnail_height)*dim->thumbnail_width);
    if (ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV61 ||
        ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV16)
      dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len =
        PAD_TO_WORD(CEILING16(dim->thumbnail_height) * dim->thumbnail_width);
    else
      dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len =
        PAD_TO_WORD(CEILING16(dim->thumbnail_height) * dim->thumbnail_width/2);
      dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_Y].offset =
        dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_Y].len -
        PAD_TO_WORD(dim->thumbnail_height * dim->thumbnail_width);
      if (ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV61 ||
          ctrl->video_ctrl.thumb_format == CAMERA_YUV_422_NV16)
        dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset =
          dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len -
          PAD_TO_WORD(dim->thumbnail_height * dim->thumbnail_width);
      else
        dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset =
          dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len -
          PAD_TO_WORD(dim->thumbnail_height * dim->thumbnail_width/2);
    /*always double padding*/
    dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_Y].len +=
      dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_Y].offset;
   dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len +=
      dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].offset;

    dim->thumb_frame_offset.frame_len = PAD_TO_4K(
      dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_Y].len +
      dim->thumb_frame_offset.mp[V4L2_MULTI_PLANE_CBCR].len );
  }
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_calc_dim_offset_app_raw -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_calc_dim_offset_app_raw(mctl_config_ctrl_t *ctrl,
  cam_ctrl_dimension_t *dim)
{
  uint32_t size;

  dim->picture_frame_offset.num_planes = 1;

  dim->picture_frame_offset.mp[0].len =
        PAD_TO_WORD(dim->raw_picture_width  * dim->raw_picture_height);
  dim->picture_frame_offset.mp[0].offset = 0;
  dim->picture_frame_offset.frame_len = dim->picture_frame_offset.mp[0].len;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_calc_stats_buf_size -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_calc_stats_buf_size(mctl_config_ctrl_t *ctrl,
  cam_stats_buf_dimension_t *dim)
{
  int rc = 0;

  uint32_t size;

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_STATS_BUF_SIZE,
    (void *)dim, sizeof(cam_stats_buf_dimension_t));

  CDBG("%s: type: %d, width : %d, height: %d\n", __func__,
    dim->type, dim->width, dim->height);

  return TRUE;
}

static int8_t config_CAMERA_SET_PARM_CID(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  cam_cid_info_t *cid_info = (cam_cid_info_t *)ctrlCmd->value;
  int inst_idx = -1, i, update_failed = 0;
  ctrlCmd->status = CAM_CTRL_FAILED;

  if (!cid_info || cid_info->num_cids > CAM_MAX_CID_NUM) {
    CDBG_ERROR("%s Invalid input ", __func__);
    return -EINVAL;
  }

  CDBG("%s User setting %d CID entries ", __func__, cid_info->num_cids);
  for (i = 0; i < cid_info->num_cids; i++) {
      inst_idx = GET_VIDEO_INST_IDX(cid_info->cid_entries[i].inst_handle);
      if (inst_idx >= 0 && inst_idx <= MSM_MAX_DEV_INST) {
        if (ctrl->video_ctrl.strm_info.user[inst_idx].req_cid) {
          CDBG_ERROR("%s Warning: Already requested CID %d for video inst %d ",
            __func__, ctrl->video_ctrl.strm_info.user[inst_idx].cid_val,
            inst_idx);
          update_failed = 1;
          continue;
        }
        ctrl->video_ctrl.strm_info.user[inst_idx].req_cid = 1;
        ctrl->video_ctrl.strm_info.user[inst_idx].cid_val =
          cid_info->cid_entries[i].cid;
        CDBG_HIGH("%s Storing CID info for video inst %d as %d", __func__,
          inst_idx, ctrl->video_ctrl.strm_info.user[inst_idx].cid_val);
      } else {
        /* As of now, the control should not come here. But in the future,
         * if MCTL node is used by HAL or some other client, this could
         * get executed. Better to add support for that. */
        inst_idx = GET_MCTLPP_INST_IDX(cid_info->cid_entries[i].inst_handle);
        if (inst_idx >= 0 && inst_idx <= MSM_MAX_DEV_INST) {
          if (ctrl->video_ctrl.strm_info.mctl[inst_idx].req_cid) {
            CDBG_ERROR("%s Warning: Already requested CID %d for mctl inst %d ",
              __func__, ctrl->video_ctrl.strm_info.mctl[inst_idx].cid_val,
              inst_idx);
            update_failed = 1;
            continue;
          }
          ctrl->video_ctrl.strm_info.mctl[inst_idx].req_cid = 1;
          ctrl->video_ctrl.strm_info.mctl[inst_idx].cid_val =
            cid_info->cid_entries[i].cid;
          CDBG_HIGH("%s Storing CID info for mctl inst %d as %d", __func__,
            inst_idx, ctrl->video_ctrl.strm_info.mctl[inst_idx].cid_val);
        } else {
          CDBG_ERROR("%s Invalid inst idx %d ", __func__, inst_idx);
          /* Update of atleast 1 CID entry has failed. Indicate to user.*/
          update_failed = 1;
        }
      }
  }
  ctrlCmd->status = update_failed ? CAM_CTRL_FAILED : CAM_CTRL_SUCCESS;
  return TRUE;
}

static int8_t config_CAMERA_GET_PARM_FRAME_RESOLUTION(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  cam_frame_resolution_t *frame_res = (cam_frame_resolution_t *)ctrlCmd->value;
  cam_ctrl_dimension_t temp_dim;
  cam_stats_buf_dimension_t stats_dim;
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  switch (frame_res->image_mode) {
  case MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW:
    temp_dim.display_width       = frame_res->width;
    temp_dim.display_height      = frame_res->height;
    temp_dim.prev_format         = frame_res->format;
    temp_dim.prev_padding_format = frame_res->padding_format;
    temp_dim.rotation            = frame_res->rotation;
    config_calc_dim_offset_app_preview(ctrl, &temp_dim);
    frame_res->frame_offset      = temp_dim.display_frame_offset;
    break;

  case MSM_V4L2_EXT_CAPTURE_MODE_VIDEO:
    temp_dim.video_width        = frame_res->width;
    temp_dim.video_height       = frame_res->height;
    temp_dim.enc_format         = frame_res->format;
    temp_dim.enc_padding_format = frame_res->padding_format;
    temp_dim.rotation           = frame_res->rotation;
    config_calc_dim_offset_app_video(ctrl, &temp_dim);
    frame_res->frame_offset     = temp_dim.video_frame_offset;
    break;

  case MSM_V4L2_EXT_CAPTURE_MODE_MAIN:
    temp_dim.picture_width       = frame_res->width;
    temp_dim.picture_height      = frame_res->height;
    temp_dim.main_img_format     = frame_res->format;
    temp_dim.main_padding_format = frame_res->padding_format;
    temp_dim.rotation            = frame_res->rotation;
    config_calc_dim_offset_app_snapshot(ctrl, &temp_dim);
    frame_res->frame_offset      = temp_dim.picture_frame_offset;
    break;

  case MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL:
    temp_dim.thumbnail_width      = frame_res->width;
    temp_dim.thumbnail_height      = frame_res->height;
    temp_dim.thumb_format         = frame_res->format;
    temp_dim.thumb_padding_format = frame_res->padding_format;
    temp_dim.rotation             = frame_res->rotation;
    config_calc_dim_offset_app_thumb(ctrl, &temp_dim);
    frame_res->frame_offset       = temp_dim.thumb_frame_offset;
    break;

  case MSM_V4L2_EXT_CAPTURE_MODE_RDI:
    CDBG("%s Getting RDI0 dimension as %d x %d", __func__,
      ctrl->dimInfo.rdi0_width, ctrl->dimInfo.rdi0_height);
    temp_dim.raw_picture_width = ctrl->dimInfo.rdi0_width;
    temp_dim.raw_picture_height = ctrl->dimInfo.rdi0_height;
    config_calc_dim_offset_app_raw(ctrl, &temp_dim);
    frame_res->frame_offset = temp_dim.picture_frame_offset;
    frame_res->width        = temp_dim.raw_picture_width;
    frame_res->height       = temp_dim.raw_picture_height;
    break;

  case MSM_V4L2_EXT_CAPTURE_MODE_RDI1:
    CDBG("%s Getting RDI1 dimension as %d x %d", __func__,
      ctrl->dimInfo.rdi1_width, ctrl->dimInfo.rdi1_height);
    temp_dim.raw_picture_width = ctrl->dimInfo.rdi1_width;
    temp_dim.raw_picture_height = ctrl->dimInfo.rdi1_height;
    config_calc_dim_offset_app_raw(ctrl, &temp_dim);
    frame_res->frame_offset = temp_dim.picture_frame_offset;
    frame_res->width        = temp_dim.raw_picture_width;
    frame_res->height       = temp_dim.raw_picture_height;
    break;

  case MSM_V4L2_EXT_CAPTURE_MODE_RDI2:
    /* TODO:
     * For now, assume CID is 0, so just get the default sensor
     * dimension in the current configuration(Qtr/Full). In future,
     * based on the CID user has configured during the call to
     * config_CAMERA_SET_PARM_CID, we need to either:
     * - query the dimension from the sensor for that
     *   particular CID and calculate the frame offset(s).
     * OR
     * - use the w, h, format specified by the user in this call
     *   to calculate the frame offset(s).*/
    if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR)
      temp_dim.raw_picture_width = ctrl->sensor_stream_fullsize
        ? ctrl->sensorCtrl.full_size_width * 2
        : ctrl->sensorCtrl.qtr_size_width * 2;
    else
      temp_dim.raw_picture_width = ctrl->sensor_stream_fullsize
        ? ctrl->sensorCtrl.full_size_width
        : ctrl->sensorCtrl.qtr_size_width;
    temp_dim.raw_picture_height = ctrl->sensor_stream_fullsize
        ? ctrl->sensorCtrl.full_size_height
        : ctrl->sensorCtrl.qtr_size_height;
    config_calc_dim_offset_app_raw(ctrl, &temp_dim);
    frame_res->frame_offset = temp_dim.picture_frame_offset;
    frame_res->width        = temp_dim.raw_picture_width;
    frame_res->height       = temp_dim.raw_picture_height;
    break;
  case MSM_V4L2_EXT_CAPTURE_MODE_AEC:
  case MSM_V4L2_EXT_CAPTURE_MODE_AWB:
  case MSM_V4L2_EXT_CAPTURE_MODE_AF:
  case MSM_V4L2_EXT_CAPTURE_MODE_IHIST:
  case MSM_V4L2_EXT_CAPTURE_MODE_CS:
  case MSM_V4L2_EXT_CAPTURE_MODE_RS:
    stats_dim.type = frame_res->image_mode;
    config_calc_stats_buf_size(ctrl, &stats_dim);
    frame_res->width = stats_dim.width;
    frame_res->height = stats_dim.height;
    frame_res->frame_offset.frame_len =
      stats_dim.width * stats_dim.height;
    frame_res->frame_offset.num_planes = 1;
    frame_res->frame_offset.mp[0].len =
        PAD_TO_WORD(frame_res->width * frame_res->height);
    frame_res->frame_offset.mp[0].offset = 0;
    frame_res->frame_offset.frame_len = frame_res->frame_offset.mp[0].len;
    break;
  case MSM_V4L2_EXT_CAPTURE_MODE_RAW: {
    sensor_get_t sensor_get;
    cam_format_t format;
    int rc = 0;

    memset(&sensor_get, 0, sizeof(sensor_get));
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_OUTPUT_CFG, &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
      ctrlCmd->status = CAM_CTRL_FAILED;
      return rc;
    }
    frame_res->format = mm_camera_convert_mbusfmt_to_camfmt(
       sensor_get.data.pxlcode);
    sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_RAW_SNAPSHOT;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
               ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
               SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
      ctrlCmd->status = CAM_CTRL_FAILED;
      break;
    }
    frame_res->width = sensor_get.data.sensor_dim.width;
    frame_res->height = sensor_get.data.sensor_dim.height;
    frame_res->frame_offset.num_planes = 1;
    frame_res->frame_offset.mp[0].len =
        PAD_TO_WORD(frame_res->width * frame_res->height);
    frame_res->frame_offset.mp[0].offset = 0;
    frame_res->frame_offset.frame_len = frame_res->frame_offset.mp[0].len;
    CDBG("%s: RAW snapshot format = %d, width = %d, height = %d",
               __func__, frame_res->format, ->width, frame_res->height);
    break;
  }
  default:
    CDBG_ERROR("%s Invalid image mode %d ", __func__, frame_res->image_mode);
    ctrlCmd->status = CAM_CTRL_FAILED;
    break;
  }

  return TRUE;
}

static int8_t config_CAMERA_GET_PP_MASK(void *parm1, void *parm2)
{
  uint8_t pp_mask = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  uint8_t *mask = (uint8_t *)ctrlCmd->value;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);

  /* for 8960, need to check if WNR is enabled by user */
  /* zsl+wnr will require pp, other wise wnr will already be done by hardware */
  if (fp_ctrl->intf.output.wd_d.denoise_enable &&
      ctrl->ops_mode == CAM_OP_MODE_ZSL) {
    pp_mask |= CAMERA_PP_MASK_TYPE_WNR;
  }

  /* TODO: how to decide if it's badger?? */
  /* for badger, we always do pp for WNR */
  //pp_mask |= CAMERA_PP_MASK_TYPE_WNR;

  *mask = pp_mask;
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  return TRUE;
}

static int8_t config_CAMERA_DO_PP_WNR(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  mm_camera_wnr_info_t *wnr_info = (mm_camera_wnr_info_t *)ctrlCmd->value;

  struct msm_pp_frame frame;
  int ext_mode;
  int idx, i, rc = 0;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  frame_proc_set_t fp_set_param;
  vfe_pp_params_t vfe_pp_params;
  frame_proc_key_t fp_key;
  int prev_opt_mode;

  prev_opt_mode = fp_ctrl->intf.input.mctl_info.opt_mode;
  fp_ctrl->intf.input.mctl_info.opt_mode = FRAME_PROC_SNAPSHOT;
  fp_key = FP_PREVIEW_RESET;
  rc = mctl_divert_set_key(ctrl, fp_key);
  if (!rc)
    CDBG_ERROR("%s Error setting PREVIEW KEY", __func__);
  fp_ctrl->intf.input.statsproc_info.aec_d.lux_idx
    = ctrl->stats_proc_ctrl.intf.output.aec_d.lux_idx;

  if (ctrl->ops_mode == CAM_OP_MODE_SNAPSHOT) {
    fp_ctrl->intf.input.statsproc_info.aec_d.snap.real_gain
      = ctrl->stats_proc_ctrl.intf.output.aec_d.snap.real_gain;
    fp_ctrl->intf.input.statsproc_info.awb_d.snapshot_wb.g_gain
      = ctrl->stats_proc_ctrl.intf.output.awb_d.snapshot_wb.g_gain;
  }else {
    fp_ctrl->intf.input.statsproc_info.aec_d.snap.real_gain
      = ctrl->stats_proc_ctrl.intf.output.aec_d.cur_real_gain;
    fp_ctrl->intf.input.statsproc_info.awb_d.snapshot_wb.g_gain
      = ctrl->stats_proc_ctrl.intf.output.awb_d.curr_gains.g_gain;
  }
  /* Now get the gamma table info from VFE interface */
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_PP_INFO,
    (void *)&vfe_pp_params, sizeof(vfe_pp_params));
  fp_ctrl->intf.input.isp_info.VFE_GAMMA_NUM_ENTRIES =
    vfe_pp_params.gamma_num_entries;
  fp_ctrl->intf.input.isp_info.RGB_gamma_table =
    vfe_pp_params.gamma_table;
  fp_ctrl->intf.input.isp_info.lumaAdaptationEnable =
    vfe_pp_params.la_enable;
  fp_ctrl->intf.input.isp_info.VFE_LA_TABLE_LENGTH =
    vfe_pp_params.luma_num_entries;
  fp_ctrl->intf.input.isp_info.LA_gamma_table =
    vfe_pp_params.luma_table;

  fp_ctrl->intf.input.mctl_info.num_main_img = 0;
  fp_ctrl->intf.input.mctl_info.num_thumb_img = 0;
  for(idx = 0; idx < wnr_info->num_frames; idx++) {
    memset(&frame,  0,  sizeof(frame));
    ext_mode = GET_IMG_MODE(wnr_info->frames[idx].instance_hdl);
    switch (ext_mode) {
      case MSM_V4L2_EXT_CAPTURE_MODE_MAIN:
      case MSM_V4L2_EXT_CAPTURE_MODE_VIDEO:
        {
          fp_ctrl->intf.input.mctl_info.picture_dim.width = wnr_info->frames[idx].frame_width;
          fp_ctrl->intf.input.mctl_info.picture_dim.height = wnr_info->frames[idx].frame_height;
          fp_ctrl->intf.input.mctl_info.main_img_format = FRAME_PROC_H2V2;
          frame.image_type = ext_mode;
          frame.path = OUTPUT_TYPE_S;
          frame.num_planes = wnr_info->frames[idx].frame_offset.num_planes;
          if(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wnr_info->frames[idx].frame_idx].local_vaddr == NULL){
            continue;
          }
          for(i = 0; i < frame.num_planes; i++) {
            frame.mp[i].data_offset = wnr_info->frames[idx].frame_offset.mp[i].offset;
            frame.mp[i].length = wnr_info->frames[idx].frame_offset.mp[i].len;
            frame.mp[i].fd = ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wnr_info->frames[idx].frame_idx].fd;
            if (i > 0) {
              frame.mp[i].vaddr =
                (unsigned long)(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wnr_info->frames[idx].frame_idx].local_vaddr) + frame.mp[i-1].length;
            } else {
              frame.mp[i].vaddr =
                (unsigned long)(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wnr_info->frames[idx].frame_idx].local_vaddr);
            }
            CDBG("%s (buf,fd,length, offset) = (0x%lu, %d, %d, %d)", __func__, frame.mp[i].vaddr, frame.mp[i].fd, frame.mp[i].length, frame.mp[i].addr_offset);
          }
          memcpy(&(fp_ctrl->intf.input.mctl_info.main_img_frame[fp_ctrl->intf.input.mctl_info.num_main_img]),
              &frame, sizeof(struct msm_pp_frame));
          fp_ctrl->intf.input.mctl_info.num_main_img++;
        }
        break;
      case MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL:
      case MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW:
        {
          fp_ctrl->intf.input.mctl_info.thumbnail_dim.width = wnr_info->frames[idx].frame_width;
          fp_ctrl->intf.input.mctl_info.thumbnail_dim.height = wnr_info->frames[idx].frame_height;
          fp_ctrl->intf.input.mctl_info.thumb_img_format = FRAME_PROC_H2V2;
          frame.image_type = ext_mode;
          frame.path = OUTPUT_TYPE_T;
          frame.num_planes = wnr_info->frames[idx].frame_offset.num_planes;
          if(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wnr_info->frames[idx].frame_idx].local_vaddr == NULL){
            continue;
          }
          for(i = 0; i < frame.num_planes; i++) {
            frame.mp[i].data_offset = wnr_info->frames[idx].frame_offset.mp[i].offset;
            frame.mp[i].length = wnr_info->frames[idx].frame_offset.mp[i].len;
            frame.mp[i].fd = ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wnr_info->frames[idx].frame_idx].fd;
            frame.mp[i].vaddr = (unsigned long)(ctrl->video_ctrl.user_buf_info[ext_mode].local_frame_info[wnr_info->frames[idx].frame_idx].local_vaddr) + frame.mp[i-1].length;
            CDBG("%s (buf,fd,length, offset) = (0x%lu, %d, %d, %d)", __func__, frame.mp[i].vaddr, frame.mp[i].fd, frame.mp[i].length, frame.mp[i].addr_offset);
          }
          memcpy(&(fp_ctrl->intf.input.mctl_info.thumb_img_frame[fp_ctrl->intf.input.mctl_info.num_thumb_img]),
              &frame, sizeof(struct msm_pp_frame));
          fp_ctrl->intf.input.mctl_info.num_thumb_img++;
        }
        break;
      default:
        CDBG_ERROR("%s: invalid image mode %d", __func__, ext_mode);
        break;
    }
  }

  if (fp_ctrl->intf.input.mctl_info.num_main_img == 0 &&
      fp_ctrl->intf.input.mctl_info.num_thumb_img == 0) {
    CDBG_ERROR("%s: invalid frame info for WNR", __func__);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return TRUE;
  }

  fp_set_param.type = FRAME_PROC_WAVELET_DENOISE;
  fp_set_param.d.set_wd.type = WAVELET_DENOISE_CALIBRATE;
  if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle, fp_set_param.type,
    &fp_set_param, &(fp_ctrl->intf))<0 ){
    CDBG_ERROR("%s Error while calibrating Wavelet Denoise", __func__);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return TRUE;
  }

  if( ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].process(
    ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
    fp_ctrl->intf.input.mctl_info.opt_mode, &(fp_ctrl->intf)) < 0){
      CDBG_ERROR("%s Error executing wavelet denoise for snapshot ", __func__);
      ctrlCmd->status = CAM_CTRL_FAILED;
  } else {
      CDBG("%s Success processing Wavelet Denoise", __func__);
      ctrlCmd->status = CAM_CTRL_SUCCESS;
  }
  if (fp_ctrl->intf.output.fd_d.fd_enable) {
    fp_key = FP_PREVIEW_SET;
    rc = mctl_divert_set_key(ctrl, fp_key);
    if (!rc)
      CDBG_ERROR("%s Error setting PREVIEW KEY", __func__);
  }
  fp_ctrl->intf.input.mctl_info.opt_mode = prev_opt_mode;

  ctrlCmd->status = CAM_CTRL_SUCCESS;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_SET_PARM_DIMENSION -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_CAMERA_SET_PARM_DIMENSION(void *parm1, void *parm2)
{
  int rc = TRUE;
  int ret = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  cam_ctrl_dimension_t *dimension = ctrlCmd->value;
  sensor_get_t sensor_get;
  axi_set_t axi_set;

  /* Set the status as SUCCESS if everything is okay. */
  ctrlCmd->status = CAM_CTRL_FAILED;

  /* Save the image formats sent by the app */
  ctrl->video_ctrl.prev_format = dimension->prev_format;
  ctrl->video_ctrl.enc_format = dimension->enc_format;
  ctrl->video_ctrl.main_img_format = dimension->main_img_format;
  ctrl->video_ctrl.thumb_format = dimension->thumb_format;
  ctrl->video_ctrl.rdi0_format = dimension->rdi0_format;
  ctrl->video_ctrl.rdi1_format = dimension->rdi1_format;

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_OUTPUT_CFG, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_HIGH("%s: sensor_get_params failed %d\n", __func__, rc);
    return rc;
  }
  ctrl->sensorCtrl.sensor_output = sensor_get.data.sensor_output;

  sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_PREVIEW;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    return rc;
  }
  ctrl->sensorCtrl.qtr_size_width = sensor_get.data.sensor_dim.width;
  ctrl->sensorCtrl.qtr_size_height = sensor_get.data.sensor_dim.height;

  sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_SNAPSHOT;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    return rc;
  }
  ctrl->sensorCtrl.full_size_width = sensor_get.data.sensor_dim.width;
  ctrl->sensorCtrl.full_size_height = sensor_get.data.sensor_dim.height;

  if(ctrl->comp_ops[MCTL_COMPID_VFE].handle) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle,
               VFE_SET_SENSOR_PARM, NULL, NULL);
    if (rc) {
        CDBG_ERROR("%s: VFE_SET_SENSOR_PARM failed.\n", __func__);
        return -EINVAL;
    }
  }

  /* we only have one AXI now. Need to remove the hard coding */
  axi_set.data.axi_obj_idx = 0;
  axi_set.data.intf_type = AXI_INTF_PIXEL_0;
  axi_set.type = AXI_PARM_PREVIEW_FORMAT;
  axi_set.data.prev_format = ctrl->video_ctrl.prev_format;
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             axi_set.type, (void *)&axi_set, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s AXI SET PARAMS failed for type %d rc = %d ",
               __func__, axi_set.type, rc);
    return rc;
  }
  /* we only have one AXI now. Need to remove the hard coding */
  axi_set.data.axi_obj_idx = 0;
  axi_set.data.intf_type = AXI_INTF_PIXEL_0;
  axi_set.type = AXI_PARM_RECORDING_FORMAT;
  axi_set.data.rec_format = ctrl->video_ctrl.enc_format;
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             axi_set.type, (void *)&axi_set, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s AXI SET PARAMS failed for type %d rc = %d ",
               __func__, axi_set.type, rc);
    return rc;
  }
  /* we only have one AXI now. Need to remove the hard coding */
  axi_set.data.axi_obj_idx = 0;
  axi_set.data.intf_type = AXI_INTF_PIXEL_0;
  axi_set.type = AXI_PARM_SNAPSHOT_FORMAT;
  axi_set.data.snap_format = ctrl->video_ctrl.main_img_format;
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             axi_set.type, (void *)&axi_set, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s AXI SET PARAMS failed for type %d rc = %d ",
               __func__, axi_set.type, rc);
    return rc;
  }
  /* we only have one AXI now. Need to remove the hard coding */
  axi_set.data.axi_obj_idx = 0;
  axi_set.data.intf_type = AXI_INTF_PIXEL_0;
  axi_set.type = AXI_PARM_THUMBNAIL_FORMAT;
  axi_set.data.thumb_format = ctrl->video_ctrl.thumb_format;
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             axi_set.type, (void *)&axi_set, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s AXI SET PARAMS failed for type %d rc = %d ",
               __func__, axi_set.type, rc);
    return rc;
  }
  rc = config_validate_dimension(ctrl, dimension, &ctrl->dimInfo);
  if (rc) {
    CDBG_ERROR("%s: config_validate_dimension failed.\n", __func__);
    return -EINVAL;
  }
  config_calc_dim_offset_app_preview(ctrl, &ctrl->dimInfo);
  config_calc_dim_offset_app_video(ctrl, &ctrl->dimInfo);
  config_calc_dim_offset_app_snapshot(ctrl, &ctrl->dimInfo);
  config_calc_dim_offset_app_thumb(ctrl, &ctrl->dimInfo);

  *(cam_ctrl_dimension_t*)(ctrlCmd->value) = ctrl->dimInfo;
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  CDBG("%s: rc = %d, pic_fmt = %d, thumb_fmt = %d, prev_fmt = %d,"
       " enc_fmt = %d, rotation = %d\n", __func__, rc,
       ctrl->dimInfo.main_img_format, ctrl->dimInfo.thumb_format,
       ctrl->dimInfo.prev_format, ctrl->dimInfo.enc_format,
       ctrl->dimInfo.rotation);
  return TRUE;

}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_CAPABILITIES -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_CAPABILITIES(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  const int num_bits = 32;
  uint32_t value;
  sensor_get_t sensor_get;
  void *faceProc_ptr = NULL;
  mctl_config_ctrl_t *ctrl =
    (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd =
    (struct msm_ctrl_cmd *)parm2;
  cam_prop_t *prop  = (cam_prop_t *)ctrlCmd->value;

  memset(prop, 0, sizeof(cam_prop_t));
  prop->jpeg_capture = 0; /* to be added when SOC sensor support is added*/
  /* open and init sensor when first query capability */
  rc = mctl_init_sensor(ctrl);
  if (rc < 0) {
    CDBG_ERROR("%s: mctl_init_sensor rc = %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_OUTPUT_CFG, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  ctrl->sensorCtrl.sensor_output = sensor_get.data.sensor_output;
  CDBG("config_proc_CAMERA_GET_CAPABILITIES : %d\n", ctrl->sensorCtrl.sensor_output.output_format);

  prop->pxlcode = mm_camera_convert_mbusfmt_to_camfmt(sensor_get.data.pxlcode);
  prop->yuv_output =
        (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR);

  //Temp: Following parameters are currently set to supported
  SET_PARM_BIT32(MM_CAMERA_PARM_CH_IMAGE_FMT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_OP_MODE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_SHARPNESS_CAP, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_SNAPSHOT_BURST_NUM, prop->parm);

  SET_PARM_BIT32(MM_CAMERA_PARM_MAXZOOM, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_LUMA_ADAPTATION, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_HDR, prop->parm);

  SET_PARM_BIT32(MM_CAMERA_PARM_CROP, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_MAX_PICTURE_SIZE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_MAX_PREVIEW_SIZE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_ASD_ENABLE, prop->parm);

  SET_PARM_BIT32(MM_CAMERA_PARM_PICT_SIZE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_ZOOM_RATIO, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_DIMENSION, prop->parm);

  SET_PARM_BIT32(MM_CAMERA_PARM_FOCUS_RECT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_AEC_ROI, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_AF_ROI, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_ZOOM, prop->parm);

  SET_PARM_BIT32(MM_CAMERA_PARM_VIDEO_DIS, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_VIDEO_ROT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_HFR, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_QUERY_FALSH4SNAP, prop->parm);

  SET_PARM_BIT32(MM_CAMERA_PARM_FOCUS_RECT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_AEC_ROI, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_AF_ROI, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_REDEYE_REDUCTION, prop->parm);

  SET_PARM_BIT32(MM_CAMERA_PARM_PREVIEW_FORMAT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_EFFECT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_EXPOSURE_COMPENSATION, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_SHARPNESS, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_CONTRAST, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_WHITE_BALANCE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_ISO, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_SATURATION, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_MAX_NUM_FACES_DECT, prop->parm);

  //parameters not supported by YUV sensor
  if(!prop->yuv_output) {
    SET_PARM_BIT32(MM_CAMERA_PARM_FPS, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_FPS_MODE, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_EXPOSURE, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_BRIGHTNESS, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_AEC_LOCK, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_AWB_LOCK, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_AEC_MTR_AREA, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_ANTIBANDING, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_ROLLOFF, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_CONTINUOUS_AF, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_HJR, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_BL_DETECTION, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_SNOW_DETECTION, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_BESTSHOT_MODE, prop->parm);
    SET_PARM_BIT32(MM_CAMERA_PARM_WAVELET_DENOISE, prop->parm);
  }
  if (ctrl->cam_sensor_info.flash_enabled)
    SET_PARM_BIT32(MM_CAMERA_PARM_LED_MODE, prop->parm);

  SET_PARM_BIT32(MM_CAMERA_PARM_LIVESHOT_MAIN, prop->parm);
//Parameters not supported by VFE 2X
#ifndef VFE_2X
  SET_PARM_BIT32(MM_CAMERA_PARM_FPS, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_FOCUS_RECT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_MCE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_SCE_FACTOR, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_HISTOGRAM, prop->parm);
#endif

#ifdef VFE_2X
  SET_PARM_BIT32(CAMERA_PARM_HFR_SKIP, prop->parm);
#endif

#ifdef MM_CAMERA_FD
  faceProc_ptr = dlopen("libmmcamera_faceproc.so", RTLD_NOW);
  if (!faceProc_ptr) {
    CDBG_HIGH("%s libmmcamera_faceproc.so lib doesn't exist", __func__);
  } else {
    SET_PARM_BIT32(MM_CAMERA_PARM_FD, prop->parm);
    dlclose(faceProc_ptr);
  }
#endif
#ifdef MM_STEREO_CAM
  SET_PARM_BIT32(MM_CAMERA_PARM_3D_FRAME_FORMAT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_3D_EFFECT, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_3D_DISPLAY_DISTANCE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_3D_VIEW_ANGLE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_3D_MANUAL_CONV_RANGE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_3D_MANUAL_CONV_VALUE, prop->parm);
  SET_PARM_BIT32(MM_CAMERA_PARM_ENABLE_3D_MANUAL_CONVERGENCE, prop->parm);
#endif
  SET_PARM_BIT32(MM_CAMERA_PARM_RAW_SNAPSHOT_FMT, prop->parm);

  prop->ops[0] |= (1<<CAMERA_OPS_STREAMING_PREVIEW);
  prop->ops[0] |= (1<<CAMERA_OPS_STREAMING_ZSL);
  prop->ops[0] |= (1<<CAMERA_OPS_STREAMING_VIDEO);
  /* prop->ops[0] |= (1<<CAMERA_OPS_CAPTURE); */
  CDBG("%s: yuv_output = %d, af_enable = %d\n", __func__, prop->yuv_output, ctrl->afCtrl.af_enable);
  if (!prop->yuv_output && ctrl->afCtrl.af_enable) {
    prop->ops[0] |= (1<<CAMERA_OPS_FOCUS);
  }
  prop->ops[0] |= (1<<CAMERA_OPS_GET_PICTURE);
  if (!prop->yuv_output)
    prop->ops[0] |= (1<<CAMERA_OPS_PREPARE_SNAPSHOT);
  prop->ops[0] |= (1<<CAMERA_OPS_SNAPSHOT);
  prop->ops[0] |= (1<<CAMERA_OPS_LIVESHOT);
  prop->ops[0] |= (1<<CAMERA_OPS_RAW_SNAPSHOT);
  prop->ops[0] |= (1<<CAMERA_OPS_VIDEO_RECORDING);
  prop->ops[0] |= (1<<CAMERA_OPS_REGISTER_BUFFER);
  prop->ops[0] |= (1<<CAMERA_OPS_UNREGISTER_BUFFER);

  prop->effect = 0x1;
  if (!prop->yuv_output)
    prop->effect = 0xffffffff;
//  else if (ctrl->sensorCtrl.fn_table.sensor_get_effects_supported) {
//    ctrl->sensorCtrl.fn_table.sensor_get_effects_supported(&(prop->effect));
//  }
  prop->modes = CAMERA_MODE_2D;
  /* is 3d mode is supported */
  prop->modes |= CAMERA_MODE_3D;

  sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_PREVIEW;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
	     ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	     SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  prop->max_preview_width = sensor_get.data.sensor_dim.width;
  prop->max_preview_height = sensor_get.data.sensor_dim.height;

  sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_SNAPSHOT;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
		 ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
		 SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  prop->max_pict_width = sensor_get.data.sensor_dim.width;
  prop->max_pict_height = sensor_get.data.sensor_dim.height;
  prop->preview_format = CAMERA_YUV_420_NV21;

  sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_VIDEO;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
		 ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
		 SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  prop->max_video_width = sensor_get.data.sensor_dim.width;
  prop->max_video_height = sensor_get.data.sensor_dim.height;

  prop->preview_sizes_cnt = sizeof (default_preview_size) / sizeof (default_preview_size[0]);
  prop->thumb_sizes_cnt = sizeof (jpeg_thumbnail_sizes) / sizeof (jpeg_thumbnail_sizes[0]);
  prop->video_sizes_cnt = sizeof (supported_video_sizes) / sizeof (supported_video_sizes[0]);
  prop->hfr_sizes_cnt = sizeof (hfr_sizes) / sizeof (hfr_sizes[0]);
#ifndef VFE_2X
  prop->vfe_output_enable = 2;
  prop->hfr_frame_skip = 0;
  prop->default_preview_width = DEFAULT_PREVIEW_WIDTH;
  prop->default_preview_height = DEFAULT_PREVIEW_HEIGHT;
  prop->bestshot_reconfigure = 0;
#else
  prop->vfe_output_enable = 1;
  prop->hfr_frame_skip = 1;
  if(prop->max_pict_width < DEFAULT_PREVIEW_WIDTH || prop->max_pict_height < DEFAULT_PREVIEW_HEIGHT) {
    prop->default_preview_width = prop->max_pict_width;
    prop->default_preview_height = prop->max_pict_height;
  }
  else {
    prop->default_preview_width = DEFAULT_PREVIEW_WIDTH;
    prop->default_preview_height = DEFAULT_PREVIEW_HEIGHT;
  }
  prop->bestshot_reconfigure = 1;
#endif

  ctrlCmd->status =  CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_DIMENSION -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_DIMENSION(void *parm1,
  void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  *(cam_ctrl_dimension_t *)(ctrlCmd->value) = ctrl->dimInfo;
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_FPS_RANGE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_FPS_RANGE(void *parm1,
  void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  cam_sensor_fps_range_t *fps_range = (cam_sensor_fps_range_t *)(ctrlCmd->value);
  sensor_get_t sensor_get;
  int8_t rc = TRUE;

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
    SENSOR_GET_PREVIEW_FPS_RANGE, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    fps_range->min_fps = 0;
    fps_range->max_fps = 0;
    return rc;
  }
  fps_range->min_fps = sensor_get.data.fps_range.min_fps;
  fps_range->max_fps = sensor_get.data.fps_range.max_fps;
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_CHANNEL_STREAM -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_GET_CHANNEL_STREAM(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  (*(uint32_t *)cmd->value) =
      ctrl->channel_stream_info;
  CDBG_HIGH("%s Channel stream info = %d ", __func__,
            (*(uint32_t *)cmd->value));
  cmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_FOCUS_DISTANCE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_GET_PARM_FOCUS_DISTANCES(void *parm1, void *parm2)
{
  int8_t rc = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  focus_distances_info_t *focusDistanceInfo =
    (focus_distances_info_t *)ctrlCmd->value;

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
    stats_proc_get_t stats_get;
    stats_get.type = STATS_PROC_AF_TYPE;
    stats_get.d.get_af.type = AF_FOCUS_DISTANCES;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].get_params(
         ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, stats_get.type,
         &stats_get, sizeof(stats_get));

    focusDistanceInfo->focus_distance[FOCUS_DISTANCE_NEAR_INDEX] =
      stats_get.d.get_af.d.af_focus_distance.focus_distance[FOCUS_DISTANCE_NEAR_INDEX];
    focusDistanceInfo->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] =
      stats_get.d.get_af.d.af_focus_distance.focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX];
    focusDistanceInfo->focus_distance[FOCUS_DISTANCE_FAR_INDEX] =
      stats_get.d.get_af.d.af_focus_distance.focus_distance[FOCUS_DISTANCE_FAR_INDEX];

    CDBG("%s: NF=%f OF=%f FF=%f \n",
      __func__,
      focusDistanceInfo->focus_distance[FOCUS_DISTANCE_NEAR_INDEX],
      focusDistanceInfo->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX],
      focusDistanceInfo->focus_distance[FOCUS_DISTANCE_FAR_INDEX]);
  }

  ctrlCmd->status = CAM_CTRL_SUCCESS;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_FOCAL_LENGTH -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_GET_PARM_FOCAL_LENGTH(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  (*(float *)cmd->value) =
      ctrl->sensorCtrl.lens_info.focal_length;
  CDBG_HIGH("%s Focal length of the sensor = %f ", __func__,
            (*(float *)cmd->value));
  cmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_HORIZONTAL_VIEW_ANGLE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_GET_PARM_HORIZONTAL_VIEW_ANGLE(void *parm1,
                                                         void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  (*(float *)cmd->value) =
      ctrl->sensorCtrl.lens_info.hor_view_angle;
  CDBG_HIGH("%s Horizontal view angle of the sensor = %f ", __func__,
            (*(float *)cmd->value));
  cmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_VERTICAL_VIEW_ANGLE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_proc_CAMERA_GET_PARM_VERTICAL_VIEW_ANGLE(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *cmd = (struct msm_ctrl_cmd *)parm2;
  (*(float *)cmd->value) =
      ctrl->sensorCtrl.lens_info.ver_view_angle;
  CDBG_HIGH("%s Vertical view angle of the sensor = %f ", __func__,
            (*(float *)cmd->value));
  cmd->status = CAM_CTRL_SUCCESS;
  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_MAX_HFR_MODE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_MAX_HFR_MODE(void *parm1,
  void *parm2)
{
  int rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  sensor_get_t sensor_get;

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
       ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
       SENSOR_GET_MAX_SUPPORTED_HFR_MODE,
       &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }

  *(camera_hfr_mode_t *)(ctrlCmd->value) =
    sensor_get.data.max_supported_hfr_mode;
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_PARM_MAX_LIVESHOT_SIZE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_PARM_MAX_LIVESHOT_SIZE(void *parm1,
  void *parm2)
{
  int rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  sensor_get_t sensor_get;
  struct camera_size_type *cam_size = (struct camera_size_type *)ctrlCmd->value;

  sensor_get.type = SENSOR_GET_DIM_INFO;
  sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_VIDEO;

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  cam_size->width = sensor_get.data.sensor_dim.width;
  cam_size->height = sensor_get.data.sensor_dim.height;
  CDBG("%s Max supported liveshot dimension at max fps mode = %d x %d",
       __func__, cam_size->width, cam_size->height);
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_SET_PARM_ZOOM -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_CAMERA_SET_PARM_ZOOM(void *parm1,
  void *parm2)
{
  int rc = 0;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  int32_t zoom_val;

  zoom_val= *((int32_t *)ctrlCmd->value);
  if((rc = config_proc_zoom(ctrl, zoom_val)) == 0) {
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  } else {
    ctrlCmd->status = CAM_CTRL_FAILED;
  }
  CDBG("%s: X", __func__);
  return rc;
}
/*===========================================================================
 * FUNCTION    - config_CAMERA_SET_BUNDLE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int8_t config_CAMERA_SET_BUNDLE(void *parm1,
  void *parm2)
{
  int i, idx, rc = TRUE;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  cam_stream_bundle_t *bundle;
  uint32_t image_mode;

  CDBG("%s: E", __func__);
  bundle = (cam_stream_bundle_t *)ctrlCmd->value;
  ctrl->video_ctrl.user_bundle_mask = 0;
  for (i = 0; i < bundle->num; i++) {
    for (idx = 0; idx < MSM_MAX_DEV_INST; idx++) {
      if (bundle->stream_handles[i] ==
          ctrl->video_ctrl.strm_info.user[idx].inst_handle) {
        image_mode= ctrl->video_ctrl.strm_info.user[idx].image_mode;
        CDBG_HIGH("%s: Adding Stream %d into bundle", __func__, image_mode);
        ctrl->video_ctrl.user_bundle_mask |=
          config_proc_get_stream_bit(image_mode);
        break;
      }
    }
  }
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  CDBG("%s: X, user_bundle_mask = 0x%x",
       __func__, ctrl->video_ctrl.user_bundle_mask);
  return rc;
}
/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_ENABLE_MOBICAT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_ENABLE_MOBICAT(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  const int max_mobicat_size = MAX_SERVER_PAYLOAD_LENGTH;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  mm_cam_mobicat_info_t info = *(mm_cam_mobicat_info_t *)ctrlCmd->value;
  stats_proc_set_t set_param;
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  if (info.enable == ctrl->mobicat_info.enable) {
    CDBG_ERROR("%s: Mobicat already enabled! returning!!", __func__);
    ctrlCmd->status = CAM_CTRL_SUCCESS;
    return TRUE;
  }
  ctrl->mobicat_info.enable = info.enable;

  /* Enable Mobicat in VFE */
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_MOBICAT,
    &info.enable, NULL);
  CDBG("%s: vfe_set_params returned: %d", __func__, rc);
  if (rc != 0) {
    ctrlCmd->status = CAM_CTRL_FAILED;
    return TRUE;
  }

  if (ctrl->comp_mask & (1 << MCTL_COMPID_STATSPROC)) {
    /* Enable Mobicat in Statsproc */
    set_param.type = STATS_PROC_MOBICAT_TYPE;
    set_param.d.set_mobicat.type = MOBICAT_ENABLE;
    set_param.d.set_mobicat.d.mobicat_enable = info.enable;

    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type,
      &set_param, &(sp_ctrl->intf));
  }

  info.mobicat_size = max_mobicat_size;
  CDBG("%s: Mobicat size %d", __func__, info.mobicat_size);
  ctrlCmd->status = (rc < 0) ? CAM_CTRL_FAILED : CAM_CTRL_SUCCESS;

  return TRUE;
}/*config_proc_CAMERA_ENABLE_MOBICAT*/

/*===========================================================================
 * FUNCTION    - config_proc_CAMERA_GET_MOBICAT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_proc_CAMERA_GET_MOBICAT(void *parm1, void *parm2)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  QCameraInfo_t mobicat_info;
  QISPInfo_t *p_isp_info = &mobicat_info.isp_info;
  Q3AInfo_t *p_3a_info = &mobicat_info.stats_proc_info;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  cam_exif_tags_t *p_metadata = (cam_exif_tags_t *)(ctrlCmd->value);
  stats_proc_get_t get_param;
  int8_t rc = TRUE;

  /* get the values from 3A and ISP */
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_MOBICAT_ISP_INFO,
    (void *)p_isp_info, sizeof(QISPInfo_t));
  if (rc != 0) {
    CDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    ctrlCmd->status = CAM_CTRL_FAILED;
    return TRUE;
  }

  if (ctrl->comp_mask & (1 << MCTL_COMPID_STATSPROC)) {
    get_param.type = STATS_PROC_MOBICAT_TYPE;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].get_params(
           ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, get_param.type,
           &get_param, sizeof(get_param));
    if (rc != 0) {
      CDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      ctrlCmd->status = CAM_CTRL_FAILED;
      return TRUE;
    }
    memcpy((Q3AInfo_t *)p_3a_info, &(get_param.d.get_mobicat.stats_proc_info),
      sizeof(Q3AInfo_t));
  }

  /* flatten the parameters into the string */
  rc = parse_mobicat_info(&mobicat_info, p_metadata);

  ctrlCmd->status = (rc < 0) ? CAM_CTRL_FAILED : CAM_CTRL_SUCCESS;

  return TRUE;
}/*config_proc_CAMERA_GET_MOBICAT*/

/*===========================================================================
 * FUNCTION    - config_proc_native_ctrl_cmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_proc_native_ctrl_cmd(void *parm1, void *parm2, int *cmdPending)
{
  int rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *v4l2_ctrlCmd  = (struct msm_ctrl_cmd *)parm2;
  struct msm_ctrl_cmd *ctrlCmd;
  uint8_t *data_ptr = (uint8_t *)v4l2_ctrlCmd->value;
  ctrlCmd = (struct msm_ctrl_cmd *)data_ptr;
  if (ctrlCmd->length)
    ctrlCmd->value = (void *)(data_ptr + sizeof(struct msm_ctrl_cmd));
  else ctrlCmd->value = NULL;
  ctrlCmd->status = 0;
  ctrlCmd->timeout_ms = v4l2_ctrlCmd->timeout_ms;
  ctrlCmd->resp_fd = v4l2_ctrlCmd->resp_fd;;
  ctrlCmd->vnode_id = v4l2_ctrlCmd->vnode_id;;
  ctrlCmd->stream_type = v4l2_ctrlCmd->stream_type;
  CDBG("%s: type =%d len %d", __func__, ctrlCmd->type, ctrlCmd->length);

  switch (ctrlCmd->type) {
    case CAMERA_SET_PARM_DIMENSION: {
      cam_ctrl_dimension_t *dim = ctrlCmd->value;
      CDBG_ERROR("%s: CAMERA_SET_PARM_DIMENSION called\n", __func__);
      /* we actual need to have a state machine in final clean up.
         Now we just simpley check the mctl state */
      if(ctrl->mctl_state != MCTL_CTRL_STATE_SERVICE_READY) {
        if (NULL == dim) {
          v4l2_ctrlCmd->status = CAM_CTRL_FAILED;
          return FALSE;
        }
      }
      rc = config_CAMERA_SET_PARM_DIMENSION(ctrl, ctrlCmd);
      break;
    }

    case CAMERA_GET_CAPABILITIES:
      rc = config_proc_CAMERA_GET_CAPABILITIES(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_MAXZOOM:
      rc = config_proc_CAMERA_GET_PARM_MAXZOOM(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_AUTO_FOCUS:
      rc = config_proc_CAMERA_SET_PARM_AUTO_FOCUS(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_FD:
      rc = config_proc_CAMERA_SET_PARM_FD(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_ZOOM:
      rc = config_CAMERA_SET_PARM_ZOOM(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_EXPOSURE_COMPENSATION:
      rc = config_proc_CAMERA_SET_PARM_EXPOSURE_COMPENSATION(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_SATURATION:
      rc = config_proc_CAMERA_SET_PARM_SATURATION(ctrl, ctrlCmd);
      break;
    case CAMERA_SET_PARM_EXPOSURE:
      rc = config_proc_CAMERA_SET_PARM_EXPOSURE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_FPS:
      rc = config_proc_CAMERA_SET_PARM_FPS(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_ISO:
      rc = config_proc_CAMERA_SET_PARM_ISO(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_BESTSHOT_MODE:
      rc = config_proc_CAMERA_SET_PARM_BESTSHOT_MODE(ctrl, ctrlCmd);
      break;
#if 0
    case CAMERA_SET_PARM_PREVIEW_FPS:
      rc = config_proc_ctrlcmd(CAMERA_SET_PARM_PREVIEW_FPS, ctrl, ctrlCmd);
      break;
#endif
    case CAMERA_SET_PARM_AF_MODE:
      rc = config_proc_CAMERA_SET_PARM_AF_MODE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_FOCUS_RECT:
      rc = config_proc_CAMERA_SET_PARM_FOCUS_RECT(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_HISTOGRAM:
      rc = config_proc_CAMERA_SET_PARM_HISTOGRAM(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_LED_MODE:
      rc = config_proc_CAMERA_SET_PARM_LED_MODE(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_FACIAL_FEATURE_INFO:
      rc = config_proc_CAMERA_GET_FD_FEATURE_INFO(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_ZOOMRATIOS:
      rc = config_proc_CAMERA_GET_PARM_ZOOMRATIOS(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_MOTION_ISO:
      rc = config_proc_CAMERA_SET_MOTION_ISO(ctrl, ctrlCmd);
      break;

    case CAMERA_AUTO_FOCUS_CANCEL:
      rc = config_proc_CAMERA_AUTO_FOCUS_CANCEL(ctrl, ctrlCmd);
      break;

    case CAMERA_PREPARE_SNAPSHOT:
      rc = config_proc_CAMERA_PREPARE_SNAPSHOT(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_FPS_MODE:
      rc = config_proc_CAMERA_SET_FPS_MODE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_AEC_MTR_AREA:
      rc = config_proc_CAMERA_SET_AEC_MTR_AREA(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_AEC_ROI:
      rc = config_proc_CAMERA_SET_PARM_AEC_ROI(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_AEC_LOCK:
      rc = config_proc_CAMERA_SET_AEC_LOCK(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_CAF:
      rc = config_proc_CAMERA_SET_CAF(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_AWB_LOCK:
      rc = config_proc_CAMERA_SET_PARM_AWB_LOCK(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_ASD_ENABLE:
      rc = config_proc_CAMERA_SET_ASD_ENABLE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_AF_ROI:
      rc = config_proc_CAMERA_SET_PARM_AF_ROI(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_RECORDING_HINT:
      rc = config_proc_CAMERA_SET_RECORDING_HINT(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_FULL_LIVESHOT:
      /* NOP. To be removed after switching to new HAL intf */
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      rc = TRUE;
      break;

    case CAMERA_SET_CHANNEL_STREAM:
      rc = config_proc_CAMERA_SET_CHANNEL_STREAM(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_LOW_POWER_MODE:
      rc = config_proc_CAMERA_SET_LOW_POWER_MODE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_DIS_ENABLE:
      rc = config_proc_CAMERA_SET_DIS_ENABLE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_SCE_FACTOR:
      rc = config_proc_CAMERA_SET_SCE_FACTOR(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_DIMENSION:
      rc = config_proc_CAMERA_GET_PARM_DIMENSION(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_FPS_RANGE:
      rc = config_proc_CAMERA_GET_PARM_FPS_RANGE(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_FOCUS_DISTANCES:
      rc = config_proc_CAMERA_GET_PARM_FOCUS_DISTANCES(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_FOCAL_LENGTH:
      rc = config_proc_CAMERA_GET_PARM_FOCAL_LENGTH(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_CHANNEL_STREAM:
      rc = config_proc_CAMERA_GET_CHANNEL_STREAM(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_HORIZONTAL_VIEW_ANGLE:
      rc = config_proc_CAMERA_GET_PARM_HORIZONTAL_VIEW_ANGLE(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_VERTICAL_VIEW_ANGLE:
      rc = config_proc_CAMERA_GET_PARM_VERTICAL_VIEW_ANGLE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_WAVELET_DENOISE:
      rc = config_proc_CAMERA_SET_PARM_WAVELET_DENOISE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_MCE:
      rc = config_proc_CAMERA_SET_PARM_MCE(ctrl, ctrlCmd);
      break;
    case CAMERA_SET_PARM_HFR:
      rc = config_proc_CAMERA_SET_PARM_HFR(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_REDEYE_REDUCTION:
      rc = config_proc_CAMERA_SET_REDEYE_REDUCTION(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_HDR:
      rc = config_proc_CAMERA_SET_PARM_HDR(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_MAX_HFR_MODE:
      rc = config_proc_CAMERA_GET_PARM_MAX_HFR_MODE(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_DEF_PREVIEW_SIZES:
      rc = config_proc_CAMERA_GET_PARM_DEF_PREVIEW_SIZES(ctrl,ctrlCmd);
      break;

    case CAMERA_GET_PARM_DEF_VIDEO_SIZES:
      rc = config_proc_CAMERA_GET_PARM_DEF_VIDEO_SIZES(ctrl,ctrlCmd);
      break;

    case CAMERA_GET_PARM_DEF_THUMB_SIZES:
      rc = config_proc_CAMERA_GET_PARM_DEF_THUMB_SIZES(ctrl,ctrlCmd);
      break;

    case CAMERA_GET_PARM_DEF_HFR_SIZES:
      rc = config_proc_CAMERA_GET_PARM_DEF_HFR_SIZES(ctrl,ctrlCmd);
      break;

    case CAMERA_GET_PARM_MAX_LIVESHOT_SIZE:
      rc = config_proc_CAMERA_GET_PARM_MAX_LIVESHOT_SIZE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_3A_CONVERGENCE:
      rc = config_CAMERA_SET_3A_CONVERGENCE(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PREVIEW_HFR:
      rc = config_proc_CAMERA_SET_PREVIEW_HFR(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_MAX_DIMENSION:
      rc = config_proc_CAMERA_GET_MAX_DIMENSION(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_MAX_NUM_FACES_DECT:
      rc = config_proc_CAMERA_GET_MAX_NUM_FACES_DECT(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_PARM_CID:
      rc = config_CAMERA_SET_PARM_CID(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_FRAME_RESOLUTION:
      rc = config_CAMERA_GET_PARM_FRAME_RESOLUTION(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PP_MASK:
      rc = config_CAMERA_GET_PP_MASK(ctrl, ctrlCmd);
      break;

    case CAMERA_DO_PP_WNR:
      rc = config_CAMERA_DO_PP_WNR(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_HDR:
      rc = config_proc_CAMERA_GET_PARM_HDR(ctrl, ctrlCmd);
      break;

    case CAMERA_SET_BUNDLE:
      rc = config_CAMERA_SET_BUNDLE(ctrl, ctrlCmd);
      break;

    case CAMERA_ENABLE_MOBICAT:
      rc = config_proc_CAMERA_ENABLE_MOBICAT(ctrl, ctrlCmd);
      break;

    case CAMERA_GET_PARM_MOBICAT:
      rc = config_proc_CAMERA_GET_MOBICAT(ctrl, ctrlCmd);
      break;

    default:
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;
  }
  v4l2_ctrlCmd->status = ctrlCmd->status;
  if (rc)
    *cmdPending = FALSE;
  else
    *cmdPending = TRUE;
  return rc;
}/*config_proc_native_ctrl_cmd*/


/*===========================================================================
 * FUNCTION    - config_proc_set_ctrl_cmd -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_proc_set_ctrl_cmd(void *parm1, void *parm2, int *cmdPending)
{
  int ret = FALSE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  struct v4l2_control *v4l2_ctrl = (struct v4l2_control *)(ctrlCmd->value);
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);

  CDBG("mctl_proc_set_ctrl_cmd: id = %d \n", v4l2_ctrl->id);
  switch (v4l2_ctrl->id) {
    case MSM_V4L2_PID_CAM_MODE: {
        ctrl->video_ctrl.op_mode = v4l2_ctrl->value;
        if(fp_ctrl->intf.output.share_d.divert_snapshot) {
          if(ctrl->video_ctrl.op_mode == MSM_V4L2_CAM_OP_ZSL) {
            frame_proc_key_t fp_key;
            fp_key = FP_SNAPSHOT_RESET;
            ret = mctl_divert_set_key(ctrl, fp_key);
          } else if (ctrl->denoise_enable) {
            frame_proc_key_t fp_key;
            fp_key = FP_SNAPSHOT_SET;
            ret = mctl_divert_set_key(ctrl, fp_key);
          }
        }
        ctrlCmd->status = CAM_CTRL_SUCCESS;
        break;
      }
    case V4L2_CID_CONTRAST:
      {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_CONTRAST(ctrl, ctrlCmd);
        if (ctrlCmd->status == CAM_CTRL_SUCCESS)
          ctrl->v4l2Ctrl.contrast = v4l2_ctrl->value;
      }
      break;

    case V4L2_CID_SATURATION: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_SATURATION(ctrl, ctrlCmd);
        if (ctrlCmd->status == CAM_CTRL_SUCCESS)
          ctrl->v4l2Ctrl.saturation = v4l2_ctrl->value;
      }
      break;

    case V4L2_CID_HUE: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_HUE(ctrl, ctrlCmd);
      }
      break;

    case MSM_V4L2_PID_EFFECT: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_EFFECT(ctrl, ctrlCmd);
      }
      break;

    case V4L2_CID_BRIGHTNESS: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_BRIGHTNESS(ctrl, ctrlCmd);
        if (ctrlCmd->status == CAM_CTRL_SUCCESS)
          ctrl->v4l2Ctrl.brightness = v4l2_ctrl->value;
      }
      break;

    case V4L2_CID_SHARPNESS: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_SHARPNESS(ctrl, ctrlCmd);
      }
      break;

    case MSM_V4L2_PID_BEST_SHOT: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_BESTSHOT_MODE(ctrl, ctrlCmd);
      }
      break;

    case V4L2_CID_AUTO_WHITE_BALANCE: {
        if (v4l2_ctrl->value)
          *(int *)ctrlCmd->value = CAMERA_WB_AUTO;
        else
          *(int *)ctrlCmd->value = CAMERA_WB_OFF;
        ret = config_proc_CAMERA_SET_PARM_WB(ctrl, ctrlCmd);
        if (ctrlCmd->status == CAM_CTRL_SUCCESS)
          ctrl->v4l2Ctrl.is_auto_wb = v4l2_ctrl->value;
      }
      break;
    case V4L2_CID_GAIN: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_ISO(ctrl, ctrlCmd);
      }
      break;

    case V4L2_CID_WHITE_BALANCE_TEMPERATURE: {
        if (v4l2_ctrl->value < 3300)
          *(int *)ctrlCmd->value = CAMERA_WB_INCANDESCENT;
        else if (v4l2_ctrl->value < 5500)
          *(int *)ctrlCmd->value = CAMERA_WB_FLUORESCENT;
        else if (v4l2_ctrl->value == 7500)
          *(int *)ctrlCmd->value = CAMERA_WB_CLOUDY_DAYLIGHT;
        else
          *(int *)ctrlCmd->value = CAMERA_WB_DAYLIGHT;
        ret = config_proc_CAMERA_SET_PARM_WB(ctrl, ctrlCmd);
        if (ctrlCmd->status == CAM_CTRL_SUCCESS)
          ctrl->v4l2Ctrl.wb_temperature = v4l2_ctrl->value;
      }
      break;
    case V4L2_CID_ZOOM_ABSOLUTE:
      *(int*)ctrlCmd->value = v4l2_ctrl->value;
      ret = config_CAMERA_SET_PARM_ZOOM(ctrl, ctrlCmd);
      break;

    case MSM_V4L2_PID_MOTION_ISO: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_MOTION_ISO(ctrl, ctrlCmd);
      }
      break;
    case V4L2_CID_POWER_LINE_FREQUENCY: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        if (CAMERA_ANTIBANDING_AUTO == v4l2_ctrl->value ||
          CAMERA_ANTIBANDING_AUTO_50HZ == v4l2_ctrl->value ||
          CAMERA_ANTIBANDING_AUTO_60HZ == v4l2_ctrl->value)
          ret = config_proc_CAMERA_ENABLE_AFD(ctrl, ctrlCmd);
        else
          ret = config_proc_CAMERA_SET_PARM_ANTIBANDING(ctrl,ctrlCmd);
      }
      break;
    case V4L2_CID_EXPOSURE: {
        ctrl->v4l2Ctrl.ev_num = v4l2_ctrl->value;
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_EXPOSURE_COMPENSATION(ctrl, ctrlCmd);
      }
      break;

    case V4L2_CID_AUTOGAIN: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_EXPOSURE(ctrl, ctrlCmd);
      }
      break;

    case MSM_V4L2_PID_EXP_METERING: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_EXPOSURE(ctrl, ctrlCmd);
      }
      break;

    case MSM_V4L2_PID_ISO: {
        *(int *)ctrlCmd->value = v4l2_ctrl->value;
        ret = config_proc_CAMERA_SET_PARM_ISO(ctrl, ctrlCmd);
      }
      break;

    case V4L2_CID_FOCUS_AUTO:
      /*set continuous auto focus flag to true*/
      *(int *)ctrlCmd->value = v4l2_ctrl->value;
      ret = config_proc_CAMERA_SET_CAF(ctrl, ctrlCmd);
      ctrlCmd->status = ret;;
      break;

    default: {
        ret = TRUE;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      }
  }
  *cmdPending = FALSE;
  return ret;
}/*config_proc_set_ctrl_cmd*/

int config_proc_private_ctrl_cmd(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  int status = CAM_CTRL_SUCCESS;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  camera_plugin_mctl_process_ops_t *ops = &ctrl->mctl_ops_for_plugin;
  camera_plugin_client_ops_t *plugin_client_ops = &ctrl->plugin_client_ops;
  camera_plugin_ops_t *camera_plugin_ops = ctrl->camera_plugin_ops;
  camera_plugin_ioctl_data_t temp_plugin_data;

  temp_plugin_data.type = ctrlCmd->status; /* borrowed the status field */
  temp_plugin_data.len = ctrlCmd->length;
  temp_plugin_data.data = ctrlCmd->value;
  rc = plugin_client_ops->private_ioctl_to_plugin(camera_plugin_ops->handle,
                                       plugin_client_ops->client_handle,
                                       &temp_plugin_data, &status);
  if (rc < 0)
    CDBG_ERROR("%s: Private ioctl failed in plugin\n", __func__);
  ctrlCmd->status = status;
  *cmdPending = FALSE;
  return rc;
}

uint32_t config_proc_get_stream_bit(uint32_t image_mode)
{
  switch (image_mode) {
    case MSM_V4L2_EXT_CAPTURE_MODE_DEFAULT:
    case MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW:
      return V4L2_DEV_STRAEMON_BIT_P;
    case MSM_V4L2_EXT_CAPTURE_MODE_VIDEO:
      return V4L2_DEV_STRAEMON_BIT_V;
    case MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL:
      return V4L2_DEV_STRAEMON_BIT_T;
    case MSM_V4L2_EXT_CAPTURE_MODE_MAIN:
      return V4L2_DEV_STRAEMON_BIT_S;
    case MSM_V4L2_EXT_CAPTURE_MODE_ISP_PIX_OUTPUT1:
      return V4L2_DEV_STREAMON_BIT_ISP_OUT1;
    case MSM_V4L2_EXT_CAPTURE_MODE_ISP_PIX_OUTPUT2:
      return V4L2_DEV_STREAMON_BIT_ISP_OUT2;
    case MSM_V4L2_EXT_CAPTURE_MODE_RAW:
      return V4L2_DEV_STREAMON_BIT_R;
    case MSM_V4L2_EXT_CAPTURE_MODE_RDI:
      return V4L2_DEV_STREAMON_BIT_RDI;
    case MSM_V4L2_EXT_CAPTURE_MODE_RDI1:
      return V4L2_DEV_STREAMON_BIT_RDI1;
    case MSM_V4L2_EXT_CAPTURE_MODE_RDI2:
      return V4L2_DEV_STREAMON_BIT_RDI2;
    default:
      return 0;
  }
  return 0;
}

