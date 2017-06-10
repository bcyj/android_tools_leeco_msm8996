/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file implements the media/module/master controller's focus logic in
   the mm-camera server. The functionalities of this modules are:

   1. config, process/parse awb/aec stats buffers and events
   2. control the statsproc interface
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

#include "camera_dbg.h"
#include "camera.h"
#include "cam_mmap.h"
#include "config_proc.h"
#include "mctl.h"
#include "mctl_stats.h"
#include "mctl_ez.h"
#include "mctl_af.h"
#include "dsps_hw_interface.h"

#ifdef ENABLE_MCTL_STATS
  #undef CDBG
  #define CDBG LOGE
#endif

/*===========================================================================
 * FUNCTION    - mctl_stats_do_awb -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void mctl_stats_do_awb(void *cctrl)
{
  int rc = TRUE;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_interface_input_t *sp_input = &(sp_ctrl->intf.input);
  vfe_3a_parms_udpate_t vfe_3a_params;

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

  vfe_3a_params.mask = VFE_TRIGGER_UPDATE_AWB;
  vfe_3a_params.stats_buf = NULL;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
             ctrl->comp_ops[MCTL_COMPID_VFE].handle,
             VFE_TRIGGER_UPDATE_FOR_3A, (void *)&vfe_3a_params);
  if (rc != VFE_SUCCESS)
    CDBG_HIGH("%s VFE Trigger update for AWB failed", __func__);
}

/*===========================================================================
 * FUNCTION    -  mctl_stats_do_rs_cs -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void mctl_stats_do_rs_cs(void *cctrl, uint32_t frame_id)
{
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  stats_proc_interface_input_t *sp_input = &(ctrl->stats_proc_ctrl.intf.input);
  dsps_get_data_t dsps_get;
  int rc = 0, pipeline_idx = ctrl->video_ctrl.def_pp_idx;

  if (ctrl->stats_proc_ctrl.RS_stats_ready &&
    ctrl->stats_proc_ctrl.CS_stats_ready) {
    ctrl->stats_proc_ctrl.RS_stats_ready = FALSE;
    ctrl->stats_proc_ctrl.CS_stats_ready = FALSE;

#if FEATURE_GYRO
    dsps_get.type = GET_DATA_FRAME;
    dsps_get.format = TYPE_Q16;
    dsps_get.id = (uint8_t) frame_id;
    if (dsps_get_params(&dsps_get)) {
      /* Gyro data unavailable for this frame */
      sp_input->gyro_info.q16_ready = 0;
      sp_input->gyro_info.q16[0] = 0;
      sp_input->gyro_info.q16[1] = 0;
      sp_input->gyro_info.q16[2] = 0;
    } else {
      sp_input->gyro_info.q16_ready = 1;
      sp_input->gyro_info.q16[0] = dsps_get.output_data.q16[0];
      sp_input->gyro_info.q16[1] = dsps_get.output_data.q16[1];
      sp_input->gyro_info.q16[2] = dsps_get.output_data.q16[2];
    }
#endif

    CDBG("%s: Stats sent to 3A for AFD frame_id=%d", __func__, frame_id);
    sp_input->mctl_info.type = STATS_PROC_AFD_TYPE;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].process(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
      sp_input->mctl_info.type, &(sp_ctrl->intf));
    CDBG("%s: Stats sent to 3A for AFD frame_id=%d", __func__, frame_id);

    mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AFD_CMD);

    if(ctrl->video_dis.enable_dis && ctrl->video_dis.sensor_has_margin) {
      CDBG("%s: Stats sent to 3A for DIS frame_id=%d", __func__, frame_id);
      sp_input->mctl_info.type = STATS_PROC_DIS_TYPE;
      sp_input->mctl_info.vfe_stats_out->vfe_stats_struct.cs_op.frame_id = frame_id;

      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].process(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
        sp_input->mctl_info.type, &(sp_ctrl->intf));

      CDBG("%s: rc = %d, output = %d", __func__, rc,
        sp_ctrl->intf.output.dis_d.has_output);
      if(rc == 0 && sp_ctrl->intf.output.dis_d.has_output) {
        /* update the mctl pp module that DIS result is available */
        mctl_pp_cmd_t cmd;
        memset(&cmd, 0, sizeof(mctl_pp_cmd_t));

        cmd.cmd_type = QCAM_MCTL_CMD_SET_DIS;
        cmd.dis_cmd = sp_ctrl->intf.output.dis_d.dis_pos;
        cmd.dis_cmd.extra_pad_w =
          ctrl->curr_output_info.output[SECONDARY].extra_pad_width;
        cmd.dis_cmd.extra_pad_h =
          ctrl->curr_output_info.output[SECONDARY].extra_pad_height;

        CDBG("%s: frame_id = %d, x_off = %d, y_off = %d extra w = %d, h = %d",
          __func__, sp_ctrl->intf.output.dis_d.dis_pos.frame_id,
          sp_ctrl->intf.output.dis_d.dis_pos.x,
          sp_ctrl->intf.output.dis_d.dis_pos.y, cmd.dis_cmd.extra_pad_w,
          cmd.dis_cmd.extra_pad_h);
        if (pipeline_idx >= 0)
          mctl_pp_cmd(&ctrl->mctl_pp_ctrl[pipeline_idx], &cmd);
        else
          CDBG_ERROR("%s Default pipeline closed. Cannot set DIS ", __func__);
      }
    }
  }
}

