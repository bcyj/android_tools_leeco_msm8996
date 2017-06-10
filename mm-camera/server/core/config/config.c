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
#include <poll.h>
#include <inttypes.h>
#include <media/msm_isp.h>
#include <assert.h>

#include "camera_dbg.h"
#include "camera.h"
#include "config_proc.h"
#include "mctl.h"
#include "mctl_af.h"
#include "cam_mmap.h"

static int config_update_stream_info(void *cctrl, void *data);

#if 0
#undef CDBG
#define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - config_v2_CAMERA_START_common -
 *
 * DESCRIPTION:
 *==========================================================================*/

static int8_t config_CAMERA_SET_CHANNEL_STREAM(void *parm1, void *parm2)
{
  int8_t rc = TRUE;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  uint32_t channel_stream_info;
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_RDI) {
    channel_stream_info = STREAM_RAW;
  } else {
    channel_stream_info = STREAM_IMAGE;
  }
  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    ispif_set_t ispif_set;
    ispif_get_t ispif_get;
    CDBG("%d", channel_stream_info);
    ispif_set.data.channel_stream_info = channel_stream_info;
    ispif_set.data.vfe_interface = VFE0;
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
  CDBG_HIGH("%s rc = %d, status = %d\n", __func__,
    rc, ctrlCmd->status);
  return TRUE;
}


/*===========================================================================
 * FUNCTION    - config_CAMERA_START_VIDEO -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_CAMERA_START_VIDEO(void *parm1, void *parm2, int *cmdPending)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  int rc;
  mod_cmd_t cmd ;
  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("%s: E, ctrl->state = %d\n", __func__, ctrl->state);
  if (ctrl->state != CAMERA_STATE_IDLE && ctrl->state != CAMERA_STATE_SENT_STOP) {
    CDBG_ERROR("CAMERA_START_VIDEO illegal state, ctrl->state = %d\n", ctrl->state);
    return -EINVAL;
  }
  /* reset vfe: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    cmd.mod_cmd_ops = MOD_CMD_RESET;
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_RESET failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  /* reset axi: */
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_RESET failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }
  /* reset camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_RESET failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }

  ctrlCmd->status = CAM_CTRL_SUCCESS;
  ctrl->stats_proc_ctrl.intf.input.mctl_info.trigger_CAF = TRUE;
  ctrl->state = CAMERA_STATE_SENT_RESET;
  ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state = STATS_PROC_STATE_PREVIEW;
  CDBG("%s: X", __func__);
  *cmdPending = TRUE;
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_STOP_VIDEO -
 *
 * DESCRIPTION:
 *==========================================================================*/
int config_CAMERA_STOP_VIDEO(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mod_cmd_t cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("config_CAMERA_STOP_VIDEO:, ctrl->state = %d\n",
    ctrl->state);

  if ((ctrl->state != CAMERA_STATE_IDLE) &&
    (ctrl->state != CAMERA_STATE_STARTED)) {
    CDBG_ERROR("config_CAMERA_STOP_VIDEO: state is not correct ctrl->state = %d\n",
      ctrl->state);
    *cmdPending = FALSE;
    return -EINVAL;
  }

  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
          ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
          ISPIF_PROCESS_STOP_IMMEDIATELY, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ISPIF_PROCESS_STOP_IMMEDIATELY failed\n", __func__);
      return rc;
    }
  }

  /* 1. stop vfe: */
  cmd.mod_cmd_ops = MOD_CMD_STOP;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_STOP failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }

  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_STOP failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }

  /* stop camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_STOP failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }

  ctrlCmd->status = CAM_CTRL_SUCCESS;

  /* stop AF */
  if (ctrl->afCtrl.af_cont_enable) {
    /* Stop AF STATS */
    mctl_af_stop(ctrl);
  }
  ctrl->stats_proc_ctrl.intf.input.mctl_info.trigger_CAF = FALSE;

  ctrl->state = CAMERA_STATE_SENT_STOP;
  CDBG("config_CAMERA_STOP_VIDEO: return ctrlCmd->status = %d \n",
    ctrlCmd->status);
  *cmdPending = TRUE;
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_START_SNAPSHOT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_CAMERA_START_SNAPSHOT(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mod_cmd_t cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("received CAMERA_START_SNAPSHOT!, ctrl->state = %d\n",
    ctrl->state);

  if (ctrl->state != CAMERA_STATE_IDLE) {
    CDBG_HIGH("%s: state is not correct ctrl->state = %d\n",
      __func__, ctrl->state);
    *cmdPending = FALSE;
    return -EINVAL;
  }

  /* reset isp */
  cmd.mod_cmd_ops = MOD_CMD_RESET;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_RESET failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  /* reset axi: */
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_RESET failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }
  /* reset camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_RESET failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  ctrl->state = CAMERA_STATE_SENT_RESET;
  *cmdPending = TRUE;
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_STOP_SNAPSHOT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_CAMERA_STOP_SNAPSHOT(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mod_cmd_t cmd;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("%s: ctrl->state = %d\n", __func__, ctrl->state);

  if (ctrl->state != CAMERA_STATE_STARTED) {
    /* issue vfe command irrespective of state*/
    CDBG_HIGH("config_CAMERA_STOP_SNAPSHOT: ctrl->state = %d\n",
      ctrl->state);
    *cmdPending = FALSE;
    return -EINVAL;
  }

  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
          ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
          ISPIF_PROCESS_STOP_IMMEDIATELY, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ISPIF_PROCESS_STOP_IMMEDIATELY failed\n", __func__);
      return rc;
    }
  }

  /* stop isp */
  cmd.mod_cmd_ops = MOD_CMD_STOP;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_STOP failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  /* stop AXI */
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_STOP failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }
  /* stop camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_STOP failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  ctrl->state = CAMERA_STATE_SENT_STOP;
  CDBG("%s: return ctrlCmd->status = %d \n",
    __func__, ctrlCmd->status);
  *cmdPending = TRUE;
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_START_LIVESHOT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_CAMERA_START_LIVESHOT(void *parm1, void *parm2,
  int *cmdPending)
{

  int rc;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("%s: E \n", __func__);

  /* Live snapshot is a special case, where the inst handle is not
   * available at the time of AXI configuration. Update the instance
   * handle before sending LIVESHOT command so that the inst handle
   * can be updated before VFE requests for a buffer. */
  ctrl->curr_output_info.output[PRIMARY].inst_handle =
    config_get_inst_handle(&ctrl->video_ctrl.strm_info, STRM_INFO_USER,
      ctrl->curr_output_info.output[PRIMARY].stream_type);
  rc = isp_sendcmd(ctrl->camfd, CMD_GENERAL,
         &ctrl->curr_output_info.output[PRIMARY].inst_handle,
         sizeof(ctrl->curr_output_info.output[PRIMARY].inst_handle),
         VFE_CMD_LIVESHOT);
  if (rc < 0) {
    CDBG("%s: START_LIVESHOT cmd failed!\n", __func__);
    *cmdPending = FALSE;
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  /* Just unblock the app since there is no START_ACK in this case */
  *cmdPending = FALSE;
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  CDBG("%s: X \n", __func__);
  return FALSE;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_STOP_LIVESHOT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_CAMERA_STOP_LIVESHOT(void *parm1, void *parm2,
  int *cmdPending)
{

  int rc;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("%s: E \n", __func__);
  ctrlCmd->status = CAM_CTRL_SUCCESS;

  rc = isp_sendcmd(ctrl->camfd, CMD_GENERAL, NULL, 0, VFE_CMD_STOP_LIVESHOT);
  if (rc < 0) {
    CDBG_ERROR("%s: STOP_LIVESHOT cmd failed!\n", __func__);
    *cmdPending = FALSE;
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  *cmdPending = TRUE;
  CDBG("%s: X \n", __func__);
  return FALSE;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_STOP_RECORDING -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_CAMERA_STOP_RECORDING(void *parm1, void *parm2, int *cmdPending)
{

  int rc = 0;
  mctl_config_ctrl_t *ctrl         = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  ctrlCmd->status = CAM_CTRL_SUCCESS;

  CDBG("config_CAMERA_STOP_RECORDING: received CAMERA_STOP_RECORDING!, ctrl->state = %d\n",
    ctrl->state);

  if ((ctrl->state != CAMERA_STATE_IDLE) &&
    (ctrl->state != CAMERA_STATE_STARTED)) {
    CDBG_HIGH("config_CAMERA_STOP_RECORDING: state is not correct ctrl->state = %d\n",
      ctrl->state);
    *cmdPending = FALSE;
    ctrlCmd->status = CAM_CTRL_FAILED;
    return -EINVAL;
  }


  /* stop vfe: */
  rc = isp_sendcmd(ctrl->camfd, CMD_GENERAL,
      NULL, 0, VFE_CMD_STOP_RECORDING);
  if (rc < 0) {
    CDBG_ERROR("config_CAMERA_STOP_RECORDING:%d vfeStopFn failed!\n", __LINE__);
    *cmdPending = FALSE;
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  CDBG("config_CAMERA_STOP_RECORDING: return ctrlCmd->status = %d \n",
    ctrlCmd->status);
  ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
    STATS_PROC_STATE_PREVIEW;
  *cmdPending = TRUE;
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_START_RECORDING -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_CAMERA_START_RECORDING(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl         = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  ctrlCmd->status = CAM_CTRL_SUCCESS;

  CDBG("%s: received CAMERA_START_RECORDING!, ctrl->state = %d\n", __func__,
    ctrl->state);

  if ((ctrl->state != CAMERA_STATE_IDLE) &&
    (ctrl->state != CAMERA_STATE_STARTED)) {
    ctrlCmd->status = CAM_CTRL_FAILED;
    CDBG_HIGH("CAMERA_START_RECORDING illegal state, ctrl->state = %d\n", ctrl->state);
    *cmdPending = FALSE;
    return rc;
  }

  if(ctrl->enableLowPowerMode) {
    /* In Low power camcorder, the inst handle for recording output is not
     * available at the time of AXI configuration. Update the instance
     * handle before sending START_RECORDING command so that the inst handle
     * can be updated before VFE requests for a buffer. */
    ctrl->curr_output_info.output[PRIMARY].inst_handle =
      config_get_inst_handle(&ctrl->video_ctrl.strm_info, STRM_INFO_USER,
        ctrl->curr_output_info.output[PRIMARY].stream_type);
  }
  /* start recording */
  rc = isp_sendcmd(ctrl->camfd, CMD_GENERAL,
         &ctrl->curr_output_info.output[PRIMARY].inst_handle,
         sizeof(ctrl->curr_output_info.output[PRIMARY].inst_handle),
         VFE_CMD_START_RECORDING);
  if (rc < 0) {
    CDBG_ERROR("vfe start record failed!\n");
    *cmdPending = FALSE;
    ctrlCmd->status = CAM_CTRL_FAILED;
    return rc;
  }
  ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state = STATS_PROC_STATE_CAMCORDER;
  *cmdPending = FALSE;
  return rc;
}
/*===========================================================================
 * FUNCTION    - config_CAMERA_START_RAW_SNAPSHOT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_CAMERA_START_RAW_SNAPSHOT(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mod_cmd_t cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("received CAMERA_START_SNAPSHOT!, ctrl->state = %d\n",
    ctrl->state);

  if (ctrl->state != CAMERA_STATE_IDLE) {
    CDBG_HIGH("%s: state is not correct ctrl->state = %d\n",
      __func__, ctrl->state);
    *cmdPending = FALSE;
    return -EINVAL;
  }

  /* reset isp */
  cmd.mod_cmd_ops = MOD_CMD_RESET;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_RESET failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  /* reset axi: */
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_RESET failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }
  /* reset camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_RESET failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  ctrl->state = CAMERA_STATE_SENT_RESET;
  *cmdPending = TRUE;
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_START_ZSL -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_CAMERA_START_ZSL(void *parm1, void *parm2, int *cmdPending)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  int rc = 0;
  mod_cmd_t cmd;
  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("%s: ctrl->state = %d\n", __func__, ctrl->state);

  if (ctrl->state != CAMERA_STATE_IDLE) {
    CDBG_ERROR("%s: state is not correct ctrl->state = %d\n", __func__,
      ctrl->state);
    *cmdPending = FALSE;
    return -EINVAL;
  }

  /* reset isp */
  cmd.mod_cmd_ops = MOD_CMD_RESET;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_RESET failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  /* reset axi: */
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_RESET failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }
  /* reset camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_RESET failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  ctrl->state = CAMERA_STATE_SENT_RESET;
  ctrl->stats_proc_ctrl.intf.input.mctl_info.trigger_CAF = TRUE;

  CDBG("%s: return ctrlCmd->status = %d \n", __func__, ctrlCmd->status);
  *cmdPending = TRUE;
  return rc;
} /* config_CAMERA_START_ZSL */

/*===========================================================================
 * FUNCTION    - config_CAMERA_STOP_ZSL -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_CAMERA_STOP_ZSL(void *parm1, void *parm2, int *cmdPending)
{
  mod_cmd_t cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  int rc = 0;

  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("%s:, ctrl->state = %d\n", __func__, ctrl->state);

  if ((ctrl->state != CAMERA_STATE_IDLE) &&
    (ctrl->state != CAMERA_STATE_STARTED)) {
    CDBG_HIGH("%s:: state is not correct ctrl->state = %d\n", __func__,
      ctrl->state);
    *cmdPending = FALSE;
    return -EINVAL;
  }

  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
          ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
          ISPIF_PROCESS_STOP_IMMEDIATELY, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ISPIF_PROCESS_STOP_IMMEDIATELY failed\n", __func__);
      return rc;
    }
  }
  /* stop vfe: */
  cmd.mod_cmd_ops = MOD_CMD_STOP;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_STOP failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  /* stop AXI: */
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_STOP failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }
  /* stop camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_STOP failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  ctrl->state = CAMERA_STATE_SENT_STOP;
  ctrl->stats_proc_ctrl.intf.input.mctl_info.trigger_CAF = FALSE;
  *cmdPending = TRUE;
  CDBG("%s: return ctrlCmd->status = %d \n", __func__, ctrlCmd->status);
  return rc;
} /* config_CAMERA_STOP_ZSL */

/*===========================================================================
 * FUNCTION    - config_calc_image_padding -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_calc_image_padding(pixel_crop_info_t *camif_op,
  vfe_output_info_t *vfe_op, frame_margin_t marginPercentage)
{
  float margin = (float)marginPercentage / 100;
  uint32_t padded_w, padded_h;

  /* ToDo: This should use Demosaic output size and not Camif */
  uint32_t input_w = camif_op->last_pixel - camif_op->first_pixel;
  uint32_t input_h = camif_op->last_line - camif_op->first_line;

  CDBG("%s: Margin = %d%%\n", __func__, marginPercentage);

  padded_w = CEILING32((vfe_op->image_width) +
    (uint16_t)(vfe_op->image_width * margin));
  padded_h = CEILING32((vfe_op->image_height) +
    (uint16_t)(vfe_op->image_height * margin));

  if ((padded_w > input_w) || (padded_h > input_h)) {
    CDBG_HIGH("%s: Not enough margin, try with lower margin.\n", __func__);
    return FALSE;
  }

  vfe_op->extra_pad_width = padded_w - vfe_op->image_width;
  vfe_op->extra_pad_height = padded_h - vfe_op->image_height;

  CDBG("%s: Old %dx%d\n", __func__, vfe_op->image_width, vfe_op->image_height);
  CDBG("%s: New %dx%d\n", __func__, padded_w, padded_h);

  return TRUE;
} /* config_calc_image_padding */

/*===========================================================================
  * FUNCTION    - config_decide_vfe2x_outputs -
  *
  * DESCRIPTION:
  *==========================================================================*/
static int config_decide_vfe2x_outputs(void *cctrl)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;

  if (ctrl->videoHint && ctrl->sensor_stream_fullsize)
     ctrl->vfeMode = VFE_OP_MODE_VIDEO;
  else
     ctrl->vfeMode = VFE_OP_MODE_PREVIEW;

  memset(&ctrl->curr_output_info, 0, sizeof(ctrl->curr_output_info));

  CDBG_HIGH("%s Current mode %d %d", __func__, ctrl->ops_mode, ctrl->vfeMode);
  ctrl->curr_output_info.num_output = 2;
  CDBG_HIGH("%s Current mode %d Full size liveshot : %s",
            __func__, ctrl->ops_mode,
            ctrl->sensor_stream_fullsize ? "Enabled" : "Disabled");

  ctrl->curr_output_info.output[PRIMARY].extra_pad_width = 0;
  ctrl->curr_output_info.output[PRIMARY].extra_pad_height = 0;
  ctrl->curr_output_info.output[SECONDARY].extra_pad_width = 0;
  ctrl->curr_output_info.output[SECONDARY].extra_pad_height = 0;

  switch (ctrl->vfeMode) {
  case VFE_OP_MODE_PREVIEW:
    ctrl->curr_output_info.output[PRIMARY].image_width =
    ctrl->dimInfo.video_width;
    ctrl->curr_output_info.output[PRIMARY].image_height =
    ctrl->dimInfo.video_height;
    ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
      ctrl->dimInfo.video_width;
    ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
      ctrl->dimInfo.video_width;
    ctrl->curr_output_info.output[PRIMARY].format =
    ctrl->video_ctrl.enc_format;
    ctrl->curr_output_info.output[PRIMARY].stream_type =
    MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
    ctrl->curr_output_info.output[PRIMARY].path =
    OUTPUT_TYPE_P;
    ctrl->curr_output_info.vfe_operation_mode =
            VFE_OUTPUTS_PREVIEW;
    ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY;
    ctrl->curr_output_info.num_output = 1;
    break;
      /* Configure secondary output for preview */
    case VFE_OP_MODE_VIDEO:
      ctrl->curr_output_info.output[PRIMARY].image_width =
         ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].image_height =
         ctrl->dimInfo.picture_height;
      ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
        ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
        ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].format =
         ctrl->video_ctrl.main_img_format;
      ctrl->curr_output_info.output[PRIMARY].stream_type =
         MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
      ctrl->curr_output_info.output[PRIMARY].path =
         OUTPUT_TYPE_S;

      ctrl->curr_output_info.output[SECONDARY].image_width =
         ctrl->dimInfo.video_width;
      ctrl->curr_output_info.output[SECONDARY].image_height =
         ctrl->dimInfo.video_height;
      ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
        ctrl->dimInfo.video_width;
      ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
        ctrl->dimInfo.video_width;
      ctrl->curr_output_info.output[SECONDARY].format =
         ctrl->video_ctrl.enc_format;
      ctrl->curr_output_info.output[SECONDARY].stream_type =
         MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
      ctrl->curr_output_info.output[SECONDARY].path =
         OUTPUT_TYPE_P;
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY |
                                              VFE_OUTPUT_SECONDARY;
      ctrl->curr_output_info.vfe_operation_mode =
         VFE_OUTPUTS_MAIN_AND_PREVIEW;

      if (ctrl->dimInfo.picture_width >
            ctrl->curr_output_info.output[SECONDARY].image_width &&
           ctrl->dimInfo.picture_height >
            ctrl->curr_output_info.output[SECONDARY].image_height)
        ctrl->sensor_stream_fullsize = 1;
      prepare_liveshot(ctrl);
      CDBG_HIGH("%s Sending LIVESHOT CMD to VFE ", __func__);
      rc = isp_sendcmd(ctrl->camfd, CMD_GENERAL, NULL, 0, VFE_CMD_LIVESHOT);
      break;
    case VFE_OP_MODE_ZSL:
      /* Configure primary output for snapshot */
      ctrl->curr_output_info.output[PRIMARY].image_width =
        ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].image_height =
        ctrl->dimInfo.picture_height;
      ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
        ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
        ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].format =
        ctrl->video_ctrl.main_img_format;
      ctrl->curr_output_info.output[PRIMARY].stream_type =
        MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
      ctrl->curr_output_info.output[PRIMARY].path =
        OUTPUT_TYPE_S;

      /* Configure secondary output for preview */
      ctrl->curr_output_info.output[SECONDARY].image_width =
        ctrl->dimInfo.video_width;
      ctrl->curr_output_info.output[SECONDARY].image_height =
        ctrl->dimInfo.video_height;
      ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
        ctrl->dimInfo.video_width;
      ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
        ctrl->dimInfo.video_width;
      ctrl->curr_output_info.output[SECONDARY].format =
        ctrl->video_ctrl.prev_format;
      ctrl->curr_output_info.output[SECONDARY].stream_type =
        MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
      ctrl->curr_output_info.output[SECONDARY].path =
        OUTPUT_TYPE_P;
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY |
                                              VFE_OUTPUT_SECONDARY;
      ctrl->curr_output_info.vfe_operation_mode =
        VFE_OUTPUTS_MAIN_AND_PREVIEW;
      break;
  case VFE_OP_MODE_SNAPSHOT:
        ctrl->curr_output_info.output[PRIMARY].image_width =
        ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].image_height =
        ctrl->dimInfo.picture_height;
        ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].format =
        ctrl->dimInfo.main_img_format;
        ctrl->curr_output_info.output[PRIMARY].stream_type =
        MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
        ctrl->curr_output_info.output[PRIMARY].path =
        OUTPUT_TYPE_S;

        ctrl->curr_output_info.output[SECONDARY].image_width =
        ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].image_height =
        ctrl->dimInfo.thumbnail_height;
        ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].format =
        ctrl->dimInfo.thumb_format;
        ctrl->curr_output_info.output[SECONDARY].stream_type =
        MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL;
        ctrl->curr_output_info.output[SECONDARY].path =
        OUTPUT_TYPE_T;
        ctrl->curr_output_info.vfe_operation_mode =
        VFE_OUTPUTS_MAIN_AND_THUMB;
        ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY |
                                              VFE_OUTPUT_SECONDARY;
      break;
    case VFE_OP_MODE_RAW_SNAPSHOT:
      ctrl->curr_output_info.output[PRIMARY].image_width =
      ctrl->dimInfo.raw_picture_width;
      ctrl->curr_output_info.output[PRIMARY].image_height =
      ctrl->dimInfo.raw_picture_height;
      ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
        ctrl->dimInfo.raw_picture_width;
      ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
        ctrl->dimInfo.raw_picture_width;
      ctrl->curr_output_info.output[PRIMARY].format =
      CAMERA_BAYER_SBGGR10; //Does this change?
      ctrl->curr_output_info.output[PRIMARY].stream_type =
      MSM_V4L2_EXT_CAPTURE_MODE_RAW;
      ctrl->curr_output_info.output[PRIMARY].path =
      OUTPUT_TYPE_S;
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY;
      ctrl->curr_output_info.vfe_operation_mode =
      VFE_OUTPUTS_RAW;
      ctrl->curr_output_info.num_output = 1;
      break;
    default:
      CDBG_HIGH("%s Invalid ops mode %d ",
                __func__, ctrl->ops_mode);
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_INVALID;
      ctrl->curr_output_info.vfe_operation_mode = 0;
      ctrl->curr_output_info.num_output = 0;
      rc = -EINVAL;
      break;
  }
   CDBG_HIGH("%s: Ports Used %d, Op mode %d", __func__,
              ctrl->curr_output_info.vfe_ports_used,
              ctrl->curr_output_info.vfe_operation_mode);
   CDBG_HIGH("%s: Primary: %dx%d, extra_pad: %dx%d, Fmt: %d, Type: %d, Path: %d",
              __func__, ctrl->curr_output_info.output[PRIMARY].image_width,
              ctrl->curr_output_info.output[PRIMARY].image_height,
              ctrl->curr_output_info.output[PRIMARY].extra_pad_width,
              ctrl->curr_output_info.output[PRIMARY].extra_pad_height,
              ctrl->curr_output_info.output[PRIMARY].format,
              ctrl->curr_output_info.output[PRIMARY].stream_type,
              ctrl->curr_output_info.output[PRIMARY].path);
   CDBG_HIGH("%s: Secondary: %dx%d, extra_pad: %dx%d, Fmt: %d, Type: %d, Path: %d",
              __func__, ctrl->curr_output_info.output[SECONDARY].image_width,
              ctrl->curr_output_info.output[SECONDARY].image_height,
              ctrl->curr_output_info.output[SECONDARY].extra_pad_width,
              ctrl->curr_output_info.output[SECONDARY].extra_pad_height,
              ctrl->curr_output_info.output[SECONDARY].format,
              ctrl->curr_output_info.output[SECONDARY].stream_type,
              ctrl->curr_output_info.output[SECONDARY].path);

   return rc;
}

