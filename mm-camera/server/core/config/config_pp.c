/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <media/msm_isp.h>
#include <media/msm_media_info.h>
#include <assert.h>

#include "camera_dbg.h"
#include "camera.h"
#include "config_proc.h"
#include "mctl.h"
#include "mctl_af.h"
#include "cam_mmap.h"

#if 0
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - config_send_crop_to_mctl_pp -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_send_crop_to_mctl_pp(void *p_ctrl, void *p_video_ctrl, void *p_crop)
{
  mctl_pp_cmd_t pp_cmd;
  mctl_config_ctrl_t *ctrl = p_ctrl;
  v4l2_video_ctrl *pvideo_ctrl = p_video_ctrl;
  zoom_scaling_params_t *crop = p_crop;
  int pipeline_idx = pvideo_ctrl->def_pp_idx;

  memset(&pp_cmd, 0, sizeof(pp_cmd));
  pp_cmd.cmd_type = QCAM_MCTL_CMD_SET_CROP;

  if (pvideo_ctrl->op_mode != MSM_V4L2_CAM_OP_VIDEO) {
    return 0; /* not video, do nothing */
  }
  if(ctrl->enableLowPowerMode) {
  /* in low power camcorder, MCTL_PP only used for recording */
    pp_cmd.crop_info.src_path = OUTPUT_TYPE_V;
    pp_cmd.crop_info.dst_w = crop->output2_width;
    pp_cmd.crop_info.dst_h = crop->output2_height;
    pp_cmd.crop_info.src_w = crop->input2_width;
    pp_cmd.crop_info.src_h = crop->input2_height;
  } else {
    /* in 2 output output1 is for video */
    pp_cmd.crop_info.src_path = ctrl->curr_output_info.output[SECONDARY].path;
    pp_cmd.crop_info.dst_w = crop->output1_width;
    pp_cmd.crop_info.dst_h = crop->output1_height;
    pp_cmd.crop_info.src_w = crop->input1_width;
    pp_cmd.crop_info.src_h = crop->input1_height;
    pp_cmd.crop_info.src_path = ctrl->curr_output_info.output[SECONDARY].path;
  }
  CDBG("%s:path=%d,in_w=%d,in_h=%d,out_w=%d,out_h=%d", __func__,
    pp_cmd.crop_info.src_path, pp_cmd.crop_info.src_w, pp_cmd.crop_info.src_h,
    pp_cmd.crop_info.dst_w, pp_cmd.crop_info.dst_h);

  if (pipeline_idx >=0 ) {
    return mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &pp_cmd);
  } else {
    CDBG_ERROR("%s Default pp pipeline is closed ", __func__);
    return -EINVAL;
  }
} /* config_send_crop_to_mctl_pp */

/*===========================================================================
 * FUNCTION    - config_pp_send_stream_on -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_pp_send_stream_on(void *parm1, void *p_ctrlCmd)
{
  int rc = 0;
  mctl_pp_cmd_t pp_cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = p_ctrlCmd;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  memset(&pp_cmd, 0, sizeof(pp_cmd));
  if(ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
    pp_cmd.stream_info.path = OUTPUT_TYPE_V;
    pp_cmd.stream_info.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_VIDEO;
  } else {
    pp_cmd.stream_info.path = OUTPUT_TYPE_P;
    pp_cmd.stream_info.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
  }
  pp_cmd.cmd_type = QCAM_MCTL_CMD_STREAMON;
  if (pipeline_idx >= 0) {
    rc = mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &pp_cmd);
  } else {
    CDBG_ERROR("%s Default pipeline closed. Cannot streamon ", __func__);
    rc = -EINVAL;
  }
  CDBG("%s: stream on, path = %d, image mode = %d rc = %d", __func__,
        pp_cmd.stream_info.path, pp_cmd.stream_info.image_mode, rc);
  return rc;
} /* config_pp_send_stream_on */

