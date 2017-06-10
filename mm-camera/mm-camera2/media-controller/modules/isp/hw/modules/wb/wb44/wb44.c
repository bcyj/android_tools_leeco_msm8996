/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "camera_dbg.h"
#include "wb44.h"
#include "isp_log.h"

#ifdef WB_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

#define WB_START_REG 0x384


/** wb_debug:
 *    @p_cmd: config cmd to be printed
 *
 *  Print the values in config cmd
 *
 *
 *  Return void
 **/
static void wb_debug(ISP_WhiteBalanceConfigCmdType* p_cmd)
{
  ISP_DBG(ISP_MOD_WB, "ISP_WhiteBalanceCfgCmd.ch0Gain = %d\n",
    p_cmd->ch0Gain);
  ISP_DBG(ISP_MOD_WB, "ISP_WhiteBalanceCfgCmd.ch1Gain = %d\n",
    p_cmd->ch1Gain);
  ISP_DBG(ISP_MOD_WB, "ISP_WhiteBalanceCfgCmd.ch2Gain = %d\n",
    p_cmd->ch2Gain);
}/*wb_debug*/

/** wb_update_hw_gain_reg:
 *    @wb_mod: Pointer to wb module
 *
 *  Update the gain registers in mod
 *
 *
 *  Return void
 **/
static void wb_update_hw_gain_reg(isp_wb_mod_t *wb_mod)
{
  wb_mod->ISP_WhiteBalanceCfgCmd.ch0Gain =
    WB_GAIN(wb_mod->awb_gain.g_gain * wb_mod->dig_gain);
  wb_mod->ISP_WhiteBalanceCfgCmd.ch1Gain =
    WB_GAIN(wb_mod->awb_gain.b_gain * wb_mod->dig_gain);
  wb_mod->ISP_WhiteBalanceCfgCmd.ch2Gain =
    WB_GAIN(wb_mod->awb_gain.r_gain * wb_mod->dig_gain);
}

/** wb_set_manual_wb:
 *    @mod: Pointer to wb module
 *    @trigger_params: pipeline trigger params
 *    @in_param_size:
 *
 *  Set manual gain values in mod before stream start
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int wb_set_manual_wb(isp_wb_mod_t *mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  int rc = 0;
  chromatix_parms_type *chroma_ptr =
   (chromatix_parms_type *)trigger_params->cfg.chromatix_ptrs.chromatixPtr;

  awb_gain_t *awb_gain = (awb_gain_t*) &trigger_params->trigger_input.
    stats_update.awb_update.gain;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }

  ISP_DBG(ISP_MOD_WB, "%s: old g= %f b= %f r= %f new g= %f b= %f r= %f", __func__,
  mod->awb_gain.g_gain, mod->awb_gain.b_gain,
  mod->awb_gain.r_gain, awb_gain->g_gain,
  awb_gain->b_gain, awb_gain->r_gain);

  /* Agreed sequence with pix pipeline before stream started is that,
   * (1) config wb
   * (2) set manual WB
   * (3) action_hw_update.
   * After stream started, set_wb uses trigger_update the wb gain. */

  if (IS_MANUAL_WB((trigger_params))) {
    mod->awb_gain.g_gain = awb_gain->g_gain;
    mod->awb_gain.b_gain = awb_gain->b_gain;
    mod->awb_gain.r_gain = awb_gain->r_gain;
    wb_update_hw_gain_reg(mod);
    mod->hw_update_pending = TRUE;
  }

  return rc;
} /*wb_set_manual_wb*/

/** wb_trigger_update:
 *    @wb_mod: Pointer to wb module
 *    @trigger_params: Pipeline trigger update params
 *    @in_params_size:
 *
 *  Perform trigger update using awb_update
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int wb_trigger_update(isp_wb_mod_t *wb_mod,
  isp_pix_trigger_update_input_t *trigger_params, uint32_t in_param_size)
{
  int rc = 0;
  awb_gain_t *awb_gain =
     (awb_gain_t*) &trigger_params->trigger_input.stats_update.awb_update.gain;

  if (in_param_size != sizeof(isp_pix_trigger_update_input_t)) {
    /* size mismatch */
    CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
      __func__, sizeof(isp_mod_set_enable_t), in_param_size);
    return -1;
  }

  if (!wb_mod->enable || !wb_mod->trigger_enable) {
      ISP_DBG(ISP_MOD_WB, "%s: enable = %d, trigger_enable = %d",
         __func__, wb_mod->enable, wb_mod->trigger_enable);
      return 0;
  }

  ISP_DBG(ISP_MOD_WB, "%s: old gain  g=%f b=%f r=%f new g=%f b=%f r=%f", __func__,
    wb_mod->awb_gain.g_gain, wb_mod->awb_gain.b_gain,
    wb_mod->awb_gain.r_gain, awb_gain->g_gain,
    awb_gain->b_gain, awb_gain->r_gain);

   if (!WB_GAIN_EQUAL(awb_gain, wb_mod->awb_gain) && !WB_GAIN_EQ_ZERO(awb_gain)) {
    wb_mod->awb_gain.g_gain = awb_gain->g_gain;
    wb_mod->awb_gain.b_gain = awb_gain->b_gain;
    wb_mod->awb_gain.r_gain = awb_gain->r_gain;
    wb_update_hw_gain_reg(wb_mod);
    wb_mod->hw_update_pending = TRUE;
  }

  return rc;
} /* wb_trigger_update */

