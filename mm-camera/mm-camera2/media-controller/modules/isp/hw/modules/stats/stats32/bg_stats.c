/*============================================================================

 Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

 ============================================================================*/
#include <unistd.h>
#include "bg_stats.h"
#include "isp_log.h"

#define MIN_RGN_WIDTH  4
#define MIN_RGN_HEIGHT 2

/** bg_stats_debug:
 *    @pcmd: Pointer to statistic configuration.
 *
 * Print statistic configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static void bg_stats_debug(ISP_StatsBg_CfgCmdType *pcmd)
{
  ISP_DBG(ISP_MOD_STATS, "%s:Bayer Grid Stats Configurations\n", __func__);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHOffset %d\n", __func__, pcmd->rgnHOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVOffset %d\n", __func__, pcmd->rgnVOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnWidth   %d\n", __func__, pcmd->rgnWidth);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHeight  %d\n", __func__, pcmd->rgnHeight);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHNum    %d\n", __func__, pcmd->rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVNum    %d\n", __func__, pcmd->rgnVNum);
  ISP_DBG(ISP_MOD_STATS, "%s:gbMax      %d\n", __func__, pcmd->gbMax);
  ISP_DBG(ISP_MOD_STATS, "%s:grMax      %d\n", __func__, pcmd->grMax);
  ISP_DBG(ISP_MOD_STATS, "%s:rMax       %d\n", __func__, pcmd->rMax);
  ISP_DBG(ISP_MOD_STATS, "%s:bMax       %d\n", __func__, pcmd->bMax);
}

/** bg_stats_config:
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
static int bg_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  ISP_StatsBg_CfgCmdType *pcmd = entry->reg_cmd;
  aec_bg_config_t *bg_config = &pix_settings->stats_cfg.aec_config.bg_config;
  tintless_stats_config_t stats_cfg;
  isp_tintless_notify_data_t tintless_data;
  isp_pix_camif_cfg_t *camif_cfg = &pix_settings->camif_cfg;
  uint32_t bg_rgn_width, bg_rgn_height;
  uint32_t camif_window_w_t, camif_window_h_t;
  int rc;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: BG not enabled", __func__);
    return 0;
  }
  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;

  if (pix_settings->stats_cfg.aec_config.bg_config.grid_info.h_num == 0 ||
    pix_settings->stats_cfg.aec_config.bg_config.grid_info.v_num == 0) {
      CDBG_ERROR("%s: ERROR Received stats config with h_num=%d v_num=%d\n",
        __func__, pix_settings->stats_cfg.aec_config.bg_config.grid_info.h_num,
        pix_settings->stats_cfg.aec_config.bg_config.grid_info.v_num);
      return -1;
    }


  /* Minimum region width cannot be smaller than 4 as per thardware constraints for vfe32 */
  if (bg_rgn_width < MIN_RGN_WIDTH) {
    bg_rgn_width = MIN_RGN_WIDTH;
  }
  /* Minimum region height cannot be smaller than 2 as per hardware constraints for vfe32 */
  if (bg_rgn_height < MIN_RGN_HEIGHT) {
    bg_rgn_height = MIN_RGN_HEIGHT;
  }
  camif_window_w_t = camif_cfg->sensor_out_info.request_crop.last_pixel -
    camif_cfg->sensor_out_info.request_crop.first_pixel + 1;
  camif_window_h_t = camif_cfg->sensor_out_info.request_crop.last_line -
    camif_cfg->sensor_out_info.request_crop.first_line + 1;

  if (!entry->need_to_do_fullsize_cfg) {
    bg_rgn_width = bg_config->roi.width /
      bg_config->grid_info.h_num;
    bg_rgn_height = bg_config->roi.height /
      bg_config->grid_info.v_num;
    /* Minimum region width cannot be smaller than 4 as per thardware constraints for vfe32 */
    if (bg_rgn_width < MIN_RGN_WIDTH) {
      bg_rgn_width = MIN_RGN_WIDTH;
    }
    /* Minimum region height cannot be smaller than 2 as per hardware constraints for vfe32 */
    if (bg_rgn_height < MIN_RGN_HEIGHT) {
      bg_rgn_height = MIN_RGN_HEIGHT;
    }

    pcmd->rgnHOffset = FLOOR2(pix_settings->stats_cfg.aec_config.bg_config.roi.left);
    pcmd->rgnVOffset = FLOOR2(pix_settings->stats_cfg.aec_config.bg_config.roi.top);
    pcmd->rgnWidth   = FLOOR2(bg_rgn_width) - 1;
    pcmd->rgnHeight  = FLOOR2(bg_rgn_height) - 1;
    pcmd->rgnHNum    = pix_settings->stats_cfg.aec_config.bg_config.grid_info.h_num - 1;
    pcmd->rgnVNum    = pix_settings->stats_cfg.aec_config.bg_config.grid_info.v_num - 1;

    pcmd->rMax       = 255 - 16;
    pcmd->grMax      = 255 - 16;
    pcmd->bMax       = 255 - 16;
    pcmd->gbMax      = 255 - 16;

    bg_config->roi.left = pcmd->rgnHOffset;
    bg_config->roi.top = pcmd->rgnVOffset;
    bg_config->roi.width = (pcmd->rgnWidth + 1) * bg_config->grid_info.h_num;
    bg_config->roi.height = (pcmd->rgnHeight + 1) * bg_config->grid_info.v_num;
    pix_settings->saved_zoom_roi.rgnHOffset = pcmd->rgnHOffset;
    pix_settings->saved_zoom_roi.rgnVOffset = pcmd->rgnVOffset;
    pix_settings->saved_zoom_roi.rgnWidth = pcmd->rgnWidth;
    pix_settings->saved_zoom_roi.rgnHeight = pcmd->rgnHeight;
    pix_settings->saved_zoom_roi.rgnHNum =  pcmd->rgnHNum;
    pix_settings->saved_zoom_roi.rgnVNum = pcmd->rgnVNum;
    pix_settings->saved_zoom_roi.rMax =  pcmd->rMax;
    pix_settings->saved_zoom_roi.grMax = pcmd->grMax;
    pix_settings->saved_zoom_roi.bMax = pcmd->bMax;
    pix_settings->saved_zoom_roi.gbMax = pcmd->gbMax;
    if (!entry->tinltess_cofig_stats) {
      entry->skip_stats = 1;
      entry->roi_config_skip_stats = TRUE;
    }
    entry->is_fullsize_stats = FALSE;
  } else {
    pcmd->rgnWidth   = FLOOR2(camif_window_w_t/64) - 1;
    pcmd->rgnHeight  = FLOOR2(camif_window_h_t/48) - 1;
    pcmd->rgnHOffset = FLOOR2((camif_window_w_t - 64 *(pcmd->rgnWidth + 1))/2);
    pcmd->rgnVOffset = FLOOR2((camif_window_h_t - 48 *(pcmd->rgnHeight + 1))/2);
    pcmd->rgnHNum    = 64 - 1;
    pcmd->rgnVNum    = 48 - 1;

    pcmd->rMax       = 255 - 16;
    pcmd->grMax      = 255 - 16;
    pcmd->bMax       = 255 - 16;
    pcmd->gbMax      = 255 - 16;
    entry->need_to_do_fullsize_cfg = FALSE;
    entry->is_fullsize_stats = TRUE;
  }
  entry->tinltess_cofig_stats = FALSE;

  if (pcmd->rgnVOffset + ((pcmd->rgnVNum + 1) * (pcmd->rgnHeight + 1)) > camif_window_h_t ||
      pcmd->rgnHOffset + ((pcmd->rgnHNum + 1) * (pcmd->rgnWidth + 1)) > camif_window_w_t) {
    CDBG_ERROR("%s: BG ROI bigger than CAMIF window %ux%u !", __func__,
      camif_window_w_t, camif_window_h_t);
    CDBG_ERROR("%s: Horizontal: %u %u %u, Vertical: %u %u %u", __func__,
      pcmd->rgnHOffset, pcmd->rgnHNum, pcmd->rgnWidth,
      pcmd->rgnVOffset, pcmd->rgnVNum, pcmd->rgnHeight);
    entry->hw_update_pending = 0;
    return -1;
  }

  if (pix_settings->tintless_data->is_supported &&
    pix_settings->tintless_data->is_enabled && (!entry->is_tintless_stats_configured)) {
    stats_cfg.camif_win_w =  camif_window_w_t;
    stats_cfg.camif_win_h =  camif_window_h_t;
    stats_cfg.stat_elem_w = FLOOR2(stats_cfg.camif_win_w / 64);
    stats_cfg.stat_elem_h = FLOOR2(stats_cfg.camif_win_h / 48);
    stats_cfg.stats_type = STATS_TYPE_BG;

    tintless_data.session_id = entry->session_id;
    tintless_data.notify_data = &stats_cfg;
    tintless_data.notify_data_size = sizeof(tintless_stats_config_t);

    rc = entry->notify_ops->notify(entry->notify_ops->parent,
           entry->notify_ops->handle,ISP_HW_MOD_NOTIFY_BG_PCA_STATS_CONFIG ,
           &tintless_data, sizeof(isp_tintless_notify_data_t));
    if (rc < 0) {
      CDBG_ERROR("%s: Unable to config tintless rc = %d\n", __func__, rc);
    }
  }

  bg_stats_debug(pcmd);
  entry->hw_update_pending = 1;
  return 0;
}