/*===========================================================================
 * FUNCTION    - config_pp_send_stream_off -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_pp_send_stream_off(void *parm1, void *p_ctrlCmd)
{
  int rc = 0;
  mctl_pp_cmd_t pp_cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = p_ctrlCmd;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  memset(&pp_cmd, 0, sizeof(pp_cmd));
  if(ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
    pp_cmd.stream_info.path = OUTPUT_TYPE_V;
	pp_cmd.stream_info.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_VIDEO;
  } else {
    pp_cmd.stream_info.path = OUTPUT_TYPE_P;
	pp_cmd.stream_info.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
  }

  pp_cmd.cmd_type = QCAM_MCTL_CMD_STREAMOFF;
  if (pipeline_idx >= 0) {
    rc = mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &pp_cmd);
  } else {
    CDBG_ERROR("%s Default pipeline closed. Cannot streamoff ", __func__);
    rc = -EINVAL;
  }

  CDBG("%s: stream on, path = %d, image mode = %d rc = %d", __func__,
       pp_cmd.stream_info.path, pp_cmd.stream_info.image_mode, rc);
  return rc;
} /* config_pp_send_stream_off */

/*===========================================================================
 * FUNCTION    - config_pp_acquire_hw -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_pp_acquire_hw(void *parm1, void *p_ctrlCmd)
{
  int rc = 0;
  mctl_pp_cmd_t pp_cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = p_ctrlCmd;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  memset(&pp_cmd, 0, sizeof(pp_cmd));
  if(ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW)
    pp_cmd.stream_info.path = OUTPUT_TYPE_P;
  else
    pp_cmd.stream_info.path = OUTPUT_TYPE_V;

  pp_cmd.cmd_type = QCAM_MCTL_CMD_ACQUIRE_HW;
  pp_cmd.stream_info.image_mode = ctrlCmd->stream_type;
  if (pipeline_idx >= 0) {
    rc = mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &pp_cmd);
  } else {
    CDBG_ERROR("%s Default pipeline closed. Cannot acquire hw ", __func__);
    rc = -EINVAL;
  }
  CDBG("%s: path = %d, image mode = %d", __func__,
       pp_cmd.stream_info.path, pp_cmd.stream_info.image_mode);
  return rc;
} /* config_pp_acquire_hw */

/*===========================================================================
 * FUNCTION    - config_pp_release_hw -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_pp_release_hw(void *parm1, void *p_ctrlCmd)
{
  int rc = 0;
  mctl_pp_cmd_t pp_cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = p_ctrlCmd;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  memset(&pp_cmd, 0, sizeof(pp_cmd));
  if(ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW)
    pp_cmd.stream_info.path = OUTPUT_TYPE_P;
  else
    pp_cmd.stream_info.path = OUTPUT_TYPE_V;

  pp_cmd.cmd_type = QCAM_MCTL_CMD_RELEASE_HW;
  pp_cmd.stream_info.image_mode = ctrlCmd->stream_type;
  if (pipeline_idx >= 0) {
    rc = mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &pp_cmd);
  } else {
    CDBG_ERROR("%s Default pipeline closed. Cannot release hw ", __func__);
    rc = -EINVAL;
  }

  CDBG("%s: path = %d, image mode = %d rc = %d", __func__,
       pp_cmd.stream_info.path, pp_cmd.stream_info.image_mode, rc);
  return rc;
} /* config_pp_release_hw */