/*===========================================================================
 * FUNCTION    - config_decide_vfe_outputs -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_decide_vfe_outputs(void *cctrl)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  pixel_crop_info_t camif_ctrl_t;

  memset(&ctrl->curr_output_info, 0, sizeof(ctrl->curr_output_info));
  CDBG("%s Current mode %d vfe mode %d", __func__, ctrl->ops_mode, ctrl->vfeMode);
  ctrl->curr_output_info.num_output = 2;
  ctrl->sensor_stream_fullsize = 0;

  ctrl->curr_output_info.output[PRIMARY].extra_pad_width = 0;
  ctrl->curr_output_info.output[PRIMARY].extra_pad_height = 0;
  ctrl->curr_output_info.output[SECONDARY].extra_pad_width = 0;
  ctrl->curr_output_info.output[SECONDARY].extra_pad_height = 0;

  switch (ctrl->vfeMode) {
    case VFE_OP_MODE_VIDEO:
      if (ctrl->videoHint) {
        if(ctrl->enableLowPowerMode) {
            /* Camcorder mode: Low power
             * Assume Video size is larger than preview size.
             * Configure primary channel with video resolution
             * and secondary with preview resolution. */
            ctrl->curr_output_info.output[SECONDARY].image_width =
              ctrl->dimInfo.display_width;
            ctrl->curr_output_info.output[SECONDARY].image_height =
              ctrl->dimInfo.display_height;
            ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
              ctrl->dimInfo.display_width;
            ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
              ctrl->dimInfo.display_width;
            ctrl->curr_output_info.output[SECONDARY].format =
              ctrl->video_ctrl.prev_format;
            ctrl->curr_output_info.output[SECONDARY].stream_type =
              MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
            ctrl->curr_output_info.output[SECONDARY].path =
              OUTPUT_TYPE_P;
            ctrl->curr_output_info.vfe_operation_mode =
              VFE_OUTPUTS_VIDEO_AND_PREVIEW;
            ctrl->curr_output_info.output[PRIMARY].image_width =
              ctrl->dimInfo.video_width;
            ctrl->curr_output_info.output[PRIMARY].image_height =
              ctrl->dimInfo.video_height;
            ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
              ctrl->dimInfo.video_width;
            ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
              ctrl->dimInfo.video_width;
            ctrl->curr_output_info.output[PRIMARY].format =
              ctrl->video_ctrl.enc_format;
            ctrl->curr_output_info.output[PRIMARY].stream_type =
              MSM_V4L2_EXT_CAPTURE_MODE_VIDEO;
            ctrl->curr_output_info.output[PRIMARY].path =
              OUTPUT_TYPE_V;
        } else {
          /* Camcorder mode: Normal
           * If full size liveshot is required, configure
           * the primary channel with main image resolution
           * and secondary channel with preview/video resolution.
           * If full size liveshot is not required, configure the
           * primary channel with dimensions same as secondary
           * channel. This is to make sure that the pipeline
           * configuration is same in camcorder mode. */
          /* Configure secondary output for preview/video */
          if (ctrl->dimInfo.display_width >= ctrl->dimInfo.video_width) {
            CDBG("%s: Preview Size is larger or Equal than Video ", __func__);
            ctrl->curr_output_info.output[SECONDARY].image_width =
              ctrl->dimInfo.display_width;
            ctrl->curr_output_info.output[SECONDARY].image_height =
              ctrl->dimInfo.display_height;
            ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
              ctrl->dimInfo.display_width;
            ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
              ctrl->dimInfo.display_width;
            ctrl->curr_output_info.output[SECONDARY].format =
              ctrl->video_ctrl.prev_format;
            ctrl->curr_output_info.output[SECONDARY].stream_type =
              MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
            ctrl->curr_output_info.output[SECONDARY].path =
              OUTPUT_TYPE_P;
            ctrl->curr_output_info.vfe_operation_mode =
              VFE_OUTPUTS_MAIN_AND_PREVIEW;
          } else {
            CDBG("%s: Preview Size is smaller than Video ", __func__);
            ctrl->curr_output_info.output[SECONDARY].image_width =
              ctrl->dimInfo.video_width;
            ctrl->curr_output_info.output[SECONDARY].image_height =
              ctrl->dimInfo.video_height;
            ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
              ctrl->dimInfo.video_width;
            ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
              ctrl->dimInfo.video_width;
            ctrl->curr_output_info.output[SECONDARY].format =
              ctrl->video_ctrl.enc_format;
            ctrl->curr_output_info.output[SECONDARY].stream_type =
              MSM_V4L2_EXT_CAPTURE_MODE_VIDEO;
            ctrl->curr_output_info.output[SECONDARY].path =
              OUTPUT_TYPE_V;
            ctrl->curr_output_info.vfe_operation_mode =
              VFE_OUTPUTS_MAIN_AND_VIDEO;
          }
          /* Enable full size streaming from the sensor only if
           * Picture resolution selected by the user is greater than
           * Video resolution.
           * But if DIS is enabled, then we also need to check if adding the
           * margin to video size will make picture size to become lesser
           * than video size. If it is, then turn off full size streaming.
           *  (Assume 10% margin for now). */
          if (ctrl->dimInfo.picture_width >
                ctrl->curr_output_info.output[SECONDARY].image_width &&
              ctrl->dimInfo.picture_height >
                ctrl->curr_output_info.output[SECONDARY].image_height) {
            /* Enable and then check the DIS condition.*/
            ctrl->sensor_stream_fullsize = 1;

            if ( ctrl->video_dis.enable_dis
                && ((ctrl->dimInfo.picture_width <
                    (ctrl->dimInfo.video_width * 1.1))
                || (ctrl->dimInfo.picture_height <
                    (ctrl->dimInfo.video_height * 1.1)))) {
              ctrl->sensor_stream_fullsize = 0;
            }
          }

          /* ToDo: Add 3D Margin requirements here only. */
          if (ctrl->video_dis.enable_dis) {
            camif_input_t camif_input;
            sensor_set_t sensor_set;

            rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
                       ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
                       SENSOR_SET_STOP_STREAM, NULL, NULL);

            if (rc < 0) {
              CDBG_ERROR("%s: SENSOR_SET_STOP_STREAM failed %d\n", __func__,
                rc);
              return rc;
            }


            if (ctrl->sensor_stream_fullsize)
              sensor_set.data.mode = SENSOR_MODE_ZSL;
            else
              sensor_set.data.mode = SENSOR_MODE_VIDEO;

            rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
                       ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
                       SENSOR_SET_MODE, &sensor_set, NULL);
            if (rc < 0) {
              CDBG_ERROR("%s: sensor_set_params failed %d\n", __func__, rc);
              return rc;
            }

            rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
                       ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
                       SENSOR_SET_START_STREAM, NULL, NULL);

            if (rc < 0) {
              CDBG_ERROR("%s: SENSOR_SET_START_STREAM failed %d\n", __func__,
                rc);
              return rc;
            }

            if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
              camif_input.obj_idx = 0;
              rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
                       ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
                       CAMIF_PARAMS_SENSOR_DIMENSION, (void *)&camif_input, NULL);
              if (rc < 0) {
                CDBG_ERROR("%s: set parm CAMIF_PARAMS_SENSOR_FORMAT failed %d",
                             __func__, rc);
                return -EINVAL;
              }

              camif_input.obj_idx = 0;
              rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
                       ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
                       CAMIF_PARAMS_SENSOR_CROP_WINDOW, (void *)&camif_input, NULL);
              if (rc < 0) {
                CDBG_ERROR("%s: set parm CAMIF_PARAMS_SENSOR_CROP_WINDOW failed %d",
                           __func__, rc);
                return -EINVAL;
              }
            }

            if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
              rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_SET_CAMIF_DIM, NULL, NULL);
              if (rc < 0)
                CDBG_HIGH("%s Error while setting CAMIF_DIM ", __func__);

              rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
                     ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                     VFE_GET_PIXEL_CROP_INFO, &camif_ctrl_t,
                     sizeof(camif_ctrl_t));
              if (rc < 0)
                CDBG_HIGH("%s Error while getting PIXEL_CROP_INFO ", __func__);
            }

            /* Lets start by assuming sensor has enough margin for DIS.
             * If it doesnt, we reset it to 0 below. This extra flag
             * is needed because the HAL sets/resets enable_dis
             * everytime there is a setParameter call from the application.
             * So we need to check both enable_dis & sensor_has_margin flags
             * to really know if we can perform DIS on the current resolution.*/
            ctrl->video_dis.sensor_has_margin = 1;

            CDBG("%s: DIS is enabled, calculate extra padding. ", __func__);
            if (config_calc_image_padding(&camif_ctrl_t,
            &ctrl->curr_output_info.output[SECONDARY], MARGIN_P_10)) {
              ctrl->video_dis.dis_margin_p = MARGIN_P_10;
              CDBG("%s: DIS 10%% Margin is Successful\n", __func__);
            } else if (config_calc_image_padding(&camif_ctrl_t,
            &ctrl->curr_output_info.output[SECONDARY], MARGIN_P_8)) {
              ctrl->video_dis.dis_margin_p = MARGIN_P_8;
              CDBG("%s: DIS 8%% Margin is Successful\n", __func__);
            } else if (config_calc_image_padding(&camif_ctrl_t,
            &ctrl->curr_output_info.output[SECONDARY], MARGIN_P_6)) {
              ctrl->video_dis.dis_margin_p = MARGIN_P_6;
              CDBG("%s: DIS 6%% Margin is Successful\n", __func__);
            } else if (config_calc_image_padding(&camif_ctrl_t,
            &ctrl->curr_output_info.output[SECONDARY], MARGIN_P_4)) {
              ctrl->video_dis.dis_margin_p = MARGIN_P_4;
              CDBG("%s: DIS 4%% Margin is Successful\n", __func__);
            } else {
              CDBG_ERROR("%s: Error Not enough Margin for DIS\n", __func__);
              ctrl->video_dis.enable_dis = FALSE;
              ctrl->video_dis.sensor_has_margin = 0;
            }
          }
          if (ctrl->sensor_stream_fullsize) {
            /* Full size liveshot enabled.
             * Configure primary output for live snapshot */
            ctrl->curr_output_info.output[PRIMARY].image_width =
              ctrl->dimInfo.picture_width;
            ctrl->curr_output_info.output[PRIMARY].image_height =
              ctrl->dimInfo.picture_height;
            ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
              ctrl->dimInfo.picture_width;
            ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
              ctrl->dimInfo.picture_width;
            ctrl->curr_output_info.output[PRIMARY].format =
              ctrl->video_ctrl.main_img_format;
            ctrl->curr_output_info.output[PRIMARY].stream_type =
              MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
            ctrl->curr_output_info.output[PRIMARY].path =
              OUTPUT_TYPE_S;
            /* If DIS is enabled, and there is a possibility of picture
             * dimensions becoming smaller than (video size + DIS margin)
             * This will result in primary scalar output being configured
             * to smaller resolution than secondary scalar, which is not
             * correct. Avoid this scenario by configuring the primary
             * dimensions to be same as secondary dimension + extra padding.
             * This will not affect live snapshot, because in this scenario,
             * HAL falls back to Video size live snapshot. */
            if (ctrl->video_dis.enable_dis) {
              if (((ctrl->curr_output_info.output[SECONDARY].image_width +
                   ctrl->curr_output_info.output[SECONDARY].extra_pad_width) >=
                  ctrl->curr_output_info.output[PRIMARY].image_width) ||
                  ((ctrl->curr_output_info.output[SECONDARY].image_height +
                   ctrl->curr_output_info.output[SECONDARY].extra_pad_height) >=
                  ctrl->curr_output_info.output[PRIMARY].image_height)) {
                ctrl->curr_output_info.output[PRIMARY].image_width =
                  ctrl->curr_output_info.output[SECONDARY].image_width +
                  ctrl->curr_output_info.output[SECONDARY].extra_pad_width;
                ctrl->curr_output_info.output[PRIMARY].image_height =
                  ctrl->curr_output_info.output[SECONDARY].image_height +
                  ctrl->curr_output_info.output[SECONDARY].extra_pad_height;
              }
            }
          } else {
            /* Full size liveshot disabled.
             * Configure primary output with dimensions same
             * as secondary output */
            ctrl->curr_output_info.output[PRIMARY].image_width =
              ctrl->curr_output_info.output[SECONDARY].image_width +
              ctrl->curr_output_info.output[SECONDARY].extra_pad_width;
            ctrl->curr_output_info.output[PRIMARY].image_height =
              ctrl->curr_output_info.output[SECONDARY].image_height +
              ctrl->curr_output_info.output[SECONDARY].extra_pad_height;
            ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
              ctrl->curr_output_info.output[PRIMARY].image_width;
            ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
              ctrl->curr_output_info.output[PRIMARY].image_width;
            ctrl->curr_output_info.output[PRIMARY].format =
              ctrl->video_ctrl.main_img_format;
            ctrl->curr_output_info.output[PRIMARY].stream_type =
              MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
            ctrl->curr_output_info.output[PRIMARY].path =
              OUTPUT_TYPE_S;
          }
          if ((ctrl->curr_output_info.output[SECONDARY].image_width >
                ctrl->curr_output_info.output[PRIMARY].image_width) ||
              (ctrl->curr_output_info.output[SECONDARY].image_height >
               ctrl->curr_output_info.output[PRIMARY].image_height)) {
            CDBG_ERROR("%s Secondary output %d x %d greater than "
                "Primary output %d x %d. Unsupported mode.", __func__,
                ctrl->curr_output_info.output[SECONDARY].image_width,
                ctrl->curr_output_info.output[SECONDARY].image_height,
                ctrl->curr_output_info.output[PRIMARY].image_width,
                ctrl->curr_output_info.output[PRIMARY].image_height);
            rc = -EINVAL;
            break;
          }
        }
        ctrl->curr_output_info.vfe_ports_used =
          VFE_OUTPUT_PRIMARY | VFE_OUTPUT_SECONDARY;

      } else {
          ctrl->curr_output_info.output[PRIMARY].image_width =
            ctrl->dimInfo.display_width;
          ctrl->curr_output_info.output[PRIMARY].image_height =
            ctrl->dimInfo.display_height;
          ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
            ctrl->dimInfo.display_width;
          ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
            ctrl->dimInfo.display_width;
          ctrl->curr_output_info.output[PRIMARY].format =
            ctrl->video_ctrl.prev_format;
          ctrl->curr_output_info.output[SECONDARY].image_width =
            ctrl->dimInfo.display_width;
          ctrl->curr_output_info.output[SECONDARY].image_height =
            ctrl->dimInfo.display_height;
          ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
            ctrl->dimInfo.display_width;
          ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
            ctrl->dimInfo.display_width;
          ctrl->curr_output_info.output[PRIMARY].stream_type =
            MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
          ctrl->curr_output_info.output[PRIMARY].path =
            OUTPUT_TYPE_P;
          ctrl->curr_output_info.vfe_operation_mode =
            VFE_OUTPUTS_PREVIEW;
          ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY;
          ctrl->curr_output_info.num_output = 1;
      }

      if (ctrl->channel_interface_mask & RDI_0) {
        ctrl->curr_output_info.output[TERTIARY1].image_width =
          ctrl->dimInfo.rdi0_width;
        ctrl->curr_output_info.output[TERTIARY1].image_height =
          ctrl->dimInfo.rdi0_height;
        ctrl->curr_output_info.output[TERTIARY1].plane[0].stride =
          ctrl->dimInfo.rdi0_width;
        ctrl->curr_output_info.output[TERTIARY1].plane[1].stride =
          ctrl->dimInfo.rdi0_width;
        ctrl->curr_output_info.output[TERTIARY1].format =
          ctrl->video_ctrl.rdi0_format;
        ctrl->curr_output_info.output[TERTIARY1].stream_type =
          MSM_V4L2_EXT_CAPTURE_MODE_RDI;
        ctrl->curr_output_info.output[TERTIARY1].path =
          OUTPUT_TYPE_R;
      }

     if (ctrl->channel_interface_mask & RDI_1) {
        ctrl->curr_output_info.output[TERTIARY2].image_width =
          ctrl->dimInfo.rdi1_width;
        ctrl->curr_output_info.output[TERTIARY2].image_height =
          ctrl->dimInfo.rdi1_height;
        ctrl->curr_output_info.output[TERTIARY2].plane[0].stride =
          ctrl->dimInfo.rdi1_width;
        ctrl->curr_output_info.output[TERTIARY2].plane[1].stride =
          ctrl->dimInfo.rdi1_width;
        ctrl->curr_output_info.output[TERTIARY2].format =
          ctrl->video_ctrl.rdi1_format;
        ctrl->curr_output_info.output[TERTIARY2].stream_type =
          MSM_V4L2_EXT_CAPTURE_MODE_RDI1;
        ctrl->curr_output_info.output[TERTIARY2].path =
          OUTPUT_TYPE_R1;
      }
      switch (ctrl->channel_interface_mask) {
        case PIX_0 | RDI_0:
          ctrl->curr_output_info.vfe_operation_mode |=
            VFE_OUTPUTS_RDI0;
          ctrl->curr_output_info.vfe_ports_used |= VFE_OUTPUT_TERTIARY1;
          ctrl->curr_output_info.num_output += 1;
          break;
        case PIX_0 | RDI_1:
          ctrl->curr_output_info.vfe_operation_mode |=
            VFE_OUTPUTS_RDI1;
          ctrl->curr_output_info.vfe_ports_used |= VFE_OUTPUT_TERTIARY2;
          ctrl->curr_output_info.num_output += 1;
          break;
        case RDI_0 | RDI_1:
          ctrl->curr_output_info.vfe_operation_mode =
            (VFE_OUTPUTS_RDI0|VFE_OUTPUTS_RDI1);
          ctrl->curr_output_info.vfe_ports_used = (VFE_OUTPUT_TERTIARY1|
            VFE_OUTPUT_TERTIARY2);
          ctrl->curr_output_info.num_output = 2;
          break;
        case RDI_1:
          ctrl->curr_output_info.vfe_operation_mode =
            VFE_OUTPUTS_RDI1;
          ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_TERTIARY2;
          ctrl->curr_output_info.num_output = 1;
          break;
        case RDI_0:
          ctrl->curr_output_info.vfe_operation_mode =
            VFE_OUTPUTS_RDI0;
          ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_TERTIARY1;
          ctrl->curr_output_info.num_output = 1;
          break;
        default:
          break;
      }
      break;

    case VFE_OP_MODE_SNAPSHOT:
      if (ctrl->dimInfo.thumbnail_width > ctrl->dimInfo.picture_width) {
        ctrl->curr_output_info.output[PRIMARY].image_width =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[PRIMARY].image_height =
          ctrl->dimInfo.thumbnail_height;
        ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[PRIMARY].format =
          ctrl->dimInfo.thumb_format;
        ctrl->curr_output_info.output[PRIMARY].stream_type =
          MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL;
        ctrl->curr_output_info.output[PRIMARY].path =
          OUTPUT_TYPE_T;

        ctrl->curr_output_info.output[SECONDARY].image_width =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[SECONDARY].image_height =
          ctrl->dimInfo.picture_height;
        ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[SECONDARY].format =
          ctrl->dimInfo.main_img_format;
        ctrl->curr_output_info.output[SECONDARY].stream_type =
          MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
        ctrl->curr_output_info.output[SECONDARY].path =
          OUTPUT_TYPE_S;
        ctrl->curr_output_info.vfe_operation_mode =
          VFE_OUTPUTS_THUMB_AND_MAIN;
      } else {
        ctrl->curr_output_info.output[PRIMARY].image_width =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].image_height =
          ctrl->dimInfo.picture_height;
        ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].format =
          ctrl->dimInfo.main_img_format;
        ctrl->curr_output_info.output[PRIMARY].stream_type =
          MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
        ctrl->curr_output_info.output[PRIMARY].path =
          OUTPUT_TYPE_S;

        ctrl->curr_output_info.output[SECONDARY].image_width =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].image_height =
          ctrl->dimInfo.thumbnail_height;
        ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].format =
          ctrl->dimInfo.thumb_format;
        ctrl->curr_output_info.output[SECONDARY].stream_type =
          MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL;
        ctrl->curr_output_info.output[SECONDARY].path =
          OUTPUT_TYPE_T;
        ctrl->curr_output_info.vfe_operation_mode =
          VFE_OUTPUTS_MAIN_AND_THUMB;
      }
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY |
                                              VFE_OUTPUT_SECONDARY;
      break;
    case VFE_OP_MODE_RAW_SNAPSHOT:
      ctrl->curr_output_info.output[PRIMARY].image_width =
        ctrl->dimInfo.raw_picture_width;
      ctrl->curr_output_info.output[PRIMARY].image_height =
        ctrl->dimInfo.raw_picture_height;
      ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
        ctrl->dimInfo.raw_picture_width;
      ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
        ctrl->dimInfo.raw_picture_width;
      ctrl->curr_output_info.output[PRIMARY].format =
        CAMERA_BAYER_SBGGR10; //Does this change?
      ctrl->curr_output_info.output[PRIMARY].stream_type =
        MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
      ctrl->curr_output_info.output[PRIMARY].path =
        OUTPUT_TYPE_S;
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY;
      ctrl->curr_output_info.vfe_operation_mode =
        VFE_OUTPUTS_RAW;
      ctrl->curr_output_info.num_output = 1;
      break;
    case VFE_OP_MODE_ZSL:
      /* Configure primary output for snapshot */
      ctrl->curr_output_info.output[PRIMARY].image_width =
        ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].image_height =
        ctrl->dimInfo.picture_height;
      ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
        ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
        ctrl->dimInfo.picture_width;
      ctrl->curr_output_info.output[PRIMARY].format =
        ctrl->video_ctrl.main_img_format;
      ctrl->curr_output_info.output[PRIMARY].stream_type =
        MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
      ctrl->curr_output_info.output[PRIMARY].path =
        OUTPUT_TYPE_S;

      /* Configure secondary output for preview */
      ctrl->curr_output_info.output[SECONDARY].image_width =
        ctrl->dimInfo.display_width;
      ctrl->curr_output_info.output[SECONDARY].image_height =
        ctrl->dimInfo.display_height;
      ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
        ctrl->dimInfo.display_width;
      ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
        ctrl->dimInfo.display_width;
      ctrl->curr_output_info.output[SECONDARY].format =
        ctrl->video_ctrl.prev_format;
      ctrl->curr_output_info.output[SECONDARY].stream_type =
        MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
      ctrl->curr_output_info.output[SECONDARY].path =
        OUTPUT_TYPE_P;
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY |
                                              VFE_OUTPUT_SECONDARY;
      ctrl->curr_output_info.vfe_operation_mode =
        VFE_OUTPUTS_MAIN_AND_PREVIEW;
      break;
    case VFE_OP_MODE_JPEG_SNAPSHOT:
      CDBG_HIGH("%s:VFE_OP_MODE_JPEG_SNAPSHOT ", __func__);
      if (ctrl->dimInfo.thumbnail_width > ctrl->dimInfo.picture_width) {
        rc = -EINVAL;
        CDBG_ERROR("%s: rc %d: ERROR", __func__, rc);
      } else {
        ctrl->curr_output_info.output[PRIMARY].image_width =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].image_height =
          ctrl->dimInfo.picture_height;
        ctrl->curr_output_info.output[PRIMARY].plane[0].stride =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].plane[1].stride =
          ctrl->dimInfo.picture_width;
        ctrl->curr_output_info.output[PRIMARY].format =
          ctrl->dimInfo.main_img_format;
        ctrl->curr_output_info.output[PRIMARY].stream_type =
          MSM_V4L2_EXT_CAPTURE_MODE_MAIN;
        ctrl->curr_output_info.output[PRIMARY].path =
          OUTPUT_TYPE_S;

        ctrl->curr_output_info.output[SECONDARY].image_width =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].image_height =
          ctrl->dimInfo.thumbnail_height;
        ctrl->curr_output_info.output[SECONDARY].plane[0].stride =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].plane[1].stride =
          ctrl->dimInfo.thumbnail_width;
        ctrl->curr_output_info.output[SECONDARY].format =
          ctrl->dimInfo.thumb_format;
        ctrl->curr_output_info.output[SECONDARY].stream_type =
          MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL;
        ctrl->curr_output_info.output[SECONDARY].path =
          OUTPUT_TYPE_T;
        ctrl->curr_output_info.vfe_operation_mode =
          VFE_OUTPUTS_JPEG_AND_THUMB;
      }
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_PRIMARY |
                                              VFE_OUTPUT_SECONDARY;
      break;
    default:
      CDBG_HIGH("%s Invalid ops mode %d ",
                __func__, ctrl->ops_mode);
      ctrl->curr_output_info.vfe_ports_used = VFE_OUTPUT_INVALID;
      ctrl->curr_output_info.vfe_operation_mode = 0;
      ctrl->curr_output_info.num_output = 0;
      rc = -EINVAL;
      break;
  }
  CDBG_HIGH("%s: Ports Used %d, Op mode %d", __func__,
    ctrl->curr_output_info.vfe_ports_used,
    ctrl->curr_output_info.vfe_operation_mode);
  CDBG_HIGH("%s Current mode %d Full size streaming : %s",
            __func__, ctrl->ops_mode,
            ctrl->sensor_stream_fullsize ? "Enabled" : "Disabled");
  CDBG_HIGH("%s: Primary: %dx%d, extra_pad: %dx%d, Fmt: %d, Type: %d, Path: %d",
    __func__, ctrl->curr_output_info.output[PRIMARY].image_width,
    ctrl->curr_output_info.output[PRIMARY].image_height,
    ctrl->curr_output_info.output[PRIMARY].extra_pad_width,
    ctrl->curr_output_info.output[PRIMARY].extra_pad_height,
    ctrl->curr_output_info.output[PRIMARY].format,
    ctrl->curr_output_info.output[PRIMARY].stream_type,
    ctrl->curr_output_info.output[PRIMARY].path);
  CDBG_HIGH("%s: Secondary: %dx%d, extra_pad: %dx%d, Fmt: %d, Type: %d, Path: %d",
    __func__, ctrl->curr_output_info.output[SECONDARY].image_width,
    ctrl->curr_output_info.output[SECONDARY].image_height,
    ctrl->curr_output_info.output[SECONDARY].extra_pad_width,
    ctrl->curr_output_info.output[SECONDARY].extra_pad_height,
    ctrl->curr_output_info.output[SECONDARY].format,
    ctrl->curr_output_info.output[SECONDARY].stream_type,
    ctrl->curr_output_info.output[SECONDARY].path);

  return rc;
}
/*===========================================================================
 * FUNCTION    - config_CAMERA_START_JPEG_SNAPSHOT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_CAMERA_START_JPEG_SNAPSHOT(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mod_cmd_t cmd;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("received CAMERA_START_JPEG_SNAPSHOT!, ctrl->state = %d\n",
    ctrl->state);

  if (ctrl->state != CAMERA_STATE_IDLE) {
    CDBG_HIGH("%s: state is not correct ctrl->state = %d\n",
      __func__, ctrl->state);
    *cmdPending = FALSE;
    return -EINVAL;
  }

  /* reset isp */
  cmd.mod_cmd_ops = MOD_CMD_RESET;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_RESET failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  /* reset axi: */
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_RESET failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }
  /* reset camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_RESET failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  ctrl->state = CAMERA_STATE_SENT_RESET;
  *cmdPending = TRUE;
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_CAMERA_STOP_JPEG_SNAPSHOT -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_CAMERA_STOP_JPEG_SNAPSHOT(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mod_cmd_t cmd;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  ctrlCmd->status = CAM_CTRL_FAILED;

  CDBG("%s: ctrl->state = %d\n", __func__, ctrl->state);

  if (ctrl->state != CAMERA_STATE_STARTED) {
    /* issue vfe command irrespective of state*/
    CDBG_HIGH("config_CAMERA_STOP_JPEG_SNAPSHOT: ctrl->state = %d\n",
      ctrl->state);
    *cmdPending = FALSE;
    return -EINVAL;
  }

  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
          ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
          ISPIF_PROCESS_STOP_IMMEDIATELY, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ISPIF_PROCESS_STOP_IMMEDIATELY failed\n", __func__);
      return rc;
    }
  }

  /* stop isp */
  cmd.mod_cmd_ops = MOD_CMD_STOP;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_VFE)) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config VFE_STOP failed, rc = %d \n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  /* stop axi */
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_CMD_OPS, &cmd);
  if (0 != rc) {
    CDBG_ERROR("%s: config AXI_STOP failed %d\n", __func__, rc);
    *cmdPending = FALSE;
    return rc;
  }
  /* stop camif: */
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PROC_CMD_OPS, &cmd);
    if (0 != rc) {
      CDBG_ERROR("%s: config CAMIF_STOP failed %d\n", __func__, rc);
      *cmdPending = FALSE;
      return rc;
    }
  }
  ctrlCmd->status = CAM_CTRL_SUCCESS;
  ctrl->state = CAMERA_STATE_SENT_STOP;
  CDBG("%s: return ctrlCmd->status = %d \n",
    __func__, ctrlCmd->status);
  *cmdPending = TRUE;
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_MSM_V4L2_STREAM_ON -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_MSM_V4L2_STREAM_ON(void *parm1, void *parm2, int *cmdPending)
{
  int rc = -1, need_low_power_mode = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  v4l2_video_ctrl *pvideo_ctrl;
  ctrl->vfeMode = VFE_OP_MODE_INVALID;
  stats_proc_set_t set_param;
  frame_proc_set_t fp_set_param;
  ispif_set_t ispif_set;

  if (ctrlCmd->vnode_id >= V4L2_MAX_VIDEO) {
    *cmdPending = FALSE;
    /* out of range */
    return -EINVAL;
  }

  if (ctrl->state == CAMERA_STATE_IDLE) {
    if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
      ispif_set.data.session_lock.acquire = true;
      ispif_set.data.session_lock.vfe_id = 0;
      rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
                 ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
                 ISPIF_SESSION_LOCK, &ispif_set, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s: ispif_set_params failed %d\n", __func__, rc);
        *cmdPending = FALSE;
        return -1;
      }
    }
  }

  if ((ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) &&
    ctrl->afCtrl.af_enable) {
    ctrl->stats_proc_ctrl.intf.output.af_d.active = 0;
    /* propogate this change to stats proc */
    set_param.type = STATS_PROC_AF_TYPE;
    set_param.d.set_af.type = AF_STATE;
    set_param.d.set_af.d.af_state = 0;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type,
      &set_param, &(ctrl->stats_proc_ctrl.intf));
    if (rc < 0) {
      CDBG_ERROR("%s: FAILED to set AF_STATE to 0\n", __func__);
      *cmdPending = FALSE;
      rc = -1;
      goto end;
    }
  }
  CDBG("%s Calling Zoom proc zoom val = %d", __func__, ctrl->zoomCtrl.zoom_val);
  if(config_proc_zoom(ctrl, ctrl->zoomCtrl.zoom_val) < 0) {
    CDBG_ERROR("%s: config_proc_zoom at zoom level = %d err",
             __func__, ctrl->zoomCtrl.zoom_val);
    *cmdPending = FALSE;
    rc = -1;
    goto end;
  }
  pvideo_ctrl = &ctrl->video_ctrl;
  if (ctrl->comp_mask & (1 << MCTL_COMPID_FRAMEPROC)) {
    fp_set_param.type = FRAME_PROC_SHARE;
    fp_set_param.d.set_share.type = FRAME_PROC_SET_STREAM;
    fp_set_param.d.set_share.d.stream_type = ctrlCmd->stream_type;
    if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
      fp_set_param.type, &fp_set_param, &(fp_ctrl->intf)) < 0 ) {
      CDBG_ERROR("%s: Error setting stream type to Frameproc", __func__);
    }
  }
  CDBG("%s: OPS MODE %d", __func__, pvideo_ctrl->op_mode);
  if (pvideo_ctrl->op_mode == MSM_V4L2_CAM_OP_DEFAULT) {
    pvideo_ctrl->op_mode = MSM_V4L2_CAM_OP_VIDEO;
  }

  rc = config_CAMERA_SET_CHANNEL_STREAM(parm1, parm2);
  switch (pvideo_ctrl->op_mode) {
    case MSM_V4L2_CAM_OP_DEFAULT:
    case MSM_V4L2_CAM_OP_PREVIEW:
      rc = config_CAMERA_START_VIDEO(parm1, parm2, cmdPending);
      /* TBD: check error? */
      pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_P;
      if(ctrl->videoHint){
        ctrl->vfeMode = VFE_OP_MODE_VIDEO;
      } else {
        ctrl->vfeMode = VFE_OP_MODE_PREVIEW;
      }
      break;
    case MSM_V4L2_CAM_OP_VIDEO:
      if ((ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) ||
        (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_RDI)) {

         if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW)
           pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_P;
         else
           pvideo_ctrl->streamon_mask |= V4L2_DEV_STREAMON_BIT_RDI;

         if (ctrl->channel_stream_info == (STREAM_IMAGE|STREAM_RAW) &&
           pvideo_ctrl->streamon_mask != (V4L2_DEV_STRAEMON_BIT_P |
               V4L2_DEV_STREAMON_BIT_RDI)) {
           rc = TRUE;
           ctrlCmd->status = CAM_CTRL_SUCCESS;
           break;
         }
          /* User will set the low power mode. But for certain features
           * like HFR, we have to force the camcorder to run in low power
           * mode, i.e configure VFE to directly output Video and Preview
           * data. This function checks for this condition and updates the
           * flag need_low_power_mode accordingly. */
          rc = config_pp_need_low_power_mode((void *)ctrl,
                     pvideo_ctrl->op_mode, &need_low_power_mode);

          if(rc < 0) {
            CDBG_ERROR("%s: config_topology_is_low_power_camcorder err = %d", __func__, rc);
            *cmdPending = FALSE;
            goto end;
          }
          /* Low power mode should be enabled either if User sets
           * it explicitly or if the flag need_low_power_mode is set. */
          ctrl->enableLowPowerMode = ctrl->enableLowPowerMode ||
                                     need_low_power_mode;
          ctrl->vfeMode = VFE_OP_MODE_VIDEO;
          CDBG("%s: Done MSM_CAM_IOCTL_V4L2_EVT_NOTIFY to kernel\n", __func__);
          if (ctrl->enableLowPowerMode) {
            ctrl->sensor_stream_fullsize = 0;
            ctrl->video_dis.enable_dis = 0;
            CDBG("%s: disable full size liveshot and DIS for low power"
                  "camcorder", __func__);
          }
#ifndef VFE_2X
          if (config_decide_vfe_outputs(ctrl) < 0) {
              *cmdPending = FALSE;
              CDBG_ERROR("%s Error deciding vfe outputs ", __func__);
              rc = -EINVAL;
              goto end;
          }

          if ((pvideo_ctrl->streamon_mask & V4L2_DEV_STRAEMON_BIT_P)
             && (NULL == ctrl->p_client_ops)) {
            if (ctrl->videoHint) {
              CDBG("%s: set video mode to mctl_pp", __func__);
              rc = config_pp_setup_pp_topology(parm1, MSM_V4L2_CAM_OP_VIDEO,
                     ctrlCmd);
              if (rc < 0) {
                *cmdPending = FALSE;
                CDBG_ERROR("%s Error setting opmode to mctl_pp ", __func__);
                return -EINVAL;
              }
              rc = config_pp_acquire_hw(parm1, ctrlCmd);
              if (rc < 0) {
                *cmdPending = FALSE;
                CDBG_ERROR("%s Error acquiring hardware ", __func__);
                return -EBUSY;
              }
              if (!ctrl->enableLowPowerMode) {
                rc = config_update_stream_info(ctrl,
                       &ctrl->pp_node.strm_info);
                if (rc < 0) {
                  *cmdPending = FALSE;
                  CDBG_ERROR("%s Error updating mctl inst ", __func__);
                  return -EBUSY;
                }
              }

              CDBG_HIGH("%s Sending START CMD to VFE ", __func__);
              config_pp_send_stream_on(parm1, ctrlCmd);
            }
          } else if (ctrl->p_client_ops && ctrl->p_client_ops->update_params) {
            uint32_t inst_handle = 0;
            cam_stream_info_def_t strm_info;
            ctrl->p_client_ops->update_params(ctrl->p_client_ops->handle,
              CAM_CLIENT_UPDATE_INST_HANDLE, &inst_handle);
            strm_info.width = ctrl->dimInfo.display_width;
            strm_info.height = ctrl->dimInfo.display_height;
            strm_info.format = ctrl->dimInfo.prev_format;
            strm_info.image_mode = MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW;
            strm_info.inst_handle = inst_handle;
            rc = config_update_stream_info(ctrl, &strm_info);
            if (rc < 0) {
              *cmdPending = FALSE;
              CDBG_ERROR("%s Error acquiring hardware ", __func__);
              return -EBUSY;
            }
          }
#else
          if (config_decide_vfe2x_outputs(ctrl) < 0) {
            CDBG_ERROR("%s Error deciding vfe outputs ", __func__);
            rc = -EINVAL;
            goto end;
          }
#endif
          rc = config_CAMERA_START_VIDEO(parm1, parm2, cmdPending);
          CDBG("%s:config_CAMERA_START_VIDEO, rc = %d", __func__, rc);
            config_proc_face_detection_cmd(parm1, 0);
            config_proc_face_detection_cmd(parm1, ctrl->fd_mode);
          break;
      } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
        rc = config_pp_acquire_hw(parm1, ctrlCmd);
        if (rc < 0) {
          *cmdPending = FALSE;
          CDBG_ERROR("%s Error acquiring hardware ", __func__);
          return -EBUSY;
        }
        CDBG_HIGH("%s Sending START RECORDING CMD to VFE ", __func__);
        ctrl->state = CAMERA_STATE_STARTED;
        config_pp_send_stream_on(parm1, ctrlCmd);
        rc = config_CAMERA_START_RECORDING(parm1, parm2, cmdPending);
        pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_V;
        ctrl->vfeMode = VFE_OP_MODE_VIDEO;
        break;
      } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN) {
#ifndef VFE_2X
        CDBG_HIGH("%s Sending LIVESHOT CMD to VFE ", __func__);
        rc = config_CAMERA_START_LIVESHOT(parm1, parm2, cmdPending);
#endif
        pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_S;
        ctrl->vfeMode = VFE_OP_MODE_VIDEO;
        break;
      } else {
        *cmdPending = FALSE;
        CDBG_ERROR("%s Invalid stream type. ", __func__);
        rc = -EINVAL;
        break;
      }
    case MSM_V4L2_CAM_OP_CAPTURE:
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL)
        pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_T;
      else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN)
        pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_S;
      if (pvideo_ctrl->streamon_mask ==
        (V4L2_DEV_STRAEMON_BIT_T|V4L2_DEV_STRAEMON_BIT_S)) {
        ctrl->vfeMode = VFE_OP_MODE_SNAPSHOT;
        /* Based on vfe op mode, decide VFE ouput dimension.
         * For eg: If video > preview, configure with video
         * dimension, else configure preview dimension. */
        if (config_decide_vfe_outputs(ctrl) < 0) {
          CDBG_ERROR("%s Error deciding vfe outputs ", __func__);
          *cmdPending = FALSE;
          rc = -1;
          goto end;
        }

        rc = config_CAMERA_START_SNAPSHOT(parm1, parm2, cmdPending);
      } else {
        /* ignore the stream on and wait for the second streamon */
        rc = 0;
        *cmdPending = FALSE;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      }
      break;
    case MSM_V4L2_CAM_OP_ZSL:
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW)
        pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_P;
      else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL)
        pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_T;
      else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN)
        pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_S;

      if (pvideo_ctrl->streamon_mask == (V4L2_DEV_STRAEMON_BIT_P |
        V4L2_DEV_STRAEMON_BIT_T | V4L2_DEV_STRAEMON_BIT_S)) {
        ctrl->vfeMode = VFE_OP_MODE_ZSL;
        /* Based on vfe op mode, decide VFE ouput dimension.
         * For eg: If video > preview, configure with video
         * dimension, else configure preview dimension. */
        if (config_decide_vfe_outputs(ctrl) < 0) {
          *cmdPending = FALSE;
          CDBG_ERROR("%s Error deciding vfe outputs ", __func__);
          rc = -1;
          goto end;
        }
        if (ctrl->fd_mode > 0) {
          config_proc_face_detection_cmd(parm1, 0);
          config_proc_face_detection_cmd(parm1, ctrl->fd_mode);
        }
        rc = config_CAMERA_START_ZSL(parm1, parm2, cmdPending);
      } else {
        /* ignore the stream on and wait till all the streamons are received */
        rc = 0;
        *cmdPending = FALSE;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      }
      break;
    case MSM_V4L2_CAM_OP_RAW:
      ctrl->vfeMode = VFE_OP_MODE_RAW_SNAPSHOT;
      /* Based on vfe op mode, decide VFE ouput dimension.
       * For eg: If video > preview, configure with video
       * dimension, else configure preview dimension. */
      if (config_decide_vfe_outputs(ctrl) < 0) {
        CDBG_ERROR("%s Error deciding vfe outputs ", __func__);
        rc = -1;
        goto end;
      }
      rc = config_CAMERA_START_RAW_SNAPSHOT(parm1, parm2, cmdPending);
      pvideo_ctrl->streamon_mask |= V4L2_DEV_STREAMON_BIT_R;
      break;
    case MSM_V4L2_CAM_OP_JPEG_CAPTURE:
      CDBG("%s:MSM_V4L2_CAM_OP_JPEG_CAPTURE", __func__);
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL)
        pvideo_ctrl->streamon_mask |= V4L2_DEV_STRAEMON_BIT_T;
      if (pvideo_ctrl->streamon_mask == V4L2_DEV_STRAEMON_BIT_T) {
        ctrl->vfeMode = VFE_OP_MODE_JPEG_SNAPSHOT;
        /* Based on vfe op mode, decide VFE ouput dimension.
         * For eg: If video > preview, configure with video
         * dimension, else configure preview dimension. */
        if (config_decide_vfe_outputs(ctrl) < 0) {
          *cmdPending = FALSE;
          CDBG_ERROR("%s Error deciding vfe outputs ", __func__);
          rc = -1;
          goto end;
        }
        rc = config_CAMERA_START_JPEG_SNAPSHOT(parm1, parm2, cmdPending);
      } else {
        /* ignore the stream on and wait till all the streamons are received */
        rc = 0;
        *cmdPending = FALSE;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      }
      break;
    default:
      break;
  }
