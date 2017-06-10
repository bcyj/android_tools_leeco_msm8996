/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential                         .
============================================================================*/
#include <sys/types.h>
#include <assert.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#ifdef _ANDROID_
#include <utils/Log.h>
#endif
#include <inttypes.h>
#include <media/msm_isp.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "camera.h"
#include "mctl.h"
#include "mctl_af.h"
#include "mctl_ez.h"
#include "config_proc.h"
#include "camera_dbg.h"
#include "cam_mmap.h"

/*===========================================================================
 * FUNCTION    - mctl_proc_MSM_V4L2_QUERY_CTRL -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int mctl_proc_MSM_V4L2_QUERY_CTRL(void *parm1, void *parm2, int *cmdPending)
{
  int ret = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  struct v4l2_queryctrl *v4l2_ctrl = (struct v4l2_queryctrl *)(ctrlCmd->value);
  __u32 ctrl_id = v4l2_ctrl->id;

  *cmdPending = FALSE;

  if (v4l2_ctrl->id & V4L2_CTRL_FLAG_NEXT_CTRL) {
    v4l2_ctrl->id = (v4l2_ctrl->id & 0x7FFFFFFF);

    if (v4l2_ctrl->id < V4L2_CID_BRIGHTNESS)
      v4l2_ctrl->id = V4L2_CID_BRIGHTNESS;
    else if (v4l2_ctrl->id < V4L2_CID_CONTRAST)
      v4l2_ctrl->id = V4L2_CID_CONTRAST;
    else if (v4l2_ctrl->id < V4L2_CID_SATURATION)
      v4l2_ctrl->id = V4L2_CID_SATURATION;
    else if (v4l2_ctrl->id < V4L2_CID_HUE)
      v4l2_ctrl->id = V4L2_CID_HUE;
    else if (v4l2_ctrl->id < V4L2_CID_EXPOSURE)
      v4l2_ctrl->id = V4L2_CID_EXPOSURE;
    else if (v4l2_ctrl->id < V4L2_CID_EXPOSURE_AUTO)
      v4l2_ctrl->id = V4L2_CID_EXPOSURE_AUTO;
    else if (v4l2_ctrl->id < V4L2_CID_EXPOSURE_AUTO_PRIORITY)
      v4l2_ctrl->id = V4L2_CID_EXPOSURE_AUTO_PRIORITY;
    else if (v4l2_ctrl->id < V4L2_CID_FOCUS_AUTO)
      v4l2_ctrl->id = V4L2_CID_FOCUS_AUTO;
    else if (v4l2_ctrl->id < V4L2_CID_ZOOM_ABSOLUTE)
      v4l2_ctrl->id = V4L2_CID_ZOOM_ABSOLUTE;
    else if (v4l2_ctrl->id < MSM_V4L2_PID_ISO)
      v4l2_ctrl->id = MSM_V4L2_PID_ISO;
    else
      ret = -1;
  }

  if (ret < 0) {
    ctrlCmd->status = CAM_CTRL_INVALID_PARM;
    return ret;
  }

  if (v4l2_ctrl->id > MSM_V4L2_PID_MAX) {
    ctrlCmd->status = CAM_CTRL_INVALID_PARM;
    return -1;
  }

  CDBG("mctl_proc_MSM_V4L2_QUERY_CTRL: id = %d\n",
    v4l2_ctrl->id);
  switch (v4l2_ctrl->id) {
    case V4L2_CID_BRIGHTNESS:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
      strlcpy((char *)v4l2_ctrl->name, "Brightness", strlen("Brightness") + 1);
      v4l2_ctrl->minimum = CAMERA_MIN_BRIGHTNESS;
      v4l2_ctrl->maximum = CAMERA_MAX_BRIGHTNESS;
      v4l2_ctrl->step = CAMERA_BRIGHTNESS_STEP;
      v4l2_ctrl->default_value = CAMERA_DEF_BRIGHTNESS;
      v4l2_ctrl->flags = 0;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_CONTRAST:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
      strlcpy((char *)v4l2_ctrl->name, "Contrast", strlen("Contrast") + 1);
      v4l2_ctrl->minimum = CAMERA_MIN_CONTRAST;
      v4l2_ctrl->maximum = CAMERA_MAX_CONTRAST;
      v4l2_ctrl->step = CAMERA_CONTRAST_STEP;
      v4l2_ctrl->default_value = CAMERA_DEF_CONTRAST;
      v4l2_ctrl->flags = V4L2_CTRL_FLAG_UPDATE;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_SATURATION:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
      strlcpy((char *)v4l2_ctrl->name, "Saturation", strlen("Saturation") + 1);
      v4l2_ctrl->minimum = CAMERA_MIN_SATURATION;
      v4l2_ctrl->maximum = CAMERA_MAX_SATURATION;
      v4l2_ctrl->step = CAMERA_SATURATION_STEP;
      v4l2_ctrl->default_value = CAMERA_DEF_SATURATION;
      v4l2_ctrl->flags = V4L2_CTRL_FLAG_UPDATE;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_HUE:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
      strlcpy((char *)v4l2_ctrl->name, "Hue", strlen("Hue") + 1);
      v4l2_ctrl->minimum = CAMERA_MIN_HUE;
      v4l2_ctrl->maximum = CAMERA_MAX_HUE;
      v4l2_ctrl->step = CAMERA_HUE_STEP;
      v4l2_ctrl->default_value = CAMERA_DEF_HUE;
      v4l2_ctrl->flags = V4L2_CTRL_FLAG_UPDATE;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case MSM_V4L2_PID_ISO:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
      strlcpy((char *)v4l2_ctrl->name, "ISO", strlen("ISO") + 1);
      v4l2_ctrl->minimum = CAMERA_ISO_AUTO;
      v4l2_ctrl->maximum = CAMERA_ISO_1600;
      v4l2_ctrl->step = 1;
      v4l2_ctrl->default_value = CAMERA_ISO_AUTO;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_FOCUS_AUTO:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_BOOLEAN;
      strlcpy((char *)v4l2_ctrl->name, "Auto focus", strlen("Auto focus") + 1);
      v4l2_ctrl->minimum = 0;
      v4l2_ctrl->maximum = 0;
      v4l2_ctrl->step = 0;
      v4l2_ctrl->default_value = 0;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_EXPOSURE:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
      strlcpy((char *)v4l2_ctrl->name, "Exposure",
        strlen("Exposure") + 1);
      v4l2_ctrl->minimum = -12;
      v4l2_ctrl->maximum = 12;
      v4l2_ctrl->step = 1;
      v4l2_ctrl->default_value = 0;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_EXPOSURE_AUTO:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_INTEGER;
      strlcpy((char *)v4l2_ctrl->name, "Auto Exposure",
        strlen("Auto Exposure") + 1);
      v4l2_ctrl->minimum = V4L2_EXPOSURE_AUTO;
      v4l2_ctrl->maximum = V4L2_EXPOSURE_AUTO;
      v4l2_ctrl->default_value = V4L2_EXPOSURE_AUTO;
      v4l2_ctrl->step = 1;
      v4l2_ctrl->default_value = V4L2_EXPOSURE_AUTO;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_EXPOSURE_AUTO_PRIORITY:
      v4l2_ctrl->type = V4L2_CTRL_TYPE_BOOLEAN;
      strlcpy((char *)v4l2_ctrl->name, "Exposure Auto Priority",
        strlen("Exposure Auto Priority") + 1);
      v4l2_ctrl->minimum = 0;
      v4l2_ctrl->maximum = 0;
      v4l2_ctrl->step = 1;
      v4l2_ctrl->default_value = FALSE;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    default:
      if (v4l2_ctrl->id >= V4L2_CID_BASE && v4l2_ctrl->id < V4L2_CID_LASTP1) {
        CDBG("%s: disabled control\n", __func__);
        v4l2_ctrl->flags = V4L2_CTRL_FLAG_DISABLED;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      } else {
        ret = -1;
        CDBG("%s: invalid id\n", __func__);
        ctrlCmd->status = CAM_CTRL_INVALID_PARM;
      }
      break;
  }

  return ret;
}/*mctl_proc_MSM_V4L2_QUERY_CTRL*/