/*===========================================================================
 * FUNCTION    - config_pp_topology_rdi -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_pp_topology_rdi(mctl_config_ctrl_t *ctrl,
  struct msm_ctrl_cmd *ctrlCmd)
{
  int rc = 0;
  mctl_pp_cmd_t cmd;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  if (pipeline_idx < 0) {
    CDBG_ERROR("%s Default pipeline closed ", __func__);
    return -EINVAL;
  }
  memset(&cmd, 0, sizeof(cmd));

  cmd.src_cfg.num_src = 1;
  cmd.src_cfg.src_idx = MCTL_PP_SRC_IDX_0;

  if (ctrl->videoHint) {
    /* Configure the second destination only in camcorder mode.*/
    cmd.src_cfg.num_dest = 2;
  } else
    cmd.src_cfg.num_dest = 1;

  cmd.src_cfg.parms.dis_enable = 0;
  cmd.src_cfg.parms.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_RDI;
  cmd.src_cfg.parms.path = OUTPUT_TYPE_R;
  cmd.src_cfg.parms.format = ctrl->dimInfo.rdi0_format;
  /* For YUV Sensors, dimInfo->rdi0_width is multiplied by 2.
   * So the actual width of the image is only rdi0_width/2. */
  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR)
    cmd.src_cfg.parms.image_width = ctrl->dimInfo.rdi0_width/2;
  else
    cmd.src_cfg.parms.image_width = ctrl->dimInfo.rdi0_width;
  cmd.src_cfg.parms.image_height = ctrl->dimInfo.rdi0_height;

  CDBG_HIGH("%s: Source fmt %d W x H = %d x %d \n", __func__,
    cmd.src_cfg.parms.format,
    cmd.src_cfg.parms.image_width,
    cmd.src_cfg.parms.image_height);

  cmd.cmd_type = QCAM_MCTL_CMD_CONFIG_SRC;
  mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);

  /* Configure src 0, dest 0 for C2D */
  cmd.dest_cfg.src_idx = MCTL_PP_SRC_IDX_0;
  cmd.dest_cfg.dest_idx = MCTL_PP_DEST_IDX_0;
  cmd.dest_cfg.parms.proc_type = MCTL_PP_C2D_CROP_2D;
  cmd.dest_cfg.parms.action_flag = MSM_MCTL_PP_VPE_FRAME_ACK;
  cmd.dest_cfg.parms.path = OUTPUT_TYPE_P;
  cmd.dest_cfg.parms.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
  cmd.dest_cfg.parms.dis_enable = 0;
  cmd.dest_cfg.parms.format = ctrl->dimInfo.prev_format;
  cmd.dest_cfg.parms.image_width = ctrl->dimInfo.display_width;
  cmd.dest_cfg.parms.image_height = ctrl->dimInfo.display_height;
  cmd.dest_cfg.parms.rotation = ROT_NONE;
  cmd.dest_cfg.parms.hw_type = PP_HW_TYPE_C2D;

  CDBG("%s: Dest %d fmt %d W x H = %d x %d \n", __func__,
    cmd.dest_cfg.dest_idx, cmd.dest_cfg.parms.format,
    cmd.dest_cfg.parms.image_width, cmd.dest_cfg.parms.image_height);

  cmd.cmd_type = QCAM_MCTL_CMD_CONFIG_DEST;
  mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);

  if (ctrl->videoHint) {
      /* Configure src 0, dest 1 for C2D */
      cmd.dest_cfg.src_idx = MCTL_PP_SRC_IDX_0;
      cmd.dest_cfg.dest_idx = MCTL_PP_DEST_IDX_1;
      cmd.dest_cfg.parms.proc_type = MCTL_PP_C2D_CROP_2D;
      cmd.dest_cfg.parms.action_flag = MSM_MCTL_PP_VPE_FRAME_ACK;
      cmd.dest_cfg.parms.path = OUTPUT_TYPE_V;
      cmd.dest_cfg.parms.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_VIDEO;
      cmd.dest_cfg.parms.dis_enable = 0;
      cmd.dest_cfg.parms.format = ctrl->dimInfo.enc_format;
      cmd.dest_cfg.parms.image_width = ctrl->dimInfo.video_width;
      cmd.dest_cfg.parms.image_height = ctrl->dimInfo.video_height;
      cmd.dest_cfg.parms.rotation = ROT_NONE;
      cmd.dest_cfg.parms.hw_type = PP_HW_TYPE_C2D;

      CDBG("%s: Dest %d fmt %d W x H = %d x %d \n", __func__,
        cmd.dest_cfg.dest_idx, cmd.dest_cfg.parms.format,
        cmd.dest_cfg.parms.image_width, cmd.dest_cfg.parms.image_height);

      cmd.cmd_type = QCAM_MCTL_CMD_CONFIG_DEST;
      mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);
  }
  return rc;
} /* config_pp_topology_rdi */
/*===========================================================================
 * FUNCTION    - config_pp_topology_video -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_pp_topology_video(mctl_config_ctrl_t *ctrl,
  int op_mode, struct msm_ctrl_cmd *ctrlCmd)
{
  int rc = 0, dis_enable = 0, split_vfe_out = 0;
  int luma_stride = 0, cbcr_stride = 0;
  mctl_pp_cmd_t cmd;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  if (pipeline_idx < 0) {
    CDBG_ERROR("%s Default pipeline closed ", __func__);
    return -EINVAL;
  }

  memset(&cmd, 0, sizeof(cmd));

  if (ctrl->videoHint && !ctrl->enableLowPowerMode) {
    /* Camcorder mode:
     * Turn ON the full pipeline processing only if its not in
     * low power mode.
     * In Low power mode, only VPE is configured and
     * used only if needed(zoom). */
      split_vfe_out = 1;
  }

  CDBG("%s: op_mode = %d split vfe output %d", __func__,
    op_mode, split_vfe_out);
  dis_enable = ctrl->video_dis.enable_dis & ctrl->video_dis.sensor_has_margin;

  if(split_vfe_out) {
    cmd.src_cfg.dimInfo = ctrl->dimInfo;
    cmd.src_cfg.num_src = 1;
    cmd.src_cfg.src_idx = MCTL_PP_SRC_IDX_0;
    cmd.src_cfg.op_mode = op_mode;
    cmd.src_cfg.parms.dis_enable = dis_enable;
    cmd.src_cfg.parms.image_mode =
      ctrl->curr_output_info.output[SECONDARY].stream_type;
    cmd.src_cfg.parms.path =
      ctrl->curr_output_info.output[SECONDARY].path;
    cmd.src_cfg.parms.format =
      ctrl->curr_output_info.output[SECONDARY].format;
    cmd.src_cfg.parms.image_width =
      ctrl->curr_output_info.output[SECONDARY].image_width +
      ctrl->curr_output_info.output[SECONDARY].extra_pad_width;
    cmd.src_cfg.parms.image_height =
      ctrl->curr_output_info.output[SECONDARY].image_height +
      ctrl->curr_output_info.output[SECONDARY].extra_pad_height;
    cmd.src_cfg.parms.plane[0] =
      ctrl->curr_output_info.output[SECONDARY].plane[0];
    cmd.src_cfg.parms.plane[1] =
      ctrl->curr_output_info.output[SECONDARY].plane[1];

    cmd.src_cfg.num_dest = 2;
    CDBG("%s: Source W x H = %d x %d, stride 0,1 = %d,%d \n", __func__,
      cmd.src_cfg.parms.image_width,
      cmd.src_cfg.parms.image_height,
      cmd.src_cfg.parms.plane[0].stride,
      cmd.src_cfg.parms.plane[1].stride);

    cmd.cmd_type = QCAM_MCTL_CMD_CONFIG_SRC;
    mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);

    /* Configure src 0, dest 0 for C2D */
    cmd.dest_cfg.src_idx = MCTL_PP_SRC_IDX_0;
    cmd.dest_cfg.dest_idx = MCTL_PP_DEST_IDX_0;
    cmd.dest_cfg.parms.proc_type = MCTL_PP_C2D_CROP_2D;
    cmd.dest_cfg.parms.action_flag = MSM_MCTL_PP_VPE_FRAME_ACK;
    cmd.dest_cfg.parms.path = OUTPUT_TYPE_P;
    cmd.dest_cfg.parms.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
    cmd.dest_cfg.parms.dis_enable = dis_enable;
    cmd.dest_cfg.parms.format = ctrl->dimInfo.prev_format;
    cmd.dest_cfg.parms.image_width = ctrl->dimInfo.display_width;
    cmd.dest_cfg.parms.image_height = ctrl->dimInfo.display_height;
    cmd.dest_cfg.parms.plane[0].stride = ctrl->dimInfo.display_width;
    cmd.dest_cfg.parms.plane[1].stride = ctrl->dimInfo.display_width;

    cmd.dest_cfg.parms.rotation = ROT_NONE;
    cmd.dest_cfg.parms.hw_type = PP_HW_TYPE_C2D;

    CDBG("%s: Dest %d W x H = %d x %d ,s0 s1 %d %d\n", __func__, cmd.dest_cfg.dest_idx,
      cmd.dest_cfg.parms.image_width, cmd.dest_cfg.parms.image_height,
      cmd.dest_cfg.parms.plane[0].stride,
      cmd.dest_cfg.parms.plane[1].stride);

    cmd.cmd_type = QCAM_MCTL_CMD_CONFIG_DEST;
    mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);

    /* Configure src 0, dest 1 for C2D/VPE */
    luma_stride = VENUS_Y_STRIDE(COLOR_FMT_NV12, ctrl->dimInfo.video_width);
    cbcr_stride = VENUS_UV_STRIDE(COLOR_FMT_NV12, ctrl->dimInfo.video_width);
    CDBG("%s: Dest 1 width %d, luma_s %d, cbcr_s %d. \n", __func__,
      ctrl->dimInfo.video_width, luma_stride, cbcr_stride);

    cmd.dest_cfg.src_idx = MCTL_PP_SRC_IDX_0;
    cmd.dest_cfg.dest_idx = MCTL_PP_DEST_IDX_1;
    /* Use C2D for both preview and video paths if VPE is not present. */