/*===========================================================================
 * FUNCTION    - mctl_stats_get_input_pix_per_region -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void mctl_stats_get_input_pix_per_region(vfe_stats_output_t *vfe_out,
  uint32_t *width, uint32_t *height)
{
  *width = vfe_out->aec_params.rgn_width;
  *height = vfe_out->aec_params.rgn_height;
} /* mctl_stats_get_input_pix_per_region */

/*===========================================================================

FUNCTION    mctl_stats_get_ae_stats_zeroregions

DESCRIPTION  Get the number horizontal and veritcal stats regions that are
             discarded due to zoom/hov crop.

===========================================================================*/
static void mctl_stats_get_ae_stats_zeroregions(void *cctrl,
  vfe_stats_output_t *vfe_out)
{
  uint16_t hsize = 0, vsize = 0;
  uint16_t hzoom = 0, vzoom = 0;
  uint16_t hclip = 0, vclip = 0;
  uint32_t h_pixels_per_region = 0, v_pixels_per_region = 0;
  int max_zero_regions = 0;
  int *zero_hregions, *zero_vregions;

  mctl_config_ctrl_t * ctrl = (mctl_config_ctrl_t *)cctrl;
  stats_proc_interface_input_t *sp_input =
    &(ctrl->stats_proc_ctrl.intf.input);
  zoom_ctrl_t *zctrl = &(ctrl->zoomCtrl);
  sensor_get_t sensor_get;

  zero_hregions = &(sp_input->mctl_info.zero_hregions);
  zero_vregions = &(sp_input->mctl_info.zero_vregions);

  if (ctrl->comp_ops[MCTL_COMPID_SENSOR].get_params(
	  ctrl->comp_ops[MCTL_COMPID_SENSOR].handle,
	  SENSOR_GET_CAMIF_CFG, &sensor_get, sizeof(sensor_get)) < 0) {
    CDBG_ERROR("%s: sensor_get_params failed \n", __func__);
    return;
  }

  hsize = sensor_get.data.camif_setting.last_pixel -
    sensor_get.data.camif_setting.first_pixel + 1;

  vsize = sensor_get.data.camif_setting.last_line -
    sensor_get.data.camif_setting.first_line + 1;
#if 0
  if (zctrl->zoomscaling.input1_width != 0 ||
    zctrl->zoomscaling.input1_height != 0) {
    hzoom = zctrl->zoomscaling.input1_width;
    vzoom = zctrl->zoomscaling.input1_height;
  } else {
    hzoom = zctrl->crop_out_x;
    vzoom = zctrl->crop_out_y;
  }

  if (hzoom > hsize || vzoom > vsize) {
    CDBG("%s: hzoom = %d, hsize = %d, vzoom = %d, vsize = %d\n",
      __FUNCTION__, hzoom, hsize, vzoom, vsize);
    *zero_hregions = 0;
    *zero_vregions = 0;
    return;
  }
#else /* todo remove when zoom is available */
  hzoom = hsize;
  vzoom = vsize - 7;
#endif
  /* Calculate horizontal and vertical clipping */
  hclip = hsize - hzoom;
  vclip = vsize - vzoom;

  /* Calculate number of horizontal/veritcal pixels per region */
  mctl_stats_get_input_pix_per_region(vfe_out,
    &h_pixels_per_region, &v_pixels_per_region);
  if (sp_input->mctl_info.numRegions == 64)
    max_zero_regions = 2;
  else if (sp_input->mctl_info.numRegions == 256)
    max_zero_regions = 6;

  /* Determine the regions to set zero weights with */
  *zero_hregions = (((hclip + 1) / 2) + h_pixels_per_region - 1) /
    h_pixels_per_region;
  *zero_vregions = (((vclip + 1) / 2) + v_pixels_per_region - 1) /
    v_pixels_per_region;

  /* Keep the center 4x4 regions */
  *zero_hregions = MIN(*zero_hregions, max_zero_regions);
  *zero_vregions = MIN(*zero_vregions, max_zero_regions);

} /* mctl_stats_get_ae_stats_zeroregions */