end:
  if (rc < 0 || ctrl->state == CAMERA_STATE_IDLE) {
    if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
      ispif_set.data.session_lock.acquire = false;
      ispif_set.data.session_lock.vfe_id = 0;
      ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
               ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
               ISPIF_SESSION_LOCK, &ispif_set, NULL);
    }
  }
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_update_stream_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_update_stream_info(void *cctrl, void *data)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  cam_stream_info_def_t *strm_info = (cam_stream_info_def_t *)data;
  int inst_idx = -1;

  inst_idx = GET_VIDEO_INST_IDX(strm_info->inst_handle);
  switch(strm_info->image_mode) {
    case MSM_V4L2_EXT_CAPTURE_MODE_AEC:
    case MSM_V4L2_EXT_CAPTURE_MODE_AWB:
    case MSM_V4L2_EXT_CAPTURE_MODE_AF:
    case MSM_V4L2_EXT_CAPTURE_MODE_IHIST:
    case MSM_V4L2_EXT_CAPTURE_MODE_CS:
    case MSM_V4L2_EXT_CAPTURE_MODE_RS:
      if (inst_idx >= 0 && inst_idx <= MSM_MAX_DEV_INST) {
        ctrl->video_ctrl.strm_info.stats[inst_idx].width =
          strm_info->width;
        ctrl->video_ctrl.strm_info.stats[inst_idx].height =
          strm_info->height;
        ctrl->video_ctrl.strm_info.stats[inst_idx].format =
          strm_info->format;
        ctrl->video_ctrl.strm_info.stats[inst_idx].inst_handle =
          strm_info->inst_handle;
        ctrl->video_ctrl.strm_info.stats[inst_idx].image_mode =
          strm_info->image_mode;

        CDBG_HIGH("%s Storing stream parameters for video inst %d as :"
             " width = %d, height %d, format = %d inst_handle = %x ", __func__,
             inst_idx, ctrl->video_ctrl.strm_info.stats[inst_idx].width,
             ctrl->video_ctrl.strm_info.stats[inst_idx].height,
             ctrl->video_ctrl.strm_info.stats[inst_idx].format,
             ctrl->video_ctrl.strm_info.stats[inst_idx].inst_handle);
      } else
        CDBG_ERROR("%s: stats instance idx out of bound, handle = 0x%x",
          __func__, strm_info->inst_handle);
      break;
    default:
      if (inst_idx >= 0 && inst_idx <= MSM_MAX_DEV_INST) {
      ctrl->video_ctrl.strm_info.user[inst_idx].format = strm_info->format;
      ctrl->video_ctrl.strm_info.user[inst_idx].width  = strm_info->width;
      ctrl->video_ctrl.strm_info.user[inst_idx].height = strm_info->height;
      ctrl->video_ctrl.strm_info.user[inst_idx].image_mode =
        strm_info->image_mode;
      ctrl->video_ctrl.strm_info.user[inst_idx].inst_handle =
        strm_info->inst_handle;
      CDBG_HIGH("%s Storing stream parameters for video inst %d as :"
         " width = %d, height %d, format = %d inst_handle = %x cid = %d",
         __func__,
         inst_idx, ctrl->video_ctrl.strm_info.user[inst_idx].width,
         ctrl->video_ctrl.strm_info.user[inst_idx].height,
         ctrl->video_ctrl.strm_info.user[inst_idx].format,
         ctrl->video_ctrl.strm_info.user[inst_idx].inst_handle,
         ctrl->video_ctrl.strm_info.user[inst_idx].cid_val);
    } else {
      inst_idx = GET_MCTLPP_INST_IDX(strm_info->inst_handle);
      if (inst_idx >= 0 && inst_idx <= MSM_MAX_DEV_INST) {
        ctrl->video_ctrl.strm_info.mctl[inst_idx].format = strm_info->format;
        ctrl->video_ctrl.strm_info.mctl[inst_idx].width  = strm_info->width;
        ctrl->video_ctrl.strm_info.mctl[inst_idx].height = strm_info->height;
        ctrl->video_ctrl.strm_info.mctl[inst_idx].image_mode =
          strm_info->image_mode;
        ctrl->video_ctrl.strm_info.mctl[inst_idx].inst_handle =
          strm_info->inst_handle;
        CDBG_HIGH("%s Storing stream parameters for mctl pp inst %d as :"
           " width = %d, height %d, format = %d inst_handle = %x cid = %d",
           __func__,
           inst_idx, ctrl->video_ctrl.strm_info.mctl[inst_idx].width,
           ctrl->video_ctrl.strm_info.mctl[inst_idx].height,
           ctrl->video_ctrl.strm_info.mctl[inst_idx].format,
           ctrl->video_ctrl.strm_info.mctl[inst_idx].inst_handle,
           ctrl->video_ctrl.strm_info.mctl[inst_idx].cid_val);
      } else {
        CDBG_ERROR("%s Invalid video inst idx %d ", __func__, inst_idx);
        return -EINVAL;
      }
    }
    break;
  }
  return 0;
}

