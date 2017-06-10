/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "mctl_divert.h"
#include "mctl_ez.h"
#include "eztune.h"
#include "camera_dbg.h"
#include "eztune_preview.h"
#include <media/msm_camera.h>

static eztune_t eztune_Ctrl;
static eztune_prev_t eztune_prev_Ctrl;

/*===========================================================================
 * FUNCTION     - close_clientsocket -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static void  close_clientsocket(eztune_t *ezctrl)
{
  uint32_t status = FALSE;
  int client_socket_id = -1;
  write(ezctrl->pipewrite_fd, &client_socket_id, sizeof(int));
  close(ezctrl->clientsocket_id);
  free(ezctrl->protocol_ptr);
  ezctrl->status = status;
  mctl_eztune_set_vfe(
    VFE_MODULE_ALL, SET_STATUS, status);
  mctl_eztune_set_3A(EZ_STATUS, status);
}

/*===========================================================================
 * FUNCTION     - close_prev_clientsocket -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static void  close_prev_clientsocket(eztune_prev_t *ezctrl)
{
  uint32_t status = FALSE;
  int client_socket_id = -1;
  write(ezctrl->prev_pipewrite_fd, &client_socket_id, sizeof(int));
  close(ezctrl->prev_clientsocket_id);
  free(ezctrl->prev_protocol_ptr);
  ezctrl->prev_protocol_ptr = NULL;
}

/*===========================================================================
 * FUNCTION     - mctl_eztune_server_connect -
 *
 * DESCRIPTION:
 * ==========================================================================*/

void mctl_eztune_server_connect(m_ctrl_t *mctl, int fd){
  int rc = 0;
  uint32_t status = TRUE;
  vfe_eztune_info_t info;

  if(mctl == NULL) {
    CDBG_ERROR("%s null poiner\n", __func__);
    return;
  }
  eztune_t *ezctrl = &eztune_Ctrl;
  ezctrl->mctl_ptr = mctl->p_cfg_ctrl;
  ezctrl->pipewrite_fd = mctl->cfg_arg.ez_write_fd;
  ezctrl->clientsocket_id = fd;
  CDBG("__debug %s starting\n", __func__);
  rc = eztune_server_connect(ezctrl);
  if(rc < 0) {
    CDBG("__debug %s Closing Client Socket\n", __func__);
    close_clientsocket(ezctrl);
    return;
  }
  ezctrl->status = status;
  mctl_eztune_set_vfe(
    VFE_MODULE_ALL, SET_STATUS, status);
  mctl_eztune_set_3A(EZ_STATUS, status);
  rc = ezctrl->mctl_ptr->comp_ops[MCTL_COMPID_VFE].get_params(
    ezctrl->mctl_ptr->comp_ops[MCTL_COMPID_VFE].handle, VFE_GET_DIAGNOSTICS_PTR,
    (void *)(&info), sizeof(info));
  ezctrl->diagnostics_ptr = (vfe_diagnostics_t *)(info.diag_ptr);
  ezctrl->chromatix_ptr = ezctrl->mctl_ptr->chromatix_ptr;
  ezctrl->af_tune_ptr = ezctrl->mctl_ptr->af_tune_ptr;
  mctl_eztune_update_diagnostics(EZ_MCTL_ISP_DEFAULT);
  CDBG("__debug %s ending\n", __func__);
}

/*===========================================================================
 * FUNCTION     - mctl_eztune_prev_server_connect -
 *
 * DESCRIPTION:
 * ==========================================================================*/
void mctl_eztune_prev_server_connect(m_ctrl_t *mctl, int fd)
{
  int rc = 0;
  if(mctl == NULL) {
    CDBG_ERROR("%s null poiner\n", __func__);
    return;
  }
  eztune_prev_Ctrl.prev_pipewrite_fd = mctl->cfg_arg.ez_prev_write_fd;
  eztune_prev_Ctrl.prev_clientsocket_id = fd;
  eztune_prev_Ctrl.mctl = mctl->p_cfg_ctrl;
  eztune_prev_Ctrl.prev_protocol_ptr = malloc(sizeof(eztune_prev_protocol_t));
  if (!eztune_prev_Ctrl.prev_protocol_ptr) {
    CDBG_EZ("%s malloc returns NULL\n", __FUNCTION__);
    close_prev_clientsocket(&eztune_prev_Ctrl);
    return;
  }

  rc = eztune_init_preview_settings(
    eztune_prev_Ctrl.prev_protocol_ptr,
    &mctl->p_cfg_ctrl->dimInfo);
  if(rc < 0) {
    CDBG_EZ("%s eztune_init_preview_settings failed\n", __FUNCTION__);
    close_prev_clientsocket(&eztune_prev_Ctrl);
    return;
  }

  eztune_prev_init_protocol_data(eztune_prev_Ctrl.prev_protocol_ptr);

  eztune_prev_Ctrl.mctl->eztune_preview_flag = 1;
  mctl_divert_set_key(eztune_prev_Ctrl.mctl, FP_PREVIEW_SET);

}