/*===========================================================================
 * FUNCTION    - mctl_proc_MSM_V4L2_GET_CTRL -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int mctl_proc_MSM_V4L2_GET_CTRL(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  struct v4l2_control *v4l2_ctrl = (struct v4l2_control *)(ctrlCmd->value);


  *cmdPending = FALSE;
  CDBG("%s: id = %d \n", __func__, v4l2_ctrl->id);

  switch (v4l2_ctrl->id) {
    case V4L2_CID_AUTO_WHITE_BALANCE:
      v4l2_ctrl->value = ctrl->v4l2Ctrl.is_auto_wb;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
      v4l2_ctrl->value = ctrl->v4l2Ctrl.wb_temperature;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_EXPOSURE_AUTO:
      v4l2_ctrl->value = V4L2_EXPOSURE_AUTO;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_BRIGHTNESS:
      v4l2_ctrl->value = ctrl->v4l2Ctrl.brightness;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_EXPOSURE:
      v4l2_ctrl->value = ctrl->v4l2Ctrl.ev_num;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_CONTRAST:
      v4l2_ctrl->value = ctrl->v4l2Ctrl.contrast;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    case V4L2_CID_SATURATION:
      v4l2_ctrl->value = ctrl->v4l2Ctrl.saturation;
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;

    default:
      ctrlCmd->status = CAM_CTRL_SUCCESS;
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - mctl_proc_MSM_V4L2_GET_CROP -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t mctl_proc_MSM_V4L2_GET_CROP(void *parm1, void *parm2, int *cmdPending)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  zoom_ctrl_t *zoomCtrl = &ctrl->zoomCtrl;
  struct msm_ctrl_cmd *ctrlCmd     = (struct msm_ctrl_cmd *)parm2;
  struct v4l2_crop *v4l2_crp     = (struct v4l2_crop *)(ctrlCmd->value);
  v4l2_video_ctrl *pvideo_ctrl = &ctrl->video_ctrl;
  int output_path = 0;
  zoom_scaling_params_t zoomscaling;
  int rc = 0, pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  *cmdPending = FALSE;

  CDBG("%s: E, op_mode = %d, in1_width=%d,in1_height=%d,out1_width=%d,out1_height=%d,"
       "in2_width=%d,in2_height=%d,out2_width=%d,out2_height=%d",
       __func__, pvideo_ctrl->op_mode,
       zoomCtrl->zoomscaling.input1_width, zoomCtrl->zoomscaling.input1_height,
       zoomCtrl->zoomscaling.output1_width, zoomCtrl->zoomscaling.output1_height,
       zoomCtrl->zoomscaling.input2_width, zoomCtrl->zoomscaling.input2_height,
       zoomCtrl->zoomscaling.output2_width, zoomCtrl->zoomscaling.output2_height);
  if(pvideo_ctrl->streamon_mask || pvideo_ctrl->streamon_bundle_mask)
    zoomscaling = zoomCtrl->zoomscaling;
  else {
    rc = -1;
    CDBG_ERROR("%s: stream off",  __func__);
  }

  if (rc < 0) {
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  switch (pvideo_ctrl->op_mode) {
    case MSM_V4L2_CAM_OP_DEFAULT:
    case MSM_V4L2_CAM_OP_PREVIEW:
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW ||
        ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_DEFAULT)
        output_path = 1;
      break;
    case MSM_V4L2_CAM_OP_VIDEO:
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW ||
        ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_DEFAULT) {
        output_path = 1;
#ifdef VFE_2X
        output_path = 2;
#endif
      } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
        CDBG_HIGH("%s: Video crop request should not come.", __func__);
        output_path = 2;
      } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN) {
        CDBG("%s: Live Shot Path. ", __func__);
        output_path = 2;
      }
      break;
    case MSM_V4L2_CAM_OP_CAPTURE:
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN)
        output_path = 2;
      else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL)
        output_path = 1;
      break;
    case MSM_V4L2_CAM_OP_ZSL:
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN)
        output_path = 2;
      else if ((ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL) ||
        (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW))
        output_path = 1;
      break;
    case MSM_V4L2_CAM_OP_RAW:
      CDBG("%s (%d): output_path = %d ?\n", __func__, __LINE__, output_path);
    default:
      break;
  }

  CDBG("%s (%d): output_path = %d\n", __func__, __LINE__, output_path);
  if (output_path == 1) {
    v4l2_crp->c.left = (zoomscaling.output1_width -
      zoomscaling.input1_width)/2;
    v4l2_crp->c.top = (zoomscaling.output1_height -
      zoomscaling.input1_height)/2;
    v4l2_crp->c.width = zoomscaling.input1_width;
    v4l2_crp->c.height = zoomscaling.input1_height;
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  } else if (output_path == 2) {
    CDBG("%s: output_path2: In: wxh:%dx%d Out: wxh:%dx%d", __func__,
      zoomscaling.input2_width, zoomscaling.input2_height,
      zoomscaling.output2_width, zoomscaling.output2_height);
    v4l2_crp->c.left = (zoomscaling.output2_width -
      zoomscaling.input2_width)/2;
    v4l2_crp->c.top = (zoomscaling.output2_height -
      zoomscaling.input2_height)/2;
    v4l2_crp->c.width = zoomscaling.input2_width;
    v4l2_crp->c.height = zoomscaling.input2_height;
    ctrlCmd->status = CAM_CTRL_SUCCESS;
  } else {
    ctrlCmd->status = CAM_CTRL_FAILED;
  }

  if (ctrl->videoHint) {
    mctl_pp_ctrl_t *pp_ctrl;
    float zoom_ratio_x, zoom_ratio_y;
    int i, j;

    if (pipeline_idx < 0) {
      CDBG_ERROR("%s Default pipeline closed ", __func__);
      return -EINVAL;
    }

    pp_ctrl = &ctrl->mctl_pp_ctrl[pipeline_idx].data.pp_ctrl;
    for (i = 0; i < pp_ctrl->num_src; i++) {
      for (j = 0; j < MCTL_PP_MAX_DEST_NUM; j++) {
        if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW ||
          ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_DEFAULT) {

          if (pp_ctrl->src[i].dest[j].data.path == OUTPUT_TYPE_P) {
            CDBG("%s: When FullLiveShot is enabled MDP will not be used to "
              "upscale Preview frames.", __func__);
            v4l2_crp->c.left = 0;
            v4l2_crp->c.top = 0;
            /* Note: Here WxH = 0x0 also works */
            v4l2_crp->c.width = pp_ctrl->src[i].dest[j].data.image_width;
            v4l2_crp->c.height = pp_ctrl->src[i].dest[j].data.image_height;
          }
        } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {

          if (pp_ctrl->src[i].dest[j].data.path == OUTPUT_TYPE_V) {
            CDBG_HIGH("%s: Video crop request shouldn't come. If it has then "
              "sending the crop_info such that no further cropping happens.",
              __func__);

            v4l2_crp->c.left = 0;
            v4l2_crp->c.top = 0;
            /* Note: Here WxH = 0x0 also works */
            v4l2_crp->c.width = pp_ctrl->src[i].dest[j].data.image_width;
            v4l2_crp->c.height = pp_ctrl->src[i].dest[j].data.image_height;
          }
        } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN) {
          if (pp_ctrl->src[i].dest[j].data.dis_enable) {
            CDBG("%s: Create crop info for main image such that it combines "
              "DIS+Zoom. ", __func__);
            cam_dis_info_t dis_info_in, dis_info_out;
            struct msm_pp_crop crop_info_in, crop_info_out;

            dis_info_in = pp_ctrl->src[i].dest[j].data.dis_info;

            dis_info_out.extra_pad_w = (float)dis_info_in.extra_pad_w *
              (float)ctrl->curr_output_info.output[PRIMARY].image_width /
              (float)pp_ctrl->src[i].data.image_width;
            dis_info_out.extra_pad_h = (float)dis_info_in.extra_pad_h *
              (float)ctrl->curr_output_info.output[PRIMARY].image_height /
              (float)pp_ctrl->src[i].data.image_height;

            dis_info_out.x = (float)dis_info_in.x *
              (float)ctrl->curr_output_info.output[PRIMARY].image_width /
              (float)pp_ctrl->src[i].data.image_width;
            dis_info_out.y = (float)dis_info_in.y *
              (float)ctrl->curr_output_info.output[PRIMARY].image_height /
              (float)pp_ctrl->src[i].data.image_height;

            crop_info_in.dst_x = 0;
            crop_info_in.dst_y = 0;
            crop_info_in.dst_w =
              ctrl->curr_output_info.output[PRIMARY].image_width;
            crop_info_in.dst_h =
              ctrl->curr_output_info.output[PRIMARY].image_height;

            if (v4l2_crp->c.width) {
              CDBG("%s: Zoom + DIS on Width", __func__);
              crop_info_in.src_w = (float)v4l2_crp->c.width;
              crop_info_in.src_x =
                (ctrl->curr_output_info.output[PRIMARY].image_width -
                crop_info_in.src_w) / 2;
            } else {
              CDBG("%s: only DIS on Width", __func__);
              crop_info_in.src_w =
                ctrl->curr_output_info.output[PRIMARY].image_width;
              crop_info_in.src_x = v4l2_crp->c.left;
            }

            if (v4l2_crp->c.height) {
              CDBG("%s: Zoom + DIS on Height", __func__);
              crop_info_in.src_h = (float)v4l2_crp->c.height;
              crop_info_in.src_y =
                (ctrl->curr_output_info.output[PRIMARY].image_height -
                crop_info_in.src_h) / 2;
            } else {
              CDBG("%s: only DIS on Height", __func__);
              crop_info_in.src_h =
                ctrl->curr_output_info.output[PRIMARY].image_height;
              crop_info_in.src_y = v4l2_crp->c.top;
            }

            CDBG("%s: DIS In: XxY: %dx%d, extra WxH: %dx%d", __func__,
              dis_info_in.x, dis_info_in.y, dis_info_in.extra_pad_w,
              dis_info_in.extra_pad_h);
            CDBG("%s: DIS Out: XxY: %dx%d, extra WxH: %dx%d", __func__,
              dis_info_out.x, dis_info_out.y, dis_info_out.extra_pad_w,
              dis_info_out.extra_pad_h);
            CDBG("%s: Crop Src: XxY: %dx%d, WxH: %dx%d", __func__,
              crop_info_in.src_x, crop_info_in.src_y, crop_info_in.src_w,
              crop_info_in.src_h);
            CDBG("%s: Crop Dst: XxY: %dx%d, WxH: %dx%d", __func__,
              crop_info_in.dst_x, crop_info_in.dst_y, crop_info_in.dst_w,
              crop_info_in.dst_h);

            mctl_pp_merge_crop_dis_offset(&crop_info_in, &dis_info_out,
              &crop_info_out);

            v4l2_crp->c.left = crop_info_out.src_x;
            v4l2_crp->c.top = crop_info_out.src_y;
            v4l2_crp->c.width = crop_info_out.src_w;
            v4l2_crp->c.height = crop_info_out.src_h;
          }
          break;
        } else {
          CDBG_ERROR("%s: Invalid Stream Type=%d", __func__,
            ctrlCmd->stream_type);
          return -1;
        }
      }
    }
  }
  memcpy(&ctrl->crop_info,v4l2_crp, sizeof(struct v4l2_crop));
  CDBG("%s: v4l2_crp->c: (%d, %d), (%d, %d)\n", __func__, v4l2_crp->c.left,
    v4l2_crp->c.top, v4l2_crp->c.width, v4l2_crp->c.height);
  return 0;
}

