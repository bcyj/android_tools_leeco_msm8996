/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "bf_stats.h"
#include "isp_log.h"

/** bf_stats_debug:
 *    @pcmd: Pointer to statistic configuration.
 *
 * Print statistic configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void vfe_bf_stats_debug(ISP_StatsBf_CfgCmdType *pcmd)
{
  ISP_DBG(ISP_MOD_STATS, "%s:Bayer Focus Stats Configurations\n", __func__);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHOffset %d\n", __func__, pcmd->rgnHOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVOffset %d\n", __func__, pcmd->rgnVOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnWidth   %d\n", __func__, pcmd->rgnWidth);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHeight  %d\n", __func__, pcmd->rgnHeight);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHNum    %d\n", __func__, pcmd->rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVNum    %d\n", __func__, pcmd->rgnVNum);
  ISP_DBG(ISP_MOD_STATS, "%s:r_fv_min   %d\n", __func__, pcmd->r_fv_min);
  ISP_DBG(ISP_MOD_STATS, "%s:gr_fv_min  %d\n", __func__, pcmd->gr_fv_min);
  ISP_DBG(ISP_MOD_STATS, "%s:b_fv_min   %d\n", __func__, pcmd->b_fv_min);
  ISP_DBG(ISP_MOD_STATS, "%s:gb_fv_min  %d\n", __func__, pcmd->gb_fv_min);
  ISP_DBG(ISP_MOD_STATS, "%s:a00        %d\n", __func__, pcmd->a00);
  ISP_DBG(ISP_MOD_STATS, "%s:a01        %d\n", __func__, pcmd->a01);
  ISP_DBG(ISP_MOD_STATS, "%s:a02        %d\n", __func__, pcmd->a02);
  ISP_DBG(ISP_MOD_STATS, "%s:a03        %d\n", __func__, pcmd->a03);
  ISP_DBG(ISP_MOD_STATS, "%s:a04        %d\n", __func__, pcmd->a04);
  ISP_DBG(ISP_MOD_STATS, "%s:a10        %d\n", __func__, pcmd->a10);
  ISP_DBG(ISP_MOD_STATS, "%s:a11        %d\n", __func__, pcmd->a11);
  ISP_DBG(ISP_MOD_STATS, "%s:a12        %d\n", __func__, pcmd->a12);
  ISP_DBG(ISP_MOD_STATS, "%s:a13        %d\n", __func__, pcmd->a13);
  ISP_DBG(ISP_MOD_STATS, "%s:a14        %d\n", __func__, pcmd->a14);
}

/** bf_stats_debug_2x13:
 *    @pcmd: Pointer to statistic configuration.
 *
 * Print statistic configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/

static void vfe_bf_stats_debug_2x13(ISP_StatsBf_CfgCmdType_2x13 *pcmd)
{
  ISP_DBG(ISP_MOD_STATS, "%s:BF Stats Configurations for 2x13 filter\n", __func__);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHOffset %d\n", __func__, pcmd->rgnHOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVOffset %d\n", __func__, pcmd->rgnVOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnWidth   %d\n", __func__, pcmd->rgnWidth);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHeight  %d\n", __func__, pcmd->rgnHeight);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHNum    %d\n", __func__, pcmd->rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVNum    %d\n", __func__, pcmd->rgnVNum);
  ISP_DBG(ISP_MOD_STATS, "%s:r_fv_min   %d\n", __func__, pcmd->r_fv_min);
  ISP_DBG(ISP_MOD_STATS, "%s:gr_fv_min  %d\n", __func__, pcmd->gr_fv_min);
  ISP_DBG(ISP_MOD_STATS, "%s:b_fv_min   %d\n", __func__, pcmd->b_fv_min);
  ISP_DBG(ISP_MOD_STATS, "%s:gb_fv_min  %d\n", __func__, pcmd->gb_fv_min);
  ISP_DBG(ISP_MOD_STATS, "%s:a000        %d\n", __func__, pcmd->a000);
  ISP_DBG(ISP_MOD_STATS, "%s:a001        %d\n", __func__, pcmd->a001);
  ISP_DBG(ISP_MOD_STATS, "%s:a002        %d\n", __func__, pcmd->a002);
  ISP_DBG(ISP_MOD_STATS, "%s:a003        %d\n", __func__, pcmd->a003);
  ISP_DBG(ISP_MOD_STATS, "%s:a004        %d\n", __func__, pcmd->a004);
  ISP_DBG(ISP_MOD_STATS, "%s:a005        %d\n", __func__, pcmd->a005);
  ISP_DBG(ISP_MOD_STATS, "%s:a006        %d\n", __func__, pcmd->a006);
  ISP_DBG(ISP_MOD_STATS, "%s:a007        %d\n", __func__, pcmd->a007);
  ISP_DBG(ISP_MOD_STATS, "%s:a008        %d\n", __func__, pcmd->a008);
  ISP_DBG(ISP_MOD_STATS, "%s:a009        %d\n", __func__, pcmd->a009);
  ISP_DBG(ISP_MOD_STATS, "%s:a010        %d\n", __func__, pcmd->a010);
  ISP_DBG(ISP_MOD_STATS, "%s:a011        %d\n", __func__, pcmd->a011);
  ISP_DBG(ISP_MOD_STATS, "%s:a012        %d\n", __func__, pcmd->a012);
  ISP_DBG(ISP_MOD_STATS, "%s:a100        %d\n", __func__, pcmd->a100);
  ISP_DBG(ISP_MOD_STATS, "%s:a101        %d\n", __func__, pcmd->a101);
  ISP_DBG(ISP_MOD_STATS, "%s:a102        %d\n", __func__, pcmd->a102);
  ISP_DBG(ISP_MOD_STATS, "%s:a103        %d\n", __func__, pcmd->a103);
  ISP_DBG(ISP_MOD_STATS, "%s:a104        %d\n", __func__, pcmd->a104);
  ISP_DBG(ISP_MOD_STATS, "%s:a105        %d\n", __func__, pcmd->a105);
  ISP_DBG(ISP_MOD_STATS, "%s:a106        %d\n", __func__, pcmd->a106);
  ISP_DBG(ISP_MOD_STATS, "%s:a107        %d\n", __func__, pcmd->a107);
  ISP_DBG(ISP_MOD_STATS, "%s:a108        %d\n", __func__, pcmd->a108);
  ISP_DBG(ISP_MOD_STATS, "%s:a109        %d\n", __func__, pcmd->a109);
  ISP_DBG(ISP_MOD_STATS, "%s:a110        %d\n", __func__, pcmd->a110);
  ISP_DBG(ISP_MOD_STATS, "%s:a111        %d\n", __func__, pcmd->a111);
  ISP_DBG(ISP_MOD_STATS, "%s:a112        %d\n", __func__, pcmd->a112);
}

/*===========================================================================
 * FUNCTION    - bf_stats_check_stream_path -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int bf_stats_check_stream_path(
  isp_hw_pix_setting_params_t *pix_settings, uint32_t stream_id)
{
   int i;
   int rc = 0;
   uint32_t path_idx;

   if (stream_id == 0) {
     path_idx = ISP_PIX_PATH_ENCODER;
   } else {
      for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
         if (pix_settings->outputs[i].stream_param.stream_id == stream_id) {
           path_idx = i;
           return path_idx;
         }
      }
      CDBG_ERROR("%s: no match stream for BF, default encoder path\n", __func__);
      return ISP_PIX_PATH_ENCODER;
   }

  return path_idx;
}

/** bf_stats_config:
 *    @entry: pointer to instance private data
 *    @pix_settings: input data
 *    @in_param_size: size of input data
 *
 * Configure submodule.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  int rc = 0;
  ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;
  uint32_t camif_window_w_t, camif_window_h_t;
  isp_pix_camif_cfg_t *camif_cfg = &pix_settings->camif_cfg;
  isp_pixel_window_info_t scaler_output[ISP_PIX_PATH_MAX];
  isp_pixel_line_info_t fov_output[ISP_PIX_PATH_MAX];
  af_config_t af_hw_cfg;
  af_config_t *af_config = &pix_settings->stats_cfg.af_config;
  uint32_t af_rgn_width, af_rgn_height;
  int i;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: BF not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;

  /*revert crop and then revert scaling to conert to bf hw config*/
  af_hw_cfg.roi.left = pix_settings->stats_cfg.af_config.roi.left;
  af_hw_cfg.roi.top = pix_settings->stats_cfg.af_config.roi.top;
  if (af_hw_cfg.roi.left == 0 || af_hw_cfg.roi.top == 0) {
    ISP_DBG(ISP_MOD_STATS, "bf_stats_config:return: %d %d\n", af_hw_cfg.roi.left, af_hw_cfg.roi.top);
    return 0;
  }
  af_hw_cfg.grid_info.h_num =
    pix_settings->stats_cfg.af_config.grid_info.h_num;
  af_hw_cfg.grid_info.v_num =
    pix_settings->stats_cfg.af_config.grid_info.v_num;
  af_hw_cfg.roi.width =
    pix_settings->stats_cfg.af_config.roi.width / af_hw_cfg.grid_info.h_num;
  af_hw_cfg.roi.height =
    pix_settings->stats_cfg.af_config.roi.height / af_hw_cfg.grid_info.v_num;

  camif_window_w_t = camif_cfg->sensor_out_info.request_crop.last_pixel -
                     camif_cfg->sensor_out_info.request_crop.first_pixel + 1;
  camif_window_h_t = camif_cfg->sensor_out_info.request_crop.last_line -
                     camif_cfg->sensor_out_info.request_crop.first_line + 1;

  af_rgn_width =
    af_config->roi.width / af_config->grid_info.h_num;
  af_rgn_height =
    af_config->roi.height / af_config->grid_info.v_num;

  /* min of pcmd->rgnHOffset = 4, min of pcmd->rgnVOffset = 2
   * based on system's input */
  pcmd->rgnHOffset = (af_hw_cfg.roi.left < 4) ? 4 : FLOOR2(af_hw_cfg.roi.left);
  pcmd->rgnVOffset = (af_hw_cfg.roi.top < 2) ? 2 : FLOOR2(af_hw_cfg.roi.top);

  pcmd->rgnWidth   = FLOOR2(af_rgn_width) - 1;
  pcmd->rgnHeight  = FLOOR2(af_rgn_height) - 1;
  pcmd->rgnHNum    = af_hw_cfg.grid_info.h_num - 1;
  pcmd->rgnVNum    = af_hw_cfg.grid_info.v_num - 1;
  pcmd->r_fv_min   = 10;
  pcmd->gr_fv_min  = 10;
  pcmd->b_fv_min   = 10;
  pcmd->gb_fv_min  = 10;
  pcmd->a00        = -4;
  pcmd->a01        = 0;
  pcmd->a02        = -2;
  pcmd->a03        = 0;
  pcmd->a04        = -4;
  pcmd->a10        = -1;
  pcmd->a11        = -1;
  pcmd->a12        = 14;
  pcmd->a13        = -1;
  pcmd->a14        = -1;

  af_config->roi.left = pcmd->rgnHOffset;
  af_config->roi.top = pcmd->rgnVOffset;
  af_config->roi.width = (pcmd->rgnWidth + 1) * af_config->grid_info.h_num;
  af_config->roi.height = (pcmd->rgnHeight + 1) * af_config->grid_info.v_num;

  /* size of window should be at least 2 pixels */
  if ((af_hw_cfg.roi.width < 2) ||
      (af_hw_cfg.roi.height < 2)) {
    CDBG_ERROR("%s: AF ROI is too small! ROI: %dx%d grid: %dx%d",
      __func__,
      pix_settings->stats_cfg.af_config.roi.width,
      pix_settings->stats_cfg.af_config.roi.height,
      af_hw_cfg.grid_info.h_num,
      af_hw_cfg.grid_info.v_num);
    return -1;
  }

  if ((pcmd->rgnHOffset + ((pcmd->rgnWidth  + 1u) * (pcmd->rgnHNum + 1u))) >= camif_window_w_t  ||
      (pcmd->rgnVOffset + ((pcmd->rgnHeight + 1u) * (pcmd->rgnVNum + 1u))) >= camif_window_h_t ) {
    CDBG_ERROR("%s: AF ROI bigger than camif window!", __func__);
    return -1;
  }

  /* rgnHOffset needs to be at least 8 and at most camif_window_width - 2 */
  if (pcmd->rgnHOffset < 8 || pcmd->rgnHOffset >= camif_window_w_t - 2) {
    CDBG_ERROR("%s: Unsupported BF stats region config: invalid offset: %d\n", __func__, pcmd->rgnHOffset);
    return -1;
  }
  entry->hw_update_pending = 1;
  return rc;
}