/*===========================================================================
 * FUNCTION     - mctl_eztune_read_and_proc_cmd -
 *
 * DESCRIPTION: mctl eztune command handler
 * ==========================================================================*/

void mctl_eztune_read_and_proc_cmd(mctl_ez_command cmd) {
  int rc = 0;
  eztune_t *ezctrl = &eztune_Ctrl;
  mctl_config_ctrl_t *mctl = ezctrl->mctl_ptr;
  switch(cmd) {
    case EZ_MCTL_SOCKET_CMD:
      CDBG("__debug %s EZ_MCTL_SOCKET_CMD \n", __func__);
      rc = eztune_server_readwrite(ezctrl);
      if(rc < 0) {
        CDBG("__debug %s Closing Client Socket\n", __func__);
        close_clientsocket(ezctrl);
      }
      break;
    case EZ_MCTL_VFE_CMD:
    case EZ_MCTL_ISP3A_CMD:
    case EZ_MCTL_SENSOR_CMD:
    case EZ_MCTL_CONFIG_CMD:
    case EZ_MCTL_PP_CMD:
      break;
    case EZ_MCTL_PREV_SOCKET_CMD:
      if(eztune_prev_Ctrl.prev_protocol_ptr) {
        rc = eztune_preview_server_run(eztune_prev_Ctrl.prev_protocol_ptr,
                                       eztune_prev_Ctrl.prev_clientsocket_id);
        if(rc < 0) {
          eztune_prev_Ctrl.mctl->eztune_preview_flag = 0;
          mctl_divert_set_key(eztune_prev_Ctrl.mctl, FP_PREVIEW_RESET);
          close_prev_clientsocket(&eztune_prev_Ctrl);
        }
      }
      break;
  }
  CDBG("__debug %s ending \n", __func__);
}

/*===========================================================================
 * FUNCTION     - mctl_eztune_set_vfe -
 *
 * DESCRIPTION: sets the vfe to enable / disable modules.
 * ==========================================================================*/
void mctl_eztune_set_vfe(vfemodule_t module, optype_t optype, int32_t value)
{
  eztune_t *ezctrl = &eztune_Ctrl;
  mctl_config_ctrl_t *mctl = ezctrl->mctl_ptr;
  ez_vfecmd_t ezcmd;
  int rc = 0;

  ezcmd.module = module;
  ezcmd.type = optype;
  rc = mctl->comp_ops[MCTL_COMPID_VFE].set_params(
     mctl->comp_ops[MCTL_COMPID_VFE].handle, VFE_SET_EZTUNE,
     (void *)&ezcmd, (void *)&value);
}

/*===========================================================================
 * FUNCTION     - mctl_eztune_set_aftuning -
 *
 * DESCRIPTION: sets  the auto focus tuning .
 * ==========================================================================*/
void mctl_eztune_set_aftuning(aftuning_optype_t optype, uint8_t value)
{
  eztune_t *ezctrl = &eztune_Ctrl;
  mctl_config_ctrl_t *mctl = ezctrl->mctl_ptr;
  actuator_params_type_t actype = 0;
  actuator_set_t actuator_set;
  switch (optype) {
    case EZ_AF_LOADPARAMS:
      actype = ACTUATOR_LOAD_PARAMS;
      break;
    case EZ_AF_LINEARTEST_ENABLE:
      actype = ACTUATOR_TEST_LINEAR;
      actuator_set.data.test.stepsize=
        ezctrl->af_tuning.linearstepsize;
      break;
    case EZ_AF_RINGTEST_ENABLE:
      actuator_set.data.test.stepsize=
        ezctrl->af_tuning.ringstepsize;
      actype = ACTUATOR_TEST_RING;
      break;
    case EZ_AF_MOVFOCUSTEST_ENABLE:
      actype = ACTUATOR_MOVE_FOCUS;
      actuator_set.data.move.direction =
        ezctrl->af_tuning.movfocdirection;
      actuator_set.data.move.num_steps =
        ezctrl->af_tuning.movfocsteps;
      break;
    case EZ_AF_DEFFOCUSTEST_ENABLE:
      actype = ACTUATOR_DEF_FOCUS;
      break;
  }

  mctl->comp_ops[MCTL_COMPID_ACTUATOR].set_params(
    mctl->comp_ops[MCTL_COMPID_ACTUATOR].handle, actype, (void *)&actuator_set, NULL);

}
/*===========================================================================
 * FUNCTION     - mctl_eztune_set_3A -
 *
 * DESCRIPTION: sets  the 3A  enable / disable .
 * ==========================================================================*/