/*===========================================================================
 * FUNCTION    - mctl_proc_do_ASD -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void mctl_proc_do_ASD(void *cctrl)
{

  int rc = TRUE;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_interface_input_t *sp_input = &(sp_ctrl->intf.input);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);

  sp_input->mctl_info.type = STATS_PROC_ASD_TYPE;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].process(
  ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, sp_input->mctl_info.type, &(sp_ctrl->intf));
  if (rc < 0)
    CDBG_ERROR("%s Stats processing failed for Whitebalance ", __func__);

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_ASD_PARMS, NULL, NULL);

  mctl_eztune_update_diagnostics(EZ_MCTL_ISP_ASD_CMD);
}

/*===========================================================================
 * FUNCTION    -  mctl_proc_send_hist_stats -
 *
 * DESCRIPTION: send the HIST stats buffer.
 *==========================================================================*/
static int mctl_proc_send_hist_stats(void *cctrl,
  uint16_t *hist_statsBuffer, int index)
{
  mctl_config_ctrl_t * ctrl = (mctl_config_ctrl_t*)cctrl;
  uint32_t reg_size_w, reg_size_h;

  int i = 0;
  camera_preview_histogram_info *histogram_info;
  uint8_t shiftBit = 0;
  CDBG("%s: E\n", __func__);
  /* send data */
  histogram_info = (camera_preview_histogram_info *)
    ctrl->video_ctrl.user_hist_buf_info.local_frame_info[index].local_vaddr;
  if (!histogram_info) {
    return -1;
  }
  CDBG("hist_statsBuffer =%p, histogram_info =%p", hist_statsBuffer, histogram_info);
  for (i=0; i<256; i++) {
    histogram_info->buffer[i] = hist_statsBuffer[i]<<shiftBit;
  }
  histogram_info->max_value = (1<<(16+shiftBit)) - 1;
  CDBG("%s: X\n", __func__);
  return 0;
}

/*===========================================================================
 * FUNCTION    -  mctl_proc_parse_hist_param -
 *
 * DESCRIPTION: parse the HIST stats buffer.
 *==========================================================================*/