/*===========================================================================
 * FUNCTION    - config_MSM_V4L2_VID_CAP_TYPE -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_MSM_V4L2_VID_CAP_TYPE(void *parm1, void *parm2, int *cmdPending)
{
  int8_t rc = 0;

  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  struct img_plane_info *plane_info = ctrlCmd->value;
  cam_stream_info_def_t strm_info;

  rc = config_plane_info((void*)ctrl, (void *)plane_info);
  if (!rc) {
    strm_info.width = plane_info->width;
    strm_info.height = plane_info->height;
    strm_info.format = config_proc_v4l2fmt_to_camfmt(plane_info->pixelformat);
    strm_info.image_mode = GET_IMG_MODE(plane_info->inst_handle);
    strm_info.inst_handle = plane_info->inst_handle;
    rc = config_update_stream_info(ctrl, &strm_info);
  } else {
    CDBG_ERROR("%s Error setting plane info. Stream info will not be saved.",
      __func__);
  }

  ctrlCmd->status = (rc >= 0) ? CAM_CTRL_SUCCESS : CAM_CTRL_FAILED;
  *cmdPending = FALSE;
  CDBG("%s rc=%d\n", __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_MSM_V4L2_STREAM_OFF -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_MSM_V4L2_STREAM_OFF(void *parm1, void *parm2, int *cmdPending)
{
  int rc = -1;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;
  v4l2_video_ctrl *pvideo_ctrl;
  ispif_set_t ispif_set;

  if (ctrlCmd->vnode_id >= V4L2_MAX_VIDEO) {
    /* out of range */
    return rc;
  }
  if (ctrl->state != CAMERA_STATE_SENT_STOP) {
    if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
      ispif_set.data.session_lock.acquire = true;
      ispif_set.data.session_lock.vfe_id = 0;
      rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
                 ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
                 ISPIF_SESSION_LOCK, &ispif_set, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s: ispif_set_params failed %d\n", __func__, rc);
        return -1;
      }
    }
  }
  pvideo_ctrl = &ctrl->video_ctrl;
  CDBG("%s: op mode %d\n", __func__,  pvideo_ctrl->op_mode);
  switch (pvideo_ctrl->op_mode) {
    case MSM_V4L2_CAM_OP_DEFAULT:
    case MSM_V4L2_CAM_OP_PREVIEW:
      if (pvideo_ctrl->streamon_mask) {
        rc = config_CAMERA_STOP_VIDEO(parm1, parm2, cmdPending);
        pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_P;
      } else {
        *cmdPending = FALSE;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
        rc = 0;  /* already stream off */
      }
      break;
    case MSM_V4L2_CAM_OP_VIDEO:
      if (pvideo_ctrl->streamon_mask) {
        if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW ||
          ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_RDI) {
          CDBG("%s: MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW", __func__);
          if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW)
            pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_P;
          else
            pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STREAMON_BIT_RDI;

          if (ctrl->channel_stream_info == (STREAM_IMAGE|STREAM_RAW) &&
            ((pvideo_ctrl->streamon_mask ==
           (V4L2_DEV_STRAEMON_BIT_P | V4L2_DEV_STREAMON_BIT_RDI)) ||
           !(ctrl->state == CAMERA_STATE_STARTED))) {
            rc = TRUE;
            ctrlCmd->status = CAM_CTRL_SUCCESS;
            break;
          }
          rc = config_CAMERA_STOP_VIDEO(parm1, parm2, cmdPending);
#ifdef VFE_2X
          if (ctrl->vfeMode == VFE_OP_MODE_VIDEO) {
            CDBG_HIGH("%s: Sending Liveshot Stop", __func__);
            rc = config_CAMERA_STOP_LIVESHOT(parm1, parm2, cmdPending);
          }
#endif
        } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_VIDEO) {
          CDBG("%s: MSM_V4L2_EXT_CAPTURE_MODE_VIDEO", __func__);
          config_pp_send_stream_off(parm1, ctrlCmd);
          rc = config_CAMERA_STOP_RECORDING(parm1, parm2, cmdPending);
          pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_V;
        } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN) {
#ifndef VFE_2X
          CDBG_HIGH("%s: Sending Liveshot StreamOff", __func__);
          rc = config_CAMERA_STOP_LIVESHOT(parm1, parm2, cmdPending);
#else
          ctrlCmd->status = CAM_CTRL_SUCCESS;
          *cmdPending = FALSE;
          rc = 0;
#endif
          pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_S;
        }
      }
      break;
    case MSM_V4L2_CAM_OP_CAPTURE:
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL)
        pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_T;
      else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN)
        pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_S;
      if (!(pvideo_ctrl->streamon_mask ==
           (V4L2_DEV_STRAEMON_BIT_T | V4L2_DEV_STRAEMON_BIT_S)) &&
           (ctrl->state == CAMERA_STATE_STARTED)) {
        CDBG_HIGH("%s Issuing STOP cmd to VFE ", __func__);
        rc = config_CAMERA_STOP_SNAPSHOT(parm1, parm2, cmdPending);
      } else {
        *cmdPending = FALSE;
        rc = 0;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      }
      break;
    case MSM_V4L2_CAM_OP_JPEG_CAPTURE:
      if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL)
        pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_T;
      if (!(pvideo_ctrl->streamon_mask ==
           (V4L2_DEV_STRAEMON_BIT_T)) &&
           (ctrl->state == CAMERA_STATE_STARTED)) {
        CDBG("%s Issuing STOP cmd to VFE ", __func__);
        rc = config_CAMERA_STOP_JPEG_SNAPSHOT(parm1, parm2, cmdPending);
      } else {
        *cmdPending = FALSE;
        rc = 0;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      }
      break;
    case MSM_V4L2_CAM_OP_ZSL:
      if (pvideo_ctrl->streamon_mask) {
        if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_PREVIEW) {
          pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_P;
        } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_THUMBNAIL) {
          pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_T;
        } else if (ctrlCmd->stream_type == MSM_V4L2_EXT_CAPTURE_MODE_MAIN) {
          pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STRAEMON_BIT_S;
        }
      }
      if (!(pvideo_ctrl->streamon_mask == (V4L2_DEV_STRAEMON_BIT_P |
        V4L2_DEV_STRAEMON_BIT_T | V4L2_DEV_STRAEMON_BIT_S)) &&
          (ctrl->state == CAMERA_STATE_STARTED))
        rc = config_CAMERA_STOP_ZSL(parm1, parm2, cmdPending);
      else {
        *cmdPending = FALSE;
        rc = 0;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      }
      break;
    case MSM_V4L2_CAM_OP_RAW:
      pvideo_ctrl->streamon_mask &= ~V4L2_DEV_STREAMON_BIT_R;
      if (!(pvideo_ctrl->streamon_mask ==
           (V4L2_DEV_STREAMON_BIT_R)) &&
           (ctrl->state == CAMERA_STATE_STARTED)) {
        CDBG_HIGH("%s Issuing STOP cmd to VFE ", __func__);
        rc = config_CAMERA_STOP_SNAPSHOT(parm1, parm2, cmdPending);
      } else {
        *cmdPending = FALSE;
        rc = 0;
        ctrlCmd->status = CAM_CTRL_SUCCESS;
      }
      break;
    default:
      break;
  }
  if (rc < 0 || ctrl->state != CAMERA_STATE_SENT_STOP) {
    if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
      ispif_set.data.session_lock.acquire = false;
      ispif_set.data.session_lock.vfe_id = 0;
      ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
               ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
               ISPIF_SESSION_LOCK, &ispif_set, NULL);
    }
  }
  return rc;
}

