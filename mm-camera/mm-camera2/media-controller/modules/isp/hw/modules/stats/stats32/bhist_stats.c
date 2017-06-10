/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "bhist_stats.h"
#include "isp_log.h"

/** bhist_stats_debug:
 *    @pcmd: Pointer to statistic configuration.
 *
 * Print statistic configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void bhist_stats_debug(ISP_StatsBhist_CfgCmdType *pcmd)
{
  ISP_DBG(ISP_MOD_STATS, "%s:Bayer Histogram Stats Configurations\n", __func__);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHOffset %d\n", __func__, pcmd->rgnHOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVOffset %d\n", __func__, pcmd->rgnVOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHNum    %d\n", __func__, pcmd->rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVNum    %d\n", __func__, pcmd->rgnVNum);
}

/** bhist_stats_config:
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
static int bhist_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  ISP_StatsBhist_CfgCmdType *pcmd = entry->reg_cmd;
  isp_pix_camif_cfg_t *camif_cfg = &pix_settings->camif_cfg;
  uint32_t camif_window_w_t, camif_window_h_t;
  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: Bhist stats not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;
  camif_window_w_t = camif_cfg->sensor_out_info.request_crop.last_pixel
    - camif_cfg->sensor_out_info.request_crop.first_pixel + 1;
  camif_window_h_t = camif_cfg->sensor_out_info.request_crop.last_line
    - camif_cfg->sensor_out_info.request_crop.first_line + 1;

  ISP_DBG(ISP_MOD_STATS, "%s:\n", __func__);
  ISP_DBG(ISP_MOD_STATS, "camif_window_w_t : %u\n", camif_window_w_t);
  ISP_DBG(ISP_MOD_STATS, "camif_window_h_t : %u\n", camif_window_h_t);
  pcmd->rgnHOffset = FLOOR2(camif_window_w_t%2);
  pcmd->rgnVOffset = FLOOR2(camif_window_h_t%2);
  pcmd->rgnHNum = FLOOR2(camif_window_w_t/2) - 1;
  pcmd->rgnVNum = FLOOR2(camif_window_h_t/2) - 1;
  bhist_stats_debug(pcmd);

  entry->hw_update_pending = 1;
  return 0;
}

/** bhist_stats_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bhist_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: enable = %d\n", __func__, in_params->enable);
  entry->enable = in_params->enable;
  return 0;
}

/** bhist_stats_trigger_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set trigger enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bhist_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: trigger_enable = %d\n", __func__, in_params->enable);
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** bhist_stats_set_params:
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
static int bhist_stats_set_params(void *ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = bhist_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = bhist_stats_config(entry, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
    rc = bhist_stats_trigger_enable(entry, (isp_mod_set_enable_t *)in_params);
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

/** bhist_stats_get_params:
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
static int bhist_stats_get_params(void *ctrl, uint32_t param_id,
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

/** bhist_stats_do_hw_update:
 *    @entry: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bhist_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsBhist_CfgCmdType);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BHIST_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = BHIST_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    entry->hw_update_pending = 0;
    entry->skip_stats = 1;
  }

  return rc;
}

/** bhist_stats_parse:
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
static int bhist_stats_parse(isp_stats_entry_t *entry, void *raw_buf,
  q3a_bhist_stats_t *bhist_stats)
{
  uint32_t * Srh, *Sbh, *Sgrh, *Sgbh;
  uint32_t *current_region;
  uint32_t i;

  Srh = bhist_stats->bayer_r_hist;
  Sbh = bhist_stats->bayer_b_hist;
  Sgrh = bhist_stats->bayer_gr_hist;
  Sgbh = bhist_stats->bayer_gb_hist;
  current_region = raw_buf;
  for (i = 0; i < 256; i++) { //0 to 255, total 256 bins
    *Srh = ((*(current_region)) & 0x007FFFFF);
    Srh++;
    current_region++;
    *Sbh = ((*(current_region)) & 0x007FFFFF);
    Sbh++;
    current_region++;
    *Sgrh = ((*(current_region)) & 0x007FFFFF);
    Sgrh++;
    current_region++;
    *Sgbh = ((*(current_region)) & 0x007FFFFF);
    Sgbh++;
    current_region++;
  }
  return 0;
}

/** bg_stats_action:
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
static int bhist_stats_action(void *ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  ISP_DBG(ISP_MOD_STATS, "%s: action code = %d\n", __func__, action_code);
  switch ((isp_stats_action_code_t)action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    rc = bhist_stats_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_BHIST_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE: {
    isp_pipeline_stats_parse_t *action_data = data;
    mct_event_stats_isp_t *isp_stats_event = action_data->parsed_stats_event;
    mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
    int buf_idx =
      action_data->raw_stats_event->u.stats.stats_buf_idxs[MSM_ISP_STATS_BHIST];
    void *raw_buf = isp_get_buf_addr(entry->buf_mgr,
      entry->buf_handle, buf_idx);
    if(!raw_buf){
      CDBG_ERROR("%s: isp_get_buf_addr failed!\n", __func__);
      return -1;
    }
     if (entry->is_first == 1 || entry->skip_stats == 1) {
        ISP_DBG(ISP_MOD_STATS, "%s: drop first stats\n", __func__);
        entry->is_first = 0;
        entry->skip_stats = 0;
        isp_stats_enqueue_buf(entry, buf_idx);
        return rc;
    }

    q3a_bhist_stats_t *bhist_stats = entry->parsed_stats_buf;
    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_BHIST);
    rc = bhist_stats_parse(entry, raw_buf, bhist_stats);
    if (entry->num_bufs != 0) {
      rc |= isp_stats_enqueue_buf(entry, buf_idx);
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_BHIST].stats_buf = bhist_stats;
      stats_data[MSM_ISP_STATS_BHIST].stats_type = MSM_ISP_STATS_BHIST;
      stats_data[MSM_ISP_STATS_BHIST].buf_size = sizeof(q3a_bhist_stats_t);
      stats_data[MSM_ISP_STATS_BHIST].used_size = sizeof(q3a_bhist_stats_t);
    } else {
      stats_data[MSM_ISP_STATS_BHIST].stats_buf = NULL;
      stats_data[MSM_ISP_STATS_BHIST].buf_size = 0;
      stats_data[MSM_ISP_STATS_BHIST].used_size = 0;
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

/** bhist_stats_init:
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
static int bhist_stats_init(void *ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsBhist_CfgCmdType *pcmd = entry->reg_cmd;

  entry->buf_len = ISP_STATS_BHIST_BUF_SIZE;
  entry->stats_type = MSM_ISP_STATS_BHIST;
  entry->fd = init_params->fd;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  entry->skip_stats = 0;

  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  pcmd->rgnHNum = 0;
  pcmd->rgnVNum = 0;

  return 0;
}

/** bhist_stats_destroy:
 *    @ctrl: pointer to instance private data
 *
 * Free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bhist_stats_destroy(void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  free(entry);
  return 0;
}

/** bhist_stats_open:
 *    @stats: isp module data
 *    @stats_type: statistic type
 *
 * Allocate instance private data for submodule.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *bhist_stats32_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsBhist_CfgCmdType *cmd = NULL;

  ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);
  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n", __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsBhist_CfgCmdType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(entry);
    return NULL;
  }
  memset(entry, 0, sizeof(*entry));
  memset(cmd, 0, sizeof(*cmd));

  entry->len_parsed_stats_buf = sizeof(q3a_bhist_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf == NULL) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = bhist_stats_init;
  entry->ops.destroy = bhist_stats_destroy;
  entry->ops.set_params = bhist_stats_set_params;
  entry->ops.get_params = bhist_stats_get_params;
  entry->ops.action = bhist_stats_action;
  return &entry->ops;
}