#ifndef VFE_40
    cmd.dest_cfg.parms.proc_type = MCTL_PP_VPE_CROP_AND_DIS;
#else
    cmd.dest_cfg.parms.proc_type = MCTL_PP_C2D_CROP_2D;
#endif
    cmd.dest_cfg.parms.action_flag = MSM_MCTL_PP_VPE_FRAME_ACK;
    cmd.dest_cfg.parms.path = OUTPUT_TYPE_V;
    cmd.dest_cfg.parms.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_VIDEO;
    cmd.dest_cfg.parms.dis_enable = dis_enable;
    cmd.dest_cfg.parms.format = ctrl->dimInfo.enc_format;
    cmd.dest_cfg.parms.image_width = ctrl->dimInfo.video_width;
    cmd.dest_cfg.parms.image_height = ctrl->dimInfo.video_height;
    cmd.dest_cfg.parms.plane[0].stride = luma_stride;
    cmd.dest_cfg.parms.plane[1].stride = cbcr_stride;

    cmd.dest_cfg.parms.rotation = ROT_NONE;
#ifndef VFE_40
    cmd.dest_cfg.parms.hw_type = PP_HW_TYPE_VPE;
#else
    cmd.dest_cfg.parms.hw_type = PP_HW_TYPE_C2D;