/*===========================================================================
 * FUNCTION    - config_write_sensor_gain -
 *
 * DESCRIPTION: Write the gain to the sensor interface.
 *==========================================================================*/
static int config_write_sensor_gain_snap(void *cctrl)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_interface_input_t *sp_input = &(sp_ctrl->intf.input);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);
  sensor_set_t sensor_set;
  stats_proc_set_t sp_set_param;
  sensor_get_t sensor_get;
  int sensor_update_ok, rc = 0;

  sensor_update_ok = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
                           ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
                           SENSOR_SET_EXPOSURE, NULL, NULL);
  if (sensor_update_ok < 0) {
    CDBG_ERROR("%s Sensor gain update failed ", __func__);
    rc = -1;
    sensor_update_ok = FALSE;
  } else
    sensor_update_ok = TRUE;

  CDBG("%s: Sensor update=%d \n",__func__, sensor_update_ok);

  sp_set_param.type = STATS_PROC_AEC_TYPE;
  sp_set_param.d.set_aec.type = AEC_SOF;
  sp_set_param.d.set_aec.d.aec_sensor_update_ok = sensor_update_ok;
  if((ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
            ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
            sp_set_param.type, &sp_set_param,
            &(sp_ctrl->intf))) < 0) {
    CDBG_ERROR("%s Stats proc set params failed ", __func__);
    rc =-1;
  }

  if (ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
	  ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	  SENSOR_GET_DIGITAL_GAIN, &sensor_get, sizeof(sensor_get))) {
    CDBG_ERROR("%s Error getting digital gain from sensor ", __func__);
    rc =-1;
  }
  sp_ctrl->digital_gain = sensor_get.data.aec_info.digital_gain;

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_CUR_FPS, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s Error getting current fps from sensor ", __func__);
    rc =-1;
  }
  sp_ctrl->intf.input.sensor_info.current_fps = sensor_get.data.fps;

  CDBG("%s: Digital Gain: %f Current FPS: %d", __func__,
    sp_ctrl->digital_gain, sp_ctrl->intf.input.sensor_info.current_fps);

  return rc;
}

/*===========================================================================
 * FUNCTION    - config_do_aec_snap-
 *
 * DESCRIPTION:
 *==========================================================================*/
static void config_do_aec_snap(void *cctrl, void *vfe_parm)
{
  int rc;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  vfe_stats_output_t *vfe_out = (vfe_stats_output_t *)vfe_parm;
  stats_proc_interface_input_t *sp_input = &(sp_ctrl->intf.input);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);

  CDBG("%s: numReg = %d, num_pixels_per_region_aec = %d\n", __func__,
    sp_input->mctl_info.numRegions, sp_input->mctl_info.pixelsPerRegion);

  if (sp_input->mctl_info.opt_state == STATS_PROC_STATE_SNAPSHOT) {
    sp_ctrl->sof_update_needed = FALSE;
  }

  sp_input->mctl_info.type = STATS_PROC_AEC_TYPE;
  if((ctrl->comp_ops[MCTL_COMPID_STATSPROC].process(
            ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
            sp_input->mctl_info.type, &(sp_ctrl->intf))) == 0) {
    if (sp_input->mctl_info.opt_state == STATS_PROC_STATE_SNAPSHOT) {
      if (ctrl->hdrCtrl.exp_bracketing_enable || ctrl->hdrCtrl.hdr_enable) {
        prepare_hdr(ctrl);
        rc = hdr_calc_sensor_gain_upon_sof(ctrl);
      if (rc < 0)
        CDBG_ERROR("%s: HDR sensor gain failed\n", __func__);
   } else
      config_write_sensor_gain_snap(ctrl);
    } else {
      /* Update gain on next SOF */
      sp_ctrl->sof_update_needed = TRUE;
    }

    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_AEC_PARAMS,
               NULL, NULL);
    if (rc != VFE_SUCCESS)
      CDBG_HIGH("%s VFE Set AEC params failed ", __func__);

    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_FLASH_PARMS,
               NULL, NULL);
    if (rc != VFE_SUCCESS)
      CDBG_HIGH("%s VFE Set FLASH params failed ", __func__);

    /*camif config - begin*/
    camera_status_t status = CAMERA_STATUS_SUCCESS;
    camif_input_t camif_input;
    /* hard coded for now. Need to get it from QCamServer */
    camif_input.obj_idx = 0;
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
               ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
               CAMIF_PARAMS_ADD_OBJ_ID, (void *)&camif_input, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s CAMIF_PARAMS_ADD_OBJ_ID failed %d ", __func__, rc);
    }

    camif_input.obj_idx = 0;
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
               ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
               CAMIF_PARAMS_STROBE_INFO, (void *)&camif_input, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s CAMIF_PARAMS_STROBE_INFO failed %d ", __func__, rc);
      goto error;
    }
    /*camif config - end*/

    float dig_gain = sp_ctrl->digital_gain;
    CDBG("%s VFE set sensor digital gain %f ", __func__, dig_gain);
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle,
               VFE_SET_SENSOR_DIG_GAIN, (void *)&dig_gain, NULL);
    if (rc != VFE_SUCCESS)
      CDBG_HIGH("%s VFE set sensor digital gain failed ", __func__);
  } else
    CDBG_HIGH("%s Error processing stats ", __func__);
error:
  CDBG("%s X", __func__);
} /* config_do_aec_snap */
/*===========================================================================
 * FUNCTION    - config_do_awb -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void config_do_awb_snap(void *cctrl)
{
  int rc = TRUE;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_interface_input_t *sp_input = &(sp_ctrl->intf.input);

  sp_input->mctl_info.type = STATS_PROC_AWB_TYPE;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].process(
             ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
             sp_input->mctl_info.type, &(sp_ctrl->intf));
  if (rc < 0)
    CDBG_ERROR("%s Stats processing failed for Whitebalance ", __func__);

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_AWB_PARMS, NULL, NULL);
  if (rc != VFE_SUCCESS)
    CDBG_HIGH("%s VFE SET AWB params failed ", __func__);
}

/*===========================================================================
 * FUNCTION    - config_do_3a_snap-
 *
 * DESCRIPTION:
 *==========================================================================*/
