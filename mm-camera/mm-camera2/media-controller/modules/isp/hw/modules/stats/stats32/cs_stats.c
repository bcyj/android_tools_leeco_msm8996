/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "cs_stats.h"
#include "isp_log.h"


/** cs_stats_config:
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
static int cs_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  uint32_t demosaic_output_width, demosaic_output_height, rgn_width, rgn_height;
  uint32_t rgn_h_num;
  cs_stat_config_type_t *cfg = entry->private;
  ISP_StatsCs_CfgType *pcmd = entry->reg_cmd;
  cs_stat_config_type_t *priv_cfg = entry->private;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: not enabled", __func__);
    return 0;
  }
  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;
  entry->session_id = pix_settings->outputs->stream_param.session_id;

  demosaic_output_width = pix_settings->demosaic_output.last_pixel -
      pix_settings->demosaic_output.first_pixel + 1;
  demosaic_output_height = pix_settings->demosaic_output.last_line -
      pix_settings->demosaic_output.first_line + 1;

  /* VFE32 support only one vertical region */
  rgn_h_num = CS_MAX_H_REGIONS;

  /*config region size, check hw limitation*/
  rgn_width = (demosaic_output_width + rgn_h_num - 1) / rgn_h_num;
  rgn_height = demosaic_output_height;

  /* 1. config rgn height and width
     2. check hw limitation */
  rgn_width = MAX(2, rgn_width);
  rgn_width = MIN(4, rgn_width);
  pcmd->rgnWidth = rgn_width - 1;
  pcmd->rgnHeight = rgn_height - 1;

  /* 1.config region num
     2.modify according to final rgn_width &height
     3.check hw limitation*/
  rgn_h_num = demosaic_output_width / rgn_width;
  rgn_h_num = MIN(CS_MAX_H_REGIONS, rgn_h_num);
  pcmd->rgnHNum = rgn_h_num - 1;

  /*config offset: cs_rgn_offset + (cs_rgn_num + 1) *
    (rs_rgn_width + 1) <= image_width*/
  pcmd->rgnHOffset = (demosaic_output_width % (rgn_width * rgn_h_num)) / 2;
  pcmd->rgnVOffset = 0;

  pcmd->shiftBits = isp_util_calculate_shift_bits(rgn_height);
  priv_cfg->shift_bits = pcmd->shiftBits;

  entry->hw_update_pending = 1;
  return 0;
}

/** cs_stats_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int cs_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->enable = in_params->enable;
  return 0;
}

/** cs_stats_trigger_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set trigger enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int cs_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** cs_stats_set_params:
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
static int cs_stats_set_params(void *ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = cs_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = cs_stats_config(entry, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
    rc = cs_stats_trigger_enable(entry, (isp_mod_set_enable_t *)in_params);
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

/** cs_stats_get_params:
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
static int cs_stats_get_params(void *ctrl, uint32_t param_id, void *in_params,
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
  case ISP_STATS_GET_CS_CONFIG: {
    uint32_t *val = out_params;
   *val = CS_MAX_H_REGIONS;
    break;
  }
  default:
    break;
  }
  return rc;
}

/** cs_stats_do_hw_update:
 *    @entry: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int cs_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsCs_CfgType);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = CS_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = CS_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    entry->hw_update_pending = 0;
  }
  return rc;
}

/** cs_stats_parse:
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
static int cs_stats_parse(isp_stats_entry_t *entry, void *raw_buf,
  q3a_cs_stats_t *cs_stats)
{
  uint32_t i;
  uint32_t *CSum;
  uint16_t *current_region;
  cs_stat_config_type_t *priv_cfg = entry->private;
  uint32_t shiftBits = priv_cfg->shift_bits;
  ISP_StatsCs_CfgType *pcmd = entry->reg_cmd;

  current_region = (uint16_t *)raw_buf;
  CSum = cs_stats->col_sum;
  /* TODO: ask Peter for the CS configuration */
  cs_stats->num_col_sum = pcmd->rgnHNum;
  for (i = 0; i < cs_stats->num_col_sum; i++)
    *CSum++ = (*current_region++) << shiftBits;
  return 0;
}

/** cs_stats_action:
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
static int cs_stats_action(void *ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  switch ((isp_stats_action_code_t)action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    rc = cs_stats_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_CS_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE: {
    isp_pipeline_stats_parse_t *action_data = data;
    mct_event_stats_isp_t *isp_stats_event = action_data->parsed_stats_event;
    mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
    int buf_idx =
      action_data->raw_stats_event->u.stats.stats_buf_idxs[MSM_ISP_STATS_CS];
    void *raw_buf = isp_get_buf_addr(entry->buf_mgr,
      entry->buf_handle, buf_idx);
    if(!raw_buf){
      CDBG_ERROR("%s: isp_get_buf_addr failed!\n", __func__);
      return -1;
    }
    q3a_cs_stats_t *cs_stats = entry->parsed_stats_buf;
    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_CS);
    rc = cs_stats_parse(entry, raw_buf, cs_stats);
    if (entry->num_bufs != 0) {
      rc |= isp_stats_enqueue_buf(entry, buf_idx);
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_CS].stats_buf = cs_stats;
      stats_data[MSM_ISP_STATS_CS].stats_type = MSM_ISP_STATS_CS;
      stats_data[MSM_ISP_STATS_CS].buf_size = sizeof(q3a_cs_stats_t);
      stats_data[MSM_ISP_STATS_CS].used_size = sizeof(q3a_cs_stats_t);
    } else {
      stats_data[MSM_ISP_STATS_CS].stats_buf = NULL;
      stats_data[MSM_ISP_STATS_CS].buf_size = 0;
      stats_data[MSM_ISP_STATS_CS].used_size = 0;
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

/** cs_stats_init:
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
static int cs_stats_init(void *ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  entry->buf_len = ISP_STATS_CS_BUF_SIZE;
  entry->stats_type = MSM_ISP_STATS_CS;
  entry->fd = init_params->fd;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;

  return 0;
}

/** cs_stats_destroy:
 *    @ctrl: pointer to instance private data
 *
 * Free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int cs_stats_destroy(void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  if (entry->private)
    free(entry->private);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  free(entry->reg_cmd);
  free(entry);
  return 0;
}

/** cs_stats32_open:
 *    @stats: isp module data
 *    @stats_type: statistic type
 *
 * Allocate instance private data for submodule.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *cs_stats32_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsCs_CfgType *cmd = NULL;
  cs_stat_config_type_t *cfg = NULL;

  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n", __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsCs_CfgType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(entry);
    return NULL;
  }
  cfg = malloc(sizeof(cs_stat_config_type_t));
  if (!cfg) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(entry);
    free(cmd);
    return NULL;
  }
  memset(entry, 0, sizeof(*entry));
  memset(cmd, 0, sizeof(*cmd));
  memset(cfg, 0, sizeof(*cfg));

  entry->len_parsed_stats_buf = sizeof(q3a_cs_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf == NULL) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(cfg);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->private = (void *)cfg;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = cs_stats_init;
  entry->ops.destroy = cs_stats_destroy;
  entry->ops.set_params = cs_stats_set_params;
  entry->ops.get_params = cs_stats_get_params;
  entry->ops.action = cs_stats_action;
  return &entry->ops;
}