static void mctl_proc_parse_hist_param(void *cctrl, vfe_stats_output_t *vfe_out)
{
  mctl_config_ctrl_t * ctrl = (mctl_config_ctrl_t*)cctrl;
  vfe_3a_parms_udpate_t vfe_3a_params;
  uint16_t *hist_statsBuffer;
  uint32_t i;
  int rc = TRUE;

  CDBG("%s: E\n", __func__);
  hist_statsBuffer = vfe_out->ihist_stats_buffer;
  if (ctrl->vfeData.enable_histogram) {
    if (mctl_proc_send_hist_stats(ctrl, hist_statsBuffer, vfe_out->ihist_index) < 0)
      CDBG_ERROR("%s cannot fill userspace stats\n", __func__);
  }
  CDBG("%s: X\n", __func__);
}
/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_IHIST -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int mctl_stats_proc_MSG_ID_STATS_IHIST(void *parm1,  void *parm2)
{
  int rc = 0;
  int i = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  static int flag = 0;
  struct msm_stats_buf release_buf;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;

  CDBG("%s: E\n", __func__);

  vfe_stats_out->ihist_done = FALSE;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
         ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
         STATS_TYPE_YUV, parm2,
         (void *)vfe_stats_out);
  if(rc < 0) {
    CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
    return rc;
  }
  if(!vfe_stats_out->ihist_done) {
    CDBG_ERROR("%s: stats parsing not done", __func__);
    return 0;
  }

  CDBG("%s: index: %d \n", __func__, vfe_stats_out->ihist_index);
  mctl_proc_parse_hist_param(ctrl, vfe_stats_out);

  if (ctrl->vfeData.enable_histogram) {
    struct v4l2_event_and_payload hist_evt;
    mm_camera_event_t *app_event;
    memset(&hist_evt, 0, sizeof(hist_evt));
    hist_evt.payload_length = 0;
    hist_evt.transaction_id = -1;
    hist_evt.payload = NULL;
    app_event = (mm_camera_event_t *)hist_evt.evt.u.data;
    hist_evt.evt.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_APP_NOTIFY_EVENT;
    app_event->event_type = MM_CAMERA_EVT_TYPE_STATS;
    app_event->e.stats.event_id = MM_CAMERA_STATS_EVT_HISTO;
    app_event->e.stats.e.stats_histo.index = i;
    ioctl(ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &hist_evt);
  }

  /*run ASD*/
  vfe_stats_out->vfe_stats_struct.ihist_op.histogram =
    ctrl->vfeData.vfe_Ihist_data;
  mctl_proc_do_ASD(ctrl);

  CDBG("%s: X rc = %d", __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION    - mctl_proc_MSG_ID_STATS_COMPOSITE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int mctl_proc_MSG_ID_STATS_COMPOSITE(void *cctrl,
  void *parm2)
{
  int8_t rc = 0;

  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *)parm2;
  struct msm_stats_buf *buf = (struct msm_stats_buf *) (adsp->data);

  //TODO: need to add support for Bayer stats
  CDBG("%s, vfe common message = 0x%x\n", __func__, buf->status_bits);

  if((buf->status_bits & VFE_STATS_AEC) && buf->aec.buff) {
    rc = mctl_stats_proc_MSG_ID_STATS_AE(cctrl, parm2);
    if (rc < 0)
      return rc;
  }

  if((buf->status_bits & VFE_STATS_AWB) && buf->awb.buff) {
    rc = mctl_stats_proc_MSG_ID_STATS_AWB(cctrl, parm2);
    if (rc < 0)
      return rc;
  }

  CDBG("%s: buf->af.buf = %lx\n",__func__, buf->af.buff);
  if((buf->status_bits & VFE_STATS_AF) && buf->af.buff) {
    rc = mctl_stats_proc_MSG_ID_STATS_AF(cctrl, parm2);
    if (rc < 0)
       return rc;
  }

  if((buf->status_bits & VFE_STATS_IHIST) && buf->ihist.buff) {
    rc = mctl_stats_proc_MSG_ID_STATS_IHIST(cctrl, parm2);
    if (rc < 0)
      return rc;
  }

  if((buf->status_bits & VFE_STATS_RS) && buf->rs.buff) {
    rc = mctl_stats_proc_MSG_ID_STATS_RS(cctrl, parm2);
    if (rc < 0)
       return rc;
  }

  if((buf->status_bits & VFE_STATS_CS) && buf->cs.buff) {
    rc = mctl_stats_proc_MSG_ID_STATS_CS(cctrl, parm2);
    if (rc < 0)
      return rc;
  }

  return rc;
}

#ifdef FEATURE_GYRO
/*==============================================================================
 * FUNCTION    - is_compute_timeframe -
 *
 * DESCRIPTION:
 *============================================================================*/
int mctl_proc_compute_timeframe(mctl_config_ctrl_t *ctrl,
    uint64_t sof_timestamp,
    dsps_set_data_t *dsps_set_data)
{
  sensor_get_t sensor_get;
  uint32_t pixel_clock;
  uint32_t pixels_per_line;
  uint32_t height;
  uint64_t frame_time;
  uint32_t fps;

  sensor_get.data.aec_info.op_mode = ctrl->sensor_op_mode;
  if(ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_GET_SENSOR_MODE_AEC_INFO, &sensor_get, sizeof(sensor_get))) {
    CDBG("%s Error getting AEC Info from sensor ", __func__);
    return -1;
  }

  pixels_per_line = sensor_get.data.aec_info.pixels_per_line;
  pixel_clock = sensor_get.data.aec_info.pclk;

  sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_PREVIEW;
  if(ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get))) {
    CDBG("%s Error getting DIM Info from sensor ", __func__);
    return -1;
  }

  height = sensor_get.data.sensor_dim.height;

  if(ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
      ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
      SENSOR_GET_CUR_FPS, &sensor_get, sizeof(sensor_get))) {
    CDBG("%s Error getting FPS from sensor ", __func__);
    return -1;
  }

  fps = (sensor_get.data.fps >> 8);

  frame_time = (uint64_t) ((pixels_per_line * height) /
      (pixel_clock / SEC_TO_USEC));

  /* Set start timestamp as frame midpoint */
  dsps_set_data->data.t_start = sof_timestamp + (frame_time / 2);

  /* Approximate end timestamp as frame midpoint of next frame */
  dsps_set_data->data.t_end = dsps_set_data->data.t_start + (SEC_TO_USEC / fps);

  return 0;
}
#endif

/*===========================================================================
 * FUNCTION    - mctl_proc_MSG_ID_SOF_ACK -
 *
 * DESCRIPTION: processing SOF_ACK message
 *==========================================================================*/
static int mctl_proc_MSG_ID_SOF_ACK(mctl_config_ctrl_t *cfg_ctrl,
  void *parm2)
{
  int rc = 0;

#ifdef FEATURE_GYRO
  dsps_set_data_t dsps_set_data;
  dsps_data_t dsps_data;
#endif
  camera_status_t status = CAMERA_STATUS_SUCCESS;
  stats_proc_ctrl_t *sp_ctrl = &(cfg_ctrl->stats_proc_ctrl);
  stats_proc_interface_input_t *sp_input = &(sp_ctrl->intf.input);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);
  zoom_ctrl_t *zctrl = &(cfg_ctrl->zoomCtrl);
  struct msm_cam_evt_msg *adsp =  (struct msm_cam_evt_msg *)parm2;
  uint64_t sof_timestamp;

  CDBG("%s: ctrl->state is %d\n", __func__, cfg_ctrl->state);
  if (cfg_ctrl->state != CAMERA_STATE_STARTED)
    return 0;

#ifdef FEATURE_GYRO
  sof_timestamp = ((uint64_t)adsp->timestamp.tv_sec * SEC_TO_USEC)
      + ((uint64_t)adsp->timestamp.tv_nsec / USEC_TO_NSEC);

  if (mctl_proc_compute_timeframe(cfg_ctrl, sof_timestamp, &dsps_set_data)) {
    CDBG_ERROR("%s: Error computing timestamps for EIS\n", __func__);
  } else {
    /* Send DSPS Request Message */
    dsps_set_data.msg_type = DSPS_GET_REPORT;
    dsps_set_data.data.id = adsp->frame_id + 1;
    dsps_proc_set_params(&dsps_set_data);
  }