/** bf_stats33_config:
 *    @entry: pointer to instance private data
 *    @pix_settings: input data
 *    @in_param_size: size of input data
 *
 * Configure submodule.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats33_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  int rc = 0;
  ISP_StatsBf_CfgCmdType_2x13 *pcmd = entry->reg_cmd;
  uint32_t camif_window_w_t, camif_window_h_t;
  isp_pix_camif_cfg_t *camif_cfg = &pix_settings->camif_cfg;
  isp_pixel_window_info_t scaler_output[ISP_PIX_PATH_MAX];
  isp_pixel_line_info_t fov_output[ISP_PIX_PATH_MAX];
  af_config_t af_hw_cfg;
  af_config_t *af_config = &pix_settings->stats_cfg.af_config;
  uint32_t af_rgn_width, af_rgn_height;
  int i;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: BF not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 0;

  /*revert crop and then revert scaling to conert to bf hw config*/
  af_hw_cfg.roi.left = pix_settings->stats_cfg.af_config.roi.left;
  af_hw_cfg.roi.top = pix_settings->stats_cfg.af_config.roi.top;
  if (af_hw_cfg.roi.left == 0 || af_hw_cfg.roi.top == 0) {
    ISP_DBG(ISP_MOD_STATS, "bf_stats_config:return: %d %d\n", af_hw_cfg.roi.left, af_hw_cfg.roi.top);
    return 0;
  }
  af_hw_cfg.grid_info.h_num =
    pix_settings->stats_cfg.af_config.grid_info.h_num;
  af_hw_cfg.grid_info.v_num =
    pix_settings->stats_cfg.af_config.grid_info.v_num;
  af_hw_cfg.roi.width =
    pix_settings->stats_cfg.af_config.roi.width / af_hw_cfg.grid_info.h_num;
  af_hw_cfg.roi.height =
    pix_settings->stats_cfg.af_config.roi.height / af_hw_cfg.grid_info.v_num;

  camif_window_w_t = camif_cfg->sensor_out_info.request_crop.last_pixel -
                     camif_cfg->sensor_out_info.request_crop.first_pixel + 1;
  camif_window_h_t = camif_cfg->sensor_out_info.request_crop.last_line -
                     camif_cfg->sensor_out_info.request_crop.first_line + 1;

  af_rgn_width =
    af_config->roi.width / af_config->grid_info.h_num;
  af_rgn_height =
    af_config->roi.height / af_config->grid_info.v_num;

  /* min of pcmd->rgnHOffset = 24, min of pcmd->rgnVOffset = 2
   * based on HW input */
  pcmd->rgnHOffset = (af_hw_cfg.roi.left < 24) ? 24 : FLOOR2(af_hw_cfg.roi.left);
  pcmd->rgnVOffset = (af_hw_cfg.roi.top < 2) ? 2 : FLOOR2(af_hw_cfg.roi.top);

  pcmd->rgnWidth   = FLOOR2(af_rgn_width) - 1;
  pcmd->rgnHeight  = FLOOR2(af_rgn_height) - 1;
  pcmd->rgnHNum    = af_hw_cfg.grid_info.h_num - 1;
  pcmd->rgnVNum    = af_hw_cfg.grid_info.v_num - 1;
  pcmd->r_fv_min   = pix_settings->stats_cfg.af_config.r_min;;
  pcmd->gr_fv_min  = pix_settings->stats_cfg.af_config.gr_min;
  pcmd->b_fv_min   = pix_settings->stats_cfg.af_config.b_min;
  pcmd->gb_fv_min  = pix_settings->stats_cfg.af_config.gb_min;
  pcmd->a000       = pix_settings->stats_cfg.af_config.hpf[0];
  pcmd->a001       = pix_settings->stats_cfg.af_config.hpf[1];
  pcmd->a002       = pix_settings->stats_cfg.af_config.hpf[2];
  pcmd->a003       = pix_settings->stats_cfg.af_config.hpf[3];
  pcmd->a004       = pix_settings->stats_cfg.af_config.hpf[4];
  pcmd->a005       = pix_settings->stats_cfg.af_config.hpf[5];
  pcmd->a006       = pix_settings->stats_cfg.af_config.hpf[6];
  pcmd->a007       = pix_settings->stats_cfg.af_config.hpf[7];
  pcmd->a008       = pix_settings->stats_cfg.af_config.hpf[8];
  pcmd->a009       = pix_settings->stats_cfg.af_config.hpf[9];
  pcmd->a010       = pix_settings->stats_cfg.af_config.hpf[10];
  pcmd->a011       = pix_settings->stats_cfg.af_config.hpf[11];
  pcmd->a012       = pix_settings->stats_cfg.af_config.hpf[12];
  pcmd->a100       = pix_settings->stats_cfg.af_config.hpf[13];
  pcmd->a101       = pix_settings->stats_cfg.af_config.hpf[14];
  pcmd->a102       = pix_settings->stats_cfg.af_config.hpf[15];
  pcmd->a103       = pix_settings->stats_cfg.af_config.hpf[16];
  pcmd->a104       = pix_settings->stats_cfg.af_config.hpf[17];
  pcmd->a105       = pix_settings->stats_cfg.af_config.hpf[18];
  pcmd->a106       = pix_settings->stats_cfg.af_config.hpf[19];
  pcmd->a107       = pix_settings->stats_cfg.af_config.hpf[20];
  pcmd->a108       = pix_settings->stats_cfg.af_config.hpf[21];
  pcmd->a109       = pix_settings->stats_cfg.af_config.hpf[22];
  pcmd->a110       = pix_settings->stats_cfg.af_config.hpf[23];
  pcmd->a111       = pix_settings->stats_cfg.af_config.hpf[24];
  pcmd->a112       = pix_settings->stats_cfg.af_config.hpf[25];

  af_config->roi.left = pcmd->rgnHOffset;
  af_config->roi.top = pcmd->rgnVOffset;
  af_config->roi.width = (pcmd->rgnWidth + 1) * af_config->grid_info.h_num;
  af_config->roi.height = (pcmd->rgnHeight + 1) * af_config->grid_info.v_num;

  /* size of window should be at least 2 pixels */
  if ((af_hw_cfg.roi.width < 2) ||
      (af_hw_cfg.roi.height < 2)) {
    CDBG_ERROR("%s: AF ROI is too small! ROI: %dx%d grid: %dx%d",
      __func__,
      pix_settings->stats_cfg.af_config.roi.width,
      pix_settings->stats_cfg.af_config.roi.height,
      af_hw_cfg.grid_info.h_num,
      af_hw_cfg.grid_info.v_num);
    return -1;
  }

  if ((pcmd->rgnHOffset + ((pcmd->rgnWidth  + 1u) * (pcmd->rgnHNum + 1u))) >= camif_window_w_t  ||
      (pcmd->rgnVOffset + ((pcmd->rgnHeight + 1u) * (pcmd->rgnVNum + 1u))) >= camif_window_h_t ) {
    CDBG_ERROR("%s: AF ROI bigger than camif window!", __func__);
    return -1;
  }

  /* rgnHOffset needs to be at least 24 and at most camif_window_width - 2 */
  if (pcmd->rgnHOffset < 24 || pcmd->rgnHOffset >= camif_window_w_t - 2) {
    CDBG_ERROR("%s: Unsupported BF stats region config: invalid offset: %d\n", __func__, pcmd->rgnHOffset);
    return -1;
  }
  entry->hw_update_pending = 1;
  return rc;
}