#endif

    CDBG("%s: Dest 1 %d W x H = %d x %d s0 s1 %d %d\n", __func__, cmd.dest_cfg.dest_idx,
      cmd.dest_cfg.parms.image_width, cmd.dest_cfg.parms.image_height,
      cmd.dest_cfg.parms.plane[0].stride,
      cmd.dest_cfg.parms.plane[1].stride);

    cmd.cmd_type = QCAM_MCTL_CMD_CONFIG_DEST;
    mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);

  } else {
    /* non split vfe case: video > preview supported */
    cmd.src_cfg.dimInfo = ctrl->dimInfo;
    cmd.src_cfg.num_src = 1;
    cmd.src_cfg.src_idx = MCTL_PP_SRC_IDX_0;
    cmd.src_cfg.op_mode = op_mode;
    cmd.src_cfg.num_dest = 1;

    cmd.src_cfg.parms.dis_enable = dis_enable;
    cmd.src_cfg.parms.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_VIDEO;
    cmd.src_cfg.parms.path = OUTPUT_TYPE_V;
    cmd.src_cfg.parms.format = ctrl->dimInfo.enc_format;
    cmd.src_cfg.parms.image_width =
      ctrl->curr_output_info.output[PRIMARY].image_width +
      ctrl->curr_output_info.output[PRIMARY].extra_pad_width;;
    cmd.src_cfg.parms.image_height =
      ctrl->curr_output_info.output[PRIMARY].image_height +
      ctrl->curr_output_info.output[PRIMARY].extra_pad_height;

    CDBG("%s: Source W x H = %d x %d \n", __func__,
      cmd.src_cfg.parms.image_width,
      cmd.src_cfg.parms.image_height);

    cmd.cmd_type = QCAM_MCTL_CMD_CONFIG_SRC;
    mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);

    /* Configure src 0, dest 0 for VPE */
    cmd.dest_cfg.src_idx = MCTL_PP_SRC_IDX_0;
    cmd.dest_cfg.dest_idx = MCTL_PP_DEST_IDX_0;