/*===========================================================================
 * FUNCTION    - mctl_stats_do_aec -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void mctl_stats_do_aec(void *cctrl, void *vfe_parm)
{
  int rc;
  mctl_config_ctrl_t *ctrl  = (mctl_config_ctrl_t *)cctrl;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  uint32_t h_pixels_per_region, v_pixels_per_region;
  vfe_stats_output_t *vfe_out = (vfe_stats_output_t *)vfe_parm;
  stats_proc_interface_input_t *sp_input = &(sp_ctrl->intf.input);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);
  frame_proc_interface_output_t *fp_output =
    &(ctrl->frame_proc_ctrl.intf.output);
  int32_t old_use_led_est;
  float vfe_dig_gain = 1;
  vfe_3a_parms_udpate_t vfe_3a_params;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  dsps_get_data_t dsps_get;

#ifndef VFE_2X
  mctl_stats_get_ae_stats_zeroregions(ctrl, vfe_out);
#endif
  mctl_stats_get_input_pix_per_region(vfe_out,
    &h_pixels_per_region, &v_pixels_per_region);

  sp_input->mctl_info.pixelsPerRegion =
    h_pixels_per_region * v_pixels_per_region;

/* TODO Club WNR with HJR if required
  if(fp_output->wd_d.denoise_enable &&
    (sp_output->aec_d.lux_idx > sp_input->chromatix->wavelet_enable_index)&&
    (ctrl->sensorCtrl.sensor_output.output_format != SENSOR_YCBCR)) {
    stats_proc_set_t set_param;
    set_param.type = STATS_PROC_AEC_TYPE;
    set_param.d.set_aec.type = AEC_ISO_MODE;
    set_param.d.set_aec.d.aec_iso_mode = CAMERA_ISO_DEBLUR;
    rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
      ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle, set_param.type,
      &set_param, &(sp_ctrl->intf));
    if (rc < 0)
      CDBG_ERROR("%s FAILED to set ISO_MODE\n", __func__);
  }
*/
  sp_input->postproc_info.wd_enabled = FALSE;
  CDBG("%s: numReg = %d, num_pixels_per_region_aec = %d\n", __func__,
    sp_input->mctl_info.numRegions, sp_input->mctl_info.pixelsPerRegion);
  sp_input->mctl_info.preview_width = ctrl->dimInfo.display_width;
  sp_input->mctl_info.preview_height = ctrl->dimInfo.display_height;
  sp_input->mctl_info.vfe_dig_gain = 1;
  flash_strobe_get_t strobe_get_parm;
  ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].get_params(
	ctrl->comp_ops[MCTL_COMPID_FLASHSTROBE].handle,
	FLASH_GET_STATE, &strobe_get_parm, sizeof(strobe_get_parm));
  sp_input->flash_info.strobe_chrg_ready = strobe_get_parm.data.strobe_ready;
#ifndef VFE_2X
  ctrl->comp_ops[MCTL_COMPID_VFE].get_params(
    ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_BLK_INC_COMP,
    (void *)vfe_out, sizeof(vfe_out));
  sp_input->isp_info.blk_inc_comp = vfe_out->blk_inc_comp;
#endif
  //sp_input.stats_type = AEC_STATS;

  if (sp_input->mctl_info.opt_state == STATS_PROC_STATE_SNAPSHOT) {
    sp_ctrl->sof_update_needed = FALSE;
  }
  old_use_led_est = (int32_t)sp_output->aec_d.use_led_estimation;