static void config_do_3a_snap(void *cctrl, void *vfe_parm)
{
  config_do_aec_snap(cctrl,vfe_parm);
  config_do_awb_snap(cctrl);
}
/*===========================================================================
 * FUNCTION    - config_prep_frame_proc-
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_prep_frame_proc(void *cctrl)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;
  frame_proc_ctrl_t *fp_ctrl = &(ctrl->frame_proc_ctrl);
  frame_proc_set_t fp_set_param;
  vfe_pp_params_t vfe_pp_params;
  int rc = 0;

  if (fp_ctrl->intf.output.wd_d.denoise_enable || fp_ctrl->intf.output.hdr_d.hdr_enable) {
    fp_ctrl->intf.input.mctl_info.picture_dim.width =
      ctrl->dimInfo.picture_width;
    fp_ctrl->intf.input.mctl_info.picture_dim.height =
      ctrl->dimInfo.picture_height;
    fp_ctrl->intf.input.mctl_info.display_dim.width =
      ctrl->dimInfo.display_width;
    fp_ctrl->intf.input.mctl_info.display_dim.height =
      ctrl->dimInfo.display_height;

    fp_ctrl->intf.input.statsproc_info.aec_d.snap.real_gain
      = ctrl->stats_proc_ctrl.intf.output.aec_d.snap.real_gain;
    fp_ctrl->intf.input.statsproc_info.awb_d.snapshot_wb.g_gain
      = ctrl->stats_proc_ctrl.intf.output.awb_d.snapshot_wb.g_gain;
    fp_ctrl->intf.input.statsproc_info.aec_d.lux_idx
      = ctrl->stats_proc_ctrl.intf.output.aec_d.lux_idx;
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
  }
  if (fp_ctrl->intf.output.wd_d.denoise_enable) {
    fp_set_param.type = FRAME_PROC_WAVELET_DENOISE;
    fp_set_param.d.set_wd.type = WAVELET_DENOISE_CALIBRATE;
    if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
             ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
             fp_set_param.type, &fp_set_param, &(fp_ctrl->intf)) < 0 ) {
      CDBG_ERROR("%s Error while calibrating Wavelet Denoise", __func__);
      return -1;
    }
  }
  if (fp_ctrl->intf.output.hdr_d.hdr_enable) {
    fp_set_param.type = FRAME_PROC_HDR;
    fp_set_param.d.set_hdr.type = FRAME_PROC_HDR_HW_INFO;
    if(ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].set_params(
             ctrl->comp_ops[MCTL_COMPID_FRAMEPROC].handle,
             fp_set_param.type, &fp_set_param, &(fp_ctrl->intf)) < 0 ) {
      CDBG_ERROR("%s Error while setting HDR info", __func__);
      return -1;
    }
  }
  return 0;
}


/*===========================================================================
 * FUNCTION    - config_update_chromatix_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int8_t config_update_chromatix_params(mctl_config_ctrl_t *ctrl)
{
  int rc = 0;
  chromatix_parms_type *chromatix_ptr = ctrl->chromatix_ptr;
  if ((TRUE == ctrl->conv_3a_info_set) && (NULL != chromatix_ptr)) {
    CDBG_HIGH("%s:%d] type %d", __func__, __LINE__,
      ctrl->conv_3a_info.conv_type);
    if (ctrl->conv_3a_info.conv_type == CAM3A_CONV_TYPE_SLOW)
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_CAMCORDER;
    else
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_PREVIEW;

    if (ctrl->conv_3a_info.force) {
      chromatix_ptr->aggressiveness_values =
        ctrl->conv_3a_info.aec_conv_speed;
      chromatix_ptr->luma_tolerance = ctrl->conv_3a_info.aec_luma_tolerance;
      chromatix_ptr->awb_aggressiveness =
        ctrl->conv_3a_info.awb_aggressiveness;
      CDBG_HIGH("%s:%d] aec_aggressive %f luma_tol %ld awb_aggr %d",
        __func__, __LINE__,
        chromatix_ptr->aggressiveness_values,
        chromatix_ptr->luma_tolerance,
        chromatix_ptr->awb_aggressiveness);
    }
  }

  return TRUE;
}/*config_update_chromatix_params*/

static void config_update_inst_handles(void *cctrl)
{
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)cctrl;

  /* Depending on the op mode, decide whether to get the
   * instance handle from the user/mctl strm_info structure.
   * get the inst handle and store it in the output[] entry
   * so that when the AXI is configured for that output,
   * it can pass the handle to VFE. It is used when VFE requests
   * for a free buffer from kernel mctl. The instance handle
   * is a bitmask as described in msm.h, contains the image mode,
   * mctl pp/video inst idx information.*/
  ctrl->curr_output_info.output[PRIMARY].inst_handle =
        config_get_inst_handle(&ctrl->video_ctrl.strm_info, STRM_INFO_USER,
          ctrl->curr_output_info.output[PRIMARY].stream_type);
  ctrl->curr_output_info.output[SECONDARY].inst_handle =
        config_get_inst_handle(&ctrl->video_ctrl.strm_info, STRM_INFO_USER,
          ctrl->curr_output_info.output[SECONDARY].stream_type);
  ctrl->curr_output_info.output[TERTIARY1].inst_handle =
        config_get_inst_handle(&ctrl->video_ctrl.strm_info, STRM_INFO_USER,
          ctrl->curr_output_info.output[TERTIARY1].stream_type);
  ctrl->curr_output_info.output[TERTIARY2].inst_handle =
        config_get_inst_handle(&ctrl->video_ctrl.strm_info, STRM_INFO_USER,
          ctrl->curr_output_info.output[TERTIARY2].stream_type);

  if (ctrl->vfeMode == VFE_OP_MODE_VIDEO) {
    if (ctrl->videoHint && !ctrl->enableLowPowerMode) {
      /* In this case, the VFE buffers for SECONDARY path are
       * allocated from mctl pp node.*/
      ctrl->curr_output_info.output[SECONDARY].inst_handle =
            config_get_inst_handle(&ctrl->video_ctrl.strm_info, STRM_INFO_MCTL,
              ctrl->curr_output_info.output[SECONDARY].stream_type);
    }
  }
  if (ctrl->p_client_ops) {
    ctrl->curr_output_info.output[PRIMARY].inst_handle =
        config_get_inst_handle(&ctrl->video_ctrl.strm_info, STRM_INFO_MCTL,
          ctrl->curr_output_info.output[PRIMARY].stream_type);
  }
  CDBG_HIGH("%s Updated the inst handles as %x, %x, %x, %x ", __func__,
    ctrl->curr_output_info.output[PRIMARY].inst_handle,
    ctrl->curr_output_info.output[SECONDARY].inst_handle,
    ctrl->curr_output_info.output[TERTIARY1].inst_handle,
    ctrl->curr_output_info.output[TERTIARY2].inst_handle);
}

/*===========================================================================
 * FUNCTION    - config_MSG_ID_RESET_ACK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_MSG_ID_RESET_ACK(void *parm1, void *parm2)
{
  int sensor_update_ok = 0, rc = 0, ispif_status = 0;
  vfe_op_mode_t op_mode;
  sensor_set_t sensor_set;
  sensor_get_t sensor_get;
  axi_set_t axi_set;
  camif_input_t camif_input;
  camif_output_t camif_output;
  camera_status_t status = CAMERA_STATUS_SUCCESS;
  int run_3a = FALSE;
  int capture_cnt;
  mod_cmd_t cmd;
  axi_config_t axi_cfg;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);
  int chromatix_type_change = 0;
  ispif_set_t ispif_set;
  csi_set_t csi_set;

  if (ctrl->hdrCtrl.exp_bracketing_enable || ctrl->hdrCtrl.hdr_enable)
    capture_cnt = ctrl->hdrCtrl.total_frames;
  else
    capture_cnt = 1;
  ctrl->hdrCtrl.current_snapshot_count = 0;
  CDBG("%s: state = %d, vfe_mode = %d\n", __func__, ctrl->state, ctrl->vfeMode);

  if (ctrl->state != CAMERA_STATE_SENT_RESET &&
    ctrl->state != CAMERA_STATE_ERROR) {
    CDBG_HIGH("VFE_RESET_ACK illegal ctrl->state %d\n", ctrl->state);
    rc = -EINVAL;
    goto end;
  }

  ctrl->state = CAMERA_STATE_RESET;

  switch (ctrl->vfeMode) {
    case VFE_OP_MODE_PREVIEW:
      sensor_set.data.mode = SENSOR_MODE_PREVIEW;
      op_mode = VFE_OP_MODE_PREVIEW;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_mode =
        STATS_PROC_MODE_2D_NORM;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_PREVIEW;
      ctrl->ops_mode = CAM_OP_MODE_PREVIEW;
      ctrl->frame_proc_ctrl.intf.input.mctl_info.opt_mode =
        FRAME_PROC_PREVIEW;
      break;

    case VFE_OP_MODE_SNAPSHOT:
      sensor_set.data.mode = SENSOR_MODE_SNAPSHOT;
      op_mode = VFE_OP_MODE_SNAPSHOT;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_mode =
        STATS_PROC_MODE_2D_NORM;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_SNAPSHOT;
      ctrl->ops_mode = CAM_OP_MODE_SNAPSHOT;
      run_3a = TRUE;
      ctrl->frame_proc_ctrl.intf.input.mctl_info.opt_mode =
        FRAME_PROC_SNAPSHOT;
      break;

    case VFE_OP_MODE_RAW_SNAPSHOT:
      sensor_set.data.mode = SENSOR_MODE_RAW_SNAPSHOT;
      op_mode = VFE_OP_MODE_RAW_SNAPSHOT;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_mode =
        STATS_PROC_MODE_2D_NORM;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_SNAPSHOT;
      ctrl->ops_mode = CAM_OP_MODE_RAW_SNAPSHOT;
      run_3a = TRUE;
      ctrl->frame_proc_ctrl.intf.input.mctl_info.opt_mode =
        FRAME_PROC_SNAPSHOT;
      break;

    case VFE_OP_MODE_VIDEO:
      if (ctrl->sensor_stream_fullsize) {
        CDBG("%s: liveshot: set sensor: zsl\n", __func__);
        sensor_set.data.mode = SENSOR_MODE_ZSL;
        ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_mode =
          STATS_PROC_MODE_2D_ZSL;
      } else {
        /* set the sensor mode as VIDEO only if we are in
         * camcorder mode. */
        if (ctrl->videoHint)
          sensor_set.data.mode = SENSOR_MODE_VIDEO;
        else
          sensor_set.data.mode = SENSOR_MODE_PREVIEW;
        ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_mode =
          STATS_PROC_MODE_2D_NORM;
      }
      op_mode = VFE_OP_MODE_VIDEO;
#ifdef VFE_2X
      ctrl->ops_mode = CAM_OP_MODE_ZSL;
#else
      ctrl->ops_mode = CAM_OP_MODE_VIDEO;
#endif
      ctrl->frame_proc_ctrl.intf.input.mctl_info.opt_mode =
        FRAME_PROC_PREVIEW;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_PREVIEW;
      break;

    case VFE_OP_MODE_ZSL:
      sensor_set.data.mode = SENSOR_MODE_ZSL;
      op_mode = VFE_OP_MODE_ZSL;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_mode =
        STATS_PROC_MODE_2D_ZSL;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_PREVIEW;
      ctrl->ops_mode = CAM_OP_MODE_ZSL;
      break;

    case VFE_OP_MODE_JPEG_SNAPSHOT:
      sensor_set.data.mode = SENSOR_MODE_SNAPSHOT;
      op_mode = VFE_OP_MODE_JPEG_SNAPSHOT;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_mode =
        STATS_PROC_MODE_2D_NORM;
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_SNAPSHOT;
      ctrl->ops_mode = CAM_OP_MODE_JPEG_SNAPSHOT;
      break;

    default:
      ctrl->stats_proc_ctrl.intf.input.mctl_info.opt_state =
        STATS_PROC_STATE_INVALID;
      ctrl->ops_mode = CAM_OP_MODE_INVALID;
      CDBG_ERROR("%s: Invalid mode", __func__);
      rc = -EINVAL;
      break;
  }
  if (rc < 0) {
    CDBG_ERROR("%s: %d: ERROR\n", __func__, __LINE__);
    goto end;
  }
  CDBG("%s: vfe_output_mode = 0x%x", __func__,
    ctrl->curr_output_info.vfe_operation_mode);
  if (ioctl(ctrl->camfd, MSM_CAM_IOCTL_SET_VFE_OUTPUT_TYPE,
            &(ctrl->curr_output_info.vfe_operation_mode)) < 0) {
    CDBG_ERROR("%s: MSM_CAM_IOCTL_SET_VFE_OUTPUT_TYPE failed...\n", __func__);
    rc = -EINVAL;
    goto end;
  }

#ifndef VFE_2X
  /* Now that the VFE configuration is decided, update the instance
   * handles corresponding to each of the vfe outputs so that it can
   * be passed during AXI configuration.*/
  config_update_inst_handles(ctrl);
#endif

  ctrl->sensor_op_mode = sensor_set.data.mode;
  CDBG("%s: vfeMode = %d\n", __func__, ctrl->vfeMode);
  CDBG("%s: line = %d\n", __func__, __LINE__);
#ifndef VFE_2X
  if (ctrl->vfeMode == VFE_OP_MODE_VIDEO) {
    switch(ctrl->hfr_mode) {
      case CAMERA_HFR_MODE_60FPS:
        sensor_set.data.mode = SENSOR_MODE_HFR_60FPS;
        CDBG("Sensor Mode 60 fps\n");
        break;

      case CAMERA_HFR_MODE_90FPS:
        sensor_set.data.mode = SENSOR_MODE_HFR_90FPS;
        CDBG("Sensor Mode 90 fps\n");
        break;

      case CAMERA_HFR_MODE_120FPS:
        sensor_set.data.mode = SENSOR_MODE_HFR_120FPS;
        CDBG("Sensor Mode 120 fps\n");
        break;

      case CAMERA_HFR_MODE_150FPS:
        CDBG("%s, Sensor mode not supported\n", __func__);
        break;

      case CAMERA_HFR_MODE_OFF:
      default:
        CDBG("%s HFR Mode %d \n", __func__, ctrl->hfr_mode);
        if (ctrl->sensor_stream_fullsize)
          sensor_set.data.mode = SENSOR_MODE_ZSL;
        break;
    }
  }
#else
 if (ctrl->vfeMode == VFE_OP_MODE_VIDEO || VFE_OP_MODE_PREVIEW) {
    switch(ctrl->hfr_mode) {
      case CAMERA_HFR_MODE_60FPS:
        sensor_set.data.mode = SENSOR_MODE_HFR_60FPS;
        CDBG("Sensor Mode 60 fps\n");
        break;

      case CAMERA_HFR_MODE_90FPS:
        sensor_set.data.mode = SENSOR_MODE_HFR_90FPS;
        CDBG("Sensor Mode 90 fps\n");
        break;

      case CAMERA_HFR_MODE_120FPS:
        sensor_set.data.mode = SENSOR_MODE_HFR_120FPS;
        CDBG("Sensor Mode 120 fps\n");
        break;

      case CAMERA_HFR_MODE_150FPS:
        CDBG("%s, Sensor mode not supported\n", __func__);
        break;

      case CAMERA_HFR_MODE_OFF:
      default:
        CDBG("%s HFR Mode %d \n", __func__, ctrl->hfr_mode);
        if (ctrl->sensor_stream_fullsize)
          sensor_set.data.mode = SENSOR_MODE_ZSL;
        break;
    }
  }