/** bf_stats_start:
 *    @entry: pointer to instance private data
 *    @start: start or stop flag
 *
 * Start/Stop stream.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_start(isp_stats_entry_t *entry, boolean start)
{
  int rc = 0;

  /* ioctl */
  return rc;
}

/** bf_stats_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->enable = in_params->enable;
  return 0;
}

/** bf_stats_trigger_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set trigger enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** bf_stats_set_params:
 *    @mod_ctrl: pointer to instance private data
 *    @params_id: parameter ID
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Set parameter function. It handle all input parameters.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_set_params(void *ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = bf_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = bf_stats_config(entry, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
    rc = bf_stats_trigger_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_TRIGGER_UPDATE:
    break;
  case ISP_STATS_SET_STREAM_CFG:
    break;
  case ISP_STATS_SET_STREAM_UNCFG:
    break;
  default:
    break;
  }
  return rc;
}

/** bf_stats33_set_params:
 *    @mod_ctrl: pointer to instance private data
 *    @params_id: parameter ID
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Set parameter function. It handle all input parameters.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats33_set_params(void *ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = bf_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = bf_stats33_config(entry, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
    rc = bf_stats_trigger_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_TRIGGER_UPDATE:
    break;
  case ISP_STATS_SET_STREAM_CFG:
    break;
  case ISP_STATS_SET_STREAM_UNCFG:
    break;
  default:
    break;
  }
  return rc;
}

/** bf_stats_get_params:
 *    @mod_ctrl: pointer to instance private data
 *    @params_id: parameter ID
 *    @in_params: input data
 *    @in_params_size: size of input data
 *    @out_params: output data
 *    @out_params_size: size of output data
 *
 * Get parameter function. It handle all parameters.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_get_params(void *ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_STATS_GET_ENABLE:
    break;
  case ISP_STATS_GET_STREAM_STATE:
    break;
  case ISP_STATS_GET_PARSED_STATS:
    break;
  case ISP_STATS_GET_STREAM_HANDLE: {
    uint32_t *handle = (uint32_t *)(out_params);
    *handle = entry->stream_handle;
    break;
  }
  default:
    break;
  }
  return rc;
}

/** bf_stats_do_hw_update:
 *    @entry: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  vfe_bf_stats_debug(pcmd);
  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsBf_CfgCmdType);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BF_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = BF_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    entry->hw_update_pending = 0;
  }

  return rc;
}

/** bf_stats33_do_hw_update:
 *    @entry: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats33_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  ISP_StatsBf_CfgCmdType_2x13 *pcmd = entry->reg_cmd;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  vfe_bf_stats_debug_2x13(pcmd);
  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsBf_CfgCmdType_2x13);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BF_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = BF_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 24;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BF_STATS33_OFF_2;
    reg_cfg_cmd[0].u.rw_info.len = BF_STATS33_LEN_2 * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    entry->hw_update_pending = 0;
  }

  return rc;
}

/** bf_stats_parse:
 *    @entry: pointer to instance private data
 *    @raw_buf: buffer with data for stats hw
 *    @bf_stats: output buffer to 3A module
 *
 * Parse BG statistics.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_parse(isp_stats_entry_t *entry, void *raw_buf,
  q3a_bf_stats_t *bf_stats)
{
  int window;
  uint32_t i, j;
  uint32_t *Sr,*Sb, *Sgr, *Sgb;
  uint32_t *r_sh, *b_sh, *gr_sh, *gb_sh;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;
  uint32_t *current_region = NULL;

  Sr     = bf_stats->bf_r_sum;
  Sb     = bf_stats->bf_b_sum;
  Sgr    = bf_stats->bf_gr_sum;
  Sgb    = bf_stats->bf_gb_sum;
  r_sh   = bf_stats->bf_r_sharp;
  b_sh   = bf_stats->bf_b_sharp;
  gr_sh  = bf_stats->bf_gr_sharp;
  gb_sh  = bf_stats->bf_gb_sharp;
  r_num  = bf_stats->bf_r_num;
  b_num  = bf_stats->bf_b_num;
  gr_num = bf_stats->bf_gr_num;
  gb_num = bf_stats->bf_gb_num;

  current_region = raw_buf;
  bf_stats->bf_region_h_num = pcmd->rgnHNum + 1;
  bf_stats->bf_region_v_num = pcmd->rgnVNum + 1;

  for (i = 0; i < ((pcmd->rgnHNum + 1u) * (pcmd->rgnVNum + 1u)); i++) {
    *Sr = ((*(current_region)) & 0x00FFFFFF);
    Sr++;
    current_region++;
    *Sb = ((*(current_region)) & 0x00FFFFFF);
    Sb++;
    current_region++;
    *Sgr = ((*(current_region)) & 0x00FFFFFF);
    Sgr++;
    current_region++;
    *Sgb = ((*(current_region)) & 0x00FFFFFF);
    Sgb++;
    current_region++;
    *r_sh = *current_region;
    r_sh++;
    current_region++;
    *b_sh = *current_region;
    b_sh++;
    current_region++;
    *gr_sh = *current_region;
    gr_sh++;
    current_region++;
    *gb_sh = *current_region;
    gb_sh++;
    current_region++;
    *r_num = ((*(current_region)) & 0x0000FFFF);
    *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
    r_num++;
    b_num++;
    current_region++;
    *gr_num = ((*(current_region)) & 0x0000FFFF);
    *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;
    gr_num++;
    gb_num++;
    current_region++;
  }
  return 0;
}

/** bf_stats33_parse:
 *    @entry: pointer to instance private data
 *    @raw_buf: buffer with data for stats hw
 *    @bf_stats: output buffer to 3A module
 *
 * Parse BG statistics.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats33_parse(isp_stats_entry_t *entry, void *raw_buf,
  q3a_bf_stats_t *bf_stats)
{
  int window;
  uint32_t i, j;
  uint32_t *Sr,*Sb, *Sgr, *Sgb;
  uint32_t *r_sh, *b_sh, *gr_sh, *gb_sh;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  ISP_StatsBf_CfgCmdType_2x13 *pcmd = entry->reg_cmd;
  uint32_t *current_region = NULL;

  Sr     = bf_stats->bf_r_sum;
  Sb     = bf_stats->bf_b_sum;
  Sgr    = bf_stats->bf_gr_sum;
  Sgb    = bf_stats->bf_gb_sum;
  r_sh   = bf_stats->bf_r_sharp;
  b_sh   = bf_stats->bf_b_sharp;
  gr_sh  = bf_stats->bf_gr_sharp;
  gb_sh  = bf_stats->bf_gb_sharp;
  r_num  = bf_stats->bf_r_num;
  b_num  = bf_stats->bf_b_num;
  gr_num = bf_stats->bf_gr_num;
  gb_num = bf_stats->bf_gb_num;

  current_region = raw_buf;
  bf_stats->bf_region_h_num = pcmd->rgnHNum + 1;
  bf_stats->bf_region_v_num = pcmd->rgnVNum + 1;

  for (i = 0; i < ((pcmd->rgnHNum + 1u) * (pcmd->rgnVNum + 1u)); i++) {
    *Sr = ((*(current_region)) & 0x00FFFFFF);
    Sr++;
    current_region++;
    *Sb = ((*(current_region)) & 0x00FFFFFF);
    Sb++;
    current_region++;
    *Sgr = ((*(current_region)) & 0x00FFFFFF);
    Sgr++;
    current_region++;
    *Sgb = ((*(current_region)) & 0x00FFFFFF);
    Sgb++;
    current_region++;
    *r_sh = *current_region;
    r_sh++;
    current_region++;
    *b_sh = *current_region;
    b_sh++;
    current_region++;
    *gr_sh = *current_region;
    gr_sh++;
    current_region++;
    *gb_sh = *current_region;
    gb_sh++;
    current_region++;
    *r_num = ((*(current_region)) & 0x0000FFFF);
    *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
    r_num++;
    b_num++;
    current_region++;
    *gr_num = ((*(current_region)) & 0x0000FFFF);
    *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;
    gr_num++;
    gb_num++;
    current_region++;
  }
  return 0;
}

/** bf_stats_action:
 *    @mod_ctrl: pointer to instance private data
 *    @action_code: action id
 *    @data: action data
 *    @data_size: action data size
 *
 * Handle all actions.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_action(void *ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  switch ((isp_stats_action_code_t)action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    rc = bf_stats_start(entry, 1);
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    rc = bf_stats_start(entry, 0);
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    rc = bf_stats_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_BF_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE: {
    isp_pipeline_stats_parse_t *action_data = data;
    mct_event_stats_isp_t *isp_stats_event = action_data->parsed_stats_event;
    mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
    int buf_idx =
      action_data->raw_stats_event->u.stats.stats_buf_idxs[MSM_ISP_STATS_BF];
    void *raw_buf = isp_get_buf_addr(entry->buf_mgr,
      entry->buf_handle, buf_idx);
    if(!raw_buf){
      CDBG_ERROR("%s: isp_get_buf_addr failed!\n", __func__);
      return -1;
    }

    q3a_bf_stats_t *bf_stats = entry->parsed_stats_buf;

    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_BF);
    rc = bf_stats_parse(entry, raw_buf, bf_stats);
    if (entry->num_bufs != 0) {
      rc |= isp_stats_enqueue_buf(entry, buf_idx);
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_BF].stats_buf = bf_stats;
      stats_data[MSM_ISP_STATS_BF].stats_type = MSM_ISP_STATS_BF;
      stats_data[MSM_ISP_STATS_BF].buf_size = sizeof(q3a_bf_stats_t);
      stats_data[MSM_ISP_STATS_BF].used_size = sizeof(q3a_bf_stats_t);
    } else {
      stats_data[MSM_ISP_STATS_BF].stats_buf = NULL;
      stats_data[MSM_ISP_STATS_BF].buf_size = 0;
      stats_data[MSM_ISP_STATS_BF].used_size = 0;
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

/** bf_stats33_action:
 *    @mod_ctrl: pointer to instance private data
 *    @action_code: action id
 *    @data: action data
 *    @data_size: action data size
 *
 * Handle all actions.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats33_action(void *ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  switch ((isp_stats_action_code_t)action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    rc = bf_stats_start(entry, 1);
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    rc = bf_stats_start(entry, 0);
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    rc = bf_stats33_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_BF_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE: {
    isp_pipeline_stats_parse_t *action_data = data;
    mct_event_stats_isp_t *isp_stats_event = action_data->parsed_stats_event;
    mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
    int buf_idx =
      action_data->raw_stats_event->u.stats.stats_buf_idxs[MSM_ISP_STATS_BF];
    void *raw_buf = isp_get_buf_addr(entry->buf_mgr,
      entry->buf_handle, buf_idx);
    if(!raw_buf){
      CDBG_ERROR("%s: isp_get_buf_addr failed!\n", __func__);
      return -1;
    }

    q3a_bf_stats_t *bf_stats = entry->parsed_stats_buf;

    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_BF);
    rc = bf_stats33_parse(entry, raw_buf, bf_stats);
    if (entry->num_bufs != 0) {
      rc |= isp_stats_enqueue_buf(entry, buf_idx);
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_BF].stats_buf = bf_stats;
      stats_data[MSM_ISP_STATS_BF].stats_type = MSM_ISP_STATS_BF;
      stats_data[MSM_ISP_STATS_BF].buf_size = sizeof(q3a_bf_stats_t);
      stats_data[MSM_ISP_STATS_BF].used_size = sizeof(q3a_bf_stats_t);
    } else {
      stats_data[MSM_ISP_STATS_BF].stats_buf = NULL;
      stats_data[MSM_ISP_STATS_BF].buf_size = 0;
      stats_data[MSM_ISP_STATS_BF].used_size = 0;
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

/** bf_stats_init:
 *    @mod_ctrl: pointer to instance private data
 *    @in_params: input data
 *    @notify_ops: notify operations
 *
 * Initialize private data.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_init(void *ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;

  entry->fd = init_params->fd;
  entry->stats_type = MSM_ISP_STATS_BF;
  entry->buf_len = ISP_STATS_BF_BUF_SIZE;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;

  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  pcmd->rgnWidth   = 0;
  pcmd->rgnHeight  = 0;
  pcmd->rgnHNum    = 17;
  pcmd->rgnVNum    = 13;
  pcmd->r_fv_min   = 0;
  pcmd->gr_fv_min  = 0;
  pcmd->b_fv_min   = 0;
  pcmd->gb_fv_min  = 0;
  pcmd->a00        = 0;
  pcmd->a01        = 0;
  pcmd->a02        = 0;
  pcmd->a03        = 0;
  pcmd->a04        = 0;
  pcmd->a10        = 0;
  pcmd->a11        = 0;
  pcmd->a12        = 0;
  pcmd->a13        = 0;
  pcmd->a14        = 0;

  return 0;
}

/** bf_stats33_init:
 *    @mod_ctrl: pointer to instance private data
 *    @in_params: input data
 *    @notify_ops: notify operations
 *
 * Initialize private data.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats33_init(void *ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsBf_CfgCmdType_2x13 *pcmd = entry->reg_cmd;

  entry->fd = init_params->fd;
  entry->stats_type = MSM_ISP_STATS_BF;
  entry->buf_len = ISP_STATS_BF_BUF_SIZE;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;

  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  pcmd->rgnWidth   = 0;
  pcmd->rgnHeight  = 0;
  pcmd->rgnHNum    = 17;
  pcmd->rgnVNum    = 13;
  pcmd->r_fv_min   = 0;
  pcmd->gr_fv_min  = 0;
  pcmd->b_fv_min   = 0;
  pcmd->gb_fv_min  = 0;
  pcmd->a000       = 0;
  pcmd->a001       = 0;
  pcmd->a002       = 0;
  pcmd->a003       = 0;
  pcmd->a004       = 0;
  pcmd->a005       = 0;
  pcmd->a006       = 0;
  pcmd->a007       = 0;
  pcmd->a008       = 0;
  pcmd->a009       = 0;
  pcmd->a010       = 0;
  pcmd->a011       = 0;
  pcmd->a012       = 0;
  pcmd->a100       = 0;
  pcmd->a101       = 0;
  pcmd->a102       = 0;
  pcmd->a103       = 0;
  pcmd->a104       = 0;
  pcmd->a105       = 0;
  pcmd->a106       = 0;
  pcmd->a107       = 0;
  pcmd->a108       = 0;
  pcmd->a109       = 0;
  pcmd->a110       = 0;
  pcmd->a111       = 0;
  pcmd->a112       = 0;

  return 0;
}

/** bf_stats_destroy:
 *    @ctrl: pointer to instance private data
 *
 * Free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bf_stats_destroy(void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  free(entry);
  return 0;
}

/** bf_stats32_open:
 *    @stats: isp module data
 *    @stats_type: statistic type
 *
 * Allocate instance private data for submodule.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *bf_stats32_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsBf_CfgCmdType *cmd = NULL;

  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n", __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsBf_CfgCmdType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(entry);
    return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));

  entry->len_parsed_stats_buf = sizeof(q3a_bf_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf == NULL) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = bf_stats_init;
  entry->ops.destroy = bf_stats_destroy;
  entry->ops.set_params = bf_stats_set_params;
  entry->ops.get_params = bf_stats_get_params;
  entry->ops.action = bf_stats_action;
  return &entry->ops;
}

/** bf_stats33_open:
 *    @stats: isp module data
 *    @stats_type: statistic type
 *
 * Allocate instance private data for submodule.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *bf_stats33_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsBf_CfgCmdType_2x13 *cmd = NULL;

  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n", __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsBf_CfgCmdType_2x13));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(entry);
    return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));

  entry->len_parsed_stats_buf = sizeof(q3a_bf_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf == NULL) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = bf_stats33_init;
  entry->ops.destroy = bf_stats_destroy;
  entry->ops.set_params = bf_stats33_set_params;
  entry->ops.get_params = bf_stats_get_params;
  entry->ops.action = bf_stats33_action;
  return &entry->ops;
}