/** wb_set_bestshot:
 *    @mod: Pointer to wb module
 *    @pix_settings: Pipeline settings
 *    @in_param_size:
 *
 *  Set wb mode based on best shot mode
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int wb_set_bestshot(isp_wb_mod_t *mod,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  int rc =0;
  chromatix_parms_type *chromatix_ptr =
     (chromatix_parms_type *)pix_settings->chromatix_ptrs.chromatixPtr;

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }

  ISP_DBG(ISP_MOD_WB, "%s: bestshot mode %d", __func__, pix_settings->bestshot_mode);
  switch(pix_settings->bestshot_mode) {
    case CAM_SCENE_MODE_FIREWORKS:
      pix_settings->wb_mode = CAM_WB_MODE_CLOUDY_DAYLIGHT;
      break;
    case CAM_SCENE_MODE_SUNSET:
    case CAM_SCENE_MODE_CANDLELIGHT:
      pix_settings->wb_mode = CAM_WB_MODE_INCANDESCENT;
      break;
    default:
      pix_settings->wb_mode = CAM_WB_MODE_AUTO;
      break;
  }
  return rc;
} /* isp_wb_set_effect */

/** wb_reset:
 *    @mod: Pointer to wb module
 *
 *  Perform reset of wb module
 *
 *
 *  Return void
 **/
static void wb_reset(isp_wb_mod_t *mod)
{
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mod->enable = 0;
  mod->trigger_enable = 0;
  mod->dig_gain = 0.0;
  mod->hw_update_pending = 0;
  memset(&mod->ISP_WhiteBalanceCfgCmd, 0, sizeof(mod->ISP_WhiteBalanceCfgCmd));
  memset(&mod->ISP_WhiteBalanceRightCfgCmd, 0, sizeof(mod->ISP_WhiteBalanceRightCfgCmd));
  memset(&mod->awb_gain, 0, sizeof(mod->awb_gain));
}

/** wb_init:
 *    @mod_ctrl: Pointer to wb module
 *    @in_params: HW mod initialization params
 *    @notify_ops: function ptr to notify ops
 *
 *  Initialize the wb module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int wb_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_wb_mod_t *wb = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  wb->fd = init_params->fd;
  wb->notify_ops = notify_ops;
  wb->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  wb_reset(wb);
  return 0;
}/* wb_init */

/** wb_config:
 *    @mod: Pointer to wb module
 *    @in_params: Pipeline settings
 *    @in_param_size:
 *
 *  Configure the wb module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int wb_config(isp_wb_mod_t *mod, isp_hw_pix_setting_params_t *in_params,
  uint32_t in_param_size)
{
  int  rc = 0;
  uint32_t i;
  chromatix_parms_type *chroma_ptr =
   (chromatix_parms_type *)in_params->chromatix_ptrs.chromatixPtr;
  chromatix_MWB_type *chromatix_MWB = &chroma_ptr->chromatix_MWB;
  Bayer_AWB_parameters_type *AWB_bayer_algo_data =
    &chroma_ptr->AWB_bayer_algo_data;

  ISP_DBG(ISP_MOD_WB, "%s\n",__func__);

  if (in_param_size != sizeof(isp_hw_pix_setting_params_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_hw_pix_setting_params_t), in_param_size);
  return -1;
  }

  mod->awb_gain.b_gain = chromatix_MWB->MWB_tl84.b_gain *
    AWB_bayer_algo_data->gain_adj[AGW_AWB_WARM_FLO].blue_gain_adj;
  mod->awb_gain.r_gain = chromatix_MWB->MWB_tl84.b_gain *
    AWB_bayer_algo_data->gain_adj[AGW_AWB_WARM_FLO].red_gain_adj;
  mod->awb_gain.g_gain = chromatix_MWB->MWB_tl84.b_gain;

  mod->dig_gain= 1.0; //no apply aec gain for now, could be used in the future


  mod->awb_gain.g_gain = ISP_WB40_INIT_G_GAIN;
  mod->awb_gain.b_gain = ISP_WB40_INIT_B_GAIN;
  mod->awb_gain.r_gain = ISP_WB40_INIT_R_GAIN;

  wb_update_hw_gain_reg(mod);

  mod->hw_update_pending = TRUE;

  return rc;
}

/** wb_enable:
 *    @wb: Pointer to wb module
 *    @enable: enable flag
 *    @in_param_size:
 *
 *  Enable wb module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int wb_enable(isp_wb_mod_t *wb,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  wb->enable = enable->enable;

  return 0;
}

/** wb_trigger_enable:
 *    @wb: Pointer to wb module
 *    @enable: enable flag
 *    @in_param_size:
 *
 *  Enable trigger update for wb module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int wb_trigger_enable(isp_wb_mod_t *wb,
                isp_mod_set_enable_t *enable,
                uint32_t in_param_size)
{
  if (in_param_size != sizeof(isp_mod_set_enable_t)) {
  /* size mismatch */
  CDBG_ERROR("%s: size mismatch, expecting = %d, received = %d",
         __func__, sizeof(isp_mod_set_enable_t), in_param_size);
  return -1;
  }
  wb->trigger_enable = enable->enable;

  return 0;
}