#endif
  ctrl->sensor_op_mode = sensor_set.data.mode;

  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
               ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
               ISPIF_PROCESS_STOP_IMMEDIATELY, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ISPIF_PROCESS_STOP_IMMEDIATELY failed\n", __func__);
      goto end;
    }

    rc = ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
               ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
               ISPIF_PROCESS_CFG, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: ispif_process_cfg failed %d\n", __func__, rc);
      goto end;
    }
  }

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_SET_STOP_STREAM, NULL, NULL);

  if (rc < 0) {
    CDBG_ERROR("%s: SENSOR_SET_STOP_STREAM failed %d\n", __func__, rc);
    return rc;
  }

  if (ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle) {
    actuator_set_t actuator_set_info;
    actuator_set_info.data.move.direction = MOVE_FAR;
    rc = ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
      ACTUATOR_RESTORE_FOCUS, &actuator_set_info, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: actuator_set failed %d\n", __func__, rc);
      goto end;
    }
  }
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_SET_MODE, &sensor_set, NULL);

  if (rc < 0) {
    CDBG_ERROR("%s: SENSOR_SET_MODE failed %d\n", __func__, rc);
    goto end;
  }

  if (ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle) {
    actuator_set_t actuator_set_info;
    actuator_set_info.data.move.direction = MOVE_NEAR;
    rc = ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
      ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
      ACTUATOR_RESTORE_FOCUS, &actuator_set_info, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: actuator_set failed %d\n", __func__, rc);
      goto end;
    }
  }

  if (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
    sensor_get.data.aec_info.op_mode = sensor_set.data.mode;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
               ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
               SENSOR_GET_SENSOR_MODE_AEC_INFO,
               &sensor_get, sizeof(sensor_get));
    if (rc < 0) {
      CDBG_ERROR("%s: SENSOR_GET_SENSOR_MODE_AEC_INFO failed %d\n", __func__, rc);
      goto end;
    }

    if ((ctrl->vfeMode != VFE_OP_MODE_SNAPSHOT)&&
        (ctrl->vfeMode != VFE_OP_MODE_RAW_SNAPSHOT)) {
      ctrl->stats_proc_ctrl.intf.input.sensor_info.preview_linesPerFrame =
        sensor_get.data.aec_info.lines_per_frame;
      ctrl->stats_proc_ctrl.intf.input.sensor_info.pixel_clock =
        sensor_get.data.aec_info.pclk;
      ctrl->stats_proc_ctrl.intf.input.sensor_info.pixel_clock_per_line =
        sensor_get.data.aec_info.pixels_per_line;
      ctrl->stats_proc_ctrl.intf.input.sensor_info.max_preview_fps =
        ctrl->stats_proc_ctrl.intf.input.sensor_info.preview_fps =
        sensor_get.data.aec_info.max_fps;
    } else {
      ctrl->stats_proc_ctrl.intf.input.sensor_info.snap_linesPerFrame =
        sensor_get.data.aec_info.lines_per_frame;
      ctrl->stats_proc_ctrl.intf.input.sensor_info.snapshot_fps =
        sensor_get.data.aec_info.max_fps;
    }

  sensor_set.type = SENSOR_SET_CHROMATIX_TYPE;
  if (ctrl->vfeMode == VFE_OP_MODE_PREVIEW ||
      ctrl->vfeMode == VFE_OP_MODE_SNAPSHOT ||
      ctrl->vfeMode == VFE_OP_MODE_RAW_SNAPSHOT ||
      ctrl->vfeMode == VFE_OP_MODE_JPEG_SNAPSHOT) {
    sensor_set.data.chromatix_type = SENSOR_LOAD_CHROMATIX_PREVIEW;
  } else if (ctrl->vfeMode == VFE_OP_MODE_ZSL) {
    sensor_set.data.chromatix_type = SENSOR_LOAD_CHROMATIX_ZSL;
  } else if (ctrl->vfeMode == VFE_OP_MODE_VIDEO) {
    switch(ctrl->hfr_mode) {
      case CAMERA_HFR_MODE_OFF:
        if (ctrl->sensor_stream_fullsize) {
          sensor_set.data.chromatix_type = SENSOR_LOAD_CHROMATIX_ZSL;
        } else if (ctrl->videoHint) {
          sensor_set.data.chromatix_type = SENSOR_LOAD_CHROMATIX_VIDEO_DEFAULT;
        } else {
          sensor_set.data.chromatix_type = SENSOR_LOAD_CHROMATIX_PREVIEW;
        }
        break;
      case CAMERA_HFR_MODE_60FPS:
        sensor_set.data.chromatix_type = SENSOR_LOAD_CHROMATIX_VIDEO_HFR_60FPS;
        break;
      case CAMERA_HFR_MODE_90FPS:
        sensor_set.data.chromatix_type = SENSOR_LOAD_CHROMATIX_VIDEO_HFR_90FPS;
        break;
      case CAMERA_HFR_MODE_120FPS:
          sensor_set.data.chromatix_type =
            SENSOR_LOAD_CHROMATIX_VIDEO_HFR_120FPS;
          break;
      case CAMERA_HFR_MODE_150FPS:
          sensor_set.data.chromatix_type =
            SENSOR_LOAD_CHROMATIX_VIDEO_HFR_150FPS;
          break;
      default:
          sensor_set.data.chromatix_type =
            SENSOR_LOAD_CHROMATIX_VIDEO_HFR_60FPS;
          break;
      }
      CDBG("%s: %d\n", __func__, __LINE__);
    }

    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
               ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
               SENSOR_GET_CHROMATIX_TYPE, &sensor_get, sizeof(sensor_get));

    if (rc < 0) {
      CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
      goto end;
    }

    rc = config_update_chromatix_params(ctrl);
    if (rc < 0) {
      CDBG_ERROR("%s: update params failed %d\n", __func__, rc);
      //fall through since the update is not critical
    }

    if (sensor_set.data.chromatix_type != sensor_get.data.chromatix_type) {
      CDBG("%s: chromatix change = %d ",__func__,chromatix_type_change);
      chromatix_type_change = 1;
      CDBG("%s: chromatix change = %d ",__func__,chromatix_type_change);
      rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
                 ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
                 SENSOR_SET_CHROMATIX_TYPE, &sensor_set, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s: sensor_set_params failed %d\n", __func__, rc);
        goto end;
      }

      rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
                 ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
                 SENSOR_GET_CHROMATIX_PTR, &sensor_get, sizeof(sensor_get));
      if (rc < 0) {
        CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
        goto end;
      }
      CDBG("%s: %d\n", __func__, __LINE__);

      ctrl->chromatix_ptr = sensor_get.data.chromatix_ptr;
      if (ctrl->chromatix_ptr) {
        rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
                   ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                   VFE_SET_CHROMATIX_PARM, (void *)ctrl->chromatix_ptr, NULL);
        if (rc != VFE_SUCCESS) {
          CDBG_ERROR("%s: vfe_set_params failed: rc = %d!\n", __func__, rc);
          goto end;
        }
        ctrl->stats_proc_ctrl.intf.input.chromatix = ctrl->chromatix_ptr;
        ctrl->stats_proc_ctrl.intf.input.af_tune_ptr = ctrl->af_tune_ptr;
        stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
        stats_proc_set_t sp_set_param;
        sp_set_param.type = STATS_PROC_CHROMATIX_RELOAD_TYPE;
        rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
                   ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
                   sp_set_param.type, &sp_set_param, &(sp_ctrl->intf));
      }
    }
  }
  CDBG("%s: %d\n", __func__, __LINE__);

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_GET_PENDING_FPS, &sensor_get, sizeof(sensor_get));
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    goto end;
  } else if (sensor_get.data.get_pending_fps) {
    sensor_set.data.aec_data.fps = sensor_get.data.get_pending_fps;
    rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
               ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
               SENSOR_SET_FPS, &sensor_set, NULL);
  }
  if (rc < 0) {
    CDBG_ERROR("%s: sensor_set fps failed %d\n", __func__, rc);
    goto end;
  }
  CDBG("%s: %d\n", __func__, __LINE__);

  /* camif config - begin*/
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    camif_input.obj_idx = 0;
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PARAMS_ADD_OBJ_ID, (void *)&camif_input, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s CAMIF_PARAMS_ADD_OBJ_ID failed %d ", __func__, rc);
      rc = 0; // ignore the error since hard coded in init return -EINVAL;
    }
    camif_input.obj_idx = 0;
    camif_input.d.mode = ctrl->ops_mode;
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PARAMS_MODE, (void *)&camif_input, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s CAMIF_PARAMS_MODE failed %d ", __func__, rc);
      rc = -EINVAL;
      goto end;
    }
    camif_input.obj_idx = 0;
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PARAMS_SENSOR_DIMENSION, (void *)&camif_input, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: set parm CAMIF_PARAMS_SENSOR_FORMAT failed %d",
               __func__, rc);
      rc = -EINVAL;
      goto end;
    }
    camif_input.obj_idx = 0;
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PARAMS_SENSOR_CROP_WINDOW, (void *)&camif_input, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: set parm CAMIF_PARAMS_SENSOR_CROP_WINDOW failed %d",
               __func__, rc);
      rc = -EINVAL;
      goto end;
    }
    CDBG("%s: %d\n", __func__, __LINE__);

    camif_input.obj_idx = 0;
    camif_input.d.format = ctrl->sensorCtrl.sensor_output.output_format;
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PARAMS_SENSOR_FORMAT, (void *)&camif_input, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: set parm CAMIF_PARAMS_SENSOR_FORMAT failed %d",
               __func__, status);
      rc = -EINVAL;
      goto end;
    }
    camif_input.obj_idx = 0;
    camif_input.d.connection_mode =
      ctrl->sensorCtrl.sensor_output.connection_mode;
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].set_params(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_PARAMS_CONNECTION_MODE, (void *)&camif_input, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: set parm CAMIF_PARAMS_CONNECTION_MODE failed %d",
                 __func__, status);
      rc = -EINVAL;
      goto end;
    }
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].get_params(
               ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
               CAMIF_PARAMS_CAMIF_DIMENSION, (void *)&camif_output,
               sizeof(camif_output));
    if (rc < 0) {
      CDBG_ERROR("%s: get parm CAMIF_PARAMS_CAMIF_DIMENSION failed %d",
                 __func__, status);
      rc = -EINVAL;
      goto end;
    }
  }
  /*camif config - end*/
  /* Set the RAW snapshot dimensions here since we get the
   * correct dimension from CAMIF interface here. */
  if (ctrl->vfeMode == VFE_OP_MODE_RAW_SNAPSHOT) {
      ctrl->curr_output_info.output[PRIMARY].image_width =
        camif_output.d.camif_window.width;
      ctrl->curr_output_info.output[PRIMARY].image_height =
        camif_output.d.camif_window.height;
  }

  if (ctrl->sensorCtrl.sensor_output.output_format == SENSOR_YCBCR)
    run_3a = FALSE;

  if (run_3a) {
    vfe_stats_output_t vfe_out;
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle,
               VFE_GET_AEC_SHIFT_BITS, (void *)&vfe_out, sizeof(vfe_out));
    config_do_3a_snap(ctrl, &vfe_out );
    /* Configure AEC and AWB params to Stats proc before using
     * this function, since frame proc needs AEC and AWB gains. */
    config_prep_frame_proc(ctrl);
  }
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_CAMIF_DIM, NULL, NULL);
  if (rc) {
    CDBG_ERROR("%s: vfe_set_params VFE_SET_CAMIF_DIM failed\n", __func__);
    rc = -EINVAL;
    goto end;
  }
  if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
    rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
             ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
             CAMIF_OPS_CONFIG, NULL);
    if (rc < 0) {
      CDBG_ERROR("%s: CAMIF_OPS_CONFIG failed %d", __func__, rc);
      rc = -EINVAL;
      goto end;
    }
  }
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_SENSOR_PARM, NULL, NULL);
  if (rc) {
    CDBG_ERROR("%s: vfe_set_params VFE_SET_SENSOR_PARM failed\n", __func__);
    rc = -EINVAL;
    goto end;
  }

  //set vfe capture number
  if (ctrl->vfeMode == VFE_OP_MODE_SNAPSHOT) {
    if ((ctrl->hdrCtrl.exp_bracketing_enable) ||
       (ctrl->hdrCtrl.hdr_enable)) {
      ctrl->curr_output_info.num_output = capture_cnt;
    }
  }

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_OUTPUT_INFO,
             (void *)&ctrl->curr_output_info, NULL);
  if (rc) {
    CDBG_ERROR("%s: vfe_set_params VFE_SET_SENSOR_PARM failed\n", __func__);
    rc = -EINVAL;
    goto end;
  }
  uint32_t crop_factor;
  rc = zoom_get_parms(&ctrl->zoomCtrl, ZOOM_PARM_GET_CROP_FACTOR,
                      (void *)&ctrl->zoomCtrl.zoom_val, (void *)&crop_factor);
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_SET_FOV_CROP_FACTOR, &crop_factor, NULL);
  CDBG("%s: zoom_val = %d, crop_factor = %d", __func__, ctrl->zoomCtrl.zoom_val, crop_factor);

  if (ctrl->is_eeprom) {
    eeprom_get_t eeprom_get;
    eeprom_get.type = EEPROM_GET_CALIB_2D_DPC;
      rc = ctrl->comp_ops[MCTL_COMPID_EEPROM].get_params(
        ctrl->comp_ops[MCTL_COMPID_EEPROM].handle,
        ctrl->sensor_op_mode, &eeprom_get, sizeof(eeprom_get_t));
        if (rc < 0) {
          CDBG_ERROR("%s: eeprom_get_params failed: rc = %d!\n", __func__, rc);
          rc = -EINVAL;
          goto end;
        }
  }
  if (ctrl->vfeMode == VFE_OP_MODE_SNAPSHOT) {
    if ((ctrl->hdrCtrl.exp_bracketing_enable) ||
       (ctrl->hdrCtrl.hdr_enable)) {
      vfe_frame_skip frame_skip;
#ifndef VFE_2X
      /*Note: leading_skip depending on sensor, it
        shall either from chromatix or queried from sensor
        Now harded as is, OEM need adjust it by sensor*/
      ctrl->hdrCtrl.leading_skip = 2;
      ctrl->hdrCtrl.skip_period = 1;
      frame_skip.output1period = ctrl->hdrCtrl.leading_skip +
        ctrl->hdrCtrl.total_frames -1;
      frame_skip.output1pattern = ((0x1 << ctrl->hdrCtrl.total_frames)-1) <<
        ctrl->hdrCtrl.leading_skip;
      frame_skip.output2period = frame_skip.output1period;
      frame_skip.output2pattern = frame_skip.output1pattern;
#else
      //set skip pattern for HDR on VFE2X
      // 10001000100010001000100010001000
      // skip 3 every 4 frames, skip period can be set to 3 for HDR ae-bracketing
      // as it will always take more than 1 frame time for exposure to take effect
      frame_skip.output1pattern = 0x8888888;
      frame_skip.output2pattern = 0x8888888;
      ctrl->hdrCtrl.skip_period = 4;
#endif
      if(0 != (rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
                          ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                          VFE_SET_FRAME_SKIP, &frame_skip, NULL))) {
        CDBG_ERROR("%s: vfe_set_params VFE_SET_FRAME_SKIP failed\n", __func__);
        rc = -EINVAL;
        goto end;
      }
    }
  }
  if (0 != (rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_CONFIG_MODE, &op_mode))) {
    CDBG_ERROR("%s: config VFE_OP_MODE_PREVIEW failed.\n", __func__);
    rc = -EINVAL;
    goto end;
  }

  /*reconfig bestshot mode if reloaded Chromatix*/
  if (chromatix_type_change == 1) {
    CDBG("%s: reconifg BSM",__func__);
    if(!bestshot_reconfig_mode(ctrl, &(ctrl->bestshotCtrl))) {
      CDBG_ERROR("%s Bestshot reconfig fail",__func__);
      chromatix_type_change = 0;
    }
  }

  if (ctrl->vfeMode == VFE_OP_MODE_SNAPSHOT ||
      ctrl->vfeMode == VFE_OP_MODE_JPEG_SNAPSHOT ||
      ctrl->vfeMode == VFE_OP_MODE_RAW_SNAPSHOT) {
    struct v4l2_event_and_payload event;
    event.payload_length = 0;
    event.transaction_id = -1;
    event.payload = NULL;
    mm_camera_event_t *cam_event = (mm_camera_event_t *)event.evt.u.data;
    /* Send events of shutter success. */
    event.evt.type = V4L2_EVENT_PRIVATE_START+MSM_CAM_APP_NOTIFY_EVENT;
    cam_event->event_type = MM_CAMERA_EVT_TYPE_CTRL;
    cam_event->e.ctrl.evt = MM_CAMERA_CTRL_EVT_SNAPSHOT_CONFIG_DONE;
    cam_event->e.ctrl.status = CAM_CTRL_SUCCESS;
    rc = ioctl(ctrl->camfd, MSM_CAM_IOCTL_V4L2_EVT_NOTIFY, &event);
    CDBG("%s: Issued shutter event.\n", __func__);
  }

  axi_set.type = AXI_PARM_RESERVE_INTF;
  axi_set.data.axi_obj_idx = 0;
  axi_set.data.interface_mask = ctrl->channel_interface_mask;
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             axi_set.type, (void *)&axi_set, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s: Failed to set AXI params %d", __func__, rc);
    goto end;
  }

  /* we only have one AXI now. Need to remove the hard coding */
  axi_set.data.axi_obj_idx = 0;
  axi_set.data.interface_mask = ctrl->channel_interface_mask;
  axi_set.type = AXI_PARM_SENSOR_DATA;
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             axi_set.type, (void *)&axi_set, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s AXI SET SENSOR DATA failed %d ", __func__, rc);
    return -EINVAL;
  }

  axi_set.type = AXI_PARM_OUTPUT_INFO;
  axi_set.data.axi_obj_idx = 0;
  axi_set.data.output_info = ctrl->curr_output_info;
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].set_params(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             axi_set.type, (void *)&axi_set, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s: Failed to set AXI params %d", __func__, rc);
    goto end;
  }
  axi_set.data.axi_obj_idx = 0;
  axi_cfg.mode = ctrl->ops_mode;
  axi_cfg.vfe_port = ctrl->curr_output_info.vfe_ports_used;
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
             ctrl->comp_ops[MCTL_COMPID_AXI].handle,
             AXI_PROC_EVENT_CONFIG, &axi_cfg);
  if (rc < 0) {
    CDBG_ERROR("%s: axi_process failed %d\n", __func__, rc);
    goto end;
  }

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
      ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_RS_CS_PARM,
      &(ctrl->rs_cs_params), sizeof(ctrl->rs_cs_params));
  if (rc < 0)
    CDBG_HIGH("%s: Failed to get RS CS params from VFE %d", __func__, rc);

  if (ctrl->video_dis.enable_dis && ctrl->video_dis.sensor_has_margin) {
    stats_proc_set_t sp_set_param;
    sp_set_param.type = STATS_PROC_DIS_TYPE;
    sp_set_param.d.set_dis.type = DIS_S_INIT_DATA;

    /* Populate CS/RS Cfg Data */
    sp_set_param.d.set_dis.d.init_data.rs_cs_parm = ctrl->rs_cs_params;

    /* Populate Frame Cfg Data */
    sp_set_param.d.set_dis.d.init_data.frame_cfg.frame_fps = 30.0;
    sp_set_param.d.set_dis.d.init_data.frame_cfg.dis_frame_width =
      ctrl->curr_output_info.output[SECONDARY].image_width;
    sp_set_param.d.set_dis.d.init_data.frame_cfg.dis_frame_height =
      ctrl->curr_output_info.output[SECONDARY].image_height;
    sp_set_param.d.set_dis.d.init_data.frame_cfg.vfe_output_width =
      ctrl->curr_output_info.output[SECONDARY].image_width +
      ctrl->curr_output_info.output[SECONDARY].extra_pad_width;
    sp_set_param.d.set_dis.d.init_data.frame_cfg.vfe_output_height =
      ctrl->curr_output_info.output[SECONDARY].image_height +
      ctrl->curr_output_info.output[SECONDARY].extra_pad_height;

    if ((ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
               ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
               sp_set_param.type, &sp_set_param, &(sp_ctrl->intf))) < 0) {
      CDBG_HIGH("%s Stats proc set params DIS_S_INIT_DATA failed ", __func__);
      ctrl->video_dis.enable_dis = FALSE;
    }
  }

  CDBG("%s: %d\n", __func__, __LINE__);
  if (ctrl->vfeMode == VFE_OP_MODE_SNAPSHOT ||
      ctrl->vfeMode == VFE_OP_MODE_RAW_SNAPSHOT ||
      ctrl->vfeMode == VFE_OP_MODE_JPEG_SNAPSHOT) {
    if(ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
      flash_led_get_t led_get_parm;
      ctrl->comp_ops[MCTL_COMPID_FLASHLED].get_params(
        ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
        FLASH_GET_MODE, &led_get_parm, sizeof(led_get_parm));
      if (led_get_parm.data.led_mode != LED_MODE_OFF &&
        led_get_parm.data.led_mode != LED_MODE_TORCH &&
        sp_ctrl->intf.output.aec_d.use_led_estimation) {
        flash_led_set_t led_set_parm;
        led_set_parm.data.led_state = MSM_CAMERA_LED_HIGH;
        sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_HIGH;
        ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
          ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
          FLASH_SET_STATE, &led_set_parm, NULL);
      }
    }
    if (sp_output->aec_d.strobe_cfg_st == STROBE_PRE_FIRED) {
      /*start timer*/
      rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
        ctrl->comp_ops[MCTL_COMPID_CAMIF].handle, CAMIF_OPS_TIMER_CONFIG, NULL);
      if (rc < 0) {
        CDBG_ERROR("%s: CAMIF_OPS_TIMER_CONFIG failed %d", __func__, rc);
        rc = -EINVAL;
        goto end;
      }
    }
  }
  CDBG("%s: %d\n", __func__, __LINE__);
  sp_ctrl->sof_update_needed = FALSE;
  switch (ctrl->vfeMode) {
    case VFE_OP_MODE_PREVIEW:
    case VFE_OP_MODE_VIDEO:
      if(ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR) {
        sensor_update_ok = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
                                 ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
                                 SENSOR_SET_EXPOSURE, NULL, NULL);
        if (sensor_update_ok < 0) {
          CDBG_HIGH("%s Sensor gain update failed ", __func__);
          rc = -1;
          sensor_update_ok = FALSE;
        } else
          sensor_update_ok = TRUE;
        CDBG("%s: Sensor update=%d \n",__func__, sensor_update_ok);
      }
      /* start AXI: */
      cmd.mod_cmd_ops = MOD_CMD_START;
      if (0 != (rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
                 ctrl->comp_ops[MCTL_COMPID_AXI].handle,
                 AXI_PROC_CMD_OPS, &cmd))) {
        CDBG_ERROR("%s: config AXI_START failed, rc = %d \n", __func__, rc);
        rc = -EINVAL;
        goto end;
      }
      /* start vfe: */
      if (0 != (rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                 ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                 VFE_CMD_OPS, &cmd))) {
        CDBG_ERROR("%s: config VFE_START failed, rc = %d \n", __func__, rc);
        rc = -EINVAL;
        goto end;
      }
      /* start camif: */
      if (ctrl->comp_mask & (1 << MCTL_COMPID_CAMIF)) {
        if (0 != (rc = ctrl->comp_ops[MCTL_COMPID_CAMIF].process(
                 ctrl->comp_ops[MCTL_COMPID_CAMIF].handle,
                 CAMIF_PROC_CMD_OPS, &cmd))) {
          CDBG_ERROR("%s: config CAMIF_START failed %d\n", __func__, rc);
          rc = -EINVAL;
          goto end;
        }
      }
      break;
    case VFE_OP_MODE_ZSL:
      rc = isp_sendcmd(ctrl->camfd, CMD_GENERAL,
              NULL, 0, VFE_CMD_ZSL);
      break;
    case VFE_OP_MODE_SNAPSHOT:
    case VFE_OP_MODE_JPEG_SNAPSHOT:
      rc = isp_sendcmd(ctrl->camfd, CMD_GENERAL, &capture_cnt,
                       sizeof(capture_cnt), VFE_CMD_CAPTURE);
      break;
    case VFE_OP_MODE_RAW_SNAPSHOT:
      rc = isp_sendcmd(ctrl->camfd, CMD_GENERAL, &capture_cnt,
                       sizeof(capture_cnt), VFE_CMD_CAPTURE_RAW);
      break;
    default:
      rc = -EINVAL;
      break;
  }

  sensor_get.data.op_mode = sensor_set.data.mode;
  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
    ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
    SENSOR_GET_CUR_RES, &sensor_get, sizeof(sensor_get));

  if (rc < 0) {
    CDBG_ERROR("%s: sensor_get_params failed %d\n", __func__, rc);
    return rc;
  }

  csi_set.data.res = sensor_get.data.cur_res;
  rc = ctrl->comp_ops[MCTL_COMPID_CSI].set_params(
      ctrl->comp_ops[MCTL_COMPID_CSI].handle,
      CSI_SET_CFG, &csi_set, NULL);

  if (rc < 0) {
    CDBG_ERROR("%s: csi_set_params failed %d\n", __func__, rc);
    return rc;
  }

  rc = ctrl->comp_ops[MCTL_COMPID_SENSOR].set_params(
             ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
             SENSOR_SET_START_STREAM, NULL, NULL);

  if (rc < 0) {
    CDBG_ERROR("%s: SENSOR_SET_START_STREAM failed %d\n", __func__, rc);
    return rc;
  }

  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    ispif_status = ctrl->comp_ops[MCTL_COMPID_ISPIF].process(
          ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
          ISPIF_PROCESS_START_ON_FRAME_BOUNDARY, NULL);

    if (ispif_status < 0) {
      CDBG_ERROR("%s: ISPIF_PROCESS_START_ON_FRAME_BOUNDARY failed \n",
          __func__);
      rc = ispif_status;
      goto end;
    }
  }
  if (rc >= 0) {
    ctrl->state = CAMERA_STATE_SENT_START;
    if(ctrl->zoom_done_pending) {
      zoom_scaling_params_t zoomscaling;
      vfe_zoom_crop_info_t zoom_crop_info;
      rc = ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
                 ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                 VFE_GET_ZOOM_CROP_INFO, (void *)&zoom_crop_info,
                 sizeof(zoom_crop_info));
      rc = zoom_get_parms(&ctrl->zoomCtrl, ZOOM_PARM_GET_SCALING_INFO,
                          (void *)&zoom_crop_info, (void *)&zoomscaling);
      config_send_crop_to_mctl_pp(ctrl, &ctrl->video_ctrl, &zoomscaling);
      ctrl->zoom_done_pending = 0;
      if (config_proc_send_zoom_done_event(ctrl, NULL) < 0) {
        CDBG_HIGH("%s: config_proc_send_zoom_done_event failed w/ rc=%d",
                  __func__, rc);
      }
    }
  }
 end:
  if (rc < 0 && (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF))) {
    ispif_set.data.session_lock.acquire = false;
    ispif_set.data.session_lock.vfe_id = 0;
    ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
                 ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
                 ISPIF_SESSION_LOCK, &ispif_set, NULL);
  }
  return rc;
} /* config_MSG_ID_RESET_ACK */