#ifndef VFE_40
    cmd.dest_cfg.parms.proc_type = MCTL_PP_VPE_CROP_AND_DIS;
#else
    cmd.dest_cfg.parms.proc_type = MCTL_PP_C2D_CROP_2D;
#endif
    cmd.dest_cfg.parms.action_flag = MSM_MCTL_PP_VPE_FRAME_ACK;
    cmd.dest_cfg.parms.path = OUTPUT_TYPE_V;
    cmd.dest_cfg.parms.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_VIDEO;
    cmd.dest_cfg.parms.dis_enable = dis_enable;
    cmd.dest_cfg.parms.format = ctrl->dimInfo.enc_format;
    cmd.dest_cfg.parms.image_width = ctrl->dimInfo.video_width;
    cmd.dest_cfg.parms.image_height = ctrl->dimInfo.video_height;
    cmd.dest_cfg.parms.rotation = ROT_NONE;
#ifndef VFE_40
    cmd.dest_cfg.parms.hw_type = PP_HW_TYPE_VPE;
#else
    cmd.dest_cfg.parms.hw_type = PP_HW_TYPE_C2D;
#endif

    CDBG("%s: Dest %d W x H = %d x %d \n", __func__, cmd.dest_cfg.dest_idx,
      cmd.dest_cfg.parms.image_width, cmd.dest_cfg.parms.image_height);

    cmd.cmd_type = QCAM_MCTL_CMD_CONFIG_DEST;
    mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);
  }

  return rc;
} /* config_pp_topology_video */

/*===========================================================================
 * FUNCTION    - config_pp_need_low_power_mode -
 *
 * DESCRIPTION:
 * This function decides, based on the certain features if we need to
 * configure camcorder in low power mode or not.
 * For certain camcorder features like HFR, we have to run in low power mode,
 * i.e. Configuring VFE to output video and preview data directly.
 * In this mode, we impose a constraint that video size should be equal or
 * larger than preview size. Also in this mode, we dont support DIS and
 * full size liveshot.
 *==========================================================================*/
int config_pp_need_low_power_mode(void *p_ctrl, int op_mode,
  int *need_low_power_mode)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = p_ctrl;
  *need_low_power_mode = 0;

  if(op_mode != MSM_V4L2_CAM_OP_VIDEO) {
    CDBG_HIGH("%s: op_mode != MSM_V4L2_CAM_OP_VIDEO. Skip low power check",
      __func__);
    goto end;
  }

  /* First check if HFR is enabled, if yes, definitely use lower power mode */
  if(ctrl->hfr_mode > CAMERA_HFR_MODE_OFF) {
    /* Constraint: video size >= preview size */
    if((ctrl->dimInfo.display_width > ctrl->dimInfo.video_width) ||
      (ctrl->dimInfo.display_height > ctrl->dimInfo.video_height)) {
      CDBG_ERROR("%s: video size %dx%d < preview size %dx%d",
        __func__, ctrl->dimInfo.video_width, ctrl->dimInfo.video_height,
        ctrl->dimInfo.display_width, ctrl->dimInfo.display_height);
      rc = -1;
    } else {
      CDBG_HIGH("%s: hfr is enabled, turn on low power mode", __func__);
      *need_low_power_mode = 1;
    }
  }

  /*forces Normal power mode for badger*/
  #ifdef VFE_40
    *need_low_power_mode = 0;
  #endif