/** wb_destroy:
 *    @mod_ctrl: Pointer to wb module
 *
 *  Deallocate and destroy the module
 *
 *
 *  return 0 on Success, negative on ERROR
 **/
static int wb_destroy (void *mod_ctrl)
{
  isp_wb_mod_t *wb = mod_ctrl;

  memset(wb,  0,  sizeof(isp_wb_mod_t));
  free(wb);
  return 0;
}

/** wb_set_params
 *    @mod_ctrl: Pointer to wb module struct
 *    @param_id: event id indicating what value is set
 *    @in_params: input event params
 *    @in_param_size: size of input params
 *
 *  Set value for parameter given by param id and pass input params
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int wb_set_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_wb_mod_t *wb = mod_ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = wb_enable(wb, (isp_mod_set_enable_t *)in_params,
                     in_param_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = wb_config(wb, (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = wb_trigger_enable(wb, (isp_mod_set_enable_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_UPDATE:
    rc = wb_trigger_update(wb, (isp_pix_trigger_update_input_t *)in_params, in_param_size);
    break;
  case ISP_HW_MOD_SET_BESTSHOT:
     rc = wb_set_bestshot(wb, (isp_hw_pix_setting_params_t *)in_params, in_param_size);
     break;
  case ISP_HW_MOD_SET_MANUAL_WB:
     rc = wb_set_manual_wb(wb, (isp_pix_trigger_update_input_t *)in_params, in_param_size);
     break;
  default:
    CDBG_ERROR("%s: param_id is not supported in this module\n", __func__);
    break;
  }
  return rc;
}

/** wb_get_params:
 *    @mod_ctrl: Pointer to wb module struct
 *    @param_id: event id indicating what param to get
 *    @in_params: input params
 *    @in_param_size: Size of Input Params
 *    @out_params: output params
 *    @out_param_size: size of output params
 *
 *  Get value of parameter given by param id
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int wb_get_params (void *mod_ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  int rc =0;
  isp_wb_mod_t *wb = mod_ctrl;

  switch (param_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE: {
    isp_mod_get_enable_t *enable = out_params;

    if (sizeof(isp_mod_get_enable_t) != out_param_size) {
      CDBG_ERROR("%s: error, out_param_size mismatch, param_id = %d",
                 __func__, param_id);
      break;
    }
    enable->enable = wb->enable;
    break;
  }
  default:
    rc = -EPERM;
    break;
  }
  return rc;
}

/** wb_do_hw_update:
 *    @wb_mod: Pointer to wb module
 *
 *  Method called at SOF, writes the value in config cmd to HW
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int wb_do_hw_update(isp_wb_mod_t *wb_mod)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (wb_mod->hw_update_pending) {
    cfg_cmd.cfg_data = (void *) &wb_mod->ISP_WhiteBalanceCfgCmd;
    cfg_cmd.cmd_len = sizeof(wb_mod->ISP_WhiteBalanceCfgCmd);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_WB40_OFF;
    reg_cfg_cmd[0].u.rw_info.len = ISP_WB40_LEN * sizeof(uint32_t);

    wb_debug(&wb_mod->ISP_WhiteBalanceCfgCmd);
    rc = ioctl(wb_mod->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    wb_mod->hw_update_pending = 0;
  }

  return rc;
}

/** wb_action:
 *    @mod_ctrl: Pointer to wb module struct
 *    @action_code:  indicates what action is required
 *    @data: input param
 *    @data_size: size of input param
 *
 *  Do the required actions for event given by action code
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int wb_action (void *mod_ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_wb_mod_t *wb = mod_ctrl;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    rc = wb_do_hw_update(wb);
    break;
  case ISP_HW_MOD_ACTION_RESET:
    wb_reset(wb);
    break;
  default:
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
    break;
  }
  return rc;
}

/** wb44_open:
 *    @version:
 *
 *  Allocate and Initialize the wb module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *wb44_open(uint32_t version)
{
  isp_wb_mod_t *wb = malloc(sizeof(isp_wb_mod_t));

  if (!wb) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(wb,  0,  sizeof(isp_wb_mod_t));
  wb->ops.ctrl = (void *)wb;
  wb->ops.init = wb_init;
  /* destroy the module object */
  wb->ops.destroy = wb_destroy;
  /* set parameter */
  wb->ops.set_params = wb_set_params;
  /* get parameter */
  wb->ops.get_params = wb_get_params;
  wb->ops.action = wb_action;
  return &wb->ops;
}