void mctl_eztune_set_3A(isp_set_optype_t optype, int32_t value)
{
  eztune_t *ezctrl = &eztune_Ctrl;
  mctl_config_ctrl_t *mctl = ezctrl->mctl_ptr;
  stats_proc_ctrl_t *sp_ctrl = &(mctl->stats_proc_ctrl);
  ez_3a_params_t *aaa_diagnostics = &(ezctrl->diagnostics_3a);
  stats_proc_set_t stat_set_param;

  switch (optype) {
    case EZ_STATUS:
      stat_set_param.type = STATS_PROC_EZ_TUNE_TYPE;
      stat_set_param.d.set_aec.type = EZ_TUNE_ENABLE;
      stat_set_param.d.set_eztune.d.eztune_enable = value;
      break;
    case EZ_AEC_ENABLE:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_DISABLE;
      stat_set_param.d.set_aec.d.ez_disable = !value;
      aaa_diagnostics->aec_params.enable = value;
      break;
    case EZ_AEC_TESTENABLE:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_TEST_ENABLE;
      stat_set_param.d.set_aec.d.ez_test_enable = value;
      aaa_diagnostics->aec_params.test_enable = value;
      break;
    case EZ_AEC_LOCK:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_LOCK_OUTPUT;
      stat_set_param.d.set_aec.d.ez_lock_output = value;
      aaa_diagnostics->aec_params.lock = value;
      break;
    case EZ_AEC_FORCEPREVEXPOSURE:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_FORCE_EXP;
      stat_set_param.d.set_aec.d.ez_force_exp = value;
      aaa_diagnostics->aec_params.prev_forceexp = value;
      break;
    case EZ_AEC_FORCEPREVGAIN:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_FORCE_GAIN;
      stat_set_param.d.set_aec.d.ez_force_gain = (float)(value)/Q10;
      aaa_diagnostics->aec_params.force_prevgain = (float)(value)/Q10;
      break;
    case EZ_AEC_FORCEPREVLINECOUNT:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_FORCE_LINECOUNT;
      stat_set_param.d.set_aec.d.ez_force_linecount = value;
      aaa_diagnostics->aec_params.force_prevlinecount= value;
      break;
    case EZ_AEC_FORCESNAPEXPOSURE:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_FORCE_SNAPSHOT;
      stat_set_param.d.set_aec.d.ez_force_snapshot = value;
      aaa_diagnostics->aec_params.snap_forceexp = value;
      break;
    case EZ_AEC_FORCESNAPGAIN:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_FORCE_SNAP_GAIN;
      stat_set_param.d.set_aec.d.ez_force_snap_gain = (float)(value)/Q10;
      aaa_diagnostics->aec_params.force_snapgain = (float)(value)/Q10;
      break;
    case EZ_AEC_FORCESNAPLINECOUNT:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_EZ_FORCE_SNAP_LINECOUNT;
      stat_set_param.d.set_aec.d.ez_force_snap_linecount = value;
      aaa_diagnostics->aec_params.force_snaplinecount = value;
      break;
    case EZ_AWB_ENABLE:
      stat_set_param.type = STATS_PROC_AWB_TYPE;
      stat_set_param.d.set_awb.type = AWB_EZ_DISABLE;
      stat_set_param.d.set_awb.d.ez_disable = value;
      aaa_diagnostics->awb_params.enable = value;
      break;
    case EZ_AWB_CONTROLENABLE:
      stat_set_param.type = STATS_PROC_AWB_TYPE;
      stat_set_param.d.set_awb.type = AWB_EZ_LOCK_OUTPUT;
      stat_set_param.d.set_awb.d.ez_lock_output = value;
      aaa_diagnostics->awb_params.lock = value;
      break;
    case EZ_AF_ENABLE:
      stat_set_param.type = STATS_PROC_AF_TYPE;
      stat_set_param.d.set_af.type = AF_EZ_DISABLE;
      stat_set_param.d.set_af.d.ez_disable = value;
      aaa_diagnostics->af_params.enable = value;
      break;
    case EZ_AEC_ABENABLE:
      stat_set_param.type = STATS_PROC_AEC_TYPE;
      stat_set_param.d.set_aec.type = AEC_ANTIBANDING;
      if (value)
        stat_set_param.d.set_aec.d.aec_atb = CAMERA_ANTIBANDING_OFF;
      else
        stat_set_param.d.set_aec.d.aec_atb = CAMERA_ANTIBANDING_AUTO;
      aaa_diagnostics->aec_params.antibanding_enable = value;
      break;
    case EZ_RELOAD_CHROMATIX:
      stat_set_param.type = STATS_PROC_CHROMATIX_RELOAD_TYPE;
      break;
  }
  mctl->comp_ops[MCTL_COMPID_STATSPROC].set_params(
    mctl->comp_ops[MCTL_COMPID_STATSPROC].handle,
    stat_set_param.type, &stat_set_param, &(sp_ctrl->intf));
}