#ifdef FEATURE_GYRO
  dsps_get.type = GET_DATA_LAST;
  dsps_get.format = TYPE_FLOAT;
  if (dsps_get_params(&dsps_get)) {
    /* Gyro data unavailable */
    sp_input->gyro_info.float_ready = 0;
    sp_input->gyro_info.flt[0] = 0.0;
    sp_input->gyro_info.flt[1] = 0.0;
    sp_input->gyro_info.flt[2] = 0.0;
  } else {
    sp_input->gyro_info.float_ready = 1;
    sp_input->gyro_info.flt[0] = dsps_get.output_data.flt[0];
    sp_input->gyro_info.flt[1] = dsps_get.output_data.flt[1];
    sp_input->gyro_info.flt[2] = dsps_get.output_data.flt[2];
  }
#endif

  sp_input->mctl_info.type = STATS_PROC_AEC_TYPE;
  rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].process(
    ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
    sp_input->mctl_info.type, &(sp_ctrl->intf));
  if (!rc) {
    if (sp_input->mctl_info.opt_state == STATS_PROC_STATE_SNAPSHOT) {
      config_proc_write_sensor_gain(ctrl);
    } else {
      /* Update gain on next SOF */
      sp_ctrl->sof_update_needed = sp_output->aec_d.sof_update;
    }
    if(sp_output->af_d.active && !sp_output->af_d.cont_af_enabled){
      CDBG_ERROR("Ignoring execution of AEC as AF is active");
      sp_ctrl->sof_update_needed = FALSE;
      return;
    }
// Instead of sp_input, use mctl states. mctl shouldnt use interface's states.
    if (sp_input->mctl_info.opt_state == STATS_PROC_STATE_PREVIEW ||
      sp_input->mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER ||
      sp_input->mctl_info.opt_mode == STATS_PROC_MODE_2D_ZSL ||
      sp_input->mctl_info.opt_mode == STATS_PROC_MODE_3D_ZSL) {
      stats_proc_get_t get_param;
      int aec_settled = 0, use_strobe = 0;
      get_param.type = STATS_PROC_AEC_TYPE;
      get_param.d.get_aec.type = AEC_LED_STROBE;
      rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].get_params(
        ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
        get_param.type, &get_param, sizeof(get_param));
      if (!rc) {
        use_strobe = get_param.d.get_aec.d.use_strobe;
        aec_settled = sp_output->aec_d.aec_flash_settled;
      } else {
        CDBG("%s Stats proc get params failed. ",__func__);
      }

      if (aec_settled == 1 || use_strobe) {
        struct msm_ctrl_cmd *ctrlCmd = ctrl->pendingPrepSnapCtrlCmd;
        if (ctrlCmd) {
          /* Sends the CTRL_CMD_DONE to kernel for pending start vfe command. */
          v4l2_ioctl.ioctl_ptr = ctrlCmd;
          if (ioctl(ctrlCmd->resp_fd, MSM_CAM_V4L2_IOCTL_CTRL_CMD_DONE, &v4l2_ioctl) < 0)
            CDBG_HIGH("%s: sending MSM_CAM_IOCTL_CTRL_CMD_DONE failed\n", __func__);
          else
            CDBG_HIGH("%s: Done sending Prep SnapCmd Ctrl",__func__);
          free(ctrl->pendingPrepSnapCtrlCmd);
          ctrl->pendingPrepSnapCtrlCmd = NULL;
        }
      }

      if (old_use_led_est == 0 && sp_output->aec_d.use_led_estimation) {
        // Change update_led_state to return the changed led state.
        flash_led_set_t led_set_parm;
        led_set_parm.data.led_state = MSM_CAMERA_LED_OFF;
        sp_ctrl->intf.input.flash_info.led_state = MSM_CAMERA_LED_OFF;
		ctrl->comp_ops[MCTL_COMPID_FLASHLED].set_params(
              ctrl->comp_ops[MCTL_COMPID_FLASHLED].handle,
              FLASH_SET_STATE, &led_set_parm, NULL);
        stats_proc_set_t set_param;
        set_param.type = STATS_PROC_AWB_TYPE;
        set_param.d.set_awb.type = AWB_RESTORE_LED_GAINS;
        rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
                   ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
                   set_param.type, &set_param, &(sp_ctrl->intf));
        if (rc)
          CDBG_HIGH("%s Failed to restore LED Gains", __func__);
      }
    }
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle,
               VFE_SET_AEC_PARAMS, NULL, NULL);
    if (rc != VFE_SUCCESS)
      CDBG_HIGH("%s VFE Set AEC params failed ", __func__);

    float dig_gain = sp_ctrl->digital_gain;
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].set_params(
               ctrl->comp_ops[MCTL_COMPID_VFE].handle,
               VFE_SET_SENSOR_DIG_GAIN, (void *)&dig_gain, NULL);
    if (rc != VFE_SUCCESS)
      CDBG_HIGH("%s VFE set sensor digital gain failed ", __func__);