/** bg_stats_fullsize_config:
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
static int bg_stats_fullsize_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings, uint32_t in_param_size)
{
  int rc = 0;
  entry->need_to_do_fullsize_cfg = pix_settings->do_fullsize_cfg;
  entry->tinltess_cofig_stats = TRUE;
  rc = bg_stats_config(entry, pix_settings, sizeof(isp_hw_pix_setting_params_t));
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
static int bg_stats_start(isp_stats_entry_t *entry, boolean start)
{
  int rc = 0;
  entry->need_to_do_fullsize_cfg = FALSE;
  entry->is_fullsize_stats = FALSE;
  entry->is_current_stats_fullsize = FALSE;
  entry->roi_config_skip_stats = FALSE;
  entry->tinltess_cofig_stats = FALSE;
  /* ioctl */
  return rc;
}

/** bg_stats_enable:
 *    @entry: pointer to instance private data
 *    @in_params: input data
 *
 * Set enable.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bg_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->enable = in_params->enable;
  return 0;
}

/** bg_stats_set_params:
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
static int bg_stats_set_params(void *ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = bg_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = bg_stats_config(entry, (isp_hw_pix_setting_params_t *)in_params,
      in_param_size);
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
    break;
  case ISP_STATS_SET_TRIGGER_UPDATE:
    break;
  case ISP_STATS_SET_STREAM_CFG:
    break;
  case ISP_STATS_SET_STREAM_UNCFG:
    break;
  case ISP_HW_MOD_SET_STATS_FULLSIZE_CFG:
    rc = bg_stats_fullsize_config(entry, (isp_hw_pix_setting_params_t *) in_params,
      in_param_size);
    break;
  default:
    break;
  }

  return rc;
}

/** bg_stats_get_params:
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
static int bg_stats_get_params(void *ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_STATS_GET_ENABLE:
    break;
  case ISP_STATS_GET_STREAM_HANDLE: {
    uint32_t *handle = (uint32_t *)(out_params);
    *handle = entry->stream_handle;
    break;
  }
  case ISP_STATS_GET_STREAM_STATE:
    break;
  case ISP_STATS_GET_PARSED_STATS:
    break;
  default:
    break;
  }

  return rc;
}

/** bg_stats_do_hw_update:
 *    @entry: pointer to instance private data
 *
 * Update hardware configuration.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bg_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  ISP_StatsBg_CfgCmdType *pcmd = (ISP_StatsBg_CfgCmdType *)entry->reg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];
  if (entry->hw_update_pending) {
    entry->is_current_stats_fullsize = entry->is_new_stats_full_size;
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsBg_CfgCmdType);
    cfg_cmd.cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BG_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = BG_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    entry->hw_update_pending = 0;
    entry->is_new_stats_full_size = entry->is_fullsize_stats;
    entry->hnum = pcmd->rgnHNum;
    entry->vnum = pcmd->rgnVNum;
	if (entry->roi_config_skip_stats) {
		entry->skip_stats = 1;
		entry->roi_config_skip_stats = FALSE;
	}
  }
  return rc;
}

/** bg_stats_parse:
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
static int bg_stats_parse(isp_stats_entry_t *entry,
                   void *raw_buf,
                   q3a_bg_stats_t *bg_stats,
                   uint32_t hnum, uint32_t vnum)
{
  uint32_t *SY,*Sr, *Sb, *Sgr, *Sgb;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  uint32_t *current_region;
  uint32_t  i, x, y;

  Sr     = bg_stats->bg_r_sum;
  Sb     = bg_stats->bg_b_sum;
  Sgr    = bg_stats->bg_gr_sum;
  Sgb    = bg_stats->bg_gb_sum;
  r_num  = bg_stats->bg_r_num;
  b_num  = bg_stats->bg_b_num;
  gr_num = bg_stats->bg_gr_num;
  gb_num = bg_stats->bg_gb_num;

  current_region = (uint32_t*)raw_buf;
  bg_stats->bg_region_h_num = hnum + 1; /*64 * 48*/
  bg_stats->bg_region_v_num = vnum + 1;
  /*
   * BG Stats expect:
   * 1 - 23bit out of 32bit r_sum
   * 2 - 23bit out of 32bit b_sum
   * 3 - 23bit out of 32bit gr_sum
   * 4 - 23bit out of 32bit gb_sum
   * 5 - 15bit out of 32bit USL bnum, 15bit out of 32bit LSL rnum
   * 6 - 15bit out of 32bit USL gbnum, 15bit out of 32bit LSL grnum
   * Expect buf_size = 72*54 * 6 = 23328  (uint32)  93312
   */
  for (i = 0; i < ((hnum + 1u) * (vnum + 1u)); i++) {
    /* 64*48 regions, total 3072 */
    /* 23 bits sum of r, b, gr, gb. */

    *Sr = ((*(current_region)) & 0x007FFFFF);
    Sr++;
    current_region++;
    *Sb = ((*(current_region)) & 0x007FFFFF);
    Sb++;
    current_region++;
    *Sgr = ((*(current_region)) & 0x007FFFFF);
    Sgr++;
    current_region++;
    *Sgb = ((*(current_region)) & 0x007FFFFF);
    Sgb++;
    current_region++;
    /*15 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
    *r_num = ((*(current_region)) & 0x00007FFF);
    *b_num = ((*(current_region)) & 0x7FFF0000) >> 16;
    current_region++;
    *gr_num = ((*(current_region)) & 0x00007FFF);
    *gb_num = ((*(current_region)) & 0x7FFF0000) >> 16;
    current_region++;
    r_num++;
    b_num++;
    gr_num++;
    gb_num++;
  }
  /* convert bayer r,g,b stat into Ysum to make
   * it work on current 3a version
   * that uses 16x16 Ysum is done in 3A code
   **/
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
static int bg_stats_action(void *ctrl, uint32_t action_code, void *data,
  uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  switch (action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    rc = bg_stats_start(entry, 1);
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    rc = bg_stats_start(entry, 0);
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    rc = bg_stats_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_BG_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE: {
    isp_pipeline_stats_parse_t *action_data = data;
    mct_event_stats_isp_t *isp_stats_event = action_data->parsed_stats_event;
    mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
    int buf_idx =
      action_data->raw_stats_event->u.stats.stats_buf_idxs[MSM_ISP_STATS_BG];
    uint32_t hnum = entry->hnum;
    uint32_t vnum = entry->vnum;
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
    q3a_bg_stats_t *bg_stats = entry->parsed_stats_buf;
    if (entry->is_current_stats_fullsize){
      isp_stats_event->is_tintless_data = TRUE;
    } else {
      isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_BG);
      isp_stats_event->is_tintless_data = FALSE;
    }

    rc = bg_stats_parse(entry, raw_buf, bg_stats, hnum, vnum);
    if (entry->num_bufs != 0) {
      rc |= isp_stats_enqueue_buf(entry, buf_idx);
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_BG].stats_buf = bg_stats;
      stats_data[MSM_ISP_STATS_BG].stats_type = MSM_ISP_STATS_BG;
      stats_data[MSM_ISP_STATS_BG].buf_size = sizeof(q3a_bg_stats_t);
      stats_data[MSM_ISP_STATS_BG].used_size = sizeof(q3a_bg_stats_t);
    } else {
      stats_data[MSM_ISP_STATS_BG].stats_buf = NULL;
      stats_data[MSM_ISP_STATS_BG].buf_size = 0;
      stats_data[MSM_ISP_STATS_BG].used_size = 0;
    }
    break;
  }
  default:
    break;
  }

  return rc;
}