/*===========================================================================
 * FUNCTION     - mctl_eztune_get_misc -
 *
 * DESCRIPTION: sets the vfe to enable / disable modules.
 * ==========================================================================*/
void mctl_eztune_get_misc_sensor(miscoptype_t optype)
{
  eztune_t *ezctrl = &eztune_Ctrl;
  mctl_config_ctrl_t *mctl = ezctrl->mctl_ptr;
  ez_config_params_t *config_diagnostics = &(ezctrl->diagnostics_conf);
  ez_sensor_params_t *sensor_diagnostics = &(ezctrl->diagnostics_sens);
  sensor_get_t sensor_get;

  switch (optype) {
    case EZ_MISC_PREVIEW_RESOLUTION:
      config_diagnostics->disp_width = mctl->dimInfo.display_width;
      config_diagnostics->disp_height = mctl->dimInfo.display_height;
      break;
    case EZ_MISC_SNAPSHOT_RESOLUTION:
      config_diagnostics->pict_width = mctl->dimInfo.picture_width;
      config_diagnostics->pict_height = mctl->dimInfo.picture_height;
      break;
    case EZ_MISC_CURRENT_RESOLUTION:
      config_diagnostics->curr_width = mctl->dimInfo.raw_picture_width;
      config_diagnostics->curr_height = mctl->dimInfo.raw_picture_height;
      break;
    case EZ_MISC_SENSOR_TYPE: {
      mctl->comp_ops[MCTL_COMPID_SENSOR].get_params(
        mctl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_GET_OUTPUT_CFG, &sensor_get, sizeof(sensor_get));
      sensor_diagnostics->sensor_type =
        sensor_get.data.sensor_output.output_format;
      break; }
    case EZ_MISC_SENSOR_FORMAT: {
      mctl->comp_ops[MCTL_COMPID_SENSOR].get_params(
        mctl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_GET_CAMIF_CFG, &sensor_get, sizeof(sensor_get));
      sensor_diagnostics->format =
        sensor_get.data.camif_setting.format;
      break; }
    case EZ_MISC_SENSOR_FULLHEIGHT:
    case EZ_MISC_SENSOR_FULLWIDTH: {
      sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_SNAPSHOT;
      mctl->comp_ops[MCTL_COMPID_SENSOR].get_params(
        mctl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
      sensor_diagnostics->fullsize_width = sensor_get.data.sensor_dim.width;
      sensor_diagnostics->fullsize_height = sensor_get.data.sensor_dim.height;
      break;}
    case EZ_MISC_SENSOR_QTRHEIGHT:
    case EZ_MISC_SENSOR_QTRWIDTH: {
      sensor_get.data.sensor_dim.op_mode = SENSOR_MODE_PREVIEW;
      mctl->comp_ops[MCTL_COMPID_SENSOR].get_params(
        mctl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_GET_DIM_INFO, &sensor_get, sizeof(sensor_get));
      sensor_diagnostics->qtrsize_width = sensor_get.data.sensor_dim.width;
      sensor_diagnostics->qtrsize_height = sensor_get.data.sensor_dim.height;
      break;}
    case EZ_MISC_SENSOR_PIXELSPERLINE:
    case EZ_MISC_SENSOR_PIXELCLKFREQ: {
      sensor_get.data.aec_info.op_mode = SENSOR_MODE_PREVIEW;
      mctl->comp_ops[MCTL_COMPID_SENSOR].get_params(
        mctl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_GET_SENSOR_MODE_AEC_INFO, &sensor_get, sizeof(sensor_get));
      sensor_diagnostics->pixelsperLine =
        sensor_get.data.aec_info.pixels_per_line;
      sensor_diagnostics->pixelclock_freq = sensor_get.data.aec_info.pclk;
      break;}
    case EZ_MISC_SENSOR_LENSSPEC: {
      mctl->comp_ops[MCTL_COMPID_SENSOR].get_params(
        mctl->comp_ops[MCTL_COMPID_SENSOR].handle,
        SENSOR_GET_LENS_INFO, &sensor_get, sizeof(sensor_get));
      sensor_diagnostics->lens_spec.focal_length =
        sensor_get.data.lens_info.focal_length;
      sensor_diagnostics->lens_spec.pixel_size =
        sensor_get.data.lens_info.pix_size;
      sensor_diagnostics->lens_spec.f_number=
        sensor_get.data.lens_info.f_number;
      sensor_diagnostics->lens_spec.total_foc_dist =
        sensor_get.data.lens_info.total_f_dist;
      sensor_diagnostics->lens_spec.hor_view_angle =
        sensor_get.data.lens_info.hor_view_angle;
      sensor_diagnostics->lens_spec.ver_view_angle =
        sensor_get.data.lens_info.ver_view_angle;
      break; }
  }
}

/*===========================================================================
 * FUNCTION     - mctl_eztune_get_3a_pp_diag -
 *
 * DESCRIPTION:
 * ==========================================================================*/
void mctl_eztune_stats_diagnostics(isp_diagtype_t optype)
{
  eztune_t *ezctrl = &eztune_Ctrl;
  mctl_config_ctrl_t *mctl = ezctrl->mctl_ptr;
  stats_proc_ctrl_t *sp_ctrl = &(mctl->stats_proc_ctrl);
  ez_aec_params_t *aec_diagnostics = &(ezctrl->diagnostics_3a.aec_params);
  ez_awb_params_t *awb_diagnostics = &(ezctrl->diagnostics_3a.awb_params);
  ez_af_params_t *af_diagnostics = &(ezctrl->diagnostics_3a.af_params);
  ez_asd_params_t *asd_diagnostics = &(ezctrl->diagnostics_pp.asd_params);
  ez_afd_params_t *afd_diagnostics = &(ezctrl->diagnostics_pp.afd_params);
  stats_proc_interface_output_t *sp_output = &(sp_ctrl->intf.output);

  switch (optype) {
    case EZ_MCTL_ISP_DEFAULT:
      aec_diagnostics->enable = 1;
      aec_diagnostics->lock = 0;
      aec_diagnostics->force_snaplinecount = 0;
      aec_diagnostics->force_prevlinecount = 0;
      aec_diagnostics->force_snapgain = 0;
      aec_diagnostics->force_snapgain = 0;
      aec_diagnostics->prev_forceexp = 0;
      aec_diagnostics->snap_forceexp = 0;
      aec_diagnostics->test_enable = 0;
      aec_diagnostics->antibanding_enable = 0;
      break;
    case EZ_MCTL_ISP_AEC_CMD:
      aec_diagnostics->luma = sp_output->aec_d.cur_luma;
      aec_diagnostics->expindex = sp_output->aec_d.exp_index;
      aec_diagnostics->luxindex = sp_output->aec_d.lux_idx;
      aec_diagnostics->touch_ROIluma = sp_output->aec_d.eztune.touch_roi_luma;
      aec_diagnostics->prev_realgain = sp_output->aec_d.cur_real_gain;
      aec_diagnostics->snap_realgain = sp_output->aec_d.snap.real_gain;
      aec_diagnostics->prev_linecount = sp_output->aec_d.cur_line_cnt;
      aec_diagnostics->snap_linecount = sp_output->aec_d.snap.line_count;
      aec_diagnostics->prev_exposuretime = sp_output->aec_d.cur_line_cnt;
      aec_diagnostics->snap_exposuretime = sp_output->aec_d.snap.exp_time;
      aec_diagnostics->prev_exposuretime = sp_output->aec_d.eztune.preview_exp_time;
      break;
    case EZ_MCTL_ISP_AWB_CMD:
      awb_diagnostics->ymin_pct = sp_output->awb_d.eztune.ymin_pct;
      awb_diagnostics->ave_rg_ratio = sp_output->awb_d.eztune.reg_ave_rg_ratio;
      awb_diagnostics->ave_bg_ratio = sp_output->awb_d.eztune.reg_ave_bg_ratio;
      awb_diagnostics->sgw_rg_ratio = sp_output->awb_d.eztune.sgw_rg_ratio;
      awb_diagnostics->sgw_bg_ratio = sp_output->awb_d.eztune.sgw_bg_ratio;
      awb_diagnostics->white_rg_ratio = sp_output->awb_d.eztune.white_ave_rg_ratio;
      awb_diagnostics->white_bg_ratio = sp_output->awb_d.eztune.white_ave_bg_ratio;
      awb_diagnostics->shifted_D50RG = sp_output->awb_d.eztune.shifted_d50_rg;
      awb_diagnostics->shifted_D50BG = sp_output->awb_d.eztune.shifted_d50_bg;
      awb_diagnostics->outdoor_pos_grcount = sp_output->awb_d.eztune.outdoor_grn_cnt;
      awb_diagnostics->indoor_pos_grcount = sp_output->awb_d.eztune.indoor_grn_cnt;
      awb_diagnostics->color_temp = sp_output->awb_d.color_temp;
      awb_diagnostics->is_compact_cluster = sp_output->awb_d.eztune.compact_cluster;
      awb_diagnostics->white_stat_on = sp_output->awb_d.eztune.current_stat_config;
      awb_diagnostics->decision = sp_output->awb_d.decision;
      awb_diagnostics->prev_wbrgain = sp_output->awb_d.curr_gains.r_gain;
      awb_diagnostics->prev_wbggain = sp_output->awb_d.curr_gains.g_gain;
      awb_diagnostics->prev_wbbgain = sp_output->awb_d.curr_gains.b_gain;
      awb_diagnostics->snap_wbrgain = sp_output->awb_d.snapshot_wb.r_gain;
      awb_diagnostics->snap_wbggain = sp_output->awb_d.snapshot_wb.g_gain;
      awb_diagnostics->snap_wbbgain = sp_output->awb_d.snapshot_wb.b_gain;
      break;
    case EZ_MCTL_ISP_AF_CMD:
      af_diagnostics->peakpos_index = sp_output->af_d.eztune.peakpos_index;
      af_diagnostics->tracing_index = sp_output->af_d.eztune.tracing_index;
      memcpy(af_diagnostics->tracing_stats,
        sp_output->af_d.eztune.tracing_stats,
          sizeof(af_diagnostics->tracing_stats));
      memcpy(af_diagnostics->tracing_pos,
        sp_output->af_d.eztune.tracing_pos,
          sizeof(af_diagnostics->tracing_pos));
      break;
    case EZ_MCTL_ISP_AFD_CMD:
      afd_diagnostics->flicker_detect = sp_output->afd_d.eztune.flicker_detect;
      afd_diagnostics->flicker_freq = sp_output->afd_d.eztune.flicker_freq;
      afd_diagnostics->status = sp_output->afd_d.eztune.status;
      afd_diagnostics->actual_peaks = sp_output->afd_d.eztune.actual_peaks;
      afd_diagnostics->multiple_peak_algo = sp_output->afd_d.eztune.multiple_peak_algo;
      afd_diagnostics->std_width = sp_output->afd_d.eztune.std_width;
      break;
    case EZ_MCTL_ISP_ASD_CMD:
      asd_diagnostics->bls_detected = sp_output->asd_d.backlight_detected;
      asd_diagnostics->bls_histbcklit_detected = sp_output->asd_d.eztune.histo_backlight_detected;
      asd_diagnostics->bls_severity = sp_output->asd_d.backlight_scene_severity;
      asd_diagnostics->bls_mixlightcase = sp_output->asd_d.eztune.mixed_light;
      break;
  }
}

/*===========================================================================
 * FUNCTION     - mctl_eztune_update_diagnostics -
 *
 * DESCRIPTION:
 * ==========================================================================*/
void mctl_eztune_update_diagnostics(isp_diagtype_t optype)
{
  eztune_t *ezctrl = &eztune_Ctrl;
    if(ezctrl->status)
      mctl_eztune_stats_diagnostics(optype);
}