/*===========================================================================
 * FUNCTION    - config_MSG_ID_STOP_ACK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_MSG_ID_STOP_ACK(void *parm1,  void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = ctrl->pendingCtrlCmd;
  int tempCmdPending;
  v4l2_video_ctrl *pvideo_ctrl;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  ispif_set_t ispif_set;

  if (ctrlCmd == NULL && ctrl->reconfig_vfe) {
    CDBG_ERROR("%s, ctrlCmd is NULL\n",__func__);
    ctrl->reconfig_vfe = 0;
    rc = config_CAMERA_START_VIDEO(parm1, parm2, &tempCmdPending);
    return rc;
  } else if (ctrlCmd) {
    ctrl->reconfig_vfe = 0;
  }

  if (!ctrlCmd) {
    CDBG_ERROR("%s ctrlCmd is NULL ", __func__);
    rc = -EINVAL;
    goto end;
  }

  CDBG("%s: ctrlCmd->type %d\n", __func__,ctrlCmd->type);

  if (ctrlCmd->type != CAMERA_STOP_PREVIEW &&
      ctrlCmd->type != CAMERA_STOP_SNAPSHOT &&
      ctrlCmd->type != CAMERA_STOP_VIDEO &&
      ctrlCmd->type != CAMERA_STOP_ZSL &&
      ctrlCmd->type != MSM_V4L2_STREAM_OFF) {
    CDBG_ERROR("%s, ctrlCmd is wrong.\n",__func__);
    rc = -EINVAL;
    goto end;
  }

  if (ctrl->state != CAMERA_STATE_SENT_STOP) {
    CDBG_ERROR("%s: ctrl->state is incorrect %d\n",
               __func__, ctrl->state);
    rc = -EINVAL;
    goto end;
  }

  pvideo_ctrl = &ctrl->video_ctrl;
  if (pvideo_ctrl->op_mode == MSM_V4L2_CAM_OP_CAPTURE &&
      ((ctrl->hdrCtrl.exp_bracketing_enable) ||
       (ctrl->hdrCtrl.hdr_enable))) {
    destroy_hdr(ctrl);
  }
#ifdef VFE_2X
  if (ctrl->vfeMode == VFE_OP_MODE_VIDEO) {
    destroy_liveshot(ctrl);
  }
#endif
  if ((ctrl->channel_stream_info != STREAM_RAW) && (ctrl->videoHint))
    config_pp_send_stream_off(parm1, ctrlCmd);

  if(pvideo_ctrl->streamon_mask) {
    CDBG_HIGH("%s: streamon_mask is not clear. Should not call PP_Release_HW",
              __func__);
  } else {
    if ((ctrl->channel_stream_info != STREAM_RAW) && (ctrl->videoHint)) {
      config_pp_release_hw(parm1, ctrlCmd);
      CDBG("%s: End pp topology", __func__);
      config_pp_end_pp_topology(parm1, ctrl->video_ctrl.op_mode);
    }
  }

  flash_led_get_t led_get_parm;
  ctrl->comp_ops[MCTL_COMPID_FLASHLED].get_params(
      ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
      FLASH_GET_MODE, &led_get_parm, sizeof(led_get_parm));

  if (led_get_parm.data.led_mode != LED_MODE_OFF &&
      led_get_parm.data.led_mode != LED_MODE_TORCH) {
    flash_led_set_t led_set_parm;
    led_set_parm.type = FLASH_SET_STATE;
    led_set_parm.data.led_state = MSM_CAMERA_LED_OFF;
    stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
    sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_OFF;
    ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
          ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
          FLASH_SET_STATE, &led_set_parm, NULL);
  }

  if (ctrl->video_dis.enable_dis && ctrl->video_dis.sensor_has_margin) {
    /* ToDo: Move this to HAL hook up of dis_enable */
    stats_proc_set_t sp_set_param;
    stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
    sp_set_param.type = STATS_PROC_DIS_TYPE;
    sp_set_param.d.set_dis.type = DIS_S_DEINIT_DATA;
    if((ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
              ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
              sp_set_param.type, &sp_set_param, &(sp_ctrl->intf))) < 0) {
      CDBG_HIGH("%s Stats proc set params DIS_S_DEINIT_DATA failed ", __func__);
      ctrl->video_dis.enable_dis = FALSE;
    }
  }
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
                       ctrl->comp_ops[MCTL_COMPID_VFE].handle,
                       VFE_RELEASE_STATS, NULL);
end:
  if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
    ispif_set.data.session_lock.acquire = false;
    ispif_set.data.session_lock.vfe_id = 0;
    ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
                 ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
                 ISPIF_SESSION_LOCK, &ispif_set, NULL);
  }
  rc = ctrl->comp_ops[MCTL_COMPID_AXI].process(
    ctrl->comp_ops[MCTL_COMPID_AXI].handle,
    AXI_PROC_EVENT_REG_UPDATE, (void *)AXI_INTF_PIXEL_0);
  if (rc < 0) {
    CDBG_ERROR("%s: AXI_PROC_EVENT_REG_UPDATE failed %d\n",
    __func__, rc);
    return rc;
  }
  /* Send ACK */
  rc = mctl_send_ctrl_cmd_done(ctrl, ctrlCmd, FALSE);
  if (rc < 0) {
    CDBG_ERROR("%s: sending ctrl_cmd_done failed rc = %d\n",
      __func__, rc);
    return rc;
  }

  /*  Clears pendingCtrlCmd in ctrl */
  if (ctrl->pendingCtrlCmd) {
    free(ctrl->pendingCtrlCmd);
    ctrl->pendingCtrlCmd = NULL;
  }
  CDBG("%s: stop ack\n", __func__);
  ctrl->state = CAMERA_STATE_IDLE;

  CDBG("%s: new ctrl->state is %d\n", __func__, ctrl->state);

  return rc;
} /* config_MSG_ID_STOP_ACK */

/*===========================================================================
 * FUNCTION    - config_MSG_ID_START_ACK -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int config_MSG_ID_START_ACK(void *parm1, void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = ctrl->pendingCtrlCmd;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  ispif_set_t ispif_set;

  if (ctrl->state == CAMERA_STATE_SENT_START) {

    if (ctrlCmd) {

      if (ctrl->comp_mask & (1 << MCTL_COMPID_ISPIF)) {
        ispif_set.data.session_lock.acquire = false;
        ispif_set.data.session_lock.vfe_id = 0;
        ctrl->comp_ops[MCTL_COMPID_ISPIF].set_params(
               ctrl->comp_ops[MCTL_COMPID_ISPIF].handle,
               ISPIF_SESSION_LOCK, &ispif_set, NULL);
      }
      CDBG("%s: ctrl->state=%d \n", __func__, ctrl->state);

      rc = mctl_send_ctrl_cmd_done(ctrl, ctrlCmd, FALSE);
      if (rc < 0) {
        CDBG_ERROR("%s: sending ctrl_cmd_done failed rc = %d\n",
          __func__, rc);
        return rc;
      }

      /* Clears pendingCtrlCmd in ctrl */
      if (ctrl->pendingCtrlCmd) {
        free(ctrl->pendingCtrlCmd);
        ctrl->pendingCtrlCmd = NULL;
      }
    } else {
      CDBG("%s This is a system VFE restart.\n", __func__);
    }
  }

  switch (ctrl->state) {
    case CAMERA_STATE_SENT_START:
      ctrl->state = CAMERA_STATE_STARTED;
      ctrl->vfe_reg_updated = FALSE;
      break;

    default:
      rc = -EINVAL;
      break;
  }
  CDBG("%s: new ctrl->state is %d\n",__func__, ctrl->state);

  return rc;
}

/*===========================================================================
FUNCTION     config_event_message

DESCRIPTION
===========================================================================*/
int config_proc_event_message_0(void *parm1,  void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_cam_evt_msg *adsp =  (struct msm_cam_evt_msg *)parm2;

  switch (adsp->msg_id) {
    case MSG_ID_RESET_ACK:
      rc = config_MSG_ID_RESET_ACK(ctrl, parm2);
      break;
    case MSG_ID_START_ACK:
      rc = config_MSG_ID_START_ACK(ctrl, parm2);
      break;
    case MSG_ID_STOP_ACK:
      rc = config_MSG_ID_STOP_ACK(ctrl, parm2);
      break;

    default:
      CDBG_ERROR("%s: Unsupported message id: %d\n", __func__, adsp->msg_id);
      rc = -EINVAL;
      break;
  }
  return rc;
}

/*===========================================================================
FUNCTION     config_v4l2_request

DESCRIPTION
===========================================================================*/
int config_v4l2_request(void *parm1, void *parm2, int *cmdPending)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  struct msm_ctrl_cmd *ctrlCmd = (struct msm_ctrl_cmd *)parm2;

  CDBG("%s, type = %d\n", __func__, ctrlCmd->type);
  switch (ctrlCmd->type) {
    case MSM_V4L2_VID_CAP_TYPE: {
        rc = config_MSM_V4L2_VID_CAP_TYPE(ctrl,ctrlCmd, cmdPending);
      }
      break;

    case MSM_V4L2_STREAM_ON: {
        rc = config_MSM_V4L2_STREAM_ON(ctrl,ctrlCmd, cmdPending);
      }
      break;

    case MSM_V4L2_STREAM_OFF: {
        rc = config_MSM_V4L2_STREAM_OFF(ctrl,ctrlCmd, cmdPending);
      }
      break;

    default:
      CDBG_ERROR("%s error! type = %d\n", __func__, ctrlCmd->type);
      ctrlCmd->status = CAM_CTRL_SUCCESS;
      *cmdPending = FALSE;
      /* stop*/
      assert(0);
      break;
  }

  return rc;
}/*config_v4l2_request*/