/* TODO
 * Set ASD & Flash parameters before triggering AEC Update.
 */
    vfe_3a_params.mask = VFE_TRIGGER_UPDATE_AEC;
    vfe_3a_params.stats_buf = NULL;
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].process(
      ctrl->comp_ops[MCTL_COMPID_VFE].handle, VFE_TRIGGER_UPDATE_FOR_3A,
      (void *)&vfe_3a_params);
    if (rc != VFE_SUCCESS)
      CDBG_HIGH("%s VFE Trigger update for AEC failed", __func__);

    /* For the beginning of CAF, we need AEC to be settled */
    CDBG("%s:aec_settled=%d\n", __func__, sp_output->aec_d.aec_settled);

    CDBG("%s: opt_state = %d, opt_mode = %d, trigger_CAF = %d, "
         "cont_af_enabled = %d, af_cont_enable = %d\n", __func__, 
         sp_input->mctl_info.opt_state, sp_input->mctl_info.opt_mode, 
         sp_input->mctl_info.trigger_CAF, sp_output->af_d.cont_af_enabled,
         ctrl->afCtrl.af_cont_enable);
    if (sp_output->aec_d.aec_settled &&
        (sp_input->mctl_info.opt_state == STATS_PROC_STATE_PREVIEW ||
         sp_input->mctl_info.opt_mode == STATS_PROC_MODE_2D_ZSL ||
         sp_input->mctl_info.opt_mode == STATS_PROC_MODE_3D_ZSL ||
         sp_input->mctl_info.opt_state == STATS_PROC_STATE_CAMCORDER)) {
      if (sp_input->mctl_info.trigger_CAF == TRUE &&
        ctrl->afCtrl.af_cont_enable && ctrl->afCtrl.af_enable) {
        sp_input->mctl_info.trigger_CAF = FALSE;
        CDBG("Trigger CAF......\n");
        mctl_af_start(ctrl, ctrl->afCtrl.parm_focusrect.current_value);
        stats_proc_set_t set_param;
        /* Set Auto Focus parameters */
        set_param.type = STATS_PROC_AF_TYPE;
        set_param.d.set_af.type = AF_START_PARAMS;
        rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
                   ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
                   set_param.type, &set_param, &(sp_ctrl->intf));
        if (rc < 0) {
          CDBG_ERROR("%s: FAILED to set AF_START_PARAMS %d\n", __func__, rc);
          return;
        }

        /* Reset Lens */
        /* If we have just come back after touch-AF we won't reset the lens */
        if (!ctrl->stats_proc_ctrl.intf.output.af_d.touch_af_enabled) {
          set_param.type = STATS_PROC_AF_TYPE;
          set_param.d.set_af.type = AF_RESET_LENS;
          rc = ctrl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
                     ctrl->comp_ops[MCTL_COMPID_STATSPROC].handle,
                     set_param.type, &set_param, &(sp_ctrl->intf));
          if (rc < 0) {
            CDBG_ERROR("%s: FAILED to set AF_RESET_LENS %d\n", __func__, rc);
            return;
          }
          if (sp_ctrl->intf.output.af_d.reset_lens == TRUE &&
            ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle) {
            rc = ctrl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
              ctrl->comp_ops[MCTL_COMPID_ACTUATOR].handle,
              ACTUATOR_DEF_FOCUS, NULL, NULL);
            if (rc != 0) {
              CDBG_ERROR("%s(%d)Failure:Reset lens failed\n",
              __FILE__, __LINE__);
            } else {
              CDBG("%s: Reset lens succeeded\n", __func__);
            }
          }
        }
        /* Reset touchAF enabled flag */
        ctrl->stats_proc_ctrl.intf.output.af_d.touch_af_enabled = FALSE;
      }

    } else
      CDBG("AEC is not settled ......\n");

  } else
    CDBG("%s Error processing stats ", __func__);

  CDBG("VFE process AEC\n");
} /* mctl_stats_do_aec */