#endif

  /*HDR check*/
  if (((cfg_ctrl->vfeMode == VFE_OP_MODE_SNAPSHOT) ||
       (cfg_ctrl->vfeMode == VFE_OP_MODE_RAW_SNAPSHOT)) &&
      ((cfg_ctrl->hdrCtrl.exp_bracketing_enable) ||
       (cfg_ctrl->hdrCtrl.hdr_enable))) {
      rc = hdr_calc_sensor_gain_upon_sof(cfg_ctrl);
      if (rc < 0) {
        CDBG_ERROR("%s: HDR sensor gain failed\n", __func__);
      }
      cfg_ctrl->hdrCtrl.current_snapshot_count++;
  }

  /* Update gain to sensor and store digital gain */
  if (sp_ctrl->sof_update_needed) {
    rc = config_proc_write_sensor_gain(cfg_ctrl);
    sp_ctrl->sof_update_needed = FALSE;
  }
  if(cfg_ctrl->zoom_done_pending) {
    cfg_ctrl->zoom_done_pending = 0;
    if (!cfg_ctrl->sensor_stream_fullsize) {
      /* Send events of zoom success. */
      CDBG("%s: call config_proc_send_zoom_done_event ", __func__);
      rc = config_proc_send_zoom_done_event(cfg_ctrl, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s: config_proc_send_zoom_done_event failed w/ rc=%d",
          __func__, rc);
        return rc;
      }
    }
  }

  /* send SOF to sensor interface*/
  /* VFE needs special treatment*/
  if (!cfg_ctrl->vfe_reg_updated) {
    uint8_t vfe_reg_updated;
    if (((cfg_ctrl->vfeMode == VFE_OP_MODE_PREVIEW) ||
      (cfg_ctrl->vfeMode == VFE_OP_MODE_VIDEO) ||
      (cfg_ctrl->vfeMode == VFE_OP_MODE_ZSL)) &&
      (cfg_ctrl->curr_output_info.vfe_operation_mode &
      ~(VFE_OUTPUTS_RDI0|VFE_OUTPUTS_RDI1))) {
      vfe_flash_parms_t vfe_flash_parms;
      if (sp_output->aec_d.strobe_cfg_st == STROBE_TO_BE_CONFIGURED) {
        stats_proc_set_t set_param;
        set_param.type = STATS_PROC_AEC_TYPE;
        set_param.d.set_aec.type = AEC_STROBE_CFG_ST;
        set_param.d.set_aec.d.aec_strobe_cfg_st = STROBE_PRE_CONFIGURED;
        cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
          cfg_ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
          set_param.type, &set_param,
          &(sp_ctrl->intf));
        vfe_flash_parms.flash_mode = VFE_FLASH_STROBE;
      } else if (sp_output->aec_d.use_led_estimation) {
        vfe_flash_parms.flash_mode = VFE_FLASH_LED;
      } else {
        vfe_flash_parms.flash_mode = VFE_FLASH_NONE;
      }
      if ((int)vfe_flash_parms.flash_mode != VFE_FLASH_NONE) {
        vfe_flash_parms.sensitivity_led_off = sp_output->aec_d.flash_si.off;
        vfe_flash_parms.sensitivity_led_low = sp_output->aec_d.flash_si.low;
        vfe_flash_parms.sensitivity_led_hi = sp_output->aec_d.flash_si.high;
        vfe_flash_parms.strobe_duration = sp_input->chromatix->AEC_strobe_flash.strobe_min_time;
        rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
          cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_FLASH_PARMS,
          (void *)&vfe_flash_parms, NULL);
        if (rc)
          CDBG("%s VFE Set FLASH params failed ", __func__);
      }
    } else {
      CDBG_HIGH("%s: In-Correct Mode", __func__);
    }
    rc = cfg_ctrl->comp_ops[MCTL_COMPID_VFE].process(
      cfg_ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SOF_NOTIFY,
      (void *)&vfe_reg_updated);
    if (!rc)
      cfg_ctrl->vfe_reg_updated = vfe_reg_updated;
    else
      rc = -EFAULT;
  }

  if (((cfg_ctrl->vfeMode == VFE_OP_MODE_PREVIEW) ||
    (cfg_ctrl->vfeMode == VFE_OP_MODE_VIDEO) ||
    (cfg_ctrl->vfeMode == VFE_OP_MODE_ZSL)) &&
    (sp_output->aec_d.strobe_cfg_st == STROBE_TO_BE_CONFIGURED) &&
    (cfg_ctrl->curr_output_info.vfe_operation_mode &
      ~(VFE_OUTPUTS_RDI0|VFE_OUTPUTS_RDI1))) {

      /* camif timer start */
      camif_input_t camif_input;
      camif_input.obj_idx = 0; /* hard coded for now */
      camif_input.d.strobe_info.enabled =
        (sp_output->aec_d.strobe_cfg_st == STROBE_PRE_FIRED);
      camif_input.d.strobe_info.duration = sp_input->chromatix->AEC_strobe_flash.strobe_min_time;
      rc = cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
        cfg_ctrl->comp_ops[MCTL_COMPID_CAMIF].handle, CAMIF_PARAMS_STROBE_INFO, (void *)&camif_input, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s: set parm CAMIF_PARAMS_STROBE_INFO failed %d",
          __func__, rc);
        return -EINVAL;
      }
  }

  return rc;
} /* mctl_proc_MSG_ID_SOF_ACK */

/*===========================================================================
 * FUNCTION    - mctl_proc_MSG_ID_SYNC_TIMER0_DONE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int mctl_proc_MSG_ID_SYNC_TIMER0_DONE(void *cctrl,
  void *parm2)
{
  int rc;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);

  stats_proc_set_t sp_set_param;
  sp_set_param.type = STATS_PROC_AEC_TYPE;
  sp_set_param.d.set_aec.type = AEC_STROBE_CFG_ST;

  if ((ctrl->vfeMode == VFE_OP_MODE_SNAPSHOT) ||
    (ctrl->vfeMode == VFE_OP_MODE_RAW_SNAPSHOT) ||
    (ctrl->vfeMode == VFE_OP_MODE_JPEG_SNAPSHOT)) {
    sp_set_param.d.set_aec.d.aec_strobe_cfg_st = STROBE_NOT_NEEDED;
    flash_strobe_set_t strobe_set_data;
    strobe_set_data.charge_enable = 1;
	ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].set_params(
	  ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].handle,
	  FLASH_SET_STATE, &strobe_set_data, NULL);
  } else {
    sp_set_param.d.set_aec.d.aec_strobe_cfg_st = STROBE_PRE_FIRED;
  }

  vfe_flash_parms_t vfe_flash_parms;
  vfe_flash_parms.flash_mode = VFE_FLASH_NONE;
  vfe_flash_parms.sensitivity_led_off = sp_output->aec_d.flash_si.off;
  vfe_flash_parms.sensitivity_led_low = sp_output->aec_d.flash_si.low;
  vfe_flash_parms.sensitivity_led_hi = sp_output->aec_d.flash_si.high;
  vfe_flash_parms.strobe_duration = 0;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_FLASH_PARMS,
    (void *)&vfe_flash_parms, NULL);
  if (rc != VFE_SUCCESS)
    CDBG_HIGH("%s VFE Set FLASH params failed ", __func__);
  ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
    sp_set_param.type, &sp_set_param, &(sp_ctrl->intf));
  return TRUE;
}

/*===========================================================================
FUNCTION     mctl_proc_v4l2_request

DESCRIPTION: this function process the commands for user app
===========================================================================*/