/** stats_init:
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
static int bg_stats_init(void *ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsBg_CfgCmdType *pcmd = entry->reg_cmd;

  entry->buf_len = ISP_STATS_BG_BUF_SIZE;
  entry->stats_type = MSM_ISP_STATS_BG;
  entry->fd = init_params->fd;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  entry->skip_stats = 0;

  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  pcmd->rgnWidth   = 0;
  pcmd->rgnHeight  = 0;
  pcmd->rgnHNum    = 63;
  pcmd->rgnVNum    = 47;
  pcmd->rMax       = 255 - 16;
  pcmd->grMax      = 255 - 16;
  pcmd->bMax       = 255 - 16;
  pcmd->gbMax      = 255 - 16;

  return 0;
}

/** stats_init:
 *    @ctrl: pointer to instance private data
 *
 * Free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int bg_stats_destroy(void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  free(entry);
  return 0;
}

/** bg_stats32_open:
 *    @stats: isp module data
 *    @stats_type: statistic type
 *
 * Allocate instance private data for submodule.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *bg_stats32_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsBg_CfgCmdType *cmd = NULL;

  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n", __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsBg_CfgCmdType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(entry);
    return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));

  entry->len_parsed_stats_buf = sizeof(q3a_bg_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf == NULL) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->is_tintless_stats_configured = FALSE;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = bg_stats_init;
  entry->ops.destroy = bg_stats_destroy;
  entry->ops.set_params = bg_stats_set_params;
  entry->ops.get_params = bg_stats_get_params;
  entry->ops.action = bg_stats_action;
  return &entry->ops;
}