/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_AE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_AE(void *parm1, void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;

  parse_stats = (!ctrl->stats_proc_ctrl.intf.output.af_d.active) ||
                  (ctrl->stats_proc_ctrl.intf.output.af_d.cont_af_enabled);
    vfe_stats_out->aec_bg_done = FALSE;
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
           ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
           STATS_TYPE_YUV, parm2,
           (void *)vfe_stats_out);
    if(rc < 0) {
      CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
      return rc;
    }
    if(!vfe_stats_out->aec_bg_done) {
      CDBG_ERROR("%s: stats parsing not done", __func__);
      return 0;
    }

    ctrl->stats_proc_ctrl.intf.input.mctl_info.numRegions =
      vfe_stats_out->numRegions;
    if(parse_stats)
      mctl_stats_do_aec(ctrl, (void *)vfe_stats_out);

  if (ctrl->p_client_ops && ctrl->p_client_ops->update_params) {
    int aec_settled = ctrl->stats_proc_ctrl.intf.output.aec_d.aec_settled;
    ctrl->p_client_ops->update_params(ctrl->p_client_ops->handle,
      CAM_CLIENT_AEC_SETTLED, &aec_settled);
  }
  mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AEC_CMD);

  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_AE */


/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_AWB -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_AWB(void *parm1, void *parm2)
{
  int rc = 0, i = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;

  vfe_stats_out->awb_done = FALSE;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
         ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
         STATS_TYPE_YUV, parm2,
         (void *)vfe_stats_out);
  if(rc < 0) {
    CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
    return rc;
  }
  if(!vfe_stats_out->awb_done) {
    CDBG_ERROR("%s: stats parsing not done", __func__);
    return 0;
  }
  ctrl->stats_proc_ctrl.intf.input.mctl_info.numRegions =
    vfe_stats_out->numRegions;
  if(parse_stats)
    mctl_stats_do_awb(ctrl);
  mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AWB_CMD);

  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_AWB */

/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_RS -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_RS(void *parm1, void *parm2)
{
  int rc = 0, i = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) parm2;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
         ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
         STATS_TYPE_YUV, parm2,
         (void *)vfe_stats_out);
  if(rc < 0) {
    CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
    return rc;
  }
  if(!vfe_stats_out->rs_done) {
    CDBG_ERROR("%s: stats parsing not done", __func__);
    return 0;
  }
  sp_ctrl->intf.input.mctl_info.rgnVnum = ctrl->rs_cs_params.rs_num_rgns;
  if(parse_stats) {
    ctrl->stats_proc_ctrl.RS_stats_ready = TRUE;
    mctl_stats_do_rs_cs(ctrl, adsp->frame_id);
  }
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_RS */

/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_CS -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_CS(void *parm1, void *parm2)
{
  int rc = 0, i = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;
  stats_proc_ctrl_t *sp_ctrl = &(ctrl->stats_proc_ctrl);
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) parm2;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;

  rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
         ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
         STATS_TYPE_YUV, parm2,
         (void *)vfe_stats_out);
  if(rc < 0) {
    CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
    return rc;
  }
  if(!vfe_stats_out->cs_done) {
    CDBG_ERROR("%s: stats parsing not done", __func__);
    return 0;
  }
  sp_ctrl->intf.input.mctl_info.rgnHnum = ctrl->rs_cs_params.cs_num_rgns;
  if(parse_stats) {
    ctrl->stats_proc_ctrl.CS_stats_ready = TRUE;
    mctl_stats_do_rs_cs(ctrl, adsp->frame_id);
  }

  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_CS */