int mctl_proc_v4l2_request (m_ctrl_t* pme, void *parm)
{
  int rc  = 0;
  mctl_config_ctrl_t *ctrl = 0;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  int cmdPending = FALSE;

  if (!pme) {
    return -1;
  }

  if (!pme->p_cfg_ctrl || !ctrlCmd)
    return -1;
  else
    ctrl = pme->p_cfg_ctrl;
  CDBG("%s, type = %d\n", __func__, ctrlCmd->type);
  switch (ctrlCmd->type) {
    case MSM_V4L2_VID_CAP_TYPE:
    case MSM_V4L2_STREAM_ON:
    case MSM_V4L2_STREAM_OFF: {
        rc = pme->p_cfg_ctrl->config_intf->config_request(
          ctrl, ctrlCmd, &cmdPending);
      }
      break;

    case MSM_V4L2_QUERY_CTRL: {
        rc = mctl_proc_MSM_V4L2_QUERY_CTRL(ctrl, ctrlCmd, &cmdPending);
      }
      break;

    case MSM_V4L2_GET_CTRL: {
        rc = mctl_proc_MSM_V4L2_GET_CTRL(ctrl, ctrlCmd, &cmdPending);
      }
      break;

    case MSM_V4L2_GET_CROP: {
        rc = mctl_proc_MSM_V4L2_GET_CROP(ctrl, ctrlCmd, &cmdPending);
      }
      break;

    case MSM_V4L2_SET_CTRL: {
        rc = config_proc_set_ctrl_cmd(ctrl, ctrlCmd, &cmdPending);
      }
      break;

    case MSM_V4L2_SET_CTRL_CMD: {
	      rc = config_proc_native_ctrl_cmd(ctrl, ctrlCmd, &cmdPending);
        break;
      }

    case MSM_V4L2_PRIVATE_CMD: {
        rc = config_proc_private_ctrl_cmd(ctrl, ctrlCmd, &cmdPending);
      }
      break;

    default:
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      break;
  }

  if (rc < 0) {
    CDBG_ERROR("Command type %d returned error %d\n", ctrlCmd->type, rc);
    v4l2_ioctl.ioctl_ptr = ctrlCmd;
    if (ioctl(ctrlCmd->resp_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE, &v4l2_ioctl) < 0)
      CDBG_ERROR("(%d)IOCTL MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE type %d failed\n",
        __LINE__, ctrlCmd->type);
    return rc;
  }

  if (cmdPending) {
    int allocPendingCtrlCmd = 1;

    if (ctrlCmd->type == MSM_V4L2_SET_CTRL_CMD) {
      /* This means the ctrlCmd->value is a native ctrl cmd packet.
       * Extract the native cmd from ctrlCmd->value */
      uint8_t *data_ptr = (uint8_t *)ctrlCmd->value;
      struct msm_ctrl_cmd *nativeCmd = (struct msm_ctrl_cmd *)data_ptr;
      switch (nativeCmd->type) {
        case CAMERA_AUTO_FOCUS_CANCEL:
        case CAMERA_STOP_SNAPSHOT:
          CDBG("%s: Should not block. send CTRL_CMD_DONE immediately",
            __FUNCTION__);
          /* No need to allocate pendingCtrlCmd */
          allocPendingCtrlCmd = 0;
          v4l2_ioctl.ioctl_ptr = ctrlCmd;
          rc = ioctl(ctrlCmd->resp_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE,
                     &v4l2_ioctl);
          if (rc < 0)
            CDBG_ERROR("IOCTL MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE type %d failed\n",
              ctrlCmd->type);
          break;
        case CAMERA_PREPARE_SNAPSHOT:
          if (ctrl->pendingPrepSnapCtrlCmd) {
            free(ctrl->pendingPrepSnapCtrlCmd);
            ctrl->pendingPrepSnapCtrlCmd = NULL;
          }
          ctrl->pendingPrepSnapCtrlCmd =
            malloc(sizeof(*ctrlCmd) + ctrlCmd->length);
          if (ctrl->pendingPrepSnapCtrlCmd == NULL) {
            CDBG_ERROR("%s:%d:Failed: ctrl->pendingCtrlCmd is NULL\n",
              __func__, __LINE__);
            v4l2_ioctl.ioctl_ptr = ctrlCmd;
            rc = ioctl(ctrlCmd->resp_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE,
              &v4l2_ioctl);
            if (rc < 0)
              CDBG_ERROR("IOCTL MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE type %d failed\n",
                ctrlCmd->type);
            return rc;
          }
          /* No need to allocate pendingCtrlCmd */
          allocPendingCtrlCmd = 0;

          memcpy(ctrl->pendingPrepSnapCtrlCmd, ctrlCmd, sizeof(*ctrlCmd));
          if (ctrlCmd->length) {
            ctrl->pendingPrepSnapCtrlCmd->value =
              ctrl->pendingPrepSnapCtrlCmd + 1;
            memcpy(ctrl->pendingPrepSnapCtrlCmd->value, ctrlCmd->value,
              ctrlCmd->length);
          } else
            ctrl->pendingPrepSnapCtrlCmd->value = NULL;

          CDBG("%s: pendingPrepSnapCtrlCmd %p", __FUNCTION__,
            ctrl->pendingPrepSnapCtrlCmd);
          break;
        default:
          CDBG("%s: default case, go ahead with allocating pendingCtrlCmd",
            __FUNCTION__);
          break;
      }
    }

    if (allocPendingCtrlCmd) {
      if (ctrl->pendingCtrlCmd)
        *((int *)(0xc0debadd)) = 0xdeadbeef;
      ctrl->pendingCtrlCmd = malloc(sizeof(*ctrlCmd) + ctrlCmd->length);
      if (!ctrl->pendingCtrlCmd) {
        CDBG_ERROR("%s: Error allocating memory\n", __FUNCTION__);
        v4l2_ioctl.ioctl_ptr = ctrlCmd;
        rc = ioctl(ctrlCmd->resp_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE,
          &v4l2_ioctl);
        if (rc < 0)
          CDBG_ERROR("IOCTL MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE type %d failed\n",
            ctrlCmd->type);
        return rc;
      }
      memcpy(ctrl->pendingCtrlCmd, ctrlCmd, sizeof(*ctrlCmd));

      if (ctrlCmd->length) {
        ctrl->pendingCtrlCmd->value = ctrl->pendingCtrlCmd + 1;
        memcpy(ctrl->pendingCtrlCmd->value, ctrlCmd->value, ctrlCmd->length);
      } else ctrl->pendingCtrlCmd->value = NULL;
        CDBG("%s: pendingCtrlCmd %p", __FUNCTION__, ctrl->pendingCtrlCmd);
    }

    if (ctrlCmd->type == MSM_V4L2_STREAM_OFF &&
        cmdPending < 0) {
      struct msm_cam_evt_msg adsp;
      memset(&adsp, 0, sizeof(struct msm_cam_evt_msg));
      adsp.msg_id = MSG_ID_STOP_ACK;
      rc  = pme->p_cfg_ctrl->config_intf->config_proc_event_message(
            pme->p_cfg_ctrl, &adsp);
    }
  } else {
    rc = mctl_send_ctrl_cmd_done(ctrl, ctrlCmd, FALSE);
    if (rc < 0) {
      CDBG_ERROR("%s: sending ctrl_cmd_done failed rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  return rc;
}/*mctl_proc_v4l2_request*/


/*===========================================================================
FUNCTION     mctl_proc_event_message

DESCRIPTION
===========================================================================*/
int mctl_proc_event_message(m_ctrl_t* pme, void *parm)
{
  int rc  = 0;
  struct msm_cam_evt_msg *adsp =  (struct msm_cam_evt_msg *)parm;

  if (!pme) {
    return -1;
  }

  assert(adsp);
  assert(adsp->type == 0);

  if (pme->p_cfg_ctrl) {

    switch (adsp->msg_id) {
      case MSG_ID_STATS_AEC:
        rc = mctl_stats_proc_MSG_ID_STATS_AE(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_STATS_AWB:
        rc = mctl_stats_proc_MSG_ID_STATS_AWB(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_STATS_AF:
        rc = mctl_stats_proc_MSG_ID_STATS_AF(pme->p_cfg_ctrl, adsp);
        break;

      case  MSG_ID_STATS_IHIST:
        rc = mctl_stats_proc_MSG_ID_STATS_IHIST(pme->p_cfg_ctrl, adsp);
        break;

      case  MSG_ID_STATS_RS:
        rc = mctl_stats_proc_MSG_ID_STATS_RS(pme->p_cfg_ctrl, parm);
        break;

      case  MSG_ID_STATS_CS:
        rc = mctl_stats_proc_MSG_ID_STATS_CS(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_STATS_AWB_AEC:
        rc = mctl_stats_proc_MSG_ID_STATS_WB_EXP(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_STATS_BG:
        rc = mctl_stats_proc_MSG_ID_STATS_BG(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_STATS_BF:
        rc = mctl_stats_proc_MSG_ID_STATS_BF(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_STATS_BE:
        rc = mctl_stats_proc_MSG_ID_STATS_BE(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_STATS_BHIST:
        rc = mctl_stats_proc_MSG_ID_STATS_BHIST(pme->p_cfg_ctrl, adsp);
        break;

      case MSG_ID_STATS_COMPOSITE:
        rc = mctl_proc_MSG_ID_STATS_COMPOSITE(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_SOF_ACK:
        rc = mctl_proc_MSG_ID_SOF_ACK(pme->p_cfg_ctrl, parm);
        break;

      case MSG_ID_PIX0_UPDATE_ACK:
        rc = config_proc_INTF_UPDATE_ACK(pme->p_cfg_ctrl, AXI_INTF_PIXEL_0);
        break;

      case MSG_ID_RDI0_UPDATE_ACK:
        rc = config_proc_INTF_UPDATE_ACK(pme->p_cfg_ctrl, AXI_INTF_RDI_0);
        break;

      case MSG_ID_RDI1_UPDATE_ACK:
        rc = config_proc_INTF_UPDATE_ACK(pme->p_cfg_ctrl, AXI_INTF_RDI_1);
        break;

      case MSG_ID_RDI2_UPDATE_ACK:
        rc = config_proc_INTF_UPDATE_ACK(pme->p_cfg_ctrl, AXI_INTF_RDI_2);
        break;

      case MSG_ID_SYNC_TIMER0_DONE:
        rc = mctl_proc_MSG_ID_SYNC_TIMER0_DONE(pme->p_cfg_ctrl, parm);
        break;

      default:
        if (CAMERA_STATE_STARTED!=pme->p_cfg_ctrl->state)
          rc  = pme->p_cfg_ctrl->config_intf->config_proc_event_message(
            pme->p_cfg_ctrl, parm);
        else
          rc  = config_proc_event_message_1(pme->p_cfg_ctrl, parm);
        break;
    } /*switch (adsp->msg_id) */

  } /*if (pme->p_cfg_ctrl) */
  else
    rc = -1;

  return rc;
}/*mctl_proc_event_message*/

/*===========================================================================
 * FUNCTION    - mctl_fetch_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_proc_fetch_params(int my_comp_id, void *cctrl, uint32_t type,
  void *data, int data_len)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  uint8_t intf_type = (type >> 24);
  uint32_t param = (type & 0x00FFFFFF);

  /* Make sure the data is not NULL. */
  if ((data == NULL) || (data_len <= 0)) {
    CDBG_ERROR("%s Invalid data passed by %d while fetching param %d from "
               "interface %d ", __func__, my_comp_id, param, intf_type);
    return -EINVAL;
  }

  if (ctrl->comp_ops[intf_type].handle) {
    CDBG("%s Component %d getting data %d from interface %d ", __func__,
         my_comp_id, param, intf_type);
    rc = ctrl->comp_ops[intf_type].get_params(ctrl->comp_ops[intf_type].handle,
                                              param, data, data_len);
    if (rc < 0) {
      CDBG_ERROR("%s Error while getting param %d from interface %d ",
                 __func__, param, intf_type);
    } else {
        CDBG("%s Successfully received data %d from interface %d ", __func__,
             param, intf_type);
    }
  } else {
    CDBG_ERROR("%s Error: either interface %d not initialized yet, or "
               "invalid interface type passed by %d ", __func__, intf_type,
               my_comp_id);
    rc = -EINVAL;
  }
  return rc;
}

/*==============================================================================
 * FUNCTION    - mctl_ops_get_mem_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
int mctl_ops_get_mem_buffer(void *userdata,
  mem_buffer_struct_t *buf, uint32_t size)
{
  int rc = 0;
  mctl_config_ctrl_t *p_cfg_ctrl = userdata;

  CDBG("%s: E, userdata = %p, size = %d\n",
    __func__, userdata, size);
#ifdef USE_ION
  if(p_cfg_ctrl->ion_dev_fd <= 0) {
    p_cfg_ctrl->ion_dev_fd = open("/dev/ion", O_RDONLY);
    if (p_cfg_ctrl->ion_dev_fd < 0) {
      CDBG_ERROR("Ion dev open failed\n");
      CDBG_ERROR("Error is %s\n", strerror(errno));
      return -1;
    }
  }
#endif
  buf->buf_size = size;
#ifdef USE_ION
  buf->ion_alloc.len = size;
  buf->ion_alloc.flags = ION_FLAG_CACHED;
  buf->ion_alloc.heap_mask =
    (0x1 << CAMERA_ION_HEAP_ID | 0x1 << ION_IOMMU_HEAP_ID);
  buf->ion_alloc.align = 4096;
  buf->buf = do_mmap_ion(p_cfg_ctrl->ion_dev_fd,
    &(buf->ion_alloc), &(buf->fd_data), &(buf->fd));
#else
  buf->buf = do_mmap(buf->buf_size, (int*)&(buf->fd));
#endif
  CDBG("%s: mem_buf vaddr = 0x%p, fd = %d\n, size = 0x%x", __func__, buf->buf,
    buf->fd, buf->buf_size);

  if (!(buf->buf)) {
    CDBG_ERROR("%s: do_mmap failed\n", __func__);
    return -1;
  }
  return rc;
} /* mctl_ops_get_mem_buffer */

static int mctl_get_stats_to_output_path(
 enum msm_stats_enum_type type)
{
  int path = 0x0;

  switch(type) {
    case MSM_STATS_TYPE_AEC:
      path = OUTPUT_TYPE_SAEC;
      break;
    case MSM_STATS_TYPE_AF:
      path = OUTPUT_TYPE_SAFC;
      break;
    case MSM_STATS_TYPE_AWB:
      path = OUTPUT_TYPE_SAWB;
      break;
    case MSM_STATS_TYPE_IHIST:
      path = OUTPUT_TYPE_IHST;
      break;
    case MSM_STATS_TYPE_COMP:
      path = OUTPUT_TYPE_CSTA;
      break;
    default:
      CDBG_ERROR("%s: Invalid stats type\n", __func__);
      break;
  }
  return path;
}

static unsigned short mctl_get_stats_to_image_mode(
 enum msm_stats_enum_type type)
{
  unsigned short image_mode = 0x0;

  switch(type) {
    case MSM_STATS_TYPE_AEC:
      image_mode = MSM_V4L2_EXT_CAPTURE_MODE_AEC;
      break;
    case MSM_STATS_TYPE_AF:
      image_mode = MSM_V4L2_EXT_CAPTURE_MODE_AF;
      break;
    case MSM_STATS_TYPE_AWB:
      image_mode = MSM_V4L2_EXT_CAPTURE_MODE_AWB;
      break;
    case MSM_STATS_TYPE_IHIST:
      image_mode = MSM_V4L2_EXT_CAPTURE_MODE_IHIST;
      break;
    case MSM_STATS_TYPE_COMP:
      image_mode = MSM_V4L2_EXT_CAPTURE_MODE_CSTA;
      break;
    default:
      CDBG_ERROR("%s: Invalid stats type\n", __func__);
      break;
  }
  return image_mode;
}

/*==============================================================================
 * FUNCTION    - mctl_ops_get_stats_op_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
void * mctl_ops_get_stats_op_buffer (void *userdata,
  void *div_frame, enum msm_stats_enum_type type)
{
  mctl_config_ctrl_t *ctrl = userdata;
  struct msm_cam_evt_divert_frame *free_frame = div_frame;
  uint16_t img_mode, idx;
  void *buf = NULL;
  int rc = 0;

  memset(free_frame, 0x0, sizeof(struct msm_cam_evt_divert_frame));
  memset(&free_frame->frame, 0x0, sizeof(struct msm_pp_frame));
  //get image mode corresponding to stats type
  img_mode = mctl_get_stats_to_image_mode(type);
  //get output path corresponding to stats type
  free_frame->frame.path = mctl_get_stats_to_output_path(type);
  free_frame->image_mode = img_mode;
  //get handle for buffer look up
  free_frame->frame.inst_handle =
    config_get_inst_handle(&ctrl->video_ctrl.strm_info,
      STRM_INFO_STATS, img_mode);

  rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_RESERVE_FREE_FRAME,
        free_frame);
  if(rc < 0) {
    CDBG_ERROR(" %s: cannot reserve frame from kernel, path = 0x%x",
      __func__, free_frame->frame.path);
    return 0;
  }

  //index of the free buffer
  idx = free_frame->frame.buf_idx;
  free_frame->frame.mp[0].vaddr =
    (unsigned long)ctrl->video_ctrl.user_buf_info[img_mode].local_frame_info[idx].local_vaddr;

  //Retrieve the local virtual address
  buf = ctrl->video_ctrl.user_buf_info[img_mode].local_frame_info[idx].local_vaddr;
  CDBG("%s: buf : [%p]", __func__,(void *)buf);

  return buf;
}

/*==============================================================================
 * FUNCTION    - mctl_ops_put_stats_op_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
int mctl_ops_put_stats_op_buffer(void *userdata,
  void *div_frame, enum msm_stats_enum_type type)
{
  mctl_config_ctrl_t *ctrl = userdata;
  struct msm_cam_evt_divert_frame *free_frame =
    (struct msm_cam_evt_divert_frame *)div_frame;
  int rc = 0;
  uint16_t img_mode;

  img_mode = mctl_get_stats_to_image_mode(type);
  //get output path corresponding to stats type
  free_frame->frame.path = mctl_get_stats_to_output_path(type);
  free_frame->image_mode = img_mode;
  //get handle for buffer look up
  free_frame->frame.inst_handle =
    config_get_inst_handle(&ctrl->video_ctrl.strm_info,
      STRM_INFO_STATS, img_mode);

  CDBG(" %s: debug path = 0x%x, img mode : %d, inst_handle :0x%x,",
    __func__, free_frame->frame.path, free_frame->image_mode,
    free_frame->frame.inst_handle);

  rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_PICT_PP_DIVERT_DONE, &free_frame->frame);
  if(rc < 0) {
    CDBG_ERROR(" %s: cannot buf done to kernel, path = 0x%x",
      __func__, free_frame->frame.path);
    return 0;
  }

  return rc;
}

/*==============================================================================
 * FUNCTION    - mctl_ops_put_mem_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
int mctl_ops_put_mem_buffer(void *userdata,
                                   mem_buffer_struct_t *buf)
{
  int rc = 0;
  mctl_config_ctrl_t *p_cfg_ctrl = userdata;
  CDBG("%s: E, userdata = %p, mem_buf = %p\n", __func__, userdata, buf->buf);

  if (p_cfg_ctrl->ion_dev_fd > 0 && buf->buf) {
#ifdef USE_ION
  do_munmap_ion(p_cfg_ctrl->ion_dev_fd,
    &(buf->fd_data), buf->buf, buf->ion_alloc.len);
#else
  do_munmap(buf->fd, buf->buf, buf->buf_size);
#endif
  }
  return rc;
} /* mctl_ops_put_mem_buffer */

/*==============================================================================
 * FUNCTION    - mctl_ops_invalidate_mem_buf -
 *
 * DESCRIPTION:
 *============================================================================*/
int mctl_ops_invalidate_mem_buf(void *userdata,
  mem_buffer_struct_t *mem_buf)
{
  int rc = 0;
  struct ion_flush_data cache_inv_data;
  struct ion_custom_data custom_data;
  mctl_config_ctrl_t *p_cfg_ctrl = userdata;

  CDBG("%s: cfg_ctrl = %p, mem_buf %p\n",
    __func__, userdata, mem_buf);
#ifdef USE_ION
  /*invalidate cache before sendign back to hardware*/
  memset(&cache_inv_data, 0, sizeof(struct ion_flush_data));
  cache_inv_data.vaddr = (void*)mem_buf->buf;
  cache_inv_data.fd = mem_buf->fd;
  cache_inv_data.handle = mem_buf->ion_alloc.handle;
  cache_inv_data.length = mem_buf->ion_alloc.len;
  custom_data.cmd = ION_IOC_INV_CACHES;
  custom_data.arg = (unsigned long)&cache_inv_data;
  if(ioctl(p_cfg_ctrl->ion_dev_fd, ION_IOC_CUSTOM, &custom_data) < 0) {
    CDBG_ERROR("%s: Cache Invalidate failed\n", __func__);
    rc = -1;
  }
#endif

  return rc;
} /* mctl_ops_invalidate_mem_buf */

void mctl_stats_init_ops(void *cctrl)
{
  mctl_config_ctrl_t *ctrl = cctrl;

#ifdef USE_ION
  if(ctrl->ion_dev_fd <= 0) {
    ctrl->ion_dev_fd = open("/dev/ion", O_RDONLY);
    if (ctrl->ion_dev_fd < 0) {
      CDBG_ERROR("Ion dev open failed\n");
      CDBG_ERROR("Error is %s\n", strerror(errno));
      return;
    }
  }
#endif

  memset(&ctrl->ops, 0, sizeof(ctrl->ops));
  ctrl->ops.fetch_params = mctl_proc_fetch_params;
  ctrl->ops.get_mem_buffer = mctl_ops_get_mem_buffer;
  ctrl->ops.put_mem_buffer = mctl_ops_put_mem_buffer;
  ctrl->ops.cache_invalidate = mctl_ops_invalidate_mem_buf;
  ctrl->ops.get_stats_op_buffer = mctl_ops_get_stats_op_buffer;
  ctrl->ops.put_stats_op_buffer = mctl_ops_put_stats_op_buffer;
  ctrl->ops.parent = cctrl;
  ctrl->ops.fd = ctrl->camfd;

  memset(&(ctrl->stats_proc_ctrl.vfe_stats_out.vfe_stats_struct), 0,
    sizeof(isp_stats_t));
}

void mctl_stats_deinit_ops(void *cctrl)
{
#ifdef USE_ION
  mctl_config_ctrl_t *p_cfg_ctrl = cctrl;
  if(p_cfg_ctrl->ion_dev_fd > 0) {
    close(p_cfg_ctrl->ion_dev_fd);
    p_cfg_ctrl->ion_dev_fd= 0;
  }
#endif
}
