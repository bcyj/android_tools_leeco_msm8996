/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "ihist_stats.h"
#include "isp_log.h"


/** ihist_stats_get_shiftbits:
 *    @entry: pointer to instance private data
 *
 * Get shift bits.
 *
 * This function executes in ISP thread context
 *
 * Return shift bits
 **/
static uint32_t ihist_stats_get_shiftbits(ISP_StatsIhist_CfgType *pcmd)
{
  return pcmd->shiftBits;
}

/** ihist_stats_config:
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
static int ihist_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  ISP_StatsIhist_CfgType *pcmd = entry->reg_cmd;
  uint32_t window_w_t, window_h_t, total_pixels;
  int32_t shift_bits;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: ihist not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;
  if (pix_settings->outputs[0].stream_param.width > 0) {
    window_w_t = pix_settings->outputs[0].stream_param.width;
    window_h_t = pix_settings->outputs[0].stream_param.height;
  } else {
    CDBG_ERROR("%s: error, width = 0\n", __func__);
    return -1;
  }
  pcmd->channelSelect = 0;
  pcmd->rgnHNum = FLOOR16(window_w_t/2) - 1;
  pcmd->rgnVNum = FLOOR16(window_h_t/2) - 1;

  total_pixels = (float)((pcmd->rgnHNum + 1) * (pcmd->rgnVNum + 1)) / 2.0;
  shift_bits = CEIL_LOG2(total_pixels);
  shift_bits -= 16;
  shift_bits = MAX(0, shift_bits);
  shift_bits = MIN(4, shift_bits);

  pcmd->shiftBits = shift_bits;
  pcmd->siteSelect = 0;

  entry->hw_update_pending = 1;
  return 0;
}

/** ihist_stats_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int ihist_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->enable = in_params->enable;
  /* after enable the first stats is corrupted */
  entry->is_first = 1;
  return 0;
}

/** ihist_stats_trigger_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set trigger enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int ihist_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** ihist_stats_set_params:
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
static int ihist_stats_set_params(void *ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = ihist_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = ihist_stats_config(entry, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
    rc = ihist_stats_trigger_enable(entry, (isp_mod_set_enable_t *)in_params);
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

/** ihist_stats_get_params:
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
static int ihist_stats_get_params(void *ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size, void *out_params,
  uint32_t out_param_size)
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

/** ihist_stats_do_hw_update:
 *    @entry: pointer to instance private data
 *    @raw_buf: buffer with data for stats hw
 *    @bg_stats: output buffer to 3A module
 *
 * Parse BG statistics.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int ihist_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsIhist_CfgType);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = IHIST_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = IHIST_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    entry->hw_update_pending = 0;
  }

  return rc;
}

/** ihist_stats_parse:
 *    @entry: pointer to instance private data
 *    @raw_buf: buffer with data for stats hw
 *    @ihist_stats: output buffer to 3A module
 *
 * Parse IHIST statistics.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int ihist_stats_parse(isp_stats_entry_t *entry, void *raw_buf,
  q3a_ihist_stats_t *ihist_stats)
{
  int i;
  uint16_t *hist_statsBuffer = NULL;

  hist_statsBuffer = (uint16_t *)raw_buf;
  for (i = 0; i < 256; i++) {
    ihist_stats->histogram[i] += *hist_statsBuffer;
    hist_statsBuffer++;
  }

  return 0;
}

/** ihist_stats_action:
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
static int ihist_stats_action(void *ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  switch ((isp_stats_action_code_t)action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    entry->is_first = 1;
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    rc = ihist_stats_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_IHIST_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE: {
    isp_pipeline_stats_parse_t *stats_parser = data;
    mct_event_stats_isp_t *isp_stats_event = stats_parser->parsed_stats_event;
    mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
    int buf_idx = stats_parser->raw_stats_event->
                  u.stats.stats_buf_idxs[MSM_ISP_STATS_IHIST];
    void *raw_buf = isp_get_buf_addr(entry->buf_mgr,
      entry->buf_handle, buf_idx);
    if(!raw_buf){
      CDBG_ERROR("%s: isp_get_buf_addr failed!\n", __func__);
      return -1;
    }

    if (entry->is_first == 1) {
      ISP_DBG(ISP_MOD_STATS, "%s: drop first ihist stats\n", __func__);
      entry->is_first = 0;
      isp_stats_enqueue_buf(entry, buf_idx);
      return rc;
    }
    q3a_ihist_stats_t *ihist_stats = entry->parsed_stats_buf;
    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_IHIST);
    rc = ihist_stats_parse(entry, raw_buf, ihist_stats);
    if (entry->num_bufs != 0) {
      rc |= isp_stats_enqueue_buf(entry, buf_idx);
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_IHIST].stats_buf = ihist_stats;
      stats_data[MSM_ISP_STATS_IHIST].stats_type = MSM_ISP_STATS_IHIST;
      stats_data[MSM_ISP_STATS_IHIST].buf_size = sizeof(q3a_ihist_stats_t);
      stats_data[MSM_ISP_STATS_IHIST].used_size = sizeof(q3a_ihist_stats_t);
    } else {
      stats_data[MSM_ISP_STATS_IHIST].stats_buf = NULL;
      stats_data[MSM_ISP_STATS_IHIST].buf_size = 0;
      stats_data[MSM_ISP_STATS_IHIST].used_size = 0;
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

/** ihist_stats_init:
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
static int ihist_stats_init(void *ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  entry->buf_len = ISP_STATS_IHIST_BUF_SIZE;
  entry->stats_type = MSM_ISP_STATS_IHIST;
  entry->is_first = 1;
  entry->fd = init_params->fd;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  entry->notify_ops = notify_ops;

  return 0;
}

/** ihist_stats_destroy:
 *    @ctrl: pointer to instance private data
 *
 * Free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int ihist_stats_destroy(void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;

  if (entry) {
    free(entry->parsed_stats_buf);
    entry->parsed_stats_buf = NULL;

    free(entry->reg_cmd);
    entry->reg_cmd = NULL;

    free(entry);
    entry = NULL;
  }

  return 0;
}

/** bhist_stats32_open:
 *    @stats: isp module data
 *    @stats_type: statistic type
 *
 * Allocate instance private data for submodule.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *ihist_stats32_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsIhist_CfgType *cmd = NULL;
  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n", __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsIhist_CfgType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(entry);
    return NULL;
  }
  memset(entry, 0, sizeof(*entry));
  memset(cmd, 0, sizeof(*cmd));

  entry->len_parsed_stats_buf = sizeof(q3a_ihist_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf == NULL) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = ihist_stats_init;
  entry->ops.destroy = ihist_stats_destroy;
  entry->ops.set_params = ihist_stats_set_params;
  entry->ops.get_params = ihist_stats_get_params;
  entry->ops.action = ihist_stats_action;
  return &entry->ops;
}