/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_WB_EXP -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_WB_EXP(void *parm1, void *parm2)
{
  int rc = 0;
  int i = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;
  struct msm_cam_evt_msg *adsp = (struct msm_cam_evt_msg *) parm2;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;

  parse_stats = (!ctrl->stats_proc_ctrl.intf.output.af_d.active) ||
                  (ctrl->stats_proc_ctrl.intf.output.af_d.cont_af_enabled);

  vfe_stats_out->wbexp_done = FALSE;
  if (!ctrl->stats_proc_ctrl.intf.output.af_d.active ||
    ctrl->stats_proc_ctrl.intf.output.af_d.cont_af_enabled) {
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
           ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
           STATS_TYPE_YUV, parm2,
           (void *)vfe_stats_out);
    if(rc < 0) {
      CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
      return rc;
    }
    if(!vfe_stats_out->wbexp_done) {
      CDBG_ERROR("%s: stats parsing not done", __func__);
      return 0;
    }
    CDBG("VFE_ID_STATS_WB_EXP numReg %d\n", vfe_stats_out->numRegions);
    ctrl->stats_proc_ctrl.intf.input.mctl_info.numRegions =
      vfe_stats_out->numRegions;
    if(parse_stats) {
      mctl_stats_do_aec(ctrl, (void *)vfe_stats_out);
      mctl_stats_do_awb(ctrl);
    }
  }
  mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AEC_CMD);
  mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AWB_CMD);
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_WB_EXP */

/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_BE -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_BE(void *parm1, void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;

  parse_stats = (!ctrl->stats_proc_ctrl.intf.output.af_d.active) ||
                  (ctrl->stats_proc_ctrl.intf.output.af_d.cont_af_enabled);

  vfe_stats_out->be_done = FALSE;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
         ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
         STATS_TYPE_BAYER, parm2,
         (void *)vfe_stats_out);
  if(rc < 0) {
    CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
    return rc;
  }
  if(!vfe_stats_out->be_done) {
    CDBG_ERROR("%s: stats parsing not done", __func__);
    return 0;
  }

  ctrl->stats_proc_ctrl.intf.input.mctl_info.numRegions =
    vfe_stats_out->numRegions;

  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_BE */

/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_BG -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_BG(void *parm1, void *parm2)
{
  int rc = 0;
  mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
  vfe_stats_output_t *vfe_stats_out =
    ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
  int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;

  parse_stats = (!ctrl->stats_proc_ctrl.intf.output.af_d.active) ||
                  (ctrl->stats_proc_ctrl.intf.output.af_d.cont_af_enabled);

  vfe_stats_out->aec_bg_done = FALSE;
  rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
         ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
         STATS_TYPE_BAYER, parm2,
         (void *)vfe_stats_out);
  if(rc < 0) {
    CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
    return rc;
  }
  if(!vfe_stats_out->aec_bg_done) {
    CDBG_ERROR("%s: stats parsing not done", __func__);
    return 0;
  }

  ctrl->stats_proc_ctrl.intf.input.mctl_info.numRegions =
    vfe_stats_out->numRegions;

  if(parse_stats)
    //mctl_stats_do_aec(ctrl, (void *)vfe_stats_out);

  //mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AEC_CMD);
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
} /* mctl_stats_proc_MSG_ID_STATS_BG */

/*===========================================================================
 * FUNCTION    - mctl_stats_proc_MSG_ID_STATS_BHIST -
 *
 * DESCRIPTION:
 *==========================================================================*/
int mctl_stats_proc_MSG_ID_STATS_BHIST(void *parm1, void *parm2)
{
    int rc = 0;
    int i = 0;
    mctl_config_ctrl_t *ctrl = (mctl_config_ctrl_t *)parm1;
    vfe_stats_output_t *vfe_stats_out =
      ctrl->stats_proc_ctrl.intf.input.mctl_info.vfe_stats_out;
    int parse_stats = (ctrl->state == CAMERA_STATE_STARTED) ? 1 : 0;
    CDBG("%s: E, rc = %d\n", __func__, rc);

    vfe_stats_out->bhist_done = FALSE;
    rc = ctrl->comp_ops[MCTL_COMPID_VFE].parse_stats(
           ctrl->comp_ops[MCTL_COMPID_VFE].handle, parse_stats,
           STATS_TYPE_BAYER, parm2,
           (void *)vfe_stats_out);
    if(rc < 0) {
      CDBG_ERROR("%s: stats parsing error = %d",  __func__, rc);
      return rc;
    }
    if(!vfe_stats_out->bhist_done) {
      CDBG_ERROR("%s: stats parsing not done", __func__);
      return 0;
    }
    if(parse_stats)
      //mctl_do_af(ctrl);
    mctl_eztune_update_diagnostics(EZ_MCTL_ISP_AF_CMD);
    CDBG("%s: X, rc = %d\n", __func__, rc);
    return rc;
} /* mctl_stats_proc_MSG_ID_STATS_BHIST */