end:
  return rc;
}
/*===========================================================================
 * FUNCTION    - config_pp_setup_pp_topology -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_pp_setup_pp_topology(void *parm1, int op_mode, void *p_ctrlCmd)
{
  int rc = 0;
  int split_vfe_out = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = p_ctrlCmd;

  CDBG("%s: op_mode = %d", __func__, op_mode);

  switch(op_mode) {
    case MSM_V4L2_CAM_OP_VIDEO:
      if (ctrl->channel_stream_info & STREAM_RAW)
        rc = config_pp_topology_rdi(ctrl, ctrlCmd);
      else
        rc = config_pp_topology_video(ctrl, op_mode, ctrlCmd);
      break;

    default:
      CDBG_ERROR("%s: PP not supported for this op mode %d",
        __func__, op_mode);
      rc = -EINVAL;
      break;
  }

  return rc;
} /* config_pp_setup_pp_topology */

/*===========================================================================
 * FUNCTION    - config_pp_end_pp_topology -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_pp_end_pp_topology(void *parm1, int op_mode)
{
  int rc = 0;
  mctl_pp_cmd_t cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  int pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  if (pipeline_idx < 0) {
    CDBG_ERROR("%s Default pipeline closed.", __func__);
    return -EINVAL;
  }

  CDBG("%s: op_mode = %d", __func__, op_mode);
  switch(op_mode) {
    case MSM_V4L2_CAM_OP_VIDEO:
      memset(&cmd, 0, sizeof(cmd));
      cmd.cmd_type = QCAM_MCTL_CMD_RESET_DEST;
      cmd.dest_cfg.src_idx = MCTL_PP_SRC_IDX_0;
      cmd.dest_cfg.dest_idx = MCTL_PP_DEST_IDX_1;
      mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);

      memset(&cmd, 0, sizeof(cmd));
      cmd.cmd_type = QCAM_MCTL_CMD_RESET_DEST;
      cmd.dest_cfg.src_idx = MCTL_PP_SRC_IDX_0;
      cmd.dest_cfg.dest_idx = MCTL_PP_DEST_IDX_0;
      mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);

      memset(&cmd, 0, sizeof(cmd));
      cmd.cmd_type = QCAM_MCTL_CMD_RESET_SRC;
      cmd.src_cfg.src_idx = MCTL_PP_SRC_IDX_0;
      mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);
      break;

    default:
      CDBG_ERROR("%s: PP not supported for this op mode %d",
        __func__, op_mode);
      rc = -EINVAL;
      break;
  }

  return rc;
} /* config_pp_end_pp_topology */

/*===========================================================================
 * FUNCTION    - config_pp_reg_pp_node_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_pp_reg_pp_node_buf(mctl_config_ctrl_t *ctrl, int image_mode)
{
  int rc = 0, i;
  uint32_t idx;
  mctl_pp_node_obj_t *pp_node = &ctrl->pp_node;
  mctl_pp_buf_info_t *buf_info;

  CDBG("%s Storing buffer info for image mode %d ", __func__, image_mode);
  buf_info = &(ctrl->video_ctrl.mctl_buf_info[image_mode]);
  rc = mctl_pp_node_get_buffer_info(pp_node, buf_info->local_frame_info);

  return rc;
} /* config_pp_reg_pp_node_buf */

/*===========================================================================
 * FUNCTION    - config_pp_unreg_pp_node_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void config_pp_unreg_pp_node_buf(mctl_config_ctrl_t *ctrl, int image_mode)
{
  mctl_pp_buf_info_t *buf_info;

  CDBG("%s Removing buffer info for image mode %d ", __func__, image_mode);
  buf_info = &(ctrl->video_ctrl.mctl_buf_info[image_mode]);
  memset(buf_info, 0, sizeof(mctl_pp_buf_info_t));
} /* config_pp_unreg_pp_node_buf */

/*===========================================================================
 * FUNCTION    - config_pp_acquire_mctl_node -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_pp_acquire_mctl_node(void *cctrl)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  mctl_pp_node_obj_t *pp_node = &ctrl->pp_node;

  if (pp_node->fd > 0) {
    CDBG_ERROR("%s Fatal!! MCTL PP Node is already in use.", __func__);
    return -EINVAL;
  }

  /* Open the MCTL PP node and prepare it */
  rc = mctl_pp_node_open(pp_node, ctrl->video_ctrl.mctl_pp_dev_name);
  if (rc < 0) {
    CDBG_ERROR("%s Error opening mctl pp node ", __func__);
    return rc;
  }
  pp_node->acquired_for_rdi = FALSE;
  CDBG("%s Video Hint = %d ", __func__, ctrl->videoHint);
  if (ctrl->channel_stream_info & STREAM_RAW) {
    pp_node->strm_info.format = ctrl->dimInfo.rdi0_format;
    pp_node->strm_info.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_RDI;
    pp_node->strm_info.width = ctrl->dimInfo.rdi0_width;
    pp_node->strm_info.height = ctrl->dimInfo.rdi0_height;
    pp_node->acquired_for_rdi = TRUE;
  } else if (ctrl->videoHint) {
    pp_node->strm_info.format =
      ctrl->curr_output_info.output[SECONDARY].format;
    pp_node->strm_info.image_mode =
      ctrl->curr_output_info.output[SECONDARY].stream_type;
    pp_node->strm_info.width =
      ctrl->curr_output_info.output[SECONDARY].image_width +
      ctrl->curr_output_info.output[SECONDARY].extra_pad_width;
    pp_node->strm_info.height =
      ctrl->curr_output_info.output[SECONDARY].image_height +
      ctrl->curr_output_info.output[SECONDARY].extra_pad_height;
  }
  CDBG("%s Preparing mctl pp node with fmt %d W x H = %d x %d, image type %d",
    __func__, pp_node->strm_info.format, pp_node->strm_info.width,
    pp_node->strm_info.height, pp_node->strm_info.image_mode);

  rc = mctl_pp_node_prepare(cctrl, pp_node, ctrl->ion_dev_fd);
  if (rc < 0) {
    CDBG_ERROR("%s Error preparing mctl pp node ", __func__);
    return rc;
  }

  /* Store the mctl pp node buffer information */
  rc = config_pp_reg_pp_node_buf(ctrl, pp_node->strm_info.image_mode);
  if (rc < 0) {
    CDBG_ERROR("%s Error registering mctl pp buffers ", __func__);
    return rc;
  }
  return rc;
} /* config_pp_acquire_mctl_node */

/*===========================================================================
 * FUNCTION    - config_pp_release_mctl_node -
 *
 * DESCRIPTION:
 *==========================================================================*/
void config_pp_release_mctl_node(void *cctrl)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  mctl_pp_node_obj_t *pp_node = &ctrl->pp_node;

  if (pp_node->fd > 0) {
    CDBG("%s Calling mctl pp node release ", __func__);

    if(mctl_pp_node_release(pp_node, ctrl->ion_dev_fd) < 0)
      CDBG_ERROR("%s Error releasing pp node ", __func__);

    /* Unregister the mctl pp node buffers with poll_cb */
    config_pp_unreg_pp_node_buf(ctrl, pp_node->strm_info.image_mode);

    mctl_pp_node_close(pp_node);
    CDBG("%s MCTL PP Node closed. ", __func__);
  } else {
    CDBG_ERROR("%s mctl pp node was not acquired. ", __func__);
  }
} /* config_pp_release_mctl_node */
